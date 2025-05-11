#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <errno.h> // Added for errno/strerror

// Global flag for program termination
volatile sig_atomic_t terminate_flag = 0;

// Global reference to the shared buffer for signal handling
Buffer *global_buffer = NULL;

// Set the global buffer reference for signal handling
void set_global_buffer(Buffer *buffer) {
    global_buffer = buffer;
}

// Signal handler for SIGINT (Ctrl+C)
void sigint_handler(int sig) {
    terminate_flag = 1;
    fprintf(stderr, "\nReceived SIGINT, initiating graceful shutdown...\n"); // Changed message to stderr

    // Wake up any threads waiting on condition variables
    if (global_buffer) {
        // Use trylock to avoid deadlock if mutex is held for a long time,
        // though lock/broadcast/unlock is standard for this pattern.
        // We'll stick to the standard lock/broadcast/unlock here for simplicity
        // as buffer operations are designed to be fast.
        pthread_mutex_lock(&global_buffer->mutex);
        pthread_cond_broadcast(&global_buffer->not_empty); // Wake up waiting consumers
        pthread_cond_broadcast(&global_buffer->not_full);  // Wake up waiting producers (manager)
        pthread_mutex_unlock(&global_buffer->mutex);
    }
}

// Setup signal handler
void setup_signal_handler() {
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = sigint_handler;
    sa.sa_flags = 0; // Standard flags
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("Failed to set up SIGINT handler");
        // Handle error: Depending on severity, might exit or log.
        // For this assignment, perror is likely sufficient.
    }
}

// Parse command line arguments
bool parse_arguments(int argc, char *argv[], int *buffer_size, int *num_workers,
                     const char **log_file, const char **search_term) { // Use const char**
    // Check if we have the correct number of arguments
    if (argc != 5) {
        return false;
    }

    // Parse buffer size
    char *endptr_size; // For strtol error checking
    *buffer_size = strtol(argv[1], &endptr_size, 10); // Use strtol for safer conversion

    if (*endptr_size != '\0' || *buffer_size <= 0) {
        fprintf(stderr, "Error: Invalid or non-positive buffer size '%s'.\n", argv[1]);
        return false;
    }

    // Parse number of workers
    char *endptr_workers; // For strtol error checking
    *num_workers = strtol(argv[2], &endptr_workers, 10); // Use strtol

    if (*endptr_workers != '\0' || *num_workers <= 0) {
        fprintf(stderr, "Error: Invalid or non-positive number of workers '%s'.\n", argv[2]);
        return false;
    }

    // Set log file and search term
    *log_file = argv[3];
    *search_term = argv[4];

    return true;
}

// Print program usage
void print_usage(char *program_name) {
    fprintf(stderr, "Usage: %s <buffer_size> <num_workers> <log_file> <search_term>\n", program_name);
    fprintf(stderr, "  <buffer_size>: Positive integer for the size of the shared buffer.\n");
    fprintf(stderr, "  <num_workers>: Positive integer for the number of worker threads.\n");
    fprintf(stderr, "  <log_file>: Path to the log file to analyze.\n");
    fprintf(stderr, "  <search_term>: The keyword or phrase to search for in log lines.\n");
}

// Check if a line contains the search term
bool line_contains_term(const char *line, const char *term) {
    // strstr returns NULL if the substring is not found
    return (strstr(line, term) != NULL);
}

// Report matches found by a worker thread
void report_worker_matches(int thread_id, int matches) {
    printf("Worker %d: %d matches found.\n", thread_id, matches); // Slightly different phrasing
}

// Generate summary report
void generate_summary_report(WorkerArgs *worker_args, int num_workers) {
    int total_matches = 0;

    printf("\n--- Log Analysis Summary ---\n"); // Slightly different header
    for (int i = 0; i < num_workers; i++) {
        printf("Worker %d reported %d matches.\n", worker_args[i].thread_id, worker_args[i].match_count); // Slightly different phrasing
        total_matches += worker_args[i].match_count;
    }

    printf("\nTotal matches across all workers: %d\n", total_matches); // Slightly different phrasing
    printf("Analysis completed.\n"); // Changed completion message
    printf("----------------------------\n"); // Added separator
}