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
    int bufferSize, rptr, wptr;
    NEXUS_AudioOutputObject connector;
    NEXUS_IsrCallbackHandle callback;
    BRAP_ChannelHandle channel;
	BRAP_CAP_ChannelSettings rap_cap_settings;
    BRAP_CAP_AudioParams rap_cap_audio_params;
    bool running;
    bool compressed;
} NEXUS_AudioCapture;

static void NEXUS_AudioCapture_P_ConvertStereo16(NEXUS_AudioCaptureHandle handle);
static void NEXUS_AudioCapture_P_Interrupt_isr(void *pParam, int param, void *pData);
static void NEXUS_AudioCapture_P_FlushDeviceBuffer_isr(NEXUS_AudioCaptureHandle handle);
static uint32_t *NEXUS_AudioCapture_P_ConvertAddress(void *pAddress);

/* Set this to one to use cached memory */
#define NEXUS_AUDIO_CAPTURE_USE_CACHED_MEMORY 0

#if NEXUS_NUM_AUDIO_CAPTURES

/***************************************************************************
Summary:
Get default settings for opening an audio capture channel
***************************************************************************/
void NEXUS_AudioCapture_GetDefaultOpenSettings(
    NEXUS_AudioCaptureOpenSettings *pSettings      /* [out] default settings */
    )
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
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
    NEXUS_AudioCaptureHandle handle;
    void *pBuffer;
    NEXUS_Error errCode;
    int watermark;

    BSTD_UNUSED(index);

    if ( index > NEXUS_NUM_AUDIO_CAPTURES )
    {
        BDBG_ERR(("Index out of range.  You must open an audio capture index < NEXUS_NUM_AUDIO_CAPTURES"));
        (void)BERR_TRACE(BERR_NOT_SUPPORTED);
        return NULL;
    }

    handle = BKNI_Malloc(sizeof(NEXUS_AudioCapture));
    if ( NULL == handle )
    {
        (void)BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        return NULL;
    }

    NEXUS_OBJECT_INIT(NEXUS_AudioCapture, handle);

    if ( NULL != pSettings )
    {
        handle->openSettings = *pSettings;
    }

    handle->callback = NEXUS_IsrCallback_Create(handle, NULL);
    if ( NULL == handle->callback )
    {
        goto err_callback;
    }

	errCode = BRAP_CAP_GetChannelDefaultSettings(g_NEXUS_audioModuleData.hRap, &handle->rap_cap_settings);
	if (errCode) {BERR_TRACE(errCode); goto error;}

    /* Setup internal buffer and thresholds */
    if ( 0 == handle->openSettings.fifoSize )
    {
        handle->bufferSize = 409600;        /* allocate larger ring buffer to avoid data loss */
    }
    else
    {
        /* round up to 256 byte boundary */
        handle->openSettings.fifoSize = (handle->openSettings.fifoSize + 255)&~255;
        handle->bufferSize = handle->openSettings.fifoSize;
    }
    handle->rap_cap_settings.sLtOrSnglRbufSettings.uiSize = handle->bufferSize;	
    if ( 0 == handle->openSettings.threshold )
    {
        watermark = handle->bufferSize/4;
    }
    else
    {
        watermark = handle->openSettings.threshold;
    }

	handle->rap_cap_settings.eCapMode = BRAP_CaptureMode_eCaptureOnly;
	handle->rap_cap_settings.eOutputPort = BRAP_OutputPort_eMax;
	handle->rap_cap_settings.eMaxBufDataMode = BRAP_BufDataMode_eStereoInterleaved;
	handle->rap_cap_settings.sLtOrSnglRbufSettings.uiWaterMark = (watermark*100)/handle->bufferSize;
	handle->rap_cap_settings.sRtRbufSettings.uiWaterMark = (watermark*100)/handle->bufferSize;;    
	errCode = BRAP_CAP_OpenChannel(g_NEXUS_audioModuleData.hRap, &handle->channel, index, &handle->rap_cap_settings);
	if (errCode) {BERR_TRACE(errCode); goto error;}

    /* Now that the channel has been opened, get the default settings */
    /* Get default parameters for a RAP PCM capture channel  */
    errCode = BRAP_CAP_GetDefaultAudioParams(handle->channel, &handle->rap_cap_audio_params);
	if (errCode) {BERR_TRACE(errCode); goto error;}
    BDBG_MSG(("RAP PCM capture channel default params received"));

    handle->rap_cap_audio_params.sDstChParams.sInputI2sParams.bAlignedToLRClk = false;
    handle->rap_cap_audio_params.sDstChParams.eCapPort = BRAP_CapInputPort_eInput0;
    handle->rap_cap_audio_params.eCapInputType = BRAP_CapInputType_eIntI2s0;
    /* default to 48kHz, stereo interleaved */
    handle->rap_cap_audio_params.eInputSamplingRate = BAVC_AudioSamplingRate_e48k;
    handle->rap_cap_audio_params.eBufDataMode = BRAP_BufDataMode_eStereoInterleaved;
    handle->rap_cap_audio_params.eInputBitsPerSample = 16; /* Only support 16 bits per sample for now. */
    /* Alloc internal buffer */
    handle->pBuffer = BMEM_Heap_Alloc(g_pCoreHandles->heap[0].mem, handle->bufferSize);
    if ( NULL == handle->pBuffer )
    {
        (void)BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
        goto error;
    }
    /* MCW working HERE is this necessary ? */
    errCode = BMEM_Heap_ConvertAddressToCached(g_pCoreHandles->heap[0].mem, handle->pBuffer, &pBuffer);
    if (errCode)
    {
        (void)BERR_TRACE(errCode); /* Continue, just use uncached address. */
    }
    handle->pCachedBuffer = pBuffer;

    handle->connector.objectType = NEXUS_AudioOutputType_eCapture;
    handle->connector.pObjectHandle = handle;
    handle->connector.port = BRAP_OutputPort_eI2s0;  /* Capture using I2S0 port */

    /* Success */
    return handle;

error:
    NEXUS_IsrCallback_Destroy(handle->callback);
err_callback:
    NEXUS_OBJECT_DESTROY(NEXUS_AudioCapture, handle);
    BKNI_Free(handle);
    (void)BERR_TRACE(BERR_OS_ERROR);
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

	if (handle->channel) {
		BDBG_MSG(("%s()",__FUNCTION__));
		BRAP_CAP_CloseChannel(handle->channel);
		handle->channel = NULL;
	}

    NEXUS_AudioOutput_Shutdown(&handle->connector);
    BMEM_Heap_Free(g_pCoreHandles->heap[0].mem, handle->pBuffer);
    NEXUS_IsrCallback_Destroy(handle->callback);
    NEXUS_OBJECT_DESTROY(NEXUS_AudioCapture, handle);
    BKNI_Free(handle);
}

NEXUS_OBJECT_CLASS_MAKE(NEXUS_AudioCapture, NEXUS_AudioCapture_Close);

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

    handle->rap_cap_audio_params.eCapInputType = BRAP_CapInputType_eIntI2s0;

    /* Setup callback */
    NEXUS_IsrCallback_Set(handle->callback, pSettings?&pSettings->dataCallback:NULL);

    if ( handle->channel )
    {
        #if NEXUS_AUDIO_CAPTURE_USE_CACHED_MEMORY
        BRAP_PcmBufInfo bufInfo;
        void *pCachedAddr;
        #endif

        #if NEXUS_AUDIO_CAPTURE_USE_CACHED_MEMORY
        /* Kick any stale data out of the cache */
        BRAP_CAP_GetBufInfo(handle->channel,&bufInfo);
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
        handle->wptr = handle->rptr = 0;
        BKNI_LeaveCriticalSection();
    }
    else
    {
        BDBG_MSG(("Starting - channel/destination not yet valid"));

        /* flush anything pending in our buffer and in raptor */
        BKNI_EnterCriticalSection();
        handle->running = true;
        handle->wptr = handle->rptr = 0;
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

    BDBG_MSG(("Stopping"));

    /* flush anything pending in our buffer and in raptor */
    BKNI_EnterCriticalSection();
    handle->running = false;
    BKNI_LeaveCriticalSection();
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
    BDBG_ASSERT(NULL != ppBuffer);
    BDBG_ASSERT(NULL != pSize);

    /* Defaults for error returns */
    *ppBuffer = NULL;
    *pSize = 0;

    BDBG_MSG(("GetBuffer - compressed %d, format %d running %d channel %p",
              handle->compressed, handle->settings.format,
              handle->running, (void *)handle->channel));

    if ( !handle->running || NULL == handle->channel )
    {
        BDBG_ERR(("running %d channel %p", handle->running, (void *)handle->channel));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    BDBG_MSG(("Before Conversion rptr %d wptr %d", handle->rptr, handle->wptr));

    if ( handle->compressed )
    {
        /* Not currently supported on this platform */
        BDBG_ERR(("Invalid capture format"));
        BDBG_ASSERT(false);
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    else
    {
        switch ( handle->settings.format )
        {
        case NEXUS_AudioCaptureFormat_e16BitStereo:
            NEXUS_AudioCapture_P_ConvertStereo16(handle);
            break;
        case NEXUS_AudioCaptureFormat_e24BitStereo:
        case NEXUS_AudioCaptureFormat_e16BitMonoLeft:
        case NEXUS_AudioCaptureFormat_e16BitMonoRight:
        case NEXUS_AudioCaptureFormat_e16BitMono:
        default:
            /* Format not currently supported on this platform */
            BDBG_ERR(("Invalid capture format"));
            BDBG_ASSERT(false);
            return BERR_TRACE(BERR_INVALID_PARAMETER);
        }
    }

    *ppBuffer = &handle->pCachedBuffer[handle->rptr];
    if ( handle->rptr == handle->wptr )
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

    BRAP_CAP_BufferReadDone(handle->channel);

    return BERR_SUCCESS;
}

/***************************************************************************
Summary:
Notify AudioCapture how much data removed from the buffer.

Description:
You can only call NEXUS_AudioCapture_WriteComplete once after a 
NEXUS_AudioCapture_GetBuffer call.  After calling it, you must call 
NEXUS_AudioCapture_GetBuffer before reading more data.
***************************************************************************/
NEXUS_Error NEXUS_AudioCapture_WriteComplete(
    NEXUS_AudioCaptureHandle handle,
    size_t amountWritten            /* The number of bytes read from the buffer */
    )
{
    int newRptr;

    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioCapture);

    if ( !handle->running || NULL == handle )
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
    return &handle->connector;
}

static void NEXUS_AudioCapture_P_ConvertStereo16(NEXUS_AudioCaptureHandle handle)
{
    BRAP_PcmBufInfo bufInfo;
    uint32_t *pCachedAddr, *pOrigAddr;
    BRAP_PcmBufSzUpdt update;
    BRAP_CAP_GetBufInfo(handle->channel,&bufInfo);
    pOrigAddr = pCachedAddr = NEXUS_AudioCapture_P_ConvertAddress(bufInfo.sLtOrSnglBufInfo.pReadPtr);
    update.uiLtOrSnglBufSzUpdt = 0;
    update.uiRtBufSzUpdt = 0;
    /* Stereo 16 is currently native capture format, just copy. */
    /* Do 32-bit copies */
    while ( bufInfo.sLtOrSnglBufInfo.uiContiguousSize > 0 &&
            handle->wptr+1 != handle->rptr )
    {
        handle->pCachedBuffer[handle->wptr++] = *pCachedAddr++;
        bufInfo.sLtOrSnglBufInfo.uiContiguousSize-=4;
        update.uiLtOrSnglBufSzUpdt += 4;
        if ( handle->wptr >= (handle->bufferSize/4) )
        {
            /* Wrap of our ringbuffer */
            handle->wptr = 0;
        }
    }
    if ( update.uiLtOrSnglBufSzUpdt > 0 )
    {
        #if NEXUS_AUDIO_CAPTURE_USE_CACHED_MEMORY
        NEXUS_FlushCache(pOrigAddr, update.uiLtOrSnglBufSzUpdt);
        #endif
        (void)BRAP_CAP_UpdateBufUsg(handle->channel, &update);
    }
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
        NEXUS_IsrCallback_Fire_isr(handle->callback);
    }
    else
    {
        NEXUS_AudioCapture_P_FlushDeviceBuffer_isr(handle);
    }
}

void NEXUS_AudioCapture_P_FlushDeviceBuffer_isr(NEXUS_AudioCaptureHandle handle)
{
    BRAP_PcmBufInfo bufInfo;
    BRAP_PcmBufSzUpdt update;
    int numTries=4;

    BDBG_MSG(("Flush Device Buffer"));

    BRAP_CAP_GetBufInfo_isr(handle->channel, &bufInfo);

    while ( bufInfo.sLtOrSnglBufInfo.uiContiguousSize > 0 &&
            (NULL == bufInfo.sRtBufInfo.pBasePtr ||
             bufInfo.sRtBufInfo.uiContiguousSize > 0) &&
            (numTries-- > 0) )
    {
        update.uiLtOrSnglBufSzUpdt = bufInfo.sLtOrSnglBufInfo.uiContiguousSize;
        update.uiRtBufSzUpdt = bufInfo.sRtBufInfo.uiContiguousSize;
        BDBG_MSG(("Flushing %u,%u bytes from device buffer", update.uiLtOrSnglBufSzUpdt,update.uiRtBufSzUpdt));
        (void)BRAP_CAP_UpdateBufUsg_isr(handle->channel, &update);
        BRAP_CAP_GetBufInfo_isr(handle->channel, &bufInfo);
    }
    BRAP_CAP_BufferReadDone_isr(handle->channel);
}

static uint32_t *NEXUS_AudioCapture_P_ConvertAddress(void *pAddress)
{
    #if NEXUS_AUDIO_CAPTURE_USE_CACHED_MEMORY
    NEXUS_Error errCode;

    errCode = BMEM_Heap_ConvertAddressToCached(g_pCoreHandles->heap[0], pAddress, &pAddress);
    if (errCode)
    {
        (void)BERR_TRACE(errCode); /* Continue, just using uncached address. */
    }
    #endif
    return pAddress;
}

/***************************************************************************
Summary:
    Connect the capture to enable capture start/stop.
 ***************************************************************************/
void NEXUS_AudioCapture_P_SetConnection(
    NEXUS_AudioCaptureHandle handle,
    bool connect
    )
{
    NEXUS_Error errCode;
    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioCapture);
    BDBG_ASSERT(handle->channel);

    if ( connect )
    {
        BRAP_CAP_AudioParams *pAudioParams = &handle->rap_cap_audio_params;
        /* Start RAP CAP operation */
        errCode = BRAP_CAP_Start(handle->channel, pAudioParams);
        if(errCode != BERR_SUCCESS)
        {
            BDBG_ERR(("BRAP_CAP_Start() failed"));
            (void)BERR_TRACE(errCode);
        }
        else
        {
            BDBG_MSG(("RAP PCM capture channel started"));
        }


        BDBG_MSG(("%s() installing NEXUS_AudioCapture_P_Interrupt_isr() for BRAP_Interrupt_eFmmDstRbufFullMark isr.",__FUNCTION__));
        errCode = BRAP_InstallAppInterruptCb(handle->channel,
                                   BRAP_Interrupt_eFmmDstRbufFullMark,
                                   NEXUS_AudioCapture_P_Interrupt_isr, handle, 0);
        if(errCode != BERR_SUCCESS)
        {
            BDBG_ERR(("BRAP_InstallAppInterruptCb(%p,BRAP_Interrupt_eFmmDstRbufFullMark,%p,%p,0) failed",
                      (void *)handle->channel,(void *)(unsigned long)NEXUS_AudioCapture_P_Interrupt_isr, (void *)handle));
            (void)BERR_TRACE(errCode);
        }
        else
        {
            BDBG_MSG(("BRAP_InstallAppInterruptCb(%p,BRAP_Interrupt_eFmmDstRbufFullMark,%p,%p,0) OK",
                      (void *)handle->channel,(void *)(unsigned long)NEXUS_AudioCapture_P_Interrupt_isr,(void *)handle));
            BDBG_MSG(("RAP PCM capture channel started"));
        }
    }
    else
    {
        if ( handle->running )
        {
            BDBG_WRN(("Implicitly stopping capture channel %p on disconnection.", (void *)handle));
            NEXUS_AudioCapture_Stop(handle);
        }
        errCode = BRAP_CAP_Stop(handle->channel);
        if (errCode) 
        {
            (void)BERR_TRACE(errCode);
        }

        BDBG_MSG(("%s() removing NEXUS_AudioCapture_P_Interrupt_isr() for BRAP_Interrupt_eFmmDstRbufFullMark isr.",__FUNCTION__));
        BRAP_RemoveAppInterruptCb(handle->channel, BRAP_Interrupt_eFmmDstRbufFullMark);
    }
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
#else

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
    NEXUS_Error errCode;
    BSTD_UNUSED(index);
    BSTD_UNUSED(pSettings);
    errCode = BERR_TRACE(BERR_NOT_SUPPORTED);
    return NULL;
}

void NEXUS_AudioCapture_Close(
    NEXUS_AudioCaptureHandle handle
    )
{
    BSTD_UNUSED(handle);
    return;
}

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
    return;
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

