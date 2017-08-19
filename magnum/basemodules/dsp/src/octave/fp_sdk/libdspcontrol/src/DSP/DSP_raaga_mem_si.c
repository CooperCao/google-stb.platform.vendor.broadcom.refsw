/****************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 ****************************************************************************/

/* NOTE: this file gets exported into the Raaga Magnum host library and so it
 *       must abide by a specific strict set of rules. Please use only ANSI C
 *       constructs and include only FPSDK headers which do as well. After any
 *       edit, always make sure that the Raaga Magnum build succeeds without
 *       any warning.
 */

#include "libdspcontrol/CHIP.h"

#include "fp_sdk_config.h"

#include "libdspcontrol/DSP.h"
#include "libdspcontrol/DSPLOG.h"

#include "bstd.h"
#include "bstd_defs.h"


/* Workaround for the missing inttypes.h */
#define PRIx64      "llx"



DSP_RET DSP_raagaMemInit(DSP *dsp, DSP_PARAMETERS *parameters)
{
    dsp->mem_regions_count = parameters->ui32MmaBufferValidEntries;
    if(dsp->mem_regions_count > DSP_RAAGA_MEM_REGIONS_NUM_ENTRIES)
        return DSP_FAILURE;

    {
        unsigned i;
        for(i = 0; i < dsp->mem_regions_count; i++)
        {
            dsp->mem_regions[i].block_handle = parameters->sMmaBuffer[i].Buffer.hBlock;
            dsp->mem_regions[i].virt_base = parameters->sMmaBuffer[i].Buffer.pAddr;
            dsp->mem_regions[i].phys_base = parameters->sMmaBuffer[i].Buffer.offset;
            dsp->mem_regions[i].size = parameters->sMmaBuffer[i].ui32Size;
        }
    }

    return DSP_SUCCESS;
}


DSP_RET DSP_raagaFindExistingRegionByPhysical(DSP *dsp, DSP_RAAGA_MEM_REGION *found, uint64_t phys_addr, size_t length)
{
    unsigned i;
    for(i = 0; i < dsp->mem_regions_count; i++)
    {
        const DSP_RAAGA_MEM_REGION *region = &(dsp->mem_regions[i]);
        if(phys_addr >= region->phys_base &&
           phys_addr < region->phys_base + region->size)
        {
            uint64_t phys_offset = phys_addr - region->phys_base;
            uint64_t remaining_in_region = region->size - phys_offset;

            found->block_handle = region->block_handle;
            found->phys_base = phys_addr;
            found->virt_base = (void *)(uintptr_t) ((uintptr_t) region->virt_base + phys_offset);
            if(remaining_in_region < length)
                found->size = remaining_in_region;
            else
                found->size = length;

            return DSP_SUCCESS;
        }
    }

    return DSP_FAILURE;
}


/* NOTE: there exist the BDSP_MMA_P_AllocateAlignedMemory / BDSP_MMA_P_FreeMemory
 * pair of wrappers that would do the exact same malloc / free job for us. We
 * don't use them because they require an intermediate BDSP_MMA_Memory structure
 * to operate on, and we risk missing something in the future when manually
 * copying fields from/to DSP_RAAGA_MEM_REGIONs. It is safer to assume, IMHO,
 * that the BMMA_* API semantic will remain stable. */

DSP_RET DSP_raagaMemAlloc(DSP *dsp, DSP_RAAGA_MEM_REGION *region, size_t size, unsigned alignment)
{
    region->block_handle = BMMA_Alloc(dsp->heap, size, alignment, NULL);
    if(region->block_handle == NULL)
    {
        DSPLOG_ERROR("DSP: BMMA_Alloc of %zu bytes, %u alignment, failed", size, alignment);
        return DSP_FAILURE;
    }

    region->virt_base = BMMA_Lock(region->block_handle);
    if(region->virt_base == NULL)
    {
        BMMA_Free(region->block_handle);
        region->block_handle = NULL;

        DSPLOG_ERROR("DSP: BMMA_Lock failed");
        return DSP_FAILURE;
    }

    region->phys_base = BMMA_LockOffset(region->block_handle);
    if(region->phys_base == 0)
    {
        BMMA_Unlock(region->block_handle, region->virt_base);
        region->virt_base = NULL;
        BMMA_Free(region->block_handle);
        region->block_handle = NULL;

        DSPLOG_ERROR("DSP: BMMA_LockOffset failed");
        return DSP_FAILURE;
    }

    DSPLOG_JUNK("DSP: allocated %zu bytes virtual = 0x%p, physical = 0x%" PRIx64,
                size, region->virt_base, region->phys_base);

    return DSP_SUCCESS;
}


DSP_RET DSP_raagaMemFree(DSP *dsp __attribute__((unused)), DSP_RAAGA_MEM_REGION *region)
{
    if(region->block_handle)
    {
        DSPLOG_JUNK("DSP: freeing memory block at virtual = 0x%p, physical = 0x%" PRIx64,
                    region->virt_base, region->phys_base);

        if(region->phys_base)
        {
            BMMA_UnlockOffset(region->block_handle, region->phys_base);
            region->phys_base = 0;
        }

        if(region->virt_base)
        {
            BMMA_Unlock(region->block_handle, region->virt_base);
            region->virt_base = NULL;
        }

        BMMA_Free(region->block_handle);
        region->block_handle = NULL;
    }

    return DSP_SUCCESS;
}
