#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>

#ifdef _WIN32
#error "This program requires Linux/Unix and is not compatible with Windows. Please use WSL or a Linux VM."
#endif

#define FIFO1_PATH "./fifo1"
#define FIFO2_PATH "./fifo2"
#define LOG_FILE "./daemon.log"
#define TIMEOUT_SECONDS 15

// Global variables
int childCounter = 0;
int totalChildren = 2;
int daemonPid = 0;
pid_t child1_pid = 0;
pid_t child2_pid = 0;
time_t child1_last_active = 0;
time_t child2_last_active = 0;

// Signal handler for SIGCHLD
void sigchld_handler(int sig) {
    int status;
    pid_t pid;
    
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        if (WIFEXITED(status)) {
            printf("Child process %d exited with status %d\n", pid, WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            printf("Child process %d killed by signal %d\n", pid, WTERMSIG(status));
        }
        childCounter += 2;
    }
}

// Signal handlers for daemon process
void sigusr1_handler(int sig) {
    FILE *log = fopen(LOG_FILE, "a");
    if (log) {
        fprintf(log, "[%ld] Received SIGUSR1 signal\n", time(NULL));
        fclose(log);
    }
}

void sighup_handler(int sig) {
    FILE *log = fopen(LOG_FILE, "a");
    if (log) {
        fprintf(log, "[%ld] Received SIGHUP signal - reconfiguring\n", time(NULL));
        fclose(log);
    }
}

void sigterm_handler(int sig) {
    FILE *log = fopen(LOG_FILE, "a");
    if (log) {
        fprintf(log, "[%ld] Received SIGTERM signal - shutting down daemon\n", time(NULL));
        fclose(log);
    }
    exit(0);
}

// Function to daemonize the process
void daemonize() {
    pid_t pid, sid;

    // Fork the parent process
    pid = fork();
    if (pid < 0) {
        perror("Failed to fork daemon process");
        exit(EXIT_FAILURE);
    }
    
    // Exit the parent process
    if (pid > 0) {
        daemonPid = pid;
        return;
    }

    // Set new session
    sid = setsid();
    if (sid < 0) {
        perror("Failed to create new session");
        exit(EXIT_FAILURE);
    }

    // Change directory to root or current directory
    // For the assignment, we'll stay in the current directory for easier file management
    // if (chdir("/") < 0) {
    //     exit(EXIT_FAILURE);
    // }

    // Close standard file descriptors
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    // Open log file for output
    int log_fd = open(LOG_FILE, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (log_fd < 0) {
        exit(EXIT_FAILURE);
    }

    // Redirect stdout and stderr to log file
    dup2(log_fd, STDOUT_FILENO);
    dup2(log_fd, STDERR_FILENO);
    close(log_fd);

    // Set signal handlers
    signal(SIGUSR1, sigusr1_handler);
    signal(SIGHUP, sighup_handler);
    signal(SIGTERM, sigterm_handler);

    // Log daemon startup
    printf("[%ld] Daemon started with PID: %d\n", time(NULL), getpid());
    printf("[%ld] Monitoring child processes: Child1=%d, Child2=%d\n", 
           time(NULL), child1_pid, child2_pid);

    // Store initial activity times
    child1_last_active = time(NULL);
    child2_last_active = time(NULL);

    // Monitor child processes
    while (1) {
        time_t current_time = time(NULL);
        
        // Check if processes are still running
        if (child1_pid > 0) {
            if (kill(child1_pid, 0) == 0) {
                // Process exists, check timeout
                if (current_time - child1_last_active > TIMEOUT_SECONDS) {
                    printf("[%ld] Child1 (PID %d) inactive for %ld seconds, terminating...\n", 
                           current_time, child1_pid, current_time - child1_last_active);
                    kill(child1_pid, SIGTERM);
                }
            } else if (errno == ESRCH) {
                // Process does not exist
                printf("[%ld] Child1 (PID %d) no longer exists\n", current_time, child1_pid);
                child1_pid = 0;
            }
        }
        
        if (child2_pid > 0) {
            if (kill(child2_pid, 0) == 0) {
                // Process exists, check timeout
                if (current_time - child2_last_active > TIMEOUT_SECONDS) {
                    printf("[%ld] Child2 (PID %d) inactive for %ld seconds, terminating...\n", 
                           current_time, child2_pid, current_time - child2_last_active);
                    kill(child2_pid, SIGTERM);
                }
            } else if (errno == ESRCH) {
                // Process does not exist
                printf("[%ld] Child2 (PID %d) no longer exists\n", current_time, child2_pid);
                child2_pid = 0;
            }
        }
        
        // Log status periodically
        printf("[%ld] Daemon monitoring child processes\n", current_time);
        
        sleep(5);
    }
}

// Function to set a file descriptor to non-blocking mode
int set_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) {
        return -1;
    }
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

// Function to read from a non-blocking file descriptor with timeout
ssize_t read_with_timeout(int fd, void *buf, size_t count, int timeout_sec) {
    fd_set readfds;
    struct timeval tv;
    
    FD_ZERO(&readfds);
    FD_SET(fd, &readfds);
    
    tv.tv_sec = timeout_sec;
    tv.tv_usec = 0;
    
    int ret = select(fd + 1, &readfds, NULL, NULL, &tv);
    
    if (ret == -1) {
        return -1;  // Error occurred
    } else if (ret == 0) {
        errno = ETIMEDOUT;
        return -1;  // Timeout occurred
    } else {
        return read(fd, buf, count);
    }
}

int main(int argc, char *argv[]) {
    // Check command line arguments
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <int1> <int2>\n", argv[0]);
        return 1;
    }

    // Parse the two integers
    int num1 = atoi(argv[1]);
    int num2 = atoi(argv[2]);

    // Variable to hold result
    int result = 0;

    // Create FIFOs
    if (mkfifo(FIFO1_PATH, 0666) == -1 && errno != EEXIST) {
        perror("Error creating FIFO1");
        return 1;
    }
    
    if (mkfifo(FIFO2_PATH, 0666) == -1 && errno != EEXIST) {
        perror("Error creating FIFO2");
        unlink(FIFO1_PATH);
        return 1;
    }

    // Setup signal handler for SIGCHLD
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("Error setting up SIGCHLD handler");
        unlink(FIFO1_PATH);
        unlink(FIFO2_PATH);
        return 1;
    }

    // Create first child process
    pid_t child1 = fork();
    
    if (child1 < 0) {
        perror("Failed to fork child1");
        unlink(FIFO1_PATH);
        unlink(FIFO2_PATH);
        return 1;
    } else if (child1 == 0) {
        // This is Child Process 1
        printf("Child 1 (PID %d) started\n", getpid());
        
        // Sleep for 10 seconds
        sleep(10);
        
        // Open the first FIFO to read the integers
        int fd1 = open(FIFO1_PATH, O_RDONLY);
        if (fd1 == -1) {
            perror("Child 1: Failed to open FIFO1");
            exit(1);
        }
        
        // Set non-blocking mode
        if (set_nonblocking(fd1) == -1) {
            perror("Child 1: Failed to set non-blocking mode");
            close(fd1);
            exit(1);
        }
        
        int nums[2];
        ssize_t bytes_read = read_with_timeout(fd1, nums, sizeof(nums), 5);
        if (bytes_read != sizeof(nums)) {
            if (errno == ETIMEDOUT) {
                fprintf(stderr, "Child 1: Timeout reading from FIFO1\n");
            } else {
                perror("Child 1: Failed to read from FIFO1");
            }
            close(fd1);
            exit(1);
        }
        
        close(fd1);
        
        // Determine the larger number
        int larger = (nums[0] > nums[1]) ? nums[0] : nums[1];
        
        // Write the larger number to the second FIFO
        int fd2 = open(FIFO2_PATH, O_WRONLY);
        if (fd2 == -1) {
            perror("Child 1: Failed to open FIFO2");
            exit(1);
        }
        
        if (write(fd2, &larger, sizeof(larger)) != sizeof(larger)) {
            perror("Child 1: Failed to write to FIFO2");
            close(fd2);
            exit(1);
        }
        
        close(fd2);
        printf("Child 1 (PID %d) completed task - determined larger number: %d\n", getpid(), larger);
        exit(0);
    }

    // Store the child1 PID for monitoring
    child1_pid = child1;

    // Create second child process
    pid_t child2 = fork();
    
    if (child2 < 0) {
        perror("Failed to fork child2");
        kill(child1, SIGTERM);
        unlink(FIFO1_PATH);
        unlink(FIFO2_PATH);
        return 1;
    } else if (child2 == 0) {
        // This is Child Process 2
        printf("Child 2 (PID %d) started\n", getpid());
        
        // Sleep for 10 seconds
        sleep(10);
        
        // Open the second FIFO to read the command (larger number)
        int fd2 = open(FIFO2_PATH, O_RDONLY);
        if (fd2 == -1) {
            perror("Child 2: Failed to open FIFO2");
            exit(1);
        }
        
        // Set non-blocking mode
        if (set_nonblocking(fd2) == -1) {
            perror("Child 2: Failed to set non-blocking mode");
            close(fd2);
            exit(1);
        }
        
        int larger;
        ssize_t bytes_read = read_with_timeout(fd2, &larger, sizeof(larger), 5);
        if (bytes_read != sizeof(larger)) {
            if (errno == ETIMEDOUT) {
                fprintf(stderr, "Child 2: Timeout reading from FIFO2\n");
            } else {
                perror("Child 2: Failed to read from FIFO2");
            }
            close(fd2);
            exit(1);
        }
        
        close(fd2);
        
        // Print the larger number
        printf("Child 2 (PID %d) Result: The larger number is %d\n", getpid(), larger);
        exit(0);
    }

    // Store the child2 PID for monitoring
    child2_pid = child2;

    // Daemonize the parent process
    daemonize();

    // This is the parent process
    // Write the numbers to the first FIFO
    int fd1 = open(FIFO1_PATH, O_WRONLY);
    if (fd1 == -1) {
        perror("Parent: Failed to open FIFO1");
        kill(child1, SIGTERM);
        kill(child2, SIGTERM);
        unlink(FIFO1_PATH);
        unlink(FIFO2_PATH);
        return 1;
    }
    
    int nums[2] = {num1, num2};
    if (write(fd1, nums, sizeof(nums)) != sizeof(nums)) {
        perror("Parent: Failed to write to FIFO1");
        close(fd1);
        kill(child1, SIGTERM);
        kill(child2, SIGTERM);
        unlink(FIFO1_PATH);
        unlink(FIFO2_PATH);
        return 1;
    }
    
    close(fd1);
    
    // Main loop
    while (childCounter < totalChildren * 2) {
        printf("Parent: Proceeding... (Counter: %d)\n", childCounter);
        sleep(2);
    }
    
    // Clean up
    printf("Parent: All children have exited. Cleaning up.\n");
    
    // Send termination signal to daemon
    if (daemonPid > 0) {
        printf("Parent: Sending SIGTERM to daemon (PID %d)\n", daemonPid);
        kill(daemonPid, SIGTERM);
    }
    
    unlink(FIFO1_PATH);
    unlink(FIFO2_PATH);
    
    printf("Parent: Exiting.\n");
    return 0;
} 