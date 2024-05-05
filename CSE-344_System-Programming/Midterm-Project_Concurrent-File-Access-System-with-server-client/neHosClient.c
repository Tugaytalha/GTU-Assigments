#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <signal.h>
#include "neHos.h"

int serverFd = -1, clientFd = -1;
char clientFifo[MAX_LENGTH], clientFifoW[MAX_LENGTH];
char buffer[MAX_LENGTH];
struct request req;
struct response resp;

void removeFifo()
{
    unlink(clientFifo)
    unlink(clientFifoW);
    close(serverFd);
}

void handle_sigint(int sig);

int parse_and_send_command(int serverFd, char *command);

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s <Connect/tryConnect> <serverPID>\n", argv[0]);
        exit(1);
    }

    char *connect_type = argv[1];
    pid_t server_pid = atoi(argv[2]);

    // Create client FIFO
    snprintf(clientFifo, CLIENT_FIFO_NAME_LEN, CLIENT_FIFO_TEMPLATE1, (long)getpid());
    if (mkfifo(clientFifo, S_IRUSR | S_IWUSR | S_IWGRP) == -1 && errno != EEXIST)
    {
        perror("mkfifo");
        exit(1);
    }
    snprintf(clientFifoW, CLIENT_FIFO_NAME_LEN, CLIENT_FIFO_TEMPLATE2, (long)getpid());
    if (mkfifo(clientFifoW, S_IRUSR | S_IWUSR | S_IWGRP) == -1 && errno != EEXIST)
    {
        perror("mkfifo");
        exit(1);
    }
    if (atexit(removeFifo) != 0)
    {
        perror("atexit");
        exit(1);
    }

    // Open server FIFO
    serverFd = open(SERVER_FIFO, O_WRONLY);
    if (serverFd == -1)
    {
        perror("open");
        exit(1);
    }

    // Send connection request
    req.pid = getpid();
    write(serverFd, &req, sizeof(struct request));

    // Open client FIFO for reading (server will write response here)
    clientFd = open(clientFifo, O_RDONLY);
    if (clientFd == -1)
    {
        perror("open");
        exit(1);
    }

    // Read response from server
    if (read(clientFd, &resp, sizeof(struct response)) != sizeof(struct response))
    {
        fprintf(stderr, "Error reading server response\n");
        exit(1);
    }

    if (!resp.accepted)
    {
        if (strcmp(connect_type, "Connect") == 0)
        {
            printf("Server queue is full. Waiting for a slot...\n");
            while (read(clientFd, &resp, sizeof(struct response)) != sizeof(struct response) || !resp.accepted)
                ;
            printf("Connection established.\n");
        }
        else
        {
            printf("Server queue is full. Exiting...\n");
            exit(1);
        }
    }
    else
    {
        printf("Connection established.\n");
    }

    int clientFdW = open(clientFifoW, O_WRONLY);
    if (clientFdW == -1)
    {
        perror("open");
        exit(1);
    }    

    // Register signal handler
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = &handle_sigint;
    if (sigaction(SIGINT, &sa, NULL) == -1)
    {
        perror("Error setting SIGINT handler");
        exit(1);
    }

    // Main loop for receiving commands
    while (1)
    {
        printf("Enter command: ");
        if (fgets(buffer, sizeof(buffer), stdin) == NULL)
        {
            break; // EOF
        }

        // Parse and send command to server
        parse_and_send_command(clientFdW, buffer);

        // Read response from server and print it
    }

    removeFifo(); // Cleanup client FIFO

    return 0;
}


void handle_sigint(int sig)
{
    // Send "quit" command to server
    struct reqCommand reqCmd;
    reqCmd.cmd = QUIT;
    reqCmd.pid = getpid();
    if (clientFd == -1)
        clientFd = open(clientFifo, O_WRONLY);
    if (clientFd == -1)
    {
        perror("open client fifo");
        exit(1);
    }
    write(clientFd, &reqCmd, sizeof(struct reqCommand));
    removeFifo();
    exit(0);
}


int parse_and_send_command(int serverFd, char *command)
{
    struct reqCommand reqCmd;

    // Parse and send command to server
    char *cmd_str = strtok(command, " ");
    if (cmd_str == NULL)
    {
        fprintf(stderr, "Invalid command\n");
        return;
    }
    if (strcmp(cmd_str, "help") == 0)
    {
        printf("Commands:\n");
        printf("--> list\n");
        printf("        List all files in the server\n");
        printf("--> readF <filename> [line_num]\n");
        printf("        Read file <filename> (optionally line number)\n");
        printf("--> writeT <filename> <line_num> <string>\n");
        printf("        Write <string> to file <filename> at line <line_num>\n");
        printf("--> upload <filename>\n");
        printf("        Upload file <filename> to server\n");
        printf("--> download <filename>\n");
        printf("        Download file <filename> from server\n");
        printf("--> archServer <archive_filename>\n");
        printf("        Archive server files to <archive_filename>\n");
        printf("--> killServer\n");
        printf("        Kill server\n");
        printf("--> quit\n");
        printf("        Quit client\n");
    }
    else if (strcmp(cmd_str, "list") == 0)
    {
        reqCmd.cmd = LIST;
    }
    else if (strcmp(cmd_str, "readF") == 0)
    {
        char *filename = strtok(NULL, " ");
        if (filename == NULL)
        {
            fprintf(stderr, "readF: Missing filename\n");
            return;
        }
        char *line_str = strtok(NULL, "\n");
        for (int i = 0; filename[i] != '\0'; i++)
        {
            reqCmd.args[i] = filename[i];
        }
        if (line_str != NULL)
        {
            reqCmd.args[strlen(filename)] = ' ';
            for (int i = 0; line_str[i] != '\0'; i++)
            {
                reqCmd.args[strlen(filename) + 1 + i] = line_str[i];
            }
        }
        reqCmd.args[strlen(filename) + (line_str == NULL ? 0 : strlen(line_str) + 1)] = '\0';
        reqCmd.cmd = READF;
    }
    else if (strcmp(cmd_str, "writeT") == 0)
    {
        char *filename = strtok(NULL, " ");
        if (filename == NULL)
        {
            fprintf(stderr, "writeT: Missing filename\n");
            return 1;
        }
        char *line_str = strtok(NULL, " ");
        int line_num = -1; // Default: append to file
        if (line_str == NULL)
        {
            fprintf(stderr, "writeT: Missing string to write\n");
            return 1;
        }
        char *string = strtok(NULL, ""); // Get the rest of the line as the string
        if (string != NULL)
        {
            line_num = atoi(line_str);
        }
        snprintf(reqCmd.args, MAX_LENGTH, "%s %d %s", filename, line_num, string);
        reqCmd.cmd = WRITET;
    }
    else if (strcmp(cmd_str, "upload") == 0)
    {
        char *filename = strtok(NULL, " ");
        if (filename == NULL)
        {
            fprintf(stderr, "upload: Missing filename\n");
            return 1;
        }
        strcpy(reqCmd.args, filename);
        reqCmd.cmd = UPLOAD;
    }
    else if (strcmp(cmd_str, "download") == 0)
    {
        char *filename = strtok(NULL, " ");
        if (filename == NULL)
        {
            fprintf(stderr, "download: Missing filename\n");
            return 1;
        }
        strcpy(reqCmd.args, filename);
        reqCmd.cmd = DOWNLOAD;
    }
    else if (strcmp(cmd_str, "archServer") == 0)
    {
        char *archive_filename = strtok(NULL, " ");
        if (archive_filename == NULL)
        {
            fprintf(stderr, "archServer: Missing archive filename\n");
            return 1;
        }
        strcpy(reqCmd.args, archive_filename);
        reqCmd.cmd = ARCHSERVER;
    }
    else if (strcmp(cmd_str, "killServer") == 0)
    {
        reqCmd.cmd = KILLSERVER;
    }
    else if (strcmp(cmd_str, "quit") == 0)
    {
        reqCmd.cmd = QUIT;
    }
    else
    {
        fprintf(stderr, "Unknown command: %s\n", cmd_str);
        return 1;
    }

    reqCmd.pid = getpid();
    write(serverFd, &reqCmd, sizeof(struct reqCommand));

    if (reqCmd.cmd == QUIT)
    {
        removeFifo();
        exit(0);
    }

    return 0;
}