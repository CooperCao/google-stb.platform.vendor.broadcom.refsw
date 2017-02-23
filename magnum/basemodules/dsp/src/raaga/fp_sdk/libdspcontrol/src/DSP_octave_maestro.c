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

#if !FEATURE_IS(SW_HOST, RAAGA_MAGNUM)
#  include <stdint.h>
#  include <inttypes.h>
#else
#  include "bstd_defs.h"

/* Workaround for the missing inttypes.h */
#define PRIx32      "x"
#define PRIu32      "u"
#endif

#include "fp_sdk_config.h"

#include "libdspcontrol/COMMON.h"
#include "libdspcontrol/DSP.h"
#include "libdspcontrol/DSPLOG.h"

#include "libsyschip/src/heartbeat_internal.h"

#include "DSP_octave_maestro.h"



#if IS_HOST(DSP_LESS)
#  error "This module is not suitable for DSP-less builds"
#endif
#if !defined(__FP4014_ONWARDS__) && !defined(__FPM1015_ONWARDS__)
#  error "This module is only for Octave and Maestro based systems"
#endif


DSP_RET DSP_enable(DSP *dsp, DSP_CORE core)
{
    DSP_CORES_BITMAP cores;

    DSPLOG_DETAIL("DSP: enabling core %u", core);

    if(core > NUM_CORES)
    {
        DSPLOG_ERROR("DSP: trying to enable a non-existent DSP core (%d)", core);
        return DSP_WRONG_CORE;
    }

    DSP_enabledStatus(dsp, &cores);
    if(cores != DSP_CORES_NONE)
    {
        DSPLOG_ERROR("DSP: trying to enable an already enabled core");
        return DSP_ALREADY_ENABLED_CORE;
    }

    DSP_writeSharedRegister(dsp, MISC_BLOCK(dsp, core, CORECTRL_CORE_ENABLE), 1);

    DSP_enabledStatus(dsp, &cores);
    if(cores == DSP_CORES_NONE)
    {
        DSPLOG_ERROR("DSP: after enable, no core seems to be running");
        return DSP_DEAD_CORE;
    }
    return DSP_SUCCESS;
}


DSP_RET DSP_enabledStatus(DSP *dsp, DSP_CORES_BITMAP *cores)
{
    uint32_t mask = 0;
    unsigned core;
    for(core = 0; core < NUM_CORES; core++)
    {
        uint32_t this_core_enabled = DSP_readSharedRegister(dsp, MISC_BLOCK(dsp, core, CORECTRL_CORE_ENABLE)) & 1;
        mask |= this_core_enabled << core;
    }
    *cores = mask;

    return DSP_SUCCESS;
}


void DSP_pollInterrupts(DSP *dsp, DSP_CORE core __unused, DSP_INTERRUPTS *interrupts)
{
    interrupts->host_intc_host_irq = DSP_readSharedRegister(dsp, MISC_BLOCK(dsp, core, INTERRUPT_HOST_IRQ_LATCHED));

#if defined(__FP4015_ONWARDS__) || defined(__FPM1015_ONWARDS__)
    interrupts->obus_fault = DSP_readSharedRegister(dsp, MISC_BLOCK(dsp, core, INTERRUPT_OBUSFAULT_STATUS));
    interrupts->obus_fault_address = DSP_readSharedRegister(dsp, MISC_BLOCK(dsp, core, INTERRUPT_OBUSFAULT_ADDRESS));
#endif
}


#if !FEATURE_IS(SW_HOST, RAAGA_MAGNUM)      /* don't interfere with Magnum about interrupts handling */

void DSP_clearInterrupts(DSP *dsp, DSP_CORE core __unused, DSP_INTERRUPTS *interrupts)
{
    DSP_writeSharedRegister(dsp, MISC_BLOCK(dsp, core, INTERRUPT_HOST_IRQ_CLEAR), interrupts->host_intc_host_irq);
#if defined(__FP4015_ONWARDS__) || defined(__FPM1015_ONWARDS__)
    if(interrupts->obus_fault & MISC_BLOCK_INTERRUPT_OBUSFAULT_FAULT_PENDING)
    {
        DSP_writeSharedRegister(dsp, MISC_BLOCK(dsp, core, INTERRUPT_OBUSFAULT_CLEAR), MISC_BLOCK_INTERRUPT_OBUSFAULT_CLEAR_FAULT);
        interrupts->obus_fault_address = 0;
    }
#endif
}


void DSP_clearAllInterrupts(DSP *dsp, DSP_CORE core __unused)
{
    DSP_writeSharedRegister(dsp, MISC_BLOCK(dsp, core, INTERRUPT_HOST_IRQ_CLEAR), 0xFFFFFFFF);
#if defined(__FP4015_ONWARDS__) || defined(__FPM1015_ONWARDS__)
    DSP_writeSharedRegister(dsp, MISC_BLOCK(dsp, core, INTERRUPT_OBUSFAULT_CLEAR), MISC_BLOCK_INTERRUPT_OBUSFAULT_CLEAR_FAULT);
#endif
}

#endif /* !FEATURE_IS(SW_HOST, RAAGA_MAGNUM) */


DSP_RET DSP_pollHeartbeat(DSP *dsp, DSP_CORE core, DSP_HEARTBEAT *heartbeat)
{
    if(core < NUM_CORES)
    {
        uint32_t heartbeat_word = DSP_readSharedRegister(dsp, MISC_BLOCK(dsp, core, CORESTATE_SYS_MBX4));
        heartbeat->phase =    HEARTBEAT_GET_PHASE(heartbeat_word);
        heartbeat->subphase = HEARTBEAT_GET_SUBPHASE(heartbeat_word);
        heartbeat->argument = HEARTBEAT_GET_ARGUMENT(heartbeat_word);

        return DSP_SUCCESS;
    }
    else
        return DSP_WRONG_CORE;
}


DSP_RET DSP_clearHeartbeat(DSP *dsp, DSP_CORE core)
{
    if(core < NUM_CORES)
    {
        DSP_writeSharedRegister(dsp, MISC_BLOCK(dsp, core, CORESTATE_SYS_MBX4),
                                HEARTBEAT_ASSEMBLE_WORD(HB_PHASE_NOT_STARTED, HB_SUBPHASE_NOT_STARTED, 0));
        return DSP_SUCCESS;
    }
    else
        return DSP_WRONG_CORE;
}
