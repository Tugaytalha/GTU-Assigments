#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>

#define WAIT_ACTIVE_OWNER  
#define WAIT_ACTIVE_ATTENDANT  
#define WAIT_MILISECONDS_AVG 300

#ifdef WAIT_ACTIVE_OWNER
#define WAIT_OWNER usleep((rand() % WAIT_MILISECONDS_AVG*2000)/3 + (WAIT_MILISECONDS_AVG*2000)/3)
#else
#define WAIT_OWNER
#endif

#ifdef WAIT_ACTIVE_ATTENDANT
#define WAIT_ATTENDANT usleep((rand() % WAIT_MILISECONDS_AVG*2000)/3 + (WAIT_MILISECONDS_AVG*2000)/3)
#else
#define WAIT_ATTENDANT
#endif


// Semaphores for synchronization
sem_t newPickup, inChargeforPickup, newAutomobile, inChargeforAutomobile;
sem_t entrance; // Semaphore to control entrance to the parking system

// Shared variables to track the availability of parking spots
int mFree_automobile = 8;        // Real parking spots for automobiles
int mFree_pickup = 4;            // Real parking spots for pickups
int tempFree_automobile = 8;     // Temporary parking spots for automobiles
int tempFree_pickup = 4;         // Temporary parking spots for pickups
int totalFreeSpots; // Total free spots

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void *carOwner(void *arg) {
    int carType = *(int *)arg;

    // Simulate arrival time
    WAIT_OWNER;
    sem_wait(&entrance); 

    pthread_mutex_lock(&mutex);
    totalFreeSpots--;
    if (carType == 0 && tempFree_automobile > 0) {
        tempFree_automobile--;
        printf("Automobile owner arrived. Remaining temporary automobile spots: %d\n", tempFree_automobile);
        sem_post(&inChargeforAutomobile);
        sem_wait(&newAutomobile);
    } else if (carType == 1 && tempFree_pickup > 0) {
        tempFree_pickup--;
        printf("Pickup owner arrived. Remaining temporary pickup spots: %d\n", tempFree_pickup);
        sem_post(&inChargeforPickup);
        sem_wait(&newPickup);
    } else {
        totalFreeSpots++;
        printf("No temporary spot available for %s. Owner leaves.\n", carType == 0 ? "automobile" : "pickup");
    }
    pthread_mutex_unlock(&mutex); 
    sem_post(&entrance); // Signal entrance AFTER releasing the mutex

    pthread_exit(NULL);
}

void *carAttendantAutomobile(void *arg) {
    while (1) {
        sem_wait(&inChargeforAutomobile);
        printf("Attendant parking an automobile.\n");
        WAIT_ATTENDANT;

        if (mFree_automobile > 0) {
            mFree_automobile--;
            tempFree_automobile++;
            printf("Automobile parked. Remaining real automobile spots: %d, Remaining temporary automobile spots: %d\n", mFree_automobile, tempFree_automobile);
        } else {
            printf("No place to park the automobile. Car attendant is waiting a bit for some automobile to leave.\n");
            usleep(1000000);
            sem_post(&inChargeforAutomobile);
        }
        sem_post(&newAutomobile);

        if (mFree_automobile == 0 && tempFree_automobile == 0) {
            // Print a message to indicate the Car attendant Automobile thread is ending with cyan color
            printf("\033[1;36mAll temp and real parking spots for automobiles are full. Car attendant Automobile thread is ending.\033[0m\n");
            pthread_exit(NULL);
        }
    }
}

void *carAttendantPickup(void *arg) {
    while (1) {
        sem_wait(&inChargeforPickup);
        printf("Attendant parking a pickup.\n");
        WAIT_ATTENDANT;

        if (mFree_pickup > 0) {
            mFree_pickup--;
            tempFree_pickup++;
            printf("Pickup parked. Remaining real pickup spots: %d, Remaining temporary pickup spots: %d\n", mFree_pickup, tempFree_pickup);
        } else {
            printf("No place to park the pickup. Car attendant is waiting some a bit to some pickup leave.\n");
            usleep(1000000);
            sem_post(&inChargeforPickup);
        }
        sem_post(&newPickup);

        if (mFree_pickup == 0 && tempFree_pickup == 0) {
            // Print a message to indicate the Car attendant Pickup thread is ending with cyan color
            printf("\033[1;36mAll temp and real parking spots for pickups are full. Car attendant Pickup thread is ending.\033[0m\n");
            pthread_exit(NULL);
        }
    }
}

int main() {
    // Calculate the total number of free spots
    totalFreeSpots = mFree_automobile + mFree_pickup + tempFree_automobile + tempFree_pickup;
    
    // Create threads for car owners and attendants
    pthread_t ownerThread, attendantAutomobileThread, attendantPickupThread;

    // Initialize semaphores
    sem_init(&newPickup, 0, 4);
    sem_init(&inChargeforPickup, 0, 0);
    sem_init(&newAutomobile, 0, 8);
    sem_init(&inChargeforAutomobile, 0, 0);
    sem_init(&entrance, 0, 1);

    // Seed the random number generator
    srand(time(NULL));

    // Create and start the attendant threads
    pthread_create(&attendantAutomobileThread, NULL, carAttendantAutomobile, NULL);
    pthread_create(&attendantPickupThread, NULL, carAttendantPickup, NULL);

    while (totalFreeSpots > 0) {
        int carType = rand() % 2; // 0 for automobile, 1 for pickup
        pthread_create(&ownerThread, NULL, carOwner, &carType);
        pthread_join(ownerThread, NULL); 
    }

    // Wait for the attendant threads to finish
    pthread_join(attendantAutomobileThread, NULL);
    pthread_join(attendantPickupThread, NULL);
    // Print a message to indicate the parking system is closing with green color, message: All parking spots are full. The parking system is closing.
    printf("\033[1;32mAll parking spots are full. The parking system is closing.\033[0m\n");

    // Destroy semaphores
    sem_destroy(&newPickup);
    sem_destroy(&inChargeforPickup);
    sem_destroy(&newAutomobile);
    sem_destroy(&inChargeforAutomobile);
    sem_destroy(&entrance);
    pthread_mutex_destroy(&mutex);

    return 0;
}