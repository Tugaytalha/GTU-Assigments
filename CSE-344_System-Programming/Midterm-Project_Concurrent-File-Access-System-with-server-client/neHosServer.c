#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>

#define MAX_CLIENTS 10

int client_count = 0;
pid_t client_pids[MAX_CLIENTS];
char *dirname;

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

void handle_client(int client_fd) {
    char buffer[1024];
    while (1) {
        // Receive command from client
        int bytes_read = read(client_fd, buffer, sizeof(buffer));
        if (bytes_read <= 0) {
            break; // Client disconnected
        }

        // Parse and execute command
        // ... (Implement command handling logic here)

        // Send response to client
        // ... (Implement response sending logic here)
    }

    close(client_fd);
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

    // Create directory if it doesn't exist
    mkdir(dirname, 0755);

    // Create log file
    char log_filename[1024];
    snprintf(log_filename, sizeof(log_filename), "%s/server.log", dirname);
    int log_fd = open(log_filename, O_CREAT | O_WRONLY | O_APPEND, 0644);
    if (log_fd == -1) {
        perror("open");
        exit(1);
    }

    // Register signal handler
    signal(SIGINT, handle_sigint);

    // Print server information
    printf("Server started. PID: %d\n", getpid());
    printf("Waiting for clients...\n");

    while (1) {
        // Accept client connection
        int client_fd = accept(server_fd, NULL, NULL);
        if (client_fd == -1) {
            perror("accept");
            continue;
        }

        // Check if maximum client limit reached
        if (client_count >= max_clients) {
            // Send error message to client and close connection
            write(client_fd, "Server queue full\n", 17);
            close(client_fd);
            continue;
        }

        // Fork a child process to handle the client
        pid_t pid = fork();
        if (pid == -1) {
            perror("fork");
            close(client_fd);
            continue;
        } else if (pid == 0) {
            // Child process
            close(server_fd); // Child doesn't need the server socket
            handle_client(client_fd);
            exit(0);
        } else {
            // Parent process
            close(client_fd); // Parent doesn't need the client socket
            client_pids[client_count++] = pid;
            printf("Client PID %d connected.\n", pid);
        }
    }

    // Close log file
    close(log_fd);

    return 0;
}