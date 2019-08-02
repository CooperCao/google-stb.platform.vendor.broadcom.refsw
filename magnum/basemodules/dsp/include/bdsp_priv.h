/******************************************************************************
 * Copyright (C) 2018 Broadcom.
 * The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to
 * the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied),
 * right to use, or waiver of any kind with respect to the Software, and
 * Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 * THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 * IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use all
 * reasonable efforts to protect the confidentiality thereof, and to use this
 * information only in connection with your use of Broadcom integrated circuit
 * products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 * OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 * RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 * IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 * A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 * ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 * THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 * OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 * INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 * RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 * HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 * EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 * WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 * FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *****************************************************************************/


#ifndef BDSP_PRIV_H_
#define BDSP_PRIV_H_

#include "bkni.h"
#include "bkni_multi.h"
#include "bdbg.h"
#include "bdsp_soft_fmm_struct.h"

BDBG_OBJECT_ID_DECLARE(BDSP_Device);

#define BDSP_INVALID_INDEX                  (unsigned int)-1


typedef struct BDSP_Device
{
    BDBG_OBJECT(BDSP_Device)
    void *pDeviceHandle;
    /* Device-level Function table */
    void (*close)(void *pDeviceHandle);
    void (*getStatus)(void *pDeviceHandle, BDSP_Status *pStatus);
    BERR_Code (*getAudioLicenseStatus)(void *pDeviceHandle, BDSP_AudioLicenseStatus *pStatus);
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

    /* Below functions are for extracting the debug information*/
    BERR_Code (*getDebugBuffer)(void *pDeviceHandle, BDSP_DebugType debugType, uint32_t dspIndex, BDSP_MMA_Memory *pBuffer, size_t *pSize);
    BERR_Code (*consumeDebugData)(void *pDeviceHandle, BDSP_DebugType debugType, uint32_t dspIndex, size_t bytesConsumed);
    BDSP_FwStatus (*getCoreDumpStatus)(void *pDeviceHandle, uint32_t dspIndex);
    BERR_Code (*runDebugService)(void *pDeviceHandle, uint32_t dspIndex);

    /* Default TSM and Datasync settings*/
    BERR_Code (*getDefaultTsmSettings)(void *pDeviceHandle, void *pSettingsBuffer, size_t settingsBufferSize);
    BERR_Code (*getDefaultDatasyncSettings)(void *pDeviceHandle, void *pSettingsBuffer, size_t settingsBufferSize);

    /*Authentication related APIs */
    BERR_Code (*getDownloadStatus)(void *pDeviceHandle, BDSP_DownloadStatus *pStatus);
    BERR_Code (*initialize)(void *pDeviceHandle);
    BERR_Code (*getRRRAddrRange)(void *pDeviceHandle, BDSP_DownloadStatus *pRRRAddrRange);
    BERR_Code (*processPAK)(void *pDeviceHandle, const BDSP_ProcessPAKSettings *pPakSettings, BDSP_ProcessPAKStatus *pStatus);

    /* API to open a SoftFMM handle*/
    BERR_Code (*softFMMOpen)(void *pDeviceHandle, BDSP_SoftFMMHandle *pSoftFMM);
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
    BERR_Code (*getAudioDelay_isrsafe)(BDSP_CTB_Input *pCtbInput, void *pStageHandle, BDSP_CTB_Output *pCTBOutput);

    BERR_Code (*addFmmOutput)(void *pStageHandle, BDSP_DataType dataType, const BDSP_FmmBufferDescriptor *pDescriptor, unsigned *pOutputIndex);
    BERR_Code (*addSoftFmmOutput)(void *pStageHandle, const BDSP_SoftFMMBufferDescriptor *pDescriptor, unsigned *pOutputIndex);
    BERR_Code (*addRaveOutput)(void *pStageHandle, const BAVC_XptContextMap *pContext, unsigned *pOutputIndex);
    BERR_Code (*addOutputStage)(void *pSrcStageHandle, BDSP_DataType dataType, void *pDstStageHandle, unsigned *pSourceInputIndex, unsigned *pDestinationInputIndex);
    BERR_Code (*addInterTaskBufferInput)(void *pStageHandle, BDSP_DataType dataType, const BDSP_InterTaskBuffer *pBufferHandle, unsigned *pInputIndex);
    BERR_Code (*addInterTaskBufferOutput)(void *pStageHandle, BDSP_DataType dataType, const BDSP_InterTaskBuffer *pBufferHandle, unsigned *pOutputIndex);
#if !B_REFSW_MINIMAL
    void (*removeOutput)(void *pStageHandle, unsigned outputIndex);
#endif /*!B_REFSW_MINIMAL*/
    void (*removeAllOutputs)(void *pStageHandle);

    BERR_Code (*addFmmInput)(void *pStageHandle, BDSP_DataType dataType, const BDSP_FmmBufferDescriptor *pDescriptor, unsigned *pOutputIndex);
    BERR_Code (*addSoftFmmInput)(void *pStageHandle, const BDSP_SoftFMMBufferDescriptor *pDescriptor, unsigned *pOutputIndex);
    BERR_Code (*addRaveInput)(void *pStageHandle, const BAVC_XptContextMap *pContext, unsigned *pOutputIndex);
    void (*removeInput)(void *pStageHandle, unsigned inputIndex);
    void (*removeAllInputs)(void *pStageHandle);

    BERR_Code (*addQueueOutput)(void *pStageHandle, void *pQueueHandle, unsigned *pOutputIndex);
#if !B_REFSW_MINIMAL
    BERR_Code (*addQueueInput)(void *pStageHandle, void *pQueueHandle, unsigned *pInputIndex);
#endif /*!B_REFSW_MINIMAL*/
    BERR_Code (*getVideoEncodeDatasyncSettings)(void *pStageHandle, BDSP_VideoEncodeTaskDatasyncSettings *pSettings);
    BERR_Code (*setVideoEncodeDatasyncSettings)(void *pStageHandle, const BDSP_VideoEncodeTaskDatasyncSettings *pSettings);
    BERR_Code (*getStageContext)(void *pStageHandle, BDSP_ContextHandle *pContextHandle);
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

	BERR_Code (*pingDsp)(void *pContextHandle);
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



BDBG_OBJECT_ID_DECLARE(BDSP_SoftFMM_Output);
typedef struct BDSP_SoftFMM_Output
{
    BDBG_OBJECT(BDSP_SoftFMM_Output)

    void *pSoftFMMOutput;

    void(*destroy)(void *pSoftFMMOutput);

    void(*getInterruptHandlers_isr)(void *pSoftFMM, BDSP_SoftFMMOutputInterruptHandlers *pHandlers);
    BERR_Code(*setInterruptHandlers_isr)(void *pSoftFMM, const BDSP_SoftFMMOutputInterruptHandlers *pHandlers);

    BERR_Code(*setBufferConfig)(void *pSoftFMMOutput, BDSP_SoftFMMBufferDescriptor  *pSoftFMMOutputBufferDescriptor, BDSP_SoftFMM_Output_HWConfig *pSoftFMMOutputHWConfig);

    BERR_Code(*getSettings)(void *pSoftFMMOutput, BDSP_SoftFMM_OutputSettings *pSoftFMMOutputSettings);
    BERR_Code(*setSettings)(void *pSoftFMMOutput, BDSP_SoftFMM_OutputSettings *pSoftFMMOutputSettings);
    BERR_Code(*setSettings_isr)(void *pSoftFMMOutput, BDSP_SoftFMM_OutputSettings *pSoftFMMOutputSettings);

    BERR_Code(*getStatus)(void *pSoftFMMOutput, BDSP_SoftFMM_OutputStatus *pSoftFMMOutputStatus);

}BDSP_SoftFMM_Output;


BDBG_OBJECT_ID_DECLARE(BDSP_SoftFMM_Input);
typedef struct BDSP_SoftFMM_Input
{
    BDBG_OBJECT(BDSP_SoftFMM_Input)

    void *pSoftFMMInput;

    void(*destroy)(void *pSoftFMMInput);

    void(*getInterruptHandlers_isr)(void *pSoftFMM, BDSP_SoftFMMInputInterruptHandlers *pHandlers);
    BERR_Code(*setInterruptHandlers_isr)(void *pSoftFMM, const BDSP_SoftFMMInputInterruptHandlers *pHandlers);

    BERR_Code(*getSettings)(void *pSoftFMMInput, BDSP_SoftFMM_InputSettings *pSoftFMMInputSettings);
    BERR_Code(*setSettings)(void *pSoftFMMInput, BDSP_SoftFMM_InputSettings *pSoftFMMInputSettings);
    BERR_Code(*setSettings_isr)(void *pSoftFMMInput, BDSP_SoftFMM_InputSettings *pSoftFMMInputSettings);

    BERR_Code(*getStatus)(void *pSoftFMMInput, BDSP_SoftFMM_InputStatus *pSoftFMMInputStatus);

}BDSP_SoftFMM_Input;

BDBG_OBJECT_ID_DECLARE(BDSP_SoftFMM_Mixer);
typedef struct BDSP_SoftFMM_Mixer
{
    BDBG_OBJECT(BDSP_SoftFMM_Mixer)

    void *pSoftFMMMixer;

    void(*destroy)(void *pSoftFMMMixer);

    BERR_Code(*start)(void *pSoftFMMMixer);
    BERR_Code(*stop)(void *pSoftFMMMixer);

    BERR_Code(*getStatus)(void *pSoftFMMMixer, BDSP_SoftFMM_MixerStatus *pSoftFMMMixerStatus);

    BERR_Code(*getSettings)(void *pSoftFMMMixer, BDSP_SoftFMM_MixerSettings *pSoftFMMMixerSettings);
    BERR_Code(*setSettings)(void *pSoftFMMMixer, BDSP_SoftFMM_MixerSettings *pSoftFMMMixerSettings);
    BERR_Code(*setSettings_isr)(void *pSoftFMMMixer, BDSP_SoftFMM_MixerSettings *pSoftFMMMixerSettings);

    BERR_Code(*addOutput)(void *pSoftFMMMixer, void *pSoftFMMOutput);
    void(*removeOutput)(void *pSoftFMMMixer, void *pSoftFMMOutput);
    void(*removeAllOutputs)(void *pSoftFMMMixer);

    BERR_Code(*addInput)(void *pSoftFMMMixer, void *pSoftFMMInput);
    void(*removeInput)(void *pSoftFMMMixer, void *pSoftFMMInput);
    void(*removeAllInputs)(void *pSoftFMMMixer);
}BDSP_SoftFMM_Mixer;


BDBG_OBJECT_ID_DECLARE(BDSP_SoftFMM);
typedef struct BDSP_SoftFMM
{
    BDBG_OBJECT(BDSP_SoftFMM)

    void *pSoftFMM;

    BERR_Code(*close)(void *pSoftFMM);

    BERR_Code(*getDefaultMixerSettings)(void *pSoftFMM, BDSP_SoftFMM_MixerSettings *pSoftFMMDefaultMixerSettings);
    BERR_Code(*createMixer)(void *pSoftFMM, const BDSP_SoftFMM_MixerSettings *pSoftFMMDefaultMixerSettings, BDSP_SoftFMMMixerHandle *pSoftFMMMixer);

    BERR_Code(*getDefaultOutputSettings)(void *pSoftFMM, BDSP_SoftFMM_OutputSettings *pSoftFMMDefaultOutputSettings);
    BERR_Code(*createOutput)(void *pSoftFMM, const BDSP_SoftFMM_OutputSettings *pSoftFMMDefaultOutputSettings, BDSP_SoftFMMOutputHandle *pSoftFMMOutput);

    BERR_Code(*getDefaultInputSettings)(void *pSoftFMM, BDSP_SoftFMM_InputSettings *pSoftFMMDefaultInputSettings);
    BERR_Code(*createInput)(void *pSoftFMM, const BDSP_SoftFMM_InputSettings *pSoftFMMDefaultInputSettings, BDSP_SoftFMMInputHandle * pSoftFMMInput);

}BDSP_SoftFMM;

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

void BDSP_P_InitSoftFMM(
    BDSP_SoftFMM *pSoftFMM,
    void *pSoftFMMHandle
    );

void BDSP_P_InitSoftFMMMixer(
    BDSP_SoftFMM_Mixer *pSoftFMMMixer,
    void *pSoftFMMMixerHandle
    );

void BDSP_P_InitSoftFMMInput(
    BDSP_SoftFMM_Input *pSoftFMMInput,
    void *pSoftFMMInputHandle
    );

void BDSP_P_InitSoftFMMOutput(
    BDSP_SoftFMM_Output *pSoftFMMOutput,
    void *pSoftFMMOutputHandle
    );

#endif /* #ifndef BDSP_PRIV_H_ */
