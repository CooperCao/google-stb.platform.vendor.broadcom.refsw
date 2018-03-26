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

#ifndef _SERVER_UDP_H__
#define _SERVER_UDP_H__

#include "resource.h"
#include "server.h"
#include "bip.h"

#ifdef __cplusplus
extern "C" {
#endif

class CWidgetEngine;
class CModel;
class CStreamerUdp;

class CServerUdp : public CServer
{
public:
    CServerUdp(CConfiguration * pCfg);
    virtual ~CServerUdp(void);
    eRet open(CWidgetEngine * pWidgetEngine, CConfiguration * pCfg);
    void close(void);
    /* TODO: generateMediaInfoForFiles is called by server_udp and need to be moved to server class.
     * This function will not be called by server_udp.h */
    void processRecvdRequest(void) { return; }
    eRet start(void);
    eRet stop(void);
    void stopStreamer(void);
    bool isStarted(void) { return(_serverStarted); }

    void     setModel(CModel * pModel) { _pModel = pModel; }
    CModel * getModel(void)            { return(_pModel); }

    void dump(bool bForce = false);

protected:
    CModel * _pModel;
    MString  _port;              /* Host port on which server will listen for any incoming http request. Defaults to port 80. */
    MString  _url;               /* URL: to be used in the future if we listen on _port for connections  */
    MString  _interfaceName;     /*
                                  *!< Binds Media Server to this specific interface name and listens for HTTP Request exclusively on that interface.
                                  *!< Defaults to none, meaning HttpServer will listen on all interfaces!
                                  */
    MString _mediaDirectoryPath; /* Media directory from which to play Media files . Default is none.Atlas cfg will provide the MediaDirectory path.*/
    MString _infoDirectoryPath;  /* generate mediaInfo under this directory. */

    bool     _pacingEnabled;
    bool     _serverStarted;
    unsigned _maxConcurrentRequestsToQueue;    /* Maximum HTTP Requests HttpServer will queue & will not accept new connections App calls BIP_HttpServer_RecvRequest to rcv a Request.*/
    unsigned _persistentConnectionTimeoutInMs; /* Duration in msec after which "idle" Http Sockets will be timed out (idle means HTTP Request hasn't arrived for this duration) */

    MList <CStreamerUdp> _streamerList; /* List of Streamer Ctx- each has a unique URL read from udpUrl.xml */

    eRet readXML(const char * udpUrlFile, CConfiguration * pCfg); /* Private readXML function */
};

#ifdef __cplusplus
}
#endif

#endif /* SERVER_HTTP_H__ */