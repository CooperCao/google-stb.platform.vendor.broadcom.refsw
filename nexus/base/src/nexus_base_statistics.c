/***************************************************************************
*  Copyright (C) 2012-2017 2xxx Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#include "blst_aa_tree.h"
#include "blst_queue.h"

BDBG_MODULE(nexus_base_statistics);
BDBG_FILE_MODULE(nexus_statistics_callback);
BDBG_FILE_MODULE(nexus_statistics_module);
BDBG_FILE_MODULE(nexus_statistics_scheduler);
BDBG_FILE_MODULE(nexus_statistics_api);

/* aggregate statistics use more memory and more CPU */
#define NEXUS_P_BASE_ENABLE_CALLBACK_AGGREGATE   0
#define NEXUS_P_BASE_ENABLE_API_AGGREGATE   0
/* requires synchronization on each event */
#define NEXUS_P_BASE_ENABLE_SCHEDULER_STATS     0
/* number of 'top' results printed in the aggregate statistics */
#define NEXUS_P_BASE_STATS_TOP_COUNT    8
/* limit accumulation/printing for events that takes less  then  number of milliseconds */
#define NEXUS_P_BASE_STATS_TOP_THRESHOLD 2

#define NEXUS_P_BASE_STATS_STACKDEPTH_WARNING  (NEXUS_P_BASE_STACKSIZE_MIN/2)
#define NEXUS_P_BASE_STATS_STACKDEPTH_THRESHOLD (2*NEXUS_P_BASE_STACKCHECK_ATOM)

#define NEXUS_P_BASE_STATS_CALLBACK_TIME_NODES   16
#define NEXUS_P_BASE_STATS_CALLBACK_WARNING_THRESHOLD 100
#define NEXUS_P_BASE_STATS_CALLBACK_AGGREGATE_NODES 512

#define NEXUS_P_BASE_STATS_CALLBACK_STACK_NODES   16

#define NEXUS_P_BASE_STATS_MODULE_WARNING_THRESHOLD 100
#define NEXUS_P_BASE_STATS_MODULE_TIME_NODES   16

#define NEXUS_P_BASE_STATS_API_WARNING_THRESHOLD 100
#define NEXUS_P_BASE_STATS_API_TIME_NODES   16
#define NEXUS_P_BASE_STATS_API_STACK_NODES 16
#define NEXUS_P_BASE_STATS_API_AGGREGATE_NODES 512

#if defined(NEXUS_Time_DiffMicroseconds)
#define NEXUS_P_BASE_STATS_TIME_UNITS  "usec"
#define NEXUS_P_BASE_STATS_MS_TO_TIME_UNITS(x)  (x)*1000
#else
#define NEXUS_P_BASE_STATS_TIME_UNITS  "msec"
#define NEXUS_P_BASE_STATS_MS_TO_TIME_UNITS(x)  (x)
#endif
typedef struct NEXUS_P_Base_Stats_Scheduler {
    unsigned count;
    long totalTime;
    long maxTime;
} NEXUS_P_Base_Stats_Scheduler;

BLST_Q_HEAD(NEXUS_P_Base_Stats_TimeList, NEXUS_P_Base_Stats_TimeNode);
typedef struct NEXUS_P_Base_Stats_TimeList NEXUS_P_Base_Stats_TimeList;
typedef struct NEXUS_P_Base_Stats_TimeKey {
    long time;
} NEXUS_P_Base_Stats_TimeKey;

typedef struct NEXUS_P_Base_Stats_TimeNode {
    BLST_Q_ENTRY(NEXUS_P_Base_Stats_TimeNode) link;
    NEXUS_P_Base_Stats_TimeKey key;
} NEXUS_P_Base_Stats_TimeNode;

static void NEXUS_P_Base_Stats_Time_ReplaceSorted(NEXUS_P_Base_Stats_TimeList *list, NEXUS_P_Base_Stats_TimeNode *node)
{
    NEXUS_P_Base_Stats_TimeNode *n;
    long time;

    BDBG_ASSERT(node == BLST_Q_FIRST(list));
    BLST_Q_REMOVE_HEAD(list, link);
    time = node->key.time;
    for(n=BLST_Q_LAST(list);;n=BLST_Q_PREV(n,link)) {
        if(n==NULL) {
            BLST_Q_INSERT_HEAD(list, node, link);
            break;
        }
        if(time>=n->key.time) {
            BLST_Q_INSERT_AFTER(list, n, node, link);
            break;
        }
    }
    return;
}

BLST_Q_HEAD(NEXUS_P_Base_Stats_StackList, NEXUS_P_Base_Stats_StackNode);
typedef struct NEXUS_P_Base_Stats_StackList NEXUS_P_Base_Stats_StackList;
typedef struct NEXUS_P_Base_Stats_StackKey {
    unsigned stack;
} NEXUS_P_Base_Stats_StackKey;

typedef struct NEXUS_P_Base_Stats_StackNode {
    BLST_Q_ENTRY(NEXUS_P_Base_Stats_StackNode) link;
    NEXUS_P_Base_Stats_StackKey key;
} NEXUS_P_Base_Stats_StackNode;

#if NEXUS_P_STACKDEPTH_STATS
static void NEXUS_P_Base_Stats_Stack_ReplaceSorted(NEXUS_P_Base_Stats_StackList *list, NEXUS_P_Base_Stats_StackNode *node)
{
    NEXUS_P_Base_Stats_StackNode *n;
    unsigned stack = node->key.stack;

    BDBG_ASSERT(node == BLST_Q_FIRST(list));
    BLST_Q_REMOVE_HEAD(list, link);
    for(n=BLST_Q_LAST(list);;n=BLST_Q_PREV(n,link)) {
        if(n==NULL) {
            BLST_Q_INSERT_HEAD(list, node, link);
            break;
        }
        if(stack>=n->key.stack) {
            BLST_Q_INSERT_AFTER(list, n, node, link);
            break;
        }
    }
    return;
}
#endif /* NEXUS_P_STACKDEPTH_STATS */

typedef struct NEXUS_P_Base_Stats_CallbackTimeNode {
    NEXUS_P_Base_Stats_TimeNode timeNode; /* must be first */
    const char *kind;
    const char *file;
    NEXUS_ModulePriority priority;
    unsigned lineNo;
    void *callback;
    void *destination;
} NEXUS_P_Base_Stats_CallbackTimeNode;

typedef struct NEXUS_P_Base_Stats_CallbackStackNode {
    NEXUS_P_Base_Stats_StackNode stackNode; /* must be first */
    const char *kind;
    const char *file;
    unsigned lineNo;
    void *callback;
    void *destination;
} NEXUS_P_Base_Stats_CallbackStackNode;

typedef struct NEXUS_P_Base_Stats_ModuleTimeNode {
    NEXUS_P_Base_Stats_TimeNode timeNode; /* must be first */
    const char *moduleName;
    NEXUS_ModulePriority modulePriority;
    const char *file;
    unsigned lineNo;
} NEXUS_P_Base_Stats_ModuleTimeNode;

typedef struct NEXUS_P_Base_Stats_ApiTimeNode {
    NEXUS_P_Base_Stats_TimeNode timeNode; /* must be first */
    const char *apiName;
    const char *moduleName;
    NEXUS_ModulePriority modulePriority;
} NEXUS_P_Base_Stats_ApiTimeNode;

typedef struct NEXUS_P_Base_Stats_ApiStackNode {
    NEXUS_P_Base_Stats_StackNode stackNode; /* must be first */
    const char *apiName;
    const char *moduleName;
} NEXUS_P_Base_Stats_ApiStackNode;

#if NEXUS_P_BASE_ENABLE_CALLBACK_AGGREGATE  || NEXUS_P_BASE_ENABLE_API_AGGREGATE 
typedef struct NEXUS_P_Base_Stats_AggregateKey {
    const void *object; /* should be unique */
    const char *file;
    unsigned lineNo;
    unsigned count;
    long totalTime;
    long maxTime;
} NEXUS_P_Base_Stats_AggregateKey;

BLST_AA_TREE_HEAD(NEXUS_P_Base_Stats_AggregateInstanceTree, NEXUS_P_Base_Stats_AggregateNode);
BLST_AA_TREE_HEAD(NEXUS_P_Base_Stats_AggregateCountTree, NEXUS_P_Base_Stats_AggregateNode);
BLST_AA_TREE_HEAD(NEXUS_P_Base_Stats_AggregateTotalTimeTree, NEXUS_P_Base_Stats_AggregateNode);
BLST_AA_TREE_HEAD(NEXUS_P_Base_Stats_AggregateMaxTimeTree, NEXUS_P_Base_Stats_AggregateNode);

typedef struct NEXUS_P_Base_Stats_AggregateNode {
    NEXUS_P_Base_Stats_AggregateKey key;
    BLST_AA_TREE_ENTRY(NEXUS_P_Base_Stats_AggregateInstanceTree) instanceNode;
    BLST_AA_TREE_ENTRY(NEXUS_P_Base_Stats_AggregateCountTree) countNode;
    BLST_AA_TREE_ENTRY(NEXUS_P_Base_Stats_AggregateTotalTimeTree) totalTimeNode;
    BLST_AA_TREE_ENTRY(NEXUS_P_Base_Stats_AggregateMaxTimeTree) maxTimeNode;
} NEXUS_P_Base_Stats_AggregateNode;


static int NEXUS_P_Base_Stats_AggregateNode_InstanceCompare(const NEXUS_P_Base_Stats_AggregateNode *node, const NEXUS_P_Base_Stats_AggregateKey *key)
{
    if(node->key.object != key->object) { return (char *)node->key.object > (char *)key->object ? 1 : -1;}
    else if(node->key.file != key->file) { return node->key.file > (char *)key->file? 1 : -1;}
    else return (int)node->key.lineNo - (int)key->lineNo;
}

BLST_AA_TREE_GENERATE_INSERT(NEXUS_P_Base_Stats_AggregateInstanceTree, const NEXUS_P_Base_Stats_AggregateKey *, NEXUS_P_Base_Stats_AggregateNode, instanceNode, NEXUS_P_Base_Stats_AggregateNode_InstanceCompare)
BLST_AA_TREE_GENERATE_FIND(NEXUS_P_Base_Stats_AggregateInstanceTree, const NEXUS_P_Base_Stats_AggregateKey *, NEXUS_P_Base_Stats_AggregateNode, instanceNode, NEXUS_P_Base_Stats_AggregateNode_InstanceCompare)
BLST_AA_TREE_GENERATE_LAST(NEXUS_P_Base_Stats_AggregateInstanceTree, NEXUS_P_Base_Stats_AggregateNode, instanceNode)
BLST_AA_TREE_GENERATE_REMOVE(NEXUS_P_Base_Stats_AggregateInstanceTree, NEXUS_P_Base_Stats_AggregateNode, instanceNode)

static int NEXUS_P_Base_Stats_AggregateNode_CountCompare(const NEXUS_P_Base_Stats_AggregateNode *node, const NEXUS_P_Base_Stats_AggregateKey *key)
{
    if(node->key.count != key->count)  {return node->key.count > key->count ? 1 : -1;}
    else return NEXUS_P_Base_Stats_AggregateNode_InstanceCompare(node, key);
}

BLST_AA_TREE_GENERATE_INSERT(NEXUS_P_Base_Stats_AggregateCountTree, const NEXUS_P_Base_Stats_AggregateKey *, NEXUS_P_Base_Stats_AggregateNode, countNode, NEXUS_P_Base_Stats_AggregateNode_CountCompare)
BLST_AA_TREE_GENERATE_REMOVE(NEXUS_P_Base_Stats_AggregateCountTree, NEXUS_P_Base_Stats_AggregateNode, countNode)
BLST_AA_TREE_GENERATE_FIRST(NEXUS_P_Base_Stats_AggregateCountTree, NEXUS_P_Base_Stats_AggregateNode, countNode)
BLST_AA_TREE_GENERATE_LAST(NEXUS_P_Base_Stats_AggregateCountTree, NEXUS_P_Base_Stats_AggregateNode, countNode)
BLST_AA_TREE_GENERATE_NEXT(NEXUS_P_Base_Stats_AggregateCountTree, NEXUS_P_Base_Stats_AggregateNode, countNode)

static int NEXUS_P_Base_Stats_AggregateNode_TotalTimeCompare(const NEXUS_P_Base_Stats_AggregateNode *node, const NEXUS_P_Base_Stats_AggregateKey *key)
{
    if(node->key.totalTime != key->totalTime)  {return node->key.totalTime > key->totalTime ? 1 : -1;}
    else return NEXUS_P_Base_Stats_AggregateNode_InstanceCompare(node, key);
}

BLST_AA_TREE_GENERATE_INSERT(NEXUS_P_Base_Stats_AggregateTotalTimeTree, const NEXUS_P_Base_Stats_AggregateKey *, NEXUS_P_Base_Stats_AggregateNode, totalTimeNode, NEXUS_P_Base_Stats_AggregateNode_TotalTimeCompare)
BLST_AA_TREE_GENERATE_REMOVE(NEXUS_P_Base_Stats_AggregateTotalTimeTree, NEXUS_P_Base_Stats_AggregateNode, totalTimeNode)
BLST_AA_TREE_GENERATE_FIRST(NEXUS_P_Base_Stats_AggregateTotalTimeTree, NEXUS_P_Base_Stats_AggregateNode, totalTimeNode)
BLST_AA_TREE_GENERATE_NEXT(NEXUS_P_Base_Stats_AggregateTotalTimeTree, NEXUS_P_Base_Stats_AggregateNode, totalTimeNode)

static int NEXUS_P_Base_Stats_AggregateNode_MaxTimeCompare(const NEXUS_P_Base_Stats_AggregateNode *node, const NEXUS_P_Base_Stats_AggregateKey *key)
{
    if(node->key.maxTime != key->maxTime)  {return node->key.maxTime > key->maxTime ? 1 : -1;}
    else return NEXUS_P_Base_Stats_AggregateNode_InstanceCompare(node, key);
}

BLST_AA_TREE_GENERATE_INSERT(NEXUS_P_Base_Stats_AggregateMaxTimeTree, const NEXUS_P_Base_Stats_AggregateKey *, NEXUS_P_Base_Stats_AggregateNode, maxTimeNode, NEXUS_P_Base_Stats_AggregateNode_MaxTimeCompare)
BLST_AA_TREE_GENERATE_REMOVE(NEXUS_P_Base_Stats_AggregateMaxTimeTree, NEXUS_P_Base_Stats_AggregateNode, maxTimeNode)
BLST_AA_TREE_GENERATE_FIRST(NEXUS_P_Base_Stats_AggregateMaxTimeTree, NEXUS_P_Base_Stats_AggregateNode, maxTimeNode)
BLST_AA_TREE_GENERATE_NEXT(NEXUS_P_Base_Stats_AggregateMaxTimeTree, NEXUS_P_Base_Stats_AggregateNode, maxTimeNode)

typedef struct NEXUS_P_Base_StateAggregateTrees {
    struct NEXUS_P_Base_Stats_AggregateInstanceTree instanceTree;
    struct NEXUS_P_Base_Stats_AggregateCountTree countTree;
    struct NEXUS_P_Base_Stats_AggregateTotalTimeTree totalTimeTree;
    struct NEXUS_P_Base_Stats_AggregateMaxTimeTree maxTimeTree;
} NEXUS_P_Base_StateAggregateTrees;


static const NEXUS_P_Base_Stats_AggregateNode *NEXUS_P_Base_Stats_AggregateNode_Replace(NEXUS_P_Base_StateAggregateTrees *aggregate,const NEXUS_P_Base_Stats_AggregateKey *key)
{
    NEXUS_P_Base_Stats_AggregateNode *node = BLST_AA_TREE_FIND(NEXUS_P_Base_Stats_AggregateInstanceTree, &aggregate->instanceTree, key);
    if(node==NULL) {
        node = BLST_AA_TREE_LAST(NEXUS_P_Base_Stats_AggregateInstanceTree, &aggregate->instanceTree);
        if(node->key.object == NULL) { /* found unused entry */
            BLST_AA_TREE_REMOVE(NEXUS_P_Base_Stats_AggregateInstanceTree, &aggregate->instanceTree, node);
        } else { /* find least used entry */
            node = BLST_AA_TREE_LAST(NEXUS_P_Base_Stats_AggregateCountTree, &aggregate->countTree);
            BLST_AA_TREE_REMOVE(NEXUS_P_Base_Stats_AggregateCountTree, &aggregate->countTree, node);
            BLST_AA_TREE_REMOVE(NEXUS_P_Base_Stats_AggregateInstanceTree, &aggregate->instanceTree, node);
            BLST_AA_TREE_REMOVE(NEXUS_P_Base_Stats_AggregateTotalTimeTree, &aggregate->totalTimeTree, node);
            BLST_AA_TREE_REMOVE(NEXUS_P_Base_Stats_AggregateMaxTimeTree, &aggregate->maxTimeTree, node);
        }
        node->key.object = key->object;
        node->key.file = key->file;
        node->key.lineNo = key->lineNo;
        node->key.count = 1;
        node->key.totalTime = key->totalTime;
        node->key.maxTime = key->totalTime;
        BLST_AA_TREE_INSERT(NEXUS_P_Base_Stats_AggregateInstanceTree, &aggregate->instanceTree, &node->key, node);
        BLST_AA_TREE_INSERT(NEXUS_P_Base_Stats_AggregateCountTree, &aggregate->countTree, &node->key, node);
        BLST_AA_TREE_INSERT(NEXUS_P_Base_Stats_AggregateTotalTimeTree, &aggregate->totalTimeTree, &node->key, node);
        BLST_AA_TREE_INSERT(NEXUS_P_Base_Stats_AggregateMaxTimeTree, &aggregate->maxTimeTree, &node->key, node);
        return node; /* return node to be populated by caller */
    } else {
        long time = key->totalTime;
        BLST_AA_TREE_REMOVE(NEXUS_P_Base_Stats_AggregateCountTree, &aggregate->countTree, node);
        node->key.count ++;
        BLST_AA_TREE_INSERT(NEXUS_P_Base_Stats_AggregateCountTree, &aggregate->countTree, &node->key, node);
        if(time>0)  {
            BLST_AA_TREE_REMOVE(NEXUS_P_Base_Stats_AggregateTotalTimeTree, &aggregate->totalTimeTree, node);
            node->key.totalTime += time;
            BLST_AA_TREE_INSERT(NEXUS_P_Base_Stats_AggregateTotalTimeTree, &aggregate->totalTimeTree, &node->key, node);
            if(time>node->key.maxTime) {
                BLST_AA_TREE_REMOVE(NEXUS_P_Base_Stats_AggregateMaxTimeTree, &aggregate->maxTimeTree, node);
                node->key.maxTime = time;
                BLST_AA_TREE_INSERT(NEXUS_P_Base_Stats_AggregateMaxTimeTree, &aggregate->maxTimeTree, &node->key, node);
            }
        }
    }
    return NULL;
}

static void NEXUS_P_Base_Stats_AggregateClear(NEXUS_P_Base_StateAggregateTrees *aggregate, NEXUS_P_Base_Stats_AggregateNode *nodeArray, size_t nodeSize, unsigned nodeCount)
{
    unsigned i;

    BLST_AA_TREE_INIT(NEXUS_P_Base_Stats_AggregateInstanceTree, &aggregate->instanceTree);
    BLST_AA_TREE_INIT(NEXUS_P_Base_Stats_AggregateCountTree, &aggregate->countTree);
    BLST_AA_TREE_INIT(NEXUS_P_Base_Stats_AggregateTotalTimeTree, &aggregate->totalTimeTree);
    BLST_AA_TREE_INIT(NEXUS_P_Base_Stats_AggregateMaxTimeTree, &aggregate->maxTimeTree);
    for(i=0;i<nodeCount;i++) {
        NEXUS_P_Base_Stats_AggregateNode *node = (NEXUS_P_Base_Stats_AggregateNode *)((uint8_t *)nodeArray + i*nodeSize);
        node->key.object = NULL;
        node->key.file = NULL;
        node->key.lineNo = nodeCount - i; /* inverse order  so we would pick entries first with smallest index */
        node->key.count = 0;
        node->key.totalTime = 0;
        node->key.maxTime = 0;
        BLST_AA_TREE_INSERT(NEXUS_P_Base_Stats_AggregateInstanceTree, &aggregate->instanceTree, &node->key, node);
    }
    return;
}
typedef struct NEXUS_P_Base_Stats_CallbackAggregateNode {
    NEXUS_P_Base_Stats_AggregateNode aggregateNode; /* must be first */
    const char *kind;
    NEXUS_ModulePriority priority;
    void *destination;
} NEXUS_P_Base_Stats_CallbackAggregateNode;

typedef struct NEXUS_P_Base_Stats_ApiAggregateNode {
    NEXUS_P_Base_Stats_AggregateNode aggregateNode; /* must be first */
    const char *moduleName;
} NEXUS_P_Base_Stats_ApiAggregateNode;

#endif /* NEXUS_P_BASE_ENABLE_CALLBACK_AGGREGATE  || NEXUS_P_BASE_ENABLE_API_AGGREGATE */



static struct NEXUS_P_Base_Stats_State {
    BKNI_MutexHandle lock; /* global lock for all statistics */
    NEXUS_P_Base_Stats_TimeList callbackTimeList;
    NEXUS_P_Base_Stats_TimeList moduleTimeList;
    NEXUS_P_Base_Stats_TimeList apiTimeList;
#if NEXUS_P_STACKDEPTH_STATS
    NEXUS_P_Base_Stats_StackList apiStackList;
    NEXUS_P_Base_Stats_StackList callbackStackList;
#endif
#if NEXUS_P_BASE_ENABLE_SCHEDULER_STATS  
    NEXUS_P_Base_Stats_Scheduler schedulers[NEXUS_ModulePriority_eMax];
#endif
#if NEXUS_P_BASE_ENABLE_CALLBACK_AGGREGATE   
    NEXUS_P_Base_StateAggregateTrees callbackAggregate;
#endif
#if NEXUS_P_BASE_ENABLE_API_AGGREGATE
    NEXUS_P_Base_StateAggregateTrees apiAggregate;
#endif
    NEXUS_P_Base_Stats_CallbackTimeNode callbackTimeNodes[NEXUS_P_BASE_STATS_CALLBACK_TIME_NODES];
    NEXUS_P_Base_Stats_ModuleTimeNode moduleTimeNodes[NEXUS_P_BASE_STATS_MODULE_TIME_NODES];
    NEXUS_P_Base_Stats_ApiTimeNode apiTimeNodes[NEXUS_P_BASE_STATS_API_TIME_NODES];
#if NEXUS_P_BASE_ENABLE_CALLBACK_AGGREGATE   
    NEXUS_P_Base_Stats_CallbackAggregateNode callbackAggregateNodes[NEXUS_P_BASE_STATS_CALLBACK_AGGREGATE_NODES];
#endif
#if NEXUS_P_BASE_ENABLE_API_AGGREGATE
    NEXUS_P_Base_Stats_ApiAggregateNode apiAggregateNodes[NEXUS_P_BASE_STATS_API_AGGREGATE_NODES];
#endif
#if NEXUS_P_STACKDEPTH_STATS
    NEXUS_P_Base_Stats_ApiStackNode apiStackNodes[NEXUS_P_BASE_STATS_API_STACK_NODES];
    NEXUS_P_Base_Stats_CallbackStackNode callbackStackNodes[NEXUS_P_BASE_STATS_CALLBACK_STACK_NODES];
#endif
} g_NEXUS_P_Base_Stats_State;


static void NEXUS_P_Base_Stats_CallbackClear(void)
{
    unsigned i;
    struct NEXUS_P_Base_Stats_State *state = &g_NEXUS_P_Base_Stats_State;

    BLST_Q_INIT(&state->callbackTimeList);
    for(i=0;i<NEXUS_P_BASE_STATS_CALLBACK_TIME_NODES;i++) {
        NEXUS_P_Base_Stats_CallbackTimeNode *node = &state->callbackTimeNodes[i];
        node->callback = NULL;
        node->timeNode.key.time = 0;
        BLST_Q_INSERT_TAIL(&state->callbackTimeList, &node->timeNode, link);
    }
#if NEXUS_P_STACKDEPTH_STATS
    BLST_Q_INIT(&state->callbackStackList);
    for(i=0;i<NEXUS_P_BASE_STATS_API_STACK_NODES;i++) {
        NEXUS_P_Base_Stats_CallbackStackNode *node = &state->callbackStackNodes[i];
        node->callback = NULL;
        node->stackNode.key.stack = 0;
        BLST_Q_INSERT_TAIL(&state->callbackStackList, &node->stackNode, link);
    }
#endif
    return;
}

static void NEXUS_P_Base_Stats_ModuleTimeClear(void)
{
    unsigned i;
    struct NEXUS_P_Base_Stats_State *state = &g_NEXUS_P_Base_Stats_State;

    BLST_Q_INIT(&state->moduleTimeList);
    for(i=0;i<NEXUS_P_BASE_STATS_API_TIME_NODES;i++) {
        NEXUS_P_Base_Stats_ModuleTimeNode *node = &state->moduleTimeNodes[i];
        node->moduleName = NULL;
        node->timeNode.key.time = 0;
        BLST_Q_INSERT_TAIL(&state->moduleTimeList, &node->timeNode, link);
    }
    return;
}

static void NEXUS_P_Base_Stats_ApiClear(void)
{
    unsigned i;
    struct NEXUS_P_Base_Stats_State *state = &g_NEXUS_P_Base_Stats_State;

    BLST_Q_INIT(&state->apiTimeList);
    for(i=0;i<NEXUS_P_BASE_STATS_API_TIME_NODES;i++) {
        NEXUS_P_Base_Stats_ApiTimeNode *node = &state->apiTimeNodes[i];
        node->moduleName = NULL;
        node->timeNode.key.time = 0;
        BLST_Q_INSERT_TAIL(&state->apiTimeList, &node->timeNode, link);
    }
#if NEXUS_P_STACKDEPTH_STATS
    BLST_Q_INIT(&state->apiStackList);
    for(i=0;i<NEXUS_P_BASE_STATS_API_STACK_NODES;i++) {
        NEXUS_P_Base_Stats_ApiStackNode *node = &state->apiStackNodes[i];
        node->moduleName = NULL;
        node->stackNode.key.stack = 0;
        BLST_Q_INSERT_TAIL(&state->apiStackList, &node->stackNode, link);
    }
#endif
    return;
}


static const char *NEXUS_P_Base_Stats_PriorityName(NEXUS_ModulePriority priority)
{
    static const char * const scheduler_names[]={
        "Idle",
        "Low",
        "Default",
        "High",
        "IdleActiveStandby",
        "LowActiveStandby",
        "HighActiveStandby"
    };
    if(priority<sizeof(scheduler_names)/sizeof(*scheduler_names)) {
        return scheduler_names[priority];
    } 
    return "Unknown";
}
#define NEXUS_P_BASE_STATS_REPORT_SEPARATOR(type) ("-------- %s ---------", type)

#if NEXUS_P_BASE_ENABLE_SCHEDULER_STATS  
static void NEXUS_P_Base_Stats_SchedulersClear(void)
{
    unsigned i;
    struct NEXUS_P_Base_Stats_State *state = &g_NEXUS_P_Base_Stats_State;
    for(i=0;i<NEXUS_ModulePriority_eMax;i++) {
        NEXUS_P_Base_Stats_Scheduler *scheduler = &state->schedulers[i];
        scheduler->count = 0;
        scheduler->totalTime = 0;
        scheduler->maxTime = 0;
    }
    return;
}

void NEXUS_P_Base_Stats_SchedulersReport(void)
{
    struct NEXUS_P_Base_Stats_State *state = &g_NEXUS_P_Base_Stats_State;
    unsigned i;
    bool printed[NEXUS_ModulePriority_eMax];
    bool separator = true;

    NEXUS_P_Base_Stats_Scheduler *maxScheduler;
    for(i=0;i<NEXUS_ModulePriority_eMax;i++) {
        printed[i] = false;
    }
    for(;;) {
        unsigned priority;
        for(maxScheduler = NULL, i=0;i<NEXUS_ModulePriority_eMax;i++) {
            NEXUS_P_Base_Stats_Scheduler *scheduler = &state->schedulers[i];
            if(!printed[i] &&  scheduler->count) {
                if(maxScheduler == NULL || maxScheduler->totalTime < scheduler->totalTime) {
                    maxScheduler = scheduler;
                }
            }
        }
        if(maxScheduler==NULL || maxScheduler->totalTime < NEXUS_P_BASE_STATS_MS_TO_TIME_UNITS(NEXUS_P_BASE_STATS_TOP_THRESHOLD)) {
            break;
        }
        priority = maxScheduler - state->schedulers;
        printed[priority] = true;
        if(separator) {
            separator = false;
            BDBG_MODULE_MSG(nexus_statistics_scheduler,NEXUS_P_BASE_STATS_REPORT_SEPARATOR("scheduler"));
        }

        BDBG_MODULE_MSG(nexus_statistics_scheduler,("%s calls %u total %u max %u avg %u " NEXUS_P_BASE_STATS_TIME_UNITS, NEXUS_P_Base_Stats_PriorityName(priority), maxScheduler->count, (unsigned)maxScheduler->totalTime, (unsigned)maxScheduler->maxTime, (unsigned)(maxScheduler->totalTime/maxScheduler->count)));
    }
    return;
}

#else
#define NEXUS_P_Base_Stats_SchedulersClear()
#define NEXUS_P_Base_Stats_SchedulersReport()
#endif


#if NEXUS_P_BASE_ENABLE_CALLBACK_AGGREGATE   
static void NEXUS_P_Base_Stats_CallbackAggregateClear(void)
{
    struct NEXUS_P_Base_Stats_State *state = &g_NEXUS_P_Base_Stats_State;

    NEXUS_P_Base_Stats_AggregateClear(&state->callbackAggregate, (NEXUS_P_Base_Stats_AggregateNode *)state->callbackAggregateNodes, sizeof(state->callbackAggregateNodes[0]),NEXUS_P_BASE_STATS_CALLBACK_AGGREGATE_NODES);
    return;
}

static void NEXUS_P_Base_Stats_CallbackAggregateReport(void)
{
    struct NEXUS_P_Base_Stats_State *state = &g_NEXUS_P_Base_Stats_State;
    unsigned i;
    NEXUS_P_Base_Stats_CallbackAggregateNode *node;

    for(    i=0, node=(NEXUS_P_Base_Stats_CallbackAggregateNode *)BLST_AA_TREE_FIRST(NEXUS_P_Base_Stats_AggregateCountTree, &state->callbackAggregate.countTree);
            node && i<NEXUS_P_BASE_STATS_TOP_COUNT;
            i++, node=(NEXUS_P_Base_Stats_CallbackAggregateNode *)BLST_AA_TREE_NEXT(NEXUS_P_Base_Stats_AggregateCountTree, &state->callbackAggregate.countTree, &node->aggregateNode)) {

        if(0 && node->aggregateNode.key.totalTime < NEXUS_P_BASE_STATS_MS_TO_TIME_UNITS(NEXUS_P_BASE_STATS_TOP_THRESHOLD)) {
            break;
        }
        if(i==0) {
            BDBG_MODULE_MSG(nexus_statistics_callback,NEXUS_P_BASE_STATS_REPORT_SEPARATOR("callback by count"));
        }
        BDBG_MODULE_MSG(nexus_statistics_callback,("%s[%s] callback %p:%p from %s:%u %u times total %u (avg %u) " NEXUS_P_BASE_STATS_TIME_UNITS, node->kind, NEXUS_P_Base_Stats_PriorityName(node->priority), node->aggregateNode.key.object, node->destination, node->aggregateNode.key.file, node->aggregateNode.key.lineNo, node->aggregateNode.key.count, (unsigned)node->aggregateNode.key.totalTime, (unsigned)(node->aggregateNode.key.totalTime/node->aggregateNode.key.count)));
    }
    for(    i=0, node=(NEXUS_P_Base_Stats_CallbackAggregateNode *)BLST_AA_TREE_FIRST(NEXUS_P_Base_Stats_AggregateTotalTimeTree, &state->callbackAggregate.totalTimeTree);
            node && i<NEXUS_P_BASE_STATS_TOP_COUNT;
            i++, node=(NEXUS_P_Base_Stats_CallbackAggregateNode *)BLST_AA_TREE_NEXT(NEXUS_P_Base_Stats_AggregateTotalTimeTree, &state->callbackAggregate.totalTimeTree, &node->aggregateNode)) {
        
        if(node->aggregateNode.key.totalTime < NEXUS_P_BASE_STATS_MS_TO_TIME_UNITS(NEXUS_P_BASE_STATS_TOP_THRESHOLD)) {
            break;
        }
        if(i==0) {
            BDBG_MODULE_MSG(nexus_statistics_callback,NEXUS_P_BASE_STATS_REPORT_SEPARATOR("callback by total time"));
        }
        BDBG_MODULE_MSG(nexus_statistics_callback,("%s[%s] callback %p:%p from %s:%u total %u (avg %u) " NEXUS_P_BASE_STATS_TIME_UNITS, node->kind, NEXUS_P_Base_Stats_PriorityName(node->priority), node->aggregateNode.key.object , node->destination, node->aggregateNode.key.file, node->aggregateNode.key.lineNo, (unsigned)node->aggregateNode.key.totalTime, (unsigned)(node->aggregateNode.key.totalTime/node->aggregateNode.key.count)));
    }
    for(    i=0, node=(NEXUS_P_Base_Stats_CallbackAggregateNode *)BLST_AA_TREE_FIRST(NEXUS_P_Base_Stats_AggregateMaxTimeTree, &state->callbackAggregate.maxTimeTree);
            node && i<NEXUS_P_BASE_STATS_TOP_COUNT;
            i++, node=(NEXUS_P_Base_Stats_CallbackAggregateNode *)BLST_AA_TREE_NEXT(NEXUS_P_Base_Stats_AggregateMaxTimeTree, &state->callbackAggregate.maxTimeTree, &node->aggregateNode)) {
        
        if(node->aggregateNode.key.totalTime < NEXUS_P_BASE_STATS_MS_TO_TIME_UNITS(NEXUS_P_BASE_STATS_TOP_THRESHOLD)) {
            break;
        }
        if(i==0) {
            BDBG_MODULE_MSG(nexus_statistics_callback,NEXUS_P_BASE_STATS_REPORT_SEPARATOR("callback by max time"));
        }
        BDBG_MODULE_MSG(nexus_statistics_callback,("%s[%s] callback %p:%p from %s:%u max %u (avg %u) " NEXUS_P_BASE_STATS_TIME_UNITS, node->kind, NEXUS_P_Base_Stats_PriorityName(node->priority), node->aggregateNode.key.object, node->destination, node->aggregateNode.key.file, node->aggregateNode.key.lineNo, (unsigned)node->aggregateNode.key.maxTime, (unsigned)(node->aggregateNode.key.totalTime/node->aggregateNode.key.count)));
    }
    return;
}

#else /* NEXUS_P_BASE_ENABLE_CALLBACK_AGGREGATE  */
#define NEXUS_P_Base_Stats_CallbackAggregateClear()
#define NEXUS_P_Base_Stats_CallbackAggregateReport()
#endif /* NEXUS_P_BASE_ENABLE_CALLBACK_AGGREGATE  */

#if NEXUS_P_BASE_ENABLE_API_AGGREGATE   
static void NEXUS_P_Base_Stats_ApiAggregateClear(void)
{
    struct NEXUS_P_Base_Stats_State *state = &g_NEXUS_P_Base_Stats_State;

    NEXUS_P_Base_Stats_AggregateClear(&state->apiAggregate, (NEXUS_P_Base_Stats_AggregateNode *)state->apiAggregateNodes, sizeof(state->apiAggregateNodes[0]),NEXUS_P_BASE_STATS_API_AGGREGATE_NODES);
    return;
}

static void NEXUS_P_Base_Stats_ApiAggregateReport(void)
{
    struct NEXUS_P_Base_Stats_State *state = &g_NEXUS_P_Base_Stats_State;
    unsigned i;
    NEXUS_P_Base_Stats_ApiAggregateNode *node;

    for(    i=0, node=(NEXUS_P_Base_Stats_ApiAggregateNode *)BLST_AA_TREE_FIRST(NEXUS_P_Base_Stats_AggregateCountTree, &state->apiAggregate.countTree);
            node && i<NEXUS_P_BASE_STATS_TOP_COUNT;
            i++, node=(NEXUS_P_Base_Stats_ApiAggregateNode *)BLST_AA_TREE_NEXT(NEXUS_P_Base_Stats_AggregateCountTree, &state->apiAggregate.countTree, &node->aggregateNode)) {

        if(0 && node->aggregateNode.key.totalTime < NEXUS_P_BASE_STATS_MS_TO_TIME_UNITS(NEXUS_P_BASE_STATS_TOP_THRESHOLD)) {
            break;
        }
        if(i==0) {
            BDBG_MODULE_MSG(nexus_statistics_api,NEXUS_P_BASE_STATS_REPORT_SEPARATOR("API by count"));
        }
        BDBG_MODULE_MSG(nexus_statistics_api,("%s[%s] %u times total %u max %u (avg %u) " NEXUS_P_BASE_STATS_TIME_UNITS, (char *)node->aggregateNode.key.object, node->moduleName, node->aggregateNode.key.count, (unsigned)node->aggregateNode.key.totalTime, (unsigned)node->aggregateNode.key.maxTime, (unsigned)(node->aggregateNode.key.totalTime/node->aggregateNode.key.count)));
    }
    for(    i=0, node=(NEXUS_P_Base_Stats_ApiAggregateNode *)BLST_AA_TREE_FIRST(NEXUS_P_Base_Stats_AggregateTotalTimeTree, &state->apiAggregate.totalTimeTree);
            node && i<NEXUS_P_BASE_STATS_TOP_COUNT;
            i++, node=(NEXUS_P_Base_Stats_ApiAggregateNode *)BLST_AA_TREE_NEXT(NEXUS_P_Base_Stats_AggregateTotalTimeTree, &state->apiAggregate.totalTimeTree, &node->aggregateNode)) {
        
        if(node->aggregateNode.key.totalTime < NEXUS_P_BASE_STATS_MS_TO_TIME_UNITS(NEXUS_P_BASE_STATS_TOP_THRESHOLD)) {
            break;
        }
        if(i==0) {
            BDBG_MODULE_MSG(nexus_statistics_api,NEXUS_P_BASE_STATS_REPORT_SEPARATOR("API by total time"));
        }
        BDBG_MODULE_MSG(nexus_statistics_api,("%s[%s] %u times total %u max %u (avg %u) " NEXUS_P_BASE_STATS_TIME_UNITS, (char *)node->aggregateNode.key.object, node->moduleName, node->aggregateNode.key.count, (unsigned)node->aggregateNode.key.totalTime, (unsigned)node->aggregateNode.key.maxTime, (unsigned)(node->aggregateNode.key.totalTime/node->aggregateNode.key.count)));
    }
    for(    i=0, node=(NEXUS_P_Base_Stats_ApiAggregateNode *)BLST_AA_TREE_FIRST(NEXUS_P_Base_Stats_AggregateMaxTimeTree, &state->apiAggregate.maxTimeTree);
            node && i<NEXUS_P_BASE_STATS_TOP_COUNT;
            i++, node=(NEXUS_P_Base_Stats_ApiAggregateNode *)BLST_AA_TREE_NEXT(NEXUS_P_Base_Stats_AggregateMaxTimeTree, &state->apiAggregate.maxTimeTree, &node->aggregateNode)) {
        
        if(node->aggregateNode.key.totalTime < NEXUS_P_BASE_STATS_MS_TO_TIME_UNITS(NEXUS_P_BASE_STATS_TOP_THRESHOLD)) {
            break;
        }
        if(i==0) {
            BDBG_MODULE_MSG(nexus_statistics_api,NEXUS_P_BASE_STATS_REPORT_SEPARATOR("API by max time"));
        }
        BDBG_MODULE_MSG(nexus_statistics_api,("%s[%s] %u times total %u max %u (avg %u) " NEXUS_P_BASE_STATS_TIME_UNITS, (char *)node->aggregateNode.key.object, node->moduleName, node->aggregateNode.key.count, (unsigned)node->aggregateNode.key.totalTime, (unsigned)node->aggregateNode.key.maxTime, (unsigned)(node->aggregateNode.key.totalTime/node->aggregateNode.key.count)));

    }
    return;
}

#else /* NEXUS_P_BASE_ENABLE_API_AGGREGATE  */
#define NEXUS_P_Base_Stats_ApiAggregateClear()
#define NEXUS_P_Base_Stats_ApiAggregateReport()
#endif /* NEXUS_P_BASE_ENABLE_API_AGGREGATE  */

NEXUS_Error NEXUS_P_Base_Stats_Init(void)
{
    NEXUS_Error rc;
    struct NEXUS_P_Base_Stats_State *state = &g_NEXUS_P_Base_Stats_State;

    rc = BKNI_CreateMutex(&state->lock);
    if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc);goto err_lock;}
    NEXUS_P_Base_Stats_CallbackClear();
    NEXUS_P_Base_Stats_ModuleTimeClear();
    NEXUS_P_Base_Stats_CallbackAggregateClear();
    NEXUS_P_Base_Stats_ApiAggregateClear();
    NEXUS_P_Base_Stats_SchedulersClear();
    NEXUS_P_Base_Stats_ApiClear();
    return NEXUS_SUCCESS;

err_lock:
    return rc;
}

void NEXUS_P_Base_Stats_Uninit(void)
{
    struct NEXUS_P_Base_Stats_State *state = &g_NEXUS_P_Base_Stats_State;

    BKNI_DestroyMutex(state->lock);
    state->lock = NULL;
    return;
}

void NEXUS_P_Base_Stats_GetCookie(NEXUS_P_Base_Stats_Cookie *cookie)
{
    NEXUS_Time_Get(&cookie->timeStart);
#if NEXUS_P_STACKDEPTH_STATS
    NEXUS_P_Base_StackCheck_MakeFrame(&cookie->stackFrame, cookie);
#endif
}

static void NEXUS_P_Base_Stats_CallbackEventProcess(long time, NEXUS_ModulePriority priority, const char *kind, void *callback, void *destination, const char *file, unsigned lineNo)
{
    struct NEXUS_P_Base_Stats_State *state = &g_NEXUS_P_Base_Stats_State;
    if(time>=NEXUS_P_BASE_STATS_MS_TO_TIME_UNITS(NEXUS_P_BASE_STATS_CALLBACK_WARNING_THRESHOLD)) {
        BDBG_MODULE_WRN(nexus_statistics_callback, ("%s callback %p:%p from %s:%u %ld " NEXUS_P_BASE_STATS_TIME_UNITS, kind, callback, destination, file, lineNo, time));
    }
    if(time>=NEXUS_P_BASE_STATS_MS_TO_TIME_UNITS(NEXUS_P_BASE_STATS_TOP_THRESHOLD)) {
        NEXUS_P_Base_Stats_CallbackTimeNode *node;
        node = (NEXUS_P_Base_Stats_CallbackTimeNode *)BLST_Q_FIRST(&state->callbackTimeList);
        if(time>node->timeNode.key.time) {
            node->timeNode.key.time = time;
            node->callback = callback;
            node->destination = destination;
            node->lineNo = lineNo;
            node->file = file;
            node->kind = kind;
            node->priority = priority;
            NEXUS_P_Base_Stats_Time_ReplaceSorted(&state->callbackTimeList, &node->timeNode);
        }
    }
#if NEXUS_P_BASE_ENABLE_SCHEDULER_STATS
    {
        NEXUS_P_Base_Stats_Scheduler *scheduler = &state->schedulers[priority];
        scheduler->count++;
        scheduler->totalTime += time;
        scheduler->maxTime = scheduler->maxTime > time ? scheduler->maxTime : time;
    }
#endif 
#if NEXUS_P_BASE_ENABLE_CALLBACK_AGGREGATE   
    {
        NEXUS_P_Base_Stats_CallbackAggregateNode *node;
        NEXUS_P_Base_Stats_AggregateKey key;
        key.object = callback;
        key.file = file;
        key.lineNo = lineNo;
        key.totalTime = time;
        node = (NEXUS_P_Base_Stats_CallbackAggregateNode *)NEXUS_P_Base_Stats_AggregateNode_Replace(&state->callbackAggregate,&key);
        if(node) {
            node->kind = kind;
            node->destination = destination;
            node->priority = priority;
        }
    }
#endif /* NEXUS_P_BASE_ENABLE_CALLBACK_AGGREGATE  */
}

static long NEXUS_P_Base_Stats_TimeDiff(const NEXUS_Time *timeStart)
{
    NEXUS_Time timeStop;
    long time;
    NEXUS_Time_Get(&timeStop);
#if defined(NEXUS_Time_DiffMicroseconds)
    time = NEXUS_Time_DiffMicroseconds(&timeStop,timeStart);
#else
    time = NEXUS_Time_Diff(&timeStop,timeStart);
#endif
    return time;
}

void NEXUS_P_Base_Stats_CallbackEvent(const NEXUS_P_Base_Stats_Cookie *cookie, NEXUS_ModulePriority priority, const char *kind, void *callback, void *destination, const char *file, unsigned lineNo)
{
    long time;
    unsigned stackUsage = 0;
    struct NEXUS_P_Base_Stats_State *state = &g_NEXUS_P_Base_Stats_State;
#if NEXUS_P_STACKDEPTH_STATS
    stackUsage = NEXUS_P_Base_StackCheck_CheckFrame(&cookie->stackFrame, cookie); 
    if(stackUsage>=NEXUS_P_BASE_STATS_STACKDEPTH_WARNING) {
        BDBG_MODULE_WRN(nexus_statistics_callback, ("%s callback %p:%p from %s:%u used %u bytes on stack", kind, callback, destination, file, lineNo, stackUsage));
    }
#endif
    BSTD_UNUSED(stackUsage);

    time = NEXUS_P_Base_Stats_TimeDiff(&cookie->timeStart);

#if !NEXUS_P_BASE_ENABLE_CALLBACK_AGGREGATE && !NEXUS_P_BASE_ENABLE_SCHEDULER_STATS  
    if(time<NEXUS_P_BASE_STATS_MS_TO_TIME_UNITS(NEXUS_P_BASE_STATS_TOP_THRESHOLD) && stackUsage<NEXUS_P_BASE_STATS_STACKDEPTH_THRESHOLD) { return;} /* if no agregate processing, then entries then 1 msec could be ignored */
#endif
    BKNI_AcquireMutex(state->lock);
    if(time>=NEXUS_P_BASE_STATS_MS_TO_TIME_UNITS(NEXUS_P_BASE_STATS_TOP_THRESHOLD)) {
        NEXUS_P_Base_Stats_CallbackEventProcess(time, priority, kind, callback, destination, file, lineNo);
    }
#if NEXUS_P_STACKDEPTH_STATS
    if(stackUsage>NEXUS_P_BASE_STATS_STACKDEPTH_THRESHOLD) {
        NEXUS_P_Base_Stats_CallbackStackNode *node = (NEXUS_P_Base_Stats_CallbackStackNode *)BLST_Q_FIRST(&state->callbackStackList);
        if(stackUsage>node->stackNode.key.stack) {
            node->stackNode.key.stack = stackUsage;
            node->callback = callback;
            node->destination = destination;
            node->lineNo = lineNo;
            node->file = file;
            node->kind = kind;
            NEXUS_P_Base_Stats_Stack_ReplaceSorted(&state->callbackStackList, &node->stackNode);
        }
    }
#endif
    BKNI_ReleaseMutex(state->lock);
    return;
}

void NEXUS_P_Base_Stats_ModuleEvent(const NEXUS_Time *timeStart,NEXUS_ModuleHandle module, const char *file, unsigned lineNo)
{
    long time = NEXUS_P_Base_Stats_TimeDiff(timeStart);
    struct NEXUS_P_Base_Stats_State *state = &g_NEXUS_P_Base_Stats_State;

    if(time>=NEXUS_P_BASE_STATS_MS_TO_TIME_UNITS(NEXUS_P_BASE_STATS_MODULE_WARNING_THRESHOLD)) {
        BDBG_MODULE_WRN(nexus_statistics_module, ("%s[%s] %s:%u %ld " NEXUS_P_BASE_STATS_TIME_UNITS, module->pModuleName, NEXUS_P_Base_Stats_PriorityName(module->settings.priority), NEXUS_P_PrepareFileName(file), lineNo, time));
    }
    if(time>=NEXUS_P_BASE_STATS_MS_TO_TIME_UNITS(NEXUS_P_BASE_STATS_TOP_THRESHOLD)) {
        NEXUS_P_Base_Stats_ModuleTimeNode *node;
        BKNI_AcquireMutex(state->lock);
        node = (NEXUS_P_Base_Stats_ModuleTimeNode *)BLST_Q_FIRST(&state->moduleTimeList);
        if(time>node->timeNode.key.time) {
            node->timeNode.key.time = time;
            node->moduleName = module->pModuleName;
            node->modulePriority = module->settings.priority;
            node->file = file;
            node->lineNo = lineNo;
            NEXUS_P_Base_Stats_Time_ReplaceSorted(&state->moduleTimeList, &node->timeNode);
        }

        BKNI_ReleaseMutex(state->lock);
    }
    return;
}


void NEXUS_P_Base_Stats_ApiEvent(const NEXUS_P_Base_Stats_Cookie *cookie, const char *api, NEXUS_ModuleHandle module)
{
    long time;
    struct NEXUS_P_Base_Stats_State *state = &g_NEXUS_P_Base_Stats_State;
    unsigned stackUsage = 0;

    BSTD_UNUSED(stackUsage);
    if(state->lock==NULL) { /* for some cases NEXUS_P_Base_Stats_ApiEvent could get called after NEXUS_P_Base_Stats_Uninit */ 
        return;
    }
#if NEXUS_P_STACKDEPTH_STATS
    stackUsage = NEXUS_P_Base_StackCheck_CheckFrame(&cookie->stackFrame, cookie);
    if(stackUsage>=NEXUS_P_BASE_STATS_STACKDEPTH_WARNING) {
        BDBG_MODULE_WRN(nexus_statistics_api, ("%s[%s] used %u bytes on stack " , api, module->pModuleName, stackUsage));
    }
#endif
    time = NEXUS_P_Base_Stats_TimeDiff(&cookie->timeStart);
    if(time>=NEXUS_P_BASE_STATS_MS_TO_TIME_UNITS(NEXUS_P_BASE_STATS_API_WARNING_THRESHOLD)) {
        BDBG_MODULE_WRN(nexus_statistics_api, ("%s[%s:%s] %ld " NEXUS_P_BASE_STATS_TIME_UNITS, api, module->pModuleName, NEXUS_P_Base_Stats_PriorityName(module->settings.priority), time));
    }
#if !NEXUS_P_BASE_ENABLE_API_AGGREGATE
    if(time<NEXUS_P_BASE_STATS_MS_TO_TIME_UNITS(NEXUS_P_BASE_STATS_TOP_THRESHOLD) && stackUsage<=NEXUS_P_BASE_STATS_STACKDEPTH_THRESHOLD) {
        return;
    }
#endif
    BKNI_AcquireMutex(state->lock);
    if(time>=NEXUS_P_BASE_STATS_MS_TO_TIME_UNITS(NEXUS_P_BASE_STATS_TOP_THRESHOLD)) {
        NEXUS_P_Base_Stats_ApiTimeNode *node = (NEXUS_P_Base_Stats_ApiTimeNode *)BLST_Q_FIRST(&state->apiTimeList);
        if(time>node->timeNode.key.time) {
            node->timeNode.key.time = time;
            node->moduleName = module->pModuleName;
            node->modulePriority = module->settings.priority;
            node->apiName = api;
            NEXUS_P_Base_Stats_Time_ReplaceSorted(&state->apiTimeList, &node->timeNode);
        }
    }
#if NEXUS_P_BASE_ENABLE_API_AGGREGATE
    {
        NEXUS_P_Base_Stats_ApiAggregateNode *node;
        NEXUS_P_Base_Stats_AggregateKey key;
        key.object = api;
        key.file = NULL;
        key.lineNo = 0;
        key.totalTime = time;
        node = (NEXUS_P_Base_Stats_ApiAggregateNode *)NEXUS_P_Base_Stats_AggregateNode_Replace(&state->apiAggregate,&key);
        if(node) {
            node->moduleName = module->pModuleName;
        }
    }
#endif 
#if NEXUS_P_STACKDEPTH_STATS
    if(stackUsage>NEXUS_P_BASE_STATS_STACKDEPTH_THRESHOLD) {
        NEXUS_P_Base_Stats_ApiStackNode *node = (NEXUS_P_Base_Stats_ApiStackNode *)BLST_Q_FIRST(&state->apiStackList);
        if(stackUsage>node->stackNode.key.stack) {
            node->stackNode.key.stack = stackUsage;
            node->moduleName = module->pModuleName;
            node->apiName = api;
            NEXUS_P_Base_Stats_Stack_ReplaceSorted(&state->apiStackList, &node->stackNode);
        }
    }
#endif
    BKNI_ReleaseMutex(state->lock);
    return;
}

#if NEXUS_P_CALLBACK_STATS 
static void NEXUS_P_Base_Stats_CallbackTimeReport(void)
{
    struct NEXUS_P_Base_Stats_State *state = &g_NEXUS_P_Base_Stats_State;
    NEXUS_P_Base_Stats_CallbackTimeNode *node;
    bool separator = true;

    for( node=(NEXUS_P_Base_Stats_CallbackTimeNode *)BLST_Q_LAST(&state->callbackTimeList);
         node!=NULL;
         node=(NEXUS_P_Base_Stats_CallbackTimeNode *)BLST_Q_PREV(&node->timeNode, link)) {
        if(node->timeNode.key.time<NEXUS_P_BASE_STATS_MS_TO_TIME_UNITS(NEXUS_P_BASE_STATS_TOP_THRESHOLD))  {
            break;
        }
        if(separator) {
            separator = false;
            BDBG_MODULE_MSG(nexus_statistics_callback,NEXUS_P_BASE_STATS_REPORT_SEPARATOR("Callback time report"));
        }
        BDBG_MODULE_MSG(nexus_statistics_callback,("%s[%s] %p:%p from %s:%u %ld " NEXUS_P_BASE_STATS_TIME_UNITS, node->kind, NEXUS_P_Base_Stats_PriorityName(node->priority), node->callback, node->destination, node->file, node->lineNo, node->timeNode.key.time));
    }
    return;
}
#endif /* NEXUS_P_CALLBACK_STATS */

#if NEXUS_P_MODULE_STATS
static void NEXUS_P_Base_Stats_ModuleTimeReport(void)
{
    struct NEXUS_P_Base_Stats_State *state = &g_NEXUS_P_Base_Stats_State;
    NEXUS_P_Base_Stats_ModuleTimeNode *node;
    bool separator=true;
    for( node=(NEXUS_P_Base_Stats_ModuleTimeNode *)BLST_Q_LAST(&state->moduleTimeList);
         node!=NULL;
         node=(NEXUS_P_Base_Stats_ModuleTimeNode *)BLST_Q_PREV(&node->timeNode,link)) {
        if(node->timeNode.key.time<NEXUS_P_BASE_STATS_MS_TO_TIME_UNITS(NEXUS_P_BASE_STATS_TOP_THRESHOLD)) {
            break;
        }
        if(separator) {
            separator = false;
            BDBG_MODULE_MSG(nexus_statistics_module,NEXUS_P_BASE_STATS_REPORT_SEPARATOR("module"));
        }
        BDBG_MODULE_MSG(nexus_statistics_module,("%s[%s] from %s:%u %ld " NEXUS_P_BASE_STATS_TIME_UNITS, node->moduleName, NEXUS_P_Base_Stats_PriorityName(node->modulePriority), NEXUS_P_PrepareFileName(node->file), node->lineNo, node->timeNode.key.time));
    }
    return;
}
#endif /* NEXUS_P_MODULE_STATS */

#if NEXUS_P_API_STATS
static void NEXUS_P_Base_Stats_ApiTimeReport(void)
{
    struct NEXUS_P_Base_Stats_State *state = &g_NEXUS_P_Base_Stats_State;
    NEXUS_P_Base_Stats_ApiTimeNode *node;
    bool separator=true;
    for( node=(NEXUS_P_Base_Stats_ApiTimeNode *)BLST_Q_LAST(&state->apiTimeList);
         node!=NULL;
         node=(NEXUS_P_Base_Stats_ApiTimeNode *)BLST_Q_PREV(&node->timeNode,link)) {
        if(node->timeNode.key.time<NEXUS_P_BASE_STATS_MS_TO_TIME_UNITS(NEXUS_P_BASE_STATS_TOP_THRESHOLD)) {
            break;
        }
        if(separator) {
            separator = false;
            BDBG_MODULE_MSG(nexus_statistics_api,NEXUS_P_BASE_STATS_REPORT_SEPARATOR("API execution time"));
        }
        BDBG_MODULE_MSG(nexus_statistics_api,("%s[%s:%s] %ld " NEXUS_P_BASE_STATS_TIME_UNITS, node->apiName, node->moduleName, NEXUS_P_Base_Stats_PriorityName(node->modulePriority), node->timeNode.key.time));
    }
    return;
}

#if NEXUS_P_STACKDEPTH_STATS
static void NEXUS_P_Base_Stats_ApiStackReport(void)
{
    struct NEXUS_P_Base_Stats_State *state = &g_NEXUS_P_Base_Stats_State;
    NEXUS_P_Base_Stats_ApiStackNode *node;
    bool separator=true;
    for( node=(NEXUS_P_Base_Stats_ApiStackNode *)BLST_Q_LAST(&state->apiStackList);
         node!=NULL;
         node=(NEXUS_P_Base_Stats_ApiStackNode *)BLST_Q_PREV(&node->stackNode,link)) {
        if(node->stackNode.key.stack<=NEXUS_P_BASE_STATS_STACKDEPTH_THRESHOLD) {
            break;
        }
        if(separator) {
            separator = false;
            BDBG_MODULE_MSG(nexus_statistics_api,NEXUS_P_BASE_STATS_REPORT_SEPARATOR("API stack usage"));
        }
        BDBG_MODULE_MSG(nexus_statistics_api,("%s[%s] %u bytes", node->apiName, node->moduleName,  node->stackNode.key.stack));
    }
    return;
}

static void NEXUS_P_Base_Stats_CallbackStackReport(void)
{
    struct NEXUS_P_Base_Stats_State *state = &g_NEXUS_P_Base_Stats_State;
    NEXUS_P_Base_Stats_CallbackStackNode *node;
    bool separator=true;
    for( node=(NEXUS_P_Base_Stats_CallbackStackNode *)BLST_Q_LAST(&state->callbackStackList);
         node!=NULL;
         node=(NEXUS_P_Base_Stats_CallbackStackNode *)BLST_Q_PREV(&node->stackNode,link)) {
        if(node->stackNode.key.stack<=NEXUS_P_BASE_STATS_STACKDEPTH_THRESHOLD) {
            break;
        }
        if(separator) {
            separator = false;
            BDBG_MODULE_MSG(nexus_statistics_callback,NEXUS_P_BASE_STATS_REPORT_SEPARATOR("Callback stack usage"));
        }
        BDBG_MODULE_MSG(nexus_statistics_callback,("%s %p:%p from %s:%u %u bytes", node->kind, node->callback, node->destination, node->file, node->lineNo, node->stackNode.key.stack));
    }
    return;
}
#else /* NEXUS_P_STACKDEPTH_STATS */
#define NEXUS_P_Base_Stats_ApiStackReport()
#define NEXUS_P_Base_Stats_CallbackStackReport()
#endif
#endif /* NEXUS_P_API_STATS */

void NEXUS_P_Base_Stats_Report(void)
{
    struct NEXUS_P_Base_Stats_State *state = &g_NEXUS_P_Base_Stats_State;
    BKNI_AcquireMutex(state->lock);
#if NEXUS_P_CALLBACK_STATS  
    NEXUS_P_Base_Stats_CallbackTimeReport();
    NEXUS_P_Base_Stats_CallbackAggregateReport();
    NEXUS_P_Base_Stats_CallbackStackReport();
    NEXUS_P_Base_Stats_SchedulersReport();
#endif
#if NEXUS_P_MODULE_STATS
    NEXUS_P_Base_Stats_ModuleTimeReport();
#endif
#if NEXUS_P_API_STATS
    NEXUS_P_Base_Stats_ApiTimeReport();
    NEXUS_P_Base_Stats_ApiAggregateReport();
    NEXUS_P_Base_Stats_ApiStackReport();
#endif

#if NEXUS_P_CALLBACK_STATS  
    NEXUS_P_Base_Stats_CallbackAggregateClear();
    NEXUS_P_Base_Stats_CallbackClear();
    NEXUS_P_Base_Stats_SchedulersClear();
#endif
#if NEXUS_P_MODULE_STATS
    NEXUS_P_Base_Stats_ModuleTimeClear();
#endif
#if NEXUS_P_API_STATS
    NEXUS_P_Base_Stats_ApiAggregateClear();
    NEXUS_P_Base_Stats_ApiClear();
#endif
    BKNI_ReleaseMutex(state->lock);
    return;
}


void NEXUS_P_Base_Statistics_StackAggregate_Update(NEXUS_P_Base_Statistics_StackAggregate *aggregate, const NEXUS_P_Base_StackCheck_Frame *frame)
{
    unsigned stackDepth;
    stackDepth = NEXUS_P_Base_StackCheck_CheckFrame(frame, frame);
    aggregate->count++;
    aggregate->total += stackDepth;
    aggregate->max = stackDepth > aggregate->max ? stackDepth : aggregate->max;
    return;
}

