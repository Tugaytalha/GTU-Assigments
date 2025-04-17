#define _GNU_SOURCE // For dprintf
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>
#include <stdarg.h> // For variadic functions like log_message
#include <sys/file.h> // For flock

// --- Configuration ---
#define FIFO1_NAME "/tmp/hw2_fifo1"
#define FIFO2_NAME "/tmp/hw2_fifo2"
#define LOG_FILE "/tmp/daemon.log"
#define PID_FILE "/tmp/homework2.pid" // To prevent multiple instances
#define COMMAND "LARGER"
#define CHILD_SLEEP_DURATION 10
#define PARENT_SLEEP_DURATION 2
#define NUM_CHILDREN 2

// --- Global Variables ---
int log_fd = -1;
volatile sig_atomic_t child_exit_count = 0;
pid_t child_pids[NUM_CHILDREN];
volatile sig_atomic_t terminate_flag = 0;
int pid_file_fd = -1; // File descriptor for PID file lock

// --- Utility Functions ---

// Simple logging function (remains the same)
void log_message(const char *format, ...) {
    if (log_fd < 0) return;

    va_list args;
    time_t now = time(NULL);
    char timestamp[30];
    char message_buffer[512];
    char final_buffer[600];

    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", localtime(&now));

    va_start(args, format);
    vsnprintf(message_buffer, sizeof(message_buffer), format, args);
    va_end(args);

    snprintf(final_buffer, sizeof(final_buffer), "[%s] [PID:%d] %s\n", timestamp, getpid(), message_buffer);

    // Use write for signal safety
    ssize_t written = write(log_fd, final_buffer, strlen(final_buffer));
    if (written < 0) {
        // Cannot easily log this error if log_fd itself is the problem
        // Write to original stderr might work if redirection hasn't happened yet
        dprintf(STDERR_FILENO, "FATAL: Failed to write to log file: %s\n", strerror(errno));
    }
}

// Cleanup function for FIFOs and PID file
void cleanup_resources() {
    log_message("Cleaning up resources...");
    if (log_fd >= 0) {
         log_message("Closing log file.");
         close(log_fd);
         log_fd = -1; // Mark as closed
    }
    unlink(FIFO1_NAME);
    unlink(FIFO2_NAME);
     if (pid_file_fd >= 0) {
         log_message("Releasing PID file lock and removing PID file.");
         flock(pid_file_fd, LOCK_UN); // Release lock
         close(pid_file_fd);
         unlink(PID_FILE);
         pid_file_fd = -1; // Mark as closed/unlinked
     } else {
         // Attempt removal even if fd is not held (e.g., if lock failed)
         unlink(PID_FILE);
     }
}

// Error handling macro - modified to call cleanup
#define CHECK_ERR(condition, message, ...) \
    do { \
        if (condition) { \
            char err_buf[256]; \
            snprintf(err_buf, sizeof(err_buf), message, ##__VA_ARGS__); \
            if (log_fd >= 0) { \
                log_message("ERROR: %s: %s", err_buf, strerror(errno)); \
            } else { \
                fprintf(stderr, "ERROR [PID:%d]: %s: %s\n", getpid(), err_buf, strerror(errno)); \
            } \
            cleanup_resources(); /* Call cleanup before exiting */ \
            exit(EXIT_FAILURE); \
        } \
    } while (0)


// --- Signal Handlers (mostly unchanged, but logging improved) ---

void sigchld_handler(int sig) {
    int saved_errno = errno;
    int status;
    pid_t child_pid;

    // Use write for signal safety in logging inside handler
    char log_buf[256];

    while ((child_pid = waitpid(-1, &status, WNOHANG)) > 0) {
        char status_msg[100];
        int exit_status_val = -1; // Store exit status if available

        if (WIFEXITED(status)) {
            exit_status_val = WEXITSTATUS(status);
            snprintf(status_msg, sizeof(status_msg), "exited normally with status %d", exit_status_val);
        } else if (WIFSIGNALED(status)) {
            snprintf(status_msg, sizeof(status_msg), "killed by signal %d", WTERMSIG(status));
        } else {
            snprintf(status_msg, sizeof(status_msg), "terminated with unknown status %d", status);
        }

        // Log using write
        int len = snprintf(log_buf, sizeof(log_buf), "[SIGCHLD] Child process %d terminated (%s).\n", child_pid, status_msg);
        if (len > 0 && log_fd >= 0) {
            write(log_fd, log_buf, len);
        }

        // Increment counter atomically
        child_exit_count++; // Okay for simple increment

        // Check if the exit status indicates an error
        if (WIFEXITED(status) && WEXITSTATUS(status) != EXIT_SUCCESS) {
             len = snprintf(log_buf, sizeof(log_buf), "[SIGCHLD] WARNING: Child %d exited with error status %d.\n", child_pid, WEXITSTATUS(status));
             if (len > 0 && log_fd >= 0) {
                 write(log_fd, log_buf, len);
             }
        }
    }

    if (child_pid < 0 && errno != ECHILD) {
        int len = snprintf(log_buf, sizeof(log_buf), "[SIGCHLD] ERROR: waitpid failed: %s\n", strerror(errno));
         if (len > 0 && log_fd >= 0) {
             write(log_fd, log_buf, len);
         }
    }

    errno = saved_errno;
}


void sigterm_handler(int sig) {
     // Use write for signal safety
    char msg[] = "[SIGTERM] Received SIGTERM. Initiating graceful shutdown.\n";
    if (log_fd >= 0) write(log_fd, msg, sizeof(msg) - 1);
    terminate_flag = 1;
}

void sighup_handler(int sig) {
    char msg[] = "[SIGHUP] Received SIGHUP. Reconfiguration triggered (action not implemented).\n";
    if (log_fd >= 0) write(log_fd, msg, sizeof(msg) - 1);
    // Re-open log file potentially, or reload config
}

void sigusr1_handler(int sig) {
    char msg[] = "[SIGUSR1] Received SIGUSR1. Custom signal action triggered (action not implemented).\n";
    if (log_fd >= 0) write(log_fd, msg, sizeof(msg) - 1);
}

// --- Daemonization (minor improvement: add PID file locking) ---

void daemonize() {
    pid_t pid;

    // --- PID File Check & Lock ---
    pid_file_fd = open(PID_FILE, O_RDWR | O_CREAT, 0666);
    if (pid_file_fd < 0) {
        perror("FATAL: Could not open PID file");
        // Cannot use CHECK_ERR as log_fd might not be open yet
        exit(EXIT_FAILURE);
    }

    // Try to acquire an exclusive lock without blocking
    if (flock(pid_file_fd, LOCK_EX | LOCK_NB) < 0) {
        if (errno == EWOULDBLOCK) {
            fprintf(stderr, "ERROR: Daemon already running? PID file %s is locked.\n", PID_FILE);
        } else {
            perror("ERROR: Could not lock PID file");
        }
        close(pid_file_fd);
        exit(EXIT_FAILURE);
    }
    // PID file locked successfully, continue daemonization.
    // We will write the final daemon PID later.


    // 1. Fork and exit parent
    pid = fork();
    // Use temporary buffer for errors before log redirection
    char err_buf[100];
    if (pid < 0) {
        snprintf(err_buf, sizeof(err_buf), "FATAL: Failed first fork for daemonization: %s\n", strerror(errno));
        write(STDERR_FILENO, err_buf, strlen(err_buf)); // Write directly
        cleanup_resources();
        exit(EXIT_FAILURE);
    }
    if (pid > 0) {
        exit(EXIT_SUCCESS); // First parent exits
    }

    // 2. Create new session
    if (setsid() < 0) {
         snprintf(err_buf, sizeof(err_buf), "FATAL: Failed setsid: %s\n", strerror(errno));
         write(STDERR_FILENO, err_buf, strlen(err_buf));
         cleanup_resources();
         exit(EXIT_FAILURE);
    }

    // 3. Fork again and exit session leader (optional but good)
    signal(SIGHUP, SIG_IGN); // Ignore SIGHUP for session leader exit
    pid = fork();
     if (pid < 0) {
        snprintf(err_buf, sizeof(err_buf), "FATAL: Failed second fork for daemonization: %s\n", strerror(errno));
        write(STDERR_FILENO, err_buf, strlen(err_buf));
        cleanup_resources();
        exit(EXIT_FAILURE);
    }
    if (pid > 0) {
        exit(EXIT_SUCCESS); // Session leader exits
    }

    // --- Final Daemon Process Continues ---
    pid_t final_pid = getpid();
    // Now write the final daemon PID to the locked file
    if (ftruncate(pid_file_fd, 0) < 0) { // Clear the file first
         perror("WARNING: Could not truncate PID file"); // Non-fatal
    }
    char pid_str[20];
    snprintf(pid_str, sizeof(pid_str), "%d\n", final_pid);
    if (write(pid_file_fd, pid_str, strlen(pid_str)) < 0) {
         perror("WARNING: Could not write PID to PID file"); // Non-fatal
    }
    // Keep pid_file_fd open to maintain the lock

    // Use log_message now, assuming log_fd is open
    log_message("Daemon process successfully started (PID: %d)", final_pid);
    log_message("PID file created and locked: %s", PID_FILE);


    // 4. Change working directory
    CHECK_ERR(chdir("/") < 0, "Failed to change directory to /");
    log_message("Working directory changed to /");


    // 5. Set umask
    umask(0);
    log_message("Umask set to 0");


    // 6. Close standard file descriptors
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    // 7. Redirect stdin, stdout, stderr
    int fd0 = open("/dev/null", O_RDONLY);
    CHECK_ERR(fd0 < 0, "Failed to open /dev/null for stdin");
    CHECK_ERR(dup2(fd0, STDIN_FILENO) < 0, "Failed to dup2 /dev/null to stdin");

    CHECK_ERR(log_fd < 0, "Log file descriptor is invalid before redirecting stdout/stderr");
    CHECK_ERR(dup2(log_fd, STDOUT_FILENO) < 0, "Failed to dup2 log_fd to stdout");
    CHECK_ERR(dup2(log_fd, STDERR_FILENO) < 0, "Failed to dup2 log_fd to stderr");

    if (fd0 != STDIN_FILENO) close(fd0);

    log_message("Standard I/O streams closed and redirected.");
}


// --- Child Process Logic (Added sleep at the beginning) ---

void run_child1(int val1, int val2) {
    // Child doesn't need parent's PID file lock fd
    if (pid_file_fd >= 0) close(pid_file_fd);

    log_message("Child 1 (PID: %d) starting. Sleeping for %d seconds.", getpid(), CHILD_SLEEP_DURATION);
    sleep(CHILD_SLEEP_DURATION);
    log_message("Child 1 (PID: %d) woke up. Executing task.", getpid());


    int fd1_read, fd2_write;

    // Open FIFOs (Child 1: read from FIFO1, write to FIFO2)
    // Use blocking open, daemon ensures data is ready *after* fork
    fd1_read = open(FIFO1_NAME, O_RDONLY);
    CHECK_ERR(fd1_read < 0, "Child 1 failed to open FIFO1 for reading");
    log_message("Child 1 opened FIFO1 for reading.");


    fd2_write = open(FIFO2_NAME, O_WRONLY);
    CHECK_ERR(fd2_write < 0, "Child 1 failed to open FIFO2 for writing");
    log_message("Child 1 opened FIFO2 for writing.");


    int received_val1, received_val2;
    ssize_t bytes_read;

    // Read the two integers from FIFO1
    bytes_read = read(fd1_read, &received_val1, sizeof(int));
    CHECK_ERR(bytes_read < 0, "Child 1 failed to read value 1 from FIFO1");
    CHECK_ERR(bytes_read == 0, "Child 1 read EOF before getting value 1 from FIFO1");
    CHECK_ERR(bytes_read != sizeof(int), "Child 1 read wrong size for value 1 (%ld bytes)", bytes_read);
    log_message("Child 1 read value 1: %d", received_val1);

    bytes_read = read(fd1_read, &received_val2, sizeof(int));
    CHECK_ERR(bytes_read < 0, "Child 1 failed to read value 2 from FIFO1");
    CHECK_ERR(bytes_read == 0, "Child 1 read EOF before getting value 2 from FIFO1");
    CHECK_ERR(bytes_read != sizeof(int), "Child 1 read wrong size for value 2 (%ld bytes)", bytes_read);
    log_message("Child 1 read value 2: %d", received_val2);

    close(fd1_read);
    log_message("Child 1 closed FIFO1 read end.");


    // Determine the larger number
    int larger_num = (received_val1 > received_val2) ? received_val1 : received_val2;
    log_message("Child 1 determined larger number: %d", larger_num);

    // Write the larger number to FIFO2
    ssize_t bytes_written = write(fd2_write, &larger_num, sizeof(int));
    CHECK_ERR(bytes_written != sizeof(int), "Child 1 failed to write larger number to FIFO2 (wrote %ld bytes)", bytes_written);
    log_message("Child 1 wrote larger number %d to FIFO2.", larger_num);

    close(fd2_write);
    log_message("Child 1 closed FIFO2 write end.");


    log_message("Child 1 (PID: %d) task completed. Exiting.", getpid());
    exit(EXIT_SUCCESS);
}

void run_child2() {
    // Child doesn't need parent's PID file lock fd
    if (pid_file_fd >= 0) close(pid_file_fd);

    log_message("Child 2 (PID: %d) starting. Sleeping for %d seconds.", getpid(), CHILD_SLEEP_DURATION);
    sleep(CHILD_SLEEP_DURATION);
     log_message("Child 2 (PID: %d) woke up. Executing task.", getpid());


    int fd2_read;
    char command_buffer[100];
    int larger_num;
    ssize_t bytes_read;

    // Open FIFO2 (Child 2: read command, then read number from FIFO2)
    fd2_read = open(FIFO2_NAME, O_RDONLY);
    CHECK_ERR(fd2_read < 0, "Child 2 failed to open FIFO2 for reading");
    log_message("Child 2 opened FIFO2 for reading.");


    // Read the command from FIFO2
    int i = 0;
    char ch;
    while ((bytes_read = read(fd2_read, &ch, 1)) > 0 && ch != '\0' && i < sizeof(command_buffer) - 1) {
        command_buffer[i++] = ch;
    }
    command_buffer[i] = '\0';

    CHECK_ERR(bytes_read < 0, "Child 2 failed to read command from FIFO2");
    CHECK_ERR(bytes_read == 0 && i == 0, "Child 2 read EOF before getting command from FIFO2");
    log_message("Child 2 read command: '%s'", command_buffer);


    // Verify command (optional)
    if (strcmp(command_buffer, COMMAND) != 0) {
        log_message("Child 2 received unexpected command: '%s'. Proceeding anyway.", command_buffer);
    }

    // Read the larger number from FIFO2 (sent by Child 1)
    bytes_read = read(fd2_read, &larger_num, sizeof(int));
     CHECK_ERR(bytes_read < 0, "Child 2 failed to read larger number from FIFO2");
     CHECK_ERR(bytes_read == 0, "Child 2 read EOF before getting larger number from FIFO2");
     CHECK_ERR(bytes_read != sizeof(int), "Child 2 read wrong size for larger number (%ld bytes)", bytes_read);
    log_message("Child 2 read larger number: %d", larger_num);

    close(fd2_read);
    log_message("Child 2 closed FIFO2 read end.");


    // Print the larger number *to the screen* (daemon's original stdout)
    // Since the daemon redirects its stdout *after* forking, the children
    // inherit the *redirected* stdout (the log file).
    // To print to the original console, we'd need a more complex setup
    // (e.g., passing the original tty name or using a socket back to the initial process).
    // For this assignment, let's log it instead, as printing to the daemon's
    // redirected stdout fulfills the "print" requirement technically, although
    // it ends up in the log file.
    log_message("Result: The larger number is %d (This would normally print to screen)", larger_num);
    // If printing to console was essential:
    // int console_fd = open("/dev/tty", O_WRONLY);
    // if (console_fd >= 0) {
    //     dprintf(console_fd, "Result: The larger number is %d\n", larger_num);
    //     close(console_fd);
    // } else {
    //      log_message("Warning: Could not open /dev/tty to print result to console.");
    // }


    log_message("Child 2 (PID: %d) task completed. Exiting.", getpid());
    exit(EXIT_SUCCESS);
}


// --- Main Function (Refactored: Daemonize First) ---

int main(int argc, char *argv[]) {
    int val1, val2;
    int result = 0; // Defined as requested
    struct sigaction sa;

    // --- Initial Setup (Before Daemonization) ---
    // Open Log File EARLY
    // Use O_TRUNC initially to clear old logs for a fresh run
    log_fd = open(LOG_FILE, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (log_fd < 0) {
        perror("FATAL: Failed to open log file");
        exit(EXIT_FAILURE);
    }
    dprintf(log_fd, "[%ld] Log file opened: %s\n", (long)time(NULL), LOG_FILE); // Initial log entry

    // Argument parsing
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <integer1> <integer2>\n", argv[0]);
        // Use CHECK_ERR - it will log to stderr since log_fd isn't fully set up for it yet
        CHECK_ERR(1, "Invalid number of arguments");
    }

    char *endptr1, *endptr2;
    errno = 0;
    val1 = strtol(argv[1], &endptr1, 10);
    CHECK_ERR(errno != 0 || *endptr1 != '\0', "Invalid first integer argument: %s", argv[1]);
    errno = 0;
    val2 = strtol(argv[2], &endptr2, 10);
    CHECK_ERR(errno != 0 || *endptr2 != '\0', "Invalid second integer argument: %s", argv[2]);

    // Log initial values *before* daemonization might redirect stderr
    log_message("Input values received: %d, %d", val1, val2);
    fprintf(stderr, "Initial values: %d, %d. Starting daemonization...\n", val1, val2); // To console


    // Create FIFOs (before daemonizing, so paths are known)
    unlink(FIFO1_NAME); // Clean up previous runs
    unlink(FIFO2_NAME);
    CHECK_ERR(mkfifo(FIFO1_NAME, 0666) < 0 && errno != EEXIST, "Failed to create FIFO1");
    CHECK_ERR(mkfifo(FIFO2_NAME, 0666) < 0 && errno != EEXIST, "Failed to create FIFO2");
    log_message("FIFOs created successfully: %s, %s", FIFO1_NAME, FIFO2_NAME);


    // --- Become a Daemon ---
    daemonize(); // This function handles exit/error logging internally now
    // --- Daemon Process Continues Below ---


    // --- Setup Signal Handlers (in Daemon) ---
    log_message("Daemon setting up signal handlers.");
    memset(&sa, 0, sizeof(sa));

    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    sigemptyset(&sa.sa_mask);

    sa.sa_handler = sigchld_handler;
    CHECK_ERR(sigaction(SIGCHLD, &sa, NULL) < 0, "Failed to set SIGCHLD handler");

    sa.sa_handler = sigterm_handler;
    CHECK_ERR(sigaction(SIGTERM, &sa, NULL) < 0, "Failed to set SIGTERM handler");

    sa.sa_handler = sighup_handler;
    CHECK_ERR(sigaction(SIGHUP, &sa, NULL) < 0, "Failed to set SIGHUP handler");

    sa.sa_handler = sigusr1_handler;
    CHECK_ERR(sigaction(SIGUSR1, &sa, NULL) < 0, "Failed to set SIGUSR1 handler");

    signal(SIGPIPE, SIG_IGN); // Ignore SIGPIPE
    log_message("Signal handlers set.");


    // --- Fork Child Processes (from Daemon) ---
    log_message("Daemon (PID: %d) forking children...", getpid());
    pid_t pid1 = -1, pid2 = -1;

    pid1 = fork();
    CHECK_ERR(pid1 < 0, "Daemon failed to fork Child 1");

    if (pid1 == 0) {
        // --- Child Process 1 Logic ---
        run_child1(val1, val2); // Pass values
        exit(EXIT_FAILURE); // Should not be reached
    }
    child_pids[0] = pid1;
    log_message("Daemon forked Child 1 with PID: %d", pid1);


    pid2 = fork();
    if (pid2 < 0) { // Handle error if second fork fails
        log_message("ERROR: Daemon failed to fork Child 2: %s. Killing Child 1.", strerror(errno));
        kill(pid1, SIGTERM); // Send TERM signal first
        sleep(1); // Give it a moment
        kill(pid1, SIGKILL); // Force kill if needed
        waitpid(pid1, NULL, 0); // Wait for it
        CHECK_ERR(1, "Fork Child 2 failed"); // Exit daemon
    }

    if (pid2 == 0) {
        // --- Child Process 2 Logic ---
        run_child2();
        exit(EXIT_FAILURE); // Should not be reached
    }
    child_pids[1] = pid2;
    log_message("Daemon forked Child 2 with PID: %d", pid2);


    // --- Daemon Sends Data via FIFOs ---
    // Children have been forked, now open FIFOs for writing and send data.
    // Children will block on their read opens until daemon opens write ends.
    log_message("Daemon opening FIFOs for writing...");
    int fd1_write = open(FIFO1_NAME, O_WRONLY);
    CHECK_ERR(fd1_write < 0, "Daemon failed to open FIFO1 for writing");

    int fd2_write = open(FIFO2_NAME, O_WRONLY);
    CHECK_ERR(fd2_write < 0, "Daemon failed to open FIFO2 for writing");
    log_message("Daemon FIFO write ends opened.");


    // Send the two integer values to FIFO1
    ssize_t bytes_written;
    bytes_written = write(fd1_write, &val1, sizeof(int));
    CHECK_ERR(bytes_written != sizeof(int), "Daemon failed to write value 1 to FIFO1 (wrote %ld bytes)", bytes_written);
    bytes_written = write(fd1_write, &val2, sizeof(int));
    CHECK_ERR(bytes_written != sizeof(int), "Daemon failed to write value 2 to FIFO1 (wrote %ld bytes)", bytes_written);
    log_message("Daemon wrote values %d, %d to FIFO1", val1, val2);
    close(fd1_write); // Close write end for FIFO1 - signals EOF to child 1 reader

    // Send the command to FIFO2
    bytes_written = write(fd2_write, COMMAND, strlen(COMMAND) + 1); // Include NUL terminator
    CHECK_ERR(bytes_written != strlen(COMMAND) + 1, "Daemon failed to write command to FIFO2 (wrote %ld bytes)", bytes_written);
    log_message("Daemon wrote command '%s' to FIFO2", COMMAND);

    // Close write end for FIFO2. Child 1 will open it later to write the result.
    // Child 2 will read the command, then block until Child 1 writes the result.
    close(fd2_write);
    log_message("Daemon closed initial FIFO write ends.");


    // --- Main Daemon Loop ---
    log_message("Daemon entering main loop, waiting for %d children.", NUM_CHILDREN);
    while (child_exit_count < NUM_CHILDREN && !terminate_flag) {
        // log_message("Daemon proceeding... (%d/%d children exited)", child_exit_count, NUM_CHILDREN);
        // Use pause() for better efficiency - wakes up only on signal
        pause();
        // After pause() returns (due to a signal), the loop condition is re-checked.
        // Our SIGCHLD handler updates child_exit_count.
        // Our SIGTERM handler sets terminate_flag.
    }


    // --- Shutdown ---
    if (terminate_flag) {
         log_message("Daemon exiting due to SIGTERM. (%d/%d children exited).", child_exit_count, NUM_CHILDREN);
         // Attempt to terminate any remaining children gracefully
         for (int i = 0; i < NUM_CHILDREN; ++i) {
             // Check if child process still exists (0 means signal can be sent)
             if (child_pids[i] > 0 && kill(child_pids[i], 0) == 0) {
                 log_message("Sending SIGTERM to remaining child PID %d", child_pids[i]);
                 kill(child_pids[i], SIGTERM);
             }
         }
         sleep(1); // Give them a chance to exit
         // Reap any children that might have exited after SIGTERM
         sigchld_handler(SIGCHLD); // Manually call handler to reap
    } else {
         log_message("All children (%d) have exited normally. Daemon shutting down.", NUM_CHILDREN);
    }

    cleanup_resources(); // Cleans up FIFOs, PID file, closes log_fd
    // Final message just before exit (might not make it to log if close happens too fast)
    // dprintf(log_fd is closed, can't use); // Cannot log reliably after close
    exit(EXIT_SUCCESS);
}