#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h> // Added for errno
#include "buffer.h"
#include "utils.h"

// External global flag for termination
extern volatile sig_atomic_t terminate_flag;

// Function prototypes
void *manager_thread(void *arg);
void *worker_thread(void *arg);

// Manager thread arguments
typedef struct {
    const char *log_file; // Use const char* for clarity
    Buffer *buffer;
} ManagerArgs;

int main(int argc, char *argv[]) {
    int buffer_size, num_workers;
    const char *log_file, *search_term; // Use const char*
    pthread_t manager_tid;
    pthread_t *worker_tids = NULL; // Initialize to NULL
    WorkerArgs *worker_args = NULL; // Initialize to NULL
    pthread_barrier_t barrier;
    Buffer *buffer = NULL; // Initialize to NULL
    int i;
    int exit_status = EXIT_SUCCESS; // Use named constant for exit status

    // Set up signal handler for SIGINT
    setup_signal_handler();

    // Parse command line arguments
    if (!parse_arguments(argc, argv, &buffer_size, &num_workers, &log_file, &search_term)) {
        print_usage(argv[0]);
        exit_status = EXIT_FAILURE;
        goto cleanup; // Use goto for centralized cleanup
    }

    printf("Starting Log Analyzer\n");
    printf("Buffer Size: %d, Workers: %d, File: %s, Search Term: \"%s\"\n",
           buffer_size, num_workers, log_file, search_term);

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
        perror("Failed to initialize barrier"); // Use perror for system errors
        exit_status = EXIT_FAILURE;
        goto cleanup;
    }

    // Allocate and initialize worker arguments and thread IDs
    worker_args = (WorkerArgs *)calloc(num_workers, sizeof(WorkerArgs)); // Use calloc
    worker_tids = (pthread_t *)malloc(num_workers * sizeof(pthread_t));

    if (!worker_args || !worker_tids) {
        perror("Memory allocation failed for worker structures");
        exit_status = EXIT_FAILURE;
        goto cleanup;
    }

    // Initialize worker arguments
    for (i = 0; i < num_workers; i++) {
        worker_args[i].thread_id = i + 1; // Start thread IDs from 1 for clearer output
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
            perror("Failed to create worker thread"); // Use perror
            // If thread creation fails, set termination flag and exit loop
            terminate_flag = 1;
            // Join already created threads before cleanup
            for (int j = 0; j < i; j++) {
                 // Use a timeout if necessary, but join is safer here
                pthread_join(worker_tids[j], NULL);
            }
            // No need to signal conditions here as signal handler does it.
            // Join manager thread (might be stuck on full buffer, handler helps)
            pthread_join(manager_tid, NULL);

            exit_status = EXIT_FAILURE;
            goto cleanup;
        }
    }

    // Wait for manager thread to finish
    pthread_join(manager_tid, NULL);

    // Wait for all worker threads to finish
    for (i = 0; i < num_workers; i++) {
        // If termination was requested, the signal handler has already broadcast.
        // Just join.
        pthread_join(worker_tids[i], NULL);
    }

    // Only generate report if not terminated by signal
    if (!terminate_flag) {
        generate_summary_report(worker_args, num_workers);
    } else {
        fprintf(stderr, "Program terminated by signal.\n"); // Add a message
    }


cleanup: // Centralized cleanup label
    if (worker_args) free(worker_args);
    if (worker_tids) free(worker_tids);
    // Barrier destroy is safe even if not initialized, if checked for return value
    // but simpler to destroy only if initialized.
    // Given the goto flow, check if barrier init succeeded implicitly.
    // Safer: Use a flag for barrier initialization state. For this complexity, OK.
    if (buffer) { // Only destroy barrier if buffer was initialized (implies barrier likely initialized)
         pthread_barrier_destroy(&barrier);
         buffer_destroy(buffer);
    }

    return exit_status;
}

// Manager thread function
void *manager_thread(void *arg) {
    ManagerArgs *args = (ManagerArgs *)arg;
    FILE *file = fopen(args->log_file, "r");
    char line[1024]; // Max line length
    size_t len;

    if (!file) {
        fprintf(stderr, "Error: Could not open file %s: %s\n", args->log_file, strerror(errno)); // Use strerror
        buffer_add_eof_marker(args->buffer); // Signal workers to stop
        return NULL;
    }

    // Read file line by line until EOF or termination
    while (fgets(line, sizeof(line), file) != NULL && !terminate_flag) {
        len = strlen(line);
        if (len > 0 && line[len-1] == '\n') {
            line[len-1] = '\0';
        }

        buffer_add(args->buffer, line);
    }

    // Add EOF marker unless terminated
    if (!terminate_flag) {
        buffer_add_eof_marker(args->buffer);
    } else {
        // If terminated, signal handler already woke up workers.
        // No need for a specific EOF marker add, buffer_remove handles termination.
    }

    fclose(file);
    return NULL;
}

// Worker thread function
void *worker_thread(void *arg) {
    WorkerArgs *args = (WorkerArgs *)arg;
    char *line;

    printf("Worker %d started\n", args->thread_id);

    // Process lines until buffer is empty and EOF marker is reached, or termination
    while (true) {
        // Get a line from the buffer
        line = buffer_remove(args->buffer);

        // buffer_remove returns NULL on EOF or termination when buffer is empty
        if (line == NULL) {
            break;
        }

        // Search for the term in the line
        if (line_contains_term(line, args->search_term)) {
            args->match_count++;
            printf("Worker %d found match: %s\n", args->thread_id, line);
        }

        free(line); // Free the line copy
    }

    // Report matches found by this worker only if not terminating
    if (!terminate_flag) {
        report_worker_matches(args->thread_id, args->match_count);

        // Wait at barrier for all workers before summary report
        // Barrier wait returns PTHREAD_BARRIER_SERIAL_THREAD for one thread.
        // Check return value or just ignore if not needed. Ignoring is simpler.
        pthread_barrier_wait(args->barrier);
    } else {
         printf("Worker %d exiting due to termination request.\n", args->thread_id);
    }

    return NULL;
}