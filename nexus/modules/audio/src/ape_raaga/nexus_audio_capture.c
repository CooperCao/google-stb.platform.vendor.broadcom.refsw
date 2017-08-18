/***************************************************************************
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
*
* API Description:
*   API name: AudioCapture
*    Specific APIs related to PCM audio capture.  This supports capture
*    of data into memory from a decoder or other source.
*
***************************************************************************/

#include "nexus_audio_module.h"
#include "nexus_audio_capture.h"
#include "nexus_audio_capture_priv.h"
#include "nexus_client_resources.h"

BDBG_MODULE(nexus_audio_capture);

#if NEXUS_NUM_AUDIO_CAPTURES
typedef struct NEXUS_AudioCapture
{
    NEXUS_OBJECT(NEXUS_AudioCapture);
    bool opened;
    bool running;
    NEXUS_AudioMultichannelFormat multichannelFormat;
    BAPE_OutputCaptureHandle apeHandle;
    NEXUS_AudioOutputObject connector;
    NEXUS_AudioCaptureSettings settings;
    BMMA_Block_Handle fifoBlock;
    void *pMemory;
    NEXUS_AudioCaptureProcessorHandle procHandle;
    /* uint32_t *pBuffer;
    int bufferSize, rptr, wptr, bufferDepth; */
    NEXUS_IsrCallbackHandle dataCallback, sampleRateCallback, overflowCallback;
    unsigned sampleRate;
    size_t fifoSize;  /* PI capture fifo size (per channel pair) */
    char name[16];   /* AUDIO CAPTURE %d */
    BMMA_Heap_Handle fifoMem; /* heap used for fifo */

#define RESOLVE_ALIAS(handle) do {(handle) = ((handle)->alias.master?(handle)->alias.master:(handle));}while(0)
#define IS_ALIAS(handle) ((handle)->alias.master != NULL)
    struct {
        NEXUS_AudioCaptureHandle master, slave;
    } alias;
} NEXUS_AudioCapture;

static NEXUS_AudioCapture g_captures[NEXUS_NUM_AUDIO_CAPTURES];

static void NEXUS_AudioCapture_P_ConvertStereo24(NEXUS_AudioCaptureProcessorHandle handle);
static void NEXUS_AudioCapture_P_ConvertStereo16(NEXUS_AudioCaptureProcessorHandle handle);
static void NEXUS_AudioCapture_P_ConvertMultichannel(NEXUS_AudioCaptureProcessorHandle handle);
static void NEXUS_AudioCapture_P_ConvertMono(NEXUS_AudioCaptureProcessorHandle handle, bool rightChannel);
static void NEXUS_AudioCapture_P_ConvertMonoMix(NEXUS_AudioCaptureProcessorHandle handle);
static size_t NEXUS_AudioCapture_P_GetContiguousSpace(NEXUS_AudioCaptureProcessorHandle handle);
static void NEXUS_AudioCapture_P_AdvanceBuffer(NEXUS_AudioCaptureProcessorHandle handle, size_t bytes);
static void NEXUS_AudioCapture_P_DataInterrupt_isr(void *pParam, int param);
static void NEXUS_AudioCapture_P_SampleRateChange_isr(void *pParam, int param, unsigned sampleRate);
static void NEXUS_AudioCapture_P_Overflow_isr(void *pParam, int param);
static void NEXUS_AudioCapture_P_FlushDeviceBuffer_isr(NEXUS_AudioCaptureHandle handle);

#define min(A,B) ((A)<(B)?(A):(B))

static BERR_Code NEXUS_AudioCapture_P_Output_GetBuffer(
    void * hCapture,
    void * pBuffers      /* [out] */
    )
{
    return BAPE_OutputCapture_GetBuffer((BAPE_OutputCaptureHandle)hCapture, (BAPE_BufferDescriptor*)pBuffers);
}

static BERR_Code NEXUS_AudioCapture_P_Output_ConsumeData(
    void * hCapture,
    unsigned numBytes                   /* Number of bytes read from the buffer */
    )
{
    return BAPE_OutputCapture_ConsumeData((BAPE_OutputCaptureHandle)hCapture, numBytes);
}

static NEXUS_Error NEXUS_AudioCapture_P_CalculateThreshold(
    NEXUS_AudioCaptureFormat format,
    unsigned bufferSize,
    unsigned inThreshold,
    unsigned * outThreshold    /* [out] */
    )
{
    BDBG_ASSERT(outThreshold != NULL);
    switch ( format )
    {
    case NEXUS_AudioCaptureFormat_e16BitStereo:
    case NEXUS_AudioCaptureFormat_eCompressed:
        *outThreshold = (inThreshold/2) & (~255);
        break;
    case NEXUS_AudioCaptureFormat_e24BitStereo:
    case NEXUS_AudioCaptureFormat_e24Bit7_1:
    case NEXUS_AudioCaptureFormat_e24Bit5_1:
        *outThreshold = inThreshold & (~255);
        break;
    case NEXUS_AudioCaptureFormat_e16BitMonoLeft:
    case NEXUS_AudioCaptureFormat_e16BitMonoRight:
    case NEXUS_AudioCaptureFormat_e16BitMono:
        *outThreshold = (inThreshold/4) & (~255);
        break;
    default:
        BDBG_ERR(("Unsupported capture format %u", format));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    if ( *outThreshold > bufferSize/2 )
    {
        BDBG_WRN(("Capture watermark threshold is greater than half the fifo size"));
        *outThreshold = bufferSize/2 & (~255);
    }

    return NEXUS_SUCCESS;
}

void NEXUS_AudioCapture_GetDefaultOpenSettings(
    NEXUS_AudioCaptureOpenSettings *pSettings      /* [out] default settings */
    )
{
    BAPE_OutputCaptureOpenSettings piSettings;
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    BAPE_OutputCapture_GetDefaultOpenSettings(&piSettings);
    pSettings->fifoSize = piSettings.bufferSize;
    pSettings->threshold = piSettings.watermarkThreshold;
    pSettings->multichannelFormat = NEXUS_AudioMultichannelFormat_eStereo;
}

NEXUS_AudioCaptureHandle NEXUS_AudioCapture_Open(     /* attr{destructor=NEXUS_AudioCapture_Close}  */
    unsigned index,
    const NEXUS_AudioCaptureOpenSettings *pSettings    /* Pass NULL for default settings */
    )
{
    NEXUS_AudioCaptureOpenSettings defaults;
    BAPE_OutputCaptureOpenSettings openSettings;
    NEXUS_AudioCaptureHandle handle;
    NEXUS_AudioCaptureHandle master = NULL;
    BAPE_OutputCaptureInterruptHandlers interrupts;
    BAPE_OutputPort connector;
    NEXUS_HeapHandle heap;
    NEXUS_AudioCaptureProcessorHandle procHandle = NULL;
    BERR_Code errCode;
    unsigned org_index = index;
    NEXUS_AudioCapabilities audioCapabilities;
    unsigned outputCount;

    BDBG_CASSERT((int)NEXUS_MAX_AUDIO_CAPTURE_OUTPUTS>=(int)NEXUS_NUM_AUDIO_CAPTURES);

    NEXUS_GetAudioCapabilities(&audioCapabilities);
    outputCount = min(audioCapabilities.numOutputs.capture, NEXUS_NUM_AUDIO_CAPTURES);

    errCode = NEXUS_CLIENT_RESOURCES_ACQUIRE(audioCapture,IdList,org_index);
    if (errCode) { errCode = BERR_TRACE(errCode); goto err_acquire; }

    if (index >= NEXUS_ALIAS_ID && index-NEXUS_ALIAS_ID < outputCount) {
        BDBG_MSG(("%d aliasing %d(%p)", index, index-NEXUS_ALIAS_ID, (void *)&g_captures[index-NEXUS_ALIAS_ID]));
        index -= NEXUS_ALIAS_ID;
        master = &g_captures[index];
        if (!master->opened) {
            BDBG_ERR(("cannot alias %d because it is not opened", index));
            goto err_index;
        }
        if (master->alias.slave) {
            BDBG_ERR(("cannot alias %d a second time", index));
            goto err_index;
        }
    }
    if ( index >= outputCount )
    {
        BDBG_ERR(("index out of range."));
        (void)BERR_TRACE(BERR_INVALID_PARAMETER);
        goto err_index;
    }

    if (!master) {
        handle = &g_captures[index];
        if ( handle->opened )
        {
            BDBG_ERR(("AudioCapture %u already open", index));
            (void)BERR_TRACE(BERR_NOT_SUPPORTED);
            goto err_index;
        }
    }
    else {
        handle = BKNI_Malloc(sizeof(*handle));
        if ( !handle )
        {
            (void)BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
            goto err_index;
        }
        BKNI_Memset(handle, 0, sizeof(*handle));
    }

    NEXUS_OBJECT_SET(NEXUS_AudioCapture, handle);

    if (master) {
        handle->alias.master = master;
        master->alias.slave = handle;
        return handle;
    }

    procHandle = BKNI_Malloc(sizeof(NEXUS_AudioCaptureProcessor));
    if ( !procHandle )
    {
        (void)BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto err_handle;
    }
    BKNI_Memset(procHandle, 0, sizeof(*procHandle));

    if ( NULL == pSettings )
    {
        NEXUS_AudioCapture_GetDefaultOpenSettings(&defaults);
        pSettings = &defaults;
    }

    BAPE_OutputCapture_GetDefaultOpenSettings(&openSettings);
    switch ( pSettings->multichannelFormat )
    {
    case NEXUS_AudioMultichannelFormat_eStereo:
        openSettings.numBuffers = 1;
        break;
    case NEXUS_AudioMultichannelFormat_e5_1:
        openSettings.numBuffers = 3;
        break;
    case NEXUS_AudioMultichannelFormat_e7_1:
        openSettings.numBuffers = 4;
        break;
    default:
        BDBG_ERR(("Unsupported multichannel format %u", pSettings->multichannelFormat));
        (void)BERR_TRACE(BERR_INVALID_PARAMETER);
        goto err_param;
    }

    openSettings.bufferSize = pSettings->fifoSize/openSettings.numBuffers;

    errCode = NEXUS_AudioCapture_P_CalculateThreshold(pSettings->format, openSettings.bufferSize, pSettings->threshold, &openSettings.watermarkThreshold);
    if ( errCode != NEXUS_SUCCESS ) { BERR_TRACE(errCode); goto err_param; }

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

    handle->sampleRateCallback = NEXUS_IsrCallback_Create(handle, NULL);
    if ( NULL == handle->sampleRateCallback )
    {
        (void)BERR_TRACE(BERR_OS_ERROR);
        goto err_sample_rate_callback;
    }

    handle->overflowCallback = NEXUS_IsrCallback_Create(handle, NULL);
    if ( NULL == handle->overflowCallback )
    {
        (void)BERR_TRACE(BERR_OS_ERROR);
        goto err_overflow_callback;
    }

    errCode = BAPE_OutputCapture_Open(NEXUS_AUDIO_DEVICE_HANDLE, index, &openSettings, &handle->apeHandle);
    if ( errCode )
    {
        (void)BERR_TRACE(errCode);
        goto err_ape_handle;
    }

    handle->fifoBlock = BMMA_Alloc(handle->fifoMem, pSettings->fifoSize * openSettings.numBuffers, 0, NULL);
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

    /*handle->pBuffer = handle->pCachedMemory;
    handle->bufferSize = pSettings->fifoSize;*/
    handle->sampleRate = 48000;
    handle->fifoSize = pSettings->fifoSize;

    /* Set up the Capture Processor */
    procHandle->piHandle = handle->apeHandle;
    procHandle->pBuffer = handle->pMemory;
    procHandle->bufferSize = pSettings->fifoSize;
    procHandle->getBuffer = NEXUS_AudioCapture_P_Output_GetBuffer;
    procHandle->consumeData = NEXUS_AudioCapture_P_Output_ConsumeData;
    procHandle->format = handle->settings.format = pSettings->format;
    handle->procHandle = procHandle;

    handle->settings.threshold = openSettings.watermarkThreshold;

    BDBG_OBJECT_SET(handle, NEXUS_AudioCapture);
    handle->opened = true;
    BKNI_Snprintf(handle->name, sizeof(handle->name), "AUDIO CAPTURE %u", index);
    NEXUS_AUDIO_OUTPUT_INIT(&handle->connector, NEXUS_AudioOutputType_eCapture, handle);
    NEXUS_OBJECT_REGISTER(NEXUS_AudioOutput, &handle->connector, Open);

    BAPE_OutputCapture_GetOutputPort(handle->apeHandle, &connector);
    handle->connector.port = (size_t)connector;
    handle->connector.pName = handle->name;

    BAPE_OutputCapture_GetInterruptHandlers(handle->apeHandle, &interrupts);
    interrupts.watermark.pCallback_isr = NEXUS_AudioCapture_P_DataInterrupt_isr;
    interrupts.watermark.pParam1 = handle;
    interrupts.sampleRate.pCallback_isr = NEXUS_AudioCapture_P_SampleRateChange_isr;
    interrupts.sampleRate.pParam1 = handle;
    interrupts.overflow.pCallback_isr = NEXUS_AudioCapture_P_Overflow_isr;
    interrupts.overflow.pParam1 = handle;
    errCode = BAPE_OutputCapture_SetInterruptHandlers(handle->apeHandle, &interrupts);
    BDBG_ASSERT(errCode == BERR_SUCCESS);

    return handle;

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
    BAPE_OutputCapture_Close(handle->apeHandle);
err_ape_handle:
    NEXUS_IsrCallback_Destroy(handle->overflowCallback);
err_overflow_callback:
    NEXUS_IsrCallback_Destroy(handle->sampleRateCallback);
err_sample_rate_callback:
    NEXUS_IsrCallback_Destroy(handle->dataCallback);
err_data_callback:
err_heap:
err_param:
    BKNI_Free(procHandle);
    NEXUS_OBJECT_UNSET(NEXUS_AudioCapture, handle);
err_handle:
err_index:
    NEXUS_CLIENT_RESOURCES_RELEASE(audioCapture,IdList,org_index);
err_acquire:
    return NULL;
}

static void NEXUS_AudioCapture_P_Finalizer(
    NEXUS_AudioCaptureHandle handle
    )
{
    unsigned index;
    NEXUS_OBJECT_ASSERT(NEXUS_AudioCapture, handle);

    if (handle->alias.master) {
        index = NEXUS_ALIAS_ID + (handle->alias.master - g_captures);
        handle->alias.master->alias.slave = NULL;
        NEXUS_CLIENT_RESOURCES_RELEASE(audioCapture,IdList, index);
        NEXUS_OBJECT_DESTROY(NEXUS_AudioCapture, handle);
        BKNI_Free(handle);
        return;
    }
    else {
        /* close slave */
        if (handle->alias.slave) {
            NEXUS_OBJECT_UNREGISTER(NEXUS_AudioCapture, handle->alias.slave, Close);
            NEXUS_AudioCapture_Close(handle->alias.slave);
        }
    }

    BDBG_ASSERT(handle->opened);
    NEXUS_AudioOutput_RemoveAllInputs(&handle->connector);
    NEXUS_AudioOutput_Shutdown(&handle->connector);
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
    BAPE_OutputCapture_Close(handle->apeHandle);
    NEXUS_IsrCallback_Destroy(handle->sampleRateCallback);
    NEXUS_IsrCallback_Destroy(handle->dataCallback);
    NEXUS_IsrCallback_Destroy(handle->overflowCallback);
    index = handle - g_captures;
    NEXUS_CLIENT_RESOURCES_RELEASE(audioCapture,IdList,index);
    BKNI_Free(handle->procHandle);
    BKNI_Memset(handle, 0, sizeof(NEXUS_AudioCapture));
}

static void NEXUS_AudioCapture_P_Release(NEXUS_AudioCaptureHandle handle)
{
    if (!IS_ALIAS(handle)) {
        NEXUS_OBJECT_UNREGISTER(NEXUS_AudioOutput, &handle->connector, Close);
    }
    return;
}

NEXUS_OBJECT_CLASS_MAKE_WITH_RELEASE(NEXUS_AudioCapture, NEXUS_AudioCapture_Close);

void NEXUS_AudioCapture_GetSettings(
    NEXUS_AudioCaptureHandle handle,
    NEXUS_AudioCaptureSettings *pSettings /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioCapture);
    RESOLVE_ALIAS(handle);
    BDBG_ASSERT(handle->opened);
    BDBG_ASSERT(NULL != pSettings);
    *pSettings = handle->settings;
}

NEXUS_Error NEXUS_AudioCapture_SetSettings(
    NEXUS_AudioCaptureHandle handle,
    const NEXUS_AudioCaptureSettings *pSettings
    )
{   
    NEXUS_Error rc;
    BAPE_OutputCaptureSettings captureSettings;

    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioCapture);
    RESOLVE_ALIAS(handle);
    BDBG_ASSERT(handle->opened);
    BDBG_ASSERT(NULL != pSettings);
    handle->settings = *pSettings;

    BAPE_OutputCapture_GetSettings(handle->apeHandle, &captureSettings);

    rc = NEXUS_AudioCapture_P_CalculateThreshold(handle->settings.format, handle->procHandle->bufferSize, pSettings->threshold, &captureSettings.watermark);
    if ( rc != NEXUS_SUCCESS ) { return BERR_TRACE(rc); }

    rc = BAPE_OutputCapture_SetSettings(handle->apeHandle, &captureSettings);
    if ( rc != NEXUS_SUCCESS ) { return BERR_TRACE(rc); }

    return NEXUS_SUCCESS;
}

void NEXUS_AudioCapture_GetDefaultStartSettings(
    NEXUS_AudioCaptureStartSettings *pSettings  /* [out] default settings */
    )
{
    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    NEXUS_CallbackDesc_Init(&pSettings->dataCallback);
    NEXUS_CallbackDesc_Init(&pSettings->sampleRateCallback);
    NEXUS_CallbackDesc_Init(&pSettings->overflowCallback);
}

NEXUS_Error NEXUS_AudioCapture_Start(
    NEXUS_AudioCaptureHandle handle,
    const NEXUS_AudioCaptureStartSettings *pSettings
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioCapture);
    RESOLVE_ALIAS(handle);

    if ( handle->running )
    {
        BDBG_ERR(("Already running"));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* Setup callback */
    NEXUS_IsrCallback_Set(handle->dataCallback, pSettings?&pSettings->dataCallback:NULL);
    NEXUS_IsrCallback_Set(handle->sampleRateCallback, pSettings?&pSettings->sampleRateCallback:NULL);
    NEXUS_IsrCallback_Set(handle->overflowCallback, pSettings?&pSettings->overflowCallback:NULL);

    /* Setup internal buffer and start */
    BKNI_EnterCriticalSection();
    NEXUS_AudioCapture_P_FlushDeviceBuffer_isr(handle);
    handle->running = true;
    handle->procHandle->wptr = handle->procHandle->rptr = handle->procHandle->bufferDepth = 0;
    BKNI_LeaveCriticalSection();

    return BERR_SUCCESS;
}

void NEXUS_AudioCapture_Stop(
    NEXUS_AudioCaptureHandle handle
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioCapture);
    RESOLVE_ALIAS(handle);

    BDBG_MSG(("Stopping"));

    BKNI_EnterCriticalSection();
    handle->running = false;
    BKNI_LeaveCriticalSection();

    NEXUS_IsrCallback_Set(handle->dataCallback, NULL);
    NEXUS_IsrCallback_Set(handle->sampleRateCallback, NULL);
    NEXUS_IsrCallback_Set(handle->overflowCallback, NULL);
}

NEXUS_Error NEXUS_AudioCapture_GetBuffer(
    NEXUS_AudioCaptureHandle handle,
    void **ppBuffer,    /* [out] attr{memory=cached} pointer to memory mapped
                                 region that contains captured data. */
    size_t *pSize       /* [out] total number of readable, contiguous bytes which the buffers are pointing to */
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioCapture);
    RESOLVE_ALIAS(handle);
    BDBG_ASSERT(NULL != ppBuffer);
    BDBG_ASSERT(NULL != pSize);

    if ( !handle->running )
    {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    return NEXUS_AudioCapture_P_GetBuffer(handle->procHandle, ppBuffer, pSize);
}

NEXUS_Error NEXUS_AudioCapture_ReadComplete(
    NEXUS_AudioCaptureHandle handle,
    size_t amountWritten            /* The number of bytes read from the buffer */
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioCapture);
    RESOLVE_ALIAS(handle);
    if (!handle->running)
    {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    return NEXUS_AudioCapture_P_ReadComplete(handle->procHandle, amountWritten);
}

NEXUS_AudioOutputHandle NEXUS_AudioCapture_GetConnector( /* attr{shutdown=NEXUS_AudioOutput_Shutdown} */
    NEXUS_AudioCaptureHandle handle
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioCapture);
    RESOLVE_ALIAS(handle);
    return &handle->connector;
}

static void NEXUS_AudioCapture_P_DataInterrupt_isr(void *pParam, int param)
{
    NEXUS_AudioCaptureHandle handle;

    handle = pParam;
    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioCapture);
    BSTD_UNUSED(param);

    BDBG_MSG(("Capture Interrupt"));

    if ( handle->running )
    {
        /* Propagate directly to app.  They will read the data out as part of GetBuffer */
        NEXUS_IsrCallback_Fire_isr(handle->dataCallback);
    }
    else
    {
        NEXUS_AudioCapture_P_FlushDeviceBuffer_isr(handle);
    }
}

static void NEXUS_AudioCapture_P_SampleRateChange_isr(void *pParam, int param, unsigned sampleRate)
{
    NEXUS_AudioCaptureHandle handle;

    handle = pParam;
    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioCapture);
    BSTD_UNUSED(param);

    BDBG_MSG(("Sample Rate Interrupt (%u)", sampleRate));

    handle->sampleRate = sampleRate;
    NEXUS_IsrCallback_Fire_isr(handle->sampleRateCallback);
}


static void NEXUS_AudioCapture_P_Overflow_isr(void *pParam, int param)
{
    NEXUS_AudioCaptureHandle handle;

    handle = pParam;
    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioCapture);
    BSTD_UNUSED(param);

    BDBG_MSG(("Capture Overflow Interrupt"));
    /* Propagate directly to app. */
    NEXUS_IsrCallback_Fire_isr(handle->overflowCallback);
}

static void NEXUS_AudioCapture_P_FlushDeviceBuffer_isr(NEXUS_AudioCaptureHandle handle)
{
    BAPE_OutputCapture_Flush_isr(handle->apeHandle);
}

NEXUS_Error NEXUS_AudioCapture_GetStatus(
    NEXUS_AudioCaptureHandle handle,
    NEXUS_AudioCaptureStatus *pStatus   /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioCapture);
    RESOLVE_ALIAS(handle);
    BDBG_ASSERT(NULL != pStatus);
    BKNI_Memset(pStatus, 0, sizeof(NEXUS_AudioCaptureStatus));
    pStatus->started = handle->running;
    pStatus->sampleRate = handle->sampleRate;
#if BSTD_CPU_ENDIAN == BSTD_ENDIAN_BIG
    pStatus->endian = NEXUS_EndianMode_eBig;
#else
    pStatus->endian = NEXUS_EndianMode_eLittle;
#endif
    return BERR_SUCCESS;
}

NEXUS_Error NEXUS_AudioCapture_P_GetBuffer(
    NEXUS_AudioCaptureProcessorHandle handle,
    void **ppBuffer,    /* [out] attr{memory=cached} pointer to memory mapped
                                 region that contains captured data. */
    size_t *pSize       /* [out] total number of readable, contiguous bytes which the buffers are pointing to */
    )
{
    BDBG_ASSERT(NULL != handle);
    BDBG_ASSERT(NULL != ppBuffer);
    BDBG_ASSERT(NULL != pSize);

    /* Defaults for error returns */
    *ppBuffer = NULL;
    *pSize = 0;

    BDBG_MSG(("GetBuffer:format %u - Before Conversion rptr %d wptr %d depth %d/%d", handle->format, handle->rptr, handle->wptr, handle->bufferDepth, handle->bufferSize));

    switch ( handle->format )
    {
    case NEXUS_AudioCaptureFormat_e24BitStereo:
        NEXUS_AudioCapture_P_ConvertStereo24(handle);
        break;
    case NEXUS_AudioCaptureFormat_e16BitStereo:
    case NEXUS_AudioCaptureFormat_eCompressed:
        NEXUS_AudioCapture_P_ConvertStereo16(handle);
        break;
    case NEXUS_AudioCaptureFormat_e16BitMonoLeft:
        NEXUS_AudioCapture_P_ConvertMono(handle, false);
        break;
    case NEXUS_AudioCaptureFormat_e16BitMonoRight:
        NEXUS_AudioCapture_P_ConvertMono(handle, true);
        break;
    case NEXUS_AudioCaptureFormat_e16BitMono:
        NEXUS_AudioCapture_P_ConvertMonoMix(handle);
        break;
    case NEXUS_AudioCaptureFormat_e24Bit5_1:
    case NEXUS_AudioCaptureFormat_e24Bit7_1:
        NEXUS_AudioCapture_P_ConvertMultichannel(handle);
        break;
    default:
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    *ppBuffer = &handle->pBuffer[handle->rptr];
    if ( handle->bufferDepth == 0 )
    {
        /* No data */
        *pSize = 0;
    }
    else if ( handle->rptr < handle->wptr )
    {
        /* no wraparound */
        *pSize = (handle->wptr - handle->rptr)*4;
    }
    else
    {
        /* Buffer has wrapped, give contiguous size to end of buffer */
        *pSize = handle->bufferSize - (4*handle->rptr);
    }

    BDBG_MSG(("After conversion depth %d rptr %d wptr %d size %lu words (%lu bytes)", handle->bufferDepth, handle->rptr, handle->wptr, (unsigned long)(*pSize)/4, (unsigned long)*pSize));

    return BERR_SUCCESS;
}

NEXUS_Error NEXUS_AudioCapture_P_ReadComplete(
    NEXUS_AudioCaptureProcessorHandle handle,
    size_t amountWritten            /* The number of bytes read from the buffer */
    )
{
    int newRptr;

    BDBG_ASSERT(NULL != handle);

    if ( amountWritten % 4 )
    {
        BDBG_ERR(("The buffer must be updated in increments of four bytes"));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    newRptr = handle->rptr + (amountWritten/4);

    if ( newRptr == (handle->bufferSize/4) )
    {
        newRptr = 0;
    }
    else if ( newRptr > (handle->bufferSize/4) )
    {
        BDBG_ERR(("Overflow - invalid number of bytes passed"));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* Sanity check error cases where the read pointer passes the write pointer */
    if ( newRptr != handle->wptr )
    {
        if ( (handle->rptr < handle->wptr) &&
             (newRptr > handle->wptr) )
        {
            BDBG_ERR(("Overflow - invalid number of bytes passed"));
            return BERR_TRACE(BERR_INVALID_PARAMETER);
        }
        else if ( (handle->rptr > handle->wptr) &&
                  (newRptr < handle->wptr) &&
                  newRptr != 0 )
        {
            BDBG_ERR(("Overflow - invalid number of bytes passed"));
            return BERR_TRACE(BERR_INVALID_PARAMETER);
        }
    }

    BDBG_MSG(("Read Complete - %lu bytes written, moving rptr from %d to %d (size %d)", (unsigned long)amountWritten, handle->rptr, newRptr, handle->bufferSize/4));

    /* Update read pointer of our buffer */
    handle->rptr = newRptr;
    handle->bufferDepth -= amountWritten;

    return BERR_SUCCESS;
}

static size_t NEXUS_AudioCapture_P_GetContiguousSpace(NEXUS_AudioCaptureProcessorHandle handle)
{
    if ( handle->bufferDepth == handle->bufferSize )
    {
        return 0;
    }
    else if ( handle->wptr < handle->rptr )
    {
        /* No Wrap */
        return handle->bufferSize - handle->bufferDepth;
    }
    else
    {
        /* Wrap - return contiguous amount */
        return handle->bufferSize - (handle->wptr*4);
    }
}

static void NEXUS_AudioCapture_P_AdvanceBuffer(NEXUS_AudioCaptureProcessorHandle handle, size_t bytes)
{
    handle->bufferDepth += bytes;
    handle->wptr += (bytes)/4;
    if ( handle->wptr >= (handle->bufferSize/4) )
    {
        handle->wptr = 0;
    }
}

static void NEXUS_AudioCapture_P_ConvertStereo24(NEXUS_AudioCaptureProcessorHandle handle)
{
    BERR_Code errCode;
    uint32_t *pSource;
    size_t bufferSize, copied;
    BAPE_BufferDescriptor bufferDescriptor;

    for ( ;; )
    {
        errCode = handle->getBuffer(handle->piHandle, &bufferDescriptor);
        if ( errCode )
        {
            (void)BERR_TRACE(errCode);
            return;
        }
        bufferSize = bufferDescriptor.bufferSize;
        if ( bufferSize == 0 )
        {
            /* Done */
            return;
        }
        BMMA_FlushCache(bufferDescriptor.buffers[BAPE_Channel_eLeft].block, bufferDescriptor.buffers[BAPE_Channel_eLeft].pBuffer, bufferSize);
        pSource = bufferDescriptor.buffers[BAPE_Channel_eLeft].pBuffer;
        copied = 0;
        while ( bufferSize >= 4 )
        {
            size_t available, bytesToCopy;
            available = NEXUS_AudioCapture_P_GetContiguousSpace(handle);
            if ( available < 4 )
            {
                /* Our buffer is full. */
                break;
            }
            if ( available >= bufferSize )
            {
                bytesToCopy = bufferSize;
            }
            else
            {
                bytesToCopy = available;
            }
            while ( bytesToCopy >= 4 )
            {
                handle->pBuffer[handle->wptr] = (*pSource++) & 0xffffff00;
                NEXUS_AudioCapture_P_AdvanceBuffer(handle, 4);
                copied += 4;
                bufferSize -= 4;
                bytesToCopy -= 4;
            }
        }
        if ( copied == 0 )
        {
            return;
        }
        errCode = handle->consumeData(handle->piHandle, copied);
        if ( errCode )
        {
            (void)BERR_TRACE(errCode);
            return;
        }
    }
}

#define SWAP16( a )  do{a=((a&0xFF)<<8|(a&0xFF00)>>8);}while(0)

static void NEXUS_AudioCapture_P_ConvertStereo16(NEXUS_AudioCaptureProcessorHandle handle)
{
    BERR_Code errCode;
    uint32_t *pSource;
    size_t bufferSize, copied;
    BAPE_BufferDescriptor bufferDescriptor;

    for ( ;; )
    {
        errCode = handle->getBuffer(handle->piHandle, &bufferDescriptor);
        if ( errCode )
        {
            (void)BERR_TRACE(errCode);
            return;
        }
        bufferSize = bufferDescriptor.bufferSize;
        if ( bufferSize == 0 )
        {
            /* Done */
            return;
        }
        BMMA_FlushCache(bufferDescriptor.buffers[BAPE_Channel_eLeft].block, bufferDescriptor.buffers[BAPE_Channel_eLeft].pBuffer, bufferSize);
        pSource = bufferDescriptor.buffers[BAPE_Channel_eLeft].pBuffer;
        copied = 0;
        while ( bufferSize >= 8 )
        {
            size_t available, bytesToCopy;
            available = NEXUS_AudioCapture_P_GetContiguousSpace(handle);
            if ( available < 4 )
            {
                /* Our buffer is full. */
                break;
            }
            if ( available >= bufferSize/2 )
            {
                bytesToCopy = bufferSize/2;
            }
            else
            {
                bytesToCopy = available;
            }
            while ( bytesToCopy >= 4 )
            {
                uint32_t sample;
#if BSTD_CPU_ENDIAN == BSTD_ENDIAN_BIG
                if ( handle->format == NEXUS_AudioCaptureFormat_eCompressed) {
                    uint16_t temp;
                    temp = (*pSource++)>>16;
                    SWAP16 (temp);
                    sample = temp << 16; /* Right */
                    temp = (*pSource++)>>16;
                    SWAP16(temp);
                    sample |= temp;      /* Left */
                }
                else {
                    sample = (*pSource++)&0xffff0000;  /* Right */
                    sample |= (*pSource++)>>16;          /* Left */
                }
#else
                sample = (*pSource++)>>16;          /* Left */
                sample |= (*pSource++)&0xffff0000;  /* Right */
#endif
                handle->pBuffer[handle->wptr] = sample;
                NEXUS_AudioCapture_P_AdvanceBuffer(handle, 4);
                copied += 8;
                bufferSize -= 8;
                bytesToCopy -= 4;
            }
        }
        if ( copied == 0 )
        {
            return;
        }
        errCode = handle->consumeData(handle->piHandle, copied);
        if ( errCode )
        {
            (void)BERR_TRACE(errCode);
            return;
        }
    }
}

static void NEXUS_AudioCapture_P_ConvertMultichannel(NEXUS_AudioCaptureProcessorHandle handle)
{
    BERR_Code errCode;
    uint32_t * pSource[4];
    size_t bufferSize, copied;
    BAPE_BufferDescriptor bufferDescriptor;
    unsigned numChPairs;
    unsigned i;

    numChPairs = (handle->format == NEXUS_AudioCaptureFormat_e24Bit7_1) ? 4 : 3;

    for ( ;; )
    {
        errCode = handle->getBuffer(handle->piHandle, &bufferDescriptor);
        if ( errCode )
        {
            (void)BERR_TRACE(errCode);
            return;
        }
        bufferSize = bufferDescriptor.bufferSize;
        if ( bufferSize == 0 )
        {
            /* Done */
            return;
        }

        for ( i = 0; i < numChPairs; i++ )
        {
            BMMA_FlushCache(bufferDescriptor.buffers[i*2].block, bufferDescriptor.buffers[i*2].pBuffer, bufferSize);
            pSource[i] = bufferDescriptor.buffers[i*2].pBuffer;
        }
        copied = 0;
        while ( bufferSize >= 8 )
        {
            size_t available, bytesToCopy;
            available = NEXUS_AudioCapture_P_GetContiguousSpace(handle);
            if ( available < 4*numChPairs*2 )
            {
                /* Our buffer is full. */
                break;
            }
            if ( available >= bufferSize*numChPairs )
            {
                bytesToCopy = bufferSize*numChPairs;
            }
            else
            {
                bytesToCopy = available;
            }
            while ( bytesToCopy >= 4*numChPairs*2 ) /* 4 bytes per sample * numchs */
            {
                for ( i = 0; i < numChPairs; i++ )
                {
                    if ( pSource[i] )
                    {
                        handle->pBuffer[handle->wptr] = (*pSource[i]++) & 0xffffff00;
                        NEXUS_AudioCapture_P_AdvanceBuffer(handle, 4);
                        handle->pBuffer[handle->wptr] = (*pSource[i]++) & 0xffffff00;
                        NEXUS_AudioCapture_P_AdvanceBuffer(handle, 4);
                    }
                    else
                    {
                        handle->pBuffer[handle->wptr] = 0;
                        NEXUS_AudioCapture_P_AdvanceBuffer(handle, 4);
                        handle->pBuffer[handle->wptr] = 0;
                        NEXUS_AudioCapture_P_AdvanceBuffer(handle, 4);
                    }
                }

                bytesToCopy -= 4*numChPairs*2;
                copied += 8;
                bufferSize -= 8;
            }
        }
        if ( copied == 0 )
        {
            return;
        }
        errCode = handle->consumeData(handle->piHandle, copied);
        if ( errCode )
        {
            (void)BERR_TRACE(errCode);
            return;
        }
    }    
}

static void NEXUS_AudioCapture_P_ConvertMono(NEXUS_AudioCaptureProcessorHandle handle, bool rightChannel)
{
    BERR_Code errCode;
    uint32_t *pSource;
    size_t bufferSize, copied;
    BAPE_BufferDescriptor bufferDescriptor;

    for ( ;; )
    {
        errCode = handle->getBuffer(handle->piHandle, &bufferDescriptor);
        if ( errCode )
        {
            (void)BERR_TRACE(errCode);
            return;
        }
        bufferSize = bufferDescriptor.bufferSize;
        if ( bufferSize == 0 )
        {
            /* Done */
            return;
        }
        BMMA_FlushCache(bufferDescriptor.buffers[BAPE_Channel_eLeft].block, bufferDescriptor.buffers[BAPE_Channel_eLeft].pBuffer, bufferSize);
        pSource = bufferDescriptor.buffers[BAPE_Channel_eLeft].pBuffer;
        copied = 0;
        while ( bufferSize >= 16 )
        {
            size_t available, bytesToCopy;
            available = NEXUS_AudioCapture_P_GetContiguousSpace(handle);
            if ( available < 4 )
            {
                /* Our buffer is full. */
                break;
            }
            if ( available >= bufferSize/4 )
            {
                bytesToCopy = bufferSize/4;
            }
            else
            {
                bytesToCopy = available;
            }
            while ( bytesToCopy > 4 )
            {
                uint32_t sample;
                if ( rightChannel )
                {
                    pSource++;                          /* Skip Left1 */
                    sample = (*pSource++)&0xffff0000;   /* Right1 */
                    pSource++;                          /* Skip Left2 */
                    sample |= (*pSource++)>>16;         /* Right2 */
                }
                else
                {
                    sample = (*pSource++)&0xffff0000;   /* Left1 */
                    pSource++;                          /* Skip Right1 */
                    sample |= (*pSource++)>>16;         /* Left2 */
                    pSource++;                          /* Skip Right2 */
                }
                handle->pBuffer[handle->wptr] = sample;
                NEXUS_AudioCapture_P_AdvanceBuffer(handle, 4);
                copied += 16;
                bufferSize -= 16;
                bytesToCopy -= 4;
            }
        }
        if ( copied == 0 )
        {
            return;
        }
        errCode = handle->consumeData(handle->piHandle, copied);
        if ( errCode )
        {
            (void)BERR_TRACE(errCode);
            return;
        }
    }
}

static void NEXUS_AudioCapture_P_ConvertMonoMix(NEXUS_AudioCaptureProcessorHandle handle)
{
    BERR_Code errCode;
    uint32_t *pSource;
    size_t bufferSize, copied;
    BAPE_BufferDescriptor bufferDescriptor;

    for ( ;; )
    {
        errCode = handle->getBuffer(handle->piHandle, &bufferDescriptor);
        if ( errCode )
        {
            (void)BERR_TRACE(errCode);
            return;
        }
        bufferSize = bufferDescriptor.bufferSize;
        if ( bufferSize == 0 )
        {
            /* Done */
            return;
        }
        BMMA_FlushCache(bufferDescriptor.buffers[BAPE_Channel_eLeft].block, bufferDescriptor.buffers[BAPE_Channel_eLeft].pBuffer, bufferSize);
        pSource = bufferDescriptor.buffers[BAPE_Channel_eLeft].pBuffer;
        copied = 0;
        while ( bufferSize >= 16 )
        {
            size_t available, bytesToCopy;
            available = NEXUS_AudioCapture_P_GetContiguousSpace(handle);
            if ( available < 4 )
            {
                /* Our buffer is full. */
                break;
            }
            if ( available >= bufferSize/4 )
            {
                bytesToCopy = bufferSize/4;
            }
            else
            {
                bytesToCopy = available;
            }
            while ( bytesToCopy > 0 )
            {
                uint32_t sample1, sample2;
                sample1 = (*pSource++)>>16;
                sample1 += (*pSource++)>>16;
                sample1 /= 2;
                sample2 = (*pSource++)>>16;
                sample2 += (*pSource++)>>16;
                sample2 /= 2;
                handle->pBuffer[handle->wptr] = (sample1<<16)|sample2;
                NEXUS_AudioCapture_P_AdvanceBuffer(handle, 4);
                copied += 16;
                bufferSize -= 16;
                bytesToCopy -= 4;
            }
        }
        if ( copied == 0 )
        {
            return;
        }
        errCode = handle->consumeData(handle->piHandle, copied);
        if ( errCode )
        {
            (void)BERR_TRACE(errCode);
            return;
        }
    }
}

#else
/* stub */

typedef struct NEXUS_AudioCapture
{
    NEXUS_OBJECT(NEXUS_AudioCapture);
} NEXUS_AudioCapture;

void NEXUS_AudioCapture_GetDefaultOpenSettings(
    NEXUS_AudioCaptureOpenSettings *pSettings      /* [out] default settings */
    )
{
    BSTD_UNUSED(pSettings);
    return;
}

NEXUS_AudioCaptureHandle NEXUS_AudioCapture_Open(     /* attr{destructor=NEXUS_AudioCapture_Close}  */
    unsigned index,
    const NEXUS_AudioCaptureOpenSettings *pSettings    /* Pass NULL for default settings */
    )
{
    BSTD_UNUSED(index);
    BSTD_UNUSED(pSettings);
    (void)BERR_TRACE(BERR_NOT_SUPPORTED);
    return NULL;
}

static void NEXUS_AudioCapture_P_Finalizer(
    NEXUS_AudioCaptureHandle handle
    )
{
    BSTD_UNUSED(handle);
}

NEXUS_OBJECT_CLASS_MAKE(NEXUS_AudioCapture, NEXUS_AudioCapture_Close);

void NEXUS_AudioCapture_GetSettings(
    NEXUS_AudioCaptureHandle handle,
    NEXUS_AudioCaptureSettings *pSettings /* [out] */
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pSettings);
}

NEXUS_Error NEXUS_AudioCapture_SetSettings(
    NEXUS_AudioCaptureHandle handle,
    const NEXUS_AudioCaptureSettings *pSettings
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pSettings);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

void NEXUS_AudioCapture_GetDefaultStartSettings(
    NEXUS_AudioCaptureStartSettings *pSettings  /* [out] default settings */
    )
{
    BSTD_UNUSED(pSettings);
}

NEXUS_Error NEXUS_AudioCapture_Start(
    NEXUS_AudioCaptureHandle handle,
    const NEXUS_AudioCaptureStartSettings *pSettings
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pSettings);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

void NEXUS_AudioCapture_Stop(
    NEXUS_AudioCaptureHandle handle
    )
{
    BSTD_UNUSED(handle);
}

NEXUS_Error NEXUS_AudioCapture_GetBuffer(
    NEXUS_AudioCaptureHandle handle,
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

NEXUS_Error NEXUS_AudioCapture_WriteComplete(
    NEXUS_AudioCaptureHandle handle,
    size_t amountWritten            /* The number of bytes read from the buffer */
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(amountWritten);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

NEXUS_AudioOutputHandle NEXUS_AudioCapture_GetConnector( /* attr{shutdown=NEXUS_AudioOutput_Shutdown} */
    NEXUS_AudioCaptureHandle handle
    )
{
    BSTD_UNUSED(handle);
    return NULL;
}

NEXUS_Error NEXUS_AudioCapture_GetStatus(
    NEXUS_AudioCaptureHandle handle,
    NEXUS_AudioCaptureStatus *pStatus   /* [out] */
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pStatus);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

#endif
