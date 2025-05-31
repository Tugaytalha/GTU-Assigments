/**
 * Multi-threaded Distributed Chat and File Server
 * Header file for server implementation
 */

#ifndef CHATSERVER_H
#define CHATSERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <ctype.h>
#include <stdbool.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <strings.h> /* For strcasecmp */

/* Constants */
#define MAX_CLIENTS 15
#define MAX_USERNAME_LEN 16
#define MAX_ROOM_NAME_LEN 32
#define MAX_MESSAGE_LEN 1024
#define MAX_COMMAND_LEN 1024
#define MAX_FILE_SIZE (3 * 1024 * 1024) // 3MB
#define MAX_ROOMS 10
#define MAX_UPLOAD_QUEUE 5
#define MAX_FILE_NAME_LEN 256
#define MAX_PATH_LEN 512
#define LOG_FILE "server_log.txt"
#define FILE_BUFFER_SIZE 8192  // Dosya transferi için tampon boyutu

/* Allowed file extensions */
#define FILE_EXT_COUNT 4
extern const char *ALLOWED_FILE_EXTENSIONS[FILE_EXT_COUNT];

/* Client status */
typedef enum {
    CLIENT_CONNECTED,
    CLIENT_DISCONNECTED,
    CLIENT_AUTHENTICATING
} ClientStatus;

/* Command types */
typedef enum {
    CMD_JOIN,
    CMD_LEAVE,
    CMD_BROADCAST,
    CMD_WHISPER,
    CMD_SENDFILE,
    CMD_EXIT,
    CMD_INVALID
} CommandType;

/* File transfer structure */
typedef struct {
    char sender_username[MAX_USERNAME_LEN + 1];
    char recipient_username[MAX_USERNAME_LEN + 1];
    char filename[MAX_FILE_NAME_LEN];
    size_t filesize;
    time_t queued_time;
    bool in_progress;
} FileTransfer;

/* Room structure */
typedef struct {
    char name[MAX_ROOM_NAME_LEN + 1];
    int member_count;
    int member_sockets[MAX_CLIENTS];
    char member_usernames[MAX_CLIENTS][MAX_USERNAME_LEN + 1];
    pthread_mutex_t room_mutex;
    bool active;
} Room;

/* Client structure */
typedef struct {
    int socket;
    struct sockaddr_in address;
    char username[MAX_USERNAME_LEN + 1];
    char current_room[MAX_ROOM_NAME_LEN + 1];
    ClientStatus status;
    pthread_t thread;
    bool in_file_transfer;  // Flag to indicate if client is currently in file transfer mode
    bool waiting_for_upload_slot; // Flag to indicate this client is waiting for an upload slot
    char queued_filename[MAX_FILE_NAME_LEN]; // Filename queued for upload when slot becomes available
    char queued_recipient[MAX_USERNAME_LEN + 1]; // Recipient for queued file upload
} Client;

/* Server structure */
typedef struct {
    int socket;
    struct sockaddr_in address;
    Client clients[MAX_CLIENTS];
    Room rooms[MAX_ROOMS];
    FileTransfer upload_queue[MAX_UPLOAD_QUEUE];
    int queue_count;
    pthread_mutex_t clients_mutex;
    pthread_mutex_t rooms_mutex;
    pthread_mutex_t queue_mutex;
    sem_t upload_semaphore;
    FILE *log_file;
    bool running;
} Server;

/* Global server instance - extern declaration */
extern Server server;

/* Function declarations */
// Server initialization and shutdown
bool initialize_server(Server *server, int port);
void shutdown_server(Server *server);
void handle_sigint(int sig);

// Client management
void *handle_client(void *arg);
bool authenticate_client(Client *client);
void disconnect_client(Server *server, Client *client);
int find_client_index(Server *server, char *username);
void notify_client(Client *client, const char *message);

// Command handling
CommandType parse_command(const char *command, char *args);
void handle_join_command(Server *server, Client *client, const char *room_name);
void handle_leave_command(Server *server, Client *client);
void handle_broadcast_command(Server *server, Client *client, const char *message);
void handle_whisper_command(Server *server, Client *client, const char *args);
void handle_sendfile_command(Server *server, Client *client, const char *args);
void handle_exit_command(Server *server, Client *client);
void process_waiting_clients(Server *server);

// Room management
int create_or_join_room(Server *server, Client *client, const char *room_name);
void leave_room(Server *server, Client *client);
int find_room_index(Server *server, const char *room_name);

// File transfer
bool validate_file_extension(const char *filename);
bool add_to_upload_queue(Server *server, const char *sender, const char *recipient, const char *filename, size_t filesize);
void *process_upload_queue(void *arg);
bool send_file(Client *sender, Client *recipient, const char *filename);
bool receive_file(Client *client, const char *filename, size_t filesize);

// Logging
void log_message(Server *server, const char *format, ...);
void log_client_action(Server *server, const char *username, const char *action);
void log_file_transfer(Server *server, const char *sender, const char *recipient, const char *filename, bool success);

// Utility functions
bool is_valid_username(const char *username);
bool is_valid_room_name(const char *room_name);
bool is_username_taken(Server *server, const char *username);
void get_timestamp(char *buffer, size_t size);

#endif /* CHATSERVER_H */
