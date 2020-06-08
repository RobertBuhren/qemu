#include "qemu/osdep.h"
#include "qapi/error.h"
#include "qemu/module.h"
#include "sysemu/sysemu.h"
#include "exec/address-spaces.h"
#include "hw/sysbus.h"
#include "qemu-common.h"
#include "qemu/osdep.h"
#include "qapi/error.h"
#include "hw/qdev-properties.h"
#include "hw/boards.h"
#include "hw/loader.h"
#include "hw/arm/psp.h"
#include "qemu/log.h"
#include "hw/arm/psp-timer.h"

static uint64_t psp_timer_read(void *opaque, hwaddr offset, unsigned int size) {
    PSPTimerState *s = PSP_TIMER(opaque);
    uint64_t val;
    hwaddr phys_base = s->psp_timer_iomem.addr;

    if (size != sizeof(uint32_t)) {
        qemu_log_mask(LOG_UNIMP, 
                      "PSP Timer: Error. Unsupported read size at offset 0x%" \
                      HWADDR_PRIx "\n", phys_base + offset);
        return 0;
    }

    switch (offset) {
        case 0:
            val = s->psp_timer_control;
            break;
        case 0x20:
            val = s->psp_timer_count;
            if (s->psp_timer_control & 0x1)
                s->psp_timer_count++;
            break;
        default:
            val = 0;
            break;
    }
    qemu_log_mask(LOG_UNIMP, 
                  "PSP Timer: Read at 0x%" HWADDR_PRIx " .Returning 0x%lx\n",
                  phys_base + offset, val);

    return val;
}



static void psp_timer_write(void *opaque, hwaddr offset, uint64_t val,
                            unsigned int size) {
    PSPTimerState *s = PSP_TIMER(opaque);
    hwaddr phys = s->psp_timer_iomem.addr;
    phys += offset;


    if (size != sizeof(uint32_t) && size != sizeof(uint8_t)) {
        qemu_log_mask(LOG_UNIMP, 
                      "PSP Timer: Error. Unsupported write size (%d) at offset 0x%" \
                      HWADDR_PRIx " with val 0x%lx\n", size, phys, val);
        return;
    }
    switch (offset) {
        case 0x0:
            s->psp_timer_control = val;
            break;
        case 0x1:
            /* XXX For single byte access by the on chip bl... */
            s->psp_timer_control |= val << 8;
            break;
        case 0x20:
            s->psp_timer_count = val;
            break;
        default:
            qemu_log_mask(LOG_UNIMP, "PSP Timer. Error: Unsupported MMIO accces." \
                          "offset=0x%" HWADDR_PRIx " value 0x%lx\n", offset,
                          val);
            break;

    }



}


static const MemoryRegionOps timer_mem_ops = {
    .read = psp_timer_read,
    .write = psp_timer_write,
    .endianness = DEVICE_LITTLE_ENDIAN,
    .impl.min_access_size = 1,
    .impl.max_access_size = 4,
};

static void psp_timer_init(Object *obj) {
    PSPTimerState *s = PSP_TIMER(obj);
    SysBusDevice *sbd = SYS_BUS_DEVICE(obj);
    s->psp_timer_control = 0;
    s->psp_timer_count = 0;

    memory_region_init_io(&s->psp_timer_iomem, obj, &timer_mem_ops, s,
                          TYPE_PSP_TIMER, PSP_TIMER_SIZE);
    sysbus_init_mmio(sbd, &s->psp_timer_iomem);
}

static const TypeInfo psp_timer_info = {
    .name = TYPE_PSP_TIMER,
    .parent = TYPE_SYS_BUS_DEVICE,
    .instance_init = psp_timer_init,
    .instance_size = sizeof(PSPTimerState),

};

static void psp_timer_register_types(void) {
    type_register_static(&psp_timer_info);

}
type_init(psp_timer_register_types);
