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

#include "board.h"

#include "display.h"
#include "video_decode.h"
#include "audio_decode.h"
#include "still_decode.h"
#include "power.h"
#include "wifi.h"
#ifdef NETAPP_SUPPORT
#include "network.h"
#include "bluetooth.h"
#endif
#ifdef PLAYBACK_IP_SUPPORT
#include "servermgr.h"
#endif
#if NEXUS_HAS_FRONTEND
#include "tuner_qam.h"
#include "tuner_vsb.h"
#include "tuner_sat.h"
#include "tuner_ofdm.h"
#ifdef MPOD_SUPPORT
#include "tuner_oob.h"
#include "tuner_upstream.h"
#endif
#ifdef SNMP_SUPPORT
#include "snmp.h"
#endif
#endif /* NEXUS_HAS_FRONTEND */
#if DVR_LIB_SUPPORT
#include "tsb.h"
#endif
BDBG_MODULE(atlas_board);

CChipFamily::CChipFamily() :
    _major(0),
    _minor(0)
{
}

CChipFamily::~CChipFamily()
{
}

bool CChipFamily::operator ==(const CChipFamily &other)
{
    return((_name == other._name) && (_revision == other._revision));
}

CChipInfo::CChipInfo() :
    _number(0),
    _revision(""),
    _major(0),
    _minor(0)
{
    _familyList.clear();
}

CChipInfo::~CChipInfo()
{
}

eRet CChipInfo::setNumber(uint32_t number)
{
    eRet ret = eRet_Ok;

    _number = number;
    return(ret);
}

eRet CChipInfo::setRevision(const char * rev)
{
    eRet    ret      = eRet_Ok;
    MString revision = rev;

    if (12 != revision.length())
    {
        ret = eRet_InvalidParameter;
    }
    else
    {
        _revision = revision;
        _major    = 'A' + strtoul(revision.mid(3, 4), NULL, 10);   /* parse letter from revision */
        _minor    = strtoul(revision.right(1), NULL, 10);          /* parse number from revision */
    }

    return(ret);
} /* setRevision */

eRet CChipInfo::addFamily(CChipFamily family)
{
    CChipFamily * pChipFamily = NULL;
    eRet          ret         = eRet_Ok;

    pChipFamily = new CChipFamily(family);
    BDBG_ASSERT(pChipFamily);
    _familyList.add(pChipFamily);
    return(ret);
}

eRet CChipInfo::removeFamily(CChipFamily family)
{
    eRet          ret     = eRet_Ok;
    CChipFamily * pFamily = NULL;

    /* find and remove family from family list */
    for (pFamily = _familyList.first(); pFamily; pFamily = _familyList.next())
    {
        if (family == *pFamily)
        {
            pFamily = _familyList.remove();
            BDBG_ASSERT(pFamily);
            delete pFamily;
        }
    }

    return(ret);
} /* removeFamily */

void CChipInfo::dump()
{
    BDBG_WRN(("chip num:%d chip revisio:1n:%c%d %s",
              getNumber(),
              getMajor(),
              getMinor(),
              getRevision()));
}

bool CChipInfo::operator ==(const CChipInfo &other)
{
    return((_number == other._number) && (_revision == other._revision));
}

CBoardFeatures::CBoardFeatures()
{
    clear();
}

CBoardFeatures::~CBoardFeatures()
{
}

void CBoardFeatures::clear()
{
    _videoHd       = false;
    _displayHd     = false;
    _dnrDcr        = false;
    _dnrBnr        = false;
    _dnrMnr        = false;
    _anr           = false;
    _deinterlacer  = false;
    _boxDetect     = 0;
    _pvrEncryption = false;
    _sharpness     = false;
    _cab           = false;
    _lab           = false;
    _mosaic        = false;
    _autoVolume    = false;
    _dolbyVolume   = false;
    _srsVolume     = false;

    memset(_videoFormatIsSupported, false, NEXUS_VideoFormat_eMax);
} /* clear */

void CBoardFeatures::dump(bool bForce)
{
    BDBG_Level level;

    if (true == bForce)
    {
        BDBG_GetModuleLevel("atlas_board", &level);
        BDBG_SetModuleLevel("atlas_board", BDBG_eMsg);
    }

    BDBG_MSG(("\nBOARD FEATURES"));
    /*
     * BDBG_MSG(("videoHd:        %d", _videoHd));
     * BDBG_MSG(("displayHd:      %d", _displayHd));
     * BDBG_MSG(("dnrDcr:         %d", _dnrDcr));
     * BDBG_MSG(("dnrBnr:         %d", _dnrBnr));
     * BDBG_MSG(("dnrMnr:         %d", _dnrMnr));
     * BDBG_MSG(("anr:            %d", _anr));
     * BDBG_MSG(("deinterlacer:   %d", _deinterlacer));
     */
    BDBG_MSG(("boxDetect:      %d", _boxDetect));
    /*
     * BDBG_MSG(("pvrEncryption:  %d", _pvrEncryption));
     * BDBG_MSG(("sharpness:      %d", _sharpness));
     * BDBG_MSG(("cab:            %d", _cab));
     * BDBG_MSG(("lab:            %d", _lab));
     * BDBG_MSG(("mosaic:         %d", _mosaic));
     */
    BDBG_MSG(("autoVolume:     %d", _autoVolume));
    BDBG_MSG(("dolbyVolume:    %d", _dolbyVolume));
    BDBG_MSG(("srsVolume:      %d", _srsVolume));

    if (true == bForce)
    {
        BDBG_SetModuleLevel("atlas_board", level);
    }
} /* dump */

CBoardResources::CBoardResources()
{
    _displayList.clear();
    _mapResourceList[eBoardResource_display] = (MAutoList <CResource> *)&_displayList;
    _graphicsList.clear();
    _mapResourceList[eBoardResource_graphics] = (MAutoList <CResource> *)&_graphicsList;
    _surfaceClientList.clear();
    _mapResourceList[eBoardResource_surfaceClient] = (MAutoList <CResource> *)&_surfaceClientList;
    _decodeVideoList.clear();
    _mapResourceList[eBoardResource_decodeVideo] = (MAutoList <CResource> *)&_decodeVideoList;
    _simpleDecodeVideoList.clear();
    _mapResourceList[eBoardResource_simpleDecodeVideo] = (MAutoList <CResource> *)&_simpleDecodeVideoList;
    _decodeAudioList.clear();
    _mapResourceList[eBoardResource_decodeAudio] = (MAutoList <CResource> *)&_decodeAudioList;
    _simpleDecodeAudioList.clear();
    _mapResourceList[eBoardResource_simpleDecodeAudio] = (MAutoList <CResource> *)&_simpleDecodeAudioList;
    _stcChannelList.clear();
    _mapResourceList[eBoardResource_stcChannel] = (MAutoList <CResource> *)&_stcChannelList;
    _pcmPlaybackList.clear();
    _mapResourceList[eBoardResource_pcmPlayback] = (MAutoList <CResource> *)&_pcmPlaybackList;
    _pcmCaptureList.clear();
    _mapResourceList[eBoardResource_pcmCapture] = (MAutoList <CResource> *)&_pcmCaptureList;
    _decodeStillList.clear();
    _mapResourceList[eBoardResource_decodeStill] = (MAutoList <CResource> *)&_decodeStillList;
    _decodeEsList.clear();
    _mapResourceList[eBoardResource_decodeEs] = (MAutoList <CResource> *)&_decodeEsList;
    _decodeMosaicList.clear();
    _mapResourceList[eBoardResource_decodeMosaic] = (MAutoList <CResource> *)&_decodeMosaicList;
    _streamerList.clear();
    _mapResourceList[eBoardResource_streamer] = (MAutoList <CResource> *)&_streamerList;
#if NEXUS_HAS_FRONTEND
    _tunerList.clear();
    _mapResourceList[eBoardResource_frontendQam]      = (MAutoList <CResource> *)&_tunerList;
    _mapResourceList[eBoardResource_frontendVsb]      = (MAutoList <CResource> *)&_tunerList;
    _mapResourceList[eBoardResource_frontendSds]      = (MAutoList <CResource> *)&_tunerList;
    _mapResourceList[eBoardResource_frontendIp]       = (MAutoList <CResource> *)&_tunerList;
    _mapResourceList[eBoardResource_frontendOfdm]     = (MAutoList <CResource> *)&_tunerList;
    _mapResourceList[eBoardResource_frontendOob]      = (MAutoList <CResource> *)&_tunerList;
    _mapResourceList[eBoardResource_frontendUpstream] = (MAutoList <CResource> *)&_tunerList;
#endif /* if NEXUS_HAS_FRONTEND */
    _lineinList.clear();
    _mapResourceList[eBoardResource_linein] = (MAutoList <CResource> *)&_lineinList;
    _recpumpList.clear();
    _mapResourceList[eBoardResource_recpump] = (MAutoList <CResource> *)&_recpumpList;
    _recordList.clear();
    _mapResourceList[eBoardResource_record] = (MAutoList <CResource> *)&_recordList;
    _recordPesList.clear();
    _mapResourceList[eBoardResource_recordPes] = (MAutoList <CResource> *)&_recordPesList;
    _recordTsdmaList.clear();
    _mapResourceList[eBoardResource_recordTsdma] = (MAutoList <CResource> *)&_recordTsdmaList;
#if defined NEXUS_HAS_DMA || defined NEXUS_HAS_XPT_DMA
    _dmaList.clear();
    _mapResourceList[eBoardResource_dma] = (MAutoList <CResource> *)&_dmaList;
#endif
    _timebaseList.clear();
    _mapResourceList[eBoardResource_timebase] = (MAutoList <CResource> *)&_timebaseList;
    _playpumpList.clear();
    _mapResourceList[eBoardResource_playpump] = (MAutoList <CResource> *)&_playpumpList;
    _playbackList.clear();
    _mapResourceList[eBoardResource_playback] = (MAutoList <CResource> *)&_playbackList;
#if  DVR_LIB_SUPPORT
    _tsbList.clear();
    _mapResourceList[eBoardResource_tsb] = (MAutoList <CResource> *)&_tsbList;
#endif
#if NEXUS_HAS_VIDEO_ENCODER
    _encodeList.clear();
    _mapResourceList[eBoardResource_encode] = (MAutoList <CResource> *)&_encodeList;
#endif
    _inputBandList.clear();
    _mapResourceList[eBoardResource_inputBand] = (MAutoList <CResource> *)&_inputBandList;
    _parserBandList.clear();
    _mapResourceList[eBoardResource_parserBand] = (MAutoList <CResource> *)&_parserBandList;
    _outputSpdifList.clear();
    _mapResourceList[eBoardResource_outputSpdif] = (MAutoList <CResource> *)&_outputSpdifList;
    _outputAudioDacList.clear();
    _mapResourceList[eBoardResource_outputAudioDac] = (MAutoList <CResource> *)&_outputAudioDacList;
    _outputAudioDacI2sList.clear();
    _mapResourceList[eBoardResource_outputAudioDacI2s] = (MAutoList <CResource> *)&_outputAudioDacI2sList;
    _outputAudioDummyList.clear();
    _mapResourceList[eBoardResource_outputAudioDummy] = (MAutoList <CResource> *)&_outputAudioDummyList;
    _outputComponentList.clear();
    _mapResourceList[eBoardResource_outputComponent] = (MAutoList <CResource> *)&_outputComponentList;
    _outputSVideoList.clear();
    _mapResourceList[eBoardResource_outputSVideo] = (MAutoList <CResource> *)&_outputSVideoList;
    _outputCompositeList.clear();
    _mapResourceList[eBoardResource_outputComposite] = (MAutoList <CResource> *)&_outputCompositeList;
    _outputRFMList.clear();
    _mapResourceList[eBoardResource_outputRFM] = (MAutoList <CResource> *)&_outputRFMList;
    _outputHdmiList.clear();
    _mapResourceList[eBoardResource_outputHdmi] = (MAutoList <CResource> *)&_outputHdmiList;
    _remoteListIr.clear();
    _mapResourceList[eBoardResource_irRemote] = (MAutoList <CResource> *)&_remoteListIr;
#if RF4CE_SUPPORT
    _remoteListRf4ce.clear();
    _mapResourceList[eBoardResource_rf4ceRemote] = (MAutoList <CResource> *)&_remoteListRf4ce;
#endif
#ifdef NETAPP_SUPPORT
    _remoteListBluetooth.clear();
    _mapResourceList[eBoardResource_bluetoothRemote] = (MAutoList <CResource> *)&_remoteListBluetooth;
#endif
#if NEXUS_HAS_UHF_INPUT
    _remoteListUhf.clear();
    _mapResourceList[eBoardResource_uhfRemote] = (MAutoList <CResource> *)&_remoteListUhf;
#endif
    _powerList.clear();
    _mapResourceList[eBoardResource_power] = (MAutoList <CResource> *)&_powerList;
#ifdef WPA_SUPPLICANT_SUPPORT
    _wifiList.clear();
    _mapResourceList[eBoardResource_wifi] = (MAutoList <CResource> *)&_wifiList;
#endif
#ifdef NETAPP_SUPPORT
    _networkList.clear();
    _mapResourceList[eBoardResource_network] = (MAutoList <CResource> *)&_networkList;

    _bluetoothList.clear();
    _mapResourceList[eBoardResource_bluetooth] = (MAutoList <CResource> *)&_bluetoothList;
#endif /* ifdef NETAPP_SUPPORT */
}

CBoardResources::~CBoardResources()
{
}

void CBoardResources::clear()
{
#ifdef WPA_SUPPLICANT_SUPPORT
    _wifiList.clear();
#endif
#ifdef NETAPP_SUPPORT
    _networkList.clear();
    /* Bluetooth device most likley cleared from decontrcution of Audio Captuer which has a audio capture cleint list of bluetooth object */
    _bluetoothList.clear();
    _remoteListBluetooth.clear();
#endif /* ifdef NETAPP_SUPPORT */
    _powerList.clear();
    _remoteListIr.clear();
#if NEXUS_HAS_UHF_INPUT
    _remoteListUhf.clear();
#endif
    _outputHdmiList.clear();
    _outputRFMList.clear();
    _outputCompositeList.clear();
    _outputSVideoList.clear();
    _outputComponentList.clear();
    _outputAudioDummyList.clear();
    _outputAudioDacList.clear();
    _outputAudioDacI2sList.clear();
    _outputSpdifList.clear();
    _parserBandList.clear();
    _inputBandList.clear();
#if NEXUS_HAS_VIDEO_ENCODER
    _encodeList.clear();
#endif
    _playbackList.clear();
    _playpumpList.clear();
    _recordTsdmaList.clear();
#if defined NEXUS_HAS_DMA || defined NEXUS_HAS_XPT_DMA
    _dmaList.clear();
#endif
    _timebaseList.clear();
    _recordPesList.clear();
    _recordList.clear();
    _recpumpList.clear();
    _lineinList.clear();
#if NEXUS_HAS_FRONTEND
    _tunerList.clear();
#endif
    _streamerList.clear();
    _decodeMosaicList.clear();
    _decodeEsList.clear();
    _decodeStillList.clear();
    _pcmCaptureList.clear();
    _pcmPlaybackList.clear();
    _stcChannelList.clear();
    _simpleDecodeAudioList.clear();
    _decodeAudioList.clear();
    _simpleDecodeVideoList.clear();
    _decodeVideoList.clear();
    _surfaceClientList.clear();
    _graphicsList.clear();
    _displayList.clear();
} /* clear */

#if NEXUS_HAS_FRONTEND
/* frontend resources may be added multiple times for a given tuner.  for example, if a tuner can do both vsb and
 * qam, it will be added once as a vsb frontend and once as a qam frontend.  the num used for each tuner will
 * be identical, and checking out either tuner will automatically check out the other.  This is done because
 * only a specific tuning object knows how to tune itself - so even if a tuner can be either qam or vsb, the
 * object that represents it can only be one or the other. */
eRet CBoardResources::addFrontend(
        const unsigned               numTuner,
        CConfiguration *             pCfg,
        NEXUS_FrontendCapabilities * pCapabilities
        )
{
    eRet                       ret    = eRet_Ok;
    CTuner *                   pTuner = NULL;
    CPlatform *                pPlatformConfig;
    NEXUS_FrontendCapabilities capabilities;

    BDBG_ASSERT(NULL != pCfg);
    pPlatformConfig = pCfg->getPlatformConfig();

    if (NULL == pCapabilities)
    {
        /* no capabilities given - discover them here */
        CHECK_PTR_ERROR_GOTO("invalid tuner number parameter", pPlatformConfig->getFrontend(numTuner), ret, eRet_InvalidParameter, error);
        NEXUS_Frontend_GetCapabilities(pPlatformConfig->getFrontend(numTuner), &capabilities);
    }
    else
    {
        /* use given capabilities */
        capabilities = *pCapabilities;
    }

    if (true == capabilities.analog)
    {
    }

    if (true == capabilities.qam)
    {
        pTuner = new CTunerQam("tunerQam", numTuner, pCfg);
        CHECK_PTR_ERROR_GOTO("unable to allocate tuner", pTuner, ret, eRet_OutOfMemory, error);

        _tunerList.add(pTuner);
    }

    if (true == capabilities.vsb)
    {
        pTuner = new CTunerVsb("tunerVsb", numTuner, pCfg);
        CHECK_PTR_ERROR_GOTO("unable to allocate tuner", pTuner, ret, eRet_OutOfMemory, error);

        _tunerList.add(pTuner);
    }

    if (true == capabilities.satellite)
    {
        pTuner = new CTunerSat("tunerSat", numTuner, pCfg);
        CHECK_PTR_ERROR_GOTO("unable to allocate tuner", pTuner, ret, eRet_OutOfMemory, error);

        _tunerList.add(pTuner);
    }

    if (true == capabilities.ofdm)
    {
        pTuner = new CTunerOfdm("tunerOfdm", numTuner, pCfg);
        CHECK_PTR_ERROR_GOTO("unable to allocate tuner", pTuner, ret, eRet_OutOfMemory, error);

        _tunerList.add(pTuner);
    }
#ifdef MPOD_SUPPORT
    if (true == capabilities.outOfBandModes[NEXUS_FrontendOutOfBandMode_eDvs178Qpsk])
    {
        CCablecard * pCableCard = NULL;
        pCableCard = new CCablecard;
        pTuner     = new CTunerOob("tunerOob", numTuner, pCfg);
        CHECK_PTR_ERROR_GOTO("unable to allocate tuner", pTuner, ret, eRet_OutOfMemory, error);
        /*pass oob tuner object to CCablecard*/
        pCableCard->initialize((CTunerOob *)pTuner);
        _tunerList.add(pTuner);
        delete pCableCard;
#ifdef SNMP_SUPPORT
        CSnmp * pSnmp = NULL;
        pSnmp = new CSnmp;
        /* pass oob tuner object to CSnmp */
        pSnmp->snmp_save_oob((CTunerOob *)pTuner);
        delete pSnmp;
#endif /* ifdef SNMP_SUPPORT */
    }

    if (true == capabilities.upstreamModes[NEXUS_FrontendUpstreamMode_ePodDvs178])
    {
        CCablecard * pCableCard = NULL;
        pCableCard = new CCablecard;
        pTuner     = new CTunerUpstream("tunerUpstream", numTuner, pCfg);
        CHECK_PTR_ERROR_GOTO("unable to allocate tuner", pTuner, ret, eRet_OutOfMemory, error);
        /*pass upstream tuner object to CCablecard*/
        pCableCard->initializeUpstream((CTunerUpstream *)pTuner);
        _tunerList.add(pTuner);
        delete pCableCard;
#ifdef SNMP_SUPPORT
        CSnmp * pSnmp = NULL;
        pSnmp = new CSnmp;
        /* pass upstream tuner object to CSnmp */
        pSnmp->snmp_save_upstream((CTunerUpstream *) pTuner);
        delete pSnmp;
#endif /* ifdef SNMP_SUPPORT */
    }
#endif /* ifdef MPOD_SUPPORT */

    if (NULL != pTuner)
    {
        BDBG_MSG(("adding Tuner #%d (%s)", numTuner, pTuner->getFrontendId().s()));
        BDBG_MSG(("    analog:    %d", capabilities.analog));
        BDBG_MSG(("    ifd:       %d", capabilities.ifd));
        BDBG_MSG(("    qam:       %d", capabilities.qam));
        BDBG_MSG(("    vsb:       %d", capabilities.vsb));
        BDBG_MSG(("    satellite: %d", capabilities.satellite));
        BDBG_MSG(("    outOfBand: %d", capabilities.outOfBand));
        BDBG_MSG(("    upstream:  %d", capabilities.upstream));
        BDBG_MSG(("    ofdm:      %d", capabilities.ofdm));
        BDBG_MSG(("    scan:      %d", capabilities.scan));
    }

error:
    return(ret);
} /* addFrontend */

#endif /* if NEXUS_HAS_FRONTEND */

/* add a number of resources to associated list */
eRet CBoardResources::add(
        eBoardResource   resource,
        const unsigned   numResources,
        const char *     name,
        CConfiguration * pCfg,
        const unsigned   startIndex,
        const unsigned   id
        )
{
    eRet        ret             = eRet_Ok;
    CPlatform * pPlatformConfig = NULL;

    BDBG_ASSERT(NULL != pCfg);
    pPlatformConfig = pCfg->getPlatformConfig();

    for (unsigned i = startIndex; i < (startIndex + numResources); i++)
    {
        switch (resource)
        {
        case eBoardResource_display:
            /* skip unsupported displays */
            if (true == pPlatformConfig->isSupportedDisplay(i))
            {
                CDisplay * pDisplay = NULL;
                pDisplay = new CDisplay(name, (id && (id != i)) ? id : i, pCfg);
                BDBG_ASSERT(pDisplay);
                _displayList.add(pDisplay);
            }
            break;

        case eBoardResource_graphics:
            /* skip unsupported graphics */
            if (true == pPlatformConfig->isSupportedDisplay(i))
            {
                CGraphics * pGraphics = NULL;
                pGraphics = new CGraphics(name, (id && (id != i)) ? id : i, pCfg);
                BDBG_ASSERT(pGraphics);
                _graphicsList.add(pGraphics);
            }
            break;

        case eBoardResource_surfaceClient:
        {
            CSurfaceClient * pSurfaceClient = NULL;
            pSurfaceClient = new CSurfaceClient(name, (id && (id != i)) ? id : i, pCfg);
            BDBG_ASSERT(pSurfaceClient);
            _surfaceClientList.add(pSurfaceClient);
        }
        break;

        case eBoardResource_decodeVideo:
            /* skip unsupported decoders */
            if (true == pPlatformConfig->isSupportedDecoder(i))
            {
                CVideoDecode * pVideoDecode = NULL;
                pVideoDecode = new CVideoDecode(name, (id && (id != i)) ? id : i, pCfg);
                BDBG_ASSERT(pVideoDecode);
                _decodeVideoList.add(pVideoDecode);
            }
            break;

        case eBoardResource_simpleDecodeVideo:
            /* skip unsupported decoders */
            if (true == pPlatformConfig->isSupportedDecoder(i))
            {
                CSimpleVideoDecode * pSimpleVideoDecode = NULL;
                pSimpleVideoDecode = new CSimpleVideoDecode(name, (id && (id != i)) ? id : i, pCfg);
                BDBG_ASSERT(pSimpleVideoDecode);
                _simpleDecodeVideoList.add(pSimpleVideoDecode);
            }
            break;

        case eBoardResource_decodeStill:
            /* skip unsupported decoders */
            if (true == pPlatformConfig->isSupportedStillDecoder(i))
            {
                CStillDecode * pStillDecode = NULL;
                pStillDecode = new CStillDecode(name, (id && (id != i)) ? id : i, pCfg);
                BDBG_ASSERT(pStillDecode);
                _decodeStillList.add(pStillDecode);
            }
            break;

        case eBoardResource_decodeAudio:
            /* skip unsupported decoders */
        {
            CAudioDecode * pAudioDecode = NULL;
            pAudioDecode = new CAudioDecode(name, (id && (id != i)) ? id : i, pCfg);
            BDBG_ASSERT(pAudioDecode);
            _decodeAudioList.add(pAudioDecode);
        }
        break;

        case eBoardResource_simpleDecodeAudio:
            /* skip unsupported decoders */
        {
            CSimpleAudioDecode * pSimpleAudioDecode = NULL;
            pSimpleAudioDecode = new CSimpleAudioDecode(name, (id && (id != i)) ? id : i, pCfg);
            BDBG_ASSERT(pSimpleAudioDecode);
            _simpleDecodeAudioList.add(pSimpleAudioDecode);
        }
        break;

        case eBoardResource_stcChannel:
        {
            CStc * pStc = NULL;
            pStc = new CStc(name, (id && (id != i)) ? id : i, pCfg);
            BDBG_ASSERT(pStc);
            _stcChannelList.add(pStc);
        }
        break;

#if NEXUS_HAS_FRONTEND
        case eBoardResource_frontendQam:
        case eBoardResource_frontendVsb:
        case eBoardResource_frontendSds:
        case eBoardResource_frontendIp:
        case eBoardResource_frontendOfdm:
        case eBoardResource_frontendOob:
        case eBoardResource_frontendUpstream:
        {
            BDBG_ERR(("use addFrontend() method for adding tuners."));
            BDBG_ASSERT(false);
        }
        break;
#endif /* if NEXUS_HAS_FRONTEND */

        case eBoardResource_recpump:
        {
            CRecpump * pRecpump = NULL;
            pRecpump = new CRecpump(name, (id && (id != i)) ? id : i, pCfg);
            BDBG_ASSERT(pRecpump);
            _recpumpList.add(pRecpump);
        }
        break;

        case eBoardResource_record:
        {
            CRecord * pRecord = NULL;
            pRecord = new CRecord(name, (id && (id != i)) ? id : i, pCfg);
            BDBG_ASSERT(pRecord);
            _recordList.add(pRecord);
        }
        break;

#if defined NEXUS_HAS_DMA || defined NEXUS_HAS_XPT_DMA
        case eBoardResource_dma:
        {
            CDma * dma = NULL;
            dma = new CDma(name, (id && (id != i)) ? id : i, pCfg);
            BDBG_ASSERT(dma);
            _dmaList.add(dma);
        }
        break;
#endif /* if defined NEXUS_HAS_DMA || defined NEXUS_HAS_XPT_DMA */
        case eBoardResource_timebase:
        {
            CTimebase * timebase = NULL;
            timebase = new CTimebase(name, (id && (id != i)) ? id : i, pCfg);
            BDBG_ASSERT(timebase);
            _timebaseList.add(timebase);
        }
        break;

        case eBoardResource_playpump:
        {
            CPlaypump * pPlaypump = NULL;
            pPlaypump = new CPlaypump(name, (id && (id != i)) ? id : i, pCfg);
            BDBG_ASSERT(pPlaypump);
            _playpumpList.add(pPlaypump);
        }
        break;
#if DVR_LIB_SUPPORT
        case eBoardResource_tsb:
        {
            CTsb * pTsb = NULL;
            pTsb = new CTsb(name, (id && (id != i)) ? id : i, pCfg);
            BDBG_ASSERT(pTsb);
            _tsbList.add(pTsb);
        }
        break;
#endif /* if DVR_LIB_SUPPORT */
        case eBoardResource_playback:
        {
            CPlayback * pPlayback = NULL;
            pPlayback = new CPlayback(name, (id && (id != i)) ? id : i, pCfg);
            BDBG_ASSERT(pPlayback);
            _playbackList.add(pPlayback);
        }
        break;
#if NEXUS_HAS_VIDEO_ENCODER
        case eBoardResource_encode:
            /* skip unsupported decoders */
            if (true == pPlatformConfig->isSupportedEncoder(i))
            {
                CEncode * pEncode = NULL;
                pEncode = new CEncode(name, (id && (id != i)) ? id : i, pCfg);
                BDBG_ASSERT(pEncode);
                _encodeList.add(pEncode);
            }
            break;
#endif /* if NEXUS_HAS_VIDEO_ENCODER */

        case eBoardResource_inputBand:
        {
            CInputBand *    pInputBand     = NULL;
            NEXUS_InputBand inputBandIndex = 0;
            NEXUS_Error     nerror         = NEXUS_SUCCESS;

            pInputBand = new CInputBand(name, (id && (id != i)) ? id : i, pCfg);
            BDBG_ASSERT(pInputBand);

            nerror = NEXUS_Platform_GetStreamerInputBand(0, &inputBandIndex);
            if (NEXUS_SUCCESS == nerror)
            {
                if (i == inputBandIndex)
                {
                    /* this is a streamer input band so allow it to be checked out multiple times */
                    pInputBand->setCheckedOutMax(32);
                }
            }

            _inputBandList.add(pInputBand);
        }
        break;

        case eBoardResource_parserBand:
        {
            CParserBand * pParserBand = NULL;
            pParserBand = new CParserBand(name, (id && (id != i)) ? id : i, pCfg);
            BDBG_ASSERT(pParserBand);
            _parserBandList.add(pParserBand);
        }
        break;

        case eBoardResource_outputSpdif:
        {
            COutputSpdif * pOutputSpdif = NULL;
            pOutputSpdif = new COutputSpdif(name, (id && (id != i)) ? id : i, pCfg);
            BDBG_ASSERT(pOutputSpdif);
            _outputSpdifList.add(pOutputSpdif);
        }
        break;

        case eBoardResource_outputAudioDac:
        {
            COutputAudioDac * pOutputAudioDac = NULL;
            pOutputAudioDac = new COutputAudioDac(name, (id && (id != i)) ? id : i, eBoardResource_outputAudioDac, pCfg);
            BDBG_ASSERT(pOutputAudioDac);
            _outputAudioDacList.add(pOutputAudioDac);
        }
        break;

        case eBoardResource_outputAudioDacI2s:
        {
            COutputAudioDacI2s * pOutputAudioDacI2s = NULL;
            pOutputAudioDacI2s = new COutputAudioDacI2s(name, (id && (id != i)) ? id : i, pCfg);
            BDBG_ASSERT(pOutputAudioDacI2s);
            _outputAudioDacI2sList.add(pOutputAudioDacI2s);
        }
        break;

        case eBoardResource_outputAudioDummy:
        {
            COutputAudioDummy * pOutputAudioDummy = NULL;
            pOutputAudioDummy = new COutputAudioDummy(name, (id && (id != i)) ? id : i, pCfg);
            BDBG_ASSERT(pOutputAudioDummy);
            _outputAudioDummyList.add(pOutputAudioDummy);
        }
        break;

        case eBoardResource_outputComponent:
        {
            COutputComponent * pOutputComponent = NULL;
            pOutputComponent = new COutputComponent(name, (id && (id != i)) ? id : i, pCfg);
            BDBG_ASSERT(pOutputComponent);
            _outputComponentList.add(pOutputComponent);
        }
        break;

        case eBoardResource_outputSVideo:
        {
            COutputSVideo * pOutputSVideo = NULL;
            pOutputSVideo = new COutputSVideo(name, (id && (id != i)) ? id : i, pCfg);
            BDBG_ASSERT(pOutputSVideo);
            _outputSVideoList.add(pOutputSVideo);
        }
        break;

        case eBoardResource_outputComposite:
        {
            COutputComposite * pOutputComposite = NULL;
            pOutputComposite = new COutputComposite(name, (id && (id != i)) ? id : i, pCfg);
            BDBG_ASSERT(pOutputComposite);
            _outputCompositeList.add(pOutputComposite);
        }
        break;

        case eBoardResource_outputRFM:
        {
#if NEXUS_HAS_RFM
            if (pPlatformConfig->getOutputRFM((id && (id != i)) ? id : i) != NULL)
            {
                COutputRFM * pOutputRFM = NULL;
                pOutputRFM = new COutputRFM(name, (id && (id != i)) ? id : i, pCfg);
                BDBG_ASSERT(pOutputRFM);
                _outputRFMList.add(pOutputRFM);
            }
#endif /* if NEXUS_HAS_RFM */
        }
        break;

        case eBoardResource_outputHdmi:
        {
            COutputHdmi * pOutputHdmi = NULL;
            pOutputHdmi = new COutputHdmi(name, (id && (id != i)) ? id : i, pCfg);
            BDBG_ASSERT(pOutputHdmi);
            _outputHdmiList.add(pOutputHdmi);
        }
        break;

        case eBoardResource_irRemote:
        {
            CIrRemote * pIrRemote = NULL;
            pIrRemote = new CIrRemote(name, (id && (id != i)) ? id : i, pCfg);
            BDBG_ASSERT(pIrRemote);
            _remoteListIr.add(pIrRemote);
        }
        break;

#if RF4CE_SUPPORT
        case eBoardResource_rf4ceRemote:
        {
            CRf4ceRemote * pRf4ceRemote = NULL;
            pRf4ceRemote = new CRf4ceRemote(name, (id && (id != i)) ? id : i, pCfg);
            BDBG_ASSERT(pRf4ceRemote);
            _remoteListRf4ce.add(pRf4ceRemote);
        }
        break;
#endif /* if RF4CE_SUPPORT */
#ifdef NETAPP_SUPPORT
        case eBoardResource_bluetoothRemote:
        {
            CBluetoothRemote * pBluetoothRemote = NULL;
            pBluetoothRemote = new CBluetoothRemote(name, i, pCfg);
            BDBG_ASSERT(pBluetoothRemote);
            _remoteListBluetooth.add(pBluetoothRemote);
        }
        break;
#endif /* ifdef NETAPP_SUPPORT */

#if NEXUS_HAS_UHF_INPUT
        case eBoardResource_uhfRemote:
        {
            CUhfRemote * pUhfRemote = NULL;
            pUhfRemote = new CUhfRemote(name, (id && (id != i)) ? id : i, pCfg);
            BDBG_ASSERT(pUhfRemote);
            _remoteListUhf.add(pUhfRemote);
        }
        break;
#endif /* if NEXUS_HAS_UHF_INPUT */

        case eBoardResource_power:
        {
            CPower * pPower = NULL;
            pPower = new CPower(name, (id && (id != i)) ? id : i, pCfg);
            BDBG_ASSERT(pPower);
            /* this is a singleton power object so allow it to be checked out multiple times */
            pPower->setCheckedOutMax(-1);
            _powerList.add(pPower);
        }
        break;
#ifdef WPA_SUPPLICANT_SUPPORT
        case eBoardResource_wifi:
        {
            CWifi * pWifi = NULL;
            pWifi = new CWifi(name, (id && (id != i)) ? id : i, pCfg);
            BDBG_ASSERT(pWifi);
            /* this is a singleton Wifi object so allow it to be checked out multiple times */
            pWifi->setCheckedOutMax(-1);
            _wifiList.add(pWifi);
        }
        break;
#endif /* ifdef WPA_SUPPLICANT_SUPPORT */
#ifdef NETAPP_SUPPORT
        case eBoardResource_network:
        {
            CNetwork * pNetwork = NULL;
            pNetwork = new CNetwork(name, (id && (id != i)) ? id : i, pCfg);
            BDBG_ASSERT(pNetwork);
            /* this is a singleton network object so allow it to be checked out multiple times */
            pNetwork->setCheckedOutMax(-1);
            _networkList.add(pNetwork);
        }
        break;
        case eBoardResource_bluetooth:
        {
            CBluetooth * pBluetooth = NULL;
            pBluetooth = new CBluetooth(name, i, pCfg);
            BDBG_ASSERT(pBluetooth);
            /* this is a singleton network object so allow it to be checked out multiple times */
            pBluetooth->setCheckedOutMax(-1);
            _bluetoothList.add(pBluetooth);
        }
        break;
#endif /* ifdef NETAPP_SUPPORT */

        case eBoardResource_streamer:
        {
            CResource * pStreamer = NULL;
            pStreamer = new CResource(name, (id && (id != i)) ? id : i, eBoardResource_streamer, pCfg);
            BDBG_ASSERT(pStreamer);
            _streamerList.add(pStreamer);
        }
        break;

        case eBoardResource_pcmPlayback:
        case eBoardResource_pcmCapture:
        case eBoardResource_decodeEs:
        case eBoardResource_decodeMosaic:
        case eBoardResource_linein:
        case eBoardResource_recordPes:
        case eBoardResource_recordTsdma:
        default:
            BDBG_WRN(("Warning: invalid resource add attempt - skipping resource:%s", boardResourceToString(resource).s()));
            BDBG_ASSERT(0);
            break;
        } /* switch */
    }

    return(ret);
} /* add */

/* find resource for a given id - if index == ANY_INDEX then find first available matching resource.
 * this method only indicates the resource exists for the given id.  to reteive the resource you must
 * reserve it (see reserveResource()) */
bool CBoardResources::findResource(
        void *         id,
        eBoardResource resource,
        unsigned       index
        )
{
    CResource * pResource = NULL;

    MList <CResource> * pResourceList = NULL;
    bool                bFound        = false;

    BDBG_ASSERT(eBoardResource_max > resource);
    BDBG_ASSERT(0 <= resource);
    BDBG_ASSERT(NULL != id);

    pResourceList = _mapResourceList[resource];
    MListItr <CResource> itr(pResourceList);

    if (ANY_INDEX == index)
    {
        /* no index specified, so search for first matching resource */
        for (pResource = itr.first(); pResource; pResource = itr.next())
        {
            if (pResource->getType() == resource)
            {
                /* found matching resource type */
                if (true == pResource->validReservation(id))
                {
                    /* not reserved for someone else */
                    bFound = true;
                    break;
                }
            }
        }
    }
    else
    {
        /* index is given so see if it exists for given id */
        pResource = itr.at(index);
        if ((NULL != pResource) && (pResource->getType() == resource))
        {
            /* resource type matches */
            if (true == pResource->validReservation(id))
            {
                /* found and not reserved */
                bFound = true;
            }
        }
    }

    return(bFound);
} /* findResource */

/* reserve resource for a given id - if index == ANY_INDEX then reserve first available matching resource */
CResource * CBoardResources::reserveResource(
        void *         id,
        eBoardResource resource,
        unsigned       index
        )
{
    CResource * pResource         = NULL;
    CResource * pReservedResource = NULL;

    MList <CResource> * pResourceList = NULL;

    BDBG_ASSERT(eBoardResource_max > resource);
    BDBG_ASSERT(0 <= resource);
    BDBG_ASSERT(NULL != id);

    pResourceList = _mapResourceList[resource];
    MListItr <CResource> itr(pResourceList);

    if (ANY_INDEX == index)
    {
        /* no index specified, so search for first matching resource */
        for (pResource = itr.first(); pResource; pResource = itr.next())
        {
            if (pResource->getType() == resource)
            {
                /* found matching resource type */
                if ((false == pResource->isCheckedOut()) && (false == pResource->isReserved()))
                {
                    /* we can only reserve resources that are not already checked out or not already reserved*/
                    pReservedResource = pResource;
                    pReservedResource->setReserved(id);
                    break;
                }
            }
        }
    }
    else
    {
        /* index is given so try to reserve it */
        pResource = itr.at(index);
        BDBG_ASSERT(NULL != pResource);
        if (pResource->getType() == resource)
        {
            /* resource type matches */
            if ((false == pResource->isCheckedOut()) && (false == pResource->isReserved()))
            {
                /* we can only reserve resources that are not already checked out or not already reserved*/
                pReservedResource = pResource;
                pReservedResource->setReserved(id);
            }
        }
    }

#if NEXUS_HAS_FRONTEND
    if (NULL != pReservedResource)
    {
        /* special handling for frontends */
        if (pReservedResource->isFrontend())
        {
            /* we've reserved a frontend - now we must check out any other frontends with the same resource
             * number since they correspond to the same physical frontend. */
            for (pResource = itr.first(); pResource; pResource = itr.next())
            {
                if (pResource->isFrontend())
                {
                    /* found frontend. */
                    if ((false == pResource->isCheckedOut()) && (false == pResource->isReserved()))
                    {
                        /* the found resource is not checked out and is not reserved for someone else */
                        if (pReservedResource->getNumber() == pResource->getNumber())
                        {
                            /* tuner number matches reserved tuner number so this tuner also corresponds to the
                             * same physical tuner - reserve it, too */
                            pResource->setReserved(id);
                        }
                    }
                }
            }
        }
    }
#endif /* if NEXUS_HAS_FRONTEND */

    return(pReservedResource);
} /* reserveResource */

/* find given already checked out resource - if index == ANY_INDEX then checkout first available matching resource */
CResource * CBoardResources::findCheckedoutResource(
        void *         id,
        eBoardResource resource,
        unsigned       index
        )
{
    CResource * pResource           = NULL;
    CResource * pCheckedOutResource = NULL;

    MList <CResource> * pResourceList = NULL;

    BDBG_ASSERT(eBoardResource_max > resource);
    BDBG_ASSERT(0 <= resource);

    pResourceList = _mapResourceList[resource];
    MListItr <CResource> itr(pResourceList);

    if (ANY_INDEX == index)
    {
        /* no index specified, so search for first matching resource */
        for (pResource = itr.first(); pResource; pResource = itr.next())
        {
            if (pResource->getType() == resource)
            {
                /* found matching resource type */
                if ((true == pResource->isCheckedOut()) && (true == pResource->validReservation(id)))
                {
                    /* found resource is not checked out and is not reserved for someone else
                     * so check it out */
                    pCheckedOutResource = pResource;
                    break;
                }
            }
        }
    }
    else
    {
        /* index is given so try to check out */
        pResource = itr.at(index);
        BDBG_ASSERT(NULL != pResource);
        if (pResource->getType() == resource)
        {
            /* resource type matches*/
            if ((true == pResource->isCheckedOut()) && (true == pResource->validReservation(id)))
            {
                /* found resource is not checked out and is not reserved for someone else
                 * so check it out */
                pCheckedOutResource = pResource;
            }
        }
    }

    if (NULL != pCheckedOutResource)
    {
        BDBG_ASSERT(resource == pCheckedOutResource->getType());
    }

    return(pCheckedOutResource);
} /* findCheckedoutResource */

/* registers the given observer for the given resource type.  if index == ANY_INDEX, the observer will
 * be registered for all resources matching both id and resource type. */
eRet CBoardResources::registerObserver(
        void *         id,
        eBoardResource resource,
        CObserver *    pObserver,
        unsigned       resourceIndex,
        eNotification  notification
        )
{
    eRet        ret       = eRet_NotAvailable; /* assume not found */
    CResource * pResource = NULL;

    MList <CResource> * pResourceList = NULL;

    BDBG_ASSERT(eBoardResource_max > resource);
    BDBG_ASSERT(0 <= resource);
    BDBG_ASSERT(NULL != id);
    BDBG_ASSERT(NULL != pObserver);

    pResourceList = _mapResourceList[resource];
    MListItr <CResource> itr(pResourceList);

    if (ANY_INDEX == resourceIndex)
    {
        /* no resourceIndex specified, so search for all matching resource */
        for (pResource = itr.first(); pResource; pResource = itr.next())
        {
            if ((pResource->getType() == resource) && (pResource->validReservation(id)))
            {
                /* found matching resource type and matching id */
                ret = pResource->registerObserver(pObserver, notification);
            }
        }
    }
    else
    {
        /* resourceIndex is given so try to reserve it */
        pResource = itr.at(resourceIndex);
        BDBG_ASSERT(NULL != pResource);
        if ((pResource->getType() == resource) && (pResource->validReservation(id)))
        {
            /* found matching resource type and matching id */
            ret = pResource->registerObserver(pObserver, notification);
        }
    }

    return(ret);
} /* registerObserver */

/* unregisters the given observer for the given resource type.  if index == ANY_INDEX, the observer will
 * be unregistered for all resources matching both id and resource type. */
eRet CBoardResources::unregisterObserver(
        void *         id,
        eBoardResource resource,
        CObserver *    pObserver,
        unsigned       resourceIndex,
        eNotification  notification
        )
{
    eRet        ret       = eRet_NotAvailable; /* assume not found */
    CResource * pResource = NULL;

    MList <CResource> * pResourceList = NULL;

    BDBG_ASSERT(eBoardResource_max > resource);
    BDBG_ASSERT(0 <= resource);
    BDBG_ASSERT(NULL != id);
    BDBG_ASSERT(NULL != pObserver);

    pResourceList = _mapResourceList[resource];
    MListItr <CResource> itr(pResourceList);

    if (ANY_INDEX == resourceIndex)
    {
        /* no resourceIndex specified, so search for all matching resource */
        for (pResource = itr.first(); pResource; pResource = itr.next())
        {
            if ((pResource->getType() == resource) && (pResource->validReservation(id)))
            {
                /* found matching resource type and matching id */
                ret = pResource->unregisterObserver(pObserver, notification);
            }
        }
    }
    else
    {
        /* resourceIndex is given so try to reserve it */
        pResource = itr.at(resourceIndex);
        BDBG_ASSERT(NULL != pResource);
        if ((pResource->getType() == resource) && (pResource->validReservation(id)))
        {
            /* found matching resource type and matching id */
            ret = pResource->unregisterObserver(pObserver, notification);
        }
    }

    return(ret);
} /* unregisterObserver */

/* check out given resource - if index == ANY_INDEX then checkout first available matching resource */
CResource * CBoardResources::checkoutResource(
        void *         id,
        eBoardResource resource,
        unsigned       index,
        uint32_t       number
        )
{
    eRet        ret                 = eRet_Ok;
    CResource * pResource           = NULL;
    CResource * pCheckedOutResource = NULL;

    MList <CResource> * pResourceList = NULL;

    BDBG_ASSERT(eBoardResource_max > resource);
    BDBG_ASSERT(0 <= resource);

    pResourceList = _mapResourceList[resource];
    MListItr <CResource> itr(pResourceList);

    if (ANY_INDEX == index)
    {
        /* no index specified, so search for first matching resource */
        for (pResource = itr.first(); pResource; pResource = itr.next())
        {
            if ((pResource->getType() == resource))
            {
                /* found matching resource type OR we are checking out a frontend. */
                if ((ANY_NUMBER == number) || (pResource->getNumber() == number))
                {
                    if ((true == pResource->isAvailForCheckout()) && (true == pResource->validReservation(id)))
                    {
                        /* the found resource is not checked out and is not reserved for someone else
                         * so check it out */
                        pCheckedOutResource = pResource;
                        ret                 = pCheckedOutResource->setCheckedOut(true, id);
                        CHECK_ERROR_GOTO("unable to checkout resource", ret, error);

                        break;
                    }
                }
            }
        }
    }
    else
    {
        /* index is given so try to check out */
        pResource = itr.at(index);
        CHECK_PTR_WARN_GOTO("requested resource does not exist", pResource, ret, eRet_NotAvailable, error);
        if (pResource->getType() == resource)
        {
            /* resource type matches*/
            if ((ANY_NUMBER == number) || (pResource->getNumber() == number))
            {
                if ((true == pResource->isAvailForCheckout()) && (true == pResource->validReservation(id)))
                {
                    /* found resource is not checked out and is not reserved for someone else
                     * so check it out */
                    pCheckedOutResource = pResource;
                    ret                 = pCheckedOutResource->setCheckedOut(true, id);
                    CHECK_ERROR_GOTO("unable to checkout resource", ret, error);
                }
            }
        }
    }

    CHECK_PTR_MSG_GOTO("resource unavailable for checkout", pCheckedOutResource, ret, eRet_NotAvailable, error);
    BDBG_ASSERT(resource == pCheckedOutResource->getType());

#if NEXUS_HAS_FRONTEND
    if (NULL != pCheckedOutResource)
    {
        /* special handling for frontends */
        if (pCheckedOutResource->isFrontend())
        {
            /* we've checked out a frontend - now we must check out any other frontends with the same resource
             * number since they correspond to the same physical frontend. */
            for (pResource = itr.first(); pResource; pResource = itr.next())
            {
                if (pResource->isFrontend())
                {
                    /* found frontend. */
                    if ((true == pResource->isAvailForCheckout()) && (true == pResource->validReservation(id)))
                    {
                        /* the found resource is not checked out and is not reserved for someone else */
                        if (pCheckedOutResource->getNumber() == pResource->getNumber())
                        {
                            /* frontend number matches found frontend number so this frontend also corresponds to the
                             * same physical frontend - check it out, too */
                            ret = pResource->setCheckedOut(true, id);
                            CHECK_ERROR_GOTO("unable to checkout resource", ret, error);
                        }
                    }
                }
            }
        }
    }
#endif /* if NEXUS_HAS_FRONTEND */

    goto done;
error:
    BDBG_MSG(("resource type: %s(%d)", boardResourceToString(resource).s(), resource));
done:
    return(pCheckedOutResource);
} /* checkoutResource */

eRet CBoardResources::checkinResource(CResource * pResource)
{
    eRet        ret                = eRet_Ok;
    CResource * pCheckedInResource = NULL;

    BDBG_ASSERT(NULL != pResource);

    if (false == pResource->isCheckedOut())
    {
        /* already checked in */
        return(ret);
    }

    ret = pResource->setCheckedOut(false);
    CHECK_ERROR_GOTO("unable to checkin resource", ret, error);

    pCheckedInResource = pResource;

#if NEXUS_HAS_FRONTEND
    /* special handling for frontends */
    if (pCheckedInResource->isFrontend())
    {
        CResource *         pResourceFound = NULL;
        MList <CResource> * pResourceList  = NULL;

        pResourceList = _mapResourceList[pCheckedInResource->getType()];

        MListItr <CResource> itr(pResourceList);

        /* we've checked in a frontend - now we must check in any other frontends with the same resource
         * number since they correspond to the same physical frontend. */
        for (pResourceFound = itr.first(); pResourceFound; pResourceFound = itr.next())
        {
            if (pResourceFound->isFrontend())
            {
                /* found frontend. */
                if (true == pResourceFound->isCheckedOut())
                {
                    /* the found resource is checked out */
                    if (pCheckedInResource->getNumber() == pResourceFound->getNumber())
                    {
                        /* frontend number matches found frontend number so this frontend also corresponds to the
                         * same physical frontend - check it in, too */
                        ret = pResourceFound->setCheckedOut(false);
                        CHECK_ERROR_GOTO("unable to checkin resource", ret, error);
                    }
                }
            }
        }
    }
#endif /* if NEXUS_HAS_FRONTEND */

error:
    return(ret);
} /* checkinResource */

void CBoardResources::dumpList(MList <CResource> * pList)
{
    MListItr <CResource> itr(pList);
    CResource *          pResource = NULL;

    for (pResource = itr.first(); pResource; pResource = itr.next())
    {
        pResource->dump();
    }
}

void CBoardResources::dump(bool bForce)
{
    BDBG_Level level;

    if (true == bForce)
    {
        BDBG_GetModuleLevel("atlas_board", &level);
        BDBG_SetModuleLevel("atlas_board", BDBG_eMsg);
    }

    BDBG_MSG(("\nRESOURCES"));
    BDBG_MSG(("========="));
    BDBG_MSG(("displays:                    %d", _displayList.total()));
    dumpList((MList <CResource> *)&_displayList);
    BDBG_MSG(("decodes (video):             %d", _decodeVideoList.total()));
    dumpList((MList <CResource> *)&_decodeVideoList);
    BDBG_MSG(("decodes (audio):             %d", _decodeAudioList.total()));
    dumpList((MList <CResource> *)&_decodeAudioList);
    BDBG_MSG(("decodes (simple video):      %d", _simpleDecodeVideoList.total()));
    dumpList((MList <CResource> *)&_simpleDecodeVideoList);
    BDBG_MSG(("decodes (simple audio):      %d", _simpleDecodeAudioList.total()));
    dumpList((MList <CResource> *)&_simpleDecodeAudioList);
    BDBG_MSG(("stc channels:                %d", _stcChannelList.total()));
    dumpList((MList <CResource> *)&_stcChannelList);
    BDBG_MSG(("PCM playbacks:               %d", _pcmPlaybackList.total()));
    dumpList(&_pcmPlaybackList);
    BDBG_MSG(("PCM captures:                %d", _pcmCaptureList.total()));
    dumpList(&_pcmCaptureList);
    BDBG_MSG(("decodes (still picture):     %d", _decodeStillList.total()));
    dumpList(&_decodeStillList);
    BDBG_MSG(("decodes (ES):                %d", _decodeEsList.total()));
    dumpList(&_decodeEsList);
    BDBG_MSG(("decodes (mosaic):            %d", _decodeMosaicList.total()));
    dumpList(&_decodeMosaicList);
    BDBG_MSG(("streamers:                   %d", _streamerList.total()));
    dumpList(&_streamerList);
#if NEXUS_HAS_FRONTEND
    BDBG_MSG(("tuners:                      %d", _tunerList.total()));
    dumpList((MList <CResource> *)&_tunerList);
#endif
    BDBG_MSG(("line-ins:                    %d", _lineinList.total()));
    dumpList(&_lineinList);
    BDBG_MSG(("recpumps:                    %d", _recpumpList.total()));
    dumpList((MList <CResource> *)&_recpumpList);
    BDBG_MSG(("records:                     %d", _recordList.total()));
    dumpList((MList <CResource> *)&_recordList);
    BDBG_MSG(("PES records:                 %d", _recordPesList.total()));
    dumpList(&_recordPesList);
    BDBG_MSG(("TSDMA records:               %d", _recordTsdmaList.total()));
    dumpList(&_recordTsdmaList);
#if defined NEXUS_HAS_DMA || defined NEXUS_HAS_XPT_DMA
    BDBG_MSG(("DMA:                         %d", _dmaList.total()));
    dumpList((MList <CResource> *)&_dmaList);
#endif
    BDBG_MSG(("Timebases:                   %d", _timebaseList.total()));
    dumpList((MList <CResource> *)&_timebaseList);
    BDBG_MSG(("playpumps:                   %d", _playpumpList.total()));
    dumpList((MList <CResource> *)&_playpumpList);
    BDBG_MSG(("playbacks:                   %d", _playbackList.total()));
    dumpList((MList <CResource> *)&_playbackList);
#if NEXUS_HAS_VIDEO_ENCODER
    BDBG_MSG(("MPEG encoders:               %d", _encodeList.total()));
    dumpList((MList <CResource> *)&_encodeList);
#endif
    BDBG_MSG(("transport input bands:       %d", _inputBandList.total()));
    dumpList((MList <CResource> *)&_inputBandList);
    BDBG_MSG(("transport parser bands:      %d", _parserBandList.total()));
    dumpList((MList <CResource> *)&_parserBandList);
    BDBG_MSG(("SPDIF output:                %d", _outputSpdifList.total()));
    dumpList((MList <CResource> *)&_outputSpdifList);
    BDBG_MSG(("Audio DAC output:            %d", _outputAudioDacList.total()));
    dumpList((MList <CResource> *)&_outputAudioDacList);
    BDBG_MSG(("Audio I2S DAC output:        %d", _outputAudioDacI2sList.total()));
    dumpList((MList <CResource> *)&_outputAudioDacI2sList);
    BDBG_MSG(("Audio Dummy output:          %d", _outputAudioDummyList.total()));
    dumpList((MList <CResource> *)&_outputAudioDummyList);
    BDBG_MSG(("Component output:            %d", _outputComponentList.total()));
    dumpList((MList <CResource> *)&_outputComponentList);
    BDBG_MSG(("SVideo output:               %d", _outputSVideoList.total()));
    dumpList((MList <CResource> *)&_outputSVideoList);
    BDBG_MSG(("Composite output:            %d", _outputCompositeList.total()));
    dumpList((MList <CResource> *)&_outputCompositeList);
    BDBG_MSG(("RFM output:                  %d", _outputRFMList.total()));
    dumpList((MList <CResource> *)&_outputRFMList);
    BDBG_MSG(("Hdmi output:                 %d", _outputHdmiList.total()));
    dumpList((MList <CResource> *)&_outputHdmiList);
    BDBG_MSG(("RemoteIr:                    %d", _remoteListIr.total()));
    dumpList((MList <CResource> *)&_remoteListIr);
#if NEXUS_HAS_UHF_INPUT
    BDBG_MSG(("RemoteUhf:                   %d", _remoteListUhf.total()));
    dumpList((MList <CResource> *)&_remoteListUhf);
#endif
    BDBG_MSG(("Power:                       %d", _powerList.total()));
    dumpList((MList <CResource> *)&_powerList);
#ifdef WPA_SUPPLICANT_SUPPORT
    BDBG_MSG(("Wifi:                        %d", _wifiList.total()));
    dumpList((MList <CResource> *)&_wifiList);
#endif
#ifdef NETAPP_SUPPORT
    BDBG_MSG(("Network:                     %d", _networkList.total()));
    dumpList((MList <CResource> *)&_networkList);
    BDBG_MSG(("Bluetooth:                     %d", _bluetoothList.total()));
    dumpList((MList <CResource> *)&_bluetoothList);
    BDBG_MSG(("Bluetooth Remote :                     %d", _remoteListBluetooth.total()));
    dumpList((MList <CResource> *)&_remoteListBluetooth);
#endif /* ifdef NETAPP_SUPPORT */

    if (true == bForce)
    {
        BDBG_SetModuleLevel("atlas_board", level);
    }
} /* dump */