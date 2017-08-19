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

#include "model.h"
#include "notification.h"
#include "atlas.h"
#include "remote.h"
#include "display.h"
#include "channelmgr.h"
#include "discovery.h"
#include "config.h"

BDBG_MODULE(atlas_model);

CModel::CModel(const char * strName) :
    CMvcModel(strName),
    _pConfig(NULL),
    _pChannelMgr(NULL),
    _pGraphics(NULL),
    _fullScreenWindowType(eWindowType_Main),
    _pVideoDecode(NULL),
    _pStillDecode(NULL),
    _pThumbExtractor(NULL),
    _pAudioDecode(NULL),
    _pRecord(NULL),
#if NEXUS_HAS_VIDEO_ENCODER
    _pEncode(NULL),
#endif
    _pPlaybackList(NULL),
    _id(0),
    _bPip(false),
    _bPipEnabled(false),
    _bipTranscodeEnabled(false),
    _ipTranscodeProfile(0),
    _bPipSwapped(false),
    _bScanSaveOffer(false),
    _lastChannelPowerSave(NULL),
    _pPower(NULL),
#ifdef DCC_SUPPORT
    _pClosedCaption(NULL),
#endif
#ifdef SNMP_SUPPORT
    _pSnmp(NULL),
#endif
    _pServerMgr(NULL),
    _pAutoDiscoveryServer(NULL),
    _pAutoDiscoveryClient(NULL),
    _pPlaylistDb(NULL),
    _pAudioCapture(NULL),
    _simpleVideoDecoderServer(NULL),
    _simpleAudioDecoderServer(NULL),
    _simpleEncoderServer(NULL),
    _dynamicRangeLast(eDynamicRange_Unknown)
#ifdef CPUTEST_SUPPORT
    , _pCpuTest(NULL)
#endif
#ifdef WPA_SUPPLICANT_SUPPORT
    , _pWifi(NULL)
#endif
{
    for (int i = 0; i < eWindowType_Max; i++)
    {
        _deferredChannelNum[i]    = "";
        _pDeferredChannel[i]      = NULL;
        _mode[i]                  = eMode_Live;
        _pStc[i]                  = NULL;
        _pSimpleVideoDecode[i]    = NULL;
        _pSimpleAudioDecode[i]    = NULL;
        _currentChannel[i]        = NULL;
        _channelTuneInProgress[i] = NULL;
        _currentPlayback[i]       = NULL;
        _currentChannel[i]        = NULL;
        _lastChannel[i]           = NULL;
        _connectId[i]             = 0;
    }
#ifndef NXCLIENT_SUPPORT
    _simpleVideoDecoderServer = NEXUS_SimpleVideoDecoderServer_Create();
    _simpleAudioDecoderServer = NEXUS_SimpleAudioDecoderServer_Create();
    _simpleEncoderServer      = NEXUS_SimpleEncoderServer_Create();
#endif

    _irRemoteList.clear();
#if NEXUS_HAS_UHF_INPUT
    _uhfRemoteList.clear();
#endif
    _displayList.clear();
}

CModel::~CModel()
{
#ifndef NXCLIENT_SUPPORT
    NEXUS_SimpleVideoDecoderServer_Destroy(_simpleVideoDecoderServer);
    NEXUS_SimpleAudioDecoderServer_Destroy(_simpleAudioDecoderServer);
    NEXUS_SimpleEncoderServer_Destroy(_simpleEncoderServer);
#endif
}

void CModel::addDisplay(CDisplay * pDisplay)
{
    BDBG_ASSERT(NULL != pDisplay);
    _displayList.add(pDisplay);
}

void CModel::removeDisplay(CDisplay * pDisplay)
{
    BDBG_ASSERT(NULL != pDisplay);
    _displayList.remove(pDisplay);
}

CDisplay * CModel::getDisplay(uint32_t num)
{
    MListItr<CDisplay> itr(&_displayList);
    CDisplay *         pDisplay = NULL;

    for (pDisplay = itr.first(); NULL != pDisplay; pDisplay = itr.next())
    {
        if (num == pDisplay->getNumber())
        {
            break;
        }
    }

    return(pDisplay);
}

void CModel::addAudioOutput(COutput * pAudioOutput)
{
    BDBG_ASSERT(NULL != pAudioOutput);
    _audioOutputList.add(pAudioOutput);
}

COutput * CModel::getAudioOutput(eBoardResource resourceType)
{
    COutput * pOutput = NULL;

    for (pOutput = _audioOutputList.first(); pOutput; pOutput = _audioOutputList.next())
    {
        if (resourceType == pOutput->getType())
        {
            break;
        }
    }

    return(pOutput);
}

void CModel::removeAudioOutput(COutput * pAudioOutput)
{
    BDBG_ASSERT(NULL != pAudioOutput);
    _audioOutputList.remove(pAudioOutput);
}

void CModel::addIrRemote(CIrRemote * pIrRemote)
{
    BDBG_ASSERT(NULL != pIrRemote);
    _irRemoteList.add(pIrRemote);
}

void CModel::removeIrRemote(CIrRemote * pIrRemote)
{
    BDBG_ASSERT(NULL != pIrRemote);
    _irRemoteList.remove(pIrRemote);
}

CIrRemote * CModel::getIrRemote(NEXUS_IrInputMode mode)
{
    MListItr<CIrRemote> itr(&_irRemoteList);
    CIrRemote *         pIrRemote = NULL;

    if (NEXUS_IrInputMode_eMax == mode)
    {
        /* no mode specified so return first remote in list */
        return(itr.first());
    }

    for (pIrRemote = itr.first(); NULL != pIrRemote; pIrRemote = itr.next())
    {
        if (mode == pIrRemote->getMode())
        {
            break;
        }
    }

    return(pIrRemote);
} /* getIrRemote */

#if RF4CE_SUPPORT
void CModel::addRf4ceRemote(CRf4ceRemote * pRf4ceRemote)
{
    BDBG_ASSERT(NULL != pRf4ceRemote);
    _rf4ceRemoteList.add(pRf4ceRemote);
}

void CModel::removeRf4ceRemote(CRf4ceRemote * pRf4ceRemote)
{
    BDBG_ASSERT(NULL != pRf4ceRemote);
    _rf4ceRemoteList.remove(pRf4ceRemote);
}

CRf4ceRemote * CModel::getRf4ceRemote()
{
    return(_rf4ceRemoteList.first());
} /* getRf4ceRemote */

#endif /* if RF4CE_SUPPORT */
#ifdef NETAPP_SUPPORT
void CModel::addBluetoothRemote(CBluetoothRemote * pBluetoothRemote)
{
    BDBG_ASSERT(NULL != pBluetoothRemote);
    _bluetoothRemoteList.add(pBluetoothRemote);
}

void CModel::removeBluetoothRemote(CBluetoothRemote * pBluetoothRemote)
{
    BDBG_ASSERT(NULL != pBluetoothRemote);
    _bluetoothRemoteList.remove(pBluetoothRemote);
}

CBluetoothRemote * CModel::getBluetoothRemote()
{
    return(_bluetoothRemoteList.first());
} /* getBluetoothRemote */

#endif /* ifdef NETAPP_SUPPORT */

#if NEXUS_HAS_UHF_INPUT
void CModel::addUhfRemote(CUhfRemote * pUhfRemote)
{
    BDBG_ASSERT(NULL != pUhfRemote);
    _uhfRemoteList.add(pUhfRemote);
}

void CModel::removeUhfRemote(CUhfRemote * pUhfRemote)
{
    BDBG_ASSERT(NULL != pUhfRemote);
    _uhfRemoteList.remove(pUhfRemote);
}

CUhfRemote * CModel::getUhfRemote()
{
    return(_uhfRemoteList.first());
} /* getIrRemote */

#endif /* if NEXUS_HAS_UHF_INPUT */

#include "nexus_simple_video_decoder_server.h"
void CModel::swapDecodeVideoWindows()
{
    CSimpleVideoDecode * pVideoDecodeMain = getSimpleVideoDecode(eWindowType_Main);
    CSimpleVideoDecode * pVideoDecodePip  = getSimpleVideoDecode(eWindowType_Pip);

    /* swap video windows each decoder is connected to */
    NEXUS_SimpleVideoDecoder_SwapWindows(getSimpleVideoDecoderServer(),
            pVideoDecodeMain->getSimpleDecoder(),
            pVideoDecodePip->getSimpleDecoder());

    /* swap video windows in CSimpleVideoDecode objects - nexus has already swapped handles underneath*/
    pVideoDecodeMain->swapVideoWindowLists(pVideoDecodePip);

    setPipSwapState(false == getPipSwapState() ? true : false);
}

void CModel::swapSimpleAudioDecode()
{
    CSimpleAudioDecode * pDecodeTmp = NULL;

    pDecodeTmp                            = _pSimpleAudioDecode[eWindowType_Main];
    _pSimpleAudioDecode[eWindowType_Main] = _pSimpleAudioDecode[eWindowType_Pip];
    _pSimpleAudioDecode[eWindowType_Pip]  = pDecodeTmp;
}

void CModel::setPipState(bool bPip)
{
    _bPip = bPip;
    notifyObservers(eNotify_PipStateChanged);
}

void CModel::setMode(
        eMode       mode,
        eWindowType windowType
        )
{
    if (eWindowType_Max == windowType)
    {
        windowType = _fullScreenWindowType;
    }

    _mode[windowType] = mode;
}

void CModel::setConnectId(
        unsigned    connectId,
        eWindowType windowType
        )
{
    if (eWindowType_Max == windowType)
    {
        windowType = _fullScreenWindowType;
    }

    _connectId[windowType] = connectId;
}

void CModel::setCurrentChannel(
        CChannel *  pChannel,
        eWindowType windowType
        )
{
    if (eWindowType_Max == windowType)
    {
        windowType = _fullScreenWindowType;
    }

    if ((NULL == pChannel) &&
        (eWindowType_Mosaic1 > windowType) &&
        (NULL != _currentChannel[windowType]) &&
        (false == _currentChannel[windowType]->isStopAllowed()))
    {
        /* non stoppable channels are those that exist in the channel list.
         * only channel list channels are saved as the last channel.
         * last channel does not make sense for stoppable channels because
         * they are not a part of a channel list.  one example of a
         * stoppable channel is discovered ip channels. */
        _lastChannel[windowType] = _currentChannel[windowType];
    }

    _currentChannel[windowType] = pChannel;

done:
    notifyObservers((NULL != pChannel) ? eNotify_CurrentChannel : eNotify_CurrentChannelNull);
} /* setCurrentChannel */

void CModel::setChannelTuneInProgress(
        CChannel *  pChannel,
        eWindowType windowType
        )
{
    if (eWindowType_Max == windowType)
    {
        windowType = _fullScreenWindowType;
    }

    _channelTuneInProgress[windowType] = pChannel;
}

void CModel::setPlayback(
        CPlayback * pPlayback,
        eWindowType windowType
        )
{
    if (eWindowType_Max == windowType)
    {
        windowType = _fullScreenWindowType;
    }

    _currentPlayback[windowType] = pPlayback;
}

#if DVR_LIB_SUPPORT
void CModel::setTsb(
        CTsb *      pTsb,
        eWindowType windowType
        )
{
    BDBG_WRN(("%s: %u", BSTD_FUNCTION, windowType));
    if (eWindowType_Max == windowType)
    {
        windowType = _fullScreenWindowType;
    }

    _currentTsb[windowType] = pTsb;
}

#endif /* if DVR_LIB_SUPPORT */

void CModel::setFullScreenWindowType(eWindowType windowType)
{
    if (_fullScreenWindowType == windowType)
    {
        /* no change so just return */
        return;
    }

    _fullScreenWindowType = windowType;

    /* swapping video windows will NOT result in any video callbacks so we'll
     * send them manually so views can update accordingly */
    notifyObservers(eNotify_CurrentChannel);
    notifyObservers(eNotify_VideoSourceChanged, getSimpleVideoDecode());
}

eWindowType CModel::getPipScreenWindowType()
{
    eWindowType winType = eWindowType_Main;

    if (eWindowType_Main == _fullScreenWindowType)
    {
        winType = eWindowType_Pip;
    }

    return(winType);
}

void CModel::resetChannelHistory()
{
    for (int i = 0; i < eWindowType_Max; i++)
    {
        _deferredChannelNum[i].clear();

        _pDeferredChannel[i] = NULL;
        _currentChannel[i]   = NULL;
        _lastChannel[i]      = NULL;
    }
}

/* search all "current channels" for matches to given channel (ignore given excluded windowType.
 * return the number of matches found */
unsigned CModel::numMatchingCurrentChannels(
        CChannel *  pChannel,
        eWindowType excluded
        )
{
    unsigned count = 0;

    for (int i = 0; i < eWindowType_Max; i++)
    {
        if ((i != excluded) && (pChannel == _currentChannel[i]))
        {
            count++;
        }
    }

    return(count);
}

void CModel::setDeferredChannelNum(
        const char * strChNum,
        CChannel *   pCh,
        eWindowType  windowType
        )
{
    if (eWindowType_Max == windowType)
    {
        windowType = getFullScreenWindowType();
    }

    if (NULL == strChNum)
    {
        _deferredChannelNum[windowType].clear();
        _pDeferredChannel[windowType] = NULL;
    }
    else
    {
        _deferredChannelNum[windowType] = MString(strChNum);
        _pDeferredChannel[windowType]   = pCh;
    }

    if ((NULL != strChNum) || (NULL != pCh))
    {
        notifyObservers(eNotify_DeferredChannel, &windowType);
    }
} /* setDeferredChannelNum */

/* send keyDown event to registered observers.  this circumvents the bwidget
 * keyDown handling heirarchy where keys are sent to the currently focused window
 * and propogates through its parents if not consumed. this function should only
 * be used for keyDown events that need to be handled irrespective of the current
 * window focus.
 *
 * note: keyDown events go to all registered observers - there is no heirarchy
 * and no way to consume a keypress along the way */
eRet CModel::sendGlobalKeyDown(eKey * pKey)
{
    BDBG_WRN(("%s key:%d", BSTD_FUNCTION, *pKey));
    return(notifyObservers(eNotify_KeyDown, pKey));
}