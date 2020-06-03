#ifndef AMD_PSP_SMN_FLASH_H
#define AMD_PSP_SMN_FLASH_H

#include "hw/sysbus.h"
#include "exec/memory.h"

/* TODO: Move address-space definitions in psp.h */
#define PSP_SMN_FLASH_BASE 0x03220000
#define PSP_SMN_FLASH_SIZE 16 * 4

#define TYPE_PSP_SMN_FLASH "amd_psp.smn-flash"
#define PSP_SMN_FLASH(obj) OBJECT_CHECK(PSPSmnFLashState, (obj), TYPE_PSP_SMN_FLASH)

typedef struct PSPSmnFlashState {
    SysBusDevice parent_obj;

    MemoryRegion psp_smn_flash;

} PSPSmnFlashState;
#endif
