# Multi-threaded Distributed Chat and File Server

A TCP-based chat and file-sharing system using the client-server model. This implementation provides a central server that accepts connections from multiple clients simultaneously, allowing users to exchange private and group messages, as well as share files with each other.

## Features

### Server Features
- Listens for incoming TCP connections on a given port
- Creates a dedicated thread for each connected client
- Supports at least 15 concurrent clients
- Validates and stores unique usernames (max 16 characters, alphanumeric only)
- Manages rooms (channels) for group messaging
- Allows private messages, room broadcasts, and file transfers
- Implements an upload queue for file sharing (simulates limited resources)
- Logs every user action with timestamps
- Applies mutexes and semaphores to ensure thread-safe queue access
- Responds gracefully to client disconnections or crashes
- Closes all active connections gracefully when receiving SIGINT (Ctrl+C)

### Client Features
- Command-line interface with color-coded messages
- Connects to the server via TCP (IP and port as arguments)
- User authentication with valid username (max 16 chars, alphanumeric)
- Command support:
  - `/join <room_name>` → Join or create a room
  - `/leave` → Leave the current room
  - `/broadcast <message>` → Send message to everyone in the room
  - `/whisper <username> <message>` → Send private message
  - `/sendfile <filename> <username>` → Send file to user
  - `/exit` → Disconnect from the server
  - `/help` → Display help information

## Technical Details
- **Multi-threading**: Each client is handled in a separate thread
- **Synchronization**: Thread-safe operations using locks and semaphores
- **File Transfer**: Queue-based system (max 5 uploads at a time)
- **Logging**: Server logs all activities with timestamped entries

## Requirements
- Python 3.6+
- Required Python package: `colorama` (for colored terminal output)

## Installation

1. Clone the repository
2. Install dependencies:
   ```
   make deps
   ```
3. Make scripts executable:
   ```
   make
   ```

## Usage

### Starting the Server
```
./server/chatserver <port>
```
Example:
```
./server/chatserver 5000
```

### Starting a Client
```
./client/chatclient <server_ip> <port>
```
Example:
```
./client/chatclient 127.0.0.1 5000
```

## Limitations and Constraints
- Usernames: Max 16 chars, alphanumeric only
- File transfer:
  - Accepted file types: .txt, .pdf, .jpg, .png
  - Max file size: 3MB
- Each room has a max capacity of 15 users
- Room names: Alphanumeric and up to 32 characters (no spaces or special characters)
- Upload queue capacity: 5 concurrent uploads

## File Structure
```
.
├── server/
│   ├── chatserver         # Server executable
│   └── chatserver.py      # Server implementation
├── client/
│   ├── chatclient         # Client executable
│   └── chatclient.py      # Client implementation
├── Makefile               # Build and utility commands
└── README.md              # Project documentation
```

## Testing
The system has been designed to handle various test scenarios including:
- Concurrent user load
- Duplicate usernames
- File upload queue limitations
- Unexpected disconnections
- Room switching
- Oversized file rejection
- SIGINT server shutdown
- Filename collisions

## License
This project is provided as an educational tool.
