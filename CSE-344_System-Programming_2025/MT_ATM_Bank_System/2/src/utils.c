#define _POSIX_C_SOURCE 200809L
#include "utils.h"

ssize_t read_line_fd(int fd, char *buf, size_t maxlen)
/* Reads from *fd* into *buf* (NUL-terminated) until newline or EOF.
 * Returns number of bytes placed in *buf* (excluding NUL) or −1 on error. */
{
    if (maxlen == 0) return 0;
    size_t n = 0;
    while (n < maxlen - 1)
    {
        char c;
        ssize_t r = read(fd, &c, 1);
        if (r == 0) break;                    /* EOF */
        if (r == -1) return -1;               /* errno set */
        if (c == '\n') break;
        buf[n++] = c;
    }
    buf[n] = '\0';
    return (ssize_t)n;
}
