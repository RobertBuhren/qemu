#ifndef AMD_PSP_MISC_H
#define AMD_PSP_MISC_H


#define TYPE_PSP_MISC "amd_psp.misc"
#define PSP_MISC(obj) OBJECT_CHECK(PSPMiscState, (obj), TYPE_PSP_MISC)

typedef struct PSPMiscReg {
    uint32_t addr;
    uint32_t val;
} PSPMiscReg;


typedef struct PSPMiscState {
    SysBusDevice parent_obj;
    MemoryRegion iomem;

    /* regs_count: Must be set to the number of PSPMiscReg elements in "regs" */
    uint32_t regs_count;

    /* regs: Pointer to an array of PSPMiscReg */
    PSPMiscReg *regs;

    /* Size of MMIO region covered by this instance */
    uint32_t mmio_size;

    /* Identifier of this instance */
    char *ident;

} PSPMiscState;


#endif
