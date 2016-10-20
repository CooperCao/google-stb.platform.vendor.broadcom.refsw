/***************************************************************************
*  Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
*
*  This program is the proprietary software of Broadcom and/or its licensors,
*  and may only be used, duplicated, modified or distributed pursuant to the terms and
*  conditions of a separate, written license agreement executed between you and Broadcom
*  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
*  no license (express or implied), right to use, or waiver of any kind with respect to the
*  Software, and Broadcom expressly reserves all rights in and to the Software and all
*  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
*  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
*  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
*
*  Except as expressly set forth in the Authorized License,
*
*  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
*  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
*  and to use this information only in connection with your use of Broadcom integrated circuit products.
*
*  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
*  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
*  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
*  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
*  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
*  USE OR PERFORMANCE OF THE SOFTWARE.
*
*  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
*  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
*  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
*  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
*  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
*  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
*  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
*  ANY LIMITED REMEDY.
*
* API Description:
*   API name: Platform linuxuser
*    linuxuser OS routines
*
***************************************************************************/

#include "nexus_types.h"
#include "nexus_base.h"
#include "nexus_platform_extint.h"
#include "nexus_platform_features.h"
#include "nexus_platform_priv.h"
#include "priv/nexus_core.h"
#include "bkni.h"
#include "bchp_hif_cpu_intr1.h"

BDBG_MODULE(nexus_platform_interrupt);

struct NEXUS_ExternalInterrupt
{
    NEXUS_ExternalInterruptSettings settings;
    NEXUS_IsrCallbackHandle callback;
    bool enabled;
    bool valid;
    unsigned irq; /* 0-based bit position (0..63) in L1 register set */
};

#ifndef NEXUS_EXTIRQ_MAX
#define NEXUS_EXTIRQ_MAX 5
#endif

static struct NEXUS_ExternalInterrupt g_NEXUS_ExternalInterrupt[NEXUS_EXTIRQ_MAX];

NEXUS_Error NEXUS_Platform_P_InitInterrupts(void)
{
    unsigned i;
    BINT_Handle intHandle = g_pCoreHandles->bint;
    NEXUS_Error rc = NEXUS_SUCCESS;
    uint32_t l1Mask[BINT_MAX_INTC_SIZE];

    NEXUS_Platform_P_ResetInterrupts();

    BINT_GetL1BitMask(intHandle, l1Mask);
    for (i=0; i<32*NEXUS_NUM_L1_REGISTERS; i++ ) {
        if ( l1Mask[i/32] & (1ul<<(i%32))) {
            BDBG_MSG(("Enabling L1 interrupt %u", i));
            rc = NEXUS_Platform_P_ConnectInterrupt(i, (NEXUS_Core_InterruptFunction)BINT_Isr, intHandle, i);
            if (rc==NEXUS_SUCCESS) {
                rc = NEXUS_Platform_P_EnableInterrupt(i);
                if (rc!=NEXUS_SUCCESS) {
                    rc = BERR_TRACE(rc); goto error;
                }
            } else if (rc != NEXUS_NOT_AVAILABLE) { /* if L1 not managed by linux, we get this error */
                rc = BERR_TRACE(rc); goto error;
            }
        }
    }


    BKNI_Memset(&g_NEXUS_ExternalInterrupt, 0, sizeof(g_NEXUS_ExternalInterrupt));
    {
        unsigned i,extIrqNo;
        const BINT_P_IntMap *intMap =  g_pCoreHandles->bint_map;/* use BINT's record of external interrupts */

        for (extIrqNo=i=0;;i++) {
            const BINT_P_IntMap *map = intMap+i;
            if(map->L1Shift==-1) {
                break;
            }
            if (BINT_MAP_IS_EXTERNAL(map)) {
                if(extIrqNo >= NEXUS_EXTIRQ_MAX) {
                    BDBG_WRN(("Exceeded number of slots for external IRQ (%u,%u), ignoring %u", extIrqNo, NEXUS_EXTIRQ_MAX, BINT_MAP_GET_L1SHIFT(map)));
                    continue;
                }
                g_NEXUS_ExternalInterrupt[extIrqNo].valid = true;
                g_NEXUS_ExternalInterrupt[extIrqNo].irq = BINT_MAP_GET_L1SHIFT(map);
                extIrqNo++;
            }
        }
        BDBG_MSG(("Found %u external interrupts", extIrqNo));
    }

    return NEXUS_SUCCESS;
error:
    return rc;
}

void NEXUS_Platform_P_UninitInterrupts(void)
{
    unsigned i;
    uint32_t l1Mask[BINT_MAX_INTC_SIZE];

    NEXUS_Platform_P_ResetInterrupts();

    BINT_GetL1BitMask(g_pCoreHandles->bint, l1Mask);
    for (i=0; i<32*NEXUS_NUM_L1_REGISTERS; i++ ) {
        if ( l1Mask[i/32] & (1ul<<(i%32))) {
            NEXUS_Platform_P_DisconnectInterrupt(i);
        }
    }
    return;
}


void NEXUS_Platform_GetDefaultExternalInterruptSettings( NEXUS_ExternalInterruptSettings *pSettings )
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    NEXUS_CallbackDesc_Init(&pSettings->callback);
}

static void NEXUS_Platform_P_ExternalInterrupt_isr(void *context, int param)
{
    BSTD_UNUSED(param);
    NEXUS_IsrCallback_Fire_isr(((struct NEXUS_ExternalInterrupt*)context)->callback);
}

NEXUS_Error NEXUS_Platform_EnableExternalInterrupt( unsigned interruptId, const NEXUS_ExternalInterruptSettings *pSettings )
{
    NEXUS_Error rc;
    struct NEXUS_ExternalInterrupt *handle;

    if (interruptId >= NEXUS_EXTIRQ_MAX || !g_NEXUS_ExternalInterrupt[interruptId].valid ){
        BDBG_ERR(("invalid external interrupt %u", interruptId));
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }

    handle = &g_NEXUS_ExternalInterrupt[interruptId];
    if (handle->enabled) {
        BDBG_WRN(("External interrupt %u already enabled", interruptId));
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }

    handle->settings = *pSettings;
    handle->callback = NEXUS_IsrCallback_Create(NULL, NULL);
    NEXUS_IsrCallback_Set(handle->callback, &pSettings->callback);
    rc = NEXUS_Platform_P_ConnectInterrupt(handle->irq, NEXUS_Platform_P_ExternalInterrupt_isr, handle, 0);
    if (!rc) {
        rc = NEXUS_Platform_P_EnableInterrupt(handle->irq);
    }
    if (!rc) {
        handle->enabled = true;
    }

    return rc;
}

void NEXUS_Platform_DisableExternalInterrupt( unsigned interruptId )
{
    struct NEXUS_ExternalInterrupt *handle;

    if (interruptId >= NEXUS_EXTIRQ_MAX || !g_NEXUS_ExternalInterrupt[interruptId].valid ){
        BDBG_ERR(("invalid external interrupt %u", interruptId));
        return;
    }

    handle = &g_NEXUS_ExternalInterrupt[interruptId];

    if (!handle->enabled) {
        BDBG_WRN(("External interrupt %u not enabled", interruptId));
        return;
    }

    NEXUS_Platform_P_DisableInterrupt(handle->irq);
    NEXUS_Platform_P_DisconnectInterrupt(handle->irq);
    NEXUS_IsrCallback_Destroy(handle->callback);
    handle->enabled = false;
}


