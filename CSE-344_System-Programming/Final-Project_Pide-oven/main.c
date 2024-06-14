#include "pide_shop.h"

int main(int argc, char* argv[]) {
    if (argc != 5) {
        fprintf(stderr, "Usage: %s [portnumber] [CookthreadPoolSize] [DeliveryPoolSize] [k]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int port = atoi(argv[1]);
    int cook_count = atoi(argv[2]);
    int delivery_count = atoi(argv[3]);
    int delivery_speed = atoi(argv[4]);

    srand(time(NULL));

    initialize();
    setup_signal_handling();
    setup_server(port);
    start_threads(cook_count, delivery_count);
    accept_connections();

    return 0;
}
