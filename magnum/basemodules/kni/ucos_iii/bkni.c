/***************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *
 * Module Description:
 *
 * Implementatation of the Magnum KNI for uC/OS-III applications.
 *
 ***************************************************************************/

#include "bstd.h"
#include "bkni.h"
#include "bkni_multi.h"
#include "bkni_event_group.h"
#include "blst_list.h"
#include "blst_slist.h"
#include <os.h>
#include <os_cfg_app.h>
#include "os_cfg.h"
#include "bsu_memory.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

extern int vprintf2(const char *templat, va_list marker);
uint32_t arch_getticks(void);

BDBG_MODULE(kernelinterface);

BDBG_OBJECT_ID(BKNI_EventGroup);
BDBG_OBJECT_ID(BKNI_Event);
BDBG_OBJECT_ID(BKNI_Mutex);

#define ASSERT_CRITICAL() (void)0
#define ASSERT_NOT_CRITICAL() (void)0
#define SET_CRITICAL() (void)0
#define CLEAR_CRITICAL() (void)0
extern unsigned char OSSchedLockNestingCtr;
extern unsigned char OSIntNestingCtr;
/*    #define CHECK_CRITICAL() ( bos_in_interrupt() || (OSSchedLockNestingCtr > 0) || (OSIntNestingCtr > 0)) */
#define CHECK_CRITICAL() (1)
#define TICKS_TO_MS(ticks)  (ticks * 1000/OS_CFG_TICK_RATE_HZ)
#define MS_TO_TICKS(x)      ((x * OS_CFG_TICK_RATE_HZ)/ 1000)

/*
static unsigned long BKNI_P_GetMicrosecondTick(void);
static int BKNI_P_SetTargetTime(struct timespec *target, int timeoutMsec);
*/

#if BKNI_DEBUG_MUTEX_TRACKING
static void BKNI_P_ForceReleaseMutex(void *);
typedef struct BKNI_P_HeartBeatTimer  {
    pthread_t thread;
    pthread_mutex_t lock;
    pthread_cond_t cond;
    bool exit;
    bool started;
} BKNI_P_HeartBeatTimer;

static BERR_Code BKNI_P_HeartBeatTimer_Init(BKNI_P_HeartBeatTimer *timer)
{
    pthread_condattr_t attr;
    int rc;
    
    rc = pthread_condattr_init(&attr);
    if (rc!=0) {
        return BERR_TRACE(BERR_OS_ERROR);
    }

#if HAS_NPTL
    rc = pthread_condattr_setclock(&attr, CLOCK_MONOTONIC);
    if (rc!=0) {
        return BERR_TRACE(BERR_OS_ERROR);
    }
#endif

    timer->exit = false;
    timer->started = false;
    pthread_mutex_init(&timer->lock, NULL);
    pthread_cond_init(&timer->cond, &attr);
    return BERR_SUCCESS;
}

static void BKNI_P_HeartBeatTimer_Uninit(BKNI_P_HeartBeatTimer *timer)
{
    if(timer->started) {
        timer->exit = true;
        pthread_cond_signal(&timer->cond);
        pthread_join(timer->thread, NULL);
    }
    pthread_cond_destroy(&timer->cond);
    pthread_mutex_destroy(&timer->lock);
    return;
}

static void *BKNI_P_HeartBeatTimer_Thread(BKNI_P_HeartBeatTimer *h, void (*timer)(void), unsigned delayms)
{
    int rc;
    rc = pthread_mutex_lock(&h->lock);
    BDBG_ASSERT(rc == 0);
    h->started = true;
    for(;;) {
        struct timespec target;
        BKNI_P_SetTargetTime(&target, (int)delayms);
        pthread_cond_timedwait(&h->cond, &h->lock, &target);
        if(h->exit) {
            break;
        }
        timer();
    }
    pthread_mutex_unlock(&h->lock);
    return NULL;
}
#endif /* if BKNI_DEBUG_MUTEX_TRACKING */

#define BKNI_P_MUTEXTRACKING_HEARTBEAT_TYPE BKNI_P_HeartBeatTimer
#define BKNI_P_MUTEXTRACKING_HEARTBEAT_INIT(x) BKNI_P_HeartBeatTimer_Init(x)
#define BKNI_P_MUTEXTRACKING_HEARTBEAT_UNINIT(x) BKNI_P_HeartBeatTimer_Uninit(x)
#define BKNI_P_MUTEXTRACKING_HEARTBEAT_LOCK(x) pthread_mutex_lock(&(x)->lock)
#define BKNI_P_MUTEXTRACKING_HEARTBEAT_UNLOCK(x) pthread_mutex_unlock(&(x)->lock)
#define BKNI_P_MUTEXTRACKING_HEARTBEAT_DECLARE(x,heartBeat,delayms) static void *BKNI_P_MutexTracking_HeartBeatThread(void *h) {return BKNI_P_HeartBeatTimer_Thread(h, x, delayms);}
#define BKNI_P_MUTEXTRACKING_HEARTBEAT_START(x) pthread_create(&(x)->thread, NULL, BKNI_P_MutexTracking_HeartBeatThread, x)
#define BKNI_P_MUTEXTRACKING_TICK_TYPE  unsigned long
#define BKNI_P_MUTEXTRACKING_TICK_GET(x) *(x) = BKNI_P_GetMicrosecondTick()
#define BKNI_P_MUTEXTRACKING_TICK_DIFF_MS(x1,x2) (*(x1) - *(x2))/1000
#define BKNI_P_MUTEXTRACKING_FORCE_RELEASE(x) BKNI_P_ForceReleaseMutex(x)

#include "bkni_mutex_tracking.inc"

struct BKNI_MutexObj 
{
    BKNI_P_MutexTracking tracking; /* must be first */
    BDBG_OBJECT(BKNI_Mutex)
    OS_MUTEX mutex;
};

#if BKNI_DEBUG_MUTEX_TRACKING
static void BKNI_P_ForceReleaseMutex(void *x)
{
   pthread_mutex_unlock(&((struct BKNI_MutexObj *)x)->mutex);
   return;
}
#endif

void * BKNI_Malloc_tagged(size_t size, const char *file, unsigned line);
void BKNI_Free_tagged(void *ptr, const char *file, unsigned line);


struct BKNI_GroupObj 
{
    BDBG_OBJECT(BKNI_EventGroup)
    BLST_D_HEAD(group, BKNI_EventObj) members;
    OS_MUTEX lock;            /* mutex for protecting signal and conditional variables */
    OS_SEM cond;           /* condition to wake up from event*/
};

struct BKNI_EventObj 
{
    BDBG_OBJECT(BKNI_Event)
    BLST_D_ENTRY(BKNI_EventObj) list;
    struct BKNI_GroupObj * group;
    OS_MUTEX lock;            /* mutex for protecting signal and conditional variables */
    OS_SEM cond;           /* condition to wake up from event*/
    bool signal;
};

#if 0
#define B_TRACK_ALLOC_LOCK() do {if (pthread_mutex_lock(&g_alloc_state_mutex)) {BKNI_Fail();}}while(0)
#define B_TRACK_ALLOC_UNLOCK() do {if (pthread_mutex_unlock(&g_alloc_state_mutex)) {BKNI_Fail();}}while(0)
#else
#define B_TRACK_ALLOC_LOCK()
#define B_TRACK_ALLOC_UNLOCK()
#endif
#define B_TRACK_ALLOC_ALLOC(size) malloc(size)
#define B_TRACK_ALLOC_FREE(ptr) free(ptr)
#define B_TRACK_ALLOC_OS "ucos_iii"

#include "bkni_track_mallocs.inc"

extern int vsnprintf(char *buf, size_t size, const char *fmt, va_list args);
extern bool bos_in_interrupt(void);

/* Create a new list head for our MUTEX free pool */
/* static BLST_S_HEAD(BKNI_P_MutexFreePool, BKNI_MutexObj) gMutexFreePool = BLST_S_INITIALIZER(gMutexFreePool); */
/* Create a new list head for our EVENT free pool */
/*static BLST_S_HEAD(BKNI_P_EventFreePool, BKNI_EventObj) gEventFreePool = BLST_S_INITIALIZER(gEventFreePool); */

uint32_t g_cp0_count_clocks_per_usec;
uint32_t g_cp0_count_clocks_per_sec;
uint32_t g_cpu_freq_hz;

#if BKNI_DEBUG_CS_TIMING
#define BKNI_DEBUG_CS_TIMING_TIME_NODES 16
/* microseconds */
#define BKNI_P_STATS_TIME_THRESHOLD 2

BLST_Q_HEAD(BKNI_P_TimeList, BKNI_P_TimeNode);
typedef struct BKNI_P_TimeList BKNI_P_TimeList;

typedef struct BKNI_P_TimeNodeData {
    unsigned long time;
    unsigned line;
    const char *file;
} BKNI_P_TimeNodeData;

typedef struct BKNI_P_TimeNode {
    BLST_Q_ENTRY(BKNI_P_TimeNode) link;
    BKNI_P_TimeNodeData data;
} BKNI_P_TimeNode;

typedef struct BKNI_P_TimeAggregate {
    unsigned count;
    unsigned long totalTime;
    unsigned long maxTime;
} BKNI_P_TimeAggregate;

/* set it to zero to disable stats */
#define BKNI_P_STATS_CS_SITES   256

#if BKNI_P_STATS_CS_SITES
#include "blst_aa_tree.h"

typedef struct BKNI_P_SiteKey {
    const char *file;
    unsigned line;
    long totalTime;
    unsigned count;
    unsigned long maxTime;
} BKNI_P_SiteKey;

BLST_AA_TREE_HEAD(BKNI_P_SiteInstanceTree, KNI_P_SiteNode);
BLST_AA_TREE_HEAD(BKNI_P_SiteTotalTimeTree, KNI_P_SiteNode);

typedef struct BKNI_P_SiteNode {
    BKNI_P_SiteKey key;
    BLST_AA_TREE_ENTRY(BKNI_P_SiteInstanceTree) instanceNode;
    BLST_AA_TREE_ENTRY(BKNI_P_SiteTotalTimeTree) totalTimeNode;
} BKNI_P_SiteNode;

static int BKNI_P_SiteNode_InstanceCompare(const BKNI_P_SiteNode *node, const BKNI_P_SiteKey *key)
{
    if(node->key.file != key->file) { return node->key.file > (char *)key->file? 1 : -1;}
    else return (int)node->key.line- (int)key->line;
}

BLST_AA_TREE_GENERATE_INSERT(BKNI_P_SiteInstanceTree, const BKNI_P_SiteKey *, BKNI_P_SiteNode, instanceNode, BKNI_P_SiteNode_InstanceCompare)
BLST_AA_TREE_GENERATE_FIND(BKNI_P_SiteInstanceTree, const BKNI_P_SiteKey *, BKNI_P_SiteNode, instanceNode, BKNI_P_SiteNode_InstanceCompare)
BLST_AA_TREE_GENERATE_REMOVE(BKNI_P_SiteInstanceTree, BKNI_P_SiteNode, instanceNode)

static int BKNI_P_SiteNode_TotalTimeCompare(const BKNI_P_SiteNode *node, const BKNI_P_SiteKey *key)
{
    if(node->key.totalTime != key->totalTime)  {return node->key.totalTime > key->totalTime ? 1 : -1;}
    else return BKNI_P_SiteNode_InstanceCompare(node, key);
}

BLST_AA_TREE_GENERATE_INSERT(BKNI_P_SiteTotalTimeTree, const BKNI_P_SiteKey *, BKNI_P_SiteNode, totalTimeNode, BKNI_P_SiteNode_TotalTimeCompare)
BLST_AA_TREE_GENERATE_REMOVE(BKNI_P_SiteTotalTimeTree, BKNI_P_SiteNode, totalTimeNode)
BLST_AA_TREE_GENERATE_FIRST(BKNI_P_SiteTotalTimeTree, BKNI_P_SiteNode, totalTimeNode)
BLST_AA_TREE_GENERATE_LAST(BKNI_P_SiteTotalTimeTree, BKNI_P_SiteNode, totalTimeNode)
BLST_AA_TREE_GENERATE_NEXT(BKNI_P_SiteTotalTimeTree, BKNI_P_SiteNode, totalTimeNode)

#endif /* BKNI_P_STATS_CS_SITES */


typedef struct BKNI_P_StatsState {
    struct {
        BKNI_P_TimeList list;
        BKNI_P_TimeAggregate aggregate;
        struct {
            BKNI_P_TimeAggregate aggregate;
            BKNI_P_TimeNodeData data[BKNI_DEBUG_CS_TIMING_TIME_NODES]; /* same as above but without linked list */
        } printCopy;
        BKNI_P_TimeNode nodes[BKNI_DEBUG_CS_TIMING_TIME_NODES];
    }csTime;
#if BKNI_P_STATS_CS_SITES
    struct {
        struct BKNI_P_SiteInstanceTree instanceTree;
        struct BKNI_P_SiteTotalTimeTree totalTimeTree;
        unsigned nextFree; /* indicates next free node that could be inserted into the trees */
        struct {
            BKNI_P_SiteKey data[BKNI_DEBUG_CS_TIMING_TIME_NODES];  /* that's right use BKNI_DEBUG_CS_TIMING_TIME_NODES (expected to be small amount of nodes used for printing) */
        } printCopy;
        BKNI_P_SiteNode nodes[BKNI_P_STATS_CS_SITES];
    } csSites;
#endif
} BKNI_P_StatsState;
static BKNI_P_StatsState g_BKNI_P_StatsState;

static void BKNI_P_StatsState_Init(void)
{
    unsigned i;
    BKNI_P_StatsState *state = &g_BKNI_P_StatsState;

    BKNI_Memset(state, 0, sizeof(*state));
    BLST_Q_INIT(&state->csTime.list);
    state->csTime.aggregate.count = 0;
    state->csTime.aggregate.totalTime = 0;
    state->csTime.aggregate.maxTime = 0;
    for(i=0;i<BKNI_DEBUG_CS_TIMING_TIME_NODES;i++) {
        BKNI_P_TimeNode *node = state->csTime.nodes+i;
        node->data.line = 0;
        node->data.time = 0;
        BLST_Q_INSERT_TAIL(&state->csTime.list, node, link);
    }
#if BKNI_P_STATS_CS_SITES
    BLST_AA_TREE_INIT(BKNI_P_SiteInstanceTree, &state->csSites.instanceTree);
    BLST_AA_TREE_INIT(BKNI_P_SiteTotalTimeTree, &state->csSites.totalTimeTree);
    state->csSites.nextFree = 0;
#endif
}

static void BKNI_P_TimeList_ReplaceSorted(BKNI_P_TimeList *list, BKNI_P_TimeNode *node)
{
    BKNI_P_TimeNode *n;
    unsigned long time = node->data.time;

    BDBG_ASSERT(node == BLST_Q_FIRST(list));
    BLST_Q_REMOVE_HEAD(list, link);
    for(n=BLST_Q_LAST(list);;n=BLST_Q_PREV(n,link)) {
        if(n==NULL) {
            BLST_Q_INSERT_HEAD(list, node, link);
            break;
        }
        if(time>=n->data.time) {
            BLST_Q_INSERT_AFTER(list, n, node, link);
            break;
        }
    }
    return;
}


static void BKNI_P_StatsState_Insert(unsigned long time, const char *file, unsigned line)
{
    BKNI_P_StatsState *state = &g_BKNI_P_StatsState;
    state->csTime.aggregate.count++;
    state->csTime.aggregate.totalTime += time;
    state->csTime.aggregate.maxTime = time > state->csTime.aggregate.maxTime ? time : state->csTime.aggregate.maxTime;
    if(time>BKNI_P_STATS_TIME_THRESHOLD) {
        BKNI_P_TimeNode *node = BLST_Q_FIRST(&state->csTime.list);
        if(time>node->data.time) {
            node->data.time = time;
            node->data.file = file;
            node->data.line = line;
            BKNI_P_TimeList_ReplaceSorted(&state->csTime.list,node);
        }
    }
#if BKNI_P_STATS_CS_SITES
    {
        BKNI_P_SiteNode *node;
        BKNI_P_SiteKey key;
        key.file = file;
        key.line = line;
        node = BLST_AA_TREE_FIND(BKNI_P_SiteInstanceTree, &state->csSites.instanceTree, &key);
        if(node) {
            BLST_AA_TREE_REMOVE(BKNI_P_SiteTotalTimeTree, &state->csSites.totalTimeTree, node);
            node->key.count ++;
            node->key.totalTime += time;
            node->key.maxTime = time > node->key.maxTime ? time: node->key.maxTime;
            BLST_AA_TREE_INSERT(BKNI_P_SiteTotalTimeTree, &state->csSites.totalTimeTree, &node->key, node);
        } else /* node==NULL */ {
            if(state->csSites.nextFree<BKNI_P_STATS_CS_SITES) {/* found unused entry */
                node = state->csSites.nodes + state->csSites.nextFree;
                state->csSites.nextFree++;
            } else { /* find entry with smallest totalTime */
                node = BLST_AA_TREE_LAST(BKNI_P_SiteTotalTimeTree, &state->csSites.totalTimeTree);
                BLST_AA_TREE_REMOVE(BKNI_P_SiteTotalTimeTree, &state->csSites.totalTimeTree, node);
                BLST_AA_TREE_REMOVE(BKNI_P_SiteInstanceTree, &state->csSites.instanceTree, node);
            }
            node->key.file = file;
            node->key.line = line;
            node->key.count = 1;
            node->key.totalTime = time == 0? 1 : time; /* rounging up totalTime to 1 */
            node->key.maxTime = time;
            BLST_AA_TREE_INSERT(BKNI_P_SiteInstanceTree, &state->csSites.instanceTree, &node->key, node);
            BLST_AA_TREE_INSERT(BKNI_P_SiteTotalTimeTree, &state->csSites.totalTimeTree, &node->key, node);
        }
    }
#endif /* BKNI_P_STATS_CS_SITES */
    return;
}

const char *
BKNI_P_PrepareFileName(const char *pFileName)
{
    const char *s;
    unsigned i;

    if(pFileName==NULL) {
        return "unknown";
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

BDBG_FILE_MODULE(bkni_statistics_cs);
void BKNI_P_Stats_Print(void)
{
    BKNI_P_TimeNode *node;
    BKNI_P_StatsState *state = &g_BKNI_P_StatsState;
    unsigned i;

    pthread_mutex_lock(&g_csMutex);
    state->csTime.printCopy.aggregate.count = state->csTime.aggregate.count;
    state->csTime.printCopy.aggregate.maxTime = state->csTime.aggregate.maxTime;
    state->csTime.printCopy.aggregate.totalTime = state->csTime.aggregate.totalTime;
    state->csTime.aggregate.count = 0;
    state->csTime.aggregate.maxTime = 0;
    state->csTime.aggregate.totalTime = 0;
    /* acquire critical section only to copy statistics */
    for(node = BLST_Q_LAST(&state->csTime.list), i=0;i<BKNI_DEBUG_CS_TIMING_TIME_NODES;i++,node=BLST_Q_PREV(node, link)) {
        BKNI_P_TimeNodeData *data = state->csTime.printCopy.data+i;
        BDBG_ASSERT(node);
        data->time = node->data.time;
        data->file = node->data.file;
        data->line = node->data.line;
        node->data.time = 0; /* clear nodes in the sorted list, but don't reinsert it's not required */
        node->data.line = 0;
        if(data->time<=BKNI_P_STATS_TIME_THRESHOLD) { /* stop copying data that wouldn't get printed */
            break;
        }
    }
#if BKNI_P_STATS_CS_SITES
    {
        BKNI_P_SiteNode *node;
        for(    i=0, node=BLST_AA_TREE_FIRST(BKNI_P_SiteTotalTimeTree, &state->csSites.totalTimeTree);
                node && i<BKNI_DEBUG_CS_TIMING_TIME_NODES;
                i++, node=BLST_AA_TREE_NEXT(BKNI_P_SiteTotalTimeTree, &state->csSites.totalTimeTree, node)) {
            BKNI_P_SiteKey *data = state->csSites.printCopy.data + i;

            if(node->key.totalTime <= BKNI_P_STATS_TIME_THRESHOLD) {
                data->totalTime = 0; /* mark it as the last */
                break;
            }
            *data =  node->key; /* copy data */
        }
        BLST_AA_TREE_INIT(BKNI_P_SiteInstanceTree, &state->csSites.instanceTree);
        BLST_AA_TREE_INIT(BKNI_P_SiteTotalTimeTree, &state->csSites.totalTimeTree);
        state->csSites.nextFree = 0;
    }
#endif /* BKNI_P_STATS_CS_SITES */
    pthread_mutex_unlock(&g_csMutex);
    if(state->csTime.printCopy.aggregate.count==0) {
        return;
    }
    BDBG_MODULE_MSG(bkni_statistics_cs, ("---- Critical Section Statistics: count:%u  total:%u max:%u avg:%u microsec ---", (unsigned)state->csTime.printCopy.aggregate.count, (unsigned)state->csTime.printCopy.aggregate.totalTime, (unsigned)state->csTime.printCopy.aggregate.maxTime, (unsigned)(state->csTime.printCopy.aggregate.totalTime/state->csTime.printCopy.aggregate.count)));
    for(i=0;i<BKNI_DEBUG_CS_TIMING_TIME_NODES;i++) {
        BKNI_P_TimeNodeData *data = state->csTime.printCopy.data+i;
        if(data->time <= BKNI_P_STATS_TIME_THRESHOLD) {
            break;
        }
       if(data->line) {
            BDBG_MODULE_MSG(bkni_statistics_cs,("%s:%u %u microsec", BKNI_P_PrepareFileName(data->file), data->line, data->time));
        }
    }
#if BKNI_P_STATS_CS_SITES
    for(i=0;i<BKNI_DEBUG_CS_TIMING_TIME_NODES;i++) {
        BKNI_P_SiteKey *data = state->csSites.printCopy.data + i;
        if(data->totalTime <= BKNI_P_STATS_TIME_THRESHOLD) {
            break;
        }
        BDBG_MODULE_MSG(bkni_statistics_cs,("%s:%u count:%u total:%u max:%u avg:%u microsec", BKNI_P_PrepareFileName(data->file), data->line, data->count, data->totalTime, data->maxTime, data->totalTime/data->count));
    }
#endif
    return;
}
#else
#define BKNI_P_StatsState_Init()
void BKNI_P_Stats_Print(void)
{
}

#endif /* BKNI_DEBUG_CS_TIMING */

static unsigned g_refcnt = 0;

BERR_Code BKNI_Init(void)
{
    BERR_Code result;

    if (g_refcnt++ == 0) {
        result = BKNI_P_MutexTrackingState_Init();
        BKNI_P_StatsState_Init();
        BKNI_P_TrackAlloc_Init();
    }
    else {
        result = 0;
    }

    return result;
}

/*
  We implement BKNI_Fail by executing jtag or software breakpoint
  instruction, causing system to drop in to the debugger. 
 */
#define BREAK_TYPE_SOFT 1
#define BREAK_TYPE_JTAG 2
#define BREAK_TYPE BREAK_TYPE_JTAG

struct critical_stance_t {
    OS_CPU_SR cpu_sr;
    unsigned int ref_count;
};

#ifdef MIPS_SDE
    struct critical_stance_t cs = {OS_ENABLE, 0};
#else
    struct critical_stance_t cs = {0, 0};
#endif

void BKNI_EnterCriticalSection(void)
{
#ifdef MIPS_SDE
    OS_CPU_SR  cpu_sr = 0;
    OS_ERR err;
    OSSchedLock(&err);
    if ((err != OS_ERR_NONE) && (err != OS_ERR_OS_NOT_RUNNING)) {
        BDBG_ERR(("OSSchedLock did not return OS_ERR_NONE or OS_ERR_OS_NOT_RUNNING\n"));
        BDBG_ASSERT(false);
    }

    OS_ENTER_CRITICAL();
    cs.cpu_sr = cpu_sr;
    cs.ref_count++;
#else
    OS_CPU_SR  cpu_sr = 0;
    OS_ENTER_CRITICAL();
    cs.cpu_sr = cpu_sr;
    cs.ref_count++;
#endif
}

void BKNI_Fail(void)
{
    /* Derefering 0 will cause a SIGSEGV will usually produce a core dump. */
    BDBG_P_PrintString("BKNI_Fail is intentionally causing a segfault. Please inspect any prior error messages or get a core dump stack trace to determine the cause of failure.\n");
    *(volatile unsigned char *)0;
    while(1);
}

BERR_Code BKNI_CreateMutex_tagged(BKNI_MutexHandle *handle, const char *file, int line)
{
    OS_ERR err;
    BKNI_MutexHandle mutex;
    ASSERT_NOT_CRITICAL();

    mutex = (BKNI_MutexHandle)BKNI_Malloc_tagged(sizeof(**handle), file, line);
    if (mutex==NULL) {
        return BERR_TRACE(BERR_OS_ERROR);
    }
    BDBG_OBJECT_SET(mutex, BKNI_Mutex);

    /* WARNING: Do not make BKNI_MutexHandle a recursive mutex. The usual motivation for doing this is to allow recursive calls back into
    Nexus or Magnum from custom callouts. That would be a violation of Nexus and Magnum architecture and will cause catastrophic failure.
    If your application needs its own recursive mutex, please create your own function and leave this unmodified. */
    OSMutexCreate(&mutex->mutex,
                  "BKNI_CreateMutex_tagged",
                  &err);
    if (err != OS_ERR_NONE) {
        BDBG_ERR(("OSMutexCreate did not return OS_ERR_NONE\n"));
        BDBG_OBJECT_DESTROY(*handle, BKNI_Mutex);
        BKNI_Free(mutex);
        return BERR_TRACE(BERR_OS_ERROR);
    }
    *handle = mutex;
    BKNI_P_MutexTracking_Init(&mutex->tracking, file, line);
    return BERR_SUCCESS;
}

void
BKNI_DestroyMutex_tagged(BKNI_MutexHandle handle, const char *file, int line)
{
    OS_ERR err;
    ASSERT_NOT_CRITICAL();
    BDBG_OBJECT_ASSERT(handle, BKNI_Mutex);

    BKNI_P_MutexTracking_Uninit(&handle->tracking);
    OSMutexDel(&handle->mutex,
               OS_OPT_DEL_ALWAYS,
               &err);
    if (err != OS_ERR_NONE) {
        BDBG_ERR(("OSMutexDel did not return OS_ERR_NONE\n"));
        BDBG_ASSERT(false);
    }
    BDBG_OBJECT_DESTROY(handle, BKNI_Mutex);
    BKNI_Free_tagged(handle, file, line);
    return ;
}

#undef BKNI_CreateMutex
BERR_Code BKNI_CreateMutex(BKNI_MutexHandle *handle)
{
    return BKNI_CreateMutex_tagged(handle, NULL, 0);
}

#undef BKNI_DestroyMutex
void BKNI_DestroyMutex(BKNI_MutexHandle handle)
{
    BKNI_DestroyMutex_tagged(handle, NULL, 0);
}

BERR_Code 
BKNI_TryAcquireMutex_tagged(BKNI_MutexHandle handle, const char *file, int line)
{
    OS_ERR err;
    BSTD_UNUSED(file);
    BSTD_UNUSED(line);
    ASSERT_NOT_CRITICAL();
    BDBG_OBJECT_ASSERT(handle, BKNI_Mutex);

    OSMutexPend(&handle->mutex,
                0,
                OS_OPT_PEND_NON_BLOCKING,
                NULL,
                &err);
    if (err == OS_ERR_NONE) {
        return BERR_SUCCESS;
    } else if (err == OS_ERR_TIMEOUT || err == OS_ERR_PEND_WOULD_BLOCK) {
        return BERR_TIMEOUT;
    } else {
        return BERR_TRACE(BERR_OS_ERROR);
    }
}

#undef BKNI_TryAcquireMutex
BERR_Code BKNI_TryAcquireMutex(BKNI_MutexHandle handle)
{
    return BKNI_TryAcquireMutex_tagged(handle, NULL, 0);
}

BERR_Code 
BKNI_AcquireMutex_tagged(BKNI_MutexHandle handle, const char *file, int line)
{
    OS_ERR err;
    ASSERT_NOT_CRITICAL();
    BDBG_OBJECT_ASSERT(handle, BKNI_Mutex);

    OSMutexPend(&handle->mutex,
                0,
                OS_OPT_PEND_BLOCKING,
                NULL,
                &err);
    if (err == OS_ERR_NONE) {
        BKNI_P_MutexTracking_AfterAcquire(&handle->tracking, file, line);
        return BERR_SUCCESS;
    } else {
        return BERR_TRACE(BERR_OS_ERROR);
    }
}

#undef BKNI_AcquireMutex
BERR_Code BKNI_AcquireMutex(BKNI_MutexHandle handle)
{
    return BKNI_AcquireMutex_tagged(handle, NULL, 0);
}

void
BKNI_ReleaseMutex(BKNI_MutexHandle handle)
{
    OS_ERR err;
    ASSERT_NOT_CRITICAL();
    BDBG_OBJECT_ASSERT(handle, BKNI_Mutex);

    OSMutexPost(&handle->mutex,
                OS_OPT_POST_NONE,
                &err);
    if (err != OS_ERR_NONE) {
        BDBG_ERR(("OSMutexPost did not return OS_ERR_NONE, err=%d, line=%d\n", err, __LINE__));
        BDBG_ASSERT(false);
    }
    return ;
}

void
BKNI_LeaveCriticalSection(void)
{
#ifdef MIPS_SDE
    OS_ERR err;
    OS_CPU_SR  cpu_sr = cs.cpu_sr;
    cs.ref_count--;
    OS_EXIT_CRITICAL();
    OSSchedUnlock(&err);
    if ((err != OS_ERR_NONE) && (err != OS_ERR_OS_NOT_RUNNING)) {
        BDBG_ERR(("OSSchedUnlock did not return OS_ERR_NONE or OS_ERR_OS_NOT_RUNNING:  err=%d\n", err));
        BDBG_ASSERT(false);
    }
#else
    OS_CPU_SR  cpu_sr = cs.cpu_sr;
    cs.ref_count--;
    OS_EXIT_CRITICAL();
#endif
}

int
BKNI_Printf(const char *fmt, ...)
{
    va_list arglist;
    int rc;

    va_start(arglist,fmt);
#if 0 //def BSU // Only necessary for BSU MIPS_SDE, but might as well to do it for ARM as well
    rc = vprintf2(fmt, arglist);
#else
    rc = vprintf(fmt, arglist);
#endif
    va_end(arglist);

    return rc;
}


int
BKNI_Snprintf(char *str, size_t len, const char *fmt, ...)
{
    va_list arglist;
    int rc;

    va_start(arglist,fmt);
    rc =vsnprintf(str, len, fmt, arglist);
    va_end(arglist);

    return rc;
}

int
BKNI_Vprintf(const char *fmt, va_list ap)
{
#if 0 //ndef NEWLIB // BSU // Only necessary for BSU MIPS_SDE, but might as well to do it for ARM as well
    return vprintf2(fmt, ap);
#else
    return vprintf(fmt, ap);
#endif
}


/**
BKNI_Delay impl notes:
This is an incredibly inefficient implementation...which is exactly
the point. Because the linux scheduler has a 10 milisecond clock tick,
this function should not hit the scheduler. It must use a busy loop.
sleep and usleep use the scheduler. nanasleep will use the scheduler
unless the pthread priority is high, which we cannot assume in this function.
Therefore a busy loop with a fine-grain time syscall does the job.
*/
void
BKNI_Delay(unsigned int microsec)
{
#ifdef MIPS_SDE
#ifdef DO_NOT_USE_FREE_RUNNING_COUNTER
    #error unsupported
#else
    int i;
    int final_count;
    for (i=0; i<microsec; i++) {
        final_count = CpuCountGet() + g_cp0_count_clocks_per_usec;
        while (CpuCountGet() < final_count);
    }
#endif
#else
    unsigned int i;
    unsigned int final_count;
    for (i=0; i<microsec; i++) {
        final_count = arch_getticks() + g_cp0_count_clocks_per_usec;
        while (arch_getticks() < final_count);
    }
#endif
}

BERR_Code
BKNI_Sleep(unsigned int millisec)
{
    unsigned int ticks = MS_TO_TICKS(millisec);
    OS_ERR err;

    if (ticks <= 0)
    {
        ticks = 1;
    }
    OSTimeDly(ticks,
              OS_OPT_TIME_DLY,
              &err);
    if (err != OS_ERR_NONE) {
        BDBG_ERR(("OSTimeDly did not return OS_ERR_NONE\n"));
        BDBG_ASSERT(false);
    }

    return BERR_SUCCESS;
}


BERR_Code 
BKNI_CreateEvent_tagged(BKNI_EventHandle *pEvent, const char *file, int line)
{
    BKNI_EventHandle event;
    BERR_Code result=BERR_SUCCESS;
    OS_ERR err;

    ASSERT_NOT_CRITICAL();

    event = BKNI_Malloc_tagged(sizeof(*event), file, line);
    *pEvent = event;
    if ( !event) {
        result = BERR_TRACE(BERR_OS_ERROR);
        goto err_no_memory;
    }
    BDBG_OBJECT_SET(event, BKNI_Event);

    OSMutexCreate(&event->lock,
                  "BKNI_CreateEvent_tagged",
                  &err);
    if (err != OS_ERR_NONE) {
        BDBG_ERR(("OSMutexCreate did not return OS_ERR_NONE\n"));
        BDBG_ASSERT(false);
    }

    OSSemCreate(&event->cond,
                "BKNI_CreateEvent",
                0,
                &err);
    if (err != OS_ERR_NONE) {
        BDBG_ERR(("OSSemCreate did not return OS_ERR_NONE\n"));
    }
    event->signal = false;
    event->group = NULL;

err_no_memory:
    return result;
}

void
BKNI_DestroyEvent_tagged(BKNI_EventHandle event, const char *file, int line)
{
    BKNI_EventGroupHandle group;
    OS_ERR err;

    ASSERT_NOT_CRITICAL();
    BDBG_OBJECT_ASSERT(event, BKNI_Event);
    group = event->group;
    /* At this point, we may have been removed from the group and event->group is NULL.
    This would be poor application code, but KNI should protect itself. */

    if (group) {
        BDBG_WRN(("Event %#x still in the group %#x, removing it", (unsigned)(unsigned long)event, (unsigned)(unsigned long)group));
        OSMutexPend(&group->lock,
                    0,
                    OS_OPT_PEND_BLOCKING,
                    NULL,
                    &err);
        if (err != OS_ERR_NONE) {
            BDBG_ERR(("OSMutexPend did not return OS_ERR_NONE %d, line=%d\n", err, __LINE__));
            BDBG_ASSERT(false);
        }
        /* if the group does not match, then the caller needs to fix their code. we can't have an event being added & removed from various
        groups and being destroyed at the same time. */
        BDBG_ASSERT(event->group == group);
        BLST_D_REMOVE(&group->members, event, list);
        OSMutexPost(&group->lock,
                    OS_OPT_POST_NONE,
                    &err);
        if (err != OS_ERR_NONE) {
            BDBG_ERR(("OSMutexPost did not return OS_ERR_NONE, err=%d, line=%d\n", err, __LINE__));
            BDBG_ASSERT(false);
        }
    }
    OSMutexDel(&event->lock,
               OS_OPT_DEL_ALWAYS,
               &err);
    if (err != OS_ERR_NONE) {
        BDBG_ERR(("OSMutexDel did not return OS_ERR_NONE\n"));
        BDBG_ASSERT(false);
    }
    OSSemDel(&event->cond,
             OS_OPT_DEL_NO_PEND,
             &err);
    if (err != OS_ERR_NONE) {
        BDBG_ERR(("OSSemDel did not return OS_ERR_NONE\n"));
        BDBG_ASSERT(false);
    }
    BDBG_OBJECT_DESTROY(event, BKNI_Event);
    BKNI_Free_tagged(event, file, line);
    return;
}

#undef BKNI_CreateEvent
BERR_Code BKNI_CreateEvent(BKNI_EventHandle *pEvent)
{
    return BKNI_CreateEvent_tagged(pEvent, NULL, 0);
}

#undef BKNI_DestroyEvent
void BKNI_DestroyEvent(BKNI_EventHandle event)
{
    BKNI_DestroyEvent_tagged(event, NULL, 0);
}

BERR_Code 
BKNI_WaitForEvent(BKNI_EventHandle event, int timeoutMsec)
{
    BERR_Code result = BERR_SUCCESS;
    int ticks;

    OS_ERR err, err2;
    if (!event) {
        return BERR_INVALID_PARAMETER;
    }
    
    if ( timeoutMsec != 0 )
    {
        ASSERT_NOT_CRITICAL();
    }
    BDBG_OBJECT_ASSERT(event, BKNI_Event);

    if (timeoutMsec!=0 && timeoutMsec!=BKNI_INFINITE) {
        if (timeoutMsec<0) {
            /* If your app is written to allow negative values to this function, then it's highly likely you would allow -1, which would
            result in an infinite hang. We recommend that you only pass positive values to this function unless you definitely mean BKNI_INFINITE. */
            BDBG_WRN(("BKNI_WaitForEvent given negative timeout. Possible infinite hang if timeout happens to be -1 (BKNI_INFINITE)."));
        }
        if (timeoutMsec<10) {
            timeoutMsec=10; /* This is used to achieve consistency between different OS's. */
        }
/*
        rc = BKNI_P_SetTargetTime(&target, timeoutMsec);
        if (rc) {
            return BERR_TRACE(BERR_OS_ERROR);
        }
*/
    }
    OSMutexPend(&event->lock,
                0,
                OS_OPT_PEND_BLOCKING,
                NULL,
                &err);
    if (err != OS_ERR_NONE) {
        BDBG_ERR(("OSMutexPend did not return OS_ERR_NONE %d, line=%d\n", err, __LINE__));
        BDBG_ASSERT(false);
    }
    if (event->signal) {
        event->signal = false;
        goto done;
    }
    if (timeoutMsec == 0) { /* wait without timeout */
        /* It is normal that BKNI_WaitForEvent could time out. Do not use BERR_TRACE. */
        result = BERR_TIMEOUT;
        goto done;
    }
    do {
        OSMutexPost(&event->lock,
                    OS_OPT_POST_NONE,
                    &err);
        if (err != OS_ERR_NONE) {
            BDBG_ERR(("OSMutexPost did not return OS_ERR_NONE, err=%d, line=%d\n", err, __LINE__));
            BDBG_ASSERT(false);
        }
        if (timeoutMsec == BKNI_INFINITE) {
            OSSemPend(&event->cond,
                      0,
                      OS_OPT_PEND_BLOCKING,
                      NULL,
                      &err);
            OSMutexPend(&event->lock,
                        0,
                        OS_OPT_PEND_BLOCKING,
                        NULL,
                        &err);
            if (err != OS_ERR_NONE) {
                BDBG_ERR(("OSMutexPend did not return OS_ERR_NONE %d, line=%d\n", err, __LINE__));
                BDBG_ASSERT(false);
            }
        } else {
            ticks = MS_TO_TICKS(timeoutMsec);
            if (ticks <= 0) {
                ticks = 1;
            }
            OSSemPend(&event->cond,
                      ticks,
                      OS_OPT_PEND_BLOCKING,
                      NULL,
                      &err2);
            if ((err2 != OS_ERR_NONE) && (err2 != OS_ERR_TIMEOUT)) {
                BDBG_ERR(("OSSemPend did not return OS_ERR_NONE or OS_ERR_TIMEOUT, err2=%d, line=%d\n", err2, __LINE__));
                BDBG_ASSERT(false);
            }
            OSMutexPend(&event->lock,
                        0,
                        OS_OPT_PEND_BLOCKING,
                        NULL,
                        &err);
            if (err != OS_ERR_NONE) {
                BDBG_ERR(("OSMutexPend did not return OS_ERR_NONE %d, line=%d\n", err, __LINE__));
                BDBG_ASSERT(false);
            }
            if (event->signal) {
                /* even if we timed out, if the signal was set, we succeed. this allows magnum to
                be resilient to large OS scheduler delays */
                result = BERR_SUCCESS;
                break;
            }
            if (err2==OS_ERR_TIMEOUT) {
                BDBG_MSG(("BKNI_WaitForEvent(%#x): timeout", (unsigned)(unsigned long)event));
                result = BERR_TIMEOUT;
                goto done;
            }
        }
        if (err != OS_ERR_NONE) {
            BDBG_ERR(("OSSemSet did not return OS_ERR_NONE, err=%d, file=%s, line=%d\n", err, __FILE__, __LINE__));
            BDBG_ASSERT(false);
        }
/*
        if(rc==EINTR) {
            BDBG_MSG(("BKNI_WaitForEvent(%#x): interrupted", (unsigned)(unsigned long)event));
            continue;
        }
        if (rc!=0) {
            result = BERR_TRACE(BERR_OS_ERROR);
            goto done;
        }
*/
    } while(!event->signal);  /* we might have been wokenup and then event has been cleared */

    event->signal = false;
done:
    OSMutexPost(&event->lock,
                OS_OPT_POST_NONE,
                &err);
    if (err != OS_ERR_NONE) {
        BDBG_ERR(("OSMutexPost did not return OS_ERR_NONE, err=%d, line=%d\n", err, __LINE__));
        BDBG_ASSERT(false);
    }
    return result;
}

void
BKNI_SetEvent(BKNI_EventHandle event)
{
    OS_ERR err;
    BKNI_EventGroupHandle group;

    BDBG_OBJECT_ASSERT(event, BKNI_Event);
    group = event->group;
    /* At this point, we may have been removed from the group and event->group is NULL.
    This is a real possibility because BKNI_SetEvent can be called from an ISR.
    Caching the group pointer allows us to safely unlock still. */

#ifdef OLD
    if (group) {
        OSMutexPend(&group->lock,
                    0,
                    OS_OPT_PEND_BLOCKING,
                    NULL,
                    &err);
        if (err != OS_ERR_NONE) {
            BDBG_ERR(("OSMutexPend did not return OS_ERR_NONE %d, line=%d\n", err, __LINE__));
            BDBG_ASSERT(false);
        }
    }
    OSMutexPend(&event->lock,
                0,
                OS_OPT_PEND_BLOCKING,
                NULL,
                &err);
    if (err != OS_ERR_NONE) {
        BDBG_ERR(("OSMutexPend did not return OS_ERR_NONE %d, line=%d\n", err, __LINE__));
        BDBG_ASSERT(false);
    }
#endif
    event->signal = true;
    OSSemPost((OS_SEM *)&event->cond,
                OS_OPT_POST_ALL,
                &err);
    if (err != OS_ERR_NONE) {
        BDBG_ERR(("OSSemPost did not return OS_ERR_NONE\n"));
        BDBG_ASSERT(false);
    }
    if (group) {
        OSSemPost((OS_SEM *)&group->cond,
                  OS_OPT_POST_ALL,
                  &err);
        if (err != OS_ERR_NONE) {
            BDBG_ERR(("OSSemPost did not return OS_ERR_NONE\n"));
            BDBG_ASSERT(false);
        }
    }
#ifdef OLD
    OSMutexPost(&event->lock,
                OS_OPT_POST_NONE,
                &err);
    if (err != OS_ERR_NONE) {
        BDBG_ERR(("OSMutexPost did not return OS_ERR_NONE, err=%d, line=%d\n", err, __LINE__));
        BDBG_ASSERT(false);
    }
    if (group) {
        OSMutexPost(&group->lock,
                    OS_OPT_POST_NONE,
                    &err);
        if (err != OS_ERR_NONE) {
            BDBG_ERR(("OSMutexPost did not return OS_ERR_NONE, err=%d, line=%d\n", err, __LINE__));
            BDBG_ASSERT(false);
        }
    }
#endif
    return ;
}

void
BKNI_ResetEvent(BKNI_EventHandle event)
{
    OS_ERR err;

    BDBG_OBJECT_ASSERT(event, BKNI_Event);
    OSMutexPend(&event->lock,
                0,
                OS_OPT_PEND_BLOCKING,
                NULL,
                &err);
    if (err != OS_ERR_NONE) {
        BDBG_ERR(("OSMutexPend did not return OS_ERR_NONE %d, line=%d\n", err, __LINE__));
        BDBG_ASSERT(false);
    }
    event->signal = false ;
    OSMutexPost(&event->lock,
                OS_OPT_POST_NONE,
                &err);
    if (err != OS_ERR_NONE) {
        BDBG_ERR(("OSMutexPost did not return OS_ERR_NONE, err=%d, line=%d\n", err, __LINE__));
        BDBG_ASSERT(false);
    }
    return ;
}

int
BKNI_Vsnprintf(char *s, size_t n, const char *fmt, va_list ap)
{
    return vsnprintf(s, n, fmt, ap);
}

BERR_Code 
BKNI_CreateEventGroup(BKNI_EventGroupHandle *pGroup)
{
    BKNI_EventGroupHandle group;
    BERR_Code result = BERR_SUCCESS;
    OS_ERR err;

    group = (BKNI_EventGroupHandle)BKNI_Malloc(sizeof(*group));
    if (!group) {
        result = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto err_no_memory;
    }
    BDBG_OBJECT_SET(group, BKNI_EventGroup);

    BLST_D_INIT(&group->members);
    OSMutexCreate(&group->lock,
                  "BKNI_CreateEventGroup",
                  &err);
    if (err != OS_ERR_NONE) {
        result = BERR_TRACE(BERR_OS_ERROR);
        goto err_mutex;
    }

    OSSemCreate(&group->cond,
                "BKNI_CreateEventGroup",
                0,
                &err);
    if (err != OS_ERR_NONE) {
        result = BERR_TRACE(BERR_OS_ERROR);
        goto err_condvar;
    }

    *pGroup = group;

    return BERR_SUCCESS;

err_condvar:
    BDBG_ERR(("before OSMutexDel:  group=%p, line=%d\n", group, __LINE__));
    OSMutexDel(&group->lock,
               OS_OPT_DEL_ALWAYS,
               &err);
err_mutex:
    BDBG_OBJECT_DESTROY(group, BKNI_EventGroup);
    BKNI_Free((void *)group);
err_no_memory:
    return result;
}

void
BKNI_DestroyEventGroup(BKNI_EventGroupHandle group)
{
    BKNI_EventHandle event;
    OS_ERR err;

    ASSERT_NOT_CRITICAL();
    BDBG_OBJECT_ASSERT(group, BKNI_EventGroup);

    OSMutexPend(&group->lock,
                0,
                OS_OPT_PEND_BLOCKING,
                0,
                &err);
    if (err != OS_ERR_NONE) {
        BDBG_ERR(("OSMutexPend did not return OS_ERR_NONE %d, line=%d\n", err, __LINE__));
        BDBG_ASSERT(false);
    }

    while( NULL != (event=BLST_D_FIRST(&group->members)) ) {
        BDBG_ASSERT(event->group == group);
        event->group = NULL;
        BLST_D_REMOVE_HEAD(&group->members, list);
    }
    OSMutexPost(&group->lock,
                OS_OPT_POST_NONE,
                &err);
    if (err != OS_ERR_NONE) {
        BDBG_ERR(("OSMutexPost did not return OS_ERR_NONE, err=%d, line=%d\n", err, __LINE__));
        BDBG_ASSERT(false);
    }

    OSMutexDel(&group->lock,
               OS_OPT_DEL_ALWAYS,
               &err);
    if (err != OS_ERR_NONE) {
        BDBG_ERR(("OSMutexDel did not return OS_ERR_NONE\n"));
        BDBG_ASSERT(false);
    }
    OSSemDel(&group->cond,
             OS_OPT_DEL_NO_PEND,
             &err);
    if (err != OS_ERR_NONE) {
        BDBG_ERR(("OSSemDel did not return OS_ERR_NONE\n"));
        BDBG_ASSERT(false);
    }
    BKNI_Free((void *)group);

    return;
}

BERR_Code
BKNI_AddEventGroup(BKNI_EventGroupHandle group, BKNI_EventHandle event)
{
    BERR_Code result = BERR_SUCCESS;
    OS_ERR err;

    ASSERT_NOT_CRITICAL();
    BDBG_OBJECT_ASSERT(group, BKNI_EventGroup);
    BDBG_OBJECT_ASSERT(event, BKNI_Event);

    /* IMPORTANT: group lock shall be acquired before event lock */
    OSMutexPend(&group->lock,
                0,
                OS_OPT_PEND_BLOCKING,
                0,
                &err);
    if (err != OS_ERR_NONE) {
        BDBG_ERR(("OSMutexPend did not return OS_ERR_NONE %d, line=%d\n", err, __LINE__));
        BDBG_ASSERT(false);
    }
    OSMutexPend(&event->lock,
                0,
                OS_OPT_PEND_BLOCKING,
                0,
                &err);
    if (err != OS_ERR_NONE) {
        BDBG_ERR(("OSMutexPend did not return OS_ERR_NONE %d, line=%d\n", err, __LINE__));
        BDBG_ASSERT(false);
    }
    if (event->group != NULL) {
        BDBG_ERR(("Event %#x already connected to the group %#x", (unsigned)(unsigned long)event, (unsigned)(unsigned long)group));
        result = BERR_TRACE(BERR_OS_ERROR);
    } else {
        BLST_D_INSERT_HEAD(&group->members, event, list);
        event->group = group;
        if (event->signal) {
            /* signal condition if signal already set */
            OSSemPost((OS_SEM *)&group->cond,
                      OS_OPT_POST_ALL,
                      &err);
            if (err != OS_ERR_NONE) {
                BDBG_ERR(("OSSemPost did not return OS_ERR_NONE\n"));
                BDBG_ASSERT(false);
            }
        }
    }
    OSMutexPost(&event->lock,
                OS_OPT_POST_NONE,
                &err);
    if (err != OS_ERR_NONE) {
        BDBG_ERR(("OSMutexPost did not return OS_ERR_NONE, err=%d, line=%d\n", err, __LINE__));
        BDBG_ASSERT(false);
    }
    OSMutexPost(&group->lock,
                OS_OPT_POST_NONE,
                &err);
    if (err != OS_ERR_NONE) {
        BDBG_ERR(("OSMutexPost did not return OS_ERR_NONE, err=%d, line=%d\n", err, __LINE__));
        BDBG_ASSERT(false);
    }
    return result;
}

BERR_Code
BKNI_RemoveEventGroup(BKNI_EventGroupHandle group, BKNI_EventHandle event)
{
    BERR_Code result = BERR_SUCCESS;
    OS_ERR err;

    OSMutexPend(&group->lock,
                0,
                OS_OPT_PEND_BLOCKING,
                NULL,
                &err);
    if (err != OS_ERR_NONE) {
        BDBG_ERR(("OSMutexPend did not return OS_ERR_NONE %d, line=%d\n", err, __LINE__));
        BDBG_ASSERT(false);
    }
    if (event->group != group) {
        BDBG_ERR(("Event %p doesn't belong to the group %p", (void *)event, (void *)group));
        result = BERR_TRACE(BERR_OS_ERROR);
    } else {
        BLST_D_REMOVE(&group->members, event, list);
        event->group = NULL;
    }

    OSMutexPost(&group->lock,
                OS_OPT_POST_NONE,
                &err);
    if (err != OS_ERR_NONE) {
        BDBG_ERR(("OSMutexPost did not return OS_ERR_NONE, err=%d, line=%d\n", err, __LINE__));
        BDBG_ASSERT(false);
    }
    return result;
}

static unsigned
group_get_events(BKNI_EventGroupHandle group, BKNI_EventHandle *events, unsigned max_events)
{
    BKNI_EventHandle ev;
    unsigned event;
    OS_ERR err;

    BDBG_OBJECT_ASSERT(group, BKNI_EventGroup);

    for(event=0, ev=BLST_D_FIRST(&group->members); ev && event<max_events ; ev=BLST_D_NEXT(ev, list)) {
        BDBG_OBJECT_ASSERT(ev, BKNI_Event);
        OSMutexPend(&ev->lock,
                    0,
                    OS_OPT_PEND_BLOCKING,
                    NULL,
                    &err);
        if (err != OS_ERR_NONE) {
            BDBG_ERR(("OSMutexPend did not return OS_ERR_NONE %d, line=%d\n", err, __LINE__));
            BDBG_ASSERT(false);
        }
        if (ev->signal) {
            ev->signal = false;
            events[event] = ev;
            event++;
        }
        OSMutexPost(&ev->lock,
                    OS_OPT_POST_NONE,
                    &err);
        if (err != OS_ERR_NONE) {
            BDBG_ERR(("OSMutexPost did not return OS_ERR_NONE, err=%d, line=%d\n", err, __LINE__));
            BDBG_ASSERT(false);
        }
    }
    return event;
}

BERR_Code
BKNI_WaitForGroup(BKNI_EventGroupHandle group, int timeoutMsec, BKNI_EventHandle *events, unsigned max_events, unsigned *nevents)
{
    int ticks;
    BERR_Code result = BERR_SUCCESS;
    OS_ERR err;

    ASSERT_NOT_CRITICAL();
    BDBG_OBJECT_ASSERT(group, BKNI_EventGroup);

    if (max_events<1) {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    if (timeoutMsec!=0 && timeoutMsec!=BKNI_INFINITE) {
        if (timeoutMsec<0) {
            /* If your app is written to allow negative values to this function, then it's highly likely you would allow -1, which would
            result in an infinite hang. We recommend that you only pass positive values to this function unless you definitely mean BKNI_INFINITE. */
            BDBG_WRN(("BKNI_WaitForGroup given negative timeout. Possible infinite hang if timeout happens to be -1 (BKNI_INFINITE)."));
        }
        if (timeoutMsec<10) {
            timeoutMsec=10; /* wait at least 10 msec */
        }
/*
        rc = BKNI_P_SetTargetTime(&target, timeoutMsec);
        if (rc) {
            return BERR_TRACE(BERR_OS_ERROR);
        }
*/
    }

    OSMutexPend(&group->lock,
                0,
                OS_OPT_PEND_BLOCKING,
                NULL,
                &err);
    if (err != OS_ERR_NONE) {
        BDBG_ERR(("OSMutexPend did not return OS_ERR_NONE %d, line=%d\n", err, __LINE__));
        BDBG_ASSERT(false);
    }
    for(;;) {
        *nevents = group_get_events(group, events, max_events);
        if (*nevents) {
            goto done;
        }
        if (timeoutMsec == 0) {
            result = BERR_TIMEOUT;
            goto done;
        }
        else if (timeoutMsec == BKNI_INFINITE) {
            OSMutexPost(&group->lock,
                        OS_OPT_POST_NONE,
                        &err);
            if (err != OS_ERR_NONE) {
                BDBG_ERR(("OSMutexPost did not return OS_ERR_NONE, group=%p, err=%d, line=%d\n", group, err, __LINE__)); while(1);
            }
            OSSemPend(&group->cond,
                      0,
                      OS_OPT_PEND_BLOCKING,
                      NULL,
                      &err);
            OSMutexPend(&group->lock,
                        0,
                        OS_OPT_PEND_BLOCKING,
                        NULL,
                        &err);
            if (err != OS_ERR_NONE) {
                BDBG_ERR(("OSMutexPend did not return OS_ERR_NONE %d, line=%d\n", err, __LINE__));
                BDBG_ASSERT(false);
            }
        }
        else {
            OSMutexPost(&group->lock,
                        OS_OPT_POST_NONE,
                        &err);
            if (err != OS_ERR_NONE) {
                BDBG_ERR(("OSMutexPost did not return OS_ERR_NONE, group=%p, err=%d, line=%d\n", group, err, __LINE__)); while(1);
            }
            ticks = MS_TO_TICKS(timeoutMsec);
            if (ticks <= 0) {
                ticks = 1;
            }
            OSSemPend(&group->cond,
                      ticks,
                      OS_OPT_PEND_BLOCKING,
                      NULL,
                      &err);
            if ((err != OS_ERR_NONE) && (err != OS_ERR_TIMEOUT)) {
                BDBG_ERR(("OSSemPend did not return OS_ERR_NONE or OS_ERR_TIMEOUT, group=%p, err=%d, line=%d\n", group, err, __LINE__));
                BDBG_ASSERT(false);
            }
            if (err == OS_ERR_TIMEOUT) {
                result = BERR_TIMEOUT;
            }
            OSMutexPend(&group->lock,
                        0,
                        OS_OPT_PEND_BLOCKING,
                        NULL,
                        &err);
            if (err != OS_ERR_NONE) {
                BDBG_ERR(("OSMutexPend did not return OS_ERR_NONE %d, line=%d\n", err, __LINE__));
                BDBG_ASSERT(false);
            }
            goto done;
        }
/*
        if(rc==EINTR) {
            BDBG_MSG(("BKNI_WaitForGroup(%#x): interrupted", (unsigned)(unsigned long)group));
            continue;
        }
        if (rc!=0) {
            BDBG_ERR(("%s() returned %d",(timeoutMsec == BKNI_INFINITE) ? "pthread_cond_wait":"pthread_cond_timedwait",rc));
            result = BERR_TRACE(BERR_OS_ERROR);
            goto done;
        }
*/
    }

done:
    OSMutexPost(&group->lock,
                OS_OPT_POST_NONE,
                &err);
    if (err != OS_ERR_NONE) {
        BDBG_ERR(("OSMutexPost did not return OS_ERR_NONE, group=%p, err=%d, line=%d\n", group, err, __LINE__)); while(1);
    }
    return result;
}

void *
BKNI_Memset(void *b, int c, size_t len)
{
    return memset(b, c, len);
}

void *
BKNI_Memcpy(void *dst, const void *src, size_t len)
{
    return memcpy(dst, src, len);
}

int
BKNI_Memcmp(const void *b1,  const void *b2, size_t len)
{
    return memcmp(b1, b2, len);
}

void *
BKNI_Memchr(const void *b, int c, size_t len)
{
    while(!len--){
        if(*(unsigned char*)b == (unsigned char)c)
            return (void *)b;
        b = ((unsigned char*)b) + 1;
    }
    /* not found */
    return 0;
}

void *
BKNI_Memmove(void *dst, const void *src, size_t len)
{
    return memcpy(dst, src, len);
}

void
BKNI_AssertIsrContext_isr(const char *filename, unsigned lineno)
{
    if ( !CHECK_CRITICAL() ) {
        BDBG_P_AssertFailed("Not in critical section", filename, lineno);
    }
}

void BKNI_Uninit(void)
{
    if (--g_refcnt == 0) {
        BKNI_P_MutexTrackingState_Uninit();
        BKNI_P_TrackAlloc_Uninit();
    }
    return;
}
