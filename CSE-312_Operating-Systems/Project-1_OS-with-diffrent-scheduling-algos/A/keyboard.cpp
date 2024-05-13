#include "keyboard.h"
#include "interrupts.h"

KeyboardDriver::KeyboardDriver(InterruptManager* manager)
: InterruptHandler(manager, 0x21)
{
    while(0x60 == inportb(0x64));
    outportb(0x60, 0xF4);
}

KeyboardDriver::~KeyboardDriver()
{
}

void KeyboardDriver::OnInterrupt(uint32_t esp)
{
    uint8_t key = inportb(0x60);
    //printf("KEY: ");
    //printfHex(key);
    //printf("\n");
    
    if(key < 0x80) // Key pressed
    {
        switch(key)
        {
            case 0x02: printf("1"); break;
            case 0x03: printf("2"); break;
            case 0x04: printf("3"); break;
            case 0x05: printf("4"); break;
            case 0x06: printf("5"); break;
            case 0x07: printf("6"); break;
            case 0x08: printf("7"); break;
            case 0x09: printf("8"); break;
            case 0x0A: printf("9"); break;
            case 0x0B: printf("0"); break;
            case 0x0C: printf("-"); break;
            case 0x0D: printf("+"); break;
            case 0x0E: printf("Backspace"); break;
            case 0x0F: printf("Tab"); break;
            case 0x10: printf("Q"); break;
            case 0x11: printf("W"); break;
            case 0x12: printf("E"); break;
            case 0x13: printf("R"); break;
            case 0x14: printf("T"); break;
            case 0x15: printf("Y"); break;
            case 0x16: printf("U"); break;
            case 0x17: printf("I"); break;
            case 0x18: printf("O"); break;
            case 0x19: printf("P"); break;
            case 0x1A: printf("["); break;
            case 0x1B: printf("]"); break;
            case 0x1C: printf("Enter"); break;
            case 0x1D: printf("LCtrl"); break;
            case 0x1E: printf("A"); break;
            case 0x1F: printf("S"); break;
            case 0x20: printf("D"); break;
            case 0x21: printf("F"); break;
            case 0x22: printf("G"); break;
            case 0x23: printf("H"); break;
            case 0x24: printf("J"); break;
            case 0x25: printf("K"); break;
            case 0x26: printf("L"); break;
            case 0x27: printf(";"); break;
            case 0x28: printf("'"); break;
            case 0x29: printf("`"); break;
            case 0x2A: printf("LShift"); break;
            case 0x2B: printf("\\"); break;
            case 0x2C: printf("Z"); break;
            case 0x2D: printf("X"); break;
            case 0x2E: printf("C"); break;
            case 0x2F: printf("V"); break;
            case 0x30: printf("B"); break;
            case 0x31: printf("N"); break;
            case 0x32: printf("M"); break;
            case 0x33: printf(","); break;
            case 0x34: printf("."); break;
            case 0x35: printf("/"); break;
            case 0x36: printf("RShift"); break;
            case 0x37: printf("Keypad *"); break;
            case 0x38: printf("LAlt"); break;
            case 0x39: printf(" "); break;
            case 0x3A: printf("CapsLock"); break;
            case 0x3B: printf("F1"); break;
            case 0x3C: printf("F2"); break;
            case 0x3D: printf("F3"); break;
            case 0x3E: printf("F4"); break;
            case 0x3F: printf("F5"); break;
            case 0x40: printf("F6"); break;
            case 0x41: printf("F7"); break;
            case 0x42: printf("F8"); break;
            case 0x43: printf("F9"); break;
            case 0x44: printf("F10"); break;
            case 0x45: printf("NumLock"); break;
            case 0x46: printf("ScrollLock"); break;
            case 0x47: printf("Keypad 7"); break;
            case 0x48: printf("Keypad 8"); break;
            case 0x49: printf("Keypad 9"); break;
            case 0x4A: printf("Keypad -"); break;
            case 0x4B: printf("Keypad 4"); break;
            case 0x4C: printf("Keypad 5"); break;
            case 0x4D: printf("Keypad 6"); break;
            case 0x4E: printf("Keypad +"); break;
            case 0x4F: printf("Keypad 1"); break;
            case 0x50: printf("Keypad 2"); break;
            case 0x51: printf("Keypad 3"); break;
            case 0x52: printf("Keypad 0"); break;
            case 0x53: printf("Keypad ."); break;
            case 0x57: printf("F11"); break;
            case 0x58: printf("F12"); break;
            default: 
                char* foo = "KEY: 0x00";
                char* hex = "0123456789ABCDEF";
                foo[6] = hex[(key >> 4) & 0xF];
                foo[7] = hex[key & 0xF];
                printf(foo);
                break;
        }
    }
}