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

#include "monitor.h"
#include "cpu_data.h"
#include "context_mgt.h"
#include "gic.h"
#include "interrupt.h"
#include "brcmstb_svc.h"
#include "brcmstb_astra.h"

static void do_astra_nsec_switch(uint64_t *ctx);

static service_do_func_t do_astra_funcs[BRCMSTB_SVC_ASTRA_MAX] =
{
    do_astra_nsec_switch
};

static void brcmstb_svc_astra_init(void)
{
}

static void brcmstb_svc_astra_proc(
    uint32_t fid,
    uint64_t *ctx,
    uint64_t flags)
{
    service_do_func_t pfunc;

    if (!is_caller_secure(flags)) {
        ctx[0] = SMC_UNK;
        return;
    }

    if (GET_SMC_CC(fid) != SMC_32) {
        ctx[0] = SMC_UNK;
        return;
    }

    pfunc = do_astra_funcs[BRCMSTB_SVC_FUNC(fid)];
    if (pfunc) {
        pfunc(ctx);
        return;
    }

    /* All fall-through cases */
    ctx[0] = SMC_UNK;
}

static void do_astra_nsec_switch(uint64_t *ctx)
{
    /* Only switch to non-secure world if enabled */
    if (is_nsec_enable())
        cm_switch_context((cpu_context_t *)ctx, NON_SECURE);
}

service_mod_desc_t brcmstb_svc_astra_desc = {
    BRCMSTB_SVC_ASTRA_MAX,
    NULL,
    brcmstb_svc_astra_init,
    brcmstb_svc_astra_proc
};

static void brcmstb_intr_astra_init(void)
{
}

static void brcmstb_intr_astra_proc(
    uint32_t iid,
    uint64_t *ctx,
    uint64_t flags)
{
#ifdef DEBUG
    ASSERT(GIC_INTR_ID(iid) == BRCMSTB_INTR_ASTRA_TIMER ||
           GIC_INTR_ID(iid) == BRCMSTB_INTR_ASTRA_IPI);
#else
    UNUSED(iid);
#endif

    if (is_caller_secure(flags))
        return;

    /* Switch to secure world */
    cm_switch_context((cpu_context_t *)ctx, SECURE);
}

DECLARE_INTERRUPT(
    astra_timer_intr,
    BRCMSTB_INTR_ASTRA_TIMER,
    brcmstb_intr_astra_init,
    brcmstb_intr_astra_proc);

DECLARE_INTERRUPT(
    astra_tzioc_intr,
    BRCMSTB_INTR_ASTRA_IPI,
    brcmstb_intr_astra_init,
    brcmstb_intr_astra_proc);
