# Multi-threaded Distributed Chat and File Server

This project implements a TCP-based chat and file-sharing system using the client-server model. The system allows multiple clients to connect to a central server, exchange messages in rooms, and share files with each other.

## Features

### Server Features
- Multi-threaded with support for up to 15 concurrent clients
- Room-based messaging system with capacity control
- Private messaging via whisper command
- Secure file transfer with upload queue (maximum 5 concurrent uploads)
- Detailed activity logging with timestamps
- Graceful handling of client disconnections and server shutdown
- Thread-safe operations with mutexes and semaphores

### Client Features
- Command-line interface with color-coded responses
- Support for all required commands:
  - `/join <room_name>` - Join or create a room
  - `/leave` - Leave the current room
  - `/broadcast <message>` - Send a message to everyone in the room
  - `/whisper <username> <message>` - Send a private message to a specific user
  - `/sendfile <filename> <username>` - Send a file to a specific user
  - `/exit` - Disconnect from the server
- File download handling with automatic collision resolution

## Building the Project

To build both the server and client, run:

```bash
make
```

This will create two executables:
- `chatserver` - The server program
- `chatclient` - The client program

To clean the build files:

```bash
make clean
```

## Running the System

### Starting the Server

```bash
./chatserver <port>
```

Example:
```bash
./chatserver 8080
```

### Connecting a Client

```bash
./chatclient <server_ip> <port>
```

Example:
```bash
./chatclient 127.0.0.1 8080
```

## File Transfer

The system supports file transfers with the following constraints:
- Maximum file size: 3MB
- Allowed file extensions: .txt, .pdf, .jpg, .png

The server maintains a queue of up to 5 concurrent file transfers to simulate limited system resources. Additional transfer requests are queued and processed in order as slots become available.

## Implementation Details

### Server Architecture
- Multi-threaded with dedicated threads for each client
- Thread synchronization using mutexes and semaphores
- Room-based messaging with thread-safe operations
- Queue-based file transfer system

### Client Architecture
- Single-threaded input processing
- Separate thread for receiving messages and file data
- Color-coded terminal output for better user experience

## Test Cases

The system has been designed to handle all the required test scenarios:

1. Concurrent User Load - Successfully handles 30+ clients connecting simultaneously
2. Duplicate Usernames - Rejects connections with already taken usernames
3. File Upload Queue - Limits concurrent uploads to 5, queues additional requests
4. Unexpected Disconnection - Gracefully handles client crashes or disconnections
5. Room Switching - Properly updates room states when clients switch rooms
6. Oversized File Rejection - Checks file size before transfer and rejects if over 3MB
7. SIGINT Server Shutdown - Notifies all clients and cleanly shuts down on Ctrl+C
8. Rejoining Rooms - Correctly handles clients leaving and rejoining rooms
9. Same Filename Collision - Handles duplicate filenames by renaming
10. File Queue Wait Duration - Tracks and reports queue wait times

## Usage Example

### Server:
```
$ ./chatserver 8080
Chat server started on port 8080
2025-05-31 22:25:32 - [SERVER] Started on port 8080
2025-05-31 22:25:32 - [SERVER] Accepting connections on port 8080
```

### Client:
```
$ ./chatclient 127.0.0.1 8080
Connected to server at 127.0.0.1:8080
Please enter your username (max 16 chars, alphanumeric only):
user123
Login successful! Welcome to the chat server.
```

## Notes

- The server logs all activities to `server_log.txt` with timestamps
- Downloaded files are stored in the `downloads` directory on the client side
- Uploaded files are stored in the `uploads` directory on the server side
