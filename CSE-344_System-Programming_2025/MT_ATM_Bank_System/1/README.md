# Banking System Simulator

This project implements a client-server based banking simulator using processes for communication. It includes a main bank server, teller processes, and client programs.

## Components

- **Bank Server**: The main server that manages accounts, handles client connections, and maintains the bank database.
- **Client**: Reads client transaction requests from files and communicates with the server.
- **Teller**: Created by the server for each client to process their transactions.

## Features

- Account creation and management
- Deposit and withdraw operations
- Log file maintenance
- Client-server communication via FIFOs
- Signal handling
- Two implementation versions:
  - Basic version using fork for process creation
  - Advanced version using custom process creation, shared memory, and semaphores

## Building the Project

To compile all components, run:

```
make
```

This will build:
- `bank_server` - Basic server implementation
- `bank_client` - Client implementation
- `advanced_server` - Advanced server implementation with shared memory

## Running the System

### Starting the Server

Basic server:
```
cd ~
cp -r /mnt/c/Users/Tugay\ Talha\ İçen/Desktop/GitHub/GTU-Assigments/CSE-344_System-Programming_2025/MT_ATM_Bank_System .
cd MT_ATM_Bank_System
./bank_server SAdaBank ServerFIFO_1
```

Advanced server:
```