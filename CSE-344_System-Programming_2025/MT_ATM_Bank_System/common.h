#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/shm.h>  // For System V shared memory functions
#include <semaphore.h>
#include <signal.h>
#include <errno.h>
#include <time.h>

#define MAX_CLIENTS 20
#define MAX_BUFFER 1024
#define MAX_BANK_ID_LEN 20
#define MAX_LOG_LINE 256

// Request types
#define REQUEST_DEPOSIT 1
#define REQUEST_WITHDRAW 2

// Client request structure
typedef struct {
    int type;               // REQUEST_DEPOSIT or REQUEST_WITHDRAW
    int amount;             // Amount to deposit or withdraw
    char bank_id[MAX_BANK_ID_LEN]; // Bank ID (empty for new clients)
    char client_fifo[MAX_BUFFER];  // Client's FIFO name for response
} ClientRequest;

// Teller response structure
typedef struct {
    int status;             // 0 for success, negative for error
    int balance;            // Current balance after transaction
    char bank_id[MAX_BANK_ID_LEN]; // Assigned bank ID
    char message[MAX_BUFFER];      // Status message
} TellerResponse;

// Shared memory structure for Teller-Server communication
typedef struct {
    int client_id;          // Client identifier
    char bank_id[MAX_BANK_ID_LEN]; // Bank ID
    int type;               // Operation type
    int amount;             // Transaction amount
    int current_balance;    // Current balance
    int status;             // Operation status (0 for success)
} SharedData;

// Error codes
#define ERR_INSUFFICIENT_FUNDS -1
#define ERR_INVALID_AMOUNT -2
#define ERR_INVALID_BANKID -3

// Paths and names
#define SERVER_FIFO_PREFIX "/tmp/ServerFIFO_"
#define CLIENT_FIFO_PREFIX "/tmp/ClientFIFO_"
#define DEFAULT_LOG_FILE "AdaBank.bankLog"

#endif /* COMMON_H */ 