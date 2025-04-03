# Inter-Process Communication with Daemon Process

This program demonstrates a communication protocol between two processes using IPC (Inter-Process Communication) through named pipes (FIFOs) with a daemon process for background operations.

## Requirements

- Linux/Unix-based system (not compatible with Windows)
- GCC compiler
- Make utility

## Description

The program consists of:

1. **Parent Process**:
   - Takes two integer arguments from the command line
   - Creates two FIFOs (named pipes)
   - Sends the two integer values to the first FIFO
   - Spawns two child processes, each handling a different FIFO
   - Converts itself into a daemon to handle logging operations
   - Handles SIGCHLD signals for child process termination

2. **Child Process 1**:
   - Reads two integers from the first FIFO
   - Determines which number is larger
   - Writes the larger number to the second FIFO

3. **Child Process 2**:
   - Reads the larger number from the second FIFO
   - Prints it to the screen

4. **Daemon Process**:
   - Handles logging operations
   - Monitors process execution
   - Handles various signals (SIGUSR1, SIGHUP, SIGTERM)

## Building

```bash
make
```

## Running

```bash
./ipc_daemon <int1> <int2>
```

Example:
```bash
./ipc_daemon 42 17
```

## Cleaning up

```bash
make clean
```

## Implementation Details

- Uses `fork()` system call to create child processes
- Uses named pipes (FIFOs) for inter-process communication
- Implements signal handlers for process management
- Uses proper error handling throughout
- Implements zombie process protection
- Logs detailed information about process execution

## Notes

- The child processes sleep for 10 seconds before executing their tasks
- The parent process prints a "proceeding" message every 2 seconds
- All processes print their exit status when terminating
- The program creates FIFOs in the current directory and cleans them up on exit
- A log file (`daemon.log`) is created for daemon operations 