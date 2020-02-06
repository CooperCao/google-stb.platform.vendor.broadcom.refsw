/******************************************************************************
 *  Copyright (C) 2018 Broadcom.
 *  The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to
 *  the terms and conditions of a separate, written license agreement executed
 *  between you and Broadcom (an "Authorized License").  Except as set forth in
 *  an Authorized License, Broadcom grants no license (express or implied),
 *  right to use, or waiver of any kind with respect to the Software, and
 *  Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 *  THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 *  IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization,
 *  constitutes the valuable trade secrets of Broadcom, and you shall use all
 *  reasonable efforts to protect the confidentiality thereof, and to use this
 *  information only in connection with your use of Broadcom integrated circuit
 *  products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 *  "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 *  OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 *  RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 *  IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 *  A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *  ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *  THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 *  OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 *  INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 *  RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 *  HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 *  EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 *  WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 *  FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ******************************************************************************/
#include "nexus_simple_decoder_module.h"
#include "nexus_video_window.h"
#include "nexus_video_decoder.h"
#include "nexus_simple_audio_priv.h"
#include "nexus_simple_decoder_impl.h"
#include "nexus_client_resources.h"
#include "nexus_message.h"
#include "priv/nexus_core.h"
#include "priv/nexus_transport_priv.h"
#include "priv/nexus_pid_channel_priv.h"
#include "priv/nexus_display_priv.h"
#if NEXUS_HAS_STREAM_MUX
#include "nexus_stream_mux.h"
#include "priv/nexus_video_encoder_priv.h"
#endif
#ifdef NEXUS_NUM_DSP_VIDEO_ENCODERS
#include "nexus_video_adj.h"
#endif
#include "tshdrbuilder.h"
#if NEXUS_HAS_SECURITY
#include "nexus_security_client.h"
#endif

BDBG_MODULE(nexus_simple_encoder);

#define BDBG_MSG_TRACE(X)

struct NEXUS_SimpleEncoderServer
{
    NEXUS_OBJECT(NEXUS_SimpleEncoderServer);
    BLST_S_ENTRY(NEXUS_SimpleEncoderServer) link;
    BLST_S_HEAD(NEXUS_SimpleEncoder_P_List, NEXUS_SimpleEncoder) encoders;
};

static BLST_S_HEAD(NEXUS_SimpleEncoderServer_P_List, NEXUS_SimpleEncoderServer) g_NEXUS_SimpleEncoderServers;
#if NEXUS_HAS_STREAM_MUX
static NEXUS_Error NEXUS_SimpleEncoder_P_AddAuxPid(NEXUS_SimpleEncoderHandle handle, unsigned pid);
#endif

struct NEXUS_SimpleEncoder
{
    NEXUS_OBJECT(NEXUS_SimpleEncoder);
    BLST_S_ENTRY(NEXUS_SimpleEncoder) link;
    NEXUS_SimpleEncoderServerHandle server;
    unsigned index;
    bool acquired;
    bool clientStarted; /* user has called Start. if false, all other started variants must be false. */
    bool started; /* actually started or may be partially stopped */
    bool startAudio, startVideo; /* StartSettings say that audio or video should be started */
    bool audioEncoderAcquired; /* Audio Encoder is opened */
    bool videoEncoderStarted;
    bool abort;
    NEXUS_SimpleEncoderServerSettings serverSettings;
    NEXUS_SimpleEncoderStartSettings startSettings;
    NEXUS_SimpleEncoderSettings settings;
    NEXUS_TaskCallbackHandle resourceChangedCallback;
    NEXUS_TaskCallbackHandle finishedCallback;
    NEXUS_CallbackHandler finishedHandler;
#if NEXUS_HAS_STREAM_MUX
    void *streamMux;
#else
    NEXUS_StreamMuxHandle streamMux;
#endif
    NEXUS_DisplayHandle transcodeDisplay;
    NEXUS_VideoWindowHandle transcodeWindow;
    struct {
        NEXUS_MemoryBlockHandle block;
        void *buffer;
        NEXUS_TimerHandle timer;
        unsigned ccValue;
    } psi;
    struct {
        bool video, audio;
    } wait;
    struct {
        struct {
            NEXUS_PidChannelHandle pidChannel;
        } video, audio, passthrough[NEXUS_SIMPLE_ENCODER_NUM_PASSTHROUGH_PIDS];
    } pids;

    /* move structs off stack */
#if NEXUS_HAS_STREAM_MUX
    struct {
        NEXUS_VideoEncoderDelayRange videoDelay;
        NEXUS_VideoEncoderStartSettings videoEncoderStartSettings;
        NEXUS_AudioMuxOutputStartSettings audioMuxStartSettings;
        NEXUS_AudioMuxOutputDelayStatus audioDelayStatus;
    } stack;
#endif
    NEXUS_MessageHandle message[NEXUS_SIMPLE_ENCODER_NUM_PASSTHROUGH_PIDS];
    struct {
        unsigned total, max;
        unsigned *pid;
    } auxpid;
    struct {
        bool inProgress;
    } programChange;
};

static NEXUS_SimpleEncoderHandle nexus_simple_encoder_p_first(void)
{
    NEXUS_SimpleEncoderServerHandle server;
    for (server = BLST_S_FIRST(&g_NEXUS_SimpleEncoderServers); server; server = BLST_S_NEXT(server, link)) {
        NEXUS_SimpleEncoderHandle encoder = BLST_S_FIRST(&server->encoders);
        if (encoder) return encoder;
    }
    return NULL;
}

static NEXUS_SimpleEncoderHandle nexus_simple_encoder_p_next(NEXUS_SimpleEncoderHandle handle)
{
    NEXUS_SimpleEncoderHandle next;
    next = BLST_S_NEXT(handle, link);
    if (!next) {
        NEXUS_SimpleEncoderServerHandle server;
        for (server = BLST_S_NEXT(handle->server, link); server; server = BLST_S_NEXT(server, link)) {
            next = BLST_S_FIRST(&server->encoders);
            if (next) break;
        }
    }
    return next;
}

static NEXUS_Error nexus_simpleencoder_p_start( NEXUS_SimpleEncoderHandle handle );
#if NEXUS_HAS_STREAM_MUX
static NEXUS_Error nexus_simpleencoder_p_pre_start( NEXUS_SimpleEncoderHandle handle );
static NEXUS_Error nexus_simpleencoder_p_start_psi( NEXUS_SimpleEncoderHandle handle );
static void nexus_simpleencoder_p_stop_psi( NEXUS_SimpleEncoderHandle handle );
static NEXUS_StcChannelHandle nexus_simpleencoder_p_getStcChannel( NEXUS_SimpleEncoderHandle handle, NEXUS_SimpleDecoderType type );
#endif
static bool ready_to_start(NEXUS_SimpleEncoderHandle handle);

NEXUS_SimpleEncoderServerHandle NEXUS_SimpleEncoderServer_Create(void)
{
    NEXUS_SimpleEncoderServerHandle handle;
    handle = BKNI_Malloc(sizeof(*handle));
    if (!handle) return NULL;
    NEXUS_OBJECT_INIT(NEXUS_SimpleEncoderServer, handle);
    BLST_S_INSERT_HEAD(&g_NEXUS_SimpleEncoderServers, handle, link);
    return handle;
}

static void NEXUS_SimpleEncoderServer_P_Finalizer( NEXUS_SimpleEncoderServerHandle handle )
{
    BLST_S_REMOVE(&g_NEXUS_SimpleEncoderServers, handle, NEXUS_SimpleEncoderServer, link);
    NEXUS_OBJECT_DESTROY(NEXUS_SimpleEncoderServer, handle);
    BKNI_Free(handle);
}

NEXUS_OBJECT_CLASS_MAKE(NEXUS_SimpleEncoderServer, NEXUS_SimpleEncoderServer_Destroy);

static void NEXUS_SimpleEncoder_P_StreamMuxFinished(void *context)
{
    NEXUS_SimpleEncoderHandle handle = context;
    NEXUS_TaskCallback_Fire(handle->finishedCallback);
}

NEXUS_SimpleEncoderHandle NEXUS_SimpleEncoder_Create( NEXUS_SimpleEncoderServerHandle server, unsigned index )
{
    NEXUS_SimpleEncoderHandle handle;

    /* find dup */
    for (handle=BLST_S_FIRST(&server->encoders); handle; handle=BLST_S_NEXT(handle, link)) {
        if (handle->index == index) {
            BERR_TRACE(NEXUS_INVALID_PARAMETER);
            return NULL;
        }
    }

    handle = BKNI_Malloc(sizeof(*handle));
    if (!handle) {
        BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
        return NULL;
    }
    NEXUS_OBJECT_INIT(NEXUS_SimpleEncoder, handle);
    handle->index = index;
    handle->server = server;
    NEXUS_OBJECT_REGISTER(NEXUS_SimpleEncoder, handle, Create);

    /* insert in order. not required, but makes debug easier */
    if (!BLST_S_FIRST(&server->encoders)) {
        BLST_S_INSERT_HEAD(&server->encoders, handle, link);
    }
    else {
        NEXUS_SimpleEncoderHandle prev;
        for (prev=BLST_S_FIRST(&server->encoders);;prev=BLST_S_NEXT(prev, link)) {
            if (!BLST_S_NEXT(prev, link)) {
                BLST_S_INSERT_AFTER(&server->encoders, prev, handle, link);
                break;
            }
        }
    }
    handle->resourceChangedCallback = NEXUS_TaskCallback_Create(handle, NULL);
    handle->finishedCallback = NEXUS_TaskCallback_Create(handle, NULL);
    NEXUS_CallbackHandler_Init(handle->finishedHandler, NEXUS_SimpleEncoder_P_StreamMuxFinished, handle);
    /* now a valid object */

    handle->settings.video.interlaced = false;
    handle->settings.video.refreshRate = 60000;
#if NEXUS_HAS_STREAM_MUX
    NEXUS_VideoEncoder_GetDefaultSettings(&handle->settings.videoEncoder);
    handle->settings.videoEncoder.variableFrameRate = false;
#if NEXUS_NUM_DSP_VIDEO_ENCODERS && !NEXUS_DSP_ENCODER_ACCELERATOR_SUPPORT
#define MAX_DSP_VIDEO_WINDOW_WIDTH 416
#define MAX_DSP_VIDEO_WINDOW_HEIGHT 224
    /* this is required for raaga video encode */
    handle->settings.videoEncoder.frameRate = NEXUS_VideoFrameRate_e29_97;
    handle->settings.video.width = MAX_DSP_VIDEO_WINDOW_WIDTH;
    handle->settings.video.height = MAX_DSP_VIDEO_WINDOW_HEIGHT;
    handle->settings.videoEncoder.bitrateMax = 400000; /* 400 Kbps for 416x224p30 dsp */
#else
    handle->settings.videoEncoder.frameRate = NEXUS_VideoFrameRate_e30;
    #if NEXUS_DSP_ENCODER_ACCELERATOR_SUPPORT /* default 480p dsp encoder accelerater */
    handle->settings.videoEncoder.bitrateMax = 2000000; /* 2 Mbps for 480p30 dsp accelerator; TODO: optimize bit efficiency */
    #else
    handle->settings.videoEncoder.bitrateMax = 2500000; /* 2.5 Mbps for 720p30 vce */
    #endif
    NEXUS_VideoEncoder_GetDefaultStartSettings(&handle->stack.videoEncoderStartSettings);
    handle->settings.video.width  = handle->stack.videoEncoderStartSettings.bounds.inputDimension.max.width;
    handle->settings.video.height = handle->stack.videoEncoderStartSettings.bounds.inputDimension.max.height;
#endif
    handle->settings.videoEncoder.streamStructure.framesP = 29; /* 29 P frames per gop */
    handle->settings.videoEncoder.streamStructure.framesB = 0; /* IP gop */
    handle->settings.videoEncoder.enableFieldPairing = true;
#endif
    handle->settings.streamMux.enable.video[0] = true;
    handle->settings.streamMux.enable.audio[0] = true;
    NEXUS_CallbackDesc_Init(&handle->settings.resourceChanged);
    NEXUS_CallbackDesc_Init(&handle->settings.finished);

#if NEXUS_HAS_AUDIO
    NEXUS_AudioEncoder_GetDefaultCodecSettings(NEXUS_AudioCodec_eAac, &handle->settings.audioEncoder);
#endif

    return handle;
}

static void NEXUS_SimpleEncoder_P_Release( NEXUS_SimpleEncoderHandle handle )
{
    NEXUS_OBJECT_ASSERT(NEXUS_SimpleEncoder, handle);
    if (handle->acquired) {
        handle->abort = true; /* server aborts encoder internally */
        NEXUS_SimpleEncoder_Release(handle);
    }
    BDBG_ASSERT(!handle->acquired);
    BDBG_ASSERT(!handle->started);
    NEXUS_OBJECT_UNREGISTER(NEXUS_SimpleEncoder, handle, Destroy);
    return;
}

static void NEXUS_SimpleEncoder_P_Finalizer( NEXUS_SimpleEncoderHandle handle )
{
    NEXUS_OBJECT_ASSERT(NEXUS_SimpleEncoder, handle);
    BLST_S_REMOVE(&handle->server->encoders, handle, NEXUS_SimpleEncoder, link);
    if (handle->resourceChangedCallback) {
        NEXUS_TaskCallback_Destroy(handle->resourceChangedCallback);
    }
    NEXUS_CallbackHandler_Shutdown(handle->finishedHandler);
    if (handle->finishedCallback) {
        NEXUS_TaskCallback_Destroy(handle->finishedCallback);
    }
    NEXUS_OBJECT_DESTROY(NEXUS_SimpleEncoder, handle);
    BKNI_Free(handle);
    return;
}

NEXUS_OBJECT_CLASS_MAKE_WITH_RELEASE(NEXUS_SimpleEncoder, NEXUS_SimpleEncoder_Destroy);

void NEXUS_SimpleEncoder_GetServerSettings( NEXUS_SimpleEncoderServerHandle server, NEXUS_SimpleEncoderHandle handle, NEXUS_SimpleEncoderServerSettings *pSettings )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_SimpleEncoder);
    if (handle->server != server) {BERR_TRACE(NEXUS_INVALID_PARAMETER); return;}
    *pSettings = handle->serverSettings;
}

NEXUS_Error NEXUS_SimpleEncoder_SetServerSettings( NEXUS_SimpleEncoderServerHandle server, NEXUS_SimpleEncoderHandle handle, const NEXUS_SimpleEncoderServerSettings *pSettings )
{
    bool change;

    BDBG_OBJECT_ASSERT(handle, NEXUS_SimpleEncoder);
    if (handle->server != server) return BERR_TRACE(NEXUS_INVALID_PARAMETER);

    change = (handle->serverSettings.videoEncoder != pSettings->videoEncoder);

    /* we might be losing our decoders, so disconnect */
    if (handle->serverSettings.videoEncoder) {
        nexus_simpleencoder_p_stop(handle);
    }

    handle->serverSettings = *pSettings;

    /* encoder cannot be automatically started here */

    if (change) {
        NEXUS_TaskCallback_Fire(handle->resourceChangedCallback);
    }

    return 0;
}

NEXUS_SimpleEncoderHandle NEXUS_SimpleEncoder_Acquire( unsigned index )
{
    NEXUS_SimpleEncoderHandle handle;
    for (handle=nexus_simple_encoder_p_first(); handle; handle = nexus_simple_encoder_p_next(handle)) {
        BDBG_OBJECT_ASSERT(handle, NEXUS_SimpleEncoder);
        if (handle->index == index) {
            int rc;
            if (handle->acquired) {
                BERR_TRACE(NEXUS_NOT_AVAILABLE);
                return NULL;
            }

            rc = NEXUS_CLIENT_RESOURCES_ACQUIRE(simpleEncoder,IdList,index);
            if (rc) { rc = BERR_TRACE(rc); return NULL; }

            handle->acquired = true;
            return handle;
        }
    }
    return NULL;
}

void NEXUS_SimpleEncoder_Release( NEXUS_SimpleEncoderHandle handle )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_SimpleEncoder);
    /* IPC handle validation will only allow this call if handle is owned by client.
    For non-IPC used, acquiring is not required, so acquired boolean is not checked in any other API. */
    if (!handle->acquired) {
        BERR_TRACE(NEXUS_NOT_AVAILABLE);
        return;
    }

    /* it could be stopped in video encoder only mode earlier, also need to fully stopped. */
    if (handle->started) {
        /* client settings are lost anyway, so force it */
        handle->settings.stopMode = NEXUS_SimpleEncoderStopMode_eAll;
        NEXUS_SimpleEncoder_Stop(handle);
    }

    NEXUS_CLIENT_RESOURCES_RELEASE(simpleEncoder,IdList,handle->index);
    handle->acquired = false;

    NEXUS_TaskCallback_Clear(handle->resourceChangedCallback);
    NEXUS_TaskCallback_Clear(handle->finishedCallback);
}

void NEXUS_SimpleEncoder_GetDefaultStartSettings( NEXUS_SimpleEncoderStartSettings *pSettings )
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->output.video.pid = 0x11;
#if NEXUS_HAS_STREAM_MUX
    NEXUS_VideoEncoder_GetDefaultStartSettings(&pSettings->output.video.settings);
    pSettings->output.video.settings.codec = NEXUS_VideoCodec_eH264;
    pSettings->output.video.settings.profile = NEXUS_VideoCodecProfile_eMain;/* main/high profile enabled CABAC to improve bit efficiency */
    pSettings->output.video.settings.level = NEXUS_VideoCodecLevel_e31;
#endif
    pSettings->output.audio.pid = 0x14;
    pSettings->output.audio.codec = NEXUS_AudioCodec_eAac;
    pSettings->output.transport.type = NEXUS_TransportType_eTs;
    pSettings->output.transport.pcrPid = 0x15;
    pSettings->output.transport.pcrInterval = 50;
    pSettings->output.transport.pmtPid = 0x55;
    pSettings->output.transport.interval = 1000; /* 1 second */
    pSettings->transcode.nonRealTimeRate = 32 * NEXUS_NORMAL_PLAY_SPEED; /* AFAP */
}

NEXUS_Error NEXUS_SimpleEncoder_Start( NEXUS_SimpleEncoderHandle handle, const NEXUS_SimpleEncoderStartSettings *pSettings )
{
    NEXUS_Error rc;
    unsigned i;
    BDBG_OBJECT_ASSERT(handle, NEXUS_SimpleEncoder);

#if NEXUS_HAS_STREAM_MUX
    if (handle->clientStarted) {
        /* NEXUS_SimpleEncoderStopMode_eVideoEncoderOnly was called. NEXUS_SimpleEncoderStartSettings must be the same. */
        if (BKNI_Memcmp(pSettings, &handle->startSettings, sizeof(*pSettings))) {
            return BERR_TRACE(NEXUS_INVALID_PARAMETER);
        }
        goto encoder_start;
    }
#endif

    if (!pSettings->input.video && !pSettings->input.audio && !pSettings->input.display) {
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }
    if (pSettings->input.display) {
        if (handle->serverSettings.nonRealTime) {
            return BERR_TRACE(NEXUS_INVALID_PARAMETER);
        }
        if (!handle->serverSettings.displayEncode.display) {
            return BERR_TRACE(NEXUS_INVALID_PARAMETER);
        }
        if (pSettings->input.video || pSettings->input.audio) {
            return BERR_TRACE(NEXUS_INVALID_PARAMETER);
        }
    }
    if (pSettings->output.transport.type == NEXUS_TransportType_eTs && !pSettings->recpump) {
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }

    handle->startAudio = (pSettings->input.display || pSettings->input.audio) && (pSettings->output.audio.codec != NEXUS_AudioCodec_eUnknown);
    if (handle->startAudio && !handle->serverSettings.audioMuxOutput) {
        handle->startAudio = false;
        return BERR_TRACE(NEXUS_NOT_AVAILABLE);
    }
    handle->startVideo = pSettings->input.display || pSettings->input.video;
    if (handle->startVideo && !handle->serverSettings.videoEncoder) {
        handle->startAudio = false;
        handle->startVideo = false;
        return BERR_TRACE(NEXUS_NOT_AVAILABLE);
    }

    if (pSettings->input.video) {
        NEXUS_OBJECT_ACQUIRE(handle, NEXUS_SimpleVideoDecoder, pSettings->input.video);
    }
    if (pSettings->input.audio) {
        NEXUS_OBJECT_ACQUIRE(handle, NEXUS_SimpleAudioDecoder, pSettings->input.audio);
    }
    if (pSettings->transcode.display) {
        NEXUS_OBJECT_ACQUIRE(handle, NEXUS_Display, pSettings->transcode.display);
    }
    if ( pSettings->recpump ) {
        NEXUS_OBJECT_ACQUIRE(handle, NEXUS_Recpump, pSettings->recpump);
    }
    for (i=0;i<NEXUS_SIMPLE_ENCODER_NUM_PASSTHROUGH_PIDS;i++) {
        if (pSettings->passthrough[i]) {
            NEXUS_OBJECT_ACQUIRE(handle, NEXUS_PidChannel, pSettings->passthrough[i]);
        }
    }

    handle->clientStarted = true;
    handle->startSettings = *pSettings;

#if NEXUS_HAS_STREAM_MUX
    /* try to make defaults work out as much as possible */
    switch (handle->startSettings.output.video.settings.codec) {
    case NEXUS_VideoCodec_eMpeg2:
        /* this is the only profile and level supported, so default to it */
        handle->startSettings.output.video.settings.profile = NEXUS_VideoCodecProfile_eMain;
        handle->startSettings.output.video.settings.level = NEXUS_VideoCodecLevel_eMain;
        break;
    default:
        break;
    }

    rc = nexus_simpleencoder_p_pre_start(handle);
    if (rc) {
        rc = BERR_TRACE(rc);
        NEXUS_SimpleEncoder_Stop(handle);
        return rc;
    }

encoder_start:
    if (ready_to_start(handle)) {
        rc = nexus_simpleencoder_p_start(handle);
        if (rc) {
            NEXUS_SimpleEncoder_Stop(handle);
            return BERR_TRACE(rc);
        }
    }
#else
    BSTD_UNUSED(rc);
#endif

    /* now wait for decoders for actual start of encoder */

    return 0;
}

void NEXUS_SimpleEncoder_Stop( NEXUS_SimpleEncoderHandle handle )
{
    unsigned i;
    BDBG_OBJECT_ASSERT(handle, NEXUS_SimpleEncoder);

    if (handle->clientStarted) {
        nexus_simpleencoder_p_stop(handle);
    }
    if (!handle->clientStarted) {
        if (handle->startSettings.input.video) {
            NEXUS_OBJECT_RELEASE(handle, NEXUS_SimpleVideoDecoder, handle->startSettings.input.video);
        }
        if (handle->startSettings.input.audio) {
            NEXUS_OBJECT_RELEASE(handle, NEXUS_SimpleAudioDecoder, handle->startSettings.input.audio);
        }
        if (handle->startSettings.transcode.display) {
            NEXUS_OBJECT_RELEASE(handle, NEXUS_Display, handle->startSettings.transcode.display);
        }
        if (handle->startSettings.recpump) {
            NEXUS_OBJECT_RELEASE(handle, NEXUS_Recpump, handle->startSettings.recpump);
        }
        for (i=0;i<NEXUS_SIMPLE_ENCODER_NUM_PASSTHROUGH_PIDS;i++) {
            if (handle->startSettings.passthrough[i]) {
                NEXUS_OBJECT_RELEASE(handle, NEXUS_PidChannel, handle->startSettings.passthrough[i]);
            }
        }
        NEXUS_SimpleEncoder_GetDefaultStartSettings(&handle->startSettings);
    }
}

void NEXUS_SimpleEncoder_GetSettings( NEXUS_SimpleEncoderHandle handle, NEXUS_SimpleEncoderSettings *pSettings )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_SimpleEncoder);
    *pSettings = handle->settings;
}

NEXUS_Error NEXUS_SimpleEncoder_SetSettings( NEXUS_SimpleEncoderHandle handle, const NEXUS_SimpleEncoderSettings *pSettings )
{
    int rc = NEXUS_SUCCESS;
    BDBG_OBJECT_ASSERT(handle, NEXUS_SimpleEncoder);
    NEXUS_TaskCallback_Set(handle->resourceChangedCallback, &pSettings->resourceChanged);
    NEXUS_TaskCallback_Set(handle->finishedCallback, &pSettings->finished);

    if (!handle->clientStarted) {
        /* if client not started, we don't know if this is background or display encode, so defer */
        goto done;
    }
#if NEXUS_HAS_STREAM_MUX
#if NEXUS_NUM_DSP_VIDEO_ENCODERS && !NEXUS_DSP_ENCODER_ACCELERATOR_SUPPORT
#else
    /* client doesn't change display format when encoding the display */
    if (handle->transcodeDisplay && !handle->startSettings.input.display) {
        NEXUS_VideoFrameRate frameRate;
        NEXUS_VideoFormat format;
        NEXUS_VideoFormatInfo info;
        NEXUS_P_FrameRate_FromRefreshRate_isrsafe(pSettings->video.refreshRate, &frameRate);
        format = NEXUS_P_VideoFormat_FromInfo_isrsafe(pSettings->video.height, frameRate, pSettings->video.interlaced);
        /* NEXUS_P_VideoFormat_FromInfo_isrsafe is not an exact match, so we must do so */
        NEXUS_VideoFormat_GetInfo(format, &info);
        if (format != NEXUS_VideoFormat_eUnknown &&
            info.width == pSettings->video.width &&
            info.height == pSettings->video.height &&
            info.verticalFreq == pSettings->video.refreshRate/10 &&
            info.interlaced == pSettings->video.interlaced) {
            NEXUS_DisplaySettings settings;

            NEXUS_Display_GetSettings(handle->transcodeDisplay, &settings);
            settings.format = format;
            settings.aspectRatio = NEXUS_DisplayAspectRatio_e16x9; /* don't care */
            settings.display3DSettings.overrideOrientation = pSettings->video.display3DSettings.overrideOrientation;
            settings.display3DSettings.orientation = pSettings->video.display3DSettings.orientation;
            rc = NEXUS_Display_SetSettings(handle->transcodeDisplay, &settings);
            if (rc) return BERR_TRACE(rc);
        }
        else {
            NEXUS_DisplayCustomFormatSettings customFormatSettings;

            if (pSettings->video.display3DSettings.overrideOrientation) {
                /* we currently only support 3D with standard video formats */
                return BERR_TRACE(NEXUS_NOT_SUPPORTED);
            }
            NEXUS_Display_GetDefaultCustomFormatSettings(&customFormatSettings);
            customFormatSettings.width = pSettings->video.width;
            customFormatSettings.height = pSettings->video.height;
            customFormatSettings.refreshRate = pSettings->video.refreshRate;
            customFormatSettings.interlaced = pSettings->video.interlaced;
            customFormatSettings.aspectRatio = NEXUS_DisplayAspectRatio_e16x9; /* don't care */
            rc = NEXUS_Display_SetCustomFormatSettings(handle->transcodeDisplay, NEXUS_VideoFormat_eCustom2, &customFormatSettings);
            if (rc) return BERR_TRACE(rc);
        }
    }
    if (handle->transcodeWindow) {
        NEXUS_VideoWindowSettings windowSettings;
        NEXUS_VideoWindow_GetSettings(handle->transcodeWindow, &windowSettings);
        if (pSettings->video.window.width && pSettings->video.window.height) {
            windowSettings.position = pSettings->video.window;
        }
        else {
            windowSettings.position.x = 0;
            windowSettings.position.y = 0;
            windowSettings.position.width = pSettings->video.width;
            windowSettings.position.height = pSettings->video.height;
        }
        if (pSettings->video.display3DSettings.overrideOrientation) {
            switch (pSettings->video.display3DSettings.orientation) {
            case NEXUS_VideoOrientation_e3D_LeftRight:
                windowSettings.position.width /= 2;
                break;
            case NEXUS_VideoOrientation_e3D_OverUnder:
                windowSettings.position.height /= 2;
                break;
            default:
                break;
            }
        }
        windowSettings.sourceClip.left   = pSettings->video.clip.left;
        windowSettings.sourceClip.right  = pSettings->video.clip.right;
        windowSettings.sourceClip.top    = pSettings->video.clip.top;
        windowSettings.sourceClip.bottom = pSettings->video.clip.bottom;
        rc = NEXUS_VideoWindow_SetSettings(handle->transcodeWindow, &windowSettings);
        if (rc) return BERR_TRACE(rc);
    }
#endif

    if (handle->audioEncoderAcquired) {
        if (handle->startSettings.input.display) {
            rc = nexus_simpleaudiodecoder_p_encoder_set_codec_settings (handle->serverSettings.displayEncode.masterAudio, &pSettings->audioEncoder);
        }
        else if (!handle->startSettings.output.audio.passthrough) {
            rc = nexus_simpleaudiodecoder_p_encoder_set_codec_settings (handle->startSettings.input.audio, &pSettings->audioEncoder);
        }
        if (rc) return BERR_TRACE(rc);
    }

    if (handle->serverSettings.videoEncoder) {
        rc = NEXUS_VideoEncoder_SetSettings(handle->serverSettings.videoEncoder, &pSettings->videoEncoder);
        if (rc) return BERR_TRACE(rc);
    }
    if (handle->streamMux) {
        NEXUS_StreamMuxSettings settings;
        NEXUS_StreamMux_GetSettings(handle->streamMux, &settings);
        if (settings.enable.video[0] != pSettings->streamMux.enable.video[0] ||
            settings.enable.audio[0] != pSettings->streamMux.enable.audio[0]) {
            settings.enable.video[0] = pSettings->streamMux.enable.video[0];
            settings.enable.audio[0] = pSettings->streamMux.enable.audio[0];
            rc = NEXUS_StreamMux_SetSettings(handle->streamMux, &settings);
            if (rc) return BERR_TRACE(rc);
        }
    }
#else
    BSTD_UNUSED(rc);
#endif

done:
    handle->settings = *pSettings;
    return 0;
}

static bool ready_to_start(NEXUS_SimpleEncoderHandle handle)
{
    return handle->clientStarted && !handle->wait.audio && !handle->wait.video && (!handle->started || !handle->videoEncoderStarted || handle->programChange.inProgress);
}

NEXUS_Error nexus_simpleencoder_p_start_audio(NEXUS_SimpleEncoderHandle handle)
{
    NEXUS_Error rc = 0;
    BDBG_OBJECT_ASSERT(handle, NEXUS_SimpleEncoder);
    if (!handle->wait.audio) return rc;
    handle->wait.audio = false;
    if (ready_to_start(handle)) {
        rc = nexus_simpleencoder_p_start(handle);
        if (rc) {
            NEXUS_SimpleEncoder_Stop(handle);
        }
    }
    return rc;
}

NEXUS_Error nexus_simpleencoder_p_start_video(NEXUS_SimpleEncoderHandle handle)
{
    NEXUS_Error rc = 0;
    BDBG_OBJECT_ASSERT(handle, NEXUS_SimpleEncoder);
    if (!handle->wait.video) return rc;
    handle->wait.video = false;
    if (ready_to_start(handle)) {
        rc = nexus_simpleencoder_p_start(handle);
        if (rc) {
            NEXUS_SimpleEncoder_Stop(handle);
        }
    }
    return rc;
}

#if NEXUS_HAS_STREAM_MUX
/**
pre_start validates state, creates any window, and links the decoders. this allows the decoders to start.
**/
static NEXUS_Error nexus_simpleencoder_p_pre_start( NEXUS_SimpleEncoderHandle handle )
{
    int rc;

    if (handle->started) {
        return BERR_TRACE(NEXUS_NOT_AVAILABLE);
    }
    /* if (handle->startSettings.input.display), nothing to do for video */
    if (handle->startSettings.input.video) {
#if NEXUS_NUM_DSP_VIDEO_ENCODERS && !NEXUS_DSP_ENCODER_ACCELERATOR_SUPPORT
        NEXUS_VideoWindowSettings windowSettings;
#endif

        if (handle->startSettings.transcode.display) {
            handle->transcodeDisplay = handle->startSettings.transcode.display;
        }
        else {
            NEXUS_DisplaySettings displaySettings;
            NEXUS_Display_GetDefaultSettings(&displaySettings);
            displaySettings.format = NEXUS_VideoFormat_e480p;
#if NEXUS_NUM_DSP_VIDEO_ENCODERS && !NEXUS_DSP_ENCODER_ACCELERATOR_SUPPORT
#else
            displaySettings.timingGenerator = NEXUS_DisplayTimingGenerator_eEncoder;
            displaySettings.frameRateMaster = NULL; /* disable frame rate tracking for now */
#endif
            handle->transcodeDisplay = NEXUS_Display_Open(handle->serverSettings.transcodeDisplayIndex, &displaySettings);
            if (!handle->transcodeDisplay) {
                rc = BERR_TRACE(NEXUS_NOT_AVAILABLE);
                goto err_opendisplay;
            }
        }
        if (handle->serverSettings.nonRealTime) {
            NEXUS_DisplayStgSettings stgSettings;
            NEXUS_Display_GetStgSettings(handle->transcodeDisplay, &stgSettings);
            stgSettings.enabled = true;
            stgSettings.nonRealTime = true;
            NEXUS_Display_SetStgSettings(handle->transcodeDisplay, &stgSettings);
        }
        if (handle->serverSettings.headless) {
            NEXUS_Display_DriveVideoDecoder(handle->transcodeDisplay);
        }

        handle->transcodeWindow = NEXUS_VideoWindow_Open(handle->transcodeDisplay, 0);
        if (!handle->transcodeWindow) {
            rc = BERR_TRACE(NEXUS_NOT_AVAILABLE);
            goto err_openwindow;
        }

#if NEXUS_NUM_DSP_VIDEO_ENCODERS && !NEXUS_DSP_ENCODER_ACCELERATOR_SUPPORT
        NEXUS_VideoWindow_GetSettings(handle->transcodeWindow, &windowSettings);
        if (handle->settings.video.width > MAX_DSP_VIDEO_WINDOW_WIDTH ||
            handle->settings.video.height > MAX_DSP_VIDEO_WINDOW_HEIGHT)
        {
            BDBG_ERR(("%dx%d exceeds DSP encoder limit of %dx%d", handle->settings.video.width, handle->settings.video.height,
                MAX_DSP_VIDEO_WINDOW_WIDTH, MAX_DSP_VIDEO_WINDOW_HEIGHT));
            rc = BERR_TRACE(NEXUS_INVALID_PARAMETER);
            goto err_encodersettings;
        }
        windowSettings.position.width = handle->settings.video.width;
        windowSettings.position.height = handle->settings.video.height;
        if (handle->settings.video.display3DSettings.overrideOrientation) {
            rc = BERR_TRACE(NEXUS_INVALID_PARAMETER);
            goto err_encodersettings;
        }
        windowSettings.pixelFormat = NEXUS_PixelFormat_eCr8_Y18_Cb8_Y08;
        windowSettings.visible = false;
        rc = NEXUS_VideoWindow_SetSettings(handle->transcodeWindow, &windowSettings);
        if (rc) {rc = BERR_TRACE(rc); goto err_encodersettings;}
#endif

        /* call once at pre-start and a second time at start */
        rc = NEXUS_SimpleEncoder_SetSettings(handle, &handle->settings);
        if (rc) {rc = BERR_TRACE(rc); goto err_encodersettings;}

        nexus_simplevideodecoder_p_add_encoder(handle->startSettings.input.video, handle->transcodeWindow, handle);

#if NEXUS_NUM_DSP_VIDEO_ENCODERS && !NEXUS_DSP_ENCODER_ACCELERATOR_SUPPORT
        /* DSP video encode requires waiting for decode to start */
        handle->wait.video = true;
#endif
    }

    if (handle->startAudio) {
        if (handle->startSettings.input.display) {
            rc = nexus_simpleaudiodecoder_p_add_encoder(
                handle->serverSettings.displayEncode.masterAudio,
                handle->serverSettings.displayEncode.slaveAudio,
                handle->serverSettings.audioMuxOutput,
                handle->startSettings.output.audio.codec,
                handle->startSettings.output.audio.passthrough,
                handle->startSettings.output.audio.sampleRate,
                handle,
                handle->serverSettings.mixer,
                true);
            if (rc) {rc = BERR_TRACE(rc); goto err_addaudioinput;}
        }
        else {
            rc = nexus_simpleaudiodecoder_p_add_encoder(
                handle->startSettings.input.audio,
                NULL,
                handle->serverSettings.audioMuxOutput,
                handle->startSettings.output.audio.codec,
                handle->startSettings.output.audio.passthrough,
                handle->startSettings.output.audio.sampleRate,
                handle,
                handle->serverSettings.mixer,
                false);
            if (rc) {rc = BERR_TRACE(rc); goto err_addaudioinput;}
        }
#if 0
        handle->wait.audio = true;
#endif
        handle->audioEncoderAcquired = true;
    }

    return 0;

err_addaudioinput:
    if (handle->startSettings.input.video) {
        nexus_simplevideodecoder_p_remove_encoder(handle->startSettings.input.video, handle->transcodeWindow, handle);
    }
err_encodersettings:
    if (handle->transcodeWindow) {
        NEXUS_VideoWindow_Close(handle->transcodeWindow);
        handle->transcodeWindow = NULL;
    }
err_openwindow:
    if (handle->transcodeDisplay && !handle->startSettings.transcode.display) {
        NEXUS_Display_Close(handle->transcodeDisplay);
    }
    handle->transcodeDisplay = NULL;
err_opendisplay:
    return rc;
}

static NEXUS_Error nexus_simpleencoder_p_set_playpump(NEXUS_PlaypumpHandle playpump, bool tts, bool pcr)
{
    NEXUS_PlaypumpSettings playpumpSettings;
    NEXUS_Playpump_GetSettings(playpump, &playpumpSettings);
    if (playpumpSettings.timestamp.forceRestamping != tts || playpumpSettings.blindSync != pcr) {
        playpumpSettings.timestamp.forceRestamping = tts;
        playpumpSettings.blindSync = pcr;
        return NEXUS_Playpump_SetSettings(playpump, &playpumpSettings);
    }
    else {
        /* no change */
        return 0;
    }
}

static void nexus_p_remove_pid_channels(NEXUS_SimpleEncoderHandle handle)
{
    NEXUS_RecpumpSettings recpumpSettings;
    unsigned i;
    NEXUS_Recpump_RemoveAllPidChannels(handle->startSettings.recpump);
#if NEXUS_HAS_SECURITY
    if (handle->pids.video.pidChannel) {
        if (handle->startSettings.output.video.keyslot) {
            NEXUS_KeySlot_RemovePidChannel(handle->startSettings.output.video.keyslot, handle->pids.video.pidChannel);
        }
    }
    if (handle->pids.audio.pidChannel) {
        if (handle->startSettings.output.audio.keyslot) {
            NEXUS_KeySlot_RemovePidChannel(handle->startSettings.output.audio.keyslot, handle->pids.audio.pidChannel);
        }
    }
    for (i=0;i<NEXUS_SIMPLE_ENCODER_NUM_PASSTHROUGH_PIDS;i++) {
        if (handle->pids.passthrough[i].pidChannel) {
            if (handle->startSettings.output.passthrough[i].keyslot) {
                NEXUS_KeySlot_RemovePidChannel(handle->startSettings.output.passthrough[i].keyslot, handle->pids.passthrough[i].pidChannel);
            }
        }
    }
#endif
    handle->pids.video.pidChannel = NULL;
    handle->pids.audio.pidChannel = NULL;
    for (i=0;i<NEXUS_SIMPLE_ENCODER_NUM_PASSTHROUGH_PIDS;i++) {
        handle->pids.passthrough[i].pidChannel = NULL;
    }
    NEXUS_Recpump_GetSettings(handle->startSettings.recpump, &recpumpSettings);
    if (recpumpSettings.pcrPidChannel) {
        recpumpSettings.pcrPidChannel = NULL;
        (void)NEXUS_Recpump_SetSettings(handle->startSettings.recpump, &recpumpSettings);
    }
    NEXUS_Playpump_CloseAllPidChannels(handle->serverSettings.playpump[2]);
    if (handle->auxpid.pid) {
        BKNI_Free(handle->auxpid.pid);
        BKNI_Memset(&handle->auxpid, 0, sizeof(handle->auxpid));
    }
}

#if NEXUS_HAS_STREAM_MUX
static NEXUS_Error nexus_simpleencoder_p_create_stream_mux(NEXUS_SimpleEncoderHandle handle)
{
    NEXUS_StreamMuxCreateSettings streamMuxCreateSettings;
    NEXUS_StreamMuxConfiguration streamMuxConfigSettings;
    int i;

    BDBG_ASSERT(!handle->streamMux);
    NEXUS_StreamMux_GetDefaultCreateSettings(&streamMuxCreateSettings);
    /* Override default stream mux mem config: */
    NEXUS_StreamMux_GetDefaultConfiguration(&streamMuxConfigSettings);
    streamMuxConfigSettings.userDataPids = 0;
    for (i=0;i<NEXUS_SIMPLE_ENCODER_NUM_PASSTHROUGH_PIDS;i++) {
        if (handle->startSettings.passthrough[i]) {
            streamMuxConfigSettings.userDataPids += 1;
        }
    }
    streamMuxConfigSettings.nonRealTime = handle->serverSettings.nonRealTime;
    NEXUS_StreamMux_GetMemoryConfiguration(&streamMuxConfigSettings, &streamMuxCreateSettings.memoryConfiguration);
    NEXUS_CallbackHandler_PrepareCallback(handle->finishedHandler, streamMuxCreateSettings.finished);
    handle->streamMux = NEXUS_StreamMux_Create(&streamMuxCreateSettings);
    if (!handle->streamMux) {
        return BERR_TRACE(NEXUS_NOT_AVAILABLE);
    }
    return NEXUS_SUCCESS;
}

static void nexus_simpleencoder_p_destroy_stream_mux(NEXUS_SimpleEncoderHandle handle)
{
    if (handle->streamMux) {
        NEXUS_StreamMux_Destroy(handle->streamMux);
        handle->streamMux = NULL;
    }
}
#else
static NEXUS_Error nexus_simpleencoder_p_create_stream_mux(NEXUS_SimpleEncoderHandle handle)
{
    BSTD_UNUSED(handle);
    return NEXUS_NOT_SUPPORTED;
}
static void nexus_simpleencoder_p_destroy_stream_mux(NEXUS_SimpleEncoderHandle handle)
{
    BSTD_UNUSED(handle);
}
#endif

static NEXUS_Error nexus_simpleencoder_p_start_stream_mux(NEXUS_SimpleEncoderHandle handle)
{
    NEXUS_Error rc;
    NEXUS_StreamMuxOutput muxOutput;
    NEXUS_PidChannelHandle pidChannel;
    bool tts;
    unsigned i;
    struct {
        NEXUS_StreamMuxStartSettings muxStartSettings;
        NEXUS_RecpumpSettings recpumpSettings;
        NEXUS_RecpumpPidChannelSettings pidSettings;
        NEXUS_RecpumpTpitFilter filter;
        NEXUS_MessageSettings messageSettings;
        NEXUS_MessageStartSettings messageStartSettings;
        NEXUS_PidChannelStatus status;
    } *data;

    data = BKNI_Malloc(sizeof(*data));
    if (!data) {
        return BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
    }

    NEXUS_Recpump_GetSettings(handle->startSettings.recpump, &data->recpumpSettings);
    tts = (data->recpumpSettings.timestampType != NEXUS_TransportTimestampType_eNone);

    NEXUS_StreamMux_GetDefaultStartSettings(&data->muxStartSettings);
    data->muxStartSettings.transportType = handle->startSettings.output.transport.type;
    data->muxStartSettings.stcChannel = nexus_simpleencoder_p_getStcChannel(handle, NEXUS_SimpleDecoderType_eVideo);
    if (!data->muxStartSettings.stcChannel) {
        data->muxStartSettings.stcChannel = nexus_simpleencoder_p_getStcChannel(handle, NEXUS_SimpleDecoderType_eAudio);
    }
    data->muxStartSettings.nonRealTime = handle->serverSettings.nonRealTime;
    data->muxStartSettings.nonRealTimeRate = handle->startSettings.transcode.nonRealTimeRate;
    data->muxStartSettings.useInitialPts = handle->startSettings.transcode.useInitialPts;
    data->muxStartSettings.initialPts = handle->startSettings.transcode.initialPts;
    if (handle->startVideo) {
        data->muxStartSettings.video[0].pid = handle->startSettings.output.video.pid;
        data->muxStartSettings.video[0].encoder = handle->serverSettings.videoEncoder;
        data->muxStartSettings.video[0].playpump = handle->serverSettings.playpump[0];
        nexus_simpleencoder_p_set_playpump(data->muxStartSettings.video[0].playpump, tts, false);
    }
    if (handle->startAudio) {
        data->muxStartSettings.audio[0].pid = handle->startSettings.output.audio.pid;
        data->muxStartSettings.audio[0].muxOutput = handle->serverSettings.audioMuxOutput;
        data->muxStartSettings.audio[0].playpump = handle->serverSettings.playpump[1];
        nexus_simpleencoder_p_set_playpump(data->muxStartSettings.audio[0].playpump, tts, false);
    }

    /* pcr.playpump is also used for PSI, so add unconditionally. */
    data->muxStartSettings.pcr.playpump = handle->serverSettings.playpump[2];
    nexus_simpleencoder_p_set_playpump(data->muxStartSettings.pcr.playpump, tts, true);
    if (handle->startSettings.output.transport.pcrPid) {
        data->muxStartSettings.pcr.pid = handle->startSettings.output.transport.pcrPid;
        data->muxStartSettings.pcr.interval = handle->startSettings.output.transport.pcrInterval;
    }
    else {
        data->muxStartSettings.pcr.interval = 0;
    }
    for (i=0;i<NEXUS_SIMPLE_ENCODER_NUM_PASSTHROUGH_PIDS;i++) {
        if (handle->startSettings.passthrough[i]) {

            NEXUS_Message_GetDefaultSettings(&data->messageSettings);
            data->messageSettings.bufferSize = 512*1024;
            data->messageSettings.maxContiguousMessageSize = 0; /* to support TS capture and in-place operation */
            data->muxStartSettings.userdata[i].message = handle->message[i] = NEXUS_Message_Open(&data->messageSettings);
            if (!handle->message[i]) {rc = BERR_TRACE(NEXUS_NOT_AVAILABLE); goto err_message;}

            NEXUS_Message_GetDefaultStartSettings(handle->message[i], &data->messageStartSettings);
            data->messageStartSettings.format = NEXUS_MessageFormat_eTs;
            data->messageStartSettings.pidChannel = handle->startSettings.passthrough[i];
            rc = NEXUS_Message_Start(handle->message[i], &data->messageStartSettings);
            if (rc) {rc = BERR_TRACE(rc); goto err_message;}
        }
    }
    /* session display encode needs low latency */
    if(handle->startSettings.input.display) {
        data->muxStartSettings.servicePeriod    = 10;/* ms */
        data->muxStartSettings.latencyTolerance = 20;/* ms */
    }
    rc = NEXUS_StreamMux_Start(handle->streamMux, &data->muxStartSettings, &muxOutput);
    if (rc) {rc = BERR_TRACE(rc); goto err_startmux;}


    if (handle->startSettings.output.transport.pcrPid) {
        pidChannel = NEXUS_Playpump_OpenPidChannel(handle->serverSettings.playpump[2], handle->startSettings.output.transport.pcrPid, NULL);
        if (!pidChannel) {rc = BERR_TRACE(NEXUS_UNKNOWN); goto err_openpid;}
        rc = NEXUS_Recpump_AddPidChannel(handle->startSettings.recpump, pidChannel, NULL);
        if (rc) {rc = BERR_TRACE(rc); goto err_openpid;}

        /* TTS format may adjust timestamp by pcr; however, client doesn't have access to pcr pid channel, so we set it here. */
        if (tts && data->recpumpSettings.adjustTimestampUsingPcrs) {
            data->recpumpSettings.pcrPidChannel = pidChannel;
            rc = NEXUS_Recpump_SetSettings(handle->startSettings.recpump, &data->recpumpSettings);
            if (rc) {rc = BERR_TRACE(rc); goto err_openpid;}
        }
    }

    if (handle->startSettings.output.transport.pmtPid) {
        NEXUS_SimpleEncoder_P_AddAuxPid(handle, handle->startSettings.output.transport.pmtPid);
        NEXUS_SimpleEncoder_P_AddAuxPid(handle, 0); /* PAT */
    }
    for (i=0;i<NEXUS_SIMPLE_ENCODER_NUM_PASSTHROUGH_PIDS;i++) {
        if (handle->startSettings.passthrough[i]) {
            NEXUS_PidChannel_GetStatus(handle->startSettings.passthrough[i], &data->status);
            pidChannel = NEXUS_Playpump_OpenPidChannel(handle->serverSettings.playpump[2], data->status.remappedPid, NULL);
            if (!pidChannel) {rc = BERR_TRACE(NEXUS_UNKNOWN); goto err_openpid;}
            rc = NEXUS_Recpump_AddPidChannel(handle->startSettings.recpump, pidChannel, NULL);
            if (rc) {rc = BERR_TRACE(rc); goto err_openpid;}

            handle->pids.passthrough[i].pidChannel = pidChannel;
#if NEXUS_HAS_SECURITY
            if (handle->startSettings.output.passthrough[i].keyslot) {
                rc = NEXUS_KeySlot_AddPidChannel(handle->startSettings.output.passthrough[i].keyslot, handle->pids.passthrough[i].pidChannel);
                if (rc) {rc = BERR_TRACE(rc); goto err_addpid;}
            }
#endif
        }
    }

    if (muxOutput.video[0]) {
        NEXUS_Recpump_GetDefaultAddPidChannelSettings(&data->pidSettings);
        if (handle->startSettings.output.video.index) {
            data->pidSettings.pidType = NEXUS_PidType_eVideo;
            data->pidSettings.pidTypeSettings.video.index = true;
            data->pidSettings.pidTypeSettings.video.codec = handle->startSettings.output.video.settings.codec;
        }
        rc = NEXUS_Recpump_AddPidChannel(handle->startSettings.recpump, muxOutput.video[0], &data->pidSettings);
        if (rc) {rc = BERR_TRACE(rc); goto err_addpid;}

        handle->pids.video.pidChannel = muxOutput.video[0];
#if NEXUS_HAS_SECURITY
        if (handle->startSettings.output.video.keyslot) {
            rc = NEXUS_KeySlot_AddPidChannel(handle->startSettings.output.video.keyslot, handle->pids.video.pidChannel);
            if (rc) {rc = BERR_TRACE(rc); goto err_addpid;}
        }
#endif

        /* may want user option for TPIT RAI indexing. required for HLS now. */
        if (handle->startSettings.output.video.raiIndex) {
            NEXUS_Recpump_GetDefaultTpitFilter(&data->filter);
            data->filter.config.mpeg.randomAccessIndicatorEnable = true;
            data->filter.config.mpeg.randomAccessIndicatorCompValue = true;
            rc = NEXUS_Recpump_SetTpitFilter(handle->startSettings.recpump, muxOutput.video[0], &data->filter);
            if (rc) {rc = BERR_TRACE(rc); goto err_addpid;}
        }
    }
    if (muxOutput.audio[0]) {
        rc = NEXUS_Recpump_AddPidChannel(handle->startSettings.recpump, muxOutput.audio[0], NULL);
        if (rc) {rc = BERR_TRACE(rc); goto err_addpid;}
        handle->pids.audio.pidChannel = muxOutput.audio[0];
#if NEXUS_HAS_SECURITY
        if (handle->startSettings.output.audio.keyslot) {
            rc = NEXUS_KeySlot_AddPidChannel(handle->startSettings.output.audio.keyslot, handle->pids.audio.pidChannel);
            if (rc) {rc = BERR_TRACE(rc); goto err_addpid;}
        }
#endif
    }

    BKNI_Free(data);
    return NEXUS_SUCCESS;

err_addpid:
err_openpid:
    nexus_p_remove_pid_channels(handle);
    NEXUS_StreamMux_Stop(handle->streamMux);
err_startmux:
err_message:
    for (i=0;i<NEXUS_SIMPLE_ENCODER_NUM_PASSTHROUGH_PIDS;i++) {
        if (handle->message[i]) {
            NEXUS_Message_Close(handle->message[i]);
            handle->message[i] = NULL;
        }
    }
    BKNI_Free(data);
    return rc;
}

/* For real-time transcode, the encoder has its own StcChannel while video/audio decoders share.
For non-real-time transcode, video and audio need separate StcChannels and the encoder can share with video or audio. */
static NEXUS_StcChannelHandle nexus_simpleencoder_p_getStcChannel( NEXUS_SimpleEncoderHandle handle, NEXUS_SimpleDecoderType type )
{
    if (handle->serverSettings.nonRealTime) {
        NEXUS_SimpleStcChannelHandle simpleStcChannel = NULL;
        if (type == NEXUS_SimpleDecoderType_eVideo && handle->startSettings.input.video) {
            simpleStcChannel = NEXUS_SimpleVideoDecoder_P_GetStcChannel(handle->startSettings.input.video);
        }
        else if (type == NEXUS_SimpleDecoderType_eAudio && handle->startSettings.input.audio) {
            simpleStcChannel = NEXUS_SimpleAudioDecoder_P_GetStcChannel(handle->startSettings.input.audio);
        }
        if (simpleStcChannel) {
            return NEXUS_SimpleStcChannel_GetServerStcChannel_priv(simpleStcChannel, type);
        }
        else {
            return NULL;
        }
    }
    else {
        return handle->serverSettings.stcChannelTranscode;
    }
}

NEXUS_Error nexus_simpleencoder_p_start_audiomux( NEXUS_SimpleEncoderHandle handle )
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    if (handle->programChange.inProgress) {
        NEXUS_VideoEncoderStatus status;
        unsigned refreshRate;
        handle->programChange.inProgress = false;

        NEXUS_AudioMuxOutput_GetDelayStatus(handle->serverSettings.audioMuxOutput, handle->startSettings.output.audio.codec, &handle->stack.audioDelayStatus);
        NEXUS_AudioMuxOutput_GetDefaultStartSettings(&handle->stack.audioMuxStartSettings);
        handle->stack.audioMuxStartSettings.stcChannel = nexus_simpleencoder_p_getStcChannel(handle, NEXUS_SimpleDecoderType_eAudio);
        handle->stack.audioMuxStartSettings.presentationDelay =
            (handle->settings.videoEncoder.encoderDelay > handle->stack.audioDelayStatus.endToEndDelay * 27000)?
            handle->settings.videoEncoder.encoderDelay/27000 : handle->stack.audioDelayStatus.endToEndDelay;
        handle->stack.audioMuxStartSettings.nonRealTime = nexus_simpleencoder_p_nonRealTime(handle);

        refreshRate = handle->settings.video.refreshRate;
        if (handle->transcodeDisplay) {
            NEXUS_DisplayStatus displayStatus;
            rc = NEXUS_Display_GetStatus(handle->transcodeDisplay, &displayStatus);
            if (!rc) {
                refreshRate = displayStatus.refreshRate;
            }
        }
        rc = NEXUS_VideoEncoder_GetStatus(handle->serverSettings.videoEncoder, &status);
        if (!rc && refreshRate) {
            /* convert encoded pictures to STC based on display refresh rate.
            refreshRate is units of 1/1000 Hz. initialStc is units of 45KHz.
            For examples, 60 pics at 60.000 Hz should be 45000.
            Use uint64_t to avoid overflow.
            */
            handle->stack.audioMuxStartSettings.initialStc = (uint64_t)(status.picturesReceived+1) * 45000 / refreshRate * 1000;
            BDBG_WRN(("Starting Audio Mux Output with initialStc = %#x @45Khz, presentationDelay=%u(ms)",
                handle->stack.audioMuxStartSettings.initialStc,handle->stack.audioMuxStartSettings.presentationDelay));
        }
        rc = NEXUS_AudioMuxOutput_Start(handle->serverSettings.audioMuxOutput, &handle->stack.audioMuxStartSettings);
        if (rc) {rc = BERR_TRACE(rc); goto err_startaudio;}

        if (handle->serverSettings.mixer) {
            rc = NEXUS_AudioMixer_Start(handle->serverSettings.mixer);
            if (rc) {rc = BERR_TRACE(rc); goto err_startaudio;}
        }
    }
err_startaudio:
    return rc;
}

static NEXUS_Error nexus_simpleencoder_p_start_videoencoder( NEXUS_SimpleEncoderHandle handle, bool startAudio )
{
    int rc;

    handle->stack.videoEncoderStartSettings = handle->startSettings.output.video.settings;
#ifdef NEXUS_NUM_DSP_VIDEO_ENCODERS
    handle->stack.videoEncoderStartSettings.bounds.inputDimension.max.width = handle->settings.video.width;
    handle->stack.videoEncoderStartSettings.bounds.inputDimension.max.height = handle->settings.video.height;
#endif
    handle->stack.videoEncoderStartSettings.stcChannel = nexus_simpleencoder_p_getStcChannel(handle, NEXUS_SimpleDecoderType_eVideo);

    if (handle->startSettings.input.display) {
        handle->stack.videoEncoderStartSettings.input = handle->serverSettings.displayEncode.display;
        /* session display encode needs low latency override:
               1) rateBufferDelay - 0 defaults to large delay; overrides it with low latency default;
               2) fieldPairing - increases encoder pre-process delay; turn it off for session encode;
               3) adaptiveLowDelayMode - to reduce encode start latency;
               */
        if(handle->stack.videoEncoderStartSettings.rateBufferDelay == 0) {
            handle->stack.videoEncoderStartSettings.rateBufferDelay        = 50;/* ms */
        }
        /* if decoder is not stopped, encoder stop/start with this mode ON might drop all frames;
           so leave it to app to control.*/
        /*videoEncoderStartSettings.adaptiveLowDelayMode = true;*/
        handle->settings.videoEncoder.enableFieldPairing = false;
    }
    else if (handle->startSettings.input.video) {
        handle->stack.videoEncoderStartSettings.input = handle->transcodeDisplay;
    }

    /* NOTE: video encoder delay is in 27MHz ticks */
    rc = NEXUS_VideoEncoder_GetDelayRange(handle->serverSettings.videoEncoder, &handle->settings.videoEncoder, &handle->stack.videoEncoderStartSettings, &handle->stack.videoDelay);
    if (rc) {
        BERR_TRACE(rc);
        handle->stack.videoDelay.min = handle->stack.videoDelay.max = 0;
    }
    BDBG_WRN(("Video encoder end-to-end delay = [%u ~ %u] ms", handle->stack.videoDelay.min/27000, handle->stack.videoDelay.max/27000));

    if (startAudio) {
        unsigned Dee;

        rc = NEXUS_AudioMuxOutput_GetDelayStatus(handle->serverSettings.audioMuxOutput, handle->startSettings.output.audio.codec, &handle->stack.audioDelayStatus);
        BDBG_WRN(("Audio codec %d end-to-end delay = %u ms", handle->startSettings.output.audio.codec, handle->stack.audioDelayStatus.endToEndDelay));

        if (rc) {
            Dee = handle->stack.videoDelay.max;
        }
        else {
            Dee = handle->stack.audioDelayStatus.endToEndDelay * 27000; /* in 27MHz ticks */
        }
        if(Dee > handle->stack.videoDelay.min)
        {
            if(Dee > handle->stack.videoDelay.max)
            {
                BDBG_ERR(("Audio Dee is way too big! Use video Dee max!"));
                Dee = handle->stack.videoDelay.max;
            }
            else
            {
                BDBG_WRN(("Use audio Dee %u ms %u ticks@27Mhz!", Dee/27000, Dee));
            }
        }
        else
        {
            Dee = handle->stack.videoDelay.min;
            BDBG_WRN(("Use video Dee %u ms %u ticks@27Mhz!", Dee/27000, Dee));
        }
        handle->settings.videoEncoder.encoderDelay = Dee;

        /* Start audio mux output */
        if(!handle->started) {
            NEXUS_AudioMuxOutput_GetDefaultStartSettings(&handle->stack.audioMuxStartSettings);
            handle->stack.audioMuxStartSettings.stcChannel = nexus_simpleencoder_p_getStcChannel(handle, NEXUS_SimpleDecoderType_eAudio);
            handle->stack.audioMuxStartSettings.presentationDelay = Dee/27000;/* in ms */
            handle->stack.audioMuxStartSettings.nonRealTime = nexus_simpleencoder_p_nonRealTime(handle);
            rc = NEXUS_AudioMuxOutput_Start(handle->serverSettings.audioMuxOutput, &handle->stack.audioMuxStartSettings);
            if (rc) return BERR_TRACE(rc);
        }
    }
    else {
        handle->settings.videoEncoder.encoderDelay = handle->stack.videoDelay.min;
    }

    /* reapply settings with new encoderDelay */
    rc = NEXUS_SimpleEncoder_SetSettings(handle, &handle->settings);
    if (rc) return BERR_TRACE(rc);

    handle->stack.videoEncoderStartSettings.nonRealTime = nexus_simpleencoder_p_nonRealTime(handle);
    rc = NEXUS_VideoEncoder_Start(handle->serverSettings.videoEncoder, &handle->stack.videoEncoderStartSettings);
    if (rc) return BERR_TRACE(rc);
    handle->videoEncoderStarted = true;/* video encoder actually started */

    return NEXUS_SUCCESS;
}

/**
actual start happens after decoders are started
**/
static NEXUS_Error nexus_simpleencoder_p_start( NEXUS_SimpleEncoderHandle handle )
{
    NEXUS_Error rc;

    BDBG_MSG(("nexus_simpleencoder_p_start %p", (void *)handle));

    if (handle->videoEncoderStarted) {
        return BERR_TRACE(NEXUS_NOT_AVAILABLE);
    }

    if (handle->startVideo) {
        rc = nexus_simpleencoder_p_start_videoencoder(handle, handle->startAudio);
        if (rc) {BERR_TRACE(rc); goto err_startvideo;}
    }
    else if(!handle->started) {
        /* Start audio mux output */
        NEXUS_AudioMuxOutput_GetDelayStatus(handle->serverSettings.audioMuxOutput, handle->startSettings.output.audio.codec, &handle->stack.audioDelayStatus);
        NEXUS_AudioMuxOutput_GetDefaultStartSettings(&handle->stack.audioMuxStartSettings);
        handle->stack.audioMuxStartSettings.stcChannel = nexus_simpleencoder_p_getStcChannel(handle, NEXUS_SimpleDecoderType_eAudio);
        handle->stack.audioMuxStartSettings.presentationDelay = handle->stack.audioDelayStatus.endToEndDelay;
        handle->stack.audioMuxStartSettings.nonRealTime = nexus_simpleencoder_p_nonRealTime(handle);
        rc = NEXUS_AudioMuxOutput_Start(handle->serverSettings.audioMuxOutput, &handle->stack.audioMuxStartSettings);
        if (rc) {rc = BERR_TRACE(rc); goto err_startaudio;}
    }

    if (handle->serverSettings.mixer && !handle->started) {
        /* must be started after NEXUS_AudioMuxOutput_Start */
        rc = NEXUS_AudioMixer_Start(handle->serverSettings.mixer);
        if (rc) {rc = BERR_TRACE(rc); goto err_mixer_start;}
    }

    if ( handle->startSettings.output.transport.type == NEXUS_TransportType_eTs ) {
        rc = nexus_simpleencoder_p_create_stream_mux(handle);
        if (rc) {rc = BERR_TRACE(rc); goto err_create_stream_mux;}

        rc = nexus_simpleencoder_p_start_psi(handle);
        if (rc) {rc = BERR_TRACE(rc); goto err_start_psi;}

        rc = nexus_simpleencoder_p_start_stream_mux(handle);
        if (rc) {rc = BERR_TRACE(rc); goto err_stream_mux;}
    }

    handle->started = true;
    return 0;

err_stream_mux:
    nexus_simpleencoder_p_stop_psi(handle);
err_start_psi:
    nexus_simpleencoder_p_destroy_stream_mux(handle);
err_create_stream_mux:
err_mixer_start:
err_startvideo:
err_startaudio:
    /* NEXUS_SimpleEncoder_Stop must be called by the caller */
    BDBG_ASSERT(rc);
    return rc;
}

void nexus_simpleencoder_p_stop_videoencoder(NEXUS_SimpleEncoderHandle handle, bool abort)
{
    if (handle->videoEncoderStarted && !handle->settings.programChange) {
        NEXUS_VideoEncoderStopSettings settings;
        NEXUS_VideoEncoder_GetDefaultStopSettings(&settings);
        settings.mode = abort?NEXUS_VideoEncoderStopMode_eAbort:NEXUS_VideoEncoderStopMode_eImmediate;
        NEXUS_VideoEncoder_Stop(handle->serverSettings.videoEncoder, &settings);
        handle->videoEncoderStarted = false;
    }
}

void nexus_simpleencoder_p_stop( NEXUS_SimpleEncoderHandle handle )
{
    BDBG_MSG(("nexus_simpleencoder_p_stop %p: %d %d %p %p mode: %u", (void *)handle, handle->started, handle->startAudio, (void *)handle->startSettings.input.video, (void *)handle->transcodeWindow, handle->settings.stopMode));
    if (handle->settings.stopMode != NEXUS_SimpleEncoderStopMode_eVideoEncoderOnly && handle->started) {
        unsigned i;
        if ( handle->startSettings.output.transport.type == NEXUS_TransportType_eTs ) {
            nexus_p_remove_pid_channels(handle);
            NEXUS_StreamMux_Stop(handle->streamMux);
            nexus_simpleencoder_p_stop_psi(handle);
            nexus_simpleencoder_p_destroy_stream_mux(handle);
            /* TS layer user data passthrough is a stream mux feature, so to close in case TS type. */
            for (i=0;i<NEXUS_SIMPLE_ENCODER_NUM_PASSTHROUGH_PIDS;i++) {
                if (handle->message[i]) {
                    NEXUS_Message_Close(handle->message[i]);
                    handle->message[i] = NULL;
                }
            }
        }
        /* stop mode eVideoEncoderOnly needs to keep audio mixer/mux output continuous in RT mode */
        if (handle->serverSettings.mixer) {
            NEXUS_AudioMixer_Stop(handle->serverSettings.mixer);
        }
        if (handle->startAudio) {
            NEXUS_AudioMuxOutput_Stop(handle->serverSettings.audioMuxOutput);
            if (handle->startSettings.input.display) {
                nexus_simpleaudiodecoder_p_remove_encoder(handle->serverSettings.displayEncode.masterAudio);
            }
            else {
                nexus_simpleaudiodecoder_p_remove_encoder(handle->startSettings.input.audio);
            }
            handle->audioEncoderAcquired = false;
        }
        handle->clientStarted = false;/* fully stopped */
        handle->started = false; /* fully stopped */
    }

    nexus_simpleencoder_p_stop_videoencoder(handle,
        (handle->startSettings.output.transport.type != NEXUS_TransportType_eMp4 &&
         handle->settings.stopMode != NEXUS_SimpleEncoderStopMode_eVideoEncoderOnly) || handle->abort);

    if (handle->settings.stopMode != NEXUS_SimpleEncoderStopMode_eVideoEncoderOnly) {
        if (handle->transcodeWindow) {
            if (handle->startSettings.input.video) {
                nexus_simplevideodecoder_p_remove_encoder(handle->startSettings.input.video, handle->transcodeWindow, handle);
            }
            NEXUS_VideoWindow_Close(handle->transcodeWindow);
            handle->transcodeWindow = NULL;
        }
        if (handle->transcodeDisplay && !handle->startSettings.transcode.display) {
            NEXUS_Display_Close(handle->transcodeDisplay);
        }
        handle->transcodeDisplay = NULL;
        handle->wait.video = false;
        handle->wait.audio = false;
    }

    handle->programChange.inProgress = false;
}

/* TSHDRBUILDER has one extra byte at the beginning to describe the variable length TS header buffer */
#define BTST_TS_HEADER_BUF_LENGTH 189
#define NEXUS_SIMPLE_ENCODER_PSI_QUEUE_CNT 32 /* TODO: use GetComplete to recycle the queue */
#define PAT_PMT_PAIR_SIZE (BTST_TS_HEADER_BUF_LENGTH*2)

static void nexus_simpleencoder_p_psi_timer( void *context )
{
    NEXUS_StreamMuxSystemData psi[2];
    NEXUS_SimpleEncoderHandle handle = context;
    uint8_t *pat, *pmt;

    handle->psi.timer = NULL;

    pat = handle->psi.buffer;
    pat += (handle->psi.ccValue % NEXUS_SIMPLE_ENCODER_PSI_QUEUE_CNT) * PAT_PMT_PAIR_SIZE;
    pmt = pat+BTST_TS_HEADER_BUF_LENGTH;

    BKNI_Memset(psi, 0, sizeof(psi));
    psi[0].size = 188;
    psi[0].pData = pat+1;
    psi[0].timestampDelta = 0;
    psi[1].size = 188;
    psi[1].pData = pmt+1;
    psi[1].timestampDelta = 0;

    pat[4] = (pat[4] & 0xf0) | (handle->psi.ccValue & 0xf);
    NEXUS_Memory_FlushCache(&pat[4], 1);

    pmt[4] = (pmt[4] & 0xf0) | (handle->psi.ccValue & 0xf);
    NEXUS_Memory_FlushCache(&pmt[4], 1);

    NEXUS_StreamMux_AddSystemDataBuffer(handle->streamMux, &psi[0]);
    NEXUS_StreamMux_AddSystemDataBuffer(handle->streamMux, &psi[1]);
    BDBG_MSG(("%p: insert PSI ccPAT=%x ccPMT=%x", (void *)handle, pat[4]&0xf, pmt[4]&0xf));

    ++handle->psi.ccValue;
    handle->psi.timer = NEXUS_ScheduleTimer(handle->startSettings.output.transport.interval, nexus_simpleencoder_p_psi_timer, handle);
}

static NEXUS_Error nexus_simpleencoder_p_start_psi( NEXUS_SimpleEncoderHandle handle )
{
    uint8_t pat_pl_buf[BTST_TS_HEADER_BUF_LENGTH], pmt_pl_buf[BTST_TS_HEADER_BUF_LENGTH];
    size_t pat_pl_size, pmt_pl_size;
    unsigned streamNum;
    TS_PAT_state patState;
    TS_PSI_header psi_header;
    TS_PMT_state pmtState;
    TS_PAT_program program;
    TS_PMT_stream pmt_stream;
    uint8_t *pat, *pmt;
    int rc;
    unsigned i;

    /* if no pmtPid, then no PSI */
    if (!handle->startSettings.output.transport.pmtPid) {
        return 0;
    }

    /* == CREATE PSI TABLES == */
    TS_PSI_header_Init(&psi_header);
    TS_PAT_Init(&patState, &psi_header, pat_pl_buf, BTST_TS_HEADER_BUF_LENGTH);

    TS_PAT_program_Init(&program, 1, handle->startSettings.output.transport.pmtPid);
    TS_PAT_addProgram(&patState, &pmtState, &program, pmt_pl_buf, BTST_TS_HEADER_BUF_LENGTH);

    if (handle->startSettings.output.transport.pcrPid) {
        TS_PMT_setPcrPid(&pmtState, handle->startSettings.output.transport.pcrPid);
    }

    if (handle->startVideo) {
        unsigned vidStreamType;
        switch(handle->startSettings.output.video.settings.codec) {
        case NEXUS_VideoCodec_eMpeg2:         vidStreamType = 0x2; break;
        case NEXUS_VideoCodec_eMpeg4Part2:    vidStreamType = 0x10; break;
        case NEXUS_VideoCodec_eH264:          vidStreamType = 0x1b; break;
        case NEXUS_VideoCodec_eH265:          vidStreamType = 0x24; break;
        case NEXUS_VideoCodec_eVc1SimpleMain: vidStreamType = 0xea; break;
        default:
            BDBG_ERR(("Video encoder codec %d is not supported!", handle->startSettings.output.video.settings.codec));
            return BERR_TRACE(NEXUS_NOT_SUPPORTED);
        }
        TS_PMT_stream_Init(&pmt_stream, vidStreamType, handle->startSettings.output.video.pid);
        TS_PMT_addStream(&pmtState, &pmt_stream, &streamNum);
    }

    if (handle->startAudio) {
        unsigned audStreamType;
        switch(handle->startSettings.output.audio.codec) {
        case NEXUS_AudioCodec_eMpeg:         audStreamType = 0x4; break;
        case NEXUS_AudioCodec_eMp3:          audStreamType = 0x4; break;
        case NEXUS_AudioCodec_eAacAdts:      audStreamType = 0xf; break; /* ADTS */
        case NEXUS_AudioCodec_eAacPlusAdts:  audStreamType = 0xf; break; /* ADTS */
        case NEXUS_AudioCodec_eAacLoas:      audStreamType = 0x11; break;/* LOAS */
        case NEXUS_AudioCodec_eAacPlusLoas:  audStreamType = 0x11; break;/* LOAS */
        case NEXUS_AudioCodec_eAc3:          audStreamType = 0x81; break;
        case NEXUS_AudioCodec_eLpcm1394:     audStreamType = 0x83; break;
        default:
            BDBG_ERR(("Audio encoder codec %d is not supported!", handle->startSettings.output.audio.codec));
            return BERR_TRACE(NEXUS_NOT_SUPPORTED);
        }

        TS_PMT_stream_Init(&pmt_stream, audStreamType, handle->startSettings.output.audio.pid);
        TS_PMT_addStream(&pmtState, &pmt_stream, &streamNum);
    }

    TS_PAT_finalize(&patState, &pat_pl_size);
    TS_PMT_finalize(&pmtState, &pmt_pl_size);

#if 0
    BDBG_MSG(("%p: PMT section:", handle));
    for(i=0; i<pmtState.size; i+=8) {
        BDBG_MSG(("%02x %02x %02x %02x %02x %02x %02x %02x", pmtState.buf[i], pmtState.buf[i+1], pmtState.buf[i+2], pmtState.buf[i+3],
            pmtState.buf[i+4], pmtState.buf[i+5], pmtState.buf[i+6], pmtState.buf[i+7]));
    }
#endif

#define PSI_BUFFER_SIZE (PAT_PMT_PAIR_SIZE*NEXUS_SIMPLE_ENCODER_PSI_QUEUE_CNT)

    handle->psi.block = NEXUS_MemoryBlock_Allocate(g_pCoreHandles->heap[g_pCoreHandles->defaultHeapIndex].nexus, PSI_BUFFER_SIZE, 0, NULL);
    if (!handle->psi.block) return BERR_TRACE(NEXUS_OUT_OF_DEVICE_MEMORY);
    rc = NEXUS_MemoryBlock_Lock(handle->psi.block, &handle->psi.buffer);
    if (rc) return BERR_TRACE(rc);

    pat = handle->psi.buffer;
    pmt = pat+BTST_TS_HEADER_BUF_LENGTH;

    /* == CREATE TS HEADERS FOR PSI INFORMATION == */
    /* Some Miracast client cannot handle AF in PSI transport packet */
    /* 1-byte offset at beggining for BTST_TS_HEADER_BUF_LENGTH */
    pat[1] = 0x47;
    pat[2] = 0x40; /* TEI = 0, Payload Unit Start = 1, Transport Priority = 0, 13 bit-pid# = 0 */
    pat[3] = 0x00;
    pat[4] = 0x10; /* scrambling = 0, adaptation field = 0, continuity counter = 0 */
    pat[5] = 0x00; /* pointer = 0 */
    BDBG_ASSERT(pat_pl_size <= BTST_TS_HEADER_BUF_LENGTH - 6); /* one packet PAT */
    BKNI_Memset(pat + 6, 0xff, BTST_TS_HEADER_BUF_LENGTH - 6); /* stuffing bytes */
    BKNI_Memcpy(pat + 6, pat_pl_buf, pat_pl_size); /* PAT table followed by stuffing bytes */

    /* Some Miracast client cannot handle AF in PSI transport packet */
    /* 1-byte offset at beggining for BTST_TS_HEADER_BUF_LENGTH */
    pmt[1] = 0x47;
    pmt[2] = 0x40 | ((handle->startSettings.output.transport.pmtPid >> 8) & 0x1f); /* TEI = 0, PUSI= 1, TP=0, 13-bit pid# */
    pmt[3] = handle->startSettings.output.transport.pmtPid & 0xff;
    pmt[4] = 0x10; /* scrambling = 0, AF = 0, continuity counter = 0 */
    pmt[5] = 0x00; /* pointer = 0 */
    BDBG_ASSERT(pmt_pl_size <= BTST_TS_HEADER_BUF_LENGTH - 6); /* one packet PMT */
    BKNI_Memset(pmt + 6, 0xff, BTST_TS_HEADER_BUF_LENGTH - 6); /* stuffing bytes */
    BKNI_Memcpy(pmt + 6, pmt_pl_buf, pmt_pl_size); /* PMT table followed by stuffing bytes */

#if 0
    BDBG_MSG(("%p: PMT packet:", handle));
    for(i=0; i < BTST_TS_HEADER_BUF_LENGTH; i+=8) {
        BDBG_MSG(("%02x %02x %02x %02x %02x %02x %02x %02x", *(pmt+i), *(pmt+i+1), *(pmt+i+2), *(pmt+i+3), *(pmt+i+4), *(pmt+i+5), *(pmt+i+6), *(pmt+i+7)));
    }
#endif

    /* copy from first to others */
    for (i=1;i<NEXUS_SIMPLE_ENCODER_PSI_QUEUE_CNT;i++) {
        BKNI_Memcpy((uint8_t*)handle->psi.buffer + PAT_PMT_PAIR_SIZE*i, handle->psi.buffer, PAT_PMT_PAIR_SIZE);
    }

    NEXUS_Memory_FlushCache(handle->psi.buffer, PSI_BUFFER_SIZE);

    handle->psi.ccValue = 0;
    nexus_simpleencoder_p_psi_timer(handle);

    return 0;
}

static void nexus_simpleencoder_p_stop_psi( NEXUS_SimpleEncoderHandle handle )
{
    if (handle->psi.timer) {
        NEXUS_CancelTimer(handle->psi.timer);
        handle->psi.timer = NULL;
    }
    if (handle->psi.block) {
        NEXUS_MemoryBlock_Free(handle->psi.block);
        handle->psi.block = NULL;
        handle->psi.buffer = NULL;
    }
}
#else
NEXUS_Error nexus_simpleencoder_p_start_audiomux( NEXUS_SimpleEncoderHandle handle )
{
    BSTD_UNUSED(handle);
    return NEXUS_SUCCESS;
}
static NEXUS_Error nexus_simpleencoder_p_start( NEXUS_SimpleEncoderHandle handle )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_SimpleEncoder);
    return BERR_TRACE(NEXUS_NOT_SUPPORTED);
}

void nexus_simpleencoder_p_stop( NEXUS_SimpleEncoderHandle handle )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_SimpleEncoder);
}
void nexus_simpleencoder_p_stop_videoencoder(NEXUS_SimpleEncoderHandle handle, bool abort)
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(abort);
}
#endif

/**
Summary:
Get a video encoder buffer if outputting to memory
**/
NEXUS_Error NEXUS_SimpleEncoder_GetVideoBuffer(
    NEXUS_SimpleEncoderHandle handle,
    const NEXUS_VideoEncoderDescriptor **pBuffer, /* [out] attr{memory=cached} pointer to NEXUS_VideoEncoderDescriptor structs */
    size_t *pSize, /* [out] number of NEXUS_VideoEncoderDescriptor elements in pBuffer */
    const NEXUS_VideoEncoderDescriptor **pBuffer2, /* [out] attr{memory=cached} pointer to NEXUS_VideoEncoderDescriptor structs after wrap around */
    size_t *pSize2 /* [out] number of NEXUS_VideoEncoderDescriptor elements in pBuffer2 */
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_SimpleEncoder);
#if NEXUS_HAS_STREAM_MUX
    if ( handle->serverSettings.videoEncoder)
    {
        return NEXUS_VideoEncoder_GetBuffer(handle->serverSettings.videoEncoder, pBuffer, pSize, pBuffer2, pSize2);
    }
    else
    {
        return BERR_NOT_SUPPORTED;
    }
#else
    BSTD_UNUSED(pBuffer);
    BSTD_UNUSED(pBuffer2);
    BSTD_UNUSED(pSize);
    BSTD_UNUSED(pSize2);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
#endif
}

/**
Summary:
Return a video encoder buffer if outputting to memory
**/
NEXUS_Error NEXUS_SimpleEncoder_VideoReadComplete(
    NEXUS_SimpleEncoderHandle handle,
    unsigned descriptorsCompleted /* must be <= pSize+pSize2 returned by last NEXUS_SimpleEncoder_GetVideoBuffer call. */
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_SimpleEncoder);
#if NEXUS_HAS_STREAM_MUX
    if ( handle->serverSettings.videoEncoder)
    {
        return NEXUS_VideoEncoder_ReadComplete(handle->serverSettings.videoEncoder, descriptorsCompleted);
    }
    else
    {
        return BERR_NOT_SUPPORTED;
    }
#else
    BSTD_UNUSED(descriptorsCompleted);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
#endif
}

/**
Summary:
Get video encoder buffer descriptor metadata
**/
NEXUS_Error NEXUS_SimpleEncoder_ReadVideoIndex(
    NEXUS_SimpleEncoderHandle handle,
    NEXUS_VideoEncoderDescriptor *pBuffer, /* attr{nelem=size;nelem_out=pRead} [out] pointer to NEXUS_VideoEncoderDescriptor structs */
    unsigned size, /* max number of NEXUS_VideoEncoderPicture elements in pBuffer */
    unsigned *pRead /* [out] number of NEXUS_VideoEncoderPicture elements read */
    )
{
   BDBG_OBJECT_ASSERT(handle, NEXUS_SimpleEncoder);
#if NEXUS_HAS_STREAM_MUX
   if ( handle->serverSettings.videoEncoder)
   {
       return NEXUS_VideoEncoder_ReadIndex(handle->serverSettings.videoEncoder, pBuffer, size, pRead);
   }
   else
   {
       return BERR_NOT_SUPPORTED;
   }
#else
   BSTD_UNUSED(pBuffer);
   BSTD_UNUSED(size);
   BSTD_UNUSED(pRead);
   return BERR_TRACE(BERR_NOT_SUPPORTED);
#endif
}

/**
Summary:
Get an audio encoder buffer if outputting to memory
**/
NEXUS_Error NEXUS_SimpleEncoder_GetAudioBuffer(
    NEXUS_SimpleEncoderHandle handle,
    const NEXUS_AudioMuxOutputFrame **pBuffer, /* [out] attr{memory=cached} pointer to NEXUS_AudioMuxOutputFrame structs */
    size_t *pSize, /* [out] number of NEXUS_AudioMuxOutputFrame elements in pBuffer */
    const NEXUS_AudioMuxOutputFrame **pBuffer2, /* [out] attr{memory=cached} pointer to NEXUS_AudioMuxOutputFrame structs after wrap around */
    size_t *pSize2 /* [out] number of NEXUS_AudioMuxOutputFrame elements in pBuffer2 */
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_SimpleEncoder);
#if NEXUS_HAS_AUDIO
    if ( handle->serverSettings.audioMuxOutput)
    {
        return NEXUS_AudioMuxOutput_GetBuffer(handle->serverSettings.audioMuxOutput, pBuffer, pSize, pBuffer2, pSize2);
    }
    else
    {
        return BERR_NOT_SUPPORTED;
    }
#else
    BSTD_UNUSED(pBuffer);
    BSTD_UNUSED(pBuffer2);
    BSTD_UNUSED(pSize);
    BSTD_UNUSED(pSize2);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
#endif
}

/**
Summary:
Return an audio encoder buffer if outputting to memory
**/
NEXUS_Error NEXUS_SimpleEncoder_AudioReadComplete(
    NEXUS_SimpleEncoderHandle handle,
    unsigned descriptorsCompleted /* must be <= pSize+pSize2 returned by last NEXUS_SimpleEncoder_GetAudioBuffer call. */
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_SimpleEncoder);
#if NEXUS_HAS_AUDIO
    if ( handle->serverSettings.audioMuxOutput )
    {
        return NEXUS_AudioMuxOutput_ReadComplete(handle->serverSettings.audioMuxOutput, descriptorsCompleted);
    }
    else
    {
        return BERR_NOT_SUPPORTED;
    }
#else
    BSTD_UNUSED(descriptorsCompleted);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
#endif
}

/**
Summary:
**/
void NEXUS_SimpleEncoder_GetStatus(
    NEXUS_SimpleEncoderHandle handle,
    NEXUS_SimpleEncoderStatus *pStatus /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_SimpleEncoder);
    BKNI_Memset(pStatus, 0, sizeof(*pStatus));
#if NEXUS_HAS_STREAM_MUX
    if (handle->streamMux) {
        NEXUS_StreamMuxStatus status;
        int rc;
        rc = NEXUS_StreamMux_GetStatus(handle->streamMux, &status);
        if (!rc) {
            pStatus->streamMux.currentTimestamp.video[0] = status.video.pid[0].currentTimestamp;
            pStatus->streamMux.currentTimestamp.audio[0] = status.audio.pid[0].currentTimestamp;
        }
    }
    if ( handle->serverSettings.videoEncoder )
    {
        BDBG_CASSERT(sizeof(pStatus->video) == sizeof(NEXUS_VideoEncoderStatus));
        NEXUS_VideoEncoder_GetStatus(handle->serverSettings.videoEncoder, (NEXUS_VideoEncoderStatus *)&pStatus->video);
        pStatus->videoEncoder.enabled = true;
        pStatus->videoEncoder.index = NEXUS_VideoEncoder_GetIndex_isrsafe(handle->serverSettings.videoEncoder);
    }
#endif
#if NEXUS_HAS_AUDIO
    if ( handle->serverSettings.audioMuxOutput )
    {
        BDBG_CASSERT(sizeof(pStatus->audio) == sizeof(NEXUS_AudioMuxOutputStatus));
        NEXUS_AudioMuxOutput_GetStatus(handle->serverSettings.audioMuxOutput, (NEXUS_AudioMuxOutputStatus *)(char *)&pStatus->audio);
    }
#endif
}

BDBG_FILE_MODULE(nexus_simple_decoder_proc);

void NEXUS_SimpleDecoderModule_P_PrintEncoder(void)
{
#if BDBG_DEBUG_BUILD
    NEXUS_SimpleEncoderHandle handle;
    for (handle=nexus_simple_encoder_p_first(); handle; handle = nexus_simple_encoder_p_next(handle)) {
        BDBG_MODULE_LOG(nexus_simple_decoder_proc, ("encode %u(%p)", handle->index, (void *)handle));
        BDBG_MODULE_LOG(nexus_simple_decoder_proc, ("  acquired %d, started %d, clientStarted %d",
            handle->acquired, handle->started, handle->clientStarted));
        BDBG_MODULE_LOG(nexus_simple_decoder_proc, ("  display %p, main display %p, audioMuxOutput %p, videoEncoder %p, streamMux %p, stcChannel %p",
            (void *)handle->transcodeDisplay,
            (void *)handle->serverSettings.displayEncode.display,
            (void *)handle->serverSettings.audioMuxOutput,
            (void *)handle->serverSettings.videoEncoder,
            (void *)handle->streamMux,
            (void *)handle->serverSettings.stcChannelTranscode));
    }
#endif
}

bool nexus_simpleencoder_p_nonRealTime(NEXUS_SimpleEncoderHandle encoder)
{
    return encoder && encoder->serverSettings.nonRealTime;
}

void nexus_simpleencoder_p_beginProgramChange(NEXUS_SimpleEncoderHandle encoder, bool *pSendEos)
{
    if (encoder->settings.programChange) {
        *pSendEos = false;
        encoder->programChange.inProgress = true;
    }
}

void NEXUS_SimpleEncoder_GetStcStatus_priv(NEXUS_SimpleEncoderHandle encoder, NEXUS_SimpleStcChannelEncoderStatus *pStatus)
{
    BKNI_Memset(pStatus, 0, sizeof(*pStatus));
    if (encoder) {
        pStatus->enabled = true;
        pStatus->timebase = encoder->serverSettings.timebase;
        pStatus->nonRealTime = encoder->serverSettings.nonRealTime;
    }
}

NEXUS_Error NEXUS_SimpleEncoder_GetCrcData( NEXUS_SimpleEncoderHandle handle, NEXUS_DisplayCrcData *pEntries, unsigned numEntries, unsigned *pNumReturned )
{
    NEXUS_DisplaySettings settings;
    int rc;
    if (!handle->transcodeDisplay) {
        return BERR_TRACE(NEXUS_NOT_AVAILABLE);
    }
    NEXUS_Display_GetSettings(handle->transcodeDisplay, &settings);
    if (settings.crcQueueSize == 0) {
        settings.crcQueueSize = 128;
        rc = NEXUS_Display_SetSettings(handle->transcodeDisplay, &settings);
        if (rc) return BERR_TRACE(rc);
    }
    return NEXUS_Display_GetCrcData(handle->transcodeDisplay, pEntries, numEntries, pNumReturned);
}

NEXUS_Error NEXUS_SimpleEncoder_InsertRandomAccessPoint( NEXUS_SimpleEncoderHandle handle )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_SimpleEncoder);
#if NEXUS_HAS_STREAM_MUX
    if (handle->serverSettings.videoEncoder) {
        return NEXUS_VideoEncoder_InsertRandomAccessPoint(handle->serverSettings.videoEncoder);
    }
#endif
    return BERR_TRACE(BERR_NOT_AVAILABLE);
}

#if NEXUS_HAS_STREAM_MUX
/* return zero if added, non-zero if already added */
static NEXUS_Error NEXUS_SimpleEncoder_P_AddAuxPid(NEXUS_SimpleEncoderHandle handle, unsigned pid)
{
    unsigned i;
    int rc;
    NEXUS_PidChannelHandle pidChannel;

    for (i=0;i<handle->auxpid.total;i++) {
        if (handle->auxpid.pid[i] == pid) {
            return -1; /* already added, no BERR_TRACE */
        }
    }
    if (handle->auxpid.total == handle->auxpid.max) {
        /* alloc more */
        unsigned n = handle->auxpid.max ? handle->auxpid.max * 2 : 16;
        void *ptr = BKNI_Malloc(n * sizeof(handle->auxpid.pid[0]));
        if (!ptr) {
            return BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
        }
        if (handle->auxpid.pid) {
            BKNI_Memcpy(ptr, handle->auxpid.pid, handle->auxpid.total * sizeof(handle->auxpid.pid[0]));
            BKNI_Free(handle->auxpid.pid);
        }
        handle->auxpid.pid = ptr;
        handle->auxpid.max = n;
    }
    handle->auxpid.pid[handle->auxpid.total++] = pid;

    pidChannel = NEXUS_Playpump_OpenPidChannel(handle->serverSettings.playpump[2], pid, NULL);
    if (!pidChannel) {
        return BERR_TRACE(NEXUS_UNKNOWN);
    }
    rc = NEXUS_Recpump_AddPidChannel(handle->startSettings.recpump, pidChannel, NULL);
    if (rc) {
        NEXUS_Playpump_ClosePidChannel(handle->serverSettings.playpump[2], pidChannel);
        return BERR_TRACE(rc);
    }

    return NEXUS_SUCCESS;
}

static NEXUS_Error NEXUS_SimpleEncoder_P_ScanAuxPid(NEXUS_SimpleEncoderHandle handle, const NEXUS_StreamMuxSystemData *pSystemDataBuffer)
{
    unsigned i, lastpid = 0x1fff;
    if (!NEXUS_P_CpuAccessibleAddress(pSystemDataBuffer->pData)) {
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }
    for (i=0;i<pSystemDataBuffer->size;i+=188) {
        const uint8_t *ptr = &((uint8_t*)(pSystemDataBuffer->pData))[i];
        unsigned pid = ((ptr[1]&0x1f) << 8) | ptr[2];
        if (pid != lastpid) {
            NEXUS_SimpleEncoder_P_AddAuxPid(handle, pid);
            lastpid = pid;
        }
    }
    return NEXUS_SUCCESS;
}
#endif

NEXUS_Error NEXUS_SimpleEncoder_AddSystemDataBuffer( NEXUS_SimpleEncoderHandle handle, const NEXUS_StreamMuxSystemData *pSystemDataBuffer )
{
#if NEXUS_HAS_STREAM_MUX
    int rc;
    BDBG_OBJECT_ASSERT(handle, NEXUS_SimpleEncoder);
    if (!handle->streamMux || handle->startSettings.output.transport.pmtPid) {
        return BERR_TRACE(NEXUS_NOT_AVAILABLE);
    }

    rc = NEXUS_SimpleEncoder_P_ScanAuxPid(handle, pSystemDataBuffer);
    if (rc) return BERR_TRACE(rc);

    return NEXUS_StreamMux_AddSystemDataBuffer(handle->streamMux, pSystemDataBuffer);
#else
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pSystemDataBuffer);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
#endif
}

void NEXUS_SimpleEncoder_GetCompletedSystemDataBuffers( NEXUS_SimpleEncoderHandle handle, unsigned *pCompletedCount )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_SimpleEncoder);
#if NEXUS_HAS_STREAM_MUX
    if (handle->streamMux) {
        size_t count;
        NEXUS_StreamMux_GetCompletedSystemDataBuffers(handle->streamMux, &count);
        *pCompletedCount = count;
        return;
    }
#endif
    *pCompletedCount = 0;
}

void nexus_simpleencoder_p_decoder_watchdog(void *context)
{
    NEXUS_SimpleEncoderHandle handle;
    BSTD_UNUSED(context);
    for (handle=nexus_simple_encoder_p_first(); handle; handle = nexus_simple_encoder_p_next(handle)) {
        BDBG_OBJECT_ASSERT(handle, NEXUS_SimpleEncoder);
        if (handle->serverSettings.nonRealTime) {
            NEXUS_SimpleEncoder_Watchdog(handle->server, handle);
        }
    }
}

void NEXUS_SimpleEncoder_Watchdog( NEXUS_SimpleEncoderServerHandle server, NEXUS_SimpleEncoderHandle handle )
{
#if NEXUS_HAS_STREAM_MUX
    int rc;
    BDBG_OBJECT_ASSERT(handle, NEXUS_SimpleEncoder);
    if (handle->server != server) {BERR_TRACE(NEXUS_INVALID_PARAMETER); return;}

    if (!handle->videoEncoderStarted) return;

    BDBG_WRN(("Video encoder %p watchdog callback fired. Restarting.", (void*)handle));
    if (handle->serverSettings.nonRealTime) {
        /* restart everything */
        nexus_simpleencoder_p_stop(handle);
        /* TODO: why is p_stop clearing clientStarted? */
        handle->clientStarted = true;
        rc = nexus_simpleencoder_p_pre_start(handle);
        if (rc) {
            BERR_TRACE(rc);
        }
        else {
            rc = nexus_simpleencoder_p_start(handle);
            if (rc) {
                BERR_TRACE(rc);
            }
            else {
                if (handle->startSettings.input.video) {
                    rc = nexus_simplevideodecoder_p_start(handle->startSettings.input.video);
                    if (rc) BERR_TRACE(rc);
                }
                if (handle->startSettings.input.audio) {
                    rc = nexus_simpleaudiodecoder_p_start(handle->startSettings.input.audio);
                    if (rc) BERR_TRACE(rc);
                }
            }
        }
    } else {
        /* restart only video encoder */
        nexus_simpleencoder_p_stop_videoencoder(handle, true);
        rc = nexus_simpleencoder_p_start_videoencoder(handle, false);
        if (rc) BERR_TRACE(rc);
    }
#else
    BSTD_UNUSED(server);
    BSTD_UNUSED(handle);
#endif
}
