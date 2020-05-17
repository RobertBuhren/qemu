#ifndef AMD_PSP_MISC_H
#define AMD_PSP_MISC_H


#define TYPE_PSP_MISC "amd_psp.misc"
/* TODO: Mage misc IO size configurable */
#define PSP_MISC_IO_SIZE 0x100000000
#define PSP_MISC(obj) OBJECT_CHECK(PSPMiscState, (obj), TYPE_PSP_MISC)

typedef struct PSPMiscReg {
    uint32_t addr;
    uint32_t val;
} PSPMiscReg;


typedef struct PSPMiscState {
    SysBusDevice parent_obj;
    MemoryRegion iomem;

    uint32_t regs_count;
    PSPMiscReg *regs;

} PSPMiscState;


#endif
