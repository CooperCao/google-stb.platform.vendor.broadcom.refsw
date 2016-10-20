/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its
 * licensors, and may only be used, duplicated, modified or distributed pursuant
 * to the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and Broadcom
 * expressly reserves all rights in and to the Software and all intellectual
 * property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 *    constitutes the valuable trade secrets of Broadcom, and you shall use all
 *    reasonable efforts to protect the confidentiality thereof, and to use
 *    this information only in connection with your use of Broadcom integrated
 *    circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
 *    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
 *    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
 *    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
 *    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
 *    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
 *    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
 *    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
 *    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
 *    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *****************************************************************************/


#include "bdsp.h"
#include "bdsp_priv.h"

BDBG_MODULE(BDSP_Stage);

void BDSP_Stage_GetDefaultCreateSettings(
    BDSP_ContextHandle hContext,
    BDSP_AlgorithmType algoType,
    BDSP_StageCreateSettings *pSettings /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(hContext, BDSP_Context);
    BDBG_ASSERT(NULL != pSettings);

    if ( hContext->getDefaultStageCreateSettings )
    {
        hContext->getDefaultStageCreateSettings(algoType, pSettings);
    }
    else
    {
        BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    }
}

BERR_Code BDSP_Stage_Create(
    BDSP_ContextHandle hContext,
    const BDSP_StageCreateSettings *pSettings,
    BDSP_StageHandle *pStage /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(hContext, BDSP_Context);
    BDBG_ASSERT(NULL != pStage);
    BDBG_ASSERT(NULL != pSettings);

    if ( hContext->createStage )
    {
        return hContext->createStage(hContext->pContextHandle, pSettings, pStage);
    }
    else
    {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}

void BDSP_Stage_Destroy(
    BDSP_StageHandle hStage
    )
{
    BDBG_OBJECT_ASSERT(hStage, BDSP_Stage);
    BDBG_ASSERT(NULL != hStage->destroy);
    hStage->destroy(hStage->pStageHandle);
}

BERR_Code BDSP_Stage_SetAlgorithm(
    BDSP_StageHandle hStage,
    BDSP_Algorithm algorithm
    )
{
    BDBG_OBJECT_ASSERT(hStage, BDSP_Stage);

    if (hStage->setAlgorithm)
    {
        return hStage->setAlgorithm(hStage->pStageHandle, algorithm);
    }
    else
    {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}

BERR_Code BDSP_Stage_GetSettings(
    BDSP_StageHandle hStage,
    void *pSettingsBuffer,        /* [out] */
    size_t settingsBufferSize
    )
{
    BDBG_OBJECT_ASSERT(hStage, BDSP_Stage);
    BDBG_ASSERT(NULL != pSettingsBuffer);
    BDBG_ASSERT(settingsBufferSize > 0);

    if ( hStage->getStageSettings )
    {
        return hStage->getStageSettings(hStage->pStageHandle, pSettingsBuffer, settingsBufferSize);
    }
    else
    {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}

BERR_Code BDSP_Stage_SetSettings(
    BDSP_StageHandle hStage,
    const void *pSettingsBuffer,
    size_t settingsBufferSize
    )
{
    BDBG_OBJECT_ASSERT(hStage, BDSP_Stage);
    BDBG_ASSERT(NULL != pSettingsBuffer);
    BDBG_ASSERT(settingsBufferSize > 0);

    if ( hStage->setStageSettings )
    {
        return hStage->setStageSettings(hStage->pStageHandle, pSettingsBuffer, settingsBufferSize);
    }
    else
    {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}


BERR_Code BDSP_Stage_GetStatus(
    BDSP_StageHandle hStage,
    void *pStatusBuffer,        /* [out] */
    size_t statusBufferSize
    )
{
    BDBG_OBJECT_ASSERT(hStage, BDSP_Stage);
    BDBG_ASSERT(NULL != pStatusBuffer);
    BDBG_ASSERT(statusBufferSize > 0);

    if ( hStage->getStageStatus )
    {
        return hStage->getStageStatus(hStage->pStageHandle, pStatusBuffer, statusBufferSize);
    }
    else
    {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}

BERR_Code BDSP_Stage_AddFmmOutput(
    BDSP_StageHandle hStage,
    BDSP_DataType dataType,
    const BDSP_FmmBufferDescriptor *pDescriptor,
    unsigned *pOutputIndex
    )
{
    BDBG_OBJECT_ASSERT(hStage, BDSP_Stage);
    BDBG_ASSERT(NULL != pDescriptor);
    BDBG_ASSERT(NULL != pOutputIndex);

    if (hStage->addFmmOutput)
    {
        return hStage->addFmmOutput(hStage->pStageHandle, dataType, pDescriptor, pOutputIndex);
    }
    else
    {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}

BERR_Code BDSP_Stage_AddRaveOutput(
    BDSP_StageHandle hStage,
    const BAVC_XptContextMap *pContext,
    unsigned *pOutputIndex /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(hStage, BDSP_Stage);
    BDBG_ASSERT(NULL != pContext);
    BDBG_ASSERT(NULL != pOutputIndex);

    if (hStage->addRaveOutput)
    {
        return hStage->addRaveOutput(hStage->pStageHandle, pContext, pOutputIndex);
    }
    else
    {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}

BERR_Code BDSP_Stage_AddOutputStage(
    BDSP_StageHandle hSourceStage,
    BDSP_DataType dataType,
    BDSP_StageHandle hDestinationStage,
    unsigned *pSourceOutputIndex, /* [out] */    /* Output index, reflects the source stage's output index for the destination stage. */
    unsigned *pDestinationInputIndex /* [out] */    /* Input index, reflects the destination stage's input index for the source stage. */
    )
{
    BDBG_OBJECT_ASSERT(hSourceStage, BDSP_Stage);
    BDBG_ASSERT(NULL != pSourceOutputIndex);

    BDBG_OBJECT_ASSERT(hDestinationStage, BDSP_Stage);
    BDBG_ASSERT(NULL != pDestinationInputIndex);

    if (hSourceStage->addOutputStage)
    {
        return hSourceStage->addOutputStage(hSourceStage->pStageHandle,
                                            dataType,
                                            hDestinationStage->pStageHandle,
                                            pSourceOutputIndex,
                                            pDestinationInputIndex);
    }
    else
    {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}

#if !B_REFSW_MINIMAL
void BDSP_Stage_RemoveOutput(
    BDSP_StageHandle hStage,
    unsigned outputIndex
    )
{
    BDBG_OBJECT_ASSERT(hStage, BDSP_Stage);
    BDBG_ASSERT(outputIndex < BDSP_AF_P_MAX_OP_FORKS);
    BDBG_ASSERT(NULL != hStage->removeOutput);
    hStage->removeOutput(hStage->pStageHandle, outputIndex);
}
#endif/*!B_REFSW_MINIMAL*/

BERR_Code BDSP_Stage_AddInterTaskBufferInput(
    BDSP_StageHandle hStage,
    BDSP_DataType dataType,
    BDSP_InterTaskBufferHandle hInterTaskBuffer,    /* inter-task buffer handle */
    unsigned *pInputIndex /* [out] */    /* input index, reflects the source stage's input index for the inter task buffer. */
    )
{
    BDBG_OBJECT_ASSERT(hStage, BDSP_Stage);
    BDBG_ASSERT(NULL != hInterTaskBuffer);
    BDBG_ASSERT(NULL != pInputIndex);

    if (hStage->addInterTaskBufferInput)
    {
        return hStage->addInterTaskBufferInput(
                    hStage->pStageHandle,
                    dataType,
                    hInterTaskBuffer->pInterTaskBufferHandle,
                    pInputIndex);
    }
    else
    {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}

BERR_Code BDSP_Stage_AddInterTaskBufferOutput(
    BDSP_StageHandle hStage,
    BDSP_DataType dataType,
    BDSP_InterTaskBufferHandle hInterTaskBuffer,    /* inter-task buffer handle */
    unsigned *pOutputIndex /* [out] */    /* Output index, reflects the source stage's output index for the inter task buffer. */
    )
{
    BDBG_OBJECT_ASSERT(hStage, BDSP_Stage);
    BDBG_ASSERT(NULL != hInterTaskBuffer);
    BDBG_ASSERT(NULL != pOutputIndex);

    if (hStage->addInterTaskBufferOutput)
    {
        return hStage->addInterTaskBufferOutput(
                    hStage->pStageHandle,
                    dataType,
                    hInterTaskBuffer->pInterTaskBufferHandle,
                    pOutputIndex);
    }
    else
    {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}

void BDSP_Stage_RemoveAllOutputs(
    BDSP_StageHandle hStage
    )
{
    BDBG_OBJECT_ASSERT(hStage, BDSP_Stage);
    BDBG_ASSERT(NULL != hStage->removeAllOutputs);
    hStage->removeAllOutputs(hStage->pStageHandle);
}

BERR_Code BDSP_Stage_AddFmmInput(
    BDSP_StageHandle hStage,
    BDSP_DataType dataType,
    const BDSP_FmmBufferDescriptor *pDescriptor,
    unsigned *pInputIndex
    )
{
    BDBG_OBJECT_ASSERT(hStage, BDSP_Stage);
    BDBG_ASSERT(NULL != pDescriptor);
    BDBG_ASSERT(NULL != pInputIndex);

    if (hStage->addFmmInput)
    {
        return hStage->addFmmInput(hStage->pStageHandle, dataType, pDescriptor, pInputIndex);
    }
    else
    {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}

BERR_Code BDSP_Stage_AddRaveInput(
    BDSP_StageHandle hStage,
    const BAVC_XptContextMap *pContext,
    unsigned *pInputIndex /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(hStage, BDSP_Stage);
    BDBG_ASSERT(NULL != pContext);
    BDBG_ASSERT(NULL != pInputIndex);

    if (hStage->addRaveInput)
    {
        return hStage->addRaveInput(hStage->pStageHandle, pContext, pInputIndex);
    }
    else
    {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}

void BDSP_Stage_RemoveInput(
    BDSP_StageHandle hStage,
    unsigned inputIndex
    )
{
    BDBG_OBJECT_ASSERT(hStage, BDSP_Stage);
    BDBG_ASSERT(inputIndex < BDSP_AF_P_MAX_IP_FORKS);
    BDBG_ASSERT(NULL != hStage->removeInput);

    hStage->removeInput(hStage->pStageHandle, inputIndex);
}

void BDSP_Stage_RemoveAllInputs(
    BDSP_StageHandle hStage
    )
{
    BDBG_OBJECT_ASSERT(hStage, BDSP_Stage);
    BDBG_ASSERT(NULL != hStage->removeAllInputs);
    hStage->removeAllInputs(hStage->pStageHandle);
}

/* Audio specific stage apis - may need to move these to bdsp_stage_audio.c */
BERR_Code BDSP_AudioStage_GetTsmSettings_isr(
    BDSP_StageHandle stage,
    BDSP_AudioTaskTsmSettings *pTsmSettings         /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(stage, BDSP_Stage);
    BDBG_ASSERT(NULL != pTsmSettings);

    BKNI_ASSERT_ISR_CONTEXT();

    if ( stage->getTsmSettings_isr )
    {
        return stage->getTsmSettings_isr(stage->pStageHandle, pTsmSettings);
    }
    else
    {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}

BERR_Code BDSP_AudioStage_SetTsmSettings_isr(
    BDSP_StageHandle stage,
    const BDSP_AudioTaskTsmSettings *pTsmSettings
    )
{
    BDBG_OBJECT_ASSERT(stage, BDSP_Stage);
    BDBG_ASSERT(NULL != pTsmSettings);

    BKNI_ASSERT_ISR_CONTEXT();

    if ( stage->setTsmSettings_isr )
    {
        return stage->setTsmSettings_isr(stage->pStageHandle, pTsmSettings);
    }
    else
    {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}

BERR_Code BDSP_AudioStage_GetTsmStatus_isr(
    BDSP_StageHandle stage,
    BDSP_AudioTaskTsmStatus *pTsmStatus         /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(stage, BDSP_Stage);
    BDBG_ASSERT(NULL != pTsmStatus);

    BKNI_ASSERT_ISR_CONTEXT();

    if ( stage->getTsmStatus_isr )
    {
        return stage->getTsmStatus_isr(stage->pStageHandle, pTsmStatus);
    }
    else
    {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}

BERR_Code BDSP_AudioStage_GetDatasyncSettings(
    BDSP_StageHandle stage,
    BDSP_AudioTaskDatasyncSettings *pDatasyncSettings         /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(stage, BDSP_Stage);
    BDBG_ASSERT(NULL != pDatasyncSettings);

    if ( stage->getDatasyncSettings )
    {
        return stage->getDatasyncSettings(stage->pStageHandle, pDatasyncSettings);
    }
    else
    {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}

BERR_Code BDSP_AudioStage_GetDatasyncSettings_isr(
    BDSP_StageHandle stage,
    BDSP_AudioTaskDatasyncSettings *pDatasyncSettings         /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(stage, BDSP_Stage);
    BDBG_ASSERT(NULL != pDatasyncSettings);

    if ( stage->getDatasyncSettings_isr )
    {
        return stage->getDatasyncSettings_isr(stage->pStageHandle, pDatasyncSettings);
    }
    else
    {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}

BERR_Code BDSP_AudioStage_SetDatasyncSettings(
    BDSP_StageHandle stage,
    const BDSP_AudioTaskDatasyncSettings *pDatasyncSettings
    )
{
    BDBG_OBJECT_ASSERT(stage, BDSP_Stage);
    BDBG_ASSERT(NULL != pDatasyncSettings);

    if ( stage->setDatasyncSettings )
    {
        return stage->setDatasyncSettings(stage->pStageHandle, pDatasyncSettings);
    }
    else
    {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}

BERR_Code BDSP_AudioStage_GetDatasyncStatus_isr(
    BDSP_StageHandle stage,
    BDSP_AudioTaskDatasyncStatus *pDatasyncStatus         /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(stage, BDSP_Stage);
    BDBG_ASSERT(NULL != pDatasyncStatus);

    if ( stage->getDatasyncStatus_isr )
    {
        return stage->getDatasyncStatus_isr(stage->pStageHandle, pDatasyncStatus);
    }
    else
    {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}

/* Retruns the datasync settings for video encoder */
BERR_Code BDSP_VideoEncodeTask_GetDatasyncSettings(
    BDSP_StageHandle stage,
    BDSP_VideoEncodeTaskDatasyncSettings *pDatasyncSettings         /* [out] */
    )
{
    BDBG_ASSERT(NULL != pDatasyncSettings);

    if ( stage->getVideoEncodeDatasyncSettings )
    {
        return stage->getVideoEncodeDatasyncSettings((void *)stage->pStageHandle, pDatasyncSettings);
    }
    else
    {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}

/* Sets datasync settings for dsp video encoder */
BERR_Code BDSP_VideoEncodeTask_SetDatasyncSettings(
    BDSP_StageHandle stage,
    const BDSP_VideoEncodeTaskDatasyncSettings *pDatasyncSettings
    )
{

    BDBG_ASSERT(NULL != pDatasyncSettings);

    if ( stage->setVideoEncodeDatasyncSettings )
    {
        return stage->setVideoEncodeDatasyncSettings((void *)stage->pStageHandle, pDatasyncSettings);
    }
    else
    {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}


/* Inter Task Buffer apis */
BERR_Code BDSP_InterTaskBuffer_Create(
    BDSP_ContextHandle hContext,
    BDSP_DataType dataType,
    BDSP_BufferType bufferType,
    BDSP_InterTaskBufferHandle *pHandle /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(hContext, BDSP_Context);
    BDBG_ASSERT(NULL != pHandle);

    if (hContext->createInterTaskBuffer)
    {
        return hContext->createInterTaskBuffer(hContext->pContextHandle, dataType, bufferType, pHandle);
    }
    else
    {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}

void BDSP_InterTaskBuffer_Destroy(
    BDSP_InterTaskBufferHandle hInterTaskBuffer
    )
{
    BDBG_OBJECT_ASSERT(hInterTaskBuffer, BDSP_InterTaskBuffer);
    BDBG_ASSERT(NULL != hInterTaskBuffer->destroy);
    hInterTaskBuffer->destroy(hInterTaskBuffer->pInterTaskBufferHandle);
}


void BDSP_InterTaskBuffer_Flush(
    BDSP_InterTaskBufferHandle hInterTaskBuffer
    )
{
    BDBG_OBJECT_ASSERT(hInterTaskBuffer, BDSP_InterTaskBuffer);
    BDBG_ASSERT(NULL != hInterTaskBuffer->flush);
    hInterTaskBuffer->flush(hInterTaskBuffer->pInterTaskBufferHandle);
}

/* Queue apis */
BERR_Code BDSP_Queue_GetDefaultSettings(
                    BDSP_ContextHandle hContext,
                    BDSP_QueueCreateSettings *pSettings)
{
    BDBG_OBJECT_ASSERT(hContext, BDSP_Context);
    BDBG_ASSERT(NULL != pSettings);
    if (hContext->getDefaultQueueSettings)
    {
        return hContext->getDefaultQueueSettings(hContext->pContextHandle, pSettings);
    }
    else
    {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}

BERR_Code BDSP_Queue_Create(
    BDSP_ContextHandle hContext,
    unsigned dspIndex,
    BDSP_QueueCreateSettings *pSettings,
    BDSP_QueueHandle *pHandle /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(hContext, BDSP_Context);
    BDBG_ASSERT(NULL != pHandle);

    if (hContext->createQueue)
    {
        return hContext->createQueue((void *)hContext->pContextHandle, dspIndex, pSettings, pHandle);
    }
    else
    {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}

void BDSP_Queue_Destroy(
    BDSP_QueueHandle hQueue
    )
{
    BDBG_OBJECT_ASSERT(hQueue, BDSP_Queue);
    BDBG_ASSERT(NULL != hQueue->destroy);
    hQueue->destroy((void *)hQueue->pQueueHandle);
}


void BDSP_Queue_Flush(
    BDSP_QueueHandle hQueue
    )
{
    BDBG_OBJECT_ASSERT(hQueue, BDSP_Queue);
    BDBG_ASSERT(NULL != hQueue->flush);
    hQueue->flush((void *)hQueue->pQueueHandle);
}

#if !B_REFSW_MINIMAL
BERR_Code BDSP_Stage_AddQueueInput(
    BDSP_StageHandle hStage,
    BDSP_QueueHandle hQueue,    /*Queue handle */
    unsigned *pInputIndex /* [out] */    /* input index, reflects the source stage's input index for the queue. */
    )
{
    BDBG_OBJECT_ASSERT(hStage, BDSP_Stage);
    BDBG_ASSERT(NULL != hQueue);
    BDBG_ASSERT(NULL != pInputIndex);

    if (hStage->addQueueInput)
    {
        return hStage->addQueueInput(
                    hStage->pStageHandle,
                    hQueue->pQueueHandle,
                    pInputIndex);

    }
    else
    {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}
#endif /*!B_REFSW_MINIMAL*/

BERR_Code BDSP_Stage_AddQueueOutput(
    BDSP_StageHandle hStage,
    BDSP_QueueHandle hQueue,    /*Queue handle */
    unsigned *pOutputIndex /* [out] */    /* Output index, reflects the source stage's output index for the queue. */
    )
{
    BDBG_OBJECT_ASSERT(hStage, BDSP_Stage);
    BDBG_ASSERT(NULL != hQueue);
    BDBG_ASSERT(NULL != pOutputIndex);

    if (hStage->addQueueOutput)
    {
        return hStage->addQueueOutput(
                    hStage->pStageHandle,
                    hQueue->pQueueHandle,
                    pOutputIndex);
    }
    else
    {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}

void BDSP_AudioCapture_GetDefaultCreateSettings(
    BDSP_AudioCaptureCreateSettings *pSettings  /* [out] */
    )
{
    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));

    pSettings->maxChannels = 2;
    pSettings->channelBufferSize = 1536*1024;
    pSettings->hHeap = NULL;
}

BERR_Code BDSP_AudioCapture_Create(
    BDSP_ContextHandle hContext,
    const BDSP_AudioCaptureCreateSettings *pSettings,
    BDSP_AudioCaptureHandle *pCapture   /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(hContext, BDSP_Context);
    BDBG_ASSERT(NULL != pSettings);
    BDBG_ASSERT(NULL != pCapture);

    if ( hContext->createCapture )
    {
        return hContext->createCapture(hContext->pContextHandle, pSettings, pCapture);
    }
    else
    {
        *pCapture = NULL;
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}

void BDSP_AudioCapture_Destroy(
    BDSP_AudioCaptureHandle hCapture
    )
{
    BDBG_ASSERT(NULL != hCapture);
    BDBG_ASSERT(NULL != hCapture->destroy);
    hCapture->destroy(hCapture->pCapHandle);
}

void BDSP_Stage_GetDefaultAudioCaptureSettings(
    BDSP_StageAudioCaptureSettings *pSettings /* [out] */
    )
{
    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
}

BERR_Code BDSP_Stage_AddAudioCapture(
    BDSP_StageHandle hStage,
    BDSP_AudioCaptureHandle hCapture,
    unsigned outputId,                  /* The output port number that needs to be captured */
    const BDSP_StageAudioCaptureSettings *pSettings  /* Optional, pass NULL for defaults */
    )
{
    BDSP_StageAudioCaptureSettings captureSettings;
    BDBG_ASSERT(NULL != hCapture);
    BDBG_ASSERT(NULL != hCapture->addToStage);
    if ( NULL == pSettings )
    {
        BDSP_Stage_GetDefaultAudioCaptureSettings(&captureSettings);
        pSettings = &captureSettings;
    }
    return hCapture->addToStage(hCapture->pCapHandle, hStage->pStageHandle, outputId, pSettings);

}

void BDSP_Stage_RemoveAudioCapture(
    BDSP_StageHandle hStage,
    BDSP_AudioCaptureHandle hCapture
    )
{
    BDBG_ASSERT(NULL != hCapture);
    BDBG_ASSERT(NULL != hCapture->removeFromStage);
    hCapture->removeFromStage(hCapture->pCapHandle, hStage->pStageHandle);
}

BERR_Code BDSP_AudioCapture_GetBuffer(
    BDSP_AudioCaptureHandle hCapture,
    BDSP_BufferDescriptor *pBuffers /* [out] */
    )
{
    BDBG_ASSERT(NULL != hCapture);
    BDBG_ASSERT(NULL != pBuffers);
    BDBG_ASSERT(NULL != hCapture->getBuffer);
    return hCapture->getBuffer(hCapture->pCapHandle, pBuffers);
}

BERR_Code BDSP_AudioCapture_ConsumeData(
    BDSP_AudioCaptureHandle hCapture,
    unsigned numBytes                   /* Number of bytes read from each buffer */
    )
{
    BDBG_ASSERT(NULL != hCapture);
    BDBG_ASSERT(NULL != hCapture->consumeData);
    return hCapture->consumeData(hCapture->pCapHandle, numBytes);
}


void BDSP_Queue_GetIoBuffer(
    BDSP_QueueHandle hQueue,
    BDSP_AF_P_sIO_BUFFER *pBuffer /*[out]*/
    )
{
    BDBG_OBJECT_ASSERT(hQueue, BDSP_Queue);
    BDBG_ASSERT(NULL != hQueue->getIoBuffer);
    hQueue->getIoBuffer((void *)hQueue->pQueueHandle, pBuffer);
}

BERR_Code BDSP_Queue_GetBuffer(
    BDSP_QueueHandle hQueue,
    BDSP_BufferDescriptor *pDescriptor /*[out] */
)
{
    BERR_Code   err = BERR_SUCCESS;
    BDBG_OBJECT_ASSERT(hQueue, BDSP_Queue);
    BDBG_ASSERT(NULL != hQueue->getBuffer);
    err = hQueue->getBuffer((void *)hQueue->pQueueHandle, pDescriptor);
    return err;
}

BERR_Code BDSP_Queue_ConsumeData(
    BDSP_QueueHandle hQueue,
    size_t readBytes
    )
{
    BERR_Code   err = BERR_SUCCESS;
    BDBG_OBJECT_ASSERT(hQueue, BDSP_Queue);
    BDBG_ASSERT(NULL != hQueue->consumeData);
    err = hQueue->consumeData((void *)hQueue->pQueueHandle, readBytes);
    return err;
}

BERR_Code BDSP_Queue_CommitData(
    BDSP_QueueHandle hQueue,
    size_t bytesWritten
    )
{
    BERR_Code   err = BERR_SUCCESS;
    BDBG_OBJECT_ASSERT(hQueue, BDSP_Queue);
    BDBG_ASSERT(NULL != hQueue->commitData);
    err = hQueue->commitData((void *)hQueue->pQueueHandle, bytesWritten);
    return err;
}

BERR_Code BDSP_Queue_GetBufferAddr(
    BDSP_QueueHandle hQueue,
    unsigned numbuf,
    void *pBuffer)
{
    BERR_Code   err = BERR_SUCCESS;
    BDBG_OBJECT_ASSERT(hQueue, BDSP_Queue);
    BDBG_ASSERT(NULL != hQueue->getOpBufferAddr);
    err = hQueue->getOpBufferAddr((void *)hQueue->pQueueHandle, numbuf, pBuffer);
    return err;
}
