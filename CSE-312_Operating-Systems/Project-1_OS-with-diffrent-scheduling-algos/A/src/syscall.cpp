#include <syscalls.h>
 
using namespace myos;
using namespace myos::common;
using namespace myos::hardwarecommunication;
 
SyscallHandler::SyscallHandler(InterruptManager* interruptManager, uint8_t InterruptNumber)
:    InterruptHandler(interruptManager, InterruptNumber  + interruptManager->HardwareInterruptOffset())
{
}

SyscallHandler::~SyscallHandler()
{
}


uint32_t myos::getpid(){
    int result = 0;
    asm("int $0x80" : "=c" (result) : "a" (4));
    return result;
}

void myos::fork(int* pid){
    asm("int $0x80" : "=c" (*pid) : "a" (0));
}

void myos::exit(){
    asm("int $0x80" : : "a" (3));
}

uint32_t myos::execve(void entrypoint()){
    uint32_t result;
    asm("int $0x80" : "=c" (result) : "a" (1), "b" ( (uint32_t) entrypoint));
    return result;
}

void myos::waitpid(common::uint32_t pid_t){
    asm("int $0x80" : : "a" (2), "b" (pid_t));
}

uint32_t SyscallHandler::HandleInterrupt(uint32_t esp)
{
    CPUState* cpu = (CPUState*)esp;
    

    switch(cpu->eax)
    {

        case 0:    // fork
            cpu->ecx = InterruptHandler::sys_fork(cpu);
            return InterruptHandler::HandleInterrupt(esp);
            break;

        case 2: // waitpid
            if( InterruptHandler::sys_waitpid(esp) ){
                return InterruptHandler::HandleInterrupt(esp);
            }
            break;

        case 1: // execve
            esp = InterruptHandler::sys_execve(cpu->ebx);
            break;
            
        case 3: // exit
            if(InterruptHandler::sys_exit())
                return InterruptHandler::HandleInterrupt(esp);
            break;

        case 4:    // getPID
            cpu->ecx = InterruptHandler::sys_getpid();
            break;

        default:
            break;
    }

    return esp;
}
