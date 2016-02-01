/***************************************************************************
*     (c)2004-2014 Broadcom Corporation
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
*   API name: wmdrmpd_io
*    Specific APIs related to Microsoft Windows Media DRM PD I/O
*
* Revision History:
*
* $brcm_Log: $
*
***************************************************************************/

#include "nexus_wmdrmpd_io_module.h"
#include "nexus_memory.h"
#include "priv/nexus_core.h"

BDBG_MODULE(nexus_wmdrmpd_io);

typedef struct NEXUS_WmDrmPdIoJob
{
    bool allocated;
    unsigned id;
    size_t completedBytes;    
    NEXUS_WmDrmPdIoJobSettings settings;
    NEXUS_WmDrmPdIoJobStatus status;
    NEXUS_WmDrmPdIoHandle hIo;
} NEXUS_WmDrmPdIoJob;

typedef struct NEXUS_WmDrmPdIo
{
    NEXUS_OBJECT(NEXUS_WmDrmPdIo);
    NEXUS_WmDrmPdIoSettings settings;
    NEXUS_WmDrmPdIoJob job;         /* We only need one of these, there is only a single DRM Core module that can make a single request at a time */
    unsigned currentJobId;
    NEXUS_TaskCallbackHandle hJobCallback;
    NEXUS_HeapHandle hHeap;
    void *pBuffer;    
} NEXUS_WmDrmPdIo;

NEXUS_OBJECT_CLASS_MAKE(NEXUS_WmDrmPdIo, NEXUS_WmDrmPdIo_Destroy);

void NEXUS_WmDrmPdIo_GetDefaultSettings(
    NEXUS_WmDrmPdIoSettings *pSettings  /* [out] */
    )
{
    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(NEXUS_WmDrmPdIoSettings));
}

NEXUS_WmDrmPdIoHandle NEXUS_WmDrmPdIo_Create(
    const NEXUS_WmDrmPdIoSettings *pSettings    /* attr{null_allowed=y} Optional.  Pass NULL for default settings. */
    )
{
    NEXUS_WmDrmPdIoSettings defaults;
    NEXUS_WmDrmPdIoHandle hIo;
    NEXUS_HeapHandle hHeap;
    NEXUS_MemoryAllocationSettings allocSettings;
    NEXUS_Error errCode;

    if ( NULL == pSettings )
    {
        NEXUS_WmDrmPdIo_GetDefaultSettings(&defaults);
        pSettings = &defaults;
    }

    hIo = BKNI_Malloc(sizeof(NEXUS_WmDrmPdIo));
    if ( NULL == hIo )
    {
        (void)BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto err_malloc;
    }

    BKNI_Memset(hIo, 0, sizeof(NEXUS_WmDrmPdIo));

    /* prefer client-accessible eFull heap */
    hHeap = NEXUS_P_DefaultHeap(NULL, NEXUS_DefaultHeapType_eFull);
    if ( NULL == hHeap )
    {
         /* but if none, use [0], which is eFull, but is not necessarily client accessible */
         hHeap = g_pCoreHandles->heap[g_pCoreHandles->defaultHeapIndex].nexus;
    }


    if (!NEXUS_P_CpuAccessibleHeap(hHeap)) 
    {
        BDBG_ERR(("IO heap is not CPU accessible.  Please provide a CPU-accessible heap in NEXUS_WmDrmPdIoSettings."));
        (void)BERR_TRACE(NEXUS_INVALID_PARAMETER);
        goto err_heap;
    }
    NEXUS_Memory_GetDefaultAllocationSettings(&allocSettings);
    allocSettings.heap = hHeap;
    errCode = NEXUS_Memory_Allocate(g_NEXUS_wmDrmPdIoModuleData.settings.ioBufferSize, &allocSettings, &hIo->pBuffer);
    if ( errCode )
    {
        (void)BERR_TRACE(errCode);
        goto err_alloc_buffer;
    }
    hIo->hHeap = hHeap;
    hIo->hJobCallback = NEXUS_TaskCallback_Create(hIo, NULL);
    if ( NULL == hIo->hJobCallback )
    {
        (void)BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto err_callback_create;
    }
    NEXUS_TaskCallback_Set(hIo->hJobCallback, &pSettings->jobCallback);
    hIo->settings = *pSettings;
    NEXUS_OBJECT_SET(NEXUS_WmDrmPdIo, hIo);
    return hIo;

err_callback_create:
    NEXUS_Memory_Free(hIo->pBuffer);
err_alloc_buffer:    
err_heap:
    BKNI_Free(hIo);
err_malloc:
    return NULL;
}

static void NEXUS_WmDrmPdIo_P_Finalizer(
    NEXUS_WmDrmPdIoHandle hIo
    )
{
    NEXUS_OBJECT_ASSERT(NEXUS_WmDrmPdIo, hIo);
    NEXUS_TaskCallback_Destroy(hIo->hJobCallback);
    NEXUS_Memory_Free(hIo->pBuffer);
    NEXUS_OBJECT_DESTROY(NEXUS_WmDrmPdIo, hIo);
    BKNI_Free(hIo);
}

NEXUS_Error NEXUS_WmDrmPdIo_GetRequest(
    NEXUS_WmDrmPdIoHandle hIo,
    NEXUS_WmDrmPdIoRequest *pRequest    /* [out] */
    )
{
    NEXUS_OBJECT_ASSERT(NEXUS_WmDrmPdIo, hIo);
    BDBG_ASSERT(NULL != pRequest);
    BKNI_Memset(pRequest, 0, sizeof(NEXUS_WmDrmPdIoRequest));
    if ( hIo->job.allocated && hIo->job.status.pending )
    {
        /* If the job is still pending, determine the state */  
        pRequest->requestId = hIo->job.id;      
        pRequest->operation = hIo->job.settings.operation;      
        pRequest->pFileHandle = hIo->job.settings.pFileHandle;
        switch ( hIo->job.settings.operation )
        {
            case NEXUS_WmDrmPdIoOperation_eOpen:
            BKNI_Memcpy(pRequest->params.open.fileName, hIo->job.settings.params.open.fileName, NEXUS_WMDRMPDIO_MAX_FNAME);
            BKNI_Memcpy(pRequest->params.open.mode, hIo->job.settings.params.open.mode, NEXUS_WMDRMPDIO_MAX_MODE);
            pRequest->pFileHandle = NULL;
            break;
        case NEXUS_WmDrmPdIoOperation_eRead:
        case NEXUS_WmDrmPdIoOperation_eWrite:
            pRequest->pBuffer = hIo->pBuffer;
            BDBG_ASSERT(hIo->job.completedBytes < hIo->job.settings.length);
            if ( (hIo->job.settings.length-hIo->job.completedBytes) > g_NEXUS_wmDrmPdIoModuleData.settings.ioBufferSize )
            {
                pRequest->length = g_NEXUS_wmDrmPdIoModuleData.settings.ioBufferSize;
            }
            else
            {
                pRequest->length = hIo->job.settings.length-hIo->job.completedBytes;
            }
            if ( hIo->job.settings.operation == NEXUS_WmDrmPdIoOperation_eWrite )
            {
                /* Copy data into kernel/user safe buffer */
                BKNI_Memcpy(hIo->pBuffer, ((uint8_t *)hIo->job.settings.pBuffer)+hIo->job.completedBytes, pRequest->length);
            }
            break;
        case NEXUS_WmDrmPdIoOperation_eSeek:
            pRequest->params.seek.whence = hIo->job.settings.params.seek.whence;
            pRequest->params.seek.offset = hIo->job.settings.params.seek.offset;
            break;
        case  NEXUS_WmDrmPdIoOperation_eUnlink:
            BKNI_Memcpy(pRequest->params.unlink.fileName, hIo->job.settings.params.unlink.fileName, NEXUS_WMDRMPDIO_MAX_FNAME);
            break;
        default:
            break;
        }
    }
    return BERR_SUCCESS;
}

void NEXUS_WmDrmPdIo_GetDefaultIoResponse(
    NEXUS_WmDrmPdIoHandle hIo,
    const NEXUS_WmDrmPdIoRequest *pRequest, /* Request Parameters */
    NEXUS_WmDrmPdIoResponse *pResponse      /* [out] */
    )
{
    NEXUS_OBJECT_ASSERT(NEXUS_WmDrmPdIo, hIo);
    BDBG_ASSERT(NULL != pRequest);
    BDBG_ASSERT(NULL != pResponse);
    BKNI_Memset(pResponse, 0, sizeof(NEXUS_WmDrmPdIoResponse));
    pResponse->operation = pRequest->operation;
    pResponse->requestId = pRequest->requestId;
}

NEXUS_Error NEXUS_WmDrmPdIo_SendResponse(
    NEXUS_WmDrmPdIoHandle hIo,
    const NEXUS_WmDrmPdIoResponse *pResponse
    )
{
    NEXUS_OBJECT_ASSERT(NEXUS_WmDrmPdIo, hIo);
    BDBG_ASSERT(NULL != pResponse);
    if ( !hIo->job.allocated || !hIo->job.status.pending || hIo->job.id != pResponse->requestId )
    {
        BDBG_WRN(("IO Response invalid, request may have been cancelled."));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    hIo->job.status.pending = false;
    hIo->job.status.errCode = pResponse->errCode;
    if ( pResponse->errCode == NEXUS_SUCCESS )
    {
        switch ( pResponse->operation )
        {
        case NEXUS_WmDrmPdIoOperation_eOpen:
            hIo->job.status.params.open.pFileHandle = pResponse->params.open.pFileHandle;
            break;
        case NEXUS_WmDrmPdIoOperation_eRead:
            BDBG_ASSERT(pResponse->params.read.length <= g_NEXUS_wmDrmPdIoModuleData.settings.ioBufferSize);
            BDBG_ASSERT(pResponse->params.read.length + hIo->job.completedBytes <= hIo->job.settings.length);
            if ( pResponse->params.read.length )
            {
                /* Copy data into request buffer */
                BKNI_Memcpy(((uint8_t *)hIo->job.settings.pBuffer)+hIo->job.completedBytes, hIo->pBuffer, pResponse->params.read.length);
            }
            hIo->job.completedBytes += pResponse->params.read.length;
            if ( hIo->job.completedBytes < hIo->job.settings.length )
            {
                /* Still more to do, wait for next request/response */
                hIo->job.status.pending = true;
            }
            hIo->job.status.params.read.length = hIo->job.completedBytes;
            break;
        case NEXUS_WmDrmPdIoOperation_eWrite:
            BDBG_ASSERT(pResponse->params.write.length <= g_NEXUS_wmDrmPdIoModuleData.settings.ioBufferSize);
            BDBG_ASSERT(pResponse->params.write.length + hIo->job.completedBytes <= hIo->job.settings.length);
            hIo->job.completedBytes += pResponse->params.write.length;
            if ( hIo->job.completedBytes < hIo->job.settings.length )
            {
                /* Still more to do, wait for next request/response */
                hIo->job.status.pending = true;
            }
            hIo->job.status.params.write.length = hIo->job.completedBytes;
            break;
        case NEXUS_WmDrmPdIoOperation_eSeek:
            hIo->job.status.params.seek.offset = pResponse->params.seek.offset;
            break;
        case NEXUS_WmDrmPdIoOperation_eTell:
            hIo->job.status.params.tell.offset = pResponse->params.tell.offset;
            break;
        default:
            break;
        }
    }
    if ( !hIo->job.status.pending && hIo->job.settings.completionEvent )
    {
        BKNI_SetEvent(hIo->job.settings.completionEvent);
    }

    return BERR_SUCCESS;
}

void NEXUS_WmDrmPdIoJob_GetDefaultSettings_priv(
    NEXUS_WmDrmPdIoJobSettings *pSettings   /* [out] */
    )
{
    NEXUS_ASSERT_MODULE();
    BDBG_ASSERT(pSettings != NULL);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
}

NEXUS_WmDrmPdIoJobHandle NEXUS_WmDrmPdIoJob_Create_priv(
    NEXUS_WmDrmPdIoHandle hIo,
    const NEXUS_WmDrmPdIoJobSettings *pSettings
    )
{
    NEXUS_ASSERT_MODULE();
    NEXUS_OBJECT_ASSERT(NEXUS_WmDrmPdIo, hIo);
    BDBG_ASSERT(pSettings != NULL);

    /* We can never have more than one outstanding request.  It should not be possible unless a synchronization error has occurred */
    BDBG_ASSERT(hIo->job.allocated == false);
    BKNI_Memset(&hIo->job, 0, sizeof(hIo->job));
    hIo->job.allocated = true;
    hIo->job.settings = *pSettings;
    hIo->job.id = ++hIo->currentJobId;
    hIo->job.hIo = hIo;
    hIo->job.status.pending = true;

    NEXUS_TaskCallback_Fire(hIo->hJobCallback);

    return &hIo->job;
}

void NEXUS_WmDrmPdIoJob_Destroy_priv(
    NEXUS_WmDrmPdIoJobHandle hJob
    )
{
    NEXUS_WmDrmPdIoHandle hIo;
    NEXUS_ASSERT_MODULE();
    BDBG_ASSERT(NULL != hJob);
    hIo = hJob->hIo;
    NEXUS_OBJECT_ASSERT(NEXUS_WmDrmPdIo, hIo);
    BDBG_ASSERT(hJob->allocated == true);
    BKNI_Memset(hJob, 0, sizeof(*hJob));
}

NEXUS_Error NEXUS_WmDrmPdIoJob_GetStatus_priv(
    NEXUS_WmDrmPdIoJobHandle hJob,
    NEXUS_WmDrmPdIoJobStatus *pStatus   /* [out] */
    )
{
    BDBG_ASSERT(NULL != hJob);
    BDBG_ASSERT(NULL != pStatus);

    BDBG_ASSERT(hJob->allocated = true);
    NEXUS_OBJECT_ASSERT(NEXUS_WmDrmPdIo, hJob->hIo);

    BKNI_Memcpy(pStatus, &hJob->status, sizeof(hJob->status));
    return NEXUS_SUCCESS;
}

