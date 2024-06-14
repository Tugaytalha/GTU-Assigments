#include "pide_shop.h"

OrderQueue order_queue;
Cook cooks[MAX_COOKS];
DeliveryPerson delivery_personnel[MAX_DELIVERY_PERSONNEL];
pthread_mutex_t oven_lock;
pthread_cond_t oven_cond;
int current_oven_load;
FILE *log_file;
int server_socket;

void initialize() {
    pthread_mutex_init(&order_queue.lock, NULL);
    pthread_cond_init(&order_queue.cond, NULL);
    order_queue.count = 0;

    pthread_mutex_init(&oven_lock, NULL);
    pthread_cond_init(&oven_cond, NULL);
    current_oven_load = 0;

    for (int i = 0; i < MAX_COOKS; i++) {
        cooks[i].id = i;
        cooks[i].is_available = 1;
    }

    for (int i = 0; i < MAX_DELIVERY_PERSONNEL; i++) {
        delivery_personnel[i].id = i;
        delivery_personnel[i].is_available = 1;
        delivery_personnel[i].deliveries_made = 0;
    }

    log_file = fopen("pide_shop.log", "w");
    if (log_file == NULL) {
        perror("Failed to open log file");
        exit(EXIT_FAILURE);
    }
}

void handle_signal(int signal) {
    if (signal == SIGINT) {
        fprintf(log_file, "Server shutting down...\n");
        fclose(log_file);
        close(server_socket);
        exit(0);
    }
}

void setup_signal_handling() {
    struct sigaction sa;
    sa.sa_handler = handle_signal;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("Failed to setup signal handler");
        exit(EXIT_FAILURE);
    }
}

void start_threads(int cook_count, int delivery_count) {
    for (int i = 0; i < cook_count; i++) {
        pthread_create(&cooks[i].thread, NULL, cook_thread, &cooks[i]);
    }

    for (int i = 0; i < delivery_count; i++) {
        pthread_create(&delivery_personnel[i].thread, NULL, delivery_thread, &delivery_personnel[i]);
    }
}
