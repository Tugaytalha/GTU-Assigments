#ifndef __INTERRUPTS_H
#define __INTERRUPTS_H

#include "types.h"
#include "GDT.h"
//#include "port.h"
#include "Process.h"

class InterruptManager;

typedef void (*InterruptHandler)(uint32_t, InterruptManager*);
typedef void (*TimerInterruptHandler)(InterruptManager*);
typedef uint32_t InterruptDescriptorTableEntry[2];

class InterruptManager
{
protected:
    InterruptDescriptorTableEntry idt[256];
    GlobalDescriptorTable* gdt; 
    TimerInterruptHandler timerInterruptCallback;
    uint8_t currentProcessIndex; 

public:
    InterruptManager(GlobalDescriptorTable* gdt);
    ~InterruptManager();
    void Activate();

    // Interrupt handling routines
    static void HandleInterruptRequest0x00(uint32_t esp, InterruptManager* interruptManager); 
    static void HandleInterruptRequest0x01(uint32_t esp, InterruptManager* interruptManager);
    static void HandleInterruptRequest0x02(uint32_t esp, InterruptManager* interruptManager);
    static void HandleInterruptRequest0x03(uint32_t esp, InterruptManager* interruptManager);
    static void HandleInterruptRequest0x04(uint32_t esp, InterruptManager* interruptManager);
    static void HandleInterruptRequest0x05(uint32_t esp, InterruptManager* interruptManager);
    static void HandleInterruptRequest0x06(uint32_t esp, InterruptManager* interruptManager);
    static void HandleInterruptRequest0x07(uint32_t esp, InterruptManager* interruptManager);
    static void HandleInterruptRequest0x08(uint32_t esp, InterruptManager* interruptManager); 
    static void HandleInterruptRequest0x09(uint32_t esp, InterruptManager* interruptManager); 
    static void HandleInterruptRequest0x0A(uint32_t esp, InterruptManager* interruptManager); 
    static void HandleInterruptRequest0x0B(uint32_t esp, InterruptManager* interruptManager); 
    static void HandleInterruptRequest0x0C(uint32_t esp, InterruptManager* interruptManager); 
    static void HandleInterruptRequest0x0D(uint32_t esp, InterruptManager* interruptManager); 
    static void HandleInterruptRequest0x0E(uint32_t esp, InterruptManager* interruptManager); 
    static void HandleInterruptRequest0x0F(uint32_t esp, InterruptManager* interruptManager); 
    static void HandleInterruptRequest0x10(uint32_t esp, InterruptManager* interruptManager); 
    static void HandleInterruptRequest0x11(uint32_t esp, InterruptManager* interruptManager); 
    static void HandleInterruptRequest0x12(uint32_t esp, InterruptManager* interruptManager); 
    static void HandleInterruptRequest0x13(uint32_t esp, InterruptManager* interruptManager); 
    static void HandleInterruptRequest0x30(uint32_t esp, InterruptManager* interruptManager); 
    static void HandleInterruptRequest0x31(uint32_t esp, InterruptManager* interruptManager); 

    // Set interrupt gate descriptor
    void SetInterruptDescriptorTableEntry(
        uint8_t interruptNumber, 
        uint16_t codeSegmentSelectorOffset, 
        void (*handler)(), 
        uint8_t DescriptorPrivilegeLevel, 
        uint8_t DescriptorType
    );
    void setTimerInterruptCallback(TimerInterruptHandler handler, void* thisPointer);

    uint8_t getCurrentProcessIndex() const;
    void setCurrentProcessIndex(uint8_t index);
};

#endif