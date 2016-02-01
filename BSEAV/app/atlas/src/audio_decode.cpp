/***************************************************************************
 * (c) 2002-2015 Broadcom Corporation
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
#include "audio_decode.h"
#include "audio_processing.h"
#include "atlas_os.h"

#define CALLBACK_ADECODE_SOURCE_CHANGED  "CallbackAudioDecodeSourceChanged"

BDBG_MODULE(atlas_audio_decode);

/* bwin io callback that is triggered when it is safe to handle callbacks */
static void bwinAudioDecodeSourceChangedCallback(
        void *       pObject,
        const char * strCallback
        )
{
    eRet           ret          = eRet_Ok;
    CAudioDecode * pAudioDecode = (CAudioDecode *)pObject;

    BDBG_ASSERT(NULL != pAudioDecode);
    BSTD_UNUSED(strCallback);

    if (true == pAudioDecode->isSourceChanged())
    {
        ret = pAudioDecode->notifyObservers(eNotify_AudioSourceChanged, pAudioDecode);
        CHECK_ERROR("error notifying observers", ret);
    }
} /* bwinAudioDecodeSourceChangedCallback */

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

static void NexusAudioDecodeSourceChangedCallback(
        void * context,
        int    param
        )
{
    CAudioDecode * pAudioDecode = (CAudioDecode *)context;

    BSTD_UNUSED(param);
    BDBG_ASSERT(NULL != pAudioDecode);

    CWidgetEngine * pWidgetEngine = pAudioDecode->getWidgetEngine();
    pAudioDecode->setSourceChanged(true);

    /* sync with bwin loop */
    if (NULL != pWidgetEngine)
    {
        pWidgetEngine->syncCallback(pAudioDecode, CALLBACK_ADECODE_SOURCE_CHANGED);
    }
} /* NEXUSAudioDecodeSourceChangedCallback */

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

CAudioDecode::CAudioDecode(
        const char *     name,
        const uint16_t   number,
        CConfiguration * pCfg
        ) :
    CResource(name, number, eBoardResource_decodeAudio, pCfg),
    _decoder(NULL),
    _sourceChangedCallbackHandle(NULL),
    _pStc(NULL),
    _pPid(NULL),
    _pWidgetEngine(NULL),
    _started(false),
    _sourceChanged(false)
#if B_HAS_EXTERNAL_ANALOG
    , _i2sInput(NULL)
#endif
#if NEXUS_NUM_AUDIO_CAPTURES
    , _audioCapture(NULL)
#endif
{
    eRet ret = eRet_Ok;

    memset(&_decoderSettings, 0, sizeof(NEXUS_AudioDecoderSettings));
    memset(&_simpleDecoderSettings, 0, sizeof(NEXUS_SimpleAudioDecoderSettings));

    BDBG_ASSERT(eRet_Ok == ret);
}

CAudioDecode::~CAudioDecode()
{
    if (_sourceChangedCallbackHandle)
    {
        B_SchedulerCallback_Destroy(_sourceChangedCallbackHandle);
        _sourceChangedCallbackHandle = NULL;
    }
}

eRet CAudioDecode::open(
        CWidgetEngine * pWidgetEngine,
        CStc *          pStc
        )
{
    eRet ret      = eRet_Ok;
    int  fifoSize = 0;
    NEXUS_AudioDecoderOpenSettings settings;

    BDBG_ASSERT(NULL != pStc);

    if (true == isOpened())
    {
        ret = eRet_InvalidState;
        CHECK_ERROR_GOTO("Audio decoder is already opened.", ret, error);
    }

    _pWidgetEngine = pWidgetEngine;
    if (NULL != _pWidgetEngine)
    {
        _pWidgetEngine->addCallback(this, CALLBACK_ADECODE_SOURCE_CHANGED, bwinAudioDecodeSourceChangedCallback);
    }

    /* save stc for start() */
    _pStc = pStc;

    NEXUS_AudioDecoder_GetDefaultOpenSettings(&settings);

    {
        int multiChFormat = GET_INT(_pCfg, AUDIO_DECODER_MULTICH_FORMAT);
        if ((0 < multiChFormat) && (NEXUS_AudioMultichannelFormat_eMax > multiChFormat))
        {
            settings.multichannelFormat = (NEXUS_AudioMultichannelFormat)multiChFormat;
        }
    }

    /* set up optional independent output delay */
    if ((0 < GET_INT(_pCfg, SPDIF_OUTPUT_DELAY)) ||
        (0 < GET_INT(_pCfg, HDMI_OUTPUT_DELAY)) ||
        (0 < GET_INT(_pCfg, DAC_OUTPUT_DELAY)))
    {
        settings.independentDelay = true;
    }

    /* change fifo size if specified */
    fifoSize = GET_INT(_pCfg, AUDIO_DECODER_FIFO_SIZE);
    if (0 < fifoSize)
    {
        settings.fifoSize = fifoSize * 1000; /* dtt - should this be 1024? */
        BDBG_WRN(("****************************************"));
        BDBG_WRN(("* Changing Audio FIFO size to %d bytes *", settings.fifoSize));
        BDBG_WRN(("****************************************"));
    }

#if 0 /*PLAYBACK_IP_SUPPORT*/
    fifoSize = GET_INT(_pCfg, AUDIO_DECODER_IP_FIFO_SIZE);
    if (0 < fifoSize)
    {
        settings.fifoSize = fifoSize; /* dtt - should this be 5*512*1024 */
        BDBG_MSG(("*******************************************************************"));
        BDBG_MSG(("* PLAYBACK IP Jitter support Changing Audio FIFO size to %d bytes *", settings.fifoSize));
        BDBG_MSG(("*******************************************************************"));
    }
#endif /* if 0 */

    _decoder = NEXUS_AudioDecoder_Open(_number, &settings);
    CHECK_PTR_ERROR_GOTO("Nexus audio decoder open failed!", _decoder, ret, eRet_NotAvailable, error);

    /* set audio decoder settings */
    {
        NEXUS_Error nError = NEXUS_SUCCESS;

        NEXUS_AudioDecoder_GetSettings(_decoder, &_decoderSettings);
        _decoderSettings.sourceChanged.callback = NexusAudioDecodeSourceChangedCallback;
        _decoderSettings.sourceChanged.context  = this;
        _decoderSettings.sourceChanged.param    = 0;
        nError = NEXUS_AudioDecoder_SetSettings(_decoder, &_decoderSettings);
        CHECK_NEXUS_ERROR_GOTO("Error applying audio decoder settings.", ret, nError, error);
    }

    if (GET_BOOL(_pCfg, AUDIO_CAPTURE) || GET_BOOL(_pCfg, AUDIO_CAPTURE_COMPRESSED))
    {
        /* dtt - open audio capture */
    }

#if B_HAS_EXTERNAL_ANALOG
    _i2sInput = NEXUS_I2sInput_Open(0, NULL);
    CHECK_PTR_ERROR_GOTO("i2s input open failed.", _i2sInput, ret, eRet_NotAvailable, error);
#endif

    /* dtt - open passthru? see bsettop_decode.c */
error:
    return(ret);
} /* open */

#include "nexus_audio_input.h"

CStc * CAudioDecode::close()
{
    CStc * pStc = NULL;

    if (GET_BOOL(_pCfg, AUDIO_CAPTURE) || GET_BOOL(_pCfg, AUDIO_CAPTURE_COMPRESSED))
    {
        /* dtt - close audio capture */
    }

#if B_HAS_EXTERNAL_ANALOG
    if (_i2sInput)
    {
        NEXUS_AudioInput_Shutdown(NEXUS_I2sInput_GetConnector(_i2sInput));
        NEXUS_I2sInput_Close(_i2sInput);
        _i2sInput = NULL;
    }
#endif /* if B_HAS_EXTERNAL_ANALOG */

    /* dtt - close passthru? see bsettop_decode.c */

    if (_decoder)
    {
        NEXUS_AudioInput_Shutdown(NEXUS_AudioDecoder_GetConnector(_decoder, NEXUS_AudioDecoderConnectorType_eStereo));
        NEXUS_AudioInput_Shutdown(NEXUS_AudioDecoder_GetConnector(_decoder, NEXUS_AudioDecoderConnectorType_eCompressed));
        NEXUS_AudioInput_Shutdown(NEXUS_AudioDecoder_GetConnector(_decoder, NEXUS_AudioDecoderConnectorType_eMultichannel));

        NEXUS_AudioDecoder_Close(_decoder);

        /* return stc */
        pStc     = _pStc;
        _pStc    = NULL;
        _decoder = NULL;
    }

    if (NULL != _pWidgetEngine)
    {
        _pWidgetEngine->removeCallback(this, CALLBACK_ADECODE_SOURCE_CHANGED);
        _pWidgetEngine = NULL;
    }

    return(pStc);
} /* close */

NEXUS_AudioInput CAudioDecode::getConnector(NEXUS_AudioConnectorType type)
{
    NEXUS_AudioInput nexusInput = NULL;

    BDBG_ASSERT(NEXUS_AudioConnectorType_eMax > type);

    if (NULL != _decoder)
    {
        nexusInput = NEXUS_AudioDecoder_GetConnector(getDecoder(), type);
    }

    return(nexusInput);
}

eRet CAudioDecode::getStatus(NEXUS_AudioDecoderStatus * pStatus)
{
    eRet        ret    = eRet_Ok;
    NEXUS_Error nerror = NEXUS_SUCCESS;

    BDBG_ASSERT(NULL != pStatus);

    nerror = NEXUS_AudioDecoder_GetStatus(getDecoder(), pStatus);
    CHECK_NEXUS_ERROR_GOTO("error getting audio decoder status", ret, nerror, error);

error:
    return(ret);
}

bool CAudioDecode::isCodecSupported(NEXUS_AudioCodec codec)
{
    bool bSupported = false;

    NEXUS_AudioDecoder_IsCodecSupported(getDecoder(), codec, &bSupported);
    return(bSupported);
}

/*** simple audio decoder class */

#include "nexus_simple_audio_decoder_server.h"
#include "board.h"

CSimpleAudioDecode::CSimpleAudioDecode(
        const char *     name,
        const uint16_t   number,
        CConfiguration * pCfg
        ) :
    CAudioDecode(name, number, pCfg),
    _simpleDecoder(NULL),
    _encodeAc3(NULL),
    _encodeDts(NULL),
    _pAutoVolumeLevel(NULL),
    _pDolbyVolume(NULL),
    _pTruVolume(NULL),
    _pBoardResources(NULL),
    _pSpdif(NULL),
    _pHdmi(NULL),
    _pDac(NULL),
    _pRFM(NULL),
    _pDummy(NULL),
    _numSpdif(0),
    _numHdmi(0),
    _resourceId(NULL),
    _downmix(eAudioDownmix_None),
    _downmixAc3(eAudioDownmix_Ac3Auto),
    _downmixDts(eAudioDownmix_DtsAuto),
    _downmixAac(eAudioDownmix_AacMatrix),
    _dualMono(eAudioDualMono_Left),
    _dolbyDRC(eDolbyDRC_None),
    _dolbyDialogNorm(false),
    _audioProcessing(eAudioProcessing_None),
    _stereoInput(NULL),
    _bEncodeConnectedDts(false),
    _bEncodeConnectedAc3(false)
{
    eRet ret = eRet_Ok;

    /* manually set type since base class CAudioDecoder constructor will default to
     * regular audio decoder type */
    setType(eBoardResource_simpleDecodeAudio);

    for (int i = 0; i < 2; i++)
    {
        _pDecoders[i] = NULL;
    }

    BDBG_ASSERT(eRet_Ok == ret);
}

CSimpleAudioDecode::~CSimpleAudioDecode()
{
    close();
}

eRet CSimpleAudioDecode::open(
        CWidgetEngine * pWidgetEngine,
        CStc *          pStc
        )
{
    eRet        ret    = eRet_Ok;
    NEXUS_Error nError = NEXUS_SUCCESS;
    int         i      = 0;

    NEXUS_SimpleAudioDecoderServerSettings settings;

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

    for (i = 0; i < 2; i++)
    {
        _pDecoders[i] = (CAudioDecode *)_pBoardResources->checkoutResource(_resourceId, eBoardResource_decodeAudio);
        if (NULL != _pDecoders[i])
        {
            ret = _pDecoders[i]->open(NULL, pStc);
            CHECK_ERROR_GOTO("Audio decode failed to open", ret, error);
        }
    }

    NEXUS_SimpleAudioDecoder_GetDefaultServerSettings(&settings);
    settings.primary   = _pDecoders[0]->getDecoder();
    settings.secondary = _pDecoders[1]->getDecoder();

    /* if supported daisy chain AVL, dolby vol, and truvolume after decoder */
    _stereoInput = _pDecoders[0]->getConnector(NEXUS_AudioConnectorType_eStereo);
    {
        NEXUS_AudioCapabilities capabilities;
        NEXUS_GetAudioCapabilities(&capabilities);

        if (true == capabilities.dsp.encoder)
        {
            if (true == capabilities.dsp.codecs[NEXUS_AudioCodec_eAc3].encode)
            {
                _encodeAc3 = NEXUS_Ac3Encode_Open(NULL);
                CHECK_PTR_ERROR_GOTO("unable to open ac-3 encode", _encodeAc3, ret, eRet_OutOfMemory, error);
            }

            if (true == capabilities.dsp.codecs[NEXUS_AudioCodec_eDts].encode)
            {
                _encodeDts = NEXUS_DtsEncode_Open(NULL);
                CHECK_PTR_ERROR_GOTO("unable to open dts encode", _encodeDts, ret, eRet_OutOfMemory, error);
            }
        }

        if (true == capabilities.dsp.autoVolumeLevel)
        {
            BDBG_MSG(("auto volume leveling supported"));
            _pAutoVolumeLevel = new CAutoVolumeLevel(_pCfg);
            CHECK_PTR_ERROR_GOTO("unable to open auto volume leveling", _pAutoVolumeLevel, ret, eRet_OutOfMemory, error);

            ret = _pAutoVolumeLevel->open();
            CHECK_ERROR_GOTO("unable to open auto volume level", ret, error);

            ret = _pAutoVolumeLevel->connect(_stereoInput);
            CHECK_ERROR_GOTO("unable to connect input to auto volume level", ret, error);

            _stereoInput = _pAutoVolumeLevel->getConnector();
        }

        if (true == capabilities.dsp.dolbyVolume258)
        {
            BDBG_MSG(("dolby volume supported"));
            _pDolbyVolume = new CDolbyVolume(_pCfg);
            CHECK_PTR_ERROR_GOTO("unable to open dolby volume", _pDolbyVolume, ret, eRet_OutOfMemory, error);

            ret = _pDolbyVolume->open();
            CHECK_ERROR_GOTO("unable to open dolby volume", ret, error);

            ret = _pDolbyVolume->connect(_stereoInput);
            CHECK_ERROR_GOTO("unable to connect input to dolby volume", ret, error);

            _stereoInput = _pDolbyVolume->getConnector();
        }

        if (true == capabilities.dsp.truVolume)
        {
            BDBG_MSG(("srs tru volume supported"));
            _pTruVolume = new CTruVolume(_pCfg);
            CHECK_PTR_ERROR_GOTO("unable to open truvolume", _pTruVolume, ret, eRet_OutOfMemory, error);

            ret = _pTruVolume->open();
            CHECK_ERROR_GOTO("unable to open truvolume", ret, error);

            ret = _pTruVolume->connect(_stereoInput);
            CHECK_ERROR_GOTO("unable to connect input to truvolume", ret, error);

            _stereoInput = _pTruVolume->getConnector();
        }
    }

    /* if a audio spdif resource has been assigned, set up audio output */
    if (NULL != _pSpdif)
    {
        /* default spdif to stereo output */
        setSpdifInput(eSpdifInput_Pcm, &settings);

        /* assign spdif outputs for simple audio decoder to use */
        settings.spdif.outputs[0] = _pSpdif->getOutput();
    }

    /* if a video hdmi resource has been assigned, set up audio output */
    if (NULL != _pHdmi)
    {
        /* default hdmi to stereo output */
        setHdmiInput(eHdmiAudioInput_Pcm, &settings);

        /* assign Hdmi outputs for simple audio decoder to use */
        settings.hdmi.outputs[0] = _pHdmi->getOutput();
    }
    settings.stcIndex = getNumber();

    _simpleDecoder = NEXUS_SimpleAudioDecoder_Create(_number, &settings);
    CHECK_PTR_ERROR_GOTO("unable to create a simple audio decoder", _simpleDecoder, ret, eRet_OutOfMemory, error);

    ret = setStc(pStc);
    CHECK_ERROR_GOTO("SetStcChannel simple audio decoder failed!", ret, error);

    /* if an audio dac resource has been assigned, set of audio output */
    if (NULL != _pDac)
    {
        ret = _pDac->connect(_stereoInput);
        CHECK_ERROR_GOTO("unable to connect input to dac output", ret, error);
    }

    if (NULL != _pRFM)
    {
        ret = _pRFM->connect(_stereoInput);
        CHECK_ERROR_GOTO("unable to connect RFM audio output", ret, error);
    }

    /* if you want to attach a Dummy output for transcode */
    if (NULL != _pDummy)
    {
        ret = _pDummy->connect(_stereoInput);
        CHECK_ERROR_GOTO("unable to connect dummy output", ret, error);
    }

    /* set audio decoder settings */
    {
        NEXUS_SimpleAudioDecoder_GetSettings(_simpleDecoder, &_simpleDecoderSettings);
        _simpleDecoderSettings.primary.sourceChanged.callback   = NexusSimpleAudioDecodeSourceChangedCallback;
        _simpleDecoderSettings.primary.sourceChanged.context    = this;
        _simpleDecoderSettings.primary.sourceChanged.param      = 0;
        _simpleDecoderSettings.secondary.sourceChanged.callback = NexusSimpleAudioDecodeSourceChangedCallback;
        _simpleDecoderSettings.secondary.sourceChanged.context  = this;
        _simpleDecoderSettings.secondary.sourceChanged.param    = 0;
        nError = NEXUS_SimpleAudioDecoder_SetSettings(_simpleDecoder, &_simpleDecoderSettings);
        CHECK_NEXUS_ERROR_GOTO("Error applying simple audio decoder settings.", ret, nError, error);
    }
error:
    return(ret);
} /* open */

void CSimpleAudioDecode::getSettings(NEXUS_SimpleAudioDecoderServerSettings * pSettings)
{
    BDBG_ASSERT(NULL != _simpleDecoder);

    NEXUS_SimpleAudioDecoder_GetServerSettings(_simpleDecoder, pSettings);
}

eRet CSimpleAudioDecode::setSettings(NEXUS_SimpleAudioDecoderServerSettings * pSettings)
{
    NEXUS_Error nerror = NEXUS_SUCCESS;
    eRet        ret    = eRet_Ok;

    BDBG_ASSERT(NULL != _simpleDecoder);

    nerror = NEXUS_SimpleAudioDecoder_SetServerSettings(_simpleDecoder, pSettings);
    CHECK_NEXUS_ERROR_GOTO("unable to set simple audio decoder server settings", ret, nerror, error);

error:
    return(ret);
}

/* connect/disconnect DTS encoder to audio decoder */
void CSimpleAudioDecode::connectEncodeDts(bool bConnect)
{
    NEXUS_Error nError = NEXUS_SUCCESS;
    eRet        ret    = eRet_Ok;

    if (NULL == _encodeDts)
    {
        return;
    }

    if (_bEncodeConnectedDts == bConnect)
    {
        return;
    }

    if (true == bConnect)
    {
        /* connect dts encoder to decoder */
        nError = NEXUS_DtsEncode_AddInput(_encodeDts, _pDecoders[0]->getConnector(NEXUS_AudioConnectorType_eMultichannel));
        CHECK_NEXUS_ERROR_GOTO("unable to add dts encode input", ret, nError, error);
    }
    else
    {
        setSpdifInput(eSpdifInput_Pcm);
        setHdmiInput(eHdmiAudioInput_Pcm);
        NEXUS_DtsEncode_RemoveInput(_encodeDts, _pDecoders[0]->getConnector(NEXUS_AudioConnectorType_eMultichannel));
    }

    _bEncodeConnectedDts = bConnect;

error:
    return;
} /* connectEncodeDts */

/* connect/disconnect AC-3 encoder to audio decoder */
void CSimpleAudioDecode::connectEncodeAc3(bool bConnect)
{
    NEXUS_Error nError = NEXUS_SUCCESS;
    eRet        ret    = eRet_Ok;

    if (NULL == _encodeAc3)
    {
        return;
    }

    if (_bEncodeConnectedAc3 == bConnect)
    {
        return;
    }

    if (true == bConnect)
    {
        /* connect ac3 encoder to decoder */
        nError = NEXUS_Ac3Encode_AddInput(_encodeAc3, _pDecoders[0]->getConnector(NEXUS_AudioConnectorType_eMultichannel));
        CHECK_NEXUS_ERROR_GOTO("unable to add Ac3 encode input", ret, nError, error);
    }
    else
    {
        setSpdifInput(eSpdifInput_Pcm);
        setHdmiInput(eHdmiAudioInput_Pcm);
        NEXUS_Ac3Encode_RemoveInput(_encodeAc3, _pDecoders[0]->getConnector(NEXUS_AudioConnectorType_eMultichannel));
    }

    _bEncodeConnectedAc3 = bConnect;

error:
    return;
} /* connectEncodeAc3 */

/* make sure the current encodeAc3 and encodeDts settings are compatible with the given audio codec */
void CSimpleAudioDecode::verifyEncode(NEXUS_AudioCodec codec)
{
    bool bEnable = false;

    switch (codec)
    {
    case NEXUS_AudioCodec_eAac:
    case NEXUS_AudioCodec_eAacLoas:
    case NEXUS_AudioCodec_eAacPlus:
    case NEXUS_AudioCodec_eAacPlusAdts:
        bEnable = true;
        break;

    default:
        break;
    } /* switch */

    if (NULL != _encodeAc3)
    {
        connectEncodeAc3(bEnable);
    }

    if (NULL != _encodeDts)
    {
        connectEncodeDts(bEnable);
    }
} /* verifyEncode */

eRet CSimpleAudioDecode::setAudioProcessing(eAudioProcessing audioProcessing)
{
    eRet ret = eRet_Ok;

    BDBG_MSG(("setAudioProcessing:%d", audioProcessing));

    /* start with disabled audio processing */
    if (NULL != _pAutoVolumeLevel) { _pAutoVolumeLevel->enable(false); }
    if (NULL != _pDolbyVolume) { _pDolbyVolume->enable(false); }
    if (NULL != _pTruVolume) { _pTruVolume->enable(false); }
    _audioProcessing = eAudioProcessing_None;

    switch (audioProcessing)
    {
    case eAudioProcessing_None:
        _audioProcessing = audioProcessing;
        break;

    case eAudioProcessing_AutoVolumeLevel:
        if (NULL != _pAutoVolumeLevel)
        {
            BDBG_MSG(("Enable auto volume level"));
            _pAutoVolumeLevel->enable(true);
            _audioProcessing = audioProcessing;
        }
        else
        {
            ret = eRet_NotAvailable;
            CHECK_ERROR_GOTO("unable to connect to auto volume level", ret, error);
        }
        break;

    case eAudioProcessing_DolbyVolume:
        if (NULL != _pDolbyVolume)
        {
            BDBG_MSG(("Enable dolby volume"));
            _pDolbyVolume->enable(true);
            _audioProcessing = audioProcessing;
        }
        else
        {
            ret = eRet_NotAvailable;
            CHECK_ERROR_GOTO("unable to connect to dolby volume", ret, error);
        }
        break;

    case eAudioProcessing_SrsTruVolume:
        if (NULL != _pTruVolume)
        {
            BDBG_MSG(("Enable truvolume"));
            _pTruVolume->enable(true);
            _audioProcessing = audioProcessing;
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

error:
    return(ret);
} /* setAudioProcessing */

eRet CSimpleAudioDecode::setSpdifInput(
        eSpdifInput                              spdifInput,
        NEXUS_SimpleAudioDecoderServerSettings * pSettings
        )
{
    NEXUS_SimpleAudioDecoderServerSettings   settings;
    NEXUS_SimpleAudioDecoderServerSettings * pSettingsTemp;
    NEXUS_AudioConnectorType                 spdifType;
    int  decoder = 0;
    eRet ret     = eRet_Ok;

    BDBG_MSG(("setSpdifInput:%d", spdifInput));

    if ((eSpdifInput_EncodeDts == spdifInput) && (NULL == _encodeDts))
    {
        ret = eRet_NotSupported;
        CHECK_ERROR_GOTO("DTS encoder is not supported on this platform", ret, error);
    }

    if ((eSpdifInput_EncodeAc3 == spdifInput) && (NULL == _encodeAc3))
    {
        ret = eRet_NotSupported;
        CHECK_ERROR_GOTO("AC-3 encoder is not supported on this platform", ret, error);
    }

    if (NULL == pSettings)
    {
        pSettingsTemp = &settings;
        getSettings(pSettingsTemp);
    }
    else
    {
        pSettingsTemp = pSettings;
    }

    switch (spdifInput)
    {
    case eSpdifInput_EncodeDts:
    case eSpdifInput_EncodeAc3:
        spdifType = NEXUS_AudioConnectorType_eMultichannel;
        decoder   = 0;
        break;

    case eSpdifInput_Compressed:
        spdifType = NEXUS_AudioConnectorType_eCompressed;
        decoder   = 1;
        break;

    case eSpdifInput_Pcm:
    default:
        spdifType = NEXUS_AudioConnectorType_eStereo;
        decoder   = 0;
        break;
    } /* switch */

    for (int codec = 0; codec < NEXUS_AudioCodec_eMax; codec++)
    {
        if ((NEXUS_AudioCodec_eAacAdts == codec) ||
            (NEXUS_AudioCodec_eAacLoas == codec) ||
            (NEXUS_AudioCodec_eAacPlusLoas == codec) ||
            (NEXUS_AudioCodec_eAacPlusAdts == codec))
        {
            if (eSpdifInput_EncodeDts == spdifInput)
            {
                /* encode dts */
                connectEncodeDts(true);
                pSettingsTemp->spdif.input[codec] = NEXUS_DtsEncode_GetConnector(_encodeDts);
                continue;
            }
            else
            if (eSpdifInput_EncodeAc3 == spdifInput)
            {
                /* encode ac3 */
                connectEncodeAc3(true);
                pSettingsTemp->spdif.input[codec] = NEXUS_Ac3Encode_GetConnector(_encodeAc3);
                continue;
            }
        }

        if (eSpdifInput_Compressed == spdifInput)
        {
            /* compressed or pcm */
            switch (codec)
            {
            /* only support ac3 and dts for spdif compressed */
            case NEXUS_AudioCodec_eAc3:
            case NEXUS_AudioCodec_eDts:
            case NEXUS_AudioCodec_eDtsLegacy:
            case NEXUS_AudioCodec_eDtsHd:
                pSettingsTemp->spdif.input[codec] = _pDecoders[decoder]->getConnector(spdifType);
                break;
            case NEXUS_AudioCodec_eAc3Plus: /* Always set this to Decoder 0 for auto conversion to ac3 if compressed */
                pSettingsTemp->spdif.input[codec] = _pDecoders[0]->getConnector(spdifType);
                break;
            case NEXUS_AudioCodec_eUnknown:
            default:
                pSettingsTemp->spdif.input[codec] = _stereoInput;
                break;
            } /* switch */
        }
        else /*(eSpdifInput_Pcm == spdifInput)*/
        {
            pSettingsTemp->spdif.input[codec] = _stereoInput;
        }
    }

    if (NULL == pSettings)
    {
        /* only set setting if no settings given */
        ret = setSettings(pSettingsTemp);
        CHECK_ERROR_GOTO("set simple audio settings failed", ret, error);
    }

error:
    return(ret);
} /* setSpdifInput */

eSpdifInput CSimpleAudioDecode::getSpdifInput(NEXUS_AudioCodec codec)
{
    NEXUS_SimpleAudioDecoderServerSettings settings;
    eSpdifInput spdifType = eSpdifInput_None;

    BDBG_ASSERT(NEXUS_AudioCodec_eMax > codec);

    getSettings(&settings);

    for (int i = 0; i < 2; i++)
    {
        if (_pDecoders[i] && (settings.spdif.input[codec] == _pDecoders[i]->getConnector(NEXUS_AudioConnectorType_eCompressed)))
        {
            spdifType = eSpdifInput_Compressed;
        }
        else
        if (settings.spdif.input[codec] == _stereoInput)
        {
            spdifType = eSpdifInput_Pcm;
        }
        else
        if ((NULL != _encodeDts) && (settings.spdif.input[codec] == NEXUS_DtsEncode_GetConnector(_encodeDts)))
        {
            spdifType = eSpdifInput_EncodeDts;
        }
        else
        if ((NULL != _encodeAc3) && (settings.spdif.input[codec] == NEXUS_Ac3Encode_GetConnector(_encodeAc3)))
        {
            spdifType = eSpdifInput_EncodeAc3;
        }
    }

    return(spdifType);
} /* getSpdifInput */

eRet CSimpleAudioDecode::setHdmiInput(
        eHdmiAudioInput                          hdmiInput,
        NEXUS_SimpleAudioDecoderServerSettings * pSettings
        )
{
    NEXUS_SimpleAudioDecoderServerSettings   settings;
    NEXUS_SimpleAudioDecoderServerSettings * pSettingsTemp;
    NEXUS_AudioConnectorType                 hdmiType;
    eRet ret     = eRet_Ok;
    int  decoder = 0;

    BDBG_MSG(("setHdmiInput:%d", hdmiInput));

    if ((eHdmiAudioInput_EncodeDts == hdmiInput) && (NULL == _encodeDts))
    {
        ret = eRet_NotSupported;
        CHECK_ERROR_GOTO("DTS encoder is not supported on this platform", ret, error);
    }

    if ((eHdmiAudioInput_EncodeAc3 == hdmiInput) && (NULL == _encodeAc3))
    {
        ret = eRet_NotSupported;
        CHECK_ERROR_GOTO("AC-3 encoder is not supported on this platform", ret, error);
    }

    pSettingsTemp = (NULL != pSettings) ? pSettings : &settings;

    if (NULL == pSettings)
    {
        pSettingsTemp = &settings;
        getSettings(pSettingsTemp);
    }
    else
    {
        pSettingsTemp = pSettings;
    }

    switch (hdmiInput)
    {
    case eHdmiAudioInput_Multichannel:
        hdmiType = NEXUS_AudioConnectorType_eMultichannel;
        decoder  = 0;
        break;

    case eHdmiAudioInput_EncodeDts:
    case eHdmiAudioInput_EncodeAc3:
        hdmiType = NEXUS_AudioConnectorType_eMultichannel;
        decoder  = 0;
        break;

    case eHdmiAudioInput_Compressed:
        hdmiType = NEXUS_AudioConnectorType_eCompressed;
        decoder  = 1;
        break;

    case eHdmiAudioInput_Pcm:
    default:
        hdmiType = NEXUS_AudioConnectorType_eStereo;
        decoder  = 0;
        break;
    } /* switch */

    for (int codec = 0; codec < NEXUS_AudioCodec_eMax; codec++)
    {
        if ((NEXUS_AudioCodec_eAacAdts == codec) ||
            (NEXUS_AudioCodec_eAacLoas == codec) ||
            (NEXUS_AudioCodec_eAacPlusLoas == codec) ||
            (NEXUS_AudioCodec_eAacPlusAdts == codec))
        {
            if (eHdmiAudioInput_EncodeDts == hdmiInput)
            {
                /* we'll assume DTS is a valid codec for hdmi receiver */
                connectEncodeDts(true);
                pSettingsTemp->hdmi.input[codec] = NEXUS_DtsEncode_GetConnector(_encodeDts);
                continue;
            }
            else
            if (eHdmiAudioInput_EncodeAc3 == hdmiInput)
            {
                /* we'll assume AC-3 is a valid codec for hdmi receiver */
                connectEncodeAc3(true);
                pSettingsTemp->hdmi.input[codec] = NEXUS_Ac3Encode_GetConnector(_encodeAc3);
                continue;
            }
        }

        if (NEXUS_AudioConnectorType_eStereo == hdmiType)
        {
            /* stereo requested */
            pSettingsTemp->hdmi.input[codec] = _stereoInput;
        }
        else
        if (true == _pHdmi->isValidAudioCodec((NEXUS_AudioCodec)codec))
        {
            /* codec supported and stereo NOT requested */
            pSettingsTemp->hdmi.input[codec] = _pDecoders[decoder]->getConnector(hdmiType);
        }
        else
        {
            /* codec NOT supported and stereo NOT requested */
            pSettingsTemp->hdmi.input[codec] = _stereoInput;

            /* check a few special cases where we have alternatives to stereo */
            switch (codec)
            {
            case NEXUS_AudioCodec_eAac:
            case NEXUS_AudioCodec_eAacLoas:
            case NEXUS_AudioCodec_eAacPlus:
            case NEXUS_AudioCodec_eAacPlusAdts:
            case NEXUS_AudioCodec_eDra:
                if (6 <= _pHdmi->getMaxAudioPcmChannels())
                {
                    pSettingsTemp->hdmi.input[codec] = _pDecoders[decoder]->getConnector(NEXUS_AudioConnectorType_eMultichannel);
                }
                break;
            case NEXUS_AudioCodec_eAc3Plus:
                if (false == _pHdmi->isValidAudioCodec(NEXUS_AudioCodec_eAc3Plus))
                {
                    /* edid says Ac3+ is UNsupported */
                    if (true == _pHdmi->isValidAudioCodec(NEXUS_AudioCodec_eAc3))
                    {
                        /* edid says regular Ac3 is supported - use decoder0 so it can convert to ac3 */
                        pSettingsTemp->hdmi.input[codec] = _pDecoders[0]->getConnector(hdmiType);
                        BDBG_WRN(("ac3+ audio codec is UNsupported by HDMI receiver - convert to ac3"));
                    }
                    else
                    if (6 <= _pHdmi->getMaxAudioPcmChannels())
                    {
                        pSettingsTemp->hdmi.input[codec] = _pDecoders[decoder]->getConnector(NEXUS_AudioConnectorType_eMultichannel);
                        BDBG_WRN(("ac3+ and ac3 codecs are UNsupported by HDMI receiver - use multichannel pcm"));
                    }
                    else
                    {
                        BDBG_WRN(("ac3+, ac3, and 5.1ch multichannel pcm is unsupported by HDMI receiver - default to stereo"));
                    }
                }
                break;
            case NEXUS_AudioCodec_eWmaPro:
                if (6 <= _pHdmi->getMaxAudioPcmChannels())
                {
                    pSettingsTemp->hdmi.input[codec] = _pDecoders[decoder]->getConnector(NEXUS_AudioConnectorType_eMultichannel);
                }
                break;
            case NEXUS_AudioCodec_eUnknown:
            default:
                break;
            } /* switch */
        }
    }

    if (NULL == pSettings)
    {
        /* only set setting if no settings given */
        ret = setSettings(pSettingsTemp);
        CHECK_ERROR_GOTO("set simple audio settings failed", ret, error);
    }

error:
    return(ret);
} /* setHdmiInput */

eHdmiAudioInput CSimpleAudioDecode::getHdmiInput(NEXUS_AudioCodec codec)
{
    NEXUS_SimpleAudioDecoderServerSettings settings;
    eHdmiAudioInput hdmiAudioType = eHdmiAudioInput_None;

    BDBG_ASSERT(NEXUS_AudioCodec_eMax > codec);

    getSettings(&settings);

    for (int i = 0; i < 2; i++)
    {
        if (settings.hdmi.input[codec] == _pDecoders[i]->getConnector(NEXUS_AudioConnectorType_eCompressed))
        {
            hdmiAudioType = eHdmiAudioInput_Compressed;
        }
        else
        if (settings.hdmi.input[codec] == _stereoInput)
        {
            hdmiAudioType = eHdmiAudioInput_Pcm;
        }
        else
        if (settings.hdmi.input[codec] == _pDecoders[i]->getConnector(NEXUS_AudioConnectorType_eMultichannel))
        {
            hdmiAudioType = eHdmiAudioInput_Multichannel;
        }
        else
        if ((NULL != _encodeDts) && (settings.hdmi.input[codec] == NEXUS_DtsEncode_GetConnector(_encodeDts)))
        {
            hdmiAudioType = eHdmiAudioInput_EncodeDts;
        }
        else
        if ((NULL != _encodeAc3) && (settings.hdmi.input[codec] == NEXUS_Ac3Encode_GetConnector(_encodeAc3)))
        {
            hdmiAudioType = eHdmiAudioInput_EncodeAc3;
        }
    }

    return(hdmiAudioType);
} /* getHdmiInput */

CStc * CSimpleAudioDecode::close()
{
    CStc * pStc = _pStc;

    if (NULL != _simpleDecoder)
    {
        NEXUS_SimpleAudioDecoderServerSettings settings;

        /* Disconnect the STC */
        setStc(NULL);

        if (NULL != _pTruVolume)
        {
            _pTruVolume->close();
            _pTruVolume = NULL;
        }

        if (NULL != _pDolbyVolume)
        {
            _pDolbyVolume->close();
            _pDolbyVolume = NULL;
        }

        if (NULL != _pAutoVolumeLevel)
        {
            _pAutoVolumeLevel->close();
            _pAutoVolumeLevel = NULL;
        }

        if (NULL != _encodeDts)
        {
            NEXUS_DtsEncode_RemoveAllInputs(_encodeDts);
            NEXUS_DtsEncode_Close(_encodeDts);
            _encodeDts = NULL;
        }

        if (NULL != _encodeAc3)
        {
            NEXUS_Ac3Encode_RemoveAllInputs(_encodeAc3);
            NEXUS_Ac3Encode_Close(_encodeAc3);
            _encodeAc3 = NULL;
        }

        NEXUS_SimpleAudioDecoder_GetDefaultServerSettings(&settings);
        settings.enabled = false;
        NEXUS_SimpleAudioDecoder_SetServerSettings(_simpleDecoder, &settings);

        NEXUS_SimpleAudioDecoder_Destroy(_simpleDecoder);
        _simpleDecoder = NULL;
    }

    _pSpdif = NULL;
    _pHdmi  = NULL;
    _pDac   = NULL;
    _pRFM   = NULL;
    _pDummy = NULL;

    for (int i = 0; i < 2; i++)
    {
        if (NULL != _pDecoders[i])
        {
            _pDecoders[i]->close();
            _pBoardResources->checkinResource(_pDecoders[i]);
            _pDecoders[i] = NULL;
        }
    }

    if (NULL != _pWidgetEngine)
    {
        _pWidgetEngine->removeCallback(this, CALLBACK_ADECODE_SOURCE_CHANGED);
        _pWidgetEngine = NULL;
    }

    return(pStc);
} /* close */

eRet CSimpleAudioDecode::start(
        CPid * pPid,
        CStc * pStc
        )
{
    eRet        ret    = eRet_Ok;
    NEXUS_Error nerror = NEXUS_SUCCESS;
    NEXUS_SimpleAudioDecoderStartSettings settings;

    BDBG_ASSERT(true == isOpened());

    if (NULL == pPid)
    {
        /* if no pid given use saved pid */
        pPid = getPid();
    }

    ret = setStc(pStc);
    CHECK_ERROR_GOTO("SetStcChannel simple audio decoder failed!", ret, error);

    /* the current encodeAc3 or encodeDts setting may not be allowed for this
     * new decode session - verify compatibility before starting  */
    verifyEncode(pPid->getAudioCodec());

#if 0
    /* TODO: giving incorrect info so disable for now */
    if (false == isCodecSupported(pPid->getAudioCodec()))
    {
        BDBG_WRN(("current decoder does not support audio codec:%d", pPid->getAudioCodec()));
        ret = eRet_NotSupported;
        goto error;
    }
#endif /* if 0 */

    NEXUS_SimpleAudioDecoder_GetDefaultStartSettings(&settings);
    settings.primary.codec      = pPid->getAudioCodec();
    settings.primary.pidChannel = pPid->getPidChannel();
    /* nerror                      = NEXUS_SimpleAudioDecoder_SetStcChannel(_simpleDecoder, pStc->getSimpleStcChannel()); */
    CHECK_NEXUS_ERROR_GOTO("starting simple audio decoder failed!", ret, nerror, error);

    /*
     * set downmix
     * start optional audio capture if necessary
     * codec object call to enable 96khz mode (only applies to aac)
     */

    nerror = NEXUS_SimpleAudioDecoder_Start(_simpleDecoder, &settings);
    CHECK_NEXUS_ERROR_GOTO("simple audio decoder failed to start", ret, nerror, error);

    /* save pid */
    _pPid    = pPid;
    _started = true;

    notifyObservers(eNotify_AudioDecodeStarted, this);
error:
    return(ret);
} /* start */

CPid * CSimpleAudioDecode::stop()
{
    CPid * pPid = NULL;

    if (false == isStarted())
    {
        BDBG_MSG(("Attempting to stop simple audio decode that is already stopped."));
        goto error;
    }

    BDBG_ASSERT(NULL != _simpleDecoder);

    NEXUS_SimpleAudioDecoder_Stop(_simpleDecoder);

    setStc(NULL);

    /* return pid */
    pPid     = _pPid;
    _started = false;

    notifyObservers(eNotify_AudioDecodeStopped, this);
error:
    return(pPid);
} /* stop */

NEXUS_AudioInput CSimpleAudioDecode::getConnector(
        uint8_t                  num,
        NEXUS_AudioConnectorType type
        )
{
    NEXUS_AudioInput nexusInput = NULL;

    BDBG_ASSERT(2 > num);
    BDBG_ASSERT(NEXUS_AudioConnectorType_eMax > type);

    if (NULL != _pDecoders[num])
    {
        nexusInput = _pDecoders[num]->getConnector(type);
    }

    return(nexusInput);
}

void CSimpleAudioDecode::setResources(
        void *            id,
        CBoardResources * pResources
        )
{
    BDBG_ASSERT(NULL != id);
    BDBG_ASSERT(NULL != pResources);

    _resourceId      = id;
    _pBoardResources = pResources;
}

eRet CSimpleAudioDecode::getStatus(NEXUS_AudioDecoderStatus * pStatus)
{
    eRet        ret    = eRet_Ok;
    NEXUS_Error nerror = NEXUS_SUCCESS;

    BDBG_ASSERT(NULL != pStatus);

    nerror = NEXUS_SimpleAudioDecoder_GetStatus(getSimpleDecoder(), pStatus);
    CHECK_NEXUS_ERROR_GOTO("error getting simple audio decoder status", ret, nerror, error);

error:
    return(ret);
}

bool CSimpleAudioDecode::isCodecSupported(NEXUS_AudioCodec codec)
{
    bool bSupported = false;

    /* simple audio decoder does not have support for determining codec support so we will
     * reach into the actual audio decoders it uses to determine compatibility.  this only
     * works because Atlas is both server/client as far as Simple audio decoder is concerned.
     * when built in nxclient mode, atlas will overload this method and always return true
     * (see audio_decode_nx.h). */
    NEXUS_AudioDecoder_IsCodecSupported(_pDecoders[0]->getDecoder(), codec, &bSupported);
    return(bSupported);
}

uint32_t CSimpleAudioDecode::getVolume()
{
    NEXUS_SimpleAudioDecoderHandle decoder = getSimpleDecoder();
    uint32_t volume                        = NEXUS_AUDIO_VOLUME_LINEAR_NORMAL;

    NEXUS_SimpleAudioDecoder_GetSettings(decoder, &_simpleDecoderSettings);

    volume =
        (_simpleDecoderSettings.primary.volumeMatrix[NEXUS_AudioChannel_eLeft][NEXUS_AudioChannel_eLeft] +
         _simpleDecoderSettings.primary.volumeMatrix[NEXUS_AudioChannel_eRight][NEXUS_AudioChannel_eRight]) / 2;

    return(volume);
}

eRet CSimpleAudioDecode::setVolume(uint32_t level)
{
    NEXUS_SimpleAudioDecoderHandle decoder = getSimpleDecoder();
    eRet        ret                        = eRet_Ok;
    NEXUS_Error nerror                     = NEXUS_SUCCESS;

    NEXUS_SimpleAudioDecoder_GetSettings(decoder, &_simpleDecoderSettings);

    _simpleDecoderSettings.primary.volumeMatrix[NEXUS_AudioChannel_eLeft][NEXUS_AudioChannel_eLeft]   = level;
    _simpleDecoderSettings.primary.volumeMatrix[NEXUS_AudioChannel_eRight][NEXUS_AudioChannel_eRight] = level;
    _simpleDecoderSettings.primary.volumeMatrix[NEXUS_AudioChannel_eLeft][NEXUS_AudioChannel_eRight]  = 0;
    _simpleDecoderSettings.primary.volumeMatrix[NEXUS_AudioChannel_eRight][NEXUS_AudioChannel_eLeft]  = 0;

    _simpleDecoderSettings.secondary.volumeMatrix[NEXUS_AudioChannel_eLeft][NEXUS_AudioChannel_eLeft]   = level;
    _simpleDecoderSettings.secondary.volumeMatrix[NEXUS_AudioChannel_eRight][NEXUS_AudioChannel_eRight] = level;
    _simpleDecoderSettings.secondary.volumeMatrix[NEXUS_AudioChannel_eLeft][NEXUS_AudioChannel_eRight]  = 0;
    _simpleDecoderSettings.secondary.volumeMatrix[NEXUS_AudioChannel_eRight][NEXUS_AudioChannel_eLeft]  = 0;

    nerror = NEXUS_SimpleAudioDecoder_SetSettings(decoder, &_simpleDecoderSettings);
    CHECK_NEXUS_ERROR_GOTO("unable to set audio volume", ret, nerror, error);
    BDBG_MSG(("Setting volume level:%d", level));

    {
        CAudioVolume volume;

        volume._left       = level;
        volume._right      = level;
        volume._muted      = _simpleDecoderSettings.primary.muted;
        volume._volumeType = NEXUS_AudioVolumeType_eLinear;
        notifyObservers(eNotify_VolumeChanged, &volume);
    }

error:
    return(ret);
} /* setVolume */

bool CSimpleAudioDecode::getMute()
{
    NEXUS_SimpleAudioDecoderHandle decoder = getSimpleDecoder();

    NEXUS_SimpleAudioDecoder_GetSettings(decoder, &_simpleDecoderSettings);

    return(_simpleDecoderSettings.primary.muted);
}

eRet CSimpleAudioDecode::setMute(bool bMute)
{
    NEXUS_SimpleAudioDecoderHandle decoder = getSimpleDecoder();
    uint32_t    level                      = NEXUS_AUDIO_VOLUME_LINEAR_NORMAL;
    eRet        ret                        = eRet_Ok;
    NEXUS_Error nerror                     = NEXUS_SUCCESS;

    NEXUS_SimpleAudioDecoder_GetSettings(decoder, &_simpleDecoderSettings);

    _simpleDecoderSettings.primary.muted   = bMute;
    _simpleDecoderSettings.secondary.muted = bMute;

    level =
        (_simpleDecoderSettings.primary.volumeMatrix[NEXUS_AudioChannel_eLeft][NEXUS_AudioChannel_eLeft] +
         _simpleDecoderSettings.primary.volumeMatrix[NEXUS_AudioChannel_eRight][NEXUS_AudioChannel_eRight]) / 2;

    nerror = NEXUS_SimpleAudioDecoder_SetSettings(decoder, &_simpleDecoderSettings);
    CHECK_NEXUS_ERROR_GOTO("unable to set audio mute", ret, nerror, error);
    BDBG_MSG(("Setting mute:%d", bMute));

    {
        CAudioVolume volume;

        volume._left       = level;
        volume._right      = level;
        volume._muted      = bMute;
        volume._volumeType = NEXUS_AudioVolumeType_eLinear;
        notifyObservers(eNotify_MuteChanged, &volume);
    }

error:
    return(ret);
} /* setMute */

eAudioDownmix CSimpleAudioDecode::getDownmix()
{
    NEXUS_SimpleAudioDecoderHandle decoder  = getSimpleDecoder();
    CPid *                          pPid    = getPid();
    eAudioDownmix                   downmix = eAudioDownmix_None;
    NEXUS_AudioCodec                codec   = NEXUS_AudioCodec_eUnknown;
    NEXUS_AudioDecoderCodecSettings codecSettings;

    if (NULL != pPid)
    {
        codec = pPid->getAudioCodec();
    }

    NEXUS_SimpleAudioDecoder_GetCodecSettings(decoder, NEXUS_SimpleAudioDecoderSelector_ePrimary, codec, &codecSettings);

    switch (codec)
    {
    case NEXUS_AudioCodec_eAc3:
    case NEXUS_AudioCodec_eAc3Plus:
        downmix = _downmixAc3;
        break;

    case NEXUS_AudioCodec_eDts:
    case NEXUS_AudioCodec_eDtsCd:
    case NEXUS_AudioCodec_eDtsExpress:
        downmix = _downmixDts;
        break;

    case NEXUS_AudioCodec_eAacAdts:
    case NEXUS_AudioCodec_eAacLoas:
    case NEXUS_AudioCodec_eAacPlusAdts:
    case NEXUS_AudioCodec_eAacPlusLoas:
        downmix = _downmixAac;
        break;

    default:
        downmix = _downmix;
        break;
    } /* switch */

    return(downmix);
} /* getDownmix */

#define SET_CODEC_DOWNMIX_AC3(decoderType, codec, downmix)                                          \
    do                                                                                              \
    {                                                                                               \
        NEXUS_SimpleAudioDecoder_GetCodecSettings(decoder, (decoderType), (codec), &codecSettings); \
        switch ((codec))                                                                            \
        {                                                                                           \
        case NEXUS_AudioCodec_eAc3:                                                                 \
            codecSettings.codecSettings.ac3.stereoDownmixMode = (downmix);                          \
            break;                                                                                  \
                                                                                                    \
        case NEXUS_AudioCodec_eAc3Plus:                                                             \
            codecSettings.codecSettings.ac3Plus.stereoDownmixMode = (downmix);                      \
            break;                                                                                  \
                                                                                                    \
        default:                                                                                    \
            BDBG_ERR(("unhandled ac3 audio codec downmix type"));                                   \
            BDBG_ASSERT(false);                                                                     \
            break;                                                                                  \
        }                                                                                           \
                                                                                                    \
        nerror = NEXUS_SimpleAudioDecoder_SetCodecSettings(decoder, (decoderType), &codecSettings); \
        CHECK_NEXUS_ERROR_GOTO("unable to set codec settings", ret, nerror, error);                 \
    } while (0)

#define SET_CODEC_DOWNMIX_DTS(decoderType, codec, downmix)                                          \
    do                                                                                              \
    {                                                                                               \
        NEXUS_SimpleAudioDecoder_GetCodecSettings(decoder, (decoderType), (codec), &codecSettings); \
        switch ((codec))                                                                            \
        {                                                                                           \
        case NEXUS_AudioCodec_eDts:                                                                 \
        case NEXUS_AudioCodec_eDtsCd:                                                               \
        case NEXUS_AudioCodec_eDtsExpress:                                                          \
            codecSettings.codecSettings.dts.stereoDownmixMode = (downmix);                          \
            break;                                                                                  \
                                                                                                    \
        default:                                                                                    \
            BDBG_ERR(("unhandled dts audio codec downmix type"));                                   \
            BDBG_ASSERT(false);                                                                     \
            break;                                                                                  \
        }                                                                                           \
                                                                                                    \
        nerror = NEXUS_SimpleAudioDecoder_SetCodecSettings(decoder, (decoderType), &codecSettings); \
        CHECK_NEXUS_ERROR_GOTO("unable to set codec settings", ret, nerror, error);                 \
    } while (0)

#define SET_CODEC_DOWNMIX_AAC(decoderType, codec, downmix)                                          \
    do                                                                                              \
    {                                                                                               \
        NEXUS_SimpleAudioDecoder_GetCodecSettings(decoder, (decoderType), (codec), &codecSettings); \
        switch ((codec))                                                                            \
        {                                                                                           \
        case NEXUS_AudioCodec_eAacAdts:                                                             \
        case NEXUS_AudioCodec_eAacLoas:                                                             \
        case NEXUS_AudioCodec_eAacPlusAdts:                                                         \
        case NEXUS_AudioCodec_eAacPlusLoas:                                                         \
            codecSettings.codecSettings.aac.downmixMode = (downmix);                                \
            break;                                                                                  \
                                                                                                    \
        default:                                                                                    \
            BDBG_ERR(("unhandled aac audio codec downmix type"));                                   \
            BDBG_ASSERT(false);                                                                     \
            break;                                                                                  \
        }                                                                                           \
                                                                                                    \
        nerror = NEXUS_SimpleAudioDecoder_SetCodecSettings(decoder, (decoderType), &codecSettings); \
        CHECK_NEXUS_ERROR_GOTO("unable to set codec settings", ret, nerror, error);                 \
    } while (0)

eRet CSimpleAudioDecode::setDownmix(eAudioDownmix audioDownmix)
{
    NEXUS_SimpleAudioDecoderHandle  decoder = getSimpleDecoder();
    NEXUS_AudioDecoderCodecSettings codecSettings;
    int32_t     volumeLeft  = NEXUS_AUDIO_VOLUME_LINEAR_NORMAL;
    int32_t     volumeRight = NEXUS_AUDIO_VOLUME_LINEAR_NORMAL;
    eRet        ret         = eRet_Ok;
    NEXUS_Error nerror      = NEXUS_SUCCESS;

    BDBG_ASSERT(false == isStarted());

    NEXUS_SimpleAudioDecoder_GetSettings(decoder, &_simpleDecoderSettings);

    switch (audioDownmix)
    {
    default:
    case eAudioDownmix_None:
        _simpleDecoderSettings.primary.volumeMatrix[NEXUS_AudioChannel_eLeft][NEXUS_AudioChannel_eLeft]   = volumeLeft;
        _simpleDecoderSettings.primary.volumeMatrix[NEXUS_AudioChannel_eRight][NEXUS_AudioChannel_eRight] = volumeRight;
        _simpleDecoderSettings.primary.volumeMatrix[NEXUS_AudioChannel_eLeft][NEXUS_AudioChannel_eRight]  = 0;
        _simpleDecoderSettings.primary.volumeMatrix[NEXUS_AudioChannel_eRight][NEXUS_AudioChannel_eLeft]  = 0;

        _simpleDecoderSettings.secondary.volumeMatrix[NEXUS_AudioChannel_eLeft][NEXUS_AudioChannel_eLeft]   = volumeLeft;
        _simpleDecoderSettings.secondary.volumeMatrix[NEXUS_AudioChannel_eRight][NEXUS_AudioChannel_eRight] = volumeRight;
        _simpleDecoderSettings.secondary.volumeMatrix[NEXUS_AudioChannel_eLeft][NEXUS_AudioChannel_eRight]  = 0;
        _simpleDecoderSettings.secondary.volumeMatrix[NEXUS_AudioChannel_eRight][NEXUS_AudioChannel_eLeft]  = 0;
        nerror = NEXUS_SimpleAudioDecoder_SetSettings(decoder, &_simpleDecoderSettings);
        CHECK_NEXUS_ERROR_GOTO("unable to set audio downmix setting", ret, nerror, error);
        BDBG_MSG(("Setting standard downmix mode"));

        _downmix = audioDownmix;
        BDBG_MSG(("setting audio downmix:%d", _downmix));

        break;

    case eAudioDownmix_Left:
        _simpleDecoderSettings.primary.volumeMatrix[NEXUS_AudioChannel_eLeft][NEXUS_AudioChannel_eLeft]   = volumeLeft;
        _simpleDecoderSettings.primary.volumeMatrix[NEXUS_AudioChannel_eRight][NEXUS_AudioChannel_eRight] = 0;
        _simpleDecoderSettings.primary.volumeMatrix[NEXUS_AudioChannel_eLeft][NEXUS_AudioChannel_eRight]  = 0;
        _simpleDecoderSettings.primary.volumeMatrix[NEXUS_AudioChannel_eRight][NEXUS_AudioChannel_eLeft]  = volumeRight;

        _simpleDecoderSettings.secondary.volumeMatrix[NEXUS_AudioChannel_eLeft][NEXUS_AudioChannel_eLeft]   = volumeLeft;
        _simpleDecoderSettings.secondary.volumeMatrix[NEXUS_AudioChannel_eRight][NEXUS_AudioChannel_eRight] = 0;
        _simpleDecoderSettings.secondary.volumeMatrix[NEXUS_AudioChannel_eLeft][NEXUS_AudioChannel_eRight]  = 0;
        _simpleDecoderSettings.secondary.volumeMatrix[NEXUS_AudioChannel_eRight][NEXUS_AudioChannel_eLeft]  = volumeRight;
        nerror = NEXUS_SimpleAudioDecoder_SetSettings(decoder, &_simpleDecoderSettings);
        CHECK_NEXUS_ERROR_GOTO("unable to set audio downmix setting", ret, nerror, error);

        BDBG_MSG(("Setting downmix left"));
        _downmix = audioDownmix;
        BDBG_MSG(("setting audio downmix:%d", _downmix));

        break;

    case eAudioDownmix_Right:
        _simpleDecoderSettings.primary.volumeMatrix[NEXUS_AudioChannel_eLeft][NEXUS_AudioChannel_eLeft]   = 0;
        _simpleDecoderSettings.primary.volumeMatrix[NEXUS_AudioChannel_eRight][NEXUS_AudioChannel_eRight] = volumeRight;
        _simpleDecoderSettings.primary.volumeMatrix[NEXUS_AudioChannel_eLeft][NEXUS_AudioChannel_eRight]  = volumeLeft;
        _simpleDecoderSettings.primary.volumeMatrix[NEXUS_AudioChannel_eRight][NEXUS_AudioChannel_eLeft]  = 0;

        _simpleDecoderSettings.secondary.volumeMatrix[NEXUS_AudioChannel_eLeft][NEXUS_AudioChannel_eLeft]   = 0;
        _simpleDecoderSettings.secondary.volumeMatrix[NEXUS_AudioChannel_eRight][NEXUS_AudioChannel_eRight] = volumeRight;
        _simpleDecoderSettings.secondary.volumeMatrix[NEXUS_AudioChannel_eLeft][NEXUS_AudioChannel_eRight]  = volumeLeft;
        _simpleDecoderSettings.secondary.volumeMatrix[NEXUS_AudioChannel_eRight][NEXUS_AudioChannel_eLeft]  = 0;
        nerror = NEXUS_SimpleAudioDecoder_SetSettings(decoder, &_simpleDecoderSettings);
        CHECK_NEXUS_ERROR_GOTO("unable to set audio downmix setting", ret, nerror, error);

        BDBG_MSG(("Setting downmix right"));
        _downmix = audioDownmix;
        BDBG_MSG(("setting audio downmix:%d", _downmix));

        break;

    case eAudioDownmix_Monomix:
        _simpleDecoderSettings.primary.volumeMatrix[NEXUS_AudioChannel_eLeft][NEXUS_AudioChannel_eLeft]     = volumeLeft/2;
        _simpleDecoderSettings.primary.volumeMatrix[NEXUS_AudioChannel_eRight][NEXUS_AudioChannel_eRight]   = volumeRight/2;
        _simpleDecoderSettings.primary.volumeMatrix[NEXUS_AudioChannel_eLeft][NEXUS_AudioChannel_eRight]    = volumeLeft/2;
        _simpleDecoderSettings.primary.volumeMatrix[NEXUS_AudioChannel_eRight][NEXUS_AudioChannel_eLeft]    = volumeRight/2;
        _simpleDecoderSettings.secondary.volumeMatrix[NEXUS_AudioChannel_eLeft][NEXUS_AudioChannel_eLeft]   = volumeLeft/2;
        _simpleDecoderSettings.secondary.volumeMatrix[NEXUS_AudioChannel_eRight][NEXUS_AudioChannel_eRight] = volumeRight/2;
        _simpleDecoderSettings.secondary.volumeMatrix[NEXUS_AudioChannel_eLeft][NEXUS_AudioChannel_eRight]  = volumeLeft/2;
        _simpleDecoderSettings.secondary.volumeMatrix[NEXUS_AudioChannel_eRight][NEXUS_AudioChannel_eLeft]  = volumeRight/2;
        nerror = NEXUS_SimpleAudioDecoder_SetSettings(decoder, &_simpleDecoderSettings);
        CHECK_NEXUS_ERROR_GOTO("unable to set audio downmix setting", ret, nerror, error);

        BDBG_MSG(("Setting downmix monomix"));
        _downmix = audioDownmix;
        BDBG_MSG(("setting audio downmix:%d", _downmix));

        break;

    case eAudioDownmix_Ac3Auto:
        SET_CODEC_DOWNMIX_AC3(NEXUS_SimpleAudioDecoderSelector_ePrimary, NEXUS_AudioCodec_eAc3, NEXUS_AudioDecoderDolbyStereoDownmixMode_eAutomatic);
        SET_CODEC_DOWNMIX_AC3(NEXUS_SimpleAudioDecoderSelector_ePrimary, NEXUS_AudioCodec_eAc3Plus, NEXUS_AudioDecoderDolbyStereoDownmixMode_eAutomatic);
        SET_CODEC_DOWNMIX_AC3(NEXUS_SimpleAudioDecoderSelector_eSecondary, NEXUS_AudioCodec_eAc3, NEXUS_AudioDecoderDolbyStereoDownmixMode_eAutomatic);
        SET_CODEC_DOWNMIX_AC3(NEXUS_SimpleAudioDecoderSelector_eSecondary, NEXUS_AudioCodec_eAc3Plus, NEXUS_AudioDecoderDolbyStereoDownmixMode_eAutomatic);

        BDBG_MSG(("Setting AC3 downmix type to Auto"));
        _downmixAc3 = audioDownmix;
        BDBG_MSG(("setting audio downmix:%d", _downmix));

        break;

    case eAudioDownmix_Ac3LtRt:
        SET_CODEC_DOWNMIX_AC3(NEXUS_SimpleAudioDecoderSelector_ePrimary, NEXUS_AudioCodec_eAc3, NEXUS_AudioDecoderDolbyStereoDownmixMode_eDolbySurroundCompatible);
        SET_CODEC_DOWNMIX_AC3(NEXUS_SimpleAudioDecoderSelector_ePrimary, NEXUS_AudioCodec_eAc3Plus, NEXUS_AudioDecoderDolbyStereoDownmixMode_eDolbySurroundCompatible);
        SET_CODEC_DOWNMIX_AC3(NEXUS_SimpleAudioDecoderSelector_eSecondary, NEXUS_AudioCodec_eAc3, NEXUS_AudioDecoderDolbyStereoDownmixMode_eDolbySurroundCompatible);
        SET_CODEC_DOWNMIX_AC3(NEXUS_SimpleAudioDecoderSelector_eSecondary, NEXUS_AudioCodec_eAc3Plus, NEXUS_AudioDecoderDolbyStereoDownmixMode_eDolbySurroundCompatible);

        BDBG_MSG(("Setting AC3 downmix type to LtRt (surround compatible)"));
        _downmixAc3 = audioDownmix;
        BDBG_MSG(("setting audio downmix:%d", _downmix));

        break;

    case eAudioDownmix_Ac3LoRo:
        SET_CODEC_DOWNMIX_AC3(NEXUS_SimpleAudioDecoderSelector_ePrimary, NEXUS_AudioCodec_eAc3, NEXUS_AudioDecoderDolbyStereoDownmixMode_eStandard);
        SET_CODEC_DOWNMIX_AC3(NEXUS_SimpleAudioDecoderSelector_ePrimary, NEXUS_AudioCodec_eAc3Plus, NEXUS_AudioDecoderDolbyStereoDownmixMode_eStandard);
        SET_CODEC_DOWNMIX_AC3(NEXUS_SimpleAudioDecoderSelector_eSecondary, NEXUS_AudioCodec_eAc3, NEXUS_AudioDecoderDolbyStereoDownmixMode_eStandard);
        SET_CODEC_DOWNMIX_AC3(NEXUS_SimpleAudioDecoderSelector_eSecondary, NEXUS_AudioCodec_eAc3Plus, NEXUS_AudioDecoderDolbyStereoDownmixMode_eStandard);

        BDBG_MSG(("Setting AC3 downmix type to LoRo (standard)"));
        _downmixAc3 = audioDownmix;
        BDBG_MSG(("setting audio downmix:%d", _downmix));

        break;

    case eAudioDownmix_DtsAuto:
        SET_CODEC_DOWNMIX_DTS(NEXUS_SimpleAudioDecoderSelector_ePrimary, NEXUS_AudioCodec_eDts, NEXUS_AudioDecoderDtsDownmixMode_eAuto);
        SET_CODEC_DOWNMIX_DTS(NEXUS_SimpleAudioDecoderSelector_ePrimary, NEXUS_AudioCodec_eDtsCd, NEXUS_AudioDecoderDtsDownmixMode_eAuto);
        SET_CODEC_DOWNMIX_DTS(NEXUS_SimpleAudioDecoderSelector_ePrimary, NEXUS_AudioCodec_eDtsExpress, NEXUS_AudioDecoderDtsDownmixMode_eAuto);
        SET_CODEC_DOWNMIX_DTS(NEXUS_SimpleAudioDecoderSelector_eSecondary, NEXUS_AudioCodec_eDts, NEXUS_AudioDecoderDtsDownmixMode_eAuto);
        SET_CODEC_DOWNMIX_DTS(NEXUS_SimpleAudioDecoderSelector_eSecondary, NEXUS_AudioCodec_eDtsCd, NEXUS_AudioDecoderDtsDownmixMode_eAuto);
        SET_CODEC_DOWNMIX_DTS(NEXUS_SimpleAudioDecoderSelector_eSecondary, NEXUS_AudioCodec_eDtsExpress, NEXUS_AudioDecoderDtsDownmixMode_eAuto);

        BDBG_MSG(("Setting DTS downmix type to Auto"));
        _downmixDts = audioDownmix;
        BDBG_MSG(("setting audio downmix:%d", _downmix));

        break;

    case eAudioDownmix_DtsLtRt:
        SET_CODEC_DOWNMIX_DTS(NEXUS_SimpleAudioDecoderSelector_ePrimary, NEXUS_AudioCodec_eDts, NEXUS_AudioDecoderDtsDownmixMode_eLtRt);
        SET_CODEC_DOWNMIX_DTS(NEXUS_SimpleAudioDecoderSelector_ePrimary, NEXUS_AudioCodec_eDtsCd, NEXUS_AudioDecoderDtsDownmixMode_eLtRt);
        SET_CODEC_DOWNMIX_DTS(NEXUS_SimpleAudioDecoderSelector_ePrimary, NEXUS_AudioCodec_eDtsExpress, NEXUS_AudioDecoderDtsDownmixMode_eLtRt);
        SET_CODEC_DOWNMIX_DTS(NEXUS_SimpleAudioDecoderSelector_eSecondary, NEXUS_AudioCodec_eDts, NEXUS_AudioDecoderDtsDownmixMode_eLtRt);
        SET_CODEC_DOWNMIX_DTS(NEXUS_SimpleAudioDecoderSelector_eSecondary, NEXUS_AudioCodec_eDtsCd, NEXUS_AudioDecoderDtsDownmixMode_eLtRt);
        SET_CODEC_DOWNMIX_DTS(NEXUS_SimpleAudioDecoderSelector_eSecondary, NEXUS_AudioCodec_eDtsExpress, NEXUS_AudioDecoderDtsDownmixMode_eLtRt);

        BDBG_MSG(("Setting DTS downmix type to LtRt (surround compatible)"));
        _downmixDts = audioDownmix;
        BDBG_MSG(("setting audio downmix:%d", _downmix));

        break;

    case eAudioDownmix_DtsLoRo:
        SET_CODEC_DOWNMIX_DTS(NEXUS_SimpleAudioDecoderSelector_ePrimary, NEXUS_AudioCodec_eDts, NEXUS_AudioDecoderDtsDownmixMode_eLoRo);
        SET_CODEC_DOWNMIX_DTS(NEXUS_SimpleAudioDecoderSelector_ePrimary, NEXUS_AudioCodec_eDtsCd, NEXUS_AudioDecoderDtsDownmixMode_eLoRo);
        SET_CODEC_DOWNMIX_DTS(NEXUS_SimpleAudioDecoderSelector_ePrimary, NEXUS_AudioCodec_eDtsExpress, NEXUS_AudioDecoderDtsDownmixMode_eLoRo);
        SET_CODEC_DOWNMIX_DTS(NEXUS_SimpleAudioDecoderSelector_eSecondary, NEXUS_AudioCodec_eDts, NEXUS_AudioDecoderDtsDownmixMode_eLoRo);
        SET_CODEC_DOWNMIX_DTS(NEXUS_SimpleAudioDecoderSelector_eSecondary, NEXUS_AudioCodec_eDtsCd, NEXUS_AudioDecoderDtsDownmixMode_eLoRo);
        SET_CODEC_DOWNMIX_DTS(NEXUS_SimpleAudioDecoderSelector_eSecondary, NEXUS_AudioCodec_eDtsExpress, NEXUS_AudioDecoderDtsDownmixMode_eLoRo);

        BDBG_MSG(("Setting DTS downmix type to LoRo (standard)"));
        _downmixDts = audioDownmix;
        BDBG_MSG(("setting audio downmix:%d", _downmix));

        break;

    case eAudioDownmix_AacMatrix:
        SET_CODEC_DOWNMIX_AAC(NEXUS_SimpleAudioDecoderSelector_ePrimary, NEXUS_AudioCodec_eAacAdts, NEXUS_AudioDecoderAacDownmixMode_eMatrix);
        SET_CODEC_DOWNMIX_AAC(NEXUS_SimpleAudioDecoderSelector_ePrimary, NEXUS_AudioCodec_eAacLoas, NEXUS_AudioDecoderAacDownmixMode_eMatrix);
        SET_CODEC_DOWNMIX_AAC(NEXUS_SimpleAudioDecoderSelector_ePrimary, NEXUS_AudioCodec_eAacPlusAdts, NEXUS_AudioDecoderAacDownmixMode_eMatrix);
        SET_CODEC_DOWNMIX_AAC(NEXUS_SimpleAudioDecoderSelector_ePrimary, NEXUS_AudioCodec_eAacPlusLoas, NEXUS_AudioDecoderAacDownmixMode_eMatrix);
        SET_CODEC_DOWNMIX_AAC(NEXUS_SimpleAudioDecoderSelector_eSecondary, NEXUS_AudioCodec_eAacAdts, NEXUS_AudioDecoderAacDownmixMode_eMatrix);
        SET_CODEC_DOWNMIX_AAC(NEXUS_SimpleAudioDecoderSelector_eSecondary, NEXUS_AudioCodec_eAacLoas, NEXUS_AudioDecoderAacDownmixMode_eMatrix);
        SET_CODEC_DOWNMIX_AAC(NEXUS_SimpleAudioDecoderSelector_eSecondary, NEXUS_AudioCodec_eAacPlusAdts, NEXUS_AudioDecoderAacDownmixMode_eMatrix);
        SET_CODEC_DOWNMIX_AAC(NEXUS_SimpleAudioDecoderSelector_eSecondary, NEXUS_AudioCodec_eAacPlusLoas, NEXUS_AudioDecoderAacDownmixMode_eMatrix);

        BDBG_MSG(("Setting AAC downmix type to (BRCM) matrix"));
        _downmixAac = audioDownmix;
        BDBG_MSG(("setting audio downmix:%d", _downmix));

        break;

    case eAudioDownmix_AacArib:
        SET_CODEC_DOWNMIX_AAC(NEXUS_SimpleAudioDecoderSelector_ePrimary, NEXUS_AudioCodec_eAacAdts, NEXUS_AudioDecoderAacDownmixMode_eArib);
        SET_CODEC_DOWNMIX_AAC(NEXUS_SimpleAudioDecoderSelector_ePrimary, NEXUS_AudioCodec_eAacLoas, NEXUS_AudioDecoderAacDownmixMode_eArib);
        SET_CODEC_DOWNMIX_AAC(NEXUS_SimpleAudioDecoderSelector_ePrimary, NEXUS_AudioCodec_eAacPlusAdts, NEXUS_AudioDecoderAacDownmixMode_eArib);
        SET_CODEC_DOWNMIX_AAC(NEXUS_SimpleAudioDecoderSelector_ePrimary, NEXUS_AudioCodec_eAacPlusLoas, NEXUS_AudioDecoderAacDownmixMode_eArib);
        SET_CODEC_DOWNMIX_AAC(NEXUS_SimpleAudioDecoderSelector_eSecondary, NEXUS_AudioCodec_eAacAdts, NEXUS_AudioDecoderAacDownmixMode_eArib);
        SET_CODEC_DOWNMIX_AAC(NEXUS_SimpleAudioDecoderSelector_eSecondary, NEXUS_AudioCodec_eAacLoas, NEXUS_AudioDecoderAacDownmixMode_eArib);
        SET_CODEC_DOWNMIX_AAC(NEXUS_SimpleAudioDecoderSelector_eSecondary, NEXUS_AudioCodec_eAacPlusAdts, NEXUS_AudioDecoderAacDownmixMode_eArib);
        SET_CODEC_DOWNMIX_AAC(NEXUS_SimpleAudioDecoderSelector_eSecondary, NEXUS_AudioCodec_eAacPlusLoas, NEXUS_AudioDecoderAacDownmixMode_eArib);

        BDBG_MSG(("Setting AAC downmix type to ARIB"));
        _downmixAac = audioDownmix;
        BDBG_MSG(("setting audio downmix:%d", _downmix));

        break;

    case eAudioDownmix_AacLtRt:
        SET_CODEC_DOWNMIX_AAC(NEXUS_SimpleAudioDecoderSelector_ePrimary, NEXUS_AudioCodec_eAacAdts, NEXUS_AudioDecoderAacDownmixMode_eLtRt);
        SET_CODEC_DOWNMIX_AAC(NEXUS_SimpleAudioDecoderSelector_ePrimary, NEXUS_AudioCodec_eAacLoas, NEXUS_AudioDecoderAacDownmixMode_eLtRt);
        SET_CODEC_DOWNMIX_AAC(NEXUS_SimpleAudioDecoderSelector_ePrimary, NEXUS_AudioCodec_eAacPlusAdts, NEXUS_AudioDecoderAacDownmixMode_eLtRt);
        SET_CODEC_DOWNMIX_AAC(NEXUS_SimpleAudioDecoderSelector_ePrimary, NEXUS_AudioCodec_eAacPlusLoas, NEXUS_AudioDecoderAacDownmixMode_eLtRt);
        SET_CODEC_DOWNMIX_AAC(NEXUS_SimpleAudioDecoderSelector_eSecondary, NEXUS_AudioCodec_eAacAdts, NEXUS_AudioDecoderAacDownmixMode_eLtRt);
        SET_CODEC_DOWNMIX_AAC(NEXUS_SimpleAudioDecoderSelector_eSecondary, NEXUS_AudioCodec_eAacLoas, NEXUS_AudioDecoderAacDownmixMode_eLtRt);
        SET_CODEC_DOWNMIX_AAC(NEXUS_SimpleAudioDecoderSelector_eSecondary, NEXUS_AudioCodec_eAacPlusAdts, NEXUS_AudioDecoderAacDownmixMode_eLtRt);
        SET_CODEC_DOWNMIX_AAC(NEXUS_SimpleAudioDecoderSelector_eSecondary, NEXUS_AudioCodec_eAacPlusLoas, NEXUS_AudioDecoderAacDownmixMode_eLtRt);

        BDBG_MSG(("Setting AAC downmix type to ARIB"));
        _downmixAac = audioDownmix;
        BDBG_MSG(("setting audio downmix:%d", _downmix));

        break;
    } /* switch */

error:
    return(ret);
} /* setDownmix */

eRet CSimpleAudioDecode::setDualMono(eAudioDualMono dualMono)
{
    NEXUS_SimpleAudioDecoderHandle decoder = getSimpleDecoder();
    eRet        ret                        = eRet_Ok;
    NEXUS_Error nerror                     = NEXUS_SUCCESS;

    BDBG_ASSERT(false == isStarted());

    NEXUS_SimpleAudioDecoder_GetSettings(decoder, &_simpleDecoderSettings);

    switch (dualMono)
    {
    case eAudioDualMono_Left:
        _simpleDecoderSettings.primary.dualMonoMode   = NEXUS_AudioDecoderDualMonoMode_eLeft;
        _simpleDecoderSettings.secondary.dualMonoMode = NEXUS_AudioDecoderDualMonoMode_eLeft;
        break;
    case eAudioDualMono_Right:
        _simpleDecoderSettings.primary.dualMonoMode   = NEXUS_AudioDecoderDualMonoMode_eRight;
        _simpleDecoderSettings.secondary.dualMonoMode = NEXUS_AudioDecoderDualMonoMode_eRight;
        break;
    default:
    case eAudioDualMono_Stereo:
        _simpleDecoderSettings.primary.dualMonoMode   = NEXUS_AudioDecoderDualMonoMode_eStereo;
        _simpleDecoderSettings.secondary.dualMonoMode = NEXUS_AudioDecoderDualMonoMode_eStereo;
        break;
    case eAudioDualMono_Monomix:
        _simpleDecoderSettings.primary.dualMonoMode   = NEXUS_AudioDecoderDualMonoMode_eMix;
        _simpleDecoderSettings.secondary.dualMonoMode = NEXUS_AudioDecoderDualMonoMode_eMix;
        break;
    } /* switch */

    nerror = NEXUS_SimpleAudioDecoder_SetSettings(decoder, &_simpleDecoderSettings);
    CHECK_NEXUS_ERROR_GOTO("unable to set audio decoder settings", ret, nerror, error);

    _dualMono = dualMono;
    BDBG_MSG(("setting dual mono:%d", _dualMono));

error:
    return(ret);
} /* setDualMono */

eRet CSimpleAudioDecode::setDolbyDRC(eDolbyDRC dolbyDRC)
{
    NEXUS_AudioDecoderCodecSettings codecSettings;
    NEXUS_SimpleAudioDecoderHandle  decoder = getSimpleDecoder();
    CPid *      pPid                        = NULL;
    eRet        ret                         = eRet_Ok;
    NEXUS_Error nerror                      = NEXUS_SUCCESS;

    BDBG_ASSERT(false == isStarted());

    pPid = getPid();
    CHECK_PTR_ERROR_GOTO("audio decoder has missing pid", pPid, ret, eRet_InvalidState, error);

    switch (pPid->getAudioCodec())
    {
    case NEXUS_AudioCodec_eAc3:
        NEXUS_SimpleAudioDecoder_GetCodecSettings(decoder, NEXUS_SimpleAudioDecoderSelector_ePrimary, NEXUS_AudioCodec_eAc3, &codecSettings);
        switch (dolbyDRC)
        {
        case eDolbyDRC_Light:
        case eDolbyDRC_Medium:
            codecSettings.codecSettings.ac3.drcMode = NEXUS_AudioDecoderDolbyDrcMode_eLine;
            codecSettings.codecSettings.ac3.boost   = GET_INT(_pCfg, AUDIO_DOLBY_DRC_AC3_BOOST);
            codecSettings.codecSettings.ac3.cut     = GET_INT(_pCfg, AUDIO_DOLBY_DRC_AC3_CUT);
            break;
        case eDolbyDRC_Heavy:
            codecSettings.codecSettings.ac3.drcMode = NEXUS_AudioDecoderDolbyDrcMode_eRf;
            codecSettings.codecSettings.ac3.boost   = 0;
            codecSettings.codecSettings.ac3.cut     = 0;
            break;
        case eDolbyDRC_None:
        default:
            codecSettings.codecSettings.ac3.drcMode = NEXUS_AudioDecoderDolbyDrcMode_eLine;
            codecSettings.codecSettings.ac3.boost   = 0;
            codecSettings.codecSettings.ac3.cut     = 0;
            break;
        } /* switch */
        nerror = NEXUS_SimpleAudioDecoder_SetCodecSettings(decoder, NEXUS_SimpleAudioDecoderSelector_ePrimary, &codecSettings);
        CHECK_NEXUS_ERROR_GOTO("unable to set codec settings", ret, nerror, error);

        NEXUS_SimpleAudioDecoder_GetCodecSettings(decoder, NEXUS_SimpleAudioDecoderSelector_eSecondary, NEXUS_AudioCodec_eAc3, &codecSettings);
        switch (dolbyDRC)
        {
        case eDolbyDRC_Light:
        case eDolbyDRC_Medium:
            codecSettings.codecSettings.ac3.drcMode = NEXUS_AudioDecoderDolbyDrcMode_eLine;
            codecSettings.codecSettings.ac3.boost   = GET_INT(_pCfg, AUDIO_DOLBY_DRC_AC3_BOOST);
            codecSettings.codecSettings.ac3.cut     = GET_INT(_pCfg, AUDIO_DOLBY_DRC_AC3_CUT);
            break;
        case eDolbyDRC_Heavy:
            codecSettings.codecSettings.ac3.drcMode = NEXUS_AudioDecoderDolbyDrcMode_eRf;
            codecSettings.codecSettings.ac3.boost   = 0;
            codecSettings.codecSettings.ac3.cut     = 0;
            break;
        case eDolbyDRC_None:
        default:
            codecSettings.codecSettings.ac3.drcMode = NEXUS_AudioDecoderDolbyDrcMode_eLine;
            codecSettings.codecSettings.ac3.boost   = 0;
            codecSettings.codecSettings.ac3.cut     = 0;
            break;
        } /* switch */
        nerror = NEXUS_SimpleAudioDecoder_SetCodecSettings(decoder, NEXUS_SimpleAudioDecoderSelector_eSecondary, &codecSettings);
        CHECK_NEXUS_ERROR_GOTO("unable to set codec settings", ret, nerror, error);
        break;

    case NEXUS_AudioCodec_eAc3Plus:
        NEXUS_SimpleAudioDecoder_GetCodecSettings(decoder, NEXUS_SimpleAudioDecoderSelector_ePrimary, NEXUS_AudioCodec_eAc3Plus, &codecSettings);
        switch (dolbyDRC)
        {
        case eDolbyDRC_Light:
        case eDolbyDRC_Medium:
            codecSettings.codecSettings.ac3Plus.drcMode = NEXUS_AudioDecoderDolbyDrcMode_eLine;
            codecSettings.codecSettings.ac3Plus.boost   = GET_INT(_pCfg, AUDIO_DOLBY_DRC_AC3_BOOST);
            codecSettings.codecSettings.ac3Plus.cut     = GET_INT(_pCfg, AUDIO_DOLBY_DRC_AC3_CUT);
            break;
        case eDolbyDRC_Heavy:
            codecSettings.codecSettings.ac3Plus.drcMode = NEXUS_AudioDecoderDolbyDrcMode_eRf;
            codecSettings.codecSettings.ac3Plus.boost   = 0;
            codecSettings.codecSettings.ac3Plus.cut     = 0;
            break;
        case eDolbyDRC_None:
        default:
            codecSettings.codecSettings.ac3Plus.drcMode = NEXUS_AudioDecoderDolbyDrcMode_eLine;
            codecSettings.codecSettings.ac3Plus.boost   = 0;
            codecSettings.codecSettings.ac3Plus.cut     = 0;
            break;
        } /* switch */
        nerror = NEXUS_SimpleAudioDecoder_SetCodecSettings(decoder, NEXUS_SimpleAudioDecoderSelector_ePrimary, &codecSettings);
        CHECK_NEXUS_ERROR_GOTO("unable to set codec settings", ret, nerror, error);

        NEXUS_SimpleAudioDecoder_GetCodecSettings(decoder, NEXUS_SimpleAudioDecoderSelector_eSecondary, NEXUS_AudioCodec_eAc3Plus, &codecSettings);
        switch (dolbyDRC)
        {
        case eDolbyDRC_Light:
        case eDolbyDRC_Medium:
            codecSettings.codecSettings.ac3Plus.drcMode = NEXUS_AudioDecoderDolbyDrcMode_eLine;
            codecSettings.codecSettings.ac3Plus.boost   = GET_INT(_pCfg, AUDIO_DOLBY_DRC_AC3_BOOST);
            codecSettings.codecSettings.ac3Plus.cut     = GET_INT(_pCfg, AUDIO_DOLBY_DRC_AC3_CUT);
            break;
        case eDolbyDRC_Heavy:
            codecSettings.codecSettings.ac3Plus.drcMode = NEXUS_AudioDecoderDolbyDrcMode_eRf;
            codecSettings.codecSettings.ac3Plus.boost   = 0;
            codecSettings.codecSettings.ac3Plus.cut     = 0;
            break;
        case eDolbyDRC_None:
        default:
            codecSettings.codecSettings.ac3Plus.drcMode = NEXUS_AudioDecoderDolbyDrcMode_eLine;
            codecSettings.codecSettings.ac3Plus.boost   = 0;
            codecSettings.codecSettings.ac3Plus.cut     = 0;
            break;
        } /* switch */
        nerror = NEXUS_SimpleAudioDecoder_SetCodecSettings(decoder, NEXUS_SimpleAudioDecoderSelector_eSecondary, &codecSettings);
        CHECK_NEXUS_ERROR_GOTO("unable to set codec settings", ret, nerror, error);
        break;

    default:
        BDBG_WRN(("ignoring attempt to set dolby drc for invalid codec:%d", pPid->getAudioCodec()));
        ret = eRet_InvalidState;
        goto error;
    } /* switch */

    _dolbyDRC = dolbyDRC;
    BDBG_MSG(("setting dolby drc:%d", _dolbyDRC));

error:
    return(ret);
} /* setDolbyDRC */

eRet CSimpleAudioDecode::setDolbyDialogNorm(bool dolbyDialogNorm)
{
    NEXUS_AudioDecoderCodecSettings codecSettings;
    NEXUS_SimpleAudioDecoderHandle  decoder = getSimpleDecoder();
    CPid *      pPid                        = NULL;
    eRet        ret                         = eRet_Ok;
    NEXUS_Error nerror                      = NEXUS_SUCCESS;

    BDBG_ASSERT(false == isStarted());

    pPid = getPid();
    CHECK_PTR_ERROR_GOTO("audio decoder has missing pid", pPid, ret, eRet_InvalidState, error);

    switch (pPid->getAudioCodec())
    {
    case NEXUS_AudioCodec_eAc3:
        NEXUS_SimpleAudioDecoder_GetCodecSettings(decoder, NEXUS_SimpleAudioDecoderSelector_ePrimary, NEXUS_AudioCodec_eAc3, &codecSettings);
        codecSettings.codecSettings.ac3.dialogNormalization = dolbyDialogNorm;
        nerror = NEXUS_SimpleAudioDecoder_SetCodecSettings(decoder, NEXUS_SimpleAudioDecoderSelector_ePrimary, &codecSettings);
        CHECK_NEXUS_ERROR_GOTO("unable to set codec settings", ret, nerror, error);
        break;

    case NEXUS_AudioCodec_eAc3Plus:
        NEXUS_SimpleAudioDecoder_GetCodecSettings(decoder, NEXUS_SimpleAudioDecoderSelector_eSecondary, NEXUS_AudioCodec_eAc3Plus, &codecSettings);
        codecSettings.codecSettings.ac3Plus.dialogNormalization = dolbyDialogNorm;
        nerror = NEXUS_SimpleAudioDecoder_SetCodecSettings(decoder, NEXUS_SimpleAudioDecoderSelector_eSecondary, &codecSettings);
        CHECK_NEXUS_ERROR_GOTO("unable to set codec settings", ret, nerror, error);
        break;

    default:
        BDBG_WRN(("ignoring attempt to set dolby dialog normalization for invalid codec:%d", pPid->getAudioCodec()));
        ret = eRet_InvalidState;
        goto error;
    } /* switch */

    _dolbyDialogNorm = dolbyDialogNorm;
    BDBG_MSG(("setting dolby dialog normalization:%d", _dolbyDialogNorm));

error:
    return(ret);
} /* setDolbyDialogNorm */

eRet CSimpleAudioDecode::setStc(CStc * pStc)
{
    eRet        ret    = eRet_Ok;
    NEXUS_Error nerror = NEXUS_SUCCESS;

    nerror = NEXUS_SimpleAudioDecoder_SetStcChannel(_simpleDecoder, (NULL != pStc) ? pStc->getSimpleStcChannel() : NULL);
    CHECK_NEXUS_ERROR_GOTO("SetStcChannel simple audio decoder failed!", ret, nerror, error);

    _pStc = pStc;

error:
    return(ret);
}