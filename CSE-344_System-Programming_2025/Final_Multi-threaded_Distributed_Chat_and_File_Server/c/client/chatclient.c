/**
 * Multi-threaded Distributed Chat and File Server
 * Client implementation
 */

#include "chatclient.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdbool.h>
#include <stdarg.h>

// Global client instance for signal handling
Client client;

/**
 * Signal handler for SIGINT (Ctrl+C)
 */
void handle_sigint(int sig) {
    printf("\nDisconnecting from server...\n");
    client.exit_flag = true;
    disconnect_from_server(&client);
    exit(0);
}

/**
 * Initialize client
 */
bool initialize_client(Client *client, const char *server_ip, int port) {
    // Initialize client structure
    memset(client, 0, sizeof(Client));
    client->connected = false;
    client->exit_flag = false;
    
    // Create socket
    client->socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client->socket < 0) {
        perror("Failed to create socket");
        return false;
    }
    
    // Configure server address
    client->server_address.sin_family = AF_INET;
    client->server_address.sin_port = htons(port);
    
    if (inet_pton(AF_INET, server_ip, &client->server_address.sin_addr) <= 0) {
        perror("Invalid address");
        close(client->socket);
        return false;
    }
    
    // Set up signal handler
    signal(SIGINT, handle_sigint);
    
    // Create uploads directory
    create_uploads_directory();
    
    return true;
}

/**
 * Connect to the server
 */
bool connect_to_server(Client *client) {
    // Connect to server
    if (connect(client->socket, (struct sockaddr*)&client->server_address, sizeof(client->server_address)) < 0) {
        perror("Connection failed");
        return false;
    }
    
    client->connected = true;
    
    // Start receiver thread
    if (pthread_create(&client->receiver_thread, NULL, receive_messages, client) != 0) {
        perror("Failed to create receiver thread");
        client->connected = false;
        close(client->socket);
        return false;
    }
    
    return true;
}

/**
 * Disconnect from the server
 */
void disconnect_from_server(Client *client) {
    if (client->connected) {
        // Send exit command if not already exiting
        if (!client->exit_flag) {
            send_command(client, "/exit");
        }
        
        // Close socket
        close(client->socket);
        client->connected = false;
        
        // Wait for receiver thread to terminate
        pthread_join(client->receiver_thread, NULL);
    }
}

/**
 * Process user input and send commands to server
 */
void process_user_input(Client *client) {
    char input[MAX_COMMAND_LEN];
    
    while (client->connected && !client->exit_flag) {
        printf("> ");
        fflush(stdout);
        
        if (fgets(input, MAX_COMMAND_LEN, stdin) == NULL) {
            // EOF or error
            break;
        }
        
        // Remove trailing newline
        size_t len = strlen(input);
        if (len > 0 && input[len - 1] == '\n') {
            input[len - 1] = '\0';
        }
        
        // Skip empty input
        if (strlen(input) == 0) {
            continue;
        }
        
        // Handle /help command locally
        if (strcmp(input, "/help") == 0) {
            print_help();
            continue;
        }
        
        // Handle /sendfile command separately
        if (strncmp(input, "/sendfile ", 10) == 0) {
            char filename[MAX_FILE_NAME_LEN] = {0};
            char recipient[MAX_USERNAME_LEN + 1] = {0};
            
            // Parse filename and recipient
            sscanf(input + 10, "%s %s", filename, recipient);
            
            if (strlen(filename) == 0 || strlen(recipient) == 0) {
                print_message("Error", "Invalid command format. Use: /sendfile <filename> <username>", COLOR_RED);
                continue;
            }
            
            handle_file_send(client, filename, recipient);
            continue;
        }
        
        // Send command to server
        if (!send_command(client, input)) {
            print_message("Error", "Failed to send command to server", COLOR_RED);
            break;
        }
        
        // Check for exit command
        if (strcmp(input, "/exit") == 0) {
            client->exit_flag = true;
            break;
        }
    }
}

/**
 * Send a command to the server
 */
bool send_command(Client *client, const char *command) {
    if (!client->connected) {
        return false;
    }
    
    ssize_t bytes_sent = send(client->socket, command, strlen(command), 0);
    return bytes_sent > 0;
}

/**
 * Handle sending a file to another user
 */
bool handle_file_send(Client *client, const char *filename, const char *recipient) {
    // Open file for reading in binary mode
    FILE *file = fopen(filename, "rb");
    if (!file) {
        if (errno == ENOENT) {
            print_message("Error", "File not found: %s", COLOR_RED, filename);
        } else if (errno == EACCES) {
            print_message("Error", "Permission denied: %s", COLOR_RED, filename);
        } else {
            print_message("Error", "Could not open file: %s (Error: %s)", COLOR_RED, filename, strerror(errno));
        }
        return false;
    }
    
    // Get file size
    fseek(file, 0, SEEK_END);
    size_t filesize = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    // Check file size
    if (filesize == 0) {
        print_message("Error", "File is empty: %s", COLOR_RED, filename);
        fclose(file);
        return false;
    }
    if (filesize > MAX_FILE_SIZE) {
        print_message("Error", "File too large: %s (Size: %zu bytes, Max: %d bytes)", 
                     COLOR_RED, filename, filesize, MAX_FILE_SIZE);
        fclose(file);
        return false;
    }
    
    // Send sendfile command
    char command[MAX_COMMAND_LEN];
    sprintf(command, "/sendfile %s %s", filename, recipient);
    
    if (!send_command(client, command)) {
        print_message("Error", "Failed to send command to server - connection error", COLOR_RED);
        fclose(file);
        return false;
    }
    
    // Wait for server to request file size
    // The server will respond and request the file size, which is handled in receive_messages()
    
    // Send file size
    char size_str[32];
    sprintf(size_str, "%zu", filesize);
    send(client->socket, size_str, strlen(size_str), 0);
    
    // Server will send a message confirming file is queued
    // Then wait for server to request file data
    
    // Read file data and send it
    char buffer[FILE_BUFFER_SIZE];
    size_t bytes_read;
    size_t total_sent = 0;
    
    while ((bytes_read = fread(buffer, 1, FILE_BUFFER_SIZE, file)) > 0) {
        ssize_t bytes_sent = send(client->socket, buffer, bytes_read, 0);
        
        if (bytes_sent <= 0) {
            print_message("Error", "Failed to send file data - connection error", COLOR_RED);
            fclose(file);
            return false;
        }
        
        total_sent += bytes_sent;
        
        // Display progress
        float progress = (float)total_sent / filesize * 100;
        printf("\rUploading: %.1f%%", progress);
        fflush(stdout);
    }
    
    printf("\n");
    fclose(file);
    
    print_message("Info", "File upload completed", COLOR_GREEN);
    return true;
}

/**
 * Thread function to receive and handle messages from the server
 */
void *receive_messages(void *arg) {
    Client *client = (Client*)arg;
    char buffer[RECV_BUFFER_SIZE];
    ssize_t bytes_read;
    
    while (client->connected && !client->exit_flag) {
        memset(buffer, 0, RECV_BUFFER_SIZE);
        bytes_read = recv(client->socket, buffer, RECV_BUFFER_SIZE - 1, 0);
        
        if (bytes_read <= 0) {
            // Connection closed or error
            if (bytes_read == 0) {
                print_message("Server", "Connection closed by server", COLOR_RED);
            } else {
                print_message("Error", "Failed to receive data from server", COLOR_RED);
            }
            
            client->connected = false;
            break;
        }
        
        buffer[bytes_read] = '\0';
        
        // Check if this is a file transfer
        if (strncmp(buffer, "FILE:", 5) == 0) {
            // Extract filename and size
            char *filename_start = buffer + 5;
            char *size_start = strchr(filename_start, ':');
            
            if (size_start) {
                *size_start = '\0';
                size_start++;
                
                size_t filesize = atol(size_start);
                handle_file_receive(client, filename_start, filesize);
            }
        }
        // Handle server prompts
        else if (strcmp(buffer, "Enter your username: ") == 0) {
            printf("%s", buffer);
            fflush(stdout);
        }
        // Handle server response messages
        else if (strncmp(buffer, "[Server]:", 9) == 0) {
            char *message = buffer + 9;
            
            // Color code messages based on content
            if (strstr(message, "Error") || strstr(message, "error") || strstr(message, "failed") || strstr(message, "Failed")) {
                print_message("Server", message, COLOR_RED);
            } else if (strstr(message, "joined") || strstr(message, "success")) {
                print_message("Server", message, COLOR_GREEN);
            } else if (strstr(message, "whisper") || strstr(message, "Whisper")) {
                print_message("Server", message, COLOR_CYAN);
            } else if (strstr(message, "file") || strstr(message, "File")) {
                print_message("Server", message, COLOR_YELLOW);
            } else {
                print_message("Server", message, COLOR_BLUE);
            }
        }
        // Handle whisper messages
        else if (strncmp(buffer, "[Whisper", 8) == 0) {
            print_message("", buffer, COLOR_CYAN);
        }
        // Handle regular user messages
        else if (buffer[0] == '[') {
            print_message("", buffer, COLOR_RESET);
        }
        // Handle any other messages
        else {
            printf("%s\n", buffer);
        }
    }
    
    pthread_exit(NULL);
}

/**
 * Handle receiving a file from another user
 */
void handle_file_receive(Client *client, const char *filename, size_t filesize) {
    // Extract only the base filename, not any path components for security
    const char *base_filename = strrchr(filename, '/');
    if (base_filename) {
        base_filename++; // Skip the slash
    } else {
        base_filename = strrchr(filename, '\\');
        if (base_filename) {
            base_filename++; // Skip the backslash
        } else {
            base_filename = filename; // No path separator found
        }
    }
    
    // Create the full path for saving the file
    char filepath[MAX_FILE_NAME_LEN + sizeof(UPLOADS_DIR) + 2];
    snprintf(filepath, sizeof(filepath), "%s/%s", UPLOADS_DIR, base_filename);
    
    // Ensure uploads directory exists
    create_uploads_directory();
    
    // Open file for writing in binary mode
    FILE *file = fopen(filepath, "wb");
    if (!file) {
        print_message("Error", "Could not create file for writing. Path: %s", COLOR_RED, filepath);
        
        // We need to consume the file data even if we can't save it
        char discard_buffer[FILE_BUFFER_SIZE];
        size_t total_discarded = 0;
        while (total_discarded < filesize) {
            size_t to_read = filesize - total_discarded;
            if (to_read > FILE_BUFFER_SIZE) to_read = FILE_BUFFER_SIZE;
            
            ssize_t bytes_read = recv(client->socket, discard_buffer, to_read, 0);
            if (bytes_read <= 0) break;
            total_discarded += bytes_read;
        }
        return;
    }
    
    print_message("File", "Receiving file '%s'... Saving to: %s", COLOR_YELLOW, base_filename, filepath);
    
    // Receive file data
    char buffer[FILE_BUFFER_SIZE];
    size_t total_received = 0;
    ssize_t bytes_read;
    
    while (total_received < filesize) {
        size_t to_read = filesize - total_received;
        if (to_read > FILE_BUFFER_SIZE) {
            to_read = FILE_BUFFER_SIZE;
        }
        
        bytes_read = recv(client->socket, buffer, to_read, 0);
        
        if (bytes_read <= 0) {
            print_message("Error", "Failed to receive file data - connection error", COLOR_RED);
            fclose(file);
            return;
        }
        
        size_t bytes_written = fwrite(buffer, 1, bytes_read, file);
        if (bytes_written != bytes_read) {
            print_message("Error", "Failed to write data to file - disk error", COLOR_RED);
            fclose(file);
            return;
        }
        
        total_received += bytes_read;
        
        // Display progress
        float progress = (float)total_received / filesize * 100;
        printf("\rDownloading: %.1f%%", progress);
        fflush(stdout);
    }
    
    printf("\n");
    fflush(file); // Make sure all data is written
    fclose(file);
    
    // Verify file was saved
    struct stat st;
    if (stat(filepath, &st) == 0 && st.st_size == filesize) {
        print_message("Success", "File '%s' received and saved to: %s", COLOR_GREEN, base_filename, filepath);
    } else {
        print_message("Warning", "File may not have been saved correctly. Check: %s", COLOR_YELLOW, filepath);
    }
}

/**
 * Print welcome message and instructions
 */
void print_welcome_message() {
    printf("\n");
    printf(COLOR_CYAN "====================================\n" COLOR_RESET);
    printf(COLOR_CYAN "  Multi-threaded Chat Client\n" COLOR_RESET);
    printf(COLOR_CYAN "====================================\n" COLOR_RESET);
    printf("\n");
    printf("Welcome to the chat client!\n");
    printf("Type " COLOR_GREEN "/help" COLOR_RESET " to see available commands.\n");
    printf("\n");
}

/**
 * Print help message with available commands
 */
void print_help() {
    printf("\n");
    printf(COLOR_CYAN "Available Commands:\n" COLOR_RESET);
    printf(COLOR_GREEN "  /join <room_name>" COLOR_RESET " - Join or create a room\n");
    printf(COLOR_GREEN "  /leave" COLOR_RESET " - Leave the current room\n");
    printf(COLOR_GREEN "  /broadcast <message>" COLOR_RESET " - Send message to everyone in the room\n");
    printf(COLOR_GREEN "  /whisper <username> <message>" COLOR_RESET " - Send private message\n");
    printf(COLOR_GREEN "  /sendfile <filename> <username>" COLOR_RESET " - Send file to user\n");
    printf(COLOR_GREEN "  /exit" COLOR_RESET " - Disconnect from the server\n");
    printf(COLOR_GREEN "  /help" COLOR_RESET " - Show this help message\n");
    printf("\n");
}

/**
 * Print a formatted message with color
 */
void print_message(const char *prefix, const char *format, const char *color, ...) {
    char message[MAX_MESSAGE_LEN];
    va_list args;
    va_start(args, color);
    vsnprintf(message, sizeof(message), format, args);
    va_end(args);

    if (strlen(prefix) > 0) {
        printf("%s%s:%s %s\n", color, prefix, COLOR_RESET, message);
    } else {
        printf("%s%s%s\n", color, message, COLOR_RESET);
    }
}

/**
 * Create uploads directory if it doesn't exist
 */
void create_uploads_directory() {
    struct stat st = {0};
    
    if (stat(UPLOADS_DIR, &st) == -1) {
        // Directory doesn't exist, create it
        #ifdef _WIN32
        mkdir(UPLOADS_DIR);
        #else
        mkdir(UPLOADS_DIR, 0700);
        #endif
        printf("Created uploads directory: %s\n", UPLOADS_DIR);
    }
}

/**
 * Main function
 */
int main(int argc, char *argv[]) {
    // Check command line arguments
    if (argc != 3) {
        printf("Usage: %s <server_ip> <port>\n", argv[0]);
        return 1;
    }
    
    // Parse port number
    int port = atoi(argv[2]);
    if (port <= 0 || port > 65535) {
        printf("Error: Invalid port number. Must be between 1 and 65535.\n");
        return 1;
    }
    
    // Initialize client
    if (!initialize_client(&client, argv[1], port)) {
        printf("Error: Failed to initialize client.\n");
        return 1;
    }
    
    // Print welcome message
    print_welcome_message();
    
    // Connect to server
    printf("Connecting to server %s:%d...\n", argv[1], port);
    if (!connect_to_server(&client)) {
        printf("Error: Failed to connect to server.\n");
        return 1;
    }
    
    // Process user input
    process_user_input(&client);
    
    // Disconnect from server
    disconnect_from_server(&client);
    
    printf("Disconnected from server.\n");
    
    return 0;
}
