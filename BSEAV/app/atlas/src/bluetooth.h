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

#ifndef _BLUETOOTH_H__
#define _BLUETOOTH_H__

#include "resource.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef NETAPP_SUPPORT
#include "netapp.h"
#include "network.h"
#include "remote.h"
#include "audio_capture_client.h"
#include "audio_capture.h"
#endif /* ifdef NETAPP_SUPPORT */

class CWidgetEngine;
#ifdef NETAPP_SUPPORT
class CBluetoothDevice;
class CBluetooth;

typedef enum eBtConnectStatus
{
    eBtConnectStatus_Unknown,
    eBtConnectStatus_Connecting,
    eBtConnectStatus_Connected,
    eBtConnectStatus_Disconnecting,
    eBtConnectStatus_Disconnected,
    eBtConnectStatus_Max
} eBtConnectStatus;

/* data for connection and disconnection */
class CBluetoothConnectionData
{
public:
    CBluetoothConnectionData() :
        index(0),
        pDevInfoList(NULL),
        pBluetoothDevice(NULL)
    {}
    virtual ~CBluetoothConnectionData() {}

    void          dump(bool bForce = false);
    bool operator ==(CBluetoothConnectionData &other)
    {
        return((index == other.index) && strncmp(pDevInfoList->cAddr, other.pDevInfoList->cAddr, strlen(pDevInfoList->cAddr)) == 0);
    }

public:
    int                  index;  /*index to connect or disconnect   */
    NETAPP_BT_DEV_INFO * pDevInfoList;
    CBluetoothDevice *   pBluetoothDevice;
};

class CBluetooth : public CAudioCaptureClient
{
public:
    CBluetooth(
            const char *     name,
            const unsigned   number,
            CConfiguration * pCfg
            );
    ~CBluetooth(void);

    eRet open(CWidgetEngine * pWidgetEngine);
    void close(void);
    eRet connectBluetooth(CBluetoothConnectionData * connectData);
    eRet connectBluetooth(uint32_t index_to_connect, NETAPP_BT_DEV_INFO * pDevInfoList);
    eRet connectBluetoothFromDiscList(uint32_t index_to_connect) { return(connectBluetooth(index_to_connect, _pBtDevInfoDiscList)); }

    /*eRet disconnectBluetooth(int index_to_disconnect); */
    eRet startDiscoveryBluetooth(void);
    eRet stopDiscoveryBluetooth(void);

    void                          setDiscoveryStatus(eRet discoveryStatus)                { _discoveryStatus = discoveryStatus; }
    eRet                          getDiscoveryStatus(void)                                { return(_discoveryStatus); }
    void                          setConnectStatus(eConnectStatus connectStatus)          { _connectStatus = connectStatus; }
    void                          setBluetoothRemote(CBluetoothRemote * pBluetoothRemote) { _pBluetoothRemote = pBluetoothRemote; }
    eConnectStatus                getConnectStatus(void)                                  { return(_connectStatus); }
    bool                          isDiscoveryStarted(void)                                { return(_bDiscoveryStarted); }
    void                          setDiscoveryStarted(bool bDiscoveryStarted)             { _bDiscoveryStarted = bDiscoveryStarted; }
    CWidgetEngine *               getWidgetEngine(void)                                   { return(_pWidgetEngine); }
    eRet                          getSavedBtListInfo(uint32_t * ulCount, NETAPP_BT_DEV_INFO ** pInfo);
    void                          returnSavedBtListInfo(uint32_t * ulCount, NETAPP_BT_DEV_INFO ** pInfo);
    eRet                          getDiscoveryBtListInfo(uint32_t * ulCount, NETAPP_BT_DEV_INFO ** pInfo);
    eRet                          updateConnectedBtList(uint32_t * ulCount, NETAPP_BT_DEV_INFO ** pInfo);
    void                          returnConnectedBtList(uint32_t * ulCount, NETAPP_BT_DEV_INFO ** pInfo);
    uint32_t                      getConnectedCount()          { return(_connListCount); }
    MAutoList<CBluetoothDevice> * getBluetoothDeviceList(void) { return(&_bluetoothDeviceList); }
    /*eRet                          updateBluetoothDeviceList(void); */
    void dumpDeviceList(void);
    void dump(NETAPP_BT_DEV_INFO * pInfo, int size, bool bForce = false);

    void btDiscoveryDoneCallback();
    void btConnectDoneCallback();
    void btDisconnectDoneCallback();
    CBluetoothRemote * _pBluetoothRemote; /* pointer to the CBluetootRemote class */

    virtual eRet disconnectBluetooth(int index_to_disconnect);
    virtual eRet updateBluetoothDeviceList(void);
    virtual eRet processAudioData(void ** buffer, unsigned bufferSize); /* take the audio buffer and send it out to bt devices that are capable */
    virtual eRet processAudioSampleRate(unsigned sampleRate);           /* process sample rate change */
    virtual eRet startAV(void);                                         /* start AV streamiing for A2DP  */
    virtual eRet stopAV(void);                                          /* stop AV streaming for A2DP  */

protected:
    CWidgetEngine * _pWidgetEngine;
    eRet            _discoveryStatus;
    eConnectStatus  _connectStatus; /*Can i keep track of this with property create an internal  device list, and maintains the connections status for that device Todo.*/
    bool            _bDiscoveryStarted;

    NETAPP_HANDLE                 _pNetApp;
    NETAPP_BT_DEV_INFO *          _pBtDevInfoDiscList;  /* results from discovery */
    uint32_t                      _discListCount;       /* number of results from discovery List */
    NETAPP_BT_DEV_INFO *          _pBtDevInfoSavedList; /* results from saveBtConnections */
    uint32_t                      _savedListCount;      /* number of results from saveBTConnections */
    NETAPP_BT_DEV_INFO *          _pBtDevInfoConnList;  /* results from saveBtConnections */
    uint32_t                      _connListCount;       /* number of results from saveBTConnections */
    MAutoList<NETAPP_BT_DEV_INFO> _bluetoothList;       /*Todo: : Don't think I need this */
    MAutoList<CBluetoothDevice>   _bluetoothDeviceList;
    NETAPP_BT_AUDIO_FORMAT        _tBtAudioFormat;
};

/*data for a particular device. */
class CBluetoothDevice
{
public:
    CBluetoothDevice(
            uint32_t             index,
            NETAPP_BT_DEV_INFO * _pBtDevInfoList,
            CBluetooth *         pBluetooth
            );
    ~CBluetoothDevice(void);
    void update(uint32_t         index,
            NETAPP_BT_DEV_INFO * _pBtDevInfoList,
            CBluetooth *         pBluetooth);
    char *               getName(void);
    char *               getAddress(void);
    bool                 isConnected(void);
    bool                 isPaired(void);
    int                  getSignalStrength(void);
    void                 setIndex(uint32_t ind)                          { _index = ind; }
    uint32_t             getIndex(void)                                  { return(_index); }  /* may decide later to hide this if we have a connect here */
    void                 setBtDevInfoList(NETAPP_BT_DEV_INFO * pDevList) { _pBtDevInfoList = pDevList; }
    NETAPP_BT_DEV_INFO * getBtDevInfoList(void)                          { return(_pBtDevInfoList); }
    void                 setBluetooth(CBluetooth * pBluetooth)           { _pBluetooth = pBluetooth; }
    eRet                 getStatusList(MStringHash * pStatus, bool bClear = true);
    void                 setConnectStatus(eBtConnectStatus connectStatus) { _connectStatus = connectStatus; }

protected:
    uint32_t             _index;          /* index into saved or discovery list */
    NETAPP_BT_DEV_INFO * _pBtDevInfoList; /*pointer  to either saved or discovery list  */
    eBtConnectStatus     _connectStatus;
    CBluetooth *         _pBluetooth;  /* pointer to the CBluetooth class,Used to get Connected  */
};

#endif /* ifdef NETAPP_SUPPORT */

#ifdef __cplusplus
}
#endif

#endif /* BLUETOOTH_H__ */