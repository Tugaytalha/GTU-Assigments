/**
 * chatclient.h - Main header file for the chat client
 * CSE 344 - System Programming
 */

#ifndef CHATCLIENT_H
#define CHATCLIENT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <termios.h>

/* Configuration constants */
#define MAX_USERNAME_LENGTH     16      /* Maximum username length */
#define MAX_MESSAGE_LENGTH      1024    /* Maximum message length */
#define MAX_COMMAND_LENGTH      1024    /* Maximum command length */
#define CLIENT_BUFFER_SIZE      4096    /* Client buffer size */
#define DOWNLOAD_DIR            "downloads" /* Directory for downloaded files */
#define MAX_FILE_SIZE           (3*1024*1024) /* 3MB maximum file size */

/* File transfer command prefix */
#define FILE_COMMAND_PREFIX "FILE_TRANSFER:"

/* ANSI color codes */
#define COLOR_RESET   "\x1B[0m"
#define COLOR_RED     "\x1B[31m"
#define COLOR_GREEN   "\x1B[32m"
#define COLOR_YELLOW  "\x1B[33m"
#define COLOR_BLUE    "\x1B[34m"
#define COLOR_MAGENTA "\x1B[35m"
#define COLOR_CYAN    "\x1B[36m"
#define COLOR_WHITE   "\x1B[37m"

/* Client state */
typedef enum {
    STATE_DISCONNECTED = 0,
    STATE_CONNECTING,
    STATE_CONNECTED,
    STATE_LOGIN,
    STATE_READY
} ClientState;

/* File receiving state */
typedef struct {
    int active;                     /* Whether file transfer is active */
    char filename[MAX_MESSAGE_LENGTH]; /* Filename */
    char sender[MAX_USERNAME_LENGTH + 1]; /* Sender username */
    size_t file_size;               /* Total file size */
    size_t received_size;           /* Bytes received so far */
    FILE* file;                     /* File pointer */
} FileReceiveState;

/* Global variables */
extern int client_socket;
extern int client_running;
extern ClientState client_state;
extern char username[MAX_USERNAME_LENGTH + 1];
extern char current_room[MAX_MESSAGE_LENGTH];
extern pthread_mutex_t client_mutex;
extern pthread_t receive_thread;
extern FileReceiveState file_state;

/* Function prototypes */
int connect_to_server(const char* ip, int port);
void disconnect_from_server(void);
void* receive_messages(void* arg);
void process_user_input(void);
void handle_command(const char* command);
void send_to_server(const char* message);
void handle_file_header(size_t header_size);
int save_file_chunk(const char* data, size_t size);
void print_welcome_message(void);
void print_help(void);
void cleanup_client(void);
void handle_signal(int sig);

#endif /* CHATCLIENT_H */
