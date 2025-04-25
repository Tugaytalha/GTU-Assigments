#ifndef BANK_COMMON_H
#define BANK_COMMON_H
#define  _POSIX_C_SOURCE 200809L
#include <semaphore.h>
#include <sys/types.h>
#include <stdint.h>

#define SHM_NAME        "/adabank_shm"
#define SEM_EMPTY_NAME  "/adabank_empty"
#define SEM_FULL_NAME   "/adabank_full"
#define SEM_MUTEX_NAME  "/adabank_mutex"

#define SERVER_LOG      "AdaBank.bankLog"
#define DB_INIT_BALANCE 0L
#define BUFSZ           32
#define MAX_ACC         1024

typedef struct {
    pid_t   teller_pid;
    int     account_id;         /* -1 == new account */
    long    amount;
    char    op;                 /* 'D' or 'W'        */
    char    client_fifo[128];
} BankRequest;

typedef struct {
    int     status;             /* 0 ok, <0 error    */
    int     account_id;
    long    balance;            /* new balance       */
} BankReply;

typedef struct {
    size_t      head, tail;
    BankRequest req[BUFSZ];
    BankReply   rep[BUFSZ];
} ShmRing;

#endif /* BANK_COMMON_H */
