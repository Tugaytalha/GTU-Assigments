#include "buffer.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h> // For fprintf in error cases

// Import the global terminate flag
extern volatile sig_atomic_t terminate_flag;

// Initialize buffer with given size
Buffer* buffer_init(int size) {
    Buffer *buffer = (Buffer*)malloc(sizeof(Buffer));
    if (!buffer) {
        perror("Failed to allocate memory for Buffer struct");
        return NULL;
    }

    buffer->data = (char**)malloc(size * sizeof(char*));
    if (!buffer->data) {
        perror("Failed to allocate memory for buffer data array");
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
        pthread_mutex_destroy(&buffer->mutex);
        free(buffer->data);
        free(buffer);
        return NULL;
    }
    if (pthread_cond_init(&buffer->not_empty, NULL) != 0) {
        perror("Failed to initialize buffer not_empty condition variable");
        pthread_mutex_destroy(&buffer->mutex);
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

    // Free all dynamically allocated strings remaining in the buffer
    for (int i = 0; i < buffer->count; i++) {
        int idx = (buffer->out + i) % buffer->size;
        if (buffer->data[idx] != NULL) { // EOF marker is NULL, other lines are strdup'd
            free(buffer->data[idx]);
        }
    }

    free(buffer->data);

    pthread_mutex_destroy(&buffer->mutex);
    pthread_cond_destroy(&buffer->not_full);
    pthread_cond_destroy(&buffer->not_empty);

    free(buffer);
}

// Add a line to the buffer (producer)
void buffer_add(Buffer *buffer, char *line) {
    pthread_mutex_lock(&buffer->mutex);

    // Wait until there's space in the buffer or termination is signaled
    while (buffer->count == buffer->size && !terminate_flag) {
        pthread_cond_wait(&buffer->not_full, &buffer->mutex);
    }

    // If termination was signaled while waiting, or if buffer is full and termination, exit
    if (terminate_flag) {
        pthread_mutex_unlock(&buffer->mutex);
        // Optionally, signal not_full in case other producers are waiting, though less critical here
        // pthread_cond_signal(&buffer->not_full); 
        return;
    }

    char *line_copy = strdup(line);
    if (!line_copy) {
        fprintf(stderr, "Error: strdup failed in buffer_add. Line not added.\n");
        // If strdup fails, we don't add to buffer, but need to unlock.
        // We could signal not_full, but if it failed due to memory, other strdups might also fail.
        pthread_mutex_unlock(&buffer->mutex);
        return;
    }

    buffer->data[buffer->in] = line_copy;
    buffer->in = (buffer->in + 1) % buffer->size;
    buffer->count++;

    pthread_cond_signal(&buffer->not_empty);
    pthread_mutex_unlock(&buffer->mutex);
}

// Add EOF marker to signal end of file
void buffer_add_eof_marker(Buffer *buffer) {
    pthread_mutex_lock(&buffer->mutex);

    // Wait until there's space in the buffer or termination is signaled
    // Even for EOF, we should respect the terminate_flag if it's set early.
    while (buffer->count == buffer->size && !terminate_flag) {
        pthread_cond_wait(&buffer->not_full, &buffer->mutex);
    }
    
    // If terminate_flag is set, we might skip adding EOF if another mechanism handles shutdown.
    // However, adding EOF is generally important for workers to terminate naturally.
    // If buffer is full and termination is requested, we might not be able to add EOF.
    if (terminate_flag && buffer->count == buffer->size) {
         pthread_mutex_unlock(&buffer->mutex);
         return; // Cannot add EOF if buffer is full and termination is requested
    }
    // If terminate_flag is set but there is space, we can still add EOF to ensure workers shutdown.

    buffer->data[buffer->in] = NULL; // Use NULL as EOF marker
    buffer->in = (buffer->in + 1) % buffer->size;
    buffer->count++;
    buffer->eof_marker_added = true;

    // Signal that the buffer is not empty (it now contains EOF or another item)
    pthread_cond_broadcast(&buffer->not_empty); // Broadcast in case multiple workers are waiting for EOF
    pthread_mutex_unlock(&buffer->mutex);
}

// Get a line from the buffer (consumer)
char* buffer_remove(Buffer *buffer) {
    pthread_mutex_lock(&buffer->mutex);

    // Wait until there's an item or termination is requested
    while (buffer->count == 0 && !terminate_flag) {
        pthread_cond_wait(&buffer->not_empty, &buffer->mutex);
    }

    // If buffer is empty AND (termination is requested OR EOF has been added and consumed by all)
    // The eof_marker_added check helps ensure that even if terminate_flag is late,
    // workers will eventually stop if EOF was the last item.
    if (buffer->count == 0 && (terminate_flag || buffer->eof_marker_added)) {
        pthread_mutex_unlock(&buffer->mutex);
        return NULL; // Signal to consumer to stop
    }
    // If count is 0 but terminate_flag is set, we must return NULL
    if (buffer->count == 0 && terminate_flag) {
        pthread_mutex_unlock(&buffer->mutex);
        return NULL;
    }
    // If count is 0, this should ideally not be reached if the above conditions are met.
    // But as a safeguard:
    if (buffer->count == 0) {
        pthread_mutex_unlock(&buffer->mutex);
        return NULL;
    }


    char *line = buffer->data[buffer->out];
    buffer->data[buffer->out] = NULL; // Optional: Clear pointer after removal
    buffer->out = (buffer->out + 1) % buffer->size;
    buffer->count--;

    pthread_cond_signal(&buffer->not_full);
    pthread_mutex_unlock(&buffer->mutex);

    return line;
}