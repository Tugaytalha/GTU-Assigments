#include <syscalls.h>
#include <common/types.h>

using namespace myos;
using namespace myos::common;

void printCollatz(uint32_t n) {
    printf("%d: ", n);

    while (n != 1) {
        printf("%d, ", n);
        if (n % 2 == 0) {
            n /= 2;
        } else {
            n = 3 * n + 1;
        }
    }
    printf("1\n");
}

extern "C" void _start() { 
    for (uint32_t i = 1; i <= 10; ++i) { // Calculate Collatz for numbers 1 to 10 
        printCollatz(i); 
    }
    exit(); // Terminate the process
}