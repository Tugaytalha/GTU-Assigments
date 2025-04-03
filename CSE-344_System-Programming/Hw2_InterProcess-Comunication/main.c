// main.c
#define _POSIX_C_SOURCE 200809L // For sigaction, strdup, etc.
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h> // For pid_t, kill, mkfifo, open, mode_t
#include <sys/stat.h>  // For mkfifo, open mode flags, umask
#include <sys/wait.h>  // For waitpid, WIFEXITED etc.
#include <signal.h>    // For signals, sigaction, kill, SA_*, sigemptyset, sig_atomic_t
#include <string.h>    // For memset, strerror, strcmp
#include <errno.h>     // For errno
#include <time.h>      // For time()
#include <sys/select.h>// For select()
#include <sys/time.h>  // For struct timeval
#include <stdarg.h>    // For va_list in logging
#include <libgen.h>    // For basename

// --- Constants ---
#define FIFO1_PATH "/tmp/ipc_daemon_fifo1" // Use /tmp for better portability
#define FIFO2_PATH "/tmp/ipc_daemon_fifo2"
#define LOG_FILE "/tmp/ipc_daemon.log"
#define TIMEOUT_SECONDS 30 // Timeout for detecting inactive children (adjust as needed)
#define CHILD_SLEEP_DURATION 10
#define DAEMON_LOOP_SLEEP 2

// --- Global Variables ---
// Use volatile sig_atomic_t for variables modified in signal handlers and accessed elsewhere
volatile sig_atomic_t child_exit_counter = 0;
volatile sig_atomic_t sigterm_received = 0; // Flag for graceful shutdown
volatile sig_atomic_t sighup_received = 0;  // Flag for SIGHUP handling

// Store PIDs globally for access in signal handler and daemon loop
pid_t child1_pid = -1;
pid_t child2_pid = -1;

// Track start times for timeout monitoring
time_t child1_start_time = 0;
time_t child2_start_time = 0;

const int total_children = 2; // Fixed number of children

FILE *log_fp = NULL; // Global log file pointer for daemon
char *prog_name = NULL; // Store program name for logging

// --- Utility Functions ---

// Log messages consistently with timestamp and PID
void log_message(const char *format, ...) {
    time_t now = time(NULL);
    struct tm *local_time = localtime(&now);
    char time_buf[30];
    strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", local_time);

    // Use stderr before daemonization/log file opening, otherwise use log_fp
    FILE *output = log_fp ? log_fp : stderr;

    fprintf(output, "[%s] [%d] ", time_buf, getpid());

    va_list args;
    va_start(args, format);
    vfprintf(output, format, args);
    va_end(args);

    fprintf(output, "\n");
    fflush(output); // Ensure message is written immediately
}

// Function to clean up resources (FIFOs, log file)
void cleanup() {
    log_message("Initiating cleanup...");
    // Unlink FIFOs only if they might exist (e.g., created by this process)
    // Check return values, but mainly log errors as cleanup might fail partially
    if (unlink(FIFO1_PATH) == -1 && errno != ENOENT) {
        log_message("Warning: Failed to unlink %s: %s", FIFO1_PATH, strerror(errno));
    } else {
        log_message("Unlinked %s", FIFO1_PATH);
    }
    if (unlink(FIFO2_PATH) == -1 && errno != ENOENT) {
        log_message("Warning: Failed to unlink %s: %s", FIFO2_PATH, strerror(errno));
    } else {
        log_message("Unlinked %s", FIFO2_PATH);
    }

    if (log_fp) {
        log_message("Closing log file.");
        fclose(log_fp);
        log_fp = NULL;
    }
    log_message("Cleanup finished.");
}

// Setup signal action helper
int setup_signal_action(int signum, void (*handler)(int)) {
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handler;
    sigemptyset(&sa.sa_mask);
    // SA_RESTART: auto-restart interrupted syscalls (like sleep, select, waitpid)
    // SA_NOCLDSTOP: only interested in termination, not stopping/continuing
    sa.sa_flags = SA_RESTART | (signum == SIGCHLD ? SA_NOCLDSTOP : 0);

    if (sigaction(signum, &sa, NULL) == -1) {
        log_message("ERROR: Failed to set signal handler for signal %d: %s", signum, strerror(errno));
        return -1;
    }
    return 0;
}

// --- Signal Handlers ---

// Signal handler for SIGCHLD
void sigchld_handler(int sig) {
    (void)sig; // Mark unused parameter
    int status;
    pid_t pid;

    // Reap all terminated children non-blockingly
    // Use a loop with WNOHANG as multiple children might terminate close together
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        const char *child_name = "Unknown Child";
        if (pid == child1_pid) child_name = "Child 1";
        else if (pid == child2_pid) child_name = "Child 2";

        if (WIFEXITED(status)) {
            log_message("SIGCHLD: %s (PID %d) exited normally with status %d.", child_name, pid, WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            log_message("SIGCHLD: %s (PID %d) was terminated by signal %d.", child_name, pid, WTERMSIG(status));
        } else {
            log_message("SIGCHLD: %s (PID %d) terminated with unknown status (%d).", child_name, pid, status);
        }

        // Increment counter by two as per assignment specification
        child_exit_counter += 2;
        log_message("SIGCHLD: Counter incremented by 2, current value: %d", child_exit_counter);

        // Reset the PID if we know which child it was, prevents timeout check on dead child
        if (pid == child1_pid) child1_pid = -1; // Mark as terminated
        if (pid == child2_pid) child2_pid = -1; // Mark as terminated

    }
    // ECHILD is expected if no more children are left to wait for when using WNOHANG
    if (pid == -1 && errno != ECHILD) {
         log_message("ERROR: waitpid failed in SIGCHLD handler: %s", strerror(errno));
    }
    // Re-install handler? Some systems might require it, but sigaction usually doesn't. Check documentation if needed.
}

// Signal handler for SIGUSR1 (example: log status)
void sigusr1_handler(int sig) {
    (void)sig;
    log_message("INFO: Received SIGUSR1. Current child exit counter: %d", child_exit_counter);
    // Add any other status logging needed
}

// Signal handler for SIGHUP (example: reconfigure/reload - here just log and set flag)
void sighup_handler(int sig) {
    (void)sig;
    log_message("INFO: Received SIGHUP. Setting flag for potential reconfiguration.");
    // Set a flag to be checked in the main daemon loop for actual action
    sighup_received = 1;
}

// Signal handler for SIGTERM (graceful shutdown)
void sigterm_handler(int sig) {
    (void)sig;
    log_message("INFO: Received SIGTERM. Initiating graceful shutdown.");
    // Set a flag to break the main loop cleanly
    sigterm_received = 1;
    // Cleanup will be called after the loop breaks or via atexit
}

// --- Daemonization Function ---
// Modifies the *calling* process to become a daemon
int become_daemon() {
    pid_t pid;

    // 1. Fork and exit parent to detach from terminal
    pid = fork();
    if (pid < 0) {
        log_message("ERROR: Failed to fork for daemonization step 1: %s", strerror(errno));
        return -1; // Error
    }
    if (pid > 0) {
        // Parent exits successfully, leaving child to continue
        log_message("INFO: Parent process (PID %d) exiting after first fork.", getpid());
        exit(EXIT_SUCCESS);
    }

    // --- Child (potential daemon) continues ---
    log_message("INFO: First child (PID %d) continuing daemonization.", getpid());

    // 2. Create a new session (setsid) to become session leader and process group leader
    if (setsid() < 0) {
        log_message("ERROR: Failed to create new session (setsid): %s", strerror(errno));
        return -1; // Error
    }
    log_message("INFO: New session created (PID %d is session leader).", getpid());

    // Optional: Ignore SIGHUP for the session leader (prevents termination if controlling terminal closes)
    // signal(SIGHUP, SIG_IGN); // Using sigaction is preferred, handle it properly below

    // 3. Fork again and exit parent (the session leader)
    // Ensures the daemon cannot reacquire a controlling terminal (SysV recommendation)
    pid = fork();
     if (pid < 0) {
         log_message("ERROR: Failed to fork for daemonization step 2: %s", strerror(errno));
         return -1; // Error
     }
     if (pid > 0) {
         // Session leader exits
         log_message("INFO: Session leader (PID %d) exiting after second fork.", getpid());
         exit(EXIT_SUCCESS);
     }

    // --- Grandchild (actual daemon) continues ---
    log_message("INFO: Daemon process (PID %d) starting.", getpid());

    // 4. Change working directory (optional but good practice)
    // Change to root to avoid preventing unmounting of the filesystem it was started from.
    // If files are relative (like log file), adjust path or change to a specific daemon dir.
    // Using /tmp for log/fifo avoids this issue. Let's comment out chdir for simplicity here.
    /*
    if (chdir("/") < 0) {
        log_message("ERROR: Failed to change directory to /: %s", strerror(errno));
        return -1; // Error
    }
    log_message("INFO: Changed working directory to /");
    */

    // 5. Set umask (controls file mode creation mask)
    // Set to 0 to have full control via open() modes, or a more restrictive mask.
    umask(0);
    log_message("INFO: Set umask to 0.");

    // 6. Close standard file descriptors (stdin, stdout, stderr)
    log_message("INFO: Closing standard file descriptors.");
    if (close(STDIN_FILENO) == -1) {
         log_message("WARNING: Failed to close STDIN_FILENO: %s", strerror(errno));
    }
    if (close(STDOUT_FILENO) == -1) {
         log_message("WARNING: Failed to close STDOUT_FILENO: %s", strerror(errno));
         // Logging might become problematic here if stderr is also closed incorrectly
    }
    if (close(STDERR_FILENO) == -1) {
         log_message("WARNING: Failed to close STDERR_FILENO: %s", strerror(errno));
    }

    // 7. Redirect stdin, stdout, stderr
    // Redirect stdin to /dev/null
    int fd_devnull = open("/dev/null", O_RDWR);
    if (fd_devnull == -1) {
        // Cannot log this easily now. Critical error.
        // Maybe attempt to log to syslog if available? Or just exit.
        perror("CRITICAL: Failed to open /dev/null"); // Might go nowhere
        return -1;
    }
    if (dup2(fd_devnull, STDIN_FILENO) == -1) {
        perror("CRITICAL: Failed to redirect stdin to /dev/null");
        close(fd_devnull);
        return -1;
    }
    // Don't close fd_devnull here if it's now STDIN_FILENO

    // Open log file for stdout and stderr (Append mode)
    // Ensure log file path is absolute or daemon has correct working dir. Using /tmp avoids this.
    int log_fd = open(LOG_FILE, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (log_fd < 0) {
        perror("CRITICAL: Failed to open log file");
        // Maybe close stdin again?
        close(STDIN_FILENO);
        return -1;
    }
    if (dup2(log_fd, STDOUT_FILENO) == -1) {
        perror("CRITICAL: Failed to redirect stdout to log file");
        close(log_fd); // Close original fd
        close(STDIN_FILENO);
        return -1;
    }
     if (dup2(log_fd, STDERR_FILENO) == -1) {
        perror("CRITICAL: Failed to redirect stderr to log file");
        close(STDOUT_FILENO); // Close the duplicated one
        close(log_fd); // Close original fd
        close(STDIN_FILENO);
        return -1;
    }

    // Now get a FILE* for easier logging with stdio functions (optional but convenient)
    // Note: We need to be careful if log_fd is closed explicitly later.
    // If using stdio (printf, fprintf to stdout/stderr), they now go to the log file.
    // Alternatively, keep using the log_message function with a dedicated FILE*
    log_fp = fdopen(log_fd, "a");
    if (!log_fp) {
        // If fdopen fails, logging might be inconsistent.
        // Try logging directly to the fd? Or rely on previous dup2.
        write(STDERR_FILENO, "ERROR: fdopen failed for log file\n", 34); // Low-level write
        // Continue cautiously, log_message might not use log_fp
    } else {
        setvbuf(log_fp, NULL, _IOLBF, 0); // Line buffering for log file is good
    }

    // We don't need the original log_fd anymore if log_fp is used or if dup2 succeeded
    // and we only use printf/fprintf(stderr,...). Let's keep log_fp open.
    // If not using log_fp, close(log_fd) is needed if > STDERR_FILENO.
    // Since we assign log_fp = fdopen(log_fd, ...), log_fd should NOT be closed directly.
    // fclose(log_fp) will close the underlying fd.

    // --- Daemon Setup Complete ---
    log_message("Daemon process initialization successful (PID %d). Logging to %s", getpid(), LOG_FILE);
    return 0; // Success
}


// --- FIFO Helper Functions ---

// Set file descriptor to non-blocking mode
int set_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) {
        log_message("ERROR: fcntl(F_GETFL) failed: %s", strerror(errno));
        return -1;
    }
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        log_message("ERROR: fcntl(F_SETFL O_NONBLOCK) failed: %s", strerror(errno));
        return -1;
    }
    return 0;
}

// Read exactly 'count' bytes from a potentially non-blocking fd with timeout
ssize_t read_exact_timeout(int fd, void *buf, size_t count, int timeout_sec) {
    size_t total_read = 0;
    ssize_t current_read;
    char *ptr = (char *)buf;
    struct timeval tv;
    fd_set readfds;
    int ret;
    time_t start_time = time(NULL);

    while (total_read < count) {
        // Calculate remaining time
        time_t elapsed = time(NULL) - start_time;
        if (elapsed >= timeout_sec) {
            errno = ETIMEDOUT;
            log_message("ERROR: Read timeout after %ld seconds (target %d).", (long)elapsed, timeout_sec);
            return -1; // Timeout
        }

        tv.tv_sec = timeout_sec - elapsed;
        tv.tv_usec = 0;

        FD_ZERO(&readfds);
        FD_SET(fd, &readfds);

        // select waits for data availability or timeout
        ret = select(fd + 1, &readfds, NULL, NULL, &tv);

        if (ret == -1) {
            if (errno == EINTR) {
                log_message("DEBUG: select interrupted by signal, retrying.");
                continue; // Interrupted by signal, retry select
            }
            log_message("ERROR: select failed in read_exact_timeout: %s", strerror(errno));
            return -1; // Select error
        } else if (ret == 0) {
            errno = ETIMEDOUT; // Timeout expired according to select
            log_message("ERROR: Read timeout during select.");
            return -1;
        }

        // Data is available (or select returned > 0), attempt to read
        current_read = read(fd, ptr + total_read, count - total_read);

        if (current_read > 0) {
            total_read += current_read;
            log_message("DEBUG: Read %zd bytes, total %zu/%zu", current_read, total_read, count);
        } else if (current_read == 0) {
            // EOF - Pipe closed by writer before we read everything?
            log_message("ERROR: Read EOF prematurely after reading %zu bytes (expected %zu).", total_read, count);
            errno = EPIPE; // Or some other suitable error
            return -1; // Indicate unexpected EOF
        } else { // current_read == -1
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // Should not happen often after select indicates readability, but possible.
                // Wait briefly or just continue loop relying on select's timeout.
                log_message("DEBUG: read returned EAGAIN/EWOULDBLOCK, continuing select loop.");
                // Small sleep might prevent busy-waiting if select behaves unexpectedly, but select should handle waiting.
                // usleep(10000); // e.g., 10ms (requires #include <unistd.h>) - Use with caution.
                continue;
            } else if (errno == EINTR) {
                 log_message("DEBUG: read interrupted by signal, retrying.");
                 continue; // Interrupted by signal, retry read within the loop
            } else {
                log_message("ERROR: read failed in read_exact_timeout: %s", strerror(errno));
                return -1; // Other read error
            }
        }
    } // end while

    return total_read; // Should be equal to 'count' if successful
}


// Write exactly 'count' bytes (handles potential partial writes)
ssize_t write_exact(int fd, const void *buf, size_t count) {
    size_t total_written = 0;
    ssize_t current_written;
    const char *ptr = (const char *)buf;

    while (total_written < count) {
        current_written = write(fd, ptr + total_written, count - total_written);

        if (current_written > 0) {
            total_written += current_written;
        } else if (current_written == 0) {
             // Should not happen for regular files/pipes unless maybe disk full?
             log_message("WARNING: write returned 0 (written %zu/%zu bytes).", total_written, count);
             errno = EIO; // Indicate generic I/O error
             return -1;
        } else { // current_written == -1
            if (errno == EINTR) {
                log_message("DEBUG: write interrupted by signal, retrying.");
                continue; // Retry write
            } else if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // This can happen if the pipe buffer is full (non-blocking write)
                // For blocking write, it shouldn't occur unless flags change.
                // For this assignment using blocking writes in parent/child1, so treat as error.
                log_message("ERROR: write returned EAGAIN/EWOULDBLOCK (unexpected for blocking write).");
                // Or potentially retry after a delay/select for writability if using non-blocking writes.
                return -1;
            }
            else {
                log_message("ERROR: write failed: %s", strerror(errno));
                return -1; // Other write error
            }
        }
    }
    return total_written; // Success
}


// --- Main Function ---

int main(int argc, char *argv[]) {
    // Store program name for logging
    // Use strdup because argv[0] might be modified or invalid later
    prog_name = basename(strdup(argv[0])); // Handles path, gets executable name

    // Initial log to stderr before potential redirection
    log_message("INFO: %s starting.", prog_name);

    // --- Argument Parsing ---
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <integer1> <integer2>\n", prog_name);
        log_message("ERROR: Invalid number of arguments. Expected 2, got %d.", argc - 1);
        free(prog_name); // Free duplicated string
        return EXIT_FAILURE;
    }

    // Use strtol for safer integer conversion
    char *endptr1, *endptr2;
    long num1_long = strtol(argv[1], &endptr1, 10);
    long num2_long = strtol(argv[2], &endptr2, 10);

    // Check for conversion errors
    if (*endptr1 != '\0' || endptr1 == argv[1] || *endptr2 != '\0' || endptr2 == argv[2]) {
        fprintf(stderr, "Error: Invalid integer argument(s).\n");
        log_message("ERROR: Invalid integer argument(s) provided: '%s', '%s'.", argv[1], argv[2]);
        free(prog_name);
        return EXIT_FAILURE;
    }
    // Check if longs fit into int (optional, depends on expected range)
    if (num1_long < INT_MIN || num1_long > INT_MAX || num2_long < INT_MIN || num2_long > INT_MAX) {
         fprintf(stderr, "Error: Integer argument(s) out of range.\n");
         log_message("ERROR: Integer argument(s) out of range.");
         free(prog_name);
         return EXIT_FAILURE;
    }
    int num1 = (int)num1_long;
    int num2 = (int)num2_long;
    log_message("INFO: Parsed arguments: num1=%d, num2=%d", num1, num2);

    // Define the result variable as requested (unused in main logic flow)
    int result = 0;
    log_message("INFO: Defined result variable with initial value %d.", result);

    // --- FIFO Creation ---
    log_message("INFO: Creating FIFOs: %s, %s", FIFO1_PATH, FIFO2_PATH);
    if (mkfifo(FIFO1_PATH, 0666) == -1) {
        if (errno != EEXIST) {
            log_message("ERROR: Failed to create %s: %s", FIFO1_PATH, strerror(errno));
            free(prog_name);
            return EXIT_FAILURE;
        }
        log_message("INFO: %s already exists.", FIFO1_PATH);
    }
    if (mkfifo(FIFO2_PATH, 0666) == -1) {
        if (errno != EEXIST) {
            log_message("ERROR: Failed to create %s: %s", FIFO2_PATH, strerror(errno));
            // Attempt to clean up FIFO1 if we created it potentially
            unlink(FIFO1_PATH); // Ignore error here, best effort
            free(prog_name);
            return EXIT_FAILURE;
        }
        log_message("INFO: %s already exists.", FIFO2_PATH);
    }
    log_message("INFO: FIFOs created or confirmed existing.");

    // --- Register Cleanup Function ---
    // This ensures cleanup happens on normal exit or exit() call
    atexit(cleanup);

    // --- Setup SIGCHLD Handler ---
    log_message("INFO: Setting up SIGCHLD handler.");
    if (setup_signal_action(SIGCHLD, sigchld_handler) == -1) {
        // Error already logged by setup_signal_action
        free(prog_name);
        return EXIT_FAILURE; // atexit will handle cleanup
    }
    log_message("INFO: SIGCHLD handler registered.");

    // --- Fork Child Process 1 ---
    log_message("INFO: Forking Child Process 1...");
    child1_pid = fork();

    if (child1_pid < 0) {
        log_message("ERROR: Failed to fork Child Process 1: %s", strerror(errno));
        free(prog_name);
        return EXIT_FAILURE; // atexit handles cleanup
    }
    else if (child1_pid == 0) {
        // --- Child Process 1 Code ---
        pid_t mypid = getpid();
        int exit_status = EXIT_SUCCESS; // Assume success initially
        int fd1_c1 = -1, fd2_c1 = -1;
        log_message("CHILD 1 (PID %d): Started.", mypid);

        // Sleep for specified duration
        log_message("CHILD 1 (PID %d): Sleeping for %d seconds.", mypid, CHILD_SLEEP_DURATION);
        sleep(CHILD_SLEEP_DURATION);
        log_message("CHILD 1 (PID %d): Woke up.", mypid);

        // --- Task: Read from FIFO1 ---
        log_message("CHILD 1 (PID %d): Opening %s for reading.", mypid, FIFO1_PATH);
        fd1_c1 = open(FIFO1_PATH, O_RDONLY);
        if (fd1_c1 == -1) {
            log_message("CHILD 1 (PID %d): ERROR: Failed to open %s for reading: %s", mypid, FIFO1_PATH, strerror(errno));
            exit(EXIT_FAILURE); // Exit child 1
        }

        // Set non-blocking for read with timeout
        if (set_nonblocking(fd1_c1) == -1) {
            log_message("CHILD 1 (PID %d): ERROR: Failed to set non-blocking mode on %s.", mypid, FIFO1_PATH);
            close(fd1_c1);
            exit(EXIT_FAILURE);
        }

        int nums_read[2];
        log_message("CHILD 1 (PID %d): Reading two integers from %s (timeout %ds)...", mypid, FIFO1_PATH, TIMEOUT_SECONDS);
        ssize_t bytes_read = read_exact_timeout(fd1_c1, nums_read, sizeof(nums_read), TIMEOUT_SECONDS);

        close(fd1_c1); // Close FIFO1 read end

        if (bytes_read != sizeof(nums_read)) {
            if (bytes_read == -1) { // Error from read_exact_timeout (already logged)
                log_message("CHILD 1 (PID %d): ERROR: Failed to read from %s.", mypid, FIFO1_PATH);
            } else { // Premature EOF or incomplete read
                log_message("CHILD 1 (PID %d): ERROR: Incomplete read from %s (got %zd bytes, expected %zu).", mypid, FIFO1_PATH, bytes_read, sizeof(nums_read));
            }
            exit_status = EXIT_FAILURE;
        } else {
            log_message("CHILD 1 (PID %d): Successfully read numbers %d, %d from %s.", mypid, nums_read[0], nums_read[1], FIFO1_PATH);

            // --- Task: Determine Larger Number ---
            int larger_num = (nums_read[0] > nums_read[1]) ? nums_read[0] : nums_read[1];
            log_message("CHILD 1 (PID %d): Determined larger number: %d.", mypid, larger_num);

            // --- Task: Write to FIFO2 ---
            log_message("CHILD 1 (PID %d): Opening %s for writing.", mypid, FIFO2_PATH);
            // Use blocking write for simplicity here, assumes Child 2 will open FIFO2 reasonably soon.
            fd2_c1 = open(FIFO2_PATH, O_WRONLY);
            if (fd2_c1 == -1) {
                log_message("CHILD 1 (PID %d): ERROR: Failed to open %s for writing: %s", mypid, FIFO2_PATH, strerror(errno));
                exit_status = EXIT_FAILURE;
            } else {
                log_message("CHILD 1 (PID %d): Writing larger number %d to %s.", mypid, larger_num, FIFO2_PATH);
                ssize_t bytes_written = write_exact(fd2_c1, &larger_num, sizeof(larger_num));
                close(fd2_c1); // Close FIFO2 write end

                if (bytes_written != sizeof(larger_num)) {
                     log_message("CHILD 1 (PID %d): ERROR: Failed/incomplete write to %s (wrote %zd bytes).", mypid, FIFO2_PATH, bytes_written);
                     exit_status = EXIT_FAILURE;
                } else {
                     log_message("CHILD 1 (PID %d): Successfully wrote to %s.", mypid, FIFO2_PATH);
                }
            }
        }

        log_message("CHILD 1 (PID %d): Task complete. Exiting with status %d.", mypid, exit_status);
        free(prog_name); // Free duplicated string in child
        exit(exit_status); // Exit Child 1
    }
    // --- Parent continues after forking Child 1 ---
    log_message("INFO: Parent (PID %d) forked Child 1 with PID %d.", getpid(), child1_pid);


    // --- Fork Child Process 2 ---
    log_message("INFO: Forking Child Process 2...");
    child2_pid = fork();

     if (child2_pid < 0) {
         log_message("ERROR: Failed to fork Child Process 2: %s", strerror(errno));
         // Critical failure: Terminate Child 1 if it was successfully started
         if (child1_pid > 0) {
             log_message("INFO: Sending SIGTERM to Child 1 (PID %d) due to Child 2 fork failure.", child1_pid);
             kill(child1_pid, SIGTERM);
         }
         free(prog_name);
         return EXIT_FAILURE; // atexit handles cleanup
     }
     else if (child2_pid == 0) {
         // --- Child Process 2 Code ---
         pid_t mypid = getpid();
         int exit_status = EXIT_SUCCESS;
         int fd2_c2 = -1;
         log_message("CHILD 2 (PID %d): Started.", mypid);

         // Sleep for specified duration
         log_message("CHILD 2 (PID %d): Sleeping for %d seconds.", mypid, CHILD_SLEEP_DURATION);
         sleep(CHILD_SLEEP_DURATION);
         log_message("CHILD 2 (PID %d): Woke up.", mypid);

         // --- Task: Read from FIFO2 ---
         log_message("CHILD 2 (PID %d): Opening %s for reading.", mypid, FIFO2_PATH);
         fd2_c2 = open(FIFO2_PATH, O_RDONLY);
         if (fd2_c2 == -1) {
             log_message("CHILD 2 (PID %d): ERROR: Failed to open %s for reading: %s", mypid, FIFO2_PATH, strerror(errno));
             exit(EXIT_FAILURE); // Exit child 2
         }

         // Set non-blocking for read with timeout
         if (set_nonblocking(fd2_c2) == -1) {
             log_message("CHILD 2 (PID %d): ERROR: Failed to set non-blocking mode on %s.", mypid, FIFO2_PATH);
             close(fd2_c2);
             exit(EXIT_FAILURE);
         }

         int larger_num_read;
         log_message("CHILD 2 (PID %d): Reading larger number from %s (timeout %ds)...", mypid, FIFO2_PATH, TIMEOUT_SECONDS);
         ssize_t bytes_read = read_exact_timeout(fd2_c2, &larger_num_read, sizeof(larger_num_read), TIMEOUT_SECONDS);

         close(fd2_c2); // Close FIFO2 read end

         if (bytes_read != sizeof(larger_num_read)) {
             if (bytes_read == -1) { // Error logged by read_exact_timeout
                 log_message("CHILD 2 (PID %d): ERROR: Failed to read from %s.", mypid, FIFO2_PATH);
             } else {
                 log_message("CHILD 2 (PID %d): ERROR: Incomplete read from %s (got %zd bytes, expected %zu).", mypid, FIFO2_PATH, bytes_read, sizeof(larger_num_read));
             }
             exit_status = EXIT_FAILURE;
         } else {
              log_message("CHILD 2 (PID %d): Successfully read larger number %d from %s.", mypid, larger_num_read, FIFO2_PATH);
             // --- Task: Print Larger Number ---
             // Print to original standard output (important requirement)
             printf("CHILD 2 (PID %d): Result: The larger number is %d\n", mypid, larger_num_read);
             fflush(stdout); // Ensure it's printed immediately
             log_message("CHILD 2 (PID %d): Printed the larger number to stdout.", mypid);
         }

         log_message("CHILD 2 (PID %d): Task complete. Exiting with status %d.", mypid, exit_status);
         free(prog_name); // Free duplicated string in child
         exit(exit_status); // Exit Child 2
     }
    // --- Parent continues after forking Child 2 ---
    log_message("INFO: Parent (PID %d) forked Child 2 with PID %d.", getpid(), child2_pid);


    // === Parent Process: Write initial data to FIFO1 ===
    int fd1_p = -1;
    int parent_initial_status = EXIT_SUCCESS;

    log_message("PARENT (PID %d): Opening %s for writing.", getpid(), FIFO1_PATH);
    // This will block until Child 1 opens FIFO1 for reading, unless O_NONBLOCK is used (not used here)
    fd1_p = open(FIFO1_PATH, O_WRONLY);
    if (fd1_p == -1) {
        log_message("PARENT (PID %d): ERROR: Failed to open %s for writing: %s", getpid(), FIFO1_PATH, strerror(errno));
        log_message("PARENT (PID %d): Critical error. Signaling children to terminate.", getpid());
        if (child1_pid > 0) kill(child1_pid, SIGTERM);
        if (child2_pid > 0) kill(child2_pid, SIGTERM);
        parent_initial_status = EXIT_FAILURE;
        // atexit handles cleanup
        free(prog_name);
        return parent_initial_status;
    }

    int nums_to_send[2] = {num1, num2};
    log_message("PARENT (PID %d): Writing numbers %d, %d to %s.", getpid(), num1, num2, FIFO1_PATH);
    ssize_t bytes_written = write_exact(fd1_p, nums_to_send, sizeof(nums_to_send));

    // Close FIFO1 write end *after* writing
    if (close(fd1_p) == -1) {
         log_message("PARENT (PID %d): WARNING: Failed to close %s write end: %s", getpid(), FIFO1_PATH, strerror(errno));
    }

    if (bytes_written != sizeof(nums_to_send)) {
        log_message("PARENT (PID %d): ERROR: Failed/incomplete write to %s (wrote %zd bytes).", getpid(), FIFO1_PATH, bytes_written);
        log_message("PARENT (PID %d): Signaling children to terminate due to write error.", getpid());
        if (child1_pid > 0) kill(child1_pid, SIGTERM);
        if (child2_pid > 0) kill(child2_pid, SIGTERM);
        parent_initial_status = EXIT_FAILURE;
        // atexit handles cleanup
        free(prog_name);
        return parent_initial_status;
    } else {
        log_message("PARENT (PID %d): Successfully wrote initial numbers to %s.", getpid(), FIFO1_PATH);
    }


    // === Parent Process: Become Daemon ===
    log_message("PARENT (PID %d): Initial tasks complete. Converting to daemon...", getpid());
    if (become_daemon() == -1) {
        // Error should have been logged by become_daemon
        // If become_daemon failed before exit(), this process might still be running.
        fprintf(stderr, "%s: Failed to become daemon.\n", prog_name);
        // Signal children? Maybe they started ok. Let SIGCHLD handle them if parent exits.
        free(prog_name);
        return EXIT_FAILURE; // atexit might run depending on where failure occurred
    }

    // --- Code below executes *only* in the final Daemon Process ---
    // Note: printf/perror now go to LOG_FILE via redirection, or use log_message()
    // The original parent process has exited.

    pid_t daemon_pid = getpid();
    log_message("DAEMON (PID %d): Now running as daemon.", daemon_pid);

    // --- Setup Daemon Signal Handlers ---
    log_message("DAEMON (PID %d): Setting up daemon signal handlers (SIGUSR1, SIGHUP, SIGTERM).", daemon_pid);
    if (setup_signal_action(SIGUSR1, sigusr1_handler) == -1 ||
        setup_signal_action(SIGHUP, sighup_handler) == -1 ||
        setup_signal_action(SIGTERM, sigterm_handler) == -1) {
        log_message("DAEMON (PID %d): ERROR: Failed to set up one or more daemon signal handlers. Exiting.", daemon_pid);
        // Signal children? They might be running ok. Let init adopt them.
        free(prog_name); // Free in daemon process
        // atexit should still run
        return EXIT_FAILURE;
    }
    log_message("DAEMON (PID %d): Daemon signal handlers registered.", daemon_pid);


    // Record start times for timeout monitoring (relative to daemon start)
    time_t now = time(NULL);
    // Only record start time if child PID is valid (i.e., fork succeeded)
    if (child1_pid > 0) child1_start_time = now;
    if (child2_pid > 0) child2_start_time = now;
    log_message("DAEMON (PID %d): Recorded start times for child monitoring (Child1: %ld, Child2: %ld).",
                 daemon_pid, (long)child1_start_time, (long)child2_start_time);

    // === Daemon Main Loop ===
    log_message("DAEMON (PID %d): Entering main monitoring loop.", daemon_pid);
    int target_exits = total_children * 2; // Target value for counter

    while (child_exit_counter < target_exits && !sigterm_received) {

        // Handle SIGHUP if flag is set
        if (sighup_received) {
            log_message("DAEMON (PID %d): Processing SIGHUP.", daemon_pid);
            // Add reconfiguration logic here if needed (e.g., reopen log file)
            // For now, just reset the flag.
            sighup_received = 0;
            log_message("DAEMON (PID %d): SIGHUP processed (Placeholder action).", daemon_pid);
        }

        log_message("DAEMON (PID %d): Proceeding... (Exit counter: %d/%d)", daemon_pid, child_exit_counter, target_exits);

        // --- Timeout Check Logic ---
        now = time(NULL);

        // Check Child 1 (only if PID is still valid - not -1)
        if (child1_pid > 0) {
            // Check if process still exists using kill(pid, 0)
            if (kill(child1_pid, 0) == 0) {
                // Process exists, check timeout
                if (now - child1_start_time > TIMEOUT_SECONDS) {
                    log_message("DAEMON (PID %d): Child 1 (PID %d) inactive timeout (%ld > %d sec). Sending SIGTERM.",
                                daemon_pid, child1_pid, (long)(now - child1_start_time), TIMEOUT_SECONDS);
                    if (kill(child1_pid, SIGTERM) == -1) {
                        log_message("DAEMON (PID %d): ERROR sending SIGTERM to Child 1 (PID %d): %s",
                                    daemon_pid, child1_pid, strerror(errno));
                        // If kill fails, maybe the process just died? SIGCHLD handler should catch it.
                        // Mark as potentially dead to avoid repeated attempts? Or rely on SIGCHLD.
                        // Let's rely on SIGCHLD or the next kill(pid, 0) check.
                    } else {
                         log_message("DAEMON (PID %d): SIGTERM sent to Child 1 (PID %d). Awaiting SIGCHLD.", daemon_pid, child1_pid);
                         // SIGCHLD handler will set child1_pid = -1 and increment counter
                    }
                }
            } else if (errno == ESRCH) {
                // Process doesn't exist according to kill(pid, 0)
                // This *should* have been caught by SIGCHLD, but check defensively.
                log_message("DAEMON (PID %d): WARNING: Child 1 (PID %d) not found via kill(0), but SIGCHLD may not have run yet or PID was reset. Counter: %d",
                            daemon_pid, child1_pid, child_exit_counter);
                // Avoid repeatedly logging this. Maybe set child1_pid = -1 here? Risky if SIGCHLD is just delayed.
                // Let's assume SIGCHLD will handle it. If counter doesn't increment, this is a problem.
            } else {
                // Other error checking process existence
                 log_message("DAEMON (PID %d): ERROR checking existence of Child 1 (PID %d) with kill(0): %s",
                             daemon_pid, child1_pid, strerror(errno));
            }
        } // end check child 1

        // Check Child 2 (similarly)
        if (child2_pid > 0) {
             if (kill(child2_pid, 0) == 0) {
                 if (now - child2_start_time > TIMEOUT_SECONDS) {
                     log_message("DAEMON (PID %d): Child 2 (PID %d) inactive timeout (%ld > %d sec). Sending SIGTERM.",
                                 daemon_pid, child2_pid, (long)(now - child2_start_time), TIMEOUT_SECONDS);
                     if (kill(child2_pid, SIGTERM) == -1) {
                         log_message("DAEMON (PID %d): ERROR sending SIGTERM to Child 2 (PID %d): %s",
                                     daemon_pid, child2_pid, strerror(errno));
                     } else {
                          log_message("DAEMON (PID %d): SIGTERM sent to Child 2 (PID %d). Awaiting SIGCHLD.", daemon_pid, child2_pid);
                     }
                 }
             } else if (errno == ESRCH) {
                  log_message("DAEMON (PID %d): WARNING: Child 2 (PID %d) not found via kill(0), but SIGCHLD may not have run yet or PID was reset. Counter: %d",
                             daemon_pid, child2_pid, child_exit_counter);
             } else {
                  log_message("DAEMON (PID %d): ERROR checking existence of Child 2 (PID %d) with kill(0): %s",
                              daemon_pid, child2_pid, strerror(errno));
             }
         } // end check child 2

        // Sleep for a while before next check
        sleep(DAEMON_LOOP_SLEEP);

    } // end while loop

    // --- Loop Exited ---
    if (sigterm_received) {
        log_message("DAEMON (PID %d): Exiting loop due to SIGTERM.", daemon_pid);
    } else if (child_exit_counter >= target_exits) {
         log_message("DAEMON (PID %d): Exiting loop: All children accounted for (Counter: %d).", daemon_pid, child_exit_counter);
    } else {
        // Should not happen based on loop condition, but log defensively
        log_message("DAEMON (PID %d): Exiting loop for unknown reason (Counter: %d).", daemon_pid, child_exit_counter);
    }

    log_message("DAEMON (PID %d): Final cleanup will be handled by atexit.", daemon_pid);
    free(prog_name); // Free duplicated string in daemon

    return EXIT_SUCCESS; // Daemon exits normally
}