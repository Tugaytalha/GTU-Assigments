#ifndef UTILS_H
#define UTILS_H

#include <stdbool.h>
#include <signal.h>

// Structure to hold program arguments
typedef struct {
    int buffer_size;
    int num_workers;
    char *log_file;
    char *search_term;
} ProgramArgs;

// Parse command line arguments into program args structure
// Returns true if arguments are valid, false otherwise
bool parse_arguments(int argc, char *argv[], ProgramArgs *args);

// Print usage information to stderr
void print_usage(const char *program_name);

// Set up signal handling (for SIGINT)
void setup_signal_handling();

// Global flag to tell threads to exit gracefully
extern volatile sig_atomic_t exit_flag;

#endif // UTILS_H
