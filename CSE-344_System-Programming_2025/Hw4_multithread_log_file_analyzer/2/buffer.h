#ifndef BUFFER_H
#define BUFFER_H

#include <pthread.h>
#include <stdbool.h>
#include <signal.h> // For sig_atomic_t

// Bounded buffer structure
typedef struct {
    char **data;           // Array of strings (lines from log file)
    int size;              // Maximum buffer capacity
    int count;             // Current number of items
    int in;                // Index for next insertion
    int out;               // Index for next removal
    bool eof_marker_added; // Flag to indicate if EOF marker has been added and placed in buffer
    pthread_mutex_t mutex; // Mutex for synchronization
    pthread_cond_t not_full;  // Condition: buffer has space
    pthread_cond_t not_empty; // Condition: buffer has items
} Buffer;

// Initialize buffer with given size. Returns NULL on failure.
Buffer* buffer_init(int size);

// Free buffer resources, including all stored strings.
void buffer_destroy(Buffer *buffer);

// Add a line to the buffer (producer). Line is duplicated.
void buffer_add(Buffer *buffer, char *line);

// Get a line from the buffer (consumer).
// Returns NULL if EOF marker is reached or termination is signaled.
// Caller is responsible for freeing the returned string.
char* buffer_remove(Buffer *buffer);

// Add EOF marker (NULL) to signal end of input.
void buffer_add_eof_marker(Buffer *buffer);

#endif // BUFFER_H