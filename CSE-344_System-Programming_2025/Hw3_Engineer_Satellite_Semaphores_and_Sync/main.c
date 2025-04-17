// main.c
#define _POSIX_C_SOURCE 200809L    /* for clock_gettime / sem_timedwait */

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <pthread.h>
#include <semaphore.h>

/* ------------------------ constants -------------------------------- */
#define NUM_ENGINEERS          3
#define CONNECTION_WINDOW_MS   5000
#define PROCESS_TIME_MIN_MS    1500
#define PROCESS_TIME_MAX_MS    3000

/* ---------------- integer → ascii (no stdio) ----------------------- */
static int int_to_buf(int n, char *buf)            /* returns length */
{
    char tmp[12];
    int  len = 0;

    if (n == 0) { buf[0]='0'; return 1; }
    if (n  < 0) { buf[len++]='-'; n = -n; }

    while (n) { tmp[len++] = '0' + (n % 10); n /= 10; }
    for (int i = (buf[0]=='-'?1:0), j = len-1; j >= 0; ++i, --j)
        buf[i] = tmp[j];
    return (buf[0]=='-') ? len+1 : len;
}

/* ------------------------- low‑level log helpers ------------------- */
static void write_const(const char *s, size_t n)   { write(1, s, n); }
static void write_int(int v)
{
    char b[16]; int l = int_to_buf(v, b); write(1, b, l);
}

/* message helpers that reproduce the screenshot */
static void log_satellite(int id,int p){
    write_const("[SATELLITE] Satellite ",22); write_int(id);
    write_const(" requesting (priority ",23); write_int(p);
    write_const(")\n",2);
}
static void log_handle(int eng,int sat,int p){
    write_const("[ENGINEER ",11);  write_int(eng);
    write_const("] Handling Satellite ",22); write_int(sat);
    write_const(" (Priority ",11); write_int(p); write_const(")\n",2);
}
static void log_finished(int eng,int sat){
    write_const("[ENGINEER ",11); write_int(eng);
    write_const("] Finished Satellite ",22); write_int(sat);
    write_const("\n",1);
}
static void log_timeout(int sat,int sec){
    write_const("[TIMEOUT]   Satellite ",22); write_int(sat);
    write_const(" timeout ",9); write_int(sec);
    write_const(" second.\n",9);
}
static void log_exit(int eng){
    write_const("[ENGINEER ",11); write_int(eng);
    write_const("] Exiting...\n",13);
}

/* ------------------------- request struct -------------------------- */
typedef struct Request {
    int               id, priority;
    struct timespec   deadline;
    sem_t             handled;
    int               aborted;
    struct Request   *next;
} Request;

/* ------------------------- globals --------------------------------- */
static Request        *queueHead  = NULL;          /* priority list  */
static pthread_mutex_t queueLock  = PTHREAD_MUTEX_INITIALIZER;
static sem_t           newRequest;
static int             runningSat = 0;
static volatile int    stopEngineers = 0;

/* simple priority‑ordered insert (high→low, FIFO for ties) */
static void queue_push(Request *r)
{
    if(!queueHead || queueHead->priority < r->priority){
        r->next = queueHead; queueHead = r; return;
    }
    Request *cur = queueHead;
    while (cur->next && cur->next->priority >= r->priority)
        cur = cur->next;
    r->next = cur->next; cur->next = r;
}
static Request* queue_pop(void)
{
    if (!queueHead) return NULL;
    Request *r = queueHead; queueHead = queueHead->next;
    return r;
}
static void queue_remove(Request *t)               /* O(n) removal */
{
    if (!queueHead) return;
    if (queueHead == t) { queueHead = t->next; return; }
    Request *cur = queueHead;
    while (cur->next && cur->next != t) cur = cur->next;
    if (cur->next) cur->next = cur->next->next;
}

/* ------------------------- satellite thread ------------------------ */
static void* satellite(void *arg)
{
    int id = *(int*)arg;
    int prio = 1 + rand() % 5;

    Request *req = malloc(sizeof(*req));
    req->id = id; req->priority = prio; req->aborted = 0; req->next=NULL;
    sem_init(&req->handled, 0, 0);

    clock_gettime(CLOCK_REALTIME, &req->deadline);
    req->deadline.tv_sec  += CONNECTION_WINDOW_MS/1000;
    req->deadline.tv_nsec += (CONNECTION_WINDOW_MS%1000)*1000000L;
    if (req->deadline.tv_nsec >= 1000000000L) {
        req->deadline.tv_sec++; req->deadline.tv_nsec-=1000000000L;
    }

    pthread_mutex_lock(&queueLock);
    queue_push(req);
    pthread_mutex_unlock(&queueLock);
    sem_post(&newRequest);

    log_satellite(id, prio);

    int rc = sem_timedwait(&req->handled, &req->deadline);
    if (rc == -1 && errno == ETIMEDOUT) {
        req->aborted = 1;
        pthread_mutex_lock(&queueLock); queue_remove(req); pthread_mutex_unlock(&queueLock);
        log_timeout(id, CONNECTION_WINDOW_MS/1000);
    }

    sem_destroy(&req->handled);
    free(req);
    return NULL;
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

        if (!req) continue;
        if (req->aborted) { free(req); continue; }

        sem_post(&req->handled);
        log_handle(eng, req->id, req->priority);

        int dur = PROCESS_TIME_MIN_MS +
                 rand() % (PROCESS_TIME_MAX_MS-PROCESS_TIME_MIN_MS+1);
        usleep(dur * 1000);

        log_finished(eng, req->id);
        free(req);
    }
    log_exit(eng);
    return NULL;
}

/* ------------------------------- main ------------------------------ */
int main(int argc, char **argv)
{
    int numSat = (argc>1)? atoi(argv[1]): 5;
    srand((unsigned)time(NULL));

    sem_init(&newRequest, 0, 0);

    pthread_t engThreads[NUM_ENGINEERS];
    int engIds[NUM_ENGINEERS];
    for (int i=0;i<NUM_ENGINEERS;i++){
        engIds[i]=i;
        pthread_create(&engThreads[i],NULL,engineer,&engIds[i]);
    }

    pthread_t *satThreads = malloc(numSat*sizeof(*satThreads));
    int       *satIds     = malloc(numSat*sizeof(*satIds));

    for (int i=0;i<numSat;i++){
        satIds[i]=i;
        pthread_create(&satThreads[i],NULL,satellite,&satIds[i]);
        usleep(250*1000);                                     /* 0.25 s stagger */
    }

    for (int i=0;i<numSat;i++) pthread_join(satThreads[i],NULL);

    stopEngineers = 1;
    for (int i=0;i<NUM_ENGINEERS;i++) sem_post(&newRequest);
    for (int i=0;i<NUM_ENGINEERS;i++) pthread_join(engThreads[i],NULL);

    free(satThreads); free(satIds);
    sem_destroy(&newRequest);
    return 0;
}
