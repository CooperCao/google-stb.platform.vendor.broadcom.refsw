/***************************************************************************
*     (c)2008-2013 Broadcom Corporation
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
#ifndef NEXUS_BASE_PRIV_H__
#define NEXUS_BASE_PRIV_H__

#include "nexus_base_os.h"
#include "nexus_base_os_priv.h"
#include "blifo.h"
#include "bkni_event_group.h"

extern NEXUS_ModuleHandle NEXUS_Base;
#define NEXUS_MODULE_SELF NEXUS_Base

#ifdef __cplusplus
extern "C"
{
#endif


BERR_Code NEXUS_P_Base_Os_Init(void);
void NEXUS_P_Base_Os_Uninit(void);
void NEXUS_P_Base_Scheduler_Init(void);
void NEXUS_P_Base_Scheduler_Uninit(void);


typedef struct NEXUS_P_Scheduler NEXUS_P_Scheduler;

BDBG_OBJECT_ID_DECLARE(NEXUS_Module);

struct b_objdb_module;
struct NEXUS_Module {
    BDBG_OBJECT(NEXUS_Module)
    BKNI_MutexHandle lock;
    NEXUS_P_Scheduler *scheduler;
    NEXUS_ModuleSettings settings;
    const char *pModuleName;
    unsigned order;
    BLST_S_ENTRY(NEXUS_Module) link;
    struct b_objdb_module *objdb; /* pointer to the object data base used by this module */
    bool enabled;
};

NEXUS_P_Scheduler *NEXUS_P_Scheduler_Create(NEXUS_ModulePriority priority, const char *name, const NEXUS_ThreadSettings *pSettings);
NEXUS_P_Scheduler *NEXUS_P_Scheduler_Init(NEXUS_ModulePriority priority, const char *name, const NEXUS_ThreadSettings *pSettings); /* initialiase scheduller for use with external thread */

void NEXUS_P_Scheduler_Stop(NEXUS_P_Scheduler *scheduler);
void NEXUS_P_Scheduler_Destroy(NEXUS_P_Scheduler *scheduler);
void NEXUS_P_MapInit(void);

typedef struct NEXUS_P_SchedulerInfo {
    BKNI_MutexHandle callback_lock; /* callback that is acquired when callback active */
} NEXUS_P_SchedulerInfo;
void NEXUS_P_SchedulerGetInfo(NEXUS_P_Scheduler *scheduler, NEXUS_P_SchedulerInfo *info);

NEXUS_Error NEXUS_P_Scheduler_Step(NEXUS_P_Scheduler *scheduler, unsigned timeout, NEXUS_P_Base_Scheduler_Status *status, bool (*complete)(void *context), void *context);
const char *NEXUS_P_PrepareFileName(const char *pFileName);
NEXUS_TimerHandle NEXUS_Module_P_ScheduleTimer(NEXUS_ModuleHandle module, unsigned delayMs, void (*pCallback)(void *),  void *pContext, const char *pFileName, unsigned lineNumber);

/**
Summary:
Adds delta in milliseconds to the timestamp.

Description:
    Delta could be only positive. Trying to add negative delta would result in no-op. And there is no provision for stable addition.
    For example:
    NEXUS_Time_Add(1); NEXUS_Time_Add(1); NEXUS_Time_Add(1); NEXUS_Time_Add(1);
    And
    NEXUS_Time_Add(4);
    Could produce different results.
**/
void NEXUS_Time_Add(
    NEXUS_Time *time,
    long delta_ms /* milliseconds */
    );

#define NEXUS_BASE_P_MAX_USER_THREADS    256

BLST_AA_TREE_HEAD(NEXUS_P_ThreadInfoTree, NEXUS_P_ThreadInfo);

struct NEXUS_P_Base_State {
    NEXUS_P_Scheduler *schedulers[NEXUS_ModulePriority_eMax];
    BLST_S_HEAD(NEXUS_P_ModuleList, NEXUS_Module) modules;
    BKNI_MutexHandle callbackHandlerLock; /* lock that serializes access to the field that holds timer handle */
    NEXUS_Base_Settings settings;
    /* for user threads (ones that aren't created with the NEXUS_Thread_Create, we maintain fixed number of 'threads' specific storages, they aren't dynamically allocated since there is no 'good' way of knowing that thread was killed and thread specific storage could be recycled */
    bool coreInit;
    struct {
        BKNI_MutexHandle lock;
        struct NEXUS_P_ThreadInfoTree tree;
        unsigned currentTick; /* this is incremented on each call to get threadInfo, and it's used to keep contant of time */
        NEXUS_P_ThreadInfo *info[NEXUS_BASE_P_MAX_USER_THREADS];
        unsigned lastUsed[NEXUS_BASE_P_MAX_USER_THREADS]; /* on each call it's get copied from the  currentTick, and it's used at the recycle time to get node that should be recycled (e.g. node that was not accessed for long */
        struct {
            void *threadId;
            NEXUS_P_ThreadInfo *threadInfo;
        } cache;
    } userThreadInfo;
    struct {
        BKNI_MutexHandle lock; /* lock used to access reference count as well as object database */
    } baseObject;
    struct {
        NEXUS_ThreadHandle thread;
        BKNI_EventHandle event;
    } monitor;
};
extern struct NEXUS_P_Base_State NEXUS_P_Base_State;

void NEXUS_P_Base_CheckForStuckCallback(const NEXUS_P_Scheduler *scheduler);

#ifdef __cplusplus
}
#endif

#endif /* NEXUS_BASE_PRIV_H__ */
