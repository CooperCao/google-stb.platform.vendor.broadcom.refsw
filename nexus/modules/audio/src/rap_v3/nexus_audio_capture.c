/***************************************************************************
*     (c)2004-2013 Broadcom Corporation
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
*   API name: AudioCapture
*    Specific APIs related to PCM audio capture.  This supports capture
*    of data into memory from a decoder or other source.
*
* Revision History:
*
* $brcm_Log: $
*
***************************************************************************/

#include "nexus_audio_module.h"
#include "nexus_audio_capture.h"

BDBG_MODULE(nexus_audio_capture);

#if NEXUS_NUM_AUDIO_CAPTURES

/***************************************************************************
Summary:
Handle for audio capture
***************************************************************************/
typedef struct NEXUS_AudioCapture
{
    NEXUS_OBJECT(NEXUS_AudioCapture);
    NEXUS_AudioCaptureOpenSettings openSettings;
    NEXUS_AudioCaptureSettings settings;
    uint32_t *pBuffer, *pCachedBuffer;
    int bufferSize, watermark, rptr, wptr, bufferDepth;
    NEXUS_AudioOutputObject connector;
    NEXUS_IsrCallbackHandle dataCallback, sampleRateCallback;
    BRAP_ChannelHandle channel;
    BRAP_DestinationHandle destination;
    bool running;
    bool opened;
    bool compressed;
    bool multichannel;
    unsigned sampleRate;
    void *pMemory, *pCachedMemory;
    BMEM_Heap_Handle fifoMem; /* heap used for fifo */

#define RESOLVE_ALIAS(handle) do {(handle) = ((handle)->alias.master?(handle)->alias.master:(handle));}while(0)
#define IS_ALIAS(handle) ((handle)->alias.master != NULL)
    struct {
        NEXUS_AudioCaptureHandle master, slave;
    } alias;
} NEXUS_AudioCapture;

static void NEXUS_AudioCapture_P_ConvertCompressed(NEXUS_AudioCaptureHandle handle);
static void NEXUS_AudioCapture_P_ConvertStereo16(NEXUS_AudioCaptureHandle handle);
static void NEXUS_AudioCapture_P_ConvertStereo24(NEXUS_AudioCaptureHandle handle);
static void NEXUS_AudioCapture_P_ConvertMultichannel(NEXUS_AudioCaptureHandle handle);
static void NEXUS_AudioCapture_P_ConvertMono(NEXUS_AudioCaptureHandle handle, bool rightChannel);
static void NEXUS_AudioCapture_P_ConvertMonoMix(NEXUS_AudioCaptureHandle handle);
static void NEXUS_AudioCapture_P_Interrupt_isr(void *pParam, int param, void *pData);
static void NEXUS_AudioCapture_P_SampleRate_isr(void *pParam, int param, void *pData);
static void NEXUS_AudioCapture_P_FlushDeviceBuffer_isr(NEXUS_AudioCaptureHandle handle);
static uint32_t *NEXUS_AudioCapture_P_ConvertAddress(void *pAddress);
static void NEXUS_AudioCapture_P_FlushChannelPair_isr(NEXUS_AudioCaptureHandle handle, BRAP_OutputChannelPair chPair);

/* Set this to one to use cached memory */
#define NEXUS_AUDIO_CAPTURE_USE_CACHED_MEMORY 0

#define NEXUS_AUDIO_CAPTURE_CAPTURE_FIFO_SIZE (g_NEXUS_audioModuleData.moduleSettings.audioCaptureChannels*g_NEXUS_audioModuleData.moduleSettings.audioCaptureFifoSize)
static NEXUS_AudioCapture g_captures[NEXUS_NUM_AUDIO_CAPTURES];

/***************************************************************************
Summary:
Get default settings for opening an audio capture channel
***************************************************************************/
void NEXUS_AudioCapture_GetDefaultOpenSettings(
    NEXUS_AudioCaptureOpenSettings *pSettings      /* [out] default settings */
    )
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->fifoSize = NEXUS_AUDIO_CAPTURE_CAPTURE_FIFO_SIZE;
}

/***************************************************************************
Summary:
Open an audio capture channel

Description:
The captured data will be formatted with the settings provided in
NEXUS_AudioCaptureSettings.  The sample rate of the data is determined by
the sample rate of the input data.  For example, if the decoder is outputting
48kHz, the data will be the same sample rate.  If the decoder is not active,
the output will be 48kHz.  Data will automatically start flowing once the
source device starts and stop when the source device stops.
***************************************************************************/
NEXUS_AudioCaptureHandle NEXUS_AudioCapture_Open(     /* attr{destructor=NEXUS_AudioCapture_Close}  */
    unsigned index,
    const NEXUS_AudioCaptureOpenSettings *pSettings    /* Pass NULL for default settings */
    )
{
    BERR_Code rc = BERR_SUCCESS;
    NEXUS_AudioCaptureHandle handle;
    NEXUS_AudioCaptureHandle master = NULL;
    NEXUS_HeapHandle heap = NULL;

    BSTD_UNUSED(index);

    #if NEXUS_NUM_AUDIO_CAPTURES
    if (index >= NEXUS_ALIAS_ID && index-NEXUS_ALIAS_ID < NEXUS_NUM_AUDIO_CAPTURES) {
        BDBG_MSG(("%d aliasing %d(%p)", index, index-NEXUS_ALIAS_ID, &g_captures[index-NEXUS_ALIAS_ID]));
        index -= NEXUS_ALIAS_ID;
        master = &g_captures[index];
        if (!master->opened) {
            BDBG_ERR(("cannot alias %d because it is not opened", index));
            return NULL;
        }
        if (master->alias.slave) {
            BDBG_ERR(("cannot alias %d a second time", index));
            return NULL;
        }
    }
    if ( index > NEXUS_NUM_AUDIO_CAPTURES )
    #endif
    {
        BDBG_ERR(("Index out of range.  You must open an audio capture index < NEXUS_NUM_AUDIO_CAPTURES"));
        (void)BERR_TRACE(BERR_NOT_SUPPORTED);
        return NULL;
    }

    if (!master) {
        handle = &g_captures[index];
        if ( handle->opened )
        {
            BDBG_ERR(("AudioCapture %u already open", index));
            (void)BERR_TRACE(BERR_NOT_SUPPORTED);
            return NULL;
        }
    }
    else {
        handle = BKNI_Malloc(sizeof(*handle));
    }
    if ( NULL == handle )
    {
        (void)BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        return NULL;
    }

    NEXUS_OBJECT_INIT(NEXUS_AudioCapture, handle);
    if (master) {
        handle->alias.master = master;
        master->alias.slave = handle;
        return handle;
    }

    if ( NULL != pSettings )
    {
        handle->openSettings = *pSettings;
    }
    else
    {
        NEXUS_AudioCapture_GetDefaultOpenSettings(&handle->openSettings);
    }

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

    /* Setup internal buffer and thresholds */
    if ( handle->openSettings.fifoSize != NEXUS_AUDIO_CAPTURE_CAPTURE_FIFO_SIZE )
    {
        BDBG_ERR(("This chip requires capture FIFO size to be set in NEXUS_AudioModuleSettings.audioCaptureFifoSize, please adjust that size to the desired value."));
        (void)BERR_TRACE(BERR_INVALID_PARAMETER);
        goto err_fifo_size;
    }


    handle->bufferSize = handle->openSettings.fifoSize;
    if ( 0 == handle->openSettings.threshold )
    {
        handle->watermark = handle->bufferSize/2;
    }
    else
    {
        handle->watermark = handle->openSettings.threshold;
    }
    heap = NEXUS_P_DefaultHeap(handle->openSettings.heap, NEXUS_DefaultHeapType_eFull);
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

    handle->fifoMem = NEXUS_Heap_GetMemHandle(heap);

    handle->pMemory = BMEM_Heap_Alloc(handle->fifoMem, handle->openSettings.fifoSize);
    if ( NULL == handle->pMemory )
    {
        (void)BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
        goto err_buffer_alloc;
        return NULL;
    }

    rc = BMEM_Heap_ConvertAddressToCached(handle->fifoMem, handle->pMemory, &handle->pCachedMemory);
    if ( rc )
    {
        (void)BERR_TRACE(rc);
        goto err_buffer_cache;
        return NULL;
    }

    handle->pCachedBuffer = handle->pBuffer = handle->pCachedMemory;



    NEXUS_AUDIO_OUTPUT_INIT(&handle->connector, NEXUS_AudioOutputType_eCapture, handle);
    NEXUS_OBJECT_REGISTER(NEXUS_AudioOutput, &handle->connector, Open);
    handle->opened = true;
    handle->connector.port = BRAP_OutputPort_eMax;  /* No actual output port */

    /* Success */
    return handle;
err_buffer_cache:
    BMEM_Heap_Free(handle->fifoMem, handle->pMemory);
err_buffer_alloc:
err_heap:
err_fifo_size:
    NEXUS_IsrCallback_Destroy(handle->sampleRateCallback);
err_sample_rate_callback:
    NEXUS_IsrCallback_Destroy(handle->dataCallback);
err_data_callback:
    NEXUS_OBJECT_DESTROY(NEXUS_AudioCapture, handle);
    BKNI_Free(handle);
    return NULL;
}

/***************************************************************************
Summary:
Close an audio capture channel
***************************************************************************/
static void NEXUS_AudioCapture_P_Finalizer(
    NEXUS_AudioCaptureHandle handle
    )
{
    NEXUS_OBJECT_ASSERT(NEXUS_AudioCapture, handle);

    if (handle->alias.master) {
        handle->alias.master->alias.slave = NULL;
        NEXUS_OBJECT_DESTROY(NEXUS_AudioCapture, handle);
        BKNI_Free(handle);
        return;
    }
    else {
        if (handle->alias.slave) {
            NEXUS_OBJECT_UNREGISTER(NEXUS_AudioCapture, handle->alias.slave, Close);
            NEXUS_AudioCapture_Close(handle->alias.slave);
        }
    }

    BDBG_ASSERT(handle->opened);
    NEXUS_AudioOutput_Shutdown(&handle->connector);
    BMEM_Free(handle->fifoMem, handle->pMemory);
    NEXUS_IsrCallback_Destroy(handle->dataCallback);
    NEXUS_IsrCallback_Destroy(handle->sampleRateCallback);
    NEXUS_OBJECT_DESTROY(NEXUS_AudioCapture, handle);
    BKNI_Memset(handle, 0, sizeof(NEXUS_AudioCapture));
}


/***************************************************************************
Summary:
Get Audio Capture Channel Settings
***************************************************************************/
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

/***************************************************************************
Summary:
Set Audio Capture Channel Settings
***************************************************************************/
NEXUS_Error NEXUS_AudioCapture_SetSettings(
    NEXUS_AudioCaptureHandle handle,
    const NEXUS_AudioCaptureSettings *pSettings
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioCapture);
    RESOLVE_ALIAS(handle);
    BDBG_ASSERT(handle->opened);
    BDBG_ASSERT(NULL != pSettings);
    handle->settings = *pSettings;
    return BERR_SUCCESS;
}

/***************************************************************************
Summary:
Get current setting of the audio capture channel
***************************************************************************/
void NEXUS_AudioCapture_GetDefaultStartSettings(
    NEXUS_AudioCaptureStartSettings *pSettings  /* [out] default settings */
    )
{
    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
}

/***************************************************************************
Summary:
Start capturing data.

Decription:
An input must be connected prior to starting.

See Also:
NEXUS_AudioCapture_Stop
NEXUS_AudioCapture_GetConnector
***************************************************************************/
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

    if ( !NEXUS_AudioOutput_P_IsConnected(&handle->connector) )
    {
        BDBG_ERR(("Not connected to any input"));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    /* Setup callback */
    NEXUS_IsrCallback_Set(handle->dataCallback, pSettings?&pSettings->dataCallback:NULL);
    NEXUS_IsrCallback_Set(handle->sampleRateCallback, pSettings?&pSettings->sampleRateCallback:NULL);

    if ( handle->channel )
    {
        #if NEXUS_AUDIO_CAPTURE_USE_CACHED_MEMORY
        BRAP_PcmBufInfo bufInfo;
        void *pCachedAddr;
        #endif

        #if NEXUS_AUDIO_CAPTURE_USE_CACHED_MEMORY
        /* Kick any stale data out of the cache */
        BRAP_GetRingBufInfo(handle->destination, &bufInfo, BRAP_OutputChannelPair_eLR);
        pCachedAddr = NEXUS_AudioCapture_P_ConvertAddress(bufInfo.sLtOrSnglBufInfo.pBasePtr);
        NEXUS_FlushCache(pCachedAddr, ((size_t)bufInfo.sLtOrSnglBufInfo.pBasePtr - (size_t)bufInfo.sLtOrSnglBufInfo.pBasePtr) + 1);
        if ( NULL != bufInfo.sRtBufInfo.pBasePtr )
        {
            pCachedAddr = NEXUS_AudioCapture_P_ConvertAddress(bufInfo.sRtBufInfo.pBasePtr);
            NEXUS_FlushCache(pCachedAddr, ((size_t)bufInfo.sRtBufInfo.pBasePtr - (size_t)bufInfo.sRtBufInfo.pBasePtr) + 1);
        }
        #endif

        BDBG_MSG(("Starting - channel and destination valid"));

        /* flush anything pending in our buffer and in raptor */
        BKNI_EnterCriticalSection();
        NEXUS_AudioCapture_P_FlushDeviceBuffer_isr(handle);
        handle->running = true;
        handle->wptr = handle->rptr = handle->bufferDepth = 0;
        BKNI_LeaveCriticalSection();
    }
    else
    {
        BDBG_MSG(("Starting - channel/destination not yet valid"));

        /* flush anything pending in our buffer and in raptor */
        BKNI_EnterCriticalSection();
        handle->running = true;
        handle->wptr = handle->rptr = handle->bufferDepth = 0;
        BKNI_LeaveCriticalSection();
    }

    return BERR_SUCCESS;
}

/***************************************************************************
Summary:
Stop capturing data.
***************************************************************************/
void NEXUS_AudioCapture_Stop(
    NEXUS_AudioCaptureHandle handle
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioCapture);
    RESOLVE_ALIAS(handle);
    BDBG_MSG(("Stopping"));

    /* flush anything pending in our buffer and in raptor */
    BKNI_EnterCriticalSection();
    handle->running = false;
    BKNI_LeaveCriticalSection();

    NEXUS_IsrCallback_Set(handle->dataCallback, NULL);
    NEXUS_IsrCallback_Set(handle->sampleRateCallback, NULL);
}

/***************************************************************************
Summary:
Get a pointer and size for the next location in the buffer that contains data

Description:
NEXUS_AudioCapture_GetBuffer is non-destructive. You can safely call it
multiple times.
***************************************************************************/
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

    /* Defaults for error returns */
    *ppBuffer = NULL;
    *pSize = 0;

    BDBG_MSG(("GetBuffer - compressed %d, format %d running %d channel %p",
              handle->compressed, handle->settings.format,
              handle->running, (void *)handle->channel));

    if ( !handle->running )
    {
        BDBG_MSG(("running %d channel %p", handle->running, handle->channel));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    if ( NULL == handle->channel )
    {
        return BERR_SUCCESS;
    }

    BDBG_MSG(("Before Conversion rptr %d wptr %d depth %d/%d", handle->rptr, handle->wptr, handle->bufferDepth, handle->bufferSize));

    if ( handle->compressed )
    {
        NEXUS_AudioCapture_P_ConvertCompressed(handle);
    }
    else
    {
        switch ( handle->settings.format )
        {
        case NEXUS_AudioCaptureFormat_e16BitStereo:
            NEXUS_AudioCapture_P_ConvertStereo16(handle);
            break;
        case NEXUS_AudioCaptureFormat_e24BitStereo:
            NEXUS_AudioCapture_P_ConvertStereo24(handle);
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
            NEXUS_AudioCapture_P_ConvertMultichannel(handle);
            break;
        default:
            BDBG_ERR(("Invalid capture format"));
            BDBG_ASSERT(false);
            return BERR_TRACE(BERR_INVALID_PARAMETER);
        }
    }

    *ppBuffer = &handle->pCachedBuffer[handle->rptr];
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

    BDBG_MSG(("rptr %d wptr %d size %d words (%d bytes)", handle->rptr, handle->wptr, (*pSize)/4, *pSize));

    BRAP_RingBufferReadDone(handle->destination);

    return BERR_SUCCESS;
}

/***************************************************************************
Summary:
Notify AudioCapture how much data removed from the buffer.

Description:
You can only call NEXUS_AudioCapture_ReadComplete once after a
NEXUS_AudioCapture_GetBuffer call.  After calling it, you must call
NEXUS_AudioCapture_GetBuffer before reading more data.
***************************************************************************/
NEXUS_Error NEXUS_AudioCapture_ReadComplete(
    NEXUS_AudioCaptureHandle handle,
    size_t amountWritten            /* The number of bytes read from the buffer */
    )
{
    int newRptr;

    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioCapture);
    RESOLVE_ALIAS(handle);

    if ( NULL == handle || !handle->running )
    {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

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

    BDBG_MSG(("Write Complete - %d bytes written, moving rptr from %d to %d (size %d)", amountWritten, handle->rptr, newRptr, handle->bufferSize/4));

    /* Update read pointer of our buffer */
    handle->rptr = newRptr;
    handle->bufferDepth -= amountWritten;

    return BERR_SUCCESS;
}

/***************************************************************************
Summary:
Get an audio connector to attach this output to a source.

Description:
Capture may be connected to a stereo source device or a compressed source
device, depending on the data format desired.
**************************************************************************/
NEXUS_AudioOutputHandle NEXUS_AudioCapture_GetConnector( /* attr{shutdown=NEXUS_AudioOutput_Shutdown} */
    NEXUS_AudioCaptureHandle handle
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioCapture);
    RESOLVE_ALIAS(handle);
    return &handle->connector;
}

static void NEXUS_AudioCapture_P_ConvertCompressed(NEXUS_AudioCaptureHandle handle)
{
    BRAP_PcmBufInfo bufInfo;
    uint32_t *pCachedAddr, *pOrigAddr;
    BRAP_PcmBufSzUpdt update;


    BRAP_GetRingBufInfo(handle->destination, &bufInfo, BRAP_OutputChannelPair_eLR);

    pOrigAddr = pCachedAddr = NEXUS_AudioCapture_P_ConvertAddress(bufInfo.sLtOrSnglBufInfo.pReadPtr);
    update.uiLtOrSnglBufSzUpdt = 0;
    update.uiRtBufSzUpdt = 0;

#if BRAP_VER >= 4 || BCHP_CHIP == 3548 || BCHP_CHIP == 3556
    /* Do 32-bit copies */
    while ( bufInfo.sLtOrSnglBufInfo.uiContiguousSize > 0 &&
            handle->wptr+1 != handle->rptr )
    {
        handle->pCachedBuffer[handle->wptr++] = *pCachedAddr++;
        handle->bufferDepth += 4;
        bufInfo.sLtOrSnglBufInfo.uiContiguousSize-=4;
        update.uiLtOrSnglBufSzUpdt += 4;
        if ( handle->wptr >= (handle->bufferSize/4) )
        {
            /* Wrap of our ringbuffer */
            handle->wptr = 0;
        }
    }
#else
    /* This hardware does not pack the 16-bit data.  Do it here instead. */
    while ( bufInfo.sLtOrSnglBufInfo.uiContiguousSize > 1 &&
            handle->wptr+1 != handle->rptr )
    {
        uint32_t data;

        data  = (*pCachedAddr++)>>16;
        data |= (*pCachedAddr++)&0xFFFF0000;
        bufInfo.sLtOrSnglBufInfo.uiContiguousSize-=8;
        update.uiLtOrSnglBufSzUpdt+=8;
        
        
        /* Store sample */
        handle->pCachedBuffer[handle->wptr++] = data;
        handle->bufferDepth += 4;
        if ( handle->wptr >= (handle->bufferSize/4) )
        {
            /* Wrap of our ringbuffer */
            handle->wptr = 0;
        }
    }
#endif
    if ( update.uiLtOrSnglBufSzUpdt > 0 )
    {
        #if NEXUS_AUDIO_CAPTURE_USE_CACHED_MEMORY
        NEXUS_FlushCache(pOrigAddr, update.uiLtOrSnglBufSzUpdt);
        #endif

        (void)BRAP_UpdateRingBufUsg(handle->destination, &update, BRAP_OutputChannelPair_eLR);
    }
}

static void NEXUS_AudioCapture_P_ConvertStereo16(NEXUS_AudioCaptureHandle handle)
{
    BRAP_PcmBufInfo bufInfo;
    BRAP_PcmBufSzUpdt update;
    uint32_t *pCachedAddrL, *pCachedAddrR, *pOrigAddrR, *pOrigAddrL;
    uint32_t data, left, right;

    BRAP_GetRingBufInfo(handle->destination, &bufInfo, BRAP_OutputChannelPair_eLR);

    pOrigAddrL = pCachedAddrL = NEXUS_AudioCapture_P_ConvertAddress(bufInfo.sLtOrSnglBufInfo.pReadPtr);
    pOrigAddrR = pCachedAddrR = NEXUS_AudioCapture_P_ConvertAddress(bufInfo.sRtBufInfo.pReadPtr);

    /* Round down to smaller of two buffers if different */
    if ( bufInfo.sRtBufInfo.uiContiguousSize < bufInfo.sLtOrSnglBufInfo.uiContiguousSize )
    {
        bufInfo.sLtOrSnglBufInfo.uiContiguousSize = bufInfo.sRtBufInfo.uiContiguousSize;
    }
    BDBG_MSG(("ConvertStereo16 - received %u bytes to convert", bufInfo.sLtOrSnglBufInfo.uiContiguousSize));

    update.uiLtOrSnglBufSzUpdt=0;
    update.uiRtBufSzUpdt=0;

    /* Do 32-bit copies */
    while ( bufInfo.sLtOrSnglBufInfo.uiContiguousSize > 0 &&
            handle->wptr+1 != handle->rptr )
    {
        /* Get right sample */
        right = *pCachedAddrR++;
        data = right & 0xFFFF0000;  /* Convert to 16-bit sample from 24-bit Left-aligned and place in upper word */
        bufInfo.sRtBufInfo.uiContiguousSize-=4;
        update.uiRtBufSzUpdt+=4;

        /* Get left sample */
        left = *pCachedAddrL++;
        data |= left >> 16; /* convert to 16-bit sample from 24-bit Left-aligned and place in lower word */
        bufInfo.sLtOrSnglBufInfo.uiContiguousSize-=4;
        update.uiLtOrSnglBufSzUpdt+=4;

        #if 0
        /* DEBUG */
        if ( left != right )
        {
            BDBG_MSG(("Convert16: L 0x%08x R 0x%08x OUT 0x%08x", left, right, data));
        }
        #endif

        /* Store sample */
        handle->pCachedBuffer[handle->wptr++] = data;
        handle->bufferDepth += 4;
        if ( handle->wptr >= (handle->bufferSize/4) )
        {
            /* Wrap of our ringbuffer */
            handle->wptr = 0;
        }
    }
    if ( update.uiLtOrSnglBufSzUpdt > 0 )
    {
        #if NEXUS_AUDIO_CAPTURE_USE_CACHED_MEMORY
        NEXUS_FlushCache(pOrigAddrL, update.uiLtOrSnglBufSzUpdt);
        NEXUS_FlushCache(pOrigAddrR, update.uiRtBufSzUpdt);
        #endif

        (void)BRAP_UpdateRingBufUsg(handle->destination, &update, BRAP_OutputChannelPair_eLR);
        BKNI_EnterCriticalSection();
        if ( handle->multichannel )
        {
            NEXUS_AudioCapture_P_FlushChannelPair_isr(handle, BRAP_OutputChannelPair_eLRSurround);
            NEXUS_AudioCapture_P_FlushChannelPair_isr(handle, BRAP_OutputChannelPair_eCentreLF);
        }
        BKNI_LeaveCriticalSection();
    }
}

static void NEXUS_AudioCapture_P_ConvertStereo24(NEXUS_AudioCaptureHandle handle)
{
    BRAP_PcmBufInfo bufInfo;
    BRAP_PcmBufSzUpdt update;
    uint32_t *pCachedAddrL, *pCachedAddrR, *pOrigAddrL, *pOrigAddrR;

    BRAP_GetRingBufInfo(handle->destination, &bufInfo, BRAP_OutputChannelPair_eLR);
    pOrigAddrL = pCachedAddrL = NEXUS_AudioCapture_P_ConvertAddress(bufInfo.sLtOrSnglBufInfo.pReadPtr);
    pOrigAddrR = pCachedAddrR = NEXUS_AudioCapture_P_ConvertAddress(bufInfo.sRtBufInfo.pReadPtr);

    /* Round down to smaller of two buffers if different */
    if ( bufInfo.sRtBufInfo.uiContiguousSize < bufInfo.sLtOrSnglBufInfo.uiContiguousSize )
    {
        bufInfo.sLtOrSnglBufInfo.uiContiguousSize = bufInfo.sRtBufInfo.uiContiguousSize;
    }

    update.uiLtOrSnglBufSzUpdt=0;
    update.uiRtBufSzUpdt=0;

    /* Do 32-bit copies */
    while ( bufInfo.sLtOrSnglBufInfo.uiContiguousSize > 0 &&
            handle->bufferDepth + 8 <= handle->bufferSize )
    {
        /* Get left sample */
        handle->pCachedBuffer[handle->wptr++] = *pCachedAddrL++;
        handle->bufferDepth += 4;
        bufInfo.sLtOrSnglBufInfo.uiContiguousSize-=4;
        update.uiLtOrSnglBufSzUpdt+=4;
        if ( handle->wptr >= (handle->bufferSize/4) )
        {
            /* Wrap of our ringbuffer */
            handle->wptr = 0;
        }

        /* Get right sample */
        handle->pCachedBuffer[handle->wptr++] = *pCachedAddrR++;  /* convert to 16-bit sample from 24-bit Left-aligned and place in lower word */
        handle->bufferDepth += 4;
        bufInfo.sRtBufInfo.uiContiguousSize-=4;
        update.uiRtBufSzUpdt+=4;
        if ( handle->wptr >= (handle->bufferSize/4) )
        {
            /* Wrap of our ringbuffer */
            handle->wptr = 0;
        }
    }
    if ( update.uiLtOrSnglBufSzUpdt > 0 )
    {
        #if NEXUS_AUDIO_CAPTURE_USE_CACHED_MEMORY
        NEXUS_FlushCache(pOrigAddrL, update.uiLtOrSnglBufSzUpdt);
        NEXUS_FlushCache(pOrigAddrR, update.uiRtBufSzUpdt);
        #endif

        (void)BRAP_UpdateRingBufUsg(handle->destination, &update, BRAP_OutputChannelPair_eLR);
        BKNI_EnterCriticalSection();
        if ( handle->multichannel )
        {
            NEXUS_AudioCapture_P_FlushChannelPair_isr(handle, BRAP_OutputChannelPair_eLRSurround);
            NEXUS_AudioCapture_P_FlushChannelPair_isr(handle, BRAP_OutputChannelPair_eCentreLF);
        }
        BKNI_LeaveCriticalSection();
    }
}

static void NEXUS_AudioCapture_P_ConvertMultichannel(NEXUS_AudioCaptureHandle handle)
{
    BRAP_PcmBufInfo bufInfo;
    BRAP_PcmBufSzUpdt update;
    uint32_t *pL, *pR, *pLs, *pRs, *pC, *pLfe;
    unsigned contigSize, samplesUsed=0;

    BRAP_GetRingBufInfo(handle->destination, &bufInfo, BRAP_OutputChannelPair_eLR);
    BDBG_MSG(("L %d R %d", bufInfo.sLtOrSnglBufInfo.uiContiguousSize, bufInfo.sRtBufInfo.uiContiguousSize));
    contigSize = (bufInfo.sRtBufInfo.uiContiguousSize < bufInfo.sLtOrSnglBufInfo.uiContiguousSize)?
        bufInfo.sRtBufInfo.uiContiguousSize:bufInfo.sLtOrSnglBufInfo.uiContiguousSize;
    pL = NEXUS_AudioCapture_P_ConvertAddress(bufInfo.sLtOrSnglBufInfo.pReadPtr);
    pR = NEXUS_AudioCapture_P_ConvertAddress(bufInfo.sRtBufInfo.pReadPtr);
    if ( handle->multichannel )
    {
        unsigned size;

        BRAP_GetRingBufInfo(handle->destination, &bufInfo, BRAP_OutputChannelPair_eLRSurround);
        pLs = NEXUS_AudioCapture_P_ConvertAddress(bufInfo.sLtOrSnglBufInfo.pReadPtr);
        pRs = NEXUS_AudioCapture_P_ConvertAddress(bufInfo.sRtBufInfo.pReadPtr);
        BDBG_MSG(("Ls %d Rs %d", bufInfo.sLtOrSnglBufInfo.uiContiguousSize, bufInfo.sRtBufInfo.uiContiguousSize));
        size = (bufInfo.sRtBufInfo.uiContiguousSize < bufInfo.sLtOrSnglBufInfo.uiContiguousSize)?
            bufInfo.sRtBufInfo.uiContiguousSize:bufInfo.sLtOrSnglBufInfo.uiContiguousSize;
        contigSize = (contigSize < size)?contigSize:size;

        BRAP_GetRingBufInfo(handle->destination, &bufInfo, BRAP_OutputChannelPair_eCentreLF);
        pC = NEXUS_AudioCapture_P_ConvertAddress(bufInfo.sLtOrSnglBufInfo.pReadPtr);
        pLfe = NEXUS_AudioCapture_P_ConvertAddress(bufInfo.sRtBufInfo.pReadPtr);
        BDBG_MSG(("C %d LFE %d", bufInfo.sLtOrSnglBufInfo.uiContiguousSize, bufInfo.sRtBufInfo.uiContiguousSize));
        size = (bufInfo.sRtBufInfo.uiContiguousSize < bufInfo.sLtOrSnglBufInfo.uiContiguousSize)?
            bufInfo.sRtBufInfo.uiContiguousSize:bufInfo.sLtOrSnglBufInfo.uiContiguousSize;
        contigSize = (contigSize < size)?contigSize:size;
    }
    else
    {
        pLs = pRs = pC = pLfe = NULL;
    }

    /* Round down to smaller of two buffers if different */
    if ( bufInfo.sRtBufInfo.uiContiguousSize < bufInfo.sLtOrSnglBufInfo.uiContiguousSize )
    {
        bufInfo.sLtOrSnglBufInfo.uiContiguousSize = bufInfo.sRtBufInfo.uiContiguousSize;
    }

    update.uiLtOrSnglBufSzUpdt=0;
    update.uiRtBufSzUpdt=0;

    BDBG_MSG(("%d contiguous bytes (size %d depth %d)", contigSize, handle->bufferSize, handle->bufferDepth));

    /* Do 32-bit copies */
    for ( ; contigSize > 0 && handle->bufferDepth + 24 <= handle->bufferSize; contigSize -= 4 )
    {
        uint32_t ls,rs,c,lfe;

        samplesUsed += 1;
        handle->bufferDepth += 24;

        /* Get left sample */
        handle->pCachedBuffer[handle->wptr++] = *pL++;
        if ( handle->wptr >= (handle->bufferSize/4) )
        {
            /* Wrap of our ringbuffer */
            handle->wptr = 0;
        }

        /* Get right sample */
        handle->pCachedBuffer[handle->wptr++] = *pR++;
        if ( handle->wptr >= (handle->bufferSize/4) )
        {
            /* Wrap of our ringbuffer */
            handle->wptr = 0;
        }

        if ( handle->multichannel )
        {
            ls = *pLs++;
            rs = *pRs++;
            c = *pC++;
            lfe = *pLfe++;
        }
        else
        {
            ls=rs=c=lfe=0;
        }

        /* Get ls sample */
        handle->pCachedBuffer[handle->wptr++] = ls;
        if ( handle->wptr >= (handle->bufferSize/4) )
        {
            /* Wrap of our ringbuffer */
            handle->wptr = 0;
        }

        /* Get rs sample */
        handle->pCachedBuffer[handle->wptr++] = rs;
        if ( handle->wptr >= (handle->bufferSize/4) )
        {
            /* Wrap of our ringbuffer */
            handle->wptr = 0;
        }

        /* Get c sample */
        handle->pCachedBuffer[handle->wptr++] = c;
        if ( handle->wptr >= (handle->bufferSize/4) )
        {
            /* Wrap of our ringbuffer */
            handle->wptr = 0;
        }

        /* Get lfe sample */
        handle->pCachedBuffer[handle->wptr++] = lfe;
        if ( handle->wptr >= (handle->bufferSize/4) )
        {
            /* Wrap of our ringbuffer */
            handle->wptr = 0;
        }
    }

    BDBG_MSG(("Conversion complete"));

    if ( samplesUsed > 0 )
    {
        #if NEXUS_AUDIO_CAPTURE_USE_CACHED_MEMORY
        NEXUS_FlushCache(pL, samplesUsed*4);
        NEXUS_FlushCache(pR, samplesUsed*4);
        if ( multichannel )
        {
            NEXUS_FlushCache(pLs, samplesUsed*4);
            NEXUS_FlushCache(pRs, samplesUsed*4);
            NEXUS_FlushCache(pC, samplesUsed*4);
            NEXUS_FlushCache(pLfe, samplesUsed*4);
        }
        #endif

        BDBG_MSG(("Consuming %d samples from each channel pair (%d bytes)", samplesUsed, samplesUsed*4));
        update.uiLtOrSnglBufSzUpdt=update.uiRtBufSzUpdt=samplesUsed*4;
        BKNI_EnterCriticalSection();

        (void)BRAP_UpdateRingBufUsg_isr(handle->destination, &update, BRAP_OutputChannelPair_eLR);
        if ( handle->multichannel )
        {
            (void)BRAP_UpdateRingBufUsg_isr(handle->destination, &update, BRAP_OutputChannelPair_eLRSurround);
            (void)BRAP_UpdateRingBufUsg_isr(handle->destination, &update, BRAP_OutputChannelPair_eCentreLF);
        }
        BKNI_LeaveCriticalSection();
    }
    else if ( contigSize > 0 )
    {
        BDBG_MSG(("Internal buffer is full (size %d depth %d)", handle->bufferSize, handle->bufferDepth));
    }
}

static void NEXUS_AudioCapture_P_ConvertMono(NEXUS_AudioCaptureHandle handle, bool rightChannel)
{
    BRAP_PcmBufInfo bufInfo;
    BRAP_PcmBufSzUpdt update;
    uint32_t *pCachedAddr, *pOrigAddr;
    uint32_t data;
    unsigned int contiguous;

    BRAP_GetRingBufInfo(handle->destination, &bufInfo, BRAP_OutputChannelPair_eLR);
    if ( rightChannel )
    {
        pOrigAddr = pCachedAddr = NEXUS_AudioCapture_P_ConvertAddress(bufInfo.sRtBufInfo.pReadPtr);
        contiguous = bufInfo.sRtBufInfo.uiContiguousSize;
    }
    else
    {
        pOrigAddr = pCachedAddr = NEXUS_AudioCapture_P_ConvertAddress(bufInfo.sLtOrSnglBufInfo.pReadPtr);
        contiguous = bufInfo.sLtOrSnglBufInfo.uiContiguousSize;
    }

    /* Round down to smaller of two buffers if different */
    if ( bufInfo.sRtBufInfo.uiContiguousSize < bufInfo.sLtOrSnglBufInfo.uiContiguousSize )
    {
        bufInfo.sLtOrSnglBufInfo.uiContiguousSize = bufInfo.sRtBufInfo.uiContiguousSize;
    }

    update.uiLtOrSnglBufSzUpdt=0;

    /* Do 32-bit copies */
    while ( bufInfo.sLtOrSnglBufInfo.uiContiguousSize > 0 &&
            handle->wptr+1 != handle->rptr )
    {
        /* Get left sample */
        data = *pCachedAddr++;
        data &= 0xFFFF0000; /* Convert to 16-bit sample from 24-bit Left-aligned and place in upper word */
        bufInfo.sLtOrSnglBufInfo.uiContiguousSize-=4;
        update.uiLtOrSnglBufSzUpdt+=4;
        contiguous-=4;

        /* Get right sample */
        data |= (*pCachedAddr++)>>16;  /* convert to 16-bit sample from 24-bit Left-aligned and place in lower word */
        bufInfo.sLtOrSnglBufInfo.uiContiguousSize-=4;
        update.uiLtOrSnglBufSzUpdt+=4;
        contiguous-=4;

        /* Store sample */
        handle->pCachedBuffer[handle->wptr++] = data;
        handle->bufferDepth += 4;
        if ( handle->wptr >= (handle->bufferSize/4) )
        {
            /* Wrap of our ringbuffer */
            handle->wptr = 0;
        }
    }

    #if NEXUS_AUDIO_CAPTURE_USE_CACHED_MEMORY
    NEXUS_FlushCache(pOrigAddr, update.uiLtOrSnglBufSzUpdt);
    #endif

    update.uiRtBufSzUpdt = update.uiLtOrSnglBufSzUpdt;

    if ( update.uiLtOrSnglBufSzUpdt > 0 || update.uiRtBufSzUpdt > 0 )
    {
        (void)BRAP_UpdateRingBufUsg(handle->destination, &update, BRAP_OutputChannelPair_eLR);
        BKNI_EnterCriticalSection();
        if ( handle->multichannel )
        {
            NEXUS_AudioCapture_P_FlushChannelPair_isr(handle, BRAP_OutputChannelPair_eLRSurround);
            NEXUS_AudioCapture_P_FlushChannelPair_isr(handle, BRAP_OutputChannelPair_eCentreLF);
        }
        BKNI_LeaveCriticalSection();
    }
}

static void NEXUS_AudioCapture_P_ConvertMonoMix(NEXUS_AudioCaptureHandle handle)
{
    BRAP_PcmBufInfo bufInfo;
    BRAP_PcmBufSzUpdt update;
    uint32_t *pCachedAddrL, *pCachedAddrR, *pOrigAddrR, *pOrigAddrL;
    int32_t data1, data2, left, right;

    BRAP_GetRingBufInfo(handle->destination, &bufInfo, BRAP_OutputChannelPair_eLR);

    pOrigAddrL = pCachedAddrL = NEXUS_AudioCapture_P_ConvertAddress(bufInfo.sLtOrSnglBufInfo.pReadPtr);
    pOrigAddrR = pCachedAddrR = NEXUS_AudioCapture_P_ConvertAddress(bufInfo.sRtBufInfo.pReadPtr);

    /* Round down to smaller of two buffers if different */
    if ( bufInfo.sRtBufInfo.uiContiguousSize < bufInfo.sLtOrSnglBufInfo.uiContiguousSize )
    {
        bufInfo.sLtOrSnglBufInfo.uiContiguousSize = bufInfo.sRtBufInfo.uiContiguousSize;
    }
    BDBG_MSG(("ConvertStereo16 - received %u bytes to convert", bufInfo.sLtOrSnglBufInfo.uiContiguousSize));

    update.uiLtOrSnglBufSzUpdt=0;
    update.uiRtBufSzUpdt=0;

    /* Do 32-bit copies */
    while ( bufInfo.sLtOrSnglBufInfo.uiContiguousSize > 0 &&
            handle->wptr+1 != handle->rptr )
    {
        /* Get left sample */
        left = (int32_t)*pCachedAddrL++;
        data1 = left>>1; /* Divide by two */
        bufInfo.sLtOrSnglBufInfo.uiContiguousSize-=4;
        update.uiLtOrSnglBufSzUpdt+=4;

        /* Get right sample */
        right = (int32_t)*pCachedAddrR++;
        data1 += (right>>1);  /* Divide by two */
        bufInfo.sRtBufInfo.uiContiguousSize-=4;
        update.uiRtBufSzUpdt+=4;

        if ( bufInfo.sLtOrSnglBufInfo.uiContiguousSize > 0 )
        {
            /* Get left sample */
            left = (int32_t)*pCachedAddrL++;
            data2 = left>>1; /* Divide by two */
            bufInfo.sLtOrSnglBufInfo.uiContiguousSize-=4;
            update.uiLtOrSnglBufSzUpdt+=4;

            /* Get right sample */
            right = (int32_t)*pCachedAddrR++;
            data2 += (right>>1);  /* Divide by two */
            bufInfo.sRtBufInfo.uiContiguousSize-=4;
            update.uiRtBufSzUpdt+=4;
        }
        else
        {
            /* Duplicate last sample if we got an odd number somehow */
            data2=data1;
        }

        /* Store sample */
        handle->pCachedBuffer[handle->wptr++] = ((uint32_t)data1&0xffff0000)|((uint32_t)data2>>16);
        handle->bufferDepth += 4;

        if ( handle->wptr >= (handle->bufferSize/4) )
        {
            /* Wrap of our ringbuffer */
            handle->wptr = 0;
        }
    }
    if ( update.uiLtOrSnglBufSzUpdt > 0 )
    {
        #if NEXUS_AUDIO_CAPTURE_USE_CACHED_MEMORY
        NEXUS_FlushCache(pOrigAddrL, update.uiLtOrSnglBufSzUpdt);
        NEXUS_FlushCache(pOrigAddrR, update.uiRtBufSzUpdt);
        #endif

        (void)BRAP_UpdateRingBufUsg(handle->destination, &update, BRAP_OutputChannelPair_eLR);
        BKNI_EnterCriticalSection();
        if ( handle->multichannel )
        {
            NEXUS_AudioCapture_P_FlushChannelPair_isr(handle, BRAP_OutputChannelPair_eLRSurround);
            NEXUS_AudioCapture_P_FlushChannelPair_isr(handle, BRAP_OutputChannelPair_eCentreLF);
        }
        BKNI_LeaveCriticalSection();
    }
}

/***************************************************************************
Summary:
    Get Destination Details for an audio capture object
 ***************************************************************************/
NEXUS_Error NEXUS_AudioCapture_P_GetDestinationDetails(
    NEXUS_AudioCaptureHandle handle,
    NEXUS_AudioInputFormat format,
    BRAP_ChannelHandle primaryChannel,
    BRAP_DstDetails *pDestInfo)
{
    int i;

    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioCapture);
    BDBG_ASSERT(NULL != primaryChannel);
    BDBG_ASSERT(NULL != pDestInfo);

    /* If the channel is still set, something has gone wrong. */
    BDBG_ASSERT(NULL == handle->channel);

    handle->channel = primaryChannel;
    handle->compressed = (format == NEXUS_AudioInputFormat_eCompressed)?true:false;
    handle->multichannel = (format == NEXUS_AudioInputFormat_ePcm5_1)?true:false;

    if ( handle->multichannel && g_NEXUS_audioModuleData.moduleSettings.audioCaptureChannels < 6 )
    {
        BDBG_ERR(("You have not allocated enough audio capture buffers for multichannel data."));
        BDBG_ERR(("Please set NEXUS_AudioModuleSettings.audioCaptureChannels to a value >= 6 for multichannel content."));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* Populate destination info */
    BKNI_Memset(&pDestInfo->uDstDetails.sRBufDetails, 0, sizeof(pDestInfo->uDstDetails.sRBufDetails));
    for(i=0; i<BRAP_OutputChannelPair_eMax; i++)
    {
        pDestInfo->uDstDetails.sRBufDetails.eBufDataMode[i] = BRAP_BufDataMode_eMaxNum;
    }
    pDestInfo->uDstDetails.sRBufDetails.bCompress = handle->compressed;
    if ( handle->multichannel )
    {
#if BRAP_VER >= 4
        pDestInfo->eAudioMode = BRAP_OutputMode_e3_2;
        pDestInfo->bLfeOn = true;
#else
        pDestInfo->uDstDetails.sRBufDetails.eAudioMode = BRAP_OutputMode_e3_2;
        pDestInfo->uDstDetails.sRBufDetails.bLfeOn = true;
#endif
    }
    else
    {
#if BRAP_VER >= 4
        pDestInfo->eAudioMode = BRAP_OutputMode_e2_0;
        pDestInfo->bLfeOn = false;
#else
        pDestInfo->uDstDetails.sRBufDetails.eAudioMode = BRAP_OutputMode_e2_0;
        pDestInfo->uDstDetails.sRBufDetails.bLfeOn = false;
#endif
    }

    pDestInfo->uDstDetails.sRBufDetails.eBufDataMode[BRAP_OutputChannelPair_eLR] =
        handle->compressed?BRAP_BufDataMode_eStereoInterleaved:BRAP_BufDataMode_eStereoNoninterleaved;
    pDestInfo->uDstDetails.sRBufDetails.eSampleRate = BAVC_AudioSamplingRate_e48k;
    pDestInfo->uDstDetails.sRBufDetails.sLtOrSnglRbufSettings[BRAP_OutputChannelPair_eLR].uiSize = handle->openSettings.fifoSize/g_NEXUS_audioModuleData.moduleSettings.audioCaptureChannels;
    if ( handle->openSettings.fifoSize && handle->openSettings.threshold )
    {
        pDestInfo->uDstDetails.sRBufDetails.sLtOrSnglRbufSettings[BRAP_OutputChannelPair_eLR].uiWaterMark =
            (handle->openSettings.threshold * 100) / pDestInfo->uDstDetails.sRBufDetails.sLtOrSnglRbufSettings[BRAP_OutputChannelPair_eLR].uiSize;
    }
    else
    {
        pDestInfo->uDstDetails.sRBufDetails.sLtOrSnglRbufSettings[BRAP_OutputChannelPair_eLR].uiWaterMark = 10; /* Default from raptest */
    }
    if ( !handle->compressed )
    {
        /* Use same for both buffers */
        pDestInfo->uDstDetails.sRBufDetails.sRtRbufSettings[BRAP_OutputChannelPair_eLR] =
            pDestInfo->uDstDetails.sRBufDetails.sLtOrSnglRbufSettings[BRAP_OutputChannelPair_eLR];
        if ( handle->multichannel )
        {
            pDestInfo->uDstDetails.sRBufDetails.eBufDataMode[BRAP_OutputChannelPair_eLRSurround] =
                handle->compressed?BRAP_BufDataMode_eStereoInterleaved:BRAP_BufDataMode_eStereoNoninterleaved;        
            
            pDestInfo->uDstDetails.sRBufDetails.eBufDataMode[BRAP_OutputChannelPair_eCentreLF] =
                handle->compressed?BRAP_BufDataMode_eStereoInterleaved:BRAP_BufDataMode_eStereoNoninterleaved;        

            pDestInfo->uDstDetails.sRBufDetails.sLtOrSnglRbufSettings[BRAP_OutputChannelPair_eLRSurround].uiSize = handle->openSettings.fifoSize/g_NEXUS_audioModuleData.moduleSettings.audioCaptureChannels;
            if ( handle->openSettings.fifoSize && handle->openSettings.threshold )
            {
                pDestInfo->uDstDetails.sRBufDetails.sLtOrSnglRbufSettings[BRAP_OutputChannelPair_eLRSurround].uiWaterMark =
                    (handle->openSettings.threshold * 100) / pDestInfo->uDstDetails.sRBufDetails.sLtOrSnglRbufSettings[BRAP_OutputChannelPair_eLRSurround].uiSize;
            }
            else
            {
                pDestInfo->uDstDetails.sRBufDetails.sLtOrSnglRbufSettings[BRAP_OutputChannelPair_eLRSurround].uiWaterMark = 10; /* Default from raptest */
            }

            pDestInfo->uDstDetails.sRBufDetails.sLtOrSnglRbufSettings[BRAP_OutputChannelPair_eCentreLF].uiSize = handle->openSettings.fifoSize/g_NEXUS_audioModuleData.moduleSettings.audioCaptureChannels;
            if ( handle->openSettings.fifoSize && handle->openSettings.threshold )
            {
                pDestInfo->uDstDetails.sRBufDetails.sLtOrSnglRbufSettings[BRAP_OutputChannelPair_eCentreLF].uiWaterMark =
                    (handle->openSettings.threshold * 100) / pDestInfo->uDstDetails.sRBufDetails.sLtOrSnglRbufSettings[BRAP_OutputChannelPair_eCentreLF].uiSize;
            }
            else
            {
                pDestInfo->uDstDetails.sRBufDetails.sLtOrSnglRbufSettings[BRAP_OutputChannelPair_eCentreLF].uiWaterMark = 10; /* Default from raptest */
            }            
            
            pDestInfo->uDstDetails.sRBufDetails.sLtOrSnglRbufSettings[BRAP_OutputChannelPair_eLRSurround] =
                pDestInfo->uDstDetails.sRBufDetails.sLtOrSnglRbufSettings[BRAP_OutputChannelPair_eLR];
            pDestInfo->uDstDetails.sRBufDetails.sRtRbufSettings[BRAP_OutputChannelPair_eLRSurround] = 
                pDestInfo->uDstDetails.sRBufDetails.sRtRbufSettings[BRAP_OutputChannelPair_eLR];

            pDestInfo->uDstDetails.sRBufDetails.sLtOrSnglRbufSettings[BRAP_OutputChannelPair_eCentreLF] =
                pDestInfo->uDstDetails.sRBufDetails.sLtOrSnglRbufSettings[BRAP_OutputChannelPair_eLR];
            pDestInfo->uDstDetails.sRBufDetails.sRtRbufSettings[BRAP_OutputChannelPair_eCentreLF] = 
                pDestInfo->uDstDetails.sRBufDetails.sRtRbufSettings[BRAP_OutputChannelPair_eLR];
        }
    }

    return BERR_SUCCESS;
}

/***************************************************************************
Summary:
    Unlink from a parent channel
 ***************************************************************************/
void NEXUS_AudioCapture_P_Disconnect(
    NEXUS_AudioCaptureHandle handle
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioCapture);
    if ( NULL != handle->channel )
    {
        if ( handle->running )
        {
            BDBG_WRN(("Implicitly stopping capture channel %p on disconnection.", (void *)handle));
            NEXUS_AudioCapture_Stop(handle);
        }
        /* Unlink from the channel's interrupt */
        handle->channel = NULL;
    }
}

static void NEXUS_AudioCapture_P_Interrupt_isr(void *pParam, int param, void *pData)
{
    NEXUS_AudioCaptureHandle handle;

    handle = pParam;
    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioCapture);
    BSTD_UNUSED(param);
    BSTD_UNUSED(pData);

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

static void NEXUS_AudioCapture_P_SampleRate_isr(void *pParam, int param, void *pData)
{
    const BRAP_DSPCHN_DestinationSampleRateChangeInfo *pInfo = pData;
    NEXUS_AudioCaptureHandle handle;

    handle = pParam;
    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioCapture);
    BSTD_UNUSED(param);

    handle->sampleRate = NEXUS_AudioModule_P_Avc2SampleRate(pInfo->eSamplingRate);
    BDBG_MSG(("Sample Rate Interrupt (%u)", handle->sampleRate));
    NEXUS_IsrCallback_Fire_isr(handle->sampleRateCallback);
}

static void NEXUS_AudioCapture_P_FlushChannelPair_isr(NEXUS_AudioCaptureHandle handle, BRAP_OutputChannelPair chPair)
{
    BRAP_PcmBufInfo bufInfo;
    BRAP_PcmBufSzUpdt update;
    int numTries=4;

    BRAP_GetRingBufInfo_isr(handle->destination, &bufInfo, chPair);

    while ( bufInfo.sLtOrSnglBufInfo.uiContiguousSize > 0 &&
            (NULL == bufInfo.sRtBufInfo.pBasePtr ||
             bufInfo.sRtBufInfo.uiContiguousSize > 0) &&
            (numTries-- > 0) )
    {
        update.uiLtOrSnglBufSzUpdt = bufInfo.sLtOrSnglBufInfo.uiContiguousSize;
        update.uiRtBufSzUpdt = bufInfo.sRtBufInfo.uiContiguousSize;
        BDBG_MSG(("Flushing %u,%u bytes from device buffer", update.uiLtOrSnglBufSzUpdt,update.uiRtBufSzUpdt));
        (void)BRAP_UpdateRingBufUsg_isr(handle->destination, &update, chPair);
        BRAP_GetRingBufInfo_isr(handle->destination, &bufInfo, chPair);
    }
}

void NEXUS_AudioCapture_P_FlushDeviceBuffer_isr(NEXUS_AudioCaptureHandle handle)
{
    BDBG_MSG(("Flush Device Buffer"));

    NEXUS_AudioCapture_P_FlushChannelPair_isr(handle, BRAP_OutputChannelPair_eLR);
    if ( handle->multichannel )
    {
        NEXUS_AudioCapture_P_FlushChannelPair_isr(handle, BRAP_OutputChannelPair_eLRSurround);
        NEXUS_AudioCapture_P_FlushChannelPair_isr(handle, BRAP_OutputChannelPair_eCentreLF);
    }
    BRAP_RingBufferReadDone_isr(handle->destination);
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
    return BERR_SUCCESS;
}

static uint32_t *NEXUS_AudioCapture_P_ConvertAddress(void *pAddress)
{
    #if NEXUS_AUDIO_CAPTURE_USE_CACHED_MEMORY
    BERR_Code rc = BERR_SUCCESS;
    rc = BMEM_Heap_ConvertAddressToCached(g_pCoreHandles->heap[0], pAddress, &pAddress);
    if(rc != BERR_SUCCESS){BERR_TRACE(rc) return NULL;}
    #endif
    return pAddress;
}

/***************************************************************************
Summary:
    Set the destination to enable capture start/stop.
 ***************************************************************************/
void NEXUS_AudioCapture_P_SetDestination(
    NEXUS_AudioCaptureHandle handle,
    BRAP_DestinationHandle destination
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioCapture);

    if ( destination )
    {
        BRAP_InstallDestinationLevelAppInterruptCb(destination,
                                                   BRAP_DestinationInterrupt_eFmmRbufFullMark,
                                                   NEXUS_AudioCapture_P_Interrupt_isr, handle, 0);
        BRAP_InstallDestinationLevelAppInterruptCb(destination,
                                                   BRAP_DestinationInterrupt_eSampleRateChange,
                                                   NEXUS_AudioCapture_P_SampleRate_isr, handle, 0);
    }
    else
    {
        if ( handle->running )
        {
            BDBG_WRN(("Implicitly stopping capture channel %p on disconnection.", (void *)handle));
            NEXUS_AudioCapture_Stop(handle);
        }
        if ( handle->destination )
        {
            BRAP_RemoveDestinationLevelAppInterruptCb(handle->destination, BRAP_DestinationInterrupt_eFmmRbufFullMark);
            BRAP_RemoveDestinationLevelAppInterruptCb(handle->destination, BRAP_DestinationInterrupt_eSampleRateChange);
        }
    }

    handle->destination = destination;
}
#else /* #if NEXUS_NUM_AUDIO_CAPTURES */
typedef struct NEXUS_AudioCapture
{
    NEXUS_OBJECT(NEXUS_AudioCapture);
} NEXUS_AudioCapture;


/***************************************************************************
Summary:
Get default settings for opening an audio capture channel
***************************************************************************/
void NEXUS_AudioCapture_GetDefaultOpenSettings(
    NEXUS_AudioCaptureOpenSettings *pSettings      /* [out] default settings */
    )
{
    BSTD_UNUSED(pSettings);
    BDBG_WRN(("Audio capture not enabled on this chipset"));
    BERR_TRACE(BERR_NOT_SUPPORTED);
}

/***************************************************************************
Summary:
Open an audio capture channel

Description:
The captured data will be formatted with the settings provided in
NEXUS_AudioCaptureSettings.  The sample rate of the data is determined by
the sample rate of the input data.  For example, if the decoder is outputting
48kHz, the data will be the same sample rate.  If the decoder is not active,
the output will be 48kHz.  Data will automatically start flowing once the
source device starts and stop when the source device stops.
***************************************************************************/
NEXUS_AudioCaptureHandle NEXUS_AudioCapture_Open(     /* attr{destructor=NEXUS_AudioCapture_Close}  */
    unsigned index,
    const NEXUS_AudioCaptureOpenSettings *pSettings    /* Pass NULL for default settings */
    )
{
    BSTD_UNUSED(pSettings);
    BSTD_UNUSED(index);
    BDBG_WRN(("Audio capture not enabled on this chipset"));
    BERR_TRACE(BERR_NOT_SUPPORTED);
    return NULL;
}

/***************************************************************************
Summary:
Close an audio capture channel
***************************************************************************/
static void NEXUS_AudioCapture_P_Finalizer(
    NEXUS_AudioCaptureHandle handle
    )
{
    BSTD_UNUSED(handle);
    BDBG_WRN(("Audio capture not enabled on this chipset"));
    BERR_TRACE(BERR_NOT_SUPPORTED);
}

/***************************************************************************
Summary:
Get Audio Capture Channel Settings
***************************************************************************/
void NEXUS_AudioCapture_GetSettings(
    NEXUS_AudioCaptureHandle handle,
    NEXUS_AudioCaptureSettings *pSettings /* [out] */
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pSettings);
    BDBG_WRN(("Audio capture not enabled on this chipset"));
    BERR_TRACE(BERR_NOT_SUPPORTED);
}

/***************************************************************************
Summary:
Set Audio Capture Channel Settings
***************************************************************************/
NEXUS_Error NEXUS_AudioCapture_SetSettings(
    NEXUS_AudioCaptureHandle handle,
    const NEXUS_AudioCaptureSettings *pSettings
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pSettings);
    BDBG_WRN(("Audio capture not enabled on this chipset"));
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

/***************************************************************************
Summary:
Get current setting of the audio capture channel
***************************************************************************/
void NEXUS_AudioCapture_GetDefaultStartSettings(
    NEXUS_AudioCaptureStartSettings *pSettings  /* [out] default settings */
    )
{
    BSTD_UNUSED(pSettings);
    BDBG_WRN(("Audio capture not enabled on this chipset"));
    BERR_TRACE(BERR_NOT_SUPPORTED);
}

/***************************************************************************
Summary:
Start capturing data.

Decription:
An input must be connected prior to starting.

See Also:
NEXUS_AudioCapture_Stop
NEXUS_AudioCapture_GetConnector
***************************************************************************/
NEXUS_Error NEXUS_AudioCapture_Start(
    NEXUS_AudioCaptureHandle handle,
    const NEXUS_AudioCaptureStartSettings *pSettings
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pSettings);
    BDBG_WRN(("Audio capture not enabled on this chipset"));
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

/***************************************************************************
Summary:
Stop capturing data.
***************************************************************************/
void NEXUS_AudioCapture_Stop(
    NEXUS_AudioCaptureHandle handle
    )
{
    BSTD_UNUSED(handle);
    BDBG_WRN(("Audio capture not enabled on this chipset"));
    BERR_TRACE(BERR_NOT_SUPPORTED);
}

/***************************************************************************
Summary:
Get a pointer and size for the next location in the buffer that contains data

Description:
NEXUS_AudioCapture_GetBuffer is non-destructive. You can safely call it
multiple times.
***************************************************************************/
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
    BDBG_WRN(("Audio capture not enabled on this chipset"));
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

/***************************************************************************
Summary:
Notify AudioCapture how much data removed from the buffer.

Description:
You can only call NEXUS_AudioCapture_ReadComplete once after a
NEXUS_AudioCapture_GetBuffer call.  After calling it, you must call
NEXUS_AudioCapture_GetBuffer before reading more data.
***************************************************************************/
NEXUS_Error NEXUS_AudioCapture_ReadComplete(
    NEXUS_AudioCaptureHandle handle,
    size_t amountWritten            /* The number of bytes read from the buffer */
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(amountWritten);
    BDBG_WRN(("Audio capture not enabled on this chipset"));
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

/***************************************************************************
Summary:
Get an audio connector to attach this output to a source.

Description:
Capture may be connected to a stereo source device or a compressed source
device, depending on the data format desired.
**************************************************************************/
NEXUS_AudioOutputHandle NEXUS_AudioCapture_GetConnector( /* attr{shutdown=NEXUS_AudioOutput_Shutdown} */
    NEXUS_AudioCaptureHandle handle
    )
{
    BSTD_UNUSED(handle);
    BDBG_WRN(("Audio capture not enabled on this chipset"));
    BERR_TRACE(BERR_NOT_SUPPORTED);
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
#endif /* NEXUS_NUM_AUDIO_CAPTURES */

static void NEXUS_AudioCapture_P_Release(NEXUS_AudioCaptureHandle handle)
{
#if NEXUS_NUM_AUDIO_CAPTURES
    if (!IS_ALIAS(handle)) {
        NEXUS_OBJECT_UNREGISTER(NEXUS_AudioOutput, &handle->connector, Close);
    }
#else
    BSTD_UNUSED(handle);
#endif
    return;
}

NEXUS_OBJECT_CLASS_MAKE_WITH_RELEASE(NEXUS_AudioCapture, NEXUS_AudioCapture_Close);
