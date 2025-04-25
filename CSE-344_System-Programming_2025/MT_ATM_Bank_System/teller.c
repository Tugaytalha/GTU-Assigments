#define _POSIX_C_SOURCE 200809L
#include "bank_common.h"
#include "teller.h"
#include <fcntl.h>
#include <signal.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>

typedef struct {
    int   account_id;
    long  amount;
    char  op;                /* 'D' or 'W' */
    char  client_fifo[128];
} TellerArgs;

/* Forward declaration of the two allowed ops */
static void *deposit(void *arg);
static void *withdraw(void *arg);

/* ───────────────── Teller() / waitTeller() ─────────────────────────── */
pid_t Teller(void *(*func)(void *), void *arg)
{
    pid_t pid = fork();
    if (pid < 0) return -1;
    if (pid == 0) {          /* child */
        func(arg);
        _exit(0);
    }
    return pid;
}
int waitTeller(pid_t pid, int *status) { return waitpid(pid, status, 0); }

/* ───────────────── Shared-memory helpers ───────────────────────────── */
static ShmRing *attach_ring(void)
{
    int fd = shm_open(SHM_NAME, O_RDWR, 0);
    if (fd == -1) { perror("shm_open teller"); _exit(2); }
    ShmRing *ring = mmap(NULL, sizeof(*ring), PROT_READ|PROT_WRITE,
                         MAP_SHARED, fd, 0);
    if (ring == MAP_FAILED) { perror("mmap teller"); _exit(2); }
    close(fd);
    return ring;
}
static sem_t *open_sem(const char *name)
{
    sem_t *s = sem_open(name, 0);
    if (s == SEM_FAILED) { perror("sem_open teller"); _exit(2); }
    return s;
}
/* ───────────────── SIGUSR1 handler (unblocks pause()) ──────────────── */
static void sigusr1(int signo) { (void)signo; }

/* Common worker for both ops */
static void teller_push_then_wait(TellerArgs *ta)
{
    ShmRing *ring = attach_ring();
    sem_t *empty = open_sem(SEM_EMPTY_NAME);
    sem_t *full  = open_sem(SEM_FULL_NAME);
    sem_t *mutex = open_sem(SEM_MUTEX_NAME);

    /* Reserve slot */
    sem_wait(empty);
    sem_wait(mutex);

    size_t slot = ring->head % BUFSZ;
    ring->req[slot] = (BankRequest){
        .teller_pid   = getpid(),
        .account_id   = ta->account_id,
        .amount       = ta->amount,
        .op           = ta->op,
        .client_fifo  = {0}
    };
    strncpy(ring->req[slot].client_fifo, ta->client_fifo,
            sizeof(ring->req[slot].client_fifo)-1);
    ring->head++;

    sem_post(mutex);
    sem_post(full);

    /* Wait for server's SIGUSR1 */
    struct sigaction sa = {.sa_handler = sigusr1};
    sigaction(SIGUSR1, &sa, NULL);
    pause();                           /* awaken by server */

    /* Read reply */
    BankReply br = ring->rep[slot];

    /* Write to client fifo */
    int fd = open(ta->client_fifo, O_WRONLY);
    if (fd != -1) {
        dprintf(fd, (br.status==0)
            ? "OK %d %ld\n"
            : "FAIL %d\n", br.account_id, br.balance);
        close(fd);
    }
}

/* ───────────────── 2 user-visible functions ───────────────────────── */
static void *deposit(void *arg)
{
    teller_push_then_wait((TellerArgs*)arg);
    return NULL;
}
static void *withdraw(void *arg)
{
    teller_push_then_wait((TellerArgs*)arg);
    return NULL;
}

/* helper so main server can call correct func */
void launch_teller_process(char op, int account_id, long amount,
                           const char *fifo)
{
    TellerArgs *ta = malloc(sizeof(*ta));
    ta->account_id = account_id;
    ta->amount     = amount;
    ta->op         = op;
    strncpy(ta->client_fifo, fifo, sizeof(ta->client_fifo)-1);
    void *(*fn)(void*) = (op=='D') ? deposit : withdraw;
    pid_t p = Teller(fn, ta);
    free(ta);                /* child has its own copy */
    (void)p;                 /* server tracks separately */
}
