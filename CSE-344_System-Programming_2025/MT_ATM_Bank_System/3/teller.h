#ifndef TELLER_H
#define TELLER_H
#include <sys/types.h>

/* launches a process that executes *func(arg) and exits */
pid_t Teller(void *(*func)(void *), void *arg);
/* thin waitpid() wrapper so instructor can intercept */
int   waitTeller(pid_t pid, int *status);

#endif
