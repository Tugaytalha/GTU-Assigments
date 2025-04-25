#define _POSIX_C_SOURCE 200809L
#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <signal.h>
#include <semaphore.h>

/*Arg bundle from server*/
struct teller_arg {
    uint32_t slot;
    char     pipe_name[128];
    uint32_t req_cnt;
};

/*Local helpers*/
static shm_bank_t *core        = NULL;   /* alias after mmap          */
static sem_t      *sem_empty   = NULL;   /* &shm->sem_empty[slot]     */
static sem_t      *sem_full    = NULL;   /* &shm->sem_full [slot]     */
static uint32_t    my_slot     = 0;

static void graceful(int sig) { _exit(0); }

static pid_t my_pid;                     /* for debug prints          */

static int open_shared_region(void)
{
    int fd = shm_open(SHM_NAME, O_RDWR, 0);
    if (fd == -1) return -1;

    void *map = mmap(NULL, sizeof(shm_layout_t),
                     PROT_READ | PROT_WRITE,
                     MAP_SHARED, fd, 0);
    close(fd);
    if (map == MAP_FAILED) return -1;

    shm_layout_t *layout = (shm_layout_t *)map;
    core      = &layout->core;
    /* sem arrays sit *after* core inside same mapping          */
    sem_empty = &layout->sem_empty[my_slot];
    sem_full  = &layout->sem_full [my_slot];
    return 0;
}

/* Parse one request text line → request_t fields.
 * Supported formats (case-insensitive):
 *      N deposit 300
 *      BankID_01 withdraw 50
 *      BankID_None deposit 10
 * Returns 0 on success, −1 on syntax error.                        */
static int parse_line(char *ln, request_t *out)
{
    char idbuf[64], opbuf[16];
    long long amt = 0;
    if (sscanf(ln, "%63s %15s %lld", idbuf, opbuf, &amt) < 3)
        return -1;

    out->amount = (int64_t)amt;
    out->acct_id = BANKID_NONE;

    if (strcasecmp(idbuf, "N") == 0 || strcasestr(idbuf, "None"))
        out->acct_id = BANKID_NONE;
    else if (strncasecmp(idbuf, "BankID_", 7) == 0)
        out->acct_id = (uint32_t)atoi(idbuf + 7);

    if (strcasecmp(opbuf, "deposit") == 0) {
        out->op = (out->acct_id == BANKID_NONE) ? REQ_OPEN : REQ_DEPOSIT;
    }
    else if (strcasecmp(opbuf, "withdraw") == 0) {
        out->op = (out->acct_id == BANKID_NONE) ? REQ_WITHDRAW : REQ_WITHDRAW;
    }
    else return -1;

    return 0;
}

/* Format a user-friendly reply string based on response_t          */
static void build_reply(const request_t *rq, const response_t *rp,
                        char *buf, size_t blen)
{
    if (rp->status != ST_OK) {
        snprintf(buf, blen, "ERROR %d\n", rp->status);
        return;
    }

    switch (rq->op) {
        case REQ_OPEN:
            snprintf(buf, blen, "BankID_%u\n", rp->acct_id);   break;
        case REQ_CLOSE:
            snprintf(buf, blen, "account closed\n");           break;
        default:
            snprintf(buf, blen, "balance %lld\n",
                     (long long)rp->balance);                  break;
    }
}

/*Teller main entry-point*/
void *teller_entry(void *varg)
{
    my_pid = getpid();
    struct teller_arg *a = (struct teller_arg *)varg;
    my_slot = a->slot;

    /*  reopen shared memory  */
    if (open_shared_region() == -1) {
        perror("teller shm_open");
        _exit(EXIT_FAILURE);
    }

    /*  open client FIFO      */
    int cfd = open(a->pipe_name, O_RDWR);
    if (cfd == -1) {
        perror("teller open client fifo");
        _exit(EXIT_FAILURE);
    }

    /* Handle SIGPIPE gracefully (client died)      */
    signal(SIGPIPE, graceful);

    char line[256], reply[128];

    for (uint32_t i = 0; i < a->req_cnt; ++i)
    {
        /* -------- read one text line from client -------- */
        ssize_t n = 0, pos = 0;
        while ((n = read(cfd, &line[pos], 1)) == 1 && line[pos] != '\n') {
            if (++pos >= (ssize_t)sizeof(line)-1) break;
        }
        if (n <= 0) break;                 /* client closed pipe       */
        line[pos] = '\0';

        request_t  rq = { .slot_id = my_slot, .teller_pid = my_pid };
        if (parse_line(line, &rq) == -1) {
            snprintf(reply, sizeof(reply), "syntax error\n");
            write(cfd, reply, strlen(reply));
            continue;
        }

        /* -------- producer side of slot protocol -------- */
        sem_wait(sem_empty);                   /* claim              */
        core->req[my_slot] = rq;
        sem_post(sem_full);                    /* notify             */

        /* Wait for server to process & return slot */
        sem_wait(sem_empty);

        response_t rp = core->rsp[my_slot];

        /* Release slot for next request / teller */
        sem_post(sem_empty);

        /* -------- send reply back to client -------- */
        build_reply(&rq, &rp, reply, sizeof(reply));
        write(cfd, reply, strlen(reply));
    }

    close(cfd);
    free(a);
    return NULL;
}
