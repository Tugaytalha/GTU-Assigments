#ifndef COMMON_H
#define COMMON_H
/* ISO C + Single-UNIX-Spec / POSIX-1.2008 headers */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <time.h>
#include <errno.h>
#include <semaphore.h>

/* ---------- tiny protocol ---------- */
#define MAX_ACCOUNTS  64
#define MAX_LINE      128

typedef enum { DEPOSIT, WITHDRAW } op_t;

typedef struct {          /* goes over the public Server FIFO            */
    int   id;             /* -1 ⇒ new account                            */
    op_t  op;
    int   amount;
    pid_t client;         /* not used by Server, handy for debugging     */
} txn_t;

/* in-memory state for one account */
typedef struct { int balance; } acct_t;

/* one anonymous shared page per Teller */
typedef struct {
    sem_t sem_client;     /* Teller -> Server (request ready)  */
    sem_t sem_server;     /* Server -> Teller (answer ready)   */
    txn_t txn;            /* request                           */
    int   result;         /* 1 = allowed, 0 = rejected         */
} shared_t;

/* terse error wrapper */
static inline void die(const char *msg) { perror(msg); perror("Exiting.."); (EXIT_FAILURE); }
#endif
