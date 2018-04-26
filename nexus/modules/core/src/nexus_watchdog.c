/***************************************************************************
*  Copyright (C) 2018 Broadcom.
*  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
*
*  This program is the proprietary software of Broadcom and/or its licensors,
*  and may only be used, duplicated, modified or distributed pursuant to
*  the terms and conditions of a separate, written license agreement executed
*  between you and Broadcom (an "Authorized License").  Except as set forth in
*  an Authorized License, Broadcom grants no license (express or implied),
*  right to use, or waiver of any kind with respect to the Software, and
*  Broadcom expressly reserves all rights in and to the Software and all
*  intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
*  THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
*  IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
*
*  Except as expressly set forth in the Authorized License,
*
*  1.     This program, including its structure, sequence and organization,
*  constitutes the valuable trade secrets of Broadcom, and you shall use all
*  reasonable efforts to protect the confidentiality thereof, and to use this
*  information only in connection with your use of Broadcom integrated circuit
*  products.
*
*  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
*  "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
*  OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
*  RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
*  IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
*  A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
*  ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
*  THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
*
*  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
*  OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
*  INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
*  RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
*  HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
*  EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
*  WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
*  FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
***************************************************************************/
#include "nexus_core_module.h"
#include "bchp_common.h"

#ifdef BCHP_TIMER_REG_START
#include "bchp_timer.h" /* TODO: we are assuming that this is available on all platforms and the register
                           names are identical. this appears to be the case at least for now */
#include "bchp_sun_top_ctrl.h"
#include "bchp_int_id_timer.h"
#endif

BDBG_MODULE(nexus_watchdog);

#define NEXUS_WATCHDOG_TIMER_FREQ 27000000 /* 27 MHz */

struct NEXUS_WatchdogCallback
{
    NEXUS_OBJECT(NEXUS_WatchdogCallback);
    BINT_CallbackHandle intCallback;
    BKNI_EventHandle event;
    NEXUS_EventCallbackHandle eventCallback;
    NEXUS_TaskCallbackHandle callback;
    NEXUS_WatchdogCallbackSettings settings;
    BLST_S_ENTRY(NEXUS_WatchdogCallback) link;
};

struct NEXUS_Watchdog
{
    NEXUS_OBJECT(NEXUS_Watchdog);
};

#ifdef BCHP_TIMER_REG_START
static struct {
    bool started;
    unsigned timeout;
    BLST_S_HEAD(watchdog_callback_list, NEXUS_WatchdogCallback) callbacks;
    uint32_t resetHistory;
    bool stopTimerOnDestroy;
} g_watchdog;

static NEXUS_WatchdogHandle g_watchdogInterface;

static void NEXUS_Watchdog_P_ReadResetHistory(void);
#endif

void NEXUS_Watchdog_P_Init(void)
{
#ifdef BCHP_TIMER_REG_START
    NEXUS_Error rc;
    BKNI_Memset(&g_watchdog, 0, sizeof(g_watchdog));
    g_watchdog.stopTimerOnDestroy = true;
    NEXUS_Watchdog_P_ReadResetHistory();
    /* must issue magic stop sequence to get control again */
    rc = NEXUS_WatchdogInterface_StopTimer(g_watchdogInterface);
    if (rc) BERR_TRACE(rc); /* keep going */
#endif
}

void NEXUS_Watchdog_P_Uninit(void)
{
#ifdef BCHP_TIMER_REG_START
    if(g_watchdog.stopTimerOnDestroy) {
        NEXUS_Error rc;
        rc = NEXUS_WatchdogInterface_StopTimer(g_watchdogInterface);
        if (rc) BERR_TRACE(rc); /* keep going */
    }
    BKNI_Memset(&g_watchdog, 0, sizeof(g_watchdog));
#endif
}

NEXUS_WatchdogHandle NEXUS_WatchdogInterface_Open( unsigned index )
{
#ifdef BCHP_TIMER_REG_START
    if (g_watchdogInterface) {
        BERR_TRACE(NEXUS_NOT_AVAILABLE);
        return NULL;
    }
    if (index != 0) {
        BERR_TRACE(NEXUS_NOT_AVAILABLE);
        return NULL;
    }
    g_watchdogInterface = BKNI_Malloc(sizeof(*g_watchdogInterface));
    if (!g_watchdogInterface) {
        BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
        return NULL;
    }
    NEXUS_OBJECT_INIT(NEXUS_Watchdog, g_watchdogInterface);
    return g_watchdogInterface;
#else
    BSTD_UNUSED(index);
    return NULL;
#endif
}

static void NEXUS_Watchdog_P_Finalizer( NEXUS_WatchdogHandle watchdog )
{
#ifdef BCHP_TIMER_REG_START
    if(g_watchdog.stopTimerOnDestroy) {
        NEXUS_Error rc;
        rc = NEXUS_WatchdogInterface_StopTimer(watchdog);
        if (rc) BERR_TRACE(rc); /* keep going */
    }
    NEXUS_OBJECT_DESTROY(NEXUS_Watchdog, g_watchdogInterface);
    BKNI_Free(watchdog);
    g_watchdogInterface = NULL;
#else
    BSTD_UNUSED(watchdog);
#endif
}

NEXUS_OBJECT_CLASS_MAKE(NEXUS_Watchdog, NEXUS_WatchdogInterface_Close);

#ifdef BCHP_TIMER_REG_START
static void nexus_p_watchdog_isr(void *context, int param)
{
    NEXUS_WatchdogCallbackHandle handle = context;
    BSTD_UNUSED(param);
    /* level-triggered, so disable till stop */
    BINT_DisableCallback_isr(handle->intCallback);
    /* Don't call NEXUS_IsrCallback_Fire_isr here. By requiring the extra hop through the task callback, we ensure more of Nexus is working
    before firing the midpoint callback, which serves as a heartbeat. */
    BKNI_SetEvent(handle->event);
}

static void nexus_watchdog_p_start_callback(void)
{
    NEXUS_WatchdogCallbackHandle handle;
    for (handle = BLST_S_FIRST(&g_watchdog.callbacks); handle; handle = BLST_S_NEXT(handle, link)) {
        BINT_EnableCallback(handle->intCallback);
    }
}

static void nexus_watchdog_p_stop_callback(void)
{
    NEXUS_WatchdogCallbackHandle handle;
    for (handle = BLST_S_FIRST(&g_watchdog.callbacks); handle; handle = BLST_S_NEXT(handle, link)) {
        BINT_DisableCallback(handle->intCallback);
    }
}
#endif

NEXUS_Error NEXUS_WatchdogInterface_SetTimeout(NEXUS_WatchdogHandle watchdog, unsigned timeout)
{
#ifdef BCHP_TIMER_REG_START
    if (!watchdog && g_watchdogInterface) {
        return BERR_TRACE(NEXUS_NOT_AVAILABLE);
    }

    if (g_watchdog.started) { /* the HW already protects against this, but it doesn't hurt to stop it in SW */
        BDBG_ERR(("NEXUS_Watchdog_SetTimeout: Timeout cannot be set while timer is counting"));
        return BERR_TRACE(NEXUS_NOT_INITIALIZED);
    }

    if (timeout<NEXUS_WATCHDOG_MIN_TIMEOUT || timeout>NEXUS_WATCHDOG_MAX_TIMEOUT) {
        BDBG_ERR(("NEXUS_Watchdog_SetTimeout: Invalid timeout value specified"));
        return BERR_TRACE(NEXUS_NOT_INITIALIZED);
    }

    BDBG_ASSERT(g_NexusCore.publicHandles.reg);
    /* write timeout value to register */
    BREG_Write32(g_NexusCore.publicHandles.reg, BCHP_TIMER_WDTIMEOUT, timeout*NEXUS_WATCHDOG_TIMER_FREQ);

    /* WD_COUNT_MODE 0 = one-shot
       WD_EVENT_MODE 0 = reset (it also gives a maskable midpoint interrupt) */
    BREG_Write32(g_NexusCore.publicHandles.reg, BCHP_TIMER_WDCTRL, 0);

    g_watchdog.timeout = timeout;
    return NEXUS_SUCCESS;
#else
    BSTD_UNUSED(watchdog);
    BSTD_UNUSED(timeout);
    return NEXUS_NOT_SUPPORTED;
#endif
}

NEXUS_Error NEXUS_WatchdogInterface_StartTimer(NEXUS_WatchdogHandle watchdog)
{
#ifdef BCHP_TIMER_REG_START
    if (!watchdog && g_watchdogInterface) {
        return BERR_TRACE(NEXUS_NOT_AVAILABLE);
    }

    if (g_watchdog.timeout==0) {
        BDBG_ERR(("NEXUS_Watchdog_StartTimer: Timeout value was not previously set"));
        return BERR_TRACE(NEXUS_NOT_INITIALIZED);
    }

#if defined(BCHP_SUN_TOP_CTRL_RESET_CTRL_watchdog_reset_enable_SHIFT)
    if (!(BREG_Read32(g_pCoreHandles->reg, BCHP_SUN_TOP_CTRL_RESET_CTRL) & BCHP_SUN_TOP_CTRL_RESET_CTRL_watchdog_reset_enable_MASK)) {
        BDBG_ERR(("NEXUS_Watchdog_StartTimer requires that BCHP_SUN_TOP_CTRL_RESET_CTRL_watchdog_reset_enable be 1. Set this in nexus_platform_pinmux.c or in your application."));
        return BERR_TRACE(NEXUS_NOT_INITIALIZED);
    }
#endif

    nexus_watchdog_p_start_callback();

    /* magic start sequence */
    BREG_Write32(g_NexusCore.publicHandles.reg, BCHP_TIMER_WDCMD, 0xff00);
    BREG_Write32(g_NexusCore.publicHandles.reg, BCHP_TIMER_WDCMD, 0x00ff);

    g_watchdog.started = true;

    return NEXUS_SUCCESS;
#else
    BSTD_UNUSED(watchdog);
    return NEXUS_NOT_SUPPORTED;
#endif
}

NEXUS_Error NEXUS_WatchdogInterface_StopTimer(NEXUS_WatchdogHandle watchdog)
{
#ifdef BCHP_TIMER_REG_START
    if (!watchdog && g_watchdogInterface) {
        return BERR_TRACE(NEXUS_NOT_AVAILABLE);
    }

    /* allow already-stopped timer to re-issue stop. needed for re-init system. */
    /* magic stop sequence */
    BREG_Write32(g_NexusCore.publicHandles.reg, BCHP_TIMER_WDCMD, 0xee00);
    BREG_Write32(g_NexusCore.publicHandles.reg, BCHP_TIMER_WDCMD, 0x00ee);
    nexus_watchdog_p_stop_callback();
    g_watchdog.started = false;
    return NEXUS_SUCCESS;
#else
    BSTD_UNUSED(watchdog);
    return NEXUS_NOT_SUPPORTED;
#endif
}

#ifdef BCHP_TIMER_REG_START
#if defined(BCHP_AON_CTRL_REG_START)
#include "bchp_aon_ctrl.h"
#endif
static void NEXUS_Watchdog_P_ReadResetHistory(void)
{
    BDBG_ASSERT(g_NexusCore.publicHandles.reg);
#if defined(BCHP_AON_CTRL_REG_START)
/* SUN_TOP_CTRL_RESET_HISTORY is NOT USED in chips that have a sys_aon always-on power island module.
   They use the reset history feature in sys_aon. */
    g_watchdog.resetHistory = BREG_Read32(g_NexusCore.publicHandles.reg, BCHP_AON_CTRL_RESET_HISTORY);
    if (g_watchdog.resetHistory) {
        uint32_t ulVal;
        ulVal = BREG_Read32(g_NexusCore.publicHandles.reg, BCHP_AON_CTRL_RESET_CTRL);
        BCHP_SET_FIELD_DATA( ulVal, AON_CTRL_RESET_CTRL, clear_reset_history, 1 );
        BREG_Write32(g_NexusCore.publicHandles.reg, BCHP_AON_CTRL_RESET_CTRL,ulVal); /* clear status */
    }
#else
    g_watchdog.resetHistory = BREG_Read32(g_NexusCore.publicHandles.reg, BCHP_SUN_TOP_CTRL_RESET_HISTORY);
    if (g_watchdog.resetHistory) {
        uint32_t ulVal;
        ulVal = BREG_Read32(g_NexusCore.publicHandles.reg, BCHP_SUN_TOP_CTRL_RESET_CTRL);
        BCHP_SET_FIELD_DATA( ulVal, SUN_TOP_CTRL_RESET_CTRL, clear_reset_history, 1 );
        BREG_Write32(g_NexusCore.publicHandles.reg, BCHP_SUN_TOP_CTRL_RESET_CTRL,ulVal); /* clear status */
    }
#endif
}
#endif

void NEXUS_Watchdog_GetLastResetStatus(bool *pStatus)
{
#if NEXUS_CPU_ARM
    BDBG_LOG(("NEXUS_Watchdog_GetLastResetStatus is not supported on ARM platforms."));
    BDBG_LOG(("Reset status, captured by BOLT, can be retrieved from Device Tree using: cat /proc/device-tree/bolt/reset-list"));
#endif
#ifdef BCHP_TIMER_REG_START
#if defined(BCHP_AON_CTRL_REG_START)
#ifdef BCHP_AON_CTRL_RESET_HISTORY_host_watchdog_timer_reset_MASK
    *pStatus = BCHP_GET_FIELD_DATA(g_watchdog.resetHistory, AON_CTRL_RESET_HISTORY,  host_watchdog_timer_reset);
#else
    *pStatus = BCHP_GET_FIELD_DATA(g_watchdog.resetHistory, AON_CTRL_RESET_HISTORY,  watchdog_timer_reset);
#endif
#else
    *pStatus = BCHP_GET_FIELD_DATA(g_watchdog.resetHistory, SUN_TOP_CTRL_RESET_HISTORY,  watchdog_timer_reset);
#endif
#else
    BSTD_UNUSED(pStatus);
#endif
}

void NEXUS_WatchdogCallback_GetDefaultSettings( NEXUS_WatchdogCallbackSettings *pSettings )
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    NEXUS_CallbackDesc_Init(&pSettings->midpointCallback);
    pSettings->stopTimerOnDestroy = true;
}

#ifdef BCHP_TIMER_REG_START
static void nexus_p_watchdog_event(void *context)
{
    NEXUS_WatchdogCallbackHandle handle = context;
    NEXUS_TaskCallback_Fire(handle->callback);
}
#endif

NEXUS_WatchdogCallbackHandle NEXUS_WatchdogCallback_Create( const NEXUS_WatchdogCallbackSettings *pSettings )
{
#ifdef BCHP_TIMER_REG_START
    NEXUS_WatchdogCallbackHandle handle;
    int rc;
    handle = BKNI_Malloc(sizeof(*handle));
    if (!handle) {
        BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
        return NULL;
    }
    NEXUS_OBJECT_INIT(NEXUS_WatchdogCallback, handle);
    BLST_S_INSERT_HEAD(&g_watchdog.callbacks, handle, link);
    g_watchdog.stopTimerOnDestroy = pSettings->stopTimerOnDestroy;
    handle->settings = *pSettings;
    handle->callback = NEXUS_TaskCallback_Create(handle, NULL);
    if (!handle->callback) {
        BERR_TRACE(NEXUS_UNKNOWN);
        goto error;
    }
    NEXUS_TaskCallback_Set(handle->callback, &pSettings->midpointCallback);
    rc = BKNI_CreateEvent(&handle->event);
    if (rc) {
        BERR_TRACE(rc);
        goto error;
    }
    handle->eventCallback = NEXUS_RegisterEvent(handle->event, nexus_p_watchdog_event, handle);
    if (!handle->eventCallback) {
        BERR_TRACE(NEXUS_UNKNOWN);
        goto error;
    }
    rc = BINT_CreateCallback(&handle->intCallback, g_pCoreHandles->bint, BCHP_INT_ID_WDINT, nexus_p_watchdog_isr, handle, 0);
    if (rc) {
        BERR_TRACE(rc);
        goto error;
    }
    (void)BINT_EnableCallback(handle->intCallback);
    return handle;

error:
    NEXUS_WatchdogCallback_Destroy(handle);
    return NULL;
#else
    BSTD_UNUSED(pSettings);
    return NULL;
#endif
}

static void NEXUS_WatchdogCallback_P_Finalizer( NEXUS_WatchdogCallbackHandle handle )
{
#ifdef BCHP_TIMER_REG_START
    if (handle->intCallback) {
        BINT_DestroyCallback(handle->intCallback);
    }
    if (handle->eventCallback) {
        NEXUS_UnregisterEvent(handle->eventCallback);
    }
    if (handle->event) {
        BKNI_DestroyEvent(handle->event);
    }
    if (handle->callback) {
        NEXUS_TaskCallback_Destroy(handle->callback);
    }
    BLST_S_REMOVE(&g_watchdog.callbacks, handle, NEXUS_WatchdogCallback, link);
    NEXUS_OBJECT_DESTROY(NEXUS_WatchdogCallback, handle);
    BKNI_Free(handle);
#else
    BSTD_UNUSED(handle);
#endif
}

NEXUS_OBJECT_CLASS_MAKE(NEXUS_WatchdogCallback, NEXUS_WatchdogCallback_Destroy);
