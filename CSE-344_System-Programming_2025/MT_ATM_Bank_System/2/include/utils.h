#ifndef ADABANK_UTILS_H
#define ADABANK_UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdint.h>

/*- fatal() ─ exit with message and errno string -*/
static inline void fatal(const char *ctx)
{
    fprintf(stderr, "Fatal: %s: %s\n", ctx, strerror(errno));
    exit(EXIT_FAILURE);
}

/*- xmkfifo() ─ create FIFO or bail out */
static inline void xmkfifo(const char *path, mode_t perm)
{
    if (mkfifo(path, perm) == -1 && errno != EEXIST)
        fatal("mkfifo");
}

/*- xopen() for common flags -*/
static inline int xopen(const char *path, int flags, mode_t perm)
{
    int fd = open(path, flags, perm);
    if (fd == -1) fatal("open");
    return fd;
}

/*- read_line_fd() – portable, stops at '\n' or EOF */
ssize_t read_line_fd(int fd, char *buf, size_t maxlen);

#endif /* ADABANK_UTILS_H */
