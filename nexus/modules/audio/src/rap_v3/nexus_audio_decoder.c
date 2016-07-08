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
#include "nexus_pid_channel.h"        /* PidChannelDesc */
#include "nexus_timebase.h"     /* Timebase */
#include "nexus_stc_channel.h"   /* StcChannel */
#include "nexus_audio_priv.h"
#include "priv/nexus_pid_channel_priv.h"

#if NEXUS_NUM_HDMI_INPUTS
#include "priv/nexus_hdmi_input_priv.h"
#endif
#if NEXUS_HAS_DIGITAL_AUDIO_INPUTS
#include "bacm_spdif.h"
#endif

#if NEXUS_NUM_AUDIO_DECODERS

BDBG_MODULE(nexus_audio_decoder);

BTRC_MODULE(ChnChange_DecodeStopAudio, ENABLE);
BTRC_MODULE(ChnChange_DecodeStartAudio, ENABLE);
BTRC_MODULE(ChnChange_DecodeStartAudioRAP, ENABLE);
BTRC_MODULE(ChnChange_DecodeFirstAudio, ENABLE);
BTRC_MODULE(ChnChange_SyncUnmuteAudio, ENABLE);
BTRC_MODULE(ChnChange_Total_Audio, ENABLE);

#if B_HAS_TRC
static const BTRC_Module *audio_btrc_modules[] = {
    BTRC_MODULE_HANDLE(ChnChange_DecodeStopAudio),
    BTRC_MODULE_HANDLE(ChnChange_DecodeStartAudio),
    BTRC_MODULE_HANDLE(ChnChange_DecodeStartAudioRAP),
    BTRC_MODULE_HANDLE(ChnChange_DecodeFirstAudio),
    BTRC_MODULE_HANDLE(ChnChange_SyncUnmuteAudio),
    BTRC_MODULE_HANDLE(ChnChange_Total_Audio)
};

#define AUDIO_BTRC_N_MODULES ((sizeof(audio_btrc_modules)/sizeof(*audio_btrc_modules)))
#endif


#if BRAP_VER >= 4
/* Static array defining CDB and ITB sizes for various Buffer Configuration Modes */
#if (BSTD_CPU_ENDIAN==BSTD_ENDIAN_BIG)
static const BAVC_CdbItbConfig sCdbItbCfg[] =
{
    /* BRAP_BufferCfgMode_eAdvAudioMode */
    { { 256 * 1024, 4, false }, { 128 * 1024, 4, false }, false },
    /* Populate this array as more new modes gets added */
};
#else
static const BAVC_CdbItbConfig sCdbItbCfg[] =
{
    /* BRAP_BufferCfgMode_eAdvAudioMode */
    { { 256 * 1024, 4, true }, { 128 * 1024, 4, false }, false },
    /* Populate this array as more new modes gets added */
};
#endif
#endif

#define NEXUS_HAS_AUDIO_TSM_LOG_SUPPORT 0

static BKNI_EventHandle g_watchdogEvent;
static NEXUS_EventCallbackHandle g_watchdogCallback;
static unsigned g_numWatchdogs;

#define LOCK_TRANSPORT()    NEXUS_Module_Lock(g_NEXUS_audioModuleData.transport)
#define UNLOCK_TRANSPORT()  NEXUS_Module_Unlock(g_NEXUS_audioModuleData.transport)

NEXUS_AudioDecoder g_decoders[NEXUS_NUM_AUDIO_DECODERS];

static void NEXUS_AudioDecoder_P_FirstPts_isr(void *pParam1, int param2, void *pData);
static void NEXUS_AudioDecoder_P_AudioTsmFail_isr(void *pParam1, int param2, void *pData);
#if NEXUS_HAS_AUDIO_TSM_LOG_SUPPORT
#include <stdio.h>
FILE *g_pTsmLogFiles[NEXUS_NUM_AUDIO_DECODERS];
static void NEXUS_AudioDecoder_P_AudioTsmLog_isr(void *pParam1, int param2, void *pData);
#endif
#if NEXUS_HAS_ASTM
static void NEXUS_AudioDecoder_P_AudioTsmPass_isr(void *pParam1, int param2, void *pData);
#endif
static void NEXUS_AudioDecoder_P_SampleRateChange_isr(void *pParam1, int param2, void *pData);
static void NEXUS_AudioDecoder_P_StatusChange_isr(void *pParam1, int param2, void *pData);
static void NEXUS_AudioDecoder_P_LayerChange_isr(void *pParam1, int param2, void *pData);
static void NEXUS_AudioDecoder_P_CdbItbError_isr(void *pParam1, int param2, void *pData);
static BERR_Code NEXUS_AudioDecoder_P_GetPtsCallback_isr(void *pContext, BAVC_PTSInfo *pPTSInfo);
static BERR_Code NEXUS_AudioDecoder_P_GetCdbLevelCallback_isr(void *pContext, unsigned *pCdbLevel);
static BERR_Code NEXUS_AudioDecoder_P_StcValidCallback_isr(void *pContext);
static void NEXUS_AudioDecoder_P_StcChannelStatusChangeEventHandler(void *context);

static void NEXUS_AudioDecoder_P_Watchdog(void *pContext);
static void NEXUS_AudioDecoder_P_LayerChange(void *pParam);
static void NEXUS_AudioDecoder_P_SourceChange(void *pParam);
static void NEXUS_AudioDecoder_P_SampleRate(void *pParam);
static NEXUS_Error NEXUS_AudioDecoder_P_ConnectionChange(void *pParam, NEXUS_AudioInput input);
static void NEXUS_AudioDecoder_P_ChannelChangeReport(void * context);
static bool NEXUS_AudioDecoder_P_CodecSupportsMultichannel(NEXUS_AudioCodec codec);
static bool NEXUS_AudioDecoder_P_CodecIsMono(NEXUS_AudioCodec codec);
static void NEXUS_AudioDecoder_P_SetOutputMode(NEXUS_AudioDecoderHandle handle);
static void NEXUS_AudioDecoder_P_StatusTimer(void *pParam);
static NEXUS_Error NEXUS_AudioDecoder_P_SetCompressedSyncMute(NEXUS_AudioDecoderHandle decoder, bool muted);

#if NEXUS_HAS_DIGITAL_AUDIO_INPUTS
static BKNI_EventHandle g_spdifFormatEvent;
static NEXUS_EventCallbackHandle g_spdifFormatCallback;
static NEXUS_AudioDecoderHandle g_spdifDecoder;
static void NEXUS_AudioDecoder_P_SpdifFormatChanged_isr(void *pParam1, int param2, void *pData);
static void NEXUS_AudioDecoder_P_SpdifFormatChanged(void *pContext);
#endif

static NEXUS_Error NEXUS_AudioDecoder_P_SetupCodec(NEXUS_AudioDecoderHandle handle, BRAP_DSPCHN_AudioType codec, BAVC_StreamType streamType);
static void NEXUS_AudioDecoder_P_SetupMixingCoefs(NEXUS_AudioDecoderHandle handle, BRAP_MixingCoef *pCoefs);
static bool NEXUS_AudioDecoder_P_FindDts(NEXUS_AudioDecoderHandle handle);
static bool NEXUS_AudioDecoder_P_FindBtsc(NEXUS_AudioDecoderHandle handle);

static NEXUS_Error NEXUS_AudioDecoder_P_SetPtsOffset(NEXUS_AudioDecoderHandle audioDecoder);

#if NEXUS_NUM_AUDIO_MIXERS
static bool NEXUS_AudioDecoder_P_IsMixedByDspMixer(NEXUS_AudioDecoderHandle handle);
#endif

static void NEXUS_AudioDecoder_P_Watchdog_isr(void *pParam, int iParam, void *pData)
{
    BKNI_EventHandle event = (BKNI_EventHandle)pParam;
    BSTD_UNUSED(iParam);
    BSTD_UNUSED(pData);
    BDBG_ERR(("Raptor watchdog interrupt fired"));
    BKNI_SetEvent(event);
}


/***************************************************************************
Summary:
    Init the audio decoder module
See Also:
    NEXUS_AudioDecoder_Uninit
 ***************************************************************************/
NEXUS_Error NEXUS_AudioDecoder_P_Init(void)
{
    NEXUS_Error errCode;

    BDBG_MSG(("Decoder Module Init"));

    g_numWatchdogs = 0;
    errCode = BKNI_CreateEvent(&g_watchdogEvent);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    g_watchdogCallback = NEXUS_RegisterEvent(g_watchdogEvent, NEXUS_AudioDecoder_P_Watchdog, NULL);
    if ( NULL == g_watchdogCallback )
    {
        errCode=BERR_TRACE(errCode);
        goto err_wd_cb;
    }

    if ( !NEXUS_GetEnv("no_watchdog") )
    {
        errCode = BRAP_InstallDeviceLevelAppInterruptCb(g_NEXUS_audioModuleData.hRap,
                                                        BRAP_DeviceLevelInterrupt_eWatchdog,
                                                        NEXUS_AudioDecoder_P_Watchdog_isr,
                                                        g_watchdogEvent,
                                                        0);
        if ( errCode )
        {
            errCode=BERR_TRACE(errCode);
            goto err_wd_isr;
        }
    }

    #if NEXUS_HAS_DIGITAL_AUDIO_INPUTS
    errCode = BKNI_CreateEvent(&g_spdifFormatEvent);
    if ( errCode )
    {
        errCode=BERR_TRACE(errCode);
        goto err_spdif_event;
    }
    g_spdifFormatCallback = NEXUS_RegisterEvent(g_spdifFormatEvent, NEXUS_AudioDecoder_P_SpdifFormatChanged, NULL);
    if ( NULL == g_spdifFormatCallback )
    {
        BERR_TRACE(BERR_OS_ERROR);
        goto err_spdif_cb;
    }
    errCode = BRAP_InstallDeviceLevelAppInterruptCb(g_NEXUS_audioModuleData.hRap,
                                                    BRAP_DeviceLevelInterrupt_eSpdifRx,
                                                    NEXUS_AudioDecoder_P_SpdifFormatChanged_isr,
                                                    NULL,
                                                    0);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        goto err_spdif_isr;
    }
    #endif

    /* initialize global decoder data */
    BKNI_Memset(&g_decoders, 0, sizeof(g_decoders));

    return BERR_SUCCESS;

#if NEXUS_HAS_DIGITAL_AUDIO_INPUTS
err_spdif_isr:
    NEXUS_UnregisterEvent(g_spdifFormatCallback);
err_spdif_cb:
    BKNI_DestroyEvent(g_spdifFormatEvent);
err_spdif_event:
    BRAP_RemoveDeviceLevelAppInterruptCb(g_NEXUS_audioModuleData.hRap,
                                         BRAP_DeviceLevelInterrupt_eWatchdog);
#endif

err_wd_isr:
    NEXUS_UnregisterEvent(g_watchdogCallback);
err_wd_cb:
    BKNI_DestroyEvent(g_watchdogEvent);

    return BERR_TRACE(errCode);
}

/***************************************************************************
Summary:
    Uninit the audio decoder module
See Also:
    NEXUS_AudioDecoder_Init
 ***************************************************************************/
void NEXUS_AudioDecoder_P_Uninit(void)
{
    int i;

    BRAP_RemoveDeviceLevelAppInterruptCb(g_NEXUS_audioModuleData.hRap,
                                         BRAP_DeviceLevelInterrupt_eWatchdog);
    #if NEXUS_HAS_DIGITAL_AUDIO_INPUTS
    BRAP_RemoveDeviceLevelAppInterruptCb(g_NEXUS_audioModuleData.hRap,
                                         BRAP_DeviceLevelInterrupt_eSpdifRx);
    NEXUS_UnregisterEvent(g_spdifFormatCallback);
    BKNI_DestroyEvent(g_spdifFormatEvent);
    #endif

    for ( i = 0; i < NEXUS_NUM_AUDIO_DECODERS; i++ )
    {
        if ( g_decoders[i].opened )
        {
            if ( g_decoders[i].started )
            {
                NEXUS_AudioDecoder_Stop(&g_decoders[i]);
            }
            NEXUS_AudioDecoder_Close(&g_decoders[i]);
        }
    }

    NEXUS_UnregisterEvent(g_watchdogCallback);
    BKNI_DestroyEvent(g_watchdogEvent);
}

/***************************************************************************
Summary:
    Get default settings for an audio decoder
See Also:
    NEXUS_AudioDecoder_Open
 ***************************************************************************/
void NEXUS_AudioDecoder_GetDefaultOpenSettings(
    NEXUS_AudioDecoderOpenSettings *pSettings   /* [out] default settings */
    )
{
    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->independentDelay = g_NEXUS_audioModuleData.moduleSettings.independentDelay;
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
    pSettings->descriptionRampTime = 1000;
}

/***************************************************************************
Summary:
    Open an audio decoder of the specified type
See Also:
    NEXUS_AudioDecoder_Close
 ***************************************************************************/
NEXUS_AudioDecoderHandle NEXUS_AudioDecoder_Open(
    unsigned index,                               /* which decoder */
    const NEXUS_AudioDecoderOpenSettings *pSettings   /* settings */
    )
{
    BRAP_ChannelSettings *pChannelSettings;
    NEXUS_RaveOpenSettings raveSettings;
    NEXUS_AudioDecoder *pDecoder=NULL;
    NEXUS_AudioDecoderOpenSettings openSettings;
    NEXUS_Error errCode;
    int j;

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

    pDecoder = &g_decoders[index];
    /* Check if handle is already open */
    if ( pDecoder->opened )
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
    NEXUS_OBJECT_INIT(NEXUS_AudioDecoder, pDecoder);
    pDecoder->index = index;
    pDecoder->trickMute = false;
    pDecoder->trickForceStopped = false;
    pDecoder->compressedFormat = NEXUS_AudioInputFormat_eCompressed;
    pDecoder->openSettings = *pSettings;

    /* TODO: the following should not be necessary, with the OBJECT_INIT doing memset above */
#if NEXUS_HAS_SYNC_CHANNEL
    pDecoder->sync.mute = false;
    pDecoder->sync.startMuted = false;
    BKNI_Memset(&pDecoder->sync.settings, 0, sizeof(NEXUS_AudioInputSyncSettings));
#endif

#if NEXUS_HAS_ASTM
    BKNI_Memset(&pDecoder->astm.settings, 0, sizeof(NEXUS_AudioDecoderAstmSettings));
    BKNI_Memset(&pDecoder->astm.status, 0, sizeof(NEXUS_AudioDecoderAstmStatus));
#endif

    for ( j = 0; j < NEXUS_AudioDecoderConnectorType_eMax; j++ )
    {
        /* Setup handle linkage */
        NEXUS_AUDIO_INPUT_INIT(&pDecoder->connectors[j], NEXUS_AudioInputType_eDecoder, pDecoder);
        pDecoder->connectors[j].disconnectCb = NEXUS_AudioDecoder_P_ConnectionChange;

        /* Set format per-connector */
        if ( j == NEXUS_AudioDecoderConnectorType_eStereo )
        {
            pDecoder->connectors[j].format = NEXUS_AudioInputFormat_ePcmStereo;
        }
        else if ( j == NEXUS_AudioDecoderConnectorType_eMultichannel )
        {
            /* The correct value for this is set later if multichannel is enabled */
            pDecoder->connectors[j].format = NEXUS_AudioInputFormat_eNone;
        }
        else if ( j == NEXUS_AudioDecoderConnectorType_eMono )
        {
            /* Not Valid */
            pDecoder->connectors[j].format = NEXUS_AudioInputFormat_eNone;
        }
        else
        {
            pDecoder->connectors[j].format = NEXUS_AudioInputFormat_eCompressed;
        }

        /* Invalidate outputs */
        BKNI_Memset(&(pDecoder->outputLists[j]), 0, sizeof(NEXUS_AudioOutputList));
    }

    NEXUS_AudioDecoder_P_TrickReset(pDecoder);

    /* Create Events */
    errCode = BKNI_CreateEvent(&pDecoder->sampleRateEvent);
    if ( errCode )
    {
        errCode=BERR_TRACE(errCode);
        goto err_sample_rate_event;
    }

    pDecoder->sampleRateCallback = NEXUS_RegisterEvent(pDecoder->sampleRateEvent, NEXUS_AudioDecoder_P_SampleRate, pDecoder);
    if ( NULL == pDecoder->sampleRateCallback )
    {
        errCode=BERR_TRACE(BERR_OS_ERROR);
        goto err_sample_rate_callback;
    }

    errCode = BKNI_CreateEvent(&pDecoder->sourceChangedEvent);
    if ( errCode )
    {
        errCode=BERR_TRACE(errCode);
        goto err_source_changed_event;
    }

    pDecoder->sourceChangeCallback = NEXUS_RegisterEvent(pDecoder->sourceChangedEvent, NEXUS_AudioDecoder_P_SourceChange, pDecoder);
    if ( NULL == pDecoder->sourceChangeCallback )
    {
        errCode=BERR_TRACE(BERR_OS_ERROR);
        goto err_source_change_callback;
    }

    errCode = BKNI_CreateEvent(&pDecoder->layerChangeEvent);
    if ( errCode )
    {
        errCode=BERR_TRACE(errCode);
        goto err_layer_change_event;
    }

    pDecoder->layerChangeCallback = NEXUS_RegisterEvent(pDecoder->layerChangeEvent, NEXUS_AudioDecoder_P_LayerChange, pDecoder);
    if ( NULL == pDecoder->layerChangeCallback )
    {
        errCode=BERR_TRACE(BERR_OS_ERROR);
        goto err_layer_callback;
    }

    errCode = BKNI_CreateEvent(&pDecoder->channelChangeReportEvent);
    if ( errCode )
     {
         errCode=BERR_TRACE(errCode);
         goto err_channel_change_report_event;
     }

     pDecoder->channelChangeReportEventHandler = NEXUS_RegisterEvent(pDecoder->channelChangeReportEvent, NEXUS_AudioDecoder_P_ChannelChangeReport, pDecoder);
     if ( NULL == pDecoder->channelChangeReportEventHandler )
     {
         errCode=BERR_TRACE(BERR_OS_ERROR);
         goto err_channel_change_report_event_handler;
     }

     errCode = BKNI_CreateEvent(&pDecoder->stc.statusChangeEvent);
     if ( errCode )
     {
         errCode=BERR_TRACE(errCode);
         goto err_stc_status_change_event;
     }

     pDecoder->stc.statusChangeEventHandler = NEXUS_RegisterEvent(pDecoder->stc.statusChangeEvent, NEXUS_AudioDecoder_P_StcChannelStatusChangeEventHandler, pDecoder);
     if ( NULL == pDecoder->stc.statusChangeEventHandler )
     {
         errCode=BERR_TRACE(BERR_OS_ERROR);
         goto err_stc_status_change_event_handler;
     }

     /* Open Raptor Channel */
    pChannelSettings = BKNI_Malloc(sizeof(*pChannelSettings));   /* this structure is HUGE, don't use the stack */
    if ( NULL == pChannelSettings )
    {
        errCode=BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto err_channel_settings_alloc;
    }
    (void)BRAP_GetDefaultChannelSettings(g_NEXUS_audioModuleData.hRap, pChannelSettings);

    pChannelSettings->eChType = BRAP_ChannelType_eDecode;
    if ( pSettings->type == NEXUS_AudioDecoderType_eAudioDescriptor )
    {
        pDecoder->isDescriptorDecoder = true;
    }
#if BRAP_VER < 4
    pChannelSettings->eChSubType = BRAP_ChannelSubType_eNone; /* for stb no primary/secondary */
#endif

    /* open time memory */
    if ( pSettings->type == NEXUS_AudioDecoderType_eDecode )
    {
        /* Use one for decode, plus any required for post-processing branches. */
        pChannelSettings->sChnRBufPool.uiMaxNumOutChPairs[0]=1+g_NEXUS_audioModuleData.moduleSettings.maximumProcessingBranches;

        #if NEXUS_DDP || NEXUS_WMA || NEXUS_HAS_DIGITAL_AUDIO_INPUTS
        /* Add one for simul mode */
        pChannelSettings->sChnRBufPool.uiMaxNumOutChPairs[0] = pChannelSettings->sChnRBufPool.uiMaxNumOutChPairs[0] + 1;
        #endif

        #if NEXUS_HAS_DIGITAL_AUDIO_INPUTS
        /* Need a ringbuffer for capture */
        pChannelSettings->sChnRBufPool.uiMaxNumInChPairs[0]=1;
        #endif

    }
    else
    {
        /* For passthrough and description, we need only one channel pair for output */
        pChannelSettings->sChnRBufPool.uiMaxNumOutChPairs[0]=1;
    }
    if ( NEXUS_GetEnv("multichannel_disabled") || NEXUS_GetEnv("audio_processing_disabled") )
    {
        pDecoder->connectors[NEXUS_AudioDecoderConnectorType_eMultichannel].format = NEXUS_AudioInputFormat_ePcmStereo;
    }
    else
    {
        switch ( pSettings->multichannelFormat )
        {
        case NEXUS_AudioMultichannelFormat_eNone:
            /* Multichannel output disabled */
            pDecoder->connectors[NEXUS_AudioDecoderConnectorType_eMultichannel].format = NEXUS_AudioInputFormat_ePcmStereo;
            break;
        case NEXUS_AudioMultichannelFormat_e5_1:
            pDecoder->connectors[NEXUS_AudioDecoderConnectorType_eMultichannel].format = NEXUS_AudioInputFormat_ePcm5_1;
            #if NEXUS_NUM_HDMI_OUTPUTS
            pChannelSettings->sChnRBufPool.uiMaxNumOutChPairs[0] += 3;
            #endif
            break;
        #if 0 /* Not supported on any current chipsets */
        case NEXUS_AudioMultichannelFormat_e7_1:
            pDecoder->connectors[NEXUS_AudioDecoderConnectorType_eMultichannel].format = NEXUS_AudioInputFormat_ePcm7_1;
            #if NEXUS_NUM_HDMI_OUTPUTS
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
    if ( pSettings->independentDelay && !g_NEXUS_audioModuleData.moduleSettings.independentDelay )
    {
        BDBG_ERR(("Independent delay must be enabled at the audio module level.  Please check NEXUS_AudioModuleSettings.independentDelay."));
        errCode = BERR_TRACE(BERR_INVALID_PARAMETER);
        goto err_channel;
    }

    pChannelSettings->bEnaIndDly = pSettings->independentDelay;
    pChannelSettings->sChnRBufPool.uiMaxNumRBuf=0;
    pChannelSettings->sChnRBufPool.sBufSettings.pLeftBufferStart=NULL;
    pChannelSettings->sChnRBufPool.sBufSettings.pRightBufferStart=NULL;
    pChannelSettings->sChnRBufPool.sBufSettings.uiSize=0;
    pChannelSettings->sChnRBufPool.sBufSettings.uiWaterMark=0;
#if BRAP_VER >= 4
    pChannelSettings->sChnRBufPool.bHbrMode = g_NEXUS_audioModuleData.moduleSettings.hbrEnabled;
#endif

    errCode = BRAP_OpenChannel(g_NEXUS_audioModuleData.hRap, &pDecoder->rapChannel, pChannelSettings);
    if ( errCode )
    {
        errCode=BERR_TRACE(errCode);
        goto err_channel;
    }

    /* Hook up decoder interrupts */
    errCode = BRAP_InstallAppInterruptCb(pDecoder->rapChannel, BRAP_Interrupt_eFirstPtsReady,
                                         NEXUS_AudioDecoder_P_FirstPts_isr, pDecoder, 0);
    if ( errCode )
        { errCode=BERR_TRACE(errCode); goto err_cb; }
    errCode = BRAP_InstallAppInterruptCb(pDecoder->rapChannel, BRAP_Interrupt_eTsmFail,
                                    NEXUS_AudioDecoder_P_AudioTsmFail_isr, pDecoder, 0);
    if ( errCode )
        { errCode=BERR_TRACE(errCode); goto err_cb; }

#if NEXUS_HAS_ASTM
    errCode = BRAP_InstallAppInterruptCb(pDecoder->rapChannel, BRAP_Interrupt_eAstmTsmPass,
        NEXUS_AudioDecoder_P_AudioTsmPass_isr, pDecoder, 0);
    if (errCode)
    {
        errCode=BERR_TRACE(errCode);
        goto err_cb;
    }
#endif
    errCode = BRAP_InstallAppInterruptCb(pDecoder->rapChannel, BRAP_Interrupt_eSampleRateChange,
                                    NEXUS_AudioDecoder_P_SampleRateChange_isr, pDecoder, 0);
    if ( errCode )
        { errCode=BERR_TRACE(errCode); goto err_cb; }
    errCode = BRAP_InstallAppInterruptCb(pDecoder->rapChannel, BRAP_Interrupt_eDecLock,
                                    NEXUS_AudioDecoder_P_StatusChange_isr, pDecoder, BRAP_Interrupt_eDecLock);
    if ( errCode )
        { errCode=BERR_TRACE(errCode); goto err_cb; }
    errCode = BRAP_InstallAppInterruptCb(pDecoder->rapChannel, BRAP_Interrupt_eDecUnlock,
                                    NEXUS_AudioDecoder_P_StatusChange_isr, pDecoder, BRAP_Interrupt_eDecUnlock);
    if ( errCode )
        { errCode=BERR_TRACE(errCode); goto err_cb; }
    errCode = BRAP_InstallAppInterruptCb(pDecoder->rapChannel, BRAP_Interrupt_eModeChange,
                                    NEXUS_AudioDecoder_P_StatusChange_isr, pDecoder, BRAP_Interrupt_eModeChange);
    if ( errCode )
        { errCode=BERR_TRACE(errCode); goto err_cb; }
    errCode = BRAP_InstallAppInterruptCb(pDecoder->rapChannel, BRAP_Interrupt_eBitRateChange,
                                    NEXUS_AudioDecoder_P_StatusChange_isr, pDecoder, BRAP_Interrupt_eBitRateChange);
    if ( errCode )
        { errCode=BERR_TRACE(errCode); goto err_cb; }
    errCode = BRAP_InstallAppInterruptCb(pDecoder->rapChannel, BRAP_Interrupt_eMpegLayerChange,
                                    NEXUS_AudioDecoder_P_LayerChange_isr, pDecoder, 0);
    if ( errCode )
        { errCode=BERR_TRACE(errCode); goto err_cb; }

    LOCK_TRANSPORT();
    NEXUS_Rave_GetDefaultOpenSettings_priv(&raveSettings);
    UNLOCK_TRANSPORT();

#if BRAP_VER >= 4
    raveSettings.config = sCdbItbCfg[0];
#else
    errCode = BRAP_GetBufferConfig(g_NEXUS_audioModuleData.hRap, BRAP_BufferCfgMode_eAdvAudioMode, &raveSettings.config);
    if ( errCode )
    {
        errCode=BERR_TRACE(errCode);
        goto err_buffer_config;
    }
#endif

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
    pDecoder->raveContext = NEXUS_Rave_Open_priv(&raveSettings);
    UNLOCK_TRANSPORT();
    if ( NULL == pDecoder->raveContext )
    {
        BDBG_ERR(("Unable to allocate RAVE context"));
        goto err_rave;
    }

    pDecoder->sourceChangeAppCallback = NEXUS_TaskCallback_Create(pDecoder, NULL);
    if ( NULL == pDecoder->sourceChangeAppCallback )
    {
        goto err_app_callback;
    }

    pDecoder->lockCallback = NEXUS_IsrCallback_Create(pDecoder, NULL);
    if ( NULL == pDecoder->lockCallback )
    {
        goto err_lock_callback;
    }

    pDecoder->ptsErrorCallback = NEXUS_IsrCallback_Create(pDecoder, NULL);
    if ( NULL == pDecoder->ptsErrorCallback )
    {
        goto err_pts_error_callback;
    }

    pDecoder->firstPtsCallback = NEXUS_IsrCallback_Create(pDecoder, NULL);
    if ( NULL == pDecoder->firstPtsCallback )
    {
        goto err_first_pts_callback;
    }

    pDecoder->overflowCallback = NEXUS_IsrCallback_Create(pDecoder, NULL);
    if ( NULL == pDecoder->overflowCallback )
    {
        goto err_overflow_callback;
    }

    pDecoder->underflowCallback = NEXUS_IsrCallback_Create(pDecoder, NULL);
    if ( NULL == pDecoder->underflowCallback )
    {
        goto err_underflow_callback;
    }

    pDecoder->streamInfoCallback = NEXUS_TaskCallback_Create(pDecoder, NULL);
    if ( NULL == pDecoder->streamInfoCallback )
    {
        goto err_stream_info_callback;
    }

    errCode = BRAP_InstallAppInterruptCb(pDecoder->rapChannel, BRAP_Interrupt_eCdbItbOverflow,
                                         NEXUS_AudioDecoder_P_CdbItbError_isr, pDecoder, (int)pDecoder->overflowCallback);
    if ( errCode )
        { errCode=BERR_TRACE(errCode); goto err_converter; }
    errCode = BRAP_InstallAppInterruptCb(pDecoder->rapChannel, BRAP_Interrupt_eCdbItbUnderflow,
                                         NEXUS_AudioDecoder_P_CdbItbError_isr, pDecoder, (int)pDecoder->underflowCallback);
    if ( errCode )
        { errCode=BERR_TRACE(errCode); goto err_converter; }
    {
        BRAP_GetDefaultProcessingStageSettings(BRAP_ProcessingType_eGenericPassthru,
                                               &g_NEXUS_StageSettings);
        errCode = BRAP_CreateProcessingStage(g_NEXUS_audioModuleData.hRap,
                                             &g_NEXUS_StageSettings,
                                             &pDecoder->converter);
        if ( errCode )
        {
            errCode = BERR_TRACE(errCode);
            goto err_converter;
        }
    }

#if BRAP_VER < 4  /* starting from base4, AD is handled by Association and FwMixer, AdFade/AdPan no longer supported */
    /* Init audio descriptor structures */
    errCode = BRAP_CreateAudioDescriptorFadeTable(g_NEXUS_audioModuleData.hRap, &pDecoder->descriptorTable);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        goto err_descriptor;
    }
    else
    {
        BRAP_GetDefaultProcessingStageSettings(BRAP_ProcessingType_eAdFade,
                                               &g_NEXUS_StageSettings);
        errCode = BRAP_CreateProcessingStage(g_NEXUS_audioModuleData.hRap,
                                             &g_NEXUS_StageSettings,
                                             &pDecoder->descriptorProcessor);
        if ( errCode )
        {
            errCode = BERR_TRACE(errCode);
            goto err_descriptor_processor;
        }
    }
#endif

#if NEXUS_HAS_AUDIO_TSM_LOG_SUPPORT
    if (NEXUS_GetEnv("rap_tsmlog"))
    {
        NEXUS_Error rc = NEXUS_SUCCESS;
        char filename[256];
        sprintf(filename,"tsmlog_ch%d.dat", index);
        g_pTsmLogFiles[index] = fopen(filename,"w");
        if (g_pTsmLogFiles[index] == NULL)
        {
            BDBG_ERR(("Couldn't open file %s for storing TSM log data. Please check write permission for current directory", filename));
        }
        fprintf(g_pTsmLogFiles[index],"   STC    \tPCR Offset\t   PTS    \tFC-TSMSTAT\t   DIFF   \tSINK Depth\t CDB Read \tCDB Write");
        /* Install interrupt handler for TSM log */
        rc = BRAP_InstallAppInterruptCb(pDecoder->rapChannel,
                BRAP_Interrupt_eTsmLog,
                NEXUS_AudioDecoder_P_AudioTsmLog_isr,
                g_pTsmLogFiles[index],
                0);
        if(rc != BERR_SUCCESS)
        {
            BDBG_ERR(("TSM log interrupt installation failed"));
        }
    }
#endif

    /* Success */
    NEXUS_AudioDecoder_P_GetDefaultSettings(&pDecoder->settings);
    (void)NEXUS_AudioDecoder_SetSettings(pDecoder, &pDecoder->settings);
    BKNI_Free(pChannelSettings);
    pDecoder->opened = true;
    pDecoder->suspended = false;

    return pDecoder;

#if BRAP_VER < 4  /* not required by base4, handled by Association */
err_descriptor_processor:
    BRAP_DestroyAudioDescriptorFadeTable(pDecoder->descriptorTable);
err_descriptor:
    BRAP_DestroyProcessingStage(pDecoder->converter);
#endif
err_converter:
    NEXUS_TaskCallback_Destroy(pDecoder->streamInfoCallback);
err_stream_info_callback:
    NEXUS_IsrCallback_Destroy(pDecoder->underflowCallback);
err_underflow_callback:
    NEXUS_IsrCallback_Destroy(pDecoder->overflowCallback);
err_overflow_callback:
    NEXUS_IsrCallback_Destroy(pDecoder->firstPtsCallback);
err_first_pts_callback:
    NEXUS_IsrCallback_Destroy(pDecoder->ptsErrorCallback);
err_pts_error_callback:
    NEXUS_IsrCallback_Destroy(pDecoder->lockCallback);
err_lock_callback:
    NEXUS_TaskCallback_Destroy(pDecoder->sourceChangeAppCallback);
err_app_callback:
    LOCK_TRANSPORT();
    NEXUS_Rave_Close_priv(pDecoder->raveContext);
    UNLOCK_TRANSPORT();
err_rave:
#if BRAP_VER < 4
err_buffer_config:
#endif
err_cb:
    BRAP_CloseChannel(pDecoder->rapChannel);
err_channel:
    BKNI_Free(pChannelSettings);
    NEXUS_UnregisterEvent(pDecoder->stc.statusChangeEventHandler);
err_stc_status_change_event_handler:
    BKNI_DestroyEvent(pDecoder->stc.statusChangeEvent);
err_stc_status_change_event:
    NEXUS_UnregisterEvent(pDecoder->channelChangeReportEventHandler);
err_channel_change_report_event_handler:
    BKNI_DestroyEvent(pDecoder->channelChangeReportEvent);
err_channel_change_report_event:
    NEXUS_UnregisterEvent(pDecoder->layerChangeCallback);
err_layer_callback:
err_channel_settings_alloc:
    BKNI_DestroyEvent(pDecoder->layerChangeEvent);
err_layer_change_event:
    NEXUS_UnregisterEvent(pDecoder->sourceChangeCallback);
err_source_change_callback:
    BKNI_DestroyEvent(pDecoder->sourceChangedEvent);
err_source_changed_event:
    NEXUS_UnregisterEvent(pDecoder->sampleRateCallback);
err_sample_rate_callback:
    BKNI_DestroyEvent(pDecoder->sampleRateEvent);
err_sample_rate_event:
    return NULL;
}

/***************************************************************************
Summary:
    Close an audio decoder of the specified type
See Also:
    NEXUS_AudioDecoder_Open
 ***************************************************************************/
static void NEXUS_AudioDecoder_P_Finalizer(
    NEXUS_AudioDecoderHandle handle
    )
{
    NEXUS_Error errCode;
    int i;

    NEXUS_OBJECT_ASSERT(NEXUS_AudioDecoder, handle);
    BDBG_ASSERT(true == handle->opened);

    if ( handle->started )
    {
        NEXUS_AudioDecoder_Stop(handle);
    }

    /* Clean up descriptor hooks */
    if ( handle->settings.descriptionDecoder )
    {
        if ( handle->settings.descriptionDecoder->started )
        {
            BDBG_WRN(("Implicitly stopping description decoder %p on decoder %p close", (void *)handle->settings.descriptionDecoder, (void *)handle));
            NEXUS_AudioDecoder_Stop(handle->settings.descriptionDecoder);
        }
        handle->settings.descriptionDecoder->descriptorParent = NULL;
        handle->settings.descriptionDecoder = NULL;
    }
    else if ( handle->descriptorParent )
    {
        if ( handle->descriptorParent->started )
        {
            BDBG_WRN(("Implicitly stopping parent decoder %p on description decoder %p close", (void *)handle->descriptorParent, (void *)handle));
            NEXUS_AudioDecoder_Stop(handle->descriptorParent);
        }
        handle->descriptorParent->settings.descriptionDecoder = NULL;
        handle->descriptorParent = NULL;
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
            BDBG_MSG(("Decoder Connector %p (type=%s) is still active. Calling NEXUS_AudioInput_Shutdown.", (void *)&handle->connectors[i], pType));
            NEXUS_AudioInput_Shutdown(&handle->connectors[i]);
        }
    }

#if BRAP_VER < 4  /* not required by base4, handled by Association */
    BRAP_DestroyProcessingStage(handle->descriptorProcessor);
    BRAP_DestroyAudioDescriptorFadeTable(handle->descriptorTable);
#endif

    BRAP_DestroyProcessingStage(handle->converter);

    if ( NULL != handle->association )
    {
        NEXUS_AudioAssociation_P_Destroy(handle->association);
    }
    errCode = BRAP_CloseChannel(handle->rapChannel);
    BDBG_ASSERT(BERR_SUCCESS == errCode);

    NEXUS_TaskCallback_Destroy(handle->sourceChangeAppCallback);
    NEXUS_IsrCallback_Destroy(handle->lockCallback);
    NEXUS_IsrCallback_Destroy(handle->firstPtsCallback);
    NEXUS_IsrCallback_Destroy(handle->ptsErrorCallback);
    NEXUS_IsrCallback_Destroy(handle->underflowCallback);
    NEXUS_IsrCallback_Destroy(handle->overflowCallback);
    NEXUS_TaskCallback_Destroy(handle->streamInfoCallback);

    LOCK_TRANSPORT();
    NEXUS_Rave_Close_priv(handle->raveContext);
    UNLOCK_TRANSPORT();

    NEXUS_UnregisterEvent(handle->stc.statusChangeEventHandler);
    BKNI_DestroyEvent(handle->stc.statusChangeEvent);
    NEXUS_UnregisterEvent(handle->channelChangeReportEventHandler);
    BKNI_DestroyEvent(handle->channelChangeReportEvent);
    NEXUS_UnregisterEvent(handle->layerChangeCallback);
    NEXUS_UnregisterEvent(handle->sourceChangeCallback);
    NEXUS_UnregisterEvent(handle->sampleRateCallback);
    BKNI_DestroyEvent(handle->layerChangeEvent);
    BKNI_DestroyEvent(handle->sourceChangedEvent);
    BKNI_DestroyEvent(handle->sampleRateEvent);
    NEXUS_OBJECT_CLEAR(NEXUS_AudioDecoder, handle);
}

NEXUS_OBJECT_CLASS_MAKE(NEXUS_AudioDecoder, NEXUS_AudioDecoder_Close);

/***************************************************************************
Summary:
    Get Settings for an audio decoder
See Also:
    NEXUS_AudioDecoder_SetSettings
 ***************************************************************************/
void NEXUS_AudioDecoder_GetSettings(
    NEXUS_AudioDecoderHandle handle,
    NEXUS_AudioDecoderSettings *pSettings   /* [out] Settings */
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioDecoder);
    BDBG_ASSERT(NULL != pSettings);

    *pSettings = handle->settings;
}

/***************************************************************************
Summary:
    Set Settings for an audio decoder
See Also:
    NEXUS_AudioDecoder_GetSettings
 ***************************************************************************/
NEXUS_Error NEXUS_AudioDecoder_SetSettings(
    NEXUS_AudioDecoderHandle handle,
    const NEXUS_AudioDecoderSettings *pSettings /* Settings */
    )
{
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

        if ( pSettings->descriptionDecoder != handle->settings.descriptionDecoder )
        {
#if BRAP_VER < 4  /* not required by base4, handled by Association */
            BRAP_AudioDescriptorFadeTableParams fadeTableParams;
            NEXUS_Error errCode;
#endif
            if ( handle->started )
            {
                BDBG_ERR(("Cannot modify audio decsriptor decoder while decoder is active"));
                return BERR_TRACE(BERR_NOT_SUPPORTED);
            }
            /* If we currently have a description decoder, unlink it */
            if ( handle->settings.descriptionDecoder )
            {
                if ( handle->settings.descriptionDecoder->started )
                {
                    BDBG_ERR(("Cannot modify audio decsriptor decoder while description decoder is active"));
                    return BERR_TRACE(BERR_NOT_SUPPORTED);
                }
                handle->settings.descriptionDecoder->descriptorParent = NULL;
                handle->settings.descriptionDecoder = NULL;
            }
            /* If we're setting a description decoder, link them */
            if ( pSettings->descriptionDecoder )
            {
                if ( pSettings->descriptionDecoder->started )
                {
                    BDBG_ERR(("Cannot modify audio decsriptor decoder while description decoder is active"));
                    return BERR_TRACE(BERR_NOT_SUPPORTED);
                }
                else if ( pSettings->descriptionDecoder->descriptorParent )
                {
                    BDBG_ERR(("Descriptor decoder is already linked to decoder %p", (void *)pSettings->descriptionDecoder->descriptorParent));
                    return BERR_TRACE(BERR_INVALID_PARAMETER);
                }
                else if ( !pSettings->descriptionDecoder->isDescriptorDecoder )
                {
                    BDBG_ERR(("Descriptor decoder %p was not opened as for AudioDescriptors.  Please verify NEXUS_AudioDecoderOpenSettings.type",
                              (void *)pSettings->descriptionDecoder));
                    return BERR_TRACE(BERR_INVALID_PARAMETER);
                }

#if BRAP_VER < 4  /* not required by base4, handled by Association */
                (void)BRAP_GetAudioDescriptorFadeTable(handle->descriptorTable, &fadeTableParams);
                fadeTableParams.hPrimaryChannel = handle->rapChannel;
                fadeTableParams.hDescriptorChannel = pSettings->descriptionDecoder->rapChannel;
                errCode = BRAP_SetAudioDescriptorFadeTable(handle->descriptorTable, &fadeTableParams);
                if ( errCode )
                {
                    return BERR_TRACE(errCode);
                }
#endif
                pSettings->descriptionDecoder->descriptorParent = handle;
                handle->settings.descriptionDecoder = pSettings->descriptionDecoder;
            }
        }

        handle->settings = *pSettings;
    }
    NEXUS_TaskCallback_Set(handle->sourceChangeAppCallback, &(handle->settings.sourceChanged));
    NEXUS_IsrCallback_Set(handle->lockCallback, &(handle->settings.lockChanged));
    NEXUS_IsrCallback_Set(handle->ptsErrorCallback, &(handle->settings.ptsError));
    NEXUS_IsrCallback_Set(handle->firstPtsCallback, &(handle->settings.firstPts));
    NEXUS_IsrCallback_Set(handle->overflowCallback, &(handle->settings.fifoOverflow));
    NEXUS_IsrCallback_Set(handle->underflowCallback, &(handle->settings.fifoUnderflow));
    NEXUS_TaskCallback_Set(handle->streamInfoCallback, &(handle->settings.streamStatusAvailable));
    return NEXUS_AudioDecoder_ApplySettings_priv(handle);
}

static const BRAP_DSPCHN_AudioType g_dualMonoCodecs[] =
{
    BRAP_DSPCHN_AudioType_eMpeg,
    BRAP_DSPCHN_AudioType_eAacAdts,
    BRAP_DSPCHN_AudioType_eAacLoas,
    BRAP_DSPCHN_AudioType_eAacSbrAdts,
    BRAP_DSPCHN_AudioType_eAacSbrLoas,
    BRAP_DSPCHN_AudioType_eAc3,
    BRAP_DSPCHN_AudioType_eAc3Plus,
    BRAP_DSPCHN_AudioType_eDts, /* BRAP_DSPCHN_AudioType_eDts is used for BD-DVD application, not used for STB */
    BRAP_DSPCHN_AudioType_eDtsBroadcast,
    BRAP_DSPCHN_AudioType_eDtshd
};

NEXUS_Error NEXUS_AudioDecoder_ApplySettings_priv(
    NEXUS_AudioDecoderHandle handle
)
{
    BRAP_ChannelConfigParams *pSettings;
    NEXUS_Error errCode;
    BRAP_DSPCHN_DualMonoMode dualMonoMode;
    BRAP_DSPCHN_Ac3DualMonoMode ac3DualMonoMode;
    int i;
#if BRAP_VER >= 4
    NEXUS_AudioInput mixerConnector;
#endif

    NEXUS_ASSERT_MODULE();

    switch ( handle->settings.dualMonoMode )
    {
    case NEXUS_AudioDecoderDualMonoMode_eLeft:
        dualMonoMode = BRAP_DSPCHN_DualMonoMode_eLeftMono;
        ac3DualMonoMode = BRAP_DSPCHN_Ac3DualMonoMode_eLtMono;
        break;
    case NEXUS_AudioDecoderDualMonoMode_eRight:
        dualMonoMode = BRAP_DSPCHN_DualMonoMode_eRightMono;
        ac3DualMonoMode = BRAP_DSPCHN_Ac3DualMonoMode_eRtMono;
        break;
    case NEXUS_AudioDecoderDualMonoMode_eMix:
        dualMonoMode = BRAP_DSPCHN_DualMonoMode_eDualMonoMix;
        ac3DualMonoMode = BRAP_DSPCHN_Ac3DualMonoMode_eDualMono;
        break;
    default:
        dualMonoMode = BRAP_DSPCHN_DualMonoMode_eStereo;
        ac3DualMonoMode = BRAP_DSPCHN_Ac3DualMonoMode_eStereo;
        break;
    }

    pSettings = BKNI_Malloc(sizeof(*pSettings));
    if ( NULL == pSettings )
    {
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }

    /* Set volume in audioparams structure for start/stop */
    NEXUS_AudioDecoder_P_SetupMixingCoefs(handle, &handle->audioParams.sMixingCoeff);

    for ( i=0; i < (int)(sizeof(g_dualMonoCodecs)/sizeof(BRAP_DSPCHN_AudioType)); i++ )
    {
        (void)BRAP_GetCurrentChannelConfig(handle->rapChannel,
                                           g_dualMonoCodecs[i],
                                           pSettings);
        pSettings->uChConfigParams.sDecConfigParams.eDualMonoMode = dualMonoMode;

        NEXUS_AudioDecoder_P_SetupMixingCoefs(handle, &pSettings->sMixingCoef);

        errCode = BRAP_SetChannelConfig(handle->rapChannel, pSettings);
        if ( errCode )
        {
            BKNI_Free(pSettings);
            return BERR_TRACE(errCode);
        }
    }

    BKNI_Free(pSettings);

#if BRAP_VER < 4
    if ( handle->descriptorParent )
    {
        uint64_t descriptorVolume;
        /* Set AD volume via PAN */
        (void)BRAP_GetCurrentProcessingStageSettings(handle->descriptorProcessor, &g_NEXUS_StageSettings);
        /* Use L->L volume for AD */
        if ( handle->settings.volumeMatrix[0][0] != handle->settings.volumeMatrix[1][1] )
        {
            BDBG_WRN(("Only mono volume control is available for Audio Descriptors.  Using left channel volume"));
        }
        descriptorVolume = 0x7fffffff;  /* This is the full volume value for AD */
        descriptorVolume *= handle->settings.volumeMatrix[0][0];
        descriptorVolume /= NEXUS_AUDIO_VOLUME_LINEAR_NORMAL;
        g_NEXUS_StageSettings.uConfigParams.sAdPanParams.uiVol = descriptorVolume;
        g_NEXUS_StageSettings.uConfigParams.sAdPanParams.bEnableAd = !handle->settings.muted;
        g_NEXUS_StageSettings.uConfigParams.sAdPanParams.uiAudioRampTimeInMs = handle->settings.descriptionRampTime;
        errCode = BRAP_SetProcessingStageSettings(handle->descriptorProcessor, &g_NEXUS_StageSettings);
        if ( errCode )
        {
            errCode = BERR_TRACE(errCode);
        }
    }
#else

    mixerConnector = NEXUS_AudioInput_P_FindByType(&handle->connectors[NEXUS_AudioDecoderConnectorType_eMultichannel], NEXUS_AudioInputType_eMixer);
    if ( NULL == mixerConnector )
    {
        mixerConnector = NEXUS_AudioInput_P_FindByType(&handle->connectors[NEXUS_AudioDecoderConnectorType_eStereo], NEXUS_AudioInputType_eMixer);
    }
    if ( mixerConnector && NEXUS_AudioMixer_P_IsSlaveChannel(mixerConnector->pObjectHandle, NEXUS_AudioDecoder_P_GetChannel(handle)) )
    {
        /* This is AD mixing using DSP mixer. Set AD volume via DSP mixer */

        BRAP_ProcessingStageHandle mixerStage = NULL;

        mixerStage = NEXUS_AudioMixer_P_GetStageHandle(mixerConnector->pObjectHandle);
        if ( mixerStage )
        {
            uint64_t descriptorVolume;

            (void)BRAP_GetCurrentProcessingStageSettings(mixerStage, &g_NEXUS_StageSettings);

            /* Use L->L volume for AD */
            if ( handle->settings.volumeMatrix[0][0] != handle->settings.volumeMatrix[1][1] )
            {
                BDBG_WRN(("Only mono volume control is available for Audio Descriptors. Using left channel volume."));
            }
            descriptorVolume = 0x7fffffff;  /* This is the full volume value for AD */
            descriptorVolume *= handle->settings.volumeMatrix[0][0];
            descriptorVolume /= NEXUS_AUDIO_VOLUME_LINEAR_NORMAL;
            g_NEXUS_StageSettings.uConfigParams.sFwMixerParams.uiSecondaryChannelVolCoeff = /*handle->settings.volumeMatrix[0][0]*/descriptorVolume;

            errCode = BRAP_SetProcessingStageSettings(mixerStage, &g_NEXUS_StageSettings);
            if ( errCode )
            {
                errCode = BERR_TRACE(errCode);
            }
        }
    }
#endif

#if NEXUS_HAS_ASTM
    if (handle->astm.settings.enableAstm)
    {
        BDBG_MSG(("ASTM is setting the sync limit for audio channel %p to %d", (void *)handle, handle->astm.settings.syncLimit));
        BRAP_SetTsmSLThreshold(handle->rapChannel, handle->astm.settings.syncLimit);
        /* PR49489 20081201 bandrews - added for audio master mode */
        BDBG_MSG(("ASTM is %s playback mode for audio channel %p", handle->astm.settings.enablePlayback ? "enabling" : "disabling", (void *)handle));
        BRAP_SetPlaybackFlag(handle->rapChannel, handle->astm.settings.enablePlayback);
/* TODO: is there a problem keeping playback flag across restarts due to output changes? */
#if 0 /* PR49215 playback support requires that audio be able to be started in playback */
        /* PR49489 20081203 bandrews - added to ensure that ASTM settings apply on restart */
        handle->audioParams.bPlayback = handle->astm.settings.enablePlayback;
#endif
    }

    NEXUS_AudioDecoder_P_SetTsm(handle);
    NEXUS_AudioDecoder_P_SetPtsOffset(handle);
#else
    BSTD_UNUSED(handle);
#endif

    return BERR_SUCCESS;
}

/***************************************************************************
Summary:
    Initialize an audio decoder program structure
See Also:
    NEXUS_AudioDecoder_Start
 ***************************************************************************/
void NEXUS_AudioDecoder_GetDefaultStartSettings(
    NEXUS_AudioDecoderStartSettings *pSettings /* [out] Program Defaults */
    )
{
    BKNI_Memset(pSettings, 0, sizeof(NEXUS_AudioDecoderStartSettings));
    pSettings->codec = NEXUS_AudioCodec_eAc3;
    pSettings->targetSyncEnabled = true;
}

static NEXUS_Error NEXUS_AudioDecoder_P_SetupCodec(
    NEXUS_AudioDecoderHandle handle,
    BRAP_DSPCHN_AudioType codec,
    BAVC_StreamType streamType
    )
{
    const NEXUS_AudioDecoderStartSettings *pProgram = &handle->programSettings;
    NEXUS_Error errCode;
    unsigned stcChannelIndex;

    BDBG_ENTER(NEXUS_AudioDecoder_P_SetupCodec);

    /* Setup compressed output format based on input type and codec */
    if ( pProgram->input )
    {
        /* For HDMI, we currently only passthrough compressed AC3 as DDP. */
        switch ( codec )
        {
        default:
            BDBG_MSG(("Configuring for compressed SPDIF passthrough"));
            handle->connectors[NEXUS_AudioDecoderConnectorType_eCompressed].format = NEXUS_AudioInputFormat_eCompressed;
            break;
        case BRAP_DSPCHN_AudioType_ePcm:
            BDBG_MSG(("Configuring for PCM SPDIF data"));
            handle->connectors[NEXUS_AudioDecoderConnectorType_eCompressed].format = NEXUS_AudioInputFormat_ePcmStereo;
            break;
        }
    }
    else
    {
        /* Always support passthrough on XPT sources */
        BDBG_MSG(("Configuring for compressed SPDIF passthrough"));
        handle->connectors[NEXUS_AudioDecoderConnectorType_eCompressed].format = NEXUS_AudioInputFormat_eCompressed;
    }

    /* Validate that we aren't trying to encode formats that aren't permitted */
    switch ( codec )
    {
    case BRAP_DSPCHN_AudioType_eAacLoas:
    case BRAP_DSPCHN_AudioType_eAacAdts:
    case BRAP_DSPCHN_AudioType_eAacSbrLoas:
    case BRAP_DSPCHN_AudioType_eAacSbrAdts:
    case BRAP_DSPCHN_AudioType_eDtsBroadcast:
    case BRAP_DSPCHN_AudioType_eDtshd:
        /* These are okay for DTS */
        break;
    default:
        if ( NEXUS_AudioDecoder_P_FindDts(handle) )
        {
            BDBG_ERR(("DTS Encoding is only supported for AAC/AAC+ at 48kHz."));
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }
        break;
    }

    /* Set Audio params - this can only be set at start time. */
    errCode = BRAP_GetDefaultChannelParams(handle->rapChannel, &handle->audioParams);
    if (errCode) { return BERR_TRACE(errCode);}

    handle->audioParams.sDspChParams.eType = codec;
    if (pProgram->stcChannel) {
        LOCK_TRANSPORT();
        NEXUS_StcChannel_GetIndex_priv(pProgram->stcChannel, &stcChannelIndex);
        UNLOCK_TRANSPORT();
    }
    else {
        stcChannelIndex = 0;
    }
    handle->audioParams.eTimebase = stcChannelIndex; /* BRAP_ChannelParams.eTimebase is misnamed. It is not a timebase (i.e. DPCR). It is an STC index. */
    if ( pProgram->stcChannel ) {
        NEXUS_StcChannelSettings stcSettings;
        NEXUS_StcChannel_GetSettings(pProgram->stcChannel, &stcSettings);
        /* tell the decoder what type of TSM to do based on NEXUS_StcChannelMode, not transport source */
        handle->audioParams.bPlayback = (stcSettings.mode != NEXUS_StcChannelMode_ePcr && NULL == pProgram->input);
    }
    else {
        /* for vsync, set bPlayback based on transport source. this is used later to set bandHold. */
        handle->audioParams.bPlayback = handle->isPlayback;
    }
    handle->audioParams.sDspChParams.i32AVOffset = handle->settings.ptsOffset;
    handle->audioParams.eInputSR = BAVC_AudioSamplingRate_e48k; /* BAVC_AudioSamplingRate_eUnknown;*/
#if !NEXUS_GLOBAL_INDEPENDENT_VOLUME
    /***************************************************************************/
    /* This is required to avoid glitches in captured audio due to massive     */
    /* timebase differences between the capture buffer and the output ports    */
    /***************************************************************************/
    handle->audioParams.bIndOpVolCtrl = false;
#endif

    NEXUS_AudioDecoder_P_SetupMixingCoefs(handle, &handle->audioParams.sMixingCoeff);

#if BRAP_VER < 4  /* not required by base4, handled by Association */
    if ( handle->settings.descriptionDecoder )
    {
        /* AD parent */
        handle->audioParams.hAdFade = handle->descriptorTable;

        /* Setup for fading */
        BRAP_GetDefaultProcessingStageSettings(BRAP_ProcessingType_eAdFade,
                                               &g_NEXUS_StageSettings);
        errCode = BRAP_SetProcessingStageSettings(handle->descriptorProcessor, &g_NEXUS_StageSettings);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
    }
    else if ( handle->descriptorParent )
    {
        /* AD child */
        handle->audioParams.hAdFade = handle->descriptorParent->descriptorTable;

        /* Setup for panning */
        BRAP_GetDefaultProcessingStageSettings(BRAP_ProcessingType_eAdPan,
                                               &g_NEXUS_StageSettings);
        errCode = BRAP_SetProcessingStageSettings(handle->descriptorProcessor, &g_NEXUS_StageSettings);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
    }
#endif

    if ( pProgram->input )
    {
        BDBG_MSG(("Starting decoder with external input"));
        handle->audioParams.eCapMode = BRAP_CaptureMode_eFullDuplex;
        handle->audioParams.eAudioSource = BRAP_AudioSource_eExtCapPort;

        switch ( pProgram->input->objectType )
        {
        case NEXUS_AudioInputType_eHdmi:
            handle->audioParams.eCapInputPort = BRAP_CapInputPort_eHdmi;
            break;
        case NEXUS_AudioInputType_eSpdif:
            handle->audioParams.eCapInputPort = BRAP_CapInputPort_eSpdif;
            break;
        default:
            BDBG_ERR(("Invalid audio input type"));
            BDBG_ASSERT(0);
            return BERR_TRACE(BERR_INVALID_PARAMETER);
        }
    }
    else
    {
        BDBG_MSG(("Starting decoder with transport input"));
        handle->audioParams.eAudioSource = BRAP_AudioSource_eXptInterface;
        handle->audioParams.sDspChParams.eStreamType = streamType;
    }

    handle->audioParams.sDspChParams.bEnableTargetSync = pProgram->targetSyncEnabled;

    BDBG_LEAVE(NEXUS_AudioDecoder_P_SetupCodec);

    return NEXUS_SUCCESS;
}

static void NEXUS_AudioDecoder_P_FifoWatchdog(void *context)
{
    NEXUS_AudioDecoderHandle audioDecoder = context;

    audioDecoder->fifoWatchdog.timer = NULL;

    if (audioDecoder->programSettings.stcChannel) {
        uint32_t cdbValidPointer, cdbReadPointer;
        unsigned depth, size;

        BKNI_EnterCriticalSection();
        NEXUS_Rave_GetCdbPointers_isr(audioDecoder->raveContext, &cdbValidPointer, &cdbReadPointer);
        NEXUS_Rave_GetCdbBufferInfo_isr(audioDecoder->raveContext, &depth, &size);
        BKNI_LeaveCriticalSection();
        if (audioDecoder->fifoWatchdog.lastCdbValidPointer == cdbValidPointer && audioDecoder->fifoWatchdog.lastCdbReadPointer == cdbReadPointer) {
            /* SWDTV-7287, reduce the timeout for monitoring frozen audio decoder from 5 secnods to 1 second.
             * Check the audio decoder 8 times every 125ms.  This change should not produce false positives,
             * since long audio frame is 64ms */
            if (audioDecoder->fifoWatchdog.staticCount < 8) {
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

    audioDecoder->fifoWatchdog.timer = NEXUS_ScheduleTimer(125, NEXUS_AudioDecoder_P_FifoWatchdog, audioDecoder);
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
        BDBG_WRN(("audio %p: audio gap zero fill unimplemented", (void *)audioDecoder));
#if 0
        BAPE_Decoder_EnterUnderflowMode(audioDecoder->channel);
        LOCK_TRANSPORT();
        NEXUS_StcChannel_ReportDecoderZeroFill_priv(audioDecoder->stc.connector);
        UNLOCK_TRANSPORT();
#endif
    }
}

NEXUS_Error NEXUS_AudioDecoder_P_ConfigureRave(NEXUS_AudioDecoderHandle handle, NEXUS_RaveHandle rave, const NEXUS_AudioDecoderStartSettings *pProgram, NEXUS_PidChannelStatus *pPidChannelStatus)
{
    NEXUS_RaveSettings raveSettings;
    NEXUS_Error errCode;
    NEXUS_PidChannelStatus pidChannelStatus;
    BAVC_StreamType streamType;

    BSTD_UNUSED(pPidChannelStatus);

    errCode = NEXUS_PidChannel_GetStatus(pProgram->pidChannel, &pidChannelStatus);
    if (errCode) return BERR_TRACE(errCode);

    errCode = NEXUS_P_TransportType_ToMagnum_isrsafe(pidChannelStatus.transportType, &streamType);
    if (errCode) return BERR_TRACE(errCode);

    LOCK_TRANSPORT();
    NEXUS_Rave_GetDefaultSettings_priv(&raveSettings);
    raveSettings.pidChannel = pProgram->pidChannel;
    /* 20120725 bandrews - bandHold cannot be enabled if pid channel is from live band */
    raveSettings.bandHold = handle->audioParams.bPlayback && pidChannelStatus.playback; /* bandHold is required for non-PCR playback only */
    raveSettings.continuityCountEnabled = !handle->isPlayback;
    raveSettings.numOutputBytesEnabled = true;
    switch ( pProgram->codec )
    {
    case NEXUS_AudioCodec_eMpeg:
    case NEXUS_AudioCodec_eMp3:
        if ( handle->descriptorParent || handle->settings.descriptionDecoder )
        {
            raveSettings.audioDescriptor = true;
        }
#if BRAP_VER >= 4
        if ( pProgram->secondaryDecoder )
        {
            raveSettings.audioDescriptor = true;
        }
#endif
        break;
    case NEXUS_AudioCodec_eAacAdts:
    case NEXUS_AudioCodec_eAacLoas:
    case NEXUS_AudioCodec_eAacPlusAdts:
    case NEXUS_AudioCodec_eAacPlusLoas:
    case NEXUS_AudioCodec_eAc3:
    case NEXUS_AudioCodec_eAc3Plus:
        /* need better solution for this */
         if ( streamType != BAVC_StreamType_eDssEs && streamType != BAVC_StreamType_eDssPes && pidChannelStatus.originalTransportType != NEXUS_TransportType_eVob  )
        {
            /* Required for MS10 support.  Harmelss otherwise. */
            raveSettings.audioDescriptor = true;
        }
        break;
    default:
        break;
    }
    errCode = NEXUS_Rave_ConfigureAudio_priv(rave, pProgram->codec, &raveSettings);
    if (errCode)
    {
        UNLOCK_TRANSPORT();
        return BERR_TRACE(errCode);
    }
    UNLOCK_TRANSPORT();
    return 0;
}

/***************************************************************************
Summary:
    Start deocding the specified program
See Also:
    NEXUS_AudioDecoder_Stop
 ***************************************************************************/
NEXUS_Error NEXUS_AudioDecoder_Start(
    NEXUS_AudioDecoderHandle handle,
    const NEXUS_AudioDecoderStartSettings *pProgram    /* What to start decoding */
    )
{
    int i;
    NEXUS_Error errCode;
    BRAP_DSPCHN_AudioType eAlgoType = BRAP_DSPCHN_AudioType_ePcm;
    bool useTsm;
    static NEXUS_PidChannelStatus pidChannelStatus;
    static NEXUS_RaveStatus raveStatus;
    BAVC_StreamType streamType=BAVC_StreamType_eTsMpeg;

    BDBG_ENTER(NEXUS_AudioDecoder_Start);

    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioDecoder);
    BDBG_ASSERT(NULL != pProgram);

    if ( handle->started )
    {
        BDBG_ERR(("This decoder is already started.  Please call stop first."));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    if ( handle->suspended )
    {
        BDBG_ERR(("This decoder is suspended.  Please call stop first."));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    if (handle->index == 0) {
        BTRC_TRACE(ChnChange_DecodeStartAudio, START);
    }

    NEXUS_AudioDecoder_P_TrickReset(handle); /* reset trick state on start */

    /* Save Program */
    /* handle->programSettings = *pProgram; */

    /* Sanity check program */
    #if NEXUS_HAS_DIGITAL_AUDIO_INPUTS
    if ( (pProgram->pidChannel && pProgram->input) ||
         (!pProgram->pidChannel && !pProgram->input) )
    {
        BDBG_ERR(("Must specify either pidChannel or digital input"));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    #else
    if ( NULL == pProgram->pidChannel )
    {
        BDBG_ERR(("No PID Channel"));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    #endif

#if NEXUS_HAS_DIGITAL_AUDIO_INPUTS
    if ( pProgram->input )
    {
        BRAP_SpdifRx_InputDetectionSettings detectionSettings;

        if ( g_spdifDecoder )
        {
            BDBG_ERR(("Can only decode one digital input at a time"));
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }

        /* must init pidChannelStatus to something because it's used later */
        BKNI_Memset(&pidChannelStatus, 0, sizeof(pidChannelStatus));

        streamType = BAVC_StreamType_ePes;
        handle->isPlayback = false;
        handle->isDss = false;
        for ( i = 0; i < NEXUS_AudioDecoderConnectorType_eMax; i++ )
        {
            errCode = NEXUS_AudioInput_P_AddInput(&handle->connectors[i], pProgram->input);
            if ( errCode )
            {
                errCode = BERR_TRACE(errCode);
            }
        }

        /* Setup format switch interrupt handling */
        BRAP_GetDefaultSpdifRxInputDetectionSettings(&detectionSettings);

        switch ( pProgram->input->objectType )
        {
        #if NEXUS_NUM_HDMI_INPUTS > 0
        case NEXUS_AudioInputType_eHdmi:
            detectionSettings.eInputPort = BRAP_CapInputPort_eHdmi;
            {
                BRAP_InputPortConfig hdmiConfig;
                unsigned hdmiIndex=0;

                NEXUS_Module_Lock(g_NEXUS_audioModuleData.hdmiInput);
                NEXUS_HdmiInput_GetIndex_priv(pProgram->input->pObjectHandle, &hdmiIndex);
                NEXUS_Module_Unlock(g_NEXUS_audioModuleData.hdmiInput);
                BRAP_GetCurrentInputConfig(g_NEXUS_audioModuleData.hRap, BRAP_CapInputPort_eHdmi, &hdmiConfig);
                hdmiConfig.sCapPortParams.uCapPortParams.sHdmiRxParams.eHdmiNo = hdmiIndex;
                BRAP_SetInputConfig(g_NEXUS_audioModuleData.hRap, &hdmiConfig);
            }
            break;
        #endif
        case NEXUS_AudioInputType_eSpdif:
            detectionSettings.eInputPort = BRAP_CapInputPort_eSpdif;
            break;
        default:
            BDBG_ERR(("Unsupported input type %d", pProgram->input->objectType));
            return BERR_TRACE(BERR_INVALID_PARAMETER);
        }

        g_spdifDecoder = handle;
        #if 0
        /* TODO: Finish */
        handle->started = true;
        #endif

        errCode = BRAP_EnableSpdifRxInputDetection(g_NEXUS_audioModuleData.hRap, &detectionSettings);
        if ( errCode )
        {
            g_spdifDecoder = NULL;
            handle->started = false;
            return BERR_TRACE(errCode);
        }

        #if 0
        return BERR_SUCCESS;
        #endif
    }
    else
#endif
    {
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
    }

    if ( !pProgram->input )
    {
        switch ( pProgram->codec )
        {
        case NEXUS_AudioCodec_eMpeg:
            /* fall through */
        case NEXUS_AudioCodec_eMp3:
            eAlgoType = BRAP_DSPCHN_AudioType_eMpeg;
            break;
        case NEXUS_AudioCodec_eAc3:
            #if defined NEXUS_AC3 || !defined NEXUS_DDP
            eAlgoType = BRAP_DSPCHN_AudioType_eAc3;
            break;
            #elif defined NEXUS_DDP && !defined NEXUS_AC3
            eAlgoType = BRAP_DSPCHN_AudioType_eAc3Plus;
            break;
            #endif
        case NEXUS_AudioCodec_eAc3Plus:
            eAlgoType = BRAP_DSPCHN_AudioType_eAc3Plus;
            break;
        case NEXUS_AudioCodec_eAacAdts:
            eAlgoType = BRAP_DSPCHN_AudioType_eAacAdts;
            break;
        case NEXUS_AudioCodec_eAacLoas:
            eAlgoType = BRAP_DSPCHN_AudioType_eAacLoas;
            break;
        case NEXUS_AudioCodec_eAacPlusLoas:
        /* baudio_format_aac_plus_loas is also handled here as it has the same value as baudio_format_aac_plus */
            eAlgoType = BRAP_DSPCHN_AudioType_eAacSbrLoas;
            break;
        case NEXUS_AudioCodec_eAacPlusAdts:
            eAlgoType = BRAP_DSPCHN_AudioType_eAacSbrAdts;
            break;
        case NEXUS_AudioCodec_eDtsLegacy: /* For DTS streams with legacy frame-sync.  These streams are something called as 14bits stream */
            eAlgoType = BRAP_DSPCHN_AudioType_eDtsBroadcast; /* BRAP_DSPCHN_AudioType_eDts; */
            break;
        case NEXUS_AudioCodec_eDts:
            /* fall through */
        case NEXUS_AudioCodec_eDtsHd:
           /* Audio Decoder FW can decode both DTS and DTS-HD stream.  For DTS-HD stream, audio decoder will decode the DTS part of stream.
            * In this mode, the audio decoder can handle non-legacy DTS frame-sync. */
            eAlgoType = BRAP_DSPCHN_AudioType_eDtshd;
            break;
        case NEXUS_AudioCodec_eWmaStd:
        case NEXUS_AudioCodec_eWmaStdTs:
            #ifndef NEXUS_WMA
            BDBG_ERR(("WMA not supported in this build.  Please export RAP_WMA_SUPPORT=y before building."));
            return BERR_TRACE(BERR_NOT_SUPPORTED);
            #endif
            eAlgoType = BRAP_DSPCHN_AudioType_eWmaStd;
            break;
        case NEXUS_AudioCodec_eWmaPro:
            #ifndef NEXUS_WMA
            BDBG_ERR(("WMA Pro is not supported in this build.  Please export RAP_WMAPRO_SUPPORT=y before building."));
            return BERR_TRACE(BERR_NOT_SUPPORTED);
            #endif
            eAlgoType = BRAP_DSPCHN_AudioType_eWmaPro;
            break;
        case NEXUS_AudioCodec_ePcm:
            eAlgoType = BRAP_DSPCHN_AudioType_ePcm;
            break;
        case NEXUS_AudioCodec_ePcmWav:
             eAlgoType = BRAP_DSPCHN_AudioType_ePcmWav;
             break;
        case NEXUS_AudioCodec_eLpcmDvd:
            eAlgoType = BRAP_DSPCHN_AudioType_eLpcmDvd;
            break;
        case NEXUS_AudioCodec_eLpcmHdDvd:
            eAlgoType = BRAP_DSPCHN_AudioType_eLpcmHdDvd;
            break;
        case NEXUS_AudioCodec_eLpcmBluRay:
            eAlgoType = BRAP_DSPCHN_AudioType_eLpcmBd;
            break;
        case NEXUS_AudioCodec_eAmr:
            eAlgoType = BRAP_DSPCHN_AudioType_eAmr;
            break;
        case NEXUS_AudioCodec_eDra:
            eAlgoType = BRAP_DSPCHN_AudioType_eDra;
            break;
        case NEXUS_AudioCodec_eCook:
            eAlgoType = BRAP_DSPCHN_AudioType_eRealAudioLbr;
            break;
#if BRAP_VER >= 4 && BCHP_CHIP == 35230
        case NEXUS_AudioCodec_eAdpcm:
            eAlgoType = BRAP_DSPCHN_AudioType_eAdpcm;
            break;
#endif
        default:
            return BERR_TRACE(BERR_INVALID_PARAMETER);
        }
    }

    /* Save Program */
    handle->programSettings = *pProgram;

    /* Setup for the proper codec */
    errCode = NEXUS_AudioDecoder_P_SetupCodec(handle, eAlgoType, streamType);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        goto err_codec;
    }

    /* Determine TSM mode */
    if ( NULL == pProgram->stcChannel ||
         NULL != NEXUS_GetEnv("force_vsync") ||
         (pProgram->pidChannel && (pidChannelStatus.transportType == NEXUS_TransportType_eEs ||
                                   pidChannelStatus.originalTransportType == NEXUS_TransportType_eWav ||
                                   pidChannelStatus.originalTransportType == NEXUS_TransportType_eEs))
#if 0
/* 20081216 bandrews - all inputs use TSM now */
        || NULL != pProgram->input )
#else
        )
#endif
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
        NEXUS_OBJECT_ACQUIRE(handle, NEXUS_PidChannel, pProgram->pidChannel);
        if (!handle->savedRaveContext) {
            errCode = NEXUS_AudioDecoder_P_ConfigureRave(handle, handle->raveContext, pProgram, NULL);
            if (errCode)
            {
                errCode = BERR_TRACE(errCode);
                goto err_configure_audio;
            }
        }
        LOCK_TRANSPORT();
        errCode = NEXUS_Rave_GetStatus_priv(handle->raveContext, &raveStatus);
        UNLOCK_TRANSPORT();
        if (errCode)
        {
            errCode = BERR_TRACE(errCode);
            goto err_rave_status;
        }
        BKNI_Memcpy(&handle->audioParams.sXptContextMap, &raveStatus.xptContextMap, sizeof(raveStatus.xptContextMap));

        /* We're ready to start.  Build up lists of outputs to check for configuration changes. */
        errCode = NEXUS_AudioDecoder_P_Start(handle);
        if ( errCode )
        {
            errCode=BERR_TRACE(errCode);
            goto err_start;
        }
    }
    if (pProgram->stcChannel) {
        NEXUS_OBJECT_ACQUIRE(handle, NEXUS_StcChannel, pProgram->stcChannel);
    }

    handle->started = true;

    if (handle->index == 0) {
        BTRC_TRACE(ChnChange_DecodeStartAudio, STOP);
    }

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
err_configure_audio:
    if ( pProgram->pidChannel )
    {
        NEXUS_OBJECT_RELEASE(handle, NEXUS_PidChannel, pProgram->pidChannel);
    }
err_codec:
    BKNI_Memset(&handle->programSettings, 0, sizeof(handle->programSettings));
    return errCode;
}

static void NEXUS_AudioDecoder_P_StopStcChannel(NEXUS_AudioDecoderHandle handle)
{
    NEXUS_Module_Lock(g_NEXUS_audioModuleData.transport);
    if (handle->programSettings.pidChannel)
    {
        NEXUS_StcChannel_DisablePidChannel_priv(handle->programSettings.stcChannel, handle->programSettings.pidChannel);
    }
    if (handle->stc.connector)
    {
        NEXUS_StcChannel_DisconnectDecoder_priv(handle->stc.connector);
        handle->stc.connector = NULL;
    }
    NEXUS_Module_Unlock(g_NEXUS_audioModuleData.transport);
}

static NEXUS_Error NEXUS_AudioDecoder_P_StartStcChannel(NEXUS_AudioDecoderHandle handle)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_StcChannelDecoderConnectionSettings stcChannelConnectionSettings;
    NEXUS_AudioDecoderStartSettings * pProgram;

    pProgram = &handle->programSettings;

    NEXUS_Module_Lock(g_NEXUS_audioModuleData.transport);

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

    NEXUS_Module_Unlock(g_NEXUS_audioModuleData.transport);

err:
    return rc;
}

NEXUS_Error NEXUS_AudioDecoder_P_Start(
    NEXUS_AudioDecoderHandle handle
    )
{
    NEXUS_Error errCode;
    const NEXUS_AudioDecoderStartSettings *pProgram;
#if NEXUS_NUM_AUDIO_MIXERS
    bool mixedByDsp = false;
#endif

    BDBG_ASSERT(NULL != handle);

    BDBG_ENTER(NEXUS_AudioDecoder_P_Start);

    pProgram = &handle->programSettings;

    if ( !handle->programSettings.input )
    {
        /* If we're doing DSOLA, force simul outputs to be PCM */
        if ( handle->audioParams.sDspChParams.uiPBRate != BRAP_DSPCHN_PLAYBACKRATE_NORMAL &&
             handle->outputLists[NEXUS_AudioDecoderConnectorType_eStereo].outputs[0] != NULL &&
             handle->outputLists[NEXUS_AudioDecoderConnectorType_eCompressed].outputs[0] != NULL )
        {
            handle->connectors[NEXUS_AudioDecoderConnectorType_eCompressed].format = NEXUS_AudioInputFormat_ePcmStereo;
        }
        else
        {
            handle->connectors[NEXUS_AudioDecoderConnectorType_eCompressed].format = NEXUS_AudioInputFormat_eCompressed;
        }
    }

    if ( handle->programSettings.input || !handle->started || handle->outputLists[NEXUS_AudioDecoderConnectorType_eCompressed].outputs[0] != NULL )
    {
        int i;
        bool changed=false;
        bool hasLocalOutputs=false;
        bool compressedIsSimul=false;
        static NEXUS_AudioOutputList outputLists[NEXUS_AudioDecoderConnectorType_eMax];
        BRAP_ChannelConfigParams *pConfig;
        NEXUS_AudioOutputHandle output;

        pConfig = BKNI_Malloc(sizeof(*pConfig)); /* must malloc this large structure to avoid possible stack overflow */
        if ( NULL == pConfig )
        {
            return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        }

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
                BDBG_MSG(("NO Output change detected for connector type %d (firstOld=%p, firstNew=%p)", i, (void *)outputLists[i].outputs[0], (void *)handle->outputLists[i].outputs[0]));
            }
        }
        /* Check if compressed format has changed and remove compressed outputs if it has... */
        if ( handle->compressedFormat != handle->connectors[NEXUS_AudioDecoderConnectorType_eCompressed].format )
        {
            BDBG_MSG(("Compressed format changed from PCM<->Compressed.  Removing compressed outputs."));
            NEXUS_AudioInput_P_RemoveDestinations(&handle->connectors[NEXUS_AudioDecoderConnectorType_eCompressed]);
            handle->compressedFormat = handle->connectors[NEXUS_AudioDecoderConnectorType_eCompressed].format;
            changed = true;
        }

        /* 20111212 bandrews - the following code is concerned with stc programming priority.
        As such, the variable in use has been renamed to stcPriority.  DecoderType is now simply
        a mapping combining type and index */

        /* Determine mode to add new outputs */
        if ( outputLists[NEXUS_AudioDecoderConnectorType_eCompressed].outputs[0] != NULL &&
             outputLists[NEXUS_AudioDecoderConnectorType_eStereo].outputs[0] == NULL &&
             outputLists[NEXUS_AudioDecoderConnectorType_eMultichannel].outputs[0] == NULL )
        {
            /* Compressed output, but no stereo or multichannel.  This is a passthrough channel. */
            if ( handle->connectors[NEXUS_AudioDecoderConnectorType_eCompressed].format == NEXUS_AudioInputFormat_eCompressed )
            {
                BDBG_MSG(("Decoder %d is a passthrough channel", handle->index));
                handle->audioParams.sDspChParams.eDecodeMode = BRAP_DSPCHN_DecodeMode_ePassThru;
            }
            else
            {
                handle->audioParams.sDspChParams.eDecodeMode = BRAP_DSPCHN_DecodeMode_eDecode;
                BDBG_MSG(("Decoder %d is a decode channel (PCM data for passthrough)", handle->index));
            }

            handle->stc.priority = 1;
        }
        else
        {
            if ( handle->descriptorParent || handle->programSettings.secondaryDecoder )
            {
                /* AD child or secondary decoder.  This is decoder type audio2 */
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
                compressedIsSimul = true;
                handle->audioParams.sDspChParams.eDecodeMode = BRAP_DSPCHN_DecodeMode_eDecode;
                BDBG_MSG(("Decoder %d is a simul channel", handle->index));
            }
            else
            {
                /* Decode Mode */
                handle->audioParams.sDspChParams.eDecodeMode = BRAP_DSPCHN_DecodeMode_eDecode;
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

                for ( j = 0; j < NEXUS_AUDIO_MAX_OUTPUTS; j++ )
                {
                    output = handle->outputLists[i].outputs[j];
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
                output = outputLists[NEXUS_AudioDecoderConnectorType_eStereo].outputs[i];
                if ( NULL == output )
                {
                    break;
                }
                else
                {
                    BDBG_MSG(("Adding output port %d to decoder %d (pcm) %p", output->port, handle->index, (void *)output));
                    handle->outputLists[NEXUS_AudioDecoderConnectorType_eStereo].outputs[i] = output;
                }
            }

            /* TODO: Add multichannel support correctly... Need 3 ports, somehow coordinated with AudioOutput */
            for ( i = 0; i < NEXUS_AUDIO_MAX_OUTPUTS; i++ )
            {
                output = outputLists[NEXUS_AudioDecoderConnectorType_eMultichannel].outputs[i];
                if ( NULL == output )
                {
                    break;
                }
                else
                {
                    BDBG_MSG(("Adding output port %d to decoder %d (multichannel)", output->port, handle->index));
                    handle->outputLists[NEXUS_AudioDecoderConnectorType_eMultichannel].outputs[i] = output;
                }
            }

            /* Add compressed ports */
            for ( i = 0; i < NEXUS_AUDIO_MAX_OUTPUTS; i++ )
            {
                output = outputLists[NEXUS_AudioDecoderConnectorType_eCompressed].outputs[i];
                if ( NULL == output )
                {
                    break;
                }
                else
                {
                    BDBG_MSG(("Adding output port %d to decoder %d (compressed) %p", output->port, handle->index, (void *)output));
                    handle->outputLists[NEXUS_AudioDecoderConnectorType_eCompressed].outputs[i] = output;
                }
            }
        }

        if ( NULL == outputLists[NEXUS_AudioDecoderConnectorType_eStereo].outputs[0] &&
             NULL == outputLists[NEXUS_AudioDecoderConnectorType_eMultichannel].outputs[0] &&
             NULL == outputLists[NEXUS_AudioDecoderConnectorType_eCompressed].outputs[0] )
        {
            BDBG_ERR(("No outputs have been connected to this decoder."));
            BKNI_Free(pConfig);
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }

        /* Create association if one does not exist */
        if ( NULL == handle->association )
        {
            for ( i = 0; i < NEXUS_AudioDecoderConnectorType_eMax; i++ )
            {
                static NEXUS_AudioOutputList localOutputs;
                NEXUS_AudioInput_P_GetOutputs(&handle->connectors[i], &localOutputs, true);
                if ( localOutputs.outputs[0] )
                {
                    hasLocalOutputs = true;
                    break;
                }
            }
            /* We only need an association if we have locally connected outputs (not via mixer) */
            if ( hasLocalOutputs )
            {
                static BRAP_AssociatedChanSettings associationSettings;
                BDBG_MSG(("Has directly connected output/s"));
                BKNI_Memset(&associationSettings, 0, sizeof(associationSettings));
                associationSettings.sChDetail[0].hRapCh = handle->rapChannel;
                handle->association = NEXUS_AudioAssociation_P_Create(&associationSettings);
                if ( NULL == handle->association )
                {
                    BKNI_Free(pConfig);
                    return BERR_TRACE(NEXUS_NOT_SUPPORTED);
                }
            }
        }

        /* Add post-processing if required now that destinations exist */
        if ( compressedIsSimul )
        {
            BRAP_GetCurrentProcessingStageSettings(handle->converter, &g_NEXUS_StageSettings);
            switch ( handle->programSettings.codec )
            {
#if BRAP_VER < 4
/* With RAAGA, only generic passthru is used it's not codec specific */
            case NEXUS_AudioCodec_eAc3Plus:
                BDBG_MSG(("Using DDP to AC3 converter stage"));
                g_NEXUS_StageSettings.eAudioProcessing = BRAP_ProcessingType_eConvertDdpToAc3;
                break;
            case NEXUS_AudioCodec_eWmaPro:
                BDBG_MSG(("Using WMA Pro passthru stage"));
                g_NEXUS_StageSettings.eAudioProcessing = BRAP_ProcessingType_eWmaProPassThru;
                break;
#endif
            default:
                BDBG_MSG(("Using generic passthru stage"));
                g_NEXUS_StageSettings.eAudioProcessing = BRAP_ProcessingType_eGenericPassthru;
                break;
            }
            errCode = BRAP_SetProcessingStageSettings(handle->converter, &g_NEXUS_StageSettings);
            if ( errCode )
            {
                BKNI_Free(pConfig);
                return BERR_TRACE(errCode);
            }

            (void)BRAP_GetCurrentChannelConfig(handle->rapChannel, handle->audioParams.sDspChParams.eType, pConfig);
            switch ( handle->programSettings.codec )
            {
            case NEXUS_AudioCodec_eAc3Plus:
#if BRAP_VER < 4
                pConfig->uChConfigParams.sDecConfigParams.uConfigParams.sAc3PlusConfigParams.bUseDDConvert =
                    (handle->compressedFormat == NEXUS_AudioInputFormat_eCompressed)?true:false;
#endif
                break;
            case NEXUS_AudioCodec_eWmaPro:
                pConfig->uChConfigParams.sDecConfigParams.uConfigParams.sWmaProConfigParams.eDecodeMode =
                    (handle->compressedFormat == NEXUS_AudioInputFormat_eCompressed)?BRAP_DSPCHN_WmaProDecodeMode_eSimulMode:BRAP_DSPCHN_WmaProDecodeMode_eDecode;
                break;
            default:
                /* Do nothing */
                break;
            }

            errCode = BRAP_SetChannelConfig(handle->rapChannel, pConfig);
            if ( errCode )
            {
                errCode = BERR_TRACE(errCode);
            }

        }
        else if ( handle->audioParams.sDspChParams.eDecodeMode == BRAP_DSPCHN_DecodeMode_ePassThru )
        {
            bool hbr = (handle->audioParams.sDspChParams.eType == BRAP_DSPCHN_AudioType_eAc3Plus) ? true : false;
            for ( i = 0; outputLists[NEXUS_AudioDecoderConnectorType_eCompressed].outputs[i] != NULL; i++ )
            {
                BDBG_MSG(("Setting Hbr mode for output port %d to %d",
                          outputLists[NEXUS_AudioDecoderConnectorType_eCompressed].outputs[i]->port, hbr));
                errCode = NEXUS_AudioOutput_P_SetHbrMode(outputLists[NEXUS_AudioDecoderConnectorType_eCompressed].outputs[i], hbr);
                if ( errCode )
                {
                    BKNI_Free(pConfig);
                    return BERR_TRACE(errCode);
                }
            }
        }

        /* Must compute this here for downmix checking later.  May compute twice, but this is harmless. */
        NEXUS_AudioDecoder_P_SetOutputMode(handle);
#if (NEXUS_WMA)
        if ( handle->programSettings.codec == NEXUS_AudioCodec_eWmaStdTs || handle->programSettings.codec == NEXUS_AudioCodec_eWmaStd )
        {
            (void) BRAP_GetCurrentChannelConfig(handle->rapChannel, handle->audioParams.sDspChParams.eType, pConfig);

            pConfig->uChConfigParams.sDecConfigParams.uConfigParams.sWmaStdConfigParams.eWmaIpType =
                handle->programSettings.codec == NEXUS_AudioCodec_eWmaStdTs ? BRAP_DSPCHN_WMAIpType_eTS : BRAP_DSPCHN_WMAIpType_eASF;

            errCode = BRAP_SetChannelConfig(handle->rapChannel, pConfig);
            if ( errCode )
            {
                BDBG_ERR(( "Failed setting WMA-TS" ));
                errCode = BERR_TRACE(errCode);
            }
        }
#endif

        /* Notify downstream objects we're about to start - should be done after outputs are connected on 7400/7401 */
        /* This will trigger connections on the 7405/3563/... */
        for ( i = 0; i < NEXUS_AudioDecoderConnectorType_eMax; i++ )
        {
            #define NEXUS_AUDIO_DECODER_MAX_PROCESSING_STAGES 2
            int numStages=0;
            static BRAP_ProcessingStageHandle stages[NEXUS_AUDIO_DECODER_MAX_PROCESSING_STAGES];
            BRAP_ChannelHandle adPair=NULL;
            BRAP_ProcessingStageHandle adStage=NULL;
            BKNI_Memset(stages, 0, sizeof(stages));

            /* Only call this for inputs about to actually start */
            if ( outputLists[i].outputs[0] != NULL )
            {
                BDBG_MSG(("Preparing to start path %d", i));

                if ( i == NEXUS_AudioDecoderConnectorType_eCompressed )
                {
                    /* coverity[dead_error_line: FALSE] */
                    /* coverity[dead_error_condition: FALSE] */
                    if ( compressedIsSimul &&
                         handle->connectors[NEXUS_AudioDecoderConnectorType_eCompressed].format == NEXUS_AudioInputFormat_eCompressed )
                    {
                        BDBG_MSG(("Adding converter stage to compressed path"));
                        stages[numStages++] = handle->converter;
                    }
                }
                else if ( handle->audioParams.sDspChParams.eType == BRAP_DSPCHN_AudioType_eMpeg ||
                          handle->audioParams.sDspChParams.eType == BRAP_DSPCHN_AudioType_eAacAdts ||
                          handle->audioParams.sDspChParams.eType == BRAP_DSPCHN_AudioType_eAacLoas ||
                          handle->audioParams.sDspChParams.eType == BRAP_DSPCHN_AudioType_eAacSbrAdts ||
                          handle->audioParams.sDspChParams.eType == BRAP_DSPCHN_AudioType_eAacSbrLoas )
                {
                    /* Activate AD only on PCM paths and for MPEG streams */
                    if ( handle->settings.descriptionDecoder || handle->descriptorParent )
                    {
                        /* AD Primary or descriptor channel.  Use appropriate post-processing. */
                        adStage = handle->descriptorProcessor;
                        adPair = (handle->settings.descriptionDecoder)?handle->settings.descriptionDecoder->rapChannel:handle->descriptorParent->rapChannel;
                    }
                }
                if ( i == NEXUS_AudioDecoderConnectorType_eStereo )
                {
                    bool downmix = false;

#if BRAP_VER >= 4
                    if ( handle->enableStereoDownmixPath )
#else
                    if ( handle->audioParams.sDspChParams.bEnableStereoDownmixPath )
#endif
                    {
                        downmix = true;
                    }

                    NEXUS_AudioInput_P_PrepareToStartWithProcessing(
                        &handle->connectors[i],
                        handle->association,
                        stages,
                        numStages,
                        adStage,
                        adPair,
                        downmix,
                        NEXUS_Audio_P_AudioTypeToCodec(handle->audioParams.sDspChParams.eType));
                }
                else
                {
                    NEXUS_AudioInput_P_PrepareToStartWithProcessing(
                        &handle->connectors[i],
                        handle->association,
                        stages,
                        numStages,
                        NULL,
                        NULL,
                        false,
                        NEXUS_Audio_P_AudioTypeToCodec(handle->audioParams.sDspChParams.eType));
                }
                #endif
            }
        }

#if NEXUS_NUM_AUDIO_MIXERS
        mixedByDsp = NEXUS_AudioDecoder_P_IsMixedByDspMixer(handle);
        if ( mixedByDsp )
        {
            /* PPMCorrection caused output level changed (phase difference between main/secondary) after stream wrap around */
            handle->audioParams.sDspChParams.bPPMCorrection = false;
        }
        else
        {
            handle->audioParams.sDspChParams.bPPMCorrection = true;
        }
#endif
        /* PPMCorrection should not be enabled with BTSC encoding. */
        if (NEXUS_AudioDecoder_P_FindBtsc(handle))
        {
            handle->audioParams.sDspChParams.bPPMCorrection = false;
        }

        /* Check for MS10 and MS11 specifics */
        switch ( handle->audioParams.sDspChParams.eType )
        {
        case BRAP_DSPCHN_AudioType_eAacSbrAdts:
        case BRAP_DSPCHN_AudioType_eAacSbrLoas:
            (void)BRAP_GetCurrentChannelConfig(handle->rapChannel, handle->audioParams.sDspChParams.eType, pConfig);
            if ( NEXUS_AudioInput_P_FindByType(&handle->connectors[NEXUS_AudioDecoderConnectorType_eStereo], NEXUS_AudioInputType_eAc3Encode) ||
                 NEXUS_AudioInput_P_FindByType(&handle->connectors[NEXUS_AudioDecoderConnectorType_eMultichannel], NEXUS_AudioInputType_eAc3Encode) )
            {
                pConfig->uChConfigParams.sDecConfigParams.uConfigParams.sAacSbrConfigParams.bDolbyTranscodeEnabled = true;
            }
            else
            {
                pConfig->uChConfigParams.sDecConfigParams.uConfigParams.sAacSbrConfigParams.bDolbyTranscodeEnabled = false;
            }

            /* Default to single decode */
            pConfig->uChConfigParams.sDecConfigParams.uConfigParams.sAacSbrConfigParams.bMsUsageMode = false;

#if defined(RAP_MS10_SUPPORT) || defined(RAP_MS11_SUPPORT)
            /* Look for multistream decoding using the FW mixer */
            if (pProgram->mixingMode == NEXUS_AudioDecoderMixingMode_eDescription)
            {
                pConfig->uChConfigParams.sDecConfigParams.uConfigParams.sAacSbrConfigParams.bMsUsageMode = true;
            }
#endif

            errCode = BRAP_SetChannelConfig(handle->rapChannel, pConfig);
            if ( errCode )
            {
                errCode = BERR_TRACE(errCode);
            }
            break;

#if BCHP_CHIP != 7422 && BCHP_CHIP != 7425
        case BRAP_DSPCHN_AudioType_eAc3:
        case BRAP_DSPCHN_AudioType_eAc3Plus:
            BDBG_CASSERT(sizeof(BRAP_DSPCHN_Ac3PlusConfigParams) == sizeof(BRAP_DSPCHN_Ac3ConfigParams));
            (void)BRAP_GetCurrentChannelConfig(handle->rapChannel, handle->audioParams.sDspChParams.eType, pConfig);

            /* Default to single decode */
            pConfig->uChConfigParams.sDecConfigParams.uConfigParams.sAc3ConfigParams.bMsUsageMode = false;

#if defined(RAP_MS10_SUPPORT) || defined(RAP_MS11_SUPPORT)
            /* Look for multistream decoding using the FW mixer */
            if (pProgram->mixingMode == NEXUS_AudioDecoderMixingMode_eDescription)
            {
                pConfig->uChConfigParams.sDecConfigParams.uConfigParams.sAc3ConfigParams.bMsUsageMode = true;
            }
#endif
            errCode = BRAP_SetChannelConfig(handle->rapChannel, pConfig);
            if ( errCode )
            {
                errCode = BERR_TRACE(errCode);
            }
            break;
#endif
        default:
            break;
        }
        BKNI_Free(pConfig);
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
    if (handle->index == 0) {
        BTRC_TRACE(ChnChange_DecodeStartAudioRAP, START);
    }

    /* Setup proper decoder output mode */
    NEXUS_AudioDecoder_P_SetOutputMode(handle);

    /* Activate Low Delay mode only for HDMI/SPDIF inputs */
    if ( handle->programSettings.input )
    {
        switch (handle->compressedFormat)
        {
        case NEXUS_AudioInputFormat_ePcmStereo:
        case NEXUS_AudioInputFormat_ePcm5_1:
        case NEXUS_AudioInputFormat_ePcm7_1:
#if BRAP_VER >= 4
        case NEXUS_AudioInputFormat_eCompressed:
            handle->audioParams.bLowDelayEnableMode = true;
#else
            handle->audioParams.eLowDelayEnableMode = BRAP_DSPCHN_LowDelayMode_eEnabled;
#endif
            break;
        default:
#if BRAP_VER >= 4
            handle->audioParams.bLowDelayEnableMode = false;
#else
            handle->audioParams.eLowDelayEnableMode = BRAP_DSPCHN_LowDelayMode_eDisabled;
#endif
            break;
        }
    }
    else
    {
#if BRAP_VER >= 4
            handle->audioParams.bLowDelayEnableMode = false;
#else
        handle->audioParams.eLowDelayEnableMode = BRAP_DSPCHN_LowDelayMode_eDisabled;
#endif
    }

#if NEXUS_HAS_SYNC_CHANNEL
    if (handle->sync.startMuted)
    {
        /* if sync says to start muted, rearm the sync mute during start */
        handle->sync.mute = true;
        BDBG_MSG(("Sync requested to start %u muted", handle->index));
    }

    /* apply the mute */
    (void)NEXUS_AudioDecoder_P_SetCompressedSyncMute(handle, handle->sync.mute);
#endif

    /* this code will apply sync mute to mixers of any PCM outputs */
    /* TODO: was this called for any other reason than to mute for sync? */
    NEXUS_AudioDecoder_ApplySettings_priv(handle);

    BDBG_MSG(("Calling BRAP_StartChannel"));

    handle->locked = false;
    handle->numFifoOverflows = handle->numFifoUnderflows = 0;
    errCode = BRAP_StartChannel(handle->rapChannel, &handle->audioParams);
    if (handle->index == 0) {
        BTRC_TRACE(ChnChange_DecodeStartAudioRAP, STOP);
    }
    if ( errCode && !handle->started ) { errCode = BERR_TRACE(errCode); goto err_dec_start; }

#if NEXUS_HAS_AUDIO_TSM_LOG_SUPPORT
    if (NEXUS_GetEnv("rap_tsmlog"))
    {
        /* Enable TSM Log */
        errCode = BRAP_EnableTsmLog(handle->rapChannel, true);
        if(errCode != BERR_SUCCESS)
        {
            BDBG_ERR(("RAP TSM Log enable failed for channel"));
        }
    }
#endif

    if ( handle->programSettings.pidChannel )
    {
        LOCK_TRANSPORT();
        NEXUS_Rave_Enable_priv(handle->raveContext);
        UNLOCK_TRANSPORT();
    }
    handle->running = true;
    if (handle->index == 0) {
        BTRC_TRACE(ChnChange_DecodeFirstAudio, START);
    }

    if ( handle->programSettings.pidChannel )
    {
        (void)NEXUS_PidChannel_ConsumerStarted(handle->programSettings.pidChannel);
    }

    errCode = NEXUS_AudioDecoder_P_SetPtsOffset(handle);
    if (errCode) return errCode;

    /* PR:49294 sync/astm must know that audio has been started *every* time it is started */
    if (handle->sync.settings.startCallback_isr) {
        BKNI_EnterCriticalSection();
        (*handle->sync.settings.startCallback_isr)(handle->sync.settings.callbackContext, 0);
        BKNI_LeaveCriticalSection();
    }

#if NEXUS_HAS_ASTM
    handle->astm.status.started = true;

    if (handle->astm.settings.lifecycle_isr)
    {
        BDBG_MSG(("Audio channel %p is notifying ASTM of its start action", (void *)handle));
            BKNI_EnterCriticalSection();
            (*handle->astm.settings.lifecycle_isr)(handle->astm.settings.callbackContext, 0);
            BKNI_LeaveCriticalSection();
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


/***************************************************************************
Summary:
    Stop deocding the current program
See Also:
    NEXUS_AudioDecoder_Start
 ***************************************************************************/
void NEXUS_AudioDecoder_Stop(
    NEXUS_AudioDecoderHandle handle
    )
{
    NEXUS_Error errCode;

    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioDecoder);

    BDBG_ENTER(NEXUS_AudioDecoder_Stop);


    if ( !handle->started && !handle->suspended )
    {
        BDBG_ERR(("Decoder not started or suspended"));
        return;
    }

    if (handle->index == 0) {
        BTRC_TRACE(ChnChange_Total_Audio, START);
        BTRC_TRACE(ChnChange_DecodeStopAudio, START);
    }

    #if NEXUS_HAS_DIGITAL_AUDIO_INPUTS
    if ( !handle->suspended )
    {
        if ( handle->programSettings.input )
        {
            BRAP_DisableSpdifRxInputDetection(g_NEXUS_audioModuleData.hRap);
            g_spdifDecoder = NULL;
        }
    }
    #endif

    errCode = NEXUS_AudioDecoder_P_Stop(handle, true);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
    }

    /* Clear trick mutes after the decoder has stopped */
    NEXUS_AudioDecoder_P_SetTrickMute(handle, false);
    NEXUS_AudioDecoder_P_SetCompressedMute(handle, false);

    handle->running = false;
    handle->started = false;
    handle->trickForceStopped = false; /* do we need to forcedly unmute on Stop, in a way it helps if in a PIP change mode decoder is moved from trickmode on one channel to normal mode on another channel, however it hurts if one stops decoder just in order to change a PID/ audio program */
    handle->suspended = false;

    if ( handle->programSettings.pidChannel )
    {
        LOCK_TRANSPORT();
        NEXUS_Rave_RemovePidChannel_priv(handle->raveContext);
        UNLOCK_TRANSPORT();
        NEXUS_OBJECT_RELEASE(handle, NEXUS_PidChannel, handle->programSettings.pidChannel);
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

    if ( handle->programSettings.stcChannel )
    {
        NEXUS_OBJECT_RELEASE(handle, NEXUS_StcChannel, handle->programSettings.stcChannel);
    }

    if (handle->index == 0) {
        BTRC_TRACE(ChnChange_DecodeStopAudio, STOP);
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
    NEXUS_Error errCode;

    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioDecoder);

    BDBG_ENTER(NEXUS_AudioDecoder_Suspend);
    if ( !handle->started )
    {
        BDBG_ERR(("Decoder not started"));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    if (handle->index == 0) {
        BTRC_TRACE(ChnChange_Total_Audio, START);
        BTRC_TRACE(ChnChange_DecodeStopAudio, START);
    }

#if NEXUS_HAS_DIGITAL_AUDIO_INPUTS
    if ( handle->programSettings.input )
    {
        BRAP_DisableSpdifRxInputDetection(g_NEXUS_audioModuleData.hRap);
        g_spdifDecoder = NULL;
    }
#endif

    errCode = NEXUS_AudioDecoder_P_Stop(handle, false);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
    }

    /* Clear trick mutes after the decoder has stopped */
    NEXUS_AudioDecoder_P_SetTrickMute(handle, false);
    NEXUS_AudioDecoder_P_SetCompressedMute(handle, false);

    handle->running = false;
    handle->started = false;
    handle->suspended = true;
    handle->trickForceStopped = false; /* do we need to forcedly unmute on Stop, in a way it helps if in a PIP change mode decoder is moved from trickmode on one channel to normal mode on another channel, however it hurts if one stops decoder just in order to change a PID/ audio program */

    if (handle->index == 0) {
        BTRC_TRACE(ChnChange_DecodeStopAudio, STOP);
    }
    BDBG_LEAVE(NEXUS_AudioDecoder_Suspend);
    return errCode;
}

/***************************************************************************
Summary:
Resumes decoding from the suspended state
***************************************************************************/
NEXUS_Error NEXUS_AudioDecoder_Resume(
    NEXUS_AudioDecoderHandle handle
    )
{
    NEXUS_Error errCode;
    int i;

    BDBG_ENTER(NEXUS_AudioDecoder_Resume);

    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioDecoder);

    if ( handle->started )
    {
        BDBG_ERR(("This decoder is already started.  Nothing to resume."));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    if ( !handle->suspended )
    {
        BDBG_ERR(("This decoder not suspended.  Nothing to resume."));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    if (handle->index == 0) {
        BTRC_TRACE(ChnChange_DecodeStartAudio, START);
    }

    NEXUS_AudioDecoder_P_TrickReset(handle); /* reset trick state on start */

#if NEXUS_HAS_DIGITAL_AUDIO_INPUTS
    if ( handle->programSettings->input )
    {
        BRAP_SpdifRx_InputDetectionSettings detectionSettings;

        if ( g_spdifDecoder )
        {
            BDBG_ERR(("Can only decode one digital input at a time"));
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }

        /* must init pidChannelStatus to something because it's used later */
        BKNI_Memset(&pidChannelStatus, 0, sizeof(pidChannelStatus));

        streamType = BAVC_StreamType_ePes;
        handle->isPlayback = false;
        handle->isDss = false;
        for ( i = 0; i < NEXUS_AudioDecoderConnectorType_eMax; i++ )
        {
            errCode = NEXUS_AudioInput_P_AddInput(&handle->connectors[i], handle->programSettings->input);
            if ( errCode )
            {
                errCode = BERR_TRACE(errCode);
            }
        }

        /* Setup format switch interrupt handling */
        BRAP_GetDefaultSpdifRxInputDetectionSettings(&detectionSettings);

        switch ( handle->programSettings->input->objectType )
        {
        #if NEXUS_NUM_HDMI_INPUTS > 0
        case NEXUS_AudioInputType_eHdmi:
            detectionSettings.eInputPort = BRAP_CapInputPort_eHdmi;
            {
                BRAP_InputPortConfig hdmiConfig;
                unsigned hdmiIndex=0;

                NEXUS_Module_Lock(g_NEXUS_audioModuleData.hdmiInput);
                NEXUS_HdmiInput_GetIndex_priv(handle->programSettings->input->pObjectHandle, &hdmiIndex);
                NEXUS_Module_Unlock(g_NEXUS_audioModuleData.hdmiInput);
                BRAP_GetCurrentInputConfig(g_NEXUS_audioModuleData.hRap, BRAP_CapInputPort_eHdmi, &hdmiConfig);
                hdmiConfig.sCapPortParams.uCapPortParams.sHdmiRxParams.eHdmiNo = hdmiIndex;
                BRAP_SetInputConfig(g_NEXUS_audioModuleData.hRap, &hdmiConfig);
            }
            break;
        #endif
        case NEXUS_AudioInputType_eSpdif:
            detectionSettings.eInputPort = BRAP_CapInputPort_eSpdif;
            break;
        default:
            BDBG_ERR(("Unsupported input type %d", handle->programSettings->input->objectType));
            return BERR_TRACE(BERR_INVALID_PARAMETER);
        }

        g_spdifDecoder = handle;
        #if 0
        /* TODO: Finish */
        handle->started = true;
        #endif

        errCode = BRAP_EnableSpdifRxInputDetection(g_NEXUS_audioModuleData.hRap, &detectionSettings);
        if ( errCode )
        {
            g_spdifDecoder = NULL;
            handle->started = false;
            return BERR_TRACE(errCode);
        }

        #if 0
        return BERR_SUCCESS;
        #endif
    }
#endif

    /* We're ready to start.  Build up lists of outputs to check for configuration changes. */
    errCode = NEXUS_AudioDecoder_P_Start(handle);
    if ( errCode )
    {
        errCode=BERR_TRACE(errCode);
        goto err_start;
    }

    handle->started = true;

    if (handle->index == 0) {
        BTRC_TRACE(ChnChange_DecodeStartAudio, STOP);
    }

    handle->started = true;
    handle->suspended = false;

    BDBG_LEAVE(NEXUS_AudioDecoder_Resume);

    /* Success */
    return BERR_SUCCESS;

err_start:
    if ( handle->programSettings.pidChannel )
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
    if ( handle->programSettings.stcChannel )
    {
        NEXUS_OBJECT_RELEASE(handle, NEXUS_StcChannel, handle->programSettings.stcChannel);
    }
    if ( handle->programSettings.pidChannel )
    {
        NEXUS_OBJECT_RELEASE(handle, NEXUS_PidChannel, handle->programSettings.pidChannel);
    }
    BKNI_Memset(&handle->programSettings, 0, sizeof(NEXUS_AudioDecoderStartSettings));
    handle->suspended = false;
    BDBG_LEAVE(NEXUS_AudioDecoder_Resume);
    return errCode;
}


NEXUS_Error NEXUS_AudioDecoder_P_Stop(NEXUS_AudioDecoderHandle handle, bool flush)
{
    NEXUS_Error errCode;

    BDBG_ENTER(NEXUS_AudioDecoder_P_Stop);

    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioDecoder);

    if ( handle->running )
    {

#if NEXUS_HAS_AUDIO_TSM_LOG_SUPPORT
    if (NEXUS_GetEnv("rap_tsmlog"))
    {
        /* Enable TSM Log */
        errCode = BRAP_EnableTsmLog(handle->rapChannel, false);
        if(errCode != BERR_SUCCESS)
        {
            BDBG_ERR(("RAP TSM Log disable failed for channel"));
        }
    }
#endif

        if ( handle->fifoWatchdog.timer )
        {
            NEXUS_CancelTimer(handle->fifoWatchdog.timer);
            handle->fifoWatchdog.timer = NULL;
        }
        if ( handle->statusCheckTimer )
        {
            NEXUS_CancelTimer(handle->statusCheckTimer);
            handle->statusCheckTimer = NULL;
        }
        handle->locked = false;
        errCode = BRAP_StopChannel(handle->rapChannel);
        if ( errCode )
        {
            errCode = BERR_TRACE(errCode);
        }

        #if (NEXUS_DDP) || (NEXUS_WMA)
        if ( (handle->outputLists[NEXUS_AudioDecoderConnectorType_eStereo].outputs[0] &&
             handle->outputLists[NEXUS_AudioDecoderConnectorType_eCompressed].outputs[0]) &&
             (handle->programSettings.codec == NEXUS_AudioCodec_eAc3Plus ||
              handle->programSettings.codec == NEXUS_AudioCodec_eWmaPro) )
        {
            BRAP_ChannelConfigParams *pConfig;
            /* Remove all post-processing */
            pConfig = BKNI_Malloc(sizeof(*pConfig)); /* must malloc this large structure to avoid possible stack overflow */
            if ( NULL == pConfig )
            {
                return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
            }
            (void)BRAP_GetCurrentChannelConfig(handle->rapChannel, BRAP_DSPCHN_AudioType_eAc3Plus, pConfig);
#if BRAP_VER < 4
            pConfig->uChConfigParams.sDecConfigParams.uConfigParams.sAc3PlusConfigParams.bUseDDConvert = false;
#endif
            errCode = BRAP_SetChannelConfig(handle->rapChannel, pConfig);
            if ( errCode )
            {
                errCode = BERR_TRACE(errCode);
            }
            (void)BRAP_GetCurrentChannelConfig(handle->rapChannel, BRAP_DSPCHN_AudioType_eWmaPro, pConfig);
            pConfig->uChConfigParams.sDecConfigParams.uConfigParams.sWmaProConfigParams.eDecodeMode = BRAP_DSPCHN_WmaProDecodeMode_eDecode;
            errCode = BRAP_SetChannelConfig(handle->rapChannel, pConfig);
            BKNI_Free(pConfig);
        }
        #endif
    }

    if ( handle->programSettings.pidChannel && flush )
    {
        LOCK_TRANSPORT();
        NEXUS_Rave_Disable_priv(handle->raveContext);
        NEXUS_Rave_Flush_priv(handle->raveContext);
        UNLOCK_TRANSPORT();
    }

    if ( handle->running )
    {
        handle->running = false;

        if (handle->programSettings.stcChannel)
        {
            NEXUS_AudioDecoder_P_StopStcChannel(handle);
        }

#if NEXUS_HAS_SYNC_CHANNEL
        /* PR:49294 sync/astm must know that audio has been started *every* time it is started */
        if (handle->sync.settings.startCallback_isr) {
            BKNI_EnterCriticalSection();
            (*handle->sync.settings.startCallback_isr)(handle->sync.settings.callbackContext, 0);
            BKNI_LeaveCriticalSection();
        }
#endif

#if NEXUS_HAS_ASTM
        handle->astm.status.started = false;

        if (handle->astm.settings.lifecycle_isr)
        {
            BDBG_MSG(("Audio channel %p is notifying ASTM of its stop action", (void *)handle));
                BKNI_EnterCriticalSection();
                (*handle->astm.settings.lifecycle_isr)(handle->astm.settings.callbackContext, 0);
                BKNI_LeaveCriticalSection();
        }

#endif
    }

    BDBG_LEAVE(NEXUS_AudioDecoder_P_Stop);

    return BERR_SUCCESS;
}

/***************************************************************************
Summary:
    Discards all data accumulated in the decoder buffer
See Also:
    NEXUS_AudioDecoder_Start
 ***************************************************************************/
NEXUS_Error NEXUS_AudioDecoder_Flush(
        NEXUS_AudioDecoderHandle handle
        )
{
    BERR_Code rc;
    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioDecoder);

    if ( !handle->started || !handle->running )
    {
        return BERR_SUCCESS;
    }

    rc = BRAP_DisableForFlush(handle->rapChannel);
    if (rc)
    {
        return BERR_TRACE(rc);
    }

    BDBG_ASSERT(handle->raveContext);

    if ( handle->programSettings.pidChannel )
    {
        LOCK_TRANSPORT();
        NEXUS_Rave_Disable_priv(handle->raveContext);
        NEXUS_Rave_Flush_priv(handle->raveContext);
        UNLOCK_TRANSPORT();
    }

    rc = BRAP_FlushChannel(handle->rapChannel);
    if (rc)
    {
        return BERR_TRACE(rc);
    } else {
        /* This is required by Raptor behavior (see JIRA:SW3548-2617) to
         * reset the PTS after a flush when the audio channel is paused.
         * The frame advance causes the PTS to then be invalidated.
         *
         * 100ms was chosen to be a reasonable frame size without being
         * too long, but the exact length shouldn't matter since the buffer
         * was already flushed.  This will be updated when the Raptor API
         * has a "advance one frame" API added to it.*/
        bool pause_state;
        if (!BRAP_GetPvrPauseStatus(handle->rapChannel,&pause_state)) {
            if (pause_state) {
                rc = BRAP_PvrFrameAdvance(handle->rapChannel,100);
                if (rc)
                {
                    return BERR_TRACE(rc);
                }
            }
        }
    }

    if ( handle->programSettings.pidChannel )
    {
        LOCK_TRANSPORT();
        NEXUS_Rave_Enable_priv(handle->raveContext);
        UNLOCK_TRANSPORT();
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
See Also:

 ***************************************************************************/
NEXUS_Error NEXUS_AudioDecoder_GetStatus(
    NEXUS_AudioDecoderHandle handle,
    NEXUS_AudioDecoderStatus *pStatus   /* [out] current status */
    )
{
    unsigned depth=0, size=0;
    BRAP_DSPCHN_PtsInfo ptsInfo;
    BERR_Code rc=BERR_SUCCESS;
    BRAP_DSPCHN_StreamInfo streamInfo;
    BRAP_Capabilities capabilities;
    BRAP_DSPCHN_DecoderVersionInfo decoderVersionInfo;

    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioDecoder);
    BDBG_ASSERT(NULL != pStatus);

    BKNI_Memset(pStatus, 0, sizeof(NEXUS_AudioDecoderStatus));
    pStatus->started = handle->started;
    pStatus->locked = handle->locked;
    pStatus->codec = handle->programSettings.codec;
    pStatus->ptsType = NEXUS_PtsType_eInterpolatedFromInvalidPTS;
    pStatus->numFifoOverflows = handle->numFifoOverflows;
    pStatus->numFifoUnderflows = handle->numFifoUnderflows;
    pStatus->numWatchdogs = g_numWatchdogs;

    if(!handle->running)
    {
        return BERR_SUCCESS;
    }

    #if NEXUS_HAS_DIGITAL_AUDIO_INPUTS
    if ( handle->programSettings.input )
    {
        BRAP_SpdifRx_Status rxStatus;
        BACM_SPDIF_ChannelStatus acmStatus;
        BRAP_SpdifRx_ChannelStatus channelStatus;
        rc = BRAP_GetSpdifRxStatus(g_NEXUS_audioModuleData.hRap, &rxStatus);
        if ( rc )
        {
            return BERR_TRACE(rc);
        }
        pStatus->codec = NEXUS_AudioCodec_eUnknown;
        if ( rxStatus.bSignalPresent || handle->programSettings.input->objectType == NEXUS_AudioInputType_eHdmi )
        {
            if ( rxStatus.eStreamType == BRAP_SpdifRx_StreamType_eLinear )
            {
                BDBG_MSG(("SPDIF Rx Reports PCM"));
                pStatus->codec = NEXUS_AudioCodec_ePcm;
                pStatus->sampleRate = NEXUS_AudioModule_P_Avc2SampleRate(rxStatus.eSampleRate);
                rc = BRAP_GetSpdifRxChannelStatus(g_NEXUS_audioModuleData.hRap, &channelStatus);
                if ( rc )
                {
                    return BERR_TRACE(rc);
                }
                BACM_SPDIF_InitChannelStatus(&acmStatus);
                BACM_SPDIF_ParseChannelStatus(&acmStatus, 8, (void *)channelStatus.aui32LtChnStatus);
                switch ( acmStatus.eWordLength )
                {
                default:
                    /* Values are already 0 */
                    break;
                case BACM_SPDIF_WordLength_eUnspecifiedOf20:
                    pStatus->maxWordLength = 20;
                    break;
                case BACM_SPDIF_WordLength_e16Of20:
                    pStatus->wordLength = 16;
                    pStatus->maxWordLength = 20;
                    break;
                case BACM_SPDIF_WordLength_e17Of20:
                    pStatus->wordLength = 17;
                    pStatus->maxWordLength = 20;
                    break;
                case BACM_SPDIF_WordLength_e18Of20:
                    pStatus->wordLength = 18;
                    pStatus->maxWordLength = 20;
                    break;
                case BACM_SPDIF_WordLength_e19Of20:
                    pStatus->wordLength = 19;
                    pStatus->maxWordLength = 20;
                    break;
                case BACM_SPDIF_WordLength_e20Of20:
                    pStatus->wordLength = 20;
                    pStatus->maxWordLength = 20;
                    break;
                case BACM_SPDIF_WordLength_eUnspecifiedOf24:
                    pStatus->maxWordLength = 24;
                    break;
                case BACM_SPDIF_WordLength_e20Of24:
                    pStatus->wordLength = 20;
                    pStatus->maxWordLength = 24;
                    break;
                case BACM_SPDIF_WordLength_e21Of24:
                    pStatus->wordLength = 21;
                    pStatus->maxWordLength = 24;
                    break;
                case BACM_SPDIF_WordLength_e22Of24:
                    pStatus->wordLength = 22;
                    pStatus->maxWordLength = 24;
                    break;
                case BACM_SPDIF_WordLength_e23Of24:
                    pStatus->wordLength = 23;
                    pStatus->maxWordLength = 24;
                    break;
                case BACM_SPDIF_WordLength_e24Of24:
                    pStatus->wordLength = 24;
                    pStatus->maxWordLength = 24;
                    break;
                }
                return BERR_SUCCESS; /* No other info to get for PCM streams */
            }
            else if ( rxStatus.eStreamType == BRAP_SpdifRx_StreamType_eCompressed )
            {
                BDBG_MSG(("SPDIF Rx Reports Compressed (%d)", rxStatus.sPc.eAudioType));
                pStatus->codec = NEXUS_Audio_P_AudioTypeToCodec(rxStatus.sPc.eAudioType);
#if BRAP_VER >= 4
                pStatus->sampleRate = NEXUS_AudioModule_P_Avc2SampleRate(rxStatus.eSampleRate);
#endif
                /* Continue to get other stream info below */
            }
        }
        if ( pStatus->codec == NEXUS_AudioCodec_eUnknown )
        {
            return BERR_SUCCESS;
        }
    }
/*#endif*/
    #endif

    BKNI_EnterCriticalSection();
    if ( handle->programSettings.pidChannel )
    {
        NEXUS_Rave_GetCdbBufferInfo_isr(handle->raveContext, &depth, &size);
    }
    if(handle->running)
    {
        rc = BRAP_GetCurrentPTS_isr(handle->rapChannel, &ptsInfo);
    }
    BKNI_LeaveCriticalSection();

    if ( rc == BERR_SUCCESS )
    {
        pStatus->pts = ptsInfo.ui32RunningPts;
        pStatus->ptsType = ptsInfo.ePtsType == BRAP_DSPCHN_PtsType_eCoded ? NEXUS_PtsType_eCoded :
            ptsInfo.ePtsType == BRAP_DSPCHN_PtsType_eInterpolatedFromValidPTS ? NEXUS_PtsType_eInterpolatedFromValidPTS : NEXUS_PtsType_eInterpolatedFromInvalidPTS;
        pStatus->ptsStcDifference = ptsInfo.i32Pts2StcPhase;
    }

    pStatus->fifoDepth = depth;
    pStatus->fifoSize = size;

    if ( handle->programSettings.pidChannel )
    {
        NEXUS_RaveStatus raveStatus;
        NEXUS_Module_Lock(g_NEXUS_audioModuleData.transport);
        NEXUS_Rave_GetAudioFrameCount_priv(handle->raveContext, &pStatus->queuedFrames);
        rc = NEXUS_Rave_GetStatus_priv(handle->raveContext, &raveStatus);
        NEXUS_Module_Unlock(g_NEXUS_audioModuleData.transport);
        if ( BERR_SUCCESS == rc )
        {
            if(raveStatus.numOutputBytes >= pStatus->fifoDepth) {
                pStatus->numBytesDecoded = raveStatus.numOutputBytes - pStatus->fifoDepth;
            } else {
                pStatus->numBytesDecoded = 0;
            }
        }
    }

    pStatus->tsm = handle->tsmEnabled;
    pStatus->timebase = handle->audioParams.eTimebase; /* BRAP_ChannelParams.eTimebase is misnamed. It is not a timebase (i.e. DPCR). It is an STC index. */
    pStatus->ptsErrorCount = handle->ptsErrorCount;

    pStatus->codec = NEXUS_Audio_P_AudioTypeToCodec(handle->audioParams.sDspChParams.eType);
#if 0
    BRAP_GetDecoderVersionNo(handle->rapChannel,&pStatus->VersionId[0],&pStatus->VersionId[1],&pStatus->VersionId[2],&pStatus->VersionId[3]);
#endif
    /* Get codec specifics */
    rc = BRAP_GetStreamInformation(handle->rapChannel, &streamInfo);
    if ( BERR_SUCCESS == rc )
    {
        unsigned frameLength, bitrate;

#if BRAP_VER >= 4 && NEXUS_HAS_DIGITAL_AUDIO_INPUTS
        /* For compressed source coming from SPDIFRx, we use the sampleRate from BRAP_SpdifRx_Status obtained above.
           It is found that the sampleRate obtained from RAP BRAP_DSPCHN_P_Get<Codec>Info is not updated as fast as
           from MAI_FORMAT. So when getting sampleRate in the SourceChange callback the value is not shown properly. */
        if ( NULL == handle->programSettings.input )
        {
            /* Use sampleRate read from BRAP_GetStreamInformation for non-Hdmi/Spdifrx case */
            pStatus->sampleRate = NEXUS_AudioModule_P_Avc2SampleRate(streamInfo.eSamplingRate);
        }
#else
        pStatus->sampleRate = NEXUS_AudioModule_P_Avc2SampleRate(streamInfo.eSamplingRate);
#endif

        pStatus->framesDecoded = streamInfo.sFrameDecAndErrInfo.uiTotalFramesDecoded;
        pStatus->frameErrors = streamInfo.sFrameDecAndErrInfo.uiTotalFramesInError;
        pStatus->dummyFrames = streamInfo.sFrameDecAndErrInfo.uiTotalFramesDummy;

        /* Convert codec back to nexus */
        pStatus->codec = NEXUS_Audio_P_AudioTypeToCodec(streamInfo.eType);

        /* Handle specifics per-codec */
        switch ( streamInfo.eType )
        {
        case BRAP_DSPCHN_AudioType_eMpeg:
            pStatus->codec = (streamInfo.uStreamInfo.sMpegInfo.eLayer == BRAP_DSPCHN_MpegAudioLayer_eMpegLayer3)?NEXUS_AudioCodec_eMp3:NEXUS_AudioCodec_eMpeg;
            pStatus->codecStatus.mpeg.channelMode = streamInfo.uStreamInfo.sMpegInfo.eChmod;
            pStatus->codecStatus.mpeg.layer = streamInfo.uStreamInfo.sMpegInfo.eLayer;
            pStatus->codecStatus.mpeg.emphasis = streamInfo.uStreamInfo.sMpegInfo.eEmphasisMode;
            pStatus->codecStatus.mpeg.original = streamInfo.uStreamInfo.sMpegInfo.bOriginal;
            pStatus->codecStatus.mpeg.bitrate = streamInfo.uStreamInfo.sMpegInfo.ui32BitRate;
            /* MPEG audio uses a CDB sync, so the frame count is bogus.  Calculate based on frame size and CDB depth */
            bitrate = (streamInfo.uStreamInfo.sMpegInfo.ui32BitRate>0)?1000*streamInfo.uStreamInfo.sMpegInfo.ui32BitRate:128000;
            if ( streamInfo.uStreamInfo.sMpegInfo.eLayer == BRAP_DSPCHN_MpegAudioLayer_eMpegLayer1 )
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
            BDBG_MSG(("Queued Frames %d bitrate %d index %d framelength %d samplerate %d", pStatus->queuedFrames, bitrate, streamInfo.uStreamInfo.sMpegInfo.ui32BitRateIndex, frameLength, pStatus->sampleRate));
            break;
        case BRAP_DSPCHN_AudioType_eAacLoas:
        case BRAP_DSPCHN_AudioType_eAacAdts:
        case BRAP_DSPCHN_AudioType_eAacSbrLoas:
        case BRAP_DSPCHN_AudioType_eAacSbrAdts:
            pStatus->codecStatus.aac.bitrate = streamInfo.uStreamInfo.sAacInfo.ui32BitRate;
            pStatus->codecStatus.aac.profile = streamInfo.uStreamInfo.sAacInfo.eProfile;
            pStatus->codecStatus.aac.numLfeChannels = streamInfo.uStreamInfo.sAacInfo.ui32LfeChannels;
            pStatus->codecStatus.aac.numBackChannels = streamInfo.uStreamInfo.sAacInfo.ui32BackChannels;
            pStatus->codecStatus.aac.numSideChannels = streamInfo.uStreamInfo.sAacInfo.ui32SideChannels;
            pStatus->codecStatus.aac.numFrontChannels = streamInfo.uStreamInfo.sAacInfo.ui32FrontChannels;
            pStatus->codecStatus.aac.lfe = streamInfo.uStreamInfo.sAacInfo.bLfeOn;
            pStatus->codecStatus.aac.acmod = streamInfo.uStreamInfo.sAacInfo.eAcmod;
            pStatus->codecStatus.aac.pseudoSurround = streamInfo.uStreamInfo.sAacInfo.bPseudoSurroundEnable;
            pStatus->codecStatus.aac.matrixIndex = streamInfo.uStreamInfo.sAacInfo.uiMatrixMixdownIdx;
            pStatus->codecStatus.aac.stereoMatrix = streamInfo.uStreamInfo.sAacInfo.bMatrixMixdownPresent;
            pStatus->codecStatus.aac.drc = streamInfo.uStreamInfo.sAacInfo.bDRCPresent;
            break;
        case BRAP_DSPCHN_AudioType_eAc3:
        case BRAP_DSPCHN_AudioType_eAc3Plus:
            pStatus->codec = (streamInfo.eType == BRAP_DSPCHN_AudioType_eAc3)?NEXUS_AudioCodec_eAc3:NEXUS_AudioCodec_eAc3Plus;
            pStatus->codecStatus.ac3.acmod = streamInfo.uStreamInfo.sAc3Info.eAcmod;
            pStatus->codecStatus.ac3.frameSizeCode = streamInfo.uStreamInfo.sAc3Info.ui32FrameSizeCode;
            pStatus->codecStatus.ac3.lfe = streamInfo.uStreamInfo.sAc3Info.bLfeOn;
            pStatus->codecStatus.ac3.copyright = streamInfo.uStreamInfo.sAc3Info.bCpyrhtPresent;
            pStatus->codecStatus.ac3.bitrate = NEXUS_AudioDecoder_P_GetAc3Bitrate(streamInfo.uStreamInfo.sAc3Info.ui32FrameSizeCode);
            pStatus->codecStatus.ac3.bitStreamId = streamInfo.uStreamInfo.sAc3Info.ui8BitStreamId;
            BDBG_CASSERT((int)NEXUS_AudioAc3DependentFrameChannelMap_eMax==(int)BRAP_DSPCHN_DependentFrameChannelMapMode_eMax);
            pStatus->codecStatus.ac3.dependentFrameChannelMap = streamInfo.uStreamInfo.sAc3Info.eDependentFrameChannelMapMode;
            break;
        case BRAP_DSPCHN_AudioType_eDtsBroadcast:
            pStatus->codecStatus.dts.crc = streamInfo.uStreamInfo.sDtsBroadcastInfo.bCrcFlag;
            pStatus->codecStatus.dts.numPcmBlocks = streamInfo.uStreamInfo.sDtsBroadcastInfo.ui32NoOfPcmBlocks;
            pStatus->codecStatus.dts.frameSize = streamInfo.uStreamInfo.sDtsBroadcastInfo.ui32FrameSize;
            BDBG_CASSERT(NEXUS_AudioDtsAmode_eOneCh_A == (NEXUS_AudioDtsAmode)BRAP_DSPCHN_DtsAmode_eOneCh_A);
            BDBG_CASSERT(NEXUS_AudioDtsAmode_eTwoCh_A_B == (NEXUS_AudioDtsAmode)BRAP_DSPCHN_DtsAmode_eTwoCh_A_B);
            BDBG_CASSERT(NEXUS_AudioDtsAmode_eTwoCh_L_R == (NEXUS_AudioDtsAmode)BRAP_DSPCHN_DtsAmode_eTwoCh_L_R);
            BDBG_CASSERT(NEXUS_AudioDtsAmode_eTwoCh_LpR_LmR == (NEXUS_AudioDtsAmode)BRAP_DSPCHN_DtsAmode_eTwoCh_LpR_LmR);
            BDBG_CASSERT(NEXUS_AudioDtsAmode_eTwoCh_LT_RT == (NEXUS_AudioDtsAmode)BRAP_DSPCHN_DtsAmode_eTwoCh_LT_RT);
            BDBG_CASSERT(NEXUS_AudioDtsAmode_eThreeCh_C_L_R == (NEXUS_AudioDtsAmode)BRAP_DSPCHN_DtsAmode_eThreeCh_C_L_R);
            BDBG_CASSERT(NEXUS_AudioDtsAmode_eThreeCh_L_R_S == (NEXUS_AudioDtsAmode)BRAP_DSPCHN_DtsAmode_eThreeCh_L_R_S);
            BDBG_CASSERT(NEXUS_AudioDtsAmode_eFourCh_C_L_R_S == (NEXUS_AudioDtsAmode)BRAP_DSPCHN_DtsAmode_eFourCh_C_L_R_S);
            BDBG_CASSERT(NEXUS_AudioDtsAmode_eFourCh_L_R_SL_SR == (NEXUS_AudioDtsAmode)BRAP_DSPCHN_DtsAmode_eFourCh_L_R_SL_SR);
            BDBG_CASSERT(NEXUS_AudioDtsAmode_eFiveCh_C_L_R_SL_SR == (NEXUS_AudioDtsAmode)BRAP_DSPCHN_DtsAmode_eFiveCh_C_L_R_SL_SR);
            BDBG_CASSERT(NEXUS_AudioDtsAmode_eSixCh_CL_CR_L_R_SL_SR == (NEXUS_AudioDtsAmode)BRAP_DSPCHN_DtsAmode_eSixCh_CL_CR_L_R_SL_SR);
            BDBG_CASSERT(NEXUS_AudioDtsAmode_eSixCh_C_L_R_LR_RR_OV == (NEXUS_AudioDtsAmode)BRAP_DSPCHN_DtsAmode_eSixCh_C_L_R_LR_RR_OV);
            BDBG_CASSERT(NEXUS_AudioDtsAmode_eSixCh_CF_CR_LF_RF_Lr_Rr == (NEXUS_AudioDtsAmode)BRAP_DSPCHN_DtsAmode_eSixCh_CF_CR_LF_RF_Lr_Rr);
            BDBG_CASSERT(NEXUS_AudioDtsAmode_eSevenCh_CL_C_CR_L_R_SL_SR == (NEXUS_AudioDtsAmode)BRAP_DSPCHN_DtsAmode_eSevenCh_CL_C_CR_L_R_SL_SR);
            BDBG_CASSERT(NEXUS_AudioDtsAmode_eEightCh_CL_CR_L_R_SL1_SL2_SR1_SR2 == (NEXUS_AudioDtsAmode)BRAP_DSPCHN_DtsAmode_eEightCh_CL_CR_L_R_SL1_SL2_SR1_SR2);
            BDBG_CASSERT(NEXUS_AudioDtsAmode_eEightCh_CL_C_CR_L_R_SL_S_SR == (NEXUS_AudioDtsAmode)BRAP_DSPCHN_DtsAmode_eEightCh_CL_C_CR_L_R_SL_S_SR);
            BDBG_CASSERT(NEXUS_AudioDtsAmode_eUserDefined == (NEXUS_AudioDtsAmode)BRAP_DSPCHN_DtsAmode_UserDefined);
            pStatus->codecStatus.dts.amode = streamInfo.uStreamInfo.sDtsBroadcastInfo.eAMode;
            pStatus->codecStatus.dts.downmixCoefficients = streamInfo.uStreamInfo.sDtsBroadcastInfo.bEmbeddedDownMixEn;
            pStatus->codecStatus.dts.drc = streamInfo.uStreamInfo.sDtsBroadcastInfo.bEmbeddedDrc;
            pStatus->codecStatus.dts.hdcdFormat = streamInfo.uStreamInfo.sDtsBroadcastInfo.bHdcdFormat;
            pStatus->codecStatus.dts.extensionPresent = streamInfo.uStreamInfo.sDtsBroadcastInfo.bExtAudio;
            switch ( streamInfo.uStreamInfo.sDtsBroadcastInfo.eExtAudioId )
            {
            case BRAP_DSPCHN_DtsExtAudId_XCh:  pStatus->codecStatus.dts.extensionDescriptor = NEXUS_AudioDtsExtension_eXCh; break;
            case BRAP_DSPCHN_DtsExtAudId_XXCh: pStatus->codecStatus.dts.extensionDescriptor = NEXUS_AudioDtsExtension_eXXCh; break;
            case BRAP_DSPCHN_DtsExtAudId_X96k: pStatus->codecStatus.dts.extensionDescriptor = NEXUS_AudioDtsExtension_eX96k; break;
            default:                           pStatus->codecStatus.dts.extensionDescriptor = NEXUS_AudioDtsExtension_eReserved; break;
            }
            pStatus->codecStatus.dts.version = streamInfo.uStreamInfo.sDtsBroadcastInfo.ui32VersionNumber;
            BDBG_CASSERT(BRAP_DSPCHN_DtsCopyHistory_Copy_prohibited == (BRAP_DSPCHN_DtsCopyHistory)NEXUS_AudioDtsCopyHistory_eCopyProhibited);
            BDBG_CASSERT(BRAP_DSPCHN_DtsCopyHistory_First_generation == (BRAP_DSPCHN_DtsCopyHistory)NEXUS_AudioDtsCopyHistory_eFirstGeneration);
            BDBG_CASSERT(BRAP_DSPCHN_DtsCopyHistory_Second_generation == (BRAP_DSPCHN_DtsCopyHistory)NEXUS_AudioDtsCopyHistory_eSecondGeneration);
            BDBG_CASSERT(BRAP_DSPCHN_DtsCopyHistory_Original_material == (BRAP_DSPCHN_DtsCopyHistory)NEXUS_AudioDtsCopyHistory_eOriginal);
            pStatus->codecStatus.dts.copyHistory = streamInfo.uStreamInfo.sDtsBroadcastInfo.eCopyHistory;
            switch ( streamInfo.uStreamInfo.sDtsBroadcastInfo.eSourcePcmResolution )
            {
            case BRAP_DSPCHN_DtsSourcePcm_ES1_16_bits:
                pStatus->codecStatus.dts.esFormat = true;
            case BRAP_DSPCHN_DtsSourcePcm_ES0_16_bits:
                pStatus->codecStatus.dts.pcmResolution = 16;
                break;
            case BRAP_DSPCHN_DtsSourcePcm_ES1_20_bits:
                pStatus->codecStatus.dts.esFormat = true;
            case BRAP_DSPCHN_DtsSourcePcm_ES0_20_bits:
                pStatus->codecStatus.dts.pcmResolution = 20;
                break;
            case BRAP_DSPCHN_DtsSourcePcm_ES1_24_bits:
                pStatus->codecStatus.dts.esFormat = true;
            case BRAP_DSPCHN_DtsSourcePcm_ES0_24_bits:
                pStatus->codecStatus.dts.pcmResolution = 24;
                break;
            default:
                break;
            }
            pStatus->codecStatus.dts.numChannels = streamInfo.uStreamInfo.sDtsBroadcastInfo.ui32NumChannels;
            pStatus->codecStatus.dts.lfe = streamInfo.uStreamInfo.sDtsBroadcastInfo.bLfeFlag;
            pStatus->codecStatus.dts.bitRate = streamInfo.uStreamInfo.sDtsBroadcastInfo.ui32BitRate;
            pStatus->codecStatus.dts.neo = streamInfo.uStreamInfo.sDtsBroadcastInfo.bDTSNeoEnable;
            pStatus->codecStatus.dts.pcmFrameSize = streamInfo.uStreamInfo.sDtsBroadcastInfo.ui32PCMFrameSize;
            break;
        case BRAP_DSPCHN_AudioType_eDtshd:
            pStatus->codecStatus.dts.crc = streamInfo.uStreamInfo.sDtsHdInfo.bCrcFlag;
            pStatus->codecStatus.dts.numPcmBlocks = streamInfo.uStreamInfo.sDtsHdInfo.ui32NoOfPcmBlocks;
            pStatus->codecStatus.dts.frameSize = streamInfo.uStreamInfo.sDtsHdInfo.ui32FrameSize;
            pStatus->codecStatus.dts.downmixCoefficients = streamInfo.uStreamInfo.sDtsHdInfo.bEmbeddedDownMixEn;
            pStatus->codecStatus.dts.drc = streamInfo.uStreamInfo.sDtsHdInfo.bEmbeddedDrc;
            pStatus->codecStatus.dts.hdcdFormat = streamInfo.uStreamInfo.sDtsHdInfo.bHdcdFormat;
            pStatus->codecStatus.dts.extensionPresent = streamInfo.uStreamInfo.sDtsHdInfo.bExtAudio;
            switch ( streamInfo.uStreamInfo.sDtsHdInfo.eExtAudioId )
            {
            case BRAP_DSPCHN_DtsExtAudId_XCh:  pStatus->codecStatus.dts.extensionDescriptor = NEXUS_AudioDtsExtension_eXCh; break;
            case BRAP_DSPCHN_DtsExtAudId_XXCh: pStatus->codecStatus.dts.extensionDescriptor = NEXUS_AudioDtsExtension_eXXCh; break;
            case BRAP_DSPCHN_DtsExtAudId_X96k: pStatus->codecStatus.dts.extensionDescriptor = NEXUS_AudioDtsExtension_eX96k; break;
            default:                           pStatus->codecStatus.dts.extensionDescriptor = NEXUS_AudioDtsExtension_eReserved; break;
            }
            pStatus->codecStatus.dts.version = streamInfo.uStreamInfo.sDtsHdInfo.ui32VersionNumber;
            pStatus->codecStatus.dts.copyHistory = streamInfo.uStreamInfo.sDtsHdInfo.eCopyHistory;
            switch ( streamInfo.uStreamInfo.sDtsHdInfo.eSourcePcmResolution )
            {
            case BRAP_DSPCHN_DtsSourcePcm_ES1_16_bits:
                pStatus->codecStatus.dts.esFormat = true;
            case BRAP_DSPCHN_DtsSourcePcm_ES0_16_bits:
                pStatus->codecStatus.dts.pcmResolution = 16;
                break;
            case BRAP_DSPCHN_DtsSourcePcm_ES1_20_bits:
                pStatus->codecStatus.dts.esFormat = true;
            case BRAP_DSPCHN_DtsSourcePcm_ES0_20_bits:
                pStatus->codecStatus.dts.pcmResolution = 20;
                break;
            case BRAP_DSPCHN_DtsSourcePcm_ES1_24_bits:
                pStatus->codecStatus.dts.esFormat = true;
            case BRAP_DSPCHN_DtsSourcePcm_ES0_24_bits:
                pStatus->codecStatus.dts.pcmResolution = 24;
                break;
            default:
                break;
            }
            pStatus->codecStatus.dts.numChannels = streamInfo.uStreamInfo.sDtsHdInfo.ui32NumChannels;
            pStatus->codecStatus.dts.lfe = streamInfo.uStreamInfo.sDtsHdInfo.bLfeFlag;
            pStatus->codecStatus.dts.bitRate = streamInfo.uStreamInfo.sDtsHdInfo.ui32BitRate;
            pStatus->codecStatus.dts.neo = streamInfo.uStreamInfo.sDtsHdInfo.bDTSNeoEnable;
            pStatus->codecStatus.dts.pcmFrameSize = streamInfo.uStreamInfo.sDtsHdInfo.ui32PCMFrameSize;
            break;
        case BRAP_DSPCHN_AudioType_eWmaStd:
            pStatus->codecStatus.wma.bitRate = streamInfo.uStreamInfo.sWmaStdInfo.ui32BitRate;
            pStatus->codecStatus.wma.original = streamInfo.uStreamInfo.sWmaStdInfo.ui32OriginalCopy;
            pStatus->codecStatus.wma.copyright = streamInfo.uStreamInfo.sWmaStdInfo.ui32Copyright;
            pStatus->codecStatus.wma.crc = streamInfo.uStreamInfo.sWmaStdInfo.bCrcPresent;
            pStatus->codecStatus.wma.stereo = (streamInfo.uStreamInfo.sWmaStdInfo.eAcmod == BRAP_DSPCHN_WmaStdAcmod_eTwoCh)?true:false;
            pStatus->codecStatus.wma.version = (streamInfo.uStreamInfo.sWmaStdInfo.eVersion == BRAP_DSPCHN_WmaStdVersion_eVersion2)?2:1;
            break;
        case BRAP_DSPCHN_AudioType_eWmaPro:
            pStatus->codecStatus.wmaPro.original = streamInfo.uStreamInfo.sWmaProInfo.ui32OriginalCopy;
            pStatus->codecStatus.wmaPro.copyright = streamInfo.uStreamInfo.sWmaProInfo.ui32Copyright;
            pStatus->codecStatus.wmaPro.crc = streamInfo.uStreamInfo.sWmaProInfo.bCrcPresent;
            switch ( streamInfo.uStreamInfo.sWmaProInfo.eStereoMode )
            {
            case BRAP_DSPCHN_WmaProStereoMode_eAuto:
                pStatus->codecStatus.wmaPro.stereoMode = NEXUS_AudioWmaProStereoMode_eAuto;
                break;
            case BRAP_DSPCHN_WmaProStereoMode_eLtRt:
                pStatus->codecStatus.wmaPro.stereoMode = NEXUS_AudioWmaProStereoMode_eLtRt;
                break;
            case BRAP_DSPCHN_WmaProStereoMode_eLoRo:
                pStatus->codecStatus.wmaPro.stereoMode = NEXUS_AudioWmaProStereoMode_eLoRo;
                break;
            default:
                pStatus->codecStatus.wmaPro.stereoMode = NEXUS_AudioWmaProStereoMode_eMax;
                break;
            }
            pStatus->codecStatus.wmaPro.bitRate = streamInfo.uStreamInfo.sWmaProInfo.ui32BitRate;
            BDBG_CASSERT(NEXUS_AudioWmaProAcmod_eUndefined == (NEXUS_AudioWmaProAcmod)BRAP_DSPCHN_WmaProAcmod_NotDefined);
            BDBG_CASSERT(NEXUS_AudioWmaProAcmod_e1_0_C == (NEXUS_AudioWmaProAcmod)BRAP_DSPCHN_WmaProAcmod_eOneCentre_1_0_C);
            BDBG_CASSERT(NEXUS_AudioWmaProAcmod_e2_0_LR == (NEXUS_AudioWmaProAcmod)BRAP_DSPCHN_WmaProAcmod_eTwoCh_2_0_L_R);
            BDBG_CASSERT(NEXUS_AudioWmaProAcmod_e3_0_LCR == (NEXUS_AudioWmaProAcmod)BRAP_DSPCHN_WmaProAcmod_eThreeCh_3_0_L_C_R);
            BDBG_CASSERT(NEXUS_AudioWmaProAcmod_e2_1_LRS == (NEXUS_AudioWmaProAcmod)BRAP_DSPCHN_WmaProAcmod_eThreeCh_2_1_L_R_S);
            BDBG_CASSERT(NEXUS_AudioWmaProAcmod_e3_1_LCRS == (NEXUS_AudioWmaProAcmod)BRAP_DSPCHN_WmaProAcmod_eFourCh_3_1_L_C_R_S);
            BDBG_CASSERT(NEXUS_AudioWmaProAcmod_e2_2_LRLsRs == (NEXUS_AudioWmaProAcmod)BRAP_DSPCHN_WmaProAcmod_eFourCh_2_2_L_R_SL_SR);
            BDBG_CASSERT(NEXUS_AudioWmaProAcmod_e3_2_LCRLsRs == (NEXUS_AudioWmaProAcmod)BRAP_DSPCHN_WmaProAcmod_eFiveCh_3_2_L_C_R_SL_SR);
            pStatus->codecStatus.wmaPro.acmod = streamInfo.uStreamInfo.sWmaProInfo.eAcmod;
            break;
        case BRAP_DSPCHN_AudioType_ePcmWav:
            pStatus->codecStatus.pcmWav.numChannels = streamInfo.uStreamInfo.sPcmWavInfo.ui32NumChannels;
            break;
        case BRAP_DSPCHN_AudioType_eAmr:
            pStatus->codecStatus.amr.bitRate = streamInfo.uStreamInfo.sAmrInfo.ui32BitRate;
            break;
        case BRAP_DSPCHN_AudioType_eDra:
            pStatus->codecStatus.dra.frameSize = streamInfo.uStreamInfo.sDraInfo.uiFrameSize;
            pStatus->codecStatus.dra.numBlocks = streamInfo.uStreamInfo.sDraInfo.uiNumBlocks;
            BDBG_CASSERT(NEXUS_AudioDraAcmod_e1_0_C == (NEXUS_AudioDraAcmod)BRAP_DSPCHN_DraAcmod_Cf);
            BDBG_CASSERT(NEXUS_AudioDraAcmod_e2_0_LR == (NEXUS_AudioDraAcmod)BRAP_DSPCHN_DraAcmod_Lf_Rf);
            BDBG_CASSERT(NEXUS_AudioDraAcmod_e2_1_LRS == (NEXUS_AudioDraAcmod)BRAP_DSPCHN_DraAcmod_Lf_Rf_Cr);
            BDBG_CASSERT(NEXUS_AudioDraAcmod_e2_2_LRLrRr == (NEXUS_AudioDraAcmod)BRAP_DSPCHN_DraAcmod_Lf_Rf_Lr_Rr);
            BDBG_CASSERT(NEXUS_AudioDraAcmod_e3_2_LRLrRrC == (NEXUS_AudioDraAcmod)BRAP_DSPCHN_DraAcmod_Lf_Rf_Lr_Rr_Cf);
            BDBG_CASSERT(NEXUS_AudioDraAcmod_e3_3_LRLrRrCrC == (NEXUS_AudioDraAcmod)BRAP_DSPCHN_DraAcmod_Lf_Rf_Lr_Rr_Cr_Cf);
            BDBG_CASSERT(NEXUS_AudioDraAcmod_e5_2_LRLrRrLsRsC == (NEXUS_AudioDraAcmod)BRAP_DSPCHN_DraAcmod_Lf_Rf_Lr_Rr_Ls_Rs_Cf);
            BDBG_CASSERT(NEXUS_AudioDraAcmod_e5_3_LRLrRrLsRsCrC == (NEXUS_AudioDraAcmod)BRAP_DSPCHN_DraAcmod_Lf_Rf_Lr_Rr_Ls_Rs_Cr_Cf);
            pStatus->codecStatus.dra.acmod = streamInfo.uStreamInfo.sDraInfo.eAcmod;
            pStatus->codecStatus.dra.lfe = streamInfo.uStreamInfo.sDraInfo.bInputLfe;
            pStatus->codecStatus.dra.stereoMode = streamInfo.uStreamInfo.sDraInfo.eStereoMode == BRAP_DSPCHN_DraStereoMode_eLoRo?NEXUS_AudioDraStereoMode_eLoRo:NEXUS_AudioDraStereoMode_eLtRt;
            break;
        case BRAP_DSPCHN_AudioType_eRealAudioLbr:
            pStatus->codecStatus.cook.frameSize = streamInfo.uStreamInfo.sRealAudioLbrInfo.uiFrameSize;
            pStatus->codecStatus.cook.stereo = (streamInfo.uStreamInfo.sRealAudioLbrInfo.eAcmod == BRAP_DSPCHN_RealAudioLbrAcmod_Lf_Rf)?true:false;
            break;
        default:
            /* No specifics for this codec */
            break;
        }
    }

    BRAP_GetCapabilities(g_NEXUS_audioModuleData.hRap, &capabilities);
    if ( capabilities.bDecodeSupported[handle->audioParams.sDspChParams.eType] )
    {
        rc = BRAP_GetDecoderVersionInfo(handle->rapChannel, &decoderVersionInfo);
        if ( BERR_SUCCESS == rc )
        {
            pStatus->algorithmProfileId = decoderVersionInfo.ui8AlgoProfileId;
            BDBG_CASSERT(sizeof(pStatus->algorithmVersionId) == sizeof(decoderVersionInfo.ui8AlgoVersionId));
            BKNI_Memcpy(pStatus->algorithmVersionId, decoderVersionInfo.ui8AlgoVersionId, sizeof(decoderVersionInfo.ui8AlgoVersionId));
            pStatus->firmwareVersionId = decoderVersionInfo.ui8FWVersionId;
        }
    }

    return NEXUS_SUCCESS;
}

/* SW7405-5118 bandrews - RAP has one less enum than AVC, so
invalid may map differently */
#define NEXUS_AUDIO_DECODER_P_RAP_PTS_INFO_TO_AVC_PTS_INFO(pAvcPts, pRapPts) \
do \
{ \
    switch ((pRapPts)->ePtsType) \
    { \
        case BRAP_DSPCHN_PtsType_eCoded: \
            (pAvcPts)->ePTSType = BAVC_PTSType_eCoded; \
            break; \
        case BRAP_DSPCHN_PtsType_eInterpolatedFromValidPTS: \
            (pAvcPts)->ePTSType = BAVC_PTSType_eInterpolatedFromValidPTS; \
            break; \
        case BRAP_DSPCHN_PtsType_eInterpolatedFromInvalidPTS: \
        default: \
            (pAvcPts)->ePTSType = BAVC_PTSType_eInterpolatedFromInvalidPTS; \
            break; \
    } \
} \
while (0)


static BERR_Code NEXUS_AudioDecoder_P_GetPtsCallback_isr(void *pContext, BAVC_PTSInfo *pPTSInfo)
{
    NEXUS_AudioDecoderHandle audioDecoder = (NEXUS_AudioDecoderHandle)pContext;
    BRAP_DSPCHN_PtsInfo ptsInfo;
    BERR_Code errCode=BERR_NOT_INITIALIZED;

    if(audioDecoder->running)
    errCode = BRAP_GetCurrentPTS_isr(audioDecoder->rapChannel, &ptsInfo);
    if ( BERR_SUCCESS == errCode )
    {
        BDBG_MSG(("GetPts - returned 0x%x, type %d", ptsInfo.ui32RunningPts, ptsInfo.ePtsType));

        /* map data structures */
        pPTSInfo->ui32CurrentPTS = ptsInfo.ui32RunningPts;
        NEXUS_AUDIO_DECODER_P_RAP_PTS_INFO_TO_AVC_PTS_INFO(pPTSInfo, &ptsInfo);
    }
    else
    {
        BDBG_MSG(("BRAP_GetCurrentPTS_isr not running or reported error %d", errCode));
    }

    return errCode;
}

static BERR_Code NEXUS_AudioDecoder_P_GetCdbLevelCallback_isr(void *pContext, unsigned *pCdbLevel)
{
    unsigned depth=0, size=0;
    NEXUS_AudioDecoderHandle audioDecoder = (NEXUS_AudioDecoderHandle)pContext;
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
    BDBG_MSG(("SetStcValid"));
    return BRAP_SetStcValidFlag_isr(audioDecoder->rapChannel);
}

static void NEXUS_AudioDecoder_P_FirstPts_isr(void *pParam1, int param2, void *pData)
{
    NEXUS_Error errCode;
    NEXUS_AudioDecoder *pDecoder = (NEXUS_AudioDecoder *)pParam1;
    BRAP_DSPCHN_PtsInfo *pPtsInfo = (BRAP_DSPCHN_PtsInfo *)pData;
    BAVC_PTSInfo pts;

    BSTD_UNUSED(param2);

    /* map data structures */
    pts.ui32CurrentPTS = pPtsInfo->ui32RunningPts;
    NEXUS_AUDIO_DECODER_P_RAP_PTS_INFO_TO_AVC_PTS_INFO(&pts, pPtsInfo);

    BDBG_MSG(("audio[%d] First PTS %08x", pDecoder->index, pPtsInfo->ui32RunningPts));
    if ( pDecoder->programSettings.stcChannel && pDecoder->stc.connector)
    {
        NEXUS_IsrCallback_Fire_isr(pDecoder->firstPtsCallback);
        errCode = NEXUS_StcChannel_RequestStc_isr(pDecoder->stc.connector, &pts);
        if ( errCode )
        {
            errCode=BERR_TRACE(errCode);
        }
    }

#if NEXUS_HAS_ASTM
    if (pDecoder->astm.settings.enableAstm)
    {
        pDecoder->astm.status.pts = pts.ui32CurrentPTS;

        if (pDecoder->astm.settings.firstPts_isr)
        {
            pDecoder->astm.settings.firstPts_isr(pDecoder->astm.settings.callbackContext, 0);
        }
    }
#endif /* NEXUS_HAS_ASTM */
}

static void NEXUS_AudioDecoder_P_AudioTsmFail_isr(void *pParam1, int param2, void *pData)
{
    NEXUS_AudioDecoder *pDecoder = (NEXUS_AudioDecoder *)pParam1;
    BRAP_DSPCHN_PtsInfo *pPtsInfo = (BRAP_DSPCHN_PtsInfo *)pData;
    NEXUS_Error errCode;
    BAVC_PTSInfo pts;

    uint32_t stc;

    BSTD_UNUSED(param2);

    if ( NULL == pPtsInfo )
    {
        /* Drop erroneous callbacks */
        return;
    }

    pts.ui32CurrentPTS = pPtsInfo->ui32RunningPts;
    NEXUS_AUDIO_DECODER_P_RAP_PTS_INFO_TO_AVC_PTS_INFO(&pts, pPtsInfo);

    BDBG_MSG(("audio[%d] Tsm fail: PTS 0x%08x, type %d",
        pDecoder->index, pts.ui32CurrentPTS, pts.ePTSType));

    NEXUS_StcChannel_GetStc_isr(pDecoder->programSettings.stcChannel, &stc);

    /* If we're in a non-standard STC trick mode */
    if ( pDecoder->trickState.stcTrickEnabled && (pDecoder->trickState.rate < 500 || pDecoder->trickState.rate > 2000) )
    {
        /* in STC trick mode, PTS might lag the STC because of decoder drop algorithm. don't reset STC in this case. */
        if ( stc > pts.ui32CurrentPTS && stc - pts.ui32CurrentPTS < 45000 * 8 )
        {
            return;
        }
    }

    BDBG_MSG(("pts2stcphase: %d", pPtsInfo->i32Pts2StcPhase));

#if NEXUS_HAS_ASTM
    if (pDecoder->astm.settings.enableAstm && pDecoder->astm.settings.syncLimit > 0)
    {
        pts.ui32CurrentPTS = (uint32_t)((int32_t)stc - pPtsInfo->i32Pts2StcPhase);
    }
#endif /* NEXUS_HAS_ASTM */

    /* PR:52308 ignore PTS errors for non-XPT inputs - we can't do anything about them from stcchannel/pcrlib anyway */
    if (!pDecoder->programSettings.input)
    {
        NEXUS_IsrCallback_Fire_isr(pDecoder->ptsErrorCallback);
        if (pDecoder->stc.connector)
        {
            errCode = NEXUS_StcChannel_PtsError_isr(pDecoder->stc.connector, &pts);
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
        pDecoder->astm.status.pts = pts.ui32CurrentPTS;

        if (pDecoder->astm.settings.tsmFail_isr)
        {
            pDecoder->astm.settings.tsmFail_isr(pDecoder->astm.settings.callbackContext, 0);
        }
    }
#endif /* NEXUS_HAS_ASTM */

    pDecoder->ptsErrorCount++;
}

#if NEXUS_HAS_ASTM
static void NEXUS_AudioDecoder_P_AudioTsmPass_isr(void *pParam1, int param2, void *pData)
{
    NEXUS_AudioDecoder *pDecoder = (NEXUS_AudioDecoder *)pParam1;
    BRAP_DSPCHN_PtsInfo ptsInfo;
    BAVC_PTSInfo pts;

    BSTD_UNUSED(param2);
    BSTD_UNUSED(pData);

    /* NOTE: pData will always be NULL for AudioTsmPass, so we must get PTS explicitly */

    BRAP_GetCurrentPTS_isr(pDecoder->rapChannel, &ptsInfo);

    pts.ui32CurrentPTS = ptsInfo.ui32RunningPts;
    NEXUS_AUDIO_DECODER_P_RAP_PTS_INFO_TO_AVC_PTS_INFO(&pts, &ptsInfo);

    BDBG_MSG(("audio[%d] TSM pass: PTS 0x%08x, type %d",
        pDecoder->index, pts.ui32CurrentPTS, pts.ePTSType));

    if (pDecoder->astm.settings.enableAstm)
    {
        pDecoder->astm.status.pts = pts.ui32CurrentPTS;

        if (pDecoder->astm.settings.tsmPass_isr)
        {
            pDecoder->astm.settings.tsmPass_isr(pDecoder->astm.settings.callbackContext, 0);
        }
    }
}
#endif

#if NEXUS_HAS_AUDIO_TSM_LOG_SUPPORT
static void NEXUS_AudioDecoder_P_AudioTsmLog_isr(void *pParam1, int param2, void *pData)
{
    BRAP_DSPCHN_TsmLogInfo  *sTsmLogInfo = (BRAP_DSPCHN_TsmLogInfo *)pData;
    FILE * tsmLogFp = (FILE *)pParam1;
    int i;
    unsigned int tsmLog[8], memAddr;

    BSTD_UNUSED (param2);
    BDBG_ERR(("Received TSM log interrupt: Buf Addr = 0x%08x, Buf Size = 0x%08x", sTsmLogInfo->uiTsmLogBufAdr, sTsmLogInfo->uiTsmLogBufSize));

    memAddr = sTsmLogInfo->uiTsmLogBufAdr;
    while (memAddr < sTsmLogInfo->uiTsmLogBufAdr + sTsmLogInfo->uiTsmLogBufSize)
    {
        fprintf(tsmLogFp,"\n");
        for (i = 0; i < 8; i++)
        {
            tsmLog[i]=*((volatile uint32_t *)memAddr); /*BRAP_MemRead32(hArc, memAddr);*/
            fprintf(tsmLogFp,"0x%08x\t", tsmLog[i]);
            memAddr += 4;
        }
    }
#if NEXUS_HAS_ASTM
    /* TODO: update ASTM TSM log status */
#endif /* NEXUS_HAS_ASTM */
}
#endif /* NEXUS_HAS_AUDIO_TSM_LOG_SUPPORT */

static void NEXUS_AudioDecoder_P_SampleRateChange_isr(void *pParam1, int param2, void *pData)
{
    NEXUS_AudioDecoder *pDecoder = (NEXUS_AudioDecoder *)pParam1;
    BRAP_DSPCHN_SampleRateChangeInfo *pSampleRateInfo = (BRAP_DSPCHN_SampleRateChangeInfo *)pData;

    BSTD_UNUSED(param2);

    if (pDecoder->index == 0) {
        BTRC_TRACE(ChnChange_DecodeFirstAudio, STOP);
        BTRC_TRACE(ChnChange_SyncUnmuteAudio, START);
    }

    pDecoder->sampleRateInfo = *pSampleRateInfo;

    BDBG_MSG(("audio[%d] Sampling Rate Info, eType=%d, eSamplingRate=%x", pDecoder->index,
        pDecoder->sampleRateInfo.eType, pDecoder->sampleRateInfo.eSamplingRate));

    BKNI_SetEvent(pDecoder->sampleRateEvent);
    BKNI_SetEvent(pDecoder->sourceChangedEvent);

#if NEXUS_HAS_SYNC_CHANNEL
    if (pDecoder->sync.settings.sampleRateCallback_isr) {
        (*pDecoder->sync.settings.sampleRateCallback_isr)(pDecoder->sync.settings.callbackContext, 0);
    }
#endif
}

static void NEXUS_AudioDecoder_P_StatusChange_isr(void *pParam1, int param2, void *pData)
{
    NEXUS_AudioDecoder *pDecoder = (NEXUS_AudioDecoder *)pParam1;
    BSTD_UNUSED(pData);
    switch (param2)
    {
    case BRAP_Interrupt_eDecLock:
        pDecoder->locked = true;
        BDBG_MSG(("decoder [%d]: lock", pDecoder->index));
        NEXUS_IsrCallback_Fire_isr(pDecoder->lockCallback);
        break;
    case BRAP_Interrupt_eDecUnlock:
        pDecoder->locked = false;
        BDBG_MSG(("decoder [%d]: unlock", pDecoder->index));
        NEXUS_IsrCallback_Fire_isr(pDecoder->lockCallback);
        break;
    case BRAP_Interrupt_eModeChange:
    case BRAP_Interrupt_eBitRateChange:
        BKNI_SetEvent(pDecoder->sourceChangedEvent);
        break;
    default:
        break;
    }
}

static void NEXUS_AudioDecoder_P_CdbItbError_isr(void *pParam1, int param2, void *pData)
{
    NEXUS_AudioDecoder *pDecoder = (NEXUS_AudioDecoder *)pParam1;
    NEXUS_IsrCallbackHandle isrCallback = (NEXUS_IsrCallbackHandle)param2;
    bool overflow=false;

    BDBG_OBJECT_ASSERT(pDecoder, NEXUS_AudioDecoder);
    BSTD_UNUSED(pData);
    BDBG_ASSERT(isrCallback == pDecoder->overflowCallback || isrCallback == pDecoder->underflowCallback);

    overflow = (isrCallback == pDecoder->overflowCallback)?true:false;
    if ( overflow )
    {
        pDecoder->numFifoOverflows++;
        BDBG_MSG(("CDB overflow - count %u", pDecoder->numFifoOverflows));
    }
    else
    {
        pDecoder->numFifoUnderflows++;
        BDBG_MSG(("CDB underflow - count %u", pDecoder->numFifoUnderflows));
    }

    NEXUS_IsrCallback_Fire_isr(isrCallback);
}

static void NEXUS_AudioDecoder_P_LayerChange_isr(void *pParam1, int param2, void *pData)
{
    NEXUS_AudioDecoder *pDecoder = (NEXUS_AudioDecoder *)pParam1;
    BSTD_UNUSED(param2);
    BSTD_UNUSED(pData);
    /* convert to task time */
    BKNI_SetEvent(pDecoder->layerChangeEvent);
    BKNI_SetEvent(pDecoder->sourceChangedEvent);
}

static void NEXUS_AudioDecoder_P_Watchdog(void *pContext)
{
    int i;

    BSTD_UNUSED(pContext);

    BDBG_WRN(("Audio Watchdog Handler Invoked"));

    g_numWatchdogs++;

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
    BRAP_ProcessWatchdogInterrupt(g_NEXUS_audioModuleData.hRap);

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
            BDBG_MSG(("Audio channel %p is notifying ASTM of its watchdog recovery", (void *)&g_decoders[i]));
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

static void NEXUS_AudioDecoder_P_LayerChange(void *pParam)
{
    NEXUS_Error errCode;
    NEXUS_AudioDecoder *pDecoder = (NEXUS_AudioDecoder *)pParam;

    BDBG_MSG(("Layer Change decoder[%d]", pDecoder->index));

    if ( pDecoder->running )
    {
        errCode = BRAP_DisableForFlush(pDecoder->rapChannel);
        if (errCode)
        {
            errCode=BERR_TRACE(errCode);
        }
        if ( pDecoder->programSettings.pidChannel )
        {
            LOCK_TRANSPORT();
            NEXUS_Rave_Disable_priv(pDecoder->raveContext);
            NEXUS_Rave_Flush_priv(pDecoder->raveContext);
            UNLOCK_TRANSPORT();
        }
        errCode = BRAP_FlushChannel(pDecoder->rapChannel);
        if (errCode)
        {
            errCode=BERR_TRACE(errCode);
        }
        if ( pDecoder->programSettings.pidChannel )
        {
            LOCK_TRANSPORT();
            NEXUS_Rave_Enable_priv(pDecoder->raveContext);
            UNLOCK_TRANSPORT();
        }
    }
    NEXUS_TaskCallback_Fire(pDecoder->sourceChangeAppCallback);
}

static void NEXUS_AudioDecoder_P_SampleRate(void *pParam)
{
    NEXUS_AudioDecoder *pDecoder = (NEXUS_AudioDecoder *)pParam;

    if(pDecoder->settings.wideGaThreshold && pDecoder->settings.gaThreshold == 0) {
        BRAP_DSPCHN_TsmDebugInfo info;
        uint32_t new_threshold;

        BRAP_GetTsmDebugInfo(pDecoder->rapChannel, &info);

        new_threshold = (info.sPtsInfo.i32TSMUpperThreshold*3)/2;

        BDBG_WRN(("Adjusting GA threshold to: %d (was %d)", new_threshold, info.sPtsInfo.i32TSMUpperThreshold));

        BRAP_SetTsmGAThreshold(pDecoder->rapChannel, new_threshold/45);
    }

    BDBG_MSG(("Sample Rate decoder[%d]", pDecoder->index));
}

static void NEXUS_AudioDecoder_P_SourceChange(void *pParam)
{
    NEXUS_AudioDecoder *pDecoder = (NEXUS_AudioDecoder *)pParam;
    BDBG_MSG(("Source Change decoder[%d]", pDecoder->index));
    BSTD_UNUSED(pDecoder);

    /* Force the timer to run once, it will reschedule itself if needed and fire the app callback when ready */
    NEXUS_AudioDecoder_P_StatusTimer(pParam);
}

/***************************************************************************
Summary:
    Get an audio connector for use in the audio mixer
See Also:

 ***************************************************************************/
NEXUS_AudioInput NEXUS_AudioDecoder_GetConnector(
    NEXUS_AudioDecoderHandle handle,
    NEXUS_AudioDecoderConnectorType type
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioDecoder);
    BDBG_ASSERT(handle->opened);

    switch ( type )
    {
    case NEXUS_AudioConnectorType_eStereo:
    case NEXUS_AudioConnectorType_eMultichannel:
    case NEXUS_AudioConnectorType_eCompressed:
        break;
    default:
        BDBG_MSG(("Unsupported connector type %u", type));
        return NULL;
    }

    return &handle->connectors[type];
}

bool NEXUS_AudioDecoder_P_IsRunning(NEXUS_AudioDecoderHandle handle)
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioDecoder);
    BDBG_ASSERT(handle->opened);
    /* This is slightly dangerous with trick modes, because a trick mute may cause this to return false.
       But, it's required for HDMI/SPDIF inputs where the decoder actually "starts stopped".  */
    return handle->running;
}

BRAP_ChannelHandle NEXUS_AudioDecoder_P_GetChannel(NEXUS_AudioDecoderHandle handle)
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioDecoder);
    BDBG_ASSERT(handle->opened);
    return handle->rapChannel;
}

#if NEXUS_HAS_DIGITAL_AUDIO_INPUTS
static void NEXUS_AudioDecoder_P_SpdifFormatChanged_isr(void *pParam1, int param2, void *pData)
{
    BSTD_UNUSED(pParam1);
    BSTD_UNUSED(param2);
    BSTD_UNUSED(pData);
    /* convert to task time */
    BKNI_SetEvent(g_spdifFormatEvent);
}

static void NEXUS_AudioDecoder_P_SpdifFormatChanged(void *pContext)
{
    NEXUS_Error errCode;
    NEXUS_AudioDecoderHandle handle = g_spdifDecoder;
    BRAP_SpdifRx_Status spdifStatus;
    bool stop=false, start=false;
    BRAP_DSPCHN_AudioType audioType=BRAP_DSPCHN_AudioType_eInvalid;

    BSTD_UNUSED(pContext);

    BDBG_ENTER(NEXUS_AudioDecoder_P_SpdifFormatChanged);

    if ( NULL == g_spdifDecoder )
    {
        /* Drop the event, nothing is actively using SPDIF */
        BDBG_MSG(("Dropped SPDIF format change - no consumer"));
        return;
    }

    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioDecoder);
    BDBG_MSG(("Processing digital input format change"));

    errCode = BRAP_GetSpdifRxStatus(g_NEXUS_audioModuleData.hRap, &spdifStatus);
    if ( errCode )
    {
        BDBG_WRN(("Invalid SPDIF RX Status on format change"));
        return;
    }

    /* Signal present is only asserted for SPDIF sources, not HDMI */
    if ( spdifStatus.bSignalPresent ||
         (handle->programSettings.input &&
          handle->programSettings.input->objectType == NEXUS_AudioInputType_eHdmi) )
    {
        BDBG_MSG(("SPDIF: Signal Present"));
        if ( spdifStatus.eStreamType == BRAP_SpdifRx_StreamType_eCompressed )
        {
            BDBG_MSG(("Compressed Input - codec is %d", spdifStatus.sPc.eAudioType));
            audioType = spdifStatus.sPc.eAudioType;
        }
        else
        {
            audioType = BRAP_DSPCHN_AudioType_ePcm;
            BDBG_MSG(("Non-compressed input.  Assuming PCM. (streamType=%d validLeft %d validRight %d)", spdifStatus.eStreamType, spdifStatus.bValidLeft, spdifStatus.bValidRight));
        }

        if ( handle->running )
        {
            if ( handle->audioParams.sDspChParams.eType != audioType )
            {
                if ( audioType == BRAP_DSPCHN_AudioType_eInvalid )
                {
                    stop = true;
                    BDBG_MSG(("Stopping decoder for invalid codec"));
                }
                else
                {
                    stop = start = true;
                    BDBG_MSG(("Restarting decoder to change format"));
                }
            }
        }
        else
        {
            if ( audioType == BRAP_DSPCHN_AudioType_eInvalid )
            {
                BDBG_MSG(("Leaving decoder stopped for invalid codec"));
            }
            else
            {
                BDBG_MSG(("Starting stopped decoder"));
                start = true;
            }
        }
    }
    else
    {
        BDBG_MSG(("SPDIF: No Signal"));
        if ( handle->running )
        {
            stop = true;
        }
    }

    if ( stop )
    {
        BDBG_MSG(("Stopping Decoder"));
        NEXUS_AudioDecoder_P_Stop(handle, true);
    }
    if ( start )
    {
        BDBG_MSG(("Starting Decoder"));
        if ( NEXUS_AudioDecoder_P_SetupCodec(handle, audioType, BAVC_StreamType_ePes) )
        {
            (void)BERR_TRACE(BERR_NOT_SUPPORTED);
            goto done;
        }
        NEXUS_AudioDecoder_P_Start(handle);
    }

done:
    BDBG_LEAVE(NEXUS_AudioDecoder_P_SpdifFormatChanged);
    NEXUS_TaskCallback_Fire(handle->sourceChangeAppCallback);
}
#endif

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
    #if NEXUS_HAS_DIGITAL_AUDIO_INPUTS
    NEXUS_Error errCode;
    BRAP_SpdifRx_ChannelStatus channelStatus;

    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioDecoder);
    BDBG_ASSERT(NULL != pStatus);

    if ( handle->started && handle->programSettings.input )
    {
        errCode = BRAP_GetSpdifRxChannelStatus(g_NEXUS_audioModuleData.hRap, &channelStatus);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
        pStatus->leftChannelLow = channelStatus.aui32LtChnStatus[0];
        pStatus->leftChannelHigh = channelStatus.aui32LtChnStatus[1];
        pStatus->rightChannelLow = channelStatus.aui32RtChnStatus[0];
        pStatus->rightChannelHigh = channelStatus.aui32RtChnStatus[1];
        return BERR_SUCCESS;
    }
    else
    {
        BDBG_WRN(("Can only retrieve channel status if a digital input is connected and the decoder is active"));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
    #else
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pStatus);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
    #endif
}

void NEXUS_AudioDecoder_GetSyncSettings_priv( NEXUS_AudioDecoderHandle audioDecoder, NEXUS_AudioInputSyncSettings *pSyncSettings )
{
    NEXUS_ASSERT_MODULE();
    *pSyncSettings = audioDecoder->sync.settings;
}

static NEXUS_Error NEXUS_AudioDecoder_P_SetPtsOffset(NEXUS_AudioDecoderHandle audioDecoder)
{
    BERR_Code errCode = BERR_SUCCESS;
    int32_t ptsOffset;

    /* NOTE: RAP does not need conversion to 27 MHz domain.  All offsets are 45 KHz, even for DSS */

    ptsOffset = audioDecoder->settings.ptsOffset;

#if NEXUS_HAS_SYNC_CHANNEL
    ptsOffset += (int32_t)audioDecoder->sync.settings.delay;
#endif

#if NEXUS_HAS_ASTM
    if (audioDecoder->astm.settings.enableAstm)
    {
        ptsOffset += audioDecoder->astm.settings.ptsOffset;
    }
#endif

    if (!audioDecoder->programSettings.input && audioDecoder->rapChannel && audioDecoder->running)
    {
        errCode = BRAP_SetPTSOffset(audioDecoder->rapChannel, ptsOffset);
        if (errCode)
        {
            return BERR_TRACE(errCode);
        }
    }

    return 0;
}

NEXUS_Error NEXUS_AudioDecoder_SetSyncSettings_priv( NEXUS_AudioDecoderHandle audioDecoder, const NEXUS_AudioInputSyncSettings *pSyncSettings )
{
    NEXUS_ASSERT_MODULE();
    audioDecoder->sync.settings = *pSyncSettings;

    NEXUS_AudioDecoder_P_SetPtsOffset(audioDecoder);

    NEXUS_AudioDecoder_P_SetSyncMute(audioDecoder, audioDecoder->sync.settings.mute);

    return 0;
}

NEXUS_Error NEXUS_AudioDecoder_GetSyncStatus_isr( NEXUS_AudioDecoderHandle audioDecoder, NEXUS_AudioInputSyncStatus *pSyncStatus )
{
    pSyncStatus->started = audioDecoder->running;
    pSyncStatus->digital = true;
    pSyncStatus->dsolaEnabled = audioDecoder->audioParams.sDspChParams.uiPBRate != BRAP_DSPCHN_PLAYBACKRATE_NORMAL;
    pSyncStatus->nonRealTime = false;
    BRAP_GetAudioDecodeDelay(audioDecoder->rapChannel, &pSyncStatus->delay);
    /* sync channel has a define for compensating for the 8 ms Sarnoff lipsync stream offsets,
    here we only compensate FMM delay */
    /* for 64 sample FMM delay -> this gets us within 0 - 1 ms, since the FMM
    delay really depends on Fs. 32K -> 2 ms, 44.1K -> 1.45 ms, 48K -> 1.33 ms */
    pSyncStatus->delay += 1;
    return 0;
}

#if NEXUS_HAS_ASTM
void NEXUS_AudioDecoder_GetAstmSettings_priv(NEXUS_AudioDecoderHandle audioDecoder, NEXUS_AudioDecoderAstmSettings * pAstmSettings )
{
    NEXUS_ASSERT_MODULE();

    *pAstmSettings = audioDecoder->astm.settings;
}

NEXUS_Error NEXUS_AudioDecoder_SetAstmSettings_priv( NEXUS_AudioDecoderHandle audioDecoder, const NEXUS_AudioDecoderAstmSettings * pAstmSettings )
{
    NEXUS_Error rc = NEXUS_SUCCESS;

    NEXUS_ASSERT_MODULE();

    /* copy settings as-is, this way ASTM will always get what it set */
    audioDecoder->astm.settings = *pAstmSettings;

    /* if ASTM is internally permitted, apply settings */
    rc = NEXUS_AudioDecoder_ApplySettings_priv(audioDecoder);

    return rc;
}

NEXUS_Error NEXUS_AudioDecoder_GetAstmStatus_isr( NEXUS_AudioDecoderHandle audioDecoder, NEXUS_AudioDecoderAstmStatus * pAstmStatus )
{
    BRAP_DSPCHN_PtsInfo ptsInfo;
    BERR_Code rc;

    rc = BRAP_GetCurrentPTS_isr(audioDecoder->rapChannel, &ptsInfo);
    audioDecoder->astm.status.ptsStcDiff = ptsInfo.i32Pts2StcPhase;

    *pAstmStatus = audioDecoder->astm.status;

    return 0;
}
#endif /* NEXUS_HAS_ASTM */

static BRAP_DSPCHN_Ac3StereoMode NEXUS_AudioDecoder_P_DownmixMode2Magnum(NEXUS_AudioDecoderDolbyStereoDownmixMode downmixMode)
{
    switch ( downmixMode )
    {
    default:
        (void)BERR_TRACE(BERR_INVALID_PARAMETER);
        /* Fall through */
    case NEXUS_AudioDecoderDolbyStereoDownmixMode_eAutomatic:
        return BRAP_DSPCHN_Ac3StereoMode_eAuto;
    case NEXUS_AudioDecoderDolbyStereoDownmixMode_eDolbySurroundCompatible:
        return BRAP_DSPCHN_Ac3StereoMode_eLtRt;
    case NEXUS_AudioDecoderDolbyStereoDownmixMode_eStandard:
        return BRAP_DSPCHN_Ac3StereoMode_eLoRo;
    }
}

static NEXUS_AudioDecoderDolbyStereoDownmixMode NEXUS_AudioDecoder_P_DownmixMode2Nexus(BRAP_DSPCHN_Ac3StereoMode downmixMode)
{
    switch ( downmixMode )
    {
    default:
        (void)BERR_TRACE(BERR_INVALID_PARAMETER);
        /* Fall through */
    case BRAP_DSPCHN_Ac3StereoMode_eAuto:
        return NEXUS_AudioDecoderDolbyStereoDownmixMode_eAutomatic;
    case BRAP_DSPCHN_Ac3StereoMode_eLtRt:
        return NEXUS_AudioDecoderDolbyStereoDownmixMode_eDolbySurroundCompatible;
    case BRAP_DSPCHN_Ac3StereoMode_eLoRo:
        return NEXUS_AudioDecoderDolbyStereoDownmixMode_eStandard;
    }
}

static BRAP_DSPCHN_Ac3CompMode NEXUS_AudioDecoder_P_DrcMode2Magnum(NEXUS_AudioDecoderDolbyDrcMode drcMode)
{
    switch ( drcMode )
    {
    case NEXUS_AudioDecoderDolbyDrcMode_eCustomA:
        return BRAP_DSPCHN_Ac3CompMode_eCustomA;
    case NEXUS_AudioDecoderDolbyDrcMode_eCustomD:
        return BRAP_DSPCHN_Ac3CompMode_eCustomD;
    case NEXUS_AudioDecoderDolbyDrcMode_eLine:
        return BRAP_DSPCHN_Ac3CompMode_eLine;
    case NEXUS_AudioDecoderDolbyDrcMode_eRf:
        return BRAP_DSPCHN_Ac3CompMode_eRf;
    default:
        (void)BERR_TRACE(BERR_INVALID_PARAMETER);
        /* Fall through */
    case NEXUS_AudioDecoderDolbyDrcMode_eOff:
        return BRAP_DSPCHN_AC3CompMode_eOff;
    }
}

static NEXUS_AudioDecoderDolbyDrcMode NEXUS_AudioDecoder_P_DrcMode2Nexus(BRAP_DSPCHN_Ac3CompMode drcMode)
{
    switch ( drcMode )
    {
    case  BRAP_DSPCHN_Ac3CompMode_eCustomA:
        return NEXUS_AudioDecoderDolbyDrcMode_eCustomA;
    case BRAP_DSPCHN_Ac3CompMode_eCustomD:
        return NEXUS_AudioDecoderDolbyDrcMode_eCustomD;
    case BRAP_DSPCHN_Ac3CompMode_eLine:
        return NEXUS_AudioDecoderDolbyDrcMode_eLine;
    case BRAP_DSPCHN_Ac3CompMode_eRf:
        return NEXUS_AudioDecoderDolbyDrcMode_eRf;
    default:
        (void)BERR_TRACE(BERR_INVALID_PARAMETER);
        /* Fall through */
    case BRAP_DSPCHN_AC3CompMode_eOff:
        return NEXUS_AudioDecoderDolbyDrcMode_eOff;
    }
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
    NEXUS_Error errCode;
    BRAP_ChannelConfigParams *pRapSettings;
    BRAP_DSPCHN_AudioType audioType;

    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioDecoder);
    BDBG_ASSERT(codec < NEXUS_AudioCodec_eMax);
    BDBG_ASSERT(NULL != pSettings);

    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->codec = codec;

    audioType = NEXUS_Audio_P_CodecToAudioType(codec);
    if ( audioType == BRAP_DSPCHN_AudioType_eInvalid )
    {
        return;
    }

    pRapSettings = BKNI_Malloc(sizeof(*pRapSettings));
    if ( NULL == pRapSettings )
    {
        errCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        return;
    }

    errCode = BRAP_GetCurrentChannelConfig(handle->rapChannel,
                                           audioType,
                                           pRapSettings);
    if ( errCode )
    {
        (void)BERR_TRACE(errCode);
        BKNI_Free(pRapSettings);
        return;
    }

    switch ( codec )
    {
    case NEXUS_AudioCodec_eAc3:
        pSettings->codecSettings.ac3.stereoDownmixMode = NEXUS_AudioDecoder_P_DownmixMode2Nexus(pRapSettings->uChConfigParams.sDecConfigParams.uConfigParams.sAc3ConfigParams.eStereoMode);
        pSettings->codecSettings.ac3.drcMode = NEXUS_AudioDecoder_P_DrcMode2Nexus(pRapSettings->uChConfigParams.sDecConfigParams.uConfigParams.sAc3ConfigParams.eCompMode);
        pSettings->codecSettings.ac3.cut = pRapSettings->uChConfigParams.sDecConfigParams.uConfigParams.sAc3ConfigParams.ui16DynRngScaleHi;
        pSettings->codecSettings.ac3.boost = pRapSettings->uChConfigParams.sDecConfigParams.uConfigParams.sAc3ConfigParams.ui16DynRngScaleLo;
        pSettings->codecSettings.ac3.dialogNormalization = pRapSettings->uChConfigParams.sDecConfigParams.uConfigParams.sAc3ConfigParams.bStreamDialogNormEnable;
        pSettings->codecSettings.ac3.dialogNormalizationValue = pRapSettings->uChConfigParams.sDecConfigParams.uConfigParams.sAc3ConfigParams.ui16UserDialogNormVal;
#if BRAP_VER < 4
        pSettings->codecSettings.ac3.cutDownmix = pRapSettings->uChConfigParams.sDecConfigParams.uConfigParams.sAc3ConfigParams.ui16DynRngScaleHi_DownmixPath;
        pSettings->codecSettings.ac3.boostDownmix = pRapSettings->uChConfigParams.sDecConfigParams.uConfigParams.sAc3ConfigParams.ui16DynRngScaleLo_DownmixPath;
        pSettings->codecSettings.ac3.scaleDownmix = pRapSettings->uChConfigParams.sDecConfigParams.uConfigParams.sAc3ConfigParams.ui16PcmScaleFactor_DownmixPath;
        pSettings->codecSettings.ac3.drcModeDownmix = NEXUS_AudioDecoder_P_DrcMode2Nexus(pRapSettings->uChConfigParams.sDecConfigParams.uConfigParams.sAc3ConfigParams.eCompMode_DownmixPath);
#endif
        pSettings->codecSettings.ac3.scale = pRapSettings->uChConfigParams.sDecConfigParams.uConfigParams.sAc3ConfigParams.ui16PcmScaleFactor;
        pSettings->codecSettings.ac3.substreamId = pRapSettings->uChConfigParams.sDecConfigParams.uConfigParams.sAc3ConfigParams.uiSubstreamIDToDecode;
        break;
    case NEXUS_AudioCodec_eAc3Plus:
        pSettings->codecSettings.ac3Plus.stereoDownmixMode = NEXUS_AudioDecoder_P_DownmixMode2Nexus(pRapSettings->uChConfigParams.sDecConfigParams.uConfigParams.sAc3PlusConfigParams.eStereoMode);
        pSettings->codecSettings.ac3Plus.drcMode = NEXUS_AudioDecoder_P_DrcMode2Nexus(pRapSettings->uChConfigParams.sDecConfigParams.uConfigParams.sAc3PlusConfigParams.eCompMode);
        pSettings->codecSettings.ac3Plus.cut = pRapSettings->uChConfigParams.sDecConfigParams.uConfigParams.sAc3PlusConfigParams.ui16DynRngScaleHi;
        pSettings->codecSettings.ac3Plus.boost = pRapSettings->uChConfigParams.sDecConfigParams.uConfigParams.sAc3PlusConfigParams.ui16DynRngScaleLo;
        pSettings->codecSettings.ac3Plus.dialogNormalization = pRapSettings->uChConfigParams.sDecConfigParams.uConfigParams.sAc3PlusConfigParams.bStreamDialogNormEnable;
        pSettings->codecSettings.ac3Plus.dialogNormalizationValue = pRapSettings->uChConfigParams.sDecConfigParams.uConfigParams.sAc3PlusConfigParams.ui16UserDialogNormVal;
#if BRAP_VER < 4
        pSettings->codecSettings.ac3Plus.cutDownmix = pRapSettings->uChConfigParams.sDecConfigParams.uConfigParams.sAc3PlusConfigParams.ui16DynRngScaleHi_DownmixPath;
        pSettings->codecSettings.ac3Plus.boostDownmix = pRapSettings->uChConfigParams.sDecConfigParams.uConfigParams.sAc3PlusConfigParams.ui16DynRngScaleLo_DownmixPath;
        pSettings->codecSettings.ac3Plus.scaleDownmix = pRapSettings->uChConfigParams.sDecConfigParams.uConfigParams.sAc3PlusConfigParams.ui16PcmScaleFactor_DownmixPath;
        pSettings->codecSettings.ac3Plus.drcModeDownmix = NEXUS_AudioDecoder_P_DrcMode2Nexus(pRapSettings->uChConfigParams.sDecConfigParams.uConfigParams.sAc3PlusConfigParams.eCompMode_DownmixPath);
#endif
        pSettings->codecSettings.ac3Plus.scale = pRapSettings->uChConfigParams.sDecConfigParams.uConfigParams.sAc3PlusConfigParams.ui16PcmScaleFactor;
        pSettings->codecSettings.ac3Plus.substreamId = pRapSettings->uChConfigParams.sDecConfigParams.uConfigParams.sAc3PlusConfigParams.uiSubstreamIDToDecode;
        break;
    case NEXUS_AudioCodec_eAacAdts:
    case NEXUS_AudioCodec_eAacLoas:
        pSettings->codecSettings.aac.cut = pRapSettings->uChConfigParams.sDecConfigParams.uConfigParams.sAacConfigParams.ui16DrcGainControlCompress;
        pSettings->codecSettings.aac.boost = pRapSettings->uChConfigParams.sDecConfigParams.uConfigParams.sAacConfigParams.ui16DrcGainControlBoost;
        pSettings->codecSettings.aac.drcTargetLevel = pRapSettings->uChConfigParams.sDecConfigParams.uConfigParams.sAacConfigParams.ui16DrcTargetLevel;
        pSettings->codecSettings.aac.downmixMode = pRapSettings->uChConfigParams.sDecConfigParams.uConfigParams.sAacConfigParams.eDownmixType;
        pSettings->codecSettings.aac.decode96Khz = pRapSettings->uChConfigParams.sDecConfigParams.uConfigParams.sAacConfigParams.bEnable96KhzDecode;
        break;
    case NEXUS_AudioCodec_eAacPlusAdts:
    case NEXUS_AudioCodec_eAacPlusLoas:
        pSettings->codecSettings.aacPlus.cut = pRapSettings->uChConfigParams.sDecConfigParams.uConfigParams.sAacSbrConfigParams.ui16DrcGainControlCompress;
        pSettings->codecSettings.aacPlus.boost = pRapSettings->uChConfigParams.sDecConfigParams.uConfigParams.sAacSbrConfigParams.ui16DrcGainControlBoost;
        pSettings->codecSettings.aacPlus.drcTargetLevel = pRapSettings->uChConfigParams.sDecConfigParams.uConfigParams.sAacSbrConfigParams.ui16DrcTargetLevel;
        pSettings->codecSettings.aacPlus.downmixMode = pRapSettings->uChConfigParams.sDecConfigParams.uConfigParams.sAacSbrConfigParams.eDownmixType;
        pSettings->codecSettings.aacPlus.decode96Khz= pRapSettings->uChConfigParams.sDecConfigParams.uConfigParams.sAacSbrConfigParams.bEnable96KhzDecode;
        BDBG_CASSERT(NEXUS_AudioDecoderDolbyPulseDrcMode_eLine == (NEXUS_AudioDecoderDolbyPulseDrcMode)BRAP_DSPCHN_DolbyPulse_CompMode_LineMode);
        BDBG_CASSERT(NEXUS_AudioDecoderDolbyPulseDrcMode_eRf == (NEXUS_AudioDecoderDolbyPulseDrcMode)BRAP_DSPCHN_DolbyPulse_CompMode_RfMode);
        pSettings->codecSettings.aacPlus.drcMode = pRapSettings->uChConfigParams.sDecConfigParams.uConfigParams.sAacSbrConfigParams.eCompMode;
        pSettings->codecSettings.aacPlus.mpegConformanceMode = pRapSettings->uChConfigParams.sDecConfigParams.uConfigParams.sAacSbrConfigParams.bMpegConformanceMode;
        break;
    case NEXUS_AudioCodec_eWmaPro:
        pSettings->codecSettings.wmaPro.dynamicRangeControlValid = pRapSettings->uChConfigParams.sDecConfigParams.uConfigParams.sWmaProConfigParams.bDrcFlag;
        pSettings->codecSettings.wmaPro.dynamicRangeControl.peakReference = pRapSettings->uChConfigParams.sDecConfigParams.uConfigParams.sWmaProConfigParams.uiPeakAmplitudeRef;
        pSettings->codecSettings.wmaPro.dynamicRangeControl.peakTarget = pRapSettings->uChConfigParams.sDecConfigParams.uConfigParams.sWmaProConfigParams.uiDesiredPeak;
        pSettings->codecSettings.wmaPro.dynamicRangeControl.averageReference = pRapSettings->uChConfigParams.sDecConfigParams.uConfigParams.sWmaProConfigParams.uiRmsAmplitudeRef;
        pSettings->codecSettings.wmaPro.dynamicRangeControl.averageTarget = pRapSettings->uChConfigParams.sDecConfigParams.uConfigParams.sWmaProConfigParams.uiDesiredRms;
        break;
    case NEXUS_AudioCodec_eDtsLegacy: /* For DTS streams with legacy frame-sync.  These streams are something called as 14bits stream */
        pSettings->codecSettings.dts.mixLfeToPrimary = pRapSettings->uChConfigParams.sDecConfigParams.uConfigParams.sDtsBroadcastConfigParams.bMixLFE2Primary;
        pSettings->codecSettings.dts.stereoDownmixMode = pRapSettings->uChConfigParams.sDecConfigParams.uConfigParams.sDtsBroadcastConfigParams.eStereoMode;
        pSettings->codecSettings.dts.enableDrc = pRapSettings->uChConfigParams.sDecConfigParams.uConfigParams.sDtsBroadcastConfigParams.bEnableDRC;
        pSettings->codecSettings.dts.boost = pRapSettings->uChConfigParams.sDecConfigParams.uConfigParams.sDtsBroadcastConfigParams.ui16DynRngScaleHi;
        pSettings->codecSettings.dts.cut = pRapSettings->uChConfigParams.sDecConfigParams.uConfigParams.sDtsBroadcastConfigParams.ui16DynRngScaleLo;
        break;
    case NEXUS_AudioCodec_eDts:
            /* fall through */
    case NEXUS_AudioCodec_eDtsHd:
        pSettings->codecSettings.dts.mixLfeToPrimary = pRapSettings->uChConfigParams.sDecConfigParams.uConfigParams.sDtsHdConfigParams.bMixLFE2Primary;
        pSettings->codecSettings.dts.stereoDownmixMode = pRapSettings->uChConfigParams.sDecConfigParams.uConfigParams.sDtsHdConfigParams.eStereoMode;
        pSettings->codecSettings.dts.enableDrc = pRapSettings->uChConfigParams.sDecConfigParams.uConfigParams.sDtsHdConfigParams.bEnableDRC;
        pSettings->codecSettings.dts.boost = pRapSettings->uChConfigParams.sDecConfigParams.uConfigParams.sDtsHdConfigParams.ui16DynRngScaleHi;
        pSettings->codecSettings.dts.cut = pRapSettings->uChConfigParams.sDecConfigParams.uConfigParams.sDtsHdConfigParams.ui16DynRngScaleLo;
        break;
#if BRAP_VER >= 4 && BCHP_CHIP == 35230
    case NEXUS_AudioCodec_eAdpcm:
        pSettings->codecSettings.adpcm.enableGain = pRapSettings->uChConfigParams.sDecConfigParams.uConfigParams.sAdpcmConfigParams.bApplyGain;
        pSettings->codecSettings.adpcm.gainFactor = pRapSettings->uChConfigParams.sDecConfigParams.uConfigParams.sAdpcmConfigParams.ui32GainFactor;
        break;
#endif
    default:
        break;
    }

    BKNI_Free(pRapSettings);
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
    NEXUS_Error errCode;
    BRAP_ChannelConfigParams *pRapSettings;
    BRAP_DSPCHN_AudioType audioType;

    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioDecoder);
    BDBG_ASSERT(NULL != pSettings);

    audioType = NEXUS_Audio_P_CodecToAudioType(pSettings->codec);
    if ( audioType == BRAP_DSPCHN_AudioType_eInvalid )
    {
        return BERR_SUCCESS;
    }

    pRapSettings = BKNI_Malloc(sizeof(*pRapSettings));
    if ( NULL == pRapSettings )
    {
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }

    errCode = BRAP_GetCurrentChannelConfig(handle->rapChannel,
                                           audioType,
                                           pRapSettings);
    if ( errCode )
    {
        BKNI_Free(pRapSettings);
        return BERR_TRACE(errCode);
    }

    switch ( pSettings->codec )
    {
    case NEXUS_AudioCodec_eAc3:
        pRapSettings->uChConfigParams.sDecConfigParams.uConfigParams.sAc3ConfigParams.eStereoMode = NEXUS_AudioDecoder_P_DownmixMode2Magnum(pSettings->codecSettings.ac3.stereoDownmixMode);
        pRapSettings->uChConfigParams.sDecConfigParams.uConfigParams.sAc3ConfigParams.eCompMode = NEXUS_AudioDecoder_P_DrcMode2Magnum(pSettings->codecSettings.ac3.drcMode);
        pRapSettings->uChConfigParams.sDecConfigParams.uConfigParams.sAc3ConfigParams.ui16DynRngScaleHi = pSettings->codecSettings.ac3.cut;
        pRapSettings->uChConfigParams.sDecConfigParams.uConfigParams.sAc3ConfigParams.ui16DynRngScaleLo = pSettings->codecSettings.ac3.boost;
        pRapSettings->uChConfigParams.sDecConfigParams.uConfigParams.sAc3ConfigParams.bStreamDialogNormEnable = pSettings->codecSettings.ac3.dialogNormalization;
        pRapSettings->uChConfigParams.sDecConfigParams.uConfigParams.sAc3ConfigParams.ui16UserDialogNormVal = pSettings->codecSettings.ac3.dialogNormalizationValue;
#if BRAP_VER < 4
        pRapSettings->uChConfigParams.sDecConfigParams.uConfigParams.sAc3ConfigParams.ui16DynRngScaleHi_DownmixPath = pSettings->codecSettings.ac3.cutDownmix;
        pRapSettings->uChConfigParams.sDecConfigParams.uConfigParams.sAc3ConfigParams.ui16DynRngScaleLo_DownmixPath = pSettings->codecSettings.ac3.boostDownmix;
        pRapSettings->uChConfigParams.sDecConfigParams.uConfigParams.sAc3ConfigParams.ui16PcmScaleFactor_DownmixPath = pSettings->codecSettings.ac3.scaleDownmix;
        pRapSettings->uChConfigParams.sDecConfigParams.uConfigParams.sAc3ConfigParams.eCompMode_DownmixPath = NEXUS_AudioDecoder_P_DrcMode2Magnum(pSettings->codecSettings.ac3.drcModeDownmix);
#endif
        pRapSettings->uChConfigParams.sDecConfigParams.uConfigParams.sAc3ConfigParams.ui16PcmScaleFactor = pSettings->codecSettings.ac3.scale;
        pRapSettings->uChConfigParams.sDecConfigParams.uConfigParams.sAc3ConfigParams.uiSubstreamIDToDecode = pSettings->codecSettings.ac3.substreamId;
        break;
    case NEXUS_AudioCodec_eAc3Plus:
        pRapSettings->uChConfigParams.sDecConfigParams.uConfigParams.sAc3PlusConfigParams.eStereoMode = NEXUS_AudioDecoder_P_DownmixMode2Magnum(pSettings->codecSettings.ac3Plus.stereoDownmixMode);
        pRapSettings->uChConfigParams.sDecConfigParams.uConfigParams.sAc3PlusConfigParams.eCompMode = NEXUS_AudioDecoder_P_DrcMode2Magnum(pSettings->codecSettings.ac3Plus.drcMode);
        pRapSettings->uChConfigParams.sDecConfigParams.uConfigParams.sAc3PlusConfigParams.ui16DynRngScaleHi = pSettings->codecSettings.ac3Plus.cut;
        pRapSettings->uChConfigParams.sDecConfigParams.uConfigParams.sAc3PlusConfigParams.ui16DynRngScaleLo = pSettings->codecSettings.ac3Plus.boost;
        pRapSettings->uChConfigParams.sDecConfigParams.uConfigParams.sAc3PlusConfigParams.bStreamDialogNormEnable = pSettings->codecSettings.ac3Plus.dialogNormalization;
        pRapSettings->uChConfigParams.sDecConfigParams.uConfigParams.sAc3PlusConfigParams.ui16UserDialogNormVal = pSettings->codecSettings.ac3Plus.dialogNormalizationValue;
#if BRAP_VER < 4
        pRapSettings->uChConfigParams.sDecConfigParams.uConfigParams.sAc3PlusConfigParams.ui16DynRngScaleHi_DownmixPath = pSettings->codecSettings.ac3Plus.cutDownmix;
        pRapSettings->uChConfigParams.sDecConfigParams.uConfigParams.sAc3PlusConfigParams.ui16DynRngScaleLo_DownmixPath = pSettings->codecSettings.ac3Plus.boostDownmix;
        pRapSettings->uChConfigParams.sDecConfigParams.uConfigParams.sAc3PlusConfigParams.ui16PcmScaleFactor_DownmixPath = pSettings->codecSettings.ac3Plus.scaleDownmix;
        pRapSettings->uChConfigParams.sDecConfigParams.uConfigParams.sAc3PlusConfigParams.eCompMode_DownmixPath = NEXUS_AudioDecoder_P_DrcMode2Magnum(pSettings->codecSettings.ac3Plus.drcModeDownmix);
#endif
        pRapSettings->uChConfigParams.sDecConfigParams.uConfigParams.sAc3PlusConfigParams.ui16PcmScaleFactor = pSettings->codecSettings.ac3Plus.scale;
        pRapSettings->uChConfigParams.sDecConfigParams.uConfigParams.sAc3PlusConfigParams.uiSubstreamIDToDecode = pSettings->codecSettings.ac3Plus.substreamId;
        break;
    case NEXUS_AudioCodec_eAacAdts:
    case NEXUS_AudioCodec_eAacLoas:
        pRapSettings->uChConfigParams.sDecConfigParams.uConfigParams.sAacConfigParams.ui16DrcGainControlCompress = pSettings->codecSettings.aac.cut;
        pRapSettings->uChConfigParams.sDecConfigParams.uConfigParams.sAacConfigParams.ui16DrcGainControlBoost = pSettings->codecSettings.aac.boost;
        pRapSettings->uChConfigParams.sDecConfigParams.uConfigParams.sAacConfigParams.ui16DrcTargetLevel = pSettings->codecSettings.aac.drcTargetLevel;
        pRapSettings->uChConfigParams.sDecConfigParams.uConfigParams.sAacConfigParams.eDownmixType = pSettings->codecSettings.aac.downmixMode;
        pRapSettings->uChConfigParams.sDecConfigParams.uConfigParams.sAacConfigParams.bEnable96KhzDecode = pSettings->codecSettings.aac.decode96Khz;
        BDBG_CASSERT(NEXUS_AudioDecoderAacDownmixMode_eMatrix == (NEXUS_AudioDecoderAacDownmixMode)BRAP_DSPCHN_AAC_DownmixType_eMatrix);
        BDBG_CASSERT(NEXUS_AudioDecoderAacDownmixMode_eArib == (NEXUS_AudioDecoderAacDownmixMode)BRAP_DSPCHN_AAC_DownmixType_eArib);
        break;
    case NEXUS_AudioCodec_eAacPlusAdts:
    case NEXUS_AudioCodec_eAacPlusLoas:
        pRapSettings->uChConfigParams.sDecConfigParams.uConfigParams.sAacSbrConfigParams.ui16DrcGainControlCompress = pSettings->codecSettings.aacPlus.cut;
        pRapSettings->uChConfigParams.sDecConfigParams.uConfigParams.sAacSbrConfigParams.ui16DrcGainControlBoost = pSettings->codecSettings.aacPlus.boost;
        pRapSettings->uChConfigParams.sDecConfigParams.uConfigParams.sAacSbrConfigParams.ui16DrcTargetLevel = pSettings->codecSettings.aacPlus.drcTargetLevel;
        pRapSettings->uChConfigParams.sDecConfigParams.uConfigParams.sAacSbrConfigParams.eDownmixType = pSettings->codecSettings.aacPlus.downmixMode;
        pRapSettings->uChConfigParams.sDecConfigParams.uConfigParams.sAacSbrConfigParams.bEnable96KhzDecode = pSettings->codecSettings.aacPlus.decode96Khz;
        pRapSettings->uChConfigParams.sDecConfigParams.uConfigParams.sAacSbrConfigParams.eCompMode = pSettings->codecSettings.aacPlus.drcMode;
        pRapSettings->uChConfigParams.sDecConfigParams.uConfigParams.sAacSbrConfigParams.bMpegConformanceMode = pSettings->codecSettings.aacPlus.mpegConformanceMode;
        BDBG_CASSERT(NEXUS_AudioDecoderAacDownmixMode_eMatrix == (NEXUS_AudioDecoderAacDownmixMode)BRAP_DSPCHN_AACSbr_DownmixType_eMatrix);
        BDBG_CASSERT(NEXUS_AudioDecoderAacDownmixMode_eArib == (NEXUS_AudioDecoderAacDownmixMode)BRAP_DSPCHN_AACSbr_DownmixType_eArib);
        break;
    case NEXUS_AudioCodec_eDtsLegacy: /* For DTS streams with legacy frame-sync.  These streams are something called as 14bits stream */
        pRapSettings->uChConfigParams.sDecConfigParams.uConfigParams.sDtsBroadcastConfigParams.bMixLFE2Primary = pSettings->codecSettings.dts.mixLfeToPrimary;
        BDBG_CASSERT(NEXUS_AudioDecoderDtsDownmixMode_eAuto == (NEXUS_AudioDecoderDtsStereoDownmixMode)BRAP_DSPCHN_DtsStereoMode_eAuto);
        BDBG_CASSERT(NEXUS_AudioDecoderDtsDownmixMode_eLtRt == (NEXUS_AudioDecoderDtsStereoDownmixMode)BRAP_DSPCHN_DtsStereoMode_eLtRt);
        BDBG_CASSERT(NEXUS_AudioDecoderDtsDownmixMode_eLoRo == (NEXUS_AudioDecoderDtsStereoDownmixMode)BRAP_DSPCHN_DtsStereoMode_eLoRo);
        pRapSettings->uChConfigParams.sDecConfigParams.uConfigParams.sDtsBroadcastConfigParams.eStereoMode = pSettings->codecSettings.dts.stereoDownmixMode;
        pRapSettings->uChConfigParams.sDecConfigParams.uConfigParams.sDtsBroadcastConfigParams.bEnableDRC = pSettings->codecSettings.dts.enableDrc;
        pRapSettings->uChConfigParams.sDecConfigParams.uConfigParams.sDtsBroadcastConfigParams.ui16DynRngScaleHi = pSettings->codecSettings.dts.boost;
        pRapSettings->uChConfigParams.sDecConfigParams.uConfigParams.sDtsBroadcastConfigParams.ui16DynRngScaleLo = pSettings->codecSettings.dts.cut;
        break;
    case NEXUS_AudioCodec_eDts:
            /* fall through */
    case NEXUS_AudioCodec_eDtsHd:
        pRapSettings->uChConfigParams.sDecConfigParams.uConfigParams.sDtsHdConfigParams.bMixLFE2Primary = pSettings->codecSettings.dts.mixLfeToPrimary;
        pRapSettings->uChConfigParams.sDecConfigParams.uConfigParams.sDtsHdConfigParams.eStereoMode = pSettings->codecSettings.dts.stereoDownmixMode;
        pRapSettings->uChConfigParams.sDecConfigParams.uConfigParams.sDtsHdConfigParams.bEnableDRC= pSettings->codecSettings.dts.enableDrc;
        pRapSettings->uChConfigParams.sDecConfigParams.uConfigParams.sDtsHdConfigParams.ui16DynRngScaleHi = pSettings->codecSettings.dts.boost;
        pRapSettings->uChConfigParams.sDecConfigParams.uConfigParams.sDtsHdConfigParams.ui16DynRngScaleLo = pSettings->codecSettings.dts.cut;
        break;
    case NEXUS_AudioCodec_eWmaPro:
        pRapSettings->uChConfigParams.sDecConfigParams.uConfigParams.sWmaProConfigParams.bDrcFlag = pSettings->codecSettings.wmaPro.dynamicRangeControlValid;
        pRapSettings->uChConfigParams.sDecConfigParams.uConfigParams.sWmaProConfigParams.uiPeakAmplitudeRef = pSettings->codecSettings.wmaPro.dynamicRangeControl.peakReference;
        pRapSettings->uChConfigParams.sDecConfigParams.uConfigParams.sWmaProConfigParams.uiDesiredPeak = pSettings->codecSettings.wmaPro.dynamicRangeControl.peakTarget;
        pRapSettings->uChConfigParams.sDecConfigParams.uConfigParams.sWmaProConfigParams.uiRmsAmplitudeRef = pSettings->codecSettings.wmaPro.dynamicRangeControl.averageReference;
        pRapSettings->uChConfigParams.sDecConfigParams.uConfigParams.sWmaProConfigParams.uiDesiredRms = pSettings->codecSettings.wmaPro.dynamicRangeControl.averageTarget;
        break;
#if BRAP_VER >= 4 && BCHP_CHIP == 35230
    case NEXUS_AudioCodec_eAdpcm:
        pRapSettings->uChConfigParams.sDecConfigParams.uConfigParams.sAdpcmConfigParams.bApplyGain = pSettings->codecSettings.adpcm.enableGain;
        pRapSettings->uChConfigParams.sDecConfigParams.uConfigParams.sAdpcmConfigParams.ui32GainFactor = pSettings->codecSettings.adpcm.gainFactor;
        break;
#endif
    default:
        BDBG_MSG(("Codec settings not available for codec %d", pSettings->codec));
        BKNI_Free(pRapSettings);
        return BERR_SUCCESS;
    }

    errCode = BRAP_SetChannelConfig(handle->rapChannel, pRapSettings);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
    }
    BKNI_Free(pRapSettings);
    return errCode;
}

static NEXUS_Error NEXUS_AudioDecoder_P_ConnectionChange(void *pParam, NEXUS_AudioInput input)
{
    NEXUS_AudioDecoder *pDecoder = pParam;

    BDBG_OBJECT_ASSERT(pDecoder, NEXUS_AudioDecoder);

    BDBG_MSG(("Audio decoder %p connection change", pParam));
    if ( pDecoder->association )
    {
        int i;
        BDBG_MSG(("Removing all destinations on connection change"));
        for ( i = 0; i < NEXUS_AudioDecoderConnectorType_eMax; i++ )
        {
            /* SW7125-1429 - Problem when digital outputs have different encoders.
                If we leave the desination here we run into issues later with HDMI connector still being associated.
                That then runs into problems because the destination is never removed resulting in segfault. */
            BSTD_UNUSED(input);
            /*if ( &pDecoder->connectors[i] != input )*/
            {
                NEXUS_AudioInput_P_RemoveDestinations(&pDecoder->connectors[i]);
            }
        }
        BDBG_MSG(("Destroying association on connection change"));
        NEXUS_AudioAssociation_P_Destroy(pDecoder->association);
        pDecoder->association = NULL;
    }
    return NEXUS_SUCCESS;
}


static NEXUS_Error NEXUS_AudioDecoder_P_SetCompressedSyncMute(NEXUS_AudioDecoderHandle decoder, bool muted)
{
    NEXUS_AudioOutputHandle output;
    NEXUS_Error rc;
    int i;

    for ( i = 0; i < NEXUS_AUDIO_MAX_OUTPUTS; i++ )
    {
        output = decoder->outputLists[NEXUS_AudioDecoderConnectorType_eCompressed].outputs[i];
        if ( output )
        {
            BDBG_MSG(("setting output mute to %d", muted));
            rc=NEXUS_AudioOutput_P_SetSyncMute(output, muted);
            if ( rc )
            {
                return BERR_TRACE(rc);
            }
        }
    }

    return NEXUS_SUCCESS;
}

NEXUS_Error NEXUS_AudioDecoder_P_SetSyncMute(NEXUS_AudioDecoderHandle decoder, bool muted)
{
    NEXUS_Error rc;
    bool wasMuted = false;

    /* if sync called this while decoder was not running, it was to set persistent startup behavior */
    if (!decoder->running)
    {
        BDBG_MSG(("SetSyncMute(%u): start muted = %d", decoder->index, muted));
        decoder->sync.startMuted = muted;
    }

    wasMuted = decoder->sync.mute;
    decoder->sync.mute = muted;

    /* if we are running, and unmuting */
    if (decoder->running && wasMuted && !muted)
    {
        /* apply the unmute edge */

        BDBG_MSG(("SetSyncMute(%u): unmute", decoder->index));

        rc = NEXUS_AudioDecoder_P_SetCompressedSyncMute(decoder, decoder->sync.mute);
        if ( rc ) { return BERR_TRACE(rc); }

        rc = NEXUS_AudioDecoder_ApplySettings_priv(decoder);
        if (rc) { return BERR_TRACE(rc); }

#if B_HAS_TRC
        if (decoder->index == 0) {
            BTRC_TRACE(ChnChange_SyncUnmuteAudio, STOP);
            if (BTRC_MODULE_HANDLE(ChnChange_DecodeStopAudio)->stats[0].count) {
                BTRC_TRACE(ChnChange_Total_Audio, STOP);
                BKNI_SetEvent(decoder->channelChangeReportEvent);
            }
        }
#endif
    }

    return NEXUS_SUCCESS;
}

NEXUS_Error NEXUS_AudioDecoder_P_SetTrickMute(NEXUS_AudioDecoderHandle decoder, bool muted)
{
    NEXUS_AudioOutputHandle output;
    NEXUS_Error rc;
    int i;

    BDBG_MSG(("SetTrickMute(%d)", muted));

    if ( decoder->trickMute != muted )
    {
        /* Passthrough or simul */
        if ( decoder->audioParams.sDspChParams.eDecodeMode != BRAP_DSPCHN_DecodeMode_eDecode )
        {
            for ( i = 0; i < NEXUS_AUDIO_MAX_OUTPUTS; i++ )
            {
                output = decoder->outputLists[NEXUS_AudioDecoderConnectorType_eCompressed].outputs[i];
                if ( output )
                {
                    BDBG_MSG(("setting output mute to %d", muted));
                    rc=NEXUS_AudioOutput_P_SetTrickMute(output, muted);
                    if ( rc )
                    {
                        return BERR_TRACE(rc);
                    }
                }
            }
        }

        decoder->trickMute = muted;

        /* Decode or simul */
        if ( decoder->audioParams.sDspChParams.eDecodeMode != BRAP_DSPCHN_DecodeMode_ePassThru )
        {
            /* This will set the decoder mute into the mixer taking into account user volume settings */
            NEXUS_AudioDecoder_ApplySettings_priv(decoder);
        }
    }

    return NEXUS_SUCCESS;
}

NEXUS_Error NEXUS_AudioDecoder_P_SetCompressedMute(NEXUS_AudioDecoderHandle decoder, bool muted)
{
    NEXUS_AudioOutputHandle output;
    NEXUS_Error rc;
    int i;

    BDBG_MSG(("SetCompressedMute(%d)", muted));

    /* Passthrough or simul */
    if ( decoder->audioParams.sDspChParams.eDecodeMode != BRAP_DSPCHN_DecodeMode_eDecode )
    {
        for ( i = 0; i < NEXUS_AUDIO_MAX_OUTPUTS; i++ )
        {
            output = decoder->outputLists[NEXUS_AudioDecoderConnectorType_eCompressed].outputs[i];
            if ( output )
            {
                BDBG_MSG(("setting output mute to %d", muted));
                rc=NEXUS_AudioOutput_P_SetCompressedMute(output, muted);
                if ( rc )
                {
                    return BERR_TRACE(rc);
                }
            }
        }
    }
    return NEXUS_SUCCESS;
}

static void NEXUS_AudioDecoder_P_ChannelChangeReport(void * context)
{
#if B_HAS_TRC
#define REPORT_FREQUENCY 10

    const unsigned chn = 0;
    unsigned i = 0;
#if (defined B_PERF_BMIPS3300) || (defined B_PERF_BMIPS4380)
    const unsigned COUNTER = 3; /* Cycles */
    const unsigned CPU_CLOCK_RATE = 405; /* MHz */
#elif (defined B_PERF_LINUX)
    const unsigned COUNTER = 0; /* Time, us */
    const unsigned CPU_CLOCK_RATE = 1;
#else
    BSTD_UNUSED(context);
    BERR_TRACE(NEXUS_NOT_SUPPORTED);
    return;
#endif

    if (BTRC_MODULE_HANDLE(ChnChange_Total_Audio)->stats[0].count % REPORT_FREQUENCY == 0) {
        if (NEXUS_GetEnv("force_vsync")) {BKNI_Printf("Channel change measurements with force_vsync=y\n");}
        if (NEXUS_GetEnv("sync_disabled")) {BKNI_Printf("Channel change measurements with sync_disabled=y\n");}
        BKNI_Printf("%-32s %6s %12s %12s %12s\n", "Name", "Cnt", "Avg(us)", "Max(us)", "Min(us)");

        for (i=0; i < AUDIO_BTRC_N_MODULES; i++) {
            const struct BTRC_P_Stats *stats = &audio_btrc_modules[i]->stats[chn];

            if (stats->count) {
                const unsigned avg = (unsigned)(((((uint64_t)stats->total_hi.perf.data[COUNTER])<<((sizeof(unsigned))*8)) + stats->total.perf.data[COUNTER])/stats->count / CPU_CLOCK_RATE);
                const unsigned max = stats->max.perf.data[COUNTER] / CPU_CLOCK_RATE;
                const unsigned min = stats->min.perf.data[COUNTER] / CPU_CLOCK_RATE;
                BKNI_Printf("%-32s %6u %12u %12u %12u\n", audio_btrc_modules[i]->name, stats->count, avg, max, min);
            }
        }
    }
#endif
    BSTD_UNUSED(context);
    return;
}

static bool NEXUS_AudioDecoder_P_CodecIsMono(NEXUS_AudioCodec codec)
{
    if ( codec == NEXUS_AudioCodec_eAmr )
    {
        return true;
    }

    return false;
}

static bool NEXUS_AudioDecoder_P_CodecSupportsMultichannel(NEXUS_AudioCodec codec)
{
    if ( NEXUS_GetEnv("multichannel_audio_disabled") || NEXUS_GetEnv("audio_processing_disabled") )
    {
        return false;
    }

    switch ( codec )
    {
    case NEXUS_AudioCodec_eMpeg:
    case NEXUS_AudioCodec_eMp3:
    case NEXUS_AudioCodec_eWmaStd:
    case NEXUS_AudioCodec_eLpcmDvd:
    case NEXUS_AudioCodec_eLpcmHdDvd:
    case NEXUS_AudioCodec_eLpcmBluRay:
    case NEXUS_AudioCodec_ePcm:
        return false;
    default:
        return true;
    }
}

static void NEXUS_AudioDecoder_P_SetOutputMode(NEXUS_AudioDecoderHandle handle)
{
    BRAP_ChannelConfigParams *pConfig;
    bool multichannel = false;
    BERR_Code errCode;

    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioDecoder);

    /* Remove all post-processing */
    pConfig = BKNI_Malloc(sizeof(*pConfig)); /* must malloc this large structure to avoid possible stack overflow */

    (void)BRAP_GetCurrentChannelConfig(handle->rapChannel,
                                       handle->audioParams.sDspChParams.eType,
                                       pConfig);

    /* Check for at least one multichannel output */
    if ( handle->outputLists[NEXUS_AudioDecoderConnectorType_eMultichannel].outputs[0] != NULL )
    {
        /* Make sure the codec supports multichannel */
        if ( NEXUS_AudioDecoder_P_CodecSupportsMultichannel(handle->programSettings.codec) )
        {
#ifdef RAP_MS10_SUPPORT
            /* MS10 AC3 encoder receives data directly from the decoder and is not technically on the multichannel path.
               enabling multichannel output will cause errors in that case. */
            NEXUS_AudioInput encoderConnector;
            bool transcodeEnabled=false;

            encoderConnector = NEXUS_AudioInput_P_FindDirect(&handle->connectors[NEXUS_AudioDecoderConnectorType_eMultichannel],
                                                             NEXUS_AudioInputType_eAc3Encode);
            if ( NULL != encoderConnector )
            {
                transcodeEnabled=true;
            }
            else
            {
                encoderConnector = NEXUS_AudioInput_P_FindDirect(&handle->connectors[NEXUS_AudioDecoderConnectorType_eMultichannel],
                                                                 NEXUS_AudioInputType_eEncoder);
                if ( NULL != encoderConnector )
                {
                    NEXUS_AudioEncoderSettings encoderSettings;
                    NEXUS_AudioEncoder_GetSettings(encoderConnector->pObjectHandle, &encoderSettings);
                    if ( encoderSettings.codec == NEXUS_AudioCodec_eAc3 )
                    {
                        transcodeEnabled=true;
                    }
                }
            }

            /* If we've found the dolby transcoder, make sure it's the only thing attached */
            if ( transcodeEnabled == true )
            {
                BDBG_MSG(("Dolby transcoder detected."));
                if ( NEXUS_AudioInput_P_GetNumDirectConnections(&handle->connectors[NEXUS_AudioDecoderConnectorType_eMultichannel]) > 1 )
                {
                    /* Something else is connected to multichannel that is not the dolby transcoder */
                    BDBG_MSG(("Connection other than dolby transcoder detected.  Enabling multichannel output."));
                    multichannel = true;
                }
                else
                {
                    NEXUS_AudioOutputList directOutputList;
                    NEXUS_AudioInput_P_GetOutputs(&handle->connectors[NEXUS_AudioDecoderConnectorType_eMultichannel], &directOutputList, true);
                    if ( directOutputList.outputs[0] != NULL )
                    {
                        BDBG_MSG(("Direct 5.1 outputs detected along with transcoder.  Enabling multichannel."));
                        multichannel = true;
                    }
                    else
                    {
                        BDBG_MSG(("Only multichannel object connected is dolby transcoder.  Not permitting multichannel output."));
                    }
                }
            }
            else
            {
                BDBG_MSG(("Dolby transcoder not detected.  Permitting multichannel output."));
                /* Something is connected to multichannel that is not the dolby transcoder */
                multichannel = true;
            }
#else
            /* We can configure the decoder in multichannel mode */
            multichannel = true;
#endif
        }
    }

    if ( handle->settings.outputMode == NEXUS_AudioDecoderOutputMode_eAuto )
    {
#if BRAP_VER >= 4
        handle->enableStereoDownmixPath = false;
#else
        handle->audioParams.sDspChParams.bEnableStereoDownmixPath = false;  /* Not required except for multichannel */
#endif
        if ( multichannel )
        {
            if ( NULL != handle->outputLists[NEXUS_AudioDecoderConnectorType_eStereo].outputs[0]  )
            {
#if BRAP_VER >= 4
                handle->enableStereoDownmixPath = true;
#else
                handle->audioParams.sDspChParams.bEnableStereoDownmixPath = true;  /* Not required except for multichannel */
#endif
            }

            switch ( handle->connectors[NEXUS_AudioDecoderConnectorType_eMultichannel].format )
            {
            case NEXUS_AudioInputFormat_ePcm5_1:
                pConfig->uChConfigParams.sDecConfigParams.eOutputMode = BRAP_OutputMode_e3_2;    /* 5.1 */
                break;
            case NEXUS_AudioInputFormat_ePcm7_1:
                pConfig->uChConfigParams.sDecConfigParams.eOutputMode = BRAP_OutputMode_e3_4;    /* 7.1 */
                break;
            default:
                if ( NEXUS_AudioDecoder_P_CodecIsMono(handle->programSettings.codec) )
                {
                    pConfig->uChConfigParams.sDecConfigParams.eOutputMode = BRAP_OutputMode_e1_0;    /* 1.0 */
                }
                else
                {
                    pConfig->uChConfigParams.sDecConfigParams.eOutputMode = BRAP_OutputMode_e2_0;    /* 2.0 */
                }
                break;
            }
        }
        else
        {
            pConfig->uChConfigParams.sDecConfigParams.eOutputMode = BRAP_OutputMode_e2_0;        /* default */
        }
    }
    else
    {
        switch ( handle->settings.outputMode )
        {
        default:
            (void)BERR_TRACE(BERR_INVALID_PARAMETER);
            /* Fall through */
        case NEXUS_AudioDecoderOutputMode_eAuto:
        case NEXUS_AudioDecoderOutputMode_e2_0:
            pConfig->uChConfigParams.sDecConfigParams.eOutputMode = BRAP_OutputMode_e2_0;
            break;
        case NEXUS_AudioDecoderOutputMode_e1_0:
            pConfig->uChConfigParams.sDecConfigParams.eOutputMode = BRAP_OutputMode_e1_0;
            break;
        case NEXUS_AudioDecoderOutputMode_e1_1:
            pConfig->uChConfigParams.sDecConfigParams.eOutputMode = BRAP_OutputMode_e1_1;
            break;
        case NEXUS_AudioDecoderOutputMode_e3_0:
            pConfig->uChConfigParams.sDecConfigParams.eOutputMode = BRAP_OutputMode_e3_0;
            break;
        case NEXUS_AudioDecoderOutputMode_e3_1:
            pConfig->uChConfigParams.sDecConfigParams.eOutputMode = BRAP_OutputMode_e3_1;
            break;
        case NEXUS_AudioDecoderOutputMode_e2_2:
            pConfig->uChConfigParams.sDecConfigParams.eOutputMode = BRAP_OutputMode_e2_2;
            break;
        case NEXUS_AudioDecoderOutputMode_e3_2:
            pConfig->uChConfigParams.sDecConfigParams.eOutputMode = BRAP_OutputMode_e3_2;
            break;
        case NEXUS_AudioDecoderOutputMode_e3_4:
            pConfig->uChConfigParams.sDecConfigParams.eOutputMode = BRAP_OutputMode_e3_4;
            break;
        }

#if BRAP_VER >= 4
        handle->enableStereoDownmixPath = false;
#else
        handle->audioParams.sDspChParams.bEnableStereoDownmixPath = false; /* Not required except for multichannel */
#endif

        if ( (pConfig->uChConfigParams.sDecConfigParams.eOutputMode > BRAP_OutputMode_e2_0 ||
             handle->settings.outputLfeMode == NEXUS_AudioDecoderOutputLfeMode_eOn) &&
             NULL != handle->outputLists[NEXUS_AudioDecoderConnectorType_eStereo].outputs[0] )
        {
            if ( NEXUS_AudioDecoder_P_CodecSupportsMultichannel(handle->programSettings.codec) )
            {
#if BRAP_VER >= 4
                handle->enableStereoDownmixPath = true;
#else
                handle->audioParams.sDspChParams.bEnableStereoDownmixPath = true;  /* Not required except for multichannel */
#endif
            }
            else
            {
                BDBG_WRN(("Forcing output mode to 2.0 for non-multichannel codec"));
                pConfig->uChConfigParams.sDecConfigParams.eOutputMode = BRAP_OutputMode_e2_0;
            }
        }
    }

    if ( handle->settings.outputLfeMode == NEXUS_AudioDecoderOutputLfeMode_eAuto )
    {
        if ( multichannel || pConfig->uChConfigParams.sDecConfigParams.eOutputMode > BRAP_OutputMode_e2_0 )
        {
            if ( NEXUS_AudioDecoder_P_CodecSupportsMultichannel(handle->programSettings.codec) )
            {
                pConfig->uChConfigParams.sDecConfigParams.bOutputLfeOn = true;
            }
            else
            {
                pConfig->uChConfigParams.sDecConfigParams.bOutputLfeOn = false;
            }
        }
        else
        {
            pConfig->uChConfigParams.sDecConfigParams.bOutputLfeOn = false;
        }
    }
    else
    {
        pConfig->uChConfigParams.sDecConfigParams.bOutputLfeOn = false;

        if ( handle->settings.outputLfeMode == NEXUS_AudioDecoderOutputLfeMode_eOn )
        {
            if ( NEXUS_AudioDecoder_P_CodecSupportsMultichannel(handle->programSettings.codec)  )
            {
                pConfig->uChConfigParams.sDecConfigParams.bOutputLfeOn = true;
            }
            else
            {
                BDBG_WRN(("Disabling LFE output for non-multichannel codec"));
            }
        }
    }

#if BRAP_VER < 4
    /* To make the PI happy, the input mode must be >= the output mode.  Do that here. */
    handle->audioParams.eInputAudMode = pConfig->uChConfigParams.sDecConfigParams.eOutputMode; /* depends on the stream */
    handle->audioParams.bInputLfePresent = pConfig->uChConfigParams.sDecConfigParams.bOutputLfeOn;
#endif

    BDBG_MSG(("Audio decoder output mode %d LFE %d", pConfig->uChConfigParams.sDecConfigParams.eOutputMode, pConfig->uChConfigParams.sDecConfigParams.bOutputLfeOn));

    errCode = BRAP_SetChannelConfig(handle->rapChannel, pConfig);
    if( errCode )
    {
        errCode = BERR_TRACE(errCode);
    }

    BKNI_Free(pConfig);
}

static void NEXUS_AudioDecoder_P_SetupMixingCoefs(NEXUS_AudioDecoderHandle handle, BRAP_MixingCoef *pCoefs)
{
    int x,y;

    if ( handle->descriptorParent == NULL )
    {
        if ( handle->settings.muted || handle->trickMute || handle->sync.mute)
        {
            BDBG_MSG(("Setting volume to mute"));
            /* Mute decoder for PCM outputs */
            for ( x = 0; x < BRAP_OutputChannel_eMax; x++ )
            {
                for ( y = 0; y < BRAP_OutputChannel_eMax; y++ )
                {
                    pCoefs->ui32Coef[x][y] = 0;
                }
            }
            NEXUS_AudioDecoder_P_SetCompressedMute(handle, true);
        }
        else
        {
            BDBG_MSG(("Setting volume to desired levels"));
            /* Apply volume settings */
            for ( x = 0; x < BRAP_OutputChannel_eMax; x++ )
            {
                for ( y = 0; y < BRAP_OutputChannel_eMax; y++ )
                {
                    pCoefs->ui32Coef[x][y] = handle->settings.volumeMatrix[x][y];
                }
            }
            NEXUS_AudioDecoder_P_SetCompressedMute(handle, false);
        }
    }
    else
    {
        for ( x = 0; x < BRAP_OutputChannel_eMax; x++ )
        {
            for ( y = 0; y < BRAP_OutputChannel_eMax; y++ )
            {
                BDBG_MSG(("Setting volume to normal for AD output"));
                if ( x == y )
                {
                    pCoefs->ui32Coef[x][y] = NEXUS_AUDIO_VOLUME_LINEAR_NORMAL;
                }
                else
                {
                    pCoefs->ui32Coef[x][y] = NEXUS_AUDIO_VOLUME_LINEAR_MIN;
                }
            }
        }
    }
}

void NEXUS_AudioDecoder_P_SetTsm(NEXUS_AudioDecoderHandle handle)
{
    bool tsm=false;

    if ( handle->tsmPermitted )
    {
        if ( handle->trickState.tsmEnabled )
        {
#if NEXUS_HAS_ASTM
            if ( handle->astm.settings.enableAstm )
            {
                BDBG_MSG(("ASTM is %s TSM for audio channel %p", handle->astm.settings.enableTsm ? "enabling" : "disabling", (void *)handle));
                if ( handle->astm.settings.enableTsm )
                {
                    tsm = true;
                }
            }
            else
#endif
            {
                tsm = true;
            }
        }
    }

    if ( tsm )
    {
        NEXUS_PidChannelStatus pidChannelStatus;
        BERR_Code errCode;
        unsigned discardThreshold;
        bool setGaThreshold = false;
        unsigned gaThreshold;

        BKNI_Memset(&pidChannelStatus, 0, sizeof(pidChannelStatus));
        if ( handle->programSettings.pidChannel )
        {
            (void)NEXUS_PidChannel_GetStatus(handle->programSettings.pidChannel, &pidChannelStatus);
        }

        /* Compute TSM details */
        if ( 0 == handle->settings.discardThreshold )
        {
            discardThreshold = (handle->audioParams.bPlayback)?30000:3000;   /* ms */
        }
        else
        {
            discardThreshold = handle->settings.discardThreshold;
        }

        errCode = BRAP_SetTSMDiscardThreshold(handle->rapChannel, discardThreshold);
        if (errCode!=BERR_SUCCESS) { errCode=BERR_TRACE(errCode); }

        if ( 0 == handle->settings.gaThreshold )
        {
            if (!handle->settings.wideGaThreshold)
            {
#if NEXUS_HAS_SIMPLE_DECODER
                if (handle->simple.settings.gaThreshold)
                {
                    /* this is for sync adjustment concealment
                     * it only takes effect if the user has not specified an explicit
                     * gaThreshold or wideGaThreshold */
                    gaThreshold = handle->simple.settings.gaThreshold;
                    setGaThreshold = true;
                }
                else
#endif
                {
                    gaThreshold = (pidChannelStatus.originalTransportType == NEXUS_TransportType_eAvi) ? 0x30 : 0x8;
                    setGaThreshold = true;
                }
            }
        }
        else
        {
            gaThreshold = handle->settings.gaThreshold;
            setGaThreshold = true;
        }

        if (setGaThreshold)
        {
            errCode = BRAP_SetTsmGAThreshold(handle->rapChannel, gaThreshold);
            if (errCode!=BERR_SUCCESS) { errCode=BERR_TRACE(errCode); }
        }
    }

    BDBG_MSG(("%s TSM",tsm?"Enabling":"Disabling"));
    BRAP_EnableTSM(handle->rapChannel, tsm);
    handle->tsmEnabled = tsm;

#if NEXUS_HAS_ASTM
    /* Only allow ASTM if we have TSM enabled for all non-ASTM controls and we have a TS source */
    if ( handle->tsmPermitted && handle->trickState.tsmEnabled &&
         handle->audioParams.sDspChParams.eStreamType == BAVC_StreamType_eTsMpeg )
    {
        BDBG_MSG(("%s ASTM mode for audio channel %p",handle->astm.settings.enableAstm?"Enabling":"Disabling", (void *)handle));
        BRAP_EnableASTM(handle->rapChannel, handle->astm.settings.enableAstm);
    }
    else
    {
        BDBG_MSG(("Disabling ASTM mode for audio channel %p", (void *)handle));
        BRAP_EnableASTM(handle->rapChannel, false);
    }
#endif
}

static bool NEXUS_AudioDecoder_P_FindDts(
    NEXUS_AudioDecoderHandle handle
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioDecoder);

    if ( NEXUS_AudioInput_P_FindByType(&handle->connectors[NEXUS_AudioDecoderConnectorType_eStereo],
                                       NEXUS_AudioInputType_eDtsEncode) )
    {
        return true;
    }
    else if ( NEXUS_AudioInput_P_FindByType(&handle->connectors[NEXUS_AudioDecoderConnectorType_eMultichannel],
                                       NEXUS_AudioInputType_eDtsEncode) )
    {
        return true;
    }

    return false;
}

static bool NEXUS_AudioDecoder_P_FindBtsc(
    NEXUS_AudioDecoderHandle handle
    )
{
    NEXUS_AudioInput input;
    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioDecoder);

    input = NEXUS_AudioInput_P_FindByType(&handle->connectors[NEXUS_AudioDecoderConnectorType_eStereo],
                                       NEXUS_AudioInputType_eRfEncoder);
    if ( input )
    {
        NEXUS_RfAudioEncoderSettings settings;
        NEXUS_RfAudioEncoder_GetSettings((NEXUS_RfAudioEncoderHandle)input->pObjectHandle,&settings);
        if (NEXUS_RfAudioEncoding_eBtsc == settings.encoding)
        {
            return true;
        }
    }
    return false;
}

static void NEXUS_AudioDecoder_P_StatusTimer(void *pParam)
{
    NEXUS_AudioDecoderHandle handle=pParam;

    handle->statusCheckTimer = NULL;

    if ( handle->running )
    {
        BRAP_DSPCHN_StreamInfo streamInfo;
        BERR_Code rc;
#if 1
        if ( handle->audioParams.sDspChParams.eType == BRAP_DSPCHN_AudioType_ePcm || handle->audioParams.sDspChParams.eDecodeMode == BRAP_DSPCHN_DecodeMode_ePassThru )
        {
            rc = 0;
        }
        else
#endif
        {
            BDBG_MSG(("Polling for stream information to be ready"));
            rc = BRAP_GetStreamInformation(handle->rapChannel, &streamInfo);
        }
        if ( 0 == rc )
        {
            /* Stream info ready.  Notify the application */
            BDBG_MSG(("Stream info ready, notifying application"));
#if NEXUS_HAS_ASTM
            /* must update this when we can get it */
            handle->astm.status.decodedCount = streamInfo.sFrameDecAndErrInfo.uiTotalFramesDecoded;
#endif
            NEXUS_TaskCallback_Fire(handle->sourceChangeAppCallback);
            NEXUS_TaskCallback_Fire(handle->streamInfoCallback);
        }
        else
        {
            /* BDBG_MSG(("Stream info not ready.")); */
            handle->statusCheckTimer = NEXUS_ScheduleTimer(25, NEXUS_AudioDecoder_P_StatusTimer, pParam);
        }
    }
}


#if NEXUS_NUM_AUDIO_MIXERS
static bool NEXUS_AudioDecoder_P_IsMixedByDspMixer(NEXUS_AudioDecoderHandle handle)
{
    NEXUS_AudioInput mixerConnector;

    mixerConnector = NEXUS_AudioInput_P_FindByType(&handle->connectors[NEXUS_AudioDecoderConnectorType_eMultichannel], NEXUS_AudioInputType_eMixer);
    if ( NULL == mixerConnector )
    {
        mixerConnector = NEXUS_AudioInput_P_FindByType(&handle->connectors[NEXUS_AudioDecoderConnectorType_eStereo], NEXUS_AudioInputType_eMixer);
    }
    if ( mixerConnector && NEXUS_AudioMixer_P_IsUsingDsp(mixerConnector->pObjectHandle) )
    {
        /* We found this decoder has a valid input to a mixer and it's using the DSP */
        return true;
    }
    else
    {
        return false;
    }
}
#endif


bool NEXUS_AudioDecoder_P_IsMultiChannelOutputEnabled(NEXUS_AudioDecoderHandle handle)
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioDecoder);

    if ( NEXUS_AudioInputFormat_ePcm5_1 == handle->connectors[NEXUS_AudioDecoderConnectorType_eMultichannel].format )
    {
        return true;
    }
    else
    {
        return false;
    }
}

NEXUS_Error NEXUS_AudioModule_Standby_priv(
    bool enabled,
    const NEXUS_StandbySettings *pSettings
    )
{
    BERR_Code rc = NEXUS_SUCCESS;
#if NEXUS_POWER_MANAGEMENT && BRAP_VER < 4
    unsigned i;

    BSTD_UNUSED(pSettings);
    for(i=0 ; i<NEXUS_NUM_AUDIO_DECODERS; i++) {
        NEXUS_AudioDecoderHandle handle = &g_decoders[i];

        if (!handle) {
            continue;
        }

        if (!handle->opened) {
            continue;
        }

        if ( handle->started ) {
            BDBG_WRN(("Audio decoder %d needs to be stopped before entering standby", i));
            return NEXUS_NOT_SUPPORTED;
        }
    }

    if(enabled) {
        rc = BRAP_Standby(g_NEXUS_audioModuleData.hRap, NULL);
    } else {
        rc = BRAP_Resume(g_NEXUS_audioModuleData.hRap);
    }
#else
    BSTD_UNUSED(enabled);
    BSTD_UNUSED(pSettings);
#endif
    return rc;
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
    NEXUS_OBJECT_ASSERT(NEXUS_AudioDecoder, audioDecoder);
    NEXUS_ASSERT_MODULE();
    BDBG_ASSERT(pSimpleSettings);

    *pSimpleSettings = audioDecoder->simple.settings;
}

NEXUS_Error NEXUS_AudioDecoder_SetSimpleSettings_priv(
    NEXUS_AudioDecoderHandle audioDecoder,
    const NEXUS_AudioDecoderSimpleSettings *pSimpleSettings
    )
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_OBJECT_ASSERT(NEXUS_AudioDecoder, audioDecoder);
    NEXUS_ASSERT_MODULE();
    BDBG_ASSERT(pSimpleSettings);

    audioDecoder->simple.settings = *pSimpleSettings;

    rc = NEXUS_AudioDecoder_ApplySettings_priv(audioDecoder);
    if (rc) BERR_TRACE(rc);

    return rc;
}
#endif
