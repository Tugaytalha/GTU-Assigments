#ifndef __MEMORY_MANAGER_H
#define __MEMORY_MANAGER_H

#include "types.h"

class MemoryManager {
public:
    MemoryManager(const void* multiboot_structure);
    uint32_t allocateBlock(); 
    void freeBlock(uint32_t addr);

private:
    uint32_t m_memoryStart; 
    uint32_t m_memoryEnd; 
    uint32_t m_nextFreeBlock; 
};

#endif