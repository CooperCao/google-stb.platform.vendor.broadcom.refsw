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
#ifndef SERVERMGR_H__
#define SERVERMGR_H__

#include "resource.h"
#include "server_http.h"
#include "server_udp.h"
#include "server_playlist.h"

#ifdef __cplusplus
extern "C" {
#endif

class CWidgetEngine;
class CServerHttp;
class CServerUdp;

class CServerMgr : public CMvcModel
{
public:
    CServerMgr(
            const char *     name,
            CConfiguration * pCfg
            );
    ~CServerMgr(void);

    eRet initialize(CConfiguration * pCfg);

    eRet open(CWidgetEngine * pWidgetEngine);
    void close(void);

    void     setModel(CModel * pModel) { _pModel = pModel; }
    CModel * getModel(void)            { return(_pModel); }

    CWidgetEngine * getWidgetEngine(void) { return(_pWidgetEngine); }
    eRet            startHttpServer(void);
    eRet            stopHttpServer(void);
    bool            isHttpServerStarted(void);
    eRet            startUdpServer(void);
    eRet            stopUdpServer(void);
    bool            isUdpServerStarted(void);

    eRet startPlaylistServer(void);
    eRet stopPlaylistServer(void);
    eRet registerObserver(CObserver * observer, eNotification notification = eNotify_All);

protected:
    CConfiguration * _pCfg;          /* initialize will initialize this */
    CModel *         _pModel;        /* initialize will initialize this */
    CWidgetEngine *  _pWidgetEngine; /* open will set this*/

    CServerHttp *     _pServerHttp;
    CServerUdp *      _pServerUdp;
    CServerPlaylist * _pServerPlaylist;
};

#ifdef __cplusplus
}
#endif

#endif /* SERVERMGR_H__ */