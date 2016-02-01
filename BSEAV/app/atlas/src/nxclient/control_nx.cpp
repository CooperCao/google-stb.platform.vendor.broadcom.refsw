/***************************************************************************
 * (c) 2002-2015 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its
 * licensors, and may only be used, duplicated, modified or distributed pursuant
 * to the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and Broadcom
 * expressly reserves all rights in and to the Software and all intellectual
 * property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 *    constitutes the valuable trade secrets of Broadcom, and you shall use all
 *    reasonable efforts to protect the confidentiality thereof, and to use
 *    this information only in connection with your use of Broadcom integrated
 *    circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
 *    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
 *    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
 *    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
 *    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
 *    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
 *    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
 *    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
 *    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
 *    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 *****************************************************************************/

#include "nxclient.h"
#include "control_nx.h"
#include "output_nx.h"
#include "video_decode_nx.h"

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
    ret = pDisplay->setColorSpace(colorSpace);
    CHECK_ERROR_GOTO("unable to set color space", ret, error);

error:
    return(ret);
} /* setColorSpace */

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
    }
    else
    {
        /* hide pip */
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

    /* keep track of which content is full screen (main or pip) */
    _pModel->setFullScreenWindowType((eWindowType_Main == _pModel->getFullScreenWindowType()) ? eWindowType_Pip : eWindowType_Main);

    NxClient_GetDefaultReconfigSettings(&reconfigSettings);
    reconfigSettings.command[0].type       = NxClient_ReconfigType_eRerouteVideoAndAudio;
    reconfigSettings.command[0].connectId1 = pVideoDecodeMain->getConnectId();
    reconfigSettings.command[0].connectId2 = pVideoDecodePip->getConnectId();
    nerror = NxClient_Reconfig(&reconfigSettings);
    CHECK_NEXUS_ERROR_GOTO("unable to swap pip windows", ret, nerror, error);

    /* swap connect id's to match underlying nxclient connections */
    {
        uint32_t tmp = pVideoDecodeMain->getConnectId();
        pVideoDecodeMain->setConnectId(pVideoDecodePip->getConnectId());
        pVideoDecodePip->setConnectId(tmp);
    }

error:
    /* notify that pip state has changed - no actual change in pip state value */
    _pModel->setPipState(_pModel->getPipState());
    return(ret);
} /* swapPip() */

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