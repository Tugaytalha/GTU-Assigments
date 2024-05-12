#ifndef __SYSCALLS_H
#define __SYSCALLS_H

#include "types.h"
#include "interrupts.h"
#include "memory_manager.h"


class SyscallHandler {
public:
    SyscallHandler(InterruptManager* interruptManager, MemoryManager* memoryManager);
    void handleSyscall(uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx);

private:
    InterruptManager* m_interruptManager;
    MemoryManager* m_memoryManager;

    void handleFork(uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx);
    void handleExecve(uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx);
    // Add more syscall handlers as needed... 
};

#endif 