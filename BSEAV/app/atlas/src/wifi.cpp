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

#include <dirent.h>
#include <signal.h>
#include "atlas.h"
#include "wpa_ctrl.h"
#include "wifi.h"

#define CALLBACK_WIFI  "CallbackWifi"

BDBG_MODULE(atlas_wifi);

STRING_TO_ENUM_INIT_CPP(CWifi, stringToConnectedState, eConnectedState)
STRING_TO_ENUM_START()
STRING_TO_ENUM_ENTRY("UNKNOWN", eConnectedState_Unknown)
STRING_TO_ENUM_ENTRY("ASSOCIATING", eConnectedState_Connecting)
STRING_TO_ENUM_ENTRY("CONNECTING", eConnectedState_Connecting)
STRING_TO_ENUM_ENTRY("4WAY_HANDSHAKE", eConnectedState_Handshake)
STRING_TO_ENUM_ENTRY("ASSOCIATED", eConnectedState_Connecting)
STRING_TO_ENUM_ENTRY("COMPLETED", eConnectedState_Connected)
STRING_TO_ENUM_ENTRY("CONNECTED", eConnectedState_Connected)
STRING_TO_ENUM_ENTRY("DISCONNECTING", eConnectedState_Disconnecting)
STRING_TO_ENUM_ENTRY("DISCONNECTED", eConnectedState_Disconnected)
STRING_TO_ENUM_ENTRY("INACTIVE", eConnectedState_Disconnected)
STRING_TO_ENUM_ENTRY("SCANNING", eConnectedState_Scanning)
STRING_TO_ENUM_ENTRY("FAILURE ASSOC REJECT", eConnectedState_Failure_Assoc_Reject)
STRING_TO_ENUM_ENTRY("FAILURE NETWORK NOT FOUND", eConnectedState_Failure_Network_Not_Found)
STRING_TO_ENUM_ENTRY("FAILURE SSID TEMP DISABLED", eConnectedState_Failure_Ssid_Temp_Disabled)
STRING_TO_ENUM_END(eConnectedState)

ENUM_TO_MSTRING_INIT_CPP(CWifi, connectedStateToString, eConnectedState)
ENUM_TO_MSTRING_START()
ENUM_TO_MSTRING_ENTRY(eConnectedState_Unknown, "UNKNOWN")
ENUM_TO_MSTRING_ENTRY(eConnectedState_Connecting, "CONNECTING")
ENUM_TO_MSTRING_ENTRY(eConnectedState_Handshake, "4WAY_HANDSHAKE")
ENUM_TO_MSTRING_ENTRY(eConnectedState_Connected, "CONNECTED")
ENUM_TO_MSTRING_ENTRY(eConnectedState_Disconnecting, "DISCONNECTING")
ENUM_TO_MSTRING_ENTRY(eConnectedState_Disconnected, "DISCONNECTED")
ENUM_TO_MSTRING_ENTRY(eConnectedState_Scanning, "SCANNING")
ENUM_TO_MSTRING_ENTRY(eConnectedState_Failure_Assoc_Reject, "FAILURE ASSOC REJECT")
ENUM_TO_MSTRING_ENTRY(eConnectedState_Failure_Network_Not_Found, "FAILURE NETWORK NOT FOUND")
ENUM_TO_MSTRING_ENTRY(eConnectedState_Failure_Ssid_Temp_Disabled, "FAILURE SSID TEMP DISABLED")
ENUM_TO_MSTRING_END()

static MStringHash gHashFreqChannel;

static CWifi * g_pWifi = NULL;
/* udhcpc responds to SIGUSR1 for ip address renew and SIGUSR2 for
 * ip address release.  the atlas udhcpc.script will also send these
 * same signals to atlas
 */
static void dhcpChanged(int sig)
{
    CWifiResponse * pResponse = NULL;
    eRet            ret       = eRet_Ok;

    if (SIGUSR1 != sig)
    {
        return;
    }

    pResponse = new CWifiResponse();
    CHECK_PTR_ERROR_GOTO("unable to allocate CWifiResponse", pResponse, ret, eRet_OutOfMemory, error);
    pResponse->_notification = eNotify_NetworkWifiConnectionStatus;

    /* save response and sync with bwin main loop */
    ret = g_pWifi->trigger(pResponse);
    CHECK_ERROR_GOTO("unable to trigger bwin loop", ret, error);

error:
    return;
} /* dhcpChanged */

MString CNetworkWifi::getChannelNum()
{
    return(gHashFreqChannel.get(_strFrequency));
}

static void wpa_msg_cb(
        char * msg,
        size_t len
        )
{
    BDBG_ERR(("wpa_msg_cb():%s", msg));
}

/* bwin io callback that is executed when wifi thread io is triggered.
 * this function handles the responses from worker thread commands
 */
void bwinWifiCallback(
        void *       pObject,
        const char * strCallback
        )
{
    eRet    ret   = eRet_Ok;
    CWifi * pWifi = (CWifi *)pObject;

    BDBG_MSG(("%s - sync'd with main loop", __FUNCTION__));

    BDBG_ASSERT(NULL != pWifi);
    BSTD_UNUSED(strCallback);

    pWifi->processWpaResponse(strCallback);
}

void CWifi::processWpaResponse(const char * strCallback)
{
    eRet            ret       = eRet_Ok;
    CWifiResponse * pResponse = NULL;
    void *          pVoid     = NULL;
    MString         strSsid;

    BSTD_UNUSED(strCallback);

    /* handle all queued Wifi responses events */
    while (NULL != (pResponse = removeResponse()))
    {
        eNotification notifyResponse;

        pVoid          = (void *)this;
        notifyResponse = eNotify_Invalid;

        BDBG_WRN(("%s notification:%s", __FUNCTION__, notificationToString(pResponse->_notification).s()));
        switch (pResponse->_notification)
        {
        case eNotify_NetworkWifiList:
            ret = updateNetworksParse(&(pResponse->_strResponse));
            CHECK_ERROR_CONTINUE("unable to parse update networks response", ret);

            notifyResponse = eNotify_NetworkWifiListUpdated;
            break;

        case eNotify_NetworkWifiScanStart:
            if (-1 != pResponse->_strResponse.find("OK"))
            {
                notifyResponse = eNotify_NetworkWifiScanStarted;
            }
            break;

        case eNotify_NetworkWifiScanStop:
            /* nothing to do */
            break;

        case eNotify_NetworkWifiScanStopped:
            notifyResponse = eNotify_NetworkWifiScanStopped;
            break;

        case eNotify_NetworkWifiScanResultRetrieve:
            ret = scanResultsParse(&(pResponse->_strResponse));
            CHECK_ERROR_CONTINUE("unable to parse scan results response", ret);

            notifyResponse = eNotify_NetworkWifiScanResult;
            break;

        case eNotify_NetworkWifiConnectedNetworkStatus:
        case eNotify_NetworkWifiConnected:
        {
            eConnectedState connectedStateOrig = getConnectedState();

            ret = connectedNetworkParse(&(pResponse->_strResponse));
            CHECK_ERROR_CONTINUE("unable to parse connected network results response", ret);

            {
                eConnectedState connectedState = getConnectedState();
                if (connectedState == eConnectedState_Disconnected)
                {
                    if (connectedStateOrig == eConnectedState_Connected)
                    {
                        /* transitioned from a connected state
                         *  state to disconnected state so stop DHCP */
                        ret = ipAddressAcquisitionStop();
                        CHECK_ERROR("DHCP STOP failure", ret);

                        ret = requestStatus();
                        CHECK_ERROR("Unable to request connection status", ret);
                    }

                    _bScanEnabled = true;

                    notifyResponse = eNotify_NetworkWifiDisconnected;
                }
                else
                if (connectedState == eConnectedState_Connected)
                {
                    if (connectedStateOrig != eConnectedState_Connected)
                    {
                        /* transitioned from a non-connected
                         * state to connected so get IP address */
                        ret = ipAddressAcquisitionStart();
                        CHECK_ERROR("DHCP START failure", ret);

                        ret = requestStatus();
                        CHECK_ERROR("Unable to request connection status", ret);
                    }

                    _bScanEnabled = true;

                    notifyResponse = eNotify_NetworkWifiConnected;
                }
                else
                if ((connectedState == eConnectedState_Connecting) ||
                    (connectedState == eConnectedState_Disconnecting))
                {
                    BDBG_MSG(("intermediate wifi connection state:%s - requesting status again", connectedStateToString(connectedState).s()));
                    BKNI_Sleep(200);
                    /* intermediate status so retry getting status */
                    ret = requestStatus();
                    CHECK_ERROR("Unable to request connection status", ret);
                }
            }
        }

        break;

        case eNotify_NetworkWifiConnect:
            if (-1 != pResponse->_strResponse.find("FAIL"))
            {
                /* connect failure */
                notifyResponse = eNotify_NetworkWifiConnectFailure;
            }
            break;

        case eNotify_NetworkWifiDisconnected:
            ret = disconnectedNetworkParse(&(pResponse->_strResponse));
            CHECK_ERROR_CONTINUE("unable to parse disconnected network results response", ret);

            notifyResponse = eNotify_NetworkWifiDisconnected;
            break;

        case eNotify_NetworkWifiConnectFailure:
        {
            ret = dhcpStop();
            CHECK_ERROR("DHCP STOP failure", ret);

            _nNetworkConnectErrors++;
            BDBG_WRN(("CONNECT FAILURE: #%d", _nNetworkConnectErrors));

            if (-1 != pResponse->_strResponse.find("WRONG_KEY"))
            {
                BDBG_ERR(("Connect failure: wrong key"));
                notifyResponse = eNotify_NetworkWifiConnectFailureWrongKey;

                disconnectWifi();
            }
            else
            if (2 < _nNetworkConnectErrors)
            {
                if (-1 != pResponse->_strResponse.find("CTRL-EVENT-SSID-TEMP-DISABLED"))
                {
                    notifyResponse = eNotify_NetworkWifiConnectFailure;
                }
                else
                if (-1 != pResponse->_strResponse.find("CTRL-EVENT-ASSOC-REJECT"))
                {
                    BDBG_ERR(("Connect failure: association rejected"));
                    notifyResponse = eNotify_NetworkWifiConnectFailureAssocReject;
                }
                else
                if (-1 != pResponse->_strResponse.find("CTRL-EVENT-NETWORK-NOT-FOUND"))
                {
                    BDBG_ERR(("Connect failure: network not found"));
                    notifyResponse = eNotify_NetworkWifiConnectFailureNetworkNotFound;
                }

                disconnectWifi();
            }
            break;
        }

        case eNotify_NetworkWifiScanFailure:
            BDBG_ERR(("Scan failure"));
            notifyResponse = eNotify_NetworkWifiScanFailure;
            break;

        case eNotify_NetworkWifiConnectionStatus:
            notifyResponse = eNotify_NetworkWifiConnectionStatus;
            break;

        case eNotify_NetworkWifiConnectAssocStart:
        {
            int indexSsidBegin = pResponse->_strResponse.find("SSID='") + strlen("SSID='");
            int indexSsidEnd   = pResponse->_strResponse.find("' ", indexSsidBegin);

            strSsid = pResponse->_strResponse.mid(indexSsidBegin, indexSsidEnd - indexSsidBegin);

            notifyResponse = eNotify_NetworkWifiConnectAssocStart;
            pVoid          = (void *)&strSsid;
        }
        break;

        default:
            BDBG_ERR(("%s:%s unhandled", __FUNCTION__, notificationToString(pResponse->_notification).s()));
            break;
        } /* switch */

        if ((eRet_Ok == ret) && (eNotify_Invalid != notifyResponse))
        {
            BDBG_MSG(("Notify Observers for Wifi event code: %s", notificationToString(notifyResponse).s()));
            ret = notifyObservers(notifyResponse, pVoid);
            CHECK_ERROR_GOTO("error notifying observers", ret, error);
        }

        /* this will also delete the original CAction */
        DEL(pResponse);
    }

error:
    return;
} /* processWpaResponse */

CWifi::CWifi(
        const char *     name,
        const uint16_t   number,
        CConfiguration * pCfg
        ) :
    CResource(name, number, eBoardResource_wifi, pCfg),
    _bThreadRun(false),
    _pollInterval(1000),
    _actionEvent(NULL),
    _pWidgetEngine(NULL),
    _scanStatus(eRet_Ok),
    _bScanStarted(false),
    _bScanEnabled(true),
    _connectedState(eConnectedState_Unknown),
    _networksListMutex(NULL),
    _scannedNetworksListMutex(NULL),
    _connectedStatusMutex(NULL),
    _noiseLevel(0),
    _nNetworkConnectErrors(0),
    _threadWorker(NULL),
    _actionMutex(NULL),
    _responseMutex(NULL),
    _parseMutex(NULL),
    _connectedStateMutex(NULL),
    _pModel(NULL),
    _pWpaControl(NULL),
    _pWpaMonitor(NULL)
{
    g_pWifi = this;
}

CWifi::~CWifi()
{
    g_pWifi = NULL;
}

eRet CWifi::readInterface()
{
    eRet ret = eRet_Ok;

    _strInterfacePath = GET_STR(_pCfg, WPA_SUPPLICANT_IFACE_PATH);

    /* get interface from interface directory */
    struct dirent * pDent = NULL;
    DIR *           pDir  = opendir(_strInterfacePath);
    CHECK_PTR_ERROR_GOTO("unable to read interface path (WPA_SUPPLICANT_IFACE_PATH)", pDir, ret, eRet_NotAvailable, error);

    while ((pDent = readdir(pDir)))
    {
        if ((MString(".") == pDent->d_name) || (MString("..") == pDent->d_name))
        {
            continue;
        }

        _strInterfaceName = pDent->d_name;
        BDBG_MSG(("selected WiFi interface:%s", _strInterfaceName.s()));
        break;
    }

    if (true == _strInterfaceName.isEmpty())
    {
        BDBG_ERR(("no interface specified in:%s", _strInterfacePath.s()));
        ret = eRet_NotAvailable;
        goto error;
    }

error:
    return(ret);
} /* readInterface */

eRet CWifi::open(CWidgetEngine * pWidgetEngine)
{
    eRet    ret    = eRet_Ok;
    int     retWpa = 0;
    MString strInterfacePathFull;

    BDBG_ASSERT(NULL != _pCfg);

    if (SIG_ERR == signal(SIGUSR1, dhcpChanged))
    {
        BDBG_WRN(("Error setting signal handler %d (udhcpc renew)", SIGUSR1));
    }
    if (SIG_ERR == signal(SIGUSR2, dhcpChanged))
    {
        BDBG_WRN(("Error setting signal handler %d (udhcpc release)", SIGUSR2));
    }

    _pWidgetEngine = pWidgetEngine;
    if (NULL != _pWidgetEngine)
    {
        _pWidgetEngine->addCallback(this, CALLBACK_WIFI, bwinWifiCallback);
    }

    _networksListMutex = B_Mutex_Create(NULL);
    CHECK_PTR_ERROR_GOTO("unable to create mutex", _networksListMutex, ret, eRet_ExternalError, error);

    _scannedNetworksListMutex = B_Mutex_Create(NULL);
    CHECK_PTR_ERROR_GOTO("unable to create mutex", _scannedNetworksListMutex, ret, eRet_ExternalError, error);

    _connectedStatusMutex = B_Mutex_Create(NULL);
    CHECK_PTR_ERROR_GOTO("unable to create mutex", _connectedStatusMutex, ret, eRet_ExternalError, error);

    _actionMutex = B_Mutex_Create(NULL);
    CHECK_PTR_ERROR_GOTO("unable to create mutex", _actionMutex, ret, eRet_ExternalError, error);

    _responseMutex = B_Mutex_Create(NULL);
    CHECK_PTR_ERROR_GOTO("unable to create mutex", _responseMutex, ret, eRet_ExternalError, error);

    _parseMutex = B_Mutex_Create(NULL);
    CHECK_PTR_ERROR_GOTO("unable to create mutex", _parseMutex, ret, eRet_ExternalError, error);

    _connectedStateMutex = B_Mutex_Create(NULL);
    CHECK_PTR_ERROR_GOTO("unable to create mutex", _connectedStateMutex, ret, eRet_ExternalError, error);

    _actionEvent = B_Event_Create(NULL);
    CHECK_PTR_ERROR_GOTO("unable to create event", _actionEvent, ret, eRet_ExternalError, error);

    ret = readInterface();
    CHECK_ERROR_GOTO("unable to read interface", ret, error);

    strInterfacePathFull = _strInterfacePath + MString("/") + _strInterfaceName;

    _pWpaControl = wpa_ctrl_open(strInterfacePathFull.s());
    CHECK_PTR_ERROR_GOTO("Unable to open control interface", _pWpaControl, ret, eRet_NotAvailable, error);

    _pWpaMonitor = wpa_ctrl_open(strInterfacePathFull.s());
    CHECK_PTR_ERROR_GOTO("Unable to open monitor interface", _pWpaMonitor, ret, eRet_NotAvailable, error);

    retWpa = wpa_ctrl_attach(_pWpaMonitor);
    CHECK_ERROR_GOTO("Unable to register monitor interface for unsolicited messages", retWpa, error);

    /* 2.4GHz to channel mapping */
    gHashFreqChannel.add("2412", "1");
    gHashFreqChannel.add("2417", "2");
    gHashFreqChannel.add("2422", "3");
    gHashFreqChannel.add("2427", "4");
    gHashFreqChannel.add("2432", "5");
    gHashFreqChannel.add("2437", "6");
    gHashFreqChannel.add("2442", "7");
    gHashFreqChannel.add("2447", "8");
    gHashFreqChannel.add("2452", "9");
    gHashFreqChannel.add("2457", "10");
    gHashFreqChannel.add("2462", "11");
    gHashFreqChannel.add("2467", "12");
    gHashFreqChannel.add("2472", "13");
    gHashFreqChannel.add("2484", "14");

    /* 3.65GHz to channel mapping */
    gHashFreqChannel.add("3657", "131");
    gHashFreqChannel.add("3660", "132");
    gHashFreqChannel.add("3662", "132");
    gHashFreqChannel.add("3665", "133");
    gHashFreqChannel.add("3667", "133");
    gHashFreqChannel.add("3670", "134");
    gHashFreqChannel.add("3672", "134");
    gHashFreqChannel.add("3675", "135");
    gHashFreqChannel.add("3677", "135");
    gHashFreqChannel.add("3680", "136");
    gHashFreqChannel.add("3682", "136");
    gHashFreqChannel.add("3685", "137");
    gHashFreqChannel.add("3687", "137");
    gHashFreqChannel.add("3690", "138");
    gHashFreqChannel.add("3692", "138");

    /* 5GHz to channel mapping */
    gHashFreqChannel.add("5035", "7");
    gHashFreqChannel.add("5040", "8");
    gHashFreqChannel.add("5045", "9");
    gHashFreqChannel.add("5055", "11");
    gHashFreqChannel.add("5060", "12");
    gHashFreqChannel.add("5080", "16");
    gHashFreqChannel.add("5170", "34");
    gHashFreqChannel.add("5180", "36");
    gHashFreqChannel.add("5190", "38");
    gHashFreqChannel.add("5200", "40");
    gHashFreqChannel.add("5210", "42");
    gHashFreqChannel.add("5220", "44");
    gHashFreqChannel.add("5230", "46");
    gHashFreqChannel.add("5240", "48");
    gHashFreqChannel.add("5250", "50");
    gHashFreqChannel.add("5260", "52");
    gHashFreqChannel.add("5270", "54");
    gHashFreqChannel.add("5280", "56");
    gHashFreqChannel.add("5290", "58");
    gHashFreqChannel.add("5300", "60");
    gHashFreqChannel.add("5310", "62");
    gHashFreqChannel.add("5320", "64");
    gHashFreqChannel.add("5500", "100");
    gHashFreqChannel.add("5510", "102");
    gHashFreqChannel.add("5520", "104");
    gHashFreqChannel.add("5530", "106");
    gHashFreqChannel.add("5540", "108");
    gHashFreqChannel.add("5550", "110");
    gHashFreqChannel.add("5560", "112");
    gHashFreqChannel.add("5570", "114");
    gHashFreqChannel.add("5580", "116");
    gHashFreqChannel.add("5590", "118");
    gHashFreqChannel.add("5600", "120");
    gHashFreqChannel.add("5610", "122");
    gHashFreqChannel.add("5620", "124");
    gHashFreqChannel.add("5630", "126");
    gHashFreqChannel.add("5640", "128");
    gHashFreqChannel.add("5660", "132");
    gHashFreqChannel.add("5670", "134");
    gHashFreqChannel.add("5680", "136");
    gHashFreqChannel.add("5690", "138");
    gHashFreqChannel.add("5700", "140");
    gHashFreqChannel.add("5710", "142");
    gHashFreqChannel.add("5720", "144");
    gHashFreqChannel.add("5745", "149");
    gHashFreqChannel.add("5755", "151");
    gHashFreqChannel.add("5765", "153");
    gHashFreqChannel.add("5775", "155");
    gHashFreqChannel.add("5785", "157");
    gHashFreqChannel.add("5795", "159");
    gHashFreqChannel.add("5805", "161");
    gHashFreqChannel.add("5825", "165");
    gHashFreqChannel.add("4915", "183");
    gHashFreqChannel.add("4920", "184");
    gHashFreqChannel.add("4925", "185");
    gHashFreqChannel.add("4935", "187");
    gHashFreqChannel.add("4940", "188");
    gHashFreqChannel.add("4945", "189");
    gHashFreqChannel.add("4960", "192");
    gHashFreqChannel.add("4980", "196");

error:
    return(ret);
} /* open */

void CWifi::close()
{
    int retWpa = 0;

    gHashFreqChannel.clear();

    if (NULL != _pWpaMonitor)
    {
        retWpa = wpa_ctrl_detach(_pWpaMonitor);
        CHECK_WIFI_ERROR("unable to detach monitor interface", retWpa);

        wpa_ctrl_close(_pWpaMonitor);
        _pWpaMonitor = NULL;
    }

    if (NULL != _pWpaControl)
    {
        wpa_ctrl_close(_pWpaControl);
        _pWpaControl = NULL;
    }

    if (NULL != _actionEvent)
    {
        B_Event_Destroy(_actionEvent);
        _actionEvent = NULL;
    }

    if (NULL != _connectedStateMutex) { B_Mutex_Destroy(_connectedStateMutex); }
    if (NULL != _parseMutex) { B_Mutex_Destroy(_parseMutex); }
    if (NULL != _responseMutex) { B_Mutex_Destroy(_responseMutex); }
    if (NULL != _actionMutex) { B_Mutex_Destroy(_actionMutex); }
    if (NULL != _networksListMutex) { B_Mutex_Destroy(_networksListMutex); }
    if (NULL != _scannedNetworksListMutex) { B_Mutex_Destroy(_scannedNetworksListMutex); }
    if (NULL != _connectedStatusMutex) { B_Mutex_Destroy(_connectedStatusMutex); }

    if (NULL != _pWidgetEngine)
    {
        _pWidgetEngine->removeCallback(this, CALLBACK_WIFI);
        _pWidgetEngine = NULL;
    }

    signal(SIGUSR2, SIG_DFL);
    signal(SIGUSR1, SIG_DFL);
} /* close */

eRet CWifi::trigger(CWifiResponse * pResponse)
{
    eRet ret = eRet_Ok;

    addResponse(pResponse);

    if (NULL != _pWidgetEngine)
    {
        BDBG_MSG(("Trigger Wifi Response event: %s", notificationToString(pResponse->_notification).s()));

        ret = _pWidgetEngine->syncCallback(this, CALLBACK_WIFI);
        CHECK_ERROR_GOTO("unable to sync with bwin main loop", ret, error);
    }
error:
    return(ret);
}

static void wifiWorkerThread(void * pParam)
{
    CWifi * pWifi = (CWifi *)pParam;
    B_Error berr  = B_ERROR_SUCCESS;
    eRet    ret   = eRet_Ok;
    MString strWpaCommand;
    MString strResponse;
    MString strNetworkNum;

    BDBG_ASSERT(NULL != pWifi);

    /* when first starting thread we will check connection status in case
     * wpa supplicant has already auto connected based on the wpa_supplicant.conf file */
    {
        CWifiResponse * pResponse = NULL;

        /* send status request */
        strWpaCommand = "STATUS";
        ret           = pWifi->sendRequest(strWpaCommand.s(), &strResponse, wpa_msg_cb);
        CHECK_ERROR_GOTO("unable to send request to wpa supplicant", ret, errorStatus);

        /* create response for status event */
        pResponse = new CWifiResponse();
        CHECK_PTR_ERROR_GOTO("unable to allocate CWifiResponse", pResponse, ret, eRet_OutOfMemory, errorStatus);
        pResponse->_notification = eNotify_NetworkWifiConnected;
        pResponse->_strResponse  = strResponse; /* from STATUS */

        /* save response and sync with bwin main loop */
        ret = pWifi->trigger(pResponse);
        CHECK_ERROR("unable to trigger bwin loop", ret);

        goto doneStatus;
errorStatus:
        DEL(pResponse);
    }
doneStatus:

    while (true == pWifi->_bThreadRun)
    {
        CAction *         pAction     = NULL;
        struct wpa_ctrl * pWpaMonitor = pWifi->getWpaMonitor();

        /* check for async messages from wpa supplicant and handle accordingly */
        if (NULL != pWpaMonitor)
        {
            while (wpa_ctrl_pending(pWpaMonitor) > 0)
            {
                char   buf[4096];
                size_t bufLen = sizeof(buf) - 1;

                memset(buf, 0, sizeof(buf));
                if (0 == wpa_ctrl_recv(pWpaMonitor, buf, &bufLen))
                {
                    MString         strBuf(buf);
                    CWifiResponse * pResponse = NULL;

                    BDBG_WRN(("wpa supplicant async monitor messsage recv:%s", buf));
                    if (-1 != strBuf.find("CTRL-EVENT-SCAN-RESULTS"))
                    {
                        pResponse = new CWifiResponse();
                        CHECK_PTR_ERROR_CONTINUE("unable to allocate CWifiResponse", pResponse, ret, eRet_OutOfMemory);
                        pResponse->_notification = eNotify_NetworkWifiScanStopped;
                        pResponse->_strResponse  = strBuf;

                        /* save response and sync with bwin main loop */
                        ret = pWifi->trigger(pResponse);
                        CHECK_ERROR_CONTINUE("unable to trigger bwin loop", ret);
                    }
                    else
                    if (-1 != strBuf.find("CTRL-EVENT-CONNECTED"))
                    {
                        /* update wpa_supplicant.conf with newly connected network */
                        strWpaCommand = "SAVE_CONFIG";
                        ret           = pWifi->sendRequest(strWpaCommand.s(), &strResponse, wpa_msg_cb);
                        CHECK_ERROR_CONTINUE("unable to send request to wpa supplicant", ret);

                        /* we are connected so get status info on newly connected network */
                        strWpaCommand = "STATUS";
                        ret           = pWifi->sendRequest(strWpaCommand.s(), &strResponse, wpa_msg_cb);
                        CHECK_ERROR_CONTINUE("unable to send request to wpa supplicant", ret);
                        /* after checking status, create response for connect event */
                        pResponse = new CWifiResponse();
                        CHECK_PTR_ERROR_CONTINUE("unable to allocate CWifiResponse", pResponse, ret, eRet_OutOfMemory);
                        pResponse->_notification = eNotify_NetworkWifiConnected;
                        pResponse->_strResponse  = strResponse; /* from STATUS */

                        /* save response and sync with bwin main loop */
                        ret = pWifi->trigger(pResponse);
                        CHECK_ERROR_CONTINUE("unable to trigger bwin loop", ret);
                    }
                    else
                    if (-1 != strBuf.find("CTRL-EVENT-DISCONNECTED"))
                    {
                        /* create response for connect event */
                        pResponse = new CWifiResponse();
                        CHECK_PTR_ERROR_CONTINUE("unable to allocate CWifiResponse", pResponse, ret, eRet_OutOfMemory);
                        pResponse->_notification = eNotify_NetworkWifiDisconnected;
                        pResponse->_strResponse  = strBuf;

                        /* save response and sync with bwin main loop */
                        ret = pWifi->trigger(pResponse);
                        CHECK_ERROR_CONTINUE("unable to trigger bwin loop", ret);
                    }
                    else
                    if ((-1 != strBuf.find("CTRL-EVENT-ASSOC-REJECT")) ||
                        (-1 != strBuf.find("CTRL-EVENT-NETWORK-NOT-FOUND")) ||
                        (-1 != strBuf.find("CTRL-EVENT-SSID-TEMP-DISABLED")))
                    {
                        /* create response for failed connect event */
                        pResponse = new CWifiResponse();
                        CHECK_PTR_ERROR_CONTINUE("unable to allocate CWifiResponse", pResponse, ret, eRet_OutOfMemory);
                        pResponse->_notification = eNotify_NetworkWifiConnectFailure;
                        pResponse->_strResponse  = strBuf;

                        /* save response and sync with bwin main loop */
                        ret = pWifi->trigger(pResponse);
                        CHECK_ERROR_CONTINUE("unable to trigger bwin loop", ret);
                    }
                    else
                    if (-1 != strBuf.find("CTRL-EVENT-SCAN-FAILED"))
                    {
                        /* create response for failed scan event */
                        pResponse = new CWifiResponse();
                        CHECK_PTR_ERROR_CONTINUE("unable to allocate CWifiResponse", pResponse, ret, eRet_OutOfMemory);
                        pResponse->_notification = eNotify_NetworkWifiScanFailure;
                        pResponse->_strResponse  = strBuf;

                        /* save response and sync with bwin main loop */
                        ret = pWifi->trigger(pResponse);
                        CHECK_ERROR_CONTINUE("unable to trigger bwin loop", ret);
                    }
                    else
                    if (-1 != strBuf.find("Trying to associate with"))
                    {
                        /* create response for start of associate action */
                        pResponse = new CWifiResponse();
                        CHECK_PTR_ERROR_CONTINUE("unable to allocate CWifiResponse", pResponse, ret, eRet_OutOfMemory);
                        pResponse->_notification = eNotify_NetworkWifiConnectAssocStart;
                        pResponse->_strResponse  = strBuf;

                        /* save response and sync with bwin main loop */
                        ret = pWifi->trigger(pResponse);
                        CHECK_ERROR_CONTINUE("unable to trigger bwin loop", ret);
                    }
                }
            }
        }

        /* wait for new action to perform but timeout every so often to
         * check for quit flag */
        if (NULL == (pAction = pWifi->getAction()))
        {
            B_Event_Reset(pWifi->_actionEvent);
            berr = B_Event_Wait(pWifi->_actionEvent, 250);
            if (B_ERROR_TIMEOUT == berr)
            {
                continue;
            }
        }

        /* handle action */
        if (NULL != (pAction = pWifi->removeAction()))
        {
            BDBG_MSG(("wifiWorkerThread action:%s", notificationToString(pAction->getId()).s()));

            switch (pAction->getId())
            {
            case eNotify_NetworkWifiList:
                strWpaCommand = "LIST_NETWORKS";
                break;

            case eNotify_NetworkWifiScanStart:
                strWpaCommand = "SCAN";
                break;

            case eNotify_NetworkWifiScanStop:
                strWpaCommand = "INVALID";
                break;

            case eNotify_NetworkWifiScanResultRetrieve:
            {
                eRet ret = eRet_Ok;

                /* get noise level before scan results */
                strWpaCommand = "BSS 0";
                ret           = pWifi->sendRequest(strWpaCommand.s(), &strResponse, wpa_msg_cb);
                CHECK_ERROR_CONTINUE("unable to send bss request to wpa supplicant", ret);

                ret = pWifi->bssNoiseLevelParse(&strResponse);
                CHECK_ERROR_CONTINUE("unable to parse bss noise level response", ret);

                strWpaCommand = "SCAN_RESULTS";
            }
            break;

            case eNotify_NetworkWifiConnectedNetworkStatus:
                strWpaCommand = "STATUS";
                break;

            case eNotify_NetworkWifiConnect:
            {
                eRet           ret      = eRet_Ok;
                CWifiCommand * pCmdData = (CWifiCommand *)pAction->getDataPtr();

                BDBG_MSG(("worker thread connect login/password:%s/%s", pCmdData->_strSSID.s(), pCmdData->_strPassword.s()));

                /* add network if it does not already exist */
                if (false == pCmdData->_bWps)
                {
                    strWpaCommand = "LIST_NETWORKS";
                    ret           = pWifi->sendRequest(strWpaCommand.s(), &strResponse, wpa_msg_cb);
                    CHECK_ERROR_CONTINUE("unable to send list networks request to wpa supplicant", ret);

                    ret = pWifi->updateNetworksParse(&strResponse);
                    CHECK_ERROR_CONTINUE("unable to parse update networks response", ret);

                    if (true == pWifi->isNetworkListEmpty())
                    {
                        strWpaCommand = "ADD_NETWORK";
                        ret           = pWifi->sendRequest(strWpaCommand.s(), &strResponse, wpa_msg_cb);
                        CHECK_ERROR_CONTINUE("unable to send add network request to wpa supplicant", ret);
                    }
                }

                if (NULL != pCmdData)
                {
                    /* disconnect the current connected network if necessary */
                    {
                        strWpaCommand = "STATUS";
                        ret           = pWifi->sendRequest(strWpaCommand.s(), &strResponse, wpa_msg_cb);
                        CHECK_ERROR_CONTINUE("unable to send request to wpa supplicant", ret);

                        ret = pWifi->connectedNetworkParse(&strResponse);
                        CHECK_ERROR_CONTINUE("unable to parse update networks response", ret);

                        if (false == pWifi->getConnectedBSSID().isEmpty())
                        {
                            strWpaCommand = "DISCONNECT";
                            ret           = pWifi->sendRequest(strWpaCommand.s(), &strResponse, wpa_msg_cb);
                            CHECK_ERROR_CONTINUE("unable to send request to wpa supplicant", ret);
                            if (strResponse == "OK")
                            {
                                pWifi->setConnectedState("DISCONNECTED");
                            }
                        }
                    }

                    /* set ssid and password if necesary */
                    if (false == pCmdData->_bWps)
                    {
                        strWpaCommand = "SET_NETWORK " + strNetworkNum + " ssid \"" + pCmdData->_strSSID + "\"";
                        ret           = pWifi->sendRequest(strWpaCommand.s(), &strResponse, wpa_msg_cb);
                        CHECK_ERROR_CONTINUE("unable to send request to wpa supplicant", ret);

                        if (pCmdData->_strPassword.isEmpty())
                        {
                            strWpaCommand = "SET_NETWORK " + strNetworkNum + " key_mgmt NONE";
                            ret           = pWifi->sendRequest(strWpaCommand.s(), &strResponse, wpa_msg_cb);
                            CHECK_ERROR_CONTINUE("unable to send request to wpa supplicant", ret);
                        }
                        else
                        {
                            strWpaCommand = "SET_NETWORK " + strNetworkNum + " key_mgmt WPA-PSK";
                            ret           = pWifi->sendRequest(strWpaCommand.s(), &strResponse, wpa_msg_cb);
                            CHECK_ERROR_CONTINUE("unable to send request to wpa supplicant", ret);
                            strWpaCommand = "SET_NETWORK " + strNetworkNum + " psk \"" + pCmdData->_strPassword + "\"";
                            ret           = pWifi->sendRequest(strWpaCommand.s(), &strResponse, wpa_msg_cb);
                            CHECK_ERROR_CONTINUE("unable to send request to wpa supplicant", ret);
                        }

                        /* select currenet network for connection */
                        strWpaCommand = "SELECT_NETWORK " + strNetworkNum;
                    }
                    else
                    {
                        /* Wifi Protected Setup Push Button Configuration */
                        BDBG_WRN(("Wifi protected setup started"));
                        strWpaCommand = "WPS_PBC";
                    }
                }
            }
            break;

            case eNotify_NetworkWifiDisconnect:
            {
                CWifiCommand * pCmdData = (CWifiCommand *)pAction->getDataPtr();
                if (false == pCmdData->_bWps)
                {
                    strWpaCommand = "DISCONNECT";
                }
                else
                {
                    BDBG_WRN(("Wifi protected setup cancelled"));
                    strWpaCommand = "WPS_CANCEL";
                }
            }
            break;

            default:
                strWpaCommand = "INVALID";
                BDBG_WRN(("wifiWorkerThread:unhandled action:%s", notificationToString(pAction->getId()).s()));
                break;
            } /* switch */

            /* send requested command to wpa supplicant, save returned response,
             * and trigger and sync with the Atlas main loop (bwin) */
            {
                eRet            ret       = eRet_Ok;
                CWifiResponse * pResponse = NULL;
                MString         strResponse;

                if (strWpaCommand != "INVALID")
                {
                    /* send request to WPA Supplicant */
                    ret = pWifi->sendRequest(strWpaCommand.s(), &strResponse, wpa_msg_cb);
                    CHECK_ERROR_CONTINUE("unable to send request to wpa supplicant", ret);
                }

                pResponse = new CWifiResponse(pAction, strResponse);
                CHECK_PTR_ERROR_CONTINUE("unable to allocate CWifiResponse", pResponse, ret, eRet_OutOfMemory);

                /* save response and sync with bwin main loop */
                ret = pWifi->trigger(pResponse);
                CHECK_ERROR_CONTINUE("unable to trigger bwin loop", ret);
            }
        }
    }
} /* wifiWorkerThread */

eRet CWifi::start()
{
    eRet             ret = eRet_Ok;
    B_ThreadSettings settings;

    if (true == getStartState())
    {
        BDBG_WRN(("attempting to start wifi thread when already started - ignored."));
        ret = eRet_InvalidState;
        goto error;
    }

    /* must set thread start state before starting thread or it will simply exit */
    setStartState(true);

    B_Thread_GetDefaultSettings(&settings);
    _threadWorker = B_Thread_Create("atlas_wifi", wifiWorkerThread, (void *)this, &settings);
    CHECK_PTR_ERROR_GOTO("unable to start wifi worker thread", _threadWorker, ret, eRet_NotAvailable, error);

    goto done;
error:
    setStartState(false);
done:
    return(ret);
} /* start */

void CWifi::stop()
{
    setStartState(false);
    if (NULL != _threadWorker)
    {
        B_Thread_Destroy(_threadWorker);
        _threadWorker = NULL;
    }

    dhcpStop();
}

eRet CWifi::startScanWifi()
{
    CAction * pAction = NULL;
    eRet      ret     = eRet_Ok;

    if (false == isScanEnabled())
    {
        /* scan is already disabled - do nothing */
        return(eRet_NotAvailable);
    }

    _bScanEnabled = false;

    /* create command */
    pAction = new CAction(eNotify_NetworkWifiScanStart);
    CHECK_PTR_ERROR_GOTO("Unable to malloc CAction", pAction, ret, eRet_OutOfMemory, error);

    /* queue command */
    addAction(pAction);

error:
    return(ret);
} /* startScanWifi */

eRet CWifi::stopScanWifi()
{
    CAction * pAction = NULL;
    eRet      ret     = eRet_Ok;

    /* create command */
    pAction = new CAction(eNotify_NetworkWifiScanStop);
    CHECK_PTR_ERROR_GOTO("Unable to malloc CAction", pAction, ret, eRet_OutOfMemory, error);

    /* queue command */
    addAction(pAction);

error:
    return(ret);
}

eRet CWifi::retrieveScanResults()
{
    CAction * pAction = NULL;
    eRet      ret     = eRet_Ok;

    /* create command */
    pAction = new CAction(eNotify_NetworkWifiScanResultRetrieve);
    CHECK_PTR_ERROR_GOTO("Unable to malloc CAction", pAction, ret, eRet_OutOfMemory, error);
    /* queue command */
    addAction(pAction);

    /* create command */
    pAction = new CAction(eNotify_NetworkWifiConnectedNetworkStatus);
    CHECK_PTR_ERROR_GOTO("Unable to malloc CAction", pAction, ret, eRet_OutOfMemory, error);
    /* queue command */
    addAction(pAction);

error:
    return(ret);
} /* retrieveScanResults */

eRet CWifi::requestStatus()
{
    CAction * pAction = NULL;
    eRet      ret     = eRet_Ok;

    /* create command */
    pAction = new CAction(eNotify_NetworkWifiConnectedNetworkStatus);
    CHECK_PTR_ERROR_GOTO("Unable to malloc CAction", pAction, ret, eRet_OutOfMemory, error);

    /* queue command */
    addAction(pAction);

error:
    return(ret);
}

eRet CWifi::connectWps()
{
    eRet ret = eRet_Ok;

    _bScanEnabled = false;

    CDataAction <CWifiCommand> * pAction  = NULL;
    CWifiCommand *               pCommand = NULL;

    CHECK_PTR_ERROR_GOTO("connect to wifi failed: not connected to WPA Supplicant", _pWpaControl, ret, eRet_InvalidState, error);

    /* reset connection error count before connecting */
    _nNetworkConnectErrors = 0;

    pCommand = new CWifiCommand();
    CHECK_PTR_ERROR_GOTO("unable to allocate wifi command", pCommand, ret, eRet_OutOfMemory, error);

    pCommand->_bWps = true;

    pAction = new CDataAction <CWifiCommand>(eNotify_NetworkWifiConnect, pCommand);
    CHECK_PTR_ERROR_GOTO("unable to allocate data action", pAction, ret, eRet_OutOfMemory, error);

    /* queue command */
    addAction(pAction);

error:
    return(ret);
} /* connectWps */

eRet CWifi::connectWifi(
        const char * strSSID,
        const char * strPassword
        )
{
    eRet ret = eRet_Ok;

    _bScanEnabled = false;

    CDataAction <CWifiCommand> * pAction  = NULL;
    CWifiCommand *               pCommand = NULL;

    CHECK_PTR_ERROR_GOTO("connect to wifi failed: not connected to WPA Supplicant", _pWpaControl, ret, eRet_InvalidState, error);

    /* reset connection error count before connecting */
    _nNetworkConnectErrors = 0;

    pCommand = new CWifiCommand();
    CHECK_PTR_ERROR_GOTO("unable to allocate wifi command", pCommand, ret, eRet_OutOfMemory, error);

    pCommand->_strSSID     = strSSID;
    pCommand->_strPassword = strPassword;

    pAction = new CDataAction <CWifiCommand>(eNotify_NetworkWifiConnect, pCommand);
    CHECK_PTR_ERROR_GOTO("unable to allocate data action", pAction, ret, eRet_OutOfMemory, error);

    /* queue command */
    addAction(pAction);

error:
    return(ret);
} /* connectWifi */

eRet CWifi::disconnectWps()
{
    eRet ret = eRet_Ok;

    CDataAction <CWifiCommand> * pAction  = NULL;
    CWifiCommand *               pCommand = NULL;

    CHECK_PTR_ERROR_GOTO("connect to wifi failed: not connected to WPA Supplicant", _pWpaControl, ret, eRet_InvalidState, error);

    pCommand = new CWifiCommand();
    CHECK_PTR_ERROR_GOTO("unable to allocate wifi command", pCommand, ret, eRet_OutOfMemory, error);

    pCommand->_bWps = true;

    pAction = new CDataAction <CWifiCommand>(eNotify_NetworkWifiDisconnect, pCommand);
    CHECK_PTR_ERROR_GOTO("unable to allocate data action", pAction, ret, eRet_OutOfMemory, error);

    /* queue command */
    addAction(pAction);

error:
    return(ret);
} /* disconnectWifi */

eRet CWifi::disconnectWifi()
{
    eRet ret = eRet_Ok;

    CDataAction <CWifiCommand> * pAction  = NULL;
    CWifiCommand *               pCommand = NULL;

    CHECK_PTR_ERROR_GOTO("connect to wifi failed: not connected to WPA Supplicant", _pWpaControl, ret, eRet_InvalidState, error);

    pCommand = new CWifiCommand();
    CHECK_PTR_ERROR_GOTO("unable to allocate wifi command", pCommand, ret, eRet_OutOfMemory, error);

    pAction = new CDataAction <CWifiCommand>(eNotify_NetworkWifiDisconnect, pCommand);
    CHECK_PTR_ERROR_GOTO("unable to allocate data action", pAction, ret, eRet_OutOfMemory, error);

    /* queue command */
    addAction(pAction);

error:
    return(ret);
} /* disconnectWifi */

eRet CWifi::updateNetworks()
{
    CAction * pAction = NULL;
    eRet      ret     = eRet_Ok;

    /* create command */
    pAction = new CAction(eNotify_NetworkWifiList);
    CHECK_PTR_ERROR_GOTO("Unable to malloc CAction", pAction, ret, eRet_OutOfMemory, error);

    /* queue command */
    addAction(pAction);

error:
    return(ret);
} /* updateNetworks */

CNetworkWifi * CWifi::getScannedNetwork(int index)
{
    CScopedMutex scopedMutex(_scannedNetworksListMutex);

    return(_scannedNetworksList[index]);
}

void CWifi::getConnectedStatus(MStringHash * pStringHash)
{
    BDBG_ASSERT(NULL != pStringHash);

    CScopedMutex scopedMutex(_connectedStatusMutex);

    for (_connectedStatusHash.first(); _connectedStatusHash.current(); _connectedStatusHash.next())
    {
        pStringHash->add(_connectedStatusHash.name(), _connectedStatusHash.value());
    }
}

MString CWifi::getConnectedBSSID()
{
    CScopedMutex scopedMutex(_connectedStatusMutex);

    return(_connectedStatusHash.get("bssid"));
}

void CWifi::clearConnectedStatus()
{
    CScopedMutex scopedMutex(_connectedStatusMutex);

    _connectedStatusHash.clear();
}

void CWifi::addConnectedStatus(
        const char * strToken,
        const char * strValue
        )
{
    CScopedMutex scopedMutex(_connectedStatusMutex);

    _connectedStatusHash.add(strToken, strValue);
}

void CWifi::notifyConnectedState()
{
    eRet ret = eRet_Ok;

    ret = notifyObservers(eNotify_NetworkWifiConnectState, (void *)this);
    CHECK_ERROR("error notifying observers", ret);
}

eRet CWifi::updateNetworksParse(MString * pStrResponse)
{
    eRet ret          = eRet_Ok;
    int  indexCurrent = 0;
    int  indexDelim   = 0;

    CScopedMutex scopedMutex(_parseMutex);
    CScopedMutex scopedMutex2(_networksListMutex);

    BDBG_MSG(("%s", __FUNCTION__));
    CHECK_PTR_ERROR_GOTO("update networks failed: not connected to WPA Supplicant", _pWpaControl, ret, eRet_InvalidState, error);

    _networksList.clear();

    /* example output:
     *  network id / ssid / bssid / flags
     *  0       FFTOOLBOX-5G    any     [DISABLED]
     */

    /* skip heading */
    indexCurrent = pStrResponse->find("/ flags") + strlen("/ flags") + 1;

    if (NULL == pStrResponse->mid(indexCurrent))
    {
        BDBG_WRN(("No networks found"));
        goto done;
    }

    /* check for multiple networks in pStrResponse */
    while (-1 != (indexDelim = pStrResponse->find('\t', indexCurrent)))
    {
        CNetworkWifi * pNetwork = NULL;

        pNetwork = new CNetworkWifi();
        CHECK_PTR_ERROR_GOTO("unable to allocate CNetworkWifi", pNetwork, ret, eRet_OutOfMemory, error);

        /* parse network id */
        pNetwork->_networkId = pStrResponse->mid(indexCurrent, indexDelim - indexCurrent).toInt();

        /* parse SSID */
        indexCurrent       = indexDelim + 1;
        indexDelim         = pStrResponse->find('\t', indexDelim + 1);
        pNetwork->_strSSID = pStrResponse->mid(indexCurrent, indexDelim - indexCurrent);

        /* skip to end of network info */
        indexDelim   = pStrResponse->find('\t', indexDelim + 1);
        indexCurrent = pStrResponse->find('\t', indexDelim + 1);

        _networksList.add(pNetwork);
    }

error:
done:
    /* printout networks for debugging purposes */
    {
        CNetworkWifi * pNetwork = NULL;
        for (pNetwork = _networksList.first(); pNetwork; pNetwork = _networksList.next())
        {
            BDBG_MSG(("List Network %d:%s", pNetwork->_networkId, pNetwork->_strSSID.s()));
        }
    }
    return(ret);
} /* updateNetworksParse */

eRet CWifi::scanResultsParse(MString * pStrResponse)
{
    eRet ret          = eRet_Ok;
    int  indexCurrent = 0;
    int  indexDelim   = 0;

    CScopedMutex scopedMutex(_parseMutex);
    CScopedMutex scopedMutex2(_scannedNetworksListMutex);

    if (true == pStrResponse->isEmpty())
    {
        BDBG_ERR(("invalid wifi scan results (empty)"));
        ret = eRet_NotAvailable;
        goto error;
    }

    _scannedNetworksList.clear();

    /* example output:
     * bssid / frequency / signal level / flags / ssid
     * b8:62:1f:e5:dd:5b       5200    -55     [WPA2-EAP-CCMP][ESS]    BCLMT-Wifi
     */

    /* skip heading */
    indexCurrent = pStrResponse->find("/ ssid") + strlen("/ ssid") + 1;

    if (NULL == pStrResponse->mid(indexCurrent))
    {
        BDBG_WRN(("No networks found during scan"));
        goto done;
    }

    /* check for multiple networks in pStrResponse */
    while (-1 != (indexDelim = pStrResponse->find('\t', indexCurrent)))
    {
        CNetworkWifi * pNetwork = NULL;
        MString        strSignal;

        pNetwork = new CNetworkWifi();
        CHECK_PTR_ERROR_GOTO("unable to allocate CNetworkWifi", pNetwork, ret, eRet_OutOfMemory, error);

        /* parse BSSID */
        pNetwork->_strBSSID = pStrResponse->mid(indexCurrent, indexDelim - indexCurrent);

        /* parse frequency */
        indexCurrent            = indexDelim + 1;
        indexDelim              = pStrResponse->find('\t', indexDelim + 1);
        pNetwork->_strFrequency = pStrResponse->mid(indexCurrent, indexDelim - indexCurrent);

        /* parse signal level */
        indexCurrent             = indexDelim + 1;
        indexDelim               = pStrResponse->find('\t', indexDelim + 1);
        strSignal                = pStrResponse->mid(indexCurrent, indexDelim - indexCurrent);
        pNetwork->_signalLevel   = strSignal.toInt();
        pNetwork->_signalToNoise = _noiseLevel - strSignal.toInt();

        /* parse flags */
        indexCurrent        = indexDelim + 1;
        indexDelim          = pStrResponse->find('\t', indexDelim + 1);
        pNetwork->_strFlags = pStrResponse->mid(indexCurrent, indexDelim - indexCurrent);

        /* parse SSID */
        indexCurrent       = indexDelim + 1;
        indexDelim         = pStrResponse->find('\n', indexDelim + 1);
        pNetwork->_strSSID = pStrResponse->mid(indexCurrent, indexDelim - indexCurrent);

        _scannedNetworksList.add(pNetwork);

        /* skip to end of line */
        indexCurrent = indexDelim + 1;

        BDBG_WRN(("%s %s %d %s %s", pNetwork->_strFrequency.s(), pNetwork->_strFrequency.s(), pNetwork->_signalToNoise, pNetwork->_strFlags.s(), pNetwork->_strSSID.s()));
    }

error:
done:
    return(ret);
} /* scanResultsParse */

eRet CWifi::connectedNetworkParse(MString * pStrResponse)
{
    eRet    ret          = eRet_Ok;
    int     indexCurrent = 0;
    int     indexDelim   = 0;
    MString strToken;
    MString strValue;

    CScopedMutex scopedMutex(_parseMutex);

    clearConnectedStatus();

    if (true == pStrResponse->isEmpty())
    {
        BDBG_ERR(("invalid connected network results (empty)"));
        ret = eRet_NotAvailable;
        goto error;
    }

    /* example output:
     *  bssid=08:bd:43:d5:ca:ff
     *  freq=5220
     *  ssid=FFTOOLBOX-5G
     *  id=0
     *  mode=station
     *  pairwise_cipher=CCMP
     *  group_cipher=CCMP
     *  key_mgmt=WPA2-PSK
     *  wpa_state=COMPLETED
     *  address=00:90:4c:1b:00:01
     *  uuid=9748206c-23ca-5797-ad20-e37060118fa3
     */

    while (-1 != (indexDelim = pStrResponse->find('=', indexCurrent)))
    {
        /* parse token */
        strToken = pStrResponse->mid(indexCurrent, indexDelim - indexCurrent);

        /* parse value */
        indexCurrent = indexDelim + 1;
        indexDelim   = pStrResponse->find("\n", indexDelim + 1);
        strValue     = pStrResponse->mid(indexCurrent, indexDelim - indexCurrent);

        /* skip to end of line */
        indexCurrent = indexDelim + 1;

        addConnectedStatus(strToken, strValue);

        if (strToken == "wpa_state")
        {
            setConnectedState(strValue);
        }
    }

error:
    return(ret);
} /* connectedNetworkParse */

eRet CWifi::disconnectedNetworkParse(MString * pStrResponse)
{
    eRet    ret          = eRet_Ok;
    int     indexCurrent = 0;
    int     indexDelim   = 0;
    MString strBSSID;

    CScopedMutex scopedMutex(_parseMutex);

    if (true == pStrResponse->isEmpty())
    {
        BDBG_ERR(("invalid disconnected network results (empty) - mark all as disconnected"));
        ret = eRet_NotAvailable;
        goto error;
    }

    /* example output:
     *  bssid=08:bd:43:d5:ca:ff reason=3 locally_generated=1
     */
    indexCurrent = pStrResponse->find("bssid=", indexCurrent) + strlen("bssid=");
    indexDelim   = indexCurrent + strlen("xx:xx:xx:xx:xx:xx");

    BDBG_WRN(("disconnect response:%s", pStrResponse->s()));
    strBSSID = pStrResponse->mid(indexCurrent, indexDelim - indexCurrent);
    BDBG_MSG(("disconnecting from:%s", strBSSID.s()));

    setConnectedState(eConnectedState_Disconnected);
error:
    clearConnectedStatus();
    return(ret);
} /* disconnectedNetworkParse */

eRet CWifi::bssNoiseLevelParse(MString * pStrResponse)
{
    eRet ret          = eRet_Ok;
    int  indexCurrent = 0;
    int  indexDelim   = 0;

    CScopedMutex scopedMutex(_parseMutex);

    if (true == pStrResponse->isEmpty())
    {
        BDBG_ERR(("invalid bss noise level results (empty) - set noise level to 0"));
        _noiseLevel = 0;
        ret         = eRet_NotAvailable;
        goto error;
    }

    /* example output:
     * id=14047
     * bssid=00:19:a9:a7:a9:ce
     * freq=5765
     * beacon_int=100
     * capabilities=0x0011
     * qual=0
     * noise=-92
     * level=-80
     * tsf=0000684791920116
     * age=13
     * ie=00076272636d77706101088c129824b048606c071255532024041134041864051884031895041e0b050a00068d5b30140100000fac040100000fac040100000fac012800851e03008f000f00ff0359005741502d5344474f2d412d332d3500000a0000269606004096001100dd180050f2020101800003a4000027a4000042435e0062322f00dd06004096010104dd050040960305dd050040960b09dd050040961401
     * flags=[WPA2-EAP-CCMP][ESS]
     * ssid=brcmwpa
     * snr=12
     * est_throughput=36000
     */
    indexCurrent = pStrResponse->find("noise=") + strlen("noise=");
    indexDelim   = pStrResponse->find('\n', indexCurrent);
    _noiseLevel  = pStrResponse->mid(indexCurrent, indexDelim - indexCurrent).toInt();

error:
    return(ret);
} /* bssNoiseLevelParse */

eRet CWifi::sendRequest(
        MString          strCommand,
        MString *        pStrResponse,
        CWifiWpaCallback replyCallback
        )
{
    eRet    ret    = eRet_Ok;
    int     retWpa = 0;
    char    bufCommand[255];
    char    bufReply[4096];
    size_t  lenCommand = sizeof(bufReply);
    size_t  lenReply   = sizeof(bufReply);
    MString strResponseDummy;

    memset(bufCommand, 0, sizeof(bufCommand));
    memset(bufReply, 0, sizeof(bufReply));

    strncpy(bufCommand, strCommand, sizeof(bufCommand)-1);
    lenCommand = strlen(bufCommand);

    pStrResponse->clear();
    BDBG_ASSERT(strCommand.length() > 0);

    BDBG_MSG(("wpa_ctrl_request():%s %lu", bufCommand, lenCommand));
    retWpa = wpa_ctrl_request(_pWpaControl, bufCommand, lenCommand, bufReply, &lenReply, replyCallback);
    CHECK_WIFI_ERROR_GOTO("unable to send request to WPA Supplicant", ret, retWpa, error);

    if (0 < lenReply)
    {
        BDBG_MSG(("wpa_ctrl_request() bufReply:%s\n", bufReply));
        *pStrResponse = bufReply;
    }

error:
    return(ret);
} /* sendRequest */

eRet CWifi::staticIpStart()
{
    MString strCommand;
    eRet ret          = eRet_Ok;
    int32_t retSystem = 0;

    if (false == _strInterfaceName.isEmpty())
    {
        strCommand = "ifconfig " + _strInterfaceName + " ";
        strCommand += MString(GET_STR(_pCfg, WPA_SUPPLICANT_STATIC_IP)) + " ";
        strCommand += GET_STR(_pCfg, WPA_SUPPLICANT_STATIC_NETMASK);

        BDBG_WRN(("%s:%s", __FUNCTION__, strCommand.s()));
        retSystem = system(strCommand.s());
        if (-1 == retSystem)
        {
            ret = eRet_ExternalError;
        }
    }

    return(ret);
} /* staticIpStart */

eRet CWifi::staticIpStop()
{
    MString strCommand = "ifconfig 0.0.0.0";
    eRet ret          = eRet_Ok;
    int32_t retSystem = 0;

    BDBG_WRN(("%s:%s", __FUNCTION__, strCommand.s()));
    retSystem = system(strCommand.s());
    if (-1 == retSystem)
    {
        ret = eRet_ExternalError;
    }

    return(ret);
}

#include <netinet/in.h>
#include <arpa/nameser.h>
#include <resolv.h>
eRet CWifi::dhcpStart()
{
    eRet    ret = eRet_Ok;
    MString strCommand;
    MString strUdhcpcProcessIdFilename = "/tmp/udhcpc." + _strInterfaceName + ".pid";
    int32_t retSystem                  = 0;

    dhcpStop();

    strCommand  = "udhcpc -i " + _strInterfaceName + " -p" + strUdhcpcProcessIdFilename;
    strCommand += " -s udhcpc.script &";

    BDBG_WRN(("%s:%s", __FUNCTION__, strCommand.s()));
    retSystem = system(strCommand.s());
    if (-1 == retSystem)
    {
        ret = eRet_ExternalError;
    }

    return(ret);
} /* dhcpStart */

#include <sys/types.h>
#include <sys/wait.h>
eRet CWifi::dhcpStop()
{
    eRet    ret = eRet_Ok;
    MString strCommand;
    MString strUdhcpcProcessIdFilename = "/tmp/udhcpc." + _strInterfaceName + ".pid";
    FILE *  pFD                        = NULL;
    char    strProcessID[16];
    char *  pStr        = NULL;
    int32_t nProcessID  = 0;
    int32_t nWaitStatus = 0;

    pFD = fopen(strUdhcpcProcessIdFilename, "r");
    if (NULL == pFD)
    {
        BDBG_MSG(("%s:no udhcpc process running for wlan0", __FUNCTION__));
        goto done;
    }

    pStr = fgets(strProcessID, sizeof(strProcessID), pFD);
    CHECK_PTR_ERROR_GOTO("unable to read UDHCPC process ID", pStr, ret, eRet_NotAvailable, error);

    nProcessID = MString(strProcessID).toInt();

    BDBG_WRN(("%s:kill %s", __FUNCTION__, strProcessID));
    kill(nProcessID, SIGTERM);
error:
done:
    if (NULL != pFD)
    {
        fclose(pFD);
        pFD = NULL;
    }

    return(ret);
} /* dhcpStop */

/* acquire new ip address using either dhcp or static ip from atlas.cfg */
eRet CWifi::ipAddressAcquisitionStart()
{
    eRet ret = eRet_Ok;

    ret = (MString("0.0.0.0") == GET_STR(_pCfg, WPA_SUPPLICANT_STATIC_IP)) ? dhcpStart() : staticIpStart();
    return(ret);
}

eRet CWifi::ipAddressAcquisitionStop()
{
    eRet ret = eRet_Ok;

    ret = (MString("0.0.0.0") == GET_STR(_pCfg, WPA_SUPPLICANT_STATIC_IP)) ? dhcpStop() : staticIpStop();
    return(ret);
}

eConnectedState CWifi::getConnectedState()
{
    return(_connectedState);
}

void CWifi::setConnectedState(eConnectedState connectedState)
{
    CScopedMutex scopedMutex(_connectedStateMutex);

    _connectedState = connectedState;
}

void CWifi::setConnectedState(const char * strConnectedState)
{
    CScopedMutex scopedMutex(_connectedStateMutex);

    _connectedState = stringToConnectedState(strConnectedState);

    if (eConnectedState_Disconnected == getConnectedState())
    {
        clearConnectedStatus();
    }
}

void CWifi::addResponse(CWifiResponse * pResponse)
{
    CScopedMutex scopedMutex(_responseMutex);

    _responseList.add(pResponse);
}

CWifiResponse * CWifi::getResponse()
{
    CWifiResponse * pResponse = NULL;
    CScopedMutex    scopedMutex(_responseMutex);

    if (0 < _responseList.total())
    {
        pResponse = (CWifiResponse *)_responseList.first();
    }

    return(pResponse);
}

CWifiResponse * CWifi::removeResponse()
{
    CWifiResponse * pResponse = NULL;
    CScopedMutex    scopedMutex(_responseMutex);

    if (0 < _responseList.total())
    {
        pResponse = (CWifiResponse *)_responseList.remove(0);
    }

    return(pResponse);
}

void CWifi::addAction(CAction * pAction)
{
    CScopedMutex scopedMutex(_actionMutex);

    _actionList.add(pAction);
    B_Event_Set(_actionEvent);
}

CAction * CWifi::getAction()
{
    CAction *    pAction = NULL;
    CScopedMutex scopedMutex(_actionMutex);

    if (0 < _actionList.total())
    {
        pAction = (CAction *)_actionList.first();
    }

    return(pAction);
}

CAction * CWifi::removeAction()
{
    CAction *    pAction = NULL;
    CScopedMutex scopedMutex(_actionMutex);

    if (0 < _actionList.total())
    {
        pAction = (CAction *)_actionList.remove(0);
    }

    return(pAction);
}