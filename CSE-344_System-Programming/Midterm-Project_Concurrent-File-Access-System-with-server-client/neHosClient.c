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
    unlink(clientFifo);
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
        perror("open client fifo");
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
        perror("open client fifo for writing");
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
    reqCmd.status = 0;
    int send_status = 0;

    // Parse and send command to server
    char cmd_str[30];
    int i = 0;
    for (; command[i] != '\0' && command[i] != '\n' && command[i] != ' '; i++)
    {
        cmd_str[i] = command[i];
    }
    cmd_str[i] = '\0';
    printf("Command:%s.\n", cmd_str);
    if (cmd_str == NULL)
    {
        fprintf(stderr, "Invalid command\n");
        return 1;
    }
    if (strcmp(cmd_str, "help") == 0)
    {
        // Check if there are any arguments
        char arg[MAX_LENGTH];
        i++;
        for (int j=0; command[i] != '\0' && command[i] != '\n'; i++, j++)
        {
            arg[j] = command[i];
        }

        if (arg == NULL)
        {
            printf("Available comments are :\n");
            printf("help, list, readF, writeT, upload, download, archServer, quit, killServer\n");
        }
        else if (strcmp(arg, "help") == 0)
        {
            printf("    help <command>\n");
            printf("        Display help for a specific command. If no command is specified, display available commands.\n");
        }
        else if (strcmp(arg, "list") == 0)
        {
            printf("    list\n");
            printf("        display the list of files in Servers directory\n");
        }
        else if (strcmp(arg, "readF") == 0)
        {
            printf("    readF <filename> <line>\n");
            printf("        display the # line of the <file>, if no line number is given\n");
            printf("        display the entire file\n");
        }
        else if (strcmp(arg, "writeT") == 0)
        {
            printf("    writeT <filename> <line> <string>\n");
            printf("        write <string> to the <line> of the <file>, if no line number is given\n");
            printf("        append the <string> to the end of the file\n");
        }
        else if (strcmp(arg, "upload") == 0)
        {
            printf("    upload <filename>\n");
            printf("        upload a file to Servers directory\n");
        }
        else if (strcmp(arg, "download") == 0)
        {
            printf("    download <filename>\n");
            printf("        download a file from Servers directory\n");
        }
        else if (strcmp(arg, "archServer") == 0)
        {
            printf("    archServer <archive_filename>\n");
            printf("        archive the Servers directory\n");
        }
        else if (strcmp(arg, "killServer") == 0)
        {
            printf("    killServer\n");
            printf("        kill the server\n");
        }
        else if (strcmp(arg, "quit") == 0)
        {
            printf("    quit\n");
            printf("        quit the client\n");
        }
        else
        {
            printf("Available comments are :\n");
            printf("help, list, readF, writeT, upload, download, archServer, quit, killServer\n");
        }
    }
    else if (strcmp(cmd_str, "list") == 0)
    {
        reqCmd.cmd = LIST;
        reqCmd.pid = getpid();
        write(serverFd, &reqCmd, sizeof(struct reqCommand));

        // Read response from server and print it
        struct respCommand respCmd;
        while (read(clientFd, &respCmd, sizeof(respCmd)) > 0)
        {
            printf("%s", respCmd.response);
            if (respCmd.status == 0)
            {
                break; // End of list
            }
        }
    }
    else if (strcmp(cmd_str, "readF") == 0)
    {
        char filename[MAX_LENGTH];
        i++;
        for (int j = 0; command[i] != '\0' && command[i] != '\n' && command[i] != ' '; i++, j++)
        {
            filename[j] = command[i];
        }
        if (filename == NULL)
        {
            fprintf(stderr, "readF: Missing filename\n");
            return 1;
        }
        char line_str[MAX_LENGTH];
        i++;
        for (int j = 0; command[i] != '\0' && command[i] != '\n'; i++, j++)
        {
            line_str[j] = command[i];
        }

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
        reqCmd.pid = getpid();
        write(serverFd, &reqCmd, sizeof(struct reqCommand));

        // Read response from server and print it
        struct respCommand respCmd;
        while (read(clientFd, &respCmd, sizeof(respCmd)) > 0)
        {
            printf("%s", respCmd.response);
            if (respCmd.status == 0)
            {
                break; // End of file
            }
        }
    }
    else if (strcmp(cmd_str, "writeT") == 0)
    {
        char filename[MAX_LENGTH];
        i++;
        for (int j = 0; command[i] != '\0' && command[i] != '\n' && command[i] != ' '; i++, j++)
        {
            filename[j] = command[i];
        }
        if (filename == NULL)
        {
            fprintf(stderr, "writeT: Missing filename\n");
            return 1;
        }
        char line_str[MAX_LENGTH];
        i++;
        for (int j = 0; command[i] != '\0' && command[i] != '\n' && command[i] != ' '; i++, j++)
        {
            line_str[j] = command[i];
        }
        int line_num = -1; // Default: append to file
        if (line_str == NULL)
        {
            fprintf(stderr, "writeT: Missing string to write\n");
            return 1;
        }
         // Get the rest of the line as the string
        char string[MAX_LENGTH];
        i++;
        for (int j = 0; command[i] != '\0' && command[i] != '\n'; i++, j++)
        {
            string[j] = command[i];
        } 

        if (string != NULL)
        {
            line_num = atoi(line_str);
        }
        snprintf(reqCmd.args, MAX_LENGTH, "%s %d %s", filename, line_num, string);
        reqCmd.cmd = WRITET;
    }
    else if (strcmp(cmd_str, "upload") == 0)
    {
        char filename[MAX_LENGTH];
        i++;    
        for (int j = 0; command[i] != '\0' && command[i] != '\n'; i++, j++)
        {
            filename[j] = command[i];
        }
        
        if (filename == NULL)
        {
            fprintf(stderr, "upload: Missing filename\n");
            return 1;
        }
        strcpy(reqCmd.args, filename);
        // write the file to the server starting from this point
        int fd = open(filename, O_RDONLY);
        if (fd == -1)
        {
            perror("open");
            return 1;
        }
        // Lock the file for wrting
        struct flock fl;
        fl.l_type = F_WRLCK;
        fl.l_whence = SEEK_SET;
        fl.l_start = 0;
        fl.l_len = 0;
        if (fcntl(fd, F_SETLKW, &fl) == -1)
        {
            perror("fcntl");
            return 1;
        }

        reqCmd.args[strlen(filename)] = ' ';
        
        // Read the file and send it to the server with requests
        int n;
        reqCmd.cmd = UPLOAD;
        n = read(fd, reqCmd.args + strlen(filename) + 1, MAX_LENGTH - strlen(filename) - 1);
        if (n == -1)
        {
            perror("read");
            return 1;
        }
        else if (n == MAX_LENGTH - strlen(filename) - 1)
        {
            reqCmd.status = 1;
        }
        write(serverFd, &reqCmd, sizeof(struct reqCommand));
        send_status = 1;

        while ((n = read(fd, reqCmd.args, MAX_LENGTH)) > 0)
        {
            if (n != MAX_LENGTH)
            {
                reqCmd.status = 0;
                reqCmd.args[n] = '\0';
            }
            write(serverFd, &reqCmd, sizeof(struct reqCommand));
        }
    }
    else if (strcmp(cmd_str, "download") == 0)
    {
        // Send the download command to the server
        char filename[MAX_LENGTH];
        i++;
        for (int j = 0; command[i] != '\0' && command[i] != '\n'; i++, j++)
        {
            filename[j] = command[i];
        }
        if (filename == NULL)
        {
            fprintf(stderr, "download: Missing filename\n");
            return 1;
        }
        strcpy(reqCmd.args, filename);
        reqCmd.cmd = DOWNLOAD;


         // Open file for writing
        char *filename2 = reqCmd.args;
        int fd = open(filename2, O_WRONLY | O_CREAT | O_TRUNC, 0644); 
        if (fd == -1) {
            perror("open");
            return 1;
        }

        // Receive file content in chunks
        struct respCommand respCmd;
        while (read(serverFd, &respCmd, sizeof(respCmd)) > 0) {
            if (write(fd, respCmd.response, strlen(respCmd.response)) == -1) {
                perror("write");
                break;
            if (respCmd.status == 0) {
                break; // End of file
            }
            }
        }

        close(fd);
        send_status = 1;
    }
    else if (strcmp(cmd_str, "archServer") == 0)
    {
        char archive_filename[MAX_LENGTH];
        i++;
        for (int j = 0; command[i] != '\0' && command[i] != '\n'; i++, j++)
        {
            archive_filename[j] = command[i];
        }

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
        reqCmd.pid = getpid();
        write(serverFd, &reqCmd, sizeof(struct reqCommand));
        removeFifo();
        exit(0);
    }
    else
    {
        fprintf(stderr, "Unknown command: %s.\n", cmd_str);
        return 1;
    }

    return 0;
}