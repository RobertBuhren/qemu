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
#include "qemu/log.h"
#include "hw/misc/ccpv5.h"
#include "hw/misc/ccpv5-linux.h"

static uint32_t ccp_queue_read(CcpV5State *s, hwaddr offset, uint32_t id) {
    uint32_t ret;
    CcpV5QState *qs;

    qs = &s->q_states[id];
    switch(offset) {
        case CCP_Q_CTRL_OFFSET:
            ret = qs->ccp_q_control;
            qemu_log_mask(LOG_UNIMP,
                          "CCP: queue %d ctrl read (val = 0x%x)\n", id, ret);
            break;
        case CCP_Q_TAIL_LO_OFFSET:
            ret = qs->ccp_q_tail;
            qemu_log_mask(LOG_UNIMP,
                          "CCP: queue %d tail read (val = 0x%x)\n", id, ret);
            break;
        case CCP_Q_HEAD_LO_OFFSET:
            ret = qs->ccp_q_head;
            qemu_log_mask(LOG_UNIMP,
                          "CCP: queue %d head read (val = 0x%x)\n", id, ret);
            break;
        case CCP_Q_STATUS_OFFSET:
            ret = qs->ccp_q_status;
            qemu_log_mask(LOG_UNIMP,
                          "CCP: queue %d status read (val = 0x%x)\n", id, ret);
            break;
        default:
            qemu_log_mask(LOG_UNIMP, "CCP: CCP queue read at unknown " \
                          "offset: 0x%" HWADDR_PRIx "\n", offset);
            ret = 0;
    }
    return ret;
} 

static void ccp_in_guest_pt(hwaddr dst, hwaddr src, uint32_t len, 
                                ccp_memtype dst_type, ccp_memtype src_type, 
                                ccp_pt_bitwise bwise, ccp_pt_byteswap bswap) {
    /* TODO: Ensure that we don't access un-accessible memory regions *
     *       Test whether dst - dst + len is within SRAM.
     */
    void* hdst;
    void* hsrc;
    hwaddr plen = len;

    if (bwise != CCP_PASSTHRU_BITWISE_NOOP ||
        bswap != CCP_PASSTHRU_BYTESWAP_NOOP) {

        qemu_log_mask(LOG_UNIMP, "CCP: Unimplemented passthrough bit ops: " \
                      "bitwise 0x%x byteswap 0x%x\n", bwise, bswap);
        return;

    }
    qemu_log_mask(LOG_UNIMP, "CCP: Performing passthrough. Copying 0x%x " \
                  "bytes from 0x%" HWADDR_PRIx " to 0x%" HWADDR_PRIx "\n",
                  len, src, dst);
    hdst = cpu_physical_memory_map(dst, &plen, true);
    hsrc = cpu_physical_memory_map(src, &plen, false);
    if (hdst == NULL || hsrc == NULL) {
        qemu_log_mask(LOG_GUEST_ERROR, "CCP: Couldn't map guest memory during" \
                      " passthrough operation\n");
        return;

    }
    memcpy(hdst, hsrc, len);
    cpu_physical_memory_unmap(hdst, len, true, 0);
    cpu_physical_memory_unmap(hsrc, len, false, 0);
}


static void ccp_passthrough(ccp5_desc *desc) {
    ccp_function func;
    ccp_memtype src_type;
    ccp_memtype dst_type;
    hwaddr src;
    hwaddr dst;
    uint32_t cbytes;
    ccp_pt_bitwise bwise;
    ccp_pt_byteswap bswap;

    func.raw = CCP5_CMD_FUNCTION(desc);
    src_type = CCP5_CMD_SRC_MEM(desc);
    dst_type = CCP5_CMD_DST_MEM(desc);
    cbytes   = CCP5_CMD_LEN(desc);
    /* TODO: Does the shift maybe cause issues? */
    src = CCP5_CMD_SRC_LO(desc) | ((hwaddr)(CCP5_CMD_SRC_HI(desc)) << 32);
    dst = CCP5_CMD_DST_LO(desc) | ((hwaddr)(CCP5_CMD_DST_HI(desc)) << 32);
    bwise = func.pt.bitwise;
    bswap = func.pt.byteswap;

    if (src_type == CCP_MEMTYPE_LOCAL && dst_type == CCP_MEMTYPE_LOCAL) {
        ccp_in_guest_pt(dst, src, cbytes, dst_type, src_type, bwise, bswap);
    } else {
        qemu_log_mask(LOG_UNIMP, "CCP: Unimplemented passthrough op: " \
                      "src 0x%" HWADDR_PRIx " dst 0x%" HWADDR_PRIx " src_type " \
                      "0x%x dst_type 0x%x cbytes 0x%x\n", src, dst, src_type,
                      dst_type, cbytes);
    }
}

static void ccp_execute(ccp5_desc *desc) {

    ccp_engine engine = CCP5_CMD_ENGINE(desc);


    switch(engine) {
        case CCP_ENGINE_AES:
            qemu_log_mask(LOG_UNIMP, "CCP: Unimplemented engine (AES)\n");
            break;
        case CCP_ENGINE_XTS_AES_128:
            qemu_log_mask(LOG_UNIMP, "CCP: Unimplemented engine (AES-XTS-128)\n");
            break;
        case CCP_ENGINE_DES3:
            qemu_log_mask(LOG_UNIMP, "CCP: Unimplemented engine (DES3)\n");
            break;
        case CCP_ENGINE_SHA:
            qemu_log_mask(LOG_UNIMP, "CCP: Unimplemented engine (SHA)\n");
            break;
        case CCP_ENGINE_RSA:
            qemu_log_mask(LOG_UNIMP, "CCP: Unimplemented engine (RSA)\n");
            break;
        case CCP_ENGINE_PASSTHRU:
            ccp_passthrough(desc);
            break;
        case CCP_ENGINE_ZLIB_DECOMPRESS:
            qemu_log_mask(LOG_UNIMP, "CCP: Unimplemented engine (ZLIB)\n");
            break;
        case CCP_ENGINE_ECC:
            qemu_log_mask(LOG_UNIMP, "CCP: Unimplemented engine (ECC)\n");
            break;
        default:
            qemu_log_mask(LOG_UNIMP, "CCP: Unknown engine! val 0x%x\n", 
                          CCP5_CMD_ENGINE(desc));
            break;
    }

}

static void ccp_process_q(CcpV5QState *qs) {

    ccp5_desc *desc;
    hwaddr req_len;
    uint32_t tail;
    uint32_t head;
    /* TODO: This operation needs to be delayed! */

    qemu_log_mask(LOG_UNIMP,
                  "CCP: queue %d start cmd at 0x%x\n",
                  qs->ccp_q_id, qs->ccp_q_tail);

    /* Clear RUN and HALT bit */
    qs->ccp_q_control &= ~((CCP_Q_RUN | CCP_Q_CTRL_HALT));

    /* TODO fix tail/head data types? */
    tail = qs->ccp_q_tail;
    head = qs->ccp_q_head;

    req_len = sizeof(ccp5_desc);

    while (tail < head) {
        desc = cpu_physical_memory_map(tail, &req_len, false);
        ccp_execute(desc);
        /* TODO: What is "access_len" ? */
        cpu_physical_memory_unmap(desc, req_len, false, 0);
        tail += sizeof(ccp5_desc);

    }

    /* TODO: Return proper CCP error codes */
    qs->ccp_q_tail = qs->ccp_q_head;
    qs->ccp_q_status = CCP_Q_STATUS_SUCCESS;
    qs->ccp_q_control |= CCP_Q_CTRL_HALT;

}

static void ccp_queue_write(CcpV5State *s, hwaddr offset, uint32_t val,
                                uint32_t id) {
    CcpV5QState *qs;

    /* TODO: Verify that queue exists ? */
    qs = &s->q_states[id];
    switch(offset) {
        case CCP_Q_CTRL_OFFSET:
            qs->ccp_q_control = val;
            qemu_log_mask(LOG_UNIMP,
                          "CCP: queue %d ctrl write (val = 0x%x)\n", id, val);
            if (val & CCP_Q_RUN) {
                ccp_process_q(qs);
            }
            break;
        case CCP_Q_TAIL_LO_OFFSET:
            qs->ccp_q_tail = val;
            qemu_log_mask(LOG_UNIMP,
                          "CCP: queue %d tail write (val = 0x%x)\n", id, val);
            break;
        case CCP_Q_HEAD_LO_OFFSET:
            qs->ccp_q_head = val;
            qemu_log_mask(LOG_UNIMP,
                          "CCP: queue %d head write (val = 0x%x)\n", id, val);
            break;
        case CCP_Q_STATUS_OFFSET:
            qs->ccp_q_status = val;
            qemu_log_mask(LOG_UNIMP,
                          "CCP: queue %d status write (val = 0x%x)\n", id, val);
            break;
        default:
            qemu_log_mask(LOG_UNIMP, "CCP: CCP queue write at unknown " \
                          "offset: 0x%" HWADDR_PRIx "\n", offset);
    }
    return;
}

static uint32_t ccp_ctrl_read(CcpV5State *s, hwaddr offset) {
    (void)s;

    /* TODO: handle or log global register access */
    switch (offset) {
        case CCP_G_Q_MASK_OFFSET:
            /* Ignored for now */
            break;
        case CCP_G_Q_PRIO_OFFSET:
            /* Ignored for now */
            break;
        default:
            qemu_log_mask(LOG_UNIMP, "CCP: Global ctrl write at offset " \
                                     "0x%" HWADDR_PRIx "\n", offset);
            break;
    }

    return 0;
}

static void ccp_ctrl_write(CcpV5State *s, hwaddr offset,
                                    uint32_t value) {
    (void)s;
    /* TODO: handle or log global register access */
    switch (offset) {
        case CCP_G_Q_MASK_OFFSET:
            /* Ignored for now */
            break;
        case CCP_G_Q_PRIO_OFFSET:
            /* Ignored for now */
            break;
        default:
            qemu_log_mask(LOG_UNIMP, "CCP: Global ctrl write at offset " \
                                     "0x%" HWADDR_PRIx " , value 0x%x\n", offset,
                                     value);
            break;
    }

    return;
}

static uint32_t ccp_config_read(CcpV5State *s, hwaddr offset) {
    (void)s;

    qemu_log_mask(LOG_UNIMP, "CCP: CCP config read at offset " \
                             "0x%" HWADDR_PRIx "\n", offset);

    switch (offset) {
        case 0x38:
            /* The on chip bootloader waits for bit 0 to go 1. */
            return 1;
            break;
        default:
            return 0;
            break;
    }
}

static void ccp_config_write(CcpV5State *s, hwaddr offset,
                                    uint32_t value) {
    (void)s;
    qemu_log_mask(LOG_UNIMP, "CCP: CCP config write at offset " \
                             "0x%" HWADDR_PRIx " , value 0x%x\n", offset,
                             value);

    return;
}

static uint64_t ccp_read(void *opaque, hwaddr offset, unsigned int size) {
    CcpV5State *s = CCP_V5(opaque);
    uint32_t ret = 0;
    uint32_t id = 0;

    /* qemu_log_mask(LOG_UNIMP, "CCP: MMIO read at 0x%" HWADDR_PRIx "\n", */
    /*               offset); */

    if (size != sizeof(uint32_t)) {
        qemu_log_mask(LOG_GUEST_ERROR, "CCP Error: Unsupported read size: " \
                                       "0x%x\n", size);
        return 0;
    }
    if (offset < CCP_Q_OFFSET) {
        ret = ccp_ctrl_read(s,offset);
    } else if (offset < CCP_CONFIG_OFFSET) {
        /* CCP queue access */
        id = (offset >> 12) - 1; // Make queue index start with 0
        ret = ccp_queue_read(s, offset & 0xfff, id);
    } else {
        ret = ccp_config_read(s,offset & 0xfff);
    }

    return ret;
}

static void ccp_write(void *opaque, hwaddr offset,
                       uint64_t value, unsigned int size) {
    CcpV5State *s = CCP_V5(opaque);
    uint32_t id = 0;
    

    if (size != sizeof(uint32_t)) {
        qemu_log_mask(LOG_GUEST_ERROR, "CCP Error: Unsupported write size:" \
                                       "0x%x\n", size);
        return;
    }

    if (offset < CCP_Q_OFFSET) {
        /* TODO: Explicit cast? */
        ccp_ctrl_write(s, offset, value);
    } else if (offset < CCP_CONFIG_OFFSET) {
        /* CCP queue access */
        id = (offset >> 12);
        if (id > CCP_Q_COUNT) {
            /* This should never happen, but... */
            qemu_log_mask(LOG_GUEST_ERROR, "CCP Error: Invalid queue id 0x%d\n",
                          id);
        }
        ccp_queue_write(s, offset & 0xfff, value, id);
    } else {
        ccp_config_write(s,offset & 0xfff, value);
    }

}

static void ccp_init_q(CcpV5State *s) {
    int i;

    for (i = 0; i < CCP_Q_COUNT; i++) {
        memset(&s->q_states[i], 0, sizeof(CcpV5QState));
        s->q_states[i].ccp_q_control = CCP_Q_CTRL_HALT;
        s->q_states[i].ccp_q_status = CCP_Q_STATUS_SUCCESS;
        s->q_states[i].ccp_q_id = i;
    }

}

static const MemoryRegionOps ccp_mem_ops = {
    .read = ccp_read,
    .write = ccp_write,
    .endianness = DEVICE_LITTLE_ENDIAN,
    /* TODO: Does the CCP really allow non-word aligned accesses ? */
    .impl.min_access_size = 1,
    .impl.max_access_size = 4,
};

static void ccp_init(Object *obj) {
    CcpV5State *s = CCP_V5(obj);
    SysBusDevice *sbd = SYS_BUS_DEVICE(obj);


    memory_region_init_io(&s->iomem, obj, &ccp_mem_ops, s,
            TYPE_CCP_V5, CCP_MMIO_SIZE);

    sysbus_init_mmio(sbd, &s->iomem);

    ccp_init_q(s);

}

static const TypeInfo ccp_info = {
    .name = TYPE_CCP_V5,
    .parent = TYPE_SYS_BUS_DEVICE,
    .instance_init = ccp_init,
    .instance_size = sizeof(CcpV5State),

};

static void ccp_register_types(void) {
    type_register_static(&ccp_info);

}
type_init(ccp_register_types);
