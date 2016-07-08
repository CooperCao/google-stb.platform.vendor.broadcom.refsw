/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
#include "config_nx.h"
#include "nxclient.h"
#include "nexus_transport_capabilities.h"
#if NEXUS_HAS_FRONTEND
#include "nexus_frontend.h"
#endif

BDBG_MODULE(atlas_config);

#undef MIN
#define MIN(a, b)  ((a) < (b) ? (a) : (b))

CConfigNx::CConfigNx() :
    CConfig()
{
}

CConfigNx::~CConfigNx()
{
}

eRet CConfigNx::initResources()
{
    NxClient_AllocSettings allocSettings;
    uint16_t               i            = 0;
    NEXUS_Error            nerror       = NEXUS_SUCCESS;
    int                    nStreamers   = GET_INT(&_cfg, RESOURCE_NUM_STREAMERS);
    int                    nOutputsHdmi = GET_INT(&_cfg, RESOURCE_NUM_OUTPUTS_HDMI);
    int                    nOutputsRfm  = GET_INT(&_cfg, RESOURCE_NUM_OUTPUTS_RFM);
    int                    maxMosaics   = 0;

#ifdef NETAPP_SUPPORT
    int nRemotesBluetooth = GET_INT(&_cfg, RESOURCE_NUM_REMOTES_BLUETOOTH);
#endif
    if (0 > nStreamers) { nStreamers = ATLAS_NUM_STREAMERS; }
    CPlatform * pPlatformConfig = _cfg.getPlatformConfig();
    eRet        ret             = eRet_Ok;

    if (0 > nOutputsHdmi) { nOutputsHdmi = NEXUS_NUM_HDMI_OUTPUTS; }
#if NEXUS_HAS_RFM
    if (0 > nOutputsRfm) { nOutputsRfm = NEXUS_NUM_RFM_OUTPUTS; }
#else
    nOutputsRfm = 0;
#endif

    BSTD_UNUSED(pPlatformConfig);
    BDBG_WRN(("%s()", __FUNCTION__));
    _pResources = new CBoardResourcesNx();
    CHECK_PTR_ERROR_GOTO("unable to allocate board resources Nx!", _pResources, ret, eRet_OutOfMemory, error);

    _pResources->add(eBoardResource_display, 2, "display", &_cfg);

    for (i = 1; i <= 4; i++)
    {
        NEXUS_DisplayMaxMosaicCoverage mosaicCoverage;
        NEXUS_Display_GetMaxMosaicCoverage(0, i, &mosaicCoverage);

        if (mosaicCoverage.maxCoverage < (i * 25))
        {
            break;
        }

        maxMosaics = i;
    }
    BDBG_WRN(("maximum number of mosaics:%d", maxMosaics));

    NxClient_GetDefaultAllocSettings(&allocSettings);
    allocSettings.simpleVideoDecoder  = 1 + maxMosaics;
    allocSettings.simpleAudioDecoder  = 1;
    allocSettings.simpleAudioPlayback = 0;
    allocSettings.inputClient         = 2;
    allocSettings.simpleEncoder       = 0;
    allocSettings.surfaceClient       = 1;
#ifdef DCC_SUPPORT
    allocSettings.surfaceClient++;
#endif
    nerror = NxClient_Alloc(&allocSettings, &_allocResultsMain);
    CHECK_NEXUS_ERROR_ASSERT("error unable to allocate the minimum number of resources", nerror);

    /* add available resources */
    for (i = 0; i < allocSettings.simpleVideoDecoder; i++)
    {
        _pResources->add(eBoardResource_simpleDecodeVideo, 1, "simpleVideoDecode", &_cfg, 0, _allocResultsMain.simpleVideoDecoder[i].id);
    }

    if (0 < allocSettings.simpleAudioDecoder)
    {
        _pResources->add(eBoardResource_simpleDecodeAudio, 1, "simpleAudioDecode", &_cfg, 0, _allocResultsMain.simpleAudioDecoder.id);
    }

    _pResources->add(eBoardResource_stcChannel, NEXUS_NUM_STC_CHANNELS, "stcChannel", &_cfg);
    _pResources->add(eBoardResource_streamer, nStreamers, "streamer", &_cfg);

    for (i = 0; i < allocSettings.inputClient; i++)
    {
#if NEXUS_HAS_UHF_INPUT
        if ((allocSettings.inputClient - 1) == i)
        {
            _pResources->add(eBoardResource_uhfRemote, 1, "uhfRemote", &_cfg, 0, _allocResultsMain.inputClient[i].id);
        }
        else
#endif /* if NEXUS_HAS_UHF_INPUT */
        {
            _pResources->add(eBoardResource_irRemote, 1, "irRemote", &_cfg, 0, _allocResultsMain.inputClient[i].id);
        }
    }
#ifdef NETAPP_SUPPORT
    _pResources->add(eBoardResource_bluetoothRemote, nRemotesBluetooth, "bluetoothRemote", &_cfg);
#endif
    _pResources->add(eBoardResource_graphics, 1, "graphics", &_cfg);

    for (i = 0; i < allocSettings.surfaceClient; i++)
    {
        _pResources->add(eBoardResource_surfaceClient, 1, "surfaceClient", &_cfg, i, _allocResultsMain.surfaceClient[i].id);
    }

    /* allocate simple video decoders for PiP using separate alloc calls
     * which is required to make pip swap work properly in nxclient mode
     */
    {
        NxClient_GetDefaultAllocSettings(&allocSettings);
        allocSettings.simpleVideoDecoder = 1;
        allocSettings.simpleAudioDecoder = 1;
        nerror                           = NxClient_Alloc(&allocSettings, &_allocResultsPip);
        CHECK_NEXUS_ERROR_ASSERT("error unable to allocate the minimum number of resources", nerror);

        /* add available resources */
        if (0 < allocSettings.simpleVideoDecoder)
        {
            _pResources->add(eBoardResource_simpleDecodeVideo, 1, "simpleVideoDecode", &_cfg, 0, _allocResultsPip.simpleVideoDecoder[0].id);
        }

        if (0 < allocSettings.simpleAudioDecoder)
        {
            _pResources->add(eBoardResource_simpleDecodeAudio, 1, "simpleAudioDecode", &_cfg, 0, _allocResultsPip.simpleAudioDecoder.id);
        }
    }

#if NEXUS_HAS_FRONTEND
    /* query nexus for frontend capabilities and add to tuner lists */
    {
        NEXUS_FrontendAcquireSettings settings;
        NEXUS_Frontend_GetDefaultAcquireSettings(&settings);
        settings.mode = NEXUS_FrontendAcquireMode_eByIndex;

        for (uint16_t i = 0; i < NEXUS_MAX_FRONTENDS; i++)
        {
            NEXUS_FrontendHandle hFrontend = NULL;

            settings.index = i;
            hFrontend      = NEXUS_Frontend_Acquire(&settings);
            if (NULL != hFrontend)
            {
                NEXUS_FrontendCapabilities frontendCapabilities;
                memset(&frontendCapabilities, 0, sizeof(frontendCapabilities));

                NEXUS_Frontend_GetCapabilities(hFrontend, &frontendCapabilities);
                _pResources->addFrontend(i, &_cfg, &frontendCapabilities);

                NEXUS_Frontend_Release(hFrontend);
                hFrontend = NULL;
            }
        }
    }
#endif /* NEXUS_HAS_FRONTEND */

    /* query nexus for transport capabilities and add to resource lists */
    {
        uint16_t                    numRecPumps = 0;
        NEXUS_TransportCapabilities capabilities;

        NEXUS_GetTransportCapabilities(&capabilities);
        numRecPumps = capabilities.numRecpumps;

        /* You can create as many records as there are recpumps. Only limit is memory */
        _pResources->add(eBoardResource_recpump, numRecPumps, "recpump", &_cfg);
        _pResources->add(eBoardResource_record, numRecPumps, "record", &_cfg);
        _pResources->add(eBoardResource_playpump, capabilities.numPlaypumps, "playpump", &_cfg);
        _pResources->add(eBoardResource_playback, capabilities.numPlaypumps, "playback", &_cfg);
#ifdef NEXUS_HAS_MXT
        _pResources->add(eBoardResource_inputBand, NEXUS_NUM_MTSIF*8, "inputBand", &_cfg);
#else
        _pResources->add(eBoardResource_inputBand, capabilities.numInputBands, "inputBand", &_cfg);
#endif
        _pResources->add(eBoardResource_parserBand, capabilities.numParserBands, "parserBand", &_cfg);
        _pResources->add(eBoardResource_timebase, capabilities.numTimebases, "timebase", &_cfg);
    }

    _pResources->add(eBoardResource_outputRFM, nOutputsRfm, "rfmOutput", &_cfg);
    _pResources->add(eBoardResource_outputHdmi, nOutputsHdmi, "hdmiOutput", &_cfg);

    _pResources->add(eBoardResource_power, 1, "power", &_cfg);
#ifdef NETAPP_SUPPORT
    _pResources->add(eBoardResource_network, 1, "network", &_cfg);
    _pResources->add(eBoardResource_bluetooth, 1, "bluetooth", &_cfg);
#endif
    _pResources->dump();

error:
    return(ret);
} /* initResources() */

void CConfigNx::uninitResources()
{
    _pResources->clear();
    NxClient_Free(&_allocResultsPip);
    NxClient_Free(&_allocResultsMain);
}