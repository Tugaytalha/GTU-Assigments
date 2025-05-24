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
    char recipient[MAX_USERNAME_LEN + 1] = {0};
    
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
    
    // Extract recipient
    j = 0;
    while (args[i] && j < MAX_USERNAME_LEN) {
        recipient[j++] = args[i++];
    }
    recipient[j] = '\0';
    
    if (strlen(filename) == 0 || strlen(recipient) == 0) {
        notify_client(client, "[Server]: Error - Filename and recipient are required. Usage: /sendfile <filename> <username>");
        return;
    }
    
    // Validate file extension
    if (!validate_file_extension(filename)) {
        notify_client(client, "[Server]: Error - Invalid file extension. Allowed: .txt, .pdf, .jpg, .png");
        return;
    }
    
    // Don't allow sending files to self
    if (strcmp(recipient, client->username) == 0) {
        notify_client(client, "[Server]: Error - Cannot send file to yourself.");
        return;
    }
    
    // Find recipient
    int recipient_index = find_client_index(server, recipient);
    if (recipient_index == -1) {
        notify_client(client, "[Server]: Error - Recipient not found or offline.");
        return;
    }
    
    // Request file size
    notify_client(client, "[Server]: Send file size in bytes:");
    
    char buffer[32];
    memset(buffer, 0, sizeof(buffer));
    ssize_t bytes_read = recv(client->socket, buffer, sizeof(buffer) - 1, 0);
    
    if (bytes_read <= 0) {
        notify_client(client, "[Server]: Error - Failed to receive file size.");
        return;
    }
    
    buffer[bytes_read] = '\0';
    char *endptr;
    size_t filesize = strtoull(buffer, &endptr, 10);
    
    if (*endptr != '\0' || filesize == 0) {
        notify_client(client, "[Server]: Error - Invalid file size format.");
        return;
    }
    
    // Validate file size
    if (filesize > MAX_FILE_SIZE) {
        notify_client(client, "[Server]: Error - File size must be between 1 and 3MB.");
        log_message(server, "[ERROR] File '%s' from user '%s' exceeds size limit.", filename, client->username);
        return;
    }
    
    // Send acknowledgment to client that we're ready for file data
    notify_client(client, "[Server]: Ready to receive file data. Please send now.");
    
    // Create file buffer for immediate processing
    char *file_buffer = (char *)malloc(filesize);
    if (!file_buffer) {
        notify_client(client, "[Server]: Error - Memory allocation failed.");
        return;
    }
    
    printf("[DEBUG] Allocated buffer for immediate file receive: %zu bytes\n", filesize);
    
    // Set client to file transfer mode
    client->in_file_transfer = true;
    
    // Receive file data immediately
    size_t total_received = 0;
    ssize_t data_received;
    
    printf("[DEBUG] Starting immediate file data reception...\n");
    
    while (total_received < filesize) {
        size_t remaining = filesize - total_received;
        size_t to_receive = (remaining > FILE_BUFFER_SIZE) ? FILE_BUFFER_SIZE : remaining;
        
        data_received = recv(client->socket, file_buffer + total_received, to_receive, 0);
        
        if (data_received <= 0) {
            printf("[ERROR] Failed to receive file data: %ld, errno: %d\n", (long)data_received, errno);
            free(file_buffer);
            client->in_file_transfer = false;
            notify_client(client, "[Server]: Error - File transfer failed during reception.");
            return;
        }
        
        total_received += data_received;
        
        // Show progress
        size_t progress = (total_received * 100) / filesize;
        if (progress % 20 == 0) {
            printf("[DEBUG] File reception progress: %zu%%\n", progress);
        }
    }
    
    printf("[DEBUG] File data received successfully: %zu bytes\n", total_received);
    
    // Reset file transfer flag
    client->in_file_transfer = false;
    
    // Now send the file to recipient immediately
    char unique_filename[MAX_FILE_NAME_LEN + 32];
    time_t now = time(NULL);
    
    // Add timestamp to filename to make it unique
    const char *dot = strrchr(filename, '.');
    if (dot) {
        int name_len = dot - filename;
        char base_name[MAX_FILE_NAME_LEN];
        strncpy(base_name, filename, name_len);
        base_name[name_len] = '\0';
        
        snprintf(unique_filename, sizeof(unique_filename), "%s_%ld%s", 
                base_name, (long)now, dot);
    } else {
        snprintf(unique_filename, sizeof(unique_filename), "%s_%ld", 
                filename, (long)now);
    }
    
    printf("[DEBUG] Sending file to recipient with unique name: %s\n", unique_filename);
    
    // Send file header to recipient
    char header[MAX_MESSAGE_LEN];
    sprintf(header, "FILE:%s:%zu\n", unique_filename, filesize);
    
    if (send(server->clients[recipient_index].socket, header, strlen(header), 0) <= 0) {
        printf("[ERROR] Failed to send header to recipient\n");
        free(file_buffer);
        notify_client(client, "[Server]: Error - Failed to send file to recipient.");
        return;
    }
    
    printf("[DEBUG] Header sent to recipient: %s", header);
    
    // Send file data to recipient
    size_t total_sent = 0;
    ssize_t data_sent;
    
    while (total_sent < filesize) {
        size_t remaining = filesize - total_sent;
        size_t to_send = (remaining > FILE_BUFFER_SIZE) ? FILE_BUFFER_SIZE : remaining;
        
        data_sent = send(server->clients[recipient_index].socket, file_buffer + total_sent, to_send, 0);
        
        if (data_sent <= 0) {
            printf("[ERROR] Failed to send file data to recipient: %ld\n", (long)data_sent);
            free(file_buffer);
            notify_client(client, "[Server]: Error - Failed to send file to recipient.");
            return;
        }
        
        total_sent += data_sent;
        
        // Show progress
        size_t progress = (total_sent * 100) / filesize;
        if (progress % 20 == 0) {
            printf("[DEBUG] File send progress: %zu%%\n", progress);
        }
    }
    
    printf("[DEBUG] File sent successfully to recipient: %zu bytes\n", total_sent);
    
    // Free buffer
    free(file_buffer);
    
    // Notify both parties
    notify_client(client, "[Server]: File sent successfully.");
    
    char notify_msg[MAX_MESSAGE_LEN];
    snprintf(notify_msg, MAX_MESSAGE_LEN, "[Server]: File '%s' received successfully from %s.", 
            filename, client->username);
    notify_client(&server->clients[recipient_index], notify_msg);
    
    // Log the transfer
    log_message(server, "[FILE] '%s' sent from %s to %s successfully", 
               filename, client->username, recipient);
    log_file_transfer(server, client->username, recipient, filename, true);
    
    printf("[COMMAND] %s completed file transfer to %s\n", client->username, recipient);
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
