#ifndef __PROCESS_H
#define __PROCESS_H

#include "types.h"
#include <string.h>

class Process 
{
public:
    // Each process will have a 4KB stack
    static const uint32_t stackSize = 4096;
    uint32_t stackPointer;
    uint32_t base; 
    char name[32]; // For process identification
    bool isFinished;

    Process(uint32_t base);
    ~Process(); 
    
    void setBase(uint32_t newBase); 
};

class ProcessTable
{
    Process* processes[16];
    uint8_t processCount;
    InterruptManager* interruptManager;

    uint8_t findEmptyProcessSlot();
public:
    ProcessTable(InterruptManager* interruptManager);

    // Returns a pointer to the newly added process
    // Returns NULL if there's no space
    Process* addNewProcess(const char* name);

    void handleTimerInterrupt();
};

#endif