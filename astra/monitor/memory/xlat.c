/******************************************************************************
 *  Copyright (C) 2018 Broadcom.
 *  The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to
 *  the terms and conditions of a separate, written license agreement executed
 *  between you and Broadcom (an "Authorized License").  Except as set forth in
 *  an Authorized License, Broadcom grants no license (express or implied),
 *  right to use, or waiver of any kind with respect to the Software, and
 *  Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 *  THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 *  IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization,
 *  constitutes the valuable trade secrets of Broadcom, and you shall use all
 *  reasonable efforts to protect the confidentiality thereof, and to use this
 *  information only in connection with your use of Broadcom integrated circuit
 *  products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 *  "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 *  OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 *  RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 *  IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 *  A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *  ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *  THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 *  OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 *  INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 *  RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 *  HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 *  EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 *  WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 *  FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ******************************************************************************/

#include <arch.h>
#include <arch_helpers.h>

#include "config.h"
#include "monitor.h"
#include "cache.h"
#include "memory.h"
#include "mmap.h"
#include "xlat.h"

#ifdef VERBOSE
#define SPACER_1 ""
#define SPACER_2 "  "
#define SPACER_3 "    "
#define LEVEL_SPACER(level)                     \
    ((level == 1) ? SPACER_1 :                  \
     (level == 2) ? SPACER_2 : SPACER_3)
#endif

static uint64_t xlat_tables[MAX_XLAT_TABLES][XLAT_TABLE_ENTRIES] __align_page __xlat_tables;
static size_t xlat_table_cnt;

static uint64_t xlat_build_desc(
    mmap_attr_t attr,
    uintptr_t paddr,
    int level)
{
    uint64_t desc;
    unsigned mem_type;

    desc = paddr;
    desc |= (level == 3) ? TABLE_DESC : BLOCK_DESC;
    desc |= (attr & MT_NSEC) ? LOWER_ATTRS(NS) : 0;
    desc |= (attr & MT_RW) ? LOWER_ATTRS(AP_RW) : LOWER_ATTRS(AP_RO);
    desc |= LOWER_ATTRS(ACCESS_FLAG);

    /*
     * Deduce shareability domain and executability of the memory region
     * from the memory type.
     *
     * Data accesses to device memory and non-cacheable normal memory are
     * coherent for all observers in the system, and correspondingly are
     * always treated as being Outer Shareable. Therefore, for these 2 types
     * of memory, it is not strictly needed to set the shareability field
     * in the translation tables.
     */
    mem_type = MT_TYPE(attr);
    if (mem_type == MT_DEVICE) {
        desc |= LOWER_ATTRS(ATTR_DEVICE_INDEX | OSH);
        /*
         * Always map device memory as execute-never.
         * This is to avoid the possibility of a speculative instruction
         * fetch, which could be an issue if this memory region
         * corresponds to a read-sensitive peripheral.
         */
        desc |= UPPER_ATTRS(XN);
    }
    else { /* Normal memory */
        /*
         * Always map read-write normal memory as execute-never.
         * (Trusted Firmware doesn't self-modify its code, therefore
         * R/W memory is reserved for data storage, which must not be
         * executable.)
         * Note that setting the XN bit here is for consistency only.
         * The enable_mmu() function sets the SCTLR_EL3.WXN bit,
         * which makes any writable memory region to be treated as
         * execute-never, regardless of the value of the XN bit in the
         * translation table.
         *
         * For read-only memory, rely on the MT_EXECUTE/MT_EXECUTE_NEVER
         * attribute to figure out the value of the XN bit.
         */
        if ((attr & MT_RW) || (attr & MT_EXECUTE_NEVER))
            desc |= UPPER_ATTRS(XN);

        if (mem_type == MT_MEMORY) {
            desc |= LOWER_ATTRS(ATTR_IWBWA_OWBWA_NTR_INDEX | ISH);
        }
        else {
            DBG_ASSERT(mem_type == MT_NON_CACHEABLE);
            desc |= LOWER_ATTRS(ATTR_NON_CACHEABLE_INDEX | OSH);
        }
    }

    VB_PRINT((mem_type == MT_MEMORY) ? "MEM" :
             (mem_type == MT_NON_CACHEABLE) ? "NC" : "DEV");
    VB_PRINT((attr & MT_RW) ? "-RW" : "-RO");
    VB_PRINT((attr & MT_NSEC) ? "-NS" : "-S");
    VB_PRINT((attr & MT_EXECUTE_NEVER) ? "-XN" : "-EXEC");
    return desc;
}

static mmap_region_t *xlat_init_table_inner(
    mmap_region_t *pmm,
    uintptr_t vaddr,
    uint64_t *table,
    int level)
{
    unsigned  level_size_shift = XLAT_L1_SIZE_SHIFT - (level - 1) * XLAT_TABLE_ENTRIES_SHIFT;
    uintptr_t level_size       = 1 << level_size_shift;
    uintptr_t level_index_mask = ((uintptr_t)XLAT_TABLE_ENTRIES_MASK) << level_size_shift;

    DBG_ASSERT(level > 0 && level <= 3);

    do  {
        uint64_t desc;

        if (!pmm->size) {
            /* Done mapping regions; finish zeroing the table */
            desc = INVALID_DESC;
        }
        else if (pmm->vaddr + pmm->size - 1 < vaddr) {
            /* This area is after the region so get next region */
            pmm++;
            continue;
        }
        else if (pmm->vaddr > vaddr + level_size - 1) {
            /* Next region is after this area. Nothing to map yet */
            desc = INVALID_DESC;
        }
        else {
            /* This area and the region overlap or contain one another */
            if (level == 3) {

                VB_PRINT("  %sVA:%p PA:%p size:0x%llx ", LEVEL_SPACER(level),
                         (void *)vaddr,
                         (void *)vaddr - pmm->vaddr + pmm->paddr,
                         (unsigned long long)level_size);

                /*
                 * Get attributes of this area on the block level.
                 * In the block level, overlapping means that this area
                 * is contained in the region. Getting attributes should
                 * always succeed.
                 */
                int attr = mmap_get_attr(pmm, vaddr, level_size);
                DBG_ASSERT(attr >= 0);

                /* Build block descriptor */
                desc = xlat_build_desc(
                    attr,
                    vaddr - pmm->vaddr + pmm->paddr,
                    level);

                VB_PRINT("\n");
            }
            else {
                /* Allocate a new table */
                uint64_t *new_table = xlat_tables[xlat_table_cnt++];
                DBG_ASSERT(xlat_table_cnt <= MAX_XLAT_TABLES);

                VB_PRINT("  %sVA:%p TB:%p size:0x%llx ", LEVEL_SPACER(level),
                         (void *)vaddr,
                         (void *)new_table,
                         (unsigned long long)level_size);

                /* Build table descriptor */
                desc = TABLE_DESC | (uintptr_t)new_table;

                VB_PRINT("\n");

                /* Recurse to fill in new table */
                pmm = xlat_init_table_inner(pmm, vaddr, new_table, level + 1);
            }
        }

        /* Go to next table entry */
        *table++ = desc;
        vaddr += level_size;

    } while (vaddr & level_index_mask);

    return pmm;
}

void xlat_init_tables(mmap_region_t *pmm)
{
    VB_PRINT("\nxlat tables:\n");

    /* Reset xlat tables */
    xlat_table_cnt = 0;

    /* Allocate L1 table */
    uint64_t *l1_table = xlat_tables[xlat_table_cnt++];

    /* Init xlat tables recursively */
    xlat_init_table_inner(pmm, 0, l1_table, 1);
}

__bootstrap void enable_mmu(void)
{
    uint64_t mair, tcr, ttbr;
    uint32_t sctlr;

    DBG_ASSERT(!(read_sctlr_el3() & SCTLR_M_BIT));

    DBG_MSG("Enabling MMU...");

    /* Set attributes in the right indices of the MAIR */
    mair  = MAIR_ATTR_SET(ATTR_DEVICE, ATTR_DEVICE_INDEX);
    mair |= MAIR_ATTR_SET(ATTR_IWBWA_OWBWA_NTR, ATTR_IWBWA_OWBWA_NTR_INDEX);
    mair |= MAIR_ATTR_SET(ATTR_NON_CACHEABLE, ATTR_NON_CACHEABLE_INDEX);
    write_mair_el3(mair);

    /* Invalidate TLBs at the current exception level */
    tlbialle3();

    /* Set TCR bits as well. */
    /* Inner & outer WBWA & shareable + T0SZ = 32 */
    tcr  = TCR_SH_INNER_SHAREABLE | TCR_RGN_OUTER_WBA | TCR_RGN_INNER_WBA;
    tcr |= TCR_EL3_RES1;
    tcr |= mmap_paddr_size_bits() << TCR_EL3_PS_SHIFT;
    tcr |= 64 - VADDR_SIZE_BITS;
    write_tcr_el3(tcr);

    /* Set TTBR bits as well */
    ttbr = (uint64_t)xlat_tables[0];
    write_ttbr0_el3(ttbr);

    /* Ensure all translation table writes have drained */
    /* into memory, the TLB invalidation is complete, */
    /* and translation register writes are committed */
    /* before enabling MMU */
    isb();
    dsb();

    sctlr  = read_sctlr_el3();
    sctlr |= SCTLR_M_BIT | SCTLR_C_BIT | SCTLR_I_BIT;
    write_sctlr_el3(sctlr);

    /* Ensure MMU enable takes effect immediately */
    isb();
}

__bootstrap void disable_mmu(void)
{
    uint32_t sctlr;

    DBG_ASSERT((read_sctlr_el3() & SCTLR_M_BIT));

    DBG_MSG("Disabling MMU...");

    /* Disable caches */
    sctlr  = read_sctlr_el3();
    sctlr &= ~(SCTLR_C_BIT | SCTLR_I_BIT);
    write_sctlr_el3(sctlr);

    /* Ensure caches disable takes effect immediately */
    isb();

    /* Clean data cache */
    dsb();
    dcsw_op_all(DCCSW);

    /* Ensure all data writes have completed, */
    /* and system register writes are committed */
    /* before disabling MMU */
    isb();
    dsb();

    /* Disable MMU */
    sctlr &= ~SCTLR_M_BIT;
    write_sctlr_el3(sctlr);

    /* Ensure MMU disable takes effect immediately */
    isb();

    /* Invalidate caches */
    dsb();
    iciallu();
    dcsw_op_all(DCISW);
}
