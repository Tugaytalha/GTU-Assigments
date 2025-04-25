#include "common.h"
#include <pthread.h>

#define MAX_CLIENT_LINES 50
#define MAX_LINE_LENGTH 256

typedef struct {
    int client_id;
    char client_fifo[MAX_BUFFER];
    char bank_id[MAX_BANK_ID_LEN];
    int type;
    int amount;
} ClientInfo;

typedef struct {
    char line[MAX_LINE_LENGTH];
    int processed;
} ClientLine;

// Global variables
char server_fifo_name[MAX_BUFFER];
int running = 1;
ClientLine client_lines[MAX_CLIENT_LINES];
int num_client_lines = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

// Forward declarations
void handle_signals(int signo);
void *client_thread(void *arg);
int parse_client_line(const char *line, char *bank_id, int *type, int *amount);

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <client_file> <server_fifo_name>\n", argv[0]);
        return 1;
    }

    // Set up signal handlers
    signal(SIGINT, handle_signals);
    signal(SIGTERM, handle_signals);

    // Open client file
    FILE *client_file = fopen(argv[1], "r");
    if (!client_file) {
        perror("fopen");
        return 1;
    }

    printf("Reading %s..\n", argv[1]);

    // Read client file
    char line[MAX_LINE_LENGTH];
    while (fgets(line, MAX_LINE_LENGTH, client_file) && num_client_lines < MAX_CLIENT_LINES) {
        // Remove newline character
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n') {
            line[len - 1] = '\0';
        }

        // Skip empty lines
        if (strlen(line) == 0) {
            continue;
        }

        strcpy(client_lines[num_client_lines].line, line);
        client_lines[num_client_lines].processed = 0;
        num_client_lines++;
    }

    fclose(client_file);

    printf("%d clients to connect.. creating clients..\n", num_client_lines);

    // Store server FIFO name
    strcpy(server_fifo_name, argv[2]);

    // Check if server FIFO exists
    struct stat st;
    if (stat(server_fifo_name, &st) == -1) {
        printf("Cannot connect %s...\n", server_fifo_name);
        printf("exiting..\n");
        return 1;
    }

    printf("Connected to Adabank..\n");

    // Create threads for each client
    pthread_t threads[MAX_CLIENT_LINES];
    for (int i = 0; i < num_client_lines; i++) {
        ClientInfo *info = (ClientInfo *)malloc(sizeof(ClientInfo));
        info->client_id = i + 1;
        sprintf(info->client_fifo, "%s%d", CLIENT_FIFO_PREFIX, info->client_id);

        // Create client FIFO
        if (mkfifo(info->client_fifo, 0666) == -1 && errno != EEXIST) {
            perror("mkfifo");
            free(info);
            continue;
        }

        // Parse client line
        if (parse_client_line(client_lines[i].line, info->bank_id, &info->type, &info->amount) != 0) {
            printf("Invalid client line: %s\n", client_lines[i].line);
            free(info);
            continue;
        }

        // Create client thread
        if (pthread_create(&threads[i], NULL, client_thread, info) != 0) {
            perror("pthread_create");
            free(info);
            continue;
        }
    }

    // Wait for all threads to complete
    for (int i = 0; i < num_client_lines; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("exiting..\n");
    
    // Clean up client FIFOs
    for (int i = 0; i < num_client_lines; i++) {
        char client_fifo[MAX_BUFFER];
        sprintf(client_fifo, "%s%d", CLIENT_FIFO_PREFIX, i + 1);
        unlink(client_fifo);
    }

    pthread_mutex_destroy(&mutex);
    return 0;
}

void handle_signals(int signo) {
    printf("\nSignal received, exiting...\n");
    running = 0;
}

void *client_thread(void *arg) {
    ClientInfo *info = (ClientInfo *)arg;
    int server_fd = -1;
    int client_fd = -1;

    if (!running) {
        free(info);
        return NULL;
    }

    // Connection message
    printf("Client%d connected..%s %d credits\n", 
           info->client_id, 
           (info->type == REQUEST_DEPOSIT) ? "depositing" : "withdrawing", 
           info->amount);

    // Mark client line as processed
    pthread_mutex_lock(&mutex);
    client_lines[info->client_id - 1].processed = 1;
    pthread_mutex_unlock(&mutex);

    // Open server FIFO
    server_fd = open(server_fifo_name, O_WRONLY);
    if (server_fd == -1) {
        perror("open server fifo");
        free(info);
        return NULL;
    }

    // Prepare client request
    ClientRequest request;
    memset(&request, 0, sizeof(ClientRequest));
    request.type = info->type;
    request.amount = info->amount;
    strcpy(request.bank_id, info->bank_id);
    strcpy(request.client_fifo, info->client_fifo);

    // Send request to server
    if (write(server_fd, &request, sizeof(ClientRequest)) != sizeof(ClientRequest)) {
        perror("write");
        close(server_fd);
        free(info);
        return NULL;
    }

    close(server_fd);

    // Open client FIFO for reading
    client_fd = open(info->client_fifo, O_RDONLY);
    if (client_fd == -1) {
        perror("open client fifo");
        free(info);
        return NULL;
    }

    // Read response from teller
    TellerResponse response;
    memset(&response, 0, sizeof(TellerResponse));
    
    if (read(client_fd, &response, sizeof(TellerResponse)) != sizeof(TellerResponse)) {
        perror("read");
        close(client_fd);
        free(info);
        return NULL;
    }

    close(client_fd);

    // Process response
    if (response.status == 0) {
        if (info->type == REQUEST_DEPOSIT && 
            (info->bank_id[0] == '\0' || strcmp(info->bank_id, "N") == 0)) {
            printf("Client%d served.. %s\n", info->client_id, response.bank_id);
        }
        else if (info->type == REQUEST_WITHDRAW && response.balance == 0) {
            printf("Client%d served.. account closed\n", info->client_id);
        }
        else {
            printf("Client%d served.. balance: %d\n", info->client_id, response.balance);
        }
    }
    else {
        printf("Client%d something went WRONG\n", info->client_id);
    }

    free(info);
    return NULL;
}

int parse_client_line(const char *line, char *bank_id, int *type, int *amount) {
    char operation[20];
    int result;

    result = sscanf(line, "%s %s %d", bank_id, operation, amount);
    if (result != 3) {
        return -1;
    }

    if (strcmp(operation, "deposit") == 0) {
        *type = REQUEST_DEPOSIT;
    }
    else if (strcmp(operation, "withdraw") == 0) {
        *type = REQUEST_WITHDRAW;
    }
    else {
        return -1;
    }

    return 0;
} 