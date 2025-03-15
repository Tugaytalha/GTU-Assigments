# Secure File and Directory Management System

A command-line utility for managing files and directories securely on Linux systems.

## Overview

This program provides various file and directory operations with proper error handling and logging. The program uses Linux system calls to perform operations such as creating, listing, reading, updating, and deleting files and directories.

## Features

- Create files and directories
- List files and directories
- List files by extension
- Read file contents
- Append content to files (with file locking)
- Delete files and directories
- Operation logging
- Help and usage information

## Requirements

- Linux-based system (Debian 11 recommended)
- GCC compiler
- make

## Compilation

To compile the program, simply run:

```bash
make
```

This will generate the `fileManager` executable.

To clean up the compiled files:

```bash
make clean
```

## Usage

Run the program without arguments to display help:

```bash
./fileManager
```

### Available Commands

- `createDir "folderName"` - Create a new directory
- `createFile "fileName"` - Create a new file
- `listDir "folderName"` - List all files in a directory
- `listFilesByExtension "folderName" ".txt"` - List files with specific extension
- `readFile "fileName"` - Read a file's content
- `appendToFile "fileName" "new content"` - Append content to a file
- `deleteFile "fileName"` - Delete a file
- `deleteDir "folderName"` - Delete an empty directory
- `showLogs` - Display operation logs

## Example Usage

1. Create a directory:
   ```bash
   ./fileManager createDir "testDir"
   ```

2. Create a file:
   ```bash
   ./fileManager createFile "testDir/example.txt"
   ```

3. List directory contents:
   ```bash
   ./fileManager listDir "testDir"
   ```

4. Append content to a file:
   ```bash
   ./fileManager appendToFile "testDir/example.txt" "Hello, World!"
   ```

5. Show logs:
   ```bash
   ./fileManager showLogs
   ```

## Implementation Details

- The program uses `fork()` for list and delete operations
- File locking is implemented for safe concurrent access
- All operations are logged in `log.txt`
- Error handling is provided for all operations 