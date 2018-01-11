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
#include "std_svc.h"

/* Table of std service module descriptors */
extern service_mod_desc_t std_svc_psci_desc;

static service_mod_desc_t *std_svc_mod_descs[STD_SVC_MOD_MAX] = {
    &std_svc_psci_desc
};

static uint32_t call_count;

/* UUID generated with name broadcom.monitor.std_svc under namespace OID */
/* UUID generated using tools/uuid */
static uuid_t uuid = {
    0xc23221bd, 0xd506, 0x35ff, 0x99, 0xc9,
    {0xe8, 0x15, 0x4c, 0xe4, 0x91, 0xf6}
};

static void std_svc_init(void)
{
    uint32_t mod;

    /* Start with 3 general queries */
    call_count = 3;

    for (mod = 0; mod < STD_SVC_MOD_MAX; mod++) {
        service_mod_desc_t *pdesc = std_svc_mod_descs[mod];

        if (pdesc) {
            if (pdesc->init)
                pdesc->init();
            call_count += pdesc->call_count;
        }
    }
}

static void std_svc_proc(
    uint32_t fid,
    uint64_t *ctx,
    uint64_t flags)
{
    uint32_t mod;

    DBG_MSG("Standard service SMC received: fid = 0x%x", fid);

    for (mod = 0; mod < STD_SVC_MOD_MAX; mod++) {
        service_mod_desc_t *pdesc = std_svc_mod_descs[mod];

        if (pdesc) {
            if (pdesc->check) {
                if (pdesc->check(fid)) {
                    if (pdesc->proc) {
                        pdesc->proc(fid, ctx, flags);
                        return;
                    }
                }
            }
        }
    }

    if ((fid & 0xFF00) == 0xFF00) {
        uint32_t *puuid = (uint32_t *)&uuid;;

        switch (fid & 0xFF) {
        case STD_SVC_CALL_COUNT:
            ctx[0] = call_count;
            return;
        case STD_SVC_CALL_UID:
            ctx[0] = puuid[0];
            ctx[1] = puuid[1];
            ctx[2] = puuid[2];
            ctx[3] = puuid[3];
            return;
        case STD_SVC_REVISION:
            ctx[0] = STD_SVC_REVISION_MAJOR;
            ctx[1] = STD_SVC_REVISION_MINOR;
            return;
        }
    }

    /* All fall-through cases */
    ctx[0] = SMC_UNK;
}

DECLARE_SERVICE(
    std_svc,
    STD_SVC_OEN,
    STD_SVC_OEN,
    std_svc_init,
    std_svc_proc);
