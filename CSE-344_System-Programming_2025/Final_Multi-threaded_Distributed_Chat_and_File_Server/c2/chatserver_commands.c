/**
 * chatserver_commands.c - Command processing implementation
 * CSE 344 - System Programming
 */

#include "chatserver_commands.h"
#include "chatserver_room.h"
#include "chatserver_files.h"
#include "chatserver_log.h"

/**
 * Process client command
 */
void process_command(Client* client, const char* message) {
    Command cmd;
    
    if (!client || !message) return;
    
    cmd = parse_command(message);
    
    switch (cmd.type) {
        case CMD_JOIN:
            handle_join_command(client, cmd.args);
            break;
        case CMD_LEAVE:
            handle_leave_command(client);
            break;
        case CMD_BROADCAST:
            handle_broadcast_command(client, cmd.args);
            break;
        case CMD_WHISPER:
            handle_whisper_command(client, cmd.args);
            break;
        case CMD_SENDFILE:
            handle_sendfile_command(client, cmd.args);
            break;
        case CMD_EXIT:
            handle_exit_command(client);
            break;
        case CMD_UNKNOWN:
        default:
            if (message[0] == '/') {
                /* Unknown command starting with '/' */
                send_error_to_client(client, "Unknown command. Type /help for available commands.");
            } else {
                /* Regular message, treat as broadcast if in a room */
                handle_broadcast_command(client, message);
            }
            break;
    }
    
    free(cmd.args);
}

/**
 * Parse command from message
 */
Command parse_command(const char* message) {
    Command cmd = {CMD_UNKNOWN, NULL};
    char *command, *args;
    char *temp = strdup(message);
    
    if (!temp) {
        return cmd;
    }
    
    /* Skip leading whitespace */
    while (*temp && isspace(*temp)) {
        temp++;
    }
    
    /* Check if message starts with '/' */
    if (temp[0] != '/') {
        cmd.args = strdup(temp);
        free(temp);
        return cmd;
    }
    
    /* Split into command and arguments */
    command = strtok(temp, " \t\n\r");
    args = strtok(NULL, "");
    
    /* Remove leading whitespace from args if any */
    if (args) {
        while (*args && isspace(*args)) {
            args++;
        }
        cmd.args = strdup(args);
    } else {
        cmd.args = strdup("");
    }
    
    /* Determine command type */
    if (strcmp(command, "/join") == 0) {
        cmd.type = CMD_JOIN;
    } else if (strcmp(command, "/leave") == 0) {
        cmd.type = CMD_LEAVE;
    } else if (strcmp(command, "/broadcast") == 0) {
        cmd.type = CMD_BROADCAST;
    } else if (strcmp(command, "/whisper") == 0) {
        cmd.type = CMD_WHISPER;
    } else if (strcmp(command, "/sendfile") == 0) {
        cmd.type = CMD_SENDFILE;
    } else if (strcmp(command, "/exit") == 0) {
        cmd.type = CMD_EXIT;
    }
    
    free(temp);
    return cmd;
}

/**
 * Handle join command
 */
void handle_join_command(Client* client, const char* args) {
    Room* room;
    char room_name[MAX_ROOM_NAME_LENGTH + 1];
    char message[SERVER_BUFFER_SIZE];
    
    if (!client || !args) return;
    
    /* Extract room name */
    if (sscanf(args, "%32s", room_name) != 1) {
        send_error_to_client(client, "Invalid room name. Usage: /join <room_name>");
        return;
    }
    
    /* Validate room name */
    if (!validate_room_name(room_name)) {
        send_error_to_client(client, "Invalid room name. Use alphanumeric characters only (max 32).");
        return;
    }
    
    /* Find or create room */
    room = find_room_by_name(room_name);
    if (!room) {
        /* Create new room */
        room = create_room(room_name);
        if (!room) {
            send_error_to_client(client, "Failed to create room. Try again later.");
            return;
        }
        
        if (add_room(room) < 0) {
            send_error_to_client(client, "Server room limit reached. Try again later.");
            free_room(room);
            return;
        }
        
        log_message("[ROOM] Room '%s' created by user '%s'", room_name, client->username);
    }
    
    /* Join room */
    if (join_room(client, room) < 0) {
        send_error_to_client(client, "Failed to join room. Room may be full.");
        return;
    }
    
    /* Notify client and room members */
    snprintf(message, sizeof(message), "You joined room '%s'", room_name);
    send_success_to_client(client, message);
    
    snprintf(message, sizeof(message), "User '%s' joined the room", client->username);
    broadcast_to_room(room, message, client);
    
    log_message("[INFO] %s joined room '%s'", client->username, room_name);
}

/**
 * Handle leave command
 */
void handle_leave_command(Client* client) {
    Room* room;
    char message[SERVER_BUFFER_SIZE];
    
    if (!client) return;
    
    pthread_mutex_lock(&client->mutex);
    room = client->current_room;
    pthread_mutex_unlock(&client->mutex);
    
    if (!room) {
        send_error_to_client(client, "You are not in any room.");
        return;
    }
    
    /* Notify room members */
    snprintf(message, sizeof(message), "User '%s' left the room", client->username);
    broadcast_to_room(room, message, client);
    
    /* Leave room */
    if (leave_room(client, room) < 0) {
        send_error_to_client(client, "Failed to leave room.");
        return;
    }
    
    send_success_to_client(client, "You left the room.");
}

/**
 * Handle broadcast command
 */
void handle_broadcast_command(Client* client, const char* args) {
    Room* room;
    char message[SERVER_BUFFER_SIZE];
    
    if (!client || !args) return;
    
    pthread_mutex_lock(&client->mutex);
    room = client->current_room;
    pthread_mutex_unlock(&client->mutex);
    
    if (!room) {
        send_error_to_client(client, "You are not in any room. Join a room first with /join <room_name>.");
        return;
    }
    
    /* Construct message */
    snprintf(message, sizeof(message), "[%s]: %s", client->username, args);
    
    /* Broadcast to room */
    broadcast_to_room(room, message, NULL);
    
    log_message("[BROADCAST] user '%s': %s", client->username, args);
}

/**
 * Handle whisper command
 */
void handle_whisper_command(Client* client, const char* args) {
    char target_username[MAX_USERNAME_LENGTH + 1];
    char *message_ptr;
    char message[SERVER_BUFFER_SIZE];
    Client* target;
    
    if (!client || !args) return;
    
    /* Extract target username and message */
    if (sscanf(args, "%16s", target_username) != 1) {
        send_error_to_client(client, "Invalid syntax. Usage: /whisper <username> <message>");
        return;
    }
    
    /* Find message part (after username) */
    message_ptr = strchr(args, ' ');
    if (!message_ptr) {
        send_error_to_client(client, "No message provided. Usage: /whisper <username> <message>");
        return;
    }
    
    /* Skip past username and any whitespace */
    message_ptr++;
    while (*message_ptr && isspace(*message_ptr)) {
        message_ptr++;
    }
    
    if (strlen(message_ptr) == 0) {
        send_error_to_client(client, "Empty message. Usage: /whisper <username> <message>");
        return;
    }
    
    /* Find target client */
    target = find_client_by_username(target_username);
    if (!target || target->status != CLIENT_CONNECTED) {
        send_error_to_client(client, "User not found or not connected.");
        return;
    }
    
    /* Send whisper */
    snprintf(message, sizeof(message), "[Whisper from %s]: %s", client->username, message_ptr);
    send_to_client(target, message);
    
    snprintf(message, sizeof(message), "[Whisper to %s]: %s", target_username, message_ptr);
    send_success_to_client(client, message);
    
    log_message("[WHISPER] From '%s' to '%s': %s", client->username, target_username, message_ptr);
}

/**
 * Handle sendfile command
 */
void handle_sendfile_command(Client* client, const char* args) {
    char filename[MAX_MESSAGE_LENGTH];
    char target_username[MAX_USERNAME_LENGTH + 1];
    Client* target;
    const char* file_ext;
    struct stat file_stat;
    char filepath[MAX_MESSAGE_LENGTH * 2];
    
    if (!client || !args) return;
    
    /* Extract filename and target username */
    if (sscanf(args, "%1023s %16s", filename, target_username) != 2) {
        send_error_to_client(client, "Invalid syntax. Usage: /sendfile <filename> <username>");
        return;
    }
    
    /* Find target client */
    target = find_client_by_username(target_username);
    if (!target || target->status != CLIENT_CONNECTED) {
        send_error_to_client(client, "User not found or not connected.");
        return;
    }
    
    /* Validate file extension */
    file_ext = strrchr(filename, '.');
    if (!file_ext || (
        strcmp(file_ext, ".txt") != 0 && 
        strcmp(file_ext, ".pdf") != 0 && 
        strcmp(file_ext, ".jpg") != 0 && 
        strcmp(file_ext, ".png") != 0)) {
        send_error_to_client(client, "Invalid file type. Allowed: .txt, .pdf, .jpg, .png");
        return;
    }
    
    /* Construct path to file in uploads directory */
    snprintf(filepath, sizeof(filepath), "%s/%s/%s", UPLOAD_DIR, client->username, filename);
    
    /* Create user directory if it doesn't exist */
    char user_dir[MAX_MESSAGE_LENGTH];
    snprintf(user_dir, sizeof(user_dir), "%s/%s", UPLOAD_DIR, client->username);
    if (mkdir(user_dir, 0777) < 0 && errno != EEXIST) {
        send_error_to_client(client, "Failed to prepare file transfer. Server error.");
        log_message("[ERROR] Failed to create user directory for '%s'", client->username);
        return;
    }
    
    /* Check if file exists and get size */
    if (stat(filepath, &file_stat) < 0) {
        send_error_to_client(client, "File not found on server.");
        return;
    }
    
    /* Check file size */
    if (file_stat.st_size > MAX_FILE_SIZE) {
        send_error_to_client(client, "File exceeds maximum size (3MB).");
        log_message("[ERROR] File '%s' from user '%s' exceeds size limit.", filename, client->username);
        return;
    }
    
    /* Add to upload queue */
    if (add_to_upload_queue(client, target, filepath, filename, file_stat.st_size) < 0) {
        send_error_to_client(client, "Failed to queue file transfer. Try again later.");
        return;
    }
    
    send_success_to_client(client, "File transfer queued. You will be notified when it completes.");
    
    log_message("[SEND FILE] '%s' queued from %s to %s", filename, client->username, target_username);
}

/**
 * Handle exit command
 */
void handle_exit_command(Client* client) {
    if (!client) return;
    
    send_success_to_client(client, "Goodbye! Disconnecting from server.");
    log_message("[EXIT] user '%s' disconnected", client->username);
    
    /* Set client status to disconnected to trigger cleanup */
    pthread_mutex_lock(&client->mutex);
    client->status = CLIENT_DISCONNECTED;
    pthread_mutex_unlock(&client->mutex);
}
