/******************************************************************************
 * (c) 2014 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its
 * licensors, and may only be used, duplicated, modified or distributed pursuant
 * to the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and Broadcom
 * expressly reserves all rights in and to the Software and all intellectual
 * property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 *    constitutes the valuable trade secrets of Broadcom, and you shall use all
 *    reasonable efforts to protect the confidentiality thereof, and to use
 *    this information only in connection with your use of Broadcom integrated
 *    circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
 *    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
 *    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
 *    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
 *    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
 *    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
 *    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
 *    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
 *    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
 *    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ******************************************************************************/

#include "bstd.h"
#include "bkni.h"

#include "blst_slist.h"
#include "blst_squeue.h"
#include "bint.h"
#include "bchp.h"
#include "bchp_scpu_host_intr2.h"
#define BCHP_INT_ID_SCPU_HOST_WATCHDOG_TIMEOUT_INTR    BCHP_INT_ID_CREATE( BCHP_SCPU_HOST_INTR2_CPU_STATUS, BCHP_SCPU_HOST_INTR2_CPU_STATUS_SCPU_TIMER_SHIFT )

#include "bsagelib_management.h"
#include "bsagelib_priv.h"

BDBG_MODULE(BSAGElib);

/* local functions */
static void BSAGElib_P_Management_DispatchAllResponse_isrsafe(BSAGElib_Handle hSAGElib);
static void BSAGElib_P_Management_WatchdogIntHandler_isr(void *parm1, int parm2);


BERR_Code
BSAGElib_Management_Register(
    BSAGElib_Handle hSAGElib,
    BSAGElib_ManagementInterface *i_management)
{
    BERR_Code rc = BERR_SUCCESS;

    BDBG_ENTER(BSAGElib_Management_Register);

    BDBG_OBJECT_ASSERT(hSAGElib, BSAGElib_P_Instance);
    BDBG_ASSERT(i_management);

    if (i_management->watchdog_isr) {
        BSAGElib_Management_CallbackItem *item;
        item = BKNI_Malloc(sizeof(*item));
        if (!item) {
            BDBG_ERR(("%s: cannot allocate callback context", __FUNCTION__));
            rc = BERR_OUT_OF_SYSTEM_MEMORY;
            goto end;
        }
        BKNI_Memset(item, 0, sizeof(*item));
        item->watchdog_isr = i_management->watchdog_isr;
        /* Insert in management list. Avoid concurrency with isr */
        BDBG_MSG(("%s: Add Watchdog callback %08x",
                  __FUNCTION__, (uint32_t)i_management->watchdog_isr));
        BKNI_EnterCriticalSection();
        BLST_SQ_INSERT_TAIL(&hSAGElib->watchdog_callbacks, item, link);
        BKNI_LeaveCriticalSection();
    }
end:
    BDBG_LEAVE(BSAGElib_Management_Register);
    return rc;
}

BERR_Code
BSAGElib_Management_Unregister(
    BSAGElib_Handle hSAGElib,
    BSAGElib_ManagementInterface *i_management)
{
    BERR_Code rc = BERR_SUCCESS;

    BDBG_ENTER(BSAGElib_Management_Unregister);

    BDBG_OBJECT_ASSERT(hSAGElib, BSAGElib_P_Instance);
    BDBG_ASSERT(i_management);

    if (i_management->watchdog_isr) {
        BSAGElib_Management_CallbackItem *item;
        for (item = BLST_SQ_FIRST(&hSAGElib->watchdog_callbacks); item; item = BLST_SQ_NEXT(item, link)) {
            if (item->watchdog_isr == i_management->watchdog_isr) {
                BDBG_MSG(("%s: Remove Watchdog callback %08x",
                          __FUNCTION__, (uint32_t)i_management->watchdog_isr));
                BKNI_EnterCriticalSection();
                BLST_SQ_REMOVE(&hSAGElib->watchdog_callbacks, item, BSAGElib_Management_CallbackItem, link);
                BKNI_LeaveCriticalSection();
                BKNI_Free(item);
                break;
            }
        }
    }

    BDBG_LEAVE(BSAGElib_Management_Unregister);
    return rc;
}

/* Invalidate all remotes and respond to pending requests.
   New commands will be rejected. */
static void
BSAGElib_P_Management_DispatchAllResponse_isrsafe(
    BSAGElib_Handle hSAGElib)
{
    BSAGElib_ClientHandle hSAGElibClient;
    for (hSAGElibClient = BLST_S_FIRST(&hSAGElib->clients); hSAGElibClient; hSAGElibClient = BLST_S_NEXT(hSAGElibClient, link)) {
        BSAGElib_RpcRemoteHandle remote;
        for (remote = BLST_S_FIRST(&hSAGElibClient->remotes); remote; remote = BLST_S_NEXT(remote, link)) {
            remote->valid = 0;
            if (remote->message->sequence) {
                BSAGElib_P_Rpc_DispatchResponse_isr(remote, BSAGE_ERR_RESET);
            }
        }
    }
}

void
BSAGElib_Management_Reset(
    BSAGElib_Handle hSAGElib)
{
    BDBG_ENTER(BSAGElib_Management_Reset);;

    if (!hSAGElib->resetPending) {
        BDBG_ERR(("%s: cannot reset SAGE, reset condition is not met.", __FUNCTION__));
        goto end;
    }

    hSAGElib->resetPending = 0;

    /* reset/clear RPC and HSI */
    BSAGElib_P_Rpc_Reset(hSAGElib);

    BKNI_EnterCriticalSection();

    BSAGElib_P_Standby_Reset_isrsafe();

    /* invalidate all existing remotes + fire pending callbacks */
    BSAGElib_P_Management_DispatchAllResponse_isrsafe(hSAGElib);

    BKNI_LeaveCriticalSection();

end:
    BDBG_LEAVE(BSAGElib_Management_Reset);
}

/* BHSI_IsrCallbackFunc prototype, see bhsi.h */
static void
BSAGElib_P_Management_WatchdogIntHandler_isr(
    void *parm1,
    int parm2)
{
    BSAGElib_Handle hSAGElib;

    BSTD_UNUSED(parm2);

    hSAGElib = (BSAGElib_Handle)parm1;
    BDBG_ASSERT(hSAGElib);

    BDBG_MSG(("%s: watchdog interrupt, instance=%p", __FUNCTION__, (void *)hSAGElib));

    hSAGElib->resetPending = 1;

    /* invalidate all existing remotes + fire pending callbacks */
    BSAGElib_P_Management_DispatchAllResponse_isrsafe(hSAGElib);

    BSAGElib_P_Rpc_Reset_isrsafe(hSAGElib);
    /* Fire all registered watchdog callbacks */
    {
        BSAGElib_Management_CallbackItem *item;
        for (item = BLST_SQ_FIRST(&hSAGElib->watchdog_callbacks); item; item = BLST_SQ_NEXT(item, link)) {
            BDBG_MSG(("%s: Fire Watchdog callback %08x",
                      __FUNCTION__, (uint32_t)item->watchdog_isr));
            item->watchdog_isr();
        }
    }
}

BERR_Code
BSAGElib_P_Management_Initialize(
    BSAGElib_Handle hSAGElib)
{
    BERR_Code rc;

    BLST_SQ_INIT(&hSAGElib->watchdog_callbacks);

    /* Create interrupt service routine (ISR) for Watchdog Timeout */
    rc = BINT_CreateCallback(&(hSAGElib->watchdogIntCallback),
                             hSAGElib->core_handles.hInt,
                             BCHP_INT_ID_SCPU_HOST_WATCHDOG_TIMEOUT_INTR,
                             BSAGElib_P_Management_WatchdogIntHandler_isr,
                             (void *) hSAGElib,
                             0x00);
    if (rc != BERR_SUCCESS) {
        BDBG_ERR(("%s: BINT_CreateCallback(watchdog) failed!", __FUNCTION__));
        goto end;
    }

    rc = BINT_EnableCallback(hSAGElib->watchdogIntCallback);
    if (rc != BERR_SUCCESS) {
        BDBG_ERR(( "%s: BINT_EnableCallback(Watchdog) failed!", __FUNCTION__));
        goto end;
    }

end:
    return rc;
}

void
BSAGElib_P_Management_Uninitialize(
    BSAGElib_Handle hSAGElib)
{
    if (hSAGElib->watchdogIntCallback) {
        BERR_Code err;
        BINT_DisableCallback(hSAGElib->watchdogIntCallback);
        err = BINT_DestroyCallback(hSAGElib->watchdogIntCallback);
        if (err != BERR_SUCCESS) {
            BDBG_ERR(("%s: BINT_DestroyCallback(watchdog) returns error %u",
                      __FUNCTION__, err));
            (void)BERR_TRACE(BERR_INVALID_PARAMETER) ;
        }
        hSAGElib->watchdogIntCallback = NULL;
    }

    do {
        BSAGElib_Management_CallbackItem *item;
        BKNI_EnterCriticalSection();
        item = BLST_SQ_FIRST(&hSAGElib->watchdog_callbacks);
        if (item) {
            BLST_SQ_REMOVE_HEAD(&hSAGElib->watchdog_callbacks, link);
        }
        BKNI_LeaveCriticalSection();
        if (item) {
            BKNI_Free(item);
            continue;
        }
    } while (0);
}
