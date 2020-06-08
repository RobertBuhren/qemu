/*
 * AMD PSP emulation
 *
 * Copyright (C) 2020 Robert Buhren <robert@robertbuhren.de>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
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

static void psp_misc_set_identifier(Object *obj, const char *str, Error **errp) {
    PSPMiscState *s = PSP_MISC(obj);

    s->ident = g_strdup(str);
}

static char *psp_misc_get_identifier(Object *obj, Error **errp) {
    PSPMiscState *s = PSP_MISC(obj);

    return g_strdup(s->ident);
}

static const char* get_region_name(hwaddr addr) {

    /* TODO Fixme */

    if (addr > PSP_SMN_BASE && addr < PSP_MMIO_BASE) {
        return PSP_SMN_NAME;
    } else if (addr > PSP_MMIO_BASE && addr < PSP_X86_BASE) {
        return PSP_MMIO_NAME;
    } else if (addr > PSP_X86_BASE && addr < PSP_UNKNOWN_BASE) {
        return PSP_X86_NAME;
    } else if (addr > PSP_UNKNOWN_BASE && addr < PSP_ROM_BASE) {
        return PSP_UNKNOWN_NAME;
    }

    return PSP_ROM_NAME;
}

static void psp_misc_write(void *opaque, hwaddr offset,
                       uint64_t value, unsigned int size) {
    PSPMiscState *misc = PSP_MISC(opaque);
    hwaddr phys_base = misc->iomem.addr;

    /* TODO: Allow write to specific registers */

    /* We expect the pre-seeded memory regions to be instantiated with
     * physical addresses. Hence the compare with "phys_base + offset".
     */
    offset += phys_base;

    qemu_log_mask(LOG_UNIMP, "%s: unimplemented device write at %s "
                  "(size %d, offset 0x%" HWADDR_PRIx ", val 0x%lx)\n",
                  misc->ident, get_region_name(offset),size, offset, value);
}

static uint64_t psp_misc_read(void *opaque, hwaddr offset, unsigned int size) {
    
    /* TODO: Get current PC and regs */
    PSPMiscState *misc = PSP_MISC(opaque);
    
    hwaddr phys_base = misc->iomem.addr;
    int i;

    /* We expect the pre-seeded memory regions to be instantiated with
     * physical addresses. Hence the compare with "phys_base + offset".
     */
    offset += phys_base;


    for (i = 0; i < misc->regs_count; i++) {
        if (offset == misc->regs[i].addr)
            return misc->regs[i].val;
    }

    qemu_log_mask(LOG_UNIMP, "%s: unimplemented device read at %s "
                  "(size %d, offset 0x%" HWADDR_PRIx ")\n",
                  misc->ident, get_region_name(offset),size, offset);
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
    
    /* TODO: What is the difference to e.g. DEFINE_PROP_UINT32 ? */
    object_property_add_uint32_ptr(obj,"psp_misc_msize", &s->mmio_size,
                                   OBJ_PROP_FLAG_READWRITE, &error_abort);
    
    object_property_add_str(obj,"psp_misc_ident",psp_misc_get_identifier,
                            psp_misc_set_identifier, &error_abort);

}

static void psp_misc_realize(DeviceState *dev, Error **errp)
{
    PSPMiscState *s = PSP_MISC(dev);
    SysBusDevice *sbd = SYS_BUS_DEVICE(dev);

    memory_region_init_io(&s->iomem, OBJECT(dev), &misc_mem_ops, s,
            TYPE_PSP_MISC, s->mmio_size);

    sysbus_init_mmio(sbd, &s->iomem);

}

static void psp_misc_class_init(ObjectClass *oc, void *data) {
    DeviceClass *dc = DEVICE_CLASS(oc);
    dc->realize = psp_misc_realize;

}

static const TypeInfo psp_misc_info = {
    .name = TYPE_PSP_MISC,
    .parent = TYPE_SYS_BUS_DEVICE,
    .instance_init = psp_misc_init,
    .instance_size = sizeof(PSPMiscState),
    .class_init = psp_misc_class_init,

};

static void psp_misc_register_types(void) {
    type_register_static(&psp_misc_info);

}
type_init(psp_misc_register_types);
