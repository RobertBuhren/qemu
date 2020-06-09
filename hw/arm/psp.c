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
#include "hw/arm/psp-misc.h"
#include "hw/arm/psp-smn.h"
#include "hw/arm/psp-timer.h"
#include "hw/arm/psp-sts.h"
#include "qemu/log.h"

PspGeneration PspNameToGen(const char* name) {
  /* TODO: Make the generation a property of the "psp" class */
  return ZEN;
}

uint32_t PspGetSramSize(PspGeneration gen) {
  switch(gen) {
    case ZEN:
      return PSP_SRAM_SIZE_ZEN;
    case ZEN_PLUS:
      return PSP_SRAM_SIZE_ZEN_PLUS;
    case ZEN2:
      return PSP_SRAM_SIZE_ZEN_2;
    default:
      return PSP_SRAM_SIZE_ZEN;
  }

}

static PSPMiscReg psp_regs[] = {
    {
        /* The on chip bootloader waits for bit 0 to go 1. */
        .addr = 0x03006038,
        .val = 0x1,
    },
    {
        /* MMIO mapped Fuse. */
        /* TODO: Read this value from other systems, i.e. Ryzen/TR */
        .addr = 0x03010104,
        .val = 0x1a060900, /* Value read from a real EPYC system */
    },
    {
        /* TODO: Document from PSPEmu */
        .addr = 0x0320004c,
        .val = 0xbc090072, 
    },
};

uint32_t PspGetSramAddr(PspGeneration gen) {
  return 0x0;
}
/* static const char* GenNames[3] = { "Zen", "Zen+", "Zen2"}; */
/* TODO: Maybe use TYPE_CPU_CLUSTER to create an SoC with multiple PSP's.
 * Example in armsse.c: armsse_init() */
/* NOTE: MachineState = state of instantiated MachineClass */

/* TODO: Check CPU Object properties */

/* Copied from hw/arm/digic_boards.c */

/* TODO: Remove code? */
void psp_load_firmware(AmdPspState *s, hwaddr addr)
{
    target_long rom_size;
    const char *filename;

    filename = bios_name;

    if (filename) {
        char *fn = qemu_find_file(QEMU_FILE_TYPE_BIOS, filename);

        if (!fn) {
            error_report("Couldn't find rom image '%s'.", filename);
            exit(1);
        }

        rom_size = load_image_targphys(fn, addr,s->sram.size);
        if (rom_size < 0 || rom_size > s->sram.size) {
            error_report("Couldn't load rom image '%s'.", filename);
            error_report("Reported SRAM size is 0x%x", (uint32_t)s->sram.size);
            exit(1);
        }
        g_free(fn);
    }
}

static void amd_psp_init(Object *obj) {

    AmdPspState *s = AMD_PSP(obj);

    /* TODO: Check which generation we actually are */
    PspGeneration gen = PspNameToGen("Zen");
    s->gen = gen;

    object_initialize_child(obj, "cpu", &s->cpu, sizeof(s->cpu),
                            ARM_CPU_TYPE_NAME("cortex-a9"),
                            &error_abort, NULL);

    object_initialize_child(obj, "smn", &s->smn, sizeof(s->smn),
                            TYPE_PSP_SMN, &error_abort, NULL);

    object_initialize_child(obj, "base_mem", &s->base_mem, sizeof(s->base_mem),
                            TYPE_PSP_MISC, &error_abort, NULL);
    
    object_initialize_child(obj, "timer1", &s->timer1, sizeof(s->timer1),
                            TYPE_PSP_TIMER, &error_abort, NULL);
    
    object_initialize_child(obj, "timer2", &s->timer2, sizeof(s->timer2),
                            TYPE_PSP_TIMER, &error_abort, NULL);
    
    object_initialize_child(obj, "psp-sts", &s->sts, sizeof(s->sts),
                            TYPE_PSP_STS, &error_abort, NULL);
}

static void amd_psp_realize(DeviceState *dev, Error **errp) {
    AmdPspState *s = AMD_PSP(dev);
    /* TODO: Maybe use "&error_abort" instead? */
    Error *err = NULL;
    uint32_t sram_size;
    uint32_t sram_addr;

    /* Enable the ARM TrustZone extensions */
    object_property_set_bool(OBJECT(&s->cpu), true, "has_el3", &err);

    /* Init CPU object. TODO convert to qdev_init_nofail */
    object_property_set_bool(OBJECT(&s->cpu), true, "realized" , &err);
    if (err != NULL) {
        error_propagate(errp, err);
        return;
    }

    object_property_set_uint(OBJECT(&s->smn), PSP_SMN_BASE, 
                             "smn-container-base", &error_abort);

    object_property_set_bool(OBJECT(&s->smn), true, "realized" , &error_abort);

    /* TODO: refactor instantiation of "base_mem" into a dedicated init 
     * function: base_mem_init().
     */
    object_property_set_uint(OBJECT(&s->base_mem), 0xFFFFFFFF,
                             "psp_misc_msize", &err);
    if (err != NULL) {
        error_propagate(errp, err);
        return;
    }

    object_property_set_str(OBJECT(&s->base_mem), "BASE MEM",
                                "psp_misc_ident", &error_abort);
    if (err != NULL) {
        error_propagate(errp, err);
        return;
    }

    object_property_set_bool(OBJECT(&s->base_mem), true, "realized", &err);
    if (err != NULL) {
        error_propagate(errp, err);
        return;
    }

    /* Init SRAM */
    sram_size = PspGetSramSize(s->gen);
    sram_addr = PspGetSramAddr(s->gen);
    memory_region_init_ram(&s->sram, OBJECT(dev), "sram", sram_size,
                           &error_abort);
    memory_region_add_subregion(get_system_memory(), sram_addr, &s->sram);

    /* Init ROM */
    memory_region_init_rom(&s->rom, OBJECT(dev), "rom", PSP_ROM_SIZE,
                           &error_abort);
    memory_region_add_subregion(get_system_memory(), PSP_ROM_BASE, &s->rom);

    /* Map SMN control registers */
    sysbus_mmio_map(SYS_BUS_DEVICE(&s->smn), 0, PSP_SMN_CTRL_BASE);

    /* Map timers */
    sysbus_mmio_map(SYS_BUS_DEVICE(&s->timer1), 0, PSP_TIMER1_BASE);
    sysbus_mmio_map(SYS_BUS_DEVICE(&s->timer2), 0, PSP_TIMER2_BASE);

    /* MAP PSP Status port */
    sysbus_mmio_map(SYS_BUS_DEVICE(&s->sts), 0, PSP_STS_ZEN1_BASE);

    /* TODO: Is this the way to go? ... */
    s->base_mem.regs = psp_regs;
    s->base_mem.regs_count = ARRAY_SIZE(psp_regs);

    /* Map the misc device as an overlap with low priority */
    /* This device covers all "unknown" psp registers */
    sysbus_mmio_map_overlap(SYS_BUS_DEVICE(&s->base_mem), 0, 0, -1000);
}

/* User-configurable options via "-device 'name','property'=" */
static Property amd_psp_properties[] = {
    DEFINE_PROP_UINT32("generation", AmdPspState, gen, 0),
    DEFINE_PROP_END_OF_LIST(),
};

static void amd_psp_class_init(ObjectClass *oc, void* data) {
    DeviceClass *dc = DEVICE_CLASS(oc);
    device_class_set_props(dc,amd_psp_properties);
    /* TODO: Add generation to the description */
    dc->desc = "AMD PSP";
    dc->realize = amd_psp_realize;
    /* TODO: Why? */
    dc->user_creatable = false;
}

/* TODO: Use "create_unimplemented_device" to log accesses to unknown mem areas */

static const TypeInfo amd_psp_type_info = {
    .name = TYPE_AMD_PSP,
    .parent = TYPE_DEVICE,
    .instance_size = sizeof(AmdPspState),
    .instance_init = amd_psp_init,
    .class_init = amd_psp_class_init,
};

static void amd_psp_register_types(void) {

    type_register_static(&amd_psp_type_info);
}

type_init(amd_psp_register_types);
