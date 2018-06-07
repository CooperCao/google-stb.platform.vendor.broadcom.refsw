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

#include <string.h>
#include <arch.h>

#include "config.h"
#include "monitor.h"
#include "boot.h"
#include "memory.h"
#include "mmap.h"
#include "xlat.h"

extern uintptr_t _mon_img_start;
extern uintptr_t _mon_img_end;

extern uintptr_t _text_start;
extern uintptr_t _text_end;

extern uintptr_t _vectors_start;
extern uintptr_t _vectors_end;

extern uintptr_t _rodata_start;
extern uintptr_t _rodata_end;

extern uintptr_t _coherent_ram_start;
extern uintptr_t _coherent_ram_end;

#define MON_IMG_START ((uintptr_t)&_mon_img_start)
#define MON_IMG_SIZE  ((uintptr_t)&_mon_img_end - \
                       (uintptr_t)&_mon_img_start)

#define TEXT_START ((uintptr_t)&_text_start)
#define TEXT_SIZE  ((uintptr_t)&_text_end - \
                    (uintptr_t)&_text_start)

#define VECTORS_START ((uintptr_t)&_vectors_start)
#define VECTORS_SIZE  ((uintptr_t)&_vectors_end - \
                       (uintptr_t)&_vectors_start)

#define RODATA_START ((uintptr_t)&_rodata_start)
#define RODATA_SIZE  ((uintptr_t)&_rodata_end - \
                      (uintptr_t)&_rodata_start)

#define COHERENT_RAM_START ((uintptr_t)&_coherent_ram_start)
#define COHERENT_RAM_SIZE  ((uintptr_t)&_coherent_ram_end - \
                            (uintptr_t)&_coherent_ram_start)

static mmap_region_t mmap_regions[MAX_MMAP_REGIONS + 1];
static size_t mmap_regions_cnt;

static uintptr_t max_paddr;
static uintptr_t max_vaddr;

void mmap_reset_regions(void)
{
    /* Reset mmap regions */
    bootstrap_memset(mmap_regions, 0, sizeof(mmap_region_t) * mmap_regions_cnt);
    mmap_regions_cnt = 0;

    /* Reset max paddr and vaddr */
    max_paddr = 0;
    max_vaddr = 0;
}

void mmap_add_region(
    uintptr_t paddr,
    uintptr_t vaddr,
    size_t size,
    uint64_t attr)
{
    uintptr_t pend;
    uintptr_t vend;
    mmap_region_t *pmm;

    if (!size)
        return;

    /* Check for max mmap regions */
    DBG_ASSERT(mmap_regions_cnt < MAX_MMAP_REGIONS);

    /* Check for alignment */
    DBG_ASSERT(IS_PAGE_ALIGNED(paddr));
    DBG_ASSERT(IS_PAGE_ALIGNED(vaddr));

    /* Round size to next page */
    size = ROUND_TO_PAGE(size);

    pend = paddr + size - 1;
    vend = vaddr + size - 1;

    /* Check for overflow */
    DBG_ASSERT(paddr < pend);
    DBG_ASSERT(vaddr < vend);

#ifdef DEBUG
    /* Check for overlapping */
    for (pmm = mmap_regions; pmm->size; pmm++) {
        uintptr_t mm_paddr = pmm->paddr;
        uintptr_t mm_vaddr = pmm->vaddr;
        uintptr_t mm_vend  = pmm->vaddr + pmm->size - 1;

        /* Check for full overlapping */
        if (((mm_vaddr >= vaddr) && (mm_vend <= vend)) ||
            ((mm_vaddr <= vaddr) && (mm_vend >= vend))) {
            /* Allowed only if mapped with the same offset */
            DBG_ASSERT((mm_vaddr - mm_paddr) == (vaddr - paddr));
        }
        else {
            /* Check for partial overlapping */
            DBG_ASSERT((mm_vend < vaddr) || (mm_vaddr > vend));
        }
    }
#endif /* DEBUG */

    /* Find correct place to insert new region */
    for (pmm = mmap_regions; pmm->size; pmm++) {
        uintptr_t mm_vaddr = pmm->vaddr;
        uintptr_t mm_vend  = pmm->vaddr + pmm->size - 1;

        if (mm_vaddr > vaddr)
            break;
        /* If full overlapping with same vaddr */
        else if (mm_vaddr == vaddr) {
            /* If same size, do nothing */
            if (mm_vend == vend)
                return;
            /* If different size, small one goes last */
            else if (mm_vend < vend)
                break;
        }
    }

    /* Push back following regions by one place */
    if (pmm != &mmap_regions[mmap_regions_cnt])
        memmove(pmm + 1, pmm,
                (uintptr_t)&mmap_regions[mmap_regions_cnt] - (uintptr_t)pmm);

    /* Fill in current mapp region info */
    pmm->paddr = paddr;
    pmm->vaddr = vaddr;
    pmm->size  = size;
    pmm->attr  = attr;

    mmap_regions_cnt++;

    /* Update max paddr and vaddr */
    if (pend > max_paddr)
        max_paddr = pend;
    if (vend > max_vaddr)
        max_vaddr = vend;
}

void mmap_add_sys_regions(ptrdiff_t load_link_offset)
{
     /*
      * Add mmap regions:
      * - The entire monitor image is mapped as a whole first;
      * - The code, rodata, coherent ram sections are remapped with new attr.
      */
    mmap_add_region(
        MON_IMG_START,
        MON_IMG_START - load_link_offset,
        MON_IMG_SIZE,
        MT_RW_DATA | MT_SEC);

    mmap_add_region(
        TEXT_START,
        TEXT_START - load_link_offset,
        TEXT_SIZE,
        MT_CODE | MT_SEC);

    mmap_add_region(
        VECTORS_START,
        VECTORS_START - load_link_offset,
        VECTORS_SIZE,
        MT_CODE | MT_SEC);

    mmap_add_region(
        RODATA_START,
        RODATA_START - load_link_offset,
        RODATA_SIZE,
        MT_RO_DATA | MT_SEC);

    mmap_add_region(
        COHERENT_RAM_START,
        COHERENT_RAM_START - load_link_offset,
        COHERENT_RAM_SIZE,
        MT_DEVICE | MT_RW | MT_EXECUTE_NEVER | MT_SEC);
}

/*
 * Returns attributes of area at `vaddr` with size `size`. It returns the
 * attributes of the innermost region that contains it. If there are partial
 * overlaps, it returns -1, as a smaller size is needed.
 */
int mmap_get_attr(
    mmap_region_t *pmm,
    uintptr_t vaddr,
    size_t size)
{
    /* Don't assume that the area is contained in the first region */
    int attr = -1;

    /*
     * Get attributes from last (innermost) region that contains the
     * requested area. Don't stop as soon as one region doesn't contain it
     * because there may be other internal regions that contain this area:
     *
     * |-----------------------------1-----------------------------|
     * |----2----|     |-------3-------|    |----5----|
     *                   |--4--|
     *
     *                   |---| <- Area we want the attributes of.
     *
     * In this example, the area is contained in regions 1, 3 and 4 but not
     * in region 2. The loop shouldn't stop at region 2 as inner regions
     * have priority over outer regions, it should stop at region 5.
     */
    for (;; ++pmm) {

        if (!pmm->size)
            return attr; /* Reached end of list */

        if (pmm->vaddr >= vaddr + size)
            return attr; /* Next region is after area so end */

        if (pmm->vaddr + pmm->size <= vaddr)
            continue; /* Next region has already been overtaken */

        if ((int)pmm->attr == attr)
            continue; /* Region doesn't override attribs so skip */

        if (pmm->vaddr > vaddr ||
            pmm->vaddr + pmm->size < vaddr + size)
            return -1; /* Region doesn't fully cover our area */

        attr = (int)pmm->attr;
    }
}

#ifdef VERBOSE
static void mmap_dump(void)
{
    DBG_PRINT("\nmmap regions:\n");
    mmap_region_t *pmm = mmap_regions;
    for (pmm = mmap_regions; pmm->size; pmm++) {
        DBG_PRINT("  VA:%p  PA:%p  size:0x%zx  attr:0x%x\n",
                  (void *)pmm->vaddr,
                  (void *)pmm->paddr,
                  pmm->size,
                  pmm->attr);
    };
}
#endif

void mmap_populate_xlat_tables(void)
{
    DBG_ASSERT(max_paddr < PADDR_SIZE);
    DBG_ASSERT(max_vaddr < VADDR_SIZE);

#ifdef VERBOSE
    mmap_dump();
#endif

    /* Call to populate xlat tables recursively */
    xlat_init_tables(mmap_regions);
}

unsigned mmap_paddr_size_bits(void)
{
    /* Physical address can't exceed 48 bits */
    DBG_ASSERT((max_paddr & ADDR_MASK_48_TO_63) == 0);

    /* 48 bits address */
    if (max_paddr & ADDR_MASK_44_TO_47)
        return TCR_PS_BITS_256TB;

    /* 44 bits address */
    if (max_paddr & ADDR_MASK_42_TO_43)
        return TCR_PS_BITS_16TB;

    /* 42 bits address */
    if (max_paddr & ADDR_MASK_40_TO_41)
        return TCR_PS_BITS_4TB;

    /* 40 bits address */
    if (max_paddr & ADDR_MASK_36_TO_39)
        return TCR_PS_BITS_1TB;

    /* 36 bits address */
    if (max_paddr & ADDR_MASK_32_TO_35)
        return TCR_PS_BITS_64GB;

    return TCR_PS_BITS_4GB;
}
