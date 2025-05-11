#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

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
    printf("\nReceived SIGINT, cleaning up and exiting...\n");
    
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
    sigaction(SIGINT, &sa, NULL);
}

// Parse command line arguments
bool parse_arguments(int argc, char *argv[], int *buffer_size, int *num_workers, 
                     char **log_file, char **search_term) {
    // Check if we have the correct number of arguments
    if (argc != 5) {
        return false;
    }

    // Parse buffer size
    *buffer_size = atoi(argv[1]);
    if (*buffer_size <= 0) {
        fprintf(stderr, "Error: Buffer size must be a positive integer\n");
        return false;
    }

    // Parse number of workers
    *num_workers = atoi(argv[2]);
    if (*num_workers <= 0) {
        fprintf(stderr, "Error: Number of workers must be a positive integer\n");
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
}

// Check if a line contains the search term
bool line_contains_term(const char *line, const char *term) {
    return (strstr(line, term) != NULL);
}

// Report matches found by a worker thread
void report_worker_matches(int thread_id, int matches) {
    printf("Worker %d found %d matches\n", thread_id, matches);
}

// Generate summary report
void generate_summary_report(WorkerArgs *worker_args, int num_workers) {
    int total_matches = 0;
    
    printf("\n--- Summary Report ---\n");
    for (int i = 0; i < num_workers; i++) {
        printf("Worker %d: %d matches\n", worker_args[i].thread_id, worker_args[i].match_count);
        total_matches += worker_args[i].match_count;
    }
    
    printf("\nTotal matches found: %d\n", total_matches);
    printf("Search completed successfully.\n");
} 