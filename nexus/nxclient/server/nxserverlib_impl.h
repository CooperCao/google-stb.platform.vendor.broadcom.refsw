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
 *
 * Module Description:
 *
 *****************************************************************************/
#ifndef NXSERVERLIB_IMPL_H__
#define NXSERVERLIB_IMPL_H__

#include "nxserverlib.h"
#include "nexus_types.h"
#include "nexus_platform.h"
#include "nexus_platform_server.h"
#include "nexus_display.h"
#if NEXUS_HAS_AUDIO
#include "nexus_audio_decoder.h"
#include "nexus_audio_output.h"
#include "nexus_audio_playback.h"
#include "nexus_audio_mixer.h"
#include "nexus_audio_encoder.h"
#include "nexus_spdif_output.h"
#include "nexus_audio_capture.h"
#include "nexus_audio_crc.h"
#endif
#if NEXUS_HAS_HDMI_OUTPUT
#include "nexus_hdmi_output.h"
#include "nexus_hdmi_output_hdcp.h"
#else
typedef void *NEXUS_HdmiOutputHdcpVersion;
#endif
#if NEXUS_HAS_SIMPLE_DECODER
#include "nexus_simple_video_decoder_server.h"
#include "nexus_simple_audio_decoder_server.h"
#include "nexus_simple_encoder_server.h"
#endif
#if NEXUS_HAS_TRANSPORT
#include "nexus_stc_channel.h"
#endif
#include "nexus_surface_compositor.h"
#include "nexus_surface_cursor.h"
#include "nexus_core_utils.h"
#include "namevalue.h"
#if NEXUS_HAS_IR_INPUT
#include "nexus_ir_input.h"
#endif
#if NEXUS_HAS_KEYPAD
#include "nexus_keypad.h"
#endif
#if NEXUS_HAS_INPUT_ROUTER
#include "nexus_input_router.h"
#else
typedef unsigned NEXUS_InputRouterCode;
#endif
#include "nexus_memory.h"
#include "nexus_types.h"
#include "blst_list.h"
#include "blst_queue.h"
#include "nxserverlib_evdev.h"
#include "nxclient.h"

#include <stdlib.h>
#include <string.h>

/* make nxserver more resilient to compiled-out modules */
#ifndef NEXUS_HAS_STREAM_MUX
#undef NEXUS_NUM_VIDEO_ENCODERS
#endif
#ifndef NEXUS_HAS_RFM
#undef NEXUS_NUM_RFM_OUTPUTS
#endif
#ifndef NEXUS_HAS_HDMI_OUTPUT
#undef NEXUS_NUM_HDMI_OUTPUTS
#endif

enum b_resource {
    b_resource_client,
    b_resource_surface_client,
    b_resource_simple_video_decoder,
    b_resource_simple_audio_decoder,
    b_resource_simple_audio_playback,
    b_resource_input_client,
    b_resource_simple_encoder,
    b_resource_audio_capture,
    b_resource_audio_crc,
    b_resource_register_standby, /* not a req, just an id */
    b_resource_max
};

struct b_audio_config
{
    struct {
        NxClient_AudioOutputMode audioCodecOutput[NEXUS_AudioCodec_eMax];
    } hdmi, spdif;
    bool hdmiAc3Plus; /* use true AC3+ -> AC3+ passthrough because HDMI supports it */
};

struct b_connect;
struct b_audio_playback_resource;
struct b_audio_resource;

struct b_client_handles
{
    struct {
        unsigned id;
        NEXUS_SimpleAudioDecoderHandle handle;
        struct b_audio_resource *r;
    } simpleAudioDecoder;
    struct {
        unsigned id;
        NEXUS_SimpleAudioPlaybackHandle handle;
        struct b_audio_playback_resource *r;
    } simpleAudioPlayback[NXCLIENT_MAX_IDS];
    struct {
        unsigned id;
        NEXUS_SimpleVideoDecoderHandle handle;
        struct video_decoder_resource *r;
    } simpleVideoDecoder[NXCLIENT_MAX_IDS];
    struct {
        unsigned id;
        NEXUS_SurfaceClientHandle handle;
    } surfaceClient[NXCLIENT_MAX_IDS];
    struct {
        unsigned id;
        NEXUS_InputClientHandle handle;
    } inputClient[NXCLIENT_MAX_IDS];
    struct {
        unsigned id;
        NEXUS_SimpleEncoderHandle handle;
        struct encoder_resource *r;
    } simpleEncoder[NXCLIENT_MAX_IDS];
#if NEXUS_HAS_AUDIO
    struct {
        unsigned id; /* server id, not client id */
        NEXUS_AudioCaptureHandle handle;
    } audioCapture;
    struct {
        unsigned id; /* server id, not client id */
        NEXUS_AudioCrcHandle handle;
    } audioCrc;
#endif
};

/**
b_req is one resource request
a client can have multiple
**/
BDBG_OBJECT_ID_DECLARE(b_req);
struct b_req {
    BDBG_OBJECT(b_req)
    BLST_D_ENTRY(b_req) link;
    nxclient_t client;
    NxClient_AllocSettings settings;
    NxClient_AllocResults results;
    struct b_client_handles handles;
};

/**
b_connect is a connection of backend resources to a single b_req
the pairing is determined based on ID's for the first connection
**/
BDBG_OBJECT_ID_DECLARE(b_connect);
struct b_connect {
    BDBG_OBJECT(b_connect)
    BLST_D_ENTRY(b_connect) link;
    struct b_req *req[b_resource_max]; /* support separate reqs per b_resource type */
    nxclient_t client;
    NxClient_ConnectSettings settings;
    unsigned windowIndex;
};

#if NEXUS_HAS_HDMI_OUTPUT
enum b_hdmi_drm_source {
    b_hdmi_drm_source_input,
    b_hdmi_drm_source_user,
    b_hdmi_drm_source_max
};

struct b_hdmi_drm_selector
{
    enum b_hdmi_drm_source eotf;
    struct
    {
        enum b_hdmi_drm_source max;
        enum b_hdmi_drm_source maxFrameAverage;
    } cll;
    struct
    {
        struct
        {
            enum b_hdmi_drm_source red;
            enum b_hdmi_drm_source green;
            enum b_hdmi_drm_source blue;
        } primaries;
        enum b_hdmi_drm_source whitePoint;
        struct
        {
            enum b_hdmi_drm_source max;
            enum b_hdmi_drm_source min;
        } luminance;
    } mdcv;
};
#endif

/**
A session is either local (with a display) or streaming (with an encoder)
**/
struct b_session {
    nxserver_t server;
    BLST_D_HEAD(b_session_client_list, b_client) clients; /* uses session_link */
    unsigned index;
    NxClient_CallbackStatus callbackStatus;
    NEXUS_SurfaceCompositorHandle surfaceCompositor;
    BKNI_EventHandle inactiveEvent;

    struct {
        NEXUS_SurfaceHandle surface;
        NEXUS_SurfaceCursorHandle cursor;
        int x,y; /* current coordinates */
    } cursor;

    unsigned numWindows;
    struct {
        struct b_connect *connect;
        struct {
            bool deinterlaced;
        } capabilities;
    } window[NEXUS_NUM_VIDEO_WINDOWS];

    struct {
        NEXUS_DisplayHandle display; /* created as init_session */
        unsigned global_index;
        bool created_by_encoder;
        NEXUS_VideoWindowHandle window[NEXUS_NUM_VIDEO_WINDOWS][NXCLIENT_MAX_IDS]; /* created on demand */
        NEXUS_VideoWindowHandle parentWindow[NEXUS_NUM_VIDEO_WINDOWS];
        NEXUS_VideoFormatInfo formatInfo;
        nxclient_t crc_client;
    } display[NXCLIENT_MAX_DISPLAYS];
    NxClient_PictureQualitySettings pictureQualitySettings;
#if NEXUS_HAS_HDMI_OUTPUT
    NEXUS_HdmiOutputHandle hdmiOutput;
    struct
    {
        struct
        {
            struct b_hdmi_drm_selector selector;
            bool inputValid;
            NEXUS_HdmiDynamicRangeMasteringInfoFrame input;
        } drm;
    } hdmi;
#endif
    struct {
        NxClient_HdcpLevel level;
        NxClient_HdcpVersion prevSelect;
        NxClient_HdcpVersion currSelect;
        enum nxserver_hdcp_state {
            nxserver_hdcp_not_pending,                   /* no hdcp authentication in progress */
            nxserver_hdcp_begin,                         /* start hdcp auth disable */
            nxserver_hdcp_pending_status_disable,        /* wait for hdcp auth disable success */
            nxserver_hdcp_pending_status_start,          /* wait for hdcp auth start success, to read authentication status */
            nxserver_hdcp_pending_disable,               /* wait for hdcp auth disable success */
            nxserver_hdcp_pending_start_retry,           /* wait for hdcp auth disable success, retrying */
            nxserver_hdcp_pending_start,                 /* wait for hdcp auth disable success */
            nxserver_hdcp_success,                       /* hdcp authentication completed */
            nxserver_hdcp_max
        } version_state;
        NEXUS_HdmiOutputHdcpVersion downstream_version;
        bool mute;
    } hdcp;
    struct {
        struct {
            void *buffer;
            unsigned size;
        } hdcp1x, hdcp2x;
    } hdcpKeys;
    nxclient_t hdmiOutput_crc_client;

    /* streaming */
    struct encoder_resource *encoder;

    struct b_audio_resource *main_audio;
    NxClient_AudioProcessingSettings audioProcessingSettings;
    NxClient_AudioSettings audioSettings;

    struct {
        NxClient_DisplaySettings displaySettings;
    } nxclient;

#if NEXUS_HAS_INPUT_ROUTER
    struct {
        NEXUS_IrInputHandle irInput;
        NEXUS_InputRouterHandle router;
#if NEXUS_HAS_KEYPAD
        NEXUS_KeypadHandle keypad;
        NEXUS_KeypadSettings keypadSettings;
#endif
        nxserver_evdev_t evdevInput;
    } input;
#endif
};

struct b_stc_caps {
    bool audio;
    bool video;
    bool encode;
};

struct b_stc {
    BLST_D_ENTRY(b_stc) link;
    unsigned index;
    int refcnt;
    struct b_stc_caps caps;
    unsigned score;
};

enum b_special_mode {
    b_special_mode_none,
    b_special_mode_linked_decoder,
    b_special_mode_linked_decoder2,
    b_special_mode_max
};

enum b_standby_state {
    b_standby_state_none,
    b_standby_state_pending,
    b_standby_state_applied,
    b_standby_state_exit
};

struct b_server {
    BLST_D_HEAD(b_client_list, b_client) clients; /* uses link */
    struct nxserver_settings settings;

    NEXUS_PlatformSettings platformSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_PlatformStatus platformStatus;
    struct b_session *session[NXCLIENT_MAX_SESSIONS];
    unsigned global_display_index;

    unsigned nextId[b_resource_max];
    struct {
        NEXUS_VideoDecoderCapabilities cap;
        enum b_special_mode special_mode;
    } videoDecoder;
    struct {
        NEXUS_DisplayCapabilities cap;
        bool driveVideoAsGraphics;
    } display;
#if NEXUS_NUM_VIDEO_ENCODERS
    struct {
        NEXUS_VideoEncoderCapabilities cap;
    } videoEncoder;
#endif
    struct b_audio_config audio_config;

    struct {
        NxClient_StandbySettings standbySettings;
        enum b_standby_state state;
        NEXUS_ThreadHandle thread_id;
        void *pm_ctx;
    } standby;
    struct {
        BLST_D_HEAD(b_stc_list, b_stc) stcs;
    } transport;
    struct {
        bool allow_decode;
    } externalApp;
};

struct b_client_standby_ack
{
    BLST_S_ENTRY(b_client_standby_ack) link;
    unsigned id;
    bool waiting;
};

BDBG_OBJECT_ID_DECLARE(b_client);
struct b_client {
    BDBG_OBJECT(b_client)
    BLST_D_ENTRY(b_client) link;
    BLST_D_ENTRY(b_client) session_link;
    NEXUS_ClientHandle nexusClient;
    NEXUS_ClientSettings clientSettings;
    nxserver_t server;
    unsigned pid;
    bool ipc; /* created over ipc, not local */
    struct b_session *session;
    NxClient_JoinSettings joinSettings;
    BLST_D_HEAD(b_req_list, b_req) requests;
    BLST_D_HEAD(b_connect_list, b_connect) connects;
    NxClient_HdcpLevel hdcp;
    bool zombie;
    struct {
        BLST_S_HEAD(b_client_standby_acklist, b_client_standby_ack) acks;
    } standby;
};

/************
nxserverlib_encoder.c API
************/
struct encoder_resource;
#if NEXUS_NUM_DSP_VIDEO_ENCODERS && !NEXUS_DSP_ENCODER_ACCELERATOR_SUPPORT
#undef NEXUS_NUM_VIDEO_ENCODERS
#define NEXUS_NUM_VIDEO_ENCODERS NEXUS_NUM_DSP_VIDEO_ENCODERS
#endif
#if NEXUS_NUM_VIDEO_ENCODERS
struct encoder_resource *video_encoder_create(bool video_only, struct b_session *session, struct b_connect *connect);
void video_encoder_destroy(struct encoder_resource *encoder);
struct video_encoder_status
{
    unsigned encoderIndex;
    NEXUS_DisplayHandle display;
    unsigned displayIndex;
};
void video_encoder_get_num(nxserver_t server, unsigned *pTotalEncoderDisplays, unsigned *pUsedEncoderDisplays);
void video_encoder_get_status(const struct encoder_resource *r, struct video_encoder_status *pstatus);
int get_transcode_encoder_index(nxserver_t server, unsigned display);
#endif
int acquire_video_encoders(struct b_connect *connect);
void release_video_encoders(struct b_connect *connect);
bool nxserver_p_connect_is_nonrealtime_encode(struct b_connect *connect);
NEXUS_DisplayHandle nxserver_p_open_encoder_display(unsigned index, bool cache);

/************
nxserverlib_video.c API
************/
struct video_decoder_resource;
int acquire_video_decoders(struct b_connect *connect, bool grab);
void release_video_decoders(struct b_connect *connect);
bool lacks_video(struct b_connect *connect);
void uninit_session_video(struct b_session *session);
void nxserverlib_video_disconnect_sd_display(struct b_session *session);
int video_init(nxserver_t server);
void video_uninit(void);
int video_get_stc_index(struct b_connect *connect);
int nxserverlib_p_swap_video_windows(struct b_connect *connect1, struct b_connect *connect2);
#if NEXUS_HAS_VIDEO_DECODER
NEXUS_VideoDecoderHandle nxserver_p_get_video_decoder(struct b_connect *connect);
#endif

/************
nxserverlib_audio.c API
************/
int bserver_set_audio_config(struct b_audio_resource *ar);
void bserver_acquire_audio_mixers(struct b_audio_resource *r, bool start);
#if NEXUS_HAS_HDMI_OUTPUT
int bserver_hdmi_edid_audio_config(struct b_session *session, const NEXUS_HdmiOutputStatus *pStatus);
#endif
enum b_audio_decoder_type
{
    b_audio_decoder_type_regular,
    b_audio_decoder_type_background,
    b_audio_decoder_type_background_nrt
};
struct b_audio_resource *audio_decoder_create(struct b_session *session, enum b_audio_decoder_type type);
void audio_decoder_destroy(struct b_audio_resource *r);
int acquire_audio_decoders(struct b_connect *connect, bool force_grab);
void release_audio_decoders(struct b_connect *connect);
int audio_init(nxserver_t server);
void audio_uninit(void);
bool lacks_audio(struct b_connect *connect);
bool has_audio(struct b_connect *connect);

int acquire_audio_playbacks(struct b_connect *connect);
void release_audio_playbacks(struct b_connect *connect);
void audio_get_encode_resources(struct b_audio_resource *r, NEXUS_AudioMixerHandle *pMixer, NEXUS_SimpleAudioDecoderHandle *pMaster, NEXUS_SimpleAudioDecoderHandle *pSlave);
int audio_get_stc_index(struct b_connect *connect);

#if NEXUS_HAS_AUDIO
NEXUS_AudioCaptureHandle nxserverlib_open_audio_capture(struct b_session *session, unsigned *id);
void nxserverlib_close_audio_capture(struct b_session *session,unsigned id);
NEXUS_AudioCrcHandle nxserverlib_open_audio_crc(struct b_session *session, unsigned *id, NxClient_AudioCrcType crcType);
void nxserverlib_close_audio_crc(struct b_session *session,unsigned id);
#endif
int nxserverlib_audio_get_status(struct b_session *session, NxClient_AudioStatus *pStatus);

void nxserverlib_p_audio_get_audio_procesing_settings(struct b_session *session, NxClient_AudioProcessingSettings *pSettings);
int  nxserverlib_p_audio_set_audio_procesing_settings(struct b_session *session, const NxClient_AudioProcessingSettings *pSettings);
int  nxserverlib_p_swap_audio(struct b_connect *connect1, struct b_connect *connect2);
void nxserverlib_p_restart_audio(struct b_session *session);
int  nxserverlib_p_session_has_sd_audio(struct b_session *session);

/************
nxserverlib_input.c API
************/
int init_input_devices(struct b_session *session);
void uninit_input_devices(struct b_session *session);

/************
nxserverlib.c API
************/
bool nxserverlib_p_native_3d_active(struct b_session *session);
void inc_id(nxserver_t server, enum b_resource r);
bool is_transcode_connect(const struct b_connect *connect);
bool is_transcode_audiodec_request(const struct b_connect *connect);
bool nxserver_p_video_only_display(struct b_session *session, unsigned displayIndex);

typedef unsigned (*get_connect_id_func)(const NxClient_ConnectSettings *pSettings, unsigned i);
unsigned get_videodecoder_connect_id(const NxClient_ConnectSettings *pSettings, unsigned i);
unsigned get_audiodecoder_connect_id(const NxClient_ConnectSettings *pSettings, unsigned i);
unsigned get_audioplayback_connect_id(const NxClient_ConnectSettings *pSettings, unsigned i);
unsigned get_encoder_connect_id(const NxClient_ConnectSettings *pSettings, unsigned i);

#if NEXUS_HAS_HDMI_OUTPUT
void nxserverlib_p_apply_hdmi_drm(const struct b_session * session, const NxClient_DisplaySettings * pSettings);
#endif

/* get the index into req->handles.XXXX[] that matches the id */
typedef unsigned (*get_req_id_func)(struct b_req *req, unsigned i);
int get_req_index(struct b_req *req, get_req_id_func func, unsigned id, unsigned *pIndex);

/************
nxserverlib_transport.c API
************/
NEXUS_Error stc_pool_init(nxserver_t server);
void stc_pool_uninit(nxserver_t server);
void stc_index_request_init(struct b_connect * connect, struct b_stc_caps * pStcReq);
int stc_index_acquire(struct b_connect * connect, const struct b_stc_caps * pStcReq);
void stc_index_release(struct b_connect * connect, int index);

#endif /* NXSERVERLIB_IMPL_H__ */
