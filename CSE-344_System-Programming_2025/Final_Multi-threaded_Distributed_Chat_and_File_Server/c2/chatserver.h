/**
 * chatserver.h - Main header file for the chat server
 * CSE 344 - System Programming
 */

#ifndef CHATSERVER_H
#define CHATSERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <time.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>
#include <errno.h>
#include <stdbool.h>

/* Configuration constants */
#define MAX_CLIENTS             15      /* Maximum number of concurrent clients */
#define MAX_USERNAME_LENGTH     16      /* Maximum username length */
#define MAX_ROOM_NAME_LENGTH    32      /* Maximum room name length */
#define MAX_ROOMS               10      /* Maximum number of concurrent rooms */
#define MAX_ROOM_CAPACITY       15      /* Maximum users per room */
#define MAX_MESSAGE_LENGTH      1024    /* Maximum message length */
#define MAX_COMMAND_LENGTH      1024    /* Maximum command length */
#define MAX_UPLOAD_QUEUE        5       /* Maximum concurrent file uploads */
#define MAX_FILE_SIZE           (3*1024*1024) /* 3MB maximum file size */
#define SERVER_BUFFER_SIZE      4096    /* Server buffer size for communications */
#define UPLOAD_DIR              "uploads"/* Directory for uploaded files */
#define LOG_FILENAME            "server_log.txt" /* Log file name */

/* Client status enum */
typedef enum {
    CLIENT_DISCONNECTED = 0,
    CLIENT_CONNECTED = 1
} ClientStatus;

/* Forward declarations */
typedef struct Client Client;
typedef struct Room Room;
typedef struct FileTransfer FileTransfer;

/* Server state */
extern int server_socket;
extern int server_running;
extern pthread_mutex_t clients_mutex;
extern pthread_mutex_t rooms_mutex;
extern pthread_mutex_t log_mutex;
extern pthread_mutex_t upload_queue_mutex;
extern sem_t upload_queue_semaphore;
extern Client* clients[MAX_CLIENTS];
extern Room* rooms[MAX_ROOMS];
extern FileTransfer* upload_queue[MAX_UPLOAD_QUEUE];
extern int upload_queue_size;
extern FILE* log_file;

/* Signal handler for clean shutdown */
void handle_signal(int sig);

/* Server initialization and cleanup */
int initialize_server(int port);
void cleanup_server(void);

/* Main server functions */
void* accept_connections(void* arg);
void shutdown_server(void);
int get_connected_client_count(void);

#endif /* CHATSERVER_H */
