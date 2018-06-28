/******************************************************************************
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
 ******************************************************************************/
#include "nexus_ir_input_module.h"
#include "nexus_ir_input_bmon_priv.h"

BDBG_MODULE(nexus_ir_input_bmon);

static struct {
    NEXUS_IrInputEventHistory queue[32];
    unsigned wptr;
    bool wrap;
} g_NEXUS_IrInputBmon;
#define QUEUE_SIZE (sizeof(g_NEXUS_IrInputBmon.queue)/sizeof(g_NEXUS_IrInputBmon.queue[0]))

void NEXUS_P_IrInput_InsertBmon_isr(const NEXUS_IrInputEvent *pEvent, NEXUS_IrInputMode mode)
{
    NEXUS_IrInputEventHistory *pHistory = &g_NEXUS_IrInputBmon.queue[g_NEXUS_IrInputBmon.wptr];
    pHistory->event = *pEvent;
    pHistory->mode = mode;
    NEXUS_GetTimestamp(&pHistory->timestamp);
    if (++g_NEXUS_IrInputBmon.wptr == QUEUE_SIZE) {
        g_NEXUS_IrInputBmon.wptr = 0;
        g_NEXUS_IrInputBmon.wrap = true;
    }
}

/* non-destructive function, reverse search from wptr */
void NEXUS_IrInputModule_GetEventHistory( NEXUS_IrInputEventHistory *pHistory, unsigned numEntries, unsigned *pNumRead )
{
    unsigned ptr = g_NEXUS_IrInputBmon.wptr;
    *pNumRead = 0;
    if (numEntries > QUEUE_SIZE) numEntries = QUEUE_SIZE;
    BKNI_EnterCriticalSection();
    while (*pNumRead < numEntries) {
        if (ptr) {
            ptr--;
        }
        else if (g_NEXUS_IrInputBmon.wrap) {
            ptr = QUEUE_SIZE-1;
        }
        else {
            break;
        }
        pHistory[*pNumRead] = g_NEXUS_IrInputBmon.queue[ptr];
        (*pNumRead)++;
    }
    BKNI_LeaveCriticalSection();
}
