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

#ifdef PLAYBACK_IP_SUPPORT

#include "channelmgr.h"
#include "nexus_core_utils.h"
#include "channel_bip.h"
#include "convert.h"
#include "audio_decode.h"
#include "murl.h"

BDBG_MODULE(atlas_channel_bip);

static void playbackCallbackERRORFromBIP(
        void * context,
        int    param
        )
{
    BSTD_UNUSED(param);
    CChannelBip * pChannel = (CChannelBip *) context;

    BDBG_ERR((" ERRORplay backCallbackERRORFromBIP "));

    if (pChannel)
    {
        pChannel->setAction(BMediaPlayerAction_eError);
        CWidgetEngine * pWidgetEngine = pChannel->getWidgetEngine();
        if (pWidgetEngine != NULL)
        {
            pWidgetEngine->syncCallback(pChannel, CALLBACK_TUNER_LOCK_STATUS_BIP);
        }
    }
    BDBG_MSG(("%s: Got EventId from IP library, Channel Ip %p", __FUNCTION__, (void *)pChannel));
} /* playbackCallbackERRORFromBIP */

static void playbackCallbackFromBIP(
        void * context,
        int    param
        )
{
    BSTD_UNUSED(param);
    CChannelBip * pChannel = (CChannelBip *) context;

    BDBG_MSG((" Callback From BIP "));
    if (pChannel)
    {
        pChannel->setAction(BMediaPlayerAction_eEof);
        CWidgetEngine * pWidgetEngine = pChannel->getWidgetEngine();
        if (pWidgetEngine != NULL)
        {
            pWidgetEngine->syncCallback(pChannel, CALLBACK_TUNER_LOCK_STATUS_BIP);
        }
    }
    BDBG_MSG(("%s: Got EventId from IP library, Channel Ip %p", __FUNCTION__, (void *)pChannel));
} /* playbackCallbackFromBIP */

static void asyncCallbackFromBIP(
        void * context,
        int    param
        )
{
    BSTD_UNUSED(param);
    CChannelBip * pChannel = (CChannelBip *) context;

    if (pChannel)
    {
        CWidgetEngine * pWidgetEngine = pChannel->getWidgetEngine();
        if (pWidgetEngine != NULL)
        {
            pWidgetEngine->syncCallback(pChannel, CALLBACK_TUNER_LOCK_STATUS_BIP);
        }
    }
    BDBG_MSG(("%s: Got EventId from IP library, Channel Ip %p", __FUNCTION__, (void *)pChannel));
}

static void bwinTunerLockStatusCallback(
        void *       pObject,
        const char * strCallback
        )
{
    CChannelBip * pChannel  = (CChannelBip *)pObject;
    BIP_Status    bipStatus = BIP_SUCCESS;

    BDBG_MSG((" bwinTunerLockStatusCallback"));
    BSTD_UNUSED(strCallback);

    if (pChannel)
    {
        bipStatus = pChannel->mediaStateMachine(BMediaPlayerAction_eMax);
        if (bipStatus != BIP_SUCCESS)
        {
            BDBG_WRN(("Calling Media State Machine status is still %d", bipStatus));
        }
    }
} /* bwinTunerLockStatusCallback */

BIP_Status CChannelBip::restartDecode(void)
{
    BIP_PlayerSettings playerSettings;
    BIP_Status         status = BIP_SUCCESS;

    if (_pPlayer)
    {
        BIP_Player_GetSettings(_pPlayer, &playerSettings);
        if (_pAudioDecode)
        {
            playerSettings.audioTrackSettings.pidTypeSettings.audio.simpleDecoder = _pAudioDecode->getSimpleDecoder();
        }
        if (_pVideoDecode)
        {
            playerSettings.videoTrackSettings.pidTypeSettings.video.simpleDecoder = _pVideoDecode->getSimpleDecoder();
        }
        if (_pStc)
        {
            playerSettings.playbackSettings.simpleStcChannel = _pStc->getSimpleStcChannel();
        }
        status = BIP_Player_SetSettings(_pPlayer, &playerSettings);
        if (status != BIP_SUCCESS)
        {
            BDBG_ERR((" restart has Failed"));
            goto error;
        }
    }
error:
    BDBG_WRN((" Out of Function"));
    return(status);
} /* restartDecode */

unsigned int CChannelBip::getLastPosition(void)
{
    unsigned int position = 0;

    if (getState() != BMediaPlayerState_eDisconnected)
    {
        BIP_Status       bipStatus;
        BIP_PlayerStatus playerStatus;

        bipStatus = BIP_Player_GetStatus(_pPlayer, &playerStatus);
        if (bipStatus != BIP_SUCCESS)
        {
            BDBG_WRN((" cannot get Player status"));
        }
        else
        {
            position = playerStatus.lastPositionInMs;
        }
    }

    return(position);
} /* getLastPosition */

unsigned int CChannelBip::getCurrentPosition(void)
{
    unsigned int position = 0;

    if (getState() != BMediaPlayerState_eDisconnected)
    {
        BIP_Status       bipStatus;
        BIP_PlayerStatus playerStatus;

        bipStatus = BIP_Player_GetStatus(_pPlayer, &playerStatus);
        if (bipStatus != BIP_SUCCESS)
        {
            BDBG_WRN((" cannot get Player status"));
        }
        else
        {
            position = playerStatus.currentPositionInMs;
        }
    }

    return(position);
} /* getCurrentPosition */

eRet CChannelBip::setState(BMediaPlayerState state)
{
    eRet ret = eRet_Ok;

    _playerState = state;

    if ((BMediaPlayerState_eStarted == _playerState) ||
        (BMediaPlayerState_eDisconnected == _playerState))
    {
        /* only notify on start and disconnect - note that there will be multiple
         * duplicate notifications, but we will need multiple start notifications
         * since the stream duration will not be available on the first one. */
        ret = notifyObservers(eNotify_ChannelStateChanged, this);
        CHECK_ERROR_GOTO("error notifying observers", ret, error);
    }

error:
    return(ret);
} /* setState */

void CChannelBip::getStats(void)
{
    BIP_PlayerStatus    pStatus;
    BIP_Status          bipStatus = BIP_SUCCESS;
    MUrl                mUrl(_url.s());
    MString             protocol = mUrl.protocol();
    BDBG_INT64_DEC_BUF(buf);

    if( (_pPlayer != NULL) && (protocol.strncmp("rtp") > -1)) {
        bipStatus = BIP_Player_GetStatus(_pPlayer,&pStatus);
        if ( bipStatus != BIP_SUCCESS)
        {
            BDBG_ERR(("Failed BIP_Player_GetStatus(), bipStatus = %d",bipStatus));
            goto error;
        }
        BDBG_WRN(("--------------------------------------------------"));
        BDBG_WRN(("\n totalConsumed                    = %lld\n"
                  " bytesReceived                    = " BDBG_INT64_DEC_FMT "\n"
                  " packetsReceived                  = %u\n"
                  " packetsDiscarded                 = %u\n"
                  " packetsOutOfSequence             = %u\n"
                  " packetsLost                      = %u\n"
                  " lossEvents                       = %u\n"
                  " packetsLostBeforeErrorCorrection = %u\n"
                  " lossEventsBeforeErrorCorrection  = %u",
                pStatus.stats.totalConsumed,
                BDBG_INT64_DEC_ARG(buf,pStatus.stats.rtpStats.bytesReceived),
                pStatus.stats.rtpStats.packetsReceived,
                pStatus.stats.rtpStats.packetsDiscarded,
                pStatus.stats.rtpStats.packetsOutOfSequence,
                pStatus.stats.rtpStats.packetsLost,
                pStatus.stats.rtpStats.lossEvents,
                pStatus.stats.rtpStats.packetsLostBeforeErrorCorrection,
                pStatus.stats.rtpStats.lossEventsBeforeErrorCorrection));
        BDBG_WRN(("Stream Protocol %s",protocol.s()));
        BDBG_WRN(("--------------------------------------------------"));
    }
    notifyObservers(eNotify_ChannelStatsShown, NULL);
error:
  return;
}

BIP_Status CChannelBip::mediaStateMachine(BMediaPlayerAction playerAction)
{
    eRet             ret       = eRet_Ok;
    BIP_Status       bipStatus = BIP_SUCCESS;
    BIP_PlayerStatus playerStatus;
    CModel *         pModel = getModel();

#if __COVERITY__
    __coverity_stack_depth__(100*1024);
#endif

    if (_pPlayer == NULL)
    {
        goto error;
    }

    if (playerAction == BMediaPlayerAction_eUnTune)
    {
        BDBG_MSG(("Player action to untune"));
        _playerState = BMediaPlayerState_eMax;
        goto error;
    }

    if (getState() != BMediaPlayerState_eDisconnected)
    {
        bipStatus = BIP_Player_GetStatus(_pPlayer, &playerStatus);
    }
    if (pModel->getIpClientTranscodeEnabled())
    {
        BIP_String_StrcpyChar(_pUrl, MString(_url + ".xcode"));

        if (pModel->getIpClientTranscodeProfile() == 1)
        {
            BIP_String_StrcpyChar(_pUrl, MString(_url + ".xcode"+ ".480p"));
        }
        else
        if (pModel->getIpClientTranscodeProfile() == 2)
        {
            BIP_String_StrcpyChar(_pUrl, MString(_url + ".xcode"+ ".720p"));
        }
        pModel->setIpClientTranscodeEnabled(false);
    }

    switch (getState())
    {
    case BMediaPlayerState_eDisconnected:
    {
        BIP_PlayerConnectSettings settings;

        BDBG_MSG(("In Disconnected state, start Connect processing on URL=%s", BIP_String_GetString(_pUrl)));
        BIP_Player_GetDefaultConnectSettings(&settings);
        settings.pUserAgent       = "Atlas BIP Player";
        if (BIP_String_GetLength(_pInterfaceName))
        {
            settings.pNetworkInterfaceName = BIP_String_GetString(_pInterfaceName);
            BDBG_MSG(("Interface value being passed in is %s ", BIP_String_GetString(_pInterfaceName)));
        }
        _asyncApiCompletionStatus = BIP_INF_IN_PROGRESS;
        bipStatus                 = BIP_Player_ConnectAsync(_pPlayer, BIP_String_GetString(_pUrl), &settings, &_asyncCallbackDesc, &_asyncApiCompletionStatus);
        if ((bipStatus != BIP_INF_IN_PROGRESS) && (bipStatus != BIP_SUCCESS))
        {
            BDBG_ERR(("BIP_Player_ProbeAsync Failed!"));
            goto error;
        }
        BDBG_MSG(("Connect Async started.."));
        setState(BMediaPlayerState_eWaitingForConnect);
        break;
    }

    case BMediaPlayerState_eWaitingForConnect:
    {
        BDBG_MSG(("BMediaPlayerState_eWaitingForConnect"));
        BIP_PlayerProbeMediaInfoSettings probeMediaInfoSettings;

        BIP_CHECK_GOTO((_asyncApiCompletionStatus == BIP_INF_IN_PROGRESS || _asyncApiCompletionStatus == BIP_SUCCESS), ("BIP_Player_ConnectAsync() Failed!"), error, _asyncApiCompletionStatus, bipStatus);
        if (_asyncApiCompletionStatus == BIP_SUCCESS)
        {
            BDBG_MSG(("BIP_Player_ConnectAsync Success!, BipStatus %d", bipStatus));

            /* BIP_Player_ConnectAsync() is successful, print & parse any custom HTTP response headers. */
            if (_pidMgr.isEmpty() && _enableDynamicTrackSelection)
            {
                BIP_Player_GetDefaultProbeMediaInfoSettings(&probeMediaInfoSettings);
                _asyncApiCompletionStatus = BIP_INF_IN_PROGRESS;
                _pMediaInfo               = NULL;

                BDBG_WRN(("In Connected state, start Probe processing.."));
                bipStatus = BIP_Player_ProbeMediaInfoAsync(_pPlayer, &probeMediaInfoSettings, &_pMediaInfo, &_asyncCallbackDesc, &_asyncApiCompletionStatus);
                if ((bipStatus != BIP_INF_IN_PROGRESS) && (bipStatus != BIP_SUCCESS))
                {
                    BDBG_ERR(("BIP_Player_ProbeAsync Failed!"));
                    goto error;
                }

                BDBG_MSG(("Media Probe processing started.."));
                setState(BMediaPlayerState_eWaitingForProbe);
            }
            else
            {
                /* SWSTB-3854: This is an issue in BIP , We must sleep to give BIP engouh time to get ready
                   for Prepare. Without delay when _enableDynamicTrackEnable is set to false resources are not
                   released quickly. */
                BKNI_Sleep(1000);
                BDBG_MSG((" Dynamic Tracks , use pids in pidMgr, Skip to Prepare"));
                setState(BMediaPlayerState_eWaitingForPrepare);
                notifyObservers(eNotify_NonTunerLockStatus, this);
            }
        }
        break;
    }

    case BMediaPlayerState_eWaitingForProbe:
    {
        BDBG_MSG(("BMediaPlayerState_eWaitingForProbe"));
        BIP_CHECK_GOTO((_asyncApiCompletionStatus == BIP_INF_IN_PROGRESS || _asyncApiCompletionStatus == BIP_SUCCESS), ("BIP_Player_ProbeMediaInfoAsync() Failed!"), error, _asyncApiCompletionStatus, bipStatus);
        if (_asyncApiCompletionStatus == BIP_SUCCESS)
        {
            bipStatus = BIP_SUCCESS;
            BDBG_MSG(("Probe has finished and now we are waiting to Prepare, call bwin Lock Status callback "));

            setState(BMediaPlayerState_eWaitingForPrepare);
            notifyObservers(eNotify_NonTunerLockStatus, this);
        }
        break;
    }

    case BMediaPlayerState_eWaitingForPrepare:
    {
        /* Callback was fired now Probe is done and we need to call NON ASYNC prepare */
        BIP_CHECK_GOTO((_asyncApiCompletionStatus == BIP_INF_IN_PROGRESS || _asyncApiCompletionStatus == BIP_SUCCESS), ("BIP_Player_PrepareAsync() Failed!"), error, _asyncApiCompletionStatus, bipStatus);
        BDBG_MSG(("In BMediaPlayerState_eWaitingForPrepare . "));
        /* Need to make sure that we set that PIDS are preset and we don't need BIP to prepare */
        if ((_enableDynamicTrackSelection == false) || ((_asyncApiCompletionStatus == BIP_SUCCESS) && (playerAction == BMediaPlayerAction_ePrepare)))
        {
            BIP_PlayerPrepareSettings prepareSettings;
            BIP_PlayerSettings        playerSettings;
            BIP_Status                bipStatus;
            BIP_PlayerPrepareStatus   prepareStatus;

            BIP_Player_GetDefaultPrepareSettings(&prepareSettings);
            BIP_Player_GetDefaultSettings(&playerSettings);
            /* Fill in after we get the Default Settings */
            BIP_Player_GetSettings(_pPlayer, &playerSettings);

            if (!_enableDynamicTrackSelection)
            {
                /* send this info to the player because we are in Dynamic mode now. Pids are set! */
                playerSettings.enableDynamicTrackSelection                                    = false;
                if(_pidMgr.getPid(0, ePidType_Video)) {
                    playerSettings.videoTrackId                                               = (_pidMgr.getPid(0, ePidType_Video))->getPid();
                    playerSettings.videoTrackSettings.pidTypeSettings.video.codec             = (_pidMgr.getPid(0, ePidType_Video))->getVideoCodec();
                }
                if(_pidMgr.getPid(0, ePidType_Audio)) {
                    playerSettings.audioTrackId                                               = (_pidMgr.getPid(0, ePidType_Audio))->getPid();
                    playerSettings.audioTrackSettings.pidSettings.pidTypeSettings.audio.codec = (_pidMgr.getPid(0, ePidType_Audio))->getAudioCodec();
                }

                if(!_pidMgr.getPid(0, ePidType_Video) && !_pidMgr.getPid(0, ePidType_Audio)) {
                    BDBG_ERR((" No Audio of Video Pids! _enableDynamicTrackSelection is set"));
                    goto error;
                }

                /* Add PCR pid only for Transport TS  */
                if ((_pidMgr.getTransportType() == NEXUS_TransportType_eTs )&& _pidMgr.getPid(0, ePidType_Pcr) )
                {
                    /* All fields should have been copied over, but overwrite the PCR field */
                    _playerStreamInfo.mpeg2Ts.pcrPid = (_pidMgr.getPid(0, ePidType_Pcr))->getPid();
                    _playerStreamInfo.containerType  = BIP_PlayerContainerType_eNexusTransportType;
                    _playerStreamInfo.transportType  = NEXUS_TransportType_eTs;
                }

                BIP_Player_SetSettings(_pPlayer, &playerSettings);
            }
            else
            {
                /* Double check playerStreamInfo when Stream Pids are not Hardcoded */
                BIP_Player_GetProbedStreamInfo(_pPlayer, &_playerStreamInfo);
                setDurationInMsecs(_playerStreamInfo.durationInMs);
            }

            BDBG_MSG(("Settings trackGroupId to this programNumber %d", _programNumber));
            /* Fill-in AV decoder handles. */
            if (_pAudioDecode)
            {
                playerSettings.audioTrackSettings.pidTypeSettings.audio.simpleDecoder = _pAudioDecode->getSimpleDecoder();
            }
            if (_pVideoDecode)
            {
                playerSettings.videoTrackSettings.pidTypeSettings.video.simpleDecoder = _pVideoDecode->getSimpleDecoder();
            }
            playerSettings.playbackSettings.simpleStcChannel = _pStc->getSimpleStcChannel();

            /* Fill-in callbacks */
            playerSettings.playbackSettings.endOfStreamCallback.callback = playbackCallbackFromBIP;
            playerSettings.playbackSettings.endOfStreamCallback.context  = this;
            playerSettings.playbackSettings.errorCallback.callback       = playbackCallbackERRORFromBIP;
            playerSettings.playbackSettings.errorCallback.context        = this;

            bipStatus = BIP_Player_Prepare(_pPlayer, &prepareSettings, &playerSettings, NULL /*&probeSettings*/, &_playerStreamInfo, &prepareStatus);
            if (bipStatus != BIP_SUCCESS)
            {
                BDBG_ERR(("BIP_Player_Prepare Failed: URL=%s", BIP_String_GetString(_pUrl)));
                goto error;
            }

            {
                CPid * videoPid = _pidMgr.getPid(0, ePidType_Video);
                if (videoPid) {
                    videoPid->dump();
                }
                CPid * audioPid = _pidMgr.getPid(0, ePidType_Audio);
                if(audioPid) {
                    audioPid->dump();
                }
                if (!prepareStatus.hVideoPidChannel && !prepareStatus.hAudioPidChannel)
                {
                    BDBG_ERR(("no audio or video Pid Channels"));
                    goto error;
                }

                if (prepareStatus.hVideoPidChannel && videoPid)
                {
                    videoPid->setPidChannel(prepareStatus.hVideoPidChannel);
                }

                if (prepareStatus.hAudioPidChannel && audioPid)
                {
                    audioPid->setPidChannel(prepareStatus.hAudioPidChannel);
                }
            }

            setState(BMediaPlayerState_eWaitingForStart);
            BDBG_MSG(("Player state is now BMediaPlayerState_eWaitingForStart (%s)", __FUNCTION__));
        }
        break;
    }

    case BMediaPlayerState_eWaitingForStart:
    {
        if (playerAction == BMediaPlayerAction_eStart)
        {
            BIP_PlayerStartSettings playerStartSettings;
            BIP_Player_GetDefaultStartSettings(&playerStartSettings);

            /* give bip player the simple decoder start settings if available.  this
               will allow bip player to then stop/start decoders if necessary */
            if ((NULL != _pAudioDecode) && (NULL != _pAudioDecode->getStartSettings()))
            {
                NEXUS_SimpleAudioDecoderStartSettings * pAudioStartSettings = _pAudioDecode->getStartSettings();

                /* transfer simple audio decoder start settings if available */
                playerStartSettings.simpleAudioStartSettings = *pAudioStartSettings;
            }
            if ((NULL != _pVideoDecode) && (NULL != _pVideoDecode->getStartSettings()))
            {
                NEXUS_SimpleVideoDecoderStartSettings * pVideoStartSettings = _pVideoDecode->getStartSettings();

                /* transfer simple video decoder start settings if available */
                playerStartSettings.simpleVideoStartSettings = *pVideoStartSettings;
            }

            bipStatus = BIP_Player_Start(_pPlayer, &playerStartSettings);
            if (bipStatus != BIP_SUCCESS)
            {
                BDBG_ERR(("BIP_Player_Start Failed: URL=%s", BIP_String_GetString(_pUrl)));
                goto error;
            }
            BDBG_MSG(("In Started state, Playing Media Stream. "));
            setState(BMediaPlayerState_eStarted);

            /*
             * ret = notifyObservers(eNotify_ChannelStateChanged, this);
             * CHECK_ERROR_GOTO("error notifying observers", ret, error);
             */
        }
        else
        {
            BDBG_ERR(("BIP ERROR, only channel bip should call start"));
        }
        /* Started now, prepare is blocking. */
        break;
    }

    case BMediaPlayerState_eStarted:
    {
        if (_pPlayer)
        {
            if (BMediaPlayerAction_eRestart != playerAction)
            {
                bipStatus = BIP_Player_GetStatus(_pPlayer, &playerStatus);
            }
            BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ("BIP_Player_GetStatus Failed"), error, bipStatus, bipStatus);

            BDBG_MSG(("Player is Started, wait for endOfStream/error Callback or user quit: position cur/last: %0.3f/%0.3f sec ", playerStatus.currentPositionInMs/1000., playerStatus.lastPositionInMs/1000.));

            switch (playerAction)
            {
            case BMediaPlayerAction_eRestart:
                bipStatus = restartDecode();
                break;
            case BMediaPlayerAction_ePause:
                bipStatus = BIP_Player_Pause(_pPlayer, NULL);
                break;
            case BMediaPlayerAction_ePlay:
                bipStatus = BIP_Player_Play(_pPlayer);
                break;
            case BMediaPlayerAction_eFf:
            case BMediaPlayerAction_eRew:
            case BMediaPlayerAction_ePlayAtRate:
                bipStatus = BIP_Player_PlayAtRateAsString(_pPlayer, MString(_trickModeRate), NULL);
                break;
            case BMediaPlayerAction_eSeek:
                bipStatus = BIP_Player_Seek(_pPlayer, _seekRate);
                break;
#if 0               /* waitting for this code from BIP */
            case BMediaPlayerAction_eFrameAdvance:
                bipStatus = BIP_Player_Pause(_pPlayer, NULL);
                BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ("BIP_Player_GetStatus Failed"), error, bipStatus, bipStatus);
                bipStatus = BIP_Player_PlayByFrame(_pPlayer, true);
                break;
            case BMediaPlayerAction_eFrameRewind:
                bipStatus = BIP_Player_Pause(_pPlayer, NULL);
                BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ("BIP_Player_GetStatus Failed"), error, bipStatus, bipStatus);
                bipStatus = BIP_Player_PlayByFrame(_pPlayer, false);
                break;
#endif /* if 0 */
            default:
                BDBG_ERR(("Unhandled playerAction , not supported %d", playerAction));
                bipStatus = BIP_SUCCESS;
                break;
            }   /* switch */

            ret = notifyObservers(eNotify_ChannelStateChanged, this);
            CHECK_ERROR_GOTO("error notifying observers", ret, error);
        }
        else
        {
            bipStatus = BIP_ERR_INTERNAL; /* need error case */
        }
    }
    break;
    default:
    {
        BDBG_ERR(("NOT SUPPORTED YET!"));
        break;
    }
    } /*
       * switch (playerState)
       * non-error path, we return the current status.
       */
    return(bipStatus);

error:
    _tuned = false;
    setState(BMediaPlayerState_eDisconnected);
    close();
    return(bipStatus);
} /* processMediaPlayerState */

CChannelBip::CChannelBip(
        const char *     strName,
        eBoardResource   type,
        CConfiguration * pCfg
        ) :
    CChannel(strName, type, pCfg),
    _pInterfaceName(NULL),
    _pUrl(NULL),
    _playerCallbackAction(BMediaPlayerAction_eMax),
    _pPlayer(NULL),
    _playerState(BMediaPlayerState_eDisconnected),
    _asyncApiCompletionStatus(BIP_SUCCESS),
    _enableDynamicTrackSelection(true),
    _pMediaInfo(NULL),
    _programNumberValid(false),
    _programNumber(0),
    _seekRate(0),
    _pVideoDecode(NULL),
    _pAudioDecode(NULL)
{
    BIP_Status bipStatus = BIP_SUCCESS;

    memset(&_asyncCallbackDesc, 0, sizeof(_asyncCallbackDesc));

    setTunerRequired(false);
    _pInterfaceName = BIP_String_Create();
    CHECK_PTR_ERROR(("BIP_String_Create() Failed"), (_pInterfaceName), bipStatus, BIP_ERR_OUT_OF_SYSTEM_MEMORY);

    _pUrl = BIP_String_Create();
    CHECK_PTR_ERROR(("BIP_String_Create() Failed"), (_pUrl), bipStatus, BIP_ERR_OUT_OF_SYSTEM_MEMORY);
    memset(&_playerStreamInfo, 0, sizeof(_playerStreamInfo));
}

CChannelBip::CChannelBip(void) :
    CChannel("CChannelBip", eBoardResource_ip, NULL),
    _pInterfaceName(NULL),
    _pUrl(NULL),
    _playerCallbackAction(BMediaPlayerAction_eMax),
    _pPlayer(NULL),
    _playerState(BMediaPlayerState_eDisconnected),
    _asyncApiCompletionStatus(BIP_SUCCESS),
    _pMediaInfo(NULL),
    _enableDynamicTrackSelection(true),
    _programNumberValid(false),
    _programNumber(0),
    _seekRate(0),
    _pVideoDecode(NULL),
    _pAudioDecode(NULL)
{
    BIP_Status bipStatus = BIP_SUCCESS;

    memset(&_asyncCallbackDesc, 0, sizeof(_asyncCallbackDesc));

    setTunerRequired(false);
    _pInterfaceName = BIP_String_Create();
    CHECK_PTR_ERROR(("BIP_String_Create() Failed"), (_pInterfaceName), bipStatus, BIP_ERR_OUT_OF_SYSTEM_MEMORY);

    _pUrl = BIP_String_Create();
    CHECK_PTR_ERROR(("BIP_String_Create() Failed"), (_pUrl), bipStatus, BIP_ERR_OUT_OF_SYSTEM_MEMORY);
    memset(&_playerStreamInfo, 0, sizeof(_playerStreamInfo));
    updateDescription();
}

CChannelBip::CChannelBip(CConfiguration * pCfg) :
    CChannel("CChannelBip", eBoardResource_ip, pCfg),
    _pInterfaceName(NULL),
    _pUrl(NULL),
    _playerCallbackAction(BMediaPlayerAction_eMax),
    _pPlayer(NULL),
    _playerState(BMediaPlayerState_eDisconnected),
    _asyncApiCompletionStatus(BIP_SUCCESS),
    _enableDynamicTrackSelection(true),
    _pMediaInfo(NULL),
    _programNumberValid(false),
    _programNumber(0),
    _seekRate(0),
    _pVideoDecode(NULL),
    _pAudioDecode(NULL)
{
    BIP_Status bipStatus = BIP_SUCCESS;

    memset(&_asyncCallbackDesc, 0, sizeof(_asyncCallbackDesc));

    setTunerRequired(false);
    _pInterfaceName = BIP_String_Create();
    CHECK_PTR_ERROR(("BIP_String_Create() Failed"), (_pInterfaceName), bipStatus, BIP_ERR_OUT_OF_SYSTEM_MEMORY);

    _pUrl = BIP_String_Create();
    CHECK_PTR_ERROR(("BIP_String_Create() Failed"), (_pUrl), bipStatus, BIP_ERR_OUT_OF_SYSTEM_MEMORY);
    memset(&_playerStreamInfo, 0, sizeof(_playerStreamInfo));
    updateDescription();
}

CChannelBip::CChannelBip(const CChannelBip & bipCh) :
    CChannel(bipCh),
    _pInterfaceName(NULL),
    _pUrl(NULL),
    _url(bipCh._url),
    _playerCallbackAction(BMediaPlayerAction_eMax),
    _pPlayer(NULL),
    _playerState(BMediaPlayerState_eDisconnected),
    _asyncApiCompletionStatus(BIP_SUCCESS),
    _enableDynamicTrackSelection(bipCh._enableDynamicTrackSelection),
    _pMediaInfo(NULL),
    _playerStreamInfo(bipCh._playerStreamInfo),
    _programNumberValid(false),
    _programNumber(0),
    _seekRate(0),
    _pVideoDecode(NULL),
    _pAudioDecode(NULL)
{
    BIP_Status bipStatus = BIP_SUCCESS;

    memset(&_asyncCallbackDesc, 0, sizeof(_asyncCallbackDesc));

    setTunerRequired(false);
    _pInterfaceName = BIP_String_Create();
    CHECK_PTR_ERROR(("BIP_String_Create() Failed"), (_pInterfaceName), bipStatus, BIP_ERR_OUT_OF_SYSTEM_MEMORY);
    bipStatus = BIP_String_StrcpyBipString(_pInterfaceName, bipCh._pInterfaceName);
    if (bipStatus != BIP_SUCCESS)
    {
        BDBG_ERR(("Cannot Create _pInterfaceName"));
    }

    _pUrl = BIP_String_Create();
    CHECK_PTR_ERROR(("BIP_String_Create() Failed"), (_pUrl), bipStatus, BIP_ERR_OUT_OF_SYSTEM_MEMORY);
    bipStatus = BIP_String_StrcpyBipString(_pUrl, bipCh._pUrl);
    if (bipStatus != BIP_SUCCESS)
    {
        BDBG_ERR(("Cannot Create _pUrl"));
    }
    setDurationInMsecs(bipCh._playerStreamInfo.durationInMs);
    memset(&_playerStreamInfo, 0, sizeof(_playerStreamInfo));
    memcpy(&_playerStreamInfo, &bipCh._playerStreamInfo, sizeof(_playerStreamInfo));
    updateDescription();
}

CChannel * CChannelBip::createCopy(CChannel * pChannel)
{
    CChannelBip * pChNew = NULL;

    pChNew = new CChannelBip(*(CChannelBip *)pChannel);
    return(pChNew);
}

void CChannelBip::updateDescription()
{
    CChannel::updateDescription();
    MUrl mUrl(_url.s());

    _strDescription = boardResourceToString(getType());

    _strDescription += mUrl.protocol();

    _strDescriptionLong  = _url;
    _strDescriptionShort = getUrlPath();
    if (_strDescriptionShort == "/channel")
    {
        /* add actual channel number if ip channel represents a live
         * channel on server */
        _strDescriptionShort += " ";
        _strDescriptionShort += getUrlQuery();
    }

    addMetadata("Description", _strDescription);
    addMetadata("Host", mUrl.server());
    addMetadata("Port", MString(mUrl.port()).s());
    addMetadata("Url Path", getUrlPath());
    addMetadata("Url Query", getUrlQuery());
    addMetadata("Security", "None");
    addMetadata("CA Cert", "None");
    addMetadata("AKE Port", "None");
    addMetadata("Key Format", "None");
    addMetadata("CKC Check", "False");
} /* updateDescription */

eRet CChannelBip::readXML(MXmlElement * xmlElemChannel)
{
    eRet    ret = eRet_Ok;
    MString strMode;

    CChannel::readXML(xmlElemChannel);

    strMode = xmlElemChannel->attrValue(XML_ATT_URL);
    if (!strMode.isEmpty())
    {
        BDBG_MSG((" URL is %s and new url is %s", _url.s(), strMode.s()));
        _url = strMode;
        BIP_String_StrcpyChar(_pUrl, _url.s());
    }

    strMode = xmlElemChannel->attrValue(XML_ATT_TRANSPORT);
    if (strMode.isEmpty())
    {
        BDBG_MSG(("No Transport set, default to TS"));
        setTransportType(NEXUS_TransportType_eTs);
        _pidMgr.setTransportType(NEXUS_TransportType_eTs);
    }
    else
    {
        setTransportType(stringToTransportType(strMode));
        _pidMgr.setTransportType(stringToTransportType(strMode));
    }

    strMode = xmlElemChannel->attrValue(XML_ATT_INTERFACE);
    if (!strMode.isEmpty())
    {
        BDBG_MSG(("Interface name is %s", strMode.s()));
        BIP_String_StrcpyChar(_pInterfaceName, strMode.s());
    }

    {
        MXmlElement * xmlElemStream = xmlElemChannel->findChild(XML_TAG_STREAM);
        if (NULL != xmlElemStream)
        {
            _pidMgr.readXML(xmlElemStream);
        }

        if (false == _pidMgr.isEmpty())
        {
            /* set dynamic flag so we do not query BIP for PIDS */
            _enableDynamicTrackSelection = false;
        }
    }

    updateDescription();
    return(ret);
} /* readXML */

void CChannelBip::writeXML(MXmlElement * xmlElemChannel)
{
    BDBG_MSG((" Write Live channel URL Entry"));
    CChannel::writeXML(xmlElemChannel);

    /* Only two values needed for now */
    xmlElemChannel->addAttr(XML_ATT_TYPE, "ip");
    xmlElemChannel->addAttr(XML_ATT_URL, _url.s());
} /* writeXML */

CChannelBip::~CChannelBip()
{
    close();
    if (_pInterfaceName)
    {
        BIP_String_Destroy(_pInterfaceName);
    }
    if (_pUrl)
    {
        BIP_String_Destroy(_pUrl);
    }
}

void CChannelBip::close()
{
    _tuned = false;

    if (NULL != _pWidgetEngine)
    {
        _pWidgetEngine->removeCallback(this, CALLBACK_TUNER_LOCK_STATUS_BIP);
    }

    if (_pPlayer)
    {
        BDBG_MSG((" BIP Player is valid, close"));
        BIP_Player_Stop(_pPlayer);
        BIP_Player_Disconnect(_pPlayer);
        BIP_Player_Destroy(_pPlayer);
        _pPlayer = NULL;
    }

    _pAudioDecode = NULL;
    _pVideoDecode = NULL;
} /* close */

eRet CChannelBip::setAudioProgram(uint16_t pid)
{
    BIP_PlayerSettings playerSettings;
    CPid *             pPid   = NULL;
    BIP_Status         status = BIP_SUCCESS;
    eRet               ret    = eRet_Ok;

    pPid = _pidMgr.findPid(pid, ePidType_Audio);
    CHECK_PTR_ERROR_GOTO("unable to find given audio pid", pPid, ret, eRet_NotAvailable, error);

    if (_pPlayer)
    {
         /* send this info to the player because we are in Dynamic mode now. Pids are set! */
        BIP_Player_GetSettings(_pPlayer, &playerSettings);
        playerSettings.enableDynamicTrackSelection                                = false;
        playerSettings.audioTrackId                                               = pPid->getPid();
        playerSettings.audioTrackSettings.pidSettings.pidTypeSettings.audio.codec = pPid->getAudioCodec();

        status = BIP_Player_SetSettings(_pPlayer, &playerSettings);
        CHECK_BIP_ERROR_GOTO("setAudioProgram failure", ret, status, error);

        /* BIP is going to change the nexus audio pid so we will update our copy of
           it here - we will need it in case we need to stop/start decode */
        {
            BIP_Status       retBip = BIP_SUCCESS;
            BIP_PlayerStatus statusPlayer;

            CHECK_PTR_ERROR_GOTO("audio decode not set in CChannelBip", _pAudioDecode, ret, eRet_NotAvailable, error);

            retBip = BIP_Player_GetStatus(_pPlayer, &statusPlayer);
            CHECK_BIP_ERROR_GOTO("unable to retrieve newly changed audio pid channel", ret, retBip, error);

            pPid->setPidChannel(statusPlayer.hAudioPidChannel);
            _pAudioDecode->setPid(pPid);
        }
    }

    goto done;
error:
    BDBG_ASSERT(false);
done:
    return(ret);
} /* setAudioProgram() */

void CChannelBip::gotoBackGroundRecord(void)
{
    BIP_PlayerSettings playerSettings;
    BIP_Status         status = BIP_SUCCESS;

    if (_pPlayer)
    {
        BIP_Player_GetSettings(_pPlayer, &playerSettings);
        playerSettings.audioTrackSettings.pidTypeSettings.audio.simpleDecoder = NULL;
        playerSettings.videoTrackSettings.pidTypeSettings.video.simpleDecoder = NULL;
        playerSettings.playbackSettings.simpleStcChannel                      = NULL;
        status = BIP_Player_SetSettings(_pPlayer, &playerSettings);
        if (status != BIP_SUCCESS)
        {
            BDBG_ERR((" GotoBackGroundRecord has Failed"));
            goto error;
        }

        _pVideoDecode = NULL;
        _pAudioDecode = NULL;
        _pStc         = NULL;
    }
error:
    BDBG_WRN((" Out of Function Ending"));
} /* gotoBackGroundRecord */

/* get PSI channel info for current tuned channel */
eRet CChannelBip::getChannelInfo(
        CHANNEL_INFO_T * pChanInfo,
        bool             bScanning
        )
{
    CPid *                      pid                      = NULL;
    eRet                        ret                      = eRet_Ok;
    int                         i                        = 0;
    bool                        trackGroupPresent        = false;
    unsigned                    videoTrackId             = 0;
    const BIP_MediaInfoStream * pMediaInfoStream         = NULL;
    BIP_MediaInfoTrackGroup *   pMediaInfoTrackGroup     = NULL;
    BIP_MediaInfoTrack *        pFirstTrackInfoForStream = NULL;
    int                         num_video_pids           = 0;
    int                         num_audio_pids           = 0;
    BIP_MediaInfoTrack *        pMediaInfoTrack;
    MString                     prgNum;

    BDBG_ASSERT(NULL != pChanInfo);
    BSTD_UNUSED(bScanning);

    if (false == _enableDynamicTrackSelection)
    {
        BDBG_MSG(("No need to getChannelInfo. pidManager contains all pids needed"));
        BDBG_ASSERT(false == _pidMgr.isEmpty());
        goto error;
    }

    /* Get the Stream object associated w/ this MediaInfo */
    pMediaInfoStream = BIP_MediaInfo_GetStream(_pMediaInfo);
    CHECK_PTR_ERROR_GOTO("pMediaInfoStream is NULL", pMediaInfoStream, ret, eRet_ExternalError, error);

    BDBG_MSG(("streamInfo: transportType=%s contentLength=%s", BIP_ToStr_NEXUS_TransportType(pMediaInfoStream->transportType), MString(pMediaInfoStream->contentLength).s()));

    /* Check the tracks*/
    if (pMediaInfoStream->numberOfTrackGroups > 1)
    {
        pMediaInfoTrackGroup         = pMediaInfoStream->pFirstTrackGroupInfo;
        pMediaInfoTrack              = pMediaInfoTrackGroup->pFirstTrackForTrackGroup;
        trackGroupPresent            = true;
        pChanInfo->num_programs      = pMediaInfoStream->numberOfTrackGroups;
        _enableDynamicTrackSelection = false;
        BDBG_MSG(("pMediaInfoStream->numberOfTrackGroups = %d", pMediaInfoStream->numberOfTrackGroups));

        /* Code to check for Program Number. This only takes effect if there are multiple programs in a Stream
         * If <stream> pids are set then program number will be ignored */
        prgNum = getUrlProgram();
        if (prgNum && !prgNum.isEmpty() && (prgNum.toInt() <= pChanInfo->num_programs))
        {
            pChanInfo->num_programs = 1; /* there is only 1 program now that we will use  */
            for (i = 1; i < prgNum.toInt(); i++)
            {
                pMediaInfoTrackGroup = pMediaInfoTrackGroup->pNextTrackGroup;
            }
            /*reset i*/
            i               = 0;
            pMediaInfoTrack = pMediaInfoTrackGroup->pFirstTrackForTrackGroup;
        }

        /* Continue normally if program is not working correctly. */
    }
    else
    {
        /* None of the track belongs to any trackGroup, in this case stream out all tracks from mediaInfoStream.
         * In this case we will always query the stream to get pid information. Pids are not saved since they can
         * dynamically chnage from IP source */
        BDBG_MSG((" Only one program"));
        pChanInfo->num_programs = 1;
        pMediaInfoTrack         = pMediaInfoStream->pFirstTrackInfoForStream;
    }

    /* check the MediaInfoTrack */
    CHECK_PTR_ERROR_GOTO("pMediaInfotrack is NULL", pMediaInfoTrack, ret, eRet_ExternalError, error);

    while (i < pChanInfo->num_programs)
    {
        num_video_pids                            = 0;
        num_audio_pids                            = 0;
        pChanInfo->program_info[i].program_number = i+1; /*pMediaInfoTrackGroup->trackGroupId;*/
        while (pMediaInfoTrack)
        {
            BDBG_MSG(("Program is %d , Found trackType=%s with trackId=%d", i, BIP_ToStr_BIP_MediaInfoTrackType(pMediaInfoTrack->trackType), pMediaInfoTrack->trackId));

            switch (pMediaInfoTrack->trackType)
            {
            case BIP_MediaInfoTrackType_eVideo:
                pChanInfo->program_info[i].video_pids[num_video_pids].pid        = pMediaInfoTrack->trackId;
                pChanInfo->program_info[i].video_pids[num_video_pids].streamType = pMediaInfoTrack->info.video.codec;
#ifndef MPOD_SUPPORT
                pChanInfo->program_info[i].maxHeight[num_video_pids] = pMediaInfoTrack->info.video.height;
                pChanInfo->program_info[i].maxWidth[num_video_pids]  = pMediaInfoTrack->info.video.width;
#endif
                num_video_pids++;
                pChanInfo->program_info[i].num_video_pids = num_video_pids;
                BDBG_MSG((" Program pid found %d, VIDEO Pid #", pMediaInfoTrack->trackId, num_video_pids+1));
                break;

            case BIP_MediaInfoTrackType_eAudio:
                pChanInfo->program_info[i].audio_pids[num_audio_pids].pid        = pMediaInfoTrack->trackId;
                pChanInfo->program_info[i].audio_pids[num_audio_pids].streamType = pMediaInfoTrack->info.audio.codec;
                num_audio_pids++;
                pChanInfo->program_info[i].num_audio_pids = num_audio_pids;
                BDBG_MSG(("Audio Program pid found %d, Audio Pid #", pMediaInfoTrack->trackId, num_audio_pids+1));
                break;
            case BIP_MediaInfoTrackType_ePcr:
                pChanInfo->program_info[i].pcr_pid = pMediaInfoTrack->trackId;
                BDBG_MSG(("PCR Program pid found %d", pMediaInfoTrack->trackId));
                break;
            default:
                BDBG_ERR(("Not Supported "));
                break;
            } /* switch */

            if (trackGroupPresent == true)
            {
                BDBG_MSG((" TrackGroup Present grab next track %d", i));
                pMediaInfoTrack = pMediaInfoTrack->pNextTrackForTrackGroup;
            }
            else
            {
                pMediaInfoTrack = pMediaInfoTrack->pNextTrackForStream;
            }
        }

        if (prgNum && !prgNum.isEmpty())
        {
            BDBG_MSG(("There was a program number used in the URL string, exit now"));
            break;
        }

        /* after we leave. grab next Track group pointer */
        if (true == trackGroupPresent)
        {
            pMediaInfoTrackGroup = pMediaInfoTrackGroup->pNextTrackGroup;
            if (pMediaInfoTrackGroup)
            {
                i++; /* increase pointer for the program info */
                pMediaInfoTrack = pMediaInfoTrackGroup->pFirstTrackForTrackGroup;
            }
            else
            {
                BDBG_MSG(("No More Groups to query. Pid extraction has been completed"));
                break;
            }
        }
        else
        {
            BDBG_MSG(("Only one track"));
            break;
        }
    }

    /* Save StreamInfo to be copied */
    BIP_Player_GetProbedStreamInfo(_pPlayer, &_playerStreamInfo);
    setDurationInMsecs(_playerStreamInfo.durationInMs);

error:
    return(ret);
} /* getChannelInfo */

/* Must create Pids. pidMgr Function only does traditional TS PMT parsing */
eRet CChannelBip::initialize(PROGRAM_INFO_T * pProgramInfo)
{
    eRet   ret = eRet_Ok;
    CPid * pid = NULL;
    int    i   = 0;

    BDBG_ASSERT(NULL != pProgramInfo);

    setProgramNum(pProgramInfo->program_number);
    _pidMgr.clearPids();

    /* check PCR pid with first Video Pid allocate PCR pid first
     * Logic in use Cases will remove PCR pid if it is the same
     * as an Audio Pid or other Video Pid */
    if (pProgramInfo->pcr_pid != 0)
    {
        pid = new CPid(pProgramInfo->pcr_pid, ePidType_Pcr);
        _pidMgr.setPcrPid(pid);
        BDBG_MSG((" Creating PCR PID %d", pProgramInfo->pcr_pid));
    }

    BDBG_MSG((" Num of Video Pids #%d", pProgramInfo->num_video_pids));

    for (i = 0; i < pProgramInfo->num_video_pids; i++)
    {
        BDBG_MSG((" Program pid found %d, VIDEO Pid #%d", pProgramInfo->video_pids[i].pid, i));
        pid = new CPid(pProgramInfo->video_pids[i].pid, ePidType_Video);
        pid->setVideoCodec((NEXUS_VideoCodec)pProgramInfo->video_pids[i].streamType);
        _pidMgr.addPid(pid);

        /* double check to make sure no PCR pids were stored that match the video pid */
        CPid * pcrPid;
        pcrPid = _pidMgr.getPid(0, ePidType_Pcr);

        /* possibly move to PidMgr:  */
        if (pcrPid && (pProgramInfo->pcr_pid == pProgramInfo->video_pids[i].pid))
        {
            BDBG_MSG(("found pcr pid already stored in pidMgr that matches video pid, remove it"));
            _pidMgr.removePid(pcrPid);
            delete pcrPid;
            pid->setPcrType(true);
            _pidMgr.setPcrPid(pid);
        }
    }

    pid = NULL;
    BDBG_MSG((" Num of Audio Pids #%d", pProgramInfo->num_audio_pids));
    for (i = 0; i < pProgramInfo->num_audio_pids; i++)
    {
        BDBG_MSG(("Audio Program pid found %d, Audio Pid #%d ", pProgramInfo->audio_pids[i].pid, i));
        pid = new CPid(pProgramInfo->audio_pids[i].pid, ePidType_Audio);
        pid->setAudioCodec((NEXUS_AudioCodec)pProgramInfo->audio_pids[i].streamType);
        _pidMgr.addPid(pid);

        /* double check to make sure no PCR pids were stored that match the video pid */
        CPid * pcrPid;
        pcrPid = _pidMgr.getPid(0, ePidType_Pcr);

        /* possibly move to PidMgr:  */
        if (pcrPid && (pProgramInfo->pcr_pid == pProgramInfo->audio_pids[i].pid))
        {
            BDBG_MSG(("found pcr pid already stored in pidMgr that matches audio pid, remove it"));
            _pidMgr.removePid(pcrPid);
            delete pcrPid;
            pid->setPcrType(true);
            _pidMgr.setPcrPid(pid);
        }
    }
#ifndef MPOD_SUPPORT
    setWidth(pProgramInfo->maxWidth[0]);
    setHeight(pProgramInfo->maxHeight[0]);
#endif
    /* set this flag to indicate we are not using the BIP mediaInfo extraction
       the channel has the PIDS and there is no need to re-probe */
    _enableDynamicTrackSelection = false;

error:
    return(ret);
} /* initialize */

eRet CChannelBip::initialize(void)
{
    eRet ret = eRet_Ok;

    _pPlayer = BIP_Player_Create(NULL);
    CHECK_PTR_ERROR_GOTO("BIP_Player_Create() Failed", _pPlayer, ret, eRet_ExternalError, error);

    setState(BMediaPlayerState_eDisconnected);

    /* Setup callbacks */
    _asyncCallbackDesc.context  = this;
    _asyncCallbackDesc.callback = asyncCallbackFromBIP;
    _asyncCallbackDesc.param    = 0;

    BDBG_MSG((" INIT CHANNEL BIP"));

    _pidMgr.dump();
    updateDescription();
error:
    return(ret);
} /* initialize */

eRet CChannelBip::tune(
        void *    id,
        CConfig * pConfig,
        bool      bWaitForLock,
        uint16_t  index
        )
{
    eRet       ret       = eRet_Ok;
    BIP_Status bipStatus = BIP_SUCCESS;

    BSTD_UNUSED(bWaitForLock);
    BSTD_UNUSED(id);
    BSTD_UNUSED(index);
    BSTD_UNUSED(pConfig);

    if ((_tuned == true) && (getState() == BMediaPlayerState_eStarted))
    {
        BDBG_WRN(("Already Tuned and BIP Player started, Might be background record!!"));
        return(ret);
    }

    ret = initialize();
    if (ret)
    {
        BDBG_ERR((" Cannot Init"));
        goto error;
    }

    /* Get the Widget Engine Callback*/
    if (NULL != _pWidgetEngine)
    {
        _pWidgetEngine->addCallback(this, CALLBACK_TUNER_LOCK_STATUS_BIP, bwinTunerLockStatusCallback);
    }
    else
    {
        BDBG_ERR(("Async is only supported for Channel IP. Please check error"));
        ret = eRet_ExternalError;
        goto error;
    }

    do
    {
        /* Media State Machine will return BIP_SUCCESS if call was
         * completed. The State Machine has to be run again to process the
         * next step. */
        bipStatus = mediaStateMachine(BMediaPlayerAction_eMax);
    }
    while (bipStatus == BIP_SUCCESS);

    if (bipStatus != BIP_INF_IN_PROGRESS)
    {
        BDBG_ERR((" Error untune"));
        ret = eRet_ExternalError;
        goto error;
    }

    _tuned = true;
    goto done;

error:
    /* Remove the WidgetCallback */
    if (NULL != _pWidgetEngine)
    {
        _pWidgetEngine->removeCallback(this, CALLBACK_TUNER_LOCK_STATUS_BIP);
    }

done:
    return(ret);
} /* tune */

eRet CChannelBip::unTune(
        CConfig * pConfig,
        bool      bFullUnTune,
        bool      bCheckInIp
        )
{
    eRet               ret          = eRet_Ok;
    BIP_Status         bipStatus    = BIP_SUCCESS;
    BMediaPlayerAction playerAction = BMediaPlayerAction_eUnTune;

    BSTD_UNUSED(pConfig);
    BSTD_UNUSED(bFullUnTune);
    BSTD_UNUSED(bCheckInIp);

    BDBG_MSG(("UNTUNE"));

    /* Remove the WidgetCallback */
    if (NULL != _pWidgetEngine)
    {
        _pWidgetEngine->removeCallback(this, CALLBACK_TUNER_LOCK_STATUS_BIP);
    }

    bipStatus = mediaStateMachine(playerAction);
    if (bipStatus != BIP_SUCCESS)
    {
        ret = eRet_ExternalError;
    }

    if (false == _pidMgr.isImmutable())
    {
        _pidMgr.clearPids();
        _enableDynamicTrackSelection = true;
    }
    _tuned = false;
    return(ret);
} /* unTune */

bool CChannelBip::operator ==(CChannel &other)
{
    CChannelBip * pOtherHttp = (CChannelBip *)&other;

    BSTD_UNUSED(pOtherHttp);
    /* check base class equivalency first */
    if (true == CChannel::operator ==(other))
    {
        /* FILL IN */
    }

    return(false);
}

/* IP Version */
eRet CChannelBip::getPsiInfo(void)
{
    return(eRet_Ok);
}

/* BIP Version handles PID internally - Funciton to Prepare*/
eRet CChannelBip::openPids(
        CSimpleAudioDecode * pAudioDecode,
        CSimpleVideoDecode * pVideoDecode
        )
{
    eRet               ret       = eRet_Ok;
    BIP_Status         bipStatus = BIP_SUCCESS;
    BMediaPlayerAction playerAction;

    /* pass in action  */
    playerAction = BMediaPlayerAction_ePrepare;
    if (pAudioDecode || pVideoDecode)
    {
        /* One handle has to be valid */
        _pVideoDecode = pVideoDecode;
        _pAudioDecode = pAudioDecode;
    }
    else
    {
        ret = eRet_ExternalError;
        goto error;
    }

    /* Check we might not be waiting to Prepare, because we re-entered */
    if (getState() == BMediaPlayerState_eStarted)
    {
        BDBG_MSG(("Player is Started, continue we may need to restart"));
        playerAction = BMediaPlayerAction_eRestart;
    }
    else
    if (getState() != BMediaPlayerState_eWaitingForPrepare)
    {
        BDBG_MSG(("_playerStatus is not ready to Prepare"));
        goto error;
    }

    bipStatus = mediaStateMachine(playerAction);
    if (bipStatus != BIP_SUCCESS)
    {
        ret = eRet_ExternalError;
        goto error;
    }

    return(ret);

error:
    return(ret);
} /* openPids */

eRet CChannelBip::closePids()
{
    /* BIP handles pids internally.  cchannelbip will have a copy of the pids/handles
       but should not create or free them */
    return(eRet_Ok);
}

eRet CChannelBip::start(
        CSimpleAudioDecode * pAudioDecode,
        CSimpleVideoDecode * pVideoDecode
        )
{
    eRet               ret          = eRet_Ok;
    BIP_Status         bipStatus    = BIP_SUCCESS;
    BMediaPlayerAction playerAction = BMediaPlayerAction_eStart;

    if (NULL != pVideoDecode) { _pVideoDecode = pVideoDecode; }
    if (NULL != pAudioDecode) { _pAudioDecode = pAudioDecode; }

    if (getState() == BMediaPlayerState_eStarted)
    {
        playerAction = BMediaPlayerAction_eRestart;
    }
    bipStatus = mediaStateMachine(playerAction);
    if (bipStatus != BIP_SUCCESS)
    {
        BDBG_ERR((" Failed to start "));
        ret = eRet_ExternalError;
        goto error;
    }

    setState(BMediaPlayerState_eStarted);

    BDBG_MSG(("Player Start API started..bipSTtatus %d", bipStatus));

    return(ret);

error:
    return(ret);
} /* start */

eRet CChannelBip::play(void)
{
    eRet       ret = eRet_Ok;
    BIP_Status bipStatus;

    if (_pPlayer)
    {
        bipStatus = mediaStateMachine(BMediaPlayerAction_ePlay);
        if (bipStatus != BIP_SUCCESS)
        {
            BDBG_ERR(("BIP_Player_Play() Failed!"));
            ret = eRet_ExternalError;
        }
        else
        {
            BDBG_ERR(("BIP_Player_Play() Success!"));
            ret = eRet_Ok;
        }
    }

    return(ret);
} /* play */

eRet CChannelBip::pause(void)
{
    eRet       ret       = eRet_Ok;
    BIP_Status bipStatus = BIP_SUCCESS;

    if (_pPlayer)
    {
        bipStatus = mediaStateMachine(BMediaPlayerAction_ePause);
        if (bipStatus != BIP_SUCCESS)
        {
            BDBG_WRN(("Calling Media State Machine status is still %d", bipStatus));
        }
    }
    return(ret);
} /* pause */

eRet CChannelBip::frameAdvance(void)
{
    eRet               ret          = eRet_Ok;
    BIP_Status         bipStatus    = BIP_SUCCESS;
    BMediaPlayerAction playerAction = BMediaPlayerAction_eFrameAdvance;

    bipStatus = mediaStateMachine(playerAction);
    if (bipStatus != BIP_SUCCESS)
    {
        BDBG_ERR((" Failed to start "));
        ret = eRet_ExternalError;
        goto error;
    }

    return(ret);

error:
    return(ret);
} /* frameAdvance */

eRet CChannelBip::frameRewind(void)
{
    eRet               ret          = eRet_Ok;
    BIP_Status         bipStatus    = BIP_SUCCESS;
    BMediaPlayerAction playerAction = BMediaPlayerAction_eFrameRewind;

    bipStatus = mediaStateMachine(playerAction);
    if (bipStatus != BIP_SUCCESS)
    {
        BDBG_ERR((" Failed to start "));
        ret = eRet_ExternalError;
        goto error;
    }

    return(ret);

error:
    return(ret);
} /* frameRewind */

eRet CChannelBip::playAtRate(void)
{
    eRet               ret          = eRet_Ok;
    BIP_Status         bipStatus    = BIP_SUCCESS;
    BMediaPlayerAction playerAction = BMediaPlayerAction_ePlayAtRate;

    bipStatus = mediaStateMachine(playerAction);
    if (bipStatus != BIP_SUCCESS)
    {
        BDBG_ERR((" Failed to play at Rate %d ", _trickModeRate));
        ret = eRet_ExternalError;
        goto error;
    }

    return(ret);

error:
    return(ret);
} /* PlayAtRate */

/****
 *  The class variable _trickModeRate is set in the CChannel class because all classes should set it the same way.
 *  Each derived class should implement the trick mode appropriately. The derived class should not change the
 *  _trickModeRate.
 ****/
eRet CChannelBip::applyTrickMode(void)
{
    eRet       ret       = eRet_Ok;
    BIP_Status bipStatus = BIP_SUCCESS;

    if (_pPlayer)
    {
        bipStatus = mediaStateMachine((0 <= _trickModeRate) ? BMediaPlayerAction_eFf : BMediaPlayerAction_eRew);
        if (bipStatus != BIP_SUCCESS)
        {
            BDBG_WRN(("Calling Media State Machine status is still %d", bipStatus));
        }
    }

    return(ret);
} /* applyTrickMode */

eRet CChannelBip::setTrickModeRate(int trickModeRate)
{
    eRet ret = eRet_InvalidParameter;

    if ((GET_INT(_pCfg, TRICK_MODE_RATE_MAX) >= trickModeRate) &&
        (GET_INT(_pCfg, TRICK_MODE_RATE_MIN) <= trickModeRate))
    {
        _trickModeRate = trickModeRate;
        ret            = eRet_Ok;
    }

    if (1 == _trickModeRate)
    {
        _trickModeState = eChannelTrick_Play;
    }
    else
    if (0 == _trickModeRate)
    {
        _trickModeState = eChannelTrick_Pause;
    }
    else
    if (0 > _trickModeRate)
    {
        _trickModeState = eChannelTrick_Rewind;
    }
    else
    {
        _trickModeState = eChannelTrick_FastForward;
    }

    return(ret);
} /* setTrickModeRate */

eRet CChannelBip::setTrickMode(bool fastFoward)
{
    eRet ret = eRet_Ok;

    if (fastFoward)
    {
        if (_trickModeRate == -1)
        {
            /* transition from Rew to FF */
            setTrickModeRate(1);
        }
        else
        if (_trickModeRate < 0)
        {
            /* currently rewinding so rewind less */
            setTrickModeRate(_trickModeRate + ABS(_trickModeRate / 2));
        }
        else
        {
            /* if rate is within the max fast forward rate */
            setTrickModeRate(_trickModeRate * 2);
        }

        if ((_trickModeRate == 0) || (_trickModeRate == 1))
        {
            setTrickModeRate(1);
        }
    }
    else
    {
        if (_trickModeRate == 1)
        {
            /* transition from FF to Rew */
            setTrickModeRate(-1);
        }
        else
        if (_trickModeRate > 0)
        {
            /* currently fast forwarding so fast forward less */
            setTrickModeRate(_trickModeRate - ABS(_trickModeRate / 2));
        }
        else
        {
            /* if rate is within the max fast rewind rate */
            setTrickModeRate(-1 * ABS(_trickModeRate * 2));
        }
    }

    BDBG_MSG(("CChannelBip::%s: Rate (%d); ", __FUNCTION__, _trickModeRate));

    return(ret);
} /* setTrickMode */

/****
 *
 *  Each derived class should implement the trick mode appropriately. This command will be from Lua Scriptengine
 *
 ****/
eRet CChannelBip::trickmode(CPlaybackTrickData * pTrickModeData)
{
    eRet ret = eRet_Ok;

    switch (pTrickModeData->_command)
    {
    case ePlaybackTrick_Play:
    case ePlaybackTrick_PlayNormal:
        setTrickModeRate(1);
        ret = play();
        CHECK_ERROR_GOTO("trick mode: play failed", ret, error);
        break;
    case ePlaybackTrick_FastForward:
        setTrickMode(true);
        ret = applyTrickMode();
        CHECK_ERROR_GOTO("trick mode: fastFoward failed", ret, error);
        break;
    case ePlaybackTrick_Rewind:
        setTrickMode(false);
        ret = applyTrickMode();
        CHECK_ERROR_GOTO("trick mode: rewind failed", ret, error);
        break;
    case ePlaybackTrick_FrameAdvance:
        ret = frameAdvance();
        CHECK_ERROR_GOTO("trick mode: FrameAdvance failed", ret, error);
        break;

    case ePlaybackTrick_FrameRewind:
        ret = frameRewind();
        CHECK_ERROR_GOTO("trick mode: FrameRewind failed", ret, error);
        break;

    case ePlaybackTrick_Pause:
        setTrickModeRate(0);
        ret = pause();
        CHECK_ERROR_GOTO("trick mode: pause failed", ret, error);
        break;

    case ePlaybackTrick_Rate:
        if (pTrickModeData->_rate > 0)
        {
            setTrickMode(true);
        }
        else
        {
            setTrickMode(false);
        }
        setTrickModeRate(pTrickModeData->_rate);
        ret = playAtRate();
        CHECK_ERROR_GOTO("trick mode: Play trickmode rate failed", ret, error);
        break;

    case ePlaybackTrick_Seek:
        setTrickModeRate(1);
        ret = seek(false, pTrickModeData->_seekPosition);
        CHECK_ERROR_GOTO("trick mode: seek failed", ret, error);
        break;

    case ePlaybackTrick_SeekRelative:
        setTrickModeRate(1);
        ret = seek(true, pTrickModeData->_seekPosition);
        CHECK_ERROR_GOTO("trick mode: seek failed", ret, error);
        break;

    default:
        BDBG_WRN(("Not supported"));
        ret = eRet_NotSupported;
        goto error;
    } /* switch */

    return(ret);

error:
    setTrickModeRate(1);
    ret = play();
    CHECK_ERROR("unable to start playback", ret);

    return(ret);
} /* trickMode */

eRet CChannelBip::stop(void)
{
    eRet ret = eRet_Ok;

    if (_pPlayer)
    {
        if (true == isStopAllowed())
        {
            ret = unTune(NULL);
        }
        else
        {
            BDBG_ERR((" Player cannot stop on Live IP Channel Decode. Tune Channel Away!!"));
        }
    }
    return(ret);
} /* stop */

eRet CChannelBip::seek(
        bool     relative,
        long int seekTime
        )
{
    eRet             ret       = eRet_Ok;
    BIP_Status       bipStatus = BIP_SUCCESS;
    BIP_PlayerStatus playerStatus;

    if (_pPlayer)
    {
        _seekRate = seekTime*1000;

        if (relative)
        {
            bipStatus = BIP_Player_GetStatus(_pPlayer, &playerStatus);
            BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ("BIP_Player_GetStatus Failed"), error, bipStatus, bipStatus);
            BDBG_MSG(("seekRate %d, current position:%lu", _seekRate, playerStatus.currentPositionInMs));
            _seekRate = playerStatus.currentPositionInMs + _seekRate;
        }

        bipStatus = mediaStateMachine(BMediaPlayerAction_eSeek);
        if (bipStatus != BIP_SUCCESS)
        {
            BDBG_WRN(("Calling Media State Machine status is still %d", bipStatus));
        }
    }

    _seekRate = 0;
error:
    return(ret);
} /* seek */

void CChannelBip::setUrl(const char * pString)
{
    _url = pString;
    BIP_String_StrcpyChar(_pUrl, _url.s());
    updateDescription();
}

MString CChannelBip::getUrlPath()
{
    MString strPath;
    MUrl    mUrl(_url.s());

    strPath = mUrl.path();
    return(strPath);
}

MString CChannelBip::getUrlQuery()
{
    MString strPath;
    MUrl    mUrl(_url.s());

    strPath = mUrl.search();
    return(strPath);
}

/* return program # if program is requested */
MString CChannelBip::getUrlProgram()
{
    MString tempString1 = _url;
    MString prgNum;
    int     index = -1;

    index = tempString1.find("program=", 0, true);

    if (index != -1)
    {
        prgNum = tempString1.mid(index + 8, (tempString1.length() - index)); /* index + 8 to go past the = char.*/
        if (false == prgNum.isEmpty())
        {
            _programNumber      = prgNum.toInt();
            _programNumberValid = true;
        }
    }

    return(prgNum);
} /* getUrlProgram */

void CChannelBip::setHost(const char * pString)
{
    /* replace host in _url then in bip _pUrl */

    if (_url.length() == 0)
    {
        BDBG_ERR((" Cannot replace HOST unless URL IS Valid"));
        return;
    }

    MUrl mUrl(_url.s());

    MString newUrl(mUrl.protocol() + MString("://") + MString(pString) + ":" + MString(mUrl.port()) + mUrl.query());

    /* HOST has been set! should not be done, pass in URL*/
    BDBG_ERR((" Don't replace HOST, please use setUrl() function"));
    _url = newUrl;

    BIP_String_StrcpyChar(_pUrl, _url.s());
    updateDescription();
} /* setHost */

MString CChannelBip::getHost()
{
    MString strPath;
    MUrl    mUrl(_url.s());

    strPath = mUrl.server();
    return(strPath);
}

void CChannelBip::setPort(uint16_t nPort)
{
    /* replace host in _url then in bip _pUrl */

    if (_url.length() == 0)
    {
        BDBG_ERR((" Cannot replace PORT unless URL IS Valid"));
        return;
    }

    MUrl mUrl(_url.s());

    MString newUrl(mUrl.protocol() + MString("://") + mUrl.server() + ":" + MString(nPort) + mUrl.query());

    /* HOST has been set! should not be done, pass in URL*/
    BDBG_ERR((" Don't replace PORT, please use setUrl() function"));
    _url = newUrl;

    BIP_String_StrcpyChar(_pUrl, _url.s());
    updateDescription();
} /* setPort */

uint16_t CChannelBip::getPort()
{
    MUrl mUrl(_url.s());

    return(mUrl.port());
}

void CChannelBip::dump()
{
    CChannel::dump();
    /* Fill in */
}

MString CChannelBip::getTimeString()
{
    MString  strTime;
    uint32_t uHours   = 0;
    uint32_t uMinutes = 0;
    uint32_t uSeconds = 0;

    uSeconds = _durationInMsecs / 1000;
    uHours   = uSeconds / 3600;
    uMinutes = (uSeconds % 3600) / 60;
    uSeconds = uSeconds % 60;

    /* add hours */
    strTime = (0 < uHours) ? MString(uHours) : "";

    /* add minutes */
    if (false == strTime.isEmpty())
    {
        strTime += ":";

        if (10 > uMinutes)
        {
            strTime += "0";
        }
    }
    strTime += (0 < uMinutes) ? MString(uMinutes) : "";

    /* add seconds */
    strTime += ":";
    if (10 > uSeconds)
    {
        strTime += "0";
    }
    strTime += MString(uSeconds);

    return(strTime);
} /* getTimeString */

#endif /* Playback IP SUPPORT */
