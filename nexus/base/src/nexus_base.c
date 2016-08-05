/***************************************************************************
*  Copyright (C) 2003-2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
* $brcm_Workfile: $
* $brcm_Revision: $
* $brcm_Date: $
*
* API Description:
*
* Revision History:
*
* $brcm_Log: $
*
***************************************************************************/
#include "nexus_base.h"
#include "nexus_base_priv.h"
#include "nexus_base_os_types.h"
#include "nexus_base_statistics.h"
#include "bkni_multi.h"
#include "b_objdb.h"

#define BDBG_MSG_TRACE(x) BDBG_MSG(x)

BDBG_MODULE(nexus_base);


#define NEXUS_P_THREAD_ID_COMPARE(node, key) (((uint8_t *)(key) > (uint8_t *)(node)->threadId) ? 1 : ( ((uint8_t *)(key) < (uint8_t *)(node)->threadId) ? -1 : 0))
BLST_AA_TREE_GENERATE_REMOVE(NEXUS_P_ThreadInfoTree, NEXUS_P_ThreadInfo, node)
BLST_AA_TREE_GENERATE_INSERT(NEXUS_P_ThreadInfoTree, void *, NEXUS_P_ThreadInfo, node, NEXUS_P_THREAD_ID_COMPARE)
BLST_AA_TREE_GENERATE_FIND(NEXUS_P_ThreadInfoTree, void *, NEXUS_P_ThreadInfo, node, NEXUS_P_THREAD_ID_COMPARE)

struct NEXUS_P_Base_State NEXUS_P_Base_State;

static const char NEXUS_P_Base_Name[] = "NEXUS_Base";

void
NEXUS_Module_GetDefaultSettings(NEXUS_ModuleSettings *pSettings)
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->priority = NEXUS_ModulePriority_eDefault;
}

BDBG_OBJECT_ID(NEXUS_Module);

NEXUS_ModuleHandle NEXUS_Base=NULL;

#if BDBG_DEBUG_BUILD
static const char *
NEXUS_P_Base_StrChr(const char *str, char c)
{
    for(;;str++) {
        char ch = *str;
        if( ch==c) {
            return str;
        } else if (ch=='\0') {
            return NULL;
        }
    }
}

static char *
NEXUS_P_Base_Strncpy(char *dst, const char *src, size_t len)
{
    char *buf;

    for(buf=dst;len>0;buf++,src++) {
        char ch = *src;
        *buf=ch;
        len--;
        if(ch=='\0') {
            break;
        }
    }
    for(;len>0;len--,buf++) {
        *buf='\0';
    }
    return dst;
}

static void
NEXUS_P_Base_SetModuleDebugLevel(const char *modulelist, BDBG_Level level)
{
    if(!modulelist) {
        return;
    }
    for(;;) {
        char buf[64];
        const char *end = NEXUS_P_Base_StrChr(modulelist, ',');
        size_t name_len;

        if (!end) {
            if(*modulelist) {
                BDBG_SetModuleLevel(modulelist, level);
            }
            break;
        }
        name_len = end-modulelist;
        if(name_len>0 && name_len<sizeof(buf)) {
            NEXUS_P_Base_Strncpy(buf, modulelist, name_len);
            buf[name_len] = '\0';
            BDBG_SetModuleLevel(buf, level);
        }
        modulelist = end+1;
    }
    return;
}
#endif /* BDBG_DEBUG_BUILD */

static const char *NEXUS_P_Scheduler_names[NEXUS_ModulePriority_eMax] = {
    "nx_sched_idle",
    "nx_sched_low",
    "nx_sched",
    "nx_sched_high",
    "nx_sched_idle_stndby",
    "nx_sched_low_stndby",
    "nx_sched_stndby",
    "nx_sched_high_stndby"
};

void
NEXUS_CallbackDesc_Init(NEXUS_CallbackDesc *desc)
{
    BDBG_ASSERT(desc);
    desc->callback = NULL;
    desc->context = NULL;
    desc->param = 0;
    return;
}

NEXUS_ModuleHandle
NEXUS_Module_Create(const char *pModuleName, const NEXUS_ModuleSettings *pSettings)
{
    NEXUS_ModuleHandle module;
    NEXUS_ModuleHandle prev,cur;
    BERR_Code rc;
    NEXUS_ModuleSettings defaultSettings;
    static unsigned g_order = 0;

    BDBG_ASSERT(pModuleName);

    if(pModuleName!=NEXUS_P_Base_Name) {
        NEXUS_LockModule();
    }

    if(pSettings==NULL) {
        NEXUS_Module_GetDefaultSettings(&defaultSettings);
        pSettings = &defaultSettings;
    }

    if ((unsigned)pSettings->priority >= sizeof(NEXUS_P_Base_State.schedulers)/sizeof(NEXUS_P_Base_State.schedulers[0])) {
        rc = BERR_TRACE(BERR_INVALID_PARAMETER);
        goto err_paramcheck;
    }

    module = BKNI_Malloc(sizeof(*module));
    if(!module) {
        rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto err_alloc;
    }
    BKNI_Memset(module, 0, sizeof(*module));
    BDBG_OBJECT_SET(module, NEXUS_Module);
    module->settings = *pSettings;
    module->order = g_order++;
    module->pModuleName = pModuleName;
    rc = BKNI_CreateMutex(&module->lock);
    if(rc!=BERR_SUCCESS) {
        rc = BERR_TRACE(rc);
        goto err_lock;
    }
    if(pModuleName!=NEXUS_P_Base_Name) {
        if(NEXUS_P_Base_State.schedulers[pSettings->priority]==NULL) { /* create scheduler on demand */
           NEXUS_P_Base_State.schedulers[pSettings->priority] = NEXUS_P_Scheduler_Create(pSettings->priority, NEXUS_P_Scheduler_names[pSettings->priority], &NEXUS_P_Base_State.settings.threadSettings[pSettings->priority]);
           if(NEXUS_P_Base_State.schedulers[pSettings->priority]==NULL) {
               rc = BERR_TRACE(BERR_OS_ERROR);
               goto err_scheduler;
           }
        }
        module->scheduler = NEXUS_P_Base_State.schedulers[pSettings->priority];
    } else {
        module->scheduler = NULL;
    }
    /* insert into the sorted list */
    for(prev=NULL, cur=BLST_S_FIRST(&NEXUS_P_Base_State.modules); cur!=NULL; cur=BLST_S_NEXT(cur, link)) {
        int cmp;
        cmp = NEXUS_P_Base_StrCmp(pModuleName, cur->pModuleName);
        if(cmp<0) {
            break; /* Bingo */
        } else if(cmp==0) {
            BDBG_ERR(("NEXUS_Module_Create: duplicated module name"));
            rc = BERR_TRACE(BERR_INVALID_PARAMETER);
            goto err_name;
        }
        prev = cur;
    }
    if(prev) {
        BLST_S_INSERT_AFTER(&NEXUS_P_Base_State.modules, prev, module, link);
    } else {
        BLST_S_INSERT_HEAD(&NEXUS_P_Base_State.modules, module, link);
    }

    if (NEXUS_P_Base_State.settings.driverModuleInit) {
        (NEXUS_P_Base_State.settings.driverModuleInit)(NEXUS_P_Base_State.settings.procContext, module, pModuleName, pSettings);
    }

    module->enabled = true;

    if(pModuleName!=NEXUS_P_Base_Name) {
        NEXUS_UnlockModule();
    }

    BDBG_MSG(("Creating module %s, priority %d", pModuleName, pSettings->priority));
    return module;
err_name:
err_scheduler:
    BKNI_DestroyMutex(module->lock);
err_lock:
    BKNI_Free(module);
err_alloc:
err_paramcheck:
    if(pModuleName!=NEXUS_P_Base_Name) {
        NEXUS_UnlockModule();
    }
    return NULL;
}


void
NEXUS_Module_Destroy(NEXUS_ModuleHandle module)
{
    BDBG_OBJECT_ASSERT(module, NEXUS_Module);
    NEXUS_LockModule();

    if (NEXUS_P_Base_State.settings.driverModuleUninit) {
        (NEXUS_P_Base_State.settings.driverModuleUninit)(NEXUS_P_Base_State.settings.procContext, module, module->pModuleName, &module->settings);
    }

    BDBG_MSG(("Destroying module %s", module->pModuleName));

    module->enabled = false;
    BLST_S_REMOVE(&NEXUS_P_Base_State.modules, module, NEXUS_Module, link);
    NEXUS_UnlockModule();
    BKNI_DestroyMutex(module->lock);
    BDBG_OBJECT_DESTROY(module, NEXUS_Module);
    BKNI_Free(module);
    return;
}

const char *NEXUS_Module_GetName(NEXUS_ModuleHandle module)
{
    BDBG_OBJECT_ASSERT(module, NEXUS_Module);
    return module->pModuleName;
}

unsigned NEXUS_Module_GetOrder(NEXUS_ModuleHandle module)
{
    BDBG_OBJECT_ASSERT(module, NEXUS_Module);
    return module->order;
}

void NEXUS_Module_GetSettings( NEXUS_ModuleHandle module, NEXUS_ModuleSettings *pSettings )
{
    BDBG_OBJECT_ASSERT(module, NEXUS_Module);
    NEXUS_LockModule();
    *pSettings = module->settings;
    NEXUS_UnlockModule();
}

/* See nexus/base/src/$(OS)/nexus_base_os.c for NEXUS_Base_GetDefaultSettings */

#if NEXUS_BASE_EXTERNAL_SCHEDULER
/**
Allow an external scheduler to drive the scheduler state machine. This allows for thread consolidation
in complex systems like linux kernel mode. */
static NEXUS_Error
NEXUS_P_Base_ExternalSchedulerInit(void)
{
    unsigned i;
    for(i=0;i<sizeof(NEXUS_P_Base_State.schedulers)/sizeof(NEXUS_P_Base_State.schedulers[0]);i++) {
        NEXUS_P_Base_State.schedulers[i] = NEXUS_P_Scheduler_Init(i, NEXUS_P_Scheduler_names[i], &NEXUS_P_Base_State.settings.threadSettings[i]);
        if(!NEXUS_P_Base_State.schedulers[i]) {
            return BERR_TRACE(NEXUS_OS_ERROR);
        }
    }
    return NEXUS_SUCCESS;
}

/* Drive the scheduler from an external context. */
NEXUS_Error
NEXUS_P_Base_ExternalScheduler_Step(NEXUS_ModulePriority priority, unsigned timeout, NEXUS_P_Base_Scheduler_Status *status, bool (*complete)(void *context), void *context)
{
    NEXUS_P_Scheduler *scheduler;

    scheduler = NEXUS_P_Base_State.schedulers[priority];
    return NEXUS_P_Scheduler_Step(scheduler, timeout, status, complete, context);
}
#endif

/* All the external context to get the scheduler's mutex. */
void
NEXUS_P_Base_GetSchedulerConfig(NEXUS_ModulePriority priority, NEXUS_Base_Scheduler_Config *config)
{
    BKNI_Memset(config, 0, sizeof(*config));
    if (priority < NEXUS_ModulePriority_eMax) {
        config->name = NEXUS_P_Scheduler_names[priority];
        config->pSettings = &NEXUS_P_Base_State.settings.threadSettings[priority];

        if (NEXUS_P_Base_State.schedulers[priority]) {
            NEXUS_P_SchedulerInfo info;
            NEXUS_P_SchedulerGetInfo(NEXUS_P_Base_State.schedulers[priority], &info);
            config->callback_lock = info.callback_lock;
        }
    }
    return;
}

NEXUS_Error
NEXUS_Base_Core_Init(void)
{
    NEXUS_Error rc;

    BDBG_ASSERT(NEXUS_Base==NULL);
    rc = NEXUS_P_Base_Os_Init();
    if(rc!=BERR_SUCCESS) {
        rc = BERR_TRACE(rc);
        goto err_os;
    }

    BKNI_Memset(&NEXUS_P_Base_State.userThreadInfo, 0, sizeof(NEXUS_P_Base_State.userThreadInfo));
    BLST_AA_TREE_INIT(NEXUS_P_ThreadInfoTree, &NEXUS_P_Base_State.userThreadInfo.tree);
    rc = BKNI_CreateMutex(&NEXUS_P_Base_State.userThreadInfo.lock);
    if(rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc);goto err_mutex; }

#if BDBG_DEBUG_BUILD
    NEXUS_P_Base_SetModuleDebugLevel(NEXUS_GetEnv("wrn_modules"), BDBG_eWrn);
    NEXUS_P_Base_SetModuleDebugLevel(NEXUS_GetEnv("msg_modules"), BDBG_eMsg);
    NEXUS_P_Base_SetModuleDebugLevel(NEXUS_GetEnv("trace_modules"), BDBG_eTrace);
#endif
    NEXUS_P_Base_State.coreInit = true;
    BDBG_CASSERT(BERR_SUCCESS==NEXUS_SUCCESS);
    BDBG_CASSERT(BERR_NOT_INITIALIZED==NEXUS_NOT_INITIALIZED);
    BDBG_CASSERT(BERR_INVALID_PARAMETER==NEXUS_INVALID_PARAMETER);
    BDBG_CASSERT(BERR_OUT_OF_SYSTEM_MEMORY==NEXUS_OUT_OF_SYSTEM_MEMORY);
    BDBG_CASSERT(BERR_OUT_OF_DEVICE_MEMORY==NEXUS_OUT_OF_DEVICE_MEMORY);
    BDBG_CASSERT(BERR_TIMEOUT==NEXUS_TIMEOUT);
    BDBG_CASSERT(BERR_OS_ERROR==NEXUS_OS_ERROR);
    BDBG_CASSERT(BERR_LEAKED_RESOURCE==NEXUS_LEAKED_RESOURCE);
    BDBG_CASSERT(BERR_NOT_SUPPORTED==NEXUS_NOT_SUPPORTED);
    BDBG_CASSERT(BERR_UNKNOWN==NEXUS_UNKNOWN);
    BDBG_CASSERT(BERR_NOT_AVAILABLE==NEXUS_NOT_AVAILABLE);

    return NEXUS_SUCCESS;

err_mutex:
    NEXUS_P_Base_Os_Uninit();
err_os:
    return BERR_TRACE(rc);
}

void
NEXUS_Base_Core_Uninit(void)
{
    unsigned i;
    BDBG_ASSERT(NEXUS_Base==NULL);
    BDBG_MSG_TRACE(("NEXUS_Base_Core_Uninit: NEXUS_P_Base_Os_Uninit"));
    BKNI_DestroyMutex(NEXUS_P_Base_State.userThreadInfo.lock);
    for(i=0;i<NEXUS_BASE_P_MAX_USER_THREADS;i++) {
        if(NEXUS_P_Base_State.userThreadInfo.info[i]) {
            BKNI_Free(NEXUS_P_Base_State.userThreadInfo.info[i]);
        }
    }
    NEXUS_P_Base_Os_Uninit();
    NEXUS_P_Base_State.coreInit = false;
    return;
}

#if NEXUS_P_DEBUG_CALLBACKS
static void NEXUS_P_Base_Monitor(void *context)
{
    BSTD_UNUSED(context);
    while (BKNI_WaitForEvent(NEXUS_P_Base_State.monitor.event, 2000) == BERR_TIMEOUT) {
        unsigned i;
        for(i=0;i<NEXUS_ModulePriority_eMax;i++) {
            if (NEXUS_P_Base_State.schedulers[i]) {
                NEXUS_P_Base_CheckForStuckCallback(NEXUS_P_Base_State.schedulers[i]);
            }
        }
    }
}
#endif

NEXUS_Error
NEXUS_Base_Init(const NEXUS_Base_Settings *pSettings)
{
    BERR_Code rc;
    unsigned i;
    NEXUS_Base_Settings settings;


    if(!pSettings) {
        NEXUS_Base_GetDefaultSettings(&settings);
        pSettings = &settings;
    }

    if(!NEXUS_P_Base_State.coreInit) {
        BDBG_ERR(("NEXUS_Base_Core_Init should be called prior to NEXUS_Base_Init"));
        NEXUS_Base_Core_Init();
    }


    BLST_S_INIT(&NEXUS_P_Base_State.modules);
    NEXUS_P_Base_State.settings = *pSettings;
    NEXUS_P_MapInit();
    NEXUS_P_Base_Stats_Init();

    BDBG_ASSERT(NEXUS_Base==NULL);
    rc = BKNI_CreateMutex(&NEXUS_P_Base_State.callbackHandlerLock);
    if(rc!=BERR_SUCCESS) {
        rc = BERR_TRACE(rc);
        goto err_calback_handler_lock;
    }
    rc = BKNI_CreateMutex(&NEXUS_P_Base_State.baseObject.lock);
    if(rc!=BERR_SUCCESS) {
        rc = BERR_TRACE(rc);
        goto err_base_object_lock;
    }

    for(i=0;i<sizeof(NEXUS_P_Base_State.schedulers)/sizeof(NEXUS_P_Base_State.schedulers[0]);i++) {
        NEXUS_P_Base_State.schedulers[i] = NULL;
    }
    NEXUS_Base = NEXUS_Module_Create(NEXUS_P_Base_Name, NULL);
    if(!NEXUS_Base) {
        rc = BERR_TRACE(BERR_OS_ERROR);
        goto err_module;
    }
    NEXUS_P_Base_Os_MarkThread("main"); 
    NEXUS_LockModule();
#if NEXUS_BASE_EXTERNAL_SCHEDULER
    rc = NEXUS_P_Base_ExternalSchedulerInit(); /* initialize schedulers, but don't start threads */
    if(rc!=BERR_SUCCESS) {
        NEXUS_UnlockModule();
        NEXUS_Module_Destroy(NEXUS_Base);
        rc = BERR_TRACE(rc);
        goto err_module;
    }
#endif
    NEXUS_UnlockModule();
    NEXUS_P_Base_Scheduler_Init();

#if NEXUS_P_DEBUG_CALLBACKS
    rc = BKNI_CreateEvent(&NEXUS_P_Base_State.monitor.event);
    if (!rc) {
        NEXUS_P_Base_State.monitor.thread = NEXUS_Thread_Create("base_monitor", NEXUS_P_Base_Monitor, NULL, NULL);
        if (!NEXUS_P_Base_State.monitor.thread) {
            BKNI_DestroyEvent(NEXUS_P_Base_State.monitor.event);
            NEXUS_P_Base_State.monitor.event = NULL;
            rc = NEXUS_UNKNOWN;
        }
    }
    if (rc) BERR_TRACE(rc); /* keep going */
#endif

    return BERR_SUCCESS;

err_module:
    BKNI_DestroyMutex(NEXUS_P_Base_State.baseObject.lock);
err_base_object_lock:
    BKNI_DestroyMutex(NEXUS_P_Base_State.callbackHandlerLock);
err_calback_handler_lock:
    return rc;
}

#define NEXUS_P_FILENAME(str) ((str)?(str):"")

void NEXUS_Base_Stop(void)
{
    unsigned i;
    BDBG_ASSERT(NEXUS_Base);
    BDBG_MSG_TRACE(("NEXUS_Base_Stop:>"));
    NEXUS_LockModule();
    BDBG_MSG_TRACE(("NEXUS_Base_Stop:Locked"));
    for(i=0;i<sizeof(NEXUS_P_Base_State.schedulers)/sizeof(NEXUS_P_Base_State.schedulers[0]);i++) {
        if(NEXUS_P_Base_State.schedulers[i]) {
            BDBG_MSG_TRACE(("NEXUS_Base_Uninit:NEXUS_P_Scheduler_Stop"));
            NEXUS_P_Scheduler_Stop(NEXUS_P_Base_State.schedulers[i]);
        }
    }
    NEXUS_UnlockModule();
}

void
NEXUS_Base_Uninit(void)
{
    unsigned i;
    BDBG_ASSERT(NEXUS_Base);
    BDBG_MSG_TRACE(("NEXUS_Base_Uninit:>"));

#if NEXUS_P_DEBUG_CALLBACKS
    if (NEXUS_P_Base_State.monitor.thread) {
        BKNI_SetEvent(NEXUS_P_Base_State.monitor.event);
        NEXUS_Thread_Destroy(NEXUS_P_Base_State.monitor.thread);
        BKNI_DestroyEvent(NEXUS_P_Base_State.monitor.event);
    }
#endif

    NEXUS_LockModule();
    BDBG_MSG_TRACE(("NEXUS_Base_Uninit:Locked"));
    for(i=0;i<sizeof(NEXUS_P_Base_State.schedulers)/sizeof(NEXUS_P_Base_State.schedulers[0]);i++) {
        if(NEXUS_P_Base_State.schedulers[i]) {
            BDBG_MSG_TRACE(("NEXUS_Base_Uninit:NEXUS_P_Scheduler_Destroy"));
            NEXUS_P_Scheduler_Destroy(NEXUS_P_Base_State.schedulers[i]);
        }
    }
    BDBG_MSG_TRACE(("NEXUS_Base_Uninit: NEXUS_P_Base_Scheduler_Uninit"));
    NEXUS_P_Base_Scheduler_Uninit();
    BDBG_MSG_TRACE(("NEXUS_Base_Uninit: NEXUS_UnlockModule"));
    NEXUS_UnlockModule();
    BDBG_MSG_TRACE(("NEXUS_Base_Uninit: NEXUS_Module_Destroy"));
    NEXUS_Module_Destroy(NEXUS_Base);
    BDBG_MSG_TRACE(("NEXUS_Base_Uninit: BKNI_DestroyMutex"));
    BKNI_DestroyMutex(NEXUS_P_Base_State.baseObject.lock);
    BKNI_DestroyMutex(NEXUS_P_Base_State.callbackHandlerLock);
    NEXUS_P_Base_Stats_Uninit();
    NEXUS_Base = NULL;
    return;
}

bool
NEXUS_Module_Assert(NEXUS_ModuleHandle module)
{
#if NEXUS_P_DEBUG_MODULE_LOCKS
    NEXUS_P_ThreadInfo *info;
    NEXUS_P_LockEntry *entry;
    
    if (!module->enabled) {
        return true;
    } 
    info = NEXUS_P_ThreadInfo_Get();
    if(info) {
        if (BLIFO_READ_PEEK(&info->stack)>0) {
            /* check if lock is held anywhere in the stack */
            entry = BLIFO_READ(&info->stack);
            while (1) {
                if (entry->module == module) return true;
                if (entry == info->locks) break;
                entry--;
            }
        }
        return false;
    }
#endif
    return true;
    BSTD_UNUSED(module);
}

#if NEXUS_P_DEBUG_MODULE_LOCKS
static void
NEXUS_Module_P_CheckLock(const char *function, NEXUS_P_ThreadInfo *info, NEXUS_ModuleHandle module, const char *pFileName, unsigned lineNumber)
{
    BSTD_UNUSED(pFileName);
    BSTD_UNUSED(lineNumber);
    BSTD_UNUSED(function);
    if(info && BLIFO_READ_PEEK(&info->stack)>0) {
        const NEXUS_P_LockEntry *entry;
        entry = BLIFO_READ(&info->stack);
        if(entry->module == module) {
            BDBG_ERR(("%s[%s]: trying apply recursive lock:%s at %s:%u old at %s:%u", function, info->pThreadName, module->pModuleName, NEXUS_P_FILENAME(pFileName), lineNumber, NEXUS_P_FILENAME(entry->pFileName), entry->lineNumber));
            BDBG_ASSERT(0);
        }
    }
    return;
}
#else
#define NEXUS_Module_P_CheckLock(function, info, module, pFileName, lineNumber)
#endif

#if BKNI_TRACK_MALLOCS
/* use _tagged versions of acquire to tunnel file/line info through */
#else
/* if they don't exist for this OS, reverse-define them */
#define BKNI_AcquireMutex_tagged(MUTEX, FILE, LINE) BKNI_AcquireMutex(MUTEX)
#define BKNI_TryAcquireMutex_tagged(MUTEX, FILE, LINE) BKNI_TryAcquireMutex(MUTEX)
#endif

void
NEXUS_Module_Lock_Tagged(NEXUS_ModuleHandle module, const char *pFileName, unsigned lineNumber)
{
    NEXUS_P_ThreadInfo *info;
    BERR_Code rc;
    NEXUS_P_MODULE_STATS_STATE();

    NEXUS_P_MODULE_STATS_START();
    BSTD_UNUSED(pFileName);
    BSTD_UNUSED(lineNumber);
    
    if (!module) {
        BDBG_ERR(("Locking a NULL module handle. It is possible that the module was not initialized and the application is calling its API."));
        BSTD_UNUSED(module);
    }
    BDBG_OBJECT_ASSERT(module, NEXUS_Module);
#if NEXUS_P_DEBUG_MODULE_LOCKS
    info = NEXUS_P_ThreadInfo_Get();
#endif
    NEXUS_Module_P_CheckLock("NEXUS_Module_Lock_Tagged", info, module, pFileName, lineNumber);
    rc = BKNI_AcquireMutex_tagged(module->lock, pFileName, lineNumber);
    BDBG_ASSERT(rc==BERR_SUCCESS);
#if NEXUS_P_DEBUG_MODULE_LOCKS
    if(info) {
        if(BLIFO_WRITE_PEEK(&info->stack)>0) {
            NEXUS_P_LockEntry *entry = BLIFO_WRITE(&info->stack);
            entry->module = module;
            entry->pFileName = pFileName;
            entry->lineNumber = lineNumber;
            BLIFO_WRITE_COMMIT(&info->stack, 1);
        } else {
            BDBG_WRN(("NEXUS_Module_Lock[%s]: overflow of lock LIFO %s:%u", info->pThreadName, NEXUS_P_FILENAME(pFileName), lineNumber));
        }
    }
#endif
    NEXUS_P_MODULE_STATS_STOP();
    return;
    BSTD_UNUSED(info);
}


bool
NEXUS_Module_TryLock_Tagged(NEXUS_ModuleHandle module, const char *pFileName, unsigned lineNumber )
{
    NEXUS_P_ThreadInfo *info;

    BERR_Code rc;
    BSTD_UNUSED(pFileName);
    BSTD_UNUSED(lineNumber);
    BDBG_OBJECT_ASSERT(module, NEXUS_Module);
#if NEXUS_P_DEBUG_MODULE_LOCKS
    info = NEXUS_P_ThreadInfo_Get();
#endif
    NEXUS_Module_P_CheckLock("NEXUS_Module_TryLock_Tagged", info, module, pFileName, lineNumber);
    rc = BKNI_TryAcquireMutex_tagged(module->lock, pFileName, lineNumber);
#if NEXUS_P_DEBUG_MODULE_LOCKS
    if(rc==BERR_SUCCESS && info) {
        if(BLIFO_WRITE_PEEK(&info->stack)>0) {
            NEXUS_P_LockEntry *entry = BLIFO_WRITE(&info->stack);
            entry->module = module;
            entry->pFileName = pFileName;
            entry->lineNumber = lineNumber;
            BLIFO_WRITE_COMMIT(&info->stack, 1);
        } else {
            BDBG_WRN(("NEXUS_Module_TryLock[%s]: overflow of lock LIFO %s:%u", info->pThreadName, NEXUS_P_FILENAME(pFileName), lineNumber));
        }
    }
#endif
    return rc==BERR_SUCCESS;
    BSTD_UNUSED(info);
}

void
NEXUS_Module_Unlock_Tagged(NEXUS_ModuleHandle module, const char *pFileName, unsigned lineNumber)
{
    NEXUS_P_ThreadInfo *info;

    BSTD_UNUSED(pFileName);
    BSTD_UNUSED(lineNumber);
    BDBG_OBJECT_ASSERT(module, NEXUS_Module);
#if NEXUS_P_DEBUG_MODULE_LOCKS
    info = NEXUS_P_ThreadInfo_Get();
    if(info) {
        if(BLIFO_READ_PEEK(&info->stack)>0) {
            NEXUS_P_LockEntry *entry = BLIFO_READ(&info->stack);
            if(entry->module!=module) {
                BDBG_ERR(("NEXUS_Module_Unlock[%s]: not paired unlock operation %s(%s:%u), last lock:%s(%s:%u) ", info->pThreadName,  module->pModuleName, NEXUS_P_FILENAME(pFileName), lineNumber, entry->module->pModuleName, NEXUS_P_FILENAME(entry->pFileName), entry->lineNumber));
                BDBG_ASSERT(0);
            }
            BLIFO_READ_COMMIT(&info->stack, 1);
        } else {
            BDBG_ERR(("NEXUS_Module_Unlock[%s]: underflow of lock LIFO (%s:%u)", info->pThreadName, NEXUS_P_FILENAME(pFileName), lineNumber));
            BDBG_ASSERT(0);
        }
    }
#endif
    BKNI_ReleaseMutex(module->lock);
    return;
    BSTD_UNUSED(info);
}

void
NEXUS_Module_Enable_Tagged(NEXUS_ModuleHandle module, const char *pFileName, unsigned lineNumber)
{    
    BSTD_UNUSED(pFileName);
    BSTD_UNUSED(lineNumber);
    BDBG_OBJECT_ASSERT(module, NEXUS_Module);
    BDBG_ASSERT(module->enabled == false);

#if NEXUS_POWER_MANAGEMENT
    {
        BKNI_MutexSettings settings;
        BERR_Code rc;

        BKNI_GetMutexSettings(module->lock, &settings);
        settings.suspended = false;
        rc = BKNI_SetMutexSettings(module->lock, &settings);
        BDBG_ASSERT(rc==BERR_SUCCESS);      
    }
#endif
    
    module->enabled = true;
    BKNI_ReleaseMutex(module->lock);
    BDBG_MSG(("Enabling %s Module", module->pModuleName));    

    return;
}

void
NEXUS_Module_Disable_Tagged(NEXUS_ModuleHandle module, const char *pFileName, unsigned lineNumber)
{   
    BERR_Code rc;
    BSTD_UNUSED(pFileName);
    BSTD_UNUSED(lineNumber);
    BDBG_OBJECT_ASSERT(module, NEXUS_Module);        
    BDBG_ASSERT(module->enabled == true);
    
    BDBG_MSG(("Disabling %s Module", module->pModuleName));
    rc = BKNI_AcquireMutex_tagged(module->lock, pFileName, lineNumber);
    BDBG_ASSERT(rc==BERR_SUCCESS);      

    module->enabled = false;
#if NEXUS_POWER_MANAGEMENT
    {
        BKNI_MutexSettings settings;

        BKNI_GetMutexSettings(module->lock, &settings);
        settings.suspended = true;
        rc = BKNI_SetMutexSettings(module->lock, &settings);
        BDBG_ASSERT(rc==BERR_SUCCESS);      
    }
#endif

    return;
}

#if NEXUS_P_DEBUG_LOCK
void
NEXUS_Module_Use(NEXUS_ModuleHandle upModule, NEXUS_ModuleHandle downModule, const char *pFileName, unsigned lineNumber)
{
    BSTD_UNUSED(pFileName);
    BSTD_UNUSED(lineNumber);
    BSTD_UNUSED(upModule);
    BSTD_UNUSED(downModule);
    return;
}
#endif

#if ! (defined(NEXUS_MODE_client) || defined(NEXUS_MODE_proxy)) || defined(NEXUS_WEBCPU_core1_server)
NEXUS_HeapHandle NEXUS_P_DefaultHeap(NEXUS_HeapHandle heap, NEXUS_DefaultHeapType heapType)
{
    if(heap==NULL) {
        const struct b_objdb_client *client = b_objdb_get_client();
        if (client) {
            heap = client->default_heaps.heap[heapType];
        }
    }
    return heap;
}
#endif /* ! (defined(NEXUS_MODE_client) || defined(NEXUS_MODE_proxy)) */

void
NEXUS_P_ThreadInfo_Init(NEXUS_P_ThreadInfo *info)
{
#if NEXUS_P_DEBUG_MODULE_LOCKS
    BLIFO_INIT(&info->stack, info->locks, NEXUS_P_BASE_MAX_LOCKS);
#endif
    info->nexusThread = NULL;
    info->threadId = NULL;
    info->pThreadName = "";
    info->client = NULL;
    return ;
}

void
NEXUS_Module_GetPriority(NEXUS_ModuleHandle module, NEXUS_ModulePriority *pPriority)
{
    BDBG_OBJECT_ASSERT(module, NEXUS_Module);
    BDBG_ASSERT(pPriority);
    *pPriority = module->settings.priority;
    return ;
}


bool NEXUS_Module_ActiveStandyCompatible(NEXUS_ModuleHandle module)
{
    BDBG_OBJECT_ASSERT(module, NEXUS_Module);
    
    if(module->settings.priority == NEXUS_ModulePriority_eIdleActiveStandby ||
       module->settings.priority == NEXUS_ModulePriority_eLowActiveStandby ||
       module->settings.priority == NEXUS_ModulePriority_eHighActiveStandby) {
    return true;
    } else {
    return false;
    }
}


#ifdef NO_OS_DIAGS
void
NEXUS_Base_NO_OS_Scheduler_Dispatch(void)
{
    int idx;
    for (idx=0; idx<NEXUS_ModulePriority_eMax; idx++)
    {
        if (NEXUS_P_Base_State.schedulers[idx])
            NEXUS_P_NO_OS_Scheduler_Thread(NEXUS_P_Base_State.schedulers[idx]);
    }
}
#endif /* NO_OS_DIAGS */

void
NEXUS_Module_EnumerateAll(void (*callback)(void *context, NEXUS_ModuleHandle module, const char *pModuleName, const NEXUS_ModuleSettings *pSettings), void *context)
{
    NEXUS_ModuleHandle module;
    /* Don't call NEXUS_LockModule because of possible deadlock. Do not call this function while adding/removing modules. */
    for(module=BLST_S_FIRST(&NEXUS_P_Base_State.modules); module!=NULL; module=BLST_S_NEXT(module, link)) {
        callback(context,module,module->pModuleName,&module->settings);
    }
    return ;
}

int NEXUS_StrCmp(const char *str1, const char *str2)
{
    if(str1==NULL) {
        if(str2==NULL) {
            return 0;
        } else {
            return 1;
        }
    } else if(str2==NULL) {
        return -1;
    } else {
        return NEXUS_P_Base_StrCmp(str1, str2);
    }
}

BDBG_OBJECT_ID(NEXUS_CallbackHandler);

BDBG_FILE_MODULE(nexus_callbackhandler);
#define BDBG_MSG_CALLBACK(x)    /* BDBG_MODULE_MSG(nexus_callbackhandler, x) */

void NEXUS_Base_P_CallbackHandler_Init(NEXUS_CallbackHandler *handler, NEXUS_ModuleHandle module, void (*pCallback)(void *), void *pContext,const char *pFileName,unsigned lineNumber)
{
    BDBG_ASSERT(handler);
    BDBG_OBJECT_INIT(handler, NEXUS_CallbackHandler);
    BDBG_MSG_CALLBACK(("Init:%#x", (unsigned)handler));
    handler->module = module;
    handler->pCallback = pCallback;
    handler->pContext = pContext;
    handler->pFileName = pFileName;
    handler->lineNumber = lineNumber;
    handler->pCallbackGuard = NULL;
    handler->timer = NULL;
    return;
}

static void NEXUS_Base_P_CallbackHandler_TimerDispatch(void *context)
{
    NEXUS_CallbackHandler *handler=context;
    BDBG_OBJECT_ASSERT(handler, NEXUS_CallbackHandler);
    /* module lock already acquired by the scheduller */
    BKNI_AcquireMutex(NEXUS_P_Base_State.callbackHandlerLock);
    handler->timer=NULL;
    BKNI_ReleaseMutex(NEXUS_P_Base_State.callbackHandlerLock);
    if(handler->pCallbackGuard) {
        NEXUS_Error rc = handler->pCallbackGuard(handler->pContext);
        if(rc!=NEXUS_SUCCESS) {
            BDBG_MSG_CALLBACK(("Timer Deflected:%#x", (unsigned)handler));
            return;
        }
    }
    BDBG_MSG_CALLBACK(("Timer:%#x", (unsigned)handler));
    handler->pCallback(handler->pContext);
    return;
}

static void NEXUS_Base_P_CallbackHandler_TimerCancel(NEXUS_CallbackHandler *handler)
{
    BDBG_MSG_CALLBACK(("TimerCanceled:%#x %#x", (unsigned)handler, (unsigned)handler->timer));
    BKNI_AcquireMutex(NEXUS_P_Base_State.callbackHandlerLock);
    if(handler->timer) {
        NEXUS_Module_CancelTimer(handler->module, handler->timer, handler->pFileName, handler->lineNumber);
        handler->timer = NULL;
    }
    BKNI_ReleaseMutex(NEXUS_P_Base_State.callbackHandlerLock);
    return;
}

/* If you get a BDBG_OBJECT_ASSERT in NEXUS_Base_P_CallbackHandler_Dispatch, set NEXUS_P_DEBUG_CALLBACK_HANDLER and retest */
#define NEXUS_P_DEBUG_CALLBACK_HANDLER 0

void NEXUS_Base_P_CallbackHandler_Dispatch(void *context, int arg)
{
    NEXUS_CallbackHandler *handler=context;
    BSTD_UNUSED(arg);
#if NEXUS_P_DEBUG_CALLBACK_HANDLER
    /* bypass BDBG_OBJECT_ASSERT macro so that caller's filename & linenumber can appear on assert. This will work because handler is not dynamically allocated,
    so we can still deref after NEXUS_Base_P_CallbackHandler_Shutdown. */
    BDBG_Object_Assert(handler, sizeof(*handler), &(handler)->bdbg_object_NEXUS_CallbackHandler, bdbg_id__NEXUS_CallbackHandler, handler->pFileName, handler->lineNumber);
#else
    BDBG_OBJECT_ASSERT(handler, NEXUS_CallbackHandler);
#endif
    if(handler->pCallbackGuard) {
        NEXUS_Error rc = handler->pCallbackGuard(handler->pContext);
        if(rc!=NEXUS_SUCCESS) {
            BDBG_MSG_CALLBACK(("Deflected:%#x", (unsigned)handler));
            return;
        }
    }

    if(NEXUS_Module_TryLock_Tagged(handler->module, handler->pFileName, handler->lineNumber)) {
        /* don't check timer, if it was set it would just cause 1:1 mapping between schedulled callback and callbacks executed */
        BDBG_MSG_CALLBACK(("Locked:%#x", (unsigned)handler));
        handler->pCallback(handler->pContext);
        NEXUS_Module_Unlock_Tagged(handler->module, handler->pFileName, handler->lineNumber);
    } else {
        BKNI_AcquireMutex(NEXUS_P_Base_State.callbackHandlerLock);
        if(handler->timer==NULL) {
            BDBG_MSG_CALLBACK(("Queued:%#x", (unsigned)handler));
            handler->timer = NEXUS_Module_P_ScheduleTimer(handler->module, 0, NEXUS_Base_P_CallbackHandler_TimerDispatch, handler, handler->pFileName, handler->lineNumber);
        } else {
            BDBG_MSG_CALLBACK(("Skipped:%#x %#x", (unsigned)handler, (unsigned)handler->timer));
            /* however if timer is already set, then multiple callbacks get collapsed, this is fine, since callbacks don't have guarantees to get called for each  'Fire' function */
        }
        BKNI_ReleaseMutex(NEXUS_P_Base_State.callbackHandlerLock);
    }
    return;
}

void NEXUS_Base_P_CallbackHandler_Stop(NEXUS_CallbackHandler *handler)
{
    BDBG_OBJECT_ASSERT(handler, NEXUS_CallbackHandler);
    BDBG_MSG_CALLBACK(("Stop:%#x", (unsigned)handler));
    /* cancel any pending timer */
    NEXUS_Base_P_CallbackHandler_TimerCancel(handler);
    return;
}


void NEXUS_Base_P_CallbackHandler_Shutdown(NEXUS_CallbackHandler *handler)
{
    BDBG_OBJECT_ASSERT(handler, NEXUS_CallbackHandler);
    BDBG_MSG_CALLBACK(("Shutdown:%#x", (unsigned)handler));
    NEXUS_Base_P_CallbackHandler_TimerCancel(handler);
#if NEXUS_P_DEBUG_CALLBACK_HANDLER
    BDBG_OBJECT_UNSET(handler, NEXUS_CallbackHandler);
    BKNI_Sleep(rand()%100); /* open up race condition */
#else
    BDBG_OBJECT_DESTROY(handler, NEXUS_CallbackHandler);
#endif
    return;
}


static void NEXUS_Base_P_FreeThreadInfo_locked(NEXUS_P_ThreadInfo *threadInfo)
{
    if (threadInfo->internal) {
        NEXUS_P_Base_State.userThreadInfo.lastUsed[threadInfo->index] = 0;
    }
    BLST_AA_TREE_REMOVE(NEXUS_P_ThreadInfoTree, &NEXUS_P_Base_State.userThreadInfo.tree, threadInfo);
    threadInfo->nexusThread = NULL;
    threadInfo->threadId = NULL;
    return;
}

static void NEXUS_Base_P_TickThreadInfo_locked(NEXUS_P_ThreadInfo *threadInfo)
{
    unsigned tick;   
    tick = NEXUS_P_Base_State.userThreadInfo.currentTick+1; 
    tick = tick == 0 ? 1 : tick; /* skip 0 */
    NEXUS_P_Base_State.userThreadInfo.currentTick = tick;
    NEXUS_P_Base_State.userThreadInfo.lastUsed[threadInfo->index] = tick;
    return;
}

static NEXUS_P_ThreadInfo *NEXUS_Base_P_AllocateThreadInfo_locked(void *threadId)
{
    unsigned i;
    unsigned min = 0;
    bool found = false;
    NEXUS_P_ThreadInfo *threadInfo = NULL;
    NEXUS_P_ThreadInfo *insertedThreadInfo = NULL;

    BDBG_ASSERT(threadId!=NULL);

    for(i=min;i<NEXUS_BASE_P_MAX_USER_THREADS;i++) {
        if(!NEXUS_P_Base_State.userThreadInfo.info[i] || NEXUS_P_Base_State.userThreadInfo.lastUsed[i]==0) { /* BINGO found unused entry */
            min = i;
            found = true;
            break;
        } else {
#if NEXUS_P_DEBUG_MODULE_LOCKS
            if(BLIFO_READ_PEEK(&NEXUS_P_Base_State.userThreadInfo.info[i]->stack)==0) {
                if(!found) {
                    if(BLIFO_READ_PEEK(&NEXUS_P_Base_State.userThreadInfo.info[min]->stack)!=0) {
                        min = i;
                    }
                }
                if(NEXUS_P_Base_State.userThreadInfo.lastUsed[i] < NEXUS_P_Base_State.userThreadInfo.lastUsed[min]) { /* search minimal entry */
                    min = i;
                }
                found = true;
            }
#else
            if(!found || NEXUS_P_Base_State.userThreadInfo.lastUsed[i] < NEXUS_P_Base_State.userThreadInfo.lastUsed[min]) { /* search minimal entry */
                min = i;
            }
            found = true;
#endif
        }
    }
    if(!found) {
        BDBG_ERR(("Can't allocate ThreadInfo"));
        return NULL;
    }
    if (!NEXUS_P_Base_State.userThreadInfo.info[min]) {
        threadInfo = BKNI_Malloc(sizeof(*threadInfo));
        if (!threadInfo) {
            return NULL;
        }
        BKNI_Memset(threadInfo, 0, sizeof(*threadInfo));
        threadInfo->index = min;
        threadInfo->internal = true;
        NEXUS_P_Base_State.userThreadInfo.info[min] = threadInfo;
    }
    else {
        threadInfo = NEXUS_P_Base_State.userThreadInfo.info[min];
        BDBG_ASSERT(threadInfo->index == min);
        BDBG_ASSERT(threadInfo->internal);
    }
    if(NEXUS_P_Base_State.userThreadInfo.lastUsed[min]) { /* if found used entry, it should be freed first */
        NEXUS_Base_P_FreeThreadInfo_locked(threadInfo);
    }
    NEXUS_P_ThreadInfo_Init(threadInfo);
    threadInfo->threadId = threadId;
    insertedThreadInfo = BLST_AA_TREE_INSERT(NEXUS_P_ThreadInfoTree, &NEXUS_P_Base_State.userThreadInfo.tree, threadId, threadInfo);
    BDBG_ASSERT(insertedThreadInfo == threadInfo); /* this could fail if there was already element with thes ame threadId */
    BSTD_UNUSED(insertedThreadInfo);
    NEXUS_Base_P_TickThreadInfo_locked(threadInfo);
    return threadInfo;
}

NEXUS_P_ThreadInfo *NEXUS_Base_P_AllocateThreadInfo(void *threadId)
{
    NEXUS_P_ThreadInfo *threadInfo;
    BKNI_AcquireMutex(NEXUS_P_Base_State.userThreadInfo.lock);
    threadInfo = BLST_AA_TREE_FIND(NEXUS_P_ThreadInfoTree, &NEXUS_P_Base_State.userThreadInfo.tree, threadId); /* try to find it in already allocated pool */
    if(threadInfo) { 
        NEXUS_P_ThreadInfo_Init(threadInfo);
        threadInfo->threadId = threadId;
    } else {
        threadInfo = NEXUS_Base_P_AllocateThreadInfo_locked(threadId); /* new threadInfo could be allocated only for unique threadId */
    }
    BKNI_ReleaseMutex(NEXUS_P_Base_State.userThreadInfo.lock);
    return threadInfo;
}


void NEXUS_Base_P_Thread_AssociateInfo(NEXUS_ThreadHandle thread, void *threadId, NEXUS_P_ThreadInfo *threadInfo)
{
    NEXUS_P_ThreadInfo *insertedThreadInfo;
    BKNI_AcquireMutex(NEXUS_P_Base_State.userThreadInfo.lock);
    threadInfo->internal = false;
    threadInfo->nexusThread = thread;
    threadInfo->threadId = threadId;
    insertedThreadInfo = BLST_AA_TREE_INSERT(NEXUS_P_ThreadInfoTree, &NEXUS_P_Base_State.userThreadInfo.tree, threadId, threadInfo);
    if(insertedThreadInfo != threadInfo) { /* insertedThreadInfo points to thread with the same key _that_ is already in the tree */
        if(NEXUS_P_Base_State.userThreadInfo.cache.threadInfo == insertedThreadInfo) {
            NEXUS_P_Base_State.userThreadInfo.cache.threadId = NULL; /* clear stale cache entry */
            NEXUS_P_Base_State.userThreadInfo.cache.threadInfo = NULL;
        }
        NEXUS_Base_P_FreeThreadInfo_locked(insertedThreadInfo);
        insertedThreadInfo = BLST_AA_TREE_INSERT(NEXUS_P_ThreadInfoTree, &NEXUS_P_Base_State.userThreadInfo.tree, threadId, threadInfo);
        BDBG_ASSERT(insertedThreadInfo == threadInfo); /* now it should be really inserted */
        BSTD_UNUSED(insertedThreadInfo);
    }
    BKNI_ReleaseMutex(NEXUS_P_Base_State.userThreadInfo.lock);
    return;
}

void NEXUS_Base_P_Thread_DisassociateInfo(NEXUS_ThreadHandle thread, NEXUS_P_ThreadInfo *threadInfo)
{
    BSTD_UNUSED(thread);
    BKNI_AcquireMutex(NEXUS_P_Base_State.userThreadInfo.lock);
    BDBG_ASSERT(threadInfo->nexusThread == thread);
    if(NEXUS_P_Base_State.userThreadInfo.cache.threadId == threadInfo->threadId) {
        NEXUS_P_Base_State.userThreadInfo.cache.threadId = NULL; /* clear stale cache entry */
        NEXUS_P_Base_State.userThreadInfo.cache.threadInfo = NULL;
    }
    NEXUS_Base_P_FreeThreadInfo_locked(threadInfo);

    BKNI_ReleaseMutex(NEXUS_P_Base_State.userThreadInfo.lock);
    return;
}

NEXUS_P_ThreadInfo *NEXUS_Base_P_Thread_GetInfo(void *threadId)
{
    NEXUS_P_ThreadInfo *threadInfo;

    if(!NEXUS_P_Base_State.coreInit) {
        return NULL; /* there are case when NEXUS_Base_P_Thread_GetInfo called before/after nexus was initialized/uninitialized */
    }
    BKNI_AcquireMutex(NEXUS_P_Base_State.userThreadInfo.lock);
    if(NEXUS_P_Base_State.userThreadInfo.cache.threadId == threadId) { /* test cache first */
        threadInfo = NEXUS_P_Base_State.userThreadInfo.cache.threadInfo;
    } else {
        threadInfo = BLST_AA_TREE_FIND(NEXUS_P_ThreadInfoTree, &NEXUS_P_Base_State.userThreadInfo.tree, threadId);
        if(threadInfo == NULL) { /* allocate a new */
            threadInfo = NEXUS_Base_P_AllocateThreadInfo_locked(threadId);
        }
        NEXUS_P_Base_State.userThreadInfo.cache.threadId = threadId; /* update cache */
        NEXUS_P_Base_State.userThreadInfo.cache.threadInfo = threadInfo;
        if(threadInfo->internal) {
            NEXUS_Base_P_TickThreadInfo_locked(threadInfo);
        }
    }
    BKNI_ReleaseMutex(NEXUS_P_Base_State.userThreadInfo.lock);
    return threadInfo;
}

void NEXUS_Base_P_TickThreadInfo(NEXUS_P_ThreadInfo *threadInfo)
{
    if(threadInfo->internal) {
        /* delay serialization  point */
        BKNI_AcquireMutex(NEXUS_P_Base_State.userThreadInfo.lock);
        NEXUS_Base_P_TickThreadInfo_locked(threadInfo);
        BKNI_ReleaseMutex(NEXUS_P_Base_State.userThreadInfo.lock);
    }
    return;
}

/* we can wrap NEXUS_Thread_Create, but cannot wrap NEXUS_Thread_Destroy because Destroy must synchronize with the thread
and a wrapper causes deadlock when the thread calls into nexus/base. */
NEXUS_ThreadHandle
NEXUS_Thread_Create(const char *pThreadName, void (*pThreadFunc)(void *), void *pContext, const NEXUS_ThreadSettings *pSettings)
{
    NEXUS_ThreadHandle thread;
    NEXUS_LockModule();
    thread = NEXUS_P_Thread_Create(pThreadName, pThreadFunc, pContext, pSettings);
    NEXUS_UnlockModule();
    return thread;
}

size_t NEXUS_P_SizeAlign(size_t v, size_t alignment)
{
    size_t r;
    r = v + (alignment - 1);
    r -= r%alignment;
    return r;
}
