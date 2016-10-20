/******************************************************************************
 * Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#include "server_udp.h"
#include "streamer_udp.h"
#include "atlas.h"
#include "atlas_cfg.h"
#include "convert.h"
#include "model.h"
#include "videolist.h"
#include "murl.h"
#include "channelmgr.h"
#include "channel.h"
#include "channel_bip.h"

#define CALLBACK_SERVER_STREAMER_STOP  "callbackServerStreamerStop"
#define TRANSCODE_PROFILE_720p_AVC     "720pAvc"
#define TRANSCODE_PROFILE_480p_AVC     "480pAvc"

BDBG_MODULE(atlas_streamer_udp);

eRet CStreamerUdp::openMedia()
{
    eRet            ret                = eRet_Ok;
    BIP_Status      bipStatus          = BIP_SUCCESS;
    CModel *        pModel             = NULL;
    CPlaybackList * pPlaybackList      = NULL;
    MString         mediaDirectoryPath = GET_STR(_pCfg, VIDEOS_PATH); /* we get it every time so that if pCfg changes and the path changes the we get the latest one.*/
    MUrl            mUrl;

    /* Check whether the received request has  valid parameter so that the stream can start streaming.*/
    {
        setStreamerInputType(_url.s());

        if (_streamerInputType == eUdpStreamerInputType_File)
        {
            int     i;
            int     j;
            MString requestedUrlPath;
            MString finalUrlPath;

            mUrl.set(_url.s());
            /*
             * Note: app may retrieve any private/custom app specific Headers from the Request using BIP_HttpRequest_GetNextHeader().
             * Prefix the base media directory path to the URL name as this example is using the absolute file path for stream names so that it becomes unique!
             */
            _mediaFileAbsoloutePath = mediaDirectoryPath.s() + MString(mUrl.path()); /*TODO: MString(mUrl.path()) should not be required , check later.*/
            requestedUrlPath        = MString(mUrl.path());

            /* Setting the flags as per the client request suffixes and creating the requested URL path by removing the suffixes from the url */
            if ((i = requestedUrlPath.findRev(".m3u8")) != -1) /*If client is requesting with HLS support*/
            {
                requestedUrlPath.truncate(i);
            }
            else
            if ((j = requestedUrlPath.findRev(".xcode")) != -1) /*If client is requesting with transcode support*/
            {
                _enableXcode = true;
                requestedUrlPath.truncate(j);
            }

            _mediaFileAbsoloutePath = mediaDirectoryPath.s() + requestedUrlPath;

            if (mUrl.search())
            {
                setProgram(mUrl.search());
            }

            BDBG_MSG(("mUrl  mUrl.path() =|%s|+++++++++++++++++++++search:|%s|\n", mUrl.path(), mUrl.search()));
            BDBG_MSG((BIP_MSG_PRE_FMT " _mediaFileAbsoloutePath --------------------> %s" BIP_MSG_PRE_ARG, _mediaFileAbsoloutePath.s()));
            /* Check whether the stream is available or not */
            pModel        = _pServerUdp->getModel();
            pPlaybackList = pModel->getPlaybackList();
            if (true == _programNumberValid)
            {
                _pVideo = pPlaybackList->find(_mediaFileAbsoloutePath.s(), _programNumber);
            }
            else
            {
                _pVideo = pPlaybackList->find(_mediaFileAbsoloutePath.s(), -1);
            }

            if (NULL == _pVideo)
            {
                ret = eRet_NotAvailable;
                CHECK_ERROR_GOTO("unable to open CStreamerHttp", ret, error);
            }
        }
        else
        if (_streamerInputType == eUdpStreamerInputType_Tuner)
        {
            CChannelMgr * pChannelMgr = NULL;
            CChannel *    pChannel    = NULL;
            pModel = _pServerUdp->getModel();
            BDBG_ASSERT(NULL != pModel);
            _pAtlasId = pModel->getId();
            _pConfig  = pModel->getConfig();

            pChannelMgr = pModel->getChannelMgr();
            BDBG_ASSERT(NULL != pChannelMgr);

            if (_tuneChannelName.isEmpty())
            {
                ret = eRet_ExternalError;
                CHECK_ERROR_GOTO("Do tuner specific operation ------------------------------->", ret, error);
            }

            pChannel = pChannelMgr->findChannel(_tuneChannelName.s());

            if (NULL == pChannel)
            {
                ret = eRet_ExternalError;
                CHECK_ERROR_GOTO("pChannelMgr->findChannel didn't find any valid channel ------------------------------->", ret, error);
            }
            /* Create local copy of the channel object */
            _pChannel = pChannel->createCopy(pChannel);
        }
    }

    return(ret);

error:
    ret = eRet_InvalidParameter;
    return(ret);
} /* openMedia */

/*** define Streamer class ***/
CStreamerUdp::CStreamerUdp(
        CConfiguration * pCfg,
        CServerUdp *     pServer
        ) :
    _mediaFileAbsoloutePath(),
    _pMethodName(NULL),
    _streamerInputType(eUdpStreamerInputType_Unknown),
    _tuneChannelName(),
    _programNumberValid(false),
    _programNumber(0),
    _rangeHeaderPresent(false),
    _rangeStartOffset(0),
    _rangeLength(0),
    _hUdpStreamer(NULL),
    _streamerStarted(false),
    _continuousPlay(true),
    _endOfStreamerRcvd(false),
    _pServerUdp(pServer),
    _pVideo(NULL),
    _pChannel(NULL),
    _pAtlasId(NULL),
    _enableXcode(false),
    _xcodeProfile(NULL),
    _interfaceName("eth0"),
    _pConfig(NULL)
{
    BDBG_ASSERT(NULL != pCfg);
    memset(&_streamerStreamInfo, 0, sizeof(_streamerStreamInfo));

    initialize((CConfiguration *)pCfg);
}

/*** define Streamer class ***/
CStreamerUdp::CStreamerUdp(void) :
    _mediaFileAbsoloutePath(),
    _pMethodName(NULL),
    _streamerInputType(eUdpStreamerInputType_Unknown),
    _tuneChannelName(),
    _programNumberValid(false),
    _programNumber(0),
    _rangeHeaderPresent(false),
    _rangeStartOffset(0),
    _rangeLength(0),
    _hUdpStreamer(NULL),
    _streamerStarted(false),
    _continuousPlay(true),
    _endOfStreamerRcvd(false),
    _pServerUdp(NULL),
    _pVideo(NULL),
    _pChannel(NULL),
    _pAtlasId(NULL),
    _enableXcode(false),
    _xcodeProfile(NULL),
    _pConfig(NULL)
{
    memset(&_streamerStreamInfo, 0, sizeof(_streamerStreamInfo));
    _pCfg = NULL;
}

CStreamerUdp::~CStreamerUdp()
{
}

eRet CStreamerUdp::initialize(CConfiguration * pCfg)
{
    _pCfg = pCfg;
    return(eRet_Ok);
}

static BIP_Status rejectRequestAndSetResponseHeaders(
        CStreamerUdp *         pStreamerUdp,
        BIP_HttpRequestHandle  hHttpRequest,
        BIP_HttpResponseStatus responseStatus
        )
{
    eRet ret = eRet_Ok;

    BSTD_UNUSED(pStreamerUdp);
    BSTD_UNUSED(hHttpRequest);
    BSTD_UNUSED(responseStatus);
    /* create function for rejecting */
error:
    return(ret);
} /* rejectRequestAndSetResponseHeaders */

void CStreamerUdp::setProgram(const char * str)
{
    MString tempString1 = str;
    MString vaiableName;
    MString prgNum;

    int index;

    index = tempString1.find('=', 0, true);

    if (index != -1)
    {
        vaiableName = tempString1.left(index);

        if (!(vaiableName.strncasecmp("program")))
        {
            prgNum = tempString1.mid(index + 1, (tempString1.length() - index)); /* index + 1 to go past the = char.*/
            if (false == prgNum.isEmpty())
            {
                _programNumber      = prgNum.toInt();
                _programNumberValid = true;
            }
        }
    }

    BDBG_MSG((BIP_MSG_PRE_FMT "<---------------------------------------------------------------------->"BIP_MSG_PRE_ARG));
    BDBG_MSG(("_programNumberValid= %d and _programNumber = %d",
              _programNumberValid,
              _programNumber
              ));
} /* setProgram */

void CStreamerUdp::setStreamerInputType(const char * url)
{
    MUrl    mUrl;
    MString tempString;
    int     index;
    MString variableName;

    _streamerInputType = eUdpStreamerInputType_File; /* we default set it as file , out side of this call will do the file specific validation */

    mUrl.set(url);

    tempString = mUrl.query();

    index = tempString.find('?', 0, true);

    if (index != -1)
    {
        variableName = tempString.left(index);
        if (!(strcasecmp(variableName, "/channel")))
        {
            _streamerInputType = eUdpStreamerInputType_Tuner;

            /* if it is tuner channel then set the channelName from the url.*/
            _tuneChannelName = tempString.mid(index + 1, (tempString.length() - index)); /* index + 1 to go past the = char.*/
        }
    }
} /* setStreamerInputType */

eRet CStreamerUdp::open(MString url)
{
    eRet ret = eRet_Ok;

    /* just set the url variable  all the work is done in the Start call for UDP/RTP */

    _url = url;
    return(ret);
}

void CStreamerUdp::close()
{
    DEL(_pChannel);
}

static void bipStreamerStopCallback(
        void * context,
        int    param
        )
{
    CStreamerUdp *  pStreamer     = (CStreamerUdp *)context;
    CServer *       pServer       = pStreamer->getUdpServer();
    CWidgetEngine * pWidgetEngine = pServer->getWidgetEngine();

    pStreamer->setEndOfStreamer();

    BDBG_ASSERT(NULL != pWidgetEngine);
    BSTD_UNUSED(param);
    BDBG_MSG(("bip UDP streamer stop callback"));

    pWidgetEngine->syncCallback(pServer, CALLBACK_SERVER_STREAMER_STOP);
}

void CStreamerUdp::stopAndDestroyBipStreamer()
{
    if (!_hUdpStreamer)
    {
        return;
    }

    /* it may happen that error occured while starting the streamer.*/
    if (_streamerStarted)
    {
        BDBG_MSG((BIP_MSG_PRE_FMT "Stopping Streamer %p" BIP_MSG_PRE_ARG, (void *)this));
        BIP_UdpStreamer_Stop(_hUdpStreamer);
        _streamerStarted = false;
    }

    BDBG_MSG((BIP_MSG_PRE_FMT " Destroying Streamer %p" BIP_MSG_PRE_ARG, (void *)_hUdpStreamer));
    BIP_UdpStreamer_Destroy(_hUdpStreamer);

    return;
} /* stopAndDestroyBipStreamer */

/* THis atlas function will generate all the information for corresponding program number of a stream.
 * The CVideo object for the for a program number of a stream is already determined after receiving the request. */
void CStreamerUdp::setStreamerStreamInfo()
{
    CPidMgr * pPidMagr = NULL;

    if (eUdpStreamerInputType_File == _streamerInputType)
    {
        BDBG_ASSERT(NULL != _pVideo);
        /* Set mediaInfo parameters.*/
        pPidMagr = _pVideo->getPidMgr();
        BDBG_ASSERT(NULL != pPidMagr);

        B_Os_Memset(&_streamerStreamInfo, 0, sizeof(_streamerStreamInfo));

        _streamerStreamInfo.transportType       = pPidMagr->getTransportType();
        _streamerStreamInfo.numberOfTrackGroups = _pVideo->getTotalStreams();
        /* TODO : Later get it from pCPidmgr class    _streamerStreamInfo.numberOfTracks = */
        _streamerStreamInfo.durationInMs              = _pVideo->getDuration();
        _streamerStreamInfo.contentLength             = _pVideo->getSize();
        _streamerStreamInfo.transportTimeStampEnabled = _pVideo->isTimestampEnabled();
    }
    else
    if (eUdpStreamerInputType_Tuner == _streamerInputType)
    {
        BDBG_ASSERT(NULL != _pChannel);

        pPidMagr = _pChannel->getPidMgr();
        BDBG_ASSERT(NULL != pPidMagr);

        B_Os_Memset(&_streamerStreamInfo, 0, sizeof(_streamerStreamInfo));

        _streamerStreamInfo.transportType       = pPidMagr->getTransportType();
        _streamerStreamInfo.numberOfTrackGroups = 1;
    }
} /* setStreamerStreamInfo */

eRet CStreamerUdp::addAllTracks()
{
    eRet                  ret       = eRet_Ok;
    BIP_Status            bipStatus = BIP_SUCCESS;
    CPidMgr *             pPidMgr   = NULL;
    CPid *                pPid      = NULL;
    BIP_StreamerTrackInfo streamerTrackInfo;
    int                   trackIndex = 0;
    uint16_t              pmtPid     = 0;

    /* Set mediaInfo parameters.*/
    if (eUdpStreamerInputType_File == _streamerInputType)
    {
        BDBG_ASSERT(NULL != _pVideo);
        pPidMgr = _pVideo->getPidMgr();
    }
    else
    if (eUdpStreamerInputType_Tuner == _streamerInputType)
    {
        BDBG_ASSERT(NULL != _pChannel);
        pPidMgr = _pChannel->getPidMgr();
    }
    BDBG_ASSERT(NULL != pPidMgr);
    BDBG_ASSERT(NULL != _hUdpStreamer);

    pmtPid = pPidMgr->getPmtPid();

    /* Add PAT & PMT */
    if (pmtPid != 0)
    /* TODO: Since PAT may contain multiple programs, we will need to substitute it w/ a PAT containing just this one PMT */
    {
        B_Os_Memset(&streamerTrackInfo, 0, sizeof(streamerTrackInfo));
        streamerTrackInfo.trackId = 0; /* PAT is always at PID == 0 */
        streamerTrackInfo.type    = BIP_MediaInfoTrackType_eOther;
        bipStatus                 = BIP_UdpStreamer_AddTrack(_hUdpStreamer, &streamerTrackInfo, NULL);
        CHECK_BIP_ERROR_GOTO("BIP_UdpStreamer_AddTrack Failed for PAT", ret, bipStatus, error);

        /* Add PMT */
        B_Os_Memset(&streamerTrackInfo, 0, sizeof(streamerTrackInfo));
        streamerTrackInfo.trackId = pmtPid;
        streamerTrackInfo.type    = BIP_MediaInfoTrackType_ePmt;
        bipStatus                 = BIP_UdpStreamer_AddTrack(_hUdpStreamer, &streamerTrackInfo, NULL);
        CHECK_BIP_ERROR_GOTO("BIP_UdpStreamer_AddTrack Failed for PMT", ret, bipStatus, error);
    }

    /* Add pcr track*/
    B_Os_Memset(&streamerTrackInfo, 0, sizeof(streamerTrackInfo));
    pPid = pPidMgr->getPid(0, ePidType_Pcr);
    /*Pcr pid won't be present for MP4 formats.*/
    if (pPid)
    {
        streamerTrackInfo.trackId = pPid->getPid();
        streamerTrackInfo.type    = BIP_MediaInfoTrackType_ePcr;
        bipStatus                 = BIP_UdpStreamer_AddTrack(_hUdpStreamer, &streamerTrackInfo, NULL);
        CHECK_BIP_ERROR_GOTO("BIP_UdpStreamer_AddTrack Failed for BIP_MediaInfoTrackType_ePcr", ret, bipStatus, error);
    }

    /* Add all video tracks.*/
    trackIndex = 0;
    pPid       = NULL;
    for (trackIndex = 0; (pPid = pPidMgr->getPid(trackIndex, ePidType_Video)) != NULL; trackIndex++)
    {
        B_Os_Memset(&streamerTrackInfo, 0, sizeof(streamerTrackInfo));
        streamerTrackInfo.trackId = pPid->getPid();
        streamerTrackInfo.type    = BIP_MediaInfoTrackType_eVideo;

        streamerTrackInfo.info.video.codec                         = pPid->getVideoCodec();
        streamerTrackInfo.info.video.width                         = pPid->getWidth();
        streamerTrackInfo.info.video.height                        = pPid->getHeight();
        streamerTrackInfo.info.video.colorDepth                    = 0;
        streamerTrackInfo.info.video.pMediaNavFileAbsolutePathName = NULL; /* TODO: once new nav apis are addded this has to be set appropriately.*/
        bipStatus = BIP_UdpStreamer_AddTrack(_hUdpStreamer, &streamerTrackInfo, NULL);
        CHECK_BIP_ERROR_GOTO("BIP_UdpStreamer_AddTrack Failed for BIP_MediaInfoTrackType_eVideo", ret, bipStatus, error);
    }

    /* Add all audio tracks.*/
    trackIndex = 0;
    pPid       = NULL;
    for (trackIndex = 0; (pPid = pPidMgr->getPid(trackIndex, ePidType_Audio)) != NULL; trackIndex++)
    {
        B_Os_Memset(&streamerTrackInfo, 0, sizeof(streamerTrackInfo));
        streamerTrackInfo.trackId = pPid->getPid();
        streamerTrackInfo.type    = BIP_MediaInfoTrackType_eAudio;

        streamerTrackInfo.info.audio.codec = pPid->getAudioCodec();

        bipStatus = BIP_UdpStreamer_AddTrack(_hUdpStreamer, &streamerTrackInfo, NULL);
        CHECK_BIP_ERROR_GOTO("BIP_UdpStreamer_AddTrack Failed for BIP_MediaInfoTrackType_eAudio", ret, bipStatus, error);
    }

    /* Add all anciliary tracks.*/
    trackIndex = 0;
    pPid       = NULL;
    for (trackIndex = 0; (pPid = pPidMgr->getPid(trackIndex, ePidType_Ancillary)) != NULL; trackIndex++)
    {
        B_Os_Memset(&streamerTrackInfo, 0, sizeof(streamerTrackInfo));
        streamerTrackInfo.trackId = pPid->getPid();
        streamerTrackInfo.type    = BIP_MediaInfoTrackType_eOther;

        bipStatus = BIP_UdpStreamer_AddTrack(_hUdpStreamer, &streamerTrackInfo, NULL);
        CHECK_BIP_ERROR_GOTO("BIP_UdpStreamer_AddTrack Failed for BIP_MediaInfoTrackType_eAudio", ret, bipStatus, error);
    }
    return(ret);

error:
    BDBG_ERR(("%s,Error ", __FUNCTION__));

    return(ret);
} /* addAllTracks */

eRet CStreamerUdp::start()
{
    eRet                    ret              = eRet_Ok;
    BIP_Status              bipStatus        = BIP_SUCCESS;
    CChannelBip *           pChannel         = NULL;
    BIP_UdpStreamerProtocol streamerProtocol = BIP_UdpStreamerProtocol_ePlainUdp;

    /* must be whitin 1Mb ( plenty of space)*/
    pChannel = new CChannelBip();
    CHECK_PTR_ERROR_GOTO("Invalid Channel", pChannel, ret, eRet_ExternalError, errorInStreamerStart);

    ret = openMedia();
    CHECK_ERROR_GOTO("cannot open Media", ret, errorInStreamerStart);

    pChannel->setUrl(_url);

    /* Create UdpStreamer */
    {
        BIP_UdpStreamerCreateSettings createStreamerSettings;

        BIP_UdpStreamer_GetDefaultCreateSettings(&createStreamerSettings);
        createStreamerSettings.endOfStreamingCallback.callback = bipStreamerStopCallback;
        createStreamerSettings.endOfStreamingCallback.context  = this;
        _hUdpStreamer = BIP_UdpStreamer_Create(&createStreamerSettings);
        BIP_CHECK_GOTO((_hUdpStreamer), ("BIP_UdpStreamer_Create Failed"), errorInStreamerStart, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus);
        BDBG_MSG((BIP_MSG_PRE_FMT " New BIP_UdpStreamer ... %p" BIP_MSG_PRE_ARG, (void *)_hUdpStreamer));
    }

    /* Populate _mediaInfoStream. */
    setStreamerStreamInfo(); /* this method internally checks whether this is for tuner input or qam input.*/

    if (eUdpStreamerInputType_File == _streamerInputType)
    {
        BIP_StreamerFileInputSettings fileInputSettings;

        BIP_Streamer_GetDefaultFileInputSettings(&fileInputSettings);

        if (_continuousPlay)
        {
            fileInputSettings.enableContinousPlay = true;
        }

        if (_streamerStreamInfo.transportType == NEXUS_TransportType_eTs)
        {
            /* App wants to pace and this is a MPEG2 TS stream, so BIP can pace it using the Playback Channel. Set that option. */
            fileInputSettings.enableHwPacing = GET_BOOL(_pCfg, HTTP_SERVER_ENABLE_HW_PACING);
        }

        bipStatus = BIP_UdpStreamer_SetFileInputSettings(
                _hUdpStreamer,
                _mediaFileAbsoloutePath.s(),
                &_streamerStreamInfo, /* MediaStreamInfo */
                &fileInputSettings
                );
        CHECK_BIP_ERROR_GOTO("BIP_UdpStreamer_SetFileInputSettings Failed", ret, bipStatus, errorInStreamerStart);

        BDBG_MSG((BIP_MSG_PRE_FMT "Method: %s, Absolute URL: %s,Byte Range: %s, TimeOffsetInMsec: start %u, end %u" BIP_MSG_PRE_ARG,
                  _pMethodName, _mediaFileAbsoloutePath.s(),
                  _rangeHeaderPresent ? "Y" : "N",
                  fileInputSettings.beginTimeOffsetInMs, fileInputSettings.endTimeOffsetInMs
                  ));

        BDBG_MSG(("----------------------------------------------------------------------------------"));
        BDBG_MSG((" Set UDPStreamer HOST %s,PORT %d ", pChannel->getHost().s(), pChannel->getPort()));
        BDBG_MSG(("----------------------------------------------------------------------------------"));

        {
            BIP_UdpStreamerOutputSettings streamerOutputSettings;

            BIP_UdpStreamer_GetDefaultOutputSettings(&streamerOutputSettings);
            bipStatus = BIP_UdpStreamer_SetOutputSettings(
                    _hUdpStreamer,
                    streamerProtocol,
                    pChannel->getHost(),
                    (const char *)MString(pChannel->getPort()).s(),
                    _interfaceName,
                    &streamerOutputSettings);
            BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ("BIP_UdpStreamer_SetOutputSettings Failed"), errorInStreamerStart, bipStatus, bipStatus);
        }
    }
    else
    {
        BDBG_ERR((" CANNOT WORK WITH TUNER YET!"));
        goto errorInStreamerStart;
    }

    /*Add Audio, video and other tracks.*/
    ret = addAllTracks();
    CHECK_ERROR_GOTO("Error while executing addAllTracks", ret, errorInStreamerStart);

    /* start streamer now*/
    bipStatus = BIP_UdpStreamer_Start(_hUdpStreamer, NULL);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ("BIP_UdpStreamer_Start Failed: hUdpStreamer %p", (void *)_hUdpStreamer), errorInStreamerStart, bipStatus, bipStatus);

    BDBG_MSG(("----------------------------------------------------------------------------------"));
    BDBG_MSG((" Started UDPStreamer HOST %s,PORT %d File %s ", pChannel->getHost().s(), pChannel->getPort(), pChannel->getUrlPath().s()));
    BDBG_MSG(("----------------------------------------------------------------------------------"));

    /* Cleanup tmp Channel */
    DEL(pChannel);
    return(ret);

errorInStreamerStart:
    if (_hUdpStreamer)
    {
        /* Note: App doesn't need to destroy the streamer & can maintain a free-list of them. */
        stopAndDestroyBipStreamer();
        _hUdpStreamer = NULL;
    }

    DEL(pChannel);

    /*
     * No UDP server in BIP only in Atlas
     */
    return(ret);
} /* start */

eRet CStreamerUdp::stop()
{
    eRet ret = eRet_Ok;

    if (eUdpStreamerInputType_Tuner == _streamerInputType)
    {
        BDBG_MSG(("<--------------------------------------------------------> calling Untune"));
        if (_pChannel->isTuned())
        {
            ret = _pChannel->unTune(_pConfig);
        }
    }
    stopAndDestroyBipStreamer();

    return(ret);
}