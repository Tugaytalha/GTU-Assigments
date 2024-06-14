#ifndef PIDE_SHOP_H
#define PIDE_SHOP_H

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <time.h>

#define MAX_COOKS 10
#define MAX_DELIVERY_PERSONNEL 10
#define OVEN_CAPACITY 6
#define APPARATUS_COUNT 3
#define MAX_ORDER_QUEUE 100

typedef struct {
    int order_id;
    int customer_id;
    int x, y;
    time_t order_time;
    time_t prep_start_time;
    time_t oven_start_time;
    time_t delivery_start_time;
} Order;

typedef struct {
    pthread_t thread;
    int id;
    int is_available;
} Cook;

typedef struct {
    pthread_t thread;
    int id;
    int is_available;
    int deliveries_made;
} DeliveryPerson;

typedef struct {
    pthread_mutex_t lock;
    pthread_cond_t cond;
    Order orders[MAX_ORDER_QUEUE];
    int count;
} OrderQueue;

extern OrderQueue order_queue;
extern Cook cooks[MAX_COOKS];
extern DeliveryPerson delivery_personnel[MAX_DELIVERY_PERSONNEL];
extern pthread_mutex_t oven_lock;
extern pthread_cond_t oven_cond;
extern int current_oven_load;
extern FILE *log_file;
extern int server_socket;

void initialize();
void setup_signal_handling();
void setup_server(int port);
void start_threads(int cook_count, int delivery_count);
void accept_connections();
void* cook_thread(void* arg);
void* delivery_thread(void* arg);
void* client_handler(void* arg);

#endif
