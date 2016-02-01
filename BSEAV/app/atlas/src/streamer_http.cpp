/******************************************************************************
 * (c) 2015 Broadcom Corporation
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
#include "server_http.h"
#include "streamer_http.h"
#include "atlas.h"
#include "atlas_cfg.h"
#include "convert.h"
#include "model.h"
#include "videolist.h"
#include "murl.h"
#include "channelmgr.h"
#include "channel.h"

#define CALLBACK_SERVER_STREAMER_STOP  "callbackServerStreamerStop"
#define TRANSCODE_PROFILE_720p_AVC     "720pAvc"
#define TRANSCODE_PROFILE_480p_AVC     "480pAvc"

BDBG_MODULE(atlas_server_http);

/*** define Streamer class ***/
CStreamerHttp::CStreamerHttp(
        CConfiguration * pCfg,
        CServerHttp *    pServer
        ) :
    _mediaFileAbsoloutePath(),
    _pMethodName(NULL),
    _pPlaySpeed(NULL),
    _streamerInputType(eHttpStreamerInputType_Unknown),
    _tuneChannelName(),
    _programNumberValid(false),
    _programNumber(0),
    _rangeHeaderPresent(false),
    _rangeStartOffset(0),
    _rangeLength(0),
    _dlnaTimeSeekRangePresent(false),
    _startTimeInMs(0),
    _endTimeInMs(0),
    _hHttpStreamer(NULL),
    _streamerStarted(false),
    _continuousPlay(true),
    _endOfStreamerRcvd(false),
    _pServerHttp(pServer),
    _pVideo(NULL),
    _pChannel(NULL),
    _pAtlasId(NULL),
    _enableHls(false),
    _enableXcode(false),
    _enableDtcpIp(false),
    _xcodeProfile(NULL),
    _pConfig(NULL)
{
    BDBG_ASSERT(NULL != pCfg);
    memset(&_streamerStreamInfo, 0, sizeof(_streamerStreamInfo));

    initialize((CConfiguration *)pCfg);
}

CStreamerHttp::~CStreamerHttp()
{
}

eRet CStreamerHttp::initialize(CConfiguration * pCfg)
{
    _pCfg = pCfg;
    return(eRet_Ok);
}

static BIP_Status rejectRequestAndSetResponseHeaders(
        CStreamerHttp *        pStreamerHttp,
        BIP_HttpRequestHandle  hHttpRequest,
        BIP_HttpResponseStatus responseStatus
        )
{
    eRet       ret       = eRet_Ok;
    BIP_Status bipStatus = BIP_SUCCESS;
    BIP_HttpServerRejectRequestSettings rejectSettings;
    CServerHttp * pServerHttp;

    pServerHttp = pStreamerHttp->getHttpServer();

    BIP_HttpServer_GetDefaultRejectRequestSettings(&rejectSettings);
    rejectSettings.httpStatus = responseStatus;
    bipStatus                 = BIP_HttpServer_RejectRequest(pServerHttp->getBipHttpServer(), hHttpRequest, &rejectSettings);
    CHECK_BIP_ERROR_GOTO("BIP_HttpServer_RejectRequest Failed", ret, bipStatus, error);

error:
    return(ret);
} /* rejectRequestAndSetResponseHeaders */

void CStreamerHttp::setProgram(const char * str)
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
#if 0
    BDBG_ERR((BIP_MSG_PRE_FMT "<---------------------------------------------------------------------->"BIP_MSG_PRE_ARG));
    BDBG_ERR(("_programNumberValid= %d and _programNumber = %d",
              _programNumberValid,
              _programNumber
              ));
#endif /* if 0 */
} /* setProgram */

void CStreamerHttp::setStreamerInputType(const char * url)
{
    MUrl    mUrl;
    MString tempString;
    int     index;
    MString variableName;

    _streamerInputType = eHttpStreamerInputType_File; /* we default set it as file , out side of this call will do the file specific validation */

    mUrl.set(url);

    tempString = mUrl.query();

    index = tempString.find('?', 0, true);

    if (index != -1)
    {
        variableName = tempString.left(index);
        if (!(strcasecmp(variableName, "/channel")))
        {
            _streamerInputType = eHttpStreamerInputType_Tuner;

            /* if it is tuner channel then set the channelName from the url.*/
            _tuneChannelName = tempString.mid(index + 1, (tempString.length() - index)); /* index + 1 to go past the = char.*/
        }
    }
} /* setStreamerInputType */

eRet CStreamerHttp::open(BIP_HttpRequestHandle hHttpRequest)
{
    eRet                   ret                = eRet_Ok;
    BIP_Status             bipStatus          = BIP_SUCCESS;
    BIP_HttpResponseStatus responseStatus     = BIP_HttpResponseStatus_e500_InternalServerError;
    CModel *               pModel             = NULL;
    CPlaybackList *        pPlaybackList      = NULL;
    MString                mediaDirectoryPath = GET_STR(_pCfg, VIDEOS_PATH); /* we get it every time so that if pCfg changes and the path changes the we get the latest one.*/
    MUrl                   mUrl;

    /* Check whether the received request has  valid parameter so that the stream can start streaming.*/
    {
        const char *          pTmpUrlPath;
        BIP_HttpRequestMethod method;

        responseStatus = BIP_HttpResponseStatus_e400_BadRequest;
        bipStatus      = BIP_HttpRequest_GetMethod(hHttpRequest, &method, &_pMethodName);
        CHECK_BIP_ERROR_GOTO("BIP_HttpRequest_GetMethod Failed", ret, bipStatus, error);

        responseStatus = BIP_HttpResponseStatus_e501_NotImplemented;
        if ((method != BIP_HttpRequestMethod_eHead) && (method != BIP_HttpRequestMethod_eGet))
        {
            ret = eRet_ExternalError;
            CHECK_ERROR_GOTO("unable to open CStreamerHttp", ret, error);
        }

        /* Retrieve the requested URL */
        responseStatus = BIP_HttpResponseStatus_e400_BadRequest;
        bipStatus      = BIP_HttpRequest_GetTarget(hHttpRequest, &pTmpUrlPath);
        CHECK_BIP_ERROR_GOTO("BIP_HttpRequest_GetTarget Failed", ret, bipStatus, error);

        setStreamerInputType(pTmpUrlPath);

        if (_streamerInputType == eHttpStreamerInputType_File)
        {
            int     i;
            int     j;
            MString requestedUrlPath;
            MString finalUrlPath;

            mUrl.set(pTmpUrlPath);
            /*
             * Note: app may retrieve any private/custom app specific Headers from the Request using BIP_HttpRequest_GetNextHeader().
             * Prefix the base media directory path to the URL name as this example is using the absolute file path for stream names so that it becomes unique!
             */
            _mediaFileAbsoloutePath = mediaDirectoryPath.s() + MString(mUrl.path()); /*TODO: MString(mUrl.path()) should not be required , check later.*/
            requestedUrlPath        = MString(mUrl.path());

            /* Setting the flags as per the client request suffixes and creating the requested URL path by removing the suffixes from the url */
#if B_HAS_DTCP_IP
            if ((i = requestedUrlPath.findRev(".dtcpIp")) != -1) /* If client is requesting with dtcp_ip support ON*/
            {
                _enableDtcpIp = true;
                if ((j = requestedUrlPath.findRev(".xcode")) != -1) /* Taking care of (dtcp_ip + transcode) requested with .dtcpIp.xcode OR .xcode.dtcpIp suffixes */
                {
                    _enableXcode = true;
                    requestedUrlPath.truncate((i < j) ? i : j);
                }
                else
                {
                    requestedUrlPath.truncate(i);
                }
            }
#endif /* if B_HAS_DTCP_IP */
            if ((i = requestedUrlPath.findRev(".m3u8")) != -1) /*If client is requesting with HLS support*/
            {
                _enableHls = true;
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
            pModel        = _pServerHttp->getModel();
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
        if (_streamerInputType == eHttpStreamerInputType_Tuner)
        {
            CChannelMgr * pChannelMgr = NULL;
            CChannel *    pChannel    = NULL;
            pModel = _pServerHttp->getModel();
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
    /* ResponseStatus already has appropriate value.*/
    bipStatus = rejectRequestAndSetResponseHeaders(this, hHttpRequest, responseStatus);

    return(ret);
} /* open */

void CStreamerHttp::close()
{
    DEL(_pChannel);
}

static void bipStreamerStopCallback(
        void * context,
        int    param
        )
{
    CStreamerHttp * pStreamer     = (CStreamerHttp *)context;
    CServer *       pServer       = pStreamer->getHttpServer();
    CWidgetEngine * pWidgetEngine = pServer->getWidgetEngine();

    pStreamer->setEndOfStreamer();

    BDBG_ASSERT(NULL != pWidgetEngine);
    BSTD_UNUSED(param);
    BDBG_MSG(("bip streamer stop callback"));

    pWidgetEngine->syncCallback(pServer, CALLBACK_SERVER_STREAMER_STOP);
}

void CStreamerHttp::stopAndDestroyBipStreamer()
{
    if (!_hHttpStreamer)
    {
        return;
    }

    /* it may happen that error occured while starting the streamer.*/
    if (_streamerStarted)
    {
        BDBG_MSG((BIP_MSG_PRE_FMT "Stopping Streamer %p" BIP_MSG_PRE_ARG, this));
        BIP_HttpServer_StopStreamer(_pServerHttp->getBipHttpServer(), _hHttpStreamer);
        _streamerStarted = false;
    }

    BDBG_MSG((BIP_MSG_PRE_FMT " Destroying Streamer %p" BIP_MSG_PRE_ARG, _hHttpStreamer));
    BIP_HttpServer_DestroyStreamer(_pServerHttp->getBipHttpServer(), _hHttpStreamer);
    return;
} /* stopAndDestroyBipStreamer */

/* THis atlas function will generate all the information for corresponding program number of a stream.
 * The CVideo object for the for a program number of a stream is already determined after receiving the request. */
void CStreamerHttp::setStreamerStreamInfo()
{
    CPidMgr * pPidMagr = NULL;

    if (eHttpStreamerInputType_File == _streamerInputType)
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
    if (eHttpStreamerInputType_Tuner == _streamerInputType)
    {
        BDBG_ASSERT(NULL != _pChannel);

        pPidMagr = _pChannel->getPidMgr();
        BDBG_ASSERT(NULL != pPidMagr);

        B_Os_Memset(&_streamerStreamInfo, 0, sizeof(_streamerStreamInfo));

        _streamerStreamInfo.transportType       = pPidMagr->getTransportType();
        _streamerStreamInfo.numberOfTrackGroups = 1;
    }
} /* setStreamerStreamInfo */

eRet CStreamerHttp::addAllTracks()
{
    eRet                  ret       = eRet_Ok;
    BIP_Status            bipStatus = BIP_SUCCESS;
    CPidMgr *             pPidMgr   = NULL;
    CPid *                pPid      = NULL;
    BIP_StreamerTrackInfo streamerTrackInfo;
    int                   trackIndex = 0;
    uint16_t              pmtPid     = 0;

    /* Set mediaInfo parameters.*/
    if (eHttpStreamerInputType_File == _streamerInputType)
    {
        BDBG_ASSERT(NULL != _pVideo);
        pPidMgr = _pVideo->getPidMgr();
    }
    else
    if (eHttpStreamerInputType_Tuner == _streamerInputType)
    {
        BDBG_ASSERT(NULL != _pChannel);
        pPidMgr = _pChannel->getPidMgr();
    }
    BDBG_ASSERT(NULL != pPidMgr);
    BDBG_ASSERT(NULL != _hHttpStreamer);

    pmtPid = pPidMgr->getPmtPid();

    /* Add PAT & PMT */
    if (pmtPid != 0)
    /* TODO: Since PAT may contain multiple programs, we will need to substitute it w/ a PAT containing just this one PMT */
    {
        B_Os_Memset(&streamerTrackInfo, 0, sizeof(streamerTrackInfo));
        streamerTrackInfo.trackId = 0; /* PAT is always at PID == 0 */
        streamerTrackInfo.type    = BIP_MediaInfoTrackType_eOther;
        bipStatus                 = BIP_HttpStreamer_AddTrack(_hHttpStreamer, &streamerTrackInfo, NULL);
        CHECK_BIP_ERROR_GOTO("BIP_HttpStreamer_AddTrack Failed for PAT", ret, bipStatus, error);

        /* Add PMT */
        B_Os_Memset(&streamerTrackInfo, 0, sizeof(streamerTrackInfo));
        streamerTrackInfo.trackId = pmtPid;
        streamerTrackInfo.type    = BIP_MediaInfoTrackType_ePmt;
        bipStatus                 = BIP_HttpStreamer_AddTrack(_hHttpStreamer, &streamerTrackInfo, NULL);
        CHECK_BIP_ERROR_GOTO("BIP_HttpStreamer_AddTrack Failed for PMT", ret, bipStatus, error);
    }

    /* Add pcr track*/
    B_Os_Memset(&streamerTrackInfo, 0, sizeof(streamerTrackInfo));
    pPid = pPidMgr->getPid(0, ePidType_Pcr);
    /*Pcr pid won't be present for MP4 formats.*/
    if (pPid)
    {
        streamerTrackInfo.trackId = pPid->getPid();
        streamerTrackInfo.type    = BIP_MediaInfoTrackType_ePcr;
        bipStatus                 = BIP_HttpStreamer_AddTrack(_hHttpStreamer, &streamerTrackInfo, NULL);
        CHECK_BIP_ERROR_GOTO("BIP_HttpStreamer_AddTrack Failed for BIP_MediaInfoTrackType_ePcr", ret, bipStatus, error);
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

        bipStatus = BIP_HttpStreamer_AddTrack(_hHttpStreamer, &streamerTrackInfo, NULL);
        CHECK_BIP_ERROR_GOTO("BIP_HttpStreamer_AddTrack Failed for BIP_MediaInfoTrackType_eVideo", ret, bipStatus, error);
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

        bipStatus = BIP_HttpStreamer_AddTrack(_hHttpStreamer, &streamerTrackInfo, NULL);
        CHECK_BIP_ERROR_GOTO("BIP_HttpStreamer_AddTrack Failed for BIP_MediaInfoTrackType_eAudio", ret, bipStatus, error);
    }

    /* Add all anciliary tracks.*/
    trackIndex = 0;
    pPid       = NULL;
    for (trackIndex = 0; (pPid = pPidMgr->getPid(trackIndex, ePidType_Ancillary)) != NULL; trackIndex++)
    {
        B_Os_Memset(&streamerTrackInfo, 0, sizeof(streamerTrackInfo));
        streamerTrackInfo.trackId = pPid->getPid();
        streamerTrackInfo.type    = BIP_MediaInfoTrackType_eOther;

        bipStatus = BIP_HttpStreamer_AddTrack(_hHttpStreamer, &streamerTrackInfo, NULL);
        CHECK_BIP_ERROR_GOTO("BIP_HttpStreamer_AddTrack Failed for BIP_MediaInfoTrackType_eAudio", ret, bipStatus, error);
    }
error:
    return(ret);
} /* addAllTracks */

eRet CStreamerHttp::start(BIP_HttpRequestHandle hHttpRequest)
{
    eRet                   ret            = eRet_Ok;
    BIP_Status             bipStatus      = BIP_SUCCESS;
    BIP_HttpResponseStatus responseStatus = BIP_HttpResponseStatus_e500_InternalServerError;
    int i;

    /* Create HttpStreamer. */
    {
        BIP_HttpServerCreateStreamerSettings createStreamerSettings;

        BIP_HttpServer_GetDefaultCreateStreamerSettings(&createStreamerSettings);
        createStreamerSettings.endOfStreamingCallback.callback = bipStreamerStopCallback;
        createStreamerSettings.endOfStreamingCallback.context  = this;

        responseStatus = BIP_HttpResponseStatus_e500_InternalServerError;
        _hHttpStreamer = BIP_HttpServer_CreateStreamer(_pServerHttp->getBipHttpServer(), &createStreamerSettings);
        CHECK_PTR_ERROR_GOTO("BIP_HttpServer_CreateStreamer Failed", _hHttpStreamer, ret, eRet_ExternalError, rejectRequest);

        BDBG_MSG((BIP_MSG_PRE_FMT " BIP_HttpStreamer created: %p" BIP_MSG_PRE_ARG, _hHttpStreamer));
    }

    /* Populate _mediaInfoStream. */
    setStreamerStreamInfo(); /* this method internally checks whether this is for tuner input or qam input.*/

    /* Now provide File input settings */
    if (eHttpStreamerInputType_File == _streamerInputType)
    {
        BIP_StreamerFileInputSettings fileInputSettings;

        BIP_Streamer_GetDefaultFileInputSettings(&fileInputSettings);

        BDBG_MSG((BIP_MSG_PRE_FMT "========> absoloute stream path streamName :|%s|" BIP_MSG_PRE_ARG, _mediaFileAbsoloutePath.s()));

        /* Parse the PlaySpeed Header and if present, configure its values in the input settings. */
        {
            const char *         pHeaderValue;
            BIP_HttpHeaderHandle hHeader;
            /* PlaySpeed.dlna.org: speed=6 */
            bipStatus = BIP_HttpRequest_GetNextHeader(hHttpRequest, NULL, "PlaySpeed.dlna.org", &hHeader, &pHeaderValue);
            if (bipStatus == BIP_SUCCESS)
            {
                /* NOTE: temp code until BIP_HttpRequest_GetPlaySpeedHeader() provides this string! */
                if (strlen(pHeaderValue) > strlen("speed="))
                {
                    _pPlaySpeed                 = pHeaderValue + strlen("speed=");
                    fileInputSettings.playSpeed = _pPlaySpeed;
                }
            }
        }

        /* Parse the Byte Range Headers. */
        {
            BIP_HttpRequestParsedRangeHeaderForBytes * pRangeHeaderParsed = NULL;

            /*
             * Check if either TimeSeekRange and/or Byte-Range Headers are present.
             * If both type of Range headers are present, then app can choose to prefer one over another.
             * DLNA Specs recommend Http ByteRange over TimeSeekRange: indicated in 7.5.4.3.3.17 of Dlna spec.
             */
            responseStatus = BIP_HttpResponseStatus_e400_BadRequest;
            bipStatus      = BIP_HttpRequest_ParseRangeHeaderForBytes(hHttpRequest, &pRangeHeaderParsed);
            if ((bipStatus != BIP_SUCCESS) && (bipStatus != BIP_INF_NOT_AVAILABLE))
            {
                ret = eRet_ExternalError;
                goto rejectRequest;
            }

            if (bipStatus == BIP_SUCCESS)
            {
                /* Note: this example is only handling the 1st Range Entry, but app can use all Entries! */
                bipStatus = BIP_HttpRequest_GetRangeEntryOffset(
                        &(pRangeHeaderParsed->pByteRangeSet[0]),
                        _streamerStreamInfo.contentLength,
                        &_rangeStartOffset,
                        &_rangeLength
                        );
                if ((bipStatus != BIP_SUCCESS) && (bipStatus != BIP_INF_NOT_AVAILABLE))
                {
                    ret = eRet_ExternalError;
                    goto rejectRequest;
                }
                BDBG_MSG((BIP_MSG_PRE_FMT "numRangeEntries %d[#0]: Range Start %llu, Length %llu " BIP_MSG_PRE_ARG,
                          pRangeHeaderParsed->byteRangeCount, _rangeStartOffset, _rangeLength));

                /* Range parsing is complete, so fill-in the values in the input settings. */
                fileInputSettings.beginByteOffset = _rangeStartOffset;
                fileInputSettings.endByteOffset   = _rangeStartOffset + _rangeLength - 1;

                _rangeHeaderPresent = true;
            }
        }

        /* If we've already parsed a Range header, we'll use it, but if not, then look for a TimeSeekRange header. */
        if (!_rangeHeaderPresent)
        {
            bipStatus = BIP_HttpRequest_ParseTimeSeekRangeDlnaOrgHeader(hHttpRequest, _streamerStreamInfo.durationInMs, &_startTimeInMs, &_endTimeInMs );
            responseStatus = BIP_HttpResponseStatus_e400_BadRequest;
            BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS || bipStatus == BIP_INF_NOT_AVAILABLE), ( "BIP_HttpRequest_ParseTimeSeekRangeDlnaOrgHeader Failed" ), rejectRequest, bipStatus, bipStatus );

            if (bipStatus == BIP_SUCCESS)
            {
                BDBG_MSG(( BIP_MSG_PRE_FMT "TimeSeekRange.dlna.org header: _startTimeInMs=%llu, _endTimeInMs=%llu"
                           BIP_MSG_PRE_ARG, _startTimeInMs, _endTimeInMs ));

                fileInputSettings.beginTimeOffsetInMs = _startTimeInMs;
                fileInputSettings.endTimeOffsetInMs   = (_endTimeInMs > 0) ? _endTimeInMs : 0;

                BDBG_MSG(( BIP_MSG_PRE_FMT "fileInputSettings: beginTimeOffsetInMs=%lu, endTimeOffsetInMs=%lu"
                           BIP_MSG_PRE_ARG, fileInputSettings.beginTimeOffsetInMs, fileInputSettings.endTimeOffsetInMs));

                _dlnaTimeSeekRangePresent = true;
            }
        }

        if (_continuousPlay)
        {
            fileInputSettings.enableContinousPlay = true;
        }

        if (_streamerStreamInfo.transportType == NEXUS_TransportType_eTs)
        {
            /* App wants to pace and this is a MPEG2 TS stream, so BIP can pace it using the Playback Channel. Set that option. */
            fileInputSettings.enableHwPacing = GET_BOOL(_pCfg, HTTP_SERVER_ENABLE_HW_PACING);
        }

        /* Set File input Settings */
        responseStatus = BIP_HttpResponseStatus_e500_InternalServerError;

        bipStatus = BIP_HttpStreamer_SetFileInputSettings(
                _hHttpStreamer,
                _mediaFileAbsoloutePath.s(),
                &_streamerStreamInfo, /* MediaStreamInfo */
                &fileInputSettings
                );
        CHECK_BIP_ERROR_GOTO("BIP_HttpStreamer_SetFileInputSettings Failed", ret, bipStatus, rejectRequest);

        BDBG_MSG((BIP_MSG_PRE_FMT "Method: %s, Absolute URL: %s, PlaySpeed: %s, Byte Range: %s, DlnaTimeSeek: %s, TimeOffsetInMsec: start %u, end %u" BIP_MSG_PRE_ARG,
                  _pMethodName, _mediaFileAbsoloutePath.s(),
                  _pPlaySpeed ? "Y" : "N",
                  _rangeHeaderPresent ? "Y" : "N",
                  _dlnaTimeSeekRangePresent ? "Y" : "N",
                  fileInputSettings.beginTimeOffsetInMs, fileInputSettings.endTimeOffsetInMs
                  ));
    } /* Input Settings. */
    else
    if (eHttpStreamerInputType_Tuner == _streamerInputType)
    {
        BIP_StreamerTunerInputSettings tunerInputSettings;
        CParserBand *                  pParserBand = NULL;

        /* first call the tune */
        BDBG_ASSERT(NULL != _pChannel);

        BDBG_ASSERT(NULL != _pConfig);

        responseStatus = BIP_HttpResponseStatus_e500_InternalServerError;

        BDBG_MSG(("<--------------------------------------------------------> calling tune"));
        ret = _pChannel->tune(_pAtlasId, _pConfig, false);
        CHECK_ERROR_GOTO("Error:Not able to get a parSerBand", ret, rejectRequest);

        pParserBand = _pChannel->getParserBand();

        if (NULL == pParserBand)
        {
            responseStatus = BIP_HttpResponseStatus_e503_ServiceUnavailable;
            ret            = eRet_InvalidParameter; /* If parserband not present we return InvalidParameter and send BIP_HttpResponseStatus_e500_InternalServerError response.*/
            CHECK_ERROR_GOTO("Error:Not able to get a parSerBand", ret, rejectRequest);
        }

        responseStatus = BIP_HttpResponseStatus_e500_InternalServerError;

        /* Now provide settings for media input source */
        BIP_Streamer_GetDefaultTunerInputSettings(&tunerInputSettings);
        bipStatus = BIP_HttpStreamer_SetTunerInputSettings(_hHttpStreamer, pParserBand->getBand(), &_streamerStreamInfo, &tunerInputSettings);
        CHECK_BIP_ERROR_GOTO("BIP_HttpStreamer_SetTunerInputSettings Failed", ret, bipStatus, rejectRequest);
    }

    /** this point onwards only possible response error BIP_HttpResponseStatus_e500_InternalServerError */
    responseStatus = BIP_HttpResponseStatus_e500_InternalServerError;

    /*Add Audio, video and other tracks.*/
    ret = addAllTracks();
    CHECK_ERROR_GOTO("Error while executing addAllTracks", ret, rejectRequest);

    /* Provide the Streamer output settings */
    {
        BIP_HttpStreamerOutputSettings streamerOutputSettings;
        BIP_HttpStreamerProtocol       streamerProtocol;

        CServerHttp * pServerHttp;

        pServerHttp = getHttpServer();

        BIP_HttpStreamer_GetDefaultOutputSettings(&streamerOutputSettings);
#if B_HAS_DTCP_IP
        if (_enableDtcpIp)
        {
            streamerOutputSettings.enableDtcpIp                         = true;
            streamerOutputSettings.dtcpIpOutput.pcpPayloadLengthInBytes = (1024*1024);
            streamerOutputSettings.dtcpIpOutput.akeTimeoutInMs          = 1000;
            streamerOutputSettings.dtcpIpOutput.copyControlInfo         = B_CCI_eCopyNever;
        }
#endif /* if B_HAS_DTCP_IP */
        streamerProtocol = _enableHls ? BIP_HttpStreamerProtocol_eHls : BIP_HttpStreamerProtocol_eDirect;
        bipStatus        = BIP_HttpStreamer_SetOutputSettings(_hHttpStreamer, streamerProtocol, &streamerOutputSettings);

        CHECK_BIP_ERROR_GOTO("BIP_HttpStreamer_SetOutputSettings Failed", ret, bipStatus, rejectRequest);
    }

    /* Add Encoder Profile. */
    {
#if NEXUS_HAS_VIDEO_ENCODER
        BIP_TranscodeProfile transcodeProfile;
        if (!_xcodeProfile) { _xcodeProfile = TRANSCODE_PROFILE_720p_AVC; }
        if (_enableXcode)
        {
            if (MString(_xcodeProfile) == TRANSCODE_PROFILE_720p_AVC)
            {
                BIP_Transcode_GetDefaultProfileFor_720p30_AVC_AAC_MPEG2_TS(&transcodeProfile);
            }
            else
            if (MString(_xcodeProfile) == TRANSCODE_PROFILE_480p_AVC)
            {
                BIP_Transcode_GetDefaultProfileFor_480p30_AVC_AAC_MPEG2_TS(&transcodeProfile);
            }
            else
            {
                BIP_Transcode_GetDefaultProfile(&transcodeProfile);
            }

            bipStatus      = BIP_HttpStreamer_AddTranscodeProfile(_hHttpStreamer, &transcodeProfile);
            responseStatus = BIP_HttpResponseStatus_e500_InternalServerError;
            BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ("BIP_HttpStreamer_AddTranscodeProfile Failed"), rejectRequest, bipStatus, bipStatus);
        }
        else
        if (_enableHls)
        {
            if (0)
            /*
             * Add a 480p profile.
             * NOTE: this is being commented out as some 480p profile encode profile default settings are not yet set correctly and thus players are not playing the initial ~10sec worth of frames.
             */
            {
                BIP_Transcode_GetDefaultProfileFor_480p30_AVC_AAC_MPEG2_TS(&transcodeProfile);
                bipStatus      = BIP_HttpStreamer_AddTranscodeProfile(_hHttpStreamer, &transcodeProfile);
                responseStatus = BIP_HttpResponseStatus_e500_InternalServerError;
                BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ("BIP_HttpStreamer_AddTranscodeProfile Failed"), rejectRequest, bipStatus, bipStatus);
            }
            /* Add a 720p profile. */
            {
                BIP_Transcode_GetDefaultProfileFor_720p30_AVC_AAC_MPEG2_TS(&transcodeProfile);
                bipStatus      = BIP_HttpStreamer_AddTranscodeProfile(_hHttpStreamer, &transcodeProfile);
                responseStatus = BIP_HttpResponseStatus_e500_InternalServerError;
                BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ("BIP_HttpStreamer_AddTranscodeProfile Failed"), rejectRequest, bipStatus, bipStatus);
            }
        }
#endif /* if NEXUS_HAS_VIDEO_ENCODER */
    }
    /* At this point, we have set sucessfully configured the Input & Output settings of the HttpStreamer. */

    /* Now Set any custom or app specific HTTP Headers that HttpStreamer should include when it sends out the Response Message. */
    {
        bipStatus = BIP_HttpStreamer_SetResponseHeader(
                _hHttpStreamer,
                "transferMode.dlna.org",
                "Streaming"
                );
        CHECK_BIP_ERROR_GOTO("BIP_HttpStreamer_SetResponseHeader Failed", ret, bipStatus, rejectRequest);
        /* App can add more custom headers here using this example above! */
    }
    /* And as a last step, start the HttpStreamer. */
    /* Limiting the number of trials to start the streamer to avoid the infinite loop in case of the lack of resources */
    for(i = 0; i < 20; i++)
    {
        BIP_HttpServerStartStreamerSettings startSettings;
        BIP_HttpServer_GetDefaultStartStreamerSettings(&startSettings);
        startSettings.streamerStartSettings.inactivityTimeoutInMs = 50000;
        bipStatus = BIP_HttpServer_StartStreamer(_pServerHttp->getBipHttpServer(), _hHttpStreamer, hHttpRequest, &startSettings);

        /*If no streaming/transcode resources are available*/
        if (bipStatus == BIP_INF_NEXUS_RESOURCE_NOT_AVAILABLE)
        {
            /* Since Streamer start failed because a Nexus resource wasn't available. We will try to see if there are any inactive streamers and stop/destroy them. */
            _pServerHttp->stopStreamer();
            BDBG_WRN((BIP_MSG_PRE_FMT "Successfully Released an inactive Streamer, Retrying BIP_HttpServer_StartStreamer()" BIP_MSG_PRE_ARG));
            continue;
        }
        else
        {
            CHECK_BIP_ERROR_GOTO("BIP_HttpServer_StartStreamer Failed", ret, bipStatus, rejectRequest);
            ret              = eRet_Ok;
            _streamerStarted = true;
            break;
        }
    }

    return(ret);

rejectRequest:
    /*TODO: later map it the bip way, set responseStatus appropriately based on BIP_Status.*/
    bipStatus = rejectRequestAndSetResponseHeaders(this, hHttpRequest, responseStatus);
    CHECK_BIP_ERROR_GOTO("rejectRequestAndSetResponseHeaders Failed", ret, bipStatus, errorInStreamerStart);

errorInStreamerStart:
    if (_hHttpStreamer)
    {
        /* Note: App doesn't need to destroy the streamer & can maintain a free-list of them. */
        stopAndDestroyBipStreamer();
        _hHttpStreamer = NULL;
    }

    /*
     * This label handles the error cases when app encounters an error during BIP_HttpServer_StartStreamer().
     * In that case, BIP_HttpServer_StartStreamer() has internally send out the error response.
     * Caller will cleanup the AppStreamerCtx.
     */
    return(ret);
} /* start */

eRet CStreamerHttp::stop()
{
    eRet ret = eRet_Ok;

    if (eHttpStreamerInputType_Tuner == _streamerInputType)
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
