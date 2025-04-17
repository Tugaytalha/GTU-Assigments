#include "pide_shop.h"

#define BUFFER_SIZE 1024
#define SERVER_PORT 8001

char* server_port;
int number_of_clients;
int town_size_p;
int town_size_q;
int stop = 0;
static int order_id = 1;

void handle_signal(int signal) {
    stop = 1;
    printf("Client generator received shutdown signal\n");
    exit(0);
}

void* client_thread(void* arg) {
    int sockfd;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    int client_id = *((int*)arg);

    free(arg);

    // Generate random position
    srand(time(NULL) + client_id);
    float x = (float)(rand() % (town_size_p * 100)) / 100.0;
    float y = (float)(rand() % (town_size_q * 100)) / 100.0;

    // Create socket
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        return NULL;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = inet_addr(server_port);

    // Connect to the server
    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection to server failed");
        close(sockfd);
        return NULL;
    }

    printf("Client %d connected to server at (%.2f, %.2f)\n", client_id, x, y);

    // Send position data to server
    snprintf(buffer, sizeof(buffer), "Client %d: position (%.2f, %.2f)\n", client_id, x, y);
    Order order;
    order.order_id = order_id++;
    order.customer_id = client_id;
    order.x = x;
    order.y = y;
    order.order_time = time(NULL);
    send(sockfd, &order, sizeof(order), 0);

    // Receive server response
    int bytes_received = recv(sockfd, buffer, sizeof(buffer) - 1, 0);
    if (bytes_received > 0) {
        buffer[bytes_received] = '\0';
        printf("Server response for client %d: %s\n", client_id, buffer);
    } else if (bytes_received == 0) {
        printf("Client %d: Server closed connection\n", client_id);
    } else {
        perror("Recv error");
    }

    close(sockfd);
    return NULL;
}

int main(int argc, char* argv[]) {
    if (argc != 5) {
        fprintf(stderr, "Usage: %s [portnumber] [numberOfClients] [p] [q]\n", argv[0]);
        exit(1);
    }

    server_port = (argv[1]);
    number_of_clients = atoi(argv[2]);
    town_size_p = atoi(argv[3]);
    town_size_q = atoi(argv[4]);

    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);

    pthread_t threads[number_of_clients];
    for (int i = 0; i < number_of_clients; i++) {
        int* client_id = malloc(sizeof(int));
        *client_id = i + 1;
        pthread_create(&threads[i], NULL, client_thread, client_id);
        usleep(100000);  // Stagger client creation
    }

    for (int i = 0; i < number_of_clients; i++) {
        pthread_join(threads[i], NULL);
    }

    return 0;
}
