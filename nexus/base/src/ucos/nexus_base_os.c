/***************************************************************************
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
*
***************************************************************************/
#if UCOS_VERSION==1
#include <ucos.h>
#include <ucos_cfg.h>
#define  OS_CFG_TICK_RATE_HZ             100u               /* Tick rate in Hertz (10 to 1000 Hz)                     */
#elif UCOS_VERSION==3
#include <os.h>
#include <os_cfg_app.h>
#endif
#include "nexus_base.h"
#include "nexus_base_priv.h"
#ifdef NEXUS_CPU_ARM
    #define ARM_V7
    #define CFG_ARCH_ARM
    #include "arm.h"
    #include "cache_ops.h"
#else
    #include "bsp_config.h"
#endif
#include <string.h>
#include <stdlib.h>

BDBG_MODULE(nexus_base_os);

/****************************************************************************
    Defines
****************************************************************************/
/* We're porting the 256-task version of uCOS. If we move to a new version,
   it will likely require a different port, or a bit of refactoring at least.
*/
#define UCOS_MAX_TASKS          (256)
#define UCOS_TASK_STACKSIZE_C   (2048 * sizeof(unsigned int))
#define UCOS_OS_TICK_RATE_MS    (1000/OS_CFG_TICK_RATE_HZ) /* How many ms in an OS tick? <<< JPF -the value is really ms per tick */
#define MAX_ENV_ENTRIES         (64)

/****************************************************************************
   uC/OS-ii has 256 max priorities.  Of this, priority 101 through 200 is
   reserved for nexus.
****************************************************************************/
#define MAX_NEXUS_PRIORITIES    (100)
#define NEXUS_PRIORITY_OFFSET   (101)

typedef unsigned int b_task_t;

typedef struct NEXUS_OsEnv {
    unsigned count;
    struct {
        const char *key;
        const char *value;
    } env[MAX_ENV_ENTRIES];
} NEXUS_OsEnv;

static NEXUS_OsEnv NEXUS_P_OsEnv = {
    0,
    {{ NULL, NULL }}
};

#if UCOS_VERSION==1
static bool allocated_priorities[MAX_NEXUS_PRIORITIES]={0,0,0,0,0,0,0,0,0,0, \
                                                        0,0,0,0,0,0,0,0,0,0, \
                                                        0,0,0,0,0,0,0,0,0,0, \
                                                        0,0,0,0,0,0,0,0,0,0, \
                                                        0,0,0,0,0,0,0,0,0,0, \
                                                        0,0,0,0,0,0,0,0,0,0, \
                                                        0,0,0,0,0,0,0,0,0,0, \
                                                        0,0,0,0,0,0,0,0,0,0, \
                                                        0,0,0,0,0,0,0,0,0,0, \
                                                        0,0,0,0,0,0,0,0,0,0};
#endif

void
NEXUS_Time_Get(NEXUS_Time *time)
{
    long osTicks;
#if UCOS_VERSION==3
    OS_ERR err;
#endif

    BDBG_ASSERT(time);
    
#if UCOS_VERSION==1
    osTicks = OSTimeGet();
#elif UCOS_VERSION==3
    osTicks = OSTimeGet(&err);
    if (err != OS_ERR_NONE) {
        BDBG_ERR(("OSTimeGet did not return OS_ERR_NONE\n"));
        BKNI_Fail();
    }
#else
    #error unknown UCOS_VERSION
#endif
    time->osTicks = osTicks;

    return;
}

long 
NEXUS_Time_Diff(const NEXUS_Time *future, const NEXUS_Time *past)
{
    return ((future->osTicks - past->osTicks) * UCOS_OS_TICK_RATE_MS);
}

void 
NEXUS_Time_Add(NEXUS_Time *time, long delta_ms)
{
    BDBG_ASSERT(time);
    time->osTicks += (delta_ms / UCOS_OS_TICK_RATE_MS);

    return;
}

void 
NEXUS_Thread_GetDefaultSettings(NEXUS_ThreadSettings *pSettings)
{
    BDBG_ASSERT(pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
#if UCOS_VERSION==1
    NEXUS_P_Get_Priority(&pSettings->priority);
#elif UCOS_VERSION==3
    pSettings->priority = 10;
#else
    #error unknown UCOS_VERSION
#endif
    pSettings->stackSize = UCOS_TASK_STACKSIZE_C;
    return;
}

BDBG_OBJECT_ID(NEXUS_Thread);

struct NEXUS_Thread {
    NEXUS_P_ThreadInfo info;
    char name[64];
    BDBG_OBJECT(NEXUS_Thread)
    int task; /* task id of the new thread */
    void (*pThreadFunc)(void *);
    void *pContext;
    b_task_t handle;
    NEXUS_ThreadSettings settings;
    unsigned * stack;
#if UCOS_VERSION==3
    OS_TCB *p_tcb;
#endif
    BLST_S_ENTRY(NEXUS_Thread) link;
} NEXUS_Thread;

static struct {
    BLST_S_HEAD(NEXUS_ThreadHead, NEXUS_Thread) threads;
} NEXUS_P_Os_State;

static int CurrentTaskID = 0;
static unsigned int NumberOftasks = 0;

static int 
NEXUS_P_ThreadEntry(void *data)
{
    unsigned taskId;
    NEXUS_ThreadHandle thread = (NEXUS_ThreadHandle)data;
#if UCOS_VERSION==3
    OS_ERR err;
#endif


    /* Get the current task ID (priority) and use it to find our task object */
    taskId = thread->settings.priority;
    if (taskId >= UCOS_MAX_TASKS) {
        BDBG_ERR(("Invalid task [%d]!!\n", taskId));
        BKNI_Fail();
        return -1;
    }

    if (!thread) {
        BDBG_ERR(("Invalid task called [%d]!!\n", taskId));
        BKNI_Fail();
        return -1;
    }

    BDBG_MSG(("NEXUS_P_ThreadEntry: task handle %p", thread));

#ifndef NEXUS_CPU_ARM
    if (((unsigned long)thread->pThreadFunc & 0xe0000000) != 0x80000000) {
        BDBG_ERR(("trying to execute task at 0x%.8lx\n", (unsigned long)thread->pThreadFunc));
        BKNI_Fail();
    }
#endif

    (*thread->pThreadFunc)(thread->pContext);

    #if OS_CFG_TASK_DEL_EN > 0
        OSTaskDel((OS_TCB *)0, &err);
    #endif

    return 0;
}

#if UCOS_VERSION==1
static int
NEXUS_P_Get_Priority(unsigned int *priority)
{
    int i;
    for (i=0; i<MAX_NEXUS_PRIORITIES; i++) {
        if (!allocated_priorities[i]) {
            allocated_priorities[i]=true;
            *priority=i+NEXUS_PRIORITY_OFFSET;
            return 0;
        }
    }
    printf("max priorities reached\n");
    return -1;
}

static int
NEXUS_P_Release_Priority(unsigned int priority)
{
    int i;
    i = priority-NEXUS_PRIORITY_OFFSET;
    if ((0 <= i) && (i < MAX_NEXUS_PRIORITIES)) {
        if (allocated_priorities[i])
            allocated_priorities[i]=false;
        else {
            printf("priority not allocated\n");
            return -1;
        }
    }
    return 0;
}
#endif

NEXUS_ThreadHandle 
NEXUS_P_Thread_Create(const char *pThreadName, void (*pThreadFunc)(void *), void *pContext, const NEXUS_ThreadSettings *pSettings)
{
    NEXUS_ThreadHandle thread;
    BERR_Code mrc;
    int stackSize;
    unsigned int priority;
    unsigned *p_task_stack;
    NEXUS_ThreadSettings settings;
#if UCOS_VERSION==1
    OS_STATUS ucosStatus;
#elif UCOS_VERSION==3
    OS_ERR err;
#endif

    if (!pThreadName || !pThreadFunc) {
        BERR_TRACE(NEXUS_INVALID_PARAMETER);
        return NULL;
    }

    if (!pSettings) {
        NEXUS_Thread_GetDefaultSettings(&settings);
    }
    else {
        settings = *pSettings;
    }
    priority=settings.priority;
    if (priority == 0) {
        BDBG_WRN(("Changing priority from 0 to 1\n"));
        priority = 1;  /* Can't use priority equal to zero in uC/OS */
#if UCOS_VERSION==1
    } else if (priority >= OS_MAX_TASKS - 1) {
        BDBG_WRN(("Changing priority from priority %d to %d\n", priority, OS_MAX_TASKS - 2));
        priority = OS_MAX_TASKS - 2;  /* Can't use the lowest priority allowed, which is reserved for the uC/OS Idle Task (OS_CFG_PRIO_MAX - 1) */
    }
#elif UCOS_VERSION==3
    } else if (priority >= OS_CFG_PRIO_MAX - 1) {
        BDBG_WRN(("Changing priority from priority %d to %d\n", priority, OS_CFG_PRIO_MAX - 2));
        priority = OS_CFG_PRIO_MAX - 2;  /* Can't use the lowest priority allowed, which is reserved for the uC/OS Idle Task (OS_CFG_PRIO_MAX - 1) */
    }
#else
    #error unknown UCOS_VERSION
#endif
    stackSize=settings.stackSize;

    BDBG_MSG(("Thread Pri: %d Stack: %d", priority, stackSize));

    if (NumberOftasks >= UCOS_MAX_TASKS) {
        BDBG_ERR(("BKNI_AddTask: Number of tasks  0x%.8x exceed UCOS_MAX_TASKS\n", NumberOftasks+1));
        return NULL;
    }

#if UCOS_VERSION==1
    if (priority >= OS_MAX_TASKS) {
        BDBG_ERR(("BKNI_AddTask: Thread priority 0x%.8x exceed OS_MAX_TASKS\n", priority));
        return NULL;
    }
#elif UCOS_VERSION==3
    if (priority >= OS_CFG_PRIO_MAX) {
        BDBG_ERR(("BKNI_AddTask: Thread priority 0x%.8x exceed OS_CFG_PRIO_MAX\n", priority));
        return NULL;
    }
#else
    #error unknown UCOS_VERSION
#endif
    /* We allocate the task stack for uCOS. After we've successfully allocated
       the memory, keep a pointer to the "stack"
    */
    thread = BKNI_Malloc(sizeof(*thread) + stackSize);
    if (!thread) {
        mrc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto err_alloc;
    }
    BKNI_Memset(thread, 0, sizeof(*thread) + stackSize);

    p_task_stack = (unsigned *)((char *)thread + sizeof(*thread));
    
    BDBG_MSG(("NEXUS_Thread_Create: task=%p, stack=%p", thread, p_task_stack));

    BDBG_OBJECT_INIT(thread, NEXUS_Thread);

    /* Allocate for the TCB */
#if UCOS_VERSION==3
    thread->p_tcb = BKNI_Malloc(sizeof(OS_TCB));
    if (!thread->p_tcb) {
        mrc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto err_alloc2;
    }
    BKNI_Memset(thread->p_tcb, 0, sizeof(OS_TCB));
#endif

    thread->stack = p_task_stack;
    strncpy(thread->name, pThreadName,sizeof(thread->name)-1);
    thread->name[sizeof(thread->name)-1]='\0';
    thread->pThreadFunc = pThreadFunc;
    thread->pContext = pContext;
    thread->settings.priority = priority;
    thread->settings.stackSize = stackSize;
    NEXUS_P_ThreadInfo_Init(&(thread->info));
    thread->info.pThreadName = thread->name;
    thread->task = CurrentTaskID;
    thread->handle = priority;

#if UCOS_VERSION==1
    ucosStatus = OSTaskCreate(NEXUS_P_ThreadEntry,
                              (void *)thread,
                              (void *)((UBYTE *)p_task_stack + stackSize),
                              priority);
#elif UCOS_VERSION==3
    OSTaskCreate((OS_TCB     *)thread->p_tcb, /**/
                 (CPU_CHAR   *)pThreadName, /**/
                 (OS_TASK_PTR )NEXUS_P_ThreadEntry, /**/
                 (void       *)thread, /**/
                 (OS_PRIO     )priority, /**/
                 (CPU_STK    *)p_task_stack,
                 (CPU_STK_SIZE)stackSize/10, /**/
                 (CPU_STK_SIZE)stackSize/4, /**/
                 (OS_MSG_QTY  )0,
                 (OS_TICK     )0,
                 (void       *)0,
                 (OS_OPT      )OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR,
                 (OS_ERR     *)&err);
    if (err != OS_ERR_NONE) {
        mrc = BERR_TRACE(BERR_OS_ERROR);
        goto err_thread;
    }
#else
    #error unknown UCOS_VERSION
#endif

    ++CurrentTaskID;
    ++NumberOftasks;
    BLST_S_INSERT_HEAD(&NEXUS_P_Os_State.threads, thread, link);
    return thread;
err_thread:
#if UCOS_VERSION==3
    BKNI_Free(thread->p_tcb);
#endif

err_alloc2:
    BKNI_Free(thread);
err_alloc:
    return NULL;
}

void 
NEXUS_Thread_Destroy(NEXUS_ThreadHandle thread)
{
#if UCOS_VERSION==3
    OS_ERR err;
#endif
    BDBG_MSG(("NEXUS_Thread_Destroy"));

    BDBG_OBJECT_ASSERT(thread, NEXUS_Thread);

    if (thread->task) {
        if (thread->task) {
            BDBG_WRN(("NEXUS_Thread_Destroy: %#lx killing (%s:%u)", (unsigned long)thread, thread->name, thread->task));

            #if OS_CFG_TASK_DEL_EN > 0
                OSTaskDel(thread->p_tcb, &err);
            #endif
        }
    }

    BLST_S_REMOVE(&NEXUS_P_Os_State.threads, thread, NEXUS_Thread, link);
    NumberOftasks--;

    BDBG_OBJECT_DESTROY(thread, NEXUS_Thread);
    BKNI_Free(thread);

    return;
}

/**
NEXUS_P_Base_Os_MarkThread allows nexus to store bookkeeping information
about external threads. This function does not need to be called for threads
nexus creates using NEXUS_Thread_Create.
threads.
**/
void
NEXUS_P_Base_Os_MarkThread(const char *name)
{
    BSTD_UNUSED(name);
    return;
}

BERR_Code
NEXUS_P_Base_Os_Init(void)
{
    BLST_S_INIT(&NEXUS_P_Os_State.threads);
    return BERR_SUCCESS;
}

void
NEXUS_P_Base_Os_Uninit(void)
{
    NEXUS_ThreadHandle thread;

    while(NULL!=(thread=BLST_S_FIRST(&NEXUS_P_Os_State.threads))) {
        BLST_S_REMOVE_HEAD(&NEXUS_P_Os_State.threads, link); /* don't free or destroy thread */
        if(thread->pThreadFunc==NULL) { /* dummy placeholder */
            BDBG_OBJECT_DESTROY(thread, NEXUS_Thread);
            BKNI_Free(thread); /* could delete it */
        }
        else
        {
            BDBG_WRN(("NEXUS_P_Base_Os_Uninit: %#lx leaked thread '%s'", (unsigned long)thread, thread->name));
        }
    }
}

const char *
NEXUS_P_Base_Os_GetTaskNameFromStack(const unsigned long *stack)
{
    BSTD_UNUSED(stack);
    return NULL;
}


NEXUS_P_ThreadInfo *
NEXUS_P_ThreadInfo_Get(void)
{
    return NULL;
}

const char *
NEXUS_GetEnv(const char *name)
{
    unsigned i;
    
    for (i=0;i<NEXUS_P_OsEnv.count;i++) {
        if (! NEXUS_P_Base_StrCmp(NEXUS_P_OsEnv.env[i].key, name)) {
            BDBG_MSG(("GetEnv: %s=%s", NEXUS_P_OsEnv.env[i].key, NEXUS_P_OsEnv.env[i].value));
            return NEXUS_P_OsEnv.env[i].value;
        }
    }
    BDBG_MSG(("GetEnv: %s", name));
    return NULL;
}

void
NEXUS_SetEnv(const char *name, const char *value)
{
    unsigned i;

    BDBG_MSG(("SetEnv: %s=%s", name, value));

    /* if already there, replace old with new */
    for (i=0;i<NEXUS_P_OsEnv.count;i++) {
        if (! NEXUS_P_Base_StrCmp(NEXUS_P_OsEnv.env[i].key, name)) {
            NEXUS_P_OsEnv.env[i].value = value;
            return;
        }
    }
    /* not already there -- add it (save new key.value pair) */
    if (i<sizeof(NEXUS_P_OsEnv.env)/sizeof(*NEXUS_P_OsEnv.env)) {
        NEXUS_P_OsEnv.env[i].key = name;
        NEXUS_P_OsEnv.env[i].value = value;
        NEXUS_P_OsEnv.count = i+1;
    }
    return;
}

void
NEXUS_FlushCache(const void *pvAddr, size_t ulNumBytes)
{
    #ifdef CACHE_WORKAROUND
        flush_dcache((unsigned long)pvAddr, (unsigned long)((unsigned char*)pvAddr + ulNumBytes));
    #else
        #ifdef NEXUS_CPU_ARM
            CACHE_FLUSH_RANGE((void *)pvAddr, (unsigned int)ulNumBytes);
        #else
            clear_d_cache((unsigned char *)pvAddr, (unsigned int)ulNumBytes);
        #endif
    #endif
}

void
NEXUS_Base_GetDefaultSettings(NEXUS_Base_Settings *pSettings)
{
    unsigned i;
    BDBG_ASSERT(pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    for(i=0;i<sizeof(pSettings->threadSettings)/sizeof(pSettings->threadSettings[0]);i++) {
        NEXUS_Thread_GetDefaultSettings(&pSettings->threadSettings[i]);
    }
    return;
}

int
NEXUS_atoi(const char *str)
{
    return atoi(str);
}

int
NEXUS_P_Base_StrCmp(const char *str1, const char *str2)
{
    return strcmp(str1, str2);
}

int b_strlen(const char *s)
{
    return strlen(s);
}

char *b_strncpy(char *dest, const char *src, int n)
{
    return strncpy(dest,src,n);
}
