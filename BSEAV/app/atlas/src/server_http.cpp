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
#include "server_http.h"
#include "streamer_http.h"
#include "atlas.h"
#include "atlas_cfg.h"
#include "convert.h"
#include "model.h"

#define CALLBACK_SERVER_REQUEST_RECVD        "callbackServerRequestRecvd"
#define CALLBACK_SERVER_STREAMER_STOP        "callbackServerStreamerStop"

#define ATLAS_BIP_SERVER_MAX_TRACKED_EVENTS  2

BDBG_MODULE(atlas_server_http);

static void bwinProcessRecvdRequest(
        void *       pObject,
        const char * strCallback
        )
{
    CServer * pServer = (CServer *)pObject;

    BSTD_UNUSED(strCallback);

    pServer->processRecvdRequest();
}

static void bwinStreamerStopCallback(
        void *       pObject,
        const char * strCallback
        )
{
    CServer * pServer = (CServer *)pObject;

    BSTD_UNUSED(strCallback);

    pServer->stopStreamer();
}

void CServerHttp::processRecvdRequest()
{
    eRet                  ret       = eRet_Ok;
    BIP_Status            bipStatus = BIP_SUCCESS;
    BIP_HttpRequestHandle hHttpRequest;
    CStreamerHttp *       pStreamer = NULL;

    /*
     * sync'd with bwin main loop
     * start processing
     */
    while (true)
    {
        hHttpRequest = NULL;

        /* Check if there is a HTTP Request available for processing! */
        {
            bipStatus = BIP_HttpServer_RecvRequest(_hHttpServer, &hHttpRequest, NULL);
            if (bipStatus == BIP_INF_TIMEOUT)
            {
                /* No request available at this time, return! */
                BDBG_MSG(("No HTTPRequest is currently available, we are done for now! "));
                break;
            }
            /* Make sure it is not an error while receiving the request! */
            CHECK_BIP_ERROR_GOTO("BIP_HttpServer_RecvRequest Failed ", ret, bipStatus, error);
        }

        /* Successfully Received a Request, create Streamer object */
        {
            /* create a new streamer object for each request. */

            pStreamer = new CStreamerHttp(_pCfg, this);
            CHECK_PTR_ERROR_GOTO("Unable to create CStreamerHttp object", pStreamer, ret, eRet_OutOfMemory, error);

            ret = pStreamer->open(hHttpRequest);
            CHECK_ERROR_GOTO("unable to open CStreamerHttp", ret, error);

            ret = pStreamer->start(hHttpRequest);
            CHECK_ERROR_GOTO("unable to start CStreamerHttp", ret, error);

            /* once streamer started add it to the server list.*/
            _streamerList.add(pStreamer);

            BDBG_MSG((BIP_MSG_PRE_FMT "CStreamerHttp %p: Streaming is started!" BIP_MSG_PRE_ARG, (void *)pStreamer));
        }
    }

    return;

error:
    /* If streamer object is created and some error occured , then this will make sure that the streamer object is deleted.*/
    DEL(pStreamer);

    BDBG_ERR((BIP_MSG_PRE_FMT " Error case: DONE" BIP_MSG_PRE_ARG));
    return;
} /* processRecvdRequest */

#define HTTP_STREAMER_MAX_INACTIVITY_TIME_IN_MSEC  5000
void CServerHttp::stopStreamer()
{
    CStreamerHttp *        pStreamer = NULL;
    BIP_HttpStreamerStatus status;

    BDBG_MSG((BIP_MSG_PRE_FMT "Stopping streamer called" BIP_MSG_PRE_ARG));
    for (pStreamer = _streamerList.first(); pStreamer; pStreamer = _streamerList.next())
    {
        if (BIP_HttpStreamer_GetStatus(pStreamer->getBipHttpStreamer(), &status) == BIP_SUCCESS)
        {
            if ((true == pStreamer->checkEndOfStreamer()) || (status.inactivityTimeInMs >= HTTP_STREAMER_MAX_INACTIVITY_TIME_IN_MSEC)) /*If end of the streaming is reached OR if some streamer is inactive for sometime, we want to release the streamer to serve the next request*/
            {
                /* first remove the Streamer from the list.*/
                _streamerList.remove(pStreamer);
                if (true == pStreamer->checkEndOfStreamer())
                {
                    BDBG_MSG((BIP_MSG_PRE_FMT "INSIDE:_streamerListTotal=%d, removed and stopping streamer since end Of streamer %p" BIP_MSG_PRE_ARG, _streamerList.total(), (void *)pStreamer));
                }
                else
                if (status.inactivityTimeInMs >= HTTP_STREAMER_MAX_INACTIVITY_TIME_IN_MSEC)
                {
                    BDBG_MSG((BIP_MSG_PRE_FMT "INSIDE:_streamerListTotal=%d, removed and stopping streamer since inactivity timedout %p" BIP_MSG_PRE_ARG, _streamerList.total(), (void *)pStreamer));
                }
                pStreamer->stop();
                pStreamer->close();
                DEL(pStreamer);
            }
        }
    }
} /* stopStreamer */

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

    pWidgetEngine->syncCallback(pServer, CALLBACK_SERVER_REQUEST_RECVD);
}

CServerHttp::CServerHttp(CConfiguration * pCfg) :
    CServer("CServerHttp", pCfg),
    _pModel(NULL),
    _port(80),
    _dtcpAkePort(8000),
    _dtcpKeyType("test"),
    _url("/"),
    _hHttpServer(NULL),
    _pacingEnabled(false),
    _enableDtcpIp(false),
    _serverStarted(false),
    _maxConcurrentRequestsToQueue(16),
    _persistentConnectionTimeoutInMs(5000)
{
}

CServerHttp::~CServerHttp()
{
}

eRet CServerHttp::open(
        CWidgetEngine *  pWidgetEngine,
        CConfiguration * pCfg
        )
{
    eRet    ret                = eRet_Ok;
    MString mediaDirectoryPath = GET_STR(pCfg, VIDEOS_PATH); /* we get it every time so that if pCfg changes and the path changes the we get the latest one.*/

    /* call base class open first */
    ret = CServer::open(pWidgetEngine);
    CHECK_ERROR_GOTO("unable to open HTTP server", ret, error);

    _port                            = GET_STR(pCfg, HTTP_SERVER_LISTENING_PORT);
    _interfaceName                   = GET_STR(pCfg, HTTP_SERVER_INTERFACE_NAME);
    _maxConcurrentRequestsToQueue    = GET_INT(pCfg, HTTP_SERVER_MAX_CONCURRENT_REQUEST);
    _persistentConnectionTimeoutInMs = GET_INT(pCfg, HTTP_SERVER_PERSISTENT_TIMEOUT_IN_MS);
    _enableDtcpIp                    = GET_BOOL(pCfg, HTTP_SERVER_ENABLE_DTCP);
    _dtcpAkePort                     = GET_STR(pCfg, HTTP_SERVER_DTCP_AKE_PORT);
    _dtcpKeyType                     = GET_STR(pCfg, HTTP_SERVER_DTCP_KEY_TYPE);
    _pacingEnabled                   = GET_BOOL(pCfg, HTTP_SERVER_ENABLE_HW_PACING);

    BDBG_MSG((BIP_MSG_PRE_FMT "mediaDirectoryPath ------------------------------------> %s" BIP_MSG_PRE_ARG, mediaDirectoryPath.s()));

    /* Create HTTP Server */
    _hHttpServer = BIP_HttpServer_Create(NULL); /* Use default settings. */
    CHECK_PTR_ERROR_GOTO("BIP_HttpServer_Create Failed", _hHttpServer, ret, eRet_ExternalError, error);

    if (NULL != _pWidgetEngine)
    {
        _pWidgetEngine->addCallback(this, CALLBACK_SERVER_REQUEST_RECVD, bwinProcessRecvdRequest);
        _pWidgetEngine->addCallback(this, CALLBACK_SERVER_STREAMER_STOP, bwinStreamerStopCallback);
    }

error:
    return(ret);
} /* open */

void CServerHttp::close()
{
    if (NULL != _pWidgetEngine)
    {
        _pWidgetEngine->removeCallback(this, CALLBACK_SERVER_REQUEST_RECVD);
        _pWidgetEngine->removeCallback(this, CALLBACK_SERVER_STREAMER_STOP);
    }

    if (_hHttpServer)
    {
        BIP_HttpServer_Destroy(_hHttpServer);
    }

    _hHttpServer = NULL;

    /* call base class close last */
    CServer::close();
} /* close */

eRet CServerHttp::start()
{
    BIP_Status bipStatus = BIP_ERR_INTERNAL;
    eRet       ret       = eRet_Ok;

    /* Set Server settings: Listening port & Callbacks */
    BIP_HttpServerSettings httpServerSettings;

    BIP_HttpServer_GetSettings(_hHttpServer, &httpServerSettings);
    httpServerSettings.requestReceivedCallback.callback = bipProcessRecvdRequest;
    httpServerSettings.requestReceivedCallback.context  = this;
    bipStatus = BIP_HttpServer_SetSettings(_hHttpServer, &httpServerSettings);
    CHECK_BIP_ERROR_GOTO("BIP_HttpServer_SetSettings Failed", ret, bipStatus, error);

    /* Start the Server */
    {
        BIP_HttpServerStartSettings httpServerStartSettings;

        BIP_HttpServer_GetDefaultStartSettings(&httpServerStartSettings);
        BDBG_MSG(("%s: Starting HttpServer...", __FUNCTION__));

        httpServerStartSettings.maxConcurrentRequestsToQueue = 64;
        httpServerStartSettings.pPort                        = _port.s();
        httpServerStartSettings.pInterfaceName               = NULL;
        if (_interfaceName.s())
        {
            httpServerStartSettings.pInterfaceName = _interfaceName.s();
        }
        httpServerStartSettings.persistentConnectionTimeoutInMs = _persistentConnectionTimeoutInMs;
#if B_HAS_DTCP_IP
        if (_enableDtcpIp)
        {
            httpServerStartSettings.enableDtcpIp = true;
            BIP_DtcpIpServer_GetDefaultStartSettings(&httpServerStartSettings.dtcpIpServer);
            httpServerStartSettings.dtcpIpServer.pAkePort  = _dtcpAkePort.s();
            httpServerStartSettings.dtcpIpServer.keyFormat = stringToDtcpKeyFormat(_dtcpKeyType);
        }
#endif /* if B_HAS_DTCP_IP */
        bipStatus = BIP_HttpServer_Start(_hHttpServer, &httpServerStartSettings);
        CHECK_BIP_ERROR_GOTO("BIP_HttpServer_Start Failed", ret, bipStatus, error);
        _serverStarted = true;
    }

error:
    return(ret);
} /* start */

eRet CServerHttp::stop()
{
    eRet            ret       = eRet_Ok;
    CStreamerHttp * pStreamer = NULL;

    if (!this)
    {
        return(ret);
    }
    BDBG_MSG((BIP_MSG_PRE_FMT " CServerHttp %p" BIP_MSG_PRE_ARG, (void *)this));

    /* Stop the Server first, so that it doesn't accept any new connections. */
    BIP_HttpServer_Stop(_hHttpServer);
    _serverStarted = false;

    for (pStreamer = _streamerList.first(); pStreamer; pStreamer = _streamerList.next())
    {
        /* first remove the Streamer from the list.*/
        _streamerList.remove(pStreamer);
        pStreamer->stop();
        pStreamer->close();

        DEL(pStreamer);
    }

    /*error:*/
    return(ret);
} /* stop */

void CServerHttp::dump(bool bForce)
{
    BDBG_Level level;
    int        i;

    if (true == bForce)
    {
        BDBG_GetModuleLevel("atlas_server_http", &level);
        BDBG_SetModuleLevel("atlas_server_http", BDBG_eMsg);
    }

    {
        MListItr <CStreamerHttp> itrStreamers(&_streamerList);
        CStreamerHttp *          pStreamer = NULL;
        for (pStreamer = itrStreamers.first(), i = 0; pStreamer; pStreamer = itrStreamers.next(), i++)
        {
            BDBG_MSG((BIP_MSG_PRE_FMT "_streamerListTotal=%d, streamerIndex=%d, streamer ++++++++++++++++++++>%p" BIP_MSG_PRE_ARG, _streamerList.total(), i, (void *)pStreamer));
        }
    }

    if (true == bForce)
    {
        BDBG_SetModuleLevel("atlas_server_http", level);
    }
} /* dump */