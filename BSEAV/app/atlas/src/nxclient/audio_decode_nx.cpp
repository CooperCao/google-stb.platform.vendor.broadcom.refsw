/******************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *****************************************************************************/

#include "nexus_audio_decoder.h"
#include "audio_decode_nx.h"
#include "atlas_os.h"
#include "nxclient.h"

#define CALLBACK_ADECODE_SOURCE_CHANGED  "CallbackAudioDecodeSourceChanged"

BDBG_MODULE(atlas_audio_decode);

/* bwin io callback that is triggered when it is safe to handle callbacks */
static void bwinSimpleAudioDecodeSourceChangedCallback(
        void *       pObject,
        const char * strCallback
        )
{
    eRet                 ret                = eRet_Ok;
    CSimpleAudioDecode * pSimpleAudioDecode = (CSimpleAudioDecode *)pObject;

    BDBG_ASSERT(NULL != pSimpleAudioDecode);
    BSTD_UNUSED(strCallback);

    if (true == pSimpleAudioDecode->isSourceChanged())
    {
        ret = pSimpleAudioDecode->notifyObservers(eNotify_AudioSourceChanged, pSimpleAudioDecode);
        CHECK_ERROR("error notifying observers", ret);
    }
} /* bwinSimpleAudioDecodeSourceChangedCallback */

static void NexusSimpleAudioDecodeSourceChangedCallback(
        void * context,
        int    param
        )
{
    CSimpleAudioDecode * pSimpleAudioDecode = (CSimpleAudioDecode *)context;

    BSTD_UNUSED(param);
    BDBG_ASSERT(NULL != pSimpleAudioDecode);

    CWidgetEngine * pWidgetEngine = pSimpleAudioDecode->getWidgetEngine();
    pSimpleAudioDecode->setSourceChanged(true);

    /* sync with bwin loop */
    if (NULL != pWidgetEngine)
    {
        pWidgetEngine->syncCallback(pSimpleAudioDecode, CALLBACK_ADECODE_SOURCE_CHANGED);
    }
} /* NEXUSAudioDecodeSourceChangedCallback */

CSimpleAudioDecodeNx::CSimpleAudioDecodeNx(
        const char *     name,
        const unsigned   number,
        CConfiguration * pCfg
        ) :
    CSimpleAudioDecode(name, number, pCfg),
    _connectId(0),
    _bEncodeAc3(false),
    _bEncodeDts(false),
    _bAutoVolumeLevel(false),
    _bDolbyVolume(false),
    _bTruVolume(false)
{
    eRet ret = eRet_Ok;

    BDBG_ASSERT(eRet_Ok == ret);
}

CSimpleAudioDecodeNx::~CSimpleAudioDecodeNx()
{
    close();
}

eRet CSimpleAudioDecodeNx::open(
        CWidgetEngine * pWidgetEngine,
        CStc *          pStc
        )
{
    eRet ret = eRet_Ok;

    BDBG_ASSERT(NULL != pStc);

    if (true == isOpened())
    {
        ret = eRet_InvalidState;
        CHECK_ERROR_GOTO("Simple audio decoder is already opened.", ret, error);
    }

    if (NULL == _pBoardResources)
    {
        ret = eRet_InvalidState;
        CHECK_ERROR_GOTO("Set board resources before opening.", ret, error);
    }

    _pWidgetEngine = pWidgetEngine;
    if (NULL != _pWidgetEngine)
    {
        _pWidgetEngine->addCallback(this, CALLBACK_ADECODE_SOURCE_CHANGED, bwinSimpleAudioDecodeSourceChangedCallback);
    }
    else
    {
        BDBG_MSG(("Opening CSimpleAudioDecode num:%d without providing widgetEngine - decoder callbacks will be ignored", getNumber()));
    }

    _simpleDecoder = NEXUS_SimpleAudioDecoder_Acquire(getNumber());
    CHECK_PTR_ERROR_GOTO("unable to acquire simple audio decoder", _simpleDecoder, ret, eRet_NotAvailable, error);

    ret = setStc(pStc);
    CHECK_ERROR_GOTO("SetStcChannel simple audio decoder failed!", ret, error);

    /* set audio decoder settings */
    {
        NEXUS_Error nError = NEXUS_SUCCESS;
        NEXUS_SimpleAudioDecoderSettings audioDecoderSettings;

        NEXUS_SimpleAudioDecoder_GetSettings(_simpleDecoder, &audioDecoderSettings);
        audioDecoderSettings.primary.sourceChanged.callback   = NexusSimpleAudioDecodeSourceChangedCallback;
        audioDecoderSettings.primary.sourceChanged.context    = this;
        audioDecoderSettings.primary.sourceChanged.param      = 0;
        audioDecoderSettings.secondary.sourceChanged.callback = NexusSimpleAudioDecodeSourceChangedCallback;
        audioDecoderSettings.secondary.sourceChanged.context  = this;
        audioDecoderSettings.secondary.sourceChanged.param    = 0;
        nError = NEXUS_SimpleAudioDecoder_SetSettings(_simpleDecoder, &audioDecoderSettings);
        CHECK_NEXUS_ERROR_GOTO("Error applying simple audio decoder settings.", ret, nError, error);
    }

    /* save audio decoder capabilities */
    {
        NEXUS_AudioCapabilities capabilities;
        NEXUS_GetAudioCapabilities(&capabilities);

        if (true == capabilities.dsp.encoder)
        {
            _bEncodeAc3 = capabilities.dsp.codecs[NEXUS_AudioCodec_eAc3].encode;
            _bEncodeDts = capabilities.dsp.codecs[NEXUS_AudioCodec_eDts].encode;
        }

        _bAutoVolumeLevel = capabilities.dsp.autoVolumeLevel;
        _bDolbyVolume     = capabilities.dsp.dolbyVolume258;
        _bTruVolume       = capabilities.dsp.truVolume;
    }
error:
    return(ret);
} /* open */

eHdmiAudioInput CSimpleAudioDecodeNx::getHdmiInput(NEXUS_AudioCodec codec)
{
    eRet                 ret    = eRet_Ok;
    NEXUS_Error          nerror = NEXUS_SUCCESS;
    NxClient_AudioStatus statusAudio;
    eHdmiAudioInput      hdmiAudioType = eHdmiAudioInput_None;

    BDBG_ASSERT(NEXUS_AudioCodec_eMax > codec);

    nerror = NxClient_GetAudioStatus(&statusAudio);
    CHECK_NEXUS_ERROR_GOTO("unable to get audio status", ret, nerror, error);

    switch (statusAudio.hdmi.outputMode)
    {
    case NxClient_AudioOutputMode_eAuto:
        BDBG_ERR(("not sure what to do with this hdmi output mode"));
        break;
    case NxClient_AudioOutputMode_ePcm:
        hdmiAudioType = eHdmiAudioInput_Pcm;
        break;
    case NxClient_AudioOutputMode_eMultichannelPcm:
        hdmiAudioType = eHdmiAudioInput_Multichannel;
        break;
    case NxClient_AudioOutputMode_ePassthrough:
        hdmiAudioType = eHdmiAudioInput_Compressed;
        break;
    case NxClient_AudioOutputMode_eTranscode:
        if (NEXUS_AudioCodec_eAc3 == statusAudio.hdmi.outputCodec)
        {
            hdmiAudioType = eHdmiAudioInput_EncodeAc3;
        }
        else
        if (NEXUS_AudioCodec_eDts == statusAudio.hdmi.outputCodec)
        {
            hdmiAudioType = eHdmiAudioInput_EncodeDts;
        }
        else
        {
            hdmiAudioType = eHdmiAudioInput_Compressed;
        }
        break;
    case NxClient_AudioOutputMode_eNone:
    case NxClient_AudioOutputMode_eMax:
    default:
        break;
    } /* switch */

error:
    return(hdmiAudioType);
} /* getHdmiInput */

eRet CSimpleAudioDecodeNx::setHdmiInput(
        eHdmiAudioInput                          hdmiInput,
        NEXUS_SimpleAudioDecoderServerSettings * pSettings
        )
{
    eRet                   ret    = eRet_Ok;
    NEXUS_Error            nerror = NEXUS_SUCCESS;
    NxClient_AudioSettings settings;

    BSTD_UNUSED(pSettings);

    NxClient_GetAudioSettings(&settings);
    settings.hdmi.transcodeCodec = NEXUS_AudioCodec_eAc3;

    switch (hdmiInput)
    {
    case eHdmiAudioInput_Multichannel:
        settings.hdmi.outputMode = NxClient_AudioOutputMode_eMultichannelPcm;
        break;

    case eHdmiAudioInput_EncodeDts:
        settings.hdmi.outputMode     = NxClient_AudioOutputMode_eTranscode;
        settings.hdmi.transcodeCodec = NEXUS_AudioCodec_eDts;
        break;

    case eHdmiAudioInput_EncodeAc3:
        settings.hdmi.outputMode = NxClient_AudioOutputMode_eTranscode;
        break;

    case eHdmiAudioInput_Compressed:
        settings.hdmi.outputMode = NxClient_AudioOutputMode_ePassthrough;
        break;

    case eHdmiAudioInput_Pcm:
    default:
        settings.hdmi.outputMode = NxClient_AudioOutputMode_ePcm;
        break;
    } /* switch */

    nerror = NxClient_SetAudioSettings(&settings);
    CHECK_NEXUS_ERROR_GOTO("unable to set hdmi output type", nerror, ret, error);
error:
    return(ret);
} /* setHdmiInput */

eSpdifInput CSimpleAudioDecodeNx::getSpdifInput(NEXUS_AudioCodec codec)
{
    eRet                 ret       = eRet_Ok;
    NEXUS_Error          nerror    = NEXUS_SUCCESS;
    eSpdifInput          spdifType = eSpdifInput_None;
    NxClient_AudioStatus statusAudio;

    BDBG_ASSERT(NEXUS_AudioCodec_eMax > codec);

    nerror = NxClient_GetAudioStatus(&statusAudio);
    CHECK_NEXUS_ERROR_GOTO("unable to get audio status", ret, nerror, error);

    switch (statusAudio.spdif.outputMode)
    {
    case NxClient_AudioOutputMode_eAuto:
        BDBG_ERR(("not sure what to do with this spdif output mode"));
        break;
    case NxClient_AudioOutputMode_ePcm:
        spdifType = eSpdifInput_Pcm;
        break;
    case NxClient_AudioOutputMode_ePassthrough:
        spdifType = eSpdifInput_Compressed;
        break;
    case NxClient_AudioOutputMode_eTranscode:
        if (NEXUS_AudioCodec_eAc3 == statusAudio.spdif.outputCodec)
        {
            spdifType = eSpdifInput_EncodeAc3;
        }
        else
        if (NEXUS_AudioCodec_eDts == statusAudio.spdif.outputCodec)
        {
            spdifType = eSpdifInput_EncodeDts;
        }
        else
        {
            spdifType = eSpdifInput_Compressed;
        }
        break;
    case NxClient_AudioOutputMode_eMultichannelPcm:
    case NxClient_AudioOutputMode_eNone:
    case NxClient_AudioOutputMode_eMax:
    default:
        break;
    } /* switch */

error:
    return(spdifType);
} /* getSpdifInput */

eRet CSimpleAudioDecodeNx::setSpdifInput(
        eSpdifInput                              spdifInput,
        NEXUS_SimpleAudioDecoderServerSettings * pSettings
        )
{
    eRet                   ret    = eRet_Ok;
    NEXUS_Error            nerror = NEXUS_SUCCESS;
    NxClient_AudioSettings settings;

    BSTD_UNUSED(pSettings);

    NxClient_GetAudioSettings(&settings);
    settings.spdif.transcodeCodec = NEXUS_AudioCodec_eAc3;

    switch (spdifInput)
    {
    case eSpdifInput_EncodeDts:
        settings.spdif.outputMode     = NxClient_AudioOutputMode_eTranscode;
        settings.spdif.transcodeCodec = NEXUS_AudioCodec_eDts;
        break;

    case eSpdifInput_EncodeAc3:
        settings.spdif.outputMode = NxClient_AudioOutputMode_eTranscode;
        break;

    case eSpdifInput_Compressed:
        settings.spdif.outputMode = NxClient_AudioOutputMode_ePassthrough;
        break;

    case eSpdifInput_Pcm:
    default:
        settings.spdif.outputMode = NxClient_AudioOutputMode_ePcm;
        break;
    } /* switch */

    nerror = NxClient_SetAudioSettings(&settings);
    CHECK_NEXUS_ERROR_GOTO("unable to set spdif output type", nerror, ret, error);

error:
    return(ret);
} /* setSpdifInput */

CStc * CSimpleAudioDecodeNx::close()
{
    CStc * pStc = _pStc;

    if (NULL != _simpleDecoder)
    {
        /* Disconnect the STC */
        setStc(NULL);

        NEXUS_SimpleAudioDecoder_Release(_simpleDecoder);
        _simpleDecoder = NULL;
    }

    if (NULL != _pWidgetEngine)
    {
        _pWidgetEngine->removeCallback(this, CALLBACK_ADECODE_SOURCE_CHANGED);
        _pWidgetEngine = NULL;
    }

    return(pStc);
} /* close */

eRet CSimpleAudioDecodeNx::setAudioProcessing(eAudioProcessing audioProcessing)
{
    eRet        ret    = eRet_Ok;
    NEXUS_Error nerror = NEXUS_SUCCESS;
    NxClient_AudioProcessingSettings settings;

    BDBG_MSG(("setAudioProcessing:%d", audioProcessing));

    NxClient_GetAudioProcessingSettings(&settings);

    /* start with disabled audio processing */
    _audioProcessing                      = eAudioProcessing_None;
    settings.avl.enabled                  = false;
    settings.truVolume.enabled            = false;
    settings.dolby.dolbyVolume258.enabled = false;

    switch (audioProcessing)
    {
    case eAudioProcessing_None:
        nerror = NxClient_SetAudioProcessingSettings(&settings);
        CHECK_NEXUS_ERROR_GOTO("unable to set audio processing settings", ret, nerror, error);
        break;

    case eAudioProcessing_AutoVolumeLevel:
        if (isAutoVolumeLevelSupported())
        {
            BDBG_MSG(("Enable auto volume level"));
            settings.avl.enabled = true;

            nerror = NxClient_SetAudioProcessingSettings(&settings);
            CHECK_NEXUS_ERROR_GOTO("You must start nxserver with the '-avl on' option to enable Auto Volume Limiting", ret, nerror, error);
        }
        else
        {
            ret = eRet_NotAvailable;
            CHECK_ERROR_GOTO("unable to connect to auto volume level", ret, error);
        }
        break;

    case eAudioProcessing_DolbyVolume:
        if (isDolbyVolumeSupported())
        {
            BDBG_MSG(("Enable dolby volume"));
            settings.dolby.dolbyVolume258.enabled = true;

            nerror = NxClient_SetAudioProcessingSettings(&settings);
            CHECK_NEXUS_ERROR_GOTO("You must start nxserver with the -ms11 or -ms12 option to enable Dolby Volume", ret, nerror, error);
        }
        else
        {
            ret = eRet_NotAvailable;
            CHECK_ERROR_GOTO("unable to connect to dolby volume", ret, error);
        }
        break;

    case eAudioProcessing_SrsTruVolume:
        if (isTruVolumeSupported())
        {
            BDBG_MSG(("Enable truvolume"));
            settings.truVolume.enabled = true;

            nerror = NxClient_SetAudioProcessingSettings(&settings);
            CHECK_NEXUS_ERROR_GOTO("You must start nxserver with the '-truVolume' option to enable TruVolume", ret, nerror, error);
        }
        else
        {
            ret = eRet_NotAvailable;
            CHECK_ERROR_GOTO("unable to connect to truvolume", ret, error);
        }
        break;

    default:
        break;
    } /* switch */

    _audioProcessing = audioProcessing;
error:
    return(ret);
} /* setAudioProcessing */

eRet CSimpleAudioDecodeNx::updateConnectSettings(NxClient_ConnectSettings * pSettings)
{
    eRet ret = eRet_Ok;

    pSettings->simpleAudioDecoder.id = getNumber();

#ifndef BDSP_MS12_SUPPORT
    /* no ms12 mixer and single audio decoder so we will need a primer for non-live channels like playback */
    /* set primer if we are connecting to a window that is not fullscreen. */
    pSettings->simpleAudioDecoder.primer = (_pModel->getFullScreenWindowType() == getWindowType()) ? false : true;
#endif

    return(ret);
} /* updateConnectSettings */

#if BDSP_MS12_SUPPORT
/* level: 0-100 */
int CSimpleAudioDecodeNx::getAudioFade()
{
    eRet                           ret     = eRet_Ok;
    int                            level   = -1;
    NEXUS_Error                    nerror  = NEXUS_SUCCESS;
    NEXUS_SimpleAudioDecoderHandle decoder = getSimpleDecoder();
    NEXUS_AudioProcessorStatus     processorStatus;

    nerror = NEXUS_SimpleAudioDecoder_GetProcessorStatus(decoder, NEXUS_SimpleAudioDecoderSelector_ePrimary, NEXUS_AudioPostProcessing_eFade, &processorStatus);
    CHECK_NEXUS_ERROR_GOTO("error getting audio decoder processor status", nerror, ret, error);

    level = processorStatus.status.fade.level;

error:
    return(level);
} /* getAudioFade */
#endif

#if BDSP_MS12_SUPPORT
/* level:0-100, duration:3-60000 msecs - note that setting audio fade before a previous request completes
   will interrupt the previous request.  if you want any previous fade requests to complete first, see
   waitAudioFadeComplete(). */
eRet CSimpleAudioDecodeNx::setAudioFade(unsigned level, unsigned duration, bool bWait)
{
    eRet ret = eRet_Ok;

    NEXUS_SimpleAudioDecoderHandle decoder = getSimpleDecoder();
    NEXUS_Error nerror                     = NEXUS_SUCCESS;

    BDBG_ASSERT(100 >= level);
    BDBG_ASSERT((3 <= duration) && (60000 >= duration));

    NEXUS_SimpleAudioDecoder_GetSettings(decoder, &_simpleDecoderSettings);

    /* sets a starting fade level if specified */
    if (-1 < _fadeStartLevel)
    {
        /* set starting level for fade */
        _simpleDecoderSettings.processorSettings[NEXUS_SimpleAudioDecoderSelector_ePrimary].fade.connected         = true;
        _simpleDecoderSettings.processorSettings[NEXUS_SimpleAudioDecoderSelector_ePrimary].fade.settings.level    = _fadeStartLevel;
        _simpleDecoderSettings.processorSettings[NEXUS_SimpleAudioDecoderSelector_ePrimary].fade.settings.duration = 100;

        BDBG_MSG(("setting starting audio fade level:%d", _fadeStartLevel));

        /* reset start level */
        _fadeStartLevel = -1;

        nerror = NEXUS_SimpleAudioDecoder_SetSettings(decoder, &_simpleDecoderSettings);
        CHECK_NEXUS_ERROR_GOTO("unable to set initial audio fade", ret, nerror, error);

        ret = waitAudioFadeComplete();
        CHECK_ERROR_GOTO("error waiting for initial audio fade to complete", ret, error);
    }

    if ((int)level == getAudioFade())
    {
        BDBG_MSG(("fade level (%d) already set!", level));
        goto done;
    }

    _simpleDecoderSettings.processorSettings[NEXUS_SimpleAudioDecoderSelector_ePrimary].fade.connected         = true;
    _simpleDecoderSettings.processorSettings[NEXUS_SimpleAudioDecoderSelector_ePrimary].fade.settings.level    = level;
    _simpleDecoderSettings.processorSettings[NEXUS_SimpleAudioDecoderSelector_ePrimary].fade.settings.duration = duration;
    nerror = NEXUS_SimpleAudioDecoder_SetSettings(decoder, &_simpleDecoderSettings);
    CHECK_NEXUS_ERROR_GOTO("unable to set audio fade", ret, nerror, error);

    BDBG_MSG(("Setting audio fade level:%d duration:%d", level, duration));

    if (true == bWait)
    {
        ret = waitAudioFadeComplete();
        CHECK_ERROR_GOTO("error waiting for audio fade to complete", ret, error);
    }

done:
error:
    return(ret);
} /* setAudioFade */
#endif

#if BDSP_MS12_SUPPORT
bool CSimpleAudioDecodeNx::isAudioFadePending()
{
    eRet                           ret                  = eRet_Timeout;
    NEXUS_SimpleAudioDecoderHandle decoder              = getSimpleDecoder();
    NEXUS_Error                    nerror               = NEXUS_SUCCESS;
    bool                           bPending             = false;
    NEXUS_AudioProcessorStatus     processorStatus;

    nerror = NEXUS_SimpleAudioDecoder_GetProcessorStatus(decoder, NEXUS_SimpleAudioDecoderSelector_ePrimary, NEXUS_AudioPostProcessing_eFade, &processorStatus);
    if (NEXUS_SUCCESS != nerror)
    {
        goto done;
    }

    if (true == processorStatus.status.fade.active)
    {
        bPending = true;
    }

done:
    return(bPending);
}
#endif

#if BDSP_MS12_SUPPORT
/* audio fade requests are not queued.  if a fade request occurs before a previous request
   completes, it will interrupt the previous request.  this method will allow you to wait
   until a previous fade request completes before starting a new one (if that's what you want to do).
   audio decode should be started before calling this function.
*/
eRet CSimpleAudioDecodeNx::waitAudioFadeComplete()
{
    eRet                           ret                  = eRet_Ok;
    NEXUS_Error                    nerror               = NEXUS_SUCCESS;
    NEXUS_SimpleAudioDecoderHandle decoder              = getSimpleDecoder();
    NEXUS_AudioProcessorStatus     processorStatus;

    if (false == isStarted())
    {
        return(eRet_Ok);
    }

    do
    {
        BKNI_Sleep(100);
        nerror = NEXUS_SimpleAudioDecoder_GetProcessorStatus(decoder, NEXUS_SimpleAudioDecoderSelector_ePrimary, NEXUS_AudioPostProcessing_eFade, &processorStatus);

        if (!nerror && processorStatus.status.fade.active)
        {
            BDBG_MSG(("%s nerror:%d type:%d active:%d remain:%d level:%d%%",
                __FUNCTION__, nerror, processorStatus.type, processorStatus.status.fade.active,
                processorStatus.status.fade.remaining, processorStatus.status.fade.level));
        }

        if (nerror)
        {
            continue;
        }
    }
    while (nerror != NEXUS_SUCCESS || processorStatus.type == NEXUS_AudioPostProcessing_eMax || processorStatus.status.fade.active);

    return(ret);
}
#endif