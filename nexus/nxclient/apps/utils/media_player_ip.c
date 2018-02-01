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

/* media probe */
#include "bmedia_probe.h"
#include "bmpeg2ts_probe.h"
#include "bfile_stdio.h"
#if B_HAS_ASF
#include "basf_probe.h"
#endif
#if B_HAS_AVI
#include "bavi_probe.h"
#endif
#include "bhevc_video_probe.h"
#include "b_playback_ip_lib.h"
#include "nexus_timebase.h"
#ifdef B_HAS_DTCP_IP
#include "b_dtcp_applib.h"
#endif

#define IP_NETWORK_MAX_JITTER 300 /* in msec */
#define MPEG_DASH_MAX_SEGMENT_SIZE  (2*1024*1024)

BDBG_MODULE(media_player);

BDBG_OBJECT_ID(media_player_ip);

typedef struct
{
    int speedNumerator;
    int speedDenominator;
}playSpeedEntry;
struct media_player_ip
{
    media_player_t parent;
    B_PlaybackIpHandle playbackIp;
    bool playbackIpLiveMode;
    NEXUS_Timebase pacingTimebase;
    B_PlaybackIpPsiInfo playbackIpPsi;
    bmedia_probe_t probe;
    const bmedia_probe_stream *stream;
    NEXUS_FilePlayHandle file;
    bool usePlaypump;
    NEXUS_PlaypumpHandle playpump1; /* this handle is used when both audio & video have different PIDS */
    NEXUS_PlaypumpHandle playpump2; /* this handle is used when audio & video have same PIDs and thus need to be fed two different playpumps */
#define MAX_PLAYSPEED_ENTRIES 16
    playSpeedEntry playSpeedFwdList[MAX_PLAYSPEED_ENTRIES];
    playSpeedEntry playSpeedRwdList[MAX_PLAYSPEED_ENTRIES];
    unsigned maxFwdSpeedIndex;
    unsigned maxRwdSpeedIndex;
#if B_HAS_DTCP_IP
    void * akeHandle;
    void * dtcpCtx;
    bool dtcpEnabled;
    int dtcpAkePort;
#endif
};

media_player_ip_t media_player_ip_create(media_player_t parent)
{
    media_player_ip_t player = NULL;

    player = BKNI_Malloc(sizeof(*player));
    if (!player) return NULL;

    memset(player, 0, sizeof(*player));
    player->parent = parent;

    /* Note:
     * For IP Media Player, we are now deferring to open the Nexus Playpump handle. It is done primarily for this reason:
     *  certain IP protocols such as MPEG DASH require a larger Playpump FIFO size. Instead of opening the playpump with the
     *  worst case memory usage, we now defer opening to the Player Start time. At that point, we know whether we would need
     *  to open a larger playpump buffer or not as we would have completed the PSI parsing on the media.
     *
     *  There is some concern of Nexus heap fragmentation but that is outweighed with other approach of just hogging up
     *  large amount of memory for all other Playback cases.
     */
    player->playpump1 = NULL;
    player->playpump2 = NULL;

#ifdef B_HAS_DTCP_IP
    if (parent->create_settings.dtcpEnabled) {
        if(DtcpInitHWSecurityParams(NULL) != BERR_SUCCESS)
        {
            BDBG_ERR(("Failed to init DtcpHW Security params"));
            goto error;
        }

        /* initialize DtcpIp library */
        /* if App hasn't already opened the Nexus M2M DMA handle, then pass-in initial arg as NULL and let DTCP/IP library open the handle */
        if ((player->dtcpCtx = DtcpAppLib_Startup(B_DeviceMode_eSink, false, B_DTCP_KeyFormat_eCommonDRM, false)) == NULL) {
            BDBG_ERR(("DtcpAppLib_Startup failed"));
            goto error;
        }
        BDBG_ASSERT(player->dtcpCtx);
        player->dtcpEnabled = true;
        player->dtcpAkePort = 8000;
        player->akeHandle = NULL;
    }
#endif
    player->playbackIp = B_PlaybackIp_Open(NULL);
    BDBG_ASSERT(player->playbackIp);

    return player;
#ifdef B_HAS_DTCP_IP
error:
#endif
    if (player) BKNI_Free(player);
    return NULL;
}

static void b_print_media_string(const bmedia_probe_stream *stream)
{
    char stream_info[512];
    bmedia_stream_to_string(stream, stream_info, sizeof(stream_info));
    printf( "Media Probe:\n" "%s\n\n", stream_info);
}

void playbackIpEventCallback(void *appCtx, B_PlaybackIpEventIds eventId)
{
    media_player_ip_t player = appCtx;
    BDBG_MSG (("%s: Got EventId %d from IP library, appCtx %p, eof CB %p, ctx %p", BSTD_FUNCTION, eventId, appCtx, (void *)(unsigned long)player->parent->start_settings.eof, player->parent->start_settings.context));
    if (eventId == B_PlaybackIpEvent_eServerEndofStreamReached) {
        if (player->parent->start_settings.eof) {
            (player->parent->start_settings.eof)(player->parent->start_settings.context);
        }
    }
}

extern int B_PlaybackIp_UtilsParsePlaySpeedString(const char *playSpeedStringOrig, int *speedNumerator, int *speedDenominator, int *direction);
static int getPlaySpeed(media_player_ip_t player, int rate, char *speedString, int speedStringSize)
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
        if (index > player->maxFwdSpeedIndex ) {
            BDBG_ERR(("%s: no +ve speed at this index: %u, max %u", BSTD_FUNCTION, index, player->maxFwdSpeedIndex));
            return -1;
        }
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

static int parsePlaySpeedString(media_player_ip_t player)
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
    char *playSpeedString;
    char *nextSpeed;

    playSpeedStringLength = strlen(player->playbackIpPsi.playSpeedString) + 1;
    if ( (playSpeedString = BKNI_Malloc(playSpeedStringLength)) == NULL)
    {
        BDBG_ERR(("%s: BKNI_Malloc failed to allocate %d bytes string for playSpeed", BSTD_FUNCTION, playSpeedStringLength));
        return -1;
    }
    strncpy(playSpeedString, player->playbackIpPsi.playSpeedString, playSpeedStringLength-1);
    playSpeedString[playSpeedStringLength-1] = '\0';

    for (i=0, j=0, nextSpeed = strtok_r(playSpeedString, ",", &savePtr);
            nextSpeed != NULL;
            nextSpeed = strtok_r(NULL, ",", &savePtr)
        )
    {
        int speedNumerator, speedDenominator;
        if (B_PlaybackIp_UtilsParsePlaySpeedString(nextSpeed, &speedNumerator, &speedDenominator, NULL) < 0) {
            BDBG_ERR(("%s: Failed to parse the playSpeedString %s", BSTD_FUNCTION, nextSpeed));
            return -1;
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
    player->maxFwdSpeedIndex = i-1;
    player->maxRwdSpeedIndex = j-1;
    for (k=0; k<i;k++)
        BDBG_MSG(("%s: +ve speed idx %d, num %d, denom %d", BSTD_FUNCTION,
                k, player->playSpeedFwdList[k].speedNumerator, player->playSpeedFwdList[k].speedDenominator));
    for (k=0; k<j;k++)
        BDBG_MSG(("%s: -ve speed idx %d, num %d, denom %d", BSTD_FUNCTION,
                k, player->playSpeedRwdList[k].speedNumerator, player->playSpeedRwdList[k].speedDenominator));
    BKNI_Free(playSpeedString);
    return 0;
}

int media_player_ip_start(media_player_ip_t player, const media_player_start_settings *psettings, const struct url *url)
{
    NEXUS_SimpleVideoDecoderStartSettings videoProgram;
    NEXUS_PlaybackPidChannelSettings playbackPidSettings;
    NEXUS_TransportType transportType = NEXUS_TransportType_eTs;
    NEXUS_TransportTimestampType timestampType = NEXUS_TransportTimestampType_eNone;
    NEXUS_PlaybackSettings playbackSettings;
    int rc=0;
    const bmedia_probe_track *track;

    NEXUS_SimpleVideoDecoder_GetDefaultStartSettings(&videoProgram);

    if (!strcasecmp(url->scheme, "http") || !strcasecmp(url->scheme, "https")) {
        /* URL contains http src info, setup IP playback */
        B_PlaybackIpSessionOpenSettings ipSessionOpenSettings;
        B_PlaybackIpSessionOpenStatus ipSessionOpenStatus;
        B_PlaybackIpSessionSetupSettings ipSessionSetupSettings;
        B_PlaybackIpSessionSetupStatus ipSessionSetupStatus;
        NEXUS_Error rc;

#if B_HAS_DTCP_IP
        if (player->dtcpEnabled)
        {
            /* Perform AKE for DTCP/IP */
            if((rc = DtcpAppLib_DoAkeOrVerifyExchKey(player->dtcpCtx, url->domain, player->dtcpAkePort, &(player->akeHandle))) != BERR_SUCCESS) {
                BDBG_ERR(("ip_client_dtcp: DTCP AKE Failed"));
                goto error;
            }
        }
#endif
        /* Setup socket setting structure used in the IP Session Open */
        B_PlaybackIp_GetDefaultSessionOpenSettings(&ipSessionOpenSettings);
        ipSessionOpenSettings.socketOpenSettings.protocol = B_PlaybackIpProtocol_eHttp;
        strncpy(ipSessionOpenSettings.socketOpenSettings.ipAddr, url->domain, sizeof(ipSessionOpenSettings.socketOpenSettings.ipAddr)-1);
        ipSessionOpenSettings.socketOpenSettings.port = url->port;
        ipSessionOpenSettings.socketOpenSettings.url = (char*)url->path;
        BDBG_WRN(("parsed url is http://%s:%d%s", ipSessionOpenSettings.socketOpenSettings.ipAddr, ipSessionOpenSettings.socketOpenSettings.port, ipSessionOpenSettings.socketOpenSettings.url));
        ipSessionOpenSettings.ipMode = B_PlaybackIpClockRecoveryMode_ePull;
        ipSessionOpenSettings.eventCallback = playbackIpEventCallback;
        ipSessionOpenSettings.appCtx = player; /* this should be app handle */
        ipSessionOpenSettings.networkTimeout = 15;  /* 15 sec timeout during network outage events */
        ipSessionOpenSettings.u.http.networkBufferSize = MPEG_DASH_MAX_SEGMENT_SIZE;

#if B_HAS_DTCP_IP
        if (player->dtcpEnabled)
        {
            ipSessionOpenSettings.security.securityProtocol = B_PlaybackIpSecurityProtocol_DtcpIp;
            ipSessionOpenSettings.security.initialSecurityContext = player->akeHandle;
        }
#endif
        rc = B_PlaybackIp_SessionOpen(player->playbackIp, &ipSessionOpenSettings, &ipSessionOpenStatus);
        if (rc) { rc = BERR_TRACE(rc); goto error; }
        BDBG_MSG (("Session Open call succeeded, HTTP status code %d", ipSessionOpenStatus.u.http.statusCode));

        /* now do session setup */
        B_PlaybackIp_GetDefaultSessionSetupSettings(&ipSessionSetupSettings);
        /* if app needs to play multiple formats (such as a DLNA DMP/DMR) (e.g. TS, VOB/PES, MP4, ASF, etc.), then set this option to do deep payload inspection */
        ipSessionSetupSettings.u.http.enablePayloadScanning = true;
        /* set a limit on how long the psi parsing should continue before returning */
        ipSessionSetupSettings.u.http.psiParsingTimeLimit = 30000; /* 30sec */
        rc = B_PlaybackIp_SessionSetup(player->playbackIp, &ipSessionSetupSettings, &ipSessionSetupStatus);
        if (rc) { rc = BERR_TRACE(rc); goto error; }
        BDBG_MSG (("Session Setup call succeeded, file handle %p", (void *)ipSessionSetupStatus.u.http.file));
        player->stream = (bmedia_probe_stream *)(ipSessionSetupStatus.u.http.stream);
        player->file = ipSessionSetupStatus.u.http.file;
        if (ipSessionSetupStatus.u.http.psi.liveChannel)
            player->playbackIpLiveMode = true;
        else
            player->playbackIpLiveMode = false;
        player->playbackIpPsi = ipSessionSetupStatus.u.http.psi;
        transportType = player->playbackIpPsi.mpegType;
    }
    else if (!strcasecmp(url->scheme, "udp") || !strcasecmp(url->scheme, "rtp")) {
        /* URL contains a live IP channel info, setup IP playback */
        B_PlaybackIpSessionOpenSettings ipSessionOpenSettings;
        B_PlaybackIpSessionOpenStatus ipSessionOpenStatus;
        B_PlaybackIpSessionSetupSettings ipSessionSetupSettings;
        B_PlaybackIpSessionSetupStatus ipSessionSetupStatus;
        NEXUS_Error rc;

        /* Setup socket setting structure used in the IP Session Open */
        B_PlaybackIp_GetDefaultSessionOpenSettings(&ipSessionOpenSettings);
        ipSessionOpenSettings.maxNetworkJitter = IP_NETWORK_MAX_JITTER;
        ipSessionOpenSettings.networkTimeout = 1;  /* 1 sec timeout during network outage events */
        ipSessionOpenSettings.u.http.networkBufferSize = MPEG_DASH_MAX_SEGMENT_SIZE;
        if (!strcasecmp(url->scheme, "rtp")) {
            ipSessionOpenSettings.socketOpenSettings.protocol = B_PlaybackIpProtocol_eRtp;
        }
        else {
            ipSessionOpenSettings.socketOpenSettings.protocol = B_PlaybackIpProtocol_eUdp;
        }
        strncpy(ipSessionOpenSettings.socketOpenSettings.ipAddr, url->domain, sizeof(ipSessionOpenSettings.socketOpenSettings.ipAddr)-1);
        ipSessionOpenSettings.socketOpenSettings.port = url->port;
#if 0
        /* needed for RTSP */
        ipSessionOpenSettings.socketOpenSettings.url = url->path;
#endif
        BDBG_MSG(("parsed url is udp://%s:%d%s", ipSessionOpenSettings.socketOpenSettings.ipAddr, ipSessionOpenSettings.socketOpenSettings.port, ipSessionOpenSettings.socketOpenSettings.url));
        ipSessionOpenSettings.ipMode = B_PlaybackIpClockRecoveryMode_ePushWithPcrSyncSlip;

        rc = B_PlaybackIp_SessionOpen(player->playbackIp, &ipSessionOpenSettings, &ipSessionOpenStatus);
        if (rc) { rc = BERR_TRACE(rc); goto error; }
        BDBG_MSG(("Session Open call succeeded"));

        /* now do session setup */
        B_PlaybackIp_GetDefaultSessionSetupSettings(&ipSessionSetupSettings);
        /* set a limit on how long the psi parsing should continue before returning */
        ipSessionSetupSettings.u.udp.psiParsingTimeLimit = 3000; /* 3sec */
        rc = B_PlaybackIp_SessionSetup(player->playbackIp, &ipSessionSetupSettings, &ipSessionSetupStatus);
        if (rc) { rc = BERR_TRACE(rc); goto error; }
        BDBG_MSG(("Session Setup call succeeded"));
        player->stream = (bmedia_probe_stream *)(ipSessionSetupStatus.u.udp.stream);
        player->file = NULL;
        player->playbackIpLiveMode = true;
        player->playbackIpPsi = ipSessionSetupStatus.u.udp.psi;
        transportType = player->playbackIpPsi.mpegType;
        BDBG_MSG(("Video Pid %d, Video Codec %d, Audio Pid %d, Audio Codec %d, Pcr Pid %d, Transport Type %d",
            player->playbackIpPsi.videoPid, player->playbackIpPsi.videoCodec, player->playbackIpPsi.audioPid, player->playbackIpPsi.audioCodec, player->playbackIpPsi.pcrPid, player->playbackIpPsi.mpegType));
    }
    else {
        /* should not have reached here */
        return BERR_TRACE(-1);
    }

    if (player->stream) {
        if (!psettings->quiet) b_print_media_string(player->stream);
        transportType = b_mpegtype2nexus(player->stream->type);
        if (player->stream->type == bstream_mpeg_type_ts) {
            if ((((bmpeg2ts_probe_stream*)player->stream)->pkt_len) == 192) {
                timestampType = NEXUS_TransportTimestampType_eBinary;
            }
        }
    }
    if (player->playbackIpPsi.numPlaySpeedEntries)
    {
        parsePlaySpeedString(player);
    }
    /* if we've created the probe, we must be stopped */
    player->parent->colorDepth = 8;
    player->usePlaypump = (player->playbackIpLiveMode || player->playbackIpPsi.hlsSessionEnabled || player->playbackIpPsi.mpegDashSessionEnabled || player->playbackIpPsi.numPlaySpeedEntries);
    if (player->usePlaypump) {
        B_PlaybackIpSettings playbackIpSettings;
        NEXUS_PlaypumpSettings playpumpSettings;
        NEXUS_PlaypumpOpenSettings playpumpOpenSettings;
        NEXUS_SimpleStcChannelSettings stcSettings;
        NEXUS_PidChannelHandle pcrPidChannel = NULL;
        NEXUS_PlaypumpOpenPidChannelSettings pidChannelSettings;
        /* playback IP channel is in the live mode, it differs from regular playback in following ways: */
        /* -use nexus playback to feed IP playloads */
        /* -configure simpleStc in the live mode to use the PCRs */
        /* for streams w/ *no* transport timestamps, following additional changes are done */
        /* -program highJitter value in the simpleStc. This tells simpleStc to increase the PCR error thresholds */
        /* -delay decode by ip jitter value to account to absorb high network jitter */
        /* -increase CDB size to make sure that additional space for dejitter buffer is available */
        /* -TODO: check if default CDB sizes are big enough for additional buffering of IP_NETWORK_MAX_JITTER msec */

        /* Open playpump1 */
        NEXUS_Playpump_GetDefaultOpenSettings(&playpumpOpenSettings);
        if (player->playbackIpPsi.mpegDashSessionEnabled)
            playpumpOpenSettings.fifoSize = MPEG_DASH_MAX_SEGMENT_SIZE;
        else
            playpumpOpenSettings.fifoSize *= 2;
        playpumpOpenSettings.numDescriptors = 200;
        player->playpump1 = NEXUS_Playpump_Open(NEXUS_ANY_ID, &playpumpOpenSettings);
        if (!player->playpump1) {
            BDBG_WRN(("%s: Couldn't open 2nd playpump for IP Player", BSTD_FUNCTION));
            goto error;
        }
        BDBG_MSG(("%s: Opened Playpump with %d fifoSize", BSTD_FUNCTION, (unsigned)playpumpOpenSettings.fifoSize));

        /* configure nexus playpump */
        NEXUS_Playpump_GetSettings(player->playpump1, &playpumpSettings);
        playpumpSettings.transportType = player->playbackIpPsi.mpegType;
        if (player->playbackIpPsi.usePlaypump2ForAudio && player->playbackIpPsi.audioCodec != NEXUS_AudioCodec_eUnknown) {
            BDBG_WRN(("%s: using 2nd playpump for audio", BSTD_FUNCTION));
            player->playpump2 = player->parent->playpump;
            NEXUS_Playpump_GetSettings(player->playpump2, &playpumpSettings);
            playpumpSettings.transportType = player->playbackIpPsi.mpegType;
            rc = NEXUS_Playpump_SetSettings(player->playpump2, &playpumpSettings);
            if (rc) { rc = BERR_TRACE(rc); goto error; }
        }
        else {
            player->playpump2 = NULL;
        }
        B_PlaybackIp_GetSettings(player->playbackIp, &playbackIpSettings);
        /* TTS streams */
        if (player->playbackIpPsi.transportTimeStampEnabled) {
            NEXUS_TimebaseSettings timebaseSettings;
            playpumpSettings.timestamp.type = NEXUS_TransportTimestampType_e32_Binary;
            /* Must use the timebase after Video decoders & Video Encodres */
            #ifdef NEXUS_NUM_VIDEO_ENCODERS
            player->pacingTimebase = NEXUS_Timebase_Open(NEXUS_Timebase_e0+NEXUS_NUM_VIDEO_DECODERS + NEXUS_NUM_VIDEO_ENCODERS);
            #else
            player->pacingTimebase = NEXUS_Timebase_Open(NEXUS_Timebase_e0+NEXUS_NUM_VIDEO_DECODERS);
            #endif
            NEXUS_Timebase_GetSettings(player->pacingTimebase,&timebaseSettings);
            timebaseSettings.sourceType = NEXUS_TimebaseSourceType_eFreeRun;
            timebaseSettings.freeze = true;
            timebaseSettings.sourceSettings.pcr.trackRange = NEXUS_TimebaseTrackRange_e122ppm;
            NEXUS_Timebase_SetSettings(player->pacingTimebase,&timebaseSettings);

            playpumpSettings.timestamp.timebase = player->pacingTimebase;
            playpumpSettings.timestamp.pacingMaxError = playbackIpSettings.ttsParams.pacingMaxError;
            playpumpSettings.timestamp.pacing = true;
            playpumpSettings.timestamp.pacingOffsetAdjustDisable = true;
            playpumpSettings.timestamp.resetPacing = true;
            playpumpSettings.timestamp.parityCheckDisable = true;
        }

        rc = NEXUS_Playpump_SetSettings(player->playpump1, &playpumpSettings);
        if (rc) { rc = BERR_TRACE(rc); goto error; }

        playbackIpSettings.useNexusPlaypump = true;

        playbackIpSettings.enableEndOfStreamLooping = psettings->loopMode == NEXUS_PlaybackLoopMode_eLoop;

        /* TTS streams */
        if (player->playbackIpPsi.transportTimeStampEnabled) {
            BDBG_WRN(("@@@ Enabling TTS in media_player_start()"));
            playbackIpSettings.ipMode = B_PlaybackIpClockRecoveryMode_ePushWithTtsNoSyncSlip;
        }
        rc = B_PlaybackIp_SetSettings(player->playbackIp, &playbackIpSettings);
        if (rc) { rc = BERR_TRACE(rc); goto error; }

        /* Open the pid channels */
        if (player->playbackIpPsi.videoCodec != NEXUS_VideoCodec_eNone && player->playbackIpPsi.videoPid && player->parent->videoDecoder) {
            NEXUS_Playpump_GetDefaultOpenPidChannelSettings(&pidChannelSettings);
            pidChannelSettings.pidType = NEXUS_PidType_eVideo;
            videoProgram.settings.pidChannel = NEXUS_Playpump_OpenPidChannel(player->playpump1, player->playbackIpPsi.videoPid, &pidChannelSettings);
            if (!videoProgram.settings.pidChannel) { rc = BERR_TRACE(rc); goto error; }
            videoProgram.settings.codec = player->playbackIpPsi.videoCodec;
            if (!player->playbackIpPsi.transportTimeStampEnabled && player->playbackIpLiveMode) { /* if timestamps are present, playpump buffer is used as de-jitter buffer */
                /* increase the ptsOffset so that CDB can be used at the de-jitter buffer */
                NEXUS_SimpleVideoDecoder_GetSettings(player->parent->videoDecoder, &player->parent->videoDecoderSettings);
                player->parent->videoDecoderSettings.ptsOffset = IP_NETWORK_MAX_JITTER * 45;    /* In 45Khz clock */
                rc = NEXUS_SimpleVideoDecoder_SetSettings(player->parent->videoDecoder, &player->parent->videoDecoderSettings);
                if (rc) { rc = BERR_TRACE(rc); goto error; }
            }
            if (player->parent->create_settings.maxWidth && player->parent->create_settings.maxHeight) {
                videoProgram.maxWidth = player->parent->create_settings.maxWidth;
                videoProgram.maxHeight = player->parent->create_settings.maxHeight;
            }
            BDBG_MSG(("%s: video pid %d, pid channel created", BSTD_FUNCTION, player->playbackIpPsi.videoPid));
        }

        /* Open the extra video pid channel if present in the stream */
        if (player->playbackIpPsi.extraVideoCodec != NEXUS_VideoCodec_eNone && player->playbackIpPsi.extraVideoPid != 0 && player->parent->videoDecoder) {
            NEXUS_Playpump_GetDefaultOpenPidChannelSettings(&pidChannelSettings);
            pidChannelSettings.pidType = NEXUS_PidType_eVideo;
            videoProgram.settings.enhancementPidChannel = NEXUS_Playpump_OpenPidChannel(player->playpump1, player->playbackIpPsi.extraVideoPid, &pidChannelSettings);
            if (!videoProgram.settings.enhancementPidChannel) { rc = BERR_TRACE(rc); goto error; }
            BDBG_MSG(("%s: extra video pid %d, codec %d is present, pid channel created", BSTD_FUNCTION, player->playbackIpPsi.extraVideoPid, player->playbackIpPsi.extraVideoCodec));
        }

        if (player->playbackIpPsi.audioCodec != NEXUS_AudioCodec_eUnknown && player->playbackIpPsi.audioPid && player->parent->audioDecoder) {
            NEXUS_SimpleAudioDecoderSettings settings;
            /* Open the audio pid channel */
            NEXUS_Playpump_GetDefaultOpenPidChannelSettings(&pidChannelSettings);
            pidChannelSettings.pidType = NEXUS_PidType_eAudio;
            pidChannelSettings.pidTypeSettings.audio.codec = player->playbackIpPsi.audioCodec;
            player->parent->audioProgram.primary.pidChannel = NEXUS_Playpump_OpenPidChannel(player->playpump2?player->playpump2:player->playpump1, player->playbackIpPsi.audioPid, &pidChannelSettings);
            if (!player->parent->audioProgram.primary.pidChannel) { rc = BERR_TRACE(rc); goto error; }
            player->parent->audioProgram.primary.codec = player->playbackIpPsi.audioCodec;
            if (!player->playbackIpPsi.transportTimeStampEnabled && player->playbackIpLiveMode) { /* if timestamps are present, playpump buffer is used as de-jitter buffer */
                /* increase the ptsOffset so that CDB can be used at the de-jitter buffer */
                NEXUS_SimpleAudioDecoder_GetSettings(player->parent->audioDecoder, &settings);
                settings.primary.ptsOffset = IP_NETWORK_MAX_JITTER * 45;    /* In 45Khz clock */
                rc = NEXUS_SimpleAudioDecoder_SetSettings(player->parent->audioDecoder, &settings);
                if (rc) { rc = BERR_TRACE(rc); goto error; }
            }
        }

        if (player->playbackIpPsi.pcrPid && player->playbackIpPsi.pcrPid != player->playbackIpPsi.audioPid && player->playbackIpPsi.pcrPid != player->playbackIpPsi.videoPid) {
            /* Open the pcr pid channel */
            NEXUS_Playpump_GetDefaultOpenPidChannelSettings(&pidChannelSettings);
            pidChannelSettings.pidType = NEXUS_PidType_eUnknown;
            pcrPidChannel = NEXUS_Playpump_OpenPidChannel(player->playpump1, player->playbackIpPsi.pcrPid, &pidChannelSettings);
            if (!pcrPidChannel) { rc = BERR_TRACE(rc); goto error; }
            BDBG_MSG(("%s: Opened pcr pid channel %p for pcr pid %u", BSTD_FUNCTION, (void *)pcrPidChannel, player->playbackIpPsi.pcrPid));
        }
        else
        {
            if (player->playbackIpPsi.pcrPid == player->playbackIpPsi.audioPid)
                pcrPidChannel = player->parent->audioProgram.primary.pidChannel;
            else
                pcrPidChannel = videoProgram.settings.pidChannel;
        }

        /* now configure the simple stc channel */
        if (!player->parent->stcChannel) { rc = BERR_TRACE(rc); goto error; }
        NEXUS_SimpleStcChannel_GetSettings(player->parent->stcChannel, &stcSettings);
        if (player->playbackIpPsi.hlsSessionEnabled || player->playbackIpPsi.mpegDashSessionEnabled || player->playbackIpPsi.numPlaySpeedEntries) {
            /* For HLS & MPEG DASH protocols, we configure clock recovery in pull mode as content is being pulled. We can't use playpump because playback doesn't understand HLS/DASH protocols. */
            stcSettings.mode = NEXUS_StcChannelMode_eAuto;
            stcSettings.modeSettings.Auto.transportType = transportType;
            if (player->playbackIpPsi.mpegType == NEXUS_TransportType_eMp4Fragment) {
                stcSettings.modeSettings.Auto.transportType = NEXUS_TransportType_eMpeg2Pes;
            }
        }
        else if (player->playbackIpPsi.transportTimeStampEnabled) {
            /* when timestamps are present, dejittering is done before feeding to xpt and */
            /* thus is treated as a live playback */
            /* High Jitter mode for tts will need to be adjusted*/
            stcSettings.modeSettings.highJitter.mode = NEXUS_SimpleStcChannelHighJitterMode_eNone;
            stcSettings.mode = NEXUS_StcChannelMode_ePcr;
            stcSettings.modeSettings.pcr.pidChannel = pcrPidChannel;
            stcSettings.modeSettings.pcr.disableJitterAdjustment = true;
            stcSettings.modeSettings.pcr.disableTimestampCorrection = true;
            stcSettings.modeSettings.Auto.transportType   = transportType;
        }
        else {
            /* when timestamps are not present, we directly feed the jittered packets to the transport */
            /* and set the max network jitter in highJitterThreshold. */
            /* This enables the SimpleStc to internally program the various stc & timebase related thresholds to account for network jitter */
            stcSettings.modeSettings.highJitter.threshold = IP_NETWORK_MAX_JITTER;
            stcSettings.modeSettings.highJitter.mode = NEXUS_SimpleStcChannelHighJitterMode_eDirect;
            stcSettings.mode = NEXUS_StcChannelMode_ePcr;
            stcSettings.modeSettings.pcr.pidChannel = pcrPidChannel;
        }
        stcSettings.sync = psettings->sync;
        rc = NEXUS_SimpleStcChannel_SetSettings(player->parent->stcChannel, &stcSettings);
        if (rc) { rc = BERR_TRACE(rc); goto error; }
        BDBG_MSG(("%s: live IP mode configuration complete...", BSTD_FUNCTION));
    }
    else
    {
        unsigned cur_program_offset = 0, prev_program = 0;
        bool prev_program_set = false;

        /* playback mode setup for either file or HTTP playback */
        if (player->parent->stcChannel) {
            NEXUS_SimpleStcChannelSettings stcSettings;
            bool setStc = false;

            NEXUS_SimpleStcChannel_GetSettings(player->parent->stcChannel, &stcSettings);
            if (stcSettings.sync != psettings->sync)
            {
                stcSettings.sync = psettings->sync;
                setStc = true;
            }
            if (stcSettings.modeSettings.Auto.transportType != transportType) {
                stcSettings.modeSettings.Auto.transportType = transportType;
                setStc = true;
            }
            if (setStc)
            {
                rc = NEXUS_SimpleStcChannel_SetSettings(player->parent->stcChannel, &stcSettings);
                if (rc) { rc = BERR_TRACE(rc); goto error; }
            }
        }

        NEXUS_Playback_GetSettings(player->parent->playback, &playbackSettings);
        playbackSettings.playpumpSettings.transportType = transportType;
        playbackSettings.playpumpSettings.timestamp.type = timestampType;
        playbackSettings.endOfStreamAction = psettings->loopMode;
        rc = NEXUS_Playback_SetSettings(player->parent->playback, &playbackSettings);
        if (rc) { rc = BERR_TRACE(rc); goto error; }

        for(track=BLST_SQ_FIRST(&player->stream->tracks);track;track=BLST_SQ_NEXT(track, link)) {
            if (track->type == bmedia_track_type_audio || track->type == bmedia_track_type_video) {
                if (prev_program_set) {
                    if (track->program != prev_program) {
                        cur_program_offset++;
                    }
                }
                prev_program = track->program;
                prev_program_set = true;
            }
            if (cur_program_offset != psettings->program) continue;

            switch(track->type) {
                case bmedia_track_type_audio:
                    if(track->info.audio.codec != baudio_format_unknown && player->parent->audioDecoder) {
                        if (!player->parent->audioProgram.primary.pidChannel) {
                            NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
                            playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eAudio;
                            playbackPidSettings.pidSettings.pidTypeSettings.audio.codec = b_audiocodec2nexus(track->info.audio.codec);
                            playbackPidSettings.pidTypeSettings.audio.simpleDecoder = player->parent->audioDecoder;
                            player->parent->audioProgram.primary.pidChannel = NEXUS_Playback_OpenPidChannel(player->parent->playback, track->number, &playbackPidSettings);
                            player->parent->audioProgram.primary.codec = playbackPidSettings.pidSettings.pidTypeSettings.audio.codec;
                        }
                    }
                    break;
                case bmedia_track_type_video:
                    if (player->parent->videoDecoder) {
                        if (track->info.video.codec == bvideo_codec_h264_svc || track->info.video.codec == bvideo_codec_h264_mvc) {
                            if (videoProgram.settings.pidChannel && !videoProgram.settings.enhancementPidChannel) {
                                NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
                                playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eVideo;
                                playbackPidSettings.pidTypeSettings.video.codec = b_videocodec2nexus(track->info.video.codec);
                                playbackPidSettings.pidTypeSettings.video.index = true;
                                playbackPidSettings.pidTypeSettings.video.simpleDecoder = player->parent->videoDecoder;
                                videoProgram.settings.enhancementPidChannel = NEXUS_Playback_OpenPidChannel(player->parent->playback, track->number, &playbackPidSettings);
                                videoProgram.settings.codec = b_videocodec2nexus(track->info.video.codec); /* overwrite */
                            }
                            break;
                        }
                        else if (track->info.video.codec != bvideo_codec_unknown) {
                            if (!videoProgram.settings.pidChannel) {
                                NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
                                playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eVideo;
                                playbackPidSettings.pidTypeSettings.video.codec = b_videocodec2nexus(track->info.video.codec);
                                playbackPidSettings.pidTypeSettings.video.index = true;
                                playbackPidSettings.pidTypeSettings.video.simpleDecoder = player->parent->videoDecoder;
                                videoProgram.settings.pidChannel = NEXUS_Playback_OpenPidChannel(player->parent->playback, track->number, &playbackPidSettings);
                                videoProgram.settings.codec = b_videocodec2nexus(track->info.video.codec);
                                if (player->parent->create_settings.maxWidth && player->parent->create_settings.maxHeight) {
                                    videoProgram.maxWidth = player->parent->create_settings.maxWidth;
                                    videoProgram.maxHeight = player->parent->create_settings.maxHeight;
                                }
                                if (track->info.video.width > videoProgram.maxWidth) {
                                    videoProgram.maxWidth = track->info.video.width;
                                }
                                if (track->info.video.height > videoProgram.maxHeight) {
                                    videoProgram.maxHeight = track->info.video.height;
                                }
                            }
                        }
                        player->parent->colorDepth = bmedia_probe_get_video_color_depth(track);
                    }
                    break;
#if 0
                    /* TODO: need playback to handle duplicate w/ different settings in the case of eOther */
                case bmedia_track_type_pcr:
                    NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
                    playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eOther;
                    player->parent->pcrPidChannel = NEXUS_Playback_OpenPidChannel(player->parent->playback, track->number, &playbackPidSettings);
                    break;
#endif
                default:
                    break;
            }
        }

#if B_HAS_AVI
        if (player->stream->type == bstream_mpeg_type_avi && ((bavi_probe_stream *)player->stream)->video_framerate) {
            NEXUS_LookupFrameRate(((bavi_probe_stream *)player->stream)->video_framerate, &videoProgram.settings.frameRate);
        }
#endif
    }

    player->parent->videoProgram = videoProgram;
    return 0;

error:
    return rc;
}

int media_player_ip_start_playback(media_player_ip_t player)
{
    int rc;
    B_PlaybackIpSessionStartSettings ipSessionStartSettings;
    B_PlaybackIpSessionStartStatus ipSessionStartStatus;
    NEXUS_PlaypumpSettings playpumpSettings;

    if (player->usePlaypump) {
        BDBG_MSG(("starting nexus playpump"));
        rc = NEXUS_Playpump_Start(player->playpump1);
        if (rc) return BERR_TRACE(rc);
        if (player->playpump2) {
            rc = NEXUS_Playpump_Start(player->playpump2);
            if (rc) return BERR_TRACE(rc);
        }
    }
    else
    {
        BDBG_MSG(("starting nexus playback"));
        rc = NEXUS_Playback_Start(player->parent->playback, player->file, NULL);
        if (rc) return BERR_TRACE(rc);
    }

    /* B_PlaybackIp_GetDefaultSessionStartSettings(&ipSessionStartSettings);*/
    memset(&ipSessionStartSettings, 0, sizeof(ipSessionStartSettings));
    /* set Nexus handles */
    ipSessionStartSettings.nexusHandles.playpump = player->playpump1;
    ipSessionStartSettings.nexusHandles.playpump2 = player->playpump2;
    ipSessionStartSettings.nexusHandles.playback = player->parent->playback;
    ipSessionStartSettings.nexusHandles.simpleVideoDecoder = player->parent->videoDecoder;
    ipSessionStartSettings.nexusHandles.simpleStcChannel = player->parent->stcChannel;
    ipSessionStartSettings.nexusHandles.simpleAudioDecoder = player->parent->audioDecoder;
    ipSessionStartSettings.nexusHandles.videoPidChannel = player->parent->videoProgram.settings.pidChannel;
    ipSessionStartSettings.nexusHandles.extraVideoPidChannel = player->parent->videoProgram.settings.enhancementPidChannel;
    ipSessionStartSettings.nexusHandles.audioPidChannel = player->parent->audioProgram.primary.pidChannel;
    ipSessionStartSettings.nexusHandles.pcrPidChannel = player->parent->pcrPidChannel;
    ipSessionStartSettings.nexusHandlesValid = true;
    ipSessionStartSettings.mpegType = playpumpSettings.transportType;
    ipSessionStartSettings.mediaPositionUsingWallClockTime = true;
    rc = B_PlaybackIp_SessionStart(player->playbackIp, &ipSessionStartSettings, &ipSessionStartStatus);
    if (rc) return BERR_TRACE(rc);
    BDBG_MSG(("playback playback in %s mode is started", player->playbackIpLiveMode?"Live":"Playback"));

    return 0;
}

void media_player_ip_stop(media_player_ip_t player)
{
    if (player->usePlaypump) {
        if (player->playpump1) {
            NEXUS_Playpump_Stop(player->playpump1);
            NEXUS_Playpump_CloseAllPidChannels(player->playpump1);
            NEXUS_Playpump_Close(player->playpump1);
            if (player->playpump2) {
                NEXUS_Playpump_Stop(player->playpump2);
                NEXUS_Playpump_CloseAllPidChannels(player->playpump2);
            }
            player->playbackIpLiveMode = false;
        }
        if (player->pacingTimebase)
        {
           NEXUS_Timebase_Close(player->pacingTimebase);
        }
    }
    else {
        /* playback mode */
        if (player->parent->playback) {
            NEXUS_Playback_Stop(player->parent->playback);
            NEXUS_Playback_CloseAllPidChannels(player->parent->playback);
        }
    }
    player->parent->videoProgram.settings.pidChannel = NULL;
    player->parent->audioProgram.primary.pidChannel = NULL;
    player->parent->pcrPidChannel = NULL;
    B_PlaybackIp_SessionStop(player->playbackIp);
    B_PlaybackIp_SessionClose(player->playbackIp);

#ifdef B_HAS_DTCP_IP
    if (player->dtcpEnabled) {
        DtcpAppLib_CloseAke(player->dtcpCtx, player->akeHandle);
    }
#endif
}

void media_player_ip_destroy(media_player_ip_t player)
{
    if (player->playbackIp) {
        B_PlaybackIp_Close(player->playbackIp);
    }
#ifdef B_HAS_DTCP_IP
    if (player->dtcpEnabled) {
        DtcpAppLib_Shutdown(player->dtcpCtx);
        DtcpCleanupHwSecurityParams();
    }
#endif
    BKNI_Free(player);
}

int media_ip_player_trick(media_player_ip_t player, int rate)
{
    B_PlaybackIpTrickModesSettings ipTrickModeSettings;
    B_PlaybackIp_GetTrickModeSettings(player->playbackIp, &ipTrickModeSettings);

    BDBG_MSG(("%s: rate %d, Playspeed num %d, values %s", BSTD_FUNCTION, rate, player->playbackIpPsi.numPlaySpeedEntries, player->playbackIpPsi.playSpeedString));

    if (rate == NEXUS_NORMAL_DECODE_RATE) {
        return B_PlaybackIp_Play(player->playbackIp);
    }
    else if (rate == 0) {
        ipTrickModeSettings.pauseMethod = B_PlaybackIpPauseMethod_UseConnectionStalling;
        return B_PlaybackIp_Pause(player->playbackIp, &ipTrickModeSettings);
    }
    else {
        if (player->playbackIpPsi.numPlaySpeedEntries) {
#define PLAYSPEED_STRING_SIZE 16
            char speedString[PLAYSPEED_STRING_SIZE];

            memset(speedString, 0, PLAYSPEED_STRING_SIZE);
            if (getPlaySpeed(player, rate, speedString, PLAYSPEED_STRING_SIZE-1) < 0)
            {
                return -1;
            }
            ipTrickModeSettings.playSpeedString = speedString;
            ipTrickModeSettings.playSpeedStringDefined = true;
            ipTrickModeSettings.method = B_PlaybackIpTrickModesMethod_UsePlaySpeed;
        }
        else {
            ipTrickModeSettings.method = B_PlaybackIpTrickModesMethod_UseByteRange;
            ipTrickModeSettings.rate = rate;
        }
        return B_PlaybackIp_TrickMode(player->playbackIp, &ipTrickModeSettings);
    }
}

int media_player_ip_get_playback_status(media_player_ip_t player, NEXUS_PlaybackStatus *pstatus )
{
    int rc;
    B_PlaybackIpStatus playbackIpStatus;
    rc = B_PlaybackIp_GetStatus(player->playbackIp, &playbackIpStatus);
    if (!rc) {
        pstatus->first = playbackIpStatus.first;
        pstatus->last = playbackIpStatus.last;
        pstatus->position = playbackIpStatus.position;
        return NEXUS_SUCCESS;
    }
    else {
        return NEXUS_UNKNOWN;
    }
}

int media_ip_player_seek(media_player_ip_t player, int offset, int whence )
{
    B_PlaybackIpStatus playbackIpStatus;
    B_PlaybackIpTrickModesSettings ipTrickModeSettings;
    int rc;
    switch (whence) {
    case SEEK_SET:
    default:
        break;
    case SEEK_CUR:
        rc = B_PlaybackIp_GetStatus(player->playbackIp, &playbackIpStatus);
        if (!rc) offset += playbackIpStatus.position;
        break;
    case SEEK_END:
        rc = B_PlaybackIp_GetStatus(player->playbackIp, &playbackIpStatus);
        if (!rc) offset += playbackIpStatus.last;
        break;
    }
    B_PlaybackIp_GetTrickModeSettings(player->playbackIp, &ipTrickModeSettings);
    ipTrickModeSettings.seekPosition = offset;
    return B_PlaybackIp_Seek(player->playbackIp, &ipTrickModeSettings);
}

#if B_REFSW_TR69C_SUPPORT
int media_player_ip_get_set_tr69c_info(void *context, enum b_tr69c_type type, union b_tr69c_info *info)
{
    media_player_ip_t player = context;
    int rc;
    switch (type)
    {
        case b_tr69c_type_get_playback_ip_status:
            rc = B_PlaybackIp_GetStatus(player->playbackIp, &info->playback_ip_status);
            if (rc) return BERR_TRACE(rc);
            break;
        default:
            return BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }
    return 0;
}
#endif

#else
/* stub API if PLAYBACK_IP_SUPPORT is not defined */
media_player_ip_t media_player_ip_create(media_player_t parent)
{
    BSTD_UNUSED(parent);
    return NULL;
}

void media_player_ip_destroy(media_player_ip_t player)
{
    BSTD_UNUSED(player);
}

int media_player_ip_start(media_player_ip_t player, const media_player_start_settings *psettings, const struct url *url)
{
    BSTD_UNUSED(player);
    BSTD_UNUSED(psettings);
    BSTD_UNUSED(url);
    return NEXUS_NOT_SUPPORTED;
}

int media_player_ip_start_playback(media_player_ip_t player)
{
    BSTD_UNUSED(player);
    return NEXUS_NOT_SUPPORTED;
}

void media_player_ip_stop(media_player_ip_t player)
{
    BSTD_UNUSED(player);
}

int media_ip_player_trick(media_player_ip_t player, int rate)
{
    BSTD_UNUSED(player);
    BSTD_UNUSED(rate);
    return NEXUS_NOT_SUPPORTED;
}

int media_player_ip_get_playback_status(media_player_ip_t player, NEXUS_PlaybackStatus *pstatus)
{
    BSTD_UNUSED(player);
    BSTD_UNUSED(pstatus);
    return NEXUS_NOT_SUPPORTED;
}

int media_ip_player_seek(media_player_ip_t player, int offset, int whence)
{
    BSTD_UNUSED(player);
    BSTD_UNUSED(offset);
    BSTD_UNUSED(whence);
    return NEXUS_NOT_SUPPORTED;
}

#if B_REFSW_TR69C_SUPPORT
int media_player_ip_get_set_tr69c_info(void *context, enum b_tr69c_type type, union b_tr69c_info *info)
{
    BSTD_UNUSED(context);
    BSTD_UNUSED(type);
    BSTD_UNUSED(info);
    return NEXUS_NOT_SUPPORTED;
}
#endif

#endif /* PLAYBACK_IP_SUPPORT */
#endif
