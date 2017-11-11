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
#include "server_playlist.h"
#include "atlas.h"
#include "atlas_cfg.h"
#include "convert.h"
#include "model.h"
#include "channelmgr.h"
#include "channel.h"

#if NEXUS_HAS_VIDEO_ENCODER
#include "nexus_video_encoder.h"
#endif

#include <iostream>
#include <fstream>
#define CALLBACK_SERVER_PLAYLIST_REQUEST_RECVD  "callbackServerPlaylistRequestRecvd"
#define ATLAS_PLAYLIST_FILE_NAME                "/playlist"

BDBG_MODULE(atlas_server_http);

static void bwinProcessPlaylistRecvdRequest(
        void *       pObject,
        const char * strCallback
        )
{
    CServer * pServer = (CServer *)pObject;

    BSTD_UNUSED(strCallback);

    pServer->processRecvdRequest();
}

CPlaylistGenerator::CPlaylistGenerator(
        CConfiguration *  pCfg,
        CServerPlaylist * pPlaylistServer
        ) :
    _pCfg(pCfg),
    _pPlaylistServer(pPlaylistServer)
{
}

CPlaylistGenerator::~CPlaylistGenerator(void)
{
}

static BIP_Status rejectRequestAndSetResponseHeaders(
        CServerPlaylist *      pServerHttp,
        BIP_HttpRequestHandle  hHttpRequest,
        BIP_HttpResponseStatus responseStatus
        )
{
    eRet       ret       = eRet_Ok;
    BIP_Status bipStatus = BIP_SUCCESS;
    BIP_HttpServerRejectRequestSettings rejectSettings;

    BIP_HttpServer_GetDefaultRejectRequestSettings(&rejectSettings);
    rejectSettings.httpStatus = responseStatus;
    bipStatus                 = BIP_HttpServer_RejectRequest(pServerHttp->getBipHttpServer(), hHttpRequest, &rejectSettings);
    CHECK_BIP_ERROR_GOTO("BIP_HttpServer_RejectRequest Failed", ret, bipStatus, error);

error:
    return(ret);
}

eRet CPlaylistGenerator::open(
        BIP_HttpRequestHandle  hHttpRequest,
        BIP_HttpResponseHandle hHttpResponse,
        BIP_HttpSocketHandle   hHttpSocket
        )
{
    eRet                   ret                = eRet_Ok;
    BIP_Status             bipStatus          = BIP_SUCCESS;
    BIP_HttpResponseStatus responseStatus     = BIP_HttpResponseStatus_e500_InternalServerError;
    MString                mediaDirectoryPath = GET_STR(_pCfg, VIDEOS_PATH); /* we get it every time so that if pCfg changes and the path changes the we get the latest one.*/

    MString           completeUrl;
    CServerPlaylist * pPlayListServer = NULL;

    /* Check whether the received request has  valid parameter so that the stream can start streaming.*/
    const char *          pTmpUrlPath;
    BIP_HttpRequestMethod method;
    BIP_HttpHeaderHandle  hHeader    = NULL;
    const char *          methodname = NULL;

    bool         iOSRequest = false;
    const char * headerValue;
    MString      requestHeader;
    MString      urlPath;

    responseStatus = BIP_HttpResponseStatus_e400_BadRequest;
    bipStatus      = BIP_HttpRequest_GetMethod(hHttpRequest, &method, &methodname);
    CHECK_BIP_ERROR_GOTO("BIP_HttpRequest_GetMethod Failed", ret, bipStatus, error);

    responseStatus = BIP_HttpResponseStatus_e501_NotImplemented;
    if (method != BIP_HttpRequestMethod_eGet)
    {
        ret = eRet_InvalidParameter;
        CHECK_ERROR_GOTO("PlaylistServer: Invalid request", ret, error);
    }

    /* Retrieve the requested URL */
    responseStatus = BIP_HttpResponseStatus_e400_BadRequest;
    bipStatus      = BIP_HttpRequest_GetTarget(hHttpRequest, &pTmpUrlPath);
    CHECK_BIP_ERROR_GOTO("BIP_HttpRequest_GetTarget Failed", ret, bipStatus, error);

    bipStatus     = BIP_HttpRequest_GetNextHeader(hHttpRequest, NULL, "User-Agent", &hHeader, &headerValue);
    requestHeader = MString(headerValue);
#if NEXUS_HAS_VIDEO_ENCODER
    /* If the streaming request is coming from any iOS device*/
    if (requestHeader.findRev("Mac OS") != -1)
    {
        iOSRequest = true;
    }
#endif /* if NEXUS_HAS_VIDEO_ENCODER */

    urlPath = MString(pTmpUrlPath);
    /* validate request */
    {   /* If it's not requested by another atlas client or if the request is not coming from allowed smart devices, then it's an invalid request*/
        if ((urlPath.strncasecmp(ATLAS_PLAYLIST_FILE_NAME) != 0) && (iOSRequest != true))
        {
            ret = eRet_InvalidParameter;
            CHECK_ERROR_GOTO("PlaylistServer: Inavalid Playlist url", ret, error);
        }
    }

    pPlayListServer = getPlayListServer();
    if (pPlayListServer != NULL)
    {
#if NEXUS_HAS_VIDEO_ENCODER
        /* Generating the streaming playback list as per the client device request.
         * Can only be true when Video encoder support is available */
        if (iOSRequest == true)
        {
            _playList = generateiOSPlaylist(pPlayListServer);
        }
        else
        {
            _playList = generateAtlasPlaylist(pPlayListServer);
        }
#else /* if NEXUS_HAS_VIDEO_ENCODER */
        _playList = generateAtlasPlaylist(pPlayListServer);
#endif /* if NEXUS_HAS_VIDEO_ENCODER */
    }
    else
    {
        _playList = NULL;
    }

    responseStatus = BIP_HttpResponseStatus_e500_InternalServerError;

    if (_playList.length() == 0)
    {
        responseStatus = BIP_HttpResponseStatus_e404_NotFound;
        ret            = eRet_NotAvailable;
        CHECK_WARN_GOTO("PlaylistServer: Invalid request", ret, error);
    }

    {
        BDBG_MSG(("Host Ip:::::::::::::|%s|", pPlayListServer->getHost()));
        BDBG_MSG(("_playList:\n%s", _playList.s()));
    }

    /*
     * Now send the response and the playlist data. Since paylisty is small it can go with the response itself.
     * Send Response.
     */
    {
        BIP_HttpSocketSendResponseSettings sendResponseSettings;
        BIP_HttpSocketSendPayloadSettings  sendPayloadSettings;

        /* Prepare the Response. */
        BIP_HttpResponse_Clear(hHttpResponse, NULL);
        bipStatus = BIP_HttpResponse_SetStatus(hHttpResponse, BIP_HttpResponseStatus_e200_OK);
        CHECK_BIP_ERROR_GOTO("BIP_HttpResponse_SetStatus Failed", ret, bipStatus, error);

        BIP_HttpSocket_GetDefaultSendResponseSettings(&sendResponseSettings);
        sendResponseSettings.timeoutInMs = -1; /* blocking call, should complete immediately. */

        bipStatus = BIP_HttpSocket_SendResponse(hHttpSocket, hHttpRequest, hHttpResponse, _playList.length(), &sendResponseSettings);
        CHECK_BIP_ERROR_GOTO("BIP_HttpSocket_SendResponse Failed", ret, bipStatus, error);

        /*BIP_HttpResponse_Print( hHttpResponse, NULL, NULL);*/

        /* Send Payload. */
        BIP_HttpSocket_GetDefaultSendPayloadSettings(&sendPayloadSettings);
        sendPayloadSettings.timeoutInMs = -1; /* blocking call, should complete immediately for <250K write size. */

        bipStatus = BIP_HttpSocket_SendPayload(hHttpSocket, (uint8_t *)_playList.s(), _playList.length(), &sendPayloadSettings);
        CHECK_BIP_ERROR_GOTO("BIP_HttpSocket_SendPayload Failed", ret, bipStatus, error);
    }

    BDBG_MSG(("Done sending payload: _playList.length = %u...............................", _playList.length()));

    return(ret);

error:
    /* ResponseStatus already has appropriate value.*/
    bipStatus = rejectRequestAndSetResponseHeaders(getPlayListServer(), hHttpRequest, responseStatus);

    return(ret);
} /* open */

/* Function to generate the playlist for Atlas client */
MString CPlaylistGenerator::generateAtlasPlaylist(CServerPlaylist * pPlayListServer)
{
    eRet          ret         = eRet_Ok;
    CVideo *      pVideo      = NULL;
    CChannelBip * pCh         = NULL;
    CChannelMgr * pChannelMgr = NULL;
    CChannel *    pChannel    = NULL;

    CModel *        pModel        = NULL;
    CPlaybackList * pPlaybackList = NULL;

    unsigned program;
    MString  playList;

    pCh = new CChannelBip(_pCfg);
    CHECK_PTR_ERROR_GOTO("Error allocating HTTP IP channel", pCh, ret, eRet_OutOfMemory, error);

    pModel        = pPlayListServer->getModel();
    pPlaybackList = pModel->getPlaybackList();
    {
        uint32_t nIndex = 0;
        while (NULL != (pVideo = pPlaybackList->getVideo(nIndex++)))
        {
            MString temp = pVideo->getVideoName();
            program = pVideo->getProgram();
            pCh->setProgram(program);

            BDBG_MSG((" Name of FILE %s,", temp.s()));
            pCh->setUrl(MString("http"+ MString("://")+ pPlayListServer->getHost()+ ":" +  pPlayListServer->getHttpServerListeningPort()+ "/" + temp.s()+ "?program=" + MString(program)));

            BDBG_MSG(("URL= %s", pCh->getUrl().s()));

            playList += pCh->getUrl() + "\n";
        }
    }

    /* Now add playlist for tuner input */
    BDBG_ASSERT(NULL != pModel);
    pChannelMgr = pModel->getChannelMgr();
    BDBG_ASSERT(NULL != pChannelMgr);
    pChannel = pChannelMgr->getFirstChannel();
    /* if pChannel is NULL that means channelMgr is empty.*/
    while (pChannel != NULL)
    {
        /* BIP Channels do not require TUNER */
        if (pChannel->isTunerRequired())
        {
            MString channelName = pChannel->getChannelNum();

            /*
             * Now populate the playlist with tuner channels.
             * TODO: Check why this fails::_playList += (pPlayListServer->getProtocol())->s() + "://" + (pPlayListServer->getHost())->s() + ":" + (pPlayListServer->getPort())->s() + "/channel?" + channelName;
             */
            /* writePlaylistEntry! */
            playList += MString(pPlayListServer->getProtocol()) + "://" + MString(pPlayListServer->getHost()) + ":" + MString(pPlayListServer->getHttpServerListeningPort()) + "/channel?" + channelName;
            playList += "\n";
        }

        pChannel = pChannelMgr->getNextChannel(pChannel, false);
    }
    /* done with channel ip.*/
    DEL(pCh);
error:
    return(playList);
} /* generateAtlasPlaylist */

/* Function to generate the playlist for iOS client */
MString CPlaylistGenerator::generateiOSPlaylist(CServerPlaylist * pPlayListServer)
{
    eRet            ret           = eRet_Ok;
    CVideo *        pVideo        = NULL;
    CChannelBip *   pCh           = NULL;
    CModel *        pModel        = NULL;
    CPlaybackList * pPlaybackList = NULL;

    CChannelMgr * pChannelMgr = NULL;
    CChannel *    pChannel    = NULL;

    MString playList;

    pModel        = pPlayListServer->getModel();
    pPlaybackList = pModel->getPlaybackList();

    const char * atlasName = GET_STR(_pCfg, ATLAS_NAME);
    const char * boardName = GET_STR(_pCfg, BOARD_NAME);

    uint32_t nIndex = 0;
    unsigned program;
    bool     isHevc;
    bool     is4k;
    bool     playListItem;

    MString       temp;
    int           i;
    std::ifstream nfoFile;
    MString       streamNfoName;
    std::string   nfoStr;
    std::string   restrictStr1("videotype=\"h265\"");
    std::string   restrictStr2("width=\"3840\"");

    pCh = new CChannelBip(_pCfg);
    CHECK_PTR_ERROR_GOTO("Error allocating HTTP IP channel", pCh, ret, eRet_OutOfMemory, error);

    NEXUS_VideoDecoderCapabilities decoderCapabilities;
    NEXUS_GetVideoDecoderCapabilities(&decoderCapabilities);

#if NEXUS_HAS_VIDEO_ENCODER
    NEXUS_VideoEncoderCapabilities encoderCapabilities;
    NEXUS_GetVideoEncoderCapabilities(&encoderCapabilities);
#endif

    /* Creating the html page containing the url links for all the streams */
    playList += "<html><head><style>";
    playList += "p {background-color:black;color:white;font:85px arial;background-color:black;text-align:center;font-weight: bold;}";
    playList += "</style><title>Atlas Streaming</title></head>";
    playList += "<p> Atlas Playlist </p>";
    playList += "<style>";
    playList += "server {";
    playList += "color:black;";
    playList += "font:75px arial;font-weight: bold;";
    playList += "padding-top: 30px;";
    playList += "display: block;";
    playList += "background-color:white;margin-left: 1cm;";
    playList += "}";
    playList += "a {";
    playList += "color:blue;";
    playList += "font:75px arial;font-weight: bold;";
    playList += "padding-top: 70px;";
    playList += "display: block;";
    playList += "background-color:white;margin-left: 4cm; ";
    playList += "}";
    playList += "video {";
    playList += "visibility: hidden";
    playList += "}";
    playList += "</style>";

    playList += "<body style='background-color:#ffffff; '><video id='video' controls> </video><center></center></body>";
    playList += "<script type='text/javascript'>";

    playList += "function serverName(aName,bName) {";
    playList += "var body = document.getElementsByTagName('center')[0];";
    playList += "var server = document.createElement('server');";
    playList += "server.innerHTML = 'Server: '+ aName + bName;";
    playList += "body.appendChild(server);";
    playList += "}";

    playList += " var numChannels = 0;";
    playList += " function add(filelink,file) { var body = document.getElementsByTagName('body')[0];";
    playList += " var a = document.createElement('a');";
    playList += " numChannels++;";
    playList += " a.setAttribute('href', filelink);";
    playList += " a.innerHTML = file;";
    playList += " a.onclick = play;";
    playList += " body.appendChild(a);";
    playList += "}";

    playList += " function play(e) {";
    playList += " var video = document.getElementById('video');";
    playList += " video.innerHTML = '';";
    playList += " var source = document.createElement('source');";
    playList += " source.src = this.href;";
    playList += " source.type = 'application/x-mpegurl';";
    playList += " video.appendChild(source);";
    playList += " video.load();";
    playList += " video.play();";
    playList += " e.preventDefault();";
    playList += "}";

    playList += " var video = document.getElementById('video');";
    playList += "function endVideo() {";
    playList += "location.reload();";
    playList += "}";
    playList += "function pauseVideo() {";
    playList += "if(!video.webkitDisplayingFullscreen) {";
    playList += "endVideo();";
    playList += "}";
    playList += "}";

    playList += "video.addEventListener('ended', endVideo, false);";
    playList += "video.addEventListener('pause', pauseVideo, false);";
    playList += "serverName('";
    playList += atlasName;
    playList += "','";
    playList += boardName;
    playList += "');";

    nIndex = 0;
    while (NULL != (pVideo = pPlaybackList->getVideo(nIndex++)))
    {
        isHevc       = false;
        is4k         = false;
        playListItem = false;
        /*
         * CChannelIp  *pCh1 = new CChannelIp(_pCfg);
         * Now reset params as per the CVideo object.
         */
        temp    = pVideo->getVideoName();
        program = pVideo->getProgram();
        pCh->setProgram(program);
        BDBG_MSG((" Name of FILE %s,", temp.s()));

        streamNfoName = temp;

        if ((i = streamNfoName.findRev(".")) != -1)
        {
            streamNfoName.truncate(i);
            streamNfoName = "videos/" + streamNfoName + ".nfo";
        }

        nfoFile.open(streamNfoName.s());
        if (nfoFile)
        {
            while (!nfoFile.eof())
            {
                nfoFile >> nfoStr;
                /* Check to see if the stream is HEVC*/
                if (nfoStr.find(restrictStr1) != std::string::npos)
                {
                    isHevc = true;
                }
                /* Check to see if the stream is 4K*/
                if (nfoStr.find(restrictStr2) != std::string::npos)
                {
                    is4k = true;
                }
            }
        }
        nfoFile.close();
#if NEXUS_HAS_VIDEO_ENCODER
        /* If the decoder/boxmode does not support 4K */
        if ((decoderCapabilities.memory[0].maxFormat > 25) && (decoderCapabilities.memory[0].maxFormat < 43))
        {
            if ((is4k == false) && (true == encoderCapabilities.videoEncoder[0].supported))
            {
                /* If the decoder/boxmode does support HEVC */
                if (decoderCapabilities.memory[0].supportedCodecs[19] == true)
                {
                    pCh->setUrl(MString("http" + MString("://") + pPlayListServer->getHost() + ":" +  pPlayListServer->getHttpServerListeningPort() + "/" + temp.s() + ".m3u8" + "?program=" + MString(program)));
                    playListItem = true;
                }
                /* If the decoder/boxmode does not support HEVC */
                else
                if ((decoderCapabilities.memory[0].supportedCodecs[19] == false) && (isHevc == false))
                {
                    pCh->setUrl(MString("http" + MString("://") + pPlayListServer->getHost() + ":" +  pPlayListServer->getHttpServerListeningPort() + "/" + temp.s() + ".m3u8" + "?program=" + MString(program)));
                    playListItem = true;
                }
            }
        }
        /* If the decoder/boxmode does support 4K */
        else
        if ((decoderCapabilities.memory[0].maxFormat > 43) && (true == encoderCapabilities.videoEncoder[0].supported))
        {
            /* If the decoder/boxmode does support HEVC */
            if (decoderCapabilities.memory[0].supportedCodecs[19] == true)
            {
                pCh->setUrl(MString("http" + MString("://") + pPlayListServer->getHost() + ":" +  pPlayListServer->getHttpServerListeningPort() + "/" + temp.s() + ".m3u8" + "?program=" + MString(program)));
                playListItem = true;
            }
            /* If the decoder/boxmode does not support HEVC */
            else
            if ((decoderCapabilities.memory[0].supportedCodecs[19] == false) && (isHevc == false))
            {
                pCh->setUrl(MString("http" + MString("://") + pPlayListServer->getHost() + ":" +  pPlayListServer->getHttpServerListeningPort() + "/" + temp.s() + ".m3u8" + "?program=" + MString(program)));
                playListItem = true;
            }
        }
#endif /* if NEXUS_HAS_VIDEO_ENCODER */
        BDBG_MSG(("URL= %s", pCh->getUrl().s()));
#if NEXUS_HAS_VIDEO_ENCODER
        /* We want to restrict few streams to be listed in HLS playlist as per the server's decoder/box mode capability and the transcode capability*/
        if (playListItem == true)
        {
            playList += "add('"+ MString(pCh->getUrl().s()) +"'"+","+"'"+ temp.s() + "');";
        }
#endif /* if NEXUS_HAS_VIDEO_ENCODER */
    }

    /* Now add iOS playlist for QAM uner input */
    BDBG_ASSERT(NULL != pModel);
    pChannelMgr = pModel->getChannelMgr();
    BDBG_ASSERT(NULL != pChannelMgr);
    pChannel = pChannelMgr->getFirstChannel();
    /* if pChannel is NULL that means channelMgr is empty.*/
    while (pChannel != NULL)
    {
        MString channelName = pChannel->getChannelNum();
        pCh->setUrl(MString("http" + MString("://") + pPlayListServer->getHost() + ":" +  pPlayListServer->getHttpServerListeningPort() + "/channel?" + channelName + ".m3u8"));
#if NEXUS_HAS_VIDEO_ENCODER
        playList += "add('"+ MString(pCh->getUrl().s()) +"'"+","+"'"+ "/channel?" + channelName + "');";
#endif /* if NEXUS_HAS_VIDEO_ENCODER */
        pChannel = pChannelMgr->getNextChannel(pChannel, false);
    }
    playList += "</script>";
    playList += "</html>";
error:
    return(playList);
} /* generateiOSPlaylist */

void CPlaylistGenerator::close()
{
    BDBG_MSG(("playlist generator closed ----------------------> "));
}

void CServerPlaylist::processRecvdRequest()
{
    eRet                              ret       = eRet_Ok;
    BIP_Status                        bipStatus = BIP_SUCCESS;
    BIP_HttpRequestHandle             hHttpRequest;
    CPlaylistGenerator *              pPlaylistGenerator = NULL;
    BIP_HttpServerRecvRequestSettings recvReqSettings;
    BIP_HttpSocketStatus              httpSocketStatus;
    BIP_HttpSocketHandle              hSocket = NULL;

    /*
     * sync'd with bwin main loop
     * start processing
     */
    while (true)
    {
        hHttpRequest = NULL;

        /* Check if there is a HTTP Request available for processing! */
        BIP_HttpServer_GetDefaultRecvRequestSettings(&recvReqSettings);
        recvReqSettings.phHttpSocket = &hSocket;
        /* here I will get the hHttpSocket through recvReqSetting and wil  own it. It will have to be destroyed at the in pPlaylistGenerator->close.*/
        bipStatus = BIP_HttpServer_RecvRequest(_hHttpPlaylistServer, &hHttpRequest, &recvReqSettings);
        if (bipStatus == BIP_INF_TIMEOUT)
        {
            /* No request available at this time, return! */
            BDBG_MSG(("No HTTPRequest is currently available, we are done for now! "));
            break;
        }
        /* This is for other bip_status errors. */
        CHECK_BIP_ERROR_GOTO("BIP_HttpServer_RecvRequest Failed ", ret, bipStatus, error);

        if (hSocket == NULL)
        {
            /* coverity: why would bipStatus ever be BIP_SUCCESS if hSocket is NULL? */
            if (bipStatus == BIP_SUCCESS)
            {
                bipStatus = BIP_ERR_INTERNAL;
                BDBG_ERR(("Should not be in this state. Error in BIP, please debug"));
            }

            CHECK_BIP_ERROR_GOTO("HttpSocket is NULL ==============================================> ", ret, bipStatus, error);
        }

        /* Now set the host */
        bipStatus = BIP_HttpSocket_GetStatus(hSocket, &httpSocketStatus);
        CHECK_BIP_ERROR_GOTO("BIP_HttpSocket_GetStatus Failed ", ret, bipStatus, error);

        _host = httpSocketStatus.pLocalIpAddress;

        BDBG_MSG(("--------------------------> httpSocketStatus.pLocalIpAddress = %s", httpSocketStatus.pLocalIpAddress));

        /* Create playlist object. */
        pPlaylistGenerator = new CPlaylistGenerator(_pCfg, this);
        CHECK_PTR_ERROR_GOTO("pPlaylistGenerator: memory allocation Failed", pPlaylistGenerator, ret, eRet_OutOfMemory, error);

        ret = pPlaylistGenerator->open(hHttpRequest, _hHttpResponse, hSocket);
        CHECK_WARN_GOTO("Can't open PlaylistGenerator", ret, error);

        pPlaylistGenerator->close();

        BIP_HttpSocket_Destroy(hSocket);
        hSocket = NULL;

        DEL(pPlaylistGenerator);
        /* If streamer object is created and some error occured , then this will make sure that the streamer object is deleted.*/
        BDBG_MSG((BIP_MSG_PRE_FMT " processRecvdRequest DONE" BIP_MSG_PRE_ARG));
    }
error:
    if (hSocket)
    {
        BIP_HttpSocket_Destroy(hSocket);
    }

    DEL(pPlaylistGenerator);
    /* If streamer object is created and some error occured , then this will make sure that the streamer object is deleted.*/
    BDBG_MSG((BIP_MSG_PRE_FMT " processRecvdRequest DONE" BIP_MSG_PRE_ARG));
    return;
} /* processRecvdRequest */

static void bipProcessRecvdRequest(
        void * context,
        int    param
        )
{
    CServer *       pServer       = (CServer *)context;
    CWidgetEngine * pWidgetEngine = pServer->getWidgetEngine();

    /* sync with bwin loop */
    BDBG_ASSERT(NULL != pWidgetEngine);
    BSTD_UNUSED(param);
    BDBG_MSG(("bip request received callback"));

    pWidgetEngine->syncCallback(pServer, CALLBACK_SERVER_PLAYLIST_REQUEST_RECVD);
}

CServerPlaylist::CServerPlaylist(CConfiguration * pCfg) :
    CServer("CServerPlaylist", pCfg),
    _pModel(NULL),
    _protocol("http"),
    _host(),
    _port("89"),
    _hHttpResponse(NULL),
    _hHttpPlaylistServer(NULL)
{
}

CServerPlaylist::~CServerPlaylist()
{
}

void CServerPlaylist::stopStreamer()
{
}

void CServerPlaylist::dump(bool bForce)
{
    BDBG_Level level;

    if (true == bForce)
    {
        BDBG_GetModuleLevel("atlas_server_http", &level);
        BDBG_SetModuleLevel("atlas_server_http", BDBG_eMsg);
    }

    if (true == bForce)
    {
        BDBG_SetModuleLevel("atlas_server_http", level);
    }
}

eRet CServerPlaylist::open(CWidgetEngine * pWidgetEngine)
{
    eRet ret = eRet_Ok;

    /* call base class open first */
    ret = CServer::open(pWidgetEngine);
    CHECK_ERROR_GOTO("unable to open HTTP server", ret, error);

    _port                    = GET_STR(_pCfg, HTTP_SERVER_PLAYLIST_PORT);
    _httpServerListeningPort = GET_STR(_pCfg, HTTP_SERVER_LISTENING_PORT);

    /* Create HTTP Server */
    _hHttpPlaylistServer = BIP_HttpServer_Create(NULL); /* Use default settings. */
    CHECK_PTR_ERROR_GOTO("BIP_HttpServer_Create Failed", _hHttpPlaylistServer, ret, eRet_OutOfMemory, error);

    if (NULL != _pWidgetEngine)
    {
        _pWidgetEngine->addCallback(this, CALLBACK_SERVER_PLAYLIST_REQUEST_RECVD, bwinProcessPlaylistRecvdRequest);
    }

    _hHttpResponse = BIP_HttpResponse_Create(this, NULL);
    CHECK_PTR_ERROR_GOTO("BIP_HttpResponse_Create Failed", _hHttpResponse, ret, eRet_OutOfMemory, error);

error:
    return(ret);
} /* open */

void CServerPlaylist::close()
{
    if (NULL != _pWidgetEngine)
    {
        _pWidgetEngine->removeCallback(this, CALLBACK_SERVER_PLAYLIST_REQUEST_RECVD);
    }

    if (_hHttpPlaylistServer)
    {
        BIP_HttpServer_Destroy(_hHttpPlaylistServer);
    }

    if (_hHttpResponse)
    {
        BIP_HttpResponse_Destroy(_hHttpResponse, this);
    }

    _hHttpPlaylistServer = NULL;
    _hHttpResponse       = NULL;
    /* call base class close last */
    CServer::close();
} /* close */

eRet CServerPlaylist::start()
{
    BIP_Status bipStatus = BIP_ERR_INTERNAL;
    eRet       ret       = eRet_Ok;

    /* Set Server settings: Listening port & Callbacks */
    BIP_HttpServerSettings httpServerSettings;

    BIP_HttpServer_GetSettings(_hHttpPlaylistServer, &httpServerSettings);
    httpServerSettings.requestReceivedCallback.callback = bipProcessRecvdRequest;
    httpServerSettings.requestReceivedCallback.context  = this;
    bipStatus = BIP_HttpServer_SetSettings(_hHttpPlaylistServer, &httpServerSettings);
    CHECK_BIP_ERROR_GOTO("BIP_HttpServer_SetSettings Failed", ret, bipStatus, error);

    /* Start the Server */
    {
        BIP_HttpServerStartSettings httpServerStartSettings;

        BIP_HttpServer_GetDefaultStartSettings(&httpServerStartSettings);
        BDBG_MSG(("%s: Starting HttpServer...", BSTD_FUNCTION));

        httpServerStartSettings.maxConcurrentRequestsToQueue = 16;
        httpServerStartSettings.pPort                        = _port.s();
        httpServerStartSettings.pInterfaceName               = NULL;
        if (!_interfaceName.isEmpty())
        {
            httpServerStartSettings.pInterfaceName = _interfaceName.s();
        }

        bipStatus = BIP_HttpServer_Start(_hHttpPlaylistServer, &httpServerStartSettings);
        CHECK_BIP_ERROR_GOTO("BIP_HttpServer_Start Failed", ret, bipStatus, error);
    }

error:
    return(ret);
} /* start */

eRet CServerPlaylist::stop()
{
    eRet ret = eRet_Ok;

    BDBG_MSG((BIP_MSG_PRE_FMT " CServerHttp %p" BIP_MSG_PRE_ARG, (void *)this));

    /* Stop the Server first, so that it doesn't accept any new connections. */
    BIP_HttpServer_Stop(_hHttpPlaylistServer);

    /*error:*/
    return(ret);
}