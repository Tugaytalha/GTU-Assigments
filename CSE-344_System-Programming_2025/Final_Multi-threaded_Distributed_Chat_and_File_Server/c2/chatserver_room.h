/**
 * chatserver_room.h - Room management definitions
 * CSE 344 - System Programming
 */

#ifndef CHATSERVER_ROOM_H
#define CHATSERVER_ROOM_H

#include "chatserver.h"
#include "chatserver_client.h"

/* Room structure */
struct Room {
    char name[MAX_ROOM_NAME_LENGTH + 1];  /* Room name */
    pthread_mutex_t mutex;                /* Room mutex for thread safety */
    Client* clients[MAX_ROOM_CAPACITY];   /* Clients in the room */
    int client_count;                     /* Number of clients in the room */
    time_t creation_time;                 /* When the room was created */
};

/* Room functions */
Room* create_room(const char* name);
void free_room(Room* room);
int validate_room_name(const char* name);
int add_room(Room* room);
void remove_room(Room* room);
Room* find_room_by_name(const char* name);
int join_room(Client* client, Room* room);
int leave_room(Client* client, Room* room);
int is_client_in_room(Client* client, Room* room);
int get_room_count(void);

#endif /* CHATSERVER_ROOM_H */
