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

/**
 * @file DSP_bm.c
 *
 * Implementation of the DSP control APIs for BM targets.
 *
 * In the BM, for historical reasons coming from the DSL world, we currently
 * use a single flat address space. The following conventions apply:
 * * DSP addresses are mapped at their original location (so normally 0x00000000
 *   for imem an 0x40000000 for dmem).
 * * Host CPU dram is remapped at high addresses (see DDR_START in DSP_bm.h).
 * * Shared memories are mapped at their original address, unless they clash
 *   with the DSP address space.
 * For this reason most of the DSP_read/write* functions below are merely wrappers
 * around the DSP_read/writeSystemData functions.
 */

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "sysctlpxy.h"

#include "libdspcontrol/CHIP.h"
#include "libdspcontrol/COMMON.h"
#include "libdspcontrol/DSP.h"
#include "libdspcontrol/DSPLOG.h"

#include "DSP_bm.h"



#if !IS_HOST(BM)
#  error "This module is for BM targets only"
#endif
#if IS_HOST(DSP_LESS)
#  error "This module is not suitable for DSP-less builds"
#endif


/*
 * Define the below macro to be extra verbose and dump memory transaction
 * on DSP_read* / DSP_write* calls.
 */
#undef DSP_BM_DUMP_DATA_ON_MEMORY_ACCESS


DSP_RET DSP_init(DSP *dsp __unused, DSP_PARAMETERS *parameters)
{
    if(parameters->port == 0)
    {
        DSPLOG_ERROR("DSP: no BM SCP port specified");
        return DSP_FAILURE;
    }

    int rv = sysctlpxy_init();
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

#if defined(__FP2012__)
    dsp->debug_console_initialised = false;
#endif

    return DSP_SUCCESS;
}


void DSP_finish(DSP *dsp __unused)
{
    sysctlpxy_disconnect();
    sysctlpxy_delete();
}


DSP_RET DSP_reset(DSP *dsp)
{
    if(sysctlpxy_reset_chip())  /*  a return value of 0 means OK */
        return DSP_FAILURE;

    /* Theoretically at this point the core(s) should be stopped.
     * In reality the core could be configured with restart=yes
     * and be already running again. So we check with DSP_enabledStatus
     * (which is currently broken, by the way.....) */
    DSP_CORES_BITMAP cores;
    DSP_enabledStatus(dsp, &cores);

    return cores == 0 ? DSP_SUCCESS : DSP_FAILURE;
}


DSP_RET DSP_enable(DSP *dsp __unused, DSP_CORE core)
{
    DSPLOG_INFO("DSP: enabling DSP core %d", core);
    if(sysctlpxy_start_core(core))  /*  a return value of 0 means OK */
        return DSP_FAILURE;
    else
        return DSP_SUCCESS;
}


DSP_RET DSP_enabledStatus(DSP *dsp __unused,
                          DSP_CORES_BITMAP *cores __unused)
{
    /*  not correctly supported by the BM */
    *cores = DSP_CORES_NONE;
    return DSP_NOT_SUPPORTED;

/*    for(unsigned i = 0; i < NUM_CORES; i++)
 *    {
 *        uint8_t core_status;
 *        if(0 == sysctlpxy_core_status(i, &core_status))
 *            cores |= (core_status & 1) << i;
 *    }
 */
}


void fill_fake_dram (DSP_DRAM_MEMORY_AREA *area, size_t size,
                     size_t base, size_t size_available, const char* user)
{
    /* Make use of the fact that static objects are initialised to zero so we
     * know we've been called before iff pAddress is non-zero. */
    if (area->pAddress)
    {
        /* If we've been called before and we haven't enough space, complain and
         * die. */
        if (area->length < size)
            FATAL_ERROR (
                "DSP_bm: fill_fake_dram: Exists, but too small for %s", user);

        /*  Otherwise, nothing to do. */
    }

    if (size > size_available)
        FATAL_ERROR (
            "DSP_bm: can't allocate %#zx bytes of space for %s", size, user);

    area->pAddress   = (void *) base;
    area->vAddress   = (void *) base;
    area->dspAddress = base;
    area->length     = size_available;

    DSPLOG_DEBUG("DSP: allocated %s memory area at pAddress=%p, vAddress=%p, length=%"PRIu32", dspAddress=%#010x",
                 user,
                 area->pAddress, area->vAddress, area->length, area->dspAddress);
}


/*
 * Allocates the single overlay memory descriptor, if the slot is free and enough memory is available.
 */
const DSP_DRAM_MEMORY_AREA *
DSP_getOverlayDRAM (DSP *dsp __unused, size_t size)
{
    /*  Let's place overlays at the 16MB mark with 1MB space */
    /*  FIXME: do it properly... */
#define OVERLAY_BASE    0x1000000
#define OVERLAY_SIZE    0x100000

    static DSP_DRAM_MEMORY_AREA overlay_area;
    fill_fake_dram (&overlay_area, size, OVERLAY_BASE, OVERLAY_SIZE, "overlay");

    return & overlay_area;
}


const DSP_DRAM_MEMORY_AREA *
DSP_getVomDRAM (DSP *dsp __unused, size_t size)
{
    /*  Place VOM code at 17MB mark with 1MB space */
    /*  FIXME: do it properly... */
#define VOM_BASE    0x1100000
#define VOM_SIZE    0x100000

    static DSP_DRAM_MEMORY_AREA vom_area;
    fill_fake_dram (&vom_area, size, VOM_BASE, VOM_SIZE, "VOM");

    return & vom_area;
}


/*
 * Write data to the BM via the SystemControlProxy implementation of the SCP protocol.
 */
DSP_RET DSP_writeSystemData(DSP *dsp __unused, uint32_t dest, const void *src, size_t n)
{
    DSPLOG_JUNK("DSP: writeSystemData dst=0x%08" PRIx32 " src=0x%" PRIxPTR " len=%#zx", dest, (uintptr_t) src, n);

    const uint8_t *c_src = src;

#ifdef DSP_BM_DUMP_DATA_ON_MEMORY_ACCESS
    DSPLOG_memdump_wrap(DSPLOG_JUNK_LEVEL, "DSP: writeSystemData", c_src, n, 16);
#endif

    return sysctlpxy_write_memory_block(dest, n, c_src) == 0 ? DSP_SUCCESS : DSP_FAILURE;
}


/*
 * Read data from the BM via the SystemControlProxy implementation of the SCP protocol.
 */
DSP_RET DSP_readSystemData(DSP *dsp __unused, void *dst, uint32_t src, size_t n)
{
    DSPLOG_JUNK("DSP: readSystemData dst=0x%" PRIxPTR " src=0x%08" PRIx32 " len=%#zx", (uintptr_t) dst, src, n);
    if(n == 0)
        return DSP_SUCCESS;

    uint8_t *c_dst = (uint8_t *) dst;
    int rv = sysctlpxy_read_memory_block(src, n, c_dst);

#ifdef DSP_BM_DUMP_DATA_ON_MEMORY_ACCESS
    DSPLOG_memdump_wrap(DSPLOG_JUNK_LEVEL, "DSP: readSystemData", c_dst, n, 16);
#endif

    return rv == 0 ? DSP_SUCCESS : DSP_FAILURE;
}


/*
 * Re-route to DSP_writeSystemData.
 */
DSP_RET DSP_writeDspData(DSP *dsp, DSP_ADDR dest, const void *src, size_t n)
{
    return DSP_writeSystemData(dsp, dest, src, n);
}


/*
 * Re-route to DSP_readSystemData.
 */
DSP_RET DSP_readDspData(DSP *dsp, void *dest, DSP_ADDR src, size_t n)
{
    return DSP_readSystemData(dsp, dest, src, n);
}


/*
 * Re-route to DSP_writeSystemData.
 */
DSP_RET DSP_writeSharedData(DSP *dsp, uint32_t dest, void *src, size_t n)
{
    return DSP_writeSystemData(dsp, dest, src, n);
}


/*
 * Re-route to DSP_readSystemData.
 */
DSP_RET DSP_readSharedData(DSP *dsp, void *dest, uint32_t src, size_t n)
{
    return DSP_readSystemData(dsp, dest, src, n);
}


/*
 * Write 4 bytes to shared memory - shorthand for DSP_writeSharedData(regAddr, &value, 4).
 */
void DSP_writeSharedRegister(DSP *dsp, uint32_t regAddr, uint32_t value)
{
    DSP_writeSharedData(dsp, regAddr, &value, 4);
}


/*
 * Read 4 bytes from shared memory - shorthand for DSP_readSharedData(regAddr, &retValue, 4).
 */
uint32_t DSP_readSharedRegister(DSP *dsp, uint32_t regAddr)
{
    uint32_t tmp;
    DSP_readSharedData(dsp, &tmp, regAddr, 4);
    return tmp;
}


/*
 * Re-route to DSP_writeSystemData.
 * We should re-route to DSP_writeDspData, but for performance
 * reasons we call DSP_writeSystemData directly.
 */
DSP_RET DSP_writeDataAsDsp(DSP *dsp, DSP_ADDR dest, void *src, size_t n)
{
    return DSP_writeSystemData(dsp, dest, src, n);
}


/*
 * Re-route to DSP_readSystemData.
 * We should re-route to DSP_readDspData, but for performance
 * reasons we call DSP_readSystemData directly.
 */
DSP_RET DSP_readDataAsDsp(DSP *dsp, void *dest, DSP_ADDR src, size_t n)
{
    return DSP_readSystemData(dsp, dest, src, n);
}
