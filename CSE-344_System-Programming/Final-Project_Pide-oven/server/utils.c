#include "pide_shop.h"

OrderQueue order_queue;
OrderQueue prepared_order_queue;
Cook cooks[MAX_COOKS];
DeliveryPerson delivery_personnel[MAX_DELIVERY_PERSONNEL];
Apparatus ovens[OVEN_CAPACITY];
Apparatus cooking_apparatus[APPARATUS_COUNT];
int current_oven_load;
FILE *log_file;
int server_socket;
sem_t oven_openings;
pthread_mutex_t oven_lock;
pthread_cond_t oven_cond;
Manager manager;


void initialize(int delivery_speed) {
    pthread_mutex_init(&order_queue.lock, NULL);
    pthread_cond_init(&order_queue.cond, NULL);
    order_queue.count = 0;

    current_oven_load = 0;

    // Initialize prepared order queue
    pthread_mutex_init(&prepared_order_queue.lock, NULL);
    pthread_cond_init(&prepared_order_queue.cond, NULL);
    prepared_order_queue.count = 0;
    
    for (int i = 0; i < OVEN_CAPACITY; i++) {
        pthread_mutex_init(&ovens[i].lock, NULL);
        pthread_cond_init(&ovens[i].cond, NULL);
        ovens[i].available = 1;
    }

    for (int i = 0; i < APPARATUS_COUNT; i++) {
        pthread_mutex_init(&cooking_apparatus[i].lock, NULL);
        pthread_cond_init(&cooking_apparatus[i].cond, NULL);
        cooking_apparatus[i].available = 1;
    }

    for (int i = 0; i < MAX_COOKS; i++) {
        cooks[i].id = i;
        cooks[i].is_available = 1;
    }

    // Initialize delivery personnel
    for (int i = 0; i < MAX_DELIVERY_PERSONNEL; i++) {
        delivery_personnel[i].id = i;
        delivery_personnel[i].is_available = 1;
        delivery_personnel[i].order_count = 0;
        delivery_personnel[i].max_orders = 3;
        delivery_personnel[i].velocity = delivery_speed;
        pthread_mutex_init(&delivery_personnel[i].lock, NULL);
        pthread_cond_init(&delivery_personnel[i].cond, NULL);
    }

    sem_init(&oven_openings, 0, OVEN_OPENINGS);


    log_file = fopen("pide_shop.log", "w");
    if (log_file == NULL) {
        perror("Failed to open log file");
        exit(EXIT_FAILURE);
    }
}

void handle_signal(int signal) {
    if (signal == SIGINT) {
        //pthread_mutex_destroy(&order_queue.lock);
        //pthread_cond_destroy(&order_queue.cond);

        // pthread_mutex_destroy(&prepared_order_queue.lock);
        // pthread_cond_destroy(&prepared_order_queue.cond);

        // for (int i = 0; i < OVEN_CAPACITY; i++) {
        //     pthread_mutex_destroy(&ovens[i].lock);
        //     pthread_cond_destroy(&ovens[i].cond);
        // }

        // for (int i = 0; i < APPARATUS_COUNT; i++) {
        //     pthread_mutex_destroy(&cooking_apparatus[i].lock);
        //     pthread_cond_destroy(&cooking_apparatus[i].cond);
        // }

        // for (int i = 0; i < MAX_COOKS; i++) {
        //     pthread_cancel(cooks[i].thread);
        // }

        // for (int i = 0; i < MAX_DELIVERY_PERSONNEL; i++) {
        //     pthread_cancel(delivery_personnel[i].thread);
        // }

        // pthread_mutex_destroy(&oven_lock);
        // pthread_cond_destroy(&oven_cond);

        // sem_destroy(&oven_openings);
        stop_server = 1;
        
        fprintf(log_file, "Server shutting down...\n");
        fclose(log_file);
        free(log_file);
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

    // Start manager thread
    pthread_create(&manager.thread, NULL, manager_thread, NULL);
}


// After this point, we have the implementation of the pseudo-inverse calculation functions beacuse it is in assignment instructions and why not :D
void transpose_conjugate(int rows, int cols, double complex input[rows][cols], double complex output[cols][rows]) {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            output[j][i] = conj(input[i][j]);
        }
    }
}

void matrix_multiply(int rows1, int cols1, int cols2, double complex A[rows1][cols1], double complex B[cols1][cols2], double complex result[rows1][cols2]) {
    for (int i = 0; i < rows1; i++) {
        for (int j = 0; j < cols2; j++) {
            result[i][j] = 0;
            for (int k = 0; k < cols1; k++) {
                result[i][j] += A[i][k] * B[k][j];
            }
        }
    }
}

// Function to perform Gauss-Jordan elimination to find the inverse of a matrix
void matrix_inverse(int n, double complex input[n][n], double complex output[n][n]) {
    double complex temp;

    // Initialize the output as an identity matrix
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (i == j)
                output[i][j] = 1.0;
            else
                output[i][j] = 0.0;
        }
    }

    // Perform Gauss-Jordan elimination
    for (int i = 0; i < n; i++) {
        temp = input[i][i];

        for (int j = 0; j < n; j++) {
            input[i][j] /= temp;
            output[i][j] /= temp;
        }

        for (int k = 0; k < n; k++) {
            if (k != i) {
                temp = input[k][i];

                for (int j = 0; j < n; j++) {
                    input[k][j] -= input[i][j] * temp;
                    output[k][j] -= output[i][j] * temp;
                }
            }
        }
    }
}

// Function to calculate the pseudo-inverse of a complex matrix A (30x40)
void pseudo_inverse(double complex A[ROWS][COLS], double complex pinvA[COLS][ROWS]) {
    // Step 1: Calculate A^H (Hermitian transpose of A)
    double complex A_H[COLS][ROWS];
    transpose_conjugate(ROWS, COLS, A, A_H);

    // Step 2: Calculate A^H * A
    double complex A_H_A[COLS][COLS];
    matrix_multiply(COLS, ROWS, COLS, A_H, A, A_H_A);

    // Step 3: Calculate (A^H * A)^(-1)
    double complex A_H_A_inv[COLS][COLS];
    matrix_inverse(COLS, A_H_A, A_H_A_inv);

    // Step 4: Calculate A_H * (A^H * A)^(-1)
    double complex A_H_A_inv_A_H[COLS][ROWS];
    matrix_multiply(COLS, COLS, ROWS, A_H_A_inv, A_H, A_H_A_inv_A_H);

    // Step 5: Resulting pseudo-inverse is stored in pinvA
    for (int i = 0; i < COLS; i++) {
        for (int j = 0; j < ROWS; j++) {
            pinvA[i][j] = A_H_A_inv_A_H[i][j];
        }
    }
}

double calculate_invert_time() {
    double complex A[ROWS][COLS];
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            A[i][j] = rand() / (double)RAND_MAX + (rand() / (double)RAND_MAX) * I;
        }
    }

    double complex pinvA[COLS][ROWS];

    clock_t start = clock();
    pseudo_inverse(A, pinvA);
    clock_t end = clock();

    return ((double)(end - start)) / CLOCKS_PER_SEC;
}

