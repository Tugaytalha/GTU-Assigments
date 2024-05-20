#include <syscalls.h>
#include <common/types.h>

using namespace myos;
using namespace myos::common;

extern "C" void _start() {
    uint32_t result = 0;
    for (int i = 0; i < 1000; ++i) {
        for (int j = 0; j < 1000; ++j) {
            result += i * j;
        }
    }
    printf("Long running program result: %d\n", result); 
    exit(); 
}