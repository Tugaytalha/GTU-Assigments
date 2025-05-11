#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h> // For STDOUT_FILENO, write
#include "buffer.h"
#include "utils.h"

// External global flag for termination, defined in utils.c
extern volatile sig_atomic_t terminate_flag;

// Function prototypes
void *manager_thread(void *arg);
void *worker_thread(void *arg);

// Manager thread arguments
typedef struct {
    const char *log_file; // Use const char* for read-only string
    Buffer *buffer;
} ManagerArgs;

int main(int argc, char *argv[]) {
    int buffer_size, num_workers;
    char *log_file_arg, *search_term_arg; // Args from command line
    pthread_t manager_tid;
    pthread_t *worker_tids = NULL; // Initialize to NULL
    WorkerArgs *worker_args_array = NULL; // Initialize to NULL
    pthread_barrier_t barrier;
    Buffer *shared_buffer = NULL; // Initialize to NULL
    int i;

    setup_signal_handler();

    if (!parse_arguments(argc, argv, &buffer_size, &num_workers, &log_file_arg, &search_term_arg)) {
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    printf("Log Analyzer Starting...\n");
    printf("Config: Buffer Size: %d, Workers: %d, File: %s, Search: \"%s\"\n", 
           buffer_size, num_workers, log_file_arg, search_term_arg);

    shared_buffer = buffer_init(buffer_size);
    if (!shared_buffer) {
        fprintf(stderr, "Critical Error: Buffer initialization failed.\n");
        return EXIT_FAILURE;
    }
    set_global_buffer(shared_buffer); // For signal handler to access

    if (pthread_barrier_init(&barrier, NULL, num_workers) != 0) {
        perror("Error: Failed to initialize barrier");
        buffer_destroy(shared_buffer);
        return EXIT_FAILURE;
    }

    worker_args_array = (WorkerArgs *)malloc(num_workers * sizeof(WorkerArgs));
    if (!worker_args_array) {
        perror("Error: Worker arguments array allocation failed");
        pthread_barrier_destroy(&barrier);
        buffer_destroy(shared_buffer);
        return EXIT_FAILURE;
    }

    worker_tids = (pthread_t *)malloc(num_workers * sizeof(pthread_t));
    if (!worker_tids) {
        perror("Error: Worker TIDs array allocation failed");
        free(worker_args_array);
        pthread_barrier_destroy(&barrier);
        buffer_destroy(shared_buffer);
        return EXIT_FAILURE;
    }

    for (i = 0; i < num_workers; i++) {
        worker_args_array[i].thread_id = i;
        worker_args_array[i].search_term = search_term_arg; // All workers search for the same term
        worker_args_array[i].match_count = 0;
        worker_args_array[i].barrier = &barrier;
        worker_args_array[i].buffer = shared_buffer;
    }

    ManagerArgs manager_args_data = { .log_file = log_file_arg, .buffer = shared_buffer };

    if (pthread_create(&manager_tid, NULL, manager_thread, &manager_args_data) != 0) {
        perror("Error: Failed to create manager thread");
        free(worker_args_array);
        free(worker_tids);
        pthread_barrier_destroy(&barrier);
        buffer_destroy(shared_buffer);
        return EXIT_FAILURE;
    }

    for (i = 0; i < num_workers; i++) {
        if (pthread_create(&worker_tids[i], NULL, worker_thread, &worker_args_array[i]) != 0) {
            fprintf(stderr, "Error: Failed to create worker thread %d\n", i);
            terminate_flag = 1; // Signal other threads to attempt shutdown
            // Attempt to join already created threads and manager
            pthread_join(manager_tid, NULL); // Manager might still be running or adding EOF
            for (int j = 0; j < i; j++) {
                pthread_join(worker_tids[j], NULL);
            }
            // Wake up any potentially waiting threads
            if (shared_buffer) {
                pthread_mutex_lock(&shared_buffer->mutex);
                pthread_cond_broadcast(&shared_buffer->not_empty);
                pthread_cond_broadcast(&shared_buffer->not_full);
                pthread_mutex_unlock(&shared_buffer->mutex);
            }
            free(worker_args_array);
            free(worker_tids);
            pthread_barrier_destroy(&barrier);
            buffer_destroy(shared_buffer);
            return EXIT_FAILURE;
        }
    }

    pthread_join(manager_tid, NULL); // Wait for manager to finish reading file

    // Ensure workers are woken up if terminate_flag was set or EOF was added
    if (terminate_flag || (shared_buffer && shared_buffer->eof_marker_added)) {
        pthread_mutex_lock(&shared_buffer->mutex);
        pthread_cond_broadcast(&shared_buffer->not_empty);
        pthread_mutex_unlock(&shared_buffer->mutex);
    }
    
    for (i = 0; i < num_workers; i++) {
        pthread_join(worker_tids[i], NULL);
    }

    if (!terminate_flag) {
        generate_summary_report(worker_args_array, num_workers);
    } else {
        printf("Program terminated prematurely by signal. Summary report skipped.\n");
    }

    // Cleanup
    free(worker_args_array);
    free(worker_tids);
    pthread_barrier_destroy(&barrier);
    buffer_destroy(shared_buffer);

    printf("Log Analyzer finished.\n");
    return EXIT_SUCCESS;
}

// Manager thread: reads log file and adds lines to buffer
void *manager_thread(void *arg) {
    ManagerArgs *args = (ManagerArgs *)arg;
    FILE *file = fopen(args->log_file, "r");
    char current_line[MAX_LINE_LENGTH]; 
    
    if (!file) {
        fprintf(stderr, "Manager Error: Could not open file %s. Signaling EOF.\n", args->log_file);
        buffer_add_eof_marker(args->buffer); // Signal workers to stop
        return NULL;
    }

    // Read file line by line, add to buffer
    while (fgets(current_line, sizeof(current_line), file) && !terminate_flag) {
        size_t len = strlen(current_line);
        if (len > 0 && current_line[len-1] == '\n') {
            current_line[len-1] = '\0'; // Remove newline
        }
        buffer_add(args->buffer, current_line);
    }

    if (ferror(file)) {
        perror("Manager Error: Error reading from file");
    }
    // Always add EOF marker, even if terminated early, to ensure workers can exit clean_liney
    buffer_add_eof_marker(args->buffer);
    
    fclose(file);
    return NULL;
}

// Worker thread: removes lines from buffer, searches for term
void *worker_thread(void *arg) {
    WorkerArgs *args = (WorkerArgs *)arg;
    char *line_to_process;
    
    // A small message to confirm worker startup, can be removed for less verbosity
    // printf("Worker %d started, searching for \"%s\"\n", args->thread_id, args->search_term);
    
    while (!terminate_flag) {
        line_to_process = buffer_remove(args->buffer);
        
        if (line_to_process == NULL) { // EOF or termination signal
            break; 
        }
        
        if (line_contains_term(line_to_process, args->search_term)) {
            args->match_count++;
            // printf("Worker %d found match: %s\n", args->thread_id, line_to_process); // Verbose
        }
        
        free(line_to_process); // Line was strdup'd by buffer_add
    }
    
    if (!terminate_flag) {
        report_worker_matches(args->thread_id, args->match_count);
        pthread_barrier_wait(args->barrier); // Synchronize before main prints summary
    } else {
        // If terminated, worker might not reach barrier, or barrier might be destroyed.
        // Simply exit.
        // char msg[100];
        // snprintf(msg, sizeof(msg), "Worker %d exiting due to termination signal.\n", args->thread_id);
        // write(STDOUT_FILENO, msg, strlen(msg)); // write is signal-safe
    }
    
    return NULL;
}