#include "pide_shop.h"

void* manager_thread(void* arg) {
    while (1) {
        pthread_mutex_lock(&prepared_order_queue.lock);

        while (prepared_order_queue.count == 0) {
            pthread_cond_wait(&prepared_order_queue.cond, &prepared_order_queue.lock);
        }

        Order order = prepared_order_queue.orders[0];
        for (int i = 0; i < prepared_order_queue.count - 1; i++) {
            prepared_order_queue.orders[i] = prepared_order_queue.orders[i + 1];
        }
        prepared_order_queue.count--;

        pthread_mutex_unlock(&prepared_order_queue.lock);

        int order_assigned = 0;
        while (!order_assigned) {
            for (int i = 0; i < MAX_DELIVERY_PERSONNEL; i++) {
                pthread_mutex_lock(&delivery_personnel[i].lock);
                if (delivery_personnel[i].is_available && delivery_personnel[i].order_count < delivery_personnel[i].max_orders) {
                    delivery_personnel[i].orders[delivery_personnel[i].order_count++] = order;
                    if (delivery_personnel[i].order_count == delivery_personnel[i].max_orders || prepared_order_queue.count == 0) {
                        delivery_personnel[i].is_available = 0;
                        pthread_cond_signal(&delivery_personnel[i].cond);
                    }
                    pthread_mutex_unlock(&delivery_personnel[i].lock);
                    order_assigned = 1;
                    fprintf(log_file, "Manager assigned order %d to delivery person %d\n", order.order_id, delivery_personnel[i].id);
                    fflush(log_file);
                    break;
                }
                pthread_mutex_unlock(&delivery_personnel[i].lock);
            }
            if (!order_assigned) {
                usleep(100000);  // Wait 100ms before trying again
            }
        }
    }
    return NULL;
}
