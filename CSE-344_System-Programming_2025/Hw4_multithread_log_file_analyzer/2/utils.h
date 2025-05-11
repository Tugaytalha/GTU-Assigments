#ifndef UTILS_H
#define UTILS_H

#include <stdbool.h>
#include <pthread.h>
#include <signal.h> // For sig_atomic_t
// Forward declare Buffer to avoid circular dependency if Buffer needs utils types
// However, in this case, WorkerArgs needs Buffer, so Buffer.h must be included.
#include "buffer.h" 

#define MAX_LINE_LENGTH 2048 // Increased max line length slightly, can be adjusted

// Worker thread arguments
typedef struct {
    int thread_id;                // Unique worker identifier
    const char *search_term;      // Term to search for (read-only)
    int match_count;              // Matches found by this worker
    pthread_barrier_t *barrier;   // Synchronization barrier
    Buffer *buffer;               // Pointer to the shared data buffer
} WorkerArgs;

// Set the global buffer reference (used by signal handler)
void set_global_buffer(Buffer *buffer);

// Setup the SIGINT signal handler
void setup_signal_handler();

// Parse command line arguments. Returns true on success.
bool parse_arguments(int argc, char *argv[], int *buffer_size, int *num_workers, 
                     char **log_file, char **search_term);

// Print program usage instructions to stderr.
void print_usage(char *program_name);

// Check if 'line' contains 'term'. Returns true if found.
bool line_contains_term(const char *line, const char *term);

// Report matches found by a single worker thread.
void report_worker_matches(int thread_id, int matches);

// Generate and print a summary report of all matches.
void generate_summary_report(WorkerArgs *worker_args, int num_workers);

#endif // UTILS_H