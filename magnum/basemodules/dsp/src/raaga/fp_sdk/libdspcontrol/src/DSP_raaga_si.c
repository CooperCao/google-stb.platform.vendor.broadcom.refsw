/****************************************************************************
 * Copyright (C) 2018 Broadcom.
 * The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to
 * the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied),
 * right to use, or waiver of any kind with respect to the Software, and
 * Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 * THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 * IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use all
 * reasonable efforts to protect the confidentiality thereof, and to use this
 * information only in connection with your use of Broadcom integrated circuit
 * products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 * OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 * RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 * IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 * A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 * ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 * THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 * OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 * INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 * RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 * HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 * EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 * WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 * FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
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

#include "libfp/src/c_utils_internal.h"

#include "DSP_raaga_mem_si.h"




DSP_RET DSP_copyToRegion(DSP *dsp __attribute__((unused)), DSP_RAAGA_MEM_REGION *region, const void *source)
{
    DSPLOG_JUNK("DSP: writeToRegion to_phys = %#x, to_soft = %p, size = %lu",
                region->phys_base, region->virt_base, region->size);

    BKNI_Memcpy(region->virt_base, source, region->size);
    BMMA_FlushCache(region->block_handle, region->virt_base, region->size);

    return DSP_SUCCESS;
}


DSP_RET DSP_copyFromRegion(DSP *dsp __attribute__((unused)), void *dest, DSP_RAAGA_MEM_REGION *region)
{
    BMMA_FlushCache(region->block_handle, region->virt_base, region->size);

    DSPLOG_JUNK("DSP: copyFromRegion from_phys = %#x, from_soft = %p, size = %lu",
                region->phys_base, region->virt_base, region->size);

    BKNI_Memcpy(dest, region->virt_base, region->size);

    return DSP_SUCCESS;
}


DSP_RET DSP_writeSystemData(DSP *dsp, uint32_t phys_dest, const void *source, size_t length)
{
    const uint8_t *src_b = (const uint8_t *) source;

    DSPLOG_JUNK("DSP: writeSystemData to_phys = %#x, size = %zu", phys_dest, length);

    while(length > 0)
    {
        DSP_RET ret;
        DSP_RAAGA_MEM_REGION region;

        ret = DSP_raagaFindExistingRegionByPhysical(dsp, &region, phys_dest, length);
        if(DSP_FAILED(ret))
            return ret;

        ret = DSP_copyToRegion(dsp, &region, src_b);
        if(DSP_FAILED(ret))
            return ret;

        src_b += region.size;
        phys_dest += region.size;
        length -= region.size;
    }

    return DSP_SUCCESS;
}


DSP_RET DSP_readSystemData(DSP *dsp, void *dest, uint32_t phys_source, size_t length)
{
    uint8_t *dest_b = (uint8_t *) dest;

    DSPLOG_JUNK("DSP: readSystemData from_phys = %#x, size = %zu", phys_source, length);

    while(length > 0)
    {
        DSP_RET ret;
        DSP_RAAGA_MEM_REGION region;

        ret = DSP_raagaFindExistingRegionByPhysical(dsp, &region, phys_source, length);
        if(DSP_FAILED(ret))
            return ret;

        ret = DSP_copyFromRegion(dsp, dest_b, &region);
        if(DSP_FAILED(ret))
            return ret;

        phys_source += region.size;
        dest_b += region.size;
        length -= region.size;
    }

    return DSP_SUCCESS;
}


void DSP_writeSharedRegister(DSP *dsp, uint32_t reg_addr, uint32_t val)
{
    DSPLOG_JUNK("DSP: writeSharedRegister address = %#x, data = %#x", reg_addr, val);
    BREG_Write32(dsp->reg, reg_addr, val);
}


uint32_t DSP_readSharedRegister(DSP *dsp, uint32_t reg_addr)
{
    uint32_t val = BREG_Read32(dsp->reg, reg_addr);
    DSPLOG_JUNK("DSP: readSharedRegister address = %#x, data = %#x", reg_addr, val);

    return val;
}


DSP_RET DSP_readSharedData(DSP *dsp, void *dest, uint32_t src, size_t len)
{
    if(!IS_MULTIPLE_OF_POW2(src, 4))
    {
        DSPLOG_ERROR("DSP_readSharedData: address must be 32 bit-aligned");
        return DSP_BAD_ADDRESS_ALIGNMENT;
    }

    if(!IS_MULTIPLE_OF_POW2(len, 4))
    {
        DSPLOG_ERROR("DSP_readSharedData: length must be a whole number of 32 bit words");
        return DSP_BAD_SIZE;
    }

    DSPLOG_JUNK("DSP: readSharedData address = %#x, length = %lu", src, len);
    {
        unsigned i;
        for(i = 0; i < len; i += 4)
            *((uint32_t *) ((char *) dest + i)) = BREG_Read32(dsp->reg, src + i);
    }

    return DSP_SUCCESS;
}


DSP_RET DSP_writeSharedData(DSP *dsp, uint32_t dest, void *src, size_t len)
{
    if(!IS_MULTIPLE_OF_POW2(dest, 4))
    {
        DSPLOG_ERROR("DSP_writeSharedData: address must be 32 bit-aligned");
        return DSP_BAD_ADDRESS_ALIGNMENT;
    }

    if(!IS_MULTIPLE_OF_POW2(len, 4))
    {
        DSPLOG_ERROR("DSP_writeSharedData: length must be a whole number of 32 bit words");
        return DSP_BAD_SIZE;
    }

    DSPLOG_JUNK("DSP: writeSharedData address = %#x, length = %lu", dest, len);
    {
        unsigned i;
        for(i = 0; i < len; i += 4)
            BREG_Write32(dsp->reg, dest + i, *((uint32_t *) ((char *) src + i)));
    }

    return DSP_SUCCESS;
}
