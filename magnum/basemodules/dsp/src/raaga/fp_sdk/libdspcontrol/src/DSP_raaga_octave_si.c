/****************************************************************************
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
 ****************************************************************************/

/* NOTE: this file gets exported into the Raaga Magnum host library and so it
 *       must abide by a specific strict set of rules. Please use only ANSI C
 *       constructs and include only FPSDK headers which do as well. After any
 *       edit, always make sure that the Raaga Magnum build succeeds without
 *       any warning.
 */

#include "libdspcontrol/CHIP.h"

#include "fp_sdk_config.h"

#include "libfp/c_utils.h"

#include "libdspcontrol/DSP.h"
#include "libdspcontrol/DSPLOG.h"

#include "DSP_octave_maestro.h"
#include "DSP_raaga_octave.h"
#include "DSP_raaga_octave_atu.h"
#include "DSP_raaga_mem_si.h"
#include "DSP_raaga_si.h"
#include "UTIL_assert.h"

#include "libfp/src/c_utils_internal.h"

#include "bchp_raaga_dsp_misc.h"



#if !IS_HOST(SILICON)
#  error "This module is for silicon targets only"
#endif
#if !defined(RAAGA)
#  error "This module is only for Raaga"
#endif
#if FEATURE_IS(ENDIANESS, BIG)
#  error "Big endian platforms not properly supported at the moment"
#endif


/* Workaround for the missing inttypes.h */
#define PRIu32      "u"
#define PRIx32      "x"


/* ================
 * Public functions
 * ================ */
DSP_RET DSP_init(DSP *dsp, DSP_PARAMETERS *parameters)
{
    DSP_RET ret;

    /* Keep a copy of provided handles and initialise mem management structures */
    dsp->reg = parameters->hReg;
    dsp->heap = parameters->hMem;

    ret = DSP_raagaMemInit(dsp, parameters);
    if(DSP_FAILED(ret))
        return ret;

    /* Cache the ATU configuration */
    DSP_readAtu(dsp, &(dsp->atu_cache));
    DSP_createAtuIndex(&(dsp->atu_cache), &(dsp->atu_index));

    return DSP_SUCCESS;
}


void DSP_finish(DSP *dsp __unused)
{
}


DSP_RET DSP_reset(DSP *dsp)
{
    unsigned core;
    uint32_t reg_val;

    DSPLOG_INFO("DSP: resetting");

    /* Disable all cores and reset the L2 $ */
    for(core = 0; core < NUM_CORES; core++)
        DSP_writeSharedRegister(dsp, MISC_BLOCK(dsp, core, CORECTRL_CORE_ENABLE), 0);

    reg_val = DSP_readSharedRegister(dsp, BCHP_RAAGA_DSP_MISC_SOFT_INIT);
    reg_val = (reg_val & ~(BCHP_MASK(RAAGA_DSP_MISC_SOFT_INIT, SW_INIT_L2C)))
              | (BCHP_FIELD_DATA(RAAGA_DSP_MISC_SOFT_INIT, SW_INIT_L2C, 0));
    DSP_writeSharedRegister(dsp, BCHP_RAAGA_DSP_MISC_SOFT_INIT, reg_val);

    /* Regenerate the ATU configuration cache */
    DSP_readAtu(dsp, &(dsp->atu_cache));
    DSP_createAtuIndex(&(dsp->atu_cache), &(dsp->atu_index));

    return DSP_SUCCESS;
}


DSP_RET DSP_writeDspData(DSP *dsp, DSP_ADDR dest, const void *src, size_t length)
{
    const uint8_t *src_b = (const uint8_t *) src;

    while(length > 0)
    {
        DSP_RET ret;
        uint32_t to_write = (uint32_t) length;
        uint64_t system_addr;

        ret = DSP_atuVirtualToPhysical(&(dsp->atu_index), dest, &system_addr, &to_write);
        if(DSP_FAILED(ret))
            return ret;

        if(system_addr > UINT32_MAX)
            return DSP_BAD_ADDRESS_RANGE;

        ret = DSP_writeSystemData(dsp, (uint32_t) system_addr, src_b, to_write);
        if(DSP_FAILED(ret))
            return ret;

        src_b += to_write;
        dest += to_write;
        length -= to_write;
    }

    return DSP_SUCCESS;
}


DSP_RET DSP_readDspData(DSP *dsp, void *dest, DSP_ADDR src, size_t length)
{
    uint8_t *dest_b = (uint8_t *) dest;

    while(length > 0)
    {
        DSP_RET ret;
        uint32_t to_read = (uint32_t) length;
        uint64_t system_addr;

        ret = DSP_atuVirtualToPhysical(&(dsp->atu_index), src, &system_addr, &to_read);
        if(DSP_FAILED(ret))
            return ret;

        if(system_addr > UINT32_MAX)
            return DSP_BAD_ADDRESS_RANGE;

        ret = DSP_readSystemData(dsp, dest_b, (uint32_t) system_addr, to_read);
        if(DSP_FAILED(ret))
            return ret;

        src += to_read;
        dest_b += to_read;
        length -= to_read;
    }

    return DSP_SUCCESS;
}


DSP_RET DSP_writeDataAsDsp(DSP *dsp, DSP_ADDR dest, void *src, size_t n)
{
    return DSP_writeDspData(dsp, dest, src, n);
}


DSP_RET DSP_readDataAsDsp(DSP *dsp, void *dest, DSP_ADDR src, size_t n)
{
    return DSP_readDspData(dsp, dest, src, n);
}


DSP_RET DSP_addrDsp2System(DSP *dsp, DSP_ADDR dsp_addr, uint32_t *sys_addr)
{
    uint64_t system_addr;
    DSP_RET ret;

    ret = DSP_atuVirtualToPhysical(&(dsp->atu_index), dsp_addr, &system_addr, NULL);

    if(DSP_FAILED(ret))
        return DSP_FAILURE;
    if(system_addr > UINT32_MAX)
        return DSP_BAD_ADDRESS_RANGE;

    *sys_addr = (uint32_t) system_addr;

    return DSP_SUCCESS;
}


DSP_RET DSP_addrSystem2Dsp(DSP *dsp, uint32_t sys_addr, DSP_ADDR *dsp_addr)
{
    DSP_RET ret;

    ret = DSP_atuPhysicalToVirtual(&(dsp->atu_index), sys_addr, dsp_addr, NULL);

    if(DSP_FAILED(ret))
        return DSP_FAILURE;

    return DSP_SUCCESS;
}


DSP_RET DSP_setOption(DSP *dsp __unused, DSP_OPTION option, void *value __unused)
{
    switch(option)
    {
    case DSP_OPTION_REFRESH_ATU_INDEX:
        /* Refresh the ATU configuration cache */
        DSP_readAtu(dsp, &(dsp->atu_cache));
        DSP_createAtuIndex(&(dsp->atu_cache), &(dsp->atu_index));
        return DSP_SUCCESS;

    default:
        return DSP_UNKNOW_OPTION;
    }
}


DSP_RET DSP_getOption(DSP *dsp __unused, DSP_OPTION option, void *value __unused)
{
    switch(option)
    {
    case DSP_OPTION_REFRESH_ATU_INDEX:
        return DSP_SUCCESS;

    default:
        return DSP_UNKNOW_OPTION;
    }
}


uint64_t DSP_readShared64BitRegister(DSP *dsp, uint32_t reg_addr)
{
    uint64_t val = BREG_Read64(dsp->reg, reg_addr);
    DSPLOG_JUNK("DSP: readShared64BitRegister address = %#x, data = %#llx", reg_addr, val);
    return val;
}


void DSP_writeShared64BitRegister(DSP *dsp, uint32_t reg_addr, uint64_t value)
{
    DSPLOG_JUNK("DSP: writeShared64BitRegister address = %#x, data = %#llx", reg_addr, value);
    BREG_Write64(dsp->reg, reg_addr, value);
}
