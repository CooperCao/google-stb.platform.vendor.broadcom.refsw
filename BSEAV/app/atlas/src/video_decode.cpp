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

#include "nexus_video_decoder.h"
#include "nexus_simple_video_decoder_server.h"
#include "nexus_core_utils.h"
#include "video_decode.h"
#include "video_window.h"
#include "output.h"
#include "atlas_os.h"
#include "channel.h"

#define CALLBACK_VDECODE_SOURCE_CHANGED  "callbackVDecodeSourceChanged"
#define CALLBACK_VDECODE_STREAM_CHANGED  "callbackVDecodeStreamChanged"

BDBG_MODULE(atlas_video_decode);

ENUM_TO_MSTRING_INIT_CPP(CSimpleVideoDecode, noiseReductionModeToString, NEXUS_VideoWindowFilterMode)
ENUM_TO_MSTRING_START()
ENUM_TO_MSTRING_ENTRY(NEXUS_VideoWindowFilterMode_eDisable, "Disabled")
ENUM_TO_MSTRING_ENTRY(NEXUS_VideoWindowFilterMode_eBypass, "Bypass")
ENUM_TO_MSTRING_ENTRY(NEXUS_VideoWindowFilterMode_eEnable, "Enabled")
ENUM_TO_MSTRING_ENTRY(NEXUS_VideoWindowFilterMode_eMax, "Invalid")
ENUM_TO_MSTRING_END()

/* bwin io callback that is triggered when it is safe to handle callbacks */
static void bwinVideoDecodeSourceChangedCallback(
        void *       pObject,
        const char * strCallback
        )
{
    CVideoDecode * pVideoDecode = (CVideoDecode *)pObject;

    BSTD_UNUSED(strCallback);
    BDBG_ASSERT(NULL != pVideoDecode);

    pVideoDecode->videoDecodeSourceChangedCallback();
} /* bwinVideoDecodeSourceChangedCallback */

/* bwin io callback that is triggered when it is safe to handle callbacks */
static void bwinSimpleVideoDecodeSourceChangedCallback(
        void *       pObject,
        const char * strCallback
        )
{
    CSimpleVideoDecode * pSimpleVideoDecode = (CSimpleVideoDecode *)pObject;

    BSTD_UNUSED(strCallback);
    BDBG_ASSERT(NULL != pSimpleVideoDecode);

    pSimpleVideoDecode->videoDecodeSourceChangedCallback();
} /* bwinSimpleVideoDecodeSourceChangedCallback */

/* bwin io callback that is triggered when it is safe to handle callbacks */
static void bwinVideoDecodeStreamChangedCallback(
        void *       pObject,
        const char * strCallback
        )
{
    CVideoDecode * pVideoDecode = (CVideoDecode *)pObject;

    BSTD_UNUSED(strCallback);
    BDBG_ASSERT(NULL != pVideoDecode);

    pVideoDecode->videoDecodeSourceChangedCallback();
} /* bwinVideoDecodeStreamChangedCallback */

/* bwin io callback that is triggered when it is safe to handle callbacks */
static void bwinSimpleVideoDecodeStreamChangedCallback(
        void *       pObject,
        const char * strCallback
        )
{
    CSimpleVideoDecode * pSimpleVideoDecode = (CSimpleVideoDecode *)pObject;

    BSTD_UNUSED(strCallback);
    BDBG_ASSERT(NULL != pSimpleVideoDecode);

    pSimpleVideoDecode->videoDecodeStreamChangedCallback();
} /* bwinSimpleVideoDecodeStreamChangedCallback */

void CVideoDecode::videoDecodeSourceChangedCallback()
{
    eRet ret = eRet_Ok;
    NEXUS_VideoDecoderStatus status;

    if (true == isSourceChanged())
    {
        setSourceChanged(false);

        getStatus(&status);

        /* ignore 0 source change callback */
        if (0 == status.aspectRatio + status.source.width + status.source.height)
        {
            goto error;
        }

        BDBG_MSG(("Notify Observers for video decoder event code: %#x", eNotify_VideoSourceChanged));
        ret = notifyObservers(eNotify_VideoSourceChanged, this);
        CHECK_ERROR_GOTO("error notifying observers", ret, error);
    }

error:
    return;
} /* videoDecodeSourceChangedCallback */

void CVideoDecode::videoDecodeStreamChangedCallback()
{
    eRet ret = eRet_Ok;

    if (true == isStreamChanged())
    {
        setStreamChanged(false);

        BDBG_MSG(("Notify Observers for video decoder event code: %#x", eNotify_VideoStreamChanged));
        ret = notifyObservers(eNotify_VideoStreamChanged, this);
        CHECK_ERROR_GOTO("error notifying observers", ret, error);
    }

error:
    return;
} /* videoDecodeStreamChangedCallback */

void CSimpleVideoDecode::videoDecodeSourceChangedCallback()
{
    eRet ret = eRet_Ok;
    NEXUS_VideoDecoderStatus status;

    if (true == isSourceChanged())
    {
        setSourceChanged(false);

        getStatus(&status);

        /* ignore 0 source change callback */
        if (0 == status.aspectRatio + status.source.width + status.source.height)
        {
            goto error;
        }

        BDBG_MSG(("Notify Observers for video decoder event code: %#x", eNotify_VideoSourceChanged));
        ret = notifyObservers(eNotify_VideoSourceChanged, this);
        CHECK_ERROR_GOTO("error notifying observers", ret, error);
    }

error:
    return;
} /* videoDecodeSourceChangedCallback */

void CSimpleVideoDecode::videoDecodeStreamChangedCallback()
{
    eRet ret = eRet_Ok;

    if (true == isStreamChanged())
    {
        setStreamChanged(false);

        BDBG_MSG(("Notify Observers for video decoder event code: %#x", eNotify_VideoStreamChanged));
        ret = notifyObservers(eNotify_VideoStreamChanged, this);
        CHECK_ERROR_GOTO("error notifying observers", ret, error);
    }

error:
    return;
} /* videoDecodeStreamChangedCallback */

/* nexus callback that is executed when the video source has changed */
static void NexusVideoDecodeSourceChangedCallback(
        void * context,
        int    param
        )
{
    CVideoDecode * pVideoDecode = (CVideoDecode *)context;

    BSTD_UNUSED(param);
    BDBG_ASSERT(NULL != pVideoDecode);

    CWidgetEngine * pWidgetEngine = pVideoDecode->getWidgetEngine();

    pVideoDecode->setSourceChanged(true);

    /* sync with bwin loop */
    if (NULL != pWidgetEngine)
    {
        pWidgetEngine->syncCallback(pVideoDecode, CALLBACK_VDECODE_SOURCE_CHANGED);
    }
} /* NexusVideoDecodeSourceChangedCallback */

/* nexus callback that is executed when the video stream has changed */
static void NexusVideoDecodeStreamChangedCallback(
        void * context,
        int    param
        )
{
    CVideoDecode * pVideoDecode = (CVideoDecode *)context;

    BSTD_UNUSED(param);
    BDBG_ASSERT(NULL != pVideoDecode);

    CWidgetEngine * pWidgetEngine = pVideoDecode->getWidgetEngine();

    pVideoDecode->setStreamChanged(true);

    /* sync with bwin loop */
    if (NULL != pWidgetEngine)
    {
        pWidgetEngine->syncCallback(pVideoDecode, CALLBACK_VDECODE_STREAM_CHANGED);
    }
} /* NexusVideoDecodeStreamChangedCallback */

/* nexus callback that is executed when the video source has changed */
static void NexusSimpleVideoDecodeSourceChangedCallback(
        void * context,
        int    param
        )
{
    CSimpleVideoDecode * pSimpleVideoDecode = (CSimpleVideoDecode *)context;

    BSTD_UNUSED(param);
    BDBG_ASSERT(NULL != pSimpleVideoDecode);

    CWidgetEngine * pWidgetEngine = pSimpleVideoDecode->getWidgetEngine();

    pSimpleVideoDecode->setSourceChanged(true);

    /* sync with bwin loop */
    if (NULL != pWidgetEngine)
    {
        pWidgetEngine->syncCallback(pSimpleVideoDecode, CALLBACK_VDECODE_SOURCE_CHANGED);
    }
} /* NexusSimpleVideoDecodeSourceChangedCallback */

/* nexus callback that is executed when the video stream has changed */
static void NexusSimpleVideoDecodeStreamChangedCallback(
        void * context,
        int    param
        )
{
    CSimpleVideoDecode * pSimpleVideoDecode = (CSimpleVideoDecode *)context;

    BSTD_UNUSED(param);
    BDBG_ASSERT(NULL != pSimpleVideoDecode);

    CWidgetEngine * pWidgetEngine = pSimpleVideoDecode->getWidgetEngine();

    pSimpleVideoDecode->setStreamChanged(true);

    /* sync with bwin loop */
    if (NULL != pWidgetEngine)
    {
        pWidgetEngine->syncCallback(pSimpleVideoDecode, CALLBACK_VDECODE_STREAM_CHANGED);
    }
} /* NexusSimpleVideoDecodeStreamChangedCallback */

CVideoDecode::CVideoDecode(
        const char *     name,
        const uint16_t   number,
        CConfiguration * pCfg
        ) :
    CResource(name, number, eBoardResource_decodeVideo, pCfg),
    _decoder(NULL),
    _pStc(NULL),
    _pPid(NULL),
    _pWidgetEngine(NULL),
    _started(false),
    _sourceChanged(false),
    _maxWidth(0),
    _maxHeight(0)
{
    eRet ret = eRet_Ok;

    BDBG_ASSERT(eRet_Ok == ret);
}

CVideoDecode::~CVideoDecode()
{
}

eRet CVideoDecode::open(
        CWidgetEngine * pWidgetEngine,
        CStc *          pStc
        )
{
    eRet ret      = eRet_Ok;
    int  fifoSize = 0;
    NEXUS_VideoDecoderOpenSettings settings;

    BDBG_ASSERT(NULL != pStc);

    if (true == isOpened())
    {
        ret = eRet_InvalidState;
        CHECK_ERROR_GOTO("Video decoder is already opened.", ret, error);
    }

    _pWidgetEngine = pWidgetEngine;

    if (NULL != _pWidgetEngine)
    {
        _pWidgetEngine->addCallback(this, CALLBACK_VDECODE_SOURCE_CHANGED, bwinVideoDecodeSourceChangedCallback);
        _pWidgetEngine->addCallback(this, CALLBACK_VDECODE_STREAM_CHANGED, bwinVideoDecodeStreamChangedCallback);
    }

    {
        CPlatform * pPlatform = _pCfg->getPlatformConfig();
        /* since open() is only called in the nexus atlas (overloaded in nxclient atlas),
         * getNumber() will return the decoder number.  nxclient atlas uses getNumber()
         * to return the nxclient id instead which getDecoderMaxVideoFormat() does not
         * use for its decoder array index. */
        NEXUS_VideoFormat maxFormat = pPlatform->getDecoderMaxVideoFormat(getNumber());

        /* get max width and height supported by video decoder */
        _maxWidth  = videoFormatToHorizRes(maxFormat).toInt();
        _maxHeight = videoFormatToVertRes(maxFormat).toInt();
    }

    NEXUS_VideoDecoder_GetDefaultOpenSettings(&settings);
    fifoSize = GET_INT(_pCfg, VIDEO_DECODER_FIFO_SIZE);
    if (0 < fifoSize)
    {
        settings.fifoSize = fifoSize * 1000; /* dtt - should this be 1024? */
        BDBG_WRN(("****************************************"));
        BDBG_WRN(("* Changing Video FIFO size to %d bytes *", settings.fifoSize));
        BDBG_WRN(("****************************************"));
    }

#ifdef PLAYBACK_IP_SUPPORT
    fifoSize = GET_INT(_pCfg, VIDEO_DECODER_IP_FIFO_SIZE);
    if (0 < fifoSize)
    {
        settings.fifoSize = fifoSize; /* jrubio- should this be 1024*1024*10 */
        BDBG_MSG(("*******************************************************************"));
        BDBG_MSG(("* PLAYBACK IP Jitter support Changing Video FIFO size to %d bytes *", settings.fifoSize));
        BDBG_MSG(("*******************************************************************"));
    }
#endif /* if PLAYBACK_IP_SUPPORT */

    if (true == GET_BOOL(_pCfg, AVC_51_SUPPORT))
    {
        BDBG_WRN(("AVC level 5.1 support enabled"));
        settings.avc51Enabled = true;
    }

    _decoder = NEXUS_VideoDecoder_Open(_number, &settings);
    CHECK_PTR_ERROR_GOTO("Nexus video decoder open failed!", _decoder, ret, eRet_ExternalError, error);

    /* set video decoder settings */
    {
        NEXUS_Error                nError = NEXUS_SUCCESS;
        NEXUS_VideoDecoderSettings videoDecoderSettings;

        NEXUS_VideoDecoder_GetSettings(_decoder, &videoDecoderSettings);
        videoDecoderSettings.streamChanged.callback = NexusVideoDecodeStreamChangedCallback;
        videoDecoderSettings.streamChanged.context  = this;
        videoDecoderSettings.streamChanged.param    = 0;
        videoDecoderSettings.sourceChanged.callback = NexusVideoDecodeSourceChangedCallback;
        videoDecoderSettings.sourceChanged.context  = this;
        videoDecoderSettings.sourceChanged.param    = 0;
        videoDecoderSettings.dropFieldMode          = GET_BOOL(_pCfg, DROP_FIELD_MODE_ENABLED);
        videoDecoderSettings.maxWidth               = _maxWidth;
        videoDecoderSettings.maxHeight              = _maxHeight;
        nError = NEXUS_VideoDecoder_SetSettings(_decoder, &videoDecoderSettings);
        CHECK_NEXUS_ERROR_GOTO("Error applying video decoder settings.", ret, nError, error);
    }

    /* dtt - create astm (adaptive system time managment) */

error:
    return(ret);
} /* open */

CStc * CVideoDecode::close()
{
    CStc * pStc = _pStc;

    if (_decoder)
    {
        NEXUS_VideoInput_Shutdown(NEXUS_VideoDecoder_GetConnector(_decoder));
        NEXUS_VideoDecoder_Close(_decoder);

        _pStc    = NULL;
        _decoder = NULL;
    }

    if (NULL != _pWidgetEngine)
    {
        _pWidgetEngine->removeCallback(this, CALLBACK_VDECODE_STREAM_CHANGED);
        _pWidgetEngine->removeCallback(this, CALLBACK_VDECODE_SOURCE_CHANGED);
        _pWidgetEngine = NULL;
    }

    return(pStc);
} /* close */

NEXUS_VideoInputHandle CVideoDecode::getConnector(void)
{
    NEXUS_VideoInputHandle   hVideoInput = NULL;
    NEXUS_VideoDecoderHandle hVideo      = getDecoder();

    if (NULL != hVideo)
    {
        hVideoInput = NEXUS_VideoDecoder_GetConnector(hVideo);
    }

    return(hVideoInput);
} /* getConnector() */

eRet CVideoDecode::getStatus(NEXUS_VideoDecoderStatus * pStatus)
{
    eRet        ret    = eRet_Ok;
    NEXUS_Error nerror = NEXUS_SUCCESS;

    BDBG_ASSERT(NULL != pStatus);

    nerror = NEXUS_VideoDecoder_GetStatus(getDecoder(), pStatus);
    CHECK_NEXUS_ERROR_GOTO("getting video decoder status failed!", ret, nerror, error);

error:
    return(ret);
}

eRet CVideoDecode::getStreamInfo(NEXUS_VideoDecoderStreamInformation * pStream)
{
    eRet        ret    = eRet_Ok;
    NEXUS_Error nerror = NEXUS_SUCCESS;

    BDBG_ASSERT(NULL != pStream);

    nerror = NEXUS_VideoDecoder_GetStreamInformation(getDecoder(), pStream);
    CHECK_NEXUS_ERROR_GOTO("getting simple video decoder stream info failed!", ret, nerror, error);

error:
    return(ret);
}

bool CVideoDecode::isCodecSupported(NEXUS_VideoCodec codec)
{
    NEXUS_VideoDecoderSettings settings;

    NEXUS_VideoDecoder_GetSettings(getDecoder(), &settings);
    return(settings.supportedCodecs[codec]);
}

/*** simple video decoder class */

#include "nexus_simple_video_decoder_server.h"
#include "board.h"

CSimpleVideoDecode::CSimpleVideoDecode(
        const char *     name,
        const uint16_t   number,
        CConfiguration * pCfg
        ) :
    CVideoDecode(name, number, pCfg),
    _simpleDecoder(NULL),
    _pBoardResources(NULL),
    _pDecoder(NULL),
    _pVideoWindow(NULL),
    _windowType(eWindowType_Max),
    _pModel(NULL),
    _pChannel(NULL)
{
    eRet ret = eRet_Ok;

    /* manually set type since base class CVideoDecoder constructor will default to
     * regular video decoder type */
    setType(eBoardResource_simpleDecodeVideo);

    NEXUS_SimpleVideoDecoder_GetDefaultStartSettings(&_startSettings);

    BDBG_ASSERT(eRet_Ok == ret);
}

CSimpleVideoDecode::~CSimpleVideoDecode()
{
    close();
}

eRet CSimpleVideoDecode::open(
        CWidgetEngine * pWidgetEngine,
        CStc *          pStc
        )
{
    eRet ret = eRet_Ok;

    BDBG_ASSERT(NULL != pStc);

    if (true == isOpened())
    {
        ret = eRet_InvalidState;
        CHECK_ERROR_GOTO("Simple Video decoder is already opened.", ret, error);
    }

    if (NULL == _pBoardResources)
    {
        ret = eRet_InvalidState;
        CHECK_ERROR_GOTO("Set board resources before opening.", ret, error);
    }

    if (NULL != pWidgetEngine)
    {
        _pWidgetEngine = pWidgetEngine;
        _pWidgetEngine->addCallback(this, CALLBACK_VDECODE_SOURCE_CHANGED, bwinSimpleVideoDecodeSourceChangedCallback);
        _pWidgetEngine->addCallback(this, CALLBACK_VDECODE_STREAM_CHANGED, bwinSimpleVideoDecodeStreamChangedCallback);
    }
    else
    {
        BDBG_MSG(("Opening CSimpleVideoDecode num:%d without providing widgetEngine - decoder callbacks will be ignored", getNumber()));
    }

    _pDecoder = (CVideoDecode *)_pBoardResources->checkoutResource(getCheckedOutId(), eBoardResource_decodeVideo);
    CHECK_PTR_ERROR_GOTO("unable to check out video decoder", _pDecoder, ret, eRet_NotAvailable, error);

    ret = _pDecoder->open(NULL, pStc);
    CHECK_ERROR_GOTO("Video decode failed to open", ret, error);

    {
        CPlatform * pPlatform = _pCfg->getPlatformConfig();
        /* since open() is only called in the nexus atlas (overloaded in nxclient atlas),
         * getNumber() will return the decoder number.  nxclient atlas uses getNumber()
         * to return the nxclient id instead which getDecoderMaxVideoFormat() does not
         * use for its decoder array index. */
        NEXUS_VideoFormat maxFormat = pPlatform->getDecoderMaxVideoFormat(getNumber());

        /* get max width and height supported by video decoder */
        _maxWidth  = videoFormatToHorizRes(maxFormat).toInt();
        _maxHeight = videoFormatToVertRes(maxFormat).toInt();
    }

    /* create simple video decoder */
    {
        NEXUS_SimpleVideoDecoderServerSettings settings;
        NEXUS_SimpleVideoDecoder_GetDefaultServerSettings(&settings);
        settings.videoDecoder = _pDecoder->getDecoder();
        settings.stcIndex     = getNumber();

        _simpleDecoder = NEXUS_SimpleVideoDecoder_Create(_pModel->getSimpleVideoDecoderServer(), getNumber(), &settings);
        CHECK_PTR_ERROR_GOTO("unable to create simple video decoder", _simpleDecoder, ret, eRet_NotAvailable, error);
    }

    ret = setStc(pStc);
    CHECK_ERROR_GOTO("SetStcChannel simple video decoder failed!", ret, error);

    /* set video decoder settings */
    {
        NEXUS_Error                nError = NEXUS_SUCCESS;
        NEXUS_VideoDecoderSettings videoDecoderSettings;

        NEXUS_SimpleVideoDecoder_GetSettings(_simpleDecoder, &videoDecoderSettings);
        videoDecoderSettings.streamChanged.callback = NexusSimpleVideoDecodeStreamChangedCallback;
        videoDecoderSettings.streamChanged.context  = this;
        videoDecoderSettings.streamChanged.param    = 0;
        videoDecoderSettings.sourceChanged.callback = NexusSimpleVideoDecodeSourceChangedCallback;
        videoDecoderSettings.sourceChanged.context  = this;
        videoDecoderSettings.sourceChanged.param    = 0;
        videoDecoderSettings.dropFieldMode          = GET_BOOL(_pCfg, DROP_FIELD_MODE_ENABLED);
        nError = NEXUS_SimpleVideoDecoder_SetSettings(_simpleDecoder, &videoDecoderSettings);
        CHECK_NEXUS_ERROR_GOTO("Error applying simple video decoder settings.", ret, nError, error);
    }

    /* set default noise reduction modes */
    {
        int noiseReduction = -1;

        noiseReduction = GET_INT(_pCfg, DNR_DEFAULT_BNR_MODE);
        if (-1 < noiseReduction)
        {
            BDBG_WRN(("setting DNR Block Noise Mode:%s", noiseReductionModeToString((NEXUS_VideoWindowFilterMode)noiseReduction).s()));
            ret = setDnrBlockNoiseMode((NEXUS_VideoWindowFilterMode)noiseReduction);
            CHECK_ERROR_GOTO("unable to set default DNR BNR mode", ret, error);
        }

        noiseReduction = GET_INT(_pCfg, DNR_DEFAULT_MNR_MODE);
        if (-1 < noiseReduction)
        {
            BDBG_WRN(("setting DNR Mosquito Noise Mode:%s", noiseReductionModeToString((NEXUS_VideoWindowFilterMode)noiseReduction).s()));
            ret = setDnrMosquitoNoiseMode((NEXUS_VideoWindowFilterMode)noiseReduction);
            CHECK_ERROR_GOTO("unable to set default DNR MNR mode", ret, error);
        }

        noiseReduction = GET_INT(_pCfg, DNR_DEFAULT_DCR_MODE);
        if (-1 < noiseReduction)
        {
            BDBG_WRN(("setting DNR Digital Contour Mode:%s", noiseReductionModeToString((NEXUS_VideoWindowFilterMode)noiseReduction).s()));
            ret = setDnrContourMode((NEXUS_VideoWindowFilterMode)noiseReduction);
            CHECK_ERROR_GOTO("unable to set default DNR DCR mode", ret, error);
        }

        noiseReduction = GET_INT(_pCfg, ANR_DEFAULT_MODE);
        if (-1 < noiseReduction)
        {
            BDBG_WRN(("setting ANR Noise Mode:%s", noiseReductionModeToString((NEXUS_VideoWindowFilterMode)noiseReduction).s()));
            ret = setAnrMode((NEXUS_VideoWindowFilterMode)noiseReduction);
            CHECK_ERROR_GOTO("unable to set default ANR mode", ret, error);
        }
    }

error:
    return(ret);
} /* open */

CStc * CSimpleVideoDecode::close()
{
    CStc * pStc = _pStc;

    if (NULL != _simpleDecoder)
    {
        NEXUS_SimpleVideoDecoderServerSettings settings;

        /* Disconnect the STC */
        setStc(NULL);

        NEXUS_SimpleVideoDecoder_GetDefaultServerSettings(&settings);
        settings.enabled = false;
        NEXUS_SimpleVideoDecoder_SetServerSettings(_pModel->getSimpleVideoDecoderServer(), _simpleDecoder, &settings);

        NEXUS_SimpleVideoDecoder_Destroy(_simpleDecoder);
        _simpleDecoder = NULL;
    }

    if (NULL != _pDecoder)
    {
        _pDecoder->close();
        _pBoardResources->checkinResource(_pDecoder);
        _pDecoder = NULL;
    }

    _videoWindowList.clear();

    if (NULL != _pWidgetEngine)
    {
        _pWidgetEngine->removeCallback(this, CALLBACK_VDECODE_STREAM_CHANGED);
        _pWidgetEngine->removeCallback(this, CALLBACK_VDECODE_SOURCE_CHANGED);
        _pWidgetEngine = NULL;
    }

    return(pStc);
} /* close */

eRet CSimpleVideoDecode::start(
        CPid * pPid,
        CStc * pStc
        )
{
    eRet        ret    = eRet_Ok;
    NEXUS_Error nerror = NEXUS_SUCCESS;

    BDBG_ASSERT(true == isOpened());

    if (NULL == pPid)
    {
        /* if no pid given use saved pid */
        pPid = getPid();
    }

    ret = setStc(pStc);
    CHECK_ERROR_GOTO("SetStcChannel simple video decoder failed!", ret, error);

#if 0
    /* TODO: giving incorrect support info so disable check for now */
    if (false == isCodecSupported(pPid->getVideoCodec()))
    {
        BDBG_WRN(("current decoder does not support video codec:%d", pPid->getVideoCodec()));
        ret = eRet_NotSupported;
        goto error;
    }
#endif /* if 0 */

    NEXUS_SimpleVideoDecoder_GetDefaultStartSettings(&_startSettings);
    _startSettings.settings.codec      = pPid->getVideoCodec();
    _startSettings.settings.pidChannel = pPid->getPidChannel();
    _startSettings.settings.frameRate  = pPid->getVideoFrameRate();
    if ((0 < _maxWidth) && (0 < _maxHeight))
    {
        _startSettings.maxWidth  = _maxWidth;
        _startSettings.maxHeight = _maxHeight;
    }

    BDBG_MSG(("Simple Video Decoder set maxWidth:%d maxHeight:%d", _maxWidth, _maxHeight));
    nerror = NEXUS_SimpleVideoDecoder_Start(_simpleDecoder, &_startSettings);
    CHECK_NEXUS_ERROR_GOTO("starting simple video decoder failed!", ret, nerror, error);

    /* save pid */
    _pPid    = pPid;
    _started = true;

    notifyObservers(eNotify_VideoDecodeStarted, this);
    notifyObservers(eNotify_DecodeStarted);
error:
    return(ret);
} /* start */

CPid * CSimpleVideoDecode::stop()
{
    CPid * pPid = NULL;

    if (false == isStarted())
    {
        BDBG_MSG(("Attempting to stop simple video decode that is already stopped."));
        goto error;
    }

    BDBG_ASSERT(NULL != _simpleDecoder);
    /* we use StopAndFree instead of Stop here to handle the case where the video
     * decoder maxWidth/maxHeight may have changed and we want nexus to reconfigure
     * memory usage.  this is to accomodate various box modes that limit
     * decoder usage based on source content. (i.e. allow one 4k decode or two 1080p decodes) */
    NEXUS_SimpleVideoDecoder_StopAndFree(_simpleDecoder);

    setStc(NULL);

    /* return pid */
    pPid     = _pPid;
    _started = false;

    notifyObservers(eNotify_VideoDecodeStopped, this);
    notifyObservers(eNotify_DecodeStopped);

error:
    return(pPid);
} /* stop */

eRet CSimpleVideoDecode::getStatus(NEXUS_VideoDecoderStatus * pStatus)
{
    eRet        ret    = eRet_Ok;
    NEXUS_Error nerror = NEXUS_SUCCESS;

    BDBG_ASSERT(NULL != pStatus);

    nerror = NEXUS_SimpleVideoDecoder_GetStatus(getSimpleDecoder(), pStatus);
    CHECK_NEXUS_ERROR_GOTO("getting simple video decoder status failed!", ret, nerror, error);

error:
    return(ret);
}

eRet CSimpleVideoDecode::getStreamInfo(NEXUS_VideoDecoderStreamInformation * pStream)
{
    eRet ret = eRet_Ok;

    BDBG_ASSERT(NULL != pStream);

    NEXUS_SimpleVideoDecoder_GetStreamInformation(getSimpleDecoder(), pStream);

    return(ret);
}

bool CSimpleVideoDecode::isCodecSupported(NEXUS_VideoCodec codec)
{
    NEXUS_VideoDecoderSettings settings;

    NEXUS_SimpleVideoDecoder_GetSettings(_simpleDecoder, &settings);
    return(NEXUS_VideoCodec_eNone != settings.supportedCodecs[codec]);
}

#define CHECK_FORMAT(h, i, f)                                \
    do {                                                     \
        if (((h) <= status.source.height) &&                 \
            ((i) == formatInfo.interlaced)) {                \
            if (NULL == pOutput) {                           \
                *pFormat = (f);                              \
                goto done;                                   \
            }                                                \
            else /* output given so check format validity */ \
            if (true == pOutput->isValidVideoFormat((f))) {  \
                *pFormat = (f);                              \
                goto done;                                   \
            }                                                \
        }                                                    \
    } while (0)

eRet CSimpleVideoDecode::getOptimalVideoFormat(
        COutput *           pOutput,
        NEXUS_VideoFormat * pFormat
        )
{
    NEXUS_VideoDecoderStatus status;
    NEXUS_VideoFormatInfo    formatInfo;
    bool                     isPal = false;
    eRet                     ret   = eRet_Ok;

    getStatus(&status);

    NEXUS_VideoFormat_GetInfo(status.format, &formatInfo);
    if ((5000 == formatInfo.verticalFreq) || (2500 == formatInfo.verticalFreq))
    {
        isPal = true;
    }

    BDBG_MSG(("%s:%dx%d", __FUNCTION__, status.source.width, status.source.height));

    if (240 > status.source.height)
    {
        /* current source height is too small */
        ret = eRet_NotAvailable;
        goto error;
    }

    if (false == isPal)
    {
        if (2160 <= _maxWidth)
        {
            CHECK_FORMAT(2160, false, NEXUS_VideoFormat_e3840x2160p60hz);
            CHECK_FORMAT(2160, false, NEXUS_VideoFormat_e3840x2160p30hz);
            CHECK_FORMAT(2160, false, NEXUS_VideoFormat_e3840x2160p24hz);
        }
        CHECK_FORMAT(1080, false, NEXUS_VideoFormat_e1080p);
        CHECK_FORMAT(1080, true, NEXUS_VideoFormat_e1080i);
        CHECK_FORMAT(720, false, NEXUS_VideoFormat_e720p);
        CHECK_FORMAT(576, false, NEXUS_VideoFormat_e576p);
        CHECK_FORMAT(576, true, NEXUS_VideoFormat_ePal);
        CHECK_FORMAT(480, false, NEXUS_VideoFormat_e480p);
        CHECK_FORMAT(status.source.height, formatInfo.interlaced, NEXUS_VideoFormat_eNtsc);
        CHECK_FORMAT(status.source.height, formatInfo.interlaced, NEXUS_VideoFormat_eNtscJapan);
        CHECK_FORMAT(status.source.height, formatInfo.interlaced, NEXUS_VideoFormat_ePalM);
    }
    else /* Pal */
    {
        if (2160 <= _maxWidth)
        {
            CHECK_FORMAT(2160, false, NEXUS_VideoFormat_e3840x2160p50hz);
            CHECK_FORMAT(2160, false, NEXUS_VideoFormat_e3840x2160p25hz);
        }
        CHECK_FORMAT(1080, false, NEXUS_VideoFormat_e1080p50hz);
        CHECK_FORMAT(1080, true, NEXUS_VideoFormat_e1080i50hz);
        CHECK_FORMAT(720, false, NEXUS_VideoFormat_e720p50hz);
        CHECK_FORMAT(576, false, NEXUS_VideoFormat_e576p);
        CHECK_FORMAT(576, true, NEXUS_VideoFormat_ePal);
        CHECK_FORMAT(status.source.height, false, NEXUS_VideoFormat_e576p);
        CHECK_FORMAT(status.source.height, formatInfo.interlaced, NEXUS_VideoFormat_ePal);
        CHECK_FORMAT(status.source.height, formatInfo.interlaced, NEXUS_VideoFormat_ePalN);
        CHECK_FORMAT(status.source.height, formatInfo.interlaced, NEXUS_VideoFormat_ePalNc);
        CHECK_FORMAT(status.source.height, formatInfo.interlaced, NEXUS_VideoFormat_eSecam);
    }

error:
    /* did not find a suitable video format to match source */
    *pFormat = status.format;
    ret      = eRet_NotAvailable;

done:
    return(ret);
} /* getOptimalVideoFormat */

#include "nexus_simple_video_decoder_server.h"

eRet CSimpleVideoDecode::addVideoWindow(CVideoWindow * pVideoWindow)
{
    NEXUS_SimpleVideoDecoderServerSettings settings;
    CPid *      pPid    = NULL;
    bool        restart = false;
    int         i       = 0;
    eRet        ret     = eRet_ExternalError;
    NEXUS_Error nerror  = NEXUS_SUCCESS;

    BDBG_ASSERT(NULL != pVideoWindow);

    if (true == _started)
    {
        pPid    = stop();
        restart = true;
    }

    NEXUS_SimpleVideoDecoder_GetServerSettings(_pModel->getSimpleVideoDecoderServer(), getSimpleDecoder(), &settings);
    for (i = 0; i < NEXUS_MAX_DISPLAYS; i++)
    {
        if (NULL == settings.window[i])
        {
            break;
        }
    }

    if (NEXUS_MAX_DISPLAYS > i)
    {
        settings.window[i] = pVideoWindow->getWindow();
        _videoWindowList.add(pVideoWindow);
        nerror = NEXUS_SimpleVideoDecoder_SetServerSettings(_pModel->getSimpleVideoDecoderServer(), getSimpleDecoder(), &settings);
        CHECK_NEXUS_ERROR_GOTO("unable to set new video window in simple decoder server settings", ret, nerror, done);
    }

done:
    if (true == restart)
    {
        start(pPid);
    }

    return(ret);
} /* addVideoWindow */

void CSimpleVideoDecode::removeVideoWindow(CVideoWindow * pVideoWindow)
{
    BDBG_ASSERT(NULL != pVideoWindow);
    _videoWindowList.remove(pVideoWindow);
}

/* swap the video window lists between the 2 simple video decoders.
 * note that this does not change the underlying connections in nexus,
 * it only changes CSimpleVideoDecoder's view. this is really only
 * used when a swap occurs in nexus and we must perform a corresponding
 * swap in CSimpleVideoDecode to match (see CControl::swapPip())
 */
void CSimpleVideoDecode::swapVideoWindowLists(CSimpleVideoDecode * pVideoDecodeOther)
{
    MList<CVideoWindow> * pVideoWindowListOther;
    MList<CVideoWindow>   videoWindowListTmp;

    BDBG_ASSERT(NULL != pVideoDecodeOther);

    /* get other video window list */
    pVideoWindowListOther = pVideoDecodeOther->getVideoWindowList();

    /* copy pVideoWindowListOther to videoWindowListTmp */
    videoWindowListTmp.clear();
    while (0 < pVideoWindowListOther->total())
    {
        videoWindowListTmp.add(pVideoWindowListOther->remove(0));
    }

    /* copy _videoWindowList to pVideoWindowListOther */
    pVideoWindowListOther->clear();
    while (0 < _videoWindowList.total())
    {
        pVideoWindowListOther->add(_videoWindowList.remove(0));
    }

    /* copy videoWindowListTmp to _videoWindowList */
    _videoWindowList.clear();
    while (0 < videoWindowListTmp.total())
    {
        _videoWindowList.add(videoWindowListTmp.remove(0));
    }
} /* swapVideoWindowLists */

eRet CSimpleVideoDecode::setStc(CStc * pStc)
{
    eRet        ret    = eRet_Ok;
    NEXUS_Error nerror = NEXUS_SUCCESS;

    nerror = NEXUS_SimpleVideoDecoder_SetStcChannel(_simpleDecoder, (NULL != pStc) ? pStc->getSimpleStcChannel() : NULL);
    CHECK_NEXUS_ERROR_GOTO("SetStcChannel simple video decoder failed!", ret, nerror, error);

    _pStc = pStc;

error:
    return(ret);
}

eRet CSimpleVideoDecode::setMaxSize(
        uint16_t width,
        uint16_t height
        )
{
    eRet     ret               = eRet_Ok;
    uint16_t maxWidthPlatform  = 0;
    uint16_t maxHeightPlatform = 0;

    if ((0 == width) || (0 == height))
    {
        BDBG_MSG(("Invalid width/height given so set to default 1920x1080"));
        width  = 1920;
        height = 1080;
    }

    {
        CPlatform * pPlatform = _pCfg->getPlatformConfig();
        /* we use getWindowType() here because getDecoderMaxVideoFormat() takes the decoder
         * number as a parameter.  getNumber() returns the video decoder ID in nxclient atlas
         * which can be just about anything. */
        NEXUS_VideoFormat maxFormatPlatform = pPlatform->getDecoderMaxVideoFormat(getWindowType());

        maxWidthPlatform  = videoFormatToHorizRes(maxFormatPlatform).toInt();
        maxHeightPlatform = videoFormatToVertRes(maxFormatPlatform).toInt();
    }

    /* limit maxWidthPlatform to 1080p width unless a higher resolution is requested and allowed by the platform */
    if (1920 >= width)
    {
        _maxWidth = MIN(1920, maxWidthPlatform);
    }
    else
    {
        _maxWidth = MIN(width, maxWidthPlatform);
    }
    /* limit maxHeightPlatform to 1080p height unless a higher resolution is requested and allowed by the platform */
    if (1080 >= height)
    {
        _maxHeight = MIN(1080, maxHeightPlatform);
    }
    else
    {
        _maxHeight = MIN(height, maxHeightPlatform);
    }

    BDBG_MSG(("video decode setMaxSize w:%d h:%d", _maxWidth, _maxHeight));

    return(ret);
} /* setMaxSize */

eRet CSimpleVideoDecode::setColorDepth(uint8_t depth)
{
    eRet                       ret    = eRet_Ok;
    NEXUS_Error                nError = NEXUS_SUCCESS;
    NEXUS_VideoDecoderSettings videoDecoderSettings;

    /* set color depth in primary video decoder only */
    NEXUS_SimpleVideoDecoder_GetSettings(_simpleDecoder, &videoDecoderSettings);
    videoDecoderSettings.colorDepth = depth;
    nError                          = NEXUS_SimpleVideoDecoder_SetSettings(_simpleDecoder, &videoDecoderSettings);
    CHECK_NEXUS_ERROR_GOTO("Error applying simple video decoder color depth settings.", ret, nError, error);

error:
    return(ret);
}

eRet CSimpleVideoDecode::setDnrBlockNoiseMode(NEXUS_VideoWindowFilterMode mode)
{
    eRet        ret    = eRet_Ok;
    NEXUS_Error nerror = NEXUS_SUCCESS;
    /* coverity[stack_use_local_overflow] */
    NEXUS_SimpleVideoDecoderPictureQualitySettings settings;

    NEXUS_SimpleVideoDecoder_GetPictureQualitySettings(_simpleDecoder, &settings);
    settings.dnr.bnr.mode = mode;
    nerror                = NEXUS_SimpleVideoDecoder_SetPictureQualitySettings(_simpleDecoder, &settings);
    CHECK_NEXUS_ERROR_GOTO("unable to set DNR Block Noise Reduction", ret, nerror, error);

error:
    return(ret);
}

NEXUS_VideoWindowFilterMode CSimpleVideoDecode::getDnrBlockNoiseMode(void)
{
    /* coverity[stack_use_local_overflow] */
    NEXUS_SimpleVideoDecoderPictureQualitySettings settings;

    NEXUS_SimpleVideoDecoder_GetPictureQualitySettings(_simpleDecoder, &settings);
    return(settings.dnr.bnr.mode);
}

eRet CSimpleVideoDecode::setDnrMosquitoNoiseMode(NEXUS_VideoWindowFilterMode mode)
{
    eRet        ret    = eRet_Ok;
    NEXUS_Error nerror = NEXUS_SUCCESS;
    /* coverity[stack_use_local_overflow] */
    NEXUS_SimpleVideoDecoderPictureQualitySettings settings;

    NEXUS_SimpleVideoDecoder_GetPictureQualitySettings(_simpleDecoder, &settings);
    settings.dnr.mnr.mode = mode;
    nerror                = NEXUS_SimpleVideoDecoder_SetPictureQualitySettings(_simpleDecoder, &settings);
    CHECK_NEXUS_ERROR_GOTO("unable to set DNR Block Noise Reduction", ret, nerror, error);

error:
    return(ret);
}

NEXUS_VideoWindowFilterMode CSimpleVideoDecode::getDnrMosquitoNoiseMode(void)
{
    /* coverity[stack_use_local_overflow] */
    NEXUS_SimpleVideoDecoderPictureQualitySettings settings;

    NEXUS_SimpleVideoDecoder_GetPictureQualitySettings(_simpleDecoder, &settings);
    return(settings.dnr.mnr.mode);
}

eRet CSimpleVideoDecode::setDnrContourMode(NEXUS_VideoWindowFilterMode mode)
{
    eRet        ret    = eRet_Ok;
    NEXUS_Error nerror = NEXUS_SUCCESS;
    /* coverity[stack_use_local_overflow] */
    NEXUS_SimpleVideoDecoderPictureQualitySettings settings;

    NEXUS_SimpleVideoDecoder_GetPictureQualitySettings(_simpleDecoder, &settings);
    settings.dnr.dcr.mode = mode;
    nerror                = NEXUS_SimpleVideoDecoder_SetPictureQualitySettings(_simpleDecoder, &settings);
    CHECK_NEXUS_ERROR_GOTO("unable to set DNR Block Noise Reduction", ret, nerror, error);

error:
    return(ret);
}

NEXUS_VideoWindowFilterMode CSimpleVideoDecode::getDnrContourMode(void)
{
    /* coverity[stack_use_local_overflow] */
    NEXUS_SimpleVideoDecoderPictureQualitySettings settings;

    NEXUS_SimpleVideoDecoder_GetPictureQualitySettings(_simpleDecoder, &settings);
    return(settings.dnr.dcr.mode);
}

eRet CSimpleVideoDecode::setAnrMode(NEXUS_VideoWindowFilterMode mode)
{
    eRet        ret    = eRet_Ok;
    NEXUS_Error nerror = NEXUS_SUCCESS;
    /* coverity[stack_use_local_overflow] */
    NEXUS_SimpleVideoDecoderPictureQualitySettings settings;

    NEXUS_SimpleVideoDecoder_GetPictureQualitySettings(_simpleDecoder, &settings);
    settings.anr.anr.mode = mode;
    nerror                = NEXUS_SimpleVideoDecoder_SetPictureQualitySettings(_simpleDecoder, &settings);
    CHECK_NEXUS_ERROR_GOTO("unable to set ANR mode", ret, nerror, error);

error:
    return(ret);
}

NEXUS_VideoWindowFilterMode CSimpleVideoDecode::getAnrMode(void)
{
    /* coverity[stack_use_local_overflow] */
    NEXUS_SimpleVideoDecoderPictureQualitySettings settings;

    NEXUS_SimpleVideoDecoder_GetPictureQualitySettings(_simpleDecoder, &settings);
    return(settings.anr.anr.mode);
}

#if HAS_VID_NL_LUMA_RANGE_ADJ
eDynamicRange CSimpleVideoDecode::getDynamicRange(void)
{
    eRet                                ret          = eRet_Ok;
    eDynamicRange                       dynamicRange = eDynamicRange_Unknown;
    NEXUS_VideoDecoderStreamInformation streamInfo;

    ret = getStreamInfo(&streamInfo);
    CHECK_ERROR_GOTO("unable to get video decoder stream info", ret, error);

    switch (streamInfo.dynamicMetadataType)
    {
    case NEXUS_VideoDecoderDynamicRangeMetadataType_eDolbyVision:
        dynamicRange = eDynamicRange_DolbyVision;
        break;

    case NEXUS_VideoDecoderDynamicRangeMetadataType_eTechnicolorPrime:
        dynamicRange = eDynamicRange_TechnicolorPrime;
        break;

    default:
    case NEXUS_VideoDecoderDynamicRangeMetadataType_eNone:
        switch(streamInfo.eotf)
        {
        case NEXUS_VideoEotf_eHdr10:
            dynamicRange = eDynamicRange_HDR10;
            break;

        case NEXUS_VideoEotf_eHlg:
            dynamicRange = eDynamicRange_HLG;
            break;

        case NEXUS_VideoEotf_eSdr:
            dynamicRange = eDynamicRange_SDR;
            break;

        default:
            break;
        }
        break;
    }

error:
    return(dynamicRange);
}
#endif

#if HAS_VID_NL_LUMA_RANGE_ADJ
eRet CSimpleVideoDecode::updatePlm()
{
    eRet        ret         = eRet_Ok;
    CDisplay *  pDisplay    = _pModel->getDisplay();
    CPlatform * pPlatform   = _pCfg->getPlatformConfig();
    CChannel *  pChannel    = getChannel();
    eWindowType windowType  = getWindowType();
    unsigned    indexWindow = 0;
    bool        bPlm        = true;

    if (NULL != pDisplay)
    {
        if ((eDynamicRange_SDR == pDisplay->getOutputDynamicRange()) ||
            (eDynamicRange_Unknown == pDisplay->getOutputDynamicRange()))
        {
            /* output is not HDR so do not set PLM settings */
            return(ret);
        }
    }

    if ((eWindowType_Mosaic1 <= windowType) && (eWindowType_Max > windowType))
    {
        indexWindow = windowType - eWindowType_Mosaic1;
    }

    if (NULL != pChannel)
    {
        bPlm = pChannel->isPlmEnabled();
    }

    pPlatform->setPlmLumaRangeAdjVideo(0, indexWindow, bPlm);
    BDBG_MSG(("setPlmLumaRangeAdjVideo() index:%d enabled:%s", indexWindow, bPlm ? "true" : "false"));

    notifyObservers(eNotify_VideoPlmChanged, this);
    return(ret);
}
#endif
