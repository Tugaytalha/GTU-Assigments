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
    snprintf(whisper_msg, MAX_MESSAGE_LEN, "[Whisper from %s]: %s", client->username, message);
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
    ssize_t bytes_read = recv(client->socket, buffer, sizeof(buffer) - 1, 0);
    if (bytes_read <= 0) {
        notify_client(client, "[Server]: Error - Failed to receive file size.");
        return;
    }
    
    buffer[bytes_read] = '\0';
    size_t filesize = atol(buffer);
    
    // Validate file size
    if (filesize <= 0 || filesize > MAX_FILE_SIZE) {
        notify_client(client, "[Server]: Error - File size must be between 1 and 3MB.");
        log_message(server, "[ERROR] File '%s' from user '%s' exceeds size limit.", filename, client->username);
        return;
    }
    
    // Add to upload queue
    if (add_to_upload_queue(server, client->username, recipient, filename, filesize)) {
        notify_client(client, "[Server]: File added to the upload queue.");
        
        // Notify recipient
        char notify_msg[MAX_MESSAGE_LEN];
        sprintf(notify_msg, "[Server]: User %s is sending you file '%s' (%zu bytes).", client->username, filename, filesize);
        notify_client(&server->clients[recipient_index], notify_msg);
        
        log_message(server, "[FILE-QUEUE] Upload '%s' from %s added to queue. Queue size: %d", 
                   filename, client->username, server->queue_count);
        printf("[COMMAND] %s initiated file transfer to %s\n", client->username, recipient);
    } else {
        notify_client(client, "[Server]: Error - Failed to queue file transfer. Queue might be full.");
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
