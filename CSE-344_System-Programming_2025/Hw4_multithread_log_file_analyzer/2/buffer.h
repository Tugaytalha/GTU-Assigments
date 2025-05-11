#ifndef BUFFER_H
#define BUFFER_H

#include <pthread.h>
#include <stdbool.h>

// Define a structure for our bounded buffer
typedef struct {
    char **data;           // Array of strings (lines from the log file)
    int capacity;          // Maximum capacity of the buffer
    int size;              // Current number of items in the buffer
    int in;                // Index for next producer insert
    int out;               // Index for next consumer remove
    bool eof_reached;      // Flag to indicate end of file processing
    pthread_mutex_t mutex; // Mutex for buffer access
    pthread_cond_t not_full;   // Condition variable for buffer not full
    pthread_cond_t not_empty;  // Condition variable for buffer not empty
} Buffer;

// Initialize the buffer with given capacity
Buffer* buffer_init(int capacity);

// Free all resources used by the buffer
void buffer_destroy(Buffer *buffer);

// Add a line to the buffer (used by manager thread)
// Returns 0 on success, -1 on error
int buffer_add(Buffer *buffer, char *line);

// Mark the buffer as having reached EOF (no more lines will be added)
void buffer_mark_eof(Buffer *buffer);

// Get a line from the buffer (used by worker threads)
// Returns NULL if EOF and buffer is empty
char* buffer_remove(Buffer *buffer);

// Check if we've reached EOF and the buffer is empty
bool buffer_is_done(Buffer *buffer);

#endif // BUFFER_H
