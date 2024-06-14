#include "pide_shop.h"

void* cook_thread(void* arg) {
    Cook* cook = (Cook*)arg;
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

        cook->is_available = 0;

        fprintf(log_file, "Cook %d started preparing order %d\n", cook->id, order.order_id);
        fflush(log_file);

        // Simulate preparation time
        sleep(rand() % 5 + 1);

        pthread_mutex_lock(&oven_lock);
        while (current_oven_load >= OVEN_CAPACITY) {
            pthread_cond_wait(&oven_cond, &oven_lock);
        }
        current_oven_load++;
        pthread_mutex_unlock(&oven_lock);

        fprintf(log_file, "Cook %d placed order %d in the oven\n", cook->id, order.order_id);
        fflush(log_file);

        // Simulate cooking time
        sleep(rand() % 3 + 1);

        pthread_mutex_lock(&oven_lock);
        current_oven_load--;
        pthread_cond_signal(&oven_cond);
        pthread_mutex_unlock(&oven_lock);

        fprintf(log_file, "Cook %d finished order %d\n", cook->id, order.order_id);
        fflush(log_file);

        cook->is_available = 1;

        // Hand over to delivery personnel
    }
    return NULL;
}
