#ifndef UTILS_H
#define UTILS_H

#include <stdbool.h>
#include <pthread.h>
#include <signal.h>
#include "buffer.h"
#include <stdio.h> // Added for FILE*

// External global flag for program termination
extern volatile sig_atomic_t terminate_flag;

// Worker thread arguments
typedef struct {
    int thread_id;
    const char *search_term;
    int match_count;
    pthread_barrier_t *barrier;
    Buffer *buffer;
} WorkerArgs;

// External global file pointer and mutex for worker output
extern FILE *worker_output_file;
extern pthread_mutex_t output_mutex;


// Set the global buffer reference for signal handling
void set_global_buffer(Buffer *buffer);

// Setup signal handler
void setup_signal_handler();

// Parse command line arguments
bool parse_arguments(int argc, char *argv[], int *buffer_size, int *num_workers,
                     const char **log_file, const char **search_term);

// Print program usage
void print_usage(char *program_name);

// Check if a line contains the search term
bool line_contains_term(const char *line, const char *term);

// Report matches found by a worker thread (No longer prints to stdout, concept changed)
// void report_worker_matches(int thread_id, int matches); // Function removed as output goes to file

// Generate summary report
void generate_summary_report(WorkerArgs *worker_args, int num_workers);

#endif // UTILS_H