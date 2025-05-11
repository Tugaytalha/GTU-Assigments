# Multithreaded Log File Analyzer

This program is a multithreaded log file analyzer that searches for specified keywords in log files. It uses POSIX threads, mutexes, condition variables, and barriers for synchronization.

## Requirements

- Linux/Unix environment with POSIX threads support
- GCC compiler
- Make (optional)

## Structure

The program consists of the following files:
- `main.c`: Main program logic with manager and worker threads
- `buffer.c/buffer.h`: Implementation of the bounded buffer with synchronization
- `utils.c/utils.h`: Utility functions for argument parsing, signal handling, etc.
- `Makefile`: For easy compilation

## Compilation

To compile the program, run:

```
make
```

Alternatively, you can compile manually:

```
gcc -Wall -Wextra -g -c main.c -o main.o
gcc -Wall -Wextra -g -c buffer.c -o buffer.o
gcc -Wall -Wextra -g -c utils.c -o utils.o
gcc main.o buffer.o utils.o -o LogAnalyzer -lpthread
```

## Usage

```
./LogAnalyzer <buffer_size> <num_workers> <log_file> <search_term>
```

Example:
```
./LogAnalyzer 20 4 logs/sample.log "ERROR"
```

### Arguments:
- `buffer_size`: Size of the shared buffer
- `num_workers`: Number of worker threads to create
- `log_file`: Path to the log file to analyze
- `search_term`: Term to search for in the log file

## Testing

Example test cases:
```
./LogAnalyzer 10 4 logs/sample.log "ERROR"
./LogAnalyzer 5 2 logs/debug.log "FAIL"
./LogAnalyzer 50 8 logs/large.log "404"
```

To check for memory leaks:
```
valgrind ./LogAnalyzer 10 4 logs/sample.log "ERROR"
```

## Implementation Details

1. The manager thread reads the input file and places lines into a shared buffer.
2. Worker threads consume lines from the buffer and search for keywords.
3. Synchronization uses POSIX mutexes, condition variables, and barriers.
4. When the manager finishes reading the file, it adds an EOF marker to the buffer.
5. Worker threads report their findings individually and wait at a barrier.
6. After all workers finish, a summary report is generated. 