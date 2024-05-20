#include "GDT.h"

// Constructor for a null GDT entry.
GlobalDescriptorTableEntry::GlobalDescriptorTableEntry()
{
    this->limit_low = 0;
    this->base_low = 0;
    this->base_middle = 0;
    this->access = 0;
    this->granularity = 0;
    this->base_high = 0;
}

// Constructor for a GDT entry.
GlobalDescriptorTableEntry::GlobalDescriptorTableEntry(uint32_t base, uint32_t limit, uint8_t access, uint8_t gran)
{
    this->limit_low = limit & 0xFFFF;
    this->base_low = base & 0xFFFF;
    this->base_middle = (base >> 16) & 0xFF;
    this->access = access;
    this->granularity = (limit >> 16) & 0xF0 | gran & 0x0F;
    this->base_high = (base >> 24) & 0xFF;
}

// Getters
uint16_t GlobalDescriptorTableEntry::getBaseLow() const { return base_low; }
uint8_t GlobalDescriptorTableEntry::getBaseMiddle() const { return base_middle; }
uint8_t GlobalDescriptorTableEntry::getBaseHigh() const { return base_high; }
uint16_t GlobalDescriptorTableEntry::getLimitLow() const { return limit_low; }
uint8_t GlobalDescriptorTableEntry::getGranularity() const { return granularity; }
uint8_t GlobalDescriptorTableEntry::getAccess() const { return access; }

// Setters
void GlobalDescriptorTableEntry::setBaseLow(uint16_t base_low) { this->base_low = base_low; }
void GlobalDescriptorTableEntry::setBaseMiddle(uint8_t base_middle) { this->base_middle = base_middle; }
void GlobalDescriptorTableEntry::setBaseHigh(uint8_t base_high) { this->base_high = base_high; }
void GlobalDescriptorTableEntry::setLimitLow(uint16_t limit_low) { this->limit_low = limit_low; }
void GlobalDescriptorTableEntry::setGranularity(uint8_t granularity) { this->granularity = granularity; }
void GlobalDescriptorTableEntry::setAccess(uint8_t access) { this->access = access; }

// Constructor for the Global Descriptor Table.
GlobalDescriptorTable::GlobalDescriptorTable()
    : nullSegmentSelector(0, 0, 0, 0),
      unusedSegmentSelector(0, 0, 0, 0),
      codeSegmentSelector(0, 0xFFFFFFFF, 0x9A, 0xCF),  // Code Segment
      dataSegmentSelector(0, 0xFFFFFFFF, 0x92, 0xCF)   // Data Segment
{
    // Load the GDT into the GDTR register.
    Load(); 
}

GlobalDescriptorTable::~GlobalDescriptorTable() {}

// Getters for the Code and Data segment selectors.
uint16_t GlobalDescriptorTable::getCodeSegmentSelector() { return (uint8_t*)&codeSegmentSelector - (uint8_t*)this; }
uint16_t GlobalDescriptorTable::getDataSegmentSelector() { return (uint8_t*)&dataSegmentSelector - (uint8_t*)this; }

// This function loads the GDT into the GDTR register.
void GlobalDescriptorTable::Load()
{
    // Create a pointer to the GDT
    uint16_t* target = (uint16_t*)this; 

    // Limit of the GDT is the size of the GDT - 1.
    uint16_t size = (sizeof(GlobalDescriptorTable) - 1) & 0xFFFF;

    // Load the size and address of the GDT into the GDTR register.
    asm volatile("lgdt (%0)": : "p" (((uint32_t)size << 16) | (uint32_t)target));
}