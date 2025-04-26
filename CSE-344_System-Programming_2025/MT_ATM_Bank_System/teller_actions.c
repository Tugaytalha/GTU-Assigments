/* teller_actions.c */
#include "teller.h"
#include <stdio.h>

/* identical bodies – the *op* field in tx distinguishes them           */
static void run(txn_t *tx)
{
    shared_t *shm = teller_shared();
    shm->txn = *tx;                     /* copy request into the page    */
    sem_post(&shm->sem_client);         /* notify Server                 */
    sem_wait(&shm->sem_server);         /* wait for verdict              */

    if (shm->result)
        fprintf(stderr, "Teller %d: operation OK\n", getpid());
    else
        fprintf(stderr, "Teller %d: operation refused\n", getpid());
}

void *deposit (void *arg) { run((txn_t *)arg); return NULL; }
void *withdraw(void *arg) { run((txn_t *)arg); return NULL; }
