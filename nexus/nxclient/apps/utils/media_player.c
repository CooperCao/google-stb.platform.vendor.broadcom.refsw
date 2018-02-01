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
#if NEXUS_HAS_PLAYBACK && NEXUS_HAS_SIMPLE_DECODER
#include "media_player.h"
#include "media_player_priv.h"
#include "glob.h"
#include "nexus_file_chunk.h"
#include "bcmindexer.h"
#include "bcmindexer_nav.h"
#include <sys/stat.h>

BDBG_MODULE(media_player);

struct media_player_ip;

BDBG_OBJECT_ID(media_player);

void media_player_get_default_create_settings( media_player_create_settings *psettings )
{
    NxClient_ConnectSettings connectSettings;
    memset(psettings, 0, sizeof(*psettings));
    NxClient_GetDefaultConnectSettings(&connectSettings);
    psettings->userDataBufferSize = connectSettings.simpleVideoDecoder[0].decoderCapabilities.userDataBufferSize;
}

void media_player_get_default_start_settings( media_player_start_settings *psettings )
{
    memset(psettings, 0, sizeof(*psettings));
    psettings->decrypt.algo = NEXUS_SecurityAlgorithm_eMax; /* none */
#if NEXUS_HAS_AUDIO
    psettings->audio.dolbyDrcMode = NEXUS_AudioDecoderDolbyDrcMode_eMax; /* none */
#endif
    psettings->stcTrick = true;
    psettings->video.pid = 0x1fff;
    psettings->video.scanMode = NEXUS_VideoDecoderScanMode_eAuto;
    psettings->audio.pid = 0x1fff;
    psettings->sync = NEXUS_SimpleStcChannelSyncMode_eDefaultAdjustmentConcealment;
    psettings->stcMaster = NEXUS_StcChannelAutoModeBehavior_eVideoMaster;
    psettings->transportType = NEXUS_TransportType_eMax; /* no override */
    psettings->playbackSettings.endOfStreamTimeout = 180; /* copied from nexus_playback */
    psettings->mediaPlayerSettings.audio.ac3_service_type = UINT_MAX;
    psettings->mediaPlayerSettings.audio.language = NULL;
    psettings->mediaPlayerSettings.enableDynamicTrackSelection = false;
}

static void endOfStreamCallback(void *context, int param)
{
    media_player_t player = context;
    BSTD_UNUSED(param);
    if (player->start_settings.eof) {
        (player->start_settings.eof)(player->start_settings.context);
    }
}

static void callback_3dtv(void *context, int param)
{
    NEXUS_SimpleVideoDecoderHandle videoDecoder = context;
    NEXUS_VideoDecoder3DTVStatus status;
    BSTD_UNUSED(param);
    NEXUS_SimpleVideoDecoder_Get3DTVStatus(videoDecoder, &status);
    BDBG_WRN(("%s stream detected", status.format == NEXUS_VideoDecoder3DTVFormat_eNone ? "2D":"3D"));
    /* App could decide to switch display into 3D mode here. */
}

static media_player_t media_player_p_create(const NxClient_AllocResults *pAllocResults, unsigned allocIndex, const media_player_create_settings *psettings, bool mosaic)
{
    NEXUS_PlaypumpOpenSettings playpumpOpenSettings;
    NEXUS_PlaybackSettings playbackSettings;
    media_player_t player;
    int rc;

    player = (media_player_t)BKNI_Malloc(sizeof(*player));
    if (!player) {
        return NULL;
    }
    memset(player, 0, sizeof(*player));
    BDBG_OBJECT_SET(player, media_player);
    player->create_settings = *psettings;
    player->allocResults = *pAllocResults;
    player->allocIndex = allocIndex;

    /* Check whether to use playback_ip or BIP.*/
    if(getenv("use_pbip"))
    {
        player->usePbip = true;
    }

    if (mosaic) {
        NxClient_AllocSettings allocSettings;
        NxClient_GetDefaultAllocSettings(&allocSettings);
        allocSettings.simpleAudioDecoder = 1;
        rc = NxClient_Alloc(&allocSettings, &player->audio.allocResults);
        if (rc) { rc = BERR_TRACE(rc); goto error_alloc;}
    }

    NEXUS_Playpump_GetDefaultOpenSettings(&playpumpOpenSettings);
    if (psettings->playback.fifoSize) {
        playpumpOpenSettings.fifoSize = psettings->playback.fifoSize;
    }
    else if (psettings->maxHeight <= 576) {
        playpumpOpenSettings.fifoSize = 1024*1024;
    }
    player->playpump = NEXUS_Playpump_Open(NEXUS_ANY_ID, &playpumpOpenSettings);
    if (!player->playpump) {
        BKNI_Free(player);
        BDBG_WRN(("no more playpumps available"));
        return NULL;
    }
    player->playback = NEXUS_Playback_Create();
    BDBG_ASSERT(player->playback);

    if (!getenv("force_vsync")) {
        player->stcChannel = NEXUS_SimpleStcChannel_Create(NULL);
        BDBG_ASSERT(player->stcChannel);
    }

    NEXUS_Playback_GetSettings(player->playback, &playbackSettings);
    playbackSettings.playpump = player->playpump;
    playbackSettings.simpleStcChannel = player->stcChannel;
    playbackSettings.endOfStreamCallback.callback = endOfStreamCallback;
    playbackSettings.endOfStreamCallback.context = player;
    rc = NEXUS_Playback_SetSettings(player->playback, &playbackSettings);
    BDBG_ASSERT(!rc);

    if (player->allocResults.simpleVideoDecoder[player->allocIndex].id) {
        player->videoDecoder = NEXUS_SimpleVideoDecoder_Acquire(player->allocResults.simpleVideoDecoder[player->allocIndex].id);
        NEXUS_SimpleVideoDecoder_GetSettings(player->videoDecoder, &player->videoDecoderSettings);
    }
    if (!player->videoDecoder) {
        BDBG_WRN(("video decoder not available"));
    }

    if (player->audio.allocResults.simpleAudioDecoder.id) {
        player->audioDecoder = NEXUS_SimpleAudioDecoder_Acquire(player->audio.allocResults.simpleAudioDecoder.id);
        if (!player->audioDecoder) {
            BDBG_WRN(("audio decoder not available"));
        }
    }
    else if (player->allocIndex == 0) {
        if (player->allocResults.simpleAudioDecoder.id) {
            player->audioDecoder = NEXUS_SimpleAudioDecoder_Acquire(player->allocResults.simpleAudioDecoder.id);
        }
        if (!player->audioDecoder) {
            BDBG_WRN(("audio decoder not available"));
        }
    }

    if(player->usePbip)
    {
        player->ip = media_player_ip_create(player);
    }
    else
    {
        player->bip = media_player_bip_create(player);;
    }
    /* if we can't create, proceed without IP support */

    return player;

error_alloc:
    BKNI_Free(player);
    return NULL;
}

static bool media_player_p_test_disconnect(media_player_t player, NEXUS_SimpleVideoDecoderStartSettings *pVideoProgram)
{
    return player->connectId && player->videoDecoder &&
        (player->videoProgram.displayEnabled != pVideoProgram->displayEnabled ||
         !player->videoDecoderSettings.supportedCodecs[pVideoProgram->settings.codec] ||
         player->colorDepth != player->videoDecoderSettings.colorDepth);
}

static int media_player_p_connect_persistents(media_player_t player)
{
    NxClient_ConnectSettings audioConnectSettings;
    int rc;

    if (!player->master || !player->audio.allocResults.simpleAudioDecoder.id) {
        return BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }
    if (!player->create_settings.audio.usePersistent) {
        BDBG_ERR(("media_player_p_connect_persistents should only used for connecting persisent decoders"));
        return BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }

    NxClient_GetDefaultConnectSettings(&audioConnectSettings);
    audioConnectSettings.simpleAudioDecoder.id = player->audio.allocResults.simpleAudioDecoder.id;
    audioConnectSettings.simpleAudioDecoder.decoderCapabilities.type = NxClient_AudioDecoderType_ePersistent;

    if (player->master && player->master->audio.persistentMasterConnectId != 0) {
        audioConnectSettings.simpleAudioDecoder.primer = true;
    }

    rc = NxClient_Connect(&audioConnectSettings, &player->audio.connectId);
    if (rc) return BERR_TRACE(rc);

    if (player->master && player->master->audio.persistentMasterConnectId == 0) {
        player->master->audio.persistentMasterConnectId = player->audio.connectId;
    }

    return 0;
}

/* connect client resources to server's resources */
static int media_player_p_connect(media_player_t player)
{
    NxClient_ConnectSettings connectSettings;
    int rc;

    if (player->videoDecoder && player->videoDecoderSettings.colorDepth != player->colorDepth) {
        /* no media_player API for colorDepth. instead, we force an NxClient_Connect based on a probe-detected
        colorDepth change from current settings. if we get dynamic changes from 8->10 in the future, we'll need a
        user setting to start with 10 bit. */
        NEXUS_SimpleVideoDecoder_GetSettings(player->videoDecoder, &player->videoDecoderSettings);
        player->videoDecoderSettings.colorDepth = player->colorDepth;
        rc = NEXUS_SimpleVideoDecoder_SetSettings(player->videoDecoder, &player->videoDecoderSettings);
        if (rc) return BERR_TRACE(rc);
    }

    NxClient_GetDefaultConnectSettings(&connectSettings);
    if (player->master) {
        unsigned i;
        media_player_t p;

        if (player->master->mosaic_start_count) {
            /* already connected but get connectIds for persistent decoders */
            if (player->create_settings.audio.usePersistent) {
                rc = media_player_p_connect_persistents(player);
                if (rc) return BERR_TRACE(rc);
            }
            player->master->mosaic_start_count++;
            return 0;
        }
        for (i=0,p=BLST_Q_FIRST(&player->master->players);p;i++,p=BLST_Q_NEXT(p,link)) {
            const media_player_create_settings *psettings = &p->create_settings;
            connectSettings.simpleVideoDecoder[i].id = player->master->allocResults.simpleVideoDecoder[i].id;
            connectSettings.simpleVideoDecoder[i].surfaceClientId = psettings->window.surfaceClientId;
            connectSettings.simpleVideoDecoder[i].windowId = psettings->window.id+i;
            if (player->create_settings.maxFormat) {
                connectSettings.simpleVideoDecoder[i].decoderCapabilities.maxFormat = player->create_settings.maxFormat;
            }
            else {
                connectSettings.simpleVideoDecoder[i].decoderCapabilities.maxWidth = player->videoProgram.maxWidth;
                connectSettings.simpleVideoDecoder[i].decoderCapabilities.maxHeight = player->videoProgram.maxHeight;
            }
            connectSettings.simpleVideoDecoder[i].decoderCapabilities.colorDepth = player->colorDepth;
            connectSettings.simpleVideoDecoder[i].decoderCapabilities.fifoSize = player->start_settings.video.fifoSize;
            connectSettings.simpleVideoDecoder[i].decoderCapabilities.itbFifoSize = player->start_settings.video.itbFifoSize;
            connectSettings.simpleVideoDecoder[i].decoderCapabilities.userDataBufferSize = player->create_settings.userDataBufferSize;
            connectSettings.simpleVideoDecoder[i].windowCapabilities.type = player->start_settings.videoWindowType;
        }
        if (player->audio.allocResults.simpleAudioDecoder.id) {
            rc = media_player_switch_audio(player);
            if (rc) return BERR_TRACE(rc);
        }
    }
    else {
        const media_player_create_settings *psettings = &player->create_settings;
        if (player->videoProgram.settings.pidChannel) {
            connectSettings.simpleVideoDecoder[0].id = player->allocResults.simpleVideoDecoder[0].id;
            connectSettings.simpleVideoDecoder[0].surfaceClientId = psettings->window.surfaceClientId;
            connectSettings.simpleVideoDecoder[0].windowId = psettings->window.id;
            if (player->create_settings.maxFormat) {
                NEXUS_VideoFormatInfo info;
                NEXUS_VideoFormat_GetInfo(player->create_settings.maxFormat, &info);
                connectSettings.simpleVideoDecoder[0].decoderCapabilities.maxWidth = player->videoProgram.maxWidth = info.width;
                connectSettings.simpleVideoDecoder[0].decoderCapabilities.maxHeight = player->videoProgram.maxHeight = info.height;
                connectSettings.simpleVideoDecoder[0].decoderCapabilities.maxFormat = player->create_settings.maxFormat;
            }
            else {
                connectSettings.simpleVideoDecoder[0].decoderCapabilities.maxWidth = player->videoProgram.maxWidth;
                connectSettings.simpleVideoDecoder[0].decoderCapabilities.maxHeight = player->videoProgram.maxHeight;
            }
            connectSettings.simpleVideoDecoder[0].decoderCapabilities.supportedCodecs[player->videoProgram.settings.codec] = true;
            connectSettings.simpleVideoDecoder[0].decoderCapabilities.colorDepth = player->colorDepth;
            connectSettings.simpleVideoDecoder[0].decoderCapabilities.fifoSize = player->start_settings.video.fifoSize;
            connectSettings.simpleVideoDecoder[0].decoderCapabilities.itbFifoSize = player->start_settings.video.itbFifoSize;
            connectSettings.simpleVideoDecoder[0].decoderCapabilities.secureVideo = player->start_settings.video.secure;
            connectSettings.simpleVideoDecoder[0].decoderCapabilities.virtualized = player->start_settings.video.virtualized;
            connectSettings.simpleVideoDecoder[0].decoderCapabilities.userDataBufferSize = player->create_settings.userDataBufferSize;
            connectSettings.simpleVideoDecoder[0].windowCapabilities.type = player->start_settings.videoWindowType;
        }
#if NEXUS_HAS_AUDIO
        if (player->audioProgram.primary.pidChannel) {
            connectSettings.simpleAudioDecoder.id = player->allocResults.simpleAudioDecoder.id;
            connectSettings.simpleAudioDecoder.primer = (player->start_settings.audio_primers == media_player_audio_primers_immediate);
            connectSettings.simpleAudioDecoder.decoderCapabilities.type = (player->create_settings.audio.usePersistent ? NxClient_AudioDecoderType_ePersistent : NxClient_AudioDecoderType_eDynamic);
        }
#endif
    }
    rc = NxClient_Connect(&connectSettings, &player->connectId);
    if (rc) return BERR_TRACE(rc);

    if (player->master) {
        player->master->mosaic_start_count++;
    }

    return 0;
}

static void media_player_p_disconnect(media_player_t player)
{
    if (player->master) {
        if (player->audio.connectId) {
            NxClient_Disconnect(player->audio.connectId);
            player->audio.connectId = 0;
        }

        if (!player->master->mosaic_start_count || --player->master->mosaic_start_count) return;
        if (player->master->connectId) {
            NxClient_Disconnect(player->master->connectId);
            player->master->connectId = 0;
        }
    }
    else {
        if (player->connectId) {
            NxClient_Disconnect(player->connectId);
            player->connectId = 0;
        }
    }
}

media_player_t media_player_create( const media_player_create_settings *psettings )
{
    NxClient_JoinSettings joinSettings;
    NxClient_AllocSettings allocSettings;
    NxClient_AllocResults allocResults;
    media_player_t player;
    int rc;
    media_player_create_settings default_settings;

    if (!psettings) {
        media_player_get_default_create_settings(&default_settings);
        psettings = &default_settings;
    }

    /* connect to server and nexus */
    NxClient_GetDefaultJoinSettings(&joinSettings);
    snprintf(joinSettings.name, NXCLIENT_MAX_NAME, "%s", "media_player");
    rc = NxClient_Join(&joinSettings);
    if (rc) {
        return NULL;
    }

    NxClient_GetDefaultAllocSettings(&allocSettings);
    allocSettings.simpleVideoDecoder = 1;
    allocSettings.simpleAudioDecoder = 1;
    rc = NxClient_Alloc(&allocSettings, &allocResults);
    if (rc) { rc = BERR_TRACE(rc); goto error_alloc;}

    player = media_player_p_create(&allocResults, 0, psettings, false);
    if (!player) goto error_create_player;

    player->settings.audio.ac3_service_type = UINT_MAX;

    return player;

error_create_player:
    NxClient_Free(&allocResults);
error_alloc:
    NxClient_Uninit();
    return NULL;
}

int media_player_create_mosaics(media_player_t *players, unsigned num_mosaics, const media_player_create_settings *psettings )
{
    NxClient_JoinSettings joinSettings;
    NxClient_AllocSettings allocSettings;
    NxClient_AllocResults allocResults;
    int rc;
    media_player_create_settings default_settings;
    unsigned i;

    if (!psettings) {
        media_player_get_default_create_settings(&default_settings);
        psettings = &default_settings;
    }

    /* connect to server and nexus */
    NxClient_GetDefaultJoinSettings(&joinSettings);
    snprintf(joinSettings.name, NXCLIENT_MAX_NAME, "%s", "media_player");
    rc = NxClient_Join(&joinSettings);
    if (rc) return BERR_TRACE(rc);

    NxClient_GetDefaultAllocSettings(&allocSettings);
    allocSettings.simpleVideoDecoder = num_mosaics;
    rc = NxClient_Alloc(&allocSettings, &allocResults);
    if (rc) { rc = BERR_TRACE(rc); goto error_alloc;}

    BKNI_Memset(players, 0, sizeof(players[0])*num_mosaics);
    for (i=0;i<num_mosaics;i++) {
        players[i] = media_player_p_create(&allocResults, i, psettings, true);
        if (!players[i]) {
            if (i == 0) {
                rc = BERR_TRACE(NEXUS_NOT_AVAILABLE);
                goto error_create_player;
            }
            /* if we can create at least one, go with it */
            break;
        }

        players[i]->master = players[0];
        BLST_Q_INSERT_TAIL(&players[0]->players, players[i], link);
    }

    return 0;

error_create_player:
    media_player_destroy_mosaics(players, num_mosaics);
    NxClient_Free(&allocResults);
error_alloc:
    NxClient_Uninit();
    BDBG_ASSERT(rc);
    return rc;
}

void media_player_destroy_mosaics( media_player_t *players, unsigned num_mosaics )
{
    int i;
    for (i=num_mosaics-1;i>=0;i--) {
        if (players[i]) {
            BLST_Q_REMOVE(&players[0]->players, players[i], link);
            media_player_destroy(players[i]);
        }
    }
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

example: udp://192.168.1.10:1234 || udp://224.1.1.10:1234 || udp://@:1234

    scheme=udp
    domain=live channel is streamed to this STB's IP address 192.168.1.10 || server is streaming to a multicast address 224.1.1.10 || this STB but user doesn't need to explicitly specify its IP address
    port=1234
    path=N/A as server is just streaming out a live channel, client doesn't get to pick a file

*/
static void parse_url(const char *s, struct url *url)
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

static int glob_errfunc(const char *epath, int eerrno)
{
    BSTD_UNUSED(epath);
    BSTD_UNUSED(eerrno);
    BDBG_ERR(("glob_errfunc %s %d", epath, eerrno));
    return -1;
}

static int detect_chunk(const char *fname, off_t *chunk_size, unsigned *first_chunk_number)
{
    char path[64];
    int rc;
    glob_t glb;

    snprintf(path, sizeof(path), "%s_*", fname);
    rc = glob(path, 0, glob_errfunc, &glb);
    if (rc) {
        BDBG_ERR(("no chunk files found with name %s", fname));
        return -1;
    }
    if (glb.gl_pathc) {
        /* TODO: first file may not be first chunk more than 9*64 */
        unsigned fname_len = strlen(fname);
        unsigned len = strlen(glb.gl_pathv[0]);
        struct stat st;
        unsigned n;

        if (len <= fname_len) {
            rc = BERR_TRACE(-1);
            goto done;
        }

        n = atoi(&glb.gl_pathv[0][fname_len+1]);
        *first_chunk_number = ((n/1000)*64) + n%1000;

        rc = stat(glb.gl_pathv[0], &st);
        if (rc) {BERR_TRACE(rc); goto done;}
        *chunk_size = st.st_size;
        printf("found file %s with chunk size %u, first chunk %u\n", glb.gl_pathv[0], (unsigned)*chunk_size, *first_chunk_number);
        rc = 0;
    }
done:
    globfree(&glb);
    return rc;
}

int media_player_start( media_player_t player, const media_player_start_settings *psettings )
{
    NEXUS_PlaybackPidChannelSettings playbackPidSettings;
    NEXUS_PlaybackSettings playbackSettings;
    NxClient_DisplaySettings displaySettings;
    int rc;
    struct url url;

    BDBG_OBJECT_ASSERT(player, media_player);
    if (player->started) {
        return BERR_TRACE(NEXUS_NOT_AVAILABLE);
    }
    if (!psettings) {
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }
    if (!psettings->stream_url) {
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }

    player->start_settings = *psettings;
    NEXUS_Playback_GetDefaultTrickModeSettings(&player->trickMode);

    /* open pid channels and configure start settings based on probe */
    NEXUS_SimpleAudioDecoder_GetDefaultStartSettings(&player->audioProgram);

    NxClient_GetDisplaySettings(&displaySettings);
    if (displaySettings.hdmiPreferences.hdcp != psettings->hdcp ||
        displaySettings.hdmiPreferences.version != psettings->hdcp_version) {
        displaySettings.hdmiPreferences.hdcp = psettings->hdcp;
        displaySettings.hdmiPreferences.version = psettings->hdcp_version;
        NxClient_SetDisplaySettings(&displaySettings);
    }

    parse_url(psettings->stream_url, &url);

    if (!strcasecmp(url.scheme, "http") || !strcasecmp(url.scheme, "https") || !strcasecmp(url.scheme, "udp") || !strcasecmp(url.scheme, "rtp")) {
        if(player->usePbip) {
            if (!player->ip) {
                BDBG_ERR(("%s: not supported. Rebuild with PLAYBACK_IP_SUPPORT=y", url.scheme));
                return NEXUS_NOT_SUPPORTED;
            }
            rc = media_player_ip_start(player->ip, psettings, &url);
            if (rc) return BERR_TRACE(rc);
        }
        else
        {
            if (!player->bip) {
                BDBG_ERR(("%s: not supported. Rebuild with PLAYBACK_IP_SUPPORT=y", psettings->stream_url));
                return NEXUS_NOT_SUPPORTED;
            }
            rc = media_player_bip_start(player->bip, psettings, psettings->stream_url);
            if (rc) return BERR_TRACE(rc);

            media_player_bip_get_color_depth(player->bip, &player->colorDepth);
        }
        player->ipActive = true;
        player->started = true; /* set started true here since we need to unwrap the resources even if rest of the code like videoDecoder/audioDecoder start failed.*/
    }
    else {
        struct dvr_crypto_settings settings;
        struct probe_results probe_results;
        const char *index_url = NULL;
        NEXUS_SimpleVideoDecoderStartSettings videoProgram;
        NEXUS_SimpleStcChannelSettings stcSettings;
        NEXUS_ChunkedFilePlayOpenSettings chunkedFilePlayOpenSettings;
        const char *probe_file;
        char first_chunk_name[128];

        if (psettings->chunked) {
            NEXUS_ChunkedFilePlay_GetDefaultOpenSettings(&chunkedFilePlayOpenSettings);
            /* firstChunkNumber can be obtained from NEXUS_ChunkedFifoRecordExportStatus.last.chunkNumber in an integrated app.
            For modular examples, we detect it. */
            detect_chunk(url.path, &chunkedFilePlayOpenSettings.chunkSize, &chunkedFilePlayOpenSettings.firstChunkNumber);
            strcpy(chunkedFilePlayOpenSettings.chunkTemplate, "%s_%04u");
            snprintf(first_chunk_name, sizeof(first_chunk_name), chunkedFilePlayOpenSettings.chunkTemplate, url.path, chunkedFilePlayOpenSettings.firstChunkNumber);
            probe_file = first_chunk_name;
        }
        else {
            probe_file = url.path;
        }

        NEXUS_SimpleVideoDecoder_GetDefaultStartSettings(&videoProgram);

        if (psettings->transportType != NEXUS_TransportType_eMax) {
            BKNI_Memset(&probe_results, 0, sizeof(probe_results));
            probe_results.transportType = psettings->transportType;
            probe_results.video[0].codec = NEXUS_VideoCodec_eMpeg2;
            probe_results.audio[0].codec = NEXUS_AudioCodec_eMpeg;
            switch (probe_results.transportType) {
            case NEXUS_TransportType_eMkv:
            case NEXUS_TransportType_eMp4:
                probe_results.useStreamAsIndex = true;
                break;
            default:
                break;
            }
        }
        else {
            struct probe_request probe_request;
            probe_media_get_default_request(&probe_request);
            probe_request.streamname = probe_file;
            probe_request.program = psettings->program;
            probe_request.quiet = psettings->quiet;
            probe_request.decrypt.algo = psettings->decrypt.algo;
            dvr_crypto_get_default_settings(&settings);
            probe_request.decrypt.key.data = settings.key.data;
            probe_request.decrypt.key.size = settings.key.size;
            rc = probe_media_request(&probe_request, &probe_results);
            if (rc) {
                BDBG_ERR(("media probe can't parse '%s'", probe_file));
                rc = -1;
                goto error;
            }
        }

        /* override probe */
        if (psettings->video.codec) {
            probe_results.video[0].codec = psettings->video.codec;
        }
        if (psettings->video.colorDepth) {
            probe_results.video[0].colorDepth = psettings->video.colorDepth;
        }
        if (psettings->video.pid != 0x1fff) {
            probe_results.video[0].pid = psettings->video.pid;
            probe_results.num_video = probe_results.video[0].pid?1:0;
        }
        if (psettings->audio.codec) {
            probe_results.audio[0].codec = psettings->audio.codec;
        }
        if (psettings->audio.pid != 0x1fff) {
            probe_results.audio[0].pid = psettings->audio.pid;
            probe_results.num_audio = probe_results.audio[0].pid?1:0;
        }

        index_url = psettings->index_url;
        if (!index_url && probe_results.useStreamAsIndex) {
            /* if user didn't specify an index, use the file as index if probe indicates */
            index_url = url.path;
        }

        if (probe_results.enableStreamProcessing) {
            index_url = url.path;
            NEXUS_Playback_GetSettings(player->playback, &playbackSettings);
            playbackSettings.enableStreamProcessing = true;
            BDBG_WRN(("enabling enableStreamProcessing for better ES navigation"));
            (void)NEXUS_Playback_SetSettings(player->playback, &playbackSettings);
        }

        if (psettings->chunked) {
            player->file = NEXUS_ChunkedFilePlay_Open(url.path, index_url, &chunkedFilePlayOpenSettings);
        }
        else {
            player->file = NEXUS_FilePlay_OpenPosix(url.path, index_url);
        }
        if (!player->file) {
            BDBG_ERR(("can't open '%s' and '%s'", url.path, index_url));
            rc = -1;
            goto error;
        }

        /* must set started = true here so pidChannels are freed if the rest of start fails */
        player->started = true;

        if (player->videoDecoder) {
            NEXUS_SimpleVideoDecoderHandle videoDecoder = player->videoDecoder;
            NEXUS_VideoDecoderSettings settings;

            if (psettings->source3d == source3d_auto) {
                NEXUS_VideoDecoderExtendedSettings extSettings;
                NEXUS_SimpleVideoDecoder_GetExtendedSettings(videoDecoder, &extSettings);
                extSettings.s3DTVStatusChanged.callback = callback_3dtv;
                extSettings.s3DTVStatusChanged.context = videoDecoder;
                extSettings.s3DTVStatusEnabled = true;
                extSettings.s3DTVStatusTimeout = 0;
                NEXUS_SimpleVideoDecoder_SetExtendedSettings(videoDecoder, &extSettings);
            }

            NEXUS_SimpleVideoDecoder_GetSettings(videoDecoder, &settings);
            if (psettings->source3d == source3d_auto || psettings->source3d == source3d_none) {
                settings.customSourceOrientation = false;
                /* for MVC codec, default to correct source orientation for fullres 3D */
                if (probe_results.num_video && probe_results.video[0].enhancement.codec==NEXUS_VideoCodec_eH264_Mvc) {
                    settings.customSourceOrientation = true;
                    settings.sourceOrientation = NEXUS_VideoDecoderSourceOrientation_e3D_LeftRightFullFrame;
                }
            }
            else if (psettings->source3d == source3d_lr)  {
                settings.customSourceOrientation = true;
                settings.sourceOrientation = NEXUS_VideoDecoderSourceOrientation_e3D_LeftRight;
            }
            else if (psettings->source3d == source3d_ou)  {
                settings.customSourceOrientation = true;
                settings.sourceOrientation = NEXUS_VideoDecoderSourceOrientation_e3D_OverUnder;
            }
            settings.scanMode = psettings->video.scanMode;
            NEXUS_SimpleVideoDecoder_SetSettings(videoDecoder, &settings);
        }

        if (player->stcChannel) {
            bool setStc = false;
            NEXUS_SimpleStcChannel_GetSettings(player->stcChannel, &stcSettings);
            if (stcSettings.sync != psettings->sync)
            {
                stcSettings.sync = psettings->sync;
                setStc = true;
            }
            if (stcSettings.modeSettings.Auto.transportType != probe_results.transportType) {
                stcSettings.modeSettings.Auto.transportType = probe_results.transportType;
                setStc = true;
            }
            if (stcSettings.modeSettings.Auto.behavior != psettings->stcMaster) {
                stcSettings.modeSettings.Auto.behavior = psettings->stcMaster;
                setStc = true;
            }
            if (stcSettings.astm != psettings->astm)
            {
                stcSettings.astm = psettings->astm;
                setStc = true;
            }
            if (setStc)
            {
                rc = NEXUS_SimpleStcChannel_SetSettings(player->stcChannel, &stcSettings);
                if (rc) { rc = BERR_TRACE(rc); goto error; }
            }
        }

        NEXUS_Playback_GetSettings(player->playback, &playbackSettings);
        playbackSettings.playpumpSettings.transportType = probe_results.transportType;
        playbackSettings.playpumpSettings.timestamp.type = probe_results.timestampType;
        playbackSettings.playpumpSettings.timestamp.pacing = psettings->pacing;
        playbackSettings.endOfStreamAction = psettings->loopMode;
        playbackSettings.timeshifting = (psettings->loopMode == NEXUS_PlaybackLoopMode_ePlay);
        playbackSettings.stcTrick = psettings->stcTrick && player->stcChannel;
        playbackSettings.startPaused = psettings->startPaused;
        playbackSettings.frameReverse.gopDepth = psettings->dqtFrameReverse;
        playbackSettings.endOfStreamTimeout = psettings->playbackSettings.endOfStreamTimeout;
        rc = NEXUS_Playback_SetSettings(player->playback, &playbackSettings);
        if (rc) { rc = BERR_TRACE(rc); goto error; }

#if NEXUS_HAS_AUDIO
        if (probe_results.num_audio && player->audioDecoder) {
            NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
            playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eAudio;
            playbackPidSettings.pidSettings.pidTypeSettings.audio.codec = probe_results.audio[0].codec;
            playbackPidSettings.pidTypeSettings.audio.simpleDecoder = player->audioDecoder;
            player->audioProgram.primary.pidChannel = NEXUS_Playback_OpenPidChannel(player->playback, probe_results.audio[0].pid, &playbackPidSettings);
            player->audioProgram.primary.codec = playbackPidSettings.pidSettings.pidTypeSettings.audio.codec;
        }
#endif
        if (probe_results.num_video && player->videoDecoder) {
            if (probe_results.video[0].enhancement.pid) {
                NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
                playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eVideo;
                playbackPidSettings.pidTypeSettings.video.codec = probe_results.video[0].enhancement.codec;
                playbackPidSettings.pidTypeSettings.video.index = true;
                playbackPidSettings.pidTypeSettings.video.simpleDecoder = player->videoDecoder;
                videoProgram.settings.enhancementPidChannel = NEXUS_Playback_OpenPidChannel(player->playback, probe_results.video[0].enhancement.pid, &playbackPidSettings);
            }
            NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
            playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eVideo;
            playbackPidSettings.pidTypeSettings.video.codec = probe_results.video[0].codec;
            playbackPidSettings.pidTypeSettings.video.index = true;
            playbackPidSettings.pidTypeSettings.video.simpleDecoder = player->videoDecoder;
            videoProgram.settings.pidChannel = NEXUS_Playback_OpenPidChannel(player->playback, probe_results.video[0].pid, &playbackPidSettings);

            videoProgram.settings.codec = probe_results.video[0].enhancement.pid ? probe_results.video[0].enhancement.codec : probe_results.video[0].codec;
            if (player->create_settings.maxFormat) {
                NEXUS_VideoFormatInfo info;
                NEXUS_VideoFormat_GetInfo(player->create_settings.maxFormat, &info);
                videoProgram.maxWidth = info.width;
                videoProgram.maxHeight = info.height;
            }
            else if (player->create_settings.maxWidth && player->create_settings.maxHeight) {
                videoProgram.maxWidth = player->create_settings.maxWidth;
                videoProgram.maxHeight = player->create_settings.maxHeight;
            }
            else {
                /* This could raise the maxWidth/Height for 4K, which is required.
                It could also lower the default maxWidth/Height for SD. Lowering is of value if the system has SD-only decoders
                or if there's video/graphics memory sharing; it is harmless otherwise. */
                videoProgram.maxWidth = probe_results.video[0].width;
                videoProgram.maxHeight = probe_results.video[0].height;
            }
            videoProgram.settings.frameRate = probe_results.video[0].frameRate;
            videoProgram.settings.aspectRatio = probe_results.video[0].aspectRatio;
            videoProgram.settings.sampleAspectRatio.x = probe_results.video[0].sampleAspectRatio.x;
            videoProgram.settings.sampleAspectRatio.y = probe_results.video[0].sampleAspectRatio.y;

            if (probe_results.videoColorMasteringMetadata.valid) {
                videoProgram.settings.eotf = probe_results.videoColorMasteringMetadata.eotf;
                memcpy(&videoProgram.settings.masteringDisplayColorVolume, &probe_results.videoColorMasteringMetadata.masteringDisplayColorVolume, sizeof(videoProgram.settings.masteringDisplayColorVolume));
                /* NOTE: contentLightLevel unused */
                BDBG_LOG(("~ eotf: %u ~", probe_results.videoColorMasteringMetadata.eotf));
                BDBG_LOG(("~ Luminance: max: %u, min: %u ~",
                    probe_results.videoColorMasteringMetadata.masteringDisplayColorVolume.luminance.max,
                    probe_results.videoColorMasteringMetadata.masteringDisplayColorVolume.luminance.min));
                BDBG_LOG(("~ Luminance: x: %d, y: %d ~",
                    probe_results.videoColorMasteringMetadata.masteringDisplayColorVolume.whitePoint.x,
                    probe_results.videoColorMasteringMetadata.masteringDisplayColorVolume.whitePoint.y));
            }

            /* outside of videoProgram */
            player->colorDepth = probe_results.video[0].colorDepth;
        }

        if (!videoProgram.settings.pidChannel
#if NEXUS_HAS_AUDIO
            && !player->audioProgram.primary.pidChannel
#endif
            ) {
            BDBG_WRN(("no content found for program %d", psettings->program));
            goto error;
        }
        videoProgram.smoothResolutionChange = psettings->smoothResolutionChange;
        videoProgram.settings.crcMode = psettings->crcMode;
        videoProgram.settings.timestampMode = psettings->video.timestampMode;
        videoProgram.settings.progressiveOverrideMode = psettings->video.progressiveOverrideMode;
        videoProgram.settings.frameRate = psettings->video.frameRate;
        player->videoProgram = videoProgram;

        if (player->start_settings.decrypt.algo == NEXUS_SecurityAlgorithm_eMax) {
            unsigned i;
            for (i=0;i<probe_results.num_other;i++) {
                if (probe_results.other[i].pid >= NXAPPS_DVR_CRYPTO_TAG_PID_BASE) {
                    player->start_settings.decrypt.algo = probe_results.other[i].pid - NXAPPS_DVR_CRYPTO_TAG_PID_BASE;
                    BDBG_WRN(("using other[%d].pid %#x to enable %s decrypt", i, probe_results.other[i].pid,
                        lookup_name(g_securityAlgoStrs, player->start_settings.decrypt.algo)));
                }
            }
        }
    }

    player->videoProgram.displayEnabled = psettings->videoWindowType != NxClient_VideoWindowType_eNone;

    if (media_player_p_test_disconnect(player->master?player->master:player, &player->videoProgram)) {
        media_player_p_disconnect(player);
    }
    if (!player->connectId) {
        rc = media_player_p_connect(player);
        if (rc) { rc = BERR_TRACE(rc); goto error; }
    }

    /* crypto */
    if (player->start_settings.decrypt.algo < NEXUS_SecurityAlgorithm_eMax) {
        struct dvr_crypto_settings settings;
        unsigned total = 0;
        dvr_crypto_get_default_settings(&settings);
        settings.algo = player->start_settings.decrypt.algo;
        if (player->videoProgram.settings.enhancementPidChannel) {
            settings.pid[total++] = player->videoProgram.settings.enhancementPidChannel;
        }
        if (player->videoProgram.settings.pidChannel) {
            settings.pid[total++] = player->videoProgram.settings.pidChannel;
        }
#if NEXUS_HAS_AUDIO
        if (player->audioProgram.primary.pidChannel) {
            settings.pid[total++] = player->audioProgram.primary.pidChannel;
        }
#endif
        player->crypto = dvr_crypto_create(&settings);
        /* if it fails, keep going */
    }

#if NEXUS_HAS_AUDIO
    /* apply codec settings */
    {
        NEXUS_AudioDecoderCodecSettings settings;
        NxClient_AudioProcessingSettings audioProcessingSettings;

        NxClient_GetAudioProcessingSettings(&audioProcessingSettings);
        NEXUS_SimpleAudioDecoder_GetCodecSettings(player->audioDecoder, NEXUS_SimpleAudioDecoderSelector_ePrimary, player->audioProgram.primary.codec, &settings);

        switch (player->audioProgram.primary.codec) {
        case NEXUS_AudioCodec_eAc3:
            audioProcessingSettings.dolby.ddre.externalPcmMode = false;
            if (player->start_settings.audio.dolbyDrcMode < NEXUS_AudioDecoderDolbyDrcMode_eMax) {
                settings.codecSettings.ac3.drcMode = settings.codecSettings.ac3.drcModeDownmix = player->start_settings.audio.dolbyDrcMode;
            }
            break;
        case NEXUS_AudioCodec_eAc3Plus:
            audioProcessingSettings.dolby.ddre.externalPcmMode = false;
            if (player->start_settings.audio.dolbyDrcMode < NEXUS_AudioDecoderDolbyDrcMode_eMax) {
                settings.codecSettings.ac3Plus.drcMode = settings.codecSettings.ac3Plus.drcModeDownmix = player->start_settings.audio.dolbyDrcMode;
            }
            break;
        case NEXUS_AudioCodec_eAc4:
            audioProcessingSettings.dolby.ddre.externalPcmMode = false;
            player->audioProgram.primary.mixingMode = NEXUS_AudioDecoderMixingMode_eStandalone;
            break;
            /* only line and rf applies for aac/aacplus, but nexus can validate params */
        case NEXUS_AudioCodec_eAacAdts:
        case NEXUS_AudioCodec_eAacLoas:
            audioProcessingSettings.dolby.ddre.externalPcmMode = false;
            if (player->start_settings.audio.dolbyDrcMode < NEXUS_AudioDecoderDolbyDrcMode_eMax) {
                settings.codecSettings.aac.drcMode = (NEXUS_AudioDecoderDolbyPulseDrcMode)player->start_settings.audio.dolbyDrcMode;
            }
            break;
        case NEXUS_AudioCodec_eAacPlusAdts:
        case NEXUS_AudioCodec_eAacPlusLoas:
            audioProcessingSettings.dolby.ddre.externalPcmMode = false;
            if (player->start_settings.audio.dolbyDrcMode < NEXUS_AudioDecoderDolbyDrcMode_eMax) {
                settings.codecSettings.aacPlus.drcMode = (NEXUS_AudioDecoderDolbyPulseDrcMode)player->start_settings.audio.dolbyDrcMode;
            }
            break;
        default:
            audioProcessingSettings.dolby.ddre.externalPcmMode = true;
            if (player->start_settings.audio.dolbyDrcMode < NEXUS_AudioDecoderDolbyDrcMode_eMax) {
                audioProcessingSettings.dolby.ddre.drcMode = audioProcessingSettings.dolby.ddre.drcModeDownmix = player->start_settings.audio.dolbyDrcMode;
            }
            else {
                audioProcessingSettings.dolby.ddre.drcMode = NEXUS_AudioDecoderDolbyDrcMode_eLine;
                audioProcessingSettings.dolby.ddre.drcModeDownmix = NEXUS_AudioDecoderDolbyDrcMode_eRf;
            }
            break;
        }
        NEXUS_SimpleAudioDecoder_SetCodecSettings(player->audioDecoder, NEXUS_SimpleAudioDecoderSelector_ePrimary, &settings);
        NxClient_SetAudioProcessingSettings(&audioProcessingSettings);

    }

    /* special start settings for persistent decode */
    if (player->create_settings.audio.usePersistent) {
        player->audioProgram.primary.mixingMode = player->start_settings.audio.mixingMode;
        player->audioProgram.master = player->start_settings.audio.master;
    }
#endif

    /* set StcChannel to all decoders before starting any */
    if (player->videoProgram.settings.pidChannel && player->stcChannel) {
        rc = NEXUS_SimpleVideoDecoder_SetStcChannel(player->videoDecoder, player->stcChannel);
        if (rc) { rc = BERR_TRACE(rc); goto error; }
    }
#if NEXUS_HAS_AUDIO
    if (player->audioProgram.primary.pidChannel && player->stcChannel) {
        rc = NEXUS_SimpleAudioDecoder_SetStcChannel(player->audioDecoder, player->stcChannel);
        if (rc) { rc = BERR_TRACE(rc); goto error; }
    }
#endif
    if (player->videoProgram.settings.pidChannel) {
        rc = NEXUS_SimpleVideoDecoder_Start(player->videoDecoder, &player->videoProgram);
        if (rc) { rc = BERR_TRACE(rc); goto error; }
    }
#if NEXUS_HAS_AUDIO
    if (player->audioProgram.primary.pidChannel) {
        player->audioProgram.primer.pcm =
        player->audioProgram.primer.compressed = player->start_settings.audio_primers;
        rc = NEXUS_SimpleAudioDecoder_Start(player->audioDecoder, &player->audioProgram);

        if (rc) { rc = BERR_TRACE(rc); goto error; }
        /* decode may fail if audio codec not supported */
    }
#endif

    if (player->ipActive) {
        if(player->usePbip) {
            media_player_ip_start_playback(player->ip);
            if (rc) { rc = BERR_TRACE(rc); goto error; }
        }
        else
        {
            media_player_bip_start_play(player->bip);
            if (rc) { rc = BERR_TRACE(rc); goto error; }
        }
    }
    else {
        BDBG_MSG(("starting nexus playback"));
        rc = NEXUS_Playback_Start(player->playback, player->file, NULL);
        if (rc) {
            /* try one more time without the index */
            if (psettings->index_url) {
                NEXUS_FilePlay_Close(player->file);
                player->file = NEXUS_FilePlay_OpenPosix(url.path, NULL);
                if (player->file) {
                    rc = NEXUS_Playback_Start(player->playback, player->file, NULL);
               }
            }
            if (rc) {
                rc = BERR_TRACE(rc);
                goto error;
            }
        }
    }
    return 0;

error:
    media_player_stop(player);
    return -1;
}

void media_player_stop(media_player_t player)
{
    BDBG_OBJECT_ASSERT(player, media_player);

    if (!player->started) return;

    if (player->videoDecoder) {
        NEXUS_SimpleVideoDecoder_Stop(player->videoDecoder);
        NEXUS_SimpleVideoDecoder_SetStcChannel(player->videoDecoder, NULL);
    }
    if (player->audioDecoder) {
        NEXUS_SimpleAudioDecoder_Stop(player->audioDecoder);
        NEXUS_SimpleAudioDecoder_SetStcChannel(player->audioDecoder, NULL);
    }
    if (player->crypto) {
        dvr_crypto_destroy(player->crypto);
    }

    if (player->ipActive) {
        if(player->usePbip) {
            media_player_ip_stop(player->ip);
        }
        else {
            media_player_bip_stop(player->bip);
        }

        /* Reset cached data.*/
        if (player->videoProgram.settings.enhancementPidChannel)
        {
            player->videoProgram.settings.enhancementPidChannel = NULL;
        }
        if (player->videoProgram.settings.pidChannel)
        {
            player->videoProgram.settings.pidChannel = NULL;
        }
#if NEXUS_HAS_AUDIO
        if (player->audioProgram.primary.pidChannel)
        {
            player->audioProgram.primary.pidChannel = NULL;
        }
#endif
        if(player->colorDepth)
        {
            player->colorDepth = 0;
        }
        player->ipActive = false;
    }
    else {
        /* regular file playback case */
        if (player->playback) {
            NEXUS_Playback_Stop(player->playback);
            if (player->videoProgram.settings.enhancementPidChannel) {
                NEXUS_Playback_ClosePidChannel(player->playback, player->videoProgram.settings.enhancementPidChannel);
                player->videoProgram.settings.enhancementPidChannel = NULL;
            }
            if (player->videoProgram.settings.pidChannel) {
                NEXUS_Playback_ClosePidChannel(player->playback, player->videoProgram.settings.pidChannel);
                player->videoProgram.settings.pidChannel = NULL;
            }
#if NEXUS_HAS_AUDIO
            if (player->audioProgram.primary.pidChannel) {
                NEXUS_Playback_ClosePidChannel(player->playback, player->audioProgram.primary.pidChannel);
                player->audioProgram.primary.pidChannel = NULL;
            }
#endif
            if (player->pcrPidChannel) {
                NEXUS_Playback_ClosePidChannel(player->playback, player->pcrPidChannel);
                player->pcrPidChannel = NULL;
            }
        }
        if (player->file) {
            NEXUS_FilePlay_Close(player->file);
        }
    }
    if (player->start_settings.forceStopDisconnect) {
        media_player_p_disconnect(player);
    }
    player->started = false;
}

int media_player_ac4_status( media_player_t player, int action )
{
#if NEXUS_HAS_AUDIO
    NEXUS_AudioDecoderStatus audStatus;
    NEXUS_AudioDecoderCodecSettings codecSettings;

    NEXUS_SimpleAudioDecoder_GetStatus(player->audioDecoder, &audStatus);
    NEXUS_SimpleAudioDecoder_GetCodecSettings(player->audioDecoder, NEXUS_SimpleAudioDecoderSelector_ePrimary, audStatus.codec, &codecSettings);

    if (action == 1) {
        BDBG_ERR(("Codec is %lu", (unsigned long)audStatus.codec));
        if ( audStatus.codec == NEXUS_AudioCodec_eAc4 ) {
            unsigned i;
            BDBG_ERR(("The current presentation index is %lu", (unsigned long)audStatus.codecStatus.ac4.currentPresentationIndex));
            BDBG_ERR(("numPresentations %lu", (unsigned long)audStatus.codecStatus.ac4.numPresentations));
            for (i = 0; i<audStatus.codecStatus.ac4.numPresentations; i++) {
                NEXUS_AudioDecoderPresentationStatus presentStatus;
                NEXUS_SimpleAudioDecoder_GetPresentationStatus(player->audioDecoder, i, &presentStatus);
                if ( presentStatus.codec != audStatus.codec ) {
                    BDBG_ERR(("Something went wrong. Presentation Status Codec doesn't match the current decode codec."));
                }
                else {
                    BDBG_ERR(("Presentation %lu index: %lu", (unsigned long)i, (unsigned long)presentStatus.status.ac4.index));
                    BDBG_ERR(("  Presentation %lu id: %s", (unsigned long)i, presentStatus.status.ac4.id));
                    BDBG_ERR(("  Presentation %lu name: %s", (unsigned long)i, presentStatus.status.ac4.name));
                    BDBG_ERR(("  Presentation %lu language: %s", (unsigned long)i, presentStatus.status.ac4.language));
                    BDBG_ERR(("  Presentation %lu associateType: %lu", (unsigned long)i, (unsigned long)presentStatus.status.ac4.associateType));
                }
            }
            BDBG_ERR(("Dialog Enhancer Max %lu", (unsigned long)audStatus.codecStatus.ac4.dialogEnhanceMax));
        }
    }
    else if (action == 2) {
        BDBG_ERR(("Codec is %lu", (unsigned long)audStatus.codec));
        if ( audStatus.codec == NEXUS_AudioCodec_eAc4 ) {
            codecSettings.codecSettings.ac4.programs[0].presentationIndex = audStatus.codecStatus.ac4.currentPresentationIndex;
            BDBG_ERR(("The currentPresentation index is %lu. Moving to the next presentation", (unsigned long)(codecSettings.codecSettings.ac4.programs[0].presentationIndex)));
            if ( codecSettings.codecSettings.ac4.programs[0].presentationIndex >= audStatus.codecStatus.ac4.numPresentations ) {
                codecSettings.codecSettings.ac4.programs[0].presentationIndex = 0;
            }
            codecSettings.codecSettings.ac4.programs[0].presentationIndex += 1;
            if ( codecSettings.codecSettings.ac4.programs[0].presentationIndex >= audStatus.codecStatus.ac4.numPresentations ) {
                codecSettings.codecSettings.ac4.programs[0].presentationIndex = 0;
            }
            NEXUS_SimpleAudioDecoder_SetCodecSettings(player->audioDecoder,NEXUS_SimpleAudioDecoderSelector_ePrimary,&codecSettings);
        }
    }
    else if (action ==3) {
        BDBG_ERR(("Codec is %lu", (unsigned long)audStatus.codec));
        if ( audStatus.codec == NEXUS_AudioCodec_eAc4 ) {
            BDBG_ERR(("The current dialog enhancement level is %ld. Incrementing it by 3", (unsigned long)(codecSettings.codecSettings.ac4.dialogEnhancerAmount)));
            codecSettings.codecSettings.ac4.dialogEnhancerAmount = codecSettings.codecSettings.ac4.dialogEnhancerAmount + 3;
            if ( codecSettings.codecSettings.ac4.dialogEnhancerAmount > 12 ) {
                codecSettings.codecSettings.ac4.dialogEnhancerAmount = 12;
            }
            NEXUS_SimpleAudioDecoder_SetCodecSettings(player->audioDecoder,NEXUS_SimpleAudioDecoderSelector_ePrimary, &codecSettings);
        }
    }
    else if (action ==4) {
        BDBG_ERR(("Codec is %lu", (unsigned long)audStatus.codec));
        if ( audStatus.codec == NEXUS_AudioCodec_eAc4 ) {
            BDBG_ERR(("The current dialog enhancement level is %ld. decrementing it by 3", (unsigned long)(codecSettings.codecSettings.ac4.dialogEnhancerAmount)));
            codecSettings.codecSettings.ac4.dialogEnhancerAmount = codecSettings.codecSettings.ac4.dialogEnhancerAmount - 3;
            if ( codecSettings.codecSettings.ac4.dialogEnhancerAmount < -12 ) {
                codecSettings.codecSettings.ac4.dialogEnhancerAmount = -12;
            }
            NEXUS_SimpleAudioDecoder_SetCodecSettings(player->audioDecoder,NEXUS_SimpleAudioDecoderSelector_ePrimary, &codecSettings);
        }
    }
#endif
      return 0;
}

int media_player_switch_audio(media_player_t player)
{
    int rc;
    if (!player->master || !player->audio.allocResults.simpleAudioDecoder.id) {
        return BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }
    if (!player->create_settings.audio.usePersistent) {
        if (player->audio.connectId) {
            rc = NxClient_RefreshConnect(player->audio.connectId);
            if (rc) return BERR_TRACE(rc);
        }
        else {
            NxClient_ConnectSettings audioConnectSettings;
            NxClient_GetDefaultConnectSettings(&audioConnectSettings);
            audioConnectSettings.simpleAudioDecoder.id = player->audio.allocResults.simpleAudioDecoder.id;
            rc = NxClient_Connect(&audioConnectSettings, &player->audio.connectId);
            if (rc) return BERR_TRACE(rc);
        }
    }
    else {
        if (player->audio.connectId)
        {
            if (player->master && player->master->audio.persistentMasterConnectId != 0)
            {
                NxClient_ReconfigSettings reconfig;
                NxClient_GetDefaultReconfigSettings(&reconfig);
                reconfig.command[0].connectId1 = player->audio.connectId;
                reconfig.command[0].connectId2 = player->master->audio.persistentMasterConnectId;
                reconfig.command[0].type = NxClient_ReconfigType_eRerouteAudio;
                rc = NxClient_Reconfig(&reconfig);
                if (rc) return BERR_TRACE(rc);
                player->master->audio.persistentMasterConnectId = player->audio.connectId;

            }
        }
        else {
            rc = media_player_p_connect_persistents(player);
            if (rc) return BERR_TRACE(rc);
        }
    }
    return 0;
}

void media_player_destroy(media_player_t player)
{
    bool uninit;
    BDBG_OBJECT_ASSERT(player, media_player);
    uninit = !player->master || !BLST_Q_FIRST(&player->master->players);
    if (player->started) {
        media_player_stop(player);
        media_player_p_disconnect(player);
    }
    if (player->playback) {
        NEXUS_Playback_Destroy(player->playback);
    }
    if (player->playpump) {
        NEXUS_Playpump_Close(player->playpump);
    }
    if (player->videoDecoder) {
        NEXUS_SimpleVideoDecoder_Release(player->videoDecoder);
    }
    if (player->audioDecoder) {
        NEXUS_SimpleAudioDecoder_Release(player->audioDecoder);
    }
    if (player->stcChannel) {
        NEXUS_SimpleStcChannel_Destroy(player->stcChannel);
    }
    if (player->ip) {
        if(player->usePbip) {
            media_player_ip_destroy(player->ip);
        }
        else{
            media_player_bip_destroy(player->bip);
        }
    }
    if(player->settings.audio.language) {
        BKNI_Free(player->settings.audio.language);
    }
    if (uninit) {
        NxClient_Free(&player->allocResults);
    }
    BDBG_OBJECT_DESTROY(player, media_player);
    BKNI_Free(player);
    if (uninit) {
        NxClient_Uninit();
    }
}

static bool index_has_rai(const char *indexfile)
{
    bool result = false;
    FILE *f;
    f = fopen(indexfile, "rb");
    if (f) {
        BNAV_AVC_Entry nav;
        if (fread(&nav, sizeof(nav), 1, f) == 1) {
            result = BNAV_get_RandomAccessIndicator(&nav);
        }
        fclose(f);
    }
    return result;
}

int media_player_trick( media_player_t player, int rate)
{
    enum { smooth_rewind_mpeg, smooth_rewind, by_rate } rewind = by_rate;
    NEXUS_PlaybackTrickModeSettings trickMode;
    int rc;

    BDBG_OBJECT_ASSERT(player, media_player);
    if (!player->started) {
        return BERR_TRACE(NEXUS_NOT_AVAILABLE);
    }
    if (player->ipActive) {
        if(player->usePbip) {
            return media_ip_player_trick(player->ip, rate);
        }
        else {
            return media_player_bip_trick(player->bip, rate);
        }
    }

    if (player->start_settings.index_url) {
        NEXUS_PlaybackSettings playbackSettings;
        NEXUS_Playback_GetSettings(player->playback, &playbackSettings);
        if (playbackSettings.playpumpSettings.transportType == NEXUS_TransportType_eTs) {
            switch (player->videoProgram.settings.codec) {
            case NEXUS_VideoCodec_eMpeg2:
                if (rate == -1000) {
                    rewind = smooth_rewind_mpeg;
                }
                else if (rate == -2000) {
                    rewind = smooth_rewind;
                }
                break;
            case NEXUS_VideoCodec_eH264:
            case NEXUS_VideoCodec_eH265:
                if (rate == -1000 && index_has_rai(player->start_settings.index_url)) {
                    rewind = smooth_rewind;
                }
                break;
            default:
                break;
            }
        }
    }

    NEXUS_Playback_GetDefaultTrickModeSettings(&trickMode);

    if (rate == NEXUS_NORMAL_DECODE_RATE) {
        rc = NEXUS_Playback_Play(player->playback);
    }
    else if (rate == 0) {
        trickMode.rate = 0;
        rc = NEXUS_Playback_Pause(player->playback);
    }
    else {
        if (rewind != by_rate) {
            trickMode.skipControl = NEXUS_PlaybackSkipControl_eHost;
            trickMode.rateControl = NEXUS_PlaybackRateControl_eDecoder;
            trickMode.rate = NEXUS_NORMAL_DECODE_RATE;

            /* For MPEG TS with index, do BRCM trick, MDqtIP, then I frame.
            For AVC/HEVC TS with index, do MDqtIP, then I frame. */
            if (rewind == smooth_rewind_mpeg) {
                trickMode.mode = NEXUS_PlaybackHostTrickMode_ePlayBrcm;
                trickMode.mode_modifier = -1;
            }
            else {
                trickMode.mode = NEXUS_PlaybackHostTrickMode_ePlayMultiPassDqtIP;
                trickMode.mode_modifier = -1;
            }
        }
        else {
            trickMode.rate = rate;
        }
        rc = NEXUS_Playback_TrickMode(player->playback, &trickMode);
    }
    if (!rc) {
        player->trickMode = trickMode;
    }
    return rc;
}

void media_player_get_trick_mode( media_player_t player, NEXUS_PlaybackTrickModeSettings *trickMode )
{
    BDBG_OBJECT_ASSERT(player, media_player);
    *trickMode = player->trickMode;
}

int media_player_frame_advance( media_player_t player, bool forward )
{
    int rc;
    BDBG_OBJECT_ASSERT(player, media_player);
    if (!player->started) {
        return BERR_TRACE(NEXUS_NOT_AVAILABLE);
    }
    if (player->ipActive) {
        if(player->usePbip) return BERR_TRACE(NEXUS_NOT_AVAILABLE);

        return media_player_bip_frame_advance(player->bip, forward);
    }
    if (player->start_settings.stcTrick) {
        unsigned i;
        /* For stc trick, the decoder must see the frame, then we can advance.
        A single FrameAdvance doesn't work for interlaced, so we repeat until a picture comes out.
        This produces a double-frame of frame advance, but it's reliable.
        If numDisplayed could be 1 for the first picture displayed, we could avoid it. */
        for (i=0;i<10;i++) {
            uint32_t next_pts;
            rc = NEXUS_SimpleVideoDecoder_GetNextPts(player->videoDecoder, &next_pts);
            if (!rc) break;
            BKNI_Sleep(10);
        }
        for (i=0;i<20;i++) {
            NEXUS_VideoDecoderStatus status;
            rc = NEXUS_Playback_FrameAdvance(player->playback, forward);
            if (rc) return BERR_TRACE(rc);
            BKNI_Sleep(17);
            rc = NEXUS_SimpleVideoDecoder_GetStatus(player->videoDecoder, &status);
            if (!rc && status.numDisplayed) break;
        }
    }
    else {
        rc = NEXUS_Playback_FrameAdvance(player->playback, forward);
        if (rc) return BERR_TRACE(rc);
    }
    return 0;
}

int media_player_get_playback_status( media_player_t player, NEXUS_PlaybackStatus *pstatus )
{
    if (player->ipActive) {
        if(player->usePbip) {
            return media_player_ip_get_playback_status(player->ip, pstatus);
        }
        else {
            return media_player_bip_status(player->bip, pstatus);
        }
    }
    return NEXUS_Playback_GetStatus(player->playback, pstatus);
}

NEXUS_SimpleVideoDecoderHandle media_player_get_video_decoder(media_player_t player)
{
    return player->videoDecoder;
}

NEXUS_SimpleAudioDecoderHandle media_player_get_audio_decoder(media_player_t player)
{
    return player->audioDecoder;
}

int media_player_seek( media_player_t player, int offset, int whence )
{
    NEXUS_PlaybackStatus status;
    int rc;
    if (player->ipActive) {
        if(player->usePbip) {
            return media_ip_player_seek(player->ip, offset, whence);
        }
        else {
            return media_player_bip_seek(player->bip, offset, whence);
        }
    }
    rc = NEXUS_Playback_GetStatus(player->playback, &status);
    if (rc) return BERR_TRACE(rc);
    switch (whence) {
    case SEEK_SET:
    default:
        break;
    case SEEK_CUR:
        offset += status.position;
        break;
    case SEEK_END:
        offset += status.last;
        break;
    }
#define TIMESHIFT_GAP 5000
    if (offset < 0 || (unsigned)offset < status.first) {
        offset = status.first;
    }
#if 0
    else if (player->start_settings.loopMode == NEXUS_PlaybackLoopMode_ePlay && status.last > TIMESHIFT_GAP && (unsigned)offset > status.last - TIMESHIFT_GAP) {
        offset = status.last - TIMESHIFT_GAP;
    }
#endif
    else if ((unsigned)offset > status.last) {
        offset = status.last;
    }
    {
        char timestr[16];
        unsigned hr = offset / 1000 / 60 / 60;
        unsigned min = (offset / 1000 / 60) % 60;
        unsigned sec = (offset / 1000) % 60;
        if (hr) {
            BKNI_Snprintf(timestr, sizeof(timestr), "%d:%02d:%02d", hr, min, sec);
        }
        else {
            BKNI_Snprintf(timestr, sizeof(timestr), "%d:%02d", min, sec);
        }
        BDBG_WRN(("seek %s", timestr));
    }
    return NEXUS_Playback_Seek(player->playback, offset);
}

#if B_REFSW_TR69C_SUPPORT
int media_player_get_set_tr69c_info(void *context, enum b_tr69c_type type, union b_tr69c_info *info)
{
    media_player_t player = context;
    int rc;
    switch (type)
    {
        case b_tr69c_type_get_playback_ip_status:
            if (player->ipActive) {
                rc = media_player_ip_get_set_tr69c_info(player->ip, type, info);
                if (rc) return BERR_TRACE(rc);
            }
            break;
        case b_tr69c_type_get_playback_status:
            rc = NEXUS_Playback_GetStatus(player->playback, &info->playback_status);
            if (rc) return BERR_TRACE(rc);
            break;
        case b_tr69c_type_get_video_decoder_status:
            rc = NEXUS_SimpleVideoDecoder_GetStatus(player->videoDecoder, &info->video_decoder_status);
            if (rc) return BERR_TRACE(rc);
            break;
        case b_tr69c_type_get_video_decoder_start_settings:
            info->video_start_settings.codec = player->videoProgram.settings.codec;
            info->video_start_settings.videoWindowType = player->start_settings.videoWindowType;
            break;
        case b_tr69c_type_get_audio_decoder_status:
            rc = NEXUS_SimpleAudioDecoder_GetStatus(player->audioDecoder, &info->audio_decoder_status);
            if (rc) return BERR_TRACE(rc);
            break;
        case b_tr69c_type_get_audio_decoder_settings:
            NEXUS_SimpleAudioDecoder_GetSettings(player->audioDecoder, &info->audio_decoder_settings);
            break;
        case b_tr69c_type_set_video_decoder_mute:
        {
            NEXUS_VideoDecoderSettings settings;
            NEXUS_SimpleVideoDecoder_GetSettings(player->videoDecoder, &settings);
            settings.mute = info->video_decoder_mute;
            rc = NEXUS_SimpleVideoDecoder_SetSettings(player->videoDecoder, &settings);
            if (rc) return BERR_TRACE(rc);
            break;
        }
        case b_tr69c_type_set_audio_decoder_mute:
        {
            NEXUS_SimpleAudioDecoderSettings settings;
            NEXUS_SimpleAudioDecoder_GetSettings(player->audioDecoder, &settings);
            settings.primary.muted = info->audio_decoder_mute;
            settings.secondary.muted = info->audio_decoder_mute;
            rc = NEXUS_SimpleAudioDecoder_SetSettings(player->audioDecoder, &settings);
            if (rc) return BERR_TRACE(rc);
            break;
        }
        default:
            return BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }
    return 0;
}
#endif
#else
typedef unsigned unused;
#endif
