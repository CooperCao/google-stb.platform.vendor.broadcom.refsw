/******************************************************************************
 *  Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 ******************************************************************************/

#include "bstd.h"
#include "bkni.h"

#include "blst_slist.h"
#include "blst_squeue.h"
#include "bint.h"
#include "bchp.h"
#include "bchp_scpu_host_intr2.h"
#define BCHP_INT_ID_SCPU_HOST_WATCHDOG_TIMEOUT_INTR    BCHP_INT_ID_CREATE( BCHP_SCPU_HOST_INTR2_CPU_STATUS, BCHP_SCPU_HOST_INTR2_CPU_STATUS_SCPU_TIMER_SHIFT )

#include "bsage_management.h"
#include "bsage_priv.h"
#include "bsagelib_priv.h"

BDBG_MODULE(BSAGE);

/* local functions */
static void BSAGE_P_Management_DispatchAllResponse_isrsafe(BSAGE_Handle hSAGE);
static void BSAGE_P_Management_WatchdogIntHandler_isr(void *parm1, int parm2);


BERR_Code
BSAGE_Management_Register(
    BSAGE_ManagementInterface *i_management)
{
    BERR_Code rc = BERR_SUCCESS;
    BSAGE_Handle hSAGE = hSAGE_Global;

    BDBG_ENTER(BSAGE_Management_Register);

    BDBG_ASSERT(i_management);

    if (i_management->watchdog_isr) {
        BSAGE_Management_CallbackItem *item;
        item = BKNI_Malloc(sizeof(*item));
        if (!item) {
            BDBG_ERR(("%s: cannot allocate callback context", BSTD_FUNCTION));
            rc = BERR_OUT_OF_SYSTEM_MEMORY;
            goto end;
        }
        BKNI_Memset(item, 0, sizeof(*item));
        item->watchdog = *i_management;
        /* Insert in management list. Avoid concurrency with isr */
        BDBG_MSG(("%s: Add Watchdog callback %p",
                  BSTD_FUNCTION, (void *)(unsigned long)i_management->watchdog_isr));
        BKNI_EnterCriticalSection();
        BLST_SQ_INSERT_TAIL(&hSAGE->watchdog_callbacks, item, link);
        BKNI_LeaveCriticalSection();
    }
end:
    BDBG_LEAVE(BSAGE_Management_Register);
    return rc;
}

BERR_Code
BSAGE_Management_Unregister(
    BSAGE_ManagementInterface *i_management)
{
    BSAGE_Handle hSAGE = hSAGE_Global;
    BERR_Code rc = BERR_SUCCESS;

    BDBG_ENTER(BSAGE_Management_Unregister);

    BDBG_ASSERT(i_management);

    if (i_management->watchdog_isr) {
        BSAGE_Management_CallbackItem *item;
        for (item = BLST_SQ_FIRST(&hSAGE->watchdog_callbacks); item; item = BLST_SQ_NEXT(item, link)) {
            if (item->watchdog.watchdog_isr == i_management->watchdog_isr) {
                BDBG_MSG(("%s: Remove Watchdog callback %p",
                          BSTD_FUNCTION, (void *)(unsigned long)i_management->watchdog_isr));
                BKNI_EnterCriticalSection();
                BLST_SQ_REMOVE(&hSAGE->watchdog_callbacks, item, BSAGE_Management_CallbackItem, link);
                BKNI_LeaveCriticalSection();
                BKNI_Free(item);
                break;
            }
        }
    }

    BDBG_LEAVE(BSAGE_Management_Unregister);
    return rc;
}
/* Invalidate all remotes and respond to pending requests.
   New commands will be rejected. */
static void
BSAGE_P_Management_DispatchAllResponse_isrsafe(
    BSAGE_Handle hSAGE)
{
    BSAGE_RpcRemoteHandle remote;
    for (remote = BLST_S_FIRST(&hSAGE->remotes); remote; remote = BLST_S_NEXT(remote, link)) {
/*        remote->valid = 0;*/
        if (remote->message->sequence) {
            remote->rpc.response_isr(remote->rpc.responseContext, BSAGE_ERR_RESET);
        }
    }
}

void
BSAGE_Management_Reset(void)
{
    BSAGE_Handle hSAGE = hSAGE_Global;
    BDBG_ENTER(BSAGE_Management_Reset);

    hSAGE->resetPending = 0;

    /* reset/clear RPC and HSI */
    BSAGE_P_Rpc_Reset(hSAGE);

    BKNI_EnterCriticalSection();

    /* invalidate all existing remotes + fire pending callbacks */
    BSAGE_P_Management_DispatchAllResponse_isrsafe(hSAGE);

    BKNI_LeaveCriticalSection();

end:
    BDBG_LEAVE(BSAGE_Management_Reset);
}

/* BHSI_IsrCallbackFunc prototype, see bhsi.h */
static void
BSAGE_P_Management_WatchdogIntHandler_isr(
    void *parm1,
    int parm2)
{
    BSAGE_Handle hSAGE;

    BSTD_UNUSED(parm2);

    hSAGE = (BSAGE_Handle)parm1;
    BDBG_ASSERT(hSAGE);

    BDBG_MSG(("%s: watchdog interrupt, instance=%p", BSTD_FUNCTION, (void *)hSAGE));

    hSAGE->resetPending = 1;

    /* invalidate all existing remotes + fire pending callbacks */
    BSAGE_P_Management_DispatchAllResponse_isrsafe(hSAGE);

    BSAGE_P_Rpc_Reset_isrsafe(hSAGE);
    /* Fire all registered watchdog callbacks */
    {
        BSAGE_Management_CallbackItem *item;
        for (item = BLST_SQ_FIRST(&hSAGE->watchdog_callbacks); item; item = BLST_SQ_NEXT(item, link)) {
            BDBG_MSG(("%s: Fire Watchdog callback %p",
                      BSTD_FUNCTION, (void *)(unsigned long)item->watchdog.watchdog_isr));
            item->watchdog.watchdog_isr(item->watchdog.context);
        }
    }
}

BERR_Code
BSAGE_P_Management_Initialize(
    BSAGE_Handle hSAGE)
{
    BERR_Code rc;

    BLST_SQ_INIT(&hSAGE->watchdog_callbacks);

    /* Create interrupt service routine (ISR) for Watchdog Timeout */
    rc = BINT_CreateCallback(&(hSAGE->watchdogIntCallback),
                             hSAGE->hInt,
                             BCHP_INT_ID_SCPU_HOST_WATCHDOG_TIMEOUT_INTR,
                             BSAGE_P_Management_WatchdogIntHandler_isr,
                             (void *) hSAGE,
                             0x00);
    if (rc != BERR_SUCCESS) {
        BDBG_ERR(("%s: BINT_CreateCallback(watchdog) failed!", BSTD_FUNCTION));
        goto end;
    }

    rc = BINT_EnableCallback(hSAGE->watchdogIntCallback);
    if (rc != BERR_SUCCESS) {
        BDBG_ERR(( "%s: BINT_EnableCallback(Watchdog) failed!", BSTD_FUNCTION));
        goto end;
    }

end:
    return rc;
}

void
BSAGE_P_Management_Uninitialize(
    BSAGE_Handle hSAGE)
{
    if (hSAGE->watchdogIntCallback) {
        BERR_Code err;
        BINT_DisableCallback(hSAGE->watchdogIntCallback);
        err = BINT_DestroyCallback(hSAGE->watchdogIntCallback);
        if (err != BERR_SUCCESS) {
            BDBG_ERR(("%s: BINT_DestroyCallback(watchdog) returns error %u",
                      BSTD_FUNCTION, err));
            (void)BERR_TRACE(BERR_INVALID_PARAMETER) ;
        }
        hSAGE->watchdogIntCallback = NULL;
    }

    do {
        BSAGE_Management_CallbackItem *item;
        BKNI_EnterCriticalSection();
        item = BLST_SQ_FIRST(&hSAGE->watchdog_callbacks);
        if (item) {
            BLST_SQ_REMOVE_HEAD(&hSAGE->watchdog_callbacks, link);
        }
        BKNI_LeaveCriticalSection();
        if (item) {
            BKNI_Free(item);
            continue;
        }
    } while (0);
}
