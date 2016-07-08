/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its
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
 *****************************************************************************/

#ifndef _AUTO_DISCOVERY_H__
#define _AUTO_DISCOVERY_H__

#include <netinet/in.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include "timer.h"
#include "resource.h"
#include "channel.h"
#include "playlist.h"
#include "b_os_lib.h"
#include "model.h"
#include "atlas_cfg.h"
#include "atlas_os.h"

#ifdef __cplusplus
extern "C" {
#endif

class CWidgetEngine;

class CAutoDiscoveryServer : public CMvcModel
{
public:
    CAutoDiscoveryServer(
            const char *     name,
            CConfiguration * pCfg
            );
    virtual ~CAutoDiscoveryServer(void);

    eRet open(CWidgetEngine * pWidgetEngine);
    void close(void);

    void connectCallback(void);

    void dump(bool bForce = false);

    CWidgetEngine * getWidgetEngine(void) { return(_pWidgetEngine); }
    void            timerCallback(void * pTimer);

    /* Call this API whenever new n/w intreface has added or deleted */
    eRet updateIfAddrs(void);

    /* This is sent out along with beacon */
    void setServerName(const char * name) { _server_name = name; }
    /* This will trigger new playlist generation by all the clients */
    void updateBeaconVersion() { _beacon_version++; }

protected:
    CWidgetEngine *    _pWidgetEngine;
    CConfiguration *   _pCfg;
    CTimer             _beaconIntervalTimer;
    MString            _server_name;
    MString            _beaconMcastAddress;
    unsigned short     _beaconMcastPort;
    int                _beaconIntervalMsec;
    unsigned           _beacon_version;
    int                _beacon_fd;
    struct sockaddr_in _beacon_addr;
    bool               _bEnabled;
    /* list of valid interface names */
    MList <if_interface> * _pIfInterfaceList;
private:
    char _beacon[50];
};

class CDiscoveredServer : public CPlaylist
{
public:
    CDiscoveredServer(const char * strIpAddress);
    CDiscoveredServer(const CDiscoveredServer &server);
    virtual ~CDiscoveredServer(void);

    void    dump(bool bForce = false);
    MString getIpAddress() { return(_strIpAddress); }
    MString getIpPort()    { return(_strPort); }

    /* async call will obtain the playlist from remote server and add itself to db */
    eRet open(CWidgetEngine * pWidgetEngine, CModel * pModel);
    void close(void);
    bool getShutDownHttpClientThread(void) { return(_bShutdownHttpClientThread); }
    void httpClientFinishedCallback();
    int  getSocket(void) { return(_socket); }

    /* IMPORTANT! you must use CScopedMutex() with getPlaylistMutex() if you are going to
     * directly access the _playlistBuffer via getPlaylistBuffer() */
    MString *     getPlayListBuffer(void)      { return(&_playlistBuffer); }
    B_MutexHandle getPlayListBufferMutex(void) { return(_playlistBufferMutex); }

    CModel * getModel(void) { return(_pModel); }

    /*public for direct fast access */
    time_t        _timeStamp;
    unsigned      _beaconVersion;
    unsigned long _s_addr;
    MString       _serverName;
protected:
    MString        _strIpAddress;
    MString        _strPort;
    CModel *       _pModel;
    int            _socket;
    B_ThreadHandle _hHttpClientThread;
    bool           _bShutdownHttpClientThread;
    MString        _playlistBuffer;
    B_MutexHandle  _playlistBufferMutex;
    bool           _bAddedToDb;
};

class CAutoDiscoveryClient : public CMvcModel
{
public:
    CAutoDiscoveryClient(
            const char *     name,
            CConfiguration * pCfg
            );
    virtual ~CAutoDiscoveryClient(void);

    eRet open(CWidgetEngine * pWidgetEngine);
    void close(void);

    eRet start(void);
    eRet stop(void);

    void     setModel(CModel * pModel) { (_pModel = pModel); }
    CModel * getModel(void)            { return(_pModel); }

    void serverFoundCallback(void);

    void dump(bool bForce = false);

    CWidgetEngine * getWidgetEngine(void) { return(_pWidgetEngine); }

    /* Call this API whenever new n/w intreface has added or deleted */
    eRet updateIfAddrs(void);

    CConfiguration * getConfiguration() { return(_pCfg); }

protected:
    void addServer(CDiscoveredServer * pDiscoveredServer);
    void removeServer(CDiscoveredServer * pDiscoveredServer);
    void removeTimedOutServers(void);
    void removeAllServers(void);
    void refreshDiscoveredServers(CDiscoveredServer * pDiscoveredServer);
    void processServerBeacons(void);
    void timerCallback(void * pTimer);
    CWidgetEngine *           _pWidgetEngine;
    CConfiguration *          _pCfg;
    MList <CDiscoveredServer> _serverList;
    CModel *                  _pModel;
    CTimer                    _beaconCollectionIntervalTimer;
    bool                      _bAutoDiscoveryClientStarted;
    MString                   _beaconMcastAddress;
    unsigned short            _beaconMcastPort;
    int                _beaconIntervalMsec;
    int                _beaconExpiryTimeout;
    struct ip_mreq     _beacon_mreq;
    int                _beacon_fd;
    struct sockaddr_in _beacon_addr;
    /* list of valid interface names */
    MList <if_interface> * _pIfInterfaceList;
    bool                   _bEnabled;
private:
    char _serverName[50];
    char _message[50];
};

#ifdef __cplusplus
}
#endif

#endif /* AUTO_DISCOVERY_H__ */