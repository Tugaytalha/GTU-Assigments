#define _POSIX_C_SOURCE 200809L
#include "bank_common.h"
#include "teller.h"
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <semaphore.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

/* ───────────────── Bank DB ───────────────── */
typedef struct { int id; long bal; } Account;
static Account db[MAX_ACC];
static size_t  db_used = 0;

static int find_acc(int id)
{
    for (size_t i=0;i<db_used;++i) if (db[i].id==id) return (int)i;
    return -1;
}
static int new_account(long first_deposit)
{
    int id = (db_used==0) ? 1 : db[db_used-1].id + 1;
    db[db_used++] = (Account){id, first_deposit};
    return id;
}

/* ───────────────── Logging ───────────────── */
static FILE *logf = NULL;
static void log_db(void)
{
    if (!logf) return;
    time_t t=time(NULL); struct tm tm; localtime_r(&t,&tm);
    char ts[64]; strftime(ts,sizeof(ts),"%H:%M %B %d %Y",&tm);
    fprintf(logf,"# Adabank Log file updated @%s\n", ts);
    for (size_t i=0;i<db_used;++i)
        fprintf(logf,"BankID_%02d %ld\n", db[i].id, db[i].bal);
    fprintf(logf,"## end of log.\n");
    fflush(logf);
}

/* ───────────────── Shared memory setup ───────────────── */
static ShmRing *ring;
static sem_t *empty,*full,*mutex;
static int    shm_ready = 0;

static void shm_sem_init(void)
{
    int fd = shm_open(SHM_NAME,O_CREAT|O_RDWR,0666);
    ftruncate(fd,sizeof(*ring));
    ring = mmap(NULL,sizeof(*ring),PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);
    close(fd);
    memset(ring,0,sizeof(*ring));

    empty = sem_open(SEM_EMPTY_NAME,O_CREAT,0666,BUFSZ);
    full  = sem_open(SEM_FULL_NAME ,O_CREAT,0666,0);
    mutex = sem_open(SEM_MUTEX_NAME, O_CREAT, 0666, 1);
    shm_ready = 1;
}

/* ─────────────────  Helpers  ───────────────── */

/* consume every ready request and wake its Teller */
static void process_requests(void)
{
    if (!shm_ready) return;
    while (sem_trywait(full) == 0) {          /* 0 ⇒ we just consumed one slot */
        sem_wait(mutex);

        size_t slot = ring->tail % BUFSZ;
        BankRequest rq = ring->req[slot];

        BankReply rp = { .status = 0,
                         .account_id = rq.account_id,
                         .balance = -1 };

        int idx = (rq.account_id == -1) ? -1 : find_acc(rq.account_id);

        if (rq.op == 'D' || rq.op == 'd') {           /* deposit */
            if (idx == -1) {                          /* new account */
                int id = new_account(rq.amount);
                rp.account_id = id;
                rp.balance   = rq.amount;
            } else {
                db[idx].bal += rq.amount;
                rp.balance   = db[idx].bal;
            }
        } else {                                     /* withdraw */
            if (idx == -1 || db[idx].bal < rq.amount) {
                rp.status = -1;       /* insufficient or no such acc */
            } else {
                db[idx].bal -= rq.amount;
                rp.balance   = db[idx].bal;
                if (db[idx].bal == 0)                 /* close acc */
                    db[idx] = db[--db_used];
            }
        }

        ring->rep[slot] = rp;
        ring->tail++;

        sem_post(mutex);
        kill(rq.teller_pid, SIGUSR1);                 /* wake teller */
    }
}

/* ───────────────── Clean up ───────────────── */
static char server_fifo[128];
static volatile sig_atomic_t stop=0;
static void die(int signo){ (void)signo; stop=1; }

static void cleanup(void)
{
    unlink(server_fifo);
    sem_unlink(SEM_EMPTY_NAME);
    sem_unlink(SEM_FULL_NAME);
    sem_unlink(SEM_MUTEX_NAME);
    shm_unlink(SHM_NAME);
    log_db();
    if (logf) fclose(logf);
}

/* ───────────────── Main ───────────────── */
int main(int argc,char*argv[])
{
    if (argc!=2){ fprintf(stderr,"Usage: %s <ServerFIFO>\n",argv[0]); exit(1);}
    strncpy(server_fifo, argv[1], sizeof(server_fifo)-1);

    /* FIFOs */
    mkfifo(server_fifo,0666);
    int srvfd = open(server_fifo,O_RDONLY|O_NONBLOCK);
    int dummy = open(server_fifo,O_WRONLY); /* keep open */

    /* log */
    logf = fopen(SERVER_LOG,"a+");

    shm_sem_init();

    /* signal */
    struct sigaction sa={.sa_handler=die}; sigaction(SIGINT,&sa,NULL);

    printf("BankServer AdaBank %s\nAdaBank is active…\n",server_fifo);

    char buf[512];
 while (!stop) {
        ssize_t n = read(srvfd, buf, sizeof(buf)-1);
        if (n > 0) {
            buf[n] = '\0';

            /* multiple lines may have arrived at once */
            char *save = NULL;
            for (char *line = strtok_r(buf, "\n", &save);
                 line;
                 line = strtok_r(NULL, "\n", &save))
            {
                char op; char idtok[64]; long amt; char fifo[128];
                if (sscanf(line, " %c %63s %ld %127s",
                           &op, idtok, &amt, fifo) != 4)
                    continue;                       /* malformed */

                /* translate account-token */
                int acc_id = -1;
                if (idtok[0] == 'N' || idtok[0] == 'n') {
                    acc_id = -1;
                } else if (!strncmp(idtok, "BankID_", 7)) {
                    acc_id = atoi(idtok + 7);
                } else {
                    acc_id = atoi(idtok);
                }

                extern void launch_teller_process(char,int,long,const char*);
                launch_teller_process(op, acc_id, amt, fifo);
            }
        } else {
            /* no FIFO input – still check for finished requests */
            usleep(20 * 1000);
        }
        process_requests();                           /* NEW – live apply */
     }

    /* graceful exit: drain the queue once more */
    process_requests();
    printf("Signal received… shutting down.\n");
    cleanup();
    return 0;
}

/* ───────────────── Server consumer thread (runs in parent) ─────────── */
    __attribute__((destructor))
    static void ring_consumer(void)
    {
        if (!shm_ready) return; 

        /* we run after main, but in same process */
        while(!stop || sem_trywait(full)==0){
            sem_wait(full);
            sem_wait(mutex);

            size_t slot = ring->tail % BUFSZ;
            BankRequest rq = ring->req[slot];

            BankReply rp={.status=0,.account_id=rq.account_id,.balance=-1};

            int idx = (rq.account_id==-1)?-1:find_acc(rq.account_id);
            if (rq.op=='D' || rq.op=='d'){
                if (idx==-1) {                     /* new account */
                    int id=new_account(rq.amount);
                    rp.account_id=id; rp.balance=rq.amount;
                }else{
                    db[idx].bal+=rq.amount;
                    rp.balance=db[idx].bal;
                }
            }else{ /* withdraw */
                if (idx==-1 || db[idx].bal<rq.amount){
                    rp.status=-1;
                }else{
                    db[idx].bal-=rq.amount;
                    rp.balance=db[idx].bal;
                    if (db[idx].bal==0){           /* close account */
                        db[idx]=db[--db_used];
                    }
                }
            }
            ring->rep[slot]=rp;
            ring->tail++;

            sem_post(mutex);
            /* wake teller */
            kill(rq.teller_pid,SIGUSR1);
        }
    }
