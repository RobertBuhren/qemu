#include "qemu/osdep.h"
#include "qapi/error.h"
#include "qemu/module.h"
#include "sysemu/sysemu.h"
#include "exec/address-spaces.h"
#include "hw/sysbus.h"
#include "qemu/osdep.h"
#include "qapi/error.h"
#include "hw/qdev-properties.h"
#include "hw/boards.h"
#include "hw/loader.h"
#include "hw/arm/psp.h"



/* static const char* GenNames[3] = { "Zen", "Zen+", "Zen2"}; */
/* TODO: Maybe use TYPE_CPU_CLUSTER to create an SoC with multiple PSP's.
 * Example in armsse.c: armsse_init() */
/* NOTE: MachineState = state of instantiated MachineClass */

/* TODO: Check CPU Object properties */

static void amd_psp_init(Object *obj) {

    AmdPspState *s = AMD_PSP(obj);

    /* TODO: Check which generation we actually are */
    PspGeneration gen = PspNameToGen("Zen");
    s->gen = gen;

    object_initialize_child(obj, "cpu", &s->cpu, sizeof(s->cpu),
                            ARM_CPU_TYPE_NAME("cortex-a8"),
                            &error_abort, NULL);

}

static void amd_psp_realize(DeviceState *dev, Error **errp) {
    AmdPspState *s = AMD_PSP(dev);
    Error *err = NULL;
    uint32_t sram_size;
    uint32_t sram_addr;

    /* Init CPU object */
    object_property_set_bool(OBJECT(&s->cpu), true, "realized" , &err);
    if (err != NULL) {
        error_propagate(errp, err);
        return;
    }

    /* Add SRAM */
    sram_size = PspGetSramSize(s->gen);
    sram_addr = PspGetSramAddr(s->gen);
    memory_region_init_ram(&s->sram, OBJECT(dev), "sram", sram_size,
                           &error_fatal);
    memory_region_add_subregion(get_system_memory(), sram_addr, &s->sram);

}

static Property amd_psp_properties[] = {
    DEFINE_PROP_UINT32("generation", AmdPspState, gen, 0),
    DEFINE_PROP_END_OF_LIST(),
};

static void amd_psp_class_init(ObjectClass *oc, void* data) {
    DeviceClass *dc = DEVICE_CLASS(oc);
    device_class_set_props(dc,amd_psp_properties);
    dc->desc = "AMD PSP";
    dc->realize = amd_psp_realize;
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
