/**
 * Multi-threaded Distributed Chat and File Server
 * Room management functions
 */

#include "chatserver.h"

/**
 * Create or join a room
 * Returns: 0 on success, -1 if room is full, -2 on other errors
 */
int create_or_join_room(Server *server, Client *client, const char *room_name) {
    int room_index = -1;
    
    // Lock rooms mutex for the operation
    pthread_mutex_lock(&server->rooms_mutex);
    
    // Check if the room already exists
    for (int i = 0; i < MAX_ROOMS; i++) {
        if (server->rooms[i].active && strcmp(server->rooms[i].name, room_name) == 0) {
            room_index = i;
            break;
        }
    }
    
    // If room doesn't exist, create it
    if (room_index == -1) {
        for (int i = 0; i < MAX_ROOMS; i++) {
            if (!server->rooms[i].active) {
                room_index = i;
                strncpy(server->rooms[i].name, room_name, MAX_ROOM_NAME_LEN);
                server->rooms[i].member_count = 0;
                server->rooms[i].active = true;
                break;
            }
        }
    }
    
    pthread_mutex_unlock(&server->rooms_mutex);
    
    // If no room was found or created, return error
    if (room_index == -1) {
        return -2;  // No available room slots
    }
    
    // Lock the specific room for adding the client
    pthread_mutex_lock(&server->rooms[room_index].room_mutex);
    
    // Check if room is full
    if (server->rooms[room_index].member_count >= MAX_CLIENTS) {
        pthread_mutex_unlock(&server->rooms[room_index].room_mutex);
        return -1;  // Room is full
    }
    
    // Add client to room
    int member_index = server->rooms[room_index].member_count;
    server->rooms[room_index].member_sockets[member_index] = client->socket;
    strncpy(server->rooms[room_index].member_usernames[member_index], client->username, MAX_USERNAME_LEN);
    server->rooms[room_index].member_count++;
    
    // Update client's current room
    strncpy(client->current_room, room_name, MAX_ROOM_NAME_LEN);
    
    pthread_mutex_unlock(&server->rooms[room_index].room_mutex);
    
    return 0;  // Success
}

/**
 * Make a client leave their current room
 */
void leave_room(Server *server, Client *client) {
    if (strlen(client->current_room) == 0) {
        return;  // Client is not in a room
    }
    
    int room_index = find_room_index(server, client->current_room);
    if (room_index == -1) {
        // Room not found, just clear client's room field
        client->current_room[0] = '\0';
        return;
    }
    
    // Lock the room for modification
    pthread_mutex_lock(&server->rooms[room_index].room_mutex);
    
    // Find client in room members
    int member_index = -1;
    for (int i = 0; i < server->rooms[room_index].member_count; i++) {
        if (server->rooms[room_index].member_sockets[i] == client->socket) {
            member_index = i;
            break;
        }
    }
    
    if (member_index != -1) {
        // Remove client from room by shifting members
        for (int i = member_index; i < server->rooms[room_index].member_count - 1; i++) {
            server->rooms[room_index].member_sockets[i] = server->rooms[room_index].member_sockets[i + 1];
            strncpy(server->rooms[room_index].member_usernames[i], 
                   server->rooms[room_index].member_usernames[i + 1], 
                   MAX_USERNAME_LEN);
        }
        
        server->rooms[room_index].member_count--;
        
        // If room is empty, mark it as inactive
        if (server->rooms[room_index].member_count == 0) {
            server->rooms[room_index].active = false;
        }
    }
    
    pthread_mutex_unlock(&server->rooms[room_index].room_mutex);
    
    // Clear client's room field
    client->current_room[0] = '\0';
    
    // Log the action
    log_message(server, "[ROOM] user '%s' left room '%s'", client->username, client->current_room);
}

/**
 * Find a room's index by name
 * Returns: room index or -1 if not found
 */
int find_room_index(Server *server, const char *room_name) {
    int index = -1;
    
    pthread_mutex_lock(&server->rooms_mutex);
    for (int i = 0; i < MAX_ROOMS; i++) {
        if (server->rooms[i].active && strcmp(server->rooms[i].name, room_name) == 0) {
            index = i;
            break;
        }
    }
    pthread_mutex_unlock(&server->rooms_mutex);
    
    return index;
}
