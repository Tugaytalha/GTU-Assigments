#include "common.h"
#include <pthread.h>

// Bank database
typedef struct {
    char bank_id[MAX_BANK_ID_LEN];
    int balance;
    int active;
} BankAccount;

typedef struct {
    BankAccount accounts[MAX_CLIENTS];
    int num_accounts;
    pthread_mutex_t mutex;
} BankDatabase;

// Shared memory key
#define SHM_KEY 12345
#define SEM_KEY_SERVER 54321
#define SEM_KEY_TELLER 65432

BankDatabase database;
char server_fifo_name[MAX_BUFFER];
char bank_name[MAX_BUFFER];
char log_file_name[MAX_BUFFER];
int running = 1;
int server_fifo_fd = -1;

// Shared memory ID
int shmid = -1;
SharedData *shared_data = NULL;

// Semaphores
sem_t *server_sem = NULL;
sem_t *teller_sem = NULL;

// Forward declarations
void init_database();
void load_database_from_log();
void update_log_file();
void handle_signals(int signo);
void cleanup_resources();
int create_bank_id(char *bank_id);
int find_account_by_id(const char *bank_id);
int deposit(const char *bank_id, int amount, int client_id);
int withdraw(const char *bank_id, int amount, int client_id);

// Advanced process creation
pid_t Teller(void *func, void *arg_func);
int waitTeller(pid_t pid, int *status);

// Process functions for deposit and withdraw
void *deposit_func(void *arg);
void *withdraw_func(void *arg);

typedef struct {
    int client_id;
    ClientRequest request;
} TellerArgs;

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <bank_name> <server_fifo_name>\n", argv[0]);
        return 1;
    }

    // Initialize
    strcpy(bank_name, argv[1]);
    strcpy(server_fifo_name, argv[2]);
    sprintf(log_file_name, "%s.bankLog", bank_name);

    // Set up signal handlers
    signal(SIGINT, handle_signals);
    signal(SIGTERM, handle_signals);

    // Initialize bank database
    init_database();

    // Create shared memory
    shmid = shmget(SHM_KEY, sizeof(SharedData), IPC_CREAT | 0666);
    if (shmid == -1) {
        perror("shmget");
        return 1;
    }

    // Attach shared memory
    shared_data = (SharedData *)shmat(shmid, NULL, 0);
    if (shared_data == (void *)-1) {
        perror("shmat");
        return 1;
    }

    // Initialize shared memory
    memset(shared_data, 0, sizeof(SharedData));

    // Create semaphores
    server_sem = sem_open("/server_sem", O_CREAT, 0666, 0);
    teller_sem = sem_open("/teller_sem", O_CREAT, 0666, 0);

    if (server_sem == SEM_FAILED || teller_sem == SEM_FAILED) {
        perror("sem_open");
        cleanup_resources();
        return 1;
    }

    printf("%s is active....\n", bank_name);
    
    // Create server FIFO
    if (mkfifo(server_fifo_name, 0666) == -1 && errno != EEXIST) {
        perror("mkfifo");
        cleanup_resources();
        return 1;
    }

    printf("Waiting for clients @%s...\n", server_fifo_name);

    // Open server FIFO
    server_fifo_fd = open(server_fifo_name, O_RDONLY);
    if (server_fifo_fd == -1) {
        perror("open");
        unlink(server_fifo_name);
        cleanup_resources();
        return 1;
    }

    // Main server loop
    while (running) {
        // Read client requests from server FIFO
        ClientRequest request;
        int bytes_read = read(server_fifo_fd, &request, sizeof(ClientRequest));
        
        if (bytes_read == sizeof(ClientRequest)) {
            printf("- Received client from PID%s\n", request.client_fifo + strlen(CLIENT_FIFO_PREFIX));
            
            // Create teller args
            TellerArgs *args = (TellerArgs *)malloc(sizeof(TellerArgs));
            args->client_id = database.num_accounts;
            memcpy(&args->request, &request, sizeof(ClientRequest));
            
            // Create teller process based on request type
            pid_t pid;
            if (request.type == REQUEST_DEPOSIT) {
                pid = Teller(deposit_func, args);
            } else {
                pid = Teller(withdraw_func, args);
            }
            
            if (pid > 0) {
                printf("-- Teller PID%d is active serving Client%d...", pid, args->client_id + 1);
                
                // Check if this is a returning client
                if (strlen(request.bank_id) > 0 && strcmp(request.bank_id, "N") != 0) {
                    int idx = find_account_by_id(request.bank_id);
                    if (idx != -1) {
                        printf("Welcome back Client%d\n", args->client_id + 1);
                    } else {
                        printf("\n");
                    }
                } else {
                    printf("\n");
                }
                
                // Wait for teller to complete
                int status;
                waitTeller(pid, &status);
            }
        }
        else if (bytes_read == 0) {
            // FIFO closed, reopen it
            close(server_fifo_fd);
            server_fifo_fd = open(server_fifo_name, O_RDONLY);
            if (server_fifo_fd == -1) {
                perror("reopen");
                break;
            }
        }
        else if (bytes_read == -1) {
            if (errno != EINTR) {
                perror("read");
                break;
            }
        }
    }

    cleanup_resources();
    printf("%s says \"Bye\"...\n", bank_name);
    return 0;
}

void init_database() {
    memset(&database, 0, sizeof(BankDatabase));
    pthread_mutex_init(&database.mutex, NULL);
    
    // Try to load from log file
    FILE *log = fopen(log_file_name, "r");
    if (log) {
        printf("Loading existing database from log...\n");
        fclose(log);
        load_database_from_log();
    } else {
        printf("No previous logs.. Creating the bank database\n");
    }
}

void load_database_from_log() {
    FILE *log = fopen(log_file_name, "r");
    if (!log) return;

    char line[MAX_LOG_LINE];
    while (fgets(line, MAX_LOG_LINE, log)) {
        // Skip comment lines
        if (line[0] == '#') continue;
        
        // Extract bank_id and balance
        char bank_id[MAX_BANK_ID_LEN];
        int final_balance = 0;
        
        // Simple parsing - assuming format "BankID_XX ... XXXX" where the last number is the balance
        char *token = strtok(line, " ");
        if (token) {
            strcpy(bank_id, token);
            
            // Extract all operations and final balance (last number in the line)
            char *last_token = token;
            while ((token = strtok(NULL, " \t\n"))) {
                if (token[0] != 'D' && token[0] != 'W') {
                    last_token = token;
                }
            }
            
            if (last_token != bank_id) {
                final_balance = atoi(last_token);
                
                // If account has positive balance, add it to database
                if (final_balance > 0) {
                    pthread_mutex_lock(&database.mutex);
                    strcpy(database.accounts[database.num_accounts].bank_id, bank_id);
                    database.accounts[database.num_accounts].balance = final_balance;
                    database.accounts[database.num_accounts].active = 1;
                    database.num_accounts++;
                    pthread_mutex_unlock(&database.mutex);
                }
            }
        }
    }
    
    fclose(log);
}

void update_log_file() {
    FILE *log = fopen(log_file_name, "w");
    if (!log) {
        perror("fopen");
        return;
    }
    
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    fprintf(log, "# %s Log file updated @%02d:%02d %s %d %d\n", 
            bank_name, t->tm_hour, t->tm_min, 
            (t->tm_mon == 0) ? "January" : 
            (t->tm_mon == 1) ? "February" : 
            (t->tm_mon == 2) ? "March" : 
            (t->tm_mon == 3) ? "April" : 
            (t->tm_mon == 4) ? "May" : 
            (t->tm_mon == 5) ? "June" : 
            (t->tm_mon == 6) ? "July" : 
            (t->tm_mon == 7) ? "August" : 
            (t->tm_mon == 8) ? "September" : 
            (t->tm_mon == 9) ? "October" : 
            (t->tm_mon == 10) ? "November" : "December",
            t->tm_mday, t->tm_year + 1900);
    
    pthread_mutex_lock(&database.mutex);
    for (int i = 0; i < database.num_accounts; i++) {
        if (database.accounts[i].active && database.accounts[i].balance > 0) {
            fprintf(log, "%s D %d %d\n", 
                    database.accounts[i].bank_id, 
                    database.accounts[i].balance, 
                    database.accounts[i].balance);
        }
    }
    pthread_mutex_unlock(&database.mutex);
    
    fprintf(log, "## end of log.\n");
    fclose(log);
}

void handle_signals(int signo) {
    printf("\nSignal received closing active Tellers\n");
    running = 0;
}

void cleanup_resources() {
    if (server_fifo_fd != -1) {
        close(server_fifo_fd);
    }
    
    printf("Removing ServerFIFO... Updating log file...\n");
    unlink(server_fifo_name);
    update_log_file();
    
    // Clean up shared memory
    if (shared_data != NULL && shared_data != (void *)-1) {
        shmdt(shared_data);
    }
    
    if (shmid != -1) {
        shmctl(shmid, IPC_RMID, NULL);
    }
    
    // Clean up semaphores
    if (server_sem != NULL && server_sem != SEM_FAILED) {
        sem_close(server_sem);
        sem_unlink("/server_sem");
    }
    
    if (teller_sem != NULL && teller_sem != SEM_FAILED) {
        sem_close(teller_sem);
        sem_unlink("/teller_sem");
    }
    
    pthread_mutex_destroy(&database.mutex);
}

int create_bank_id(char *bank_id) {
    static int next_id = 1;
    
    pthread_mutex_lock(&database.mutex);
    
    // Find the next available ID
    while (1) {
        sprintf(bank_id, "BankID_%d", next_id);
        int found = 0;
        
        for (int i = 0; i < database.num_accounts; i++) {
            if (database.accounts[i].active && 
                strcmp(database.accounts[i].bank_id, bank_id) == 0) {
                found = 1;
                break;
            }
        }
        
        if (!found) break;
        next_id++;
    }
    
    // Create new account
    int idx = database.num_accounts++;
    strcpy(database.accounts[idx].bank_id, bank_id);
    database.accounts[idx].balance = 0;
    database.accounts[idx].active = 1;
    
    pthread_mutex_unlock(&database.mutex);
    
    return idx;
}

int find_account_by_id(const char *bank_id) {
    pthread_mutex_lock(&database.mutex);
    
    for (int i = 0; i < database.num_accounts; i++) {
        if (database.accounts[i].active && 
            strcmp(database.accounts[i].bank_id, bank_id) == 0) {
            pthread_mutex_unlock(&database.mutex);
            return i;
        }
    }
    
    pthread_mutex_unlock(&database.mutex);
    return -1;
}

int deposit(const char *bank_id, int amount, int client_id) {
    if (amount <= 0) return ERR_INVALID_AMOUNT;
    
    int idx;
    char new_bank_id[MAX_BANK_ID_LEN];
    
    // New client case
    if (bank_id[0] == '\0' || strcmp(bank_id, "N") == 0) {
        idx = create_bank_id(new_bank_id);
        pthread_mutex_lock(&database.mutex);
        strcpy(shared_data->bank_id, new_bank_id);
    }
    else {
        idx = find_account_by_id(bank_id);
        if (idx == -1) return ERR_INVALID_BANKID;
        pthread_mutex_lock(&database.mutex);
        strcpy(shared_data->bank_id, bank_id);
    }
    
    // Update balance
    database.accounts[idx].balance += amount;
    int balance = database.accounts[idx].balance;
    
    pthread_mutex_unlock(&database.mutex);
    
    // Update log
    printf("Client%d deposited %d credits... updating log\n", client_id + 1, amount);
    update_log_file();
    
    return balance;
}

int withdraw(const char *bank_id, int amount, int client_id) {
    if (amount <= 0) return ERR_INVALID_AMOUNT;
    if (bank_id[0] == '\0' || strcmp(bank_id, "N") == 0) return ERR_INVALID_BANKID;
    
    int idx = find_account_by_id(bank_id);
    if (idx == -1) return ERR_INVALID_BANKID;
    
    pthread_mutex_lock(&database.mutex);
    
    // Check if there are sufficient funds
    if (database.accounts[idx].balance < amount) {
        pthread_mutex_unlock(&database.mutex);
        printf("Client%d withdraws %d credit.. operation not permitted.\n", client_id + 1, amount);
        return ERR_INSUFFICIENT_FUNDS;
    }
    
    // Update balance
    database.accounts[idx].balance -= amount;
    int balance = database.accounts[idx].balance;
    
    // Check if account is now empty
    if (balance == 0) {
        database.accounts[idx].active = 0;
        printf("Client%d withdraws %d credits... updating log... Bye Client%d\n", client_id + 1, amount, client_id + 1);
    } else {
        printf("Client%d withdraws %d credits... updating log\n", client_id + 1, amount);
    }
    
    pthread_mutex_unlock(&database.mutex);
    
    // Update log
    update_log_file();
    
    return balance;
}

pid_t Teller(void *func, void *arg_func) {
    pid_t pid = fork();
    
    if (pid == 0) {
        // Child process executes the function
        void *(*function)(void *) = func;
        function(arg_func);
        exit(0);
    }
    
    return pid;
}

int waitTeller(pid_t pid, int *status) {
    return waitpid(pid, status, 0);
}

void *deposit_func(void *arg) {
    TellerArgs *args = (TellerArgs *)arg;
    ClientRequest *request = &args->request;
    TellerResponse response;
    memset(&response, 0, sizeof(TellerResponse));
    
    // Open client FIFO for writing
    int client_fifo_fd = open(request->client_fifo, O_WRONLY);
    if (client_fifo_fd == -1) {
        perror("open client fifo");
        free(args);
        return NULL;
    }
    
    // Initialize shared memory for this operation
    shared_data->client_id = args->client_id;
    shared_data->type = REQUEST_DEPOSIT;
    shared_data->amount = request->amount;
    strcpy(shared_data->bank_id, request->bank_id);
    shared_data->status = 0;
    
    // Execute deposit operation
    int result = deposit(request->bank_id, request->amount, args->client_id);
    
    // Set response based on operation result
    if (result >= 0) {
        response.status = 0;
        response.balance = result;
        shared_data->current_balance = result;
        
        // For new clients, fill the bank_id from shared memory
        if (request->bank_id[0] == '\0' || strcmp(request->bank_id, "N") == 0) {
            strcpy(response.bank_id, shared_data->bank_id);
            sprintf(response.message, "Deposit successful. New account created with ID %s", response.bank_id);
        } else {
            strcpy(response.bank_id, request->bank_id);
            sprintf(response.message, "Deposit successful. New balance: %d", result);
        }
    } else {
        response.status = result;
        shared_data->status = result;
        switch (result) {
            case ERR_INVALID_AMOUNT:
                strcpy(response.message, "Invalid amount for deposit");
                break;
            case ERR_INVALID_BANKID:
                strcpy(response.message, "Invalid bank ID");
                break;
            default:
                strcpy(response.message, "Unknown error");
                break;
        }
    }
    
    // Send response back to client
    write(client_fifo_fd, &response, sizeof(TellerResponse));
    close(client_fifo_fd);
    
    free(args);
    return NULL;
}

void *withdraw_func(void *arg) {
    TellerArgs *args = (TellerArgs *)arg;
    ClientRequest *request = &args->request;
    TellerResponse response;
    memset(&response, 0, sizeof(TellerResponse));
    
    // Open client FIFO for writing
    int client_fifo_fd = open(request->client_fifo, O_WRONLY);
    if (client_fifo_fd == -1) {
        perror("open client fifo");
        free(args);
        return NULL;
    }
    
    // Initialize shared memory for this operation
    shared_data->client_id = args->client_id;
    shared_data->type = REQUEST_WITHDRAW;
    shared_data->amount = request->amount;
    strcpy(shared_data->bank_id, request->bank_id);
    shared_data->status = 0;
    
    // Execute withdraw operation
    int result = withdraw(request->bank_id, request->amount, args->client_id);
    
    // Set response based on operation result
    if (result >= 0) {
        response.status = 0;
        response.balance = result;
        shared_data->current_balance = result;
        strcpy(response.bank_id, request->bank_id);
        
        if (result == 0) {
            sprintf(response.message, "Withdrawal successful. Account closed.");
        } else {
            sprintf(response.message, "Withdrawal successful. New balance: %d", result);
        }
    } else {
        response.status = result;
        shared_data->status = result;
        switch (result) {
            case ERR_INSUFFICIENT_FUNDS:
                strcpy(response.message, "Insufficient funds");
                break;
            case ERR_INVALID_AMOUNT:
                strcpy(response.message, "Invalid amount for withdrawal");
                break;
            case ERR_INVALID_BANKID:
                strcpy(response.message, "Invalid bank ID or new client cannot withdraw");
                break;
            default:
                strcpy(response.message, "Unknown error");
                break;
        }
    }
    
    // Send response back to client
    write(client_fifo_fd, &response, sizeof(TellerResponse));
    close(client_fifo_fd);
    
    free(args);
    return NULL;
} 