/***************************************************************************
*  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
*   API name: AudioDecoder
*    API for audio decoder management.
*
***************************************************************************/

#include "nexus_audio_module.h"
#include "priv/nexus_audio_decoder_priv.h"

BDBG_MODULE(nexus_audio_decoder);

static NEXUS_AudioDecoder g_decoders[NEXUS_NUM_AUDIO_DECODERS];

static void NEXUS_AudioDecoder_P_Watchdog(void *pParam);
static void NEXUS_AudioDecoder_P_SampleRate(void *pParam);
static void NEXUS_AudioDecoder_P_ChannelChangeReport(void * context);
static void NEXUS_AudioDecoder_P_FirstPts_isr(void *pParam1, int param2, BAPE_DecoderTsmStatus *pTsmStatus);
static void NEXUS_AudioDecoder_P_AudioTsmFail_isr(void *pParam1, int param2, BAPE_DecoderTsmStatus *pTsmStatus);
static void NEXUS_AudioDecoder_P_AudioTsmPass_isr(void *pParam1, int param2, BAPE_DecoderTsmStatus *pTsmStatus);
static void NEXUS_AudioDecoder_P_SampleRateChange_isr(void *pParam1, int param2, BAVC_AudioSamplingRate sampleRate);
static void NEXUS_AudioDecoder_P_Lock_isr(void *pParam1, int param2);
static BERR_Code NEXUS_AudioDecoder_P_GetPtsCallback_isr(void *pContext, BAVC_PTSInfo *pPTSInfo);
static BERR_Code NEXUS_AudioDecoder_P_GetCdbLevelCallback_isr(void *pContext, unsigned *pCdbLevel);
static BERR_Code NEXUS_AudioDecoder_P_StcValidCallback_isr(void *pContext);
static void NEXUS_AudioDecoder_P_FifoWatchdog(void *context);
static void NEXUS_AudioDecoder_P_DebugLogger(void *context);
static void NEXUS_AudioDecoder_P_Watchdog_isr(void *pParam1, int param2);
static BAVC_AudioCompressionStd NEXUS_AudioDecoder_P_CodecToMagnum(NEXUS_AudioCodec codec);
static void NEXUS_AudioDecoder_P_StcChannelStatusChangeEventHandler(void *context);

#define LOCK_TRANSPORT()    NEXUS_Module_Lock(g_NEXUS_audioModuleData.settings.modules.transport)
#define UNLOCK_TRANSPORT()  NEXUS_Module_Unlock(g_NEXUS_audioModuleData.settings.modules.transport)

static BKNI_EventHandle g_watchdogEvent;
static NEXUS_EventCallbackHandle g_watchdogCallback;

static void NEXUS_AudioDecoder_P_Watchdog_isr(void *pParam1, int param2)
{
    BSTD_UNUSED(pParam1);
    BSTD_UNUSED(param2);
    BDBG_ERR(("Audio Watchdog Interrupt Received"));
    BKNI_SetEvent_isr(g_watchdogEvent);
}

NEXUS_Error NEXUS_AudioDecoder_P_Init(void)
{
    BERR_Code errCode;
    BAPE_InterruptHandlers interrupts;

    errCode = BKNI_CreateEvent(&g_watchdogEvent);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }
    g_watchdogCallback = NEXUS_RegisterEvent(g_watchdogEvent, NEXUS_AudioDecoder_P_Watchdog, NULL);
    if ( NULL == g_watchdogCallback )
    {
        BKNI_DestroyEvent(g_watchdogEvent);
        g_watchdogEvent = NULL;
        return BERR_TRACE(BERR_OS_ERROR);
    }

    BAPE_GetInterruptHandlers(NEXUS_AUDIO_DEVICE_HANDLE, &interrupts);
    interrupts.watchdog.pCallback_isr = NEXUS_AudioDecoder_P_Watchdog_isr;
    errCode = BAPE_SetInterruptHandlers(NEXUS_AUDIO_DEVICE_HANDLE, &interrupts);
    if ( errCode )
    {
        NEXUS_UnregisterEvent(g_watchdogCallback);
        g_watchdogCallback = NULL;
        BKNI_DestroyEvent(g_watchdogEvent);
        g_watchdogEvent = NULL;
        return BERR_TRACE(errCode);
    }

    return BERR_SUCCESS;
}

void NEXUS_AudioDecoder_P_Uninit(void)
{
    BAPE_InterruptHandlers interrupts;

    BAPE_GetInterruptHandlers(NEXUS_AUDIO_DEVICE_HANDLE, &interrupts);
    interrupts.watchdog.pCallback_isr = NULL;
    BAPE_SetInterruptHandlers(NEXUS_AUDIO_DEVICE_HANDLE, &interrupts);

    NEXUS_UnregisterEvent(g_watchdogCallback);
    g_watchdogCallback = NULL;
    BKNI_DestroyEvent(g_watchdogEvent);
    g_watchdogEvent = NULL;
}

/***************************************************************************
Summary:
Get default open settings for an audio decoder
***************************************************************************/
void NEXUS_AudioDecoder_GetDefaultOpenSettings(
    NEXUS_AudioDecoderOpenSettings *pSettings   /* [out] default settings */
    )
{
    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->independentDelay = g_NEXUS_audioModuleData.settings.independentDelay;
}

static void NEXUS_AudioDecoder_P_GetDefaultSettings(
    NEXUS_AudioDecoderSettings *pSettings   /* [out] default settings */
    )
{
    int i;
    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(NEXUS_AudioDecoderSettings));
    for ( i = 0; i < NEXUS_AudioChannel_eMax; i++ )
    {
        pSettings->volumeMatrix[i][i] = NEXUS_AUDIO_VOLUME_LINEAR_NORMAL;
    }
}

/***************************************************************************
Summary:
Open an audio decoder of the specified type
***************************************************************************/
NEXUS_AudioDecoderHandle NEXUS_AudioDecoder_Open( /* attr{destructor=NEXUS_AudioDecoder_Close}  */
    unsigned index,
    const NEXUS_AudioDecoderOpenSettings *pSettings   /* settings */
    )
{
    NEXUS_AudioDecoderHandle handle;
    NEXUS_AudioDecoderOpenSettings openSettings;
    NEXUS_RaveOpenSettings raveSettings;
    BAPE_DecoderOpenSettings decoderOpenSettings;
    BAPE_DecoderInterruptHandlers interrupts;
    BAPE_MixerInput mixerInput;
    BERR_Code errCode;
    unsigned j;

    if ( index == NEXUS_ANY_ID )
    {
        for ( index = 0; index < NEXUS_NUM_AUDIO_DECODERS; index++ )
        {
            if (!g_decoders[index].opened) break;
        }
    }

    if ( index >= NEXUS_NUM_AUDIO_DECODERS )
    {
        errCode=BERR_TRACE(BERR_INVALID_PARAMETER);
        return NULL;
    }

    handle = &g_decoders[index];

    /* Check if handle is already open */
    if ( handle->opened )
    {
        BDBG_ERR(("Decoder %d already opened", index));
        errCode=BERR_TRACE(BERR_INVALID_PARAMETER);
        return NULL;
    }

    if ( NULL == pSettings )
    {
        NEXUS_AudioDecoder_GetDefaultOpenSettings(&openSettings);
        pSettings = &openSettings;
    }

    /* Reset to default state */
    NEXUS_OBJECT_INIT(NEXUS_AudioDecoder, handle);
    handle->index = index;
    handle->trickMute = false;
    handle->trickForceStopped = false;
    handle->openSettings = *pSettings;

#if NEXUS_HAS_ASTM
    BKNI_Memset(&handle->astm.settings, 0, sizeof(NEXUS_AudioDecoderAstmSettings));
    BKNI_Memset(&handle->astm.status, 0, sizeof(NEXUS_AudioDecoderAstmStatus));
#endif

    for ( j = 0; j < NEXUS_AudioDecoderConnectorType_eMax; j++ )
    {
        /* Setup handle linkage */
        handle->connectors[j].objectType = NEXUS_AudioInputType_eDecoder;
        handle->connectors[j].pObjectHandle = handle;

        /* Set format per-connector */
        if ( j == NEXUS_AudioDecoderConnectorType_eStereo )
        {
            handle->connectors[j].format = NEXUS_AudioInputFormat_ePcmStereo;
        }
        else if ( j == NEXUS_AudioDecoderConnectorType_eMultichannel )
        {
            /* The correct value for this is set later if multichannel is enabled */
            handle->connectors[j].format = NEXUS_AudioInputFormat_eNone;
        }
        else
        {
            handle->connectors[j].format = NEXUS_AudioInputFormat_eCompressed;
        }

        /* Invalidate outputs */
        BKNI_Memset(&(handle->outputLists[j]), 0, sizeof(NEXUS_AudioOutputList));
    }

    NEXUS_AudioDecoder_P_TrickReset(handle);

    /* Create Events */
    errCode = BKNI_CreateEvent(&handle->sampleRateEvent);
    if ( errCode )
    {
        errCode=BERR_TRACE(errCode);
        goto err_sample_rate_event;
    }

    handle->sampleRateCallback = NEXUS_RegisterEvent(handle->sampleRateEvent, NEXUS_AudioDecoder_P_SampleRate, handle);
    if ( NULL == handle->sampleRateCallback )
    {
        errCode=BERR_TRACE(BERR_OS_ERROR);
        goto err_sample_rate_callback;
    }

    errCode = BKNI_CreateEvent(&handle->channelChangeReportEvent);
    if ( errCode )
    {
        errCode=BERR_TRACE(errCode);
        goto err_channel_change_report_event;
    }

     handle->channelChangeReportEventHandler = NEXUS_RegisterEvent(handle->channelChangeReportEvent, NEXUS_AudioDecoder_P_ChannelChangeReport, handle);
     if ( NULL == handle->channelChangeReportEventHandler )
     {
         errCode=BERR_TRACE(BERR_OS_ERROR);
         goto err_channel_change_report_event_handler;
     }

    errCode = BKNI_CreateEvent(&handle->stc.statusChangeEvent);
    if ( errCode )
    {
        errCode=BERR_TRACE(errCode);
        goto err_stc_status_change_event;
    }

    handle->stc.statusChangeEventHandler = NEXUS_RegisterEvent(handle->stc.statusChangeEvent, NEXUS_AudioDecoder_P_StcChannelStatusChangeEventHandler, handle);
    if ( NULL == handle->stc.statusChangeEventHandler )
    {
        errCode=BERR_TRACE(BERR_OS_ERROR);
        goto err_stc_status_change_event_handler;
    }

    if ( NEXUS_GetEnv("multichannel_disabled") || NEXUS_GetEnv("audio_processing_disabled") )
    {
        handle->connectors[NEXUS_AudioDecoderConnectorType_eMultichannel].format = NEXUS_AudioInputFormat_ePcmStereo;
    }
    else
    {
        switch ( pSettings->multichannelFormat )
        {
        case NEXUS_AudioMultichannelFormat_eNone:
            /* Multichannel output disabled */
            handle->connectors[NEXUS_AudioDecoderConnectorType_eMultichannel].format = NEXUS_AudioInputFormat_ePcmStereo;
            break;
        case NEXUS_AudioMultichannelFormat_e5_1:
            handle->connectors[NEXUS_AudioDecoderConnectorType_eMultichannel].format = NEXUS_AudioInputFormat_ePcm5_1;
            #if NEXUS_NUM_HDMI_OUTPUTS && 0
            /* TODO: Notify PI of 5.1 vs. 7.1 */
            pChannelSettings->sChnRBufPool.uiMaxNumOutChPairs[0] += 3;
            #endif
            break;
        #if 0 /* Not supported on any current chipsets */
        case NEXUS_AudioMultichannelFormat_e7_1:
            handle->connectors[NEXUS_AudioDecoderConnectorType_eMultichannel].format = NEXUS_AudioInputFormat_ePcm7_1;
            #if NEXUS_NUM_HDMI_OUTPUTS && 0
            /* TODO: Notify PI of 5.1 vs. 7.1 */
            pChannelSettings->sChnRBufPool.uiMaxNumOutChPairs[0] += 4;
            #endif
            break;
        #endif
        default:
            BDBG_ERR(("Unsupported multichannel audio format"));
            errCode = BERR_TRACE(BERR_INVALID_PARAMETER);
            goto err_channel;
        }
    }
    if ( pSettings->independentDelay && !g_NEXUS_audioModuleData.settings.independentDelay )
    {
        BDBG_ERR(("Independent delay must be enabled at the audio module level.  Please check NEXUS_AudioModuleSettings.independentDelay."));
        errCode = BERR_TRACE(BERR_INVALID_PARAMETER);
        goto err_channel;
    }

    if (NEXUS_GetEnv("softaudio_log"))
    {
        handle->debugLogTimer = NEXUS_ScheduleTimer(150, NEXUS_AudioDecoder_P_DebugLogger, handle);
    }

    /* Open APE decoder */
    BAPE_Decoder_GetDefaultOpenSettings(&decoderOpenSettings);
    decoderOpenSettings.independentDelay = openSettings.independentDelay;
    errCode = BAPE_Decoder_Open(NEXUS_AUDIO_DEVICE_HANDLE, index, &decoderOpenSettings, &handle->channel);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        goto err_channel;
    }

    /* Init connectors */
    BAPE_Decoder_GetStereoPcmConnector(handle->channel, &mixerInput);
    handle->connectors[NEXUS_AudioDecoderConnectorType_eStereo].port = (uint32_t)mixerInput;
    BAPE_Decoder_GetMultichannelPcmConnector(handle->channel, &mixerInput);
    handle->connectors[NEXUS_AudioDecoderConnectorType_eMultichannel].port = (uint32_t)mixerInput;
    BAPE_Decoder_GetCompressedConnector(handle->channel, &mixerInput);
    handle->connectors[NEXUS_AudioDecoderConnectorType_eCompressed].port = (uint32_t)mixerInput;

    /* Hook up decoder interrupts */
    BAPE_Decoder_GetInterruptHandlers(handle->channel, &interrupts);
    interrupts.firstPts.pCallback_isr = NEXUS_AudioDecoder_P_FirstPts_isr;
    interrupts.firstPts.pParam1 = handle;
    interrupts.tsmFail.pCallback_isr = NEXUS_AudioDecoder_P_AudioTsmFail_isr;
    interrupts.tsmFail.pParam1 = handle;
#if NEXUS_HAS_ASTM
    interrupts.tsmPass.pCallback_isr = NEXUS_AudioDecoder_P_AudioTsmPass_isr;
    interrupts.tsmPass.pParam1 = handle;
#endif
    interrupts.sampleRateChange.pCallback_isr = NEXUS_AudioDecoder_P_SampleRateChange_isr;
    interrupts.sampleRateChange.pParam1 = handle;
    interrupts.lock.pCallback_isr = NEXUS_AudioDecoder_P_Lock_isr;
    interrupts.lock.pParam1 = handle;
    interrupts.lock.param2 = true;
    interrupts.unlock.pCallback_isr = NEXUS_AudioDecoder_P_Lock_isr;
    interrupts.unlock.pParam1 = handle;
    interrupts.unlock.param2 = false;
    BAPE_Decoder_SetInterruptHandlers(handle->channel, &interrupts);

    LOCK_TRANSPORT();
    NEXUS_Rave_GetDefaultOpenSettings_priv(&raveSettings);
    UNLOCK_TRANSPORT();

    BAPE_Decoder_GetDefaultCdbItbConfig(handle->channel, &raveSettings.config);

    if ( pSettings->fifoSize == 0 )
    {
        /* NOTE: Don't automatically increase CDB/ITB for IP Settop internally. */
    }
    else
    {
        /* Make ITB proportional to CDB */
        raveSettings.config.Itb.Length = 1024*((raveSettings.config.Itb.Length/1024) * (pSettings->fifoSize/1024))/(raveSettings.config.Cdb.Length/1024);
        BDBG_ASSERT(0 != raveSettings.config.Itb.Length);
        raveSettings.config.Cdb.Length = pSettings->fifoSize;
    }

    LOCK_TRANSPORT();
    handle->raveContext = NEXUS_Rave_Open_priv(&raveSettings);
    UNLOCK_TRANSPORT();
    if ( NULL == handle->raveContext )
    {
        BDBG_ERR(("Unable to allocate RAVE context"));
        goto err_rave;
    }

    handle->sourceChangeAppCallback = NEXUS_IsrCallback_Create(handle, NULL);
    if ( NULL == handle->sourceChangeAppCallback )
    {
        goto err_app_callback;
    }

    handle->lockCallback = NEXUS_IsrCallback_Create(handle, NULL);
    if ( NULL == handle->lockCallback )
    {
        goto err_lock_callback;
    }

    handle->ptsErrorCallback = NEXUS_IsrCallback_Create(handle, NULL);
    if ( NULL == handle->ptsErrorCallback )
    {
        goto err_pts_error_callback;
    }

    handle->firstPtsCallback = NEXUS_IsrCallback_Create(handle, NULL);
    if ( NULL == handle->firstPtsCallback )
    {
        goto err_first_pts_callback;
    }

    /* Success */
    NEXUS_AudioDecoder_P_GetDefaultSettings(&handle->settings);
    (void)NEXUS_AudioDecoder_SetSettings(handle, &handle->settings);
    handle->opened = true;

    return handle;

err_first_pts_callback:
    NEXUS_IsrCallback_Destroy(handle->ptsErrorCallback);
err_pts_error_callback:
    NEXUS_IsrCallback_Destroy(handle->lockCallback);
err_lock_callback:
    NEXUS_IsrCallback_Destroy(handle->sourceChangeAppCallback);
err_app_callback:
    LOCK_TRANSPORT();
    NEXUS_Rave_Close_priv(handle->raveContext);
    UNLOCK_TRANSPORT();
err_rave:
    BAPE_Decoder_Close(handle->channel);
err_channel:
    NEXUS_UnregisterEvent(handle->stc.statusChangeEventHandler);
err_stc_status_change_event_handler:
    BKNI_DestroyEvent(handle->stc.statusChangeEvent);
err_stc_status_change_event:
    NEXUS_UnregisterEvent(handle->channelChangeReportEventHandler);
err_channel_change_report_event_handler:
    BKNI_DestroyEvent(handle->channelChangeReportEvent);
err_channel_change_report_event:
    NEXUS_UnregisterEvent(handle->sampleRateCallback);
err_sample_rate_callback:
    BKNI_DestroyEvent(handle->sampleRateEvent);
err_sample_rate_event:
    BKNI_Memset(handle, 0, sizeof(NEXUS_AudioDecoder));
    return NULL;
}

NEXUS_OBJECT_CLASS_MAKE(NEXUS_AudioDecoder, NEXUS_AudioDecoder_Close);

/***************************************************************************
Summary:
Close an audio decoder of the specified type
***************************************************************************/
static void NEXUS_AudioDecoder_P_Finalizer(
    NEXUS_AudioDecoderHandle handle
    )
{
    int i;

    NEXUS_OBJECT_ASSERT(NEXUS_AudioDecoder, handle);
    BDBG_ASSERT(true == handle->opened);

    if ( handle->started )
    {
        NEXUS_AudioDecoder_Stop(handle);
    }

    if ( handle->debugLogTimer )
    {
        NEXUS_CancelTimer(handle->debugLogTimer);
        handle->debugLogTimer = NULL;
    }

    for ( i = 0; i < NEXUS_AudioDecoderConnectorType_eMax; i++ )
    {
        if ( handle->connectors[i].pMixerData )
        {
            const char *pType;
            switch ( i )
            {
            case NEXUS_AudioDecoderConnectorType_eStereo:
                pType = "stereo";
                break;
            case NEXUS_AudioDecoderConnectorType_eCompressed:
                pType = "compressed";
                break;
            case NEXUS_AudioDecoderConnectorType_eMultichannel:
                pType = "multichannel";
                break;
            default:
                pType = "unknown";
                break;
            }
            BDBG_WRN(("Decoder Connector %p (type=%s) is still active. Calling NEXUS_AudioInput_Shutdown.", &handle->connectors[i], pType));
            NEXUS_AudioInput_Shutdown(&handle->connectors[i]);
        }
    }

    BAPE_Decoder_Close(handle->channel);

    NEXUS_IsrCallback_Destroy(handle->ptsErrorCallback);
    NEXUS_IsrCallback_Destroy(handle->lockCallback);
    NEXUS_IsrCallback_Destroy(handle->sourceChangeAppCallback);
    NEXUS_IsrCallback_Destroy(handle->firstPtsCallback);
    LOCK_TRANSPORT();
    NEXUS_Rave_Close_priv(handle->raveContext);
    UNLOCK_TRANSPORT();
    NEXUS_UnregisterEvent(handle->stc.statusChangeEventHandler);
    BKNI_DestroyEvent(handle->stc.statusChangeEvent);
    NEXUS_UnregisterEvent(handle->channelChangeReportEventHandler);
    BKNI_DestroyEvent(handle->channelChangeReportEvent);
    NEXUS_UnregisterEvent(handle->sampleRateCallback);
    BKNI_DestroyEvent(handle->sampleRateEvent);
    BKNI_Memset(handle, 0, sizeof(NEXUS_AudioDecoder));
}

/***************************************************************************
Summary:
Get Settings for an audio decoder
***************************************************************************/
void NEXUS_AudioDecoder_GetSettings(
    NEXUS_AudioDecoderHandle handle,
    NEXUS_AudioDecoderSettings *pSettings   /* [out] Settings */
    )
{
    NEXUS_OBJECT_ASSERT(NEXUS_AudioDecoder, handle);
    BDBG_ASSERT(NULL != pSettings);

    *pSettings = handle->settings;
}

/***************************************************************************
Summary:
Set Settings for an audio decoder
***************************************************************************/
NEXUS_Error NEXUS_AudioDecoder_SetSettings(
    NEXUS_AudioDecoderHandle handle,
    const NEXUS_AudioDecoderSettings *pSettings /* Settings */
    )
{
    NEXUS_OBJECT_ASSERT(NEXUS_AudioDecoder, handle);

    if ( NULL == pSettings )
    {
        NEXUS_AudioDecoder_P_GetDefaultSettings(&handle->settings);
    }
    else
    {
        if ( pSettings->fifoThreshold != handle->settings.fifoThreshold )
        {
            BERR_Code rc;

            LOCK_TRANSPORT();

            rc = NEXUS_Rave_SetCdbThreshold_priv(handle->raveContext, pSettings->fifoThreshold);

            UNLOCK_TRANSPORT();

            if (rc) return BERR_TRACE(rc);
        }

        handle->settings = *pSettings;
    }
    NEXUS_IsrCallback_Set(handle->sourceChangeAppCallback, &(handle->settings.sourceChanged));
    NEXUS_IsrCallback_Set(handle->lockCallback, &(handle->settings.lockChanged));
    return NEXUS_AudioDecoder_ApplySettings_priv(handle);
}

/***************************************************************************
Summary:
Initialize an audio decoder program structure
***************************************************************************/
void NEXUS_AudioDecoder_GetDefaultStartSettings(
    NEXUS_AudioDecoderStartSettings *pSettings /* [out] Program Defaults */
    )
{
    BKNI_Memset(pSettings, 0, sizeof(NEXUS_AudioDecoderStartSettings));
    pSettings->codec = NEXUS_AudioCodec_eAc3;
}

/***************************************************************************
Summary:
Start deocding the specified program
***************************************************************************/
NEXUS_Error NEXUS_AudioDecoder_Start(
    NEXUS_AudioDecoderHandle handle,
    const NEXUS_AudioDecoderStartSettings *pProgram    /* What to start decoding */
    )
{
    int i;
    NEXUS_Error errCode;
    bool useTsm, playback=false, playbackValid = false;
    NEXUS_PidChannelStatus pidChannelStatus;
    NEXUS_RaveSettings raveSettings;
    BAVC_StreamType streamType=BAVC_StreamType_eTsMpeg;
    BAPE_DecoderStartSettings *pStartSettings;
    BAPE_DecoderTsmSettings tsmSettings;


    BDBG_ENTER(NEXUS_AudioDecoder_Start);

    NEXUS_OBJECT_ASSERT(NEXUS_AudioDecoder, handle);
    BDBG_ASSERT(NULL != pProgram);

    if ( handle->started )
    {
        BDBG_ERR(("This decoder is already started.  Please call stop first."));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    pStartSettings = &handle->apeStartSettings;
    BAPE_Decoder_GetDefaultStartSettings(pStartSettings);

    NEXUS_AudioDecoder_P_TrickReset(handle); /* reset trick state on start */

    /* Save Program */
    handle->programSettings = *pProgram;

    /* Sanity check program */
    if ( NULL == pProgram->pidChannel )
    {
        BDBG_ERR(("No PID Channel"));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* Transport source */
    errCode = NEXUS_PidChannel_GetStatus(pProgram->pidChannel, &pidChannelStatus);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
    }

    handle->isPlayback = pidChannelStatus.playback;

    /* There are 4 types of DSS A/V streams:
    DSS SD video - this is DSS ES
    DSS HD video - this is DSS PES
    DSS MPEG audio - this actually uses MPEG1 system headers, but it's very similar to PES, therefore DSS PES
        Therefore we convert DSS ES to DSS PES here.
    DSS AC3 audio - uses MPEG2 System PES, therefore DSS PES
    */
    if (pidChannelStatus.transportType == NEXUS_TransportType_eDssEs)
    {
        pidChannelStatus.transportType = NEXUS_TransportType_eDssPes;
    }
    /* All DSS is converted to PES above */
    handle->isDss = (pidChannelStatus.transportType == NEXUS_TransportType_eDssPes)?true:false;

    /* Convert to AVC transport type */
    errCode = NEXUS_P_TransportType_ToMagnum_isrsafe(pidChannelStatus.transportType, &streamType);
    if (errCode) return BERR_TRACE(errCode);

    switch( pidChannelStatus.transportType )
    {
    case NEXUS_TransportType_eEs:
        streamType = BAVC_StreamType_eEs;
        break;
    case NEXUS_TransportType_eTs:
        streamType = BAVC_StreamType_eTsMpeg;
        break;
    case NEXUS_TransportType_eDssEs:
        streamType = BAVC_StreamType_eDssEs;
        break;
    case NEXUS_TransportType_eDssPes:
        streamType = BAVC_StreamType_eDssPes;
        break;
    case NEXUS_TransportType_eMpeg2Pes:
        streamType = BAVC_StreamType_ePes;
        break;
    case NEXUS_TransportType_eMpeg1Ps:
        streamType = BAVC_StreamType_eMpeg1System;
        break;
    default:
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    pStartSettings->streamType = streamType;

    pStartSettings->codec = NEXUS_AudioDecoder_P_CodecToMagnum(pProgram->codec);

    if (pProgram->stcChannel)
    {
        NEXUS_StcChannelSettings stcSettings;
        LOCK_TRANSPORT();
        NEXUS_StcChannel_GetIndex_priv(pProgram->stcChannel, &pStartSettings->stcIndex);
        UNLOCK_TRANSPORT();
        NEXUS_StcChannel_GetSettings(pProgram->stcChannel, &stcSettings);
        playback = (stcSettings.mode != NEXUS_StcChannelMode_ePcr);
        playbackValid = true;
    }

    /* Compute TSM details */
    BAPE_Decoder_GetTsmSettings(handle->channel, &tsmSettings);
    tsmSettings.playback = handle->isPlayback && playback; /* implictly requires stcChannel!=NULL for this to compute to true */
    tsmSettings.ptsOffset = handle->settings.ptsOffset + handle->syncSettings.delay;
    tsmSettings.thresholds.discard = (tsmSettings.playback)?30000:3000;   /* ms */
    tsmSettings.thresholds.grossAdjustment = (pidChannelStatus.originalTransportType == NEXUS_TransportType_eAvi) ? 0x30 : 0x8;
    BAPE_Decoder_SetTsmSettings(handle->channel, &tsmSettings);

    /* Determine TSM mode */
    if ( NULL == pProgram->stcChannel ||
         NULL != NEXUS_GetEnv("force_vsync") ||
         (pProgram->pidChannel && (pidChannelStatus.transportType == NEXUS_TransportType_eEs ||
                                   pidChannelStatus.originalTransportType == NEXUS_TransportType_eWav ||
                                   pidChannelStatus.originalTransportType == NEXUS_TransportType_eEs)) )
    {
        useTsm = false;
    }
    else
    {
        useTsm = true;
    }
    handle->tsmPermitted = useTsm;
    NEXUS_AudioDecoder_P_SetTsm(handle);

    if ( pProgram->pidChannel )
    {
        LOCK_TRANSPORT();
        NEXUS_Rave_GetDefaultSettings_priv(&raveSettings);
        raveSettings.pidChannel = pProgram->pidChannel;
        raveSettings.bandHold = handle->isPlayback && (playbackValid ? playback : 1); /* if stcChannel is not provided, bandHold should be set based on pidchannel flag alone */
        raveSettings.continuityCountEnabled = !handle->isPlayback;
        errCode = NEXUS_Rave_ConfigureAudio_priv(handle->raveContext, pProgram->codec, &raveSettings);
        if (errCode)
        {
            UNLOCK_TRANSPORT();
            return BERR_TRACE(errCode);
        }
        errCode = NEXUS_Rave_GetStatus_priv(handle->raveContext, &handle->raveStatus);
        UNLOCK_TRANSPORT();
        if (errCode)
        {
            errCode = BERR_TRACE(errCode);
            goto err_rave_status;
        }
        pStartSettings->pContextMap = &handle->raveStatus.xptContextMap;

        /* We're ready to start.  Build up lists of outputs to check for configuration changes. */
        errCode = NEXUS_AudioDecoder_P_Start(handle);
        if ( errCode )
        {
            errCode=BERR_TRACE(errCode);
            goto err_start;
        }
    }

    handle->started = true;

    BDBG_LEAVE(NEXUS_AudioDecoder_Start);

    /* Success */
    return BERR_SUCCESS;

err_start:
err_rave_status:
    if ( pProgram->pidChannel )
    {
        LOCK_TRANSPORT();
        NEXUS_Rave_RemovePidChannel_priv(handle->raveContext);
        UNLOCK_TRANSPORT();
    }
    else
    {
        for ( i = 0; i < NEXUS_AudioDecoderConnectorType_eMax; i++ )
        {
            NEXUS_AudioInput_P_RemoveInput(&handle->connectors[i], handle->programSettings.input);
        }
    }
    return errCode;
}

/***************************************************************************
Summary:
Stop deocding the current program
***************************************************************************/
void NEXUS_AudioDecoder_Stop(
    NEXUS_AudioDecoderHandle handle
    )
{
    NEXUS_Error errCode;

    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioDecoder);

    BDBG_ENTER(NEXUS_AudioDecoder_Stop);

    if ( !handle->started )
    {
        BDBG_ERR(("Decoder not started"));
        return;
    }

    /* Make sure trick mute is stopped before stopping */
    NEXUS_AudioDecoder_P_SetTrickMute(handle, false);

    errCode = NEXUS_AudioDecoder_P_Stop(handle, true);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
    }

    handle->running = false;
    handle->started = false;
    handle->trickForceStopped = false; /* do we need to forcedly unmute on Stop, in a way it helps if in a PIP change mode decoder is moved from trickmode on one channel to normal mode on another channel, however it hurts if one stops decoder just in order to change a PID/ audio program */

    if ( handle->programSettings.pidChannel )
    {
        LOCK_TRANSPORT();
        NEXUS_Rave_RemovePidChannel_priv(handle->raveContext);
        UNLOCK_TRANSPORT();
    }
    /* PidChannel and input are mutually exclusive - but coverity does not like the comparison */
    else if ( handle->programSettings.input )
    {
        int i;
        for ( i = 0; i < NEXUS_AudioDecoderConnectorType_eMax; i++ )
        {
            NEXUS_AudioInput_P_RemoveInput(&handle->connectors[i], handle->programSettings.input);
        }
    }

    BKNI_Memset(&handle->programSettings, 0, sizeof(handle->programSettings));

    BDBG_LEAVE(NEXUS_AudioDecoder_Stop);
}

/***************************************************************************
Summary:
Stop decoding the current program without flushing
***************************************************************************/
NEXUS_Error NEXUS_AudioDecoder_Suspend(
    NEXUS_AudioDecoderHandle handle
    )
{
    BSTD_UNUSED(handle);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

/***************************************************************************
Summary:
Resumes decoding from the suspended state
***************************************************************************/
NEXUS_Error NEXUS_AudioDecoder_Resume(
    NEXUS_AudioDecoderHandle handle
    )
{
    BSTD_UNUSED(handle);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

/***************************************************************************
Summary:
Discards all data accumulated in the decoder buffer
***************************************************************************/
NEXUS_Error NEXUS_AudioDecoder_Flush(
    NEXUS_AudioDecoderHandle handle
    )
{
    BERR_Code rc = BERR_SUCCESS;
    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioDecoder);

    if ( !handle->started || !handle->running )
    {
        return BERR_SUCCESS;
    }

    BAPE_Decoder_DisableForFlush(handle->channel);

    BDBG_ASSERT(handle->raveContext);

    if ( handle->programSettings.pidChannel )
    {
        LOCK_TRANSPORT();
        NEXUS_Rave_Disable_priv(handle->raveContext);
        NEXUS_Rave_Flush_priv(handle->raveContext);
        UNLOCK_TRANSPORT();
    }

    rc = BAPE_Decoder_Flush(handle->channel);
    if ( rc )
    {
        (void)BERR_TRACE(rc);
    }

    if ( handle->programSettings.pidChannel )
    {
        LOCK_TRANSPORT();
        NEXUS_Rave_Enable_priv(handle->raveContext);
        UNLOCK_TRANSPORT();
    }

    return rc;
}

/***************************************************************************
Summary:
    Get codec-specific decoder settings
***************************************************************************/
void NEXUS_AudioDecoder_GetCodecSettings(
    NEXUS_AudioDecoderHandle handle,
    NEXUS_AudioCodec codec,
    NEXUS_AudioDecoderCodecSettings *pSettings  /* [out] settings for specified codec */
    )
{
    BAPE_DecoderCodecSettings codecSettings;
    BAVC_AudioCompressionStd avcCodec;

    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioDecoder);
    BDBG_ASSERT(NULL != pSettings);

    avcCodec = NEXUS_AudioDecoder_P_CodecToMagnum(codec);

    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->codec = codec;

    BAPE_Decoder_GetCodecSettings(handle->channel, avcCodec, &codecSettings);
    switch ( codec )
    {
    case NEXUS_AudioCodec_eAc3:
        BDBG_CASSERT(NEXUS_AudioDecoderDolbyDrcMode_eLine == BAPE_Ac3DrcMode_eLine);
        BDBG_CASSERT(NEXUS_AudioDecoderDolbyDrcMode_eRf == BAPE_Ac3DrcMode_eRf);
        BDBG_CASSERT(NEXUS_AudioDecoderDolbyDrcMode_eCustomA == BAPE_Ac3DrcMode_eCustomA);
        BDBG_CASSERT(NEXUS_AudioDecoderDolbyDrcMode_eCustomD == BAPE_Ac3DrcMode_eCustomD);
        pSettings->codecSettings.ac3.drcMode = codecSettings.codecSettings.ac3.drcMode;
        pSettings->codecSettings.ac3.cut = codecSettings.codecSettings.ac3.drcScaleHi;
        pSettings->codecSettings.ac3.boost = codecSettings.codecSettings.ac3.drcScaleLow;
        BDBG_CASSERT(NEXUS_AudioDecoderDolbyStereoDownmixMode_eAutomatic == BAPE_Ac3StereoMode_eAuto);
        BDBG_CASSERT(NEXUS_AudioDecoderDolbyStereoDownmixMode_eDolbySurroundCompatible == BAPE_Ac3StereoMode_eLtRt);
        BDBG_CASSERT(NEXUS_AudioDecoderDolbyStereoDownmixMode_eStandard == BAPE_Ac3StereoMode_eLoRo);
        pSettings->codecSettings.ac3.stereoDownmixMode = codecSettings.codecSettings.ac3.stereoMode;
        break;
    case NEXUS_AudioCodec_eAc3Plus:
        pSettings->codecSettings.ac3Plus.drcMode = codecSettings.codecSettings.ac3Plus.drcMode;
        pSettings->codecSettings.ac3Plus.cut = codecSettings.codecSettings.ac3Plus.drcScaleHi;
        pSettings->codecSettings.ac3Plus.stereoDownmixMode = codecSettings.codecSettings.ac3Plus.stereoMode;
        pSettings->codecSettings.ac3Plus.boost = codecSettings.codecSettings.ac3Plus.drcScaleLow;
        break;
    default:
        return;
    }
}

/***************************************************************************
Summary:
    Set codec-specific decoder settings
***************************************************************************/
NEXUS_Error NEXUS_AudioDecoder_SetCodecSettings(
    NEXUS_AudioDecoderHandle handle,
    const NEXUS_AudioDecoderCodecSettings *pSettings
    )
{
    BAPE_DecoderCodecSettings codecSettings;
    BAVC_AudioCompressionStd avcCodec;
    BERR_Code errCode;

    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioDecoder);
    BDBG_ASSERT(NULL != pSettings);

    avcCodec = NEXUS_AudioDecoder_P_CodecToMagnum(pSettings->codec);

    BAPE_Decoder_GetCodecSettings(handle->channel, avcCodec, &codecSettings);
    switch ( pSettings->codec )
    {
    case NEXUS_AudioCodec_eAc3:
        BDBG_CASSERT(NEXUS_AudioDecoderDolbyDrcMode_eLine == BAPE_Ac3DrcMode_eLine);
        BDBG_CASSERT(NEXUS_AudioDecoderDolbyDrcMode_eRf == BAPE_Ac3DrcMode_eRf);
        BDBG_CASSERT(NEXUS_AudioDecoderDolbyDrcMode_eCustomA == BAPE_Ac3DrcMode_eCustomA);
        BDBG_CASSERT(NEXUS_AudioDecoderDolbyDrcMode_eCustomD == BAPE_Ac3DrcMode_eCustomD);
        codecSettings.codecSettings.ac3.drcMode = pSettings->codecSettings.ac3.drcMode;
        codecSettings.codecSettings.ac3.drcScaleHi = pSettings->codecSettings.ac3.cut;
        codecSettings.codecSettings.ac3.drcScaleLow = pSettings->codecSettings.ac3.boost;
        BDBG_CASSERT(NEXUS_AudioDecoderDolbyStereoDownmixMode_eAutomatic == BAPE_Ac3StereoMode_eAuto);
        BDBG_CASSERT(NEXUS_AudioDecoderDolbyStereoDownmixMode_eDolbySurroundCompatible == BAPE_Ac3StereoMode_eLtRt);
        BDBG_CASSERT(NEXUS_AudioDecoderDolbyStereoDownmixMode_eStandard == BAPE_Ac3StereoMode_eLoRo);
        codecSettings.codecSettings.ac3.stereoMode = pSettings->codecSettings.ac3.stereoDownmixMode;
        break;
    case NEXUS_AudioCodec_eAc3Plus:
        codecSettings.codecSettings.ac3Plus.drcMode = pSettings->codecSettings.ac3Plus.drcMode;
        codecSettings.codecSettings.ac3Plus.drcScaleHi = pSettings->codecSettings.ac3Plus.cut;
        codecSettings.codecSettings.ac3Plus.stereoMode = pSettings->codecSettings.ac3Plus.stereoDownmixMode;
        codecSettings.codecSettings.ac3Plus.drcScaleLow = pSettings->codecSettings.ac3Plus.boost;
        break;
    default:
        return BERR_SUCCESS;
    }

    errCode = BAPE_Decoder_SetCodecSettings(handle->channel, &codecSettings);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }
    return BERR_SUCCESS;
}

static unsigned NEXUS_AudioDecoder_P_GetAc3Bitrate(unsigned frameSizeCode)
{
    const unsigned bitrateTable[] = {

        32000,  /* '000000' 32 kbps  */
        32000,  /* '000001' 32 kbps  */
        40000,  /* '000010' 40 kbps  */
        40000,  /* '000011' 40 kbps  */
        48000,  /* '000100' 48 kbps  */
        48000,  /* '000101' 48 kbps  */
        56000,  /* '000110' 56 kbps  */
        56000,  /* '000111' 56 kbps  */
        64000,  /* '001000' 64 kbps  */
        64000,  /* '001001' 64 kbps  */
        80000,  /* '001010' 80 kbps  */
        80000,  /* '001011' 80 kbps  */
        96000,  /* '001100' 96 kbps  */
        96000,  /* '001101' 96 kbps  */
        112000, /* '001110' 112 kbps */
        112000, /* '001111' 112 kbps */

        128000, /* '010000' 128 kbps */
        128000, /* '010001' 128 kbps */
        160000, /* '010010' 160 kbps */
        160000, /* '010011' 160 kbps */
        192000, /* '010100' 192 kbps */
        192000, /* '010101' 192 kbps */
        224000, /* '010110' 224 kbps */
        224000, /* '010111' 224 kbps */
        256000, /* '011000' 256 kbps */
        256000, /* '011001' 256 kbps */
        320000, /* '011010' 320 kbps */
        320000, /* '011011' 320 kbps */
        384000, /* '011100' 384 kbps */
        384000, /* '011101' 384 kbps */
        448000, /* '011110' 448 kbps */
        448000, /* '011111' 448 kbps */

        512000, /* '100000' 512 kbps */
        512000, /* '100001' 512 kbps */
        576000, /* '100010' 576 kbps */
        576000, /* '100011' 576 kbps */
        640000, /* '100100' 640 kbps */
        640000  /* '100101' 640 kbps */
    };

    if ( frameSizeCode < 38 )
    {
        return bitrateTable[frameSizeCode];
    }
    else
    {
        return 0;
    }
}

/***************************************************************************
Summary:
    Get the current audio decoder status
***************************************************************************/
NEXUS_Error NEXUS_AudioDecoder_GetStatus(
    NEXUS_AudioDecoderHandle handle,
    NEXUS_AudioDecoderStatus *pStatus   /* [out] current status */
    )
{
    unsigned depth=0, size=0;
    BAPE_DecoderStatus decoderStatus;

    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioDecoder);
    BDBG_ASSERT(NULL != pStatus);

    BKNI_Memset(pStatus, 0, sizeof(NEXUS_AudioDecoderStatus));
    pStatus->started = handle->started;
    pStatus->locked = handle->locked;
    pStatus->codec = handle->programSettings.codec;
    pStatus->ptsType = NEXUS_PtsType_eInterpolatedFromInvalidPTS;
    pStatus->numFifoOverflows = handle->numFifoOverflows;
    pStatus->numFifoUnderflows = handle->numFifoUnderflows;

    if(!handle->running)
    {
        return BERR_SUCCESS;
    }

    BKNI_EnterCriticalSection();
    if ( handle->programSettings.pidChannel )
    {
        NEXUS_Rave_GetCdbBufferInfo_isr(handle->raveContext, &depth, &size);
    }
    BKNI_LeaveCriticalSection();

    pStatus->fifoDepth = depth;
    pStatus->fifoSize = size;

    pStatus->tsm = handle->tsmEnabled;
    pStatus->timebase = handle->apeStartSettings.stcIndex;
    pStatus->ptsErrorCount = handle->ptsErrorCount;
    pStatus->codec = handle->programSettings.codec;

    if ( handle->programSettings.pidChannel )
    {
        LOCK_TRANSPORT();
        NEXUS_Rave_GetAudioFrameCount_priv(handle->raveContext, &pStatus->queuedFrames);
        UNLOCK_TRANSPORT();
    }

    /* Get decoder info */
    BAPE_Decoder_GetStatus(handle->channel, &decoderStatus);
    {
        unsigned frameLength, bitrate;

        if ( decoderStatus.sampleRate == BAVC_AudioSamplingRate_eUnknown )
        {
            pStatus->sampleRate = 0;
        }
        else
        {
            pStatus->sampleRate = NEXUS_AudioModule_P_Avc2SampleRate(decoderStatus.sampleRate);
        }

        pStatus->pts = decoderStatus.tsmStatus.ptsInfo.ui32CurrentPTS;
        pStatus->ptsType = decoderStatus.tsmStatus.ptsInfo.ePTSType == BAVC_PTSType_eCoded ? NEXUS_PtsType_eCoded :
            decoderStatus.tsmStatus.ptsInfo.ePTSType == BAVC_PTSType_eInterpolatedFromValidPTS ? NEXUS_PtsType_eInterpolatedFromValidPTS : NEXUS_PtsType_eInterpolatedFromInvalidPTS;
        pStatus->ptsStcDifference = decoderStatus.tsmStatus.ptsStcDifference;

        pStatus->framesDecoded = decoderStatus.framesDecoded;
        pStatus->frameErrors = decoderStatus.frameErrors;
        pStatus->dummyFrames = decoderStatus.dummyFrames;

        /* Handle specifics per-codec */
        switch ( decoderStatus.codec )
        {
        case BAVC_AudioCompressionStd_eMpegL1:
        case BAVC_AudioCompressionStd_eMpegL2:
        case BAVC_AudioCompressionStd_eMpegL3:
            pStatus->codec = (decoderStatus.codecStatus.mpeg.layer == 3)?NEXUS_AudioCodec_eMp3:NEXUS_AudioCodec_eMpeg;
            pStatus->codecStatus.mpeg.channelMode = decoderStatus.codecStatus.mpeg.mpegChannelMode;
            switch (  decoderStatus.codecStatus.mpeg.layer )
            {
            default:
            case 1:
                pStatus->codecStatus.mpeg.layer = NEXUS_AudioMpegLayer_e1;
                pStatus->codec = NEXUS_AudioCodec_eMpeg;
                break;
            case 2:
                pStatus->codecStatus.mpeg.layer = NEXUS_AudioMpegLayer_e2;
                pStatus->codec = NEXUS_AudioCodec_eMpeg;
                break;
            case 3:
                pStatus->codecStatus.mpeg.layer = NEXUS_AudioMpegLayer_e3;
                pStatus->codec = NEXUS_AudioCodec_eMp3;
                break;
            }
            pStatus->codecStatus.mpeg.emphasis = decoderStatus.codecStatus.mpeg.emphasisMode;
            pStatus->codecStatus.mpeg.original = decoderStatus.codecStatus.mpeg.original;
            pStatus->codecStatus.mpeg.bitrate = decoderStatus.codecStatus.mpeg.bitRate;
            if (  pStatus->sampleRate > 0 )
            {
                /* MPEG audio uses a CDB sync, so the frame count is bogus.  Calculate based on frame size and CDB depth */
                bitrate = (decoderStatus.codecStatus.mpeg.bitRate>0)?1000*decoderStatus.codecStatus.mpeg.bitRate:128000;
                if ( decoderStatus.codecStatus.mpeg.layer == 1 )
                {
                    frameLength = (48*bitrate)/pStatus->sampleRate;
                }
                else
                {
                    frameLength = (144*bitrate)/pStatus->sampleRate;
                }
                if ( frameLength == 0 )
                {
                    pStatus->queuedFrames = 0;
                }
                else
                {
                    pStatus->queuedFrames = pStatus->fifoDepth/frameLength;
                }
                BDBG_MSG(("Queued Frames %d bitrate %d index %d framelength %d samplerate %d", pStatus->queuedFrames, bitrate, decoderStatus.codecStatus.mpeg.bitRateCode, frameLength, pStatus->sampleRate));
            }
            break;
        case BAVC_AudioCompressionStd_eAacLoas:
        case BAVC_AudioCompressionStd_eAacAdts:
        case BAVC_AudioCompressionStd_eAacPlusLoas:
        case BAVC_AudioCompressionStd_eAacPlusAdts:
            /* Convert channel mode to aac acmod */
            switch ( decoderStatus.codecStatus.aac.channelMode )
            {
            case BAPE_ChannelMode_e1_0: pStatus->codecStatus.aac.acmod = NEXUS_AudioAacAcmod_eOneCenter_1_0_C; break;
            case BAPE_ChannelMode_e1_1: pStatus->codecStatus.aac.acmod = NEXUS_AudioAacAcmod_eTwoMono_1_ch1_ch2; break;
            case BAPE_ChannelMode_e2_0: pStatus->codecStatus.aac.acmod = NEXUS_AudioAacAcmod_eTwoChannel_2_0_L_R; break;
            case BAPE_ChannelMode_e3_0: pStatus->codecStatus.aac.acmod = NEXUS_AudioAacAcmod_eThreeChannel_3_0_L_C_R; break;
            case BAPE_ChannelMode_e2_1: pStatus->codecStatus.aac.acmod = NEXUS_AudioAacAcmod_eThreeChannel_2_1_L_R_S; break;
            case BAPE_ChannelMode_e3_1: pStatus->codecStatus.aac.acmod = NEXUS_AudioAacAcmod_eFourChannel_3_1_L_C_R_S; break;
            case BAPE_ChannelMode_e2_2: pStatus->codecStatus.aac.acmod = NEXUS_AudioAacAcmod_eFourChannel_2_2_L_R_SL_SR; break;
            case BAPE_ChannelMode_e3_2: pStatus->codecStatus.aac.acmod = NEXUS_AudioAacAcmod_eFiveChannel_3_2_L_C_R_SL_SR; break;
            default:                    pStatus->codecStatus.aac.acmod = NEXUS_AudioAacAcmod_eMax; break;
            }
            pStatus->codecStatus.aac.profile = decoderStatus.codecStatus.aac.profile;
            pStatus->codecStatus.aac.lfe = decoderStatus.codecStatus.aac.lfe;
            pStatus->codecStatus.aac.pseudoSurround = decoderStatus.codecStatus.aac.pseudoSurround;
            pStatus->codecStatus.aac.drc = decoderStatus.codecStatus.aac.drc;
            pStatus->codecStatus.aac.bitrate = decoderStatus.codecStatus.aac.bitRate;
            break;
        case BAVC_AudioCompressionStd_eAc3:
        case BAVC_AudioCompressionStd_eAc3Plus:
            pStatus->codec = (decoderStatus.codec == BAVC_AudioCompressionStd_eAc3)?NEXUS_AudioCodec_eAc3:NEXUS_AudioCodec_eAc3Plus;
            pStatus->codecStatus.ac3.acmod = decoderStatus.codecStatus.ac3.acmod;
            pStatus->codecStatus.ac3.frameSizeCode = decoderStatus.codecStatus.ac3.frameSizeCode;
            pStatus->codecStatus.ac3.lfe = decoderStatus.codecStatus.ac3.lfe;
            pStatus->codecStatus.ac3.copyright = decoderStatus.codecStatus.ac3.copyright;
            pStatus->codecStatus.ac3.bitrate = NEXUS_AudioDecoder_P_GetAc3Bitrate(decoderStatus.codecStatus.ac3.frameSizeCode);
            break;
        default:
            /* No specifics for this codec */
            break;
        }
    }

    return NEXUS_SUCCESS;
}

/***************************************************************************
Summary:
    Get raw channel status information from the decoder

Description:
    When the decoder is connected to a digital input, this routine can
    return the raw channel status bit information from the input device.
    Currently, this applies to HDMI or SPDIF inputs only.  This routine
    will return an error if not connected to a digital input.

See Also:
    NEXUS_SpdifOutput_SetRawChannelStatus
***************************************************************************/
NEXUS_Error NEXUS_AudioDecoder_GetRawChannelStatus(
    NEXUS_AudioDecoderHandle handle,
    NEXUS_AudioRawChannelStatus *pStatus   /* [out] current status */
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pStatus);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

/***************************************************************************
Summary:
    Get an audio connector for use in the audio mixer
***************************************************************************/
NEXUS_AudioInput NEXUS_AudioDecoder_GetConnector( /* attr{shutdown=NEXUS_AudioInput_Shutdown} */
    NEXUS_AudioDecoderHandle handle,
    NEXUS_AudioDecoderConnectorType type
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioDecoder);
    BDBG_ASSERT(type < NEXUS_AudioDecoderConnectorType_eMax);
    return &handle->connectors[type];
}

NEXUS_Error NEXUS_AudioDecoder_ApplySettings_priv(
    NEXUS_AudioDecoderHandle handle
    )
{
    BAPE_DecoderTsmSettings tsmSettings;
    BAPE_DecoderSettings decoderSettings;
    BAPE_MixerInputVolume volume;
    unsigned i,j;
    BERR_Code errCode;

    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioDecoder);

    BAPE_Decoder_GetTsmSettings(handle->channel, &tsmSettings);
    tsmSettings.ptsOffset = handle->settings.ptsOffset + handle->syncSettings.delay;
    BAPE_Decoder_SetTsmSettings(handle->channel, &tsmSettings);

    /* TODO: Handle output mode when multichannel is supported */
    BAPE_Decoder_GetSettings(handle->channel, &decoderSettings);
    switch ( handle->settings.dualMonoMode )
    {
    default:
    case NEXUS_AudioDecoderDualMonoMode_eStereo:
        decoderSettings.dualMonoMode = BAPE_DualMonoMode_eStereo;
        break;
    case NEXUS_AudioDecoderDualMonoMode_eLeft:
        decoderSettings.dualMonoMode = BAPE_DualMonoMode_eLeft;
        break;
    case NEXUS_AudioDecoderDualMonoMode_eRight:
        decoderSettings.dualMonoMode = BAPE_DualMonoMode_eRight;
        break;
    case NEXUS_AudioDecoderDualMonoMode_eMix:
        decoderSettings.dualMonoMode = BAPE_DualMonoMode_eMix;
        break;
    }
    BAPE_Decoder_SetSettings(handle->channel, &decoderSettings);

    /* Apply volume settings */
    NEXUS_AudioInput_P_GetVolume(&handle->connectors[NEXUS_AudioDecoderConnectorType_eStereo], &volume);
    for ( i = 0; i < NEXUS_AudioChannel_eMax; i++ )
    {
        for ( j = 0; j < NEXUS_AudioChannel_eMax; j++ )
        {
            volume.coefficients[i][j] = handle->settings.volumeMatrix[j][i];
        }
    }
    volume.muted = handle->settings.muted;
    errCode = NEXUS_AudioInput_P_SetVolume(&handle->connectors[NEXUS_AudioDecoderConnectorType_eStereo], &volume);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }
    errCode = NEXUS_AudioInput_P_SetVolume(&handle->connectors[NEXUS_AudioDecoderConnectorType_eMultichannel], &volume);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    return BERR_SUCCESS;
}

#if NEXUS_HAS_ASTM
void NEXUS_AudioDecoder_GetAstmSettings_priv(
    NEXUS_AudioDecoderHandle audioDecoder,
    NEXUS_AudioDecoderAstmSettings * pAstmSettings  /* [out] */
    )
{
    BSTD_UNUSED(audioDecoder);
    BSTD_UNUSED(pAstmSettings);
}

NEXUS_Error NEXUS_AudioDecoder_SetAstmSettings_priv(
    NEXUS_AudioDecoderHandle audioDecoder,
    const NEXUS_AudioDecoderAstmSettings * pAstmSettings
    )
{
    BSTD_UNUSED(audioDecoder);
    BSTD_UNUSED(pAstmSettings);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

NEXUS_Error NEXUS_AudioDecoder_GetAstmStatus_isr(
    NEXUS_AudioDecoderHandle audioDecoder,
    NEXUS_AudioDecoderAstmStatus * pAstmStatus  /* [out] */
    )
{
    BSTD_UNUSED(audioDecoder);
    BSTD_UNUSED(pAstmStatus);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}
#endif

void NEXUS_AudioDecoder_GetSyncSettings_priv(
    NEXUS_AudioDecoderHandle audioDecoder,
    NEXUS_AudioInputSyncSettings *pSyncSettings  /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(audioDecoder, NEXUS_AudioDecoder);
    BDBG_ASSERT(NULL != pSyncSettings);
    *pSyncSettings = audioDecoder->syncSettings;
}

NEXUS_Error NEXUS_AudioDecoder_SetSyncSettings_priv(
    NEXUS_AudioDecoderHandle audioDecoder,
    const NEXUS_AudioInputSyncSettings *pSyncSettings
    )
{
    BDBG_OBJECT_ASSERT(audioDecoder, NEXUS_AudioDecoder);
    BDBG_ASSERT(NULL != pSyncSettings);
    audioDecoder->syncSettings = *pSyncSettings;
    return NEXUS_AudioDecoder_ApplySettings_priv(audioDecoder);
}

NEXUS_Error NEXUS_AudioDecoder_GetSyncStatus_isr(
    NEXUS_AudioDecoderHandle audioDecoder,
    NEXUS_AudioInputSyncStatus *pSyncStatus  /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(audioDecoder, NEXUS_AudioDecoder);
    BDBG_ASSERT(NULL != pSyncStatus);
    BKNI_Memset(pSyncStatus, 0, sizeof(*pSyncStatus));
    pSyncStatus->started = audioDecoder->running;
    pSyncStatus->digital = true;
    pSyncStatus->dsolaEnabled = false;
    pSyncStatus->nonRealTime = false;
    BAPE_GetDecoderPathDelay(NEXUS_AUDIO_DEVICE_HANDLE, &pSyncStatus->delay);
    /*
     * for 64 sample FMM delay -> this gets us within 0 - 1 ms, since the FMM
     * delay really depends on Fs. 32K -> 2 ms, 44.1K -> 1.45 ms, 48K -> 1.33 ms
     */
    pSyncStatus->delay += 1;
    return BERR_SUCCESS;
}

bool NEXUS_AudioDecoder_P_IsRunning(NEXUS_AudioDecoderHandle handle)
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioDecoder);
    return handle->started;
}

static bool NEXUS_AudioDecoder_P_PrintDebugLog(void)
{
    char line[128];
    unsigned i;
    for(i=0;i<8;i++)
    {
        BERR_Code rc = BAPE_GetDebugLog(NEXUS_AUDIO_DEVICE_HANDLE, line, sizeof(line));
        if(rc!=BERR_SUCCESS) { return false; }
        if(*line=='\0') { return false; }
        BKNI_Printf("$$$ AUART %s", line);
    }
    return true;
}


static void NEXUS_AudioDecoder_P_Watchdog(void *pParam)
{
    unsigned i;

    BSTD_UNUSED(pParam);
    BDBG_WRN(("Audio Watchdog Handler Invoked"));

    for(i=0;i<256;i++)
    {
        if(!NEXUS_AudioDecoder_P_PrintDebugLog())
        {
            break;
        }
    }

    LOCK_TRANSPORT();
    for ( i = 0; i < NEXUS_NUM_AUDIO_DECODERS; i++ )
    {
        if ( g_decoders[i].opened && g_decoders[i].running && NULL == g_decoders[i].programSettings.input )
        {
            NEXUS_Rave_Disable_priv(g_decoders[i].raveContext);
            NEXUS_Rave_Flush_priv(g_decoders[i].raveContext);
        }
    }
    UNLOCK_TRANSPORT();

    /* Process watchdog event */
    BAPE_ProcessWatchdogInterrupt(NEXUS_AUDIO_DEVICE_HANDLE);

    /* Restart RAVE contexts */
    LOCK_TRANSPORT();
    for ( i = 0; i < NEXUS_NUM_AUDIO_DECODERS; i++ )
    {
        if ( g_decoders[i].opened && g_decoders[i].running && NULL == g_decoders[i].programSettings.input )
        {
            NEXUS_Rave_Enable_priv(g_decoders[i].raveContext);
        }
    }
    UNLOCK_TRANSPORT();

#if NEXUS_HAS_ASTM
    for ( i = 0; i < NEXUS_NUM_AUDIO_DECODERS; i++ )
    {
        if ( g_decoders[i].opened && g_decoders[i].running && g_decoders[i].astm.settings.enableAstm )
        {
            NEXUS_Callback astm_watchdog_isr = g_decoders[i].astm.settings.watchdog_isr;
            BDBG_MSG(("Audio channel %p is notifying ASTM of its watchdog recovery", &g_decoders[i]));
            if (astm_watchdog_isr)
            {
                BKNI_EnterCriticalSection();
                astm_watchdog_isr(g_decoders[i].astm.settings.callbackContext, 0);
                BKNI_LeaveCriticalSection();
            }
        }
    }
#endif
}

static void NEXUS_AudioDecoder_P_SampleRate(void *pParam)
{
    NEXUS_AudioDecoder *pDecoder = (NEXUS_AudioDecoder *)pParam;

    #if 0 /* TODO */
    if(pDecoder->settings.wideGaThreshold) {
        BRAP_DSPCHN_TsmDebugInfo info;
        uint32_t new_threshold;

        BRAP_GetTsmDebugInfo(pDecoder->rapChannel, &info);

        new_threshold = (info.sPtsInfo.i32TSMUpperThreshold*3)/2;

        BDBG_WRN(("Adjusting GA threshold to: %d (was %d)", new_threshold, info.sPtsInfo.i32TSMUpperThreshold));

        BRAP_SetTsmGAThreshold(pDecoder->rapChannel, new_threshold/45);
    }
    #else
    BSTD_UNUSED(pDecoder);
    #endif

    BDBG_MSG(("Sample Rate decoder[%d]", pDecoder->index));
}

static void NEXUS_AudioDecoder_P_ChannelChangeReport(void * context)
{
    BSTD_UNUSED(context);
}

static void NEXUS_AudioDecoder_P_FirstPts_isr(void *pParam1, int param2, BAPE_DecoderTsmStatus *pTsmStatus)
{
    NEXUS_Error errCode;
    NEXUS_AudioDecoder *pDecoder = (NEXUS_AudioDecoder *)pParam1;

    BSTD_UNUSED(param2);

    BDBG_MSG(("audio[%d] First PTS %08x", pDecoder->index, pTsmStatus->ptsInfo.ui32CurrentPTS));
    if ( pDecoder->programSettings.stcChannel && pDecoder->stc.connector)
    {
        NEXUS_IsrCallback_Fire_isr(pDecoder->firstPtsCallback);
        errCode = NEXUS_StcChannel_RequestStc_isr(pDecoder->stc.connector, &pTsmStatus->ptsInfo);
        if ( errCode )
        {
            errCode=BERR_TRACE(errCode);
        }
    }
}

static void NEXUS_AudioDecoder_P_AudioTsmFail_isr(void *pParam1, int param2, BAPE_DecoderTsmStatus *pTsmStatus)
{
    NEXUS_AudioDecoder *pDecoder = (NEXUS_AudioDecoder *)pParam1;
    NEXUS_Error errCode;

    uint32_t stc;

    BSTD_UNUSED(param2);
    BDBG_ASSERT(NULL != pTsmStatus);

    BDBG_MSG(("audio[%d] Tsm fail: PTS 0x%08x, type %d",
        pDecoder->index, pTsmStatus->ptsInfo.ui32CurrentPTS, pTsmStatus->ptsInfo.ePTSType));

    NEXUS_StcChannel_GetStc_isr(pDecoder->programSettings.stcChannel, &stc);

    /* If we're in a non-standard STC trick mode */
    if ( pDecoder->trickState.stcTrickEnabled && pDecoder->trickState.rate != 800 && pDecoder->trickState.rate != 1200 )
    {
        /* in STC trick mode, PTS might lag the STC because of decoder drop algorithm. don't reset STC in this case. */
        if ( stc > pTsmStatus->ptsInfo.ui32CurrentPTS && stc - pTsmStatus->ptsInfo.ui32CurrentPTS < 45000 * 8 )
        {
            return;
        }
    }

    BDBG_MSG(("pts2stcphase: %d", pTsmStatus->ptsStcDifference));

#if NEXUS_HAS_ASTM
    if (pDecoder->astm.settings.enableAstm && pDecoder->astm.settings.syncLimit > 0)
    {
        pTsmStatus->ptsInfo.ui32CurrentPTS = (uint32_t)((int32_t)stc - pTsmStatus->ptsStcDifference);
    }
#endif /* NEXUS_HAS_ASTM */

    /* PR:52308 ignore PTS errors for non-XPT inputs - we can't do anything about them from stcchannel/pcrlib anyway */
    if (!pDecoder->programSettings.input)
    {
        NEXUS_IsrCallback_Fire_isr(pDecoder->ptsErrorCallback);
        if (pDecoder->stc.connector)
        {
            errCode = NEXUS_StcChannel_PtsError_isr(pDecoder->stc.connector, &pTsmStatus->ptsInfo);
            if (errCode)
            {
                errCode=BERR_TRACE(errCode);
                /* keep going */
            }
        }
    }

#if NEXUS_HAS_ASTM
    if (pDecoder->astm.settings.enableAstm)
    {
        pDecoder->astm.status.pts = pTsmStatus->ptsInfo.ui32CurrentPTS;

        if (pDecoder->astm.settings.tsmFail_isr)
        {
            pDecoder->astm.settings.tsmFail_isr(pDecoder->astm.settings.callbackContext, 0);
        }
    }
#endif /* NEXUS_HAS_ASTM */

    pDecoder->ptsErrorCount++;
}

static void NEXUS_AudioDecoder_P_AudioTsmPass_isr(void *pParam1, int param2, BAPE_DecoderTsmStatus *pTsmStatus)
{
    NEXUS_AudioDecoder *pDecoder = (NEXUS_AudioDecoder *)pParam1;

    BSTD_UNUSED(param2);
    BDBG_ASSERT(NULL != pTsmStatus);

    BDBG_MSG(("audio[%d] TSM pass: PTS 0x%08x, type %d",
        pDecoder->index, pTsmStatus->ptsInfo.ui32CurrentPTS, pTsmStatus->ptsInfo.ePTSType));

    if (pDecoder->astm.settings.enableAstm)
    {
        pDecoder->astm.status.pts = pTsmStatus->ptsInfo.ui32CurrentPTS;

        if (pDecoder->astm.settings.tsmPass_isr)
        {
            pDecoder->astm.settings.tsmPass_isr(pDecoder->astm.settings.callbackContext, 0);
        }
    }
}

static void NEXUS_AudioDecoder_P_SampleRateChange_isr(void *pParam1, int param2, BAVC_AudioSamplingRate sampleRate)
{
    NEXUS_AudioDecoder *pDecoder = (NEXUS_AudioDecoder *)pParam1;

    BSTD_UNUSED(param2);

    BDBG_MSG(("audio[%d] Sampling Rate Info, samplingRate=%x", pDecoder->index, sampleRate));

    BKNI_SetEvent_isr(pDecoder->sampleRateEvent);
    NEXUS_IsrCallback_Fire_isr(pDecoder->sourceChangeAppCallback);

    if (pDecoder->syncSettings.sampleRateCallback_isr) {
        (*pDecoder->syncSettings.sampleRateCallback_isr)(pDecoder->syncSettings.callbackContext, 0);
    }

}

static void NEXUS_AudioDecoder_P_Lock_isr(void *pParam1, int param2)
{
    NEXUS_AudioDecoder *pDecoder = (NEXUS_AudioDecoder *)pParam1;

    pDecoder->locked = param2;
    BDBG_MSG(("Decoder %u %s", pDecoder->index, param2?"lock":"unlock"));
    NEXUS_IsrCallback_Fire_isr(pDecoder->sourceChangeAppCallback);
}

void NEXUS_AudioDecoder_P_SetTsm(NEXUS_AudioDecoderHandle handle)
{
    BAPE_DecoderTsmSettings tsmSettings;

    BAPE_Decoder_GetTsmSettings(handle->channel, &tsmSettings);
    tsmSettings.tsmEnabled = false;

    if ( handle->tsmPermitted )
    {
        if ( handle->trickState.tsmEnabled )
        {
#if NEXUS_HAS_ASTM
            if ( handle->astm.settings.enableAstm )
            {
                BDBG_MSG(("ASTM is %s TSM for audio channel %p", handle->astm.settings.enableTsm ? "enabling" : "disabling", handle));
                if ( handle->astm.settings.enableTsm )
                {
                    tsmSettings.tsmEnabled = true;
                }
            }
            else
#endif
            {
                tsmSettings.tsmEnabled = true;
            }
        }
    }

    BDBG_MSG(("%s TSM",tsmSettings.tsmEnabled?"Enabling":"Disabling"));

#if NEXUS_HAS_ASTM
    /* Only allow ASTM if we have TSM enabled for all non-ASTM controls and we have a TS source */
    if ( handle->tsmPermitted && handle->trickState.tsmEnabled &&
         handle->apeStartSettings.streamType == BAVC_StreamType_eTsMpeg )
    {
        BDBG_MSG(("%s ASTM mode for audio channel %p",handle->astm.settings.enableAstm?"Enabling":"Disabling", handle));
        tsmSettings.astmEnabled = handle->astm.settings.enableAstm;
    }
    else
    {
        BDBG_MSG(("Disabling ASTM mode for audio channel %p", handle));
        tsmSettings.astmEnabled = false;
    }
#endif

    BAPE_Decoder_SetTsmSettings(handle->channel, &tsmSettings);

    handle->tsmEnabled = tsmSettings.tsmEnabled;
}

static void NEXUS_AudioDecoder_P_StopStcChannel(NEXUS_AudioDecoderHandle handle)
{
    LOCK_TRANSPORT();
    if (handle->programSettings.pidChannel)
    {
        NEXUS_StcChannel_DisablePidChannel_priv(handle->programSettings.stcChannel, handle->programSettings.pidChannel);
    }
    if (handle->stc.connector)
    {
        NEXUS_StcChannel_DisconnectDecoder_priv(handle->stc.connector);
        handle->stc.connector = NULL;
    }
    UNLOCK_TRANSPORT();
}

static NEXUS_Error NEXUS_AudioDecoder_P_StartStcChannel(NEXUS_AudioDecoderHandle handle)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_StcChannelDecoderConnectionSettings stcChannelConnectionSettings;
    NEXUS_AudioDecoderStartSettings * pProgram;

    pProgram = &handle->programSettings;

    LOCK_TRANSPORT();

    NEXUS_StcChannel_GetDefaultDecoderConnectionSettings_priv(&stcChannelConnectionSettings);
    stcChannelConnectionSettings.type = NEXUS_StcChannelDecoderType_eAudio;
    stcChannelConnectionSettings.priority = handle->stc.priority;
    stcChannelConnectionSettings.getPts_isr = NEXUS_AudioDecoder_P_GetPtsCallback_isr;
    stcChannelConnectionSettings.getCdbLevel_isr = NEXUS_AudioDecoder_P_GetCdbLevelCallback_isr;
    stcChannelConnectionSettings.stcValid_isr = NEXUS_AudioDecoder_P_StcValidCallback_isr;
    stcChannelConnectionSettings.pCallbackContext = handle;
    stcChannelConnectionSettings.statusUpdateEvent = handle->stc.statusChangeEvent;
    handle->stc.connector = NEXUS_StcChannel_ConnectDecoder_priv(pProgram->stcChannel, &stcChannelConnectionSettings);
    if (!handle->stc.connector) { UNLOCK_TRANSPORT(); rc = BERR_TRACE(NEXUS_NOT_SUPPORTED); goto err; }
    if (pProgram->pidChannel)
    {
        NEXUS_StcChannel_EnablePidChannel_priv(pProgram->stcChannel, pProgram->pidChannel);
    }

    UNLOCK_TRANSPORT();

err:
    return rc;
}

NEXUS_Error NEXUS_AudioDecoder_P_Start(NEXUS_AudioDecoderHandle handle)
{
    NEXUS_Error errCode;
    const NEXUS_AudioDecoderStartSettings *pProgram;

    BDBG_ASSERT(NULL != handle);

    BDBG_ENTER(NEXUS_AudioDecoder_P_Start);

    pProgram = &handle->programSettings;

    if ( handle->programSettings.input || !handle->started || handle->outputLists[NEXUS_AudioDecoderConnectorType_eCompressed].outputs[0] != NULL )
    {
        int i;
        bool changed=false;
        NEXUS_AudioOutputList outputLists[NEXUS_AudioDecoderConnectorType_eMax];

        /* Get output lists */
        for ( i = 0; i < NEXUS_AudioDecoderConnectorType_eMax; i++ )
        {
            NEXUS_AudioInput_P_GetOutputs(&handle->connectors[i], &outputLists[i], false);
            /* Check for changes */
            if ( BKNI_Memcmp(&outputLists[i], &handle->outputLists[i], sizeof(NEXUS_AudioOutputList)) )
            {
                BDBG_MSG(("Output change detected for connector type %d", i));
                changed = true;
            }
            else
            {
                BDBG_MSG(("NO Output change detected for connector type %d (firstOld=%p, firstNew=%p)", i, outputLists[i].outputs[0], handle->outputLists[i].outputs[0]));
            }
        }

        /* Determine mode to add new outputs */
        if ( outputLists[NEXUS_AudioDecoderConnectorType_eCompressed].outputs[0] != NULL &&
             outputLists[NEXUS_AudioDecoderConnectorType_eStereo].outputs[0] == NULL &&
             outputLists[NEXUS_AudioDecoderConnectorType_eMultichannel].outputs[0] == NULL )
        {
            /* Compressed output, but no stereo or multichannel.  This is a passthrough channel. */
            if ( handle->connectors[NEXUS_AudioDecoderConnectorType_eCompressed].format == NEXUS_AudioInputFormat_eCompressed )
            {
                BDBG_MSG(("Decoder %d is a passthrough channel", handle->index));
            }
            else
            {
                BDBG_MSG(("Decoder %d is a decode channel (PCM data for passthrough)", handle->index));
            }

            handle->stc.priority = 1;
        }
        else
        {
            if ( handle->descriptorParent )
            {
                /* AD child.  This is decoder type audio2 */
                handle->stc.priority = 2;
            }
            else
            {
                /* Standard decode channel.  Compressed will be simul */
                handle->stc.priority = 0;
            }
            /* Determine decoder mode as decode or simul based on presence of compressed outputs */
            if ( outputLists[NEXUS_AudioDecoderConnectorType_eCompressed].outputs[0] != NULL )
            {
                /* Simul Mode */
                BDBG_MSG(("Decoder %d is a simul channel", handle->index));
            }
            else
            {
                /* Decode Mode */
                BDBG_MSG(("Decoder %d is a decode channel", handle->index));
            }
        }

        /* This process is slow and involves memory allocation/free.  Only do it if something changed */
        if ( changed )
        {
            /* Remove all outputs */
            for ( i = 0; i < NEXUS_AudioDecoderConnectorType_eMax; i++ )
            {
                int j;

                handle->hdmiOutput = NULL;  /* Invalidate this, it will be re-discovered when ports are added */
                for ( j = 0; j < NEXUS_AUDIO_MAX_OUTPUTS; j++ )
                {
                    NEXUS_AudioOutputHandle output = handle->outputLists[i].outputs[j];
                    if ( output )
                    {
                        BDBG_MSG(("Removing output port %d from decoder %d", output->port, handle->index));
                        /* Clear this output port */
                        handle->outputLists[i].outputs[j] = NULL;
                    }
                }
            }

            /* Now, actually attach outputs */
            for ( i = 0; i < NEXUS_AUDIO_MAX_OUTPUTS; i++ )
            {
                NEXUS_AudioOutputHandle output = outputLists[NEXUS_AudioDecoderConnectorType_eStereo].outputs[i];
                if ( NULL == output )
                {
                    break;
                }
                else
                {
                    BDBG_MSG(("Adding output port %d to decoder %d (pcm) %p", output->port, handle->index, output));
                    handle->outputLists[NEXUS_AudioDecoderConnectorType_eStereo].outputs[i] = output;
                    if ( output->objectType == NEXUS_AudioOutputType_eHdmi )
                    {
                        handle->hdmiOutput = output;
                    }
                }
            }

            /* TODO: Add multichannel support correctly... Need 3 ports, somehow coordinated with AudioOutput */
            for ( i = 0; i < NEXUS_AUDIO_MAX_OUTPUTS; i++ )
            {
                NEXUS_AudioOutputHandle output = outputLists[NEXUS_AudioDecoderConnectorType_eMultichannel].outputs[i];
                if ( NULL == output )
                {
                    break;
                }
                else
                {
                    BDBG_MSG(("Adding output port %d to decoder %d (multichannel)", output->port, handle->index));
                    handle->outputLists[NEXUS_AudioDecoderConnectorType_eMultichannel].outputs[i] = output;
                    if ( output->objectType == NEXUS_AudioOutputType_eHdmi )
                    {
                        handle->hdmiOutput = output;
                    }
                }
            }

            /* Add compressed ports */
            for ( i = 0; i < NEXUS_AUDIO_MAX_OUTPUTS; i++ )
            {
                NEXUS_AudioOutputHandle output = outputLists[NEXUS_AudioDecoderConnectorType_eCompressed].outputs[i];
                if ( NULL == output )
                {
                    break;
                }
                else
                {
                    BDBG_MSG(("Adding output port %d to decoder %d (compressed) %p", output->port, handle->index, output));
                    handle->outputLists[NEXUS_AudioDecoderConnectorType_eCompressed].outputs[i] = output;
                    if ( output->objectType == NEXUS_AudioOutputType_eHdmi )
                    {
                        handle->hdmiOutput = output;
                    }
                }
            }
        }

        if ( NULL == outputLists[NEXUS_AudioDecoderConnectorType_eStereo].outputs[0] &&
             NULL == outputLists[NEXUS_AudioDecoderConnectorType_eMultichannel].outputs[0] &&
             NULL == outputLists[NEXUS_AudioDecoderConnectorType_eCompressed].outputs[0] )
        {
            BDBG_ERR(("No outputs have been connected to this decoder."));
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }

        /* Notify downstream objects we're about to start - should be done after outputs are connected on 7400/7401 */
        /* This will trigger connections on the 7405/3563/... */
        for ( i = 0; i < NEXUS_AudioDecoderConnectorType_eMax; i++ )
        {
            /* Only call this for inputs about to actually start */
            if ( outputLists[i].outputs[0] != NULL )
            {
                BDBG_MSG(("Preparing to start path %d", i));

                NEXUS_AudioInput_P_PrepareToStart(&handle->connectors[i]);
            }
        }
    }

    /* Setup StcChannel */
    if ( pProgram->stcChannel )
    {
        errCode = NEXUS_AudioDecoder_P_StartStcChannel(handle);
        if (errCode) { errCode = BERR_TRACE(errCode); goto err_set_tsm; }

        if ( pProgram->pidChannel && NULL == handle->fifoWatchdog.timer )
        {
            handle->fifoWatchdog.timer = NEXUS_ScheduleTimer(250, NEXUS_AudioDecoder_P_FifoWatchdog, handle);
        }
    }

    /* After all that, we're ready to go.  Start. */
    handle->ptsErrorCount = 0;
    BDBG_MSG(("Starting Decoder %d", handle->index));

    /* Setup proper decoder output mode */
    /* TODO: Set output mode */

    /* Re-apply settings (?) */
    NEXUS_AudioDecoder_ApplySettings_priv(handle);
    BDBG_MSG(("Starting with codec %u", handle->apeStartSettings.codec));

    handle->locked = false;
    handle->numFifoOverflows = handle->numFifoUnderflows = 0;
    errCode = BAPE_Decoder_Start(handle->channel, &handle->apeStartSettings);
    if ( errCode && !handle->started ) { errCode = BERR_TRACE(errCode); goto err_dec_start; }

    if ( handle->programSettings.pidChannel )
    {
        LOCK_TRANSPORT();
        NEXUS_Rave_Enable_priv(handle->raveContext);
        UNLOCK_TRANSPORT();
    }
    handle->running = true;

    if ( handle->programSettings.pidChannel )
    {
        (void)NEXUS_PidChannel_ConsumerStarted(handle->programSettings.pidChannel);
    }

    /* PR:49294 sync/astm must know that audio has been started *every* time it is started */
    if (handle->syncSettings.startCallback_isr) {
        BKNI_EnterCriticalSection();
        (*handle->syncSettings.startCallback_isr)(handle->syncSettings.callbackContext, 0);
        BKNI_LeaveCriticalSection();
    }

#if NEXUS_HAS_ASTM
    handle->astm.status.started = true;

    if (handle->astm.settings.enableAstm)
    {
        BDBG_MSG(("Audio channel %p is notifying ASTM of its start action", handle));
        if (handle->astm.settings.lifecycle_isr)
        {
            BKNI_EnterCriticalSection();
            (*handle->astm.settings.lifecycle_isr)(handle->astm.settings.callbackContext, 0);
            BKNI_LeaveCriticalSection();
        }
    }

#endif

    BDBG_LEAVE(NEXUS_AudioDecoder_P_Start);

    return BERR_SUCCESS;

err_dec_start:
    if (handle->fifoWatchdog.timer)
    {
        NEXUS_CancelTimer(handle->fifoWatchdog.timer);
        handle->fifoWatchdog.timer = NULL;
    }
err_set_tsm:
    if (handle->programSettings.stcChannel)
    {
        NEXUS_AudioDecoder_P_StopStcChannel(handle);
    }
    return errCode;
}

NEXUS_Error NEXUS_AudioDecoder_P_Stop(NEXUS_AudioDecoderHandle handle, bool flush)
{
    BDBG_ENTER(NEXUS_AudioDecoder_P_Stop);

    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioDecoder);

    if ( handle->running )
    {
        if ( handle->fifoWatchdog.timer )
        {
            NEXUS_CancelTimer(handle->fifoWatchdog.timer);
            handle->fifoWatchdog.timer = NULL;
        }

        handle->locked = false;
        BAPE_Decoder_Stop(handle->channel);

        if ( handle->programSettings.pidChannel && flush )
        {
            LOCK_TRANSPORT();
            NEXUS_Rave_Disable_priv(handle->raveContext);
            NEXUS_Rave_Flush_priv(handle->raveContext);
            UNLOCK_TRANSPORT();
        }

        handle->running = false;

        if ( handle->programSettings.stcChannel)
        {
            NEXUS_AudioDecoder_P_StopStcChannel(handle);
        }

        /* PR:49294 sync/astm must know that audio has been started *every* time it is started */
        if (handle->syncSettings.startCallback_isr) {
            BKNI_EnterCriticalSection();
            (*handle->syncSettings.startCallback_isr)(handle->syncSettings.callbackContext, 0);
            BKNI_LeaveCriticalSection();
        }

#if NEXUS_HAS_ASTM
        handle->astm.status.started = false;

        if (handle->astm.settings.enableAstm)
        {
            BDBG_MSG(("Audio channel %p is notifying ASTM of its stop action", handle));
            if (handle->astm.settings.lifecycle_isr)
            {
                BKNI_EnterCriticalSection();
                (*handle->astm.settings.lifecycle_isr)(handle->astm.settings.callbackContext, 0);
                BKNI_LeaveCriticalSection();
            }
        }

#endif
    }

    BDBG_LEAVE(NEXUS_AudioDecoder_P_Stop);

    return BERR_SUCCESS;
}

NEXUS_Error NEXUS_AudioDecoder_P_SetTrickMute(NEXUS_AudioDecoderHandle handle, bool muted)
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(muted);

    /* TODO -- tricky now */

    return BERR_SUCCESS;
}

static BERR_Code NEXUS_AudioDecoder_P_GetPtsCallback_isr(void *pContext, BAVC_PTSInfo *pPTSInfo)
{
    NEXUS_AudioDecoderHandle handle = pContext;
    BAPE_DecoderTsmStatus tsmStatus;
    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioDecoder);
    BDBG_ASSERT(NULL != pPTSInfo);
    BAPE_Decoder_GetTsmStatus_isr(handle->channel, &tsmStatus);
    *pPTSInfo = tsmStatus.ptsInfo;
    return BERR_SUCCESS;
}

static BERR_Code NEXUS_AudioDecoder_P_GetCdbLevelCallback_isr(void *pContext, unsigned *pCdbLevel)
{
    unsigned depth=0, size=0;
    NEXUS_AudioDecoderHandle audioDecoder = (NEXUS_AudioDecoderHandle)pContext;
    BDBG_OBJECT_ASSERT(audioDecoder, NEXUS_AudioDecoder);
    if ( audioDecoder->programSettings.pidChannel )
    {
        NEXUS_Rave_GetCdbBufferInfo_isr(audioDecoder->raveContext, &depth, &size);
    }
    BDBG_MSG(("GetCdbLevel - returned %d", depth));
    *pCdbLevel = depth;
    return 0;
}

static BERR_Code NEXUS_AudioDecoder_P_StcValidCallback_isr(void *pContext)
{
    NEXUS_AudioDecoderHandle audioDecoder = (NEXUS_AudioDecoderHandle)pContext;
    BDBG_OBJECT_ASSERT(audioDecoder, NEXUS_AudioDecoder);
    return BAPE_Decoder_SetStcValid_isr(audioDecoder->channel);
}

static void NEXUS_AudioDecoder_P_FifoWatchdog(void *context)
{
    NEXUS_AudioDecoderHandle audioDecoder = context;
    unsigned timeout=150;

    audioDecoder->fifoWatchdog.timer = NULL;

    if (audioDecoder->programSettings.stcChannel)
    {
        uint32_t cdbValidPointer, cdbReadPointer;
        unsigned depth, size;

        BKNI_EnterCriticalSection();
        NEXUS_Rave_GetCdbPointers_isr(audioDecoder->raveContext, &cdbValidPointer, &cdbReadPointer);
        NEXUS_Rave_GetCdbBufferInfo_isr(audioDecoder->raveContext, &depth, &size);
        BKNI_LeaveCriticalSection();
        if (audioDecoder->fifoWatchdog.lastCdbValidPointer == cdbValidPointer && audioDecoder->fifoWatchdog.lastCdbReadPointer == cdbReadPointer) {
            if (audioDecoder->fifoWatchdog.staticCount < 20) {
                audioDecoder->fifoWatchdog.staticCount++;
            }
        }
        else {
            audioDecoder->fifoWatchdog.staticCount = 0;
            audioDecoder->fifoWatchdog.lastCdbValidPointer = cdbValidPointer;
            audioDecoder->fifoWatchdog.lastCdbReadPointer = cdbReadPointer;
        }

        if (audioDecoder->stc.connector)
        {
            LOCK_TRANSPORT();
            NEXUS_StcChannel_GetDefaultDecoderFifoWatchdogStatus_priv(&audioDecoder->fifoWatchdog.status);
            audioDecoder->fifoWatchdog.status.isHung = (audioDecoder->fifoWatchdog.staticCount > 4) && (audioDecoder->trickState.rate >= NEXUS_NORMAL_DECODE_RATE);
            audioDecoder->fifoWatchdog.status.percentFull = size ? depth * 100 / size : 0;
            audioDecoder->fifoWatchdog.status.frameSyncUnlocked = audioDecoder->locked;
            NEXUS_StcChannel_ReportDecoderHang_priv(audioDecoder->stc.connector, &audioDecoder->fifoWatchdog.status);
            UNLOCK_TRANSPORT();
        }
    }

    audioDecoder->fifoWatchdog.timer = NEXUS_ScheduleTimer(timeout, NEXUS_AudioDecoder_P_FifoWatchdog, audioDecoder);
}

static void NEXUS_AudioDecoder_P_StcChannelStatusChangeEventHandler(void *context)
{
    NEXUS_AudioDecoderHandle audioDecoder = context;
    NEXUS_StcChannelDecoderConnectionStatus status;

    if (!audioDecoder->stc.connector) return;

    LOCK_TRANSPORT();
    NEXUS_StcChannel_GetDecoderConnectionStatus_priv(audioDecoder->stc.connector, &status);
    UNLOCK_TRANSPORT();

    if (status.flush || status.zeroFill)
    {
        /* reset count if we are doing something about it */
        audioDecoder->fifoWatchdog.staticCount = 0;
    }

    /* if both flush and zero-fill are set, flush first */

    if (status.flush) {
        BDBG_WRN(("Audio FIFO watchdog flush"));
        NEXUS_AudioDecoder_Flush(audioDecoder);
        LOCK_TRANSPORT();
        NEXUS_StcChannel_ReportDecoderFlush_priv(audioDecoder->stc.connector);
        UNLOCK_TRANSPORT();
    }

    /* this is a one-shot, like a flush */
    if (status.zeroFill)
    {
        BDBG_WRN(("audio %p: audio gap zero fill unimplemented", audioDecoder));
#if 0
        BAPE_Decoder_EnterUnderflowMode(audioDecoder->channel);
        LOCK_TRANSPORT();
        NEXUS_StcChannel_ReportDecoderZeroFill_priv(audioDecoder->stc.connector);
        UNLOCK_TRANSPORT();
#endif
    }
}

static void NEXUS_AudioDecoder_P_DebugLogger(void *context)
{
    NEXUS_AudioDecoderHandle audioDecoder = context;
    unsigned timeout=150;

    audioDecoder->debugLogTimer = NULL;

    if (NEXUS_AudioDecoder_P_PrintDebugLog())
    {
        timeout=10;
    }

    audioDecoder->debugLogTimer = NEXUS_ScheduleTimer(timeout, NEXUS_AudioDecoder_P_DebugLogger, audioDecoder);
}

static BAVC_AudioCompressionStd NEXUS_AudioDecoder_P_CodecToMagnum(NEXUS_AudioCodec codec)
{
    switch ( codec )
    {
    case NEXUS_AudioCodec_eMpeg:
        /* Really, L1,L2,L3 are equivalent */
        return BAVC_AudioCompressionStd_eMpegL2;
    case NEXUS_AudioCodec_eMp3:
        return BAVC_AudioCompressionStd_eMpegL3;
    case NEXUS_AudioCodec_eAc3:
        return BAVC_AudioCompressionStd_eAc3;
    case NEXUS_AudioCodec_eAc3Plus:
        return BAVC_AudioCompressionStd_eAc3Plus;
    case NEXUS_AudioCodec_eAacAdts:
        return BAVC_AudioCompressionStd_eAacAdts;
    case NEXUS_AudioCodec_eAacLoas:
        return BAVC_AudioCompressionStd_eAacLoas;
    case NEXUS_AudioCodec_eAacPlusLoas:
        return BAVC_AudioCompressionStd_eAacPlusLoas;
    case NEXUS_AudioCodec_eAacPlusAdts:
        return BAVC_AudioCompressionStd_eAacPlusAdts;
    case NEXUS_AudioCodec_eDts:
        return BAVC_AudioCompressionStd_eDts;
    case NEXUS_AudioCodec_eWmaStd:
        return BAVC_AudioCompressionStd_eWmaStd;
    case NEXUS_AudioCodec_eWmaPro:
        return BAVC_AudioCompressionStd_eWmaPro;
    case NEXUS_AudioCodec_ePcm:
        return BAVC_AudioCompressionStd_ePcm;
    case NEXUS_AudioCodec_ePcmWav:
        return BAVC_AudioCompressionStd_ePcmWav;
    case NEXUS_AudioCodec_eLpcmDvd:
        return BAVC_AudioCompressionStd_eLpcmDvd;
    case NEXUS_AudioCodec_eLpcmHdDvd:
        return BAVC_AudioCompressionStd_eLpcmHdDvd;
    case NEXUS_AudioCodec_eLpcmBluRay:
        return BAVC_AudioCompressionStd_eLpcmBd;
    case NEXUS_AudioCodec_eAmr:
        return BAVC_AudioCompressionStd_eAmr;
    case NEXUS_AudioCodec_eDra:
        return BAVC_AudioCompressionStd_eDra;
    default:
        return BAVC_AudioCompressionStd_eMax;
    }
}

NEXUS_Error NEXUS_AudioModule_Standby_priv(bool enabled, const NEXUS_StandbySettings *pSettings)
{
#if NEXUS_POWER_MANAGEMENT
    BERR_Code rc;
    unsigned i;

    BSTD_UNUSED(pSettings);

    for(i=0 ; i<NEXUS_NUM_AUDIO_DECODERS; i++) {
    NEXUS_AudioDecoderHandle handle = &g_decoders[i];
    if ( handle->started ) {
        BDBG_WRN(("Audio decoder %d needs to be stopped before entering standby", i));
        return NEXUS_NOT_SUPPORTED;
    }
    }

    if(enabled) {
    rc = BAPE_Standby(g_NEXUS_audioModuleData.apeHandle);
    } else {
    rc = BAPE_Resume(g_NEXUS_audioModuleData.apeHandle);
    }

    return rc;
#else
    BSTD_UNUSED(enabled);
    BSTD_UNUSED(pSettings);
    return NEXUS_SUCCESS;
#endif
}

NEXUS_Error NEXUS_AudioDecoder_GetExtendedStatus(
    NEXUS_AudioDecoderHandle handle,
    NEXUS_AudioDecoderExtendedStatus *pStatus /* [out] */
    )
{
    BERR_Code errCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioDecoder);
    BDBG_ASSERT(NULL != pStatus);
    LOCK_TRANSPORT();
    if ( handle->raveContext )
    {
        NEXUS_RaveStatus raveStatus;

        errCode = NEXUS_Rave_GetStatus_priv(handle->raveContext, &raveStatus);
        if ( errCode == BERR_SUCCESS )
        {
            pStatus->raveIndex = raveStatus.index;
        }
    }
    UNLOCK_TRANSPORT();

    return errCode;
}

void NEXUS_AudioDecoder_IsCodecSupported(
    NEXUS_AudioDecoderHandle handle,
    NEXUS_AudioCodec codec,
    bool *pSupported
    )
{
    NEXUS_AudioCapabilities caps;

    BSTD_UNUSED(handle);
    BDBG_ASSERT(codec < NEXUS_AudioCodec_eMax);
    BDBG_ASSERT(NULL != pSupported);

    NEXUS_GetAudioCapabilities(&caps);
    *pSupported = caps.dsp.codecs[codec].decode;
}

void NEXUS_AudioDecoder_IsPassthroughCodecSupported(
    NEXUS_AudioDecoderHandle handle,
    NEXUS_AudioCodec codec,
    bool *pSupported
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED (codec);
    BDBG_ASSERT(NULL != pSupported);
    *pSupported = true;
    return;
}

NEXUS_Error NEXUS_AudioDecoder_GetAncillaryDataBuffer(
    NEXUS_AudioDecoderHandle handle,
    void **pBuffer,   /* [out] attr{memory=cached} pointer to ancillary data */
    size_t *pSize     /* [out] number of bytes of userdata */
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pBuffer);
    BSTD_UNUSED(pSize);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

void NEXUS_AudioDecoder_AncillaryDataReadComplete(
    NEXUS_AudioDecoderHandle handle,
    size_t size   /* number of bytes of userdata consumed by the application */
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(size);
}

void NEXUS_AudioDecoder_FlushAncillaryData(
    NEXUS_AudioDecoderHandle handle
    )
{
    BSTD_UNUSED(handle);
}

NEXUS_Error NEXUS_AudioDecoder_GetPresentationStatus(
    NEXUS_AudioDecoderHandle handle,
    unsigned presentationIndex,
    NEXUS_AudioDecoderPresentationStatus *pStatus
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(presentationIndex);
    BSTD_UNUSED(pStatus);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

#if NEXUS_HAS_SIMPLE_DECODER
void NEXUS_AudioDecoder_GetSimpleSettings_priv(
    NEXUS_AudioDecoderHandle audioDecoder,
    NEXUS_AudioDecoderSimpleSettings *pSimpleSettings  /* [out] */
    )
{
    BSTD_UNUSED(audioDecoder);
    BERR_TRACE(NEXUS_NOT_SUPPORTED);
    BKNI_Memset(pSimpleSettings, 0, sizeof(NEXUS_AudioDecoderSimpleSettings));
}

NEXUS_Error NEXUS_AudioDecoder_SetSimpleSettings_priv(
    NEXUS_AudioDecoderHandle audioDecoder,
    const NEXUS_AudioDecoderSimpleSettings *pSimpleSettings
    )
{
    BSTD_UNUSED(audioDecoder);
    BSTD_UNUSED(pSimpleSettings);
    return BERR_TRACE(NEXUS_NOT_SUPPORTED);
}
#endif
