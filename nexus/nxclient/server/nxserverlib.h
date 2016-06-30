/******************************************************************************
 *  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.
 ******************************************************************************/
#ifndef NXSERVERLIB_H__
#define NXSERVERLIB_H__

#include "nexus_types.h"
#include "nxclient.h"
#include "nxclient_config.h"
#include "bkni_multi.h"
#include "nexus_platform.h"
#if NEXUS_HAS_IR_INPUT
#include "nexus_ir_input.h"
#endif
#if NEXUS_NUM_AUDIO_CAPTURES
#include "nexus_audio_capture.h"
#endif
#include "nxserver_ipc_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*****************
general purpose nexus multiprocess server

see nexus/nxclient/include for client-side API
see nexus/nxclient/server for reference implementation
*****************/

typedef struct b_client *nxclient_t;
typedef struct b_server *nxserver_t;
struct b_session;

/* external index allocation allows nxserver to cooperation with external allocation of resources
for features like transcode */
enum nxserverlib_index_type {
    nxserverlib_index_type_video_decoder,     /* first param to NEXUS_VideoDecoder_Open */
    nxserverlib_index_type_stc_index,         /* NEXUS_StcChannelSettings.index (aka serial STC) */
    nxserverlib_index_type_max
};

/* Dolby MS Type */
enum nxserverlib_dolby_ms_type {
    nxserverlib_dolby_ms_type_none,
    nxserverlib_dolby_ms_type_ms11,
    nxserverlib_dolby_ms_type_ms12,
    nxserverlib_dolby_ms_type_max
};

enum nxserverlib_svp_type {
    nxserverlib_svp_type_none,
    nxserverlib_svp_type_cdb, /* Force CRR for video CDB */
    nxserverlib_svp_type_cdb_urr /* Force CRR for video CDB + default to all secure only buffers */
};

struct nxserver_settings
{
    BKNI_MutexHandle lock; /* mutex held by upper layer when NxClient functions are called.
                              can be used by nxserverlib to synchronize internal callbacks. */
    bool cursor;
    unsigned framebuffers;
    bool native_3d;
    NEXUS_ClientMode client_mode;
    NEXUS_Certificate certificate; /* compared to NxClient_JoinSettings.certificate */
    bool transcode;
    NEXUS_MemoryConfigurationSettings memConfigSettings; /* pass in memconfig used to init system */
    enum nxserverlib_svp_type svp; /* Secure Video Path: settings */
    unsigned growHeapBlockSize; /* NXCLIENT_DYNAMIC_HEAP will grow and shrink with these contiguous blocks.
                                   Should be >= the largest contiguous block requried. */
    unsigned timeout;
    bool prompt;
    bool grab; /* allow clients to grab resources from other clients */
    struct {
        unsigned width, height;
    } fbsize;
    bool allowCompositionBypass;
    struct {
        unsigned fifoSize; /* override fifo size */
        bool dynamicPictureBuffers;
    } videoDecoder;
    struct {
        unsigned fifoSize; /* default fifo size. if 0, use internal defaults. */
        bool audioDescription; /* open third decoder for audio description. */
        bool enablePassthroughBuffer;   /* reserve a playback buffer for app-driven audio passthrough data */
    } audioDecoder;
    struct {
        unsigned sampleRate; /* fixed mixer sample rate, if specified */
    } audioMixer;
    struct {
        bool i2sEnabled;
    } audioInputs;
    struct {
        unsigned fifoSize; /* default fifo size. if 0, use internal defaults. */
    } audioPlayback;
#define NXCLIENT_MAX_SESSIONS 4
    struct nxserver_session_settings {
        struct {
            bool hd, sd, encode;
            bool encode_video_only; /* if encode && encode_video_only, then full-screen video and no graphics */
        } output; /* if per-display settings grow, refactor into output[nxserver_display_type] */
#define IS_SESSION_DISPLAY_ENABLED(nxserver_session_settings)	(nxserver_session_settings.output.hd || nxserver_session_settings.output.sd)
#if NEXUS_HAS_IR_INPUT
        NEXUS_IrInputMode ir_input_mode;
#endif
        bool evdevInput;
        bool graphics;
        unsigned audioPlaybacks; /* control initial distribution of audio playbacks across sessions */
        bool avl;
        bool truVolume;
        enum nxserverlib_dolby_ms_type dolbyMs; /* Dolby Multi Stream (MS11, MS12, ...) */
#if NEXUS_NUM_AUDIO_CAPTURES
        NEXUS_AudioCaptureFormat audioCapture;
#endif
        bool karaoke;
    } session[NXCLIENT_MAX_SESSIONS];

    NxClient_DisplaySettings display; /* only session 0 */
    struct {
        struct {
            bool initialFormat; /* use user-specified display format at init, but follow HDMI EDID preferred format for any hotplug after that. */
        } hd;
        struct {
            NEXUS_Rect graphicsPosition;
        } sd;
    } display_init;

    struct {
        NEXUS_HeapHandle heap[NEXUS_MAX_HEAPS]; /* client heaps */

        /* callback when client connects. return non-zero to reject client. pClientSettings may be modified. */
        int (*connect)(nxclient_t client, const NxClient_JoinSettings *pJoinSettings, NEXUS_ClientSettings *pClientSettings);

        /* callback when client disconnects */
        void (*disconnect)(nxclient_t client, const NxClient_JoinSettings *pJoinSettings);

        /* DEVEL: other client callbacks. these are subject to change during development. only called for ipc clients. */
        void (*nxclient_alloc)(nxclient_t client, const NxClient_AllocSettings *pSettings, const NxClient_AllocResults *pResults);
        void (*nxclient_free)(nxclient_t client, const NxClient_AllocResults *pResults);
        void (*nxclient_connect)(nxclient_t client, const NxClient_ConnectSettings *pSettings, unsigned connectId);
        void (*nxclient_disconnect)(nxclient_t client, unsigned connectId);
        /* if callback returns non-zero, client's change is not made. */
        int  (*nxclient_setSurfaceComposition)(nxclient_t client, unsigned surfaceClientId, NEXUS_SurfaceComposition *pSettings);
    } client;

#if NEXUS_HAS_HDMI_OUTPUT
    struct {
        NEXUS_HdmiOutputHdcpKsv txAksv;
        NEXUS_HdmiOutputHdcpKey encryptedTxKeySet[NEXUS_HDMI_OUTPUT_HDCP_NUM_KEYS];
        char *hdcp2xBinFile;
        char *hdcp1xBinFile;
        NxClient_HdcpLevel alwaysLevel;
        NxClient_HdcpVersion versionSelect;
    } hdcp;
    struct {
        bool drm;
        bool ignoreVideoEdid; /* use HDMI EDID for audio, but not video */
        struct {
            char vendorName[NEXUS_HDMI_SPD_VENDOR_NAME_MAX+1];
            char description[NEXUS_HDMI_SPD_DESCRIPTION_MAX+1];
        } spd;
    } hdmi;
#endif
    unsigned standby_timeout; /* Number of seconds where server waits for clients to acknowledge standby state change */

    struct {
        bool enabled;
        struct {
            NEXUS_DisplayHandle handle; /* by passing display, external app gives up ability to call NEXUS_Display_Close, SetGraphicsSettings,
                                           SetGraphicsFramebuffer, and SetSettings (format change).
                                           Graphics must be done using nxclient and SurfaceCompositor. */
        } display[NEXUS_MAX_DISPLAYS];

        /* set true we must alloc index externally */
        bool enableAllocIndex[nxserverlib_index_type_max];
        int (*allocIndex)(void *callback_context, enum nxserverlib_index_type type, unsigned *pIndex);
        void (*freeIndex)(void *callback_context, enum nxserverlib_index_type type, unsigned index);
        void *callback_context;
    } externalApp; /* Allow external application to open displays, outputs and decoders, then invoke nxserver as needed. See nxserverlib_allow_decode below. */
};

/* Capture some cmdline settings which are parsed and applied to NEXUS_PlatformSettings and
NEXUS_MemoryConfigurationSettings and not passed into nxserver. If you are not using the cmdline
interface to nxserver, you can set these params and call nxserver_modify_platform_settings,
or you can modify NEXUS_PlatformSettings and NEXUS_MemoryConfigurationSettings directly as required. */
struct nxserver_cmdline_settings
{
    struct {
        const char *str[5];
        unsigned total;
    } memconfig, heap;
    bool avc51;
    bool frontend;
    unsigned maxDataRate;
    bool remux;
    unsigned audio_output_delay;
    NEXUS_AudioLoudnessEquivalenceMode loudnessMode;
    struct {
        unsigned dacBandGapAdjust;
        bool dacDetection;
        bool mtgOff;
    } video;
};


/* nxserver_lib.c API */
void nxserver_get_default_settings(struct nxserver_settings *settings);
nxserver_t nxserverlib_init(const struct nxserver_settings *settings);
void nxserverlib_uninit(nxserver_t server);
struct b_session *nxserver_get_client_session(nxclient_t client);
/* get_singleton is used for nxclient local to use externally opened nxserverlib */
nxserver_t nxserverlib_get_singleton(void);
void nxserverlib_get_settings(nxserver_t server, struct nxserver_settings *settings);
#if NEXUS_HAS_INPUT_ROUTER
/* allow external receipt of user input with nxserver distribution */
int nxserverlib_send_input(nxclient_t client, unsigned inputClientId, const NEXUS_InputRouterCode *pCode);
#endif
int nxserverlib_set_server_alpha(nxclient_t client, unsigned alpha);

/* nxserver_cmdline.c */
int nxserver_parse_cmdline(int argc, char **argv,
    struct nxserver_settings *settings,
    struct nxserver_cmdline_settings *cmdline_settings
    );
int nxserver_modify_platform_settings(
    const struct nxserver_settings *settings,
    const struct nxserver_cmdline_settings *cmdline_settings,
    NEXUS_PlatformSettings *pPlatformSettings,
    NEXUS_MemoryConfigurationSettings *pMemConfigSettings
    );
void nxserver_set_client_heaps(
    struct nxserver_settings *settings,
    const NEXUS_PlatformSettings *pPlatformSettings
    );
int nxserver_heap_by_type(
    const NEXUS_PlatformSettings *pPlatformSettings,
    unsigned heapType /* NEXUS_HEAP_TYPE bitmasks in NEXUS_PlatformSettings.heap[].heapType */
    );

/* nxserver.c API */
nxserver_t nxserver_init(int argc, char **argv, bool blocking);
void nxserver_uninit(nxserver_t server);

/* only callable by nxserverlic_ipc.c. */
nxclient_t NxClient_P_CreateClient(nxserver_t server, const NxClient_JoinSettings *pJoinSettings, NEXUS_Certificate *pCert, unsigned pid);
void NxClient_P_DestroyClient(nxclient_t client);

/**
If nxserver_settings.externalApp.enabled is true, this function allows control of decoders to be passed to and from nxserver.
If allow is true, external app must have already closed VideoDecoder, VideoWindow, StcChannel, AudioDecoder, AudioMixer, AudioPlayback
and all other audio filter graph nodes other than the audio outputs that come from NEXUS_PlatformConfiguration.
External app can continue use of frontend, transport and nxclient graphics.
**/
int nxserverlib_allow_decode(nxserver_t server, bool allow);

/* nxserverlib_ipc.c API */
int nxserver_ipc_init(nxserver_t server, BKNI_MutexHandle lock);
void nxserver_ipc_uninit(void);
void nxserver_ipc_close_client(nxclient_t client);

/* server-side implementation of NxClient API
called by nxserver.c from IPC calls, or called by nxclient_local. */
void        NxClient_P_GetDefaultAllocSettings(nxclient_t client, NxClient_AllocSettings *pSettings );
NEXUS_Error NxClient_P_Alloc(nxclient_t client, const NxClient_AllocSettings *pSettings, NxClient_AllocResults *pResults );
void        NxClient_P_Free(nxclient_t client, const NxClient_AllocResults *pResults );
void        NxClient_P_GetDefaultConnectSettings(nxclient_t client, NxClient_ConnectSettings *pSettings );
NEXUS_Error NxClient_P_Connect(nxclient_t client, const NxClient_ConnectSettings *pSettings, unsigned *pConnectId );
NEXUS_Error NxClient_P_RefreshConnect(nxclient_t client, unsigned connectId);
void        NxClient_P_Disconnect(nxclient_t client, unsigned connectId);
void        NxClient_P_GetAudioSettings(nxclient_t client, NxClient_AudioSettings *pSettings );
NEXUS_Error NxClient_P_SetAudioSettings(nxclient_t client, const NxClient_AudioSettings *pSettings );

void        NxClient_P_GetDisplaySettings(nxclient_t client, struct b_session *session, NxClient_DisplaySettings *pSettings );
NEXUS_Error NxClient_P_SetDisplaySettings(nxclient_t client, struct b_session *session, const NxClient_DisplaySettings *pSettings );
NEXUS_Error NxClient_P_GetDisplayStatus(struct b_session *session, NxClient_DisplayStatus *pStatus );

NEXUS_Error NxClient_P_General(nxclient_t client, enum nxclient_p_general_param_type type, const nxclient_p_general_param *param, nxclient_p_general_output *output);

NEXUS_Error NxClient_P_GetStandbyStatus(nxclient_t client, NxClient_StandbyStatus *pStatus);
void        NxClient_P_AcknowledgeStandby(nxclient_t client, bool done);
NEXUS_Error NxClient_P_SetStandbySettings(nxclient_t client, const NxClient_StandbySettings *pSettings);

NEXUS_Error NxClient_P_Config_GetJoinSettings(nxclient_t client, NEXUS_ClientHandle nexusClient, NxClient_JoinSettings *pSettings );
void        NxClient_P_Config_GetSurfaceClientComposition(nxclient_t client, NEXUS_ClientHandle nexusClient, NEXUS_SurfaceClientHandle surfaceClient, NEXUS_SurfaceComposition *pComposition );
NEXUS_Error NxClient_P_Config_SetSurfaceClientComposition(nxclient_t client, NEXUS_ClientHandle nexusClient, NEXUS_SurfaceClientHandle surfaceClient, const NEXUS_SurfaceComposition *pComposition );
NEXUS_Error NxClient_P_Config_GetConnectList(nxclient_t client, NEXUS_ClientHandle nexusClient, NxClient_ConnectList *pList );
NEXUS_Error NxClient_P_Config_RefreshConnect(nxclient_t client, NEXUS_ClientHandle nexusClient, unsigned connectId );
void        NxClient_P_Config_GetConnectSettings(nxclient_t client, NEXUS_ClientHandle nexusClient, unsigned connectId, NxClient_ConnectSettings *pSettings );
void        NxClient_P_Config_GetInputClientServerFilter(nxclient_t client, NEXUS_ClientHandle nexusClient, NEXUS_InputClientHandle inputClient, unsigned *pFilter );
NEXUS_Error NxClient_P_Config_SetInputClientServerFilter(nxclient_t client, NEXUS_ClientHandle nexusClient, NEXUS_InputClientHandle inputClient, unsigned filter );

void        NxClient_P_GetPictureQualitySettings(nxclient_t client, NxClient_PictureQualitySettings *pSettings );
NEXUS_Error NxClient_P_SetPictureQualitySettings(nxclient_t client, const NxClient_PictureQualitySettings *pSettings );

NEXUS_Error NxClient_P_GetCallbackStatus(nxclient_t client, NxClient_CallbackStatus *pStatus );
NEXUS_Error NxClient_P_GetAudioStatus(nxclient_t client, NxClient_AudioStatus *pStatus );

void NxClient_P_GetSurfaceClientComposition(nxclient_t client, unsigned surfaceClientId, NEXUS_SurfaceComposition *pComposition);
NEXUS_Error NxClient_P_SetSurfaceClientComposition(nxclient_t client, unsigned surfaceClientId, const NEXUS_SurfaceComposition *pComposition);

/* local server calls for client control */
struct nxclient_status
{
    NEXUS_ClientHandle handle;
    unsigned pid;
};
void nxclient_get_status(nxclient_t client, struct nxclient_status *pstatus);
NEXUS_Error nxserver_p_focus_input_client(nxclient_t client);
NEXUS_Error nxserver_p_focus_surface_client(nxclient_t client);
int nxclient_p_parse_password_file(const char *filename, NEXUS_Certificate *certificate);

/* nxclient_socket.c, can be overridden */
int b_nxclient_client_connect(void);
int b_nxclient_socket_listen(void);

bool nxserver_is_standby(nxserver_t server);

#ifdef __cplusplus
}
#endif

#endif
