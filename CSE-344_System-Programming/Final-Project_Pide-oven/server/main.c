#include "pide_shop.h"

int port, cook_count, delivery_count, delivery_speed;
int stop_server = 0;

int main(int argc, char* argv[]) {
    if (argc != 5) {
        fprintf(stderr, "Usage: %s [portnumber] [CookthreadPoolSize] [DeliveryPoolSize] [k]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    port = atoi(argv[1]);
    cook_count = atoi(argv[2]);
    delivery_count = atoi(argv[3]);
    delivery_speed = atoi(argv[4]);


    srand(time(NULL));

    initialize(delivery_speed);
    setup_signal_handling();
    setup_server(port);
    start_threads(cook_count, delivery_count);
    accept_connections();

    for (int i = 0; i < cook_count; i++) {
        pthread_join(cooks[i].thread, NULL);
    }
    for (int i = 0; i < delivery_count; i++) {
        pthread_join(delivery_personnel[i].thread, NULL);
    }
    pthread_join(manager.thread, NULL);

    return 0;
}
