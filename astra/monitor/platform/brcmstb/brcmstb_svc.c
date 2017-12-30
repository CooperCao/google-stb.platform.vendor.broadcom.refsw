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

#include <uuid.h>

#include "monitor.h"
#include "service.h"
#include "brcmstb_svc.h"

/* Table of brcmstb service module descriptors */
extern service_mod_desc_t brcmstb_svc_psci_desc;
extern service_mod_desc_t brcmstb_svc_dvfs_desc;
extern service_mod_desc_t brcmstb_svc_astra_desc;
extern service_mod_desc_t brcmstb_svc_linux_desc;

static service_mod_desc_t *brcmstb_svc_mod_descs[BRCMSTB_SVC_MOD_MAX] = {
    &brcmstb_svc_psci_desc,
    &brcmstb_svc_dvfs_desc,
    &brcmstb_svc_astra_desc,
    &brcmstb_svc_linux_desc
};

static uint32_t call_count;

/* UUID generated with name broadcom.monitor.brcmstb_svc under namespace OID */
/* UUID generated using tools/uuid */
static uuid_t uuid = {
    0xc2da1e9e, 0x3537, 0x36f1, 0x8f, 0x14,
    {0x8d, 0xd5, 0x18, 0x12, 0xae, 0x1a}
};

static void brcmstb_svc_init(void)
{
    uint32_t mod;

    /* Start with 3 general queries */
    call_count = 3;

    for (mod = 0; mod < BRCMSTB_SVC_MOD_MAX; mod++) {
        service_mod_desc_t *pdesc = brcmstb_svc_mod_descs[mod];

        if (pdesc) {
            if (pdesc->init)
                pdesc->init();
            call_count += pdesc->call_count;
        }
    }
}

static void brcmstb_svc_proc(
    uint32_t fid,
    uint64_t *ctx,
    uint64_t flags)
{
    uint32_t mod = BRCMSTB_SVC_MOD(fid);

    // DBG_MSG("Brcmstb service SMC received: fid = 0x%x", fid);

    if (mod < BRCMSTB_SVC_MOD_MAX) {
        service_mod_desc_t *pdesc = brcmstb_svc_mod_descs[mod];

        if (pdesc) {
            if (pdesc->proc) {
                pdesc->proc(fid, ctx, flags);
                return;
            }
        }
    }

    if (mod == 0xFF) {
        uint32_t *puuid = (uint32_t *)&uuid;;

        switch (BRCMSTB_SVC_FUNC(fid)) {
        case BRCMSTB_SVC_CALL_COUNT:
            ctx[0] = call_count;
            return;
        case BRCMSTB_SVC_CALL_UID:
            ctx[0] = puuid[0];
            ctx[1] = puuid[1];
            ctx[2] = puuid[2];
            ctx[3] = puuid[3];
            return;
        case BRCMSTB_SVC_REVISION:
            ctx[0] = BRCMSTB_SVC_REVISION_MAJOR;
            ctx[1] = BRCMSTB_SVC_REVISION_MINOR;
            return;
        }
    }

    /* All fall-through cases */
    ctx[0] = SMC_UNK;
}

DECLARE_SERVICE(
    brcmstb_svc,
    BRCMSTB_SVC_OEN,
    BRCMSTB_SVC_OEN,
    brcmstb_svc_init,
    brcmstb_svc_proc);
