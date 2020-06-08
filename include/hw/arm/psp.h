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
#ifndef AMD_PSP_H
#define AMD_PSP_H

#include "hw/arm/boot.h"
#include "hw/sysbus.h"
#include "target/arm/cpu.h"
#include "hw/arm/psp-misc.h"
#include "hw/arm/psp-smn.h"
#include "hw/arm/psp-timer.h"

#define TYPE_AMD_PSP "amd-psp"
#define AMD_PSP(obj) OBJECT_CHECK(AmdPspState, (obj), TYPE_AMD_PSP)

#define PSP_SMN_CTRL_BASE 0x03220000
#define PSP_TIMER1_BASE 0x03010400
#define PSP_TIMER2_BASE 0x03010424

#define PSP_SMN_BASE 0x01000000
#define PSP_SMN_NAME "PSP SMN"

#define PSP_MMIO_BASE 0x03000000
#define PSP_MMIO_NAME "PSP MMIO"

#define PSP_X86_BASE 0x04000000
#define PSP_X86_NAME "PSP X86"

#define PSP_UNKNOWN_BASE  0xfc000000
#define PSP_UNKNOWN_NAME "PSP UNKNOWN"

#define PSP_ROM_BASE  0xffff0000
#define PSP_ROM_NAME "PSP ROM"

#define PSP_ROM_SIZE  0x10000

#define PSP_SRAM_ADDR  0x0
#define PSP_SRAM_SIZE_ZEN  0x40000
#define PSP_SRAM_SIZE_ZEN_PLUS  0x40000
#define PSP_SRAM_SIZE_ZEN_2  0x50000


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

  /* This device covers every MMIO address we have not covered somewhere else */
  PSPMiscState base_mem;

  /* This device represents the SMN address space including control registers */
  PSPSmnState smn;

  /* PSP Timer 1 at 0x03010400*/
  PSPTimerState timer1;
  
  /* PSP Timer 2 at 0x03010424*/
  PSPTimerState timer2;

} AmdPspState;

/* Loads a binary blob into the PSP address space */
void psp_load_firmware(AmdPspState *s, hwaddr addr);

uint32_t PspGetSramAddr(PspGeneration gen);

uint32_t PspGetSramSize(PspGeneration gen);

const char* PspGenToName(PspGeneration gen);
PspGeneration PspNameToGen(const char* name);

#endif
