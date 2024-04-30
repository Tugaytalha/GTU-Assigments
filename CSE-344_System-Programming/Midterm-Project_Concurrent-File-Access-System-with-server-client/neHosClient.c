#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define PORT 8080 // Change this to your desired port

void error(const char *msg) {
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <Connect/tryConnect> <ServerPID>\n", argv[0]);
        exit(1);
    }

    char *command = argv[1];
    int server_pid = atoi(argv[2]);

    // Create socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        error("ERROR opening socket");
    }

    // Get server address
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(PORT);

    // Connect to server
    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        error("ERROR connecting");
    }

    // Send connection request
    char buffer[256];
    snprintf(buffer, sizeof(buffer), "%s %d\n", command, getpid());
    write(sockfd, buffer, strlen(buffer));

    // Receive server response
    int bytes_read = read(sockfd, buffer, sizeof(buffer));
    if (bytes_read <= 0) {
        error("ERROR reading from socket");
    }
    printf("%s", buffer);

    // Handle user input and send commands to server
    while (1) {
        printf("> ");
        fgets(buffer, sizeof(buffer), stdin);

        // Remove trailing newline
        buffer[strcspn(buffer, "\n")] = 0;

        // Send command to server
        write(sockfd, buffer, strlen(buffer));

        // Receive server response
        bytes_read = read(sockfd, buffer, sizeof(buffer));
        if (bytes_read <= 0) {
            break; // Server disconnected
        }
        printf("%s", buffer);

        // Check for quit command
        if (strcmp(buffer, "quit") == 0) {
            break;
        }
    }

    // Close socket
    close(sockfd);

    return 0;
}