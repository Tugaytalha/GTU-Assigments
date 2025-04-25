/*─────────────────────────────────────────────────────────────
 * client.c  –  front-end that spawns one worker process per
 *              script line and communicates with AdaBank.
 * Fixed 25-Apr-2025: correct token order “account op amount”.
 *────────────────────────────────────────────────────────────*/
#define _POSIX_C_SOURCE 200809L
#include "bank_common.h"

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

static char server_fifo[128];

static void child(const char *line)
{
    /*  <AccountID | N> <deposit|withdraw> <Amount>  */
    char acctok[64], opword[16];
    long amt;

    if (sscanf(line, " %63s %15s %ld", acctok, opword, &amt) != 3)
        _exit(1);

    /* normalise opcode */
    char opcode = toupper((unsigned char)opword[0]);
    if (opcode != 'D' && opcode != 'W')          /* ignore weird verbs */
        _exit(1);

    /* create a private FIFO */
    char myfifo[128];
    snprintf(myfifo, sizeof(myfifo), "/tmp/bankcli_%d", getpid());
    mkfifo(myfifo, 0666);

    /* send request line to server FIFO */
    int srv = open(server_fifo, O_WRONLY);
    dprintf(srv, "%c %s %ld %s\n", opcode, acctok, amt, myfifo);
    close(srv);

    /* wait for answer */
    int fd = open(myfifo, O_RDONLY);
    char ans[128];
    ssize_t n = read(fd, ans, sizeof(ans) - 1);
    if (n > 0) {
        ans[n] = '\0';
        printf("Client[%d] %s", getpid(), ans);
    }
    close(fd);
    unlink(myfifo);
    _exit(0);
}

int main(int argc, char *argv[])
{
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <script.file> <ServerFIFO>\n", argv[0]);
        return 1;
    }
    strncpy(server_fifo, argv[2], sizeof(server_fifo) - 1);

    FILE *f = fopen(argv[1], "r");
    if (!f) { perror("open script"); return 1; }

    char *line = NULL; size_t len = 0;
    while (getline(&line, &len, f) > 0) {
        if (fork() == 0) child(line);
    }
    free(line);
    fclose(f);

    while (wait(NULL) > 0) /* reap */ ;
    return 0;
}
