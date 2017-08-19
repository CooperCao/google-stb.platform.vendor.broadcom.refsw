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

#include "board_nx.h"
#include "network.h"
#include "display_nx.h"
#include "video_decode_nx.h"
#include "audio_decode_nx.h"
#include "output_nx.h"
#include "bluetooth_nx.h"
#include "wifi.h"

#if NEXUS_HAS_FRONTEND
#include "tuner_qam_nx.h"
#include "tuner_vsb_nx.h"
#include "tuner_sat_nx.h"
#include "tuner_ofdm_nx.h"
#ifdef MPOD_SUPPORT
#include "tuner_oob_nx.h"
#endif
#endif /* NEXUS_HAS_FRONTEND */

BDBG_MODULE(atlas_board);

CBoardResourcesNx::CBoardResourcesNx() :
    CBoardResources()
{
}

#if NEXUS_HAS_FRONTEND
/* frontend resources may be added multiple times for a given tuner.  for example, if a tuner can do both vsb and
 * qam, it will be added once as a vsb frontend and once as a qam frontend.  the num used for each tuner will
 * be identical, and checking out either tuner will automatically check out the other.  This is done because
 * only a specific tuning object knows how to tune itself - so even if a tuner can be either qam or vsb, the
 * object that represents it can only be one or the other. */
eRet CBoardResourcesNx::addFrontend(
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
        pTuner = new CTunerQamNx("tunerQamNx", numTuner, pCfg);
        CHECK_PTR_ERROR_GOTO("unable to allocate tuner", pTuner, ret, eRet_OutOfMemory, error);

        _tunerList.add(pTuner);
    }

    if (true == capabilities.vsb)
    {
        pTuner = new CTunerVsbNx("tunerVsbNx", numTuner, pCfg);
        CHECK_PTR_ERROR_GOTO("unable to allocate tuner", pTuner, ret, eRet_OutOfMemory, error);

        _tunerList.add(pTuner);
    }

    if (true == capabilities.satellite)
    {
        pTuner = new CTunerSatNx("tunerSatNx", numTuner, pCfg);
        CHECK_PTR_ERROR_GOTO("unable to allocate tuner", pTuner, ret, eRet_OutOfMemory, error);

        _tunerList.add(pTuner);
    }

    if (true == capabilities.ofdm)
    {
        pTuner = new CTunerOfdmNx("tunerOfdmNx", numTuner, pCfg);
        CHECK_PTR_ERROR_GOTO("unable to allocate tuner", pTuner, ret, eRet_OutOfMemory, error);

        _tunerList.add(pTuner);
    }
#ifdef MPOD_SUPPORT
    if (true == capabilities.outOfBandModes[NEXUS_FrontendOutOfBandMode_eDvs178Qpsk])
    {
        CCablecard * pCableCard = NULL;
        pCableCard = new CCablecard;
        pTuner     = new CTunerOobNx("tunerOobNx", numTuner, pCfg);
        CHECK_PTR_ERROR_GOTO("unable to allocate tuner", pTuner, ret, eRet_OutOfMemory, error);
        /*pass oob tuner object to CCablecard*/
        pCableCard->initialize((CTunerOobNx *)pTuner);
        _tunerList.add(pTuner);
        delete pCableCard;
#ifdef SNMP_SUPPORT
        CSnmp * pSnmp = NULL;
        pSnmp = new CSnmp;
        /* pass oob tuner object to CSnmp */
        pSnmp->snmp_save_oob((CTunerOobNx *)pTuner);
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
eRet CBoardResourcesNx::add(
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
                pDisplay = new CDisplayNx(MString(name)+"Nx", (id && (id != i)) ? id : i, pCfg);
                BDBG_ASSERT(pDisplay);
                _displayList.add(pDisplay);
            }
            break;

        case eBoardResource_graphics:
            /* skip unsupported graphics */
            if (true == pPlatformConfig->isSupportedDisplay(i))
            {
                CGraphics * pGraphics = NULL;
                pGraphics = new CGraphicsNx(MString(name)+"Nx", (id && (id != i)) ? id : i, pCfg);
                BDBG_ASSERT(pGraphics);
                _graphicsList.add(pGraphics);
            }
            break;

        case eBoardResource_surfaceClient:
        {
            CSurfaceClient * pSurfaceClient = NULL;
            pSurfaceClient = new CSurfaceClientNx(name, (id && (id != i)) ? id : i, pCfg);
            BDBG_ASSERT(pSurfaceClient);
            _surfaceClientList.add(pSurfaceClient);
        }
        break;

        case eBoardResource_simpleDecodeVideo:
            /* skip unsupported decoders */
            if (true == pPlatformConfig->isSupportedDecoder(i))
            {
                CSimpleVideoDecode * pSimpleVideoDecode = NULL;
                pSimpleVideoDecode = new CSimpleVideoDecodeNx(MString(name)+"Nx", (id && (id != i)) ? id : i, pCfg);
                BDBG_ASSERT(pSimpleVideoDecode);
                _simpleDecodeVideoList.add(pSimpleVideoDecode);
            }
            break;

        case eBoardResource_simpleDecodeAudio:
            /* skip unsupported decoders */
        {
            CSimpleAudioDecode * pSimpleAudioDecode = NULL;
            pSimpleAudioDecode = new CSimpleAudioDecodeNx(MString(name)+"Nx", (id && (id != i)) ? id : i, pCfg);
            BDBG_ASSERT(pSimpleAudioDecode);
            _simpleDecodeAudioList.add(pSimpleAudioDecode);
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

        case eBoardResource_parserBand:
        {
            CParserBand * pParserBand = NULL;
            pParserBand = new CParserBandNx(MString(name)+"Nx", (id && (id != i)) ? id : i, pCfg);
            BDBG_ASSERT(pParserBand);
            _parserBandList.add(pParserBand);
        }
        break;

        case eBoardResource_irRemote:
        {
            CIrRemote * pIrRemote = NULL;
            pIrRemote = new CIrRemoteNx(MString(name)+"Nx", (id && (id != i)) ? id : i, pCfg);
            BDBG_ASSERT(pIrRemote);
            _remoteListIr.add(pIrRemote);
        }
        break;

#if NEXUS_HAS_UHF_INPUT
        case eBoardResource_uhfRemote:
        {
            CUhfRemote * pUhfRemote = NULL;
            pUhfRemote = new CUhfRemoteNx(MString(name)+"Nx", (id && (id != i)) ? id : i, pCfg);
            BDBG_ASSERT(pUhfRemote);
            _remoteListUhf.add(pUhfRemote);
        }
        break;
#endif /* if NEXUS_HAS_UHF_INPUT */
#ifdef WPA_SUPPLICANT_SUPPORT
        case eBoardResource_wifi:
        {
            CWifi * pWifi = NULL;
            pWifi = new CWifi(MString(name)+"Nx", (id && (id != i)) ? id : i, pCfg);
            BDBG_ASSERT(pWifi);
            /* this is a singleton Wifi object so allow it to be checked out multiple times */
            pWifi->setCheckedOutMax(-1);
            _wifiList.add(pWifi);
        }
        break;
#endif /* ifdef WPA_SUPPLICANT_SUPPORT */
#ifdef NETAPP_SUPPORT
        case eBoardResource_bluetoothRemote:
        {
            CBluetoothRemote * pBluetoothRemote = NULL;
            pBluetoothRemote = new CBluetoothRemote(MString(name)+"Nx", (id && (id != i)) ? id : i, pCfg);
            BDBG_ASSERT(pBluetoothRemote);
            _remoteListBluetooth.add(pBluetoothRemote);
        }
        break;

        case eBoardResource_network:
        {
            CNetwork * pNetwork = NULL;
            pNetwork = new CNetwork(MString(name)+"Nx", (id && (id != i)) ? id : i, pCfg);
            BDBG_ASSERT(pNetwork);
            /* this is a singleton network object so allow it to be checked out multiple times */
            pNetwork->setCheckedOutMax(-1);
            _networkList.add(pNetwork);
        }
        break;
        case eBoardResource_bluetooth:
        {
            CBluetoothNx * pBluetooth = NULL;
            pBluetooth = new CBluetoothNx(MString(name)+"Nx", (id && (id != i)) ? id : i, pCfg);
            BDBG_ASSERT(pBluetooth);
            /* this is a singleton network object so allow it to be checked out multiple times */
            pBluetooth->setCheckedOutMax(-1);
            _bluetoothList.add(pBluetooth);
        }
        break;
#endif /* ifdef NETAPP_SUPPORT */

        case eBoardResource_outputRFM:
        {
            COutputRFM * pOutputRFM = NULL;
            pOutputRFM = new COutputRFM(MString(name)+"Nx", (id && (id != i)) ? id : i, pCfg);
            BDBG_ASSERT(pOutputRFM);
            _outputRFMList.add(pOutputRFM);
        }
        break;

        case eBoardResource_power:
        {
            CPowerNx * pPowerNx = NULL;
            pPowerNx = new CPowerNx(MString(name)+"Nx", (id && (id != i)) ? id : i, pCfg);
            BDBG_ASSERT(pPowerNx);
            _powerList.add(pPowerNx);
        }
        break;

        case eBoardResource_outputHdmi:
        {
            COutputHdmiNx * pOutputHdmi = NULL;
            pOutputHdmi = new COutputHdmiNx(MString(name)+"Nx", (id && (id != i)) ? id : i, pCfg);
            BDBG_ASSERT(pOutputHdmi);
            _outputHdmiList.add(pOutputHdmi);
        }
        break;

        default:
            ret = CBoardResources::add(resource, numResources, name, pCfg, startIndex, id);
            break;
        } /* switch */
    }

    return(ret);
} /* add */
