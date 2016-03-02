/***************************************************************************
 * (c) 2002-2016 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its
 * licensors, and may only be used, duplicated, modified or distributed pursuant
 * to the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and Broadcom
 * expressly reserves all rights in and to the Software and all intellectual
 * property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 *    constitutes the valuable trade secrets of Broadcom, and you shall use all
 *    reasonable efforts to protect the confidentiality thereof, and to use
 *    this information only in connection with your use of Broadcom integrated
 *    circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
 *    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
 *    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
 *    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
 *    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
 *    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
 *    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
 *    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
 *    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
 *    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
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
        const uint16_t   number,
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
        BDBG_ERR(("TTTTTTTTTTTTTt not sure what to do with this hdmi output mode"));
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
     /* set primer if we are connecting to a window that is not fullscreen. */
     pSettings->simpleAudioDecoder.primer = (_pModel->getFullScreenWindowType() == getWindowType()) ? false : true;

     return(ret);
} /* updateConnectSettings */
