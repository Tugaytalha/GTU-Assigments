#include "types.h"
#include "GDT.h"
#include "interrupts.h"
#include "keyboard.h"
#include "Process.h"
#include "memory_manager.h" 
#include "syscalls.h"


// #define for debugging
// #define DEBUG_KERNEL

#ifdef DEBUG_KERNEL
    #define debug_print(x)  printf(x)
#else
    #define debug_print(x)
#endif




void printf(char* str)
{
    static uint16_t* VideoMemory = (uint16_t*)0xb8000;

    for(int i = 0; str[i] != '\0'; ++i)
        VideoMemory[i] = (VideoMemory[i] & 0xFF00) | str[i];
}

void printfHex(uint32_t num)
{
    char* str = "0x00000000";
    for(int i = 9; i >= 2; i--)
    {
        uint32_t temp = num % 16;
        if(temp >= 10)
            str[i] = temp - 10 + 'A';
        else
            str[i] = temp + '0';
        num /= 16;
    }
    printf(str);
}

typedef void (*constructor)();
extern "C" constructor start_ctors;
extern "C" constructor end_ctors;
extern "C" void callConstructors()
{
    for(constructor* i = &start_ctors; i != &end_ctors; i++)
        (*i)();
}

extern "C" void kernelMain(const void* multiboot_structure, uint32_t /*multiboot_magic*/)
{
    printf("Initializing...\n");
    GDT gdt;
    InterruptManager interrupts(&gdt);
    KeyboardDriver keyboard(&interrupts);
    MemoryManager memoryManager(multiboot_structure);
    SyscallHandler syscalls(&interrupts, &memoryManager);
    
    #ifdef DEBUG_KERNEL
    printf("Kernel Stack is: ");
    printfHex((uint32_t) &kernel_stack);
    printf("\n");
    printf("Kernel End is: ");
    printfHex((uint32_t) &kernelMain);
    printf("\n");
    #endif 

    printf("Preparing Process Table\n");
    ProcessTable processTable(&interrupts);

    // Add the init process
    printf("Adding init process\n");
    Process* initProcess = processTable.addNewProcess("init"); 
    if (initProcess == NULL) {
        printf("Error creating init process\n");
        while(1); 
    }

    printf("Enabling Interrupts\n");
    interrupts.Activate();


    printf("Jumping to user mode\n");
    asm volatile("  \
        mov %0, %%edx; \
        mov %1, %%ecx; \
        int $0x30;   \
        "
        :
        : "r" (initProcess->stackPointer), "r" (initProcess->base));
    // Should never reach here
    while(1); 
}