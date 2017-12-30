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
 ******************************************************************************/
#ifndef NXCLIENT_H__
#define NXCLIENT_H__

#include "nexus_types.h"
#include "nexus_surface_compositor_types.h"
#include "nxclient_global.h"
#include "nxclient_standby.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
NxClient Overview

NxClient is a library above Nexus which allows a client to allocate and connect resources from a server application and also set global settings.
Resources include video decoders, audio decoders, surface clients, and user input clients.
Global settings usually pertain to the display and audio outputs. Typical settings include display format, video window and surface client position, audio level, and audio output format.

An example series of calls for video decoder is:

    NxClient_Join                    - connect to nexus driver and server app
    NxClient_Alloc                   - ask server to create resources. result is an id acquire can be acquired from nexus driver.
    NEXUS_SimpleVideoDecoder_Acquire - acquire handle from nexus driver using id.
    NxClient_Connect                 - request that the server connect those resources to underlying hardware for exclusive use
    NEXUS_SimpleVideoDecoder_Start   - you are now decoding

An example series of calls for graphics is:

    NxClient_Join                    - connect to nexus driver and server app
    NxClient_Alloc                   - ask server to create resources. result is an id acquire can be acquired from nexus driver.
    NEXUS_SurfaceClient_Acquire      - acquire handle from nexus driver using id.
    NEXUS_SurfaceClient_Set          - you are now displaying graphics

NxClient can be called from any process, and be called multiple times from the same process.
A library can Join, Alloc and Connect without any reference to another library's Join, Alloc and Connect calls.
This allows reusable libraries to be integrated in larger applications.

Garbage collection and handle tracking is done at a process-level, not a thread-level.
**/

#define NXCLIENT_MAX_IDS 16
#define NXCLIENT_MAX_VIDEO_WINDOWS NXCLIENT_MAX_IDS
#define NXCLIENT_MAX_NAME 32

/* standard client heap indices. use in NEXUS_ClientConfiguration.heap[] */
#define NXCLIENT_DEFAULT_HEAP            0
#define NXCLIENT_FULL_HEAP               1
#define NXCLIENT_VIDEO_SECURE_HEAP       2
#define NXCLIENT_SECONDARY_GRAPHICS_HEAP 3
#define NXCLIENT_DYNAMIC_HEAP            4
#define NXCLIENT_EXPORT_HEAP             5
#define NXCLIENT_SECURE_GRAPHICS_HEAP    6

#define NXCLIENT_NOT_ALLOWED             NEXUS_MAKE_ERR_CODE(0x200, 1)


/**
Summary:
Join with Nexus server application (using app ipc) and Nexus driver (using Nexus multiprocess)

Description:
This dynamically registers a client with both nexus and the server app.
NxClient_Join can be called from multiple processes, or multiple times from the same process.

NxClient_Join is reference counted within the same process. You must call NxClient_Uninit an equal
number of times to detach from nexus and the server app.
**/
typedef struct NxClient_JoinSettings
{
    char name[NXCLIENT_MAX_NAME];
    bool tunnelCapable; /* deprecated */
    unsigned timeout; /* in seconds. if unable to join, try again for this number of seconds. defaults to 0. */
    unsigned session; /* set index to choose which session in a multi-session server configuration */
    bool ignoreStandbyRequest; /* deprecated. see NxClient_RegisterAcknowledgeStandby() instead. */
    NEXUS_ClientMode mode; /* requested client mode. server may reject the request. */
    NEXUS_Certificate certificate; /* allows NEXUS_ClientMode_eProtected (aka trusted) status if server requires */
} NxClient_JoinSettings;

void NxClient_GetDefaultJoinSettings(
    NxClient_JoinSettings *pSettings
    );

NEXUS_Error NxClient_Join(
    const NxClient_JoinSettings *pSettings
    );

void NxClient_Uninit(void);

/**
Summary:
Request that server allocate handles for the client

Description:
The handles are exclusively owned by the client.

The simpleVideoDecoder, simpleAudioDecoder and simpleAudioPlayback handles
come disconnected from any backend or underlying resources.
For example, SimpleVideoDecoder has no VideoDecoder connected to it after Alloc.
You must call NxClient_Connect to use them.
**/

typedef enum NxClient_AudioCrcType {
    NxClient_AudioCrcType_eStereo,
    NxClient_AudioCrcType_eMultichannel,
    NxClient_AudioCrcType_eMax
} NxClient_AudioCrcType;

typedef enum NxClient_AudioCaptureType {
    NxClient_AudioCaptureType_e16BitStereo,
    NxClient_AudioCaptureType_e24BitStereo,
    NxClient_AudioCaptureType_e24Bit5_1,
    NxClient_AudioCaptureType_eCompressed,   /*If running in MS-12 mode this will grab the AC3 encoded output of the DDRE
                                                If running in non-MS mode this will grab the transcoded AC3 output of DDP/AAC.*/
    NxClient_AudioCaptureType_eCompressed4x, /*If running in MS-12 mode this will grab the DDP encoded output of the DDRE
                                                If running in non-MS mode this will grab the compressed DDP/AAC output rather
                                                than the transcode AC3 output.*/
    NxClient_AudioCaptureType_eMax
} NxClient_AudioCaptureType;


typedef struct NxClient_AllocSettings
{
    unsigned simpleVideoDecoder;
    unsigned simpleAudioDecoder; /* only 1 supported */
    unsigned simpleAudioPlayback;
    unsigned surfaceClient;
    unsigned inputClient;
    unsigned simpleEncoder;
    unsigned audioCapture;
    unsigned audioCrc;
    struct {
        NxClient_AudioCrcType type;
    } audioCrcType;
    struct {
        NxClient_AudioCaptureType type;
    } audioCaptureType;
} NxClient_AllocSettings;

typedef struct NxClient_AllocResults
{
    struct {
        unsigned id;
    } surfaceClient[NXCLIENT_MAX_IDS];
    struct {
        unsigned id;
    } simpleVideoDecoder[NXCLIENT_MAX_IDS];
    struct {
        unsigned id;
    } simpleAudioDecoder;
    struct {
        unsigned id;
    } simpleAudioPlayback[NXCLIENT_MAX_IDS];
    struct {
        unsigned id;
    } inputClient[NXCLIENT_MAX_IDS];
    struct {
        unsigned id;
    } simpleEncoder[NXCLIENT_MAX_IDS];
    struct {
        unsigned id; /* includes NEXUS_ALIAS_ID */
    } audioCapture;
    struct {
        unsigned id;
    } audioCrc;
} NxClient_AllocResults;

void NxClient_GetDefaultAllocSettings(
    NxClient_AllocSettings *pSettings
    );

NEXUS_Error NxClient_Alloc(
    const NxClient_AllocSettings *pSettings,
    NxClient_AllocResults *pResults
    );

void NxClient_Free(
    const NxClient_AllocResults *pResults
    );

typedef enum NxClient_VideoDecoderConnectType
{
    NxClient_VideoDecoderConnectType_eRegular, /* decoder, feeder and window */
    /* TODO: add eFeeder - feeder and window, used for graphics-as-video */
    NxClient_VideoDecoderConnectType_eWindowOnly, /* used for HDMI input */
    NxClient_VideoDecoderConnectType_eMax
} NxClient_VideoDecoderConnectType;

/**
Summary:
Capabilities that the video decoder must be able to meet in order to be connected.
Additional start-time programming may be required.

Description:
Because fifoSize and avc51Enabled are set at NEXUS_VideoDecoder_Open time, they are requested only here.

This supportedCodecs[] is only used for selecting an AVD, SID or DSP decoder (SID is used for MJPEG where
available. DSP for VP6 on some platforms.) For AVD, the app should also set
NEXUS_VideoDecoderSettings.supportedCodecs[] to control actual memory allocation at runtime.

maxWidth, maxHeight, and colorDepth default to zero, which means any decoder can be used.
Non-zero settings request a capable decoder. These settings can also be changed at runtime to
reduce memory allocation.
**/
typedef struct NxClient_VideoDecoderCapabilities
{
    /* used at open time, not changeable after */
    unsigned fifoSize; /* CDB size in bytes */
    unsigned itbFifoSize; /* ITB size in bytes */
    bool avc51Enabled;
    bool secureVideo;
    unsigned userDataBufferSize; /* Size of userdata buffer in bytes. Increase this for high bitrate userdata applications. */

    /* the following are used to request a decoder and can also be changed
    at runtime to reduce memory allocation */
    unsigned colorDepth; /* if 0 or 8, get any. If 10, get 10-bit capable decoder. */

    NEXUS_VideoFormat maxFormat;
    unsigned maxWidth, maxHeight; /* Set maxFormat = eNone to use these. Assumes p60. Required for sub-SD resolutions like CIF and QCIF. */
    NxClient_VideoDecoderConnectType connectType;

    bool supportedCodecs[NEXUS_VideoCodec_eMax];
    struct {
        unsigned colorDepth; /* If 0 (default), prefer feeder with >= specified decoder colorDepth to avoid downconvert,
                                but do not fail if not available.
                                If 10, require feeder with >= specified decoder colorDepth to avoid downconvert. */
    } feeder;
} NxClient_VideoDecoderCapabilities;

typedef enum NxClient_VideoWindowType
{
    NxClient_VideoWindowType_eMain, /* full screen capable */
    NxClient_VideoWindowType_ePip,  /* reduced size only. typically quarter screen. */
    NxClient_VideoWindowType_eNone, /* app will do video as graphics */
    NxClient_VideoWindowType_Max
} NxClient_VideoWindowType;

typedef struct NxClient_VideoWindowCapabilities
{
    NxClient_VideoWindowType type;
    unsigned maxWidth, maxHeight; /* deprecated. use 'type' instead. */
    bool deinterlaced; /* deprecated. use 'type' instead. */
    bool encoder; /* required to notify nxclient this is a transcode decode */
} NxClient_VideoWindowCapabilities;

/**
Summary: Audio Decoder Type
**/
typedef enum NxClient_AudioDecoderType
{
    NxClient_AudioDecoderType_eDynamic,     /* Only one Dynamic decoder will be attached to the display at any given time.
                                               By default decoders will be created as dynamic decoders. */
    NxClient_AudioDecoderType_ePersistent,  /* More than one persistent decoder may connect to the primary display. Persistent
                                               decoders can exist in parallel with the single active Dynamic decoder as well. */
    NxClient_AudioDecoderType_eStandalone,  /* Standalone decoders are display-less decoders that are only used for
                                               application managed output, such as capturing to memory or standalone encode. */
    NxClient_AudioDecoderType_Max
} NxClient_AudioDecoderType;

/**
Summary:
**/
typedef struct NxClient_AudioDecoderCapabilities
{
    bool encoder; /* if true, prefer decoders which are dedicated for transcode */
    NxClient_AudioDecoderType type; /* specify the type for this audio decoder. */
} NxClient_AudioDecoderCapabilities;

/**
Summary:
**/
typedef struct NxClient_EncoderCapabilities
{
    bool interlaced;
    unsigned maxWidth, maxHeight;
    NEXUS_VideoFrameRate maxFrameRate;
} NxClient_EncoderCapabilities;

/**
Summary:
Request server to connect client resources to hardware resources.

Description:
A connection makes a resource usable.
SurfaceClient and InputClient do not require Connect.

For video, main and PIP decoders must be connected with separate NxClient_Connect calls.

A set of mosaic video decoders must be connected with a single NxClient_Connect call.

Audio and video that will be lipsynced (using SimpleStcChannel) should be connected with a single NxClient_Connect call.
They can be connected with separate NxClient_Connect calls, but two STCs will be allocated and
only one used if they are lipsynced, so it is not recommended.

The ids used by Connect can come from multiple Allocs as long as multiple ids of the same type come from the same Alloc.
So, simpleVideoDecoder[].id can come from one Alloc and simpleAudioDecoder.id can come from another Alloc,
but simpleVideoDecoder[0].id and simpleVideoDecoder[1].id must come from the same Alloc.
**/
typedef struct NxClient_ConnectSettings
{
    struct {
        unsigned id; /* id used to acquire SimpleVideoDecoder. 0 is no request. */
        unsigned surfaceClientId; /* id for top-level surface client */
        unsigned windowId; /* Same as window_id param in NEXUS_SurfaceClient_AcquireVideoWindow.
                              It is relative to the surfaceClientId, which is its parent.
                              If you have only one video window under a SurfaceClient, always use 0.
                              If you have more than one video window under a SurfaceClient, this id differentiates. */
        NxClient_VideoDecoderCapabilities decoderCapabilities;
        NxClient_VideoWindowCapabilities windowCapabilities;
    } simpleVideoDecoder[NXCLIENT_MAX_IDS];
    struct {
        unsigned id; /* id used to acquire SimpleAudioDecoder. 0 is no request. */
        bool primer; /* if decoder is already in use, use primers instead */
        NxClient_AudioDecoderCapabilities decoderCapabilities;
    } simpleAudioDecoder;
    struct {
        unsigned id; /* id used to acquire SimpleAudioPlayback. 0 is no request. */
        struct {
            bool enabled;
            unsigned index;
        } i2s;
        bool compressed; /* True if this playback will be used to route compressed data to SPDIF/HDMI outputs */
    } simpleAudioPlayback[NXCLIENT_MAX_IDS];
    struct {
        unsigned id; /* id used to acquire SimpleEncoder. 0 is no request. */
        bool display; /* Encode the display and audio for this session. Not for file transcoding. */
        bool nonRealTime;
        /* TODO: this API is under development, subject to change */
        struct {
            bool cpuAccessible; /* deprecated */
        } audio, video;
        NxClient_EncoderCapabilities encoderCapabilities;
    } simpleEncoder[NXCLIENT_MAX_IDS];
} NxClient_ConnectSettings;

void NxClient_GetDefaultConnectSettings(
    NxClient_ConnectSettings *pSettings
    );

/**
Ask server to connect underlying resources to this client's handles.

For example, the server will connect a VideoDecoder to the client's SimpleVideoDecoder, or a VideoEncoder
to a client's SimpleVideoEncoder. Other resource allocations will be made as well, like allocating
VideoWindows, serial STC's, timebases, StreamMux instances, etc.

If scarce resources are already in use by another client, the server may grab them away from that client
to fulfill this request. The resource allocation algorithm is a simple "last one wins."
Decode and encode resources used by transcode cannot be grabbed away.

After receiving the resource, the server may grab them away from this client.
As long as the connectId stays valid (that is, until NxClient_Disconnect is called),
the server will restore resources after the other client is done with them.
**/
NEXUS_Error NxClient_Connect(
    const NxClient_ConnectSettings *pSettings,
    unsigned *pConnectId
    );

/**
If any resources were grabbed away by another client, this will grab them back.

This is functionally equivalent to calling Disconnect;Connect with the same settings, but without a forced disconnect if resources weren't grabbed away.
**/
NEXUS_Error NxClient_RefreshConnect(unsigned connectId);

/**
Release underlying resources and remove the request for resources.

The connectId is no longer valid. Underlying resources will not be restored without a new Connect call.
**/
void NxClient_Disconnect(unsigned connectId);

/**
Allow client to set server-level composition settings on its own surfaces
including position, zorder, alpha blending more more.
Not used for video window positioning which is done directly with SurfaceClient API.

See NxClient_Config_SetSurfaceClientComposition for an API to change another client's surface composition.
**/
void NxClient_GetSurfaceClientComposition(
    unsigned surfaceClientId, /* top-level surface client id */
    NEXUS_SurfaceComposition *pSettings
    );

NEXUS_Error NxClient_SetSurfaceClientComposition(
    unsigned surfaceClientId, /* top-level surface client id */
    const NEXUS_SurfaceComposition *pSettings
    );

/**
NxClient_Reconfig allows for reconfigurating multiple NxClient
connections without having to disconnect
**/
typedef enum NxClient_ReconfigType
{
    NxClient_ReconfigType_eRerouteVideoAndAudio, /* for instance, main/PIP swap for video and audio */
    NxClient_ReconfigType_eRerouteVideo,         /* for instance, main/PIP swap, but keep audio the same */
    NxClient_ReconfigType_eRerouteAudio,         /* for instance, main/PIP swap for audio */
    NxClient_ReconfigType_eMax
} NxClient_ReconfigType;

typedef struct NxClient_ReconfigSettings
{
    struct {
        unsigned connectId1, connectId2;
        NxClient_ReconfigType type;
    } command[5];
} NxClient_ReconfigSettings;

void NxClient_GetDefaultReconfigSettings(
    NxClient_ReconfigSettings *pSettings
    );

NEXUS_Error NxClient_Reconfig(
    const NxClient_ReconfigSettings *pSettings
    );

/**
Change mode after Join
We currently support raising your privilege, from eUntrusted to eProtected, or eProtected to eVerified.
**/
typedef struct NxClient_ClientModeSettings
{
    NEXUS_ClientMode mode;
    NEXUS_Certificate certificate;
} NxClient_ClientModeSettings;

void NxClient_GetDefaultClientModeSettings(
    NxClient_ClientModeSettings *pSettings
    );

NEXUS_Error NxClient_SetClientMode(
    const NxClient_ClientModeSettings *pSettings
    );

#ifdef __cplusplus
}
#endif

#endif /* NXCLIENT_H__ */
