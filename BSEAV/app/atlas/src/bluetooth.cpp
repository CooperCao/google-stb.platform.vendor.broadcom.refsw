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
#include "bluetooth.h"
#include "atlas.h"
#include "atlas_cfg.h"
#include "convert.h"
#include "nexus_platform_standby.h"
/*#include "LinuxInputSource.h"*/
#include <vector>

#define CALLBACK_NETAPP_BTDISCOVERYDONE   "callbackNetAppBtDiscoveryDone"
#define CALLBACK_NETAPP_BTCONNECTDONE     "callbackNetAppBtConnectDone"
#define CALLBACK_NETAPP_BTDISCONNECTDONE  "callbackNetAppBtDisconnectDone"

BDBG_MODULE(atlas_bluetooth);

#ifdef NETAPP_SUPPORT
/*typedef std::vector<LinuxInputSource*>InputSourceList;
 * static InputSourceList inputSourceList; */

static void print_debug_bt_dev_list(
        NETAPP_BT_DEV_INFO * pDevInfoList,
        int                  size
        )
{
    int i;

    if ((size == 0) || (pDevInfoList == NULL))
    {
        BDBG_MSG(("List is empty\n"));
        return;
    }
    for (i = 0; i < size; i++)
    {
        BDBG_MSG(("\n%d:**************************\n", i));
        BDBG_MSG(("Name:       %s\n", pDevInfoList[i].cName));
        BDBG_MSG(("BD Addr:    %s\n", pDevInfoList[i].cAddr));
        BDBG_MSG(("RSSI:       %d dB\n", pDevInfoList[i].lRssi));
        BDBG_MSG(("Services:   [%s%s%s%s%s%s%s%s%s]0x%04x\n",
                  (pDevInfoList[i].ulServiceMask & NETAPP_BT_SERVICE_HID)   ? "HID " : "",
                  (pDevInfoList[i].ulServiceMask & NETAPP_BT_SERVICE_HSP)   ? "HSP " : "",
                  (pDevInfoList[i].ulServiceMask & NETAPP_BT_SERVICE_HFP)   ? "HFP " : "",
                  (pDevInfoList[i].ulServiceMask & NETAPP_BT_SERVICE_OPP)   ? "OPP " : "",
                  (pDevInfoList[i].ulServiceMask & NETAPP_BT_SERVICE_FTP)   ? "FTP " : "",
                  (pDevInfoList[i].ulServiceMask & NETAPP_BT_SERVICE_A2DP_SOURCE)  ? "A2DP-Source " : "",
                  (pDevInfoList[i].ulServiceMask & NETAPP_BT_SERVICE_A2DP_SINK)  ? "A2DP-Sink " : "",
                  (pDevInfoList[i].ulServiceMask & NETAPP_BT_SERVICE_AVRCP) ? "AVRCP " : "",
                  (pDevInfoList[i].ulServiceMask & NETAPP_BT_SERVICE_BLE) ? "HOGP" : "",
                  pDevInfoList[i].ulServiceMask));
        BDBG_MSG(("COD Major:  0x%02x\n", pDevInfoList[i].ucMajorClassDev));
        BDBG_MSG(("COD Minor:  0x%02x\n", pDevInfoList[i].ucMinorClassDev));
        BDBG_MSG(("COD Service:0x%04x\n", pDevInfoList[i].usServiceClassDev));
        BDBG_MSG(("%d:**************************\n", i));
    }
} /* print_debug_bt_dev_list */

CBluetoothDevice::CBluetoothDevice(
        uint32_t             index,
        NETAPP_BT_DEV_INFO * pBtDevInfoList,
        CBluetooth *         pBluetooth
        ) :
    _connectStatus(eBtConnectStatus_Unknown)
{
    _index          = index;
    _pBtDevInfoList = pBtDevInfoList;
    _pBluetooth     = pBluetooth;
}

CBluetoothDevice::~CBluetoothDevice()
{
}

void CBluetoothDevice::update(
        uint32_t             index,
        NETAPP_BT_DEV_INFO * pBtDevInfoList,
        CBluetooth *         pBluetooth
        )
{
    _index          = index;
    _pBtDevInfoList = pBtDevInfoList;
    _pBluetooth     = pBluetooth;
}

char * CBluetoothDevice::getName()
{
    return(_pBtDevInfoList[_index].cName);
}

char * CBluetoothDevice::getAddress()
{
    return(_pBtDevInfoList[_index].cAddr);
}

bool CBluetoothDevice::isConnected()
{
    eRet                 ret                = eRet_Ok;
    NETAPP_BT_DEV_INFO * pBtDevInfoConnList = NULL;
    uint32_t             connListCount      = 0;

#if 1
    _pBluetooth->returnConnectedBtList(&connListCount, &pBtDevInfoConnList);
    CHECK_ERROR_GOTO("unable to get connected list", ret, error);
#endif
    if ((connListCount > 0) && _pBtDevInfoList && pBtDevInfoConnList)
    {
        for (uint32_t i = 0; i < connListCount; i++)
        {
            if ((strncmp(_pBtDevInfoList[_index].cName, pBtDevInfoConnList[i].cName, sizeof(pBtDevInfoConnList[i].cName)) == 0) &&
                (strncmp(_pBtDevInfoList[_index].cAddr, pBtDevInfoConnList[i].cAddr, sizeof(pBtDevInfoConnList[i].cAddr)) == 0)
                )
            { return(true); }
        }
    }

error:
    return(false);
} /* isConnected */

bool CBluetoothDevice::isPaired()
{
    NETAPP_BT_DEV_INFO * pBtDevInfoSavedList;
    uint32_t             savedListCount;

    _pBluetooth->returnSavedBtListInfo(&savedListCount, &pBtDevInfoSavedList);

    /* Compare the pointers of  the device's list is equal to the saved list */
    return(pBtDevInfoSavedList == _pBtDevInfoList);
}

int CBluetoothDevice::getSignalStrength()
{
    eRet                 ret                = eRet_Ok;
    NETAPP_BT_DEV_INFO * pBtDevInfoConnList = NULL;
    uint32_t             connListCount      = 0;

    if (isConnected()) /* connected */
    {
        _pBluetooth->returnConnectedBtList(&connListCount, &pBtDevInfoConnList);
        CHECK_ERROR_GOTO("unable to get connected list", ret, error);
        if ((connListCount > 0) && _pBtDevInfoList && pBtDevInfoConnList)
        {
            for (uint32_t i = 0; i < connListCount; i++)
            {
                if ((strncmp(_pBtDevInfoList[_index].cName, pBtDevInfoConnList[i].cName, sizeof(pBtDevInfoConnList[i].cName)) == 0) &&
                    (strncmp(_pBtDevInfoList[_index].cAddr, pBtDevInfoConnList[i].cAddr, sizeof(pBtDevInfoConnList[i].cAddr)) == 0)
                    )
                { return(pBtDevInfoConnList[i].lRssi); }
            }
        }
    }

    /*either in discovery or saved( saved doesn't return a signal strength*/
error:
    return(_pBtDevInfoList[_index].lRssi);
} /* getSignalStrength */

eRet CBluetoothDevice::getStatusList(
        MStringHash * pStatus,
        bool          bClear
        )
{
    char str[32];
    eRet ret = eRet_Ok;

    if (true == bClear)
    {
        pStatus->clear();
    }

    if (_pBtDevInfoList)
    {
        snprintf(str, 32, "%s", getAddress());
        pStatus->add("BDADDR", str);
        snprintf(str, 32, "%s", getName());
        pStatus->add("BDNAME", str);
        /*snprintf(str, 32, "%d dB",  _pBtDevInfoList[_index].lRssi); */
        snprintf(str, 32, "%d dB", getSignalStrength());
        pStatus->add("Signal", str);
        snprintf(str, 32, "[%s%s%s%s%s%s%s%s%s]",
                (_pBtDevInfoList[_index].ulServiceMask & NETAPP_BT_SERVICE_HID)   ? "HID " : "",
                (_pBtDevInfoList[_index].ulServiceMask & NETAPP_BT_SERVICE_HSP)   ? "HSP " : "",
                (_pBtDevInfoList[_index].ulServiceMask & NETAPP_BT_SERVICE_HFP)   ? "HFP " : "",
                (_pBtDevInfoList[_index].ulServiceMask & NETAPP_BT_SERVICE_OPP)   ? "OPP " : "",
                (_pBtDevInfoList[_index].ulServiceMask & NETAPP_BT_SERVICE_FTP)   ? "FTP " : "",
                (_pBtDevInfoList[_index].ulServiceMask & NETAPP_BT_SERVICE_A2DP_SOURCE)  ? "A2DP-Source " : "",
                (_pBtDevInfoList[_index].ulServiceMask & NETAPP_BT_SERVICE_A2DP_SINK)  ? "A2DP-Sink " : "",
                (_pBtDevInfoList[_index].ulServiceMask & NETAPP_BT_SERVICE_AVRCP) ? "AVRCP " : "",
                (_pBtDevInfoList[_index].ulServiceMask & NETAPP_BT_SERVICE_BLE) ? "HOGP" : "");
        pStatus->add("Service", str);
        snprintf(str, 32, "0x%02x", _pBtDevInfoList[_index].ucMajorClassDev);
        pStatus->add("COD Major", str);
        snprintf(str, 32, "0x%02x", _pBtDevInfoList[_index].ucMinorClassDev);
        pStatus->add("COD Minor", str);
        snprintf(str, 32, "0x%04x", _pBtDevInfoList[_index].usServiceClassDev);
        pStatus->add("COD Service", str);
    }
    else
    {
        BDBG_WRN(("Can't get Status: BtDevInfoList is NULL"));
        ret = eRet_NotAvailable;
    }

    return(ret);
} /* getStatusList */

static void bwinNetAppBtDiscoveryDoneCallback(
        void *       pObject,
        const char * strCallback
        )
{
    CBluetooth * pBluetooth = (CBluetooth *)pObject;

    BSTD_UNUSED(strCallback);

    pBluetooth->btDiscoveryDoneCallback();
}

static void bwinNetAppBtConnectDoneCallback(
        void *       pObject,
        const char * strCallback
        )
{
    CBluetooth * pBluetooth = (CBluetooth *)pObject;

    BSTD_UNUSED(strCallback);

    pBluetooth->btConnectDoneCallback();
}

static void bwinNetAppBtDisconnectDoneCallback(
        void *       pObject,
        const char * strCallback
        )
{
    CBluetooth * pBluetooth = (CBluetooth *)pObject;

    BSTD_UNUSED(strCallback);

    pBluetooth->btDisconnectDoneCallback();
}

void CBluetooth::btDiscoveryDoneCallback()
{
    if (eRet_Ok == getDiscoveryStatus())
    {
        /* Getting a new updated BT list */
        FRE(_pBtDevInfoDiscList);

        /* retrieve results */
        NETAPP_RETCODE retNetApp = NETAPP_SUCCESS;
        eRet           unused    = eRet_Ok;
        retNetApp = NetAppBluetoothGetDiscoveryResults(_pNetApp, &_pBtDevInfoDiscList, &_discListCount);
        CHECK_NETAPP_ERROR_GOTO("unable to get DiscoveryResults", unused, retNetApp, error);

        BDBG_MSG(("%s() Received NETAPP_CB_BT_DISCOVERY_RESULTS, result=%d count=%d\n", __FUNCTION__, retNetApp, _discListCount));
        updateBluetoothDeviceList();
        /*dumpDeviceList(); */
    }
    else
    {
        BDBG_WRN(("Bluetooth Discovery failed!"));
    }
error:
    notifyObservers(eNotify_BluetoothDiscoveryResult, this); /* signifes  discovery is done and results are ready */
}                                                            /* btDiscoveryDoneCallback */

void CBluetooth::btConnectDoneCallback()
{
    if (eConnectStatus_Connected == getConnectStatus())
    {
        NETAPP_BT_DEV_INFO * info   = NULL;
        uint32_t             nCount = 0;
        BDBG_MSG((" Bluetooth btConnectDoneCallback !"));

        /* retrieve results */
        NETAPP_RETCODE retNetAppResults = NETAPP_SUCCESS;
        retNetAppResults = NetAppBluetoothGetConnectedDevices(_pNetApp, &info, &nCount);

        if (NETAPP_SUCCESS == retNetAppResults)
        {
            updateBluetoothDeviceList();
        }
    }
    else
    {
        BDBG_MSG(("Bluetooth connect failed!"));
    }

    notifyObservers(eNotify_BluetoothConnectionStatus, this);
} /* btConnectDoneCallback */

void CBluetooth::btDisconnectDoneCallback()
{
    if (eConnectStatus_Disconnected == getConnectStatus())
    {
        NETAPP_BT_DEV_INFO * info   = NULL;
        uint32_t             nCount = 0;
        BDBG_MSG((" Bluetooth btDisconnectDoneCallback !"));

        /* retrieve results */
        NETAPP_RETCODE retNetAppResults = NETAPP_SUCCESS;
        retNetAppResults = NetAppBluetoothGetConnectedDevices(_pNetApp, &info, &nCount);

        if (NETAPP_SUCCESS == retNetAppResults)
        {
            updateBluetoothDeviceList();
        }
    }
    else
    {
        BDBG_MSG(("Bluetooth disconnect failed!"));
    }

    notifyObservers(eNotify_BluetoothConnectionStatus, this);
} /* btConnectDoneCallback */

/*
 * Process the Bluetooth callbacks. Need to register these with bwin
 * void CBluetooth::bluetooth_callback(
 */
static void bluetooth_callback(
        void *         pParam,   /* !< The pParam from NetAppOpen */
        NETAPP_CB_TYPE tCbType,  /* !< The Callback type */
        const void *   pvBuffer, /* !< Pointer to data specific to the callback */
        uint32_t       ulData0,  /* !< Callback specific data 0 */
        NETAPP_RETCODE tResult,  /* !< Callback results from the */
        NETAPP_IFACE   tIFace
        ) /* !< Callback Info structure */
{
    CBluetooth *    pBluetooth    = (CBluetooth *)pParam;
    CWidgetEngine * pWidgetEngine = pBluetooth->getWidgetEngine();

    (void)ulData0;
    (void)tIFace;
    (void)pvBuffer;

    switch (tCbType)
    {
    case NETAPP_CB_CONNECT:
        BDBG_MSG(("%s() Received NETAPP_CB_CONNECT (%s) result=%d\n",
                  __FUNCTION__, (tIFace == NETAPP_IFACE_WIRELESS) ? "Wireless" : "Bluetooth", tResult));

        if (tIFace == NETAPP_IFACE_BLUETOOTH)
        {
            if (tResult == NETAPP_SUCCESS)
            {
                NETAPP_BT_DEV_INFO * pDevInfo = (NETAPP_BT_DEV_INFO *) pvBuffer;

                BDBG_MSG(("\n***CONNECTED BT DEVICE******\n"));
                BDBG_MSG(("Name:       %s\n", pDevInfo->cName));
                BDBG_MSG(("HD Addr:    %s\n", pDevInfo->cAddr));
                BDBG_MSG(("RSSI:       %d dB\n", pDevInfo->lRssi));
                BDBG_MSG(("****************************\n"));
            }

            /* sync with bwin loop */
            if (NULL != pWidgetEngine)
            {
                pBluetooth->setConnectStatus(eConnectStatus_Connected);
                pWidgetEngine->syncCallback(pBluetooth, CALLBACK_NETAPP_BTCONNECTDONE);
            }
        }
        break;

    case NETAPP_CB_DISCONNECT:
        BDBG_MSG(("%s() Received NETAPP_CB_DISCONNECT (%s) result=%d\n",
                  __FUNCTION__, (tIFace == NETAPP_IFACE_WIRELESS) ? "Wireless" : "Bluetooth", tResult));

        if (tIFace == NETAPP_IFACE_BLUETOOTH)
        {
            /* sync with bwin loop */
            if (NULL != pWidgetEngine)
            {
                pBluetooth->setConnectStatus(eConnectStatus_Disconnected);
                pWidgetEngine->syncCallback(pBluetooth, CALLBACK_NETAPP_BTDISCONNECTDONE);
            }
        }
        break;

    case NETAPP_CB_BT_DISCOVERY_RESULTS:
    {
        /* sync with bwin loop */
        if (NULL != pWidgetEngine)
        {
            pBluetooth->setDiscoveryStatus(NETAPP_SUCCESS == tResult ? eRet_Ok : eRet_ExternalError);
            pWidgetEngine->syncCallback(pBluetooth, CALLBACK_NETAPP_BTDISCOVERYDONE);
        }

        break;
    }

    case NETAPP_CB_HOTPLUG:
    {
        NETAPP_HOTPLUG_DEVICE_INFO * info = (NETAPP_HOTPLUG_DEVICE_INFO *)pvBuffer;
        BDBG_MSG(("%s() Received NETAPP_CB_HOTPLUG, result=%d\n", __FUNCTION__, tResult));

        if (info->tType == NETAPP_HOTPLUG_DEVICE_USB_INPUT)
        {
            if (info->tAction == NETAPP_HOTPLUG_ADD)
            {
                pBluetooth->_pBluetoothRemote->addInputSource(info->cSysName);
            }
            else
            {
                /* Got a hotplug remove  */
                pBluetooth->_pBluetoothRemote->removeInputSource(info->cSysName);
            }
        }

        break;
    }

    case NETAPP_CB_BT_AVK_STATE:
        BDBG_MSG(("%s() Received NETAPP_CB_BT_AVK_STATE result=%d\n", __FUNCTION__, tResult));

#if 0
        if (tResult == NETAPP_SUCCESS)
        {
            switch (ulData0)
            {
            case NETAPP_BT_AVK_STATE_PLAY:
                NexusBtAvkSink_Start(&hBtAvkSink, hNexus, (NETAPP_BT_AUDIO_FORMAT *)pvBuffer);
                break;

            case NETAPP_BT_AVK_STATE_STOP:
                NexusBtAvkSink_Stop(hBtAvkSink);
                hBtAvkSink = NULL;
                break;
            default:
                break;
            } /* switch */
        }
#endif /* if 0 */
        break;

    case NETAPP_CB_BT_BATTERY_LEVEL:
        BDBG_MSG(("%s() Received NETAPP_CB_BT_BATTERY_LEVEL result=%d\n", __FUNCTION__, tResult));
#if 0
        if (tResult == NETAPP_SUCCESS)
        {
            BDBG_MSG(("%s() Bluetooth remote battery level is:%d\n", __FUNCTION__, ulData0));
        }
#endif /* if 0 */
        break;

    default:
        break;
    } /* switch */
}     /* bluetooth_callback */

CBluetooth::CBluetooth(
        const char *     name,
        const uint16_t   number,
        CConfiguration * pCfg
        ) :
    CAudioCaptureClient(name, number, eBoardResource_bluetooth, pCfg),
    _pBluetoothRemote(NULL),
    _pWidgetEngine(NULL),
    _discoveryStatus(eRet_Ok),
    _connectStatus(eConnectStatus_Unknown),
    _bDiscoveryStarted(false),
#ifdef NETAPP_SUPPORT
    _pNetApp(NULL),
    _pBtDevInfoDiscList(NULL),
    _discListCount(0),
    _pBtDevInfoSavedList(NULL),
    _savedListCount(0),
    _pBtDevInfoConnList(NULL),
    _connListCount(0)
#endif /* ifdef NETAPP_SUPPORT */
{
    _tBtAudioFormat.tMode           = NETAPP_BT_AV_MODE_STEREO;
    _tBtAudioFormat.ulSampleRate    = 44100;
    _tBtAudioFormat.ucBitsPerSample = 16;
}

CBluetooth::~CBluetooth()
{
    close();
}

/* TODO: who gets to call the NetappOpen. Not all chips have wifi and not all have bt */
eRet CBluetooth::open(CWidgetEngine * pWidgetEngine)
{
    eRet                 ret       = eRet_Ok;
    NETAPP_RETCODE       retNetApp = NETAPP_SUCCESS;
    NETAPP_SETTINGS      settings;
    NETAPP_OPEN_SETTINGS settingsOpen;
    NETAPP_INIT_SETTINGS settingsInit;

    _pWidgetEngine = pWidgetEngine;

    if (NULL != _pWidgetEngine)
    {
        _pWidgetEngine->addCallback(this, CALLBACK_NETAPP_BTDISCOVERYDONE, bwinNetAppBtDiscoveryDoneCallback);
        _pWidgetEngine->addCallback(this, CALLBACK_NETAPP_BTCONNECTDONE, bwinNetAppBtConnectDoneCallback);
        _pWidgetEngine->addCallback(this, CALLBACK_NETAPP_BTDISCONNECTDONE, bwinNetAppBtDisconnectDoneCallback);
    }
#if 1
    retNetApp = NetAppGetDefaultSettings(&settings);
    CHECK_NETAPP_ERROR_GOTO("unable to get default settings", ret, retNetApp, error);

    retNetApp = NetAppGetDefaultInitSettings(&settingsInit);
    CHECK_NETAPP_ERROR_GOTO("unable to get default init settings", ret, retNetApp, error);

    settingsOpen.tCallback = bluetooth_callback;
    settingsOpen.pParam    = this;

    retNetApp = NetAppOpen(&_pNetApp, &settingsOpen, &settingsInit, &settings);
    CHECK_NETAPP_ERROR_GOTO("unable to open NetApp", ret, retNetApp, error);

    updateBluetoothDeviceList(); /* to get saved to show up before waiting for results */
#endif /* if 1 */

error:
    return(ret);
} /* open */

void CBluetooth::close()
{
    if (NULL != _pNetApp)
    {
        NetAppClose(_pNetApp);
        _pNetApp = NULL;
    }

    if (NULL != _pWidgetEngine)
    {
        _pWidgetEngine->removeCallback(this, CALLBACK_NETAPP_BTDISCOVERYDONE);
        _pWidgetEngine->removeCallback(this, CALLBACK_NETAPP_BTCONNECTDONE);
        _pWidgetEngine->removeCallback(this, CALLBACK_NETAPP_BTDISCONNECTDONE);
        _pWidgetEngine = NULL;
    }
}

/* there is 2 different bt list to choose from.
 *    index_to_connect:   is based on  wihch list you choose
 *    pDevInfoList:     Either 1)save bt device connection list or the 2) list that comes from the discovery
 */
/* used for lua commands  */
eRet CBluetooth::connectBluetooth(
        uint32_t             index_to_connect,
        NETAPP_BT_DEV_INFO * pDevInfoList
        )
{
    eRet           ret       = eRet_Ok;
    NETAPP_RETCODE retNetApp = NETAPP_SUCCESS;

    BDBG_ASSERT(NULL != _pNetApp);

    if (index_to_connect >= _discListCount)
    {
        BDBG_ERR(("index to connect(%d) out of range.  %d  of bt connections in discoverd list", index_to_connect, _discListCount));
    }
    BDBG_MSG(("connectBluetooth  index_to_connect %d", index_to_connect));
    retNetApp = NetAppBluetoothConnect(_pNetApp, &pDevInfoList[index_to_connect]);
    CHECK_NETAPP_ERROR_GOTO("unable to connect to Bluetooth device", ret, retNetApp, error);

    setConnectStatus(eConnectStatus_Connecting);
error:
    if (retNetApp != NETAPP_SUCCESS)
    {
        setConnectStatus(eConnectStatus_Disconnecting); /* using disconnecting as  a failure in attempt to connect Todo*/
    }
    notifyObservers(eNotify_BluetoothConnectionStatus, this);
    return(ret);
} /* connectBluetooth */

eRet CBluetooth::connectBluetooth(CBluetoothConnectionData * connectData)
{
    eRet               ret              = eRet_Ok;
    NETAPP_RETCODE     retNetApp        = NETAPP_SUCCESS;
    CBluetoothDevice * pBluetoothDevice = connectData->pBluetoothDevice;

    BDBG_ASSERT(NULL != _pNetApp);
    BDBG_ASSERT(NULL != pBluetoothDevice);
    NETAPP_BT_DEV_INFO * pDevInfoList = pBluetoothDevice->getBtDevInfoList();
    BDBG_ASSERT(NULL != pDevInfoList);

    if (pBluetoothDevice->getIndex() >= _discListCount)
    {
        BDBG_ERR(("index to connect(%d) out of range.  %d  of bt connections in discoverd list", pBluetoothDevice->getIndex(), _discListCount));
    }
    BDBG_MSG(("connectBluetooth  index_to_connect %d", pBluetoothDevice->getIndex()));
    retNetApp = NetAppBluetoothConnect(_pNetApp, &pDevInfoList[pBluetoothDevice->getIndex()]);
    CHECK_NETAPP_ERROR_GOTO("unable to connect to Bluetooth device", ret, retNetApp, error);

    setConnectStatus(eConnectStatus_Connecting);
error:
    if (retNetApp != NETAPP_SUCCESS)
    {
        setConnectStatus(eConnectStatus_Disconnecting); /* using disconnecting as  a failure in attempt to connect Todo*/
    }
    notifyObservers(eNotify_BluetoothConnectionStatus, this);
    return(ret);
} /* connectBluetooth */

/*   Only One bluetooth list to choose from
 *    index_to_connect:   is based on  wihch list you choose
 *     pDevInfoList:     1)save bt device connection list
 *  This is really meant to be Unpair
 */
eRet CBluetooth::disconnectBluetooth(int index_to_disconnect)
{
    eRet                 ret          = eRet_Ok;
    NETAPP_RETCODE       retNetApp    = NETAPP_SUCCESS;
    NETAPP_BT_DEV_INFO * pDevInfoList = NULL;
    uint32_t             count        = 0;

    BDBG_ASSERT(NULL != _pNetApp);

    /* Getting a new updated BT list */
    ret = getSavedBtListInfo(&_savedListCount, &_pBtDevInfoSavedList);

    if (_savedListCount == 0)
    {
        BDBG_ERR(("No saved devices found\n"));
        ret = eRet_InvalidState;
        goto error;
    }

    BDBG_MSG(("\n\n device %d to disconnect:", index_to_disconnect));

    if ((unsigned)index_to_disconnect < _savedListCount)
    {
        retNetApp = NetAppBluetoothDisconnect(_pNetApp, &_pBtDevInfoSavedList[index_to_disconnect]);
        CHECK_NETAPP_ERROR_GOTO("unable to disconnect from bluetooth", ret, retNetApp, error);
    }

    /* Remove from data base list as well */
    if ((unsigned)index_to_disconnect < _savedListCount)
    {
        retNetApp = NetAppBluetoothDeleteSavedDevInfo(_pNetApp, &_pBtDevInfoSavedList[index_to_disconnect]);
        if (!retNetApp)
        {
            _savedListCount--;
        }
        BDBG_WRN(("removing bluetooth device from database\n"));
    }
    /* updating the connection list after a disconnect just occured */
    updateConnectedBtList(&count, &pDevInfoList);
    updateBluetoothDeviceList();
    setConnectStatus(eConnectStatus_Disconnected);

error:
    notifyObservers(eNotify_BluetoothConnectionStatus, this);
    return(ret);
} /* disconnectBluetooth */

eRet CBluetooth::startDiscoveryBluetooth()
{
    eRet           ret       = eRet_Ok;
    NETAPP_RETCODE retNetApp = NETAPP_SUCCESS;

    BDBG_ASSERT(NULL != _pNetApp);

    if (true == isDiscoveryStarted())
    {
        return(ret);
    }

    BDBG_MSG(("start bluetooth discovery"));

    retNetApp = NetAppBluetoothDiscovery(_pNetApp, NETAPP_BT_SERVICE_NONE); /* maybe we can just scan for NETAPP_BT_SERVICE_HID */
    CHECK_NETAPP_ERROR_GOTO("unable to start bluetooth discovery", ret, retNetApp, error);

    notifyObservers(eNotify_BluetoothDiscoveryStarted, this);
error:
    return(ret);
} /* startDiscoveryBluetooth */

eRet CBluetooth::stopDiscoveryBluetooth()
{
    eRet           ret       = eRet_Ok;
    NETAPP_RETCODE retNetApp = NETAPP_SUCCESS;

    BDBG_ASSERT(NULL != _pNetApp);

    if (false == isDiscoveryStarted())
    {
        return(eRet_Ok);
    }

    BDBG_MSG(("stop Bluetooth Discovery"));

    retNetApp = NetAppBluetoothDiscoveryAbort(_pNetApp);
    CHECK_NETAPP_ERROR_GOTO("unable to stop bluetooth discovery", ret, retNetApp, error);

    notifyObservers(eNotify_BluetoothDiscoveryStopped);
error:
    return(eRet_Ok);
} /* stopDiscoveryBluetooth */

eRet CBluetooth::getSavedBtListInfo(
        uint32_t *            ulCount,
        NETAPP_BT_DEV_INFO ** pInfo
        )
{
    eRet           ret       = eRet_Ok;
    NETAPP_RETCODE retNetApp = NETAPP_SUCCESS;

    /* Getting a new updated BT list */
    if (_pBtDevInfoSavedList != NULL)
    {
        free(_pBtDevInfoSavedList);
        _pBtDevInfoSavedList = NULL;
        _savedListCount      = 0;
    }

    retNetApp = NetAppBluetoothGetSavedBtDevList(_pNetApp, &_pBtDevInfoSavedList, &_savedListCount);

    if (_savedListCount && (_pBtDevInfoSavedList != NULL))
    {
#if 1
        BDBG_MSG(("\nSaved Device List:"));
        print_debug_bt_dev_list(_pBtDevInfoSavedList, _savedListCount);
#endif
        *pInfo   = _pBtDevInfoSavedList;
        *ulCount = _savedListCount;
    }
    else
    if ((_savedListCount == 0) && (retNetApp == NETAPP_OUT_OF_MEMORY))
    {
        BDBG_WRN(("Nothing in saved  bt list or could not get Saved BT list\n"));
    }
    else
    {
        CHECK_NETAPP_ERROR_GOTO("unable to get saved BT device list", ret, retNetApp, error);
    }

error:
    notifyObservers(eNotify_BluetoothListStatusDone);
    return(ret);
} /* getSavedBtListInfo */

/* want a function that doesn't update hte connected pointer  list */
void CBluetooth::returnSavedBtListInfo(
        uint32_t *            ulCount,
        NETAPP_BT_DEV_INFO ** pBtDevInfo
        )
{
    *pBtDevInfo = _pBtDevInfoSavedList;
    *ulCount    = _savedListCount;
}

eRet CBluetooth::getDiscoveryBtListInfo(
        uint32_t *            ulCount,
        NETAPP_BT_DEV_INFO ** pInfo
        )
{
    eRet           ret       = eRet_Ok;
    NETAPP_RETCODE retNetApp = NETAPP_SUCCESS;

    *ulCount = _discListCount;
    *pInfo   = _pBtDevInfoDiscList;

    if (*ulCount)
    {
        BDBG_MSG(("Discovery  Device List:"));
        print_debug_bt_dev_list(_pBtDevInfoDiscList, _discListCount);
    }
    else
    {
        BDBG_MSG(("Nothing in  discovery  bt list\n"));
        retNetApp = NETAPP_NOT_FOUND;
    }

    notifyObservers(eNotify_BluetoothListStatusDone);

    return(ret);
} /* getDiscoveryBtListInfo */

eRet CBluetooth::updateConnectedBtList(
        uint32_t *            ulCount,
        NETAPP_BT_DEV_INFO ** pBtDevInfo
        )
{
    eRet           ret       = eRet_Ok;
    NETAPP_RETCODE retNetApp = NETAPP_SUCCESS;

    BDBG_ASSERT(NULL != _pNetApp);

    retNetApp = NetAppBluetoothGetConnectedDevices(_pNetApp, &_pBtDevInfoConnList, &_connListCount);
    BDBG_MSG(("%s: NetAppBluetoothGetConnectedDevices result _connListCount %d", __FUNCTION__, _connListCount));

    if (_connListCount && (_pBtDevInfoConnList != NULL))
    {
        BDBG_MSG(("%s: Connected Device List:", __FUNCTION__));
        print_debug_bt_dev_list(_pBtDevInfoConnList, _connListCount);
        *pBtDevInfo = _pBtDevInfoConnList;
        *ulCount    = _connListCount;
    }
    else
    if ((_savedListCount == 0) && (retNetApp == NETAPP_OUT_OF_MEMORY))
    {
        BDBG_WRN(("Nothing in saved  bt list or could not get Saved BT list\n"));
    }
    else
    {
        CHECK_NETAPP_ERROR_GOTO("unable to get saved BT device list", ret, retNetApp, error);
    }
error:

    notifyObservers(eNotify_BluetoothListStatusDone);
    return(ret);
} /* updateConnectedBtList */

/* want a function that doesn't update hte connected pointer  list */
void CBluetooth::returnConnectedBtList(
        uint32_t *            ulCount,
        NETAPP_BT_DEV_INFO ** pBtDevInfo
        )
{
    *pBtDevInfo = _pBtDevInfoConnList;
    *ulCount    = _connListCount;
}

/* Input provides list of bluetooth devices,  this is called whenever we get a discover done callback or a connected callback*/
eRet CBluetooth::updateBluetoothDeviceList()
{
    CBluetoothDevice * pBluetoothDevice = NULL;
    eRet               ret              = eRet_Ok;
    uint32_t           i;
    bool               foundConnectedA2DPSink = false;

    MListItr <CBluetoothDevice> itr(&_bluetoothDeviceList);
    _bluetoothDeviceList.clear(); /* clear the bluetooth device list and rebuild it */

    /* Add the saved List first */
    getSavedBtListInfo(&_savedListCount, &_pBtDevInfoSavedList);
    for (i = 0; i < _savedListCount; i++)
    {
        /* update bluetooth  info if already in bt device list */
        for (pBluetoothDevice = itr.first(); pBluetoothDevice; pBluetoothDevice = itr.next())
        {
            if ((MString(pBluetoothDevice->getName()) == _pBtDevInfoSavedList[i].cName) &&
                (MString(pBluetoothDevice->getAddress()) == _pBtDevInfoSavedList[i].cAddr))
            {
                /*
                 * if cleared the list should not be in here!!!
                 * found match in list - update it
                 */
                BDBG_WRN(("Item in saved list already exist. Ignore"));
                break;
            }
        }

        if (NULL == pBluetoothDevice)
        {
            /* add new bluetooth device */
            CBluetoothDevice * pBluetoothDeviceNew = new CBluetoothDevice(i, _pBtDevInfoSavedList, this);
            CHECK_PTR_ERROR_GOTO("unable to allocate new bluetooth memory", pBluetoothDeviceNew, ret, eRet_OutOfMemory, error);
            BDBG_MSG(("add new  Saved bluetooth device to bluetooth list:%s addr:%s", pBluetoothDeviceNew->getName(), pBluetoothDeviceNew->getAddress()));
            _bluetoothDeviceList.add(pBluetoothDeviceNew);
        }
    }

    /* need to mark devices that are connected !!!*/
    updateConnectedBtList(&_connListCount, &_pBtDevInfoConnList);
    for (i = 0; i < _connListCount; i++)
    {
        /* update bluetooth  info if already in bt device list */
        for (pBluetoothDevice = itr.first(); pBluetoothDevice; pBluetoothDevice = itr.next())
        {
            if ((MString(pBluetoothDevice->getName()) == _pBtDevInfoConnList[i].cName) &&
                (MString(pBluetoothDevice->getAddress()) == _pBtDevInfoConnList[i].cAddr))
            {
                /*
                 * if cleared the list should not be in here!!!
                 * found match in list - update it
                 */
                pBluetoothDevice->setConnectStatus(eBtConnectStatus_Connected);
                /* Start A2DP if its the right device */
                if (_pBtDevInfoSavedList[i].ulServiceMask && NETAPP_BT_SERVICE_A2DP_SINK)
                {
                    BDBG_MSG(("Found A2DP device"));
                    foundConnectedA2DPSink = true;
                }
                break;
            }
        }
    }

    if (foundConnectedA2DPSink)
    {
        BDBG_WRN(("A2DP connected device found, but atlas must be built for Nxclient to send audio"));
    }
    /* Add discovered list */
    for (i = 0; i < _discListCount; i++)
    {
        /* update bluetooth  info if already in bt device list */
        for (pBluetoothDevice = itr.first(); pBluetoothDevice; pBluetoothDevice = itr.next())
        {
            if ((MString(pBluetoothDevice->getName()) == _pBtDevInfoDiscList[i].cName) &&
                (MString(pBluetoothDevice->getAddress()) == _pBtDevInfoDiscList[i].cAddr))
            {
                /*
                 * if cleared the list should not be in here
                 * found match in list - update it
                 */
                BDBG_WRN(("Item is discoved list already exist. Ignore "));
                break;
            }
        }

        if (NULL == pBluetoothDevice)
        {
            /* add new bluetooth device */
            CBluetoothDevice * pBluetoothDeviceNew = new CBluetoothDevice(i, _pBtDevInfoDiscList, this);
            CHECK_PTR_ERROR_GOTO("unable to allocate new bluetooth memory", pBluetoothDeviceNew, ret, eRet_OutOfMemory, error);
            BDBG_MSG(("add new Discovered bluetooth device to bluetooth list:%s addr:%s", pBluetoothDeviceNew->getName(), pBluetoothDeviceNew->getAddress()));
            _bluetoothDeviceList.add(pBluetoothDeviceNew);
        }
    }

    dumpDeviceList();
error:
    return(ret);
} /* updateBluetoothDeviceList */

void CBluetooth::dumpDeviceList()
{
    BDBG_Level         level;
    CBluetoothDevice * pBluetoothDevice = NULL;

    MListItr <CBluetoothDevice> itr(&_bluetoothDeviceList);
    int i = 0;

#if 1
    BDBG_GetModuleLevel("atlas_bluetooth", &level);
    BDBG_SetModuleLevel("atlas_bluetooth", BDBG_eMsg);
#endif
    for (pBluetoothDevice = itr.first(); pBluetoothDevice; pBluetoothDevice = itr.next())
    {
        BDBG_MSG(("\n%d:***********dump Device List***************\n", i));
        BDBG_MSG(("Name:        %s\n", pBluetoothDevice->getName()));
        BDBG_MSG(("BD Addr: %s\n", pBluetoothDevice->getAddress()));
        BDBG_MSG(("RSSI:        %d dB\n", pBluetoothDevice->getSignalStrength()));
        BDBG_MSG(("Paired:      %d \n", pBluetoothDevice->isPaired()));
        BDBG_MSG(("isConnected:     %d \n", pBluetoothDevice->isConnected()));
#if 0
        BDBG_MSG(("Services:    [%s%s%s%s%s%s%s%s%s]0x%04x\n",
                  (pDevInfoList[i].ulServiceMask & NETAPP_BT_SERVICE_HID)   ? "HID " : "",
                  (pDevInfoList[i].ulServiceMask & NETAPP_BT_SERVICE_HSP)   ? "HSP " : "",
                  (pDevInfoList[i].ulServiceMask & NETAPP_BT_SERVICE_HFP)   ? "HFP " : "",
                  (pDevInfoList[i].ulServiceMask & NETAPP_BT_SERVICE_OPP)   ? "OPP " : "",
                  (pDevInfoList[i].ulServiceMask & NETAPP_BT_SERVICE_FTP)   ? "FTP " : "",
                  (pDevInfoList[i].ulServiceMask & NETAPP_BT_SERVICE_A2DP_SOURCE)  ? "A2DP-Source " : "",
                  (pDevInfoList[i].ulServiceMask & NETAPP_BT_SERVICE_A2DP_SINK)  ? "A2DP-Sink " : "",
                  (pDevInfoList[i].ulServiceMask & NETAPP_BT_SERVICE_AVRCP) ? "AVRCP " : "",
                  (pDevInfoList[i].ulServiceMask & NETAPP_BT_SERVICE_BLE) ? "HOGP" : "",
                  pDevInfoList[i].ulServiceMask));
        BDBG_MSG(("COD Major:   0x%02x\n", pDevInfoList[i].ucMajorClassDev));
        BDBG_MSG(("COD Minor:   0x%02x\n", pDevInfoList[i].ucMinorClassDev));
        BDBG_MSG(("COD Service:0x%04x\n", pDevInfoList[i].usServiceClassDev));
#endif /* if 0 */
        BDBG_MSG(("%d:***********dumpDevice List***************\n", i));
        i++;
    }

#if 1
    BDBG_SetModuleLevel("atlas_bluetooth", level);
#endif
} /* dumpDeviceList */

void CBluetooth::dump(
        NETAPP_BT_DEV_INFO * pDevInfoList,
        int                  size,
        bool                 bForce
        )
{
    BDBG_Level level;

    BDBG_ASSERT(NULL != pDevInfoList);

    if (true == bForce)
    {
        BDBG_GetModuleLevel("atlas_bluetooth", &level);
        BDBG_SetModuleLevel("atlas_bluetooth", BDBG_eMsg);
    }

    int i;
    if ((size == 0) || (pDevInfoList == NULL))
    {
        BDBG_MSG(("List is empty\n"));
        return;
    }
    for (i = 0; i < size; i++)
    {
        BDBG_MSG(("\n%d:**************************\n", i));
        BDBG_MSG(("Name:        %s\n", pDevInfoList[i].cName));
        BDBG_MSG(("BD Addr: %s\n", pDevInfoList[i].cAddr));
        BDBG_MSG(("RSSI:        %d dB\n", pDevInfoList[i].lRssi));
        BDBG_MSG(("Services:    [%s%s%s%s%s%s%s%s%s]0x%04x\n",
                  (pDevInfoList[i].ulServiceMask & NETAPP_BT_SERVICE_HID)   ? "HID " : "",
                  (pDevInfoList[i].ulServiceMask & NETAPP_BT_SERVICE_HSP)   ? "HSP " : "",
                  (pDevInfoList[i].ulServiceMask & NETAPP_BT_SERVICE_HFP)   ? "HFP " : "",
                  (pDevInfoList[i].ulServiceMask & NETAPP_BT_SERVICE_OPP)   ? "OPP " : "",
                  (pDevInfoList[i].ulServiceMask & NETAPP_BT_SERVICE_FTP)   ? "FTP " : "",
                  (pDevInfoList[i].ulServiceMask & NETAPP_BT_SERVICE_A2DP_SOURCE)  ? "A2DP-Source " : "",
                  (pDevInfoList[i].ulServiceMask & NETAPP_BT_SERVICE_A2DP_SINK)  ? "A2DP-Sink " : "",
                  (pDevInfoList[i].ulServiceMask & NETAPP_BT_SERVICE_AVRCP) ? "AVRCP " : "",
                  (pDevInfoList[i].ulServiceMask & NETAPP_BT_SERVICE_BLE) ? "HOGP" : "",
                  pDevInfoList[i].ulServiceMask));
        BDBG_MSG(("COD Major:   0x%02x\n", pDevInfoList[i].ucMajorClassDev));
        BDBG_MSG(("COD Minor:   0x%02x\n", pDevInfoList[i].ucMinorClassDev));
        BDBG_MSG(("COD Service:0x%04x\n", pDevInfoList[i].usServiceClassDev));
        BDBG_MSG(("%d:**************************\n", i));
    }

    if (true == bForce)
    {
        BDBG_SetModuleLevel("atlas_bluetooth", level);
    }
} /* dump */

eRet CBluetooth::processAudioData(
        void **  buffer,
        unsigned bufferSize
        )
{
    eRet ret = eRet_Ok;

    BSTD_UNUSED(buffer);
    BSTD_UNUSED(bufferSize);
    BDBG_ERR(("%s:Only supported in Atlas NxClient mode(export NEXUS_MODE=client). Implementation left as stub. Implemented in NX client version  ", __FUNCTION__));

    return(ret);
} /* processAudioData */

eRet CBluetooth::processAudioSampleRate(unsigned sampleRate)
{
    eRet ret = eRet_Ok;

    BSTD_UNUSED(sampleRate);
    BDBG_ERR(("%s:Only supported in Atlas NxClient mode (export NEXUS_MODE=client). Implementation left as stub. Implemented in NX client version  ", __FUNCTION__));
    return(ret);
} /* processAudioData */

eRet CBluetooth::startAV(void)
{
    eRet ret = eRet_Ok;

    BDBG_ERR(("%s:Only supported in Atlas NxClient mode (export NEXUS_MODE=client). Implementation left as stub. Implemented in NX client version  ", __FUNCTION__));
    return(ret);
} /* startAV */

eRet CBluetooth::stopAV(void)
{
    eRet ret = eRet_Ok;

    BDBG_ERR(("%s:Only supported in Atlas NxClient mode (export NEXUS_MODE=client). Implementation left as stub. Implemented in NX client version  ", __FUNCTION__));
    return(ret);
} /* stopAV */

#endif /* ifdef NETAPP_SUPPORT */