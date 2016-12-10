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

#ifndef WIFI_H__
#define WIFI_H__

#include "view.h"
#include "bwidgets.h"
#include "config.h"
#include "action.h"
#include "network.h"

#ifdef WPA_SUPPLICANT_SUPPORT
typedef void (* CWifiWpaCallback)(char * msg, size_t len);
class CModel;

class CWifiCommand
{
public:
    CWifiCommand() {}
    virtual ~CWifiCommand() {}
public:
    MString _strSSID;
    MString _strPassword;
};

class CWifiResponse
{
public:
    CWifiResponse(void) :
        _notification(eNotify_Invalid),
        _pAction(NULL) {}
    CWifiResponse(
            CAction * pAction,
            MString   strResponse
            ) :
        _notification(eNotify_Invalid),
        _pAction(pAction),
        _strResponse(strResponse)
    {
        BDBG_ASSERT(NULL != pAction);
        _notification = pAction->getId();
    }

    virtual ~CWifiResponse() { DEL(_pAction); }

public:
    eNotification _notification;
    CAction *     _pAction;
    MString       _strResponse;
};

class CNetworkWifi
{
public:
    CNetworkWifi(void) :
        _networkId(0),
        _bAdHoc(false),
        _bWPS(false),
        _bandwidth(0),
        _signalToNoise(0),
        _signalLevel(0) {}
    virtual ~CNetworkWifi() {}

    bool operator ==(CNetworkWifi &other)
    {
        return(_strBSSID == other._strBSSID);
    }

    MString getChannelNum(void);
    bool    hasSecurity(void) { return(-1 < _strFlags.find("WPA") ? true : false); }
public:
    uint32_t _networkId;
    MString  _strSSID;
    MString  _strBSSID;
    MString  _strFrequency;
    MString  _strPassword;
    MString  _strFlags;
    bool     _bAdHoc;
    bool     _bWPS;
    MString  _strRSSI;
    int32_t  _bandwidth;
    int32_t  _signalToNoise;
    int32_t  _signalLevel;
};

class CWifi : public CResource
{
public:
    CWifi(
            const char *     name,
            const uint16_t   number,
            CConfiguration * pCfg
            );
    virtual ~CWifi(void);

    eRet open(CWidgetEngine * _pWidgetEngine);
    void close(void);
    eRet start(void);
    void stop(void);

    void wifiRssiCallback();
    void wifiScanDoneCallback();
    void wifiConnectDoneCallback();
    void wifiUpdateNetworkSettings();

    eRet                      readInterface(void);
    eRet                      connectWifi(const char * strSSID, const char * strPassword);
    eRet                      disconnectWifi(void);
    eRet                      startScanWifi(void);
    eRet                      stopScanWifi(void);
    eRet                      retrieveScanResults(void);
    eRet                      updateNetworks(void);
    eRet                      updateNetworksParse(MString * pStrResponse);
    eRet                      scanResultsParse(MString * pStrResponse);
    eRet                      connectedNetworkParse(MString * pStrResponse);
    eRet                      bssNoiseLevelParse(MString * pStrResponse);
    eRet                      disconnectedNetworkParse(MString * pStrResponse);
    MAutoList<CNetworkWifi> * getScannedNetworkList(void) { return(&_scannedNetworksList); }
    eRet                      sendRequest(MString strCommand, MString * pStrResponse = NULL, CWifiWpaCallback replyCallback = NULL);
    eRet                      trigger(CWifiResponse * pResponse);
    CWidgetEngine *           getWidgetEngine(void) { return(_pWidgetEngine); }
    void                      addAction(CAction * pAction);
    CAction *                 getAction(void);
    CAction *                 removeAction(void);
    void                      addResponse(CWifiResponse * pResponse);
    CWifiResponse *           getResponse(void);
    CWifiResponse *           removeResponse(void);
    eRet                      trigger(CAction * pAction);
    void                      setModel(CModel * pModel)    { _pModel = pModel;  }
    CModel *                  getModel(void)               { return(_pModel); }
    bool                      getStartState(void)          { return(_bThreadRun); }
    void                      setStartState(bool bStarted) { _bThreadRun = bStarted; }
    MStringHash *             getConnectedStatus(void)     { return(&_connectedStatusHash); }
    MString                   getConnectedBSSID(void)      { return(_connectedStatusHash.get("bssid")); }
    void                      clearConnectedStatus(void)   { _connectedStatusHash.clear(); }
    bool                      isNetworkListEmpty(void)     { return(0 < _networksList.total() ? false : true); }

    struct wpa_ctrl * getWpaControl(void) { return(_pWpaControl); }
    struct wpa_ctrl * getWpaMonitor(void) { return(_pWpaMonitor); }

public:
    bool          _bThreadRun;
    uint16_t      _pollInterval;
    B_EventHandle _actionEvent;

protected:
    CWidgetEngine *         _pWidgetEngine;
    eRet                    _scanStatus;
    bool                    _bScanStarted;
    eConnectStatus          _connectStatus;
    MAutoList<CNetworkWifi> _networksList;
    MAutoList<CNetworkWifi> _scannedNetworksList;
    MString                 _strInterfacePath;
    MString                 _strInterfaceName;
    MStringHash             _connectedStatusHash;
    int32_t                 _noiseLevel;

    B_ThreadHandle        _threadWorker;
    MList <CAction>       _actionList;
    B_MutexHandle         _actionMutex;
    MList <CWifiResponse> _responseList;
    B_MutexHandle         _responseMutex;
    CModel *              _pModel;

    struct wpa_ctrl * _pWpaControl;
    struct wpa_ctrl * _pWpaMonitor;
};

#endif /* WPA_SUPPLICANT_SUPPORT */
#endif /* WIFI_H__ */