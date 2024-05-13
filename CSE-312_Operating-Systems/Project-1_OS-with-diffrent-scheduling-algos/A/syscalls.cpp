#include "syscalls.h"
#include "Process.h" // Include the Process class definition
#include "kernel.cpp" // For debugging 

// #define for debugging
// #define DEBUG_SYSCALLS
#ifdef DEBUG_SYSCALLS
    #define debug_print(x)  printf(x)
#else
    #define debug_print(x)
#endif

// Global Process Table object
extern ProcessTable processTable;

// Constructor for SyscallHandler
SyscallHandler::SyscallHandler(InterruptManager* interruptManager, MemoryManager* memoryManager)
    : m_interruptManager(interruptManager), m_memoryManager(memoryManager) {
    
    // Register the syscall handler 
    m_interruptManager->RegisterInterruptHandler(0x30, this); 
}

/*
    * handleSyscall - Handle the system call based on the value in EAX
    * 
    * @param eax: The value in the extended accumulator register to determine the syscall
    * @param ebx: The value in the extended base register
    * @param ecx: The value in the extended counter register
    * @param edx: The value in the extended data register
    
*/
void SyscallHandler::handleSyscall(uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx) {

    debug_print("Syscall handler triggered! EAX: ");
    printfHex(eax);
    debug_print("\n");

    // Handle the syscall based on the value in EAX
    switch (eax) {
        case 0: // SYS_FORK
            handleFork(eax, ebx, ecx, edx);
            break;
        case 1: // SYS_EXECVE
            handleExecve(eax, ebx, ecx, edx);
            break;
        // Add more cases for other syscalls...

        default:
            debug_print("Invalid Syscall\n");
            break;
    }
}

void SyscallHandler::handleFork(uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx) {
    debug_print("Handle Fork\n");

    // 1. Create a new process entry in the process table.
    Process* newProcess = processTable.addNewProcess("child");
    if (newProcess == NULL) {
        // Handle error (out of memory, process table full, etc.)
        debug_print("Error: Fork failed\n"); 
        return;
    }

    // 2. Copy the parent process's memory into the child process.
    //    (This is a simplified example, you'd need to handle page tables, etc.)
    for (uint32_t i = 0; i < 4096; ++i) { // Assuming 4KB process memory for now
        newProcess->memory[i] = processTable.getCurrentProcess()->memory[i]; 
    }

    // 3. Set up the child process's registers (EIP, ESP, etc.)
    //    - EIP should point to the same instruction the parent was about to execute. 
    newProcess->eip = processTable.getCurrentProcess()->eip;
    
    //    - ESP should point to the child's new stack (make sure there's enough space).
    newProcess->stackPointer = newProcess->stack; // Assuming 'stack' is the top of the child's stack
    newProcess->stackPointer -= 4; // Allocate space on the stack for the return value

    // 4. Set the return value of fork:
    //    - Parent process gets the child's PID.
    //    - Child process gets 0.
    processTable.getCurrentProcess()->eax = newProcess->pid; 
    newProcess->eax = 0; // Child's return value

    debug_print("Fork Successful\n");
}

void SyscallHandler::handleExecve(uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx) {
    // Implement execve() system call. 
    // 1. Get arguments:
    //    - ebx: path to the executable
    //    - ecx: argv (array of arguments)
    //    - edx: envp (array of environment variables)

    // 2. Load the new executable into memory (you'll need to handle ELF loading here). 
    //    - Overwrite the current process's memory.

    // 3. Set up the process's registers to start executing the new program.

    // 4. There's no return from execve if successful. The process now runs the new executable. 
}

// ... (Implement other syscall handler functions here) 