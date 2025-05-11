#ifndef BUFFER_H
#define BUFFER_H

#include <pthread.h>
#include <stdbool.h>

// Bounded buffer structure
typedef struct {
    char **data;           // Array of strings (lines from log file)
    int size;              // Maximum buffer capacity
    int count;             // Current number of items
    int in;                // Index for next insertion
    int out;               // Index for next removal
    bool eof_marker_added; // Flag to indicate if EOF marker has been added
    pthread_mutex_t mutex; // Mutex for synchronization
    pthread_cond_t not_full;  // Condition for buffer not full
    pthread_cond_t not_empty; // Condition for buffer not empty
} Buffer;

// Initialize buffer with given size
Buffer* buffer_init(int size);

// Free buffer resources
void buffer_destroy(Buffer *buffer);

// Add a line to the buffer (producer)
void buffer_add(Buffer *buffer, char *line);

// Get a line from the buffer (consumer)
// Returns NULL when EOF marker is reached
char* buffer_remove(Buffer *buffer);

// Add EOF marker to signal end of file
void buffer_add_eof_marker(Buffer *buffer);

#endif // BUFFER_H 