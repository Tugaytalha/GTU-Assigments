# Homework #2: Inter-Process Communication with Daemon Process

## Implementation Overview

This project implements a communication protocol between two processes using IPC (Inter-Process Communication) through named pipes (FIFOs). It also introduces a daemon process to handle background operations such as logging and monitoring.

### Key Components

1. **Parent Process**: 
   - Takes two integer arguments from the command line
   - Creates two FIFOs (named pipes)
   - Sends the two integer values to the first FIFO
   - Forks two child processes, each handling a different FIFO
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
   - Monitors inactive child processes and terminates them if necessary

## Technical Implementation Details

### IPC Implementation

Named pipes (FIFOs) are used for inter-process communication:
- FIFO1: Used to send the two integer values from the parent to Child Process 1
- FIFO2: Used to send the larger number from Child Process 1 to Child Process 2

Non-blocking I/O operations with timeouts are implemented to avoid deadlocks:
- `set_nonblocking()` function sets file descriptors to non-blocking mode
- `read_with_timeout()` function uses `select()` to implement timeout for read operations

### Signal Handling

Various signals are handled for proper process management:
- SIGCHLD: Handled by the parent process to detect child process termination
- SIGUSR1, SIGHUP, SIGTERM: Handled by the daemon process for various control operations

### Zombie Process Prevention

Zombie processes are prevented by:
- Using the SIGCHLD signal handler with waitpid() to reap terminated child processes
- Implementing proper exit status reporting
- Using non-blocking calls to waitpid() with WNOHANG flag

### Timeout Mechanism

A timeout mechanism monitors inactive child processes:
- The daemon tracks last activity time for each child process
- If a process is inactive for longer than TIMEOUT_SECONDS (15 seconds), it's terminated
- The daemon periodically checks process status using kill(pid, 0)

### Error Handling

Comprehensive error handling is implemented throughout the code:
- All system calls are checked for errors
- Appropriate error messages are displayed using perror()
- Resources are properly cleaned up in error conditions
- Error logging in the daemon process

## Testing and Validation

The test.sh script tests various aspects of the program:

1. **Basic Functionality**: Tests with two positive integers
2. **Negative Numbers**: Tests with negative integers
3. **Equal Numbers**: Tests with equal integers
4. **Invalid Input**: Tests with insufficient arguments
5. **Zombie Process Check**: Verifies no zombie processes are created
6. **Log File Verification**: Confirms daemon logging is working

### Example Test Results

```
Test 1: Basic functionality with two positive integers
Running: ./ipc_daemon 42 17
Child 1 (PID 12345) started
Child 2 (PID 12346) started
Child 1 (PID 12345) completed task - determined larger number: 42
Child 2 (PID 12346) Result: The larger number is 42
Child process 12345 exited with status 0
Child process 12346 exited with status 0
Parent: All children have exited. Cleaning up.
Expected result: The larger number is 42
```

## Bonus Implementation

### Zombie Protection Method (15 points)

Zombie processes are prevented through:
- Properly setting up a SIGCHLD handler
- Using waitpid() with WNOHANG to collect exit status information
- Using sigaction() with SA_NOCLDSTOP and SA_RESTART flags
- Implementing a monitoring system in the daemon process

### Exit Status Printing (15 points)

Exit statuses are printed for all processes:
- WIFEXITED and WEXITSTATUS macros are used to determine if a process exited normally
- WIFSIGNALED and WTERMSIG macros are used to determine if a process was terminated by a signal
- All exit statuses are logged to both console and log file

## Conclusion

The implemented solution successfully demonstrates inter-process communication using named pipes with a daemon process for background operations. It handles signal processing correctly and includes protection against zombie processes. The implementation is robust with comprehensive error handling and follows best practices for system programming. 