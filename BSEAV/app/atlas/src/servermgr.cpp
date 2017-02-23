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
#include "servermgr.h"
#include "atlas.h"
#include "atlas_cfg.h"
#include "model.h"

BDBG_MODULE(atlas_servermgr);

CServerMgr::CServerMgr(
        const char *     name,
        CConfiguration * pCfg
        ) :
    CMvcModel(name),
    _pModel(NULL),
    _pWidgetEngine(NULL),
    _pServerHttp(NULL),
    _pServerUdp(NULL),
    _pServerPlaylist(NULL)
{
    BDBG_ASSERT(NULL != pCfg);
    initialize(pCfg);
}

CServerMgr::~CServerMgr()
{
}

eRet CServerMgr::initialize(CConfiguration * pCfg)
{
    _pCfg = pCfg;
    return(eRet_Ok);
}

eRet CServerMgr::open(CWidgetEngine * pWidgetEngine)
{
    eRet ret = eRet_Ok;

    BDBG_ASSERT(NULL != pWidgetEngine);
    _pWidgetEngine = pWidgetEngine;

    /* Create Http Server object */
    _pServerHttp = new CServerHttp(_pCfg);
    CHECK_PTR_ERROR_GOTO("Unable to allocate Http Server object", _pServerHttp, ret, eRet_OutOfMemory, error);

    ret = _pServerHttp->open(_pWidgetEngine, _pCfg);
    CHECK_ERROR_GOTO("Unable to open HTTP server object", ret, error);

    /* Create UDP Server object */
    _pServerUdp = new CServerUdp(_pCfg);
    CHECK_PTR_ERROR_GOTO("Unable to allocate Http Server object", _pServerHttp, ret, eRet_OutOfMemory, error);

    ret = _pServerUdp->open(_pWidgetEngine, _pCfg);
    if (ret != eRet_Ok)
    {
        /* This is optional so print out informational message */
        BDBG_MSG(("UDP server is not started create udpUrl.xml"));
    }

    /* Now Create HTTP Playlist server object.*/
    _pServerPlaylist = new CServerPlaylist(_pCfg);
    CHECK_PTR_ERROR_GOTO("Unable to allocate Http Server object", _pServerHttp, ret, eRet_OutOfMemory, error);

    ret = _pServerPlaylist->open(_pWidgetEngine);
    CHECK_ERROR_GOTO("Unable to open HTTP server playlist object", ret, error);

error:
    return(ret);
} /* open */

eRet CServerMgr::registerObserver(
        CObserver *   observer,
        eNotification notification
        )
{
    eRet ret = eRet_Ok;
    CMvcModel::registerObserver(observer, notification);

    ret = _pServerHttp->registerObserver(observer, notification);
    CHECK_ERROR_GOTO("Unable to registerObserver for _pServerHttp", ret, error);

    if (_pServerUdp)
    {
        ret = _pServerUdp->registerObserver(observer, notification);
        CHECK_ERROR_GOTO("Unable to registerObserver for _pServerHttp", ret, error);
    }

    ret = _pServerPlaylist->registerObserver(observer, notification);
    CHECK_ERROR_GOTO("Unable to registerObserver for _pServerPlaylist", ret, error);

error:
    return(ret);
} /* registerObserver */

eRet CServerMgr::startHttpServer(void)
{
    eRet ret = eRet_Ok;

    if (true == _pServerHttp->isStarted())
    {
        /* server is already started */
        return(ret);
    }

    _pServerHttp->setModel(_pModel);
    /* Start the server.*/
    ret = _pServerHttp->start();
    CHECK_ERROR_GOTO("Unable to start HTTP server object", ret, error);

error:
    return(ret);
} /* startHttpServer */

eRet CServerMgr::startUdpServer(void)
{
    eRet ret = eRet_Ok;

    if (true == _pServerUdp->isStarted())
    {
        /* server is already started */
        return(ret);
    }

    _pServerUdp->setModel(_pModel);
    /* Start the server.*/
    ret = _pServerUdp->start();
    if (ret != eRet_Ok)
    {
        /* this is optional that it is why this is a MSG and not ERR */
        BDBG_MSG(("UDP Server not started "));
        ret = eRet_ExternalError;
    }
error:
    return(ret);
} /* startUdpServer */

eRet CServerMgr::startPlaylistServer(void)
{
    eRet ret = eRet_Ok;

    _pServerPlaylist->setModel(_pModel);
    ret = _pServerPlaylist->start();
    CHECK_ERROR_GOTO("Unable to start Playlist server object", ret, error);

error:
    return(ret);
}

eRet CServerMgr::stopHttpServer(void)
{
    eRet ret = eRet_Ok;

    if (false == _pServerHttp->isStarted())
    {
        /* server is already stopped */
        return(ret);
    }

    /* Stop the server*/
    ret = _pServerHttp->stop();
    CHECK_ERROR_GOTO("Unable to stop HTTP server object", ret, error);

error:
    return(ret);
} /* stopHttpServer */

eRet CServerMgr::stopUdpServer(void)
{
    eRet ret = eRet_Ok;

    if (false == _pServerUdp->isStarted())
    {
        /* server is already stopped */
        return(ret);
    }

    /* Stop the server*/
    ret = _pServerUdp->stop();
    CHECK_ERROR_GOTO("Unable to stop UDP server object", ret, error);

error:
    return(ret);
} /* stopUdpServer */

bool CServerMgr::isUdpServerStarted(void)
{
    return((NULL == _pServerUdp) ? false : _pServerUdp->isStarted());
}

bool CServerMgr::isHttpServerStarted(void)
{
    return((NULL == _pServerHttp) ? false : _pServerHttp->isStarted());
}

eRet CServerMgr::stopPlaylistServer(void)
{
    eRet ret = eRet_Ok;

    ret = _pServerPlaylist->stop();
    CHECK_ERROR_GOTO("Unable to stop HTTP server object", ret, error);

error:
    return(ret);
}

void CServerMgr::close()
{
    stopHttpServer(); /*TODO:Right now we do it here, later it can be moved.*/
    _pServerHttp->close();
    DEL(_pServerHttp);

    if (_pServerUdp)
    {
        stopUdpServer();
        _pServerUdp->close();
        DEL(_pServerUdp);
    }

    stopPlaylistServer();
    _pServerPlaylist->close();
    DEL(_pServerPlaylist);
} /* close */