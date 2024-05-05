#include "neHos.h"

int client_count = 0;
pid_t client_pids[MAX_CLIENTS];
char *dirname;
volatile sig_atomic_t running = 1;


void handle_sigint(int sig) {
    // Send kill signal to all clients
    for (int i = 0; i < client_count; i++) {
        kill(client_pids[i], SIGKILL);
    }

    // Wait for all clients to terminate
    while (wait(NULL) > 0);

    printf("\nServer shutting down...\n");
    exit(0);
}

void handle_client(pid_t client_pid) {
    char buffer[MAX_LENGTH];
    char client_fifo_r[MAX_LENGTH], client_fifo_w[MAX_LENGTH];

    // Open client FIFO (previously created by client)
    snprintf(client_fifo_w, CLIENT_FIFO_NAME_LEN, CLIENT_FIFO_TEMPLATE1, (long) client_pid);
    snprintf(client_fifo_r, CLIENT_FIFO_NAME_LEN, CLIENT_FIFO_TEMPLATE2, (long) client_pid);
    int clientFdW = open(client_fifo_w, O_WRONLY);
    if (clientFdW == -1) {
        perror("open client FIFO for writing");
        exit(ERR_FIFO_OPEN);
    }
    int clientFdR = open(client_fifo_r, O_RDONLY);
    if (clientFdR == -1) {
        perror("open client FIFO for reading");
        exit(ERR_FIFO_OPEN);
    }

    while (1) {
        struct reqCommand req;
        struct respCommand respCmd;
        // Receive command from client
        int bytes_read = read(clientFd, &req, sizeof(struct reqCommand));
        if (bytes_read == -1) {
            perror("read from client FIFO");
            exit(ERR_FIFO_READ);
        } else if (bytes_read == 0) {
            break;
        }

        // Parse and execute command
        switch (req.cmd) {
            case LIST: {
                // Open directory
                DIR *dirp = opendir(dirname);
                if (dirp == NULL) {
                    perror("opendir error");
                    res.status = -1;
                    break;
                }

                // Read directory entries
                struct dirent *dp;
                char response_buffer[100000] = ""; 
                while ((dp = readdir(dirp)) != NULL) {
                    if (strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0) {
                        strcat(response_buffer, dp->d_name);
                        strcat(response_buffer, "\n"); 
                    }
                }

                // Send response in chunks
                int offset = 0;
                while (offset < strlen(response_buffer)) {
                    int bytes_to_send = MIN(MAX_LENGTH, strlen(response_buffer) - offset);
                    strncpy(respCmd.response, response_buffer + offset, bytes_to_send);
                    respCmd.status = (offset + bytes_to_send < strlen(response_buffer)) ? 1 : 0; // 1 if more data to send

                    if (write(clientFdW, &respCmd, sizeof(respCmd)) == -1) {
                        perror("write to client FIFO");
                        break; // Exit loop on error 
                    }

                    offset += bytes_to_send;
                }

                closedir(dirp);
                break;
            }
            case READF: {
                // Parse arguments
                char *filename = strtok(req.args, " ");
                char *line_str = strtok(NULL, " ");
                int line_num = (line_str != NULL) ? atoi(line_str) : -1; 

                // Open file
                char filepath[MAX_LENGTH];
                snprintf(filepath, MAX_LENGTH, "%s/%s", dirname, filename);
                int fd = open(filepath, O_RDONLY); 
                if (fd == -1) {
                    perror("open file");
                    res.status = -1;
                    break; 
                }

                // Lock file for reading
                struct flock lock;
                memset(&lock, 0, sizeof(lock));
                lock.l_type = F_RDLCK; // Shared lock for reading
                fcntl(fd, F_SETLKW, &lock); // Wait if lock is not available

                // Read file content
                char buffer[MAX_LENGTH];
                ssize_t bytes_read;
                int current_line = 1;
                while ((bytes_read = read(fd, buffer, MAX_LENGTH)) > 0) {
                    // Process buffer and send lines
                    int j = 0;
                    for (int i = 0; i < bytes_read; i++) {
                        if (buffer[i] == '\n') {
                            current_line++;
                        }

                        if (line_num == -1 || current_line == line_num) {
                            respCmd.response[j++] = buffer[i];
                        } 

                        // Check if buffer is full or end of line reached
                        if ((line_num == -1 && (i == MAX_LENGTH - 1 ||| i == bytes_read - 1)) || (line_num != -1 && current_line > line_num)) {
                            if (bytes_read != MAX_LENGTH) {
                                respCmd.status = 0; // End of file
                                respCmd.response[j] = '\0'; // Null-terminate string
                            } else {
                                respCmd.status = 1; // More data to send
                            }
                            // Send chunk
                            if (write(clientFdW, &respCmd, sizeof(respCmd)) == -1) {
                                perror("write to client FIFO");
                                break; 
                            }
                            memset(respCmd.response, 0, MAX_LENGTH); // Clear buffer 
                        }
                        
                        if (line_num != -1 && current_line > line_num) {
                            break; // Exit loop if specific line was read
                        }
                    }
                }

                // Unlock file
                lock.l_type = F_UNLCK; 
                fcntl(fd, F_SETLK, &lock);


                fclose(fp);
                break;
            }
            case WRITET: {
                // Parse arguments
                char *filename = strtok(req.args, " ");
                char *line_str = strtok(NULL, " ");
                int line_num = (line_str != NULL) ? atoi(line_str) : -1;
                char *string = strtok(NULL, ""); // Get the rest of the line as the string

                // Open file
                char filepath[MAX_LENGTH];
                snprintf(filepath, MAX_LENGTH, "%s/%s", dirname, filename);
                int fd = open(filepath, O_RDWR | O_CREAT, 0644); 
                if (fd == -1) {
                    perror("open file");
                    res.status = -1;
                    break; 
                }

                // Lock file for writing
                struct flock lock;
                memset(&lock, 0, sizeof(lock));
                lock.l_type = F_WRLCK; // Exclusive lock for writing
                fcntl(fd, F_SETLKW, &lock); 

                char buffer[MAX_LENGTH]; 
                

                // Modify buffer 
                if (line_num == -1) {
                    // Append end of file
                    int offset = lseek(fd, 0, SEEK_END); // Move to end of file
                    if (offset == -1 || write(fd, string, strlen(string)) == -1 || write(fd, "\n", 1) == -1){
                        perror("write to file");
                        res.status = -1;
                        break; 
                    }
                } else {
                    // Copy file to a temp file, inserting the new line
                    char temp_filepath[MAX_LENGTH];
                    int current_line = 1;
                    snprintf(temp_filepath, MAX_LENGTH, "%s/%s.tmp", dirname, filename);
                    int temp_fd = open(temp_filepath, O_CREAT | O_WRONLY, 0644);
                    if (temp_fd == -1) {
                        perror("open temp file");
                        res.status = -1;
                        break; 
                    }
                    while (read(fd, buffer, MAX_LENGTH) > 0) {
                        for (int i = 0; i < strlen(buffer); i++) {
                            if (current_line == line_num) {
                                write(temp_fd, string, strlen(string));
                                write(temp_fd, "\n", 1);
                            }
                            write(temp_fd, buffer + i, 1);
                            if (buffer[i] == '\n') {
                                current_line++;
                            }
                        }
                    }

                    // Unlock file
                    lock.l_type = F_UNLCK; 
                    fcntl(fd, F_SETLK, &lock);

                    // Close and write back to original file
                    close(fd);
                    close(temp_fd);

                    // Lock file
                    struct flock lock;
                    memset(&lock, 0, sizeof(lock));
                    lock.l_type = F_WRLCK; // Exclusive lock for writing
                    fcntl(fd, F_SETLKW, &lock);                   

                    temp_fd = open(temp_filepath, O_RDONLY);
                    fd = open(filepath, O_WRONLY | O_TRUNC, 0644);
                    if (temp_fd == -1 || fd == -1) {
                        perror("open temp file for reading or original file for writing");
                        res.status = -1;
                        break; 
                    }
                    while (read(temp_fd, buffer, MAX_LENGTH) > 0) {
                        write(fd, buffer, strlen(buffer));
                    }                 
                }

                // Unlock file
                lock.l_type = F_UNLCK; 
                fcntl(fd, F_SETLK, &lock);

                close(fd);
                break;
            } 
        }

    }

    close(clientFd);
    client_count--;

    // Remove client PID from array
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (client_pids[i] == getpid()) {
            client_pids[i] = 0;
            break;
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <dirname> <max_clients>\n", argv[0]);
        exit(1);
    }

    dirname = argv[1];
    int max_clients = atoi(argv[2]);
    int serverFd, clientFd;
    char clientFifo[MAX_LENGTH];
    char buffer[MAX_LENGTH];
    pid_t queue[MAX_CLIENTS];
    struct request req;
    struct response resp;
    int seqNum = 0;

    // Create directory if it doesn't exist
    mkdir(dirname, 0777);

    umask(0);    /* We get the permissions we want */
    if (mkfifo(SERVER_FIFO, S_IRUSR | S_IWUSR | S_IWGRP) == -1
            && errno != EEXIST){
        perror("mkfifo error");
        exit(ERR_FIFO_CREATE);
    }
    printf("Server Started PID: %d\n", getpid());
    printf("Waiting for clients...\n");
    serverFd = open(SERVER_FIFO, O_RDONLY);
    if (serverFd == -1){
        perror("open error");
        exit(ERR_FIFO_OPEN);
    }

    if (signal(SIGPIPE, SIG_IGN) == SIG_ERR){
        perror("signal error");
        exit(ERR_SIGNAL);
    }

    // Create log file
    char log_filename[1024];
    // Concatenate the dirname with "/server.log" manually
    strcpy(log_filename, dirname);
    strcat(log_filename, "/server.log");
    int log_fd = open(log_filename, O_CREAT | O_WRONLY | O_APPEND, 0644);
    if (log_fd == -1) {
        perror("Log file open error");
        exit(ERR_LOG_CREATE);
    }

    // Register signal handler
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = &handle_sigint;
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("Error setting SIGINT handler");
        exit(1);
    }

    // Print server information
    printf("Server started. PID: %d\n", getpid());
    printf("Waiting for clients...\n");

    while (running) {

        if (read(serverFd, &req, sizeof(struct request)) != sizeof(struct request)) {
            fprintf(stderr, "Error reading request; discarding\n");
            continue; 
        }



        // Open client FIFO (previously created by client) 
        snprintf(clientFifo, CLIENT_FIFO_NAME_LEN, CLIENT_FIFO_TEMPLATE1, (long) req.pid);
        clientFd = open(clientFifo, O_WRONLY);
        if (clientFd == -1) { /* Open failed, give up on client */
            errMsg("open %s", clientFifo);
            continue;
        }
        
        
        if (client_count >= max_clients) {
            printf("Connection request PID %d... Que FULL\n", req.pid);
        }
        
        do {
            resp.accepted = 1;
            // Check if maximum client limit reached
            if (client_count >= max_clients) {
                // Send error message to client and close connection
                resp.accepted = 0;
                if (write(clientFd, &resp, sizeof(struct response)) != sizeof(struct response))
                    fprintf(stderr, "Error writing to FIFO %s\n", clientFifo);
                close(clientFd);
                continue;
            }
            // sleep a bit
            usleep(1000);
        } while(!resp.accepted);

        // Send response to client
        if (write(clientFd, &resp, sizeof(struct response)) != sizeof(struct response))
            fprintf(stderr, "Error writing to FIFO %s\n", clientFifo);
        if (close(clientFd) == -1)
            errMsg("close");

        // Fork a child process to handle the client
        pid_t pid = fork();
        if (pid == -1) {
            perror("fork");
            continue;
        } else if (pid == 0) {
            // Child process
            close(serverFd); // Child doesn't need the server FIFO
            handle_client(req.pid);
            _exit(0);
        } else {
            // Parent process
            client_pids[client_count++] = pid;
            printf("Client PID %d connected.\n", pid);
        }
    }

    // Close log file
    close(log_fd);

    unlink(SERVER_FIFO);

    return 0;
}