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
*   API name: AudioPlayback
*    Specific APIs related to PCM audio playback.  This supports playback
*    of data from memory.  It can be routed either to a mixer or directly
*    to output devices.
*
* Revision History:
*
* $brcm_Log: $
*
***************************************************************************/
#include "nexus_audio_module.h"

BDBG_MODULE(nexus_audio_playback);

#if NEXUS_NUM_AUDIO_PLAYBACKS
/***************************************************************************
Summary:
Handle for audio playback
***************************************************************************/
typedef struct NEXUS_AudioPlayback
{
    NEXUS_OBJECT(NEXUS_AudioPlayback);
    NEXUS_AudioInputObject connector;
    BAPE_PlaybackHandle channel;
    NEXUS_TaskCallbackHandle appCallback;
    NEXUS_AudioPlaybackOpenSettings openSettings;
    NEXUS_AudioPlaybackStartSettings startSettings;
    NEXUS_AudioPlaybackSettings settings;
    BKNI_EventHandle event;
    NEXUS_EventCallbackHandle eventCallback;
    void *pLastAddress;
    size_t bytesPlayed;
    bool started;
    bool opened;
} NEXUS_AudioPlayback;

static NEXUS_AudioPlayback g_playbacks[NEXUS_NUM_AUDIO_PLAYBACKS];
static void NEXUS_AudioPlayback_P_DataEvent(void *pParam);
static void NEXUS_AudioPlayback_P_BufferFree_isr(void *pParam1, int param2);

/***************************************************************************
Summary:
Get default settings for an audio playback channel
***************************************************************************/
void NEXUS_AudioPlayback_GetDefaultOpenSettings(
    NEXUS_AudioPlaybackOpenSettings *pSettings      /* [out] default settings */
    )
{
    BAPE_PlaybackOpenSettings openSettings;
    BDBG_ASSERT(NULL != pSettings);
    BAPE_Playback_GetDefaultOpenSettings(&openSettings);
    pSettings->fifoSize = openSettings.fifoSize;
    pSettings->threshold = openSettings.watermarkThreshold;
}

/***************************************************************************
Summary:
Open an audio playback channel
***************************************************************************/
NEXUS_AudioPlaybackHandle NEXUS_AudioPlayback_Open(     /* attr{destructor=NEXUS_AudioPlayback_Close}  */
    unsigned index,
    const NEXUS_AudioPlaybackOpenSettings *pSettings    /* Pass NULL for default settings */
    )
{
    BERR_Code errCode;
    NEXUS_AudioPlayback *pChannel;
    NEXUS_AudioPlaybackOpenSettings defaultSettings;
    BAPE_PlaybackOpenSettings openSettings;
    BAPE_PlaybackInterruptHandlers interrupts;
    BAPE_MixerInput input;

    if ( index >= NEXUS_NUM_AUDIO_PLAYBACKS )
    {
        BDBG_ERR(("Playback channel %u not available on this chipset", index));
        errCode = BERR_TRACE(BERR_INVALID_PARAMETER);
        return NULL;
    }

    pChannel = &g_playbacks[index];
    if ( pChannel->opened )
    {
        BDBG_ERR(("Playback channel %u already open", index));
        errCode = BERR_TRACE(BERR_INVALID_PARAMETER);
        return NULL;
    }
    NEXUS_OBJECT_INIT(NEXUS_AudioPlayback, pChannel);
    pChannel->settings.leftVolume = pChannel->settings.rightVolume = NEXUS_AUDIO_VOLUME_LINEAR_NORMAL;

    if ( NULL == pSettings )
    {
        NEXUS_AudioPlayback_GetDefaultOpenSettings(&defaultSettings);
        pSettings = &defaultSettings;
    }

    /* Setup Connector */
    pChannel->connector.format = NEXUS_AudioInputFormat_ePcmStereo;
    pChannel->connector.objectType = NEXUS_AudioInputType_ePlayback;
    pChannel->connector.pObjectHandle = pChannel;    

    /* Open Playback Channel */
    BAPE_Playback_GetDefaultOpenSettings(&openSettings);
    openSettings.fifoSize = pSettings->fifoSize;
    openSettings.watermarkThreshold = pSettings->threshold;
    errCode = BAPE_Playback_Open(NEXUS_AUDIO_DEVICE_HANDLE,
                                 index,
                                 &openSettings,
                                 &pChannel->channel);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        goto err_channel;
    }
    BAPE_Playback_GetConnector(pChannel->channel, &input);
    pChannel->connector.port = (uint32_t)input;

    BAPE_Playback_GetInterruptHandlers(pChannel->channel, &interrupts);
    interrupts.watermark.pCallback_isr = NEXUS_AudioPlayback_P_BufferFree_isr;
    interrupts.watermark.pParam1 = pChannel;
    errCode = BAPE_Playback_SetInterruptHandlers(pChannel->channel, &interrupts);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        goto err_interrupt;
    }

    pChannel->appCallback = NEXUS_TaskCallback_Create(pChannel, NULL);
    if ( NULL == pChannel->appCallback )
    {
        errCode = BERR_TRACE(BERR_OS_ERROR);
        goto err_callback;
    }

    errCode = BKNI_CreateEvent(&pChannel->event);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        goto err_event;
    }

    pChannel->eventCallback = NEXUS_RegisterEvent(pChannel->event, NEXUS_AudioPlayback_P_DataEvent, pChannel);
    if ( NULL == pChannel->eventCallback )
    {
        errCode = BERR_TRACE(BERR_OS_ERROR);
        goto err_event_callback;
    }

    /* Success */
    pChannel->opened = true;
    return pChannel;

err_event_callback:
    BKNI_DestroyEvent(pChannel->event);
err_event:
    NEXUS_TaskCallback_Destroy(pChannel->appCallback);    
err_callback:
err_interrupt:
    BAPE_Playback_Close(pChannel->channel);
err_channel:
    BKNI_Memset(pChannel, 0, sizeof(*pChannel));
    return NULL;
}

NEXUS_OBJECT_CLASS_MAKE(NEXUS_AudioPlayback, NEXUS_AudioPlayback_Close);

/***************************************************************************
Summary:
Close an audio playback channel
***************************************************************************/
static void NEXUS_AudioPlayback_P_Finalizer(
    NEXUS_AudioPlaybackHandle handle
    )
{
    NEXUS_OBJECT_ASSERT(NEXUS_AudioPlayback, handle);
    if ( handle->started )
    {
        BDBG_WRN(("Automatically stopping audio playback channel %p on close", handle));
        NEXUS_AudioPlayback_Stop(handle);
    }
    NEXUS_AudioInput_Shutdown(&handle->connector);
    BAPE_Playback_Close(handle->channel);
    NEXUS_TaskCallback_Destroy(handle->appCallback);
    NEXUS_UnregisterEvent(handle->eventCallback);
    BKNI_DestroyEvent(handle->event);
    BKNI_Memset(handle, 0, sizeof(NEXUS_AudioPlayback));
}

void NEXUS_AudioPlayback_Flush(
    NEXUS_AudioPlaybackHandle handle
    )
{
    NEXUS_OBJECT_ASSERT(NEXUS_AudioPlayback, handle);
    if ( handle->started )
    {
        (void)BAPE_Playback_Flush(handle->channel);
    }
}

/***************************************************************************
Summary:
Get default settings for an audio playback channel
***************************************************************************/
void NEXUS_AudioPlayback_GetDefaultStartSettings(
    NEXUS_AudioPlaybackStartSettings *pSettings  /* [out] Default Settings */
    )
{
    BAPE_PlaybackStartSettings startSettings;

    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));

    BAPE_Playback_GetDefaultStartSettings(&startSettings);

    /* Setup default parameters (the non-zero ones) */
    pSettings->sampleRate = NEXUS_AudioModule_P_Avc2SampleRate(startSettings.sampleRate);
    pSettings->bitsPerSample = startSettings.bitsPerSample;
    pSettings->stereo = startSettings.isStereo;
    pSettings->signedData = startSettings.isSigned;
    pSettings->startThreshold = (size_t)startSettings.startThreshold;
}

/***************************************************************************
Summary:
Start playing data data from an audio playback channel
***************************************************************************/
NEXUS_Error NEXUS_AudioPlayback_Start(
    NEXUS_AudioPlaybackHandle handle,
    const NEXUS_AudioPlaybackStartSettings *pSettings
    )
{
    NEXUS_Error errCode;
    NEXUS_AudioOutputList outputList;
    BAPE_PlaybackStartSettings startSettings;

    NEXUS_OBJECT_ASSERT(NEXUS_AudioPlayback, handle);
    BDBG_ASSERT(NULL != pSettings);

    if ( handle->started )
    {
        BDBG_ERR(("Audio playback channel %p already started.", handle));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* Check that we have some outputs ready */
    NEXUS_AudioInput_P_GetOutputs(&handle->connector, &outputList, false);
    if ( NULL == outputList.outputs[0] )
    {
        BDBG_ERR(("No outputs are connected to this playback channel.  Please connect outputs before starting."));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    BAPE_Playback_GetDefaultStartSettings(&startSettings);
    startSettings.sampleRate = NEXUS_AudioModule_P_SampleRate2Avc(pSettings->sampleRate);
    startSettings.bitsPerSample = pSettings->bitsPerSample;
    startSettings.isStereo = pSettings->stereo;
    startSettings.isSigned = pSettings->signedData;
    startSettings.startThreshold = pSettings->startThreshold;
    startSettings.loopEnabled = pSettings->loopAround;

    NEXUS_TaskCallback_Set(handle->appCallback, &pSettings->dataCallback);

    /* Connect to outputs */
    NEXUS_AudioInput_P_PrepareToStart(&handle->connector);

    /* Reset bytes played */
    handle->bytesPlayed = 0;
    /* Save Settings */
    handle->started = true;
    handle->startSettings = *pSettings;  
    handle->pLastAddress = NULL;

    /* Start Playback */
    errCode = BAPE_Playback_Start(handle->channel, &startSettings);
    if ( errCode )
    {
        handle->started = false;
        return BERR_TRACE(errCode);
    }

    return BERR_SUCCESS;
}

/***************************************************************************
Summary:
Stop playing data from an audio playback channel
***************************************************************************/
void NEXUS_AudioPlayback_Stop(
    NEXUS_AudioPlaybackHandle handle
    )
{
    NEXUS_OBJECT_ASSERT(NEXUS_AudioPlayback, handle);

    if ( handle->started )
    {
        BAPE_Playback_Stop(handle->channel);
        handle->started = false;
    }
}

/***************************************************************************
Summary:
Stop playing data from an audio playback channel without flushing.
***************************************************************************/
NEXUS_Error NEXUS_AudioPlayback_Suspend(
    NEXUS_AudioPlaybackHandle handle
    )
{
    BSTD_UNUSED(handle);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

/***************************************************************************
Summary:
Resume playing data from an audio playback channel.

Description:
***************************************************************************/
NEXUS_Error NEXUS_AudioPlayback_Resume(
    NEXUS_AudioPlaybackHandle handle
    )
{
    BSTD_UNUSED(handle);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

/***************************************************************************
Summary:
Get a pointer and size for the next location in the buffer which can accept data

Description:
NEXUS_AudioPlayback_GetBuffer is non-destructive. You can safely call it multiple times.
***************************************************************************/
NEXUS_Error NEXUS_AudioPlayback_GetBuffer(
    NEXUS_AudioPlaybackHandle handle,
    void **pBuffer, /* [out] attr{memory=cached} pointer to memory mapped region that is ready for playback data */
    size_t *pSize   /* [out] total number of writeable, contiguous bytes which buffer is pointing to */
    )
{
    BERR_Code errCode;
    unsigned bufSize;

    NEXUS_OBJECT_ASSERT(NEXUS_AudioPlayback, handle);
    BDBG_ASSERT(NULL != pBuffer);
    BDBG_ASSERT(NULL != pSize);

    errCode = BAPE_Playback_GetBuffer(handle->channel, pBuffer, &bufSize);
    if ( errCode ) 
    {
        return BERR_TRACE(errCode);
    }
    *pSize = bufSize;

    if ( bufSize > 0 )
    {
        errCode = BMEM_Heap_ConvertAddressToCached(g_pCoreHandles->heap[0].mem, *pBuffer, pBuffer);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
    }

    handle->pLastAddress = *pBuffer;  /* Save this for the cacheflush on write complete */
    BDBG_MSG(("NEXUS_AudioPlayback_GetBuffer %p, %d", *pBuffer, *pSize));

    return BERR_SUCCESS;
}

/***************************************************************************
Summary:
Notify AudioPlayback how much data was added into the buffer.

Description:
You can only call NEXUS_AudioPlayback_ReadComplete once after a 
NEXUS_AudioPlayback_GetBuffer call.  After calling it, you must call 
NEXUS_AudioPlayback_GetBuffer before adding more data.
***************************************************************************/
NEXUS_Error NEXUS_AudioPlayback_ReadComplete(
    NEXUS_AudioPlaybackHandle handle,
    size_t amountWritten            /* The number of bytes written to the buffer */
    )
{
    BERR_Code errCode;

    NEXUS_OBJECT_ASSERT(NEXUS_AudioPlayback, handle);

    /* this code assumes (and asserts) that there will be at least one get_buffer before each write_complete. 
    this should be true regardless of cache flush, but it makes the cache flush algo easier too. */
    if ( amountWritten > 0 )
    {
        if ( NULL == handle->pLastAddress )
        {
            BDBG_ERR(("You must call NEXUS_AudioPlayback_GetBuffer before calling NEXUS_AudioPlayback_ReadComplete"));
            BDBG_ASSERT(NULL != handle->pLastAddress);
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }
        BMEM_Heap_FlushCache(g_pCoreHandles->heap[0].mem, handle->pLastAddress, amountWritten);        
    }
    handle->pLastAddress = NULL;

    errCode = BAPE_Playback_CommitData(handle->channel, amountWritten);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }
    BDBG_MSG(("NEXUS_AudioPlayback_ReadComplete received %u bytes", amountWritten));

    /* Increment bytes played for status purposes */
    handle->bytesPlayed += amountWritten;

    return BERR_SUCCESS;
}

/***************************************************************************
Summary:
Get current status of the audio playback channel
***************************************************************************/
void NEXUS_AudioPlayback_GetStatus(
    NEXUS_AudioPlaybackHandle handle,
    NEXUS_AudioPlaybackStatus *pStatus      /* [out] Current Status */
    )
{
    BAPE_PlaybackStatus status;

    NEXUS_OBJECT_ASSERT(NEXUS_AudioPlayback, handle);
    BDBG_ASSERT(NULL != pStatus);

    BKNI_Memset(pStatus, 0, sizeof(*pStatus));

    pStatus->started = handle->started;
    pStatus->startSettings = handle->startSettings;

    BAPE_Playback_GetStatus(handle->channel, &status);
    pStatus->fifoSize = status.fifoSize;
    pStatus->queuedBytes = status.queuedBytes;
}

/***************************************************************************
Summary:
Get an audio connector for use with downstream components.  
**************************************************************************/
NEXUS_AudioInput NEXUS_AudioPlayback_GetConnector( /* attr{shutdown=NEXUS_AudioInput_Shutdown} */
    NEXUS_AudioPlaybackHandle handle
    )
{
    NEXUS_OBJECT_ASSERT(NEXUS_AudioPlayback, handle);
    return &handle->connector;
}

/* Handle the raptor buffer interrupt */
static void NEXUS_AudioPlayback_P_BufferFree_isr(void *pParam1, int param2)
{
    NEXUS_AudioPlaybackHandle handle = pParam1;

    NEXUS_OBJECT_ASSERT(NEXUS_AudioPlayback, handle);
    BSTD_UNUSED(param2);

    BKNI_SetEvent(handle->event);
}

/* Handle the raptor data ready callback */
static void NEXUS_AudioPlayback_P_DataEvent(void *pParam)
{
    NEXUS_Error errCode;
    NEXUS_AudioPlaybackHandle handle = pParam;
    void *pBuffer;
    size_t size=0;

    NEXUS_OBJECT_ASSERT(NEXUS_AudioPlayback, handle);

    /* non-blocking get of buffer space */
    /* If it fails, or if there's no space available, don't send a callback */
    errCode = NEXUS_AudioPlayback_GetBuffer(handle, &pBuffer, &size);
    if ( errCode )
    {
        BDBG_MSG(("No space available"));
        return;
    }
    else if ( size == 0 )
    {
        NEXUS_AudioPlayback_ReadComplete(handle, 0);
        return;
    }

    /* call back to the user with the space available -- this will do nothing if no callback is present */
    NEXUS_TaskCallback_Fire(handle->appCallback);
}

/* Private routine to get channel state */
bool NEXUS_AudioPlayback_P_IsRunning(NEXUS_AudioPlaybackHandle handle)
{
    NEXUS_OBJECT_ASSERT(NEXUS_AudioPlayback, handle);
    return handle->started;
}

/***************************************************************************
Summary:
Get current setting of the audio playback channel
***************************************************************************/
void NEXUS_AudioPlayback_GetSettings(
    NEXUS_AudioPlaybackHandle handle,
    NEXUS_AudioPlaybackSettings *pSettings  /* [out] Current settings */
    )
{
    NEXUS_OBJECT_ASSERT(NEXUS_AudioPlayback, handle);
    BDBG_ASSERT(NULL != pSettings);
    *pSettings = handle->settings;
}

/***************************************************************************
Summary:
Set current setting of the audio playback channel
***************************************************************************/
NEXUS_Error NEXUS_AudioPlayback_SetSettings(
    NEXUS_AudioPlaybackHandle handle,
    const NEXUS_AudioPlaybackSettings *pSettings
    )
{
    BAPE_MixerInputVolume vol;
    NEXUS_Error errCode;

    NEXUS_OBJECT_ASSERT(NEXUS_AudioPlayback, handle);
    BDBG_ASSERT(NULL != pSettings);

    NEXUS_AudioInput_P_GetVolume(&handle->connector, &vol);
    vol.coefficients[BAPE_Channel_eLeft][BAPE_Channel_eLeft] = pSettings->leftVolume;
    vol.coefficients[BAPE_Channel_eRight][BAPE_Channel_eRight] = pSettings->rightVolume;
    vol.muted = pSettings->muted;
    errCode = NEXUS_AudioInput_P_SetVolume(&handle->connector, &vol);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    handle->settings = *pSettings;

    return BERR_SUCCESS;
}

#else
/***************************************************************************
Summary:
Get default settings for an audio playback channel
***************************************************************************/
void NEXUS_AudioPlayback_GetDefaultOpenSettings(
    NEXUS_AudioPlaybackOpenSettings *pSettings      /* [out] default settings */
    )
{
    BSTD_UNUSED(pSettings);
}

/***************************************************************************
Summary:
Open an audio playback channel
***************************************************************************/
NEXUS_AudioPlaybackHandle NEXUS_AudioPlayback_Open(     /* attr{destructor=NEXUS_AudioPlayback_Close}  */
    unsigned index,
    const NEXUS_AudioPlaybackOpenSettings *pSettings    /* Pass NULL for default settings */
    )
{
    NEXUS_Error errCode;
    BSTD_UNUSED(index);
    BSTD_UNUSED(pSettings);
    errCode = BERR_TRACE(BERR_NOT_SUPPORTED);
    return NULL;
}

/***************************************************************************
Summary:
Close an audio playback channel
***************************************************************************/
void NEXUS_AudioPlayback_Close(
    NEXUS_AudioPlaybackHandle handle
    )
{
    BSTD_UNUSED(handle);
}

/***************************************************************************
Summary:
Get default settings for an audio playback channel
***************************************************************************/
void NEXUS_AudioPlayback_GetDefaultStartSettings(
    NEXUS_AudioPlaybackStartSettings *pSettings  /* [out] Default Settings */
    )
{
    BSTD_UNUSED(pSettings);
}

/***************************************************************************
Summary:
Start playing data data from an audio playback channel
***************************************************************************/
NEXUS_Error NEXUS_AudioPlayback_Start(
    NEXUS_AudioPlaybackHandle handle,
    const NEXUS_AudioPlaybackStartSettings *pSettings
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pSettings);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

/***************************************************************************
Summary:
Stop playing data from an audio playback channel.

Description:
This will stop the channel and flush all data from the FIFO.
***************************************************************************/
void NEXUS_AudioPlayback_Stop(
    NEXUS_AudioPlaybackHandle handle
    )
{
    BSTD_UNUSED(handle);
}

/***************************************************************************
Summary:
Get a pointer and size for the next location in the buffer which can accept data

Description:
NEXUS_AudioPlayback_GetBuffer is non-destructive. You can safely call it multiple times.
***************************************************************************/
NEXUS_Error NEXUS_AudioPlayback_GetBuffer(
    NEXUS_AudioPlaybackHandle handle,
    void **pBuffer, /* [out] attr{memory=cached} pointer to memory mapped region that is ready for playback data */
    size_t *pSize   /* [out] total number of writeable, contiguous bytes which buffer is pointing to */
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pBuffer);
    BSTD_UNUSED(pSize);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

/***************************************************************************
Summary:
Notify AudioPlayback how much data was added into the buffer.

Description:
You can only call NEXUS_AudioPlayback_ReadComplete once after a 
NEXUS_AudioPlayback_GetBuffer call.  After calling it, you must call 
NEXUS_AudioPlayback_GetBuffer before adding more data.
***************************************************************************/
NEXUS_Error NEXUS_AudioPlayback_ReadComplete(
    NEXUS_AudioPlaybackHandle handle,
    size_t amountWritten            /* The number of bytes written to the buffer */
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(amountWritten);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

/***************************************************************************
Summary:
Get current status of the audio playback channel
***************************************************************************/
void NEXUS_AudioPlayback_GetStatus(
    NEXUS_AudioPlaybackHandle handle,
    NEXUS_AudioPlaybackStatus *pStatus      /* [out] Current Status */
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pStatus);
}

/***************************************************************************
Summary:
Get an audio connector for use with downstream components.  
**************************************************************************/
NEXUS_AudioInput NEXUS_AudioPlayback_GetConnector( /* attr{shutdown=NEXUS_AudioInput_Shutdown} */
    NEXUS_AudioPlaybackHandle handle
    )
{
    BSTD_UNUSED(handle);
    return NULL;
}

/***************************************************************************
Summary:
Get current setting of the audio playback channel
***************************************************************************/
void NEXUS_AudioPlayback_GetSettings(
    NEXUS_AudioPlaybackHandle handle,
    NEXUS_AudioPlaybackSettings *pSettings  /* [out] Current settings */
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pSettings);
}

/***************************************************************************
Summary:
Set current setting of the audio playback channel
***************************************************************************/
NEXUS_Error NEXUS_AudioPlayback_SetSettings(
    NEXUS_AudioPlaybackHandle handle,
    const NEXUS_AudioPlaybackSettings *pSettings
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pSettings);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

void NEXUS_AudioPlayback_Flush(
    NEXUS_AudioPlaybackHandle handle
    )
{
    BSTD_UNUSED(handle);
}

#endif /* #if NEXUS_NUM_AUDIO_PLAYBACKS */

