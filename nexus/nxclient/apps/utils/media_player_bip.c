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
#include "media_player.h"

#if NEXUS_HAS_PLAYBACK && NEXUS_HAS_SIMPLE_DECODER
#include "media_player_priv.h"
#if PLAYBACK_IP_SUPPORT
#include "bip.h"

BDBG_MODULE(media_player);

BDBG_OBJECT_ID(media_player_bip);

typedef struct
{
    int speedNumerator;
    int speedDenominator;
}playSpeedEntry;

struct media_player_bip
{
    media_player_t          parent;
    BIP_PlayerHandle        hPlayer;
    BIP_MediaInfoHandle     hMediaInfo;

    unsigned                colorDepth;
    unsigned                initialPlaybackPositionInMs;
    const char              *pPlaySpeedString;
    bool                    playSpeedEntryPresent;
    BIP_HttpResponseHandle  hResponseHandle;
#define MAX_PLAYSPEED_ENTRIES 16
    playSpeedEntry playSpeedFwdList[MAX_PLAYSPEED_ENTRIES];
    playSpeedEntry playSpeedRwdList[MAX_PLAYSPEED_ENTRIES];
    unsigned maxFwdSpeedIndex;
    unsigned maxRwdSpeedIndex;
};

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
    media_player_bip_t   player = (media_player_bip_t)context;

    BSTD_UNUSED( param );
    BDBG_WRN(( BIP_MSG_PRE_FMT "Got endOfStreamer Callback from BIP: eof CB ctx %p)" BIP_MSG_PRE_ARG, player->parent->start_settings.context ));
    if (player->parent->start_settings.eof) {
        (player->parent->start_settings.eof)(player->parent->start_settings.context);
    }
}

media_player_bip_t media_player_bip_create(media_player_t parent)
{
    media_player_bip_t bipPlayer = NULL;
    BIP_Status bipStatus;

    /* Initialize BIP */
    bipStatus = BIP_Init(NULL);
    BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_Init Failed" ), errorBipInit, bipStatus, bipStatus );

    /* Initialize DtcpIpClientFactory */
    bipStatus = BIP_DtcpIpClientFactory_Init(NULL);
    BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS || bipStatus == BIP_ERR_NOT_SUPPORTED), ( "BIP_DtcpIpClientFactory_Init Failed" ), errorDtcpIpFactoryInit, bipStatus, bipStatus );

    /* Initialize SslClientFactory */
    {
        BIP_SslClientFactoryInitSettings settings;

        BIP_SslClientFactory_GetDefaultInitSettings(&settings);
        settings.pRootCaCertPath = "./host.cert";
        bipStatus = BIP_SslClientFactory_Init(&settings);
        BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS || bipStatus == BIP_ERR_NOT_SUPPORTED), ( "BIP_SslClientFactory_Init Failed" ), errorSslClientFactoryInit, bipStatus, bipStatus );
    }

    bipPlayer = (media_player_bip_t)BKNI_Malloc(sizeof(*bipPlayer));

    memset (bipPlayer, 0, sizeof(*bipPlayer));

    bipPlayer->parent = parent;

    bipPlayer->hPlayer = BIP_Player_Create( NULL );
    BIP_CHECK_GOTO(( bipPlayer->hPlayer ), ( "BIP_Player_Create Failed" ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );

    return bipPlayer;

error:
    if (bipPlayer) BKNI_Free(bipPlayer);

errorSslClientFactoryInit:
    BIP_SslClientFactory_Uninit();
errorDtcpIpFactoryInit:
    BIP_DtcpIpClientFactory_Uninit();
errorBipInit:
    BIP_Uninit();

    return NULL;
}

extern int B_PlaybackIp_UtilsParsePlaySpeedString(const char *playSpeedStringOrig, int *speedNumerator, int *speedDenominator, int *direction);
static int parsePlaySpeedString(
    media_player_bip_t player,
    const char *pContentFeatureHeader
    )
{
    /*
    * Server typically sends the playSpeedString in the following format:
    * e.g. given a playSpeedString like -16, -8, -4, -1, -1/8, -1/4, 1/4, 1/8, 4, 8, 16:
    *
    * When doing trickmodes, app will provide the playSpeed string the playback_ip.
    *
    * This parsing logic makes it easier to find the next speed string.
    * We separate out the -ve & +ve speeds.
    */
    unsigned i, j, k;
    /* +ve speeds */
    char *savePtr;
    unsigned playSpeedStringLength;
    char *playSpeedString = NULL;
    char *nextSpeed;
    const char *pPlaySpeed = NULL;
    int rc = 0;
    /* From contentFeature header value look for DLNA.ORG_PS parameter value.*/
    pPlaySpeed = strstr(pContentFeatureHeader, "DLNA.ORG_PS");

    if(pPlaySpeed == NULL)
    {
        BDBG_MSG(("PlaySpeed Param:DLNA.ORG_PS not available."));
        rc = 0;
        goto error;
    }

    pPlaySpeed += strlen("DLNA.ORG_PS=");

    playSpeedStringLength = strlen(pPlaySpeed) + 1;
    if ( (playSpeedString = (char *)BKNI_Malloc(playSpeedStringLength)) == NULL)
    {
        BDBG_ERR(("%s: BKNI_Malloc failed to allocate %d bytes string for playSpeed", BSTD_FUNCTION, playSpeedStringLength));
        rc = -1;
        goto error;
    }
    strncpy(playSpeedString, pPlaySpeed, playSpeedStringLength-1);
    playSpeedString[playSpeedStringLength-1] = '\0';

    for (i=0, j=0, nextSpeed = strtok_r(playSpeedString, ",", &savePtr);
            nextSpeed != NULL;
            nextSpeed = strtok_r(NULL, ",", &savePtr)
        )
    {
        int speedNumerator, speedDenominator;
        if (B_PlaybackIp_UtilsParsePlaySpeedString(nextSpeed, &speedNumerator, &speedDenominator, NULL) < 0) {
            BDBG_ERR(("%s: Failed to parse the playSpeedString %s", BSTD_FUNCTION, nextSpeed));
            rc = -1;
            goto error;
        }
        if (speedNumerator < 0)
        {
            /* -ve speeds */
            player->playSpeedRwdList[j].speedNumerator = speedNumerator;
            player->playSpeedRwdList[j].speedDenominator = speedDenominator;
            j++;
        }
        else
        {
            /* +ve speeds */
            player->playSpeedFwdList[i].speedNumerator = speedNumerator;
            player->playSpeedFwdList[i].speedDenominator = speedDenominator;
            i++;
        }
    }

    if(i || j) {
        player->playSpeedEntryPresent = true;
    }

    player->maxFwdSpeedIndex = i-1;
    player->maxRwdSpeedIndex = j-1;
    for (k=0; k<i;k++)
        BDBG_MSG(("%s: +ve speed idx %d, num %d, denom %d", BSTD_FUNCTION,
                k, player->playSpeedFwdList[k].speedNumerator, player->playSpeedFwdList[k].speedDenominator));
    for (k=0; k<j;k++)
        BDBG_MSG(("%s: -ve speed idx %d, num %d, denom %d", BSTD_FUNCTION,
                k, player->playSpeedRwdList[k].speedNumerator, player->playSpeedRwdList[k].speedDenominator));


error:
    if(playSpeedString)
    {
        BKNI_Free(playSpeedString);
    }
    return rc;
}

void media_player_bip_get_color_depth(media_player_bip_t player, unsigned *pColorDepth)
{
    if(player->colorDepth)
    {
        *pColorDepth = player->colorDepth;
    }
}

int media_player_bip_start(media_player_bip_t player, const media_player_start_settings *psettings, const char *url)
{
    BIP_Status bipStatus;
    NEXUS_AudioCodec    audioCodec = NEXUS_AudioCodec_eUnknown;
    NEXUS_VideoCodec    videoCodec = NEXUS_VideoCodec_eUnknown;

    BSTD_UNUSED( psettings );
    BDBG_ASSERT(player->hPlayer);
    /* Connect to Server */
    {
        BIP_PlayerConnectSettings settings;
        BIP_Player_GetDefaultConnectSettings(&settings);
        settings.pUserAgent = "BIP Player";
        settings.phHttpResponse = &player->hResponseHandle;
        settings.pAdditionalHeaders = psettings->additional_headers;
        bipStatus = BIP_Player_Connect( player->hPlayer, url, &settings );
        BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_Player_Connect Failed to Connect to URL=%s", url ), errorConnect, bipStatus, bipStatus );
    }
    BDBG_MSG(( BIP_MSG_PRE_FMT "Player connected for url:|%s|" BIP_MSG_PRE_ARG, url));

    if (player->hResponseHandle)
    {
        const char *pPlaySpeedString = NULL;
        BIP_HttpHeaderHandle hHeader = NULL;
        /* Get ContentFeatures.dlna.org header value */
        bipStatus = BIP_HttpResponse_GetNextHeader(
                        player->hResponseHandle,
                        NULL,
                        "ContentFeatures.dlna.org",
                        &hHeader,
                        &pPlaySpeedString
                        );
        BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS || bipStatus == BIP_INF_NOT_AVAILABLE), ( "BIP_HttpResponse_GetNextHeader Failed" ), error, bipStatus, bipStatus );

        /* Extract DLNA.ORG_PS params from it.*/
        if(bipStatus == BIP_SUCCESS && pPlaySpeedString)
        {
            BDBG_MSG(( BIP_MSG_PRE_FMT "ContentFeatures.dlna.org header value is |%s|" BIP_MSG_PRE_ARG, pPlaySpeedString));
            parsePlaySpeedString(player, pPlaySpeedString);
        }
    }

    /* Probe media to find Video and Audio decoder related information.*/
    {
        BIP_PlayerProbeMediaInfoSettings   probeMediaInfoSettings;
        BIP_Player_GetDefaultProbeMediaInfoSettings( &probeMediaInfoSettings );
        bipStatus = BIP_Player_ProbeMediaInfo(player->hPlayer, &probeMediaInfoSettings, &player->hMediaInfo );
        BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_Player_Probe Failed!"), error, bipStatus, bipStatus );
        BDBG_MSG(( BIP_MSG_PRE_FMT "BIP_Player_ProbeMediaInfo is done." BIP_MSG_PRE_ARG));
    }

    /* Now once the probing is done , prepare the BIP Player */
    {
        BIP_PlayerPrepareSettings               prepareSettings;
        BIP_PlayerSettings                      playerSettings;
        BIP_PlayerPrepareStatus                 prepareStatus;
        BIP_MediaInfoTrack                      mediaInfoTrack;
        NEXUS_SimpleVideoDecoderStartSettings   videoProgram;
        NEXUS_SimpleAudioDecoderStartSettings   audioProgram;

        /* Once probing and player prepare is done set video and audio programs.*/
        NEXUS_SimpleVideoDecoder_GetDefaultStartSettings(&videoProgram);
        /* set AudioProgram details.*/
        NEXUS_SimpleAudioDecoder_GetDefaultStartSettings(&audioProgram);

        BIP_Player_GetDefaultPrepareSettings(&prepareSettings);
        BIP_Player_GetDefaultSettings(&playerSettings);

        /* Get first Video & Audio track entry from pMediaInfoStream .*/
        BDBG_ASSERT( player->hMediaInfo );
        {
            if(psettings->video.pid != 0) /* This flgs is coming from app to indicate video is disabled.*/
            {
                if (playerGetTrackOfType(player->hMediaInfo, BIP_MediaInfoTrackType_eVideo, &mediaInfoTrack) )
                {
                   playerSettings.videoTrackId = mediaInfoTrack.trackId;
                   playerSettings.videoTrackSettings.pidTypeSettings.video.codec = mediaInfoTrack.info.video.codec;
                   videoCodec = mediaInfoTrack.info.video.codec;
                   if(player->parent->create_settings.maxFormat)
                   {
                       NEXUS_VideoFormatInfo info;
                       NEXUS_VideoFormat_GetInfo(player->parent->create_settings.maxFormat, &info);
                       videoProgram.maxWidth = info.width;
                       videoProgram.maxHeight = info.height;
                   }
                   else if(player->parent->create_settings.maxWidth && player->parent->create_settings.maxHeight)
                   {
                       videoProgram.maxWidth = player->parent->create_settings.maxWidth;
                       videoProgram.maxHeight = player->parent->create_settings.maxHeight;
                   }
                   else if(mediaInfoTrack.info.video.height && mediaInfoTrack.info.video.width)  /* check whether probe has found a width and height.*/
                   {
                       videoProgram.maxWidth = mediaInfoTrack.info.video.width;
                       videoProgram.maxHeight = mediaInfoTrack.info.video.height;
                   }

                   player->colorDepth = mediaInfoTrack.info.video.colorDepth;

                   BDBG_MSG(( BIP_MSG_PRE_FMT "Found a valid Video Track with trackId=%d , codec=%s" BIP_MSG_PRE_ARG, playerSettings.videoTrackId,BIP_ToStr_NEXUS_VideoCodec(videoCodec)));
                }
            }

            if(psettings->mediaPlayerSettings.audio.language)
            {
                BDBG_MSG(( BIP_MSG_PRE_FMT "preferred audio language is |%s|" BIP_MSG_PRE_ARG, psettings->mediaPlayerSettings.audio.language));
                playerSettings.pPreferredAudioLanguage = psettings->mediaPlayerSettings.audio.language;
            }

            if(psettings->mediaPlayerSettings.audio.ac3_service_type != UINT_MAX)
            {
                playerSettings.ac3Descriptor.bsmodValid = true;
                playerSettings.ac3Descriptor.bsmod = psettings->mediaPlayerSettings.audio.ac3_service_type;
            }

            if(psettings->mediaPlayerSettings.enableDynamicTrackSelection == true)
            {
                playerSettings.enableDynamicTrackSelection = true;
            }

            if(psettings->audio.pid != 0) /* This flgs is coming from app to indicate audio is disabled.*/
            {
                /* If language and/or ac3_service_type is specified than let bip_player internally find out the track. */
                if((psettings->mediaPlayerSettings.audio.ac3_service_type == UINT_MAX) && (psettings->mediaPlayerSettings.audio.language == NULL) )
                {
                    if (playerGetTrackOfType(player->hMediaInfo, BIP_MediaInfoTrackType_eAudio, &mediaInfoTrack) )
                    {
                        playerSettings.audioTrackId = mediaInfoTrack.trackId;
                        playerSettings.audioTrackSettings.pidSettings.pidTypeSettings.audio.codec = mediaInfoTrack.info.audio.codec;
                        audioCodec = mediaInfoTrack.info.audio.codec;
                        BDBG_MSG(( BIP_MSG_PRE_FMT "Found a valid Audio Track with trackId=%d , codec=%s" BIP_MSG_PRE_ARG, playerSettings.audioTrackId,BIP_ToStr_NEXUS_AudioCodec(audioCodec)));
                    }
                }
            }

            /* Make sure Probe found atleast a Video or an Audio Track. */
            BIP_CHECK_GOTO( (playerSettings.videoTrackId != UINT_MAX || playerSettings.audioTrackId != UINT_MAX), ( "Neither Audio nor Video Tracks found for URL=%s", url ), error, bipStatus, bipStatus );
        }

        {
            playerSettings.playbackSettings.endOfStreamCallback.callback = playbackDoneCallbackFromBIP;
            playerSettings.playbackSettings.endOfStreamCallback.context = player;
            playerSettings.playbackSettings.errorCallback.callback = playbackDoneCallbackFromBIP;
            playerSettings.playbackSettings.errorCallback.context = player;
            playerSettings.playbackSettings.endOfStreamAction = psettings->loopMode;
            playerSettings.playbackSettings.simpleStcChannel = player->parent->stcChannel;
            /* TODO: missing sync mode passed to BIP from media player, since bip uses NEXUS_SimpleStcChannel_GetDefaultSettings */

            if (audioCodec != NEXUS_AudioCodec_eUnknown || psettings->mediaPlayerSettings.audio.language || psettings->mediaPlayerSettings.audio.ac3_service_type != UINT_MAX)
            {
                playerSettings.audioTrackSettings.pidTypeSettings.audio.simpleDecoder = player->parent->audioDecoder;
            }
            if (videoCodec != NEXUS_VideoCodec_eUnknown)
            {
                playerSettings.videoTrackSettings.pidTypeSettings.video.simpleDecoder = player->parent->videoDecoder;
            }

            bipStatus = BIP_Player_Prepare(player->hPlayer, &prepareSettings, &playerSettings, NULL, NULL, &prepareStatus);
            BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_Player_Prepare Failed: URL=%s", url ), error, bipStatus, bipStatus );
            BDBG_MSG(( BIP_MSG_PRE_FMT "BIP_Player_Prepare: prepared" BIP_MSG_PRE_ARG));


            /* psettings->video.pid != 0 :This flgs is coming from app to indicate video is disabled.*/
            if ((videoCodec != NEXUS_VideoCodec_eUnknown) && (psettings->video.pid != 0))
            {
                videoProgram.settings.codec = videoCodec;
                videoProgram.settings.pidChannel = prepareStatus.hVideoPidChannel;
                player->parent->videoProgram = videoProgram;
            }

            /* psettings->audio.pid != 0 :This flgs is coming from app to indicate audio is disabled.*/
            /* audioCodec we get from prepare status since we may not have done explicit audioTrack selection
               in app and player has selected track internall during BIP_Player_Prepare..*/
            if ((prepareStatus.audioCodec != NEXUS_AudioCodec_eUnknown) && (psettings->audio.pid != 0))
            {
                audioProgram.primary.codec = prepareStatus.audioCodec;
                audioProgram.primary.pidChannel = prepareStatus.hAudioPidChannel;
                player->parent->audioProgram = audioProgram;
            }
        }
    }

    return 0;
error:
    /* Disconnect */
    BIP_Player_Disconnect(player->hPlayer);
errorConnect:
    return bipStatus;
}

int media_player_bip_start_play(media_player_bip_t player)
{
    BIP_PlayerStartSettings startSettings;
    BIP_Status bipStatus;

    BIP_Player_GetDefaultStartSettings(&startSettings);
    startSettings.timePositionInMs = player->initialPlaybackPositionInMs;
    startSettings.simpleVideoStartSettings = player->parent->videoProgram;
    startSettings.simpleAudioStartSettings = player->parent->audioProgram;
    bipStatus = BIP_Player_Start(player->hPlayer, &startSettings);
    BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_Player_Start Failed" ), error, bipStatus, bipStatus );
    BDBG_MSG(( BIP_MSG_PRE_FMT "BIP_Player_Start: started" BIP_MSG_PRE_ARG));

    return 0;
error:
    return bipStatus;
}

int media_player_bip_set_settings(media_player_bip_t player, const media_player_settings *psettings)
{
    BIP_Status bipStatus = BIP_SUCCESS;
    BIP_PlayerSettings          playerSettings;

    BIP_Player_GetSettings( player->hPlayer, &playerSettings );

    if(psettings->audio.language)
    {
        playerSettings.audioTrackId = UINT_MAX;
        playerSettings.pPreferredAudioLanguage = psettings->audio.language;
        BDBG_MSG(( BIP_MSG_PRE_FMT "changing preferred audio language to |%s|" BIP_MSG_PRE_ARG, psettings->audio.language));
    }
    if(psettings->audio.ac3_service_type != UINT_MAX)
    {
        playerSettings.ac3Descriptor.bsmod = psettings->audio.ac3_service_type;
        playerSettings.ac3Descriptor.bsmodValid = true;
        playerSettings.audioTrackId = UINT_MAX;
        BDBG_MSG(( BIP_MSG_PRE_FMT "changing ac3 service type to %d" BIP_MSG_PRE_ARG, psettings->audio.ac3_service_type));
    }
    if(psettings->enableDynamicTrackSelection == true)
    {
        playerSettings.enableDynamicTrackSelection = true;
    }
    /* Even if we may not found any valid track we still need to call SetSettings to set the language and/or ac3_service_type(In that case trackId will be UINT_MAX). */
    bipStatus = BIP_Player_SetSettings( player->hPlayer, &playerSettings );
    BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_Player_SetSettings Failed:" ), error, bipStatus, bipStatus );

error:
    return bipStatus;
}


int media_player_bip_status(media_player_bip_t player, NEXUS_PlaybackStatus *pstatus )
{
    BIP_Status bipStatus;
    BIP_PlayerStatus playerStatus;

    bipStatus = BIP_Player_GetStatus(player->hPlayer, &playerStatus);
    BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_Player_GetStatus Failed" ), error, bipStatus, bipStatus );

    pstatus->first = 0; /* Later need to be updated with aprropritae member once new playerStatus changes are integrated.*/
    pstatus->last = playerStatus.lastPositionInMs;
    pstatus->position = playerStatus.currentPositionInMs;

    BDBG_MSG(( BIP_MSG_PRE_FMT "playerStatus.lastPositionInMs=%d and playerStatus.currentPositionInMs=%d" BIP_MSG_PRE_ARG, (int)playerStatus.lastPositionInMs, (int)playerStatus.currentPositionInMs));

    return 0;
error:
    return bipStatus;
}

int media_player_bip_seek( media_player_bip_t player, int offset, int whence )
{
    BIP_PlayerStatus playerStatus;
    BIP_Status bipStatus = BIP_SUCCESS;

    switch (whence) {
    case SEEK_SET:
    default:
        break;
    case SEEK_CUR:

        bipStatus = BIP_Player_GetStatus(player->hPlayer, &playerStatus);
        BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_Player_GetStatus Failed" ), error, bipStatus, bipStatus );

        offset += playerStatus.currentPositionInMs;
        break;
    case SEEK_END:
        bipStatus = BIP_Player_GetStatus(player->hPlayer, &playerStatus);
        BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_Player_GetStatus Failed" ), error, bipStatus, bipStatus );

        offset += playerStatus.lastPositionInMs;
        break;
    }

    bipStatus = BIP_Player_Seek(player->hPlayer, offset);
    BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_Player_Seek Failed" ), error, bipStatus, bipStatus );

error:
    return bipStatus;
}

static int getPlaySpeed(media_player_bip_t player, int rate, char *speedString, int speedStringSize)
{
    /* Find the playSpeed entry corresponding to the specified rate & return the speedString */
    /*
     * e.g. given a playSpeedString like -16, -8, -4, -1, -1/8, -1/4, 1/4, 1/8, 4, 8, 16:
     * speedIndex == 2000, speedString = "1/4"
     * speedIndex == 3000, speedString = "1/8"
     * speedIndex == 4000, speedString = "4"
     * speedIndex == 5000, speedString = "8"
     * speedIndex == -1000, speedString = "-1/4"
     * speedIndex == -2000, speedString = "-1/8"
     * speedIndex == -3000, speedString = "-1"
     * speedIndex == -4000, speedString = "-4"
     * etc.
     */
    int speedIndex;
    unsigned index;

    /* player normalizes rate in NEXUS_NORMAL_DECODE_RATE, but we want to mapt it to an index to particular speed */
    if (abs(rate) >= NEXUS_NORMAL_DECODE_RATE)
        speedIndex = rate / NEXUS_NORMAL_DECODE_RATE;
    else {
        BDBG_ERR(("%s: rate (%d) is not normalized in NEXUS_NORMAL_DECODE_RATE units", BSTD_FUNCTION, rate));
        return -1;
    }

    if (speedIndex > 1) {
        /* +ve speeds */
        index = speedIndex - 2; /* minimum value of +ve speedIndex is 2 */
        snprintf(speedString, speedStringSize-1, "%d/%d", player->playSpeedFwdList[index].speedNumerator, player->playSpeedFwdList[index].speedDenominator);
    }
    else
    {
        /* -ve speeds */
        index = abs(speedIndex) - 1; /* minimum value of -ve speedIndex is 1 */
        if (index > player->maxRwdSpeedIndex) {
            BDBG_ERR(("%s: no -ve speed at this index: %u, max %u", BSTD_FUNCTION, index, player->maxRwdSpeedIndex));
            return -1;
        }
        index = player->maxRwdSpeedIndex - index; /* -ve speed index starts from the end of the list */
        snprintf(speedString, speedStringSize-1, "%d/%d", player->playSpeedRwdList[index].speedNumerator, player->playSpeedRwdList[index].speedDenominator);
    }

    BDBG_MSG(("%s: rate %d, speedIndex %d, speed string %s index %d", BSTD_FUNCTION, rate, speedIndex, speedString, index));
    return 0;
}

int media_player_bip_trick(media_player_bip_t player, int rate)
{
    BIP_Status bipStatus = BIP_SUCCESS;

    BDBG_MSG(( BIP_MSG_PRE_FMT "PlaySpeed rate %d" BIP_MSG_PRE_ARG, rate));

    if (rate == NEXUS_NORMAL_DECODE_RATE) {
         bipStatus = BIP_Player_Play(player->hPlayer);
         BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_Player_Play Failed" ), error, bipStatus, bipStatus );
    }
    else if (rate == 0) {
        bipStatus = BIP_Player_Pause(player->hPlayer, NULL);
        BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_Player_Pause Failed" ), error, bipStatus, bipStatus );
        BDBG_WRN(("Paused Playback!"));
    }
    else {
        if(player->playSpeedEntryPresent) {
#define PLAYSPEED_STRING_SIZE 16
            char speedString[PLAYSPEED_STRING_SIZE];

            memset(speedString, 0, PLAYSPEED_STRING_SIZE);
            if (getPlaySpeed(player, rate, speedString, PLAYSPEED_STRING_SIZE-1) < 0)
            {
                BDBG_ERR(( BIP_MSG_PRE_FMT "getPlaySpeed Failed" BIP_MSG_PRE_ARG));
                return -1;
            }
            BDBG_MSG(("Playspeed ---------------------------------------------->:|%s|", speedString));
            bipStatus = BIP_Player_PlayAtRateAsString(player->hPlayer, speedString, NULL);
            BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_Player_PlayAtRateAsString Failed" ), error, bipStatus, bipStatus );
        }
        else
        {
            BDBG_MSG(("PlayAtRate ---------------------------------------------->:|%d|", rate));
            bipStatus = BIP_Player_PlayAtRate(player->hPlayer, rate, NULL);
            BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_Player_PlayAtRate Failed" ), error, bipStatus, bipStatus );
        }
    }
    BDBG_MSG(( BIP_MSG_PRE_FMT "PlaySpeed at rate %d is done" BIP_MSG_PRE_ARG, rate));
error:
    return bipStatus;
}

int media_player_bip_frame_advance(media_player_bip_t player, bool forward)
{
    BIP_Status bipStatus = BIP_SUCCESS;

    BDBG_MSG(( BIP_MSG_PRE_FMT "FrameAdvance: direction %s" BIP_MSG_PRE_ARG, forward?"Forward":"Reverse"));

    bipStatus = BIP_Player_PlayByFrame(player->hPlayer, forward);
    BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_Player_PlayByFrame Failed" ), error, bipStatus, bipStatus );
    BDBG_MSG(( BIP_MSG_PRE_FMT "FrameAdvance: direction %s is done!" BIP_MSG_PRE_ARG, forward?"Forward":"Reverse"));
error:
    return bipStatus;
}

#if 0
#if B_REFSW_TR69C_SUPPORT
int media_player_bip_get_set_tr69c_info(void *context, enum b_tr69c_type type, union b_tr69c_info *info)
{
    media_player_bip_t player = context;
    int rc;
    switch (type)
    {
        case b_tr69c_type_get_playback_ip_status:
            rc = B_PlaybackIp_GetStatus(player->playbackIp, &info->playback_ip_status);
            bipStatus = BIP_Player_GetStatus(player->hPlayer, &playerStatus);
            BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_Player_GetStatus Failed" ), error, bipStatus, bipStatus );
            if (rc) return BERR_TRACE(rc);
            break;
        default:
            return BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }
    return 0;
}
#endif
#endif

void media_player_bip_stop(media_player_bip_t player)
{
    if (player->hPlayer)
    {
        BIP_Player_Stop(player->hPlayer);
        BIP_Player_Disconnect(player->hPlayer);
        BDBG_MSG(( BIP_MSG_PRE_FMT "BIP_Player_Disconnect:disconnected" BIP_MSG_PRE_ARG));
    }
}

void media_player_bip_destroy( media_player_bip_t player)
{
    if(player->hPlayer)
    {
        BIP_Player_Destroy(player->hPlayer);
    }
    BKNI_Free(player);

    BIP_SslClientFactory_Uninit();
    BIP_DtcpIpClientFactory_Uninit();
    BIP_Uninit();
}

#else
/* stub API if PLAYBACK_IP_SUPPORT is not defined */
media_player_bip_t media_player_bip_create(media_player_t parent)
{
    BSTD_UNUSED(parent);
    return NULL;
}

void media_player_bip_destroy(media_player_bip_t player)
{
    BSTD_UNUSED(player);
}

int media_player_bip_start(media_player_bip_t player, const media_player_start_settings *psettings, const char *url)
{
    BSTD_UNUSED(player);
    BSTD_UNUSED(psettings);
    BSTD_UNUSED(url);
    return NEXUS_NOT_SUPPORTED;
}

void media_player_bip_get_color_depth(media_player_bip_t player, unsigned *pColorDepth)
{
    BSTD_UNUSED(player);
    BSTD_UNUSED(pColorDepth);
}

int media_player_bip_start_play(media_player_bip_t player)
{
    BSTD_UNUSED(player);
    return NEXUS_NOT_SUPPORTED;
}

int media_player_bip_set_settings(media_player_bip_t player, const media_player_settings *psettings)
{
    BSTD_UNUSED(player);
    BSTD_UNUSED(psettings);
    return NEXUS_NOT_SUPPORTED;
}

void media_player_bip_stop(media_player_bip_t player)
{
    BSTD_UNUSED(player);
}

int media_player_bip_trick(media_player_bip_t player, int rate)
{
    BSTD_UNUSED(player);
    BSTD_UNUSED(rate);
    return NEXUS_NOT_SUPPORTED;
}

int media_player_bip_status(media_player_bip_t player, NEXUS_PlaybackStatus *pstatus)
{
    BSTD_UNUSED(player);
    BSTD_UNUSED(pstatus);
    return NEXUS_NOT_SUPPORTED;
}

int media_player_bip_seek( media_player_bip_t player, int offset, int whence )
{
    BSTD_UNUSED(player);
    BSTD_UNUSED(offset);
    BSTD_UNUSED(whence);
    return NEXUS_NOT_SUPPORTED;
}

int media_player_bip_frame_advance(media_player_bip_t player, bool forward)
{
    BSTD_UNUSED(player);
    BSTD_UNUSED(forward);
    return NEXUS_NOT_SUPPORTED;
}

#endif
#endif
