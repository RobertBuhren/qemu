#include "qemu/osdep.h"
#include "qapi/error.h"
#include "qemu/module.h"
#include "sysemu/sysemu.h"
#include "exec/address-spaces.h"
#include "hw/sysbus.h"
#include "qemu/osdep.h"
#include "qemu/units.h"
#include "qapi/error.h"
#include "hw/qdev-properties.h"
#include "hw/boards.h"
#include "hw/loader.h"
#include "hw/arm/psp.h"


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

    /* TODO rework: Use generic_loader: docs/generic-loader.txt */
    psp_load_firmware(psp,PSP_ROM_BASE);
    /* Set PC to high-vec */
    psp->cpu.env.regs[15] = 0xffff0000;


}

static void psp_zen_machine_init(MachineClass *mc) {
    mc->desc = "AMD PSP Zen";
    mc->init = &zen_init;
    mc->block_default_type = IF_NONE;
    mc->min_cpus = 1;
    mc->max_cpus = 1;
    mc->default_cpus = 1;
    mc->default_cpu_type = ARM_CPU_TYPE_NAME("cortex-a8");
    /* The PSP does not have ram, however the generic-loader device apparently
     * validates this value, so we set it here to an sufficiently large value
     * TODO: Verify that this has not other, unwanted side-effects.
     */
    mc->default_ram_size = 1 * GiB;
    mc->default_ram_id = "psp-ram";
}
DEFINE_MACHINE("amd-psp_zen", psp_zen_machine_init)
