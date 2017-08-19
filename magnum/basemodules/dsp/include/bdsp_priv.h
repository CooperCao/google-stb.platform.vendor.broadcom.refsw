/******************************************************************************
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
 *****************************************************************************/


#ifndef BDSP_PRIV_H_
#define BDSP_PRIV_H_

#include "bkni.h"
#include "bkni_multi.h"
#include "bdbg.h"

BDBG_OBJECT_ID_DECLARE(BDSP_Device);

#define BDSP_INVALID_INDEX                  (unsigned int)-1


typedef struct BDSP_Device
{
    BDBG_OBJECT(BDSP_Device)
    void *pDeviceHandle;
    /* Device-level Function table */
    void (*close)(void *pDeviceHandle);
    void (*initialize)(void *pDeviceHandle);
    void (*getStatus)(void *pDeviceHandle, BDSP_Status *pStatus);
    void (*getDefaultContextSettings)(void *pDeviceHandle,BDSP_ContextType contextType, BDSP_ContextCreateSettings *pSettings);
    BERR_Code (*createContext)(void *pDeviceHandle, const BDSP_ContextCreateSettings *pSettings, BDSP_ContextHandle *pContext);
    BERR_Code (*powerStandby)(void *pDeviceHandle, BDSP_StandbySettings     *pSettings);
    BERR_Code (*powerResume)(void *pDeviceHandle);
    void (*getAlgorithmInfo)(BDSP_Algorithm algorithm, BDSP_AlgorithmInfo *pInfo);

    /* Below functions provide external interrupt handles to SW */
    BERR_Code (*allocateExternalInterrupt)(void *pDeviceHandle, uint32_t dspIndex, BDSP_ExternalInterruptHandle *pInterruptHandle);
    BERR_Code (*freeExternalInterrupt)(void  *pInterruptHandle);
    BERR_Code (*getExternalInterruptInfo)(void *pInterruptHandle, BDSP_ExternalInterruptInfo **pInfo);
    BERR_Code (*processAudioCapture)(void *pDeviceHandle);
}BDSP_Device;

void BDSP_P_InitDevice(
    BDSP_Device *pDevice,
    void *pDeviceHandle
    );


BDBG_OBJECT_ID_DECLARE(BDSP_InterTaskBuffer);
typedef struct BDSP_InterTaskBuffer
{
    BDBG_OBJECT(BDSP_InterTaskBuffer)
    void *pInterTaskBufferHandle;

    /* Intertask buffer function table */
    void (*destroy)(void *pInterTaskBufferHandle);
    void (*flush)(void *pInterTaskBufferHandle);
}BDSP_InterTaskBuffer;

BDBG_OBJECT_ID_DECLARE(BDSP_Queue);
typedef struct BDSP_Queue
{
    BDBG_OBJECT(BDSP_Queue)
    void *pQueueHandle;

    /* Queue function table */
    void (*destroy)(void *pQueueHandle);
    void (*flush)(void *pQueueHandle);
    void (*getIoBuffer)(void *pQueueHandle, BDSP_AF_P_sIO_BUFFER *pBuffer /*[out]*/);
    BERR_Code (*getBuffer)(void *pQueueHandle ,BDSP_BufferDescriptor *pDescriptor /*[out] */);
    BERR_Code (*consumeData)(void *pQueueHandle, size_t readBytes);
    BERR_Code (*commitData)(void *pQueueHandle, size_t bytesWritten);
    BERR_Code (*getOpBufferAddr)(void *pQueueHandle, unsigned numbuffers, void *pBuffer /*[out] */);
}BDSP_Queue;

BDBG_OBJECT_ID_DECLARE(BDSP_Stage);
typedef struct BDSP_Stage
{

    BDBG_OBJECT(BDSP_Stage)
    void *pStageHandle;

    /* Stage level function table */
    void (*destroy)(void *pStageHandle);
    BERR_Code (*setAlgorithm)(void *pStageHandle, BDSP_Algorithm algorithm);
    BERR_Code (*getStageSettings)(void *pStageHandle, void *pSettingsBuffer, size_t settingsSize);
    BERR_Code (*setStageSettings)(void *pStageHandle, const void *pSettingsBuffer, size_t settingsSize);
    BERR_Code (*getStageStatus)(void *pStageHandle, void *pStatusBuffer, size_t statusSize);

    /* Audio stage specific function table */
    BERR_Code (*getTsmSettings_isr)(void *pStageHandle, BDSP_AudioTaskTsmSettings *pTsmSettings);
    BERR_Code (*setTsmSettings_isr)(void *pStageHandle, const BDSP_AudioTaskTsmSettings *pTsmSettings);
    BERR_Code (*getTsmStatus_isr)(void *pStageHandle, BDSP_AudioTaskTsmStatus *pTsmStatus);
    BERR_Code (*getDatasyncSettings)(void *pStageHandle, BDSP_AudioTaskDatasyncSettings *pSettings);
    BERR_Code (*getDatasyncSettings_isr)(void *pStageHandle, BDSP_AudioTaskDatasyncSettings *pSettings);
    BERR_Code (*setDatasyncSettings)(void *pStageHandle, const BDSP_AudioTaskDatasyncSettings *pSettings);
    BERR_Code (*getDatasyncStatus_isr)(void *pStageHandle, BDSP_AudioTaskDatasyncStatus *pStatus);

    BERR_Code (*addFmmOutput)(void *pStageHandle, BDSP_DataType dataType, const BDSP_FmmBufferDescriptor *pDescriptor, unsigned *pOutputIndex);
    BERR_Code (*addRaveOutput)(void *pStageHandle, const BAVC_XptContextMap *pContext, unsigned *pOutputIndex);
    BERR_Code (*addOutputStage)(void *pSrcStageHandle, BDSP_DataType dataType, void *pDstStageHandle, unsigned *pSourceInputIndex, unsigned *pDestinationInputIndex);
    BERR_Code (*addInterTaskBufferInput)(void *pStageHandle, BDSP_DataType dataType, const BDSP_InterTaskBuffer *pBufferHandle, unsigned *pInputIndex);
    BERR_Code (*addInterTaskBufferOutput)(void *pStageHandle, BDSP_DataType dataType, const BDSP_InterTaskBuffer *pBufferHandle, unsigned *pOutputIndex);
#if !B_REFSW_MINIMAL
    void (*removeOutput)(void *pStageHandle, unsigned outputIndex);
#endif /*!B_REFSW_MINIMAL*/
    void (*removeAllOutputs)(void *pStageHandle);

    BERR_Code (*addFmmInput)(void *pStageHandle, BDSP_DataType dataType, const BDSP_FmmBufferDescriptor *pDescriptor, unsigned *pOutputIndex);
    BERR_Code (*addRaveInput)(void *pStageHandle, const BAVC_XptContextMap *pContext, unsigned *pOutputIndex);
    void (*removeInput)(void *pStageHandle, unsigned inputIndex);
    void (*removeAllInputs)(void *pStageHandle);

    BERR_Code (*addQueueOutput)(void *pStageHandle, void *pQueueHandle, unsigned *pOutputIndex);
#if !B_REFSW_MINIMAL
    BERR_Code (*addQueueInput)(void *pStageHandle, void *pQueueHandle, unsigned *pInputIndex);
#endif /*!B_REFSW_MINIMAL*/
    BERR_Code (*getVideoEncodeDatasyncSettings)(void *pStageHandle, BDSP_VideoEncodeTaskDatasyncSettings *pSettings);
    BERR_Code (*setVideoEncodeDatasyncSettings)(void *pStageHandle, const BDSP_VideoEncodeTaskDatasyncSettings *pSettings);

}BDSP_Stage;


BDBG_OBJECT_ID_DECLARE(BDSP_Context);

typedef struct BDSP_Context
{
    BDBG_OBJECT(BDSP_Context)
    void *pContextHandle;
    /* Context-level Function table */
    void (*destroy)(void *pContextHandle);
    void (*getInterruptHandlers)(void *pContextHandle, BDSP_ContextInterruptHandlers *pInterrupts);
    BERR_Code (*setInterruptHandlers)(void *pContextHandle, const BDSP_ContextInterruptHandlers *pInterrupts);
    BERR_Code (*processWatchdogInterrupt)(void *pContextHandle);
    void (*getDefaultTaskSettings)(void *pContextHandle, BDSP_TaskCreateSettings *pSettings);
    BERR_Code (*createTask)(void *pContextHandle, const BDSP_TaskCreateSettings *pSettings, BDSP_TaskHandle *pTask);

    void (*getDefaultStageCreateSettings)(BDSP_AlgorithmType stageType, BDSP_StageCreateSettings *pSettings);
    BERR_Code (*createStage)(void *pContextHandle, const BDSP_StageCreateSettings *pSettings, BDSP_StageHandle *pStage);
    BERR_Code (*createInterTaskBuffer)(void *pContextHandle, BDSP_DataType dataType,  BDSP_BufferType bufferType, BDSP_InterTaskBufferHandle *pInterTaskBufferHandle);

    /* Capture api */
    BERR_Code (*createCapture)(void *pContextHandle, const BDSP_AudioCaptureCreateSettings *pCaptureCreateSettings, BDSP_AudioCaptureHandle *pCapture);

    BERR_Code (*createQueue)(void *pContextHandle, unsigned dspIndex, BDSP_QueueCreateSettings *pSettings, BDSP_QueueHandle *pQueueHandle);
    BERR_Code (*getDefaultQueueSettings)(void *pContextHandle, BDSP_QueueCreateSettings *pSettings);
}BDSP_Context;

void BDSP_P_InitContext(
    BDSP_Context *pContext,
    void *pContextHandle
    );

BDBG_OBJECT_ID_DECLARE(BDSP_Task);

typedef struct BDSP_Task
{
    BDBG_OBJECT(BDSP_Task)
    void *pTaskHandle;
    /* Task-level Function table */
    void      (*destroy)(void *pTaskHandle);
    void (*getDefaultTaskStartSettings)(void *pTaskHandle, BDSP_TaskStartSettings *pSettings);
    BERR_Code (*start)(void *pTaskHandle, BDSP_TaskStartSettings *pSettings);
    BERR_Code (*stop)(void *pTaskHandle);

    /* Audio task Function table */
    BERR_Code (*pause)(void *pTaskHandle);
    BERR_Code (*resume)(void *pTaskHandle);
    BERR_Code (*advance)(void *pTaskHandle, unsigned ms);
    void      (*getAudioInterruptHandlers_isr)(void *pTaskHandle, BDSP_AudioInterruptHandlers *pHandlers);
    BERR_Code (*setAudioInterruptHandlers_isr)(void *pTaskHandle, const BDSP_AudioInterruptHandlers *pHandlers);
    BERR_Code (*audioGapFill)(void *pTaskHandle);
/* PAUSE-UNPAUSE */
    BERR_Code (*freeze)(void *pTaskHandle,const BDSP_AudioTaskFreezeSettings *pFreezeSettings);
    BERR_Code (*unfreeze)(void *pTaskHandle,const BDSP_AudioTaskUnFreezeSettings *pUnFreezeSettings);
/* PAUSE-UNPAUSE */
    BERR_Code (*retreiveGateOpenSettings)(void *pTaskHandle, BDSP_TaskGateOpenSettings *pSettings);

    /* Video task Function table */
    BERR_Code (*getPictureCount_isr)(void *pTaskHandle,unsigned *pPictureCount);
    BERR_Code (*peekAtPicture_isr)(void *pTaskHandle, unsigned index,dramaddr_t **pUnifiedPicture);
    BERR_Code (*getNextPicture_isr)(void *pTaskHandle,uint32_t **pUnifiedPicture);
    BERR_Code (*releasePicture_isr)(void *pTaskHandle,uint32_t *pUnifiedPicture);
    BERR_Code (*getPictureDropPendingCount_isr)(void *pTaskHandle, unsigned *pPictureDropPendingCount);
    BERR_Code (*requestPictureDrop_isr)(void *pTaskHandle,unsigned *pPictureDropRequestCount);
    BERR_Code (*displayInterruptEvent_isr)(void *pTaskHandle);

    /* Video Encode task Function table */
    BERR_Code (*inputPictureBufferCount_isr)(void *pTaskHandle, uint32_t    *pPpbCount);
    BERR_Code (*getPictureBuffer_isr)(void *pTaskHandle, dramaddr_t *pPpBuf);
    BERR_Code (*putPicture_isr)(void *pTaskHandle, dramaddr_t pPictureParamBufAddress);
    BERR_Code (*putCcData_isr)(void *pTaskHandle, void  *pCCDAddress);
    /* SCM task function table */
    BERR_Code (*sendScmCommand)(void *pTaskHandle, BDSP_Raaga_P_SCM_CmdPayload *pScmCmdPayload);

}BDSP_Task;

/***************************************************************************
Summary:
        BDSP Audio Capture Handle
***************************************************************************/
BDBG_OBJECT_ID_DECLARE(BDSP_AudioCapture);

typedef struct BDSP_AudioCapture
{
    BDBG_OBJECT(BDSP_AudioCapture)
    void *pCapHandle;

    void (*destroy)(void *pCapHandle);
    BERR_Code (*addToStage)(void *pCapHandle, void *pStageHandle, unsigned outputId, const BDSP_StageAudioCaptureSettings *pSettings);
    void (*removeFromStage)(void *pCapHandle, void *pStageHandle);
    BERR_Code (*getBuffer)(void *pCapHandle, BDSP_BufferDescriptor *pBuffers);
    BERR_Code (*consumeData)(void *pCapHandle, uint32_t numBytes);
} BDSP_AudioCapture;

BDBG_OBJECT_ID_DECLARE(BDSP_ExternalInterrupt);

/* Handle for External interrupt to DSP */
typedef struct BDSP_ExternalInterrupt
{
    BDBG_OBJECT(BDSP_ExternalInterrupt)
    BDSP_Handle hDsp;
    void * pExtInterruptHandle;

}BDSP_ExternalInterrupt;

void BDSP_P_InitTask(
    BDSP_Task *pTask,
    void *pTaskHandle
    );

void BDSP_P_InitStage(
    BDSP_Stage *pStage,
    void *pStageHandle
    );

void BDSP_P_InitInterTaskBuffer(
    BDSP_InterTaskBuffer *pInterTaskBuffer,
    void *pInterTaskBufferHandle
    );

void BDSP_P_InitQueue(
    BDSP_Queue *pQueue,
    void *pQueueHandle
    );


#endif /* #ifndef BDSP_PRIV_H_ */
