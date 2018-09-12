/******************************************************************************
 * Copyright (C) 2018 Broadcom.
 * The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to
 * the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied),
 * right to use, or waiver of any kind with respect to the Software, and
 * Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 * THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 * IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use all
 * reasonable efforts to protect the confidentiality thereof, and to use this
 * information only in connection with your use of Broadcom integrated circuit
 * products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 * OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 * RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 * IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 * A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 * ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 * THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 * OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 * INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 * RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 * HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 * EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 * WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 * FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
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
#include "nexus_audio_equalizer.h"

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

struct b_audio_equalizer_resource {
    unsigned numStages;
    NEXUS_AudioEqualizerHandle eqHandle;
    NEXUS_AudioEqualizerStageHandle stageHandle[NEXUS_MAX_AUDIO_STAGES_PER_EQUALIZER];
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
    NEXUS_AudioProcessorHandle advancedTsm; /* Only for MS11/MS12 configurations*/

    BLST_Q_HEAD(b_audio_playback_resource_list, b_audio_playback_resource) audioPlaybackList;
    NEXUS_AudioEncoderHandle audioEncoder;
    NEXUS_SimpleAudioDecoderHandle masterSimpleAudioDecoder; /* holds audio configuration in b_audio_mode_playback.
                                                               allows for universal MoveServerSettings for all config changes. */
    struct b_audio_equalizer_resource hdmiEqualizer;
    struct b_audio_equalizer_resource spdifEqualizer;
    struct b_audio_equalizer_resource dacEqualizer;
    struct b_audio_equalizer_resource i2sEqualizer[NEXUS_MAX_AUDIO_I2S_OUTPUTS];
    struct b_audio_equalizer_resource rfmEqualizer;

    struct b_session *session;
    struct b_connect *connect;
    bool secure; /* used for internal state of secure mode prior to first connect */
    bool localSession;
};

static struct {
    struct b_session *session;
    NxClient_AudioCaptureType captureType;
#if !NEXUS_AUDIO_BUFFER_CAPTURE_EXT
    NEXUS_AudioCaptureHandle handle;
#else
    NEXUS_AudioBufferCaptureHandle handle;
#endif
} g_capture[NEXUS_MAX_AUDIO_CAPTURE_OUTPUTS];

#if !NEXUS_AUDIO_BUFFER_CAPTURE_EXT
static void nxserverlib_p_configure_audio_output_capture(struct b_session *session, NEXUS_SimpleAudioDecoderServerSettings *sessionSettings, NxClient_AudioCaptureType captureType);
#else
static void nxserverlib_p_configure_audio_buffer_capture(struct b_session *session, NEXUS_AudioBufferCaptureCreateSettings *captureSettings, NxClient_AudioCaptureType captureType);
#endif

static enum nxserverlib_dolby_ms_type  b_dolby_ms(const struct b_session *session) {
    return session->server->settings.session[session->index].dolbyMs;
}

bool b_dolby_ms_capable(const struct b_session *session) {
    enum nxserverlib_dolby_ms_type ms = b_dolby_ms(session);
    return ms == nxserverlib_dolby_ms_type_ms11 || ms == nxserverlib_dolby_ms_type_ms12;
}

void  bserver_set_default_audio_settings(struct b_session *session)
{
    NEXUS_AudioCapabilities audioCapabilities;
    nxserver_t server = session->server;
    NEXUS_GetAudioCapabilities(&audioCapabilities);

    if (server->settings.session[session->index].dolbyMs == nxserverlib_dolby_ms_type_ms11 ||
        server->settings.session[session->index].dolbyMs == nxserverlib_dolby_ms_type_ms12) {
        NEXUS_AudioMixerSettings mixerSettings;

        if (audioCapabilities.dsp.processing[NEXUS_AudioPostProcessing_eAdvancedTsm]) {
            session->audioProcessingSettings.advancedTsm.mode = NEXUS_AudioAdvancedTsmMode_ePpm;
        }
        if (server->settings.session[session->index].dolbyMs == nxserverlib_dolby_ms_type_ms11 && audioCapabilities.dsp.dolbyVolume258) {
            NEXUS_DolbyVolume258_GetDefaultSettings(&session->audioProcessingSettings.dolby.dolbyVolume258);
            session->audioProcessingSettings.dolby.dolbyVolume258.enabled = false;
        }
        if (audioCapabilities.dsp.dolbyDigitalReencode) {
            NEXUS_DolbyDigitalReencode_GetDefaultSettings(&session->audioProcessingSettings.dolby.ddre);
            session->audioProcessingSettings.dolby.ddre.fixedEncoderFormat = true;
        }

        NEXUS_AudioMixer_GetDefaultSettings(&mixerSettings);
        BKNI_Memcpy(&session->audioProcessingSettings.dolby.dolbySettings, &mixerSettings.dolby, sizeof(NEXUS_AudioMixerDolbySettings));

        session->audioSettings.dolbyMsAllowed = true;
    }
    else {
        if (server->settings.session[session->index].avl && audioCapabilities.dsp.autoVolumeLevel) {
            NEXUS_AutoVolumeLevel_GetDefaultSettings(&session->audioProcessingSettings.avl);
            session->audioProcessingSettings.avl.enabled = false;
        }
        if (server->settings.session[session->index].truVolume && audioCapabilities.dsp.truVolume) {
            NEXUS_TruVolume_GetDefaultSettings(&session->audioProcessingSettings.truVolume);
            session->audioProcessingSettings.truVolume.enabled = false;
        }
    }
}

static bool b_is_aac(NEXUS_AudioCodec codec) {
    switch (codec) {
    case NEXUS_AudioCodec_eAacAdts:
    case NEXUS_AudioCodec_eAacLoas:
    case NEXUS_AudioCodec_eAacPlusAdts:
    case NEXUS_AudioCodec_eAacPlusLoas:
        return true;
    default:
        return false;
    }
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
    return 0;

error:
    b_audio_close_pb(r, pb);
    return rc;
}

static void b_audio_connect_pb(struct b_audio_resource *r)
{
    int rc;
    bool connected;
    struct b_audio_playback_resource *pb, *lastpb = NULL;

    pb = BLST_Q_FIRST(&r->audioPlaybackList);
    while (pb) {
        rc = 0;
        connected = is_connected_to_a_mixer(NEXUS_AudioPlayback_GetConnector(pb->audioPlayback), r);
        if (!connected) {
            if (r->mixer[nxserver_audio_mixer_persistent]) {
                rc = NEXUS_AudioMixer_AddInput(r->mixer[nxserver_audio_mixer_persistent], b_audio_get_pb_output(pb));
            }
            else {
                if (r->mixer[nxserver_audio_mixer_stereo]) {
                    rc = NEXUS_AudioMixer_AddInput(r->mixer[nxserver_audio_mixer_stereo], b_audio_get_pb_output(pb));
                }
                if (r->mixer[nxserver_audio_mixer_multichannel]) {
                    rc = NEXUS_AudioMixer_AddInput(r->mixer[nxserver_audio_mixer_multichannel], b_audio_get_pb_output(pb));
                }
            }
            if (rc) {
                b_audio_close_pb(r, pb);
                if (lastpb) {
                    pb = BLST_Q_NEXT(lastpb, link);
                }
                else {
                    pb = BLST_Q_FIRST(&r->audioPlaybackList);
                }
                continue;
            }
        }
        lastpb = pb;
        pb = BLST_Q_NEXT(pb, link);
    }
    return;
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

        NEXUS_SimpleAudioPlayback_GetServerSettings(connect->client->session->audio.playbackServer, req->handles.simpleAudioPlayback[index].handle, &settings);
        settings.playback = NULL;
        settings.i2sInput = NULL;
        (void)NEXUS_SimpleAudioPlayback_SetServerSettings(connect->client->session->audio.playbackServer, req->handles.simpleAudioPlayback[index].handle, &settings);
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

        NEXUS_SimpleAudioPlayback_GetServerSettings(connect->client->session->audio.playbackServer, req->handles.simpleAudioPlayback[index].handle, &settings);
        settings.i2sInput = pb->i2sInput;
        settings.playback = pb->audioPlayback;
        rc = NEXUS_SimpleAudioPlayback_SetServerSettings(connect->client->session->audio.playbackServer, req->handles.simpleAudioPlayback[index].handle, &settings);
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
    else if (r->advancedTsm) {
        switch (type)
        {
        default:
        case NEXUS_AudioConnectorType_eStereo:
            return NEXUS_AudioProcessor_GetConnectorByType(r->advancedTsm, NEXUS_AudioConnectorType_eStereo);
        case NEXUS_AudioConnectorType_eMultichannel:
            return NEXUS_AudioProcessor_GetConnectorByType(r->advancedTsm, NEXUS_AudioConnectorType_eMultichannel);
        }
    }
    else if (r->dolbyVolume258)
    {
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
    /* filter graph is mixer[->advancedTsm[->avl][->truVolume][->dolbyVolume258][->ddre] */
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

static BERR_Code b_audio_equalizer_open(struct b_audio_equalizer_resource *equalizer, NEXUS_AudioOutputHandle outputHandle)
{
    NEXUS_AudioEqualizerSettings equalizerSettings;
    NEXUS_AudioEqualizer_GetDefaultSettings(&equalizerSettings);
    equalizer->eqHandle = NEXUS_AudioEqualizer_Create(&equalizerSettings);
    if (!equalizer->eqHandle){
        return BERR_TRACE(BERR_NOT_AVAILABLE);
    }
    return NEXUS_AudioOutput_SetEqualizer(outputHandle, equalizer->eqHandle);
}

static void b_audio_equalizer_close(struct b_audio_equalizer_resource *equalizer, NEXUS_AudioOutputHandle outputHandle, bool onlyStages)
{
    unsigned i;
    NEXUS_AudioEqualizer_RemoveAllStages(equalizer->eqHandle);
    NEXUS_AudioOutput_ClearEqualizer(outputHandle);
    for (i = 0; i < equalizer->numStages; i++) {
        NEXUS_AudioEqualizerStage_Destroy(equalizer->stageHandle[i]);
    }
    if (onlyStages) {
        equalizer->numStages = 0;
        BKNI_Memset(equalizer->stageHandle, 0, sizeof(equalizer->stageHandle));
        NEXUS_AudioOutput_SetEqualizer(outputHandle, equalizer->eqHandle);
    }
    else {
        NEXUS_AudioEqualizer_Destroy(equalizer->eqHandle);
        BKNI_Memset(equalizer, 0, sizeof(struct b_audio_equalizer_resource));
    }
}

static bool b_is_equalizer_changed(const NxClient_AudioEqualizer *pNewEqSettings, const struct b_audio_equalizer_resource *pOldEqSettings)
{
    NEXUS_AudioEqualizerStageSettings stageSettings;
    unsigned i;

    if (pNewEqSettings->numStages != pOldEqSettings->numStages) {
        return true;
    }
    else if (pNewEqSettings->numStages != 0) {
        for (i = 0; i < pNewEqSettings->numStages; i++) {
            NEXUS_AudioEqualizerStage_GetSettings(pOldEqSettings->stageHandle[i], &stageSettings);
            if (pNewEqSettings->stageSettings[i].type != stageSettings.type) {
                return true;
            }
        }
    }
    return false;
}

static bool b_is_any_equalizer_changed(const struct b_audio_resource *r)
{
    bool eqChanged = false;

    #if NEXUS_HAS_HDMI_OUTPUT
    if (r->session->server->settings.audioOutputs.hdmiEnabled[0]) {
        eqChanged |= b_is_equalizer_changed(&r->session->audioSettings.hdmi.equalizer, &r->hdmiEqualizer);
    }
    #endif

    if (r->session->server->settings.audioOutputs.spdifEnabled[0]) {
        eqChanged |= b_is_equalizer_changed(&r->session->audioSettings.spdif.equalizer, &r->spdifEqualizer);
    }

    if (r->session->server->settings.audioOutputs.dacEnabled[0] ||
        (r->session->server->settings.audioOutputs.i2sEnabled[0] && nxserverlib_p_audio_i2s0_shares_with_dac(r->session) == true)) {
        eqChanged |= b_is_equalizer_changed(&r->session->audioSettings.dac.equalizer, &r->dacEqualizer);
    }

    if (r->session->server->settings.audioOutputs.i2sEnabled[0] && nxserverlib_p_audio_i2s0_shares_with_dac(r->session) == false) {
        eqChanged |= b_is_equalizer_changed(&r->session->audioSettings.i2s[0].equalizer, &r->i2sEqualizer[0]);
    }

    if (r->session->server->settings.audioOutputs.i2sEnabled[1]) {
        eqChanged |= b_is_equalizer_changed(&r->session->audioSettings.i2s[1].equalizer, &r->i2sEqualizer[1]);
    }

    if (r->session->server->settings.audioOutputs.rfmEnabled) {
        eqChanged |= b_is_equalizer_changed(&r->session->audioSettings.rfm.equalizer, &r->rfmEqualizer);
    }

    return eqChanged;
}

static void b_audio_equalizer_apply_settings(NEXUS_AudioEqualizerStageSettings *pOldStageSettings, NEXUS_AudioEqualizerStageSettings *pNewStageSettings)
{
    pNewStageSettings->enabled = pOldStageSettings->enabled;
    pNewStageSettings->rampSettings.enable = pOldStageSettings->rampSettings.enable;
    pNewStageSettings->rampSettings.stepSize = pOldStageSettings->rampSettings.stepSize;
    BKNI_Memcpy(&pNewStageSettings->modeSettings, &pOldStageSettings->modeSettings, sizeof(pOldStageSettings->modeSettings));
}

static void b_configure_equalizers(struct b_audio_resource *r, bool suspended)
{
    unsigned i, eqIndex;
    NEXUS_AudioEqualizerStageSettings stageSettings;
    nxserver_t server = r->session->server;
#if NEXUS_HAS_HDMI_OUTPUT
    bool hdmiChanged = false;
#endif
    bool spdifChanged = false;
    bool i2s0Changed = false;
    bool i2s1Changed = false;
    bool dacChanged = false;
    #if NEXUS_HAS_RFM && NEXUS_NUM_RFM_OUTPUTS
    bool rfmChanged = false;
    #endif
    NEXUS_AudioCapabilities cap;
    unsigned stagesInUse = 0;

    NEXUS_GetAudioCapabilities(&cap);

    stagesInUse = (r->hdmiEqualizer.numStages + r->spdifEqualizer.numStages + r->dacEqualizer.numStages +
                   r->i2sEqualizer[0].numStages + r->i2sEqualizer[1].numStages + r->rfmEqualizer.numStages);

    /* If suspended just tear everything down and rebuild in case we moved an eq around */
    if (suspended) {
        #if NEXUS_HAS_HDMI_OUTPUT
        if (r->session->server->settings.audioOutputs.hdmiEnabled[0]) {
            if (b_is_equalizer_changed(&r->session->audioSettings.hdmi.equalizer, &r->hdmiEqualizer)) {
                hdmiChanged = true;
                if (r->hdmiEqualizer.eqHandle) {
                    stagesInUse -= r->hdmiEqualizer.numStages;
                    if (r->session->audioSettings.hdmi.equalizer.numStages == 0) {
                        b_audio_equalizer_close(&r->hdmiEqualizer, NEXUS_HdmiOutput_GetAudioConnector(server->platformConfig.outputs.hdmi[0]), false);
                    }
                    else {
                        b_audio_equalizer_close(&r->hdmiEqualizer, NEXUS_HdmiOutput_GetAudioConnector(server->platformConfig.outputs.hdmi[0]), true);
                    }
                }
                else {
                    b_audio_equalizer_open(&r->hdmiEqualizer, NEXUS_HdmiOutput_GetAudioConnector(server->platformConfig.outputs.hdmi[0]));
                }
            }
        }
        #endif
        if (r->session->server->settings.audioOutputs.spdifEnabled[0]) {
            if (b_is_equalizer_changed(&r->session->audioSettings.spdif.equalizer, &r->spdifEqualizer)) {
                spdifChanged = true;
                if (r->spdifEqualizer.eqHandle) {
                    stagesInUse -= r->spdifEqualizer.numStages;
                    if (r->session->audioSettings.spdif.equalizer.numStages == 0) {
                        b_audio_equalizer_close(&r->spdifEqualizer, NEXUS_SpdifOutput_GetConnector(server->platformConfig.outputs.spdif[0]), false);
                    }
                    else {
                        b_audio_equalizer_close(&r->spdifEqualizer, NEXUS_SpdifOutput_GetConnector(server->platformConfig.outputs.spdif[0]), true);
                    }
                }
                else {
                    b_audio_equalizer_open(&r->spdifEqualizer, NEXUS_SpdifOutput_GetConnector(server->platformConfig.outputs.spdif[0]));
                }
            }
        }
        #if NEXUS_HAS_RFM && NEXUS_NUM_RFM_OUTPUTS
        if (r->session->server->settings.audioOutputs.rfmEnabled) {
            if (b_is_equalizer_changed(&r->session->audioSettings.rfm.equalizer, &r->rfmEqualizer)) {
                rfmChanged = true;
                if (r->rfmEqualizer.eqHandle) {
                    stagesInUse -= r->rfmEqualizer.numStages;
                    if (r->session->audioSettings.rfm.equalizer.numStages == 0) {
                        b_audio_equalizer_close(&r->rfmEqualizer, NEXUS_Rfm_GetAudioConnector(server->platformConfig.outputs.rfm[0]), false);
                    }
                    else {
                        b_audio_equalizer_close(&r->rfmEqualizer, NEXUS_Rfm_GetAudioConnector(server->platformConfig.outputs.rfm[0]), true);
                    }
                }
                else {
                    b_audio_equalizer_open(&r->rfmEqualizer, NEXUS_Rfm_GetAudioConnector(server->platformConfig.outputs.rfm[0]));
                }
            }
        }
        #endif
        if (r->session->server->settings.audioOutputs.dacEnabled[0]) {
            if (b_is_equalizer_changed(&r->session->audioSettings.dac.equalizer, &r->dacEqualizer)) {
                dacChanged = true;
                if (r->dacEqualizer.eqHandle) {
                    stagesInUse -= r->dacEqualizer.numStages;
                    if (r->session->audioSettings.dac.equalizer.numStages == 0) {
                        b_audio_equalizer_close(&r->dacEqualizer, NEXUS_AudioDac_GetConnector(server->platformConfig.outputs.audioDacs[0]), false);
                    }
                    else {
                        b_audio_equalizer_close(&r->dacEqualizer, NEXUS_AudioDac_GetConnector(server->platformConfig.outputs.audioDacs[0]), true);
                    }
                }
                else {
                    b_audio_equalizer_open(&r->dacEqualizer, NEXUS_AudioDac_GetConnector(server->platformConfig.outputs.audioDacs[0]));
                }
            }
        }
        if (r->session->server->settings.audioOutputs.i2sEnabled[0]) {
            if (nxserverlib_p_audio_i2s0_shares_with_dac(r->session)) {
                if (b_is_equalizer_changed(&r->session->audioSettings.dac.equalizer, &r->i2sEqualizer[0])) {
                    i2s0Changed = true;
                    if (r->i2sEqualizer[0].eqHandle) {
                        stagesInUse -= r->i2sEqualizer[0].numStages;
                        if (r->session->audioSettings.dac.equalizer.numStages == 0) {
                            b_audio_equalizer_close(&r->i2sEqualizer[0], NEXUS_I2sOutput_GetConnector(server->platformConfig.outputs.i2s[0]), false);
                        }
                        else {
                            b_audio_equalizer_close(&r->i2sEqualizer[0], NEXUS_I2sOutput_GetConnector(server->platformConfig.outputs.i2s[0]), true);
                        }
                    }
                    else {
                        b_audio_equalizer_open(&r->i2sEqualizer[0], NEXUS_I2sOutput_GetConnector(server->platformConfig.outputs.i2s[0]));
                    }
                }
            }
            else {
                if (b_is_equalizer_changed(&r->session->audioSettings.i2s[0].equalizer, &r->i2sEqualizer[0])) {
                    i2s0Changed = true;
                    if (r->i2sEqualizer[0].eqHandle) {
                        stagesInUse -= r->i2sEqualizer[0].numStages;
                        if (r->session->audioSettings.i2s[0].equalizer.numStages == 0) {
                            b_audio_equalizer_close(&r->i2sEqualizer[0], NEXUS_I2sOutput_GetConnector(server->platformConfig.outputs.i2s[0]), false);
                        }
                        else {
                            b_audio_equalizer_close(&r->i2sEqualizer[0], NEXUS_I2sOutput_GetConnector(server->platformConfig.outputs.i2s[0]), true);
                        }
                    }
                    else {
                        b_audio_equalizer_open(&r->i2sEqualizer[0], NEXUS_I2sOutput_GetConnector(server->platformConfig.outputs.i2s[0]));
                    }
                }
            }
        }
        if (r->session->server->settings.audioOutputs.i2sEnabled[1]) {
            if (b_is_equalizer_changed(&r->session->audioSettings.i2s[1].equalizer, &r->i2sEqualizer[1])) {
                i2s1Changed = true;
                if (r->i2sEqualizer[1].eqHandle) {
                    stagesInUse -= r->i2sEqualizer[1].numStages;
                    if (r->session->audioSettings.i2s[1].equalizer.numStages == 0) {
                        b_audio_equalizer_close(&r->i2sEqualizer[1], NEXUS_I2sOutput_GetConnector(server->platformConfig.outputs.i2s[1]), false);
                    }
                    else {
                        b_audio_equalizer_close(&r->i2sEqualizer[1], NEXUS_I2sOutput_GetConnector(server->platformConfig.outputs.i2s[1]), true);
                    }
                }
                else {
                    b_audio_equalizer_open(&r->i2sEqualizer[1], NEXUS_I2sOutput_GetConnector(server->platformConfig.outputs.i2s[1]));
                }
            }
        }
    }

    #if NEXUS_HAS_HDMI_OUTPUT
    if (r->session->server->settings.audioOutputs.hdmiEnabled[0]) {
        if (r->session->audioSettings.hdmi.equalizer.numStages > 0) {
            if (r->hdmiEqualizer.eqHandle) {
                eqIndex = 0;
                for (i = 0; i < r->session->audioSettings.hdmi.equalizer.numStages; i++) {
                    if (hdmiChanged) {
                        if (stagesInUse < cap.numEqualizerStages) {
                            if (r->session->audioSettings.hdmi.equalizer.stageSettings[i].type != NEXUS_AudioEqualizerStageType_eMax) {
                                NEXUS_AudioEqualizerStage_GetDefaultSettings(r->session->audioSettings.hdmi.equalizer.stageSettings[i].type, &stageSettings);
                                b_audio_equalizer_apply_settings(&r->session->audioSettings.hdmi.equalizer.stageSettings[i], &stageSettings);
                                r->hdmiEqualizer.stageHandle[eqIndex] = NEXUS_AudioEqualizerStage_Create(&stageSettings);
                                if (r->hdmiEqualizer.stageHandle[eqIndex]) {
                                    NEXUS_AudioEqualizer_AddStage(r->hdmiEqualizer.eqHandle, r->hdmiEqualizer.stageHandle[eqIndex]);
                                    r->hdmiEqualizer.numStages++;
                                    eqIndex++;
                                    stagesInUse++;
                                }
                            }
                        }
                        else {
                            BDBG_WRN(("Unable to add HDMI Equalizer Stage.  Max Stages allowed %u", cap.numEqualizerStages));
                            break;
                        }
                    }
                    else {
                        if (r->hdmiEqualizer.stageHandle[i]) {
                            NEXUS_AudioEqualizerStage_GetSettings(r->hdmiEqualizer.stageHandle[i], &stageSettings);
                            b_audio_equalizer_apply_settings(&r->session->audioSettings.hdmi.equalizer.stageSettings[i], &stageSettings);
                            NEXUS_AudioEqualizerStage_SetSettings(r->hdmiEqualizer.stageHandle[i], &stageSettings);
                        }
                    }
                }
                if (r->hdmiEqualizer.numStages == 0) {
                    b_audio_equalizer_close(&r->hdmiEqualizer, NEXUS_HdmiOutput_GetAudioConnector(server->platformConfig.outputs.hdmi[0]), false);
                }
            }
        }
    }
    #endif

    if (r->session->server->settings.audioOutputs.spdifEnabled[0]) {
        if (r->session->audioSettings.spdif.equalizer.numStages > 0) {
            if (r->spdifEqualizer.eqHandle) {
                eqIndex = 0;
                for (i = 0; i < r->session->audioSettings.spdif.equalizer.numStages; i++) {
                    if (spdifChanged) {
                        if (stagesInUse < cap.numEqualizerStages) {
                            if (r->session->audioSettings.spdif.equalizer.stageSettings[i].type != NEXUS_AudioEqualizerStageType_eMax) {
                                NEXUS_AudioEqualizerStage_GetDefaultSettings(r->session->audioSettings.spdif.equalizer.stageSettings[i].type, &stageSettings);
                                b_audio_equalizer_apply_settings(&r->session->audioSettings.spdif.equalizer.stageSettings[i], &stageSettings);
                                r->spdifEqualizer.stageHandle[eqIndex] = NEXUS_AudioEqualizerStage_Create(&stageSettings);
                                if (r->spdifEqualizer.stageHandle[eqIndex]) {
                                    NEXUS_AudioEqualizer_AddStage(r->spdifEqualizer.eqHandle, r->spdifEqualizer.stageHandle[eqIndex]);
                                    r->spdifEqualizer.numStages++;
                                    eqIndex++;
                                    stagesInUse++;
                                }
                            }
                        }
                        else {
                            BDBG_WRN(("Unable to add Spdif Equalizer Stage.  Max Stages allowed %u", cap.numEqualizerStages));
                            break;
                        }
                    }
                    else {
                        if (r->spdifEqualizer.stageHandle[i]) {
                            NEXUS_AudioEqualizerStage_GetSettings(r->spdifEqualizer.stageHandle[i], &stageSettings);
                            b_audio_equalizer_apply_settings(&r->session->audioSettings.spdif.equalizer.stageSettings[i], &stageSettings);
                            NEXUS_AudioEqualizerStage_SetSettings(r->spdifEqualizer.stageHandle[i], &stageSettings);
                        }
                    }
                    if (r->spdifEqualizer.numStages == 0) {
                        b_audio_equalizer_close(&r->spdifEqualizer, NEXUS_SpdifOutput_GetConnector(server->platformConfig.outputs.spdif[0]), false);
                    }
                }
            }
        }
    }

    if (r->session->server->settings.audioOutputs.dacEnabled[0]) {
        if (r->session->audioSettings.dac.equalizer.numStages > 0) {
            if (r->dacEqualizer.eqHandle) {
                eqIndex = 0;
                for (i = 0; i < r->session->audioSettings.dac.equalizer.numStages; i++) {
                    if (dacChanged) {
                        if (stagesInUse < cap.numEqualizerStages) {
                            if (r->session->audioSettings.dac.equalizer.stageSettings[i].type != NEXUS_AudioEqualizerStageType_eMax) {
                                NEXUS_AudioEqualizerStage_GetDefaultSettings(r->session->audioSettings.dac.equalizer.stageSettings[i].type, &stageSettings);
                                b_audio_equalizer_apply_settings(&r->session->audioSettings.dac.equalizer.stageSettings[i], &stageSettings);
                                r->dacEqualizer.stageHandle[eqIndex] = NEXUS_AudioEqualizerStage_Create(&stageSettings);
                                if (r->dacEqualizer.stageHandle[eqIndex]) {
                                    NEXUS_AudioEqualizer_AddStage(r->dacEqualizer.eqHandle, r->dacEqualizer.stageHandle[eqIndex]);
                                    r->dacEqualizer.numStages++;
                                    eqIndex++;
                                    stagesInUse++;
                                }
                            }
                        }
                        else {
                            BDBG_WRN(("Unable to add DAC Equalizer Stage.  Max Stages allowed %u", cap.numEqualizerStages));
                            break;
                        }
                    }
                    else {
                        if (r->dacEqualizer.stageHandle[i]) {
                            NEXUS_AudioEqualizerStage_GetSettings(r->dacEqualizer.stageHandle[i], &stageSettings);
                            b_audio_equalizer_apply_settings(&r->session->audioSettings.dac.equalizer.stageSettings[i], &stageSettings);
                            NEXUS_AudioEqualizerStage_SetSettings(r->dacEqualizer.stageHandle[i], &stageSettings);
                        }
                    }
                }
                if (r->dacEqualizer.numStages == 0) {
                    b_audio_equalizer_close(&r->dacEqualizer, NEXUS_AudioDac_GetConnector(server->platformConfig.outputs.audioDacs[0]), false);
                }
            }
        }
    }


    if (r->session->server->settings.audioOutputs.i2sEnabled[0]) {
        if (nxserverlib_p_audio_i2s0_shares_with_dac(r->session)) {
            if (r->session->audioSettings.dac.equalizer.numStages > 0) {
                if (r->i2sEqualizer[0].eqHandle) {
                    eqIndex = 0;
                    for (i = 0; i < r->session->audioSettings.dac.equalizer.numStages; i++) {
                        if (i2s0Changed) {
                            if (stagesInUse < cap.numEqualizerStages) {
                                if (r->session->audioSettings.dac.equalizer.stageSettings[i].type != NEXUS_AudioEqualizerStageType_eMax) {
                                    NEXUS_AudioEqualizerStage_GetDefaultSettings(r->session->audioSettings.dac.equalizer.stageSettings[i].type, &stageSettings);
                                    b_audio_equalizer_apply_settings(&r->session->audioSettings.dac.equalizer.stageSettings[i], &stageSettings);
                                    r->i2sEqualizer[0].stageHandle[eqIndex] = NEXUS_AudioEqualizerStage_Create(&stageSettings);
                                    if (r->i2sEqualizer[0].stageHandle[eqIndex]) {
                                        NEXUS_AudioEqualizer_AddStage(r->i2sEqualizer[0].eqHandle, r->i2sEqualizer[0].stageHandle[eqIndex]);
                                        r->i2sEqualizer[0].numStages++;
                                        eqIndex++;
                                        stagesInUse++;
                                    }
                                }
                            }
                            else {
                                BDBG_WRN(("Unable to add I2S0 Equalizer Stage.  Max Stages allowed %u", cap.numEqualizerStages));
                                break;
                            }
                        }
                        else {
                            if (r->i2sEqualizer[0].stageHandle[i]) {
                                NEXUS_AudioEqualizerStage_GetSettings(r->i2sEqualizer[0].stageHandle[i], &stageSettings);
                                b_audio_equalizer_apply_settings(&r->session->audioSettings.dac.equalizer.stageSettings[i], &stageSettings);
                                NEXUS_AudioEqualizerStage_SetSettings(r->i2sEqualizer[0].stageHandle[i], &stageSettings);
                            }
                        }
                    }
                    if (r->i2sEqualizer[0].numStages == 0) {
                        b_audio_equalizer_close(&r->i2sEqualizer[0], NEXUS_I2sOutput_GetConnector(server->platformConfig.outputs.i2s[0]), false);
                    }
                }
            }
        }
        else {
            if (r->session->audioSettings.i2s[0].equalizer.numStages > 0) {
                if (r->i2sEqualizer[0].eqHandle) {
                    eqIndex = 0;
                    for (i = 0; i < r->session->audioSettings.i2s[0].equalizer.numStages; i++) {
                        if (i2s0Changed) {
                            if (stagesInUse < cap.numEqualizerStages) {
                                if (r->session->audioSettings.i2s[0].equalizer.stageSettings[i].type != NEXUS_AudioEqualizerStageType_eMax) {
                                    NEXUS_AudioEqualizerStage_GetDefaultSettings(r->session->audioSettings.i2s[0].equalizer.stageSettings[i].type, &stageSettings);
                                    b_audio_equalizer_apply_settings(&r->session->audioSettings.i2s[0].equalizer.stageSettings[i], &stageSettings);
                                    r->i2sEqualizer[0].stageHandle[eqIndex] = NEXUS_AudioEqualizerStage_Create(&stageSettings);
                                    if (r->i2sEqualizer[0].stageHandle[eqIndex]) {
                                        NEXUS_AudioEqualizer_AddStage(r->i2sEqualizer[0].eqHandle, r->i2sEqualizer[0].stageHandle[eqIndex]);
                                        r->i2sEqualizer[0].numStages++;
                                        eqIndex++;
                                        stagesInUse++;
                                    }
                                }
                            }
                            else {
                                BDBG_WRN(("Unable to add I2S0 Equalizer Stage.  Max Stages allowed %u", cap.numEqualizerStages));
                                break;
                            }
                        }
                        else {
                            if (r->i2sEqualizer[0].stageHandle[i]) {
                                NEXUS_AudioEqualizerStage_GetSettings(r->i2sEqualizer[0].stageHandle[i], &stageSettings);
                                b_audio_equalizer_apply_settings(&r->session->audioSettings.i2s[0].equalizer.stageSettings[i], &stageSettings);
                                NEXUS_AudioEqualizerStage_SetSettings(r->i2sEqualizer[0].stageHandle[i], &stageSettings);
                            }
                        }
                    }
                    if (r->i2sEqualizer[0].numStages == 0) {
                        b_audio_equalizer_close(&r->i2sEqualizer[0], NEXUS_I2sOutput_GetConnector(server->platformConfig.outputs.i2s[0]), false);
                    }
                }
            }
        }
    }

  if (r->session->server->settings.audioOutputs.i2sEnabled[1]) {
      if (r->session->audioSettings.i2s[1].equalizer.numStages > 0) {
          if (r->i2sEqualizer[1].eqHandle) {
              eqIndex = 0;
              for (i = 0; i < r->session->audioSettings.i2s[1].equalizer.numStages; i++) {
                  if (i2s1Changed) {
                      if (stagesInUse < cap.numEqualizerStages) {
                          if (r->session->audioSettings.i2s[1].equalizer.stageSettings[i].type != NEXUS_AudioEqualizerStageType_eMax) {
                              NEXUS_AudioEqualizerStage_GetDefaultSettings(r->session->audioSettings.i2s[1].equalizer.stageSettings[i].type, &stageSettings);
                              b_audio_equalizer_apply_settings(&r->session->audioSettings.i2s[1].equalizer.stageSettings[i], &stageSettings);
                              r->i2sEqualizer[1].stageHandle[eqIndex] = NEXUS_AudioEqualizerStage_Create(&stageSettings);
                              if (r->i2sEqualizer[1].stageHandle[eqIndex]) {
                                  NEXUS_AudioEqualizer_AddStage(r->i2sEqualizer[1].eqHandle, r->i2sEqualizer[1].stageHandle[eqIndex]);
                                  r->i2sEqualizer[1].numStages++;
                                  eqIndex++;
                                  stagesInUse++;
                              }
                          }
                      }
                      else {
                          BDBG_WRN(("Unable to add I2S1 Equalizer Stage.  Max Stages allowed %u", cap.numEqualizerStages));
                          break;
                      }
                  }
                  else {
                      if (r->i2sEqualizer[1].stageHandle[i]) {
                          NEXUS_AudioEqualizerStage_GetSettings(r->i2sEqualizer[1].stageHandle[i], &stageSettings);
                          b_audio_equalizer_apply_settings(&r->session->audioSettings.i2s[1].equalizer.stageSettings[i], &stageSettings);
                          NEXUS_AudioEqualizerStage_SetSettings(r->i2sEqualizer[1].stageHandle[i], &stageSettings);
                      }
                  }
              }
          }
          if (r->i2sEqualizer[1].numStages == 0) {
              b_audio_equalizer_close(&r->i2sEqualizer[1], NEXUS_I2sOutput_GetConnector(server->platformConfig.outputs.i2s[1]), false);
          }
      }
  }
  #if NEXUS_HAS_RFM && NEXUS_NUM_RFM_OUTPUTS
  if (r->session->server->settings.audioOutputs.rfmEnabled) {
      if (r->session->audioSettings.rfm.equalizer.numStages > 0) {
          if (r->rfmEqualizer.eqHandle) {
              eqIndex = 0;
              for (i = 0; i < r->session->audioSettings.rfm.equalizer.numStages; i++) {
                  if (rfmChanged) {
                      if (stagesInUse < cap.numEqualizerStages) {
                          if (r->session->audioSettings.rfm.equalizer.stageSettings[i].type != NEXUS_AudioEqualizerStageType_eMax) {
                              NEXUS_AudioEqualizerStage_GetDefaultSettings(r->session->audioSettings.rfm.equalizer.stageSettings[i].type, &stageSettings);
                              b_audio_equalizer_apply_settings(&r->session->audioSettings.rfm.equalizer.stageSettings[i], &stageSettings);
                              r->rfmEqualizer.stageHandle[eqIndex] = NEXUS_AudioEqualizerStage_Create(&stageSettings);
                              if (r->spdifEqualizer.stageHandle[eqIndex]) {
                                  NEXUS_AudioEqualizer_AddStage(r->rfmEqualizer.eqHandle, r->rfmEqualizer.stageHandle[eqIndex]);
                                  r->rfmEqualizer.numStages++;
                                  eqIndex++;
                                  stagesInUse++;
                              }
                          }
                      }
                      else {
                          BDBG_WRN(("Unable to add RFM Equalizer Stage.  Max Stages allowed %u", cap.numEqualizerStages));
                          break;
                        }
                  }
                  else {
                      if (r->rfmEqualizer.stageHandle[i]) {
                          NEXUS_AudioEqualizerStage_GetSettings(r->rfmEqualizer.stageHandle[i], &stageSettings);
                          b_audio_equalizer_apply_settings(&r->session->audioSettings.rfm.equalizer.stageSettings[i], &stageSettings);
                          NEXUS_AudioEqualizerStage_SetSettings(r->rfmEqualizer.stageHandle[i], &stageSettings);
                      }
                  }
              }
              if (r->rfmEqualizer.numStages == 0) {
                  b_audio_equalizer_close(&r->rfmEqualizer, NEXUS_Rfm_GetAudioConnector(server->platformConfig.outputs.rfm[0]), false);
              }
          }
      }
  }
  #endif
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

int nxserverlib_p_audio_i2s0_shares_with_dac(struct b_session *session)
{
    nxserver_t server = session->server;
    return (server->settings.session[session->index].i2sOutputEnabled[0] == false &&
            server->settings.audioOutputs.dacEnabled[0] == false);
}

static void b_audio_update_persistent_settings(struct b_session *session, struct b_audio_resource *r)
{
    int i;
    BERR_Code rc;
    NEXUS_SimpleAudioDecoderHandle activeDecoder;
    NEXUS_SimpleAudioDecoderHandle persistentDecoder;
    NEXUS_SimpleAudioDecoderServerSettings settings;
    NEXUS_SimpleAudioDecoderServerSettings persisentSettings;

    activeDecoder = b_audio_get_active_decoder(session->main_audio);
    NEXUS_SimpleAudioDecoder_GetServerSettings(session->audio.server, activeDecoder, &settings);
    for (i = 0; i < NEXUS_MAX_AUDIO_DECODERS; i++) {
        if (settings.persistent[i].decoder != NULL) {
            persistentDecoder = settings.persistent[i].decoder;
            NEXUS_SimpleAudioDecoder_GetServerSettings(session->audio.server, persistentDecoder, &persisentSettings);
            persisentSettings.mixers.multichannel = r->mixer[nxserver_audio_mixer_multichannel];
            if (session->audioSettings.dolbyMsAllowed) {
                persisentSettings.mixers.stereo = NULL;
                persisentSettings.capabilities.ms12 = (b_dolby_ms(session) == nxserverlib_dolby_ms_type_ms12) ? true : false;
                persisentSettings.capabilities.ms11 = (b_dolby_ms(session) == nxserverlib_dolby_ms_type_ms11) ? true : false;
            }
            else {
                persisentSettings.mixers.stereo = r->mixer[nxserver_audio_mixer_stereo];
                persisentSettings.capabilities.ms12 = persisentSettings.capabilities.ms11 = NULL;
            }
            rc = NEXUS_SimpleAudioDecoder_SetServerSettings(session->audio.server, persistentDecoder, &persisentSettings, false);
            if (rc) {BERR_TRACE(rc);}
        }
    }
    rc = NEXUS_SimpleAudioDecoder_SetServerSettings(session->audio.server, activeDecoder, &settings, false);
    if (rc) {BERR_TRACE(rc);}
    return;
}

BERR_Code nxserverlib_p_audio_create_downstream(struct b_session *session, struct b_audio_resource *r)
{
    BERR_Code rc = BERR_SUCCESS;
    NEXUS_AudioMixerSettings mixerSettings;
    NEXUS_AudioCapabilities cap;
    nxserver_t server = session->server;

    NEXUS_GetAudioCapabilities(&cap);
    if (r->localSession) {
        /* Open mixer to mix the description and primary audio */
        NEXUS_AudioMixer_GetDefaultSettings(&mixerSettings);
        mixerSettings.mixUsingDsp = session->server->settings.session[session->index].output.encode ||
            server->settings.session[r->session->index].avl ||
            server->settings.session[r->session->index].truVolume ||
            server->settings.audioDecoder.audioDescription ||
            server->settings.session[r->session->index].persistentDecoderSupport ||
            b_dolby_ms_capable(session);

        if (!cap.dsp.mixer) {
            mixerSettings.mixUsingDsp = false;
        }

        if (b_dolby_ms_capable(session) && !session->audioSettings.dolbyMsAllowed) {
            mixerSettings.mixUsingDsp = false;
        }
        mixerSettings.outputSampleRate = (server->settings.audioMixer.sampleRate >= 32000 && server->settings.audioMixer.sampleRate <= 96000) ? server->settings.audioMixer.sampleRate : mixerSettings.outputSampleRate;
        if (mixerSettings.mixUsingDsp) {
            if (b_dolby_ms_capable(session)) {
                mixerSettings.outputSampleRate = 48000;
            }
        }

        /* If we are doing Dolby MS then we need a multichannel DSP mixer otherwise
           if a dsp mixer is required it should be used for stereo */
        r->dspMixer[nxserver_audio_mixer_multichannel] = false;
        r->dspMixer[nxserver_audio_mixer_stereo] = false;
        if (mixerSettings.mixUsingDsp && b_dolby_ms_capable(session))
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
            BKNI_Memcpy(&mixerSettings.dolby, &session->audioProcessingSettings.dolby.dolbySettings, sizeof(NEXUS_AudioMixerDolbySettings));
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
            return BERR_TRACE(NEXUS_NOT_AVAILABLE);
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
            return BERR_TRACE(NEXUS_NOT_AVAILABLE);
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
                return BERR_TRACE(NEXUS_NOT_AVAILABLE);
            }
            if ( r->mixer[nxserver_audio_mixer_stereo] ) {
                NEXUS_AudioMixer_AddInput(r->mixer[nxserver_audio_mixer_stereo], NEXUS_AudioMixer_GetConnector(r->mixer[nxserver_audio_mixer_persistent]));
            }
            if ( r->mixer[nxserver_audio_mixer_multichannel] ) {
                NEXUS_AudioMixer_AddInput(r->mixer[nxserver_audio_mixer_multichannel], NEXUS_AudioMixer_GetConnector(r->mixer[nxserver_audio_mixer_persistent]));
            }
        }

        if (b_dolby_ms_capable(session) && session->audioSettings.dolbyMsAllowed) {
            if (cap.dsp.processing[NEXUS_AudioPostProcessing_eAdvancedTsm]) {
                NEXUS_AudioInputHandle audioInput = b_audio_get_pcm_input(r, NEXUS_AudioConnectorType_eMultichannel);
                NEXUS_AudioProcessorOpenSettings procOpenSettings;

                NEXUS_AudioProcessor_GetDefaultOpenSettings(&procOpenSettings);
                procOpenSettings.type = NEXUS_AudioPostProcessing_eAdvancedTsm;
                r->advancedTsm = NEXUS_AudioProcessor_Open(&procOpenSettings);
                if (r->advancedTsm) {
                    rc = NEXUS_AudioProcessor_AddInput(r->advancedTsm, audioInput);
                    if (rc) {
                        BDBG_ERR(("NEXUS_AudioProcessor_AddInput %d", rc));
                        NEXUS_AudioProcessor_Close(r->advancedTsm);
                        r->advancedTsm = NULL;
                    }
                    else {
                        NEXUS_AudioProcessorSettings processorSettings;
                        NEXUS_AudioProcessor_GetSettings(r->advancedTsm, &processorSettings);
                        BKNI_Memcpy(&processorSettings.settings.advancedTsm, &session->audioProcessingSettings.advancedTsm, sizeof(NEXUS_AudioAdvancedTsmSettings));
                        NEXUS_AudioProcessor_SetSettings(r->advancedTsm, &processorSettings);
                    }
                }
            }

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
                    r->dolbyVolume258 = NEXUS_DolbyVolume258_Open(&session->audioProcessingSettings.dolby.dolbyVolume258);
                    if (r->dolbyVolume258) {
                        rc = NEXUS_DolbyVolume258_AddInput(r->dolbyVolume258, audioInput);
                        if (rc) {
                            BDBG_ERR(("NEXUS_DolbyVolume258_AddInput %d", rc));
                            NEXUS_DolbyVolume258_Close(r->dolbyVolume258);
                            r->dolbyVolume258 = NULL;
                        }
                    }
                }
            }

            if (!cap.dsp.dolbyDigitalReencode) {
                BDBG_ERR(("DDRE not available"));
            }
            else {
                NEXUS_AudioInputHandle audioInput = b_audio_get_pcm_input(r, NEXUS_AudioConnectorType_eMultichannel);
                r->ddre = NEXUS_DolbyDigitalReencode_Open(&session->audioProcessingSettings.dolby.ddre);
                if (r->ddre) {
                    rc = NEXUS_DolbyDigitalReencode_AddInput(r->ddre, audioInput);
                    if (rc) {
                        BDBG_ERR(("NEXUS_DolbyDigitalReencode_AddInput %d", rc));
                        NEXUS_DolbyDigitalReencode_Close(r->ddre);
                        r->ddre = NULL;
                    }
                }
            }
        }
        else if (!b_dolby_ms_capable(session)) { /* Only if MS11/12 is not enabled so we can fall back to legacy for them. */

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
                    r->avl = NEXUS_AutoVolumeLevel_Open(&session->audioProcessingSettings.avl);
                    if (r->avl) {
                        rc = NEXUS_AutoVolumeLevel_AddInput(r->avl, audioInput);
                        if (rc) {
                            BDBG_ERR(("NEXUS_AutoVolumeLevel_AddInput %d", rc));
                            NEXUS_AutoVolumeLevel_Close(r->avl);
                            r->avl = NULL;
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
            return BERR_NOT_SUPPORTED;
        }

        if (nxserverlib_p_session_has_sd_audio(r->session)) {
            if (cap.numOutputs.dac > 0) {
                rc = NEXUS_AudioOutput_AddInput(NEXUS_AudioDac_GetConnector(server->platformConfig.outputs.audioDacs[0]), b_audio_get_pcm_input(r, NEXUS_AudioConnectorType_eStereo));
                BERR_TRACE(rc);
                server->settings.audioOutputs.dacEnabled[0] = true;
            }

            if (cap.numOutputs.i2s > 0 &&
                (server->settings.audioOutputs.dacEnabled[0] == false ||
                 server->settings.session[r->session->index].i2sOutputEnabled[0])) {
                rc = NEXUS_AudioOutput_AddInput(NEXUS_I2sOutput_GetConnector(server->platformConfig.outputs.i2s[0]), b_audio_get_pcm_input(r, NEXUS_AudioConnectorType_eStereo));
                BERR_TRACE(rc);
                server->settings.audioOutputs.i2sEnabled[0] = true;
            }

            if (cap.numOutputs.i2s > 1 && server->settings.session[r->session->index].i2sOutputEnabled[1]) {
                rc = NEXUS_AudioOutput_AddInput(NEXUS_I2sOutput_GetConnector(server->platformConfig.outputs.i2s[1]), b_audio_get_pcm_input(r, NEXUS_AudioConnectorType_eStereo));
                BERR_TRACE(rc);
                server->settings.audioOutputs.i2sEnabled[1] = true;
            }

            /* If there is no DAC or I2S we need a dummy to make sure we can do simul mode / consume PCM from DDRE if configured for encode only */
            /* Need to be careful if when we allow i2s to be configurable and not just pcm stereo to add dummy also */
            if (cap.numOutputs.dac == 0 && cap.numOutputs.i2s == 0 && cap.numOutputs.dummy > 0) {
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
                server->settings.audioOutputs.rfmEnabled = true;
            }
            #endif
        }
        if (cap.numOutputs.spdif > 0) {
            server->settings.audioOutputs.spdifEnabled[0] = true;
        }
        #if NEXUS_HAS_HDMI_OUTPUT
        if (cap.numOutputs.hdmi > 0) {
            server->settings.audioOutputs.hdmiEnabled[0] = true;
        }
        #endif
    }
    return BERR_SUCCESS;
}

void audio_decoder_get_default_create_settings(b_audio_decoder_create_settings * create_settings)
{
    NEXUS_AudioCapabilities cap;

    BDBG_ASSERT(create_settings != NULL);
    BKNI_Memset(create_settings, 0, sizeof(!create_settings));
    NEXUS_GetAudioCapabilities(&cap);
    if ( cap.numDsps > 0 ) {
        if ( cap.dsp.dspSecureDecode ) {
            create_settings->secure = true;
        }
    }
    else if ( cap.numSoftAudioCores > 0 ) /* arm decode only */
    {
        if ( cap.dsp.softAudioSecureDecode ) {
            create_settings->secure = true;
        }
    }
}

BERR_Code audio_decoder_p_create_decoders(struct b_audio_resource *r, enum b_audio_decoder_type type, b_audio_decoder_create_settings * create_settings)
{
    NEXUS_AudioDecoderOpenSettings audioOpenSettings;
    unsigned index;
    struct b_session *session = r->session;
    nxserver_t server = session->server;
    NEXUS_AudioCapabilities cap;

    NEXUS_GetAudioCapabilities(&cap);

    NEXUS_AudioDecoder_GetDefaultOpenSettings(&audioOpenSettings);

    /* Settings common across all decoder types */
    if (server->settings.audioDecoder.fifoSize) {
        audioOpenSettings.fifoSize = server->settings.audioDecoder.fifoSize;
    }
    r->secure = false;
    if ( cap.numDsps > 0 )
    {
        if ( cap.dsp.dspSecureDecode )
        {
            audioOpenSettings.cdbHeap = server->settings.client.heap[NXCLIENT_VIDEO_SECURE_HEAP];
            r->secure = true;
        }
    }
    else if ( cap.numSoftAudioCores > 0 ) /* arm decode only */
    {
        if ( create_settings->secure )
        {
            if ( !cap.dsp.softAudioSecureDecode )
            {
                BDBG_WRN(("WARNING: secure decode requested, but device support does not appear to be present. Attempting secure mode anyway."));
                BERR_TRACE(NEXUS_INVALID_PARAMETER);
            }
            r->secure = true;
            audioOpenSettings.cdbHeap = server->settings.client.heap[NXCLIENT_VIDEO_SECURE_HEAP];
        }
    }

    if (server->settings.svp != nxserverlib_svp_type_none) {
        if (!audioOpenSettings.cdbHeap) {
            return BERR_TRACE(NEXUS_NOT_AVAILABLE);
        }
    }

    if (type == b_audio_decoder_type_regular) {
        r->mode = b_audio_mode_playback;
        if (b_alloc_audio_index(r, &index)) {
            return BERR_TRACE(NEXUS_UNKNOWN);
        }
        audioOpenSettings.karaokeSupported = server->settings.session[session->index].karaoke;
        r->audioDecoder[nxserver_audio_decoder_primary] = NEXUS_AudioDecoder_Open(index, &audioOpenSettings);
        if (!r->audioDecoder[nxserver_audio_decoder_primary]) {
            return BERR_TRACE(NEXUS_UNKNOWN);
        }

        BDBG_MSG(("open AudioDecoder[%u](%p) for %p", nxserver_audio_decoder_primary, (void*)r->audioDecoder[nxserver_audio_decoder_primary], (void*)r));
        if (server->settings.audioDecoder.enablePassthroughBuffer) {
            r->passthroughPlayback = NEXUS_AudioPlayback_Open(NEXUS_ANY_ID, NULL);
            if (!r->passthroughPlayback) {
                return BERR_TRACE(NEXUS_UNKNOWN);
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
    }
    else if ( type == b_audio_decoder_type_persistent ) {
        r->mode = b_audio_mode_decode;
        if (b_alloc_audio_index(r, &index)) {
            return BERR_TRACE(NEXUS_UNKNOWN);
        }
        audioOpenSettings.karaokeSupported = server->settings.session[session->index].karaoke;
        r->audioDecoder[nxserver_audio_decoder_primary] = NEXUS_AudioDecoder_Open(index, &audioOpenSettings);
        if (!r->audioDecoder[nxserver_audio_decoder_primary]) {
            return BERR_TRACE(NEXUS_UNKNOWN);
        }
    }
    else if ( type == b_audio_decoder_type_standalone ) {
        r->mode = b_audio_mode_decode;
        if (b_alloc_audio_index(r, &index)) {
            return BERR_TRACE(NEXUS_UNKNOWN);
        }
        audioOpenSettings.karaokeSupported = server->settings.session[session->index].karaoke;
        r->audioDecoder[nxserver_audio_decoder_primary] = NEXUS_AudioDecoder_Open(index, &audioOpenSettings);
        if (!r->audioDecoder[nxserver_audio_decoder_primary]) {
            return BERR_TRACE(NEXUS_UNKNOWN);
        }
        if (b_alloc_audio_index(r, &index)) {
            return BERR_TRACE(NEXUS_UNKNOWN);
        }
        audioOpenSettings.karaokeSupported = false;
        audioOpenSettings.type = NEXUS_AudioDecoderType_ePassthrough;
        r->audioDecoder[nxserver_audio_decoder_passthrough] = NEXUS_AudioDecoder_Open(index, &audioOpenSettings);
        if (!r->audioDecoder[nxserver_audio_decoder_passthrough]) {
            return BERR_TRACE(NEXUS_UNKNOWN);
        }
    }
    /* transcode */
    else {
        NEXUS_AudioMixerSettings mixerSettings;

        r->mode = b_audio_mode_transcode;
        if (b_alloc_audio_index(r, &index)) {
            return BERR_TRACE(NEXUS_UNKNOWN);
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
            return BERR_TRACE(NEXUS_UNKNOWN);

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
    return BERR_SUCCESS;

}

struct b_audio_resource *audio_decoder_create(struct b_session *session, enum b_audio_decoder_type type, b_audio_decoder_create_settings * create_settings)
{
    unsigned i;
    int rc;
    struct b_audio_resource *r;
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

    rc = audio_decoder_p_create_decoders(r, type, create_settings);
    if (rc) {
        BERR_TRACE(rc);
        goto error;
    }

    if (type == b_audio_decoder_type_regular) {
        if (r->localSession) {
            rc = nxserverlib_p_audio_create_downstream(session, r);
            if (rc) {
                BERR_TRACE(rc);
                goto error;
            }

            /* Open playbacks */
            for (i=0;i<session->server->settings.session[session->index].audioPlaybacks;i++) {
                b_audio_open_pb(r, false, 0);
            }
            if (r->session->index == 0 && session->server->settings.audioInputs.i2sEnabled) {
                for (i=0;i<cap.numInputs.i2s;i++) {
                    b_audio_open_pb(r, true, i);
                }
            }
            /* Connect Open Playbacks to mixers */
            b_audio_connect_pb(r);
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
        NEXUS_SimpleAudioDecoderServerSettings simpleServerSettings;
        rc = bserver_set_audio_config(r, false);
        if (rc) {rc = BERR_TRACE(rc); goto error;}
        NEXUS_SimpleAudioDecoder_GetServerSettings(session->audio.server, r->masterSimpleAudioDecoder, &simpleServerSettings);
        simpleServerSettings.masterHandle = r->masterSimpleAudioDecoder;
        simpleServerSettings.simplePlayback = session->audio.playbackServer;
        rc = NEXUS_SimpleAudioDecoder_SetServerSettings(session->audio.server, r->masterSimpleAudioDecoder, &simpleServerSettings, false);
        if (rc) {rc = BERR_TRACE(rc); goto error;}
    }

    if (r->localSession && type == b_audio_decoder_type_regular) {
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

void nxserverlib_p_audio_destroy_downstream(struct b_audio_resource *r)
{
    unsigned i;
    NEXUS_AudioCapabilities audioCapabilities;
    nxserver_t server = r->session->server;

    NEXUS_GetAudioCapabilities(&audioCapabilities);

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
                server->settings.audioOutputs.dacEnabled[0] = false;
                if (r->dacEqualizer.eqHandle) {
                    b_audio_equalizer_close(&r->dacEqualizer, NEXUS_AudioDac_GetConnector(server->platformConfig.outputs.audioDacs[0]), false);
                }
            }
            if (audioCapabilities.numOutputs.i2s > 0) {
                NEXUS_AudioOutput_RemoveAllInputs(NEXUS_I2sOutput_GetConnector(server->platformConfig.outputs.i2s[0]));
                server->settings.audioOutputs.i2sEnabled[0] = false;
                if (r->i2sEqualizer[0].eqHandle) {
                    b_audio_equalizer_close(&r->i2sEqualizer[0], NEXUS_I2sOutput_GetConnector(server->platformConfig.outputs.i2s[0]), false);
                }
                if (audioCapabilities.numOutputs.i2s > 1) {
                    NEXUS_AudioOutput_RemoveAllInputs(NEXUS_I2sOutput_GetConnector(server->platformConfig.outputs.i2s[1]));
                    server->settings.audioOutputs.i2sEnabled[1] = false;
                    if (r->i2sEqualizer[1].eqHandle) {
                        b_audio_equalizer_close(&r->i2sEqualizer[1], NEXUS_I2sOutput_GetConnector(server->platformConfig.outputs.i2s[1]), false);
                    }
                }
            }
            #if NEXUS_HAS_RFM && NEXUS_NUM_RFM_OUTPUTS
            if (server->platformConfig.outputs.rfm[0]) {
                NEXUS_AudioOutput_RemoveAllInputs(NEXUS_Rfm_GetAudioConnector(server->platformConfig.outputs.rfm[0]));
                server->settings.audioOutputs.rfmEnabled = false;
                 if (r->rfmEqualizer.eqHandle) {
                     b_audio_equalizer_close(&r->rfmEqualizer, NEXUS_Rfm_GetAudioConnector(server->platformConfig.outputs.rfm[0]), false);
                 }
            }
            #endif
            #if NEXUS_HAS_HDMI_OUTPUT
            if (audioCapabilities.numOutputs.hdmi > 0) {
                NEXUS_AudioOutput_RemoveAllInputs(NEXUS_HdmiOutput_GetAudioConnector(server->platformConfig.outputs.hdmi[0]));
                server->settings.audioOutputs.hdmiEnabled[0] = false;
                if (r->hdmiEqualizer.eqHandle) {
                     b_audio_equalizer_close(&r->hdmiEqualizer, NEXUS_HdmiOutput_GetAudioConnector(server->platformConfig.outputs.hdmi[0]), false);
                }
            }
            #endif
            if (audioCapabilities.numOutputs.spdif > 0) {
                NEXUS_AudioOutput_RemoveAllInputs(NEXUS_SpdifOutput_GetConnector(server->platformConfig.outputs.spdif[0]));
                server->settings.audioOutputs.spdifEnabled[0] = false;
                if (r->spdifEqualizer.eqHandle) {
                     b_audio_equalizer_close(&r->spdifEqualizer, NEXUS_SpdifOutput_GetConnector(server->platformConfig.outputs.spdif[0]), false);
                }
            }

        }
        NEXUS_AudioMixer_RemoveAllInputs(r->mixer[nxserver_audio_mixer_stereo]);
    }

    if (r->mixer[nxserver_audio_mixer_multichannel]) {
        NEXUS_AudioMixer_RemoveAllInputs(r->mixer[nxserver_audio_mixer_multichannel]);
    }

    if (r->mixer[nxserver_audio_mixer_persistent]) {
        NEXUS_AudioMixer_RemoveAllInputs(r->mixer[nxserver_audio_mixer_persistent]);
    }

    if (r->audioEncoder) {
        NEXUS_AudioEncoder_Close(r->audioEncoder);
        r->audioEncoder = NULL;
    }
    if (r->ddre) {
        NEXUS_DolbyDigitalReencode_Close(r->ddre);
        r->ddre = NULL;
    }
    if (r->dolbyVolume258)
    {
        NEXUS_DolbyVolume258_Close(r->dolbyVolume258);
        r->dolbyVolume258 = NULL;
    }
    if (r->advancedTsm) {
        NEXUS_AudioProcessor_Close(r->advancedTsm);
        r->advancedTsm = NULL;
    }
    if (r->avl)
    {
        NEXUS_AutoVolumeLevel_Close(r->avl);
        r->avl = NULL;
    }
    if (r->truVolume)
    {
        NEXUS_TruVolume_Close(r->truVolume);
        r->truVolume = NULL;
    }
    if (r->mixer[nxserver_audio_mixer_persistent]) {
        NEXUS_AudioMixer_Close(r->mixer[nxserver_audio_mixer_persistent]);
        r->mixer[nxserver_audio_mixer_persistent] = NULL;
    }
    if (r->mixer[nxserver_audio_mixer_stereo]) {
        NEXUS_AudioMixer_Close(r->mixer[nxserver_audio_mixer_stereo]);
        r->mixer[nxserver_audio_mixer_stereo] = NULL;
    }
    if (r->mixer[nxserver_audio_mixer_multichannel]) {
        NEXUS_AudioMixer_Close(r->mixer[nxserver_audio_mixer_multichannel]);
        r->mixer[nxserver_audio_mixer_multichannel] = NULL;
    }
}

void audio_decoder_p_destroy_decoder(struct b_audio_resource *r)
{
    unsigned i;
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
}

void audio_decoder_destroy(struct b_audio_resource *r)
{
    int rc;
    struct b_audio_playback_resource *pb;

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
            rc = NEXUS_SimpleAudioDecoder_Suspend(r->masterSimpleAudioDecoder);
            if (rc) { BERR_TRACE(rc); }
        }
        NEXUS_SimpleAudioDecoder_Destroy(r->masterSimpleAudioDecoder);
    }

    nxserverlib_p_audio_destroy_downstream(r);

    while ((pb = BLST_Q_FIRST(&r->audioPlaybackList))) {
        b_audio_close_pb(r, pb);
    }
    audio_decoder_p_destroy_decoder(r);
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
    bool secureChange = false;
    NEXUS_AudioCapabilities cap;

    if (!nxserver_p_allow_grab(connect->client)) {
        grab = false;
    }
    if (!connect->settings.simpleAudioDecoder.id) {
        /* no request */
        return 0;
    }

    NEXUS_GetAudioCapabilities(&cap);

    if (req->handles.simpleAudioDecoder.r) {
        if (req->handles.simpleAudioDecoder.r->connect != connect) {
            BDBG_ERR(("already connected to %p", (void*)req->handles.simpleAudioDecoder.r->connect));
            return BERR_TRACE(NEXUS_NOT_AVAILABLE);
        }
        else if (req->handles.simpleAudioDecoder.r->connect->settings.simpleAudioDecoder.decoderCapabilities.type == NxClient_AudioDecoderType_ePersistent) {
            BDBG_MSG(("Persistent Decoder is already acquired"));
            return 0;
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
            /* nxserver_settings.grab == false only prevents inter-client grabbing. allow audio decoders within
            a client to grab, which enables priming. */
            if (connect->client == session->main_audio->connect->client) {
                grab = true;
            }
            if (!grab) {
                return BERR_TRACE(NEXUS_NOT_AVAILABLE);
            }
        }

        r = session->main_audio;

        if (r->secure != connect->settings.simpleAudioDecoder.decoderCapabilities.secure )
        {
            BDBG_MSG(("NEED TO RECREATE DYNAMIC MAIN AUDIO DECODER"));
            secureChange = true;
        }
    }
    else {
        b_audio_decoder_create_settings create_settings;
        b_audio_decoder_type decoder_type = b_audio_decoder_type_background;
        audio_decoder_get_default_create_settings(&create_settings);
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
        create_settings.secure = connect->settings.simpleAudioDecoder.decoderCapabilities.secure;
        r = audio_decoder_create(session, decoder_type, &create_settings);
    }
    if (!r) {
        return BERR_TRACE(NEXUS_NOT_AVAILABLE);
    }

    if (is_main_audio(connect)) {
        NEXUS_SimpleAudioDecoderServerSettings settings;
        NEXUS_SimpleAudioDecoderHandle mainAudioDecoder;
        NEXUS_SimpleAudioDecoder_GetServerSettings(session->audio.server, audioDecoder, &settings);
        settings.type = NEXUS_SimpleAudioDecoderType_eDynamic;
        if (session->audioSettings.dolbyMsAllowed) {
            settings.capabilities.ms12 = (b_dolby_ms(session) == nxserverlib_dolby_ms_type_ms12) ? true : false;
            settings.capabilities.ms11 = (b_dolby_ms(session) == nxserverlib_dolby_ms_type_ms11) ? true : false;
        }
        else {
            settings.capabilities.ms12 = settings.capabilities.ms11 = false;
        }
        rc = NEXUS_SimpleAudioDecoder_SetServerSettings(session->audio.server, audioDecoder, &settings, false);

        mainAudioDecoder = b_audio_get_decoder(session->main_audio->connect);
        if (r->mode == b_audio_mode_decode) {
            audio_release_stc_index(connect, mainAudioDecoder);
            BDBG_MSG(("transfer audio %p: connect %p(%p) -> connect %p(%p)",
                      (void*)r, (void*)session->main_audio->connect, (void*)mainAudioDecoder,
                      (void*)connect, (void*)audioDecoder));
            rc = NEXUS_SimpleAudioDecoder_MoveServerSettings(session->audio.server, mainAudioDecoder, audioDecoder, secureChange ? false : true);
            if (rc) { rc = BERR_TRACE(rc); goto err_setsettings; }

            rc = audio_acquire_stc_index(connect, audioDecoder);
            if (rc) { rc = BERR_TRACE(rc); goto err_setsettings; }

            session->main_audio->connect->req[b_resource_simple_audio_decoder]->handles.simpleAudioDecoder.r = NULL;
        }
        else if (audioDecoder) {
            audio_release_stc_index(connect, r->masterSimpleAudioDecoder);
            BDBG_MSG(("transfer audio %p: playback(%p) -> connect %p(%p)",
                      (void*)r, (void*)r->masterSimpleAudioDecoder, (void*)connect, (void*)audioDecoder));
            rc = NEXUS_SimpleAudioDecoder_MoveServerSettings(session->audio.server, r->masterSimpleAudioDecoder, audioDecoder, secureChange ? false : true);
            if (rc) { rc = BERR_TRACE(rc); goto err_setsettings; }
            rc = audio_acquire_stc_index(connect, audioDecoder);
            if (rc) { rc = BERR_TRACE(rc); goto err_setsettings; }

            r->mode = b_audio_mode_decode;
        }

        session->main_audio->connect = connect;
        if (secureChange) {
            b_audio_decoder_create_settings create_settings;
            audio_decoder_p_destroy_decoder(r);
            audio_decoder_get_default_create_settings(&create_settings);
            create_settings.secure = connect->settings.simpleAudioDecoder.decoderCapabilities.secure;
            rc = audio_decoder_p_create_decoders(r, b_audio_decoder_type_regular, &create_settings);
            if (rc) { rc = BERR_TRACE(rc); goto err_setsettings; }
            r->mode = b_audio_mode_decode;
            rc = bserver_set_audio_config(r, true);
            if (rc) { rc = BERR_TRACE(rc); goto err_setsettings; }
        }

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
        if (session->audioSettings.dolbyMsAllowed) {
            settings.capabilities.ms12 = (b_dolby_ms(session) == nxserverlib_dolby_ms_type_ms12) ? true : false;
            settings.capabilities.ms11 = (b_dolby_ms(session) == nxserverlib_dolby_ms_type_ms11) ? true : false;
        }
        else {
            settings.capabilities.ms12 = settings.capabilities.ms11 = false;
        }
        /* MS12 will not have a persistent mixer, use multichannel mixer */
        if ( server->settings.session[r->session->index].dolbyMs == nxserverlib_dolby_ms_type_ms12 ) {
            settings.mixers.persistent = NULL;
            settings.mixers.multichannel = session->main_audio->mixer[nxserver_audio_mixer_multichannel];
            if (session->audioSettings.dolbyMsAllowed) {
                settings.mixers.stereo = NULL;
            }
            else {
                settings.mixers.stereo = session->main_audio->mixer[nxserver_audio_mixer_stereo];
            }
        }
        else if (session->main_audio->mixer[nxserver_audio_mixer_persistent])
        {
            settings.mixers.multichannel = settings.mixers.persistent = NULL;
            settings.mixers.stereo = session->main_audio->mixer[nxserver_audio_mixer_persistent];
        }
        else {
            BDBG_ERR(("No Persistent Decoder support in this configuration"));
            rc = BERR_TRACE(NEXUS_NOT_AVAILABLE); goto err_setsettings;
        }
        rc = NEXUS_SimpleAudioDecoder_SetServerSettings(session->audio.server, audioDecoder, &settings, false);
        if (rc) { rc = BERR_TRACE(rc); goto err_setsettings; }
        /* TBD - should this be any different for persistent decoders??? */
        {
            int i;
            NEXUS_SimpleAudioDecoderHandle activeDecoder;
            activeDecoder = b_audio_get_active_decoder(session->main_audio);
            NEXUS_SimpleAudioDecoder_GetServerSettings(session->audio.server, activeDecoder, &settings);
            for (i = 0; i < NEXUS_MAX_AUDIO_DECODERS; i++) {
                if (settings.persistent[i].decoder == NULL) {
                    settings.persistent[i].decoder = audioDecoder;
                    settings.persistent[i].suspended = false;
                    break;
                }
            }

            rc = NEXUS_SimpleAudioDecoder_SetServerSettings(session->audio.server, activeDecoder, &settings, false);
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
        if (session->audioSettings.dolbyMsAllowed) {
            settings.capabilities.ms12 = (b_dolby_ms(session) == nxserverlib_dolby_ms_type_ms12) ? true : false;
            settings.capabilities.ms11 = (b_dolby_ms(session) == nxserverlib_dolby_ms_type_ms11) ? true : false;
        }
        else {
            settings.capabilities.ms12 = settings.capabilities.ms11 = false;
        }
        rc = NEXUS_SimpleAudioDecoder_SetServerSettings(session->audio.server, audioDecoder, &settings, false);
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
        if (session->audioSettings.dolbyMsAllowed) {
            settings.capabilities.ms12 = (b_dolby_ms(session) == nxserverlib_dolby_ms_type_ms12) ? true : false;
            settings.capabilities.ms11 = (b_dolby_ms(session) == nxserverlib_dolby_ms_type_ms11) ? true : false;
        }
        else {
            settings.capabilities.ms12 = settings.capabilities.ms11 = false;
        }
        rc = NEXUS_SimpleAudioDecoder_SetServerSettings(session->audio.server, audioDecoder, &settings, false);
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
            if (settings.persistent[i].decoder == audioDecoder) {
                settings.persistent[i].decoder = NULL;
                settings.persistent[i].suspended = false;
                break;
            }
        }
        NEXUS_SimpleAudioDecoder_SetServerSettings(session->audio.server, activeDecoder, &settings, false);
    }
err_setmastersettings:
    if (is_main_audio(connect)) {
        /* failed swap: leave state alone. acquire_stc should never fail because of release_stc. */
    }
    else {
        NEXUS_SimpleAudioDecoderServerSettings settings;
        NEXUS_SimpleAudioDecoder_GetServerSettings(session->audio.server, audioDecoder, &settings);
        settings.primary = NULL;
        (void)NEXUS_SimpleAudioDecoder_SetServerSettings(session->audio.server, audioDecoder, &settings, false);
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
        (void)NEXUS_SimpleAudioDecoder_MoveServerSettings(session->audio.server, mainAudioDecoder, r->masterSimpleAudioDecoder, true);
        session->main_audio->connect = NULL;
        r->mode = b_audio_mode_playback;
    }
    else if (r->mode != b_audio_mode_playback) {
        if (settings.type == NEXUS_SimpleAudioDecoderType_ePersistent) {
            NEXUS_SimpleAudioDecoder_Stop(audioDecoder);
        }
        audio_release_stc_index(connect, audioDecoder);
        BDBG_MSG(("release transcode audio %p: connect %p", (void*)r, (void*)r->connect));
        settings.primary = settings.secondary = NULL;
        (void)NEXUS_SimpleAudioDecoder_SetServerSettings(session->audio.server, audioDecoder, &settings, false);
        if (settings.type == NEXUS_SimpleAudioDecoderType_ePersistent) {
            int i;
            NEXUS_SimpleAudioDecoderServerSettings settings;
            NEXUS_SimpleAudioDecoderHandle activeDecoder;
            activeDecoder = b_audio_get_active_decoder(session->main_audio);

            NEXUS_SimpleAudioDecoder_GetServerSettings(session->audio.server, activeDecoder, &settings);
            for (i = 0; i < NEXUS_MAX_AUDIO_DECODERS; i++) {
                if (settings.persistent[i].decoder == audioDecoder) {
                    settings.persistent[i].decoder = NULL;
                    settings.persistent[i].suspended = false;
                    break;
                }
            }
            NEXUS_SimpleAudioDecoder_SetServerSettings(session->audio.server, activeDecoder, &settings, false);
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

int bserver_set_audio_config(struct b_audio_resource *r, bool forceReconfig)
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
    bool ac3CompressedAllowed = false;
    bool ddpCompressedAllowed = false;
    bool suspended = false;
    bool equalizerChanged = false;
    bool forceLegacy = false;
    bool recreateDownstream = false;

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

    if (r->localSession) {
        if (b_dolby_ms_capable(session)) {
            if ((r->ddre && !session->audioSettings.dolbyMsAllowed) ||
                (!r->ddre && session->audioSettings.dolbyMsAllowed)) {
                if (!suspended) {
                    rc = NEXUS_SimpleAudioDecoder_Suspend(simpleAudioDecoder);
                    if (rc) {
                        return BERR_TRACE(rc);
                    }
                    else {
                        suspended = true;
                    }
                }
                nxserverlib_p_audio_destroy_downstream(r);
                recreateDownstream = true;
            }
            else if (!r->mixer[nxserver_audio_mixer_multichannel]) {
            /* if there is no multi channel mixer then we
               must have had an issue previously, try again */
                recreateDownstream = true;
            }

            if (recreateDownstream) {
                rc = nxserverlib_p_audio_create_downstream(session, r);
                /* Need to figure out a fall back */
                if (rc) {return BERR_TRACE(rc);}
                b_audio_connect_pb(r);
                b_audio_update_persistent_settings(session, r);
            }

            if (!session->audioSettings.dolbyMsAllowed) {
                forceLegacy = true;
            }
        }
    }

    equalizerChanged = b_is_any_equalizer_changed(r);
    if (equalizerChanged) {
        if (!suspended) {
            rc = NEXUS_SimpleAudioDecoder_Suspend(simpleAudioDecoder);
            if (rc) {
                BERR_TRACE(rc);
            }
            else {
                suspended = true;
            }
        }
    }

    b_configure_equalizers(r, suspended);

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

    if (forceLegacy) {
        audioSettings.capabilities.ms12 = audioSettings.capabilities.ms11 = false;
    }
    else {
        audioSettings.capabilities.ms12 = (b_dolby_ms(session) == nxserverlib_dolby_ms_type_ms12) ? true : false;
        audioSettings.capabilities.ms11 = (b_dolby_ms(session) == nxserverlib_dolby_ms_type_ms11) ? true : false;
    }

    /* If there is a captue attached and we had to recreate then reconfigure the capture output */
    if (audioSettings.capture.output != NULL && recreateDownstream) {
        for (i=0;i<cap.numOutputs.capture;i++) {
#if !NEXUS_AUDIO_BUFFER_CAPTURE_EXT
            if (g_capture[i].handle == audioSettings.capture.output) {
                nxserverlib_p_configure_audio_output_capture(session, &audioSettings, g_capture[i].captureType);
            }
#else
            /* Ring buffer capture can't swap without closing. Just close */
            if (g_capture[i].handle) {
                NEXUS_AudioBufferCapture_Stop(g_capture[i].handle);
                NEXUS_AudioBufferCapture_Destroy(g_capture[i].handle);
                g_capture[i].handle = NULL;
            }
#endif
        }
    }

    if (r->localSession) {
        BKNI_Memset(&audioSettings.spdif, 0, sizeof(audioSettings.spdif));
        BKNI_Memset(&audioSettings.hdmi, 0, sizeof(audioSettings.hdmi));
        BKNI_Memset(&audioSettings.dac, 0 , sizeof(audioSettings.dac));
        BKNI_Memset(&audioSettings.i2s, 0 , sizeof(audioSettings.i2s));

#if NEXUS_HAS_HDMI_OUTPUT
        /* set up per codec outputs for HDMI and SPDIF */
        audioSettings.hdmi.outputs[0] = r->session->hdmiOutput;
        if (r->session->hdmiOutput) {
            if ( session->audioSettings.hdmi.transcodeCodec == NEXUS_AudioCodec_eAc3Plus &&
                 ( server->settings.session[r->session->index].dolbyMs != nxserverlib_dolby_ms_type_ms12 ||
                   !cap.dsp.codecs[NEXUS_AudioCodec_eAc3Plus].encode ) ) {
                session->audioSettings.hdmi.transcodeCodec = NEXUS_AudioCodec_eAc3;
            }

            if ( ( config->hdmi.audioCodecOutput[NEXUS_AudioCodec_eAc3Plus] == NxClient_AudioOutputMode_ePassthrough && session->audioSettings.hdmi.compressedOverride[NEXUS_AudioCodec_eAc3Plus] == NxClientAudioCodecSupport_eDefault && config->hdmiAc3Plus) ||
                 session->audioSettings.hdmi.compressedOverride[NEXUS_AudioCodec_eAc3Plus] == NxClientAudioCodecSupport_eEnabled ) {
                ddpCompressedAllowed = true;
            }

            if ( ( config->hdmi.audioCodecOutput[NEXUS_AudioCodec_eAc3] == NxClient_AudioOutputMode_ePassthrough && session->audioSettings.hdmi.compressedOverride[NEXUS_AudioCodec_eAc3] == NxClientAudioCodecSupport_eDefault ) ||
                 session->audioSettings.hdmi.compressedOverride[NEXUS_AudioCodec_eAc3] == NxClientAudioCodecSupport_eEnabled ) {
                ac3CompressedAllowed = true;
            }


            for (i=0;i<NEXUS_AudioCodec_eMax;i++) {
                NxClient_AudioOutputMode outputMode;
                NEXUS_AudioCodec transcodeCodec;
                bool tryPassthrough = false;
                bool tryTranscode = false;
                bool tryMultiPCM  = false;
                bool tryStereoPCM = false;

                if (encode_display && session->audioSettings.hdmi.outputMode != NxClient_AudioOutputMode_ePcm) {
                    BDBG_WRN(("forcing hdmi to pcm output because we are encoding the display"));
                    session->audioSettings.hdmi.outputMode = NxClient_AudioOutputMode_ePcm;
                }

                audioSettings.hdmi.input[i] = NULL;
                outputMode = session->audioSettings.hdmi.outputMode;
                transcodeCodec = session->audioSettings.hdmi.transcodeCodec;

                if ( outputMode == NxClient_AudioOutputMode_eAuto ||
                     outputMode == NxClient_AudioOutputMode_ePassthrough ) {
                    outputMode = config->hdmi.audioCodecOutput[i];

                    switch (session->audioSettings.hdmi.compressedOverride[i]) {
                    case NxClientAudioCodecSupport_eEnabled: /* Do compressed no matter if we can support it */
                        outputMode = NxClient_AudioOutputMode_ePassthrough;
                        break;
                    case NxClientAudioCodecSupport_eDisabled: /* If we were compressed fall back, if transcode mabe we can still do that */
                        if (outputMode == NxClient_AudioOutputMode_ePassthrough) {
                            if (i != NEXUS_AudioCodec_eAc3Plus || !ac3CompressedAllowed) {
                                outputMode = NxClient_AudioOutputMode_ePcm;
                            }
                        }
                        break;
                    default:
                        break;
                    }

                    if (i == NEXUS_AudioCodec_eAc3Plus && (!ddpCompressedAllowed) &&
                        server->settings.session[r->session->index].dolbyMs == nxserverlib_dolby_ms_type_ms12 && r->ddre ) {
                        outputMode = NxClient_AudioOutputMode_ePcm;
                    }

                    if (outputMode == NxClient_AudioOutputMode_ePcm) {
                        outputMode = (session->audioSettings.hdmi.outputMode == NxClient_AudioOutputMode_eAuto) ?
                            NxClient_AudioOutputMode_eTranscode : NxClient_AudioOutputMode_eMultichannelPcm;
                    }
                }

                switch (outputMode) {
                case NxClient_AudioOutputMode_ePassthrough: /* Passthrough -> Multi PCM -> Stereo PCM */
                    tryPassthrough = tryMultiPCM = tryStereoPCM = true;
                    break;
                case NxClient_AudioOutputMode_eTranscode: /* Transcode -> Multi PCM -> Stereo PCM */
                    tryTranscode = tryMultiPCM = tryStereoPCM = true;
                    break;
                case NxClient_AudioOutputMode_eMultichannelPcm: /* Multi PCM -> Stereo PCM */
                    tryMultiPCM = tryStereoPCM = true;
                    break;
                default:
                case NxClient_AudioOutputMode_ePcm: /* Stereo PCM */
                    tryStereoPCM = true;
                    break;
                 case NxClient_AudioOutputMode_eNone: /* None don't connect anything*/
                    break;
                }

                if (tryPassthrough) {
                    if (i == NEXUS_AudioCodec_eAc3Plus && (!ddpCompressedAllowed)) {
                        if (r->audioDecoder[nxserver_audio_decoder_primary] && ac3CompressedAllowed) {
                            /* AC3+ --> AC3 downconvert */
                            audioSettings.hdmi.input[i] = NEXUS_AudioDecoder_GetConnector(r->audioDecoder[nxserver_audio_decoder_primary], NEXUS_AudioConnectorType_eCompressed);
                            session->audio.hdmi.outputMode[i] = NxClient_AudioOutputMode_ePassthrough;
                        }
                    }
                    else if (r->audioDecoder[nxserver_audio_decoder_passthrough]) {
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
                        session->audio.hdmi.outputMode[i] = NxClient_AudioOutputMode_ePassthrough;
                    }
                }

                if (tryTranscode && audioSettings.hdmi.input[i] == NULL) {
                    if (b_dolby_ms_capable(session) && !forceLegacy) {
                        if ( (transcodeCodec != NEXUS_AudioCodec_eAc3 && transcodeCodec != NEXUS_AudioCodec_eAc3Plus) || !r->ddre ) {
                            return BERR_TRACE(NEXUS_NOT_SUPPORTED);
                        }
                        if ( server->settings.session[r->session->index].dolbyMs == nxserverlib_dolby_ms_type_ms12 ) {
                            /* Try AC3 Plus Encode */
                            if ( transcodeCodec == NEXUS_AudioCodec_eAc3Plus && ddpCompressedAllowed ) {
                                audioSettings.hdmi.input[i] = NEXUS_DolbyDigitalReencode_GetConnector(r->ddre, NEXUS_AudioConnectorType_eCompressed4x);
                                session->audio.hdmi.outputMode[i] = NxClient_AudioOutputMode_eTranscode;
                            }
                        }
                        /* Try AC3 Encode */
                        if ( audioSettings.hdmi.input[i] == NULL )
                        {
                            if ( ac3CompressedAllowed ) {
                                audioSettings.hdmi.input[i] = NEXUS_DolbyDigitalReencode_GetConnector(r->ddre, NEXUS_AudioConnectorType_eCompressed);
                                session->audio.hdmi.outputMode[i] = NxClient_AudioOutputMode_eTranscode;
                            }
                        }
                    }
                    else if (r->audioEncoder && transcodeCodec < NEXUS_AudioCodec_eMax && cap.dsp.codecs[transcodeCodec].encode) {
                        if ( transcodeCodec != NEXUS_AudioCodec_eAc3 ||
                             ( b_is_aac(i) && ac3CompressedAllowed ) ) {

                            NEXUS_AudioEncoderSettings settings;
                            NEXUS_AudioEncoder_GetSettings(r->audioEncoder, &settings);
                            settings.codec = transcodeCodec;
                            rc = NEXUS_AudioEncoder_SetSettings(r->audioEncoder, &settings);
                            if (!rc) {
                                audioSettings.hdmi.input[i] = NULL;
                                if (config->hdmi.audioCodecOutput[transcodeCodec] == NxClient_AudioOutputMode_ePassthrough)
                                {
                                    audioSettings.hdmi.input[i] = NEXUS_AudioEncoder_GetConnector(r->audioEncoder);
                                    session->audio.hdmi.outputMode[i] = NxClient_AudioOutputMode_eTranscode;
                                }
                            }
                        }
                    }
                }

                if (tryMultiPCM && audioSettings.hdmi.input[i] == NULL && config->maxPCMChannels >= 6) {
                    audioSettings.hdmi.input[i] = b_audio_get_pcm_input(r, NEXUS_AudioConnectorType_eMultichannel);
                    session->audio.hdmi.outputMode[i] = NxClient_AudioOutputMode_eMultichannelPcm;
                }

                if (tryStereoPCM && audioSettings.hdmi.input[i] == NULL) {
                    audioSettings.hdmi.input[i] = b_audio_get_pcm_input(r, NEXUS_AudioConnectorType_eStereo);
                    session->audio.hdmi.outputMode[i] = NxClient_AudioOutputMode_ePcm;
                }

                if (audioSettings.hdmi.input[i] == NULL) {
                    audioSettings.hdmi.input[i] = NULL;
                    session->audio.hdmi.outputMode[i] = NxClient_AudioOutputMode_eNone;
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
                if ( session->audioSettings.spdif.transcodeCodec == NEXUS_AudioCodec_eAc3Plus &&
                     ( server->settings.session[r->session->index].dolbyMs != nxserverlib_dolby_ms_type_ms12 ||
                       !cap.dsp.codecs[NEXUS_AudioCodec_eAc3Plus].encode ||
                       !server->settings.session[r->session->index].allowSpdif4xCompressed) ) {
                    session->audioSettings.spdif.transcodeCodec = NEXUS_AudioCodec_eAc3;
                }

                ddpCompressedAllowed = ac3CompressedAllowed = false;

                if ( ( ( config->spdif.audioCodecOutput[NEXUS_AudioCodec_eAc3Plus] == NxClient_AudioOutputMode_ePassthrough && session->audioSettings.spdif.compressedOverride[NEXUS_AudioCodec_eAc3Plus] == NxClientAudioCodecSupport_eDefault ) ||
                       session->audioSettings.spdif.compressedOverride[NEXUS_AudioCodec_eAc3Plus] == NxClientAudioCodecSupport_eEnabled ) &&
                     server->settings.session[r->session->index].allowSpdif4xCompressed) {
                    ddpCompressedAllowed = true;
                }

                if ( ( config->spdif.audioCodecOutput[NEXUS_AudioCodec_eAc3] == NxClient_AudioOutputMode_ePassthrough && session->audioSettings.spdif.compressedOverride[NEXUS_AudioCodec_eAc3] == NxClientAudioCodecSupport_eDefault ) ||
                     session->audioSettings.spdif.compressedOverride[NEXUS_AudioCodec_eAc3] == NxClientAudioCodecSupport_eEnabled ) {
                    ac3CompressedAllowed = true;
                }

                for (i=0;i<NEXUS_AudioCodec_eMax;i++) {
                    NxClient_AudioOutputMode outputMode;
                    NEXUS_AudioCodec transcodeCodec;
                    bool tryPassthrough = false;
                    bool tryTranscode = false;
                    bool tryStereoPCM = false;

                    if (encode_display && session->audioSettings.spdif.outputMode != NxClient_AudioOutputMode_ePcm) {
                        BDBG_WRN(("forcing spdif to pcm output because we are encoding the display"));
                        session->audioSettings.spdif.outputMode = NxClient_AudioOutputMode_ePcm;
                    }


                    audioSettings.spdif.input[i] = NULL;
                    outputMode = session->audioSettings.spdif.outputMode;
                    transcodeCodec = session->audioSettings.spdif.transcodeCodec;

                    if ( outputMode == NxClient_AudioOutputMode_eAuto ||
                         outputMode == NxClient_AudioOutputMode_ePassthrough ) {
                        outputMode = config->spdif.audioCodecOutput[i];

                        switch (session->audioSettings.hdmi.compressedOverride[i]) {
                        case NxClientAudioCodecSupport_eEnabled: /* Do compressed no matter if we can support it */
                            outputMode = NxClient_AudioOutputMode_ePassthrough;
                            break;
                        case NxClientAudioCodecSupport_eDisabled: /* If we were compressed fall back, if transcode mabe we can still do that */
                            if (outputMode == NxClient_AudioOutputMode_ePassthrough) {
                                if (i != NEXUS_AudioCodec_eAc3Plus || !ac3CompressedAllowed) {
                                    outputMode = NxClient_AudioOutputMode_ePcm;
                                }
                            }
                            break;
                        default:
                            break;
                        }

                        if (i == NEXUS_AudioCodec_eAc3Plus && (!ddpCompressedAllowed) &&
                            server->settings.session[r->session->index].dolbyMs == nxserverlib_dolby_ms_type_ms12 && r->ddre ) {
                            outputMode = NxClient_AudioOutputMode_ePcm;
                        }

                        if (session->audioSettings.spdif.outputMode == NxClient_AudioOutputMode_eAuto &&
                            outputMode == NxClient_AudioOutputMode_ePcm) {
                            outputMode = NxClient_AudioOutputMode_eTranscode;
                        }
                    }
                    switch (outputMode) {
                    case NxClient_AudioOutputMode_ePassthrough: /* Passthrough -> Stereo PCM */
                        tryPassthrough = tryStereoPCM = true;
                        break;
                    case NxClient_AudioOutputMode_eTranscode: /* Transcode -> Stereo PCM */
                        tryTranscode = tryStereoPCM = true;
                        break;
                    case NxClient_AudioOutputMode_eMultichannelPcm:
                        BDBG_WRN(("eMultichannelPcm not supported for spdif.  Fall back to ePCM"));
                        /* fall through */
                    default:
                    case NxClient_AudioOutputMode_ePcm: /* Stereo PCM */
                        tryStereoPCM = true;
                        break;

                    case NxClient_AudioOutputMode_eNone: /* None don't connect anything*/
                        break;
                    }


                    if (tryPassthrough) {
                        if (i==NEXUS_AudioCodec_eAc3Plus) {
                            if ( r->audioDecoder[nxserver_audio_decoder_passthrough] && ddpCompressedAllowed ) {
                                audioSettings.spdif.input[i] = NEXUS_AudioDecoder_GetConnector(r->audioDecoder[nxserver_audio_decoder_passthrough], NEXUS_AudioConnectorType_eCompressed);
                                session->audio.spdif.outputMode[i] = NxClient_AudioOutputMode_ePassthrough;
                            }
                            else if (r->audioDecoder[nxserver_audio_decoder_primary] && ac3CompressedAllowed) {
                                /* AC3+ --> AC3 downconvert */
                                audioSettings.spdif.input[i] = NEXUS_AudioDecoder_GetConnector(r->audioDecoder[nxserver_audio_decoder_primary], NEXUS_AudioConnectorType_eCompressed);
                                session->audio.spdif.outputMode[i] = NxClient_AudioOutputMode_ePassthrough;
                            }
                        } else if (r->audioDecoder[nxserver_audio_decoder_passthrough]) {
                            audioSettings.spdif.input[i] = NEXUS_AudioDecoder_GetConnector(r->audioDecoder[nxserver_audio_decoder_passthrough], NEXUS_AudioConnectorType_eCompressed);
                            session->audio.spdif.outputMode[i] = NxClient_AudioOutputMode_ePassthrough;
                        }

                    }

                    if (tryTranscode && audioSettings.spdif.input[i] == NULL) {
                        if (b_dolby_ms_capable(session) && !forceLegacy) {
                            if ( (transcodeCodec != NEXUS_AudioCodec_eAc3 && transcodeCodec != NEXUS_AudioCodec_eAc3Plus) || !r->ddre ) {
                                return BERR_TRACE(NEXUS_NOT_SUPPORTED);
                            }
                            if ( server->settings.session[r->session->index].dolbyMs == nxserverlib_dolby_ms_type_ms12 && ddpCompressedAllowed ) {
                                /* Try AC3 Plus Encode */
                                if ( transcodeCodec == NEXUS_AudioCodec_eAc3Plus ) {
                                    audioSettings.spdif.input[i] = NEXUS_DolbyDigitalReencode_GetConnector(r->ddre, NEXUS_AudioConnectorType_eCompressed4x);
                                    session->audio.spdif.outputMode[i] = NxClient_AudioOutputMode_eTranscode;
                                }
                            }
                            /* Try AC3 Encode */
                            if ( audioSettings.spdif.input[i] == NULL && ac3CompressedAllowed) {
                                audioSettings.spdif.input[i] = NEXUS_DolbyDigitalReencode_GetConnector(r->ddre, NEXUS_AudioConnectorType_eCompressed);
                                session->audio.spdif.outputMode[i] = NxClient_AudioOutputMode_eTranscode;
                            }
                        }
                        else if (r->audioEncoder && transcodeCodec < NEXUS_AudioCodec_eMax && cap.dsp.codecs[transcodeCodec].encode) {
                            if ( transcodeCodec != NEXUS_AudioCodec_eAc3 ||
                                 ( b_is_aac(i) && ac3CompressedAllowed ) ) {
                                NEXUS_AudioEncoderSettings settings;
                                NEXUS_AudioEncoder_GetSettings(r->audioEncoder, &settings);
                                /* TODO: can't allow different hdmi/spdif transcode codecs with only one AudioEncoder. for now, last one wins. */
                                settings.codec = transcodeCodec;
                                rc = NEXUS_AudioEncoder_SetSettings(r->audioEncoder, &settings);
                                if (!rc) {
                                    audioSettings.spdif.input[i] = NEXUS_AudioEncoder_GetConnector(r->audioEncoder);
                                    session->audio.spdif.outputMode[i] = NxClient_AudioOutputMode_eTranscode;
                                }
                            }
                        }
                    }

                    if (tryStereoPCM && audioSettings.spdif.input[i] == NULL) {
                        audioSettings.spdif.input[i] = b_audio_get_pcm_input(r, NEXUS_AudioConnectorType_eStereo);
                        session->audio.spdif.outputMode[i] = NxClient_AudioOutputMode_ePcm;
                    }

                    if (audioSettings.spdif.input[i] == NULL) {
                        session->audio.spdif.outputMode[i] = NxClient_AudioOutputMode_eNone;
                    }
                }
            }
        }
        if (r->session->index == 0) {
            if (r->session->server->settings.audioOutputs.dacEnabled[0]) {
                audioSettings.dac.input = b_audio_get_pcm_input(r, NEXUS_AudioConnectorType_eStereo);
                audioSettings.dac.output = server->platformConfig.outputs.audioDacs[0];
                audioSettings.dac.presentation = session->audioSettings.dac.presentation;
            }
            if (r->session->server->settings.audioOutputs.i2sEnabled[0]) {
                audioSettings.i2s[0].input = b_audio_get_pcm_input(r, NEXUS_AudioConnectorType_eStereo);
                audioSettings.i2s[0].output = server->platformConfig.outputs.i2s[0];
                if (nxserverlib_p_audio_i2s0_shares_with_dac(session)) {
                    audioSettings.i2s[0].presentation = session->audioSettings.dac.presentation;
                }
                else {
                    audioSettings.i2s[0].presentation = session->audioSettings.i2s[0].presentation;;
                }
            }
            if (r->session->server->settings.audioOutputs.i2sEnabled[1]) {
                audioSettings.i2s[1].input = b_audio_get_pcm_input(r, NEXUS_AudioConnectorType_eStereo);
                audioSettings.i2s[1].output = server->platformConfig.outputs.i2s[1];
                audioSettings.i2s[1].presentation = session->audioSettings.i2s[1].presentation;;
            }
        }
    }

    /* If we are using Dolby MS, the only conection from Decoder->DspMixer is via Multichannel
       NOTE - if no DAC is present, this may need to fall back further -- to compressed? */
    if (b_dolby_ms_capable(session)) {
        audioSettings.syncConnector = NEXUS_AudioConnectorType_eMultichannel;
    }
    else {
        audioSettings.syncConnector = NEXUS_AudioConnectorType_eStereo;
    }

    rc = NEXUS_SimpleAudioDecoder_SetServerSettings(session->audio.server, simpleAudioDecoder, &audioSettings, forceReconfig);
    if (rc) { BERR_TRACE(rc); }
    if (suspended) {
        rc |= NEXUS_SimpleAudioDecoder_Resume(simpleAudioDecoder);
    }
    if (rc) { return BERR_TRACE(rc); }

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
        if (session->nxclient.displaySettings.hdmiPreferences.preventUnsupportedFormat) {
            config->hdmi.audioCodecOutput[i] = NxClient_AudioOutputMode_ePcm;
            if (pStatus->audioCodecSupported[i]) {
                switch (i) {
                case NEXUS_AudioCodec_ePcm:
                    config->maxPCMChannels = pStatus->maxAudioPcmChannels;
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
                    if ( capabilities.dsp.codecs[NEXUS_AudioCodec_eAc3].encode && pStatus->audioCodecSupported[NEXUS_AudioCodec_eAc3] ) {
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
        else {
            switch (i) {
            case NEXUS_AudioCodec_ePcm:
                config->hdmi.audioCodecOutput[i] = NxClient_AudioOutputMode_ePcm;
                config->maxPCMChannels = 8;
                BDBG_WRN(("It is not recommended that preventUnsupportedFormat be disabled for production."));
                break;
            case NEXUS_AudioCodec_eAc3Plus:
                config->hdmiAc3Plus = true;
                config->hdmi.audioCodecOutput[i] = NxClient_AudioOutputMode_ePassthrough;
                break;
            default:
                config->hdmi.audioCodecOutput[i] = NxClient_AudioOutputMode_ePassthrough;
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
#if !NEXUS_AUDIO_BUFFER_CAPTURE_EXT
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
#endif
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
#if NEXUS_HAS_HDMI_OUTPUT
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
#if !NEXUS_AUDIO_BUFFER_CAPTURE_EXT
     BKNI_Memcpy(&server->settings.audioCapture.clockSources, &config.audioCapture, sizeof(NEXUS_AudioOutputClockSource)*NEXUS_MAX_AUDIO_CAPTURE_OUTPUTS);
#endif
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

#if NEXUS_AUDIO_BUFFER_CAPTURE_EXT
void nxserverlib_p_configure_audio_buffer_capture(struct b_session *session, NEXUS_AudioBufferCaptureCreateSettings *captureSettings, NxClient_AudioCaptureType captureType)
{
    NEXUS_AudioCapabilities capabilities;
    NEXUS_GetAudioCapabilities(&capabilities);

    switch (captureType) {
    case NxClient_AudioCaptureType_e16BitStereo:
    case NxClient_AudioCaptureType_e24BitStereo:
        captureSettings->input = b_audio_get_pcm_input(session->main_audio, NEXUS_AudioConnectorType_eStereo);
        break;
    case NxClient_AudioCaptureType_e24Bit5_1:
        captureSettings->input = b_audio_get_pcm_input(session->main_audio, NEXUS_AudioConnectorType_eMultichannel);
        captureSettings->numChannels = 6;
        break;
    case NxClient_AudioCaptureType_eCompressed:
        captureSettings->input = b_audio_get_compressed_input(session->main_audio, NEXUS_AudioConnectorType_eCompressed);
        break;
    case NxClient_AudioCaptureType_eCompressed4x:
        if (session->server->settings.session[session->index].dolbyMs == nxserverlib_dolby_ms_type_ms12 &&
            session->audioSettings.dolbyMsAllowed && capabilities.dsp.codecs[NEXUS_AudioCodec_eAc3Plus].encode) {
                captureSettings->input = b_audio_get_compressed_input(session->main_audio, NEXUS_AudioConnectorType_eCompressed4x);
            }
        else {
            captureSettings->input = b_audio_get_compressed_input(session->main_audio, NEXUS_AudioConnectorType_eCompressed);
        }
        break;
    default:
        return;
    }
    return;
}

NEXUS_AudioBufferCaptureHandle nxserverlib_p_open_audio_buffer_capture(struct b_session *session, unsigned *id, NxClient_AudioCaptureType captureType)
{
    unsigned i;
    NEXUS_AudioCapabilities audioCapabilities;
    int rc;
    NEXUS_AudioBufferCaptureCreateSettings captureSettings;
    NEXUS_AudioBufferCaptureHandle handle;

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

    NEXUS_AudioBufferCapture_GetDefaultCreateSettings(&captureSettings);
    captureSettings.channelBufferSize = 1536*1024*4;
    nxserverlib_p_configure_audio_buffer_capture(session, &captureSettings, captureType);

    if (captureSettings.input == NULL) {
        return NULL;
    }

    handle = NEXUS_AudioBufferCapture_Create(&captureSettings);
    if (!handle) {
        return NULL;
    }
    rc = NEXUS_AudioBufferCapture_Start(handle);
    if (rc) {
        NEXUS_AudioBufferCapture_Destroy(handle);
        return NULL;
    }

    g_capture[i].handle = handle;
    g_capture[i].session = session;
    g_capture[i].captureType = captureType;
    *id = i;
    return handle;
}

NEXUS_AudioBufferCaptureHandle nxserverlib_open_audio_capture(struct b_session *session, unsigned *id, NxClient_AudioCaptureType captureType)
{
    return nxserverlib_p_open_audio_buffer_capture(session, id, captureType);
}

#else
static void nxserverlib_p_configure_audio_output_capture(struct b_session *session, NEXUS_SimpleAudioDecoderServerSettings *sessionSettings, NxClient_AudioCaptureType captureType)
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
            else if (b_dolby_ms_capable(session)) {
                transcodeCodec = NEXUS_AudioCodec_eAc3;
                outputMode = NxClient_AudioOutputMode_eTranscode;
            }
            else
            {
                outputMode = NxClient_AudioOutputMode_ePassthrough;
            }
        }
        else if (captureType == NxClient_AudioCaptureType_eCompressed) {
            if (b_dolby_ms_capable(session)) {
                transcodeCodec = NEXUS_AudioCodec_eAc3;
                outputMode = NxClient_AudioOutputMode_eTranscode;
            }
            else if (b_is_aac(i)) {
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
            if (b_dolby_ms_capable(session)) {
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
        default: /* unreachable */
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

NEXUS_AudioCaptureHandle nxserverlib_p_open_audio_output_capture(struct b_session *session, unsigned *id, NxClient_AudioCaptureType captureType)
{
        unsigned i;
    NEXUS_AudioCapabilities audioCapabilities;
    int rc;
    NEXUS_AudioCaptureHandle handle;
    NEXUS_AudioCaptureOpenSettings settings;
    NEXUS_SimpleAudioDecoderHandle audioDecoder;
    NEXUS_SimpleAudioDecoderServerSettings sessionSettings;
    NEXUS_AudioOutputSettings outputSettings;

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

    rc = NEXUS_SimpleAudioDecoder_Suspend(audioDecoder);
    if (rc) {
        BERR_TRACE(rc);
        BDBG_ERR(("unable to suspend to open audio capture"));
        goto err_suspend;
    }

    NEXUS_AudioOutput_GetSettings(NEXUS_AudioCapture_GetConnector(handle), &outputSettings);
    outputSettings.pll = session->server->settings.audioCapture.clockSources[i].pll;
    outputSettings.nco = session->server->settings.audioCapture.clockSources[i].nco;
    NEXUS_AudioOutput_SetSettings(NEXUS_AudioCapture_GetConnector(handle), &outputSettings);

    NEXUS_SimpleAudioDecoder_GetServerSettings(session->audio.server, audioDecoder, &sessionSettings);
    sessionSettings.capture.output = handle;
    nxserverlib_p_configure_audio_output_capture(session, &sessionSettings, captureType);
    rc = NEXUS_SimpleAudioDecoder_SetServerSettings(session->audio.server, audioDecoder, &sessionSettings, false);
    if ( rc ) goto err_add_mixer;

    rc = NEXUS_SimpleAudioDecoder_Resume(audioDecoder);
    if (rc) {rc = BERR_TRACE(rc);}

    g_capture[i].session = session;
    g_capture[i].handle = handle;
    g_capture[i].captureType = captureType;
    *id = i;
    return handle;

err_add_mixer:
    rc = NEXUS_SimpleAudioDecoder_Resume(audioDecoder);
    if (rc) {rc = BERR_TRACE(rc);}
err_suspend:
    NEXUS_AudioCapture_Close(handle);
    return NULL;
}

NEXUS_AudioCaptureHandle nxserverlib_open_audio_capture(struct b_session *session, unsigned *id, NxClient_AudioCaptureType captureType)
{
    return nxserverlib_p_open_audio_output_capture(session, id, captureType);
}
#endif

void nxserverlib_close_audio_capture(struct b_session *session,unsigned id)
{
    NEXUS_AudioCapabilities audioCapabilities;
#if !NEXUS_AUDIO_BUFFER_CAPTURE_EXT
    NEXUS_SimpleAudioDecoderHandle audioDecoder;
    NEXUS_SimpleAudioDecoderServerSettings sessionSettings;
    int rc;
    NEXUS_GetAudioCapabilities(&audioCapabilities);

    if (id < audioCapabilities.numOutputs.capture) {
        BDBG_ASSERT(g_capture[id].handle);

        audioDecoder = b_audio_get_active_decoder(session->main_audio);
        rc = NEXUS_SimpleAudioDecoder_Suspend(audioDecoder);
        if (rc) {rc = BERR_TRACE(rc);}
        NEXUS_AudioOutput_RemoveAllInputs(NEXUS_AudioCapture_GetConnector(g_capture[id].handle));
        NEXUS_SimpleAudioDecoder_GetServerSettings(session->audio.server, audioDecoder, &sessionSettings);
        sessionSettings.capture.output = NULL;
        NEXUS_SimpleAudioDecoder_SetServerSettings(session->audio.server, audioDecoder, &sessionSettings, false);

        NEXUS_AudioCapture_Close(g_capture[id].handle);
        g_capture[id].handle = NULL;

        rc = NEXUS_SimpleAudioDecoder_Resume(audioDecoder);
        if (rc) {rc = BERR_TRACE(rc);}
    }
#else
    NEXUS_GetAudioCapabilities(&audioCapabilities);
    BSTD_UNUSED(session);

    if (id < audioCapabilities.numOutputs.capture) {
        BDBG_ASSERT(g_capture[id].handle);
        NEXUS_AudioBufferCapture_Stop(g_capture[id].handle);
        NEXUS_AudioBufferCapture_Destroy(g_capture[id].handle);
        g_capture[id].handle = NULL;
    }
#endif
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
    int rc;

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
    #if NEXUS_HAS_HDMI_OUTPUT
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
#if NEXUS_HAS_HDMI_OUTPUT
            if (crcType == NxClient_AudioCrcType_eMultichannel) {
                inputSettings.output = NEXUS_HdmiOutput_GetAudioConnector(session->hdmiOutput);
            }
            else
#endif
            {
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
    #if NEXUS_HAS_HDMI_OUTPUT
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

            rc = NEXUS_SimpleAudioDecoder_Suspend(audioDecoder);
            if (rc) {rc = BERR_TRACE(rc);}
            rc = NEXUS_SimpleAudioDecoder_Resume(audioDecoder);
            if (rc) {rc = BERR_TRACE(rc);}

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
    int rc;

    NEXUS_GetAudioCapabilities(&cap);

    if (cap.numCrcs > 0)
    {
        BDBG_ASSERT(id < NxClient_AudioCrcType_eMax && g_crc[id].handle);

        audioDecoder = b_audio_get_active_decoder(session->main_audio);
        rc = NEXUS_SimpleAudioDecoder_Suspend(audioDecoder);
        if (rc) {rc = BERR_TRACE(rc);}

        NEXUS_AudioCrc_ClearInput(g_crc[id].handle);
        NEXUS_AudioCrc_Close(g_crc[id].handle);
        g_crc[id].handle = NULL;

        rc = NEXUS_SimpleAudioDecoder_Resume(audioDecoder);
        if (rc) {rc = BERR_TRACE(rc);}
    }
}

void nxserverlib_p_restart_audio(struct b_session *session)
{
    NEXUS_SimpleAudioDecoderHandle audioDecoder;
    int rc;
    audioDecoder = b_audio_get_active_decoder(session->main_audio);
    rc = NEXUS_SimpleAudioDecoder_Suspend(audioDecoder);
    if (rc) {rc = BERR_TRACE(rc);}
    rc = NEXUS_SimpleAudioDecoder_Resume(audioDecoder);
    if (rc) {rc = BERR_TRACE(rc);}
}

int nxserverlib_audio_get_status(struct b_session *session, NxClient_AudioStatus *pStatus)
{
    enum nxserverlib_dolby_ms_type dolbyMs;
    NEXUS_AudioCodec currentCodec = NEXUS_AudioCodec_ePcm;
    NEXUS_AudioDecoderStatus decoderStatus;
    int rc;

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

    pStatus->hdmi.outputMode = session->audio.hdmi.outputMode[currentCodec];
    pStatus->hdmi.channelMode = session->audioSettings.hdmi.channelMode;
    switch (pStatus->hdmi.outputMode) {
    case NxClient_AudioOutputMode_ePassthrough: pStatus->hdmi.outputCodec = (currentCodec==NEXUS_AudioCodec_ePcm)?NEXUS_AudioCodec_eUnknown:currentCodec; break;
    case NxClient_AudioOutputMode_eTranscode:   pStatus->hdmi.outputCodec = session->audioSettings.hdmi.transcodeCodec; break;
    case NxClient_AudioOutputMode_eAuto: BDBG_ERR(("unexpected hdmi NxClient_AudioOutputMode_eAuto state")); /* fall through */
    default: pStatus->hdmi.outputCodec = NEXUS_AudioCodec_ePcm; break;
    }

    pStatus->spdif.outputMode = session->audio.spdif.outputMode[currentCodec];
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

    pStatus->i2s[0].outputMode = NxClient_AudioOutputMode_ePcm;
    pStatus->i2s[0].channelMode = session->audioSettings.i2s[0].channelMode;
    pStatus->i2s[0].outputCodec = NEXUS_AudioCodec_ePcm;

    pStatus->i2s[1].outputMode = NxClient_AudioOutputMode_ePcm;
    pStatus->i2s[1].channelMode = session->audioSettings.i2s[1].channelMode;
    pStatus->i2s[1].outputCodec = NEXUS_AudioCodec_ePcm;

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
    bool restart = false;
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
    if (session->main_audio->advancedTsm) {
        NEXUS_AudioProcessorSettings currentAudioProcessorSettings;
        NEXUS_AudioProcessor_GetSettings(session->main_audio->advancedTsm, &currentAudioProcessorSettings);
        if ( 0 != BKNI_Memcmp(&currentAudioProcessorSettings.settings.advancedTsm, &pSettings->advancedTsm, sizeof(NEXUS_AudioAdvancedTsmSettings)) ) {
            BKNI_Memcpy(&currentAudioProcessorSettings.settings.advancedTsm, &pSettings->advancedTsm, sizeof(NEXUS_AudioAdvancedTsmSettings));
            rc = NEXUS_AudioProcessor_SetSettings(session->main_audio->advancedTsm, &currentAudioProcessorSettings);
            if (rc) return BERR_TRACE(rc);
            restart = true;
        }
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
            rc = NEXUS_AudioMixer_SetSettings(session->main_audio->mixer[nxserver_audio_mixer_multichannel], &mixerSettings);
            if (rc) return BERR_TRACE(rc);
        }
    }
    session->audioProcessingSettings = *pSettings;
    if (restart) {
        nxserverlib_p_restart_audio(session);
    }
    return 0;
}

static int swap_persistent_audio_decoders(struct b_connect *destConnect, struct b_connect *srcConnect)
{

    NEXUS_SimpleAudioDecoderServerSettings settings;
    NEXUS_SimpleAudioDecoderHandle srcDecoder, destDecoder;
    struct b_session *session;
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
    }
    else
    {
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }

    srcDecoder = b_audio_get_decoder(srcConnect);
    destDecoder = b_audio_get_decoder(destConnect);
    audio_release_stc_index(srcConnect, srcDecoder);
    BDBG_MSG(("transfer audio %p: connect %p(%p) -> connect %p(%p)",
              (void*)r, (void*)srcConnect, (void*)srcDecoder,
              (void*)destConnect, (void*)destDecoder));
    rc = NEXUS_SimpleAudioDecoder_MoveServerSettings(session->audio.server, srcDecoder, destDecoder, true);
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
            if (settings.persistent[i].decoder == srcDecoder) {
                settings.persistent[i].decoder = NULL;
                settings.persistent[i].suspended = false;
                break;
            }
        }
        NEXUS_SimpleAudioDecoder_SetServerSettings(session->audio.server, activeDecoder, &settings, false);

        NEXUS_SimpleAudioDecoder_GetServerSettings(session->audio.server, srcDecoder, &settings);
        settings.primary = NULL;
        (void)NEXUS_SimpleAudioDecoder_SetServerSettings(session->audio.server, srcDecoder, &settings, false);

        NEXUS_SimpleAudioDecoder_GetServerSettings(session->audio.server, destDecoder, &settings);
        settings.primary = NULL;
        (void)NEXUS_SimpleAudioDecoder_SetServerSettings(session->audio.server, destDecoder, &settings, false);
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
struct b_audio_resource *audio_decoder_create(struct b_session *session, enum b_audio_decoder_type type, b_audio_decoder_create_settings * create_settings) { BSTD_UNUSED(session);BSTD_UNUSED(type);BSTD_UNUSED(create_settings);return NULL; }
void audio_decoder_destroy(struct b_audio_resource *r) {BSTD_UNUSED(r); }
int audio_init(nxserver_t server) { BSTD_UNUSED(server);return 0; }
void release_audio_decoders(struct b_connect *connect) {BSTD_UNUSED(connect); }
int nxserverlib_p_swap_audio(struct b_connect *connect1, struct b_connect *connect2) { BSTD_UNUSED(connect1);BSTD_UNUSED(connect2);return 0; }
void audio_uninit(void) { }
#if NEXUS_HAS_HDMI_OUTPUT
int bserver_hdmi_edid_audio_config(struct b_session *session, const NEXUS_HdmiOutputStatus *pStatus) {BSTD_UNUSED(session);BSTD_UNUSED(pStatus);return 0;}
#endif
int bserver_set_audio_config(struct b_audio_resource *r, bool forceReconfig) {BSTD_UNUSED(r);return 0;}
int audio_get_stc_index(struct b_connect *connect) {BSTD_UNUSED(connect);return 0;}
int nxserverlib_p_audio_i2s0_shares_with_dac(struct b_session *session) {BSTD_UNUSED(session);return 0;}
bool b_dolby_ms_capable(const struct b_session *session) {BSTD_UNUSED(session);return false;}
void bserver_set_default_audio_settings(struct b_session *session) {BSTD_UNUSED(session);}
#endif /* NEXUS_HAS_AUDIO */
