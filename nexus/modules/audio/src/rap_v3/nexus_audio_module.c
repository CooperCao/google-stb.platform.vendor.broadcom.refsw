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
 * Module Description:
 *
 **************************************************************************/
#include "nexus_audio_module.h"

BDBG_MODULE(nexus_audio_module);

/* global module handle & data */
NEXUS_ModuleHandle g_NEXUS_audioModule;
NEXUS_AudioModuleData g_NEXUS_audioModuleData;
BRAP_ProcessingStageSettings g_NEXUS_StageSettings;

/* required for the debug output */
extern NEXUS_AudioDecoder g_decoders[NEXUS_NUM_AUDIO_DECODERS];

void NEXUS_AudioModule_Print(void)
{
#if BDBG_DEBUG_BUILD
    int i;
    BDBG_LOG(("Audio:"));

    BDBG_LOG((" handles: xpt:%p hdmi in:%p hdmi out:%p fe:%p rap:%p", (void *)g_NEXUS_audioModuleData.transport, (void *)g_NEXUS_audioModuleData.hdmiInput, (void *)g_NEXUS_audioModuleData.hdmiOutput, (void *)g_NEXUS_audioModuleData.frontend, (void *)g_NEXUS_audioModuleData.hRap));
    BDBG_LOG((" img: ctxt:%p i:%p", (void *)g_NEXUS_audioModuleData.img_context, (void *)&g_NEXUS_audioModuleData.img_interface));
    BDBG_LOG((" settings: wd:%d id:%d hbre:%d maxpb:%d",g_NEXUS_audioModuleData.moduleSettings.watchdogEnabled,g_NEXUS_audioModuleData.moduleSettings.independentDelay,g_NEXUS_audioModuleData.moduleSettings.hbrEnabled,g_NEXUS_audioModuleData.moduleSettings.maximumProcessingBranches));

    for (i=0; i < NEXUS_NUM_AUDIO_DECODERS; i++) {
        NEXUS_AudioDecoderStatus status;
        NEXUS_AudioDecoderHandle handle = &g_decoders[i];
        if (handle->opened) {
            NEXUS_Error errCode;
            NEXUS_PidChannelStatus pidChannelStatus;

            errCode = NEXUS_AudioDecoder_GetStatus(handle, &status);
            if( errCode )
            {
                BDBG_LOG((" channel%d: can't get audio decoder status, errCode=%x", i, errCode));
                continue;
            }
            errCode = NEXUS_PidChannel_GetStatus(handle->programSettings.pidChannel, &pidChannelStatus);
            if( errCode )
            {
                BDBG_LOG((" channel%d: can't get pidChannel status for pidCh=%p, errCode=%x", i, (void *)handle->programSettings.pidChannel, errCode));
                continue;
            }
            BDBG_LOG((" channel%d: (%p) %s", i, (void *)handle->rapChannel, status.locked ? "locked " : ""));
            BDBG_LOG(("  started=%c, codec=%d, pid=%#x, pidCh=%p, stcCh=%p", status.started ? 'y' : 'n',
                status.started ? status.codec : 0, pidChannelStatus.pid, (void *)handle->programSettings.pidChannel, (void *)handle->programSettings.stcChannel));
            BDBG_LOG(("  fifo: %d/%d (%d%%), queued: %d", status.fifoDepth, status.fifoSize, status.fifoSize ? status.fifoDepth*100/status.fifoSize : 0, status.queuedFrames));
            BDBG_LOG(("  TSM: %s pts=%#x pts_stc_diff=%d errors=%d", status.tsm ? "enabled" : "disabled", status.pts, status.ptsStcDifference, status.ptsErrorCount));
            BDBG_LOG(("  watchdogs: %d", status.numWatchdogs));
        }
    }
#endif
}

NEXUS_ModuleHandle NEXUS_AudioModule_Init(const NEXUS_AudioModuleSettings *pSettings)
{
    BERR_Code errCode;
    NEXUS_ModuleSettings moduleSettings;
    BRAP_Settings *pRapSettings;

    BDBG_ASSERT(NULL != pSettings); /* no default */
    BDBG_ASSERT(NULL != pSettings->modules.transport); /* required module */
    BDBG_ASSERT(NULL == g_NEXUS_audioModule);

    /* init global module handle */
    NEXUS_Module_GetDefaultSettings(&moduleSettings);
    moduleSettings.priority = NEXUS_ModulePriority_eLow; /* decoder interface is slow */
    moduleSettings.dbgPrint = NEXUS_AudioModule_Print;
    moduleSettings.dbgModules = "nexus_audio_module";
    g_NEXUS_audioModule = NEXUS_Module_Create("audio", &moduleSettings);
    if ( NULL == g_NEXUS_audioModule )
    {
        errCode=BERR_TRACE(BERR_OS_ERROR);
        return NULL;
    }
    NEXUS_LockModule();

    /* Get defaults if required */
    if ( NULL == pSettings )
    {
        NEXUS_AudioModule_GetDefaultSettings(NULL, &g_NEXUS_audioModuleData.moduleSettings);
        pSettings = &g_NEXUS_audioModuleData.moduleSettings;
    }
    else
    {
        g_NEXUS_audioModuleData.moduleSettings = *pSettings;
    }

    /* Save transport handle */
    g_NEXUS_audioModuleData.transport = pSettings->modules.transport;
    g_NEXUS_audioModuleData.hdmiInput = pSettings->modules.hdmiInput;
    g_NEXUS_audioModuleData.surface = pSettings->modules.surface;
    #if NEXUS_CONNECT_RFAUDIO_AND_FRONTEND
    g_NEXUS_audioModuleData.frontend = pSettings->modules.frontend;
    if ( NULL == g_NEXUS_audioModuleData.frontend )
    {
        BDBG_WRN(("Frontend module handle not provided.  Unable to perform automatic RF audio standard detection"));
    }
    #endif

    /* Initialize Raptor */
    pRapSettings = BKNI_Malloc(sizeof(BRAP_Settings));
    if ( NULL == pRapSettings )
    {
        errCode=BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto err_malloc;
    }
    BRAP_GetDefaultSettings(pRapSettings);

#if BRAP_VER < 4
    pRapSettings->sDeviceRBufPool.bHbrMode = pSettings->hbrEnabled;
#endif

    #if NEXUS_NUM_AUDIO_CAPTURES
    pRapSettings->sDeviceRBufPool.uiMaxNumRBuf[0] = NEXUS_NUM_AUDIO_CAPTURES;
    if ( pSettings->audioCaptureChannels > 1 )
    {
        pRapSettings->sDeviceRBufPool.uiMaxNumRBuf[0] *= 2;
        if ( pSettings->audioCaptureChannels == 6 )
        {
            pRapSettings->sDeviceRBufPool.uiMaxNumRBuf[1] = 2*NEXUS_NUM_AUDIO_CAPTURES;
            pRapSettings->sDeviceRBufPool.uiMaxNumRBuf[2] = 2*NEXUS_NUM_AUDIO_CAPTURES;
        }
    }
    pRapSettings->sDeviceRBufPool.sDstBufSettings[BRAP_OutputChannelPair_eLR].uiSize = pSettings->audioCaptureFifoSize;
    #endif

    /* Image download */
    if (Nexus_Core_P_Img_Create(NEXUS_CORE_IMG_ID_RAP,&g_NEXUS_audioModuleData.img_context,&g_NEXUS_audioModuleData.img_interface )== NEXUS_SUCCESS)
    {
        BDBG_WRN(("FW download used"));
        pRapSettings->pImgInterface = (BIMG_Interface*)&g_NEXUS_audioModuleData.img_interface;
        pRapSettings->pImgContext   = g_NEXUS_audioModuleData.img_context;
    }
    else
    {
        BDBG_WRN(("FW download not used"));
        g_NEXUS_audioModuleData.img_context= NULL;
        /* TODO remove the lines below after rap supports the default interface */
        pRapSettings->pImgInterface = &BRAP_IMG_Interface;
        pRapSettings->pImgContext = BRAP_IMG_Context;
    }

#if NEXUS_GLOBAL_INDEPENDENT_VOLUME
    /***************************************************************************/
    /* This is required to avoid glitches in captured audio due to massive     */
    /* timebase differences between the capture buffer and the output ports    */
    /***************************************************************************/
#if BRAP_VER < 4
    pRapSettings->bIndOpVolCtrl = false;      
#endif
#endif

    pRapSettings->sDeviceRBufPool.uiNumPostMixingInputBuffers = pSettings->numPostMixingInputBuffers;
#if defined(RAP_MS10_SUPPORT) || defined(RAP_MS11_SUPPORT)
    pRapSettings->sDeviceRBufPool.uiNumPostMixingOutputBuffers = 2;
#else
    pRapSettings->sDeviceRBufPool.uiNumPostMixingOutputBuffers = pSettings->numPostMixingOutputBuffers;
#endif

    BDBG_MSG(("Opening RAP"));

    /* Open the RAP device (first instance only) */
    errCode = BRAP_Open(&g_NEXUS_audioModuleData.hRap,
                        g_pCoreHandles->chp,
                        g_pCoreHandles->reg,
                        g_pCoreHandles->heap[0].mem,
                        g_pCoreHandles->bint,
                        g_pCoreHandles->tmr,
                        pRapSettings);

    BKNI_Free(pRapSettings);

    if ( errCode )
    {
        errCode=BERR_TRACE(errCode);
        goto err_rap;
    }


#if BCHP_CHIP == 7405 || BCHP_CHIP == 7336 || BCHP_CHIP == 7335 || BCHP_CHIP == 7325 || \
    BCHP_CHIP==7342 || BCHP_CHIP==7340
    /* This is required for stereo downmix while doing multichannnel pcm on HDMI */
    {
        BRAP_DownMixingCoef sDnMixingCoeff;

        sDnMixingCoeff.eInputMode = BRAP_OutputMode_e3_2;
        sDnMixingCoeff.bInputLfePresent = true;
        sDnMixingCoeff.eOutputMode = BRAP_OutputMode_e2_0;
        sDnMixingCoeff.bOutputLfePresent = false;

        sDnMixingCoeff.uDnCoeff.ui32DnCoef5_1To2_0[0][0] = 0x005A6000;
        sDnMixingCoeff.uDnCoeff.ui32DnCoef5_1To2_0[0][1] = 0x0;
        sDnMixingCoeff.uDnCoeff.ui32DnCoef5_1To2_0[0][2] = 0x00400000;
        sDnMixingCoeff.uDnCoeff.ui32DnCoef5_1To2_0[0][3] = 0x0;
        sDnMixingCoeff.uDnCoeff.ui32DnCoef5_1To2_0[0][4] = 0x00400000;
        sDnMixingCoeff.uDnCoeff.ui32DnCoef5_1To2_0[0][5] = 0x0;
        sDnMixingCoeff.uDnCoeff.ui32DnCoef5_1To2_0[1][0] = 0x0;
        sDnMixingCoeff.uDnCoeff.ui32DnCoef5_1To2_0[1][1] = 0x005A6000;
        sDnMixingCoeff.uDnCoeff.ui32DnCoef5_1To2_0[1][2] = 0x0;
        sDnMixingCoeff.uDnCoeff.ui32DnCoef5_1To2_0[1][3] = 0x00400000;
        sDnMixingCoeff.uDnCoeff.ui32DnCoef5_1To2_0[1][4] = 0x00400000;
        sDnMixingCoeff.uDnCoeff.ui32DnCoef5_1To2_0[1][5] = 0x0;
        errCode = BRAP_SetDownMixCoef(g_NEXUS_audioModuleData.hRap,&sDnMixingCoeff);
        if (errCode)
        {
            BDBG_ERR(("Failed to set Downmix coef"));
        }
    }

#endif

    /* The BRAP_OutputPort enum must map to nexus_core_audio.h's NEXUS_AudioOutputPort.
    Because there is an eclectic list of values, I'm CASSERT'ing more than one. */
    BDBG_CASSERT(BRAP_OutputPort_eMax == (BRAP_OutputPort)NEXUS_AudioOutputPort_eMax);
    BDBG_CASSERT(BRAP_OutputPort_eI2s2 == (BRAP_OutputPort)NEXUS_AudioOutputPort_eI2s2);
    BDBG_CASSERT(BRAP_OutputPort_eMaiMulti3 == (BRAP_OutputPort)NEXUS_AudioOutputPort_eMaiMulti3);
    BDBG_CASSERT(BRAP_OutputPort_eSpdif1 == (BRAP_OutputPort)NEXUS_AudioOutputPort_eSpdif1);

    BDBG_MSG(("Initializing DACS"));
    errCode = NEXUS_AudioDac_P_Init();
    if ( errCode )
    {
        errCode=BERR_TRACE(errCode);
        goto err_dac;
    }

    #if NEXUS_NUM_SPDIF_OUTPUTS
    BDBG_MSG(("Initializing SPDIF Outputs"));
    errCode = NEXUS_SpdifOutput_P_Init();
    if ( errCode )
    {
        errCode=BERR_TRACE(errCode);
        goto err_spdif;
    }
    #endif

    #if NEXUS_NUM_HDMI_OUTPUTS
    BDBG_MSG(("Initializing HDMI audio outputs"));
    NEXUS_AudioOutput_P_InitHdmiOutput();
    #endif

    #if NEXUS_NUM_AUDIO_DECODERS
    BDBG_MSG(("Initializing Decoders"));
    errCode = NEXUS_AudioDecoder_P_Init();
    if ( errCode )
    {
        errCode=BERR_TRACE(errCode);
        goto err_decoder;
    }
    #endif
    #if NEXUS_NUM_HDMI_INPUTS
    BDBG_MSG(("Initializing HDMI Input"));
    {
        BRAP_InputPortConfig inputConfig;
        /* Initialize the external input in RAP */
        errCode = BRAP_GetDefaultInputConfig(g_NEXUS_audioModuleData.hRap, BRAP_CapInputPort_eHdmi, &inputConfig);
        if ( errCode )
        {
            errCode = BERR_TRACE(errCode);
            goto err_spdif;
        }
        inputConfig.eBufDataMode = BRAP_BufDataMode_eStereoNoninterleaved;
        inputConfig.eCapPort = BRAP_CapInputPort_eHdmi; /* TODO: Support > 1 -- Enum has no values for this */
        inputConfig.eInputBitsPerSample = BRAP_InputBitsPerSample_e16;
        inputConfig.sCapPortParams.uCapPortParams.sInputI2sParams.bAlignedToLRClk = false;
        inputConfig.eSampleRate = BAVC_AudioSamplingRate_e48k;
        errCode = BRAP_SetInputConfig(g_NEXUS_audioModuleData.hRap, &inputConfig);
        if ( errCode )
        {
            errCode = BERR_TRACE(errCode);
            goto err_spdif;
        }
    }
    #endif

    #if NEXUS_NUM_AUDIO_CAPTURES
    BDBG_MSG(("Initializing capture channel manager"));
    errCode = NEXUS_AudioCaptureChannel_P_Init();
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        goto err_spdif;
    }
    #endif

    #if NEXUS_NUM_RF_AUDIO_DECODERS
    BDBG_MSG(("Initializing RF Audio Decoders"));
    errCode = NEXUS_RfAudioDecoder_P_Init();
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        goto err_rf;
    }
    #endif

    /* Success */
    NEXUS_UnlockModule();
    return g_NEXUS_audioModule;
#if NEXUS_NUM_RF_AUDIO_DECODERS
err_rf:
#endif
#if NEXUS_NUM_SPDIF_OUTPUTS || NEXUS_NUM_AUDIO_CAPTURES || NEXUS_NUM_HDMI_OUTPUTS || NEXUS_NUM_HDMI_INPUTS
err_spdif:
#endif
#if NEXUS_NUM_AUDIO_DECODERS
err_decoder:
#endif
err_dac:
    BRAP_Close(g_NEXUS_audioModuleData.hRap);
err_rap:
err_malloc:
    NEXUS_UnlockModule();
    NEXUS_Module_Destroy(g_NEXUS_audioModule);
    return NULL;
}

void NEXUS_AudioModule_Uninit(void)
{
    NEXUS_LockModule();  
    #if NEXUS_NUM_RF_AUDIO_DECODERS
    NEXUS_RfAudioDecoder_P_Uninit();
    #endif
    #if NEXUS_NUM_AUDIO_DECODERS
    NEXUS_AudioDecoder_P_Uninit();
    #endif
    #if NEXUS_NUM_AUDIO_CAPTURES
    NEXUS_AudioCaptureChannel_P_Uninit();
    #endif
    BRAP_Close(g_NEXUS_audioModuleData.hRap);
    Nexus_Core_P_Img_Destroy(g_NEXUS_audioModuleData.img_context);
    NEXUS_UnlockModule();
    NEXUS_Module_Destroy(g_NEXUS_audioModule);
    g_NEXUS_audioModule = NULL;

    BKNI_Memset(&g_NEXUS_audioModuleData, 0, sizeof(g_NEXUS_audioModuleData));
}

void NEXUS_AudioModule_GetDefaultSettings(
    const struct NEXUS_Core_PreInitState *preInitState,
    NEXUS_AudioModuleSettings *pSettings    /* [out] */
    )
{
    BSTD_UNUSED(preInitState);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));

    /* enable watchdog by default */
    pSettings->watchdogEnabled = true;
    /* If dolby digital plus is enabled, default HBR to on */
    #if NEXUS_DDP
    pSettings->hbrEnabled = true;
    #endif

#if BRAP_VER >= 4
    pSettings->maximumProcessingBranches = 2;
#else
    /* Allow for one post-processing branch by default, this is typical */
    pSettings->maximumProcessingBranches = 1;
#endif

    if ( NEXUS_GetEnv("max_processing_branches") )
    {
        pSettings->maximumProcessingBranches = NEXUS_atoi(NEXUS_GetEnv("max_processing_branches"));
        BDBG_WRN(("max_processing_branches changed to %d",pSettings->maximumProcessingBranches));
    }

    if ( NEXUS_GetEnv("no_independent_delay") )
    {
        pSettings->independentDelay = false; /* Disable independent delay if requested */
    }
    else
    {
        pSettings->independentDelay = true; /* Enable independent delay by default */
    }

    BRAP_GetDefaultRingBufferSize(&pSettings->audioCaptureFifoSize);
    pSettings->audioCaptureChannels = (NEXUS_GetEnv("audio_capture_multichannel")?6:2);

    /* intialize buffers for post-mixing DSP. For now 2 for stereo processing */
#if defined RAP_MS11_SUPPORT
    pSettings->numPostMixingInputBuffers = 2;
    pSettings->numPostMixingOutputBuffers = 9; /* stereo (2) + 5.1 (6) + compressed (1) */
#endif
}

BAVC_AudioSamplingRate NEXUS_AudioModule_P_SampleRate2Avc(unsigned sampleRate)
{
    switch ( sampleRate )
    {
    case 48000:
        return BAVC_AudioSamplingRate_e48k;
    case 44100:
        return BAVC_AudioSamplingRate_e44_1k;
    case 32000:
        return BAVC_AudioSamplingRate_e32k;
    case 96000:
        return BAVC_AudioSamplingRate_e96k;
    case 16000:
        return BAVC_AudioSamplingRate_e16k;
    case 22050:
        return BAVC_AudioSamplingRate_e22_05k;
    case 24000:
        return BAVC_AudioSamplingRate_e24k;
    case 64000:
        return BAVC_AudioSamplingRate_e64k;
    case 88200:
        return BAVC_AudioSamplingRate_e88_2k;
    case 128000:
        return BAVC_AudioSamplingRate_e128k;
    case 176400:
        return BAVC_AudioSamplingRate_e176_4k;
    case 192000:
        return BAVC_AudioSamplingRate_e192k;
    case 8000:
        return BAVC_AudioSamplingRate_e8k;
    case 12000:
        return BAVC_AudioSamplingRate_e12k;
    case 11025:
        return BAVC_AudioSamplingRate_e11_025k;
    default:
        BDBG_MSG(("Unrecognized sample rate (%u)- defaulting to BAVC_AudioSamplingRate_e48k", sampleRate));
        return BAVC_AudioSamplingRate_e48k;
    }
}

unsigned NEXUS_AudioModule_P_Avc2SampleRate(BAVC_AudioSamplingRate sampleRate)
{
    switch ( sampleRate )
    {
    case BAVC_AudioSamplingRate_e48k:
        return 48000;
    case BAVC_AudioSamplingRate_e44_1k:
        return 44100;
    case BAVC_AudioSamplingRate_e32k:
        return 32000;
    case BAVC_AudioSamplingRate_e96k:
        return 96000;
    case BAVC_AudioSamplingRate_e16k:
        return 16000;
    case BAVC_AudioSamplingRate_e22_05k:
        return 22050;
    case BAVC_AudioSamplingRate_e24k:
        return 24000;
    case BAVC_AudioSamplingRate_e64k:
        return 64000;
    case BAVC_AudioSamplingRate_e88_2k:
        return 88200;
    case BAVC_AudioSamplingRate_e128k:
        return 128000;
    case BAVC_AudioSamplingRate_e176_4k:
        return 176400;
    case BAVC_AudioSamplingRate_e192k:
        return 192000;
    case BAVC_AudioSamplingRate_e8k:
        return 8000;
    case BAVC_AudioSamplingRate_e12k:
        return 12000;
    case BAVC_AudioSamplingRate_e11_025k:
        return 11025;
    default:
        BDBG_MSG(("Unrecognized sample rate (%u) - defaulting to 48000", sampleRate));
        return 48000;
        /* Fall through */
    }
}

NEXUS_Error NEXUS_AudioModule_SetRampStepSettings(
    const NEXUS_AudioRampStepSettings *pRampStepSettings  /* ramping step size for scale change for all output ports */
    )
{
    BDBG_ASSERT(NULL != g_NEXUS_audioModule);
#if BRAP_VER >= 4
    BRAP_SetRampStepSize(g_NEXUS_audioModuleData.hRap, BRAP_RampParameter_eSampleRateConversion,pRampStepSettings->srcRampStep);
    BRAP_SetRampStepSize(g_NEXUS_audioModuleData.hRap, BRAP_RampParameter_eVolume,pRampStepSettings->mixerRampStep);
#else
#if (BCHP_CHIP!=7550)
    BRAP_SetSrcRampStepSize(g_NEXUS_audioModuleData.hRap,pRampStepSettings->srcRampStep);
#endif
    BRAP_SetMixerRampStepSize(g_NEXUS_audioModuleData.hRap,pRampStepSettings->mixerRampStep);
#endif
    return NEXUS_SUCCESS;
}
void NEXUS_AudioModule_GetRampStepSettings(
    NEXUS_AudioRampStepSettings *pRampStepSettings      /* [out] ramping step size for scale change for all output ports */
    )
{
    BDBG_ASSERT(NULL != g_NEXUS_audioModule);
#if BRAP_VER >= 4
    BRAP_GetRampStepSize(g_NEXUS_audioModuleData.hRap, BRAP_RampParameter_eSampleRateConversion,&pRampStepSettings->srcRampStep);
    BRAP_GetRampStepSize(g_NEXUS_audioModuleData.hRap, BRAP_RampParameter_eVolume,&pRampStepSettings->mixerRampStep);
#else
#if (BCHP_CHIP!=7550)
    BRAP_GetSrcRampStepSize(g_NEXUS_audioModuleData.hRap,&pRampStepSettings->srcRampStep);
#endif
    BRAP_GetMixerRampStepSize(g_NEXUS_audioModuleData.hRap,&pRampStepSettings->mixerRampStep);
#endif
    return;
}

NEXUS_Error NEXUS_AudioModule_EnableExternalMclk(
    unsigned mclkIndex,
    NEXUS_AudioOutputPll pllIndex,
    NEXUS_ExternalMclkRate mclkRate
    )
{
    BERR_Code errCode;
    BRAP_OP_ExtMClkSettings mclkSettings;
    mclkSettings.ePll = pllIndex;
    mclkSettings.eMClockRate = mclkRate;
    errCode = BRAP_OP_ExtMClkConfig(g_NEXUS_audioModuleData.hRap, mclkIndex, &mclkSettings);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }
    return BERR_SUCCESS;
}

BRAP_DSPCHN_AudioType NEXUS_Audio_P_CodecToAudioType(NEXUS_AudioCodec codec)
{
    switch ( codec )
    {
    case NEXUS_AudioCodec_eMpeg:
    case NEXUS_AudioCodec_eMp3:
        return BRAP_DSPCHN_AudioType_eMpeg;
    case NEXUS_AudioCodec_eAacAdts:
        return BRAP_DSPCHN_AudioType_eAacAdts;
    case NEXUS_AudioCodec_eAacLoas:
        return BRAP_DSPCHN_AudioType_eAacLoas;
    case NEXUS_AudioCodec_eAacPlusLoas:
        return BRAP_DSPCHN_AudioType_eAacSbrLoas;
    case NEXUS_AudioCodec_eAacPlusAdts:
        return BRAP_DSPCHN_AudioType_eAacSbrAdts;
    case NEXUS_AudioCodec_eAc3:
        return BRAP_DSPCHN_AudioType_eAc3;
    case NEXUS_AudioCodec_eAc3Plus:
        return BRAP_DSPCHN_AudioType_eAc3Plus;
    case NEXUS_AudioCodec_eDtsLegacy: /* For DTS streams with legacy frame-sync.  These streams are something called as 14bits stream */
        return BRAP_DSPCHN_AudioType_eDtsBroadcast;
    case NEXUS_AudioCodec_eLpcmDvd:
        return BRAP_DSPCHN_AudioType_eLpcmDvd;
    case NEXUS_AudioCodec_eLpcmHdDvd:
        return BRAP_DSPCHN_AudioType_eLpcmHdDvd;
    case NEXUS_AudioCodec_eLpcmBluRay:
        return BRAP_DSPCHN_AudioType_eLpcmBd;
    case NEXUS_AudioCodec_eDts:
            /* fall through */
    case NEXUS_AudioCodec_eDtsHd:
        return BRAP_DSPCHN_AudioType_eDtshd; /* For DTS streams with non-legacy frame-sync */
    case NEXUS_AudioCodec_eWmaStd:
        return BRAP_DSPCHN_AudioType_eWmaStd;
    case NEXUS_AudioCodec_eWmaPro:
        return BRAP_DSPCHN_AudioType_eWmaPro;
#if 0
    case NEXUS_AudioCodec_eAvs:
        return ;
#endif
    case NEXUS_AudioCodec_ePcm:
        return BRAP_DSPCHN_AudioType_ePcm;
    case NEXUS_AudioCodec_ePcmWav:
        return BRAP_DSPCHN_AudioType_ePcmWav;
    case NEXUS_AudioCodec_eAmr:
        return BRAP_DSPCHN_AudioType_eAmr;
    case NEXUS_AudioCodec_eDra:
        return BRAP_DSPCHN_AudioType_eDra;
    case NEXUS_AudioCodec_eCook:
        return BRAP_DSPCHN_AudioType_eRealAudioLbr;
#if BRAP_VER >= 4 && BCHP_CHIP == 35230
    case NEXUS_AudioCodec_eAdpcm:
        return BRAP_DSPCHN_AudioType_eAdpcm;
#endif
    default:
        return BRAP_DSPCHN_AudioType_eInvalid;
    }
}

NEXUS_AudioCodec NEXUS_Audio_P_AudioTypeToCodec(BRAP_DSPCHN_AudioType codec)
{
    switch ( codec )
    {
    case BRAP_DSPCHN_AudioType_eMpeg:
        return NEXUS_AudioCodec_eMpeg;  /* In status, you must verify another field to determine MP3/MP[12] */
    case BRAP_DSPCHN_AudioType_eAacAdts:
        return NEXUS_AudioCodec_eAacAdts;
    case BRAP_DSPCHN_AudioType_eAacLoas:
        return NEXUS_AudioCodec_eAacLoas;
    case BRAP_DSPCHN_AudioType_eAacSbrLoas:
        return NEXUS_AudioCodec_eAacPlusLoas;
    case BRAP_DSPCHN_AudioType_eAacSbrAdts:
        return NEXUS_AudioCodec_eAacPlusAdts;
    case BRAP_DSPCHN_AudioType_eAc3:
        return NEXUS_AudioCodec_eAc3;
    case BRAP_DSPCHN_AudioType_eAc3Plus:
        return NEXUS_AudioCodec_eAc3Plus;
    case BRAP_DSPCHN_AudioType_eDtsBroadcast:
        return NEXUS_AudioCodec_eDtsLegacy; /* For DTS streams with legacy frame-sync.  These streams are something called as 14bits stream */
    case BRAP_DSPCHN_AudioType_eLpcmDvd:
        return NEXUS_AudioCodec_eLpcmDvd;
    case BRAP_DSPCHN_AudioType_eLpcmHdDvd:
        return NEXUS_AudioCodec_eLpcmHdDvd;
    case BRAP_DSPCHN_AudioType_eLpcmBd:
        return NEXUS_AudioCodec_eLpcmBluRay;
    case BRAP_DSPCHN_AudioType_eDtshd:
        return NEXUS_AudioCodec_eDtsHd; /* For DTS streams wit non-legacy frame-sync */
    case BRAP_DSPCHN_AudioType_eWmaStd:
        return NEXUS_AudioCodec_eWmaStd;
    case BRAP_DSPCHN_AudioType_eWmaPro:
        return NEXUS_AudioCodec_eWmaPro;
    case BRAP_DSPCHN_AudioType_ePcm:
        return NEXUS_AudioCodec_ePcm;
    case BRAP_DSPCHN_AudioType_ePcmWav:
        return NEXUS_AudioCodec_ePcmWav;
    case BRAP_DSPCHN_AudioType_eAmr:
        return NEXUS_AudioCodec_eAmr;
    case BRAP_DSPCHN_AudioType_eDra:
        return NEXUS_AudioCodec_eDra;
    case BRAP_DSPCHN_AudioType_eRealAudioLbr:
        return NEXUS_AudioCodec_eCook;
#if BRAP_VER >= 4 && BCHP_CHIP == 35230
    case BRAP_DSPCHN_AudioType_eAdpcm:
        return NEXUS_AudioCodec_eAdpcm;
#endif
    default:
        BDBG_WRN(("Unknown magnum audio codec %d", codec));
        return NEXUS_AudioCodec_eUnknown;
    }
}

BRAP_ProcessingType NEXUS_Audio_P_CodecToProcessingType(NEXUS_AudioCodec codec)
{
    switch ( codec )
    {
    case NEXUS_AudioCodec_eAc3:
        return BRAP_ProcessingType_eEncodeAc3;
    case NEXUS_AudioCodec_eDts:
        return BRAP_ProcessingType_eEncodeDts;
#if BRAP_VER >= 4
    case NEXUS_AudioCodec_eSbc:
        return BRAP_ProcessingType_eEncodeSbc;
    case NEXUS_AudioCodec_eAacLoas:
    case NEXUS_AudioCodec_eAacAdts:
    case NEXUS_AudioCodec_eAacPlusLoas:
    case NEXUS_AudioCodec_eAacPlusAdts:
        return BRAP_ProcessingType_eEncodeAacHe;
#endif
    default:
        return BRAP_ProcessingType_eNone;
    }
}


#include "bchp_common.h"
#include "bchp_aud_fmm_misc.h"

#if defined BCHP_VCXO_2_RM_REG_START
#define NUM_VCXOS 3
#elif defined BCHP_VCXO_1_RM_REG_START
#define NUM_VCXOS 2
#elif defined BCHP_VCXO_0_RM_REG_START
#define NUM_VCXOS 1
#else
#define NUM_VCXOS 0
#endif

#if defined BCHP_AUD_FMM_PLL2_REG_START
#define NUM_PLLS 3
#elif defined BCHP_AUD_FMM_PLL1_REG_START
#define NUM_PLLS 2
#elif defined BCHP_AUD_FMM_PLL0_REG_START
#define NUM_PLLS 1
#else
#define NUM_PLLS 0
#endif

#if defined BCHP_AUD_FMM_MISC_STC_UPPERi_ARRAY_END
#define NUM_STCS (BCHP_AUD_FMM_MISC_STC_UPPERi_ARRAY_END + 1)
#else
#define NUM_STCS 0
#endif

void NEXUS_GetAudioCapabilities(NEXUS_AudioCapabilities *pCaps)
{
    unsigned i;
    BRAP_DSPCHN_AudioType audioType;
    BRAP_ProcessingType processingType;
    BRAP_Capabilities rapCaps;

    BDBG_ASSERT(NULL != pCaps);
    BKNI_Memset(pCaps, 0, sizeof(NEXUS_AudioCapabilities));
    #if NEXUS_NUM_I2S_INPUTS
    pCaps->numInputs.i2s = NEXUS_NUM_I2S_INPUTS;
    #endif
    #if NEXUS_NUM_AUDIO_CAPTURES
    pCaps->numOutputs.capture = NEXUS_NUM_AUDIO_CAPTURES;
    #endif
    #if NEXUS_NUM_HDMI_OUTPUTS
    pCaps->numOutputs.hdmi = NEXUS_NUM_HDMI_OUTPUTS;
    #endif
    #if NEXUS_NUM_I2S_OUTPUTS
    pCaps->numOutputs.i2s = NEXUS_NUM_I2S_OUTPUTS;
    #endif
    #if NEXUS_NUM_RFM_OUTPUTS
    pCaps->numOutputs.rfmod = NEXUS_NUM_RFM_OUTPUTS;
    #endif
    #if NEXUS_NUM_SPDIF_OUTPUTS
    pCaps->numOutputs.spdif = NEXUS_NUM_SPDIF_OUTPUTS;
    #endif
    #if NEXUS_NUM_AUDIO_DECODERS
    pCaps->numDecoders = NEXUS_NUM_AUDIO_DECODERS;
    #endif
    #if NEXUS_NUM_AUDIO_PLAYBACKS
    pCaps->numPlaybacks = NEXUS_NUM_AUDIO_PLAYBACKS;
    #endif
    pCaps->numVcxos = NUM_VCXOS;
    pCaps->numPlls = NUM_PLLS;
    pCaps->numStcs = NUM_STCS;


    pCaps->numDsps = 1; /* All RAP STB chips are single-dsp */

    BRAP_GetCapabilities(g_NEXUS_audioModuleData.hRap, &rapCaps);

    for ( i = 0; i < NEXUS_AudioCodec_eMax; i++ )
    {
        audioType = NEXUS_Audio_P_CodecToAudioType(i);
        processingType = NEXUS_Audio_P_CodecToProcessingType(i);

        if  ( audioType != BRAP_DSPCHN_AudioType_eInvalid )
        {
            pCaps->dsp.codecs[i].decode = rapCaps.bDecodeSupported[audioType];
        }
        if ( processingType != BRAP_ProcessingType_eNone )
        {
            pCaps->dsp.codecs[i].encode = rapCaps.bProcessingSupported[processingType];
            if ( pCaps->dsp.codecs[i].encode )
            {
                pCaps->dsp.encoder = true;
            }
        }
    }

    pCaps->dsp.autoVolumeLevel = rapCaps.bProcessingSupported[BRAP_ProcessingType_eAvl];
    pCaps->dsp._3dSurround = rapCaps.bProcessingSupported[BRAP_ProcessingType_eBrcm3DSurround];
    pCaps->dsp.decodeRateControl = rapCaps.bProcessingSupported[BRAP_ProcessingType_eDSOLA];
    pCaps->dsp.dolbyDigitalReencode = rapCaps.bProcessingSupported[BRAP_ProcessingType_eDDRE];
    pCaps->dsp.dolbyVolume = rapCaps.bProcessingSupported[BRAP_ProcessingType_eDolbyVolume];
    pCaps->dsp.dolbyVolume258 = rapCaps.bProcessingSupported[BRAP_ProcessingType_eDDRE];
    pCaps->dsp.mixer = rapCaps.bProcessingSupported[BRAP_ProcessingType_eFwMixer];
    if ( rapCaps.bProcessingSupported[BRAP_ProcessingType_eBtsc] )
    {
        pCaps->dsp.rfEncoder.supported = true;
        pCaps->dsp.rfEncoder.encodings[NEXUS_RfAudioEncoding_eBtsc] = true;
    }

    pCaps->dsp.truSurroundHd = rapCaps.bProcessingSupported[BRAP_ProcessingType_eSrsHd];
    pCaps->dsp.truVolume = rapCaps.bProcessingSupported[BRAP_ProcessingType_eSrsTruVolume];
}

void NEXUS_AudioOutputPll_GetSettings(
    NEXUS_AudioOutputPll pll,
    NEXUS_AudioOutputPllSettings *pSettings       /* [out] Current Settings */
    )
{
    BRAP_OP_PllSettings pllSettings;

    (void)BRAP_OP_Pll_GetSettings(g_NEXUS_audioModuleData.hRap, (int)pll, &pllSettings);
    pSettings->mode = NEXUS_AudioOutputPllMode_eVcxo;
    pSettings->modeSettings.vcxo.vcxo = pllSettings.uiVcxo;
}

NEXUS_Error NEXUS_AudioOutputPll_SetSettings(
    NEXUS_AudioOutputPll pll,
    const NEXUS_AudioOutputPllSettings *pSettings
    )
{
    BRAP_OP_PllSettings pllSettings;
    BERR_Code errCode;

    if ( pSettings->mode != NEXUS_AudioOutputPllMode_eVcxo )
    {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    (void)BRAP_OP_Pll_GetSettings(g_NEXUS_audioModuleData.hRap, (int)pll, &pllSettings);
    pllSettings.uiVcxo = pSettings->modeSettings.vcxo.vcxo;
    errCode = BRAP_OP_Pll_SetSettings(g_NEXUS_audioModuleData.hRap, (int)pll, &pllSettings);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    return BERR_SUCCESS;
}

void NEXUS_GetAudioFirmwareVersion(
    NEXUS_AudioFirmwareVersion *pVersion /* [out] */
    )
{
    BRAP_RevisionInfo versionInfo;

    BDBG_ASSERT(NULL != pVersion);
    BKNI_Memset(pVersion, 0, sizeof(*pVersion));

    if ( BERR_SUCCESS == BRAP_GetRevision(g_NEXUS_audioModuleData.hRap, &versionInfo) )
    {
        pVersion->major = versionInfo.sPIVersion.ui32MajorVersion;
        pVersion->minor = versionInfo.sPIVersion.ui32MinorVersion;
    }
}

void NEXUS_AudioModule_GetDefaultUsageSettings(
    NEXUS_AudioModuleUsageSettings *pSettings   /* [out] */
    )
{
    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
}

NEXUS_Error NEXUS_AudioModule_GetMemoryEstimate(
    const NEXUS_AudioModuleUsageSettings *pSettings,
    NEXUS_AudioModuleMemoryEstimate *pEstimate  /* [out] */
    )
{
    BSTD_UNUSED(pSettings);
    BDBG_ASSERT(NULL != pEstimate);
    BKNI_Memset(pEstimate, 0, sizeof(*pEstimate));

    return BERR_SUCCESS;
}
