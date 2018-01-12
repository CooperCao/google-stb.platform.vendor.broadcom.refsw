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

#ifndef _MEMORY_H_
#define _MEMORY_H_

#include <stddef.h>
#include <stdint.h>
#include "config.h"

#define IS_PAGE_ALIGNED(addr)   (((addr) & PAGE_SIZE_MASK) == 0)
#define ALIGN_TO_PAGE(addr)     ( (addr) & ~PAGE_SIZE_MASK)
#define ROUND_TO_PAGE(addr)     (((addr) + PAGE_SIZE_MASK) & ~PAGE_SIZE_MASK)

/*
 * Shifts and masks to access fields of an mmap_attr_t
 */
/* Memory types */
#define MT_TYPE_MASK            0x7
#define MT_TYPE(_attr)          ((_attr) & MT_TYPE_MASK)
/* Access permissions (RO/RW) */
#define MT_PERM_SHIFT           3
/* Security state (SECURE/NS) */
#define MT_SEC_SHIFT            4
/* Access permissions for instruction execution (EXECUTE/EXECUTE_NEVER) */
#define MT_EXECUTE_SHIFT        5

/*
 * Memory mapping attributes
 */
typedef enum  {
    /*
     * Memory types
     *
     * These are organised so that, going down the list, the memory types
     * are getting weaker; conversely going up the list the memory types are
     * getting stronger.
     *
     * Values up to 7 are reserved to add new memory types in the future.
     */
    MT_DEVICE,
    MT_NON_CACHEABLE,
    MT_MEMORY,

    /* Access permissions */
    MT_RO               = 0 << MT_PERM_SHIFT,
    MT_RW               = 1 << MT_PERM_SHIFT,

    /* Security state */
    MT_SECURE           = 0 << MT_SEC_SHIFT,
    MT_NS               = 1 << MT_SEC_SHIFT,

    /*
     * Access permissions for instruction execution
     *
     * Access permissions for instruction execution are only relevant for
     * normal read-only memory, i.e. MT_MEMORY | MT_RO. They are ignored
     * (and potentially overridden) otherwise:
     *  - Device memory is always marked as execute-never.
     *  - Read-write normal memory is always marked as execute-never.
     */
    MT_EXECUTE          = 0 << MT_EXECUTE_SHIFT,
    MT_EXECUTE_NEVER    = 1 << MT_EXECUTE_SHIFT,

} mmap_attr_t;

#define MT_CODE         (MT_MEMORY | MT_RO | MT_EXECUTE)
#define MT_RO_DATA      (MT_MEMORY | MT_RO | MT_EXECUTE_NEVER)
#define MT_RW_DATA      (MT_MEMORY | MT_RW | MT_EXECUTE_NEVER)
#define MT_SOC_REG      (MT_DEVICE | MT_RW | MT_EXECUTE_NEVER)

void mmap_add_region(
    uintptr_t paddr,
    uintptr_t vaddr,
    size_t size,
    uint64_t attr);

void mmap_add_sys_regions(ptrdiff_t load_link_offset);

#ifdef VERBOSE
void mmap_dump(void);
#endif

void mmap_populate_xlat_tables(void);
void enable_mmu(void);

#endif /* _MEMORY_H_ */
