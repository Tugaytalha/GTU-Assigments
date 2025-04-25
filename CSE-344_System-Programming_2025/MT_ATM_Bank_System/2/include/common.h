#ifndef ADABANK_COMMON_H
#define ADABANK_COMMON_H  1

/*Feature switches*/
#ifndef _POSIX_C_SOURCE
#   define _POSIX_C_SOURCE 200809L   /* ↑ for shm_open, sigaction …  */
#endif

/*Standard & POSIX API*/
#include <stdint.h>
#include <stdbool.h>
#include <sys/types.h>

/*Compile-time configuration*/
#define MAX_ACCOUNTS    1024            /* persistent DB capacity*/
#define MAX_SLOTS       32              /* concurrent teller slots*/
#define MAX_NAME_LEN    32              /* printable client identifier*/

#define SERVER_FIFO_FILE   "Adabank.fifo"   /* created in server CWD*/
#define CLIENT_FIFO_TMPL   "/tmp/adabank_%d.fifo"  /* %d → client PID */

#define FIFO_PERM       0600            /* rw owner only*/
#define SHM_NAME        "/adabank_db"   /* POSIX shared-memory object*/
#define SEM_NAME_BASE   "/adabank_sem"  /* will append _emptyNN/_fullNN*/

#define BANKID_NONE     0u              /* “N” in the spec (= new acct)*/

/*Enumerations*/
typedef enum : uint8_t {
    REQ_DEPOSIT  = 0,
    REQ_WITHDRAW = 1,
    REQ_OPEN     = 2,   /* internal: new account*/
    REQ_CLOSE    = 3    /* internal: withdraw all → close*/
} op_t;

/* Unified status codes returned to a client (negative = error) */
typedef enum : int32_t {
    ST_OK              =  0,
    ST_EINVAL_ACCOUNT  = -1,
    ST_EINVAL_AMOUNT   = -2,
    ST_NSF             = -3,   /* Non-sufficient funds*/
    ST_INTERNAL        = -4
} status_t;

/*Persistent DB records*/
typedef struct {
    uint32_t acct_id;          /* 1 … MAX_ACCOUNTS, 0 = unused*/
    int64_t  balance;          /* current balance ≥ 0*/
    uint64_t total_deposits;   /* lifetime aggregate – for log*/
    uint64_t total_withdraws;  /* lifetime aggregate – for log*/
} account_t;

/*Server ⇆ Teller request slot*/
typedef struct {
    uint32_t slot_id;          /* index 0 … MAX_SLOTS-1*/
    pid_t    teller_pid;       /* filled by Teller*/
    op_t     op;               /* requested operation*/
    uint32_t acct_id;          /* BANKID_NONE (=0) means new account*/
    int64_t  amount;           /* ≥ 0; ignored for REQ_CLOSE*/
} request_t;

typedef struct {
    status_t status;           /* ST_OK or negative error*/
    uint32_t acct_id;          /* possibly new/closed id*/
    int64_t  balance;          /* post-operation balance*/
} response_t;

/*Complete shared-memory organisation*/
typedef struct {
    account_t  db[MAX_ACCOUNTS];
    request_t  req[MAX_SLOTS];
    response_t rsp[MAX_SLOTS];
} shm_bank_t;

typedef struct {
    shm_bank_t core;                    /* DB + req + rsp         */
    sem_t      sem_empty[MAX_SLOTS];
    sem_t      sem_full [MAX_SLOTS];
} shm_layout_t;


/*Process-creation wrappers*/
/* 100-point requirement wrappers – implemented in teller_lib.c*/
#ifdef __cplusplus
extern "C" {
#endif

pid_t Teller(void *(*func)(void *), void *arg);          /* clone-based */
int   waitTeller(pid_t pid, int *status);                /* waitpid()*/

#ifdef __cplusplus
}
#endif

/*Convenience helper macros*/
#define BANKID_TO_STRING(buf, id) \
    (snprintf((buf), sizeof(buf), "BankID_%u", (id)), (buf))

static inline bool is_valid_amount(int64_t a)
{
    return a >= 0;
}

static inline bool is_valid_account(uint32_t id)
{
    return id > 0 && id <= MAX_ACCOUNTS;
}

#endif /* ADABANK_COMMON_H */
