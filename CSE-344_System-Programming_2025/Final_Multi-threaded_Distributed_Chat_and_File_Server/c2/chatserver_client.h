/**
 * chatserver_client.h - Client handling definitions
 * CSE 344 - System Programming
 */

#ifndef CHATSERVER_CLIENT_H
#define CHATSERVER_CLIENT_H

#include "chatserver.h"

/* Client structure */
struct Client {
    int socket;                         /* Client socket descriptor */
    struct sockaddr_in address;         /* Client address */
    pthread_mutex_t mutex;              /* Client mutex for thread safety */
    char username[MAX_USERNAME_LENGTH + 1]; /* Client username */
    char ip_address[INET_ADDRSTRLEN];   /* Client IP address as string */
    ClientStatus status;                /* Client connection status */
    Room* current_room;                 /* Current room the client is in, NULL if not in a room */
    time_t connect_time;                /* Time client connected */
    time_t last_activity;               /* Time of last client activity */
};

/* Client functions */
Client* create_client(int socket, struct sockaddr_in addr);
void free_client(Client* client);
void disconnect_client(Client* client);
void* handle_client(void* arg);
int validate_username(const char* username);
int add_client(Client* client);
void remove_client(Client* client);
Client* find_client_by_username(const char* username);
void send_to_client(Client* client, const char* message);
void broadcast_to_room(Room* room, const char* message, Client* exclude);
void send_error_to_client(Client* client, const char* message);
void send_success_to_client(Client* client, const char* message);

#endif /* CHATSERVER_CLIENT_H */
