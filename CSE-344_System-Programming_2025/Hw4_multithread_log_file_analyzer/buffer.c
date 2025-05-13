#include "buffer.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h> // Added for stderr in error handling

// Import the global terminate flag
extern volatile sig_atomic_t terminate_flag;

// Initialize buffer with given size
Buffer* buffer_init(int size) {
    Buffer *buffer = (Buffer*)malloc(sizeof(Buffer));
    if (!buffer) {
        perror("Failed to allocate buffer struct"); // Use perror for system errors
        return NULL;
    }

    buffer->data = (char**)calloc(size, sizeof(char*)); // Use calloc to zero-initialize pointers
    if (!buffer->data) {
        perror("Failed to allocate buffer data array");
        free(buffer);
        return NULL;
    }

    buffer->size = size;
    buffer->count = 0;
    buffer->in = 0;
    buffer->out = 0;
    buffer->eof_marker_added = false;

    if (pthread_mutex_init(&buffer->mutex, NULL) != 0) {
        perror("Failed to initialize buffer mutex");
        free(buffer->data);
        free(buffer);
        return NULL;
    }
    if (pthread_cond_init(&buffer->not_full, NULL) != 0) {
        perror("Failed to initialize buffer not_full condition variable");
        pthread_mutex_destroy(&buffer->mutex); // Clean up initialized mutex
        free(buffer->data);
        free(buffer);
        return NULL;
    }
    if (pthread_cond_init(&buffer->not_empty, NULL) != 0) {
        perror("Failed to initialize buffer not_empty condition variable");
        pthread_mutex_destroy(&buffer->mutex); // Clean up initialized primitives
        pthread_cond_destroy(&buffer->not_full);
        free(buffer->data);
        free(buffer);
        return NULL;
    }

    return buffer;
}

// Free buffer resources
void buffer_destroy(Buffer *buffer) {
    if (!buffer) return;

    pthread_mutex_lock(&buffer->mutex); // Lock before accessing buffer state

    // Free all strings currently in the buffer
    for (int i = 0; i < buffer->count; i++) {
        int idx = (buffer->out + i) % buffer->size;
        if (buffer->data[idx] != NULL) { // Check for NULL EOF marker
            free(buffer->data[idx]);
            buffer->data[idx] = NULL; // Avoid double free (though loop logic should prevent)
        }
    }
    pthread_mutex_unlock(&buffer->mutex); // Unlock after processing data

    // Free the buffer array
    free(buffer->data);
    buffer->data = NULL; // Prevent double free

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

    // Wait until there's space or termination is requested
    while (buffer->count == buffer->size && !terminate_flag) {
        pthread_cond_wait(&buffer->not_full, &buffer->mutex);
    }

    // If termination requested, exit without adding
    if (terminate_flag) {
        pthread_mutex_unlock(&buffer->mutex);
        return;
    }

    char *line_copy = strdup(line);
    if (!line_copy) {
        perror("Failed to duplicate line in buffer_add");
        // Handle error: In this case, we might skip this line and continue.
        // A more complex program might need a dedicated error state.
        pthread_mutex_unlock(&buffer->mutex);
        return;
    }

    buffer->data[buffer->in] = line_copy;
    buffer->in = (buffer->in + 1) % buffer->size;
    buffer->count++;

    // Signal that the buffer is not empty
    pthread_cond_signal(&buffer->not_empty);
    pthread_mutex_unlock(&buffer->mutex);
}

// Add EOF marker to signal end of file
void buffer_add_eof_marker(Buffer *buffer) {
    pthread_mutex_lock(&buffer->mutex);

    // Wait until there's space or termination is requested
    while (buffer->count == buffer->size && !terminate_flag) {
        pthread_cond_wait(&buffer->not_full, &buffer->mutex);
    }

    // If termination requested, exit without adding marker
    if (terminate_flag) {
        pthread_mutex_unlock(&buffer->mutex);
        return;
    }

    buffer->data[buffer->in] = NULL; // NULL serves as the EOF marker
    buffer->in = (buffer->in + 1) % buffer->size;
    buffer->count++;
    buffer->eof_marker_added = true;

    // Signal that the buffer is not empty
    pthread_cond_broadcast(&buffer->not_empty); // Broadcast to wake all waiting workers
    pthread_mutex_unlock(&buffer->mutex);
}

// Get a line from the buffer (consumer)
char* buffer_remove(Buffer *buffer) {
    pthread_mutex_lock(&buffer->mutex);

    // Wait until there's an item, EOF is added, or termination is requested
    while (buffer->count == 0 && !buffer->eof_marker_added && !terminate_flag) {
        pthread_cond_wait(&buffer->not_empty, &buffer->mutex);
    }

    // If buffer is empty and EOF marker or termination is present, signal done
    if (buffer->count == 0) {
        pthread_mutex_unlock(&buffer->mutex);
        return NULL;
    }

    // Remove the item from the buffer
    char *line = buffer->data[buffer->out];
    buffer->data[buffer->out] = NULL; // Clear the pointer after removal
    buffer->out = (buffer->out + 1) % buffer->size;
    buffer->count--;

    // Signal that the buffer is not full
    pthread_cond_signal(&buffer->not_full);
    pthread_mutex_unlock(&buffer->mutex);

    return line;
}