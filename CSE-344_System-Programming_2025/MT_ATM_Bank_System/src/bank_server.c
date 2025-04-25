#define _GNU_SOURCE                 /* for POLLRDHUP, etc.           */
#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/poll.h>
#include <unistd.h>
#include <semaphore.h>
#include <time.h>
#include <sys/wait.h>

/*Shared-memory organisation*/
typedef struct {
    shm_bank_t core;                      /* DB + req + rsp            */
    sem_t      sem_empty[MAX_SLOTS];      /* 1 → slot free             */
    sem_t      sem_full [MAX_SLOTS];      /* 0 → waiting, 1 → ready    */
} shm_layout_t;

/*Global / file-scope data*/
static shm_layout_t *g_shm   = NULL;      /* mapped shared memory      */
static int           g_shm_fd = -1;       /* FD from shm_open()        */
static int           g_fifo_rd = -1;      /* SERVER_FIFO read end      */
static int           g_fifo_wd = -1;      /* dummy write end           */
static volatile sig_atomic_t g_running = 1;

static FILE         *g_logf = NULL;
static char          g_log_filename[64] = "AdaBank.bankLog";

/*Utility: time-stamp string*/
static void log_timestamp(FILE *fp)
{
    time_t     now = time(NULL);
    struct tm  tm;
    localtime_r(&now, &tm);

    char buf[64];
    strftime(buf, sizeof(buf), "# Adabank Log file updated @%H:%M %B %d %Y\n",
             &tm);
    fputs(buf, fp);
}

/*DB helpers*/
static inline account_t *get_account(uint32_t id)
{
    if (!is_valid_account(id)) return NULL;
    return &g_shm->core.db[id - 1];       /* 1-based external ids */
}

static uint32_t allocate_account(void)
{
    for (uint32_t i = 1; i <= MAX_ACCOUNTS; ++i)
    {
        account_t *a = &g_shm->core.db[i - 1];
        if (a->acct_id == 0) {            /* free slot               */
            a->acct_id = i;
            a->balance = 0;
            a->total_deposits = a->total_withdraws = 0;
            return i;
        }
    }
    return 0;                             /* DB full – unrealistic   */
}

/*Request processing (critical)*/
static void process_request(uint32_t slot)
{
    request_t  *rq = &g_shm->core.req[slot];
    response_t *rp = &g_shm->core.rsp[slot];

    /* Default response (in case we early-exit on error) */
    rp->status   = ST_INTERNAL;
    rp->acct_id  = rq->acct_id;
    rp->balance  = -1;

    switch (rq->op)
    {
        case REQ_DEPOSIT:
        {
            if (!is_valid_amount(rq->amount))
            { rp->status = ST_EINVAL_AMOUNT; break; }

            account_t *a = get_account(rq->acct_id);
            if (!a)
            { rp->status = ST_EINVAL_ACCOUNT; break; }

            a->balance         += rq->amount;
            a->total_deposits  += rq->amount;

            rp->status  = ST_OK;
            rp->balance = a->balance;
            break;
        }

        case REQ_WITHDRAW:
        {
            if (!is_valid_amount(rq->amount))
            { rp->status = ST_EINVAL_AMOUNT; break; }

            account_t *a = get_account(rq->acct_id);
            if (!a)
            { rp->status = ST_EINVAL_ACCOUNT; break; }

            if (a->balance < rq->amount)
            { rp->status = ST_NSF; break; }

            a->balance          -= rq->amount;
            a->total_withdraws  += rq->amount;

            /* If becomes zero → potential close handled client-side */
            rp->status  = ST_OK;
            rp->balance = a->balance;
            break;
        }

        case REQ_OPEN:    /* internal helper – allocate & deposit */
        {
            uint32_t new_id = allocate_account();
            if (new_id == 0) {                       /* out of IDs   */
                rp->status = ST_INTERNAL;
                break;
            }
            account_t *a = get_account(new_id);
            a->balance        = rq->amount;
            a->total_deposits = rq->amount;

            rp->status  = ST_OK;
            rp->acct_id = new_id;
            rp->balance = a->balance;
            break;
        }

        case REQ_CLOSE:
        {
            account_t *a = get_account(rq->acct_id);
            if (!a) { rp->status = ST_EINVAL_ACCOUNT; break; }

            /* Invariant: teller only sends CLOSE if balance == 0 */
            memset(a, 0, sizeof(*a));                /* mark free    */

            rp->status  = ST_OK;
            rp->balance = 0;
            break;
        }
    }

    /* ---------- logging (append-only, coarse-grained) ---------- */
    char idbuf[32];
    BANKID_TO_STRING(idbuf, rp->acct_id);

    switch (rq->op)
    {
        case REQ_DEPOSIT:
            fprintf(g_logf, "%s D %ld %ld\n", idbuf,
                    (long)rq->amount, (long)rp->balance);
            break;
        case REQ_WITHDRAW:
            fprintf(g_logf, "%s W %ld %ld\n", idbuf,
                    (long)rq->amount, (long)rp->balance);
            break;
        case REQ_OPEN:
            fprintf(g_logf, "%s +OPEN %ld\n",
                    idbuf, (long)rp->balance);
            break;
        case REQ_CLOSE:
            fprintf(g_logf, "%s -CLOSE\n", idbuf);
            break;
    }
    fflush(g_logf);

    /* Slot is now ready for teller → post sem_empty in caller */
}

/*Poll helper for SERVER_FIFO*/
static ssize_t read_line(int fd, char *buf, size_t maxlen)
{
    size_t n = 0;
    while (n < maxlen - 1)
    {
        char c;
        ssize_t r = read(fd, &c, 1);
        if (r <= 0) return r;                 /* error / EOF */
        if (c == '\n') break;
        buf[n++] = c;
    }
    buf[n] = '\0';
    return (ssize_t)n;
}

/*SIGINT / SIGTERM → graceful*/
static void sig_int_term(int sig)
{
    (void)sig;
    g_running = 0;
}

static void sig_chld(int sig)
{
    (void)sig;
    /* Simply reap here – no blocking; we don't need exit status   */
    while (waitpid(-1, NULL, WNOHANG) > 0)
        ;
}

/*Initialisation steps*/
static void create_or_open_shm(void)
{
    /*  Create with O_EXCL first – if already exists, just open.   */
    g_shm_fd = shm_open(SHM_NAME,
                        O_CREAT | O_EXCL | O_RDWR, FIFO_PERM);
    bool first_time = false;

    if (g_shm_fd < 0 && errno == EEXIST)
    {
        g_shm_fd = shm_open(SHM_NAME, O_RDWR, FIFO_PERM);
    }
    else
    {
        first_time = true;
    }
    if (g_shm_fd < 0)
    {
        perror("shm_open");
        exit(EXIT_FAILURE);
    }

    /* Size the object */
    size_t shm_size = sizeof(shm_layout_t);
    if (first_time)
    {
        if (ftruncate(g_shm_fd, shm_size) == -1) {
            perror("ftruncate");
            exit(EXIT_FAILURE);
        }
    }

    /* Map in */
    g_shm = mmap(NULL, shm_size,
                 PROT_READ | PROT_WRITE,
                 MAP_SHARED, g_shm_fd, 0);
    if (g_shm == MAP_FAILED)
    {
        perror("mmap");
        exit(EXIT_FAILURE);
    }

    if (first_time)
    {
        memset(g_shm, 0, shm_size);
        /* Initialise semaphores */
        for (int i = 0; i < MAX_SLOTS; ++i)
        {
            sem_init(&g_shm->sem_empty[i], 1, 1); /* shared=1, val=1 */
            sem_init(&g_shm->sem_full [i], 1, 0); /* shared=1, val=0 */
        }
    }
}

/*FIFO initialisation*/
static void create_server_fifo(void)
{
    unlink(SERVER_FIFO_FILE);
    if (mkfifo(SERVER_FIFO_FILE, FIFO_PERM) == -1)
    {
        perror("mkfifo");
        exit(EXIT_FAILURE);
    }

    g_fifo_rd = open(SERVER_FIFO_FILE, O_RDONLY | O_NONBLOCK);
    if (g_fifo_rd == -1) { perror("open fifo rd"); exit(EXIT_FAILURE); }

    /* Dummy writer to keep FIFO from getting EOF when no writers */
    g_fifo_wd = open(SERVER_FIFO_FILE, O_WRONLY);
    if (g_fifo_wd == -1) { perror("open fifo wd"); exit(EXIT_FAILURE); }
}

/*Spawn teller given a FIFO announcement*/
struct teller_arg {
    uint32_t slot;
    char     pipe_name[128];
    uint32_t req_cnt;
};

extern void *teller_entry(void *);    /* defined in teller.c */

static void spawn_teller(const char *pipe_name, uint32_t req_cnt)
{
    /* Find a free slot (sem_empty == 1 and sem_full == 0) */
    uint32_t slot = MAX_SLOTS;
    for (uint32_t i = 0; i < MAX_SLOTS; ++i)
    {
        int sval;
        sem_getvalue(&g_shm->sem_empty[i], &sval);
        if (sval == 1) { slot = i; break; }
    }
    if (slot == MAX_SLOTS)
    {
        fprintf(stderr, "No free teller slots!\n");
        return;
    }

    struct teller_arg *arg = calloc(1, sizeof(*arg));
    arg->slot    = slot;
    arg->req_cnt = req_cnt;
    strncpy(arg->pipe_name, pipe_name, sizeof(arg->pipe_name) - 1);

    pid_t pid = Teller(teller_entry, arg);
    if (pid < 0) {
        perror("Teller()");
        free(arg);
        return;
    }

    fprintf(stdout, "-- Teller %d is active serving %s …\n",
            pid, pipe_name);
}

/*Main event loop*/
static void run_event_loop(void)
{
    char line[256];

    struct pollfd pfd = {
        .fd     = g_fifo_rd,
        .events = POLLIN
    };

    while (g_running)
    {
        /* 1. Check shared-memory slots that are ready */
        for (uint32_t i = 0; i < MAX_SLOTS; ++i)
        {
            if (sem_trywait(&g_shm->sem_full[i]) == 0)
            {
                process_request(i);
                sem_post(&g_shm->sem_empty[i]);
            }
        }

        /* 2. Poll FIFO (100 ms timeout) */
        int pr = poll(&pfd, 1, 100);
        if (pr > 0 && (pfd.revents & POLLIN))
        {
            ssize_t n = read_line(g_fifo_rd, line, sizeof(line));
            if (n > 0)
            {
                /* Expected line format: <pid> <pipeName> <cnt> */
                pid_t pid;
                char  pipe_name[128];
                unsigned cnt;
                if (sscanf(line, "%d %127s %u",
                           &pid, pipe_name, &cnt) == 3)
                {
                    fprintf(stdout,
                            "- Received %u requests from PID %d …\n",
                            cnt, pid);
                    spawn_teller(pipe_name, cnt);
                }
            }
        }
    }
}

/*Graceful shutdown sequence*/
static void cleanup(void)
{
    puts("Signal received – closing active Tellers …");

    /* Kill remaining tellers (best-effort) */
    kill(0, SIGTERM);                       /* send to our process grp */

    /* Reap them */
    while (waitpid(-1, NULL, WNOHANG) > 0)
        ;

    /* Flush final log marker */
    fputs("## end of log.\n", g_logf);
    fclose(g_logf);

    /* Unlink FIFO and SHM */
    close(g_fifo_rd);
    close(g_fifo_wd);
    unlink(SERVER_FIFO_FILE);

    munmap(g_shm, sizeof(shm_layout_t));
    close(g_shm_fd);
    shm_unlink(SHM_NAME);
}

/*main*/
int main(void)
{
    puts("> BankServer AdaBank  #" SERVER_FIFO_FILE);
    puts("Adabank is active …");

    /* open/create log */
    g_logf = fopen(g_log_filename, "a+");
    if (!g_logf) { perror("fopen log"); exit(EXIT_FAILURE); }
    log_timestamp(g_logf);

    /* install signals */
    struct sigaction sa_int = { .sa_handler = sig_int_term };
    sigemptyset(&sa_int.sa_mask);
    sigaction(SIGINT,  &sa_int, NULL);
    sigaction(SIGTERM, &sa_int, NULL);

    struct sigaction sa_chld = { .sa_handler = sig_chld, .sa_flags = SA_RESTART };
    sigemptyset(&sa_chld.sa_mask);
    sigaction(SIGCHLD, &sa_chld, NULL);
    signal(SIGPIPE, SIG_IGN);

    create_or_open_shm();
    create_server_fifo();

    puts("Waiting for clients @" SERVER_FIFO_FILE " …");
    run_event_loop();

    cleanup();
    puts("Adabank says “Bye” …");
    return 0;
}
