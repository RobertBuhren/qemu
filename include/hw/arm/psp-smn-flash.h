#ifndef AMD_PSP_SMN_FLASH_H
#define AMD_PSP_SMN_FLASH_H

#include "hw/sysbus.h"
#include "exec/memory.h"

/* TODO: Configurable size based on image */
#define PSP_SMN_FLASH_SIZE_16 16 * 1024 * 1024
#define PSP_SMN_FLASH_SIZE_32 2 * PSP_SMN_FLASH_SIZE_16

#define TYPE_PSP_SMN_FLASH "amd_psp.smnflash"
#define PSP_SMN_FLASH(obj) OBJECT_CHECK(PSPSmnFlashState, (obj), TYPE_PSP_SMN_FLASH)

typedef struct PSPSmnFlashState {
    SysBusDevice parent_obj;

    MemoryRegion psp_smn_flash;


} PSPSmnFlashState;
#endif
