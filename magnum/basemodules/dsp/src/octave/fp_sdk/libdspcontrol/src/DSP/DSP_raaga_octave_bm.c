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
#include "../UTIL_assert.h"



static __unused
void compile_time_assertions(void)
{
    /* Addresses width assertion */
    COMPILE_TIME_ASSERT(sizeof(SYSTEM_ADDR) == 4);
    COMPILE_TIME_ASSERT(sizeof(SHARED_ADDR) == 4);
    COMPILE_TIME_ASSERT(sizeof(DSP_ADDR)    == 4);
    COMPILE_TIME_ASSERT(sizeof(SYSTEM_ADDR_SIZE) == 4);
    COMPILE_TIME_ASSERT(sizeof(SHARED_ADDR_SIZE) == 4);
    COMPILE_TIME_ASSERT(sizeof(DSP_ADDR_SIZE)    == 4);
}


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
    DSP_fetchAtuRevision(dsp);
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


uint32_t DSP_writeSystemData(DSP *dsp __unused, SYSTEM_ADDR dest, const void *src, SYSTEM_ADDR_SIZE n)
{
    DSPLOG_JUNK("DSP: writing data dst=0x%08" PRIx32 " len=0x%" PRIx32, dest, n);

    return sysctlpxy_write_memory_block(dest, n, (const uint8_t *) src) == 0 ? DSP_SUCCESS : DSP_FAILURE;
}


uint32_t DSP_readSystemData(DSP *dsp __unused, void *dest, SYSTEM_ADDR src, SYSTEM_ADDR_SIZE n)
{
    DSPLOG_JUNK("DSP: reading data src=0x%08" PRIx32 " len=0x%" PRIx32, src, n);
    if(n == 0)
        return DSP_SUCCESS;

    return sysctlpxy_read_memory_block(src, n, (uint8_t *) dest) == 0 ? DSP_SUCCESS : DSP_FAILURE;
}


DSP_RET DSP_writeSharedData(DSP *dsp __unused, SHARED_ADDR dest, const void *src, SHARED_ADDR_SIZE n)
{
    DSPLOG_JUNK("DSP: writing shared data dst=0x%08" PRIx32 " len=0x%" PRIx32, dest, n);
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


DSP_RET DSP_readSharedData(DSP *dsp __unused, void *dest, SHARED_ADDR src, SHARED_ADDR_SIZE n)
{
    DSPLOG_JUNK("DSP: reading shared data src=0x%08" PRIx32 " len=0x%" PRIx32, src, n);
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


void DSP_writeSharedRegister(DSP *dsp, SHARED_ADDR reg_addr, uint32_t value)
{
    DSPLOG_JUNK("DSP: write shared register reg=0x%08" PRIx32 " value=0x%08" PRIx32, reg_addr, value);
    DSP_writeSharedData(dsp, reg_addr, &value, sizeof(value));
}


uint32_t DSP_readSharedRegister(DSP *dsp, SHARED_ADDR reg_addr)
{
    uint32_t value;
    DSP_readSharedData(dsp, &value, reg_addr, sizeof(reg_addr));
    DSPLOG_JUNK("DSP: read shared register reg=0x%08" PRIx32 " value=0x%08" PRIx32, reg_addr, value);

    return value;
}


uint64_t DSP_readShared64BitRegister(DSP *dsp, SHARED_ADDR reg_addr)
{
    return (uint64_t) DSP_readSharedRegister(dsp, reg_addr) |
           (((uint64_t) DSP_readSharedRegister(dsp, reg_addr + 4)) << 32);
}


void DSP_writeShared64BitRegister(DSP *dsp, SHARED_ADDR reg_addr, uint64_t value)
{
    DSP_writeSharedRegister(dsp, reg_addr + 4, (uint32_t) (value >> 32));
    DSP_writeSharedRegister(dsp, reg_addr, (uint32_t) value);
}
