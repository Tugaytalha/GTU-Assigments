#include "buffer.h"
#include <stdlib.h>
#include <string.h>

// Import the global terminate flag
extern volatile sig_atomic_t terminate_flag;

// Initialize buffer with given size
Buffer* buffer_init(int size) {
    Buffer *buffer = (Buffer*)malloc(sizeof(Buffer));
    if (!buffer) return NULL;

    buffer->data = (char**)malloc(size * sizeof(char*));
    if (!buffer->data) {
        free(buffer);
        return NULL;
    }

    buffer->size = size;
    buffer->count = 0;
    buffer->in = 0;
    buffer->out = 0;
    buffer->eof_marker_added = false;

    pthread_mutex_init(&buffer->mutex, NULL);
    pthread_cond_init(&buffer->not_full, NULL);
    pthread_cond_init(&buffer->not_empty, NULL);

    return buffer;
}

// Free buffer resources
void buffer_destroy(Buffer *buffer) {
    if (!buffer) return;

    // Free all strings in the buffer
    for (int i = 0; i < buffer->count; i++) {
        int idx = (buffer->out + i) % buffer->size;
        if (buffer->data[idx] != NULL) {
            free(buffer->data[idx]);
        }
    }

    // Free the buffer array
    free(buffer->data);

    // Destroy synchronization primitives
    pthread_mutex_destroy(&buffer->mutex);
    pthread_cond_destroy(&buffer->not_full);
    pthread_cond_destroy(&buffer->not_empty);

    // Free the buffer struct
    free(buffer);
}

// Add a line to the buffer (producer)
void buffer_add(Buffer *buffer, char *line) {
    pthread_mutex_lock(&buffer->mutex);

    // Wait until there's space in the buffer
    while (buffer->count == buffer->size) {
        pthread_cond_wait(&buffer->not_full, &buffer->mutex);
    }

    // Make a copy of the line and add it to the buffer
    buffer->data[buffer->in] = strdup(line);
    buffer->in = (buffer->in + 1) % buffer->size;
    buffer->count++;

    // Signal that the buffer is not empty
    pthread_cond_signal(&buffer->not_empty);
    pthread_mutex_unlock(&buffer->mutex);
}

// Add EOF marker to signal end of file
void buffer_add_eof_marker(Buffer *buffer) {
    pthread_mutex_lock(&buffer->mutex);

    // Wait until there's space in the buffer
    while (buffer->count == buffer->size) {
        pthread_cond_wait(&buffer->not_full, &buffer->mutex);
    }

    // Use NULL as EOF marker
    buffer->data[buffer->in] = NULL;
    buffer->in = (buffer->in + 1) % buffer->size;
    buffer->count++;
    buffer->eof_marker_added = true;

    // Signal that the buffer is not empty
    pthread_cond_signal(&buffer->not_empty);
    pthread_mutex_unlock(&buffer->mutex);
}

// Get a line from the buffer (consumer)
char* buffer_remove(Buffer *buffer) {
    pthread_mutex_lock(&buffer->mutex);

    // Wait until there's an item in the buffer or termination is requested
    while (buffer->count == 0 && !terminate_flag) {
        pthread_cond_wait(&buffer->not_empty, &buffer->mutex);
    }

    // If termination is requested and the buffer is empty, return NULL
    if (buffer->count == 0 && terminate_flag) {
        pthread_mutex_unlock(&buffer->mutex);
        return NULL;
    }

    // Remove the item from the buffer
    char *line = buffer->data[buffer->out];
    buffer->out = (buffer->out + 1) % buffer->size;
    buffer->count--;

    // Signal that the buffer is not full
    pthread_cond_signal(&buffer->not_full);
    pthread_mutex_unlock(&buffer->mutex);

    return line;
} 