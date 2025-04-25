#define _POSIX_C_SOURCE 200809L
#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>

#define MAX_LINE 256

static const char *server_fifo      = SERVER_FIFO_FILE;
static const char *scenario_name    = NULL;      /* for logs only   */
static int         server_fd_write  = -1;

/*Per-child client logic*/
static void run_single_client(char *request_line)
{
    pid_t   me   = getpid();
    char    pipe_name[64];
    snprintf(pipe_name, sizeof(pipe_name), CLIENT_FIFO_TMPL, (int)me);

    /* Build private FIFO */
    if (mkfifo(pipe_name, FIFO_PERM) == -1)
    {
        perror("mkfifo client");
        _exit(EXIT_FAILURE);
    }

    /* 1. Announce to the server */
    char announce[MAX_LINE];
    snprintf(announce, sizeof(announce),
             "%d %s 1\n", (int)me, pipe_name);

    if (write(server_fd_write, announce, strlen(announce)) == -1)
    {
        perror("write SERVER_FIFO");
        unlink(pipe_name);
        _exit(EXIT_FAILURE);
    }

    /* 2. Open our private pipe (same fd for RDWR simplifies half-duplex) */
    int pfd = open(pipe_name, O_RDWR);
    if (pfd == -1) { perror("open client fifo"); unlink(pipe_name); _exit(EXIT_FAILURE); }

    /* 3. Send the request line to the Teller */
    size_t len = strlen(request_line);
    if (request_line[len-1] != '\n') { request_line[len++] = '\n'; request_line[len] = '\0'; }

    if (write(pfd, request_line, len) != (ssize_t)len)
    {
        perror("write request");
        close(pfd); unlink(pipe_name); _exit(EXIT_FAILURE);
    }

    /* 4. Wait for single-line response */
    char resp[MAX_LINE] = {0};
    ssize_t rd = read(pfd, resp, sizeof(resp)-1);
    if (rd <= 0)
        fprintf(stderr, "Client %d: no response / broken pipe\n", (int)me);
    else
        printf("Client%05d  %s", (int)me, resp);      /* already \n-terminated */

    close(pfd);
    unlink(pipe_name);
    _exit(0);
}

/*Scenario-file parsing helper*/
static int read_scenario(FILE *fp, char ***lines_out)
{
    size_t cap = 16, sz = 0;
    char **lines = calloc(cap, sizeof(char*));
    if (!lines) return -1;

    char buf[MAX_LINE];
    while (fgets(buf, sizeof(buf), fp))
    {
        if (buf[0] == '\n' || buf[0] == '#') continue;        /* skip blanks/comments */
        if (sz == cap)
        {
            cap *= 2;
            char **tmp = realloc(lines, cap * sizeof(char*));
            if (!tmp) { perror("realloc"); exit(EXIT_FAILURE); }
            lines = tmp;
        }
        lines[sz++] = strdup(buf);        /* keep trailing NL; child may trim */
    }
    *lines_out = lines;
    return (int)sz;
}

/*main                                 */
int main(int argc, char *argv])
{
    if (argc < 2 || argc > 3)
    {
        fprintf(stderr, "Usage: %s <scenario_file|- for stdin> [server_fifo]\n",
                argv[0]);
        return EXIT_FAILURE;
    }

    if (argc == 3) server_fifo = argv[2];
    scenario_name = argv[1];

    /* 1. Open scenario */
    FILE *scn = NULL;
    if (strcmp(scenario_name, "-") == 0)
        scn = stdin;
    else
        scn = fopen(scenario_name, "r");
    if (!scn) { perror("open scenario"); return EXIT_FAILURE; }

    /* 2. Slurp lines */
    char **lines = NULL;
    int    nlines = read_scenario(scn, &lines);
    if (nlines < 0) { fprintf(stderr, "read scenario failed\n"); return EXIT_FAILURE; }
    if (scn != stdin) fclose(scn);

    if (nlines == 0) { puts("No requests – nothing to do."); return EXIT_SUCCESS; }

    printf("Reading %s …\n%d clients to connect … creating clients …\n",
           scenario_name, nlines);

    /* 3. Open SERVER_FIFO write-end once (shared by children)      */
    server_fd_write = open(server_fifo, O_WRONLY | O_NONBLOCK);
    if (server_fd_write == -1)
    {
        fprintf(stderr, "Cannot connect %s … exiting.\n", server_fifo);
        return EXIT_FAILURE;
    }

    /* 4. Fork one child per request line */
    for (int i = 0; i < nlines; ++i)
    {
        pid_t pid = fork();
        if (pid == -1) { perror("fork"); continue; }

        if (pid == 0)   /* child */
        {
            run_single_client(lines[i]);
        }
        /* parent → continue */
    }

    /* 5. Parent: wait all children, report status */
    int status;
    while (wait(&status) > 0) ;    /* reap everybody */

    close(server_fd_write);
    for (int i = 0; i < nlines; ++i) free(lines[i]);
    free(lines);

    puts("exiting …");
    return EXIT_SUCCESS;
}
