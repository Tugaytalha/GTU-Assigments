#include "Process.h"
#include "kernel.cpp" // for debugging functions

// #define for debugging
// #define DEBUG_PROCESS

#ifdef DEBUG_PROCESS
    #define debug_print(x)  printf(x)
#else
    #define debug_print(x)
#endif

Process::Process(uint32_t base)
{
    this->stackPointer = base + stackSize - 4; 
    this->base = base;
    this->state = 0; // 0 means Ready
    debug_print("Process Created at: ");
    debug_printHex((uint32_t) this);
    debug_print("\n");
}

Process::~Process()
{
    debug_print("Process Destroyed at: ");
    debug_printHex((uint32_t) this);
    debug_print("\n");
}

void Process::setBase(uint32_t newBase)
{
    this->base = newBase;
    this->stackPointer = this->base + stackSize - 4;
}

ProcessTable::ProcessTable(InterruptManager* interruptManager)
{
    for (int i = 0; i < MAX_PROCESSES; i++)
    {
        this->processes[i] = NULL;
    }
    this->processCount = 0;
    this->interruptManager = interruptManager;
    this->interruptManager->setTimerInterruptCallback(&ProcessTable::handleTimerInterrupt, this);
}

uint8_t ProcessTable::findEmptyProcessSlot()
{
    for (int i = 0; i < MAX_PROCESSES; i++)
    {
        if (this->processes[i] == NULL)
        {
            return i;
        }
    }
    return 0xFF;
}

/*
    * addNewProcess - Add a new process to the process table
    * 
    * @param name: The name of the process
    * 
    * @return: A pointer to the newly added process
    *          NULL if there's no space
*/
Process* ProcessTable::addNewProcess(const char* name)
{
    uint8_t index = this->findEmptyProcessSlot();
    if (index == 0xFF)
    {
        return NULL; // No free slots available
    }

    uint32_t base = 0x100000 + index * 0x10000;
    this->processes[index] = new Process(base);
    strcpy(this->processes[index]->name, name);
    this->processCount++;
    return this->processes[index];
}

void ProcessTable::handleTimerInterrupt()
{
    debug_print("Timer Interrupt\n");
    uint8_t currentProcessIndex = this->interruptManager->getCurrentProcessIndex();
    uint8_t nextProcessIndex = (currentProcessIndex + 1) % this->processCount; 

    // Skip finished processes
    while (this->processes[nextProcessIndex]->state == 3) {
        nextProcessIndex = (nextProcessIndex + 1) % this->processCount;
    }

    this->interruptManager->setCurrentProcessIndex(nextProcessIndex);

    Process* nextProcess = this->processes[nextProcessIndex];

    // Output Process Table information
    printf("\n--- Process Table ---\n"); 
    for (int i = 0; i < MAX_PROCESSES; i++)
    {
        if (this->processes[i] != NULL)
        {
            printf("Process: %s (", this->processes[i]->name);
            if (i == this->interruptManager->getCurrentProcessIndex()) {
                printf("Running");
            } else if (this->processes[i]->state == 3) {
                printf("Finished");
            } else if (this->processes[i]->state == 2) {
                printf("Blocked");
            } else {
                printf("Ready");
            }
            printf(")\n");
            printf("    Base: ");
            printfHex(this->processes[i]->base);
            printf(", Stack Pointer: ");
            printfHex(this->processes[i]->stackPointer);
            printf("\n");
        }
    }

    debug_print("Switching to: ");
    debug_print(nextProcess->name);
    debug_print("\n");
    
    // Context switch
    asm volatile("  \
        mov %0, %%esp; \
        "
        :
        : "r" (nextProcess->stackPointer));
}