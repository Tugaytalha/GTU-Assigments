/**
 * Secure File and Directory Management System
 * A program to manage files and directories with secure operations and logging
 */

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/file.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <time.h>

#define LOG_FILE "log.txt"
#define MAX_BUFFER 1024
#define MAX_PATH 256

// Helper function to write to stdout
void write_stdout(const char *message) {
    write(STDOUT_FILENO, message, strlen(message));
}

// Helper function to write to stderr
void write_stderr(const char *message) {
    write(STDERR_FILENO, message, strlen(message));
}

// Helper function to write error messages
void write_error(const char *prefix) {
    char error_msg[MAX_BUFFER];
    snprintf(error_msg, sizeof(error_msg), "%s: %s\n", prefix, strerror(errno));
    write_stderr(error_msg);
}

// Function to get current timestamp as a string
void get_timestamp(char *timestamp, size_t size) {
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    strftime(timestamp, size, "%Y-%m-%d %H:%M:%S", tm_info);
}

// Function to log operations
void log_operation(const char *message) {
    int log_fd = open(LOG_FILE, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (log_fd == -1) {
        write_error("Error opening log file");
        return;
    }
    
    char timestamp[64];
    get_timestamp(timestamp, sizeof(timestamp));
    
    char log_entry[MAX_BUFFER];
    snprintf(log_entry, sizeof(log_entry), "[%s] %s\n", timestamp, message);
    
    if (write(log_fd, log_entry, strlen(log_entry)) == -1) {
        write_error("Error writing to log file");
    }
    
    close(log_fd);
}

// Create directory function
void create_directory(const char *dir_name) {
    struct stat st = {0};
    char log_message[MAX_BUFFER];
    char error_buf[MAX_BUFFER];
    
    // Check if directory already exists
    if (stat(dir_name, &st) != -1) {
        snprintf(error_buf, sizeof(error_buf), "Error: Directory \"%s\" already exists.\n", dir_name);
        write_stderr(error_buf);
        snprintf(log_message, sizeof(log_message), "Failed to create directory \"%s\". Directory already exists.", dir_name);
    } else {
        // Create directory with permissions 0755
        if (mkdir(dir_name, 0755) == 0) {
            snprintf(error_buf, sizeof(error_buf), "Directory \"%s\" created successfully.\n", dir_name);
            write_stdout(error_buf);
            snprintf(log_message, sizeof(log_message), "Directory \"%s\" created successfully.", dir_name);
        } else {
            write_error("Error creating directory");
            snprintf(log_message, sizeof(log_message), "Failed to create directory \"%s\". %s", dir_name, strerror(errno));
        }
    }
    
    log_operation(log_message);
}

// Create file function
void create_file(const char *file_name) {
    struct stat st = {0};
    char log_message[MAX_BUFFER];
    char error_buf[MAX_BUFFER];
    char timestamp[64];
    
    // Check if file already exists
    if (stat(file_name, &st) != -1) {
        snprintf(error_buf, sizeof(error_buf), "Error: File \"%s\" already exists.\n", file_name);
        write_stderr(error_buf);
        snprintf(log_message, sizeof(log_message), "Failed to create file \"%s\". File already exists.", file_name);
        log_operation(log_message);
        return;
    }
    
    // Create file with write permissions
    int fd = open(file_name, O_WRONLY | O_CREAT, 0644);
    if (fd == -1) {
        write_error("Error creating file");
        snprintf(log_message, sizeof(log_message), "Failed to create file \"%s\". %s", file_name, strerror(errno));
        log_operation(log_message);
        return;
    }
    
    // Get timestamp
    get_timestamp(timestamp, sizeof(timestamp));
    
    // Write timestamp to file
    char content[MAX_BUFFER];
    snprintf(content, sizeof(content), "File created at: %s\n", timestamp);
    
    if (write(fd, content, strlen(content)) == -1) {
        write_error("Error writing to file");
        close(fd);
        snprintf(log_message, sizeof(log_message), "Failed to write to file \"%s\". %s", file_name, strerror(errno));
        log_operation(log_message);
        return;
    }
    
    close(fd);
    snprintf(error_buf, sizeof(error_buf), "File \"%s\" created successfully.\n", file_name);
    write_stdout(error_buf);
    snprintf(log_message, sizeof(log_message), "File \"%s\" created successfully.", file_name);
    log_operation(log_message);
}

// List directory contents using fork()
void list_directory(const char *dir_name) {
    char log_message[MAX_BUFFER];
    char error_buf[MAX_BUFFER];
    
    // Check if directory exists
    struct stat st = {0};
    if (stat(dir_name, &st) == -1 || !S_ISDIR(st.st_mode)) {
        snprintf(error_buf, sizeof(error_buf), "Error: Directory \"%s\" not found.\n", dir_name);
        write_stderr(error_buf);
        snprintf(log_message, sizeof(log_message), "Failed to list directory \"%s\". Directory not found.", dir_name);
        log_operation(log_message);
        return;
    }
    
    pid_t pid = fork();
    
    if (pid < 0) {
        // Fork failed
        write_error("Fork failed");
        snprintf(log_message, sizeof(log_message), "Failed to list directory \"%s\". Fork failed: %s", dir_name, strerror(errno));
        log_operation(log_message);
        return;
    } else if (pid == 0) {
        // Child process
        DIR *dir;
        struct dirent *entry;
        
        dir = opendir(dir_name);
        if (dir == NULL) {
            snprintf(error_buf, sizeof(error_buf), "Error: Could not open directory \"%s\".\n", dir_name);
            write_stderr(error_buf);
            exit(EXIT_FAILURE);
        }
        
        snprintf(error_buf, sizeof(error_buf), "Contents of directory \"%s\":\n", dir_name);
        write_stdout(error_buf);
        
        while ((entry = readdir(dir)) != NULL) {
            snprintf(error_buf, sizeof(error_buf), "- %s\n", entry->d_name);
            write_stdout(error_buf);
        }
        
        closedir(dir);
        exit(EXIT_SUCCESS);
    } else {
        // Parent process
        int status;
        waitpid(pid, &status, 0);
        
        if (WIFEXITED(status) && WEXITSTATUS(status) == EXIT_SUCCESS) {
            snprintf(log_message, sizeof(log_message), "Listed contents of directory \"%s\" successfully.", dir_name);
        } else {
            snprintf(log_message, sizeof(log_message), "Failed to list contents of directory \"%s\".", dir_name);
        }
        
        log_operation(log_message);
    }
}

// List files by extension using fork()
void list_files_by_extension(const char *dir_name, const char *extension) {
    char log_message[MAX_BUFFER];
    char error_buf[MAX_BUFFER];
    
    // Check if directory exists
    struct stat st = {0};
    if (stat(dir_name, &st) == -1 || !S_ISDIR(st.st_mode)) {
        snprintf(error_buf, sizeof(error_buf), "Error: Directory \"%s\" not found.\n", dir_name);
        write_stderr(error_buf);
        snprintf(log_message, sizeof(log_message), "Failed to list files by extension in \"%s\". Directory not found.", dir_name);
        log_operation(log_message);
        return;
    }
    
    pid_t pid = fork();
    
    if (pid < 0) {
        // Fork failed
        write_error("Fork failed");
        snprintf(log_message, sizeof(log_message), "Failed to list files by extension in \"%s\". Fork failed: %s", 
                dir_name, strerror(errno));
        log_operation(log_message);
        return;
    } else if (pid == 0) {
        // Child process
        DIR *dir;
        struct dirent *entry;
        int found = 0;
        
        dir = opendir(dir_name);
        if (dir == NULL) {
            snprintf(error_buf, sizeof(error_buf), "Error: Could not open directory \"%s\".\n", dir_name);
            write_stderr(error_buf);
            exit(EXIT_FAILURE);
        }
        
        if ((strlen(extension) < 2) || extension[0] != '.') {
            write_stderr("Error: Extension must start with a period (e.g. \".txt\").\n");
            exit(EXIT_FAILURE);
        }
        
        snprintf(error_buf, sizeof(error_buf), "Files with extension \"%s\" in directory \"%s\":\n", extension, dir_name);
        write_stdout(error_buf);
        
        while ((entry = readdir(dir)) != NULL) {
            const char *file_name = entry->d_name;
            int name_len = strlen(file_name);
            int ext_len = strlen(extension);
            
            if (name_len > ext_len && 
                strcmp(file_name + name_len - ext_len, extension) == 0) {
                snprintf(error_buf, sizeof(error_buf), "- %s\n", file_name);
                write_stdout(error_buf);
                found = 1;
            }
        }
        
        if (!found) {
            snprintf(error_buf, sizeof(error_buf), "No files with extension \"%s\" found in \"%s\".\n", extension, dir_name);
            write_stdout(error_buf);
        }
        
        closedir(dir);
        exit(EXIT_SUCCESS);
    } else {
        // Parent process
        int status;
        waitpid(pid, &status, 0);
        
        if (WIFEXITED(status) && WEXITSTATUS(status) == EXIT_SUCCESS) {
            snprintf(log_message, sizeof(log_message), "Listed files with extension \"%s\" in directory \"%s\" successfully.", 
                    extension, dir_name);
        } else {
            snprintf(log_message, sizeof(log_message), "Failed to list files with extension \"%s\" in directory \"%s\".", 
                    extension, dir_name);
        }
        
        log_operation(log_message);
    }
}

// Read file function
void read_file(const char *file_name) {
    char log_message[MAX_BUFFER];
    char error_buf[MAX_BUFFER];
    char buffer[MAX_BUFFER];
    
    // Check if file exists
    struct stat st = {0};
    if (stat(file_name, &st) == -1) {
        snprintf(error_buf, sizeof(error_buf), "Error: File \"%s\" not found.\n", file_name);
        write_stderr(error_buf);
        snprintf(log_message, sizeof(log_message), "Failed to read file \"%s\". File not found.", file_name);
        log_operation(log_message);
        return;
    }
    
    // Open file for reading
    int fd = open(file_name, O_RDONLY);
    if (fd == -1) {
        write_error("Error opening file");
        snprintf(log_message, sizeof(log_message), "Failed to read file \"%s\". %s", file_name, strerror(errno));
        log_operation(log_message);
        return;
    }
    
    // Read and display file content
    snprintf(error_buf, sizeof(error_buf), "Contents of file \"%s\":\n", file_name);
    write_stdout(error_buf);
    
    ssize_t bytes_read;
    while ((bytes_read = read(fd, buffer, sizeof(buffer))) > 0) {
        write(STDOUT_FILENO, buffer, bytes_read);
    }
    
    if (bytes_read == -1) {
        write_error("Error reading file");
        snprintf(log_message, sizeof(log_message), "Error reading from file \"%s\". %s", file_name, strerror(errno));
        log_operation(log_message);
        close(fd);
        return;
    }
    
    write_stdout("\n");
    close(fd);
    snprintf(log_message, sizeof(log_message), "File \"%s\" read successfully.", file_name);
    log_operation(log_message);
}

// Append to file function with file locking
void append_to_file(const char *file_name, const char *content) {
    char log_message[MAX_BUFFER];
    char error_buf[MAX_BUFFER];
    
    // Check if file exists
    struct stat st = {0};
    if (stat(file_name, &st) == -1) {
        snprintf(error_buf, sizeof(error_buf), "Error: File \"%s\" not found.\n", file_name);
        write_stderr(error_buf);
        snprintf(log_message, sizeof(log_message), "Failed to append to file \"%s\". File not found.", file_name);
        log_operation(log_message);
        return;
    }
    
    // Open file for appending with file locking
    int fd = open(file_name, O_WRONLY | O_APPEND);
    if (fd == -1) {
        write_error("Error opening file");
        snprintf(log_message, sizeof(log_message), "Error: Cannot write to \"%s\". File is locked or read-only.\n", file_name);
        log_operation(log_message);
        return;
    }
    
    // Try to get an exclusive lock
    if (flock(fd, LOCK_EX | LOCK_NB) == -1) {
        snprintf(error_buf, sizeof(error_buf), "Error: Cannot write to \"%s\". File is locked or read-only.\n", file_name);
        write_stderr(error_buf);
        snprintf(log_message, sizeof(log_message), "Failed to append to file \"%s\". File is locked or read-only.", file_name);
        log_operation(log_message);
        close(fd);
        return;
    }
    
    // Write content to file
    if (write(fd, content, strlen(content)) == -1) {
        write_error("Error writing to file");
        flock(fd, LOCK_UN);  // Release the lock
        close(fd);
        snprintf(log_message, sizeof(log_message), "Failed to append to file \"%s\". %s", file_name, strerror(errno));
        log_operation(log_message);
        return;
    }
    
    // Add a newline if the content doesn't end with one
    if (content[strlen(content) - 1] != '\n') {
        if (write(fd, "\n", 1) == -1) {
            write_error("Error writing newline to file");
        }
    }
    
    // Release the lock and close the file
    flock(fd, LOCK_UN);
    close(fd);
    
    snprintf(error_buf, sizeof(error_buf), "Content appended to file \"%s\" successfully.\n", file_name);
    write_stdout(error_buf);
    snprintf(log_message, sizeof(log_message), "Content appended to file \"%s\" successfully.", file_name);
    log_operation(log_message);
}

// Delete file function using fork()
void delete_file(const char *file_name) {
    char log_message[MAX_BUFFER];
    char error_buf[MAX_BUFFER];
    
    // Check if file exists
    struct stat st = {0};
    if (stat(file_name, &st) == -1) {
        snprintf(error_buf, sizeof(error_buf), "Error: File \"%s\" not found.\n", file_name);
        write_stderr(error_buf);
        snprintf(log_message, sizeof(log_message), "Failed to delete file \"%s\". File not found.", file_name);
        log_operation(log_message);
        return;
    }
    
    pid_t pid = fork();
    
    if (pid < 0) {
        // Fork failed
        write_error("Fork failed");
        snprintf(log_message, sizeof(log_message), "Failed to delete file \"%s\". Fork failed: %s", file_name, strerror(errno));
        log_operation(log_message);
        return;
    } else if (pid == 0) {
        // Child process
        if (unlink(file_name) == 0) {
            snprintf(error_buf, sizeof(error_buf), "File \"%s\" deleted successfully.\n", file_name);
            write_stdout(error_buf);
            exit(EXIT_SUCCESS);
        } else {
            write_error("Error deleting file");
            exit(EXIT_FAILURE);
        }
    } else {
        // Parent process
        int status;
        waitpid(pid, &status, 0);
        
        if (WIFEXITED(status) && WEXITSTATUS(status) == EXIT_SUCCESS) {
            snprintf(log_message, sizeof(log_message), "File \"%s\" deleted successfully.", file_name);
        } else {
            snprintf(log_message, sizeof(log_message), "Failed to delete file \"%s\".", file_name);
        }
        
        log_operation(log_message);
    }
}

// Delete directory function using fork()
void delete_directory(const char *dir_name) {
    char log_message[MAX_BUFFER];
    char error_buf[MAX_BUFFER];
    
    // Check if directory exists
    struct stat st = {0};
    if (stat(dir_name, &st) == -1 || !S_ISDIR(st.st_mode)) {
        snprintf(error_buf, sizeof(error_buf), "Error: Directory \"%s\" not found.\n", dir_name);
        write_stderr(error_buf);
        snprintf(log_message, sizeof(log_message), "Failed to delete directory \"%s\". Directory not found.", dir_name);
        log_operation(log_message);
        return;
    }
    
    // Check if directory is empty
    DIR *dir = opendir(dir_name);
    if (dir == NULL) {
        write_error("Error opening directory");
        snprintf(log_message, sizeof(log_message), "Failed to delete directory \"%s\". %s", dir_name, strerror(errno));
        log_operation(log_message);
        return;
    }
    
    struct dirent *entry;
    int is_empty = 1;
    
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            is_empty = 0;
            break;
        }
    }
    
    closedir(dir);
    
    if (!is_empty) {
        snprintf(error_buf, sizeof(error_buf), "Error: Directory \"%s\" is not empty.\n", dir_name);
        write_stderr(error_buf);
        snprintf(log_message, sizeof(log_message), "Failed to delete directory \"%s\". Directory is not empty.", dir_name);
        log_operation(log_message);
        return;
    }
    
    pid_t pid = fork();
    
    if (pid < 0) {
        // Fork failed
        write_error("Fork failed");
        snprintf(log_message, sizeof(log_message), "Failed to delete directory \"%s\". Fork failed: %s", dir_name, strerror(errno));
        log_operation(log_message);
        return;
    } else if (pid == 0) {
        // Child process
        if (rmdir(dir_name) == 0) {
            snprintf(error_buf, sizeof(error_buf), "Directory \"%s\" deleted successfully.\n", dir_name);
            write_stdout(error_buf);
            exit(EXIT_SUCCESS);
        } else {
            write_error("Error deleting directory");
            exit(EXIT_FAILURE);
        }
    } else {
        // Parent process
        int status;
        waitpid(pid, &status, 0);
        
        if (WIFEXITED(status) && WEXITSTATUS(status) == EXIT_SUCCESS) {
            snprintf(log_message, sizeof(log_message), "Directory \"%s\" deleted successfully.", dir_name);
        } else {
            snprintf(log_message, sizeof(log_message), "Failed to delete directory \"%s\".", dir_name);
        }
        
        log_operation(log_message);
    }
}

// Show logs function
void show_logs() {
    char log_message[MAX_BUFFER];
    char buffer[MAX_BUFFER];
    
    int log_fd = open(LOG_FILE, O_RDONLY);
    if (log_fd == -1) {
        if (errno == ENOENT) {
            write_stdout("No logs found. Log file does not exist yet.\n");
            return;
        }
        
        write_error("Error opening log file");
        snprintf(log_message, sizeof(log_message), "Failed to show logs. %s", strerror(errno));
        log_operation(log_message);
        return;
    }
    
    write_stdout("Log entries:\n");
    
    ssize_t bytes_read;
    while ((bytes_read = read(log_fd, buffer, sizeof(buffer))) > 0) {
        write(STDOUT_FILENO, buffer, bytes_read);
    }
    
    if (bytes_read == -1) {
        write_error("Error reading log file");
        snprintf(log_message, sizeof(log_message), "Error reading from log file. %s", strerror(errno));
        log_operation(log_message);
        close(log_fd);
        return;
    }
    
    close(log_fd);
    snprintf(log_message, sizeof(log_message), "Logs displayed successfully.");
    log_operation(log_message);
}

// Display help
void display_help() {
    const char *help_text = 
        "Usage: fileManager <command> [arguments]\n"
        "Commands:\n"
        "  createDir \"folderName\" - Create a new directory\n"
        "  createFile \"fileName\" - Create a new file\n"
        "  listDir \"folderName\" - List all files in a directory\n"
        "  listFilesByExtension \"folderName\" \".txt\" - List files with specific extension\n"
        "  readFile \"fileName\" - Read a file's content\n"
        "  appendToFile \"fileName\" \"new content\" - Append content to a file\n"
        "  deleteFile \"fileName\" - Delete a file\n"
        "  deleteDir \"folderName\" - Delete an empty directory\n"
        "  showLogs - Display operation logs\n";
    
    write_stdout(help_text);
}

int main(int argc, char *argv[]) {
    char error_buf[MAX_BUFFER];
    
    // If no arguments provided, display help
    if (argc < 2) {
        display_help();
        return 0;
    }
    
    // Process commands
    if (strcmp(argv[1], "createDir") == 0) {
        if (argc != 3) {
            write_stderr("Error: createDir command requires a directory name.\n");
            return 1;
        }
        create_directory(argv[2]);
    } else if (strcmp(argv[1], "createFile") == 0) {
        if (argc != 3) {
            write_stderr("Error: createFile command requires a file name.\n");
            return 1;
        }
        create_file(argv[2]);
    } else if (strcmp(argv[1], "listDir") == 0) {
        if (argc != 3) {
            write_stderr("Error: listDir command requires a directory name.\n");
            return 1;
        }
        list_directory(argv[2]);
    } else if (strcmp(argv[1], "listFilesByExtension") == 0) {
        if (argc != 4) {
            write_stderr("Error: listFilesByExtension command requires a directory name and an extension.\n");
            return 1;
        }
        list_files_by_extension(argv[2], argv[3]);
    } else if (strcmp(argv[1], "readFile") == 0) {
        if (argc != 3) {
            write_stderr("Error: readFile command requires a file name.\n");
            return 1;
        }
        read_file(argv[2]);
    } else if (strcmp(argv[1], "appendToFile") == 0) {
        if (argc != 4) {
            write_stderr("Error: appendToFile command requires a file name and content.\n");
            return 1;
        }
        append_to_file(argv[2], argv[3]);
    } else if (strcmp(argv[1], "deleteFile") == 0) {
        if (argc != 3) {
            write_stderr("Error: deleteFile command requires a file name.\n");
            return 1;
        }
        delete_file(argv[2]);
    } else if (strcmp(argv[1], "deleteDir") == 0) {
        if (argc != 3) {
            write_stderr("Error: deleteDir command requires a directory name.\n");
            return 1;
        }
        delete_directory(argv[2]);
    } else if (strcmp(argv[1], "showLogs") == 0) {
        show_logs();
    } else {
        snprintf(error_buf, sizeof(error_buf), "Error: Unknown command '%s'\n", argv[1]);
        write_stderr(error_buf);
        display_help();
        return 1;
    }
    
    return 0;
} 