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
#include "board.h"
#include "tsb.h"
#include "dvrmgr.h"
#include <unistd.h> /* file i/o */
#include <fcntl.h>

BDBG_MODULE(atlas_tsb);

/* CTsb Class*/
CTsb::CTsb(
        const char *     name,
        const unsigned   number,
        CConfiguration * pCfg
        ) :
    CResource(name, number, eBoardResource_tsb, pCfg),
    _pParserBand(NULL),
    _pPidMgr(NULL),
    _hTsbService(NULL),
    _serviceCallback(NULL),
    _pDvrMgr(NULL),
    _pSimpleVideoDecode(NULL),
    _pSimpleAudioDecode(NULL),
    _pStc(NULL),
    _pVideoPid(NULL),
    _pAudioPid(NULL),
    _tsbDecode(false),
    _enabled(false)
{
    BDBG_MSG(("CTsb constructor >>>"));
    _windowType = eWindowType_Max;
    _index      = number;
    BDBG_MSG(("CTsb constructor <<<"));
}

CTsb::~CTsb()
{
    BDBG_MSG(("CTsb destructor >>>"));
}

eRet CTsb::start()
{
    B_DVR_TSBServiceInputEsStream    inputEsStream;
    B_DVR_TSBServiceSettings         tsbServiceSettings;
    B_DVR_TSBServiceRequest          tsbServiceRequest;
    NEXUS_PlaybackPidChannelSettings playbackPidSettings;
    NEXUS_TransportCapabilities      xptCapabilities;
    CPid * pPid = NULL;

    BDBG_MSG(("CTsb::start %x >>>", this));
    BDBG_ASSERT(_pDvrMgr);
    BDBG_ASSERT(_pSimpleVideoDecode);
    BDBG_ASSERT(_pStc);
    BDBG_ASSERT(_pPidMgr);
    NEXUS_GetTransportCapabilities(&xptCapabilities);
    memset((void *)&tsbServiceRequest, 0, sizeof(B_DVR_TSBServiceRequest));
    sprintf(tsbServiceRequest.programName, "%s_%u", _pDvrMgr->getTsbPrefix(), _index);
    BDBG_MSG(("TSB program name %s", tsbServiceRequest.programName));
    strcpy(tsbServiceRequest.subDir, _pDvrMgr->getTsbSubDir());
    BDBG_MSG(("TSB SubDir name %s", tsbServiceRequest.subDir));
    tsbServiceRequest.input             = eB_DVR_TSBServiceInputQam;
    tsbServiceRequest.maxTSBBufferCount = _pDvrMgr->getNumTsbBuffers();
    tsbServiceRequest.maxTSBTime        = _pDvrMgr->getTsbWindow();
#if 0
    tsbServiceRequest.recpumpIndex  = NEXUS_ANY_ID;
    tsbServiceRequest.playpumpIndex = NEXUS_ANY_ID;
#endif
    tsbServiceRequest.recpumpIndex  = xptCapabilities.numRecpumps-1 - _index;
    tsbServiceRequest.playpumpIndex = xptCapabilities.numPlaypumps-1-_index;
    tsbServiceRequest.volumeIndex   = _pDvrMgr->getTsbVolume();
    BDBG_MSG(("TSB volume %u", tsbServiceRequest.volumeIndex));
    _hTsbService = B_DVR_TSBService_Open(&tsbServiceRequest);
    BDBG_ASSERT(_hTsbService);
    _serviceCallback = (B_DVR_ServiceCallback)tsb_callback;
    B_DVR_TSBService_InstallCallback(_hTsbService, _serviceCallback, (void *)this);
    BDBG_MSG(("TB_DVR_TSBService_InstallCallback "));
    B_DVR_TSBService_GetSettings(_hTsbService, &tsbServiceSettings);
    if (_pSimpleAudioDecode)
    {
        tsbServiceSettings.tsbPlaybackSettings.simpleAudioDecoder[0] = _pSimpleAudioDecode->getSimpleDecoder();
    }
    tsbServiceSettings.tsbPlaybackSettings.simpleVideoDecoder[0] = _pSimpleVideoDecode->getSimpleDecoder();
    tsbServiceSettings.tsbPlaybackSettings.simpleStcChannel      = _pStc->getSimpleStcChannel();
    tsbServiceSettings.parserBand = _pParserBand->getBand();
    B_DVR_TSBService_SetSettings(_hTsbService, &tsbServiceSettings);
    _tsbPlayback = tsbServiceSettings.tsbPlaybackSettings.playback;
    BDBG_MSG(("B_DVR_TSBService_Get/SetSettings "));

    pPid = _pPidMgr->getPid(0, ePidType_Video);
    memset((void *)&inputEsStream, 0, sizeof(inputEsStream));
    inputEsStream.esStreamInfo.pid              = pPid->getPid();
    inputEsStream.esStreamInfo.pidType          = eB_DVR_PidTypeVideo;
    inputEsStream.esStreamInfo.codec.videoCodec = pPid->getVideoCodec();
    B_DVR_TSBService_AddInputEsStream(_hTsbService, &inputEsStream);
    NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
    playbackPidSettings.pidSettings.pidType                 = NEXUS_PidType_eVideo;
    playbackPidSettings.pidTypeSettings.video.codec         = pPid->getVideoCodec();
    playbackPidSettings.pidTypeSettings.video.index         = true;
    playbackPidSettings.pidTypeSettings.video.simpleDecoder = _pSimpleVideoDecode->getSimpleDecoder();
    _pVideoPid = new CPid(*pPid);
    _pVideoPid->setPidChannel(NEXUS_Playback_OpenPidChannel(_tsbPlayback, pPid->getPid(), &playbackPidSettings));

    pPid = _pPidMgr->getPid(0, ePidType_Audio);
    memset((void *)&inputEsStream, 0, sizeof(inputEsStream));
    inputEsStream.esStreamInfo.pid              = pPid->getPid();
    inputEsStream.esStreamInfo.pidType          = eB_DVR_PidTypeAudio;
    inputEsStream.esStreamInfo.codec.audioCodec = pPid->getAudioCodec();
    B_DVR_TSBService_AddInputEsStream(_hTsbService, &inputEsStream);

    NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
    playbackPidSettings.pidSettings.pidType                     = NEXUS_PidType_eAudio;
    playbackPidSettings.pidSettings.pidTypeSettings.audio.codec = pPid->getAudioCodec();
    if (_pSimpleAudioDecode)
    {
        playbackPidSettings.pidTypeSettings.audio.simpleDecoder = _pSimpleAudioDecode->getSimpleDecoder();
    }
    _pAudioPid = new CPid(*pPid);
    _pAudioPid->setPidChannel(NEXUS_Playback_OpenPidChannel(_tsbPlayback, pPid->getPid(), &playbackPidSettings));

    pPid = _pPidMgr->getPid(0, ePidType_Pcr);
    memset((void *)&inputEsStream, 0, sizeof(inputEsStream));
    inputEsStream.esStreamInfo.pid     = pPid->getPid();
    inputEsStream.esStreamInfo.pidType = eB_DVR_PidTypeData;
    B_DVR_TSBService_AddInputEsStream(_hTsbService, &inputEsStream);
    B_DVR_TSBService_Start(_hTsbService, NULL);
    _enabled = true;
    BDBG_MSG(("CTsb::start >>>"));
    return(eRet_Ok);
} /* start */

eRet CTsb::stop()
{
    eRet ret = eRet_Ok;

    BDBG_MSG(("CTsb::stop >>>"));

    if (!_enabled)
    {
        BDBG_WRN((" Record is not active"));
        ret = eRet_ExternalError;
        goto error;
    }
    B_DVR_TSBService_Stop(_hTsbService);
    B_DVR_TSBService_RemoveCallback(_hTsbService);
    NEXUS_Playback_ClosePidChannel(_tsbPlayback, _pVideoPid->getPidChannel());
    NEXUS_Playback_ClosePidChannel(_tsbPlayback, _pAudioPid->getPidChannel());
    delete _pVideoPid;
    delete _pAudioPid;
    _pVideoPid   = NULL;
    _pAudioPid   = NULL;
    _tsbPlayback = NULL;
    B_DVR_TSBService_Close(_hTsbService);
    _enabled   = false;
    _tsbDecode = false;
error:
    BDBG_MSG(("CTsb::stop >>>"));
    return(ret);
} /* stop */

eRet CTsb::play()
{
    B_DVR_OperationSettings operationSettings;

    BDBG_MSG(("CTsb::play >>>"));
    if (!_tsbDecode)
    {
        BDBG_WRN(("tsbDecode inactive"));
    }
    else
    {
        memset(&operationSettings, 0, sizeof(operationSettings));
        operationSettings.operation = eB_DVR_OperationPlay;
        B_DVR_TSBService_SetOperation(_hTsbService, &operationSettings);
        _tsbState = "Play";
        notifyObservers(eNotify_TsbStateChanged, this);
    }
    BDBG_MSG(("CTsb::play <<<"));
    return(eRet_Ok);
} /* play */

eRet CTsb::pause()
{
    B_DVR_OperationSettings operationSettings;

    BDBG_MSG(("CTsb::pause >>>"));
    tsbDecode();
    memset(&operationSettings, 0, sizeof(operationSettings));
    operationSettings.operation = eB_DVR_OperationPause;
    B_DVR_TSBService_SetOperation(_hTsbService, &operationSettings);
    _tsbState = "Pause";
    notifyObservers(eNotify_TsbStateChanged, this);
    BDBG_MSG(("CTsb::pause <<<"));
    return(eRet_Ok);
}

eRet CTsb::forward()
{
    BDBG_MSG(("CTsb::forward >>>"));
    if (!_tsbDecode)
    {
        BDBG_MSG(("tsbDecode inactive"));
    }
    else
    {
        B_DVR_OperationSettings operationSettings;
        operationSettings.operation      = eB_DVR_OperationFastForward;
        operationSettings.operationSpeed = 4;
        B_DVR_TSBService_SetOperation(_hTsbService, &operationSettings);
        _tsbState = "Forward";
        notifyObservers(eNotify_TsbStateChanged, this);
    }
    BDBG_MSG(("CTsb::forward <<<"));
    return(eRet_Ok);
} /* forward */

eRet CTsb::rewind()
{
    B_DVR_OperationSettings operationSettings;

    BDBG_MSG(("CTsb::rewind >>>"));
    tsbDecode();
    memset(&operationSettings, 0, sizeof(operationSettings));
    operationSettings.operation      = eB_DVR_OperationFastRewind;
    operationSettings.operationSpeed = 4;
    B_DVR_TSBService_SetOperation(_hTsbService, &operationSettings);
    _tsbState = "Rewind";
    notifyObservers(eNotify_TsbStateChanged, this);
    BDBG_MSG(("CTsb::rewind <<<"));
    return(eRet_Ok);
}

eRet CTsb::slowForward()
{
    BDBG_MSG(("CTsb::slowForward >>>"));
    if (!_tsbDecode)
    {
        BDBG_MSG(("tsbDecode inactive"));
    }
    else
    {
        B_DVR_TSBServiceStatus  tsbServiceStatus;
        B_DVR_OperationSettings operationSettings;
        B_DVR_TSBService_GetStatus(_hTsbService, &tsbServiceStatus);
        if (tsbServiceStatus.state == NEXUS_PlaybackState_ePaused)
        {
            operationSettings.operation = eB_DVR_OperationForwardFrameAdvance;
            B_DVR_TSBService_SetOperation(_hTsbService, &operationSettings);
            _tsbState = "Pause";
        }
        else
        {
            operationSettings.operation      = eB_DVR_OperationSlowForward;
            operationSettings.operationSpeed = 4;
            B_DVR_TSBService_SetOperation(_hTsbService, &operationSettings);
            _tsbState = "SlowForward";
        }
        notifyObservers(eNotify_TsbStateChanged, this);
    }
    BDBG_MSG(("CTsb::slowForward <<<"));
    return(eRet_Ok);
} /* forward */

eRet CTsb::slowRewind()
{
    B_DVR_OperationSettings operationSettings;
    B_DVR_TSBServiceStatus  tsbServiceStatus;

    BDBG_MSG(("CTsb::slowRewind >>>"));

    B_DVR_TSBService_GetStatus(_hTsbService, &tsbServiceStatus);
    tsbDecode();
    memset(&operationSettings, 0, sizeof(operationSettings));
    if (tsbServiceStatus.state == NEXUS_PlaybackState_ePaused)
    {
        operationSettings.operation = eB_DVR_OperationReverseFrameAdvance;
        B_DVR_TSBService_SetOperation(_hTsbService, &operationSettings);
        _tsbState = "Pause";
    }
    else
    {
        operationSettings.operation      = eB_DVR_OperationSlowRewind;
        operationSettings.operationSpeed = 4;
        B_DVR_TSBService_SetOperation(_hTsbService, &operationSettings);
        _tsbState = "SlowRewind";
    }
    notifyObservers(eNotify_TsbStateChanged, this);
    BDBG_MSG(("CTsb::slowRewind <<<"));
    return(eRet_Ok);
} /* slowRewind */

eRet CTsb::getStatus(B_DVR_TSBServiceStatus * pStatus)
{
    BDBG_MSG(("CTsb::getStatus >>>"));
    if (_hTsbService)
    {
        B_DVR_TSBService_GetStatus(_hTsbService, pStatus);
    }
    BDBG_MSG(("CTsb::getStatus <<<"));
    return(eRet_Ok);
}

eRet CTsb::tsbDecode()
{
    BDBG_MSG(("CTsb::tsbDecode >>>"));
    if (!_tsbDecode)
    {
        B_DVR_TSBServiceStatus  tsbServiceStatus;
        B_DVR_OperationSettings operationSettings;
        decodeStop();
        _tsbDecode = true;
        decodeStart();
        B_DVR_TSBService_GetStatus(_hTsbService, &tsbServiceStatus);
        operationSettings.operation = eB_DVR_OperationTSBPlayStart;
        operationSettings.seekTime  = tsbServiceStatus.tsbRecEndTime;
        B_DVR_TSBService_SetOperation(_hTsbService, &operationSettings);
        BDBG_MSG(("tsbDecode activated"));
    }
    else
    {
        BDBG_MSG(("tsbDecode already active"));
    }
    BDBG_MSG(("CTsb::tsbDecode <<<"));
    return(eRet_Ok);
} /* tsbDecode */

eRet CTsb::liveDecode()
{
    BDBG_MSG(("CTsb::liveDecode >>>"));
    if (_tsbDecode)
    {
        B_DVR_OperationSettings operationSettings;
        operationSettings.operation = eB_DVR_OperationTSBPlayStop;
        B_DVR_TSBService_SetOperation(_hTsbService, &operationSettings);
        decodeStop();
        _tsbDecode = false;
        decodeStart();
        BDBG_MSG(("liveDecode activated"));
    }
    else
    {
        BDBG_MSG(("liveDecode already active"));
    }
    BDBG_MSG(("CTsb::liveDecode <<<"));
    return(eRet_Ok);
} /* liveDecode */

void CTsb::tsb_callback(
        void *        context,
        int           index,
        B_DVR_Event   event,
        B_DVR_Service service
        )
{
    CTsb * pTsb = (CTsb *)context;

    BSTD_UNUSED(index);
    BSTD_UNUSED(event);
    BSTD_UNUSED(service);
    BDBG_MSG(("CTsb::tsb_callback >>>"));
    switch (event)
    {
    case eB_DVR_EventStartOfRecording:
    {
        BDBG_MSG(("TSB buffering started"));
    }
    break;
    case eB_DVR_EventEndOfRecording:
    {
        BDBG_MSG(("TSB buffering stopped"));
    }
    break;
    case eB_DVR_EventHDStreamRecording:
    {
        BDBG_MSG(("TSB stream source is HD"));
    }
    break;
    case eB_DVR_EventSDStreamRecording:
    {
        BDBG_MSG(("TSB stream source is SD"));
    }
    break;
    case eB_DVR_EventMediaProbed:
    {
        BDBG_MSG(("TSB is probed and stream parameters are available"));
    }
    break;
    case eB_DVR_EventStartOfPlayback:
    {
        B_DVR_OperationSettings operationSettings;
        B_DVR_TSBServiceStatus  tsbServiceStatus;
        B_DVR_ERROR             rc = B_DVR_SUCCESS;
        BDBG_MSG(("BOF TSB"));
        B_DVR_TSBService_GetStatus(pTsb->_hTsbService, &tsbServiceStatus);
        rc = B_DVR_TSBService_GetIFrameTimeStamp(pTsb->_hTsbService, &tsbServiceStatus.tsbRecStartTime);
        if (rc == B_DVR_SUCCESS)
        {
            operationSettings.operation = eB_DVR_OperationSeek;
            operationSettings.seekTime  = tsbServiceStatus.tsbRecStartTime;
            B_DVR_TSBService_SetOperation(pTsb->_hTsbService, &operationSettings);
            operationSettings.operation = eB_DVR_OperationPlay;
            B_DVR_TSBService_SetOperation(pTsb->_hTsbService, &operationSettings);
            pTsb->_tsbState = "Play";
            pTsb->notifyObservers(eNotify_TsbStateChanged, pTsb);
        }
        else
        {
            BDBG_MSG(("live decode started -> not enough data to start TSB playback"));
            pTsb->liveDecode();
            pTsb->_tsbState = "live";
            pTsb->notifyObservers(eNotify_TsbStateChanged, pTsb);
        }
    }
    break;
    case eB_DVR_EventEndOfPlayback:
    {
        BDBG_MSG(("EOF TSB"));
        BDBG_MSG(("live decode started"));
        pTsb->liveDecode();
        pTsb->_tsbState = "live";
        pTsb->notifyObservers(eNotify_TsbStateChanged, pTsb);
    }
    break;
    case eB_DVR_EventOverFlow:
    {
        BDBG_WRN(("TSB record overflow"));
    }
    break;
    default:
        BDBG_WRN(("invalid event"));
    } /* switch */
    BDBG_MSG(("CTsb::tsb_callback <<<"));
    return;
} /* tsb_callback */

eRet CTsb::decodeStart()
{
    CPid * aPid = NULL;
    CPid * vPid = NULL;

    BDBG_MSG(("CTsb::decodeStart >>>"));
    if (_tsbDecode)
    {
        vPid = _pVideoPid;
        aPid = _pAudioPid;
        _pStc->setTransportType(NEXUS_TransportType_eTs);
        _pStc->setStcType(eStcType_PvrPlayback);
        _pStc->configure();
    }
    else
    {
        vPid = _pPidMgr->getPid(0, ePidType_Video);
        aPid = _pPidMgr->getPid(0, ePidType_Audio);
        _pStc->setStcType(eStcType_ParserBand);
        _pStc->configure(_pPidMgr->getPid(0, ePidType_Pcr));
    }
    if (vPid)
    {
        _pSimpleVideoDecode->start(vPid, _pStc);
    }
    if (aPid && _pSimpleAudioDecode)
    {
        _pSimpleAudioDecode->start(aPid, _pStc);
    }
    BDBG_MSG(("CTsb::decodeStart <<<"));
    return(eRet_Ok);
} /* decodeStart */

eRet CTsb::decodeStop()
{
    CPid * aPid = NULL;
    CPid * vPid = NULL;

    BDBG_MSG(("CTsb::decodeStop %x>>>", this));
    vPid = _pPidMgr->getPid(0, ePidType_Video);
    aPid = _pPidMgr->getPid(0, ePidType_Audio);
    if (vPid)
    {
        NEXUS_VideoDecoderSettings settings;
        NEXUS_SimpleVideoDecoder_GetSettings(_pSimpleVideoDecode->getSimpleDecoder(), &settings);
        settings.channelChangeMode = NEXUS_VideoDecoder_ChannelChangeMode_eHoldUntilTsmLock;
        NEXUS_SimpleVideoDecoder_SetSettings(_pSimpleVideoDecode->getSimpleDecoder(), &settings);
        _pSimpleVideoDecode->stop();
    }
    if (aPid && _pSimpleAudioDecode)
    {
        _pSimpleAudioDecode->stop();
    }
    BDBG_MSG(("CTsb::decodeStop <<<"));
    return(eRet_Ok);
} /* decodeStop */

MString CTsb::getTimeString()
{
    uint32_t seconds = 0;
    uint32_t minutes = 0;
    uint32_t hours   = 0;
    char     buf[64];

    BDBG_MSG(("CTsb::getTimeString >>>"));
    buf[0]  = '\0';
    seconds = _pDvrMgr->getTsbWindow() / 1000;
    hours   = seconds / 3600;
    minutes = (seconds % 3600) / 60;
    seconds = seconds % 60;

    if (0 < hours)
    {
        BKNI_Snprintf(buf, 64, "%d:%02d:%02d", hours, minutes, seconds);
    }
    else
    {
        BKNI_Snprintf(buf, 64, "%d:%02d", minutes, seconds);
    }
    BDBG_MSG(("CTsb::getTimeString <<<"));
    return(MString(buf));
} /* getTimeString */

void CTsb::setSimpleAudioDecode(CSimpleAudioDecode * pSimpleAudioDecode)
{
    _pSimpleAudioDecode = pSimpleAudioDecode;
    if (_pAudioPid && _tsbPlayback)
    {
        NEXUS_PlaybackPidChannelSettings playbackPidSettings;
        NEXUS_Playback_GetPidChannelSettings(_tsbPlayback, _pAudioPid->getPidChannel(), &playbackPidSettings);
        if (_pSimpleAudioDecode)
        {
            playbackPidSettings.pidTypeSettings.audio.simpleDecoder = _pSimpleAudioDecode->getSimpleDecoder();
        }
        else
        {
            playbackPidSettings.pidTypeSettings.audio.simpleDecoder = NULL;
        }
        NEXUS_Playback_SetPidChannelSettings(_tsbPlayback, _pAudioPid->getPidChannel(), &playbackPidSettings);
    }
    return;
} /* setSimpleAudioDecode */