/**
 * chatserver_files.h - File transfer definitions
 * CSE 344 - System Programming
 */

#ifndef CHATSERVER_FILES_H
#define CHATSERVER_FILES_H

#include "chatserver.h"
#include "chatserver_client.h"

/* File transfer status enum */
typedef enum {
    TRANSFER_PENDING = 0,
    TRANSFER_IN_PROGRESS,
    TRANSFER_COMPLETED,
    TRANSFER_FAILED
} TransferStatus;

/* File transfer structure */
struct FileTransfer {
    Client* sender;                  /* Sending client */
    Client* recipient;               /* Receiving client */
    char filename[MAX_MESSAGE_LENGTH]; /* Server-local path to the file */
    char original_filename[MAX_MESSAGE_LENGTH]; /* Original file name */
    size_t file_size;                /* File size in bytes */
    time_t queue_time;               /* Time added to queue */
    time_t start_time;               /* Time transfer started */
    time_t end_time;                 /* Time transfer completed */
    TransferStatus status;           /* Transfer status */
    pthread_t thread_id;             /* Thread ID for transfer thread */
};

/* File header structure (sent to recipient before file data) */
typedef struct {
    size_t file_size;                /* File size in bytes */
    char filename[MAX_MESSAGE_LENGTH]; /* Original filename */
    char sender[MAX_USERNAME_LENGTH + 1]; /* Sender username */
} FileHeader;

/* File transfer functions */
int add_to_upload_queue(Client* sender, Client* recipient, const char* filepath, const char* original_filename, size_t file_size);
void* process_upload_queue(void* arg);
void notify_transfer_complete(FileTransfer* transfer, int success);
void remove_from_upload_queue(FileTransfer* transfer);
int handle_file_receive(Client* client, const char* buffer, size_t size);

#endif /* CHATSERVER_FILES_H */
