/*─────────────────────────────────────────────────────────────
 * server.c  –  AdaBank main server  (debug version)
 * Fully fixed & instrumented 25-Apr-2025
 *────────────────────────────────────────────────────────────*/
#define _POSIX_C_SOURCE 200809L
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
#include <time.h>
#include <unistd.h>

#ifndef DBG
#define DBG 1
#endif
#define PDBG(fmt, ...) do{ if(DBG)fprintf(stderr,"[DBG] "fmt"\n",##__VA_ARGS__);}while(0)

/*────────── Bank-side DB ─────────*/
typedef struct { int id; long bal; } Account;
static Account db[MAX_ACC];
static size_t  db_used = 0;

static int  find_acc(int id){ for(size_t i=0;i<db_used;++i) if(db[i].id==id) return (int)i; return -1; }
static int  new_account(long x){ int id=(db_used?db[db_used-1].id+1:1); db[db_used++] = (Account){id,x}; return id; }

/*────────── Logging ─────────*/
static FILE *logf=NULL;
static void log_db(void){
    if(!logf) return;
    char ts[64];
    time_t t=time(NULL); struct tm tm; localtime_r(&t,&tm);
    strftime(ts,sizeof ts,"%H:%M %B %d %Y",&tm);
    fprintf(logf,"# Adabank Log file updated @%s\n",ts);
    for(size_t i=0;i<db_used;++i)
        fprintf(logf,"BankID_%02d %ld\n",db[i].id,db[i].bal);
    fprintf(logf,"## end of log.\n"); fflush(logf);
}

/*────────── Ring-buffer IPC ─────────*/
static ShmRing *ring; static sem_t *empty,*full,*mutex;
static void shm_sem_init(void){
    int fd=shm_open(SHM_NAME,O_CREAT|O_RDWR,0666); ftruncate(fd,sizeof*ring);
    ring=mmap(NULL,sizeof*ring,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0); close(fd);
    memset(ring,0,sizeof*ring);
    empty=sem_open(SEM_EMPTY_NAME,O_CREAT,0666,BUFSZ);
    full =sem_open(SEM_FULL_NAME ,O_CREAT,0666,0);
    mutex=sem_open(SEM_MUTEX_NAME,O_CREAT,0666,1);
}

/* consume every ready request */
static void process_requests(void){
    while(sem_trywait(full)==0){
        sem_wait(mutex);
        size_t slot = ring->tail % BUFSZ;
        BankRequest rq = ring->req[slot];
        BankReply   rp = { .status=0, .account_id=rq.account_id, .balance=-1 };

        int idx = (rq.account_id==-1)?-1:find_acc(rq.account_id);
        if(rq.op=='D'||rq.op=='d'){
            if(idx==-1){
                int id=new_account(rq.amount);
                rp.account_id=id; rp.balance=rq.amount;
                PDBG("Deposit %ld → new BankID_%02d",rq.amount,id);
            }else{
                db[idx].bal += rq.amount; rp.balance=db[idx].bal;
                PDBG("Deposit %ld → BankID_%02d balance %ld",rq.amount,db[idx].id,rp.balance);
            }
        }else{ /* withdraw */
            if(idx==-1||db[idx].bal<rq.amount){
                rp.status=-1; PDBG("Withdraw %ld FAIL id=%d",rq.amount,rq.account_id);
            }else{
                db[idx].bal-=rq.amount; rp.balance=db[idx].bal;
                PDBG("Withdraw %ld → BankID_%02d balance %ld",rq.amount,db[idx].id,rp.balance);
                if(db[idx].bal==0){ PDBG("BankID_%02d closed",db[idx].id); db[idx]=db[--db_used]; }
            }
        }
        ring->rep[slot]=rp; ring->tail++;
        sem_post(mutex); sem_post(empty);
        kill(rq.teller_pid,SIGUSR1);
    }
}

/*────────── Shutdown handling ─────────*/
static char server_fifo[128]; static volatile sig_atomic_t stop=0;
static void die(int s){(void)s;stop=1;}
static void cleanup(void){
    unlink(server_fifo);
    sem_unlink(SEM_EMPTY_NAME); sem_unlink(SEM_FULL_NAME); sem_unlink(SEM_MUTEX_NAME);
    shm_unlink(SHM_NAME); log_db(); if(logf)fclose(logf);
}

/*────────── main ─────────*/
int main(int argc,char*argv[]){
    if(argc!=2){fprintf(stderr,"Usage: %s <ServerFIFO>\n",argv[0]);return 1;}
    strncpy(server_fifo,argv[1],sizeof server_fifo-1);

    unlink(server_fifo); mkfifo(server_fifo,0666);
    int srvfd=open(server_fifo,O_RDONLY|O_NONBLOCK);
    int keep =open(server_fifo,O_WRONLY); (void)keep; /* keep writer open */

    logf=fopen(SERVER_LOG,"a+");
    shm_sem_init();
    struct sigaction sa={.sa_handler=die}; sigaction(SIGINT,&sa,NULL);

    printf("BankServer AdaBank %s\nAdaBank is active…\n",server_fifo);

    char buf[512];
    while(!stop){
        ssize_t n=read(srvfd,buf,sizeof buf-1);
        if(n>0){
            buf[n]='\0';
            char *save=NULL;
            for(char*line=strtok_r(buf,"\n",&save);line;line=strtok_r(NULL,"\n",&save)){
                char op,idtok[64],fifo[128]; long amt;
                if(sscanf(line," %c %63s %ld %127s",&op,idtok,&amt,fifo)!=4){PDBG("Bad line %s",line);continue;}
                int acc=-1;
                if(idtok[0]!='N'&&idtok[0]!='n'){
                    acc=!strncmp(idtok,"BankID_",7)?atoi(idtok+7):atoi(idtok);
                }
                PDBG("REQ op=%c acc=%d amt=%ld fifo=%s",op,acc,amt,fifo);
                extern void launch_teller_process(char,int,long,const char*);
                launch_teller_process(op,acc,amt,fifo);
            }
        }else usleep(20*1000);
        process_requests();
    }
    process_requests(); printf("Signal received… shutting down.\n"); cleanup(); return 0;
}
