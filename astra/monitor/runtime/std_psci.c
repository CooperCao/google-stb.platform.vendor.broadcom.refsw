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
#include "psci.h"
#include "std_svc.h"
#include "std_psci.h"

static void do_psci_version(uint64_t *ctx);
static void do_psci_cpu_suspend(uint64_t *ctx);
static void do_psci_cpu_off(uint64_t *ctx);
static void do_psci_cpu_on(uint64_t *ctx);
static void do_psci_affinity_info(uint64_t *ctx);
static void do_psci_migrate(uint64_t *ctx);
static void do_psci_migrate_info_type(uint64_t *ctx);
static void do_psci_migrate_info_up_cpu(uint64_t *ctx);
static void do_psci_system_off(uint64_t *ctx);
static void do_psci_system_reset(uint64_t *ctx);
static void do_psci_features(uint64_t *ctx);
static void do_psci_cpu_freeze(uint64_t *ctx);
static void do_psci_cpu_default_suspend(uint64_t *ctx);
static void do_psci_node_hw_state(uint64_t *ctx);
static void do_psci_system_suspend(uint64_t *ctx);
static void do_psci_set_suspend_mode(uint64_t *ctx);
static void do_psci_stat_residency(uint64_t *ctx);
static void do_psci_stat_count(uint64_t *ctx);

static service_do_func_t do_psci_funcs[STD_SVC_PSCI_MAX] =
{
    do_psci_version,
    do_psci_cpu_suspend,
    do_psci_cpu_off,
    do_psci_cpu_on,
    do_psci_affinity_info,
    do_psci_migrate,
    do_psci_migrate_info_type,
    do_psci_migrate_info_up_cpu,
    do_psci_system_off,
    do_psci_system_reset,
    do_psci_features,
    do_psci_cpu_freeze,
    do_psci_cpu_default_suspend,
    do_psci_node_hw_state,
    do_psci_system_suspend,
    do_psci_set_suspend_mode,
    do_psci_stat_residency,
    do_psci_stat_count
};

static bool std_svc_psci_check(uint32_t fid)
{
    return (STD_SVC_PSCI_MOD(fid) == STD_SVC_PSCI_MOD_BITS);
}

static void std_svc_psci_init(void)
{
}

static void std_svc_psci_proc(
    uint32_t fid,
    uint64_t *ctx,
    uint64_t flags)
{
    service_do_func_t pfunc;

    if (is_caller_secure(flags)) {
        ctx[0] = SMC_UNK;
        return;
    }

    if (GET_SMC_CC(fid) == SMC_32) {
        /* 32-bit PSCI function, clear top bits of arguments */
        ctx[0] = (uint32_t)ctx[0];
        ctx[1] = (uint32_t)ctx[1];
        ctx[2] = (uint32_t)ctx[2];
        ctx[3] = (uint32_t)ctx[3];
    }

    pfunc = do_psci_funcs[STD_SVC_PSCI_FUNC(fid)];
    if (pfunc) {
        pfunc(ctx);
        return;
    }

    /* All fall-through cases */
    ctx[0] = SMC_UNK;
}

service_mod_desc_t std_svc_psci_desc = {
    STD_SVC_PSCI_MAX,
    std_svc_psci_check,
    std_svc_psci_init,
    std_svc_psci_proc
};

static void do_psci_version(uint64_t *ctx)
{
    ctx[0] = (STD_SVC_PSCI_VERSION_MAJOR << 16) |
             (STD_SVC_PSCI_VERSION_MINOR);
}

static void do_psci_cpu_suspend(uint64_t *ctx)
{
    ctx[0] = STD_SVC_PSCI_NOT_SUPPORTED;
}

static void do_psci_cpu_off(uint64_t *ctx)
{
    int ret;

    ret = psci_cpu_off();

    ctx[0] = (uint64_t)ret;
}

static void do_psci_cpu_on(uint64_t *ctx)
{
    uint64_t target_mpidr = ctx[1];
    uint64_t entry_point = ctx[2];
    uint64_t context_id = ctx[3];
    int ret;

    ret = psci_cpu_on(
        target_mpidr,
        entry_point,
        context_id);

    ctx[0] = (uint64_t)ret;
}

static void do_psci_affinity_info(uint64_t *ctx)
{
    ctx[0] = STD_SVC_PSCI_NOT_SUPPORTED;
}

static void do_psci_migrate(uint64_t *ctx)
{
    ctx[0] = STD_SVC_PSCI_NOT_SUPPORTED;
}

static void do_psci_migrate_info_type(uint64_t *ctx)
{
    ctx[0] = STD_SVC_PSCI_NOT_SUPPORTED;
}

static void do_psci_migrate_info_up_cpu(uint64_t *ctx)
{
    ctx[0] = STD_SVC_PSCI_NOT_SUPPORTED;
}

static void do_psci_system_off(uint64_t *ctx)
{
    ctx[0] = STD_SVC_PSCI_NOT_SUPPORTED;
}

static void do_psci_system_reset(uint64_t *ctx)
{
    ctx[0] = STD_SVC_PSCI_NOT_SUPPORTED;
}

static void do_psci_features(uint64_t *ctx)
{
    ctx[0] = STD_SVC_PSCI_NOT_SUPPORTED;
}

static void do_psci_cpu_freeze(uint64_t *ctx)
{
    ctx[0] = STD_SVC_PSCI_NOT_SUPPORTED;
}

static void do_psci_cpu_default_suspend(uint64_t *ctx)
{
    ctx[0] = STD_SVC_PSCI_NOT_SUPPORTED;
}

static void do_psci_node_hw_state(uint64_t *ctx)
{
    ctx[0] = STD_SVC_PSCI_NOT_SUPPORTED;
}

static void do_psci_system_suspend(uint64_t *ctx)
{
    ctx[0] = STD_SVC_PSCI_NOT_SUPPORTED;
}

static void do_psci_set_suspend_mode(uint64_t *ctx)
{
    ctx[0] = STD_SVC_PSCI_NOT_SUPPORTED;
}

static void do_psci_stat_residency(uint64_t *ctx)
{
    ctx[0] = STD_SVC_PSCI_NOT_SUPPORTED;
}

static void do_psci_stat_count(uint64_t *ctx)
{
    ctx[0] = STD_SVC_PSCI_NOT_SUPPORTED;
}
