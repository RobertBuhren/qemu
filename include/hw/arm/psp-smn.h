#ifndef AMD_PSP_SMN_H
#define AMD_PSP_SMN_H

#include "hw/sysbus.h"
#include "exec/memory.h"
#include "hw/arm/psp-misc.h"
#include "hw/arm/psp-smn-flash.h"

#define PSP_SMN_CTRL_SIZE 16 * 4
#define PSP_SMN_SLOT_SIZE 1024 * 1024 /* 1M */
#define PSP_SMN_SLOT_COUNT 32
#define PSP_SMN_SLOT_NAME_LEN  20
#define PSP_SMN_SLOT_NAME "psp-slot_"

/* Flash base address for Zen1 */
#define PSP_SMN_FLASH_BASE 0xa000000

#define TYPE_PSP_SMN "amd_psp.smn"
#define PSP_SMN(obj) OBJECT_CHECK(PSPSmnState, (obj), TYPE_PSP_SMN)


typedef uint32_t PSPSmnAddr;

typedef struct PSPSmnState {
    SysBusDevice parent_obj;

    /* MemoryRegion containing the MMIO control registers */
    MemoryRegion psp_smn_control;

    /* MemoryRegions containing the SMN slots */
    MemoryRegion psp_smn_containers[PSP_SMN_SLOT_COUNT];

    /* Full 32bit SMN address space */
    MemoryRegion psp_smn_space;

    /* PSP Misc device that covers the whole SMN address space */
    PSPMiscState psp_smn_misc;

    /* Base address of the smn range. */
    hwaddr psp_smn_base;

    /* Current SMN state */
    PSPSmnAddr psp_smn_slots[32];

    /* The SMN attached flash */
    PSPSmnFlashState psp_smn_flash;

} PSPSmnState;
#endif
