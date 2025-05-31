/**
 * chatserver_commands.h - Command processing definitions
 * CSE 344 - System Programming
 */

#ifndef CHATSERVER_COMMANDS_H
#define CHATSERVER_COMMANDS_H

#include "chatserver.h"
#include "chatserver_client.h"

/* Command types */
typedef enum {
    CMD_UNKNOWN = 0,
    CMD_JOIN,
    CMD_LEAVE,
    CMD_BROADCAST,
    CMD_WHISPER,
    CMD_SENDFILE,
    CMD_EXIT
} CommandType;

/* Command structure */
typedef struct {
    CommandType type;
    char *args;
} Command;

/* Command functions */
void process_command(Client* client, const char* message);
Command parse_command(const char* message);
void handle_join_command(Client* client, const char* args);
void handle_leave_command(Client* client);
void handle_broadcast_command(Client* client, const char* args);
void handle_whisper_command(Client* client, const char* args);
void handle_sendfile_command(Client* client, const char* args);
void handle_exit_command(Client* client);

#endif /* CHATSERVER_COMMANDS_H */
