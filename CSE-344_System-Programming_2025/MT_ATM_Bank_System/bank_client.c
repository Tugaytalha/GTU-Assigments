/* bank_client.c  — minimal tweak: read transactions from a file
 * Usage:  BankClient  <client-file | ->  <ServerFIFO>
 */
#include "common.h"

static void usage(const char *p)
{
    fprintf(stderr, "Usage: %s <ClientFile|-> <ServerFIFO>\n", p);
    exit(EXIT_FAILURE);
}

int main(int argc, char **argv)
{
    if (argc != 3) usage(argv[0]);

    const char *client_path = argv[1];
    const char *srv_fifo    = argv[2];

    /* 2. open the client script (or use stdin if "-" given) */
    printf("Reading %s\n", client_path);
    FILE *in = strcmp(client_path, "-") == 0 ? stdin : fopen(client_path, "r");
    if (!in) die("open client file");

    /* 1. open the server FIFO for writing */
    int fd = open(srv_fifo, O_WRONLY);
    if (fd < 0) die("open fifo");


    /* 3. unchanged parsing / sending loop ---------------------------- */
    char line[MAX_LINE];
    while (fgets(line, sizeof line, in)) {
        txn_t t = { .id = -1, .client = getpid() };
        char  op[16];

        if (sscanf(line, "N %15s %d", op, &t.amount) == 2)             /* new acct */
            t.id = -1;
        else if (sscanf(line, "BankID_%d %15s %d",
                        &t.id, op, &t.amount) != 3)
            continue;                                                  /* bad line */

        t.op = (strncmp(op, "deposit", 7) == 0) ? DEPOSIT : WITHDRAW;
        if (write(fd, &t, sizeof t) != sizeof t)
            perror("write");
    }

    if (in != stdin) fclose(in);
    close(fd);
    return 0;
}
