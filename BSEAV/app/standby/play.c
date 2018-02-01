/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.

 ******************************************************************************/

#include "standby.h"
#include "util.h"
#include "media_probe.h"

BDBG_MODULE(play);

extern B_StandbyNexusHandles g_StandbyNexusHandles;
extern B_DeviceState g_DeviceState;

#if BIP_SUPPORT
static BIP_PlayerHandle bip_create(void)
{
    BIP_PlayerHandle hPlayer = NULL;
    BIP_Status bipStatus;

    /* Initialize BIP */
    bipStatus = BIP_Init(NULL);
    BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_Init Failed" ), errorBipInit, bipStatus, bipStatus );

#if 0
    /* Initialize DtcpIpClientFactory */
    bipStatus = BIP_DtcpIpClientFactory_Init(NULL);
    BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS || bipStatus == BIP_ERR_NOT_SUPPORTED), ( "BIP_DtcpIpClientFactory_Init Failed" ), errorDtcpIpFactoryInit, bipStatus, bipStatus );
#endif

    /* Initialize SslClientFactory */
    {
        BIP_SslClientFactoryInitSettings settings;

        BIP_SslClientFactory_GetDefaultInitSettings(&settings);
        settings.pRootCaCertPath = "./host.cert";
        bipStatus = BIP_SslClientFactory_Init(&settings);
        BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS || bipStatus == BIP_ERR_NOT_SUPPORTED), ( "BIP_SslClientFactory_Init Failed" ), errorSslClientFactoryInit, bipStatus, bipStatus );
    }

    hPlayer = BIP_Player_Create( NULL );
    BIP_CHECK_GOTO(( hPlayer ), ( "BIP_Player_Create Failed" ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );

    return hPlayer;

error:
errorSslClientFactoryInit:
    BIP_SslClientFactory_Uninit();
errorDtcpIpFactoryInit:
    BIP_DtcpIpClientFactory_Uninit();
errorBipInit:
    BIP_Uninit();

    return NULL;
}

static void bip_destroy(BIP_PlayerHandle hPlayer)
{
    if(hPlayer)
    {
        BIP_Player_Destroy(hPlayer);
    }

    BIP_SslClientFactory_Uninit();
    BIP_DtcpIpClientFactory_Uninit();
    BIP_Uninit();
}
#endif

void playback_open(unsigned id)
{
    NEXUS_PlaybackSettings playbackSettings;

    if(g_StandbyNexusHandles.playback[id])
        return;

    g_StandbyNexusHandles.playpump[id] = NEXUS_Playpump_Open(id, NULL);
    BDBG_ASSERT(g_StandbyNexusHandles.playpump[id]);
    g_StandbyNexusHandles.playback[id] = NEXUS_Playback_Create();
    BDBG_ASSERT(g_StandbyNexusHandles.playback[id]);

    NEXUS_Playback_GetSettings(g_StandbyNexusHandles.playback[id], &playbackSettings);
    playbackSettings.playpump = g_StandbyNexusHandles.playpump[id];
    playbackSettings.stcChannel = g_StandbyNexusHandles.stcChannel[id];
    NEXUS_Playback_SetSettings(g_StandbyNexusHandles.playback[id], &playbackSettings);

#if BIP_SUPPORT
    g_StandbyNexusHandles.bipPlayer[id] = bip_create();
    BDBG_ASSERT(g_StandbyNexusHandles.bipPlayer[id]);
#endif
}

void playback_close(unsigned id)
{
    if(g_StandbyNexusHandles.playback[id])
        NEXUS_Playback_Destroy(g_StandbyNexusHandles.playback[id]);
    g_StandbyNexusHandles.playback[id] = NULL;
    if(g_StandbyNexusHandles.playpump[id])
        NEXUS_Playpump_Close(g_StandbyNexusHandles.playpump[id]);
    g_StandbyNexusHandles.playpump[id] = NULL;

#if BIP_SUPPORT
    if (g_StandbyNexusHandles.bipPlayer[id])
        bip_destroy(g_StandbyNexusHandles.bipPlayer[id]);
    g_StandbyNexusHandles.bipPlayer[id] = NULL;
#endif
}

#undef min
#define min(A,B) ((A)<(B)?(A):(B))

/* parse_url()

example: http://player.vimeo.com:80/play_redirect?quality=hd&codecs=h264&clip_id=638324

scheme=http
domain=player.vimeo.com
port=80
path=/play_redirect?quality=hd&codecs=h264&clip_id=638324

example: file://videos/cnnticker.mpg or videos/cnnticker.mpg

scheme=file
domain=
port=0
path=videos/cnnticker.mpg

 */
static void parse_url(const char *s, struct url_t *url)
{
    const char *server, *file;

    memset(url, 0, sizeof(*url));

    server = strstr(s, "://");
    if (!server) {
        strcpy(url->scheme, "file");
        server = s;
    }
    else {
        strncpy(url->scheme, s, server-s);
        server += strlen("://"); /* points to the start of server name */
    }

    if (!strcmp(url->scheme, "file")) {
        strncpy(url->path, server, sizeof(url->path)-1);
    }
    else {
        char *port;
        file = strstr(server, "/"); /* should point to start of file name */
        if (file) {
            strncpy(url->domain, server, min(sizeof(url->domain)-1, (unsigned)(file-server)));
            strncpy(url->path, file, sizeof(url->path)-1);
        }
        else {
            strncpy(url->domain, server, sizeof(url->domain)-1);
        }

        /* now server string is null terminated, look for explicit port number in the format of server:port */
        port = strstr(url->domain, ":");
        if (port) {
            *port = 0;
            url->port = atoi(port+1);
        }
        else {
            url->port = 80; /* default port */
        }
    }
}

#if BIP_SUPPORT
static bool playerGetTrackOfType(
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
            BDBG_MSG(( BIP_MSG_PRE_FMT "Found trackType=%s with trackId=%d" BIP_MSG_PRE_ARG, BIP_ToStr_BIP_MediaInfoTrackType(trackType), pMediaInfoTrack->trackId));
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
    return (trackFound);
} /* getTrackOfType */

static void playbackDoneCallbackFromBIP(
    void *context,
    int   param
    )
{
    BSTD_UNUSED( context );
    BSTD_UNUSED( param );
    BDBG_WRN(( BIP_MSG_PRE_FMT "Got endOfStreamer Callback from BIP)" BIP_MSG_PRE_ARG ));
}

int bip_p_setup(unsigned id)
{
    BIP_PlayerHandle hPlayer = g_StandbyNexusHandles.bipPlayer[id];
    BIP_Status bipStatus;
    BIP_MediaInfoHandle hMediaInfo;
    BIP_PlayerStreamInfo       playerStreamInfo;
    NEXUS_AudioCodec    audioCodec = NEXUS_AudioCodec_eUnknown;
    NEXUS_VideoCodec    videoCodec = NEXUS_VideoCodec_eUnknown;
    const char *url = g_DeviceState.playfile[id];
    struct opts_t *opts = &g_DeviceState.opts[id];

    BDBG_ASSERT(g_StandbyNexusHandles.bipPlayer[id]);
    /* Connect to Server */
    {
        BIP_PlayerConnectSettings settings;
        BIP_Player_GetDefaultConnectSettings(&settings);
        settings.pUserAgent = "BIP Player";
        settings.pAdditionalHeaders = "getAvailableSeekRange.dlna.org:1\r\n";
        bipStatus = BIP_Player_Connect( hPlayer, url, &settings );
        BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_Player_Connect Failed to Connect to URL=%s", url ), errorConnect, bipStatus, bipStatus );
    }
    BDBG_MSG(( BIP_MSG_PRE_FMT "Player connected for url:|%s|" BIP_MSG_PRE_ARG, url));

    /* Probe media to find Video and Audio decoder related information.*/
    {
        BIP_PlayerProbeMediaInfoSettings   probeMediaInfoSettings;
        BIP_Player_GetDefaultProbeMediaInfoSettings( &probeMediaInfoSettings );
        bipStatus = BIP_Player_ProbeMediaInfo(hPlayer, &probeMediaInfoSettings, &hMediaInfo );
        BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_Player_Probe Failed!"), error, bipStatus, bipStatus );
        BDBG_MSG(( BIP_MSG_PRE_FMT "BIP_Player_ProbeMediaInfo is done." BIP_MSG_PRE_ARG));

        /* Map the MediaStream Info to the Player's StreamInfo. */
        bipStatus = BIP_Player_GetProbedStreamInfo( hPlayer, &playerStreamInfo );
        BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_Player_GetProbedStreamInfo Failed!"), error, bipStatus, bipStatus );
    }

    /*BIP_MediaInfo_Print(hMediaInfo);*/

    /* Now once the probing is done , prepare the BIP Player */
    {
        BIP_PlayerPrepareSettings  prepareSettings;
        BIP_PlayerSettings         playerSettings;
        BIP_PlayerPrepareStatus    prepareStatus;
        BIP_MediaInfoTrack         mediaInfoTrack;;


        BIP_Player_GetDefaultPrepareSettings(&prepareSettings);
        BIP_Player_GetDefaultSettings(&playerSettings);

        /* Get first Video & Audio track entry from pMediaInfoStream .*/
        if ( hMediaInfo )
        {
            if (playerGetTrackOfType(hMediaInfo, BIP_MediaInfoTrackType_eVideo, &mediaInfoTrack) )
            {
               playerSettings.videoTrackId = mediaInfoTrack.trackId;
               playerSettings.videoTrackSettings.pidTypeSettings.video.codec = mediaInfoTrack.info.video.codec;
               videoCodec = mediaInfoTrack.info.video.codec;
               opts->width = mediaInfoTrack.info.video.width;
               opts->height = mediaInfoTrack.info.video.height;
               opts->videoPid = mediaInfoTrack.trackId;
               BDBG_MSG(( BIP_MSG_PRE_FMT "Found a valid Video Track with trackId=%d , codec=%s" BIP_MSG_PRE_ARG, playerSettings.videoTrackId,BIP_ToStr_NEXUS_VideoCodec(videoCodec)));
            }

            if (playerGetTrackOfType(hMediaInfo, BIP_MediaInfoTrackType_eAudio, &mediaInfoTrack) )
            {
                playerSettings.audioTrackId = mediaInfoTrack.trackId;
                playerSettings.audioTrackSettings.pidSettings.pidTypeSettings.audio.codec = mediaInfoTrack.info.audio.codec;
                audioCodec = mediaInfoTrack.info.audio.codec;
                opts->audioPid = mediaInfoTrack.trackId;
                BDBG_MSG(( BIP_MSG_PRE_FMT "Found a valid Audio Track with trackId=%d , codec=%s" BIP_MSG_PRE_ARG, playerSettings.audioTrackId,BIP_ToStr_NEXUS_AudioCodec(audioCodec)));
            }

            if (playerStreamInfo.transportType == NEXUS_TransportType_eTs)
               {
                   if (playerGetTrackOfType(hMediaInfo, BIP_MediaInfoTrackType_ePcr, &mediaInfoTrack))
                   {
                       playerStreamInfo.mpeg2Ts.pcrPid = mediaInfoTrack.trackId;
                   }
                   else
                   {
                       BDBG_WRN((BIP_MSG_PRE_FMT "Not able to found any PCR track" BIP_MSG_PRE_ARG));
                   }
               }

            /* Make sure Probe found atleast a Video or an Audio Track. */
            BIP_CHECK_GOTO( (playerSettings.videoTrackId != UINT_MAX || playerSettings.audioTrackId != UINT_MAX), ( "Neither Audio nor Video Tracks found for URL=%s", url ), error, bipStatus, bipStatus );
        }

        {
            playerSettings.playbackSettings.endOfStreamCallback.callback = playbackDoneCallbackFromBIP;
            playerSettings.playbackSettings.errorCallback.callback = playbackDoneCallbackFromBIP;
            playerSettings.playbackSettings.endOfStreamAction = NEXUS_PlaybackLoopMode_eLoop;

            if (audioCodec != NEXUS_AudioCodec_eUnknown)
            {
                playerSettings.audioTrackSettings.pidTypeSettings.audio.primary = g_StandbyNexusHandles.audioDecoder[id];
                playerSettings.hDisplay = g_StandbyNexusHandles.displayHD;
                playerSettings.hWindow = g_StandbyNexusHandles.windowHD[id];
            }
            if (videoCodec != NEXUS_VideoCodec_eUnknown)
            {
                playerSettings.videoTrackSettings.pidTypeSettings.video.decoder = g_StandbyNexusHandles.videoDecoder[id];
            }
            playerSettings.playbackSettings.stcChannel = g_StandbyNexusHandles.stcChannel[id];

            bipStatus = BIP_Player_Prepare(hPlayer, &prepareSettings, &playerSettings, NULL, &playerStreamInfo, &prepareStatus);
            BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_Player_Prepare Failed: URL=%s", url ), error, bipStatus, bipStatus );
            BDBG_MSG(( BIP_MSG_PRE_FMT "BIP_Player_Prepare: prepared" BIP_MSG_PRE_ARG));


            if ((videoCodec != NEXUS_VideoCodec_eUnknown))
            {
                opts->videoCodec = videoCodec;
                g_StandbyNexusHandles.videoPidChannel[id] = prepareStatus.hVideoPidChannel;
            }

            if ((prepareStatus.audioCodec != NEXUS_AudioCodec_eUnknown))
            {
                opts->audioCodec = prepareStatus.audioCodec;
                g_StandbyNexusHandles.audioPidChannel[id] = prepareStatus.hAudioPidChannel;
            }
        }
    }

    return 0;
error:
    /* Disconnect */
    BIP_Player_Disconnect(hPlayer);
errorConnect:
    return bipStatus;
}
#endif

static void playback_p_setup(unsigned id)
{
    NEXUS_PlaybackSettings playbackSettings;
    NEXUS_PlaybackPidChannelSettings playbackPidSettings;

    NEXUS_Playback_GetSettings(g_StandbyNexusHandles.playback[id], &playbackSettings);
    playbackSettings.playpumpSettings.transportType = g_DeviceState.opts[id].transportType;
    NEXUS_Playback_SetSettings(g_StandbyNexusHandles.playback[id], &playbackSettings);

    NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
    playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eVideo;
    playbackPidSettings.pidTypeSettings.video.codec = g_DeviceState.opts[id].videoCodec;
    playbackPidSettings.pidTypeSettings.video.index = true;
    playbackPidSettings.pidTypeSettings.video.decoder = g_StandbyNexusHandles.videoDecoder[id];
    g_StandbyNexusHandles.videoPidChannel[id] = NEXUS_Playback_OpenPidChannel(g_StandbyNexusHandles.playback[id], g_DeviceState.opts[id].videoPid, &playbackPidSettings);

    NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
    playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eAudio;
    playbackPidSettings.pidTypeSettings.audio.primary = g_StandbyNexusHandles.audioDecoder[id];
    g_StandbyNexusHandles.audioPidChannel[id] = NEXUS_Playback_OpenPidChannel(g_StandbyNexusHandles.playback[id], g_DeviceState.opts[id].audioPid, &playbackPidSettings);

}

int playback_setup(unsigned id)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    struct url_t url;

    parse_url(g_DeviceState.playfile[id], &url);
    g_DeviceState.url = url;

    if (!strcasecmp(url.scheme, "http") || !strcasecmp(url.scheme, "https") || !strcasecmp(url.scheme, "udp") || !strcasecmp(url.scheme, "rtp")) {
        /* Playback Ip Setup */
#if BIP_SUPPORT
        rc = bip_p_setup(id);
        g_DeviceState.playbackIpActive[id] = true;
#else
        fprintf(stderr, "PLAYBACK_IP_SUPPORT not enabled!!\n");
#endif
    } else {
        struct probe_request probe_request;
        struct probe_results probe_results;

        probe_media_get_default_request(&probe_request);
        probe_request.streamname = url.path;
        rc = probe_media_request(&probe_request, &probe_results);
        if(rc) { rc = BERR_TRACE(rc); goto err; }

        g_DeviceState.opts[id].transportType = probe_results.transportType;
        g_DeviceState.opts[id].audioPid = probe_results.audio[0].pid;
        g_DeviceState.opts[id].audioCodec = probe_results.audio[0].codec;
        g_DeviceState.opts[id].videoPid = probe_results.video[0].pid;
        g_DeviceState.opts[id].videoCodec = probe_results.video[0].codec;
        g_DeviceState.opts[id].width= probe_results.video[0].width;
        g_DeviceState.opts[id].height = probe_results.video[0].height;

        playback_p_setup(id);
    }

    if(g_DeviceState.power_mode == ePowerModeS0)
        decoder_setup(id);

err:
    return rc;
}

int playback_start(unsigned id)
{
    NEXUS_Error rc=0;
    char *navfile=NULL;

    if(g_DeviceState.playback_started[id])
        return rc;

    if(g_DeviceState.playbackIpActive[id] ) {
#if BIP_SUPPORT
        BIP_PlayerStartSettings startSettings;
        BIP_Status bipStatus;
        unsigned cnt = 300;

        while(!isIpReachable(g_DeviceState.url.domain, g_DeviceState.url.port) && cnt) {
            BKNI_Sleep(100);
            cnt--;
        }
        if(!cnt) {
            BDBG_WRN(("Timed out waiting for network"));
            goto err;
        }
        BIP_Player_GetDefaultStartSettings(&startSettings);
        startSettings.videoStartSettings = g_StandbyNexusHandles.videoProgram[id];
        startSettings.audioStartSettings = g_StandbyNexusHandles.audioProgram[id];
        bipStatus = BIP_Player_Start(g_StandbyNexusHandles.bipPlayer[id], &startSettings);
        BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_Player_Start Failed" ), err, bipStatus, rc );
        BDBG_MSG(( BIP_MSG_PRE_FMT "BIP_Player_Start: started" BIP_MSG_PRE_ARG));
#endif
    } else
    {
        if(g_DeviceState.opts[id].transportType == NEXUS_TransportType_eMkv ||
                g_DeviceState.opts[id].transportType == NEXUS_TransportType_eMp4 ) {
            navfile = (char *)g_DeviceState.playfile[id];
        }
        g_StandbyNexusHandles.filePlay[id] = NEXUS_FilePlay_OpenPosix(g_DeviceState.playfile[id], navfile);
        if (!g_StandbyNexusHandles.filePlay[id]) {
            fprintf(stderr, "CANNOT OPEN PLAYBACK FILE %s\n", g_DeviceState.playfile[id]);
            goto err;
        }

        /* Start playback */
        rc = NEXUS_Playback_Start(g_StandbyNexusHandles.playback[id], g_StandbyNexusHandles.filePlay[id], NULL);
        if (rc) { BERR_TRACE(rc); goto err;}
    }

    g_DeviceState.playback_started[id] = true;

err:
    return rc;
}
int start_play_context(unsigned id)
{
    NEXUS_Error rc;

    if(g_DeviceState.playfile[id] == NULL) {
        BDBG_WRN(("No input file specified\n"));
        rc = NEXUS_UNKNOWN; rc = BERR_TRACE(rc); goto playback_err;
    }

    printf("Playing file %s\n", g_DeviceState.playfile[id]);

    rc = playback_setup(id);
    if(rc) { rc = BERR_TRACE(rc); goto playback_err; }

    rc = playback_start(id);
    if(rc) { rc = BERR_TRACE(rc); goto playback_err; }

    if(g_DeviceState.power_mode == ePowerModeS0) {
        set_max_decode_size(id);
        add_window_input(id);
        rc = decode_start(id);
        if(rc) { rc = BERR_TRACE(rc); goto decode_err; }
    } else {
        /* We might be in S1 mode and cannot start decoder upon resume
           Set the input source to none, so app does not attempt to star decode */
        g_DeviceState.source[id] = eInputSourceNone;
    }

    return rc;

decode_err:
    stop_play_context(id);
playback_err:
    return rc;
}

void playback_stop(unsigned id)
{
    if(!g_DeviceState.playback_started[id])
        return;

    if (g_DeviceState.playbackIpActive[id]) {
#if BIP_SUPPORT
        BIP_PlayerHandle hPlayer = g_StandbyNexusHandles.bipPlayer[id];
        BIP_Player_Stop(hPlayer);
        if(g_DeviceState.power_mode == ePowerModeS0) {
            BIP_Player_Disconnect(hPlayer);
            BDBG_MSG(( BIP_MSG_PRE_FMT "BIP_Player_Disconnect:disconnected" BIP_MSG_PRE_ARG));
        }
#endif
    } else
    {
        /*Stop playback */
        NEXUS_Playback_Stop(g_StandbyNexusHandles.playback[id]);
        /* Close File. Required for umount */
        NEXUS_FilePlay_Close(g_StandbyNexusHandles.filePlay[id]);
    }



    g_DeviceState.playback_started[id] = false;
}

void stop_play_context(unsigned id)
{
    playback_stop(id);
    if(g_DeviceState.power_mode == ePowerModeS0) {
        decode_stop(id);
        remove_window_input(id);
    }
    g_DeviceState.source[id] = eInputSourceNone;
    g_DeviceState.playfile[id] = NULL;
}

int record_start(unsigned id)
{
    NEXUS_Error rc=0;

#if NEXUS_HAS_RECORD
    NEXUS_RecordPidChannelSettings pidSettings;
    char *mpgfile = id==0?"videos/record0.mpg":"videos/record1.mpg";
    char *navfile = id==0?"videos/record0.nav":"videos/record1.nav";

    if(g_DeviceState.record_started[id])
        return rc;

    g_StandbyNexusHandles.fileRec[id] = NEXUS_FileRecord_OpenPosix(mpgfile, navfile);
    if (!g_StandbyNexusHandles.fileRec[id]) {
        fprintf(stderr, "CANNOT OPEN RECORD FILE %d\n", id);
        goto err;
    }

    /* configure the video pid for indexing */
    NEXUS_Record_GetDefaultPidChannelSettings(&pidSettings);
    pidSettings.recpumpSettings.pidType = NEXUS_PidType_eVideo;
    pidSettings.recpumpSettings.pidTypeSettings.video.index = true;
    pidSettings.recpumpSettings.pidTypeSettings.video.codec = g_DeviceState.opts[id].videoCodec;
    NEXUS_Record_AddPidChannel(g_StandbyNexusHandles.record[id], g_StandbyNexusHandles.videoPidChannel[id], &pidSettings);

    NEXUS_Record_AddPidChannel(g_StandbyNexusHandles.record[id], g_StandbyNexusHandles.audioPidChannel[id], NULL);

    rc = NEXUS_Record_Start(g_StandbyNexusHandles.record[id], g_StandbyNexusHandles.fileRec[id]);
    if (rc) { BERR_TRACE(rc); goto err;}

    g_DeviceState.record_started[id] = true;

err:
#endif

    return rc;
}

void record_stop(unsigned id)
{
#if NEXUS_HAS_RECORD
    if(!g_DeviceState.record_started[id])
        return;

    NEXUS_Record_Stop(g_StandbyNexusHandles.record[id]);
    NEXUS_Record_RemoveAllPidChannels(g_StandbyNexusHandles.record[id]);
    NEXUS_FileRecord_Close(g_StandbyNexusHandles.fileRec[id]);

    g_DeviceState.record_started[id] = false;
#endif
}

void record_open(unsigned id)
{
#if NEXUS_HAS_RECORD
    NEXUS_RecordSettings recordSettings;

    if(g_StandbyNexusHandles.record[id])
        return;

    g_StandbyNexusHandles.recpump[id] = NEXUS_Recpump_Open(id, NULL);
    BDBG_ASSERT(g_StandbyNexusHandles.recpump[id]);
    g_StandbyNexusHandles.record[id] = NEXUS_Record_Create();
    BDBG_ASSERT(g_StandbyNexusHandles.record[id]);

    NEXUS_Record_GetSettings(g_StandbyNexusHandles.record[id], &recordSettings);
    recordSettings.recpump = g_StandbyNexusHandles.recpump[id];
    NEXUS_Record_SetSettings(g_StandbyNexusHandles.record[id], &recordSettings);
#endif
}

void record_close(unsigned id)
{
#if NEXUS_HAS_RECORD
    if(g_StandbyNexusHandles.record[id])
        NEXUS_Record_Destroy(g_StandbyNexusHandles.record[id]);
    g_StandbyNexusHandles.record[id] = NULL;
    if(g_StandbyNexusHandles.recpump[id])
        NEXUS_Recpump_Close(g_StandbyNexusHandles.recpump[id]);
    g_StandbyNexusHandles.recpump[id] = NULL;
#endif
}
