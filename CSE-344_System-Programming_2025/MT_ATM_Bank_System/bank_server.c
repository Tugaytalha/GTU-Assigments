/* bank_server.c */
#include "common.h"
#include "teller.h"

static acct_t accounts[MAX_ACCOUNTS];
static int    fifo_fd   = -1;
static const char *fifo_name;


/* ---------- load existing accounts from log if present ---------- */
static void load_accounts(void)
{
    FILE *f = fopen("AdaBank.bankLog", "r");
    if (!f) return;
    char line[MAX_LINE];
    while (fgets(line, sizeof line, f)) {
        if (line[0] == '#' || line[0] == '\n')
            continue;
        int id, bal;
        if (sscanf(line, "BankID_%d %d", &id, &bal) == 2
            && id >= 1 && id < MAX_ACCOUNTS) {
            accounts[id].balance = bal;
        }
    }
    fclose(f);
}

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
    for (int i = 1; i < MAX_ACCOUNTS; ++i)
        if (accounts[i].balance)
            fprintf(f, "BankID_%02d %d\n", i, accounts[i].balance);
    fclose(f);
}

/* ---------- main loop ---------- */
int main(int argc, char **argv)
{
    if (argc != 2) { fprintf(stderr, "Usage: %s <ServerFIFO>\n", argv[0]); return 1; }
    fifo_name = argv[1];

    say("BankServer AdaBank  #" );
    say(fifo_name);
    say("\nAdabank is active …\n");

    /* load from previous log if exists */
    load_accounts();
    if (accounts[0].balance == 0)
        say("No previous logs … Creating the bank database\n");

    signal(SIGINT, cleanup);
    mkfifo(fifo_name, 0666);
    fifo_fd = open(fifo_name, O_RDONLY);
    if (fifo_fd < 0) die("first open the fifo");
    puts("Waiting for client transactions.");

    while (1) {
        txn_t req;
        ssize_t n = read(fifo_fd, &req, sizeof req);
        sayf("- Received client from PID %d …\n", req.client);

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

        if (tx->id >= 1 && tx->id < MAX_ACCOUNTS) {
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

        if (ok && tx->op == DEPOSIT)
            sayf2("Client%02d deposited %d credits … updating log\n",
                tx->id, tx->amount);
        else if (ok && tx->op == WITHDRAW)
            sayf2("Client%02d withdraws %d credits … updating log\n",
                tx->id, tx->amount);
        else
            say("Operation not permitted.\n");


        /* 5. clean up ------------------------------------------------- */
        waitTeller(pid, NULL);
        log_accounts();
    }
}
