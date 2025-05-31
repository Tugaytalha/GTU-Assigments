/**
 * chatserver.c - Main server implementation for the multi-threaded chat and file server
 * CSE 344 - System Programming
 */

#include "chatserver.h"
#include "chatserver_client.h"
#include "chatserver_room.h"
#include "chatserver_commands.h"
#include "chatserver_files.h"
#include "chatserver_log.h"

/* Global server state */
int server_socket = -1;
int server_running = 0;
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t rooms_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t upload_queue_mutex = PTHREAD_MUTEX_INITIALIZER;
sem_t upload_queue_semaphore;
Client* clients[MAX_CLIENTS] = {NULL};
Room* rooms[MAX_ROOMS] = {NULL};
FileTransfer* upload_queue[MAX_UPLOAD_QUEUE] = {NULL};
int upload_queue_size = 0;
FILE* log_file = NULL;

/**
 * Signal handler for graceful shutdown
 */
void handle_signal(int sig) {
    if (sig == SIGINT) {
        log_message("[SHUTDOWN] SIGINT received. Shutting down server...");
        shutdown_server();
    }
}

/**
 * Initialize the server on the specified port
 */
int initialize_server(int port) {
    struct sockaddr_in server_addr;
    int opt = 1;
    
    /* Create socket */
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("socket creation failed");
        return -1;
    }
    
    /* Set socket options */
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt failed");
        close(server_socket);
        return -1;
    }
    
    /* Configure server address */
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);
    
    /* Bind socket to port */
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind failed");
        close(server_socket);
        return -1;
    }
    
    /* Start listening for connections */
    if (listen(server_socket, MAX_CLIENTS) < 0) {
        perror("listen failed");
        close(server_socket);
        return -1;
    }
    
    /* Create uploads directory if it doesn't exist */
    if (mkdir(UPLOAD_DIR, 0777) < 0 && errno != EEXIST) {
        perror("Failed to create uploads directory");
        close(server_socket);
        return -1;
    }
    
    /* Initialize upload queue semaphore */
    if (sem_init(&upload_queue_semaphore, 0, MAX_UPLOAD_QUEUE) < 0) {
        perror("sem_init failed");
        close(server_socket);
        return -1;
    }
    
    /* Initialize log file */
    log_file = fopen(LOG_FILENAME, "w");
    if (!log_file) {
        perror("Failed to open log file");
        close(server_socket);
        sem_destroy(&upload_queue_semaphore);
        return -1;
    }
    
    /* Register signal handler */
    signal(SIGINT, handle_signal);
    
    server_running = 1;
    return 0;
}

/**
 * Cleanup server resources
 */
void cleanup_server(void) {
    int i;
    
    /* Close server socket */
    if (server_socket >= 0) {
        close(server_socket);
        server_socket = -1;
    }
    
    /* Clean up clients */
    pthread_mutex_lock(&clients_mutex);
    for (i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i]) {
            disconnect_client(clients[i]);
            clients[i] = NULL;
        }
    }
    pthread_mutex_unlock(&clients_mutex);
    
    /* Clean up rooms */
    pthread_mutex_lock(&rooms_mutex);
    for (i = 0; i < MAX_ROOMS; i++) {
        if (rooms[i]) {
            free_room(rooms[i]);
            rooms[i] = NULL;
        }
    }
    pthread_mutex_unlock(&rooms_mutex);
    
    /* Clean up upload queue */
    pthread_mutex_lock(&upload_queue_mutex);
    for (i = 0; i < MAX_UPLOAD_QUEUE; i++) {
        if (upload_queue[i]) {
            free(upload_queue[i]);
            upload_queue[i] = NULL;
        }
    }
    upload_queue_size = 0;
    pthread_mutex_unlock(&upload_queue_mutex);
    
    /* Destroy synchronization primitives */
    pthread_mutex_destroy(&clients_mutex);
    pthread_mutex_destroy(&rooms_mutex);
    pthread_mutex_destroy(&log_mutex);
    pthread_mutex_destroy(&upload_queue_mutex);
    sem_destroy(&upload_queue_semaphore);
    
    /* Close log file */
    if (log_file) {
        fclose(log_file);
        log_file = NULL;
    }
}

/**
 * Accept and handle incoming client connections
 */
void* accept_connections(void* arg) {
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    int client_socket;
    pthread_t client_thread;
    
    log_message("[SERVER] Accepting connections on port %d", *(int*)arg);
    
    while (server_running) {
        /* Accept new connection */
        client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &addr_len);
        if (client_socket < 0) {
            if (server_running) {
                perror("accept failed");
            }
            continue;
        }
        
        /* Create client structure and assign thread */
        Client* new_client = create_client(client_socket, client_addr);
        if (!new_client) {
            close(client_socket);
            continue;
        }
        
        /* Start client handler thread */
        if (pthread_create(&client_thread, NULL, handle_client, (void*)new_client) != 0) {
            perror("pthread_create failed");
            free_client(new_client);
            continue;
        }
        
        /* Detach thread to allow it to clean up on exit */
        pthread_detach(client_thread);
    }
    
    return NULL;
}

/**
 * Initiate server shutdown
 */
void shutdown_server(void) {
    int i;
    char shutdown_message[] = "SERVER_SHUTDOWN";
    
    /* Set running flag to false to stop accept loop */
    server_running = 0;
    
    log_message("[SHUTDOWN] Disconnecting %d clients, saving logs.", get_connected_client_count());
    
    /* Notify all clients about shutdown */
    pthread_mutex_lock(&clients_mutex);
    for (i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i] && clients[i]->status == CLIENT_CONNECTED) {
            send(clients[i]->socket, shutdown_message, strlen(shutdown_message), 0);
        }
    }
    pthread_mutex_unlock(&clients_mutex);
    
    /* Close server socket to stop accept() */
    if (server_socket >= 0) {
        close(server_socket);
        server_socket = -1;
    }
}

/**
 * Get count of connected clients
 */
int get_connected_client_count(void) {
    int i, count = 0;
    
    pthread_mutex_lock(&clients_mutex);
    for (i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i] && clients[i]->status == CLIENT_CONNECTED) {
            count++;
        }
    }
    pthread_mutex_unlock(&clients_mutex);
    
    return count;
}

/**
 * Main entry point
 */
int main(int argc, char* argv[]) {
    int port;
    pthread_t accept_thread;
    
    /* Check command line arguments */
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        return EXIT_FAILURE;
    }
    
    port = atoi(argv[1]);
    if (port <= 0 || port > 65535) {
        fprintf(stderr, "Invalid port number. Must be between 1-65535.\n");
        return EXIT_FAILURE;
    }
    
    /* Initialize server */
    if (initialize_server(port) < 0) {
        fprintf(stderr, "Failed to initialize server.\n");
        return EXIT_FAILURE;
    }
    
    printf("Chat server started on port %d\n", port);
    log_message("[SERVER] Started on port %d", port);
    
    /* Start accept thread */
    if (pthread_create(&accept_thread, NULL, accept_connections, &port) != 0) {
        perror("Failed to create accept thread");
        cleanup_server();
        return EXIT_FAILURE;
    }
    
    /* Wait for accept thread to finish (on shutdown) */
    pthread_join(accept_thread, NULL);
    
    /* Clean up resources */
    cleanup_server();
    
    printf("Server shutdown complete.\n");
    return EXIT_SUCCESS;
}
