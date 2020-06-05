#ifndef AMD_PSP_SMN_H
#define AMD_PSP_SMN_H

#include "hw/sysbus.h"
#include "exec/memory.h"

#define PSP_SMN_CTRL_SIZE 16 * 4

#define TYPE_PSP_SMN "amd_psp.smn"
#define PSP_SMN(obj) OBJECT_CHECK(PSPSmnState, (obj), TYPE_PSP_SMN)


typedef uint32_t PSPSmnAddr;

typedef struct PSPSmnState {
    SysBusDevice parent_obj;

    MemoryRegion psp_smn_control;
    SysBusDevice psp_flash;

    PSPSmnAddr psp_smn_slots[32];

} PSPSmnState;
#endif
