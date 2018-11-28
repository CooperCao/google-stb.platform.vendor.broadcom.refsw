/***************************************************************************
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
 **************************************************************************/
#ifndef NEXUS_SIMPLE_AUDIO_DECODER_SERVER_H__
#define NEXUS_SIMPLE_AUDIO_DECODER_SERVER_H__

#include "nexus_types.h"
#include "nexus_simple_audio_decoder.h"
#include "nexus_simple_decoder_types.h"
#include "nexus_stc_channel.h"
#if NEXUS_HAS_AUDIO
#include "nexus_audio_decoder.h"
#include "nexus_spdif_output.h"
#include "nexus_audio_dac.h"
#include "nexus_i2s_output.h"
#include "nexus_audio_playback.h"
#include "nexus_i2s_input.h"
#include "nexus_audio_capture.h"
#include "nexus_audio_mixer.h"
#endif
#if NEXUS_HAS_HDMI_OUTPUT
#include "nexus_hdmi_output.h"
#endif
#include "nexus_simple_audio_playback_server.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
This server-side API is semi-private. In multi-process systems, only server apps like nxserver will call it.
Client apps will not call it. Therefore, this API is subject to non-backward compatible change.
**/

#define NEXUS_MAX_SIMPLE_DECODER_SPDIF_OUTPUTS 2
#define NEXUS_MAX_SIMPLE_DECODER_HDMI_OUTPUTS 2
#define NEXUS_MAX_SIMPLE_DECODER_I2S_OUTPUTS 2

/**
Summary: Simple Audio Decoder Type
**/
typedef enum NEXUS_SimpleAudioDecoderType {
    NEXUS_SimpleAudioDecoderType_eDynamic,
    NEXUS_SimpleAudioDecoderType_ePersistent,
    NEXUS_SimpleAudioDecoderType_eStandalone,
    NEXUS_SimpleAudioDecoderType_eMax
} NEXUS_SimpleAudioDecoderType;

typedef struct NEXUS_SimpleAudioDecoderServer *NEXUS_SimpleAudioDecoderServerHandle;

/* settings set by the server for this client, so not transferred with MoveServerSettings */
typedef struct NEXUS_SimpleAudioDecoderClientSettings
{
    bool secure;
} NEXUS_SimpleAudioDecoderClientSettings;

/* settings set by server and transferred to the active decoder using MoveServerSettings */
typedef struct NEXUS_SimpleAudioDecoderServerSettings
{
    NEXUS_SimpleAudioDecoderHandle masterHandle; /* The main master simple audio decoder handle, should remain NULL for anything else */
    NEXUS_SimpleAudioDecoderType type;
    NEXUS_SimpleDecoderDisableMode disableMode;
    NEXUS_AudioDecoderHandle primary;   /* for decode and simul */
    NEXUS_AudioDecoderHandle secondary; /* for compressed passthrough */
    NEXUS_AudioDecoderHandle description; /* for audio description */
    NEXUS_AudioPlaybackHandle passthroughPlayback; /* For passthroughBuffer mode */

    struct {
        NEXUS_SimpleAudioDecoderHandle decoder; /* Only valid for dynamic decoders */
        bool suspended;
    } persistent[NEXUS_MAX_AUDIO_DECODERS];

    struct {
        bool ms11;
        bool ms12;
    } capabilities; /* allows the server to specify this simple decoder's capabilities */

    struct {
        NEXUS_AudioMixerHandle stereo, multichannel, persistent;
    } mixers;

    NEXUS_AudioConnectorType syncConnector; /* Connector to use for Sync Channel */
    
    struct {
        NEXUS_SpdifOutputHandle outputs[NEXUS_MAX_SIMPLE_DECODER_SPDIF_OUTPUTS];
        NEXUS_AudioInputHandle input[NEXUS_MAX_AUDIOCODECS]; /* per codec, specify the final stage to be connected to spdif.
            use NEXUS_AudioCodec_eUnknown to specify default configuration for playback-only. */
    } spdif;
    struct {
        NEXUS_HdmiOutputHandle outputs[NEXUS_MAX_SIMPLE_DECODER_HDMI_OUTPUTS];
        NEXUS_AudioInputHandle input[NEXUS_MAX_AUDIOCODECS]; /* per codec, specify the final stage to be connected to hdmi.
            use NEXUS_AudioCodec_eUnknown to specify default configuration for playback-only. */
    } hdmi;
    struct {
        NEXUS_AudioCaptureHandle output;
        NEXUS_AudioInputHandle input[NEXUS_MAX_AUDIOCODECS]; /* per codec, specify the final stage to be connected to audio capture. */
    } capture;
    struct {
        NEXUS_AudioDacHandle output;
        NEXUS_AudioInputHandle input;
        NEXUS_AudioPresentation presentation; /* If alternate stereo and AC4 stream connect to the decoders alternate stereo path, other wise connects to input. */
    } dac;

    struct {
        NEXUS_I2sOutputHandle output;
        NEXUS_AudioInputHandle input;
        NEXUS_AudioPresentation presentation; /* If alternate stereo and AC4 stream connect to the decoders alternate stereo path, other wise connects to input. */
    } i2s[NEXUS_MAX_SIMPLE_DECODER_I2S_OUTPUTS];

    NEXUS_SimpleAudioPlaybackServerHandle simplePlayback;
} NEXUS_SimpleAudioDecoderServerSettings;

void NEXUS_SimpleAudioDecoder_GetDefaultServerSettings(
    NEXUS_SimpleAudioDecoderServerSettings *pSettings /* [out] */
    );

NEXUS_SimpleAudioDecoderServerHandle NEXUS_SimpleAudioDecoderServer_Create( /* attr{destructor=NEXUS_SimpleAudioDecoderServer_Destroy}  */
    void
    );

void NEXUS_SimpleAudioDecoderServer_Destroy(
    NEXUS_SimpleAudioDecoderServerHandle handle
    );

NEXUS_SimpleAudioDecoderHandle NEXUS_SimpleAudioDecoder_Create( /* attr{destructor=NEXUS_SimpleAudioDecoder_Destroy}  */
    NEXUS_SimpleAudioDecoderServerHandle server,
    unsigned index,
    const NEXUS_SimpleAudioDecoderServerSettings *pSettings /* attr{null_allowed=y} */
    );

void NEXUS_SimpleAudioDecoder_Destroy(
    NEXUS_SimpleAudioDecoderHandle handle
    );

void NEXUS_SimpleAudioDecoder_GetServerSettings(
    NEXUS_SimpleAudioDecoderServerHandle server,
    NEXUS_SimpleAudioDecoderHandle handle,
    NEXUS_SimpleAudioDecoderServerSettings *pSettings /* [out] */
    );

NEXUS_Error NEXUS_SimpleAudioDecoder_SetServerSettings(
    NEXUS_SimpleAudioDecoderServerHandle server,
    NEXUS_SimpleAudioDecoderHandle handle,
    const NEXUS_SimpleAudioDecoderServerSettings *pSettings,
    bool forceReconfig
    );

void NEXUS_SimpleAudioDecoder_GetClientSettings(
    NEXUS_SimpleAudioDecoderServerHandle server,
    NEXUS_SimpleAudioDecoderHandle handle,
    NEXUS_SimpleAudioDecoderClientSettings *pSettings /* [out] */
    );

NEXUS_Error NEXUS_SimpleAudioDecoder_SetClientSettings(
    NEXUS_SimpleAudioDecoderServerHandle server,
    NEXUS_SimpleAudioDecoderHandle handle,
    const NEXUS_SimpleAudioDecoderClientSettings *pSettings
    );

NEXUS_Error NEXUS_SimpleAudioDecoder_MoveServerSettings(
    NEXUS_SimpleAudioDecoderServerHandle server,
    NEXUS_SimpleAudioDecoderHandle source,
    NEXUS_SimpleAudioDecoderHandle destination,
    bool allowRestart
    );
#define NEXUS_SimpleAudioDecoder_SwapServerSettings NEXUS_SimpleAudioDecoder_MoveServerSettings

void NEXUS_SimpleAudioDecoderModule_LoadDefaultSettings(
    NEXUS_AudioDecoderHandle audio
    );

void NEXUS_SimpleAudioDecoder_GetStcIndex(
    NEXUS_SimpleAudioDecoderServerHandle server,
    NEXUS_SimpleAudioDecoderHandle handle,
    int *pStcIndex /* returns -1 if StcChannel is unused */
    );

void NEXUS_SimpleAudioDecoder_SetStcIndex(
    NEXUS_SimpleAudioDecoderServerHandle server,
    NEXUS_SimpleAudioDecoderHandle handle,
    int stcIndex
    );

#ifdef __cplusplus
}
#endif

#endif
