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
    BRAP_PB_AudioParams audioParams;
    BRAP_ChannelHandle rapChannel;
    NEXUS_TaskCallbackHandle appCallback;
    NEXUS_AudioPlaybackOpenSettings openSettings;
    NEXUS_AudioPlaybackStartSettings startSettings;
    NEXUS_AudioPlaybackSettings settings;
    BKNI_EventHandle event;
    NEXUS_EventCallbackHandle eventCallback;
    unsigned fifoSize;
    void *pLastAddress;
    size_t bytesPlayed;
    bool started;
    bool opened;
    BRAP_OutputPort outputList[NEXUS_AUDIO_MAX_OUTPUTS];
} NEXUS_AudioPlayback;

static NEXUS_AudioPlayback g_playbacks[NEXUS_NUM_AUDIO_PLAYBACKS];
static void NEXUS_AudioPlayback_P_DataEvent(void *pParam);
static void NEXUS_AudioPlayback_P_BufferFree_isr(void *pParam1, int param2, void *pData);
static NEXUS_Error NEXUS_AudioPlayback_P_ConnectionChange(void *pParam, NEXUS_AudioInput input);
static void NEXUS_AudioPlayback_P_SetSampleRate(NEXUS_AudioPlaybackHandle handle);

/***************************************************************************
Summary:
Get default settings for an audio playback channel
***************************************************************************/
void NEXUS_AudioPlayback_GetDefaultOpenSettings(
    NEXUS_AudioPlaybackOpenSettings *pSettings      /* [out] default settings */
    )
{
    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
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
    BRAP_PB_ChannelSettings *pChannelSettings;
    int i;
    if (index == NEXUS_ANY_ID) {
        for (index=0;index<NEXUS_NUM_AUDIO_PLAYBACKS;index++) {
            if (!g_playbacks[index].opened) break;
        }
        if (index == NEXUS_NUM_AUDIO_PLAYBACKS) {
            BDBG_ERR(("No more playback channels available"));
            return NULL;
        }
    }
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
    NEXUS_AUDIO_INPUT_INIT(&pChannel->connector, NEXUS_AudioInputType_ePlayback, pChannel);
    pChannel->connector.format = NEXUS_AudioInputFormat_ePcmStereo;
    pChannel->connector.disconnectCb = NEXUS_AudioPlayback_P_ConnectionChange;

    /* Invalidate outputs */
    for (i=0;i<NEXUS_AUDIO_MAX_OUTPUTS;i++)
    {
        pChannel->outputList[i] = BRAP_OutputPort_eMax;
    }

    /* Open Raptor Channel */
    pChannelSettings = BKNI_Malloc(sizeof(*pChannelSettings));
    if ( NULL == pChannelSettings )
    {
        errCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        return NULL;
    }

    (void)BRAP_PB_GetChannelDefaultSettings(g_NEXUS_audioModuleData.hRap, pChannelSettings);

    #if (BCHP_CHIP==7400)
    pChannelSettings->eOutputPort = (index==1) ? BRAP_OutputPort_eDac1 : BRAP_OutputPort_eDac0;
    #else
    pChannelSettings->eOutputPort = BRAP_OutputPort_eDac0;
    #endif
    pChannel->outputList[0] = pChannelSettings->eOutputPort; /* indicate this initial DAC assignment */
    pChannelSettings->eBufDataMode = BRAP_BufDataMode_eStereoInterleaved;
    pChannel->fifoSize = pSettings->fifoSize;
    if ( pSettings->fifoSize != 0 )
    {
        /* Round up size to nearest 256-byte boundary */
        pChannelSettings->sLtOrSnglRbufSettings.uiSize = (pSettings->fifoSize + 255) & ~255;
    }

    if ( pSettings->threshold && pSettings->fifoSize )
    {
        unsigned threshold;
        /* User specified a threshold.  Convert to % of fifo size */
        threshold = (100*pSettings->threshold)/pSettings->fifoSize;
        if ( threshold >= 100 )
        {
            BDBG_WRN(("Invalid FIFO threshold requested.  Using Defaults."));
            threshold=0;
        }
        BDBG_MSG(("Setting caller-provided threshold %u.",threshold));
        BDBG_MSG(("pSettings->threshold =  %u. pSettings->fifoSize = %u",
                  pSettings->threshold, pSettings->fifoSize));
        pChannelSettings->sLtOrSnglRbufSettings.uiWaterMark = threshold;
    }
    else
    {
        /* Use defaults from raptor */
        pChannelSettings->sLtOrSnglRbufSettings.uiWaterMark=50;
        BDBG_MSG(("Setting default threshold %u.",pChannelSettings->sLtOrSnglRbufSettings.uiWaterMark));
    }

    errCode = BRAP_PB_OpenChannel(g_NEXUS_audioModuleData.hRap, &pChannel->rapChannel, index, pChannelSettings);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        goto err_channel;
    }

    errCode = BRAP_InstallAppInterruptCb(pChannel->rapChannel,
                                         BRAP_Interrupt_eFmmRbufFreeByte,
                                         NEXUS_AudioPlayback_P_BufferFree_isr, pChannel, 0);
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
    BKNI_Free(pChannelSettings);
    pChannel->opened = true;
    return pChannel;

err_event_callback:
    BKNI_DestroyEvent(pChannel->event);
err_event:
    NEXUS_TaskCallback_Destroy(pChannel->appCallback);
err_callback:
    BRAP_RemoveAppInterruptCb(pChannel->rapChannel, BRAP_Interrupt_eFmmRbufFreeByte);
err_interrupt:
    BRAP_PB_CloseChannel(pChannel->rapChannel);
err_channel:
    BKNI_Free(pChannelSettings);
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
    NEXUS_Error errCode;
    int i;

    NEXUS_OBJECT_ASSERT(NEXUS_AudioPlayback, handle);
    if ( handle->started )
    {
        BDBG_WRN(("Automatically stopping audio playback channel %p on close", (void *)handle));
        NEXUS_AudioPlayback_Stop(handle);
    }
    NEXUS_AudioInput_Shutdown(&handle->connector);

    /* Remove all outputs except DAC */
    for ( i = 0; i < NEXUS_AUDIO_MAX_OUTPUTS; i++ )
    {
        BRAP_OutputPort output = handle->outputList[i];
        if ( BRAP_OutputPort_eMax != output )
        {
            if ( ( BRAP_OutputPort_eRfMod != output ) &&
                 ( BRAP_OutputPort_eDac0 != output ) &&
                 ( BRAP_OutputPort_eDac1 != output ) )
            {
                BDBG_MSG(("%s() Removing output port %d from playback", __FUNCTION__,output));
                errCode = BRAP_RemoveOutputPort(handle->rapChannel, output);
                if ( errCode ) {    /* Report error and continue */
                    errCode = BERR_TRACE(errCode);
                }
                /* Clear this output port */
                handle->outputList[i] = BRAP_OutputPort_eMax;
            }
        }
    }

    BRAP_RemoveAppInterruptCb(handle->rapChannel, BRAP_Interrupt_eFmmRbufFreeByte);
    BRAP_PB_CloseChannel(handle->rapChannel);
    NEXUS_TaskCallback_Destroy(handle->appCallback);
    NEXUS_UnregisterEvent(handle->eventCallback);
    BKNI_DestroyEvent(handle->event);
    NEXUS_OBJECT_CLEAR(NEXUS_AudioPlayback, handle);
}

NEXUS_OBJECT_CLASS_MAKE(NEXUS_AudioPlayback, NEXUS_AudioPlayback_Close);

/***************************************************************************
Summary:
Get default settings for an audio playback channel
***************************************************************************/
void NEXUS_AudioPlayback_GetDefaultStartSettings(
    NEXUS_AudioPlaybackStartSettings *pSettings  /* [out] Default Settings */
    )
{
    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    /* Setup default parameters (the non-zero ones) */
    pSettings->sampleRate = 48000;
    pSettings->bitsPerSample = 16;
    pSettings->stereo = true;
    pSettings->signedData = true;
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
    int i;
    bool changed=false;

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

    /* Check for changes */
    changed = false;
    for (i=0;i<NEXUS_AUDIO_MAX_OUTPUTS;i++)
    {
        if (!outputList.outputs[i])
        {
            continue;
        }
        else
        {
            BDBG_MSG(("outputList.outputs[%d]->port = %d",i,outputList.outputs[i]->port));
            BDBG_MSG(("handle->outputList[%d] = %d",i,handle->outputList[i]));
        }

        changed = ( outputList.outputs[i] &&
                   (outputList.outputs[i]->port != handle->outputList[i]));
        if (changed)
        {
            break;
        }
    }
    if ( changed )
    {
        BDBG_MSG(("Output change detected for playback connector."));
    }
    else
    {
        BDBG_MSG(("NO Output change detected for playback connector."));
    }


    (void)BRAP_PB_GetDefaultAudioParams(handle->rapChannel, &handle->audioParams);
    if ( 0 == pSettings->sampleRate )
    {
        /* Pick a default sample rate, it will be overridden later. */
        handle->audioParams.eInputSamplingRate = BAVC_AudioSamplingRate_e48k;
    }
    else
    {
        handle->audioParams.eInputSamplingRate = NEXUS_AudioModule_P_SampleRate2Avc(pSettings->sampleRate);
    }
    switch ( pSettings->bitsPerSample )
    {
    case 8:
        handle->audioParams.eInputBitsPerSample = BRAP_InputBitsPerSample_e8;
        break;
    case 16:
        handle->audioParams.eInputBitsPerSample = BRAP_InputBitsPerSample_e16;
        break;
    case 32:
        handle->audioParams.eInputBitsPerSample = BRAP_InputBitsPerSample_e32;
        break;
    default:
        BDBG_ERR(("Unsupported number of bits per channel %u", pSettings->bitsPerSample));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    handle->audioParams.bLoopAroundEn = pSettings->loopAround;
    handle->audioParams.eBufDataMode = (pSettings->stereo)?BRAP_BufDataMode_eStereoInterleaved:BRAP_BufDataMode_eMono;
    handle->audioParams.bIsSigned = pSettings->signedData;
    handle->audioParams.ePcmOutputMode = BRAP_PcmOutputMode_eStereo;
    if ( handle->fifoSize )
    {
        handle->audioParams.ui32RbufMemSize = handle->fifoSize;
    }

    NEXUS_TaskCallback_Set(handle->appCallback, &pSettings->dataCallback);

    /* This process is slow and involves memory allocation/free.  Only do it if something changed */
    if ( changed )
    {
        /* attach output ports */
        for ( i = 0; i < NEXUS_AUDIO_MAX_OUTPUTS; i++ )
        {
            NEXUS_AudioOutputHandle output = outputList.outputs[i];
            if ( NULL == output )
            {
                BDBG_MSG(("Done adding %d output ports to PCM playback", i));
                break;
            }
            else if (outputList.outputs[i]->port != handle->outputList[i])
            {
                if (!NEXUS_AudioOutput_P_IsDacSlave(output))
                {
                    BDBG_MSG(("Adding output port %d to PCM playback", output->port));
                    errCode = BRAP_AddOutputPort(handle->rapChannel, false, BRAP_OutputChannelPair_eLR, output->port);
                    /* TODO: is this necessary? (comment from native Settop version) */
                    /* If PCM is going to HDMI (Mai) and decode is not started, than the app must set the HDMI sample rate.
                    Any decode sample rate interrupt will overwrite this. PCM start and Decode start must be synchronized. */

                }
                else
                {
                    BDBG_MSG(("Not adding output port %d to PCM playback, it's a DacSlave", output->port));
                    errCode = NEXUS_SUCCESS;
                }
                if ( errCode )
                {
                    /* Report and continue */
                    errCode = BERR_TRACE(errCode);
                }
                else
                {
                    handle->outputList[i] = output->port;
                }
            }
        }
    }
    /* Connect to outputs */
    NEXUS_AudioInput_P_PrepareToStart(&handle->connector);

    /* Reset bytes played */
    handle->bytesPlayed = 0;
    /* Save Settings */
    handle->started = true;
    handle->startSettings = *pSettings;
    handle->pLastAddress = NULL;

    /* Start Playback */
    errCode = BRAP_PB_Start(handle->rapChannel, &handle->audioParams);
    if ( errCode )
    {
        handle->started = false;
        return BERR_TRACE(errCode);
    }

    if ( 0 == handle->startSettings.sampleRate )
    {
        /* Make sure runtime sample rate is set */
        NEXUS_AudioPlayback_P_SetSampleRate(handle);
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
    BDBG_MSG(("%s()",__FUNCTION__));

    if ( handle->started )
    {
        BRAP_PB_Stop(handle->rapChannel);
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
    BRAP_PcmBufInfo bufInfo;

    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioPlayback);
    BDBG_ASSERT(NULL != pBuffer);
    BDBG_ASSERT(NULL != pSize);

    errCode = BRAP_PB_GetBufInfo(handle->rapChannel, &bufInfo);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    if ( bufInfo.sRtBufInfo.uiContiguousSize )
    {
        BDBG_ERR(("Dual channel PCM playback not supported."));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    errCode = BMEM_Heap_ConvertAddressToCached(g_pCoreHandles->heap[0].mem, bufInfo.sLtOrSnglBufInfo.pWritePtr, pBuffer);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }
    handle->pLastAddress = *pBuffer;  /* Save this for the cacheflush on write complete */
    *pSize = bufInfo.sLtOrSnglBufInfo.uiContiguousSize;
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
    BRAP_PcmBufSzUpdt update;

    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioPlayback);


    /* this code assumes (and asserts) that there will be at least one get_buffer before each write_complete.
    this should be true regardless of cache flush, but it makes the cache flush algo easier too. */
    if ( NULL == handle->pLastAddress )
    {
        BDBG_ERR(("You must call NEXUS_AudioPlayback_GetBuffer before calling NEXUS_AudioPlayback_ReadComplete"));
        BDBG_ASSERT(NULL != handle->pLastAddress);
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
    if ( amountWritten > 0 )
    {
        BMEM_Heap_FlushCache(g_pCoreHandles->heap[0].mem, handle->pLastAddress, amountWritten);
    }
    handle->pLastAddress = NULL;

    update.uiLtOrSnglBufSzUpdt = amountWritten;
    update.uiRtBufSzUpdt = 0;
    errCode = BRAP_PB_UpdateBufUsg(handle->rapChannel, &update);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }
    BDBG_MSG(("NEXUS_AudioPlayback_ReadComplete received %u bytes", amountWritten));

    if ( handle->started )
    {
        errCode = BRAP_PB_BufferWriteDone(handle->rapChannel);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
    }

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
    BERR_Code errCode;
    BRAP_PcmBufInfo bufInfo;

    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioPlayback);
    BDBG_ASSERT(NULL != pStatus);

    BKNI_Memset(pStatus, 0, sizeof(*pStatus));

    pStatus->started = handle->started;
    pStatus->startSettings = handle->startSettings;

    errCode = BRAP_PB_GetBufInfo(handle->rapChannel, &bufInfo);
    if ( errCode == BERR_SUCCESS )
    {
        pStatus->fifoSize = (((uint8_t*)bufInfo.sLtOrSnglBufInfo.pEndPtr)+1) - (uint8_t*)bufInfo.sLtOrSnglBufInfo.pBasePtr; /* number of bytes */
        pStatus->queuedBytes = pStatus->fifoSize - bufInfo.sLtOrSnglBufInfo.uiTotalSize; /* total - usable free == queued */
    }
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
static void NEXUS_AudioPlayback_P_BufferFree_isr(void *pParam1, int param2, void *pData)
{
    NEXUS_AudioPlaybackHandle handle = pParam1;

    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioPlayback);
    BSTD_UNUSED(param2);
    BSTD_UNUSED(pData);
    BDBG_MSG(("%s()",__FUNCTION__));

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

    BDBG_MSG(("%s()",__FUNCTION__));

    /* non-blocking get of buffer space */
    /* If it fails, or if there's no space available, don't send a callback */
    errCode = NEXUS_AudioPlayback_GetBuffer(handle, &pBuffer, &size);
    if ( errCode )
    {
        BDBG_MSG(("No space available"));
        return;
    }
    #if 1 /* This is necessary if the BRAP_P_WATERMARK_WORKAROUND 1 is used in brap_priv.h */
    else if ( size == 0 )
    {
        NEXUS_AudioPlayback_ReadComplete(handle, 0);
        return;
    }
    #endif
    /* call back to the user with the space available -- this will do nothing if no callback is present */
    BDBG_MSG(("NEXUS_TaskCallback_Fire(%p)", (void *)handle->appCallback));
    NEXUS_TaskCallback_Fire(handle->appCallback);
}

/* Private routine to get channel state */
bool NEXUS_AudioPlayback_P_IsRunning(NEXUS_AudioPlaybackHandle handle)
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioPlayback);
    BDBG_MSG(("%s() returning %d",__FUNCTION__,handle->started));
    return handle->started;
}

/* Private routine to get channel handle */
BRAP_ChannelHandle NEXUS_AudioPlayback_P_GetChannel(NEXUS_AudioPlaybackHandle handle)
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioPlayback);
    return handle->rapChannel;
}

static NEXUS_Error NEXUS_AudioPlayback_P_ConnectionChange(void *pParam, NEXUS_AudioInput input)
{
    NEXUS_Error errCode;
    NEXUS_AudioPlayback *pPlayback = pParam;
    int j;

    BSTD_UNUSED(input);
    BDBG_ASSERT(NULL != pPlayback);

    BDBG_MSG(("Audio playback %p connection change", pParam));

    for ( j = 0; j < NEXUS_AUDIO_MAX_OUTPUTS; j++ )
    {
        BRAP_OutputPort output = pPlayback->outputList[j];
        if ( output != BRAP_OutputPort_eMax )
        {
            if ( ( BRAP_OutputPort_eRfMod != output ) &&
                 ( BRAP_OutputPort_eDac0 != output ) &&
                 ( BRAP_OutputPort_eDac1 != output ) )
            {
                BDBG_MSG(("%s() Removing output port %d from playback", __FUNCTION__,output));
                errCode = BRAP_RemoveOutputPort(pPlayback->rapChannel, output);
                if ( errCode )
                {
                    BDBG_ERR(("Trouble removing output port %d from PCM playback", output));
                    /* Report error and continue */
                    errCode = BERR_TRACE(errCode);
                }
                /* Clear this output port */
                pPlayback->outputList[j] = BRAP_OutputPort_eMax;
            }
        }
    }
    return NEXUS_SUCCESS;
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
    BDBG_ASSERT(NULL != handle);
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
    NEXUS_Error errCode = NEXUS_SUCCESS;
    int32_t volume;
    NEXUS_AudioOutputList outputList;
    int j;

    BDBG_ASSERT(NULL != handle);
    BDBG_ASSERT(NULL != pSettings);
    handle->settings = *pSettings;

    if ( pSettings->muted )
    {
        volume = NEXUS_AudioModule_P_Vol2Magnum(NEXUS_AudioVolumeType_eLinear, 0);
    }
    else
    {
        if (pSettings->leftVolume != pSettings->rightVolume)
        {
            BDBG_ERR(("7400 has no support for leftVolume != rightVolume, using leftVolume"));
            /* Report error and continue */
            errCode = BERR_TRACE(NEXUS_INVALID_PARAMETER);
        }
        volume = NEXUS_AudioModule_P_Vol2Magnum(NEXUS_AudioVolumeType_eLinear, pSettings->leftVolume);
    }

    BDBG_MSG(("Setting volume %d as %d", pSettings->leftVolume, volume));

    NEXUS_AudioInput_P_GetOutputs(&handle->connector, &outputList, false);

    for ( j = 0; j < NEXUS_AUDIO_MAX_OUTPUTS; j++ )
    {
        NEXUS_AudioOutputHandle output = outputList.outputs[j];
        if ( output )
        {
            if (!NEXUS_AudioOutput_P_IsDacSlave(output))
            {
                errCode = BRAP_MIXER_SetInputScaling(handle->rapChannel, output->port, volume);
                if ( errCode )
                {
                    /* Report error and continue */
                    errCode = BERR_TRACE(errCode);
                }
            }
        }
    }

    NEXUS_AudioPlayback_P_SetSampleRate(handle);

    return errCode;
}

/***************************************************************************
Summary:
Flush an audio playback channel
***************************************************************************/
void NEXUS_AudioPlayback_Flush(
    NEXUS_AudioPlaybackHandle handle
    )
{
    BSTD_UNUSED(handle);
}

static void NEXUS_AudioPlayback_P_SetSampleRate(NEXUS_AudioPlaybackHandle handle)
{

    BDBG_ASSERT(NULL != handle);

    if ( handle->started && handle->startSettings.sampleRate == 0 && handle->settings.sampleRate > 0 )
    {
        unsigned i;
        NEXUS_AudioOutputList outputList;

        NEXUS_AudioInput_P_GetOutputs(&handle->connector, &outputList, false);

        for ( i = 0; i < NEXUS_AUDIO_MAX_OUTPUTS; i++ )
        {
            NEXUS_AudioOutputHandle output = outputList.outputs[i];
            if ( output )
            {
                if (!NEXUS_AudioOutput_P_IsDacSlave(output))
                {
                    BERR_Code errCode;
                    NEXUS_AudioOutputSettings outputSettings;

                    NEXUS_AudioOutput_GetSettings(output, &outputSettings);
                    errCode = BRAP_PB_SetSRCFrequency(handle->rapChannel, output->port, outputSettings.defaultSampleRate, handle->settings.sampleRate);
                    if ( errCode )
                    {
                        (void)BERR_TRACE(errCode);
                    }
                }
            }
        }
    }
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

/***************************************************************************
Summary:
Flush an audio playback channel
***************************************************************************/
void NEXUS_AudioPlayback_Flush(
    NEXUS_AudioPlaybackHandle handle
    )
{
    BSTD_UNUSED(handle);
}

#endif /* #if NEXUS_NUM_AUDIO_PLAYBACKS */

