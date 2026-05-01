# GTU Assignments 🎓

![GTU](https://img.shields.io/badge/Gebze%20Technical%20University-blue?style=flat&logo=university)
![License](https://img.shields.io/badge/license-MIT-lightgrey.svg)
![C](https://img.shields.io/badge/C-99-orange.svg)
![Python](https://img.shields.io/badge/Python-3.6+-green.svg)
![System Programming](https://img.shields.io/badge/System%20Programming-344-blueviolet.svg)

A comprehensive collection of **Gebze Technical University (GTU)** Computer Engineering assignments completed by Tugay Talha İçen. This repository contains various programming assignments focusing on system programming, multi-threading, distributed systems, and inter-process communication.

## 🎓 About GTU

**Gebze Technical University (GTU)** is one of Turkey's leading technical universities, located in Gebze, Kocaeli. The Computer Engineering department offers rigorous training in:
- System Programming
- Operating Systems
- Distributed Systems
- Computer Networks
- Software Engineering

## 📚 Assignment Overview

### CSE-344 System Programming (Spring 2025)

This course covers advanced system programming concepts including file systems, inter-process communication, synchronization mechanisms, multi-threading, and distributed systems.

---

## 📝 Assignments

### Homework 1: File Management System (C)

**Objective**: Implement a file management system with basic CRUD operations.

**Files**:
- `fileManager.c` - Main file management implementation
- `Makefile` - Build configuration
- `test_script.sh` - Automated testing script

**Features**:
- Create, read, update, delete (CRUD) operations on files
- Directory navigation and management
- File permissions handling
- Error handling and reporting

**Build & Run**:
```bash
cd CSE-344_System-Programming_2025/Hw1_File_Management_System_2025
make
./fileManager
```

---

### Homework 2: Inter-Process Communication (C)

**Objective**: Demonstrate IPC mechanisms including pipes, shared memory, and message queues.

**Files**:
- `main.c` - IPC implementation
- `Makefile` - Build configuration

**Features**:
- Pipe communication between parent and child processes
- Shared memory segments for data exchange
- Message queue implementation
- Process synchronization

**Build & Run**:
```bash
cd CSE-344_System-Programming_2025/Hw2_InterProcess-Communication
make
./main
```

---

### Homework 3: Engineer Satellite - Semaphores and Synchronization (C)

**Objective**: Simulate satellite communication systems using semaphores and synchronization primitives.

**Files**:
- `main.c` - Satellite simulation with semaphores
- `Makefile` - Build configuration
- `output.png` - Sample output visualization

**Features**:
- Semaphore-based synchronization
- Multi-process satellite communication simulation
- Resource allocation and deadlock prevention
- Real-time data transmission simulation

**Concepts Demonstrated**:
- P and V operations (wait and signal)
- Critical section management
- Race condition prevention
- Resource sharing in distributed systems

**Build & Run**:
```bash
cd CSE-344_System-Programming_2025/Hw3_Engineer_Satellite_Semaphores_and_Sync
make
./hw3
```

---

### 🏆 Final Project: Multi-threaded Distributed Chat and File Server

**Objective**: Design and implement a TCP-based, multi-threaded chat and file-sharing system using client-server architecture.

**Complexity**: ⭐⭐⭐⭐⭐ (Highest difficulty in the course)

#### Architecture

```
┌─────────────────────────────────────────────────────┐
│                    Server                        │
│  ┌──────────────────────────────────────────┐   │
│  │  Main Thread (Connection Acceptor)        │   │
│  │  ┌────────┐ ┌────────┐ ┌────────┐    │   │
│  │  │Thread 1│ │Thread 2│ │Thread N│    │   │
│  │  │Client 1│ │Client 2│ │Client N│    │   │
│  │  └────────┘ └────────┘ └────────┘    │   │
│  └──────────────────────────────────────────┘   │
│  ┌──────────────────────────────────────────┐   │
│  │  Upload Queue (Semaphore-limited)         │   │
│  └──────────────────────────────────────────┘   │
│  ┌──────────────────────────────────────────┐   │
│  │  Logging System (Thread-safe)              │   │
│  └──────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────┘
```

#### Features

##### Server Features
- ✅ TCP connection listener on specified port
- ✅ Dedicated thread per connected client (15+ concurrent clients)
- ✅ Unique username validation (max 16 chars, alphanumeric only)
- ✅ Room (channel) management for group messaging
- ✅ Private messaging, room broadcasts, and file transfers
- ✅ Upload queue simulation (limited resources with semaphores)
- ✅ Comprehensive logging with timestamps
- ✅ Thread-safe queue access with mutexes and semaphores
- ✅ Graceful disconnection handling
- ✅ SIGINT (Ctrl+C) graceful shutdown

##### Client Features
- ✅ Command-line interface with color-coded messages (colorama)
- ✅ TCP connection to server (IP + port)
- ✅ User authentication with validation
- ✅ Command support:
  - `/join <room_name>` → Join or create a room
  - `/leave` → Leave current room
  - `/broadcast <message>` → Send to everyone in room
  - `/whisper <username> <message>` → Private message
  - `/sendfile <filename> <username>` → Send file to user
  - `/exit` → Disconnect from server
  - `/help` → Display help

##### Technical Implementation
- **Multi-threading**: pthreads for each client connection
- **Synchronization**: Mutexes and semaphores for thread safety
- **File Transfer**: Queue-based system (max 5 concurrent uploads)
- **Logging**: Timestamped server activity logs
- **Two Implementations**: Both C and Python versions available

#### Constraints & Limitations
| Parameter | Limitation |
|-----------|-------------|
| Username | Max 16 chars, alphanumeric only |
| File Types | .txt, .pdf, .jpg, .png |
| File Size | Max 3MB |
| Room Capacity | Max 15 users per room |
| Room Name | Alphanumeric, max 32 chars, no spaces |
| Upload Queue | Max 5 concurrent uploads |

#### File Structure
```
Final_Multi-threaded_Distributed_Chat_and_File_Server/
├── c/                          # C implementation
│   ├── server/
│   │   ├── chatserver.c       # Main server logic
│   │   ├── chatserver_files.c  # File transfer handling
│   │   ├── chatserver_client.c # Client connection handler
│   │   ├── chatserver_commands.c # Command processing
│   │   ├── chatserver_logging.c # Logging system
│   │   ├── chatserver_rooms.c # Room management
│   │   └── main.c             # Entry point
│   ├── client/
│   │   └── chatclient.c       # Client implementation
│   ├── test_chatserver.py      # Python test script
│   ├── Makefile                # Build configuration
│   └── Final Project.pdf      # Project documentation
└── python/                     # Python implementation
    ├── server/
    │   ├── chatserver.py       # Main server
    │   └── server_log.txt     # Server logs
    ├── client/
    │   ├── chatclient.py       # Client implementation
    │   ├── a.txt              # Sample file
    │   └── downloads/         # Downloaded files
    ├── example_log.txt         # Sample logs
    ├── Makefile                # Build configuration
    └── README.md              # Python version docs
```

#### Requirements
- **C Version**: GCC compiler, pthreads library
- **Python Version**: Python 3.6+, `colorama` package

#### Installation & Usage

##### C Version
```bash
cd CSE-344_System-Programming_2025/Final_Multi-threaded_Distributed_Chat_and_File_Server/c

# Build
make

# Start server
./server/chatserver <port>
# Example: ./server/chatserver 5000

# Start client (new terminal)
./client/chatclient <server_ip> <port>
# Example: ./client/chatclient 127.0.0.1 5000
```

##### Python Version
```bash
cd CSE-344_System-Programming_2025/Final_Multi-threaded_Distributed_Chat_and_File_Server/python

# Install dependencies
pip install colorama

# Make executable
make

# Start server
./server/chatserver <port>
# Example: ./server/chatserver 5000

# Start client (new terminal)
./client/chatclient <server_ip> <port>
# Example: ./client/chatclient 127.0.0.1 5000
```

#### Testing Scenarios
- ✅ Concurrent user load (15+ clients)
- ✅ Duplicate username rejection
- ✅ File upload queue limitations
- ✅ Unexpected client disconnections
- ✅ Room switching and management
- ✅ Oversized file rejection
- ✅ SIGINT server shutdown
- ✅ Filename collision handling

---

## 🛠️ Technologies Used

| Technology | Purpose |
|-----------|---------|
| C (C99) | System programming, pthreads, IPC |
| Python 3.6+ | High-level implementation, rapid prototyping |
| POSIX Threads | Multi-threading support |
| Semaphores | Process synchronization |
| TCP Sockets | Network communication |
| GCC | C compilation |
| Make | Build automation |
| colorama | Colored terminal output (Python) |

## 📊 Skills Demonstrated

### System Programming
- ✅ File I/O operations
- ✅ Process management
- ✅ Memory management
- ✅ Signal handling (SIGINT)

### Concurrent Programming
- ✅ Multi-threading (pthreads)
- ✅ Semaphore implementation
- ✅ Mutex usage for critical sections
- ✅ Thread-safe queue management

### Network Programming
- ✅ TCP socket programming
- ✅ Client-server architecture
- ✅ Concurrent connection handling
- ✅ Data serialization

### Distributed Systems
- ✅ Chat room management
- ✅ File transfer protocols
- ✅ Distributed logging
- ✅ Resource allocation

## 🎯 Learning Outcomes

Through these assignments, I gained hands-on experience in:
1. **Low-level system programming** with C
2. **Concurrent programming** concepts and pitfalls
3. **Synchronization primitives** (mutexes, semaphores)
4. **Network programming** with TCP/IP
5. **Distributed system design** principles
6. **Debugging multi-threaded applications**
7. **Building scalable server architectures**

## 🤝 Contributing

These are my personal assignments completed during my studies. While I welcome feedback and suggestions, please note:
- ⚠️ **Do not copy** these assignments if you're a current GTU student (academic integrity matters!)
- ✅ Feel free to use as **learning references**
- ✅ Suggestions for code improvements are welcome
- ✅ Bug reports and optimization ideas appreciated

## 📄 License

This project is licensed under the **MIT License**. See the [LICENSE](LICENSE) file for details.

## 📧 Contact

**Tugay Talha İçen**
- GitHub: [@Tugaytalha](https://github.com/Tugaytalha)
- Twitter: [@TugayTalhaIcen](https://twitter.com/TugayTalhaIcen)
- LinkedIn: [Tugay Talha İçen](https://linkedin.com/in/tugaytalhaicen)

**Repository Link**: [https://github.com/Tugaytalha/GTU-Assignments](https://github.com/Tugaytalha/GTU-Assignments)

## 🙏 Acknowledgments

- **Gebze Technical University** for the excellent Computer Engineering curriculum
- **CSE-344 Course Staff** for well-designed assignments
- **Fellow students** for collaborative learning
- **Open source community** for libraries and tools

---

🎓 **Proud GTU Computer Engineering Alumni**

⭐ Don't forget to star this repo if you found it helpful!

🐛 Found an issue? Use the [Issues](https://github.com/Tugaytalha/GTU-Assignments/issues) tab.

📖 **Education is the passport to the future, for tomorrow belongs to those who prepare for it today.**
