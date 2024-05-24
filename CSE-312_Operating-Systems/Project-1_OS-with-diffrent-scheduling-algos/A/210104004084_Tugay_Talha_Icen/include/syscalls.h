#ifndef _MYOS_SYSCALLS_H
#define _MYOS_SYSCALLS_H

#include <common/types.h>
#include <hardwarecommunication/interrupts.h>
#include <multitasking.h>

namespace myos
{
    enum SyscallNumber {
        SYSCALL_FORK = 0,
        SYSCALL_EXECVE = 1,
        SYSCALL_WAITPID = 2,
        SYSCALL_EXIT = 3,
        SYSCALL_GETPID = 4,
        // other syscalls
    };
    
    class SyscallHandler : public hardwarecommunication::InterruptHandler
    {
        
    public:
        SyscallHandler(hardwarecommunication::InterruptManager* interruptManager, myos::common::uint8_t InterruptNumber);
        ~SyscallHandler();
        
        virtual myos::common::uint32_t HandleInterrupt(myos::common::uint32_t esp);

    };
    
    common::uint32_t getpid();
    void fork(int* pid);
    void exit();
    common::uint32_t execve(void entrypoint());
    void waitpid(common::uint32_t pid_t);
}


#endif