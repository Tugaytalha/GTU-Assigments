#ifndef NEHOSSERVER_H
#define NEHOSSERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h> 
#include <dirent.h>
#include <errno.h>

// Error codes
#define ERR_FIFO_CREATE 1
#define ERR_FIFO_OPEN 2
#define ERR_FIFO_WRITE 3
#define ERR_FIFO_READ 4
#define ERR_FIFO_CLOSE 5

#define ERR_FORK 10

#define ERR_INVALID_COMMAND 20

#define SOCKET_CREATE_ERROR 30
#define SOCKET_BIND_ERROR 31

#define ERR_LOG_CREATE 40

#define ERR_SIGNAL 50
#define ERR_SET_SIG_HANDLER 51

// Constants
#define SERVER_FIFO "/tmp/neHos_sv"    /* Well-known name for server's FIFO */
#define CLIENT_FIFO_TEMPLATE1 "/tmp/neHos1_cl.%ld"   /* Template for building client FIFO name */
#define CLIENT_FIFO_TEMPLATE2 "/tmp/neHos2_cl.%ld"   /* Template for building client FIFO name */
#define CLIENT_FIFO_NAME_LEN (sizeof(CLIENT_FIFO_TEMPLATE1) + 20)

#define MAX_LENGTH 5096

#define MAX_CLIENTS 30

// ENUM for commands
enum command {
    LIST,
    READF,
    WRITET,
    UPLOAD,
    DOWNLOAD,
    ARCHSERVER,
    KILLSERVER,
    QUIT
};

// Structs
struct request {
    pid_t pid;
};

struct reqCommand
{
    pid_t pid;
    enum command cmd;
    int status;
    char args[MAX_LENGTH];    
};

struct respCommand
{
    char response[MAX_LENGTH];
    int status;
};


struct response {
    int accepted;
};

#endif
