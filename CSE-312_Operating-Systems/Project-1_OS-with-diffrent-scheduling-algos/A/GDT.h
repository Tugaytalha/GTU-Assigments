#ifndef __GDT_H
#define __GDT_H

#include "types.h"

class GlobalDescriptorTableEntry
{
private:
    uint16_t limit_low;           // The lower 16 bits of the limit
    uint16_t base_low;            // The lower 16 bits of the base
    uint8_t base_middle;          // The next 8 bits of the base
    uint8_t access;             // Access flags
    uint8_t granularity;         // Granularity and limit high bits
    uint8_t base_high;           // The last 8 bits of the base

public:
    GlobalDescriptorTableEntry();
    GlobalDescriptorTableEntry(uint32_t base, uint32_t limit, uint8_t access, uint8_t gran);

    // Getters
    uint16_t getBaseLow() const;
    uint8_t getBaseMiddle() const;
    uint8_t getBaseHigh() const;
    uint16_t getLimitLow() const;
    uint8_t getGranularity() const;
    uint8_t getAccess() const;

    // Setters
    void setBaseLow(uint16_t base_low);
    void setBaseMiddle(uint8_t base_middle);
    void setBaseHigh(uint8_t base_high);
    void setLimitLow(uint16_t limit_low);
    void setGranularity(uint8_t granularity);
    void setAccess(uint8_t access);
};

class GlobalDescriptorTable
{
private:
    GlobalDescriptorTableEntry nullSegmentSelector;
    GlobalDescriptorTableEntry unusedSegmentSelector;
    GlobalDescriptorTableEntry codeSegmentSelector;
    GlobalDescriptorTableEntry dataSegmentSelector;

public:
    GlobalDescriptorTable();
    ~GlobalDescriptorTable();

    uint16_t getCodeSegmentSelector();
    uint16_t getDataSegmentSelector();

    // This function loads the GDT into the GDTR register.
    void Load(); 
};

#endif