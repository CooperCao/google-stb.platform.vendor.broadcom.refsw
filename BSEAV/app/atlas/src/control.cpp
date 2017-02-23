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

#include "control.h"
#include "notification.h"
#include "atlas.h"
#include "channelmgr.h"
#include "channel_qam.h"
#include "playback.h"
#ifdef PLAYBACK_IP_SUPPORT
#include "servermgr.h"
#endif
#if DVR_LIB_SUPPORT
#include "tsb.h"
#endif
#include "record.h"
#include "remote.h"
#include "encode.h"
#include "screen.h"
#include "screen_main.h"
#ifdef DCC_SUPPORT
#include "closed_caption.h"
#endif
#ifdef POWERSTANDBY_SUPPORT
#include "pmlib.h"
#endif
#include "network.h"
#include "wifi.h"
#ifdef NETAPP_SUPPORT
#include "bluetooth.h"
#endif
#include "playlist.h"
#include "playlist_db.h"
#include "discovery.h"
#ifdef CPUTEST_SUPPORT
#include "cputest.h"
#endif

#include "nexus_parser_band.h"
#include "nexus_video_decoder.h"

BDBG_MODULE(atlas_control);

CControl::CControl(const char * strName) :
    CController(strName),
    _id(NULL),
    _pModel(NULL),
    _pConfig(NULL),
    _pChannelMgr(NULL),
    _pCfg(NULL),
    _pWidgetEngine(NULL),
    _deferredChannelUpDownTimer(this),
    _deferredChannel10KeyTimer(this),
    _deferredChannelPipTimer(this),
    _tunerLockCheckTimer(this),
    _powerOnTimer(this),
    _viewList(false),
    _recordingChannels(false),
    _encodingChannels(false)
{
    _viewList.clear();
    _recordingChannels.clear();
    _encodingChannels.clear();
    /* initialize notification filter */
}

CControl::~CControl()
{
    uninitialize();
}

eRet CControl::initialize(
        void *          id,
        CConfig *       pConfig,
        CChannelMgr *   pChannelMgr,
        CWidgetEngine * pWidgetEngine
        )
{
    eRet ret = eRet_Ok;

    BDBG_ASSERT(NULL != pConfig);
    BDBG_ASSERT(NULL != pChannelMgr);
    BDBG_ASSERT(NULL != pWidgetEngine);
    BDBG_ASSERT(NULL != _pModel);

    ATLAS_MEMLEAK_TRACE("BEGIN");

    _pConfig       = pConfig;
    _pChannelMgr   = pChannelMgr;
    _pCfg          = pConfig->getCfg();
    _pWidgetEngine = pWidgetEngine;
    _id            = id;

    _deferredChannelUpDownTimer.setWidgetEngine(_pWidgetEngine);
    _deferredChannelUpDownTimer.setTimeout(GET_INT(_pCfg, DEFERRED_CHANNEL_CHANGE_UP_DOWN_TIMEOUT));

    _deferredChannel10KeyTimer.setWidgetEngine(_pWidgetEngine);
    _deferredChannel10KeyTimer.setTimeout(GET_INT(_pCfg, DEFERRED_CHANNEL_CHANGE_10_KEY_TIMEOUT));

    _deferredChannelPipTimer.setWidgetEngine(_pWidgetEngine);
    _deferredChannelPipTimer.setTimeout(GET_INT(_pCfg, DEFERRED_CHANNEL_CHANGE_UP_DOWN_TIMEOUT));

    _tunerLockCheckTimer.setWidgetEngine(_pWidgetEngine);
    _tunerLockCheckTimer.setTimeout(GET_INT(_pCfg, TUNER_LOCK_CHECK_TIMEOUT));

    _powerOnTimer.setWidgetEngine(_pWidgetEngine);
    _powerOnTimer.setTimeout(GET_INT(_pCfg, POWER_ON_TIMEOUT));

    ATLAS_MEMLEAK_TRACE("END");
    return(ret);
} /* initialize */

eRet CControl::uninitialize()
{
    eRet ret = eRet_Ok;

    _pConfig       = NULL;
    _pChannelMgr   = NULL;
    _pCfg          = NULL;
    _pWidgetEngine = NULL;
    _id            = NULL;

    return(ret);
}

void CControl::addView(
        CView *      pView,
        const char * name
        )
{
    BDBG_ASSERT(NULL != pView);
    BDBG_ASSERT(NULL != name);

    CViewListNode * pNode = new CViewListNode(pView, name);
    BDBG_ASSERT(NULL != pNode);
    _viewList.add(pNode);
}

void CControl::removeView(CView * pView)
{
    MListItr <CViewListNode> itr(&_viewList);
    CViewListNode *          pNode = NULL;

    for (pNode = itr.first(); NULL != pNode; pNode = itr.next())
    {
        if (pNode->_pView == pView)
        {
            /* found it */
            _viewList.remove(pNode);
            DEL(pNode);
            break;
        }
    }
}

CView * CControl::findView(const char * name)
{
    MListItr <CViewListNode> itr(&_viewList);
    CViewListNode *          pNode = NULL;
    CView *                  pView = NULL;
    BDBG_ASSERT(name);

    for (pNode = itr.first(); NULL != pNode; pNode = itr.next())
    {
        if (pNode->_strName == name)
        {
            /* found it */
            pView = pNode->_pView;
            break;
        }
    }

    return(pView);
} /* findView */

/* at this point, all widgets/windows and bwidgets have decided to ignore this key press
 * so we get one last chance to respond to them */
eRet CControl::processKeyEvent(CRemoteEvent * pRemoteEvent)
{
    eRet ret = eRet_Ok;

    BDBG_ASSERT(NULL != pRemoteEvent);
    BDBG_ASSERT(NULL != _pWidgetEngine);

    if ((ePowerMode_S0 != getPowerMode()) &&
        (eKey_Power != pRemoteEvent->getCode()))
    {
        goto done;
    }

    switch (pRemoteEvent->getCode())
    {
    case eKey_ChannelUp:
        channelUp();
        break;

    case eKey_ChannelDown:
        channelDown();
        break;

    case eKey_VolumeUp:
    {
        int32_t vol = getVolume() + ((NEXUS_AUDIO_VOLUME_LINEAR_NORMAL - NEXUS_AUDIO_VOLUME_LINEAR_MIN)/ GET_INT(_pCfg, VOLUME_STEPS));
        setVolume(NEXUS_AUDIO_VOLUME_LINEAR_NORMAL > vol ? vol : NEXUS_AUDIO_VOLUME_LINEAR_NORMAL);
    }
    break;

    case eKey_VolumeDown:
    {
        int32_t vol = getVolume() - ((NEXUS_AUDIO_VOLUME_LINEAR_NORMAL - NEXUS_AUDIO_VOLUME_LINEAR_MIN) / GET_INT(_pCfg, VOLUME_STEPS));
        setVolume(NEXUS_AUDIO_VOLUME_LINEAR_MIN < vol ? vol : NEXUS_AUDIO_VOLUME_LINEAR_MIN);
    }
    break;

    case eKey_Mute:
        setMute(true == getMute() ? false : true);
        break;

    case eKey_Menu:
    {
        {
            eKey key = (eKey)pRemoteEvent->getCode();

            /* key needs to be handled irrespective of window focus */
            _pModel->sendGlobalKeyDown(&key);
        }
    }
    break;

    case eKey_Power:
    {
        if (ePowerMode_S0 == getPowerMode())
        {
            /* turn power OFF */
            eKey key = (eKey)pRemoteEvent->getCode();

            /* key needs to be handled irrespective of window focus */
            _pModel->sendGlobalKeyDown(&key);
        }
        else
        {
            /* turn power ON */
            setPowerMode(ePowerMode_S0);
        }
    }
    break;

    case eKey_Info:
        break;

    case eKey_Stop:
    {
        CChannel *  pChannel  = _pModel->getCurrentChannel();
        CPlayback * pPlayback = _pModel->getPlayback();

        if ((NULL != pPlayback) && (true == pPlayback->isActive()))
        {
            playbackStop();
        }
        else
        if (0 < _recordingChannels.total())
        {
            recordStop();
        }
        else
        if (0 < _encodingChannels.total())
        {
            encodeStop();
        }
        else
        if ((NULL != pChannel) &&
            (true == pChannel->isTuned()) &&
            (true == pChannel->isStopAllowed()))
        {
            /* stop is only allowed for channels that are not a part of the
             * channelmgr's channel lists.  one example of a stoppable channel
             * is an auto-discovered streaming channel */
            unTuneChannel(pChannel, true);
            tuneLastChannel();
        }
    }
    break;

    case eKey_FastForward:
    {
        CChannel *  pChannel  = _pModel->getCurrentChannel();
        CPlayback * pPlayback = _pModel->getPlayback();
#ifdef DVR_LIB_SUPPORT
        CTsb * pTsb = _pModel->getTsb();
        if ((NULL != pTsb) && (true == pTsb->isActive()))
        {
            pTsb->forward();
        }
        else
#endif /* ifdef DVR_LIB_SUPPORT */
        if ((NULL != pPlayback) && (true == pPlayback->isActive()))
        {
            pPlayback->trickMode(eKey_FastForward);
        }
        else
        if ((NULL != pChannel) && (true == pChannel->isTuned()))
        {
            pChannel->setTrickMode(true); /* sets class variables that all derived classes will use */
            pChannel->applyTrickMode();   /* derived classes do this */
        }
    }
    break;
    case eKey_Rewind:
    {
        CChannel *  pChannel  = _pModel->getCurrentChannel();
        CPlayback * pPlayback = _pModel->getPlayback();
#ifdef DVR_LIB_SUPPORT
        CTsb * pTsb = _pModel->getTsb();
        if ((NULL != pTsb) && (true == pTsb->isActive()))
        {
            pTsb->rewind();
        }
        else
#endif /* ifdef DVR_LIB_SUPPORT */
        if ((NULL != pPlayback) && (true == pPlayback->isActive()))
        {
            pPlayback->trickMode(eKey_Rewind);
        }
        else
        if ((NULL != pChannel) && (true == pChannel->isTuned()))
        {
            pChannel->setTrickMode(false); /* sets class variables that all derived classes will use */
            pChannel->applyTrickMode();    /* derived classes do this */
        }
    }
    break;
    case eKey_Play:
    {
        CChannel *  pChannel  = _pModel->getCurrentChannel();
        CPlayback * pPlayback = _pModel->getPlayback();
#ifdef DVR_LIB_SUPPORT
        CTsb * pTsb = _pModel->getTsb();
        if ((NULL != pTsb) && (true == pTsb->isActive()))
        {
            pTsb->play();
        }
        else
#endif /* ifdef DVR_LIB_SUPPORT */
        if ((NULL != pPlayback) && (true == pPlayback->isActive()))
        {
            ret = pPlayback->start();
            CHECK_ERROR_GOTO("unable to start playback", ret, error);
        }
        else
        if ((NULL != pChannel) && (true == pChannel->isTuned()))
        {
            pChannel->setTrickModeRate(1);
            ret = pChannel->play();
            CHECK_ERROR_GOTO("unable to start channel", ret, error);
        }
    }
    break;
    case eKey_Pause:
    {
        CChannel *  pChannel  = _pModel->getCurrentChannel();
        CPlayback * pPlayback = _pModel->getPlayback();

#ifdef DVR_LIB_SUPPORT
        CTsb * pTsb = _pModel->getTsb();
        if ((NULL != pTsb) && (true == pTsb->isActive()))
        {
            pTsb->pause();
        }
        else
#endif /* ifdef DVR_LIB_SUPPORT */
        if ((NULL != pPlayback) && (true == pPlayback->isActive()))
        {
            ret = pPlayback->pause();
            CHECK_ERROR_GOTO("unable to pause playback", ret, error);
        }
        else
        if ((NULL != pChannel) && (true == pChannel->isTuned()))
        {
            /* toggle pause/play */
            pChannel->setTrickModeRate((0 != pChannel->getTrickModeRate()) ? 0 : 1);

            if (0 == pChannel->getTrickModeRate())
            {
                ret = pChannel->pause();
                CHECK_ERROR_GOTO("unable to pause channel", ret, error);
            }
            else
            {
                ret = pChannel->play();
                CHECK_ERROR_GOTO("unable to play channel", ret, error);
            }
        }
    }
    break;
    case eKey_JumpFwd:
    {
        CChannel *  pChannel  = _pModel->getCurrentChannel();
        CPlayback * pPlayback = _pModel->getPlayback();

#ifdef DVR_LIB_SUPPORT
        CTsb * pTsb = _pModel->getTsb();
        if ((NULL != pTsb) && (true == pTsb->isActive()))
        {
            pTsb->slowForward();
        }
        else
#endif /* ifdef DVR_LIB_SUPPORT */
        if ((NULL != pPlayback) && (true == pPlayback->isActive()))
        {
            /* TODO: playback seek forward */
            pPlayback->trickMode(eKey_JumpFwd);
        }
        else
        if ((NULL != pChannel) && (true == pChannel->isTuned()))
        {
            pChannel->setTrickModeRate(1);
            pChannel->seek(true, _pCfg->getInt("TRICK_MODE_SEEK") /*seconds */);
        }
    }
    break;

    case eKey_JumpRev:
    {
        CChannel *  pChannel  = _pModel->getCurrentChannel();
        CPlayback * pPlayback = _pModel->getPlayback();

#ifdef DVR_LIB_SUPPORT
        CTsb * pTsb = _pModel->getTsb();
        if ((NULL != pTsb) && (true == pTsb->isActive()))
        {
            pTsb->slowRewind();
        }
        else
#endif /* ifdef DVR_LIB_SUPPORT */
        if ((NULL != pPlayback) && (true == pPlayback->isActive()))
        {
            pPlayback->trickMode(eKey_JumpRev);
        }
        else
        if ((NULL != pChannel) && (true == pChannel->isTuned()))
        {
            pChannel->setTrickModeRate(1);
            pChannel->seek(true, -(_pCfg->getInt("TRICK_MODE_SEEK")) /*seconds */);
        }
    }
    break;
    case eKey_Record:
    {
        {
            eKey key = (eKey)pRemoteEvent->getCode();

            /* key needs to be handled irrespective of window focus */
            _pModel->sendGlobalKeyDown(&key);
        }
    }
    break;
    case eKey_0:
    case eKey_1:
    case eKey_2:
    case eKey_3:
    case eKey_4:
    case eKey_5:
    case eKey_6:
    case eKey_7:
    case eKey_8:
    case eKey_9:
    case eKey_Dot:
        tenKey((eKey)pRemoteEvent->getCode());
        break;

    case eKey_Enter:
    {
        MString strChNum = _pModel->getDeferredChannelNum();

        if (true == strChNum.isEmpty())
        {
            tuneLastChannel();
        }
        else
        {
            tuneDeferredChannel();
        }
    }
    break;

    case eKey_Last:
        tuneLastChannel();
        break;

    case eKey_Pip:
        showPip(_pModel->getPipState() ? false : true);
        break;

    case eKey_Swap:
        swapPip();
        break;

    default:
        break;
    } /* switch */

error:
done:
    return(ret);
} /* processKeyEvent */

eRet CControl::unTuneAllChannels()
{
    CChannel * pChannel = NULL;
    eRet       ret      = eRet_Ok;

    /* untune current channels - these may not be a part of the channelmgr's
     * channel list. they may be streaming channels from a networked http/udp/rtp/rtsp server  */
    for (int i = 0; i < eWindowType_Max; i++)
    {
        pChannel = _pModel->getCurrentChannel((eWindowType)i);
        if (NULL != pChannel)
        {
            ret = unTuneChannel(pChannel, true, (eWindowType)i);
            CHECK_ERROR_GOTO("unable to untune channel", ret, error);
        }
    }

    /* untune all channels */
    while (NULL != (pChannel = _pChannelMgr->findTunedChannel()))
    {
        if (true == pChannel->isTuned())
        {
            ret = unTuneChannel(pChannel, true);
            CHECK_ERROR_GOTO("unable to untune channel", ret, error);
        }
    }

error:
    return(ret);
} /* unTuneAllChannels */

eRet CControl::stopAllPlaybacks()
{
    CPlayback * pPlayback = NULL;
    int         i         = 0;
    eRet        ret       = eRet_Ok;

    for (i = 0; i < eWindowType_Max; i++)
    {
        pPlayback = _pModel->getPlayback((eWindowType)i);

        if ((NULL != pPlayback) && (true == pPlayback->isActive()))
        {
            ret = playbackStop(NULL, (eWindowType)i, false);
            CHECK_ERROR_GOTO("unable to stop playback", ret, error);
        }
    }

error:
    return(ret);
} /* stopAllPlaybacks */

eRet CControl::stopAllRecordings()
{
    CChannel * pChannel = NULL;
    eRet       ret      = eRet_Ok;

    /* stop all records */
    for (pChannel = _recordingChannels.first(); pChannel; pChannel = _recordingChannels.next())
    {
        if (true == pChannel->isRecording())
        {
            ret = recordStop(pChannel);
            CHECK_ERROR_GOTO("unable to stop record", ret, error);
        }
    }

error:
    return(ret);
} /* stopAllRecordings */

eRet CControl::stopAllEncodings()
{
    eRet ret = eRet_Ok;

#if NEXUS_HAS_VIDEO_ENCODER
    CChannel *         pChannel = NULL;
    MListItr<CChannel> itr(&_encodingChannels);

    /* stop all encodes */
    for (pChannel = itr.first(); pChannel; pChannel = itr.next())
    {
        if (true == pChannel->isEncoding())
        {
            ret = encodeStop(pChannel);
            CHECK_ERROR_GOTO("unable to stop encode", ret, error);
        }
    }

error:
#endif /* if NEXUS_HAS_VIDEO_ENCODER */
    return(ret);
} /* stopAllEncodings */

void CControl::onIdle()
{
    /*
     * ***                      ***
     * *** CHECK FOR EXIT FIRST ***
     * ***                      ***
     */
    if (true == GET_BOOL(_pCfg, EXIT_APPLICATION))
    {
        showPip(false);
        stopAllRecordings();
        stopAllPlaybacks();
        stopAllEncodings();
        unTuneAllChannels();

        /* signal main loop to terminate */
        _pWidgetEngine->stop();

        /* we're quitting so skip subsequent idle loop processing */
        goto done;
    }

    if (ePowerMode_S0 != getPowerMode())
    {
        /* powered off so do nothing */
        goto done;
    }

    if (true == GET_BOOL(_pCfg, FIRST_TUNE))
    {
        CChannel * pChannel = NULL;
        BDBG_ASSERT(NULL != _pChannelMgr);
        pChannel = _pChannelMgr->getFirstChannel();

        SET(_pCfg, FIRST_TUNE, false);
        setWindowGeometry();

        if (NULL == pChannel)
        {
            /* verify default channel list file version */
            if (eRet_ExternalError == _pChannelMgr->verifyChannelListFile(GET_STR(_pCfg, CHANNELS_LIST)))
            {
                BDBG_WRN(("Invalid channel list version in file:%s.  Perform channel scan to generate a new channel list or copy default channel list file from release bundle.", GET_STR(_pCfg, CHANNELS_LIST)));
            }
        }
        else
        {
            CChannel * pChannelLast = _pModel->getLastTunedChannel();
            if (NULL != pChannelLast)
            {
                /* since last tuned channel exists, use that as the first tune.
                 * this can happen when coming out of standby. */
                pChannel = pChannelLast;
            }
            tuneChannel(pChannel, eWindowType_Main);
        }
    }
    else
    if (eMode_Live == _pModel->getMode())
    {
        /* tune attempt if untuned (every 2 secs) */
        if (true == GET_BOOL(_pCfg, ENABLE_IDLE_TUNE))
        {
            CPlayback * pPlayback = _pModel->getPlayback();
            CChannel *  pChannel  = NULL;

            if (NULL == pPlayback)
            {
                pChannel = _pModel->getCurrentChannel();
                if (NULL == pChannel)
                {
                    pChannel = _pChannelMgr->getFirstChannel();
                }
            }

            if ((NULL != pChannel) && (false == pChannel->isTuned()))
            {
                static B_Time timeLastTune = B_TIME_ZERO;
                B_Time        timeCurrent  = B_TIME_ZERO;

                B_Time_Get(&timeCurrent);

                if (2000 < B_Time_Diff(&timeCurrent, &timeLastTune))
                {
                    /* no playback and current channel is not tuned - retry */
                    BDBG_MSG(("tuning in idle loop channel:%s", pChannel->getChannelNum().s()));
                    tuneChannel(pChannel);
                    timeLastTune = timeCurrent;
                }
            }
        }
    }

done:
    return;
} /* onIdle */

/* check validity of notification based on mode.  return true if given notification
 * was valid.  returns false otherwise.  note that this method may make changes
 * to ensure the given notification is valid.  for example if we are in live mode
 * and tuned to a channel when a eNotify_PlaybackStart notification arrives,
 * this method will unTune and return true.
 */
bool CControl::validateNotification(
        CNotification & notification,
        eMode         mode
        )
{
    bool bValid = false;

    switch (mode)
    {
    case eMode_Live:
        bValid = true;
        break;

    case eMode_Scan:
        switch (notification.getId())
        {
        case eNotify_KeyUp:
        case eNotify_KeyDown:
        {
            CRemoteEvent * pRemoteEvent = (CRemoteEvent *)notification.getData();
            switch (pRemoteEvent->getCode())
            {
            case eKey_ChannelUp:
            case eKey_ChannelDown:
                /* no tuning during channel scan */
                break;
            default:
                bValid = true;
                break;
            }   /* switch */
        }
        break;
        case eNotify_ScanStop:
        case eNotify_ScanStopped:
            bValid = true;
            break;
        default:
        case eNotify_RecordStart:
        case eNotify_RecordStop:
        case eNotify_EncodeStart:
        case eNotify_EncodeStop:
            /* no tuning, playback, record, encode, or channel list changes
             * during channel scan */
            break;
        } /* switch */
        break;
    case eMode_Playback:
        switch (notification.getId())
        {
        case eNotify_KeyUp:
        case eNotify_KeyDown:
        {
            CRemoteEvent * pRemoteEvent = (CRemoteEvent *)notification.getData();
            switch (pRemoteEvent->getCode())
            {
            case eKey_0:
            case eKey_1:
            case eKey_2:
            case eKey_3:
            case eKey_4:
            case eKey_5:
            case eKey_6:
            case eKey_7:
            case eKey_8:
            case eKey_9:
                /* no 10 key tuning during playback */
                break;
            default:
                bValid = true;
                break;
            }   /* switch */
        }
        default:
            bValid = true;
            break;
        } /* switch */

        bValid = true;
        break;
    case eMode_Record:
        break;
    default:
        bValid = true;

        break;
    } /* switch */

    return(bValid);
} /* validateNotification */

void CControl::processNotification(CNotification & notification)
{
    eRet ret = eRet_Ok;

    if (false == validateNotification(notification, _pModel->getMode()))
    {
        /* notification consumed (invalid) so return */
        goto done;
    }

    /* handle key event notification */
    switch (notification.getId())
    {
    case eNotify_KeyUp:
    case eNotify_KeyDown:
        ret = processKeyEvent((CRemoteEvent *)notification.getData());
        break;

    case eNotify_VirtualKeyDown:
    {
        /* use any valid remote to enter remote command into bwidgets */
        CRemoteEvent * pRemoteEvent = (CRemoteEvent *)notification.getData();
        CIrRemote *    pIrRemote    = _pModel->getIrRemote();

        if (NULL != pIrRemote)
        {
            pIrRemote->submitCode(pRemoteEvent);
        }
        else
        {
            BDBG_WRN(("Unable to submit virtual keypress"));
        }
    }
    break;

    case eNotify_ChUp:
        channelUp();
        break;

    case eNotify_ChDown:
        channelDown();
        break;

#if RF4CE_SUPPORT
    case eNotify_AddRf4ceRemote:
    {
        CRf4ceRemoteData * pRf4ceData = (CRf4ceRemoteData *)notification.getData();
        addRf4ceRemote(pRf4ceData->_name);
    }
    break;
    case eNotify_DisplayRf4ceRemotes:
        displayRf4ceRemotes();
        break;
    case eNotify_RemoveRf4ceRemote:
    {
        CRf4ceRemoteData * pRf4ceData = (CRf4ceRemoteData *)notification.getData();
        removeRf4ceRemote(pRf4ceData->_pairingRefNum);
    }
    break;
#endif /* if RF4CE_SUPPORT */

    case eNotify_Tune:
    {
        CChannelData * pChannelData = (CChannelData *)notification.getData();
        CChannel *     pChannel     = NULL;

        if (NULL == pChannelData->_pChannel)
        {
            /* tune based on given channel number */

            /* if "0" we'll tune to the first channel */
            if (0 == pChannelData->_strChannel.toInt())
            {
                pChannel = _pChannelMgr->getFirstChannel();
                tuneChannel(pChannel, pChannelData->_windowType, pChannelData->_tunerIndex);
            }
            else /* if "-1" we'll simply untune */
            if (-1 == pChannelData->_strChannel.toInt())
            {
                unTuneChannel(NULL, true);
            }
            else
            {
                pChannel = _pChannelMgr->findChannel(pChannelData->_strChannel);
                tuneChannel(pChannel, pChannelData->_windowType, pChannelData->_tunerIndex);
            }
        }
        else
        {
            pChannel = pChannelData->_pChannel;

            if (-1 == pChannelData->_strChannel.toInt())
            {
                unTuneChannel(pChannel, false, pChannelData->_windowType);
            }
            else
            {
                tuneChannel(pChannel, pChannelData->_windowType, pChannelData->_tunerIndex);
            }
        }
    }
    break;

    case eNotify_NonTunerLockStatus:
    {
        CChannel * pChannel = (CChannel *)notification.getData();

        if ((NULL != pChannel))
        {
            CChannel * pChannelTuning = NULL;
            /* tuner locked so start decode */
            BDBG_MSG(("received Non tuner lock status channel:%s", boardResourceToString(pChannel->getType()).s()));

            /* look to see if channel associated with tune lock, maps to a previous tune attempt */
            for (int i = 0; i < eWindowType_Max; i++)
            {
                pChannelTuning = _pModel->getChannelTuneInProgress((eWindowType)i);

                if ((pChannelTuning) && (pChannelTuning == pChannel))
                {
                    /* tuner associated with tune lock status change maps to
                     * a channel that is tuning in progress */
                    ret = decodeChannel(pChannelTuning, (eWindowType)i);
                    CHECK_ERROR_GOTO("unable to start decoding channel", ret, error);
                    break;
                }
            }
        }
    }
    break;
    case eNotify_TunerLockStatus:
    {
#if NEXUS_HAS_FRONTEND
        CTuner * pTuner = (CTuner *)notification.getData();
        if ((NULL != pTuner) && (true == pTuner->getLockState()))
        {
            CChannel * pChannelTuning = NULL;
            /* tuner locked so start decode */

            /* look to see if channel associated with tune lock, maps to a previous tune attempt */
            for (int i = 0; i < eWindowType_Max; i++)
            {
                pChannelTuning = _pModel->getChannelTuneInProgress((eWindowType)i);

                if ((pChannelTuning) && (pChannelTuning->getTuner() == pTuner))
                {
                    /* tuner associated with tune lock status change maps to
                     * a channel that is tuning in progress */
                    ret = decodeChannel(pChannelTuning, (eWindowType)i);
                    CHECK_ERROR_GOTO("unable to start decoding channel", ret, error);
                    break;
                }
            }
        }
#endif /* if NEXUS_HAS_FRONTEND */
    }
    break;

    case eNotify_RefreshPlaybackList:
    {
        updatePlaybackList();
    }
    break;

#if NEXUS_HAS_FRONTEND
    case eNotify_ScanStart:
    {
        CTunerScanData * pScanData = (CTunerScanData *)notification.getData();
        scanTuner(pScanData);
    }
    break;
#endif /* if NEXUS_HAS_FRONTEND */

    case eNotify_ChannelListLoad:
    {
        CChannelMgrLoadSaveData * pLoadData = (CChannelMgrLoadSaveData *)notification.getData();
        int totalChannels                   = 0;
        BDBG_WRN(("Load channel list command (%s/%s) append:%d", pLoadData->_strFileName.s(), pLoadData->_strListName.s(), pLoadData->_append));

        showPip(false);

        /* Check the list of recorded channels and stop them */
        totalChannels = _recordingChannels.total();
        while (totalChannels)
        {
            recordStop();
            totalChannels--;
        }

        /* Stop encoding Channels */
        totalChannels = _encodingChannels.total();
        while (totalChannels)
        {
            encodeStop();
            totalChannels--;
        }

        for (int winType = 0; winType < eWindowType_Max; winType++)
        {
            CChannel * pChannel = _pModel->getCurrentChannel((eWindowType)winType);
            if (NULL != pChannel)
            {
                unTuneChannel(pChannel, true);
            }
        }

        _pModel->resetChannelHistory();
        if (eRet_Ok != _pChannelMgr->loadChannelList(pLoadData->_strFileName.s(), pLoadData->_append))
        {
            BDBG_WRN(("channel list XML version MISmatch (%s)", pLoadData->_strFileName.s()));
        }

        _pChannelMgr->dumpChannelList(true);
        tuneChannel();
    }
    break;

    case eNotify_PlaylistRemoved:
    {
        CPlaylist * pPlaylistRemoved = (CPlaylist *)notification.getData();
        CChannel *  pChannel         = NULL;

        /* only make changes if current channel is affected */
        eWindowType   windowTypeFull = _pModel->getFullScreenWindowType();
        CChannel *    pChCurrentFull = _pModel->getCurrentChannel(windowTypeFull);
        eWindowType   windowTypePip  = _pModel->getPipScreenWindowType();
        CChannel *    pChCurrentPip  = _pModel->getCurrentChannel(windowTypePip);
        CChannelMgr * pChannelMgr    = _pModel->getChannelMgr();
        int           i              = 0;

        for (pChannel = pPlaylistRemoved->getChannel(i); pChannel; pChannel = pPlaylistRemoved->getChannel(++i))
        {
            if (pChannel == pChCurrentFull)
            {
                recordStop(pChannel);
                encodeStop(pChannel);

                BDBG_WRN(("playlist containing currently tuned full screen channel removed - tuning to first channel in channel list..."));
                tuneChannel(pChannelMgr->getFirstChannel(windowTypeFull), windowTypeFull);
            }
            else
            if (pChannel == pChCurrentPip)
            {
                recordStop(pChannel);
                encodeStop(pChannel);

                BDBG_WRN(("playlist containing currently tuned pip channel removed - hiding pip..."));
                showPip(false);

                /* set current pip channel so next time pip is shown it will default to the first channel in channel list */
                _pModel->setCurrentChannel(pChannelMgr->getFirstChannel(windowTypePip), windowTypePip);
            }
        }
    }
    break;

    case eNotify_ChannelListChanged:
    {
        CChannelMgrListChangedData * pData = (CChannelMgrListChangedData *)notification.getData();

        if ((NULL == pData) || (false == pData->isAdd()))
        {
            /* channel list has been replaced or channels possibly deleted so reset current channel
             * to first in the channel list. */
            _pModel->setCurrentChannel(_pChannelMgr->getFirstChannel());
        }
    }
    break;

    case eNotify_ChannelListSave:
    {
        CChannelMgrLoadSaveData * pSaveData = (CChannelMgrLoadSaveData *)notification.getData();
        BDBG_WRN(("Save channel list command (%s/%s)", pSaveData->_strFileName.s(), pSaveData->_strListName.s()));

        _pChannelMgr->saveChannelList(pSaveData->_strFileName.s(), pSaveData->_append);
    }
    break;

    case eNotify_ChannelListDump:
    {
        _pChannelMgr->dumpChannelList(true);
    }
    break;

    case eNotify_SetAudioProgram:
    {
        uint16_t * pPid = (uint16_t *)notification.getData();
        CHECK_PTR_ERROR_GOTO("pid not specified in setAudioProgram notification", pPid, ret, eRet_InvalidParameter, error);

        setAudioProgram(*pPid);
    }
    break;

    case eNotify_SetAudioProcessing:
    {
        eAudioProcessing * pAudioProcessing = (eAudioProcessing *)notification.getData();
        CHECK_PTR_ERROR_GOTO("AudioProcessing not specified in setAudioProcessing notification", pAudioProcessing, ret, eRet_InvalidParameter, error);

        setAudioProcessing(*pAudioProcessing);
    }
    break;

#ifdef CPUTEST_SUPPORT
    case eNotify_SetCpuTestLevel:
    {
        int * pLevel = (int *)notification.getData();
        CHECK_PTR_ERROR_GOTO("Cpu Test Level not specified in setCpuTestLevel notification", pLevel, ret, eRet_InvalidParameter, error);

        setCpuTestLevel(*pLevel);
    }
    break;
#endif /* ifdef CPUTEST_SUPPORT */

    case eNotify_SetSpdifInput:
    {
        eSpdifInput * pSpdifInput = (eSpdifInput *)notification.getData();
        CHECK_PTR_ERROR_GOTO("Spdif input not specified in setSpdifInput notification", pSpdifInput, ret, eRet_InvalidParameter, error);

        setSpdifInput(*pSpdifInput);
    }
    break;

    case eNotify_SetHdmiAudioInput:
    {
        eHdmiAudioInput * pHdmiInput = (eHdmiAudioInput *)notification.getData();
        CHECK_PTR_ERROR_GOTO("Hdmi input not specified in setHdmiAudioInput notification", pHdmiInput, ret, eRet_InvalidParameter, error);

        setHdmiAudioInput(*pHdmiInput);
    }
    break;

    case eNotify_SetAudioDownmix:
    {
        eAudioDownmix * pDownmix = (eAudioDownmix *)notification.getData();
        CHECK_PTR_ERROR_GOTO("Audio downmix not specified in setAudioDownmix notification", pDownmix, ret, eRet_InvalidParameter, error);

        setAudioDownmix(*pDownmix);
    }
    break;

    case eNotify_SetAudioDualMono:
    {
        eAudioDualMono * pDualMono = (eAudioDualMono *)notification.getData();
        CHECK_PTR_ERROR_GOTO("Audio DualMono not specified in setAudioDualMono notification", pDualMono, ret, eRet_InvalidParameter, error);

        setAudioDualMono(*pDualMono);
    }
    break;

    case eNotify_SetDolbyDRC:
    {
        eDolbyDRC * pDolbyDRC = (eDolbyDRC *)notification.getData();
        CHECK_PTR_ERROR_GOTO("Dolby DRC not specified in setDolbyDRC notification", pDolbyDRC, ret, eRet_InvalidParameter, error);

        setDolbyDRC(*pDolbyDRC);
    }
    break;

    case eNotify_SetDolbyDialogNorm:
    {
        bool * pDolbyDialogNorm = (bool *)notification.getData();
        CHECK_PTR_ERROR_GOTO("Dolby Dialog Normalization not specified in setDolbyDialogNorm notification", pDolbyDialogNorm, ret, eRet_InvalidParameter, error);

        setDolbyDialogNorm(*pDolbyDialogNorm);
    }
    break;

    case eNotify_SetContentMode:
    {
        NEXUS_VideoWindowContentMode * pContentMode = (NEXUS_VideoWindowContentMode *)notification.getData();
        CHECK_PTR_ERROR_GOTO("Content Mode not specified in setContentMode notification", pContentMode, ret, eRet_InvalidParameter, error);

        setContentMode(*pContentMode);
    }
    break;

    case eNotify_SetColorSpace:
    {
        NEXUS_ColorSpace * pColorSpace = (NEXUS_ColorSpace *)notification.getData();
        CHECK_PTR_ERROR_GOTO("Color space not specified in setColorSpace notification", pColorSpace, ret, eRet_InvalidParameter, error);

        setColorSpace(*pColorSpace);
    }
    break;

    case eNotify_SetColorDepth:
    {
        uint8_t * pColorDepth = (uint8_t *)notification.getData();
        CHECK_PTR_ERROR_GOTO("Color depth not specified in setColorDepth notification", pColorDepth, ret, eRet_InvalidParameter, error);

        setColorDepth(*pColorDepth);
    }
    break;

    case eNotify_SetMpaaDecimation:
    {
        bool * pMpaaDecimation = (bool *)notification.getData();
        CHECK_PTR_ERROR_GOTO("MPAA decimation not specified in setMpaaDecimation notification", pMpaaDecimation, ret, eRet_InvalidParameter, error);

        setMpaaDecimation(*pMpaaDecimation);
    }
    break;

    case eNotify_SetDeinterlacer:
    {
        bool * pDeinterlacer = (bool *)notification.getData();
        CHECK_PTR_ERROR_GOTO("Deinterlace setting not specified in setDeinterlacer notification", pDeinterlacer, ret, eRet_InvalidParameter, error);

        setDeinterlacer(*pDeinterlacer);
    }
    break;

    case eNotify_SetBoxDetect:
    {
        bool * pBoxDetect = (bool *)notification.getData();
        CHECK_PTR_ERROR_GOTO("BoxDetect setting not specified in setBoxDetect notification", pBoxDetect, ret, eRet_InvalidParameter, error);

        setBoxDetect(*pBoxDetect);
    }
    break;

    case eNotify_SetAspectRatio:
    {
        NEXUS_DisplayAspectRatio * pAspectRatio = (NEXUS_DisplayAspectRatio *)notification.getData();
        CHECK_PTR_ERROR_GOTO("AspectRatio setting not specified in setAspectRatio notification", pAspectRatio, ret, eRet_InvalidParameter, error);

        setAspectRatio(*pAspectRatio);
    }
    break;

    case eNotify_SetVideoFormat:
    {
        NEXUS_VideoFormat * pVideoFormat = (NEXUS_VideoFormat *)notification.getData();
        CHECK_PTR_ERROR_GOTO("Video format not specified in setVideoFormat notification", pVideoFormat, ret, eRet_InvalidParameter, error);

        setVideoFormat(*pVideoFormat);
    }
    break;

    case eNotify_SetAutoVideoFormat:
    {
        bool * pAutoVideoFormat = (bool *)notification.getData();
        CHECK_PTR_ERROR_GOTO("Video format not specified in setAutoVideoFormat notification", pAutoVideoFormat, ret, eRet_InvalidParameter, error);

        setAutoVideoFormat(*pAutoVideoFormat);
    }
    break;

    case eNotify_PlaybackListDump:
    {
        dumpPlaybackList(true);
    }
    break;

    case eNotify_PlaybackStart:
    {
        CPlaybackData * pSaveData = (CPlaybackData *)notification.getData();
        BDBG_MSG(("Playback File (%s/%s) Video Path (%s), Video 0x%p", pSaveData->_strFileName.s(), pSaveData->_strIndexName.s(), pSaveData->_strPath.s(), (void *)pSaveData->_video));

        if (pSaveData->_video)
        {
            playbackStart(pSaveData->_video);
        }
        else
        {
            playbackStart(pSaveData->_strFileName, pSaveData->_strIndexName, pSaveData->_strPath);
        }
    }
    break;

    case eNotify_PlaybackTrickMode:
    {
        CPlayback *          pPlayback = _pModel->getPlayback();
        CChannel *           pChannel  = _pModel->getCurrentChannel();
        CPlaybackTrickData * pSaveData = (CPlaybackTrickData *)notification.getData();

        if (pSaveData && pPlayback && pPlayback->isActive())
        {
            pPlayback->trickMode(pSaveData);
        }
        else
        if ((NULL != pChannel) && (true == pChannel->isTuned()) && (true == pChannel->timelineSupport()))
        {
            pChannel->trickmode(pSaveData);
        }
    }
    break;

    case eNotify_PlaybackStop:
    {
        CPlayback *     pPlayback = _pModel->getPlayback();
        CPlaybackData * pSaveData = (CPlaybackData *)notification.getData();

        BDBG_MSG(("Stop Playback File (%s) ", pSaveData->_strFileName.s()));
        if (pPlayback && pPlayback->isActive())
        {
            playbackStop(pSaveData->_strFileName, _pModel->getFullScreenWindowType(), pSaveData->_bTuneLastChannel);
        }
    }
    break;

    case eNotify_SetVolume:
    {
        int * pPercent = (int *)notification.getData();
        setVolume(NEXUS_AUDIO_VOLUME_LINEAR_NORMAL * *pPercent / 100);
    }
    break;

    case eNotify_SetMute:
    {
        bool * pMute = (bool *)notification.getData();
        setMute(*pMute);
    }
    break;

    case eNotify_RecordStart:
    {
        CRecordData * pSaveData = (CRecordData *)notification.getData();
        if (pSaveData != NULL)
        {
            recordStart(pSaveData);
        }
        else
        {
            recordStart();
        }
    }
    break;

    case eNotify_RecordStop:
    {
        CRecordData * pSaveData = (CRecordData *)notification.getData();

        BDBG_WRN(("Stop Record of File (%s) ", pSaveData->_strFileName.s()));
        recordStop();
    }
    break;

    case eNotify_EncodeStart:
    {
#if NEXUS_HAS_VIDEO_ENCODER
        CTranscodeData * pEncodeData = (CTranscodeData *)notification.getData();
        if (pEncodeData != NULL)
        {
            encodeStart(pEncodeData->_strFileName.s(), pEncodeData->_strPath.s());
        }
#endif /* if NEXUS_HAS_VIDEO_ENCODER */
    }
    break;

    case eNotify_EncodeStop:
    {
#if NEXUS_HAS_VIDEO_ENCODER
        encodeStop();
#endif
    }
    break;

#if NEXUS_HAS_FRONTEND
    case eNotify_ScanStopped:
    {
        CTunerScanNotificationData * pScanData       = (CTunerScanNotificationData *)notification.getData();
        CBoardResources *            pBoardResources = _pConfig->getBoardResources();

        /* the scan is done - check tuner back in */
        BDBG_ASSERT(NULL != pScanData->getTuner());
        pBoardResources->checkinResource(pScanData->getTuner());

        _pModel->setMode(eMode_Invalid);

        if (false == pScanData->getAppendToChannelList())
        {
            /* channel list was replaced so set a new current channel */
            _pModel->setCurrentChannel(_pChannelMgr->getFirstChannel());
        }

        _pChannelMgr->dumpChannelList(true);
        tuneChannel();
    }
    break;
#endif /* if NEXUS_HAS_FRONTEND */

    case eNotify_VideoSourceChanged:
    {
        CSimpleVideoDecode * pVideoDecode = (CSimpleVideoDecode *)notification.getData();
        CDisplay *           pDisplay     = (CDisplay *)_pModel->getDisplay();

        BDBG_ASSERT(NULL != pVideoDecode);

        if ((NULL != pDisplay) && (true == pDisplay->isAutoFormat()))
        {
            ret = setOptimalVideoFormat(pDisplay, pVideoDecode);
            CHECK_ERROR_GOTO("unable to set optimal video format", ret, error);
        }
    }
    break;

    case eNotify_VideoFormatChanged:
    {
        CDisplay * pDisplay = (CDisplay *)_pModel->getDisplay();
        if (NULL != pDisplay)
        {
            /* if our video format is unsupported by component outputs, they need to be moved
             * from the HD display to the SD display. */
            NEXUS_VideoFormat format = pDisplay->getFormat();
            eRet              ret    = eRet_Ok;

            ret = setComponentDisplay(format);
            CHECK_ERROR("Unable to attach component output to proper display", ret);
        }
    }
    break;

    case eNotify_HdmiHotplugEvent:
    {
        COutputHdmi *        pOutput      = (COutputHdmi *)notification.getData();
        CDisplay *           pDisplay     = _pModel->getDisplay();
        CGraphics *          pGraphics    = _pModel->getGraphics();
        CSimpleVideoDecode * pVideoDecode = _pModel->getSimpleVideoDecode();
        BDBG_ASSERT(NULL != pOutput);

        CHECK_PTR_ERROR_GOTO("unable to get display", pDisplay, ret, eRet_NotAvailable, error);
        CHECK_PTR_ERROR_GOTO("unable to get graphics", pGraphics, ret, eRet_NotAvailable, error);
        CHECK_PTR_ERROR_GOTO("unable to get video decoder", pVideoDecode, ret, eRet_NotAvailable, error);

        if (true == pDisplay->isAutoFormat())
        {
            BDBG_MSG(("Auto video format set - ignore HDMI preferred format"));
            /* auto format is set so we will ignore hdmi preferred format */
            ret = setOptimalVideoFormat(pDisplay, pVideoDecode);
            CHECK_ERROR_GOTO("unable to set optimal video format", ret, error);
        }
        else
        {
            if (true == pOutput->isConnected())
            {
                /* connected */
                NEXUS_VideoFormat format = NEXUS_VideoFormat_eUnknown;

                /* set hdmi preferred format */
                format = pOutput->getPreferredVideoFormat();

                BDBG_MSG(("Set HDMI preferred video format: %s", videoFormatToString(format).s()));
                if (NEXUS_VideoFormat_eUnknown != format)
                {
                    setVideoFormat(format);
                }
            }
            else
            {
                BDBG_MSG(("Hdmi disconnected - Set atlas.cfg preferred video format:%s", GET_STR(_pCfg, PREFERRED_FORMAT_HD)));
                /* disconnected - revert to default preferred HD format */
                setVideoFormat(stringToVideoFormat(GET_STR(_pCfg, PREFERRED_FORMAT_HD)));
            }
        }
    }
    break;

    case eNotify_ShowPip:
    {
        bool * pShow = (bool *)notification.getData();
        showPip(*pShow);
    }
    break;

    case eNotify_SwapPip:
        swapPip();
        break;

    case eNotify_Exit:
        SET(_pCfg, EXIT_APPLICATION, "true");
        break;

    case eNotify_Timeout:
    {
        CTimer * pTimer = (CTimer *)notification.getData();

        if ((pTimer == &_deferredChannelUpDownTimer) ||
            (pTimer == &_deferredChannel10KeyTimer))
        {
            ret = tuneDeferredChannel();
            CHECK_WARN_GOTO("Unable to tune to deferred channel", ret, error);
        }
        else
        if (pTimer == &_deferredChannelPipTimer)
        {
            eWindowType windowType = _pModel->getPipScreenWindowType();

            ret = tuneDeferredChannel(windowType);
            CHECK_WARN_GOTO("Unable to tune to deferred channel", ret, error);
        }
        else
        if (pTimer == &_tunerLockCheckTimer)
        {
            BDBG_WRN(("tuner lock check timer fired"));
        }
        else
        if (pTimer == &_powerOnTimer)
        {
            BDBG_WRN(("Power ON timer fired"));
            ret = setPowerMode(ePowerMode_S0);
            CHECK_ERROR_GOTO("unable to power ON!", ret, error);
        }
    }
    break;

    case eNotify_SetVbiSettings:
    {
        eRet              ret        = eRet_Ok;
        CDisplayVbiData * pVbiData   = (CDisplayVbiData *)notification.getData();
        CDisplay *        pDisplayHD = _pModel->getDisplay(0);
        CDisplay *        pDisplaySD = _pModel->getDisplay(1);

        if (NULL != pDisplaySD)
        {
            ret = pDisplaySD->setVbiSettings(pVbiData);
            CHECK_ERROR_GOTO("unable to set SD display VBI settings", ret, error);
        }
        else
        {
            BDBG_WRN(("SD display does not exist - unable to set VBI settings for this display"));
        }

        if (NULL != pDisplayHD)
        {
            /* only set macrovision setting for HD display */
            CDisplayVbiData vbiDataCurrent = pDisplayHD->getVbiSettings();

            vbiDataCurrent.bMacrovision    = pVbiData->bMacrovision;
            vbiDataCurrent.macrovisionType = pVbiData->macrovisionType;
            ret                            = pDisplayHD->setVbiSettings(&vbiDataCurrent);
            CHECK_ERROR_GOTO("unable to set HD display VBI settings", ret, error);
        }
        else
        {
            BDBG_WRN(("HD display does not exist - unable to set VBI settings for this display"));
        }
    }
    break;

    case eNotify_SetPowerMode:
    {
        eRet       ret       = eRet_Ok;
        ePowerMode powerMode = *((ePowerMode *)notification.getData());
        ret = setPowerMode(powerMode);
        CHECK_ERROR_GOTO("unable to set power mode", ret, error);
    }
    break;

    case eNotify_ipClientTranscodeEnable:
    {
        bool * transcodeEnabled;
        transcodeEnabled = (bool *)notification.getData();
        _pModel->setIpClientTranscodeEnabled(*transcodeEnabled);
        if (*transcodeEnabled)
        {
            BDBG_WRN(("Transcoding is enabled for the IP streaming for this client"));
        }
        else
        {
            BDBG_WRN(("Transcoding is disabled for the IP streaming for this client"));
        }
    }
    break;

    case eNotify_ipClientTranscodeProfile:
    {
        int * transcodeProfile = (int *)notification.getData();
        BDBG_WRN(("You entered %d", *transcodeProfile));

        if (*transcodeProfile == 1)
        {
            BDBG_WRN(("Transcode profile is set to 480p30 for the next IP streaming for this client"));
        }
        else
        if (*transcodeProfile == 2)
        {
            BDBG_WRN(("Transcode profile is set to 720p30 for the next IP streaming for this client"));
        }
        else
        {
            BDBG_WRN(("Invalid Transcode profile. Please enter 1 for 480p30 OR 2 for 720p30 profile"));
        }

        _pModel->setIpClientTranscodeProfile(*transcodeProfile);
    }
    break;

#ifdef DCC_SUPPORT
    case eNotify_ClosedCaptionEnable:
    {
        if (true == GET_BOOL(_pCfg, DCC_ENABLED))
        {
            CClosedCaption *    pClosedCaption = _pModel->getClosedCaption();
            bool *              ccEnabled;
            digitalCC_setting * dccSetting = pClosedCaption->dcc_getsetting();
            ccEnabled           = (bool *)notification.getData();
            dccSetting->enabled = *ccEnabled;
            pClosedCaption->dcc_set(dccSetting);
        }
        break;
    }
    case eNotify_ClosedCaptionMode:
    {
        if (true == GET_BOOL(_pCfg, DCC_ENABLED))
        {
            CClosedCaption *    pClosedCaption = _pModel->getClosedCaption();
            B_Dcc_Type *        ccMode;
            digitalCC_setting * dccSetting = pClosedCaption->dcc_getsetting();
            ccMode             = (B_Dcc_Type *)notification.getData();
            dccSetting->ccMode = *ccMode;
            pClosedCaption->dcc_set(dccSetting);
        }
        break;
    }
#endif /* ifdef DCC_SUPPORT */

#if defined (NETAPP_SUPPORT) || defined (WPA_SUPPLICANT_SUPPORT)
    case eNotify_NetworkWifiScanStart:
    case eNotify_NetworkWifiScanResultRetrieve:
    case eNotify_NetworkWifiScanStop:
    case eNotify_NetworkWifiConnect:
    case eNotify_NetworkWifiDisconnect:
    case eNotify_NetworkWifiConnectionStatus:
    {
        CBoardResources * pBoardResources = _pConfig->getBoardResources();
#ifdef NETAPP_SUPPORT
        CNetwork * pNetwork = (CNetwork *)pBoardResources->checkoutResource(_id, eBoardResource_network);
#elif WPA_SUPPLICANT_SUPPORT
        CWifi * pNetwork = (CWifi *)pBoardResources->checkoutResource(_id, eBoardResource_wifi);
#endif
        CHECK_PTR_ERROR_GOTO("unable to checkout network resource", pNetwork, ret, eRet_NotAvailable, errorWifi);

        switch (notification.getId())
        {
        case eNotify_NetworkWifiScanStart:
            pNetwork->startScanWifi();
            break;

#ifdef WPA_SUPPLICANT_SUPPORT
        case eNotify_NetworkWifiScanResultRetrieve:
            pNetwork->retrieveScanResults();
            break;
#endif
        case eNotify_NetworkWifiScanStop:
            pNetwork->stopScanWifi();
            break;

        case eNotify_NetworkWifiConnect:
        {
            CNetworkWifiConnectData * pConnectData = (CNetworkWifiConnectData *)notification.getData();

            pNetwork->stopScanWifi();

#ifdef WPA_SUPPLICANT_SUPPORT
            if (true == pConnectData->isWps())
            {
                ret = pNetwork->connectWps();
                CHECK_ERROR_GOTO("unable to connect to wifi wps", ret, errorWifi);
            }
            else
#endif
            {
                ret = pNetwork->connectWifi(pConnectData->_strSsid, pConnectData->_strPassword);
                CHECK_ERROR_GOTO("unable to connect to wifi", ret, errorWifi);
            }
        }
        break;

        case eNotify_NetworkWifiDisconnect:
        {
            CNetworkWifiConnectData * pConnectData = (CNetworkWifiConnectData *)notification.getData();

#ifdef WPA_SUPPLICANT_SUPPORT
            if (true == pConnectData->isWps())
            {
                ret = pNetwork->disconnectWps();
                CHECK_ERROR_GOTO("unable to disconnect to wifi wps", ret, errorWifi);
            }
            else
#endif
            {
                pNetwork->stopScanWifi();
                ret = pNetwork->disconnectWifi();
                CHECK_ERROR_GOTO("unable to disconnect from wifi", ret, errorWifi);
            }
        }
        break;

        case eNotify_NetworkWifiConnectionStatus:
        {
#ifdef PLAYBACK_IP_SUPPORT
            /* ip address updated from dhcp */
            CAutoDiscoveryServer * pAutoDiscoveryServer = _pModel->getAutoDiscoveryServer();
            CAutoDiscoveryClient * pAutoDiscoveryClient = _pModel->getAutoDiscoveryClient();
            if (NULL != pAutoDiscoveryServer)
            {
                ret = pAutoDiscoveryServer->updateIfAddrs();
                CHECK_ERROR("unable update ip address in auto discovery server", ret);
            }
            if (NULL != pAutoDiscoveryClient)
            {
                ret = pAutoDiscoveryClient->updateIfAddrs();
                CHECK_ERROR("unable update ip address in auto discovery client", ret);
            }
#endif /* ifdef PLAYBACK_IP_SUPPORT */
        }
        break;
        default:
            break;
        }   /* switch */

errorWifi:
        pBoardResources->checkinResource(pNetwork);
        pNetwork = NULL;
    }
#endif /* if defined (NETAPP_SUPPORT) || defined (WPA_SUPPLICANT_SUPPORT) */
        break;

#ifdef NETAPP_SUPPORT
    case eNotify_BluetoothDiscoveryStart:
    case eNotify_BluetoothDiscoveryStop:
    case eNotify_BluetoothConnect:
    case eNotify_BluetoothDisconnect:
    case eNotify_BluetoothGetSavedBtListInfo:
    case eNotify_BluetoothGetDiscBtListInfo:
    case eNotify_BluetoothGetConnBtListInfo:
    case eNotify_BluetoothConnectBluetoothFromDiscList:
    case eNotify_BluetoothA2DPStart:
    case eNotify_BluetoothA2DPStop:
    {
        CBoardResources * pBoardResources = _pConfig->getBoardResources();
        CBluetooth *      pBluetooth      = (CBluetooth *)pBoardResources->checkoutResource(_id, eBoardResource_bluetooth);
        CHECK_PTR_ERROR_GOTO("unable to checkout bluetooth resource", pBluetooth, ret, eRet_NotAvailable, errorBluetooth);

        switch (notification.getId())
        {
        case eNotify_BluetoothDiscoveryStart:
            pBluetooth->startDiscoveryBluetooth();
            break;

        case eNotify_BluetoothDiscoveryStop:
            pBluetooth->stopDiscoveryBluetooth();
            break;

        case eNotify_BluetoothConnect:
        {
            pBluetooth->stopDiscoveryBluetooth();
            CBluetoothConnectionData * pConnectionData = (CBluetoothConnectionData *)notification.getData();
            ret = pBluetooth->connectBluetooth(pConnectionData->index, pConnectionData->pDevInfoList);
            /* ret = pBluetooth->connectBluetooth(pConnectionData); */
            CHECK_ERROR_GOTO("unable to connect to bluetooth device", ret, errorBluetooth);
        }
        break;

        case eNotify_BluetoothDisconnect:
        {
            CBluetoothConnectionData * pConnectionData = (CBluetoothConnectionData *)notification.getData();
            ret = pBluetooth->disconnectBluetooth(pConnectionData->index);
            CHECK_ERROR_GOTO("unable to disconnect from bluetooth device", ret, errorBluetooth);
        }
        break;
        case eNotify_BluetoothGetSavedBtListInfo:
        {
            uint32_t             ulCount = 0;
            NETAPP_BT_DEV_INFO * pInfo   = NULL;
            ret = pBluetooth->getSavedBtListInfo(&ulCount, &pInfo);
            CHECK_ERROR_GOTO("unable to getSavedBtListInfo ", ret, errorBluetooth);
        }
        break;
        case eNotify_BluetoothGetDiscBtListInfo:
        {
            uint32_t             ulCount = 0;
            NETAPP_BT_DEV_INFO * pInfo   = NULL;
            ret = pBluetooth->getDiscoveryBtListInfo(&ulCount, &pInfo);
            CHECK_ERROR_GOTO("unable to getDiscoveryBtListInfo ", ret, errorBluetooth);
        }
        break;
        case eNotify_BluetoothGetConnBtListInfo:
        {
            uint32_t             ulCount = 0;
            NETAPP_BT_DEV_INFO * pInfo   = NULL;
            ret = pBluetooth->updateConnectedBtList(&ulCount, &pInfo);
            CHECK_ERROR_GOTO("unable to getConnectedBtListInfo ", ret, errorBluetooth);
        }
        break;
        case eNotify_BluetoothConnectBluetoothFromDiscList:
        {
            CBluetoothConnectionData * pConnectionData = (CBluetoothConnectionData *)notification.getData();
            ret = pBluetooth->connectBluetoothFromDiscList(pConnectionData->index);
            CHECK_ERROR_GOTO("unable to connectBluetoothFromDiscList from bluetooth", ret, errorBluetooth);
        }
        break;
        case eNotify_BluetoothA2DPStart:
        {
            CAudioCapture * pAudioCapture = _pModel->getAudioCapture();
            ret = pAudioCapture->start();
            if (ret)
            {
                CHECK_ERROR("unable to start Audio Capture from bluetooth", ret);
                pAudioCapture->stop();
            }
            /* need to wait for  Bluetooth to be connected ???*/
            BKNI_Sleep(3000);
            ret = pBluetooth->startAV();
            if (ret)
            {
                CHECK_ERROR("unable to start AV send from bluetooth", ret);
                pBluetooth->stopAV();
            }
        }
        break;
        case eNotify_BluetoothA2DPStop:
        {
            CAudioCapture * pAudioCapture = _pModel->getAudioCapture();

            ret = pBluetooth->stopAV();
            CHECK_ERROR_GOTO("unable to stop AV send from bluetooth", ret, errorBluetooth);

            ret = pAudioCapture->stop();
            CHECK_ERROR_GOTO("unable to stop  Audio Capture from bluetooth", ret, errorBluetooth);
        }
        break;
        default:
            break;
        }   /* switch */

errorBluetooth:
        pBoardResources->checkinResource(pBluetooth);
        pBluetooth = NULL;
    }
    break;
#endif /* ifdef NETAPP_SUPPORT */

#ifdef PLAYBACK_IP_SUPPORT
    case eNotify_ShowDiscoveredPlaylists:
    {
        int           index       = 0;
        int *         pIndex      = (int *)notification.getData();
        CPlaylistDb * pPlaylistDb = _pModel->getPlaylistDb();

        if (NULL != pPlaylistDb)
        {
            if (NULL == pIndex)
            {
                index = 0;
            }
            else
            {
                index = *pIndex;
            }
            pPlaylistDb->dump(true, index);
        }
    }
    break;

    case eNotify_ShowPlaylist:
    {
        CPlaylistData * pPlaylistData = (CPlaylistData *)notification.getData();
        CPlaylist *     pPlaylist     = NULL;
        CPlaylistDb *   pPlaylistDb   = _pModel->getPlaylistDb();
        int             i             = 0;

        /* find playlist using IP and print contents */
        for (pPlaylist = pPlaylistDb->getPlaylist(i); pPlaylist; pPlaylist = pPlaylistDb->getPlaylist(++i))
        {
            CChannelBip * pChannelBip;

            pChannelBip = (CChannelBip *)pPlaylist->getChannel(0);
            if ((NULL != pChannelBip) && (pPlaylistData->_strIp == pChannelBip->getHost()))
            {
                pPlaylist->dump(true, pPlaylistData->_nIndex);
            }
        }
    }
    break;

    case eNotify_StreamChannel:
    {
        CPlaylist *   pPlaylist   = NULL;
        CPlaylistDb * pPlaylistDb = _pModel->getPlaylistDb();
        int           i           = 0;

        /* given channel cannot be tuned - must search for match in playlist database */
        CChannelBip * pChannelIp = (CChannelBip *)notification.getData();

        BDBG_WRN(("CControl::eNotify_ChannelStream: %s:%s %d",
                  pChannelIp->getHost().s(), pChannelIp->getUrlPath().s(), pChannelIp->getProgram()));

        for (pPlaylist = pPlaylistDb->getPlaylist(i); pPlaylist; pPlaylist = pPlaylistDb->getPlaylist(++i))
        {
            CChannelBip * pChannelPlaylist;

            pChannelPlaylist = (CChannelBip *)pPlaylist->getChannel(0);
            if ((NULL != pChannelPlaylist) && (pChannelIp->getHost() == pChannelPlaylist->getHost()))
            {
                /* found playlist - now search for channel */
                int j = 0;
                BDBG_WRN(("%s %s Playlist:", pChannelPlaylist->getHost().s(), pPlaylist->getName().s()));

                for (j = 0; j < pPlaylist->numChannels(); j++)
                {
                    pChannelPlaylist = (CChannelBip *)pPlaylist->getChannel(j);
                    if ((NULL != pChannelPlaylist) && (pChannelIp->getUrlPath() == pChannelPlaylist->getUrlPath()))
                    {
                        BDBG_WRN(("Streaming channel: %s %d", pChannelPlaylist->getUrlPath().s(), pChannelPlaylist->getProgram()));
                        ret = tuneChannel(pChannelPlaylist);
                        CHECK_ERROR_GOTO("unable to stream channel", ret, error);
                        break;
                    }
                }
                break;
            }
        }
    }
    break;
#endif /* ifdef PLAYBACK_IP_SUPPORT */

    default:
        break;
    } /* switch */

error:
done:
    return;
} /* processNotification */

/* the given channel is copied and added to the channel list if it passes filters */
eRet CControl::addChannelToChList(CChannel * pChannel)
{
    eRet       ret       = eRet_Ok;
    uint32_t   retFilter = CHMGR_FILTER_PASS;
    CChannel * pChNew    = NULL;

    retFilter = _pChannelMgr->filterChannel(pChannel);
    if (CHMGR_FILTER_PASS == retFilter)
    {
        /* given channel passed filter based on content */

        /*
         * each program will be represented by it's own channel object
         * use copy constructor
         */
        pChNew = pChannel->createCopy(pChannel);
        CHECK_PTR_ERROR_GOTO("unable to create new channel for adding to channel list", pChNew, ret, eRet_OutOfMemory, error);

        _pChannelMgr->addChannel(pChNew);
    }

    ret = (CHMGR_FILTER_PASS == retFilter ? eRet_Ok : eRet_InvalidParameter);

error:
    return(ret);
} /* addChannelToChList */

#if NEXUS_HAS_FRONTEND
/* returns 1 if channel was added to channel list, 0 otherwise */
static bool addChannelCallback(
        CChannel * pNewChannel,
        void *     context
        )
{
    CControl * pControl = (CControl *)context;

    BDBG_ASSERT(NULL != pControl);

    return(eRet_Ok == pControl->addChannelToChList(pNewChannel));
}

#endif /* if NEXUS_HAS_FRONTEND */
/* untune the given channel. */
eRet CControl::unTuneChannel(
        CChannel *  pChannel,
        bool        bFullUnTune,
        eWindowType windowType
        )
{
    eRet ret = eRet_Ok;

#if DVR_LIB_SUPPORT
    CTsb * pTsb = _pModel->getTsb(windowType);
#endif

#ifdef MPOD_SUPPORT
    CCablecard * pCablecard = NULL;
    cablecard_t  cablecard  = cablecard_get_instance();
#endif
#ifdef DCC_SUPPORT
    CClosedCaption * pClosedCaption = _pModel->getClosedCaption();
#endif
    if (eWindowType_Max == windowType)
    {
        windowType = _pModel->getFullScreenWindowType();
    }

    if (NULL == pChannel)
    {
        pChannel = _pModel->getCurrentChannel(windowType);
        CHECK_PTR_ERROR_GOTO("current channel is NULL, ignoring untune request", pChannel, ret, eRet_NotAvailable, error);
    }

    if (false == pChannel->isTuned())
    {
        /* We should return OK because Channels may fail to tune and may untune themselves. Specially Channels
           that are use ASYNC calls. */
        BDBG_WRN((" Already untuned channel"));
        ret = eRet_Ok;
        goto error;
    }
#ifdef MPOD_SUPPORT
    if (cablecard->cablecard_in)
    {
        pCablecard = _pModel->getCableCard();
        pCablecard->cablecard_disable_program(pChannel);
    }
#endif /* ifdef MPOD_SUPPORT */

#if DVR_LIB_SUPPORT
    if (getDvrMgr())
    {
        pTsb->stop();
    }
#endif /* if DVR_LIB_SUPPORT */

#ifdef DCC_SUPPORT
    if (true == GET_BOOL(_pCfg, DCC_ENABLED))
    {
        pClosedCaption->dcc_reset();
    }
#endif /* ifdef DCC_SUPPORT */

    if (_pModel->getCurrentChannel(windowType) == pChannel)
    {
        CSimpleVideoDecode * pVideoDecode = NULL;
        CSimpleAudioDecode * pAudioDecode = NULL;

        if (eWindowType_Pip == windowType)
        {
            /* if pip stop/disconnect pip decoder */
            pVideoDecode = _pModel->getSimpleVideoDecode(windowType);
            pAudioDecode = _pModel->getSimpleAudioDecode(windowType);

            stopDecoders(pVideoDecode, pAudioDecode);
            disconnectDecoders(windowType);

            /* clear current channel */
            _pModel->setCurrentChannel(NULL, windowType);
        }
        else
        {
            /* if non-pip then stop/disconnect all non-pip decoders */
            for (int winType = 0; winType < eWindowType_Max; winType++)
            {
                if (eWindowType_Pip == (eWindowType)winType)
                {
                    continue;
                }

                pVideoDecode = _pModel->getSimpleVideoDecode((eWindowType)winType);
                pAudioDecode = _pModel->getSimpleAudioDecode((eWindowType)winType);

                stopDecoders(pVideoDecode, pAudioDecode);
                disconnectDecoders((eWindowType)winType);

                /* clear current channel */
                _pModel->setCurrentChannel(NULL, (eWindowType)winType);
            }
        }
    }

    if ((false == pChannel->isRecording()) && (false == pChannel->isEncoding()))
    {
        BDBG_MSG(("CControl::unTuneChannel()"));
        pChannel->closePids();

        BDBG_MSG(("Channel is NOT recording. Completely untune this Channel/Tuner"));
        ret = pChannel->unTune(_pConfig, bFullUnTune);
        CHECK_ERROR_GOTO("unable to unTune!", ret, error);

        pChannel->dump(true);
    }
    else
    {
        pChannel->gotoBackGroundRecord();
    }

error:
    return(ret);
} /* unTuneChannel */

/* New implementation of tune channel with emphasis on IP Channel */
eRet CControl::tuneChannel(
        CChannel *  pChannel,
        eWindowType windowType,
        uint16_t    tunerIndex
        )
{
    eRet ret = eRet_Ok;

    CChannel *  pCurrentChannel = _pModel->getCurrentChannel(windowType);
    CChannel *  pFirstChannel   = _pChannelMgr->getFirstChannel();
    CPlayback * pPlayback       = _pModel->getPlayback(windowType);

    BDBG_ASSERT(NULL != _pModel);

    if (eWindowType_Max == windowType)
    {
        windowType = _pModel->getFullScreenWindowType();
    }

    _pModel->setDeferredChannelNum(NULL, NULL, windowType);

    if ((NULL != pPlayback) && (true == pPlayback->isActive()))
    {
        playbackStop(NULL, windowType, false);
    }

    if (NULL == pChannel)
    {
        pChannel = (NULL != pCurrentChannel) ? pCurrentChannel : pFirstChannel;
        CHECK_PTR_ERROR_GOTO("Channel list empty - Unable to tune!", pChannel, ret, eRet_NotAvailable, error);
    }

    /* untune current channel if necessary */
    if ((NULL != pCurrentChannel) && (true == pCurrentChannel->isTuned()))
    {
        bool bForceUntune = ((true == pCurrentChannel->isTunerRequired()) && (false == pChannel->isTunerRequired()));

        BDBG_MSG(("untune channel because actual tuner FORCED:%d", bForceUntune));
        /* only do full untune when transitioning from a channel with an actual tuner, to one
         * without a tuner (i.e. qam channel to streamer channel).  this will ensure that the
         * new channel will not mistakenly decode the previous input band.  we do not fully
         * untune every time because it can slow down channel change. */
        unTuneChannel(pCurrentChannel, bForceUntune, windowType);
    }

    if (false == pChannel->isTuned())
    {
        /* TUNE! */
        pChannel->setModel(_pModel);
        pChannel->setWidgetEngine(_pWidgetEngine);

        BDBG_MSG(("tuning channel:%s windowType:%d", pChannel->getName().s(), windowType));
        pChannel->dump(true);

        _pModel->setChannelTuneInProgress(pChannel, windowType);
        ret = pChannel->tune(_id, _pConfig, false /* async tune */, tunerIndex);
        CHECK_WARN_GOTO("tuning failed", ret, error);
    }
    else
    {
        BDBG_MSG(("channel tune decode immediately!"));
        /* no need to wait for tuner lock so decode now */
        ret = decodeChannel(pChannel, windowType);
        CHECK_ERROR_GOTO("unable to decode channel", ret, error);
    }

error:
    _pModel->setMode(eMode_Live, windowType);
    _pModel->setCurrentChannel(pChannel, windowType);

    return(ret);
} /* tuneChannel */

eRet CControl::decodeChannel(
        CChannel *  pChannel,
        eWindowType windowType
        )
{
    BDBG_ASSERT(NULL != pChannel);
    BDBG_ASSERT(eWindowType_Max != windowType);

    eRet                 ret          = eRet_Ok;
    CStc *               pStc         = _pModel->getStc(windowType);
    CSimpleVideoDecode * pVideoDecode = _pModel->getSimpleVideoDecode(windowType);
    CSimpleAudioDecode * pAudioDecode = _pModel->getSimpleAudioDecode(windowType);
    CPid *               pVideoPid    = NULL;
    CPid *               pAudioPid    = NULL;

#ifdef DVR_LIB_SUPPORT
    CTsb * pTsb = _pModel->getTsb(windowType);
#endif
#ifdef MPOD_SUPPORT
    cablecard_t  cablecard  = cablecard_get_instance();
    CCablecard * pCablecard = NULL;
#endif

    if (0 < GET_INT(_pCfg, MAXDATARATE_PARSERBAND))
    {
        if (NULL != pChannel->getParserBand())
        {
            /* parser band max data rate has been changed in atlas.cfg so print out value so it is obvious
             * which parser band is in use as well as what the max data rate is set to */
            BDBG_WRN(("Parserband #%d Max Data Rate:%d", pChannel->getParserBand()->getNumber(), pChannel->getParserBand()->getMaxDataRate()));
        }
    }

    {
        pChannel->setStc(pStc);
        pVideoPid = pChannel->getPid(0, ePidType_Video);
        pAudioPid = pChannel->getPid(0, ePidType_Audio);

        if ((NULL == pVideoPid) && (NULL == pAudioPid))
        {
            /*
             * coverity[stack_use_local_overflow]
             * coverity[stack_use_overflow]
             */
            CHANNEL_INFO_T chanInfo;
            int            minor = 1;

            /* no pids - let's look for some */

            /* get PSI channel info for current tuned channel */
            ret = pChannel->getChannelInfo(&chanInfo, true);
            CHECK_WARN_GOTO("PSI channel info retrieval failed", ret, error);

            /*pChannel->unTune(_pConfig);*/

            if (0 < chanInfo.num_programs)
            {
                CChannel * pChNew = NULL;
                pChNew = pChannel->createCopy(pChannel);
                CHECK_PTR_ERROR_GOTO("error copying channel object", pChNew, ret, eRet_OutOfMemory, error);

                /* update missing pid info in current channel */
                pChannel->initialize(&chanInfo.program_info[0]);
                pChannel->setMinor(minor);
                minor++;

                /* try to add additional found programs if valid */
                for (int i = 1; i < chanInfo.num_programs; i++)
                {
                    /* since all these channels share a common frequency, we'll assign minor
                     * channel numbers here (minor numbers start at 1) */
                    pChNew->initialize(&chanInfo.program_info[i]);
                    pChNew->setMinor(minor);

                    /* try adding channel to channel list */
                    if (eRet_Ok == addChannelToChList(pChNew))
                    {
                        minor++;
                    }
                }

                /* newly added channels are probably out of order with respect to the
                 * other existing channels, so we'll sort */
                _pChannelMgr->sortChannelList();

                delete pChNew;
            }

            /* Get Pids again */
            pVideoPid = pChannel->getPid(0, ePidType_Video);
            pAudioPid = pChannel->getPid(0, ePidType_Audio);
        }

        ret = pChannel->openPids(pAudioDecode, pVideoDecode);
        CHECK_ERROR_GOTO("Issue Opening Pids for this Channel", ret, error);
    }
#if DVR_LIB_SUPPORT
    if (getDvrMgr())
    {
        pTsb->setBand(pChannel->getParserBand());
        pTsb->setDvrMgr(getDvrMgr());
        pTsb->setSimpleVideoDecode(pVideoDecode);
        pTsb->setSimpleAudioDecode(pAudioDecode);
        pTsb->setStc(pStc);
        pTsb->setWindowType(windowType);
        pTsb->setPidMgr(pChannel->getPidMgr());
        pTsb->start();
    }
    else
    {
        BDBG_WRN(("No storage space for TSB"));
    }
#endif /* if DVR_LIB_SUPPORT */
    /* One Tuning is done now finish setup */
#ifdef MPOD_SUPPORT
    if (cablecard->cablecard_in)
    {
        pCablecard = _pModel->getCableCard();
        pCablecard->cablecard_enable_program(pChannel);
    }
#endif /* ifdef MPOD_SUPPORT */

    ret = connectDecoders(pVideoDecode, pAudioDecode, pChannel->getWidth(), pChannel->getHeight(), pVideoPid, windowType);
    CHECK_ERROR_GOTO("unable to connect decoders", ret, error);

    ret = startDecoders(pVideoDecode, pVideoPid, pAudioDecode, pAudioPid, pStc);
    CHECK_ERROR_GOTO("unable to start decoders", ret, error);

    pChannel->start(pAudioDecode, pVideoDecode);

error:
    /* clear tune in progress indicator */
    _pModel->setChannelTuneInProgress(NULL, windowType);
    return(ret);
} /* decodeChannel */

eRet CControl::tuneDeferredChannel(eWindowType windowType)
{
    eRet        ret       = eRet_Ok;
    CChannel *  pChannel  = NULL;
    CPlayback * pPlayback = _pModel->getPlayback(windowType);
    MString     strChNum;

    if (eWindowType_Pip == windowType)
    {
        _deferredChannelPipTimer.stop();
    }
    else /* eWindowType_Main or eWindowType_Max */
    {
        /* make sure both timer stop in the case that the user is trying both 10key tuning
         * and ch+/- tuning at the same time */
        _deferredChannelUpDownTimer.stop();
        _deferredChannel10KeyTimer.stop();
    }

    strChNum = _pModel->getDeferredChannelNum(windowType);
    BDBG_MSG(("deferred channel num:%s", strChNum.s()));
    if (false == strChNum.isEmpty())
    {
        /* find the channel in channel list that corresponds to deferred channel num string */
        pChannel = _pChannelMgr->findChannel(strChNum.s(), windowType);
        if (NULL == pChannel)
        {
            /* cannot find channel in list - reset deferred channel */
            _pModel->setDeferredChannelNum(NULL, NULL, windowType);
        }
        CHECK_PTR_WARN_GOTO("Unable to find deferred channel in channel list", pChannel, ret, eRet_NotAvailable, error);
    }
    else
    {
        pChannel = _pChannelMgr->getFirstChannel(windowType);
    }

    if (pPlayback && pPlayback->isActive())
    {
        playbackStop(NULL, windowType, false);
    }

    ret = tuneChannel(pChannel, windowType);

error:
    return(ret);
} /* tuneDeferredChannel */

eRet CControl::tuneLastChannel()
{
    eRet       ret          = eRet_Ok;
    CChannel * pChannelLast = _pModel->getLastTunedChannel();

    if (NULL != pChannelLast)
    {
        ret = tuneChannel(pChannelLast);
    }

    return(ret);
}

eRet CControl::encodeStart(
        const char * fileName,
        const char * path
        )
{
#if NEXUS_HAS_VIDEO_ENCODER
    eRet                 ret             = eRet_Ok;
    CSimpleVideoDecode * pVideoDecode    = _pModel->getSimpleVideoDecode();
    CChannel *           pCurrentChannel = _pModel->getCurrentChannel();
    CBoardResources *    pBoardResources = NULL;
    CPlaybackList *      pPlaybackList   = _pModel->getPlaybackList();
    /* Each Channel has its own ParserBand*/
    CParserBand * pEncodeParserBand = NULL;
    CVideo *      video             = NULL;
    CEncode *     pEncode           = NULL;
    MString       indexName;
    MString       strPath = path;

    BDBG_ASSERT(NULL != _pModel);
    BDBG_ASSERT(NULL != _pConfig);
    BDBG_ASSERT(NULL != pPlaybackList);
    BDBG_ASSERT(NULL != pCurrentChannel);

    pBoardResources = _pConfig->getBoardResources();
    /* Add transcode check here*/
    if ((false == pCurrentChannel->isTuned()) && pCurrentChannel->isEncoding())
    {
        BDBG_WRN(("Must be tuned to a channel and Channel must not be trancoding already %s", pCurrentChannel->isEncoding() ? "true" : "false"));
        ret = eRet_NotSupported;
        goto done;
    }

    pEncode = (CEncode *)pBoardResources->checkoutResource(_id, eBoardResource_encode);
    CHECK_PTR_ERROR_GOTO("unable to checkout Encode", pEncode, ret, eRet_NotAvailable, done);
    pEncode->setBoardResources(pBoardResources);
    pEncode->setModel(_pModel);
    ret = pEncode->open();
    CHECK_ERROR_GOTO("unable to open Encode", ret, error);

    /* search for existing recording - we may be overwriting it */
    video = pPlaybackList->find(fileName);
    if (NULL == video)
    {
        video = new CVideo(_pCfg);
        CHECK_PTR_ERROR_GOTO("unable to allocate video object", video, ret, eRet_OutOfMemory, error);
    }

    pEncode->setBand(pCurrentChannel->getParserBand());

    pEncodeParserBand = pEncode->getBand();
    if (pEncodeParserBand == NULL)
    {
        BDBG_ERR(("EncodeParserBand is NULL"));
        ret = eRet_ExternalError;
        goto error;
    }

    ret = pCurrentChannel->dupParserBand(pEncodeParserBand);
    CHECK_ERROR_GOTO("unable to duplicate ParserBand", ret, error);

    /* Check Path */
    if (false == strPath.isEmpty())
    {
        video->setVideosPath(strPath);
    }

    if ((NULL == fileName) || MString(fileName).isEmpty())
    {
        video->setVideoName(pPlaybackList->getNextAtlasName());
    }
    else
    {
        video->setVideoName(fileName);
    }

    /*
     * indexName is the same as fileName
     * Not used currently
     */
    indexName = video->getVideoName();
#if 0
    indexName.truncate(video->getVideoName().find(".", 0, false));
    indexName = indexName+".nav";
#endif
    video->setIndexName(indexName);
    /* generate info name for new file. */
    video->generateInfoName(video->getVideoName().s());

    /* copy channel's pidmgr - the pids we want to use to record may
     * need to copy the program pid info only
     **/
    pEncode->dupPidMgr(pCurrentChannel->getPidMgr());

    pEncode->setVideo(video);
    pEncode->setPlaybackList(pPlaybackList);
    pCurrentChannel->setEncode(pEncode);
    _encodingChannels.add(pCurrentChannel);
    ret = pEncode->start();
    CHECK_ERROR_GOTO("encode start failed!", ret, error);

    BDBG_MSG(("Encoding CurrentChannel"));
    _pModel->addEncode(pEncode);

    /* save decoder source resolution */
    {
        NEXUS_VideoDecoderStatus status;
        ret = pVideoDecode->getStatus(&status);
        if (eRet_Ok == ret)
        {
            video->setWidth(status.source.width);
            video->setHeight(status.source.height);
        }
    }

    if (pPlaybackList->find(fileName) == NULL)
    {
        pPlaybackList->addVideo(video, 0);
        pPlaybackList->createInfo(video);
        pPlaybackList->sync();
    }

    BDBG_MSG(("Added a Video on the playback list"));

done:
    return(ret);

error:

    BDBG_ERR(("Error In Encode"));
    DEL(video);
    _encodingChannels.remove(pCurrentChannel);
    pCurrentChannel->setEncode(NULL);
    pEncode->setBand(NULL);
    pEncode->stop();
    pEncode->close();
    /* check in Encode now */
    if (pEncode != NULL)
    {
        CBoardResources * pBoardResources = _pConfig->getBoardResources();

        ret = pBoardResources->checkinResource(pEncode);
        if (ret != eRet_Ok)
        {
            BDBG_ERR(("unable to checkin Record "));
        }
    }

    return(ret);

#else /* if NEXUS_HAS_VIDEO_ENCODER */
    BSTD_UNUSED(fileName);
    BSTD_UNUSED(path);
    return(eRet_NotSupported);

#endif /* if NEXUS_HAS_VIDEO_ENCODER */
} /* encodeStart */

/* Encode Stop */
eRet CControl::encodeStop(CChannel * pChannel)
{
#if NEXUS_HAS_VIDEO_ENCODER

    eRet       ret             = eRet_Ok;
    CChannel * pCurrentChannel = _pModel->getCurrentChannel();
    CEncode *  pEncode         = NULL;

    if (pChannel != NULL)
    {
        if (false == pChannel->isEncoding())
        {
            BDBG_WRN(("cannot call en code stop on a no recording Channel"));
            ret = eRet_ExternalError;
            goto error;
        }

        pEncode = pChannel->getEncode();
        pCurrentChannel->setEncode(NULL);
        ret = pEncode->stop();
        pEncode->close();
        _encodingChannels.remove(pChannel);
        if (pChannel != pCurrentChannel)
        {
            unTuneChannel(pChannel, true);
        }
        goto done;
    }

    if (pCurrentChannel->isEncoding())
    {
        pEncode = pCurrentChannel->getEncode();
        pCurrentChannel->setEncode(NULL);
        ret = pEncode->stop();
        pEncode->close();
        _encodingChannels.remove(pCurrentChannel);
        CHECK_ERROR_GOTO("Encode error!", ret, error);
    }
    else
    {
        for (pChannel = _encodingChannels.first(); pChannel != NULL; pChannel = _encodingChannels.next())
        {
            if (pChannel->isEncoding())
            {
                pEncode = pChannel->getEncode();
                pCurrentChannel->setEncode(NULL);
                ret = pEncode->stop();
                pEncode->close();
                _encodingChannels.remove(pChannel);
                unTuneChannel(pChannel, true);
                break;
            }
        }
    }

done:
    /* check in Encode now */
    if (pEncode != NULL)
    {
        CBoardResources * pBoardResources = _pConfig->getBoardResources();

        ret = pBoardResources->checkinResource(pEncode);
        CHECK_ERROR_GOTO("unable to checkin Encode ", ret, error);
    }

    _pModel->removeEncode(pEncode);

error:
    return(ret);

#else /* if NEXUS_HAS_VIDEO_ENCODER */
    BSTD_UNUSED(pChannel);
    return(eRet_NotSupported);

#endif /* if NEXUS_HAS_VIDEO_ENCODER */
} /* encodeStop */

eRet CControl::recordStart(CRecordData * pRecordData)
{
    eRet ret = eRet_Ok;

    CChannel *           pCurrentChannel = _pModel->getCurrentChannel();
    CSimpleVideoDecode * pVideoDecode    = _pModel->getSimpleVideoDecode();
    CBoardResources *    pBoardResources = NULL;
    CPid *               pVideoPid       = NULL;
    CPid *               pAudioPid       = NULL;
    CPid *               pPcrPid         = NULL;
    CPlayback *          pPlayback       = NULL;
    CPlaybackList *      pPlaybackList   = _pModel->getPlaybackList();
    CParserBand *        pParserBand     = NULL;
    CVideo *             video           = NULL;
    CRecord *            pRecord         = NULL;
    eWindowType          windowType      = eWindowType_Main;
    MString              indexName;
    MString              strPath;

#if NEXUS_HAS_SECURITY
    NEXUS_SecurityAlgorithm algo = NEXUS_SecurityAlgorithm_eMax;
#endif

    /* check for what is playing on main window first. Then Query to see
     * if there is a playback associated with it */
    windowType = _pModel->getFullScreenWindowType();
    pPlayback  = _pModel->getPlayback(windowType);

    if (pPlayback && pPlayback->isActive())
    {
        BDBG_WRN(("To Record you Must not be in playback mode"));
        ret = eRet_NotSupported;
        goto done;
    }

    BDBG_ASSERT(NULL != _pModel);
    BDBG_ASSERT(NULL != _pConfig);
    BDBG_ASSERT(NULL != pPlaybackList);
    BDBG_ASSERT(NULL != pCurrentChannel);

    if (false == pCurrentChannel->isRecordEnabled())
    {
        BDBG_WRN(("Record disabled for current channel"));
        ret = eRet_NotAvailable;
        goto done;
    }

    pBoardResources = _pConfig->getBoardResources();
    pParserBand     = pCurrentChannel->getParserBand();

    if ((false == pCurrentChannel->isTuned()) || pCurrentChannel->isRecording())
    {
        BDBG_WRN(("Must be tuned to a channel and Channel must not be recording already %s", pCurrentChannel->isRecording() ? "true" : "false"));
        ret = eRet_NotSupported;
        goto done;
    }

    pRecord = (CRecord *)pBoardResources->checkoutResource(_id, eBoardResource_record);
    CHECK_PTR_ERROR_GOTO("unable to checkout Record", pRecord, ret, eRet_NotAvailable, done);

    /* Board Resources needs to be passed down in order to checkout recpumps */
    pRecord->setResources(NULL, pBoardResources);
    ret = pRecord->open(); /* must be done after passing in Board Resources*/
    CHECK_ERROR_GOTO("Unable to open record", ret, error);

    _pModel->addRecord(pRecord);

    if (pRecordData != NULL)
    {
        /* search for existing recording - we may be overwriting it */
        video = pPlaybackList->find(pRecordData->_strFileName.s());

        if (NULL != video)
        {
            pPlaybackList->removeVideo(video);
            video->closeVideo();
            DEL(video);
        }

#if NEXUS_HAS_SECURITY
        if (pRecordData->_security)
        {
            algo = stringToSecurityAlgorithm(pRecordData->_security);
        }
#endif /* if NEXUS_HAS_SECURITY */
    }

    video = new CVideo(_pCfg);
    CHECK_PTR_ERROR_GOTO("unable to allocate video object", video, ret, eRet_OutOfMemory, error);

    pRecord->setBand(pParserBand);

    /* Check Path */
    if ((pRecordData != NULL) && ((false == pRecordData->_strPath.isEmpty())))
    {
        strPath = pRecordData->_strPath;
        video->setVideosPath(strPath);
    }
    BDBG_MSG(("Video Path = %s", video->getVideosPath().s()));

    /* Check Video Name */
    if ((pRecordData == NULL) || ((NULL == pRecordData->_strFileName) || pRecordData->_strFileName.isEmpty()))
    {
        video->setVideoName(pPlaybackList->getNextAtlasName());
    }
    else
    {
        BDBG_MSG(("Setting Record File Name to %s", pRecordData->_strFileName.s()));
        video->setVideoName(pRecordData->_strFileName);
    }

    /* generate info name */
    video->generateInfoName(video->getVideoName().s());

    /* Record needs to manage its own pids and close once its done using them */
    pRecord->dupPidMgr(pCurrentChannel->getPidMgr());

    pVideoPid = pRecord->getPid(0, ePidType_Video);
    if (pVideoPid)
    {
#if NEXUS_HAS_SECURITY
        if (algo < NEXUS_SecurityAlgorithm_eMax)
        {
            pVideoPid->encrypt(algo, NULL, true);
        }
#endif /* if NEXUS_HAS_SECURITY */
        /* indexName is the same as fileName */
        indexName = video->getVideoName();
        indexName.truncate(video->getVideoName().find(".", 0, false));
        indexName = indexName+".nav";
        video->setIndexName(indexName);
    }
    else
    {
        video->setIndexName(video->getVideoName());
    }

    pAudioPid = pRecord->getPid(0, ePidType_Audio);
    if (pAudioPid)
    {
#if NEXUS_HAS_SECURITY
        if (algo < NEXUS_SecurityAlgorithm_eMax)
        {
            pAudioPid->encrypt(algo, NULL, true);
        }
#endif /* if NEXUS_HAS_SECURITY */
    }

    pPcrPid = pRecord->getPid(0, ePidType_Pcr);

    /* Save Video here, since it has useful information for Record */
    pRecord->setVideo(video);

    /*
     * Move this to Channel Class
     * open pids
     */
    if (pVideoPid)
    {
        ret = pVideoPid->open(pRecord);
        CHECK_ERROR_GOTO("open video pid channel failed", ret, error);
    }

    if (pAudioPid)
    {
        ret = pAudioPid->open(pRecord);
        CHECK_ERROR_GOTO("open audio pid channel failed", ret, error);
    }

    if (pPcrPid && pPcrPid->isUniquePcrPid())
    {
        ret = pPcrPid->open(pRecord);
        CHECK_ERROR_GOTO("open pcr pid channel failed", ret, error);
    }

    pCurrentChannel->setRecord(pRecord);
    _recordingChannels.add(pCurrentChannel);

    ret = pRecord->start();
    CHECK_ERROR_GOTO("unable to start recording.", ret, error);

    /*
     * Add to the top of the Playback List. Sort will be done when Atlas  Shuts down
     * copy records pidmgr which only copies info not the pidChannel Handle
     * video object is just an information container.
     */
    video->dupPidMgr(pRecord->getPidMgr());

    /* save decoder source resolution */
    {
        NEXUS_VideoDecoderStatus status;
        ret = pVideoDecode->getStatus(&status);
        if (eRet_Ok == ret)
        {
            video->setWidth(status.source.width);
            video->setHeight(status.source.height);
        }
    }

    pPlaybackList->addVideo(video, 0);
    pPlaybackList->createInfo(video);
    pPlaybackList->sync();
    BDBG_MSG(("Added a Record and a video to the playlist"));

done:
    return(ret);

error:
    if (pVideoPid)
    {
        pVideoPid->close(pRecord);
    }

    if (pAudioPid)
    {
        pAudioPid->close(pRecord);
    }

    if (pPcrPid && pPcrPid->isUniquePcrPid())
    {
        pPcrPid->close(pRecord);
    }

    DEL(video);

    _recordingChannels.remove(pCurrentChannel);
    pCurrentChannel->setRecord(NULL);
    pRecord->setBand(NULL);

    /* check in Record now */
    if (pRecord != NULL)
    {
        CBoardResources * pBoardResources = _pConfig->getBoardResources();
        pRecord->stop();
        pRecord->close();
        ret = pBoardResources->checkinResource(pRecord);
        CHECK_ERROR_GOTO("unable to checkin Record ", ret, error);
    }

    BDBG_ERR((" failed to start Record!"));
    return(ret);
} /* recordStart */

/* Right now we don't use fileName.
 * Multiple Records will need fileName */
eRet CControl::recordStop(CChannel * pChannel)
{
    eRet       ret             = eRet_Ok;
    CChannel * pCurrentChannel = _pModel->getCurrentChannel();
    CRecord *  pRecord         = NULL;

    /*_pRecord->printPids(); */

    if (pChannel != NULL)
    {
        if (false == pChannel->isRecording())
        {
            BDBG_WRN(("cannot call record stop on a no recording Channel"));
            ret = eRet_ExternalError;
            goto error;
        }

        pRecord = pChannel->getRecord();
        pChannel->setRecord(NULL);
        pRecord->stop();
        pRecord->closePids();
        _recordingChannels.remove(pChannel);
        if (pChannel != pCurrentChannel)
        {
            unTuneChannel(pChannel, true);
        }
        goto done;
    }

    if ((pCurrentChannel != NULL) && pCurrentChannel->isRecording())
    {
        pRecord = pCurrentChannel->getRecord();
        pCurrentChannel->setRecord(NULL);
        pRecord->stop();
        pRecord->closePids();
        _recordingChannels.remove(pCurrentChannel);
        goto done;
    }
    else
    {
        MListItr<CChannel> itr(&_recordingChannels);

        for (pChannel = itr.first(); pChannel; pChannel = itr.next())
        {
            if (pChannel->isRecording())
            {
                pRecord = pChannel->getRecord();
                pChannel->setRecord(NULL);
                pRecord->stop();
                pRecord->closePids();
                _recordingChannels.remove(pChannel);
                unTuneChannel(pChannel, true);
                break;
            }
        }
    }

done:
    /* check in Record now */
    if (pRecord != NULL)
    {
        CBoardResources * pBoardResources = _pConfig->getBoardResources();

        /* record needs to be closed*/
        pRecord->close();
        ret = pBoardResources->checkinResource(pRecord);
        CHECK_ERROR_GOTO("unable to checkin Record ", ret, error);
    }

    _pModel->removeRecord(pRecord);

error:
    return(ret);
} /* recordStop */

eRet CControl::playbackStart(
        const char * fileName,
        const char * indexName,
        const char * path,
        eWindowType  windowType
        )
{
    CVideo *        pVideo        = NULL;
    CPlayback *     pPlayback     = _pModel->getPlayback(windowType);
    CPlaybackList * pPlaybackList = _pModel->getPlaybackList();
    eRet            ret           = eRet_Ok;

    BDBG_ASSERT(NULL != _pModel);
    BDBG_ASSERT(NULL != _pConfig);
    BSTD_UNUSED(indexName);
    BSTD_UNUSED(path);

    if (eWindowType_Max == windowType)
    {
        windowType = _pModel->getFullScreenWindowType();
    }

    /* Check the media Filename First */
    if (pPlaybackList != NULL)
    {
        pVideo = pPlaybackList->find(fileName);

        /* Check it one last time if its still NULL*/
        if (pVideo == NULL)
        {
            BDBG_WRN((" This file is not in playbackList, Try to add new file"));
            updatePlaybackList(); /* use default Path for now for right now */
            pPlaybackList->sync();
            pVideo = pPlaybackList->find(fileName);
        }

        /* Check for empty Filename this means resume playback */
        if ((NULL == fileName) && (pPlayback->isActive()))
        {
            /* resume playback */
            ret = pPlayback->play();
            goto done;
        }

        if (!pVideo)
        {
            BDBG_WRN((" Filename does not exist!"));
            goto done;
        }

        ret = playbackStart(pVideo, windowType);
    }

done:
    return(ret);
} /* playbackStart */

eRet CControl::playbackStart(
        CVideo *    pVideo,
        eWindowType windowType
        )
{
    CChannel *           pCurrentChannel = _pModel->getCurrentChannel(windowType);
    CStc *               pStc            = _pModel->getStc(windowType);
    CSimpleVideoDecode * pVideoDecode    = _pModel->getSimpleVideoDecode(windowType);
    CSimpleAudioDecode * pAudioDecode    = _pModel->getSimpleAudioDecode(windowType);
    CPlayback *          pPlayback       = _pModel->getPlayback(windowType);
    CPid *               pVideoPid       = NULL;
    CPid *               pAudioPid       = NULL;
    CPid *               pPcrPid         = NULL;
    eRet                 ret             = eRet_Ok;

    BDBG_ASSERT(NULL != _pModel);
    BDBG_ASSERT(NULL != _pConfig);

    if (eWindowType_Max == windowType)
    {
        windowType = _pModel->getFullScreenWindowType();
    }

    if (!pPlayback)
    {
        CBoardResources * pBoardResources = _pConfig->getBoardResources();
        pPlayback = (CPlayback *)pBoardResources->checkoutResource(_id, eBoardResource_playback);
        CHECK_PTR_ERROR_GOTO("unable to checkout Playback ", pPlayback, ret, eRet_NotAvailable, done);
        /* need to pass in the resources*/
        pPlayback->setResources(NULL, pBoardResources);
        ret = pPlayback->open(getWidgetEngine());
        CHECK_ERROR_GOTO("Cannot open pPlayback", ret, error);

        _pModel->setPlayback(pPlayback, windowType);
    }
    else
    {
        /* Change to new playback */
        if (pPlayback->getVideo() != pVideo)
        {
            BDBG_MSG((" Switch Playback %s to %s ", __FUNCTION__, pVideo->getVideoName().s()));

            stopDecoders(pVideoDecode, pAudioDecode);
            disconnectDecoders(windowType);

            /* pPlayback->stop(); */
            pPlayback->close(); /* stops & closes Playback */
            ret = pPlayback->open(getWidgetEngine());
            CHECK_ERROR_GOTO("Cannot open pPlayback", ret, error);
        }
        else
        {
            return(pPlayback->play());
        }
    }

    if (0 < GET_INT(_pCfg, MAXDATARATE_PLAYBACK))
    {
        /* playback max data rate has been changed in atlas.cfg so print out value so it is obvious
         * which playback is in use as well as what the max data rate is set to */
        BDBG_WRN(("Playback #%d Max Data Rate:%d", pPlayback->getNumber(), pPlayback->getMaxDataRate()));
    }

#if 0
    /* Check Path */
    if (NULL == path)
    {
        CConfiguration * pCfg    = _pConfig->getCfg();
        MString          newPath = GET_STR(pCfg, VIDEOS_PATH);
        newPath += "/";
        /* check video path now */
        if (pVideo && pVideo->_path.isNull())
        {
            pVideo->_path = newPath;
        }
        else
        if (pVideo && (pVideo->_path.strncmp(newPath) != 0))
        {
            /* overwrite */
            pVideo->_path = newPath;
        }
    }
    else
    {
        BDBG_WRN(("TODO: support path function param"));
    }
#endif /* if 0 */

    if ((NULL != pCurrentChannel) && (true == pCurrentChannel->isTuned()))
    {
        BDBG_WRN((" Current Channel is Tuned, This channel will now be UnTuned"));
        /* Untune the Channel */
        unTuneChannel(pCurrentChannel, true, windowType);
    }

    /* pPlayback->dump(); */
    pPlayback->setVideo(pVideo);
    /* copy videos pidmgr - the pids we want to use to record may
     * already be opened and ready for use. */
    pPlayback->dupPidMgr(pVideo->getPidMgr());

    /* Populate pids */
    pVideoPid = pPlayback->getPid(0, ePidType_Video);
    if (pVideoPid && pVideoPid->isDVREncryption())
    {
        BDBG_MSG((" Decrypting  Video PID"));
        pVideoPid->encrypt();
    }
    pAudioPid = pPlayback->getPid(0, ePidType_Audio);
    if (pAudioPid && pAudioPid->isDVREncryption())
    {
        pAudioPid->encrypt();
    }
    pPcrPid = pPlayback->getPid(0, ePidType_Pcr);

    if (pPcrPid == NULL)
    {
        /* This is normal for Playback */
        BDBG_MSG(("NULL PCRPID"));
    }
    else
    if (pPcrPid->isUniquePcrPid() && pPcrPid->isDVREncryption())
    {
        pPcrPid->encrypt();
    }

    CHECK_PTR_ERROR_GOTO("NULL STC", pStc, ret, eRet_InvalidState, error);

    /* Configure STC first
     * Add this to playback
     *
     */
    pPlayback->setStc(pStc);

    /* open pids */
    if ((NULL != pVideoPid) && (false == pVideoPid->isOpen()))
    {
        CHECK_PTR_ERROR_GOTO("missing playback - playback failed", pPlayback, ret, eRet_InvalidState, error);
        pVideoPid->setVideoDecoder(pVideoDecode);
        ret = pVideoPid->open(pPlayback);
        CHECK_ERROR_GOTO("open playback video pid channel failed", ret, error);
    }

    if ((NULL != pAudioPid) && (false == pAudioPid->isOpen()))
    {
        CHECK_PTR_ERROR_GOTO("missing playback - playback failed", pPlayback, ret, eRet_InvalidState, error);
        pAudioPid->setAudioDecoder(pAudioDecode);
        ret = pAudioPid->open(pPlayback);
        CHECK_ERROR_GOTO("open playback video pid channel failed", ret, error);
    }

    if ((NULL != pPcrPid) && (false == pPcrPid->isOpen()))
    {
        /* only open pcr pid channel if it is different from audio/video pids */
        if (false == pPcrPid->isOpen())
        {
            CHECK_PTR_ERROR_GOTO("missing playback - playback failed", pPlayback, ret, eRet_InvalidState, error);
            ret = pPcrPid->open(pPlayback);
            CHECK_ERROR_GOTO("open playback video pid channel failed", ret, error);
        }
    }

    ret = connectDecoders(pVideoDecode, pAudioDecode, pVideo->getWidth(), pVideo->getHeight(), pVideoPid, windowType);
    CHECK_ERROR_GOTO("unable to connect decoders", ret, error);

    ret = startDecoders(pVideoDecode, pVideoPid, pAudioDecode, pAudioPid, pStc);
    CHECK_ERROR_GOTO("unable to start decoders", ret, error);

    ret = pPlayback->start();
    CHECK_ERROR_GOTO("unable to start playback", ret, error);

    _pModel->setMode(eMode_Playback, windowType);
    _pModel->setPlayback(pPlayback, windowType);

done:
    return(ret);

error:
    stopDecoders(pVideoDecode, pAudioDecode);
    disconnectDecoders(windowType);

    if (pVideoPid)
    {
        /* Check to see if PCR pid is the same as Video Pid */
        if (pPcrPid == pVideoPid)
        {
            pPcrPid = NULL;
        }
        pVideoPid->close(pPlayback);
    }
    if (pAudioPid)
    {
        pAudioPid->close(pPlayback);
    }
    if (pPcrPid && pPcrPid->isUniquePcrPid())
    {
        pPcrPid->close(pPlayback);
    }

    if (pPlayback)
    {
        pPlayback->close(); /* stops & closes Playback */
        /* check in playback! */
        CBoardResources * pBoardResources = _pConfig->getBoardResources();
        ret = pBoardResources->checkinResource(pPlayback);

        _pModel->setPlayback(NULL, windowType);
    }
    /* Re Tune the channel */
    if (NULL != pCurrentChannel)
    {
        BDBG_WRN((" Playback Failed return to Previous Channel "));
        /* tune the Channel */
        tuneChannel(pCurrentChannel, windowType);
    }

    return(ret);
} /* playbackStart */

/* Right now we don't use mediaName.
 * Multiple Playbacks will need MediaName */
eRet CControl::playbackStop(
        const char * mediaName,
        eWindowType  windowType,
        bool         bTuneLast
        )
{
    eRet                 ret          = eRet_Ok;
    CSimpleVideoDecode * pVideoDecode = _pModel->getSimpleVideoDecode(windowType);
    CSimpleAudioDecode * pAudioDecode = _pModel->getSimpleAudioDecode(windowType);
    CPlayback *          pPlayback    = _pModel->getPlayback(windowType);

    BDBG_ASSERT(NULL != _pModel);
    BSTD_UNUSED(mediaName);

    if (eWindowType_Max == windowType)
    {
        windowType = _pModel->getFullScreenWindowType();
    }

    stopDecoders(pVideoDecode, pAudioDecode);
    disconnectDecoders(windowType);

    /*
     * pPlayback->dump();
     * pPlayback->printPids();
     */

    ret = pPlayback->close(); /* stops then closes */
    CHECK_ERROR_GOTO("playback error!", ret, error);

    /* check in playback! */
    {
        CBoardResources * pBoardResources = _pConfig->getBoardResources();

        ret = pBoardResources->checkinResource(pPlayback);
        CHECK_ERROR_GOTO("unable to checkin Playback ", ret, error);
    }

    _pModel->setPlayback(NULL, windowType);

    if (true == bTuneLast)
    {
        if (_pModel->getFullScreenWindowType() == windowType)
        {
            /* Tune to the last Channel if we are in the full screen window */
            tuneChannel(_pModel->getLastTunedChannel(windowType));
        }
    }

error:
    return(ret);
} /* playbackStop */

#if NEXUS_HAS_FRONTEND
eRet CControl::scanTuner(CTunerScanData * pScanData)
{
    eRet              ret             = eRet_Ok;
    CBoardResources * pBoardResources = NULL;
    CTuner *          pTuner          = NULL;
    CChannel *        pCurrentChannel = _pModel->getCurrentChannel();
    CChannel *        pChannel        = NULL;

    MList <CChannel> foundChannelList;

    BDBG_ASSERT(NULL != pScanData);
    pBoardResources = _pConfig->getBoardResources();

    showPip(false);

    {
        CPlayback * pPlayback = _pModel->getPlayback();
        if (NULL != pPlayback)
        {
            playbackStop();
        }
    }

    if (true == pScanData->_appendToChannelList)
    {
        CChannel * pChannelLast = _pChannelMgr->getLastChannel();

        /* untune only channels matching tuner type if appending to current channel list */
        while (NULL != (pChannel = _pChannelMgr->findTunedChannel(pScanData->getTunerType())))
        {
            /* untune tuners */
            if (pChannel->isRecording())
            {
                recordStop(pChannel);
                if (pChannel == pCurrentChannel)
                {
                    unTuneChannel(pChannel);
                    pCurrentChannel = NULL;
                }
            }
            else
            {
                unTuneChannel(pChannel);
            }
        }

        /* set the starting major channel number for the newly found channnels */
        if (NULL != pChannelLast)
        {
            pScanData->_majorStartNum = pChannelLast->getMajor() + 1;
        }
    }
    else
    {
        /* untune all channels if replacing current channel list */
        while (NULL != (pChannel = _pChannelMgr->findTunedChannel()))
        {
            /* untune tuners */
            if (pChannel->isRecording())
            {
                recordStop(pChannel);
                if (pChannel == pCurrentChannel)
                {
                    unTuneChannel(pChannel);
                    pCurrentChannel = NULL;
                }
            }
            else
            {
                unTuneChannel(pChannel);
            }
        }
    }

    /* check out tuner to use for scan */
    pTuner = (CTuner *)pBoardResources->checkoutResource(_id, pScanData->getTunerType());
    CHECK_PTR_ERROR_GOTO("No tuner available - scan failed!", pTuner, ret, eRet_NotAvailable, errorNoTuner);

    BDBG_ASSERT(true == pTuner->isFrontend());
    pTuner->setConfig(_pConfig);

    if (false == pScanData->_appendToChannelList)
    {
        /* we are replacing current channel list so clear it */
        _pChannelMgr->clearChannelList();
    }

    /* scan for channels - note that scan() will dynamically allocate found channel objects and
     * return them to the specified callback routine.  this is an asynchronous call so it will
     * return immediately. */
    ret = pTuner->open(getWidgetEngine());
    CHECK_ERROR_GOTO("unable to open tuner", ret, error);

    ret = pTuner->scan(this, pScanData, addChannelCallback, (void *)this);
    CHECK_ERROR_GOTO("unable to scan with tuner", ret, error);

    _pModel->setMode(eMode_Scan);
    goto done;

error:
    if (NULL != pTuner)
    {
        pTuner->close();
        pBoardResources->checkinResource(pTuner);
    }
errorNoTuner:
    tuneChannel();

done:
    return(ret);
} /* scanTuner */

#endif /* NEXUS_HAS_FRONTEND */

#if RF4CE_SUPPORT
eRet CControl::addRf4ceRemote(const char * remote_name)
{
    CRemote::addRf4ceRemote(remote_name);

    return(eRet_Ok);
}

eRet CControl::removeRf4ceRemote(int pairingRefNum)
{
    CRemote::removeRf4ceRemote(pairingRefNum);

    return(eRet_Ok);
}

eRet CControl::displayRf4ceRemotes()
{
    CRemote::displayRf4ceRemotes();

    return(eRet_Ok);
}

#endif /* if RF4CE_SUPPORT */

eRet CControl::channelUp()
{
    eRet       ret          = eRet_Ok;
    CChannel * pChannel     = NULL;
    CChannel * pChannelNext = NULL;
    MString    strChNum;

    /* attempt to find deferred channel number if it exists */
    strChNum = _pModel->getDeferredChannelNum();
    if (false == strChNum.isEmpty())
    {
        pChannel = _pChannelMgr->findChannel(strChNum.s());
    }

    if (NULL == pChannel)
    {
        /* no deferred channel so use current channel */
        pChannel = _pModel->getCurrentChannel();
        if (NULL == pChannel)
        {
            /* no current channel so use last tuned channel */
            pChannel = _pModel->getLastTunedChannel();
        }
    }

    pChannelNext = _pChannelMgr->getNextChannel(pChannel);
    if (NULL == pChannelNext)
    {
        BDBG_WRN(("NO next channel so try retrieving first channel in list"));
        /* no next channel (current channel may not be a member of the channel list)
         * so we'll try and get the first channel in list. */
        pChannelNext = _pChannelMgr->getFirstChannel();
    }
    CHECK_PTR_ERROR_GOTO("Unable to tune - channel list is empty", pChannelNext, ret, eRet_NotAvailable, error);

    _pModel->setDeferredChannelNum(pChannelNext->getChannelNum(), pChannelNext);
    _deferredChannelUpDownTimer.start();

error:
    return(ret);
} /* channelUp */

eRet CControl::channelDown()
{
    eRet       ret          = eRet_Ok;
    CChannel * pChannel     = NULL;
    CChannel * pChannelPrev = NULL;
    MString    strChNum;

    /* attempt to find deferred channel number if it exists */
    strChNum = _pModel->getDeferredChannelNum();
    if (false == strChNum.isEmpty())
    {
        pChannel = _pChannelMgr->findChannel(strChNum.s());
    }

    if (NULL == pChannel)
    {
        /* no deferred channel so use current channel */
        pChannel = _pModel->getCurrentChannel();
        if (NULL == pChannel)
        {
            /* no current channel so use last tuned channel */
            pChannel = _pModel->getLastTunedChannel();
        }
    }

    pChannelPrev = _pChannelMgr->getPrevChannel(pChannel);
    if (NULL == pChannelPrev)
    {
        BDBG_WRN(("NO prev channel so try retrieving first channel in list"));
        /* no prev channel (current channel may not be a member of the channel list)
         * so we'll try and get the first channel in list. */
        pChannelPrev = _pChannelMgr->getFirstChannel();
    }
    CHECK_PTR_ERROR_GOTO("Unable to tune - channel list is empty", pChannelPrev, ret, eRet_NotAvailable, error);

    _pModel->setDeferredChannelNum(pChannelPrev->getChannelNum(), pChannelPrev);
    _deferredChannelUpDownTimer.start();

error:
    return(ret);
} /* channelDown */

eRet CControl::tenKey(eKey key)
{
    eRet    ret      = eRet_Ok;
    int     dotIndex = 0;
    MString strChNum;
    MString strChNumNew;

    BDBG_ASSERT((eKey_0 == key) || (eKey_1 == key) || (eKey_2 == key) || (eKey_3 == key) ||
            (eKey_4 == key) || (eKey_5 == key) || (eKey_6 == key) || (eKey_7 == key) ||
            (eKey_8 == key) || (eKey_9 == key) || (eKey_Dot == key));

    /* 10 key already disabled during playback so no need to check for active playback here */

    /* attempt to find deferred channel number if it exists */
    strChNum = _pModel->getDeferredChannelNum();

    /* find dot index if exists */
    dotIndex = strChNum.find('.');

    if (eKey_Dot == key)
    {
        if (0 < dotIndex)
        {
            /* dot exists - and we received another dot so clear minor channel number */
            strChNum = strChNum.left(dotIndex);
        }

        strChNum += MString(".");
    }
    else
    {
        /* concatenate new digit */
        strChNum += MString(key - eKey_0);
    }

    _pModel->setDeferredChannelNum(strChNum.s());
    _deferredChannel10KeyTimer.start();

    return(ret);
} /* tenKey */

int32_t CControl::getVolume(void)
{
    MList<COutput> * pAudioOutputList = _pModel->getAudioOutputList();
    COutput *        pAudioOutput     = NULL;
    int32_t          volume           = 0;
    eRet             ret              = eRet_Ok;

    CHECK_PTR_ERROR_GOTO("unable to get audio output list", pAudioOutputList, ret, eRet_InvalidState, error);
    {
        MListItr <COutput> itr(pAudioOutputList);
        for (pAudioOutput = itr.first(); pAudioOutput; pAudioOutput = itr.next())
        {
            CAudioVolume vol;

            vol = pAudioOutput->getVolume();

            volume = MAX(volume, vol._left);
            volume = MAX(volume, vol._right);
        }
    }

error:
    return(volume);
} /* getVolume */

eRet CControl::setVolume(int32_t level)
{
    MList<COutput> * pAudioOutputList = _pModel->getAudioOutputList();
    COutput *        pAudioOutput     = NULL;
    eRet             ret              = eRet_Ok;

    CHECK_PTR_ERROR_GOTO("unable to get audio output list", pAudioOutputList, ret, eRet_InvalidState, error);
    {
        MListItr <COutput> itr(pAudioOutputList);
        for (pAudioOutput = itr.first(); pAudioOutput; pAudioOutput = itr.next())
        {
            CAudioVolume vol;

            BDBG_MSG(("setVolume for output:%s%d level:%x", pAudioOutput->getName(), pAudioOutput->getNumber(), level));
            vol        = pAudioOutput->getVolume();
            vol._left  = level;
            vol._right = level;
            pAudioOutput->setVolume(vol);
        }
    }

error:
    return(ret);
} /* setVolume */

bool CControl::getMute(void)
{
    MList<COutput> * pAudioOutputList = _pModel->getAudioOutputList();
    COutput *        pAudioOutput     = NULL;
    bool             bMuted           = false;
    eRet             ret              = eRet_Ok;

    CHECK_PTR_ERROR_GOTO("unable to get audio output list", pAudioOutputList, ret, eRet_InvalidState, error);
    {
        MListItr <COutput> itr(pAudioOutputList);
        for (pAudioOutput = itr.first(); pAudioOutput; pAudioOutput = itr.next())
        {
            CAudioVolume vol;

            vol     = pAudioOutput->getVolume();
            bMuted |= vol._muted;
        }
    }

error:
    return(bMuted);
} /* getMute */

eRet CControl::setMute(bool muted)
{
    MList<COutput> * pAudioOutputList = _pModel->getAudioOutputList();
    COutput *        pAudioOutput     = NULL;
    eRet             ret              = eRet_Ok;

    CHECK_PTR_ERROR_GOTO("unable to get audio output list", pAudioOutputList, ret, eRet_InvalidState, error);
    {
        MListItr <COutput> itr(pAudioOutputList);
        for (pAudioOutput = itr.first(); pAudioOutput; pAudioOutput = itr.next())
        {
            BDBG_MSG(("setMute for output:%s%d mute:%d", pAudioOutput->getName(), pAudioOutput->getNumber(), muted));
            pAudioOutput->setMute(muted);
        }
    }

error:
    return(ret);
} /* setMute */

void CControl::dumpPlaybackList(bool bForce)
{
    CPlaybackList * pPlaybackList = _pModel->getPlaybackList();

    if (NULL != pPlaybackList)
    {
        pPlaybackList->dump(bForce);
    }
    else
    {
        BDBG_MSG(("Playback list is EMPTY"));
    }
} /* dumpPlaybackList */

void CControl::updatePlaybackList()
{
    CPlaybackList * pPlaybackList = _pModel->getPlaybackList();

#ifdef PLAYBACK_IP_SUPPORT
    CServerMgr * pServerMgr         = _pModel->getServerMgr();
    bool         bHttpServerStarted = false;
#endif /* ifdef PLAYBACK_IP_SUPPORT */

    ATLAS_MEMLEAK_TRACE("BEGIN");

#ifdef PLAYBACK_IP_SUPPORT
    if (NULL != pServerMgr)
    {
        bHttpServerStarted = pServerMgr->isHttpServerStarted();

        if (true == bHttpServerStarted)
        {
            /* note that clients may still continue to play after stop since they will buffer
             * content and retry to get new content automatically.  if refresh from
             * disk takes longer than the client's retry timeout, client ip playback
             * will fail.  given that the retry timeout is up to 5000msecs, in most
             * cases client playback will continue uninterrupted
             */
            pServerMgr->stopHttpServer();
        }
    }
#endif /* ifdef PLAYBACK_IP_SUPPORT */

    showPip(false);
    stopAllPlaybacks();
    stopAllRecordings();
    stopAllEncodings();

    BDBG_ASSERT(NULL != pPlaybackList);
    pPlaybackList->refreshFromDisk();

#ifdef PLAYBACK_IP_SUPPORT
    if ((NULL != pServerMgr) &&
        (true == bHttpServerStarted) &&
        (false == pServerMgr->isHttpServerStarted()))
    {
        /* restart http server if started when this function
         * was called but not currently running */
        pServerMgr->startHttpServer();
    }
#endif /* ifdef PLAYBACK_IP_SUPPORT */

    ATLAS_MEMLEAK_TRACE("END");
} /* updatePlaybackList */

#if NEXUS_HAS_FRONTEND
/* call initialize in one of each tuner type. this is primarily used for tuners to send out
 * overall initialization info at atlas startup. */
void CControl::initializeTuners()
{
    CBoardResources * pBoardResources = NULL;
    CTuner *          pTuner          = NULL;

    ATLAS_MEMLEAK_TRACE("BEGIN");

    /* checkout tuners and initialize state */
    pBoardResources = _pConfig->getBoardResources();
    BDBG_ASSERT(NULL != pBoardResources);

    if (true == pBoardResources->findResource(_id, eBoardResource_frontendQam, 0))
    {
        pTuner = (CTuner *)pBoardResources->checkoutResource(_id, eBoardResource_frontendQam, 0);
        pTuner->initCapabilities();
        pBoardResources->checkinResource(pTuner);
        pTuner = NULL;
    }

    if (true == pBoardResources->findResource(_id, eBoardResource_frontendVsb, 0))
    {
        pTuner = (CTuner *)pBoardResources->checkoutResource(_id, eBoardResource_frontendVsb, 0);
        pTuner->initCapabilities();
        pBoardResources->checkinResource(pTuner);
        pTuner = NULL;
    }

    if (true == pBoardResources->findResource(_id, eBoardResource_frontendSds, 0))
    {
        pTuner = (CTuner *)pBoardResources->checkoutResource(_id, eBoardResource_frontendSds, 0);
        pTuner->initCapabilities();
        pBoardResources->checkinResource(pTuner);
        pTuner = NULL;
    }

    if (true == pBoardResources->findResource(_id, eBoardResource_frontendIp, 0))
    {
        pTuner = (CTuner *)pBoardResources->checkoutResource(_id, eBoardResource_frontendIp, 0);
        pTuner->initCapabilities();
        pBoardResources->checkinResource(pTuner);
        pTuner = NULL;
    }

    if (true == pBoardResources->findResource(_id, eBoardResource_frontendOfdm, 0))
    {
        pTuner = (CTuner *)pBoardResources->checkoutResource(_id, eBoardResource_frontendOfdm, 0);
        pTuner->initCapabilities();
        pBoardResources->checkinResource(pTuner);
        pTuner = NULL;
    }

    if (true == pBoardResources->findResource(_id, eBoardResource_frontendOob, 0))
    {
        pTuner = (CTuner *)pBoardResources->checkoutResource(_id, eBoardResource_frontendOob, 0);
        pTuner->initCapabilities();
        pBoardResources->checkinResource(pTuner);
        pTuner = NULL;
    }

    if (true == pBoardResources->findResource(_id, eBoardResource_frontendUpstream, 0))
    {
        pTuner = (CTuner *)pBoardResources->checkoutResource(_id, eBoardResource_frontendUpstream, 0);
        pTuner->initCapabilities();
        pBoardResources->checkinResource(pTuner);
        pTuner = NULL;
    }

    ATLAS_MEMLEAK_TRACE("END");
} /* initializeTuners */

#endif /* if NEXUS_HAS_FRONTEND */

/* these audio decode macros pause playback and stop decode so audio inputs changes can be
 * made and then return playback/decode to original state */
#define AUDIO_DECODE_CHANGE_INIT(decode)          \
    bool restartPlayback = false;                 \
    bool                   restartDecode = false; \
    CPid *                 pPid          = NULL;  \
    NEXUS_PlaybackPosition position      = 0;     \
    CStc *                 pStc          = (decode)->getStc();

#define AUDIO_DECODE_CHANGE_START(decode, playback)                                                 \
    do                                                                                              \
    {                                                                                               \
        if ((NULL != (playback)) && ((playback)->isActive()))                                       \
        {                                                                                           \
            restartPlayback = (!(playback)->isPaused());                                            \
            ret             = (playback)->pause(false, &position);                                  \
            CHECK_ERROR_GOTO("unable to pause playback", ret, error);                               \
        }                                                                                           \
                                                                                                    \
        if ((NULL != (decode)) && (true == (decode)->isStarted()))                                  \
        {                                                                                           \
            pPid = (decode)->stop();                                                                \
            CHECK_PTR_ERROR_GOTO("unable to stop decode", pPid, ret, eRet_InvalidParameter, error); \
            restartDecode = true;                                                                   \
        }                                                                                           \
    } while (0);

#define AUDIO_DECODE_CHANGE_FINISH(decode, playback)                        \
    do                                                                      \
    {                                                                       \
        if ((NULL != (decode)) && (true == restartDecode))                  \
        {                                                                   \
            ret = (decode)->start(pPid, pStc);                              \
            CHECK_ERROR_GOTO("unable to restart decoder", ret, error);      \
        }                                                                   \
                                                                            \
        if ((NULL != (playback)) && ((playback)->isActive()))               \
        {                                                                   \
            if (restartPlayback) {                                          \
                ret = (playback)->start(position);                          \
                CHECK_ERROR_GOTO("unable to restart playback", ret, error); \
            }                                                               \
            else {                                                          \
                (playback)->seek(position);                                 \
            }                                                               \
        }                                                                   \
    } while (0);

eRet CControl::setAudioProgram(uint16_t pid)
{
    CSimpleAudioDecode * pAudioDecode = _pModel->getSimpleAudioDecode();
    CStc *               pStc         = NULL;
    CPid *               pPidOld      = NULL;
    CPid *               pPidNew      = NULL;
    eRet                 ret          = eRet_Ok;

    BDBG_ASSERT(NULL != pAudioDecode);
    pStc = pAudioDecode->getStc();

    switch (_pModel->getMode())
    {
    case eMode_Live:
    {
        CChannel * pCurrentCh = _pModel->getCurrentChannel();
        BDBG_ASSERT(NULL != pCurrentCh);

        /* give the channel object a chance to set the audio program.
           if it chooses not to, handle it here */
        if (eRet_Ok != pCurrentCh->setAudioProgram(pid))
        {
            /* find pid object matching given pid num */
            pPidNew = pCurrentCh->findPid(pid, ePidType_Audio);
            CHECK_PTR_ERROR_GOTO("unable to find given pid number in current channel", pPidNew, ret, eRet_InvalidParameter, error);

            if (pPidNew != pAudioDecode->getPid())
            {
                /* newly requested pid is different than current so change to it */
                pPidOld = pAudioDecode->stop();
                pPidOld->close();

                ret = pPidNew->open(pCurrentCh->getParserBand());
                CHECK_ERROR_GOTO("unable to open pid channel", ret, error);
                ret = pAudioDecode->start(pPidNew, pStc);
                CHECK_ERROR_GOTO("unable to start audio decoder", ret, error);
            }
        }
    }
    break;

    case eMode_Playback:
    {
        CPlayback *            pPlayback       = _pModel->getPlayback();
        NEXUS_PlaybackPosition position        = 0;
        bool                   restartPlayback = false;

        BDBG_ASSERT(NULL != pPlayback);

        if (pPlayback->isActive())
        {
            restartPlayback = (false == pPlayback->isPaused());
            ret             = pPlayback->pause(false, &position);
            CHECK_ERROR_GOTO("unable to pause playback", ret, error);
        }

        /* find pid object matching given pid num */
        pPidNew = pPlayback->findPid(pid, ePidType_Audio);
        CHECK_PTR_ERROR_GOTO("unable to find given pid number in current channel", pPidNew, ret, eRet_InvalidParameter, error);

        if (pPidNew != pAudioDecode->getPid())
        {
            /* newly requested pid is different than current so change to it */
            pPidOld = pAudioDecode->stop();
            pPidOld->close(pPlayback);

            pPidNew->setAudioDecoder(pAudioDecode);
            ret = pPidNew->open(pPlayback);
            CHECK_ERROR_GOTO("unable to open playback pid", ret, error);

            ret = pAudioDecode->start(pPidNew, pStc);
            CHECK_ERROR_GOTO("unable to start audio decode", ret, error);
        }

        if (pPlayback->isActive())
        {
            if (restartPlayback)
            {
                ret = pPlayback->start(position);
                CHECK_ERROR_GOTO("unable to restart playback", ret, error);
            }
            else
            {
                pPlayback->seek(position);
            }
        }
    }
    break;

    default:
        break;
    } /* switch */

error:
    return(ret);
} /* setAudioProgram */

eRet CControl::setAudioProcessing(eAudioProcessing audioProcessing)
{
    CPlayback *          pPlayback    = _pModel->getPlayback();
    CSimpleAudioDecode * pAudioDecode = _pModel->getSimpleAudioDecode();
    eRet                 ret          = eRet_Ok;

    BDBG_ASSERT(NULL != pAudioDecode);

    AUDIO_DECODE_CHANGE_INIT(pAudioDecode)
    AUDIO_DECODE_CHANGE_START(pAudioDecode, pPlayback)

    pAudioDecode->setAudioProcessing(audioProcessing);

    AUDIO_DECODE_CHANGE_FINISH(pAudioDecode, pPlayback)

error:
    return(ret);
} /* setAudioProcessing */

#ifdef CPUTEST_SUPPORT
/* percent: 0-100 */
eRet CControl::setCpuTestLevel(int nLevel)
{
    CCpuTest * pCpuTest = _pModel->getCpuTest();
    uint32_t   nDelay   = 0;
    eRet       ret      = eRet_Ok;

    BDBG_ASSERT(NULL != pCpuTest);
    BDBG_ASSERT(0 <= nLevel);
    BDBG_ASSERT(100 >= nLevel);

    if (0 < nLevel)
    {
        pCpuTest->start(nLevel);
    }
    else
    {
        pCpuTest->stop();
    }
error:
    return(ret);
} /* setCpuTestLevel */

#endif /* ifdef CPUTEST_SUPPORT */

eRet CControl::setSpdifInput(eSpdifInput spdifInput)
{
    CPlayback *          pPlayback    = _pModel->getPlayback();
    CSimpleAudioDecode * pAudioDecode = _pModel->getSimpleAudioDecode();
    eRet                 ret          = eRet_Ok;

    BDBG_ASSERT(NULL != pAudioDecode);

    AUDIO_DECODE_CHANGE_INIT(pAudioDecode)
    AUDIO_DECODE_CHANGE_START(pAudioDecode, pPlayback)

    pAudioDecode->setSpdifInput(spdifInput);

    AUDIO_DECODE_CHANGE_FINISH(pAudioDecode, pPlayback)

error:
    return(ret);
} /* setSpdifInput */

eRet CControl::setHdmiAudioInput(eHdmiAudioInput hdmiInput)
{
    CPlayback *          pPlayback    = _pModel->getPlayback();
    CSimpleAudioDecode * pAudioDecode = _pModel->getSimpleAudioDecode();
    eRet                 ret          = eRet_Ok;

    BDBG_ASSERT(NULL != pAudioDecode);

    AUDIO_DECODE_CHANGE_INIT(pAudioDecode)
    AUDIO_DECODE_CHANGE_START(pAudioDecode, pPlayback)

    pAudioDecode->setHdmiInput(hdmiInput);

    AUDIO_DECODE_CHANGE_FINISH(pAudioDecode, pPlayback)
error:
    return(ret);
} /* setHdmiAudioInput */

eRet CControl::setAudioDownmix(eAudioDownmix audioDownmix)
{
    CSimpleAudioDecode * pAudioDecode = _pModel->getSimpleAudioDecode();
    CPlayback *          pPlayback    = _pModel->getPlayback();
    eRet                 ret          = eRet_Ok;

    BDBG_ASSERT(NULL != pAudioDecode);

    AUDIO_DECODE_CHANGE_INIT(pAudioDecode)
    AUDIO_DECODE_CHANGE_START(pAudioDecode, pPlayback)

    pAudioDecode->setDownmix(audioDownmix);

    AUDIO_DECODE_CHANGE_FINISH(pAudioDecode, pPlayback)

error:
    return(ret);
} /* setAudioDownmix */

eRet CControl::setAudioDualMono(eAudioDualMono audioDualMono)
{
    CSimpleAudioDecode * pAudioDecode = _pModel->getSimpleAudioDecode();
    CPlayback *          pPlayback    = _pModel->getPlayback();
    eRet                 ret          = eRet_Ok;

    BDBG_ASSERT(NULL != pAudioDecode);

    AUDIO_DECODE_CHANGE_INIT(pAudioDecode)
    AUDIO_DECODE_CHANGE_START(pAudioDecode, pPlayback)

    pAudioDecode->setDualMono(audioDualMono);

    AUDIO_DECODE_CHANGE_FINISH(pAudioDecode, pPlayback)

error:
    return(ret);
} /* setAudioDualMono */

eRet CControl::setDolbyDRC(eDolbyDRC dolbyDRC)
{
    CSimpleAudioDecode * pAudioDecode = _pModel->getSimpleAudioDecode();
    CPlayback *          pPlayback    = _pModel->getPlayback();
    eRet                 ret          = eRet_Ok;

    BDBG_ASSERT(NULL != pAudioDecode);

    AUDIO_DECODE_CHANGE_INIT(pAudioDecode)
    AUDIO_DECODE_CHANGE_START(pAudioDecode, pPlayback)

    pAudioDecode->setDolbyDRC(dolbyDRC);

    AUDIO_DECODE_CHANGE_FINISH(pAudioDecode, pPlayback)

error:
    return(ret);
} /* setDolbyDRC */

eRet CControl::setDolbyDialogNorm(bool dolbyDialogNorm)
{
    CSimpleAudioDecode * pAudioDecode = _pModel->getSimpleAudioDecode();
    CPlayback *          pPlayback    = _pModel->getPlayback();
    eRet                 ret          = eRet_Ok;

    BDBG_ASSERT(NULL != pAudioDecode);

    AUDIO_DECODE_CHANGE_INIT(pAudioDecode)
    AUDIO_DECODE_CHANGE_START(pAudioDecode, pPlayback)

    pAudioDecode->setDolbyDialogNorm(dolbyDialogNorm);

    AUDIO_DECODE_CHANGE_FINISH(pAudioDecode, pPlayback)

error:
    return(ret);
} /* setDolbyDialogNorm */

eRet CControl::setContentMode(NEXUS_VideoWindowContentMode contentMode)
{
    CDisplay * pDisplay = NULL;
    eRet       ret      = eRet_Ok;

    for (uint32_t i = 0; i < _pModel->numDisplays(); i++)
    {
        pDisplay = _pModel->getDisplay(i);
        if (NULL != pDisplay)
        {
            ret = pDisplay->setContentMode(contentMode);
            CHECK_ERROR_GOTO("unable to set content mode", ret, error);
        }
    }

error:
    return(ret);
} /* setContentMode */

eRet CControl::setColorSpace(NEXUS_ColorSpace colorSpace)
{
    CDisplay * pDisplay = NULL;
    eRet       ret      = eRet_Ok;

    for (uint32_t i = 0; i < _pModel->numDisplays(); i++)
    {
        pDisplay = _pModel->getDisplay(i);
        if (NULL != pDisplay)
        {
            ret = pDisplay->setColorSpace(&colorSpace);
            CHECK_ERROR_GOTO("unable to set color space", ret, error);
        }
    }

error:
    return(ret);
} /* setColorSpace */

/* set HDMI output color depth.  video decoder color depth is determined by
 * the atlas.cfg setting: DECODER_COLOR_DEPTH and is set when the main video
 * decoder is initialized (see CAtlas::videoDecodeInitialize()
 */
eRet CControl::setColorDepth(uint8_t colorDepth)
{
    eRet       ret        = eRet_NotSupported;
    CDisplay * pDisplayHD = _pModel->getDisplay(0);

    /* set color depth in hdmi output if it exists */
    if (NULL != pDisplayHD)
    {
        ret = pDisplayHD->setColorDepth(&colorDepth);
        CHECK_ERROR_GOTO("unable to set color depth", ret, error);
    }

error:
    return(ret);
} /* setColorDepth */

eRet CControl::setMpaaDecimation(bool bMpaaDecimation)
{
    CDisplay * pDisplay = NULL;
    eRet       ret      = eRet_Ok;

    for (uint32_t i = 0; i < _pModel->numDisplays(); i++)
    {
        pDisplay = _pModel->getDisplay(i);
        if (NULL != pDisplay)
        {
            ret = pDisplay->setMpaaDecimation(bMpaaDecimation);
            CHECK_ERROR_GOTO("unable to set MPAA decimation", ret, error);
        }
    }

error:
    return(ret);
} /* setMpaaDecimation */

eRet CControl::setDeinterlacer(bool bDeinterlacer)
{
    CDisplay * pDisplay = NULL;
    eRet       ret      = eRet_Ok;

    for (uint32_t i = 0; i < _pModel->numDisplays(); i++)
    {
        pDisplay = _pModel->getDisplay(i);
        if (NULL != pDisplay)
        {
            if (576 >= videoFormatToVertRes(pDisplay->getMaxFormat()).toInt())
            {
                /* SD display so only set deinterlacer if option is enabled */
                if (true == GET_BOOL(_pCfg, DEINTERLACER_ENABLED_SD))
                {
                    BDBG_WRN(("setting deinterlacer on SD display:%s", bDeinterlacer ? "true" : "false"));
                    ret = pDisplay->setDeinterlacer(bDeinterlacer);
                    CHECK_ERROR_GOTO("unable to set deinterlacer", ret, error);
                }
            }
            else
            {
                /* HD display */
                ret = pDisplay->setDeinterlacer(bDeinterlacer);
                CHECK_ERROR_GOTO("unable to set deinterlacer", ret, error);
            }
        }
    }

error:
    return(ret);
} /* setDeinterlacer */

eRet CControl::setBoxDetect(bool bBoxDetect)
{
    CDisplay * pDisplay = NULL;
    eRet       ret      = eRet_Ok;

    for (uint32_t i = 0; i < _pModel->numDisplays(); i++)
    {
        pDisplay = _pModel->getDisplay(i);
        if (NULL != pDisplay)
        {
            ret = pDisplay->setBoxDetect(bBoxDetect);
            CHECK_ERROR_GOTO("unable to set Box Detect", ret, error);
        }
    }

error:
    return(ret);
} /* setBoxDetect */

eRet CControl::setAspectRatio(NEXUS_DisplayAspectRatio aspectRatio)
{
    CDisplay * pDisplay = NULL;
    eRet       ret      = eRet_Ok;

    for (uint32_t i = 0; i < _pModel->numDisplays(); i++)
    {
        pDisplay = _pModel->getDisplay(i);
        if (NULL != pDisplay)
        {
            ret = pDisplay->setAspectRatio(aspectRatio);
            CHECK_ERROR_GOTO("unable to set Aspect Ratio", ret, error);
        }
    }

error:
    return(ret);
} /* setAspectRatio */

eRet CControl::setVideoFormat(NEXUS_VideoFormat videoFormat)
{
    CDisplay *  pDisplayHD = NULL;
    CDisplay *  pDisplaySD = NULL;
    CGraphics * pGraphics  = NULL;
    eRet        ret        = eRet_Ok;

    if (NEXUS_VideoFormat_eUnknown == videoFormat)
    {
        BDBG_WRN(("attempting to set unknown video format"));
        ret = eRet_InvalidParameter;
        goto error;
    }

    pDisplayHD = _pModel->getDisplay(0);
    pDisplaySD = _pModel->getDisplay(1);
    pGraphics  = _pModel->getGraphics();

    BDBG_ASSERT(NULL != pDisplayHD);
    BDBG_ASSERT(NULL != pGraphics);

    if (NULL != pDisplaySD)
    {
        ret = pDisplaySD->setFormat(videoFormat, pGraphics, false);
        CHECK_ERROR_GOTO("unable to set SD video format", ret, error);
    }

    ret = pDisplayHD->setFormat(videoFormat, pGraphics);
    CHECK_ERROR_GOTO("unable to set HD video format", ret, error);

error:
    return(ret);
} /* setVideoFormat */

eRet CControl::setAutoVideoFormat(bool bAutoVideoFormat)
{
    CDisplay * pDisplay = _pModel->getDisplay();
    eRet       ret      = eRet_Ok;

    BDBG_ASSERT(NULL != pDisplay);

    pDisplay->setAutoFormat(bAutoVideoFormat);

    return(ret);
}

/* adds component outputs to either HD or SD display based on current format.
 * if current format on the HD display is unsupported on component, then it
 * is relocated to the SD display. */
eRet CControl::setComponentDisplay(NEXUS_VideoFormat vFormat)
{
    eRet       ret        = eRet_Ok;
    CDisplay * pDisplayHD = _pModel->getDisplay(0);
    CDisplay * pDisplaySD = _pModel->getDisplay(1);

    if ((NULL == pDisplayHD) || (NULL == pDisplaySD))
    {
        return(ret);
    }

    /* check HD display for component outputs and move to proper display if necessary */
    {
        MList <COutput> * pListOutputs = pDisplayHD->getOutputList();
        COutput *         pOutput      = NULL;

        for (pOutput = pListOutputs->first(); pOutput; pOutput = pListOutputs->next())
        {
            if (eBoardResource_outputComponent == pOutput->getType())
            {
                if (false == pOutput->isValidVideoFormat(vFormat))
                {
                    BDBG_WRN(("Current HD display video format (%s) is UNsupported on Component outputs - move to SD display",
                              videoFormatToString(vFormat).s()));
                    /* move component to SD display*/
                    pDisplayHD->removeOutput(pOutput);
                    ret = pDisplaySD->addOutput(pOutput);
                    CHECK_ERROR_GOTO("Unable to add component output to SD display", ret, error);
                }
            }
        }
    }

    /* check SD display for component outputs and move to proper display if necessary */
    {
        MList <COutput> * pListOutputs = pDisplaySD->getOutputList();
        COutput *         pOutput      = NULL;

        for (pOutput = pListOutputs->first(); pOutput; pOutput = pListOutputs->next())
        {
            if (eBoardResource_outputComponent == pOutput->getType())
            {
                if (true == pOutput->isValidVideoFormat(vFormat))
                {
                    BDBG_WRN(("Current HD display video format (%s) IS supported on Component outputs - move to HD display",
                              videoFormatToString(vFormat).s()));
                    /* move component to HD display */
                    pDisplaySD->removeOutput(pOutput);
                    ret = pDisplayHD->addOutput(pOutput);
                    CHECK_ERROR_GOTO("Unable to add component output to HD display", ret, error);
                }
            }
        }
    }

error:
    return(ret);
} /* setComponentDisplay */

eRet CControl::setOptimalVideoFormat(
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
        MList <COutput> *  pListOutputs = pDisplay->getOutputList();
        COutput *          pOutput      = NULL;
        NEXUS_VideoFormat  formatMax    = NEXUS_VideoFormat_eUnknown;
        NEXUS_VideoFormat  formatTemp   = NEXUS_VideoFormat_eUnknown;
        MListItr <COutput> itr(pListOutputs);

        /* check video outputs for optimal video format */
        for (pOutput = pListOutputs->first(); pOutput; pOutput = pListOutputs->next())
        {
            if (eRet_Ok == pVideoDecode->getOptimalVideoFormat(pOutput, &formatTemp))
            {
                if (videoFormatToVertRes(formatTemp).toInt() > videoFormatToVertRes(formatMax).toInt())
                {
                    formatMax = formatTemp;
                }
            }
        }

        if (NEXUS_VideoFormat_eUnknown == formatMax)
        {
            BDBG_WRN(("unable to find valid video format!"));
            ret = eRet_NotSupported;
        }
        else
        {
            ret = setComponentDisplay(formatMax);
            CHECK_ERROR_GOTO("Unable to set component to proper display", ret, error);

            ret = setVideoFormat(formatMax);
            CHECK_ERROR_GOTO("Unable to set video format", ret, error);
        }
    }

error:
    return(ret);
} /* setOptimalVideoFormat */

/* sets the main and pip window geometry. */
eRet CControl::setWindowGeometry()
{
    MList<CDisplay> *   pDisplayList = _pModel->getDisplayList();
    CDisplay *          pDisplay     = NULL;
    MListItr <CDisplay> itr(pDisplayList);
    eRet                ret = eRet_Ok;

    for (pDisplay = pDisplayList->first(); pDisplay; pDisplay = pDisplayList->next())
    {
        ret = pDisplay->updateVideoWindowGeometry();
        CHECK_ERROR_GOTO("unable to update video window geometry", ret, error);
    }

error:
    return(ret);
} /* setWindowGeometry */

eRet CControl::showWindowType(
        eWindowType windowType,
        bool        bShow
        )
{
    eRet                 ret          = eRet_Ok;
    CSimpleVideoDecode * pVideoDecode = _pModel->getSimpleVideoDecode(windowType);

    MList<CVideoWindow> * pVideoWindowList = pVideoDecode->getVideoWindowList();
    CVideoWindow *        pVideoWindow     = NULL;

    for (pVideoWindow = pVideoWindowList->first(); pVideoWindow; pVideoWindow = pVideoWindowList->next())
    {
        pVideoWindow->setVisible(bShow);
    }

    return(ret);
} /* showWindowType */

eRet CControl::applyVbiSettings(uint32_t nDisplayIndex)
{
    CDisplay * pDisplay = _pModel->getDisplay(nDisplayIndex);
    eRet       ret      = eRet_Ok;

    if (NULL == pDisplay)
    {
        return(eRet_InvalidState);
    }

    CDisplayVbiData vbiSettings = pDisplay->getVbiSettings();
    ret = pDisplay->setVbiSettings(&vbiSettings);
    CHECK_ERROR("unable to set VBI settings after pip swap", ret);

error:
    return(ret);
} /* applyVbiSettings */

eRet CControl::showPip(bool bShow)
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

    showWindowType(pipWinType, bShow);
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

eRet CControl::swapPip()
{
    eRet                 ret                 = eRet_InvalidState;
    CSimpleAudioDecode * pAudioDecode        = NULL;
    CStc *               pStc                = NULL;
    eMode                mode                = eMode_Max;
    CPlayback *          pPlayback           = _pModel->getPlayback(_pModel->getFullScreenWindowType());
    CChannel *           pChannel            = _pModel->getCurrentChannel();
    bool                 bSingleAudioDecoder = false;

#ifdef DVR_LIB_SUPPORT
    CTsb * pTsb = _pModel->getTsb(_pModel->getFullScreenWindowType());
#endif
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

    /* if we only have one audio decoder, we will swap it between main and pip.  the downside
     * to this is that if we are swapping a playback, audio will lag for several seconds while
     * the buffer fills up.  if we have 2 audio decoders, audio will be primed and ready
     * after swap */
    bSingleAudioDecoder = ((NULL == _pModel->getSimpleAudioDecode()) ||
                           (NULL == _pModel->getSimpleAudioDecode(_pModel->getPipScreenWindowType())));

#if DVR_LIB_SUPPORT
    if (pTsb)
    {
        pTsb->setSimpleAudioDecode(NULL);
    }
    else
#endif /* if DVR_LIB_SUPPORT */
    /* First Change the Full Screen PVR */

    if (true == bSingleAudioDecoder)
    {
        /* we only have 1 audio decoder */
        if (pPlayback != NULL)
        {
            CPid * pAudioPid = NULL;
            pAudioDecode = _pModel->getSimpleAudioDecode();
            CHECK_PTR_ERROR_GOTO("pAudioDecode null", pAudioDecode, ret, eRet_NotAvailable, error);
            CHECK_PTR_ERROR_GOTO("pPlayback null", pPlayback, ret, eRet_NotAvailable, error);
            pAudioDecode->stop();

            BDBG_MSG(("Turn off Audio to PLAYBACK %s audio", pPlayback->getVideoName().s()));
            BDBG_MSG(("WINDOW fullscreen %d", _pModel->getFullScreenWindowType()));

            pAudioPid = pPlayback->getPid(0, ePidType_Audio);
            if (NULL != pAudioPid)
            {
                pAudioPid->setAudioDecoder(NULL);
                ret = pAudioPid->setSettings(pPlayback);
                CHECK_ERROR_GOTO("error setting playback in audio pid", ret, error);
            }
        }

        /* swap main/pip audio decoders (actually there is only 1 audio decoder so this will simply
         * move it between main and pip so the fullscreen window will always have audio */
        _pModel->swapSimpleAudioDecode();
    }
    else
    {
        /* we have 2 audio decoders */
        CSimpleAudioDecode * pAudioDecodeFullScreen = _pModel->getSimpleAudioDecode();
        CSimpleAudioDecode * pAudioDecodeDecimated  = _pModel->getSimpleAudioDecode(_pModel->getPipScreenWindowType());

        NEXUS_SimpleAudioDecoder_SwapServerSettings(
                _pModel->getSimpleAudioDecoderServer(),
                pAudioDecodeFullScreen->getSimpleDecoder(),
                pAudioDecodeDecimated->getSimpleDecoder());
    }

    /* keep track of which content is full screen (main or pip) */
    _pModel->setFullScreenWindowType((eWindowType_Main == _pModel->getFullScreenWindowType()) ? eWindowType_Pip : eWindowType_Main);

    /* the following model variables will be basd on which window type is currently fullscreen */
    pAudioDecode = _pModel->getSimpleAudioDecode();
    pStc         = _pModel->getStc();
    mode         = _pModel->getMode();

    BDBG_ASSERT(NULL != pAudioDecode);
    BDBG_ASSERT(NULL != pStc);

    /* swap decoder to video window connections between the main/pip decoders */
    _pModel->swapDecodeVideoWindows();

    if (true == bSingleAudioDecoder)
    {
        CPid * pAudioPid = NULL;

        pAudioDecode->stop();

        /* setup audio decode with tsm POST swap */
        switch (mode)
        {
        case eMode_Live:
        {
            CChannel * pChannel = _pModel->getCurrentChannel();
            if (NULL != pChannel)
            {
                BDBG_MSG(("swap to CHANNEL %s audio", pChannel->getChannelNum().s()));
                pAudioPid = pChannel->getPid(0, ePidType_Audio);
#ifdef DVR_LIB_SUPPORT
                pTsb = _pModel->getTsb();
                pTsb->setSimpleAudioDecode(pAudioDecode);
#endif
            }
            break;
        }
        case eMode_Playback:
        {
            pPlayback = _pModel->getPlayback();
            if (NULL != pPlayback)
            {
                BDBG_MSG(("swap to PLAYBACK %s audio", pPlayback->getVideoName().s()));
                pAudioPid = pPlayback->getPid(0, ePidType_Audio);
            }
            break;
        }
#ifdef DVR_LIB_SUPPORT
        case eMode_Tsb:
        {
            pTsb = _pModel->getTsb();
            pTsb->setSimpleAudioDecode(pAudioDecode);
            pAudioPid = pTsb->getTsbPlaybackAudioPid();
            pStc      = pTsb->getStc();
            break;
        }
#endif /* ifdef DVR_LIB_SUPPORT */
        default:
            break;
        } /* switch */

        if ((NULL != pAudioPid) && (pAudioPid->isOpen()))
        {
            ret = pAudioDecode->start(pAudioPid, pStc);
            CHECK_ERROR_GOTO("unable to start audio decode", ret, error);
        }
    }

    /* restart VBI closed captioning based on full screen video window */
    ret = applyVbiSettings();
    CHECK_ERROR("unable to set VBI settings after pip swap", ret);

    ret = eRet_Ok;

error:
    /* notify that pip state has changed - no actual change in pip state value */
    _pModel->setPipState(_pModel->getPipState());
    return(ret);
} /* swapPip */

ePowerMode CControl::getPowerMode()
{
    CBoardResources * pBoardResources = NULL;
    CPower *          pPower          = NULL;
    ePowerMode        mode            = ePowerMode_Max;

    pBoardResources = _pConfig->getBoardResources();
    BDBG_ASSERT(NULL != pBoardResources);
    pPower = (CPower *)pBoardResources->checkoutResource(_id, eBoardResource_power);

    mode = pPower->getMode();

    pBoardResources->checkinResource(pPower);
    return(mode);
}

eRet CControl::ipServerStart()
{
    eRet                   ret                  = eRet_Ok;
    CServerMgr *           pServerMgr           = _pModel->getServerMgr();
    CAutoDiscoveryClient * pAutoDiscoveryClient = _pModel->getAutoDiscoveryClient();

    ATLAS_MEMLEAK_TRACE("BEGIN");

    /* start atlas server manager */
    if (NULL != pServerMgr)
    {
        ret = pServerMgr->startHttpServer();
        CHECK_ERROR_GOTO("unable to start ip http server", ret, error);
        ret = pServerMgr->startPlaylistServer();
        CHECK_ERROR_GOTO("unable to start ip playlist server", ret, error);

        /* Start UDP Streamer, optional */
        pServerMgr->startUdpServer();
    }

    /* start auto discovery client */
    if (NULL != pAutoDiscoveryClient)
    {
        ret = pAutoDiscoveryClient->start();
        CHECK_ERROR_GOTO("unable to start auto discovery client object", ret, error);
    }

error:
    ATLAS_MEMLEAK_TRACE("END");
    return(ret);
} /* ipServerStart */

eRet CControl::ipServerStop()
{
    eRet                   ret                  = eRet_Ok;
    CServerMgr *           pServerMgr           = _pModel->getServerMgr();
    CAutoDiscoveryClient * pAutoDiscoveryClient = _pModel->getAutoDiscoveryClient();

    /* stop atlas server manager */
    if (NULL != pServerMgr)
    {
        ret = pServerMgr->stopHttpServer();
        CHECK_ERROR("unable to stop ip http server", ret);
        ret = pServerMgr->stopPlaylistServer();
        CHECK_ERROR("unable to stop ip playlist server", ret);

        /* optional */
        pServerMgr->stopUdpServer();
    }

    /* stop auto discovery client */
    if (NULL != pAutoDiscoveryClient)
    {
        ret = pAutoDiscoveryClient->stop();
        CHECK_ERROR_GOTO("unable to stop auto discovery client object", ret, error);
    }

error:
    return(ret);
} /* ipServerStop */

eRet CControl::setPowerMode(ePowerMode mode)
{
    eRet              ret             = eRet_Ok;
    CBoardResources * pBoardResources = NULL;
    CPower *          pPower          = NULL;
    CDisplay *        pDisplayHD      = _pModel->getDisplay(0);
    CDisplay *        pDisplaySD      = _pModel->getDisplay(1);
    CGraphics *       pGraphics       = _pModel->getGraphics();

    pBoardResources = _pConfig->getBoardResources();
    BDBG_ASSERT(NULL != pBoardResources);
    pPower = (CPower *)pBoardResources->checkoutResource(_id, eBoardResource_power);

    BDBG_MSG(("CControl::%s mode:%d", __FUNCTION__, mode));
    if (pPower->getMode() == mode)
    {
        /* requested mode matches existing mode - still call setMode() which
         * will send out mode change notification that views may be waiting for */
        ret = pPower->setMode(mode);
        CHECK_ERROR_GOTO("Unable to re-set power mode", ret, error);
        goto done;
    }

    switch (mode)
    {
    case ePowerMode_S0:
        /* On */
    {
        ret = pPower->setMode(mode, pGraphics);
        CHECK_ERROR_GOTO("unable to set power mode S0", ret, error);

        if (NULL != pDisplayHD)
        {
            pDisplayHD->enableOutputs(true);
        }
        if (NULL != pDisplaySD)
        {
            pDisplaySD->enableOutputs(true);
        }

        /* enabling outputs only happens on vsync so add slight delay */
        BKNI_Sleep(100);

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
                ret = unTuneChannel(pChannel, true);
                CHECK_ERROR_GOTO("unable to untune channel", ret, error);
            }
        }
    }

        if (NULL != pDisplayHD)
        {
            pDisplayHD->enableOutputs(false);
        }
        if (NULL != pDisplaySD)
        {
            pDisplaySD->enableOutputs(false);
        }

        /* disabling outputs only happens on vsync so add slight delay */
        BKNI_Sleep(100);

        ret = pPower->setMode(mode, pGraphics);
        CHECK_ERROR_GOTO("unable to set power mode", ret, error);

        break;

    default:
        break;
    } /* switch */

error:
done:
    pBoardResources->checkinResource(pPower);
    return(ret);
} /* setPower */

eRet CControl::connectDecoders(
        CSimpleVideoDecode * pVideoDecode,
        CSimpleAudioDecode * pAudioDecode,
        uint32_t             width,
        uint32_t             height,
        CPid *               pVideoPid,
        eWindowType          windowType
        )
{
    eRet ret = eRet_Ok;

    BSTD_UNUSED(pAudioDecode);

    if (NULL != pVideoDecode)
    {
        NEXUS_VideoCodec videoCodec = NEXUS_VideoCodec_eUnknown;
        if (NULL != pVideoPid)
        {
            videoCodec = pVideoPid->getVideoCodec();
        }

        /* set max width/height in video decoder (primarily for 4k support) */
        if ((0 == width) &&
            (0 == height) &&
            (NEXUS_VideoCodec_eH265 == videoCodec) &&
            (eWindowType_Main == windowType))
        {
            /* assume max size is 4K video resolution if codec is H.265 (HEVC)
             * and width/height is unknown */
            ret = pVideoDecode->setMaxSize(3840, 2160);
        }
        else
        {
            /* setMaxSize() may not honor request if platform does not support given sizes */
            ret = pVideoDecode->setMaxSize(width, height);
        }
        CHECK_WARN("unable to set max video decoder size", ret);
    }

    return(ret);
} /* connectDecoders */

void CControl::disconnectDecoders(eWindowType winType)
{
    BSTD_UNUSED(winType);
    return;
} /* disconnectDecoders() */

eRet CControl::startDecoders(
        CSimpleVideoDecode * pVideoDecode,
        CPid *               pVideoPid,
        CSimpleAudioDecode * pAudioDecode,
        CPid *               pAudioPid,
        CStc *               pStc
        )
{
    eRet ret = eRet_Ok;

    if ((NULL != pVideoDecode) && (NULL != pVideoPid))
    {
        ret = pVideoDecode->start(pVideoPid, pStc);
        CHECK_WARN("unable to start video decode", ret);
    }

    if ((NULL != pAudioDecode) && (NULL != pAudioPid))
    {
        ret = pAudioDecode->start(pAudioPid, (NULL != pVideoPid) ? pStc : NULL);
        CHECK_WARN("unable to start audio decode", ret);
    }

    {
        eRet retVbi = eRet_Ok;
        retVbi = applyVbiSettings();
        CHECK_WARN("unable to set VBI settings after decoder start", retVbi);
    }

    return(ret);
} /* startDecoders */

eRet CControl::stopDecoders(
        CSimpleVideoDecode * pVideoDecode,
        CSimpleAudioDecode * pAudioDecode
        )
{
    eRet ret = eRet_Ok;

    if (NULL != pVideoDecode)
    {
        pVideoDecode->stop();
    }
    if (NULL != pAudioDecode)
    {
        pAudioDecode->stop();
    }

    return(ret);
} /* stopDecoders */
