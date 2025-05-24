/**
 * Multi-threaded Distributed Chat and File Server
 * File transfer functionality
 */

#include "chatserver.h"

/**
 * Check if a file extension is allowed
 */
bool validate_file_extension(const char *filename) {
    if (!filename) {
        return false;
    }
    
    const char *dot = strrchr(filename, '.');
    if (!dot) {
        return false;  // No extension
    }
    
    for (int i = 0; i < FILE_EXT_COUNT; i++) {
        if (strcasecmp(dot, ALLOWED_FILE_EXTENSIONS[i]) == 0) {
            return true;
        }
    }
    
    return false;
}

/**
 * Add a file transfer to the upload queue
 */
bool add_to_upload_queue(Server *server, const char *sender, const char *recipient, const char *filename, size_t filesize) {
    bool result = false;
    
    // Wait for a slot in the queue (use semaphore)
    if (sem_wait(&server->upload_semaphore) != 0) {
        perror("sem_wait failed");
        return false;
    }
    
    // Lock the queue for modification
    pthread_mutex_lock(&server->queue_mutex);
    
    // Find an empty slot in the queue
    int slot = -1;
    for (int i = 0; i < MAX_UPLOAD_QUEUE; i++) {
        if (!server->upload_queue[i].in_progress) {
            slot = i;
            break;
        }
    }
    
    if (slot != -1) {
        // Fill the queue slot
        strncpy(server->upload_queue[slot].sender_username, sender, MAX_USERNAME_LEN);
        strncpy(server->upload_queue[slot].recipient_username, recipient, MAX_USERNAME_LEN);
        strncpy(server->upload_queue[slot].filename, filename, MAX_FILE_NAME_LEN);
        server->upload_queue[slot].filesize = filesize;
        server->upload_queue[slot].queued_time = time(NULL);
        server->upload_queue[slot].in_progress = true;
        
        server->queue_count++;
        result = true;
        
        // Create a new thread to process this file transfer
        pthread_t file_thread;
        pthread_create(&file_thread, NULL, process_upload_queue, &server->upload_queue[slot]);
        pthread_detach(file_thread);
    }
    
    pthread_mutex_unlock(&server->queue_mutex);
    
    return result;
}

/**
 * Process a file in the upload queue
 */
void *process_upload_queue(void *arg) {
    FileTransfer *transfer = (FileTransfer *)arg;
    
    // Validate input
    if (!transfer) {
        printf("[ERROR] Invalid transfer argument in process_upload_queue\n");
        pthread_exit(NULL);
    }
    
    // Get server reference
    extern Server server;
    Server *server_ptr = &server;
    
    printf("[DEBUG] Starting file transfer: %s -> %s (%s, %zu bytes)\n", 
           transfer->sender_username, transfer->recipient_username, 
           transfer->filename, transfer->filesize);
    
    // Find sender and recipient with safety checks
    int sender_index = find_client_index(server_ptr, transfer->sender_username);
    int recipient_index = find_client_index(server_ptr, transfer->recipient_username);
    
    printf("[DEBUG] Sender index: %d, Recipient index: %d\n", sender_index, recipient_index);
    
    if (sender_index == -1 || recipient_index == -1) {
        // Sender or recipient not found, cancel transfer
        printf("[DEBUG] User not found - cancelling transfer\n");
        log_message(server_ptr, "[FILE] Transfer of '%s' from %s to %s failed: user disconnected",
                  transfer->filename, transfer->sender_username, transfer->recipient_username);
        
        // Notify sender if still connected
        if (sender_index != -1) {
            notify_client(&server_ptr->clients[sender_index], 
                        "[Server]: File transfer cancelled - recipient disconnected.");
        }
        
        // Release queue slot
        pthread_mutex_lock(&server_ptr->queue_mutex);
        transfer->in_progress = false;
        server_ptr->queue_count--;
        pthread_mutex_unlock(&server_ptr->queue_mutex);
        
        sem_post(&server_ptr->upload_semaphore);
        pthread_exit(NULL);
    }
    
    Client *sender = &server_ptr->clients[sender_index];
    Client *recipient = &server_ptr->clients[recipient_index];
    
    // Validate client pointers
    if (!sender || !recipient) {
        printf("[ERROR] Invalid client pointers\n");
        // Release queue slot
        pthread_mutex_lock(&server_ptr->queue_mutex);
        transfer->in_progress = false;
        server_ptr->queue_count--;
        pthread_mutex_unlock(&server_ptr->queue_mutex);
        
        sem_post(&server_ptr->upload_semaphore);
        pthread_exit(NULL);
    }
    
    printf("[DEBUG] Sender: %s (socket: %d), Recipient: %s (socket: %d)\n", 
           sender->username, sender->socket, recipient->username, recipient->socket);
    
    // Create a unique filename to prevent collisions
    char unique_filename[MAX_FILE_NAME_LEN + 32];
    time_t now = time(NULL);
    
    // Add timestamp to filename to make it unique
    const char *dot = strrchr(transfer->filename, '.');
    if (dot) {
        int name_len = dot - transfer->filename;
        char base_name[MAX_FILE_NAME_LEN];
        strncpy(base_name, transfer->filename, name_len);
        base_name[name_len] = '\0';
        
        snprintf(unique_filename, sizeof(unique_filename), "%s_%ld%s", 
                base_name, (long)now, dot);
    } else {
        snprintf(unique_filename, sizeof(unique_filename), "%s_%ld", 
                transfer->filename, (long)now);
    }
    
    printf("[DEBUG] Unique filename: %s\n", unique_filename);
    
    // Send file transfer start notification
    char start_msg[MAX_MESSAGE_LEN];
    sprintf(start_msg, "[Server]: Starting file transfer of '%s'...", transfer->filename);
    notify_client(sender, start_msg);
    
    sprintf(start_msg, "[Server]: Receiving file '%s' from %s...", 
           transfer->filename, sender->username);
    notify_client(recipient, start_msg);
    
    // Request file data from sender
    notify_client(sender, "[Server]: Please send file data now.");
    
    // Set sender to file transfer mode to prevent interpreting file data as commands
    sender->in_file_transfer = true;
    
    // Create a buffer for file data with safety check
    if (transfer->filesize > MAX_FILE_SIZE) {
        printf("[ERROR] File size too large: %zu\n", transfer->filesize);
        notify_client(sender, "[Server]: File transfer failed - file too large.");
        notify_client(recipient, "[Server]: File transfer failed - file too large.");
        
        sender->in_file_transfer = false;
        
        // Release queue slot
        pthread_mutex_lock(&server_ptr->queue_mutex);
        transfer->in_progress = false;
        server_ptr->queue_count--;
        pthread_mutex_unlock(&server_ptr->queue_mutex);
        
        sem_post(&server_ptr->upload_semaphore);
        pthread_exit(NULL);
    }
    
    char *file_buffer = (char *)malloc(transfer->filesize + 1); // +1 for safety
    if (!file_buffer) {
        printf("[ERROR] Memory allocation failed for %zu bytes\n", transfer->filesize);
        notify_client(sender, "[Server]: File transfer failed - memory allocation error.");
        notify_client(recipient, "[Server]: File transfer failed - server error.");
        
        sender->in_file_transfer = false;
        log_file_transfer(server_ptr, sender->username, recipient->username, transfer->filename, false);
        
        // Release queue slot
        pthread_mutex_lock(&server_ptr->queue_mutex);
        transfer->in_progress = false;
        server_ptr->queue_count--;
        pthread_mutex_unlock(&server_ptr->queue_mutex);
        
        sem_post(&server_ptr->upload_semaphore);
        pthread_exit(NULL);
    }
    
    printf("[DEBUG] Allocated %zu bytes for file buffer\n", transfer->filesize);
    
    // Receive file data
    size_t total_received = 0;
    ssize_t bytes_received;
    size_t last_progress = 0;
    
    printf("[DEBUG] Starting to receive file data...\n");
    
    while (total_received < transfer->filesize) {
        size_t remaining = transfer->filesize - total_received;
        size_t to_receive = (remaining > FILE_BUFFER_SIZE) ? FILE_BUFFER_SIZE : remaining;
        
        bytes_received = recv(sender->socket, file_buffer + total_received, to_receive, 0);
        
        if (bytes_received <= 0) {
            // Error or connection closed
            printf("[ERROR] Failed to receive data: %ld, errno: %d\n", (long)bytes_received, errno);
            free(file_buffer);
            
            notify_client(sender, "[Server]: File transfer failed - connection error.");
            notify_client(recipient, "[Server]: File transfer failed - sender disconnected.");
            
            sender->in_file_transfer = false;
            log_file_transfer(server_ptr, sender->username, recipient->username, transfer->filename, false);
            
            // Release queue slot
            pthread_mutex_lock(&server_ptr->queue_mutex);
            transfer->in_progress = false;
            server_ptr->queue_count--;
            pthread_mutex_unlock(&server_ptr->queue_mutex);
            
            sem_post(&server_ptr->upload_semaphore);
            pthread_exit(NULL);
        }
        
        total_received += bytes_received;
        
        // Display progress only when it changes by at least 10%
        size_t current_progress = (total_received * 100) / transfer->filesize;
        if (current_progress >= last_progress + 10) {
            printf("[DEBUG] Receiving file '%s': %zu%%\n", transfer->filename, current_progress);
            last_progress = current_progress;
        }
    }
    
    printf("[DEBUG] Received all file data: %zu bytes\n", total_received);
    
    // Send file data to recipient
    notify_client(recipient, "[Server]: Saving file...");
    
    // Send header with filename and size
    char header[MAX_MESSAGE_LEN];
    sprintf(header, "FILE:%s:%zu\n", unique_filename, transfer->filesize);
    
    printf("[DEBUG] Sending header to recipient: %s", header);
    
    if (send(recipient->socket, header, strlen(header), 0) <= 0) {
        printf("[ERROR] Failed to send header to recipient\n");
        free(file_buffer);
        notify_client(sender, "[Server]: File transfer failed - recipient disconnected.");
        notify_client(recipient, "[Server]: File transfer failed - connection error.");
        
        sender->in_file_transfer = false;
        log_file_transfer(server_ptr, sender->username, recipient->username, transfer->filename, false);
        
        // Release queue slot
        pthread_mutex_lock(&server_ptr->queue_mutex);
        transfer->in_progress = false;
        server_ptr->queue_count--;
        pthread_mutex_unlock(&server_ptr->queue_mutex);
        
        sem_post(&server_ptr->upload_semaphore);
        pthread_exit(NULL);
    }
    
    // Send file data
    size_t total_sent = 0;
    ssize_t bytes_sent;
    last_progress = 0;
    
    printf("[DEBUG] Starting to send file data to recipient...\n");
    
    while (total_sent < transfer->filesize) {
        size_t remaining = transfer->filesize - total_sent;
        size_t to_send = (remaining > FILE_BUFFER_SIZE) ? FILE_BUFFER_SIZE : remaining;
        
        bytes_sent = send(recipient->socket, file_buffer + total_sent, to_send, 0);
        
        if (bytes_sent <= 0) {
            // Error or connection closed
            printf("[ERROR] Failed to send data to recipient: %ld\n", (long)bytes_sent);
            free(file_buffer);
            
            notify_client(sender, "[Server]: File transfer failed - recipient disconnected.");
            notify_client(recipient, "[Server]: File transfer failed - connection error.");
            
            sender->in_file_transfer = false;
            log_file_transfer(server_ptr, sender->username, recipient->username, transfer->filename, false);
            
            // Release queue slot
            pthread_mutex_lock(&server_ptr->queue_mutex);
            transfer->in_progress = false;
            server_ptr->queue_count--;
            pthread_mutex_unlock(&server_ptr->queue_mutex);
            
            sem_post(&server_ptr->upload_semaphore);
            pthread_exit(NULL);
        }
        
        total_sent += bytes_sent;
        
        // Display progress only when it changes by at least 10%
        size_t current_progress = (total_sent * 100) / transfer->filesize;
        if (current_progress >= last_progress + 10) {
            printf("[DEBUG] Sending file '%s': %zu%%\n", transfer->filename, current_progress);
            last_progress = current_progress;
        }
    }
    
    printf("[DEBUG] Sent all file data: %zu bytes\n", total_sent);
    
    // Free buffer
    free(file_buffer);
    
    // Reset file transfer flag
    sender->in_file_transfer = false;
    
    // Calculate queue wait time
    time_t transfer_time = time(NULL) - transfer->queued_time;
    
    // Notify completion
    char complete_msg[MAX_MESSAGE_LEN];
    sprintf(complete_msg, "[Server]: File '%s' sent successfully.", transfer->filename);
    notify_client(sender, complete_msg);
    
    sprintf(complete_msg, "[Server]: File '%s' received successfully from %s.", 
           transfer->filename, sender->username);
    notify_client(recipient, complete_msg);
    
    // Log successful transfer
    log_message(server_ptr, "[FILE] '%s' from user '%s' started upload after %ld seconds in queue.",
               transfer->filename, sender->username, (long)transfer_time);
    log_file_transfer(server_ptr, sender->username, recipient->username, transfer->filename, true);
    
    printf("[DEBUG] File transfer completed successfully\n");
    
    // Release queue slot
    pthread_mutex_lock(&server_ptr->queue_mutex);
    transfer->in_progress = false;
    server_ptr->queue_count--;
    pthread_mutex_unlock(&server_ptr->queue_mutex);
    
    sem_post(&server_ptr->upload_semaphore);
    pthread_exit(NULL);
}
