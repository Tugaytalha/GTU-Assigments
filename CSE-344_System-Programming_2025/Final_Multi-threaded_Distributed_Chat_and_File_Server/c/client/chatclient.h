/**
 * Multi-threaded Distributed Chat and File Server
 * Client header file
 */

#ifndef CHATCLIENT_H
#define CHATCLIENT_H

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

/* ANSI color codes for terminal output */
#define COLOR_RED     "\x1b[31m"
#define COLOR_GREEN   "\x1b[32m"
#define COLOR_YELLOW  "\x1b[33m"
#define COLOR_BLUE    "\x1b[34m"
#define COLOR_MAGENTA "\x1b[35m"
#define COLOR_CYAN    "\x1b[36m"
#define COLOR_RESET   "\x1b[0m"

/* Constants */
#define MAX_USERNAME_LEN 16
#define MAX_MESSAGE_LEN 1024
#define MAX_COMMAND_LEN 1024
#define MAX_FILE_NAME_LEN 256
#define MAX_FILE_SIZE (3 * 1024 * 1024) // 3MB
#define FILE_BUFFER_SIZE 8192
#define RECV_BUFFER_SIZE 2048
#define UPLOADS_DIR "uploads"

/* Client structure */
typedef struct {
    int socket;
    struct sockaddr_in server_address;
    char username[MAX_USERNAME_LEN + 1];
    bool connected;
    pthread_t receiver_thread;
    bool exit_flag;
} Client;

/* Function declarations */
// Client initialization and connection
bool initialize_client(Client *client, const char *server_ip, int port);
bool connect_to_server(Client *client);
void disconnect_from_server(Client *client);
void handle_sigint(int sig);

// Command handling
void process_user_input(Client *client);
bool send_command(Client *client, const char *command);
bool handle_file_send(Client *client, const char *filename, const char *recipient);

// Message reception
void *receive_messages(void *arg);
void handle_file_receive(Client *client, const char *header, size_t filesize);
void handle_file_receive_with_data(Client *client, const char *filename, size_t filesize, 
                                   const char *initial_data, size_t initial_len);

// Utility functions
void print_welcome_message();
void print_help();
void print_message(const char *prefix, const char *format, const char *color, ...);
void create_uploads_directory();

#endif /* CHATCLIENT_H */
