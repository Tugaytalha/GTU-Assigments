/**
 * Multi-threaded Distributed Chat and File Server
 * Client management functions
 */

#include "chatserver.h"

/**
 * Authenticate a new client
 */
bool authenticate_client(Client *client) {
    char buffer[MAX_MESSAGE_LEN];
    ssize_t bytes_read;
    
    // Request username
    send(client->socket, "Enter your username: ", 21, 0);
    
    // Set client status to authenticating
    client->status = CLIENT_AUTHENTICATING;
    
    // Receive username
    bytes_read = recv(client->socket, buffer, MAX_USERNAME_LEN, 0);
    if (bytes_read <= 0) {
        printf("[ERROR] Failed to receive username\n");
        return false;
    }
    
    buffer[bytes_read] = '\0';
    
    // Remove newline character if present
    if (buffer[bytes_read - 1] == '\n') {
        buffer[bytes_read - 1] = '\0';
    }
    
    // Validate username
    if (!is_valid_username(buffer)) {
        send(client->socket, "[ERROR] Invalid username. Must be alphanumeric, 1-16 characters.\n", 66, 0);
        return false;
    }
    
    // Check if username is already taken - use global server
    pthread_mutex_lock(&server.clients_mutex);
    bool taken = is_username_taken(&server, buffer);
    pthread_mutex_unlock(&server.clients_mutex);
    
    if (taken) {
        send(client->socket, "[ERROR] Username already taken. Choose another.\n", 48, 0);
        log_message(&server, "[REJECTED] Duplicate username attempted: %s", buffer);
        return false;
    }
    
    // Username is valid and available, store it
    strncpy(client->username, buffer, MAX_USERNAME_LEN);
    client->status = CLIENT_CONNECTED;
    
    return true;
}

/**
 * Disconnect a client and clean up resources
 */
void disconnect_client(Server *server, Client *client) {
    if (client->status != CLIENT_CONNECTED) {
        return;
    }
    
    // Leave current room if in one
    if (strlen(client->current_room) > 0) {
        leave_room(server, client);
    }
    
    // Set client status to disconnected
    pthread_mutex_lock(&server->clients_mutex);
    client->status = CLIENT_DISCONNECTED;
    close(client->socket);
    pthread_mutex_unlock(&server->clients_mutex);
    
    log_client_action(server, client->username, "disconnected");
    printf("[DISCONNECT] Client %s disconnected\n", client->username);
}

/**
 * Find client index by username
 */
int find_client_index(Server *server, char *username) {
    int index = -1;
    
    pthread_mutex_lock(&server->clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (server->clients[i].status == CLIENT_CONNECTED && 
            strcmp(server->clients[i].username, username) == 0) {
            index = i;
            break;
        }
    }
    pthread_mutex_unlock(&server->clients_mutex);
    
    return index;
}

/**
 * Send a notification message to a client
 */
void notify_client(Client *client, const char *message) {
    if (client->status == CLIENT_CONNECTED) {
        send(client->socket, message, strlen(message), 0);
        send(client->socket, "\n", 1, 0);
    }
}

/**
 * Check if a username is valid (alphanumeric, 1-16 chars)
 */
bool is_valid_username(const char *username) {
    if (username == NULL || strlen(username) == 0 || strlen(username) > MAX_USERNAME_LEN) {
        return false;
    }
    
    for (int i = 0; username[i]; i++) {
        if (!isalnum(username[i])) {
            return false;
        }
    }
    
    return true;
}

/**
 * Check if a room name is valid (alphanumeric, 1-32 chars)
 */
bool is_valid_room_name(const char *room_name) {
    if (room_name == NULL || strlen(room_name) == 0 || strlen(room_name) > MAX_ROOM_NAME_LEN) {
        return false;
    }
    
    for (int i = 0; room_name[i]; i++) {
        if (!isalnum(room_name[i])) {
            return false;
        }
    }
    
    return true;
}

/**
 * Check if a username is already taken
 */
bool is_username_taken(Server *server, const char *username) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (server->clients[i].status == CLIENT_CONNECTED && 
            strcmp(server->clients[i].username, username) == 0) {
            return true;
        }
    }
    
    return false;
}
