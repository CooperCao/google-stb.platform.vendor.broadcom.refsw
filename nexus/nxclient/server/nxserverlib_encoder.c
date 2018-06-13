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
 *****************************************************************************/
#include "nxserverlib_impl.h"

BDBG_MODULE(nxserverlib_encoder);

#define BDBG_MSG_TRACE(X) /* BDBG_MSG(X) */

#if NEXUS_NUM_VIDEO_ENCODERS

static struct {
    bool open;
} g_encoder[NEXUS_NUM_VIDEO_ENCODERS];

struct encoder_resource {
    unsigned index;
    NEXUS_SimpleEncoderHandle handle;
    NEXUS_SimpleEncoderServerSettings settings;
    struct b_session *session;
    struct b_connect *connect;
};

static int get_transcode_display_index(nxserver_t server, unsigned encoder)
{
    if (encoder >= NEXUS_MAX_VIDEO_ENCODERS || !server->videoEncoder.cap.videoEncoder[encoder].supported) {
        return -1;
    }
    else {
        return server->videoEncoder.cap.videoEncoder[encoder].displayIndex;
    }
}

int get_transcode_encoder_index(nxserver_t server, unsigned display)
{
    unsigned i;
    for (i=0;i<NEXUS_NUM_VIDEO_ENCODERS;i++) {
        if (get_transcode_display_index(server, i) == (int)display) return i;
    }
    return -1;
}

/* has the encoder display already been used for a local display? if yes, pick another encoder. */
static bool is_local_display(nxserver_t server, unsigned display)
{
    unsigned i, j;
    for (i=0;i<NXCLIENT_MAX_SESSIONS;i++) {
        struct b_session *session = server->session[i];
        if (session && IS_SESSION_DISPLAY_ENABLED(server->settings.session[session->index])) {
            for (j=0;j<NXCLIENT_MAX_DISPLAYS;j++) {
                if (!session->display[j].display) break;
                if (j > 0 && server->settings.transcode == nxserver_transcode_sd) continue;
                if (session->display[j].global_index == display) {
                    return true;
                }
            }
        }
    }
    return false;
}

void video_encoder_get_num(nxserver_t server, unsigned *pTotalDisplays, unsigned *pUsedDisplays)
{
    unsigned i;
    *pTotalDisplays = *pUsedDisplays = 0;
    for (i=0;i<NEXUS_NUM_VIDEO_ENCODERS;i++) {
        if (get_transcode_display_index(server, i) != -1) {
            (*pTotalDisplays)++;
            if (!g_encoder[i].open) {
                (*pUsedDisplays)++;
            }
        }
    }
}

void video_encoder_get_status(const struct encoder_resource *r, struct video_encoder_status *pstatus)
{
    pstatus->encoderIndex = r->index;
    pstatus->display = r->settings.displayEncode.display;
    pstatus->displayIndex = get_transcode_display_index(r->session->server, r->index);
}

static bool valid_encoder_timebase(NEXUS_Timebase timebase)
{
    /* must be valid result from Open, and not timebase 0. */
    return timebase && timebase != NEXUS_Timebase_eInvalid;
}

static NEXUS_Error video_encoder_acquire_stc_channel(struct b_connect * connect, NEXUS_SimpleEncoderServerSettings *pSettings)
{
    struct b_stc_caps stcreq;
    NEXUS_StcChannelSettings settings;
    NEXUS_Error rc;

    if (valid_encoder_timebase(pSettings->timebase)) {
        BDBG_ERR(("leaking timebase %lu", pSettings->timebase));
    }
    pSettings->timebase = NEXUS_Timebase_Open(NEXUS_ANY_ID);
    if (!valid_encoder_timebase(pSettings->timebase)) {
        rc = BERR_TRACE(NEXUS_NOT_AVAILABLE);
        goto err_open_timebase;
    }
    pSettings->nonRealTime = nxserver_p_connect_is_nonrealtime_encode(connect);

    /* For real-time transcode, the encoder has its own StcChannel while video/audio decoders share.
    For non-real-time transcode, video and audio need separate StcChannels and the encoder can share with video or audio. */
    if (pSettings->nonRealTime) {
        pSettings->stcChannelTranscode = NULL;
        return 0;
    }

    BDBG_MSG_TRACE(("SEN%u acquiring STC", index));

    stc_index_request_init(connect, &stcreq);
    stcreq.encode = true; /* not a property of the connect, but from the caller context */
    /* RT mode encoder STC doesn't need video decoder capability! */
    if (!nxserver_p_connect_is_nonrealtime_encode(connect)) {
        stcreq.video = false;
    }
    NEXUS_StcChannel_GetDefaultSettings(0, &settings);
    /* encoders do not share stc indices (even amongst themselves) */
    settings.stcIndex = stc_index_acquire(connect, &stcreq);
    if (settings.stcIndex == -1) {
        rc = BERR_TRACE(NEXUS_NOT_AVAILABLE);
        goto err_stc_index_acquire;
    }
    BDBG_MSG_TRACE(("SEN%u acquired STC%u", index, settings.stcIndex));
    /* all STC's for transcode must use the same timebase. alloc in reverse order based on encoder index. */
    settings.timebase = pSettings->timebase;
    settings.mode = NEXUS_StcChannelMode_eAuto;
    settings.pcrBits = NEXUS_StcChannel_PcrBits_eFull42;/* ViCE2 requires 42-bit STC broadcast */
    pSettings->stcChannelTranscode = NEXUS_StcChannel_Open(NEXUS_ANY_ID, &settings);
    if (!pSettings->stcChannelTranscode) {
        rc = BERR_TRACE(NEXUS_NOT_AVAILABLE);
        goto err_stc_channel_open;
    }

    return 0;

err_stc_channel_open:
    stc_index_release(connect, settings.stcIndex);
err_stc_index_acquire:
    NEXUS_Timebase_Close(pSettings->timebase);
    pSettings->timebase = NEXUS_Timebase_eInvalid;
err_open_timebase:
    return rc;
}

static void video_encoder_release_stc_channel(struct b_connect * connect, unsigned index, NEXUS_SimpleEncoderServerSettings *pSettings)
{
    BSTD_UNUSED(index);

    if (pSettings->stcChannelTranscode)
    {
        NEXUS_StcChannelSettings settings;

        NEXUS_StcChannel_GetSettings(pSettings->stcChannelTranscode, &settings);
        BDBG_MSG_TRACE(("SEN%u releasing STC", index));
        stc_index_release(connect, settings.stcIndex);
        BDBG_MSG_TRACE(("SEN%u released STC%u", index, settings.stcIndex));
        NEXUS_StcChannel_Close(pSettings->stcChannelTranscode);
        pSettings->stcChannelTranscode = NULL;
    }
    if (valid_encoder_timebase(pSettings->timebase)) {
        NEXUS_Timebase_Close(pSettings->timebase);
        pSettings->timebase = NEXUS_Timebase_eInvalid;
    }
}

static void print_open_encoders(nxserver_t server)
{
    unsigned index, displayIndex;
    for (index=0; index < NEXUS_NUM_VIDEO_ENCODERS; index++) {
        unsigned rr;
        if (!server->settings.memConfigSettings.videoEncoder[index].used) continue;
        displayIndex = get_transcode_display_index(server, index);
        NEXUS_VideoFrameRate_GetRefreshRate(server->videoEncoder.cap.videoEncoder[index].memory.maxFrameRate, &rr);
        BDBG_WRN(("  %d: %ux%up%u display %d, used %c, graphics %c, local %c", index,
            server->videoEncoder.cap.videoEncoder[index].memory.maxWidth,
            server->videoEncoder.cap.videoEncoder[index].memory.maxHeight,
            rr/1000,
            displayIndex,
            g_encoder[index].open?'y':'n',
            server->display.cap.display[displayIndex].graphics.width?'y':'n',
            is_local_display(server, displayIndex)?'y':'n'));
    }
}

static NEXUS_DisplayHandle nxserver_p_open_encoder_display(unsigned index)
{
    NEXUS_DisplayHandle display;
    NEXUS_DisplaySettings displaySettings;

    NEXUS_Display_GetDefaultSettings(&displaySettings);
    displaySettings.format = NEXUS_VideoFormat_e480p;
#if NEXUS_NUM_DSP_VIDEO_ENCODERS && !NEXUS_DSP_ENCODER_ACCELERATOR_SUPPORT
#else
    displaySettings.timingGenerator = NEXUS_DisplayTimingGenerator_eEncoder;
    displaySettings.frameRateMaster = NULL; /* disable frame rate tracking for now */
#endif
    display = NEXUS_Display_Open(index, &displaySettings);
    return display;
}

static void nxserver_p_close_encoder_display(NEXUS_DisplayHandle display)
{
    NEXUS_Display_Close(display);
}

static unsigned video_encoder_cap(unsigned maxWidth, unsigned maxHeight, unsigned refreshRate)
{
    return (maxWidth+15)/16 * (maxHeight+15)/16 * refreshRate/1000;
}

static void nxserver_p_video_encoder_watchdog(void *context, int param)
{
    struct encoder_resource *r = context;
    BSTD_UNUSED(param);
    NEXUS_SimpleEncoder_Watchdog(r->session->encoder.server, r->handle);
}

struct encoder_resource *video_encoder_create(bool video_only, struct b_session *session, struct b_connect *connect)
{
    struct encoder_resource *r;
    NEXUS_VideoEncoderOpenSettings encoderOpenSettings;
    NEXUS_AudioMuxOutputCreateSettings muxCreateSettings;
    int displayIndex = -1;
    unsigned i;
    unsigned index;
    bool transcode = connect != NULL;
    unsigned r2;
    unsigned cap2;

    /* find available encoder */
    if (NEXUS_VideoFrameRate_GetRefreshRate(connect->settings.simpleEncoder[0].encoderCapabilities.maxFrameRate, &r2)) {
        r2 = 30000;
    }
    cap2 = video_encoder_cap(connect->settings.simpleEncoder[0].encoderCapabilities.maxWidth, connect->settings.simpleEncoder[0].encoderCapabilities.maxHeight, r2);
    for (index=0; index < NEXUS_NUM_VIDEO_ENCODERS; index++) {
        unsigned r1;
        bool pick = true;
        if (!session->server->settings.memConfigSettings.videoEncoder[index].used) continue;
        displayIndex = get_transcode_display_index(session->server, index);
        if (NEXUS_VideoFrameRate_GetRefreshRate(session->server->videoEncoder.cap.videoEncoder[index].memory.maxFrameRate, &r1)) {
            r1 = 30000;
        }

        if (g_encoder[index].open ||
            (displayIndex == -1) ||
            (!video_only && !session->server->display.cap.display[displayIndex].graphics.width) ||
            is_local_display(session->server, displayIndex)) pick = false;

        if (pick) {
            unsigned cap1 = video_encoder_cap(session->server->videoEncoder.cap.videoEncoder[index].memory.maxWidth, session->server->videoEncoder.cap.videoEncoder[index].memory.maxHeight, r1);
            if (cap2 > cap1) {
                pick = false;
            }
            else {
                if ((r1 == 25000 || r1 == 50000) && r2 > 50000) {
                    pick = false;
                }
            }
        }

        BDBG_MSG(("compare encoder%u: %ux%up%u with connect: %dx%dp%d --> %s", index,
            session->server->videoEncoder.cap.videoEncoder[index].memory.maxWidth,
            session->server->videoEncoder.cap.videoEncoder[index].memory.maxHeight,
            r1/1000,
            connect->settings.simpleEncoder[0].encoderCapabilities.maxWidth,
            connect->settings.simpleEncoder[0].encoderCapabilities.maxHeight,
            r2/1000,
            pick?"yes":"no"));

        if (pick) break;
    }
    if (index == NEXUS_NUM_VIDEO_ENCODERS) {
        /* because encoder systems are complex, give information that may explain the failure */
        BDBG_WRN(("unable to get %ux%up%u video encoder from pool",
            connect->settings.simpleEncoder[0].encoderCapabilities.maxWidth,
            connect->settings.simpleEncoder[0].encoderCapabilities.maxHeight,
            r2/1000));
        print_open_encoders(session->server);
        return NULL;
    }

    if (g_encoder[index].open) {
        return NULL;
    }

    r = BKNI_Malloc(sizeof(*r));
    if (!r) {
        BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
        return NULL;
    }
    BKNI_Memset(r, 0, sizeof(*r));
    r->index = index;
    r->session = session;
    BDBG_MSG(("video_encoder_create %p index=%d transcode=%d", (void*)r, r->index, transcode));

#if NEXUS_NUM_DSP_VIDEO_ENCODERS && !NEXUS_DSP_ENCODER_ACCELERATOR_SUPPORT
    BDBG_WRN(("DSP encoder %d using display %d", index, displayIndex));
#else
#if NEXUS_DSP_ENCODER_ACCELERATOR_SUPPORT
    BDBG_WRN(("DSP(VIP) encoder %d using display %d", index, displayIndex));
#else
    BDBG_WRN(("VCE encoder %d using display %d", index, displayIndex));
#endif
#endif

    nxserver_p_disable_local_display(session->server, displayIndex);

    r->settings.headless = (session->server->session[0]->display[0].display == NULL);
    if (transcode) {
        r->settings.transcodeDisplayIndex = displayIndex;
    }
    else {
        r->settings.displayEncode.display = nxserver_p_open_encoder_display(displayIndex);
        if (!r->settings.displayEncode.display) goto error;
    }

    if (connect->settings.simpleAudioDecoder.id) {
        NEXUS_AudioMuxOutput_GetDefaultCreateSettings(&muxCreateSettings);
        muxCreateSettings.data.heap = session->server->settings.client.heap[NXCLIENT_FULL_HEAP];
        muxCreateSettings.index.heap = session->server->settings.client.heap[NXCLIENT_FULL_HEAP];
        r->settings.audioMuxOutput = NEXUS_AudioMuxOutput_Create(&muxCreateSettings);
        if (!r->settings.audioMuxOutput) {
            BERR_TRACE(NEXUS_NOT_AVAILABLE);
            goto error;
        }
    }

    NEXUS_VideoEncoder_GetDefaultOpenSettings(&encoderOpenSettings);
    encoderOpenSettings.data.heap = session->server->settings.client.heap[NXCLIENT_FULL_HEAP];
    encoderOpenSettings.index.heap = session->server->settings.client.heap[NXCLIENT_FULL_HEAP];
    encoderOpenSettings.memoryConfig.maxWidth = session->server->settings.memConfigSettings.videoEncoder[index].maxWidth;
    encoderOpenSettings.memoryConfig.maxHeight = session->server->settings.memConfigSettings.videoEncoder[index].maxHeight;
    encoderOpenSettings.memoryConfig.interlaced = session->server->settings.memConfigSettings.videoEncoder[index].interlaced;
    if (connect->settings.simpleEncoder[0].encoderCapabilities.lowDelay) {
        encoderOpenSettings.type = NEXUS_VideoEncoderType_eSingle;
    }
    encoderOpenSettings.watchdogCallback.callback = nxserver_p_video_encoder_watchdog;
    encoderOpenSettings.watchdogCallback.context = r;
    r->settings.videoEncoder = NEXUS_VideoEncoder_Open(index, &encoderOpenSettings);
    if (!r->settings.videoEncoder) {
        BERR_TRACE(NEXUS_NOT_AVAILABLE);
        goto error;
    }
    for (i=0;i<NEXUS_SIMPLE_ENCODER_NUM_PLAYPUMPS;i++) {
        NEXUS_PlaypumpOpenSettings playpumpConfig;
        NEXUS_Playpump_GetDefaultOpenSettings(&playpumpConfig);
        playpumpConfig.fifoSize = 16384; /* reduce FIFO size allocated for playpump */
        playpumpConfig.numDescriptors = 256; /* set number of descriptors */
        playpumpConfig.streamMuxCompatible = true;
        r->settings.playpump[i] = NEXUS_Playpump_Open(NEXUS_ANY_ID, &playpumpConfig);
        if (!r->settings.playpump[i]) goto error;
    }

    g_encoder[index].open = true;
    return r;

error:
    video_encoder_destroy(r);
    return NULL;
}

void video_encoder_destroy(struct encoder_resource *r)
{
    unsigned i;

    BDBG_MSG(("video_encoder_destroy %p %d", (void*)r, r->index));
    g_encoder[r->index].open = false;
    if (r->settings.videoEncoder) {
        NEXUS_VideoEncoder_Close(r->settings.videoEncoder);
    }
    if (r->settings.displayEncode.display) {
        nxserver_p_close_encoder_display(r->settings.displayEncode.display);
    }
    nxserver_p_reenable_local_display(r->session->server);
    if (r->settings.audioMuxOutput) {
        NEXUS_AudioMuxOutput_Destroy(r->settings.audioMuxOutput);
    }
    for (i=0;i<NEXUS_SIMPLE_ENCODER_NUM_PLAYPUMPS;i++) {
        if (r->settings.playpump[i]) {
            NEXUS_Playpump_Close(r->settings.playpump[i]);
        }
    }
    if (r->settings.stcChannelTranscode) {
        BDBG_WRN(("STC channel was still valid during destroy; should have been released during encoder release"));
        NEXUS_StcChannel_Close(r->settings.stcChannelTranscode);
    }
    BKNI_Free(r);
}

static unsigned get_encoder_req_id(struct b_req *req, unsigned i) { return req->handles.simpleEncoder[i].id; }

int acquire_video_encoders(struct b_connect *connect)
{
    struct b_req *req = connect->req[b_resource_simple_encoder];
    unsigned i;
    struct b_session *session = connect->client->session;

    for (i=0;i<NXCLIENT_MAX_IDS;i++) {
        unsigned index;
        int rc;
        struct encoder_resource *r;

        if (!connect->settings.simpleEncoder[i].id) break;
        if (i > 0) {
            /* requiring only one encoder per connect for now */
            return BERR_TRACE(NEXUS_NOT_SUPPORTED);
        }

        if (get_req_index(req, get_encoder_req_id, connect->settings.simpleEncoder[i].id, &index)) {
            return BERR_TRACE(NEXUS_INVALID_PARAMETER);
        }
        if (req->handles.simpleEncoder[index].r) {
            BDBG_ERR(("already connected to %p", (void*)req->handles.simpleEncoder[index].r->connect));
            return BERR_TRACE(NEXUS_NOT_AVAILABLE);
        }

        if (!connect->settings.simpleEncoder[i].display) {
            struct b_req *req;
            r = video_encoder_create(true, session, connect);
            if (!r) {
                return BERR_TRACE(NEXUS_NOT_AVAILABLE);
            }

            req = connect->req[b_resource_simple_audio_decoder];
            if (req && req->handles.simpleAudioDecoder.r) {
                audio_get_encode_resources(req->handles.simpleAudioDecoder.r, &r->settings.mixer, NULL, NULL);
            }
        }
        else {
            r = session->encoder.encoder;
            if (!r || r->connect) {
                BDBG_ERR(("nxserver was not started with '-session%d encode' option", session->index));
                return BERR_TRACE(NEXUS_NOT_AVAILABLE);
            }

            audio_get_encode_resources(session->main_audio, &r->settings.mixer, &r->settings.displayEncode.masterAudio, &r->settings.displayEncode.slaveAudio);
        }

        rc = video_encoder_acquire_stc_channel(connect, &r->settings);
        if (rc) return BERR_TRACE(rc);

        rc = NEXUS_SimpleEncoder_SetServerSettings(session->encoder.server, req->handles.simpleEncoder[index].handle, &r->settings);
        if (rc) return BERR_TRACE(rc);

        req->handles.simpleEncoder[index].r = r;
        r->connect = connect;
        /* TODO: this further solidifies the requirement of only 1 encoder set connect. we should remove code that implies N. */
        r->handle = req->handles.simpleEncoder[index].handle;
    }
    return 0;
}

void release_video_encoders(struct b_connect *connect)
{
    struct b_req *req = connect->req[b_resource_simple_encoder];
    unsigned i;
    for (i=0;i<NXCLIENT_MAX_IDS;i++) {
        NEXUS_SimpleEncoderServerSettings settings;
        unsigned index;
        struct encoder_resource *r;
        struct b_session *session = connect->client->session;

        if (!connect->settings.simpleEncoder[i].id) break;

        if (get_req_index(req, get_encoder_req_id, connect->settings.simpleEncoder[i].id, &index)) {
            BERR_TRACE(NEXUS_INVALID_PARAMETER);
            continue;
        }

        r = req->handles.simpleEncoder[index].r;
        if (!r) continue;

        NEXUS_SimpleEncoder_GetServerSettings(session->encoder.server, req->handles.simpleEncoder[index].handle, &settings);
        settings.videoEncoder = NULL;
        settings.stcChannelTranscode = NULL;
        settings.mixer = NULL;
        settings.audioMuxOutput = NULL;
        settings.displayEncode.display = NULL;
        settings.displayEncode.masterAudio = NULL;
        settings.displayEncode.slaveAudio = NULL;
        (void)NEXUS_SimpleEncoder_SetServerSettings(session->encoder.server, req->handles.simpleEncoder[index].handle, &settings);
        /* now we can release it, since it shouldn't be in use anymore */
        video_encoder_release_stc_channel(connect, index, &r->settings);

        if (!connect->settings.simpleEncoder[i].display) {
            video_encoder_destroy(r);
        }
        else if (!session->server->settings.session[session->index].output.encode) {
            video_encoder_destroy(r);
            session->encoder.encoder = NULL;
        }
        else {
            r->connect = NULL;
            r->handle = NULL;
        }
        req->handles.simpleEncoder[index].r = NULL;
    }
}

bool nxserver_p_connect_is_nonrealtime_encode(struct b_connect *connect)
{
#if NEXUS_NUM_DSP_VIDEO_ENCODERS
    BSTD_UNUSED(connect);
    return false;
#else
    return connect && connect->settings.simpleEncoder[0].id && connect->settings.simpleEncoder[0].nonRealTime;
#endif
}
#else
int acquire_video_encoders(struct b_connect *connect)
{
    BSTD_UNUSED(connect);
    if (!connect->settings.simpleEncoder[0].id) {
        return 0;
    }
    return NEXUS_NOT_SUPPORTED;
}
void release_video_encoders(struct b_connect *connect)
{
    BSTD_UNUSED(connect);
}
bool nxserver_p_connect_is_nonrealtime_encode(struct b_connect *connect)
{
    BSTD_UNUSED(connect);
    return false;
}
#endif
