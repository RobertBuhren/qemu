#ifndef AMD_PSP_MISC_H
#define AMD_PSP_MISC_H


#define TYPE_PSP_MISC "amd_psp.misc"
#define PSP_MISC_IO_SIZE 0x100000000
#define PSP_MISC(obj) OBJECT_CHECK(PSPMiscState, (obj), TYPE_PSP_MISC)

typedef struct PSPMiscReg {
    uint32_t addr;
    uint32_t val;
} PSPMiscReg;


typedef struct PSPMiscState {
    SysBusDevice parent_obj;
    MemoryRegion iomem;

    /* TODO: Define length at compile time */
    uint32_t regs[1];
} PSPMiscState;


#endif
