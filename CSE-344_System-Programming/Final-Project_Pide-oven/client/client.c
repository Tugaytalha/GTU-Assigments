#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include <time.h>
#include "pide_shop.h"

#define SERVER_PORT 8080 // You can change this to the port your server is listening on

void send_order(int customer_id, int order_id, const char *server_ip) {
    int sockfd;
    struct sockaddr_in server_addr;
    Order order;

    // Create socket
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        exit(EXIT_FAILURE);
    }

    // Connect to the server
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // Create an order
    order.order_id = order_id;
    order.customer_id = customer_id;
    order.x = rand() % 100;  // Random x coordinate
    order.y = rand() % 100;  // Random y coordinate
    order.order_time = time(NULL);

    // Send order to the server
    if (send(sockfd, &order, sizeof(order), 0) < 0) {
        perror("Send failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("Order %d from customer %d sent to the server.\n", order_id, customer_id);

    close(sockfd);
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <customer_id> <order_id> <server_ip>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int customer_id = atoi(argv[1]);
    int order_id = atoi(argv[2]);
    const char *server_ip = argv[3];

    srand(time(NULL));

    send_order(customer_id, order_id, server_ip);

    return 0;
}
