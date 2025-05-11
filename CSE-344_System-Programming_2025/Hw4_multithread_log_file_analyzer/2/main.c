#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#ifdef _WIN32
#include <io.h>
#define F_OK 0
#define access _access
#else
#include <unistd.h>
#endif
#include <errno.h>
#include "buffer.h"
#include "utils.h"

#define MAX_LINE_LENGTH 4096

// Worker thread argument structure
typedef struct {
    int thread_id;
    Buffer *buffer;
    char *search_term;
    int match_count;
    pthread_barrier_t *barrier;
} WorkerArgs;

// Global total match count for summary
static int total_matches = 0;
static pthread_mutex_t total_mutex = PTHREAD_MUTEX_INITIALIZER;

// Worker thread function
void* worker_thread(void *arg) {
    WorkerArgs *args = (WorkerArgs*)arg;
    char *line;
    int local_matches = 0;

    while (!exit_flag) {
        // Get a line from the buffer
        line = buffer_remove(args->buffer);
        
        // If NULL returned, we've reached EOF and buffer is empty
        if (line == NULL) {
            break;
        }

        // Search for the term in the line
        if (strstr(line, args->search_term) != NULL) {
            local_matches++;
        }

        // Free the line (we own it now)
        free(line);
    }

    // Save our match count
    args->match_count = local_matches;

    // Add to total count (protected by mutex)
    pthread_mutex_lock(&total_mutex);
    total_matches += local_matches;
    pthread_mutex_unlock(&total_mutex);

    // Print individual worker results
    printf("Worker %d found %d matches\n", args->thread_id, local_matches);

    // Wait at barrier for all threads to finish
    pthread_barrier_wait(args->barrier);

    // Thread 0 is responsible for printing the summary
    if (args->thread_id == 0) {
        printf("\n--- Summary Report ---\n");
        printf("Total matches found: %d\n", total_matches);
        printf("---------------------\n");
    }

    return NULL;
}

// Main function (manager thread)
int main(int argc, char *argv[]) {
    // Parse command line arguments
    ProgramArgs args;
    if (!parse_arguments(argc, argv, &args)) {
        return EXIT_FAILURE;
    }

    // Set up signal handling for SIGINT
    setup_signal_handling();

    // Print program parameters
    printf("Analyzing log file: %s\n", args.log_file);
    printf("Buffer size: %d\n", args.buffer_size);
    printf("Number of workers: %d\n", args.num_workers);
    printf("Search term: \"%s\"\n\n", args.search_term);

    // Initialize the shared buffer
    Buffer *buffer = buffer_init(args.buffer_size);
    if (!buffer) {
        fprintf(stderr, "Failed to initialize buffer\n");
        return EXIT_FAILURE;
    }

    // Set up barrier for worker synchronization
    pthread_barrier_t barrier;
    if (pthread_barrier_init(&barrier, NULL, args.num_workers) != 0) {
        perror("Failed to initialize barrier");
        buffer_destroy(buffer);
        return EXIT_FAILURE;
    }

    // Create worker thread arguments and threads
    pthread_t *workers = (pthread_t*)malloc(args.num_workers * sizeof(pthread_t));
    WorkerArgs *worker_args = (WorkerArgs*)malloc(args.num_workers * sizeof(WorkerArgs));
    
    if (!workers || !worker_args) {
        perror("Failed to allocate memory for workers");
        if (workers) free(workers);
        if (worker_args) free(worker_args);
        pthread_barrier_destroy(&barrier);
        buffer_destroy(buffer);
        return EXIT_FAILURE;
    }

    // Initialize and create worker threads
    for (int i = 0; i < args.num_workers; i++) {
        worker_args[i].thread_id = i;
        worker_args[i].buffer = buffer;
        worker_args[i].search_term = args.search_term;
        worker_args[i].match_count = 0;
        worker_args[i].barrier = &barrier;

        int rc = pthread_create(&workers[i], NULL, worker_thread, &worker_args[i]);
        if (rc != 0) {
            fprintf(stderr, "Failed to create worker thread %d: %s\n", i, strerror(rc));
            
            // Cleanup and exit
            buffer_mark_eof(buffer);  // Signal EOF to any running threads
            
            // Wait for created threads to finish
            for (int j = 0; j < i; j++) {
                pthread_join(workers[j], NULL);
            }
            
            free(workers);
            free(worker_args);
            pthread_barrier_destroy(&barrier);
            buffer_destroy(buffer);
            return EXIT_FAILURE;
        }
    }

    // Manager thread: read the file and fill the buffer
    FILE *file = fopen(args.log_file, "r");
    if (!file) {
        perror("Failed to open log file");
        
        // Signal EOF and clean up
        buffer_mark_eof(buffer);
        
        for (int i = 0; i < args.num_workers; i++) {
            pthread_join(workers[i], NULL);
        }
        
        free(workers);
        free(worker_args);
        pthread_barrier_destroy(&barrier);
        buffer_destroy(buffer);
        return EXIT_FAILURE;
    }

    // Read file line by line
    char line[MAX_LINE_LENGTH];
    while (!exit_flag && fgets(line, sizeof(line), file) != NULL) {
        // Remove newline character if present
        size_t len = strlen(line);
        if (len > 0 && line[len-1] == '\n') {
            line[len-1] = '\0';
        }
        
        // Add line to buffer
        if (buffer_add(buffer, line) != 0) {
            if (exit_flag) {
                break;  // If we're exiting, this is expected
            } else {
                fprintf(stderr, "Failed to add line to buffer\n");
                break;
            }
        }
    }

    // Mark EOF in buffer
    buffer_mark_eof(buffer);
    fclose(file);

    // Wait for all worker threads to finish
    for (int i = 0; i < args.num_workers; i++) {
        pthread_join(workers[i], NULL);
    }

    // Clean up resources
    free(workers);
    free(worker_args);
    pthread_barrier_destroy(&barrier);
    buffer_destroy(buffer);
    pthread_mutex_destroy(&total_mutex);

    return EXIT_SUCCESS;
}
