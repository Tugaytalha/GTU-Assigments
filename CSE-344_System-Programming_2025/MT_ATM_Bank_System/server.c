/* server.c — complete, self-contained */

#define  _POSIX_C_SOURCE 200809L
#include "bank_common.h"
#include "teller.h"

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <semaphore.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

/* ───────── Config / debug ───────── */
static int DBG = 0;
#define D(...) do { if (DBG) fprintf(stderr, __VA_ARGS__); } while (0)

/* ───────── Bank DB ───────── */
typedef struct { int id; long bal; } Account;

static Account db[MAX_ACC];
static size_t  db_used = 0;

static int find_acc(int id)
{
    for (size_t i = 0; i < db_used; ++i)
        if (db[i].id == id) return (int)i;
    return -1;
}
static int new_account(long first_deposit)
{
    int id = (db_used == 0) ? 1 : db[db_used - 1].id + 1;
    db[db_used++] = (Account){ id, first_deposit };
    return id;
}

/* ───────── Logging ───────── */
static FILE *logf = NULL;

static void log_db(void)
{
    if (!logf) return;
    time_t t = time(NULL); struct tm tm; localtime_r(&t, &tm);
    char ts[64]; strftime(ts, sizeof(ts), "%H:%M %B %d %Y", &tm);

    fprintf(logf, "# Adabank Log file updated @%s\n", ts);
    for (size_t i = 0; i < db_used; ++i)
        fprintf(logf, "BankID_%02d %ld\n", db[i].id, db[i].bal);
    fprintf(logf, "## end of log.\n");
    fflush(logf);
}

/* ───────── Shared-memory ring buffer ───────── */
static ShmRing *ring;
static sem_t   *sem_empty, *sem_full, *sem_mutex;

static void shm_sem_init(void)
{
    int fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (fd == -1) { perror("shm_open"); exit(1); }
    ftruncate(fd, sizeof(*ring));
    ring = mmap(NULL, sizeof(*ring), PROT_READ | PROT_WRITE,
                MAP_SHARED, fd, 0);
    if (ring == MAP_FAILED) { perror("mmap"); exit(1); }
    close(fd);
    memset(ring, 0, sizeof(*ring));

    sem_empty = sem_open(SEM_EMPTY_NAME, O_CREAT, 0666, BUFSZ);
    sem_full  = sem_open(SEM_FULL_NAME,  O_CREAT, 0666, 0);
    sem_mutex = sem_open(SEM_MUTEX_NAME, O_CREAT, 0666, 1);
    if (sem_empty == SEM_FAILED || sem_full == SEM_FAILED
        || sem_mutex == SEM_FAILED)
    {
        perror("sem_open"); exit(1);
    }
}

/* ───────── Helpers ───────── */
static volatile sig_atomic_t stop_flag = 0;
static void on_sigint(int s) { (void)s; stop_flag = 1; }

/* consume all ready requests */
static void consume_requests(void)
{
    while (sem_trywait(sem_full) == 0) {
        sem_wait(sem_mutex);

        size_t slot = ring->tail % BUFSZ;
        BankRequest rq = ring->req[slot];
        BankReply   rp = { .status = 0,
                           .account_id = rq.account_id,
                           .balance = -1 };

        int idx = (rq.account_id == -1) ? -1 : find_acc(rq.account_id);

        bool is_dep = (rq.op == 'D' || rq.op == 'd');
        if (is_dep) {                              /* deposit */
            if (idx == -1) {
                int new_id   = new_account(rq.amount);
                rp.account_id = new_id;
                rp.balance    = rq.amount;
                D("[+] new acc %d deposit %ld => %ld\n",
                   new_id, rq.amount, rp.balance);
            } else {
                db[idx].bal += rq.amount;
                rp.balance   = db[idx].bal;
                rp.account_id = db[idx].id;
                D("[+] acc %d deposit %ld => %ld\n",
                   rp.account_id, rq.amount, rp.balance);
            }
        } else {                                   /* withdraw */
            if (idx == -1 || db[idx].bal < rq.amount) {
                rp.status = -1;
                D("[-] acc %d withdraw %ld => FAIL\n",
                   rq.account_id, rq.amount);
            } else {
                db[idx].bal -= rq.amount;
                rp.balance   = db[idx].bal;
                rp.account_id = db[idx].id;
                D("[ ] acc %d withdraw %ld => %ld\n",
                   rp.account_id, rq.amount, rp.balance);

                if (db[idx].bal == 0) {            /* close */
                    D("[ ] acc %d closed\n", db[idx].id);
                    db[idx] = db[--db_used];
                }
            }
        }

        ring->rep[slot] = rp;
        ring->tail++;

        sem_post(sem_mutex);
        kill(rq.teller_pid, SIGUSR1);
    }
}

/* ───────── Cleanup ───────── */
static char server_fifo[128];

static void cleanup(void)
{
    unlink(server_fifo);
    shm_unlink(SHM_NAME);
    sem_unlink(SEM_EMPTY_NAME);
    sem_unlink(SEM_FULL_NAME);
    sem_unlink(SEM_MUTEX_NAME);
    log_db();
    if (logf) fclose(logf);
}

/* ───────── Main ───────── */
int main(int argc, char *argv[])
{
    DBG = getenv("ADB_DEBUG") ? 1 : 0;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <ServerFIFO>\n", argv[0]);
        return 1;
    }
    strncpy(server_fifo, argv[1], sizeof(server_fifo) - 1);

    /* FIFOs ----------------------------------------------------------- */
    mkfifo(server_fifo, 0666);
    int srvfd = open(server_fifo, O_RDONLY | O_NONBLOCK);
    int keep  = open(server_fifo, O_WRONLY);      /* prevent EOF loop   */

    /* log & shm ------------------------------------------------------- */
    logf = fopen(SERVER_LOG, "a+");
    shm_sem_init();

    /* signals --------------------------------------------------------- */
    struct sigaction sa = { .sa_handler = on_sigint };
    sigaction(SIGINT, &sa, NULL);

    printf("BankServer AdaBank %s\nAdaBank is active…\n", server_fifo);

    /* main loop ------------------------------------------------------- */
    char buf[512];
    while (!stop_flag) {

        ssize_t n = read(srvfd, buf, sizeof(buf) - 1);
        if (n > 0) {
            buf[n] = 0;
            char *save = NULL;
            for (char *line = strtok_r(buf, "\n", &save);
                 line;
                 line = strtok_r(NULL, "\n", &save))
            {
                char op;
                char idtok[64];
                long amt;
                char fifo[128];

                if (sscanf(line, " %c %63s %ld %127s",
                           &op, idtok, &amt, fifo) != 4)
                    continue;                     /* malformed */

                /* translate account token */
                int acc_id = -1;
                if (idtok[0] == 'N' || idtok[0] == 'n') {
                    acc_id = -1;
                } else if (strncmp(idtok, "BankID_", 7) == 0) {
                    acc_id = atoi(idtok + 7);
                } else {
                    acc_id = atoi(idtok);
                }

                D("REQ: %c  acc=%d amt=%ld fifo=%s\n",
                  op, acc_id, amt, fifo);

                extern void launch_teller_process(char,int,long,const char*);
                launch_teller_process(op, acc_id, amt, fifo);
            }
        } else if (n == 0) {          /* all writers closed — reopen */
            close(srvfd);
            srvfd = open(server_fifo, O_RDONLY | O_NONBLOCK);
        }

        consume_requests();
        usleep(20 * 1000);            /* 20 ms idle nap */
    }

    /* graceful exit --------------------------------------------------- */
    consume_requests();
    printf("Signal received… shutting down.\n");
    cleanup();
    return 0;
}
