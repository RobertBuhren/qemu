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
#ifndef AMD_CCP_V5_H
#define AMD_CCP_V5_H

#include "hw/sysbus.h"
#include "exec/memory.h"

#define TYPE_CCP_V5 "amd.ccpV5"
#define CCP_V5(obj) OBJECT_CHECK(CcpV5State, (obj), TYPE_CCP_V5)


/* Global sizes */
#define CCP_Q_SIZE           4096   /* Size of a single queue */
#define CCP_CTRL_SIZE        4096   /* Size of the general control region */ 
#define CCP_CONFIG_SIZE      4096   /* Size of the general config region */
#define CCP_Q_COUNT          5      /* Number of queues */

/* Global offsets */
#define CCP_CTRL_OFFSET      0x0    /* Offset of the general control region */
#define CCP_Q_OFFSET         0x1000 /* Offset of the first queue */
#define CCP_CONFIG_OFFSET    0x6000 /* Offset of the general config region */

/* Per queue offsets */
#define CCP_Q_CTRL_OFFSET    0x0    /* Control register offset */
#define CCP_Q_HEAD_LO_OFFSET 0x4    /* Queue Head register offset */
#define CCP_Q_TAIL_LO_OFFSET 0x8    /* Queue Tail register offset */ 
#define CCP_Q_STATUS_OFFSET  0x100  /* Queue status register offset */

/* Global registers */
#define CCP_G_Q_MASK_OFFSET  0x0
#define CCP_G_Q_PRIO_OFFSET  0x4

/* Queue registers */
/* The HALT bit, which indicates whether the queue is currently processing requests. */
#define CCP_Q_CTRL_HALT                     BIT(1)

/* Status register success indicator. */
#define CCP_Q_STATUS_SUCCESS                0

/* Run bit. Starts a CCP operation */
#define CCP_Q_RUN BIT(0)

/* TODO: indent */
#define CCP_Q_STATUS_SUCCESS                0
#define CCP_Q_STATUS_ERROR                  1

/* Total size of the CCP MMIO region */
#define CCP_MMIO_SIZE CCP_Q_COUNT * CCP_Q_SIZE + \
        CCP_CTRL_SIZE + CCP_CONFIG_SIZE

typedef struct CcpV5QState {
  uint32_t ccp_q_control;
  uint32_t ccp_q_tail;
  uint32_t ccp_q_head;
  uint32_t ccp_q_status;

  uint32_t ccp_q_id;

} CcpV5QState;

typedef struct CcpV5State {
    SysBusDevice parent_obj;
    MemoryRegion iomem;

    CcpV5QState q_states[CCP_Q_COUNT];

} CcpV5State;


#endif
