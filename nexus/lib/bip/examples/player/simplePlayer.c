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
 *
 * Module Description:
 *  Example program to demonstrate receiving live or playback content over IP Channels (UDP/RTP/RTSP/HTTP based)
 *
 ******************************************************************************/
#include "nxclient.h"
#include "nexus_platform_client.h"

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
    BDBG_MSG(( BIP_MSG_PRE_FMT " Settings hEvent=%p)" BIP_MSG_PRE_ARG, pAppCtx->hCompletionEvent ));
    pAppCtx->playbackDone = true;
    B_Event_Set(pAppCtx->hCompletionEvent);
}

int main(int argc, char *argv[])
{
    AppCtx *pAppCtx;
    CmdOptions cmdOptions;
    BIP_Status bipStatus;
    NEXUS_Error rc = NEXUS_UNKNOWN;
    NEXUS_PidChannelHandle videoPidChannel = NULL, audioPidChannel = NULL;
    NEXUS_AudioCodec audioCodec;
    NEXUS_VideoCodec videoCodec;
    BIP_PlayerHandle hPlayer = NULL;
    NEXUS_SimpleVideoDecoderHandle hSimpleVideoDecoder = NULL;
    NEXUS_SimpleAudioDecoderHandle hSimpleAudioDecoder = NULL;
    NEXUS_SimpleStcChannelHandle hSimpleStcChannel = NULL;

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

        pAppCtx->hCompletionEvent = B_Event_Create(NULL);
        BIP_CHECK_GOTO(( pAppCtx->hCompletionEvent ), ( "B_Event_Create Failed" ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );
    }

    /* Initialize Nexus */
    {
        NxClient_JoinSettings joinSettings;

        NxClient_GetDefaultJoinSettings(&joinSettings);
        snprintf(joinSettings.name, NXCLIENT_MAX_NAME, "%s", argv[0]);
        rc = NxClient_Join(&joinSettings);
        BIP_CHECK_GOTO(( rc == NEXUS_SUCCESS ), ( "NxClient_Join Failed" ), error, BIP_ERR_INTERNAL, bipStatus );
    }

    /* Initialize DtcpIpClientFactory */
    {
        bipStatus = BIP_DtcpIpClientFactory_Init(NULL);
        BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS || bipStatus == BIP_ERR_NOT_SUPPORTED), ( "BIP_DtcpIpClientFactory_Init Failed" ), errorBipInit, bipStatus, bipStatus );
    }

    {
        NxClient_AllocSettings      allocSettings;
        NxClient_AllocResults       allocResults;
        NxClient_ConnectSettings    connectSettings;

        NxClient_GetDefaultAllocSettings(&allocSettings);
        allocSettings.simpleVideoDecoder = 1;
        allocSettings.simpleAudioDecoder = 1;
        rc = NxClient_Alloc( &allocSettings, &allocResults );
        BIP_CHECK_GOTO(( rc == NEXUS_SUCCESS ), ( "NxClient_Alloc Failed" ), error, BIP_INF_NEXUS_RESOURCE_NOT_AVAILABLE, bipStatus );

        NxClient_GetDefaultConnectSettings( &connectSettings );
        connectSettings.simpleVideoDecoder[0].id = allocResults.simpleVideoDecoder[0].id;
        connectSettings.simpleAudioDecoder.id = allocResults.simpleAudioDecoder.id;
        connectSettings.simpleVideoDecoder[0].decoderCapabilities.maxWidth = pAppCtx->maxWidth;
        connectSettings.simpleVideoDecoder[0].decoderCapabilities.maxHeight = pAppCtx->maxHeight;
        rc = NxClient_Connect(&connectSettings, &pAppCtx->connectId);
        BIP_CHECK_GOTO(( rc == NEXUS_SUCCESS ), ( "NxClient_Connect Failed" ), error, BIP_INF_NEXUS_RESOURCE_NOT_AVAILABLE, bipStatus );

        /* Successfully connected to the NxServer. Acquire AV Decoder & Transcode Handles. */
        hSimpleVideoDecoder = NEXUS_SimpleVideoDecoder_Acquire(allocResults.simpleVideoDecoder[0].id);
        BIP_CHECK_GOTO(( hSimpleVideoDecoder ), ( "NEXUS_SimpleVideoDecoder_Acquire Failed" ), error, BIP_INF_NEXUS_RESOURCE_NOT_AVAILABLE, bipStatus );

        hSimpleAudioDecoder = NEXUS_SimpleAudioDecoder_Acquire(allocResults.simpleAudioDecoder.id);
        BIP_CHECK_GOTO(( hSimpleAudioDecoder ), ( "NEXUS_SimpleAudioDecoder_Acquire Failed" ), error, BIP_INF_NEXUS_RESOURCE_NOT_AVAILABLE, bipStatus );

        hSimpleStcChannel = NEXUS_SimpleStcChannel_Create(NULL);
        BIP_CHECK_GOTO(( hSimpleStcChannel ), ( "NEXUS_SimpleStcChannel_Create Failed" ), error, BIP_INF_NEXUS_RESOURCE_NOT_AVAILABLE, bipStatus );

        /* We have successfully acquired all needed resources. */
    }

    /* Create BIP Player instance */
    {
        hPlayer = BIP_Player_Create( NULL );
        BIP_CHECK_GOTO(( hPlayer ), ( "BIP_Player_Create Failed" ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );
    }

    /* Connect to Server & do HTTP Request/Response exchange. */
    {
        BIP_PlayerConnectSettings settings;
        const char *responseHeaders = NULL;

        BIP_Player_GetDefaultConnectSettings(&settings);
        settings.pUserAgent = "BIP Player Example";
        bipStatus = BIP_Player_Connect( hPlayer, BIP_String_GetString(pAppCtx->hUrl), &settings );
        BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_Player_Connect Failed to Connect to URL=%s", BIP_String_GetString(pAppCtx->hUrl) ), error, bipStatus, bipStatus );
        BDBG_MSG(("http response headers >>> \n%s", responseHeaders));
    }

    /* Prepare the BIP Player using the default settings. */
    {
        BIP_PlayerPrepareSettings   prepareSettings;
        BIP_PlayerSettings          playerSettings;
        BIP_PlayerPrepareStatus     prepareStatus;

        BIP_Player_GetDefaultPrepareSettings(&prepareSettings);
        BIP_Player_GetDefaultSettings(&playerSettings);
        playerSettings.audioTrackSettings.pidTypeSettings.audio.simpleDecoder = hSimpleAudioDecoder;
        playerSettings.videoTrackSettings.pidTypeSettings.video.simpleDecoder = hSimpleVideoDecoder;
        playerSettings.playbackSettings.simpleStcChannel = hSimpleStcChannel;

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
        bipStatus = BIP_Player_Prepare(hPlayer, &prepareSettings, &playerSettings, NULL/*&probeSettings*/, NULL/*&streamInfo*/, &prepareStatus);
        BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_Player_Prepare Failed: URL=%s", BIP_String_GetString(pAppCtx->hUrl) ), error, bipStatus, bipStatus );

        audioCodec = prepareStatus.audioCodec;
        audioPidChannel = prepareStatus.hAudioPidChannel;

        videoCodec = prepareStatus.videoCodec;
        videoPidChannel = prepareStatus.hVideoPidChannel;
    }

    /* Set up & start video decoder. */
    if (videoPidChannel)
    {
        NEXUS_SimpleVideoDecoderStartSettings videoProgram;
        rc = NEXUS_SimpleVideoDecoder_SetStcChannel(hSimpleVideoDecoder, hSimpleStcChannel);
        BIP_CHECK_GOTO(( rc == NEXUS_SUCCESS ), ( "NEXUS_SimpleVideoDecoder_SetStcChannel Failed" ), error, BIP_ERR_NEXUS, bipStatus );

        NEXUS_SimpleVideoDecoder_GetDefaultStartSettings(&videoProgram);
        videoProgram.displayEnabled = true;
        videoProgram.settings.codec = videoCodec;
        videoProgram.settings.pidChannel = videoPidChannel;
        rc = NEXUS_SimpleVideoDecoder_Start(hSimpleVideoDecoder, &videoProgram);
        BIP_CHECK_GOTO(( rc == NEXUS_SUCCESS ), ( "NEXUS_SimpleVideoDecoder_Start Failed" ), error, BIP_ERR_NEXUS, bipStatus );
    }

    /* Set up & start audio decoder. */
    if (audioPidChannel)
    {
        NEXUS_SimpleAudioDecoderStartSettings audioProgram;
        rc = NEXUS_SimpleAudioDecoder_SetStcChannel(hSimpleAudioDecoder, hSimpleStcChannel);
        BIP_CHECK_GOTO(( rc == NEXUS_SUCCESS ), ( "NEXUS_SimpleAudioDecoder_SetStcChannel Failed" ), error, BIP_ERR_NEXUS, bipStatus );

        NEXUS_SimpleAudioDecoder_GetDefaultStartSettings(&audioProgram);
        audioProgram.primary.codec = audioCodec;
        audioProgram.primary.pidChannel = audioPidChannel;
        rc = NEXUS_SimpleAudioDecoder_Start(hSimpleAudioDecoder, &audioProgram);
        BIP_CHECK_GOTO(( rc == NEXUS_SUCCESS ), ( "NEXUS_SimpleAudioDecoder_Start Failed" ), error, BIP_ERR_NEXUS, bipStatus );
    }

    /* NEXUS Setup is complete, now Prepare & start BIP_Player. */
    {
        bipStatus = BIP_Player_Start(hPlayer, NULL);
        BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_Player_Start Failed: URL=%s", BIP_String_GetString(pAppCtx->hUrl) ), error, bipStatus, bipStatus );
    }

    printf("player>\n");
    do
    {
        runTimeCmdParsing(pAppCtx,&cmdOptions);

        if ( cmdOptions.cmd == PlayerCmd_eSeek)
        {
            BDBG_WRN(("Starting Seek to %d ms", cmdOptions.seekPositionInMs));
            bipStatus = BIP_Player_Seek(hPlayer, cmdOptions.seekPositionInMs);
            BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_Player_Seek Failed: URL=%s", BIP_String_GetString(pAppCtx->hUrl) ), error, bipStatus, bipStatus );
            BDBG_WRN(("Seeked to %d ms", cmdOptions.seekPositionInMs));
        }
        else if ( cmdOptions.cmd == PlayerCmd_eRelativeSeekFwd || cmdOptions.cmd == PlayerCmd_eRelativeSeekRev)
        {
            BIP_PlayerStatus playerStatus;

            bipStatus = BIP_Player_GetStatus(hPlayer, &playerStatus);
            BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_Player_Status Failed: URL=%s", BIP_String_GetString(pAppCtx->hUrl) ), error, bipStatus, bipStatus );
            if ( cmdOptions.cmd == PlayerCmd_eRelativeSeekFwd)
            {
                cmdOptions.seekPositionInMs = playerStatus.currentPositionInMs + pAppCtx->jumpOffsetInMs;
                BDBG_WRN(("Starting Seek forward from currentPositionInMs=%lu ms by offset=%d, seekedPosition=%d", playerStatus.currentPositionInMs, pAppCtx->jumpOffsetInMs, cmdOptions.seekPositionInMs));
            }
            else
            {
                cmdOptions.seekPositionInMs = playerStatus.currentPositionInMs > pAppCtx->jumpOffsetInMs ?  playerStatus.currentPositionInMs - pAppCtx->jumpOffsetInMs : 0;
                BDBG_WRN(("Starting Seek backward from currentPositionInMs=%lu ms by offset=%d, seekedPosition=%d", playerStatus.currentPositionInMs, pAppCtx->jumpOffsetInMs, cmdOptions.seekPositionInMs));
            }
            bipStatus = BIP_Player_Seek(hPlayer, cmdOptions.seekPositionInMs);
            BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_Player_Seek Failed: URL=%s", BIP_String_GetString(pAppCtx->hUrl) ), error, bipStatus, bipStatus );
            BDBG_WRN(("Seeked to %d ms", cmdOptions.seekPositionInMs));
        }
        else if ( cmdOptions.cmd == PlayerCmd_ePause)
        {
            BDBG_WRN(("Pausing Playback!"));
            bipStatus = BIP_Player_Pause(hPlayer, NULL);
            BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_Player_Pause Failed: URL=%s", BIP_String_GetString(pAppCtx->hUrl) ), error, bipStatus, bipStatus );
            BDBG_WRN(("Paused Playback!"));
        }
        else if ( cmdOptions.cmd == PlayerCmd_ePlay)
        {
            BDBG_WRN(("Resuming Playback!"));
            bipStatus = BIP_Player_Play(hPlayer);
            BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_Player_Play Failed: URL=%s", BIP_String_GetString(pAppCtx->hUrl) ), error, bipStatus, bipStatus );
            BDBG_WRN(("Playback Resumed!"));
        }
        else if ( cmdOptions.cmd == PlayerCmd_ePlayAtRate)
        {
            BDBG_WRN(("Playing at Rate=%s", cmdOptions.playSpeed));
            bipStatus = BIP_Player_PlayAtRateAsString(hPlayer, cmdOptions.playSpeed, NULL);
            BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_Player_PlayAtRateAsString Failed: URL=%s", BIP_String_GetString(pAppCtx->hUrl) ), error, bipStatus, bipStatus );
            BDBG_WRN(("Started Playing at Rate=%s", cmdOptions.playSpeed));
        }
        else if ( pAppCtx->printStatus || cmdOptions.cmd == PlayerCmd_ePrintStatus)
        {
            BIP_PlayerStatus playerStatus;
            bipStatus = BIP_Player_GetStatus(hPlayer, &playerStatus);
            BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_Player_Status Failed: URL=%s", BIP_String_GetString(pAppCtx->hUrl) ), error, bipStatus, bipStatus );
            BDBG_WRN(("Player position cur/last: %0.3f/%0.3f sec ", playerStatus.currentPositionInMs/1000., playerStatus.lastPositionInMs/1000. ));
        }
        else if ( pAppCtx->printServerStatus || cmdOptions.cmd == PlayerCmd_ePrintServerStatus)
        {
            BIP_PlayerStatusFromServer serverStatus;
            BIP_PlayerGetStatusFromServerSettings settings;
            BIP_Player_GetDefaultGetStatusFromServerSettings(&settings);
            settings.getAvailableSeekRange = true;
            bipStatus = BIP_Player_GetStatusFromServer(hPlayer, &settings, &serverStatus);
            BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_Player_StatusFromServer Failed: URL=%s", BIP_String_GetString(pAppCtx->hUrl) ), error, bipStatus, bipStatus );
            BDBG_WRN(("Player position cur/last: %0.3f/%0.3f sec ", serverStatus.currentPositionInMs/1000., serverStatus.availableSeekRange.durationInMs/1000. ));
        }
        else
        {
            if (cmdOptions.cmd != PlayerCmd_eMax) BDBG_WRN(("Not yet handling this cmd=%d", cmdOptions.cmd));
        }
    } while (pAppCtx->playbackDone == false && cmdOptions.cmd != PlayerCmd_eQuit);
    bipStatus = BIP_SUCCESS;

error:
    /* Stop the Player & Decoders. */
    {
        BIP_Player_Stop(hPlayer);
        if (audioPidChannel) NEXUS_SimpleAudioDecoder_Stop(hSimpleAudioDecoder);
        if (audioPidChannel) NEXUS_SimpleAudioDecoder_SetStcChannel(hSimpleAudioDecoder, NULL);

        if (videoPidChannel) NEXUS_SimpleVideoDecoder_Stop(hSimpleVideoDecoder);
        if (videoPidChannel) NEXUS_SimpleVideoDecoder_SetStcChannel(hSimpleVideoDecoder, NULL);
    }

    /* Close the Resources. */
    {
        if (hPlayer) BIP_Player_Disconnect(hPlayer);
        NEXUS_SimpleVideoDecoder_Release(hSimpleVideoDecoder);
        NEXUS_SimpleAudioDecoder_Release(hSimpleAudioDecoder);
        NEXUS_SimpleStcChannel_Destroy(hSimpleStcChannel);
        if (pAppCtx->hCompletionEvent) B_Event_Destroy(pAppCtx->hCompletionEvent);
        if (hPlayer) BIP_Player_Destroy(hPlayer);
errorParseOptions:
        unInitAppCtx( pAppCtx );
        BIP_DtcpIpClientFactory_Uninit();
        NxClient_Uninit();
    }
errorBipInit:
    BIP_Uninit();
    return (0);
}
