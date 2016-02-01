/******************************************************************************
 *    (c)2008-2015 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *  Example program to demonstrate receiving live or playback content over IP Channels (UDP/RTP/RTSP/HTTP based)
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 ******************************************************************************/
#define NXCLIENT_SUPPORT 1
#if NXCLIENT_SUPPORT
#include "nxclient.h"
#include "nexus_platform_client.h"
#else
#include "nexus_platform.h"
#endif

#include "bip.h"
#include "nexus_video_decoder_extra.h"
#include "nexus_video_window.h"
#include "bmedia_probe.h"
#if NEXUS_HAS_SYNC_CHANNEL
#include "nexus_sync_channel.h"
#endif
#include "cmd_parsing.h"

BDBG_MODULE(player);

static void playbackDoneCallbackFromBIP(
    void *context,
    int   param
    )
{
    AppCtx *pAppCtx = context;

    BSTD_UNUSED( param );
    BDBG_WRN(( BIP_MSG_PRE_FMT "Got endOfStreamer Callback from BIP: Settings hEvent=%p)" BIP_MSG_PRE_ARG, pAppCtx->hCompletionEvent ));
    pAppCtx->playbackDone = true;
    B_Event_Set(pAppCtx->hCompletionEvent);
}

static void asyncApiCompletionCallbackFromBip(
    void *context,
    int   param
    )
{
    B_EventHandle event = context;
    BSTD_UNUSED( param );

    B_Event_Set( event );
} /* asyncApiCompletionCallbackFromBip */

static BIP_Status processMediaPlayerState(
    AppCtx *pAppCtx,
    CmdOptions *pCmdOptions
    )
{
    BIP_Status          bipStatus = BIP_INF_IN_PROGRESS;
    NEXUS_VideoCodec    videoCodec = NEXUS_VideoCodec_eUnknown;

    switch (pAppCtx->playerState)
    {
    case BMediaPlayerState_eDisconnected:
        {
            BIP_PlayerConnectSettings settings;

            BDBG_MSG(("In Disconnected state, start Connect processing on URL=%s..", BIP_String_GetString(pAppCtx->hUrl) ));
            BIP_Player_GetDefaultConnectSettings(&settings);
            settings.pUserAgent = "BIP Player Example";
            pAppCtx->asyncApiCompletionStatus = BIP_INF_IN_PROGRESS;
            bipStatus = BIP_Player_ConnectAsync( pAppCtx->hPlayer, BIP_String_GetString(pAppCtx->hUrl), &settings, &pAppCtx->asyncCallbackDesc, &pAppCtx->asyncApiCompletionStatus );
            BIP_CHECK_GOTO(( bipStatus == BIP_INF_IN_PROGRESS ), ( "BIP_Player_ConnectAsync Failed!"), error, bipStatus, bipStatus );

            BDBG_MSG(("Connect processing started.."));
            pAppCtx->playerState = BMediaPlayerState_eWaitingForConnect;

            break;
        }

    case BMediaPlayerState_eWaitingForConnect:
        {
            /* Check if the BIP_Player_ConnectAsync is still in progress, completed successfully or w/ an error status. */
            BIP_CHECK_GOTO((pAppCtx->asyncApiCompletionStatus==BIP_INF_IN_PROGRESS || pAppCtx->asyncApiCompletionStatus == BIP_SUCCESS), ("BIP_Player_ConnectAsync() Failed!"), error, pAppCtx->asyncApiCompletionStatus, bipStatus);

            if (pAppCtx->asyncApiCompletionStatus == BIP_SUCCESS)
            {
                BIP_PlayerProbeMediaInfoSettings probeMediaInfoSettings;

                /* BIP_Player_ConnectAsync() is successful,parse any custom HTTP response headers. */

                BDBG_MSG(("In Connected state, start Probe processing.."));
                BIP_Player_GetDefaultProbeMediaInfoSettings( &probeMediaInfoSettings );
                pAppCtx->asyncApiCompletionStatus = BIP_INF_IN_PROGRESS;
                pAppCtx->hMediaInfo = NULL;
                bipStatus = BIP_Player_ProbeMediaInfoAsync(pAppCtx->hPlayer, &probeMediaInfoSettings, &pAppCtx->hMediaInfo, &pAppCtx->asyncCallbackDesc, &pAppCtx->asyncApiCompletionStatus);
                BIP_CHECK_GOTO(( bipStatus == BIP_INF_IN_PROGRESS ), ( "BIP_Player_ProbeAsync Failed!"), error, bipStatus, bipStatus );

                BDBG_MSG(("Media Probe processing started.."));
                pAppCtx->playerState = BMediaPlayerState_eWaitingForProbe;
            }
            /* else Connect is still in progress..., nothing to do yet.. */
            break;
        }

    case BMediaPlayerState_eWaitingForProbe:
        {
            /* Check if the BIP_Player_ProbeAsync is still in progress, completed successfully or w/ an error status. */
            BIP_CHECK_GOTO((pAppCtx->asyncApiCompletionStatus==BIP_INF_IN_PROGRESS || pAppCtx->asyncApiCompletionStatus == BIP_SUCCESS), ("BIP_Player_ProbeAsync() Failed!"), error, pAppCtx->asyncApiCompletionStatus, bipStatus);

            if (pAppCtx->asyncApiCompletionStatus == BIP_SUCCESS)
            {
                const BIP_MediaInfoStream *pMediaInfoStream;

                /* BIP_Player_ProbeMediaInfoAsync() is successful, so start the Prepare processing */

                /* Get the Stream object associated w/ this MediaInfo */
                pMediaInfoStream = BIP_MediaInfo_GetStream(pAppCtx->hMediaInfo);
                BIP_CHECK_GOTO(( pMediaInfoStream ), ( "BIP_MediaInfo_GetStream Failed!"), error, bipStatus, bipStatus );
                pAppCtx->streamDurationInMs = pMediaInfoStream->durationInMs;

                /* Map the MediaStream Info to the Player's StreamInfo. */
                BIP_Player_GetProbedStreamInfo(pAppCtx->hPlayer, &pAppCtx->playerStreamInfo);

                /* Now that we have the Media Information, App should select tracks to play and prepare the player for playing the stream. */
                /* Prepare the BIP Player using the default settings. */
                {
                    BIP_PlayerPrepareSettings   prepareSettings;
                    BIP_PlayerSettings          playerSettings;
                    BIP_MediaInfoTrack          mediaInfoTrack;

                    BIP_Player_GetDefaultPrepareSettings(&prepareSettings);
                    BIP_Player_GetDefaultSettings(&playerSettings);

                    /* Get first Video & Audio track entry from pMediaInfoStream .*/
                    if (pAppCtx->hMediaInfo)
                    {
                        if (playerGetTrackOfType(pAppCtx->hMediaInfo, BIP_MediaInfoTrackType_eVideo, &mediaInfoTrack) )
                        {
                            playerSettings.videoTrackId = mediaInfoTrack.trackId;
                            playerSettings.videoTrackSettings.pidTypeSettings.video.codec = mediaInfoTrack.info.video.codec;
                            pAppCtx->maxHeight = mediaInfoTrack.info.video.height;
                            pAppCtx->maxWidth = mediaInfoTrack.info.video.width;
                            videoCodec = mediaInfoTrack.info.video.codec;
                        }

                        if (playerGetTrackOfType(pAppCtx->hMediaInfo, BIP_MediaInfoTrackType_eAudio, &mediaInfoTrack) )
                        {
                            playerSettings.audioTrackId = mediaInfoTrack.trackId;
                            playerSettings.audioTrackSettings.pidSettings.pidTypeSettings.audio.codec = mediaInfoTrack.info.audio.codec;
                        }

                        /* Make sure Probe found atleast a Video or an Audio Track. */
                        BIP_CHECK_GOTO( (playerSettings.videoTrackId != UINT_MAX || playerSettings.audioTrackId != UINT_MAX), ( "Neither Audio nor Video Tracks found for URL=%s", BIP_String_GetString(pAppCtx->hUrl) ), error, bipStatus, bipStatus );
                    }

                    /* Once the probing is done ,allocate & acquire NxClient Resources. */
                    {
                        NxClient_AllocSettings      allocSettings;
                        NxClient_AllocResults       allocResults;
                        NxClient_ConnectSettings    connectSettings;
                        NEXUS_Error                 rc = NEXUS_UNKNOWN;

                        NxClient_GetDefaultAllocSettings(&allocSettings);
                        allocSettings.simpleVideoDecoder = 1;
                        allocSettings.simpleAudioDecoder = 1;
                        rc = NxClient_Alloc( &allocSettings, &allocResults );
                        BIP_CHECK_GOTO(( rc == NEXUS_SUCCESS ), ( "NxClient_Alloc Failed" ), error, BIP_INF_NEXUS_RESOURCE_NOT_AVAILABLE, bipStatus );

                        NxClient_GetDefaultConnectSettings( &connectSettings );
                        connectSettings.simpleVideoDecoder[0].id = allocResults.simpleVideoDecoder[0].id;
                        connectSettings.simpleVideoDecoder[0].decoderCapabilities.supportedCodecs[videoCodec] = true;
                        if(pAppCtx->maxWidth && pAppCtx->maxHeight)
                        {
                            connectSettings.simpleVideoDecoder[0].decoderCapabilities.maxWidth = pAppCtx->maxWidth;
                            connectSettings.simpleVideoDecoder[0].decoderCapabilities.maxHeight = pAppCtx->maxHeight;
                        }
                        /* This is done since in some cases probing may not provide video width and height. In that case for HEVC we force the max Height and max Width.*/
                        else if((NEXUS_VideoCodec_eH265 == videoCodec) || (NEXUS_VideoCodec_eVp9 == videoCodec))
                        {
                            connectSettings.simpleVideoDecoder[0].decoderCapabilities.maxWidth = pAppCtx->maxWidth = 3840;
                            connectSettings.simpleVideoDecoder[0].decoderCapabilities.maxHeight = pAppCtx->maxHeight = 2160;
                        }

                        connectSettings.simpleAudioDecoder.id = allocResults.simpleAudioDecoder.id;
                        rc = NxClient_Connect(&connectSettings, &pAppCtx->connectId);
                        BIP_CHECK_GOTO(( rc == NEXUS_SUCCESS ), ( "NxClient_Connect Failed" ), error, BIP_INF_NEXUS_RESOURCE_NOT_AVAILABLE, bipStatus );

                        /* Successfully connected to the NxServer. Acquire AV Decoder & Transcode Handles. */
                        pAppCtx->hSimpleVideoDecoder = NEXUS_SimpleVideoDecoder_Acquire(allocResults.simpleVideoDecoder[0].id);
                        BIP_CHECK_GOTO(( pAppCtx->hSimpleVideoDecoder ), ( "NEXUS_SimpleVideoDecoder_Acquire Failed" ), error, BIP_INF_NEXUS_RESOURCE_NOT_AVAILABLE, bipStatus );

                        pAppCtx->hSimpleAudioDecoder = NEXUS_SimpleAudioDecoder_Acquire(allocResults.simpleAudioDecoder.id);
                        BIP_CHECK_GOTO(( pAppCtx->hSimpleAudioDecoder ), ( "NEXUS_SimpleAudioDecoder_Acquire Failed" ), error, BIP_INF_NEXUS_RESOURCE_NOT_AVAILABLE, bipStatus );

                        pAppCtx->hSimpleStcChannel = NEXUS_SimpleStcChannel_Create(NULL);
                        BIP_CHECK_GOTO(( pAppCtx->hSimpleStcChannel ), ( "NEXUS_SimpleStcChannel_Create Failed" ), error, BIP_INF_NEXUS_RESOURCE_NOT_AVAILABLE, bipStatus );
                        /* We have successfully acquired all needed resources. */
                    }

                    /* Fill-in AV decoder handles. */
                    playerSettings.audioTrackSettings.pidTypeSettings.audio.simpleDecoder = pAppCtx->hSimpleAudioDecoder;
                    playerSettings.videoTrackSettings.pidTypeSettings.video.simpleDecoder = pAppCtx->hSimpleVideoDecoder;
                    playerSettings.playbackSettings.simpleStcChannel = pAppCtx->hSimpleStcChannel;

                    /* Fill-in callbacks */
                    playerSettings.playbackSettings.endOfStreamCallback.callback = playbackDoneCallbackFromBIP;
                    playerSettings.playbackSettings.endOfStreamCallback.context = pAppCtx;
                    playerSettings.playbackSettings.errorCallback.callback = playbackDoneCallbackFromBIP;
                    playerSettings.playbackSettings.errorCallback.context = pAppCtx;
                    if (pAppCtx->enableContinousPlay)
                    {
                        playerSettings.playbackSettings.endOfStreamAction = NEXUS_PlaybackLoopMode_eLoop;
                    }
                    else
                    {
                        playerSettings.playbackSettings.endOfStreamAction = NEXUS_PlaybackLoopMode_ePause;
                    }
                    pAppCtx->asyncApiCompletionStatus = BIP_INF_IN_PROGRESS;

                    /* In this example, Prepare() will internally select the 1st Audio & Video TrackId and use them to create the PidChannels. */
                    bipStatus = BIP_Player_PrepareAsync(pAppCtx->hPlayer, &prepareSettings, &playerSettings, NULL/*&probeSettings*/, &pAppCtx->playerStreamInfo, &pAppCtx->prepareStatus, &pAppCtx->asyncCallbackDesc, &pAppCtx->asyncApiCompletionStatus);
                    BIP_CHECK_GOTO(( bipStatus == BIP_INF_IN_PROGRESS ), ( "BIP_Player_ProbeAsync Failed!"), error, bipStatus, bipStatus );
                    BDBG_WRN(("Player Prepare API started: transportType=%s video codec=%s trackId=%d; audio codec=%s trackId=%d ",
                                BIP_ToStr_NEXUS_TransportType(pMediaInfoStream->transportType),
                                BIP_ToStr_NEXUS_VideoCodec(playerSettings.videoTrackSettings.pidTypeSettings.video.codec), playerSettings.videoTrackId,
                                BIP_ToStr_NEXUS_AudioCodec(playerSettings.audioTrackSettings.pidSettings.pidTypeSettings.audio.codec), playerSettings.audioTrackId
                                ));
                    pAppCtx->playerState = BMediaPlayerState_eWaitingForPrepare;
                }
            }
            /* else Probe API is still in progress..., nothing to do yet.. */
        }

    case BMediaPlayerState_eWaitingForPrepare:
        {
            /* Check if the BIP_Player_PrepareAsync() is still in progress, completed successfully or w/ an error status. */
            BIP_CHECK_GOTO((pAppCtx->asyncApiCompletionStatus==BIP_INF_IN_PROGRESS || pAppCtx->asyncApiCompletionStatus == BIP_SUCCESS), ("BIP_Player_PrepareAsync() Failed!"), error, pAppCtx->asyncApiCompletionStatus, bipStatus);

            if (pAppCtx->asyncApiCompletionStatus == BIP_SUCCESS)
            {
                /* BIP_Player_PrepareAsync() is successful, so get ready for starting the Player. */
                NEXUS_Error             rc = NEXUS_UNKNOWN;
                NEXUS_PidChannelHandle  videoPidChannel = NULL, audioPidChannel = NULL;
                NEXUS_AudioCodec        audioCodec;
                NEXUS_VideoCodec        videoCodec;

                /* Configure the AV Decoder Start Settings & Start them. */
                audioCodec = pAppCtx->prepareStatus.audioCodec;
                audioPidChannel = pAppCtx->prepareStatus.hAudioPidChannel;

                videoCodec = pAppCtx->prepareStatus.videoCodec;
                videoPidChannel = pAppCtx->prepareStatus.hVideoPidChannel;

                /* Set up & start video decoder. */
                if (videoPidChannel)
                {
                    NEXUS_SimpleVideoDecoderStartSettings videoProgram;
                    rc = NEXUS_SimpleVideoDecoder_SetStcChannel(pAppCtx->hSimpleVideoDecoder, pAppCtx->hSimpleStcChannel);
                    BIP_CHECK_GOTO(( rc == NEXUS_SUCCESS ), ( "NEXUS_SimpleVideoDecoder_SetStcChannel Failed" ), error, BIP_ERR_NEXUS, bipStatus );

                    NEXUS_SimpleVideoDecoder_GetDefaultStartSettings(&videoProgram);
                    videoProgram.displayEnabled = true;
                    videoProgram.settings.codec = videoCodec;
                    videoProgram.settings.pidChannel = videoPidChannel;
                    videoProgram.maxWidth = pAppCtx->maxWidth;
                    videoProgram.maxHeight = pAppCtx->maxHeight;
                    rc = NEXUS_SimpleVideoDecoder_Start(pAppCtx->hSimpleVideoDecoder, &videoProgram);
                    BIP_CHECK_GOTO(( rc == NEXUS_SUCCESS ), ( "NEXUS_SimpleVideoDecoder_Start Failed" ), error, BIP_ERR_NEXUS, bipStatus );
                }

                /* Set up & start audio decoder. */
                if (audioPidChannel)
                {
                    NEXUS_SimpleAudioDecoderStartSettings audioProgram;
                    rc = NEXUS_SimpleAudioDecoder_SetStcChannel(pAppCtx->hSimpleAudioDecoder, pAppCtx->hSimpleStcChannel);
                    BIP_CHECK_GOTO(( rc == NEXUS_SUCCESS ), ( "NEXUS_SimpleAudioDecoder_SetStcChannel Failed" ), error, BIP_ERR_NEXUS, bipStatus );

                    NEXUS_SimpleAudioDecoder_GetDefaultStartSettings(&audioProgram);
                    audioProgram.primary.codec = audioCodec;
                    audioProgram.primary.pidChannel = audioPidChannel;
                    rc = NEXUS_SimpleAudioDecoder_Start(pAppCtx->hSimpleAudioDecoder, &audioProgram);
                    BIP_CHECK_GOTO(( rc == NEXUS_SUCCESS ), ( "NEXUS_SimpleAudioDecoder_Start Failed" ), error, BIP_ERR_NEXUS, bipStatus );
                }

                /* NEXUS Setup is complete, now start BIP_Player. */
                {
                    BIP_PlayerStartSettings startSettings;

                    BIP_Player_GetDefaultStartSettings(&startSettings);
                    startSettings.timePositionInMs = pAppCtx->initialPlaybackPositionInMs;
                    pAppCtx->playbackDone = false;
                    pAppCtx->asyncApiCompletionStatus = BIP_INF_IN_PROGRESS;
                    bipStatus = BIP_Player_StartAsync(pAppCtx->hPlayer, &startSettings, &pAppCtx->asyncCallbackDesc, &pAppCtx->asyncApiCompletionStatus);
                    BIP_CHECK_GOTO(( bipStatus == BIP_INF_IN_PROGRESS ), ( "BIP_Player_Start Failed: URL=%s", BIP_String_GetString(pAppCtx->hUrl) ), error, bipStatus, bipStatus );
                    BDBG_MSG(("Player Start API started..")); /* Player state transition: Prepared --> Starting. */
                }

                pAppCtx->playerState = BMediaPlayerState_eWaitingForStart;
                break;
            }
            /* else Prepare is still in progress..., nothing to do yet.. */
            break;
        }

    case BMediaPlayerState_eWaitingForStart:
        {
            /* Check if the BIP_Player_StartAsync is still in progress, completed successfully or w/ an error status. */
            BIP_CHECK_GOTO((pAppCtx->asyncApiCompletionStatus==BIP_INF_IN_PROGRESS || pAppCtx->asyncApiCompletionStatus == BIP_SUCCESS), ("BIP_Player_StartAsync() Failed!"), error, pAppCtx->asyncApiCompletionStatus, bipStatus);

            if (pAppCtx->asyncApiCompletionStatus == BIP_SUCCESS)
            {
                /* BIP_Player_StartAsync() is successful, Player is now started. */
                BDBG_MSG(("In Started state, Playing Media Stream. "));
                pAppCtx->playerState = BMediaPlayerState_eStarted;
                bipStatus = BIP_SUCCESS;
            }
            /* else Start API is still in progress..., nothing to do yet.. */
            break;
        }

    case BMediaPlayerState_eStarted:
        {
            if (pAppCtx->playbackDone == true || pCmdOptions->cmd == PlayerCmd_eQuit)
            {
                BDBG_WRN(("Playback is complete: %s", pAppCtx->playbackDone?"Reached EndOfStream" : "User Quit"));
                bipStatus = BIP_SUCCESS;
                goto done;
            }
            else
            {
                if ( pCmdOptions->cmd == PlayerCmd_eSeek)
                {
                    BDBG_WRN(("Starting Seek to %d ms", pCmdOptions->seekPositionInMs));
                    pAppCtx->asyncApiCompletionStatus = BIP_INF_IN_PROGRESS;
                    bipStatus = BIP_Player_SeekAsync(pAppCtx->hPlayer, pCmdOptions->seekPositionInMs, &pAppCtx->asyncCallbackDesc, &pAppCtx->asyncApiCompletionStatus);
                    BIP_CHECK_GOTO(( bipStatus == BIP_INF_IN_PROGRESS ), ( "BIP_Player_SeekAsync Failed: URL=%s", BIP_String_GetString(pAppCtx->hUrl) ), error, bipStatus, bipStatus );
                    pAppCtx->playerState = BMediaPlayerState_eWaitingForSeek;
                    pAppCtx->lastSeekPositionInMs = pCmdOptions->seekPositionInMs;
                    BDBG_WRN(("Async Seeked Started..."));
                }
                else if ( pCmdOptions->cmd == PlayerCmd_eRelativeSeekFwd || pCmdOptions->cmd == PlayerCmd_eRelativeSeekRev)
                {
                    BIP_PlayerStatus playerStatus;

                    bipStatus = BIP_Player_GetStatus(pAppCtx->hPlayer, &playerStatus);
                    BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_Player_Status Failed: URL=%s", BIP_String_GetString(pAppCtx->hUrl) ), error, bipStatus, bipStatus );
                    if ( pCmdOptions->cmd == PlayerCmd_eRelativeSeekFwd)
                    {
                        pCmdOptions->seekPositionInMs = playerStatus.currentPositionInMs + pAppCtx->jumpOffsetInMs;
                        BDBG_WRN(("Starting Seek forward from currentPositionInMs=%d ms by offset=%d, seekedPosition=%d", playerStatus.currentPositionInMs, pAppCtx->jumpOffsetInMs, pCmdOptions->seekPositionInMs));
                    }
                    else
                    {
                        pCmdOptions->seekPositionInMs = playerStatus.currentPositionInMs > pAppCtx->jumpOffsetInMs ?  playerStatus.currentPositionInMs - pAppCtx->jumpOffsetInMs : 0;
                        BDBG_WRN(("Starting Seek backward from currentPositionInMs=%d ms by offset=%d, seekedPosition=%d", playerStatus.currentPositionInMs, pAppCtx->jumpOffsetInMs, pCmdOptions->seekPositionInMs));
                    }
                    bipStatus = BIP_Player_SeekAsync(pAppCtx->hPlayer, pCmdOptions->seekPositionInMs, &pAppCtx->asyncCallbackDesc, &pAppCtx->asyncApiCompletionStatus);
                    BIP_CHECK_GOTO(( bipStatus == BIP_INF_IN_PROGRESS ), ( "BIP_Player_SeekAsync Failed: URL=%s", BIP_String_GetString(pAppCtx->hUrl) ), error, bipStatus, bipStatus );
                    pAppCtx->playerState = BMediaPlayerState_eWaitingForSeek;
                    pAppCtx->lastSeekPositionInMs = pCmdOptions->seekPositionInMs;
                    BDBG_WRN(("Async Seeked Started..."));
                }
                else if ( pCmdOptions->cmd == PlayerCmd_ePause)
                {
                    BDBG_WRN(("Pausing Playback!"));
                    bipStatus = BIP_Player_Pause(pAppCtx->hPlayer, NULL);
                    BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_Player_Pause Failed: URL=%s", BIP_String_GetString(pAppCtx->hUrl) ), error, bipStatus, bipStatus );
                    BDBG_WRN(("Paused Playback!"));
                }
                else if ( pCmdOptions->cmd == PlayerCmd_ePlay)
                {
                    BDBG_WRN(("Resuming Playback!"));
                    bipStatus = BIP_Player_Play(pAppCtx->hPlayer);
                    BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_Player_Play Failed: URL=%s", BIP_String_GetString(pAppCtx->hUrl) ), error, bipStatus, bipStatus );
                    BDBG_WRN(("Playback Resumed!"));
                }
                else if ( pCmdOptions->cmd == PlayerCmd_ePlayAtRate)
                {
                    BDBG_WRN(("Playing at Rate=%s", pCmdOptions->playSpeed));
                    bipStatus = BIP_Player_PlayAtRateAsString(pAppCtx->hPlayer, pCmdOptions->playSpeed, NULL);
                    BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_Player_PlayAtRateAsString Failed: URL=%s", BIP_String_GetString(pAppCtx->hUrl) ), error, bipStatus, bipStatus );
                    BDBG_WRN(("Started Playing at Rate=%s", pCmdOptions->playSpeed));
                }
                else if ( pAppCtx->printStatus || pCmdOptions->cmd == PlayerCmd_ePrintStatus)
                {
                    BIP_PlayerStatus playerStatus;
                    bipStatus = BIP_Player_GetStatus(pAppCtx->hPlayer, &playerStatus);
                    BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_Player_Status Failed: URL=%s", BIP_String_GetString(pAppCtx->hUrl) ), error, bipStatus, bipStatus );
                    BDBG_WRN(("Player position cur/last: %0.3f/%0.3f sec ", playerStatus.currentPositionInMs/1000., playerStatus.lastPositionInMs/1000. ));
                }
                else if ( pAppCtx->printServerStatus || pCmdOptions->cmd == PlayerCmd_ePrintServerStatus)
                {
                    BIP_PlayerStatusFromServer serverStatus;
                    BIP_PlayerGetStatusFromServerSettings settings;
                    BIP_Player_GetDefaultGetStatusFromServerSettings(&settings);
                    settings.getAvailableSeekRange = true;
                    bipStatus = BIP_Player_GetStatusFromServer(pAppCtx->hPlayer, &settings, &serverStatus);
                    BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_Player_StatusFromServer Failed: URL=%s", BIP_String_GetString(pAppCtx->hUrl) ), error, bipStatus, bipStatus );
                    BDBG_WRN(("Player position cur/last: %0.3f/%0.3f sec ", serverStatus.currentPositionInMs/1000., serverStatus.availableSeekRange.durationInMs/1000. ));
                }
                else
                {
                    if (pCmdOptions->cmd != PlayerCmd_eMax) BDBG_WRN(("Not yet handling this cmd=%d", pCmdOptions->cmd));
                }

                bipStatus = BIP_SUCCESS;
            }
            break;
        }
    case BMediaPlayerState_eWaitingForSeek:
        {
            /* Check if the BIP_Player_StartAsync is still in progress, completed successfully or w/ an error status. */
            BIP_CHECK_GOTO((pAppCtx->asyncApiCompletionStatus==BIP_INF_IN_PROGRESS || pAppCtx->asyncApiCompletionStatus == BIP_SUCCESS), ("BIP_Player_SeekAsync() Failed!"), error, pAppCtx->asyncApiCompletionStatus, bipStatus);

            if (pAppCtx->asyncApiCompletionStatus == BIP_SUCCESS)
            {
                /* BIP_Player_SeekAsync() is successful. */
                BDBG_WRN(("Successfully Seeked to position=%d ms", pAppCtx->lastSeekPositionInMs ));
                pAppCtx->playerState = BMediaPlayerState_eStarted;
                bipStatus = BIP_SUCCESS;
            }
            else
            {
                if (pCmdOptions->cmd != PlayerCmd_eMax) BDBG_WRN(("Seek still in progress, ignoring this cmd=%d", pCmdOptions->cmd));
            }
            /* else Start API is still in progress..., nothing to do yet.. */
            break;
        }

    default:
        {
            BDBG_ASSERT(NULL);
        }
    } /* switch (playerState) */
    /* non-error path, we return the current status. */
    return (bipStatus);

done:
error:
    /* Stop the Player & Decoders. */
    {
        if (pAppCtx->hPlayer) BIP_Player_Stop(pAppCtx->hPlayer);

        if (pAppCtx->hSimpleAudioDecoder)
        {
            NEXUS_SimpleAudioDecoder_Stop(pAppCtx->hSimpleAudioDecoder);
            NEXUS_SimpleAudioDecoder_SetStcChannel(pAppCtx->hSimpleAudioDecoder, NULL);
        }

        if (pAppCtx->hSimpleVideoDecoder)
        {
            NEXUS_SimpleVideoDecoder_Stop(pAppCtx->hSimpleVideoDecoder);
            NEXUS_SimpleVideoDecoder_SetStcChannel(pAppCtx->hSimpleVideoDecoder, NULL);
        }

        if (pAppCtx->hPlayer) BIP_Player_Disconnect(pAppCtx->hPlayer);
    }

    return (bipStatus);
} /* processMediaPlayerState */

int main(int argc, char *argv[])
{
    AppCtx                  *pAppCtx;
    NEXUS_Error             rc = NEXUS_UNKNOWN;
    BIP_Status              bipStatus;
    CmdOptions              cmdOptions;

    /* Initialize BIP */
    {
        bipStatus = BIP_Init(NULL);
        BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_Init Failed" ), errorBipInit, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );
    }

    /* Allocate & initialize App Ctx */
    {
        pAppCtx = initAppCtx();
        BIP_CHECK_GOTO(( pAppCtx ), ( "initAppCtx Failed" ), errorBipInit, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );

        /* Parse commandline options */
        bipStatus = parseOptions( argc, argv, pAppCtx );
        BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "parseOptions Failed" ), errorParseOptions, bipStatus, bipStatus );
    }

    /* Initialize Nexus */
    {
        NxClient_JoinSettings joinSettings;

        NxClient_GetDefaultJoinSettings(&joinSettings);
        snprintf(joinSettings.name, NXCLIENT_MAX_NAME, "%s", argv[0]);
        rc = NxClient_Join(&joinSettings);
        BIP_CHECK_GOTO(( rc == NEXUS_SUCCESS ), ( "NxClient_Join Failed" ), error, BIP_ERR_INTERNAL, bipStatus );
    }

    /* Init DtcpIpClientFactory */
    {
        bipStatus = BIP_DtcpIpClientFactory_Init(NULL);
        BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS || bipStatus == BIP_ERR_NOT_SUPPORTED), ( "BIP_DtcpIpClientFactory_Init Failed" ), errorBipInit, bipStatus, bipStatus );
    }

    /* Initialize SslClientFactory */
    {
#define TEST_ROOT_CA_PATH   "./host.cert"
        BIP_SslClientFactoryInitSettings settings;
        BIP_SslClientFactory_GetDefaultInitSettings(&settings);
        settings.pRootCaCertPath = TEST_ROOT_CA_PATH;
        bipStatus = BIP_SslClientFactory_Init(&settings);
        BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_SslClientFactory_Init Failed" ), errorBipInit, bipStatus, bipStatus );
    }

    /* Create BIP Player instance */
    {
        pAppCtx->hPlayer = BIP_Player_Create( NULL );
        BIP_CHECK_GOTO(( pAppCtx->hPlayer ), ( "BIP_Player_Create Failed" ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );
    }

    /* For Async mode usage of BIP Player APIs, create a completion event & setup the callback descriptor. */
    /* Note: this simple example uses the same callback function for all async completion or callbacks from BIP Player. */
    {
        pAppCtx->hCompletionEvent = B_Event_Create(NULL);
        BIP_CHECK_GOTO(( pAppCtx->hCompletionEvent ), ( "B_Event_Create Failed" ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );
        pAppCtx->asyncCallbackDesc.context = pAppCtx->hCompletionEvent;
        pAppCtx->asyncCallbackDesc.callback = asyncApiCompletionCallbackFromBip;
        pAppCtx->asyncCallbackDesc.param = 0;
    }

    /* Now App has done basic initialization & can play a URL. Lets kick off App Player processing. */
    printf("player>\n");
    do
    {
        /* Check for user-input */
        bipStatus = runTimeCmdParsing(pAppCtx, &cmdOptions);
        if ( bipStatus != BIP_SUCCESS ) break;

        /* process Media Player State */
        bipStatus = processMediaPlayerState(pAppCtx, &cmdOptions);
        if (bipStatus != BIP_SUCCESS && bipStatus != BIP_INF_IN_PROGRESS ) break; /* Error in Player state processing, we are done! */
    } while (pAppCtx->playbackDone == false && cmdOptions.cmd != PlayerCmd_eQuit);

error:
    /* Close the Resources. */
    {
        NEXUS_SimpleVideoDecoder_Release(pAppCtx->hSimpleVideoDecoder);
        NEXUS_SimpleAudioDecoder_Release(pAppCtx->hSimpleAudioDecoder);
        NEXUS_SimpleStcChannel_Destroy(pAppCtx->hSimpleStcChannel);

        if (pAppCtx->hPlayer) BIP_Player_Destroy(pAppCtx->hPlayer);

errorParseOptions:
        if (pAppCtx->hCompletionEvent) B_Event_Destroy(pAppCtx->hCompletionEvent);
        unInitAppCtx( pAppCtx );
        BIP_DtcpIpClientFactory_Uninit();
        NxClient_Uninit();
    }
    BIP_SslClientFactory_Uninit();
errorBipInit:
    BIP_Uninit();
    return (bipStatus);
}
