#ifndef AMD_PSP_H
#define AMD_PSP_H

#include "hw/arm/boot.h"
#include "hw/sysbus.h"
#include "target/arm/cpu.h"

#define TYPE_AMD_PSP "amd-psp"
#define AMD_PSP(obj) OBJECT_CHECK(AmdPspState, (obj), TYPE_AMD_PSP)

#define PSP_ROM_ADDR  0xffff0000
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


static uint32_t PspGetSramSize(PspGeneration gen) {
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

static uint32_t PspGetSramAddr(PspGeneration gen) {
  return 0x0;
}

/* static const char* PspGenToName(PspGeneration gen) { */
/*   return GenNames[gen]; */
/* } */

static PspGeneration PspNameToGen(const char* name) {
  /* TODO */
  return ZEN;

}

typedef struct AmdPspState {
  /*< private >*/
  DeviceState parent_obj;
  /*< public >*/

  MemoryRegion sram;
  MemoryRegion rom;

  PspGeneration gen;
  ARMCPU cpu;

} AmdPspState;
#endif
