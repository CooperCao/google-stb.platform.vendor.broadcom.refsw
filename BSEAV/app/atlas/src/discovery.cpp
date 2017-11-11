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
#include "atlas.h"
#include "atlas_os.h"
#include "atlas_cfg.h"
#include "convert.h"
#include "discovery.h"
#include "channel_bip.h"
#include "playlist_db.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <string.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>

#define CALLBACK_AUTO_DISCOVERY_CONNECT       "callbackAutoDiscoveryConnect"
#define CALLBACK_AUTO_DISCOVERY_SERVER_FOUND  "callbackAutoDiscoveryServerFound"

#define BEACON_MCAST_ADDRESS                  "239.99.99.99"
#define BEACON_MCAST_PORT                     9999
#define BEACON_MCAST_INTERVAL                 5000 /* in msec seconds */

BDBG_MODULE(atlas_auto_discovery);

static void bwinConnectCallback(
        void *       pObject,
        const char * strCallback
        )
{
    CAutoDiscoveryServer * pServer = (CAutoDiscoveryServer *)pObject;

    BSTD_UNUSED(strCallback);

    pServer->connectCallback();
}

static void bipConnectCallback(
        void *       pParam,
        const void * pBuffer,
        uint32_t     nData
        )
{
    CAutoDiscoveryServer * pServer       = (CAutoDiscoveryServer *)pParam;
    CWidgetEngine *        pWidgetEngine = pServer->getWidgetEngine();

    BDBG_ASSERT(NULL != pWidgetEngine);
    BSTD_UNUSED(pBuffer);
    BSTD_UNUSED(nData);
    BDBG_MSG(("bip connect start callback"));

    pWidgetEngine->syncCallback(pServer, CALLBACK_AUTO_DISCOVERY_CONNECT);
}

void CAutoDiscoveryServer::connectCallback()
{
    BDBG_MSG(("connect request received"));
    /* sync'd with bwin main loop */
}

/*************************** CAutoDiscoveryServer ***************************/

/* coverity[secure_coding] */
CAutoDiscoveryServer::CAutoDiscoveryServer(
        const char *     name,
        CConfiguration * pCfg
        ) :
    CMvcModel(name),
    _pWidgetEngine(NULL),
    _pCfg(pCfg),
    _beaconIntervalTimer(this),
    _server_name(GET_STR(pCfg, ATLAS_NAME)),
    _beaconMcastAddress(GET_STR(pCfg, AUTO_DISCOVERY_BEACON_MCAST_ADDRESS)),
    _beaconMcastPort((unsigned short)GET_INT(pCfg, AUTO_DISCOVERY_BEACON_MCAST_PORT)),
    _beaconIntervalMsec(GET_INT(pCfg, AUTO_DISCOVERY_BEACON_INTERVAL)),
    _beacon_version(1),
    _beacon_fd(0),
    _bEnabled(GET_BOOL(pCfg, ATLAS_SERVER_ENABLED) && GET_BOOL(pCfg, AUTO_DISCOVERY_SERVER_ENABLED)),
    _pIfInterfaceList(NULL)
{
    BKNI_Memset(_beacon, 0, sizeof(_beacon));
    BKNI_Memset(&_beacon_addr, 0, sizeof(_beacon_addr));
    if (_bEnabled)
    {
        _pIfInterfaceList = get_ifaddrs();
        /* every time same server starts enforce a fresh new beacon */
        srand(time(NULL));
        /* coverity[secure_coding] */
        _beacon_version = random();
    }
}

CAutoDiscoveryServer::~CAutoDiscoveryServer()
{
    free_ifaddrs(_pIfInterfaceList);
    _pIfInterfaceList = NULL;
}

eRet CAutoDiscoveryServer::updateIfAddrs()
{
    free_ifaddrs(_pIfInterfaceList);
    _pIfInterfaceList = NULL;
    _pIfInterfaceList = get_ifaddrs();
    return(eRet_Ok);
}

eRet CAutoDiscoveryServer::open(CWidgetEngine * pWidgetEngine)
{
    eRet ret = eRet_Ok;

    BSTD_UNUSED(bipConnectCallback);

    if (!_bEnabled)
    {
        return(ret);
    }

    _pWidgetEngine = pWidgetEngine;
    if (NULL != _pWidgetEngine)
    {
        _pWidgetEngine->addCallback(this, CALLBACK_AUTO_DISCOVERY_CONNECT, bwinConnectCallback);
    }

    _beaconIntervalTimer.setWidgetEngine(_pWidgetEngine);

    _beacon_addr.sin_family      = AF_INET;
    _beacon_addr.sin_addr.s_addr = inet_addr(_beaconMcastAddress.s());
    _beacon_addr.sin_port        = htons(_beaconMcastPort);

    _beacon_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (_beacon_fd < 0)
    {
        BDBG_ERR(("Failed to open beacon socket"));
        ret = eRet_ExternalError;
    }
    /* start beaconing */
    _beaconIntervalTimer.start(100);
    return(ret);
} /* open */

void CAutoDiscoveryServer::close()
{
    if (!_bEnabled)
    {
        return;
    }

    if (_beaconIntervalTimer.isStarted())
    {
        _beaconIntervalTimer.stop();
    }
    _beaconIntervalTimer.setWidgetEngine(NULL);
    if (_beacon_fd > 0)
    {
        ::close(_beacon_fd);
    }
    if (NULL != _pWidgetEngine)
    {
        _pWidgetEngine->removeCallback(this, CALLBACK_AUTO_DISCOVERY_CONNECT);
        _pWidgetEngine = NULL;
    }
} /* close */

void CAutoDiscoveryServer::timerCallback(void * pTimer)
{
    BSTD_UNUSED(pTimer);
    int        beacon_size;
    static int ifRefreshInterval = 0;
    const int  minute            = 15000;
    ifRefreshInterval += _beaconIntervalMsec;
    if (ifRefreshInterval > minute)
    {
        ifRefreshInterval = 0;
        updateIfAddrs();
    }
    /* _beacon_version++; */

    if_interface * pIfInterface = NULL;
    for (pIfInterface = _pIfInterfaceList->first(); pIfInterface; pIfInterface = _pIfInterfaceList->next())
    {
        if (setsockopt(_beacon_fd, SOL_SOCKET, SO_BINDTODEVICE, pIfInterface->if_name.s(), pIfInterface->if_name.length()) != 0)
        {
            BDBG_ERR(("Failed to bind interface %s", pIfInterface->if_name.s()));
        }
#if 0
        /*beacon_size= sprintf(_beacon,"%s("INET_ADDR_PRINTF_FMT") %u ",_server_name.s(),INET_ADDR_PRINTF_ARG(pIfInterface->s_addr),_beacon_version);*/
        if (_server_name.strncasecmp("Atlas") == 0)
        {
            beacon_size = sprintf(_beacon, "%s("INET_ADDR_PRINTF_FMT ") %u ", GET_STR(_pCfg, BOARD_NAME), INET_ADDR_PRINTF_ARG(pIfInterface->s_addr), _beacon_version);
        }
        else
        {
            beacon_size = sprintf(_beacon, "%s(%s) %u ", _server_name.s(), GET_STR(_pCfg, BOARD_NAME), _beacon_version);
        }
#endif /* if 0 max size of _beacon is 50 */
        beacon_size = snprintf(_beacon, 49, "%s(%s) %u ", _server_name.s(), GET_STR(_pCfg, BOARD_NAME), _beacon_version);
        /*BDBG_MSG(("beacon %s, size %d",_beacon,beacon_size)); */

        /* coverity[check_return] */
        BDBG_MSG(("sending beacon:%s from ip:" INET_ADDR_PRINTF_FMT, _beacon, INET_ADDR_PRINTF_ARG(pIfInterface->s_addr)));
        sendto(_beacon_fd, _beacon, beacon_size, 0, (struct sockaddr *) &_beacon_addr, sizeof(_beacon_addr));
    }
    _beaconIntervalTimer.start(_beaconIntervalMsec);
} /* timerCallback */

void CAutoDiscoveryServer::dump(bool bForce)
{
    BDBG_Level level;

    if (true == bForce)
    {
        BDBG_GetModuleLevel("atlas_auto_discovery", &level);
        BDBG_SetModuleLevel("atlas_auto_discovery", BDBG_eMsg);
    }

    /* print out relevent data here */

    if (true == bForce)
    {
        BDBG_SetModuleLevel("atlas_auto_discovery", level);
    }
} /* dump */

/*************************** CDiscoveredServer ***************************/
#define CALLBACK_HTTP_CLIENT_FINISHED  "callbackHttpClientFinished"

static void bwinHttpClientFinishedCallback(
        void *       pObject,
        const char * strCallback
        )
{
    CDiscoveredServer * pServer = (CDiscoveredServer *)pObject;

    BSTD_UNUSED(strCallback);

    pServer->httpClientFinishedCallback();
}

static void triggerHttpClientFinishedCallback(
        void *       pParam,
        const void * pBuffer,
        uint32_t     nData
        )
{
    CDiscoveredServer * pServer       = (CDiscoveredServer *)pParam;
    CWidgetEngine *     pWidgetEngine = pServer->getWidgetEngine();

    BDBG_ASSERT(NULL != pWidgetEngine);
    BSTD_UNUSED(pBuffer);
    BSTD_UNUSED(nData);

    pWidgetEngine->syncCallback(pServer, CALLBACK_HTTP_CLIENT_FINISHED);
}

void CDiscoveredServer::httpClientFinishedCallback()
{
    CChannelBip * pIpChannel = NULL;
    char          url[512];
    int           index = 0;
    int           err   = 0;

    /* clean up thread */
    if (_hHttpClientThread)
    {
        B_Thread_Destroy(_hHttpClientThread);
        _hHttpClientThread = NULL;
    }

    /* we have received the playlist or failed */
    if (_playlistBuffer.isEmpty())
    {
        BDBG_WRN(("Empty playlist received from http://%s:%s", _strIpAddress.s(), _strPort.s()));
        close();
        return;
    }

    {
        /* protect access to _playlistBuffer with mutex */
        CScopedMutex scopedMutex(_playlistBufferMutex);

        BDBG_MSG(("playlist:%s", _playlistBuffer.s()));

        while (index < _playlistBuffer.length())
        {
            /* coverity[secure_coding]*/
            err = sscanf(_playlistBuffer.s()+ index, "%511s", url);
            BDBG_MSG((" Code Returned by Sscanf is %d", err));
            pIpChannel = new CChannelBip(getWidgetEngine()->getCfg());
            if (!pIpChannel)
            {
                BDBG_ERR(("Failed to create ChannelIp"));
                break;
            }
            pIpChannel->setUrl(url);
            /* discovered channels are not in the channel list so they can be stopped.
             * bip channels in the channel list cannot be stopped - user must tune away instead. */
            pIpChannel->setStopAllowed(true);
            BDBG_MSG(("adding channel:%s", url));
            addChannel(pIpChannel);

            index = _playlistBuffer.find('\n', index);
            if (index == -1)
            {
                break;
            }
            index++;
        }
    }

    setName(_serverName.s());
    addMetadata("Server", _serverName.s());
    addMetadata("IP Address", _strIpAddress.s());
    addMetadata("Port", _strPort.s());
    if (getModel()->getPlaylistDb()->addPlaylist((void *)this, this) == eRet_Ok)
    {
        _bAddedToDb = true;
    }
} /* httpClientFinishedCallback */

#define HTTP_POST_SIZE  1024
#define HTTP_RBUF_SIZE  2048

static void HttpClientThread(void * pParam)
{
    CDiscoveredServer * pServer         = (CDiscoveredServer *)pParam;
    int                 sd              = pServer->getSocket();
    MString *           pPlayListBuffer = pServer->getPlayListBuffer(); /* protect with mutex! */
    B_MutexHandle       playListMutex   = pServer->getPlayListBufferMutex();
    struct addrinfo     hints;
    struct addrinfo *   addrInfo     = NULL;
    struct addrinfo *   addrInfoNode = NULL;
    int                 rc           = 0;
    char *              post         = NULL;
    char *              rbuf         = NULL;
    ssize_t             n            = 0;
    ssize_t             len          = 0;
    char *              p            = NULL;
    char                tok;

    if (sd <= 0)
    {
        BDBG_ERR(("http client socket invalid"));
        goto error;
    }

    BKNI_Memset(&hints, 0, sizeof(hints));
    hints.ai_family   = AF_UNSPEC;    /* we dont know the family */
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    if (getaddrinfo(pServer->getIpAddress().s(), pServer->getIpPort().s(), &hints, &addrInfo) != 0)
    {
        BDBG_ERR(("getaddrinfo failed for server:port: %s:%s\n", pServer->getIpAddress().s(), pServer->getIpPort().s()));
        perror("getaddrinfo");
        goto error;
    }
    for (addrInfoNode = addrInfo; addrInfoNode != NULL; addrInfoNode = addrInfo->ai_next)
    {
        if (addrInfoNode->ai_family == AF_INET)
        {
            break;
        }
    }
    if (!addrInfoNode)
    {
        BDBG_ERR(("ERROR: no IPv4 address available for this server, no support for IPv6 yet"));
        goto error;
    }
    addrInfo = addrInfoNode;

    rc = connect(sd, addrInfo->ai_addr, addrInfo->ai_addrlen);
    if (rc < 0)
    {
        BDBG_WRN(("Connect Error: Server busy or unavailable:"));
        goto error;
    }
    BDBG_WRN(("Connected to http://%s:%s", pServer->getIpAddress().s(), pServer->getIpPort().s()));

    post = (char *) BKNI_Malloc(HTTP_POST_SIZE);
    if (!post)
    {
        BDBG_ERR(("Failed to allocate the post buffer"));
        goto error;
    }

    /* Build HTTP Get Request */
    n = snprintf(post, HTTP_POST_SIZE,
            "GET /%s HTTP/1.1\r\n"
            "Host: %s:%s\r\n"
            "Rate: %d\r\n"
            "PlaySpeed.dlna.org: speed=1\r\n"
            "User-Agent: Test App\r\n"
            "connection: keep-alive\r\n"
            "\r\n",
            "playlist",
            pServer->getIpAddress().s(), pServer->getIpPort().s(), 100);
    BDBG_MSG(("Sending HTTP Request:----->[\n%s]\n", post));

    /* Send HTTP Get Request */
    n = write(sd, post, strlen(post));
    if ((unsigned)n != strlen(post))
    {
        BDBG_ERR(("Failed to write HTTP Get Request: rc %d\n", n));
        perror("write(): ");
        goto error;
    }

    rbuf = (char *) BKNI_Malloc(HTTP_RBUF_SIZE);
    if (!rbuf)
    {
        BDBG_ERR(("Failed to allocate the rbuf  buffer"));
        goto error;
    }
    BKNI_Memset(rbuf, 0, HTTP_RBUF_SIZE);
    BKNI_Sleep(100);
    if ((n = read(sd, rbuf, HTTP_RBUF_SIZE)) <= 0)
    {
        BDBG_ERR(("Failed to read HTTP Response: rc = %d\n", n));
        perror("printing error code for read read(): ");
        goto error;
    }
    rbuf[n] = '\0';

    /* Scan for end of HTTP header */
    p = strstr(rbuf, "\r\n\r\n");
    if (p)
    {
        p  += 4;
        tok = *p;
        *p  = 0;
        len = strlen(rbuf);
    }
    else
    {
        BDBG_WRN(("No HTTP Header\n"));
        len = 0;
        p   = rbuf;
        tok = 0;
    }

    BDBG_MSG(("Total response len %d, payload len %d\n", n, n-len));
    BDBG_WRN(("HTTP Response:http://%s:%s -->[\n%s]\n", pServer->getIpAddress().s(), pServer->getIpPort().s(), rbuf));
    *p = tok;

    {
        /* the pPlaylistBuffer is shared so protect with mutex */
        CScopedMutex scopedMutex(playListMutex);

        pPlayListBuffer->clear();
        pPlayListBuffer->strncpy(p, n-len);

        while (!pServer->getShutDownHttpClientThread())
        {
            if ((n = read(sd, rbuf, HTTP_RBUF_SIZE)) <= 0)
            {
                /*BDBG_ERR(("read failed: n = %d\n", n));
                 * perror("read(): ");*/
                break;
            }
            pPlayListBuffer->strncat(rbuf, n);
        }
    }
error:
    if (post)
    {
        BKNI_Free(post);
    }
    if (rbuf)
    {
        BKNI_Free(rbuf);
    }
    triggerHttpClientFinishedCallback(pServer, NULL, 0);
    /* addrInfo is freed later when socket it closed */

    /* coverity[leaked_storage] */
} /* HttpClientThread */

CDiscoveredServer::CDiscoveredServer(const char * strIpAddress) :
    CPlaylist(strIpAddress),
    _timeStamp(0),
    _beaconVersion(0),
    _s_addr(0),
    _serverName(""),
    _strIpAddress(strIpAddress),
    _strPort("89"),
    _pModel(NULL),
    _socket(0),
    _hHttpClientThread(0),
    _bShutdownHttpClientThread(false),
    _playlistBuffer(""),
    _playlistBufferMutex(NULL),
    _bAddedToDb(false)
{
}

CDiscoveredServer::CDiscoveredServer(const CDiscoveredServer &server) :
    CPlaylist(server),
    _timeStamp(server._timeStamp),
    _beaconVersion(server._beaconVersion),
    _s_addr(server._s_addr),
    _serverName(server._serverName),
    _strIpAddress(server._strIpAddress),
    _strPort(server._strPort),
    _pModel(server._pModel),
    _socket(server._socket),
    _hHttpClientThread(0),
    _bShutdownHttpClientThread(false),
    _playlistBuffer(server._playlistBuffer),
    _playlistBufferMutex(NULL),
    _bAddedToDb(server._bAddedToDb)
{
    /* even though the Server is copied you still need to create
     * another HTTP Client thread and Shutdown Client thread */
}

CDiscoveredServer::~CDiscoveredServer()
{
}

void CDiscoveredServer::dump(bool bForce)
{
    BDBG_Level level;

    if (true == bForce)
    {
        BDBG_GetModuleLevel("atlas_auto_discovery", &level);
        BDBG_SetModuleLevel("atlas_auto_discovery", BDBG_eMsg);
    }

    /* print out relevent data here */

    if (true == bForce)
    {
        BDBG_SetModuleLevel("atlas_auto_discovery", level);
    }

    /* call base class */
    CPlaylist::dump(bForce);
} /* dump */

eRet CDiscoveredServer::open(
        CWidgetEngine * pWidgetEngine,
        CModel *        pModel
        )
{
    eRet               ret = eRet_Ok;
    struct sockaddr_in localAddr;
    struct sockaddr_in addr;

    /* already open */
    if (_pModel)
    {
        close();
    }
    ret     = CPlaylist::open(pWidgetEngine);
    _pModel = pModel;

    if (NULL != _pWidgetEngine)
    {
        _pWidgetEngine->addCallback(this, CALLBACK_HTTP_CLIENT_FINISHED, bwinHttpClientFinishedCallback);
    }

    _socket = socket(AF_INET, SOCK_STREAM, 0);
    if (_socket < 0)
    {
        BDBG_ERR(("Socket Open Err"));
        perror("Socket Open Err");
        ret = eRet_NotAvailable;
        goto error;
    }

    memset(&localAddr, 0, sizeof(localAddr));
    localAddr.sin_family      = AF_INET;
    localAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    localAddr.sin_port        = htons(0);

    if (bind(_socket, (struct sockaddr *) &localAddr, sizeof(localAddr)))
    {
        BDBG_ERR(("Socket Bind Err"));
        perror("Socket Bind Err");
        ret = eRet_NotAvailable;
        ::close(_socket);
        goto error;
    }

    /*update the remote server information */
    _strPort.strncpy(pWidgetEngine->getCfg()->get("HTTP_SERVER_PLAYLIST_PORT", "89"));
    addr.sin_addr.s_addr = _s_addr;
    _strIpAddress.strncpy(inet_ntoa(addr.sin_addr));

    _playlistBufferMutex = B_Mutex_Create(NULL);
    CHECK_PTR_ERROR_GOTO("unable to create mutex", _playlistBufferMutex, ret, eRet_OutOfMemory, error);

    {
        CScopedMutex scopedMutex(_playlistBufferMutex);
        _playlistBuffer.clear();
    }

    _hHttpClientThread = B_Thread_Create(NULL, HttpClientThread, this, NULL);
    if (!_hHttpClientThread)
    {
        BDBG_ERR(("Failed to start http client thread"));
        ::close(_socket);
        ret = eRet_ExternalError;
        goto error;
    }
    return(ret);

error:
    _pModel = NULL;
    CPlaylist::close();
    return(ret);
} /* open */

void CDiscoveredServer::close(void)
{
    /* signal to shutdown the thread */
    _bShutdownHttpClientThread = true;
    /* this should unblock the thread */
    if (_socket > 0)
    {
        shutdown(_socket, SHUT_RDWR);
    }
    if (_hHttpClientThread)
    {
        B_Thread_Destroy(_hHttpClientThread);
        _hHttpClientThread = NULL;
    }
    if (_socket > 0)
    {
        ::close(_socket);
        _socket = 0;
    }

    if (false == _playlistBuffer.isEmpty())
    {
        CScopedMutex scopedMutex(_playlistBufferMutex);
        _playlistBuffer.clear();
    }
    if (NULL != _playlistBufferMutex)
    {
        B_Mutex_Destroy(_playlistBufferMutex);
        _playlistBufferMutex = NULL;
    }

    if (getModel())
    {
        if (_bAddedToDb)
        {
            getModel()->getPlaylistDb()->removePlaylist((void *)this, this);
            _bAddedToDb = false;
        }
    }
    if (NULL != _pWidgetEngine)
    {
        _pWidgetEngine->removeCallback(this, CALLBACK_HTTP_CLIENT_FINISHED);
    }
    _pModel = NULL;
    CPlaylist::close();
} /* close */

/*************************** CAutoDiscoveryClient ***************************/

static void bwinServerFoundCallback(
        void *       pObject,
        const char * strCallback
        )
{
    CAutoDiscoveryClient * pClient = (CAutoDiscoveryClient *)pObject;

    BSTD_UNUSED(strCallback);

    pClient->serverFoundCallback();
}

static void bipServerFoundCallback(
        void *       pParam,
        const void * pBuffer,
        uint32_t     nData
        )
{
    CAutoDiscoveryClient * pClient       = (CAutoDiscoveryClient *)pParam;
    CWidgetEngine *        pWidgetEngine = pClient->getWidgetEngine();

    BDBG_ASSERT(NULL != pWidgetEngine);
    BSTD_UNUSED(pBuffer);
    BSTD_UNUSED(nData);
    BDBG_MSG(("atlas server found callback"));

    pWidgetEngine->syncCallback(pClient, CALLBACK_AUTO_DISCOVERY_SERVER_FOUND);
}

void CAutoDiscoveryClient::serverFoundCallback()
{
}

CAutoDiscoveryClient::CAutoDiscoveryClient(
        const char *     name,
        CConfiguration * pCfg
        ) :
    CMvcModel(name),
    _pWidgetEngine(NULL),
    _pCfg(pCfg),
    _pModel(NULL),
    _beaconCollectionIntervalTimer(this),
    _bAutoDiscoveryClientStarted(false),
    _beaconMcastAddress(GET_STR(pCfg, AUTO_DISCOVERY_BEACON_MCAST_ADDRESS)),
    _beaconMcastPort((unsigned short)GET_INT(pCfg, AUTO_DISCOVERY_BEACON_MCAST_PORT)),
    _beaconIntervalMsec(GET_INT(pCfg, AUTO_DISCOVERY_BEACON_INTERVAL)),
    _beaconExpiryTimeout((GET_INT(pCfg, AUTO_DISCOVERY_BEACON_INTERVAL)*4)/1000),
    _beacon_fd(0),
    _bEnabled(GET_BOOL(pCfg, AUTO_DISCOVERY_CLIENT_ENABLED))
{
    _serverList.clear();
    BKNI_Memset(&_beacon_addr, 0, sizeof(_beacon_addr));
    BKNI_Memset(&_beacon_mreq, 0, sizeof(_beacon_mreq));
    BKNI_Memset(_serverName, 0, sizeof(_serverName));
    BKNI_Memset(_message, 0, sizeof(_message));
    if (_bEnabled)
    {
        _pIfInterfaceList = get_ifaddrs();
    }
    else
    {
        _pIfInterfaceList = NULL;
    }
}

CAutoDiscoveryClient::~CAutoDiscoveryClient()
{
    if (!_bEnabled)
    {
        return;
    }

    if (_bAutoDiscoveryClientStarted)
    {
        stop();
    }

    free_ifaddrs(_pIfInterfaceList);

    close();
}

eRet CAutoDiscoveryClient::updateIfAddrs()
{
    eRet ret = eRet_Ok;

    if (!_bEnabled)
    {
        return(ret);
    }

    bool started = _bAutoDiscoveryClientStarted;

    if (started)
    {
        stop();
    }

    free_ifaddrs(_pIfInterfaceList);
    _pIfInterfaceList = get_ifaddrs();

    if (started)
    {
        start();
    }

    return(ret);
} /* updateIfAddrs */

eRet CAutoDiscoveryClient::open(CWidgetEngine * pWidgetEngine)
{
    eRet ret = eRet_Ok;

    BSTD_UNUSED(bipServerFoundCallback);

    if (!_bEnabled)
    {
        return(ret);
    }

    _pWidgetEngine = pWidgetEngine;
    if (NULL != _pWidgetEngine)
    {
        _pWidgetEngine->addCallback(this, CALLBACK_AUTO_DISCOVERY_SERVER_FOUND, bwinServerFoundCallback);
    }

    _beaconCollectionIntervalTimer.setWidgetEngine(_pWidgetEngine);

    _beacon_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (_beacon_fd < 0)
    {
        BDBG_ERR(("Failed to open beacon client socket"));
        ret = eRet_ExternalError;
        goto error_exit;
    }
    _beacon_addr.sin_family      = AF_INET;
    _beacon_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    _beacon_addr.sin_port        = htons(_beaconMcastPort);

    if (bind(_beacon_fd, (struct sockaddr *)&_beacon_addr, sizeof(_beacon_addr)) < 0)
    {
        BDBG_ERR(("Failed to bind client socket port %u", _beaconMcastPort));
        ret = eRet_ExternalError;
        goto error_exit;
    }
    /* make this non blocking socket  */
    if (fcntl(_beacon_fd, F_SETFL, O_NONBLOCK) < 0)
    {
        BDBG_ERR(("Failed to set nonblocking client socket port %u", _beaconMcastPort));
        ret = eRet_ExternalError;
        goto error_exit;
    }
    return(ret);

error_exit:
    if (_beacon_fd > 0)
    {
        ::close(_beacon_fd);
        _beacon_fd = 0;
    }
    return(ret);
} /* open */

void CAutoDiscoveryClient::close()
{
    if (!_bEnabled)
    {
        return;
    }

    removeAllServers();

    if (_bAutoDiscoveryClientStarted)
    {
        stop();
        _bAutoDiscoveryClientStarted = false;
    }

    if (_beaconCollectionIntervalTimer.isStarted())
    {
        _beaconCollectionIntervalTimer.stop();
    }

    _beaconCollectionIntervalTimer.setWidgetEngine(NULL);

    if (_beacon_fd > 0)
    {
        ::close(_beacon_fd);
        _beacon_fd = 0;
    }
    if (NULL != _pWidgetEngine)
    {
        _pWidgetEngine->removeCallback(this, CALLBACK_AUTO_DISCOVERY_SERVER_FOUND);
        _pWidgetEngine = NULL;
    }
} /* close */

eRet CAutoDiscoveryClient::start()
{
    eRet           ret          = eRet_Ok;
    if_interface * pIfInterface = NULL;

    if (!_bEnabled)
    {
        return(ret);
    }

    _beacon_mreq.imr_multiaddr.s_addr = inet_addr(_beaconMcastAddress.s());

    for (pIfInterface = _pIfInterfaceList->first(); pIfInterface; pIfInterface = _pIfInterfaceList->next())
    {
        _beacon_mreq.imr_interface.s_addr = pIfInterface->s_addr;
        /*_beacon_mreq.imr_interface.s_addr = htonl(INADDR_ANY);*/
        BDBG_MSG(("adding %s on %s", _beaconMcastAddress.s(), inet_ntoa(_beacon_mreq.imr_interface)));
        if (setsockopt(_beacon_fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &_beacon_mreq, sizeof(_beacon_mreq)) < 0)
        {
            BDBG_ERR(("Failed to add %s on %s", _beaconMcastAddress.s(), inet_ntoa(_beacon_mreq.imr_interface)));
            ret = eRet_ExternalError;
            goto error_exit;
        }
    }
    _beaconCollectionIntervalTimer.start(1000);
    _bAutoDiscoveryClientStarted = true;
    return(ret);

error_exit:
    return(ret);
} /* start */

eRet CAutoDiscoveryClient::stop()
{
    eRet           ret          = eRet_Ok;
    if_interface * pIfInterface = NULL;

    if (!_bEnabled)
    {
        return(ret);
    }

    for (pIfInterface = _pIfInterfaceList->first(); pIfInterface; pIfInterface = _pIfInterfaceList->next())
    {
        _beacon_mreq.imr_interface.s_addr = pIfInterface->s_addr;
        /*_beacon_mreq.imr_interface.s_addr = htonl(INADDR_ANY);*/
        BDBG_MSG(("removing %s on %s", _beaconMcastAddress.s(), inet_ntoa(_beacon_mreq.imr_interface)));
        if (setsockopt(_beacon_fd, IPPROTO_IP, IP_DROP_MEMBERSHIP, &_beacon_mreq, sizeof(_beacon_mreq)) < 0)
        {
            BDBG_ERR(("Failed to drop %s on %s", _beaconMcastAddress.s(), inet_ntoa(_beacon_mreq.imr_interface)));
            ret = eRet_ExternalError;
            goto error_exit;
        }
    }

    if (_beaconCollectionIntervalTimer.isStarted())
    {
        _beaconCollectionIntervalTimer.stop();
    }
    _bAutoDiscoveryClientStarted = false;
    /* shutdown will cause the recvfrom to return,
     * useful for blocking socket  */
    /* shutdown(_beacon_fd,SHUT_RD); */
    return(ret);

error_exit:
    return(ret);
} /* stop */

void CAutoDiscoveryClient::dump(bool bForce)
{
    BDBG_Level level;

    if (true == bForce)
    {
        BDBG_GetModuleLevel("atlas_auto_discovery", &level);
        BDBG_SetModuleLevel("atlas_auto_discovery", BDBG_eMsg);
    }
    /* print out relevent data here */
    if (true == bForce)
    {
        BDBG_SetModuleLevel("atlas_auto_discovery", level);
    }
}

void CAutoDiscoveryClient::addServer(CDiscoveredServer * pDiscoveredServer)
{
    CDiscoveredServer * pServer = NULL;

    pServer = new CDiscoveredServer(*pDiscoveredServer);
    if (pServer->open(_pWidgetEngine, _pModel) != eRet_Ok)
    {
        BDBG_ERR(("Failed to open discovered server "));
    }
    _serverList.add(pServer);
    BDBG_WRN(("%s joined", pServer->_serverName.s()));
}

void CAutoDiscoveryClient::removeServer(CDiscoveredServer * pDiscoveredServer)
{
    BDBG_WRN(("%s left", pDiscoveredServer->_serverName.s()));
    _serverList.remove(pDiscoveredServer);
    pDiscoveredServer->close();
    delete(pDiscoveredServer);
}

void CAutoDiscoveryClient::removeTimedOutServers(void)
{
    CDiscoveredServer * pServer     = NULL;
    CDiscoveredServer * pServerNext = NULL;
    time_t              timestamp;

    time(&timestamp);

    for (pServer = _serverList.first(); pServer; pServer = pServerNext)
    {
        pServerNext = _serverList.next();
        if ((timestamp- pServer->_timeStamp) > _beaconExpiryTimeout)
        {
            removeServer(pServer);
        }
    }
} /* removeTimedOutServers */

void CAutoDiscoveryClient::removeAllServers(void)
{
    CDiscoveredServer * pServer = NULL;

    while (_serverList.total())
    {
        pServer = _serverList.first();
        removeServer(pServer);
    }
}

void CAutoDiscoveryClient::refreshDiscoveredServers(CDiscoveredServer * pDiscoveredServer)
{
    CDiscoveredServer * pServer     = NULL;
    CDiscoveredServer * pServerNext = NULL;
    time_t              timestamp;
    bool                bAddNewServer = true;

    time(&timestamp);

    if (pDiscoveredServer == NULL)
    {
        return;
    }

    for (pServer = _serverList.first(); pServer; pServer = pServerNext)
    {
        pServerNext = _serverList.next();
        if (pServer->_s_addr == pDiscoveredServer->_s_addr)
        {
            if (pServer->_beaconVersion == pDiscoveredServer->_beaconVersion)
            {
                pServer->_timeStamp = pDiscoveredServer->_timeStamp;
                bAddNewServer       = false;
                break;
            }
            else
            {
                /* beacon version changed, remove and add new server  */
                removeServer(pServer);
                bAddNewServer = true;
                break;
            }
        }
    }
    if (bAddNewServer && pDiscoveredServer->_s_addr)
    {
        addServer(pDiscoveredServer);
    }
    /* BDBG_WRN(("name:%s ip_addr %lx,beacon_ver %d timestamp to expire %d",pServer->_serverName.s(), pServer->_s_addr, pServer->_beaconVersion, pDiscoveredServer->_timeStamp));*/
} /* refreshDiscoveredServers */

void CAutoDiscoveryClient::processServerBeacons(void)
{
    struct sockaddr_in addr;
    socklen_t          addrlen = (socklen_t)sizeof(addr);
    struct timeval     tv;
    int                err = 0;

    tv.tv_usec = 0;
    CDiscoveredServer discoveredServer("");
    int               cnt = 0;

    BKNI_Memset(_message, 0, sizeof(_message));
    BKNI_Memset(_serverName, 0, sizeof(_serverName));
    discoveredServer._beaconVersion = 0;

    while (recvfrom(_beacon_fd, _message, sizeof(_message), 0, (struct sockaddr *)&addr, &addrlen) > 0)
    {
        time(&discoveredServer._timeStamp);
        /* coverity[secure_coding] */
        err = sscanf(_message, "%49s %u", _serverName, &discoveredServer._beaconVersion);
        BDBG_MSG(("Code from Sscanf is %d", err));
        discoveredServer._s_addr = addr.sin_addr.s_addr;
        discoveredServer._serverName.strncpy((const char *)_serverName);
        /*
         * BDBG_WRN(("%s %u",_serverName,discoveredServer._beaconVersion));
         * BDBG_WRN(("url http://%s:%u", inet_ntoa(addr.sin_addr),AUTO_DISCOVERY_HTTP_PORT));
         */
        refreshDiscoveredServers(&discoveredServer);
        BKNI_Memset(_message, 0, sizeof(_message));
        BKNI_Memset(_serverName, 0, sizeof(_serverName));
        discoveredServer._s_addr        = 0;
        discoveredServer._beaconVersion = 0;
        discoveredServer._serverName    = "";
        cnt++;
    }
    BDBG_MSG(("%d beacons received", cnt));
    /* expire the servers not sending out beacons */
    removeTimedOutServers();
} /* processServerBeacons */

void CAutoDiscoveryClient::timerCallback(void * pTimer)
{
    BSTD_UNUSED(pTimer);
    processServerBeacons();
    _beaconCollectionIntervalTimer.start(_beaconIntervalMsec);
}