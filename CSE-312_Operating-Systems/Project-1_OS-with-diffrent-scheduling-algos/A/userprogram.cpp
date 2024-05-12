#include "types.h" 

// Very basic user-space 'printf' for demo. 
// In a real OS, this would involve a system call to the kernel.
void print(char* str) {
    static uint16_t* VideoMemory = (uint16_t*)0xb8000;
    for(int i = 0; str[i] != '\0'; ++i) {
        VideoMemory[i] = (VideoMemory[i] & 0xFF00) | str[i];
    }
}

void user_program() {
    print("Hello from user program!\n");
    while (1); // Prevent the user program from exiting
}

extern "C" void _start() {
    user_program();
}