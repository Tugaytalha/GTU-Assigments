/**
 * chatclient.c - Main client implementation
 * CSE 344 - System Programming
 */

#include "chatclient.h"

/* Global variables */
int client_socket = -1;
int client_running = 0;
ClientState client_state = STATE_DISCONNECTED;
char username[MAX_USERNAME_LENGTH + 1] = {0};
char current_room[MAX_MESSAGE_LENGTH] = {0};
pthread_mutex_t client_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_t receive_thread;
FileReceiveState file_state = {0};

/**
 * Signal handler
 */
void handle_signal(int sig) {
    if (sig == SIGINT) {
        printf("\nDisconnecting from server...\n");
        client_running = 0;
        
        /* Send exit command if connected */
        if (client_state >= STATE_CONNECTED) {
            send_to_server("/exit");
        }
        
        /* Close socket and clean up */
        disconnect_from_server();
        cleanup_client();
        
        exit(EXIT_SUCCESS);
    }
}

/**
 * Connect to the chat server
 */
int connect_to_server(const char* ip, int port) {
    struct sockaddr_in server_addr;
    
    /* Create socket */
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0) {
        perror("Socket creation failed");
        return -1;
    }
    
    /* Configure server address */
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    
    /* Convert IP address from text to binary form */
    if (inet_pton(AF_INET, ip, &server_addr.sin_addr) <= 0) {
        perror("Invalid IP address");
        close(client_socket);
        client_socket = -1;
        return -1;
    }
    
    /* Set client state to connecting */
    client_state = STATE_CONNECTING;
    
    /* Connect to server */
    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        close(client_socket);
        client_socket = -1;
        client_state = STATE_DISCONNECTED;
        return -1;
    }
    
    /* Create downloads directory if it doesn't exist */
    if (mkdir(DOWNLOAD_DIR, 0777) < 0 && errno != EEXIST) {
        perror("Failed to create downloads directory");
    }
    
    /* Set client state to connected */
    client_state = STATE_CONNECTED;
    client_running = 1;
    
    return 0;
}

/**
 * Disconnect from the server
 */
void disconnect_from_server(void) {
    pthread_mutex_lock(&client_mutex);
    
    if (client_socket >= 0) {
        close(client_socket);
        client_socket = -1;
    }
    
    client_state = STATE_DISCONNECTED;
    client_running = 0;
    
    pthread_mutex_unlock(&client_mutex);
}

/**
 * Clean up client resources
 */
void cleanup_client(void) {
    /* Close file if open */
    if (file_state.file) {
        fclose(file_state.file);
        file_state.file = NULL;
        file_state.active = 0;
    }
    
    /* Close socket if open */
    if (client_socket >= 0) {
        close(client_socket);
        client_socket = -1;
    }
    
    /* Destroy mutex */
    pthread_mutex_destroy(&client_mutex);
}

/**
 * Send message to server
 */
void send_to_server(const char* message) {
    pthread_mutex_lock(&client_mutex);
    
    if (client_socket >= 0 && client_state >= STATE_CONNECTED) {
        send(client_socket, message, strlen(message), 0);
    }
    
    pthread_mutex_unlock(&client_mutex);
}

/**
 * Thread function to receive messages from server
 */
void* receive_messages(void* arg __attribute__((unused))) {
    char buffer[CLIENT_BUFFER_SIZE];
    int bytes_read;
    
    while (client_running) {
        memset(buffer, 0, sizeof(buffer));
        
        pthread_mutex_lock(&client_mutex);
        if (client_socket < 0) {
            pthread_mutex_unlock(&client_mutex);
            break;
        }
        pthread_mutex_unlock(&client_mutex);
        
        /* Receive data from server */
        bytes_read = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
        
        if (bytes_read <= 0) {
            /* Connection closed or error */
            if (client_running) {
                printf(COLOR_RED "\nDisconnected from server.\n" COLOR_RESET);
                disconnect_from_server();
            }
            break;
        }
        
        buffer[bytes_read] = '\0';
        
        /* Check if this is a file transfer command */
        if (strncmp(buffer, FILE_COMMAND_PREFIX, strlen(FILE_COMMAND_PREFIX)) == 0) {
            size_t header_size = 0;
            sscanf(buffer + strlen(FILE_COMMAND_PREFIX), "%zu", &header_size);
            handle_file_header(header_size);
            continue;
        }
        
        /* Check for server shutdown message */
        if (strcmp(buffer, "SERVER_SHUTDOWN") == 0) {
            printf(COLOR_RED "\nServer is shutting down. You will be disconnected.\n" COLOR_RESET);
            disconnect_from_server();
            break;
        }
        
        /* Regular message, print it */
        printf("%s\n", buffer);
        
        /* Check for login state transition */
        if (client_state == STATE_CONNECTED && 
            strstr(buffer, "Login successful") != NULL) {
            client_state = STATE_READY;
        }
    }
    
    return NULL;
}

/**
 * Handle file header received from server
 */
void handle_file_header(size_t header_size) {
    char sender[MAX_USERNAME_LENGTH + 1];
    char filename[MAX_MESSAGE_LENGTH];
    size_t file_size;
    char filepath[MAX_MESSAGE_LENGTH * 2];
    /* Buffer for messages - not used in current implementation */
    
    /* Receive file header */
    char* header_buffer = (char*)malloc(header_size);
    if (!header_buffer) {
        printf(COLOR_RED "Memory allocation error during file transfer.\n" COLOR_RESET);
        return;
    }
    
    ssize_t bytes_read = recv(client_socket, header_buffer, header_size, 0);
    if ((size_t)bytes_read != header_size) {
        printf(COLOR_RED "Error receiving file header.\n" COLOR_RESET);
        free(header_buffer);
        return;
    }
    
    /* Parse header information */
    memcpy(&file_size, header_buffer, sizeof(file_size));
    memcpy(filename, header_buffer + sizeof(file_size), MAX_MESSAGE_LENGTH);
    memcpy(sender, header_buffer + sizeof(file_size) + MAX_MESSAGE_LENGTH, MAX_USERNAME_LENGTH + 1);
    
    free(header_buffer);
    
    /* Check file size */
    if (file_size > MAX_FILE_SIZE) {
        printf(COLOR_RED "File too large: %zu bytes (max: %d).\n" COLOR_RESET, file_size, MAX_FILE_SIZE);
        return;
    }
    
    printf(COLOR_YELLOW "\nReceiving file '%s' from %s (%zu bytes)...\n" COLOR_RESET, 
           filename, sender, file_size);
           
    /* Check for duplicate filename and rename if necessary */
    snprintf(filepath, sizeof(filepath), "%s/%s", DOWNLOAD_DIR, filename);
    
    /* If file already exists, add a numeric suffix */
    int suffix = 1;
    char new_filepath[MAX_MESSAGE_LENGTH * 2];
    strcpy(new_filepath, filepath);
    
    while (access(new_filepath, F_OK) == 0) {
        /* File exists, try a new name */
        char* dot = strrchr(filename, '.');
        if (dot) {
            /* File has extension */
            char basename[MAX_MESSAGE_LENGTH];
            char extension[MAX_MESSAGE_LENGTH];
            
            /* Split into basename and extension */
            size_t basename_len = dot - filename;
            strncpy(basename, filename, basename_len);
            basename[basename_len] = '\0';
            strcpy(extension, dot);
            
            /* Create new filename with suffix */
            char basename_with_suffix[MAX_MESSAGE_LENGTH];
            snprintf(basename_with_suffix, sizeof(basename_with_suffix), "%s_%d", basename, suffix);
            snprintf(new_filepath, sizeof(new_filepath), "%s/%s%s", 
                     DOWNLOAD_DIR, basename_with_suffix, extension);
        } else {
            /* File has no extension */
            snprintf(new_filepath, sizeof(new_filepath), "%s/%s_%d", 
                     DOWNLOAD_DIR, filename, suffix);
        }
        
        suffix++;
    }
    
    /* Open file for writing */
    FILE* file = fopen(new_filepath, "wb");
    if (!file) {
        printf(COLOR_RED "Failed to open file for writing: %s\n" COLOR_RESET, new_filepath);
        return;
    }
    
    /* Set up file receiving state */
    file_state.active = 1;
    file_state.file = file;
    strncpy(file_state.filename, filename, MAX_MESSAGE_LENGTH - 1);
    strncpy(file_state.sender, sender, MAX_USERNAME_LENGTH);
    file_state.file_size = file_size;
    file_state.received_size = 0;
    
    /* Receive file data */
    char data_buffer[CLIENT_BUFFER_SIZE];
    size_t remaining = file_size;
    
    while (remaining > 0 && client_running) {
        size_t to_read = remaining > sizeof(data_buffer) ? sizeof(data_buffer) : remaining;
        bytes_read = recv(client_socket, data_buffer, to_read, 0);
        
        if (bytes_read <= 0) {
            printf(COLOR_RED "\nError receiving file data.\n" COLOR_RESET);
            break;
        }
        
        /* Write to file */
        if (fwrite(data_buffer, 1, (size_t)bytes_read, file) != (size_t)bytes_read) {
            printf(COLOR_RED "\nError writing to file.\n" COLOR_RESET);
            break;
        }
        
        remaining -= bytes_read;
        file_state.received_size += bytes_read;
        
        /* Show progress for large files */
        if (file_size > 1024 * 1024) {
            printf("\rReceiving: %zu/%zu bytes (%.1f%%)", 
                   file_state.received_size, file_size, 
                   (float)file_state.received_size / file_size * 100);
            fflush(stdout);
        }
    }
    
    /* Close file */
    fclose(file);
    file_state.file = NULL;
    file_state.active = 0;
    
    if (file_state.received_size == file_size) {
        /* Successful transfer */
        printf(COLOR_GREEN "\nFile received successfully: %s\n" COLOR_RESET, 
               strrchr(new_filepath, '/') ? strrchr(new_filepath, '/') + 1 : new_filepath);
    } else {
        /* Failed transfer */
        printf(COLOR_RED "\nFile transfer incomplete: %zu/%zu bytes received.\n" COLOR_RESET, 
               file_state.received_size, file_size);
        
        /* Remove incomplete file */
        unlink(new_filepath);
    }
}

/**
 * Process user input
 */
void process_user_input(void) {
    char input[MAX_MESSAGE_LENGTH];
    
    while (client_running) {
        /* Read user input */
        if (fgets(input, sizeof(input), stdin) == NULL) {
            break;
        }
        
        /* Remove trailing newline */
        size_t len = strlen(input);
        if (len > 0 && input[len - 1] == '\n') {
            input[len - 1] = '\0';
        }
        
        /* Skip empty input */
        if (strlen(input) == 0) {
            continue;
        }
        
        /* Process command or send message */
        if (client_state >= STATE_CONNECTED) {
            /* Send to server */
            send_to_server(input);
            
            /* Check for exit command */
            if (strcmp(input, "/exit") == 0) {
                client_running = 0;
                break;
            }
        }
    }
}

/**
 * Print welcome message
 */
void print_welcome_message(void) {
    printf(COLOR_CYAN);
    printf("╔══════════════════════════════════════════════╗\n");
    printf("║           Welcome to Chat Client             ║\n");
    printf("╚══════════════════════════════════════════════╝\n");
    printf(COLOR_RESET);
}

/**
 * Print help message
 */
void print_help(void) {
    printf(COLOR_YELLOW);
    printf("Available commands:\n");
    printf("  /join <room_name>       - Join or create a room\n");
    printf("  /leave                  - Leave the current room\n");
    printf("  /broadcast <message>    - Send message to everyone in the room\n");
    printf("  /whisper <user> <msg>   - Send private message\n");
    printf("  /sendfile <file> <user> - Send file to user\n");
    printf("  /exit                   - Disconnect from server\n");
    printf(COLOR_RESET);
}

/**
 * Main entry point
 */
int main(int argc, char* argv[]) {
    /* Check command line arguments */
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <server_ip> <port>\n", argv[0]);
        return EXIT_FAILURE;
    }
    
    /* Parse port number */
    int port = atoi(argv[2]);
    if (port <= 0 || port > 65535) {
        fprintf(stderr, "Invalid port number. Must be between 1-65535.\n");
        return EXIT_FAILURE;
    }
    
    /* Register signal handler */
    signal(SIGINT, handle_signal);
    
    /* Print welcome message */
    print_welcome_message();
    
    /* Connect to server */
    if (connect_to_server(argv[1], port) < 0) {
        fprintf(stderr, "Failed to connect to server at %s:%d\n", argv[1], port);
        return EXIT_FAILURE;
    }
    
    printf("Connected to server at %s:%d\n", argv[1], port);
    
    /* Start receive thread */
    if (pthread_create(&receive_thread, NULL, receive_messages, NULL) != 0) {
        perror("Failed to create receive thread");
        disconnect_from_server();
        return EXIT_FAILURE;
    }
    
    /* Process user input */
    process_user_input();
    
    /* Wait for receive thread to finish */
    if (receive_thread) {
        pthread_cancel(receive_thread);
        pthread_join(receive_thread, NULL);
    }
    
    /* Clean up */
    cleanup_client();
    
    printf("Disconnected from server.\n");
    return EXIT_SUCCESS;
}
