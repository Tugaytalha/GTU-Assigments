/**
 * Multi-threaded Distributed Chat and File Server
 * Main server entry point
 */

#include "chatserver.h"

/**
 * Main function
 */
int main(int argc, char *argv[]) {
    // Check command line arguments
    if (argc != 2) {
        printf("Usage: %s <port>\n", argv[0]);
        return 1;
    }
    
    // Parse port number
    int port = atoi(argv[1]);
    if (port <= 0 || port > 65535) {
        printf("Error: Invalid port number. Must be between 1 and 65535.\n");
        return 1;
    }
    
    // Initialize server - use the global server variable
    if (!initialize_server(&server, port)) {
        printf("Error: Failed to initialize server.\n");
        return 1;
    }
    
    printf("[INFO] Server listening on port %d...\n", port);
    
    // Accept client connections
    struct sockaddr_in client_address;
    socklen_t address_length = sizeof(client_address);
    int client_socket;
    
    // Main server loop
    while (server.running) {
        client_socket = accept(server.socket, (struct sockaddr*)&client_address, &address_length);
        
        if (!server.running) {
            // Server is shutting down, stop accepting connections
            break;
        }
        
        if (client_socket < 0) {
            // Error accepting connection
            if (errno == EINTR) {
                // Interrupted by signal, check if server is still running
                continue;
            }
            
            perror("Accept failed");
            continue;
        }
        
        // Find an available client slot
        int slot = -1;
        pthread_mutex_lock(&server.clients_mutex);
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (server.clients[i].status == CLIENT_DISCONNECTED) {
                slot = i;
                break;
            }
        }
        pthread_mutex_unlock(&server.clients_mutex);
        
        if (slot == -1) {
            // No available slots, reject connection
            const char *msg = "[Server]: Server is full. Please try again later.\n";
            send(client_socket, msg, strlen(msg), 0);
            close(client_socket);
            log_message(&server, "[REJECTED] Connection rejected - server full");
            continue;
        }
        
        // Initialize client structure
        pthread_mutex_lock(&server.clients_mutex);
        server.clients[slot].socket = client_socket;
        server.clients[slot].address = client_address;
        server.clients[slot].status = CLIENT_AUTHENTICATING;
        server.clients[slot].current_room[0] = '\0';
        pthread_mutex_unlock(&server.clients_mutex);
        
        // Create a thread to handle the client
        pthread_create(&server.clients[slot].thread, NULL, handle_client, &server.clients[slot]);
        pthread_detach(server.clients[slot].thread);
    }
    
    // Cleanup and shutdown
    shutdown_server(&server);
    
    return 0;
}
