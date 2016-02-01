/***************************************************************************
*     (c)2004-2014 Broadcom Corporation
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
*   API name: AudioDecoder
*    API for audio decoder management.
*
* Revision History:
*
* $brcm_Log: $
*
***************************************************************************/
#include "nexus_audio_module.h"
#include "nexus_pid_channel.h"        /* PidChannelDesc */
#include "nexus_timebase.h"     /* Timebase */
#include "nexus_stc_channel.h"   /* StcChannel */
#include "nexus_audio_priv.h"
#include "priv/nexus_pid_channel_priv.h"


#if NEXUS_NUM_AUDIO_DECODERS

BDBG_MODULE(nexus_audio_decoder);

BTRC_MODULE(ChnChange_DecodeStopAudio, ENABLE);
BTRC_MODULE(ChnChange_DecodeStartAudio, ENABLE);
BTRC_MODULE(ChnChange_DecodeStartAudioRAP, ENABLE);
BTRC_MODULE(ChnChange_DecodeFirstAudio, ENABLE);
BTRC_MODULE(ChnChange_SyncUnmuteAudio, ENABLE); /* TODO: add when sync_channel support available */
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

static BKNI_EventHandle g_watchdogEvent;
static NEXUS_EventCallbackHandle g_watchdogCallback;
static unsigned g_numWatchdogs;

#define LOCK_TRANSPORT()    NEXUS_Module_Lock(g_NEXUS_audioModuleData.transport)
#define UNLOCK_TRANSPORT()  NEXUS_Module_Unlock(g_NEXUS_audioModuleData.transport)

static NEXUS_AudioDecoder g_decoders[NEXUS_NUM_AUDIO_DECODERS];

static void NEXUS_AudioDecoder_P_FirstPts_isr(void *pParam1, int param2, void *pData);
static void NEXUS_AudioDecoder_P_AudioPtsError_isr(void *pParam1, int param2, void *pData);
#if defined (NEXUS_HAS_AUDIO_TSM_LOG_SUPPORT) && BRAP_SUPPORT_TSM_LOG
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
static BERR_Code NEXUS_AudioDecoder_P_GetPtsCallback_isr(void *pContext, BAVC_PTSInfo *pPTSInfo);
static BERR_Code NEXUS_AudioDecoder_P_GetCdbLevelCallback_isr(void *pContext, unsigned *pCdbLevel);
static BERR_Code NEXUS_AudioDecoder_P_StcValidCallback_isr(void *pContext);
static void NEXUS_AudioDecoder_P_Watchdog(void *pContext);
static void NEXUS_AudioDecoder_P_LayerChange(void *pParam);
static NEXUS_Error NEXUS_AudioDecoder_P_Disconnect(void *pHandle, struct NEXUS_AudioInputObject *pInput);
static NEXUS_AudioCodec NEXUS_AudioDecoder_P_MagnumToCodec(BRAP_DSPCHN_AudioType codec,BRAP_DSPCHN_AacXptFormat xptfmt);
static NEXUS_Error NEXUS_AudioDecoder_P_SetMixerInputScaling(NEXUS_AudioDecoderHandle handle);
static void NEXUS_AudioDecoder_P_ChannelChangeReport(void * context);
static bool NEXUS_AudioDecoder_P_CodecSupportsMultichannel(NEXUS_AudioCodec codec);
static void NEXUS_AudioDecoder_P_SetOutputMode(NEXUS_AudioDecoderHandle handle);
static int32_t NEXUS_AudioOutput_P_Vol2dB(int32_t volume);
#if (BCHP_CHIP==7400)
bool NEXUS_AudioDecoder_P_IsPassthru(NEXUS_AudioDecoderHandle handle);
#endif
static void NEXUS_AudioDecoder_P_Watchdog_isr(void *pParam, int iParam, void *pData)
{
    BKNI_EventHandle event = (BKNI_EventHandle)pParam;
    BSTD_UNUSED(iParam);
    BSTD_UNUSED(pData);
    BDBG_ERR(("Raptor watchdog interrupt fired"));
    BKNI_SetEvent(event);
}

static bool g_bSupportedAlgos[BRAP_MAX_AUDIO_TYPES];

/***************************************************************************
Summary:
    Init the audio decoder module
See Also:
    NEXUS_AudioDecoder_Uninit
 ***************************************************************************/
NEXUS_Error NEXUS_AudioDecoder_P_Init(bool *pbSupportAlgos)
{
    NEXUS_Error errCode;

    BDBG_MSG(("Decoder Module Init"));

    g_numWatchdogs = 0;

    BKNI_Memcpy(g_bSupportedAlgos, pbSupportAlgos, sizeof(g_bSupportedAlgos));

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
See Also:
    NEXUS_AudioDecoder_Close
 ***************************************************************************/
NEXUS_AudioDecoderHandle NEXUS_AudioDecoder_Open(
    unsigned index,                               /* which decoder */
    const NEXUS_AudioDecoderOpenSettings *pSettings   /* settings */
    )
{
    BRAP_DEC_ChannelSettings *pChannelSettings;
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
    pDecoder->syncMute = false;
    pDecoder->trickForceStopped = false;
    for ( j = 0; j < NEXUS_AudioDecoderConnectorType_eMax; j++ )
    {
        /* Setup handle linkage */
        NEXUS_AUDIO_INPUT_INIT(&pDecoder->connectors[j], NEXUS_AudioInputType_eDecoder, pDecoder);
        pDecoder->connectors[j].disconnectCb = NEXUS_AudioDecoder_P_Disconnect;

        /* Set format per-connector */
        if ( j == NEXUS_AudioDecoderConnectorType_eStereo )
        {
            pDecoder->connectors[j].format = NEXUS_AudioInputFormat_ePcmStereo;
        }
        else if ( j == NEXUS_AudioDecoderConnectorType_eMultichannel )
        {
            /* The correct value for this is set later if multichannel is enabled */
            pDecoder->connectors[j].format = NEXUS_AudioInputFormat_ePcm5_1;
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

    errCode = BKNI_CreateEvent(&pDecoder->sourceChangedEvent);
    if ( errCode )
    {
        errCode=BERR_TRACE(errCode);
        goto err_source_changed_event;
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

    /* Open Raptor Channel */
    pChannelSettings = BKNI_Malloc(sizeof(BRAP_DEC_ChannelSettings));   /* this structure is HUGE, don't use the stack */
    if ( NULL == pChannelSettings )
    {
        errCode=BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto err_channel_settings_alloc;
    }
    (void)BRAP_DEC_GetChannelDefaultSettings(g_NEXUS_audioModuleData.hRap, pChannelSettings);

    pChannelSettings->bLargeRbuf = g_NEXUS_audioModuleData.moduleSettings.independentDelay;

    errCode = BRAP_DEC_OpenChannel(g_NEXUS_audioModuleData.hRap, &pDecoder->rapChannel, index, pChannelSettings);
    if ( errCode )
    {
        BERR_TRACE(errCode);
        goto err_channel;
    }

    /* Hook up decoder interrupts */
    errCode = BRAP_InstallAppInterruptCb(pDecoder->rapChannel, BRAP_Interrupt_eFirstPtsReady,
                                         NEXUS_AudioDecoder_P_FirstPts_isr, pDecoder, 0);
    if ( errCode )
        { BERR_TRACE(errCode); goto err_cb; }
    errCode = BRAP_InstallAppInterruptCb(pDecoder->rapChannel, BRAP_Interrupt_ePtsError,
                                    NEXUS_AudioDecoder_P_AudioPtsError_isr, pDecoder, 0);
    if ( errCode )
        { BERR_TRACE(errCode); goto err_cb; }
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
    errCode = BRAP_InstallAppInterruptCb(pDecoder->rapChannel, BRAP_Interrupt_eCdbItbUnderflow,
                                    NEXUS_AudioDecoder_P_StatusChange_isr, pDecoder, BRAP_Interrupt_eCdbItbUnderflow);
    if ( errCode )
        { BERR_TRACE(errCode); goto err_cb; }

    LOCK_TRANSPORT();
    NEXUS_Rave_GetDefaultOpenSettings_priv(&raveSettings);
    UNLOCK_TRANSPORT();

    errCode = BRAP_GetBufferConfig(g_NEXUS_audioModuleData.hRap, BRAP_BufferCfgMode_eAdvAudioMode, &raveSettings.config);
    if ( errCode )
    {
        errCode=BERR_TRACE(errCode);
        goto err_buffer_config;
    }

    if ( NULL == pSettings || pSettings->fifoSize == 0 )
    {
        /* Use large (IPSTB) buffers by default */
        raveSettings.config.Cdb.Length *= 2;
        raveSettings.config.Itb.Length *= 2;
        NEXUS_AudioDecoder_GetDefaultOpenSettings(&pDecoder->openSettings);
    }
    else
    {
        /* Make ITB proportional to CDB */
        raveSettings.config.Itb.Length = 1024*((raveSettings.config.Itb.Length/1024) * (pSettings->fifoSize/1024))/(raveSettings.config.Cdb.Length/1024);
        BDBG_ASSERT(0 != raveSettings.config.Itb.Length);
        raveSettings.config.Cdb.Length = pSettings->fifoSize;
        pDecoder->openSettings = *pSettings;
    }

    LOCK_TRANSPORT();
    pDecoder->raveContext = NEXUS_Rave_Open_priv(&raveSettings);
    UNLOCK_TRANSPORT();
    if ( NULL == pDecoder->raveContext )
    {
        BDBG_ERR(("Unable to allocate RAVE context"));
        goto err_rave;
    }


#if NEXUS_HAS_AUDIO_TSM_LOG_SUPPORT
    if (NEXUS_GetEnv("rap_tsmlog"))
    {
    #if BRAP_SUPPORT_TSM_LOG
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
    #else
        BDBG_ERR(("Build with BRAP_SUPPORT_TSM_LOG=y for tsmlog support. (can't be used in kernelmode builds.)"));
    #endif
    }
#endif

    /* Success */
    NEXUS_AudioDecoder_P_GetDefaultSettings(&pDecoder->settings);
    (void)NEXUS_AudioDecoder_SetSettings(pDecoder, &pDecoder->settings);
    BKNI_Free(pChannelSettings);
    pDecoder->opened = true;
    return pDecoder;

err_rave:
err_buffer_config:
err_cb:
    BRAP_DEC_CloseChannel(pDecoder->rapChannel);
err_channel:
    BKNI_Free(pChannelSettings);
    NEXUS_UnregisterEvent(pDecoder->channelChangeReportEventHandler);
err_channel_change_report_event_handler:
    BKNI_DestroyEvent(pDecoder->channelChangeReportEvent);
err_channel_change_report_event:
    NEXUS_UnregisterEvent(pDecoder->layerChangeCallback);
err_layer_callback:
err_channel_settings_alloc:
    BKNI_DestroyEvent(pDecoder->layerChangeEvent);
err_layer_change_event:
    BKNI_DestroyEvent(pDecoder->sourceChangedEvent);
err_source_changed_event:
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
    int i, j;

    NEXUS_OBJECT_ASSERT(NEXUS_AudioDecoder, handle);
    BDBG_ASSERT(true == handle->opened);

    if ( handle->started )
    {
        NEXUS_AudioDecoder_Stop(handle);
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

    /* Remove all outputs */
    for ( i = 0; i < NEXUS_AudioDecoderConnectorType_eMax; i++ )
    {
        for ( j = 0; j < NEXUS_AUDIO_MAX_OUTPUTS; j++ )
        {
            NEXUS_AudioOutputHandle output = handle->outputLists[i].outputs[j];
            if ( NULL != output )
            {
                if (!NEXUS_AudioOutput_P_IsDacSlave(output))
                {
                    if ( ( NEXUS_AudioDecoderConnectorType_eMultichannel == i ) &&
                         ( output->objectType == NEXUS_AudioOutputType_eHdmi ) )
                    {
                        BDBG_MSG(("Removing multichannel MAI from decoder %d rapChannel = %p",
                                  handle->index, (void *)handle->rapChannel));
                        errCode = BRAP_RemoveOutputPort(handle->rapChannel, BRAP_OutputPort_eMai);
                        if ( errCode ) {    /* Report error and continue */
                            errCode = BERR_TRACE(errCode);
                        }
                        errCode = NEXUS_AudioOutput_P_ZeroDelay(output);
                        if ( errCode )
                        {
                            /* Report error and continue */
                            errCode = BERR_TRACE(errCode);
                        }

                        /* Remove I2S0 */
                        BDBG_MSG(("Removing I2S0 Output Port from ch[0]"));
                        errCode = BRAP_RemoveOutputPort(handle->rapChannel, BRAP_OutputPort_eI2s0);
                        if ( errCode ) {    /* Report error and continue */
                            errCode = BERR_TRACE(errCode);
                        }
#if NEXUS_NUM_I2S_OUTPUTS > 1
                        /* Remove I2S1 */
                        BDBG_MSG(("Removing I2S1 Output Port from ch[0]"));
                        errCode = BRAP_RemoveOutputPort(handle->rapChannel, BRAP_OutputPort_eI2s1);
                        if ( errCode ) {    /* Report error and continue */
                            errCode = BERR_TRACE(errCode);
                        }
#endif
#if NEXUS_NUM_I2S_OUTPUTS > 2
                        /* Remove I2S2 */
                        BDBG_MSG(("Removing I2S2 Output Port from ch[0]"));
                        errCode = BRAP_RemoveOutputPort(handle->rapChannel, BRAP_OutputPort_eI2s2);
                        if ( errCode ) {    /* Report error and continue */
                            errCode = BERR_TRACE(errCode);
                        }
#endif
                    }
                    else
                    {
                        BDBG_MSG(("Removing output port %d from decoder %d", output->port, handle->index));
                        errCode = BRAP_RemoveOutputPort(handle->rapChannel, output->port);
                        if ( errCode ) {    /* Report error and continue */
                            errCode = BERR_TRACE(errCode);
                        }
                        errCode = NEXUS_AudioOutput_P_ZeroDelay(output);
                        if ( errCode ) {    /* Report error and continue */
                            errCode = BERR_TRACE(errCode);
                        }
                    }
                }
                /* Clear this output port */
                handle->outputLists[i].outputs[j] = NULL;
                if ( output->objectType == NEXUS_AudioOutputType_eHdmi )
                {
                    handle->hdmiOutput = NULL;
                }
            }
        }
    }

    BRAP_DEC_CloseChannel(handle->rapChannel);

    if(handle->raveContext){
        LOCK_TRANSPORT();
        NEXUS_Rave_Close_priv(handle->raveContext);
        UNLOCK_TRANSPORT();
        handle->raveContext = NULL;
    }
    NEXUS_UnregisterEvent(handle->channelChangeReportEventHandler);
    BKNI_DestroyEvent(handle->channelChangeReportEvent);
    NEXUS_UnregisterEvent(handle->layerChangeCallback);
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
        handle->settings = *pSettings;
    }

    return NEXUS_AudioDecoder_ApplySettings_priv(handle);
}

NEXUS_Error NEXUS_AudioDecoder_SetChannelMode_priv(
    NEXUS_AudioDecoderHandle handle,
    NEXUS_AudioChannelMode channelMode
    )
{
    int i;
    NEXUS_Error errCode = NEXUS_SUCCESS;
    NEXUS_AudioOutputList outputList;
    NEXUS_AudioOutputSettings nOutputSettings;

    BDBG_MSG(("%s(%p,%d)",__FUNCTION__, (void *)handle,channelMode));

    /* Get output lists */
    NEXUS_AudioInput_P_GetOutputs(&handle->connectors[NEXUS_AudioDecoderConnectorType_eStereo], &outputList, false);


    for ( i = 0; i < NEXUS_AUDIO_MAX_OUTPUTS; i++ )
    {
        NEXUS_AudioOutputHandle output = outputList.outputs[i];
        if ( ( NULL != output ) && (!NEXUS_AudioOutput_P_IsDacSlave(output)) )
        {
            NEXUS_AudioOutput_GetSettings(output,&nOutputSettings);
            nOutputSettings.channelMode = channelMode;
            NEXUS_AudioOutput_SetSettings(output,&nOutputSettings);
        }
    }
    return errCode;
}

static const BRAP_DSPCHN_AudioType g_dualMonoCodecs[] =
{
    BRAP_DSPCHN_AudioType_eMpeg,
    BRAP_DSPCHN_AudioType_eAac,
    BRAP_DSPCHN_AudioType_eAacSbr,
    BRAP_DSPCHN_AudioType_eAc3,
    BRAP_DSPCHN_AudioType_eAc3Plus
};

NEXUS_Error NEXUS_AudioDecoder_ApplySettings_priv(
    NEXUS_AudioDecoderHandle handle
    )
{
    NEXUS_Error errCode;
    BRAP_DSPCHN_DualMonoMode dualMonoMode;
    BRAP_DSPCHN_Ac3DualMonoMode ac3DualMonoMode;
    int i;
    BRAP_DEC_ConfigParams *pSettings;

    NEXUS_ASSERT_MODULE();

    BDBG_MSG(("%s(%p)",__FUNCTION__, (void *)handle));
    if (handle->audioParams.sDspChParams.eDecodeMode != BRAP_DSPCHN_DecodeMode_ePassThru)
    {
        switch ( handle->settings.dualMonoMode )
        {
        case NEXUS_AudioDecoderDualMonoMode_eLeft:
            dualMonoMode = BRAP_DSPCHN_DualMonoMode_eLeftMono;
            ac3DualMonoMode = BRAP_DSPCHN_Ac3DualMonoMode_eLtMono;
            BDBG_MSG(("NEXUS_AudioDecoderDualMonoMode_eLeft"));
            break;
        case NEXUS_AudioDecoderDualMonoMode_eRight:
            dualMonoMode = BRAP_DSPCHN_DualMonoMode_eRightMono;
            ac3DualMonoMode = BRAP_DSPCHN_Ac3DualMonoMode_eRtMono;
            BDBG_MSG(("NEXUS_AudioDecoderDualMonoMode_eRight"));
            break;
        case NEXUS_AudioDecoderDualMonoMode_eMix:
            dualMonoMode = BRAP_DSPCHN_DualMonoMode_eDualMonoMix;
            ac3DualMonoMode = BRAP_DSPCHN_Ac3DualMonoMode_eDualMono;
            BDBG_MSG(("NEXUS_AudioDecoderDualMonoMode_eMix"));
            break;
        default:
            dualMonoMode = BRAP_DSPCHN_DualMonoMode_eStereo;
            ac3DualMonoMode = BRAP_DSPCHN_Ac3DualMonoMode_eStereo;
            BDBG_MSG(("BRAP_DSPCHN_DualMonoMode_eStereo"));
            break;
        }

        pSettings = BKNI_Malloc(sizeof(*pSettings));
        if ( NULL == pSettings )
        {
            return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        }

        /* Here we "take apart" the volumeMatrix settings, which are not supported on 7400, and use them to determine the
           desired downmix mode which is supported on the 7400.  */

        if ((handle->settings.volumeMatrix[NEXUS_AudioChannel_eLeft][NEXUS_AudioChannel_eLeft]   != 0) &&
            (handle->settings.volumeMatrix[NEXUS_AudioChannel_eRight][NEXUS_AudioChannel_eRight] == 0) &&
            (handle->settings.volumeMatrix[NEXUS_AudioChannel_eLeft][NEXUS_AudioChannel_eRight] == 0) &&
            (handle->settings.volumeMatrix[NEXUS_AudioChannel_eRight][NEXUS_AudioChannel_eLeft] != 0))
        {
            /* baudio_downmix_left */
            BDBG_MSG(("Setting downmix left"));
            handle->mono_to_all = false;
            errCode = NEXUS_AudioDecoder_SetChannelMode_priv(handle,NEXUS_AudioChannelMode_eLeft);
            if ( errCode )
            {
                return BERR_TRACE(errCode);
            }
        }

        else if ((handle->settings.volumeMatrix[NEXUS_AudioChannel_eLeft][NEXUS_AudioChannel_eLeft]   == 0) &&
            (handle->settings.volumeMatrix[NEXUS_AudioChannel_eRight][NEXUS_AudioChannel_eRight] != 0) &&
            (handle->settings.volumeMatrix[NEXUS_AudioChannel_eLeft][NEXUS_AudioChannel_eRight] != 0) &&
            (handle->settings.volumeMatrix[NEXUS_AudioChannel_eRight][NEXUS_AudioChannel_eLeft] == 0))
        {
            /* baudio_downmix_right */
            BDBG_MSG(("Setting downmix right"));
            handle->mono_to_all = false;
            errCode = NEXUS_AudioDecoder_SetChannelMode_priv(handle,NEXUS_AudioChannelMode_eRight);
            if ( errCode )
            {
                return BERR_TRACE(errCode);
            }
        }
        else if ((handle->settings.volumeMatrix[NEXUS_AudioChannel_eLeft][NEXUS_AudioChannel_eLeft] ==
                  handle->settings.volumeMatrix[NEXUS_AudioChannel_eLeft][NEXUS_AudioChannel_eRight] ) &&
                (handle->settings.volumeMatrix[NEXUS_AudioChannel_eRight][NEXUS_AudioChannel_eRight] ==
                 handle->settings.volumeMatrix[NEXUS_AudioChannel_eRight][NEXUS_AudioChannel_eLeft] ))
        {
            /* baudio_downmix_monomix */
            BDBG_MSG(("Setting downmix monomix"));
            handle->mono_to_all = true;
            errCode = NEXUS_AudioDecoder_SetChannelMode_priv(handle,NEXUS_AudioChannelMode_eStereo);
            if ( errCode )
            {
                return BERR_TRACE(errCode);
            }
        }
        else
        {
            if (!((handle->settings.volumeMatrix[NEXUS_AudioChannel_eLeft][NEXUS_AudioChannel_eLeft]   != 0) &&
                 (handle->settings.volumeMatrix[NEXUS_AudioChannel_eRight][NEXUS_AudioChannel_eRight] != 0) &&
                 (handle->settings.volumeMatrix[NEXUS_AudioChannel_eLeft][NEXUS_AudioChannel_eRight] == 0) &&
                 (handle->settings.volumeMatrix[NEXUS_AudioChannel_eRight][NEXUS_AudioChannel_eLeft] == 0)))
            {
                BDBG_ERR(("unsupported volumeMatrix settings"));
            }
            /* baudio_downmix_stereo, baudio_downmix_multichannel, baudio_downmix_none */
            BDBG_MSG(("Setting standard downmix mode"));
            handle->mono_to_all = false;
            errCode = NEXUS_AudioDecoder_SetChannelMode_priv(handle,BRAP_OP_ChannelOrder_eNormal);
            if ( errCode )
            {
                return BERR_TRACE(errCode);
            }
        }

        /* Set mixer input volume for start/stop */
        errCode = NEXUS_AudioDecoder_P_SetMixerInputScaling(handle);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }

        for ( i=0; i < (int)(sizeof(g_dualMonoCodecs)/sizeof(BRAP_DSPCHN_AudioType)); i++ )
        {
            if (g_bSupportedAlgos[g_dualMonoCodecs[i]])
            {
                errCode = BRAP_DEC_GetCurrentConfig(handle->rapChannel,
                                                   g_dualMonoCodecs[i],
                                                   pSettings);
                if ( errCode )
                {
                    (void)BERR_TRACE(errCode);
                }
                pSettings->eDualMonoMode = dualMonoMode;

                errCode = BRAP_DEC_SetConfig(handle->rapChannel, pSettings);
                if ( errCode )
                {
                    BDBG_ERR(("NEXUS_AudioDecoder %d BRAP_DSPCHN_AudioType %d, dualMonoMode %d",
                              handle->index,g_dualMonoCodecs[i],dualMonoMode));
                    return BERR_TRACE(errCode);
                }
            }
        }

        BKNI_Free(pSettings);
    }
#if NEXUS_HAS_ASTM
    /* TODO: tsm flag from programSettings as stcChannel != NULL */
    BRAP_DSPCHN_EnableTSM(handle->rapChannel, handle->astm.settings.enableTsm);
    /* TODO: apply playback changes to rap */
    NEXUS_AudioDecoder_P_SetTsm(handle);
#endif
    return NEXUS_SUCCESS;
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
    raveSettings.bandHold = handle->audioParams.bPlayback; /* bandHold is required for non-PCR playback only */
    raveSettings.continuityCountEnabled = !handle->isPlayback;
    raveSettings.numOutputBytesEnabled = true;
    raveSettings.audioDescriptor = false; /* Audio Descriptors not supported on this platform */
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
    BRAP_DSPCHN_AudioType eAlgoType = BRAP_DSPCHN_AudioType_eMpeg;
    bool useTsm;
    static NEXUS_PidChannelStatus pidChannelStatus;
    static NEXUS_RaveStatus raveStatus;
    NEXUS_StcChannelSettings stcSettings;
    BRAP_DEC_ConfigParams configParams;
    BAVC_StreamType streamType=BAVC_StreamType_eTsMpeg;
    unsigned discardThreshold;
    unsigned stcChannelIndex;
    unsigned tsmGaThreshold;

    BDBG_ENTER(NEXUS_AudioDecoder_Start);

    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioDecoder);
    BDBG_ASSERT(NULL != pProgram);

    if ( handle->started )
    {
        BDBG_ERR(("This decoder is already started.  Please call stop first."));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    if (handle->index == 0) {
        BTRC_TRACE(ChnChange_DecodeStartAudio, START);
    }

    NEXUS_AudioDecoder_P_TrickReset(handle); /* reset trick state on start */

    /* Save Program */
    handle->programSettings = *pProgram;

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

    /* Mark channel status invalid */
    handle->channelStatusValid = false;
    if ( NULL == pProgram->stcChannel )
    {
        NEXUS_StcChannel_GetDefaultSettings(handle->index, &stcSettings);
    }
    else
    {
        NEXUS_StcChannel_GetSettings(pProgram->stcChannel, &stcSettings);
    }


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
            eAlgoType = BRAP_DSPCHN_AudioType_eAc3;
            break;
        case NEXUS_AudioCodec_eAc3Plus:
            eAlgoType = BRAP_DSPCHN_AudioType_eAc3Plus;
            break;
        case NEXUS_AudioCodec_eAac:
            eAlgoType = BRAP_DSPCHN_AudioType_eAac;
            break;
        case NEXUS_AudioCodec_eAacPlus:
        case NEXUS_AudioCodec_eAacPlusAdts:
            /* baudio_format_aac_plus_adts is also handled here as it has the same value as baudio_format_aac_plus */
            eAlgoType = BRAP_DSPCHN_AudioType_eAacSbr;
            break;
        case NEXUS_AudioCodec_eDts:
            eAlgoType = BRAP_DSPCHN_AudioType_eDts;
            break;
        case NEXUS_AudioCodec_eWmaStd:
            eAlgoType = BRAP_DSPCHN_AudioType_eWmaStd;
            break;
        case NEXUS_AudioCodec_eWmaPro:
            eAlgoType = BRAP_DSPCHN_AudioType_eWmaPro;
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
        default:
            return BERR_TRACE(BERR_INVALID_PARAMETER);
        }
        #if (BCHP_CHIP==7400)
        /* 7400 can passthru, but not decode DTS */
        if ((pProgram->codec == NEXUS_AudioCodec_eDts) && (!NEXUS_AudioDecoder_P_IsPassthru(handle)))
        {
            BDBG_ERR(("NEXUS_AudioCodec_eDts codec only supported for passthru."));
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }
        #endif
        if ( !g_bSupportedAlgos[eAlgoType] )
        {
            BDBG_ERR(("Audio codec not supported."));
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }

    }
#if 0   /* TODO */
    /* Set Config params - this can be changed on the fly, but here we're just
    doing it at start time. */
    errCode = BRAP_DEC_GetCurrentConfig(rap_channel->rapChannel, eAlgoType, &configParams);
    if (errCode) { return BERR_TRACE(errCode);}

    configParams.eDualMonoMode = rap_channel->dual_mono_mode;

    errCode = BRAP_DEC_SetConfig(rap_channel->rapChannel, &configParams);
    if (errCode) { return BERR_TRACE(errCode);}
#else
    BSTD_UNUSED(configParams);
#endif

    /* Set Audio params - this can only be set at start time. */
    errCode = BRAP_DEC_GetDefaultAudioParams(handle->rapChannel, &handle->audioParams);
    if (errCode) { return BERR_TRACE(errCode);}

    if (pProgram->stcChannel) {
        LOCK_TRANSPORT();
        NEXUS_StcChannel_GetIndex_priv(pProgram->stcChannel, &stcChannelIndex);
        UNLOCK_TRANSPORT();
    }
    else {
        stcChannelIndex = 0;
    }
    handle->audioParams.eTimebase =  stcChannelIndex; /* BRAP_ChannelParams.eTimebase is misnamed. It is not a timebase (i.e. DPCR). It is an STC index. */
    /* tell the decoder what type of TSM to do based on NEXUS_StcChannelMode, not transport source */
    handle->audioParams.bPlayback = (stcSettings.mode != NEXUS_StcChannelMode_ePcr);
    handle->audioParams.sDspChParams.i32AVOffset = handle->settings.ptsOffset;
    handle->audioParams.sDspChParams.eType = eAlgoType;
    handle->audioParams.sDspChParams.eStreamType = streamType;

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

        discardThreshold = (handle->isPlayback)?30:3;   /* seconds */
        discardThreshold *= (handle->isDss)?27000000:45000; /* Convert to PTS units (27 MHz DSS or 45 KHz MPEG */

        errCode = BRAP_DSPCHN_SetTSMDiscardThreshold(handle->rapChannel, discardThreshold);
        if (errCode!=BERR_SUCCESS) { BERR_TRACE(errCode); goto err_tsm; }

        tsmGaThreshold = pidChannelStatus.originalTransportType==NEXUS_TransportType_eAvi?0x30:0x8;
        errCode = BRAP_DSPCHN_SetTsmGAThreshold(handle->rapChannel, tsmGaThreshold);
        if (errCode!=BERR_SUCCESS) { BERR_TRACE(errCode); goto err_tsm; }


        /* Determine TSM mode */
        if ( NULL == pProgram->stcChannel ||
             NULL != NEXUS_GetEnv("force_vsync") ||
             pidChannelStatus.originalTransportType == NEXUS_TransportType_eEs ||
             pidChannelStatus.transportType == NEXUS_TransportType_eEs )
        {
            useTsm = false;
        }
        else
        {
            useTsm = true;
        }
        handle->tsmPermitted = useTsm;
        NEXUS_AudioDecoder_P_SetTsm(handle);

        if ( pProgram->codec == NEXUS_AudioCodec_eAacPlusLoas )
        {
            handle->audioParams.sDspChParams.eAacXptFormat = BRAP_DSPCHN_AacXptFormat_eLoas;
        }
        else
        {
            handle->audioParams.sDspChParams.eAacXptFormat = BRAP_DSPCHN_AacXptFormat_eAdts;
        }

        /* We're ready to start.  Build up lists of outputs to check for configuration changes. */
        errCode = NEXUS_AudioDecoder_P_Start(handle);
        if ( errCode )
        {
            errCode=BERR_TRACE(errCode);
            goto err_start;
        }
    }

    handle->started = true;

    if (handle->index == 0) {
        BTRC_TRACE(ChnChange_DecodeStartAudio, STOP);
    }

    BDBG_LEAVE(NEXUS_AudioDecoder_Start);

    /* Success */
    return BERR_SUCCESS;

err_start:
err_tsm:
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

    BDBG_ASSERT(NULL != handle);

    BDBG_ENTER(NEXUS_AudioDecoder_P_Start);

    pProgram = &handle->programSettings;

    if ( handle->programSettings.input || !handle->started )
    {
        int i, j;
        bool changed=false;
        bool compressedIsSimul=false;
        BRAP_OutputChannelPair stereoPair = BRAP_OutputChannelPair_eLR;
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
                BDBG_MSG(("NO Output change detected for connector type %d (firstOld=%p, firstNew=%p)", i, (void *)outputLists[i].outputs[0], (void *)handle->outputLists[i].outputs[0]));
            }
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
            handle->audioParams.sDspChParams.eDecodeMode = BRAP_DSPCHN_DecodeMode_ePassThru;
            handle->audioParams.sAudioOutputParams.uiOutputBitsPerSample = 16;
            handle->stc.priority = 1;
            BDBG_MSG(("Decoder %d is a passthrough channel", handle->index));
        }
        else
        {
            /* Standard decode channel.  Compressed will be simul */
            compressedIsSimul = true;
            handle->stc.priority = 0;
            /* Determine decoder mode as decode or simul based on presence of compressed outputs */
            if ( outputLists[NEXUS_AudioDecoderConnectorType_eCompressed].outputs[0] != NULL )
            {
                /* Simul Mode */
                handle->audioParams.sDspChParams.eDecodeMode = BRAP_DSPCHN_DecodeMode_eSimulMode;
                handle->audioParams.sSimulPtAudioOutputParams.uiOutputBitsPerSample = 16;
                /* Copy the mixer and Spdif FM params from the corresponding params of first context */
                handle->audioParams.sSimulPtAudioOutputParams.sMixerParams = handle->audioParams.sAudioOutputParams.sMixerParams;
                handle->audioParams.sSimulPtAudioOutputParams.sSpdifFmParams = handle->audioParams.sAudioOutputParams.sSpdifFmParams;
                BDBG_MSG(("Decoder %d is a simul channel", handle->index));
            }
            else
            {
                /* Decode Mode */
                handle->audioParams.sDspChParams.eDecodeMode = BRAP_DSPCHN_DecodeMode_eDecode;
                handle->audioParams.sAudioOutputParams.uiOutputBitsPerSample = 24;
                BDBG_MSG(("Decoder %d is a decode channel", handle->index));
            }
            /* Determine multichannel mode based upon presence of multichannel outputs */
            if ( outputLists[NEXUS_AudioDecoderConnectorType_eMultichannel].outputs[0] != NULL )
            {
                /* Multichannel is active.  Stereo will be downmixed */
                stereoPair = BRAP_OutputChannelPair_eDownMixedLR;
                handle->audioParams.sDspChParams.bEnableStereoDownmixPath = true;   /* In multichannel mode, enable simultaneous downmix */
            }
            else
            {
                handle->audioParams.sDspChParams.bEnableStereoDownmixPath = false;  /* Not required except for multichannel */
            }
            BDBG_MSG(("%sdownmixing stereo pair",handle->audioParams.sDspChParams.bEnableStereoDownmixPath?"":"NOT "));
        }

        /* This process is slow and involves memory allocation/free.  Only do it if something changed */
        if ( changed )
        {
            /* Remove all outputs */
            for ( i = 0; i < NEXUS_AudioDecoderConnectorType_eMax; i++ )
            {
                for ( j = 0; j < NEXUS_AUDIO_MAX_OUTPUTS; j++ )
                {
                    NEXUS_AudioOutputHandle output = handle->outputLists[i].outputs[j];
                    if ( output )
                    {
                        if (!NEXUS_AudioOutput_P_IsDacSlave(output))
                        {
                            if ( ( NEXUS_AudioDecoderConnectorType_eMultichannel == i ) &&
                                 ( output->objectType == NEXUS_AudioOutputType_eHdmi ) )
                            {
                                BDBG_MSG(("Removing multichannel MAI from decoder %d rapChannel = %p",
                                          handle->index, (void *)handle->rapChannel));
                                errCode = BRAP_RemoveOutputPort(handle->rapChannel, BRAP_OutputPort_eMai);
                                if ( errCode ) {    /* Report error and continue */
                                    errCode = BERR_TRACE(errCode);
                                }
                                errCode = NEXUS_AudioOutput_P_ZeroDelay(output);
                                if ( errCode )
                                {
                                    /* Report error and continue */
                                    errCode = BERR_TRACE(errCode);
                                }
                                /* Remove I2S0 */
                                BDBG_MSG(("Removing I2S0 Output Port from ch[0]"));
                                errCode = BRAP_RemoveOutputPort(handle->rapChannel, BRAP_OutputPort_eI2s0);
                                if ( errCode ) {    /* Report error and continue */
                                    errCode = BERR_TRACE(errCode);
                                }
#if NEXUS_NUM_I2S_OUTPUTS > 1
                                /* Remove I2S1 */
                                BDBG_MSG(("Removing I2S1 Output Port from ch[0]"));
                                errCode = BRAP_RemoveOutputPort(handle->rapChannel, BRAP_OutputPort_eI2s1);
                                if ( errCode ) {    /* Report error and continue */
                                    errCode = BERR_TRACE(errCode);
                                }
#endif
#if NEXUS_NUM_I2S_OUTPUTS > 2
                                /* Remove I2S2 */
                                BDBG_MSG(("Removing I2S2 Output Port from ch[0]"));
                                errCode = BRAP_RemoveOutputPort(handle->rapChannel, BRAP_OutputPort_eI2s2);
                                if ( errCode ) {    /* Report error and continue */
                                    errCode = BERR_TRACE(errCode);
                                }
#endif
                            }
                            else
                            {
                                BDBG_MSG(("Removing output port %d from decoder %d", output->port, handle->index));
                                errCode = BRAP_RemoveOutputPort(handle->rapChannel, output->port);
                                if ( errCode ) {    /* Report error and continue */
                                    errCode = BERR_TRACE(errCode);
                                }
                                errCode = NEXUS_AudioOutput_P_ZeroDelay(output);
                                if ( errCode ) {    /* Report error and continue */
                                    errCode = BERR_TRACE(errCode);
                                }
                            }
                        }
                        /* Clear this output port */
                        handle->outputLists[i].outputs[j] = NULL;
                        if ( output->objectType == NEXUS_AudioOutputType_eHdmi )
                        {
                            handle->hdmiOutput = NULL;
                        }
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
                    if (!NEXUS_AudioOutput_P_IsDacSlave(output))
                    {
                        if ( output->objectType == NEXUS_AudioOutputType_eHdmi )
                        {
                            BRAP_OutputSettings out_cfg;
                            handle->hdmiOutput = output;

                            errCode = BRAP_GetOutputConfig(g_NEXUS_audioModuleData.hRap, BRAP_OutputPort_eMai, &out_cfg);
                            if ( errCode ) {    /* Report error and continue */
                                errCode = BERR_TRACE(errCode);
                            }
                            out_cfg.eOutput = BRAP_OutputPort_eMai;
                            out_cfg.uOutputPortSettings.sMaiSettings.eMaiMuxSelector = BRAP_OutputPort_eFlex;   /* Route MAI to flex */
#if BCHP_CHIP != 7401 && BCHP_CHIP != 7403
                            out_cfg.uOutputPortSettings.sMaiSettings.eMaiAudioFormat = BRAP_OP_MaiAudioFormat_eSpdif2Channel;
#else
                            out_cfg.uOutputPortSettings.sMaiSettings.eMaiAudioFormat = BRAP_OP_MaiAudioFormat_ePcmStereo;
#endif
                            errCode = BRAP_SetOutputConfig(g_NEXUS_audioModuleData.hRap, &out_cfg);
                            if ( errCode ) {    /* Report error and continue */
                                errCode = BERR_TRACE(errCode);
                            }
                        }
                        NEXUS_AudioOutput_P_ApplyConfig(output); /* must set independent delay before adding port */
                        BDBG_MSG(("Adding stereo output %d to decoder %d with channel pair %d", output->port, handle->index, stereoPair));
                        errCode = BRAP_AddOutputPort(handle->rapChannel, false, stereoPair, output->port);
                    }
                    else
                    {
                        errCode = NEXUS_SUCCESS;
                    }
                    if ( errCode )
                    {
                        /* Report and continue */
                        errCode = BERR_TRACE(errCode);
                    }
                    else
                    {
                        handle->outputLists[NEXUS_AudioDecoderConnectorType_eStereo].outputs[i] = output;
                    }
                }
            }

            /* Add multichannel ports */
            for ( i = 0; i < NEXUS_AUDIO_MAX_OUTPUTS; i++ )
            {
                NEXUS_AudioOutputHandle output = outputLists[NEXUS_AudioDecoderConnectorType_eMultichannel].outputs[i];
                if ( NULL == output )
                {
                    break;
                }
                else if ( output->objectType == NEXUS_AudioOutputType_eHdmi )
                {
                    BRAP_OutputSettings out_cfg;

                    errCode = BRAP_GetOutputConfig(g_NEXUS_audioModuleData.hRap, BRAP_OutputPort_eMai, &out_cfg);
                    if ( errCode ) {    /* Report error and continue */
                        errCode = BERR_TRACE(errCode);
                    }
                    out_cfg.eOutput = BRAP_OutputPort_eMai;
#if BCHP_CHIP != 7401 && BCHP_CHIP != 7403
                    out_cfg.uOutputPortSettings.sMaiSettings.eMaiMuxSelector = BRAP_OutputPort_eFlex;   /* Route MAI to flex */
                    out_cfg.uOutputPortSettings.sMaiSettings.eMaiAudioFormat = BRAP_OP_MaiAudioFormat_eSpdif6Channel;
#endif
                    errCode = BRAP_SetOutputConfig(g_NEXUS_audioModuleData.hRap, &out_cfg);
                    if ( errCode ) {    /* Report error and continue */
                        errCode = BERR_TRACE(errCode);
                    }
                    NEXUS_AudioOutput_P_ApplyConfig(output); /* set config before adding port */

                    BDBG_MSG(("Adding output port %d to decoder %d (multichannel)", output->port, handle->index));
                    /* Add I2S0 */
                    BDBG_MSG(("Adding I2S0 Output Port to ch[0] for PCM 5.1"));
                    errCode = BRAP_AddOutputPort(handle->rapChannel, false, BRAP_OutputChannelPair_eLR, BRAP_OutputPort_eI2s0);
                    if ( errCode ) {    /* Report error and continue */
                        errCode = BERR_TRACE(errCode);
                    }

#if NEXUS_NUM_I2S_OUTPUTS > 1
                    /* Add I2S1 */
                    BDBG_MSG(("Adding I2S1 Output Port to ch[0] for PCM 5.1"));
                    errCode = BRAP_AddOutputPort(handle->rapChannel, false, BRAP_OutputChannelPair_eLRSurround, BRAP_OutputPort_eI2s1);
                    if ( errCode ) {    /* Report error and continue */
                        errCode = BERR_TRACE(errCode);
                    }
#endif

#if NEXUS_NUM_I2S_OUTPUTS > 2
                    /* Add I2S2 */
                    BDBG_MSG(("Adding I2S2 Output Port to ch[0] for PCM 5.1"));
                    errCode = BRAP_AddOutputPort(handle->rapChannel, false, BRAP_OutputChannelPair_eCentreLF, BRAP_OutputPort_eI2s2);
                    if ( errCode ) {    /* Report error and continue */
                        errCode = BERR_TRACE(errCode);
                    }
#endif
                    /* Add HDMI */
                    BDBG_MSG(("Adding MAI Output Port to ch[0] for PCM 5.1"));
                    errCode = BRAP_AddOutputPort(handle->rapChannel, false, BRAP_OutputChannelPair_eLR, BRAP_OutputPort_eMai);

                    if ( errCode ) {    /* Report error and continue */
                        errCode = BERR_TRACE(errCode);
                    }
                    else
                    {
                        BDBG_MSG(("(%p)->outputLists[NEXUS_AudioDecoderConnectorType_eMultichannel].outputs[%d] = %p",
                                  (void *)handle,i, (void *)output));
                        handle->outputLists[NEXUS_AudioDecoderConnectorType_eMultichannel].outputs[i] = output;
                    }

                    BDBG_MSG(("handle->hdmiOutput = output (%p)", (void *)output));
                    handle->hdmiOutput = output;

                    /* only support PCM 5.1 on 7400, not 7.1 */
                    handle->connectors[NEXUS_AudioDecoderConnectorType_eMultichannel].format = NEXUS_AudioInputFormat_ePcm5_1;
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
                    BDBG_MSG(("Adding compressed output %d to decoder %d with LR channel pair", output->port, handle->index));
                    {
                        BRAP_OutputSettings out_cfg;

                        if ( output->objectType == NEXUS_AudioOutputType_eHdmi )
                        {
                            handle->hdmiOutput = output;

                            errCode = BRAP_GetOutputConfig(g_NEXUS_audioModuleData.hRap, BRAP_OutputPort_eMai, &out_cfg);
                            if ( errCode ) {    /* Report error and continue */
                                errCode = BERR_TRACE(errCode);
                            }
                            out_cfg.eOutput = BRAP_OutputPort_eMai;
#if BCHP_CHIP != 7401 && BCHP_CHIP != 7403
                            out_cfg.uOutputPortSettings.sMaiSettings.eMaiMuxSelector = BRAP_OutputPort_eFlex;   /* Route MAI to flex */
                            out_cfg.uOutputPortSettings.sMaiSettings.eMaiAudioFormat = BRAP_OP_MaiAudioFormat_eSpdifCompressed;
#endif
                            errCode = BRAP_SetOutputConfig(g_NEXUS_audioModuleData.hRap, &out_cfg);
                            if ( errCode ) {    /* Report error and continue */
                                errCode = BERR_TRACE(errCode);
                            }
                        }
                    }
                    NEXUS_AudioOutput_P_ApplyConfig(output); /* must set independent delay before adding port */
                    errCode = BRAP_AddOutputPort(handle->rapChannel, compressedIsSimul, BRAP_OutputChannelPair_eLR, output->port);
                    if ( errCode )
                    {
                        /* Report and continue */
                        errCode = BERR_TRACE(errCode);
                    }
                    else
                    {
                        handle->outputLists[NEXUS_AudioDecoderConnectorType_eCompressed].outputs[i] = output;
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

        /* Must compute this here for downmix checking later.  May compute twice, but this is harmless. */
        NEXUS_AudioDecoder_P_SetOutputMode(handle);

        /* Notify downstream objects we're about to start - should be done after outputs are connected on 7400/7401 */
        /* This will trigger connections on the 7405/3563/... */
        for ( i = 0; i < NEXUS_AudioDecoderConnectorType_eMax; i++ )
        {
            /* Only call this for inputs about to actually start */
            if ( outputLists[i].outputs[0] != NULL )
            {
                BDBG_MSG(("Preparing to start path %d", i));

                /* TODO -- convert to PrepareToStartWithProcessing and start */
                NEXUS_AudioInput_P_PrepareToStartWithProcessing(&handle->connectors[i]);
            }
        }
    }

    /* Setup StcChannel */
    if ( pProgram->stcChannel )
    {
        errCode = NEXUS_AudioDecoder_P_StartStcChannel(handle);
        if (errCode) { errCode = BERR_TRACE(errCode); goto err_set_tsm; }
    }

    /* After all that, we're ready to go.  Start. */
    handle->ptsErrorCount = 0;
    /* Re-apply settings, needed to determine output mode */
    NEXUS_AudioDecoder_ApplySettings_priv(handle);

    BDBG_MSG(("Starting Decoder %d", handle->index));
    if (handle->index == 0) {
        BTRC_TRACE(ChnChange_DecodeStartAudioRAP, START);
    }

    /* Setup proper decoder output mode */
    NEXUS_AudioDecoder_P_SetOutputMode(handle);

    BDBG_MSG(("Starting Decoder %d", handle->index));
    handle->locked = false;
    handle->fakeFramesDecoded = 0;
    errCode = BRAP_DEC_Start(handle->rapChannel, &handle->audioParams);
    if (handle->index == 0) {
        BTRC_TRACE(ChnChange_DecodeStartAudioRAP, STOP);
    }
    if ( errCode && !handle->started ) { errCode = BERR_TRACE(errCode); goto err_dec_start; }

#if NEXUS_HAS_AUDIO_TSM_LOG_SUPPORT
    if (NEXUS_GetEnv("rap_tsmlog"))
    {
        #if BRAP_SUPPORT_TSM_LOG
        /* Enable TSM Log */
        errCode = BRAP_DSPCHN_EnableTsmLog(handle->rapChannel, true);
        if(errCode != BERR_SUCCESS)
        {
            BDBG_ERR(("RAP TSM Log enable failed for channel"));
        }
        #else
        BDBG_ERR(("Build with BRAP_SUPPORT_TSM_LOG=y for tsmlog support. (can't be used in kernelmode builds.)"));
        #endif
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

    if ( NULL == handle->programSettings.input )
    {
        errCode = BRAP_DSPCHN_SetPTSOffset(handle->rapChannel, (int32_t)handle->syncSettings.delay + handle->settings.ptsOffset);
        if (errCode) { errCode = BERR_TRACE(errCode); goto err_set_pts_offset; }
    }

    /* PR:49294 sync/astm must know that audio has been started *every* time it is started */
    if (handle->syncSettings.startCallback_isr) {
        BKNI_EnterCriticalSection();
        (*handle->syncSettings.startCallback_isr)(handle->syncSettings.callbackContext, 0);
        BKNI_LeaveCriticalSection();
    }
    BDBG_LEAVE(NEXUS_AudioDecoder_P_Start);

    return BERR_SUCCESS;

err_set_pts_offset:
    NEXUS_AudioDecoder_P_Stop(handle, true);
    return errCode;

err_dec_start:
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

    if ( !handle->started )
    {
        BDBG_ERR(("Decoder not started"));
        return;
    }

    if (handle->index == 0) {
        BTRC_TRACE(ChnChange_Total_Audio, START);
        BTRC_TRACE(ChnChange_DecodeStopAudio, START);
    }

    /* Make sure mutes are stopped before stopping */
    NEXUS_AudioDecoder_P_SetTrickMute(handle, false);
    NEXUS_AudioDecoder_P_SetSyncMute(handle, false);

    /* Mark channel status invalid */
    handle->channelStatusValid = false;

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
        NEXUS_OBJECT_RELEASE(handle, NEXUS_PidChannel, handle->programSettings.pidChannel);
    }
    /* PidChannel and input are mutually exclusive - but coverity does not like the comparison */
    else if ( handle->programSettings.input )
    {
        int i;
        for ( i = 0; i < NEXUS_AudioDecoderConnectorType_eMax; i++ )
        {
            if ( i == NEXUS_AudioDecoderConnectorType_eMultichannel )
                continue;

            NEXUS_AudioInput_P_RemoveInput(&handle->connectors[i], handle->programSettings.input);
        }
    }

    if (handle->index == 0) {
        BTRC_TRACE(ChnChange_DecodeStopAudio, STOP);
    }
    BKNI_Memset(&handle->programSettings, 0, sizeof(handle->programSettings));
    /* TODO: notify sync */
    BDBG_LEAVE(NEXUS_AudioDecoder_Stop);
}

NEXUS_Error NEXUS_AudioDecoder_P_Stop(NEXUS_AudioDecoderHandle handle, bool flush)
{
    NEXUS_Error errCode;

    BDBG_ENTER(NEXUS_AudioDecoder_P_Stop);

    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioDecoder);

    if ( handle->running )
    {

#if defined (NEXUS_HAS_AUDIO_TSM_LOG_SUPPORT) && BRAP_SUPPORT_TSM_LOG
        if (NEXUS_GetEnv("rap_tsmlog"))
        {
            /* Disable TSM Log */
            errCode = BRAP_DSPCHN_EnableTsmLog(handle->rapChannel, false);
            if(errCode != BERR_SUCCESS)
            {
                BDBG_ERR(("RAP TSM Log disable failed for channel"));
            }
        }
#endif

        errCode = BRAP_DEC_Stop(handle->rapChannel);
        if ( errCode )
        {
            errCode = BERR_TRACE(errCode);
        }

        if ( handle->programSettings.pidChannel && flush )
        {
            LOCK_TRANSPORT();
            NEXUS_Rave_Disable_priv(handle->raveContext);
            NEXUS_Rave_Flush_priv(handle->raveContext);
            UNLOCK_TRANSPORT();
        }

        handle->running = false;

        if (handle->programSettings.stcChannel)
        {
            NEXUS_AudioDecoder_P_StopStcChannel(handle);
        }

        /* PR:49294 sync/astm must know that audio has been started *every* time it is started */
        if (handle->syncSettings.startCallback_isr) {
            BKNI_EnterCriticalSection();
            (*handle->syncSettings.startCallback_isr)(handle->syncSettings.callbackContext, 0);
            BKNI_LeaveCriticalSection();
        }
    }

    BDBG_LEAVE(NEXUS_AudioDecoder_P_Stop);

    return BERR_SUCCESS;
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

    rc = BRAP_DEC_DisableForFlush(handle->rapChannel);
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

    rc = BRAP_DEC_Flush(handle->rapChannel);
    if (rc)
    {
        return BERR_TRACE(rc);
    }

    handle->channelStatusValid = false;

    if ( handle->programSettings.pidChannel )
    {
        LOCK_TRANSPORT();
        NEXUS_Rave_Enable_priv(handle->raveContext);
        UNLOCK_TRANSPORT();
    }
    return BERR_SUCCESS;
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

    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioDecoder);
    BDBG_ASSERT(NULL != pStatus);

    BKNI_Memset(pStatus, 0, sizeof(NEXUS_AudioDecoderStatus));
    pStatus->started = handle->started;
    pStatus->locked = handle->locked;
    pStatus->codec = handle->programSettings.codec;
    pStatus->ptsType = NEXUS_PtsType_eInterpolatedFromInvalidPTS;
    pStatus->numWatchdogs = g_numWatchdogs;

    if(!handle->running)
    {
        return BERR_SUCCESS;
    }

    BKNI_EnterCriticalSection();
    if ( handle->programSettings.pidChannel )
    {
        NEXUS_Rave_GetCdbBufferInfo_isr(handle->raveContext, &depth, &size);
    }
    if(handle->running)
    {
        rc = BRAP_DSPCHN_GetCurrentPTS_isr(handle->rapChannel, &ptsInfo);
    }
    BKNI_LeaveCriticalSection();

    if(rc==BERR_SUCCESS)
    {
        pStatus->pts = ptsInfo.ui32RunningPts;
        pStatus->ptsType = ptsInfo.ePtsType == BRAP_DSPCHN_PtsType_eCoded ? NEXUS_PtsType_eCoded :
                         ptsInfo.ePtsType == BRAP_DSPCHN_PtsType_eInterpolatedFromValidPTS ? NEXUS_PtsType_eInterpolatedFromValidPTS : NEXUS_PtsType_eInterpolatedFromInvalidPTS;
    }

    pStatus->fifoDepth = depth;
    pStatus->fifoSize = size;

    if ( handle->programSettings.pidChannel )
    {
        NEXUS_Module_Lock(g_NEXUS_audioModuleData.transport);
        NEXUS_Rave_GetAudioFrameCount_priv(handle->raveContext, &pStatus->queuedFrames);
        NEXUS_Module_Unlock(g_NEXUS_audioModuleData.transport);
    }

    pStatus->tsm = handle->tsmEnabled;
    pStatus->timebase = handle->audioParams.eTimebase;
    pStatus->ptsErrorCount = handle->ptsErrorCount;

    if (( !handle->channelStatusValid ) || ( handle->trickMute ))
    {
        /* Raptor will report bogus information until it's locked.  Just bail out with what we have */
        pStatus->codec = NEXUS_AudioCodec_eUnknown;
        return BERR_SUCCESS;
    }

    BRAP_DSPCHN_GetDecoderLockStatus(handle->rapChannel,&handle->channelStatusValid);

    if ( !handle->channelStatusValid )
    {
        /* Raptor will report bogus information until it's locked.  Just bail out with what we have */
        pStatus->codec = NEXUS_AudioCodec_eUnknown;
        return BERR_SUCCESS;
    }

    /* Get codec specifics */
    rc = BRAP_DSPCHN_GetStreamInformation(handle->rapChannel, &streamInfo);

    if ( BERR_SUCCESS == rc )
    {
        unsigned frameLength, bitrate;
        pStatus->sampleRate = NEXUS_AudioModule_P_Avc2SampleRate(streamInfo.eSamplingRate);

        if (handle->locked)
            handle->fakeFramesDecoded++;
        pStatus->framesDecoded = handle->fakeFramesDecoded; /* this is a work-around for info not available from RAP, obviously not accurate. */

        /* Convert codec back to nexus */
        pStatus->codec = NEXUS_AudioDecoder_P_MagnumToCodec(streamInfo.eType,handle->audioParams.sDspChParams.eAacXptFormat);

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
        case BRAP_DSPCHN_AudioType_eAac:
        case BRAP_DSPCHN_AudioType_eAacSbr:
            pStatus->codecStatus.aac.acmod = streamInfo.uStreamInfo.sAacInfo.eAcmod;
            pStatus->codecStatus.aac.profile = streamInfo.uStreamInfo.sAacInfo.eProfile;
            pStatus->codecStatus.aac.lfe = streamInfo.uStreamInfo.sAacInfo.bLfeOn;
            pStatus->codecStatus.aac.pseudoSurround = streamInfo.uStreamInfo.sAacInfo.bPseudoSurroundEnable;
            pStatus->codecStatus.aac.drc = streamInfo.uStreamInfo.sAacInfo.bDRCPresent;
            pStatus->codecStatus.aac.bitrate = streamInfo.uStreamInfo.sAacInfo.ui32BitRate;
            break;
        case BRAP_DSPCHN_AudioType_eAc3:
        case BRAP_DSPCHN_AudioType_eAc3Plus:
            pStatus->codec = (streamInfo.eType == BRAP_DSPCHN_AudioType_eAc3)?NEXUS_AudioCodec_eAc3:NEXUS_AudioCodec_eAc3Plus;
            pStatus->codecStatus.ac3.acmod = streamInfo.uStreamInfo.sAc3Info.eAcmod;
            pStatus->codecStatus.ac3.frameSizeCode = streamInfo.uStreamInfo.sAc3Info.ui32FrameSizeCode;
            pStatus->codecStatus.ac3.lfe = streamInfo.uStreamInfo.sAc3Info.bLfeOn;
            pStatus->codecStatus.ac3.copyright = streamInfo.uStreamInfo.sAc3Info.bCpyrhtPresent;
            break;
        default:
            /* No specifics for this codec */
            break;
        }
    }
    else
    {
        pStatus->codec = NEXUS_AudioCodec_eUnknown;
    }

    return NEXUS_SUCCESS;
}

static BERR_Code NEXUS_AudioDecoder_P_GetPtsCallback_isr(void *pContext, BAVC_PTSInfo *pPTSInfo)
{
    NEXUS_AudioDecoderHandle audioDecoder = (NEXUS_AudioDecoderHandle)pContext;
    BRAP_DSPCHN_PtsInfo ptsInfo;
    BERR_Code errCode=BERR_NOT_INITIALIZED;

    if(audioDecoder->running)
    errCode = BRAP_DSPCHN_GetCurrentPTS_isr(audioDecoder->rapChannel, &ptsInfo);
    if ( BERR_SUCCESS == errCode )
    {
        BDBG_MSG(("GetPts - returned 0x%x, type %d", ptsInfo.ui32RunningPts, ptsInfo.ePtsType));

    /* map data structures */
    pPTSInfo->ui32CurrentPTS = ptsInfo.ui32RunningPts;
    pPTSInfo->ePTSType = ptsInfo.ePtsType;
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
    return BRAP_DSPCHN_SetStcValidFlag_isr(audioDecoder->rapChannel);
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
    pts.ePTSType = pPtsInfo->ePtsType;

    BDBG_MSG(("audio[%d] First PTS %08x", pDecoder->index, pPtsInfo->ui32RunningPts));
    if ( pDecoder->programSettings.stcChannel && pDecoder->stc.connector)
    {
        errCode = NEXUS_StcChannel_RequestStc_isr(pDecoder->stc.connector, &pts);
        if ( errCode )
        {
            errCode=BERR_TRACE(errCode);
        }
    }
}

static void NEXUS_AudioDecoder_P_AudioPtsError_isr(void *pParam1, int param2, void *pData)
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
    pts.ePTSType = pPtsInfo->ePtsType;

    BDBG_MSG(("audio[%d] PTS error: PTS 0x%08x, type %d",
        pDecoder->index, pts.ui32CurrentPTS, pts.ePTSType));

    NEXUS_StcChannel_GetStc_isr(pDecoder->programSettings.stcChannel, &stc);

#if NEXUS_HAS_ASTM
    if (pDecoder->astm.settings.syncLimit > 0)
    {
        pts.ui32CurrentPTS = (uint32_t)((int32_t)stc - pPtsInfo->i32Pts2StcPhase);
    }
#endif /* NEXUS_HAS_ASTM */

    if (pDecoder->stc.connector)
    {
        errCode = NEXUS_StcChannel_PtsError_isr(pDecoder->stc.connector, &pts);
        if (errCode)
        {
            errCode=BERR_TRACE(errCode);
            /* keep going */
        }
    }

#if NEXUS_HAS_ASTM
   pDecoder->astm.status.pts = pts.ui32CurrentPTS;

    if (pDecoder->astm.settings.tsmFail_isr)
    {
        pDecoder->astm.settings.tsmFail_isr(pDecoder->astm.settings.callbackContext, 0);
    }
#endif /* NEXUS_HAS_ASTM */

    pDecoder->ptsErrorCount++;
}

#if NEXUS_HAS_ASTM
static void NEXUS_AudioDecoder_P_AudioTsmPass_isr(void *pParam1, int param2, void *pData)
{
    NEXUS_AudioDecoder *pDecoder = (NEXUS_AudioDecoder *)pParam1;
    BRAP_DSPCHN_PtsInfo *pPtsInfo = (BRAP_DSPCHN_PtsInfo *)pData;
    BAVC_PTSInfo pts;

    uint32_t stc;

    BSTD_UNUSED(param2);

    if ( NULL == pPtsInfo )
    {
        /* Drop erroneous callbacks */
        return;
    }

    pts.ui32CurrentPTS = pPtsInfo->ui32RunningPts;
    pts.ePTSType = pPtsInfo->ePtsType;

    BDBG_MSG(("audio[%d] TSM pass: PTS 0x%08x, type %d",
        pDecoder->index, pts.ui32CurrentPTS, pts.ePTSType));

        NEXUS_StcChannel_GetStc_isr(pDecoder->programSettings.stcChannel, &stc);

        if (pDecoder->astm.settings.syncLimit > 0)
        {
            pts.ui32CurrentPTS = (uint32_t)((int32_t)stc - pPtsInfo->i32Pts2StcPhase);
        }

        pDecoder->astm.status.pts = pts.ui32CurrentPTS;

        if (pDecoder->astm.settings.tsmPass_isr)
        {
            pDecoder->astm.settings.tsmPass_isr(pDecoder->astm.settings.callbackContext, 0);
        }
}

#endif

#if NEXUS_HAS_AUDIO_TSM_LOG_SUPPORT && BRAP_SUPPORT_TSM_LOG
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
#if B_HAS_TRC
        if (BTRC_MODULE_HANDLE(ChnChange_DecodeStopAudio)->stats[0].count) {
            BTRC_TRACE(ChnChange_Total_Audio, STOP);
            BKNI_SetEvent(pDecoder->channelChangeReportEvent);
        }
#endif
    }

    pDecoder->sampleRateInfo = *pSampleRateInfo;

    /* only call these if sample rate is useful */
    if (pDecoder->sampleRateInfo.eSamplingRate != BAVC_AudioSamplingRate_eUnknown)
    {
    #if 0 /* TODO -- Send to 1 or more display-registered hooks */
        /* link from base_rap to base_hdmi if channel's connected to HDMI */
        struct b_base_rap *rap = &b_root.rap[0];
        if ( (rap->actual.hdmi_mode == baudio_output_mode_compressed && rap_ch == &rap->chn[1]) ||
             (rap->actual.hdmi_mode == baudio_output_mode_pcm && rap_ch == &rap->chn[0]) ||
             (rap->actual.hdmi_mode == baudio_output_mode_multichannel && rap_ch == &rap->chn[0]) )
        {
            bsettop_p_hdmi_rap_sample_rate_change_isr(&b_root.video, rap_ch->sampling_rate_info.eSamplingRate);
        }
    #else
        if ( NULL != pDecoder->hdmiOutput )
        {
            /* Propagate sample rate changes to HDMI if connected */
            NEXUS_AudioOutput_P_SampleRateChange_isr(pDecoder->hdmiOutput, pSampleRateInfo->eSamplingRate);
        }
    #endif
        /* enable bdecode_status calls to RAP */
        pDecoder->channelStatusValid = true;
    }
    else {
        pDecoder->channelStatusValid = false;
    }

    BDBG_MSG(("audio[%d] Sampling Rate Info, eType=%d, eSamplingRate=%x", pDecoder->index,
        pDecoder->sampleRateInfo.eType, pDecoder->sampleRateInfo.eSamplingRate));

    BKNI_SetEvent(pDecoder->sampleRateEvent);
    BKNI_SetEvent(pDecoder->sourceChangedEvent);

    if (pDecoder->syncSettings.sampleRateCallback_isr) {
        (*pDecoder->syncSettings.sampleRateCallback_isr)(pDecoder->syncSettings.callbackContext, 0);
    }
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
        break;
    case BRAP_Interrupt_eDecUnlock:
        pDecoder->locked = false;
        pDecoder->fakeFramesDecoded = 0;
        BDBG_MSG(("decoder [%d]: unlock", pDecoder->index));
        break;
    case BRAP_Interrupt_eCdbItbUnderflow:
        BDBG_MSG(("decoder [%d]: underflow", pDecoder->index));
        break;
    case BRAP_Interrupt_eModeChange:
    case BRAP_Interrupt_eBitRateChange:
        BKNI_SetEvent(pDecoder->sourceChangedEvent);
        break;
    default:
        break;
    }
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
        if ( g_decoders[i].opened &&g_decoders[i].running )
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
        if ( g_decoders[i].opened &&g_decoders[i].running )
        {
            NEXUS_Rave_Enable_priv(g_decoders[i].raveContext);
        }
    }
    UNLOCK_TRANSPORT();
}

static void NEXUS_AudioDecoder_P_LayerChange(void *pParam)
{
    NEXUS_Error errCode;
    NEXUS_AudioDecoder *pDecoder = (NEXUS_AudioDecoder *)pParam;

    BDBG_MSG(("Layer Change decoder[%d]", pDecoder->index));

    if ( pDecoder->running )
    {
        errCode = BRAP_DEC_DisableForFlush(pDecoder->rapChannel);
        if (errCode)
        {
            BERR_TRACE(errCode);
        }
        if ( pDecoder->programSettings.pidChannel )
        {
            LOCK_TRANSPORT();
            NEXUS_Rave_Disable_priv(pDecoder->raveContext);
            NEXUS_Rave_Flush_priv(pDecoder->raveContext);
            UNLOCK_TRANSPORT();
        }
        errCode = BRAP_DEC_Flush(pDecoder->rapChannel);
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
    BDBG_ASSERT(type < NEXUS_AudioDecoderConnectorType_eMax);

#if !NEXUS_NUM_HDMI_OUTPUTS
    if ( type == NEXUS_AudioDecoderConnectorType_eMultichannel )
    {
        BDBG_ERR(("Multichannel not currently supported"));
        return NULL;
    }
#endif

    return &handle->connectors[type];
}

bool NEXUS_AudioDecoder_P_IsRunning(NEXUS_AudioDecoderHandle handle)
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioDecoder);
    BDBG_ASSERT(handle->opened);
    return handle->started;         /* Return true if the user has requested a start, regardless of state */
}

BRAP_ChannelHandle NEXUS_AudioDecoder_P_GetChannel(NEXUS_AudioDecoderHandle handle)
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioDecoder);
    BDBG_ASSERT(handle->opened);
    return handle->rapChannel;
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

#endif

void NEXUS_AudioDecoder_GetSyncSettings_priv( NEXUS_AudioDecoderHandle audioDecoder, NEXUS_AudioInputSyncSettings *pSyncSettings )
{
    NEXUS_ASSERT_MODULE();
    *pSyncSettings = audioDecoder->syncSettings;
}

NEXUS_Error NEXUS_AudioDecoder_SetSyncSettings_priv( NEXUS_AudioDecoderHandle audioDecoder, const NEXUS_AudioInputSyncSettings *pSyncSettings )
{
    unsigned syncPtsOffset;
    NEXUS_Error errCode;

    NEXUS_ASSERT_MODULE();
    audioDecoder->syncSettings = *pSyncSettings;

    /* PR:48453 bandrews - convert delay to transport compatible pts units */
    syncPtsOffset = audioDecoder->syncSettings.delay;
#if 0
    if (audioDecoder->isDss)
    {
        syncPtsOffset *= 600;
    }
#endif

    if (audioDecoder->rapChannel && audioDecoder->running)
    {
        errCode = BRAP_DSPCHN_SetPTSOffset(audioDecoder->rapChannel, (int32_t)syncPtsOffset +
                                           audioDecoder->syncSettings.delay + audioDecoder->settings.ptsOffset);
        if (errCode)
        {
            return BERR_TRACE(errCode);
        }
    }

    NEXUS_AudioDecoder_P_SetSyncMute(audioDecoder, audioDecoder->syncSettings.mute);

    return NEXUS_SUCCESS;
}

NEXUS_Error NEXUS_AudioDecoder_GetSyncStatus_isr( NEXUS_AudioDecoderHandle audioDecoder, NEXUS_AudioInputSyncStatus *pSyncStatus )
{
    pSyncStatus->started = audioDecoder->running;
    pSyncStatus->digital = true;
    pSyncStatus->dsolaEnabled = audioDecoder->audioParams.sDspChParams.ePBRate != BRAP_DSPCHN_PlayBackRate_e100;
    pSyncStatus->nonRealTime = false;
    BRAP_GetAudioDecoderDelay(&pSyncStatus->delay);
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
    NEXUS_ASSERT_MODULE();

    audioDecoder->astm.settings = *pAstmSettings;

    BRAP_DSPCHN_EnableASTM(audioDecoder->rapChannel, audioDecoder->astm.settings.enableAstm);
    BRAP_DSPCHN_SetTsmSLThreshold(audioDecoder->rapChannel, audioDecoder->astm.settings.syncLimit);

    NEXUS_AudioDecoder_ApplySettings_priv(audioDecoder);

    return 0;
}

NEXUS_Error NEXUS_AudioDecoder_GetAstmStatus_isr( NEXUS_AudioDecoderHandle audioDecoder, NEXUS_AudioDecoderAstmStatus * pAstmStatus )
{
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
        return BRAP_DSPCHN_Ac3CompMode_eLine; /* 7400 RAP doesn't have BRAP_DSPCHN_AC3CompMode_eOff */
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
        /* 7400 RAP doesn't have BRAP_DSPCHN_AC3CompMode_eOff */
        return NEXUS_AudioDecoderDolbyDrcMode_eLine;
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
    BRAP_DEC_ConfigParams *pRapDecSettings;

    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioDecoder);
    BDBG_ASSERT(codec < NEXUS_AudioCodec_eMax);
    BDBG_ASSERT(NULL != pSettings);

    pRapDecSettings = BKNI_Malloc(sizeof(*pRapDecSettings));
    if ( NULL == pRapDecSettings )
    {
        errCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        return;
    }

    pSettings->codec = codec;
    switch ( codec )
    {
    case NEXUS_AudioCodec_eAc3:
        errCode = BRAP_DEC_GetCurrentConfig(handle->rapChannel,
                                               BRAP_DSPCHN_AudioType_eAc3,
                                               #if BCHP_CHIP == 3563
                                               BRAP_MAX_PP_BRANCH_SUPPORTED,
                                               BRAP_MAX_PP_PER_BRANCH_SUPPORTED,
                                               #endif
                                               pRapDecSettings);
        if ( errCode )
        {
            errCode = BERR_TRACE(errCode);
            BKNI_Free(pRapDecSettings);
            return;
        }

        pSettings->codecSettings.ac3.stereoDownmixMode = NEXUS_AudioDecoder_P_DownmixMode2Nexus(pRapDecSettings->uConfigParams.sAc3ConfigParams.eStereoMode);
        pSettings->codecSettings.ac3.drcMode = NEXUS_AudioDecoder_P_DrcMode2Nexus(pRapDecSettings->uConfigParams.sAc3ConfigParams.eCompMode);
        pSettings->codecSettings.ac3.cut = pRapDecSettings->uConfigParams.sAc3ConfigParams.ui16DynRngScaleHi;
        pSettings->codecSettings.ac3.boost = pRapDecSettings->uConfigParams.sAc3ConfigParams.ui16DynRngScaleLo;
        pSettings->codecSettings.ac3.dialogNormalization = pRapDecSettings->uConfigParams.sAc3ConfigParams.bDialNormEnable;
        break;
    case NEXUS_AudioCodec_eAc3Plus:
        errCode = BRAP_DEC_GetCurrentConfig(handle->rapChannel,
                                               BRAP_DSPCHN_AudioType_eAc3Plus,
                                               #if BCHP_CHIP == 3563
                                               BRAP_MAX_PP_BRANCH_SUPPORTED,
                                               BRAP_MAX_PP_PER_BRANCH_SUPPORTED,
                                               #endif
                                               pRapDecSettings);
        if ( errCode )
        {
            errCode = BERR_TRACE(errCode);
            BKNI_Free(pRapDecSettings);
            return;
        }
        pSettings->codecSettings.ac3Plus.stereoDownmixMode = NEXUS_AudioDecoder_P_DownmixMode2Nexus(pRapDecSettings->uConfigParams.sAc3PlusConfigParams.eStereoMode);
        pSettings->codecSettings.ac3Plus.drcMode = NEXUS_AudioDecoder_P_DrcMode2Nexus(pRapDecSettings->uConfigParams.sAc3PlusConfigParams.eCompMode);
        pSettings->codecSettings.ac3Plus.cut = pRapDecSettings->uConfigParams.sAc3PlusConfigParams.ui16DynRngScaleHi;
        pSettings->codecSettings.ac3Plus.boost = pRapDecSettings->uConfigParams.sAc3PlusConfigParams.ui16DynRngScaleLo;
        pSettings->codecSettings.ac3Plus.dialogNormalization = pRapDecSettings->uConfigParams.sAc3PlusConfigParams.bDialNormEnable;
        break;
    case NEXUS_AudioCodec_eAac:
    case NEXUS_AudioCodec_eAacLoas:
        errCode = BRAP_DEC_GetCurrentConfig(handle->rapChannel,
                                               BRAP_DSPCHN_AudioType_eAac,
                                               #if BCHP_CHIP == 3563
                                               BRAP_MAX_PP_BRANCH_SUPPORTED,
                                               BRAP_MAX_PP_PER_BRANCH_SUPPORTED,
                                               #endif
                                               pRapDecSettings);
        if ( errCode )
        {
            errCode = BERR_TRACE(errCode);
            BKNI_Free(pRapDecSettings);
            return;
        }
        pSettings->codecSettings.aac.downmixMode = pRapDecSettings->uConfigParams.sAacConfigParams.eDownmixType;
        pSettings->codecSettings.aac.drcTargetLevel = (uint16_t)pRapDecSettings->uConfigParams.sAacConfigParams.ui32DrcTargetLevel;
        pSettings->codecSettings.aac.cut = (uint16_t)pRapDecSettings->uConfigParams.sAacConfigParams.ui32DrcGainControlCompress;
        pSettings->codecSettings.aac.boost = (uint16_t)pRapDecSettings->uConfigParams.sAacConfigParams.ui32DrcGainControlBoost;
        break;
    case NEXUS_AudioCodec_eAacPlusAdts:
    case NEXUS_AudioCodec_eAacPlusLoas:

        errCode = BRAP_DEC_GetCurrentConfig(handle->rapChannel,
                                               BRAP_DSPCHN_AudioType_eAacSbr,
                                               #if BCHP_CHIP == 3563
                                               BRAP_MAX_PP_BRANCH_SUPPORTED,
                                               BRAP_MAX_PP_PER_BRANCH_SUPPORTED,
                                               #endif
                                               pRapDecSettings);
        if ( errCode )
        {
            errCode = BERR_TRACE(errCode);
            BKNI_Free(pRapDecSettings);
            return;
        }
        pSettings->codecSettings.aacPlus.downmixMode =  pRapDecSettings->uConfigParams.sAacSbrConfigParams.eDownmixType;
        pSettings->codecSettings.aacPlus.drcTargetLevel = (uint16_t)pRapDecSettings->uConfigParams.sAacSbrConfigParams.ui32DrcTargetLevel;
        pSettings->codecSettings.aacPlus.cut = (uint16_t)pRapDecSettings->uConfigParams.sAacSbrConfigParams.ui32DrcGainControlCompress;
        pSettings->codecSettings.aacPlus.boost = (uint16_t)pRapDecSettings->uConfigParams.sAacSbrConfigParams.ui32DrcGainControlBoost;
        break;
    default:
        break;
    }

    BKNI_Free(pRapDecSettings);
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
    BRAP_DEC_ConfigParams *pRapDecSettings;

    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioDecoder);
    BDBG_ASSERT(NULL != pSettings);

    pRapDecSettings = BKNI_Malloc(sizeof(*pRapDecSettings));
    if ( NULL == pRapDecSettings )
    {
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }

    switch ( pSettings->codec )
    {
    case NEXUS_AudioCodec_eAc3:
        errCode = BRAP_DEC_GetCurrentConfig(handle->rapChannel,
                                               BRAP_DSPCHN_AudioType_eAc3,
                                               #if BCHP_CHIP == 3563
                                               BRAP_MAX_PP_BRANCH_SUPPORTED,
                                               BRAP_MAX_PP_PER_BRANCH_SUPPORTED,
                                               #endif
                                               pRapDecSettings);
        if ( errCode )
        {
            BKNI_Free(pRapDecSettings);
            return BERR_TRACE(errCode);
        }
        pRapDecSettings->uConfigParams.sAc3ConfigParams.eStereoMode = NEXUS_AudioDecoder_P_DownmixMode2Magnum(pSettings->codecSettings.ac3.stereoDownmixMode);
        pRapDecSettings->uConfigParams.sAc3ConfigParams.eCompMode = NEXUS_AudioDecoder_P_DrcMode2Magnum(pSettings->codecSettings.ac3.drcMode);
        pRapDecSettings->uConfigParams.sAc3ConfigParams.ui16DynRngScaleHi = pSettings->codecSettings.ac3.cut;
        pRapDecSettings->uConfigParams.sAc3ConfigParams.ui16DynRngScaleLo = pSettings->codecSettings.ac3.boost;
        pRapDecSettings->uConfigParams.sAc3ConfigParams.bDialNormEnable = pSettings->codecSettings.ac3.dialogNormalization;
        BDBG_MSG(("sAc3ConfigParams.eStereoMode = %d",pRapDecSettings->uConfigParams.sAc3ConfigParams.eStereoMode));
        BDBG_MSG(("sAc3ConfigParams.eCompMode = %d",pRapDecSettings->uConfigParams.sAc3ConfigParams.eCompMode));
        BDBG_MSG(("sAc3ConfigParams.ui16DynRngScaleHi = %us",pRapDecSettings->uConfigParams.sAc3ConfigParams.ui16DynRngScaleHi));
        BDBG_MSG(("sAc3ConfigParams.ui16DynRngScaleLo = %us",pRapDecSettings->uConfigParams.sAc3ConfigParams.ui16DynRngScaleLo));
        BDBG_MSG(("sAc3ConfigParams.bDialNormEnable = %d",pRapDecSettings->uConfigParams.sAc3ConfigParams.bDialNormEnable));
        break;
    case NEXUS_AudioCodec_eAc3Plus:
        errCode = BRAP_DEC_GetCurrentConfig(handle->rapChannel,
                                               BRAP_DSPCHN_AudioType_eAc3Plus,
                                               #if BCHP_CHIP == 3563
                                               BRAP_MAX_PP_BRANCH_SUPPORTED,
                                               BRAP_MAX_PP_PER_BRANCH_SUPPORTED,
                                               #endif
                                               pRapDecSettings);
        if ( errCode )
        {
            BKNI_Free(pRapDecSettings);
            return BERR_TRACE(errCode);
        }
        pRapDecSettings->uConfigParams.sAc3PlusConfigParams.eStereoMode = NEXUS_AudioDecoder_P_DownmixMode2Magnum(pSettings->codecSettings.ac3Plus.stereoDownmixMode);
        pRapDecSettings->uConfigParams.sAc3PlusConfigParams.eCompMode = NEXUS_AudioDecoder_P_DrcMode2Magnum(pSettings->codecSettings.ac3Plus.drcMode);
        pRapDecSettings->uConfigParams.sAc3PlusConfigParams.ui16DynRngScaleHi = pSettings->codecSettings.ac3Plus.cut;
        pRapDecSettings->uConfigParams.sAc3PlusConfigParams.ui16DynRngScaleLo = pSettings->codecSettings.ac3Plus.boost;
        pRapDecSettings->uConfigParams.sAc3PlusConfigParams.bDialNormEnable =  pSettings->codecSettings.ac3Plus.dialogNormalization;
        BDBG_MSG(("sAc3PlusConfigParams.eCompMode = %d",pRapDecSettings->uConfigParams.sAc3PlusConfigParams.eCompMode));
        BDBG_MSG(("sAc3PlusConfigParams.ui16DynRngScaleHi = %us",pRapDecSettings->uConfigParams.sAc3PlusConfigParams.ui16DynRngScaleHi));
        BDBG_MSG(("sAc3PlusConfigParams.ui16DynRngScaleLo = %us",pRapDecSettings->uConfigParams.sAc3PlusConfigParams.ui16DynRngScaleLo));
        BDBG_MSG(("sAc3PlusConfigParams.bDialNormEnable = %d",pRapDecSettings->uConfigParams.sAc3PlusConfigParams.bDialNormEnable));
        break;
    case NEXUS_AudioCodec_eAac:
    case NEXUS_AudioCodec_eAacLoas:
        errCode = BRAP_DEC_GetCurrentConfig(handle->rapChannel,
                                               BRAP_DSPCHN_AudioType_eAac,
                                               #if BCHP_CHIP == 3563
                                               BRAP_MAX_PP_BRANCH_SUPPORTED,
                                               BRAP_MAX_PP_PER_BRANCH_SUPPORTED,
                                               #endif
                                               pRapDecSettings);
        if ( errCode )
        {
            BKNI_Free(pRapDecSettings);
            return BERR_TRACE(errCode);
        }
        pRapDecSettings->uConfigParams.sAacConfigParams.eDownmixType = pSettings->codecSettings.aac.downmixMode;
        pRapDecSettings->uConfigParams.sAacConfigParams.ui32DrcTargetLevel = (uint32_t)pSettings->codecSettings.aac.drcTargetLevel;
        pRapDecSettings->uConfigParams.sAacConfigParams.ui32DrcGainControlCompress = (uint32_t)pSettings->codecSettings.aac.cut;
        pRapDecSettings->uConfigParams.sAacConfigParams.ui32DrcGainControlBoost = (uint32_t)pSettings->codecSettings.aac.boost;
        break;
    case NEXUS_AudioCodec_eAacPlusAdts:
    case NEXUS_AudioCodec_eAacPlusLoas:

        errCode = BRAP_DEC_GetCurrentConfig(handle->rapChannel,
                                               BRAP_DSPCHN_AudioType_eAacSbr,
                                               #if BCHP_CHIP == 3563
                                               BRAP_MAX_PP_BRANCH_SUPPORTED,
                                               BRAP_MAX_PP_PER_BRANCH_SUPPORTED,
                                               #endif
                                               pRapDecSettings);
        if ( errCode )
        {
            BKNI_Free(pRapDecSettings);
            return BERR_TRACE(errCode);
        }
        pRapDecSettings->uConfigParams.sAacSbrConfigParams.eDownmixType = pSettings->codecSettings.aacPlus.downmixMode;
        pRapDecSettings->uConfigParams.sAacSbrConfigParams.ui32DrcTargetLevel = (uint32_t)pSettings->codecSettings.aacPlus.drcTargetLevel;
        pRapDecSettings->uConfigParams.sAacSbrConfigParams.ui32DrcGainControlCompress = (uint32_t)pSettings->codecSettings.aacPlus.cut;
        pRapDecSettings->uConfigParams.sAacSbrConfigParams.ui32DrcGainControlBoost = (uint32_t)pSettings->codecSettings.aacPlus.boost;
        break;
    default:
        BDBG_WRN(("Codec settings not available for codec %d", pSettings->codec));
        BKNI_Free(pRapDecSettings);
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    errCode = BRAP_DEC_SetConfig(handle->rapChannel, pRapDecSettings);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
    }
    BKNI_Free(pRapDecSettings);
    return errCode;
}

static NEXUS_Error NEXUS_AudioDecoder_P_Disconnect(void *pHandle, struct NEXUS_AudioInputObject *pInput)
{
    NEXUS_AudioDecoderHandle decoder = (NEXUS_AudioDecoderHandle) pHandle;
    NEXUS_AudioOutputList *pOutputList;
    bool multichannel;
    int j;
    NEXUS_Error errCode;

    BDBG_OBJECT_ASSERT(decoder, NEXUS_AudioDecoder);


    BDBG_MSG(("%s(): Decoder %i disconnects input %p",__FUNCTION__, decoder->index, (void *)pInput));
    /* Get output list */
    switch (pInput->format)
    {
        case NEXUS_AudioInputFormat_ePcm5_1:
        case NEXUS_AudioInputFormat_ePcm7_1:
            BDBG_MSG(("%s(): NEXUS_AudioDecoderConnectorType_eMultichannel",__FUNCTION__));
            pOutputList = &decoder->outputLists[NEXUS_AudioDecoderConnectorType_eMultichannel];
            multichannel = true;
            break;
        case NEXUS_AudioInputFormat_eCompressed:
            BDBG_MSG(("%s(): NEXUS_AudioDecoderConnectorType_eCompressed",__FUNCTION__));
            pOutputList = &decoder->outputLists[NEXUS_AudioDecoderConnectorType_eCompressed];
            multichannel = false;
            break;
        case NEXUS_AudioInputFormat_eNone:
        case NEXUS_AudioInputFormat_ePcmStereo:
        default:
            BDBG_MSG(("%s(): NEXUS_AudioDecoderConnectorType_eStereo",__FUNCTION__));
            pOutputList = &decoder->outputLists[NEXUS_AudioDecoderConnectorType_eStereo];
            multichannel = false;
            break;
    }
    for ( j = 0; j < NEXUS_AUDIO_MAX_OUTPUTS; j++ )
    {
        NEXUS_AudioOutputHandle output = pOutputList->outputs[j];
        if ( output )
        {
            if (!NEXUS_AudioOutput_P_IsDacSlave(output))
            {
                if (multichannel)
                {
                    BDBG_MSG(("Removing multichannel MAI from decoder %d rapChannel = %p",
                              decoder->index, (void *)decoder->rapChannel));
                    errCode = BRAP_RemoveOutputPort(decoder->rapChannel, BRAP_OutputPort_eMai);
                    if ( errCode ) {    /* Report error and continue */
                        errCode = BERR_TRACE(errCode);
                    }
                    errCode = NEXUS_AudioOutput_P_ZeroDelay(output);
                    if ( errCode )
                    {
                        /* Report error and continue */
                        errCode = BERR_TRACE(errCode);
                    }
                    /* Remove I2S0 */
                    BDBG_MSG(("Removing I2S0 Output Port from ch[0]"));
                    errCode = BRAP_RemoveOutputPort(decoder->rapChannel, BRAP_OutputPort_eI2s0);
                    if ( errCode ) {
                        errCode = BERR_TRACE(errCode);
                    }

#if NEXUS_NUM_I2S_OUTPUTS > 1
                    /* Remove I2S1 */
                    BDBG_MSG(("Removing I2S1 Output Port from ch[0]"));
                    errCode = BRAP_RemoveOutputPort(decoder->rapChannel, BRAP_OutputPort_eI2s1);
                    if ( errCode ) {
                        errCode = BERR_TRACE(errCode);
                    }
#endif

#if NEXUS_NUM_I2S_OUTPUTS > 2
                    /* Remove I2S2 */
                    BDBG_MSG(("Removing I2S2 Output Port from ch[0]"));
                    errCode = BRAP_RemoveOutputPort(decoder->rapChannel, BRAP_OutputPort_eI2s2);
                    if ( errCode ) {
                        errCode = BERR_TRACE(errCode);
                    }
#endif
                }
                else
                {
                    BDBG_MSG(("Removing output port %d from decoder %d rapChannel = %p",
                              output->port, decoder->index, (void *)decoder->rapChannel));
                    errCode = BRAP_RemoveOutputPort(decoder->rapChannel, output->port);
                    if ( errCode )
                    {
                        /* Report error and continue */
                        errCode = BERR_TRACE(errCode);
                    }
                    errCode = NEXUS_AudioOutput_P_ZeroDelay(output);
                    if ( errCode )
                    {
                        /* Report error and continue */
                        errCode = BERR_TRACE(errCode);
                    }
                }
            }
            /* Clear this output port */
            pOutputList->outputs[j] = NULL;
            if ( output->objectType == NEXUS_AudioOutputType_eHdmi )
            {
                decoder->hdmiOutput = NULL;
            }
        }
    }

    return BERR_SUCCESS;
}

NEXUS_Error NEXUS_AudioDecoder_P_SetSyncMute(NEXUS_AudioDecoderHandle decoder, bool muted)
{
    NEXUS_AudioOutputHandle output;
    NEXUS_Error rc;
    int i;

    BDBG_MSG(("SetSyncMute(%d)", muted));

    if ( decoder->syncMute != muted )
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
                    rc=NEXUS_AudioOutput_P_SetSyncMute(output, muted);
                    if ( rc )
                    {
                        return BERR_TRACE(rc);
                    }
                }
            }
        }

        decoder->syncMute = muted;

        /* Decode or simul */
        if ( decoder->audioParams.sDspChParams.eDecodeMode != BRAP_DSPCHN_DecodeMode_ePassThru )
        {
            /* This will set the decoder mute into the mixer taking into account user volume settings */
            NEXUS_AudioDecoder_ApplySettings_priv(decoder);
        }
    }

    return NEXUS_SUCCESS;
}

NEXUS_Error NEXUS_AudioDecoder_P_SetTrickMute(NEXUS_AudioDecoderHandle decoder, bool muted)
{
    NEXUS_AudioOutputHandle output;
    NEXUS_Error rc;
    int i;

    BDBG_MSG(("SetTrickMute(%d)", muted));

    if (muted)
    {
        decoder->channelStatusValid = false;
    }

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

static void NEXUS_AudioDecoder_P_ChannelChangeReport(void * context)
{
#if B_HAS_TRC
#define REPORT_FREQUENCY 10
#define CPU_CLOCK_RATE_MHZ 405 /* MHz */

    const unsigned chn = 0;
    unsigned i = 0;

    if (BTRC_MODULE_HANDLE(ChnChange_Total_Audio)->stats[0].count % REPORT_FREQUENCY == 0) {
        if (NEXUS_GetEnv("force_vsync")) {BKNI_Printf("Channel change measurements with force_vsync=y\n");}
        if (NEXUS_GetEnv("sync_disabled")) {BKNI_Printf("Channel change measurements with sync_disabled=y\n");}
        BKNI_Printf("%-32s %6s %12s %12s %12s\n", "Name", "Cnt", "Avg(us)", "Max(us)", "Min(us)");

        for (i=0; i < AUDIO_BTRC_N_MODULES; i++) {
            const struct BTRC_P_Stats *stats = &audio_btrc_modules[i]->stats[chn];
            const unsigned avg = (unsigned)(((((uint64_t)stats->total_hi.perf.data[3])<<((sizeof(unsigned))*8)) + stats->total.perf.data[3])/stats->count / CPU_CLOCK_RATE_MHZ);
            const unsigned max = stats->max.perf.data[3] / CPU_CLOCK_RATE_MHZ;
            const unsigned min = stats->min.perf.data[3] / CPU_CLOCK_RATE_MHZ;

            if (stats->count) {
                BKNI_Printf("%-32s %6u %12u %12u %12u\n", audio_btrc_modules[i]->name, stats->count, avg, max, min);
            }
        }
    }
#endif
    BSTD_UNUSED(context);
    return;
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
        return false;
    default:
        return true;
    }
}

static void NEXUS_AudioDecoder_P_SetOutputMode(NEXUS_AudioDecoderHandle handle)
{
    BRAP_DEC_ConfigParams *pConfig;
    bool multichannel = false;
    bool bOutputLfeOn;

    BERR_Code errCode;

    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioDecoder);

    /* Remove all post-processing */
    pConfig = BKNI_Malloc(sizeof(*pConfig)); /* must malloc this large structure to avoid possible stack overflow */

    (void)BRAP_DEC_GetCurrentConfig(handle->rapChannel,
                                       handle->audioParams.sDspChParams.eType,
                                       pConfig);

    /* Check for at least one multichannel output */
    if ( handle->outputLists[NEXUS_AudioDecoderConnectorType_eMultichannel].outputs[0] != NULL )
    {
        /* Make sure the codec supports multichannel */
        if ( NEXUS_AudioDecoder_P_CodecSupportsMultichannel(handle->programSettings.codec) )
        {
            /* We can configure the decoder in multichannel mode */
            multichannel = true;
        }
    }

    if ( handle->settings.outputMode == NEXUS_AudioDecoderOutputMode_eAuto )
    {
        handle->audioParams.sDspChParams.bEnableStereoDownmixPath = false;  /* Not required except for multichannel */
        if ( multichannel )
        {
            if ( NULL != handle->outputLists[NEXUS_AudioDecoderConnectorType_eStereo].outputs[0]  )
            {
                handle->audioParams.sDspChParams.bEnableStereoDownmixPath = true;  /* Not required except for multichannel */
            }
            switch ( handle->connectors[NEXUS_AudioDecoderConnectorType_eMultichannel].format )
            {
            case NEXUS_AudioInputFormat_ePcm5_1:
                pConfig->eOutputMode = BRAP_OutputMode_e3_2;    /* 5.1 */
                break;
            default:
                pConfig->eOutputMode = BRAP_OutputMode_e2_0;    /* 2.0 */
                break;
            }
        }
        else
        {
            pConfig->eOutputMode = BRAP_OutputMode_e2_0;        /* default */
        }
        if ((handle->mono_to_all) && (pConfig->eOutputMode != BRAP_OutputMode_e3_2))
        {
            pConfig->eOutputMode = BRAP_OutputMode_e1_0;        /* downmix monomix */
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
                pConfig->eOutputMode = BRAP_OutputMode_e2_0;
                break;
            case NEXUS_AudioDecoderOutputMode_e1_0:
                pConfig->eOutputMode = BRAP_OutputMode_e1_0;
                break;
            case NEXUS_AudioDecoderOutputMode_e1_1:
                pConfig->eOutputMode = BRAP_OutputMode_e1_1;
                break;
            case NEXUS_AudioDecoderOutputMode_e3_0:
                pConfig->eOutputMode = BRAP_OutputMode_e3_0;
                break;
            case NEXUS_AudioDecoderOutputMode_e3_1:
                pConfig->eOutputMode = BRAP_OutputMode_e3_1;
                break;
            case NEXUS_AudioDecoderOutputMode_e2_2:
                pConfig->eOutputMode = BRAP_OutputMode_e2_2;
                break;
            case NEXUS_AudioDecoderOutputMode_e3_2:
                pConfig->eOutputMode = BRAP_OutputMode_e3_2;
                break;
        }

        handle->audioParams.sDspChParams.bEnableStereoDownmixPath = false; /* Not required except for multichannel */

        if ( (pConfig->eOutputMode > BRAP_OutputMode_e2_0 ||
             handle->settings.outputLfeMode == NEXUS_AudioDecoderOutputLfeMode_eOn) &&
             NULL != handle->outputLists[NEXUS_AudioDecoderConnectorType_eStereo].outputs[0] )
        {
            if ( NEXUS_AudioDecoder_P_CodecSupportsMultichannel(handle->programSettings.codec) )
            {
                handle->audioParams.sDspChParams.bEnableStereoDownmixPath = true;  /* Not required except for multichannel */
            }
            else
            {
                BDBG_WRN(("Forcing output mode to 2.0 for non-multichannel codec"));
                pConfig->eOutputMode = BRAP_OutputMode_e2_0;
            }
        }
    }

    pConfig->bMonoToAll = ((handle->mono_to_all) && (pConfig->eOutputMode == BRAP_OutputMode_e1_0));


    if ( handle->settings.outputLfeMode == NEXUS_AudioDecoderOutputLfeMode_eAuto )
    {
        if ( multichannel || pConfig->eOutputMode > BRAP_OutputMode_e2_0 )
        {
            if ( NEXUS_AudioDecoder_P_CodecSupportsMultichannel(handle->programSettings.codec) )
            {
                bOutputLfeOn = true;
            }
            else
            {
                bOutputLfeOn = false;
            }
        }
        else
        {
            bOutputLfeOn = false;
        }
    }
    else
    {
        bOutputLfeOn = false;

        if ( handle->settings.outputLfeMode == NEXUS_AudioDecoderOutputLfeMode_eOn )
        {
            if ( NEXUS_AudioDecoder_P_CodecSupportsMultichannel(handle->programSettings.codec)  )
            {
                bOutputLfeOn = true;
            }
            else
            {
                BDBG_WRN(("Disabling LFE output for non-multichannel codec"));
            }
        }
    }
    switch (handle->audioParams.sDspChParams.eType)
    {   /* turn on LFE (if supported) */
        case BRAP_DSPCHN_AudioType_eAac:
            pConfig->uConfigParams.sAacConfigParams.ui32bOutputLfeOn = bOutputLfeOn;
            break;
        case BRAP_DSPCHN_AudioType_eAacSbr:
            pConfig->uConfigParams.sAacSbrConfigParams.ui32bOutputLfeOn = bOutputLfeOn;
            break;
        case BRAP_DSPCHN_AudioType_eAc3:
            pConfig->uConfigParams.sAc3ConfigParams.bOutputLfeOn = bOutputLfeOn;
            break;
        case BRAP_DSPCHN_AudioType_eAc3Plus:
            pConfig->uConfigParams.sAc3PlusConfigParams.bOutputLfeOn = bOutputLfeOn;
            break;
        case BRAP_DSPCHN_AudioType_eDts:
            pConfig->uConfigParams.sDtsConfigParams.bOutputLfeOn = bOutputLfeOn;
            break;
        case BRAP_DSPCHN_AudioType_eLpcmBd:
            pConfig->uConfigParams.sBdlpcmConfigParams.bOutputLfeOn = bOutputLfeOn;
            break;
        case BRAP_DSPCHN_AudioType_eLpcmHdDvd:
            pConfig->uConfigParams.sLpcmHdDvdConfigParams.bOutputLfeOn = bOutputLfeOn;
            break;
        case BRAP_DSPCHN_AudioType_eDtshd:
            pConfig->uConfigParams.sDtshdConfigParams.bOutputLfeOn = bOutputLfeOn;
            break;
        case BRAP_DSPCHN_AudioType_eLpcmDvd:
            pConfig->uConfigParams.sLpcmDvdConfigParams.bOutputLfeOn = bOutputLfeOn;
            break;
            break;
        case BRAP_DSPCHN_AudioType_eAc3Lossless:
            pConfig->uConfigParams.sAc3LosslessConfigParams.bOutputLfeOn = bOutputLfeOn;
            break;
        case BRAP_DSPCHN_AudioType_eMlp:
            pConfig->uConfigParams.sMlpConfigParams.bOutputLfeOn = bOutputLfeOn;
            break;
        case BRAP_DSPCHN_AudioType_eWmaPro:
            pConfig->uConfigParams.sWmaProConfigParams.bLfeOn = bOutputLfeOn;
            break;
        case BRAP_DSPCHN_AudioType_eMpeg:
        case BRAP_DSPCHN_AudioType_eWmaStd:
        default:
            break;
    }

    BDBG_MSG(("Audio decoder output mode %d LFE %d", pConfig->eOutputMode, bOutputLfeOn));

    errCode = BRAP_DEC_SetConfig(handle->rapChannel, pConfig);
    if( errCode )
    {
        errCode = BERR_TRACE(errCode);
    }

    BKNI_Free(pConfig);
}


static NEXUS_AudioCodec NEXUS_AudioDecoder_P_MagnumToCodec(BRAP_DSPCHN_AudioType codec,BRAP_DSPCHN_AacXptFormat xptfmt)
{
    switch ( codec )
    {
    case BRAP_DSPCHN_AudioType_eMpeg:
        return NEXUS_AudioCodec_eMpeg;  /* In status, you must verify another field to determine MP3/MP[12] */
    case BRAP_DSPCHN_AudioType_eAac:
        if (xptfmt == BRAP_DSPCHN_AacXptFormat_eAdts)
        {
            return NEXUS_AudioCodec_eAacAdts;
        }
        else
        {
            return NEXUS_AudioCodec_eAacLoas;
        }
    case BRAP_DSPCHN_AudioType_eAacSbr:
        if (xptfmt == BRAP_DSPCHN_AacXptFormat_eAdts)
        {
            return NEXUS_AudioCodec_eAacPlusAdts;
        }
        else
        {
            return NEXUS_AudioCodec_eAacPlusLoas;
        }
    case BRAP_DSPCHN_AudioType_eAc3:
        return NEXUS_AudioCodec_eAc3;
    case BRAP_DSPCHN_AudioType_eAc3Plus:
        return NEXUS_AudioCodec_eAc3Plus;
    case BRAP_DSPCHN_AudioType_eDts:
        return NEXUS_AudioCodec_eDts;
    case BRAP_DSPCHN_AudioType_eLpcmDvd:
        return NEXUS_AudioCodec_eLpcmDvd;
    case BRAP_DSPCHN_AudioType_eLpcmHdDvd:
        return NEXUS_AudioCodec_eLpcmHdDvd;
    case BRAP_DSPCHN_AudioType_eLpcmBd:
        return NEXUS_AudioCodec_eLpcmBluRay;
    case BRAP_DSPCHN_AudioType_eDtshd:
        return NEXUS_AudioCodec_eDtsHd;
    case BRAP_DSPCHN_AudioType_eWmaStd:
        return NEXUS_AudioCodec_eWmaStd;
    case BRAP_DSPCHN_AudioType_eWmaPro:
        return NEXUS_AudioCodec_eWmaPro;
    case BRAP_DSPCHN_AudioType_ePcmWav:
        return NEXUS_AudioCodec_ePcmWav;
    default:
        BDBG_WRN(("Unknown magnum audio codec %d", codec));
        return NEXUS_AudioCodec_eUnknown;
    }
}

static NEXUS_Error NEXUS_AudioDecoder_P_SetMixerInputScaling(NEXUS_AudioDecoderHandle handle)
{
    unsigned int uiScaleValue,vol;
    NEXUS_AudioOutputList *pOutputList;
    int i,j;
    NEXUS_Error errCode = NEXUS_SUCCESS;

    if ( handle->settings.muted || handle->trickMute  || handle->syncMute ) {
        uiScaleValue = 9000;
    }
    else {
        vol = ((handle->settings.volumeMatrix[0][0] +
               handle->settings.volumeMatrix[0][1] +
               handle->settings.volumeMatrix[1][0] +
               handle->settings.volumeMatrix[1][1])/2);
        uiScaleValue = NEXUS_AudioOutput_P_Vol2dB(vol);
    }

    for ( i = 0; i < NEXUS_AudioDecoderConnectorType_eMax; i++ )
    {
        if (i == NEXUS_AudioDecoderConnectorType_eCompressed)
        {
            continue;
        }
        pOutputList = &handle->outputLists[i];
        for ( j = 0; j < NEXUS_AUDIO_MAX_OUTPUTS; j++ )
        {
            NEXUS_AudioOutputHandle output = pOutputList->outputs[j];
            if ( output )
            {
                if (!NEXUS_AudioOutput_P_IsDacSlave(output))
                {
                    BDBG_MSG(("BRAP_MIXER_SetInputScaling() output port %d from handle %d scale value %u",
                              output->port, handle->index, uiScaleValue));
                    errCode = BRAP_MIXER_SetInputScaling(handle->rapChannel, output->port, uiScaleValue);
                    if ( errCode )
                    {
                        /* Report error and continue */
                        errCode = BERR_TRACE(errCode);
                    }
                }
            }
        }
    }

    return errCode;
}

void NEXUS_AudioDecoder_P_SetTsm(NEXUS_AudioDecoderHandle handle)
{
    bool tsm=false;

    if ( handle->tsmPermitted )
    {
        if ( handle->trickState.tsmEnabled )
        {
#if NEXUS_HAS_ASTM
            if ( handle->astm.permitted && handle->astm.settings.enableAstm )
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

    BDBG_MSG(("%s TSM",tsm?"Enabling":"Disabling"));
    BRAP_DSPCHN_EnableTSM(handle->rapChannel, tsm);
    handle->tsmEnabled = tsm;

#if NEXUS_HAS_ASTM
    /* Only allow ASTM if we have TSM enabled for all non-ASTM controls and we have a TS source */
    if ( handle->tsmPermitted && handle->trickState.tsmEnabled &&
         handle->audioParams.sDspChParams.eStreamType == BAVC_StreamType_eTsMpeg )
    {
        if (handle->astm.permitted)
        {
            BDBG_MSG(("%s ASTM mode for audio channel %p",handle->astm.settings.enableAstm?"Enabling":"Disabling", (void *)handle));
            BRAP_DSPCHN_EnableASTM(handle->rapChannel, handle->astm.settings.enableAstm);
        }
    }
    else
    {
        BDBG_MSG(("Disabling ASTM mode for audio channel %p", (void *)handle));
        BRAP_DSPCHN_EnableASTM(handle->rapChannel, false);
    }
#endif
}

#if (BCHP_CHIP==7400)
bool NEXUS_AudioDecoder_P_IsPassthru(
    NEXUS_AudioDecoderHandle handle
    )
{
    int i;
    bool passthru=false;
    NEXUS_AudioOutputList outputLists[NEXUS_AudioDecoderConnectorType_eMax];

    BDBG_ASSERT(NULL != handle);

    /* Get output lists */
    for ( i = 0; i < NEXUS_AudioDecoderConnectorType_eMax; i++ )
    {
        NEXUS_AudioInput_P_GetOutputs(&handle->connectors[i], &outputLists[i], false);
    }

    if ( outputLists[NEXUS_AudioDecoderConnectorType_eCompressed].outputs[0] != NULL &&
         outputLists[NEXUS_AudioDecoderConnectorType_eStereo].outputs[0] == NULL &&
         outputLists[NEXUS_AudioDecoderConnectorType_eMultichannel].outputs[0] == NULL )
    {
        /* Compressed output, but no stereo or multichannel.  This is a passthrough channel. */
        passthru=true;
    }
    return passthru;
}
#endif

NEXUS_Error NEXUS_AudioModule_Standby_priv(bool enabled, const NEXUS_StandbySettings *pSettings)
{
    /* Not supported on this platform, but don't complain if called. */
    BERR_Code rc = NEXUS_SUCCESS;
    BSTD_UNUSED(enabled);
    BSTD_UNUSED(pSettings);
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

/******************************************************************************
The array to represent the value of volume in hex corresponding to the value
in DB from rap_v3/nexus_audio_output.c.
The table of HEX values is from

    HEX = (2^23) * 10^(DB/20)

Note: 23 is the number of bits in the volume control field.

The volume can range from 0-1. 0 in hex corresponds to the 139 DB from the above
Formula. The array is truncated to 90 values, corresponding to the 90 dB
attenuation range of NEXUS_AudioDecoder_P_SetMixerInputScaling().
******************************************************************************/
static const int32_t g_db2linear[] =
{
    0x800000,   0x721482,   0x65AC8C,   0x5A9DF7,   0x50C335,
    0x47FACC,   0x4026E7,   0x392CED,   0x32F52C,   0x2D6A86,
    0x287A26,   0x241346,   0x2026F3,   0x1CA7D7,   0x198A13,
    0x16C310,   0x144960,   0x12149A,   0x101D3F,   0xE5CA1,
    0xCCCCC,    0xB6873,    0xA2ADA,    0x90FCB,    0x81385,
    0x732AE,    0x66A4A,    0x5B7B1,    0x51884,    0x48AA7,
    0x40C37,    0x39B87,    0x33718,    0x2DD95,    0x28DCE,
    0x246B4,    0x20756,    0x1CEDC,    0x19C86,    0x16FA9,
    0x147AE,    0x1240B,    0x10449,    0xE7FA,     0xCEC0,
    0xB844,     0xA43A,     0x925E,     0x8273,     0x7443,
    0x679F,     0x5C5A,     0x524F,     0x495B,     0x4161,
    0x3A45,     0x33EF,     0x2E49,     0x2940,     0x24C4,
    0x20C4,     0x1D34,     0x1A07,     0x1732,     0x14AC,
    0x126D,     0x106C,     0xEA3,      0xD0B,      0xBA0,
    0xA5C,      0x93C,      0x83B,      0x755,      0x689,
    0x5D3,      0x531,      0x4A0,      0x420,      0x3AD,
    0x346,      0x2EB,      0x29A,      0x251,      0x211,
    0x1D7,      0x1A4,      0x176,      0x14D,      0x129,
    0x109
};
static int32_t NEXUS_AudioOutput_P_Vol2dB(int32_t volume)
{   /* return attenuation in hundredths DB. (1 dB resolution) */
    int32_t atten = 90*100;
    int h=89,l=0,m=44;
    if (volume==g_db2linear[0]) {
        return 0;
    }
    else if (volume>g_db2linear[90]) {
        while (h>=l) {
            if (volume<=g_db2linear[m] && volume>g_db2linear[m+1]) {
                atten = m*100;
                break;
            }
            else if (volume>g_db2linear[m]) { /* below */
                h=(m-1);
                m=((h-l)/2)+l;
            }
            else  { /* above */
                l=(m+1);
                m=((h-l)/2)+l;
            }
        }
    }
    return atten;
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
