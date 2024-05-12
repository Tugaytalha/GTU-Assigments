#include "interrupts.h"
#include "kernel.cpp" // for debugging functions

// #define for debugging
// #define DEBUG_INTERRUPT

#ifdef DEBUG_INTERRUPT
    #define debug_print(x)  printf(x)
#else
    #define debug_print(x)
#endif

InterruptManager::InterruptManager(GlobalDescriptorTable* gdt)
: timerInterruptCallback(0), currentProcessIndex(0)
{
    this->gdt = gdt;
    for (uint16_t i = 0; i < 256; i++)
    {
        SetInterruptDescriptorTableEntry(i, 0, 0, 0, 0);
    }
    
    // Setting interrupt handlers
    SetInterruptDescriptorTableEntry(0x00, gdt->getCodeSegmentSelector(), &InterruptManager::HandleInterruptRequest0x00, 0, 0x8E);
    SetInterruptDescriptorTableEntry(0x01, gdt->getCodeSegmentSelector(), &InterruptManager::HandleInterruptRequest0x01, 0, 0x8E);
    SetInterruptDescriptorTableEntry(0x02, gdt->getCodeSegmentSelector(), &InterruptManager::HandleInterruptRequest0x02, 0, 0x8E);
    SetInterruptDescriptorTableEntry(0x03, gdt->getCodeSegmentSelector(), &InterruptManager::HandleInterruptRequest0x03, 0, 0x8E);
    SetInterruptDescriptorTableEntry(0x04, gdt->getCodeSegmentSelector(), &InterruptManager::HandleInterruptRequest0x04, 0, 0x8E);
    SetInterruptDescriptorTableEntry(0x05, gdt->getCodeSegmentSelector(), &InterruptManager::HandleInterruptRequest0x05, 0, 0x8E);
    SetInterruptDescriptorTableEntry(0x06, gdt->getCodeSegmentSelector(), &InterruptManager::HandleInterruptRequest0x06, 0, 0x8E);
    SetInterruptDescriptorTableEntry(0x07, gdt->getCodeSegmentSelector(), &InterruptManager::HandleInterruptRequest0x07, 0, 0x8E);
    SetInterruptDescriptorTableEntry(0x08, gdt->getCodeSegmentSelector(), &InterruptManager::HandleInterruptRequest0x08, 0, 0x8E);
    SetInterruptDescriptorTableEntry(0x09, gdt->getCodeSegmentSelector(), &InterruptManager::HandleInterruptRequest0x09, 0, 0x8E);
    SetInterruptDescriptorTableEntry(0x0A, gdt->getCodeSegmentSelector(), &InterruptManager::HandleInterruptRequest0x0A, 0, 0x8E);
    SetInterruptDescriptorTableEntry(0x0B, gdt->getCodeSegmentSelector(), &InterruptManager::HandleInterruptRequest0x0B, 0, 0x8E);
    SetInterruptDescriptorTableEntry(0x0C, gdt->getCodeSegmentSelector(), &InterruptManager::HandleInterruptRequest0x0C, 0, 0x8E);
    SetInterruptDescriptorTableEntry(0x0D, gdt->getCodeSegmentSelector(), &InterruptManager::HandleInterruptRequest0x0D, 0, 0x8E);
    SetInterruptDescriptorTableEntry(0x0E, gdt->getCodeSegmentSelector(), &InterruptManager::HandleInterruptRequest0x0E, 0, 0x8E);
    SetInterruptDescriptorTableEntry(0x0F, gdt->getCodeSegmentSelector(), &InterruptManager::HandleInterruptRequest0x0F, 0, 0x8E);
    SetInterruptDescriptorTableEntry(0x10, gdt->getCodeSegmentSelector(), &InterruptManager::HandleInterruptRequest0x10, 0, 0x8E);
    SetInterruptDescriptorTableEntry(0x11, gdt->getCodeSegmentSelector(), &InterruptManager::HandleInterruptRequest0x11, 0, 0x8E);
    SetInterruptDescriptorTableEntry(0x12, gdt->getCodeSegmentSelector(), &InterruptManager::HandleInterruptRequest0x12, 0, 0x8E);
    SetInterruptDescriptorTableEntry(0x13, gdt->getCodeSegmentSelector(), &InterruptManager::HandleInterruptRequest0x13, 0, 0x8E);

    // Setting handler for system call interrupt
    SetInterruptDescriptorTableEntry(0x30, gdt->getCodeSegmentSelector(), &InterruptManager::HandleInterruptRequest0x30, 0, 0xEE);

    // Setting handler for exit system call
    SetInterruptDescriptorTableEntry(0x31, gdt->getCodeSegmentSelector(), &InterruptManager::HandleInterruptRequest0x31, 0, 0xEE); 

    // Initializing PIC
    outb(0x20, 0x11);
    outb(0xA0, 0x11);
    outb(0x21, 0x20);
    outb(0xA1, 0x28);
    outb(0x21, 0x04);
    outb(0xA1, 0x02);
    outb(0x21, 0x01);
    outb(0xA1, 0x01);
    outb(0x21, 0x0);
    outb(0xA1, 0x0);

    // Enabling keyboard interrupt
    outb(0x21, 0xFD);

    // Enabling timer interrupt
    outb(0x21, 0xFE); 
}

InterruptManager::~InterruptManager() {}

void InterruptManager::Activate()
{
    // Load the address of the IDT into the IDTR register
    uint32_t idt_address = (uint32_t)this;
    uint16_t idt_size = sizeof(InterruptManager)-1;
    
    // Load the IDT
    asm volatile("lidt (%0)": :"p" (((uint32_t)idt_size<<16) | idt_address)); 
}

// Setting IDT entry
void InterruptManager::SetInterruptDescriptorTableEntry(
    uint8_t interruptNumber, 
    uint16_t codeSegmentSelectorOffset, 
    void (*handler)(), 
    uint8_t DescriptorPrivilegeLevel, 
    uint8_t DescriptorType)
{
    // Calculate the offset of the handler function
    uint32_t handler_address = (uint32_t) handler;

    // Set the IDT entry
    idt[interruptNumber][0] = handler_address & 0xFFFF;
    idt[interruptNumber][1] = codeSegmentSelectorOffset << 16 | (handler_address >> 16) & 0xFFFF |
                            DescriptorType << 8 | DescriptorPrivilegeLevel << 13 | 0x8000;
}

// Interrupt handlers
void InterruptManager::HandleInterruptRequest0x00(uint32_t esp, InterruptManager* interruptManager)
{
    printf("INTERRUPT 0");
    while(1);
}

void InterruptManager::HandleInterruptRequest0x01(uint32_t esp, InterruptManager* interruptManager)
{
    debug_print("Keyboard Interrupt\n");

    // Read the keyboard scan code from port 0x60
    uint8_t key = inb(0x60);

    // Forward the key to the keyboard driver
    keyboardDriver.handleKeyboardInterrupt(key);
    
    // Acknowledge the interrupt to the PIC
    outb(0x20, 0x20);
}

void InterruptManager::HandleInterruptRequest0x02(uint32_t esp, InterruptManager* interruptManager) { printf("INTERRUPT 2\n"); while(1); }
void InterruptManager::HandleInterruptRequest0x03(uint32_t esp, InterruptManager* interruptManager) { printf("INTERRUPT 3\n"); while(1); }
void InterruptManager::HandleInterruptRequest0x04(uint32_t esp, InterruptManager* interruptManager) { printf("INTERRUPT 4\n"); while(1); }
void InterruptManager::HandleInterruptRequest0x05(uint32_t esp, InterruptManager* interruptManager) { printf("INTERRUPT 5\n"); while(1); }
void InterruptManager::HandleInterruptRequest0x06(uint32_t esp, InterruptManager* interruptManager) { printf("INTERRUPT 6\n"); while(1); }
void InterruptManager::HandleInterruptRequest0x07(uint32_t esp, InterruptManager* interruptManager) { printf("INTERRUPT 7\n"); while(1); }
void InterruptManager::HandleInterruptRequest0x08(uint32_t esp, InterruptManager* interruptManager) { printf("INTERRUPT 8\n"); while(1); }
void InterruptManager::HandleInterruptRequest0x09(uint32_t esp, InterruptManager* interruptManager) { printf("INTERRUPT 9\n"); while(1); }
void InterruptManager::HandleInterruptRequest0x0A(uint32_t esp, InterruptManager* interruptManager) { printf("INTERRUPT 10\n"); while(1); }
void InterruptManager::HandleInterruptRequest0x0B(uint32_t esp, InterruptManager* interruptManager) { printf("INTERRUPT 11\n"); while(1); }
void InterruptManager::HandleInterruptRequest0x0C(uint32_t esp, InterruptManager* interruptManager) { printf("INTERRUPT 12\n"); while(1); }
void InterruptManager::HandleInterruptRequest0x0D(uint32_t esp, InterruptManager* interruptManager) { printf("INTERRUPT 13\n"); while(1); }
void InterruptManager::HandleInterruptRequest0x0E(uint32_t esp, InterruptManager* interruptManager) { printf("INTERRUPT 14\n"); while(1); }
void InterruptManager::HandleInterruptRequest0x0F(uint32_t esp, InterruptManager* interruptManager) { printf("INTERRUPT 15\n"); while(1); }
void InterruptManager::HandleInterruptRequest0x10(uint32_t esp, InterruptManager* interruptManager) { printf("INTERRUPT 16\n"); while(1); }
void InterruptManager::HandleInterruptRequest0x11(uint32_t esp, InterruptManager* interruptManager) { printf("INTERRUPT 17\n"); while(1); }
void InterruptManager::HandleInterruptRequest0x12(uint32_t esp, InterruptManager* interruptManager) { printf("INTERRUPT 18\n"); while(1); }
void InterruptManager::HandleInterruptRequest0x13(uint32_t esp, InterruptManager* interruptManager) { 
    debug_print("Timer Interrupt\n");
    if(timerInterruptCallback != 0)
    {
        timerInterruptCallback(this);
    }
    outb(0x20, 0x20);
}


void InterruptManager::HandleInterruptRequest0x30(uint32_t esp, InterruptManager* interruptManager) { 
    debug_print("System call interrupt\n");
    syscallHandler.handleSyscallInterrupt(); 
}

void InterruptManager::HandleInterruptRequest0x31(uint32_t esp, InterruptManager* interruptManager) {
    debug_print("Exit system call\n");
    syscallHandler.handleExitSyscall();
}

void InterruptManager::setTimerInterruptCallback(TimerInterruptHandler handler, void* thisPointer)
{
    timerInterruptCallback = (TimerInterruptHandler)((uint32_t)handler + (uint32_t)thisPointer);
}

uint8_t InterruptManager::getCurrentProcessIndex() const {
    return currentProcessIndex;
}

void InterruptManager::setCurrentProcessIndex(uint8_t index) {
    currentProcessIndex = index;
}