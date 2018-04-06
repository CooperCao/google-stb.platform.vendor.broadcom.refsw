/******************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *****************************************************************************/

#ifndef _BRCMSTB_S3_H_
#define _BRCMSTB_S3_H_

#include <stdint.h>

/* Magic number in upper 16-bits */
#define BRCMSTB_S3_MAGIC_MASK           0xffff0000
#define BRCMSTB_S3_MAGIC_SHORT          0x5AFE0000

enum {
    /* Restore random key for AES memory verification (off = fixed key) */
    S3_FLAG_LOAD_RANDKEY                = (1 << 0),

    /* Scratch buffer page table is present */
    S3_FLAG_SCRATCH_BUFFER_TABLE        = (1 << 1),

    /* Skip all memory verification */
    S3_FLAG_NO_MEM_VERIFY               = (1 << 2),

    /*
     * Modification of this bit reserved for bootloader only.
     * 1=PSCI started Linux, 0=Direct jump to Linux.
     */
    S3_FLAG_PSCI_BOOT                   = (1 << 3),

    /*
     * Modification of this bit reserved for bootloader only.
     * 1=64 bit boot, 0=32 bit boot.
     */
    S3_FLAG_BOOTED64                    = (1 << 4),
    /*
     * Modification of this bit reserved for bootloader only.
     * 1=restore DTU state map, 0=do not restore.
     */
    S3_FLAG_DTU                         = (1 << 5),
};

#define AON_REG_MAGIC_FLAGS             0
#define AON_REG_CONTROL_LOW             1
#define AON_REG_CONTROL_HIGH            2
#define AON_REG_S3_HASH                 3 /* hash of S3 params */
#define AON_REG_CONTROL_HASH_LEN        7
#define AON_REG_DRAM_SCRAMBLE_FLAGS     8
#define AON_REG_PSCI_BASE               10 /* valid if PSCI present */
#define AON_REG_AVS_FLAGS               11
#define AON_REG_SSBL_BOOT_FLAGS         12
/* Spare: a whole 3 registers! */
#define AON_REG_MEMSYS_STATE            16

#define BOOTLOADER_SCRATCH_SIZE         64
#define BRCMSTB_HASH_LEN                (128 / 8) /* 128-bit hash */
#define IMAGE_DESCRIPTORS_BUFSIZE       (2 * 1024)

/*
 * Store up to 64 4KB page entries; this number is flexible, as long as
 * brcmstb_bootloader_scratch_table::num_entries is adjusted accordingly
 */
#define BRCMSTB_SCRATCH_BUF_SIZE        (256 * 1024)
#define BRCMSTB_DTU_STATE_MAP_ENTRIES   (8*1024)
#define BRCMSTB_DTU_CONFIG_ENTRIES      (512)
#define BRCMSTB_DTU_COUNT               (2)

struct brcmstb_bootloader_scratch_table {
    /* System page size, in KB; likely 4 (i.e., 4KB) */
    uint32_t page_size;

    /*
     * Number of page entries in this table. Provided for flexibility, but
     * should be BRCMSTB_SCRATCH_BUF_SIZE / PAGE_SIZE
     */
    uint16_t num_entries;
    uint16_t reserved;
    struct {
        uint32_t upper;
        uint32_t lower;
    } entries[];

} __attribute__((packed));

struct brcmstb_bootloader_dtu_table {
    uint32_t dtu_state_map[BRCMSTB_DTU_STATE_MAP_ENTRIES];
    uint32_t dtu_config[BRCMSTB_DTU_CONFIG_ENTRIES];
};

struct brcmstb_s3_params {
    /* scratch memory for bootloader */
    uint8_t scratch[BOOTLOADER_SCRATCH_SIZE];

    uint32_t magic; /* BRCMSTB_S3_MAGIC */
    uint64_t reentry; /* PA */

    /* descriptors */
    uint32_t hash[BRCMSTB_HASH_LEN / 4];

    /*
     * If 0, then ignore this parameter (there is only one set of
     *   descriptors)
     *
     * If non-0, then a second set of descriptors is stored at:
     *
     *   descriptors + desc_offset_2
     *
     * The MAC result of both descriptors is XOR'd and stored in @hash
     */
    uint32_t desc_offset_2;

    /*
     * (Physical) address of a brcmstb_bootloader_scratch_table, for
     * providing a large DRAM buffer to the bootloader
     */
    uint64_t buffer_table;

    uint32_t spare[70];

    uint8_t descriptors[IMAGE_DESCRIPTORS_BUFSIZE];
    struct brcmstb_bootloader_dtu_table dtu[BRCMSTB_DTU_COUNT];

} __attribute__((packed));

#endif /* _BRCMSTB_S3_H_ */
