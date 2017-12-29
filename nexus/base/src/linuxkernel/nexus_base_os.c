/***************************************************************************
*  Copyright (C) 2008-2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#include <linux/version.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,8,0)
#include <linux/kconfig.h>
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,37)
#include <generated/autoconf.h>
#else
#include <linux/autoconf.h>
#endif
#include <linux/pci.h>
#include <linux/jiffies.h>
#include <linux/sched.h>
#include <linux/workqueue.h>

BDBG_MODULE(nexus_base_os);

void
NEXUS_Time_Get_isrsafe(NEXUS_Time *time)
{
    *time = jiffies;
    return;
}

long
NEXUS_Time_Diff_isrsafe(const NEXUS_Time *future, const NEXUS_Time *past)
{
    long jiffies_diff = (long)(*future) - (long)(*past); /* this would take care  by its own of wrap in the jiffies */
    long msec_diff; /* however jiffies_to_msecs takes unsigned and return unsigned, so if difference negative, convert it */
    if(jiffies_diff>=0) {
        msec_diff = jiffies_to_msecs(jiffies_diff);
    } else {
        msec_diff = -(long)jiffies_to_msecs(-jiffies_diff);
    }
    return msec_diff;
}

void
NEXUS_Time_Add(NEXUS_Time *time, long delta_ms)
{
    if(delta_ms > 0) {
        long delta_jiffies = msecs_to_jiffies(delta_ms);
        *time += delta_jiffies;
    }
    return;
}

void
NEXUS_Thread_GetDefaultSettings(NEXUS_ThreadSettings *pSettings)
{
    memset(pSettings, 0, sizeof(*pSettings));
    pSettings->priority = 1;
    pSettings->stackSize = 8*1024;
}

BDBG_OBJECT_ID(NEXUS_Thread);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30)

/* use kthread() */
#include <linux/kthread.h>

struct NEXUS_Thread {
    BDBG_OBJECT(NEXUS_Thread)
    char name[16];
    bool started;
    void (*pThreadFunc)(void *);
    void *pContext;
    NEXUS_ThreadSettings settings;
    NEXUS_P_ThreadInfo info;
    struct task_struct *task;
};

static int NEXUS_P_ThreadStart(void *data)
{
    NEXUS_ThreadHandle thread = data;

    NEXUS_Base_P_Thread_AssociateInfo(thread, current, &thread->info);
    NEXUS_LockModule();
    thread->started = true;
    NEXUS_UnlockModule();
    thread->pThreadFunc(thread->pContext);

    /* do not return until stopped */
    while (1) {
        set_current_state(TASK_INTERRUPTIBLE); /* go half asleep before checking condition */
        if (kthread_should_stop()) break;
        schedule();
    }

    return 0;
}

NEXUS_ThreadHandle
NEXUS_P_Thread_Create(const char *pThreadName, void (*pThreadFunc)(void *), void *pContext, const NEXUS_ThreadSettings *pSettings)
{
    NEXUS_ThreadHandle  thread;
    BERR_Code mrc;
    NEXUS_ThreadSettings defaultSettings;

    BDBG_ASSERT(pThreadName);
    BDBG_ASSERT(pThreadFunc);

    if(!pSettings) {
        NEXUS_Thread_GetDefaultSettings(&defaultSettings);
        pSettings = &defaultSettings;
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
    NEXUS_P_ThreadInfo_Init(&thread->info);
    thread->info.pThreadName = thread->name;

    /* in linux kernel stack size is fixed to 2 4K pages */
#define LINUX_KERNEL_STACK_SIZE (8*1024)
    if (thread->settings.stackSize < LINUX_KERNEL_STACK_SIZE) {
        BDBG_WRN(("NEXUS_Thread_Create: %s stack size %u forced to %u",  thread->name, (unsigned)thread->settings.stackSize, LINUX_KERNEL_STACK_SIZE));
    }
    thread->settings.stackSize = LINUX_KERNEL_STACK_SIZE;
    thread->started = false;

    thread->task = kthread_run(NEXUS_P_ThreadStart, thread, thread->name);
    if (!thread->task) {
        BKNI_Free(thread);
        mrc = BERR_TRACE(BERR_UNKNOWN);
        return NULL;
    }

    return thread;

err_alloc:
    return NULL;
}

void
NEXUS_Thread_Destroy(NEXUS_ThreadHandle thread)
{
    BDBG_OBJECT_ASSERT(thread, NEXUS_Thread);
    kthread_stop(thread->task);

    NEXUS_LockModule();
    if(thread->started) {
        NEXUS_Base_P_Thread_DisassociateInfo(thread, &thread->info);
    }
    BDBG_OBJECT_DESTROY(thread, NEXUS_Thread);
    BKNI_Free(thread);
    NEXUS_UnlockModule();
    return;
}

#else /* LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30) */

/* use kernel_thread() */

struct NEXUS_Thread {
    struct task_struct *task;
    BDBG_OBJECT(NEXUS_Thread)
    char name[16];
    void (*pThreadFunc)(void *);
    void *pContext;
    struct work_struct tq;
    NEXUS_ThreadSettings settings;
    NEXUS_P_ThreadInfo info;
};

/* This private function is called from the new kernel thread */
static int
NEXUS_P_ThreadEntry(void *data)
{
    NEXUS_ThreadHandle task = data;

    BDBG_ASSERT(task);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
    daemonize(task->name);
    allow_signal(SIGTERM);
    allow_signal(SIGINT);
    allow_signal(SIGKILL);
#else
    daemonize();
    strncpy(current->comm, task->name, sizeof(current->comm));
    current->comm[sizeof(current->comm)-1] = '\0';
    /* Must lock this before mucking with signals */
    spin_lock_irq(&current->sigmask_lock);
    siginitsetinv(&current->blocked, sigmask(SIGKILL)|sigmask(SIGINT)|sigmask(SIGTERM));
    spin_unlock_irq(&current->sigmask_lock);
#endif

    task->task = current;
    NEXUS_Base_P_Thread_AssociateInfo(task, current, &task->info);
    /* ignore the return value */
    (task->pThreadFunc)(task->pContext);
    task->task = NULL;
    return 0;
}

static void __attribute__((no_instrument_function))
NEXUS_P_ThreadStart(void *data)
{
    NEXUS_ThreadHandle task = data;
    kernel_thread((int (*)(void *))NEXUS_P_ThreadEntry, (void *)task, 0);
}

NEXUS_ThreadHandle
NEXUS_P_Thread_Create(const char *pThreadName, void (*pThreadFunc)(void *), void *pContext, const NEXUS_ThreadSettings *pSettings)
{
    NEXUS_ThreadHandle  thread;
    BERR_Code mrc;
    NEXUS_ThreadSettings defaultSettings;

    if (!pThreadName || !pThreadFunc) {
        BERR_TRACE(NEXUS_INVALID_PARAMETER);
        return NULL;
    }

    if (!pSettings) {
        NEXUS_Thread_GetDefaultSettings(&defaultSettings);
        pSettings = &defaultSettings;
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
    NEXUS_P_ThreadInfo_Init(&thread->info);
    thread->info.pThreadName = thread->name;

    /* in linux kernel stack size is fixed to 2 4K pages */
#define LINUX_KERNEL_STACK_SIZE (8*1024)
    if (thread->settings.stackSize < LINUX_KERNEL_STACK_SIZE) {
        BDBG_WRN(("NEXUS_Thread_Create: %s stack size %u forced to %u",  thread->name, thread->settings.stackSize, LINUX_KERNEL_STACK_SIZE));
    }
    thread->settings.stackSize = LINUX_KERNEL_STACK_SIZE;

    /* start launcher with task queue */
    INIT_WORK(&thread->tq, NEXUS_P_ThreadStart, thread);
    schedule_work(&thread->tq);

    return thread;

err_alloc:
    return NULL;
}

void
NEXUS_Thread_Destroy(NEXUS_ThreadHandle thread)
{
    BDBG_OBJECT_ASSERT(thread, NEXUS_Thread);
    if (thread->task) {
        BKNI_Sleep(10);
        if (thread->task) {
            BDBG_WRN(("NEXUS_Thread_Destroy: %#lx killing (%s:%u)", (unsigned long)thread, thread->name, thread->task->pid));
            kill_proc(thread->task->pid, SIGTERM, 1);
            /* Wait for task to die. */
            BKNI_Sleep(500);
        }
    }
    
    /* must lock after join and before accessing global state */
    NEXUS_LockModule();
    NEXUS_Base_P_Thread_DisassociateInfo(thread, &thread->info); 

    /* now we are sure the thread is in zombie state. We
    notify keventd to clean the process up. */
    kill_proc(2, SIGCHLD, 1);
    BDBG_OBJECT_DESTROY(thread, NEXUS_Thread);
    BKNI_Free(thread);
    NEXUS_UnlockModule();
    return;
}
#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30) */

#if NEXUS_CPU_ARM
#include "bcmdriver_arm.h"
#endif

void
NEXUS_FlushCache_isrsafe(const void *pvAddr, size_t ulNumBytes)
{
    static const char *addrTypeStr[NEXUS_AddrType_eMax] = {"cached","uncached","fake","unknown"};
    if (ulNumBytes) {
        NEXUS_AddrType addrtype = NEXUS_GetAddrType(pvAddr);
        switch (addrtype) {
        case NEXUS_AddrType_eFake:
          /* Nexus interface should be used with NEXUS_MemoryType_eFull to avoid this.
            Or, if eFull memory is limited, we could add a boolean to skip driver flush and require application flush. */
#if !NEXUS_CACHEFLUSHALL
          /* on B53 there is no correct flush_all (correct such as it would flush all cache hierarchy on all CPU */
            BDBG_ERR(("Can't flush fake address %p at %s:%u", pvAddr, BSTD_FILE, __LINE__));
            BKNI_Delay(100 * 1000); /* This delay is intentional, other option is BKNI_Fail */
            break;
#else
            BDBG_WRN(("flushing fake address %p results in flush all", pvAddr));
            pvAddr = 0;
            ulNumBytes = ~0;
            /* fall through */
#endif
        case NEXUS_AddrType_eCached:
        case NEXUS_AddrType_eUnknown: /* Unknown could correspond to dynamically managed memory mappings. */
#if NEXUS_CPU_ARM
            brcm_cpu_dcache_flush(pvAddr, ulNumBytes);
#else
            dma_cache_wback_inv((unsigned long)pvAddr, ulNumBytes);
#endif
            break;
        default:
            BDBG_WRN(("flushing invalid %s address %#lx", addrTypeStr[addrtype], (unsigned long)pvAddr));
            break;
        }
    }
    return;
}


NEXUS_P_ThreadInfo *
NEXUS_P_ThreadInfo_Get(void)
{
    return NEXUS_Base_P_Thread_GetInfo(current);
}

static struct {
    unsigned count; /* provides short-circuit to full array search */
    struct {
#define NEXUS_KEY_SIZE 32
#define NEXUS_VALUE_SIZE 64
        char key[NEXUS_KEY_SIZE];
        char value[NEXUS_VALUE_SIZE];
    } env[64];
} NEXUS_P_OsEnv = {
    0
   /* *** */
};

const char *
NEXUS_GetEnv_isrsafe(const char *name)
{
    unsigned i;
    NEXUS_P_CheckEnv_isrsafe(name);
    for(i=0;i<NEXUS_P_OsEnv.count;i++) {
        if (NEXUS_P_OsEnv.env[i].key[0] && NEXUS_P_Base_StrCmp(NEXUS_P_OsEnv.env[i].key, name)==0) {
            return NEXUS_P_OsEnv.env[i].value;
        }
    }
    return NULL;
}

void
NEXUS_SetEnv(const char *name, const char *value)
{
    unsigned i;
    unsigned freeslot = NEXUS_P_OsEnv.count;

    NEXUS_P_CheckEnv_isrsafe(name);
    for(i=0;i<NEXUS_P_OsEnv.count;i++) {
        if (!NEXUS_P_OsEnv.env[i].key[0] && freeslot == NEXUS_P_OsEnv.count) {
            freeslot = i;
        }

        if (NEXUS_P_OsEnv.env[i].key[0] && NEXUS_P_Base_StrCmp(NEXUS_P_OsEnv.env[i].key, name)==0) {
            if (!value) {
                /* if we're unsetting, free the slot but don't reduce the count */
                NEXUS_P_OsEnv.env[i].key[0] = 0;
                NEXUS_P_OsEnv.env[i].value[0] = 0;
            }
            else {
                b_strncpy(NEXUS_P_OsEnv.env[i].value, value, NEXUS_VALUE_SIZE);
            }
            return;
        }
    }
    if (!value) return;

    /* save new key.value pair */
    if (freeslot<sizeof(NEXUS_P_OsEnv.env)/sizeof(*NEXUS_P_OsEnv.env)) {
        b_strncpy(NEXUS_P_OsEnv.env[freeslot].key, name, NEXUS_KEY_SIZE);
        b_strncpy(NEXUS_P_OsEnv.env[freeslot].value, value, NEXUS_VALUE_SIZE);
        if (freeslot == NEXUS_P_OsEnv.count) {
            NEXUS_P_OsEnv.count++;
        }
    }
    else {
        BDBG_WRN(("Unable to store NEXUS_SetEnv(%s,%s)", name, value));
    }
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
    return (int)simple_strtol(str, NULL, 0);
}

int
NEXUS_P_Base_StrCmp_isrsafe(const char *str1, const char *str2)
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

int b_strlen(const char *s) 
{
    unsigned i = 0;
    while (*s++) i++;
    return i;
}

char *b_strncpy(char *dest, const char *src, int n)
{
    char *org = dest;
    while (n--) {
        *dest++ = *src;
        if (!*src++) break;
    }
    return org;
}

void NEXUS_P_Base_Os_MarkThread(const char *name)
{
    BSTD_UNUSED(name);
}

#if B_HAS_BPROFILE
const char *NEXUS_P_Base_Os_GetTaskNameFromStack(const unsigned long *stack)
{
    BSTD_UNUSED(stack);
    return NULL;
}
#endif
