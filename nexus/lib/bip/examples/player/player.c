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
#if NXCLIENT_SUPPORT
#include "nxclient.h"
#include "nexus_platform_client.h"
#else
#if NEXUS_HAS_SYNC_CHANNEL
#include "nexus_sync_channel.h"
#endif
#include "nexus_platform.h"
#endif

#include "bip.h"
#include "cmd_parsing.h"

#include "nexus_video_decoder_extra.h"
#include "nexus_video_window.h"
#include "bmedia_probe.h"

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

int main(int argc, char *argv[])
{
    AppCtx *pAppCtx;
    CmdOptions cmdOptions;
    NEXUS_Error rc = NEXUS_UNKNOWN;
    NEXUS_PidChannelHandle videoPidChannel = NULL, audioPidChannel = NULL;
    NEXUS_AudioCodec    audioCodec = NEXUS_AudioCodec_eUnknown;
    NEXUS_VideoCodec    videoCodec = NEXUS_VideoCodec_eUnknown;
    BIP_PlayerHandle hPlayer = NULL;
    BIP_Status bipStatus;
    BIP_PlayerStartSettings startSettings;
#if NXCLIENT_SUPPORT
    NEXUS_SimpleVideoDecoderHandle hSimpleVideoDecoder = NULL;
    NEXUS_SimpleAudioDecoderHandle hSimpleAudioDecoder = NULL;
    NEXUS_SimpleStcChannelHandle hSimpleStcChannel = NULL;
#else
#if NEXUS_HAS_SYNC_CHANNEL
    NEXUS_SyncChannelSettings syncChannelSettings;
    NEXUS_SyncChannelHandle syncChannel = NULL;
#endif
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_StcChannelHandle stcChannel = NULL;
    NEXUS_DisplayHandle display = NULL;
    NEXUS_VideoWindowHandle window = NULL;
    NEXUS_VideoDecoderHandle videoDecoder = NULL;
    NEXUS_AudioDecoderHandle pcmDecoder = NULL;
#endif


    /* Initialize BIP */
    bipStatus = BIP_Init(NULL);
    BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_Init Failed" ), errorBipInit, bipStatus, bipStatus );

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
#if NXCLIENT_SUPPORT
    {
        NxClient_JoinSettings joinSettings;

        NxClient_GetDefaultJoinSettings(&joinSettings);
        snprintf(joinSettings.name, NXCLIENT_MAX_NAME, "%s", argv[0]);
        rc = NxClient_Join(&joinSettings);
        BIP_CHECK_GOTO(( rc == NEXUS_SUCCESS ), ( "NxClient_Join Failed" ), error, BIP_ERR_INTERNAL, bipStatus );
    }
#else /* !NXCLIENT_SUPPORT */
    {
        NEXUS_Platform_GetDefaultSettings(&pAppCtx->platformSettings);
        pAppCtx->platformSettings.openFrontend = false;
        rc = NEXUS_Platform_Init(&pAppCtx->platformSettings);
        BIP_CHECK_GOTO(( rc == NEXUS_SUCCESS ), ( "NEXUS_Platform_Init Failed" ), error, BIP_ERR_NEXUS, bipStatus );
        NEXUS_Platform_GetConfiguration(&platformConfig);
    }
#endif /* NXCLIENT_SUPPORT */

    /* Initialize DtcpIpClientFactory */
    {
        BIP_DtcpIpClientFactoryInitSettings settings;

        BIP_DtcpIpClientFactory_GetDefaultInitSettings(&settings);
        if (strcasecmp(BIP_String_GetString(pAppCtx->hDtcpIpKeyFormat), DTCP_IP_KEY_FORMAT_COMMON_DRM ) == 0)
        {
            settings.keyFormat = B_DTCP_KeyFormat_eCommonDRM;
        }
        else
        {
            settings.keyFormat = B_DTCP_KeyFormat_eTest;
        }
        bipStatus = BIP_DtcpIpClientFactory_Init( &settings );
        BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS || bipStatus == BIP_ERR_NOT_SUPPORTED), ( "BIP_DtcpIpClientFactory_Init Failed" ), errorBipInit, bipStatus, bipStatus );
    }

    /* Initialize SslClientFactory */
    {
#define TEST_ROOT_CA_PATH   "./host.cert"
        BIP_SslClientFactoryInitSettings settings;

        BIP_SslClientFactory_GetDefaultInitSettings(&settings);
        settings.pRootCaCertPath = TEST_ROOT_CA_PATH;
        bipStatus = BIP_SslClientFactory_Init(&settings);
        BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS || bipStatus == BIP_ERR_NOT_SUPPORTED), ( "BIP_SslClientFactory_Init Failed" ), errorBipInit, bipStatus, bipStatus );
    }

    /* Create BIP Player instance */
    {
        pAppCtx->hPlayer = hPlayer = BIP_Player_Create( NULL );
        BIP_CHECK_GOTO(( hPlayer ), ( "BIP_Player_Create Failed" ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );
    }

#ifndef NXCLIENT_SUPPORT
#if NEXUS_HAS_SYNC_CHANNEL
    {
        /* Create a sync channel */
        NEXUS_SyncChannel_GetDefaultSettings(&syncChannelSettings);
        syncChannelSettings.enablePrecisionLipsync = true;
        if (pAppCtx->enableLowLatencyMode)
        {
            syncChannelSettings.enablePrecisionLipsync = !pAppCtx->disablePrecisionLipsync;
            BDBG_LOG(("LowLatencyMode: syncChannelSettings.enablePrecisionLipsync = %d", syncChannelSettings.enablePrecisionLipsync));
        }
        syncChannel = NEXUS_SyncChannel_Create(&syncChannelSettings);
        BIP_CHECK_GOTO(( syncChannel ), ( "NEXUS_SyncChannel_Create Failed" ), error, BIP_ERR_NEXUS, bipStatus );
        BDBG_MSG (("Using Nexus STC channel for lipsync"));
    }
#endif
    {
        /* Once probing is done open stc channel, video and audio decoders.*/
        /* Open StcChannel */
        if (!pAppCtx->disableTsm)
        {
            NEXUS_StcChannelSettings stcChannelSettings;

            NEXUS_StcChannel_GetDefaultSettings(NEXUS_ANY_ID, &stcChannelSettings);
            stcChannelSettings.mode = NEXUS_StcChannelMode_eAuto;
            stcChannel = NEXUS_StcChannel_Open(NEXUS_ANY_ID, &stcChannelSettings);
            BIP_CHECK_GOTO(( stcChannel ), ( "NEXUS_StcChannel_Open Failed" ), error, BIP_ERR_NEXUS, bipStatus );
        }

        /* Open Video decoder */
        if (!pAppCtx->disableVideo)
        {
            NEXUS_VideoDecoderOpenSettings videoDecoderOpenSettings;

            NEXUS_VideoDecoder_GetDefaultOpenSettings(&videoDecoderOpenSettings);
            /* Increase the CDB size as it may be used as the dejitter buffer */
            videoDecoderOpenSettings.fifoSize = videoDecoderOpenSettings.fifoSize*2;
            videoDecoder = NEXUS_VideoDecoder_Open(0, &videoDecoderOpenSettings);
            BIP_CHECK_GOTO(( videoDecoder ), ( "NEXUS_VideoDecoder_Open Failed" ), error, BIP_ERR_NEXUS, bipStatus );
        }

        /* Open audio decoder */
        if (!pAppCtx->disableAudio)
        {
            NEXUS_AudioDecoderOpenSettings audioDecoderOpenSettings;

            NEXUS_AudioDecoder_GetDefaultOpenSettings(&audioDecoderOpenSettings);
            BDBG_MSG(("fifo size %d, type %d\n", audioDecoderOpenSettings.fifoSize, audioDecoderOpenSettings.type));
            audioDecoderOpenSettings.fifoSize = 2*audioDecoderOpenSettings.fifoSize; /* just doubled size is enough for using audio CDB as dejitter buffer. */
            audioDecoderOpenSettings.type = NEXUS_AudioDecoderType_eDecode;
            pcmDecoder = NEXUS_AudioDecoder_Open(0, &audioDecoderOpenSettings);
            BIP_CHECK_GOTO(( pcmDecoder ), ( "NEXUS_AudioDecoder_Open Failed" ), error, BIP_ERR_NEXUS, bipStatus );
        }

        /* Bring up video display and outputs */
        if (!pAppCtx->disableVideo)
        {
            NEXUS_DisplaySettings displaySettings;

            NEXUS_Display_GetDefaultSettings(&displaySettings);
            /* Note: change to display type back for panel output */

            displaySettings.displayType = NEXUS_DisplayType_eAuto;
            displaySettings.format = pAppCtx->displayFormat;
            display = NEXUS_Display_Open(0, &displaySettings);
            BIP_CHECK_GOTO(( display ), ( "NEXUS_Display_Open Failed" ), error, BIP_ERR_NEXUS, bipStatus );

            window = NEXUS_VideoWindow_Open(display, 0);
            BIP_CHECK_GOTO(( window ), ( "NEXUS_VideoDecoder_Open Failed" ), error, BIP_ERR_NEXUS, bipStatus );
#if NEXUS_NUM_HDMI_OUTPUTS
            /* Install hotplug callback -- video only for now */
            rc = NEXUS_Display_AddOutput(display, NEXUS_HdmiOutput_GetVideoConnector(platformConfig.outputs.hdmi[0]));
            BIP_CHECK_GOTO(( rc == NEXUS_SUCCESS ), ( "NEXUS_Display_AddOutput Failed" ), error, BIP_ERR_NEXUS, bipStatus );
#endif
            BDBG_MSG(("Display is Opened\n"));
        }
    }
#endif

    /* External while loop: this is to look for a new url request to simulate channel change! */
    do
    {
        /* Re-Initialize all parameter*/
        videoPidChannel = NULL;
        audioPidChannel = NULL;
        audioCodec = NEXUS_AudioCodec_eUnknown;
        videoCodec = NEXUS_VideoCodec_eUnknown;
#if NXCLIENT_SUPPORT
        hSimpleVideoDecoder = NULL;
        hSimpleAudioDecoder = NULL;
        hSimpleStcChannel = NULL;
#endif
        /*reset command flag to play */
        cmdOptions.cmd = PlayerCmd_ePlay;

        /* Connect to Server & do Player protocol specific exchange. */
        {
            BIP_PlayerConnectSettings settings;

            BIP_Player_GetDefaultConnectSettings(&settings);
            settings.pUserAgent = "BIP Player Example";
            /* Pass-in an additional header to request the seekable range from the server. Some servers use it for indicating duration for both time-shifted & DVR streams. */
#if 1
            settings.pAdditionalHeaders = "getAvailableSeekRange.dlna.org:1\r\n";
#else
            settings.pAdditionalHeaders = "getAvailableSeekRange.dlna.org:1\r\nPlaySpeed.dlna.org: speed=8\r\n";
#endif
            if (pAppCtx->enableTimeshifting) settings.dataAvailabilityModel = BIP_PlayerDataAvailabilityModel_eLimitedRandomAccess;
            settings.maxIpNetworkJitterInMs = pAppCtx->maxIpNetworkJitterInMs;
            if (BIP_String_GetLength(pAppCtx->hInterfaceName)) settings.pNetworkInterfaceName = BIP_String_GetString(pAppCtx->hInterfaceName);
            bipStatus = BIP_Player_Connect( hPlayer, BIP_String_GetString(pAppCtx->hUrl), &settings );
            BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_Player_Connect Failed to Connect to URL=%s", BIP_String_GetString(pAppCtx->hUrl) ), error, bipStatus, bipStatus );
        }

        /* Probe media to find out Video and Audio decoder related information.*/
        {
            BIP_PlayerProbeMediaInfoSettings probeMediaInfoSettings;

            BIP_Player_GetDefaultProbeMediaInfoSettings( &probeMediaInfoSettings );
            pAppCtx->hMediaInfo = NULL;
            probeMediaInfoSettings.enablePayloadScanning = pAppCtx->enablePayloadScanning;
            bipStatus = BIP_Player_ProbeMediaInfo(hPlayer, &probeMediaInfoSettings, &pAppCtx->hMediaInfo );
            BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_Player_Probe Failed!"), error, bipStatus, bipStatus );

            /* Map the MediaStream Info to the Player's StreamInfo. */
            bipStatus = BIP_Player_GetProbedStreamInfo(pAppCtx->hPlayer, &pAppCtx->playerStreamInfo);
            BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_Player_GetProbedStreamInfo Failed!"), error, bipStatus, bipStatus );
        }

        BIP_MediaInfo_Print(pAppCtx->hMediaInfo);

        /* Prepare the BIP Player. */
        {
            BIP_PlayerPrepareSettings   prepareSettings;
            BIP_PlayerSettings          playerSettings;
            BIP_PlayerPrepareStatus     prepareStatus;
            BIP_MediaInfoTrack          mediaInfoTrack;
            const BIP_MediaInfoStream   *pMediaInfoStream;

            BIP_Player_GetDefaultPrepareSettings(&prepareSettings);
            BIP_Player_GetDefaultSettings(&playerSettings);

            /* Get first Video & Audio track entry from pMediaInfoStream .*/
            pMediaInfoStream = BIP_MediaInfo_GetStream(pAppCtx->hMediaInfo);
            BIP_CHECK_GOTO(( pMediaInfoStream ), ( "BIP_MediaInfo_GetStream Failed!"), error, bipStatus, bipStatus );
            pAppCtx->streamDurationInMs = pMediaInfoStream->durationInMs;

            if (pAppCtx->hMediaInfo)
            {
               if (!pAppCtx->disableVideo && playerGetTrackOfType(pAppCtx->hMediaInfo, BIP_MediaInfoTrackType_eVideo, &mediaInfoTrack) )
               {
                   playerSettings.videoTrackId = mediaInfoTrack.trackId;
                   playerSettings.videoTrackSettings.pidTypeSettings.video.codec = mediaInfoTrack.info.video.codec;
                   videoCodec = mediaInfoTrack.info.video.codec;
                   pAppCtx->maxHeight = mediaInfoTrack.info.video.height;
                   pAppCtx->maxWidth = mediaInfoTrack.info.video.width;
               }


               if (!pAppCtx->disableAudio)
               {
                   bool trackFound = false;

                   if (BIP_String_GetLength(pAppCtx->hLanguage)>1 || pAppCtx->ac3ServiceType != BIP_MediaInfoAudioAc3Bsmod_eMax)
                   {
                       playerSettings.pPreferredAudioLanguage = BIP_String_GetString(pAppCtx->hLanguage);
                       if (pAppCtx->ac3ServiceType != BIP_MediaInfoAudioAc3Bsmod_eMax)
                       {
                           playerSettings.ac3Descriptor.bsmod = pAppCtx->ac3ServiceType;
                           playerSettings.ac3Descriptor.bsmodValid = true;
                       }
                   }
                   else
                   {
                       trackFound = playerGetTrackOfType(pAppCtx->hMediaInfo, BIP_MediaInfoTrackType_eAudio, &mediaInfoTrack);
                       if ( trackFound)
                       {
                           playerSettings.audioTrackId = mediaInfoTrack.trackId;
                           playerSettings.audioTrackSettings.pidSettings.pidTypeSettings.audio.codec = mediaInfoTrack.info.audio.codec;
                           audioCodec = mediaInfoTrack.info.audio.codec;
                       }
                       else
                       {
                           BDBG_WRN((BIP_MSG_PRE_FMT "Not able to found any Audio track" BIP_MSG_PRE_ARG));
                       }
                   }
               }

               /* Make sure Probe found atleast a Video or an Audio Track. */
               BIP_CHECK_GOTO( (playerSettings.videoTrackId != UINT_MAX || playerSettings.audioTrackId != UINT_MAX), ( "Neither Audio nor Video Tracks found for URL=%s", BIP_String_GetString(pAppCtx->hUrl) ), error, bipStatus, bipStatus );
            }

#if NXCLIENT_SUPPORT
            {
                /* Once probing is done acquire video and audio decoders.*/
                NxClient_AllocSettings      allocSettings;
                NxClient_AllocResults       allocResults;
                NxClient_ConnectSettings    connectSettings;

                NxClient_GetDefaultAllocSettings(&allocSettings);
                allocSettings.simpleVideoDecoder = pAppCtx->disableVideo ? 0:1;
                allocSettings.simpleAudioDecoder = pAppCtx->disableAudio ? 0:1;
                rc = NxClient_Alloc( &allocSettings, &allocResults );
                BIP_CHECK_GOTO(( rc == NEXUS_SUCCESS ), ( "NxClient_Alloc Failed" ), error, BIP_INF_NEXUS_RESOURCE_NOT_AVAILABLE, bipStatus );

                NxClient_GetDefaultConnectSettings( &connectSettings );

                if (!pAppCtx->disableVideo && videoCodec != NEXUS_VideoCodec_eUnknown)
                {
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
                }
                if (!pAppCtx->disableAudio && (audioCodec != NEXUS_AudioCodec_eUnknown || BIP_String_GetLength(pAppCtx->hLanguage)>1 || pAppCtx->ac3ServiceType != BIP_MediaInfoAudioAc3Bsmod_eMax))
                {
                    connectSettings.simpleAudioDecoder.id = allocResults.simpleAudioDecoder.id;
                }

                /* Successfully connected to the NxServer. Acquire AV Decoder & Transcode Handles. */
                rc = NxClient_Connect(&connectSettings, &pAppCtx->connectId);
                BIP_CHECK_GOTO(( rc == NEXUS_SUCCESS ), ( "NxClient_Connect Failed" ), error, BIP_INF_NEXUS_RESOURCE_NOT_AVAILABLE, bipStatus );

                if (!pAppCtx->disableVideo && videoCodec != NEXUS_VideoCodec_eUnknown)
                {

                    hSimpleVideoDecoder = NEXUS_SimpleVideoDecoder_Acquire(allocResults.simpleVideoDecoder[0].id);
                    BIP_CHECK_GOTO(( hSimpleVideoDecoder ), ( "NEXUS_SimpleVideoDecoder_Acquire Failed" ), error, BIP_INF_NEXUS_RESOURCE_NOT_AVAILABLE, bipStatus );
                }

                if (!pAppCtx->disableAudio && (audioCodec != NEXUS_AudioCodec_eUnknown || BIP_String_GetLength(pAppCtx->hLanguage)>1 || pAppCtx->ac3ServiceType != BIP_MediaInfoAudioAc3Bsmod_eMax))
                {

                    hSimpleAudioDecoder = NEXUS_SimpleAudioDecoder_Acquire(allocResults.simpleAudioDecoder.id);
                    BIP_CHECK_GOTO(( hSimpleAudioDecoder ), ( "NEXUS_SimpleAudioDecoder_Acquire Failed" ), error, BIP_INF_NEXUS_RESOURCE_NOT_AVAILABLE, bipStatus );
                }

                if (!pAppCtx->disableTsm)
                {
                    hSimpleStcChannel = NEXUS_SimpleStcChannel_Create(NULL);
                    BIP_CHECK_GOTO(( hSimpleStcChannel ), ( "NEXUS_SimpleStcChannel_Create Failed" ), error, BIP_INF_NEXUS_RESOURCE_NOT_AVAILABLE, bipStatus );
                }

                /* We have successfully acquired all needed resources. */
            }

            /* Now set the player setting.*/
            playerSettings.audioTrackSettings.pidTypeSettings.audio.simpleDecoder = hSimpleAudioDecoder;
            playerSettings.videoTrackSettings.pidTypeSettings.video.simpleDecoder = hSimpleVideoDecoder;
            playerSettings.playbackSettings.simpleStcChannel = hSimpleStcChannel;
            if (pAppCtx->enableAudioPrimer)
            {
                playerSettings.audioConnectId = pAppCtx->connectId;
                prepareSettings.enableAudioPrimer = true;
            }

#else /* NXCLIENT_SUPPORT */
            /* Now set the player setting */
            if (!pAppCtx->disableAudio && (audioCodec != NEXUS_AudioCodec_eUnknown || BIP_String_GetLength(pAppCtx->hLanguage)>1 || pAppCtx->ac3ServiceType != BIP_MediaInfoAudioAc3Bsmod_eMax))
            {
                playerSettings.audioTrackSettings.pidTypeSettings.audio.primary = pcmDecoder;
            }
            if (!pAppCtx->disableVideo && videoCodec != NEXUS_VideoCodec_eUnknown)
            {
                playerSettings.videoTrackSettings.pidTypeSettings.video.decoder = videoDecoder;
                playerSettings.hDisplay = display;                        /* Needed for clock recovery setup of live ip stream. */
                playerSettings.hWindow = window;
            }
            playerSettings.playbackSettings.stcChannel = stcChannel;
#endif /* NXCLIENT_SUPPORT */

            playerSettings.playbackSettings.errorCallback.callback = playbackDoneCallbackFromBIP;
            playerSettings.playbackSettings.errorCallback.context = pAppCtx;
            if (pAppCtx->enableTimeshifting || pAppCtx->playerStreamInfo.dataAvailabilityModel == BIP_PlayerDataAvailabilityModel_eLimitedRandomAccess)
            {
                playerSettings.playbackSettings.endOfStreamAction = NEXUS_PlaybackLoopMode_ePlay;
                playerSettings.playbackSettings.beginningOfStreamAction = NEXUS_PlaybackLoopMode_ePlay;
                playerSettings.playbackSettings.playErrorHandling = NEXUS_PlaybackErrorHandlingMode_eAbort;
                prepareSettings.pauseTimeoutAction = NEXUS_PlaybackLoopMode_ePlay;
                prepareSettings.timeshiftBufferMaxDurationInMs = pAppCtx->timeshiftBufferMaxDurationInMs;
            }
            else if (pAppCtx->enableContinousPlay)
            {
                playerSettings.playbackSettings.endOfStreamAction = NEXUS_PlaybackLoopMode_eLoop;
                playerSettings.playbackSettings.beginningOfStreamAction = NEXUS_PlaybackLoopMode_eLoop;
            }
            else
            {
                playerSettings.playbackSettings.endOfStreamAction = NEXUS_PlaybackLoopMode_ePause;
                playerSettings.playbackSettings.endOfStreamCallback.callback = playbackDoneCallbackFromBIP;
                playerSettings.playbackSettings.endOfStreamCallback.context = pAppCtx;
                playerSettings.playbackSettings.beginningOfStreamAction = NEXUS_PlaybackLoopMode_ePlay;
            }
            playerSettings.playbackSettings.startPaused = pAppCtx->startPaused;
            playerSettings.clockRecoveryMode = pAppCtx->clockRecoveryMode;
            if (pAppCtx->usePlaypump)
            {
                pAppCtx->playerStreamInfo.usePlaypump = true;
            }
            playerSettings.enableDynamicTrackSelection = !pAppCtx->disableDynamicTrackSelection;
            bipStatus = BIP_Player_Prepare(hPlayer, &prepareSettings, &playerSettings, NULL/*&probeSettings*/, &pAppCtx->playerStreamInfo, &prepareStatus);
            BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_Player_Prepare Failed: URL=%s", BIP_String_GetString(pAppCtx->hUrl) ), error, bipStatus, bipStatus );

            audioPidChannel = prepareStatus.hAudioPidChannel;
            audioCodec = prepareStatus.audioCodec;
            videoPidChannel = prepareStatus.hVideoPidChannel;
            videoCodec = prepareStatus.videoCodec;
        }

        {
            /* GetDefault setting here since some of the start settings will be modified by next videoStart and audioStart section. */
            BIP_Player_GetDefaultStartSettings(&startSettings);
        }

        /* Set up & start video decoder. */
#if NXCLIENT_SUPPORT
        {
            if (pAppCtx->enableLowLatencyMode)
            {
                BDBG_LOG(("WARNING: Not able to enable/disable precisionLipsync in nxclient mode (check nxserver code)"));
            }
            if (!pAppCtx->disableTsm && pAppCtx->stcSyncMode != -1 && pAppCtx->enableLowLatencyMode)
            {
                NEXUS_SimpleStcChannelSettings stcSettings;
                NEXUS_SimpleStcChannel_GetSettings(hSimpleStcChannel, &stcSettings);
                stcSettings.sync = pAppCtx->stcSyncMode;
                BDBG_LOG(("LowLatencyMode: stcSettings.sync=%d",stcSettings.sync));
                rc = NEXUS_SimpleStcChannel_SetSettings(hSimpleStcChannel, &stcSettings);
                BIP_CHECK_GOTO(( rc == NEXUS_SUCCESS ), ( "NEXUS_SimpleStcChannel_SetSettings Failed" ), error, BIP_ERR_NEXUS, bipStatus );
            }

            if (!pAppCtx->disableVideo && videoPidChannel)
            {
                NEXUS_SimpleVideoDecoderStartSettings videoProgram;
                rc = NEXUS_SimpleVideoDecoder_SetStcChannel(hSimpleVideoDecoder, hSimpleStcChannel);
                BIP_CHECK_GOTO(( rc == NEXUS_SUCCESS ), ( "NEXUS_SimpleVideoDecoder_SetStcChannel Failed" ), error, BIP_ERR_NEXUS, bipStatus );

                NEXUS_SimpleVideoDecoder_GetDefaultStartSettings(&videoProgram);
                videoProgram.displayEnabled = true;
                videoProgram.settings.codec = videoCodec;
                videoProgram.settings.pidChannel = videoPidChannel;
                videoProgram.maxWidth = pAppCtx->maxWidth;
                videoProgram.maxHeight = pAppCtx->maxHeight;
                rc = NEXUS_SimpleVideoDecoder_Start(hSimpleVideoDecoder, &videoProgram);
                BIP_CHECK_GOTO(( rc == NEXUS_SUCCESS ), ( "NEXUS_SimpleVideoDecoder_Start Failed" ), error, BIP_ERR_NEXUS, bipStatus );
                startSettings.simpleVideoStartSettings = videoProgram;
                BDBG_MSG(("Started Video Decoder..."));
            }
            /* Set up & start audio decoder. */
            if (!pAppCtx->disableAudio && audioPidChannel)
            {
                NEXUS_SimpleAudioDecoderStartSettings audioProgram;

                rc = NEXUS_SimpleAudioDecoder_SetStcChannel(hSimpleAudioDecoder, hSimpleStcChannel);
                BIP_CHECK_GOTO(( rc == NEXUS_SUCCESS ), ( "NEXUS_SimpleAudioDecoder_SetStcChannel Failed" ), error, BIP_ERR_NEXUS, bipStatus );

                NEXUS_SimpleAudioDecoder_GetDefaultStartSettings(&audioProgram);
                audioProgram.primary.codec = audioCodec;
                audioProgram.primary.pidChannel = audioPidChannel;
                if (pAppCtx->enableAudioPrimer)
                {
                    audioProgram.primer.compressed = true;
                    audioProgram.primer.pcm = true;
                }
                if (pAppCtx->enableLowLatencyMode)
                {
                    audioProgram.primary.latencyMode = pAppCtx->audioDecoderLatencyMode;
                    BDBG_LOG(("NEXUS_AudioDecoderLatencyMode=%d", pAppCtx->audioDecoderLatencyMode));
                }
                rc = NEXUS_SimpleAudioDecoder_Start(hSimpleAudioDecoder, &audioProgram);
                BIP_CHECK_GOTO(( rc == NEXUS_SUCCESS ), ( "NEXUS_SimpleAudioDecoder_Start Failed" ), error, BIP_ERR_NEXUS, bipStatus );
                startSettings.simpleAudioStartSettings = audioProgram;
                BDBG_MSG(("Started Audio Decoder..."));
            }
        }
#else /* NXCLIENT_SUPPORT */
        {
#if NEXUS_HAS_SYNC_CHANNEL
            /* Connect sync channel for AV Lipsync. */
            {
                NEXUS_SyncChannel_GetSettings(syncChannel, &syncChannelSettings);
                if (!pAppCtx->disableVideo)
                {
                    syncChannelSettings.videoInput = NEXUS_VideoDecoder_GetConnector(videoDecoder);
                }
                if (!pAppCtx->disableAudio && audioCodec != NEXUS_AudioCodec_eUnknown)
                {
                    syncChannelSettings.audioInput[0] = NEXUS_AudioDecoder_GetConnector(pcmDecoder, NEXUS_AudioDecoderConnectorType_eStereo);
                }
                NEXUS_SyncChannel_SetSettings(syncChannel, &syncChannelSettings);
            }
#endif
            /* Setup & start Video Decoder */
            if (!pAppCtx->disableVideo && videoPidChannel)
            {
                NEXUS_VideoDecoderSettings  videoDecoderSettings;
                NEXUS_VideoDecoderStartSettings videoProgram;

                NEXUS_VideoDecoder_GetSettings(videoDecoder, &videoDecoderSettings);

                if(pAppCtx->maxWidth && pAppCtx->maxHeight)
                {
                    videoDecoderSettings.maxWidth = pAppCtx->maxWidth;
                    videoDecoderSettings.maxHeight = pAppCtx->maxHeight;
                    BDBG_MSG(("Video decoder width %d, height %d", pAppCtx->maxWidth, pAppCtx->maxHeight));
                }
                /* This is done since in some cases probing may not provide video width and height. In that case for HEVC we force the max Height and max Width.*/
                else if((NEXUS_VideoCodec_eH265 == videoCodec) || (NEXUS_VideoCodec_eVp9 == videoCodec))
                {
                    videoDecoderSettings.maxWidth = pAppCtx->maxWidth = 3840;
                    videoDecoderSettings.maxHeight = pAppCtx->maxHeight = 2160;
                    BDBG_MSG(("Video decoder width %d, height %d", pAppCtx->maxWidth, pAppCtx->maxHeight));
                }

                rc = NEXUS_VideoDecoder_SetSettings(videoDecoder, &videoDecoderSettings);
                BIP_CHECK_GOTO(( rc == NEXUS_SUCCESS ), ( "NEXUS_VideoDecoder_SetSettings Failed" ), error, BIP_ERR_NEXUS, bipStatus );
                BDBG_MSG(("Video Decoder settings are modified for IP \n"));

                /* connect video decoder to display */
                NEXUS_VideoWindow_AddInput(window, NEXUS_VideoDecoder_GetConnector(videoDecoder));

                NEXUS_VideoDecoder_GetDefaultStartSettings(&videoProgram);
                videoProgram.codec = videoCodec;
                videoProgram.pidChannel = videoPidChannel;
                videoProgram.stcChannel = stcChannel;
                rc = NEXUS_VideoDecoder_Start(videoDecoder, &videoProgram);
                BIP_CHECK_GOTO(( rc == NEXUS_SUCCESS ), ( "NEXUS_VideoDecoder_Start Failed" ), error, BIP_ERR_NEXUS, bipStatus );
                BDBG_MSG(("Video Decoder is Started\n"));
                startSettings.videoStartSettings = videoProgram;
            }

            /* Setup & start Audio decoder. */
            if (!pAppCtx->disableAudio && audioPidChannel)
            {
                NEXUS_AudioDecoderStartSettings audioProgram;

#if NEXUS_NUM_HDMI_OUTPUTS
                NEXUS_AudioOutput_AddInput( NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0] ), NEXUS_AudioDecoder_GetConnector(pcmDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
#endif

                NEXUS_AudioDecoder_GetDefaultStartSettings(&audioProgram);
                audioProgram.codec = audioCodec;
                audioProgram.pidChannel = audioPidChannel;
                audioProgram.stcChannel = stcChannel;
                if (pAppCtx->enableLowLatencyMode)
                {
                    audioProgram.latencyMode = pAppCtx->audioDecoderLatencyMode;
                    BDBG_LOG(("NEXUS_AudioDecoderLatencyMode=%d", pAppCtx->audioDecoderLatencyMode));
                }
                rc = NEXUS_AudioDecoder_Start(pcmDecoder, &audioProgram);
                BIP_CHECK_GOTO(( rc == NEXUS_SUCCESS ), ( "NEXUS_AudioDecoder_Start Failed" ), error, BIP_ERR_NEXUS, bipStatus );
                BDBG_MSG(("Audio Decoder is Started\n"));
                startSettings.audioStartSettings = audioProgram;

#if 0
                if (pAppCtx->enableLowLatencyMode)
                {
                    /* TODO: need to stop and restart for TSM to get the low delay number from the audio decoder */
                    BDBG_LOG(("Stopping and Re-starting audio decoder for low delay time to get passed to TSM \n"));
                    NEXUS_AudioDecoder_Stop(pcmDecoder);
                    rc = NEXUS_AudioDecoder_Start(pcmDecoder, &audioProgram);
                    BIP_CHECK_GOTO(( rc == NEXUS_SUCCESS ), ( "NEXUS_AudioDecoder_Start Failed" ), error, BIP_ERR_NEXUS, bipStatus );
                    BDBG_MSG(("Audio Decoder is Started\n"));
                }
#endif
            }
        }
#endif /* NXCLIENT_SUPPORT */

#if NXCLIENT_SUPPORT
        if (0)
        {
            NEXUS_VideoDecoderTrickState videoDecoderTrickSettings;
            NEXUS_SimpleVideoDecoder_GetTrickState(hSimpleVideoDecoder, &videoDecoderTrickSettings);
            videoDecoderTrickSettings.decodeMode = NEXUS_VideoDecoderDecodeMode_eI;
            videoDecoderTrickSettings.tsmEnabled = NEXUS_TsmMode_eDisabled;
            videoDecoderTrickSettings.hostTrickModesEnabled = true;
            videoDecoderTrickSettings.tsmEnabled = NEXUS_TsmMode_eSimulated;
            videoDecoderTrickSettings.stcTrickEnabled = false;
            videoDecoderTrickSettings.rate = (atoi(getenv("speed"))*NEXUS_NORMAL_DECODE_RATE);
            BDBG_WRN(("rate = %d", videoDecoderTrickSettings.rate));
            BDBG_ASSERT(NEXUS_SimpleVideoDecoder_SetTrickState(hSimpleVideoDecoder, &videoDecoderTrickSettings) == NEXUS_SUCCESS);
        }
#endif
        /* NEXUS Setup is complete, now Prepare & start BIP_Player. */
        {
            startSettings.timePositionInMs = pAppCtx->initialPlaybackPositionInMs;
            bipStatus = BIP_Player_Start(hPlayer, &startSettings);
            BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_Player_Start Failed: URL=%s", BIP_String_GetString(pAppCtx->hUrl) ), error, bipStatus, bipStatus );
        }

        if (pAppCtx->startPaused && pAppCtx->enableAutoPlayAfterStartingPaused)
        {
            bipStatus = BIP_Player_Play(hPlayer);
            BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS || bipStatus == BIP_INF_NOT_AVAILABLE), ( "BIP_Player_Play Failed: URL=%s", BIP_String_GetString(pAppCtx->hUrl) ), error, bipStatus, bipStatus );
            if (bipStatus == BIP_SUCCESS) BDBG_WRN(("Playback Resumed from startPaused!"));
            BIP_Player_PrintStatus(hPlayer);
        }
        if (pAppCtx->stressTrickModes)
        {
            BDBG_WRN(("Running trickmode stress test"));
            bipStatus = stressTrickModes(pAppCtx);
            BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "stressTrickModes Failed: URL=%s", BIP_String_GetString(pAppCtx->hUrl) ), error, bipStatus, bipStatus );
        }
        printf("player>\n");
        do
        {
            bipStatus = runTimeCmdParsing(pAppCtx, &cmdOptions);
            if ( bipStatus != BIP_SUCCESS ) goto error;

            if ( cmdOptions.cmd == PlayerCmd_eSeek)
            {
                BDBG_WRN(("Starting Seek to %d ms", cmdOptions.seekPositionInMs));
                bipStatus = BIP_Player_Seek(hPlayer, cmdOptions.seekPositionInMs);
                BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS || bipStatus == BIP_INF_NOT_AVAILABLE), ( "BIP_Player_Seek Failed: URL=%s", BIP_String_GetString(pAppCtx->hUrl) ), error, bipStatus, bipStatus );
                if (bipStatus == BIP_SUCCESS) BDBG_WRN(("Seeked to %d ms", cmdOptions.seekPositionInMs));
            }
            else if ( cmdOptions.cmd == PlayerCmd_eRelativeSeekFwd || cmdOptions.cmd == PlayerCmd_eRelativeSeekRev)
            {
                BIP_PlayerStatus playerStatus;

                bipStatus = BIP_Player_GetStatus(hPlayer, &playerStatus);
                BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_Player_Status Failed: URL=%s", BIP_String_GetString(pAppCtx->hUrl) ), error, bipStatus, bipStatus );
                if ( cmdOptions.cmd == PlayerCmd_eRelativeSeekFwd)
                {
                    cmdOptions.seekPositionInMs = playerStatus.currentPositionInMs + pAppCtx->jumpOffsetInMs;
                    BDBG_WRN(("Starting Seek forward from currentPositionInMs=%lu ms by offset=%u, seekedPosition=%d", playerStatus.currentPositionInMs, pAppCtx->jumpOffsetInMs, cmdOptions.seekPositionInMs));
                }
                else
                {
                    cmdOptions.seekPositionInMs = playerStatus.currentPositionInMs > pAppCtx->jumpOffsetInMs ?  playerStatus.currentPositionInMs - pAppCtx->jumpOffsetInMs : 0;
                    BDBG_WRN(("Starting Seek backward from currentPositionInMs=%lu ms by offset=%u, seekedPosition=%d", playerStatus.currentPositionInMs, pAppCtx->jumpOffsetInMs, cmdOptions.seekPositionInMs));
                }
                bipStatus = BIP_Player_Seek(hPlayer, cmdOptions.seekPositionInMs);
                BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS || bipStatus == BIP_INF_NOT_AVAILABLE), ( "BIP_Player_Seek Failed: URL=%s", BIP_String_GetString(pAppCtx->hUrl) ), error, bipStatus, bipStatus );
                if (bipStatus == BIP_SUCCESS) BDBG_WRN(("Seeked to %d ms", cmdOptions.seekPositionInMs));
            }
            else if ( cmdOptions.cmd == PlayerCmd_ePause)
            {
                BDBG_WRN(("Pausing Playback!"));
                bipStatus = BIP_Player_Pause(hPlayer, NULL);
                BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS || bipStatus == BIP_INF_NOT_AVAILABLE || bipStatus == BIP_INF_PLAYER_CANT_PAUSE), ( "BIP_Player_Pause Failed: URL=%s", BIP_String_GetString(pAppCtx->hUrl) ), error, bipStatus, bipStatus );
                if (bipStatus == BIP_SUCCESS) BDBG_WRN(("Paused Playback!"));
                BIP_Player_PrintStatus(hPlayer);
            }
            else if ( cmdOptions.cmd == PlayerCmd_ePlay)
            {
                BDBG_WRN(("Resuming Playback!"));
                bipStatus = BIP_Player_Play(hPlayer);
                BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS || bipStatus == BIP_INF_NOT_AVAILABLE), ( "BIP_Player_Play Failed: URL=%s", BIP_String_GetString(pAppCtx->hUrl) ), error, bipStatus, bipStatus );
                if (bipStatus == BIP_SUCCESS) BDBG_WRN(("Playback Resumed!"));
                BIP_Player_PrintStatus(hPlayer);
            }
            else if ( cmdOptions.cmd == PlayerCmd_ePlayAtRate)
            {
                BIP_PlayerPlayAtRateSettings settings;

                BIP_Player_GetDefaultPlayAtRateSettings(&settings);
                BDBG_WRN(("Playing at Rate=%s", cmdOptions.playSpeed));
                if (pAppCtx->usePlaypump)
                {
                    settings.playRateMethod = BIP_PlayerPlayRateMethod_eUsePlaySpeed; /* Note: this enables PBIP to I-frame based trickmodes, similar to server side trickmodes. */
                }
                bipStatus = BIP_Player_PlayAtRateAsString(hPlayer, cmdOptions.playSpeed, &settings);
                BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS || bipStatus == BIP_INF_NOT_AVAILABLE), ( "BIP_Player_PlayAtRateAsString Failed: URL=%s", BIP_String_GetString(pAppCtx->hUrl) ), error, bipStatus, bipStatus );
                if (bipStatus == BIP_SUCCESS) BDBG_WRN(("Started Playing at Rate=%s", cmdOptions.playSpeed));
                BIP_Player_PrintStatus(hPlayer);
            }
            else if(cmdOptions.cmd == PlayerCmd_ePlayAtRawRate)
            {
                BIP_PlayerPlayAtRateSettings settings;

                BIP_Player_GetDefaultPlayAtRateSettings(&settings);
                if (pAppCtx->usePlaypump)
                {
                    settings.playRateMethod = BIP_PlayerPlayRateMethod_eUsePlaySpeed;
                }
                bipStatus = BIP_Player_PlayAtRate(hPlayer, cmdOptions.rate, &settings);
                BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS || bipStatus == BIP_INF_NOT_AVAILABLE), ( "BIP_Player_PlayAtRate Failed: URL=%s", BIP_String_GetString(pAppCtx->hUrl) ), error, bipStatus, bipStatus );
                if (bipStatus == BIP_SUCCESS) BDBG_WRN(("Started Playing at Rate=%d", cmdOptions.rate));
                BIP_Player_PrintStatus(hPlayer);
            }
            else if(cmdOptions.cmd == PlayerCmd_ePlayLanguageSpecificTrack)
            {
                 BIP_PlayerSettings          playerSettings;

                 BIP_Player_GetSettings( hPlayer, &playerSettings );
                 playerSettings.audioTrackId = UINT_MAX;
                 playerSettings.pPreferredAudioLanguage = BIP_String_GetString(pAppCtx->hLanguage);
                 bipStatus = BIP_Player_SetSettings( hPlayer, &playerSettings );
                 BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_Player_SetSettings Failed: for language=%s", BIP_String_GetString(pAppCtx->hLanguage) ), error, bipStatus, bipStatus );
            }
            else if(cmdOptions.cmd == PlayerCmd_ePlayBsmodSpecificTrack)
            {
                 BIP_PlayerSettings          playerSettings;
                 BIP_Player_GetSettings( hPlayer, &playerSettings );

                 BDBG_MSG(("New ac3 service type bsmod=%d", pAppCtx->ac3ServiceType));
                 BIP_Player_GetSettings( hPlayer, &playerSettings );
                 playerSettings.audioTrackId = UINT_MAX;
                 playerSettings.ac3Descriptor.bsmod = pAppCtx->ac3ServiceType;
                 playerSettings.ac3Descriptor.bsmodValid = true;
                 bipStatus = BIP_Player_SetSettings( hPlayer, &playerSettings );
                 BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_Player_SetSettings Failed: for ac3 service type bsmod=%d", pAppCtx->ac3ServiceType ), error, bipStatus, bipStatus );
            }
            else if (cmdOptions.cmd == PlayerCmd_ePlayNewUrl)
            {
                BDBG_WRN(("Changing channel to %s", BIP_String_GetString(pAppCtx->hUrl)));
            }
            else if ( cmdOptions.cmd == PlayerCmd_eFrameAdvance || cmdOptions.cmd == PlayerCmd_eFrameReverse )
            {
                BDBG_WRN(("Calling Frame %s", cmdOptions.cmd == PlayerCmd_eFrameAdvance?"Advance":"Reverse" ));
                bipStatus = BIP_Player_PlayByFrame(hPlayer, cmdOptions.cmd == PlayerCmd_eFrameAdvance? true : false );
                BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_Player_PlayByFrame Failed: URL=%s", BIP_String_GetString(pAppCtx->hUrl) ), error, bipStatus, bipStatus );
                if (bipStatus == BIP_SUCCESS) BDBG_WRN(("Played 1 frame!"));
                BIP_Player_PrintStatus(hPlayer);
            }
            else if (cmdOptions.cmd == PlayerCmd_eStressTrickModes)
            {
                BDBG_WRN(("Running trickmode stress test"));
                bipStatus = stressTrickModes(pAppCtx);
                BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "stressTrickModes Failed: URL=%s", BIP_String_GetString(pAppCtx->hUrl) ), error, bipStatus, bipStatus );
            }
            if ( pAppCtx->printStatus || cmdOptions.cmd == PlayerCmd_ePrintStatus)
            {
                BIP_PlayerStatus playerStatus;
                BIP_Player_PrintStatus(hPlayer);
                bipStatus = BIP_Player_GetStatus(hPlayer, &playerStatus);
                BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS || bipStatus == BIP_INF_NOT_AVAILABLE), ( "BIP_Player_Status Failed: URL=%s", BIP_String_GetString(pAppCtx->hUrl) ), error, bipStatus, bipStatus );
                if (bipStatus == BIP_SUCCESS) BDBG_WRN(("Player position cur/last: %0.3f/%0.3f sec ", playerStatus.currentPositionInMs/1000., playerStatus.lastPositionInMs/1000. ));
            }
            else if ( pAppCtx->printServerStatus || cmdOptions.cmd == PlayerCmd_ePrintServerStatus)
            {
                BIP_PlayerStatusFromServer serverStatus;
                BIP_PlayerGetStatusFromServerSettings settings;
                BIP_Player_GetDefaultGetStatusFromServerSettings(&settings);
                settings.getAvailableSeekRange = true;
                settings.timeoutInMs = 5000;
                bipStatus = BIP_Player_GetStatusFromServer(hPlayer, &settings, &serverStatus);
                BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS || bipStatus == BIP_INF_NOT_AVAILABLE), ( "BIP_Player_StatusFromServer Failed: URL=%s", BIP_String_GetString(pAppCtx->hUrl) ), error, bipStatus, bipStatus );
                BDBG_WRN(("Player position cur/last: %0.3f/%0.3f sec ", serverStatus.currentPositionInMs/1000., serverStatus.availableSeekRange.lastPositionInMs/1000. ));
                BIP_Player_PrintStatus(hPlayer);
            }
            else
            {
                if (cmdOptions.cmd != PlayerCmd_eMax) BDBG_WRN(("Not yet handling this cmd=%d", cmdOptions.cmd));
            }
        } while ( cmdOptions.cmd != PlayerCmd_eQuit && cmdOptions.cmd != PlayerCmd_ePlayNewUrl && pAppCtx->playbackDone == false);
        bipStatus = BIP_SUCCESS;

        /* Once player started reset the following flags, since runtime one can again select another language or services.*/
        /* New channel if required will set it run time.*/
        pAppCtx->ac3ServiceType = BIP_MediaInfoAudioAc3Bsmod_eMax;


error:
        /* Stop the Player & Decoders. */
        {
            if (hPlayer) BIP_Player_Stop(hPlayer);
#if NXCLIENT_SUPPORT
            if(hSimpleAudioDecoder)
            {
                NEXUS_SimpleAudioDecoder_Stop(hSimpleAudioDecoder);
                NEXUS_SimpleAudioDecoder_SetStcChannel(hSimpleAudioDecoder, NULL);
            }

            if(hSimpleVideoDecoder)
            {
                NEXUS_SimpleVideoDecoder_Stop(hSimpleVideoDecoder);
                NEXUS_SimpleVideoDecoder_SetStcChannel(hSimpleVideoDecoder, NULL);
            }
#else /* NXCLIENT_SUPPORT */
            if (pcmDecoder) NEXUS_AudioDecoder_Stop(pcmDecoder);
            if (videoDecoder) NEXUS_VideoDecoder_Stop(videoDecoder);
#if NEXUS_NUM_HDMI_OUTPUTS
            NEXUS_AudioOutput_RemoveAllInputs(NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]));
#endif

#endif /* NXCLIENT_SUPPORT */
        }

        /* Close the Resources. */
        {
            if (hPlayer) BIP_Player_Disconnect(hPlayer);

#if NXCLIENT_SUPPORT
            if(hSimpleVideoDecoder) NEXUS_SimpleVideoDecoder_Release(hSimpleVideoDecoder);
            if(hSimpleAudioDecoder)NEXUS_SimpleAudioDecoder_Release(hSimpleAudioDecoder);
            if(hSimpleStcChannel)NEXUS_SimpleStcChannel_Destroy(hSimpleStcChannel);
#endif /* NXCLIENT_SUPPORT */
        }
    } while ( cmdOptions.cmd == PlayerCmd_ePlayNewUrl && pAppCtx->playbackDone == false );
    /* End of External while loop*/

#ifndef NXCLIENT_SUPPORT
#if NEXUS_HAS_SYNC_CHANNEL
    if (syncChannel) NEXUS_SyncChannel_Destroy(syncChannel);
#endif
    if (pcmDecoder) NEXUS_AudioDecoder_Close(pcmDecoder);
    if (window) NEXUS_VideoWindow_RemoveAllInputs(window);
    if (videoDecoder) NEXUS_VideoInput_Shutdown(NEXUS_VideoDecoder_GetConnector(videoDecoder));
    if (window) NEXUS_VideoWindow_Close(window);
    if (display) NEXUS_Display_Close(display);
    if (videoDecoder) NEXUS_VideoDecoder_Close(videoDecoder);
    if (stcChannel) NEXUS_StcChannel_Close(stcChannel);
#endif

    if (hPlayer) BIP_Player_Destroy(hPlayer);
    if (pAppCtx->hCompletionEvent) B_Event_Destroy(pAppCtx->hCompletionEvent);

errorParseOptions:
    unInitAppCtx( pAppCtx );
    BIP_DtcpIpClientFactory_Uninit();
#if NXCLIENT_SUPPORT
    NxClient_Uninit();
#else /* !NXCLIENT_SUPPORT */
    NEXUS_Platform_Uninit();
#endif /* NXCLIENT_SUPPORT */

    BIP_SslClientFactory_Uninit();
errorBipInit:
    BIP_Uninit();
    return (bipStatus);
}
