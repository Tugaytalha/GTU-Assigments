#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#ifdef _WIN32
#include <io.h>
#define F_OK 0
#define access _access
#else
#include <unistd.h>
#endif

// Global flag that will be set when SIGINT is received
volatile sig_atomic_t exit_flag = 0;

// Signal handler for SIGINT
static void handle_sigint(int sig) {
    exit_flag = 1;
}

// Set up signal handling for SIGINT
void setup_signal_handling() {
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handle_sigint;
    sigaction(SIGINT, &sa, NULL);
}

// Print usage information
void print_usage(const char *program_name) {
    fprintf(stderr, "Usage: %s <buffer_size> <num_workers> <log_file> <search_term>\n", 
            program_name ? program_name : "./LogAnalyzer");
}

// Parse command line arguments
bool parse_arguments(int argc, char *argv[], ProgramArgs *args) {
    if (!args) {
        return false;
    }

    // Check if we have the right number of arguments
    if (argc != 5) {
        print_usage(argv[0]);
        return false;
    }

    // Parse buffer size
    char *endptr;
    args->buffer_size = (int)strtol(argv[1], &endptr, 10);
    if (*endptr != '\0' || args->buffer_size <= 0) {
        fprintf(stderr, "Error: Buffer size must be a positive integer\n");
        print_usage(argv[0]);
        return false;
    }

    // Parse number of workers
    args->num_workers = (int)strtol(argv[2], &endptr, 10);
    if (*endptr != '\0' || args->num_workers <= 0) {
        fprintf(stderr, "Error: Number of workers must be a positive integer\n");
        print_usage(argv[0]);
        return false;
    }

    // Check if log file exists and is readable
    args->log_file = argv[3];
    if (access(args->log_file, R_OK) != 0) {
        fprintf(stderr, "Error: Cannot access log file '%s'\n", args->log_file);
        print_usage(argv[0]);
        return false;
    }

    // Get search term
    args->search_term = argv[4];
    if (strlen(args->search_term) == 0) {
        fprintf(stderr, "Error: Search term cannot be empty\n");
        print_usage(argv[0]);
        return false;
    }

    return true;
}
