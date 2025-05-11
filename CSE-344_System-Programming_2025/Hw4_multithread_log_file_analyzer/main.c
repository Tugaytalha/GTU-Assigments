#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include "buffer.h"
#include "utils.h"

// External global flag for termination
extern volatile sig_atomic_t terminate_flag;

// Function prototypes
void *manager_thread(void *arg);
void *worker_thread(void *arg);

// Manager thread arguments
typedef struct {
    char *log_file;
    Buffer *buffer;
} ManagerArgs;

int main(int argc, char *argv[]) {
    int buffer_size, num_workers;
    char *log_file, *search_term;
    pthread_t manager_tid;
    pthread_t *worker_tids;
    WorkerArgs *worker_args;
    pthread_barrier_t barrier;
    Buffer *buffer;
    int i;

    // Set up signal handler for SIGINT
    setup_signal_handler();

    // Parse command line arguments
    if (!parse_arguments(argc, argv, &buffer_size, &num_workers, &log_file, &search_term)) {
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    printf("Starting Log Analyzer\n");
    printf("Buffer Size: %d, Workers: %d, File: %s, Search Term: \"%s\"\n", 
           buffer_size, num_workers, log_file, search_term);

    // Initialize buffer
    buffer = buffer_init(buffer_size);
    if (!buffer) {
        fprintf(stderr, "Error: Failed to initialize buffer\n");
        return EXIT_FAILURE;
    }

    // Set global buffer reference for signal handling
    set_global_buffer(buffer);

    // Initialize barrier for worker threads
    if (pthread_barrier_init(&barrier, NULL, num_workers) != 0) {
        fprintf(stderr, "Error: Failed to initialize barrier\n");
        buffer_destroy(buffer);
        return EXIT_FAILURE;
    }

    // Allocate and initialize worker arguments
    worker_args = (WorkerArgs *)malloc(num_workers * sizeof(WorkerArgs));
    worker_tids = (pthread_t *)malloc(num_workers * sizeof(pthread_t));
    
    if (!worker_args || !worker_tids) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        if (worker_args) free(worker_args);
        if (worker_tids) free(worker_tids);
        pthread_barrier_destroy(&barrier);
        buffer_destroy(buffer);
        return EXIT_FAILURE;
    }

    // Initialize worker arguments
    for (i = 0; i < num_workers; i++) {
        worker_args[i].thread_id = i;
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
        fprintf(stderr, "Error: Failed to create manager thread\n");
        free(worker_args);
        free(worker_tids);
        pthread_barrier_destroy(&barrier);
        buffer_destroy(buffer);
        return EXIT_FAILURE;
    }

    // Create worker threads
    for (i = 0; i < num_workers; i++) {
        if (pthread_create(&worker_tids[i], NULL, worker_thread, &worker_args[i]) != 0) {
            fprintf(stderr, "Error: Failed to create worker thread %d\n", i);
            // Wait for manager thread to finish and clean up
            pthread_join(manager_tid, NULL);
            for (int j = 0; j < i; j++) {
                pthread_join(worker_tids[j], NULL);
            }
            free(worker_args);
            free(worker_tids);
            pthread_barrier_destroy(&barrier);
            buffer_destroy(buffer);
            return EXIT_FAILURE;
        }
    }

    // Wait for manager thread to finish
    pthread_join(manager_tid, NULL);

    // Wait for all worker threads to finish with a timeout
    for (i = 0; i < num_workers; i++) {
        // If termination was requested, don't wait forever
        if (terminate_flag) {
            // Send another broadcast to ensure workers wake up
            pthread_mutex_lock(&buffer->mutex);
            pthread_cond_broadcast(&buffer->not_empty);
            pthread_mutex_unlock(&buffer->mutex);
        }
        
        pthread_join(worker_tids[i], NULL);
    }

    // Only generate report if normal termination
    if (!terminate_flag) {
        generate_summary_report(worker_args, num_workers);
    }

    // Clean up resources
    free(worker_args);
    free(worker_tids);
    pthread_barrier_destroy(&barrier);
    buffer_destroy(buffer);

    return EXIT_SUCCESS;
}

// Manager thread function
void *manager_thread(void *arg) {
    ManagerArgs *args = (ManagerArgs *)arg;
    FILE *file = fopen(args->log_file, "r");
    char line[1024];  // Assuming max line length of 1024 chars
    
    if (!file) {
        fprintf(stderr, "Error: Could not open file %s\n", args->log_file);
        buffer_add_eof_marker(args->buffer);  // Signal workers to stop
        return NULL;
    }

    // Read file line by line
    while (fgets(line, sizeof(line), file) && !terminate_flag) {
        // Remove newline character
        size_t len = strlen(line);
        if (len > 0 && line[len-1] == '\n') {
            line[len-1] = '\0';
        }
        
        // Add line to buffer
        buffer_add(args->buffer, line);
    }

    // Add EOF marker to signal end of file
    buffer_add_eof_marker(args->buffer);
    
    fclose(file);
    return NULL;
}

// Worker thread function
void *worker_thread(void *arg) {
    WorkerArgs *args = (WorkerArgs *)arg;
    char *line;
    
    printf("Worker %d started\n", args->thread_id);
    
    // Process lines until EOF marker is reached or termination is requested
    while (!terminate_flag) {
        // Get a line from the buffer
        line = buffer_remove(args->buffer);
        
        // Check if this is the EOF marker or if termination was requested
        if (line == NULL) {
            break;
        }
        
        // Search for the term in the line
        if (line_contains_term(line, args->search_term)) {
            args->match_count++;
            printf("Worker %d found match: %s\n", args->thread_id, line);
        }
        
        free(line);  // Free the line as we're done with it
    }
    
    // Report matches found by this worker (only if not terminating)
    if (!terminate_flag) {
        report_worker_matches(args->thread_id, args->match_count);
        
        // Wait at barrier for all workers
        pthread_barrier_wait(args->barrier);
    }
    
    return NULL;
} 