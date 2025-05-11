#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h> // For write in signal handler

// Global flag for program termination
volatile sig_atomic_t terminate_flag = 0;

// Global reference to the shared buffer for signal handling
static Buffer *g_shared_buffer_for_signal = NULL; // Made static

// Set the global buffer reference for signal handling
void set_global_buffer(Buffer *buffer) {
    g_shared_buffer_for_signal = buffer;
}

// Signal handler for SIGINT (Ctrl+C)
void sigint_handler(int sig) {
    // Use write for async-signal safety instead of printf
    const char msg[] = "\nSIGINT received, initiating graceful shutdown...\n";
    write(STDOUT_FILENO, msg, sizeof(msg) - 1);
    
    terminate_flag = 1;
    
    // Wake up any threads waiting on condition variables
    if (g_shared_buffer_for_signal) {
        pthread_mutex_lock(&g_shared_buffer_for_signal->mutex);
        pthread_cond_broadcast(&g_shared_buffer_for_signal->not_empty);
        pthread_cond_broadcast(&g_shared_buffer_for_signal->not_full);
        pthread_mutex_unlock(&g_shared_buffer_for_signal->mutex);
    }
}

// Setup signal handler
void setup_signal_handler() {
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = sigint_handler;
    sigemptyset(&sa.sa_mask); // Ensure no other signals are blocked during handler
    // sa.sa_flags = SA_RESTART; // Optional: restart syscalls if interrupted
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("Failed to set up SIGINT handler");
        // Depending on policy, might exit here or just warn
    }
}

// Parse command line arguments
bool parse_arguments(int argc, char *argv[], int *buffer_size, int *num_workers, 
                     char **log_file, char **search_term) {
    if (argc != 5) {
        fprintf(stderr, "Incorrect number of arguments provided.\n");
        return false;
    }

    *buffer_size = atoi(argv[1]);
    if (*buffer_size <= 0) {
        fprintf(stderr, "Error: Buffer size must be a positive integer. Value provided: %s\n", argv[1]);
        return false;
    }

    *num_workers = atoi(argv[2]);
    if (*num_workers <= 0) {
        fprintf(stderr, "Error: Number of workers must be a positive integer. Value provided: %s\n", argv[2]);
        return false;
    }

    *log_file = argv[3];
    *search_term = argv[4];

    // Basic check for empty search term, though strstr handles it.
    if (strlen(*search_term) == 0) {
        fprintf(stderr, "Warning: Search term is empty.\n");
    }


    return true;
}

// Print program usage
void print_usage(char *program_name) {
    fprintf(stderr, "Usage: %s <buffer_size:int> <num_workers:int> <log_file:string> <search_term:string>\n", program_name);
    fprintf(stderr, "Example: %s 10 4 /var/log/syslog \"ERROR\"\n", program_name);
}

// Check if a line contains the search term (case-sensitive)
bool line_contains_term(const char *line, const char *term) {
    if (!line || !term) return false; // Basic null check
    return (strstr(line, term) != NULL);
}

// Report matches found by a worker thread
void report_worker_matches(int thread_id, int matches) {
    printf("Worker %d: Processed and found %d matching entries.\n", thread_id, matches);
}

// Generate summary report of all findings
void generate_summary_report(WorkerArgs *worker_args_list, int num_workers) {
    int total_matches = 0;
    
    printf("\n--- Aggregated Search Results ---\n");
    for (int i = 0; i < num_workers; i++) {
        printf("  Worker %d reported %d matches.\n", worker_args_list[i].thread_id, worker_args_list[i].match_count);
        total_matches += worker_args_list[i].match_count;
    }
    
    printf("\nTotal unique matches found across all workers: %d\n", total_matches);
    printf("Log analysis completed successfully.\n");
}