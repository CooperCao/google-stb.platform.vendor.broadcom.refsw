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


#ifndef BDSP_STAGE_H_
#define BDSP_STAGE_H_

#include "bdsp_raaga_fw_settings.h"
#include "bdsp_raaga_fw.h"
#include "bdsp_raaga_fw_status.h"
#include "bdsp_task.h"

/***************************************************************************
Summary:
Audio Capture Handle
***************************************************************************/
typedef struct BDSP_AudioCapture *BDSP_AudioCaptureHandle;

/***************************************************************************
Summary:
Audio Capture Create Settings
***************************************************************************/
typedef struct BDSP_AudioCaptureCreateSettings
{
    unsigned maxChannels;       /* Maximum number of channels to capture.  1 = mono/compressed.  2 = stereo.  6 = 5.1.  Default = 2. */
    size_t channelBufferSize;   /* Channel buffer size in bytes.  Default is 1536kB. */
	BMMA_Heap_Handle hHeap;     /* Memory Heap to use for allocating buffers.  If NULL, the default heap will be used. */
	BDSP_MMA_Memory Outputbuffer[BDSP_AF_P_MAX_CHANNELS]; /* Memory Allocated for the output buffer*/
} BDSP_AudioCaptureCreateSettings;

/***************************************************************************
Summary:
Queue Create Settings
***************************************************************************/
typedef struct BDSP_QueueCreateSettings
{
    BDSP_DataType dataType;
    unsigned numBuffers;
    struct
    {
        unsigned bufferSize;              /* Buffer length in bytes (must be a multiple of 4) */
		BDSP_MMA_Memory buffer;
    } bufferInfo[BDSP_AF_P_MAX_CHANNELS];
    bool input;                           /* True if the host will write to the queue instead of reading output */
} BDSP_QueueCreateSettings;

/***************************************************************************
Summary:
Buffer Descriptor
***************************************************************************/
typedef struct BDSP_BufferDescriptor
{
    bool interleaved;               /* If true, every other channel will have valid pointers,
                                       e.g. L for L/R, Ls for Ls/Rs, etc.  */
    unsigned numBuffers;            /* Number of buffers.  For mono/interleaved stereo this is 1.  For
                                       non-interleaved stereo, it's 2.  For non-interleaved 7.1 it's 8. */
    struct
    {
        BDSP_MMA_Memory buffer;     /* Buffer base address prior to wraparound */
        BDSP_MMA_Memory wrapBuffer; /* Buffer address after wraparound (NULL if no wrap has occurred) */
    } buffers[BDSP_AF_P_MAX_CHANNELS];

    unsigned bufferSize;            /* Buffer size before wraparound in bytes */
    unsigned wrapBufferSize;        /* Buffer size after wraparound in bytes */
} BDSP_BufferDescriptor;

/***************************************************************************
Summary:
Get default stage create settings
***************************************************************************/
void BDSP_Stage_GetDefaultCreateSettings(
    BDSP_ContextHandle hContext,
    BDSP_AlgorithmType algoType,
    BDSP_StageCreateSettings *pSettings /* [out] */
    );

/***************************************************************************
Summary:
Create a Stage
***************************************************************************/
BERR_Code BDSP_Stage_Create(
    BDSP_ContextHandle hContext,
    const BDSP_StageCreateSettings *pSettings,
    BDSP_StageHandle *pStage /* [out] */
    );

/***************************************************************************
Summary:
Destory a Stage
***************************************************************************/
void BDSP_Stage_Destroy(
    BDSP_StageHandle hStage
    );

/***************************************************************************
Summary:
Set the algorithm for a stage
***************************************************************************/
BERR_Code BDSP_Stage_SetAlgorithm(
    BDSP_StageHandle hStage,
    BDSP_Algorithm algorithm
    );

/***************************************************************************
Summary:
Get settings for a particular stage
***************************************************************************/
BERR_Code BDSP_Stage_GetSettings(
    BDSP_StageHandle hStage,
    void *pSettingsBuffer,        /* [out] */
    size_t settingsBufferSize     /* [in] */
    );

/***************************************************************************
Summary:
Set settings for a particular stage
***************************************************************************/
BERR_Code BDSP_Stage_SetSettings(
    BDSP_StageHandle hStage,
    const void *pSettingsBuffer,    /* [out] */
    size_t settingsBufferSize       /* [in] */
    );

/***************************************************************************
Summary:
Get status from a particular stage
***************************************************************************/
BERR_Code BDSP_Stage_GetStatus(
    BDSP_StageHandle hStage,
    void *pStatusBuffer,        /* [out] */
    size_t statusBufferSize     /* [in] */
    );

/***************************************************************************
Summary:
Add an FMM output to a stage
***************************************************************************/
BERR_Code BDSP_Stage_AddFmmOutput(
    BDSP_StageHandle hStage,
    BDSP_DataType dataType,
    const BDSP_FmmBufferDescriptor *pDescriptor,
    unsigned *pOutputIndex
    );

/***************************************************************************
Summary:
Add a Soft FMM output to a stage
***************************************************************************/
BERR_Code BDSP_Stage_AddSoftFmmOutput(
	BDSP_StageHandle hStage,
	const BDSP_SoftFMMBufferDescriptor *pDescriptor,
	unsigned *pOutputIndex
);

/***************************************************************************
Summary:
Add a Rave output to a stage
***************************************************************************/
BERR_Code BDSP_Stage_AddRaveOutput(
    BDSP_StageHandle hStage,
    const BAVC_XptContextMap *pContext,
    unsigned *pOutputIndex /* [out] */
    );

/***************************************************************************
Summary:
Connect an output stage to a stage
***************************************************************************/
BERR_Code BDSP_Stage_AddOutputStage(
    BDSP_StageHandle hSourceStage,
    BDSP_DataType dataType,
    BDSP_StageHandle hDestinationStage,
    unsigned *pSourceOutputIndex, /* [out] */    /* Output index, reflects the source stage's output index for the destination stage. */
    unsigned *pDestinationInputIndex /* [out] */    /* Input index, reflects the destination stage's input index for the source stage. */
    );

/***************************************************************************
Summary:
Connect an inter task buffer at output
***************************************************************************/
BERR_Code BDSP_Stage_AddInterTaskBufferOutput(
    BDSP_StageHandle hStage,
    BDSP_DataType dataType,
    BDSP_InterTaskBufferHandle hInterTaskBuffer,    /* inter-task buffer handle */
    unsigned *pOutputIndex /* [out] */    /* Output index, reflects the source stage's output index for the inter task buffer. */
    );

/***************************************************************************
Summary:
Connect an inter task buffer at input
***************************************************************************/
BERR_Code BDSP_Stage_AddInterTaskBufferInput(
    BDSP_StageHandle hStage,
    BDSP_DataType dataType,
    BDSP_InterTaskBufferHandle hInterTaskBuffer,    /* inter-task buffer handle */
    unsigned *pInputIndex /* [out] */    /* input index, reflects the source stage's input index for the inter task buffer. */
    );

#if !B_REFSW_MINIMAL
/***************************************************************************
Summary:
Remove an output from a stage
***************************************************************************/
void BDSP_Stage_RemoveOutput(
    BDSP_StageHandle hStage,
    unsigned outputIndex
    );
#endif /*!B_REFSW_MINIMAL*/

/***************************************************************************
Summary:
Remove all outputs from a stage
***************************************************************************/
void BDSP_Stage_RemoveAllOutputs(
    BDSP_StageHandle hStage
    );

/***************************************************************************
Summary:
Add an FMM Input to a stage
***************************************************************************/
BERR_Code BDSP_Stage_AddFmmInput(
    BDSP_StageHandle hStage,
    BDSP_DataType dataType,
    const BDSP_FmmBufferDescriptor *pDescriptor,
    unsigned *pInputIndex
    );

/***************************************************************************
Summary:
Add a Soft FMM Input to a stage
***************************************************************************/
BERR_Code BDSP_Stage_AddSoftFmmInput(
	BDSP_StageHandle hStage,
	const BDSP_SoftFMMBufferDescriptor *pDescriptor,
	unsigned *pInputIndex
);

/***************************************************************************
Summary:
Add a rave input to a stage
***************************************************************************/
BERR_Code BDSP_Stage_AddRaveInput(
    BDSP_StageHandle hStage,
    const BAVC_XptContextMap *pContext,
    unsigned *pInputIndex /* [out] */
    );

/***************************************************************************
Summary:
Remove an input to a stage
***************************************************************************/
void BDSP_Stage_RemoveInput(
    BDSP_StageHandle hStage,
    unsigned inputIndex
    );

/***************************************************************************
Summary:
Remove all inputs to a stage
***************************************************************************/
void BDSP_Stage_RemoveAllInputs(
    BDSP_StageHandle hStage
    );

/***************************************************************************
Summary:
Get TSM settings for a particular stage
***************************************************************************/
BERR_Code BDSP_AudioStage_GetTsmSettings_isr(
    BDSP_StageHandle stage,
    BDSP_AudioTaskTsmSettings *pTsmSettings         /* [out] */
    );

/***************************************************************************
Summary:
Set TSM settings for a particular stage
***************************************************************************/
BERR_Code BDSP_AudioStage_SetTsmSettings_isr(
    BDSP_StageHandle stage,
    const BDSP_AudioTaskTsmSettings *pTsmSettings
    );

/***************************************************************************
Summary:
Get TSM status from a particular stage
***************************************************************************/
BERR_Code BDSP_AudioStage_GetTsmStatus_isr(
    BDSP_StageHandle stage,
    BDSP_AudioTaskTsmStatus *pTsmStatus         /* [out] */
    );

/***************************************************************************
Summary:
Get Datasync settings for a particular stage
***************************************************************************/
BERR_Code BDSP_AudioStage_GetDatasyncSettings(
    BDSP_StageHandle stage,
    BDSP_AudioTaskDatasyncSettings *pDatasyncSettings         /* [out] */
    );
BERR_Code BDSP_AudioStage_GetDatasyncSettings_isr(
    BDSP_StageHandle stage,
    BDSP_AudioTaskDatasyncSettings *pDatasyncSettings         /* [out] */
    );

/***************************************************************************
Summary:
Set Datasync settings for a particular stage
***************************************************************************/
BERR_Code BDSP_AudioStage_SetDatasyncSettings(
    BDSP_StageHandle stage,
    const BDSP_AudioTaskDatasyncSettings *pDatasyncSettings
    );

/***************************************************************************
Summary:
Get Datasync status from a particular stage
***************************************************************************/
BERR_Code BDSP_AudioStage_GetDatasyncStatus_isr(
    BDSP_StageHandle stage,
    BDSP_AudioTaskDatasyncStatus *pDatasyncStatus         /* [out] */
    );

/* Inter task buffer apis */
/***************************************************************************
Summary:
Create an inter task buffer
***************************************************************************/
BERR_Code BDSP_InterTaskBuffer_Create(
    BDSP_ContextHandle hContext,
    BDSP_DataType dataType,
    BDSP_BufferType bufferType,
    BDSP_InterTaskBufferHandle *pHandle /* [out] */
    );

/***************************************************************************
Summary:
Destroy an inter task buffer
***************************************************************************/
void BDSP_InterTaskBuffer_Destroy(
    BDSP_InterTaskBufferHandle hInterTaskBuffer
    );

/***************************************************************************
Summary:
Flush an inter task buffer
***************************************************************************/
void BDSP_InterTaskBuffer_Flush(
    BDSP_InterTaskBufferHandle hInterTaskBuffer
    );

/*Queue apis */
/***************************************************************************
Summary:
Default Values for Queue Settings
***************************************************************************/
BERR_Code BDSP_Queue_GetDefaultSettings(
                    BDSP_ContextHandle hContext,
                    BDSP_QueueCreateSettings *pSettings);

/***************************************************************************
Summary:
Create an Queue
***************************************************************************/
BERR_Code BDSP_Queue_Create(
    BDSP_ContextHandle hContext,
    unsigned dspIndex,
    BDSP_QueueCreateSettings *pSettings,
    BDSP_QueueHandle *pHandle /* [out] */
    );

/***************************************************************************
Summary:
Destroy an Queue
***************************************************************************/
void BDSP_Queue_Destroy(
    BDSP_QueueHandle hQueue
    );

/***************************************************************************
Summary:
Flush an Queue
***************************************************************************/
void BDSP_Queue_Flush(
    BDSP_QueueHandle hQueue
    );

#if !B_REFSW_MINIMAL
/***************************************************************************
Summary:
Add an Queue as Input
***************************************************************************/
BERR_Code BDSP_Stage_AddQueueInput(
    BDSP_StageHandle hStage,
    BDSP_QueueHandle hQueue,    /*Queue handle */
    unsigned *pInputIndex /* [out] */    /* input index, reflects the source stage's input index for the queue */
    );
#endif /* !B_REFSW_MINIMAL*/

/***************************************************************************
Summary:
Add an Queue as Output
***************************************************************************/
BERR_Code BDSP_Stage_AddQueueOutput(
    BDSP_StageHandle hStage,
    BDSP_QueueHandle hQueue,    /*Queue handle */
    unsigned *pOutputIndex /* [out] */    /* output index, reflects the source stage's output index for the queue. */
    );

/***************************************************************************
Summary:
Get Default Audio Capture Create Settings
***************************************************************************/
void BDSP_AudioCapture_GetDefaultCreateSettings(
    BDSP_AudioCaptureCreateSettings *pSettings  /* [out] */
    );

BERR_Code BDSP_Stage_GetContext(
    BDSP_StageHandle hStage,
    BDSP_ContextHandle *pContextHandle /* [out] */
    );

/***************************************************************************
Summary:
Get Datasync settings for a particular task of video Encode type
***************************************************************************/
BERR_Code BDSP_VideoEncodeTask_GetDatasyncSettings(
    BDSP_StageHandle stage,
    BDSP_VideoEncodeTaskDatasyncSettings *pDataSyncSettings         /* [out] */
    );

/***************************************************************************
Summary:
Set Datasync settings for a particular task of video Encode type
***************************************************************************/
BERR_Code BDSP_VideoEncodeTask_SetDatasyncSettings(
    BDSP_StageHandle stage,
    const BDSP_VideoEncodeTaskDatasyncSettings *pDataSyncSettings
    );

/***************************************************************************
Summary:
Create an audio capture context

Description:
An Audio Capture object is used to capture a copy of the DSP data into host
memory prior to being consumed by intended consumer (e.g. audio mixer hardware).
For the data to be copied, a thread must call BDSP_ProcessAudioCapture() on a
background thread at a frequent interval (e.g. 10ms).

To bind a capture handle to a task and output, you must call
BDSP_AudioCapture_Add before calling BDSP_Task_Start.  After
BDSP_Task_Stop has been called, BDSP_AudioCapture_Remove should
also be called.
***************************************************************************/
BERR_Code BDSP_AudioCapture_Create(
    BDSP_ContextHandle hContext,
    const BDSP_AudioCaptureCreateSettings *pSettings,
    BDSP_AudioCaptureHandle *pCapture   /* [out] */
    );

/***************************************************************************
Summary:
Destroy an audio capture context
***************************************************************************/
void BDSP_AudioCapture_Destroy(
    BDSP_AudioCaptureHandle hCapture
    );

/***************************************************************************
Summary:
Settings to add a capture handle to a task
***************************************************************************/
typedef struct BDSP_StageChannelPairInfo{
	unsigned  bufferSize; /*Size of the channel Pair */
	BDSP_MMA_Memory outputBuffer; /* Memory Allocated for the channel Pair*/
} BDSP_StageChannelPairInfo;

typedef struct BDSP_StageAudioCaptureSettings
{
    bool updateRead; /* This flag is enabled when there is no consumer for the output buffer
                       and the capture thread is expected to update the buffer read pointers */
	unsigned  numChannelPair; /*Num of channel Pair */
	BDSP_StageChannelPairInfo channelPairInfo[BDSP_AF_P_MAX_CHANNEL_PAIR]; /* Memory Info of the channel Pair */
} BDSP_StageAudioCaptureSettings;


/***************************************************************************
Summary:
Get Default settings to add a capture handle to a task
***************************************************************************/
void BDSP_Stage_GetDefaultAudioCaptureSettings(
    BDSP_StageAudioCaptureSettings *pSettings /* [out] */
    );

/***************************************************************************
Summary:
Add an audio capture handle to a task

Description:
An Audio Capture object is used to capture a copy of the DSP data into host
memory prior to being consumed by intended consumer (e.g. audio mixer hardware).
For the data to be copied, a thread must call BDSP_ProcessAudioCapture() on a
background thread at a frequent interval (e.g. 10ms).

To bind a capture handle to a task and output, you must call
BDSP_AudioCapture_Add before calling BDSP_Task_Start.  After
BDSP_Task_Stop has been called, BDSP_AudioCapture_Remove should
also be called.
***************************************************************************/
BERR_Code BDSP_Stage_AddAudioCapture(
    BDSP_StageHandle hStage,
    BDSP_AudioCaptureHandle hCapture,
    unsigned outputId,                  /* The output port number that needs to be captured */
    const BDSP_StageAudioCaptureSettings *pSettings  /* Optional, pass NULL for defaults */
    );

/***************************************************************************
Summary:
Remove an audio capture handle from a task
***************************************************************************/
void BDSP_Stage_RemoveAudioCapture(
    BDSP_StageHandle hStage,
    BDSP_AudioCaptureHandle hCapture
    );


/***************************************************************************
Summary:
Get the Buffer Descriptors
***************************************************************************/
BERR_Code BDSP_AudioCapture_GetBuffer(
    BDSP_AudioCaptureHandle hCapture,
    BDSP_BufferDescriptor *pBuffers /* [out] */
    );

/***************************************************************************
Summary:
Consume data from the capture buffers
***************************************************************************/
BERR_Code BDSP_AudioCapture_ConsumeData(
    BDSP_AudioCaptureHandle hCapture,
    unsigned numBytes                   /* Number of bytes read from each buffer */
    );


/***************************************************************************
Summary:
Get Circular Buffer Pointers of Queue for use in algorithm settings.
***************************************************************************/
void BDSP_Queue_GetIoBuffer(
    BDSP_QueueHandle hQueue,
    BDSP_AF_P_sIO_BUFFER *pBuffer /*[out]*/
    );

/***************************************************************************
Summary:
Get the buffer address(es) and size available to read data from the queue.
***************************************************************************/
BERR_Code BDSP_Queue_GetBuffer(
    BDSP_QueueHandle hQueue,
    BDSP_BufferDescriptor *pDescriptor /*[out] */
);

/***************************************************************************
Summary:
Advances read pointer of Queue on checking the BDSP_DataType. Called each
time host reads data from the queue.  If more than one buffer is returned, each buffer is
advanced by the number of bytes specified in readBytes.
***************************************************************************/
BERR_Code BDSP_Queue_ConsumeData(
    BDSP_QueueHandle hQueue,
    size_t readBytes
    );

/***************************************************************************
Summary:
Advances write pointer of Queue on checking the BDSP_DataType. Called each
time host puts data into the queue.  If more than one buffer is returned, each buffer is
advanced by the number of bytes specified in bytesWritten.
***************************************************************************/
BERR_Code BDSP_Queue_CommitData(
    BDSP_QueueHandle hQueue,
    size_t bytesWritten
    );

/***************************************************************************
Summary:
Returns the addresses of the Buffer which is stored as part of RDB by the BDSP back to the VEE
***************************************************************************/
BERR_Code BDSP_Queue_GetBufferAddr(
    BDSP_QueueHandle hQueue,
    unsigned numbuf,
    void *pBuffer
    );


/*********************************************************************
Summary:
    This structure contain elements that is returned by
        CalcThreshold_BlockTime_AudOffset API.

Description:

        ui32Threshold and ui32BlockTime goes to FW and
        ui32AudOffset goes to Application and TSM user cfg

See Also:
**********************************************************************/
typedef struct BDSP_CTB_Output
{
    uint32_t ui32Threshold;                                 /* Interms of samples */
    uint32_t ui32BlockTime;                                 /* Interms of Time (msec)  */
    uint32_t ui32AudOffset;                                 /* AudOffset in Time (msec) */
}BDSP_CTB_Output;

typedef struct BDSP_CTB_Input
{
    BDSP_AudioTaskDelayMode 		audioTaskDelayMode;
    BDSP_TaskRealtimeMode 			realtimeMode;
    BDSP_Audio_AudioInputSource     eAudioIpSourceType;           /* Capture port Type    */
}BDSP_CTB_Input;

BERR_Code BDSP_GetAudioDelay_isrsafe(
    BDSP_CTB_Input   *pCtbInput,
    BDSP_StageHandle  hStage,
    BDSP_CTB_Output  *pCTBOutput
);
#endif /* BDSP_STAGE_H_ */
