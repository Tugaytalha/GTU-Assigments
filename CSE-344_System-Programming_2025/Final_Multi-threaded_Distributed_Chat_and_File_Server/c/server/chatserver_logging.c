/**
 * Multi-threaded Distributed Chat and File Server
 * Logging functionality
 */

#include "chatserver.h"
#include <stdarg.h>

/**
 * Get current timestamp formatted as string
 */
void get_timestamp(char *buffer, size_t size) {
    time_t now = time(NULL);
    struct tm *timeinfo = localtime(&now);
    
    strftime(buffer, size, "%Y-%m-%d %H:%M:%S", timeinfo);
}

/**
 * Log a message to the server log file
 */
void log_message(Server *server, const char *format, ...) {
    if (!server->log_file) {
        return;
    }
    
    char timestamp[32];
    get_timestamp(timestamp, sizeof(timestamp));
    
    // Write timestamp
    fprintf(server->log_file, "%s - ", timestamp);
    
    // Write formatted message
    va_list args;
    va_start(args, format);
    vfprintf(server->log_file, format, args);
    va_end(args);
    
    // Add newline
    fprintf(server->log_file, "\n");
    
    // Flush to ensure log is written immediately
    fflush(server->log_file);
}

/**
 * Log a client action
 */
void log_client_action(Server *server, const char *username, const char *action) {
    log_message(server, "[USER: %s] - %s", username, action);
}

/**
 * Log a file transfer
 */
void log_file_transfer(Server *server, const char *sender, const char *recipient, const char *filename, bool success) {
    if (success) {
        log_message(server, "[SEND FILE] '%s' sent from %s to %s (success)", 
                   filename, sender, recipient);
    } else {
        log_message(server, "[SEND FILE] '%s' sent from %s to %s (failed)", 
                   filename, sender, recipient);
    }
}
