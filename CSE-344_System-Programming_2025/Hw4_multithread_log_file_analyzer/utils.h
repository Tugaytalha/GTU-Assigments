#ifndef UTILS_H
#define UTILS_H

#include <stdbool.h>
#include <pthread.h>
#include <signal.h>
#include "buffer.h"

// Worker thread arguments
typedef struct {
    int thread_id;                // Thread identifier
    char *search_term;           // Term to search for
    int match_count;             // Number of matches found by this thread
    pthread_barrier_t *barrier;  // Barrier for synchronization
    Buffer *buffer;              // Pointer to the shared buffer
} WorkerArgs;

// Set the global buffer reference for signal handling
void set_global_buffer(Buffer *buffer);

// Signal handler for SIGINT
void setup_signal_handler();

// Parse command line arguments
bool parse_arguments(int argc, char *argv[], int *buffer_size, int *num_workers, 
                     char **log_file, char **search_term);

// Print program usage
void print_usage(char *program_name);

// Check if a line contains the search term
bool line_contains_term(const char *line, const char *term);

// Report matches found by a worker thread
void report_worker_matches(int thread_id, int matches);

// Generate summary report
void generate_summary_report(WorkerArgs *worker_args, int num_workers);

#endif // UTILS_H 