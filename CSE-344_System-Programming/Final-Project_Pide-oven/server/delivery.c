#include "pide_shop.h"
#include <math.h>

void* delivery_thread(void* arg) {
    DeliveryPerson* delivery_person = (DeliveryPerson*)arg;
    while (1) {
        pthread_mutex_lock(&delivery_person->lock);

        while (delivery_person->order_count == 0) {
            pthread_cond_wait(&delivery_person->cond, &delivery_person->lock);
        }

        Order orders[3];
        int order_count = delivery_person->order_count;
        for (int i = 0; i < order_count; i++) {
            orders[i] = delivery_person->orders[i];
        }
        delivery_person->order_count = 0;
        delivery_person->is_available = 1;

        pthread_mutex_unlock(&delivery_person->lock);

        for (int i = 0; i < order_count; i++) {
            fprintf(log_file, "Delivery person %d started delivering order %d\n", delivery_person->id, orders[i].order_id);
            fflush(log_file);

            // Simulate delivery time based on distance and velocity
            float distance = sqrt(orders[i].x * orders[i].x + orders[i].y * orders[i].y);
            sleep(distance / delivery_person->velocity);

            fprintf(log_file, "Delivery person %d delivered order %d\n", delivery_person->id, orders[i].order_id);
            fflush(log_file);

            delivery_person->deliveries_made++;
        }
    }
    return NULL;
}
