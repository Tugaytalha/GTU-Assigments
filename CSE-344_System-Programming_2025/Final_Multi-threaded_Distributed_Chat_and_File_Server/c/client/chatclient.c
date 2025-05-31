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
    (void)sig;  // Kullanılmayan parametre uyarısını engellemek için
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
                print_message("Error", "Invalid command format. Use: /sendfile <filename> <username> T", COLOR_RED);
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
    
    // Send file size
    char size_str[32];
    sprintf(size_str, "%zu", filesize);
    if (send(client->socket, size_str, strlen(size_str), 0) <= 0) {
        print_message("Error", "Failed to send file size - connection error", COLOR_RED);
        fclose(file);
        return false;
    }
    
    // Wait for server acknowledgment
    char ack_buffer[256]; // Increased buffer size to handle longer messages
    ssize_t ack_bytes = recv(client->socket, ack_buffer, sizeof(ack_buffer) - 1, 0);
    if (ack_bytes <= 0) {
        print_message("Error", "Failed to receive server acknowledgment", COLOR_RED);
        fclose(file);
        return false;
    }
    ack_buffer[ack_bytes] = '\0';
    
    // Check if server indicated the queue is full
    if (strstr(ack_buffer, "queue is full") != NULL) {
        print_message("Info", "File upload queued. Waiting for available slot...", COLOR_CYAN);
        fclose(file); // Close the file for now
        
        // Set socket to non-blocking mode for the wait period
        int flags = fcntl(client->socket, F_GETFL, 0);
        fcntl(client->socket, F_SETFL, flags | O_NONBLOCK);
        
        // Wait for the "slot available" message from server
        char queue_msg[256];
        int consecutive_errors = 0;
        int max_consecutive_errors = 5; // Allow some transient errors
        
        while (1) {
            // Use a timeout to check periodically if client is still connected
            fd_set readfds;
            struct timeval tv;
            FD_ZERO(&readfds);
            FD_SET(client->socket, &readfds);
            tv.tv_sec = 2;  // 2 second timeout
            tv.tv_usec = 0;
            
            int activity = select(client->socket + 1, &readfds, NULL, NULL, &tv);
            
            if (activity < 0) {
                if (errno == EINTR) {
                    // Interrupted by signal, not an error
                    continue;
                }
                print_message("Error", "Select failed while waiting for upload slot: %s", COLOR_RED, strerror(errno));
                fcntl(client->socket, F_SETFL, flags); // Restore original socket flags
                return false;
            }
            else if (activity == 0) {
                // Timeout, just continue waiting
                continue;
            }
            
            // Data is available to read
            if (FD_ISSET(client->socket, &readfds)) {
                memset(queue_msg, 0, sizeof(queue_msg));
                ssize_t bytes = recv(client->socket, queue_msg, sizeof(queue_msg) - 1, 0);
                
                if (bytes < 0) {
                    if (errno == EAGAIN || errno == EWOULDBLOCK) {
                        // No data available yet, not an error
                        continue;
                    }
                    consecutive_errors++;
                    print_message("Warning", "Receive error: %s (retry %d/%d)", COLOR_YELLOW, 
                                 strerror(errno), consecutive_errors, max_consecutive_errors);
                    
                    if (consecutive_errors >= max_consecutive_errors) {
                        print_message("Error", "Too many consecutive errors while waiting for upload slot", COLOR_RED);
                        fcntl(client->socket, F_SETFL, flags); // Restore original socket flags
                        return false;
                    }
                    continue;
                }
                else if (bytes == 0) {
                    print_message("Error", "Server closed connection while waiting for upload slot", COLOR_RED);
                    fcntl(client->socket, F_SETFL, flags); // Restore original socket flags
                    return false;
                }
                
                // Reset error counter on successful receive
                consecutive_errors = 0;
                queue_msg[bytes] = '\0';
                
                // Check if we received the notification that a slot is available
                if (strstr(queue_msg, "slot is now available") != NULL) {
                    print_message("Info", "Upload slot available, resuming file transfer", COLOR_CYAN);
                    
                    // Add a small delay to ensure server is ready
                    usleep(100000); // 100ms delay
                    
                    // Restore blocking mode
                    fcntl(client->socket, F_SETFL, flags);
                    
                    // Wait for the "Send file size" prompt with a timeout
                    fd_set readfds;
                    struct timeval tv;
                    FD_ZERO(&readfds);
                    FD_SET(client->socket, &readfds);
                    tv.tv_sec = 3;  // 3 second timeout
                    tv.tv_usec = 0;
                    
                    int prompt_activity = select(client->socket + 1, &readfds, NULL, NULL, &tv);
                    if (prompt_activity <= 0) {
                        print_message("Error", "Timeout waiting for server prompt", COLOR_RED);
                        return false;
                    }
                    
                    char size_prompt[256];
                    ssize_t prompt_bytes = recv(client->socket, size_prompt, sizeof(size_prompt) - 1, 0);
                    if (prompt_bytes <= 0) {
                        print_message("Error", "Failed to receive size prompt: %s", COLOR_RED, 
                                     (prompt_bytes < 0) ? strerror(errno) : "Connection closed");
                        return false;
                    }
                    size_prompt[prompt_bytes] = '\0';
                    print_message("Debug", "Received prompt: %s", COLOR_BLUE, size_prompt);
                    
                    // Reopen the file
                    file = fopen(filename, "rb");
                    if (!file) {
                        print_message("Error", "Failed to reopen file: %s (%s)", COLOR_RED, filename, strerror(errno));
                        return false;
                    }
                    
                    // Send the file size again
                    char size_str[32];
                    sprintf(size_str, "%zu", filesize);
                    if (send(client->socket, size_str, strlen(size_str), 0) <= 0) {
                        print_message("Error", "Failed to send file size: %s", COLOR_RED, strerror(errno));
                        fclose(file);
                        return false;
                    }
                    
                    // Get acknowledgment for file size
                    ssize_t ack2_bytes = recv(client->socket, ack_buffer, sizeof(ack_buffer) - 1, 0);
                    if (ack2_bytes <= 0) {
                        print_message("Error", "Failed to receive server acknowledgment: %s", COLOR_RED, 
                                     (ack2_bytes < 0) ? strerror(errno) : "Connection closed");
                        fclose(file);
                        return false;
                    }
                    ack_buffer[ack2_bytes] = '\0';
                    
                    // Check the new acknowledgment
                    if (strstr(ack_buffer, "Error") != NULL) {
                        print_message("Error", "Server rejected file transfer: %s", COLOR_RED, ack_buffer);
                        fclose(file);
                        return false;
                    }
                    
                    // Now we can break out of the waiting loop
                    break;
                }
                
                // If we received other messages, print them but keep waiting
                if (strlen(queue_msg) > 0) {
                    print_message("Server", "%s", COLOR_WHITE, queue_msg);
                }
            }
        }
    }
    // Check if server acknowledged with an error
    else if (strstr(ack_buffer, "Error") != NULL) {
        print_message("Error", "Server rejected file transfer: %s", COLOR_RED, ack_buffer);
        fclose(file);
        return false;
    }
    // If we get here, either we got normal acknowledgment or we've already handled the queue and received acknowledgment
    
    // Read file data and send it
    char buffer[FILE_BUFFER_SIZE];
    size_t bytes_read;
    size_t total_sent = 0;
    size_t last_progress = 0;
    
    while ((bytes_read = fread(buffer, 1, FILE_BUFFER_SIZE, file)) > 0) {
        ssize_t bytes_sent = send(client->socket, buffer, bytes_read, 0);
        
        if (bytes_sent <= 0) {
            print_message("Error", "Failed to send file data - connection error", COLOR_RED);
            fclose(file);
            return false;
        }
        
        total_sent += bytes_sent;
        
        // Display progress only when it changes by at least 1%
        size_t current_progress = (total_sent * 100) / filesize;
        if (current_progress > last_progress) {
            printf("\rUploading: %zu%%", current_progress);
            fflush(stdout);
            last_progress = current_progress;
        }
    }
    
    printf("\n");
    fclose(file);
    
    // Wait for final confirmation
    char final_buffer[256];
    ssize_t final_bytes = recv(client->socket, final_buffer, sizeof(final_buffer) - 1, 0);
    if (final_bytes <= 0) {
        print_message("Error", "Failed to receive final confirmation", COLOR_RED);
        return false;
    }
    final_buffer[final_bytes] = '\0';
    
    if (strstr(final_buffer, "successfully") != NULL) {
        print_message("Success", "File upload completed successfully", COLOR_GREEN);
        return true;
    } else {
        print_message("Error", "File upload failed: %s", COLOR_RED, final_buffer);
        return false;
    }
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
            //printf("DEBUG: Received file header: %s\n", buffer); // Debug
            
            // Extract filename and size
            char *filename_start = buffer + 5;
            char *size_start = strchr(filename_start, ':');
            
            if (size_start) {
                *size_start = '\0';
                size_start++;
                
                // Remove newline if present
                char *newline = strchr(size_start, '\n');
                char *remaining_data = NULL;
                size_t remaining_len = 0;
                
                if (newline) {
                    *newline = '\0';
                    remaining_data = newline + 1;
                    remaining_len = bytes_read - (remaining_data - buffer);
                }
                
                char *endptr;
                size_t filesize = strtoull(size_start, &endptr, 10);
                //printf("DEBUG: Parsed filename='%s', filesize=%zu, remaining_len=%zu\n",
                //       filename_start, filesize, remaining_len); // Debug
                
                if (*endptr == '\0' && filesize > 0) {
                    handle_file_receive_with_data(client, filename_start, filesize, remaining_data, remaining_len);
                } else {
                    print_message("Error", "Invalid file size received", COLOR_RED);
                }
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

void handle_file_receive_with_data(Client *client, const char *filename, size_t filesize, 
                                   const char *initial_data, size_t initial_len) {
    //printf("DEBUG: Starting file receive: filename='%s', filesize=%zu, initial_len=%zu\n",
    //       filename, filesize, initial_len); // Debug
    
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
    
    //printf("DEBUG: File will be saved to: %s\n", filepath); // Debug
    
    // Ensure uploads directory exists
    create_uploads_directory();
    
    // Open file for writing in binary mode
    FILE *file = fopen(filepath, "wb");
    if (!file) {
        print_message("Error", "Could not create file for writing. Path: %s, Error: %s", 
                     COLOR_RED, filepath, strerror(errno));
        
        // We need to consume the file data even if we can't save it
        char discard_buffer[FILE_BUFFER_SIZE];
        size_t total_discarded = initial_len;
        
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
    
    // Write initial data if any
    size_t total_received = 0;
    if (initial_data && initial_len > 0) {
        size_t data_to_write = (initial_len > filesize) ? filesize : initial_len;
        size_t bytes_written = fwrite(initial_data, 1, data_to_write, file);
        if (bytes_written != data_to_write) {
            print_message("Error", "Failed to write initial data to file - disk error", COLOR_RED);
            fclose(file);
            return;
        }
        total_received = data_to_write;
        //printf("DEBUG: Wrote %zu bytes of initial data\n", data_to_write); // Debug
    }
    
    // Receive remaining file data
    char buffer[FILE_BUFFER_SIZE];
    ssize_t bytes_read;
    size_t last_progress = 0;
    
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
        
        size_t bytes_written = fwrite(buffer, 1, (size_t)bytes_read, file);
        if (bytes_written != (size_t)bytes_read) {
            print_message("Error", "Failed to write data to file - disk error", COLOR_RED);
            fclose(file);
            return;
        }
        
        total_received += bytes_read;
        
        // Display progress only when it changes by at least 1%
        size_t current_progress = (total_received * 100) / filesize;
        if (current_progress > last_progress) {
            printf("\rDownloading: %zu%%", current_progress);
            fflush(stdout);
            last_progress = current_progress;
        }
    }
    
    printf("\n");
    fflush(file); // Make sure all data is written
    fclose(file);
    
    //printf("DEBUG: File receive completed. Total received: %zu bytes\n", total_received); // Debug
    
    // Verify file was saved
    struct stat st;
    if (stat(filepath, &st) == 0 && (size_t)st.st_size == filesize) {
        print_message("Success", "File '%s' received and saved to: %s", COLOR_GREEN, base_filename, filepath);
    } else {
        if (stat(filepath, &st) == 0) {
            print_message("Warning", "File size mismatch. Expected: %zu, Actual: %lld. Check: %s", 
                         COLOR_YELLOW, filesize, (long long)st.st_size, filepath);
        } else {
            print_message("Warning", "File may not have been saved correctly. Check: %s", COLOR_YELLOW, filepath);
        }
    }
}

void handle_file_receive(Client *client, const char *filename, size_t filesize) {
    handle_file_receive_with_data(client, filename, filesize, NULL, 0);
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
