#define _POSIX_C_SOURCE 200809L
#include "bank_common.h"
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <ctype.h>

static char server_fifo[128];
static void child(const char *line)
{
    char op[8], idtok[32]; long amt;
    if (sscanf(line, "%7s %31s %ld", op, idtok, &amt) != 3) _exit(1);

    char opcode = toupper((unsigned char)op[0]);
    if (opcode != 'D' && opcode != 'W') _exit(1);

    char myfifo[128]; snprintf(myfifo,sizeof(myfifo),"/tmp/bankcli_%d",getpid());
    mkfifo(myfifo,0666);

    int srv = open(server_fifo,O_WRONLY);
    dprintf(srv,"%c %s %ld %s\n", opcode, idtok, amt, myfifo);
    close(srv);

    int fd = open(myfifo,O_RDONLY);
    char ans[128]; ssize_t n=read(fd,ans,sizeof(ans)-1);
    if (n>0){ ans[n]=0; printf("Client[%d] %s",getpid(),ans); }
    close(fd); unlink(myfifo);
    _exit(0);
}
int main(int argc,char*argv[])
{
    if (argc!=3){fprintf(stderr,"Usage: %s <script.file> <ServerFIFO>\n",argv[0]);return 1;}
    strncpy(server_fifo,argv[2],sizeof(server_fifo)-1);

    FILE *f=fopen(argv[1],"r"); if(!f){perror("open script");return 1;}

    char *line=NULL; size_t len=0;
    while(getline(&line,&len,f)>0){
        pid_t p=fork();
        if(p==0) child(line);
    }
    free(line); fclose(f);

    while(wait(NULL)>0);
    return 0;
}
