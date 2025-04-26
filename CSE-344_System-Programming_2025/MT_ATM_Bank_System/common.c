/* common.c */
#include "common.h"

/* --- tiny write-only helpers (avoid stdio buffering) ----------------- */
static void say(const char *msg)
{
    write(STDOUT_FILENO, msg, strlen(msg));
}
static void sayf(const char *fmt, int n)
{
    char buf[128];
    int  len = snprintf(buf, sizeof buf, fmt, n);
    write(STDOUT_FILENO, buf, len);
}
