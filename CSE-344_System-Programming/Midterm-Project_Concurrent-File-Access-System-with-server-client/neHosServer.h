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

#define ERR_FIFO_CREATE 1
#define ERR_FIFO_OPEN 2
#define ERR_FIFO_WRITE 3
#define ERR_FIFO_READ 4
#define ERR_FIFO_CLOSE 5

#define ERR_FORK 10

#define ERR_INVALID_COMMAND 20

#define SOCKET_CREATE_ERROR 30
#define SOCKET_BIND_ERROR 31

#define LOG_CREATE_ERROR 40

#define MAX_LENGTH 2048

#define MAX_CLIENTS 10

struct request {
    pid_t pid;
    int seqLen;
};

struct response {
    int seqNum;
};

#endif
