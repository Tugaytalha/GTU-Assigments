#include <memorymanagement.h>

namespace myos {
    MemoryManager::MemoryManager() : nextProgramAddress(PROGRAM_SEGMENT_BASE) { }

    common::uint32_t MemoryManager::AllocateProgramSpace(common::uint32_t size) {
        if (nextProgramAddress + size >= PROGRAM_SEGMENT_BASE + MAX_PROGRAM_SIZE) {
            // Out of memory
            return 0; 
        }

        common::uint32_t allocatedAddress = nextProgramAddress;
        nextProgramAddress += size;
        return allocatedAddress; 
    }
}