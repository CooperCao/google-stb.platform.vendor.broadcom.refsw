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
    bool suspended;
    char name[11];   /* PLAYBACK %d */
} NEXUS_AudioPlayback;

#if NEXUS_NUM_AUDIO_PLAYBACKS
static NEXUS_AudioPlayback g_playbacks[NEXUS_NUM_AUDIO_PLAYBACKS];
#else
#ifndef NEXUS_NUM_AUDIO_PLAYBACKS
#define NEXUS_NUM_AUDIO_PLAYBACKS 0
#endif
static NEXUS_AudioPlayback g_playbacks[1];
#endif

static void NEXUS_AudioPlayback_P_DataEvent(void *pParam);
static void NEXUS_AudioPlayback_P_BufferFree_isr(void *pParam1, int param2);
static void NEXUS_AudioPlayback_P_SetConnectorFormat(NEXUS_AudioPlaybackHandle handle, const NEXUS_AudioPlaybackStartSettings *pSettings);

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
    BKNI_Memset(pSettings, 0, sizeof(NEXUS_AudioPlaybackOpenSettings));
    BAPE_Playback_GetDefaultOpenSettings(&openSettings);    
    pSettings->fifoSize = openSettings.bufferSize;
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
    NEXUS_HeapHandle heap;
    BAPE_Connector input;

    if (index == NEXUS_ANY_ID) {
        for (index=0;(int)index<NEXUS_NUM_AUDIO_PLAYBACKS;index++) {
            if (!g_playbacks[index].opened) break;
        }
        if (index == NEXUS_NUM_AUDIO_PLAYBACKS) {
            BDBG_ERR(("No more playback channels available"));
            return NULL;
        }
    }
    if ( (int)index >= NEXUS_NUM_AUDIO_PLAYBACKS )
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
    pChannel->settings.contentReferenceLevel = 20;

    if ( NULL == pSettings )
    {
        NEXUS_AudioPlayback_GetDefaultOpenSettings(&defaultSettings);
        pSettings = &defaultSettings;
    }

    /* Setup Connector */
    BKNI_Snprintf(pChannel->name, sizeof(pChannel->name), "PLAYBACK %u", index);
    NEXUS_AUDIO_INPUT_INIT(&pChannel->connector, NEXUS_AudioInputType_ePlayback, pChannel);
    pChannel->connector.pName = pChannel->name;
    pChannel->connector.format = NEXUS_AudioInputFormat_ePcmStereo;

    /* Open Playback Channel */
    BAPE_Playback_GetDefaultOpenSettings(&openSettings);
    heap = NEXUS_P_DefaultHeap(pSettings->heap, NEXUS_DefaultHeapType_eFull);
    if ( NULL == heap )
    {
        heap = g_pCoreHandles->heap[g_pCoreHandles->defaultHeapIndex].nexus;
    }
    if (!NEXUS_P_CpuAccessibleHeap(heap))
    {
        BDBG_ERR(("Playback heap is not CPU accessible.  Please provide a CPU-accessible heap in NEXUS_AudioPlaybackOpenSettings."));
        (void)BERR_TRACE(NEXUS_INVALID_PARAMETER);
        goto err_heap;
    }
    openSettings.heap = NEXUS_Heap_GetMemHandle(heap);
    openSettings.numBuffers = 1;
    openSettings.bufferSize = pSettings->fifoSize;
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
    pChannel->connector.port = (size_t)input;

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
    BDBG_OBJECT_SET(pChannel, NEXUS_AudioPlayback);
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
err_heap:
    BKNI_Memset(pChannel, 0, sizeof(*pChannel));
    return NULL;
}

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
        BDBG_WRN(("Automatically stopping audio playback channel %p on close", (void *)handle));
        NEXUS_AudioPlayback_Stop(handle);
    }
    NEXUS_AudioInput_Shutdown(&handle->connector);
    BAPE_Playback_Close(handle->channel);
    NEXUS_TaskCallback_Destroy(handle->appCallback);
    NEXUS_UnregisterEvent(handle->eventCallback);
    BKNI_DestroyEvent(handle->event);
    BKNI_Memset(handle, 0, sizeof(NEXUS_AudioPlayback));
}

NEXUS_OBJECT_CLASS_MAKE(NEXUS_AudioPlayback, NEXUS_AudioPlayback_Close);

void NEXUS_AudioPlayback_Flush(
    NEXUS_AudioPlaybackHandle handle
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioPlayback);
    BAPE_Playback_Flush(handle->channel);
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
    pSettings->sampleRate = startSettings.sampleRate;
    pSettings->bitsPerSample = startSettings.bitsPerSample;
    pSettings->stereo = startSettings.isStereo;
    pSettings->signedData = startSettings.isSigned;
    pSettings->startThreshold = (size_t)startSettings.startThreshold;
#if BSTD_CPU_ENDIAN == BSTD_ENDIAN_BIG
    pSettings->endian = NEXUS_EndianMode_eBig;
#else
    pSettings->endian = NEXUS_EndianMode_eLittle;
#endif
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
    BAPE_PlaybackSettings pbSettings;
    BAPE_MixerInputVolume vol;
    int attenuation = 0;

    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioPlayback);
    BDBG_ASSERT(NULL != pSettings);

    if ( handle->started )
    {
        BDBG_ERR(("Audio playback channel %p already started.", (void *)handle));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* Check that we have some outputs ready */
    NEXUS_AudioInput_P_GetOutputs(&handle->connector, &outputList, false);
    if ( NULL == outputList.outputs[0] )
    {
        BDBG_ERR(("No outputs are connected to this playback channel.  Please connect outputs before starting."));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    BAPE_Playback_GetSettings(handle->channel, &pbSettings);
    pbSettings.compressedData = pSettings->compressed;
    errCode = BAPE_Playback_SetSettings(handle->channel, &pbSettings);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    BAPE_Playback_GetDefaultStartSettings(&startSettings);
    startSettings.sampleRate = pSettings->sampleRate;
    startSettings.bitsPerSample = pSettings->compressed ? 16 : pSettings->bitsPerSample;
    startSettings.isStereo = pSettings->compressed ? true : pSettings->stereo;
    startSettings.isSigned = pSettings->compressed ? true : pSettings->signedData;
    startSettings.startThreshold = pSettings->startThreshold;
    startSettings.loopEnabled = pSettings->loopAround;
#if BSTD_CPU_ENDIAN == BSTD_ENDIAN_BIG
    startSettings.reverseEndian = pSettings->endian==NEXUS_EndianMode_eLittle?1:0;
#else
    startSettings.reverseEndian = pSettings->endian==NEXUS_EndianMode_eBig?1:0;
#endif
    NEXUS_TaskCallback_Set(handle->appCallback, &pSettings->dataCallback);

    /* Connect to outputs */
    NEXUS_AudioPlayback_P_SetConnectorFormat(handle, pSettings);
    errCode = NEXUS_AudioInput_P_PrepareToStart(&handle->connector);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    NEXUS_AudioInput_P_GetVolume(&handle->connector, &vol);
    if (handle->settings.contentReferenceLevel < 11)
    {
        attenuation = 11 - handle->settings.contentReferenceLevel;
    }
    vol.coefficients[BAPE_Channel_eLeft][BAPE_Channel_eLeft] = (int32_t)(((uint64_t)handle->settings.leftVolume  * NEXUS_Audio_P_ConvertDbToLinear(attenuation)) / 0x800000);
    vol.coefficients[BAPE_Channel_eRight][BAPE_Channel_eRight] = (int32_t)(((uint64_t)handle->settings.rightVolume  * NEXUS_Audio_P_ConvertDbToLinear(attenuation)) / 0x800000);
    vol.muted = handle->settings.muted;
    errCode = NEXUS_AudioInput_P_SetVolume(&handle->connector, &vol);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    /* Reset bytes played */
    handle->bytesPlayed = 0;
    /* Save Settings */
    handle->started = true;
    handle->suspended = false;
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
    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioPlayback);

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
    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioPlayback);
    if ( !handle->suspended && handle->started)
    {
        NEXUS_AudioPlayback_Stop(handle);
        handle->suspended = true;
        return BERR_SUCCESS;
    }
    BDBG_ERR(("Playback not started, so can't suspend it!"));
    return BERR_TRACE(NEXUS_NOT_INITIALIZED);
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

    NEXUS_Error errCode;
    NEXUS_AudioOutputList outputList;
    BAPE_PlaybackStartSettings startSettings;
    BAPE_MixerInputVolume vol;
    int attenuation = 0;

    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioPlayback);

    if ( handle->started )
    {
        BDBG_ERR(("Audio playback channel %p already started.", (void *)handle));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    if ( !handle->suspended )
    {
        BDBG_ERR(("Audio playback channel %p is not suspended.", (void *)handle));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    /* Check that we have some outputs ready */
    NEXUS_AudioInput_P_GetOutputs(&handle->connector, &outputList, false);
    if ( NULL == outputList.outputs[0] )
    {
        BDBG_ERR(("No outputs are connected to this playback channel.  Please connect outputs before starting."));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    BAPE_Playback_GetDefaultStartSettings(&startSettings);
    startSettings.sampleRate = handle->startSettings.sampleRate;
    startSettings.bitsPerSample = handle->startSettings.bitsPerSample;
    startSettings.isStereo = handle->startSettings.stereo;
    startSettings.isSigned = handle->startSettings.signedData;
    startSettings.startThreshold = handle->startSettings.startThreshold;
    startSettings.loopEnabled = handle->startSettings.loopAround;

    NEXUS_TaskCallback_Set(handle->appCallback, &handle->startSettings.dataCallback);

    /* Connect to outputs */
    errCode = NEXUS_AudioInput_P_PrepareToStart(&handle->connector);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    NEXUS_AudioInput_P_GetVolume(&handle->connector, &vol);
    if (handle->settings.contentReferenceLevel < 11)
    {
        attenuation = 11 - handle->settings.contentReferenceLevel;
    }
    vol.coefficients[BAPE_Channel_eLeft][BAPE_Channel_eLeft] = (int32_t)(((uint64_t)handle->settings.leftVolume  * NEXUS_Audio_P_ConvertDbToLinear(attenuation)) / 0x800000);
    vol.coefficients[BAPE_Channel_eRight][BAPE_Channel_eRight] = (int32_t)(((uint64_t)handle->settings.rightVolume  * NEXUS_Audio_P_ConvertDbToLinear(attenuation)) / 0x800000);
    vol.muted = handle->settings.muted;
    errCode = NEXUS_AudioInput_P_SetVolume(&handle->connector, &vol);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    /* Save Settings */
    handle->started = true;
    handle->suspended = false;

    /* Start Playback */
    errCode = BAPE_Playback_Start(handle->channel, &startSettings);
    if ( errCode )
    {
        handle->started = false;
        return BERR_TRACE(errCode);
    }

    return BERR_SUCCESS;


}

static NEXUS_Error NEXUS_AudioPlayback_P_GetBuffer(
    NEXUS_AudioPlaybackHandle handle,
    void **pBuffer, /* [out] attr{memory=cached} pointer to memory mapped region that is ready for playback data */
    size_t *pSize   /* [out] total number of writeable, contiguous bytes which buffer is pointing to */
    )
{
    BERR_Code errCode;
    BAPE_BufferDescriptor bufferDescriptor;

    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioPlayback);
    BDBG_ASSERT(NULL != pBuffer);
    BDBG_ASSERT(NULL != pSize);

    errCode = BAPE_Playback_GetBuffer(handle->channel, &bufferDescriptor);
    if ( errCode ) 
    {
        return BERR_TRACE(errCode);
    }
    *pSize = bufferDescriptor.bufferSize;

    if ( bufferDescriptor.bufferSize > 0 )
    {
        errCode = BMEM_Heap_ConvertAddressToCached(g_pCoreHandles->heap[g_pCoreHandles->defaultHeapIndex].mem, bufferDescriptor.buffers[BAPE_Channel_eLeft].pBuffer, pBuffer);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
    }
    else
    {
        *pBuffer = NULL;
    }

    return BERR_SUCCESS;
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

    errCode = NEXUS_AudioPlayback_P_GetBuffer(handle, pBuffer, pSize);
    if ( errCode ) 
    {
        return BERR_TRACE(errCode);
    }

    if ( *pSize > 0 )
    {
        handle->pLastAddress = *pBuffer;  /* Save this for the cacheflush on write complete */
    }

    BDBG_MSG(("NEXUS_AudioPlayback_GetBuffer %p, %lu", (void *)*pBuffer, (unsigned long)*pSize));

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

    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioPlayback);

    /* this code assumes (and asserts) that there will be at least one get_buffer before each write_complete. 
    this should be true regardless of cache flush, but it makes the cache flush algo easier too. */
    if ( amountWritten > 0 )
    {
        if ( NULL == handle->pLastAddress )
        {
            BDBG_ERR(("You must call NEXUS_AudioPlayback_GetBuffer before calling NEXUS_AudioPlayback_ReadComplete"));
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }
        BMEM_Heap_FlushCache(g_pCoreHandles->heap[g_pCoreHandles->defaultHeapIndex].mem, handle->pLastAddress, amountWritten);
    }
    handle->pLastAddress = NULL;

    errCode = BAPE_Playback_CommitData(handle->channel, amountWritten);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }
    BDBG_MSG(("NEXUS_AudioPlayback_ReadComplete received %lu bytes", (unsigned long)amountWritten));

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

    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioPlayback);
    BDBG_ASSERT(NULL != pStatus);

    BKNI_Memset(pStatus, 0, sizeof(*pStatus));

    pStatus->started = handle->started;
    pStatus->startSettings = handle->startSettings;

    BAPE_Playback_GetStatus(handle->channel, &status);
    pStatus->fifoSize = status.fifoSize;
    pStatus->queuedBytes = status.queuedBytes;
    pStatus->playedBytes = handle->bytesPlayed - status.queuedBytes;
}

/***************************************************************************
Summary:
Get an audio connector for use with downstream components.  
**************************************************************************/
NEXUS_AudioInput NEXUS_AudioPlayback_GetConnector( /* attr{shutdown=NEXUS_AudioInput_Shutdown} */
    NEXUS_AudioPlaybackHandle handle
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioPlayback);
    return &handle->connector;
}

/* Handle the raptor buffer interrupt */
static void NEXUS_AudioPlayback_P_BufferFree_isr(void *pParam1, int param2)
{
    NEXUS_AudioPlaybackHandle handle = pParam1;

    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioPlayback);
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

    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioPlayback);

    /* non-blocking get of buffer space */
    /* If it fails, or if there's no space available, don't send a callback */
    errCode = NEXUS_AudioPlayback_P_GetBuffer(handle, &pBuffer, &size);
    if ( errCode )
    {
        BDBG_MSG(("No space available"));
        return;
    }

    /* call back to the user with the space available -- this will do nothing if no callback is present */
    NEXUS_TaskCallback_Fire(handle->appCallback);
}

/* Private routine to get channel state */
bool NEXUS_AudioPlayback_P_IsRunning(NEXUS_AudioPlaybackHandle handle)
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioPlayback);
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
    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioPlayback);
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
    BAPE_PlaybackSettings apeSettings;
    NEXUS_Error errCode;
    int attenuation = 0;

    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioPlayback);
    BDBG_ASSERT(NULL != pSettings);

    NEXUS_AudioInput_P_GetVolume(&handle->connector, &vol);
    if (pSettings->contentReferenceLevel < 11)
    {
        attenuation = 11 - pSettings->contentReferenceLevel;
    }
    vol.coefficients[BAPE_Channel_eLeft][BAPE_Channel_eLeft] = (int32_t)(((uint64_t)pSettings->leftVolume  * NEXUS_Audio_P_ConvertDbToLinear(attenuation)) / 0x800000);
    vol.coefficients[BAPE_Channel_eRight][BAPE_Channel_eRight] = (int32_t)(((uint64_t)pSettings->rightVolume  * NEXUS_Audio_P_ConvertDbToLinear(attenuation)) / 0x800000);
    vol.muted = pSettings->muted;
    errCode = NEXUS_AudioInput_P_SetVolume(&handle->connector, &vol);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    BAPE_Playback_GetSettings(handle->channel, &apeSettings);
    apeSettings.sampleRate = pSettings->sampleRate;
    errCode = BAPE_Playback_SetSettings(handle->channel, &apeSettings);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    handle->settings = *pSettings;

    return BERR_SUCCESS;
}

NEXUS_AudioPlaybackHandle
NEXUS_AudioPlayback_P_GetPlaybackByIndex(
    unsigned index
    )
{
#if NEXUS_NUM_AUDIO_PLAYBACKS
    BDBG_ASSERT(index < NEXUS_NUM_AUDIO_PLAYBACKS);
    return g_playbacks[index].opened?&g_playbacks[index]:NULL;
#else
    BSTD_UNUSED(index);
    return NULL;
#endif
}

static void NEXUS_AudioPlayback_P_SetConnectorFormat(NEXUS_AudioPlaybackHandle handle, const NEXUS_AudioPlaybackStartSettings *pSettings)
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioPlayback);
    if ( pSettings->compressed )
    {
        BDBG_MSG(("Receiving Compressed.  Updating connector format."));
        handle->connector.format = NEXUS_AudioInputFormat_eCompressed;
    }
    else
    {
        /* TODO: Extend for multichannel when required */
        handle->connector.format = NEXUS_AudioInputFormat_ePcmStereo;
    }
}
