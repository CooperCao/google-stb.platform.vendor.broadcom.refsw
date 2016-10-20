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

#include "board_nx.h"
#include "network.h"
#include "display_nx.h"
#include "video_decode_nx.h"
#include "audio_decode_nx.h"
#include "output_nx.h"
#include "bluetooth_nx.h"

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

/* add a number of resources to associated list */
eRet CBoardResourcesNx::add(
        eBoardResource   resource,
        const uint16_t   numResources,
        const char *     name,
        CConfiguration * pCfg,
        const uint16_t   startIndex,
        const unsigned   id
        )
{
    eRet        ret             = eRet_Ok;
    CPlatform * pPlatformConfig = NULL;

    BDBG_ASSERT(NULL != pCfg);
    pPlatformConfig = pCfg->getPlatformConfig();

    for (uint16_t i = startIndex; i < (startIndex + numResources); i++)
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
        {
            CTuner * pTuner = NULL;
            pTuner = new CTunerQamNx(MString(name)+"Nx", (id && (id != i)) ? id : i, pCfg);
            BDBG_ASSERT(pTuner);
            _tunerList.add(pTuner);
        }
        break;
        case eBoardResource_frontendVsb:
        {
            CTuner * pTuner = NULL;
            pTuner = new CTunerVsbNx(MString(name)+"Nx", (id && (id != i)) ? id : i, pCfg);
            BDBG_ASSERT(pTuner);
            _tunerList.add(pTuner);
        }
        break;
        case eBoardResource_frontendSds:
        {
            CTuner * pTuner = NULL;
            pTuner = new CTunerSatNx(MString(name)+"Nx", (id && (id != i)) ? id : i, pCfg);
            BDBG_ASSERT(pTuner);
            _tunerList.add(pTuner);
        }
        break;
        case eBoardResource_frontendOfdm:
        {
            CTuner * pTuner = NULL;
            pTuner = new CTunerOfdmNx(MString(name)+"Nx", (id && (id != i)) ? id : i, pCfg);
            BDBG_ASSERT(pTuner);
            _tunerList.add(pTuner);
        }
        break;
#ifdef MPOD_SUPPORT
        case eBoardResource_frontendOob:
        {
            CTuner * pTuner = NULL;
            pTuner = new CTunerOobNx(MString(name)+"Nx", (id && (id != i)) ? id : i, pCfg);
            BDBG_ASSERT(pTuner);
            _tunerList.add(pTuner);
        }
        break;
#endif /* MPOD_SUPPORT */
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