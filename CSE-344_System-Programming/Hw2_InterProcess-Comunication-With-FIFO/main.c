#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>

#define FIFO1 "fifo1"
#define FIFO2 "fifo2"
#define MAX_LENGTH 256

#define ERR_FIFO_CREATE 1
#define ERR_FIFO_OPEN 2
#define ERR_FIFO_WRITE 3
#define ERR_FIFO_READ 4
#define ERR_FIFO_CLOSE 5

#define ERR_FORK 10

// Define global atomatic child process count
sig_atomic_t child_count = 0;

// Signal handler for SIGCHLD
void sigchld_handler(int signum) {
    int status;
    pid_t pid;

    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        if (WIFEXITED(status)) {
            printf("Child process %d exited with status: %d\n", pid, WEXITSTATUS(status));
            ++child_count;
        }
    }
}

// Function to convert a string to lowercase
void to_lower(char *str) {
    for (int i = 0; str[i] != '\0'; i++) {
        if (str[i] >= 'A' && str[i] <= 'Z') {
            str[i] += 32;
        }
    }
}

// Takes an integer as argument and use it as size of the random number array.
int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <integer>\n", argv[0]);
        return 1;
    }

    // Seed random number generator
    srand(time(NULL));

    // Create FIFOs
    if (mkfifo(FIFO1, 0666) == -1 || mkfifo(FIFO2, 0666) == -1) {
        perror("Error creating FIFOs");
        return ERR_FIFO_CREATE;
    }

    // Fork child processes
    pid_t pid1 = fork();
    if (pid1 == -1) {
        perror("Error forking first child process");
        return ERR_FORK;
    }
    else if (pid1 == 0) {
        // Child Process 1
        int sum = 0;
        int fd_read = open(FIFO1, O_RDONLY);

        // Sleep for 10 seconds
        sleep(10);

        if (fd_read == -1) {
            perror("Error opening FIFO1 for reading in Child Process 1");
            _exit(ERR_FIFO_OPEN);
        }
        if (read(fd_read, numbers, sizeof(numbers)) == -1) {
            perror("Error reading from FIFO1 in Child Process 1");
            _exit(ERR_FIFO_READ);
        }
        close(fd_read);

        for (int i = 0; i < array_size; i++) {
            sum += numbers[i];
        }

        // Open FIFO2 for writing
        int fd_write = open(FIFO2, O_WRONLY);
        if (fd_write == -1) {
            perror("Error opening FIFO2 for writing in Child Process 1");
            _exit(ERR_FIFO_OPEN);
        }
        if (write(fd_write, &sum, sizeof(sum)) == -1) {
            perror("Error writing to FIFO2 in Child Process 1");
            _exit(ERR_FIFO_WRITE);
        }
        close(fd_write);

        _exit(EXIT_SUCCESS);
    } else {
        pid_t pid2 = fork();
        if (pid2 == -1) {
            perror("Error forking second child process");
            return ERR_FORK;
        }
        else if (pid2 == 0) {
            // Child Process 2
            char received_command[MAX_LENGTH];
            int fd_read = open(FIFO2, O_RDONLY);
            if (fd_read == -1) {
                perror("Error opening FIFO2 for reading in Child Process 2");
                _exit(ERR_FIFO_OPEN);
            }
            if (read(fd_read, received_command, sizeof(received_command)) == -1) {
                perror("Error reading from FIFO2 in Child Process 2");
                _exit(ERR_FIFO_READ);
            }

            if (strcmp(received_command, "multiply") != 0) {
                fprintf(stderr, "Error: Invalid command received in Child Process 2\n");
                return 1;
            }

            // Print readed numbers and command
            printf("Received numbers: ");
            for (int i = 0; i < array_size; i++) {
                printf("%d ", numbers[i]);
            }
            printf("\nCommand: %s\n", received_command);

            int product = 1;
            for (int i = 0; i < array_size; i++) {
                product *= numbers[i];
            }

            // Read other child's result from FIFO2
            int sum;
            if (read(fd_read, &sum, sizeof(sum)) == -1) {
                perror("Error reading from FIFO2 in Child Process 2");
                _exit(ERR_FIFO_READ);
            }
            close(fd_read);

            printf("Sum of results from all child processes: %d\n", product+sum);

            _exit(EXIT_SUCCESS);
        }
        else {
            // Parent Process
            // Set SIGCHLD handler
            signal(SIGCHLD, sigchld_handler);

            // Open FIFOs for writing and reading
            int fd1 = open(FIFO1, O_WRONLY);
            int fd2 = open(FIFO2, O_WRONLY);

            if (fd1 == -1 || fd2 == -1) {
                perror("Error opening FIFOs for writing");
                return ERR_FIFO_OPEN;
            }

            array_size = atoi(argv[1]);
            // Write random numbers to FIFO1
            int numbers[array_size];
            for (int i = 0; i < array_size; i++) {
                numbers[i] = rand() % 10;
            }

            if (write(fd1, numbers, sizeof(numbers)) == -1) {
                perror("Error writing to FIFO1");
                return ERR_FIFO_WRITE;
            }

            // Write random numbers and command and to FIFO2
            char command[] = "multiply";
            if (write(fd2, numbers, sizeof(numbers)) == -1) {
                perror("Error writing random ints to FIFO2");
                return ERR_FIFO_WRITE;
            }
            if (write(fd2, command, sizeof(command)) == -1) {
                perror("Error writing command to FIFO2");
                return ERR_FIFO_WRITE;
            }


            int child_count = 0;
            while (1) {
                printf("Proceeding...\n");
                sleep(2);
                // Check if all children have exited
                if (waitpid(pid1, NULL, WNOHANG) > 0 && waitpid(pid2, NULL, WNOHANG) > 0) {
                    break;
                }
            }
            // Close FIFOs
            close(fd1);
            close(fd2);

            // Remove FIFOs
            unlink(FIFO1);
            unlink(FIFO2);

            return 0;
        }
    }
}
