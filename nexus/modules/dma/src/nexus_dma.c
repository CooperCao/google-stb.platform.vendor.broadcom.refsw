/***************************************************************************
 *  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
 * Module Description:
 *
 **************************************************************************/
#include "nexus_dma_module.h"
#include "nexus_base.h"
#include "blst_squeue.h"
#include "nexus_client_resources.h"
#include "priv/nexus_core_security.h"

#if NEXUS_NUM_SHARF_DMA_CHANNELS && (!NEXUS_NUM_DMA_CHANNELS)
#define NEXUS_SHARF_DMA_KEY_REQUIRES_DESCRIPTOR 1 /* requires key and IV to be in a separate descriptor */
#endif

BDBG_MODULE(nexus_dma);
#define BDBG_MSG_TRACE(x) /* BDBG_MSG(x) */

typedef struct NEXUS_DmaJob
{
    NEXUS_OBJECT(NEXUS_DmaJob);
    BLST_S_ENTRY(NEXUS_DmaJob) jobNode;
    BLST_SQ_ENTRY(NEXUS_DmaJob) activeNode;

    BMMD_ContextHandle ctx;
    NEXUS_DmaHandle parent;
    NEXUS_IsrCallbackHandle completionCallback_isr;
    enum {
        NEXUS_DmaJob_P_StateIdle,
        NEXUS_DmaJob_P_StateQueued /* job is queued in activeNode */
    } state;
    BKNI_EventHandle nexusEvent; /* event that is used to notify nexus module */
    NEXUS_DmaJobSettings settings;
    unsigned numBlocks;
    bool flushAfter; /* true if ANY NEXUS_DmaJobBlockSettings[].cached is true. i.e. flushAfter is per-job, while .cached is per-jobBlock */
    BMMD_ContextBlockSettings blockSettings[1]; /* variable size array, must be last entry in the structure */
} NEXUS_DmaJob;

typedef struct NEXUS_Dma
{
    NEXUS_OBJECT(NEXUS_Dma);
    BLST_S_ENTRY(NEXUS_Dma) link;
    unsigned pubIndex; /* this is the index that NEXUS_Dma_Open was called with */
    unsigned privIndex; /* this is the internal index. on platforms that have both MEM_DMA and SHARF_DMA cores,
                           the MEM_DMA cores are numbered first from 0, then followed by SHARF_DMA cores */
    NEXUS_DmaSettings settings;
    BLST_S_HEAD(JobList, NEXUS_DmaJob) jobList; /* this keeps track of the opened BMMD_Context's */
} NEXUS_Dma;

static void NEXUS_Dma_P_CompleteCallback_isr(void *pParam, int iParam);


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
    BMMD_ContextSettings ctxSettings;
    BERR_Code rc;

    BDBG_OBJECT_ASSERT(handle, NEXUS_Dma);
    BDBG_ASSERT(pSettings);

    BDBG_CASSERT(BMMD_EndianMode_eMax==(BMMD_EndianMode)NEXUS_DmaEndianMode_eMax);
    BDBG_CASSERT(BMMD_SwapMode_eMax==(BMMD_SwapMode)NEXUS_DmaSwapMode_eMax);

    /* apply settings to all MMD contexts */
    for (pJob=BLST_S_FIRST(&handle->jobList); pJob; pJob=BLST_S_NEXT(pJob, jobNode)) {
        BMMD_Context_GetSettings(pJob->ctx, &ctxSettings);
        ctxSettings.endianMode = pSettings->endianMode;
        ctxSettings.swapMode = pSettings->swapMode;
        rc = BMMD_Context_SetSettings(pJob->ctx, &ctxSettings);
        if (rc!=BERR_SUCCESS) {
            rc = BERR_TRACE(rc);
            return NEXUS_INVALID_PARAMETER;
        }
    }

    BKNI_Memcpy(&handle->settings, pSettings, sizeof(*pSettings));
    return NEXUS_SUCCESS;
}


static void NEXUS_Dma_P_Finalizer( NEXUS_DmaHandle handle)
{
    NEXUS_OBJECT_ASSERT(NEXUS_Dma, handle);

    BLST_S_REMOVE(&g_NEXUS_dmaModuleData.channel[handle->privIndex].dmaHandles, handle, NEXUS_Dma, link);
    NEXUS_OBJECT_DESTROY(NEXUS_Dma, handle);
    BKNI_Free(handle);
}

static void NEXUS_Dma_P_Release(NEXUS_DmaHandle handle)
{
    NEXUS_DmaJob *pJob;

    NEXUS_OBJECT_ASSERT(NEXUS_Dma, handle);
    NEXUS_OBJECT_UNREGISTER(NEXUS_Dma, handle, Destroy);
    NEXUS_CLIENT_RESOURCES_RELEASE(dma,Count,NEXUS_ANY_ID);

    /* forcefully close jobs, this among other things would release reference counter for NEXUS_Dma, so NEXUS_Dma_Close would get called */
    while ((pJob = BLST_S_FIRST(&handle->jobList))) {
        BDBG_WRN(("NEXUS_Dma_Close: stale job %#lx", (unsigned long)pJob));
        NEXUS_DmaJob_Destroy(pJob);
    }
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

    if (index == NEXUS_ANY_ID) {
        index = 0;
    }
    if (!pSettings) {
        NEXUS_Dma_GetDefaultSettings(&settings);
        pSettings = &settings;
    }

    if (pSettings->coreType>=NEXUS_DmaCoreType_eMax) {
        rc = BERR_TRACE(NEXUS_INVALID_PARAMETER);
        return NULL;
    }
    if (pSettings->coreType==NEXUS_DmaCoreType_eM2m && (int)index>=NEXUS_NUM_DMA_CHANNELS) { /* the cast to int suppresses a benign compiler warning */
        BDBG_ERR(("DMA channel %d is not supported on this chipset", index));
        rc = BERR_TRACE(NEXUS_INVALID_PARAMETER);
        return NULL;
    }
    if (pSettings->coreType==NEXUS_DmaCoreType_eSharf && (int)index>=NEXUS_NUM_SHARF_DMA_CHANNELS) {
        BDBG_ERR(("SHARF DMA channel %d is not supported on this chipset", index));
        rc = BERR_TRACE(NEXUS_INVALID_PARAMETER);
        return NULL;
    }
    rc = NEXUS_CLIENT_RESOURCES_ACQUIRE(dma,Count,NEXUS_ANY_ID);
    if (rc) { rc = BERR_TRACE(rc); return NULL; }

    dma = BKNI_Malloc(sizeof(*dma));
    if (!dma) {
        NEXUS_CLIENT_RESOURCES_RELEASE(dma,Count,NEXUS_ANY_ID);
        rc = BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
        return NULL;
    }

    NEXUS_OBJECT_INIT(NEXUS_Dma, dma);

    dma->pubIndex = index;
    dma->privIndex = pSettings->coreType==NEXUS_DmaCoreType_eM2m ? index : index+NEXUS_NUM_DMA_CHANNELS;

    BLST_S_INIT(&dma->jobList);
    BLST_S_INSERT_HEAD(&g_NEXUS_dmaModuleData.channel[dma->privIndex].dmaHandles, dma, link);
    BKNI_Memcpy(&dma->settings, pSettings, sizeof(*pSettings));
    NEXUS_OBJECT_REGISTER(NEXUS_Dma, dma, Create);

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
    const NEXUS_DmaJobSettings *pSettings
    )
{
    NEXUS_Error rc;
    BMMD_ContextSettings ctxSettings;

    if (pSettings->numBlocks != handle->settings.numBlocks) {
        rc = BERR_TRACE(NEXUS_INVALID_PARAMETER);
        goto error;
    }

    BMMD_Context_GetSettings(handle->ctx, &ctxSettings);
    ctxSettings.endianMode = handle->parent->settings.endianMode;
    ctxSettings.swapMode = handle->parent->settings.swapMode;
    ctxSettings.callback_isr = NEXUS_Dma_P_CompleteCallback_isr;
    ctxSettings.pParm1 = handle;
    /* BMMD will ignore sharf settings if not needed */
    BDBG_CASSERT(BMMD_SharfMode_eMax==(BMMD_SharfMode)NEXUS_DmaSharfMode_eMax);
    ctxSettings.sharf.useBspKey = pSettings->sharf.useBspKey;
    ctxSettings.sharf.mode = pSettings->sharf.mode;
    ctxSettings.sharf.shaContext = pSettings->sharf.shaContext;

    if (pSettings->keySlot == NULL) {
        ctxSettings.scramMode = BMMD_ScramMode_eNone;
        ctxSettings.keyslot = 0; /* don't care */
    }
    else {
        NEXUS_SecurityKeySlotInfo keySlotInfo;

        switch (pSettings->dataFormat) {
            default:
            case NEXUS_DmaDataFormat_eBlock:
                ctxSettings.scramMode = BMMD_ScramMode_eBlock;
                break;
            case NEXUS_DmaDataFormat_eMpeg:
                ctxSettings.scramMode = BMMD_ScramMode_eMpeg;
                break;
            case NEXUS_DmaDataFormat_eDss:
                ctxSettings.scramMode = BMMD_ScramMode_eDss;
                break;
        }
        NEXUS_KeySlot_GetInfo(pSettings->keySlot, &keySlotInfo);
        if (keySlotInfo.keySlotEngine!=NEXUS_SecurityEngine_eM2m) {
            rc = BERR_TRACE(BERR_INVALID_PARAMETER);
            goto error;
        }
        ctxSettings.keyslot = keySlotInfo.keySlotNumber;
    }
    rc = BMMD_Context_SetSettings(handle->ctx, &ctxSettings);
    if (rc!=BERR_SUCCESS) {
        rc = NEXUS_UNKNOWN;
        goto error;
    }

    NEXUS_IsrCallback_Set(handle->completionCallback_isr, &pSettings->completionCallback);
    handle->settings = *pSettings;
    return NEXUS_SUCCESS;

error:
    return rc;
}

static void NEXUS_DmaJob_P_Release(NEXUS_DmaJobHandle handle)
{
    NEXUS_OBJECT_ASSERT(NEXUS_DmaJob, handle);
    NEXUS_OBJECT_UNREGISTER(NEXUS_DmaJob, handle, Destroy);
}

static void NEXUS_DmaJob_P_Finalizer(NEXUS_DmaJobHandle handle)
{
    NEXUS_OBJECT_ASSERT(NEXUS_DmaJob, handle);

    BMMD_Context_Destroy(handle->ctx);
    /* the assumption is that after BMMD_Context_Destroy() has been called, the ISR callback will not fire */
    BLST_S_REMOVE(&handle->parent->jobList, handle, NEXUS_DmaJob, jobNode);
    NEXUS_OBJECT_RELEASE(handle, NEXUS_Dma, handle->parent);
    NEXUS_IsrCallback_Destroy(handle->completionCallback_isr);
    NEXUS_OBJECT_DESTROY(NEXUS_DmaJob, handle);
    BKNI_Free(handle);
    return;
}

NEXUS_OBJECT_CLASS_MAKE_WITH_RELEASE(NEXUS_DmaJob, NEXUS_DmaJob_Destroy);

NEXUS_DmaJobHandle NEXUS_DmaJob_Create(
    NEXUS_DmaHandle dmaHandle,
    const NEXUS_DmaJobSettings *pSettings
    )
{
    NEXUS_DmaJob *pJob;
    NEXUS_DmaJobSettings jobSettings;
    NEXUS_Error rc;
    BMMD_ContextSettings ctxSettings;
    NEXUS_HeapHandle heap;

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

    pJob = BKNI_Malloc(sizeof(*pJob)+sizeof(BMMD_ContextBlockSettings)*(pSettings->numBlocks+1)); /* +1 for NEXUS_SHARF_DMA_KEY_REQUIRES_DESCRIPTOR */
    if (NULL == pJob) {
        rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        return NULL;
    }

    BKNI_Memset(pJob, 0, sizeof(*pJob)+sizeof(BMMD_ContextBlockSettings)*(pSettings->numBlocks+1));
    NEXUS_OBJECT_INIT(NEXUS_DmaJob, pJob);

    pJob->nexusEvent = NULL;
    pJob->parent = dmaHandle;
    pJob->settings = *pSettings;
    pJob->completionCallback_isr = NEXUS_IsrCallback_Create(pJob, NULL);

    /* create the context with maxNumBlocks and memoryBounds. change the settings afterwards */
    BMMD_Context_GetDefaultSettings(&ctxSettings);
    ctxSettings.maxNumBlocks = pSettings->numBlocks+1;
    heap = NEXUS_P_DefaultHeap(dmaHandle->settings.boundsHeap, NEXUS_DefaultHeapType_eBounds);
    if (heap) {
        NEXUS_MemoryStatus status;
        rc = NEXUS_Heap_GetStatus(heap, &status);
        if (rc) {rc = BERR_TRACE(rc); goto error;}
        ctxSettings.memoryBounds.offset = status.offset;
        ctxSettings.memoryBounds.size = status.size;
    }
    pJob->ctx = BMMD_Context_Create(g_NEXUS_dmaModuleData.channel[dmaHandle->privIndex].mmd, &ctxSettings);
    if (!pJob->ctx) {
        goto error;
    }

    rc = NEXUS_Dma_P_ApplyJobSettings(pJob, pSettings);
    if (rc!=NEXUS_SUCCESS) {
        BMMD_Context_Destroy(pJob->ctx);
        goto error;
    }

    NEXUS_OBJECT_REGISTER(NEXUS_DmaJob, pJob, Create);
    NEXUS_OBJECT_ACQUIRE(pJob, NEXUS_Dma, dmaHandle); /* acquire parent when job added, so if job is used inside nexus it's parent wouldn't get released */

    BLST_S_INSERT_HEAD(&dmaHandle->jobList, pJob, jobNode);
    return pJob;

error:
    NEXUS_IsrCallback_Destroy(pJob->completionCallback_isr);
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
    return NEXUS_Dma_P_ApplyJobSettings(handle, pSettings);
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
    BMMD_ContextStatus ctxStatus;

    BDBG_OBJECT_ASSERT(handle, NEXUS_DmaJob);
    BDBG_ASSERT(NULL != pStatus);
    BDBG_MSG_TRACE(("NEXUS_DmaJob_GetStatus>:%#lx", (unsigned long)handle));

    errCode = BMMD_Context_GetStatus(handle->ctx, &ctxStatus);
    if (errCode) {
        pStatus->currentState = NEXUS_DmaJobState_eUnknown;
        return BERR_TRACE(errCode);
    }

    switch (ctxStatus.state) {
        case BMMD_ContextState_eIdle:
            pStatus->currentState = NEXUS_DmaJobState_eComplete; /* in MMD, eIdle may mean eComplete, but Nexus users are expecting eComplete */
            break;
        case BMMD_ContextState_eInProgress:
            pStatus->currentState = NEXUS_DmaJobState_eInProgress;
            break;
        default:
            pStatus->currentState = NEXUS_DmaJobState_eUnknown;
            break;
    }

    BDBG_MSG_TRACE(("NEXUS_DmaJob_GetStatus<:%#lx", (unsigned long)handle));
    return BERR_SUCCESS;
}

static void NEXUS_Dma_P_CompleteCallback_isr(void *pParam, int iParam)
{
    unsigned i;
    NEXUS_DmaJobHandle handle = (NEXUS_DmaJobHandle)pParam;

    BDBG_OBJECT_ASSERT(handle, NEXUS_DmaJob);
    BSTD_UNUSED(iParam);

    BDBG_MSG(("completed job:%#lx", (unsigned long)handle));
    handle->state = NEXUS_DmaJob_P_StateIdle;

    if (handle->flushAfter) {
        BMMD_ContextBlockSettings *pSettings;
        pSettings = handle->blockSettings;
        for (i=0, pSettings = handle->blockSettings; i<handle->numBlocks; i++, pSettings++) {
            NEXUS_FlushCache_isr(pSettings->dstPtr, pSettings->size);
        }
        handle->flushAfter = false;
    }

    if(handle->nexusEvent) {
        BKNI_SetEvent_isr(handle->nexusEvent);
    }
    else {
        NEXUS_IsrCallback_Fire_isr(handle->completionCallback_isr);
    }
}

#if NEXUS_SHARF_DMA_KEY_REQUIRES_DESCRIPTOR
unsigned NEXUS_DmaJob_P_KeySize(NEXUS_DmaSharfMode mode) {
    unsigned rv = 0;

    switch (mode) {
    case NEXUS_DmaSharfMode_eSha1:
        rv = 16;
        break;
    case NEXUS_DmaSharfMode_eAes128CbcDecrypt:
    case NEXUS_DmaSharfMode_eAes128CbcEncrypt:
    case NEXUS_DmaSharfMode_eAes128EcbDecrypt:
    case NEXUS_DmaSharfMode_eAes128EcbEncrypt:
    case NEXUS_DmaSharfMode_eCmac:
        rv = 16;
        break;
    default:
        BERR_TRACE(NEXUS_INVALID_PARAMETER);
        break;
    }
    return rv;
}

unsigned NEXUS_DmaJob_P_IVSize(NEXUS_DmaSharfMode mode) {
    unsigned rv = 0;

    switch (mode) {
    case NEXUS_DmaSharfMode_eSha1:
        rv = 16;
        break;
    case NEXUS_DmaSharfMode_eAes128CbcDecrypt:
    case NEXUS_DmaSharfMode_eAes128CbcEncrypt:
    case NEXUS_DmaSharfMode_eAes128EcbDecrypt:
    case NEXUS_DmaSharfMode_eAes128EcbEncrypt:
    case NEXUS_DmaSharfMode_eCmac:
        rv = 16;
        break;
    default:
        BERR_TRACE(NEXUS_INVALID_PARAMETER);
        break;
    }
    return rv;
}
 #endif /* NEXUS_SHARF_DMA_KEY_REQUIRES_DESCRIPTOR */


static NEXUS_Error
NEXUS_DmaJob_P_ProcessBlocks(NEXUS_DmaJobHandle handle, const NEXUS_DmaJobBlockSettings *pSettings, unsigned nBlocks)
{
    NEXUS_Error errCode;
    BMMD_ContextBlockSettings *blockSettings = handle->blockSettings;
    uint32_t srcOffset, dstOffset;
    unsigned i = 0, dmaLength = 0;

    if (nBlocks > handle->settings.numBlocks) {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    handle->numBlocks = nBlocks;

    BDBG_MSG(("  started job:%#lx nBlocks:%u", (unsigned long)handle, nBlocks));

    for (i=0; i<nBlocks; i++,pSettings++) {
        /* convert to offset */
        srcOffset = NEXUS_AddrToOffset((void *)pSettings->pSrcAddr);
        if (0 == srcOffset) {
            BDBG_ERR(("Invalid Source Address: [%u]%#lx", i, (unsigned long)pSettings->pSrcAddr));
            return BERR_TRACE(BERR_INVALID_PARAMETER);
        }
        if (pSettings->cached) {
            if (!NEXUS_P_CpuAccessibleAddress(pSettings->pSrcAddr)) {
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
                BDBG_ERR(("Invalid Destination Address: [%u]%#lx", i, (unsigned long)pSettings->pDestAddr));
                return BERR_TRACE(BERR_INVALID_PARAMETER);
            }
            if (pSettings->cached) {
                if (!NEXUS_P_CpuAccessibleAddress(pSettings->pDestAddr)) {
                    return BERR_TRACE(BERR_INVALID_PARAMETER);
                }
                NEXUS_FlushCache(pSettings->pDestAddr, pSettings->blockSize);
            }
        }
        handle->flushAfter = pSettings->cached;

        BMMD_Context_GetDefaultBlockSettings(&blockSettings[i]);
        blockSettings[i].src = srcOffset;
        blockSettings[i].dst = dstOffset;
        blockSettings[i].size = pSettings->blockSize;
        blockSettings[i].resetCrypto = pSettings->resetCrypto;
        blockSettings[i].sgScramStart = pSettings->scatterGatherCryptoStart;
        blockSettings[i].sgScramEnd = pSettings->scatterGatherCryptoEnd;
        blockSettings[i].srcPtr = pSettings->pSrcAddr;
        blockSettings[i].dstPtr = pSettings->pDestAddr;
        /* BMMD will ignore sharf settings if not needed */
        blockSettings[i].sharf.keyPresent = pSettings->sharf.keyPresent;
        blockSettings[i].sharf.digestPresent = pSettings->sharf.digestPresent;
        dmaLength += pSettings->blockSize;
    }

#if NEXUS_SHARF_DMA_KEY_REQUIRES_DESCRIPTOR
{
    unsigned dataOffset = 0; /* used to track how much key/IV needs to be peeled off the first descriptor */
    int j;

    /* this is a fixup of the first descriptor for hardware (such as 7550) which requires that a provided
       key and IV are in their own, initial descriptor. if the caller does not pass in the data blocks
       in the correct format, this prepends a block to hold the key and iv and performs the
       necessary fixup */
    pSettings -= nBlocks;

    if (pSettings[0].sharf.keyPresent) {
        dataOffset += NEXUS_DmaJob_P_KeySize(handle->settings.sharf.mode);
    }
    if (pSettings[0].sharf.digestPresent) {
        dataOffset += NEXUS_DmaJob_P_IVSize(handle->settings.sharf.mode);
    }
    /* if data and key or IV are combined in the first block, perform the fixup */
    if (dataOffset>0 && pSettings[0].blockSize>dataOffset) {
        for (j=nBlocks; j>=0 ; j--) {
            blockSettings[j+1] = blockSettings[j];
        }
        blockSettings[0].size = dataOffset;
        blockSettings[1].size -= dataOffset;
        blockSettings[1].src += dataOffset;
        blockSettings[1].sgScramStart = false;
        blockSettings[1].sharf.keyPresent = false;
        blockSettings[1].sharf.digestPresent = false;
        nBlocks++;
    }
}
#endif /* NEXUS_SHARF_DMA_KEY_REQUIRES_DESCRIPTOR */

    handle->state = NEXUS_DmaJob_P_StateQueued;
    errCode = BMMD_Context_Enqueue(handle->ctx, blockSettings, nBlocks); /* ISR callback may fire before this function returns, which is fine */
    switch (errCode) {
        case BERR_SUCCESS: /* completed */
            handle->state = NEXUS_DmaJob_P_StateIdle;
            goto completed;
            break;
        case BMMD_QUEUED:
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
    BMMD_ContextBlockSettings *blockSettings = handle->blockSettings;
    uint32_t srcOffset, dstOffset;
    unsigned i, dmaLength = 0;

    BDBG_MSG(("  started job:%#lx nBlocks:%u", (unsigned long)handle, nBlocks));

    for (i=0; i<nBlocks; i++,pSettings++) {
        srcOffset = pSettings->srcOffset;
        dstOffset = pSettings->destOffset;

        BMMD_Context_GetDefaultBlockSettings(&blockSettings[i]);
        blockSettings[i].src = srcOffset;
        blockSettings[i].dst = dstOffset;
        blockSettings[i].size = pSettings->blockSize;
        blockSettings[i].resetCrypto = pSettings->resetCrypto;
        blockSettings[i].sgScramStart = pSettings->scatterGatherCryptoStart;
        blockSettings[i].sgScramEnd = pSettings->scatterGatherCryptoEnd;

        /* BMMD will ignore sharf settings if not needed */
        blockSettings[i].sharf.keyPresent = pSettings->sharf.keyPresent;
        blockSettings[i].sharf.digestPresent = pSettings->sharf.digestPresent;
        dmaLength += pSettings->blockSize;
    }
    handle->flushAfter = false; /* pSettings->cached is ignored */

    handle->state = NEXUS_DmaJob_P_StateQueued;
    errCode = BMMD_Context_Enqueue(handle->ctx, blockSettings, nBlocks); /* ISR callback may fire before this function returns, which is fine */
    switch (errCode) {
        case BERR_SUCCESS: /* completed */
            handle->state = NEXUS_DmaJob_P_StateIdle;
            goto completed;
            break;
        case BMMD_QUEUED:
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
NEXUS_DmaJob_ProcessBlocks_priv(NEXUS_DmaJobHandle handle, const NEXUS_DmaJobBlockSettings *pSettings, unsigned nBlocks, BKNI_EventHandle event)
{
    NEXUS_ASSERT_MODULE();
    BDBG_OBJECT_ASSERT(handle, NEXUS_DmaJob);
    BDBG_ASSERT(NULL != pSettings);

    handle->nexusEvent = event;
    return NEXUS_DmaJob_P_ProcessBlocks(handle, pSettings, nBlocks);
}

