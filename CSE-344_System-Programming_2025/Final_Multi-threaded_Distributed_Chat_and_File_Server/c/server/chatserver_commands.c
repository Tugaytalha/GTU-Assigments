/**
 * Multi-threaded Distributed Chat and File Server
 * Command handling functions
 */

#include "chatserver.h"

/**
 * Parse a command string and extract arguments
 */
CommandType parse_command(const char *command, char *args) {
    if (command[0] != '/') {
        return CMD_INVALID;
    }
    
    // Extract command and arguments
    char cmd[MAX_COMMAND_LEN] = {0};
    int i = 1;
    int j = 0;
    
    // Parse command name
    while (command[i] && !isspace(command[i]) && j < MAX_COMMAND_LEN - 1) {
        cmd[j++] = command[i++];
    }
    cmd[j] = '\0';
    
    // Skip whitespace
    while (command[i] && isspace(command[i])) {
        i++;
    }
    
    // Copy arguments
    strcpy(args, command + i);
    
    // Determine command type
    if (strcmp(cmd, "join") == 0) {
        return CMD_JOIN;
    } else if (strcmp(cmd, "leave") == 0) {
        return CMD_LEAVE;
    } else if (strcmp(cmd, "broadcast") == 0) {
        return CMD_BROADCAST;
    } else if (strcmp(cmd, "whisper") == 0) {
        return CMD_WHISPER;
    } else if (strcmp(cmd, "sendfile") == 0) {
        return CMD_SENDFILE;
    } else if (strcmp(cmd, "exit") == 0) {
        return CMD_EXIT;
    }
    
    return CMD_INVALID;
}

/**
 * Handle /join command
 */
void handle_join_command(Server *server, Client *client, const char *room_name) {
    if (strlen(room_name) == 0) {
        notify_client(client, "[Server]: Error - Room name is required. Usage: /join <room_name>");
        return;
    }
    
    if (!is_valid_room_name(room_name)) {
        notify_client(client, "[Server]: Error - Invalid room name. Use alphanumeric characters only (max 32).");
        return;
    }
    
    // Leave current room if in one
    if (strlen(client->current_room) > 0) {
        leave_room(server, client);
    }
    
    // Create or join the room
    int result = create_or_join_room(server, client, room_name);
    
    if (result == 0) {
        // Success
        char message[MAX_MESSAGE_LEN];
        sprintf(message, "[Server]: You joined the room '%s'", room_name);
        notify_client(client, message);
        
        log_client_action(server, client->username, "joined room");
        printf("[COMMAND] %s joined room '%s'\n", client->username, room_name);
    } else if (result == -1) {
        // Room full
        notify_client(client, "[Server]: Error - Room is full (max 15 users).");
    } else {
        // Other error
        notify_client(client, "[Server]: Error - Could not join room.");
    }
}

/**
 * Process clients waiting for an upload slot
 * This function checks all clients to find one that is waiting for an upload slot
 * and initiates their file transfer if a slot is available
 */
void process_waiting_clients(Server *server) {
    int slots_checked = 0;
    int max_attempts = 5; // Try this many times to assign slots to waiting clients
    
    // Log current semaphore value for debugging
    int sem_value;
    sem_getvalue(&server->upload_semaphore, &sem_value);
    log_message(server, "[FILE_QUEUE] Processing waiting clients. Current semaphore value: %d", sem_value);
    
    // Check if there are any waiting clients at all before trying to acquire semaphore
    bool has_waiting_clients = false;
    pthread_mutex_lock(&server->clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (server->clients[i].status == CLIENT_CONNECTED && 
            server->clients[i].waiting_for_upload_slot) {
            has_waiting_clients = true;
            break;
        }
    }
    pthread_mutex_unlock(&server->clients_mutex);
    
    if (!has_waiting_clients) {
        log_message(server, "[FILE_QUEUE] No clients waiting for upload slots");
        return;
    }
    
    // Try to acquire a semaphore slot
    while (slots_checked < max_attempts) {
        slots_checked++;
        
        if (sem_trywait(&server->upload_semaphore) == -1) {
            if (errno != EAGAIN) {
                log_message(server, "[ERROR] sem_trywait failed in process_waiting_clients: %s", strerror(errno));
            } else {
                log_message(server, "[FILE_QUEUE] No slots available for waiting clients");
            }
            return; // No slots available
        }
        
        // We acquired a slot, now find a waiting client
        Client *waiting_client = NULL;
        int waiting_client_index = -1;
        
        pthread_mutex_lock(&server->clients_mutex);
        
        // Find the first client waiting for an upload slot
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (server->clients[i].status == CLIENT_CONNECTED && 
                server->clients[i].waiting_for_upload_slot) {
                waiting_client = &server->clients[i];
                waiting_client_index = i;
                
                // Capture the queued info first before changing flags
                char filename[MAX_FILE_NAME_LEN];
                char recipient[MAX_USERNAME_LEN + 1];
                strncpy(filename, waiting_client->queued_filename, MAX_FILE_NAME_LEN);
                strncpy(recipient, waiting_client->queued_recipient, MAX_USERNAME_LEN + 1);
                
                // Reset the waiting flag
                waiting_client->waiting_for_upload_slot = false;
                
                // Mark client as in file transfer
                waiting_client->in_file_transfer = true;
                
                log_message(server, "[FILE_QUEUE] Found waiting client: %s. Assigned slot %d.", 
                          waiting_client->username, slots_checked);
                
                // Unlock before notification to prevent potential deadlocks
                pthread_mutex_unlock(&server->clients_mutex);
                
                // Send notifications with a small delay between them to ensure proper ordering
                notify_client(waiting_client, "[Server]: A file upload slot is now available. Your queued file transfer will begin.");
                usleep(50000); // 50ms delay to ensure messages are processed in order
                notify_client(waiting_client, "[Server]: Send file size in bytes:");
                
                log_message(server, "[FILE_QUEUE] Successfully notified client %s to start transfer.", 
                          waiting_client->username);
                
                // Try to process more waiting clients if available
                process_waiting_clients(server);
                return;
            }
        }
        
        // If we get here, we acquired a semaphore but found no waiting clients.
        // This shouldn't normally happen, but just in case...
        pthread_mutex_unlock(&server->clients_mutex);
        log_message(server, "[FILE_QUEUE] Warning: Acquired semaphore but no waiting clients found. Releasing slot.");
        sem_post(&server->upload_semaphore);
        return;
    }
}

/**
 * Handle /leave command
 */
void handle_leave_command(Server *server, Client *client) {
    if (strlen(client->current_room) == 0) {
        notify_client(client, "[Server]: Error - You are not in a room.");
        return;
    }
    
    char room_name[MAX_ROOM_NAME_LEN + 1];
    strcpy(room_name, client->current_room);
    
    leave_room(server, client);
    
    notify_client(client, "[Server]: You left the room.");
    
    log_client_action(server, client->username, "left room");
    printf("[COMMAND] %s left room '%s'\n", client->username, room_name);
}

/**
 * Handle /broadcast command
 */
void handle_broadcast_command(Server *server, Client *client, const char *message) {
    if (strlen(client->current_room) == 0) {
        notify_client(client, "[Server]: Error - Join a room first to broadcast messages.");
        return;
    }
    
    if (strlen(message) == 0) {
        notify_client(client, "[Server]: Error - Message is required. Usage: /broadcast <message>");
        return;
    }
    
    int room_index = find_room_index(server, client->current_room);
    if (room_index == -1) {
        notify_client(client, "[Server]: Error - Room not found.");
        return;
    }
    
    // Prepare broadcast message
    char broadcast_msg[MAX_MESSAGE_LEN];
    sprintf(broadcast_msg, "[%s]: %s", client->username, message);
    
    // Send message to all members in the room
    pthread_mutex_lock(&server->rooms[room_index].room_mutex);
    for (int i = 0; i < server->rooms[room_index].member_count; i++) {
        int member_socket = server->rooms[room_index].member_sockets[i];
        
        // Skip sender
        if (member_socket != client->socket) {
            send(member_socket, broadcast_msg, strlen(broadcast_msg), 0);
            send(member_socket, "\n", 1, 0);
        }
    }
    pthread_mutex_unlock(&server->rooms[room_index].room_mutex);
    
    notify_client(client, "[Server]: Message sent to room");
    
    log_message(server, "[BROADCAST] user '%s': %s", client->username, message);
    printf("[COMMAND] %s broadcasted to '%s'\n", client->username, client->current_room);
}

/**
 * Handle /whisper command
 */
void handle_whisper_command(Server *server, Client *client, const char *args) {
    char recipient[MAX_USERNAME_LEN + 1] = {0};
    char message[MAX_MESSAGE_LEN] = {0};
    
    // Parse recipient and message
    int i = 0;
    int j = 0;
    
    // Extract recipient
    while (args[i] && !isspace(args[i]) && j < MAX_USERNAME_LEN) {
        recipient[j++] = args[i++];
    }
    recipient[j] = '\0';
    
    // Skip whitespace
    while (args[i] && isspace(args[i])) {
        i++;
    }
    
    // Extract message
    strncpy(message, args + i, MAX_MESSAGE_LEN - 1);
    
    if (strlen(recipient) == 0 || strlen(message) == 0) {
        notify_client(client, "[Server]: Error - Recipient and message are required. Usage: /whisper <username> <message>");
        return;
    }
    
    // Don't allow whispering to self
    if (strcmp(recipient, client->username) == 0) {
        notify_client(client, "[Server]: Error - Cannot whisper to yourself.");
        return;
    }
    
    // Find recipient
    int recipient_index = find_client_index(server, recipient);
    if (recipient_index == -1) {
        notify_client(client, "[Server]: Error - Recipient not found or offline.");
        return;
    }
    
    // Send whisper
    char whisper_msg[MAX_MESSAGE_LEN];
    size_t prefix_len = snprintf(whisper_msg, MAX_MESSAGE_LEN, "[Whisper from %s]: ", client->username);
    if (prefix_len < MAX_MESSAGE_LEN) {
        strncat(whisper_msg, message, MAX_MESSAGE_LEN - prefix_len - 1);
    }
    notify_client(&server->clients[recipient_index], whisper_msg);
    
    notify_client(client, "[Server]: Whisper sent");
    
    log_message(server, "[WHISPER] %s to %s: %s", client->username, recipient, message);
    printf("[COMMAND] %s sent whisper to %s\n", client->username, recipient);
}

/**
 * Handle /sendfile command
 */
void handle_sendfile_command(Server *server, Client *client, const char *args) {
    char filename[MAX_FILE_NAME_LEN] = {0};
    char recipient_username[MAX_USERNAME_LEN + 1] = {0};
    bool semaphore_acquired = false; // Track if semaphore is held

    // Set in_file_transfer early to protect subsequent operations
    // from the main command parser in handle_client.
    client->in_file_transfer = true;
    
    // Parse filename and recipient
    int i = 0;
    int j = 0;
    
    // Extract filename
    while (args[i] && !isspace(args[i]) && j < MAX_FILE_NAME_LEN - 1) {
        filename[j++] = args[i++];
    }
    filename[j] = '\0';
    
    // Skip whitespace
    while (args[i] && isspace(args[i])) {
        i++;
    }
    
    // Extract recipient username
    j = 0;
    while (args[i] && j < MAX_USERNAME_LEN) {
        recipient_username[j++] = args[i++];
    }
    recipient_username[j] = '\0';
    
    if (strlen(filename) == 0 || strlen(recipient_username) == 0) {
        notify_client(client, "[Server]: Error - Filename and recipient are required. Usage: /sendfile <filename> <username>");
        client->in_file_transfer = false; // Reset flag
        return;
    }
    
    if (!validate_file_extension(filename)) {
        notify_client(client, "[Server]: Error - Invalid file extension. Allowed: .txt, .pdf, .jpg, .png");
        client->in_file_transfer = false; // Reset flag
        return;
    }
    
    if (strcmp(recipient_username, client->username) == 0) {
        notify_client(client, "[Server]: Error - Cannot send file to yourself.");
        client->in_file_transfer = false; // Reset flag
        return;
    }
    
    int recipient_index = find_client_index(server, recipient_username);
    if (recipient_index == -1) {
        notify_client(client, "[Server]: Error - Recipient not found or offline.");
        client->in_file_transfer = false; // Reset flag
        return;
    }

    log_message(server, "[FILE_QUEUE] Client %s attempting to send '%s' to %s. Checking queue...", 
                client->username, filename, recipient_username);

    // Attempt to acquire a semaphore slot
    if (sem_trywait(&server->upload_semaphore) == -1) {
        if (errno == EAGAIN) {
            log_message(server, "[FILE_QUEUE] Upload queue full for %s. Client will be notified when a slot is available.", client->username);
            
            // Get current semaphore value for debugging
            int sem_value;
            sem_getvalue(&server->upload_semaphore, &sem_value);
            log_message(server, "[FILE_QUEUE] Current semaphore value: %d (max: 5)", sem_value);
            
            // Add this file transfer request to a queue to be processed later
            pthread_mutex_lock(&server->clients_mutex);
            client->waiting_for_upload_slot = true;
            strncpy(client->queued_filename, filename, MAX_FILE_NAME_LEN);
            strncpy(client->queued_recipient, recipient_username, MAX_USERNAME_LEN);
            
            // Log all currently waiting clients for debugging
            int waiting_count = 0;
            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (server->clients[i].status == CLIENT_CONNECTED && 
                    server->clients[i].waiting_for_upload_slot) {
                    waiting_count++;
                }
            }
            pthread_mutex_unlock(&server->clients_mutex);
            
            log_message(server, "[FILE_QUEUE] Client %s marked as waiting for upload slot (%d clients waiting total)", 
                      client->username, waiting_count);
            
            // Notify client about queuing
            notify_client(client, "[Server]: File upload queue is full. Your request has been queued. You will be notified when a slot becomes available.");
            
            // Don't wait for the semaphore here - just return
            client->in_file_transfer = false;
            return;
        } else {
            perror("sem_trywait failed");
            log_message(server, "[ERROR] sem_trywait failed for %s: %s", client->username, strerror(errno));
            notify_client(client, "[Server]: Error - Could not check file transfer queue.");
            client->in_file_transfer = false; // Reset flag
            return; // Semaphore not acquired
        }
    } else {
        semaphore_acquired = true;
        log_message(server, "[FILE_QUEUE] Slot acquired immediately for %s.", client->username);
    }

    // SEMAPHORE IS ACQUIRED if semaphore_acquired is true

    // Request file size
    notify_client(client, "[Server]: Send file size in bytes:");
    
    char buffer[32];
    memset(buffer, 0, sizeof(buffer));
    ssize_t bytes_read = recv(client->socket, buffer, sizeof(buffer) - 1, 0);
    
    if (bytes_read <= 0) {
        notify_client(client, "[Server]: Error - Failed to receive file size.");
        client->in_file_transfer = false;
        if (semaphore_acquired) {
            log_message(server, "[FILE_QUEUE] Slot released by %s (failed to recv size).", client->username);
            sem_post(&server->upload_semaphore);
            process_waiting_clients(server);
        }
        return;
    }
    
    buffer[bytes_read] = '\0';
    char *endptr;
    size_t filesize = strtoull(buffer, &endptr, 10);
    
    if (*endptr != '\0' || (*endptr == '\0' && buffer[0] == '\0') || filesize == 0) { // also check if buffer was empty
        notify_client(client, "[Server]: Error - Invalid file size format.");
        client->in_file_transfer = false;
        if (semaphore_acquired) {
            log_message(server, "[FILE_QUEUE] Slot released by %s (invalid file size format: '%s').", client->username, buffer);
            sem_post(&server->upload_semaphore);
            process_waiting_clients(server);
        }
        return;
    }
    
    if (filesize > MAX_FILE_SIZE) {
        notify_client(client, "[Server]: Error - File size must be between 1 and 3MB.");
        log_message(server, "[ERROR] File '%s' from user '%s' exceeds size limit (%zu bytes).", filename, client->username, filesize);
        client->in_file_transfer = false;
        if (semaphore_acquired) {
            log_message(server, "[FILE_QUEUE] Slot released by %s (file too large).", client->username);
            sem_post(&server->upload_semaphore);
            process_waiting_clients(server);
        }
        return;
    }
    
    notify_client(client, "[Server]: Ready to receive file data. Please send now.");
    
    char *file_buffer_ptr = (char *)malloc(filesize);
    if (!file_buffer_ptr) {
        notify_client(client, "[Server]: Error - Memory allocation failed for file buffer.");
        log_message(server, "[ERROR] Failed to malloc file buffer for %s (size %zu).", client->username, filesize);
        client->in_file_transfer = false;
        if (semaphore_acquired) {
            log_message(server, "[FILE_QUEUE] Slot released by %s (malloc failed).", client->username);
            sem_post(&server->upload_semaphore);
            process_waiting_clients(server);
        }
        return;
    }
    
    // client->in_file_transfer is already true
    size_t total_received = 0;
    ssize_t data_received_chunk;
    
    while (total_received < filesize) {
        size_t remaining = filesize - total_received;
        size_t to_receive = (remaining > FILE_BUFFER_SIZE) ? FILE_BUFFER_SIZE : remaining;
        data_received_chunk = recv(client->socket, file_buffer_ptr + total_received, to_receive, 0);
        if (data_received_chunk <= 0) {
            log_message(server, "[ERROR] Failed to receive file data chunk from %s (read %ld, errno %d).", client->username, (long)data_received_chunk, errno);
            notify_client(client, "[Server]: Error - File transfer failed during reception.");
            free(file_buffer_ptr);
            client->in_file_transfer = false;
            if (semaphore_acquired) {
                log_message(server, "[FILE_QUEUE] Slot released by %s (recv data chunk failed).", client->username);
                sem_post(&server->upload_semaphore);
                process_waiting_clients(server);
            }
            return;
        }
        total_received += data_received_chunk;
    }
    
    // File received from sender, now send to recipient
    char unique_filename[MAX_FILE_NAME_LEN + 64];
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    int client_id = -1;
    
    // Find client index to use as unique identifier
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (&server->clients[i] == client) {
            client_id = i;
            break;
        }
    }
    
    // Extract the file extension
    const char *dot_char = strrchr(filename, '.');
    if (dot_char) {
        int name_len = dot_char - filename;
        char base_name[MAX_FILE_NAME_LEN];
        strncpy(base_name, filename, name_len);
        base_name[name_len] = '\0';
        // Use both seconds and nanoseconds along with client ID for guaranteed uniqueness
        snprintf(unique_filename, sizeof(unique_filename), "%s_%ld_%ld_%d%s", 
                base_name, (long)ts.tv_sec, (long)(ts.tv_nsec / 1000000), client_id, dot_char);
    } else {
        snprintf(unique_filename, sizeof(unique_filename), "%s_%ld_%ld_%d", 
                filename, (long)ts.tv_sec, (long)(ts.tv_nsec / 1000000), client_id);
    }
    
    log_message(server, "[FILE] Generated unique filename: %s", unique_filename);
    
    char header[MAX_MESSAGE_LEN];
    sprintf(header, "FILE:%s:%zu\n", unique_filename, filesize);
    if (send(server->clients[recipient_index].socket, header, strlen(header), 0) <= 0) {
        log_message(server, "[ERROR] Failed to send file header to recipient %s.", recipient_username);
        notify_client(client, "[Server]: Error - Failed to initiate file transfer with recipient.");
        free(file_buffer_ptr);
        client->in_file_transfer = false;
        if (semaphore_acquired) {
            log_message(server, "[FILE_QUEUE] Slot released by %s (send header to recp failed).", client->username);
            sem_post(&server->upload_semaphore);
            process_waiting_clients(server);
        }
        return;
    }
    
    size_t total_sent = 0;
    ssize_t data_sent_chunk;
    while (total_sent < filesize) {
        size_t remaining_to_send = filesize - total_sent;
        size_t to_send_chunk = (remaining_to_send > FILE_BUFFER_SIZE) ? FILE_BUFFER_SIZE : remaining_to_send;
        data_sent_chunk = send(server->clients[recipient_index].socket, file_buffer_ptr + total_sent, to_send_chunk, 0);
        if (data_sent_chunk <= 0) {
            log_message(server, "[ERROR] Failed to send file data chunk to %s (sent %ld, errno %d).", recipient_username, (long)data_sent_chunk, errno);
            notify_client(client, "[Server]: Error - Failed to send file data to recipient.");
            // Recipient might be disconnected, sender's part was mostly ok up to here.
            free(file_buffer_ptr);
            client->in_file_transfer = false;
            if (semaphore_acquired) {
                sem_post(&server->upload_semaphore);
                log_message(server, "[FILE_QUEUE] Slot released by %s (send data chunk to recp failed).", client->username);
                process_waiting_clients(server);
            }
            return;
        }
        total_sent += data_sent_chunk;
    }
    
    free(file_buffer_ptr);
    notify_client(client, "[Server]: File sent successfully.");
    char notify_msg[MAX_MESSAGE_LEN];
    snprintf(notify_msg, MAX_MESSAGE_LEN, "[Server]: File '%s' (as '%s') received successfully from %s.", filename, unique_filename, client->username);
    notify_client(&server->clients[recipient_index], notify_msg);
    
    log_message(server, "[FILE] '%s' (sent as '%s') from %s to %s successfully (%zu bytes)", filename, unique_filename, client->username, recipient_username, filesize);
    // log_file_transfer(server, client->username, recipient_username, filename, true); // Assuming this function exists and is defined

    client->in_file_transfer = false; // Reset flag on successful completion
    if (semaphore_acquired) {
        sem_post(&server->upload_semaphore);
        log_message(server, "[FILE_QUEUE] Slot released by %s (transfer complete).", client->username);
        process_waiting_clients(server);
    }
}

/**
 * Handle /exit command
 */
void handle_exit_command(Server *server, Client *client) {
    // Mark server parameter as used to avoid compiler warning
    (void)server;
    
    notify_client(client, "[Server]: Disconnected. Goodbye!");
    client->status = CLIENT_DISCONNECTED;
}
