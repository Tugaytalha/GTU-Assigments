#include "neHosServer.h"

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

void handle_client(int clientFd) {
    char buffer[1024];
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
    int serverFd, clientFd dummy_fd;
    struct sockaddr_in server_addr, client_addr;
    char client_fifo[MAX_LENGTH];
    char buffer[MAX_LENGTH];

    // Create directory if it doesn't exist
    mkdir(dirname, 0777);
    

    // Create log file
    char log_filename[1024];
    // Concatenate the dirname with "/server.log" manually
    strcpy(log_filename, dirname);
    strcat(log_filename, "/server.log");
    int log_fd = open(log_filename, O_CREAT | O_WRONLY | O_APPEND, 0644);
    if (log_fd == -1) {
        perror("Log file open error");
        exit(LOG_CREATE_ERROR);
    }

    // Create a socket
    serverFd = socket(AF_INET, SOCK_STREAM, 0);
    if (serverFd < 0) {
        perror("socket creation failed");
        exit(SOCKET_CREATE_ERROR);
    }

    // Set server address details
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY; // Listen on all available interfaces
    server_addr.sin_port = htons(PORT);

    // Bind the socket to the address and port
    if (bind(serverFd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind failed");
        exit(SOCKET_BIND_ERROR);
    }

    // Register signal handler
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = &handle_sigint;
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("Error setting SIGINT handler");
        exit(1);
    }

    // Listen for incoming connections
    if (listen(serverFd, BACKLOG) < 0) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    // Print server information
    printf("Server started. PID: %d\n", getpid());
    printf("Waiting for clients...\n");

    while (running) {
        // Accept client connection
        int clientFd = accept(serverFd, NULL, NULL);
        if (clientFd == -1) {
            perror("Client connection error");
            continue;
        }

        // Check if maximum client limit reached
        if (client_count >= max_clients) {
            // Send error message to client and close connection
            write(clientFd, "Server queue full\n", 17);
            close(clientFd);
            continue;
        }

        // Fork a child process to handle the client
        pid_t pid = fork();
        if (pid == -1) {
            perror("fork");
            close(clientFd);
            continue;
        } else if (pid == 0) {
            // Child process
            close(serverFd); // Child doesn't need the server socket
            handle_client(clientFd);
            _exit(0);
        } else {
            // Parent process
            close(clientFd); // Parent doesn't need the client socket
            client_pids[client_count++] = pid;
            printf("Client PID %d connected.\n", pid);
        }
    }

    // Close log file
    close(log_fd);

    return 0;
}