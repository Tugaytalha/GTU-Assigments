#ifndef __MYOS__MEMORYMANAGER_H
#define __MYOS__MEMORYMANAGER_H

namespace myos {
    class MemoryManager {
    public:
        static const uint32_t PROGRAM_SEGMENT_BASE = 0x200000; // Start loading programs at this address
        static const uint32_t MAX_PROGRAM_SIZE = 1024 * 1024; // Maximum program size 1MB
        uint32_t nextProgramAddress; 

        MemoryManager();
        uint32_t AllocateProgramSpace(uint32_t size); 
    };
}
#endif 