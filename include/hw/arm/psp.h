#ifndef AMD_PSP_H
#define AMD_PSP_H

#include "hw/arm/boot.h"
#include "hw/sysbus.h"
#include "target/arm/cpu.h"

#define TYPE_AMD_PSP "amd-psp"
#define AMD_PSP(obj) OBJECT_CHECK(AmdPspState, (obj), TYPE_AMD_PSP)

#define PSP_ROM_ADDR  0xffff0000
#define PSP_ROM_SIZE  0x10000

#define PSP_SRAM_ADDR  0x0
#define PSP_SRAM_SIZE_ZEN  0x40000
#define PSP_SRAM_SIZE_ZEN_PLUS  0x40000
#define PSP_SRAM_SIZE_ZEN_2  0x50000

/* extern const char** GenNames; */


typedef enum PspGeneration {
  ZEN = 0,
  ZEN_PLUS,
  ZEN2,
} PspGeneration;

typedef struct AmdPspState {
  /*< private >*/
  DeviceState parent_obj;
  /*< public >*/

  MemoryRegion sram;
  MemoryRegion rom;

  PspGeneration gen;
  ARMCPU cpu;

} AmdPspState;

/* Loads a binary blob into the PSP address space */
void load_firmware(AmdPspState *s, hwaddr addr);

uint32_t PspGetSramAddr(PspGeneration gen);

uint32_t PspGetSramSize(PspGeneration gen);

const char* PspGenToName(PspGeneration gen);
PspGeneration PspNameToGen(const char* name);

#endif
