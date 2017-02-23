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

#include "nexus_audio_capture.h"
#include "bluetooth_nx.h"
#include "atlas_os.h"
#include "nxclient.h"

BDBG_MODULE(atlas_bluetooth);

#ifdef NETAPP_SUPPORT
CBluetoothNx::CBluetoothNx(
        const char *     name,
        const uint16_t   number,
        CConfiguration * pCfg
        ) :
    CBluetooth(name, number, pCfg)
{
    _tBtAudioFormat.tMode           = NETAPP_BT_AV_MODE_STEREO;
    _tBtAudioFormat.ulSampleRate    = 44100;
    _tBtAudioFormat.ucBitsPerSample = 16;
    _eBtAVState                     = eBtAVState_Off;
}

CBluetoothNx::~CBluetoothNx()
{
}

/*   Only One bluetooth list to choose from
 *    index_to_connect:   is based on  wihch list you choose
 *     pDevInfoList:     1)save bt device connection list
 *  This is really meant to be Unpair
 */
eRet CBluetoothNx::disconnectBluetooth(int index_to_disconnect)
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
        if ((_eBtAVState == eBtAVState_On) && (_pBtDevInfoSavedList[index_to_disconnect].ulServiceMask & NETAPP_BT_SERVICE_A2DP_SINK))
        {
            /* Stop A2DP before you can disconnect */
            BDBG_MSG(("%s: Stop A2DP", __FUNCTION__));
            notifyObservers(eNotify_BluetoothA2DPStop);
            /* Let the call to BSA_Stop occur  before disconnecting*/
            BKNI_Sleep(2000);
        }

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

/* Input provides list of bluetooth devices,  this is called whenever we get a discover done callback or a connected callback*/
eRet CBluetoothNx::updateBluetoothDeviceList()
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
            BDBG_MSG(("%s: Searching Device connlistname %s devicename %s(service mask: %d)",
                      __FUNCTION__, _pBtDevInfoConnList[i].cName, pBluetoothDevice->getName(), _pBtDevInfoConnList[i].ulServiceMask));
            if ((MString(pBluetoothDevice->getName()) == _pBtDevInfoConnList[i].cName) &&
                (MString(pBluetoothDevice->getAddress()) == _pBtDevInfoConnList[i].cAddr))
            {
                /*
                 * if cleared the list should not be in here!!!
                 * found match in list - update it
                 */
                pBluetoothDevice->setConnectStatus(eBtConnectStatus_Connected);
                /* Start A2DP if its the right device */
                if (_pBtDevInfoConnList[i].ulServiceMask & NETAPP_BT_SERVICE_A2DP_SINK)
                {
                    BDBG_MSG(("%s: Found A2DP device", __FUNCTION__));
                    foundConnectedA2DPSink = true;
                    break;
                }
            }
        }
    }

    BDBG_MSG(("%s: _eBtAVState %d , foundConnectedA2DPSink %d ", __FUNCTION__, _eBtAVState, foundConnectedA2DPSink));
    if ((_eBtAVState == eBtAVState_Off) && foundConnectedA2DPSink)
    {
        BDBG_MSG(("%s: nx_client version: A2DP connected device found, starting A2DP...", __FUNCTION__));
        BDBG_WRN(("For best bluetooth audio quality, exit out of Bluetooth menu to avoid simultaneous discovery and a2dp streaming "));
        BDBG_WRN(("BSA has a mechanism to decreases bit rate for A2DP during a discovery"));
        notifyObservers(eNotify_BluetoothA2DPStart);
    }
    else
    if ((_eBtAVState == eBtAVState_On) && !foundConnectedA2DPSink)
    {
        BDBG_MSG(("%s:nx_client version:: NO A2DP connected device found, stoppping A2DP...", __FUNCTION__));
        notifyObservers(eNotify_BluetoothA2DPStop);
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

eRet CBluetoothNx::processAudioData(
        void **  buffer,
        unsigned bufferSize
        )
{
    eRet ret = eRet_Ok;

    NETAPP_RETCODE retNetApp = NETAPP_SUCCESS;

    /*  BDBG_WRN(("%s: >>> process Audio Data buffer addr %p bufferSize %d ", __FUNCTION__, *buffer, bufferSize)); */
    retNetApp = NetAppBluetoothSendAudioBuffer(_pNetApp, *buffer, bufferSize);
    CHECK_NETAPP_ERROR("unable to send Audio Buffer  to Bluetooth device", retNetApp);

    return(ret);
} /* processAudioData */

eRet CBluetoothNx::processAudioSampleRate(unsigned sampleRate)
{
    eRet           ret       = eRet_Ok;
    NETAPP_RETCODE retNetApp = NETAPP_SUCCESS;

    BSTD_UNUSED(sampleRate);

    BDBG_MSG(("%s: processAudioSampleRate %d ", __FUNCTION__, sampleRate));
    NetAppBluetoothAvStop(_pNetApp);
    _tBtAudioFormat.ulSampleRate = sampleRate;

    /* Let the call to BSA_Stop occur  before calling start again */
    BKNI_Sleep(1000);

    retNetApp = NetAppBluetoothAvStart(_pNetApp, false, &_tBtAudioFormat);

    return(ret);
} /* processAudioData */

eRet CBluetoothNx::startAV(void)
{
    eRet ret = eRet_Ok;

    if (_eBtAVState == eBtAVState_Off)
    {
        BDBG_MSG(("%s: bluetooth start AV ", __FUNCTION__));
        NetAppBluetoothAvStart(_pNetApp, false, &_tBtAudioFormat);
        _eBtAVState = eBtAVState_On;
    }
    return(ret);
} /* startAV */

eRet CBluetoothNx::stopAV(void)
{
    eRet ret = eRet_Ok;

    if (_eBtAVState == eBtAVState_On)
    {
        BDBG_MSG(("%s: bluetooth stop AV ", __FUNCTION__));
        NetAppBluetoothAvStop(_pNetApp);
        _eBtAVState = eBtAVState_Off;
    }
    return(ret);
} /* stopAV */

#endif /* ifdef NETAPP_SUPPORT */