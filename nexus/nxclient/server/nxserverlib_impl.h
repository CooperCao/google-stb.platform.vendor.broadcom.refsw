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
 ******************************************************************************/
#ifndef NXSERVERLIB_IMPL_H__
#define NXSERVERLIB_IMPL_H__

#include "nxserverlib.h"
#include "nexus_types.h"
#include "nexus_platform.h"
#include "nexus_platform_server.h"
#include "nexus_display.h"
#if NEXUS_HAS_VIDEO_DECODER
#include "nexus_video_decoder.h"
#else
typedef void *NEXUS_VideoDecoderCapabilities;
#endif
#if NEXUS_HAS_AUDIO
#include "nexus_audio_decoder.h"
#include "nexus_audio_output.h"
#include "nexus_audio_playback.h"
#include "nexus_audio_mixer.h"
#include "nexus_audio_encoder.h"
#include "nexus_spdif_output.h"
#include "nexus_audio_capture.h"
#include "nexus_audio_crc.h"
#if NEXUS_AUDIO_BUFFER_CAPTURE_EXT
#include "nexus_audio_buffer_capture.h"
#endif
#endif
#include "nexus_core_compat.h"
#if NEXUS_HAS_HDMI_INPUT
#include "nexus_hdmi_input_hdcp.h"
#include "nexus_hdmi_input_hdcp_types.h"
#endif
#if NEXUS_HAS_HDMI_OUTPUT
#include "nexus_hdmi_output.h"
#include "nexus_hdmi_output_extra.h"
#include "nexus_hdmi_output_hdcp.h"
#else
typedef void *NEXUS_HdmiOutputHdcpVersion;
#endif
#if NEXUS_HAS_SIMPLE_DECODER
#include "nexus_simple_video_decoder_server.h"
#include "nexus_simple_audio_decoder_server.h"
#include "nexus_simple_encoder_server.h"
#include "nexus_simple_audio_playback_server.h"
#else
#undef NEXUS_HAS_VIDEO_DECODER
typedef void *NEXUS_SimpleAudioDecoderServerSettings;
typedef void *NEXUS_SimpleVideoDecoderServerSettings;
typedef void *NEXUS_SimpleAudioDecoderHandle;
typedef void *NEXUS_SimpleAudioPlaybackHandle;
typedef void *NEXUS_SimpleVideoDecoderHandle;
typedef void *NEXUS_SimpleEncoderHandle;
typedef void *NEXUS_SimpleVideoDecoderServerHandle;
typedef void *NEXUS_SimpleAudioDecoderServerHandle;
typedef void *NEXUS_SimpleEncoderServerHandle;
typedef void *NEXUS_SimpleAudioPlaybackServerHandle;
#endif
#if NEXUS_HAS_TRANSPORT
#include "nexus_stc_channel.h"
#else
typedef void *NEXUS_PidChannelHandle;
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
#if NEXUS_HAS_VIDEO_ENCODER
#include "nexus_video_encoder.h"
#endif
#include "nexus_memory.h"
#include "nexus_types.h"
#include "blst_list.h"
#include "blst_queue.h"
#include "nxserverlib_evdev.h"
#include "nxclient.h"
#include "nexus_watchdog.h"

#include <stdlib.h>
#include <string.h>

/* make nxserver more resilient to compiled-out modules */
#if !defined NEXUS_HAS_STREAM_MUX || !defined NEXUS_HAS_SIMPLE_DECODER
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
    unsigned maxPCMChannels; /* Max Number of PCM channels support by downstream device. */
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
        #if NEXUS_AUDIO_BUFFER_CAPTURE_EXT
        NEXUS_AudioBufferCaptureHandle handle;
        #else
        NEXUS_AudioCaptureHandle handle;
        #endif
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
    unsigned id;
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
    struct
    {
        enum b_hdmi_drm_source outputMode;
        enum b_hdmi_drm_source priorityMode;
    } dolbyVision;
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

typedef struct nxserver_cec_ir_input *nxserver_cec_ir_input_t;

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
        NEXUS_SimpleVideoDecoderServerHandle server;
    } video;
    struct {
        NEXUS_DisplayHandle display;
    } component, composite;
    struct {
        NEXUS_SimpleEncoderServerHandle server;
        /* streaming */
        struct encoder_resource *encoder;
    } encoder;

    struct {
        NEXUS_DisplayHandle display; /* created as init_session */
        unsigned global_index;
        bool created_by_encoder;
        NEXUS_VideoWindowHandle window[NEXUS_NUM_VIDEO_WINDOWS][NXCLIENT_MAX_IDS]; /* created on demand */
        struct b_connect *mosaic_connect[NEXUS_NUM_VIDEO_WINDOWS][NXCLIENT_MAX_IDS];
        NEXUS_VideoWindowHandle parentWindow[NEXUS_NUM_VIDEO_WINDOWS];
        NEXUS_VideoFormatInfo formatInfo;
        nxclient_t crc_client;
        NEXUS_SurfaceHandle graphic; /* for NxClient_SlaveDisplayMode_eGraphics */
        bool graphicsOnly;
    } display[NXCLIENT_MAX_DISPLAYS];
    NxClient_PictureQualitySettings pictureQualitySettings;
#if NEXUS_HAS_HDMI_OUTPUT
    NEXUS_HdmiOutputHandle hdmiOutput;
    struct
    {
        struct
        {
            struct b_hdmi_drm_selector selector;
            bool eotfValid;
            bool smdValid;
            NEXUS_HdmiDynamicRangeMasteringInfoFrame input;
#if NEXUS_HAS_VIDEO_DECODER
            NEXUS_VideoDecoderDynamicRangeMetadataType dynamicMetadataType;
#endif
            struct
            {
                NEXUS_HdmiOutputDolbyVisionMode outputMode;
                NEXUS_HdmiOutputDolbyVisionPriorityMode priorityMode;
            } dolbyVision;
            NEXUS_HdmiOutputEdidRxHdrdb hdrdb;
            NEXUS_VideoEotf lastInputEotf;
        } drm;
        struct {
            nxclient_t client;
        } repeater;
        bool defaultSdActive; /* our last format change was caused by defaultSdFormat */
    } hdmi;
#endif
#if NEXUS_HAS_HDMI_INPUT
    NEXUS_HdmiInputHandle hdmiInput;
#endif
#if NEXUS_HAS_HDMI_OUTPUT
    struct {
        NxClient_HdcpLevel level; /* currently applied settings */
        NxClient_HdcpVersion version_select;
        enum nxserver_hdcp_state {
            nxserver_hdcp_not_pending,                   /* no hdcp authentication in progress */
            nxserver_hdcp_begin,                         /* need to start hdcp authentication */
            nxserver_hdcp_follow,                        /* started in eFollow, waiting to learn result */
            nxserver_hdcp_pending_start_retry,           /* authenticating w/ restart. lastHdcpError likely set. may mute. */
            nxserver_hdcp_pending_start,                 /* authenticating w/o restart. */
            nxserver_hdcp_success,                       /* hdcp authentication completed */
            nxserver_hdcp_max
        } version_state;
        bool mute;
        NEXUS_HdmiOutputHdcpError lastHdcpError;
        NEXUS_HdmiOutputHdcpState prev_state;
        unsigned prev_state_cnt;
    } hdcp;
#endif
    struct {
        struct {
            void *buffer;
            unsigned size;
        } hdcp1x, hdcp2x;
    } hdcpKeys;
    nxclient_t hdmiOutput_crc_client;

    struct b_audio_resource *main_audio;
    NxClient_AudioProcessingSettings audioProcessingSettings;
    NxClient_AudioSettings audioSettings;
    struct {
        NEXUS_SimpleAudioDecoderServerHandle server;
        NEXUS_SimpleAudioPlaybackServerHandle playbackServer;
        struct {
            NxClient_AudioOutputMode outputMode[NEXUS_AudioCodec_eMax]; /* Configured output format */
        } hdmi, spdif;
    } audio;

    struct {
        NxClient_DisplaySettings displaySettings;
    } nxclient;

#if NEXUS_HAS_INPUT_ROUTER
    struct {
        NEXUS_IrInputHandle irInput[NXSERVER_IR_INPUTS];
        NEXUS_InputRouterHandle router;
#if NEXUS_HAS_KEYPAD
        NEXUS_KeypadHandle keypad;
        NEXUS_KeypadSettings keypadSettings;
#endif
        nxserver_evdev_t evdevInput;
        nxserver_cec_ir_input_t cec;
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

enum b_standby_state {
    b_standby_state_none,
    b_standby_state_pending,
    b_standby_state_applied,
    b_standby_state_exit
};

struct nxserver_watchdog_client {
    unsigned timeout;
    unsigned pettime; /* clock tick in seconds */
};

struct b_server {
    BLST_D_HEAD(b_client_list, b_client) clients; /* uses link */
    struct nxserver_settings settings;

    NEXUS_PlatformSettings platformSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_PlatformStatus platformStatus;
    NEXUS_PlatformCapabilities platformCap;
    struct b_session *session[NXCLIENT_MAX_SESSIONS];
    unsigned global_display_index;

    unsigned nextId[b_resource_max];
    struct {
        NEXUS_VideoDecoderCapabilities cap;
    } videoDecoder;
    struct {
        NEXUS_DisplayCapabilities cap;
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
        NEXUS_Timestamp last_standby_timestamp;
        NEXUS_Timestamp last_resume_timestamp;
        NEXUS_StandbyMode last_standby_mode;
    } standby;
    struct {
        BLST_D_HEAD(b_stc_list, b_stc) stcs;
    } transport;
    struct {
        bool allow_decode;
    } externalApp;
    struct {
        struct b_session *session;
        unsigned local_display_index;
        NEXUS_DisplaySettings settings;
    } disabled_local_display;
    struct {
        struct nxserver_watchdog_client state;
        NEXUS_WatchdogHandle handle;
        bool shutdown;
    } watchdog;
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
    struct nxclient_connect_settings connect_settings;
    nxserver_t server;
    unsigned pid;
    bool ipc; /* created over ipc, not local */
    struct b_session *session;
    NxClient_JoinSettings joinSettings;
    BLST_D_HEAD(b_req_list, b_req) requests;
    BLST_D_HEAD(b_connect_list, b_connect) connects;
    NxClient_HdcpLevel hdcp_level; /* client preferences */
    NxClient_HdcpVersion hdcp_version;
    bool hdmiInputRepeater;
    bool zombie;
    struct {
        BLST_S_HEAD(b_client_standby_acklist, b_client_standby_ack) acks;
    } standby;
    struct nxserver_watchdog_client watchdog;
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

/************
nxserverlib_video.c API
************/
struct video_decoder_resource;
int acquire_video_decoders(struct b_connect *connect, bool grab);
void release_video_decoders(struct b_connect *connect);
bool lacks_video(struct b_connect *connect);
void uninit_session_video(struct b_session *session);
void nxserverlib_video_disconnect_display(nxserver_t server, NEXUS_DisplayHandle display);
void nxserverlib_video_close_windows(struct b_session *session, unsigned local_display_index);
int video_init(nxserver_t server);
void video_uninit(void);
int video_get_stc_index(struct b_connect *connect);
int nxserverlib_p_swap_video_windows(struct b_connect *connect1, struct b_connect *connect2);
#if NEXUS_HAS_VIDEO_DECODER
NEXUS_VideoDecoderHandle nxserver_p_get_video_decoder(struct b_connect *connect);
#endif
void nxserverlib_p_clear_video_cache(void);

/************
nxserverlib_audio.c API
************/
int bserver_set_audio_config(struct b_audio_resource *ar);
int bserver_set_audio_mixer_config(struct b_audio_resource *ar);
void bserver_acquire_audio_mixers(struct b_audio_resource *r, bool start);
#if NEXUS_HAS_HDMI_OUTPUT
int bserver_hdmi_edid_audio_config(struct b_session *session, const NEXUS_HdmiOutputStatus *pStatus);
#endif
typedef enum b_audio_decoder_type
{
    b_audio_decoder_type_regular,
    b_audio_decoder_type_persistent,
    b_audio_decoder_type_standalone,
    b_audio_decoder_type_background,
    b_audio_decoder_type_background_nrt
} b_audio_decoder_type;
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
#if NEXUS_AUDIO_BUFFER_CAPTURE_EXT
NEXUS_AudioBufferCaptureHandle nxserverlib_open_audio_capture(struct b_session *session, unsigned *id, NxClient_AudioCaptureType captureType);
#else
NEXUS_AudioCaptureHandle nxserverlib_open_audio_capture(struct b_session *session, unsigned *id, NxClient_AudioCaptureType captureType);
#endif
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
int  nxserverlib_p_audio_i2s0_shares_with_dac(struct b_session *session);
bool b_dolby_ms_capable(const struct b_session *session);
void bserver_set_default_audio_settings(struct b_session *session);

/************
nxserverlib_input.c API
************/
int init_input_devices(struct b_session *session);
#if NEXUS_HAS_CEC
nxserver_cec_ir_input_t nxserverlib_init_cec_ir_input(struct b_session *session);
void nxserverlib_uninit_cec_ir_input(nxserver_cec_ir_input_t cec);
#endif
void uninit_input_devices(struct b_session *session);

/************
nxserverlib.c API
************/
bool nxserverlib_p_native_3d_active(struct b_session *session);
bool nxserverlib_p_dolby_vision_active(struct b_session *session);
void inc_id(nxserver_t server, enum b_resource r);
bool is_transcode_connect(const struct b_connect *connect);
bool is_transcode_audiodec_request(const struct b_connect *connect);
bool nxserver_p_video_only_display(struct b_session *session, unsigned displayIndex);
int nxserver_p_reenable_local_display(nxserver_t server);
void nxserver_p_disable_local_display(nxserver_t server, unsigned displayIndex);
bool nxserver_p_allow_grab(nxclient_t client);
NEXUS_Error nxserver_p_pet_watchdog(nxserver_t server, struct nxserver_watchdog_client *watchdog, unsigned timeout);

typedef unsigned (*get_connect_id_func)(const NxClient_ConnectSettings *pSettings, unsigned i);
unsigned get_videodecoder_connect_id(const NxClient_ConnectSettings *pSettings, unsigned i);
unsigned get_audiodecoder_connect_id(const NxClient_ConnectSettings *pSettings, unsigned i);
unsigned get_audioplayback_connect_id(const NxClient_ConnectSettings *pSettings, unsigned i);
unsigned get_encoder_connect_id(const NxClient_ConnectSettings *pSettings, unsigned i);

bool is_video_request(const struct b_connect *connect);
int b_connect_acquire(nxclient_t client, struct b_connect *connect);
void b_connect_release(nxclient_t client, struct b_connect *connect);
NEXUS_Error NxClient_P_SetDisplaySettingsNoRollback(nxclient_t client, struct b_session *session, const NxClient_DisplaySettings *pSettings);

#if NEXUS_HAS_HDMI_OUTPUT
void nxserverlib_p_apply_hdmi_drm(const struct b_session * session, const NxClient_DisplaySettings * pSettings, bool rxChanged);
#endif

/* get the index into req->handles.XXXX[] that matches the id */
typedef unsigned (*get_req_id_func)(struct b_req *req, unsigned i);
int get_req_index(struct b_req *req, get_req_id_func func, unsigned id, unsigned *pIndex);
unsigned nxserver_p_millisecond_tick(void);

/************
nxserverlib_transport.c API
************/
NEXUS_Error stc_pool_init(nxserver_t server);
void stc_pool_uninit(nxserver_t server);
void stc_index_request_init(struct b_connect * connect, struct b_stc_caps * pStcReq);
int stc_index_acquire(struct b_connect * connect, const struct b_stc_caps * pStcReq);
void stc_index_release(struct b_connect * connect, int index);
void nxserver_p_reserve_timebase(NEXUS_Timebase timebase);

/************
nxserverlib_thermal.c API
************/
NEXUS_Error nxserver_p_thermal_init(nxserver_t server);
void nxserver_p_thermal_uninit(nxserver_t server);
NEXUS_Error nxserver_get_thermal_status(nxclient_t client, NxClient_ThermalStatus *pStatus);

#endif /* NXSERVERLIB_IMPL_H__ */
