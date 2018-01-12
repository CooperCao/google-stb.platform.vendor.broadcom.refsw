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
#include "gic.h"
#include "interrupt.h"

extern uintptr_t _interrupt_descs_start;
extern uintptr_t _interrupt_descs_end;

#define INTERRUPT_DESCS_START  ((uintptr_t)&_interrupt_descs_start)
#define INTERRUPT_DESCS_END    ((uintptr_t)&_interrupt_descs_end)
#define INTERRUPT_DESCS_NUM    ((INTERRUPT_DESCS_END - \
                                 INTERRUPT_DESCS_START) / sizeof(interrupt_desc_t))

void interrupt_init(void)
{
    interrupt_desc_t *pdesc;
    size_t idx;

    /* Init GIC distributor and CPU interface */
    gic_dist_init();
    gic_cpuif_init();

    pdesc = (interrupt_desc_t *)INTERRUPT_DESCS_START;
    for (idx = 0, pdesc = (interrupt_desc_t *)INTERRUPT_DESCS_START;
         idx < INTERRUPT_DESCS_NUM;
         idx++, pdesc++) {

        ASSERT(pdesc->intr_id < gic_intr_max());
        ASSERT(pdesc->proc);

        /* Init the interrupt */
        if (pdesc->init)
            pdesc->init();

        /* Enable the interrupt */
        gic_sec_intr_enable(pdesc->intr_id);
    }

    INFO_MSG("Interrupt init done");
}

void interrupt_proc(
    uint32_t iid,
    uint64_t *ctx,
    uint64_t flags)
{
    UNUSED(iid);
    iid = gic_intr_iid();

    interrupt_desc_t *pdesc;
    size_t idx;

    pdesc = (interrupt_desc_t *)INTERRUPT_DESCS_START;
    for (idx = 0, pdesc = (interrupt_desc_t *)INTERRUPT_DESCS_START;
         idx < INTERRUPT_DESCS_NUM;
         idx++, pdesc++) {

        if (pdesc->intr_id == GIC_INTR_ID(iid)) {
            /* Process the interrupt */
            if (pdesc->proc) {
                pdesc->proc(iid, ctx, flags);
                return;
            }
        }
    }

    /* Process unknown interrupt */
    gic_intr_ack(iid);
    gic_intr_end(iid);
}
