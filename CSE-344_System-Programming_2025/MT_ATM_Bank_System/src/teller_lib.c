#define _GNU_SOURCE                 /* clone() flags, MAP_STACK …    */

#include "common.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sched.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>
#include <pthread.h>

/*Stack bookkeeping table*/
struct stack_rec {
    pid_t              pid;
    void              *stack_base;    /* mmap-ed address (start)     */
    size_t             stack_size;    /* bytes                       */
    struct stack_rec  *next;
};

static struct {
    struct stack_rec *head;
    pthread_mutex_t   mtx;
} g_tbl = { .head = NULL, .mtx = PTHREAD_MUTEX_INITIALIZER };

/*Internal helper prototypes*/
static int  add_record(pid_t, void*, size_t);
static void drop_record(pid_t);

/*Child trampoline – calls user’s *func*/
static int child_trampoline(void *raw)
{
    /* raw points to a struct { void*(*f)(void*); void* arg; }       */
    void *(*fn)(void *) = ((void **)(raw))[0];
    void * arg          = ((void **)(raw))[1];
    free(raw);                                    /* not needed now  */

    fn(arg);                                      /* run user code   */
    _exit(0);                                     /* never returns   */
}

/*API*/
pid_t Teller(void *(*func)(void *), void *arg)
{
    if (!func) { errno = EINVAL; return -1; }

    /* 1. Allocate stack (1 MiB) – MAP_STACK ensures guard pages     */
    const size_t stksz = 1 << 20;                 /* 1 MiB           */
    void *stack = mmap(NULL, stksz,
                       PROT_READ | PROT_WRITE,
                       MAP_PRIVATE | MAP_ANONYMOUS | MAP_STACK,
                       -1, 0);
    if (stack == MAP_FAILED) return -1;

    /* 2. Pack user func+arg into heap so child can free-stand       */
    void **pack = malloc(2 * sizeof(void*));
    if (!pack) { munmap(stack, stksz); return -1; }
    pack[0] = (void *)func;
    pack[1] = arg;

    /* 3. clone() – stack grows down so pass (stack + stksz)         */
    int flags = SIGCHLD;               /* child sends SIGCHLD on exit */
    pid_t pid = clone(child_trampoline,
                      (char*)stack + stksz,
                      flags,
                      pack);
    if (pid == -1)
    {
        perror("clone");
        munmap(stack, stksz);
        free(pack);
        return -1;
    }

    /* 4. Register stack for later cleanup                           */
    if (add_record(pid, stack, stksz) != 0)
    {
        /* Bookkeeping failed – shouldn’t happen; clean up parent’s
           view so at worst we leak memory, not break logic.        */
        /* (We *cannot* safely unmap now – child is using it!)       */
        fprintf(stderr, "Teller: bookkeeping failed for pid %d\n", pid);
    }

    return pid;                                 /* parent returns pid */
}

int waitTeller(pid_t pid, int *status)
{
    int rc = waitpid(pid, status, 0);
    if (rc == -1) return -1;                    /* errno set by wait */

    /* Reclaim stack mapping */
    drop_record(pid);
    return rc;
}

/*Stack table – tiny linked-list helpers*/
static int add_record(pid_t pid, void *base, size_t sz)
{
    struct stack_rec *rec = malloc(sizeof(*rec));
    if (!rec) return -1;
    rec->pid        = pid;
    rec->stack_base = base;
    rec->stack_size = sz;

    pthread_mutex_lock(&g_tbl.mtx);
    rec->next = g_tbl.head;
    g_tbl.head = rec;
    pthread_mutex_unlock(&g_tbl.mtx);
    return 0;
}

static void drop_record(pid_t pid)
{
    pthread_mutex_lock(&g_tbl.mtx);
    struct stack_rec *prev = NULL, *cur = g_tbl.head;
    while (cur && cur->pid != pid) {
        prev = cur; cur = cur->next;
    }
    if (cur) {
        if (prev) prev->next = cur->next;
        else      g_tbl.head = cur->next;
    }
    pthread_mutex_unlock(&g_tbl.mtx);

    if (cur) {
        munmap(cur->stack_base, cur->stack_size);
        free(cur);
    }
}
