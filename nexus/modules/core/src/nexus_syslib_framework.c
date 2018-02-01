/***************************************************************************
 *  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 * Module Description:
 *
 **************************************************************************/

#include "nexus_base.h"
#include "nexus_core_module.h"
#include "priv/nexus_syslib_framework.h"
#include "bsyslib.h"

/*
The syslib framework must be synchronized with the module that uses its functionality.
Therefore a module handle is required for any usage of this framework.
*/
#if BDBG_DEBUG_BUILD
#define NEXUS_SYSlib_RegisterEvent(module, event, callback, context) \
    NEXUS_Module_RegisterEvent(module, event, callback, context, BSTD_FILE, BSTD_LINE)
#define NEXUS_SYSlib_UnregisterEvent(module, event) \
    NEXUS_Module_UnregisterEvent(module, event, BSTD_FILE, BSTD_LINE)
#define NEXUS_SYSlib_ScheduleTimer(module, delayMs, callback, context) \
    NEXUS_Module_ScheduleTimer(module, delayMs, callback, context, BSTD_FILE, BSTD_LINE)
#define NEXUS_SYSlib_CancelTimer(module, timer) \
    NEXUS_Module_CancelTimer(module, timer, BSTD_FILE, BSTD_LINE)
#else
#define NEXUS_SYSlib_RegisterEvent(module, event, callback, context) \
    NEXUS_Module_RegisterEvent(module, event, callback, context, NULL, 0)
#define NEXUS_SYSlib_UnregisterEvent(module, event) \
    NEXUS_Module_UnregisterEvent(module, event, NULL, 0)
#define NEXUS_SYSlib_ScheduleTimer(module, delayMs, callback, context) \
    NEXUS_Module_ScheduleTimer(module, delayMs, callback, context, NULL, 0)
#define NEXUS_SYSlib_CancelTimer(module, timer) \
    NEXUS_Module_CancelTimer(module, timer, NULL, 0)
#endif

BDBG_MODULE(nexus_syslib_framework);

static void NEXUS_SYSlib_P_TimerHandler(void *context);
static void NEXUS_SYSlib_P_ScheduleTimerHandler(void *context);

struct NEXUS_SYSlib_Context
{
    void * module;
};

struct NEXUS_SYSlib_Timer {
    void * module;
    BKNI_EventHandle event;
    NEXUS_EventCallbackHandle eventHandler;
    NEXUS_TimerHandle nTimer;
    BSYSlib_Timer_Settings settings;
};

void NEXUS_SYSlib_P_GetTime_isr(void * pvParm1, int iParm2, unsigned long * pulNow)
{
    NEXUS_Time now;
    /* these statics are not affected by module synchronization */
    static NEXUS_Time beginning;
    static bool beginningSet = false;

    BSTD_UNUSED(pvParm1);
    BSTD_UNUSED(iParm2);
    if (!beginningSet) {
        NEXUS_Time_Get(&beginning);
        beginningSet = true;
    }
    NEXUS_Time_Get(&now);
    *pulNow = NEXUS_Time_Diff(&now, &beginning);
    return;
}

BERR_Code NEXUS_SYSlib_P_CreateTimer(void * pvParm1, int iParm2, BSYSlib_Timer_Handle *phTimer)
{
    struct NEXUS_SYSlib_Timer *pTimer;
    NEXUS_SYSlib_ContextHandle context = pvParm1;
    BSTD_UNUSED(iParm2);
    *phTimer = pTimer = BKNI_Malloc(sizeof(struct NEXUS_SYSlib_Timer));
    if (!pTimer) {
        return BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
    }
    BKNI_Memset(pTimer, 0, sizeof(*pTimer));

    BKNI_CreateEvent(&pTimer->event);
    pTimer->eventHandler = NEXUS_SYSlib_RegisterEvent(context->module, pTimer->event, NEXUS_SYSlib_P_ScheduleTimerHandler, pTimer);
    pTimer->module = context->module;

    return 0;
}

void NEXUS_SYSlib_P_DestroyTimer(void * pvParm1, int iParm2, BSYSlib_Timer_Handle hTimer)
{
    struct NEXUS_SYSlib_Timer *pTimer = (struct NEXUS_SYSlib_Timer *)hTimer;
    BSTD_UNUSED(pvParm1);
    BSTD_UNUSED(iParm2);
    if (pTimer->nTimer) {
        NEXUS_SYSlib_CancelTimer(pTimer->module, pTimer->nTimer);
    }
    NEXUS_SYSlib_UnregisterEvent(pTimer->module, pTimer->eventHandler);
    BKNI_DestroyEvent(pTimer->event);
    BKNI_Free(pTimer);
}

BERR_Code NEXUS_SYSlib_P_StartTimer_isr(void * pvParm1, int iParm2,  BSYSlib_Timer_Handle hTimer, const BSYSlib_Timer_Settings * psSettings)
{
    struct NEXUS_SYSlib_Timer *pTimer = (struct NEXUS_SYSlib_Timer *)hTimer;
    BSTD_UNUSED(pvParm1);
    BSTD_UNUSED(iParm2);
    pTimer->settings = *psSettings;
    /* convert from isr to task so we can schedule the timer */
    BKNI_SetEvent(pTimer->event);
    return 0;
}

static void NEXUS_SYSlib_P_ScheduleTimerHandler(void *context)
{
    struct NEXUS_SYSlib_Timer *pTimer = (struct NEXUS_SYSlib_Timer *)context;
    if (pTimer->nTimer) {
        NEXUS_SYSlib_CancelTimer(pTimer->module, pTimer->nTimer);
    }
    pTimer->nTimer = NEXUS_SYSlib_ScheduleTimer(pTimer->module, pTimer->settings.ulTimeout, NEXUS_SYSlib_P_TimerHandler, pTimer);
}

static void NEXUS_SYSlib_P_TimerHandler(void *context)
{
    struct NEXUS_SYSlib_Timer *pTimer = (struct NEXUS_SYSlib_Timer *)context;
    /* save callback to local variable because CancelTimer_isr will NULL it out */
    BSYSlib_Timer_ExpiryHandler callback = pTimer->settings.pfTimerExpired;

    pTimer->nTimer = NULL;
    if (callback) {
        BDBG_MSG(("fire timer %p", (void *)pTimer));
        (*callback)(pTimer->settings.pvParm1, pTimer->settings.iParm2, pTimer);
    }
}

void NEXUS_SYSlib_P_CancelTimer_isr(void * pvParm1, int iParm2, BSYSlib_Timer_Handle hTimer)
{
    struct NEXUS_SYSlib_Timer *pTimer = (struct NEXUS_SYSlib_Timer *)hTimer;
    BSTD_UNUSED(pvParm1);
    BSTD_UNUSED(iParm2);
    /* can't cancel with nexus at isr time, so just wipe out the callback */
    pTimer->settings.pfTimerExpired = NULL;
    BDBG_MSG(("cancel timer %p", (void *)pTimer));
    return;
}

void NEXUS_SYSlib_GetDefaultContextSettings_priv(
    NEXUS_SYSlib_ContextSettings * pSettings
)
{
    BDBG_ASSERT(pSettings);
    pSettings->module = NULL;
}

NEXUS_Error NEXUS_SYSlib_CreateContext_priv(NEXUS_SYSlib_ContextHandle * pContext, const NEXUS_SYSlib_ContextSettings * pSettings)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_SYSlib_ContextHandle context;

    BDBG_ASSERT(pSettings);

    context = (NEXUS_SYSlib_ContextHandle)BKNI_Malloc(sizeof(struct NEXUS_SYSlib_Context));
    if (!context) {
        return BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
    }
    BKNI_Memset(context, 0, sizeof(struct NEXUS_SYSlib_Context));

    BDBG_ASSERT(pSettings->module); /* it is a programming error to leave this NULL */
    context->module = pSettings->module;

    *pContext = context;

    return rc;
}

void NEXUS_SYSlib_DestroyContext_priv(NEXUS_SYSlib_ContextHandle context)
{
    BDBG_ASSERT(context);

    BKNI_Free(context);
}

#if 0
/* test code */
static BERR_Code NEXUS_SYSlib_P_Expired(void *context, int parm, BSYSlib_Timer_Handle timer)
{
    BDBG_ASSERT(context == NULL);
    BDBG_ASSERT(parm == 3);
    BSTD_UNUSED(timer);
    BKNI_Printf("NEXUS_SYSlib_P_Expired\n");
    return 0;
}

void NEXUS_SYSlib_P_Test()
{
    BERR_Code rc;
    BSYSlib_Timer_Handle timer;
    BSYSlib_Timer_Settings settings;
    unsigned i;

    rc = NEXUS_SYSlib_P_CreateTimer(NULL, 1, &timer);
    BDBG_ASSERT(!rc);

    settings.pfTimerExpired = NEXUS_SYSlib_P_Expired;
    settings.pvParm1 = NULL;
    settings.iParm2 = 3;

    for (i=0;i<10;i++) {
        settings.ulTimeout = 1000;
        BKNI_EnterCriticalSection();
        rc = NEXUS_SYSlib_P_StartTimer_isr(NULL, 2, timer, &settings);
        BKNI_LeaveCriticalSection();
        BDBG_ASSERT(!rc);
        BKNI_Sleep(2000);
        BKNI_EnterCriticalSection();
        NEXUS_SYSlib_P_CancelTimer_isr(NULL, 2, timer);
        BKNI_LeaveCriticalSection();

        settings.ulTimeout = 5000;
        BKNI_EnterCriticalSection();
        rc = NEXUS_SYSlib_P_StartTimer_isr(NULL, 2, timer, &settings);
        BKNI_LeaveCriticalSection();
        BDBG_ASSERT(!rc);
        BKNI_Sleep(2000);
        BKNI_EnterCriticalSection();
        NEXUS_SYSlib_P_CancelTimer_isr(NULL, 2, timer);
        BKNI_LeaveCriticalSection();
    }

    NEXUS_SYSlib_P_DestroyTimer(NULL, 1, timer);
}
#endif
