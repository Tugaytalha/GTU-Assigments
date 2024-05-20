#include <multitasking.h>
#include <memorymanager.h>

using namespace myos;
using namespace myos::common;

// Helper function to copy a CPUState
static void copyCPUState(CPUState* dest, const CPUState* src) {
    dest->eax = src->eax;
    dest->ebx = src->ebx;
    dest->ecx = src->ecx;
    dest->edx = src->edx;
    dest->esi = src->esi;
    dest->edi = src->edi;
    dest->ebp = src->ebp;
    dest->error = src->error;
    dest->eip = src->eip;
    dest->cs = src->cs;
    dest->eflags = src->eflags;
    dest->esp = src->esp;
    dest->ss = src->ss;
}

Task::Task(GlobalDescriptorTable *gdt, void entrypoint())
{
    cpustate = (CPUState*)(stack + 4096 - sizeof(CPUState));
    
    cpustate -> eax = 0;
    cpustate -> ebx = 0;
    cpustate -> ecx = 0;
    cpustate -> edx = 0;

    cpustate -> esi = 0;
    cpustate -> edi = 0;
    cpustate -> ebp = 0;
    
    /*
    cpustate -> gs = 0;
    cpustate -> fs = 0;
    cpustate -> es = 0;
    cpustate -> ds = 0;
    */
    
    // cpustate -> error = 0;    
   
    // cpustate -> esp = ;
    cpustate -> eip = (uint32_t)entrypoint;
    cpustate -> cs = gdt->CodeSegmentSelector();
    // cpustate -> ss = ;
    cpustate -> eflags = 0x202;

    state = READY;
    waitppid = -1; // Initialize to -1, meaning no parent to wait for 
}

Task::~Task()
{
}

int TaskManager::Fork(CPUState* cpustate) {
    // 0. Check if task limit has been reached
    if(numTasks >= 256)
        return -1;

    // 1. Create a new Task for the child process.
    Task* childTask = new Task(cpustate->cs, (void(*)())cpustate->eip);

    // 2. Copy the parent's CPU state to the child's CPU state.
    copyCPUState(childTask->cpustate, cpustate);

    // 3. Adjust the child's esp to point to its own stack.
    childTask->cpustate->esp -= 4096; // Assuming 4KB stacks

    // 4. Assign a new PID to the child process.
    int childPID = AssignNextPID(); 
    if (childPID < 0) {
        return -1; // No more PIDs available
    }
    childTask->pid = childPID;

    // 4,5. Set the child's waitppid to the parent's PID.
    childTask->waitppid = tasks[currentTask]->pid; 

    // 5. Add the child task to the TaskManager.
    AddTask(childTask);

    // 6. Return the child PID in the parent process and 0 in the child process.
    if (currentTask >= 0) {
        return childPID; // Parent process
    } else {
        return 0;        // Child process (this will be running in the child context)
    }
}
        

bool TaskManager::Execve(const char* programName, CPUState* cpustate) {
    // 1. Find the program in memory.
    uint32_t programAddress = findProgramAddress(programName);
    if (programAddress == 0) {
        return false;  // Program not found
    }

    // 2. Allocate memory for the program.
    MemoryManager mm;
    uint32_t allocatedAddress = mm.AllocateProgramSpace(1024 * 1024); // Allocate 1MB for the program (you'll need to determine the actual size)

    if (allocatedAddress == 0) {
        return false; // Memory allocation failed
    }

    // 3. Copy the program to the allocated memory.
    memcpy((void*)allocatedAddress, (void*)programAddress, 1024 * 1024); // (Again, replace 1MB with the actual size)

    // 4. Update the current task's CPU state:
    cpustate->eip = allocatedAddress;   // Set the entry point 
    cpustate->esp = allocatedAddress + 1024 * 1024;  // Adjust stack pointer 
    cpustate->ebp = allocatedAddress + 1024 * 1024;  // Adjust base pointer
    

    return true; 
}


int TaskManager::Waitpid(int pid, int* status, CPUState* cpustate) {
    // 1. Check if waiting for any child (pid == -1) or a specific child.
    bool waitForAnyChild = (pid == -1);

    // 2. Find the child process to wait for.
    int childIndex = -1;
    for (int i = 0; i < numTasks; i++) {
        if ((waitForAnyChild && tasks[i]->waitppid == tasks[currentTask]->pid && tasks[i]->state == Task::TERMINATED) || 
            (!waitForAnyChild && tasks[i]->pid == pid && tasks[i]->state == Task::TERMINATED)) {
            childIndex = i;
            break;
        }
    }

    if (childIndex == -1) {
        // No terminated child found to wait for.
        if (waitForAnyChild) {
            // Block the parent process to wait for any child to exit.
            tasks[currentTask]->state = Task::BLOCKED;
            tasks[cuurentTask]->waitppid = pid; // Set the waitppid to the specific child PID
            Schedule(cpustate);
            return 0; // Indicate success (parent will be unblocked later)
        } else {
            return -1; // Specific child process not found
        }
    }

    // 3. Child process found, retrieve exit status.
    Task* childTask = tasks[childIndex];
    if (status != nullptr) {
        *status = childTask->cpustate->eax; // Assuming exit status in eax
    }

    // 4. Reset waitppid for the terminated child 
    childTask->waitppid = -1;  

    return childTask->pid; // Return the PID of the terminated child 
}

TaskManager::TaskManager() {
    numTasks = 0;
    currentTask = -1;
    nextPID = 1; // Start PIDs from 1
}

TaskManager::~TaskManager()
{
}

bool TaskManager::AddTask(Task* task)
{
    if(numTasks >= 256)
        return false;
    tasks[numTasks++] = task;
    return true;
}

// src/multitasking.cpp

CPUState* TaskManager::Schedule(CPUState* cpustate) {
    if (numTasks <= 0) return cpustate;

    if (currentTask >= 0) {
        tasks[currentTask]->cpustate = cpustate;
    }

    // Find the next READY task or unblock a waiting parent
    int nextTaskIndex = (currentTask + 1) % numTasks;
    while (tasks[nextTaskIndex]->state != Task::READY) {
        // ... (Check for blocked parents - same as before) ...

        nextTaskIndex = (nextTaskIndex + 1) % numTasks;

        // ... (Prevent infinite loop - same as before) ... 
    }

    currentTask = nextTaskIndex; 

    // Print Process Table
    printf("----------------- Process Table -----------------\n");
    printf("PID\tState\t\tWait PID\n"); 
    for (int i = 0; i < numTasks; ++i) {
        printf("%d\t", tasks[i]->pid);
        switch (tasks[i]->state) {
            case Task::RUNNING: printf("RUNNING  \t"); break;
            case Task::READY:   printf("READY    \t"); break;
            case Task::BLOCKED: printf("BLOCKED  \t"); break;
            case Task::TERMINATED: printf("TERMINATED\t"); break;
        }
        printf("%d\n", tasks[i]->waitppid);
    }
    printf("-------------------------------------------------\n");

    return tasks[currentTask]->cpustate; 
}

int TaskManager::AssignNextPID() {
    if (nextPID < 256) {
        return nextPID++;
    } else {
        return -1; // No more PIDs available
    }
}


    