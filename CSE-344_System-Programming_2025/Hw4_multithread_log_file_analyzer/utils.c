#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <errno.h>

// Global flag for program termination
volatile sig_atomic_t terminate_flag = 0;

// Global reference to the shared buffer for signal handling
Buffer *global_buffer = NULL;

// Global file pointer and mutex for worker output (defined in main.c, extern here)
extern FILE *worker_output_file;
extern pthread_mutex_t output_mutex;

// Set the global buffer reference for signal handling
void set_global_buffer(Buffer *buffer) {
    global_buffer = buffer;
}

// Signal handler for SIGINT (Ctrl+C)
void sigint_handler(int sig) {
    terminate_flag = 1;
    fprintf(stderr, "\nReceived SIGINT, initiating graceful shutdown...\n");

    // Wake up any threads waiting on condition variables
    if (global_buffer) {
        pthread_mutex_lock(&global_buffer->mutex);
        pthread_cond_broadcast(&global_buffer->not_empty);
        pthread_cond_broadcast(&global_buffer->not_full);
        pthread_mutex_unlock(&global_buffer->mutex);
    }
}

// Setup signal handler
void setup_signal_handler() {
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = sigint_handler;
    sa.sa_flags = 0;
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("Failed to set up SIGINT handler");
    }
}

// Parse command line arguments
bool parse_arguments(int argc, char *argv[], int *buffer_size, int *num_workers,
                     const char **log_file, const char **search_term) {
    if (argc != 5) {
        return false;
    }

    char *endptr_size;
    *buffer_size = strtol(argv[1], &endptr_size, 10);

    if (*endptr_size != '\0' || *buffer_size <= 0) {
        fprintf(stderr, "Error: Invalid or non-positive buffer size '%s'.\n", argv[1]);
        return false;
    }

    char *endptr_workers;
    *num_workers = strtol(argv[2], &endptr_workers, 10);

    if (*endptr_workers != '\0' || *num_workers <= 0) {
        fprintf(stderr, "Error: Invalid or non-positive number of workers '%s'.\n", argv[2]);
        return false;
    }

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
    fprintf(stderr, "Matched lines will be written to 'worker_matches.log'.\n"); // Added file info
}

// Check if a line contains the search term
bool line_contains_term(const char *line, const char *term) {
    return (strstr(line, term) != NULL);
}

// Report matches found by a worker thread (This function is removed/replaced)
// The actual writing to file happens inside the worker_thread function now.
// Keep a stub or remove it entirely if it's no longer called.
// Removing it entirely is cleaner if no longer used.

// Generate summary report
void generate_summary_report(WorkerArgs *worker_args, int num_workers) {
    int total_matches = 0;

    printf("\n--- Log Analysis Summary ---\n");
    printf("Individual worker match counts:\n"); // Added heading
    for (int i = 0; i < num_workers; i++) {
        printf("Worker %d: %d matches reported.\n", worker_args[i].thread_id, worker_args[i].match_count); // Slightly different phrasing
        total_matches += worker_args[i].match_count;
    }

    printf("\nTotal matches across all workers: %d\n", total_matches);
    printf("Matched lines logged to 'worker_matches.log'.\n"); // Indicate where matches are
    printf("Analysis completed.\n");
    printf("----------------------------\n");
}