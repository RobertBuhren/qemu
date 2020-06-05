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
#include "hw/arm/psp-smn.h"

/* DEFINE_PROP_STRING("rom_path", AmdPspState,flash_rom_path), */

/* PSPSmnCTRL: Controller, attaches SMN devices on demand? */
static void psp_smn_write(void *opaque, hwaddr offset,
                       uint64_t value, unsigned int size) {
    PSPSmnState *smn = PSP_SMN(opaque);
    uint32_t idx;

    switch (size) {
        case 4:
            idx = (offset / 4) * 2;
            smn->psp_smn_slots[idx] = ((uint32_t) value & 0xffff) << 20;
            smn->psp_smn_slots[idx + 1] = ((uint32_t) value >> 16) << 20;
            break;
        case 2:
            idx = offset / 2;
            smn->psp_smn_slots[idx] = (uint32_t) value << 20;
            break;
        default:
            /* TODO: Log */
            break;
    }
}

static uint64_t psp_smn_read(void *opaque, hwaddr offset, unsigned int size) {
    
    PSPSmnState *smn = PSP_SMN(opaque);
    uint32_t idx;
    uint32_t val = 0;

    switch (size) {
        case 4:
            /* Each 4 byte access programs two slots.
             * SMN regions are 1M large, thus each SMN address is 1M aligned.
             */
            idx = (offset / 4) * 2;
            val = ((smn->psp_smn_slots[idx + 1] >> 20) << 16) |
                  (smn->psp_smn_slots[idx] >> 20);
            break;
            /* TODO: Log access */
        default:
            break;

    }
    return val;
}

static const MemoryRegionOps smn_ctlr_ops = {
    .read = psp_smn_read,
    .write = psp_smn_write,
    .endianness = DEVICE_LITTLE_ENDIAN,
    .impl.min_access_size = 1,
    .impl.max_access_size = 4,
};

static void psp_smn_init(Object *obj)
{
    PSPSmnState *s = PSP_SMN(obj);
    SysBusDevice *sbd = SYS_BUS_DEVICE(obj);

    memory_region_init_io(&s->psp_smn_control, obj, &smn_ctlr_ops, s,
            TYPE_PSP_SMN, PSP_SMN_CTRL_SIZE);

    sysbus_init_mmio(sbd, &s->psp_smn_control);

}

static const TypeInfo psp_smn_info = {
    .name = TYPE_PSP_SMN,
    .parent = TYPE_SYS_BUS_DEVICE,
    .instance_init = psp_smn_init,
    .instance_size = sizeof(PSPSmnState),

};

static void psp_smn_register_types(void) {
    type_register_static(&psp_smn_info);

}
type_init(psp_smn_register_types);
