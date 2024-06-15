#include "pide_shop.h"

double invert_time;

void setup_server(int port) {
    struct sockaddr_in server_addr;

    invert_time = calculate_invert_time();

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Failed to create socket");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Failed to bind socket");
        exit(EXIT_FAILURE);
    }

    if (listen(server_socket, 5) == -1) {
        perror("Failed to listen on socket");
        exit(EXIT_FAILURE);
    }

    printf("Server ip: %s\n", inet_ntoa(server_addr.sin_addr));
    printf("Server listening on port %d\n", port);
}

void* client_handler(void* arg) {
    int client_socket = *(int*)arg;
    free(arg);

    Order new_order;
    recv(client_socket, &new_order, sizeof(Order), 0);

    pthread_mutex_lock(&order_queue.lock);
    order_queue.orders[order_queue.count++] = new_order;
    pthread_cond_signal(&order_queue.cond);
    pthread_mutex_unlock(&order_queue.lock);

    fprintf(log_file, "Received order %d from customer %d\n", new_order.order_id, new_order.customer_id);
    fflush(log_file);

    close(client_socket);
    return NULL;
}

void accept_connections() {
    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        int client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_len);

        if (client_socket == -1) {
            perror("Failed to accept connection");
            continue;
        }

        int* pclient = malloc(sizeof(int));
        *pclient = client_socket;
        pthread_t thread;
        pthread_create(&thread, NULL, client_handler, pclient);
        pthread_detach(thread);
    }
}

