#include "common.h"
#include "teller.h"

static acct_t accounts[MAX_ACCOUNTS];
static int    deposits   [MAX_ACCOUNTS];
static int    withdrawals[MAX_ACCOUNTS];

static int    fifo_fd   = -1;
static const char *fifo_name;

/* ---------- load existing accounts from log if present ---------- */
static void load_accounts(void)
{
    FILE *f = fopen("AdaBank.bankLog", "r");
    if (!f) return;

    char line[MAX_LINE];
    while (fgets(line, sizeof line, f)) {
        if (line[0] == '#' || line[0] == '\n') continue;

        int id, d = 0, w = 0, bal = 0;
        /* new rich format */
        if (sscanf(line, "BankID_%d D %d W %d %d", &id, &d, &w, &bal) == 4) {
            if (id >= 1 && id < MAX_ACCOUNTS) {
                deposits[id]    = d;
                withdrawals[id] = w;
                accounts[id].balance = bal;
            }
        }
        /* old simple format (balance only) */
        else if (sscanf(line, "BankID_%d %d", &id, &bal) == 2) {
            if (id >= 1 && id < MAX_ACCOUNTS)
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

    for (int i = 1; i < MAX_ACCOUNTS; ++i) {
        if (accounts[i].balance)
            fprintf(f, "BankID_%02d D %d W %d %d\n",
                    i, deposits[i], withdrawals[i], accounts[i].balance);
    }
    fprintf(f, "## end of log.\n");
    fclose(f);
}

/* ---------- main loop ---------- */
int main(int argc, char **argv)
{
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <ServerFIFO>\n", argv[0]);
        return 1;
    }
    fifo_name = argv[1];

    say("BankServer AdaBank  #"); say(fifo_name); say("\n");

    load_accounts();
    if (deposits[1] == 0 && withdrawals[1] == 0 && accounts[1].balance == 0)
        say("No previous logs … Creating the bank database\n");
    else
        say("Previous log found … Continuing with existing database\n");

    signal(SIGINT, cleanup);
    mkfifo(fifo_name, 0666);
    fifo_fd = open(fifo_name, O_RDONLY);
    if (fifo_fd < 0) die("first open the fifo");

    say("Waiting for client transactions.\n");

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
        sayf("- Received client from PID %d …\n", req.client);

        /* --- one Teller ------------------------------------------------ */
        pid_t pid = (req.op == DEPOSIT)
                      ? Teller(deposit,  &req)
                      : Teller(withdraw, &req);
        shared_t *shm = teller_shared();

        sem_wait(&shm->sem_client);     /* wait for request */

        /* --- apply transaction ---------------------------------------- */
        txn_t *tx = &shm->txn;
        int ok = 0;

        if (tx->id == -1) {             /* allocate new account */
            for (int i = 1; i < MAX_ACCOUNTS; ++i)
                if (!deposits[i] && !withdrawals[i] && !accounts[i].balance) {
                    tx->id = i; break;
                }
        }

        if (tx->id >= 1 && tx->id < MAX_ACCOUNTS) {
            acct_t *a = &accounts[tx->id];
            if (tx->op == DEPOSIT) {
                a->balance      += tx->amount;
                deposits[tx->id] += tx->amount;
                ok = 1;
            } else if (tx->op == WITHDRAW && a->balance >= tx->amount) {
                a->balance         -= tx->amount;
                withdrawals[tx->id] += tx->amount;
                ok = 1;
            }
        }

        shm->result = ok;
        sem_post(&shm->sem_server);     /* wake Teller */

        if (ok && tx->op == DEPOSIT)
            sayf2("Client%02d deposited %d credits … updating log\n",
                   tx->id, tx->amount);
        else if (ok && tx->op == WITHDRAW)
            sayf2("Client%02d withdraws %d credits … updating log\n",
                   tx->id, tx->amount);
        else
            say("Operation not permitted. No account or no money.\n");

        waitTeller(pid, NULL);
        log_accounts();
    }
}
