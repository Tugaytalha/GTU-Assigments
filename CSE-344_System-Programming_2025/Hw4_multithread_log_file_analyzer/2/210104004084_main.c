#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include "buffer.h"
#include "utils.h"

// External global flag for termination
extern volatile sig_atomic_t terminate_flag;

// Function prototypes
void *manager_thread(void *arg);
void *worker_thread(void *arg);

// Manager thread arguments
typedef struct {
    const char *log_file;
    Buffer *buffer;
} ManagerArgs;

// Global file pointer for worker output (requires synchronization if multiple workers write)
FILE *worker_output_file = NULL;
pthread_mutex_t output_mutex = PTHREAD_MUTEX_INITIALIZER; // Mutex for file access

int main(int argc, char *argv[]) {
    int buffer_size, num_workers;
    const char *log_file, *search_term;
    pthread_t manager_tid;
    pthread_t *worker_tids = NULL;
    WorkerArgs *worker_args = NULL;
    pthread_barrier_t barrier;
    Buffer *buffer = NULL;
    int i;
    int exit_status = EXIT_SUCCESS;

    // Set up signal handler for SIGINT
    setup_signal_handler();

    // Parse command line arguments
    if (!parse_arguments(argc, argv, &buffer_size, &num_workers, &log_file, &search_term)) {
        print_usage(argv[0]);
        exit_status = EXIT_FAILURE;
        goto cleanup;
    }

    printf("Starting Log Analyzer\n");
    printf("Buffer Size: %d, Workers: %d, File: %s, Search Term: \"%s\"\n",
           buffer_size, num_workers, log_file, search_term);

    // Open file for worker output (overwrite existing file)
    worker_output_file = fopen("worker_matches.log", "w");
    if (!worker_output_file) {
        perror("Failed to open worker output file");
        exit_status = EXIT_FAILURE;
        goto cleanup;
    }

    // Initialize buffer
    buffer = buffer_init(buffer_size);
    if (!buffer) {
        fprintf(stderr, "Error: Failed to initialize buffer\n");
        exit_status = EXIT_FAILURE;
        goto cleanup;
    }

    // Set global buffer reference for signal handling
    set_global_buffer(buffer);

    // Initialize barrier for worker threads
    if (pthread_barrier_init(&barrier, NULL, num_workers) != 0) {
        perror("Failed to initialize barrier");
        exit_status = EXIT_FAILURE;
        goto cleanup;
    }

    // Allocate and initialize worker arguments and thread IDs
    worker_args = (WorkerArgs *)calloc(num_workers, sizeof(WorkerArgs));
    worker_tids = (pthread_t *)malloc(num_workers * sizeof(pthread_t));

    if (!worker_args || !worker_tids) {
        perror("Memory allocation failed for worker structures");
        exit_status = EXIT_FAILURE;
        goto cleanup;
    }

    // Initialize worker arguments
    for (i = 0; i < num_workers; i++) {
        worker_args[i].thread_id = i + 1;
        worker_args[i].search_term = search_term;
        worker_args[i].match_count = 0;
        worker_args[i].barrier = &barrier;
        worker_args[i].buffer = buffer;
    }

    // Create manager thread arguments
    ManagerArgs manager_args = {
        .log_file = log_file,
        .buffer = buffer
    };

    // Create manager thread
    if (pthread_create(&manager_tid, NULL, manager_thread, &manager_args) != 0) {
        perror("Failed to create manager thread");
        exit_status = EXIT_FAILURE;
        goto cleanup;
    }

    // Create worker threads
    for (i = 0; i < num_workers; i++) {
        if (pthread_create(&worker_tids[i], NULL, worker_thread, &worker_args[i]) != 0) {
            perror("Failed to create worker thread");
            terminate_flag = 1;
            for (int j = 0; j < i; j++) {
                pthread_join(worker_tids[j], NULL);
            }
            pthread_join(manager_tid, NULL);

            exit_status = EXIT_FAILURE;
            goto cleanup;
        }
    }

    // Wait for manager thread to finish
    pthread_join(manager_tid, NULL);

    // Wait for all worker threads to finish
    for (i = 0; i < num_workers; i++) {
        pthread_join(worker_tids[i], NULL);
    }

    // Only generate report if not terminated by signal
    if (!terminate_flag) {
        generate_summary_report(worker_args, num_workers);
    } else {
        fprintf(stderr, "Program terminated by signal.\n");
    }


cleanup:
    if (worker_args) free(worker_args);
    if (worker_tids) free(worker_tids);
    if (buffer) {
         pthread_barrier_destroy(&barrier);
         buffer_destroy(buffer);
    }
    // Close the worker output file
    if (worker_output_file) {
        fclose(worker_output_file);
        worker_output_file = NULL; // Set to NULL after closing
    }
    // Destroy the output mutex
    pthread_mutex_destroy(&output_mutex);


    return exit_status;
}

// Manager thread function
void *manager_thread(void *arg) {
    ManagerArgs *args = (ManagerArgs *)arg;
    FILE *file = fopen(args->log_file, "r");
    char line[1024];
    size_t len;

    if (!file) {
        fprintf(stderr, "Error: Could not open file %s: %s\n", args->log_file, strerror(errno));
        buffer_add_eof_marker(args->buffer);
        return NULL;
    }

    while (fgets(line, sizeof(line), file) != NULL && !terminate_flag) {
        len = strlen(line);
        if (len > 0 && line[len-1] == '\n') {
            line[len-1] = '\0';
        }

        buffer_add(args->buffer, line);
    }

    if (!terminate_flag) {
        buffer_add_eof_marker(args->buffer);
    }

    fclose(file);
    return NULL;
}

// Worker thread function
void *worker_thread(void *arg) {
    WorkerArgs *args = (WorkerArgs *)arg;
    char *line;

    printf("Worker %d started\n", args->thread_id); // Keep this initial print to stdout

    while (true) {
        line = buffer_remove(args->buffer);

        if (line == NULL) {
            break;
        }

        // Search for the term and write to file if found
        if (line_contains_term(line, args->search_term)) {
            args->match_count++;
            // Write the matching line to the output file
            pthread_mutex_lock(&output_mutex);
            if (worker_output_file) { // Check if file is still open
                fprintf(worker_output_file, "Worker %d found match: %s\n", args->thread_id, line);
                fflush(worker_output_file); // Flush immediately to see output as it happens
            }
            pthread_mutex_unlock(&output_mutex);
        }

        free(line);
    }

    if (!terminate_flag) {
        // Reporting matches found by this worker (now printed in summary)
        // report_worker_matches(args->thread_id, args->match_count); // Removed terminal print

        pthread_barrier_wait(args->barrier);
    } else {
         printf("Worker %d exiting due to termination request.\n", args->thread_id);
    }

    return NULL;
}