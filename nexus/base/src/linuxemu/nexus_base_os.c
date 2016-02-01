/***************************************************************************
*     (c)2008-2012 Broadcom Corporation
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
#include "nexus_base.h"
#include "nexus_base_priv.h"
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/time.h>

BDBG_MODULE(nexus_base_os);

void
NEXUS_Time_Get(NEXUS_Time *time)
{
    int rc;
    struct timeval now;
    rc = gettimeofday(&now, NULL);
    if (rc) {
        /* don't pass back uninit memory w/ no possible return code */
        time->tv_sec = 0;
        time->tv_nsec = 0;
        rc = BERR_TRACE(BERR_OS_ERROR);
        return;
    }
    time->tv_sec = now.tv_sec;
    time->tv_nsec = now.tv_usec/1000;
    return;
}

long
NEXUS_Time_Diff(const NEXUS_Time *future, const NEXUS_Time *past)
{
    return 1000*(future->tv_sec - past->tv_sec) + (future->tv_nsec - past->tv_nsec)/1000000;
}

void
NEXUS_Time_Add(NEXUS_Time *time, long delta_ms)
{
    time->tv_nsec += (delta_ms%1000) * 1000000;
    time->tv_sec += delta_ms/1000;
    if (time->tv_nsec > 1000000000) {
        time->tv_nsec -= 1000000000;
        time->tv_sec++;
    }
    return;
}

static const NEXUS_ThreadSettings NEXUS_DefaultThreadSettings = {
    1, /* priority */
    64*1024 /* stackSize */
};

void
NEXUS_Thread_GetDefaultSettings(NEXUS_ThreadSettings *pSettings)
{
    BDBG_ASSERT(pSettings);
    *pSettings = NEXUS_DefaultThreadSettings;
    return;
}

BDBG_OBJECT_ID(NEXUS_Thread);

struct NEXUS_Thread {
    char name[16];
    BDBG_OBJECT(NEXUS_Thread)
    pthread_t thread;
    void (*pThreadFunc)(void *);
    void *pContext;
    NEXUS_ThreadSettings settings;
    NEXUS_P_ThreadInfo info;
    const uint8_t *stack_top;
    size_t stack_len;
    BLST_S_ENTRY(NEXUS_Thread) link;
};

static struct {
    pthread_key_t threadKey;
    BLST_S_HEAD(NEXUS_ThreadHead, NEXUS_Thread) threads;
} NEXUS_P_Os_State;


static void *
NEXUS_P_ThreadEntry(void *t)
{
    uint8_t buf[1];

    NEXUS_ThreadHandle thread = t;
    pthread_setspecific(NEXUS_P_Os_State.threadKey, t);
    thread->stack_top = buf;
    BDBG_MSG(("thread '%s' stack %#lx", thread->name, (unsigned long)thread->stack_top));
    thread->pThreadFunc(thread->pContext);
    return NULL;
}

NEXUS_ThreadHandle
NEXUS_Thread_Create(const char *pThreadName, void (*pThreadFunc)(void *), void *pContext, const NEXUS_ThreadSettings *pSettings)
{
    NEXUS_ThreadHandle  thread;
    BERR_Code mrc;
    int rc;
    pthread_attr_t attr;

    BDBG_ASSERT(pThreadName);
    BDBG_ASSERT(pThreadFunc);

    if(!pSettings) {
        pSettings = &NEXUS_DefaultThreadSettings;
    }

    thread = BKNI_Malloc(sizeof(*thread));
    if(!thread) {
        mrc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto err_alloc;
    }
    BDBG_OBJECT_INIT(thread, NEXUS_Thread);
    strncpy(thread->name, pThreadName,sizeof(thread->name)-1);
    thread->name[sizeof(thread->name)-1]='\0';
    thread->pThreadFunc = pThreadFunc;
    thread->pContext = pContext;
    thread->settings = *pSettings;
    thread->stack_top = NULL;
    thread->stack_len = pSettings->stackSize;
    NEXUS_P_ThreadInfo_Init(&thread->info);
    thread->info.pThreadName = thread->name;

    rc = pthread_attr_init(&attr);
    if (rc!=0) {
        rc = BERR_TRACE(BERR_OS_ERROR);
        goto err_attr;
    }
    rc = pthread_attr_setstacksize(&attr, pSettings->stackSize);
    if (rc!=0) {
        mrc = BERR_TRACE(BERR_OS_ERROR);
        goto err_stack;
    }
    if (pthread_create(&thread->thread, &attr, NEXUS_P_ThreadEntry, thread)) {
        mrc = BERR_TRACE(BERR_OS_ERROR);
        goto err_thread;
    }
    pthread_attr_destroy(&attr);
    BLST_S_INSERT_HEAD(&NEXUS_P_Os_State.threads, thread, link);
    return thread;

err_thread:
err_stack:
    pthread_attr_destroy(&attr);
err_attr:
err_alloc:
    return NULL;
}

void
NEXUS_Thread_Destroy(NEXUS_ThreadHandle thread)
{
    int rc;
    BDBG_OBJECT_ASSERT(thread, NEXUS_Thread);
    rc = pthread_join(thread->thread, NULL);
    if(rc!=0) {
        BDBG_WRN(("pthread_join: failed %d for thread %s(%#lx)", rc, thread->info.pThreadName, (unsigned long)thread));
    }
    NEXUS_LockModule();
    BLST_S_REMOVE(&NEXUS_P_Os_State.threads, thread, NEXUS_Thread, link);
    NEXUS_UnlockModule();
    BDBG_OBJECT_DESTROY(thread, NEXUS_Thread);
    BKNI_Free(thread);
    return;
}

static void
NEXUS_P_Base_Os_MarkThread_locked(const char *name)
{
    NEXUS_ThreadHandle thread;
    uint8_t buf[1];

    BDBG_ASSERT(name);
    thread = pthread_getspecific(NEXUS_P_Os_State.threadKey);
    if(thread!=NULL) {
        BDBG_WRN(("NEXUS_P_Base_MarkThread: duplicate make '%s'<>'%s'", name, thread->name));
        return;
    }
    thread = BKNI_Malloc(sizeof(*thread));
    if(!thread) {
        NEXUS_Error rc = BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
        BSTD_UNUSED(rc);
        return;
    }
    pthread_setspecific(NEXUS_P_Os_State.threadKey, thread);
    /* initialiaze main pseudo thread */
    BDBG_OBJECT_INIT(thread, NEXUS_Thread);
    strncpy(thread->name, name, sizeof(thread->name)-1);
    NEXUS_P_ThreadInfo_Init(&thread->info);
    thread->info.pThreadName = thread->name;
    thread->pThreadFunc = NULL;
    thread->pContext = NULL;
    thread->settings = NEXUS_DefaultThreadSettings;
    thread->stack_top = buf + 1024; /* Guess top of the stack */
    thread->stack_len = 64*1024;
    BDBG_MSG(("thread '%s' stack %#lx", thread->name, (unsigned long)thread->stack_top));
    BLST_S_INSERT_HEAD(&NEXUS_P_Os_State.threads, thread, link);
    return;
}

void
NEXUS_P_Base_Os_MarkThread(const char *name)
{
    /* NEXUS_LockModule can't be uses since NEXUS_P_Base_Os_MarkThread_locked would create key, and Unlock would see then Unlock without Lock */
    BDBG_OBJECT_ASSERT(NEXUS_MODULE_SELF, NEXUS_Module);
    BKNI_AcquireMutex(NEXUS_MODULE_SELF->lock);
    NEXUS_P_Base_Os_MarkThread_locked(name);
    BKNI_ReleaseMutex(NEXUS_MODULE_SELF->lock);
    return;
}

BERR_Code
NEXUS_P_Base_Os_Init(void)
{
    int rc;

    BLST_S_INIT(&NEXUS_P_Os_State.threads);
    rc = pthread_key_create(&NEXUS_P_Os_State.threadKey, NULL);
    if(rc!=0) {
        return BERR_TRACE(BERR_OS_ERROR);
    }

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

    pthread_key_delete(NEXUS_P_Os_State.threadKey);
    return;
}

const char *
NEXUS_P_Base_Os_GetTaskNameFromStack(const unsigned long *stack)
{
    NEXUS_ThreadHandle  thread;
    /* don't acquire critical section here, no one shall create tasks while running profile report */
    for(thread = BLST_S_FIRST(&NEXUS_P_Os_State.threads); thread ; thread = BLST_S_NEXT(thread, link)) {
        if ((uint32_t)stack < (uint32_t)thread->stack_top && (uint32_t)stack >= (uint32_t)thread->stack_top - thread->stack_len) {
            /* bingo */
            BDBG_MSG(("'%s' thread for stack %#lx (%#lx...%#lx)", thread->name, (unsigned long)stack, (unsigned long)thread->stack_top - thread->stack_len, (unsigned long)thread->stack_top));
            return thread->name;
        }
    }
    BDBG_WRN(("unknown thread for stack %#lx", (unsigned long)stack));
    return NULL;
}


NEXUS_P_ThreadInfo *
NEXUS_P_ThreadInfo_Get(void)
{
    NEXUS_ThreadHandle  thread;
    thread = pthread_getspecific(NEXUS_P_Os_State.threadKey);
    if(thread) {
        return &thread->info;
    }
    return NULL;
}

const char *
NEXUS_GetEnv(const char *name)
{
    return getenv(name);
}

void
NEXUS_SetEnv(const char *name, const char *value)
{
    setenv(name, value, 1);
    return;
}


void
NEXUS_FlushCache(const void *pvAddr, size_t ulNumBytes)
{
    BSTD_UNUSED(pvAddr);
    BSTD_UNUSED(ulNumBytes);
    return;
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

