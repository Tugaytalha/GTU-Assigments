/**
 * chatserver_log.c - Logging implementation
 * CSE 344 - System Programming
 */

#include "chatserver_log.h"
#include <stdarg.h>

/**
 * Log a message to the log file with timestamp
 */
void log_message(const char* format, ...) {
    va_list args;
    time_t now;
    struct tm *timeinfo;
    char timestamp[64];
    
    if (!log_file) return;
    
    /* Get current time */
    time(&now);
    timeinfo = localtime(&now);
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", timeinfo);
    
    /* Lock mutex to ensure thread safety */
    pthread_mutex_lock(&log_mutex);
    
    /* Print timestamp */
    fprintf(log_file, "%s - ", timestamp);
    
    /* Print message */
    va_start(args, format);
    vfprintf(log_file, format, args);
    va_end(args);
    
    /* Add newline */
    fprintf(log_file, "\n");
    
    /* Flush to ensure immediate write */
    fflush(log_file);
    
    /* Unlock mutex */
    pthread_mutex_unlock(&log_mutex);
    
    /* Also print to console for debugging */
    printf("%s - ", timestamp);
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    printf("\n");
}

/**
 * Initialize logging
 */
void log_init(void) {
    log_file = fopen(LOG_FILENAME, "w");
    if (!log_file) {
        perror("Failed to open log file");
        return;
    }
    
    log_message("[SERVER] Logging initialized");
}

/**
 * Close log file
 */
void log_close(void) {
    if (log_file) {
        log_message("[SERVER] Logging closed");
        fclose(log_file);
        log_file = NULL;
    }
}
