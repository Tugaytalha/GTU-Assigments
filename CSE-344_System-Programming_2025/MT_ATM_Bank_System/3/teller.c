/* teller.c — complete */

#define  _POSIX_C_SOURCE 200809L
#include "bank_common.h"
#include "teller.h"

#include <errno.h>
#include <fcntl.h>
#include <semaphore.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

/* ───────── optional debug ───────── */
static int DBG = 0;
#define D(...) do { if (DBG) fprintf(stderr, __VA_ARGS__); } while (0)

/* ───────── arg bundle ───────── */
typedef struct {
    int   account_id;
    long  amount;
    char  op;                /* 'D' or 'W' */
    char  client_fifo[128];
} TellerArgs;

/* ───────── forward declarations ───────── */
static void *deposit(void *arg);
static void *withdraw(void *arg);

/* ───────── Teller() / waitTeller() ───────── */
pid_t Teller(void *(*fn)(void *), void *arg)
{
    pid_t pid = fork();
    if (pid < 0) return -1;
    if (pid == 0) {                      /* child */
        DBG = getenv("ADB_DEBUG") ? 1 : 0;
        fn(arg);
        _exit(0);
    }
    return pid;
}
int waitTeller(pid_t p, int *st) { return waitpid(p, st, 0); }

/* ─────────  SHM helpers ───────── */
static ShmRing *ring_attach(void)
{
    int fd = shm_open(SHM_NAME, O_RDWR, 0);
    if (fd == -1) { perror("shm_open teller"); _exit(2); }
    ShmRing *r = mmap(NULL, sizeof(*r),
                      PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (r == MAP_FAILED) { perror("mmap teller"); _exit(2); }
    close(fd);
    return r;
}
static sem_t *sopen(const char *name)
{
    sem_t *s = sem_open(name, 0);
    if (s == SEM_FAILED) { perror("sem_open teller"); _exit(2); }
    return s;
}

/* ───────── SIGUSR1 handler ───────── */
static void wake(int s) { (void)s; }

/* ───────── common worker ───────── */
static void push_and_wait(TellerArgs *ta)
{
    ShmRing *ring = ring_attach();
    sem_t *empty  = sopen(SEM_EMPTY_NAME);
    sem_t *full   = sopen(SEM_FULL_NAME);
    sem_t *mutex  = sopen(SEM_MUTEX_NAME);

    sem_wait(empty);
    sem_wait(mutex);

    size_t slot = ring->head % BUFSZ;
    ring->req[slot].teller_pid = getpid();
    ring->req[slot].account_id = ta->account_id;
    ring->req[slot].amount     = ta->amount;
    ring->req[slot].op         = ta->op;
    strncpy(ring->req[slot].client_fifo,
            ta->client_fifo,
            sizeof(ring->req[slot].client_fifo) - 1);
    ring->head++;

    sem_post(mutex);
    sem_post(full);

    struct sigaction sa = { .sa_handler = wake };
    sigaction(SIGUSR1, &sa, NULL);
    pause();                                /* sleep until reply */

    BankReply br = ring->rep[slot];

    int fd = open(ta->client_fifo, O_WRONLY);
    if (fd != -1) {
        if (br.status == 0)
            dprintf(fd, "OK %d %ld\n", br.account_id, br.balance);
        else
            dprintf(fd, "FAIL\n");
        close(fd);
    }

    D("teller %d finished (%c)\n", getpid(), ta->op);
}

/* ───────── two exposed ops ───────── */
static void *deposit(void *arg)  { push_and_wait(arg); return NULL; }
static void *withdraw(void *arg) { push_and_wait(arg); return NULL; }

/* helper for server side */
void launch_teller_process(char op, int acc, long amt, const char *fifo)
{
    TellerArgs *ta = malloc(sizeof(*ta));
    *ta = (TellerArgs){ .account_id = acc, .amount = amt, .op = op };
    strncpy(ta->client_fifo, fifo, sizeof(ta->client_fifo)-1);

    pid_t p = Teller((op == 'D' || op == 'd') ? deposit : withdraw, ta);
    (void)p;        /* server keeps no handle; teller is fire-and-forget */
}
