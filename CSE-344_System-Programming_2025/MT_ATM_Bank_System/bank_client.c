#include "common.h"

static void usage(const char *p) { fprintf(stderr, "Usage: %s <ServerFIFO>\n", p); exit(1); }

int main(int argc, char **argv)
{
    if (argc != 2) usage(argv[0]);
    const char *srv = argv[1];

    int fd = open(srv, O_WRONLY);
    if (fd < 0) die("open fifo");

    char line[MAX_LINE];
    while (fgets(line, sizeof line, stdin)) {
        txn_t t = { .id = -1, .client = getpid() };
        char op[8];

        if (sscanf(line, "N %7s %d", op, &t.amount) == 2)              /* new acct */
            t.id = -1;
        else if (sscanf(line, "BankID_%d %7s %d", &t.id, op, &t.amount) != 3)
            continue;                                                  /* bad line  */

        t.op = (strncmp(op, "deposit", 7) == 0) ? DEPOSIT : WITHDRAW;
        write(fd, &t, sizeof t);
    }
    close(fd);
    return 0;
}
