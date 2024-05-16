#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

// Semaphores for synchronization
sem_t newPickup, inChargeforPickup, newAutomobile, inChargeforAutomobile;

// Shared variables to track the availability of parking spots
int mFree_automobile = 8;
int mFree_pickup = 4;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

// Function for the carOwner thread
void *carOwner(void *arg) {
    int vehicleType = *((int *)arg);

    // Check if the vehicle is a pickup or an automobile
    if (vehicleType % 2 == 0) { // Pickup
        pthread_mutex_lock(&mutex);
        if (mFree_pickup > 0) {
            mFree_pickup--;
            printf("Pickup owner arrived. Remaining pickup spots: %d\n", mFree_pickup);
            pthread_mutex_unlock(&mutex);
            // Signal the semaphore to indicate the attendant can park the pickup
            sem_post(&inChargeforPickup);
            // Wait for the semaphore to indicate a new pickup spot is available
            sem_wait(&newPickup);
        } else {
            pthread_mutex_unlock(&mutex);
            printf("No pickup spot available. Pickup owner leaves.\n");
        }
    } else { // Automobile
        pthread_mutex_lock(&mutex);
        if (mFree_automobile > 0) {
            mFree_automobile--;
            printf("Automobile owner arrived. Remaining automobile spots: %d\n", mFree_automobile);
            pthread_mutex_unlock(&mutex);
            // Signal the semaphore to indicate the attendant can park the automobile
            sem_post(&inChargeforAutomobile);
            // Wait for the semaphore to indicate a new automobile spot is available
            sem_wait(&newAutomobile);
        } else {
            pthread_mutex_unlock(&mutex);
            printf("No automobile spot available. Automobile owner leaves.\n");
        }
    }
    pthread_exit(NULL);
}

// Function for the carAttendant thread
void *carAttendant(void *arg) {
    int vehicleType = *((int *)arg);

    // Check if the vehicle is a pickup or an automobile
    if (vehicleType % 2 == 0) { // Pickup
        // Wait for the semaphore to indicate a pickup is ready to be parked by the attendant
        sem_wait(&inChargeforPickup);
        printf("Attendant parking a pickup.\n");
        sleep(1); // Simulate the time taken to park the vehicle
        // Signal the semaphore to indicate the new pickup spot is available
        sem_post(&newPickup);
        pthread_mutex_lock(&mutex);
        mFree_pickup++;
        pthread_mutex_unlock(&mutex);
    } else { // Automobile
        // Wait for the semaphore to indicate an automobile is ready to be parked by the attendant
        sem_wait(&inChargeforAutomobile);
        printf("Attendant parking an automobile.\n");
        sleep(1); // Simulate the time taken to park the vehicle
        // Signal the semaphore to indicate the new automobile spot is available
        sem_post(&newAutomobile);
        pthread_mutex_lock(&mutex);
        mFree_automobile++;
        pthread_mutex_unlock(&mutex);
    }
    pthread_exit(NULL);
}

int main() {
    // Create threads for car owners and attendants
    pthread_t ownerThread[12], attendantThread[12];
    int vehicleTypes[12];

    // Initialize semaphores
    sem_init(&newPickup, 0, 4);
    sem_init(&inChargeforPickup, 0, 0);
    sem_init(&newAutomobile, 0, 8);
    sem_init(&inChargeforAutomobile, 0, 0);

    // Seed the random number generator
    srand(time(NULL));

    // Create and start the owner and attendant threads
    for (int i = 0; i < 12; i++) {
        vehicleTypes[i] = rand();
        pthread_create(&ownerThread[i], NULL, carOwner, (void *)&vehicleTypes[i]);
        pthread_create(&attendantThread[i], NULL, carAttendant, (void *)&vehicleTypes[i]);
    }

    // Wait for all threads to finish
    for (int i = 0; i < 12; i++) {
        pthread_join(ownerThread[i], NULL);
        pthread_join(attendantThread[i], NULL);
    }

    // Destroy semaphores
    sem_destroy(&newPickup);
    sem_destroy(&inChargeforPickup);
    sem_destroy(&newAutomobile);
    sem_destroy(&inChargeforAutomobile);
    pthread_mutex_destroy(&mutex);

    return 0;
}
