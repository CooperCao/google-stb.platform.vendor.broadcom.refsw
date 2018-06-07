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

#include <errno.h>
#include <inttypes.h>
#include <string.h>
#include <stdint.h>

#include "sysctlpxy.h"

#include "libdspcontrol/CHIP.h"
#include "libfp/c_utils.h"

#include "libdspcontrol/DSP.h"
#include "libdspcontrol/DSPLOG.h"

#include "libfp/src/c_utils_internal.h"

#include "DSP_octave_maestro.h"
#include "DSP_raaga_octave.h"
#include "DSP_raaga_octave_atu.h"
#include "UTIL_assert.h"



/* ================
 * Public functions
 * ================ */
DSP_RET DSP_init(DSP *dsp, DSP_PARAMETERS *parameters)
{
    int rv = 0;
    if(parameters->port == 0)
    {
        DSPLOG_ERROR("DSP: no BM SCP port specified");
        return DSP_FAILURE;
    }

    rv = sysctlpxy_init();
    if(!rv)
    {
        DSPLOG_DETAIL("DSP: connecting to SCP server at %s:%d", "localhost", parameters->port);
        rv = sysctlpxy_connect("localhost", parameters->port);
    }

    if(rv)
    {
        DSPLOG_ERROR("DSP: could not connect to SCP server, error=\"%s\"", strerror(errno));
        return DSP_FAILURE;
    }

    /* Cache the ATU configuration */
    DSP_readAtu(dsp, &(dsp->atu_cache));
    DSP_createAtuIndex(&(dsp->atu_cache), &(dsp->atu_index));

    return DSP_SUCCESS;
}


void DSP_finish(DSP *dsp __unused)
{
    sysctlpxy_disconnect();
    sysctlpxy_delete();
}


DSP_RET DSP_reset(DSP *dsp)
{
    unsigned core;
    DSP_CORES_BITMAP cores;

    DSPLOG_DETAIL("DSP: resetting");

    if(sysctlpxy_reset_chip())  /* a return value of 0 means OK */
        return DSP_FAILURE;

    /* check if any core is running */
    DSP_enabledStatus(dsp, &cores);
    if(cores != DSP_CORES_NONE)
    {
        DSPLOG_ERROR("DSP: after reset some cores are still running");
        return DSP_FAILURE;
    }

    /* perform some extra checks to make sure the subsystem behaves as expected */
    for(core = 0; core < NUM_CORES; core++)
    {
        uint32_t mutex;

        /* FLAGS_SYS_FLG0 must have been reset to 0 */
        if(DSP_readSharedRegister(dsp, MISC_BLOCK(dsp, core, CORECTRL_SYS_FLG0_STATUS)))
            FATAL_ERROR("DSP: FLAGS_SYS_FLG0 is not clear after reset");

        /* lock the mutexes, they should be all free */
        for(mutex = MISC_BLOCK(dsp, core, CORESTATE_SYS_MTX0); mutex <= MISC_BLOCK(dsp, core, CORESTATE_SYS_MTX3); mutex += 4)
            if(0 == DSP_readSharedRegister(dsp, mutex))
                FATAL_ERROR("DSP: mutex MUTEXES_SYS_MTX%d was not unlocked after reset", (mutex - MISC_BLOCK(dsp, core, CORESTATE_SYS_MTX0)) / 4);
        for(mutex = MISC_BLOCK(dsp, core, CORESTATE_USR_MTX0); mutex <= MISC_BLOCK(dsp, core, CORESTATE_USR_MTX7); mutex += 4)
            if(0 == DSP_readSharedRegister(dsp, mutex))
                FATAL_ERROR("DSP: mutex MUTEXES_USR_MTX%d was not unlocked after reset", (mutex - MISC_BLOCK(dsp, core, CORESTATE_USR_MTX0)) / 4);
        /* now free them */
        for(mutex = MISC_BLOCK(dsp, core, CORESTATE_SYS_MTX0); mutex <= MISC_BLOCK(dsp, core, CORESTATE_USR_MTX7); mutex += 4)
            DSP_writeSharedRegister(dsp, mutex, 1);
    }

    /* Regenerate the ATU configuration cache */
    DSP_readAtu(dsp, &(dsp->atu_cache));
    DSP_createAtuIndex(&(dsp->atu_cache), &(dsp->atu_index));

    return DSP_SUCCESS;
}


uint32_t DSP_writeSystemData(DSP *dsp __unused, uint32_t dest, const void *src, size_t n)
{
    DSPLOG_JUNK("DSP: writing data dst=0x%08" PRIx32 " len=0x%zx", dest, n);

    return sysctlpxy_write_memory_block(dest, n, (const uint8_t *) src) == 0 ? DSP_SUCCESS : DSP_FAILURE;
}


uint32_t DSP_readSystemData(DSP *dsp __unused, void *dest, uint32_t src, size_t n)
{
    DSPLOG_JUNK("DSP: reading data src=0x%08" PRIx32 " len=0x%zx", src, n);
    if(n == 0)
        return DSP_SUCCESS;

    return sysctlpxy_read_memory_block(src, n, (uint8_t *) dest) == 0 ? DSP_SUCCESS : DSP_FAILURE;
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


DSP_RET DSP_writeDspData(DSP *dsp, DSP_ADDR dest, const void *src, size_t length)
{
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

        ret = DSP_writeSystemData(dsp, (uint32_t) system_addr, src, to_write);
        if(DSP_FAILED(ret))
            return ret;

        src += to_write;
        dest += to_write;
        length -= to_write;
    }

    return DSP_SUCCESS;
}


DSP_RET DSP_readDspData(DSP *dsp, void *dest, DSP_ADDR src, size_t length)
{
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

        ret = DSP_readSystemData(dsp, dest, (uint32_t) system_addr, to_read);
        if(DSP_FAILED(ret))
            return ret;

        src += to_read;
        dest += to_read;
        length -= to_read;
    }

    return DSP_SUCCESS;
}


DSP_RET DSP_writeSharedData(DSP *dsp __unused, uint32_t dest, void *src, size_t n)
{
    DSPLOG_JUNK("DSP: writing shared data dst=0x%08" PRIx32 " len=0x%zx", dest, n);
    if(!IS_MULTIPLE_OF_POW2(dest, 4))
    {
        DSPLOG_ERROR("DSP: destination address not word-aligned");
        return DSP_BAD_ADDRESS_ALIGNMENT;
    }
    if(!IS_MULTIPLE_OF_POW2(n, 4))
    {
        DSPLOG_ERROR("DSP: transfer length not a word multiple");
        return DSP_BAD_SIZE;
    }

    return sysctlpxy_write_memory_block(dest, n, (const uint8_t *) src) == 0 ? DSP_SUCCESS : DSP_FAILURE;
}


DSP_RET DSP_readSharedData(DSP *dsp __unused, void *dest, uint32_t src, size_t n)
{
    DSPLOG_JUNK("DSP: reading shared data src=0x%08"PRIx32" len=0x%zx", src, n);
    if(!IS_MULTIPLE_OF_POW2(src, 4))
    {
        DSPLOG_ERROR("DSP: source address not word-aligned");
        return DSP_BAD_ADDRESS_ALIGNMENT;
    }
    if(!IS_MULTIPLE_OF_POW2(n, 4))
    {
        DSPLOG_ERROR("DSP: transfer length not a word multiple");
        return DSP_BAD_SIZE;
    }

    return sysctlpxy_read_memory_block(src, n, (uint8_t *) dest) == 0 ? DSP_SUCCESS : DSP_FAILURE;
}


void DSP_writeSharedRegister(DSP *dsp, uint32_t reg_addr, uint32_t value)
{
    DSPLOG_JUNK("DSP: write shared register reg=%#010" PRIx32 " value=%#010x", reg_addr, value);
    DSP_writeSharedData(dsp, reg_addr, &value, sizeof(value));
}


uint32_t DSP_readSharedRegister(DSP *dsp, uint32_t reg_addr)
{
    uint32_t value;
    DSP_readSharedData(dsp, &value, reg_addr, sizeof(reg_addr));
    DSPLOG_JUNK("DSP: read shared register reg=%#010" PRIx32 " value=%#010x", reg_addr, value);

    return value;
}


DSP_RET DSP_writeDataAsDsp(DSP *dsp, DSP_ADDR dest, void *src, size_t n)
{
    return DSP_writeDspData(dsp, dest, src, n);
}


DSP_RET DSP_readDataAsDsp(DSP *dsp, void *dest, DSP_ADDR src, size_t n)
{
    return DSP_readDspData(dsp, dest, src, n);
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
    return (uint64_t) DSP_readSharedRegister(dsp, reg_addr) |
           (((uint64_t) DSP_readSharedRegister(dsp, reg_addr + 4)) << 32);
}


void DSP_writeShared64BitRegister(DSP *dsp, uint32_t reg_addr, uint64_t value)
{
    DSP_writeSharedRegister(dsp, reg_addr + 4, (uint32_t) (value >> 32));
    DSP_writeSharedRegister(dsp, reg_addr, (uint32_t) value);
}


/*
 * Memory layout.
 */
const DSP_MEMORY_LAYOUT *DSP_getMemoryLayout(DSP *dsp __unused)
{
    static const DSP_MEMORY_AREA areas[0] = {};  /* no well defined memory areas yet */
    static DSP_MEMORY_LAYOUT layout =
    {
        sizeof (areas) / sizeof (DSP_MEMORY_AREA),
        areas
    };

    return &layout;
}
