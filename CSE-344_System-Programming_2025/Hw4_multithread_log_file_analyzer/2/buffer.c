#include "buffer.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

// Initialize the buffer with given capacity
Buffer* buffer_init(int capacity) {
    if (capacity <= 0) {
        return NULL;
    }

    Buffer *buffer = (Buffer*)malloc(sizeof(Buffer));
    if (!buffer) {
        perror("Failed to allocate buffer");
        return NULL;
    }

    buffer->data = (char**)malloc(capacity * sizeof(char*));
    if (!buffer->data) {
        perror("Failed to allocate buffer data");
        free(buffer);
        return NULL;
    }

    buffer->capacity = capacity;
    buffer->size = 0;
    buffer->in = 0;
    buffer->out = 0;
    buffer->eof_reached = false;

    // Initialize synchronization primitives
    if (pthread_mutex_init(&buffer->mutex, NULL) != 0) {
        perror("Failed to initialize buffer mutex");
        free(buffer->data);
        free(buffer);
        return NULL;
    }

    if (pthread_cond_init(&buffer->not_full, NULL) != 0) {
        perror("Failed to initialize not_full condition variable");
        pthread_mutex_destroy(&buffer->mutex);
        free(buffer->data);
        free(buffer);
        return NULL;
    }

    if (pthread_cond_init(&buffer->not_empty, NULL) != 0) {
        perror("Failed to initialize not_empty condition variable");
        pthread_cond_destroy(&buffer->not_full);
        pthread_mutex_destroy(&buffer->mutex);
        free(buffer->data);
        free(buffer);
        return NULL;
    }

    return buffer;
}

// Free all resources used by the buffer
void buffer_destroy(Buffer *buffer) {
    if (!buffer) {
        return;
    }

    // Free all remaining strings in the buffer
    pthread_mutex_lock(&buffer->mutex);
    for (int i = 0; i < buffer->size; i++) {
        int idx = (buffer->out + i) % buffer->capacity;
        if (buffer->data[idx]) {
            free(buffer->data[idx]);
            buffer->data[idx] = NULL;
        }
    }
    pthread_mutex_unlock(&buffer->mutex);

    // Destroy synchronization primitives
    pthread_cond_destroy(&buffer->not_empty);
    pthread_cond_destroy(&buffer->not_full);
    pthread_mutex_destroy(&buffer->mutex);

    // Free buffer data
    free(buffer->data);
    free(buffer);
}

// Add a line to the buffer (used by manager thread)
int buffer_add(Buffer *buffer, char *line) {
    if (!buffer || !line) {
        return -1;
    }

    pthread_mutex_lock(&buffer->mutex);

    // Wait while the buffer is full and we haven't reached EOF
    while (buffer->size == buffer->capacity) {
        pthread_cond_wait(&buffer->not_full, &buffer->mutex);
    }

    // If someone marked EOF while we were waiting, abort
    if (buffer->eof_reached) {
        pthread_mutex_unlock(&buffer->mutex);
        return -1;
    }

    // Add the line to the buffer (make a copy)
    buffer->data[buffer->in] = strdup(line);
    if (!buffer->data[buffer->in]) {
        pthread_mutex_unlock(&buffer->mutex);
        return -1;
    }

    // Update buffer state
    buffer->in = (buffer->in + 1) % buffer->capacity;
    buffer->size++;

    // Signal that the buffer is not empty anymore
    pthread_cond_signal(&buffer->not_empty);
    pthread_mutex_unlock(&buffer->mutex);

    return 0;
}

// Mark the buffer as having reached EOF (no more lines will be added)
void buffer_mark_eof(Buffer *buffer) {
    if (!buffer) {
        return;
    }

    pthread_mutex_lock(&buffer->mutex);
    buffer->eof_reached = true;
    
    // Signal all waiting consumers that EOF has been reached
    pthread_cond_broadcast(&buffer->not_empty);
    pthread_mutex_unlock(&buffer->mutex);
}

// Get a line from the buffer (used by worker threads)
char* buffer_remove(Buffer *buffer) {
    if (!buffer) {
        return NULL;
    }

    pthread_mutex_lock(&buffer->mutex);

    // Wait while the buffer is empty and EOF has not been reached
    while (buffer->size == 0 && !buffer->eof_reached) {
        pthread_cond_wait(&buffer->not_empty, &buffer->mutex);
    }

    // If buffer is empty and EOF has been reached, we're done
    if (buffer->size == 0 && buffer->eof_reached) {
        pthread_mutex_unlock(&buffer->mutex);
        return NULL;
    }

    // Get the line from the buffer
    char *line = buffer->data[buffer->out];
    buffer->data[buffer->out] = NULL;

    // Update buffer state
    buffer->out = (buffer->out + 1) % buffer->capacity;
    buffer->size--;

    // Signal that the buffer is not full anymore
    pthread_cond_signal(&buffer->not_full);
    pthread_mutex_unlock(&buffer->mutex);

    return line;
}

// Check if we've reached EOF and the buffer is empty
bool buffer_is_done(Buffer *buffer) {
    if (!buffer) {
        return true;
    }

    pthread_mutex_lock(&buffer->mutex);
    bool done = (buffer->eof_reached && buffer->size == 0);
    pthread_mutex_unlock(&buffer->mutex);

    return done;
}
