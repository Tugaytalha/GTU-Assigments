#ifndef __KEYBOARD_H
#define __KEYBOARD_H

#include "types.h"
#include "interrupts.h"

class KeyboardDriver
{
    InterruptManager* interruptManager;

public:
    KeyboardDriver(InterruptManager* interruptManager);
    ~KeyboardDriver();
    void handleKeyboardInterrupt(uint8_t scancode);
};

#endif