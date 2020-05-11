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
#include "hw/arm/psp-misc.h"

/* TODO: Migrate misc regs from PSPEmu.
 * TODO: Implement call back to handle more complicated misc devices
 */
static const PSPMiscReg psp_regs[] = {
    {
        /* The on chip bootloader waits for bit 0 to go 1. */
        .addr = 0x03006038,
        .val = 0x1,
    },
    {
        /* TODO: Timer dev. Move to own device */
        .addr = 0x03010104,
        .val = 0xffffffff,
    },
};

static void psp_misc_write(void *opaque, hwaddr offset,
                       uint64_t value, unsigned int size) {
    qemu_log_mask(LOG_UNIMP, "%s: unimplemented device write "
                  "(size %d, offset 0x%" HWADDR_PRIx ", val 0x%lx)\n",
                  TYPE_PSP_MISC, size, offset, value);
}

static uint64_t psp_misc_read(void *opaque, hwaddr offset, unsigned int size) {
    
    /* TODO: Get current PC and regs */
    int i;

    for (i = 0; i < ARRAY_SIZE(psp_regs); i++) {
        if (offset == psp_regs[i].addr)
            return psp_regs[i].val;
    }

    qemu_log_mask(LOG_UNIMP, "%s: unimplemented device read "
                  "(size %d, offset 0x%" HWADDR_PRIx ")\n",
                  TYPE_PSP_MISC, size, offset);
    return 0;
}

static const MemoryRegionOps misc_mem_ops = {
    .read = psp_misc_read,
    .write = psp_misc_write,
    .endianness = DEVICE_LITTLE_ENDIAN,
    .impl.min_access_size = 1,
    .impl.max_access_size = 4,
};

static void psp_misc_init(Object *obj)
{
    PSPMiscState *s = PSP_MISC(obj);
    SysBusDevice *sbd = SYS_BUS_DEVICE(obj);

    memory_region_init_io(&s->iomem, obj, &misc_mem_ops, s,
            TYPE_PSP_MISC, PSP_MISC_IO_SIZE);

    sysbus_init_mmio(sbd, &s->iomem);

}

static const TypeInfo psp_misc_info = {
    .name = TYPE_PSP_MISC,
    .parent = TYPE_SYS_BUS_DEVICE,
    .instance_init = psp_misc_init,
    .instance_size = sizeof(PSPMiscState),

};

static void psp_misc_register_types(void) {
    type_register_static(&psp_misc_info);

}
type_init(psp_misc_register_types);
