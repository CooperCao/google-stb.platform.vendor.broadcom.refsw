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
*   API name: Audio Module
*    Module includes
*
***************************************************************************/

#include "nexus_audio_module.h"
#include "nexus_audio_capture_priv.h"

BDBG_MODULE(nexus_audio_input_capture);

#if NEXUS_NUM_AUDIO_INPUT_CAPTURES

typedef struct NEXUS_AudioInputCapture
{
    NEXUS_OBJECT(NEXUS_AudioInputCapture);
    NEXUS_AudioInputObject connector;
    BAPE_InputCaptureHandle apeHandle;
    NEXUS_AudioInputCaptureStartSettings startSettings;
    NEXUS_AudioInputCaptureSettings settings;
    BAPE_InputPort inputPort;
    unsigned index;
    bool opened;
    bool running;
    BMMA_Block_Handle fifoBlock;
    void *pMemory;
    NEXUS_AudioCaptureProcessorHandle procHandle;
    BKNI_EventHandle inputFormatChangeEvent;
    NEXUS_EventCallbackHandle inputFormatChangeEventHandler;
    NEXUS_TaskCallbackHandle sourceChangedCallback;
    NEXUS_IsrCallbackHandle dataCallback;
    NEXUS_AudioCaptureFormat format;
    bool compressed;
    bool hbr;
    unsigned numPcmChannels;
    size_t fifoSize;
    char name[16];   /* INPUT CAPTURE %d */
    BMMA_Heap_Handle fifoMem; /* heap used for fifo */
} NEXUS_AudioInputCapture;

static NEXUS_AudioInputCapture g_inputCaptures[NEXUS_MAX_AUDIO_INPUT_CAPTURES];

static void NEXUS_AudioInputCapture_P_FlushDeviceBuffer_isr(NEXUS_AudioInputCaptureHandle handle);
static void NEXUS_AudioInputCapture_P_DataInterrupt_isr(void *pParam, int param);
static void NEXUS_AudioInputCapture_P_InputFormatChange_isr(void *pParam1, int param2);
static void NEXUS_AudioInputCapture_P_InputCaptureHalted_isr(void *pParam1, int param2);
static void NEXUS_AudioInputCapture_P_InputFormatChange(void *pContext);
static void NEXUS_AudioInputCapture_P_SetConnectorFormat(NEXUS_AudioInputCaptureHandle handle, bool compressed, bool hbr, unsigned numPcmChannels);

/***************************************************************************
Summary:
Get default Open-time settings for AudioInputCapture
***************************************************************************/
void NEXUS_AudioInputCapture_GetDefaultOpenSettings(
    NEXUS_AudioInputCaptureOpenSettings *pSettings  /* [out] */
    )
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
}

static BERR_Code NEXUS_AudioCapture_P_Input_GetBuffer(
    void * hCapture,
    void * pBuffers      /* [out] */
    )
{
    return BAPE_InputCapture_GetBuffer((BAPE_InputCaptureHandle)hCapture, (BAPE_BufferDescriptor*)pBuffers);
}

static BERR_Code NEXUS_AudioCapture_P_Input_ConsumeData(
    void * hCapture,
    unsigned numBytes                   /* Number of bytes read from the buffer */
    )
{
    return BAPE_InputCapture_ConsumeData((BAPE_InputCaptureHandle)hCapture, numBytes);
}

/***************************************************************************
Summary:
Open an AudioInputCapture handle
***************************************************************************/
NEXUS_AudioInputCaptureHandle NEXUS_AudioInputCapture_Open(
    unsigned index,
    const NEXUS_AudioInputCaptureOpenSettings *pSettings
    )
{
    NEXUS_AudioInputCaptureHandle handle;
    NEXUS_AudioInputCaptureOpenSettings defaults;
    BAPE_InputCaptureOpenSettings openSettings;
    BAPE_Connector connector;
    NEXUS_HeapHandle heap;
    NEXUS_AudioCaptureProcessorHandle procHandle = NULL;
    BERR_Code errCode;
    unsigned i;
    NEXUS_AudioCapabilities audioCapabilities;

    NEXUS_GetAudioCapabilities(&audioCapabilities);


    if ( index >= audioCapabilities.numInputCaptures )
    {
        BDBG_ERR(("Invalid input capture index %u.  This platform only supports %u input captures.", index, audioCapabilities.numInputCaptures));
        (void)BERR_TRACE(BERR_INVALID_PARAMETER);
        return NULL;
    }
    handle = &g_inputCaptures[index];
    if ( handle->opened )
    {
        BDBG_ERR(("Input capture %u is already open.", index));
        (void)BERR_TRACE(BERR_INVALID_PARAMETER);
        return NULL;
    }

    if ( NULL == pSettings )
    {
        NEXUS_AudioInputCapture_GetDefaultOpenSettings(&defaults);
        pSettings = &defaults;
    }


    NEXUS_OBJECT_INIT(NEXUS_AudioInputCapture, handle);
    handle->index = index;
    handle->opened = true;
    BKNI_Snprintf(handle->name, sizeof(handle->name), "INPUT CAPTURE %d", index);
    NEXUS_AUDIO_INPUT_INIT(&handle->connector, NEXUS_AudioInputType_eInputCapture, handle);
    NEXUS_OBJECT_REGISTER(NEXUS_AudioInput, &handle->connector, Open);
    handle->connector.pName = handle->name;
    NEXUS_CallbackDesc_Init(&handle->settings.sourceChanged);
    NEXUS_CallbackDesc_Init(&handle->settings.dataCallback);



    BAPE_InputCapture_GetDefaultOpenSettings(&openSettings);
    /* if we are capturing to memory, set up our data processor */
    if ( pSettings->fifoSize )
    {
        switch ( pSettings->multichannelFormat )
        {
        case NEXUS_AudioMultichannelFormat_eStereo:
            BDBG_MSG(("Opening Capture to Memory in stereo Mode"));
            openSettings.numBuffers = 1;
            openSettings.bufferSize = pSettings->fifoSize;
            break;
        case NEXUS_AudioMultichannelFormat_e5_1:
            BDBG_MSG(("Opening Capture to Memory in 6 ch Mode"));
            openSettings.numBuffers = 3;
            openSettings.bufferSize = pSettings->fifoSize/3;
            break;
        default:
            BDBG_ERR(("Unsupported multichannel format %u", pSettings->multichannelFormat));
            (void)BERR_TRACE(BERR_INVALID_PARAMETER);
            goto err_param;
        }
        if ( pSettings->threshold )
        {
            switch ( pSettings->format )
            {
            case NEXUS_AudioCaptureFormat_e16BitStereo:
                openSettings.watermarkThreshold = (pSettings->threshold * 2) & (~255);
                break;
            case NEXUS_AudioCaptureFormat_e24BitStereo:
                openSettings.watermarkThreshold = pSettings->threshold & (~255);
                break;
            case NEXUS_AudioCaptureFormat_e16BitMonoLeft:
            case NEXUS_AudioCaptureFormat_e16BitMonoRight:
            case NEXUS_AudioCaptureFormat_e16BitMono:
                openSettings.watermarkThreshold = (pSettings->threshold * 4) & (~255);
                break;
            case NEXUS_AudioCaptureFormat_e24Bit5_1:
                openSettings.watermarkThreshold = (pSettings->threshold / 3) & (~255);
                break;
            default:
                BDBG_ERR(("Unsupported capture format %u", pSettings->format));
                (void)BERR_TRACE(BERR_INVALID_PARAMETER);
                goto err_param;
            }
            if ( openSettings.watermarkThreshold > openSettings.bufferSize/2 )
            {
                BDBG_WRN(("Capture watermark threshold is greater than half the fifo size"));
                openSettings.watermarkThreshold = openSettings.bufferSize/2 & (~255);
            }
        }

        procHandle = BKNI_Malloc(sizeof(NEXUS_AudioCaptureProcessor));
        if ( !procHandle )
        {
            (void)BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
            goto err_param;
        }
        BKNI_Memset(procHandle, 0, sizeof(*procHandle));

        BDBG_MSG(("%s - Memory Capture mode, allocating resources", BSTD_FUNCTION));
        heap = NEXUS_P_DefaultHeap(pSettings->heap, NEXUS_DefaultHeapType_eFull);
        if ( NULL == heap )
        {
            heap = g_pCoreHandles->heap[g_pCoreHandles->defaultHeapIndex].nexus;
        }
        if (!NEXUS_P_CpuAccessibleHeap(heap))
        {
            BDBG_ERR(("Capture heap is not CPU accessible.  Please provide a CPU-accessible heap in NEXUS_AudioCaptureOpenSettings."));
            (void)BERR_TRACE(NEXUS_INVALID_PARAMETER);
            goto err_heap;
        }
        handle->fifoMem = NEXUS_Heap_GetMmaHandle(heap);

        handle->dataCallback = NEXUS_IsrCallback_Create(handle, NULL);
        if ( NULL == handle->dataCallback )
        {
            (void)BERR_TRACE(BERR_OS_ERROR);
            goto err_data_callback;
        }
        handle->fifoBlock = BMMA_Alloc(handle->fifoMem, pSettings->fifoSize, 0, NULL);
        if ( NULL == handle->fifoBlock )
        {
            (void)BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
            goto err_buffer_alloc;
        }

        handle->pMemory = BMMA_Lock(handle->fifoBlock);
        if ( NULL == handle->pMemory )
        {
            (void)BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
            goto err_buffer_alloc;
        }

        /* save the pi buffer size and threshold */
        handle->fifoSize = openSettings.bufferSize;
        handle->settings.threshold = openSettings.watermarkThreshold;
        handle->format = pSettings->format;

    }
    else
    {
        BDBG_MSG(("%s - Capture to FMM mode", BSTD_FUNCTION));
    }

    for ( i = 0; i < NEXUS_AudioChannel_eMax; i++ )
    {
        handle->settings.volumeMatrix[i][i] = NEXUS_AUDIO_VOLUME_LINEAR_NORMAL;
    }
    errCode = BAPE_InputCapture_Open(NEXUS_AUDIO_DEVICE_HANDLE,
                                     NEXUS_AUDIO_CAPTURE_INDEX(NEXUS_AudioInputType_eInputCapture, index),
                                     &openSettings,
                                     &handle->apeHandle);
    if ( errCode )
    {
        (void)BERR_TRACE(errCode);
        goto err_ape_open;
    }
    BAPE_InputCapture_GetConnector(handle->apeHandle, &connector);
    handle->connector.port = (size_t)connector;

    errCode = BKNI_CreateEvent(&handle->inputFormatChangeEvent);
    if ( errCode )
    {
        errCode=BERR_TRACE(errCode);
        goto err_input_format_change_event;
    }

    handle->inputFormatChangeEventHandler = NEXUS_RegisterEvent(handle->inputFormatChangeEvent, NEXUS_AudioInputCapture_P_InputFormatChange, handle);
    if ( NULL == handle->inputFormatChangeEventHandler )
    {
        errCode=BERR_TRACE(BERR_OS_ERROR);
        goto err_input_format_change_event_handler;
    }

    handle->sourceChangedCallback = NEXUS_TaskCallback_Create(handle, NULL);
    if ( NULL == handle->sourceChangedCallback )
    {
       errCode = BERR_TRACE(BERR_OS_ERROR);
       goto err_source_change_callback;
    }

    if ( procHandle )
    {
        /* Set up the Capture Processor */
        procHandle->piHandle = handle->apeHandle;
        procHandle->pBuffer = handle->pMemory;
        procHandle->bufferSize = pSettings->fifoSize;
        procHandle->getBuffer = NEXUS_AudioCapture_P_Input_GetBuffer;
        procHandle->consumeData = NEXUS_AudioCapture_P_Input_ConsumeData;
        procHandle->format = handle->format;
        handle->connector.format = NEXUS_AudioInputFormat_ePcmStereo;
        handle->procHandle = procHandle;
    }

    return handle;

err_source_change_callback:
    NEXUS_UnregisterEvent(handle->inputFormatChangeEventHandler);
err_input_format_change_event_handler:
    BKNI_DestroyEvent(handle->inputFormatChangeEvent);
err_input_format_change_event:
err_buffer_alloc:
    if ( handle->pMemory )
    {
        BMMA_Unlock(handle->fifoBlock, handle->pMemory);
        handle->pMemory = NULL;
    }
    if ( handle->fifoBlock )
    {
        BMMA_Free(handle->fifoBlock);
        handle->fifoBlock = NULL;
    }
    if ( handle->dataCallback )
    {
        NEXUS_IsrCallback_Destroy(handle->dataCallback);
    }
    BAPE_InputCapture_Close(handle->apeHandle);
err_data_callback:
err_heap:
    BKNI_Free(procHandle);
err_param:
    NEXUS_OBJECT_UNSET(NEXUS_AudioInputCapture, handle);

err_ape_open:
    BKNI_Memset(handle, 0, sizeof(NEXUS_AudioInputCapture));
    return NULL;
}

static void NEXUS_AudioInputCapture_P_Finalizer(
    NEXUS_AudioInputCaptureHandle handle
    )
{
    NEXUS_OBJECT_ASSERT(NEXUS_AudioInputCapture, handle);
    NEXUS_AudioInput_Shutdown(&handle->connector);
    if ( handle->running )
    {
        NEXUS_AudioInputCapture_Stop(handle);
    }
    NEXUS_AudioInput_Shutdown(&handle->connector);

    BAPE_InputCapture_Close(handle->apeHandle);
    NEXUS_TaskCallback_Destroy(handle->sourceChangedCallback);
    NEXUS_UnregisterEvent(handle->inputFormatChangeEventHandler);
    BKNI_DestroyEvent(handle->inputFormatChangeEvent);
    if ( handle->dataCallback )
    {
        NEXUS_IsrCallback_Destroy(handle->dataCallback);
    }
    if ( handle->pMemory )
    {
        BMMA_Unlock(handle->fifoBlock, handle->pMemory);
        handle->pMemory = NULL;
    }
    if ( handle->fifoBlock )
    {
        BMMA_Free(handle->fifoBlock);
        handle->fifoBlock = NULL;
    }
    if ( handle->procHandle )
    {
        BKNI_Free(handle->procHandle);
    }
    NEXUS_OBJECT_DESTROY(NEXUS_AudioInputCapture, handle);
    BKNI_Memset(handle, 0, sizeof(NEXUS_AudioInputCapture));    /* Destroys magic numbers and marks as free */
}

static void NEXUS_AudioInputCapture_P_Release(NEXUS_AudioInputCaptureHandle handle)
{
    NEXUS_OBJECT_UNREGISTER(NEXUS_AudioInput, &handle->connector, Close);
    return;
}

NEXUS_OBJECT_CLASS_MAKE_WITH_RELEASE(NEXUS_AudioInputCapture, NEXUS_AudioInputCapture_Close);

/***************************************************************************
Summary:
Get Default Start-time settings for AudioInputCapture
***************************************************************************/
void NEXUS_AudioInputCapture_GetDefaultStartSettings(
    NEXUS_AudioInputCaptureStartSettings *pSettings /* [out] */
    )
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
}

#if NEXUS_HAS_HDMI_INPUT
#include "priv/nexus_hdmi_input_priv.h"
static void NEXUS_P_AudioInputCapture_ConnectHdmiInput(NEXUS_AudioInputCaptureHandle handle, bool connected)
{
    if (handle->startSettings.input) {
        NEXUS_HdmiInputHandle hdmiInput = handle->startSettings.input->pObjectHandle;
        if (handle->startSettings.input->objectType == NEXUS_AudioInputType_eHdmi && hdmiInput) {
            NEXUS_Module_Lock(g_NEXUS_audioModuleData.internalSettings.modules.hdmiInput);
            NEXUS_HdmiInput_AudioConnected_priv(hdmiInput, connected);
            NEXUS_Module_Unlock(g_NEXUS_audioModuleData.internalSettings.modules.hdmiInput);
        }
        else if (handle->startSettings.input->objectType == NEXUS_AudioInputType_eHdmi)
        {
            BERR_TRACE(NEXUS_INVALID_PARAMETER);
        }
    }
}
#else
#define NEXUS_P_AudioInputCapture_ConnectHdmiInput(handle, connected)
#endif

/***************************************************************************
Summary:
Start capturing input data
***************************************************************************/
NEXUS_Error NEXUS_AudioInputCapture_Start(
    NEXUS_AudioInputCaptureHandle handle,
    const NEXUS_AudioInputCaptureStartSettings *pSettings
    )
{
    NEXUS_Error errCode;
    BAPE_InputCaptureStartSettings apeStartSettings;
    BAPE_InputCaptureInterruptHandlers inputCaptureInterrupts;
    BAPE_InputPort inputPort;

    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioInputCapture);

    BDBG_ASSERT(NULL != pSettings);
    BDBG_ASSERT(NULL != pSettings->input);

    inputPort = NEXUS_AudioInput_P_GetInputPort(pSettings->input);
    if ( inputPort == 0 )
    {
        BDBG_ERR(("Invalid input specified to NEXUS_AudioInputCapture"));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    if ( handle->running )
    {
        BDBG_ERR(("Input Capture %u already started", handle->index));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    handle->startSettings = *pSettings;
    handle->inputPort = inputPort;
    if ( NEXUS_AudioInput_P_SupportsFormatChanges(pSettings->input) )
    {
        NEXUS_AudioInputPortStatus inputPortStatus;

        errCode = NEXUS_AudioInput_P_SetFormatChangeInterrupt(pSettings->input, NEXUS_AudioInputType_eInputCapture, NEXUS_AudioInputCapture_P_InputFormatChange_isr, handle, 0);
        if ( errCode )
        {
            errCode = BERR_TRACE(errCode);
            goto err_format_interrupt;
        }

        errCode = NEXUS_AudioInput_P_GetInputPortStatus(pSettings->input, &inputPortStatus);
        if ( errCode )
        {
            errCode = BERR_TRACE(errCode);
            goto err_status;
        }

        handle->compressed = inputPortStatus.compressed;
        handle->numPcmChannels = inputPortStatus.numPcmChannels;
        handle->hbr = inputPortStatus.hbr;

        /* Set output data type correctly */
        NEXUS_AudioInputCapture_P_SetConnectorFormat(handle, inputPortStatus.compressed, inputPortStatus.hbr, inputPortStatus.numPcmChannels);
    }

    if ( handle->procHandle )
    {
        /* register the data ready interrupt */
        NEXUS_IsrCallback_Set(handle->dataCallback, &handle->settings.dataCallback);
    }

    errCode = NEXUS_AudioInput_P_PrepareToStart(&handle->connector);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        goto err_prepare_to_start;
    }

    BAPE_InputCapture_GetInterruptHandlers(handle->apeHandle, &inputCaptureInterrupts);
    inputCaptureInterrupts.inputHalted.pCallback_isr = NEXUS_AudioInputCapture_P_InputCaptureHalted_isr;
    inputCaptureInterrupts.inputHalted.pParam1 = handle;
    if ( handle->procHandle )
    {
        BDBG_MSG(("Register APE Data Interrupt"));
        inputCaptureInterrupts.watermark.pCallback_isr = NEXUS_AudioInputCapture_P_DataInterrupt_isr;
        inputCaptureInterrupts.watermark.pParam1 = handle;
    }
    errCode = BAPE_InputCapture_SetInterruptHandlers(handle->apeHandle, &inputCaptureInterrupts);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        goto err_set_interrupts;
    }

    BAPE_InputCapture_GetDefaultStartSettings(&apeStartSettings);
    apeStartSettings.input = inputPort;
    errCode = BAPE_InputCapture_Start(handle->apeHandle, &apeStartSettings);
    if ( errCode == BERR_NOT_SUPPORTED )
    {
        BDBG_MSG(("InputCapture start is delayed because current input format is unsupported"));
        BDBG_MSG(("InputCapture will restart at next format change"));
    }
    else if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        goto err_start;
    }
    handle->running = true;

    NEXUS_P_AudioInputCapture_ConnectHdmiInput(handle, true);

    return BERR_SUCCESS;

err_start:
    BAPE_InputCapture_GetInterruptHandlers(handle->apeHandle, &inputCaptureInterrupts);
    inputCaptureInterrupts.inputHalted.pCallback_isr = NULL;
    inputCaptureInterrupts.watermark.pCallback_isr = NULL;
    (void)BAPE_InputCapture_SetInterruptHandlers(handle->apeHandle, &inputCaptureInterrupts);
err_set_interrupts:
err_prepare_to_start:
err_status:
    if ( NEXUS_AudioInput_P_SupportsFormatChanges(pSettings->input) )
    {
        NEXUS_AudioInput_P_SetFormatChangeInterrupt(pSettings->input, NEXUS_AudioInputType_eInputCapture, NULL, handle, 0);
    }
err_format_interrupt:
    return BERR_TRACE(errCode);
}


/***************************************************************************
Summary:
Stop capturing input data
***************************************************************************/
void NEXUS_AudioInputCapture_Stop(
    NEXUS_AudioInputCaptureHandle handle
    )
{
    BAPE_InputCaptureInterruptHandlers inputCaptureInterrupts;

    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioInputCapture);
    if ( !handle->running )
    {
        return;
    }

    if ( NEXUS_AudioInput_P_SupportsFormatChanges(handle->startSettings.input) )
    {
        NEXUS_AudioInput_P_SetFormatChangeInterrupt(handle->startSettings.input, NEXUS_AudioInputType_eInputCapture, NULL, handle, 0);
    }
    BAPE_InputCapture_Stop(handle->apeHandle);
    BAPE_InputCapture_GetInterruptHandlers(handle->apeHandle, &inputCaptureInterrupts);
    inputCaptureInterrupts.inputHalted.pCallback_isr = NULL;
    inputCaptureInterrupts.watermark.pCallback_isr = NULL;
    (void)BAPE_InputCapture_SetInterruptHandlers(handle->apeHandle, &inputCaptureInterrupts);
    NEXUS_P_AudioInputCapture_ConnectHdmiInput(handle, false);

    handle->running = false;
}

/***************************************************************************
Summary:
Get current settings for AudioInputCapture
***************************************************************************/
void NEXUS_AudioInputCapture_GetSettings(
    NEXUS_AudioInputCaptureHandle handle,
    NEXUS_AudioInputCaptureSettings *pSettings  /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioInputCapture);
    *pSettings = handle->settings;
}

/***************************************************************************
Summary:
Set settings for AudioInputCapture
***************************************************************************/
NEXUS_Error NEXUS_AudioInputCapture_SetSettings(
    NEXUS_AudioInputCaptureHandle handle,
    const NEXUS_AudioInputCaptureSettings *pSettings
    )
{
    BERR_Code errCode;
    unsigned i, j;
    BAPE_MixerInputVolume volume;

    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioInputCapture);
    BDBG_ASSERT(NULL != pSettings);

    NEXUS_AudioInput_P_GetVolume(&handle->connector, &volume);
    for ( i = 0; i < NEXUS_AudioChannel_eMax; i++ )
    {
        for ( j = 0; j < NEXUS_AudioChannel_eMax; j++ )
        {
            volume.coefficients[i][j] = pSettings->volumeMatrix[j][i];
        }
    }
    volume.muted = pSettings->muted;
    BDBG_WRN(("Setting volume... first is %#x", volume.coefficients[0][0]));
    errCode = NEXUS_AudioInput_P_SetVolume(&handle->connector, &volume);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    NEXUS_TaskCallback_Set(handle->sourceChangedCallback, &pSettings->sourceChanged);

    handle->settings = *pSettings;
    return BERR_SUCCESS;
}

/***************************************************************************
Summary:
Get current status for AudioInputCapture
***************************************************************************/
NEXUS_Error NEXUS_AudioInputCapture_GetStatus(
    NEXUS_AudioInputCaptureHandle handle,
    NEXUS_AudioInputCaptureStatus *pStatus  /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioInputCapture);
    BDBG_ASSERT(NULL != pStatus);
    BKNI_Memset(pStatus, 0, sizeof(*pStatus));
    pStatus->started = handle->running;
    pStatus->codec = NEXUS_AudioCodec_eUnknown;
    pStatus->numPcmChannels = 2;
    if ( handle->running )
    {
        if ( NEXUS_AudioInput_P_SupportsFormatChanges(handle->startSettings.input) )
        {
            NEXUS_AudioInputPortStatus inputPortStatus;
            NEXUS_Error errCode;

            errCode = NEXUS_AudioInput_P_GetInputPortStatus(handle->startSettings.input, &inputPortStatus);
            if ( errCode )
            {
                return BERR_TRACE(errCode);
            }

            pStatus->sampleRate = inputPortStatus.sampleRate;
            if ( inputPortStatus.signalPresent )
            {
                pStatus->inputSignalPresent = true;
                pStatus->inputSignalValid = true;
                if ( inputPortStatus.compressed )
                {
                    pStatus->codec = inputPortStatus.codec;
                }
                else
                {
                    pStatus->codec = NEXUS_AudioCodec_ePcm;
                    pStatus->numPcmChannels = inputPortStatus.numPcmChannels;
                }
            }
        }
        else
        {
            pStatus->inputSignalPresent = true;
            pStatus->inputSignalValid = true;
        }
    }

    return BERR_SUCCESS;
}

NEXUS_AudioInputHandle NEXUS_AudioInputCapture_GetConnector(
    NEXUS_AudioInputCaptureHandle handle
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioInputCapture);
    return &handle->connector;
}

static void NEXUS_AudioInputCapture_P_InputFormatChange_isr(void *pParam1, int param2)
{
    NEXUS_AudioInputCaptureHandle handle = (NEXUS_AudioInputCaptureHandle)pParam1;
    BSTD_UNUSED(param2);
    /* convert to task time */
    BDBG_MSG(("Input Format change interrupt"));
    BKNI_SetEvent_isr(handle->inputFormatChangeEvent);
}

static void NEXUS_AudioInputCapture_P_InputCaptureHalted_isr(void *pParam1, int param2)
{
    NEXUS_AudioInputCaptureHandle handle = (NEXUS_AudioInputCaptureHandle)pParam1;
    BSTD_UNUSED(param2);
    BSTD_UNUSED(handle);
    BDBG_MSG(("Ignoring Input Capture Halt interrupt"));
}

static void NEXUS_AudioInputCapture_P_InputFormatChange(void *pParam)
{
    NEXUS_AudioInputCaptureHandle handle = (NEXUS_AudioInputCaptureHandle)pParam;
    NEXUS_AudioInputPortStatus inputPortStatus;
    BAPE_InputCaptureStatus apeInputCaptureStatus;
    NEXUS_Error errCode;
    bool restart = false;

    BDBG_MSG(("Processing Input Format change"));

    if ( !handle->running ){
        BDBG_MSG(("Not Started.  Aborting."));
        return;
    }

    NEXUS_TaskCallback_Fire(handle->sourceChangedCallback);

    errCode = NEXUS_AudioInput_P_GetInputPortStatus(handle->startSettings.input, &inputPortStatus);
    if ( errCode )
    {
        (void)BERR_TRACE(errCode);
        return;
    }

    BAPE_InputCapture_GetStatus(handle->apeHandle, &apeInputCaptureStatus );

    /* We only need to restart if APE's decoder is halted, or didn't start (not running). */
    if ( apeInputCaptureStatus.halted )
    {
        BDBG_MSG(("InputCapture has been halted, restart required"));
        restart = true;
    }
    else if ( ! apeInputCaptureStatus.running )
    {
        BDBG_MSG(("InputCapture is not running, restart required"));
        restart = true;
    }

    if ( restart == true )
    {
        BERR_Code errCode;
        BAPE_InputCaptureStartSettings apeStartSettings;

        BAPE_InputCapture_Stop(handle->apeHandle);

        BAPE_InputCapture_GetDefaultStartSettings(&apeStartSettings);
        apeStartSettings.input = handle->inputPort;
        BDBG_MSG(("Restarting InputCapture"));

        /* Set output data type correctly */
        NEXUS_AudioInputCapture_P_SetConnectorFormat(handle, inputPortStatus.compressed, inputPortStatus.hbr, inputPortStatus.numPcmChannels);

        /* Update downstream mixers */
        handle->running = false;
        (void)NEXUS_AudioInput_P_PrepareToStart(&handle->connector);
        handle->running = true;

        errCode = BAPE_InputCapture_Start(handle->apeHandle, &apeStartSettings);
        if ( errCode == BERR_NOT_SUPPORTED )
        {
            BDBG_MSG(("InputCapture start is delayed because current input format is unsupported"));
            BDBG_MSG(("InputCapture will restart at next format change"));
        }
        else if ( errCode )
        {
            errCode = BERR_TRACE(errCode);
        }

        handle->compressed = inputPortStatus.compressed;
        handle->numPcmChannels = inputPortStatus.numPcmChannels;
        handle->hbr = inputPortStatus.hbr;
    }
}

static void NEXUS_AudioInputCapture_P_SetConnectorFormat(NEXUS_AudioInputCaptureHandle handle, bool compressed, bool hbr, unsigned numPcmChannels)
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioInputCapture);
    if ( compressed )
    {
        BDBG_MSG(("Receiving Compressed.  Updating connector format."));
        BSTD_UNUSED(hbr);   /* TODO: Do we need to distinguish this for HDM setup? */
        handle->connector.format = NEXUS_AudioInputFormat_eCompressed;
    }
    else
    {
        BDBG_MSG(("Receiving PCM with %u channels.  Updating connector format.", numPcmChannels));
        switch ( numPcmChannels )
        {
        case 6:
            handle->connector.format = NEXUS_AudioInputFormat_ePcm5_1;
            break;
        case 8:
            handle->connector.format = NEXUS_AudioInputFormat_ePcm7_1;
            break;
        default:
            handle->connector.format = NEXUS_AudioInputFormat_ePcmStereo;
            break;
        }
    }

    #if 0
    if ( handle->procHandle )
    {
        handle->procHandle->format = handle->connector.format;
    }
    #endif
}

bool NEXUS_AudioInputCapture_P_IsRunning(NEXUS_AudioInputCaptureHandle handle)
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioInputCapture);
    return handle->running;
}

NEXUS_AudioInputCaptureHandle NEXUS_AudioInputCapture_P_GetInputCaptureByIndex(
    unsigned index
    )
{

    NEXUS_AudioCapabilities audioCapabilities;
    NEXUS_GetAudioCapabilities(&audioCapabilities);

    BDBG_ASSERT(index < audioCapabilities.numInputCaptures);
    return g_inputCaptures[index].opened?&g_inputCaptures[index]:NULL;
}

NEXUS_Error NEXUS_AudioInputCapture_GetBuffer(
    NEXUS_AudioInputCaptureHandle handle,
    void **ppBuffer,    /* [out] attr{memory=cached} pointer to memory mapped
                                 region that contains captured data. */
    size_t *pSize       /* [out] total number of readable, contiguous bytes which the buffers are pointing to */
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioInputCapture);
    BDBG_ASSERT(NULL != ppBuffer);
    BDBG_ASSERT(NULL != pSize);

    if ( !handle->running )
    {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    return NEXUS_AudioCapture_P_GetBuffer(handle->procHandle, ppBuffer, pSize);
}

NEXUS_Error NEXUS_AudioInputCapture_ReadComplete(
    NEXUS_AudioInputCaptureHandle handle,
    size_t amountWritten            /* The number of bytes read from the buffer */
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioInputCapture);

    if (!handle->running)
    {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    return NEXUS_AudioCapture_P_ReadComplete(handle->procHandle, amountWritten);
}

static void NEXUS_AudioInputCapture_P_FlushDeviceBuffer_isr(NEXUS_AudioInputCaptureHandle handle)
{
    BAPE_InputCapture_Flush_isr(handle->apeHandle);
}

static void NEXUS_AudioInputCapture_P_DataInterrupt_isr(void *pParam, int param)
{
    NEXUS_AudioInputCaptureHandle handle;

    handle = pParam;
    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioInputCapture);
    BSTD_UNUSED(param);

    BDBG_MSG(("Data Ready Interrupt, running %d, dataCallback %p", handle->running, (void *)handle->dataCallback));

    if ( handle->running )
    {
        if ( handle->dataCallback )
        {
            /* Propagate directly to app.  They will read the data out as part of GetBuffer */
            NEXUS_IsrCallback_Fire_isr(handle->dataCallback);
        }
    }
    else
    {
        NEXUS_AudioInputCapture_P_FlushDeviceBuffer_isr(handle);
    }
}

#else /* #if NEXUS_NUM_AUDIO_INPUT_CAPTURES */

typedef struct NEXUS_AudioInputCapture
{
    BDBG_OBJECT(NEXUS_AudioInputCapture)
} NEXUS_AudioInputCapture;

void NEXUS_AudioInputCapture_GetDefaultOpenSettings(
    NEXUS_AudioInputCaptureOpenSettings *pSettings  /* [out] */
    )
{
    BSTD_UNUSED(pSettings);
}

NEXUS_AudioInputCaptureHandle NEXUS_AudioInputCapture_Open(
    unsigned index,
    const NEXUS_AudioInputCaptureOpenSettings *pSettings
    )
{
    BSTD_UNUSED(index);
    BSTD_UNUSED(pSettings);
    (void)BERR_TRACE(BERR_NOT_SUPPORTED);
    return NULL;
}

void NEXUS_AudioInputCapture_Close(
    NEXUS_AudioInputCaptureHandle handle
    )
{
    BSTD_UNUSED(handle);
}

void NEXUS_AudioInputCapture_GetDefaultStartSettings(
    NEXUS_AudioInputCaptureStartSettings *pSettings /* [out] */
    )
{
    BSTD_UNUSED(pSettings);
}

NEXUS_Error NEXUS_AudioInputCapture_Start(
    NEXUS_AudioInputCaptureHandle handle,
    const NEXUS_AudioInputCaptureStartSettings *pSettings
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pSettings);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

void NEXUS_AudioInputCapture_Stop(
    NEXUS_AudioInputCaptureHandle handle
    )
{
    BSTD_UNUSED(handle);
}

void NEXUS_AudioInputCapture_GetSettings(
    NEXUS_AudioInputCaptureHandle handle,
    NEXUS_AudioInputCaptureSettings *pSettings  /* [out] */
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pSettings);
}

NEXUS_Error NEXUS_AudioInputCapture_SetSettings(
    NEXUS_AudioInputCaptureHandle handle,
    const NEXUS_AudioInputCaptureSettings *pSettings
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pSettings);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

NEXUS_Error NEXUS_AudioInputCapture_GetStatus(
    NEXUS_AudioInputCaptureHandle handle,
    NEXUS_AudioInputCaptureStatus *pStatus  /* [out] */
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pStatus);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

NEXUS_AudioInputHandle NEXUS_AudioInputCapture_GetConnector(
    NEXUS_AudioInputCaptureHandle handle
    )
{
    BSTD_UNUSED(handle);
    return NULL;
}

bool NEXUS_AudioInputCapture_P_IsRunning(NEXUS_AudioInputCaptureHandle handle)
{
    BSTD_UNUSED(handle);
    return false;
}

NEXUS_AudioInputCaptureHandle NEXUS_AudioInputCapture_P_GetInputCaptureByIndex(
    unsigned index
    )
{
    BSTD_UNUSED(index);
    return NULL;
}

NEXUS_Error NEXUS_AudioInputCapture_GetBuffer(
    NEXUS_AudioInputCaptureHandle handle,
    void **ppBuffer,    /* [out] attr{memory=cached} pointer to memory mapped
                                 region that contains captured data. */
    size_t *pSize       /* [out] total number of readable, contiguous bytes which the buffers are pointing to */
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(ppBuffer);
    BSTD_UNUSED(pSize);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

NEXUS_Error NEXUS_AudioInputCapture_ReadComplete(
    NEXUS_AudioInputCaptureHandle handle,
    size_t amountWritten            /* The number of bytes read from the buffer */
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(amountWritten);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

#endif /* #if NEXUS_NUM_AUDIO_INPUT_CAPTURES */