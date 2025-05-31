/**
 * chatserver_log.h - Logging definitions
 * CSE 344 - System Programming
 */

#ifndef CHATSERVER_LOG_H
#define CHATSERVER_LOG_H

#include "chatserver.h"

/* Logging functions */
void log_message(const char* format, ...);
void log_init(void);
void log_close(void);

#endif /* CHATSERVER_LOG_H */
