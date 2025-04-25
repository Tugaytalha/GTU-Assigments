#ifndef TELLER_H
#define TELLER_H
#include "common.h"

/* create a Teller process that runs *func(arg)* only                  */
pid_t Teller(void *(*func)(void *), void *arg);

/* blocking reap (simple wrapper around waitpid)                       */
int   waitTeller(pid_t pid, int *status);

/* gives access to the shared page that Teller() just allocated         */
shared_t *teller_shared(void);

void *deposit(void *arg);
void *withdraw(void *arg);
#endif
