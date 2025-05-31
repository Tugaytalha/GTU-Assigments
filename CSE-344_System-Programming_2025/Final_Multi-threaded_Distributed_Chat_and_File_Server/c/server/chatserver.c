/**
 * Multi-threaded Distributed Chat and File Server
 * Server implementation
 */

#include "chatserver.h"
#include <stdarg.h>

// Global server instance for signal handling
Server server;

// Define the allowed file extensions array
const char *ALLOWED_FILE_EXTENSIONS[FILE_EXT_COUNT] = {
    ".txt", ".pdf", ".jpg", ".png"
};

/**
 * Signal handler for SIGINT (Ctrl+C)
 */
void handle_sigint(int sig) {
    (void)sig; // Mark sig as used to avoid warning
    printf("\n[SHUTDOWN] SIGINT received. Shutting down server...\n");
    log_message(&server, "[SHUTDOWN] SIGINT received. Disconnecting %d clients, saving logs.", 0);
    server.running = false;
    
    // Notify all clients
    pthread_mutex_lock(&server.clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (server.clients[i].status == CLIENT_CONNECTED) {
            notify_client(&server.clients[i], "[Server]: Server shutting down. Goodbye!");
            close(server.clients[i].socket);
        }
    }
    pthread_mutex_unlock(&server.clients_mutex);
    
    // Close server socket
    close(server.socket);
}

/**
 * Initialize server on specified port
 */
bool initialize_server(Server *server, int port) {
    // Initialize server structure
    memset(server, 0, sizeof(Server));
    server->running = true;
    
    // Initialize mutexes and semaphores
    if (pthread_mutex_init(&server->clients_mutex, NULL) != 0 ||
        pthread_mutex_init(&server->rooms_mutex, NULL) != 0 ||
        pthread_mutex_init(&server->queue_mutex, NULL) != 0 ||
        sem_init(&server->upload_semaphore, 0, MAX_UPLOAD_QUEUE) != 0) {
        perror("Failed to initialize synchronization primitives");
        return false;
    }
    
    // Initialize rooms
    for (int i = 0; i < MAX_ROOMS; i++) {
        if (pthread_mutex_init(&server->rooms[i].room_mutex, NULL) != 0) {
            perror("Failed to initialize room mutex");
            return false;
        }
        server->rooms[i].active = false;
    }
    
    // Open log file
    server->log_file = fopen(LOG_FILE, "a");
    if (!server->log_file) {
        perror("Failed to open log file");
        return false;
    }
    
    // Create server socket
    server->socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server->socket < 0) {
        perror("Failed to create socket");
        return false;
    }
    
    // Set socket options to reuse address
    int opt = 1;
    if (setsockopt(server->socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("Failed to set socket options");
        return false;
    }
    
    // Configure server address
    server->address.sin_family = AF_INET;
    server->address.sin_addr.s_addr = INADDR_ANY;
    server->address.sin_port = htons(port);
    
    // Bind server socket
    if (bind(server->socket, (struct sockaddr*)&server->address, sizeof(server->address)) < 0) {
        perror("Failed to bind socket");
        return false;
    }
    
    // Listen for connections
    if (listen(server->socket, 5) < 0) {
        perror("Failed to listen on socket");
        return false;
    }
    
    // Initialize client slots as available
    for (int i = 0; i < MAX_CLIENTS; i++) {
        server->clients[i].status = CLIENT_DISCONNECTED;
    }
    
    // Setup signal handler for SIGINT
    signal(SIGINT, handle_sigint);
    
    log_message(server, "[INFO] Server initialized and listening on port %d", port);
    printf("[INFO] Server initialized and listening on port %d\n", port);
    
    return true;
}

/**
 * Shut down the server and clean up resources
 */
void shutdown_server(Server *server) {
    // Close all client connections
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (server->clients[i].status == CLIENT_CONNECTED) {
            close(server->clients[i].socket);
        }
    }
    
    // Close server socket
    close(server->socket);
    
    // Clean up synchronization primitives
    pthread_mutex_destroy(&server->clients_mutex);
    pthread_mutex_destroy(&server->rooms_mutex);
    pthread_mutex_destroy(&server->queue_mutex);
    sem_destroy(&server->upload_semaphore);
    
    // Clean up room mutexes
    for (int i = 0; i < MAX_ROOMS; i++) {
        if (server->rooms[i].active) {
            pthread_mutex_destroy(&server->rooms[i].room_mutex);
        }
    }
    
    // Close log file
    if (server->log_file) {
        fflush(server->log_file);
        fclose(server->log_file);
    }
    
    printf("[INFO] Server shut down\n");
}

/**
 * Main client handler thread function
 */
void *handle_client(void *arg) {
    Client *client = (Client*)arg;
    Server *server_ptr = &server; // Using global server instance
    char buffer[MAX_COMMAND_LEN];
    ssize_t bytes_read;
    
    // Authenticate client
    if (!authenticate_client(client)) {
        close(client->socket);
        client->status = CLIENT_DISCONNECTED;
        pthread_exit(NULL);
    }
    
    // Welcome message
    sprintf(buffer, "[Server]: Welcome, %s! You are now connected.", client->username);
    notify_client(client, buffer);
    
    log_client_action(server_ptr, client->username, "connected to the server");
    printf("[CONNECT] New client connected: %s from %s\n", 
           client->username, inet_ntoa(client->address.sin_addr));
    
    // Initialize file transfer flag
    client->in_file_transfer = false;

    // Main command processing loop
    while (client->status == CLIENT_CONNECTED && server_ptr->running) {
        memset(buffer, 0, MAX_COMMAND_LEN);
        bytes_read = recv(client->socket, buffer, MAX_COMMAND_LEN - 1, 0);
        
        if (bytes_read <= 0) {
            // Client disconnected or error
            if (bytes_read == 0) {
                log_client_action(server_ptr, client->username, "disconnected (connection closed)");
                printf("[DISCONNECT] Client %s disconnected.\n", client->username);
            } else {
                log_client_action(server_ptr, client->username, "lost connection (recv error)");
                printf("[ERROR] Recv failed for client %s: %s\n", client->username, strerror(errno));
            }
            break;
        }
        
        buffer[bytes_read] = '\0'; // Null-terminate what was read
        
        // If client is in file transfer mode, data is not commands.
        if (client->in_file_transfer) {
            continue;
        }
        
        // Trim trailing newline and carriage return characters from the potential command string
        buffer[strcspn(buffer, "\r\n")] = 0;
        
        // If the buffer is empty after trimming (e.g., it was just a newline or whitespace)
        if (buffer[0] == '\0') {
            continue;
        }
        
        // Process command
        char args[MAX_COMMAND_LEN] = {0};
        CommandType cmd_type = parse_command(buffer, args);
        
        // Don't process commands if in file transfer mode
        if (!client->in_file_transfer) {
            switch (cmd_type) {
                case CMD_JOIN:
                    handle_join_command(server_ptr, client, args);
                    break;
                case CMD_LEAVE:
                    handle_leave_command(server_ptr, client);
                    break;
                case CMD_BROADCAST:
                    handle_broadcast_command(server_ptr, client, args);
                    break;
                case CMD_WHISPER:
                    handle_whisper_command(server_ptr, client, args);
                    break;
                case CMD_SENDFILE:
                    handle_sendfile_command(server_ptr, client, args);
                    break;
                case CMD_EXIT:
                    handle_exit_command(server_ptr, client);
                    break;
                case CMD_INVALID:
                    if (!client->in_file_transfer) {
                        if (strlen(buffer) > 2) {
                            notify_client(client, "[Server]: Invalid command. Type /help for available commands.");
                        } else {
                            log_message(server_ptr, "[INFO] Suppressed 'Invalid command' notification to %s for short input: \"%s\"", client->username, buffer);
                        }
                    }
                    break;
            }
        }
    }
    
    // Client disconnected or server shutting down
    disconnect_client(server_ptr, client);
    pthread_exit(NULL);
}
