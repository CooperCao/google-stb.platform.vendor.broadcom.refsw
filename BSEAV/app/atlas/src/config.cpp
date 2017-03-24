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

#include "config.h"
#include "nexus_transport_capabilities.h"
#if NEXUS_HAS_FRONTEND
#include "nexus_frontend.h"
#endif

BDBG_MODULE(atlas_config);

#undef MIN
#define MIN(a, b)  ((a) < (b) ? (a) : (b))

eRet CConfig::addChip(CChipInfo info)
{
    eRet        ret       = eRet_Ok;
    CChipInfo * pChipInfo = NULL;

    pChipInfo = new CChipInfo(info);
    BDBG_ASSERT(pChipInfo);
    _chipList.add(pChipInfo);

    return(ret);
}

eRet CConfig::removeChip(CChipInfo info)
{
    eRet        ret   = eRet_Ok;
    CChipInfo * pInfo = NULL;

    /* find and remove chip info from chip list */
    for (pInfo = _chipList.first(); pInfo; pInfo = _chipList.next())
    {
        if (info == *pInfo)
        {
            pInfo = _chipList.remove();
            BDBG_ASSERT(pInfo);
            delete pInfo;
        }
    }

    return(ret);
} /* removeChip */

void CConfig::dump()
{
    MListItr <CChipInfo> itr(&_chipList);
    CChipInfo *          pInfo = NULL;

    BDBG_WRN(("\nCHIP INFO"));
    for (pInfo = itr.first(); pInfo; pInfo = itr.next())
    {
        pInfo->dump();
    }

    _features.dump();
    _pResources->dump();
}

MString CConfig::getBoardName()
{
    MString strBoard(BCHP_CHIP);
    FILE *  fp = fopen("/proc/device-tree/bolt/board", "r");

    if (fp)
    {
        char sBoard[32];
        memset(sBoard, 0, sizeof(strBoard));
        fgets(sBoard, sizeof(sBoard), fp);
        if (0 < MString(sBoard).length())
        {
            strBoard = sBoard;
        }

        fclose(fp);
    }

    return(strBoard);
} /* getBoardName */

/* MString-ify #define value */
#define MXSTR_DEF(def)  MString(XSTR_DEF(def))
/* string-ify #define value */
#define XSTR_DEF(def)   STR_DEF(def)
/* MString-ify #define name */
#define MSTR_DEF(def)   MString(STR_DEF(def))
/* string-ify #define name */
#define STR_DEF(def)    #def

eRet CConfig::initChipInfo()
{
    eRet      ret = eRet_Ok;
    CChipInfo info;

#ifndef BCHP_VER
#error BCHP_VER support must be added
#endif

    /* add main chip to list */
    info.setNumber(BCHP_CHIP);

    SET(&_cfg, BOARD_NAME, getBoardName().s());
    BDBG_WRN(("Board Name: %s", getBoardName().s()));

    if (eRet_Ok != info.setRevision(MXSTR_DEF(BCHP_VER)))
    {
        ret = ATLAS_ERROR("BCHP_VER is invalid", eRet_ExternalError);
    }
    else
    {
        addChip(info);
    }

    return(ret);
} /* initChipInfo */

eRet CConfig::initFeatures()
{
    eRet           ret = eRet_Ok;
    CBoardFeatures features;

    /* set default features */
    _features._pvrEncryption = true;

    /* set optional features */
    NEXUS_DisplayCapabilities capsDisplay;
    NEXUS_GetDisplayCapabilities(&capsDisplay);
    _features._boxDetect = capsDisplay.numLetterBoxDetect;

    memcpy(_features._videoFormatIsSupported, capsDisplay.displayFormatSupported, NEXUS_VideoFormat_eMax);

    NEXUS_AudioCapabilities capsAudio;
    NEXUS_GetAudioCapabilities(&capsAudio);
    _features._autoVolume  = capsAudio.dsp.autoVolumeLevel;
    _features._dolbyVolume = capsAudio.dsp.dolbyVolume258;
    _features._srsVolume   = capsAudio.dsp.truVolume;

#if 0
#ifndef B_XVD_NO_HD_SUPPORT
    _features._videoHd = true;
#endif
#ifndef B_DISPLAY_NO_HD_SUPPORT
    _features._displayHd = true;
#endif
#if     B_HAS_DCR
    _features._dnrDcr = true;
#endif
#if     B_HAS_BNR
    _features._dnrBnr = true;
#endif
#if     B_HAS_MNR
    _features._dnrMnr = true;
#endif
#if     B_HAS_ANR
    _features._anr = true;
#endif
#if     B_HAS_MAD
    _features._deinterlacer = true;
#endif
#if     B_HAS_SHARPNESS
    _features._sharpness = true;
#endif
#if     B_HAS_CAB
    _features._cab = true;
#endif
#if     B_HAS_LAB
    _features._lab = true;
#endif
#if     B_MAX_MOSAIC_DECODES > 0
    _features._mosaic = true;
#endif
#if     B_HAS_SECAM
    _features._videoFormatIsSupported[bvideo_format_secam] = true;
#endif
#endif /* if 0 */

    _features.dump(true);

    return(ret);
} /* initFeatures */

eRet CConfig::initResources()
{
    eRet        ret                  = eRet_Ok;
    int         nDisplays            = GET_INT(&_cfg, RESOURCE_NUM_DISPLAYS);
    int         nGraphics            = GET_INT(&_cfg, RESOURCE_NUM_GRAPHICS);
    int         nSurfaceClients      = 1;
    int         nSimpleVideoDecoders = GET_INT(&_cfg, RESOURCE_NUM_SIMPLE_VIDEO_DECODERS);
    int         nSimpleAudioDecoders = GET_INT(&_cfg, RESOURCE_NUM_SIMPLE_AUDIO_DECODERS);
    int         nStillDecoders       = GET_INT(&_cfg, RESOURCE_NUM_STILL_DECODERS);
    int         nStreamers           = GET_INT(&_cfg, RESOURCE_NUM_STREAMERS);
    int         nRemotesIr           = GET_INT(&_cfg, RESOURCE_NUM_REMOTES_IR);
    int         nRemotesRf4ce        = GET_INT(&_cfg, RESOURCE_NUM_REMOTES_RF4CE);
    int         nRemotesBluetooth    = GET_INT(&_cfg, RESOURCE_NUM_REMOTES_BLUETOOTH);
    int         nRemotesUhf          = GET_INT(&_cfg, RESOURCE_NUM_REMOTES_UHF);
    int         nFrontends           = GET_INT(&_cfg, RESOURCE_NUM_FRONTENDS);
    int         nOutputsSpdif        = GET_INT(&_cfg, RESOURCE_NUM_OUTPUTS_SPDIF);
    int         nOutputsDac          = GET_INT(&_cfg, RESOURCE_NUM_OUTPUTS_DAC);
    int         nOutputsDacI2s       = GET_INT(&_cfg, RESOURCE_NUM_OUTPUTS_DAC_I2S);
    int         nOutputsHdmi         = GET_INT(&_cfg, RESOURCE_NUM_OUTPUTS_HDMI);
    int         nOutputsComponent    = GET_INT(&_cfg, RESOURCE_NUM_OUTPUTS_COMPONENT);
    int         nOutputsComposite    = GET_INT(&_cfg, RESOURCE_NUM_OUTPUTS_COMPOSITE);
    int         nOutputsRfm          = GET_INT(&_cfg, RESOURCE_NUM_OUTPUTS_RFM);
    CPlatform * pPlatformConfig      = _cfg.getPlatformConfig();

    /* if atlas.cfg does not override number of resources, use nexus values */
    NEXUS_AudioCapabilities audioCapabilities;

    NEXUS_GetAudioCapabilities(&audioCapabilities);

    if (0 > nDisplays) { nDisplays = NEXUS_NUM_DISPLAYS; }
    if (0 > nGraphics) { nGraphics = ATLAS_NUM_GRAPHICS; }
    if (0 > nSimpleVideoDecoders) { nSimpleVideoDecoders = ATLAS_NUM_SIMPLE_VIDEO_DECODES; }
    if (0 > nSimpleAudioDecoders) { nSimpleAudioDecoders = audioCapabilities.numDecoders; }
    if (0 > nStillDecoders) { nStillDecoders = NEXUS_NUM_STILL_DECODES; }
    if (0 > nStreamers) { nStreamers = ATLAS_NUM_STREAMERS; }
    if (0 > nRemotesIr) { nRemotesIr = ATLAS_NUM_IR_REMOTES; }
    if (0 > nRemotesRf4ce) { nRemotesRf4ce = ATLAS_NUM_RF4CE_REMOTES; }
    if (0 > nRemotesBluetooth) { nRemotesBluetooth = ATLAS_NUM_BLUETOOTH_REMOTES; }
    if (0 > nRemotesUhf) { nRemotesUhf = NEXUS_NUM_UHF_INPUTS; }
    if (0 > nFrontends) { nFrontends = NEXUS_MAX_FRONTENDS; }
    if (0 > nOutputsSpdif) { nOutputsSpdif = NEXUS_NUM_SPDIF_OUTPUTS; }
    if (0 > nOutputsDac) { nOutputsDac = audioCapabilities.numOutputs.dac; }
    if (0 > nOutputsDacI2s) { nOutputsDacI2s = audioCapabilities.numOutputs.i2s; }
    if (0 > nOutputsHdmi) { nOutputsHdmi = NEXUS_NUM_HDMI_OUTPUTS; }
    if (0 > nOutputsComponent) { nOutputsComponent = NEXUS_NUM_COMPONENT_OUTPUTS; }
    if (0 > nOutputsComposite) { nOutputsComposite = NEXUS_NUM_COMPOSITE_OUTPUTS; }
#if NEXUS_HAS_RFM
    if (0 > nOutputsRfm) { nOutputsRfm = NEXUS_NUM_RFM_OUTPUTS; }
#else
    nOutputsRfm = 0;
#endif

#ifdef DCC_SUPPORT
    nSurfaceClients++;
#endif
    _pResources = new CBoardResources();
    CHECK_PTR_ERROR_GOTO("unable to allocate board resources!", _pResources, ret, eRet_OutOfMemory, error);

    _pResources->add(eBoardResource_display, nDisplays, "display", &_cfg);
    _pResources->add(eBoardResource_graphics, nGraphics, "graphics", &_cfg);
    _pResources->add(eBoardResource_surfaceClient, nSurfaceClients, "surfaceClient", &_cfg);
    _pResources->add(eBoardResource_decodeVideo, NEXUS_NUM_VIDEO_DECODERS, "videoDecode", &_cfg);
    _pResources->add(eBoardResource_simpleDecodeVideo, nSimpleVideoDecoders, "simpleVideoDecode", &_cfg);
    _pResources->add(eBoardResource_decodeAudio, NEXUS_NUM_AUDIO_DECODERS, "audioDecode", &_cfg);
    _pResources->add(eBoardResource_simpleDecodeAudio, nSimpleAudioDecoders, "simpleAudioDecode", &_cfg);
    _pResources->add(eBoardResource_stcChannel, NEXUS_NUM_STC_CHANNELS, "stcChannel", &_cfg);
    /*
     *_pResources->add(eBoardResource_pcmPlayback, ATLAS_NUM_PCM_PLAYBACKS, "pcmPlayback", &_cfg);
     *_pResources->add(eBoardResource_pcmCapture, ATLAS_NUM_PCM_CAPTURES, "pcmCapture", &_cfg);
     */
    _pResources->add(eBoardResource_decodeStill, nStillDecoders, "decodeStill", &_cfg);
    /*_pResources->add(eBoardResource_decodeEs, ATLAS_NUM_ES_DECODES, "decodeEs", &_cfg);*/
    _pResources->add(eBoardResource_streamer, nStreamers, "streamer", &_cfg);
    /*
     *_pResources->add(eBoardResource_linein, ATLAS_NUM_LINEIN, "linein", &_cfg);
     *_pResources->add(eBoardResource_decodeMosaic, NEXUS_NUM_MOSAIC_DECODES, "mosaic", &_cfg);
     */
    _pResources->add(eBoardResource_irRemote, nRemotesIr, "irRemote", &_cfg);
#if RF4CE_SUPPORT
    _pResources->add(eBoardResource_rf4ceRemote, nRemotesRf4ce, "rf4ceRemote", &_cfg);
#endif
#ifdef NETAPP_SUPPORT
    _pResources->add(eBoardResource_bluetoothRemote, nRemotesBluetooth, "bluetoothRemote", &_cfg);
#endif
#if NEXUS_HAS_UHF_INPUT
    _pResources->add(eBoardResource_uhfRemote, nRemotesUhf, "uhfRemote", &_cfg);
#endif

#if NEXUS_HAS_FRONTEND
    /* query nexus for frontend capabilities and add to tuner lists */
    {
        for (int i = 0; i < nFrontends; i++)
        {
            if (NULL != pPlatformConfig->getFrontend(i))
            {
                _pResources->addFrontend(i, &_cfg);
            }
        }
    }
#else /* if NEXUS_HAS_FRONTEND */
    BSTD_UNUSED(pPlatformConfig);
#endif /* NEXUS_HAS_FRONTEND */

    /* query nexus for transport capabilities and add to resource lists */
    {
        uint16_t                    numRecPumps  = 0;
        uint16_t                    numPlayPumps = 0;
        NEXUS_TransportCapabilities capabilities;

        NEXUS_GetTransportCapabilities(&capabilities);
        numRecPumps  = capabilities.numRecpumps;
        numPlayPumps = capabilities.numPlaypumps;

#if DVR_LIB_SUPPORT
        numRecPumps  -= 2;
        numPlayPumps -= 2;

        _pResources->add(eBoardResource_tsb, 2, "tsb", &_cfg);
#endif /* if DVR_LIB_SUPPORT */

        /* You can create as many records as there are recpumps. Only limit is memory */
        _pResources->add(eBoardResource_recpump, numRecPumps, "recpump", &_cfg);
        _pResources->add(eBoardResource_record, numRecPumps, "record", &_cfg);
        /* There is only a limited amount of playpumps. Playbacks are virtual and you can have as many as you want. But you only
         * have a limited amount of playpumps */
        _pResources->add(eBoardResource_playpump, numPlayPumps, "playpump", &_cfg);
        _pResources->add(eBoardResource_playback, numPlayPumps, "playback", &_cfg);

#if NEXUS_HAS_VIDEO_ENCODER
        /* Hard code to 1 , but will be refactored for 14.3 Unified Release */
        _pResources->add(eBoardResource_encode, 1, "encode", &_cfg);
#endif

#ifdef NEXUS_HAS_MXT
        _pResources->add(eBoardResource_inputBand, NEXUS_NUM_MTSIF*8, "inputBand", &_cfg);
#else
        _pResources->add(eBoardResource_inputBand, capabilities.numInputBands, "inputBand", &_cfg);
#endif
        _pResources->add(eBoardResource_parserBand, capabilities.numParserBands, "parserBand", &_cfg);
        /* DTT todo recordTsdma?  recordPes? */
    }
#if defined NEXUS_HAS_DMA || defined NEXUS_HAS_XPT_DMA
    if (NEXUS_NUM_DMA_CHANNELS > 0)
    {
        _pResources->add(eBoardResource_dma, NEXUS_NUM_DMA_CHANNELS, "dma", &_cfg);
    }
#endif /* if defined NEXUS_HAS_DMA || defined NEXUS_HAS_XPT_DMA */

    if (ATLAS_NUM_TIMEBASES > 0)
    {
        _pResources->add(eBoardResource_timebase, ATLAS_NUM_TIMEBASES, "timebase", &_cfg);
    }

    _pResources->add(eBoardResource_power, 1, "power", &_cfg);
#ifdef WPA_SUPPLICANT_SUPPORT
    _pResources->add(eBoardResource_wifi, 1, "wifi", &_cfg);
#endif
#ifdef NETAPP_SUPPORT
    _pResources->add(eBoardResource_network, 1, "network", &_cfg);
    _pResources->add(eBoardResource_bluetooth, 1, "bluetooth", &_cfg);
#endif

    /* add output resources */
    _pResources->add(eBoardResource_outputSpdif, nOutputsSpdif, "spdifOutput", &_cfg);
    _pResources->add(eBoardResource_outputAudioDac, nOutputsDac, "audioDacOutput", &_cfg);
    _pResources->add(eBoardResource_outputAudioDacI2s, nOutputsDacI2s, "audioDacOutputI2s", &_cfg);

#if NEXUS_NUM_AUDIO_DUMMY_OUTPUTS
    _pResources->add(eBoardResource_outputAudioDummy, NEXUS_NUM_AUDIO_DUMMY_OUTPUTS, "audioDummyOutput", &_cfg);
#endif
    _pResources->add(eBoardResource_outputComponent, nOutputsComponent, "componentOutput", &_cfg);
    _pResources->add(eBoardResource_outputSVideo, NEXUS_NUM_SVIDEO_OUTPUTS, "SVideoOutput", &_cfg);
    _pResources->add(eBoardResource_outputComposite, nOutputsComposite, "compositeOutput", &_cfg);
    _pResources->add(eBoardResource_outputRFM, nOutputsRfm, "rfmOutput", &_cfg);
    _pResources->add(eBoardResource_outputHdmi, nOutputsHdmi, "hdmiOutput", &_cfg);

    _pResources->dump(true);

error:
    return(ret);
} /* initResources */

void CConfig::uninitResources()
{
    _pResources->clear();
}

CConfig::CConfig() :
    CMvcModel("CConfig"),
    _revision(0),
    _pResources(NULL)
{
    _name = "BCM_BOARD_STR";
    _chipList.clear();
    _features.clear();

    /* update configuration settings from file */
    _cfg.read(GET_STR(&_cfg, CONFIG_FILENAME));
}

CConfig::~CConfig()
{
}

eRet CConfig::initialize()
{
    eRet ret = eRet_Ok;

    _cfg.initialize();

    ret = initChipInfo();
    CHECK_ERROR_GOTO("error initializing chip info", ret, error);

    ret = initFeatures();
    CHECK_ERROR_GOTO("error initializing board features", ret, error);

    ret = initResources();
    CHECK_ERROR_GOTO("error initializing board resources", ret, error);

error:
    return(ret);
} /* initialize */

void CConfig::uninitialize()
{
    uninitResources();
}