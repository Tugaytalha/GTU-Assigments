/**
 * chatserver_files.c - File transfer implementation
 * CSE 344 - System Programming
 */

#include "chatserver_files.h"
#include "chatserver_log.h"

#define FILE_CHUNK_SIZE 4096
#define FILE_COMMAND_PREFIX "FILE_TRANSFER:"

/**
 * Add file transfer to upload queue
 */
int add_to_upload_queue(Client* sender, Client* recipient, const char* filepath, const char* original_filename, size_t file_size) {
    FileTransfer* transfer;
    pthread_t thread_id;
    int queue_position = -1;
    
    if (!sender || !recipient || !filepath || !original_filename) {
        return -1;
    }
    
    /* Create transfer structure */
    transfer = (FileTransfer*)malloc(sizeof(FileTransfer));
    if (!transfer) {
        perror("Failed to allocate file transfer");
        return -1;
    }
    
    /* Initialize transfer fields */
    transfer->sender = sender;
    transfer->recipient = recipient;
    strncpy(transfer->filename, filepath, MAX_MESSAGE_LENGTH - 1);
    strncpy(transfer->original_filename, original_filename, MAX_MESSAGE_LENGTH - 1);
    transfer->file_size = file_size;
    transfer->queue_time = time(NULL);
    transfer->start_time = 0;
    transfer->end_time = 0;
    transfer->status = TRANSFER_PENDING;
    
    /* Add to queue */
    pthread_mutex_lock(&upload_queue_mutex);
    
    /* Find empty slot */
    for (int i = 0; i < MAX_UPLOAD_QUEUE; i++) {
        if (upload_queue[i] == NULL) {
            upload_queue[i] = transfer;
            upload_queue_size++;
            queue_position = i;
            break;
        }
    }
    
    /* Log queue status */
    log_message("[FILE-QUEUE] Upload '%s' from %s added to queue. Queue size: %d", 
                original_filename, sender->username, upload_queue_size);
                
    pthread_mutex_unlock(&upload_queue_mutex);
    
    if (queue_position < 0) {
        free(transfer);
        return -1;
    }
    
    /* Wait on semaphore before starting transfer thread */
    if (sem_wait(&upload_queue_semaphore) < 0) {
        perror("sem_wait failed");
        pthread_mutex_lock(&upload_queue_mutex);
        upload_queue[queue_position] = NULL;
        upload_queue_size--;
        pthread_mutex_unlock(&upload_queue_mutex);
        free(transfer);
        return -1;
    }
    
    /* Start transfer thread */
    if (pthread_create(&thread_id, NULL, process_upload_queue, (void*)transfer) != 0) {
        perror("pthread_create failed for file transfer");
        sem_post(&upload_queue_semaphore);
        pthread_mutex_lock(&upload_queue_mutex);
        upload_queue[queue_position] = NULL;
        upload_queue_size--;
        pthread_mutex_unlock(&upload_queue_mutex);
        free(transfer);
        return -1;
    }
    
    /* Store thread ID and detach */
    transfer->thread_id = thread_id;
    pthread_detach(thread_id);
    
    return 0;
}

/**
 * Process a file transfer from the upload queue
 */
void* process_upload_queue(void* arg) {
    FileTransfer* transfer = (FileTransfer*)arg;
    FILE* file = NULL;
    char buffer[FILE_CHUNK_SIZE];
    char message[SERVER_BUFFER_SIZE];
    size_t bytes_read = 0;
    size_t total_bytes_sent = 0;
    FileHeader header;
    
    if (!transfer) {
        sem_post(&upload_queue_semaphore);
        return NULL;
    }
    
    /* Mark transfer as in progress */
    transfer->start_time = time(NULL);
    transfer->status = TRANSFER_IN_PROGRESS;
    
    /* Log transfer start */
    log_message("[FILE] '%s' from user '%s' started upload after %ld seconds in queue.", 
                transfer->original_filename, transfer->sender->username, 
                (long)(transfer->start_time - transfer->queue_time));
                
    /* Notify sender that transfer has started */
    snprintf(message, sizeof(message), "File transfer of '%s' to %s has started.", 
             transfer->original_filename, transfer->recipient->username);
    send_success_to_client(transfer->sender, message);
    
    /* Open file */
    file = fopen(transfer->filename, "rb");
    if (!file) {
        log_message("[ERROR] Failed to open file '%s' for transfer", transfer->filename);
        notify_transfer_complete(transfer, 0);
        sem_post(&upload_queue_semaphore);
        return NULL;
    }
    
    /* Prepare and send file header */
    memset(&header, 0, sizeof(header));
    header.file_size = transfer->file_size;
    strncpy(header.filename, transfer->original_filename, MAX_MESSAGE_LENGTH - 1);
    strncpy(header.sender, transfer->sender->username, MAX_USERNAME_LENGTH);
    
    /* Check if recipient is still connected before sending */
    pthread_mutex_lock(&clients_mutex);
    int recipient_connected = (transfer->recipient->status == CLIENT_CONNECTED);
    pthread_mutex_unlock(&clients_mutex);
    
    if (!recipient_connected) {
        log_message("[FILE] Recipient '%s' disconnected, aborting transfer", 
                    transfer->recipient->username);
        fclose(file);
        notify_transfer_complete(transfer, 0);
        sem_post(&upload_queue_semaphore);
        return NULL;
    }
    
    /* Construct file transfer command */
    char header_buffer[sizeof(FileHeader) + 32];
    snprintf(header_buffer, sizeof(header_buffer), "%s%lu\n", FILE_COMMAND_PREFIX, (unsigned long)sizeof(header));
    
    /* Send command prefix and header size */
    pthread_mutex_lock(&transfer->recipient->mutex);
    if (send(transfer->recipient->socket, header_buffer, strlen(header_buffer), 0) <= 0) {
        pthread_mutex_unlock(&transfer->recipient->mutex);
        fclose(file);
        notify_transfer_complete(transfer, 0);
        sem_post(&upload_queue_semaphore);
        return NULL;
    }
    
    /* Send file header */
    if (send(transfer->recipient->socket, &header, sizeof(header), 0) <= 0) {
        pthread_mutex_unlock(&transfer->recipient->mutex);
        fclose(file);
        notify_transfer_complete(transfer, 0);
        sem_post(&upload_queue_semaphore);
        return NULL;
    }
    pthread_mutex_unlock(&transfer->recipient->mutex);
    
    /* Send file data in chunks */
    while ((bytes_read = fread(buffer, 1, FILE_CHUNK_SIZE, file)) > 0) {
        /* Check if recipient is still connected before sending chunk */
        pthread_mutex_lock(&clients_mutex);
        recipient_connected = (transfer->recipient->status == CLIENT_CONNECTED);
        pthread_mutex_unlock(&clients_mutex);
        
        if (!recipient_connected) {
            log_message("[FILE] Recipient '%s' disconnected during transfer, aborting", 
                        transfer->recipient->username);
            fclose(file);
            notify_transfer_complete(transfer, 0);
            sem_post(&upload_queue_semaphore);
            return NULL;
        }
        
        /* Send chunk */
        pthread_mutex_lock(&transfer->recipient->mutex);
        if (send(transfer->recipient->socket, buffer, bytes_read, 0) <= 0) {
            pthread_mutex_unlock(&transfer->recipient->mutex);
            fclose(file);
            notify_transfer_complete(transfer, 0);
            sem_post(&upload_queue_semaphore);
            return NULL;
        }
        pthread_mutex_unlock(&transfer->recipient->mutex);
        
        total_bytes_sent += bytes_read;
    }
    
    /* Close file */
    fclose(file);
    
    /* Verify all bytes were sent */
    if (total_bytes_sent != transfer->file_size) {
        log_message("[ERROR] File transfer incomplete: %zu of %zu bytes sent", 
                    total_bytes_sent, transfer->file_size);
        notify_transfer_complete(transfer, 0);
    } else {
        /* Success! */
        log_message("[SEND FILE] '%s' sent from %s to %s (success)", 
                    transfer->original_filename, transfer->sender->username, transfer->recipient->username);
        notify_transfer_complete(transfer, 1);
    }
    
    /* Release semaphore and exit */
    sem_post(&upload_queue_semaphore);
    return NULL;
}

/**
 * Notify clients about transfer completion and clean up
 */
void notify_transfer_complete(FileTransfer* transfer, int success) {
    char message[SERVER_BUFFER_SIZE];
    
    if (!transfer) return;
    
    transfer->end_time = time(NULL);
    transfer->status = success ? TRANSFER_COMPLETED : TRANSFER_FAILED;
    
    /* Notify sender */
    if (transfer->sender && transfer->sender->status == CLIENT_CONNECTED) {
        if (success) {
            snprintf(message, sizeof(message), "File '%s' successfully transferred to %s.", 
                     transfer->original_filename, transfer->recipient->username);
            send_success_to_client(transfer->sender, message);
        } else {
            snprintf(message, sizeof(message), "File '%s' transfer to %s failed.", 
                     transfer->original_filename, transfer->recipient->username);
            send_error_to_client(transfer->sender, message);
        }
    }
    
    /* Notify recipient if transfer failed */
    if (!success && transfer->recipient && transfer->recipient->status == CLIENT_CONNECTED) {
        snprintf(message, sizeof(message), "File '%s' from %s failed to transfer.", 
                 transfer->original_filename, transfer->sender->username);
        send_error_to_client(transfer->recipient, message);
    }
    
    /* Remove from upload queue */
    remove_from_upload_queue(transfer);
}

/**
 * Remove transfer from upload queue
 */
void remove_from_upload_queue(FileTransfer* transfer) {
    int i;
    
    if (!transfer) return;
    
    pthread_mutex_lock(&upload_queue_mutex);
    
    /* Find transfer and remove */
    for (i = 0; i < MAX_UPLOAD_QUEUE; i++) {
        if (upload_queue[i] == transfer) {
            upload_queue[i] = NULL;
            upload_queue_size--;
            break;
        }
    }
    
    pthread_mutex_unlock(&upload_queue_mutex);
    
    /* Free transfer structure */
    free(transfer);
}

/**
 * Handle incoming file data
 * This would be used if the server was forwarding files directly from client to client
 * For this implementation, we're using server-managed file transfers
 */
int handle_file_receive(Client* client __attribute__((unused)), const char* buffer __attribute__((unused)), size_t size __attribute__((unused))) {
    /* This is a placeholder for client-side file receiving implementation */
    /* In our current design, clients don't send files directly to the server */
    /* Instead, the server reads files from its local storage and sends them to recipients */
    return 0;
}
