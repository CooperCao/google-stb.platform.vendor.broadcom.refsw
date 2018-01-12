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

#ifndef _XLAT_H_
#define _XLAT_H_

#include <stddef.h>
#include <stdint.h>

#include "mmap.h"

#define XLAT_TABLE_SIZE_SHIFT   PAGE_SIZE_SHIFT
#define XLAT_TABLE_SIZE         (1 << XLAT_TABLE_SIZE_SHIFT)

#define XLAT_ENTRY_SIZE_SHIFT   3
#define XLAT_ENTRY_SIZE         (1 << XLAT_ENTRY_SIZE_SHIFT)

/* Values for number of entries in each MMU translation table */
#define XLAT_TABLE_ENTRIES_SHIFT (XLAT_TABLE_SIZE_SHIFT - XLAT_ENTRY_SIZE_SHIFT)
#define XLAT_TABLE_ENTRIES      (1 << XLAT_TABLE_ENTRIES_SHIFT)
#define XLAT_TABLE_ENTRIES_MASK (XLAT_TABLE_ENTRIES - 1)

/* Values to convert a memory address to an index into a translation table */
#define XLAT_L3_SIZE_SHIFT      PAGE_SIZE_SHIFT
#define XLAT_L2_SIZE_SHIFT      (XLAT_L3_SIZE_SHIFT + XLAT_TABLE_ENTRIES_SHIFT)
#define XLAT_L1_SIZE_SHIFT      (XLAT_L2_SIZE_SHIFT + XLAT_TABLE_ENTRIES_SHIFT)

/* Table entry descriptor */
#define INVALID_DESC            0x0
#define BLOCK_DESC              0x1
#define TABLE_DESC              0x3
#define UNSET_DESC             ~0x0ull

/* Lower attributes */
#define LOWER_ATTRS(x)                  (((x) & 0xfff) << 2)

#define NON_GLOBAL                      (0x1 << 9)
#define ACCESS_FLAG                     (0x1 << 8)
#define NSH                             (0x0 << 6)
#define OSH                             (0x2 << 6)
#define ISH                             (0x3 << 6)
#define AP_RO                           (0x1 << 5)
#define AP_RW                           (0x0 << 5)
#define NS                              (0x1 << 3)
#define ATTR_NON_CACHEABLE_INDEX        0x2
#define ATTR_DEVICE_INDEX               0x1
#define ATTR_IWBWA_OWBWA_NTR_INDEX      0x0

/* Upper attributes */
#define UPPER_ATTRS(x)                  (x & 0x7) << 52

#define XN                              (0x1ull << 2)
#define PXN                             (0x1ull << 1)
#define CONT_HINT                       (0x1ull << 0)

/* MAIR attributes */
#define MAIR_ATTR_SET(attr, index)      (attr << (index << 3))

#define ATTR_NON_CACHEABLE              (0x44)
#define ATTR_DEVICE                     (0x4)
#define ATTR_IWBWA_OWBWA_NTR            (0xff)

void xlat_init_tables(mmap_region_t *pmm);

#endif /* _XLAT_H_ */
