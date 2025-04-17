/***********************************************************************
 *  hw3  – satellite ground‑station simulation (pure C, no stdio.h)
 *         *** corrected version – 17‑Apr‑2025 ***
 **********************************************************************/
#define _POSIX_C_SOURCE 200809L   /* clock_gettime(), sem_timedwait() */
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <pthread.h>
#include <semaphore.h>

/* -------------------- constants & helpers ------------------------- */
#define NUM_ENGINEERS          3
#define CONNECTION_WINDOW_MS   5000
#define PROCESS_TIME_MIN_MS    4000
#define PROCESS_TIME_MAX_MS    9000

/* fast itoa (positive only, we never pass negatives) */
static int u32_to_str(unsigned v, char *buf) {
    char tmp[11]; int n = 0;
    do { tmp[n++] = '0' + (v % 10); v /= 10; } while (v);
    for (int i = 0; i < n; ++i) buf[i] = tmp[n-1-i];
    return n;
}

/* build‑once / write‑once logging macros to avoid inter‑leaving */
#define LINE_BUF       char line[96]; int pos = 0
#define PUT_CONST(s)   do{ size_t L=sizeof(s)-1; memcpy(line+pos,s,L); pos+=L;}while(0)
#define PUT_INT(v)     do{ pos+=u32_to_str((v),line+pos);}while(0)
#define FLUSH()        write(1,line,pos)

static void log_satellite(int id,int pr){
    LINE_BUF;
    PUT_CONST("[SATELLITE] Satellite "); PUT_INT(id);
    PUT_CONST(" requesting (priority "); PUT_INT(pr); PUT_CONST(")\n"); FLUSH();
}
static void log_handle(int eng,int sat,int pr){
    LINE_BUF;
    PUT_CONST("[ENGINEER "); PUT_INT(eng); PUT_CONST("] Handling Satellite ");
    PUT_INT(sat); PUT_CONST(" (Priority "); PUT_INT(pr); PUT_CONST(")\n"); FLUSH();
}
static void log_finished(int eng,int sat){
    LINE_BUF;
    PUT_CONST("[ENGINEER "); PUT_INT(eng); PUT_CONST("] Finished Satellite ");
    PUT_INT(sat); PUT_CONST("\n"); FLUSH();
}
static void log_timeout(int sat,int sec){
    LINE_BUF;
    PUT_CONST("[TIMEOUT]   Satellite "); PUT_INT(sat);
    PUT_CONST(" timeout "); PUT_INT(sec); PUT_CONST(" second.\n"); FLUSH();
}
static void log_exit(int eng){
    LINE_BUF;
    PUT_CONST("[ENGINEER "); PUT_INT(eng); PUT_CONST("] Exiting...\n"); FLUSH();
}

/* ------------------------- request struct -------------------------- */
typedef struct Request {
    int               id, priority;
    struct timespec   deadline;
    sem_t             handled;
    volatile int      aborted;          /* set by satellite on timeout */
    struct Request   *next;
} Request;

/* ------------------------- globals --------------------------------- */
static Request        *queueHead  = NULL;          /* linked list (prio order) */
static pthread_mutex_t queueLock  = PTHREAD_MUTEX_INITIALIZER;
static sem_t           newRequest;
static volatile int    stopEngineers = 0;

/* priority‑ordered linked‑list helpers */
static void queue_push(Request *r)
{
    if (!queueHead || queueHead->priority >= r->priority) {
        r->next = queueHead; queueHead = r; return;
    }
    Request *c = queueHead;
    while (c->next && c->next->priority < r->priority) c = c->next;
    r->next = c->next; c->next = r;
}
static Request* queue_pop(void)
{
    if (!queueHead) return NULL;
    Request *r = queueHead; queueHead = r->next; return r;
}

/* ------------------------- satellite thread ------------------------ */
static void* satellite(void *arg)
{
    int id   = *(int*)arg;
    int prio = 1 + rand()%5;

    Request *req = malloc(sizeof(*req));
    req->id = id; req->priority = prio; req->aborted = 0; req->next = NULL;
    sem_init(&req->handled, 0, 0);

    clock_gettime(CLOCK_REALTIME, &req->deadline);
    req->deadline.tv_sec  += CONNECTION_WINDOW_MS / 1000;
    req->deadline.tv_nsec += (CONNECTION_WINDOW_MS % 1000) * 1000000L;
    if (req->deadline.tv_nsec >= 1000000000L) {
        req->deadline.tv_sec++; req->deadline.tv_nsec -= 1000000000L;
    }

    /* post request */
    pthread_mutex_lock(&queueLock); queue_push(req); pthread_mutex_unlock(&queueLock);
    sem_post(&newRequest);
    log_satellite(id, prio);

    /* wait for engineer or timeout */
    if (sem_timedwait(&req->handled, &req->deadline) == -1 && errno == ETIMEDOUT) {
        req->aborted = 1;                              /* engineer will notice & free */
        log_timeout(id, CONNECTION_WINDOW_MS/1000);
    }
    return NULL;                                      /* never free/destroy here  */
}

/* ------------------------- engineer thread ------------------------- */
static void* engineer(void *arg)
{
    int eng = *(int*)arg;
    while (1) {
        sem_wait(&newRequest);
        if (stopEngineers) break;

        pthread_mutex_lock(&queueLock);
        Request *req = queue_pop();
        pthread_mutex_unlock(&queueLock);
        if (!req) continue;          /* could wake spuriously */

        if (req->aborted) {          /* timed‑out before pick‑up */
            sem_destroy(&req->handled);
            free(req);
            continue;
        }

        sem_post(&req->handled);                 /* tell satellite we took it */
        log_handle(eng, req->id, req->priority);

        /* simulate work */
        int dur = PROCESS_TIME_MIN_MS +
                  rand() % (PROCESS_TIME_MAX_MS-PROCESS_TIME_MIN_MS+1);
        usleep((useconds_t)dur * 1000);

        log_finished(eng, req->id);
        sem_destroy(&req->handled);
        free(req);
    }
    log_exit(eng);
    return NULL;
}

/* ------------------------------- main ------------------------------ */
int main(int argc, char **argv)
{
    int numSat = (argc>1) ? atoi(argv[1]) : 5;
    srand((unsigned)time(NULL));

    sem_init(&newRequest, 0, 0);

    pthread_t engThread[NUM_ENGINEERS];
    int       engId   [NUM_ENGINEERS];
    for (int i=0;i<NUM_ENGINEERS;i++){
        engId[i]=i;
        pthread_create(&engThread[i], NULL, engineer, &engId[i]);
    }

    pthread_t *satThread = malloc(sizeof(*satThread)*numSat);
    int       *satId     = malloc(sizeof(*satId)*numSat);

    for (int i=0;i<numSat;i++){
        satId[i]=i;
        pthread_create(&satThread[i], NULL, satellite, &satId[i]);
        usleep(250000);                               /* 0.25 s stagger */
    }

    for (int i=0;i<numSat;i++) pthread_join(satThread[i], NULL);

    stopEngineers = 1;
    for (int i=0;i<NUM_ENGINEERS;i++) sem_post(&newRequest);
    for (int i=0;i<NUM_ENGINEERS;i++) pthread_join(engThread[i], NULL);

    free(satThread); free(satId);
    sem_destroy(&newRequest);
    return 0;
}
