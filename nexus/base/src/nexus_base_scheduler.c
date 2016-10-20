/***************************************************************************
*  Broadcom Proprietary and Confidential. (c)2008-2016 Broadcom. All rights reserved.
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
***************************************************************************/
#include "nexus_base.h"
#include "nexus_base_priv.h"
#include "nexus_base_statistics.h"
#include "blst_slist.h"
#include "blst_squeue.h"
#include "blst_list.h"
#include "bkni.h"

#define BDBG_MSG_TRACE(x) /* BDBG_MSG(x) */
BDBG_MODULE(nexus_base_scheduler);

BDBG_OBJECT_ID(NEXUS_EventCallback);


struct NEXUS_EventCallback {
    BDBG_OBJECT(NEXUS_EventCallback)
    /* list sorted by the event */
    BLST_S_ENTRY(NEXUS_EventCallback) list;
    BKNI_EventHandle event;
    NEXUS_ModuleHandle module;
    void (*pCallback)(void *);
    void *pContext;
    bool deleted;
#if NEXUS_P_DEBUG_CALLBACKS
    const char *pFileName;
    unsigned lineNumber;
#endif
};
#if NEXUS_P_DEBUG_CALLBACKS
#define NEXUS_P_CALLBACK_FILENAME(x)    x
#define NEXUS_P_CALLBACK_LINENO(x)    x
#else
#define NEXUS_P_CALLBACK_FILENAME(x)    ""
#define NEXUS_P_CALLBACK_LINENO(x)    0
#endif

BDBG_OBJECT_ID(NEXUS_Timer);

struct NEXUS_Timer {
    BDBG_OBJECT(NEXUS_Timer)
    BLST_S_ENTRY(NEXUS_Timer) list;
    NEXUS_Time time;
    NEXUS_ModuleHandle module;
    void (*pCallback)(void *);
    void *pContext;
    bool deleted;
#if NEXUS_P_DEBUG_CALLBACKS
    const char *pFileName;
    unsigned lineNumber;
#endif
};

typedef enum { NEXUS_P_CallbackType_eTask, NEXUS_P_CallbackType_eIsr } NEXUS_P_CallbackType;

struct NEXUS_CallbackCommon {
    NEXUS_P_CallbackType type;
    NEXUS_P_Scheduler *scheduler;
    bool armed; /* true if callback fired */
    bool queued; /* true if callback queued, -> callaback in the scheduler list */
    bool stopped; /* true if callback stopped */
    bool deleted; /* true is callback deleted */
    NEXUS_CallbackDesc desc;
    BLST_D_ENTRY(NEXUS_CallbackCommon) global_list;
    void *object; /* object associated with the callback */
#if NEXUS_P_DEBUG_CALLBACKS
    const char *pFileName;
    unsigned lineNumber;
    NEXUS_Time startTime;
#endif
};

BDBG_OBJECT_ID(NEXUS_TaskCallback);
struct NEXUS_TaskCallback {
    struct NEXUS_CallbackCommon  common; /* must be first member */
    BLST_SQ_ENTRY(NEXUS_TaskCallback) scheduler_list;
    BDBG_OBJECT(NEXUS_TaskCallback)
};

BDBG_OBJECT_ID(NEXUS_IsrCallback);

struct NEXUS_IsrCallback {
    struct NEXUS_CallbackCommon  common; /* must be first member */
    BLST_S_ENTRY(NEXUS_IsrCallback) list_temp;
    BLST_D_ENTRY(NEXUS_IsrCallback) list;
    bool armed_save; /* saved 'armed' status */
    BDBG_OBJECT(NEXUS_IsrCallback)
};

typedef struct NEXUS_P_SchedulerRequest {
    int timeout;
} NEXUS_P_SchedulerRequest;

typedef struct NEXUS_P_SchedulerResponse {
    BKNI_EventHandle events[5];
    unsigned nevents;
    BERR_Code result;
} NEXUS_P_SchedulerResponse;


BDBG_OBJECT_ID(NEXUS_P_Scheduler);

static struct {
    BLST_D_HEAD(NEXUS_CallbackCommonHead, NEXUS_CallbackCommon) callbacks;
} NEXUS_P_Base_Scheduler_State;

BLST_D_HEAD(NEXUS_P_HeadIsrCallbacks, NEXUS_IsrCallback); /* list of IsrCallbacks */
BLST_S_HEAD(NEXUS_P_HeadIsrCallbacks_Temp, NEXUS_IsrCallback); /* temporary list of armed IsrCallbacks */

struct NEXUS_P_Scheduler {
    BKNI_EventGroupHandle group;
    BKNI_EventHandle control; /* control event */
    BKNI_MutexHandle callback_lock; /* callback that is acquired when callback active */
    bool exit;
    bool timerDirty;
    bool finished;
    uint8_t priority; /* used only for debugging */
    BLST_S_HEAD(head_event, NEXUS_EventCallback) events; /* list of events */
    BLST_S_HEAD(head_timer, NEXUS_Timer) timers; /* sorted list of timers */
    BLST_S_HEAD(head_free_timer, NEXUS_Timer) free_timers; /* store completed/cancelled timers to reduce memory churn */
    BLST_SQ_HEAD(head_task_callbacks, NEXUS_TaskCallback) task_callbacks; /* list of callbacks */
    struct {
        struct NEXUS_P_HeadIsrCallbacks list; /* list of IsrCallbacks */
        unsigned deleted;
        unsigned armed;
    } isr_callbacks;
    NEXUS_ThreadHandle thread;
    BDBG_OBJECT(NEXUS_P_Scheduler)
    struct NEXUS_CallbackCommon *current_callback;
};

const char *
NEXUS_P_PrepareFileName(const char *pFileName)
{
    const char *s;
    unsigned i;

    if(pFileName==NULL) {
        return "";
    }
    for(s=pFileName;*s != '\0';s++) { } /* search forward */

    for(i=0;s!=pFileName;s--) { /* search backward */
        if(*s=='/' || *s=='\\') {
            i++;
            if(i>3) {
                return s+1;
            }
        }
    }
    return pFileName;
}

NEXUS_EventCallbackHandle
NEXUS_Module_RegisterEvent(NEXUS_ModuleHandle module, BKNI_EventHandle event, void (*pCallback)(void *), void *pContext, const char *pFileName, unsigned lineNumber)
{
    BERR_Code rc=BERR_SUCCESS;
    NEXUS_EventCallbackHandle callback;
    NEXUS_EventCallbackHandle cur_event;
    NEXUS_P_Scheduler *scheduler;
    bool need_add_event = true;

    /* must verify module is locked to avoid race */
    BDBG_OBJECT_ASSERT(module, NEXUS_Module);
    BDBG_ASSERT(NEXUS_Module_Assert(module));

    callback = BKNI_Malloc(sizeof(*callback));
    if(!callback) {
        rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto err_alloc;
    }
    BKNI_Memset(callback, 0, sizeof(*callback));
    BDBG_OBJECT_SET(callback, NEXUS_EventCallback);

    callback->event = event;
    callback->pCallback = pCallback;
    callback->pContext = pContext;
#if NEXUS_P_DEBUG_CALLBACKS
    callback->pFileName = NEXUS_P_PrepareFileName(pFileName);
    callback->lineNumber = lineNumber;
#endif
    callback->module = module;
    callback->deleted = false;
    NEXUS_LockModule();
    BDBG_ASSERT(module->scheduler);
    scheduler = module->scheduler;

    /* XXX same event could belong to multiple schedulers */
    for(cur_event = BLST_S_FIRST(&(scheduler->events)); cur_event!=NULL ;cur_event = BLST_S_NEXT(cur_event, list)) {
        if(cur_event->event == event) {
            break;
        }
    }
    if(cur_event==NULL) {
        BLST_S_INSERT_HEAD(&scheduler->events, callback, list);
    } else { /* tuck new event after existing one */
        BLST_S_INSERT_AFTER(&scheduler->events, cur_event, callback, list);
        /* test whether there are alive handlers */
        for(;cur_event && cur_event->event==event; cur_event = BLST_S_NEXT(cur_event, list)) {
            if(cur_event != callback && !cur_event->deleted) {
                need_add_event = false;
            }
        }
    }
    if(need_add_event) {
        rc = BKNI_AddEventGroup(scheduler->group, event);
    }
    NEXUS_UnlockModule();
    if(rc!=BERR_SUCCESS) {
        rc = BERR_TRACE(rc);
        goto err_add;
    }
    return callback;

err_add:
    callback->deleted = true; /* don't free it here, just mark for deletion */
err_alloc:
    return NULL;
    BSTD_UNUSED(pFileName);
    BSTD_UNUSED(lineNumber);
}

void
NEXUS_Module_UnregisterEvent(NEXUS_ModuleHandle module, NEXUS_EventCallbackHandle event, const char *pFileName, unsigned lineNumber)
{
    NEXUS_EventCallbackHandle cur_event;
    NEXUS_P_Scheduler *scheduler;

    /* must verify module is locked to avoid race */
    BDBG_OBJECT_ASSERT(module, NEXUS_Module);
    BDBG_ASSERT(NEXUS_Module_Assert(module));
    
    BSTD_UNUSED(pFileName);
    BSTD_UNUSED(lineNumber);
    BDBG_OBJECT_ASSERT(event, NEXUS_EventCallback);
    BDBG_ASSERT(module->scheduler);
    scheduler = module->scheduler;
    /* XXX same event could belong to multiple schedulers */
    NEXUS_LockModule();
    for(cur_event = BLST_S_FIRST(&(scheduler->events)); cur_event!=NULL ;cur_event = BLST_S_NEXT(cur_event, list)) {
        BDBG_OBJECT_ASSERT(cur_event, NEXUS_EventCallback);
        if(cur_event == event) {
            if(!event->deleted) {
                BKNI_RemoveEventGroup(scheduler->group, event->event);
                event->deleted = true; /* don't free it yet, just mark for deletion. We can't delete it since we could step into the thread that holds reference to this callback */
            } else {
                cur_event = NULL;
            }
            break;
        }
    }
    NEXUS_UnlockModule();
    if(cur_event==NULL) {
        BDBG_ERR(("NEXUS_Module_UnregisterEvent: module %s unregistering stale event:%#lx at %s:%u", module->pModuleName, (unsigned long)event, NEXUS_P_PrepareFileName(pFileName), lineNumber));
    }
    BKNI_SetEvent(scheduler->control); /* wakeup thread to release resources */
    return;
}

NEXUS_TimerHandle
NEXUS_Module_ScheduleTimer(NEXUS_ModuleHandle module, unsigned delayMs, void (*pCallback)(void *),  void *pContext, const char *pFileName, unsigned lineNumber)
{
    /* must verify module is locked to avoid race */
    BDBG_OBJECT_ASSERT(module, NEXUS_Module);
    BDBG_ASSERT(NEXUS_Module_Assert(module));

    return NEXUS_Module_P_ScheduleTimer(module, delayMs, pCallback, pContext, pFileName, lineNumber);
}
    
NEXUS_TimerHandle
NEXUS_Module_P_ScheduleTimer(NEXUS_ModuleHandle module, unsigned delayMs, void (*pCallback)(void *),  void *pContext, const char *pFileName, unsigned lineNumber)
{
    BERR_Code rc;
    NEXUS_TimerHandle timer, prev, cur;
    NEXUS_P_Scheduler *scheduler;

    NEXUS_LockModule();
    BDBG_ASSERT(module->scheduler);
    scheduler = module->scheduler;
    
    timer = BLST_S_FIRST(&scheduler->free_timers);
    if (timer) {
        BLST_S_REMOVE_HEAD(&scheduler->free_timers, list);
    }
    else {
        timer = BKNI_Malloc(sizeof(*timer));
        if(!timer) {
            rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
            goto err_alloc;
        }
    }
    BKNI_Memset(timer, 0, sizeof(*timer));
    BDBG_OBJECT_SET(timer, NEXUS_Timer);

    NEXUS_Time_Get(&timer->time);
    NEXUS_Time_Add(&timer->time, delayMs);
    timer->pCallback = pCallback;
    timer->pContext = pContext;
#if NEXUS_P_DEBUG_CALLBACKS
    timer->pFileName = NEXUS_P_PrepareFileName(pFileName);
    timer->lineNumber = lineNumber;
#endif
    timer->module = module;
    timer->deleted = false;
    
    for(prev=NULL,cur=BLST_S_FIRST(&scheduler->timers);cur;cur=BLST_S_NEXT(cur, list)) {
        BDBG_OBJECT_ASSERT(cur, NEXUS_Timer);
        if (NEXUS_Time_Diff(&cur->time, &timer->time) > 0) {
            break;
        }
        prev = cur;
    }
    BDBG_MSG_TRACE(("NEXUS_Module_ScheduleTimer: %#lx timer %#lx(%#lx) %s:%u (%#lx:%s:%u)", (unsigned long)scheduler, (unsigned long)timer, (unsigned long)timer->pContext, timer->pFileName, timer->lineNumber, (unsigned long)prev, prev==NULL?"FIRST":prev->pFileName, prev?prev->lineNumber:0));
    if (prev==NULL) {
        scheduler->timerDirty = true;
        BLST_S_INSERT_HEAD(&scheduler->timers, timer, list);
        BKNI_SetEvent(scheduler->control); /* wakeup thread right the way */
    } else {
        BLST_S_INSERT_AFTER(&scheduler->timers, prev, timer, list); /* thread would be woken up on its own after timeout expires */
    }
    NEXUS_UnlockModule();
    return timer;
err_alloc:
    NEXUS_UnlockModule();
    return NULL;
    BSTD_UNUSED(pFileName);
    BSTD_UNUSED(lineNumber);
}

void
NEXUS_Module_CancelTimer(NEXUS_ModuleHandle module, NEXUS_TimerHandle timer, const char *pFileName, unsigned lineNumber)
{
    NEXUS_TimerHandle cur;
    NEXUS_P_Scheduler *scheduler;

    /* must verify module is locked to avoid race */
    BDBG_OBJECT_ASSERT(module, NEXUS_Module);
    BDBG_ASSERT(NEXUS_Module_Assert(module));
    
    BSTD_UNUSED(pFileName);
    BSTD_UNUSED(lineNumber);
    BDBG_OBJECT_ASSERT(timer, NEXUS_Timer);
    NEXUS_LockModule();
    BDBG_ASSERT(module->scheduler);
    scheduler = module->scheduler;
    for(cur=BLST_S_FIRST(&scheduler->timers);cur;cur=BLST_S_NEXT(cur, list)) {
        if(cur==timer && !cur->deleted) {
            cur->deleted = true;/* don't free it yet, just mark for deletion. We can't delete it since we could step into the thread that holds reference to this callback */
            break;
        }
    }
    if(cur==NULL) {
        if (!timer->deleted) {
            timer->deleted = true;
        }
        else {
            BDBG_ERR(("NEXUS_Module_CancelTimer: module %s canceling expired timer:%#lx at %s:%u", module->pModuleName, (unsigned long)timer, NEXUS_P_PrepareFileName(pFileName), lineNumber));
        }
    }
    NEXUS_UnlockModule();
    BKNI_SetEvent(scheduler->control); /* wakeup thread to release resources */
    return;
}

static void
NEXUS_P_CallbackCommon_Init(struct NEXUS_CallbackCommon *callback, NEXUS_ModuleHandle module, void *interfaceHandle, NEXUS_P_CallbackType type, const NEXUS_CallbackSettings *pSettings, const char *pFileName, unsigned lineNumber)
{
    BSTD_UNUSED(pSettings);

    BDBG_ASSERT(module->scheduler);
    NEXUS_CallbackDesc_Init(&callback->desc);
    callback->type = type;
    callback->armed = false;
    callback->queued = false;
    callback->stopped = false;
    callback->deleted = false;
    callback->object = interfaceHandle;
#if NEXUS_P_DEBUG_CALLBACKS
    callback->pFileName = NEXUS_P_PrepareFileName(pFileName);
    callback->lineNumber = lineNumber;
#endif
    NEXUS_LockModule();
    callback->scheduler = module->scheduler;
    BLST_D_INSERT_HEAD(&NEXUS_P_Base_Scheduler_State.callbacks, callback, global_list);
    NEXUS_UnlockModule();
    return;
    BSTD_UNUSED(pFileName);
    BSTD_UNUSED(lineNumber);
}

#define NEXUS_P_CALLBACK_DESC_COOKIE_VALUE 0xCA11BACC
void
NEXUS_CallbackDesc_Init(NEXUS_CallbackDesc *desc)
{
    BDBG_ASSERT(desc);
    desc->callback = NULL;
    desc->context = NULL;
    desc->param = 0;
    desc->private_cookie = NEXUS_P_CALLBACK_DESC_COOKIE_VALUE;
    return;
}
NEXUS_CallbackDesc NEXUS_P_CallbackDescByValue(void)
{
    NEXUS_CallbackDesc desc;
    NEXUS_CallbackDesc_Init(&desc);
    return desc;
}

NEXUS_IsrCallbackHandle
NEXUS_Module_IsrCallback_Create(NEXUS_ModuleHandle module,  void *interfaceHandle, const NEXUS_CallbackSettings *pSettings, const char *pFileName, unsigned lineNumber)
{
    NEXUS_IsrCallbackHandle  callback;

    /* must verify module is locked to avoid race */
    BDBG_OBJECT_ASSERT(module, NEXUS_Module);
    BDBG_ASSERT(NEXUS_Module_Assert(module));
    
    BSTD_UNUSED(pSettings);
    BSTD_UNUSED(interfaceHandle);

    callback = BKNI_Malloc(sizeof(*callback));
    if(!callback) {
        BERR_Code rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        BSTD_UNUSED(rc);
        return NULL;
    }
    BDBG_OBJECT_INIT(callback, NEXUS_IsrCallback);
    callback->armed_save = false;
    NEXUS_P_CallbackCommon_Init(&callback->common, module, interfaceHandle, NEXUS_P_CallbackType_eIsr, pSettings, pFileName, lineNumber);

    NEXUS_LockModule();
    BKNI_EnterCriticalSection();
    BLST_D_INSERT_HEAD(&module->scheduler->isr_callbacks.list, callback, list);
    BKNI_LeaveCriticalSection();
    NEXUS_UnlockModule();
    return callback;
}

void
NEXUS_Module_IsrCallback_Destroy(NEXUS_ModuleHandle module, NEXUS_IsrCallbackHandle callback)
{
    NEXUS_P_Scheduler *scheduler;
    NEXUS_IsrCallbackHandle cur;
    
    /* must verify module is locked to avoid race */
    BDBG_OBJECT_ASSERT(module, NEXUS_Module);
    BDBG_ASSERT(NEXUS_Module_Assert(module));
    
    BDBG_OBJECT_ASSERT(callback, NEXUS_IsrCallback);
    NEXUS_LockModule();
    BDBG_ASSERT(module->scheduler);
    scheduler = module->scheduler;
    BKNI_EnterCriticalSection();
    for(cur=BLST_D_FIRST(&scheduler->isr_callbacks.list);cur!=NULL;cur=BLST_D_NEXT(cur, list)) {
        BDBG_OBJECT_ASSERT(cur, NEXUS_IsrCallback);
        if(cur==callback && !callback->common.deleted) {
            /* we can't neither free element or move it from the list, since scheduler thread could hold reference to the element with released lock.
             * So just mark is as deleted, and let thread to find and free it */
            callback->common.deleted = true;
            if(callback->common.armed) {
                BDBG_ASSERT(scheduler->isr_callbacks.armed>0);
                callback->common.armed=false;
                scheduler->isr_callbacks.armed--;
            }
            BLST_D_REMOVE(&scheduler->isr_callbacks.list, callback, list); /* move item to the front of list, it shall be safe, in worst case thread would pass through list multiple times */
            BLST_D_INSERT_HEAD(&scheduler->isr_callbacks.list, callback, list);
            scheduler->isr_callbacks.deleted++;
            break;
        }
    }
    BKNI_LeaveCriticalSection();
    if(cur==NULL) {
        BDBG_OBJECT_ASSERT(callback, NEXUS_IsrCallback);
        BDBG_ERR(("NEXUS_IsrCallback_Destroy: module %s destroying stale IsrCallback:%#lx at %s:%u", module->pModuleName, (unsigned long)callback, NEXUS_P_CALLBACK_FILENAME(callback->common.pFileName), NEXUS_P_CALLBACK_LINENO(callback->common.lineNumber)));
    } else {
        BLST_D_REMOVE(&NEXUS_P_Base_Scheduler_State.callbacks, &callback->common, global_list); /* remove callback from the global list */
    }
    BKNI_SetEvent(scheduler->control); /* wakeup thread to release resources */
    NEXUS_UnlockModule();
}
#define NEXUS_P_FILENAME(str) ((str)?(str):"")

static void NEXUS_Module_P_Callback_Test(const NEXUS_CallbackDesc *pDesc, const char *debug)
{
    if (pDesc && pDesc->private_cookie != NEXUS_P_CALLBACK_DESC_COOKIE_VALUE) {
        BDBG_ERR(("uninitialized NEXUS_CallbackDesc from %s", debug));
    }

#if NEXUS_P_DEBUG_MODULE_LOCKS && NEXUS_P_DEBUG_CALLBACKS
    if(pDesc && pDesc->callback) {
        NEXUS_P_ThreadInfo *info = NEXUS_P_ThreadInfo_Get();
        if(info) {
            unsigned stack_depth = BLIFO_READ_PEEK(&info->stack);
            if(stack_depth>2) { /*  One API sets callbacks for another API */
                if(pDesc->callback != NEXUS_Base_P_CallbackHandler_Dispatch) {
                    const NEXUS_P_LockEntry *entry;

                    /* first determine if there's a passthroughCallbacks exception */                    
                    entry = BLIFO_READ(&info->stack);
                    while(stack_depth>0) {
                        if (entry->module->settings.passthroughCallbacks) break;
                        entry--;
                        stack_depth--;
                    }
                    if (!stack_depth) {
                        /* we have a failure */
                        BDBG_ERR(("Detected callback handler %s set by the nexus code without using NEXUS_CallbackHandler (%u)", debug, stack_depth));
                        BDBG_ERR(("All callbacks handled by the nexus should use NEXUS_CallbackHandler, limited call stack follows"));
                        entry = BLIFO_READ(&info->stack);
                        while(stack_depth>0) {
                            BDBG_ERR(("%s %u:%s(%s:%u)",info->pThreadName, stack_depth, entry->module->pModuleName, NEXUS_P_FILENAME(entry->pFileName), entry->lineNumber));
                            entry--;
                            stack_depth--;
                        }
                    }
                }
            }
        }
    }
#endif
}

void
NEXUS_Module_IsrCallback_Set(NEXUS_IsrCallbackHandle callback, const NEXUS_CallbackDesc *pDesc, const char *debug)
{
    BDBG_OBJECT_ASSERT(callback, NEXUS_IsrCallback);
    NEXUS_Module_P_Callback_Test(pDesc, debug);
    BKNI_EnterCriticalSection();
    if (pDesc) {
        callback->common.desc = *pDesc;
    }
    else {
        callback->common.desc.callback = NULL;
    }
    BKNI_LeaveCriticalSection();
    return;
}

void
NEXUS_Module_IsrCallback_Fire_isr(NEXUS_ModuleHandle module, NEXUS_IsrCallbackHandle callback)
{
    NEXUS_P_Scheduler *scheduler;

    BDBG_OBJECT_ASSERT(module, NEXUS_Module);
    BDBG_OBJECT_ASSERT(callback, NEXUS_IsrCallback);
    BKNI_ASSERT_ISR_CONTEXT();

    if(callback->common.deleted) {
        BDBG_WRN(("NEXUS_IsrCallback_Fire_isr: %#lx using stale callback", (unsigned long)callback));
        /* fall through */
    }
    if(callback->common.desc.callback==NULL) { /* short-circuit empty callbacks */
        goto done;
    }
    BDBG_ASSERT(module->scheduler);
    scheduler = module->scheduler;
    if(!callback->common.armed) {
        callback->common.armed=true;
        scheduler->isr_callbacks.armed++;
    }
    BLST_D_REMOVE(&scheduler->isr_callbacks.list, callback, list); /* move item to the  front of list, it shall be safe, in worst case thread would pass through list multiple times */
    BLST_D_INSERT_HEAD(&scheduler->isr_callbacks.list, callback, list);
    BKNI_SetEvent_isr(scheduler->control); /* wakeup thread to release resources */

done:
    return;
}

void
NEXUS_Callback_GetDefaultSettings( NEXUS_CallbackSettings *settings)
{
    BKNI_Memset(settings, 0, sizeof(*settings));
    settings->priority = NEXUS_ModulePriority_eDefault;
    return;
}

NEXUS_TaskCallbackHandle
NEXUS_Module_TaskCallback_Create(NEXUS_ModuleHandle module, void *interfaceHandle, const NEXUS_CallbackSettings *pSettings, const char *pFileName, unsigned lineNumber)
{
    NEXUS_TaskCallbackHandle callback;

    /* must verify module is locked to avoid race */
    BDBG_OBJECT_ASSERT(module, NEXUS_Module);
    BDBG_ASSERT(NEXUS_Module_Assert(module));
    BDBG_ASSERT(interfaceHandle);

    callback = BKNI_Malloc(sizeof(*callback));
    if(!callback) {
        goto err_alloc;
    }
    BDBG_OBJECT_INIT(callback, NEXUS_TaskCallback);
    NEXUS_P_CallbackCommon_Init(&callback->common, module, interfaceHandle, NEXUS_P_CallbackType_eTask, pSettings, pFileName, lineNumber);
    return callback;

err_alloc:
    return NULL;
}

void
NEXUS_Module_TaskCallback_Destroy( NEXUS_ModuleHandle module, NEXUS_TaskCallbackHandle callback)
{
    NEXUS_P_Scheduler *scheduler;
    
    /* must verify module is locked to avoid race */
    BDBG_OBJECT_ASSERT(module, NEXUS_Module);
    BDBG_ASSERT(NEXUS_Module_Assert(module));
    BDBG_OBJECT_ASSERT(callback, NEXUS_TaskCallback);
    BDBG_ASSERT(callback->common.scheduler == module->scheduler);
    BDBG_ASSERT(!callback->common.deleted);
    NEXUS_LockModule();
    callback->common.deleted = true; /* mark callback as deleted */
    BLST_D_REMOVE(&NEXUS_P_Base_Scheduler_State.callbacks, &callback->common, global_list); /* remove callback from the global list */
    scheduler = module->scheduler;
    if(!callback->common.queued) { /* add callback into the queue */
        callback->common.queued = true;
        BLST_SQ_INSERT_HEAD(&scheduler->task_callbacks, callback, scheduler_list);
    }
    BKNI_SetEvent(scheduler->control); /* wakeup thread to release resources */
    NEXUS_UnlockModule();
    return;
}

void
NEXUS_Module_TaskCallback_Set(NEXUS_TaskCallbackHandle callback, const NEXUS_CallbackDesc *pDesc, const char *debug)
{
    BDBG_OBJECT_ASSERT(callback, NEXUS_TaskCallback);
    NEXUS_Module_P_Callback_Test(pDesc, debug);
    NEXUS_LockModule();
    if (pDesc) {
        callback->common.desc = *pDesc;
    }
    else {
        callback->common.desc.callback = NULL;
    }
    NEXUS_UnlockModule();
    return;
}

void
NEXUS_Module_TaskCallback_Fire(NEXUS_ModuleHandle module, NEXUS_TaskCallbackHandle callback)
{
    BDBG_OBJECT_ASSERT(callback, NEXUS_TaskCallback);
    BDBG_OBJECT_ASSERT(module, NEXUS_Module);
    BDBG_ASSERT(callback->common.scheduler == module->scheduler);
    if(callback->common.deleted) {
        BDBG_WRN(("NEXUS_TaskCallback_Fire: %#lx using stale callback", (unsigned long)callback));
        /* fall through */
    }
    if(callback->common.desc.callback==NULL) {
        return;
    }
    NEXUS_LockModule();
    BDBG_ASSERT(module->scheduler);
    callback->common.armed = true;
    if(!callback->common.stopped && !callback->common.queued) {
        NEXUS_P_Scheduler *scheduler;
        callback->common.queued = true;
        scheduler = callback->common.scheduler;
        BLST_SQ_INSERT_HEAD(&scheduler->task_callbacks, callback, scheduler_list);
        BKNI_SetEvent(scheduler->control); /* wakeup thread */
    }
    NEXUS_UnlockModule();
    return;
}

static void
NEXUS_P_Scheduler_IsrCallbacks(NEXUS_P_Scheduler *scheduler)
{
    NEXUS_IsrCallbackHandle callback, next_callback;
    struct NEXUS_P_HeadIsrCallbacks_Temp deleted_list, armed_list;

    BLST_S_INIT(&deleted_list);
    BLST_S_INIT(&armed_list);

    BKNI_EnterCriticalSection();
    /* 1. Collect deleted and armed callbacks */
    for(callback=BLST_D_FIRST(&scheduler->isr_callbacks.list);
        callback && (scheduler->isr_callbacks.deleted!=0 || scheduler->isr_callbacks.armed!=0);
    ) {
        NEXUS_IsrCallbackHandle next_callback = BLST_D_NEXT(callback, list);
        BDBG_OBJECT_ASSERT(callback, NEXUS_IsrCallback);
        if(callback->common.deleted) {
            /* move callback into the deleted list */
            BLST_D_REMOVE(&scheduler->isr_callbacks.list, callback, list);
            BLST_S_INSERT_HEAD(&deleted_list, callback, list_temp);
            if(callback->common.armed) {
                scheduler->isr_callbacks.armed--;
            }
            scheduler->isr_callbacks.deleted--;
        } else if(callback->common.armed) {
            /* add callback into the armed list */
            callback->common.armed = false;
            scheduler->isr_callbacks.armed--;
            BLST_S_INSERT_HEAD(&armed_list, callback, list_temp);
        }
        callback = next_callback;
    }
    if(scheduler->isr_callbacks.deleted+scheduler->isr_callbacks.armed>0) {
        if(scheduler->isr_callbacks.deleted>0) {
            BDBG_WRN(("NEXUS_P_Scheduler_IsrCallbacks: %p stale deleted counter %u", (void *)scheduler, scheduler->isr_callbacks.deleted));
        }
        if(scheduler->isr_callbacks.armed>0) {
            BDBG_WRN(("NEXUS_P_Scheduler_IsrCallbacks: %p stale armed counter %u", (void *)scheduler, scheduler->isr_callbacks.armed));
        }
        BDBG_ASSERT(scheduler->isr_callbacks.deleted==0 && scheduler->isr_callbacks.armed==0); /* if counters aren't zero there is something wrong with either lists or ISR/TASK synchronization */
        /* code below is only executed in the release version */
        scheduler->isr_callbacks.deleted=0;
        scheduler->isr_callbacks.armed=0;
    }
    BKNI_LeaveCriticalSection();

    /* 2. Recycle deleted callbacks */
    for(callback=BLST_S_FIRST(&deleted_list);callback;) {
        next_callback=BLST_S_NEXT(callback, list_temp);
        BDBG_OBJECT_DESTROY(callback, NEXUS_IsrCallback);
        BKNI_Free(callback);
        callback=next_callback;
    }
    /* 3. Call armed callbacks */
    for(callback=BLST_S_FIRST(&armed_list);callback;callback=BLST_S_NEXT(callback, list_temp)) {
        NEXUS_Callback callbackFunc = callback->common.desc.callback;
        BDBG_OBJECT_ASSERT(callback, NEXUS_IsrCallback);
        if(callbackFunc!=NULL) {  /* Cache data from the callback descriptor prior to releasing lock */
            if(!callback->common.stopped) {
                NEXUS_CallbackDesc desc;
                NEXUS_P_CALLBACK_STATS_STATE();  

                NEXUS_P_CALLBACK_STATS_START();  
                /* ensure we get a coherent desc in case NEXUS_IsrCallback_Set is timesliced in here. */
                BKNI_EnterCriticalSection();
                desc = callback->common.desc;
                BKNI_LeaveCriticalSection();

                callback->armed_save = false;
#if NEXUS_P_DEBUG_CALLBACKS
                NEXUS_Time_Get(&callback->common.startTime);
#endif
                scheduler->current_callback = &callback->common;
                NEXUS_UnlockModule();
                
                /* It's possible that NEXUS_StopCallbacks or NEXUS_IsrCallback_Destroy is run here, so retest the common flags. */
                BKNI_AcquireMutex(scheduler->callback_lock); /* this expected to always succeed, the only exception if NEXUS_StopCallbacks is running */
                if (!callback->common.stopped && !callback->common.deleted && desc.callback) {
                    BDBG_MSG_TRACE(("NEXUS_P_Scheduler_IsrCallbacks: %#lx callback: %#lx (%#lx, %u)", (unsigned long)scheduler,
                        (unsigned long)desc.callback, (unsigned long)desc.context, (unsigned)desc.param));
                    desc.callback(desc.context, desc.param);
                }
                BKNI_ReleaseMutex(scheduler->callback_lock);
                
                NEXUS_LockModule();
                NEXUS_P_CALLBACK_STATS_STOP(IsrCallbacks, callback, desc.callback, NEXUS_P_CALLBACK_FILENAME(callback->common.pFileName), NEXUS_P_CALLBACK_LINENO(callback->common.lineNumber));  
                scheduler->current_callback = NULL;
            } else {
                callback->armed_save = true;
            }
        }
    }
    return;
}

void
NEXUS_P_SchedulerGetInfo(NEXUS_P_Scheduler *scheduler, NEXUS_P_SchedulerInfo *info)
{
    BDBG_OBJECT_ASSERT(scheduler, NEXUS_P_Scheduler);
    info->callback_lock = scheduler->callback_lock;
    return;
}

static bool
NEXUS_P_SchedulerGetRequest(NEXUS_P_Scheduler *scheduler, NEXUS_P_SchedulerRequest *request)
{
    unsigned i;
    long timeout = 0;

    BDBG_OBJECT_ASSERT(scheduler, NEXUS_P_Scheduler);
    NEXUS_LockModule();
    request->timeout = 0;
    if(scheduler->exit) {
        goto done;
    }
    for(i=0;i<4;) {
        NEXUS_Time curtime;
        /* coverity[use_after_free] */
        NEXUS_TimerHandle first_timer = BLST_S_FIRST(&scheduler->timers);
        if(!first_timer) {
            timeout = BKNI_INFINITE;
            break;
        }
        BDBG_OBJECT_ASSERT(first_timer, NEXUS_Timer);
        NEXUS_Time_Get(&curtime);
        timeout = NEXUS_Time_Diff(&first_timer->time, &curtime);
        if(timeout>0 && !first_timer->deleted) {
             break;
        }
        timeout = 0; /* Clear timeout for expired timer callouts */
        BLST_S_REMOVE_HEAD(&scheduler->timers, list);
        if(!first_timer->deleted) {
            NEXUS_P_CALLBACK_STATS_STATE();  
            NEXUS_UnlockModule();
            NEXUS_P_CALLBACK_STATS_START();  
            NEXUS_Module_Lock(first_timer->module);
            if(!first_timer->deleted) {

                BDBG_MSG_TRACE(("NEXUS_P_SchedulerGetRequest: %#lx timer %#lx(%#lx) %s:%u", (unsigned long)scheduler, (unsigned long)first_timer, (unsigned long)first_timer->pContext, first_timer->pFileName, first_timer->lineNumber));
                i++;
                first_timer->pCallback(first_timer->pContext);
            }
            NEXUS_Module_Unlock(first_timer->module);
            NEXUS_LockModule();
            NEXUS_P_CALLBACK_STATS_STOP(Timer,first_timer,first_timer->pCallback,NEXUS_P_CALLBACK_FILENAME(first_timer->pFileName),NEXUS_P_CALLBACK_LINENO(first_timer->lineNumber));
        }
        if(!scheduler->timerDirty) {
            scheduler->timerDirty = false;
        }
        BDBG_OBJECT_DESTROY(first_timer, NEXUS_Timer);
        BLST_S_INSERT_HEAD(&scheduler->free_timers, first_timer, list);
    }
    for(i=0;i<4;) {
        NEXUS_TaskCallbackHandle callback;
        NEXUS_CallbackDesc desc;
        NEXUS_P_CALLBACK_STATS_STATE();  

        callback = BLST_SQ_FIRST(&scheduler->task_callbacks);
        if(!callback) {
            break;
        }
        BDBG_OBJECT_ASSERT(callback, NEXUS_TaskCallback);
        BDBG_ASSERT(callback->common.queued);
        callback->common.queued = false;
        BLST_SQ_REMOVE_HEAD(&scheduler->task_callbacks, scheduler_list);
        if(callback->common.deleted) {
            BDBG_OBJECT_DESTROY(callback, NEXUS_TaskCallback);
            BKNI_Free(callback);
            continue;
        }

        /* ensure we get a coherent desc in case NEXUS_TaskCallback_Set is timesliced in after the NEXUS_UnlockModule(). */
        desc = callback->common.desc;
#if NEXUS_P_DEBUG_CALLBACKS
        NEXUS_Time_Get(&callback->common.startTime);
#endif
        scheduler->current_callback = &callback->common;
        NEXUS_UnlockModule(); /* we release our mutex, before grabbing other */

        /* It's possible that NEXUS_StopCallbacks or NEXUS_TaskCallback_Destroy is run here, so test the common flags. */
        NEXUS_P_CALLBACK_STATS_START();  
        BKNI_AcquireMutex(scheduler->callback_lock); /* this expected to always succeed, the only exception if NEXUS_StopCallbacks is running */
        if(callback->common.armed && !callback->common.stopped && !callback->common.deleted) {
            i++;
            callback->common.armed = false;
            if (desc.callback) {
                desc.callback(desc.context, desc.param);
                BDBG_MSG_TRACE(("NEXUS_P_SchedulerGetRequest: %#lx callback %#lx(%#lx, %u)", (unsigned long)scheduler, (unsigned long)desc.callback, (unsigned long)desc.context, (unsigned)desc.param));
            }
        }
        BKNI_ReleaseMutex(scheduler->callback_lock);
        NEXUS_LockModule();
        NEXUS_P_CALLBACK_STATS_STOP(TaskCallback, callback, desc.callback, NEXUS_P_CALLBACK_FILENAME(callback->common.pFileName), NEXUS_P_CALLBACK_LINENO(callback->common.lineNumber));  
        scheduler->current_callback = NULL;
    }
    if(i==4) {
        timeout = 0; /* Clear timeout if maxed out number of callback callouts */
    }
    if(scheduler->isr_callbacks.armed!=0 || scheduler->isr_callbacks.deleted!=0) {
        NEXUS_P_Scheduler_IsrCallbacks(scheduler);
    }
    request->timeout = timeout;
    NEXUS_UnlockModule();
    return true;

done:
    scheduler->finished = true;
    NEXUS_UnlockModule();
    return false;
}


static bool
NEXUS_P_SchedulerProcessResponse(NEXUS_P_Scheduler *scheduler, const NEXUS_P_SchedulerResponse *response)
{
    unsigned i;
    unsigned events_left;
    NEXUS_EventCallbackHandle cur_event;

    BDBG_OBJECT_ASSERT(scheduler, NEXUS_P_Scheduler);
    NEXUS_LockModule();
    if (scheduler->exit) {
        goto done;
    }
    if (response->result==BERR_TIMEOUT) {
        goto timeout;
    } else if (response->result!=BERR_SUCCESS) {
        goto done;
    }
    /* coverity[use_after_free] */
    for(events_left=response->nevents, cur_event = BLST_S_FIRST(&scheduler->events); cur_event!=NULL && events_left>0; ) {
        if(cur_event->deleted) { /* recycle deleted event handlers */
            NEXUS_EventCallbackHandle deleted_event = cur_event;
            cur_event = BLST_S_NEXT(cur_event, list);
            BLST_S_REMOVE(&scheduler->events, deleted_event, NEXUS_EventCallback, list);
            BDBG_OBJECT_DESTROY(deleted_event, NEXUS_EventCallback);
            BKNI_Free(deleted_event);
            continue;
        }
        for(i=0;i<response->nevents;i++) {
            if(response->events[i]==cur_event->event) {
                do {
                    NEXUS_P_CALLBACK_STATS_STATE();  
                    NEXUS_P_CALLBACK_STATS_START();  
                    /* XXX move "hot" entries to head of the list */
                    NEXUS_UnlockModule();
                    NEXUS_Module_Lock(cur_event->module);
                    if (!cur_event->deleted) {
                        BDBG_MSG_TRACE(("NEXUS_P_SchedulerProcessResponse: %#lx event:%#lx(%#lx)", (unsigned long)scheduler, (unsigned long)cur_event->pCallback, (unsigned long)cur_event->pContext));
                        cur_event->pCallback(cur_event->pContext);
                    }
                    NEXUS_Module_Unlock(cur_event->module);
                    NEXUS_LockModule();
                    NEXUS_P_CALLBACK_STATS_STOP(Event, cur_event, cur_event->pCallback, NEXUS_P_CALLBACK_FILENAME(cur_event->pFileName), NEXUS_P_CALLBACK_LINENO(cur_event->lineNumber));  
                    cur_event = BLST_S_NEXT(cur_event, list);
                } while(cur_event && response->events[i]==cur_event->event); /* traverse through all callback that share the same event */
                events_left--; /* we decrement events_left only for callback event, it has consequence that if control event signalled then we would walk and recycle all deleted events */
                break;
            }
        }
        if(i==response->nevents) {
            cur_event = BLST_S_NEXT(cur_event, list);
        }
    }
timeout:
    NEXUS_UnlockModule();
    return true;
done:
    scheduler->finished = true;
    NEXUS_UnlockModule();
    return false;
}

NEXUS_Error
NEXUS_P_Scheduler_Step(NEXUS_P_Scheduler *scheduler, unsigned timeout, NEXUS_P_Base_Scheduler_Status *status, bool (*complete)(void *context), void *context)
{
    NEXUS_P_SchedulerRequest request;
    NEXUS_P_SchedulerResponse response;
    NEXUS_Time curTime, targetTime;
    int timeDiff;

    BDBG_ASSERT(status);

    NEXUS_Time_Get(&targetTime);
    timeDiff = (int)timeout;
    if(timeDiff<0) {
        timeDiff = 0;
    }
    NEXUS_Time_Add(&targetTime, timeDiff);
    for(;;) {
        bool success;

        success = NEXUS_P_SchedulerGetRequest(scheduler, &request);
        if(!success) {
            goto done;
        }
        status->timeout = request.timeout;
        if(complete && complete(context)) {
            request.timeout = 0;
        } else if((int)timeout != BKNI_INFINITE) {
            NEXUS_Time_Get(&curTime);
            timeDiff = NEXUS_Time_Diff(&targetTime, &curTime);
            /* clock granularity may be as high as 10 msec, so only detect wrap if it exceeds this */
            if(timeDiff>(int)timeout+10) {
                BDBG_WRN(("NEXUS_P_Scheduler_Step:%#lx: time wrap %d:%d", (unsigned long)scheduler, (int)timeDiff, (int)timeout));
                request.timeout = 0; /* force exit from the loop */
            } else if(timeDiff<=0) {
                request.timeout = 0;
            } else if(request.timeout==BKNI_INFINITE || timeDiff < request.timeout) {
                request.timeout = timeDiff;
            }
        }
        BDBG_MSG_TRACE(("NEXUS_P_Scheduler_Step:%#lx: %u(%u)", (unsigned long)scheduler, (unsigned)request.timeout, (unsigned)timeout));
        response.result = BKNI_WaitForGroup(scheduler->group, request.timeout, response.events, sizeof(response.events)/sizeof(*response.events), &response.nevents);
        success = NEXUS_P_SchedulerProcessResponse(scheduler, &response);
        if(!success) {
            goto done;
        }
        if(request.timeout==0) {
            break;
        }
    }
    status->idle = BLST_S_FIRST(&scheduler->events) == NULL && BLST_S_FIRST(&scheduler->timers) == NULL && BLST_SQ_FIRST(&scheduler->task_callbacks) == NULL && BLST_D_FIRST(&scheduler->isr_callbacks.list) == NULL;
    status->exit = false;
    return NEXUS_SUCCESS;
done:
    status->timeout = 0;
    status->idle = false;
    status->exit = true;
    return NEXUS_SUCCESS;
}

#ifdef NO_OS_DIAGS
NEXUS_Error
NEXUS_P_NO_OS_Scheduler_Step(NEXUS_P_Scheduler *scheduler, unsigned timeout, NEXUS_P_Base_Scheduler_Status *status, bool (*complete)(void *context), void *context)
{
    NEXUS_P_SchedulerRequest request;
    NEXUS_P_SchedulerResponse response;

    BDBG_ASSERT(status);

    {
        bool success;

        success = NEXUS_P_SchedulerGetRequest(scheduler, &request);
        if(!success) {
            goto done;
        }
        response.result = BKNI_WaitForGroup(scheduler->group, request.timeout, response.events, sizeof(response.events)/sizeof(*response.events), &response.nevents);
        success = NEXUS_P_SchedulerProcessResponse(scheduler, &response);
        if(!success) {
            goto done;
        }
    }
done:
    status->timeout = 0;
    status->idle = false;
    status->exit = true;
    return NEXUS_SUCCESS;
}

void
NEXUS_P_NO_OS_Scheduler_Thread(void *s)
{
    NEXUS_P_Scheduler *scheduler = s;
    NEXUS_P_Base_Scheduler_Status status;
    NEXUS_P_NO_OS_Scheduler_Step(scheduler, BKNI_INFINITE, &status, NULL, NULL);
}

#endif /* NO_OS_DIAGS */

static void
NEXUS_P_Scheduler_Thread(void *s)
{
    NEXUS_P_Scheduler *scheduler = s;
    NEXUS_P_Base_Scheduler_Status status;

    for(;;) {
        NEXUS_P_Scheduler_Step(scheduler, BKNI_INFINITE, &status, NULL, NULL);
        if(status.exit==true) {break;}
    }
    return;
}


NEXUS_P_Scheduler *
NEXUS_P_Scheduler_Init(NEXUS_ModulePriority priority, const char *name, const NEXUS_ThreadSettings *pSettings)
{
    NEXUS_P_Scheduler *scheduler;
    BERR_Code rc;

    BSTD_UNUSED(name);
    BSTD_UNUSED(pSettings);
    NEXUS_ASSERT_MODULE(); /* we need lock held to task wouldn't start in middle of initialization */

    scheduler = BKNI_Malloc(sizeof(*scheduler));
    if(!scheduler) {
        rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto err_alloc;
    }
    BDBG_OBJECT_INIT(scheduler, NEXUS_P_Scheduler);
    scheduler->priority = priority;
    scheduler->exit = false;
    scheduler->timerDirty = true;
    scheduler->finished = false;
    scheduler->thread = NULL;
    BLST_S_INIT(&scheduler->events);
    BLST_S_INIT(&scheduler->timers);
    BLST_S_INIT(&scheduler->free_timers);
    BLST_SQ_INIT(&scheduler->task_callbacks);
    BLST_D_INIT(&scheduler->isr_callbacks.list);
    scheduler->isr_callbacks.deleted = 0;
    scheduler->isr_callbacks.armed = 0;
    scheduler->current_callback = NULL;
    rc = BKNI_CreateEvent(&scheduler->control);
    if(rc!=BERR_SUCCESS) {
        rc = BERR_TRACE(rc);
        goto err_event;
    }
    rc = BKNI_CreateMutex(&scheduler->callback_lock);
    if(rc!=BERR_SUCCESS) {
        rc = BERR_TRACE(rc);
        goto err_mutex;
    }
    rc = BKNI_CreateEventGroup(&scheduler->group);
    if(rc!=BERR_SUCCESS) {
        rc = BERR_TRACE(rc);
        goto err_group;
    }
    rc = BKNI_AddEventGroup(scheduler->group, scheduler->control);
    if(rc!=BERR_SUCCESS) {
        rc = BERR_TRACE(rc);
        goto err_addevent;
    }
    return scheduler;

err_addevent:
    BKNI_DestroyEventGroup(scheduler->group);
err_group:
    BKNI_DestroyMutex(scheduler->callback_lock);
err_mutex:
    BKNI_DestroyEvent(scheduler->control);
err_event:
    BKNI_Free(scheduler);
err_alloc:
    return NULL;

}

NEXUS_P_Scheduler *
NEXUS_P_Scheduler_Create(NEXUS_ModulePriority priority, const char *name, const NEXUS_ThreadSettings *pSettings)
{
    NEXUS_P_Scheduler *scheduler;
    BERR_Code rc;

    NEXUS_ASSERT_MODULE(); /* we need lock held to task wouldn't start in middle of initialization */

    scheduler = NEXUS_P_Scheduler_Init(priority, name, pSettings);
    if(!scheduler) {
        rc = BERR_TRACE(NEXUS_OS_ERROR);
        goto err_scheduller;
    }
    scheduler->thread = NEXUS_P_Thread_Create(name, NEXUS_P_Scheduler_Thread, scheduler, pSettings);
    if(!scheduler->thread) {
        rc = BERR_TRACE(NEXUS_OS_ERROR);
        goto err_task;
    }
    return scheduler;
err_task:
    scheduler->finished = true;
    NEXUS_P_Scheduler_Destroy(scheduler);
err_scheduller:
    return NULL;
}

void 
NEXUS_P_Scheduler_Stop(NEXUS_P_Scheduler *scheduler)
{
    BDBG_OBJECT_ASSERT(scheduler, NEXUS_P_Scheduler);
    NEXUS_ASSERT_MODULE();
    scheduler->exit=true;
    NEXUS_UnlockModule();
    BKNI_SetEvent(scheduler->control); /* wakeup thread to release resources */
    BDBG_MSG_TRACE(("NEXUS_P_Scheduler_Stop:NEXUS_Thread_Destroy"));
    if(scheduler->thread) {
        NEXUS_Thread_Destroy(scheduler->thread);
        scheduler->thread = NULL;
    }
    NEXUS_LockModule();
}

void
NEXUS_P_Scheduler_Destroy(NEXUS_P_Scheduler *scheduler)
{
    NEXUS_TimerHandle timer;
    NEXUS_EventCallbackHandle event;
    NEXUS_IsrCallbackHandle isr_callback;
    NEXUS_TaskCallbackHandle task_callback;

    BDBG_MSG_TRACE(("NEXUS_P_Scheduler_Destroy:> %#lx", (unsigned long)scheduler));

    BDBG_OBJECT_ASSERT(scheduler, NEXUS_P_Scheduler);
    NEXUS_ASSERT_MODULE();
    
    NEXUS_P_Scheduler_Stop(scheduler); /* should already be called, but call here for backward compat */

    BDBG_MSG_TRACE(("NEXUS_P_Scheduler_Destroy: Free timers"));
    while(NULL!=(timer=BLST_S_FIRST(&scheduler->timers))) {
        if(!timer->deleted) {
            BDBG_WRN(("NEXUS_P_Scheduler_Destroy: %#lx lost timer %#lx (%s:%u)", (unsigned long)scheduler, (unsigned long)timer, NEXUS_P_CALLBACK_FILENAME(timer->pFileName), NEXUS_P_CALLBACK_LINENO(timer->lineNumber)));
        }
        BLST_S_REMOVE_HEAD(&scheduler->timers, list);
        BDBG_OBJECT_DESTROY(timer, NEXUS_Timer);
        BKNI_Free(timer);
    }
    while(NULL!=(timer=BLST_S_FIRST(&scheduler->free_timers))) {
        BLST_S_REMOVE_HEAD(&scheduler->free_timers, list);
        BKNI_Free(timer);
    }
    BDBG_MSG_TRACE(("NEXUS_P_Scheduler_Destroy: Free events"));
    while(NULL!=(event=BLST_S_FIRST(&scheduler->events))) {
        BDBG_MSG_TRACE(("NEXUS_P_Scheduler_Destroy: %#lx Free event(%#lx)", (unsigned long)scheduler, (unsigned long)event));
        if(!event->deleted) {
            BDBG_WRN(("NEXUS_P_Scheduler_Destroy: %#lx lost event %#lx (%s:%u)", (unsigned long)scheduler, (unsigned long)event, NEXUS_P_CALLBACK_FILENAME(event->pFileName), NEXUS_P_CALLBACK_LINENO(event->lineNumber)));
        }
        BLST_S_REMOVE_HEAD(&scheduler->events, list);
        BDBG_OBJECT_DESTROY(event, NEXUS_EventCallback);
        BKNI_Free(event);
    }
    BDBG_MSG_TRACE(("NEXUS_P_Scheduler_Destroy: Free IsrCallback"));
    while(NULL!=(isr_callback=BLST_D_FIRST(&scheduler->isr_callbacks.list))) {
        BDBG_MSG_TRACE(("NEXUS_P_Scheduler_Destroy: %#lx Free IsrCallback(%#lx)", (unsigned long)scheduler, (unsigned long)isr_callback));
        BLST_D_REMOVE_HEAD(&scheduler->isr_callbacks.list, list);
        if(!isr_callback->common.deleted) {
            BDBG_ERR(("NEXUS_P_Scheduler_Destroy: %#lx lost IsrCallback %#lx (%s:%u)", (unsigned long)scheduler, (unsigned long)isr_callback, NEXUS_P_CALLBACK_FILENAME(isr_callback->common.pFileName), NEXUS_P_CALLBACK_LINENO(isr_callback->common.lineNumber)));
            /* do not free. nexus code must be fixed. */
        }
        else {
            BDBG_OBJECT_DESTROY(isr_callback, NEXUS_IsrCallback);
            BKNI_Free(isr_callback);
        }
    }
    BDBG_MSG_TRACE(("NEXUS_P_Scheduler_Destroy: Free TaskCallback"));
    while(NULL!=(task_callback=BLST_SQ_FIRST(&scheduler->task_callbacks))) {
        BDBG_MSG_TRACE(("NEXUS_P_Scheduler_Destroy: %#lx Free TaskCallback(%#lx)", (unsigned long)scheduler, (unsigned long)task_callback));
        if(!task_callback->common.deleted) {
            BDBG_ERR(("NEXUS_P_Scheduler_Destroy: %#lx lost TaskCallback %#lx (%s:%u)", (unsigned long)scheduler, (unsigned long)task_callback, NEXUS_P_CALLBACK_FILENAME(task_callback->common.pFileName), NEXUS_P_CALLBACK_LINENO(task_callback->common.lineNumber)));
        }
        BLST_SQ_REMOVE_HEAD(&scheduler->task_callbacks, scheduler_list);
        if(!task_callback->common.deleted) {
            BDBG_ERR(("NEXUS_P_Scheduler_Destroy: %#lx lost TaskCallback %#lx (%s:%u)", (unsigned long)scheduler, (unsigned long)task_callback, NEXUS_P_CALLBACK_FILENAME(task_callback->common.pFileName), NEXUS_P_CALLBACK_LINENO(task_callback->common.lineNumber)));
            /* do not free. nexus code must be fixed. */
        }
        else {
            BDBG_OBJECT_DESTROY(task_callback, NEXUS_TaskCallback);
            BKNI_Free(task_callback);
        }
    }

    BDBG_MSG_TRACE(("NEXUS_P_Scheduler_Destroy: BKNI_RemoveEventGroup"));
    BKNI_RemoveEventGroup(scheduler->group, scheduler->control);
    BDBG_MSG_TRACE(("NEXUS_P_Scheduler_Destroy: BKNI_DestroyEventGroup"));
    BKNI_DestroyEventGroup(scheduler->group);
    BDBG_MSG_TRACE(("NEXUS_P_Scheduler_Destroy: BKNI_DestroyMutex"));
    BKNI_DestroyMutex(scheduler->callback_lock);
    BDBG_MSG_TRACE(("NEXUS_P_Scheduler_Destroy: BKNI_DestroyEvent"));
    BKNI_DestroyEvent(scheduler->control);
    BDBG_OBJECT_DESTROY(scheduler, NEXUS_P_Scheduler);
    BDBG_MSG_TRACE(("NEXUS_P_Scheduler_Destroy: BKNI_Free"));
    BKNI_Free(scheduler);
    BDBG_MSG_TRACE(("NEXUS_P_Scheduler_Destroy:< %#lx", (unsigned long)scheduler));
    return;
}

void
NEXUS_P_Base_Scheduler_Init(void)
{
    BLST_D_INIT(&NEXUS_P_Base_Scheduler_State.callbacks);
    return;
}

void
NEXUS_P_Base_Scheduler_Uninit(void)
{
    struct NEXUS_CallbackCommon *callback;

    while(NULL!=(callback=BLST_D_FIRST(&NEXUS_P_Base_Scheduler_State.callbacks)))  {
        BDBG_WRN(("leaked task callback: %p", (void *)callback));
        if(callback->type == NEXUS_P_CallbackType_eTask) {
            BDBG_OBJECT_ASSERT((NEXUS_TaskCallbackHandle)callback, NEXUS_TaskCallback);
        } else if(callback->type == NEXUS_P_CallbackType_eIsr) {
            BDBG_OBJECT_ASSERT((NEXUS_IsrCallbackHandle)callback, NEXUS_IsrCallback);
        } else {
            BDBG_ERR(("illegal scheduler callback type = %x", callback->type));
            BDBG_ASSERT(0);
        }
        BLST_D_REMOVE_HEAD(&NEXUS_P_Base_Scheduler_State.callbacks, global_list);
        BDBG_WRN(("leaked callback: %#lx scheduler:%#lx object:%#lx allocated at:%s:%u", (unsigned long)callback, (unsigned long)callback->scheduler, (unsigned long)callback->object, NEXUS_P_CALLBACK_FILENAME(callback->pFileName), NEXUS_P_CALLBACK_LINENO(callback->lineNumber)));
        if(callback->type == NEXUS_P_CallbackType_eTask) {
            BDBG_OBJECT_DESTROY((NEXUS_TaskCallbackHandle)callback, NEXUS_TaskCallback);
        } else if(callback->type == NEXUS_P_CallbackType_eIsr) {
            BDBG_OBJECT_DESTROY((NEXUS_IsrCallbackHandle)callback, NEXUS_IsrCallback);
        }
        BKNI_Free(callback);
    }
    return;
}

void
NEXUS_Base_P_StopCallbacks(void *interfaceHandle)
{
    NEXUS_P_Scheduler *schedulers[NEXUS_ModulePriority_eMax]; /* list of schedulers */
    unsigned nschedulers;
    unsigned i;
    NEXUS_P_Scheduler *scheduler;
    struct NEXUS_CallbackCommon *callback;

    NEXUS_LockModule();
    for(nschedulers=0, callback=BLST_D_FIRST(&NEXUS_P_Base_Scheduler_State.callbacks); callback; callback=BLST_D_NEXT(callback, global_list)) {

        if(callback->object != interfaceHandle) {
            continue;
        }
        callback->stopped = true;
        
        scheduler = callback->scheduler;
        if (callback != scheduler->current_callback) {
            /* if this callback isn't current in the scheduler, we don't have to synchronize */
            continue;
        }
        
        for(i=0;i<nschedulers;i++) {
            if(schedulers[i]==scheduler) {
                goto duplicate;
            }
        }
        if(nschedulers==sizeof(schedulers)/sizeof(*schedulers)) {
            BDBG_ERR(("NEXUS_Base_P_StopCallbacks: overflow of scheduler array:%u", nschedulers));
            /* This should never happen, but if it does don't overflow array. */
            break;
        }
        schedulers[nschedulers] = scheduler;
        nschedulers++;
duplicate:
        ;
    }
    NEXUS_UnlockModule(); /* release module lock */

    /* here all relevant callbacks are marked as 'stopped'. now, we must synchronize with every scheduler
    where the callback was current. this could be zero, one or more than one.
    to ensure this function returns with no stopped callback still running on any scheduler. */
    for(i=0;i<nschedulers;i++) {
        scheduler = schedulers[i];
        /* a simple acquire/release pair is enough to ensure that any active callback (which may include a stopped callback) completes.
        if you find that your application has deadlocked here, please debug your application and don't relax this code.
        for instance, you may be trying to close an interface inside a callback, or your close function might be protected with the same
        mutex which is being acquired inside a callback. both of these cases will result in a deadlock here. */
        BKNI_AcquireMutex(scheduler->callback_lock);
        BKNI_ReleaseMutex(scheduler->callback_lock);
    }
    /* now we know that no stopped callbacks are running on any scheduler. */
    return;
}

void
NEXUS_Base_P_StartCallbacks(void *interfaceHandle)
{
    NEXUS_P_Scheduler *scheduler;
    struct NEXUS_CallbackCommon *callback;

    NEXUS_LockModule();
    for(callback=BLST_D_FIRST(&NEXUS_P_Base_Scheduler_State.callbacks); callback; callback=BLST_D_NEXT(callback, global_list)) {

        if(callback->object != interfaceHandle) {
            continue;
        }
        if(!callback->stopped) {
            continue;
        }
        callback->stopped = false;
        scheduler = callback->scheduler;
        if(callback->type == NEXUS_P_CallbackType_eTask) {
            BDBG_OBJECT_ASSERT((NEXUS_TaskCallbackHandle)callback, NEXUS_TaskCallback);
            if(callback->armed && !callback->queued) {
                callback->queued = true;
                BDBG_OBJECT_ASSERT((NEXUS_TaskCallbackHandle)callback, NEXUS_TaskCallback);
                BLST_SQ_INSERT_HEAD(&scheduler->task_callbacks, (NEXUS_TaskCallbackHandle)callback, scheduler_list);
                BKNI_SetEvent(scheduler->control); /* wakeup thread */
            }
        } else if(callback->type == NEXUS_P_CallbackType_eIsr) {
            BDBG_OBJECT_ASSERT((NEXUS_IsrCallbackHandle)callback, NEXUS_IsrCallback);
            if(((NEXUS_IsrCallbackHandle)callback)->armed_save) {
                ((NEXUS_IsrCallbackHandle)callback)->armed_save = false;
                BKNI_EnterCriticalSection();
                if(!callback->armed) {
                    callback->armed=true;
                    scheduler->isr_callbacks.armed++;
                }
                BLST_D_REMOVE(&scheduler->isr_callbacks.list, (NEXUS_IsrCallbackHandle)callback, list); /* move item to the front of list, it shall be safe, in worst case thread would pass through list multiple times */
                BLST_D_INSERT_HEAD(&scheduler->isr_callbacks.list, (NEXUS_IsrCallbackHandle)callback, list);
                BKNI_LeaveCriticalSection();
                BKNI_SetEvent(scheduler->control); /* wakeup thread to release resources */
            }
        }
    }
    NEXUS_UnlockModule();
    return;
}

#if NEXUS_P_DEBUG_CALLBACKS
void NEXUS_P_Base_CheckForStuckCallback(const NEXUS_P_Scheduler *scheduler)
{
    NEXUS_LockModule();
    if (scheduler->current_callback) {
        NEXUS_Time now;
        unsigned duration;
        NEXUS_Time_Get(&now);
        duration = NEXUS_Time_Diff(&now, &scheduler->current_callback->startTime);
        if (duration > 5000) {
            BDBG_ERR(("stuck callback %s:%d for %d msec, module priority %d", scheduler->current_callback->pFileName, scheduler->current_callback->lineNumber, duration,
                scheduler->priority));
        }
    }
    NEXUS_UnlockModule();
}
#endif
