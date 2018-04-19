/******************************************************************************
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
 ******************************************************************************/
#include "nexus_transport_module.h"
#include "nexus_base.h"
#include "blst_squeue.h"
#include "nexus_client_resources.h"
#include "priv/nexus_core_security.h"

BDBG_MODULE(nexus_xpt_dma);
#define BDBG_MSG_TRACE(x) /* BDBG_MSG(x) */

typedef struct NEXUS_P_DmaPidChannel {
    BLST_S_ENTRY(NEXUS_P_DmaPidChannel) link;
    NEXUS_P_HwPidChannel *pid_channel;
} NEXUS_P_DmaPidChannel;

struct NEXUS_DmaJobCallback {
    void (*function)(void*);
    void *context;
};

typedef struct NEXUS_DmaJob
{
    NEXUS_OBJECT(NEXUS_DmaJob);
    BLST_S_ENTRY(NEXUS_DmaJob) jobNode;
    BLST_SQ_ENTRY(NEXUS_DmaJob) activeNode;
    BLST_S_HEAD(NEXUS_P_DmaPidList, NEXUS_P_DmaPidChannel) pid_list;

    BXPT_Dma_ContextHandle ctx;
    NEXUS_DmaHandle parent;
    NEXUS_TaskCallbackHandle completionCallback;
    BKNI_EventHandle completionEvent;
    NEXUS_EventCallbackHandle completionEventHandler;
    struct {
        void *srcAddrPtr, *dstAddrPtr;
    } *memory;

    enum {
        NEXUS_DmaJob_P_StateIdle,
        NEXUS_DmaJob_P_StateQueued /* job is queued in activeNode */
    } state;

    struct NEXUS_DmaJobCallback cryptoCallback;
    NEXUS_DmaJobSettings settings;
    unsigned numBlocks;
    bool flushAfter; /* true if ANY NEXUS_DmaJobBlockSettings[].cached is true. i.e. flushAfter is per-job, while .cached is per-jobBlock */
    BXPT_Dma_ContextBlockSettings blockSettings[1]; /* variable size array, must be last entry in the structure */
} NEXUS_DmaJob;

typedef struct NEXUS_Dma
{
    NEXUS_OBJECT(NEXUS_Dma);
    BLST_S_ENTRY(NEXUS_Dma) link;
    unsigned index;
    NEXUS_DmaSettings settings;
    BLST_S_HEAD(JobList, NEXUS_DmaJob) jobList; /* this keeps track of the opened BXPT_Dma_Context's */
} NEXUS_Dma;

static void NEXUS_Dma_P_CompleteCallback_isr(void *pParam, int iParam);
static void NEXUS_Dma_P_CompleteEvent(void* context);


void NEXUS_Dma_GetDefaultSettings(
    NEXUS_DmaSettings *pSettings
    )
{
    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    return;
}

static NEXUS_Error NEXUS_Dma_P_ApplySettings(
    NEXUS_DmaHandle handle, 
    const NEXUS_DmaSettings *pSettings
    )
{
    NEXUS_DmaJobHandle pJob;
    BXPT_Dma_ContextSettings ctxSettings;
    BERR_Code rc;
    
    BDBG_OBJECT_ASSERT(handle, NEXUS_Dma);
    BDBG_ASSERT(pSettings);

    /* apply settings to all XPT_Dma contexts */
    for (pJob=BLST_S_FIRST(&handle->jobList); pJob; pJob=BLST_S_NEXT(pJob, jobNode)) {
        BXPT_Dma_Context_GetSettings(pJob->ctx, &ctxSettings);
        ctxSettings.endianMode = pSettings->endianMode;
        ctxSettings.swapMode = pSettings->swapMode;
        rc = BXPT_Dma_Context_SetSettings(pJob->ctx, &ctxSettings);
        if (rc!=BERR_SUCCESS) {
            rc = BERR_TRACE(rc);
            return NEXUS_INVALID_PARAMETER;
        }
    }

    BKNI_Memcpy(&handle->settings, pSettings, sizeof(*pSettings));
    return NEXUS_SUCCESS;
}


static void NEXUS_Dma_P_Finalizer(NEXUS_DmaHandle handle)
{
    NEXUS_DmaJob *pJob;
    NEXUS_OBJECT_ASSERT(NEXUS_Dma, handle);

    while ((pJob = BLST_S_FIRST(&handle->jobList))) {
        BDBG_WRN(("NEXUS_Dma_Close: stale job %#lx", (unsigned long)pJob));
        NEXUS_OBJECT_UNREGISTER(NEXUS_DmaJob, pJob, Destroy);
        NEXUS_DmaJob_Destroy(pJob);
    }

    BLST_S_REMOVE(&pTransport->dmaChannel[handle->index].dmaHandles, handle, NEXUS_Dma, link);
    if (BLST_S_EMPTY(&pTransport->dmaChannel[handle->index].dmaHandles)) {
        BXPT_Dma_CloseChannel(pTransport->dmaChannel[handle->index].dma);
        pTransport->dmaChannel[handle->index].dma = NULL;
    }
    NEXUS_OBJECT_DESTROY(NEXUS_Dma, handle);
    BKNI_Free(handle);
}

static void NEXUS_Dma_P_Release(NEXUS_DmaHandle handle)
{
    NEXUS_OBJECT_ASSERT(NEXUS_Dma, handle);
    NEXUS_CLIENT_RESOURCES_RELEASE(dma,Count,NEXUS_ANY_ID);
    return;
}

NEXUS_OBJECT_CLASS_MAKE_WITH_RELEASE(NEXUS_Dma, NEXUS_Dma_Close);

NEXUS_DmaHandle NEXUS_Dma_Open(
    unsigned index,
    const NEXUS_DmaSettings *pSettings  /* may be NULL for default settings */
    )
{
    NEXUS_Error rc;
    NEXUS_DmaHandle dma;
    NEXUS_DmaSettings settings;

#if BXPT_DMA_HAS_MEMDMA_MCPB
    if (index == NEXUS_ANY_ID) {
        unsigned i;
        /* search for unused HW channels */
        for (i=0; i<NEXUS_NUM_DMA_CHANNELS; i++) {
            if (pTransport->dmaChannel[i].dma==NULL) {
                index = i;
                break;
            }
        }
        if (i==NEXUS_NUM_DMA_CHANNELS) { /* if all HW channels are used, we have to virtualize */
            #define DEFAULT_VIRTUAL_CHANNEL 0
            BDBG_WRN(("All %u DMA channels in use. Virtualizing on channel %u", NEXUS_NUM_DMA_CHANNELS, DEFAULT_VIRTUAL_CHANNEL));
            index = DEFAULT_VIRTUAL_CHANNEL;
        }
    }
#else
    /* MCPB channels are shared between playpump and DMA. for DMA, count back from MAX */
    if (index == NEXUS_ANY_ID) {
        int i;
        unsigned firstInUse = NEXUS_NUM_DMA_CHANNELS;
        for (i=NEXUS_NUM_DMA_CHANNELS-1; i>=0; i--) {
            if (pTransport->dmaChannel[i].dma==NULL && pTransport->playpump[i].playpump==NULL) {
                index = i;
                break;
            }
            else if (firstInUse==NEXUS_NUM_DMA_CHANNELS && pTransport->dmaChannel[i].dma) {
                firstInUse = i;
            }
        }
        if (i<0) {
            if (firstInUse<NEXUS_NUM_DMA_CHANNELS) {
                BDBG_WRN(("All DMA channels in use. Virtualizing on channel %u", firstInUse));
                index = firstInUse;
            }
            else {
                BDBG_ERR(("No DMA channels available"));
                rc = BERR_TRACE(NEXUS_UNKNOWN);
                return NULL;
            }
        }
    }
    else if (index<NEXUS_NUM_DMA_CHANNELS) {
        unsigned newIndex = NEXUS_NUM_DMA_CHANNELS - 1 - index;
        if (pTransport->playpump[newIndex].playpump) {
            BDBG_ERR(("DMA channel %u's MCPB channel is already used by playback", index));
            BERR_TRACE(NEXUS_INVALID_PARAMETER);
            return NULL;
        }
        BDBG_MSG(("Using DMA index %u -> %u", index, newIndex));
        index = newIndex;
    }
#endif

    if (index>=NEXUS_NUM_DMA_CHANNELS) {
        BDBG_ERR(("DMA channel %d is not supported on this chipset", index));
        BERR_TRACE(NEXUS_INVALID_PARAMETER);
        return NULL;
    }

    if (!pSettings) {
        NEXUS_Dma_GetDefaultSettings(&settings);
        pSettings = &settings;
    }

    if (pSettings->coreType!=NEXUS_DmaCoreType_eM2m) {
        BERR_TRACE(NEXUS_INVALID_PARAMETER);
        return NULL;        
    }

    if (pTransport->dmaChannel[index].dma==NULL) {
        BXPT_Dma_Settings dmaSettings;
        BXPT_Dma_GetDefaultSettings(&dmaSettings);
        rc = BXPT_Dma_OpenChannel(pTransport->xpt, &pTransport->dmaChannel[index].dma, index, &dmaSettings);
        if (rc) {
            BERR_TRACE(rc);
            return NULL;
        }
    }
    
    rc = NEXUS_CLIENT_RESOURCES_ACQUIRE(dma,Count,NEXUS_ANY_ID);
    if (rc) { 
        BXPT_Dma_CloseChannel(pTransport->dmaChannel[index].dma);
        BERR_TRACE(rc);
        return NULL; 
    }

    dma = BKNI_Malloc(sizeof(*dma));
    if (!dma) {
        NEXUS_CLIENT_RESOURCES_RELEASE(dma,Count,NEXUS_ANY_ID);
        BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
        return NULL;
    }

    NEXUS_OBJECT_INIT(NEXUS_Dma, dma);
    dma->index = index;

    BLST_S_INIT(&dma->jobList);
    BLST_S_INSERT_HEAD(&pTransport->dmaChannel[index].dmaHandles, dma, link);

    BKNI_Memcpy(&dma->settings, pSettings, sizeof(*pSettings));
    return dma;    
}

NEXUS_Error NEXUS_Dma_GetSettings(
    NEXUS_DmaHandle handle,
    NEXUS_DmaSettings *pSettings
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_Dma);
    BDBG_ASSERT(NULL != pSettings);
    *pSettings = handle->settings;
    return BERR_SUCCESS;
}

NEXUS_Error NEXUS_Dma_SetSettings(
    NEXUS_DmaHandle handle,
    const NEXUS_DmaSettings *pSettings
    )
{
    NEXUS_Error errCode;

    BDBG_OBJECT_ASSERT(handle, NEXUS_Dma);
    BDBG_ASSERT(pSettings);

    if (handle->settings.coreType != pSettings->coreType) {
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }
    errCode = NEXUS_Dma_P_ApplySettings(handle, pSettings);
    if (errCode!=BERR_SUCCESS) {
        return BERR_TRACE(errCode);
    }

    return NEXUS_SUCCESS;
}

void NEXUS_DmaJob_GetDefaultSettings(
    NEXUS_DmaJobSettings *pSettings
    )
{
    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->numBlocks = 1;
    pSettings->keySlot = NULL;
    NEXUS_CallbackDesc_Init(&pSettings->completionCallback);
}

static NEXUS_Error NEXUS_Dma_P_ApplyJobSettings(
    NEXUS_DmaJobHandle handle, 
    const NEXUS_DmaJobSettings *pSettings,
    bool setChannelSettings
    )
{
    NEXUS_Error rc;
    BXPT_Dma_Handle xdma = pTransport->dmaChannel[handle->parent->index].dma;
    BXPT_Dma_ScramMode scramMode;
    BXPT_Dma_Settings chnSettings;
    BXPT_Dma_ContextSettings ctxSettings;

    if (pSettings->numBlocks != handle->settings.numBlocks) {
        rc = BERR_TRACE(NEXUS_INVALID_PARAMETER);
        goto error;
    }

    BXPT_Dma_Context_GetSettings(handle->ctx, &ctxSettings);
    ctxSettings.endianMode = handle->parent->settings.endianMode;
    switch (handle->parent->settings.swapMode) {
        case NEXUS_DmaSwapMode_eNone:
            ctxSettings.swapMode = BXPT_Dma_SwapMode_eNone;
            break;
        case NEXUS_DmaSwapMode_eWord:
            ctxSettings.swapMode = BXPT_Dma_SwapMode_eWord;
            break;
        case NEXUS_DmaSwapMode_eByte:
            ctxSettings.swapMode = BXPT_Dma_SwapMode_eByte;
            break;
        default:
            ctxSettings.swapMode = BXPT_Dma_SwapMode_eMax;
            break;
    }
    ctxSettings.callback_isr = NEXUS_Dma_P_CompleteCallback_isr;
    ctxSettings.pParm1 = handle;

    switch (pSettings->dataFormat) {
        default:
        case NEXUS_DmaDataFormat_eBlock:
            scramMode = BXPT_Dma_ScramMode_eBlock;
            break;
        case NEXUS_DmaDataFormat_eMpeg:
            scramMode = BXPT_Dma_ScramMode_eMpeg;
            break;
        case NEXUS_DmaDataFormat_eDss:
            scramMode = BXPT_Dma_ScramMode_eDss;
            break;
    }

    if (pSettings->keySlot == NULL) {
        /* associate a bypass keyslot with the transfer. */
        ctxSettings.pidChannelNum = NEXUS_PidChannel_GetBypassKeySlotIndex_isrsafe(pSettings->bypassKeySlot);
    }
    else {
        NEXUS_KeySlotFastInfo keySlotInfo;

        NEXUS_KeySlot_GetFastInfo(pSettings->keySlot, &keySlotInfo);
        if (!keySlotInfo.dma.valid) { rc = BERR_TRACE(NEXUS_INVALID_PARAMETER); goto error; }
        ctxSettings.pidChannelNum = keySlotInfo.dma.pidChannelIndex;
    }

    ctxSettings.useRPipe = pSettings->useRPipe;

    if (setChannelSettings) {
        BXPT_Dma_GetSettings(xdma, &chnSettings);
        chnSettings.scramMode = scramMode;
        chnSettings.timestampEnabled = pSettings->timestampType!=NEXUS_TransportTimestampType_eNone;
        rc = BXPT_Dma_SetSettings(xdma, &chnSettings);
        if (rc!=BERR_SUCCESS) {
            rc = BERR_TRACE(NEXUS_UNKNOWN);
            goto error;
        }
    }
    
    rc = BXPT_Dma_Context_SetSettings(handle->ctx, &ctxSettings);
    if (rc!=BERR_SUCCESS) {
        rc = BERR_TRACE(NEXUS_UNKNOWN);
        goto error;
    }

    NEXUS_TaskCallback_Set(handle->completionCallback, &pSettings->completionCallback);
    handle->settings = *pSettings;
    return NEXUS_SUCCESS;

error:
    return rc;
}

NEXUS_Error
NEXUS_DmaJob_P_HwPidChannel_Disconnect(NEXUS_DmaJobHandle handle, NEXUS_P_HwPidChannel *pidChannel)
{
    BERR_Code rc;
    NEXUS_P_DmaPidChannel *dma_pid;
    unsigned index;

    BLST_S_DICT_FIND(&handle->pid_list, dma_pid, pidChannel, pid_channel, link);
    if(dma_pid==NULL) {
        BDBG_WRN(("NEXUS_ClosePidChannel: %#lx can't find pid:%#lx", (unsigned long)handle, (unsigned long)pidChannel));
        rc = BERR_TRACE(NEXUS_INVALID_PARAMETER);
        goto err_not_found;
    }
    BLST_S_DICT_REMOVE(&handle->pid_list, dma_pid, pidChannel, NEXUS_P_DmaPidChannel, pid_channel, link);
    BKNI_Free(dma_pid);

    BXPT_Dma_Context_ConfigurePidChannel(handle->ctx, pidChannel->status.pidChannelIndex, 0, false);

    index = pidChannel->status.pidChannelIndex - BXPT_P_MEMDMA_PID_CHANNEL_START;
    if (index >= BXPT_DMA_NUM_PID_CHANNELS) {
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }
    if (pTransport->hwDmaPidChannelRefCnt[index]) {
        pTransport->hwDmaPidChannelRefCnt[index]--;
    }

    pTransport->hwDmaPidChannelRefCnt[index]++;
    return 0;

err_not_found:
    return rc;
}

static void NEXUS_DmaJob_P_Finalizer(NEXUS_DmaJobHandle handle)
{
    NEXUS_P_DmaPidChannel *next_dma_pid;
    NEXUS_OBJECT_ASSERT(NEXUS_DmaJob, handle);
    for (next_dma_pid = BLST_S_FIRST(&handle->pid_list); next_dma_pid;) {
        NEXUS_P_DmaPidChannel *dma_pid = next_dma_pid;
        next_dma_pid = BLST_S_NEXT(dma_pid, link);
        NEXUS_PidChannel_Close(BLST_S_FIRST(&dma_pid->pid_channel->swPidChannels));
    }
    BXPT_Dma_Context_Destroy(handle->ctx);

    BLST_S_REMOVE(&handle->parent->jobList, handle, NEXUS_DmaJob, jobNode);
    NEXUS_UnregisterEvent(handle->completionEventHandler);
    BKNI_DestroyEvent(handle->completionEvent);
    NEXUS_TaskCallback_Destroy(handle->completionCallback);
    BKNI_Free(handle->memory);
    NEXUS_OBJECT_DESTROY(NEXUS_DmaJob, handle);
    BKNI_Free(handle);
    return;
}

NEXUS_OBJECT_CLASS_MAKE(NEXUS_DmaJob, NEXUS_DmaJob_Destroy);

NEXUS_DmaJobHandle NEXUS_DmaJob_Create(
    NEXUS_DmaHandle dmaHandle,
    const NEXUS_DmaJobSettings *pSettings
    )
{
    NEXUS_DmaJob *pJob, *pFirstJob;
    NEXUS_DmaJobSettings jobSettings;
    NEXUS_Error rc;
    BXPT_Dma_ContextSettings ctxSettings;
    NEXUS_HeapHandle heap;
    bool isFirstJob = true;

    BDBG_OBJECT_ASSERT(dmaHandle, NEXUS_Dma);

    if (NULL == pSettings) {
        NEXUS_DmaJob_GetDefaultSettings(&jobSettings);
        pSettings = &jobSettings;
    }
    if (pSettings->numBlocks<=0) {
        rc = BERR_TRACE(BERR_INVALID_PARAMETER);
        return NULL;
    }
    if(pSettings->dataFormat >= NEXUS_DmaDataFormat_eMax) {
        rc = BERR_TRACE(BERR_INVALID_PARAMETER);
        return NULL;
    }

    pJob = BKNI_Malloc(sizeof(*pJob)+sizeof(BXPT_Dma_ContextBlockSettings)*(pSettings->numBlocks));
    if (NULL == pJob) {
        rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        return NULL;
    }

    BKNI_Memset(pJob, 0, sizeof(*pJob)+sizeof(BXPT_Dma_ContextBlockSettings)*(pSettings->numBlocks));
    NEXUS_OBJECT_INIT(NEXUS_DmaJob, pJob);
    BLST_S_INIT(&pJob->pid_list);

    pJob->memory = BKNI_Malloc(sizeof(*pJob->memory)*pSettings->numBlocks);
    if (NULL == pJob->memory) {
        rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto error;
    }
    BKNI_Memset(pJob->memory, 0, sizeof(*pJob->memory)*pSettings->numBlocks);
    pJob->parent = dmaHandle;
    pJob->settings = *pSettings;
    pJob->completionCallback = NEXUS_TaskCallback_Create(pJob, NULL);

    rc = BKNI_CreateEvent(&pJob->completionEvent);
    if (rc) { rc=BERR_TRACE(rc); goto error; }
    pJob->completionEventHandler = NEXUS_RegisterEvent(pJob->completionEvent, NEXUS_Dma_P_CompleteEvent, pJob);
    if (pJob->completionEventHandler == NULL) { rc=BERR_TRACE(rc); goto error; }

    /* create the context with maxNumBlocks and memoryBounds. change the settings afterwards */
    BXPT_Dma_Context_GetDefaultSettings(&ctxSettings);
    ctxSettings.maxNumBlocks = pSettings->numBlocks;
    heap = NEXUS_P_DefaultHeap(dmaHandle->settings.boundsHeap, NEXUS_DefaultHeapType_eBounds);
    if (heap) {
        NEXUS_MemoryStatus status;
        rc = NEXUS_Heap_GetStatus(heap, &status);
        if (rc) {rc = BERR_TRACE(rc); goto error;}
        ctxSettings.memoryBounds.offset = status.offset;
        ctxSettings.memoryBounds.size = status.size;
    }
    pJob->ctx = BXPT_Dma_Context_Create(pTransport->dmaChannel[dmaHandle->index].dma, &ctxSettings);
    if (!pJob->ctx) {
        goto error;
    }

    /* in XPT_DMA, certain settings are per-channel, not per-context.
       new jobs must match old jobs' settings */
    {
        NEXUS_DmaHandle d;
        for (d=BLST_S_FIRST(&pTransport->dmaChannel[dmaHandle->index].dmaHandles); d; d=BLST_S_NEXT(d, link)) {
            pFirstJob = BLST_S_FIRST(&d->jobList);
            if (pFirstJob) {
                isFirstJob = false;
                if ((pFirstJob->settings.dataFormat != pSettings->dataFormat) || pFirstJob->settings.timestampType != pSettings->timestampType) {
                    BDBG_ERR(("NEXUS_DmaDataFormat and TimestampType must match for all jobs created against channel %u", dmaHandle->index));
                    rc = BERR_TRACE(NEXUS_NOT_SUPPORTED);
                    BXPT_Dma_Context_Destroy(pJob->ctx);
                    goto error;
                }
            }
        }
    }

    rc = NEXUS_Dma_P_ApplyJobSettings(pJob, pSettings, isFirstJob);
    if (rc!=NEXUS_SUCCESS) {
        BXPT_Dma_Context_Destroy(pJob->ctx);
        goto error;
    }
    BLST_S_INSERT_HEAD(&dmaHandle->jobList, pJob, jobNode);
    return pJob;

error:
    if (pJob->completionEventHandler) {
        NEXUS_UnregisterEvent(pJob->completionEventHandler);
    }
    if (pJob->completionEvent) {
        BKNI_DestroyEvent(pJob->completionEvent);
    }
    NEXUS_TaskCallback_Destroy(pJob->completionCallback);
    if (pJob->memory) {
        BKNI_Free(pJob->memory);
    }
    BDBG_OBJECT_DESTROY(pJob, NEXUS_DmaJob);
    BKNI_Free(pJob);
    return NULL;    
}

void NEXUS_DmaJob_GetSettings(
    NEXUS_DmaJobHandle handle,
    NEXUS_DmaJobSettings *pSettings
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_DmaJob);
    BDBG_ASSERT(NULL != pSettings);
    *pSettings = handle->settings;
    return;
}

NEXUS_Error NEXUS_DmaJob_SetSettings(
    NEXUS_DmaJobHandle handle,
    const NEXUS_DmaJobSettings *pSettings
    )
{
    unsigned numJobs;
    NEXUS_DmaJobHandle pJob;

    for (numJobs=0,pJob=BLST_S_FIRST(&handle->parent->jobList); pJob; pJob=BLST_S_NEXT(pJob, jobNode)) {
        numJobs++;
    }

    /* in XPT_DMA, certain settings are per-channel, not per-context.
       disallow changing those settings if there is more than one context */
    if (numJobs > 1) {
        if ((handle->settings.dataFormat != pSettings->dataFormat) || (handle->settings.timestampType != pSettings->timestampType)) {
            return BERR_TRACE(NEXUS_NOT_SUPPORTED);
        }
    }

    return NEXUS_Dma_P_ApplyJobSettings(handle, pSettings, true);
}

void NEXUS_DmaJob_GetDefaultBlockSettings(
    NEXUS_DmaJobBlockSettings *pSettings
    )
{
    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->cached = true;
    return;
}

void NEXUS_DmaJob_GetDefaultBlockOffsetSettings(
    NEXUS_DmaJobBlockOffsetSettings *pSettings
    )
{
    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    return;
}

NEXUS_Error NEXUS_DmaJob_GetStatus(
    NEXUS_DmaJobHandle handle,
    NEXUS_DmaJobStatus *pStatus
    )
{
    NEXUS_Error errCode;
    BXPT_Dma_ContextStatus ctxStatus;

    BDBG_OBJECT_ASSERT(handle, NEXUS_DmaJob);
    BDBG_ASSERT(NULL != pStatus);
    BDBG_MSG_TRACE(("NEXUS_DmaJob_GetStatus>:%#lx", (unsigned long)handle));

    errCode = BXPT_Dma_Context_GetStatus(handle->ctx, &ctxStatus);
    if (errCode) {
        pStatus->currentState = NEXUS_DmaJobState_eUnknown;
        return BERR_TRACE(errCode);
    }

    switch (ctxStatus.state) {
        case BXPT_Dma_ContextState_eIdle:
            pStatus->currentState = NEXUS_DmaJobState_eComplete;
            break;
        case BXPT_Dma_ContextState_eInProgress:
            pStatus->currentState = NEXUS_DmaJobState_eInProgress;
            break;
        default:
            pStatus->currentState = NEXUS_DmaJobState_eUnknown;
            break;
    }

    if (handle->state==NEXUS_DmaJob_P_StateQueued) { /* we're not complete until CompleteEvent is executed */
        pStatus->currentState = NEXUS_DmaJobState_eInProgress;
    }
    BDBG_MSG_TRACE(("NEXUS_DmaJob_GetStatus<:%#lx", (unsigned long)handle));
    return BERR_SUCCESS;
}

NEXUS_Error NEXUS_DmaJob_P_Wait(NEXUS_DmaJobHandle handle)
{
    unsigned i;
    for(i=0;i<100;i++) {
        BXPT_Dma_ContextStatus ctxStatus;
        NEXUS_Error rc;
        rc = BXPT_Dma_Context_GetStatus(handle->ctx, &ctxStatus);
        if (rc) return BERR_TRACE(rc);
        if (ctxStatus.state == BXPT_Dma_ContextState_eIdle) {
            return NEXUS_SUCCESS;
        }
        if (i % 20 == 19) {
            BDBG_WRN(("NEXUS_Recpump_StopData: %p waiting for DMA to complete", (void*)handle));
        }
        BKNI_Sleep(1);
    }
    BDBG_ERR(("NEXUS_Recpump_StopData: %p timed out waiting for DMA to complete", (void*)handle));
    return BERR_TRACE(NEXUS_TIMEOUT);
}

static void NEXUS_Dma_P_CompleteEvent(void* context)
{
    NEXUS_DmaJobHandle handle = context;
    unsigned i;
    BDBG_OBJECT_ASSERT(handle, NEXUS_DmaJob);

    BDBG_MSG(("completed job:%#lx", (unsigned long)handle));
    handle->state = NEXUS_DmaJob_P_StateIdle;

    if (handle->flushAfter) {
        BXPT_Dma_ContextBlockSettings *pSettings;
        pSettings = handle->blockSettings;
        for (i=0, pSettings = handle->blockSettings; i<handle->numBlocks; i++, pSettings++) {
            NEXUS_FlushCache(handle->memory[i].dstAddrPtr, pSettings->size);
        }
        handle->flushAfter = false;
    }

    if (handle->cryptoCallback.function) {
        struct NEXUS_DmaJobCallback cryptoCallback = handle->cryptoCallback;
        /* clear before calling, because the callback may recursively set it again */
        handle->cryptoCallback.function = NULL;
        cryptoCallback.function(cryptoCallback.context);
    }

    NEXUS_TaskCallback_Fire(handle->completionCallback);
}

static void NEXUS_Dma_P_CompleteCallback_isr(void *pParam, int iParam)
{
    NEXUS_DmaJobHandle handle = (NEXUS_DmaJobHandle)pParam;

    BDBG_OBJECT_ASSERT(handle, NEXUS_DmaJob);
    BSTD_UNUSED(iParam);

    BKNI_SetEvent_isr(handle->completionEvent);
}

static NEXUS_Error
NEXUS_DmaJob_P_ProcessBlocks(NEXUS_DmaJobHandle handle, const NEXUS_DmaJobBlockSettings *pSettings, unsigned nBlocks)
{
    NEXUS_Error errCode;
    BXPT_Dma_ContextBlockSettings *blockSettings = handle->blockSettings;
    uint64_t srcOffset, dstOffset;
    unsigned i = 0, dmaLength = 0;

    if (handle->state != NEXUS_DmaJob_P_StateIdle) {
        return BERR_TRACE(NEXUS_NOT_AVAILABLE);
    }
    if (nBlocks > handle->settings.numBlocks) {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    handle->numBlocks = nBlocks;

    BDBG_MSG(("  started job:%#lx nBlocks:%u", (unsigned long)handle, nBlocks));

    for (i=0; i<nBlocks; i++,pSettings++) {
        /* convert to offset */
        srcOffset = NEXUS_AddrToOffset((void *)pSettings->pSrcAddr);
        if (0 == srcOffset) {
            BDBG_ERR(("Unusable pSrcAddr %p. NEXUS_Dma requires Nexus heap memory, not kernel allocated memory.", pSettings->pSrcAddr));
            return BERR_TRACE(BERR_INVALID_PARAMETER);
        }
        if (pSettings->cached) {
            if (!NEXUS_P_CpuAccessibleAddress(pSettings->pSrcAddr)) {
                BDBG_ERR(("Cannot do driver-side cacheflush of pSrcAddr %p (" BDBG_UINT64_FMT ") with no memory mapping. Set cached=false and flush in the app.",
                    pSettings->pSrcAddr, BDBG_UINT64_ARG(srcOffset)));
                return BERR_TRACE(BERR_INVALID_PARAMETER);
            }
            NEXUS_FlushCache(pSettings->pSrcAddr, pSettings->blockSize);
        }
        if (pSettings->pSrcAddr == pSettings->pDestAddr) {
            dstOffset = srcOffset;
        }
        else {
            dstOffset = NEXUS_AddrToOffset(pSettings->pDestAddr);
            if (0 == dstOffset) {
                BDBG_ERR(("Unusable pDestAddr %p. NEXUS_Dma requires Nexus heap memory, not kernel allocated memory.", pSettings->pDestAddr));
                return BERR_TRACE(BERR_INVALID_PARAMETER);
            }
            if (pSettings->cached) {
                if (!NEXUS_P_CpuAccessibleAddress(pSettings->pDestAddr)) {
                    BDBG_ERR(("Cannot do driver-side cacheflush of pDestAddr %p (" BDBG_UINT64_FMT ") with no memory mapping. Set cached=false and flush in the app.",
                        pSettings->pDestAddr, BDBG_UINT64_ARG(dstOffset)));
                    return BERR_TRACE(BERR_INVALID_PARAMETER);
                }
                NEXUS_FlushCache(pSettings->pDestAddr, pSettings->blockSize);
            }
        }
        handle->flushAfter = pSettings->cached;

        handle->memory[i].srcAddrPtr = (void*)pSettings->pSrcAddr;
        handle->memory[i].dstAddrPtr = pSettings->pDestAddr;

        BXPT_Dma_Context_GetDefaultBlockSettings(&blockSettings[i]);
        blockSettings[i].src = srcOffset;
        blockSettings[i].dst = dstOffset;
        blockSettings[i].size = pSettings->blockSize;
        blockSettings[i].resetCrypto = pSettings->resetCrypto;
        blockSettings[i].sgScramStart = pSettings->scatterGatherCryptoStart;
        blockSettings[i].sgScramEnd = pSettings->scatterGatherCryptoEnd;
        blockSettings[i].securityBtp = pSettings->securityBtp;
        dmaLength += pSettings->blockSize;
    }

    handle->state = NEXUS_DmaJob_P_StateQueued;
    errCode = BXPT_Dma_Context_Enqueue(handle->ctx, blockSettings, nBlocks); /* ISR callback may fire before this function returns, which is fine */
    switch (errCode) {
        case BERR_SUCCESS: /* completed */
            handle->state = NEXUS_DmaJob_P_StateIdle;
            goto completed;
            break;
        case BXPT_DMA_QUEUED:
            if (dmaLength <= handle->settings.busyWaitThreshold) {
                unsigned busyWait = handle->settings.busyWait;
                for(; busyWait>0; busyWait--) {
                    BKNI_Delay(1);
                    if (handle->state==NEXUS_DmaJob_P_StateIdle) {
                        goto completed;
                    }
                }
            }
            break;
        default:
            handle->state = NEXUS_DmaJob_P_StateIdle;
            return BERR_TRACE(errCode);
    }

    BDBG_MSG(("   queued job:%#lx", (unsigned long)handle));
    return NEXUS_DMA_QUEUED;
completed:
    return NEXUS_SUCCESS;
}

NEXUS_Error
NEXUS_DmaJob_ProcessBlocks(
    NEXUS_DmaJobHandle handle, 
    const NEXUS_DmaJobBlockSettings *pSettings, 
    unsigned nBlocks
    )
{
    NEXUS_Error rc;

    BDBG_OBJECT_ASSERT(handle, NEXUS_DmaJob);
    BDBG_ASSERT(NULL != pSettings);
    BDBG_MSG_TRACE(("NEXUS_DmaJob_ProcessBlocks>:%#lx", (unsigned long)handle));
    rc = NEXUS_DmaJob_P_ProcessBlocks(handle, pSettings, nBlocks);
    BDBG_MSG_TRACE(("NEXUS_DmaJob_ProcessBlocks<:%#lx", (unsigned long)handle));
    return rc;
}

static NEXUS_Error
NEXUS_DmaJob_P_ProcessBlocksOffset(NEXUS_DmaJobHandle handle, const NEXUS_DmaJobBlockOffsetSettings *pSettings, unsigned nBlocks)
{
    NEXUS_Error errCode;
    BXPT_Dma_ContextBlockSettings *blockSettings = handle->blockSettings;
    NEXUS_Addr srcOffset, dstOffset;
    unsigned i, dmaLength = 0;

    if (handle->state != NEXUS_DmaJob_P_StateIdle) {
        return BERR_TRACE(NEXUS_NOT_AVAILABLE);
    }
    if (nBlocks > handle->settings.numBlocks) {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    BDBG_MSG(("  started job:%#lx nBlocks:%u", (unsigned long)handle, nBlocks));

    for (i=0; i<nBlocks; i++,pSettings++) {
        srcOffset = pSettings->srcOffset;
        dstOffset = pSettings->destOffset;

        BXPT_Dma_Context_GetDefaultBlockSettings(&blockSettings[i]);
        blockSettings[i].src = srcOffset;
        blockSettings[i].dst = dstOffset;
        blockSettings[i].size = pSettings->blockSize;
        blockSettings[i].resetCrypto = pSettings->resetCrypto;
        blockSettings[i].sgScramStart = pSettings->scatterGatherCryptoStart;
        blockSettings[i].sgScramEnd = pSettings->scatterGatherCryptoEnd;
        blockSettings[i].securityBtp = pSettings->securityBtp;
        dmaLength += pSettings->blockSize;
    }
    handle->flushAfter = false; /* pSettings->cached is ignored */

    handle->state = NEXUS_DmaJob_P_StateQueued;

    errCode = BXPT_Dma_Context_Enqueue(handle->ctx, blockSettings, nBlocks); /* ISR callback may fire before this function returns, which is fine */
    switch (errCode) {
        case BERR_SUCCESS: /* completed */
            handle->state = NEXUS_DmaJob_P_StateIdle;
            goto completed;
            break;
        case BXPT_DMA_QUEUED:
            if (dmaLength <= handle->settings.busyWaitThreshold) {
                unsigned busyWait = handle->settings.busyWait;
                for(; busyWait>0; busyWait--) {
                    BKNI_Delay(1);
                    if (handle->state==NEXUS_DmaJob_P_StateIdle) {
                        goto completed;
                    }
                }
            }
            break;
        default:
            handle->state = NEXUS_DmaJob_P_StateIdle;
            return BERR_TRACE(errCode);
    }

    BDBG_MSG(("   queued job:%#lx", (unsigned long)handle));
    return NEXUS_DMA_QUEUED;
completed:
    return NEXUS_SUCCESS;
}

NEXUS_Error 
NEXUS_DmaJob_ProcessBlocksOffset(
    NEXUS_DmaJobHandle handle,
    const NEXUS_DmaJobBlockOffsetSettings *pSettings,
    unsigned nBlocks
    )
{
    NEXUS_Error rc;

    BDBG_OBJECT_ASSERT(handle, NEXUS_DmaJob);
    BDBG_ASSERT(NULL != pSettings);
    BDBG_MSG_TRACE(("NEXUS_DmaJob_ProcessBlocksOffset>:%#lx", (unsigned long)handle));
    rc = NEXUS_DmaJob_P_ProcessBlocksOffset(handle, pSettings, nBlocks);
    BDBG_MSG_TRACE(("NEXUS_DmaJob_ProcessBlocksOffset<:%#lx", (unsigned long)handle));
    return rc;
}


/* used by nexus_playpump_crypto.c, nexus_recpump.c, etc */
NEXUS_Error
NEXUS_DmaJob_ProcessBlocks_priv(NEXUS_DmaJobHandle handle, const NEXUS_DmaJobBlockSettings *pSettings, unsigned nBlocks, void (*callback)(void *), void *context)
{
    NEXUS_ASSERT_MODULE();
    BDBG_OBJECT_ASSERT(handle, NEXUS_DmaJob);
    BDBG_ASSERT(NULL != pSettings);

    handle->cryptoCallback.function = callback;
    handle->cryptoCallback.context = context;
    return NEXUS_DmaJob_P_ProcessBlocks(handle, pSettings, nBlocks);
}

NEXUS_PidChannelHandle NEXUS_DmaJob_OpenPidChannel(NEXUS_DmaJobHandle handle, unsigned pid, const NEXUS_PidChannelSettings *pSettings)
{
    NEXUS_PidChannelSettings pidSettings;
    NEXUS_PidChannelHandle pidChannel;
    NEXUS_P_DmaPidChannel *dma_pid = NULL;
    BERR_Code rc;

    if (handle->settings.dataFormat!=NEXUS_DmaDataFormat_eMpeg) {
        BERR_TRACE(NEXUS_NOT_SUPPORTED);
        return NULL;
    }
    if (handle->settings.keySlot || handle->settings.bypassKeySlot) {
        BERR_TRACE(NEXUS_NOT_SUPPORTED);
        return NULL;
    }

    if (pSettings==NULL) {
        NEXUS_PidChannel_GetDefaultSettings(&pidSettings);
    }
    else {
        pidSettings = *pSettings;
    }
    if (pidSettings.pidChannelIndex!=NEXUS_PID_CHANNEL_OPEN_ANY) {
        BERR_TRACE(NEXUS_NOT_SUPPORTED);
        return NULL;
    }

    pidChannel = NEXUS_PidChannel_OpenDma_Priv(pid, &pidSettings);
    if (pidChannel==NULL) {
        goto error;
    }
    NEXUS_OBJECT_REGISTER(NEXUS_PidChannel, pidChannel, Open);

    pidChannel->hwPidChannel->dma = handle;
    rc = BXPT_Dma_Context_ConfigurePidChannel(handle->ctx, pidChannel->hwPidChannel->status.pidChannelIndex, pid, true);
    if (rc) {
        goto error;
    }

    dma_pid = BKNI_Malloc(sizeof(*dma_pid));
    if (!dma_pid) { BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY); goto error; }
    BKNI_Memset(dma_pid, 0, sizeof(*dma_pid));
    dma_pid->pid_channel = pidChannel->hwPidChannel;
    BLST_S_DICT_ADD(&handle->pid_list, dma_pid, NEXUS_P_DmaPidChannel, pid_channel, link, error);

    BDBG_MSG(("Allocate DMA pidchannel %u,%p:%p", pidChannel->hwPidChannel->status.pidChannelIndex, (void*)pidChannel, (void*)pidChannel->hwPidChannel));
    return pidChannel;

error:
    if (dma_pid) {
        BKNI_Free(dma_pid);
    }
    if (pidChannel) {
        NEXUS_PidChannel_Close(pidChannel);
    }
    return 0;
}

unsigned NEXUS_PidChannel_GetBypassKeySlotIndex_isrsafe(NEXUS_BypassKeySlot bypassKeySlot)
{
    /* for current ARM systems, BXPT_NUM_PID_CHANNELS is 768 and BXPT_DMA_NUM_PID_CHANNELS is 256,
    so we reserve 1023 and 1022 for clear M2M. but this should adapt if this changes. */
    switch (bypassKeySlot) {
    default: BERR_TRACE(NEXUS_INVALID_PARAMETER); /* pass through to G2GR */
    case NEXUS_BypassKeySlot_eG2GR:
        return BXPT_DMA_NUM_PID_CHANNELS+BXPT_NUM_PID_CHANNELS-1;
    case NEXUS_BypassKeySlot_eGR2R:
        return BXPT_DMA_NUM_PID_CHANNELS+BXPT_NUM_PID_CHANNELS-2;
    case NEXUS_BypassKeySlot_eGT2T:
        return BXPT_DMA_NUM_PID_CHANNELS+BXPT_NUM_PID_CHANNELS-3;
    /* if extended, NEXUS_BypassKeySlot_eMax-1 must be the lowest pidChannelIndex for NEXUS_PidChannel_OpenDma_Priv's search to work */
    }
}

NEXUS_PidChannelHandle NEXUS_PidChannel_OpenDma_Priv(unsigned pid, const NEXUS_PidChannelSettings *pSettings)
{
    unsigned index;
    bool found = false;
    NEXUS_PidChannelHandle pidChannel = NULL;
    NEXUS_P_HwPidChannel *hwPidChannel = NULL;
    unsigned lowestReservedPidChannel = NEXUS_PidChannel_GetBypassKeySlotIndex_isrsafe(NEXUS_BypassKeySlot_eMax-1);
    unsigned highestReservedPidChannel = NEXUS_NUM_DMA_CHANNELS + 1; /* +1 for SAGE-reserved channel */
    BSTD_UNUSED(pSettings);

    /* The search increments from highest to lowest in this way:
    highestReservedPidChannel refers to the SAGE reservation for XPT DMA pid filtering.
    lowestReservedPidChannel refers to the SAGE reservation for bypass keyslots.
    So there are 4 regions of pid channels:
    1. Parser band/playback pid channels (not XPT DMA)
    2. SAGE-reserved for XPT DMA pid filtering
    3. XPT DMA
    4. SAGE-reserved for bypass keyslot
    */
    for (index=highestReservedPidChannel; index<lowestReservedPidChannel; index++) {
        if (index >= BXPT_DMA_NUM_PID_CHANNELS) {
            BERR_TRACE(NEXUS_INVALID_PARAMETER);
            return NULL;
        }
        if (pTransport->hwDmaPidChannelRefCnt[index] == 0) {
            found = true;
            break;
        }
    }
    if (!found) {
        BDBG_ERR(("No more DMA pid channels available"));
        return NULL;
    }

    hwPidChannel = BKNI_Malloc(sizeof(*hwPidChannel));
    if (!hwPidChannel) {
        BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
        return NULL;
    }
    NEXUS_OBJECT_INIT(NEXUS_P_HwPidChannel, hwPidChannel);
    BLST_S_INIT(&hwPidChannel->swPidChannels);
    hwPidChannel->playpump = NULL;
    hwPidChannel->parserBand = NULL;
    hwPidChannel->status.pid = pid;
    hwPidChannel->status.pidChannelIndex = BXPT_P_MEMDMA_PID_CHANNEL_START + index;
    hwPidChannel->settingsPrivValid = false;
    pidChannel = NEXUS_PidChannel_P_Create(hwPidChannel);
    if (!pidChannel) {
        BKNI_Free(hwPidChannel);
    }
    pTransport->hwDmaPidChannelRefCnt[index]++;
    return pidChannel;
}

void NEXUS_PidChannel_CloseDma_Priv(NEXUS_PidChannelHandle pidChannel)
{
    unsigned index = pidChannel->hwPidChannel->status.pidChannelIndex-BXPT_P_MEMDMA_PID_CHANNEL_START;
    if (index >= BXPT_DMA_NUM_PID_CHANNELS) {
        BERR_TRACE(NEXUS_INVALID_PARAMETER);
        return;
    }
    if (pTransport->hwDmaPidChannelRefCnt[index]) {
        pTransport->hwDmaPidChannelRefCnt[index]--;
    }
    BKNI_Free(pidChannel->hwPidChannel);
    BKNI_Free(pidChannel);
}

void NEXUS_DmaJob_GetDefaultBlockDirectSettings( NEXUS_DmaJobBlockDirectSettings *pSettings)
{
    BDBG_ASSERT(NULL != pSettings);
    BXPT_Dma_Context_GetDefaultBlockSettings((BXPT_Dma_ContextBlockSettings*)pSettings);
    return;
}

NEXUS_Error NEXUS_DmaJob_ProcessBlocksDirect( NEXUS_DmaJobHandle handle, const NEXUS_DmaJobBlockDirectSettings *pSettings, unsigned nBlocks)
{
    NEXUS_Error errCode;
    const BXPT_Dma_ContextBlockSettings *blockSettings;

    NEXUS_ASSERT_STRUCTURE(NEXUS_DmaJobBlockDirectSettings, BXPT_Dma_ContextBlockSettings);
    NEXUS_ASSERT_FIELD(NEXUS_DmaJobBlockDirectSettings, srcOffset, BXPT_Dma_ContextBlockSettings, src);
    NEXUS_ASSERT_FIELD(NEXUS_DmaJobBlockDirectSettings, destOffset, BXPT_Dma_ContextBlockSettings, dst);
    NEXUS_ASSERT_FIELD(NEXUS_DmaJobBlockDirectSettings, blockSize, BXPT_Dma_ContextBlockSettings, size);
    NEXUS_ASSERT_FIELD(NEXUS_DmaJobBlockDirectSettings, resetCrypto, BXPT_Dma_ContextBlockSettings, resetCrypto);
    NEXUS_ASSERT_FIELD(NEXUS_DmaJobBlockDirectSettings, scatterGatherCryptoStart, BXPT_Dma_ContextBlockSettings, sgScramStart);
    NEXUS_ASSERT_FIELD(NEXUS_DmaJobBlockDirectSettings, scatterGatherCryptoEnd, BXPT_Dma_ContextBlockSettings, sgScramEnd);
    NEXUS_ASSERT_FIELD(NEXUS_DmaJobBlockDirectSettings, securityBtp, BXPT_Dma_ContextBlockSettings, securityBtp);

    if (handle->state != NEXUS_DmaJob_P_StateIdle) {
        return BERR_TRACE(NEXUS_NOT_AVAILABLE);
    }
    if(!NEXUS_P_CpuAccessibleAddress(pSettings)) {
        return BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }
    blockSettings =  (void *)pSettings;
    BDBG_MSG(("  started job:%p nBlocks:%u", (void *)handle, nBlocks));

    handle->flushAfter = false; /* pSettings->cached is ignored */

    handle->state = NEXUS_DmaJob_P_StateQueued;

    errCode = BXPT_Dma_Context_Enqueue(handle->ctx, blockSettings, nBlocks); /* ISR callback may fire before this function returns, which is fine */
    switch (errCode) {
        case BERR_SUCCESS: /* completed */
            handle->state = NEXUS_DmaJob_P_StateIdle;
            goto done;
        case BXPT_DMA_QUEUED:
            errCode = NEXUS_DMA_QUEUED;
            break;
        default:
            handle->state = NEXUS_DmaJob_P_StateIdle;
            errCode = BERR_TRACE(errCode);
            goto done;
    }

    BDBG_MSG(("   queued job:%p", (void *)handle));
done:
    return errCode;
}
