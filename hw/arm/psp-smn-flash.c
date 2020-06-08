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
#include "hw/arm/psp-smn-flash.h"



/* static const MemoryRegionOps smn_flash_ops = { */
/*     .read = psp_smn_flash_read, */
/*     .write = psp_smn_flash_write, */
/*     .endianness = DEVICE_LITTLE_ENDIAN, */
/*     .impl.min_access_size = 1, */
/*     .impl.max_access_size = 4, */
/* }; */

static void psp_smn_flash_init(Object *obj)
{
    char *rom_file;
    PSPSmnFlashState *s = PSP_SMN_FLASH(obj);

    memory_region_init_ram(&s->psp_smn_flash, obj, "flash",
                           PSP_SMN_FLASH_SIZE_16, &error_abort);

    if (bios_name != NULL) {
        rom_file = qemu_find_file(QEMU_FILE_TYPE_BIOS, bios_name);
        if (rom_file != NULL) {
            /* TODO: Error check */
            load_image_mr(rom_file, &s->psp_smn_flash);

        }
    }
}

static const TypeInfo psp_smn_flash_info = {
    .name = TYPE_PSP_SMN_FLASH,
    .parent = TYPE_SYS_BUS_DEVICE,
    .instance_init = psp_smn_flash_init,
    .instance_size = sizeof(PSPSmnFlashState),

};

static void psp_smn_register_types(void) {
    type_register_static(&psp_smn_flash_info);

}
type_init(psp_smn_register_types);
