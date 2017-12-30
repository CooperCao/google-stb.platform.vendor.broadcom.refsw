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

#include <string.h>
#include <smcc.h>

#include "monitor.h"
#include "service.h"

extern uintptr_t _service_descs_start;
extern uintptr_t _service_descs_end;

#define SERVICE_DESCS_START  ((uintptr_t)&_service_descs_start)
#define SERVICE_DESCS_END    ((uintptr_t)&_service_descs_end)
#define SERVICE_DESCS_NUM    ((SERVICE_DESCS_END - \
                               SERVICE_DESCS_START) / sizeof(service_desc_t))

/* Table to lookup service descriptor index by OEN */
uint8_t service_oen_to_desc_idx[OEN_LIMIT];

void service_init(void)
{
    service_desc_t *pdesc;
    size_t idx, oen;

    /* Init the OEN table to invalid state */
    memset(service_oen_to_desc_idx, -1, sizeof(service_oen_to_desc_idx));

    pdesc = (service_desc_t *)SERVICE_DESCS_START;
    for (idx = 0, pdesc = (service_desc_t *)SERVICE_DESCS_START;
         idx < SERVICE_DESCS_NUM;
         idx++, pdesc++) {

        ASSERT(pdesc->oen_start <= pdesc->oen_end);
        ASSERT(pdesc->oen_end < OEN_LIMIT);
        ASSERT(pdesc->proc);

        /* Init the service */
        if (pdesc->init)
            pdesc->init();

        for (oen = pdesc->oen_start; oen <= pdesc->oen_end; oen++)
            service_oen_to_desc_idx[oen] = idx;
    }

    INFO_MSG("SMC service init done");
}
