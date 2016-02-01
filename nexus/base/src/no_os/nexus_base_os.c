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
#include <string.h>
#include <stdlib.h>
#include "nexus_base.h"
#include "nexus_base_priv.h"


extern uint32_t total_ticks;

BDBG_MODULE(nexus_base_os);

void 
NEXUS_Time_Get(NEXUS_Time *time)
{
    time->tv_sec = total_ticks/100;
    time->tv_usec = (total_ticks%100)*1000;

    return;
}

long 
NEXUS_Time_Diff(const NEXUS_Time *future, const NEXUS_Time *past)
{
        /* tv_sec future-past should always be >=0, but not so with tv_usec. It could be negative, so handle it. */
    return 1000*(future->tv_sec - past->tv_sec) + (future->tv_usec - past->tv_usec)/1000;
}

void 
NEXUS_Time_Add(NEXUS_Time *time, long delta_ms)
{
    time->tv_usec += (delta_ms%1000) * 1000;
    time->tv_sec += delta_ms/1000;
    if (time->tv_usec > 1000000) {
        time->tv_usec -= 1000000;
        time->tv_sec++;
    }
    return;
}


static const NEXUS_ThreadSettings NEXUS_DefaultThreadSettings = {
    100,   /* priority (default to "something in the middle") */
    8*1024 /* stackSize */
};

void 
NEXUS_Thread_GetDefaultSettings(NEXUS_ThreadSettings *pSettings)
{
    BDBG_ASSERT(pSettings);
    *pSettings = NEXUS_DefaultThreadSettings;
    return;
}

#if (BCHP_CHIP==7358) || (BCHP_CHIP==7408) /* This is needed until nexus_base.c /main/47 or later is used */
BERR_Code BERR_StoreError_tagged(const char *file, unsigned lineno, const char *error, BERR_Code rc)
{
    /* not yet implemented */
    if (rc) {
        BKNI_Printf("!!!Error %s(%#x) at %s:%d\n", error, rc, file, lineno); /* mimic original BERR_TRACE behavior */
    }
    return rc;
}

void NEXUS_GetLastError_Os(BERR_Code *code)
{
    /* not yet implemented */
    *code = 0;
    return;
}
#endif

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


int taskSpawn(void)
{
    return 1;
}

NEXUS_ThreadHandle 
NEXUS_P_Thread_Create(const char *pThreadName, void (*pThreadFunc)(void *), void *pContext, const NEXUS_ThreadSettings *pSettings)
{
    NEXUS_ThreadHandle thread;
    BERR_Code mrc;
    int priority, stackSize, task=0;

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

    #if 0
    task = taskSpawn(
        thread->name, priority, 0 /*options?*/, stackSize,
        (FUNCPTR)NEXUS_P_ThreadEntry,
    (int)thread,
        0,0,0,0,0,0,0,0,0);
    #else
    task = taskSpawn();
    #endif

    if (task) return thread;

    BKNI_Free(thread);
    return NULL;
}

void 
NEXUS_Thread_Destroy(NEXUS_ThreadHandle thread)
{
    BSTD_UNUSED(thread);
    return;
}

static void
NEXUS_P_Base_Os_MarkThread_locked(const char *name)
{
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
    return BERR_SUCCESS;
}

void 
NEXUS_P_Base_Os_Uninit(void)
{
    return;
}


NEXUS_P_ThreadInfo *
NEXUS_P_ThreadInfo_Get(void)
{
    /* Not sure what this is supposed to do */
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

const char *
NEXUS_GetEnv(const char *name)
{
    unsigned i;

    for (i=0;i<NEXUS_P_OsEnv.count;i++) {
        if (!NEXUS_P_Base_StrCmp(NEXUS_P_OsEnv.env[i].key, name)) {
            return NEXUS_P_OsEnv.env[i].value;
        }
    }
    return NULL;
}

void 
NEXUS_SetEnv(const char *name, const char *value)
{
    unsigned i;
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
    return;
}

void 
NEXUS_FlushCache(const void *pvAddr, size_t ulNumBytes)
{
    #ifdef CACHE_WORKAROUND
        flush_dcache((unsigned long)pvAddr, (unsigned long)((unsigned char*)pvAddr + ulNumBytes));
    #else
        clear_d_cache((uint32_t)pvAddr, (uint32_t)ulNumBytes);
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

int
NEXUS_P_Base_StrCmp(const char *str1, const char *str2)
{
    int ch1, ch2, diff;

    for(;;) {
        ch1 = *str1++;
        ch2 = *str2++;
        diff = ch1 - ch2;
        if (diff) {
            return diff;
        } else if (ch1=='\0') {
            return 0;
        }
    }
}
