#include "pide_shop.h"

void* delivery_thread(void* arg) {
    DeliveryPerson* delivery_person = (DeliveryPerson*)arg;
    while (1) {
        pthread_mutex_lock(&order_queue.lock);

        while (order_queue.count == 0) {
            pthread_cond_wait(&order_queue.cond, &order_queue.lock);
        }

        Order order = order_queue.orders[0];
        for (int i = 0; i < order_queue.count - 1; i++) {
            order_queue.orders[i] = order_queue.orders[i + 1];
        }
        order_queue.count--;

        pthread_mutex_unlock(&order_queue.lock);

        delivery_person->is_available = 0;

        fprintf(log_file, "Delivery person %d started delivering order %d\n", delivery_person->id, order.order_id);
        fflush(log_file);

        // Simulate delivery time
        sleep(1 * SLEEP_MULTIPLIER);

        fprintf(log_file, "Delivery person %d delivered order %d\n", delivery_person->id, order.order_id);
        fflush(log_file);

        delivery_person->is_available = 1;
        delivery_person->deliveries_made++;
    }
    return NULL;
}
