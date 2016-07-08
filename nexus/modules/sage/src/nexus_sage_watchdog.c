/***************************************************************************
 *     (c)2013 Broadcom Corporation
 *
 *  This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
 **************************************************************************/

#include "nexus_sage_module.h"
#include "bsagelib_management.h"

BDBG_MODULE(nexus_sage_watchdog);

/* local functions */
static void NEXUS_Sage_WatchdogEventhandler(void *dummy);
static void NEXUS_Sage_P_WatchdogIntHandler_isr(void);

/* Nexus Sage Watchdog uses an EventCallback to treat an interrupt asynchronously */

/* Internal global context */
static struct NEXUS_Sage_Watchdog_P_State {
    BKNI_EventHandle event;
    NEXUS_EventCallbackHandle eventCallback;
    BSAGElib_ManagementInterface i_management;
} g_sage_watchdog;

/* Global to control Watchdog lifecycle */
static int _g_init = 0;


/* This event handler is called whenever watchdogEvent registered event is set.
 * The watchdogEvent is set inside NEXUS_Sage_WatchdogIntHandler_isr()
 * The main idea is that watchdog Event handling is realised in sync
 * i.e. Nexus Sage module is locked in the meanwhile. */
static void NEXUS_Sage_WatchdogEventhandler(void *dummy)
{
    NEXUS_SageHandle sage;
    NEXUS_Error rc;
    unsigned i;

    /* Nexus Sage is locked. Any NEXUS_Sage* API will be waiting until function returns. */

    BSTD_UNUSED(dummy);

    BDBG_MSG(("%s: Reset in progress", __FUNCTION__));

    /* Re-Start SAGE */
    BSAGElib_Management_Reset(g_NEXUS_sageModule.hSAGElib);
    rc = NEXUS_SageModule_P_Start();
    BDBG_ASSERT(rc == NEXUS_SUCCESS);
    if(rc != NEXUS_SUCCESS) {
        BDBG_ERR(("%s - NEXUS_SageModule_P_Start() fails %d", __FUNCTION__, rc));
        /* TODO: define the watchdog error policy */
    }

    /* block until booted */
    if (!NEXUS_Sage_P_CheckSageBooted()) {
        BERR_TRACE(NEXUS_UNKNOWN);
        return;
    }

    /* fire watchdog callbacks (1 per application instance) */
    for (sage = BLST_S_FIRST(&g_NEXUS_sageModule.instances); sage; sage = BLST_S_NEXT(sage, link)) {
        if (sage->watchdogCallback) {
            BDBG_MSG(("FIRE upper layer callback for sage %p", (void *)sage));
            NEXUS_TaskCallback_Fire(sage->watchdogCallback);
        }
        else {
            BDBG_WRN(("NO upper layer callback"));
        }
    }

    for (i=0;i<NEXUS_SAGE_MAX_WATCHDOG_EVENTS;i++) {
        if (g_NEXUS_sageModule.watchdogEvent[i]) {
            BKNI_SetEvent(g_NEXUS_sageModule.watchdogEvent[i]);
        }
    }
}

NEXUS_Error NEXUS_Sage_AddWatchdogEvent_priv(BKNI_EventHandle event)
{
    unsigned i;
    for (i=0;i<NEXUS_SAGE_MAX_WATCHDOG_EVENTS;i++) {
        if (!g_NEXUS_sageModule.watchdogEvent[i]) {
            g_NEXUS_sageModule.watchdogEvent[i] = event;
            return NEXUS_SUCCESS;
        }
    }
    return BERR_TRACE(NEXUS_NOT_AVAILABLE);
}

void NEXUS_Sage_RemoveWatchdogEvent_priv(BKNI_EventHandle event)
{
    unsigned i;
    for (i=0;i<NEXUS_SAGE_MAX_WATCHDOG_EVENTS;i++) {
        if (g_NEXUS_sageModule.watchdogEvent[i] == event) {
            g_NEXUS_sageModule.watchdogEvent[i] = NULL;
            break;
        }
    }
}

/* The ISR callback is registered in HSI and will be fire uppon watchdog interrupt */
static void NEXUS_Sage_P_WatchdogIntHandler_isr(void)
{
    g_NEXUS_sageModule.reset = 1;
    g_NEXUS_sageModule.booted = 0;

    BDBG_MSG(("%s: Reset interrupt", __FUNCTION__));

    BKNI_SetEvent_isr(g_sage_watchdog.event);
}

/* Called once per Nexus Sage instance to (un)register an upper layer watchdog callback
 * NEXUS_Sage_P_WatchdogLink is called in NEXUS_Sage_Open
 * NEXUS_Sage_P_WatchdogUnlink is called in Nexus_Sage_Close */
NEXUS_Error NEXUS_Sage_P_WatchdogLink(NEXUS_SageHandle sage, const NEXUS_CallbackDesc *watchdogCallback)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_CallbackSettings callbackSettings;

    if (!watchdogCallback->callback) {
        BDBG_WRN(("%s: Cannot link Sage instance %p with a watchdog Callback",
                  __FUNCTION__, (void *)sage));
        goto end;
    }

    NEXUS_Callback_GetDefaultSettings(&callbackSettings);
    sage->watchdogCallback = NEXUS_TaskCallback_Create(sage, &callbackSettings);
    if (sage->watchdogCallback == NULL) {
        BDBG_ERR(("NEXUS_TaskCallback_Create failure for watchdogCallback"));
        rc = BERR_TRACE(NEXUS_OS_ERROR);
        goto end;
    }

    NEXUS_TaskCallback_Set(sage->watchdogCallback, watchdogCallback);
    BDBG_MSG(("instance %p : watchdog link %p", (void *)sage, (void *)watchdogCallback));

end:
    return rc;
}
void NEXUS_Sage_P_WatchdogUnlink(NEXUS_SageHandle sage)
{
    if (!sage->watchdogCallback) {
        return;
    }
    NEXUS_TaskCallback_Destroy(sage->watchdogCallback);
    sage->watchdogCallback = NULL;
    BDBG_MSG(("instance %p : watchdog unlink", (void *)sage));
}

/* Called once during Nexus Sage module initialization */
NEXUS_Error NEXUS_Sage_P_WatchdogInit(void)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    BERR_Code magnumRc;

    if (_g_init) {
        BDBG_ERR(("%s: already init", __FUNCTION__));
        rc = BERR_TRACE(NEXUS_INVALID_PARAMETER);
        goto err;
    }
    _g_init = 1;

    BKNI_Memset(&g_sage_watchdog, 0, sizeof(g_sage_watchdog));

    magnumRc = BKNI_CreateEvent(&g_sage_watchdog.event);
    if (magnumRc != BERR_SUCCESS) {
        BDBG_ERR(( "BKNI_CreateEvent(watchdog)" ));
        rc = BERR_TRACE(NEXUS_NOT_INITIALIZED);
        goto err;
    }

    g_sage_watchdog.eventCallback = NEXUS_RegisterEvent(g_sage_watchdog.event,
                                                        NEXUS_Sage_WatchdogEventhandler, NULL);
    if (g_sage_watchdog.eventCallback == NULL) {
        BDBG_ERR(( "NEXUS_RegisterEvent(watchdog) failed!" ));
        rc = BERR_TRACE(NEXUS_NOT_INITIALIZED);
        goto err;
    }

    g_sage_watchdog.i_management.watchdog_isr = NEXUS_Sage_P_WatchdogIntHandler_isr;
    magnumRc = BSAGElib_Management_Register(g_NEXUS_sageModule.hSAGElib,
                                            &g_sage_watchdog.i_management);
    if (magnumRc != BERR_SUCCESS) {
        BDBG_ERR(("%s: Cannot register watchdog callback (%u)", __FUNCTION__, magnumRc));
        rc = BERR_TRACE(NEXUS_INVALID_PARAMETER);
        goto err;
    }

    BDBG_MSG(("watchdog initialized"));
err:
    return rc;
}

/* Called once during Nexus Sage module cleanup (uninitialization) */
void NEXUS_Sage_P_WatchdogUninit(void)
{
    BERR_Code magnumRc;

    if (!_g_init) {
        return;
    }

    magnumRc = BSAGElib_Management_Unregister(g_NEXUS_sageModule.hSAGElib,
                                              &g_sage_watchdog.i_management);
    if (magnumRc != BERR_SUCCESS) {
        BDBG_ERR(("%s: Cannot Unregister watchdog callback (%u)", __FUNCTION__, magnumRc));
        (void)BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }

    if (g_sage_watchdog.eventCallback) {
        NEXUS_UnregisterEvent(g_sage_watchdog.eventCallback);
        g_sage_watchdog.eventCallback = NULL;
    }

    if (g_sage_watchdog.event) {
        BKNI_DestroyEvent(g_sage_watchdog.event);
        g_sage_watchdog.event = NULL;
    }

    _g_init = 0;
}
