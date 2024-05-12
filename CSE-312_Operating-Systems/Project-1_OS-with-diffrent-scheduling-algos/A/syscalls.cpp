#include "syscalls.h"

SyscallHandler::SyscallHandler(InterruptManager* interruptManager, MemoryManager* memoryManager)
    : m_interruptManager(interruptManager), m_memoryManager(memoryManager) {
    
    // Register the syscall handler 
    m_interruptManager->RegisterInterruptHandler(0x30, this); 
}

void SyscallHandler::handleSyscall(uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx) {
    
    switch (eax) {
        case 0: // SYS_FORK
            handleFork(eax, ebx, ecx, edx);
            break;
        case 1: // SYS_EXECVE
            handleExecve(eax, ebx, ecx, edx);
            break;
        // Add more cases for other syscalls...

        default:
            // Invalid syscall number
            break;
    }
}

void SyscallHandler::handleFork(uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx) {
    // Implement fork() system call. 
    // ...
}

void SyscallHandler::handleExecve(uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx) {
    // Implement execve() system call. 
    // ... 
}

// ... (Implement other syscall handler functions here) 