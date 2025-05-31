/**
 * chatserver_room.c - Room management implementation
 * CSE 344 - System Programming
 */

#include "chatserver_room.h"
#include "chatserver_log.h"

/**
 * Create a new room
 */
Room* create_room(const char* name) {
    Room* room;
    
    if (!name || !validate_room_name(name)) {
        return NULL;
    }
    
    room = (Room*)malloc(sizeof(Room));
    if (!room) {
        perror("Failed to allocate room");
        return NULL;
    }
    
    /* Initialize room fields */
    memset(room, 0, sizeof(Room));
    strncpy(room->name, name, MAX_ROOM_NAME_LENGTH);
    room->client_count = 0;
    room->creation_time = time(NULL);
    pthread_mutex_init(&room->mutex, NULL);
    
    return room;
}

/**
 * Free room resources
 */
void free_room(Room* room) {
    int i;
    
    if (!room) return;
    
    pthread_mutex_lock(&room->mutex);
    
    /* Set current_room to NULL for all clients in the room */
    for (i = 0; i < MAX_ROOM_CAPACITY; i++) {
        if (room->clients[i]) {
            pthread_mutex_lock(&room->clients[i]->mutex);
            if (room->clients[i]->current_room == room) {
                room->clients[i]->current_room = NULL;
            }
            pthread_mutex_unlock(&room->clients[i]->mutex);
            
            room->clients[i] = NULL;
        }
    }
    
    pthread_mutex_unlock(&room->mutex);
    pthread_mutex_destroy(&room->mutex);
    
    free(room);
}

/**
 * Validate room name (alphanumeric, max 32 chars)
 */
int validate_room_name(const char* name) {
    int i, len;
    
    if (!name) return 0;
    
    len = strlen(name);
    if (len == 0 || len > MAX_ROOM_NAME_LENGTH) {
        return 0;
    }
    
    for (i = 0; i < len; i++) {
        if (!isalnum(name[i])) {
            return 0;
        }
    }
    
    return 1;
}

/**
 * Add room to rooms array
 */
int add_room(Room* room) {
    int i;
    
    if (!room) return -1;
    
    pthread_mutex_lock(&rooms_mutex);
    
    /* Find empty slot */
    for (i = 0; i < MAX_ROOMS; i++) {
        if (rooms[i] == NULL) {
            rooms[i] = room;
            pthread_mutex_unlock(&rooms_mutex);
            return 0;
        }
    }
    
    /* No empty slots */
    pthread_mutex_unlock(&rooms_mutex);
    return -1;
}

/**
 * Remove room from rooms array
 */
void remove_room(Room* room) {
    int i;
    
    if (!room) return;
    
    pthread_mutex_lock(&rooms_mutex);
    
    /* Find room and remove */
    for (i = 0; i < MAX_ROOMS; i++) {
        if (rooms[i] == room) {
            rooms[i] = NULL;
            break;
        }
    }
    
    pthread_mutex_unlock(&rooms_mutex);
}

/**
 * Find room by name
 */
Room* find_room_by_name(const char* name) {
    int i;
    Room* found = NULL;
    
    if (!name || strlen(name) == 0) return NULL;
    
    pthread_mutex_lock(&rooms_mutex);
    
    for (i = 0; i < MAX_ROOMS; i++) {
        if (rooms[i] && strcmp(rooms[i]->name, name) == 0) {
            found = rooms[i];
            break;
        }
    }
    
    pthread_mutex_unlock(&rooms_mutex);
    return found;
}

/**
 * Add client to room
 */
int join_room(Client* client, Room* room) {
    int i;
    
    if (!client || !room) return -1;
    
    /* First check if client is already in another room */
    pthread_mutex_lock(&client->mutex);
    if (client->current_room != NULL && client->current_room != room) {
        /* Leave current room first */
        leave_room(client, client->current_room);
    }
    pthread_mutex_unlock(&client->mutex);
    
    pthread_mutex_lock(&room->mutex);
    
    /* Check if room is full */
    if (room->client_count >= MAX_ROOM_CAPACITY) {
        pthread_mutex_unlock(&room->mutex);
        return -1;
    }
    
    /* Check if client is already in this room */
    for (i = 0; i < MAX_ROOM_CAPACITY; i++) {
        if (room->clients[i] == client) {
            pthread_mutex_unlock(&room->mutex);
            return 0; /* Already in room */
        }
    }
    
    /* Find empty slot */
    for (i = 0; i < MAX_ROOM_CAPACITY; i++) {
        if (room->clients[i] == NULL) {
            room->clients[i] = client;
            room->client_count++;
            
            pthread_mutex_lock(&client->mutex);
            client->current_room = room;
            pthread_mutex_unlock(&client->mutex);
            
            pthread_mutex_unlock(&room->mutex);
            
            /* Log room join */
            log_message("[JOIN] user '%s' joined room '%s'", client->username, room->name);
            
            return 0;
        }
    }
    
    /* No empty slots (should never reach here due to client_count check) */
    pthread_mutex_unlock(&room->mutex);
    return -1;
}

/**
 * Remove client from room
 */
int leave_room(Client* client, Room* room) {
    int i, found = 0;
    
    if (!client || !room) return -1;
    
    pthread_mutex_lock(&room->mutex);
    
    /* Find client in room */
    for (i = 0; i < MAX_ROOM_CAPACITY; i++) {
        if (room->clients[i] == client) {
            room->clients[i] = NULL;
            room->client_count--;
            found = 1;
            break;
        }
    }
    
    pthread_mutex_unlock(&room->mutex);
    
    if (found) {
        pthread_mutex_lock(&client->mutex);
        if (client->current_room == room) {
            client->current_room = NULL;
        }
        pthread_mutex_unlock(&client->mutex);
        
        /* Log room leave */
        log_message("[ROOM] user '%s' left room '%s'", client->username, room->name);
        
        /* If room is empty, remove it */
        pthread_mutex_lock(&room->mutex);
        if (room->client_count == 0) {
            pthread_mutex_unlock(&room->mutex);
            
            log_message("[ROOM] Room '%s' is empty, removing", room->name);
            remove_room(room);
            free_room(room);
        } else {
            pthread_mutex_unlock(&room->mutex);
        }
        
        return 0;
    }
    
    return -1;
}

/**
 * Check if client is in room
 */
int is_client_in_room(Client* client, Room* room) {
    int i, found = 0;
    
    if (!client || !room) return 0;
    
    pthread_mutex_lock(&room->mutex);
    
    for (i = 0; i < MAX_ROOM_CAPACITY; i++) {
        if (room->clients[i] == client) {
            found = 1;
            break;
        }
    }
    
    pthread_mutex_unlock(&room->mutex);
    return found;
}

/**
 * Get total number of rooms
 */
int get_room_count(void) {
    int i, count = 0;
    
    pthread_mutex_lock(&rooms_mutex);
    
    for (i = 0; i < MAX_ROOMS; i++) {
        if (rooms[i] != NULL) {
            count++;
        }
    }
    
    pthread_mutex_unlock(&rooms_mutex);
    
    return count;
}
