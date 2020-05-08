#include "qemu/osdep.h"
#include "qapi/error.h"
#include "qemu/module.h"
#include "sysemu/sysemu.h"
#include "exec/address-spaces.h"
#include "hw/sysbus.h"
#include "qemu/osdep.h"
#include "qemu-common.h"
#include "qapi/error.h"
#include "hw/qdev-properties.h"
#include "hw/boards.h"
#include "hw/loader.h"
#include "hw/arm/psp.h"

/* Copied from hw/arm/digic_boards.c */
static void load_firmware(AmdPspState *s, hwaddr addr)
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

static void zen_init(MachineState *machine) {

    AmdPspState *psp;
    /* TODO: Make gen a property of the Soc */
    PspGeneration gen = PspNameToGen("Zen");
    uint32_t sram_size = PspGetSramSize(gen);
    (void)sram_size;
    (void)gen;


    psp = AMD_PSP(object_new(TYPE_AMD_PSP));
    object_property_add_child(OBJECT(machine), "soc", OBJECT(psp),
                              &error_abort);
    
    /* Why do we call "... unref" ? */
    object_unref(OBJECT(psp));

    object_property_set_bool(OBJECT(psp), true, "realized", &error_abort);

    /* TODO rework */
    load_firmware(psp,PspGetSramAddr(psp->gen));


}

static void psp_zen_machine_init(MachineClass *mc) {
    mc->desc = "AMD PSP Zen";
    mc->init = &zen_init;
    mc->block_default_type = IF_NONE;
    mc->min_cpus = 1;
    mc->max_cpus = 1;
    mc->default_cpus = 1;
    mc->default_cpu_type = ARM_CPU_TYPE_NAME("cortex-a8");
    mc->default_ram_size = 1; // TODO
    mc->default_ram_id = "psp-ram";
}
DEFINE_MACHINE("amd-psp_zen", psp_zen_machine_init)
