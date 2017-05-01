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
 *
 * Module Description:
 *
 *****************************************************************************/
#include "nxserverlib_impl.h"

BDBG_MODULE(nxserverlib_audio);

#if NEXUS_HAS_AUDIO

#define BDBG_MSG_TRACE(X) /* BDBG_MSG(X) */

#include "nexus_audio_input.h"
#include "nexus_audio_output.h"
#include "nexus_audio_dummy_output.h"
#include "nexus_auto_volume_level.h"
#include "nexus_tru_volume.h"
#include "nexus_dolby_digital_reencode.h"
#include "nexus_dolby_volume.h"

enum nxserver_audio_decoder {
    nxserver_audio_decoder_primary,
    nxserver_audio_decoder_passthrough, /* the passthrough decoder. */
    nxserver_audio_decoder_description, /* the secondary decode for audio description. follows NEXUS_SimpleAudioDecoderServerSettings naming. */
    nxserver_audio_decoder_max
};

enum nxserver_audio_mixer {
    nxserver_audio_mixer_stereo,
    nxserver_audio_mixer_multichannel,
    nxserver_audio_mixer_persistent,
    nxserver_audio_mixer_max
};

enum b_audio_mode {
    b_audio_mode_playback,   /* no decode, but AudioPlayback possible. masterSimpleAudioDecoder has outputs. */
    b_audio_mode_decode,     /* client simpleAudioDecoder has outputs */
    b_audio_mode_transcode   /* transcode-only audio */
};

/* TODO: support hdmi -> simple audio decoder, which will allow for PCM mixing and transcode.
one option is if NxClient_Connect has simpleAudioDecoder.id set, then route through decoder. */

/**
for main audio decoder, all handles may be used.
for trancode audio decoders, only
audioDecoder[nxserver_audio_decoder_primary],
audioDecoder[nxserver_audio_decoder_description] and audioEncoder are used.
**/

struct b_audio_playback_resource {
    BLST_Q_ENTRY(b_audio_playback_resource) link;
    /* either I2sInput or AudioPlayback, never both */
    NEXUS_I2sInputHandle i2sInput;
    NEXUS_AudioPlaybackHandle audioPlayback;
    struct b_connect *connect; /* if not NULL, this is in use. */
};

struct b_audio_resource {
    enum b_audio_mode mode;
    NEXUS_AudioDecoderHandle audioDecoder[nxserver_audio_decoder_max];
    NEXUS_AudioPlaybackHandle passthroughPlayback;
    unsigned dspIndex;
    NEXUS_AudioMixerHandle mixer[nxserver_audio_mixer_max];
    bool dspMixer[nxserver_audio_mixer_max];

    /* the following post-processing stages are optional, but created in this order, after mixer */
    NEXUS_AutoVolumeLevelHandle avl;
    NEXUS_TruVolumeHandle truVolume;
    NEXUS_DolbyVolume258Handle dolbyVolume258;
    NEXUS_DolbyDigitalReencodeHandle ddre;

    BLST_Q_HEAD(b_audio_playback_resource_list, b_audio_playback_resource) audioPlaybackList;
    NEXUS_AudioEncoderHandle audioEncoder;
    NEXUS_SimpleAudioDecoderHandle masterSimpleAudioDecoder; /* holds audio configuration in b_audio_mode_playback.
                                                                allows for universal SwapServerSettings for all config changes. */

    struct b_session *session;
    struct b_connect *connect;
    bool localSession;
};

bool b_audio_dolby_ms_enabled(enum nxserverlib_dolby_ms_type type)
{
    switch ( type )
    {
    case nxserverlib_dolby_ms_type_ms11:
    case nxserverlib_dolby_ms_type_ms12:
        return true;
        break; /* unreachable */
    default:
    case nxserverlib_dolby_ms_type_none:
    case nxserverlib_dolby_ms_type_max:
        break;
    }

    return false;
}

/* get SimpleAudioDecoder for this connect, active or not */
static NEXUS_SimpleAudioDecoderHandle b_audio_get_decoder(struct b_connect *connect)
{
    if (!connect) {
        return NULL;
    }
    BDBG_OBJECT_ASSERT(connect, b_connect);
    if (!connect->req[b_resource_simple_audio_decoder]) {
        return NULL;
    }
    else {
        return connect->req[b_resource_simple_audio_decoder]->handles.simpleAudioDecoder.handle;
    }
}

/* get SimpleAudioDecoder that has the outputs */
static NEXUS_SimpleAudioDecoderHandle b_audio_get_active_decoder(struct b_audio_resource *r)
{
    return r->connect ? b_audio_get_decoder(r->connect) : r->masterSimpleAudioDecoder;
}

static struct {
    struct b_audio_resource *r;
} g_decoders[NEXUS_NUM_AUDIO_DECODERS];

static int b_alloc_audio_index(struct b_audio_resource *r, unsigned *pIndex)
{
    unsigned i;
    NEXUS_AudioCapabilities audioCapabilities;
    NEXUS_GetAudioCapabilities(&audioCapabilities);
    for (i=0;i<audioCapabilities.numDecoders;i++) {
        if (!g_decoders[i].r) {
            g_decoders[i].r = r;
#if NEXUS_COMMON_PLATFORM_VERSION < NEXUS_PLATFORM_VERSION(14,2)
            *pIndex = i;
#else
            /* nxclient index doesn't have to match AudioDecoder index */
            *pIndex = NEXUS_ANY_ID;
#endif
            return 0;
        }
    }
    return -1;
}

static void b_dealloc_audio_index(struct b_audio_resource *r)
{
    unsigned i;
    NEXUS_AudioCapabilities audioCapabilities;
    NEXUS_GetAudioCapabilities(&audioCapabilities);

    for (i=0;i<audioCapabilities.numDecoders;i++) {
        if (g_decoders[i].r == r) {
            g_decoders[i].r = NULL;
        }
    }
}

static struct b_audio_resource *g_dummyOutputs[NEXUS_MAX_AUDIO_DUMMY_OUTPUTS];

static unsigned get_audio_playback_req_id(struct b_req *req, unsigned i) { return req->handles.simpleAudioPlayback[i].id; }

static void b_audio_close_pb(struct b_audio_resource *r, struct b_audio_playback_resource *pb)
{
    if (pb->audioPlayback) {
        NEXUS_AudioPlayback_Close(pb->audioPlayback);
    }
    if (pb->i2sInput) {
        NEXUS_I2sInput_Close(pb->i2sInput);
    }
    BLST_Q_REMOVE(&r->audioPlaybackList, pb, link);
    BKNI_Free(pb);
}

static NEXUS_AudioInputHandle b_audio_get_pb_output(struct b_audio_playback_resource *pb)
{
    if (pb->i2sInput) {
        return NEXUS_I2sInput_GetConnector(pb->i2sInput);
    }
    else {
        return NEXUS_AudioPlayback_GetConnector(pb->audioPlayback);
    }
}

static int b_audio_open_pb(struct b_audio_resource *r, bool i2s, unsigned index)
{
    struct b_audio_playback_resource *pb;
    int rc;
    nxserver_t server = r->session->server;

    pb = BKNI_Malloc(sizeof(*pb));
    if (!pb) return BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
    BKNI_Memset(pb, 0, sizeof(*pb));
    BLST_Q_INSERT_TAIL(&r->audioPlaybackList, pb, link);

    if (i2s) {
        pb->i2sInput = NEXUS_I2sInput_Open(index, NULL);
        if (!pb->i2sInput) { rc = BERR_TRACE(-1); goto error; }
    }
    else {
        NEXUS_AudioPlaybackOpenSettings openSettings;
        NEXUS_AudioPlayback_GetDefaultOpenSettings(&openSettings);
        openSettings.heap = server->settings.client.heap[NXCLIENT_FULL_HEAP]; /* eFull mapping */
        if (server->settings.audioPlayback.fifoSize) {
            openSettings.fifoSize = server->settings.audioPlayback.fifoSize;
        }
        pb->audioPlayback = NEXUS_AudioPlayback_Open(NEXUS_ANY_ID, &openSettings);
        if (!pb->audioPlayback) { rc = BERR_TRACE(-1); goto error; }
    }
    if ( r->mixer[nxserver_audio_mixer_persistent] )
    {
        rc = NEXUS_AudioMixer_AddInput(r->mixer[nxserver_audio_mixer_persistent], b_audio_get_pb_output(pb));
    }
    else
    {
        if (r->mixer[nxserver_audio_mixer_stereo]) {
            rc = NEXUS_AudioMixer_AddInput(r->mixer[nxserver_audio_mixer_stereo], b_audio_get_pb_output(pb));
            if (rc) { rc = BERR_TRACE(rc); goto error; }
        }
        if (r->mixer[nxserver_audio_mixer_multichannel]) {
            rc = NEXUS_AudioMixer_AddInput(r->mixer[nxserver_audio_mixer_multichannel], b_audio_get_pb_output(pb));
            if (rc) { rc = BERR_TRACE(rc); goto error; }
        }
    }
    return 0;

error:
    b_audio_close_pb(r, pb);
    return rc;
}

struct b_audio_playback_resource *b_audio_get_pb(struct b_audio_resource *r, bool i2s)
{
    struct b_audio_playback_resource *pb;

    /* find an unused one in this session */
    for (pb = BLST_Q_FIRST(&r->audioPlaybackList); pb; pb = BLST_Q_NEXT(pb, link)) {
        if (i2s == (pb->i2sInput != NULL)) {
            if (!pb->connect) return pb;
        }
    }
    return NULL;
}

void release_audio_playbacks(struct b_connect *connect)
{
    struct b_req *req = connect->req[b_resource_simple_audio_playback];
    unsigned i;
    for (i=0;i<NXCLIENT_MAX_IDS;i++) {
        NEXUS_SimpleAudioPlaybackServerSettings settings;
        unsigned index;

        if (!connect->settings.simpleAudioPlayback[i].id) break;

        if (get_req_index(req, get_audio_playback_req_id, connect->settings.simpleAudioPlayback[i].id, &index)) {
            BERR_TRACE(NEXUS_INVALID_PARAMETER);
            continue;
        }

        if (!req->handles.simpleAudioPlayback[index].r) continue;

        req->handles.simpleAudioPlayback[index].r->connect = NULL;

        NEXUS_SimpleAudioPlayback_GetServerSettings(connect->client->session->audio.server, req->handles.simpleAudioPlayback[index].handle, &settings);
        settings.playback = NULL;
        settings.i2sInput = NULL;
        (void)NEXUS_SimpleAudioPlayback_SetServerSettings(connect->client->session->audio.server, req->handles.simpleAudioPlayback[index].handle, &settings);
    }
}

int acquire_audio_playbacks(struct b_connect *connect)
{
    struct b_req *req = connect->req[b_resource_simple_audio_playback];
    unsigned i;
    struct b_audio_resource *main_audio = connect->client->session->main_audio;

    if (!connect->settings.simpleAudioPlayback[0].id) {
        /* no request */
        return 0;
    }

    if (!main_audio || !(main_audio->mixer[nxserver_audio_mixer_stereo] || main_audio->mixer[nxserver_audio_mixer_multichannel])) {
        return BERR_TRACE(NEXUS_NOT_AVAILABLE);
    }

    for (i=0;i<NXCLIENT_MAX_IDS;i++) {
        NEXUS_SimpleAudioPlaybackServerSettings settings;
        unsigned index;
        int rc;
        struct b_audio_playback_resource *pb;

        if (!connect->settings.simpleAudioPlayback[i].id) break;

        if (get_req_index(req, get_audio_playback_req_id, connect->settings.simpleAudioPlayback[i].id, &index)) {
            return BERR_TRACE(NEXUS_INVALID_PARAMETER);
        }
        pb = b_audio_get_pb(main_audio, connect->settings.simpleAudioPlayback[i].i2s.enabled);
        if (!pb) { return BERR_TRACE(NEXUS_NOT_AVAILABLE); }

        NEXUS_SimpleAudioPlayback_GetServerSettings(connect->client->session->audio.server, req->handles.simpleAudioPlayback[index].handle, &settings);
        settings.i2sInput = pb->i2sInput;
        settings.playback = pb->audioPlayback;
        rc = NEXUS_SimpleAudioPlayback_SetServerSettings(connect->client->session->audio.server, req->handles.simpleAudioPlayback[index].handle, &settings);
        if (rc) return BERR_TRACE(rc);

        req->handles.simpleAudioPlayback[index].r = pb;
        pb->connect = connect;
    }
    return 0;
}

static bool is_persistent_audio(struct b_connect *connect)
{
    return (connect->settings.simpleAudioDecoder.decoderCapabilities.type == NxClient_AudioDecoderType_ePersistent);
}

static bool is_standalone_audio(struct b_connect *connect)
{
    return (connect->settings.simpleAudioDecoder.decoderCapabilities.type == NxClient_AudioDecoderType_eStandalone);
}

static bool is_main_audio(struct b_connect *connect)
{
    return connect->settings.simpleAudioDecoder.id && !is_transcode_connect(connect) && !is_persistent_audio(connect) && !is_standalone_audio(connect);
}

bool has_audio(struct b_connect *connect)
{
    return connect->client->session->main_audio && connect->client->session->main_audio->connect == connect;
}

bool lacks_audio(struct b_connect *connect)
{
    return is_main_audio(connect) && (!connect->client->session->main_audio || connect != connect->client->session->main_audio->connect);
}

bool is_connected_to_a_mixer(NEXUS_AudioInputHandle input, struct b_audio_resource *r)
{
    bool connected = false;
    bool anyConnected = false;
    if (r->mixer[nxserver_audio_mixer_stereo]) {
        NEXUS_AudioInput_IsConnectedToInput(input,
                                            NEXUS_AudioMixer_GetConnector(r->mixer[nxserver_audio_mixer_stereo]),
                                            &connected);
        anyConnected |= connected;
    }
    if (r->mixer[nxserver_audio_mixer_multichannel]) {
        NEXUS_AudioInput_IsConnectedToInput(input,
                                            NEXUS_AudioMixer_GetConnector(r->mixer[nxserver_audio_mixer_multichannel]),
                                            &connected);
        anyConnected |= connected;
    }
    if (r->mixer[nxserver_audio_mixer_persistent]) {
        NEXUS_AudioInput_IsConnectedToInput(input,
                                            NEXUS_AudioMixer_GetConnector(r->mixer[nxserver_audio_mixer_persistent]),
                                            &connected);
        anyConnected |= connected;
    }
    return anyConnected;
}

static NEXUS_AudioInputHandle b_audio_get_pcm_input(struct b_audio_resource *r, NEXUS_AudioConnectorType type)
{
    /* filter graph is mixer[->avl][->truVolume][->dolbyVolume258][->ddre] */
    if (r->ddre) {
        switch (type)
        {
        default:
        case NEXUS_AudioConnectorType_eStereo:
            return NEXUS_DolbyDigitalReencode_GetConnector(r->ddre, NEXUS_AudioConnectorType_eStereo);
        case NEXUS_AudioConnectorType_eMultichannel:
            return NEXUS_DolbyDigitalReencode_GetConnector(r->ddre, NEXUS_AudioConnectorType_eMultichannel);
        }
    }
    else if (r->dolbyVolume258) {
        return NEXUS_DolbyVolume258_GetConnector(r->dolbyVolume258);
    }
    else if (r->truVolume && type == NEXUS_AudioConnectorType_eStereo) {
        return NEXUS_TruVolume_GetConnector(r->truVolume);
    }
    else if (r->avl && type == NEXUS_AudioConnectorType_eStereo) {
        return NEXUS_AutoVolumeLevel_GetConnector(r->avl);
    }
    else {
        switch (type)
        {
        default:
        case NEXUS_AudioConnectorType_eStereo:
            if (r->mixer[nxserver_audio_mixer_stereo]) {
                return NEXUS_AudioMixer_GetConnector(r->mixer[nxserver_audio_mixer_stereo]);
            }
            else {
                return NEXUS_AudioDecoder_GetConnector(r->audioDecoder[nxserver_audio_decoder_primary], NEXUS_AudioConnectorType_eStereo);
            }
        case NEXUS_AudioConnectorType_eMultichannel:
            if (r->mixer[nxserver_audio_mixer_multichannel]) {
                return NEXUS_AudioMixer_GetConnector(r->mixer[nxserver_audio_mixer_multichannel]);
            }
            else {
                return NEXUS_AudioDecoder_GetConnector(r->audioDecoder[nxserver_audio_decoder_primary], NEXUS_AudioConnectorType_eMultichannel);
            }
        }
    }
}

static NEXUS_AudioInputHandle b_audio_get_compressed_input(struct b_audio_resource *r, NEXUS_AudioConnectorType type)
{
    /* filter graph is mixer[->avl][->truVolume][->dolbyVolume258][->ddre] */
    if (r->ddre) {
        switch (type)
        {
        default:
        case NEXUS_AudioConnectorType_eCompressed:
            return NEXUS_DolbyDigitalReencode_GetConnector(r->ddre, NEXUS_AudioConnectorType_eCompressed);
        case NEXUS_AudioConnectorType_eCompressed4x:
            return NEXUS_DolbyDigitalReencode_GetConnector(r->ddre, NEXUS_AudioConnectorType_eCompressed4x);
        }
    }
    else {
        switch (type)
        {
        case NEXUS_AudioConnectorType_eCompressed:
            return NEXUS_AudioDecoder_GetConnector(r->audioDecoder[nxserver_audio_decoder_primary], NEXUS_AudioConnectorType_eCompressed);
        default:
        case NEXUS_AudioConnectorType_eCompressed4x:
            return NEXUS_AudioDecoder_GetConnector(r->audioDecoder[nxserver_audio_decoder_passthrough], NEXUS_AudioConnectorType_eCompressed);
        }
    }
}

int nxserverlib_p_session_has_sd_audio(struct b_session *session)
{
    struct nxserver_session_settings *session_settings;

    session_settings = session->server->settings.session;
    if (session->index == 0)
        return (session_settings[1].output.sd == 0);
    else
        return (session_settings[session->index].output.sd != 0);
}

struct b_audio_resource *audio_decoder_create(struct b_session *session, enum b_audio_decoder_type type)
{
    unsigned i;
    int rc;
    NEXUS_AudioDecoderOpenSettings audioOpenSettings;
    NEXUS_AudioMixerSettings mixerSettings;
    struct b_audio_resource *r;
    unsigned index;
    nxserver_t server = session->server;
    bool localSession = false;
    NEXUS_AudioCapabilities cap;

    NEXUS_GetAudioCapabilities(&cap);
    if (type == b_audio_decoder_type_regular || type == b_audio_decoder_type_persistent) {
        localSession = IS_SESSION_DISPLAY_ENABLED(server->settings.session[session->index]);
    }

    r = BKNI_Malloc(sizeof(*r));
    if (!r) {
        BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
        return NULL;
    }
    memset(r, 0, sizeof(*r));
    r->session = session;
    r->localSession = localSession;
    BDBG_MSG(("create %p", (void*)r));

    NEXUS_AudioDecoder_GetDefaultOpenSettings(&audioOpenSettings);
    if (server->settings.audioDecoder.fifoSize) {
        audioOpenSettings.fifoSize = server->settings.audioDecoder.fifoSize;
    }
    if (type == b_audio_decoder_type_regular) {
        r->mode = b_audio_mode_playback;
        if (b_alloc_audio_index(r, &index)) {
            rc = BERR_TRACE(NEXUS_UNKNOWN);
            goto error;
        }
        audioOpenSettings.dspIndex = 0;
        audioOpenSettings.cdbHeap = server->settings.client.heap[NXCLIENT_VIDEO_SECURE_HEAP];
        if (server->settings.svp != nxserverlib_svp_type_none) {
            if (!audioOpenSettings.cdbHeap) {
                rc = BERR_TRACE(NEXUS_NOT_AVAILABLE);
                goto error;
            }
        }
        audioOpenSettings.karaokeSupported = server->settings.session[session->index].karaoke;
        audioOpenSettings.multichannelFormat = NEXUS_AudioMultichannelFormat_e5_1;
        r->audioDecoder[nxserver_audio_decoder_primary] = NEXUS_AudioDecoder_Open(index, &audioOpenSettings);
        if (!r->audioDecoder[nxserver_audio_decoder_primary]) {
            rc = BERR_TRACE(NEXUS_UNKNOWN);
            goto error;
        }

        BDBG_MSG(("open AudioDecoder[%u](%p) for %p", nxserver_audio_decoder_primary, (void*)r->audioDecoder[nxserver_audio_decoder_primary], (void*)r));
        if (server->settings.audioDecoder.enablePassthroughBuffer) {
            r->passthroughPlayback = NEXUS_AudioPlayback_Open(NEXUS_ANY_ID, NULL);
            if (!r->passthroughPlayback) {
                rc = BERR_TRACE(NEXUS_UNKNOWN);
                goto error;
            }
        }

        if (r->localSession) {
            if (!b_alloc_audio_index(r, &index)) {
                r->audioDecoder[nxserver_audio_decoder_passthrough] = NEXUS_AudioDecoder_Open(index, &audioOpenSettings);
            }
            if (!r->audioDecoder[nxserver_audio_decoder_passthrough]) {
                /* allow this to fail */
                BDBG_WRN(("unable to open pass-through audio decoder"));
            }
            else {
                BDBG_MSG(("open AudioDecoder[%u](%p) for %p", nxserver_audio_decoder_passthrough, (void*)r->audioDecoder[nxserver_audio_decoder_passthrough], (void*)r));
            }

            if (session->index == 0 && (server->settings.audioDecoder.audioDescription || server->settings.session[session->index].karaoke)) {
                if (!b_alloc_audio_index(r, &index)) {
                    audioOpenSettings.type = NEXUS_AudioDecoderType_eAudioDescriptor;
                    r->audioDecoder[nxserver_audio_decoder_description] = NEXUS_AudioDecoder_Open(index, &audioOpenSettings);
                }
                if (!r->audioDecoder[nxserver_audio_decoder_description]) {
                    /* allow this to fail */
                    BDBG_WRN(("unable to open audio description decoder"));
                }
                else {
                    BDBG_MSG(("open AudioDecoder[%u](%p) for %p (audio description)", nxserver_audio_decoder_description, (void*)r->audioDecoder[nxserver_audio_decoder_description], (void*)r));
                }
            }
        }

        /* Open mixer to mix the description and primary audio */
        NEXUS_AudioMixer_GetDefaultSettings(&mixerSettings);
        mixerSettings.mixUsingDsp = session->server->settings.session[session->index].output.encode ||
                                    server->settings.session[r->session->index].avl ||
                                    server->settings.session[r->session->index].truVolume ||
                                    server->settings.audioDecoder.audioDescription ||
                                    server->settings.session[r->session->index].persistentDecoderSupport ||
                                    b_audio_dolby_ms_enabled(server->settings.session[r->session->index].dolbyMs);
        mixerSettings.outputSampleRate = (server->settings.audioMixer.sampleRate >= 32000 && server->settings.audioMixer.sampleRate <= 96000) ? server->settings.audioMixer.sampleRate : mixerSettings.outputSampleRate;
        if (mixerSettings.mixUsingDsp) {
            mixerSettings.dspIndex = audioOpenSettings.dspIndex; /* mixer dspIndex must match decoder */
            if (b_audio_dolby_ms_enabled(server->settings.session[r->session->index].dolbyMs))
            {
                mixerSettings.outputSampleRate = 48000;
            }
        }

        /* If we are doing Dolby MS then we need a multichannel DSP mixer otherwise
           if a dsp mixer is required it should be used for stereo */
        r->dspMixer[nxserver_audio_mixer_multichannel] = false;
        r->dspMixer[nxserver_audio_mixer_stereo] = false;
        if (mixerSettings.mixUsingDsp && b_audio_dolby_ms_enabled(server->settings.session[r->session->index].dolbyMs))
        {
            r->dspMixer[nxserver_audio_mixer_multichannel] = true;
        }
        else if (mixerSettings.mixUsingDsp)
        {
            r->dspMixer[nxserver_audio_mixer_stereo] = true;
        }

        /* Only open a multichannel mixer if we are in Dolby MS mode or we are NOT using DSP mixers
           (only one DSP mixer can connect to a Decoder, but multiple FMM mixers can connect
            to a single Decoder) */
        if (r->dspMixer[nxserver_audio_mixer_multichannel]) {
            r->mixer[nxserver_audio_mixer_multichannel] = NEXUS_AudioMixer_Open(&mixerSettings);
        }
        else {
            NEXUS_AudioMixerSettings fmmMixerSettings;
            NEXUS_AudioMixer_GetDefaultSettings(&fmmMixerSettings);
            fmmMixerSettings.fixedOutputFormatEnabled = true;
            fmmMixerSettings.fixedOutputFormat = NEXUS_AudioMultichannelFormat_e5_1;
            r->mixer[nxserver_audio_mixer_multichannel] = NEXUS_AudioMixer_Open(&fmmMixerSettings);
        }
        BDBG_MSG(("open AudioMixer multichannel (%p)%s", (void*)r->mixer[nxserver_audio_mixer_multichannel], r->dspMixer[nxserver_audio_mixer_multichannel]?" DSP":""));
        if (!r->mixer[nxserver_audio_mixer_multichannel]) {
            rc = BERR_TRACE(NEXUS_UNKNOWN);
            goto error;
        }

        /* If we are using FMM mixers or we didn't create a Multichannel mixer above,
           then create a stereo mixer here */
        if (r->dspMixer[nxserver_audio_mixer_stereo]) {
            r->mixer[nxserver_audio_mixer_stereo] = NEXUS_AudioMixer_Open(&mixerSettings);
        }
        else {
            NEXUS_AudioMixerSettings fmmMixerSettings;
            NEXUS_AudioMixer_GetDefaultSettings(&fmmMixerSettings);
            fmmMixerSettings.fixedOutputFormatEnabled = true;
            fmmMixerSettings.fixedOutputFormat = NEXUS_AudioMultichannelFormat_eStereo;
            r->mixer[nxserver_audio_mixer_stereo] = NEXUS_AudioMixer_Open(&fmmMixerSettings);
        }
        BDBG_MSG(("open AudioMixer stereo(%p)%s", (void*)r->mixer[nxserver_audio_mixer_stereo], r->dspMixer[nxserver_audio_mixer_stereo]?" DSP":""));
        if (!r->mixer[nxserver_audio_mixer_stereo]) {
            rc = BERR_TRACE(NEXUS_UNKNOWN);
            goto error;
        }

        /* Create Intermediate mixer for persistent decoders */
        /* MS12v1.3 - Only create this mixer for non MS12 cases */
        if ( server->settings.session[r->session->index].persistentDecoderSupport &&
             server->settings.session[r->session->index].dolbyMs != nxserverlib_dolby_ms_type_ms12 ) {
            NEXUS_AudioMixer_GetDefaultSettings(&mixerSettings);
            mixerSettings.intermediate = true;
            mixerSettings.fixedOutputFormat = NEXUS_AudioMultichannelFormat_eStereo;
            mixerSettings.fixedOutputFormatEnabled = true;
            r->mixer[nxserver_audio_mixer_persistent] = NEXUS_AudioMixer_Open(&mixerSettings);
            BDBG_MSG(("open Intermediate AudioMixer (%p)%s", (void*)r->mixer[nxserver_audio_mixer_persistent], r->dspMixer[nxserver_audio_mixer_persistent]?" DSP":""));
            if (!r->mixer[nxserver_audio_mixer_persistent]) {
                rc = BERR_TRACE(NEXUS_UNKNOWN);
                goto error;
            }
            if ( r->mixer[nxserver_audio_mixer_stereo] ) {
                NEXUS_AudioMixer_AddInput(r->mixer[nxserver_audio_mixer_stereo], NEXUS_AudioMixer_GetConnector(r->mixer[nxserver_audio_mixer_persistent]));
            }
            if ( r->mixer[nxserver_audio_mixer_multichannel] ) {
                NEXUS_AudioMixer_AddInput(r->mixer[nxserver_audio_mixer_multichannel], NEXUS_AudioMixer_GetConnector(r->mixer[nxserver_audio_mixer_persistent]));
            }
        }
        for (i=0;i<session->server->settings.session[session->index].audioPlaybacks;i++) {
            b_audio_open_pb(r, false, 0);
        }
        if (r->session->index == 0 && session->server->settings.audioInputs.i2sEnabled) {
            for (i=0;i<cap.numInputs.i2s;i++) {
                b_audio_open_pb(r, true, i);
            }
        }

        if (b_audio_dolby_ms_enabled(server->settings.session[r->session->index].dolbyMs)) {
            /* only MS11 has DV258. In MS12, volume leveling and other DAPv2 features are built into
               the Dsp Mixer */
            if ( server->settings.session[r->session->index].dolbyMs == nxserverlib_dolby_ms_type_ms11 )
            {
                if ( !cap.dsp.dolbyVolume258 )
                {
                    BDBG_ERR(("DolbyVolume258 not available"));
                }
                else {
                    NEXUS_AudioInputHandle audioInput = b_audio_get_pcm_input(r, NEXUS_AudioConnectorType_eMultichannel);
                    r->dolbyVolume258 = NEXUS_DolbyVolume258_Open(NULL);
                    if (r->dolbyVolume258) {
                        rc = NEXUS_DolbyVolume258_AddInput(r->dolbyVolume258, audioInput);
                        if (rc) {
                            BDBG_ERR(("NEXUS_DolbyVolume258_AddInput %d", rc));
                            NEXUS_DolbyVolume258_Close(r->dolbyVolume258);
                            r->dolbyVolume258 = NULL;
                        }
                        else {
                            NEXUS_DolbyVolume258_GetSettings(r->dolbyVolume258, &session->audioProcessingSettings.dolby.dolbyVolume258);
                            session->audioProcessingSettings.dolby.dolbyVolume258.enabled = false;
                            NEXUS_DolbyVolume258_SetSettings(r->dolbyVolume258, &session->audioProcessingSettings.dolby.dolbyVolume258);
                        }
                    }
                }
            }

            if (!cap.dsp.dolbyDigitalReencode) {
                BDBG_ERR(("DDRE not available"));
            }
            else {
                NEXUS_AudioInputHandle audioInput = b_audio_get_pcm_input(r, NEXUS_AudioConnectorType_eMultichannel);
                r->ddre = NEXUS_DolbyDigitalReencode_Open(NULL);
                if (r->ddre) {
                    rc = NEXUS_DolbyDigitalReencode_AddInput(r->ddre, audioInput);
                    if (rc) {
                        BDBG_ERR(("NEXUS_DolbyDigitalReencode_AddInput %d", rc));
                        NEXUS_DolbyDigitalReencode_Close(r->ddre);
                        r->ddre = NULL;
                    }
                    else {
                        NEXUS_DolbyDigitalReencode_GetSettings(r->ddre, &session->audioProcessingSettings.dolby.ddre);
                        if ( server->settings.session[r->session->index].dolbyMs == nxserverlib_dolby_ms_type_ms12 ) {
                            session->audioProcessingSettings.dolby.ddre.fixedEncoderFormat = true;
                        }
                        NEXUS_DolbyDigitalReencode_SetSettings(r->ddre, &session->audioProcessingSettings.dolby.ddre);
                    }
                }
            }
        }
        else {
            /* non-MS11 filter graph */
            if (r->localSession && cap.dsp.codecs[NEXUS_AudioCodec_eAc3].encode) {
                NEXUS_AudioEncoderSettings encoderSettings;
                NEXUS_AudioEncoder_GetDefaultSettings(&encoderSettings);
                r->audioEncoder = NEXUS_AudioEncoder_Open(&encoderSettings);
                if (r->audioEncoder) {
                    NEXUS_AudioEncoder_AddInput(r->audioEncoder,
                        NEXUS_AudioDecoder_GetConnector(r->audioDecoder[nxserver_audio_decoder_primary], NEXUS_AudioConnectorType_eStereo));
                }
            }

            if (server->settings.session[r->session->index].avl) {
                if (!cap.dsp.autoVolumeLevel) {
                    BDBG_ERR(("auto volume level not available"));
                }
                else {
                    NEXUS_AudioInputHandle audioInput = b_audio_get_pcm_input(r, NEXUS_AudioConnectorType_eStereo);
                    r->avl = NEXUS_AutoVolumeLevel_Open(NULL);
                    if (r->avl) {
                        rc = NEXUS_AutoVolumeLevel_AddInput(r->avl, audioInput);
                        if (rc) {
                            BDBG_ERR(("NEXUS_AutoVolumeLevel_AddInput %d", rc));
                            NEXUS_AutoVolumeLevel_Close(r->avl);
                            r->avl = NULL;
                        }
                        else {
                            NEXUS_AutoVolumeLevel_GetSettings(r->avl, &session->audioProcessingSettings.avl);
                            session->audioProcessingSettings.avl.enabled = false;
                            NEXUS_AutoVolumeLevel_SetSettings(r->avl, &session->audioProcessingSettings.avl);
                        }
                    }
                }
            }

            if (server->settings.session[r->session->index].truVolume) {
                if (!cap.dsp.truVolume) {
                    BDBG_ERR(("SRS TruVolume not available"));
                }
                else {
                    NEXUS_AudioInputHandle audioInput = b_audio_get_pcm_input(r, NEXUS_AudioConnectorType_eStereo);
                    r->truVolume = NEXUS_TruVolume_Open(NULL);
                    if (r->truVolume) {
                        rc = NEXUS_TruVolume_AddInput(r->truVolume, audioInput);
                        if (rc) {
                            BDBG_ERR(("NEXUS_TruVolume_AddInput %d", rc));
                            NEXUS_TruVolume_Close(r->truVolume);
                            r->truVolume = NULL;
                        }
                        else {
                            NEXUS_TruVolume_GetSettings(r->truVolume, &session->audioProcessingSettings.truVolume);
                            session->audioProcessingSettings.truVolume.enabled = false;
                            NEXUS_TruVolume_SetSettings(r->truVolume, &session->audioProcessingSettings.truVolume);
                        }
                    }
                }
            }
        }

        if ((r->truVolume || r->avl) && server->settings.audioDecoder.audioDescription) {
            BDBG_ERR(("AVL or truVolume with Audio Description is not supported"));
            goto error;
        }

        if (nxserverlib_p_session_has_sd_audio(r->session)) {
            if (cap.numOutputs.dac > 0)
            {
                rc = NEXUS_AudioOutput_AddInput(NEXUS_AudioDac_GetConnector(server->platformConfig.outputs.audioDacs[0]), b_audio_get_pcm_input(r, NEXUS_AudioConnectorType_eStereo));
                BERR_TRACE(rc);
            }
            else if (cap.numOutputs.i2s > 0) {
                rc = NEXUS_AudioOutput_AddInput(NEXUS_I2sOutput_GetConnector(server->platformConfig.outputs.i2s[0]), b_audio_get_pcm_input(r, NEXUS_AudioConnectorType_eStereo));
                BERR_TRACE(rc);
            }
            else if (cap.numOutputs.dummy > 0) {
                unsigned i;
                for (i=0;i<cap.numOutputs.dummy;i++) {
                    if (!g_dummyOutputs[i]) {
                        rc = NEXUS_AudioOutput_AddInput( NEXUS_AudioDummyOutput_GetConnector(server->platformConfig.outputs.audioDummy[i]), b_audio_get_pcm_input(r, NEXUS_AudioConnectorType_eStereo));
                        BERR_TRACE(rc);
                        g_dummyOutputs[i] = r;
                        break;
                    }
                }
            }
            #if NEXUS_NUM_RFM_OUTPUTS
            if (server->platformConfig.outputs.rfm[0]) {
                rc = NEXUS_AudioOutput_AddInput(NEXUS_Rfm_GetAudioConnector(server->platformConfig.outputs.rfm[0]), b_audio_get_pcm_input(r, NEXUS_AudioConnectorType_eStereo));
                BERR_TRACE(rc);
            }
            #endif
        }
    }
    else if ( type == b_audio_decoder_type_persistent ) {
        r->mode = b_audio_mode_decode;
        if (b_alloc_audio_index(r, &index)) {
            rc = BERR_TRACE(NEXUS_UNKNOWN);
            goto error;
        }
        audioOpenSettings.karaokeSupported = server->settings.session[session->index].karaoke;
        r->audioDecoder[nxserver_audio_decoder_primary] = NEXUS_AudioDecoder_Open(index, &audioOpenSettings);
        if (!r->audioDecoder[nxserver_audio_decoder_primary]) {
            rc = BERR_TRACE(NEXUS_UNKNOWN);
            goto error;
        }
    }
    else if ( type == b_audio_decoder_type_standalone ) {
        r->mode = b_audio_mode_decode;
        if (b_alloc_audio_index(r, &index)) {
            rc = BERR_TRACE(NEXUS_UNKNOWN);
            goto error;
        }
        audioOpenSettings.karaokeSupported = server->settings.session[session->index].karaoke;
        r->audioDecoder[nxserver_audio_decoder_primary] = NEXUS_AudioDecoder_Open(index, &audioOpenSettings);
        if (!r->audioDecoder[nxserver_audio_decoder_primary]) {
            rc = BERR_TRACE(NEXUS_UNKNOWN);
            goto error;
        }
        if (b_alloc_audio_index(r, &index)) {
            rc = BERR_TRACE(NEXUS_UNKNOWN);
            goto error;
        }
        audioOpenSettings.karaokeSupported = false;
        audioOpenSettings.type = NEXUS_AudioDecoderType_ePassthrough;
        r->audioDecoder[nxserver_audio_decoder_passthrough] = NEXUS_AudioDecoder_Open(index, &audioOpenSettings);
        if (!r->audioDecoder[nxserver_audio_decoder_passthrough]) {
            rc = BERR_TRACE(NEXUS_UNKNOWN);
            goto error;
        }
    }
    /* transcode */
    else {
        r->mode = b_audio_mode_transcode;
        if (b_alloc_audio_index(r, &index)) {
            rc = BERR_TRACE(NEXUS_UNKNOWN);
            goto error;
        }
        if (cap.numDsps > 1) {
            /* DSP 1 supports two transcodes only */
            unsigned i, total = 0;
            for (i=0;i<cap.numDecoders;i++) {
                if (g_decoders[i].r && g_decoders[i].r->dspIndex == 1) total++;
            }
            if (total < 2) {
                r->dspIndex = audioOpenSettings.dspIndex = 1;
            }
        }
        r->audioDecoder[nxserver_audio_decoder_primary] = NEXUS_AudioDecoder_Open(index, &audioOpenSettings);
        if (!r->audioDecoder[nxserver_audio_decoder_primary]) {
            rc = BERR_TRACE(NEXUS_UNKNOWN);
            goto error;
        }
        BDBG_MSG(("open AudioDecoder0(%p) for %p (transcode)", (void*)r->audioDecoder[nxserver_audio_decoder_primary], (void*)r));

        /* DSP mixer for transcode decouples audio decode and encode. This allows encode to proceed even
        if decode is stalled, flushed, etc. */
        NEXUS_AudioMixer_GetDefaultSettings(&mixerSettings);
        mixerSettings.mixUsingDsp = true;
        mixerSettings.dspIndex = audioOpenSettings.dspIndex; /* mixer dspIndex must match decoder */
        r->mixer[nxserver_audio_mixer_stereo] = NEXUS_AudioMixer_Open(&mixerSettings);
        if (!r->mixer[nxserver_audio_mixer_stereo]) {
            BDBG_WRN(("unable to open a DSP AudioMixer for transcode"));
            /* still allow transcode */
        }
    }

    if (!r->localSession && type != b_audio_decoder_type_background_nrt && cap.numOutputs.dummy > 0) {
        unsigned i;
        for (i=0;i<cap.numOutputs.dummy;i++) {
            if (!g_dummyOutputs[i]) {
                rc = NEXUS_AudioOutput_AddInput(
                    NEXUS_AudioDummyOutput_GetConnector(server->platformConfig.outputs.audioDummy[i]),
                    r->mixer[nxserver_audio_mixer_stereo] ? b_audio_get_pcm_input(r, NEXUS_AudioConnectorType_eStereo) : NEXUS_AudioDecoder_GetConnector(r->audioDecoder[nxserver_audio_decoder_primary], NEXUS_AudioDecoderConnectorType_eStereo));
                if (rc) {rc = BERR_TRACE(rc); goto error;}
                g_dummyOutputs[i] = r;
                break;
            }
        }
    }

    /* create a master so that we get configured even if first audio client is pcm playback */
    r->masterSimpleAudioDecoder = NEXUS_SimpleAudioDecoder_Create(session->audio.server, server->nextId[b_resource_simple_audio_decoder], NULL);
    if (!r->masterSimpleAudioDecoder) {
        rc = BERR_TRACE(NEXUS_NOT_AVAILABLE);
        goto error;
    }
    inc_id(server, b_resource_simple_audio_decoder);

    if (type == b_audio_decoder_type_regular) {
        rc = bserver_set_audio_config(r);
        if (rc) {rc = BERR_TRACE(rc); goto error;}
    }

    if (r->localSession) {
        NEXUS_AudioMixerSettings mixerSettings;
        NEXUS_AudioOutputSettings outputSettings;

        /* init session->audioSettings */
        /* dac is master */
        if (cap.numOutputs.dac > 0) {
            NEXUS_AudioOutput_GetSettings(NEXUS_AudioDac_GetConnector(server->platformConfig.outputs.audioDacs[0]), &outputSettings);
        }
        else if (cap.numOutputs.i2s > 0) {
            NEXUS_AudioOutput_GetSettings(NEXUS_I2sOutput_GetConnector(server->platformConfig.outputs.i2s[0]), &outputSettings);
        }
        else {
            BKNI_Memset(&outputSettings, 0, sizeof(outputSettings));
        }
        session->audioSettings.volumeType = outputSettings.volumeType;
        session->audioSettings.leftVolume = outputSettings.leftVolume;
        session->audioSettings.rightVolume = outputSettings.rightVolume;

        NEXUS_AudioMixer_GetDefaultSettings(&mixerSettings);
        BKNI_Memcpy(&session->audioSettings.loopbackVolumeMatrix, mixerSettings.loopbackVolumeMatrix, sizeof(int32_t)*NEXUS_AudioChannel_eMax*NEXUS_AudioChannel_eMax);
    }

    return r;

error:
    audio_decoder_destroy(r);
    return NULL;
}

void audio_decoder_destroy(struct b_audio_resource *r)
{
    unsigned i;
    struct b_audio_playback_resource *pb;
    NEXUS_AudioCapabilities audioCapabilities;
    nxserver_t server = r->session->server;

    NEXUS_GetAudioCapabilities(&audioCapabilities);
    BDBG_MSG(("destroy %p", (void*)r));

    if (r->masterSimpleAudioDecoder) {
        bool connected = false;
        for (pb = BLST_Q_FIRST(&r->audioPlaybackList); pb; pb = BLST_Q_NEXT(pb, link)) {
            if (pb->audioPlayback) {
                /* If PCM Playbacks are connected to this SimpleDecoder's mixer we need to stop them */
                connected = is_connected_to_a_mixer(NEXUS_AudioPlayback_GetConnector(pb->audioPlayback), r);
            }
        }
        if (connected) {
            NEXUS_SimpleAudioDecoder_Suspend(r->masterSimpleAudioDecoder);
        }

        NEXUS_SimpleAudioDecoder_Destroy(r->masterSimpleAudioDecoder);
    }
    if (r->audioEncoder) {
        NEXUS_AudioEncoder_RemoveAllInputs(r->audioEncoder);
    }

    if (audioCapabilities.numOutputs.dummy > 0) {
        for (i = 0; i < audioCapabilities.numOutputs.dummy; i++)
        {
            if (g_dummyOutputs[i] == r) {
                NEXUS_AudioOutput_RemoveAllInputs(NEXUS_AudioDummyOutput_GetConnector(server->platformConfig.outputs.audioDummy[i]));
                g_dummyOutputs[i] = NULL;
            }
        }
    }

    if (r->mixer[nxserver_audio_mixer_stereo]) {
        if (r->localSession && r->session->index == 0) {
            if (audioCapabilities.numOutputs.dac > 0) {
                NEXUS_AudioOutput_RemoveAllInputs(NEXUS_AudioDac_GetConnector(server->platformConfig.outputs.audioDacs[0]));
            }
            else if (audioCapabilities.numOutputs.i2s > 0) {
                NEXUS_AudioOutput_RemoveAllInputs(NEXUS_I2sOutput_GetConnector(server->platformConfig.outputs.i2s[0]));
            }
            #if NEXUS_HAS_RFM && NEXUS_NUM_RFM_OUTPUTS
            if (server->platformConfig.outputs.rfm[0]) {
                NEXUS_AudioOutput_RemoveAllInputs(NEXUS_Rfm_GetAudioConnector(server->platformConfig.outputs.rfm[0]));
            }
            #endif
            #if NEXUS_NUM_HDMI_OUTPUTS
            if (audioCapabilities.numOutputs.hdmi > 0) {
                NEXUS_AudioOutput_RemoveAllInputs(NEXUS_HdmiOutput_GetAudioConnector(server->platformConfig.outputs.hdmi[0]));
            }
            if (audioCapabilities.numOutputs.spdif > 0) {
                NEXUS_AudioOutput_RemoveAllInputs(NEXUS_SpdifOutput_GetConnector(server->platformConfig.outputs.spdif[0]));
            }
            #endif
        }
        NEXUS_AudioMixer_RemoveAllInputs(r->mixer[nxserver_audio_mixer_stereo]);
    }

    if (r->mixer[nxserver_audio_mixer_multichannel]) {
        NEXUS_AudioMixer_RemoveAllInputs(r->mixer[nxserver_audio_mixer_multichannel]);
    }

    if (r->mixer[nxserver_audio_mixer_persistent]) {
        NEXUS_AudioMixer_RemoveAllInputs(r->mixer[nxserver_audio_mixer_persistent]);
    }

    while ((pb = BLST_Q_FIRST(&r->audioPlaybackList))) {
        b_audio_close_pb(r, pb);
    }
    if (r->audioEncoder) {
        NEXUS_AudioEncoder_Close(r->audioEncoder);
    }
    if (r->ddre) {
        NEXUS_DolbyDigitalReencode_Close(r->ddre);
    }
    if (r->dolbyVolume258) {
        NEXUS_DolbyVolume258_Close(r->dolbyVolume258);
    }
    if (r->avl)
    {
        NEXUS_AutoVolumeLevel_Close(r->avl);
    }
    if (r->truVolume)
    {
        NEXUS_TruVolume_Close(r->truVolume);
    }
    if (r->mixer[nxserver_audio_mixer_persistent]) {
        NEXUS_AudioMixer_Close(r->mixer[nxserver_audio_mixer_persistent]);
    }
    if (r->mixer[nxserver_audio_mixer_stereo]) {
        NEXUS_AudioMixer_Close(r->mixer[nxserver_audio_mixer_stereo]);
    }
    if (r->mixer[nxserver_audio_mixer_multichannel]) {
        NEXUS_AudioMixer_Close(r->mixer[nxserver_audio_mixer_multichannel]);
    }
    for (i=0;i<nxserver_audio_decoder_max;i++) {
        if (r->audioDecoder[i]) {
            NEXUS_AudioInput_Shutdown(NEXUS_AudioDecoder_GetConnector(r->audioDecoder[i], NEXUS_AudioConnectorType_eMultichannel));
            NEXUS_AudioInput_Shutdown(NEXUS_AudioDecoder_GetConnector(r->audioDecoder[i], NEXUS_AudioConnectorType_eStereo));
            NEXUS_AudioDecoder_Close(r->audioDecoder[i]);
        }
    }
    if (r->passthroughPlayback) {
        NEXUS_AudioInput_Shutdown(NEXUS_AudioPlayback_GetConnector(r->passthroughPlayback));
        NEXUS_AudioPlayback_Close(r->passthroughPlayback);
    }
    b_dealloc_audio_index(r);
    BKNI_Free(r);
}

static NEXUS_Error audio_acquire_stc_index(struct b_connect * connect, NEXUS_SimpleAudioDecoderHandle audioDecoder)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    struct b_stc_caps stcreq;
    int stcIndex;

    NEXUS_SimpleAudioDecoder_GetStcIndex(connect->client->session->audio.server, audioDecoder, &stcIndex);
    if (stcIndex == -1)
    {
        stc_index_request_init(connect, &stcreq);
        /* NRT mode audio and video decoder STCs are different, so audio
               STC doesn't need to have video decoder capability! */
        if (nxserver_p_connect_is_nonrealtime_encode(connect)) {
            stcreq.video = false;
        }
        stcIndex = stc_index_acquire(connect, &stcreq);
        if (stcIndex == -1)
        {
            rc = BERR_TRACE(NEXUS_NOT_AVAILABLE);
            goto error;
        }
        BDBG_MSG_TRACE(("SAD %p acquired STC%u", audioDecoder, stcIndex));
        NEXUS_SimpleAudioDecoder_SetStcIndex(connect->client->session->audio.server, audioDecoder, stcIndex);
    }
    else
    {
        BDBG_WRN(("SAD %p attempted to acquire STC -1", (void*)audioDecoder));
        rc = BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }

error:
    return rc;
}

static void audio_release_stc_index(struct b_connect *connect, NEXUS_SimpleAudioDecoderHandle audioDecoder)
{
    int stcIndex;

    NEXUS_SimpleAudioDecoder_GetStcIndex(connect->client->session->audio.server, audioDecoder, &stcIndex);
    if (stcIndex != -1)
    {
        BDBG_MSG_TRACE(("SAD %p releasing STC%u", (void*)audioDecoder, stcIndex));
        stc_index_release(connect, stcIndex);
        stcIndex = -1;
        NEXUS_SimpleAudioDecoder_SetStcIndex(connect->client->session->audio.server, audioDecoder, stcIndex);
    }
}

/**
transitions:
playback/decode should use swap
transcode sits by itself
**/
int acquire_audio_decoders(struct b_connect *connect, bool force_grab)
{
    int rc;
    struct b_session *session = connect->client->session;
    struct b_audio_resource *r = NULL;
    bool grab = !is_transcode_connect(connect);
    NEXUS_SimpleAudioDecoderHandle audioDecoder = b_audio_get_decoder(connect);
    struct b_req *req = connect->req[b_resource_simple_audio_decoder];

    if (!session->server->settings.grab) {
        grab = false;
    }
    if (!connect->settings.simpleAudioDecoder.id) {
        /* no request */
        return 0;
    }

    if (req->handles.simpleAudioDecoder.r) {
        if (req->handles.simpleAudioDecoder.r->connect != connect) {
            BDBG_ERR(("already connected to %p", (void*)req->handles.simpleAudioDecoder.r->connect));
            return BERR_TRACE(NEXUS_NOT_AVAILABLE);
        }
    }
    if (session->main_audio && session->main_audio->connect == connect) {
        BDBG_WRN(("already connected"));
        return 0;
    }

    if (is_main_audio(connect)) {
        if (!session->main_audio) {
            return BERR_TRACE(NEXUS_NOT_AVAILABLE);
        }
        if (session->main_audio->connect) {
            if (!force_grab && connect->settings.simpleAudioDecoder.primer) {
                return 0;
            }
            if (!grab) {
                return BERR_TRACE(NEXUS_NOT_AVAILABLE);
            }
        }
        r = session->main_audio;
    }
    else {
        b_audio_decoder_type decoder_type = b_audio_decoder_type_background;
        if ( connect->settings.simpleAudioDecoder.decoderCapabilities.type == NxClient_AudioDecoderType_ePersistent ) {
            /* We just want to get a connectId and prepare for possible swapping */
            if (connect->settings.simpleAudioDecoder.primer) {
                return 0;
            }
            decoder_type = b_audio_decoder_type_persistent;
        }
        else if ( connect->settings.simpleAudioDecoder.decoderCapabilities.type == NxClient_AudioDecoderType_eStandalone ) {
            decoder_type = b_audio_decoder_type_standalone;
        }
        else if ( nxserver_p_connect_is_nonrealtime_encode(connect) ) {
            decoder_type = b_audio_decoder_type_background_nrt;
        }
        r = audio_decoder_create(session, decoder_type);
    }
    if (!r) {
        return BERR_TRACE(NEXUS_NOT_AVAILABLE);
    }

    if (is_main_audio(connect)) {
        NEXUS_SimpleAudioDecoderServerSettings settings;
        NEXUS_SimpleAudioDecoderHandle mainAudioDecoder;
        NEXUS_SimpleAudioDecoder_GetServerSettings(session->audio.server, audioDecoder, &settings);
        settings.type = NEXUS_SimpleAudioDecoderType_eDynamic;
        rc = NEXUS_SimpleAudioDecoder_SetServerSettings(session->audio.server, audioDecoder, &settings);

        mainAudioDecoder = b_audio_get_decoder(session->main_audio->connect);
        if (r->mode == b_audio_mode_decode) {
            audio_release_stc_index(connect, mainAudioDecoder);
            BDBG_MSG(("transfer audio %p: connect %p(%p) -> connect %p(%p)",
                      (void*)r, (void*)session->main_audio->connect, (void*)mainAudioDecoder,
                      (void*)connect, (void*)audioDecoder));
            rc = NEXUS_SimpleAudioDecoder_SwapServerSettings(session->audio.server, mainAudioDecoder, audioDecoder);
            if (rc) { rc = BERR_TRACE(rc); goto err_setsettings; }

            rc = audio_acquire_stc_index(connect, audioDecoder);
            if (rc) { rc = BERR_TRACE(rc); goto err_setsettings; }

            session->main_audio->connect->req[b_resource_simple_audio_decoder]->handles.simpleAudioDecoder.r = NULL;
        }
        else if (audioDecoder) {
            audio_release_stc_index(connect, r->masterSimpleAudioDecoder);
            BDBG_MSG(("transfer audio %p: playback(%p) -> connect %p(%p)",
                      (void*)r, (void*)r->masterSimpleAudioDecoder, (void*)connect, (void*)audioDecoder));
            rc = NEXUS_SimpleAudioDecoder_SwapServerSettings(session->audio.server,
                r->masterSimpleAudioDecoder,
                audioDecoder);
            if (rc) { rc = BERR_TRACE(rc); goto err_setsettings; }

            rc = audio_acquire_stc_index(connect, audioDecoder);
            if (rc) { rc = BERR_TRACE(rc); goto err_setsettings; }

            r->mode = b_audio_mode_decode;
        }

        session->main_audio->connect = connect;
    }
    else if ( connect->settings.simpleAudioDecoder.decoderCapabilities.type == NxClient_AudioDecoderType_ePersistent ) {
        NEXUS_SimpleAudioDecoderServerSettings settings;
        nxserver_t server = session->server;

        if ( !server->settings.session[r->session->index].persistentDecoderSupport ) {
            BDBG_ERR(("nxserver must be started with -persistent_sad option to use type ePersistent"));
            rc = BERR_TRACE(NEXUS_NOT_AVAILABLE); goto err_setsettings;
        }
        BDBG_MSG(("acquire persistent audio %p: connect %p", (void*)r, (void*)connect));
        NEXUS_SimpleAudioDecoder_GetServerSettings(session->audio.server, audioDecoder, &settings);
        settings.primary = r->audioDecoder[nxserver_audio_decoder_primary];
        settings.type = NEXUS_SimpleAudioDecoderType_ePersistent;
        /* MS12 will not have a persistent mixer, use multichannel mixer */
        if ( server->settings.session[r->session->index].dolbyMs == nxserverlib_dolby_ms_type_ms12 ) {
            settings.capabilities.ms12 = true;
            settings.mixers.persistent = NULL;
            settings.mixers.multichannel = session->main_audio->mixer[nxserver_audio_mixer_multichannel];
        } else if ( session->main_audio->mixer[nxserver_audio_mixer_persistent] ) {
            settings.mixers.multichannel = settings.mixers.persistent = NULL;
            settings.mixers.stereo = session->main_audio->mixer[nxserver_audio_mixer_persistent];
        }
        else {
            BDBG_ERR(("No Persistent Decoder support in this configuration"));
            rc = BERR_TRACE(NEXUS_NOT_AVAILABLE); goto err_setsettings;
        }
        rc = NEXUS_SimpleAudioDecoder_SetServerSettings(session->audio.server, audioDecoder, &settings);
        if (rc) { rc = BERR_TRACE(rc); goto err_setsettings; }
        /* TBD - should this be any different for persistent decoders??? */
        {
            int i;
            NEXUS_SimpleAudioDecoderHandle activeDecoder;
            activeDecoder = b_audio_get_active_decoder(session->main_audio);
            NEXUS_SimpleAudioDecoder_GetServerSettings(session->audio.server, activeDecoder, &settings);
            for (i = 0; i < NEXUS_MAX_AUDIO_DECODERS; i++) {
                if (settings.persistent[i].decoder == NULL) {
                    settings.persistent[i].decoder = r->audioDecoder[nxserver_audio_decoder_primary];
                    settings.persistent[i].suspended = false;
                    break;
                }
            }

            rc = NEXUS_SimpleAudioDecoder_SetServerSettings(session->audio.server, activeDecoder, &settings);
            if (rc) { rc = BERR_TRACE(rc); goto err_setmastersettings; }
        }
        rc = audio_acquire_stc_index(connect, audioDecoder);
        if (rc) { rc = BERR_TRACE(rc); goto err_setsettings; }
    }
    else if ( connect->settings.simpleAudioDecoder.decoderCapabilities.type == NxClient_AudioDecoderType_eStandalone ) {
        NEXUS_SimpleAudioDecoderServerSettings settings;
        BDBG_MSG(("acquire standalone audio %p: connect %p", (void*)r, (void*)connect));
        NEXUS_SimpleAudioDecoder_GetServerSettings(session->audio.server, audioDecoder, &settings);
        settings.primary = r->audioDecoder[nxserver_audio_decoder_primary];
        settings.secondary = r->audioDecoder[nxserver_audio_decoder_passthrough];
        settings.type = NEXUS_SimpleAudioDecoderType_eStandalone;
        settings.mixers.stereo = settings.mixers.multichannel = settings.mixers.persistent = NULL;
        rc = NEXUS_SimpleAudioDecoder_SetServerSettings(session->audio.server, audioDecoder, &settings);
        if (rc) { rc = BERR_TRACE(rc); goto err_setsettings; }
        /* TBD - should this be any different for standalone decoders??? */
        rc = audio_acquire_stc_index(connect, audioDecoder);
        if (rc) { rc = BERR_TRACE(rc); goto err_setsettings; }
    }
    else {
        /* transcode - no swap. simple_encoder configures filter graph internally based on decode codec */
        NEXUS_SimpleAudioDecoderServerSettings settings;
        BDBG_ASSERT(r->mode == b_audio_mode_transcode); /* no mode switch */
        BDBG_MSG(("acquire transcode audio %p: connect %p", (void*)r, (void*)connect));
        NEXUS_SimpleAudioDecoder_GetServerSettings(session->audio.server, audioDecoder, &settings);
        settings.primary = r->audioDecoder[nxserver_audio_decoder_primary];
        settings.type = NEXUS_SimpleAudioDecoderType_eDynamic;
        rc = NEXUS_SimpleAudioDecoder_SetServerSettings(session->audio.server, audioDecoder, &settings);
        if (rc) { rc = BERR_TRACE(rc); goto err_setsettings; }
        rc = audio_acquire_stc_index(connect, audioDecoder);
        if (rc) { rc = BERR_TRACE(rc); goto err_setsettings; }
    }

    req->handles.simpleAudioDecoder.r = r;
    r->connect = connect;
    return 0;

err_setsettings:
    if ( connect->settings.simpleAudioDecoder.decoderCapabilities.type == NxClient_AudioDecoderType_ePersistent ) {
        int i;
        NEXUS_SimpleAudioDecoderServerSettings settings;
        NEXUS_SimpleAudioDecoderHandle activeDecoder;
        activeDecoder = b_audio_get_active_decoder(session->main_audio);

        NEXUS_SimpleAudioDecoder_GetServerSettings(session->audio.server, activeDecoder, &settings);
        for (i = 0; i < NEXUS_MAX_AUDIO_DECODERS; i++) {
            if (settings.persistent[i].decoder == r->audioDecoder[nxserver_audio_decoder_primary]) {
                settings.persistent[i].decoder = NULL;
                settings.persistent[i].suspended = false;
                break;
            }
        }
        NEXUS_SimpleAudioDecoder_SetServerSettings(session->audio.server, activeDecoder, &settings);
    }
err_setmastersettings:
    if (is_main_audio(connect)) {
        /* failed swap: leave state alone. acquire_stc should never fail because of release_stc. */
    }
    else {
        NEXUS_SimpleAudioDecoderServerSettings settings;
        NEXUS_SimpleAudioDecoder_GetServerSettings(session->audio.server, audioDecoder, &settings);
        settings.primary = NULL;
        (void)NEXUS_SimpleAudioDecoder_SetServerSettings(session->audio.server, audioDecoder, &settings);
        audio_decoder_destroy(r);
    }
    return rc;
}

void release_audio_decoders(struct b_connect *connect)
{
    struct b_audio_resource *r;
    struct b_session *session = connect->client->session;
    struct b_req *req = connect->req[b_resource_simple_audio_decoder];
    NEXUS_SimpleAudioDecoderServerSettings settings;
    NEXUS_SimpleAudioDecoderHandle audioDecoder;

    if (!req) return;
    r = req->handles.simpleAudioDecoder.r;
    if (!r) return;

    audioDecoder = b_audio_get_decoder(r->connect);
    NEXUS_SimpleAudioDecoder_GetServerSettings(session->audio.server, audioDecoder, &settings);

    if (r->mode == b_audio_mode_decode && settings.type == NEXUS_SimpleAudioDecoderType_eDynamic) {
        NEXUS_SimpleAudioDecoderHandle mainAudioDecoder = b_audio_get_decoder(session->main_audio->connect);
        /* when releasing the main decoder, transfer back to masterSimpleAudioDecoder */
        audio_release_stc_index(connect, mainAudioDecoder);
        BDBG_MSG(("transfer audio %p: connect %p -> playback", (void*)r, (void*)session->main_audio->connect));
        (void)NEXUS_SimpleAudioDecoder_SwapServerSettings(session->audio.server, mainAudioDecoder, r->masterSimpleAudioDecoder);
        session->main_audio->connect = NULL;
        r->mode = b_audio_mode_playback;
    }
    else if (r->mode != b_audio_mode_playback) {
        audio_release_stc_index(connect, audioDecoder);
        BDBG_MSG(("release transcode audio %p: connect %p", (void*)r, (void*)r->connect));
        settings.primary = settings.secondary = NULL;
        (void)NEXUS_SimpleAudioDecoder_SetServerSettings(session->audio.server, audioDecoder, &settings);
        if (settings.type == NEXUS_SimpleAudioDecoderType_ePersistent) {
            int i;
            NEXUS_SimpleAudioDecoderServerSettings settings;
            NEXUS_SimpleAudioDecoderHandle activeDecoder;
            activeDecoder = b_audio_get_active_decoder(session->main_audio);

            NEXUS_SimpleAudioDecoder_GetServerSettings(session->audio.server, activeDecoder, &settings);
            for (i = 0; i < NEXUS_MAX_AUDIO_DECODERS; i++) {
                if (settings.persistent[i].decoder == r->audioDecoder[nxserver_audio_decoder_primary]) {
                    settings.persistent[i].decoder = NULL;
                    settings.persistent[i].suspended = false;
                    break;
                }
            }
            NEXUS_SimpleAudioDecoder_SetServerSettings(session->audio.server, activeDecoder, &settings);
        }
        audio_decoder_destroy(r);
    }

    req->handles.simpleAudioDecoder.r = NULL;
}

static void bserver_init_audio_config(nxserver_t server)
{
    unsigned i;
    struct b_audio_config *config = &server->audio_config;
    NEXUS_AudioCapabilities cap;

    NEXUS_GetAudioCapabilities(&cap);
    BKNI_Memset(config, 0, sizeof(*config));

    for (i=0;i<NEXUS_AudioCodec_eMax;i++) {
        /* HDMI output will be adjusted after reading the EDID */
        config->hdmi.audioCodecOutput[i] = NxClient_AudioOutputMode_ePcm;

        /* SPDIF output is defaulted based on the SPDIF standard and our capabilities */
        if (i != NEXUS_AudioCodec_eUnknown && !cap.dsp.codecs[i].decode) {
            config->spdif.audioCodecOutput[i] = NxClient_AudioOutputMode_ePassthrough;
        }
        else {
            /* TODO: Add transcode support for DtsLegacy */
            switch (i) {
            case NEXUS_AudioCodec_eAc3:
            case NEXUS_AudioCodec_eAc3Plus: /* for AC3+, DSP does downconvert */
            case NEXUS_AudioCodec_eDts:
            case NEXUS_AudioCodec_eDtsHd:
                /* We are assuming SPDIF can decode AC3 and DTS audio, so we passthrough even if we can decode */
                config->spdif.audioCodecOutput[i] = NxClient_AudioOutputMode_ePassthrough;
                break;
            case NEXUS_AudioCodec_eAacAdts: /* for AAC, DSP decode/encode */
            case NEXUS_AudioCodec_eAacLoas:
            case NEXUS_AudioCodec_eAacPlusLoas:
            case NEXUS_AudioCodec_eAacPlusAdts:
                if (cap.dsp.codecs[NEXUS_AudioCodec_eAc3].encode) {
                    config->spdif.audioCodecOutput[i] = NxClient_AudioOutputMode_eTranscode; /* to ac3 */
                    break;
                }
                /* fall through to pcm */
            default:
                config->spdif.audioCodecOutput[i] = NxClient_AudioOutputMode_ePcm;
                break;
            }
        }
    }
}

void bserver_acquire_audio_mixers(struct b_audio_resource *r, bool start)
{
    #if NEXUS_AUDIO_MODULE_FAMILY == NEXUS_AUDIO_MODULE_FAMILY_APE_RAAGA
    if (start) {
        if (r->mixer[nxserver_audio_mixer_stereo])
        {
            BDBG_MSG(("Explicitly starting stereo mixer %p", (void*)r->mixer[nxserver_audio_mixer_stereo]));
            NEXUS_AudioMixer_Start(r->mixer[nxserver_audio_mixer_stereo]);
        }
        if (r->mixer[nxserver_audio_mixer_multichannel])
        {
            BDBG_MSG(("Explicitly starting multichannel mixer %p", (void*)r->mixer[nxserver_audio_mixer_multichannel]));
            NEXUS_AudioMixer_Start(r->mixer[nxserver_audio_mixer_multichannel]);
        }
        if (r->mixer[nxserver_audio_mixer_persistent])
        {
            BDBG_MSG(("Explicitly starting persistent mixer %p", (void*)r->mixer[nxserver_audio_mixer_persistent]));
            NEXUS_AudioMixer_Start(r->mixer[nxserver_audio_mixer_persistent]);
        }
    }
    else {
        if (r->mixer[nxserver_audio_mixer_persistent])
        {
            BDBG_MSG(("Explicitly stopping stereo mixer %p", (void*)r->mixer[nxserver_audio_mixer_stereo]));
            NEXUS_AudioMixer_Stop(r->mixer[nxserver_audio_mixer_persistent]);
        }
        if (r->mixer[nxserver_audio_mixer_stereo])
        {
            BDBG_MSG(("Explicitly stopping multichannel mixer %p", (void*)r->mixer[nxserver_audio_mixer_multichannel]));
            NEXUS_AudioMixer_Stop(r->mixer[nxserver_audio_mixer_stereo]);
        }
        if (r->mixer[nxserver_audio_mixer_multichannel])
        {
            BDBG_MSG(("Explicitly stopping persistent mixer %p", (void*)r->mixer[nxserver_audio_mixer_persistent]));
            NEXUS_AudioMixer_Stop(r->mixer[nxserver_audio_mixer_multichannel]);
        }
    }
    #else
    BSTD_UNUSED(r);
    BSTD_UNUSED(start);
    #endif
}

int bserver_set_audio_config(struct b_audio_resource *r)
{
    NEXUS_SimpleAudioDecoderHandle simpleAudioDecoder = NULL;
    NEXUS_SimpleAudioDecoderServerSettings audioSettings;
    unsigned i;
    int rc;
    nxserver_t server;
    struct b_session *session;
    const struct b_audio_config *config;
    bool encode_display;
    NEXUS_AudioCapabilities cap;

    if (!r) {
        return 0;
    }
    session = r->session;
    server = r->session->server;
    config = &server->audio_config;
    NEXUS_GetAudioCapabilities(&cap);
    simpleAudioDecoder = b_audio_get_active_decoder(r);

    if (!simpleAudioDecoder) {
        return BERR_TRACE(-1);
    }

    /* TODO: if we are encoding the display, we can't switch audio output config. force to pcm. */
    encode_display = server->settings.session[r->session->index].output.encode;

    NEXUS_SimpleAudioDecoder_GetServerSettings(session->audio.server, simpleAudioDecoder, &audioSettings);

    audioSettings.primary = r->audioDecoder[nxserver_audio_decoder_primary];
    audioSettings.secondary = r->audioDecoder[nxserver_audio_decoder_passthrough];
    audioSettings.description = r->audioDecoder[nxserver_audio_decoder_description];
    audioSettings.passthroughPlayback = r->passthroughPlayback;

    audioSettings.mixers.stereo = r->mixer[nxserver_audio_mixer_stereo];
    audioSettings.mixers.multichannel = r->mixer[nxserver_audio_mixer_multichannel];
    audioSettings.mixers.persistent = r->mixer[nxserver_audio_mixer_persistent];

    if (r->localSession) {
        BKNI_Memset(&audioSettings.spdif, 0, sizeof(audioSettings.spdif));
        BKNI_Memset(&audioSettings.hdmi, 0, sizeof(audioSettings.hdmi));

#if NEXUS_HAS_HDMI_OUTPUT
        /* set up per codec outputs for HDMI and SPDIF */
        audioSettings.hdmi.outputs[0] = r->session->hdmiOutput;
        if (r->session->hdmiOutput) {
            for (i=0;i<NEXUS_AudioCodec_eMax;i++) {
                NxClient_AudioOutputMode outputMode;
                NEXUS_AudioCodec transcodeCodec;
                if (encode_display && session->audioSettings.hdmi.outputMode != NxClient_AudioOutputMode_ePcm) {
                    BDBG_WRN(("forcing hdmi to pcm output because we are encoding the display"));
                    session->audioSettings.hdmi.outputMode = NxClient_AudioOutputMode_ePcm;
                }
                if (session->audioSettings.hdmi.outputMode == NxClient_AudioOutputMode_eAuto) {
                    outputMode = config->hdmi.audioCodecOutput[i];
                    if (config->hdmiAc3Plus &&
                        server->settings.session[r->session->index].dolbyMs == nxserverlib_dolby_ms_type_ms12 &&
                        cap.dsp.codecs[NEXUS_AudioCodec_eAc3Plus].encode) {
                        /* Check for MS12 and DDP encode, as some MS12 configs are AC3 encode only */
                        transcodeCodec = NEXUS_AudioCodec_eAc3Plus;
                    }
                    else {
                        transcodeCodec = NEXUS_AudioCodec_eAc3;
                    }
                }
                else {
                    outputMode = session->audioSettings.hdmi.outputMode;
                    transcodeCodec = session->audioSettings.hdmi.transcodeCodec;
                }
                switch (outputMode) {
                case NxClient_AudioOutputMode_ePassthrough:
                    if (i==NEXUS_AudioCodec_eAc3Plus && !config->hdmiAc3Plus && session->audioSettings.hdmi.outputMode == NxClient_AudioOutputMode_eAuto) {
                        if (r->audioDecoder[nxserver_audio_decoder_primary]) {
                            /* AC3+ --> AC3 downconvert */
                            audioSettings.hdmi.input[i] = NEXUS_AudioDecoder_GetConnector(r->audioDecoder[nxserver_audio_decoder_primary], NEXUS_AudioConnectorType_eCompressed);
                            break;
                        }
                    }
                    if (r->audioDecoder[nxserver_audio_decoder_passthrough]) {
                        /* TODO: some receivers may have an issue with all DTSHD formats being sent with
                         * eCompressed16x (although the spec says it should be supported). */
                        if (i==NEXUS_AudioCodec_eDtsHd || i==NEXUS_AudioCodec_eMlp) {
                            audioSettings.hdmi.input[i] = NEXUS_AudioDecoder_GetConnector(r->audioDecoder[nxserver_audio_decoder_passthrough], NEXUS_AudioConnectorType_eCompressed16x);
                            if (audioSettings.hdmi.input[i] == NULL) {
                                /* Some older chips do not support eCompressed16x HBR passthrough. Revert
                                 * to standard compressed output and the audio block with automatically
                                 * send what is possible. */
                                audioSettings.hdmi.input[i] = NEXUS_AudioDecoder_GetConnector(r->audioDecoder[nxserver_audio_decoder_passthrough], NEXUS_AudioConnectorType_eCompressed);
                            }
                        }
                        else {
                            audioSettings.hdmi.input[i] = NEXUS_AudioDecoder_GetConnector(r->audioDecoder[nxserver_audio_decoder_passthrough], NEXUS_AudioConnectorType_eCompressed);
                        }
                    }
                    break;
                case NxClient_AudioOutputMode_eTranscode:
                    if (b_audio_dolby_ms_enabled(server->settings.session[r->session->index].dolbyMs)) {
                        audioSettings.hdmi.input[i] = NULL;
                        if ( (transcodeCodec != NEXUS_AudioCodec_eAc3 && transcodeCodec != NEXUS_AudioCodec_eAc3Plus) || !r->ddre )
                        {
                            return BERR_TRACE(NEXUS_NOT_SUPPORTED);
                        }
                        if ( server->settings.session[r->session->index].dolbyMs == nxserverlib_dolby_ms_type_ms12 )
                        {
                            /* Try AC3 Plus Encode */
                            if ( transcodeCodec == NEXUS_AudioCodec_eAc3Plus && cap.dsp.codecs[NEXUS_AudioCodec_eAc3Plus].encode )
                            {
                                if (config->hdmi.audioCodecOutput[NEXUS_AudioCodec_eAc3Plus] == NxClient_AudioOutputMode_ePassthrough && config->hdmiAc3Plus)
                                {
                                    audioSettings.hdmi.input[i] = NEXUS_DolbyDigitalReencode_GetConnector(r->ddre, NEXUS_AudioConnectorType_eCompressed4x);
                                }
                            }
                        }
                        /* Try AC3 Encode */
                        if ( audioSettings.hdmi.input[i] == NULL )
                        {
                            if ( config->hdmi.audioCodecOutput[NEXUS_AudioCodec_eAc3] == NxClient_AudioOutputMode_ePassthrough )
                            {
                                audioSettings.hdmi.input[i] = NEXUS_DolbyDigitalReencode_GetConnector(r->ddre, NEXUS_AudioConnectorType_eCompressed);
                            }
                        }
                        /* Fall back to PCM */
                        if ( audioSettings.hdmi.input[i] == NULL )
                        {
                            audioSettings.hdmi.input[i] = b_audio_get_pcm_input(r, NEXUS_AudioConnectorType_eStereo);
                        }
                        break;
                    }
                    else if (r->audioEncoder && transcodeCodec < NEXUS_AudioCodec_eMax && cap.dsp.codecs[transcodeCodec].encode) {
                        NEXUS_AudioEncoderSettings settings;
                        NEXUS_AudioEncoder_GetSettings(r->audioEncoder, &settings);
                        settings.codec = transcodeCodec;
                        rc = NEXUS_AudioEncoder_SetSettings(r->audioEncoder, &settings);
                        if (!rc) {
                            audioSettings.hdmi.input[i] = NULL;
                            if (config->hdmi.audioCodecOutput[transcodeCodec] == NxClient_AudioOutputMode_ePassthrough)
                            {
                                audioSettings.hdmi.input[i] = NEXUS_AudioEncoder_GetConnector(r->audioEncoder);
                            }
                            /* Fall back to PCM */
                            if ( audioSettings.hdmi.input[i] == NULL )
                            {
                                audioSettings.hdmi.input[i] = b_audio_get_pcm_input(r, NEXUS_AudioConnectorType_eStereo);
                            }
                            break;
                        }
                    }
                    /* else, fall through */
                default:
                case NxClient_AudioOutputMode_ePcm:
                    audioSettings.hdmi.input[i] = b_audio_get_pcm_input(r, NEXUS_AudioConnectorType_eStereo);
                    break;
                case NxClient_AudioOutputMode_eMultichannelPcm:
                    audioSettings.hdmi.input[i] = b_audio_get_pcm_input(r, NEXUS_AudioConnectorType_eMultichannel);
                    break;
                case NxClient_AudioOutputMode_eNone:
                    audioSettings.hdmi.input[i] = NULL;
                    break;
                }
            }
        }
#endif
        if (cap.numOutputs.spdif > 0) {
            /* TODO: support dual SPDIF output */
            if (r->session->index == 0) {
                audioSettings.spdif.outputs[0] = server->platformConfig.outputs.spdif[0];
            }
            if (audioSettings.spdif.outputs[0]) {
                for (i=0;i<NEXUS_AudioCodec_eMax;i++) {
                    NxClient_AudioOutputMode outputMode;
                    NEXUS_AudioCodec transcodeCodec;
                    if (encode_display && session->audioSettings.spdif.outputMode != NxClient_AudioOutputMode_ePcm) {
                        BDBG_WRN(("forcing spdif to pcm output because we are encoding the display"));
                        session->audioSettings.spdif.outputMode = NxClient_AudioOutputMode_ePcm;
                    }
                    if (session->audioSettings.spdif.outputMode == NxClient_AudioOutputMode_eAuto) {
                        outputMode = config->spdif.audioCodecOutput[i];
                        transcodeCodec = NEXUS_AudioCodec_eAc3; /* TODO */
                    }
                    else {
                        outputMode = session->audioSettings.spdif.outputMode;
                        transcodeCodec = session->audioSettings.spdif.transcodeCodec;
                    }

                    switch (outputMode) {
                    case NxClient_AudioOutputMode_ePassthrough:
                        if (i==NEXUS_AudioCodec_eAc3Plus) {
                            if (r->audioDecoder[nxserver_audio_decoder_passthrough] && server->settings.session[r->session->index].allowSpdif4xCompressed) {
                                audioSettings.spdif.input[i] = NEXUS_AudioDecoder_GetConnector(r->audioDecoder[nxserver_audio_decoder_passthrough], NEXUS_AudioConnectorType_eCompressed);
                                break;
                            }
                            else if (r->audioDecoder[nxserver_audio_decoder_primary])
                            {
                                /* AC3+ --> AC3 downconvert */
                                audioSettings.spdif.input[i] = NEXUS_AudioDecoder_GetConnector(r->audioDecoder[nxserver_audio_decoder_primary], NEXUS_AudioConnectorType_eCompressed);
                                break;
                            }
                        }
                        if (r->audioDecoder[nxserver_audio_decoder_passthrough]) {
                            audioSettings.spdif.input[i] = NEXUS_AudioDecoder_GetConnector(r->audioDecoder[nxserver_audio_decoder_passthrough], NEXUS_AudioConnectorType_eCompressed);
                        }
                        break;
                    case NxClient_AudioOutputMode_eTranscode:
                        if (b_audio_dolby_ms_enabled(server->settings.session[r->session->index].dolbyMs)) {
                            if (transcodeCodec != NEXUS_AudioCodec_eAc3 || !r->ddre) {
                                return BERR_TRACE(NEXUS_NOT_SUPPORTED);
                            }
                            audioSettings.spdif.input[i] = NEXUS_DolbyDigitalReencode_GetConnector(r->ddre, NEXUS_AudioConnectorType_eCompressed);
                            break;
                        }
                        else if (r->audioEncoder && transcodeCodec < NEXUS_AudioCodec_eMax && cap.dsp.codecs[transcodeCodec].encode) {
                            NEXUS_AudioEncoderSettings settings;
                            NEXUS_AudioEncoder_GetSettings(r->audioEncoder, &settings);
                            /* TODO: can't allow different hdmi/spdif transcode codecs with only one AudioEncoder. for now, last one wins. */
                            settings.codec = transcodeCodec;
                            rc = NEXUS_AudioEncoder_SetSettings(r->audioEncoder, &settings);
                            if (!rc) {
                                audioSettings.spdif.input[i] = NEXUS_AudioEncoder_GetConnector(r->audioEncoder);
                                break;
                            }
                        }
                        /* else, fall through */
                    default:
                    case NxClient_AudioOutputMode_ePcm:
                        audioSettings.spdif.input[i] = b_audio_get_pcm_input(r, NEXUS_AudioConnectorType_eStereo);
                        break;
                    case NxClient_AudioOutputMode_eNone:
                        audioSettings.spdif.input[i] = NULL;
                        break;
                    case NxClient_AudioOutputMode_eMultichannelPcm:
                        return BERR_TRACE(NEXUS_NOT_SUPPORTED);
                    }
                }
            }
        }
    }

    /* If we are using Dolby MS, the only conection from Decoder->DspMixer is via Multichannel
       NOTE - if no DAC is present, this may need to fall back further -- to compressed? */
    if (b_audio_dolby_ms_enabled(server->settings.session[r->session->index].dolbyMs)) {
        audioSettings.syncConnector = NEXUS_AudioConnectorType_eMultichannel;
    }
    else {
        audioSettings.syncConnector = NEXUS_AudioConnectorType_eStereo;
    }

    rc = NEXUS_SimpleAudioDecoder_SetServerSettings(session->audio.server, simpleAudioDecoder, &audioSettings);
    if (rc) return BERR_TRACE(rc);

    return 0;
}

int bserver_set_audio_mixer_config(struct b_audio_resource *r)
{
    int rc;

    if ( r->mixer[nxserver_audio_mixer_multichannel] != NULL ) {
        NEXUS_AudioMixerSettings mixerSettings;

        NEXUS_AudioMixer_GetSettings(r->mixer[nxserver_audio_mixer_multichannel], &mixerSettings);
        if ( 0 != BKNI_Memcmp(&mixerSettings.loopbackVolumeMatrix, r->session->audioSettings.loopbackVolumeMatrix, sizeof(int32_t)*NEXUS_AudioChannel_eMax*NEXUS_AudioChannel_eMax) ) {
            BKNI_Memcpy(&mixerSettings.loopbackVolumeMatrix, r->session->audioSettings.loopbackVolumeMatrix, sizeof(int32_t)*NEXUS_AudioChannel_eMax*NEXUS_AudioChannel_eMax);
            rc = NEXUS_AudioMixer_SetSettings(r->mixer[nxserver_audio_mixer_multichannel], &mixerSettings);
            if (rc) {return BERR_TRACE(rc);}
        }
    }

    return 0;
}


#if NEXUS_HAS_HDMI_OUTPUT
int bserver_hdmi_edid_audio_config(struct b_session *session, const NEXUS_HdmiOutputStatus *pStatus)
{
    unsigned i;
    nxserver_t server = session->server;
    struct b_audio_config *config = &server->audio_config;
    NEXUS_AudioCapabilities capabilities;

    NEXUS_GetAudioCapabilities(&capabilities);

    config->hdmiAc3Plus = pStatus->audioCodecSupported[NEXUS_AudioCodec_eAc3Plus];
    for (i=0;i<NEXUS_AudioCodec_eMax;i++) {
        config->hdmi.audioCodecOutput[i] = NxClient_AudioOutputMode_ePcm;
        if (pStatus->audioCodecSupported[i]) {
            switch (i) {
            case NEXUS_AudioCodec_ePcm:
                break;
            default:
                config->hdmi.audioCodecOutput[i] = NxClient_AudioOutputMode_ePassthrough;
                break;
            }
        }
        else {
            /* Special cases when native codec is not supported */
            switch(i) {
            case NEXUS_AudioCodec_eAacAdts:
            case NEXUS_AudioCodec_eAacLoas:
            case NEXUS_AudioCodec_eAacPlusAdts:
            case NEXUS_AudioCodec_eAacPlusLoas:
                if ( (capabilities.dsp.codecs[NEXUS_AudioCodec_eAc3].encode || (capabilities.dsp.dolbyDigitalReencode && b_audio_dolby_ms_enabled(server->settings.session[session->index].dolbyMs))) && pStatus->audioCodecSupported[NEXUS_AudioCodec_eAc3] ) {
                    config->hdmi.audioCodecOutput[i] = NxClient_AudioOutputMode_eTranscode;
                }
                break;
            case NEXUS_AudioCodec_eAc3Plus:
                /* downconvert DDP to AC3 if receiver only supports the latter.
                both AC3+ --> AC3+ and AC3+ --> AC3 use ePassthrough, but we prefer AC3+ over AC3 if EDID confirms it is possible (see config->hdmiAc3Plus). */
                if (pStatus->audioCodecSupported[NEXUS_AudioCodec_eAc3]) {
                    config->hdmi.audioCodecOutput[i] = NxClient_AudioOutputMode_ePassthrough;
                }
                break;
            case NEXUS_AudioCodec_eDtsHd:
                /* DTS is layered so the RX should be able to handle just the baselayer out HDMI */
                if (pStatus->audioCodecSupported[NEXUS_AudioCodec_eDts]) {
                    config->hdmi.audioCodecOutput[i] = NxClient_AudioOutputMode_ePassthrough;
                }
                break;
            default:
                break;
            }
        }
    }
    return 0;
}
#endif

int audio_init(nxserver_t server)
{
    NEXUS_AudioOutputEnabledOutputs outputs;
    NEXUS_AudioOutputClockConfig config;
    NEXUS_AudioOutputSettings outputSettings;
    unsigned i;
    NEXUS_AudioDecoderHandle audio;
    int rc;
    NEXUS_AudioCapabilities cap;
    unsigned timingResourcesAvailable, timingResourcesUsed = 0;

    audio = NEXUS_AudioDecoder_Open(0, NULL);
    if (!audio) return BERR_TRACE(NEXUS_NOT_AVAILABLE);
    NEXUS_SimpleAudioDecoderModule_LoadDefaultSettings(audio);
    NEXUS_AudioDecoder_Close(audio);

    NEXUS_AudioOutput_GetDefaultEnabledOutputs(&outputs);
    NEXUS_GetAudioCapabilities(&cap);
    timingResourcesAvailable = cap.numPlls + cap.numNcos;
    /* SPDIF/HDMI use separate PLL/NCO because of possible transcode or DDRE.
    Dummy outputs start after SPDIF/HDMI.
    */
    timingResourcesUsed++; /* dac */
    for(i=0;i<cap.numOutputs.spdif;i++) {
        if (timingResourcesUsed <= timingResourcesAvailable) {
            outputs.spdif[i] = timingResourcesUsed++;
        }
    }
    for(i=0;i<cap.numOutputs.hdmi;i++) {
        if (timingResourcesUsed <= timingResourcesAvailable) {
            outputs.hdmi[i] = timingResourcesUsed++;
        }
    }
    for (i=0; i<cap.numOutputs.dummy;i++) {
        if (timingResourcesUsed <= timingResourcesAvailable) {
            outputs.audioDummy[i] = timingResourcesUsed++;
        }
    }
    for (i=0; i<cap.numOutputs.capture;i++) {
        if (timingResourcesUsed <= timingResourcesAvailable) {
            outputs.audioCapture[i] = timingResourcesUsed++;
        }
        else /* If there isn't a resource available for the capture use spdif or dummy */
        {
            if (i < cap.numOutputs.spdif && cap.numOutputs.spdif > 0) {
                outputs.audioCapture[i] = outputs.spdif[i];
            }
            else if (cap.numOutputs.spdif > 0) {
                outputs.audioCapture[i] = outputs.spdif[0];
            }
            else if (i < cap.numOutputs.dummy && cap.numOutputs.dummy > 0) {
                outputs.audioCapture[i] = outputs.audioDummy[i];
            }
            else if (cap.numOutputs.dummy > 0) {
                outputs.audioCapture[i] = outputs.audioDummy[0];
            }
        }
    }
    rc = NEXUS_AudioOutput_CreateClockConfig(&outputs, &config);
    if (rc) return BERR_TRACE(rc);

    /* if we exceed timingResourcesAvailable, leave resource in default configuration. */
    for (i=0;i<cap.numOutputs.spdif;i++) {
        if (config.spdif[i].pll != NEXUS_AudioOutputPll_eMax) {
            NEXUS_AudioOutput_GetSettings(NEXUS_SpdifOutput_GetConnector(server->platformConfig.outputs.spdif[i]), &outputSettings);
            outputSettings.pll = config.spdif[i].pll;
            /* spdif doesn't have nco */
            NEXUS_AudioOutput_SetSettings(NEXUS_SpdifOutput_GetConnector(server->platformConfig.outputs.spdif[i]), &outputSettings);
        }
    }
#if NEXUS_NUM_HDMI_OUTPUTS
    for(i=0;i<cap.numOutputs.hdmi;i++) {
        if (config.hdmi[i].pll != NEXUS_AudioOutputPll_eMax || config.hdmi[i].nco != NEXUS_AudioOutputNco_eMax) {
            NEXUS_AudioOutput_GetSettings(NEXUS_HdmiOutput_GetAudioConnector(server->platformConfig.outputs.hdmi[i]), &outputSettings);
            outputSettings.pll = config.hdmi[i].pll;
            outputSettings.nco = config.hdmi[i].nco;
            NEXUS_AudioOutput_SetSettings(NEXUS_HdmiOutput_GetAudioConnector(server->platformConfig.outputs.hdmi[i]), &outputSettings);
        }
    }
#endif
    for(i=0;i<cap.numOutputs.dummy;i++) {
        if (config.audioDummy[i].pll != NEXUS_AudioOutputPll_eMax || config.audioDummy[i].nco != NEXUS_AudioOutputNco_eMax) {
            NEXUS_AudioOutput_GetSettings(NEXUS_AudioDummyOutput_GetConnector(server->platformConfig.outputs.audioDummy[i]), &outputSettings);
            outputSettings.pll = config.audioDummy[i].pll;
            outputSettings.nco = config.audioDummy[i].nco;
            NEXUS_AudioOutput_SetSettings(NEXUS_AudioDummyOutput_GetConnector(server->platformConfig.outputs.audioDummy[i]), &outputSettings);
        }
     }
     BKNI_Memcpy(&server->settings.audioCapture.clockSources, &config.audioCapture, sizeof(NEXUS_AudioOutputClockSource)*NEXUS_MAX_AUDIO_CAPTURE_OUTPUTS);

     bserver_init_audio_config(server);
     return 0;
}

void audio_uninit(void)
{
}

void audio_get_encode_resources(struct b_audio_resource *r, NEXUS_AudioMixerHandle *pMixer, NEXUS_SimpleAudioDecoderHandle *pMaster, NEXUS_SimpleAudioDecoderHandle *pSlave)
{
    *pMixer = r->mixer[nxserver_audio_mixer_stereo];
    if (pMaster) *pMaster = r->masterSimpleAudioDecoder;
    if (pSlave) *pSlave = b_audio_get_decoder(r->connect);
}

/* Return the SimpleStcChannel's stcIndex currently in use.
If none in use, return -1. */
int audio_get_stc_index(struct b_connect *connect)
{
    int stcIndex = -1;
    NEXUS_SimpleAudioDecoderHandle audioDecoder = b_audio_get_decoder(connect);
    if (audioDecoder) {
        NEXUS_SimpleAudioDecoder_GetStcIndex(connect->client->session->audio.server, audioDecoder, &stcIndex);
    }
    return stcIndex;
}

static void nxserverlib_configure_audio_capture(struct b_session *session, NEXUS_SimpleAudioDecoderServerSettings *sessionSettings, NxClient_AudioCaptureType captureType)
{
    /* set up per codec outputs for Capture */
    int i;
    int rc;
    NEXUS_AudioCapabilities capabilities;
    NEXUS_GetAudioCapabilities(&capabilities);

    for (i=0;i<NEXUS_AudioCodec_eMax;i++) {
        NxClient_AudioOutputMode outputMode;
        NEXUS_AudioCodec transcodeCodec;

        if (captureType == NxClient_AudioCaptureType_eCompressed4x) {
            if (session->server->settings.session[session->index].dolbyMs == nxserverlib_dolby_ms_type_ms12 &&
                capabilities.dsp.codecs[NEXUS_AudioCodec_eAc3Plus].encode) {
                /* Check for MS12 and DDP encode, as some MS12 configs are AC3 encode only */
                transcodeCodec = NEXUS_AudioCodec_eAc3Plus;
                outputMode = NxClient_AudioOutputMode_eTranscode;
            }
            else if (b_audio_dolby_ms_enabled(session->server->settings.session[session->index].dolbyMs)) {
                transcodeCodec = NEXUS_AudioCodec_eAc3;
                outputMode = NxClient_AudioOutputMode_eTranscode;
            }
            else
            {
                outputMode = NxClient_AudioOutputMode_ePassthrough;
            }
        }
        else if (captureType == NxClient_AudioCaptureType_eCompressed) {
            if (b_audio_dolby_ms_enabled(session->server->settings.session[session->index].dolbyMs)) {
                transcodeCodec = NEXUS_AudioCodec_eAc3;
                outputMode = NxClient_AudioOutputMode_eTranscode;
            }
            else if (i==NEXUS_AudioCodec_eAacAdts ||
                     i==NEXUS_AudioCodec_eAacLoas ||
                     i==NEXUS_AudioCodec_eAacPlusAdts ||
                     i==NEXUS_AudioCodec_eAacPlusLoas) {
                transcodeCodec = NEXUS_AudioCodec_eAc3;
                outputMode = NxClient_AudioOutputMode_eTranscode;
            }
            else
            {
                outputMode = NxClient_AudioOutputMode_ePassthrough;
            }
        }
        else if (captureType == NxClient_AudioCaptureType_e24Bit5_1) {
            outputMode = NxClient_AudioOutputMode_eMultichannelPcm;
        }
        else
        {
            outputMode = NxClient_AudioOutputMode_ePcm;
        }

        switch (outputMode) {
        case NxClient_AudioOutputMode_eTranscode:
            if (b_audio_dolby_ms_enabled(session->server->settings.session[session->index].dolbyMs)) {
                if (transcodeCodec == NEXUS_AudioCodec_eAc3Plus) {
                    sessionSettings->capture.input[i] = b_audio_get_compressed_input(session->main_audio, NEXUS_AudioConnectorType_eCompressed4x);
                    break;
                }
                else
                {
                    sessionSettings->capture.input[i] = b_audio_get_compressed_input(session->main_audio, NEXUS_AudioConnectorType_eCompressed);
                    break;
                }
            }
            else if (session->main_audio->audioEncoder && transcodeCodec < NEXUS_AudioCodec_eMax && capabilities.dsp.codecs[transcodeCodec].encode) {
                NEXUS_AudioEncoderSettings settings;
                NEXUS_AudioEncoder_GetSettings(session->main_audio->audioEncoder, &settings);
                settings.codec = transcodeCodec;
                rc = NEXUS_AudioEncoder_SetSettings(session->main_audio->audioEncoder, &settings);
                if (!rc) {
                    sessionSettings->capture.input[i] = NEXUS_AudioEncoder_GetConnector(session->main_audio->audioEncoder);
                    break;

                }
            }
            /* else, fall through */
        case NxClient_AudioOutputMode_ePassthrough:
            if (i == NEXUS_AudioCodec_eAc3Plus &&
                captureType == NxClient_AudioCaptureType_eCompressed) {
                sessionSettings->capture.input[i] = b_audio_get_compressed_input(session->main_audio, NEXUS_AudioConnectorType_eCompressed);
            }
            else {
                sessionSettings->capture.input[i] = b_audio_get_compressed_input(session->main_audio, NEXUS_AudioConnectorType_eCompressed4x);
            }
            break;
        default:
        case NxClient_AudioOutputMode_ePcm:
            sessionSettings->capture.input[i] = b_audio_get_pcm_input(session->main_audio, NEXUS_AudioConnectorType_eStereo);
            break;
        case NxClient_AudioOutputMode_eMultichannelPcm:
            sessionSettings->capture.input[i] = b_audio_get_pcm_input(session->main_audio, NEXUS_AudioConnectorType_eMultichannel);
            break;
        }
    }
    return;
}

static struct {
    struct b_session *session;
    NEXUS_AudioCaptureHandle handle;
} g_capture[NEXUS_MAX_AUDIO_CAPTURE_OUTPUTS];

NEXUS_AudioCaptureHandle nxserverlib_open_audio_capture(struct b_session *session, unsigned *id, NxClient_AudioCaptureType captureType)
{
    NEXUS_AudioCaptureHandle handle;
    NEXUS_AudioCaptureOpenSettings settings;
    int rc;
    unsigned i;
    NEXUS_SimpleAudioDecoderHandle audioDecoder;
    NEXUS_SimpleAudioDecoderServerSettings sessionSettings;
    NEXUS_AudioOutputSettings outputSettings;
    NEXUS_AudioCapabilities audioCapabilities;

    NEXUS_GetAudioCapabilities(&audioCapabilities);

    if (!session->main_audio->mixer[nxserver_audio_mixer_stereo] && !session->main_audio->mixer[nxserver_audio_mixer_multichannel]) {
        BDBG_ERR(("no mixers in this session"));
        return NULL;
    }

    for (i=0;i<audioCapabilities.numOutputs.capture;i++) {
        if (!g_capture[i].handle) break;
    }
    if (i == audioCapabilities.numOutputs.capture) {
        BDBG_ERR(("no audio captures left %d > %d", i, audioCapabilities.numOutputs.capture));
        return NULL;
    }
    NEXUS_AudioCapture_GetDefaultOpenSettings(&settings);
    settings.heap = session->server->settings.client.heap[NXCLIENT_FULL_HEAP];
    if (captureType  == NxClient_AudioCaptureType_e24BitStereo) {
        settings.format = NEXUS_AudioCaptureFormat_e24BitStereo;
    }
    else if (captureType  == NxClient_AudioCaptureType_e24Bit5_1) {
        settings.multichannelFormat = NEXUS_AudioMultichannelFormat_e5_1;
        settings.format = NEXUS_AudioCaptureFormat_e24Bit5_1;
        settings.fifoSize *= 3;  /* Increase FIFO for 6 channels instead of stereo */
    }

    handle = NEXUS_AudioCapture_Open(i, &settings);
    if (!handle) {
        BDBG_ERR(("unable to open audio capture %d", i));
        return NULL;
    }

    audioDecoder = b_audio_get_active_decoder(session->main_audio);

    NEXUS_SimpleAudioDecoder_Suspend(audioDecoder);

    NEXUS_AudioOutput_GetSettings(NEXUS_AudioCapture_GetConnector(handle), &outputSettings);
    outputSettings.pll = session->server->settings.audioCapture.clockSources[i].pll;
    outputSettings.nco = session->server->settings.audioCapture.clockSources[i].nco;
    NEXUS_AudioOutput_SetSettings(NEXUS_AudioCapture_GetConnector(handle), &outputSettings);

    NEXUS_SimpleAudioDecoder_GetServerSettings(session->audio.server, audioDecoder, &sessionSettings);
    sessionSettings.capture.output = handle;
    nxserverlib_configure_audio_capture(session, &sessionSettings, captureType);
    rc = NEXUS_SimpleAudioDecoder_SetServerSettings(session->audio.server, audioDecoder, &sessionSettings);
    if ( rc ) goto err_add_mixer;

    NEXUS_SimpleAudioDecoder_Resume(audioDecoder);

    g_capture[i].session = session;
    g_capture[i].handle = handle;
    *id = i;
    return handle;

err_add_mixer:
    NEXUS_AudioCapture_Close(handle);
    NEXUS_SimpleAudioDecoder_Resume(audioDecoder);
    return NULL;
}

void nxserverlib_close_audio_capture(struct b_session *session,unsigned id)
{
    NEXUS_SimpleAudioDecoderHandle audioDecoder;
    NEXUS_SimpleAudioDecoderServerSettings sessionSettings;
    NEXUS_AudioCapabilities audioCapabilities;
    NEXUS_GetAudioCapabilities(&audioCapabilities);

    if (id < audioCapabilities.numOutputs.capture) {
        BDBG_ASSERT(g_capture[id].handle);

        audioDecoder = b_audio_get_active_decoder(session->main_audio);
        NEXUS_SimpleAudioDecoder_Suspend(audioDecoder);
        NEXUS_AudioOutput_RemoveAllInputs(NEXUS_AudioCapture_GetConnector(g_capture[id].handle));
        NEXUS_SimpleAudioDecoder_GetServerSettings(session->audio.server, audioDecoder, &sessionSettings);
        sessionSettings.capture.output = NULL;
        NEXUS_SimpleAudioDecoder_SetServerSettings(session->audio.server, audioDecoder, &sessionSettings);

        NEXUS_AudioCapture_Close(g_capture[id].handle);
        g_capture[id].handle = NULL;

        NEXUS_SimpleAudioDecoder_Resume(audioDecoder);
    }
}


static struct {
    struct b_session *session;
    NEXUS_AudioCrcHandle handle;
} g_crc[NxClient_AudioCrcType_eMax];

NEXUS_AudioCrcHandle nxserverlib_open_audio_crc(struct b_session *session, unsigned *id, NxClient_AudioCrcType crcType)
{
    NEXUS_AudioCrcHandle handle;
    NEXUS_AudioCrcOpenSettings openSettings;
    NEXUS_AudioCrcInputSettings inputSettings;
    NEXUS_SimpleAudioDecoderHandle audioDecoder;
    NEXUS_AudioCapabilities cap;

    NEXUS_GetAudioCapabilities(&cap);

    if (cap.numCrcs > 0)
    {
        if (!session->main_audio->mixer[nxserver_audio_mixer_stereo] && !session->main_audio->mixer[nxserver_audio_mixer_multichannel]) {
            BDBG_ERR(("No mixers in this session"));
            return NULL;
        }

        if (crcType < NxClient_AudioCrcType_eMax) {
            if (g_crc[crcType].handle) {
                BDBG_ERR(("CRC already opened"));
                return NULL;
            }

            NEXUS_AudioCrc_GetDefaultOpenSettings (&openSettings);

            openSettings.dataWidth = 24;
            openSettings.initialValue = 0;
            openSettings.captureMode = NEXUS_AudioCrcMode_eFreeRun;
            openSettings.numEntries = 48000/256;
            openSettings.samplingPeriod = 48000/2;
            if (crcType == NxClient_AudioCrcType_eMultichannel) {
    #if NEXUS_NUM_HDMI_OUTPUTS
                if (cap.numOutputs.hdmi > 0) {
                    if (session->audioSettings.hdmi.outputMode != NxClient_AudioOutputMode_eMultichannelPcm)
                    {
                        BDBG_ERR(("No outputs configured for multichannel, set hdmi for multichannel before enabling crc"));
                        return NULL;
                    }

                    if (!session->hdmiOutput) {
                        BDBG_ERR(("Session does not contain an HDMI output"));
                        return NULL;
                    }
                    openSettings.numChannelPairs = 3;
                }
                else {
                    BDBG_ERR(("HDMI is not enabled so there is no multichannel output capable of CRC"));
                    return NULL;
                }
    #endif
            }
            else {
                openSettings.numChannelPairs = 1;
            }

            handle = NEXUS_AudioCrc_Open(crcType, &openSettings);
            if (!handle) {
                BDBG_ERR(("unable to open audio capture %d", crcType));
                return NULL;
            }

            NEXUS_AudioCrc_GetDefaultInputSettings(&inputSettings);
            inputSettings.sourceType = NEXUS_AudioCrcSourceType_eOutputPort;
            if (crcType == NxClient_AudioCrcType_eMultichannel) {
                inputSettings.output = NEXUS_HdmiOutput_GetAudioConnector(session->hdmiOutput);
            }
            else {
                if (cap.numOutputs.dac > 0) {
                    inputSettings.output = NEXUS_AudioDac_GetConnector(session->server->platformConfig.outputs.audioDacs[0]);
                }
                else if (cap.numOutputs.i2s > 0) {
                    inputSettings.output = NEXUS_I2sOutput_GetConnector(session->server->platformConfig.outputs.i2s[0]);
                }
                else if (cap.numOutputs.spdif > 0) {
                    if (session->audioSettings.spdif.outputMode == NxClient_AudioOutputMode_ePcm) {
                        inputSettings.output = NEXUS_SpdifOutput_GetConnector(session->server->platformConfig.outputs.spdif[0]);
                    }
                }
                if (inputSettings.output == NULL) {
    #if NEXUS_NUM_HDMI_OUTPUTS
                    if (cap.numOutputs.hdmi > 0) {
                        if (session->audioSettings.hdmi.outputMode == NxClient_AudioOutputMode_ePcm) {
                            inputSettings.output = NEXUS_HdmiOutput_GetAudioConnector(session->hdmiOutput);
                        }
                        else {
                            BDBG_ERR(("No stereo outputs for the stereo crc"));
                            goto err_add_output;
                        }
                    }
                    else {
                        BDBG_ERR(("No stereo outputs for the stereo crc"));
                        goto err_add_output;
                    }
    #else
                    BDBG_ERR(("No stereo outputs for the stereo crc"));
                    goto err_add_output;
    #endif
                }
            }

            NEXUS_AudioCrc_SetInput(handle, &inputSettings);

            audioDecoder = b_audio_get_active_decoder(session->main_audio);

            NEXUS_SimpleAudioDecoder_Suspend(audioDecoder);
            NEXUS_SimpleAudioDecoder_Resume(audioDecoder);

            g_crc[crcType].session = session;
            g_crc[crcType].handle = handle;
            *id = crcType;
            return handle;

        err_add_output:
            NEXUS_AudioCrc_Close(handle);
            return NULL;
        }
        else {
            BDBG_ERR(("invalid CRC capture type %d", crcType));
            return NULL;
        }
    }
    else
    {
        BDBG_ERR(("Platform does not support audio CRC capture"));
    }
    return NULL;
}

void nxserverlib_close_audio_crc(struct b_session *session,unsigned id)
{
    NEXUS_SimpleAudioDecoderHandle audioDecoder;
    NEXUS_AudioCapabilities cap;

    NEXUS_GetAudioCapabilities(&cap);

    if (cap.numCrcs > 0)
    {
        BDBG_ASSERT(id < NxClient_AudioCrcType_eMax && g_crc[id].handle);

        audioDecoder = b_audio_get_active_decoder(session->main_audio);
        NEXUS_SimpleAudioDecoder_Suspend(audioDecoder);

        NEXUS_AudioCrc_ClearInput(g_crc[id].handle);
        NEXUS_AudioCrc_Close(g_crc[id].handle);
        g_crc[id].handle = NULL;

        NEXUS_SimpleAudioDecoder_Resume(audioDecoder);
    }
}

void nxserverlib_p_restart_audio(struct b_session *session)
{
    NEXUS_SimpleAudioDecoderHandle audioDecoder;
    audioDecoder = b_audio_get_active_decoder(session->main_audio);
    NEXUS_SimpleAudioDecoder_Suspend(audioDecoder);
    NEXUS_SimpleAudioDecoder_Resume(audioDecoder);
}

int nxserverlib_audio_get_status(struct b_session *session, NxClient_AudioStatus *pStatus)
{
    enum nxserverlib_dolby_ms_type dolbyMs;
    NEXUS_AudioCodec currentCodec = NEXUS_AudioCodec_ePcm;
    NEXUS_AudioDecoderStatus decoderStatus;
    int rc;
    struct b_audio_config *config = &session->server->audio_config;

    BKNI_Memset(pStatus, 0, sizeof(*pStatus));
    if (!session->main_audio) {
        return 0;
    }
    if (!session->main_audio->localSession) {
        return 0;
    }
    if (session->index != 0) {
        /* TODO: second local session. need per-session NxClient_AudioSettings. */
        return 0;
    }

    rc = NEXUS_SimpleAudioDecoder_GetStatus(b_audio_get_active_decoder(session->main_audio), &decoderStatus);
    if (!rc && decoderStatus.started) {
        currentCodec = decoderStatus.codec;
    }

    if (session->audioSettings.hdmi.outputMode == NxClient_AudioOutputMode_eAuto) {
        pStatus->hdmi.outputMode = config->hdmi.audioCodecOutput[currentCodec];
    }
    else {
        pStatus->hdmi.outputMode = session->audioSettings.hdmi.outputMode;
    }
    pStatus->hdmi.channelMode = session->audioSettings.hdmi.channelMode;
    switch (pStatus->hdmi.outputMode) {
    case NxClient_AudioOutputMode_ePassthrough: pStatus->hdmi.outputCodec = (currentCodec==NEXUS_AudioCodec_ePcm)?NEXUS_AudioCodec_eUnknown:currentCodec; break;
    case NxClient_AudioOutputMode_eTranscode:   pStatus->hdmi.outputCodec = session->audioSettings.hdmi.transcodeCodec; break;
    case NxClient_AudioOutputMode_eAuto: BDBG_ERR(("unexpected hdmi NxClient_AudioOutputMode_eAuto state")); /* fall through */
    default: pStatus->hdmi.outputCodec = NEXUS_AudioCodec_ePcm; break;
    }

    if (session->audioSettings.spdif.outputMode == NxClient_AudioOutputMode_eAuto) {
        pStatus->spdif.outputMode = config->spdif.audioCodecOutput[currentCodec];
    }
    else {
        pStatus->spdif.outputMode = session->audioSettings.spdif.outputMode;
    }
    pStatus->spdif.channelMode = session->audioSettings.spdif.channelMode;
    switch (pStatus->spdif.outputMode) {
    case NxClient_AudioOutputMode_ePassthrough: pStatus->spdif.outputCodec = (currentCodec==NEXUS_AudioCodec_ePcm)?NEXUS_AudioCodec_eUnknown:currentCodec; break;
    case NxClient_AudioOutputMode_eTranscode:   pStatus->spdif.outputCodec = session->audioSettings.spdif.transcodeCodec; break;
    case NxClient_AudioOutputMode_eAuto: BDBG_ERR(("unexpected spdif NxClient_AudioOutputMode_eAuto state")); /* fall through */
    default: pStatus->spdif.outputCodec = NEXUS_AudioCodec_ePcm; break;
    }

    pStatus->dac.outputMode = NxClient_AudioOutputMode_ePcm;
    pStatus->dac.channelMode = session->audioSettings.dac.channelMode;
    pStatus->dac.outputCodec = NEXUS_AudioCodec_ePcm;

    pStatus->rfm.outputMode = NxClient_AudioOutputMode_ePcm;
    pStatus->rfm.channelMode = session->audioSettings.rfm.channelMode;
    pStatus->rfm.outputCodec = NEXUS_AudioCodec_ePcm;

    dolbyMs = session->server->settings.session[session->index].dolbyMs;
    pStatus->dolbySupport.ddre = (dolbyMs == nxserverlib_dolby_ms_type_ms11 || dolbyMs == nxserverlib_dolby_ms_type_ms12);
    pStatus->dolbySupport.mixer = (dolbyMs == nxserverlib_dolby_ms_type_ms12);
    pStatus->dolbySupport.dolbyVolume258 = (dolbyMs == nxserverlib_dolby_ms_type_ms11);

    return 0;
}

void nxserverlib_p_audio_get_audio_procesing_settings(struct b_session *session, NxClient_AudioProcessingSettings *pSettings)
{
    *pSettings = session->audioProcessingSettings;
    if (session->server->settings.session[session->index].dolbyMs == nxserverlib_dolby_ms_type_ms12)
    {
            NEXUS_AudioMixerSettings mixerSettings;
            NEXUS_AudioMixer_GetSettings(session->main_audio->mixer[nxserver_audio_mixer_multichannel], &mixerSettings);
            BKNI_Memcpy(&pSettings->dolby.dolbySettings, &mixerSettings.dolby, sizeof(NEXUS_AudioMixerDolbySettings));
    }
}

int  nxserverlib_p_audio_set_audio_procesing_settings(struct b_session *session, const NxClient_AudioProcessingSettings *pSettings)
{
    int rc;
    if (!session->main_audio) {
        return BERR_TRACE(NEXUS_NOT_AVAILABLE);
    }
    if ((pSettings->avl.enabled && session->main_audio->avl) || !pSettings->avl.enabled) {
        if (session->main_audio->avl) {
            NEXUS_AutoVolumeLevelSettings currentAVLSettings;
            NEXUS_AutoVolumeLevel_GetSettings(session->main_audio->avl, &currentAVLSettings);
            if ( 0 != BKNI_Memcmp(&currentAVLSettings, &pSettings->avl, sizeof(NEXUS_AutoVolumeLevelSettings)) ) {
                rc = NEXUS_AutoVolumeLevel_SetSettings(session->main_audio->avl, &pSettings->avl);
                if (rc) return BERR_TRACE(rc);
            }
        }
    }
    else {
        return BERR_TRACE(NEXUS_NOT_AVAILABLE);
    }
    if ((pSettings->truVolume.enabled && session->main_audio->truVolume) || !pSettings->truVolume.enabled) {
        if (session->main_audio->truVolume) {
            NEXUS_TruVolumeSettings currentTruVolumeSettings;
            NEXUS_TruVolume_GetSettings(session->main_audio->truVolume, &currentTruVolumeSettings);
            if ( 0 != BKNI_Memcmp(&currentTruVolumeSettings, &pSettings->truVolume, sizeof(NEXUS_TruVolumeSettings)) ) {
                rc = NEXUS_TruVolume_SetSettings(session->main_audio->truVolume, &pSettings->truVolume);
                if (rc) return BERR_TRACE(rc);
            }
        }
    }
    else {
        return BERR_TRACE(NEXUS_NOT_AVAILABLE);
    }
    if ((pSettings->dolby.dolbyVolume258.enabled && session->main_audio->dolbyVolume258) || !pSettings->dolby.dolbyVolume258.enabled) {
        if (session->main_audio->dolbyVolume258) {
            NEXUS_DolbyVolume258Settings currentDolbyVolumeSettings;
            NEXUS_DolbyVolume258_GetSettings(session->main_audio->dolbyVolume258, &currentDolbyVolumeSettings);
            if ( 0 != BKNI_Memcmp(&currentDolbyVolumeSettings, &pSettings->dolby.dolbyVolume258, sizeof(NEXUS_DolbyVolume258Settings)) ) {
                rc = NEXUS_DolbyVolume258_SetSettings(session->main_audio->dolbyVolume258, &pSettings->dolby.dolbyVolume258);
                if (rc) return BERR_TRACE(rc);
            }
        }
    }
    else {
        return BERR_TRACE(NEXUS_NOT_AVAILABLE);
    }
    if (session->main_audio->ddre) {
        NEXUS_DolbyDigitalReencodeSettings currentDDRESettings;
        NEXUS_DolbyDigitalReencode_GetSettings(session->main_audio->ddre, &currentDDRESettings);
        if ( 0 != BKNI_Memcmp(&currentDDRESettings, &pSettings->dolby.ddre, sizeof(NEXUS_DolbyDigitalReencodeSettings)) ) {
            rc = NEXUS_DolbyDigitalReencode_SetSettings(session->main_audio->ddre, &pSettings->dolby.ddre);
            if (rc) return BERR_TRACE(rc);
        }
    }
    if (session->server->settings.session[session->index].dolbyMs == nxserverlib_dolby_ms_type_ms12)
    {
        NEXUS_AudioMixerSettings mixerSettings;
        NEXUS_AudioMixer_GetSettings(session->main_audio->mixer[nxserver_audio_mixer_multichannel], &mixerSettings);
        if ( 0 != BKNI_Memcmp(&mixerSettings.dolby, &pSettings->dolby.dolbySettings, sizeof(NEXUS_AudioMixerDolbySettings)) ) {
            BKNI_Memcpy(&mixerSettings.dolby, &pSettings->dolby.dolbySettings, sizeof(NEXUS_AudioMixerDolbySettings));
            NEXUS_AudioMixer_SetSettings(session->main_audio->mixer[nxserver_audio_mixer_multichannel], &mixerSettings);
        }
    }
    session->audioProcessingSettings = *pSettings;
    return 0;
}

static int swap_persistent_audio_decoders(struct b_connect *destConnect, struct b_connect *srcConnect)
{

    NEXUS_SimpleAudioDecoderServerSettings settings;
    NEXUS_SimpleAudioDecoderHandle srcDecoder, destDecoder;
    struct b_session *session;
    nxserver_t server;
    struct b_req *destReq = destConnect->req[b_resource_simple_audio_decoder];
    struct b_req *srcReq = srcConnect->req[b_resource_simple_audio_decoder];
    struct b_audio_resource *r;
    int rc;


    if ( destConnect->settings.simpleAudioDecoder.decoderCapabilities.type != NxClient_AudioDecoderType_ePersistent ||
         srcConnect->settings.simpleAudioDecoder.decoderCapabilities.type != NxClient_AudioDecoderType_ePersistent ) {
         BDBG_ERR(("swap_persistent_audio_decoders requires persistent decoders"));
         return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }

    if (srcReq->handles.simpleAudioDecoder.r) {
        if (srcReq->handles.simpleAudioDecoder.r->connect != srcConnect) {
            BDBG_ERR(("already connected to %p", (void*)destReq->handles.simpleAudioDecoder.r->connect));
            return BERR_TRACE(NEXUS_INVALID_PARAMETER);
        }
        r = srcReq->handles.simpleAudioDecoder.r;
        session = r->session;
        server = r->session->server;
    }
    else
    {
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }

    srcDecoder = b_audio_get_decoder(destConnect);
    destDecoder = b_audio_get_decoder(srcConnect);
    audio_release_stc_index(srcConnect, srcDecoder);
    BDBG_MSG(("transfer audio %p: connect %p(%p) -> connect %p(%p)",
              (void*)r, (void*)srcConnect, (void*)srcDecoder,
              (void*)destConnect, (void*)destDecoder));
    rc = NEXUS_SimpleAudioDecoder_SwapServerSettings(session->audio.server, srcDecoder, destDecoder);
    if (rc) {BERR_TRACE(rc); goto err_setsettings;}

    rc = audio_acquire_stc_index(destConnect, destDecoder);
    if (rc) { BERR_TRACE(rc); goto err_setsettings;}
    destReq->handles.simpleAudioDecoder.r = r;
    destReq->handles.simpleAudioDecoder.r->connect = destConnect;
    srcReq->handles.simpleAudioDecoder.r = NULL;
    return 0;

err_setsettings:
    {
        int i;
        NEXUS_SimpleAudioDecoderHandle activeDecoder;
        activeDecoder = b_audio_get_active_decoder(session->main_audio);

        NEXUS_SimpleAudioDecoder_GetServerSettings(session->audio.server, activeDecoder, &settings);
        for (i = 0; i < NEXUS_MAX_AUDIO_DECODERS; i++) {
            if (settings.persistent[i].decoder == r->audioDecoder[nxserver_audio_decoder_primary]) {
                settings.persistent[i].decoder = NULL;
                settings.persistent[i].suspended = false;
                break;
            }
        }
        NEXUS_SimpleAudioDecoder_SetServerSettings(session->audio.server, activeDecoder, &settings);

        NEXUS_SimpleAudioDecoder_GetServerSettings(session->audio.server, srcDecoder, &settings);
        settings.primary = NULL;
        (void)NEXUS_SimpleAudioDecoder_SetServerSettings(session->audio.server, srcDecoder, &settings);

        NEXUS_SimpleAudioDecoder_GetServerSettings(session->audio.server, destDecoder, &settings);
        settings.primary = NULL;
        (void)NEXUS_SimpleAudioDecoder_SetServerSettings(session->audio.server, destDecoder, &settings);
        audio_decoder_destroy(r);
    }
    return rc;

}

int nxserverlib_p_swap_audio(struct b_connect *connect1, struct b_connect *connect2)
{
    struct b_session *session = connect1->client->session;
    if (session->main_audio->connect == connect1) {
        return acquire_audio_decoders(connect2, true);
    }
    else if (session->main_audio->connect == connect2) {
        return acquire_audio_decoders(connect1, true);
    }
    else {
        if (connect1->settings.simpleAudioDecoder.decoderCapabilities.type == NxClient_AudioDecoderType_ePersistent ||
            connect2->settings.simpleAudioDecoder.decoderCapabilities.type == NxClient_AudioDecoderType_ePersistent) {
            return swap_persistent_audio_decoders(connect1, connect2);
        }
        else {
            /* neither has it, so it's a no-op */
            return 0;
        }
    }
}
#else
int acquire_audio_playbacks(struct b_connect *connect) { BSTD_UNUSED(connect);return 0; }
bool has_audio(struct b_connect *connect) { BSTD_UNUSED(connect);return false; }
bool lacks_audio(struct b_connect *connect) { BSTD_UNUSED(connect);return true; }
int nxserverlib_audio_get_status(struct b_session *session, NxClient_AudioStatus *pStatus) { BSTD_UNUSED(session);BSTD_UNUSED(pStatus);return 0; }
void nxserverlib_p_audio_get_audio_procesing_settings(struct b_session *session, NxClient_AudioProcessingSettings *pSettings) {BSTD_UNUSED(session);BSTD_UNUSED(pSettings); }
int  nxserverlib_p_audio_set_audio_procesing_settings(struct b_session *session, const NxClient_AudioProcessingSettings *pSettings) { BSTD_UNUSED(session);BSTD_UNUSED(pSettings);return 0; }
void release_audio_playbacks(struct b_connect *connect) {BSTD_UNUSED(connect); }
int acquire_audio_decoders(struct b_connect *connect, bool force_grab) { BSTD_UNUSED(connect);BSTD_UNUSED(force_grab);return 0; }
void bserver_acquire_audio_mixers(struct b_audio_resource *r, bool start) {BSTD_UNUSED(r);BSTD_UNUSED(start); }
struct b_audio_resource *audio_decoder_create(struct b_session *session, enum b_audio_decoder_type type) { BSTD_UNUSED(session);BSTD_UNUSED(type);return NULL; }
void audio_decoder_destroy(struct b_audio_resource *r) {BSTD_UNUSED(r); }
int audio_init(nxserver_t server) { BSTD_UNUSED(server);return 0; }
void release_audio_decoders(struct b_connect *connect) {BSTD_UNUSED(connect); }
int nxserverlib_p_swap_audio(struct b_connect *connect1, struct b_connect *connect2) { BSTD_UNUSED(connect1);BSTD_UNUSED(connect2);return 0; }
void audio_uninit(void) { }
#if NEXUS_HAS_HDMI_OUTPUT
int bserver_hdmi_edid_audio_config(struct b_session *session, const NEXUS_HdmiOutputStatus *pStatus) {BSTD_UNUSED(session);BSTD_UNUSED(pStatus);return 0;}
#endif
int bserver_set_audio_config(struct b_audio_resource *r) {BSTD_UNUSED(r);return 0;}
int audio_get_stc_index(struct b_connect *connect) {BSTD_UNUSED(connect);return 0;}
#endif /* NEXUS_HAS_AUDIO */
