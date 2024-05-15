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

// Function for the carOwner thread
void *carOwner(void *arg) {
    int vehicleType = *((int *)arg);

    // Check if the vehicle is a pickup or an automobile
    if (vehicleType % 2 == 0) { // Pickup
        if (mFree_pickup > 0) {
            // Wait for the semaphore to indicate a new pickup is ready to be parked
            sem_wait(&newPickup);
            mFree_pickup--;
            printf("Pickup owner parked. Remaining pickup spots: %d\n", mFree_pickup);
            // Signal the semaphore to indicate the attendant can park the pickup
            sem_post(&inChargeforPickup);
        } else {
            printf("No pickup spot available. Pickup owner leaves.\n");
        }
    } else { // Automobile
        if (mFree_automobile > 0) {
            // Wait for the semaphore to indicate a new automobile is ready to be parked
            sem_wait(&newAutomobile);
            mFree_automobile--;
            printf("Automobile owner parked. Remaining automobile spots: %d\n", mFree_automobile);
            // Signal the semaphore to indicate the attendant can park the automobile
            sem_post(&inChargeforAutomobile);
        } else {
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
        // Signal the semaphore to indicate the new pickup spot is available
        sem_post(&newPickup);
    } else { // Automobile
        // Wait for the semaphore to indicate an automobile is ready to be parked by the attendant
        sem_wait(&inChargeforAutomobile);
        printf("Attendant parking an automobile.\n");
        // Signal the semaphore to indicate the new automobile spot is available
        sem_post(&newAutomobile);
    }
    pthread_exit(NULL);
}

int main() {
    // Create threads for car owners and attendants
    pthread_t ownerThread[12], attendantThread[12];
    int vehicleTypes[12];

    // Initialize semaphores
    sem_init(&newPickup, 0, 1);
    sem_init(&inChargeforPickup, 0, 0);
    sem_init(&newAutomobile, 0, 1);
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

    return 0;
}
