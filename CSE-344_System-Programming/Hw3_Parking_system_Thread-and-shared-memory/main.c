#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define WAIT_ACTIVE  // Comment this line to disable the wait time
#define WAIT_MILISECONDS_AVG 1000

#ifdef WAIT_ACTIVE
#define WAIT usleep(rand() % WAIT_MILISECONDS_AVG*1000000 + (WAIT_MILISECONDS_AVG*1000000)/2)
#else
#define WAIT
#endif

// Semaphores for synchronization
sem_t newPickup, inChargeforPickup, newAutomobile, inChargeforAutomobile;

// Shared variables to track the availability of parking spots
int mFree_automobile = 8;        // Real parking spots for automobiles
int mFree_pickup = 4;            // Real parking spots for pickups
int tempFree_automobile = 8;     // Temporary parking spots for automobiles
int tempFree_pickup = 4;         // Temporary parking spots for pickups
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void *carOwnerAutomobile(void *arg) {
    while (1) {
        // Simulate arrival time
        WAIT;

        pthread_mutex_lock(&mutex);
        if (tempFree_automobile > 0) {
            tempFree_automobile--;
            printf("Automobile owner arrived. Remaining temporary automobile spots: %d\n", tempFree_automobile);
            pthread_mutex_unlock(&mutex);
            // Signal the semaphore to indicate the attendant can park the automobile
            sem_post(&inChargeforAutomobile);
            // Wait for the semaphore to indicate a new automobile spot is available
            sem_wait(&newAutomobile);
        } else {
            pthread_mutex_unlock(&mutex);
            printf("No temporary automobile spot available. Automobile owner leaves.\n");
        }
    }
    pthread_exit(NULL);
}

void *carOwnerPickup(void *arg) {
    while (1) {
        // Simulate arrival time
        WAIT;

        pthread_mutex_lock(&mutex);
        if (tempFree_pickup > 0) {
            tempFree_pickup--;
            printf("Pickup owner arrived. Remaining temporary pickup spots: %d\n", tempFree_pickup);
            pthread_mutex_unlock(&mutex);
            // Signal the semaphore to indicate the attendant can park the pickup
            sem_post(&inChargeforPickup);
            // Wait for the semaphore to indicate a new pickup spot is available
            sem_wait(&newPickup);
        } else {
            pthread_mutex_unlock(&mutex);
            printf("No temporary pickup spot available. Pickup owner leaves.\n");
        }
    }
    pthread_exit(NULL);
}

void *carAttendantAutomobile(void *arg) {
    while (1) {
        // Wait for the semaphore to indicate an automobile is ready to be parked by the attendant
        sem_wait(&inChargeforAutomobile);
        printf("Attendant parking an automobile.\n");
        // Simulate the time taken to park the vehicle
        WAIT;

        pthread_mutex_lock(&mutex);
        if (mFree_automobile > 0) {
            mFree_automobile--;
            printf("Automobile parked. Remaining real automobile spots: %d\n", mFree_automobile);
        } else {
            tempFree_automobile++;
            printf("Automobile moved to temporary spot. Remaining temporary automobile spots: %d\n", tempFree_automobile);
        }
        pthread_mutex_unlock(&mutex);

        // Signal the semaphore to indicate the new automobile spot is available
        sem_post(&newAutomobile);
    }
    pthread_exit(NULL);
}

void *carAttendantPickup(void *arg) {
    while (1) {
        // Wait for the semaphore to indicate a pickup is ready to be parked by the attendant
        sem_wait(&inChargeforPickup);
        printf("Attendant parking a pickup.\n");
        // Simulate the time taken to park the vehicle
        WAIT;

        pthread_mutex_lock(&mutex);
        if (mFree_pickup > 0) {
            mFree_pickup--;
            printf("Pickup parked. Remaining real pickup spots: %d\n", mFree_pickup);
        } else {
            tempFree_pickup++;
            printf("Pickup moved to temporary spot. Remaining temporary pickup spots: %d\n", tempFree_pickup);
        }
        pthread_mutex_unlock(&mutex);

        // Signal the semaphore to indicate the new pickup spot is available
        sem_post(&newPickup);
    }
    pthread_exit(NULL);
}

int main() {
    // Create threads for car owners and attendants
    pthread_t ownerAutomobileThread, ownerPickupThread, attendantAutomobileThread, attendantPickupThread;

    // Initialize semaphores
    sem_init(&newPickup, 0, 4);
    sem_init(&inChargeforPickup, 0, 0);
    sem_init(&newAutomobile, 0, 8);
    sem_init(&inChargeforAutomobile, 0, 0);

    // Seed the random number generator
    srand(time(NULL));

    // Create and start the owner and attendant threads
    pthread_create(&ownerAutomobileThread, NULL, carOwnerAutomobile, NULL);
    pthread_create(&ownerPickupThread, NULL, carOwnerPickup, NULL);
    pthread_create(&attendantAutomobileThread, NULL, carAttendantAutomobile, NULL);
    pthread_create(&attendantPickupThread, NULL, carAttendantPickup, NULL);

    // Wait for all threads to finish (in this simulation, they will run indefinitely)
    pthread_join(ownerAutomobileThread, NULL);
    pthread_join(ownerPickupThread, NULL);
    pthread_join(attendantAutomobileThread, NULL);
    pthread_join(attendantPickupThread, NULL);

    // Destroy semaphores
    sem_destroy(&newPickup);
    sem_destroy(&inChargeforPickup);
    sem_destroy(&newAutomobile);
    sem_destroy(&inChargeforAutomobile);
    pthread_mutex_destroy(&mutex);

    return 0;
}
