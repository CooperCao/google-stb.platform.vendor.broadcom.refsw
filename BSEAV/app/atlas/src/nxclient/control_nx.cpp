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

#include "nxclient.h"
#include "control_nx.h"
#include "output_nx.h"
#include "video_decode_nx.h"
#include "audio_decode_nx.h"
#include "display_nx.h"
#include "power_nx.h"

BDBG_MODULE(atlas_control);

CControlNx::CControlNx(const char * strName) :
    CControl(strName)
{
}

eRet CControlNx::setOptimalVideoFormat(
        CDisplay *           pDisplay,
        CSimpleVideoDecode * pVideoDecode
        )
{
    NEXUS_Error              nerror = NEXUS_SUCCESS;
    NEXUS_VideoDecoderStatus status;
    eRet                     ret = eRet_Ok;

    BDBG_ASSERT(NULL != pVideoDecode);
    BDBG_ASSERT(NULL != pDisplay);

    nerror = pVideoDecode->getStatus(&status);
    if (eRet_Ok == CHECK_NEXUS_ERROR("Video source changed notification received but unable to get video decoder status", nerror))
    {
        NEXUS_VideoFormat formatOptimal;

        /* NxClient mode does not allow us access to the outputs so we'll just get the format
         * based on the source decode only.  there is a chance the output will not support the
         * given format but for now, we'll blindly set it. */
        if (eRet_Ok == pVideoDecode->getOptimalVideoFormat(NULL, &formatOptimal))
        {
            setVideoFormat(formatOptimal);
        }
    }

    return(ret);
} /* setOptimalVideoFormat */

eRet CControlNx::setVideoFormat(NEXUS_VideoFormat videoFormat)
{
    CDisplay *  pDisplayHD = NULL;
    CGraphics * pGraphics  = NULL;
    eRet        ret        = eRet_Ok;

    if (NEXUS_VideoFormat_eUnknown == videoFormat)
    {
        videoFormat = stringToVideoFormat(GET_STR(_pCfg, PREFERRED_FORMAT_HD));
        BDBG_WRN(("attempting to set unknown video format - default to PREFERRED_FORMAT_HD:%s", GET_STR(_pCfg, PREFERRED_FORMAT_HD)));
    }

    pDisplayHD = _pModel->getDisplay(0);
    pGraphics  = _pModel->getGraphics();

    BDBG_ASSERT(NULL != pDisplayHD);
    BDBG_ASSERT(NULL != pGraphics);

    /* only need to set the HD display format - nxserver will automatically change the SD display
     * video format if necessary */
    ret = pDisplayHD->setFormat(videoFormat, pGraphics);
    CHECK_ERROR_GOTO("unable to set HD video format", ret, error);

error:
    return(ret);
} /* setVideoFormat */

eRet CControlNx::setDeinterlacer(bool bDeinterlacer)
{
    CDisplay * pDisplay = _pModel->getDisplay(0);
    eRet       ret      = eRet_Ok;

    BDBG_ASSERT(NULL != pDisplay);

    if (true == GET_BOOL(_pCfg, DEINTERLACER_ENABLED_SD))
    {
        BDBG_WRN(("DEINTERLACER_ENABLED_SD atlas.cfg option has no effect in nxclient mode"));
    }

    /* We only have to set the deinterlacer setting for the hd display.  nxserver will
     * handle the sd display if necessary */
    ret = pDisplay->setDeinterlacer(bDeinterlacer);
    CHECK_ERROR_GOTO("unable to set deinterlacer", ret, error);

error:
    return(ret);
} /* setDeinterlacer */

eRet CControlNx::setContentMode(NEXUS_VideoWindowContentMode contentMode)
{
    CDisplay * pDisplay = _pModel->getDisplay();
    eRet       ret      = eRet_Ok;

    BDBG_ASSERT(NULL != pDisplay);

    /* We only have to set the content mode setting for the hd display.  nxserver will
     * handle the sd display if necessary */
    ret = pDisplay->setContentMode(contentMode);
    CHECK_ERROR_GOTO("unable to set content mode", ret, error);

error:
    return(ret);
} /* setContentMode */

eRet CControlNx::setColorSpace(NEXUS_ColorSpace colorSpace)
{
    CDisplay * pDisplay = _pModel->getDisplay();
    eRet       ret      = eRet_Ok;

    BDBG_ASSERT(NULL != pDisplay);

    /* We only have to set the colorspace setting for the hd display.  nxserver will
     * handle the sd display (for component) if necessary */
    ret = pDisplay->setColorSpace(&colorSpace);
    CHECK_ERROR_GOTO("unable to set color space", ret, error);

error:
    return(ret);
} /* setColorSpace */

eRet CControlNx::setColorDepth(uint8_t colorDepth)
{
    CDisplay * pDisplay = _pModel->getDisplay();
    eRet       ret      = eRet_Ok;

    BDBG_ASSERT(NULL != pDisplay);

    /* We only have to set the colordepth setting for the hd display.  nxserver will
     * handle the sd display (for component) if necessary */
    ret = pDisplay->setColorDepth(&colorDepth);
    CHECK_ERROR_GOTO("unable to set color depth", ret, error);

error:
    return(ret);
} /* setColorDepth */

eRet CControlNx::setMpaaDecimation(bool bMpaaDecimation)
{
    CDisplay * pDisplay = _pModel->getDisplay();
    eRet       ret      = eRet_Ok;

    BDBG_ASSERT(NULL != pDisplay);

    /* We only have to set the mpaa decimation setting for the hd display.  nxserver will
     * handle the sd display if necessary (mpaa decimation only applies to the component
     * output which could be attached to either display) */
    ret = pDisplay->setMpaaDecimation(bMpaaDecimation);
    CHECK_ERROR_GOTO("unable to set MPAA decimation", ret, error);

error:
    return(ret);
} /* setMpaaDecimation */

eRet CControlNx::showPip(bool bShow)
{
    eRet        ret          = eRet_Ok;
    eWindowType pipWinType   = _pModel->getPipScreenWindowType();
    eWindowType fullWinType  = _pModel->getFullScreenWindowType();
    CChannel *  pChannelMain = NULL;
    CChannel *  pChannelPip  = NULL;
    static bool bFirst       = true;

    if (false == _pModel->getPipEnabled())
    {
        BDBG_WRN(("PIP is disabled"));
        goto error;
    }

    if (_pModel->getPipState() == bShow)
    {
        /* this will generate a necessary pip change notification even though nothing changed */
        _pModel->setPipState(bShow);
        return(ret);
    }

    if (true == bShow)
    {
        /* show pip */

        /* get last pip channel */
        pChannelPip = _pModel->getLastTunedChannel(pipWinType);
        if (NULL == pChannelPip)
        {
            pChannelMain = _pModel->getCurrentChannel(fullWinType);
            pChannelPip  = _pModel->getCurrentChannel(pipWinType);

            if (true == bFirst)
            {
                MString strInitialChannelOverride = GET_STR(_pCfg, PIP_INITIAL_CHANNEL);
                if (1.0 <= strInitialChannelOverride.toFloat())
                {
                    /* use atlas.cfg setting for initial pip channel */
                    pChannelPip = _pChannelMgr->findChannel(strInitialChannelOverride.s(), pipWinType);
                }
                else
                if (NULL != pChannelMain)
                {
                    MString strChannelNumMain = MString(pChannelMain->getMajor()) + "." + MString(pChannelMain->getMinor());

                    /* first showPip will tune pip to the same channel as full window.  find
                     * corresponding channel object in PIP channel list using channelmgr */
                    pChannelPip = _pChannelMgr->findChannel(strChannelNumMain.s(), pipWinType);
                }

                bFirst = false;
            }

            if (NULL == pChannelPip)
            {
                /* get first channel in PIP channel list */
                pChannelPip = _pChannelMgr->getFirstChannel(pipWinType);
            }
        }

#if BDSP_MS12_SUPPORT
        /* start audio fade at 1 for the decimated window. after showing, decimated window audio will
           then rise up to the requested level */
        {
            CSimpleAudioDecode * pAudioDecode = _pModel->getSimpleAudioDecode(_pModel->getPipScreenWindowType());
            if (NULL != pAudioDecode)
            {
                /* we are showing pip so start it's fade level at 1% so we can ramp up
                   the volume level gradually once audio source changed notification occurs */
                pAudioDecode->setAudioFadeStartLevel(1);
            }
        }
#endif
    }
    else
    {
        /* hide pip */

#if BDSP_MS12_SUPPORT
        setAudioFade(false);

        if (false == GET_BOOL(_pCfg, EXIT_APPLICATION))
        {
            CSimpleAudioDecode * pAudioDecode = _pModel->getSimpleAudioDecode(_pModel->getPipScreenWindowType());
            waitAudioFadeComplete(pAudioDecode);
        }
#endif

        eMode mode = _pModel->getMode(pipWinType);

        switch (mode)
        {
        case eMode_Live:
        {
            CChannel * pChannelPip = _pModel->getCurrentChannel(pipWinType);
            if (NULL != pChannelPip)
            {
                ret = unTuneChannel(pChannelPip, true, pipWinType);
                CHECK_ERROR("Unable to unTune PIP channel", ret);
            }
            break;
        }

        case eMode_Playback:
        {
            CPlayback * pPlaybackPip = _pModel->getPlayback(pipWinType);
            if (NULL != pPlaybackPip)
            {
                ret = playbackStop(NULL, pipWinType);
                CHECK_ERROR("Unable to stop PIP playback", ret);
            }
        }
        break;

        default:
            break;
        } /* switch */
    }

    _pModel->setPipState(bShow);

    if (true == bShow)
    {
        if (NULL != pChannelPip)
        {
            eWindowType pipWinType = _pModel->getPipScreenWindowType();

            _pModel->setDeferredChannelNum(pChannelPip->getChannelNum().s(), pChannelPip, pipWinType);
            _deferredChannelPipTimer.start();
        }
    }
error:
    return(ret);
} /* showPip */

eRet CControlNx::swapPip()
{
    CSimpleVideoDecodeNx *    pVideoDecodeMain = (CSimpleVideoDecodeNx *)_pModel->getSimpleVideoDecode(eWindowType_Main);
    CSimpleVideoDecodeNx *    pVideoDecodePip  = (CSimpleVideoDecodeNx *)_pModel->getSimpleVideoDecode(eWindowType_Pip);
    CChannel *                pChannel         = (CChannel *)_pModel->getCurrentChannel();
    NxClient_ReconfigSettings reconfigSettings;
    eRet                      ret    = eRet_Ok;
    NEXUS_Error               nerror = NEXUS_SUCCESS;

    BDBG_ASSERT(NULL != pVideoDecodeMain);
    BDBG_ASSERT(NULL != pVideoDecodePip);

    if (false == _pModel->getPipState())
    {
        BDBG_WRN(("Swap is unavailable while PIP window is hidden"));
        goto error;
    }

    if (false == _pModel->getPipEnabled())
    {
        BDBG_WRN(("PIP is disabled"));
        goto error;
    }

    if ((NULL != pChannel) && (false == pChannel->isPipSwapSupported()))
    {
        BDBG_WRN(("PIP swap is disabled by channel"));
        goto error;
    }

    /* keep track of which content is full screen (main or pip) */
    _pModel->setFullScreenWindowType((eWindowType_Main == _pModel->getFullScreenWindowType()) ? eWindowType_Pip : eWindowType_Main);

    NxClient_GetDefaultReconfigSettings(&reconfigSettings);
#if BDSP_MS12_SUPPORT
    /* ms12 mixer supported so we are using 2 audio decoders and crossfading audio between main/pip.
       no need to swap audio since we will just adjust the fade values of each. */
    reconfigSettings.command[0].type       = NxClient_ReconfigType_eRerouteVideo;
#else
    /* single audio decoder so we need to swap audio too */
    reconfigSettings.command[0].type       = NxClient_ReconfigType_eRerouteVideoAndAudio;
#endif
    reconfigSettings.command[0].connectId1 = _pModel->getConnectId(eWindowType_Main);
    reconfigSettings.command[0].connectId2 = _pModel->getConnectId(eWindowType_Pip);
    nerror = NxClient_Reconfig(&reconfigSettings);
    CHECK_NEXUS_ERROR_GOTO("unable to swap pip windows", ret, nerror, error);

#if BDSP_MS12_SUPPORT
    /* if we have dual audio decoders, and if pip is visible, fade main/pip audio */
    setAudioFade(_pModel->getPipState());
#endif

error:
    /* notify that pip state has changed - no actual change in pip state value */
    _pModel->setPipState(_pModel->getPipState());
    return(ret);
} /* swapPip() */

bool CControlNx::checkPower(void)
{
    CBoardResources * pBoardResources = NULL;
    CPowerNx *        pPowerNx        = NULL;
    bool              powerOn         = false;

    pBoardResources = _pConfig->getBoardResources();
    BDBG_ASSERT(NULL != pBoardResources);
#if POWERSTANDBY_SUPPORT
    pPowerNx = (CPowerNx *)pBoardResources->checkoutResource(_id, eBoardResource_power);
    if (pPowerNx == NULL)
    {
        BDBG_WRN(("Power resource is not available "));
        return(false);
    }

    BDBG_MSG(("CControlNX::%s mode:S%d", BSTD_FUNCTION, pPowerNx->getMode()));
    powerOn = pPowerNx->checkPower();
    pBoardResources->checkinResource(pPowerNx);
    BKNI_Sleep(1000);
    return(powerOn);

#else /* if POWERSTANDBY_SUPPORT */
    BSTD_UNUSED(pPowerNx);
    return(powerOn);

#endif /* if POWERSTANDBY_SUPPORT */
} /* checkPower */

eRet CControlNx::setPowerMode(ePowerMode mode)
{
    eRet ret = eRet_Ok;

#if POWERSTANDBY_SUPPORT
    CBoardResources * pBoardResources = NULL;
    CPowerNx *        pPowerNx        = NULL;
    CDisplay *        pDisplayHD      = _pModel->getDisplay(0);
    CDisplay *        pDisplaySD      = _pModel->getDisplay(1);
    CGraphics *       pGraphics       = _pModel->getGraphics();

    pBoardResources = _pConfig->getBoardResources();
    BDBG_ASSERT(NULL != pBoardResources);
    pPowerNx = (CPowerNx *)pBoardResources->checkoutResource(_id, eBoardResource_power);

    BDBG_MSG(("CControlNx::%s mode:S%d", BSTD_FUNCTION, mode));
    if (pPowerNx->getMode() == mode)
    {
        /* requested mode matches existing mode - still call setMode() which
         * will send out mode change notification that views may be waiting for */
        ret = pPowerNx->setMode(mode);
        CHECK_ERROR_GOTO("Unable to re-set power mode", ret, error);
        goto done;
    }

    switch (mode)
    {
    case ePowerMode_S0:
        /* On */
    {
        ret = pPowerNx->setMode(mode, pGraphics);
        CHECK_ERROR_GOTO("unable to set power mode S0", ret, error);

        if (NULL != pDisplayHD)
        {
            pDisplayHD->enableOutputs(true);
            pDisplayHD->waitForDisplaySettingsApply();
        }
        if (NULL != pDisplaySD)
        {
            pDisplaySD->enableOutputs(true);
            pDisplaySD->waitForDisplaySettingsApply();
        }

        ipServerStart();

        SET(_pCfg, FIRST_TUNE, "true");
    }
    break;

    case ePowerMode_S3:
    /*
     * Deep Sleep Standby
     * fall-through
     */

    case ePowerMode_S2:
        /*
         * Passive Standby
         * stop recordings and untune channels
         */
    {
        stopAllRecordings();
        stopAllEncodings();
        stopAllPlaybacks();
        /* Save last Channel before stopping everything */
        {
            CChannel * pChannel = _pModel->getCurrentChannel();

            if ((NULL != pChannel) && (true == pChannel->isTuned()))
            {
                /* before we untune, we will save the current "last" channel because
                 * the untune command will overwrite this with the current channel.
                 * we will then restore this saved last channel after transitioning
                 * to S0 mode.  see FIRST_TUNE */
                _pModel->saveLastTunedChannelPowerSave();
                ret = unTuneChannel(pChannel, true);
                CHECK_ERROR_GOTO("unable to untune channel", ret, error);
            }
        }
        unTuneAllChannels();
        ipServerStop();
    }
    /* fall-through */

    case ePowerMode_S1:
        /* Active Standby */
    {
        ret = showPip(false);
        CHECK_ERROR_GOTO("unable to hide PiP", ret, error);

        stopAllPlaybacks();

        /* untune live channel */
        {
            CChannel * pChannel = _pModel->getCurrentChannel();

            if ((NULL != pChannel) && (true == pChannel->isTuned()))
            {
                /* before we untune, we will save the current "last" channel because
                 * the untune command will overwrite this with the current channel.
                 * we will then restore this saved last channel after transitioning
                 * to S0 mode.  see FIRST_TUNE */
                _pModel->saveLastTunedChannelPowerSave();

                ret = unTuneChannel(pChannel, true);
                CHECK_ERROR_GOTO("unable to untune channel", ret, error);
            }
        }
    }

        if (NULL != pDisplayHD)
        {
            pDisplayHD->enableOutputs(false);
            pDisplayHD->waitForDisplaySettingsApply();
        }
        if (NULL != pDisplaySD)
        {
            pDisplaySD->enableOutputs(false);
            pDisplaySD->waitForDisplaySettingsApply();
        }

        ret = pPowerNx->setMode(mode, pGraphics);
        CHECK_ERROR_GOTO("unable to set power mode", ret, error);

        break;

    default:
        break;
    } /* switch */

error:
done:
    pBoardResources->checkinResource(pPowerNx);
#endif /* if POWERSTANDBY_SUPPORT */
    return(ret);
} /* setPower */

ePowerMode CControlNx::getPowerMode()
{
    ePowerMode mode = ePowerMode_Max;

#if POWERSTANDBY_SUPPORT
    CPowerNx *        pPowerNx        = NULL;
    CBoardResources * pBoardResources = NULL;

    pBoardResources = _pConfig->getBoardResources();
    BDBG_ASSERT(NULL != pBoardResources);
    pPowerNx = (CPowerNx *)pBoardResources->checkoutResource(_id, eBoardResource_power);
    if (pPowerNx == NULL)
    {
        mode = ePowerMode_S0;
        goto done;
    }

    mode = pPowerNx->getMode();

    pBoardResources->checkinResource(pPowerNx);
done:
#else /* if POWERSTANDBY_SUPPORT */
    mode = ePowerMode_S0;
#endif /* if POWERSTANDBY_SUPPORT */
    return(mode);
} /* getPowerMode */

int32_t CControlNx::getVolume(void)
{
    MList<COutput> * pAudioOutputList = _pModel->getAudioOutputList();
    COutput *        pAudioOutput     = NULL;
    int32_t          volume           = 0;
    eRet             ret              = eRet_Ok;

    CHECK_PTR_ERROR_GOTO("unable to get audio output list", pAudioOutputList, ret, eRet_InvalidState, error);

    pAudioOutput = (COutput *)pAudioOutputList->first();
    if (NULL != pAudioOutput)
    {
        CAudioVolume vol;

        vol = pAudioOutput->getVolume();

        volume = MAX(volume, vol._left);
        volume = MAX(volume, vol._right);
    }

error:
    return(volume);
} /* getVolume */

eRet CControlNx::setVolume(int32_t level)
{
    MList<COutput> * pAudioOutputList = _pModel->getAudioOutputList();
    COutput *        pAudioOutput     = NULL;
    eRet             ret              = eRet_Ok;

    CHECK_PTR_ERROR_GOTO("unable to get audio output list", pAudioOutputList, ret, eRet_InvalidState, error);

    pAudioOutput = (COutput *)pAudioOutputList->first();
    if (NULL != pAudioOutput)
    {
        CAudioVolume vol;

        BDBG_MSG(("setVolume for output:%s%d level:%x", pAudioOutput->getName(), pAudioOutput->getNumber(), level));
        vol             = pAudioOutput->getVolume();
        vol._volumeType = NEXUS_AudioVolumeType_eLinear;
        vol._left       = level;
        vol._right      = level;
        pAudioOutput->setVolume(vol);
    }

error:
    return(ret);
} /* setVolume */

bool CControlNx::getMute(void)
{
    MList<COutput> * pAudioOutputList = _pModel->getAudioOutputList();
    COutput *        pAudioOutput     = NULL;
    bool             bMuted           = false;
    eRet             ret              = eRet_Ok;

    CHECK_PTR_ERROR_GOTO("unable to get audio output list", pAudioOutputList, ret, eRet_InvalidState, error);

    pAudioOutput = (COutput *)pAudioOutputList->first();
    if (NULL != pAudioOutput)
    {
        CAudioVolume vol;

        vol     = pAudioOutput->getVolume();
        bMuted |= vol._muted;
    }

error:
    return(bMuted);
} /* getMute */

eRet CControlNx::setMute(bool muted)
{
    MList<COutput> * pAudioOutputList = _pModel->getAudioOutputList();
    COutput *        pAudioOutput     = NULL;
    eRet             ret              = eRet_Ok;

    CHECK_PTR_ERROR_GOTO("unable to get audio output list", pAudioOutputList, ret, eRet_InvalidState, error);

    pAudioOutput = (COutput *)pAudioOutputList->first();
    if (NULL != pAudioOutput)
    {
        BDBG_MSG(("setMute for output:%s%d mute:%d", pAudioOutput->getName(), pAudioOutput->getNumber(), muted));
        pAudioOutput->setMute(muted);
    }

error:
    return(ret);
} /* setMute */

eRet CControlNx::connectDecoders(
        CSimpleVideoDecode * pVideoDecode,
        CSimpleAudioDecode * pAudioDecode,
        uint32_t             width,
        uint32_t             height,
        CPid *               pVideoPid,
        eWindowType          winType
        )
{
    eRet                     ret       = eRet_Ok;
    NEXUS_Error              nerror    = NEXUS_SUCCESS;
    unsigned                 connectId = 0;
    NxClient_ConnectSettings settings;

    if (0 != _pModel->getConnectId(winType))
    {
        BDBG_MSG(("ignore connectDecoders call for windowType:%d", winType));
        return(ret);
    }

    BDBG_MSG(("ACCEPT connectDecoders call for windowType:%d", winType));

    NxClient_GetDefaultConnectSettings(&settings);

    /* call base class first */
    ret = CControl::connectDecoders(pVideoDecode, pAudioDecode, width, height, pVideoPid, winType);
    CHECK_ERROR_GOTO("unable to set max size in video decoder", ret, error);

    if (NULL != pVideoDecode)
    {
        int index = 0;

        if ((eWindowType_Mosaic1 <= winType) && (eWindowType_Max > winType))
        {
            /* disable closed caption routing in mosaic decoders */
            {
                CDisplayNx * pDisplayHD = (CDisplayNx *)_pModel->getDisplay(0);
                CDisplayNx * pDisplaySD = (CDisplayNx *)_pModel->getDisplay(1);

                if (NULL != pDisplayHD)
                {
                    CDisplayVbiData vbiData = pDisplayHD->getVbiSettings();
                    vbiData.bClosedCaptions = false;
                    ret                     = pDisplayHD->setVbiSettings(&vbiData);
                    CHECK_ERROR_GOTO("unable set disable closed caption passthru for mosaic channel", ret, error);
                }
                if (NULL != pDisplaySD)
                {
                    CDisplayVbiData vbiData = pDisplaySD->getVbiSettings();
                    vbiData.bClosedCaptions = false;
                    ret                     = pDisplaySD->setVbiSettings(&vbiData);
                    CHECK_ERROR_GOTO("unable set disable closed caption passthru for mosaic channel", ret, error);
                }
            }

            for (int i = eWindowType_Mosaic1; i < eWindowType_Max; i++)
            {
                CSimpleVideoDecode * pVideoDecodeMosaic = _pModel->getSimpleVideoDecode((eWindowType)i);

                if (NULL == pVideoDecodeMosaic)
                {
                    continue;
                }

                BDBG_MSG(("connecting MOSAIC%d", i));
                ((CSimpleVideoDecodeNx *)pVideoDecodeMosaic)->updateConnectSettings(&settings, index++);
            }
        }
        else
        {
            BDBG_MSG(("connecting single video decoder"));
            ((CSimpleVideoDecodeNx *)pVideoDecode)->updateConnectSettings(&settings, index++);
        }
    }

    if (NULL != pAudioDecode)
    {
        if ((eWindowType_Main == winType) || (eWindowType_Pip == winType))
        {
            ((CSimpleAudioDecodeNx *)pAudioDecode)->updateConnectSettings(&settings);
#if BDSP_MS12_SUPPORT
            /* if we are doing ms12 mixer based audio fading between main/pip, both audio
               decoders must be persistent */
            settings.simpleAudioDecoder.decoderCapabilities.type = NxClient_AudioDecoderType_ePersistent;
#endif
        }
    }

    nerror = NxClient_Connect(&settings, &connectId);
    CHECK_NEXUS_ERROR_GOTO("unable to connect to simple decoders", nerror, ret, error);

    if ((eWindowType_Mosaic1 <= winType) && (eWindowType_Max > winType))
    {
        for (int i = eWindowType_Mosaic1; i < eWindowType_Max; i++)
        {
            _pModel->setConnectId(connectId, (eWindowType)i);
        }
    }
    else
    {
        _pModel->setConnectId(connectId, winType);
    }

error:
    return(ret);
} /* connectDecoders */

void CControlNx::disconnectDecoders(eWindowType winType)
{
    uint32_t             connectId    = _pModel->getConnectId(winType);
    CSimpleVideoDecode * pVideoDecode = _pModel->getSimpleVideoDecode(winType);

    if (NULL != pVideoDecode)
    {
        pVideoDecode->setChannel(NULL);
    }

    if (0 == connectId)
    {
        return;
    }

    NxClient_Disconnect(connectId);

    if ((eWindowType_Mosaic1 <= winType) && (eWindowType_Max > winType))
    {
        for (int i = eWindowType_Mosaic1; i < eWindowType_Max; i++)
        {
            _pModel->setConnectId(0, (eWindowType)i);
        }
    }
    else
    {
        _pModel->setConnectId(0, winType);
    }
} /* disconnectDecoders() */

eRet CControlNx::setWindowGeometry()
{
    eRet ret = eRet_Ok;

    CSimpleVideoDecode * pVideoDecodeMain = _pModel->getSimpleVideoDecode(eWindowType_Main);
    CSimpleVideoDecode * pVideoDecodePip  = _pModel->getSimpleVideoDecode(eWindowType_Pip);
    MRect                rectFull(0, 0, 1000, 1000);

    if (NULL != pVideoDecodeMain) { pVideoDecodeMain->setVideoWindowGeometryPercent(&rectFull); }
    if (NULL != pVideoDecodePip) { pVideoDecodePip->setVideoWindowGeometryPercent(&rectFull); }

    /* setVideoWindGeometry will give you the correct error messages */
    return(ret);
}

#if BDSP_MS12_SUPPORT
/* if we have dual audio decoders, set master/mixing mode */
void CControlNx::setMixingMode(eWindowType windowType, NEXUS_AudioDecoderMixingMode mixingMode)
{
    CSimpleAudioDecode * pAudioDecode = _pModel->getSimpleAudioDecode(windowType);

    if ((NULL == _pModel->getSimpleAudioDecode(eWindowType_Main)) ||
        (NULL == _pModel->getSimpleAudioDecode(eWindowType_Pip)))
    {
        return;
    }

    if (NULL == pAudioDecode)
    {
        return;
    }

    if ((eWindowType_Main != windowType) && (eWindowType_Pip != windowType))
    {
        return;
    }

    pAudioDecode->setMaster((_pModel->getFullScreenWindowType() == windowType) ? true : false);
    //pAudioDecode->setMaster((eWindowType_Main == windowType) ? true : false);
    pAudioDecode->setMixingMode(mixingMode);
}
#endif

#if BDSP_MS12_SUPPORT
void CControlNx::setAudioFade(CSimpleAudioDecode * pAudioDecode, bool bPipState)
{
    CSimpleAudioDecode * pAudioDecodeFull = _pModel->getSimpleAudioDecode(_pModel->getFullScreenWindowType());
    CSimpleAudioDecode * pAudioDecodeDecimated = _pModel->getSimpleAudioDecode(_pModel->getPipScreenWindowType());
    int  duration       = GET_INT(_pCfg, AUDIO_DECODER_FADE_DURATION);
    int  levelFull      = (true == bPipState) ? GET_INT(_pCfg, AUDIO_DECODER_FADE_LEVEL_FULL) : 100;
    int  levelDecimated = GET_INT(_pCfg, AUDIO_DECODER_FADE_LEVEL_DECIMATED);
    eRet ret            = eRet_Ok;

    if ((NULL == pAudioDecode) || (NULL == pAudioDecodeFull) || (NULL == pAudioDecodeDecimated))
    {
        return;
    }

    if (pAudioDecode == pAudioDecodeFull)
    {
        ret = pAudioDecodeFull->setAudioFade(levelFull, duration);
        CHECK_WARN("error setting MAIN audio fade", ret);
    }
    else
    {
        ret = pAudioDecodeDecimated->setAudioFade(levelDecimated, duration);
        CHECK_WARN("error setting PIP audio fade", ret);
    }
}
#endif
#if BDSP_MS12_SUPPORT
/* if we have dual audio decoders, set audio fade. returns eRet_NotSupported if no change was made. */
eRet CControlNx::setAudioFade(bool bPipState)
{
    int  duration       = GET_INT(_pCfg, AUDIO_DECODER_FADE_DURATION);
    int  levelFull      = GET_INT(_pCfg, AUDIO_DECODER_FADE_LEVEL_FULL);
    int  levelDecimated = GET_INT(_pCfg, AUDIO_DECODER_FADE_LEVEL_DECIMATED);
    int  level          = 100;
    eRet ret            = eRet_NotSupported;

    CSimpleAudioDecode * pAudioDecodeMain = _pModel->getSimpleAudioDecode(eWindowType_Main);
    CSimpleAudioDecode * pAudioDecodePip  = _pModel->getSimpleAudioDecode(eWindowType_Pip);

    if ((NULL == pAudioDecodeMain) || (NULL == pAudioDecodePip))
    {
        return(ret);
    }

    if (true == bPipState)
    {
        level = (eWindowType_Pip == _pModel->getFullScreenWindowType()) ? levelDecimated : levelFull;
    }
    else
    {
        level = (eWindowType_Pip == _pModel->getFullScreenWindowType()) ? 0 : 100;
    }

    BDBG_MSG(("set audio fade main:%d pip:%d", level, 100-level));
    ret = pAudioDecodeMain->setAudioFade(level, duration);
    CHECK_WARN("error setting MAIN audio fade", ret);
    ret = pAudioDecodePip->setAudioFade(100 - level, duration);
    CHECK_WARN("error setting PIP audio fade", ret);

    return(ret);
}
#endif
#if BDSP_MS12_SUPPORT
void CControlNx::waitAudioFadeComplete(CSimpleAudioDecode * pAudioDecode)
{
    NEXUS_SimpleAudioDecoderSettings settings;
    eRet ret = eRet_Ok;

    if (NULL == pAudioDecode)
    {
        return;
    }

    NEXUS_SimpleAudioDecoder_GetSettings(pAudioDecode->getSimpleDecoder(), &settings);
    if (true == settings.processorSettings[NEXUS_SimpleAudioDecoderSelector_ePrimary].fade.connected)
    {
        ret = pAudioDecode->waitAudioFadeComplete();
        CHECK_ERROR("timeout waiting for previous audio fade to complete on decoder", ret);
    }

error:
    return;
}
#endif