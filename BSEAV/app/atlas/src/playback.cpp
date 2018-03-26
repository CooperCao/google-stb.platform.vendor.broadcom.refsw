/******************************************************************************
 * Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

#include "band.h"
#include "playback.h"
#include "convert.h"
#include "xmltags.h"
#include "mxmlparser.h"
#include "board.h"
#if NEXUS_NUM_DMA_CHANNELS
#include "dma.h"
#endif
#include <unistd.h> /* file i/o */
#include <fcntl.h>

#define CALLBACK_PLAYBACK_BEGINENDOFSTREAM  "CallbackPlaybackBeginEndOfStream"
BDBG_MODULE(atlas_playback);

static void bwinBeginEndOfStreamCallback(
        void *       pObject,
        const char * strCallback
        )
{
    CPlayback * pPlayback = (CPlayback *)pObject;

    BDBG_ASSERT(NULL != pPlayback);
    BSTD_UNUSED(strCallback);

    pPlayback->beginEndOfStreamCallback();
} /* bwinBeginEndOfStreamCallback */

/* Playback EOF Callback */
static void nexusBeginEndOfStreamCallback(
        void * context,
        int    param
        )
{
    CPlayback * pPlayback = (CPlayback *)context;

    BSTD_UNUSED(pPlayback);
    BDBG_ASSERT(NULL != pPlayback);

    pPlayback->setBeginEndCallbackStatus(param);
    if (param)
    {
        BDBG_WRN(("end of stream"));
    }
    else
    {
        BDBG_WRN(("beginning of stream"));
    }

    {
        CWidgetEngine * pWidgetEngine = pPlayback->getWidgetEngine();
        if (NULL != pWidgetEngine)
        {
            pWidgetEngine->syncCallback(pPlayback, CALLBACK_PLAYBACK_BEGINENDOFSTREAM);
        }
    }

    return;
} /* nexusBeginEndOfStreamCallback */

void CPlayback::beginEndOfStreamCallback()
{
    eRet                   ret  = eRet_Ok;
    NEXUS_PlaybackLoopMode mode = NEXUS_PlaybackLoopMode_eMax;
    NEXUS_PlaybackSettings settings;

    settings = getSettings();

    if (0 == getBeginEndCallbackStatus())
    {
        /* beginning of the stream */
        mode = settings.beginningOfStreamAction;
    }
    else
    {
        /* end of the stream */
        mode = settings.endOfStreamAction;
    }

    switch (mode)
    {
    case NEXUS_PlaybackLoopMode_ePause:
        _trickModeRate  = 1.0;
        _trickModeState = ePlaybackTrick_Pause;
        break;

    case NEXUS_PlaybackLoopMode_ePlay:
        _trickModeRate  = 1.0;
        _trickModeState = ePlaybackTrick_PlayNormal;
        break;

    case NEXUS_PlaybackLoopMode_eLoop:
    default:
        break;
    } /* switch */

    ret = notifyObservers(eNotify_PlaybackStateChanged, this);
    CHECK_ERROR_GOTO("error notifying observers", ret, error);

error:
    return;
} /* beginEndOfStreamCallback */

/* CPlaypump Classes */
CPlaypump::CPlaypump(
        const char *     name,
        const unsigned   number,
        CConfiguration * pCfg
        ) :
    CResource(name, number, eBoardResource_playpump, pCfg),
    _file(NULL),
    _customFile(NULL),
    _stickyFile(NULL),
    _playpump(NULL),
    _pBoardResources(NULL),
    _isIp(false),
    _active(false),
    _allocated(false),
    _currentVideo(NULL),
    _trickModeRate(0)
{
    memset(&_playpumpSettings, 0, sizeof(_playpumpSettings));
    memset(&_playpumpOpenSettings, 0, sizeof(_playpumpOpenSettings));

    NEXUS_Playpump_GetDefaultSettings(&_playpumpSettings);
    NEXUS_Playpump_GetDefaultOpenSettings(&_playpumpOpenSettings);
}

CPlaypump::~CPlaypump()
{
    BDBG_MSG(("Closing Playpump %s:%d", (_playpump) ? "Active" : "NotActive", _number));
    close();
}

eRet CPlaypump::close()
{
    if (NULL != _playpump)
    {
        NEXUS_Playpump_Close(_playpump);
        _playpump = NULL;
    }

    return(eRet_Ok);
}

eRet CPlaypump::initialize()
{
    NEXUS_Playpump_GetDefaultOpenSettings(&_playpumpOpenSettings);
    return(eRet_Ok);
}

eRet CPlaypump::uninitialize()
{
    NEXUS_Playpump_GetDefaultOpenSettings(&_playpumpOpenSettings);
    return(eRet_Ok);
}

/* Might need to close playpump resource and split this function */
eRet CPlaypump::open()
{
    eRet ret = eRet_Ok;

    if (_allocated && _playpump)
    {
        NEXUS_Playpump_GetDefaultSettings(&_playpumpSettings);
        if (0 < GET_INT(_pCfg, MAXDATARATE_PLAYBACK))
        {
            BDBG_MSG(("Max Data Rate for PLAYBACK %d changed to:%d", _number, GET_INT(_pCfg, MAXDATARATE_PLAYBACK)));
            _playpumpSettings.maxDataRate = GET_INT(_pCfg, MAXDATARATE_PLAYBACK);
        }
        ret = setSettings(&_playpumpSettings);
        CHECK_ERROR_GOTO("failed reset of playpump", ret, done);
        goto done;
    }

    ret = CResource::initialize();
    CHECK_ERROR_GOTO("resource initialization failed", ret, error);

    /* take any id but remember the atlas resource is _number only, not NEXUS */
    _playpump = NEXUS_Playpump_Open(NEXUS_ANY_ID, &_playpumpOpenSettings);
    CHECK_PTR_ERROR_GOTO("Unable to open playpump", _playpump, ret, eRet_ExternalError, error);

    _allocated = true;

done:
error:
    BDBG_MSG(("Atlas playpump %d", _number));
    return(ret);
} /* open */

/* Start Playpump */
eRet CPlaypump::start()
{
    eRet        ret = eRet_Ok;
    NEXUS_Error rc  = NEXUS_SUCCESS;

    CHECK_PTR_ERROR_GOTO("Nexus Playpump not opened", _playpump, ret, eRet_NotAvailable, error);

    rc = NEXUS_Playpump_Start(_playpump);
    CHECK_NEXUS_ERROR_GOTO("Unable to start Playpump", ret, rc, error);

    BDBG_MSG(("Playpump Start Completed: %d", _number));
error:
    return(ret);
}

eRet CPlaypump::stop()
{
    eRet ret = eRet_Ok;

    CHECK_PTR_ERROR_GOTO("playpump not available ", _playpump, ret, eRet_NotAvailable, error);

    NEXUS_Playpump_Stop(_playpump);
    BDBG_MSG(("Playpump Stop Completed"));
    return(ret);

error:
    return(ret);
} /* stopPlaypump */

void CPlaypump::dump(void)
{
    BDBG_MSG(("Playpump number %d", _number));
}

eRet CPlaypump::setSettings(NEXUS_PlaypumpSettings * playpumpSettings)
{
    NEXUS_Error nerror = NEXUS_SUCCESS;
    eRet        ret    = eRet_Ok;

    BKNI_Memcpy(&_playpumpSettings, playpumpSettings, sizeof(_playpumpSettings));

    /* Overide MaxDataRate */
    if (0 < GET_INT(_pCfg, MAXDATARATE_PLAYBACK))
    {
        BDBG_MSG(("Max Data Rate for PLAYBACK %d changed to:%d", _number, GET_INT(_pCfg, MAXDATARATE_PLAYBACK)));
        _playpumpSettings.maxDataRate = GET_INT(_pCfg, MAXDATARATE_PLAYBACK);
    }
    nerror = NEXUS_Playpump_SetSettings(_playpump, &_playpumpSettings);
    CHECK_NEXUS_ERROR_GOTO("Cannot Set Playpump Settings", ret, nerror, error);

    BDBG_MSG(("Playpump SetSettings Success!"));
error:
    return(ret);
} /* setSettings */

/* CPlayback Class Functions */
CPlayback::CPlayback(
        const char *     name,
        const unsigned   number,
        CConfiguration * pCfg
        ) :
    CResource(name, number, eBoardResource_playback, pCfg),
    _pWidgetEngine(NULL),
    _hIoHandle(0),
    _file(NULL),
    _customFile(NULL),
    _stickyFile(NULL),
    _playback(NULL),
    _pPlaypump(NULL),
    _pBoardResources(NULL),
    _pStc(NULL),
    _isIp(false),
    _active(false),
    _allocated(false),
    _currentVideo(NULL),
    _trickModeRate(0.0),
    _trickModeMaxDecodeRate(2.0),
    _trickModeState(ePlaybackTrick_Stop),
    _beginEndOfStreamStatus(0)
#if (defined NEXUS_HAS_DMA || defined NEXUS_HAS_XPT_DMA)
    , _pDma(NULL)
#endif
{
    _trickModeMaxDecodeRate = GET_DOUBLE(pCfg, TRICK_MODE_MAX_DECODE_RATE);
    if (_trickModeMaxDecodeRate < 1.0)
    {
        _trickModeMaxDecodeRate = 1.0;
    }
    if (_trickModeMaxDecodeRate > 2.0)
    {
        _trickModeMaxDecodeRate = 2.0;
    }

    memset(&_playbackSettings, 0, sizeof(_playbackSettings));
    memset(&_playbackStartSettings, 0, sizeof(_playbackStartSettings));
    memset(&_trickModeSettings, 0, sizeof(_trickModeSettings));

    NEXUS_Playback_GetDefaultSettings(&_playbackSettings);
    NEXUS_Playback_GetDefaultStartSettings(&_playbackStartSettings);
    NEXUS_Playback_GetDefaultTrickModeSettings(&_trickModeSettings);
    _trickModeSettings.maxDecoderRate = _trickModeMaxDecodeRate * NEXUS_NORMAL_PLAY_SPEED;
    _pidMgr.initialize(pCfg);
}

CPlayback::~CPlayback()
{
    close();
}

void CPlayback::dump(void)
{
    BDBG_MSG(("playback number %d", _number));
}

eRet CPlayback::open(CWidgetEngine * pWidgetEngine)
{
    eRet ret = eRet_Ok;

    if (_allocated && (_playback != NULL))
    {
        /*
         * Reset structure ready to use again
         * Initialize Playback Settings
         */
        _pStc = NULL;
        goto done;
    }

    ret = CResource::initialize();
    CHECK_ERROR_GOTO("resource initialization failed", ret, error);

    BDBG_MSG(("INIT PLAYBACK!"));

    _playback = NEXUS_Playback_Create();
    CHECK_PTR_ERROR_GOTO("Unable to open playback", _playback, ret, eRet_ExternalError, error);

    BDBG_MSG(("added playback #%d", _number));
    _allocated = true;

done:
    _pWidgetEngine = pWidgetEngine;
    if (NULL != _pWidgetEngine)
    {
        /* begin/end of stream callback only handled if playback is given a widget engine handle */
        _pWidgetEngine->addCallback(this, CALLBACK_PLAYBACK_BEGINENDOFSTREAM, bwinBeginEndOfStreamCallback);
    }

    if (_pPlaypump == NULL)
    {
        _pPlaypump = (CPlaypump *)_pBoardResources->checkoutResource(getCheckedOutId(), eBoardResource_playpump);
        CHECK_PTR_ERROR_GOTO("Unable to grab playpump resource", _pPlaypump, ret, eRet_ExternalError, error);
    }

    ret = _pPlaypump->open();
    CHECK_ERROR_GOTO("Cannot open Playpump", ret, error);

    /* Initialize Playback Settings */
    NEXUS_Playback_GetDefaultSettings(&_playbackSettings);
    _playbackSettings.playpump                           = _pPlaypump->getPlaypump();
    _playbackSettings.beginningOfStreamAction            = (NEXUS_PlaybackLoopMode)GET_INT(_pCfg, PLAYBACK_BEGIN_STREAM_ACTION);
    _playbackSettings.beginningOfStreamCallback.callback = nexusBeginEndOfStreamCallback;
    _playbackSettings.beginningOfStreamCallback.context  = this;
    _playbackSettings.beginningOfStreamCallback.param    = 0;
    _playbackSettings.endOfStreamAction                  = (NEXUS_PlaybackLoopMode)GET_INT(_pCfg, PLAYBACK_END_STREAM_ACTION);
    _playbackSettings.endOfStreamCallback.callback       = nexusBeginEndOfStreamCallback;
    _playbackSettings.endOfStreamCallback.context        = this;
    _playbackSettings.endOfStreamCallback.param          = 1;
    _playbackSettings.enableStreamProcessing             = false;
    ret = setSettings(&_playbackSettings);
    CHECK_ERROR_GOTO("cannot set playback settings", ret, error);

    BKNI_Memset(&_playbackStartSettings, 0, sizeof(_playbackStartSettings));
    NEXUS_Playback_GetDefaultTrickModeSettings(&_trickModeSettings);
    _trickModeSettings.maxDecoderRate = _trickModeMaxDecodeRate * NEXUS_NORMAL_PLAY_SPEED;

    return(ret);

error:
    close();
    return(ret);
} /* open */

eRet CPlayback::close(CPidMgr * pPidMgr)
{
    eRet ret = eRet_Ok;

    BDBG_MSG(("  %s() STRN CMP value is %s, ", BSTD_FUNCTION, (ePlaybackTrick_Stop != _trickModeState) ? "not-Stopped" : "Stop"));
    stop(pPidMgr);

    if (NULL != _playback)
    {
        NEXUS_Playback_Destroy(_playback);
        _playback = NULL;
    }

    if (NULL != _pPlaypump)
    {
        _pPlaypump->close();
        _pBoardResources->checkinResource(_pPlaypump);
        _pPlaypump = NULL;
    }

#if  defined NEXUS_HAS_DMA || defined NEXUS_HAS_XPT_DMA
    if (_pDma != NULL)
    {
        _pDma->close();
        _pBoardResources->checkinResource(_pDma);
        _pDma = NULL;
    }
#endif /* if  defined NEXUS_HAS_DMA || defined NEXUS_HAS_XPT_DMA */

    _isIp = false;
    return(ret);
} /* close */

eRet CPlayback::setStc(CStc * pStc)
{
    eRet ret = eRet_Ok;

    CHECK_PTR_ERROR_GOTO("NULL CSTC handle", pStc, ret, eRet_ExternalError, error);
    CHECK_PTR_ERROR_GOTO("must set a valid Playback Video", _currentVideo, ret, eRet_ExternalError, error);

    /* Overwrite the current STC handle*/
    _pStc = pStc;

    if (_pStc == NULL)
    {
        _playbackSettings.simpleStcChannel = NULL;
        _playbackSettings.stcTrick         = false;
        ret = setSettings(&_playbackSettings);
        CHECK_ERROR_GOTO("error settings playback settings", ret, error);
        goto done;
    }

    _pStc->setStcType(eStcType_PvrPlayback);
    _pStc->setTransportType(_pidMgr.getTransportType());
    BDBG_MSG(("Transport Type = %d ", _pidMgr.getTransportType()));
    ret = _pStc->configure(NULL);
    CHECK_ERROR_GOTO("error configuring stc channel", ret, error);

    /* We need to detect the way we Set up the STC */
    _playbackSettings.simpleStcChannel               = (NEXUS_SimpleStcChannel *) _pStc->getSimpleStcChannel();
    _playbackSettings.stcTrick                       = true;
    _playbackSettings.playpumpSettings.transportType = _pidMgr.getTransportType();
    ret = setSettings(&_playbackSettings);
    CHECK_ERROR_GOTO("error settings playback settings", ret, error);

done:
error:
    return(ret);
} /* setStc */

void CPlayback::setTS()
{
    _playbackSettings.playpumpSettings.transportType = _pidMgr.getTransportType();
    setSettings(&_playbackSettings);
}

eRet CPlayback::start(NEXUS_FilePlayHandle file)
{
    NEXUS_Error rc = NEXUS_SUCCESS;

    _playbackStartSettings.mpeg2TsIndexType = NEXUS_PlaybackMpeg2TsIndexType_eSelf;
    rc = NEXUS_Playback_Start(_playback, file, &_playbackStartSettings);
    if (rc)
    {
        BDBG_ERR(("NEXUS Error (%d) at %d\n", rc, __LINE__));
        goto error;
    }

    _active = true;
    return(eRet_Ok);

error:
    return(eRet_ExternalError);
} /* start */

eRet CPlayback::start()
{
    if (_currentVideo == NULL)
    {
        BDBG_ERR(("Please setVideo(CVideo *Video) before calling %s ", BSTD_FUNCTION));
        return(eRet_ExternalError);
    }

    return(start(_currentVideo));
}

eRet CPlayback::start(NEXUS_PlaybackPosition pos)
{
    eRet ret;

    if (_currentVideo == NULL)
    {
        BDBG_ERR(("Please setVideo(CVideo *Video) before calling %s ", BSTD_FUNCTION));
        return(eRet_ExternalError);
    }
    ret = start(_currentVideo);
    if (ret == eRet_Ok)
    {
        (void)NEXUS_Playback_Seek(_playback, pos);
    }

    return(ret);
} /* start */

eRet CPlayback::seek(NEXUS_PlaybackPosition pos)
{
    eRet ret = eRet_Ok;

    if (_currentVideo == NULL)
    {
        BDBG_ERR(("Please setVideo(CVideo *Video) before calling %s ", BSTD_FUNCTION));
        return(eRet_ExternalError);
    }
    if (NEXUS_SUCCESS != NEXUS_Playback_Seek(_playback, pos))
    {
        ret = eRet_InvalidState;
    }

    return(ret);
}

/* LUA playback */
eRet CPlayback::start(
        const char *    filename,
        CPlaybackList * pPlaybackList
        )
{
    CVideo * pVideo = NULL;
    eRet     ret    = eRet_Ok;

    BDBG_ASSERT(NULL != filename);
    BDBG_ASSERT(NULL != pPlaybackList);

    /* find corresponding video object in list */
    pVideo = pPlaybackList->find(filename);
    CHECK_PTR_ERROR_GOTO("unable to start playback - given file does not exist in playback list", pVideo, ret, eRet_NotAvailable, error);

    ret = start(pVideo);

error:
    return(ret);
} /* start */

eRet CPlayback::start(CVideo * pVideo)
{
    eRet        ret    = eRet_Ok;
    NEXUS_Error nerror = NEXUS_SUCCESS;
    MString     fullFilePath;
    MString     fullIndexPath;

    if (_currentVideo && (ePlaybackTrick_Stop != _trickModeState))
    {
        BDBG_MSG((" %s() State Trickmode is %d, ", BSTD_FUNCTION, _trickModeState));

        BDBG_MSG(("Resuming Play back"));
        play(ePlaybackTrick_Play, false);
        goto done;
    }

    BDBG_ASSERT(NULL != pVideo);

    /* Now we are using this playback! */
    pVideo->inUse();

    fullFilePath  = pVideo->getVideoNamePath();
    fullIndexPath = pVideo->getIndexNamePath();

    BDBG_MSG(("PLAYBACK START %s, %s", fullFilePath.s(), fullIndexPath.s()));
    BDBG_MSG(("PLAYBACK HAS INDEX? %s", (pVideo->hasIndex()) ? "true" : "false"));
    BDBG_MSG(("PLAYBACK REQUIRES INDEX? %s", (pVideo->isIndexRequired()) ? "true" : "false"));

    if (!pVideo->hasIndex())
    {
        /* no index available - may need to use file as index */
        _file = NEXUS_FilePlay_OpenPosix(fullFilePath.s(), pVideo->isIndexRequired() ? fullFilePath.s() : NULL);
    }
    else
    {
        /* use the provided index, which may be the file itself */
        _file = NEXUS_FilePlay_OpenPosix(fullFilePath.s(), fullIndexPath.s());
        if (NULL == _file)
        {
            BDBG_MSG(("PLAYBACK INDEX NOT FOUND, TRY FILE AS INDEX: %s", fullIndexPath.s()));
            /* index file not available (index generation may be in progress), use file as index */
            _file = NEXUS_FilePlay_OpenPosix(fullFilePath.s(), pVideo->isIndexRequired() ? fullFilePath.s() : NULL);
        }
    }
    if (!_file)
    {
        BDBG_ERR(("can't open files: '%s' '%s'", fullFilePath.s(), fullIndexPath.s()));
        ret = eRet_InvalidParameter;
        goto error;
    }

    NEXUS_Playback_GetSettings(_playback, &_playbackSettings);
    if (true == pVideo->isAudioOnly())
    {
        _playbackSettings.stcTrick = false;
    }
    _playbackSettings.playpumpSettings.timestamp.type = pVideo->isTimestampEnabled() ?
                                                        NEXUS_TransportTimestampType_eMod300 : NEXUS_TransportTimestampType_eNone;
    nerror = NEXUS_Playback_SetSettings(_playback, &_playbackSettings);
    CHECK_NEXUS_ERROR_GOTO("error settings playback settings", ret, nerror, error);

    /* Start playback */
    NEXUS_Playback_GetDefaultStartSettings(&_playbackStartSettings);
    setTrickModeRate(1.0);

    if (fullFilePath == fullIndexPath)
    {
        _playbackStartSettings.bitrate          = NEXUS_PlaybackMode_eAutoBitrate;
        _playbackStartSettings.mpeg2TsIndexType = NEXUS_PlaybackMpeg2TsIndexType_eSelf;
    }

    nerror = NEXUS_Playback_Start(_playback, _file, &_playbackStartSettings);
    CHECK_NEXUS_ERROR_GOTO("error starting playback", ret, nerror, error);

    _active = true;

done:
    setTrickModeRate(1.0);
    _trickModeState = ePlaybackTrick_PlayNormal;
    ret             = notifyObservers(eNotify_PlaybackStateChanged, this);
    CHECK_ERROR_GOTO("error notifying observers", ret, error);

    return(eRet_Ok);

error:

#if  defined NEXUS_HAS_DMA || defined NEXUS_HAS_XPT_DMA
    if (_pDma != NULL)
    {
        _pDma->close();
        _pBoardResources->checkinResource(_pDma);
        _pDma = NULL;
    }
#endif /* if  defined NEXUS_HAS_DMA || defined NEXUS_HAS_XPT_DMA */
    return(ret);
} /* start */

/* Playbck from Video Object */
eRet CPlayback::play(
        ePlaybackTrick playMode,
        bool           bNotify
        )
{
    NEXUS_Error nerror = NEXUS_SUCCESS;
    eRet        ret    = eRet_Ok;

    if ((_currentVideo == NULL) && (_isIp == false))
    {
        BDBG_WRN(("No Video attached to this playback or Video is not currently Playing Back "));
        BDBG_WRN(("isIP %s", (_isIp) ? "true" : "false"));
        return(eRet_ExternalError);
    }

    switch (playMode)
    {
    case ePlaybackTrick_Play:
        nerror = NEXUS_Playback_Play(_playback);
        CHECK_NEXUS_ERROR_GOTO("Can not continue playback", ret, nerror, error);
        setTrickModeRate(1.0);
        break;

    case ePlaybackTrick_PlayNormal:
    {
        NEXUS_PlaybackTrickModeSettings trickSettings;
        NEXUS_Playback_GetDefaultTrickModeSettings(&trickSettings);
        trickSettings.maxDecoderRate = _trickModeMaxDecodeRate * NEXUS_NORMAL_PLAY_SPEED;
        trickSettings.mode           = NEXUS_PlaybackHostTrickMode_eNormal;
        trickSettings.skipControl    = NEXUS_PlaybackSkipControl_eDecoder;
        trickSettings.rateControl    = NEXUS_PlaybackRateControl_eDecoder;
        nerror                       = NEXUS_Playback_TrickMode(_playback, &trickSettings);
        CHECK_NEXUS_ERROR_GOTO("trick mode: PlayNormal failed", ret, nerror, error);
    }
    break;

    case ePlaybackTrick_PlayI:
    {
        NEXUS_PlaybackTrickModeSettings trickSettings;
        NEXUS_Playback_GetDefaultTrickModeSettings(&trickSettings);
        trickSettings.maxDecoderRate = _trickModeMaxDecodeRate * NEXUS_NORMAL_PLAY_SPEED;
        trickSettings.mode           = NEXUS_PlaybackHostTrickMode_ePlayI;
        trickSettings.skipControl    = NEXUS_PlaybackSkipControl_eDecoder;
        trickSettings.rateControl    = NEXUS_PlaybackRateControl_eDecoder;
        nerror                       = NEXUS_Playback_TrickMode(_playback, &trickSettings);
        CHECK_NEXUS_ERROR_GOTO("trick mode: PlayI failed", ret, nerror, error);
    }
    break;

    case ePlaybackTrick_PlayIP:
    {
        NEXUS_PlaybackTrickModeSettings trickSettings;
        NEXUS_Playback_GetDefaultTrickModeSettings(&trickSettings);
        trickSettings.maxDecoderRate = _trickModeMaxDecodeRate * NEXUS_NORMAL_PLAY_SPEED;
        trickSettings.mode           = NEXUS_PlaybackHostTrickMode_ePlayIP;
        trickSettings.skipControl    = NEXUS_PlaybackSkipControl_eDecoder;
        trickSettings.rateControl    = NEXUS_PlaybackRateControl_eDecoder;
        nerror                       = NEXUS_Playback_TrickMode(_playback, &trickSettings);
        CHECK_NEXUS_ERROR_GOTO("trick mode: PlayIP failed", ret, nerror, error);
    }
    break;

    default:
        BDBG_ERR(("Unsupported play mode"));
        ret = eRet_NotSupported;
        goto error;
        break;
    } /* switch */

    setTrickModeRate(1.0);
    _trickModeState = playMode;

    if (true == bNotify)
    {
        ret = notifyObservers(eNotify_PlaybackStateChanged, this);
        CHECK_ERROR_GOTO("error notifying observers", ret, error);
    }

error:
    return(ret);
} /* play */

bool CPlayback::isActive(void)
{
    if ((_currentVideo == NULL) || (_currentVideo->isPlaybackActive() == false))
    {
        return(false);
    }
    else
    {
        return(true);
    }
}

void CPlayback::setVideo(CVideo * pVideo)
{
    _currentVideo = pVideo;
}

bool CPlayback::hasIndex()
{
    bool index = false;

    if (_isIp)
    {
        index = true;
    }
    else
    if (_currentVideo != NULL)
    {
        index = _currentVideo->hasIndex();
    }

    return(index);
}

eRet CPlayback::stop(CPidMgr * pidMgr)
{
    eRet ret = eRet_Ok;

    if ((_isIp == false) && ((_currentVideo == NULL) || (_currentVideo->isPlaybackActive() == false)))
    {
        BDBG_MSG(("Playback is not active"));
        return(eRet_ExternalError);
    }

    BDBG_MSG((" STOPING %s file and its Playback Active %d", _currentVideo->getVideoName().s(), _currentVideo->isPlaybackActive()));

    /* Close all Pids associate with this playback and close the PID channels */
    if (NULL != _playback)
    {
        NEXUS_Playback_Stop(_playback);
    }

    /* IP playback Case*/
    if ((isIp() == true) && (pidMgr != NULL))
    {
        pidMgr->closePlaybackPidChannels(this);
        /* clear internal PidMgr as well, Just in case anything was set */
        _pidMgr.closePlaybackPidChannels(this);
    }

    if (isIp() == false)
    {
        _pidMgr.closePlaybackPidChannels(this);
        if (_file)
        {
            NEXUS_FilePlay_Close(_file);
        }
        _currentVideo->closeVideo();
    }

    _currentVideo = NULL;
    setTrickModeRate(1.0);
    _trickModeState = ePlaybackTrick_Stop;

    /* Clear pids */
    _pidMgr.clearPids();

    if (_isIp != true)
    {
        ret = notifyObservers(eNotify_PlaybackStateChanged, this);
        CHECK_ERROR_GOTO("error notifying observers", ret, error);
    }

error:
    if (NULL != _pWidgetEngine)
    {
        _pWidgetEngine->removeCallback(this, CALLBACK_PLAYBACK_BEGINENDOFSTREAM);
        _pWidgetEngine = NULL;
    }
    return(ret);
} /* stop */

eRet CPlayback::pause(bool bNotify)
{
    eRet        ret    = eRet_Ok;
    NEXUS_Error nerror = NEXUS_SUCCESS;

    nerror = NEXUS_Playback_Pause(_playback);
    CHECK_NEXUS_ERROR_GOTO("Playback Pause Failed", ret, nerror, error);

    _trickModeState = ePlaybackTrick_Pause;

    if (true == bNotify)
    {
        ret = notifyObservers(eNotify_PlaybackStateChanged, this);
        CHECK_ERROR_GOTO("error notifying observers", ret, error);
    }

error:
    return(ret);
} /* pause */

/*
 * Pause (if not already paused) and return playback position.
 */
eRet CPlayback::pause(
        bool                     bNotify,
        NEXUS_PlaybackPosition * pPos
        )
{
    NEXUS_Error nerror = NEXUS_SUCCESS;
    eRet        ret    = eRet_Ok;

    if (!isPaused())
    {
        ret = pause(bNotify);
    }

    if (pPos)
    {
        NEXUS_PlaybackStatus status;
        *pPos  = 0;
        nerror = NEXUS_Playback_GetStatus(_playback, &status);
        if (nerror == NEXUS_SUCCESS)
        {
            *pPos = status.position;
        }
    }
    return(ret);
} /* pause */

/*
 * Needs a little more setup
 * This TrickMode overRides the Regular Trickmode - Command Line ONLY
 */
eRet CPlayback::trickMode(CPlaybackTrickData * pTrickModeData)
{
    eRet        ret    = eRet_Ok;
    NEXUS_Error nerror = NEXUS_SUCCESS;

    switch (pTrickModeData->_command)
    {
    case ePlaybackTrick_Play:
    case ePlaybackTrick_PlayI:
    case ePlaybackTrick_PlayIP:
    case ePlaybackTrick_PlayNormal:
        ret = play(pTrickModeData->_command, false);
        CHECK_ERROR_GOTO("trick mode: play failed", ret, error);
        break;

    case ePlaybackTrick_FrameAdvance:
        nerror = NEXUS_Playback_FrameAdvance(_playback, true);
        CHECK_NEXUS_ERROR_GOTO("trick mode: frame advance failed", ret, nerror, error);
        break;

    case ePlaybackTrick_FrameRewind:
        nerror = NEXUS_Playback_FrameAdvance(_playback, false);
        CHECK_NEXUS_ERROR_GOTO("trick mode: frame rewind failed", ret, nerror, error);
        break;

    case ePlaybackTrick_Pause:
        ret = pause(false);
        CHECK_ERROR_GOTO("trick mode: pause failed", ret, error);
        break;

    case ePlaybackTrick_Rate:
    {
        NEXUS_PlaybackTrickModeSettings trickSettings;
        NEXUS_Playback_GetDefaultTrickModeSettings(&trickSettings);
        trickSettings.maxDecoderRate = _trickModeMaxDecodeRate * NEXUS_NORMAL_PLAY_SPEED;

        trickSettings.rate = NEXUS_NORMAL_DECODE_RATE * pTrickModeData->_rate;
        if (!trickSettings.rate && pTrickModeData->_rate)
        {
            trickSettings.rate = pTrickModeData->_rate > 0 ? 1 : -1;
        }

        BDBG_MSG(("Atlas Trick Mode Rate %f", pTrickModeData->_rate));

        nerror = NEXUS_Playback_TrickMode(_playback, &trickSettings);
        CHECK_NEXUS_ERROR_GOTO("trick mode: Rate failed", ret, nerror, error);

        setTrickModeRate(pTrickModeData->_rate);

        if (1.0 == pTrickModeData->_rate)
        {
            pTrickModeData->_command = ePlaybackTrick_Play;
        }
        else
        if (0.0 < pTrickModeData->_rate)
        {
            pTrickModeData->_command = ePlaybackTrick_FastForward;
        }
        else
        {
            pTrickModeData->_command = ePlaybackTrick_Rewind;
        }
    }
    break;

    case ePlaybackTrick_Host:
    {
        NEXUS_PlaybackTrickModeSettings trickSettings;

        BDBG_WRN(("host(%d,%d,%f)", pTrickModeData->_trick, pTrickModeData->_modeModifier, pTrickModeData->_rate));

        NEXUS_Playback_GetDefaultTrickModeSettings(&trickSettings);
        trickSettings.maxDecoderRate = _trickModeMaxDecodeRate * NEXUS_NORMAL_PLAY_SPEED;
        trickSettings.skipControl    = NEXUS_PlaybackSkipControl_eHost;
        trickSettings.rateControl    = NEXUS_PlaybackRateControl_eDecoder;
        trickSettings.mode_modifier  = pTrickModeData->_modeModifier;
        trickSettings.mode           = pTrickModeData->_trick;
        trickSettings.rate           = NEXUS_NORMAL_DECODE_RATE / pTrickModeData->_rate;
        if (!trickSettings.rate && pTrickModeData->_rate)
        {
            trickSettings.rate = 1;
        }

        setTrickModeRate(1.0 / pTrickModeData->_rate);
        BDBG_MSG(("trickSettings.mode %d", (int)trickSettings.mode));
        BDBG_MSG(("trickSettings.rate %d", trickSettings.rate));

        nerror = NEXUS_Playback_TrickMode(_playback, &trickSettings);
        CHECK_NEXUS_ERROR_GOTO("trick mode: Host failed", ret, nerror, error);
    }
    break;

    case ePlaybackTrick_Seek:
    {
        uint32_t             pos = pTrickModeData->_seekPosition;
        NEXUS_PlaybackStatus status;

        BDBG_WRN(("seek pos:%d", pos));
        nerror = NEXUS_Playback_GetStatus(_playback, &status);
        CHECK_NEXUS_ERROR_GOTO("trick mode: get playback status failed", ret, nerror, error);

        if (status.first > pos)
        {
            pos = status.first;
        }
        else
        if (status.last < pos)
        {
            pos = status.last;
        }

        nerror = NEXUS_Playback_Seek(_playback, pos);
        CHECK_NEXUS_ERROR_GOTO("trick mode: Seek failed", ret, nerror, error);

        /* keep current _trickModeState - just issue notifications */
        goto done;
    }
    break;

    default:
        goto error;
    } /* switch */

    _trickModeState = pTrickModeData->_command;
    goto done;

error:
    ret = play(ePlaybackTrick_Play, false);
    CHECK_ERROR("unable to start playback", ret);

done:
    ret = notifyObservers(eNotify_PlaybackStateChanged, this);
    CHECK_ERROR_GOTO("error notifying observers", ret, error);

    return(ret);
} /* trickMode */

eRet CPlayback::setTrickModeRate(float trickModeRate)
{
    eRet ret = eRet_InvalidParameter;

    if ((GET_INT(_pCfg, TRICK_MODE_RATE_MAX) >= (int)trickModeRate) &&
        (GET_INT(_pCfg, TRICK_MODE_RATE_MIN) <= (int)trickModeRate))
    {
        _trickModeRate = trickModeRate;
        ret            = eRet_Ok;
    }

    if (1 == _trickModeRate)
    {
        _trickModeState = ePlaybackTrick_PlayNormal;
    }
    else
    if (0 > _trickModeRate)
    {
        _trickModeState = ePlaybackTrick_Rewind;
    }
    else
    {
        _trickModeState = ePlaybackTrick_FastForward;
    }

    return(ret);
} /* setTrickModeRate */

eRet CPlayback::trickMode(eKey key)
{
    eRet        ret    = eRet_Ok;
    NEXUS_Error nerror = NEXUS_SUCCESS;

    switch (key)
    {
    case eKey_FastForward:
    {
        if (_trickModeRate == -1)
        {
            /* transition from Rew to FF */
            setTrickModeRate(1);
        }
        else
        if ((_trickModeRate >= 1.0) && (_trickModeRate < 1.2))
        {
            setTrickModeRate(_trickModeRate + 0.1);
        }
        else
        if ((_trickModeRate >= 1.2) && (_trickModeRate < 2.0))
        {
            setTrickModeRate(_trickModeRate + 0.2);
        }
        else
        if (_trickModeRate <= -1.0)
        {
            /* currently rewinding so rewind less */
            setTrickModeRate(_trickModeRate + ABS(_trickModeRate / 2.0));
        }
        else
        if (_trickModeRate >= 2.0)
        {
            setTrickModeRate(_trickModeRate * 2);
        }
    }
    break;
    case eKey_Rewind:
    {
        if (_trickModeRate == 1)
        {
            /* transition from FF to Rew */
            setTrickModeRate(-1);
        }
        else
        if ((_trickModeRate > 1.0) && (_trickModeRate <= 1.21))
        {
            setTrickModeRate(_trickModeRate - 0.1);
        }
        else
        if ((_trickModeRate > 1.2) && (_trickModeRate <= 2.01))
        {
            setTrickModeRate(_trickModeRate - 0.2);
        }
        else
        if (_trickModeRate > 2.01)
        {
            /* currently fast forwarding so fast forward less */
            setTrickModeRate(_trickModeRate - ABS(_trickModeRate / 2.0));
        }
        else
        if (_trickModeRate <= -1.0)
        {
            setTrickModeRate(-1 * ABS(_trickModeRate * 2));
        }
    }
    break;
    case eKey_JumpFwd:
    {
        if (_trickModeState != ePlaybackTrick_Pause)
        {
            if ((_trickModeRate <= -1.0) || (_trickModeRate >= 1.0))
            {
                setTrickModeRate(0.9);
            }
            else
            if ((_trickModeRate > 0.1) && (_trickModeRate < 1.0))
            {
                setTrickModeRate(_trickModeRate - 0.1);
            }
            else
            if ((_trickModeRate > -1.0) && (_trickModeRate <= 0.0))
            {
                setTrickModeRate(_trickModeRate - 0.1);
            }
        }
        else
        {
            nerror = NEXUS_Playback_FrameAdvance(_playback, true);
            CHECK_NEXUS_ERROR_GOTO("trick mode: frame advance failed", ret, nerror, error);
        }
    }
    break;
    case eKey_JumpRev:
    {
        if (_trickModeState != ePlaybackTrick_Pause)
        {
            if ((_trickModeRate <= -1.0) || (_trickModeRate >= 1.0))
            {
                setTrickModeRate(-0.9);
            }
            else
            if ((_trickModeRate < -0.1) && (_trickModeRate > -1.0))
            {
                setTrickModeRate(_trickModeRate + 0.1);
            }
            else
            if ((_trickModeRate < 1.0) && (_trickModeRate >= 0.0))
            {
                setTrickModeRate(_trickModeRate + 0.1);
            }
        }
        else
        {
            nerror = NEXUS_Playback_FrameAdvance(_playback, false);
            CHECK_NEXUS_ERROR_GOTO("trick mode: frame (reverse) failed", ret, nerror, error);
        }
    }
    break;
    default:
        break;
    } /* switch */
    if ((_trickModeRate > 0.99) && (_trickModeRate < 1.01))
    {
        setTrickModeRate(1.0);
    }

    if ((_trickModeState != ePlaybackTrick_Pause))
    {
        if (_trickModeRate == 1.0)
        {
            if (_trickModeState != ePlaybackTrick_PlayNormal)
            {
                /* If Already in Playback Normal State- Ignore Command */
                nerror = NEXUS_Playback_Play(_playback);
                CHECK_NEXUS_ERROR_GOTO("unable to start playback", ret, nerror, error);

                _trickModeState = ePlaybackTrick_PlayNormal;
            }
        }
        else
        {
            NEXUS_Playback_GetDefaultTrickModeSettings(&_trickModeSettings);
            _trickModeSettings.maxDecoderRate = _trickModeMaxDecodeRate * NEXUS_NORMAL_PLAY_SPEED;
            _trickModeSettings.rate           = NEXUS_NORMAL_DECODE_RATE *(_trickModeRate);

            nerror = NEXUS_Playback_TrickMode(_playback, &_trickModeSettings);
            CHECK_NEXUS_ERROR_GOTO("unable to set trick mode", ret, nerror, error);

            BDBG_MSG((" TrickMode %s , rate %d", ((key == eKey_JumpFwd) || (key == eKey_FastForward)) ? "Forward" : "Rewind", _trickModeSettings.rate));
        }

        ret = notifyObservers(eNotify_PlaybackStateChanged, this);
        CHECK_ERROR_GOTO("error notifying observers", ret, error);
    }

error:
    return(ret);
} /* trickMode */

void CPlayback::dupPidMgr(CPidMgr * pPidMgr)
{
    BDBG_ASSERT(NULL != pPidMgr);
    _pidMgr = *pPidMgr;
}

void CPlayback::closePids(void)
{
    _pidMgr.closePlaybackPids(this);
}

NEXUS_PlaybackSettings CPlayback::getSettings(void)
{
    NEXUS_Playback_GetSettings(_playback, &_playbackSettings);
    return(_playbackSettings);
}

eRet CPlayback::setSettings(NEXUS_PlaybackSettings * pPlaybackSettings)
{
    NEXUS_Error nerror = NEXUS_SUCCESS;
    eRet        ret    = eRet_Ok;

    BDBG_ASSERT(NULL != pPlaybackSettings);

    _playbackSettings = *pPlaybackSettings;

    /*
     * Make sure nothing is playing because it will not set the settings
     * should not be needed
     */
    if (ePlaybackTrick_Stop != _trickModeState)
    {
        BDBG_MSG((" %s() State is not stopped, ", BSTD_FUNCTION));
        NEXUS_Playback_Stop(_playback);
    }

    nerror = NEXUS_Playback_SetSettings(_playback, &_playbackSettings);
    CHECK_NEXUS_ERROR_GOTO("cannot setSettings for Playback", ret, nerror, error);

    BDBG_MSG(("Playback SetSettings Success!"));

error:
    return(ret);
} /* setSettings */

MString CPlayback::getVideoName()
{
    return(_currentVideo->getVideoName());
}

MString CPlayback::getTimeString()
{
    eRet                 ret     = eRet_Ok;
    NEXUS_Error          nerror  = NEXUS_SUCCESS;
    uint32_t             seconds = 0;
    uint32_t             minutes = 0;
    uint32_t             hours   = 0;
    MString              strTime;
    NEXUS_PlaybackStatus nPlaybackStatus;

    BDBG_ASSERT(NULL != getPlayback());

    nerror = NEXUS_Playback_GetStatus(getPlayback(), &nPlaybackStatus);
    CHECK_NEXUS_ERROR_GOTO("error getting nexus playback status", ret, nerror, error);

    seconds = nPlaybackStatus.last / 1000;
    hours   = seconds / 3600;
    minutes = (seconds % 3600) / 60;
    seconds = seconds % 60;

    /* add hours */
    strTime = (0 < hours) ? MString(hours) : "";

    /* add minutes */
    if (false == strTime.isEmpty())
    {
        strTime += ":";

        if (10 > minutes)
        {
            strTime += "0";
        }
    }
    strTime += (0 < minutes) ? MString(minutes) : "";

    /* add seconds */
    strTime += ":";
    if (10 > seconds)
    {
        strTime += "0";
    }
    strTime += MString(seconds);

error:
    return(strTime);
} /* getTimeString */

CPid * CPlayback::getPid(
        unsigned index,
        ePidType type
        )
{
    return(_pidMgr.getPid(index, type));
}

CPid * CPlayback::findPid(
        unsigned pidNum,
        ePidType type
        )
{
    return(_pidMgr.findPid(pidNum, type));
}

void CPlayback::printPids(void)
{
    BDBG_MSG((" Playback Pids"));
    _pidMgr.print();
}

uint32_t CPlayback::getMaxDataRate()
{
    uint32_t maxDataRate = 0;

    BDBG_ASSERT(NULL != _pPlaypump);

    if (0 < GET_INT(_pCfg, MAXDATARATE_PLAYBACK))
    {
        CPlatform *              pPlatform = _pCfg->getPlatformConfig();
        NEXUS_PlatformSettings * pSettings = pPlatform->getPlatformSettings();

        /* changed max data rate */
        maxDataRate = pSettings->transportModuleSettings.maxDataRate.playback[getNumber()];
    }
    else
    {
        /* default max data rate */
        NEXUS_PlaypumpSettings nsettings;
        NEXUS_Playpump_GetSettings(_pPlaypump->getPlaypump(), &nsettings);
        maxDataRate = nsettings.maxDataRate;
    }

    return(maxDataRate);
} /* getMaxDataRate */