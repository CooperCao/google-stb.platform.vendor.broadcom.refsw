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

#ifndef _SERVER_PLAYLIST_H__
#define _SERVER_PLAYLIST_H__

#include "resource.h"
#include "server.h"
#include "bip.h"
#include "channel_bip.h"

#ifdef __cplusplus
extern "C" {
#endif

class CWidgetEngine;
class CModel;
class CStreamerHttp;

class CServerPlaylist : public CServer
{
public:
    CServerPlaylist(CConfiguration * pCfg);
    virtual ~CServerPlaylist(void);

    eRet open(CWidgetEngine * pWidgetEngine);
    void close(void);

    eRet start(void);
    eRet stop(void);

    void     setModel(CModel * pModel) { _pModel = pModel; }
    CModel * getModel(void)            { return(_pModel); }

    const char *           getProtocol()                { return(_protocol); }
    const char *           getHost()                    { return(_host); }
    const char *           getPort()                    { return(_port); }
    const char *           getHttpServerListeningPort() { return(_httpServerListeningPort); }
    BIP_HttpResponseHandle getResponseHandle()          { return(_hHttpResponse); }
    BIP_HttpServerHandle   getBipHttpServer(void)       { return(_hHttpPlaylistServer); }

    void stopStreamer(void);

    void processRecvdRequest(void);
    void dump(bool bForce = false);

protected:
    CModel *               _pModel;
    MString                _protocol;
    MString                _host;
    MString                _port;                    /* Host port on which server will listen for any incoming http playlist request. Defaults to port 89. */
    MString                _httpServerListeningPort; /* Host port on which server will listen for any incoming playback. */
    MString                _interfaceName;           /*Binds playlist Server to this specific interface name and listens for HTTP Request exclusively on that interface. */
    BIP_HttpResponseHandle _hHttpResponse;
    BIP_HttpServerHandle   _hHttpPlaylistServer;
};

class CPlaylistGenerator
{
public:
    CPlaylistGenerator(
            CConfiguration *  pCfg,
            CServerPlaylist * pPlaylistServer
            );
    virtual ~CPlaylistGenerator(void);

    eRet initialize(CConfiguration * pCfg);

    eRet open(
            BIP_HttpRequestHandle  hHttpRequest,
            BIP_HttpResponseHandle hHttpResnponse,
            BIP_HttpSocketHandle   hHttpSocket
            );
    MString generateAtlasPlaylist(
            CServerPlaylist * pPlayListServer
            );
    MString generateiOSPlaylist(
            CServerPlaylist * pPlayListServer
            );
    CServerPlaylist * getPlayListServer() { return(_pPlaylistServer); }

    void close(void);

protected:
    MString          _playList;
    CConfiguration * _pCfg;

    CServerPlaylist * _pPlaylistServer;
};

#ifdef __cplusplus
}
#endif

#endif /* ifndef _SERVER_PLAYLIST_H__ */