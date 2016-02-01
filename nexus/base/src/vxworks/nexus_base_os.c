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
#include "nexus_base.h"
#include "nexus_base_priv.h"

#include <taskLib.h>
#include <timers.h>

BDBG_MODULE(nexus_base_os);

void NEXUS_Time_Get(NEXUS_Time *time)
{
        int rc;
        struct timespec vxworks_time;

    BDBG_ASSERT(time);
        rc = clock_gettime(CLOCK_REALTIME, &vxworks_time);
        if (rc!=0) {
                BDBG_ERR(("clock_gettime returned %d, ignored", rc));
        }
        else {
                time->tv_sec = vxworks_time.tv_sec;
                time->tv_usec = vxworks_time.tv_nsec / 1000;    /* convert to usec */
                BDBG_ASSERT(time->tv_usec < 1000000); /* if this isn't right, things go bad downstream */
        }

    return;
}

long NEXUS_Time_Diff(const NEXUS_Time *future, const NEXUS_Time *past)
{
        /* tv_sec future-past should always be >=0, but not so with tv_usec. It could be negative, so handle it. */
    return 1000*(future->tv_sec - past->tv_sec) + ((int)future->tv_usec - (int)past->tv_usec)/1000;
}

void NEXUS_Time_Add(NEXUS_Time *time, long delta_ms)
{
    time->tv_usec += (delta_ms%1000) * 1000;
    time->tv_sec += delta_ms/1000;
    if (time->tv_usec > 1000000) {
        time->tv_usec -= 1000000;
        time->tv_sec++;
    }
    return;
}

#if 0
/* Convert System priority to VxWorks priority.
** That is, convert 0..100 (0=highest,100=lowest) to range to 0..255 (0=highest,255=lowest)
*/
static int b_convert_priority(int is)
{
    if (is >= 0 && is <= 100)
        is = is * 255 / 100;
    else
        is = 100; /* use this as a default */
    return is;
}
#endif

static const NEXUS_ThreadSettings NEXUS_DefaultThreadSettings = {
    100,   /* priority (default to "something in the middle") */
    8*1024 /* stackSize */
};

void NEXUS_Thread_GetDefaultSettings(NEXUS_ThreadSettings *pSettings)
{
    BDBG_ASSERT(pSettings);
    *pSettings = NEXUS_DefaultThreadSettings;
    return;
}

BDBG_OBJECT_ID(NEXUS_Thread);

struct NEXUS_Thread {
    int task; /* task id of the new thread */
    BDBG_OBJECT(NEXUS_Thread)
    char name[16];
    void (*pThreadFunc)(void *);
    void *pContext;
    NEXUS_ThreadSettings settings;
    NEXUS_P_ThreadInfo info;
};

/* This private function is called from the new kernel thread */
static int NEXUS_P_ThreadEntry(void *data)
{
    NEXUS_ThreadHandle task = data;

    BDBG_ASSERT(task);

    task->task = taskIdSelf();
    (task->pThreadFunc)(task->pContext);
    task->task = 0;
    return 0;
}


NEXUS_ThreadHandle NEXUS_P_Thread_Create(const char *pThreadName, void (*pThreadFunc)(void *), void *pContext, const NEXUS_ThreadSettings *pSettings)
{
    NEXUS_ThreadHandle thread;
    BERR_Code mrc;
    int priority, stackSize, task;

    BDBG_ASSERT(pThreadName);
    BDBG_ASSERT(pThreadFunc);

    if (!pSettings) pSettings = &NEXUS_DefaultThreadSettings;

    thread = BKNI_Malloc(sizeof(*thread));
    if (!thread) {
        mrc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        return NULL;
    }
    BKNI_Memset(thread, 0, sizeof(*thread));

    /* Use locals so we can change them witout changing the settings they provided */
    priority  = pSettings->priority;
    stackSize = pSettings->stackSize;
    /* If they used -1 then use these defaults */
    if (priority < 0)  priority = 70;
    if (stackSize < 0) stackSize = 8*1024;

    BDBG_OBJECT_INIT(thread, NEXUS_Thread);
    strncpy(thread->name, pThreadName,sizeof(thread->name)-1);
    thread->name[sizeof(thread->name)-1]='\0';
    thread->pThreadFunc = pThreadFunc;
    thread->pContext = pContext;
    thread->settings = *pSettings;
    NEXUS_P_ThreadInfo_Init(&thread->info);
    thread->info.pThreadName = thread->name;

    task = taskSpawn(
        thread->name, priority, 0 /*options?*/, stackSize,
        (FUNCPTR)NEXUS_P_ThreadEntry,
    (int)thread,
        0,0,0,0,0,0,0,0,0);

    if (task) return thread;

    BKNI_Free(thread);
    return NULL;
}

void NEXUS_Thread_Destroy(NEXUS_ThreadHandle thread)
{
    BDBG_OBJECT_ASSERT(thread, NEXUS_Thread);
    if (thread->task) {
        BKNI_Sleep(10);
        if (thread->task) {
            BDBG_WRN(("NEXUS_Thread_Destroy: %#lx killing (%s:%u)", (unsigned long)thread, thread->name, thread->task));
            taskDelete(thread->task);
            /* Wait for task to die. */
            BKNI_Sleep(500);
        }
    }

    BDBG_OBJECT_DESTROY(thread, NEXUS_Thread);
    BKNI_Free(thread);
    return;
}

#include <cacheLib.h>

void NEXUS_FlushCache(const void *pvAddr, size_t ulNumBytes)
{
    /* Note: the definition for the VxWorks 'cacheFlush' function says "this operation MAY also invalidate the cache tags".
    ** This function by definition must flush AND invalidate.
    ** Because some cache libraries do invalidate and some don't we have to make sure it does (even if it costs a second
    ** invalidate) or we will get data corruption issues.
    */
    if (!((unsigned long)pvAddr & 0x20000000)) {
        cacheFlush(DATA_CACHE, (void *) pvAddr, ulNumBytes);
        cacheInvalidate(DATA_CACHE, (void *) pvAddr, ulNumBytes);
    }
    return;
}


NEXUS_P_ThreadInfo *NEXUS_P_ThreadInfo_Get(void)
{
    return NULL;
}

static struct {
    unsigned count;
    struct {
        const char *key,*value;
    } env[64];
} NEXUS_P_OsEnv = {
    0,
   /* *** */
    {{ NULL, NULL }}
};

const char *NEXUS_GetEnv(const char *name)
{
    unsigned i;
    for (i=0;i<NEXUS_P_OsEnv.count;i++) {
        if (!NEXUS_P_Base_StrCmp(NEXUS_P_OsEnv.env[i].key, name)) {
            return NEXUS_P_OsEnv.env[i].value;
        }
    }
    return NULL;
}

void NEXUS_SetEnv(const char *name, const char *value)
{
    unsigned i;
    if (value) {
    	/* if already there, replace old with new */
    	for (i=0;i<NEXUS_P_OsEnv.count;i++) {
        	if (!NEXUS_P_Base_StrCmp(NEXUS_P_OsEnv.env[i].key, name)) {
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
    } else {
    	/* if there, delete the entry */
    	for (i=0;i<NEXUS_P_OsEnv.count;i++) {
        	if (!NEXUS_P_Base_StrCmp(NEXUS_P_OsEnv.env[i].key, name)) {
            	NEXUS_P_OsEnv.env[i].key = ""; /* just make the key so it doesn't match */
            	return;
        	}
    	}
    }
    return;
}

/* I'm tired of always having to type "msg_modules" */
void set(char *message_modules) { NEXUS_SetEnv("msg_modules", message_modules); }

#include <kernelLib.h>
BERR_Code NEXUS_P_Base_Os_Init(void)
{
#if 1
    /* Our system uses scheduler tasks to do a lot of the system work.
    ** There are four tasks with the names: sched_idle, sched, sched_low, and sched_high.
    ** These tasks are all created at the same priority and are assumed to time slice and get equal run-time.
    ** Its the functions assigned to these tasks that designate how "sched_high" is considered "high priority".
    ** We need to ensure that tasks of the priority are time-sliced so ...
    */
    kernelTimeSlice(1); /* run each task for 1 millisecond slices */
#endif
    return BERR_SUCCESS;
}

void NEXUS_P_Base_Os_Uninit(void)
{
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

int NEXUS_atoi(const char *str)
{
    return atoi(str);
}

#include <ctype.h>
int
strcasecmp(const char *s1, const char *s2)
{
    int i;
    for (i=0;s1[i] && s2[i];i++) {
        char c1 = tolower((int)s1[i]);
        char c2 = tolower((int)s2[i]);
        if (c1 < c2) return -1;
        if (c1 > c2) return 1;
    }
    /* if one string is longer than the other then they don't compare */
    if (s1[i]) return 1; /*s1 longer than s2*/
    if (s2[i]) return -1;/*s2 longer than s1*/
    return 0;
}


int
strncasecmp(const char *s1, const char *s2, size_t n)
{
    int i;
    for (i=0;i<(int)n;i++) {
        char c1, c2;
        if (!s1[i] && !s2[i]) return 0; /*equal if both same when hit the ends*/
        if (!s1[i]) return -1; /*s2 longer than s1*/
        if (!s2[i]) return 1;  /*s1 longer than s2*/
        c1 = tolower((int)s1[i]);
        c2 = tolower((int)s2[i]);
        if (c1 < c2) return -1;
        if (c1 > c2) return 1;
    }
    return 0;
}


