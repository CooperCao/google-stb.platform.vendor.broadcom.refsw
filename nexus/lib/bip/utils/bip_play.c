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
******************************************************************************/

#include "bip.h"
#include "bip_play.h"
#include <sys/time.h>

BDBG_MODULE(bip_play);

static bool BIP_Play_GetTrackOfType(
    BIP_MediaInfoHandle hMediaInfo,
    BIP_MediaInfoTrackType trackType,
    BIP_MediaInfoTrack *pMediaInfoTrackOut
    );
static bool BIP_Play_GetSpecialAudioTrackType(
        BIP_MediaInfoHandle             hMediaInfo,
        BIP_MediaInfoTrack              *pMediaInfoTrackOut,
        const char                      *language,
        unsigned                        ac3ServiceType
    );
static BIP_Status BIP_Play_NexusResourcesInit(BIP_Play_Context *pCtx);
static void BIP_Play_NexusResourcesUninit(BIP_Play_Context *pCtx);
static BIP_Status BIP_Play_UpdateCodecInfo(
        BIP_Play_Context *pCtx, /* In */
        BIP_PlayerSettings *pPlayerSettings /* In/Out */
    );
static void BIP_Play_UpdatePlayerSettings(
    BIP_Play_Context* pCtx, /* In */
    BIP_PlayerSettings *pSettings /* Out */
    );
static BIP_Status BIP_Play_SetupNexusAVDecoders(
        BIP_Play_Context *pCtx
    );
static void BIP_Play_DoneCallbackFromBIP(
    void *context,
    int   param
    );
BIP_Status BIP_Play_WaitForCommand(
        char *pBuf,
        int timeout
    );
static BIP_Status BIP_Play_HandleRuntimeCmd(
        BIP_Play_Context *pCtx,
        BIP_Play_ParsedRuntimeCmdInfo *pCmd
    );
static void BIP_Play_HandleRemoteKey(
        BIP_Play_Context *pCtx, /* In */
        b_remote_key key, /* In */
        BIP_Play_ParsedRuntimeCmdInfo *pCmd /* Out */
    );
static void BIP_Play_CheckAndPrintStatus(
        BIP_Play_Context *pCtx /* In */
    );
static void BIP_Play_PrintCommandCompletionStatus(
        BIP_Play_ParsedRuntimeCmdInfo *pCmd,
        BIP_Status bipStatus
    );
static float BIP_Play_FractToFloat(char *str);

int main (int argc, char **argv)
{
    BIP_Play_Context *pCtx;
    BIP_Status bipStatus = BIP_ERR_INTERNAL;
    BIP_SslClientFactoryInitSettings sslSettings;
    BIP_Play_ParsedRuntimeCmdInfo runtimeCmd;
    NEXUS_Error rc;
#if NXCLIENT_SUPPORT
    NxClient_JoinSettings joinSettings;
#else
    NEXUS_PlatformSettings platformSettings;
#endif
#ifdef B_HAS_DTCP_IP
    BIP_DtcpIpClientFactoryInitSettings dtcpSettings;
#endif

    /* Print usage */
    if ((argc == 1) || (strcmp(argv[1], "-help") == 0))
    {
        BIP_Play_PrintUsage(argv[0]);
        goto errorBipPlayInit;
    }

#if NXCLIENT_SUPPORT
    /* Join nxserver */
    NxClient_GetDefaultJoinSettings(&joinSettings);
    snprintf(joinSettings.name, NXCLIENT_MAX_NAME, "%s", argv[0]);
    rc = NxClient_Join(&joinSettings);
    BIP_CHECK_GOTO((rc == NEXUS_SUCCESS), ("NxClient_Join Failed"), errorNexusInit, BIP_ERR_INTERNAL, bipStatus);
#else /* NXCLIENT_SUPPORT */
    /* Initialize nexus platform */
    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    rc = NEXUS_Platform_Init(&platformSettings);
    BIP_CHECK_GOTO((rc == NEXUS_SUCCESS), ("NEXUS_Platform_Init Failed"), errorNexusInit, BIP_ERR_NEXUS, bipStatus);
#endif /* NXCLIENT_SUPPORT */
    BDBG_MSG((BIP_MSG_PRE_FMT "Nexus initialization : done" BIP_MSG_PRE_ARG));

    /* Initialize BIP */
    bipStatus = BIP_Init(NULL);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ("BIP_Init Failed"), errorBipInit, bipStatus, bipStatus);
    BDBG_MSG((BIP_MSG_PRE_FMT "BIP Initialization complete" BIP_MSG_PRE_ARG));

    /* Initialize BIP Play context */
    bipStatus = BIP_Play_Init(&pCtx);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ("BIP_Play_Init failed"), errorBipPlayInit, bipStatus, bipStatus);
    BDBG_MSG((BIP_MSG_PRE_FMT "BIP Play initialization complete" BIP_MSG_PRE_ARG));

    /* Parse command line arguments */
    bipStatus = BIP_Play_ParseOptions(argc, argv, &pCtx->options, &pCtx->hUrls[0], &pCtx->numUrls);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "Argument parsing Failed" ), errorOptionsParse, bipStatus, bipStatus);
    BDBG_MSG((BIP_MSG_PRE_FMT "Parsed command line options successfully" BIP_MSG_PRE_ARG));

    /* Initialize Nexus Resources */
    bipStatus = BIP_Play_NexusResourcesInit(pCtx);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ("BIP_Play_NexusResourcesInit Failed"), errorNexusResourcesInit, BIP_ERR_NEXUS, bipStatus);
    BDBG_MSG((BIP_MSG_PRE_FMT "Nexus resource allocation complete" BIP_MSG_PRE_ARG));

    /* Initialize DtcpIpClientFactory */
#ifdef B_HAS_DTCP_IP
    BIP_DtcpIpClientFactory_GetDefaultInitSettings(&dtcpSettings);
    dtcpSettings.keyFormat = pCtx->options.keyFormat;
    bipStatus = BIP_DtcpIpClientFactory_Init(&dtcpSettings);
    BIP_CHECK_GOTO(((bipStatus == BIP_SUCCESS) || (bipStatus == BIP_ERR_NOT_SUPPORTED)), ("BIP_DtcpIpClientFactory_Init Failed"), errorDtcpIpInit, bipStatus, bipStatus);
    BDBG_MSG((BIP_MSG_PRE_FMT "DTCP IP Client Factory Initialization complete" BIP_MSG_PRE_ARG));
#endif /* B_HAS_DTCP_IP */

    /* Initialize SslClientFactory */
    BIP_SslClientFactory_GetDefaultInitSettings(&sslSettings);
    sslSettings.pRootCaCertPath = TEST_ROOT_CA_PATH;
    bipStatus = BIP_SslClientFactory_Init(&sslSettings);
    BIP_CHECK_GOTO(((bipStatus == BIP_SUCCESS) || (bipStatus == BIP_ERR_NOT_SUPPORTED)), ("BIP_SslClientFactory_Init Failed"), errorSslInit, bipStatus, bipStatus);
    BDBG_MSG((BIP_MSG_PRE_FMT "SSL Client Factory initialization complete" BIP_MSG_PRE_ARG));

    /* Create BIP Player instance */
    pCtx->hPlayer = BIP_Player_Create(NULL);
    BIP_CHECK_GOTO((pCtx->hPlayer), ("BIP_Player_Create Failed"), errorSslInit, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus);
    BDBG_MSG((BIP_MSG_PRE_FMT "BIP Player created successfully" BIP_MSG_PRE_ARG));

    /* Disable Gui if command line option says so */
    if (pCtx->hGui)
    {
        BIP_Play_GuiSetAlpha(pCtx->hGui, pCtx->options.alpha);
        if (pCtx->options.disableGui)
            BIP_Play_GuiDisable(pCtx->hGui);
        else
            BIP_Play_GuiEnable(pCtx->hGui);
    }

    /* Initialize loop parameters */
    pCtx->playingNow = 0;
    runtimeCmd.command = BIP_Play_RuntimeCmd_eMax;
    do {
        BIP_PlayerConnectSettings playerConnectSettings;
        BIP_PlayerProbeMediaInfoSettings probeMediaInfoSettings;
        BIP_PlayerSettings playerSettings;
        BIP_PlayerPrepareStatus prepareStatus;
        BIP_PlayerPrepareSettings prepareSettings;

        /* Choose which URL to tune to (command line or run time url) */
        if (runtimeCmd.command == BIP_Play_RuntimeCmd_ePlayNewUrl)
            BIP_String_StrcpyCharN(pCtx->currentUrl, pCtx->lastCmd.strArg, MAX_RUNTIME_COMMAND_SIZE);
        else
            BIP_String_StrcpyBipString(pCtx->currentUrl, pCtx->hUrls[pCtx->playingNow]);

        /* Re-Initialize all parameter*/
        pCtx->videoPidChannel = NULL;
        pCtx->audioPidChannel = NULL;
        pCtx->audioCodec = NEXUS_AudioCodec_eUnknown;
        pCtx->videoCodec = NEXUS_VideoCodec_eUnknown;
        pCtx->playbackDone = false;

        /* Connect to Server & do Player protocol specific exchange. */
        BIP_Player_GetDefaultConnectSettings(&playerConnectSettings);
        playerConnectSettings.pUserAgent = "BIP Play Util";
        /* Set interface name if specified at command line */
        if (pCtx->options.interface)
            playerConnectSettings.pNetworkInterfaceName = pCtx->options.interface;
        /* Pass-in an additional header to request the seekable range from the server.
         * Some servers use it for indicating duration for both time-shifted & DVR streams. */
        playerConnectSettings.pAdditionalHeaders = "getAvailableSeekRange.dlna.org:1\r\n";
        if (pCtx->options.enableTimeShift)
            playerConnectSettings.dataAvailabilityModel = BIP_PlayerDataAvailabilityModel_eLimitedRandomAccess;

        /* Connect to server */
        bipStatus = BIP_Player_Connect(pCtx->hPlayer, BIP_String_GetString(pCtx->currentUrl), &playerConnectSettings);
        BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS),
            ("BIP_Player_Connect Failed to Connect to URL=%s", BIP_String_GetString(pCtx->currentUrl)), errorPlayerConnect, bipStatus, bipStatus);
        BDBG_MSG((BIP_MSG_PRE_FMT "Bip play connected to server" BIP_MSG_PRE_ARG));

        /* Probe media to find out Video and Audio decoder related information.*/
        BIP_Player_GetDefaultProbeMediaInfoSettings(&probeMediaInfoSettings);
        pCtx->hMediaInfo = NULL;
        probeMediaInfoSettings.enablePayloadScanning = pCtx->options.enablePayloadScanning;
        bipStatus = BIP_Player_ProbeMediaInfo(pCtx->hPlayer, &probeMediaInfoSettings, &pCtx->hMediaInfo);
        BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ("BIP_Player_Probe Failed!"), errorProbeMediaFailed, bipStatus, bipStatus);
        BDBG_MSG((BIP_MSG_PRE_FMT "Probing media info completed" BIP_MSG_PRE_ARG));

        /* Map the MediaStream Info to the Player's StreamInfo. */
        bipStatus = BIP_Player_GetProbedStreamInfo(pCtx->hPlayer, &pCtx->playerStreamInfo);
        BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ("BIP_Player_GetProbedStreamInfo Failed!"), errorProbeMediaFailed, bipStatus, bipStatus);

        /* Prepare the BIP Player. */
        BIP_Player_GetDefaultSettings(&playerSettings);
        bipStatus = BIP_Play_UpdateCodecInfo(pCtx, &playerSettings);
        BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ("Getting media codec info failed!"), errorProbeMediaFailed, bipStatus, bipStatus);

        /* Now set the player setting.*/
        BIP_Play_UpdatePlayerSettings(pCtx, &playerSettings);

        /* Prepare BIP Player */
        BIP_Player_GetDefaultPrepareSettings(&prepareSettings);
        if (pCtx->options.enableTimeShift ||
            (pCtx->playerStreamInfo.dataAvailabilityModel == BIP_PlayerDataAvailabilityModel_eLimitedRandomAccess))
        {
            prepareSettings.pauseTimeoutAction = NEXUS_PlaybackLoopMode_ePlay;
            prepareSettings.timeshiftBufferMaxDurationInMs = pCtx->options.timeShiftDuration;
        }
        bipStatus = BIP_Player_Prepare(pCtx->hPlayer, &prepareSettings, &playerSettings,
                                       NULL/*&probeSettings*/, &pCtx->playerStreamInfo, &prepareStatus);
        BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ("BIP_Player_Prepare Failed: URL=%s", BIP_String_GetString(pCtx->currentUrl)), errorPlayerPrepare, bipStatus, bipStatus);
        BDBG_MSG((BIP_MSG_PRE_FMT "Bip Player prepare complete" BIP_MSG_PRE_ARG));

        pCtx->audioPidChannel = prepareStatus.hAudioPidChannel;
        pCtx->videoPidChannel = prepareStatus.hVideoPidChannel;

        /* GetDefault setting here since some of the start settings will be modified by next videoStart and audioStart section. */
        BIP_Player_GetDefaultStartSettings(&pCtx->startSettings);

        /* Setup audio and video decoders */
        bipStatus = BIP_Play_SetupNexusAVDecoders(pCtx);
        BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ("Failed to Setup AV decoders"), errorSetupDecoder, bipStatus, bipStatus);
        BDBG_MSG((BIP_MSG_PRE_FMT "Nexus Audio/Video decoders initialized" BIP_MSG_PRE_ARG));

        /* NEXUS Setup is complete, now Prepare & start BIP_Player. */
        pCtx->startSettings.timePositionInMs = pCtx->options.initialPosition;
        bipStatus = BIP_Player_Start(pCtx->hPlayer, &pCtx->startSettings);
        BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ("BIP_Player_Start Failed: URL=%s", BIP_String_GetString(pCtx->currentUrl)),
                        errorPlayerStart, bipStatus, bipStatus);
        BDBG_MSG((BIP_MSG_PRE_FMT "BIP Player started!" BIP_MSG_PRE_ARG));

        if (pCtx->options.startPaused && pCtx->options.enableAutoPlayAfterStartingPaused)
        {
            bipStatus = BIP_Player_Play(pCtx->hPlayer);
            BIP_CHECK_GOTO(((bipStatus == BIP_SUCCESS) || (bipStatus == BIP_INF_NOT_AVAILABLE)), ("BIP_Player_Play Failed: URL=%s", BIP_String_GetString(pCtx->currentUrl)),
                            errorPlayerStart, bipStatus, bipStatus);
            if (bipStatus == BIP_SUCCESS)
                BDBG_WRN((BIP_MSG_PRE_FMT "Playback Resumed from startPaused!" BIP_MSG_PRE_ARG));
            BIP_Player_PrintStatus(pCtx->hPlayer);
        }

        /* Set playback rate */
        pCtx->rate = (pCtx->options.startPaused && (!pCtx->options.enableAutoPlayAfterStartingPaused)) ? 0.0 : 1.0;

        /* Initialize stream info in GUI */
        {
            BIP_Play_GuiStreamInfo streamInfo;

            memset(&streamInfo, 0, sizeof(streamInfo));
            streamInfo.currentPos = pCtx->startSettings.timePositionInMs;
            streamInfo.duration = pCtx->streamDurationInMs;
            streamInfo.rate = pCtx->rate;
            streamInfo.setPosition = pCtx->startSettings.timePositionInMs;
            streamInfo.vid = pCtx->videoCodec;
            streamInfo.vidWidth = pCtx->maxWidth;
            streamInfo.vidHeight = pCtx->maxHeight;
            streamInfo.aud = pCtx->audioCodec;
            strncpy(streamInfo.url, BIP_String_GetString(pCtx->currentUrl), MAX_TEXT_DATA_LEN);

            BIP_Play_GuiSetStreamInfo(pCtx->hGui, &streamInfo);
            BDBG_MSG((BIP_MSG_PRE_FMT "Initialized UI elements" BIP_MSG_PRE_ARG));
        }

        fprintf(stdout, "bip_play> ");
        fflush(stdout);

        /* Start Run time command parsing loop */
        runtimeCmd.command = BIP_Play_RuntimeCmd_eMax;
        do {
            char buffer[MAX_RUNTIME_COMMAND_SIZE];
            bool cmdReceived = false;

            /* Poll on keyboard/remote inputs */
            buffer[0] = '\0';
            bipStatus = BIP_Play_WaitForCommand(buffer, 10);
            if (bipStatus == BIP_SUCCESS)
            {
                bipStatus = BIP_Play_ParseRunTimeCmd(buffer, &runtimeCmd);

                if (bipStatus == BIP_SUCCESS)
                {
                    cmdReceived = true;
                    if (runtimeCmd.command != BIP_Play_RuntimeCmd_eNone)
                        printf("bip_play: Command received \"%s\"\n", buffer);
                }
                else
                {
                    BIP_Play_PrintRuntimeCmdUsage();
                }

                if (runtimeCmd.command != BIP_Play_RuntimeCmd_eQuit)
                {
                    fprintf(stdout, "bip_play> ");
                    fflush(stdout);
                }
            }
            else if (pCtx->input && (BERR_SUCCESS == binput_wait(pCtx->input, 10)))
            {
                b_remote_key key;
                bool repeat;

                /* Only if remote keys are enabled */
                if (!pCtx->options.disableRemoteKeys)
                {
                    /* Get remote key */
                    binput_read(pCtx->input, &key, &repeat);
                    if (!repeat)
                    {
                        /* Translate remote key to BIP_Play_ParsedRuntimeCmdInfo */
                        BIP_Play_HandleRemoteKey(pCtx, key, &runtimeCmd);
                        if (runtimeCmd.command != BIP_Play_RuntimeCmd_eMax)
                        {
                            printf("bip_play: Command received (from remote) \"%s\"\n",
                                   BIP_Play_LookupRuntimeCommand(runtimeCmd.command));
                            cmdReceived = true;
                        }
                    }
                }
            }

            /* A valid command is received */
            if (cmdReceived)
            {
                /* Process the run time command */
                bipStatus = BIP_Play_HandleRuntimeCmd(pCtx, &runtimeCmd);
                BIP_Play_PrintCommandCompletionStatus(&runtimeCmd, bipStatus);
                if (bipStatus != BIP_SUCCESS)
                    continue;
                /* Save last command */
                if (runtimeCmd.command != BIP_Play_RuntimeCmd_eLast)
                    pCtx->lastCmd = runtimeCmd;
            }

            BIP_Play_CheckAndPrintStatus(pCtx);
            /* Set playback done flag if we have gone passed the marker specified at command line */
            if ((unsigned)pCtx->playerStatus.currentPositionInMs > (unsigned)pCtx->options.playTime)
                pCtx->playbackDone = true;
        } while ((runtimeCmd.command != BIP_Play_RuntimeCmd_eQuit)
                 && (runtimeCmd.command != BIP_Play_RuntimeCmd_ePlayNewUrl)
                 && (runtimeCmd.command != BIP_Play_RuntimeCmd_eTune)
                 && (!pCtx->playbackDone));

        if (pCtx->playbackDone)
            BDBG_LOG((BIP_MSG_PRE_FMT "End of Stream playback..." BIP_MSG_PRE_ARG));
        else if (runtimeCmd.command == BIP_Play_RuntimeCmd_eQuit)
            BDBG_LOG(("User Quit, we are done..."));

        /* Once player started reset the following flags, since runtime one can again select another language or services.*/
        /* New channel if required will set it run time.*/
        pCtx->options.ac3ServiceType = UINT_MAX;

errorPlayerStart:
        BIP_Player_Stop(pCtx->hPlayer);

errorSetupDecoder:
#if NXCLIENT_SUPPORT
        NEXUS_SimpleAudioDecoder_Stop(pCtx->hSimpleAudioDecoder);
        NEXUS_SimpleAudioDecoder_SetStcChannel(pCtx->hSimpleAudioDecoder, NULL);
        NEXUS_SimpleVideoDecoder_Stop(pCtx->hSimpleVideoDecoder);
        NEXUS_SimpleVideoDecoder_SetStcChannel(pCtx->hSimpleVideoDecoder, NULL);
#else /* NXCLIENT_SUPPORT */
        NEXUS_AudioDecoder_Stop(pCtx->pcmDecoder);
        NEXUS_VideoDecoder_Stop(pCtx->videoDecoder);
#endif /* NXCLIENT_SUPPORT */

errorPlayerPrepare:
errorProbeMediaFailed:
errorPlayerConnect:
        BIP_Player_Disconnect(pCtx->hPlayer);
        BDBG_MSG((BIP_MSG_PRE_FMT "BIP Player disconnected" BIP_MSG_PRE_ARG));

        /* Increment track index only, if a new URL has NOT been specified at run time*/
        if ((runtimeCmd.command != BIP_Play_RuntimeCmd_ePlayNewUrl)
            && (runtimeCmd.command != BIP_Play_RuntimeCmd_eTune))
        {
            pCtx->playingNow++;
            pCtx->playingNow %= pCtx->numUrls;

            /* Playback of a URL completed - Move on to next, loop around or exit */
            if ((pCtx->playingNow == 0) && pCtx->options.loop)
                break;
        }
    } while (runtimeCmd.command != BIP_Play_RuntimeCmd_eQuit);

    BIP_Player_Destroy(pCtx->hPlayer);

errorSslInit:
    BIP_SslClientFactory_Uninit();

#ifdef B_HAS_DTCP_IP
errorDtcpIpInit:
    BIP_DtcpIpClientFactory_Uninit();
#endif

errorNexusResourcesInit:
    BIP_Play_NexusResourcesUninit(pCtx);

errorOptionsParse:
errorBipPlayInit:
    BIP_Play_Uninit(pCtx);

errorBipInit:
    BIP_Uninit();

errorNexusInit:
#if NXCLIENT_SUPPORT
    NxClient_Uninit();
#else /* NXCLIENT_SUPPORT */
    NEXUS_Platform_Uninit();
#endif /* NXCLIENT_SUPPORT */
    return bipStatus;
}

static BIP_Status BIP_Play_NexusResourcesInit(BIP_Play_Context *pCtx)
{
    BIP_Status bipStatus = BIP_ERR_INTERNAL;
    NEXUS_Error rc;
#if NXCLIENT_SUPPORT
    NxClient_DisplaySettings displaySettings;
#endif
    BDBG_ENTER(BIP_Play_NexusResourcesInit);

#ifndef NXCLIENT_SUPPORT
    NEXUS_Platform_GetConfiguration(&pCtx->platformConfig);
#endif

    /* Create a sync channel */
    NEXUS_SyncChannel_GetDefaultSettings(&pCtx->syncChannelSettings);
    pCtx->syncChannelSettings.enablePrecisionLipsync = true;
    pCtx->syncChannel = NEXUS_SyncChannel_Create(&pCtx->syncChannelSettings);
    BIP_CHECK_GOTO((pCtx->syncChannel), ("NEXUS_SyncChannel_Create Failed"), error, BIP_ERR_NEXUS, bipStatus);
    BDBG_MSG ((BIP_MSG_PRE_FMT "Using Nexus STC channel for lipsync" BIP_MSG_PRE_ARG));

#if NXCLIENT_SUPPORT
    /* Acquire video and audio decoders.*/
    NxClient_GetDefaultAllocSettings(&pCtx->allocSettings);
    pCtx->allocSettings.simpleVideoDecoder = pCtx->options.disableVideo ? 0:1;
    pCtx->allocSettings.simpleAudioDecoder = pCtx->options.disableAudio ? 0:1;
    rc = NxClient_Alloc(&pCtx->allocSettings, &pCtx->allocResults );
    BIP_CHECK_GOTO((rc == NEXUS_SUCCESS), ("NxClient_Alloc Failed"), error, BIP_INF_NEXUS_RESOURCE_NOT_AVAILABLE, bipStatus);
    BDBG_MSG((BIP_MSG_PRE_FMT "NxClient resources allocated" BIP_MSG_PRE_ARG));

    NxClient_GetDefaultConnectSettings(&pCtx->connectSettings);

    if (!pCtx->options.disableVideo) /* TBD: Why is this needed? && pCtx->videoCodec != NEXUS_VideoCodec_eUnknown) */
    {
        pCtx->connectSettings.simpleVideoDecoder[0].id = pCtx->allocResults.simpleVideoDecoder[0].id;
        /* TBD: Is this needed? */
        /* pCtx->connectSettings.simpleVideoDecoder[0].decoderCapabilities.supportedCodecs[pCtx->videoCodec] = true;*/
        /* TBD: 'Player' is probing the stream to set the max width and height
         * Is that really necessary, can't we just leave it at default or set it to max? */
        if(pCtx->maxWidth && pCtx->maxHeight)
        {
            pCtx->connectSettings.simpleVideoDecoder[0].decoderCapabilities.maxWidth = pCtx->maxWidth;
            pCtx->connectSettings.simpleVideoDecoder[0].decoderCapabilities.maxHeight = pCtx->maxHeight;
        }
        /* This is done since in some cases probing may not provide video width and height. In that case for HEVC we force the max Height and max Width.*/
        else if((NEXUS_VideoCodec_eH265 == pCtx->videoCodec) || (NEXUS_VideoCodec_eVp9 == pCtx->videoCodec))
        {
            pCtx->connectSettings.simpleVideoDecoder[0].decoderCapabilities.maxWidth = pCtx->maxWidth = 3840;
            pCtx->connectSettings.simpleVideoDecoder[0].decoderCapabilities.maxHeight = pCtx->maxHeight = 2160;
        }
    }
    if (!pCtx->options.disableAudio) /* TBD: Why is this needed? && pCtx->audioCodec != NEXUS_AudioCodec_eUnknown) */
    {
        pCtx->connectSettings.simpleAudioDecoder.id = pCtx->allocResults.simpleAudioDecoder.id;
    }

    /* Successfully connected to the NxServer. Acquire AV Decoder & Transcode Handles. */
    rc = NxClient_Connect(&pCtx->connectSettings, &pCtx->connectId);
    BIP_CHECK_GOTO((rc == NEXUS_SUCCESS), ("NxClient_Connect Failed"), error, BIP_INF_NEXUS_RESOURCE_NOT_AVAILABLE, bipStatus);
    BDBG_MSG((BIP_MSG_PRE_FMT "Nxclient connected to server" BIP_MSG_PRE_ARG));

    if (!pCtx->options.disableVideo) /* TBD: Why is this needed? && pCtx->videoCodec != NEXUS_VideoCodec_eUnknown) */
    {
        pCtx->hSimpleVideoDecoder = NEXUS_SimpleVideoDecoder_Acquire(pCtx->allocResults.simpleVideoDecoder[0].id);
        BIP_CHECK_GOTO((pCtx->hSimpleVideoDecoder), ("NEXUS_SimpleVideoDecoder_Acquire Failed"), error, BIP_INF_NEXUS_RESOURCE_NOT_AVAILABLE, bipStatus);
        BDBG_MSG((BIP_MSG_PRE_FMT "Simple Video decoder acquired" BIP_MSG_PRE_ARG));
    }

    if (!pCtx->options.disableAudio) /* TBD: Why is this needed? && pCtx->audioCodec != NEXUS_AudioCodec_eUnknown) */
    {
        pCtx->hSimpleAudioDecoder = NEXUS_SimpleAudioDecoder_Acquire(pCtx->allocResults.simpleAudioDecoder.id);
        BIP_CHECK_GOTO((pCtx->hSimpleAudioDecoder), ("NEXUS_SimpleAudioDecoder_Acquire Failed"), error, BIP_INF_NEXUS_RESOURCE_NOT_AVAILABLE, bipStatus);
        BDBG_MSG((BIP_MSG_PRE_FMT "Simple Audio decoder acquired" BIP_MSG_PRE_ARG));
    }

    NxClient_GetDisplaySettings(&displaySettings);
    displaySettings.format = pCtx->options.displayFormat;
    NxClient_SetDisplaySettings(&displaySettings);

    pCtx->hSimpleStcChannel = NEXUS_SimpleStcChannel_Create(NULL);
    BIP_CHECK_GOTO((pCtx->hSimpleStcChannel), ("NEXUS_SimpleStcChannel_Create Failed"), error, BIP_INF_NEXUS_RESOURCE_NOT_AVAILABLE, bipStatus);
    BDBG_MSG((BIP_MSG_PRE_FMT "Simple STC Channel created" BIP_MSG_PRE_ARG));

    /* We have successfully acquired all needed resources. */
#else /* NXCLIENT_SUPPORT */
    /* Open Video decoder */
    if (!pCtx->options.disableVideo)
    {
        NEXUS_VideoDecoderOpenSettings videoDecoderOpenSettings;

        NEXUS_VideoDecoder_GetDefaultOpenSettings(&videoDecoderOpenSettings);
        /* Increase the CDB size as it may be used as the dejitter buffer */
        videoDecoderOpenSettings.fifoSize = pCtx->options.videoCdbSize*1024*1024;
        pCtx->videoDecoder = NEXUS_VideoDecoder_Open(0, &videoDecoderOpenSettings);
        BIP_CHECK_GOTO((pCtx->videoDecoder), ("NEXUS_VideoDecoder_Open Failed"), error, BIP_ERR_NEXUS, bipStatus);
        BDBG_MSG((BIP_MSG_PRE_FMT "Video decoder opened" BIP_MSG_PRE_ARG));
    }

    /* Open audio decoder */
    if (!pCtx->options.disableAudio)
    {
        NEXUS_AudioDecoderOpenSettings audioDecoderOpenSettings;

        NEXUS_AudioDecoder_GetDefaultOpenSettings(&audioDecoderOpenSettings);
        BDBG_MSG((BIP_MSG_PRE_FMT "fifo size %d, type %d\n" BIP_MSG_PRE_ARG,
                  audioDecoderOpenSettings.fifoSize, audioDecoderOpenSettings.type));
        audioDecoderOpenSettings.fifoSize = 2*audioDecoderOpenSettings.fifoSize; /* just doubled size is enough for using audio CDB as dejitter buffer. */
        audioDecoderOpenSettings.type = NEXUS_AudioDecoderType_eDecode;
        pCtx->pcmDecoder = NEXUS_AudioDecoder_Open(0, &audioDecoderOpenSettings);
        BIP_CHECK_GOTO((pCtx->pcmDecoder), ("NEXUS_AudioDecoder_Open Failed"), error, BIP_ERR_NEXUS, bipStatus);
        BDBG_MSG((BIP_MSG_PRE_FMT "Audio decoder opened" BIP_MSG_PRE_ARG));
    }

    /* Bring up video display and outputs */
    if (!pCtx->options.disableVideo)
    {
        NEXUS_DisplaySettings displaySettings;

        NEXUS_Display_GetDefaultSettings(&displaySettings);
        /* Note: change to display type back for panel output */

        displaySettings.displayType = NEXUS_DisplayType_eAuto;
        displaySettings.format = pCtx->options.displayFormat;
        pCtx->display = NEXUS_Display_Open(0, &displaySettings);
        BIP_CHECK_GOTO((pCtx->display), ("NEXUS_Display_Open Failed"), error, BIP_ERR_NEXUS, bipStatus);

        pCtx->window = NEXUS_VideoWindow_Open(pCtx->display, 0);
        BIP_CHECK_GOTO((pCtx->window), ("NEXUS_VideoDecoder_Open Failed"), error, BIP_ERR_NEXUS, bipStatus);
#if NEXUS_NUM_HDMI_OUTPUTS
        /* Install hotplug callback -- video only for now */
        rc = NEXUS_Display_AddOutput(pCtx->display, NEXUS_HdmiOutput_GetVideoConnector(pCtx->platformConfig.outputs.hdmi[0]));
        BIP_CHECK_GOTO((rc == NEXUS_SUCCESS), ("NEXUS_Display_AddOutput (videoDecoder) Failed"), error, BIP_ERR_NEXUS, bipStatus);
#endif
        BDBG_MSG((BIP_MSG_PRE_FMT "Display is Opened\n" BIP_MSG_PRE_ARG));
    }

    /* Bring up video display and outputs */
    if (!pCtx->options.disableAudio)
    {
#if NEXUS_NUM_HDMI_OUTPUTS
        rc = NEXUS_AudioOutput_AddInput(NEXUS_HdmiOutput_GetAudioConnector(pCtx->platformConfig.outputs.hdmi[0] ),
                        NEXUS_AudioDecoder_GetConnector(pCtx->pcmDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
        BIP_CHECK_GOTO((rc == NEXUS_SUCCESS), ("NEXUS_Display_AddOutput (audioDecoder) Failed"), error, BIP_ERR_NEXUS, bipStatus);
#endif
    }
#endif /* NXCLIENT_SUPPORT */

    /* No errors, set return value to success */
    bipStatus = BIP_SUCCESS;

error:
    BDBG_LEAVE(BIP_Play_NexusResourcesInit);
    return bipStatus;
}

static void BIP_Play_NexusResourcesUninit(BIP_Play_Context *pCtx)
{
    BDBG_ENTER(BIP_Play_NexusResourcesUninit);

#if NXCLIENT_SUPPORT
    if (pCtx->hSimpleVideoDecoder)
        NEXUS_SimpleVideoDecoder_Release(pCtx->hSimpleVideoDecoder);
    if (pCtx->hSimpleAudioDecoder)
        NEXUS_SimpleAudioDecoder_Release(pCtx->hSimpleAudioDecoder);
    if (pCtx->hSimpleStcChannel)
        NEXUS_SimpleStcChannel_Destroy(pCtx->hSimpleStcChannel);
#else /* NXCLIENT_SUPPORT */
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_AudioOutput_RemoveAllInputs(NEXUS_HdmiOutput_GetAudioConnector(pCtx->platformConfig.outputs.hdmi[0]));
#endif
    if (pCtx->videoDecoder)
        NEXUS_VideoDecoder_Close(pCtx->videoDecoder);
    if (pCtx->pcmDecoder)
        NEXUS_AudioDecoder_Close(pCtx->pcmDecoder);
    if (pCtx->window)
    {
        NEXUS_VideoWindow_RemoveAllInputs(pCtx->window);
        NEXUS_VideoWindow_Close(pCtx->window);
    }
    if (pCtx->display)
    {
        NEXUS_Display_RemoveAllOutputs(pCtx->display);
        NEXUS_Display_Close(pCtx->display);
    }
#endif /* NXCLIENT_SUPPORT */

    if (pCtx->syncChannel)
       NEXUS_SyncChannel_Destroy(pCtx->syncChannel);

    BDBG_LEAVE(BIP_Play_NexusResourcesUninit);
}

BIP_Status BIP_Play_Init(BIP_Play_Context** pBipPlayCtx)
{
    unsigned i;
    BIP_Status bipStatus = BIP_ERR_INTERNAL;
    BIP_Play_Context *pCtx;
    struct binput_settings inputSettings;

    BDBG_ENTER(BIP_Play_Init);

    *pBipPlayCtx = NULL;
    pCtx = B_Os_Calloc(1, sizeof(BIP_Play_Context));
    BIP_CHECK_GOTO((pCtx), ("Error allocating memory for BIP Play context"), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus);
    *pBipPlayCtx = pCtx;

    /* Initialize BIP play context params */
    memset(pCtx, 0, sizeof(BIP_Play_Context));
    pCtx->rate = 1.0;
    pCtx->audioCodec = NEXUS_AudioCodec_eUnknown;
    pCtx->videoCodec = NEXUS_VideoCodec_eUnknown;
    pCtx->lastCmd.command = BIP_Play_RuntimeCmd_eMax;

    /* Allocate string resources for Urls */
    for (i = 0; i < MAX_NUM_URLS; i++)
    {
        pCtx->hUrls[i] = BIP_String_Create();
        if (pCtx->hUrls[i] == NULL)
        {
            break;
        }
    }
    pCtx->numUrls = i;
    BIP_CHECK_GOTO((pCtx->numUrls), ("Error allocating URL string resources"), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus);

    if (pCtx->numUrls != MAX_NUM_URLS)
        BDBG_WRN((BIP_MSG_PRE_FMT "Allocated only (%d) URL string resources" BIP_MSG_PRE_ARG, pCtx->numUrls));

    pCtx->currentUrl = BIP_String_Create();
    BIP_CHECK_GOTO((pCtx->currentUrl), ("BIP_String_Create() failed"), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus);
    BDBG_MSG((BIP_MSG_PRE_FMT "BIP Play URL strings allocated" BIP_MSG_PRE_ARG));

    /* Create playback completion event */
    pCtx->hCompletionEvent = B_Event_Create(NULL);
    BIP_CHECK_GOTO((pCtx->hCompletionEvent), ("B_Event_Create Failed"), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus);

    /* Initialize remote input */
    binput_get_default_settings(&inputSettings);
    pCtx->input = binput_open(&inputSettings);
    if (pCtx->input == NULL)
        BDBG_WRN((BIP_MSG_PRE_FMT "Error opening remote input, remote keys will not work!" BIP_MSG_PRE_ARG));
    BDBG_MSG((BIP_MSG_PRE_FMT "Remote input client initialized" BIP_MSG_PRE_ARG));

    /* Intiialize GUI */
    bipStatus = BIP_Play_GuiInit(&pCtx->hGui);
    if (bipStatus != BIP_SUCCESS)
        BDBG_WRN((BIP_MSG_PRE_FMT "Error initializing gui, no ui will be seen!" BIP_MSG_PRE_ARG));
    BDBG_MSG((BIP_MSG_PRE_FMT "Gui initialization complete" BIP_MSG_PRE_ARG));

    bipStatus = BIP_SUCCESS;

error:
    BDBG_LEAVE(BIP_Play_Init);
    return bipStatus;
}

void BIP_Play_Uninit(BIP_Play_Context* pCtx)
{
    unsigned i;

    BDBG_ENTER(BIP_Play_Uninit);

    if (pCtx)
    {
        for (i = 0; i < MAX_NUM_URLS; i++)
        {
            if (pCtx->hUrls[i] != NULL)
                BIP_String_Destroy(pCtx->hUrls[i]);
        }

        if (pCtx->currentUrl)
            BIP_String_Destroy(pCtx->currentUrl);

        if (pCtx->hCompletionEvent)
            B_Event_Destroy(pCtx->hCompletionEvent);

        if (pCtx->input)
            binput_close(pCtx->input);

        if (pCtx->hGui)
            BIP_Play_GuiUninit(pCtx->hGui);

        B_Os_Free(pCtx);
    }

    BDBG_LEAVE(BIP_Play_Uninit);
}

static bool BIP_Play_GetTrackOfType(
    BIP_MediaInfoHandle hMediaInfo,
    BIP_MediaInfoTrackType trackType,
    BIP_MediaInfoTrack *pMediaInfoTrackOut
    )
{
    bool                    trackFound = false;
    BIP_MediaInfoStream     *pMediaInfoStream;
    BIP_MediaInfoTrackGroup *pMediaInfoTrackGroup = NULL;
    bool                    trackGroupPresent = false;
    BIP_MediaInfoTrack      *pMediaInfoTrack;

    BDBG_ENTER(BIP_Play_GetTrackOfType);

    pMediaInfoStream = BIP_MediaInfo_GetStream(hMediaInfo);
    BDBG_ASSERT(pMediaInfoStream);
    if (!pMediaInfoStream) return false;

    if (pMediaInfoStream->numberOfTrackGroups != 0)
    {
        pMediaInfoTrackGroup = pMediaInfoStream->pFirstTrackGroupInfo;
        pMediaInfoTrack = pMediaInfoTrackGroup->pFirstTrackForTrackGroup;
        trackGroupPresent = true;
    }
    else
    {
        /* None of the track belongs to any trackGroup, in this case stream out all tracks from mediaInfoStream.*/
        pMediaInfoTrack = pMediaInfoStream->pFirstTrackInfoForStream;
    }

    if (!pMediaInfoTrack) return false;

    while (pMediaInfoTrack)
    {
        if (pMediaInfoTrack->trackType == trackType)
        {
            BDBG_MSG((BIP_MSG_PRE_FMT "Found trackType=%s with trackId=%d" BIP_MSG_PRE_ARG, BIP_ToStr_BIP_MediaInfoTrackType(trackType), pMediaInfoTrack->trackId));
            *pMediaInfoTrackOut = *pMediaInfoTrack;
            trackFound = true;
            break;
        }

        if (true == trackGroupPresent)
        {
            pMediaInfoTrack = pMediaInfoTrack->pNextTrackForTrackGroup;
        }
        else
        {
            pMediaInfoTrack = pMediaInfoTrack->pNextTrackForStream;
        }
    }

    BDBG_LEAVE(BIP_Play_GetTrackOfType);
    return (trackFound);
} /* BIP_Play_GetTrackOfType */

static bool BIP_Play_GetSpecialAudioTrackType(
        BIP_MediaInfoHandle             hMediaInfo,
        BIP_MediaInfoTrack              *pMediaInfoTrackOut,
        const char                      *language,
        unsigned                        ac3ServiceType
        )
{
    bool                    trackFound = false;
    BIP_MediaInfoStream     *pMediaInfoStream;
    BIP_MediaInfoTrackGroup *pMediaInfoTrackGroup = NULL;
    bool                    trackGroupPresent = false;
    BIP_MediaInfoTrack      *pMediaInfoTrack;

    BDBG_ENTER(BIP_Play_GetSpecialAudioTrackType);

    pMediaInfoStream = BIP_MediaInfo_GetStream(hMediaInfo);
    BDBG_ASSERT(pMediaInfoStream);
    if (!pMediaInfoStream) return false;

    if (pMediaInfoStream->numberOfTrackGroups != 0)
    {
        pMediaInfoTrackGroup = pMediaInfoStream->pFirstTrackGroupInfo;
        pMediaInfoTrack = pMediaInfoTrackGroup->pFirstTrackForTrackGroup;
        trackGroupPresent = true;
    }
    else
    {
        /* None of the track belongs to any trackGroup, in this case stream out all tracks from mediaInfoStream.*/
        pMediaInfoTrack = pMediaInfoStream->pFirstTrackInfoForStream;
    }

    if (!pMediaInfoTrack) return false;

    while (pMediaInfoTrack)
    {
        if (pMediaInfoTrack->trackType == BIP_MediaInfoTrackType_eAudio)
        {
            if(language && ac3ServiceType != UINT_MAX) /* If both are set */
            {
                int ret = 0;

                if(pMediaInfoTrack->info.audio.pLanguage) /* check whether track specific language info is at all present.*/
                {
                    ret = strncmp(language, pMediaInfoTrack->info.audio.pLanguage, BIP_MEDIA_INFO_LANGUAGE_FIELD_SIZE);
                }

                if((!ret) && (pMediaInfoTrack->info.audio.descriptor.ac3.bsmod == ac3ServiceType))
                {
                    BDBG_MSG((BIP_MSG_PRE_FMT "Found Special Audio track for language = %s and pMediaInfoTrack->info.audio.descriptor.ac3.bsmod = %d"
                              BIP_MSG_PRE_ARG, pMediaInfoTrack->info.audio.pLanguage, pMediaInfoTrack->info.audio.descriptor.ac3.bsmod));
                    *pMediaInfoTrackOut = *pMediaInfoTrack;
                    trackFound = true;
                    break;
                }
            }
            else if( language && pMediaInfoTrack->info.audio.pLanguage )
            {
                if(!strncmp(language, pMediaInfoTrack->info.audio.pLanguage, BIP_MEDIA_INFO_LANGUAGE_FIELD_SIZE))
                {
                    BDBG_MSG((BIP_MSG_PRE_FMT "Found Audio track for language = %s and trackId =%d"
                              BIP_MSG_PRE_ARG, pMediaInfoTrack->info.audio.pLanguage, pMediaInfoTrack->trackId));
                    *pMediaInfoTrackOut = *pMediaInfoTrack;
                    trackFound = true;
                    break;
                }
            }
            else if(ac3ServiceType != UINT_MAX
                    && (pMediaInfoTrack->info.audio.descriptor.ac3.bsmodValid)
                    && (pMediaInfoTrack->info.audio.descriptor.ac3.bsmod == ac3ServiceType))
            {
                    BDBG_MSG((BIP_MSG_PRE_FMT "Found Special Audio track for ac3ServiceType = %d and trackId =%d"
                              BIP_MSG_PRE_ARG, ac3ServiceType, pMediaInfoTrack->trackId));
                    *pMediaInfoTrackOut = *pMediaInfoTrack;
                    trackFound = true;
                    break;
            }
        }

        if (true == trackGroupPresent)
        {
            pMediaInfoTrack = pMediaInfoTrack->pNextTrackForTrackGroup;
        }
        else
        {
            pMediaInfoTrack = pMediaInfoTrack->pNextTrackForStream;
        }
    }

    BDBG_LEAVE(BIP_Play_GetSpecialAudioTrackType);
    return (trackFound);
} /* BIP_Play_GetSpecialAudioTrackType */

static BIP_Status BIP_Play_UpdateCodecInfo(
        BIP_Play_Context *pCtx, /* In */
        BIP_PlayerSettings *pPlayerSettings /* In/Out */
    )
{
    BIP_MediaInfoTrack mediaInfoTrack;
    const BIP_MediaInfoStream *pMediaInfoStream;
    BIP_Status bipStatus = BIP_ERR_INTERNAL;

    BDBG_ENTER(BIP_Play_UpdateCodecInfo);

    /* Get first Video & Audio track entry from pMediaInfoStream .*/
    pMediaInfoStream = BIP_MediaInfo_GetStream(pCtx->hMediaInfo);
    BIP_CHECK_GOTO((pMediaInfoStream), ( "BIP_MediaInfo_GetStream Failed!"), error, bipStatus, bipStatus);
    pCtx->streamDurationInMs = pMediaInfoStream->durationInMs;

    if (pCtx->hMediaInfo)
    {
        /* Set BIP_player video codec info */
       if (!pCtx->options.disableVideo && BIP_Play_GetTrackOfType(pCtx->hMediaInfo, BIP_MediaInfoTrackType_eVideo, &mediaInfoTrack))
       {
           pPlayerSettings->videoTrackId = mediaInfoTrack.trackId;
           pPlayerSettings->videoTrackSettings.pidTypeSettings.video.codec = mediaInfoTrack.info.video.codec;
           pCtx->videoCodec = mediaInfoTrack.info.video.codec;
           pCtx->maxHeight = mediaInfoTrack.info.video.height;
           pCtx->maxWidth = mediaInfoTrack.info.video.width;
       }

        /* Set BIP_player audio codec info */
       if(!pCtx->options.disableAudio)
       {
           bool trackFound = false;

           if((strlen(pCtx->options.language) > 1) || (pCtx->options.ac3ServiceType != UINT_MAX))
           {
               trackFound = BIP_Play_GetSpecialAudioTrackType(
                                pCtx->hMediaInfo,
                                &mediaInfoTrack,
                                (strlen(pCtx->options.language) > 1) ? pCtx->options.language : NULL,
                                pCtx->options.ac3ServiceType);
           }
           else
           {
               trackFound = BIP_Play_GetTrackOfType(pCtx->hMediaInfo, BIP_MediaInfoTrackType_eAudio, &mediaInfoTrack);
           }
           if (trackFound)
           {
               pPlayerSettings->audioTrackId = mediaInfoTrack.trackId;
               pPlayerSettings->audioTrackSettings.pidSettings.pidTypeSettings.audio.codec = mediaInfoTrack.info.audio.codec;
               pCtx->audioCodec = mediaInfoTrack.info.audio.codec;
           }
           else
           {
               BDBG_WRN((BIP_MSG_PRE_FMT "Not able to found any Audio track" BIP_MSG_PRE_ARG));
           }
       }

       /* Make sure Probe found atleast a Video or an Audio Track. */
       BIP_CHECK_GOTO(((pPlayerSettings->videoTrackId != UINT_MAX) || (pPlayerSettings->audioTrackId != UINT_MAX)), ("Neither Audio nor Video Tracks found for URL=%s", BIP_String_GetString(pCtx->currentUrl)), error, bipStatus, bipStatus);

       /* Found atleast one video or audio track */
       bipStatus = BIP_SUCCESS;
    }

error:

    BDBG_LEAVE(BIP_Play_UpdateCodecInfo);
    return bipStatus;
}

static void BIP_Play_UpdatePlayerSettings(
    BIP_Play_Context* pCtx, /* In */
    BIP_PlayerSettings *pSettings /* Out */
    )
{
    BDBG_ENTER(BIP_Play_UpdatePlayerSettings);

    /* Set audio/video decoder hanldes for BIP Player*/
#if NXCLIENT_SUPPORT
    pSettings->audioTrackSettings.pidTypeSettings.audio.simpleDecoder = pCtx->hSimpleAudioDecoder;
    pSettings->videoTrackSettings.pidTypeSettings.video.simpleDecoder = pCtx->hSimpleVideoDecoder;
    pSettings->playbackSettings.simpleStcChannel = pCtx->hSimpleStcChannel;
#else /* NXCLIENT_SUPPORT */
    if ((!pCtx->options.disableAudio) && (pCtx->audioCodec != NEXUS_AudioCodec_eUnknown))
        pSettings->audioTrackSettings.pidTypeSettings.audio.primary = pCtx->pcmDecoder;
    if ((!pCtx->options.disableVideo) && (pCtx->videoCodec != NEXUS_VideoCodec_eUnknown))
    {
        pSettings->videoTrackSettings.pidTypeSettings.video.decoder = pCtx->videoDecoder;
        pSettings->hDisplay = pCtx->display; /* Needed for clock recovery setup of live ip stream. */
        pSettings->hWindow = pCtx->window;
    }
    pSettings->playbackSettings.stcChannel = pCtx->stcChannel;
#endif /* NXCLIENT_SUPPORT */

    /* Set end of stream handling settings  */
    pSettings->playbackSettings.errorCallback.callback = BIP_Play_DoneCallbackFromBIP;
    pSettings->playbackSettings.errorCallback.context = pCtx;
    if (pCtx->options.enableTimeShift ||
        (pCtx->playerStreamInfo.dataAvailabilityModel == BIP_PlayerDataAvailabilityModel_eLimitedRandomAccess))
    {
        pSettings->playbackSettings.endOfStreamAction = NEXUS_PlaybackLoopMode_ePlay;
        pSettings->playbackSettings.beginningOfStreamAction = NEXUS_PlaybackLoopMode_ePlay;
        pSettings->playbackSettings.playErrorHandling = NEXUS_PlaybackErrorHandlingMode_eAbort;
    }
    else if (pCtx->options.loop)
    {
        pSettings->playbackSettings.endOfStreamAction = NEXUS_PlaybackLoopMode_eLoop;
        pSettings->playbackSettings.beginningOfStreamAction = NEXUS_PlaybackLoopMode_eLoop;
    }
    else
    {
        pSettings->playbackSettings.endOfStreamAction = NEXUS_PlaybackLoopMode_ePause;
        pSettings->playbackSettings.endOfStreamCallback.callback = BIP_Play_DoneCallbackFromBIP;
        pSettings->playbackSettings.endOfStreamCallback.context = pCtx;
        pSettings->playbackSettings.beginningOfStreamAction = NEXUS_PlaybackLoopMode_ePlay;
    }
    pSettings->playbackSettings.startPaused = pCtx->options.startPaused;

    /* Use playpump if specified at command line */
    if (pCtx->options.usePlaypump)
    {
        pCtx->playerStreamInfo.usePlaypump = true;
    }

    BDBG_LEAVE(BIP_Play_UpdatePlayerSettings);
    return;
}

static void BIP_Play_DoneCallbackFromBIP(
    void *context,
    int   param
    )
{
    BIP_Play_Context *pCtx = context;

    BSTD_UNUSED( param );
    BDBG_WRN((BIP_MSG_PRE_FMT "Got endOfStream Callback from BIP: Settings hEvent=%p)" BIP_MSG_PRE_ARG, pCtx->hCompletionEvent));
    pCtx->playbackDone = true;
    B_Event_Set(pCtx->hCompletionEvent);
}

static BIP_Status BIP_Play_SetupNexusAVDecoders(
        BIP_Play_Context *pCtx
    )
{
    NEXUS_Error rc = NEXUS_UNKNOWN;
    BIP_Status bipStatus = BIP_ERR_NEXUS;

#if NXCLIENT_SUPPORT
    NEXUS_SimpleVideoDecoderStartSettings videoProgram;
    NEXUS_SimpleAudioDecoderStartSettings audioProgram;
    NEXUS_SimpleVideoDecoderServerSettings videoServerSettings;
    NEXUS_SimpleAudioDecoderServerSettings audioServerSettings;
#else /* NXCLIENT_SUPPORT */
    NEXUS_VideoDecoderSettings videoDecoderSettings;
    NEXUS_VideoDecoderStartSettings videoProgram;
    NEXUS_AudioDecoderStartSettings audioProgram;
#endif /* NXCLIENT_SUPPORT */

    BDBG_ENTER(BIP_Play_SetupNexusAVDecoders);

    /* Connect sync channel for AV Lipsync. */
#if NEXUS_HAS_SYNC_CHANNEL
    NEXUS_SyncChannel_GetSettings(pCtx->syncChannel, &pCtx->syncChannelSettings);
#if NXCLIENT_SUPPORT
    NEXUS_SimpleVideoDecoder_GetServerSettings(pCtx->hSimpleVideoDecoder, &videoServerSettings);
    NEXUS_SimpleAudioDecoder_GetServerSettings(pCtx->hSimpleAudioDecoder, &audioServerSettings);
    if (!pCtx->options.disableVideo && (pCtx->videoCodec != NEXUS_VideoCodec_eUnknown))
        pCtx->syncChannelSettings.videoInput = NEXUS_VideoDecoder_GetConnector(videoServerSettings.videoDecoder);
    if (!pCtx->options.disableAudio && (pCtx->audioCodec != NEXUS_AudioCodec_eUnknown))
        pCtx->syncChannelSettings.audioInput[0] = NEXUS_AudioDecoder_GetConnector(audioServerSettings.primary,
                                                    audioServerSettings.syncConnector);
#else /* NXCLIENT_SUPPORT */
    if (!pCtx->options.disableVideo && (pCtx->videoCodec != NEXUS_VideoCodec_eUnknown))
        pCtx->syncChannelSettings.videoInput = NEXUS_VideoDecoder_GetConnector(pCtx->videoDecoder);

    if (!pCtx->options.disableAudio && (pCtx->audioCodec != NEXUS_AudioCodec_eUnknown))
        pCtx->syncChannelSettings.audioInput[0] = NEXUS_AudioDecoder_GetConnector(pCtx->pcmDecoder,
                                                            NEXUS_AudioDecoderConnectorType_eStereo);
#endif /* NXCLIENT_SUPPORT */
    NEXUS_SyncChannel_SetSettings(pCtx->syncChannel, &pCtx->syncChannelSettings);
    BDBG_MSG((BIP_MSG_PRE_FMT "Sync channel setup complete" BIP_MSG_PRE_ARG));
#endif /* NEXUS_HAS_SYNC_CHANNEL */

#if NXCLIENT_SUPPORT
    /* Set up & start video decoder. */
    if (!pCtx->options.disableVideo && pCtx->videoPidChannel)
    {
        rc = NEXUS_SimpleVideoDecoder_SetStcChannel(pCtx->hSimpleVideoDecoder, pCtx->hSimpleStcChannel);
        BIP_CHECK_GOTO((rc == NEXUS_SUCCESS), ("NEXUS_SimpleVideoDecoder_SetStcChannel Failed"), error, BIP_ERR_NEXUS, bipStatus);

        NEXUS_SimpleVideoDecoder_GetDefaultStartSettings(&videoProgram);
        videoProgram.displayEnabled = true;
        videoProgram.settings.codec = pCtx->videoCodec;
        videoProgram.settings.pidChannel = pCtx->videoPidChannel;
        videoProgram.maxWidth = pCtx->maxWidth;
        videoProgram.maxHeight = pCtx->maxHeight;
        videoProgram.smoothResolutionChange = pCtx->options.enableSmoothing;
        rc = NEXUS_SimpleVideoDecoder_Start(pCtx->hSimpleVideoDecoder, &videoProgram);
        BIP_CHECK_GOTO((rc == NEXUS_SUCCESS), ("NEXUS_SimpleVideoDecoder_Start Failed"), error, BIP_ERR_NEXUS, bipStatus);
        pCtx->startSettings.simpleVideoStartSettings = videoProgram;
        BDBG_MSG((BIP_MSG_PRE_FMT "Simple Video decoder started" BIP_MSG_PRE_ARG));
    }
    /* Set up & start audio decoder. */
    if (!pCtx->options.disableAudio && pCtx->audioPidChannel)
    {
        rc = NEXUS_SimpleAudioDecoder_SetStcChannel(pCtx->hSimpleAudioDecoder, pCtx->hSimpleStcChannel);
        BIP_CHECK_GOTO((rc == NEXUS_SUCCESS), ("NEXUS_SimpleAudioDecoder_SetStcChannel Failed"), error, BIP_ERR_NEXUS, bipStatus);

        NEXUS_SimpleAudioDecoder_GetDefaultStartSettings(&audioProgram);
        audioProgram.primary.codec = pCtx->audioCodec;
        audioProgram.primary.pidChannel = pCtx->audioPidChannel;
        rc = NEXUS_SimpleAudioDecoder_Start(pCtx->hSimpleAudioDecoder, &audioProgram);
        BIP_CHECK_GOTO((rc == NEXUS_SUCCESS), ("NEXUS_SimpleAudioDecoder_Start Failed"), error, BIP_ERR_NEXUS, bipStatus);
        pCtx->startSettings.simpleAudioStartSettings = audioProgram;
        BDBG_MSG((BIP_MSG_PRE_FMT "Simple Audio decoder started" BIP_MSG_PRE_ARG));
    }
#else /* NXCLIENT_SUPPORT */
    /* Setup & start Video Decoder */
    if (!pCtx->options.disableVideo && pCtx->videoPidChannel)
    {
        NEXUS_VideoDecoder_GetSettings(pCtx->videoDecoder, &videoDecoderSettings);

        if(pCtx->maxWidth && pCtx->maxHeight)
        {
            videoDecoderSettings.maxWidth = pCtx->maxWidth;
            videoDecoderSettings.maxHeight = pCtx->maxHeight;
            BDBG_MSG((BIP_MSG_PRE_FMT "Video decoder width %d, height %d" BIP_MSG_PRE_ARG, pCtx->maxWidth, pCtx->maxHeight));
        }
        /* This is done since in some cases probing may not provide video width and height. In that case for HEVC we force the max Height and max Width.*/
        else if((NEXUS_VideoCodec_eH265 == pCtx->videoCodec) || (NEXUS_VideoCodec_eVp9 == pCtx->videoCodec))
        {
            videoDecoderSettings.maxWidth = pCtx->maxWidth = 3840;
            videoDecoderSettings.maxHeight = pCtx->maxHeight = 2160;
            BDBG_MSG((BIP_MSG_PRE_FMT "Video decoder width %d, height %d" BIP_MSG_PRE_ARG, pCtx->maxWidth, pCtx->maxHeight));
        }

        rc = NEXUS_VideoDecoder_SetSettings(pCtx->videoDecoder, &videoDecoderSettings);
        BIP_CHECK_GOTO((rc == NEXUS_SUCCESS), ("NEXUS_VideoDecoder_SetSettings Failed"), error, BIP_ERR_NEXUS, bipStatus);
        BDBG_MSG((BIP_MSG_PRE_FMT "Video Decoder settings are modified for IP \n" BIP_MSG_PRE_ARG));

        /* connect video decoder to display */
        NEXUS_VideoWindow_AddInput(pCtx->window, NEXUS_VideoDecoder_GetConnector(pCtx->videoDecoder));

        /* Start Video */
        NEXUS_VideoDecoder_GetDefaultStartSettings(&videoProgram);
        videoProgram.codec = pCtx->videoCodec;
        videoProgram.pidChannel = pCtx->videoPidChannel;
        videoProgram.stcChannel = pCtx->stcChannel;
        rc = NEXUS_VideoDecoder_Start(pCtx->videoDecoder, &videoProgram);
        BIP_CHECK_GOTO((rc == NEXUS_SUCCESS), ("NEXUS_VideoDecoder_Start Failed"), error, BIP_ERR_NEXUS, bipStatus);
        BDBG_MSG((BIP_MSG_PRE_FMT "Video Decoder is Started\n" BIP_MSG_PRE_ARG));
        pCtx->startSettings.videoStartSettings = videoProgram;
        BDBG_MSG((BIP_MSG_PRE_FMT "Video decoder started" BIP_MSG_PRE_ARG));
    }

    /* Setup & start Audio decoder. */
    if (!pCtx->options.disableAudio && pCtx->audioPidChannel)
    {
        NEXUS_AudioDecoder_GetDefaultStartSettings(&audioProgram);
        audioProgram.codec = pCtx->audioCodec;
        audioProgram.pidChannel = pCtx->audioPidChannel;
        audioProgram.stcChannel = pCtx->stcChannel;
        /* Start Audio */
        rc = NEXUS_AudioDecoder_Start(pCtx->pcmDecoder, &audioProgram);
        BIP_CHECK_GOTO((rc == NEXUS_SUCCESS), ("NEXUS_AudioDecoder_Start Failed"), error, BIP_ERR_NEXUS, bipStatus);
        BDBG_MSG((BIP_MSG_PRE_FMT "Audio Decoder is Started\n" BIP_MSG_PRE_ARG));
        pCtx->startSettings.audioStartSettings = audioProgram;
        BDBG_MSG((BIP_MSG_PRE_FMT "Audio decoder started" BIP_MSG_PRE_ARG));
    }
#endif /* NXCLIENT_SUPPORT */

    /* Setup picture quality control settings */
#if NXCLIENT_SUPPORT
    if (!pCtx->options.disableVideo && pCtx->videoPidChannel)
    {
        NEXUS_SimpleVideoDecoderPictureQualitySettings settings;

        NEXUS_SimpleVideoDecoder_GetPictureQualitySettings(pCtx->hSimpleVideoDecoder, &settings);
        settings.mad.deinterlace = pCtx->options.enableMad;
        settings.common.sharpnessEnable = (pCtx->options.videoSharpness != 0);
        settings.common.sharpness = pCtx->options.videoSharpness;
        settings.scaler.verticalDejagging = pCtx->options.enableDejagging;
        settings.scaler.horizontalLumaDeringing = settings.scaler.verticalLumaDeringing =
        settings.scaler.horizontalChromaDeringing = settings.scaler.verticalChromaDeringing = pCtx->options.enableDeringing;
        NEXUS_SimpleVideoDecoder_GetPictureQualitySettings(pCtx->hSimpleVideoDecoder, &settings);
    }

    /* Setup dolby audio drc mode */
    if (!pCtx->options.disableAudio && pCtx->audioPidChannel)
    {
        NEXUS_AudioDecoderCodecSettings settings;
        NEXUS_SimpleAudioDecoder_GetCodecSettings(pCtx->hSimpleAudioDecoder, NEXUS_SimpleAudioDecoderSelector_ePrimary,
                                                  pCtx->audioCodec, &settings);
        switch (pCtx->audioCodec) {
            case NEXUS_AudioCodec_eAc3: settings.codecSettings.ac3.drcMode = pCtx->options.dolbyDrcMode; break;
            case NEXUS_AudioCodec_eAc3Plus: settings.codecSettings.ac3Plus.drcMode = pCtx->options.dolbyDrcMode; break;
            /* only line and rf applie for aac/aacplus, but nexus can validate params */
            case NEXUS_AudioCodec_eAac: settings.codecSettings.aac.drcMode = (NEXUS_AudioDecoderDolbyPulseDrcMode)pCtx->options.dolbyDrcMode; break;
            case NEXUS_AudioCodec_eAacPlus: settings.codecSettings.aacPlus.drcMode = (NEXUS_AudioDecoderDolbyPulseDrcMode)pCtx->options.dolbyDrcMode; break;
            default: /* just ignore */ break;
        }
        NEXUS_SimpleAudioDecoder_SetCodecSettings(pCtx->hSimpleAudioDecoder, NEXUS_SimpleAudioDecoderSelector_ePrimary,
                                                  &settings);
    }

    /* Set display content mode */
    {
        NEXUS_SurfaceClientSettings settings;
        NEXUS_SurfaceClientHandle surfaceClient = bgui_surface_client(pCtx->hGui->gui);
        NEXUS_SurfaceClient_GetSettings(surfaceClient, &settings);
        settings.composition.contentMode = pCtx->options.contentMode;
        NEXUS_SurfaceClient_SetSettings(surfaceClient, &settings);
    }
#else
    /* Setup picture quality control settings */
    if (!pCtx->options.disableVideo && pCtx->videoPidChannel)
    {
        NEXUS_VideoWindowMadSettings madSettings;
        NEXUS_VideoWindowScalerSettings scalerSettings;
        NEXUS_PictureCtrlCommonSettings commonSettings;

        NEXUS_VideoWindow_GetMadSettings(pCtx->window, &madSettings);
        madSettings.deinterlace = pCtx->options.enableMad;
        NEXUS_VideoWindow_SetMadSettings(pCtx->window, &madSettings);

        NEXUS_PictureCtrl_GetCommonSettings(pCtx->window, &commonSettings);
        commonSettings.sharpnessEnable = (pCtx->options.videoSharpness != 0);
        commonSettings.sharpness = pCtx->options.videoSharpness;
        NEXUS_PictureCtrl_SetCommonSettings(pCtx->window, &commonSettings);

        NEXUS_VideoWindow_GetScalerSettings(pCtx->window, &scalerSettings);
        scalerSettings.verticalDejagging = pCtx->options.enableDejagging;
        scalerSettings.horizontalLumaDeringing = scalerSettings.verticalLumaDeringing =
        scalerSettings.horizontalChromaDeringing = scalerSettings.verticalChromaDeringing = pCtx->options.enableDeringing;
        NEXUS_VideoWindow_SetScalerSettings(pCtx->window, &scalerSettings);
    }

    /* Setup dolby audio drc mode */
    if (!pCtx->options.disableAudio && pCtx->audioPidChannel)
    {
        NEXUS_AudioDecoderCodecSettings settings;
        NEXUS_AudioDecoder_GetCodecSettings(pCtx->pcmDecoder, pCtx->audioCodec, &settings);
        switch (pCtx->audioCodec) {
            case NEXUS_AudioCodec_eAc3: settings.codecSettings.ac3.drcMode = pCtx->options.dolbyDrcMode; break;
            case NEXUS_AudioCodec_eAc3Plus: settings.codecSettings.ac3Plus.drcMode = pCtx->options.dolbyDrcMode; break;
            /* only line and rf applied for aac/aacplus, but nexus can validate params */
            case NEXUS_AudioCodec_eAac: settings.codecSettings.aac.drcMode = (NEXUS_AudioDecoderDolbyPulseDrcMode)pCtx->options.dolbyDrcMode; break;
            case NEXUS_AudioCodec_eAacPlus: settings.codecSettings.aacPlus.drcMode = (NEXUS_AudioDecoderDolbyPulseDrcMode)pCtx->options.dolbyDrcMode; break;
            default: /* just ignore */ break;
        }
        NEXUS_AudioDecoder_SetCodecSettings(pCtx->pcmDecoder, &settings);
    }

    /* Set display content mode */
    {
        NEXUS_VideoWindowSettings settings;

        NEXUS_VideoWindow_GetSettings(pCtx->window, &settings);
        settings.contentMode = pCtx->options.contentMode;
        NEXUS_VideoWindow_SetSettings(pCtx->window, &settings);
    }
#endif

    /* No errors, set return value to success */
    bipStatus = BIP_SUCCESS;

error:

    BDBG_LEAVE(BIP_Play_SetupNexusAVDecoders);
    return bipStatus;
}

BIP_Status BIP_Play_WaitForCommand(
        char *pBuf,
        int timeout
    )
{
    size_t  bytesRead;
    BIP_Status bipStatus;

    memset(pBuf, 0, MAX_RUNTIME_COMMAND_SIZE);
    bipStatus = BIP_Fd_ReadWithTimeout(STDIN_FILENO, MAX_RUNTIME_COMMAND_SIZE-1, timeout, pBuf, &bytesRead);
    pBuf[bytesRead-1] = '\0';   /* Replace \n w/ Null Char. */

    return bipStatus;
}

static BIP_Status BIP_Play_HandleRuntimeCmd(
        BIP_Play_Context *pCtx,
        BIP_Play_ParsedRuntimeCmdInfo *pCmd
    )
{
    int intArg;
    float rate;
    BIP_PlayerStatus status;
    bool rateChanged = false;
    BIP_PlayerSettings settings;
    BIP_MediaInfoTrack mediaInfoTrack;
    BIP_Play_RuntimeCommand command;
    BIP_Status bipStatus = BIP_SUCCESS;
    BIP_PlayerStatusFromServer serverStatus;
    BIP_PlayerGetStatusFromServerSettings serverSettings;

    BDBG_ENTER(BIP_Play_HandleRuntimeCmd);

    command = pCmd->command;
    intArg = pCmd->intArg;

    /* Command translation to a more basic operation */
    switch (command)
    {
        case BIP_Play_RuntimeCmd_eTogglePlay:
            if (pCtx->rate != 1.0)
                command = BIP_Play_RuntimeCmd_ePlay;
            else
                command = BIP_Play_RuntimeCmd_ePause;
            break;
        case BIP_Play_RuntimeCmd_eJumpBack:
            bipStatus = BIP_Player_GetStatus(pCtx->hPlayer, &status);
            if (bipStatus == BIP_SUCCESS)
            {
                command = BIP_Play_RuntimeCmd_eSeek;
                if ((status.currentPositionInMs - pCtx->options.jumpOffset) > status.firstPositionInMs)
                    intArg = status.currentPositionInMs - pCtx->options.jumpOffset;
                else
                    intArg = status.firstPositionInMs;
            }
            break;
        case BIP_Play_RuntimeCmd_eJumpFwd:
            bipStatus = BIP_Player_GetStatus(pCtx->hPlayer, &status);
            if (bipStatus == BIP_SUCCESS)
            {
                command = BIP_Play_RuntimeCmd_eSeek;
                if ((status.currentPositionInMs + pCtx->options.jumpOffset) < status.lastPositionInMs)
                    intArg = status.currentPositionInMs + pCtx->options.jumpOffset;
                else
                    intArg = status.lastPositionInMs;
            }
            break;
        default:
            break;
    }

    /* Handle runtime commands */
    switch (command)
    {
        case BIP_Play_RuntimeCmd_ePlay:
            /* If not already playing */
            if (pCtx->rate != 1.0)
            {
                bipStatus = BIP_Player_Play(pCtx->hPlayer);
                BIP_CHECK_GOTO(((bipStatus == BIP_SUCCESS) || (bipStatus == BIP_INF_NOT_AVAILABLE)),
                                ("BIP_Player_Play Failed: URL=%s", BIP_String_GetString(pCtx->currentUrl)),
                                 error, bipStatus, bipStatus);
                BIP_Player_PrintStatus(pCtx->hPlayer);
                BDBG_MSG((BIP_MSG_PRE_FMT "Resumed Playback!" BIP_MSG_PRE_ARG));
                pCtx->rate = 1.0;
                rateChanged = true;
            }
            else
                BDBG_WRN((BIP_MSG_PRE_FMT "Already playing, ignoring command!" BIP_MSG_PRE_ARG));
            break;
        case BIP_Play_RuntimeCmd_ePause:
            /* If not already paused */
            if (pCtx->rate != 0.0)
            {
                bipStatus = BIP_Player_Pause(pCtx->hPlayer, NULL);
                BIP_CHECK_GOTO(((bipStatus == BIP_SUCCESS) || (bipStatus == BIP_INF_NOT_AVAILABLE) || (bipStatus == BIP_INF_PLAYER_CANT_PAUSE)),
                                ( "BIP_Player_Pause Failed: URL=%s", BIP_String_GetString(pCtx->currentUrl)), error, bipStatus, bipStatus);
                BIP_Player_PrintStatus(pCtx->hPlayer);
                BDBG_MSG((BIP_MSG_PRE_FMT "Paused Playback!" BIP_MSG_PRE_ARG));
                pCtx->rate = 0.0;
                rateChanged = true;
            }
            else
                BDBG_WRN((BIP_MSG_PRE_FMT "Already paused, ignoring command!" BIP_MSG_PRE_ARG));
            break;
        case BIP_Play_RuntimeCmd_eSeek:
            BDBG_MSG((BIP_MSG_PRE_FMT "Starting Seek to %d ms" BIP_MSG_PRE_ARG, intArg));
            /* Seek to specified position */
            bipStatus = BIP_Player_Seek(pCtx->hPlayer, intArg);
            BIP_CHECK_GOTO(((bipStatus == BIP_SUCCESS) || (bipStatus == BIP_INF_NOT_AVAILABLE)),
                            ("BIP_Player_Seek Failed: URL=%s", BIP_String_GetString(pCtx->currentUrl)),
                            error, bipStatus, bipStatus);
            if (bipStatus == BIP_SUCCESS)
                BDBG_MSG((BIP_MSG_PRE_FMT "Seeked to %d ms" BIP_MSG_PRE_ARG, intArg));
            break;
        case BIP_Play_RuntimeCmd_ePlayAtRate:
            rate = BIP_Play_FractToFloat(pCmd->strArg);
            if (pCtx->rate != rate)
            {
                /* Change playback rate if not already at specified rate */
                BDBG_MSG((BIP_MSG_PRE_FMT "Changing playback rate to : %sx" BIP_MSG_PRE_ARG, pCmd->strArg));
                bipStatus = BIP_Player_PlayAtRateAsString(pCtx->hPlayer, pCmd->strArg, NULL);
                BIP_CHECK_GOTO(((bipStatus == BIP_SUCCESS) || (bipStatus == BIP_INF_NOT_AVAILABLE)),
                                ("BIP_Player_PlayAtRate Failed: URL=%s", BIP_String_GetString(pCtx->currentUrl)),
                                error, bipStatus, bipStatus);
                if (bipStatus == BIP_SUCCESS)
                    BDBG_MSG((BIP_MSG_PRE_FMT "Started playing at rate : %sx" BIP_MSG_PRE_ARG, pCmd->strArg));
                BIP_Player_PrintStatus(pCtx->hPlayer);

                /* Update rate */
                pCtx->rate = rate;
                rateChanged = true;
            }
            else
                BDBG_WRN((BIP_MSG_PRE_FMT "Already playing at rate (%3.2f), ignoring command!" BIP_MSG_PRE_ARG, pCtx->rate));
            break;
        case BIP_Play_RuntimeCmd_eFrameAdvance:
        case BIP_Play_RuntimeCmd_eFrameReverse:
            BDBG_MSG((BIP_MSG_PRE_FMT "Calling Frame %s" BIP_MSG_PRE_ARG,
                     (command == BIP_Play_RuntimeCmd_eFrameAdvance) ? "Advance" : "Reverse"));
            bipStatus = BIP_Player_PlayByFrame(pCtx->hPlayer, (command == BIP_Play_RuntimeCmd_eFrameAdvance) ? true : false);
            BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ("BIP_Player_PlayByFrame Failed: URL=%s", BIP_String_GetString(pCtx->currentUrl)), error, bipStatus, bipStatus);
            if (bipStatus == BIP_SUCCESS)
                BDBG_MSG((BIP_MSG_PRE_FMT "Played 1 frame!" BIP_MSG_PRE_ARG));
            BIP_Player_PrintStatus(pCtx->hPlayer);
            break;
        case BIP_Play_RuntimeCmd_ePrintStatus:
            BIP_Player_PrintStatus(pCtx->hPlayer);
            bipStatus = BIP_Player_GetStatus(pCtx->hPlayer, &status);
            BIP_CHECK_GOTO(((bipStatus == BIP_SUCCESS) || (bipStatus == BIP_INF_NOT_AVAILABLE)),
                            ("BIP_Player_Status Failed: URL=%s", BIP_String_GetString(pCtx->currentUrl)),
                            error, bipStatus, bipStatus);
            if (bipStatus == BIP_SUCCESS)
                BDBG_LOG((BIP_MSG_PRE_FMT "Player position cur/last: %0.3f/%0.3f sec " BIP_MSG_PRE_ARG,
                          status.currentPositionInMs/1000., status.lastPositionInMs/1000.));
            break;
        case BIP_Play_RuntimeCmd_ePrintServerStatus:
            BIP_Player_GetDefaultGetStatusFromServerSettings(&serverSettings);
            serverSettings.getAvailableSeekRange = true;
            serverSettings.timeoutInMs = 5000;
            bipStatus = BIP_Player_GetStatusFromServer(pCtx->hPlayer, &serverSettings, &serverStatus);
            BIP_CHECK_GOTO(((bipStatus == BIP_SUCCESS) || (bipStatus == BIP_INF_NOT_AVAILABLE)),
                            ("BIP_Player_StatusFromServer Failed: URL=%s", BIP_String_GetString(pCtx->currentUrl)),
                            error, bipStatus, bipStatus);
            BDBG_LOG((BIP_MSG_PRE_FMT "Player position cur/last: %0.3f/%0.3f sec " BIP_MSG_PRE_ARG,
                      serverStatus.currentPositionInMs/1000., serverStatus.availableSeekRange.lastPositionInMs/1000. ));
            BIP_Player_PrintStatus(pCtx->hPlayer);
            break;
        case BIP_Play_RuntimeCmd_eTune:
            /* Change stream */
            BIP_CHECK_GOTO((intArg <= (int)pCtx->numUrls), ("Invalid index specified valid indices [1 - %d]", pCtx->numUrls),
                           error, BIP_ERR_INVALID_PARAMETER, bipStatus);
            pCtx->playingNow = intArg - 1;
            break;
        case BIP_Play_RuntimeCmd_eGui:
            pCtx->options.disableGui = false;
            if (pCtx->hGui)
                BIP_Play_GuiEnable(pCtx->hGui);
            break;
        case BIP_Play_RuntimeCmd_eNoGui:
            pCtx->options.disableGui = true;
            if (pCtx->hGui)
                BIP_Play_GuiDisable(pCtx->hGui);
            break;
        case BIP_Play_RuntimeCmd_eRemoteKeys:
            pCtx->options.disableRemoteKeys = false;
            break;
        case BIP_Play_RuntimeCmd_eNoRemoteKeys:
            pCtx->options.disableRemoteKeys = true;
            break;
        case BIP_Play_RuntimeCmd_ePlayNewUrl:
            /* Nothing to be done, main loop will handle this */
            break;
        case BIP_Play_RuntimeCmd_ePlayLanguage:
            /* Change audio language */
            BIP_Player_GetSettings(pCtx->hPlayer, &settings);
            if(BIP_Play_GetSpecialAudioTrackType(pCtx->hMediaInfo, &mediaInfoTrack,
                                                 pCmd->strArg, UINT_MAX))
            {
                settings.audioTrackId = mediaInfoTrack.trackId;
                bipStatus = BIP_Player_SetSettings(pCtx->hPlayer, &settings );
                BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS),
                               ("BIP_Player_SetSettings Failed: for language=%s", pCtx->options.language),
                                error, bipStatus, bipStatus);
                /* Update the language in options */
                strncpy(pCtx->options.language, pCmd->strArg, AUDIO_LANG_LEN);
                BDBG_MSG((BIP_MSG_PRE_FMT "Audio language changed to : [%s]" BIP_MSG_PRE_ARG, pCtx->options.language));
            }
            else
                BDBG_WRN((BIP_MSG_PRE_FMT "Can't change audio track since |%s| Language specific audio track doesn't exist."
                          BIP_MSG_PRE_ARG, pCtx->options.language));
            break;
        case BIP_Play_RuntimeCmd_ePlayBsmod:
            BIP_Player_GetSettings(pCtx->hPlayer, &settings);

            if(BIP_Play_GetSpecialAudioTrackType(pCtx->hMediaInfo,
                           &mediaInfoTrack,
                           NULL,
                           intArg))
            {
                settings.audioTrackId = mediaInfoTrack.trackId;
                BDBG_MSG((BIP_MSG_PRE_FMT "New bsmod specific trackId is --------------------> %d"
                          BIP_MSG_PRE_ARG, settings.audioTrackId));
                bipStatus = BIP_Player_SetSettings(pCtx->hPlayer, &settings);
                BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS),
                                ("BIP_Player_SetSettings Failed: for language=%s", pCtx->options.language),
                                error, bipStatus, bipStatus);
                pCtx->options.ac3ServiceType = intArg;
                BDBG_MSG((BIP_MSG_PRE_FMT "Stream BSMOD changed to : %d" BIP_MSG_PRE_ARG, pCtx->options.ac3ServiceType));
            }
            else
                BDBG_WRN((BIP_MSG_PRE_FMT "Can't change audio track since |%d| bsmod specific audio track doesn't exist."
                          BIP_MSG_PRE_ARG, intArg));
            break;
        case BIP_Play_RuntimeCmd_eLast:
            /* Repeat last command if the last command was not 'last' */
            if (pCtx->lastCmd.command != BIP_Play_RuntimeCmd_eLast)
                bipStatus = BIP_Play_HandleRuntimeCmd(pCtx, &pCtx->lastCmd);
            break;
        case BIP_Play_RuntimeCmd_eQuit:
        default:
            break;
    }

    /* Update GUI to reflect playback rate change */
    if (pCtx->hGui && rateChanged)
        BIP_Play_GuiSetRate(pCtx->hGui, pCtx->rate);

error:
    BDBG_LEAVE(BIP_Play_HandleRuntimeCmd);
    return bipStatus;
}

static void BIP_Play_HandleRemoteKey(
        BIP_Play_Context *pCtx, /* In */
        b_remote_key key, /* In */
        BIP_Play_ParsedRuntimeCmdInfo *pCmd /* Out */
    )
{
    pCmd->command = BIP_Play_RuntimeCmd_eMax;

    BDBG_ENTER(BIP_Play_HandleRemoteKey);

    /* Translate remote key to run time command info */
    switch (key)
    {
        case b_remote_key_play:
            pCmd->command = BIP_Play_RuntimeCmd_ePlay; break;
        case b_remote_key_pause:
            pCmd->command = BIP_Play_RuntimeCmd_ePause; break;
        case b_remote_key_fast_forward:
            pCmd->command = BIP_Play_RuntimeCmd_ePlayAtRate;
            if (pCtx->rate == 1.0) sprintf(pCmd->strArg, "2");
            else if (pCtx->rate == 2.0) sprintf(pCmd->strArg, "4");
            else if (pCtx->rate == 4.0) sprintf(pCmd->strArg, "8");
            else if (pCtx->rate == 8.0) sprintf(pCmd->strArg, "16");
            else sprintf(pCmd->strArg, "1");
            break;
        case b_remote_key_rewind:
            pCmd->command = BIP_Play_RuntimeCmd_ePlayAtRate;
            if (pCtx->rate == 1) sprintf(pCmd->strArg, "-2");
            else if (pCtx->rate == -2.0) sprintf(pCmd->strArg, "-4");
            else if (pCtx->rate == -4.0) sprintf(pCmd->strArg, "-8");
            else if (pCtx->rate == -8.0) sprintf(pCmd->strArg, "-16");
            else sprintf(pCmd->strArg, "-1");
            break;
        case b_remote_key_stop:
            pCmd->command = BIP_Play_RuntimeCmd_ePause; break;
        case b_remote_key_clear:
        case b_remote_key_back:
            BIP_Play_GuiHideStatus(pCtx->hGui);
            break;
        case b_remote_key_down:
            BIP_Play_GuiShowStatus(pCtx->hGui);
            break;
        case b_remote_key_right:
            pCmd->command = BIP_Play_RuntimeCmd_eJumpFwd; break;
        case b_remote_key_left:
            pCmd->command = BIP_Play_RuntimeCmd_eJumpBack; break;
        case b_remote_key_select:
            pCmd->command = BIP_Play_RuntimeCmd_eTogglePlay; break;
        case b_remote_key_chan_up:
            pCmd->command = BIP_Play_RuntimeCmd_eTune;
            pCmd->intArg = (pCtx->playingNow + 1) % pCtx->numUrls;
            pCmd->intArg++;
            break;
        case b_remote_key_chan_down:
            pCmd->command = BIP_Play_RuntimeCmd_eTune;
            pCmd->intArg = (pCtx->playingNow + pCtx->numUrls - 1) % pCtx->numUrls;
            pCmd->intArg++;
            break;
        default:
            break;
    }

    BDBG_LEAVE(BIP_Play_HandleRemoteKey);
}

static void BIP_Play_CheckAndPrintStatus(
        BIP_Play_Context *pCtx /* In */
    )
{
    struct timeval now;
    unsigned timeSinceLast;
    static struct timeval last = {0};
    BIP_PlayerGetStatusFromServerSettings serverSettings;

    B_Time_Get(&now);
    timeSinceLast = B_Time_DiffMicroseconds(&now, &last)/1000;

    if (timeSinceLast > BIP_PLAY_STATUS_PRINT_PERIOD)
    {
        if (pCtx->options.printStatus)
            BIP_Player_PrintStatus(pCtx->hPlayer);

        /* Print player status */
        if (BIP_SUCCESS == BIP_Player_GetStatus(pCtx->hPlayer, &pCtx->playerStatus))
        {
            if (pCtx->options.printStatus)
                BDBG_LOG((BIP_MSG_PRE_FMT "Player position cur/last: %0.3f/%0.3f sec " BIP_MSG_PRE_ARG,
                          pCtx->playerStatus.currentPositionInMs/1000.,
                          pCtx->playerStatus.lastPositionInMs/1000.));
            /* Update GUI */
            if (pCtx->hGui)
            {
                BIP_Play_GuiSetPosition(pCtx->hGui, pCtx->playerStatus.currentPositionInMs);
                BIP_Play_GuiSetPlayerStatus(pCtx->hGui, &pCtx->playerStatus);
            }
        }

        /* Print server status */
        if (pCtx->options.printServerStats)
        {
            BIP_Player_GetDefaultGetStatusFromServerSettings(&serverSettings);
            serverSettings.getAvailableSeekRange = true;
            serverSettings.timeoutInMs = BIP_PLAY_STATUS_PRINT_PERIOD >> 1;

            if (BIP_SUCCESS == BIP_Player_GetStatusFromServer(pCtx->hPlayer, &serverSettings, &pCtx->serverStatus))
                BDBG_LOG((BIP_MSG_PRE_FMT "Player position cur/last: %0.3f/%0.3f sec " BIP_MSG_PRE_ARG,
                          pCtx->serverStatus.currentPositionInMs/1000.,
                          pCtx->serverStatus.availableSeekRange.lastPositionInMs/1000.));
        }

        /* Get audio video decode status and update gui */
        if (!pCtx->options.disableVideo)
        {
            NEXUS_VideoDecoderStatus status;
#if NXCLIENT_SUPPORT
            NEXUS_SimpleVideoDecoder_GetStatus(pCtx->hSimpleVideoDecoder, &status);
#else
            NEXUS_VideoDecoder_GetStatus(pCtx->videoDecoder, &status);
#endif
            BIP_Play_GuiSetVideoDecoderStatus(pCtx->hGui, &status);
        }
        if (!pCtx->options.disableAudio)
        {
            NEXUS_AudioDecoderStatus status;
#if NXCLIENT_SUPPORT
            NEXUS_SimpleAudioDecoder_GetStatus(pCtx->hSimpleAudioDecoder, &status);
#else
            NEXUS_AudioDecoder_GetStatus(pCtx->pcmDecoder, &status);
#endif
            BIP_Play_GuiSetAudioDecoderStatus(pCtx->hGui, &status);
        }

        /* Update time */
        last = now;
    }
}

static void BIP_Play_PrintCommandCompletionStatus(
        BIP_Play_ParsedRuntimeCmdInfo *pCmd,
        BIP_Status bipStatus
    )
{
    if (pCmd->command != BIP_Play_RuntimeCmd_eNone)
        printf("bip_play: Command completed : \"%s\" - Status = 0x%x (%s)\n",
                BIP_Play_LookupRuntimeCommand(pCmd->command),
                bipStatus, BIP_StatusGetText(bipStatus));
}

static float BIP_Play_FractToFloat(char *str)
{
    int den, num;
    float ret;
    char *saveptr, *token, *delim = "/";

    token = strtok_r(str, delim, &saveptr);
    if (token)
    {
        sscanf(token, "%d", &num);
        token = strtok_r(NULL, delim, &saveptr);
        if (token)
           sscanf(token, "%d", &den);
        else
            den = 1;
        ret = (float)num/(float)den;
    }
    else
    {
        sscanf(str, "%f", &ret);
    }

    return ret;
}
