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
#include "network.h"
#include "atlas.h"
#include "atlas_cfg.h"
#include "convert.h"
#include "nexus_platform_standby.h"

#define CALLBACK_NETAPP_WIFIRSSI         "callbackNetAppWifiRssi"
#define CALLBACK_NETAPP_WIFISCANDONE     "callbackNetAppWifiScanDone"
#define CALLBACK_NETAPP_WIFICONNECTDONE  "callbackNetAppWifiConnectDone"
#define CALLBACK_NETAPP_WIFISETSETTINGS  "callbackNetAppWifiSetSettings"

BDBG_MODULE(atlas_network);

#ifdef NETAPP_SUPPORT

static void bwinNetAppWifiRssiCallback(
        void *       pObject,
        const char * strCallback
        )
{
    CNetwork * pNetwork = (CNetwork *)pObject;

    BSTD_UNUSED(strCallback);

    pNetwork->wifiRssiCallback();
}

static void bwinNetAppWifiScanDoneCallback(
        void *       pObject,
        const char * strCallback
        )
{
    CNetwork * pNetwork = (CNetwork *)pObject;

    BSTD_UNUSED(strCallback);

    pNetwork->wifiScanDoneCallback();
}

static void bwinNetAppWifiConnectDoneCallback(
        void *       pObject,
        const char * strCallback
        )
{
    CNetwork * pNetwork = (CNetwork *)pObject;

    BSTD_UNUSED(strCallback);

    pNetwork->wifiConnectDoneCallback();
}

static void bwinNetAppWifiSetSettings(
        void *       pObject,
        const char * strCallback
        )
{
    CNetwork * pNetwork = (CNetwork *)pObject;

    BSTD_UNUSED(strCallback);

    pNetwork->wifiUpdateNetworkSettings();
}

void CNetwork::wifiRssiCallback()
{
    NETAPP_WIFI_AP_INFO info;

    if (eConnectStatus_Connected != getConnectStatus())
    {
        return;
    }

    /* retrieve results */
    NETAPP_RETCODE retNetAppResults = NETAPP_SUCCESS;
    retNetAppResults = NetAppWiFiGetConnectedApInfo(_pNetApp, NETAPP_IFACE_WIRELESS, &info);

    if (NETAPP_SUCCESS == retNetAppResults)
    {
        updateNetwork(&info);
    }

    notifyObservers(eNotify_NetworkWifiRssiResult, this);
} /* wifiRssiCallback */

void CNetwork::wifiScanDoneCallback()
{
    if (eRet_Ok == getScanStatus())
    {
        NETAPP_WIFI_AP_INFO * pInfoList = NULL;
        uint32_t              nCount    = 0;

        /* retrieve results */
        NETAPP_RETCODE retNetAppResults = NETAPP_SUCCESS;
        retNetAppResults = NetAppWiFiGetScanResults(_pNetApp, NETAPP_IFACE_WIRELESS, &pInfoList, &nCount);

        BDBG_MSG(("wifi scan results num:%d", nCount));
        if (NETAPP_SUCCESS == retNetAppResults)
        {
            /* save results */
            for (uint32_t i = 0; i < nCount; i++)
            {
                updateNetwork(&(pInfoList[i]));
            }
        }
        FRE(pInfoList);
    }
    else
    {
        BDBG_WRN(("Wifi scan failed!"));
    }

    notifyObservers(eNotify_NetworkWifiScanResult, this);
} /* wifiScanDoneCallback */

void CNetwork::wifiConnectDoneCallback()
{
    if (eConnectStatus_Connected == getConnectStatus())
    {
        NETAPP_WIFI_AP_INFO info;

        /* retrieve results */
        NETAPP_RETCODE retNetAppResults = NETAPP_SUCCESS;
        retNetAppResults = NetAppWiFiGetConnectedApInfo(_pNetApp, NETAPP_IFACE_WIRELESS, &info);

        if (NETAPP_SUCCESS == retNetAppResults)
        {
            updateNetwork(&info);
            dump(&info, true);

            /* request dhcp ip address - response handled in wifiUpdateNetworkSettings() */
            NetAppSetNetworkSettings(_pNetApp, NETAPP_IFACE_WIRELESS, NETAPP_IP_MODE_DYNAMIC, NULL);
        }

        notifyObservers(eNotify_NetworkWifiConnected);
    }
    else
    {
        BDBG_WRN(("Wifi connect failed!"));
    }

    notifyObservers(eNotify_NetworkWifiConnectionStatus, this);
} /* wifiConnectDoneCallback */

void CNetwork::wifiUpdateNetworkSettings()
{
    NETAPP_IP_SETTINGS settingsIp;

    BDBG_ASSERT(NULL != _pNetApp);

    if (eRet_Ok == getNetworkSettings(&settingsIp))
    {
        dumpIp(&settingsIp, true);
        notifyObservers(eNotify_NetworkWifiConnectionStatus, this);
    }
}

static void netappCallback(
        void *         pParam,
        NETAPP_CB_TYPE typeCallback,
        const void *   pBuffer,
        uint32_t       nData,
        NETAPP_RETCODE retNetApp,
        NETAPP_IFACE   iFace
        )
{
    CNetwork *      pNetwork      = (CNetwork *)pParam;
    CWidgetEngine * pWidgetEngine = pNetwork->getWidgetEngine();

    BSTD_UNUSED(pBuffer);
    BSTD_UNUSED(nData);
    BSTD_UNUSED(iFace);
    BDBG_MSG(("netapp callback type:%s", netappCallbackToString(typeCallback).s()));

    switch (typeCallback)
    {
    case NETAPP_CB_RSSI_EVENT:
        pWidgetEngine->syncCallback(pNetwork, CALLBACK_NETAPP_WIFIRSSI);
        break;

    case NETAPP_CB_SCAN_DONE:
    {
        /* sync with bwin loop */
        if (NULL != pWidgetEngine)
        {
            pNetwork->setScanStatus(NETAPP_SUCCESS == retNetApp ? eRet_Ok : eRet_ExternalError);
            pWidgetEngine->syncCallback(pNetwork, CALLBACK_NETAPP_WIFISCANDONE);
        }
    }
    break;

    case NETAPP_CB_CONNECT:
    {
        /* sync with bwin loop */
        if ((NULL != pWidgetEngine) && (iFace == NETAPP_IFACE_WIRELESS))
        {
            pNetwork->setConnectStatus(NETAPP_SUCCESS == retNetApp ? eConnectStatus_Connected : eConnectStatus_Disconnected);
            pWidgetEngine->syncCallback(pNetwork, CALLBACK_NETAPP_WIFICONNECTDONE);
        }
    }
    break;

    case NETAPP_CB_SETSETTINGS:
        /* sync with bwin loop */
        if (NULL != pWidgetEngine)
        {
            pWidgetEngine->syncCallback(pNetwork, CALLBACK_NETAPP_WIFISETSETTINGS);
        }
        break;

    default:
        break;
    } /* switch */
}     /* netappCallback */

CNetwork::CNetwork(
        const char *     name,
        const unsigned   number,
        CConfiguration * pCfg
        ) :
    CResource(name, number, eBoardResource_network, pCfg),
    _pWidgetEngine(NULL),
    _scanStatus(eRet_Ok),
    _bScanStarted(false),
    _connectStatus(eConnectStatus_Unknown),
#ifdef NETAPP_SUPPORT
    _pNetApp(NULL)
#endif
{
}

CNetwork::~CNetwork()
{
    close();
}

#include <sys/stat.h>
eRet CNetwork::open(CWidgetEngine * pWidgetEngine)
{
    eRet                 ret       = eRet_Ok;
    NETAPP_RETCODE       retNetApp = NETAPP_SUCCESS;
    NETAPP_SETTINGS      settings;
    NETAPP_OPEN_SETTINGS settingsOpen;
    NETAPP_INIT_SETTINGS settingsInit;
    static char          strCwd[1024];

    memset(strCwd, 0, sizeof(strCwd));
    strncpy(strCwd, GET_STR(_pCfg, NETWORK_DB_PATH), sizeof(strCwd));

    _pWidgetEngine = pWidgetEngine;

    if (NULL != _pWidgetEngine)
    {
        _pWidgetEngine->addCallback(this, CALLBACK_NETAPP_WIFIRSSI, bwinNetAppWifiRssiCallback);
        _pWidgetEngine->addCallback(this, CALLBACK_NETAPP_WIFISCANDONE, bwinNetAppWifiScanDoneCallback);
        _pWidgetEngine->addCallback(this, CALLBACK_NETAPP_WIFICONNECTDONE, bwinNetAppWifiConnectDoneCallback);
        _pWidgetEngine->addCallback(this, CALLBACK_NETAPP_WIFISETSETTINGS, bwinNetAppWifiSetSettings);
    }

    retNetApp = NetAppGetDefaultSettings(&settings);
    CHECK_NETAPP_ERROR_GOTO("unable to get default settings", ret, retNetApp, error);
    settings.bAutoReConnect = true;
    settings.bForceWiFi     = true;

    retNetApp = NetAppGetDefaultInitSettings(&settingsInit);
    CHECK_NETAPP_ERROR_GOTO("unable to get default init settings", ret, retNetApp, error);
    settingsInit.tTaskPriority = NETAPP_OS_PRIORITY_LOWEST;

    {
        struct stat stats;
        if ((stat(strCwd, &stats) == 0) && S_ISDIR(stats.st_mode))
        {
            /* NETWORK_DB_PATH is valid so use it */
            settingsInit.pDBPath = strCwd;
        }
    }

    BDBG_WRN(("using Network database path:%s", settingsInit.pDBPath));

    settingsOpen.tCallback = netappCallback;
    settingsOpen.pParam    = this;

    retNetApp = NetAppOpen(&_pNetApp, &settingsOpen, &settingsInit, &settings);
    CHECK_NETAPP_ERROR_GOTO("unable to open NetApp", ret, retNetApp, error);

error:
    return(ret);
} /* open */

void CNetwork::close()
{
    if (NULL != _pNetApp)
    {
        NetAppClose(_pNetApp);
        _pNetApp = NULL;
    }

    if (NULL != _pWidgetEngine)
    {
        _pWidgetEngine->removeCallback(this, CALLBACK_NETAPP_WIFIRSSI);
        _pWidgetEngine->removeCallback(this, CALLBACK_NETAPP_WIFISCANDONE);
        _pWidgetEngine->removeCallback(this, CALLBACK_NETAPP_WIFICONNECTDONE);
        _pWidgetEngine->removeCallback(this, CALLBACK_NETAPP_WIFISETSETTINGS);
        _pWidgetEngine = NULL;
    }
} /* close */

eRet CNetwork::connectWifi(
        const char * ssid,
        const char * password
        )
{
    eRet                ret       = eRet_Ok;
    NETAPP_RETCODE      retNetApp = NETAPP_SUCCESS;
    NETAPP_WIFI_AP_INFO settingsConnect;
    MString             strSSID(ssid);
    MString             strPassword(password);

    BDBG_ASSERT(NULL != _pNetApp);
    BDBG_ASSERT(false == strSSID.isNull());

    if (eConnectStatus_Connecting == getConnectStatus())
    {
        BDBG_WRN(("attempting to connect while existing connection attempt has not yet completed"));
        return(eRet_Busy);
    }

    if (eConnectStatus_Disconnected != getConnectStatus())
    {
        ret = disconnectWifi();
        CHECK_ERROR_GOTO("failure disconnecting wifi from current network", ret, error);
    }

    memset(&settingsConnect, 0, sizeof(NETAPP_WIFI_AP_INFO));

    strncpy(settingsConnect.cSSID, strSSID.s(), NETAPP_MAX_SSID_LEN);

    if (false == strPassword.isNull())
    {
        strncpy(settingsConnect.cPassword, strPassword.s(), NETAPP_MAX_PASSWORD_LEN);
    }

    BDBG_MSG(("connectWifi ssid:%s pass:%s", settingsConnect.cSSID, settingsConnect.cPassword));

    settingsConnect.tSecurity = NETAPP_WIFI_SECURITY_AUTO_DETECT;
    setConnectStatus(eConnectStatus_Connecting);

    retNetApp = NetAppWiFiConnect(_pNetApp, NETAPP_IFACE_WIRELESS, &settingsConnect);
    CHECK_NETAPP_ERROR_GOTO("unable to connect to wifi", ret, retNetApp, error);

error:
    notifyObservers(eNotify_NetworkWifiConnectionStatus, this);
    return(ret);
} /* connectWifi */

eRet CNetwork::disconnectWifi()
{
    eRet           ret       = eRet_Ok;
    NETAPP_RETCODE retNetApp = NETAPP_SUCCESS;

    BDBG_ASSERT(NULL != _pNetApp);

    if (eConnectStatus_Disconnecting == getConnectStatus())
    {
        BDBG_WRN(("attempting to disconnect while existing disconnect attempt has not yet completed"));
        return(eRet_Busy);
    }

    if (eConnectStatus_Connected != getConnectStatus())
    {
        BDBG_WRN(("cannot disconnect - not currently connected"));
        goto done;
    }

    BDBG_MSG(("disconnectWifi()"));
    setConnectStatus(eConnectStatus_Disconnecting);

    retNetApp = NetAppWiFiDisconnect(_pNetApp, NETAPP_IFACE_WIRELESS);
    CHECK_NETAPP_ERROR_GOTO("unable to disconnect from wifi", ret, retNetApp, error);

    setConnectStatus(eConnectStatus_Disconnected);
done:
    notifyObservers(eNotify_NetworkWifiDisconnected);
error:
    notifyObservers(eNotify_NetworkWifiConnectionStatus, this);
    return(ret);
} /* disconnectWifi */

eRet CNetwork::startScanWifi()
{
    eRet           ret       = eRet_Ok;
    NETAPP_RETCODE retNetApp = NETAPP_SUCCESS;

    BDBG_ASSERT(NULL != _pNetApp);
    if (true == isScanStarted())
    {
        return(ret);
    }

    BDBG_MSG(("start wifi scan"));
    retNetApp = NetAppWiFiStartScan(_pNetApp, NETAPP_IFACE_WIRELESS, 5000, 200);
    CHECK_NETAPP_ERROR_GOTO("unable to start wifi scan", ret, retNetApp, error);

    setScanStarted(true);

    notifyObservers(eNotify_NetworkWifiScanStarted);
error:
    return(ret);
} /* startScanWifi */

eRet CNetwork::stopScanWifi()
{
    BDBG_ASSERT(NULL != _pNetApp);

    if (false == isScanStarted())
    {
        return(eRet_Ok);
    }

    BDBG_MSG(("stop wifi scan"));
    NetAppWiFiStopScan(_pNetApp, NETAPP_IFACE_WIRELESS);

    setScanStarted(false);

    notifyObservers(eNotify_NetworkWifiScanStopped);
    return(eRet_Ok);
} /* stopScanWifi */

eRet CNetwork::getStatusList(
        MStringHash * pStatus,
        bool          bClear
        )
{
    NETAPP_WIFI_AP_INFO info;
    NETAPP_IP_SETTINGS  settings;
    char *              pIfaceName = NULL;
    char                str[32];
    char                ipBuf[NETAPP_ENET_LEN + 1];
    eRet                ret = eRet_Ok;

    BDBG_ASSERT(NULL != _pNetApp);

    if (true == bClear)
    {
        pStatus->clear();
    }

    ret = getConnectedWifiNetwork(&info);
    CHECK_ERROR_GOTO("unable to get connected wifi network info", ret, error);
    {
        snprintf(str, 32, "%s", info.cSSID);
        pStatus->add("SSID", str);
        snprintf(str, 32, "%s", info.cBSSID);
        pStatus->add("BSSID", str);
        snprintf(str, 32, "%s", info.bAdHoc ? "Y" : "N");
        pStatus->add("AdHoc", str);
        snprintf(str, 32, "%s", info.bWPS ? "Y" : "N");
        pStatus->add("WPS", str);
        snprintf(str, 32, "%d dB", info.lRSSI);
        pStatus->add("Signal", str);

        switch (info.tChanBandwidth)
        {
        case NETAPP_WIFI_BANDWIDTH_10MHz:
            snprintf(str, 32, "10 MHz");
            break;
        case NETAPP_WIFI_BANDWIDTH_20MHz:
            snprintf(str, 32, "20 MHz");
            break;
        case NETAPP_WIFI_BANDWIDTH_40MHz:
            snprintf(str, 32, "40 MHz");
            break;
        case NETAPP_WIFI_BANDWIDTH_80MHz:
            snprintf(str, 32, "80 MHz");
            break;
        case NETAPP_WIFI_BANDWIDTH_160MHz:
            snprintf(str, 32, "160 MHz");
            break;
        case NETAPP_WIFI_BANDWIDTH_INVALID:
        default:
            snprintf(str, 32, "Invalid");
            break;
        } /* switch */
        pStatus->add("Bandwidth", str);

        snprintf(str, 32, "%d%s Mbps", info.lRate / 2, (info.lRate & 1) ? ".5" : "");
        pStatus->add("Rate", str);
        snprintf(str, 32, "%d dBm", info.lPhyNoise);
        pStatus->add("Noise", str);
    }

    NetAppGetIfaceName(_pNetApp, NETAPP_IFACE_WIRELESS, &pIfaceName);

    ret = getNetworkSettings(&settings);
    CHECK_ERROR_GOTO("unable to get network ip settings", ret, error);
    {
        snprintf(str, 32, "%s", pIfaceName);
        pStatus->add("Interface", str);
        snprintf(str, 32, "%s", settings.cMacAddress);
        pStatus->add("MAC Address", str);
        snprintf(str, 32, "%s", NetAppNtoA(settings.tIpv4Settings.tIpAddress, ipBuf));
        pStatus->add("IP Address", str);
        snprintf(str, 32, "%s", NetAppNtoA(settings.tIpv4Settings.tSubnetMask, ipBuf));
        pStatus->add("Subnet Mask", str);
        snprintf(str, 32, "%s", NetAppNtoA(settings.tIpv4Settings.tGateway, ipBuf));
        pStatus->add("Gateway", str);
        snprintf(str, 32, "%s", NetAppNtoA(settings.tIpv4Settings.tPrimaryDNS, ipBuf));
        pStatus->add("Primary DNS", str);
        snprintf(str, 32, "%s", NetAppNtoA(settings.tIpv4Settings.tSecondaryDNS, ipBuf));
        pStatus->add("Secondary DNS", str);
    }

error:
    DEL(pIfaceName);
    return(ret);
} /* getStatusList */

eRet CNetwork::getConnectedWifiNetwork(NETAPP_WIFI_AP_INFO * pInfo)
{
    eRet           ret       = eRet_Ok;
    NETAPP_RETCODE retNetApp = NETAPP_SUCCESS;

    BDBG_ASSERT(NULL != _pNetApp);

    retNetApp = NetAppWiFiGetConnectedApInfo(_pNetApp, NETAPP_IFACE_WIRELESS, pInfo);
    CHECK_NETAPP_ERROR_GOTO("unable to get network connection info", ret, retNetApp, error);

error:
    return(ret);
}

eRet CNetwork::getNetworkSettings(
        NETAPP_IP_SETTINGS * pSettings,
        NETAPP_IFACE         interface
        )
{
    eRet           ret       = eRet_Ok;
    NETAPP_RETCODE retNetApp = NETAPP_SUCCESS;

    BDBG_ASSERT(NULL != _pNetApp);
    BDBG_ASSERT(NULL != pSettings);

    retNetApp = NetAppGetNetworkSettings(_pNetApp, interface, pSettings);
    CHECK_NETAPP_ERROR_GOTO("unable to get network settings", ret, retNetApp, error);

error:
    return(ret);
}

eRet CNetwork::updateNetwork(NETAPP_WIFI_AP_INFO * pScanResult)
{
    MListItr <NETAPP_WIFI_AP_INFO> itr(&_networkList);
    NETAPP_WIFI_AP_INFO *          pNetwork = NULL;
    eRet ret = eRet_Ok;

    if (true == MString(pScanResult->cSSID).isEmpty())
    {
        goto error;
    }

    /* update wifi network info if already in network list */
    for (pNetwork = itr.first(); pNetwork; pNetwork = itr.next())
    {
        if ((MString(pNetwork->cSSID) == pScanResult->cSSID) &&
            (pNetwork->ulChannel == pScanResult->ulChannel))
        {
            /* found match in list - update it */
            memcpy(pNetwork, pScanResult, sizeof(NETAPP_WIFI_AP_INFO));
            break;
        }
    }

    if (NULL == pNetwork)
    {
        /* add new wifi network */
        NETAPP_WIFI_AP_INFO * pNetworkNew = (NETAPP_WIFI_AP_INFO *)malloc(sizeof(NETAPP_WIFI_AP_INFO));
        CHECK_PTR_ERROR_GOTO("unable to allocate new network memory", pNetworkNew, ret, eRet_OutOfMemory, error);

        memcpy(pNetworkNew, pScanResult, sizeof(NETAPP_WIFI_AP_INFO));
        BDBG_WRN(("add new network to network list:%s ch:%d", pNetworkNew->cSSID, pNetworkNew->ulChannel));
        _networkList.add(pNetworkNew);
    }

error:
    return(ret);
} /* updateNetwork */

void CNetwork::dumpIp(
        NETAPP_IP_SETTINGS * pSettingsIp,
        bool                 bForce
        )
{
    BDBG_Level level;

    BDBG_ASSERT(NULL != pSettingsIp);

    if (true == bForce)
    {
        BDBG_GetModuleLevel("atlas_network", &level);
        BDBG_SetModuleLevel("atlas_network", BDBG_eMsg);
    }

    {
        char ipBuf[NETAPP_ENET_LEN + 1];

        BDBG_MSG(("Mac Address: %s", pSettingsIp->cMacAddress));
        BDBG_MSG(("IP Address: %s", NetAppNtoA(pSettingsIp->tIpv4Settings.tIpAddress, ipBuf)));
        BDBG_MSG(("Subnet Mask: %s", NetAppNtoA(pSettingsIp->tIpv4Settings.tSubnetMask, ipBuf)));
        BDBG_MSG(("Gateway: %s", NetAppNtoA(pSettingsIp->tIpv4Settings.tGateway, ipBuf)));
        BDBG_MSG(("Primary DNS: %s", NetAppNtoA(pSettingsIp->tIpv4Settings.tPrimaryDNS, ipBuf)));
        BDBG_MSG(("Secondary DNS: %s", NetAppNtoA(pSettingsIp->tIpv4Settings.tSecondaryDNS, ipBuf)));
    }

    if (true == bForce)
    {
        BDBG_SetModuleLevel("atlas_network", level);
    }
} /* dumpIp */

void CNetwork::dump(
        NETAPP_WIFI_AP_INFO * pInfo,
        bool                  bForce
        )
{
    BDBG_Level level;

    BDBG_ASSERT(NULL != pInfo);

    if (true == bForce)
    {
        BDBG_GetModuleLevel("atlas_network", &level);
        BDBG_SetModuleLevel("atlas_network", BDBG_eMsg);
    }

    BDBG_MSG(("SSID:%s", pInfo->cSSID));
    BDBG_MSG(("BSSID:%s", pInfo->cBSSID));
    BDBG_MSG(("Password:%s", pInfo->cPassword));
    BDBG_MSG(("Signal Strength:%ld dB", pInfo->lRSSI));
    BDBG_MSG(("Supported Modes A:%d B:%d G:%d N:%d AC:%d",
              pInfo->tMode & NETAPP_WIFI_802_11_MODE_A ? 1 : 0,
              pInfo->tMode & NETAPP_WIFI_802_11_MODE_B ? 1 : 0,
              pInfo->tMode & NETAPP_WIFI_802_11_MODE_G ? 1 : 0,
              pInfo->tMode & NETAPP_WIFI_802_11_MODE_N ? 1 : 0,
              pInfo->tMode & NETAPP_WIFI_802_11_MODE_AC ? 1 : 0));
    BDBG_MSG(("Security Type auto:%d none:%d wep:%d wpaAES:%d wpaTKIP:%d wpa2AES:%d wpaTKIP:%d noSupport:%d",
              pInfo->tSecurity & NETAPP_WIFI_SECURITY_AUTO_DETECT ? 1 : 0,
              pInfo->tSecurity & NETAPP_WIFI_SECURITY_NONE ? 1 : 0,
              pInfo->tSecurity & NETAPP_WIFI_SECURITY_WEP ? 1 : 0,
              pInfo->tSecurity & NETAPP_WIFI_SECURITY_WPA_PSK_AES ? 1 : 0,
              pInfo->tSecurity & NETAPP_WIFI_SECURITY_WPA_PSK_TKIP ? 1 : 0,
              pInfo->tSecurity & NETAPP_WIFI_SECURITY_WPA2_PSK_AES ? 1 : 0,
              pInfo->tSecurity & NETAPP_WIFI_SECURITY_WPA2_PSK_TKIP ? 1 : 0,
              pInfo->tSecurity & NETAPP_WIFI_SECURITY_NOT_SUPPORTED ? 1 : 0));
    BDBG_MSG(("AdHoc:%d", pInfo->bAdHoc));
    BDBG_MSG(("WPS:%d", pInfo->bWPS));
    BDBG_MSG(("Center Channel:%d", pInfo->ulChannel));
    BDBG_MSG(("Primary Channel:%d", pInfo->ulPrimChannel));
    BDBG_MSG(("Rate:%d%s Mbps", pInfo->lRate / 2, (pInfo->lRate & 1) ? ".5" : ""));
    BDBG_MSG(("Noise:%d dBm", pInfo->lPhyNoise));
    switch (pInfo->tChanBandwidth)
    {
    case NETAPP_WIFI_BANDWIDTH_INVALID:
        BDBG_MSG(("Bandwidth:invalid"));
        break;
    case NETAPP_WIFI_BANDWIDTH_10MHz:
        BDBG_MSG(("Bandwidth:10MHz"));
        break;
    case NETAPP_WIFI_BANDWIDTH_20MHz:
        BDBG_MSG(("Bandwidth:20MHz"));
        break;
    case NETAPP_WIFI_BANDWIDTH_40MHz:
        BDBG_MSG(("Bandwidth:40MHz"));
        break;
    case NETAPP_WIFI_BANDWIDTH_80MHz:
        BDBG_MSG(("Bandwidth:80MHz"));
        break;
    case NETAPP_WIFI_BANDWIDTH_160MHz:
        BDBG_MSG(("Bandwidth:160MHz"));
        break;
    default:
        break;
    } /* switch */
    BDBG_MSG(("channel spec:%s", pInfo->cChannelSpec));

    if (true == bForce)
    {
        BDBG_SetModuleLevel("atlas_network", level);
    }
} /* dump */

void CNetwork::dump(bool bForce)
{
    BDBG_Level level;

    MListItr <NETAPP_WIFI_AP_INFO> itr(&_networkList);
    NETAPP_WIFI_AP_INFO *          pNetwork = NULL;

    if (true == bForce)
    {
        BDBG_GetModuleLevel("atlas_network", &level);
        BDBG_SetModuleLevel("atlas_network", BDBG_eMsg);
    }

    BDBG_MSG(("Wifi network list:"));
    for (pNetwork = itr.first(); pNetwork; pNetwork = itr.next())
    {
        dump(pNetwork, bForce);
    }

    if (true == bForce)
    {
        BDBG_SetModuleLevel("atlas_network", level);
    }
} /* dump */

#endif /* ifdef NETAPP_SUPPORT */