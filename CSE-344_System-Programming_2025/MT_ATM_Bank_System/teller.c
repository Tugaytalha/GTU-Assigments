/* teller.c */
#include "teller.h"

static shared_t *g_shm = NULL;          /* visible to both parent & child */

shared_t *teller_shared(void) { return g_shm; }

pid_t Teller(void *(*func)(void *), void *arg)
{
    /* 1. one anonymous shared page ----------------------------------- */
    g_shm = mmap(NULL, sizeof(shared_t), PROT_READ | PROT_WRITE,
                 MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (g_shm == MAP_FAILED) die("mmap");

    if (sem_init(&g_shm->sem_client, /*pshared=*/1, /*value=*/0) == -1 ||
        sem_init(&g_shm->sem_server, 1, 0) == -1)
        die("sem_init");

    /* 2. fork – child will *only* run func(arg) ----------------------- */
    pid_t pid = fork();
    if (pid < 0) die("fork");

    if (pid == 0) {                     /* -------- Teller child ------- */
        (*func)(arg);
        munmap(g_shm, sizeof *g_shm);
        _exit(0);                       /* never return to caller       */
    }
    return pid;                         /* -------- parent (Server) ---- */
}

int waitTeller(pid_t pid, int *status) { return waitpid(pid, status, 0); }
