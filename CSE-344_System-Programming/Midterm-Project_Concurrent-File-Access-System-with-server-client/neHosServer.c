#include "neHos.h"

int client_count = 0;
pid_t client_pids[MAX_CLIENTS];
char *dirname;
volatile sig_atomic_t running = 1;


void handle_sigint(int sig) {
    // Send kill signal to all clients
    for (int i = 0; i < client_count; i++) {
        kill(client_pids[i], SIGKILL);
    }

    // Wait for all clients to terminate
    while (wait(NULL) > 0);

    printf("\nServer shutting down...\n");
    exit(0);
}

void handle_client(pid_t client_pid) {
    char buffer[MAX_LENGTH];
    char client_fifo_r[MAX_LENGTH], client_fifo_w[MAX_LENGTH];

    // Open client FIFO (previously created by client)
    snprintf(client_fifo_r, CLIENT_FIFO_NAME_LEN, CLIENT_FIFO_TEMPLATE1, (long) client_pid);
    snprintf(client_fifo_w, CLIENT_FIFO_NAME_LEN, CLIENT_FIFO_TEMPLATE2, (long) client_pid);
    int clientFdR = open(client_fifo_r, O_RDONLY);
    if (clientFd == -1) {
        perror("open");
        exit(1);
    }
    int clientFdW = open(client_fifo_w, O_WRONLY);
    if (clientFd == -1) {
        perror("open");
        exit(1);
    }
    
    while (1) {
        // Receive command from client
        int bytes_read = read(clientFd, buffer, sizeof(buffer));
        if (bytes_read <= 0) {
            break; // Client disconnected
        }

        // Parse and execute command
        // ... (Implement command handling logic here)

        // Send response to client
        // ... (Implement response sending logic here)
    }

    close(clientFd);
    client_count--;

    // Remove client PID from array
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (client_pids[i] == getpid()) {
            client_pids[i] = 0;
            break;
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <dirname> <max_clients>\n", argv[0]);
        exit(1);
    }

    dirname = argv[1];
    int max_clients = atoi(argv[2]);
    int serverFd, clientFd;
    char clientFifo[MAX_LENGTH];
    char buffer[MAX_LENGTH];
    struct request req;
    struct response resp;
    int seqNum = 0;

    // Create directory if it doesn't exist
    mkdir(dirname, 0777);

    umask(0);    /* We get the permissions we want */
    if (mkfifo(SERVER_FIFO, S_IRUSR | S_IWUSR | S_IWGRP) == -1
            && errno != EEXIST){
        perror("mkfifo error");
        exit(ERR_FIFO_CREATE);
    }
    serverFd = open(SERVER_FIFO, O_RDONLY);
    if (serverFd == -1){
        perror("open error");
        exit(ERR_FIFO_OPEN);
    }

    if (signal(SIGPIPE, SIG_IGN) == SIG_ERR){
        perror("signal error");
        exit(ERR_SIGNAL);
    }

    // Create log file
    char log_filename[1024];
    // Concatenate the dirname with "/server.log" manually
    strcpy(log_filename, dirname);
    strcat(log_filename, "/server.log");
    int log_fd = open(log_filename, O_CREAT | O_WRONLY | O_APPEND, 0644);
    if (log_fd == -1) {
        perror("Log file open error");
        exit(ERR_LOG_CREATE);
    }

    // Register signal handler
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = &handle_sigint;
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("Error setting SIGINT handler");
        exit(1);
    }

    // Print server information
    printf("Server started. PID: %d\n", getpid());
    printf("Waiting for clients...\n");

    while (running) {

        if (read(serverFd, &req, sizeof(struct request)) != sizeof(struct request)) {
            fprintf(stderr, "Error reading request; discarding\n");
            continue; 
        }



        // Open client FIFO (previously created by client) 
        snprintf(clientFifo, CLIENT_FIFO_NAME_LEN, CLIENT_FIFO_TEMPLATE1, (long) req.pid);
        clientFd = open(clientFifo, O_WRONLY);
        if (clientFd == -1) { /* Open failed, give up on client */
            errMsg("open %s", clientFifo);
            continue;
        }
        
        
        
        
        resp.accepted = 1;
        // Check if maximum client limit reached
        if (client_count >= max_clients) {
            // Send error message to client and close connection
            resp.accepted = 0;
            if (write(clientFd, &resp, sizeof(struct response)) != sizeof(struct response))
                fprintf(stderr, "Error writing to FIFO %s\n", clientFifo);
            close(clientFd);
            continue;
        }

        // Send response to client
        if (write(clientFd, &resp, sizeof(struct response)) != sizeof(struct response))
            fprintf(stderr, "Error writing to FIFO %s\n", clientFifo);
        if (close(clientFd) == -1)
            errMsg("close");

        // Fork a child process to handle the client
        pid_t pid = fork();
        if (pid == -1) {
            perror("fork");
            continue;
        } else if (pid == 0) {
            // Child process
            close(serverFd); // Child doesn't need the server FIFO
            handle_client(req.pid);
            _exit(0);
        } else {
            // Parent process
            client_pids[client_count++] = pid;
            printf("Client PID %d connected.\n", pid);
        }
    }

    // Close log file
    close(log_fd);

    unlink(SERVER_FIFO);

    return 0;
}