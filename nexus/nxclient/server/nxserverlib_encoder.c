/******************************************************************************
 *    (c)2011-2014 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
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

static NEXUS_Error video_encoder_acquire_stc_channel(struct b_connect * connect, unsigned index, NEXUS_SimpleEncoderServerSettings *pSettings)
{
    struct b_stc_caps stcreq;
    NEXUS_StcChannelSettings settings;
    NEXUS_TransportCapabilities cap;

    NEXUS_GetTransportCapabilities(&cap);
    if (index >= cap.numTimebases) {
        return BERR_TRACE(NEXUS_NOT_AVAILABLE);
    }
    pSettings->timebase = NEXUS_Timebase_e0 + cap.numTimebases - 1  - index;
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
        return BERR_TRACE(NEXUS_NOT_AVAILABLE);
    }
    BDBG_MSG_TRACE(("SEN%u acquired STC%u", index, settings.stcIndex));
    /* all STC's for transcode must use the same timebase. alloc in reverse order based on encoder index. */
    settings.timebase = pSettings->timebase;
    settings.mode = NEXUS_StcChannelMode_eAuto;
    settings.pcrBits = NEXUS_StcChannel_PcrBits_eFull42;/* ViCE2 requires 42-bit STC broadcast */
    pSettings->stcChannelTranscode = NEXUS_StcChannel_Open(NEXUS_ANY_ID, &settings);
    if (!pSettings->stcChannelTranscode) {
        stc_index_release(connect, settings.stcIndex);
        return BERR_TRACE(NEXUS_NOT_AVAILABLE);
    }

    return 0;
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
}

static void print_open_encoders(nxserver_t server)
{
    unsigned index, displayIndex;
    for (index=0; index < NEXUS_NUM_VIDEO_ENCODERS; index++) {
        if (!server->settings.memConfigSettings.videoEncoder[index].used) continue;
        displayIndex = get_transcode_display_index(server, index);
        BDBG_WRN(("  %d: display %d, used %c, graphics %c, local %c", index,
            displayIndex,
            g_encoder[index].open?'y':'n',
            server->display.cap.display[displayIndex].graphics.width?'y':'n',
            is_local_display(server, displayIndex)?'y':'n'));
    }
}

static struct {
    NEXUS_DisplayHandle display;
    unsigned index;
} g_cachedEncoderDisplay;
NEXUS_DisplayHandle nxserver_p_open_encoder_display(unsigned index, bool cache)
{
    NEXUS_DisplayHandle display;
    NEXUS_DisplaySettings displaySettings;

    /* cache display for video-as-graphics */
    if (g_cachedEncoderDisplay.display && g_cachedEncoderDisplay.index == index) {
        return g_cachedEncoderDisplay.display;
    }

    NEXUS_Display_GetDefaultSettings(&displaySettings);
    displaySettings.format = NEXUS_VideoFormat_e480p;
#if NEXUS_NUM_DSP_VIDEO_ENCODERS && !NEXUS_DSP_ENCODER_ACCELERATOR_SUPPORT
#else
    displaySettings.timingGenerator = NEXUS_DisplayTimingGenerator_eEncoder;
    displaySettings.frameRateMaster = NULL; /* disable frame rate tracking for now */
#endif
    display = NEXUS_Display_Open(index, &displaySettings);
    if (cache) {
        g_cachedEncoderDisplay.display = display;
        g_cachedEncoderDisplay.index = index;
    }
    return display;
}

static void nxserver_p_close_encoder_display(NEXUS_DisplayHandle display)
{
    /* never close display0 because it's needed globally to drive video-as-graphics */
    if (display == g_cachedEncoderDisplay.display) return;
    NEXUS_Display_Close(display);
}

struct encoder_resource *video_encoder_create(bool video_only, struct b_session *session, struct b_connect *connect)
{
    struct encoder_resource *r;
    NEXUS_VideoEncoderOpenSettings encoderOpenSettings;
    NEXUS_AudioMuxOutputCreateSettings muxCreateSettings;
    NEXUS_StreamMuxCreateSettings streamMuxCreateSettings;
    NEXUS_StreamMuxConfiguration streamMuxConfigSettings;
    int displayIndex = -1;
    unsigned i;
    unsigned index;
    bool transcode = connect != NULL;

    /* find available encoder */
    for (index=0; index < NEXUS_NUM_VIDEO_ENCODERS; index++) {
        if (!session->server->settings.memConfigSettings.videoEncoder[index].used) continue;
        displayIndex = get_transcode_display_index(session->server, index);
        if (g_encoder[index].open ||
            (displayIndex == -1) ||
            (!video_only && !session->server->display.cap.display[displayIndex].graphics.width) ||
            is_local_display(session->server, displayIndex)) continue;
        /* else, use this one */
        break;
    }
    if (index == NEXUS_NUM_VIDEO_ENCODERS) {
        /* because encoder systems are complex, give information that may explain the failure */
        BDBG_WRN(("unable to get video encoder from pool"));
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
    BDBG_MSG(("video_encoder_create %p index=%d transcode=%d", r, r->index, transcode));

#if NEXUS_NUM_DSP_VIDEO_ENCODERS && !NEXUS_DSP_ENCODER_ACCELERATOR_SUPPORT
    BDBG_WRN(("DSP encoder %d using display %d", index, displayIndex));
#else
#if NEXUS_DSP_ENCODER_ACCELERATOR_SUPPORT
    BDBG_WRN(("DSP(VIP) encoder %d using display %d", index, displayIndex));
#else
    BDBG_WRN(("VCE encoder %d using display %d", index, displayIndex));
#endif
#endif
    r->settings.transcodeDisplay = nxserver_p_open_encoder_display(displayIndex, false);
    /* window is opened internally */
    if (!r->settings.transcodeDisplay) goto error;

    if (transcode) {
        r->settings.displayEncode.display = NULL;
    }
    else {
        r->settings.displayEncode.display = r->settings.transcodeDisplay;
    }

    if (nxserver_p_connect_is_nonrealtime_encode(connect)) {
        NEXUS_DisplayStgSettings stgSettings;
        NEXUS_Display_GetStgSettings(r->settings.transcodeDisplay, &stgSettings);
        stgSettings.enabled = true;
        stgSettings.nonRealTime = true;
        NEXUS_Display_SetStgSettings(r->settings.transcodeDisplay, &stgSettings);
    }

    NEXUS_AudioMuxOutput_GetDefaultCreateSettings(&muxCreateSettings);
    muxCreateSettings.data.heap = session->server->settings.client.heap[NXCLIENT_FULL_HEAP];
    muxCreateSettings.index.heap = session->server->settings.client.heap[NXCLIENT_FULL_HEAP];
    r->settings.audioMuxOutput = NEXUS_AudioMuxOutput_Create(&muxCreateSettings);
    if (!r->settings.audioMuxOutput) {
        BERR_TRACE(NEXUS_NOT_AVAILABLE);
        goto error;
    }

    NEXUS_VideoEncoder_GetDefaultOpenSettings(&encoderOpenSettings);
    encoderOpenSettings.data.heap = session->server->settings.client.heap[NXCLIENT_FULL_HEAP];
    encoderOpenSettings.index.heap = session->server->settings.client.heap[NXCLIENT_FULL_HEAP];
    encoderOpenSettings.memoryConfig.maxWidth = session->server->settings.memConfigSettings.videoEncoder[index].maxWidth;
    encoderOpenSettings.memoryConfig.maxHeight = session->server->settings.memConfigSettings.videoEncoder[index].maxHeight;
    encoderOpenSettings.memoryConfig.interlaced = session->server->settings.memConfigSettings.videoEncoder[index].interlaced;
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

    NEXUS_StreamMux_GetDefaultCreateSettings(&streamMuxCreateSettings);
    /* Override default stream mux mem config:
       Allow minimum stream mux memory config with one ts user data pid.
       TODO: expose to nxclient API */
    NEXUS_StreamMux_GetDefaultConfiguration(&streamMuxConfigSettings);
    streamMuxConfigSettings.userDataPids = 1;
    streamMuxConfigSettings.nonRealTime  = nxserver_p_connect_is_nonrealtime_encode(connect);
    NEXUS_StreamMux_GetMemoryConfiguration(&streamMuxConfigSettings, &streamMuxCreateSettings.memoryConfiguration);
#if 0
    /* because finished is set at create-time only, SimpleEncoder will need an external "finished" function. */
    streamMuxCreateSettings.finished.callback = transcoderFinishCallback;
    streamMuxCreateSettings.finished.context = pContext->finishEvent;
#endif
    r->settings.streamMux = NEXUS_StreamMux_Create(&streamMuxCreateSettings);
    if (!r->settings.streamMux) {
        BERR_TRACE(NEXUS_NOT_AVAILABLE);
        goto error;
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

    BDBG_MSG(("video_encoder_destroy %p %d", r, r->index));
    g_encoder[r->index].open = false;
    if (r->settings.videoEncoder) {
        NEXUS_VideoEncoder_Close(r->settings.videoEncoder);
    }
    if (r->settings.transcodeDisplay) {
        nxserver_p_close_encoder_display(r->settings.transcodeDisplay);
    }
    if (r->settings.audioMuxOutput) {
        NEXUS_AudioMuxOutput_Destroy(r->settings.audioMuxOutput);
    }
    for (i=0;i<NEXUS_SIMPLE_ENCODER_NUM_PLAYPUMPS;i++) {
        if (r->settings.playpump[i]) {
            NEXUS_Playpump_Close(r->settings.playpump[i]);
        }
    }
    if (r->settings.streamMux) {
        NEXUS_StreamMux_Destroy(r->settings.streamMux);
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

        if (!connect->settings.simpleEncoder[i].display) {
            struct b_req *req;
            r = video_encoder_create(true, connect->client->session, connect);
            if (!r) {
                return BERR_TRACE(NEXUS_NOT_AVAILABLE);
            }

            req = connect->req[b_resource_simple_audio_decoder];
            if (req && req->handles.simpleAudioDecoder.r) {
                audio_get_encode_resources(req->handles.simpleAudioDecoder.r, &r->settings.mixer, NULL, NULL);
            }
        }
        else {
            struct b_session *session = connect->client->session;

            r = session->encoder;
            if (!r || r->connect) {
                BDBG_ERR(("nxserver was not started with '-session%d encode' option", session->index));
                return BERR_TRACE(NEXUS_NOT_AVAILABLE);
            }

            audio_get_encode_resources(session->main_audio, &r->settings.mixer, &r->settings.displayEncode.masterAudio, &r->settings.displayEncode.slaveAudio);
        }

        rc = video_encoder_acquire_stc_channel(connect, r->index, &r->settings);
        if (rc) return BERR_TRACE(rc);

        rc = NEXUS_SimpleEncoder_SetServerSettings(req->handles.simpleEncoder[index].handle, &r->settings);
        if (rc) return BERR_TRACE(rc);

        req->handles.simpleEncoder[index].r = r;
        r->connect = connect;
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

        NEXUS_SimpleEncoder_GetServerSettings(req->handles.simpleEncoder[index].handle, &settings);
        settings.videoEncoder = NULL;
        settings.stcChannelTranscode = NULL;
        settings.mixer = NULL;
        settings.audioMuxOutput = NULL;
        settings.displayEncode.display = NULL;
        settings.displayEncode.masterAudio = NULL;
        settings.displayEncode.slaveAudio = NULL;
        (void)NEXUS_SimpleEncoder_SetServerSettings(req->handles.simpleEncoder[index].handle, &settings);
        /* now we can release it, since it shouldn't be in use anymore */
        video_encoder_release_stc_channel(connect, index, &r->settings);

        if (!connect->settings.simpleEncoder[i].display) {
            video_encoder_destroy(r);
        }
        else if (!session->server->settings.session[session->index].output.encode) {
            video_encoder_destroy(r);
            session->encoder = NULL;
        }
        else {
            r->connect = NULL;
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
