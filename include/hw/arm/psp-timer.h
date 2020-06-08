#ifndef AMD_PSP_TIMER_H
#define AMD_PSP_TIMER_H

#include "hw/sysbus.h"
#include "exec/memory.h"

#define TYPE_PSP_TIMER "amd_psp.timer"
#define PSP_TIMER(obj) OBJECT_CHECK(PSPTimerState, (obj), TYPE_PSP_TIMER)

/* PSP Timer iomem size */
#define PSP_TIMER_SIZE 0x24

typedef struct PSPTimerState {
    SysBusDevice parent_obj;

    /* MemoryRegion containing the MMIO registers */
    MemoryRegion psp_timer_iomem;

    uint32_t psp_timer_control;
    uint32_t psp_timer_count;


} PSPTimerState;
#endif
