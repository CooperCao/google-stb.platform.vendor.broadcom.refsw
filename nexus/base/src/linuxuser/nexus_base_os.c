/***************************************************************************
*  Copyright (C) 2018 Broadcom.
*  The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
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
#include "nexus_base.h"
#include "nexus_base_priv.h"
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/prctl.h>
#include <sys/time.h>

#if NEXUS_CPU_ARM && !NEXUS_BASE_MODE_PROXY && !B_REFSW_SYSTEM_MODE_CLIENT
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include "bcm_driver.h"
#endif

BDBG_MODULE(nexus_base_os);

#define NEXUS_P_USE_PTHREAD_TLS   1

void
NEXUS_Time_Get_isrsafe(NEXUS_Time *time)
{
    int rc;

    /* We no longer use gettimeofday. clock_gettime(CLOCK_MONOTONIC) is resilient to calendar time changes, which
    applications may need to perform. */
    rc = clock_gettime(CLOCK_MONOTONIC, time);
    if (rc!=0) {
        BDBG_ERR(("clock_gettime returned %d, ignored", rc));
    }
    return;
}

long
NEXUS_Time_Diff_isrsafe(const NEXUS_Time *future, const NEXUS_Time *past)
{
    return 1000*(future->tv_sec - past->tv_sec) + (future->tv_nsec - past->tv_nsec)/1000000;
}

long NEXUS_Time_DiffMicroseconds(const NEXUS_Time *future, const NEXUS_Time *past)
{
    return 1000000*(future->tv_sec - past->tv_sec) + (future->tv_nsec - past->tv_nsec)/1000;
}

void
NEXUS_Time_Add(NEXUS_Time *time, long delta_ms)
{
    if(delta_ms>0) {
        time->tv_nsec += (delta_ms%1000) * 1000000;
        time->tv_sec += delta_ms/1000;
        if (time->tv_nsec > 1000000000) {
            time->tv_nsec -= 1000000000;
            time->tv_sec++;
        }
    }
    return;
}

void NEXUS_GetTimestamp(NEXUS_Timestamp *timestamp)
{
    struct timeval tv;
    int rc;
    rc = gettimeofday(&tv, NULL);
    if (rc) {
        BERR_TRACE(rc);
        timestamp->val = 0;
    }
    else {
        timestamp->val = (((uint64_t)tv.tv_sec) << 32) | (unsigned)tv.tv_usec;
    }
}

void NEXUS_GetWallclockFromTimestamp( const NEXUS_Timestamp *timestamp, struct timeval *tv )
{
    tv->tv_sec = timestamp->val >> 32;
    tv->tv_usec = timestamp->val & 0xFFFFFFFF;
}

void
NEXUS_Thread_GetDefaultSettings(NEXUS_ThreadSettings *pSettings)
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->priority = 1;
    pSettings->stackSize = 64*1024;
#ifdef PTHREAD_STACK_MIN
    if (pSettings->stackSize < PTHREAD_STACK_MIN) {
        pSettings->stackSize = PTHREAD_STACK_MIN;
    }
#endif
}

BDBG_OBJECT_ID(NEXUS_Thread);

struct NEXUS_Thread {
    NEXUS_P_ThreadInfo info; /* must be the first member */
    char name[16];
    BDBG_OBJECT(NEXUS_Thread)
    pthread_t thread;
    void (*pThreadFunc)(void *);
    void *pContext;
    NEXUS_ThreadSettings settings;
    const uint8_t *stack_top;
    size_t stack_len;
    BLST_S_ENTRY(NEXUS_Thread) link;
};

static struct {
#if NEXUS_P_USE_PTHREAD_TLS
    pthread_key_t threadKey;
#endif
    BLST_S_HEAD(NEXUS_ThreadHead, NEXUS_Thread) threads;
} NEXUS_P_Os_State;



static void *
NEXUS_P_ThreadEntry(void *t)
{
    uint8_t buf[1];
    NEXUS_ThreadHandle thread = t;
    void *threadId = (void *)pthread_self();
    
    thread->stack_top = buf;
    BDBG_MSG(("thread '%s' stack %#lx", thread->name, (unsigned long)thread->stack_top));
    /* Push the name into the Linux process so it will be visible from procps tools */
    prctl(PR_SET_NAME, thread->name, 0, 0, 0);
    
#if NEXUS_P_USE_PTHREAD_TLS
    thread->info.threadId = threadId;
    pthread_setspecific(NEXUS_P_Os_State.threadKey, &thread->info);
#else
    NEXUS_Base_P_Thread_AssociateInfo(thread, threadId, &thread->info); 
#endif
    thread->pThreadFunc(thread->pContext);
    return NULL;
}

NEXUS_ThreadHandle
NEXUS_P_Thread_Create(const char *pThreadName, void (*pThreadFunc)(void *), void *pContext, const NEXUS_ThreadSettings *pSettings)
{
    NEXUS_ThreadHandle  thread;
    BERR_Code mrc;
    int rc;
    pthread_attr_t attr;
    NEXUS_ThreadSettings defaultSettings;

    if (!pThreadName || !pThreadFunc) {
        BERR_TRACE(NEXUS_INVALID_PARAMETER);
        return NULL;
    }

    if(!pSettings) {
        NEXUS_Thread_GetDefaultSettings(&defaultSettings);
        pSettings = &defaultSettings;
    }

    thread = BKNI_Malloc(sizeof(*thread));
    if(!thread) {
        mrc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        BSTD_UNUSED(mrc);
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
    thread->info.nexusThread = thread;
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
    BKNI_Free(thread);
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
        BDBG_ERR(("pthread_join: failed %d for thread %s(%#lx)", rc, thread->info.pThreadName, (unsigned long)thread));
    }
    
    /* must lock after join and before accessing global state */
    NEXUS_LockModule();
    BLST_S_REMOVE(&NEXUS_P_Os_State.threads, thread, NEXUS_Thread, link);
    if(rc==0) {
#if !NEXUS_P_USE_PTHREAD_TLS
        NEXUS_Base_P_Thread_DisassociateInfo(thread, &thread->info); 
#endif
        BDBG_OBJECT_DESTROY(thread, NEXUS_Thread);
        BKNI_Free(thread);
    }
    NEXUS_UnlockModule();
    return;
}

#if __GNUC__ >= 6
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Warray-bounds"
#endif

static void
NEXUS_P_Base_Os_MarkThread_locked(const char *name)
{
    NEXUS_ThreadHandle thread;
    NEXUS_P_ThreadInfo *threadInfo;
    uint8_t buf[1];
    void *threadId = (void *)pthread_self();

    BDBG_ASSERT(name);
#if NEXUS_P_USE_PTHREAD_TLS
    threadInfo = pthread_getspecific(NEXUS_P_Os_State.threadKey);
#else
    threadInfo = NEXUS_Base_P_Thread_GetInfo(threadId);
#endif
    if(threadInfo!=NULL) {
        thread = threadInfo->nexusThread;
        if(thread) {
            BDBG_WRN(("NEXUS_P_Base_MarkThread: duplicate make '%s'<>'%s'", name, thread->name));
            return;
        }
    }
    thread = BKNI_Malloc(sizeof(*thread));
    if(!thread) {
        NEXUS_Error rc = BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
        BSTD_UNUSED(rc);
        return;
    }
    /* initialize main pseudo thread */
    BDBG_OBJECT_INIT(thread, NEXUS_Thread);
    strncpy(thread->name, name, sizeof(thread->name)-1);
    NEXUS_P_ThreadInfo_Init(&thread->info);
    thread->info.pThreadName = thread->name;
    thread->info.nexusThread = thread;
    thread->info.threadId = threadId;
    thread->pThreadFunc = NULL;
    thread->pContext = NULL;
    NEXUS_Thread_GetDefaultSettings(&thread->settings);
    /* coverity[overrun: FALSE] */
    /* coverity[illegal_address: FALSE] */
    thread->stack_top = buf + 1024*8; /* This function is being called from a stack which already has some
        unknown amount of data on it. We need to guess top of the stack. */
    thread->stack_len = 64*1024;
    BDBG_MSG(("external thread '%s' stack %#lx", thread->name, (unsigned long)thread->stack_top));
    BLST_S_INSERT_HEAD(&NEXUS_P_Os_State.threads, thread, link);
#if NEXUS_P_USE_PTHREAD_TLS
    pthread_setspecific(NEXUS_P_Os_State.threadKey, &thread->info);
#else
    NEXUS_Base_P_Thread_AssociateInfo(thread, threadId, &thread->info); /* associates thread info with the thread */
#endif
    return;
}
#if __GNUC__ >= 6
#pragma GCC diagnostic pop
#endif

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

#if NEXUS_CPU_ARM && !NEXUS_BASE_MODE_PROXY
static int g_bcmdriver = -1;
#endif
#if NEXUS_P_USE_PTHREAD_TLS
static void NEXUS_P_Base_Os_FreeThreadInfo(void *info)
{
    NEXUS_P_ThreadInfo *threadInfo = info;

    if(threadInfo->nexusThread==NULL) { /* thread are not created with NEXUS_P_ThreadInfo */
        free(info); /* data was allocated with malloc, not with BKNI_Malloc */
    }
    return;
}
#endif
BERR_Code
NEXUS_P_Base_Os_Init(void)
{
    BLST_S_INIT(&NEXUS_P_Os_State.threads);
#if NEXUS_P_USE_PTHREAD_TLS
    {
        int rc = pthread_key_create(&NEXUS_P_Os_State.threadKey, NEXUS_P_Base_Os_FreeThreadInfo);
        if(rc!=0) {
            return BERR_TRACE(BERR_OS_ERROR);
        }
    }
#endif
#if NEXUS_CPU_ARM && !NEXUS_BASE_MODE_PROXY && !B_REFSW_SYSTEM_MODE_CLIENT
    /* Open user-mode driver */
    g_bcmdriver = open("/dev/brcm0", O_RDWR);
    if ( g_bcmdriver < 0 ) {
        return BERR_TRACE(BERR_OS_ERROR);
    }
#endif
    return BERR_SUCCESS;
}

void
NEXUS_P_Base_Os_Uninit(void)
{
    NEXUS_ThreadHandle thread;

    while(NULL!=(thread=BLST_S_FIRST(&NEXUS_P_Os_State.threads))) {
        BLST_S_REMOVE_HEAD(&NEXUS_P_Os_State.threads, link); /* don't free or destroy thread */
        if(thread->pThreadFunc!=NULL) { /* dummy placeholder */
            BDBG_WRN(("NEXUS_P_Base_Os_Uninit: %#lx leaked thread '%s'", (unsigned long)thread, thread->name));
        }
#if !NEXUS_P_USE_PTHREAD_TLS
        NEXUS_Base_P_Thread_DisassociateInfo(thread, &thread->info);
#endif
        BDBG_OBJECT_DESTROY(thread, NEXUS_Thread);
        BKNI_Free(thread); /* could delete it */
    }
#if NEXUS_P_USE_PTHREAD_TLS
    pthread_key_delete(NEXUS_P_Os_State.threadKey);
#endif
#if NEXUS_CPU_ARM && !NEXUS_BASE_MODE_PROXY
    close(g_bcmdriver);
    g_bcmdriver = -1;
#endif
    return;
}

#if NEXUS_CPU_ARM && !NEXUS_BASE_MODE_PROXY && !B_REFSW_SYSTEM_MODE_CLIENT
static BERR_Code NEXUS_P_Base_Os_CacheFlush(char *addr, int nbytes)
{
    BERR_Code result=0;
    t_bcm_linux_mem_addr_range addr_range;
    addr_range.address = (unsigned long)addr;
    addr_range.length =  (unsigned)nbytes ;
    if ( ioctl(g_bcmdriver, BRCM_IOCTL_FLUSH_DCACHE_RANGE, &addr_range) ) {
        return BERR_TRACE(BERR_OS_ERROR);
    }
    return result;
}
#endif

#if B_HAS_BPROFILE
const char *
NEXUS_P_Base_Os_GetTaskNameFromStack(const unsigned long *stack)
{
    NEXUS_ThreadHandle  thread;
    /* don't acquire critical section here, no one shall create tasks while running profile report */
    for(thread = BLST_S_FIRST(&NEXUS_P_Os_State.threads); thread ; thread = BLST_S_NEXT(thread, link)) {
        if ((uint8_t *)stack < (uint8_t *)thread->stack_top && (uint8_t *)stack >= (uint8_t *)thread->stack_top - thread->stack_len) {
            /* bingo */
            BDBG_MSG(("'%s' thread for stack %p (%p...%p)", thread->name, (void *)stack, (void *)(thread->stack_top - thread->stack_len), (void *)thread->stack_top));
            return thread->name;
        }
    }
#if 0
    for(thread = BLST_S_FIRST(&NEXUS_P_Os_State.threads); thread ; thread = BLST_S_NEXT(thread, link)) {
        BDBG_LOG(("'%s' thread for stack %p (%p...%p)", thread->name, (void *)stack, (void *)(thread->stack_top - thread->stack_len), (void *)thread->stack_top));
    }
#endif
    BDBG_WRN(("unknown thread for stack %p", (void *)stack));
    return NULL;
}
#endif

NEXUS_P_ThreadInfo *
NEXUS_P_ThreadInfo_Get(void)
{
    NEXUS_P_ThreadInfo *threadInfo;
#if NEXUS_P_USE_PTHREAD_TLS
    threadInfo = pthread_getspecific(NEXUS_P_Os_State.threadKey);
    if(threadInfo==NULL) {
        threadInfo = malloc(sizeof(*threadInfo)); /* Can't use BKNI_Malloc since this data for the main thread will stay alive past BKNI_Uninit */
        if(threadInfo) {
            NEXUS_P_ThreadInfo_Init(threadInfo);
            pthread_setspecific(NEXUS_P_Os_State.threadKey, threadInfo);
        }
    }
#else /* NEXUS_P_USE_PTHREAD_TLS */
    threadInfo = NEXUS_Base_P_Thread_GetInfo((void *)pthread_self());
#endif /* NEXUS_P_USE_PTHREAD_TLS */
    return threadInfo;
}

/* coverity[-tainted_string_return_content] */
const char *
NEXUS_GetEnv(const char *name)
{
    NEXUS_P_CheckEnv_isrsafe(name);
    return getenv(name);
}

void
NEXUS_SetEnv(const char *name, const char *value)
{
    NEXUS_P_CheckEnv_isrsafe(name);
    if (value) {
        setenv(name, value, 1);
    }
    else {
        unsetenv(name);
    }
    return;
}

#if !B_REFSW_SYSTEM_MODE_CLIENT
#if NEXUS_CPU_ARM
#if NEXUS_BASE_MODE_PROXY
BERR_Code  NEXUS_Platform_P_CacheFlush( void* addr, size_t nbytes );
#define cacheflush(pvAddr, ulNumBytes, DCACHE)  NEXUS_Platform_P_CacheFlush(pvAddr, ulNumBytes)
#else
#define cacheflush(pvAddr, ulNumBytes, DCACHE)  NEXUS_P_Base_Os_CacheFlush(pvAddr, ulNumBytes)
#endif
#elif defined B_REFSW_ANDROID
#include <sys/linux-syscalls.h>
#include <sys/cachectl.h>
#define cacheflush(pvAddr, ulNumBytes, DCACHE) syscall(__NR_cacheflush, (long int) pvAddr, ulNumBytes, DCACHE)
#else
#include <sys/cachectl.h>
#endif

void
NEXUS_FlushCache_isrsafe(const void *pvAddr, size_t ulNumBytes)
{
    int rc;
    NEXUS_AddrType addrtype;
    static const char *addrTypeStr[NEXUS_AddrType_eMax] = {"cached","uncached","fake","unknown"};

    addrtype = NEXUS_GetAddrType(pvAddr);
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
    case NEXUS_AddrType_eUnknown: /* must flush eUnknown because file module must flush OS malloc'd memory or dynamically managed memory mappings. */
        rc = cacheflush((void *)pvAddr, ulNumBytes, DCACHE);
        if (rc<0) {
            BDBG_ERR(("cacheflush has returned error %d, addr %p, size %u, ignored", rc, pvAddr, (unsigned)ulNumBytes));
        }
        break;
    default:
        BDBG_WRN(("flushing invalid %s address %#lx", addrTypeStr[addrtype], (unsigned long)pvAddr));
        break;
    }
    return;
}
#endif /* B_REFSW_SYSTEM_MODE_CLIENT */

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
NEXUS_P_Base_StrCmp_isrsafe(const char *str1, const char *str2)
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

