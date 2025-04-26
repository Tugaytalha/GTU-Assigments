#include "common.h"
#include "teller.h"

static acct_t accounts[MAX_ACCOUNTS];
static int    fifo_fd   = -1;
static const char *fifo_name;

/* ---------- housekeeping ---------- */
static void cleanup(int signo)
{
    (void)signo;
    if (fifo_fd != -1) close(fifo_fd);
    if (fifo_name)     unlink(fifo_name);
    puts("Adabank shutting down. Bye!");
    exit(EXIT_SUCCESS);
}

static void log_accounts(void)
{
    FILE *f = fopen("AdaBank.bankLog", "w");
    if (!f) die("Cannot open log file");
    time_t now = time(NULL);
    fprintf(f, "# Adabank Log file updated @%s", ctime(&now));
    for (int i = 0; i < MAX_ACCOUNTS; ++i)
        if (accounts[i].balance)
            fprintf(f, "BankID_%02d %d\n", i, accounts[i].balance);
    fclose(f);
}

/* TODO: Implement our own version of printf() with write, without using stdio.h funtions*/


/* ---------- main loop ---------- */
int main(int argc, char **argv)
{
    if (argc != 2) { fprintf(stderr, "Usage: %s <ServerFIFO>\n", argv[0]); return 1; }
    fifo_name = argv[1];

    signal(SIGINT, cleanup);
    printf("Adabank is activating…");
    mkfifo(fifo_name, 0666);
    fifo_fd = open(fifo_name, O_RDONLY);
    if (fifo_fd < 0) die("first open the fifo");
    puts("Waiting for client transactions.");


    while (1) {
        txn_t req;
        ssize_t n = read(fifo_fd, &req, sizeof req);

        if (n == 0) {                   /* writer closed ⇒ reopen FIFO   */
            close(fifo_fd);
            fifo_fd = open(fifo_name, O_RDONLY);
            continue;
        } else if (n != sizeof req) {
            if (errno == EINTR) continue;
            perror("read"); continue;
        }

        /* 1. spawn a Teller for exactly *one* request ----------------- */
        pid_t pid = (req.op == DEPOSIT)
                      ? Teller(deposit,  &req)
                      : Teller(withdraw, &req);
        shared_t *shm = teller_shared();

        /* 2. wait for Teller to drop the request into shared mem ------ */
        sem_wait(&shm->sem_client);

        /* 3. critical section: apply the transaction ------------------ */
        txn_t *tx = &shm->txn;
        int ok = 0;

        if (tx->id == -1) {             /* opening a new account        */
            for (int i = 1; i < MAX_ACCOUNTS; ++i)
                if (!accounts[i].balance) { tx->id = i; break; }
        }

        if (tx->id >= 0 && tx->id < MAX_ACCOUNTS) {
            acct_t *a = &accounts[tx->id];
            if (tx->op == DEPOSIT) {
                a->balance += tx->amount;
                ok = 1;
            } else if (tx->op == WITHDRAW && a->balance >= tx->amount) {
                a->balance -= tx->amount;
                ok = 1;
            }
        }

        /* 4. wake Teller up with the verdict -------------------------- */
        shm->result = ok;
        sem_post(&shm->sem_server);

        /* 5. clean up ------------------------------------------------- */
        waitTeller(pid, NULL);
        log_accounts();
    }
}
