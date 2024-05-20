#include "memory_manager.h"
#include "types.h"

// #define DEBUG_MEMORY_MANAGER
#ifdef DEBUG_MEMORY_MANAGER
    #include "kernel.cpp"
    #define debug_print(x)  printf(x)
#else
    #define debug_print(x)
#endif


MemoryManager::MemoryManager(const void* multiboot_structure) {
    // Parse the multiboot structure to find the available memory regions. 
    // For simplicity, assume we only have one large usable memory region.
    // In a real OS, you'd handle multiple regions and reserved areas.

    m_memoryStart = 0x100000; // Start after kernel 
    m_memoryEnd = (1024 * 1024 * 16); // Assume 16MB for now
    m_nextFreeBlock = m_memoryStart;

    #ifdef DEBUG_MEMORY_MANAGER
    debug_print("Memory Manager Initialized:\n");
    debug_print("Start: ");
    printfHex(m_memoryStart);
    debug_print("\nEnd: ");
    printfHex(m_memoryEnd);
    debug_print("\n");
    #endif
}

uint32_t MemoryManager::allocateBlock() {
    if (m_nextFreeBlock >= m_memoryEnd) {
        // Out of memory!
        debug_print("Memory Manager: Out of memory!\n");
        return 0; // Return 0 to indicate failure
    }

    uint32_t allocatedAddress = m_nextFreeBlock;
    m_nextFreeBlock += 4096; // Allocate in 4KB blocks
    return allocatedAddress;
}

void MemoryManager::freeBlock(uint32_t addr) {
    // For now, we don't actually free memory. This would involve more
    // complex memory management with data structures to track free blocks.
    // Implement a proper memory allocation scheme here for a real OS.
    debug_print("Memory Manager: Memory freed (not actually implemented yet)\n");
}
