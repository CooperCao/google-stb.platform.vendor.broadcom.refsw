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
*
* API Description:
*   API name: FMT
*    Audio Format Related Routines
*
***************************************************************************/
#include "bape_dma_output_priv.h"
#include "bott_dma.h"

BDBG_MODULE(bape_dma_output_priv);

typedef struct DmaJob {
    BOTT_Dma_ContextHandle hCtx;
    unsigned consumed;
    BLST_SQ_ENTRY(DmaJob) link;
} DmaJob;

typedef struct BAPE_DmaOutput {
    BAPE_Handle hDevice;
    BOTT_Dma_Handle hOttDma;
    unsigned nJobs;
    DmaJob *job;
    BLST_SQ_HEAD(ActiveJobList, DmaJob) activeJobList;
    BLST_SQ_HEAD(FreeJobList, DmaJob) freeJobList;
    BAPE_BufferGroupHandle hUpstreamBG;
    BAPE_BufferGroupHandle hBG;
    BAPE_BufferGroupFilterHandle hFilter;
    unsigned maxRequestSize;
    bool enabled;
    unsigned peeked;
    bool alreadySubmitting;
} BAPE_DmaOutput;

#if 0
static BAPE_DmaOutputHandle g_hDma;
#endif

static BERR_Code BAPE_DmaOutput_P_Enable_isr(BAPE_DmaOutputHandle hDma, bool enable);
static void BAPE_DmaOutput_P_DataReady_isr(void *pParam1, int param2);

static BERR_Code
BAPE_DmaOutput_P_Read_isr(BAPE_DmaOutputHandle hDma, BAPE_BufferDescriptor *pDesc)
{
    BERR_Code rc;
    if (hDma->hFilter) {
        rc = BAPE_BufferGroupFilter_P_Read_isr(hDma->hFilter, pDesc);
    }
    else {
        rc = BAPE_BufferGroup_Read_isr(hDma->hBG, pDesc);
    }
    if (rc != BERR_SUCCESS) {
        return BERR_TRACE(rc);
    }
    if (hDma->peeked) {
        if (hDma->peeked < pDesc->bufferSize) {
            pDesc->bufferSize -= hDma->peeked;
            pDesc->buffers[0].offset += hDma->peeked;
            pDesc->buffers[0].pBuffer = (uint8_t *)pDesc->buffers[0].pBuffer + hDma->peeked;
        }
        else if (hDma->peeked == pDesc->bufferSize + pDesc->wrapBufferSize) {
            pDesc->bufferSize = pDesc->wrapBufferSize = 0;
        }
        else {
            unsigned remaining;
            unsigned wrapAdvance;
            BDBG_ASSERT(hDma->peeked >= pDesc->bufferSize);
            BDBG_ASSERT(pDesc->bufferSize + pDesc->wrapBufferSize > hDma->peeked);
            remaining = pDesc->bufferSize + pDesc->wrapBufferSize - hDma->peeked;
            BDBG_ASSERT(pDesc->wrapBufferSize >= remaining);
            wrapAdvance = pDesc->wrapBufferSize - remaining;
            pDesc->buffers[0].offset = pDesc->buffers[0].wrapOffset + wrapAdvance;
            pDesc->buffers[0].pBuffer = (uint8_t *)pDesc->buffers[0].pWrapBuffer + wrapAdvance;
            pDesc->bufferSize = remaining;
            pDesc->wrapBufferSize = 0;
        }
    }
    return BERR_SUCCESS;
}

static BERR_Code
BAPE_DmaOutput_P_ReallySubmit_isr(BAPE_DmaOutputHandle hDma)
{
    BAPE_BufferDescriptor desc;
    BOTT_Dma_ContextBlockSettings blk[2];
    unsigned nBlks;
    BERR_Code rc;

    for (;;) {
        DmaJob *job = BLST_SQ_FIRST(&hDma->freeJobList);
        if (job == NULL) {
            BDBG_MSG(("%s: no free jobs", BSTD_FUNCTION));
            return BERR_SUCCESS;
        }
        rc = BAPE_DmaOutput_P_Read_isr(hDma, &desc);
        if (rc != BERR_SUCCESS) {
            return BERR_TRACE(rc);
        }
        if (desc.bufferSize == 0) {
            BDBG_MSG(("%s: ringbuffer empty", BSTD_FUNCTION));
            return BERR_SUCCESS;
        }
        if (desc.bufferSize >= hDma->maxRequestSize) {
            desc.bufferSize = hDma->maxRequestSize;
            desc.wrapBufferSize = 0;
        }
        else if (desc.bufferSize + desc.wrapBufferSize > hDma->maxRequestSize) {
            desc.wrapBufferSize = hDma->maxRequestSize - desc.bufferSize;
        }
        BLST_SQ_REMOVE_HEAD(&hDma->freeJobList, link);
        BOTT_Dma_Context_GetDefaultBlockSettings(&blk[0]);
        BMMA_FlushCache_isrsafe(desc.buffers[0].block, desc.buffers[0].pBuffer, desc.bufferSize);
        blk[0].src = desc.buffers[0].offset;
        blk[0].perMap = 10;
        blk[0].dstMode.reg = true;
        blk[0].dstMode.size = BOTT_Dma_Size_e32;
        blk[0].dst = BCHP_DVP_CFG_MAI0_DAT;
        blk[0].dstMode.dreq = true;
        blk[0].dstMode.inc = false;
        blk[0].length.x = desc.bufferSize;
        if (desc.wrapBufferSize) {
            blk[1] = blk[0];
            BMMA_FlushCache_isrsafe(desc.buffers[0].block, desc.buffers[0].pWrapBuffer, desc.wrapBufferSize);
            blk[1].src = desc.buffers[0].wrapOffset;
            blk[1].length.x = desc.wrapBufferSize;
            nBlks = 2;
        }
        else {
            nBlks = 1;
        }
        BDBG_MSG(("%s(%d): enqueue %d/%d", BSTD_FUNCTION, (unsigned)(job - hDma->job), desc.bufferSize, desc.wrapBufferSize));
        rc = BOTT_Dma_Context_Enqueue_isr(job->hCtx, blk, nBlks);
        if (rc != BOTT_DMA_QUEUED) {
            BLST_SQ_INSERT_TAIL(&hDma->freeJobList, job, link);
            return BERR_TRACE(rc);
        }
        job->consumed = desc.bufferSize + desc.wrapBufferSize;
        hDma->peeked += job->consumed;
        BLST_SQ_INSERT_TAIL(&hDma->activeJobList, job, link);
        return BERR_SUCCESS;
    }
}

static BERR_Code
BAPE_DmaOutput_P_Submit_isr(BAPE_DmaOutputHandle hDma)
{
    if (!hDma->alreadySubmitting) {
        BERR_Code rc;
        hDma->alreadySubmitting = true;
        rc = BAPE_DmaOutput_P_ReallySubmit_isr(hDma);
        hDma->alreadySubmitting = false;
        return rc;
    }
    BDBG_MSG(("%s: recursion averted", BSTD_FUNCTION));
    return BERR_SUCCESS;
}

static void
BAPE_DmaOutput_P_Complete_isr(void *pParm1, int iParm2)
{
    BAPE_DmaOutputHandle hDma = pParm1;
    DmaJob *job = BLST_SQ_FIRST(&hDma->activeJobList);
    BDBG_ASSERT(job == hDma->job + iParm2);
    BLST_SQ_REMOVE_HEAD(&hDma->activeJobList, link);
    hDma->peeked -= job->consumed;
    if (hDma->hFilter) {
        BAPE_BufferGroupFilter_P_ReadComplete_isr(hDma->hFilter, job->consumed);
    }
    else {
        BAPE_BufferGroup_ReadComplete_isr(hDma->hBG, job->consumed);
    }
    BLST_SQ_INSERT_TAIL(&hDma->freeJobList, job, link);
    BDBG_MSG(("%s(%d): consumed %d", BSTD_FUNCTION, iParm2, job->consumed));
    BAPE_DmaOutput_P_Submit_isr(hDma);
}

BAPE_DmaOutputHandle
BAPE_DmaOutput_P_Open(BAPE_Handle hDevice)
{
    BOTT_Dma_Settings settings;
    BAPE_DmaOutput *hDma;
    BERR_Code rc;

    hDma = BKNI_Malloc(sizeof(*hDma));
    if (hDma == 0) {
        return NULL;
    }
    BKNI_Memset(hDma, 0, sizeof(*hDma));
    BOTT_Dma_GetDefaultSettings(&settings);
    settings.engineIndex = 11;
    rc = BOTT_Dma_Open(hDevice->chpHandle, hDevice->regHandle, hDevice->memHandle, hDevice->intHandle, &settings, &hDma->hOttDma);
    if (rc != BERR_SUCCESS) {
        BKNI_Free(hDma);
        return NULL;
    }
    hDma->hDevice = hDevice;
#if 0
    g_hDma = hDma;
#endif
    return hDma;
}

BERR_Code
BAPE_DmaOutput_P_UnBind(BAPE_DmaOutputHandle hDma)
{
    BERR_Code rc = BAPE_DmaOutput_P_Enable(hDma, false);
    if (rc != BERR_SUCCESS) {
        return BERR_TRACE(rc);
    }
    if (hDma->job) {
        unsigned i;
        BLST_SQ_INIT(&hDma->freeJobList);
        for (i = 0; i < hDma->nJobs; i++) {
            if (hDma->job[i].hCtx) {
                BOTT_Dma_Context_Destroy(hDma->job[i].hCtx);
            }
        }
        BKNI_Free(hDma->job);
        hDma->job = NULL;
        hDma->nJobs = 0;
    }
    if (hDma->hUpstreamBG) {
        BAPE_BufferGroup_UnLinkOutput(hDma->hUpstreamBG, hDma->hBG);
        hDma->hUpstreamBG = NULL;
    }
    if (hDma->hBG) {
        BAPE_BufferGroup_Close(hDma->hBG);
        hDma->hBG = NULL;
    }
    if (hDma->hFilter) {
        BAPE_BufferGroupFilter_P_Close(hDma->hFilter);
        hDma->hFilter = NULL;
    }
    return BERR_SUCCESS;
}

void
BAPE_DmaOutput_P_Close(BAPE_DmaOutputHandle hDma)
{
    BOTT_Dma_Close(hDma->hOttDma);
    BKNI_Free(hDma);
}

BERR_Code
BAPE_DmaOutput_P_Bind(BAPE_DmaOutputHandle hDma, BAPE_BufferGroupHandle hUpstreamBG, BAPE_BufferGroupFilterFn *func, void *ctx)
{
    BAPE_BufferGroupOpenSettings settings;
    BERR_Code rc;
    unsigned i;
    BAPE_BufferGroupInterruptHandlers interrupts;
    BAPE_BufferGroupStatus status;

    if (hDma->hUpstreamBG && hDma->hUpstreamBG != hUpstreamBG) {
        rc = BAPE_DmaOutput_P_UnBind(hDma);
        if (rc != BERR_SUCCESS) {
            return BERR_TRACE(rc);
        }
    }
    if (hDma->hUpstreamBG != NULL) {
        return BERR_SUCCESS;
    }
    BAPE_BufferGroup_GetStatus_isrsafe(hUpstreamBG, &status);
    BAPE_BufferGroup_GetDefaultOpenSettings(&settings);

    BDBG_WRN(("upstream interleaved %d numChannels %d bufferSize %d", status.interleaved, status.numChannels, (unsigned)status.bufferSize));

    settings.interleaved = status.interleaved;
    settings.numChannels = 1;
    settings.bufferSize = status.bufferSize;
    settings.bufferless = true;
    rc = BAPE_BufferGroup_Open(hDma->hDevice, &settings, &hDma->hBG);
    if (rc != BERR_SUCCESS) {
        goto error;
    }
    rc = BAPE_BufferGroup_LinkOutput(hUpstreamBG, hDma->hBG);
    if (rc != BERR_SUCCESS) {
        goto error;
    }
    hDma->hUpstreamBG = hUpstreamBG;
    hDma->maxRequestSize = status.bufferSize / 3;
    BAPE_BufferGroup_GetInterruptHandlers(hDma->hBG, &interrupts);
    interrupts.dataReady.pCallback_isr = BAPE_DmaOutput_P_DataReady_isr;
    interrupts.dataReady.pParam1 = hDma;
    BAPE_BufferGroup_SetInterruptHandlers(hDma->hBG, &interrupts);
    if (func) {
        hDma->hFilter = BAPE_BufferGroupFilter_P_Open(hDma->hDevice, hDma->hBG, status.bufferSize, func, ctx);
    }
    /* Create enough jobs */
    hDma->nJobs = 4;
    hDma->job = BKNI_Malloc(sizeof(DmaJob) * hDma->nJobs);
    if (hDma->job == NULL) {
        goto error;
    }
    BKNI_Memset(hDma->job, 0, sizeof(DmaJob) * hDma->nJobs);
    for (i = 0; i < hDma->nJobs; i++) {
        BOTT_Dma_ContextSettings settings;
        settings.maxNumBlocks = 2;
        settings.callback_isr = BAPE_DmaOutput_P_Complete_isr;
        settings.pParm1 = hDma;
        settings.pParm2 = i;
        hDma->job[i].hCtx = BOTT_Dma_Context_Create(hDma->hOttDma, &settings);
        if (hDma->job[i].hCtx == NULL) {
            rc = BERR_OUT_OF_SYSTEM_MEMORY;
            goto error;
        }
        BLST_SQ_INSERT_TAIL(&hDma->freeJobList, hDma->job + i, link);
    }
    return BERR_SUCCESS;
error:
    if (hDma->job) {
        BLST_SQ_INIT(&hDma->freeJobList);
        for (i = 0; i < hDma->nJobs; i++) {
            if (hDma->job[i].hCtx) {
                BOTT_Dma_Context_Destroy(hDma->job[i].hCtx);
            }
        }
        BKNI_Free(hDma->job);
        hDma->job = NULL;
        hDma->nJobs = 0;
    }
    if (hDma->hUpstreamBG) {
        BAPE_BufferGroup_UnLinkOutput(hDma->hUpstreamBG, hDma->hBG);
        hDma->hUpstreamBG = NULL;
    }
    if (hDma->hBG) {
        BAPE_BufferGroup_Close(hDma->hBG);
        hDma->hBG = NULL;
    }
    return BERR_TRACE(rc);
}

static BERR_Code
BAPE_DmaOutput_P_Enable_isr(BAPE_DmaOutputHandle hDma, bool enable)
{
    BERR_Code rc = BERR_SUCCESS;
    if (hDma->enabled == enable) {
        return BERR_SUCCESS;
    }
    if (enable) {
        if (hDma->hUpstreamBG == NULL) {
            return BERR_TRACE(BERR_UNKNOWN);
        }
        rc = BAPE_BufferGroup_Enable_isr(hDma->hBG, true);
        if (rc != BERR_SUCCESS) {
            return BERR_TRACE(rc);
        }
        if (hDma->hFilter) {
            rc = BAPE_BufferGroupFilter_P_Enable_isr(hDma->hFilter, true);
            if (rc != BERR_SUCCESS) {
                BAPE_BufferGroup_Enable_isr(hDma->hBG, false);
                return BERR_TRACE(rc);
            }
        }
        rc = BAPE_DmaOutput_P_Submit_isr(hDma);
        if (rc != BERR_SUCCESS) {
            BAPE_BufferGroupFilter_P_Enable_isr(hDma->hFilter, false);
            BAPE_BufferGroup_Enable_isr(hDma->hBG, false);
        }
        if (rc == BERR_SUCCESS) {
            hDma->enabled = true;
        }
    }
    else {
        rc = BOTT_Dma_Flush_isr(hDma->hOttDma);
        if (rc == BERR_SUCCESS) {
            rc = BAPE_BufferGroup_Enable_isr(hDma->hBG, false);
        }
        if (rc == BERR_SUCCESS) {
            hDma->enabled = false;
        }
    }
    return rc;
}

BERR_Code
BAPE_DmaOutput_P_Enable(BAPE_DmaOutputHandle hDma, bool enable)
{
    BERR_Code rc;
    BKNI_EnterCriticalSection();
    rc = BAPE_DmaOutput_P_Enable_isr(hDma, enable);
    BKNI_LeaveCriticalSection();
    return rc;
}

#if 0
void
BAPE_DmaOutput_P_Run(void)
{
    BKNI_EnterCriticalSection();
    BOTT_DMA_P_DumpActiveCtxStatus_isrsafe(g_hDma->hOttDma);
    if (g_hDma->enabled) {
        BAPE_DmaOutput_P_Submit_isr(g_hDma);
    }
    BKNI_LeaveCriticalSection();
}
#endif

static void
BAPE_DmaOutput_P_DataReady_isr(void *pParam1, int param2)
{
    BAPE_DmaOutputHandle hDma;
    BSTD_UNUSED(param2);
    hDma = (BAPE_DmaOutputHandle)pParam1;
    /*BOTT_DMA_P_DumpActiveCtxStatus_isrsafe(hDma->hOttDma);*/
    if (hDma->enabled) {
        BAPE_DmaOutput_P_Submit_isr(hDma);
    }
}
