/**
 * chatserver_client.c - Client handling implementation
 * CSE 344 - System Programming
 */

#include "chatserver_client.h"
#include "chatserver_room.h"
#include "chatserver_commands.h"
#include "chatserver_log.h"

/**
 * Create a new client structure
 */
Client* create_client(int socket, struct sockaddr_in addr) {
    Client* client = (Client*)malloc(sizeof(Client));
    if (!client) {
        perror("Failed to allocate client");
        return NULL;
    }
    
    /* Initialize client fields */
    memset(client, 0, sizeof(Client));
    client->socket = socket;
    client->address = addr;
    client->status = CLIENT_CONNECTED;
    client->current_room = NULL;
    client->connect_time = time(NULL);
    client->last_activity = time(NULL);
    pthread_mutex_init(&client->mutex, NULL);
    
    /* Store IP address as string */
    inet_ntop(AF_INET, &(addr.sin_addr), client->ip_address, INET_ADDRSTRLEN);
    
    return client;
}

/**
 * Free client resources
 */
void free_client(Client* client) {
    if (!client) return;
    
    pthread_mutex_destroy(&client->mutex);
    
    /* Close socket if still open */
    if (client->socket >= 0) {
        close(client->socket);
        client->socket = -1;
    }
    
    free(client);
}

/**
 * Disconnect a client and clean up resources
 */
void disconnect_client(Client* client) {
    if (!client) return;
    
    pthread_mutex_lock(&client->mutex);
    
    /* If client is in a room, leave it */
    if (client->current_room) {
        leave_room(client, client->current_room);
        client->current_room = NULL;
    }
    
    /* Set status to disconnected */
    client->status = CLIENT_DISCONNECTED;
    
    /* Close socket */
    if (client->socket >= 0) {
        close(client->socket);
        client->socket = -1;
    }
    
    pthread_mutex_unlock(&client->mutex);
    
    /* Log disconnect */
    if (strlen(client->username) > 0) {
        log_message("[DISCONNECT] user '%s' lost connection. Cleaned up resources.", client->username);
    } else {
        log_message("[DISCONNECT] Unnamed client from %s disconnected", client->ip_address);
    }
    
    /* Remove from clients list */
    remove_client(client);
}

/**
 * Main client handler thread function
 */
void* handle_client(void* arg) {
    Client* client = (Client*)arg;
    char buffer[SERVER_BUFFER_SIZE];
    char username[MAX_USERNAME_LENGTH + 1];
    int bytes_read;
    
    if (!client) {
        return NULL;
    }
    
    /* Send welcome message */
    send_to_client(client, "Welcome to the chat server!\nPlease enter your username (max 16 chars, alphanumeric only):");
    
    /* Wait for username */
    bytes_read = recv(client->socket, buffer, sizeof(buffer) - 1, 0);
    if (bytes_read <= 0) {
        disconnect_client(client);
        return NULL;
    }
    
    buffer[bytes_read] = '\0';
    sscanf(buffer, "%16s", username); /* Read up to 16 characters */
    
    /* Validate username */
    if (!validate_username(username)) {
        send_error_to_client(client, "Invalid username. Use alphanumeric characters only (max 16).");
        disconnect_client(client);
        return NULL;
    }
    
    /* Check if username is already taken */
    if (find_client_by_username(username) != NULL) {
        send_error_to_client(client, "Username already taken. Choose another.");
        log_message("[REJECTED] Duplicate username attempted: %s", username);
        disconnect_client(client);
        return NULL;
    }
    
    /* Set client username and add to active clients */
    strncpy(client->username, username, MAX_USERNAME_LENGTH);
    if (add_client(client) < 0) {
        send_error_to_client(client, "Server full. Try again later.");
        disconnect_client(client);
        return NULL;
    }
    
    /* Log successful login */
    log_message("[LOGIN] user '%s' connected from %s", client->username, client->ip_address);
    
    /* Send welcome message with available commands */
    send_success_to_client(client, "Login successful! Welcome to the chat server.");
    send_to_client(client, "Available commands:\n"
                   "/join <room_name> - Join or create a room\n"
                   "/leave - Leave the current room\n"
                   "/broadcast <message> - Send message to everyone in the room\n"
                   "/whisper <username> <message> - Send private message\n"
                   "/sendfile <filename> <username> - Send file to user\n"
                   "/exit - Disconnect from the server");
    
    /* Main client loop */
    while (client->status == CLIENT_CONNECTED) {
        memset(buffer, 0, sizeof(buffer));
        bytes_read = recv(client->socket, buffer, sizeof(buffer) - 1, 0);
        
        /* Check for disconnect */
        if (bytes_read <= 0) {
            break;
        }
        
        buffer[bytes_read] = '\0';
        client->last_activity = time(NULL);
        
        /* Process command */
        process_command(client, buffer);
    }
    
    /* Clean up when client disconnects */
    disconnect_client(client);
    return NULL;
}

/**
 * Validate username (alphanumeric, max 16 chars)
 */
int validate_username(const char* username) {
    int i, len;
    
    if (!username) return 0;
    
    len = strlen(username);
    if (len == 0 || len > MAX_USERNAME_LENGTH) {
        return 0;
    }
    
    for (i = 0; i < len; i++) {
        if (!isalnum(username[i])) {
            return 0;
        }
    }
    
    return 1;
}

/**
 * Add client to clients array
 */
int add_client(Client* client) {
    int i;
    
    if (!client) return -1;
    
    pthread_mutex_lock(&clients_mutex);
    
    /* Find empty slot */
    for (i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i] == NULL) {
            clients[i] = client;
            pthread_mutex_unlock(&clients_mutex);
            return 0;
        }
    }
    
    /* No empty slots */
    pthread_mutex_unlock(&clients_mutex);
    return -1;
}

/**
 * Remove client from clients array
 */
void remove_client(Client* client) {
    int i;
    
    if (!client) return;
    
    pthread_mutex_lock(&clients_mutex);
    
    /* Find client and remove */
    for (i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i] == client) {
            clients[i] = NULL;
            break;
        }
    }
    
    pthread_mutex_unlock(&clients_mutex);
}

/**
 * Find client by username
 */
Client* find_client_by_username(const char* username) {
    int i;
    Client* found = NULL;
    
    if (!username || strlen(username) == 0) return NULL;
    
    pthread_mutex_lock(&clients_mutex);
    
    for (i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i] && clients[i]->status == CLIENT_CONNECTED && 
            strcmp(clients[i]->username, username) == 0) {
            found = clients[i];
            break;
        }
    }
    
    pthread_mutex_unlock(&clients_mutex);
    return found;
}

/**
 * Send message to client
 */
void send_to_client(Client* client, const char* message) {
    if (!client || !message || client->status != CLIENT_CONNECTED) return;
    
    pthread_mutex_lock(&client->mutex);
    if (client->socket >= 0) {
        send(client->socket, message, strlen(message), 0);
        send(client->socket, "\n", 1, 0);  /* Append newline for client formatting */
    }
    pthread_mutex_unlock(&client->mutex);
}

/**
 * Broadcast message to all clients in a room
 */
void broadcast_to_room(Room* room, const char* message, Client* exclude) {
    int i;
    
    if (!room || !message) return;
    
    pthread_mutex_lock(&room->mutex);
    
    for (i = 0; i < MAX_ROOM_CAPACITY; i++) {
        if (room->clients[i] && room->clients[i] != exclude && 
            room->clients[i]->status == CLIENT_CONNECTED) {
            send_to_client(room->clients[i], message);
        }
    }
    
    pthread_mutex_unlock(&room->mutex);
}

/**
 * Send error message to client (red color in terminal)
 */
void send_error_to_client(Client* client, const char* message) {
    char colored_message[SERVER_BUFFER_SIZE];
    
    if (!client || !message) return;
    
    /* Add ANSI color code for red */
    snprintf(colored_message, sizeof(colored_message), "\033[1;31m%s\033[0m", message);
    send_to_client(client, colored_message);
}

/**
 * Send success message to client (green color in terminal)
 */
void send_success_to_client(Client* client, const char* message) {
    char colored_message[SERVER_BUFFER_SIZE];
    
    if (!client || !message) return;
    
    /* Add ANSI color code for green */
    snprintf(colored_message, sizeof(colored_message), "\033[1;32m%s\033[0m", message);
    send_to_client(client, colored_message);
}
