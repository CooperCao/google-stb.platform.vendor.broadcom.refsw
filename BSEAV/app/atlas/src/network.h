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

#ifndef _NETWORK_H__
#define _NETWORK_H__

#include "resource.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef NETAPP_SUPPORT
#include "netapp.h"
#endif

class CWidgetEngine;

typedef enum eConnectStatus
{
    eConnectStatus_Unknown,
    eConnectStatus_Connecting,
    eConnectStatus_Connected,
    eConnectStatus_Disconnecting,
    eConnectStatus_Disconnected,
    eConnectStatus_Max
} eConnectStatus;

class CNetworkWifiConnectData
{
public:
    CNetworkWifiConnectData() :
        _bConnected(false) {}
    virtual ~CNetworkWifiConnectData() {}

    void          dump(bool bForce = false);
    bool operator ==(CNetworkWifiConnectData &other)
    {
        return((_strSsid == other._strSsid) && (_strPassword == other._strPassword));
    }

public:
    MString _strSsid;
    MString _strPassword;
    bool    _bConnected;
};

class CNetwork : public CResource
{
public:
    CNetwork(
            const char *     name,
            const uint16_t   number,
            CConfiguration * pCfg
            );
    ~CNetwork(void);

    eRet open(CWidgetEngine * pWidgetEngine);
    void close(void);

    eRet connectWifi(const char * strBSSID, const char * strPassword);
    eRet disconnectWifi(void);
    eRet startScanWifi(void);
    eRet stopScanWifi(void);

    void            setScanStatus(eRet scanStatus)                 { _scanStatus = scanStatus; }
    eRet            getScanStatus(void)                            { return(_scanStatus); }
    void            setConnectStatus(eConnectStatus connectStatus) { _connectStatus = connectStatus; }
    eConnectStatus  getConnectStatus(void)                         { return(_connectStatus); }
    CWidgetEngine * getWidgetEngine(void)                          { return(_pWidgetEngine); }
    eRet            getStatusList(MStringHash * pStatus, bool bClear = true);

#ifdef NETAPP_SUPPORT
    eRet                  getConnectedWifiNetwork(NETAPP_WIFI_AP_INFO * pInfo);
    eRet                  getNetworkSettings(NETAPP_IP_SETTINGS * pSettings, NETAPP_IFACE interface = NETAPP_IFACE_WIRELESS);
    eRet                  updateNetwork(NETAPP_WIFI_AP_INFO * pScanResult);
    NETAPP_WIFI_AP_INFO * getNetworkInfo(uint32_t nIndex) { return(_networkList.get(nIndex)); }
    void                  dumpIp(NETAPP_IP_SETTINGS * pSettingsIp, bool bForce = false);
    void                  dump(NETAPP_WIFI_AP_INFO * pInfo, bool bForce = false);
#endif /* ifdef NETAPP_SUPPORT */
    void dump(bool bForce = false);

    void wifiRssiCallback();
    void wifiScanDoneCallback();
    void wifiConnectDoneCallback();
    void wifiUpdateNetworkSettings();
protected:
    CWidgetEngine * _pWidgetEngine;
    eRet            _scanStatus;
    eConnectStatus  _connectStatus;
#ifdef NETAPP_SUPPORT
    NETAPP_HANDLE                  _pNetApp;
    MAutoList<NETAPP_WIFI_AP_INFO> _networkList;
#endif
};

#ifdef __cplusplus
}
#endif

#endif /* NETWORK_H__ */