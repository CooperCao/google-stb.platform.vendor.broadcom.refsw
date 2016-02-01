/***************************************************************************
 *     (c)2010-2013 Broadcom Corporation
 *
 *  This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
 **************************************************************************/
#ifndef NEXUS_SIMPLE_AUDIO_DECODER_SERVER_H__
#define NEXUS_SIMPLE_AUDIO_DECODER_SERVER_H__

#include "nexus_types.h"
#include "nexus_simple_audio_decoder.h"
#include "nexus_simple_audio_playback.h"
#include "nexus_simple_decoder_types.h"
#include "nexus_stc_channel.h"
#if NEXUS_HAS_AUDIO
#include "nexus_audio_decoder.h"
#include "nexus_spdif_output.h"
#include "nexus_audio_playback.h"
#include "nexus_i2s_input.h"
#else
#include "../../audio/include/nexus_audio_decoder.h"
#include "../../audio/include/nexus_spdif_output.h"
#include "../../audio/include/nexus_audio_playback.h"
#include "../../audio/include/nexus_i2s_input.h"
#endif
#if NEXUS_HAS_HDMI_OUTPUT
#include "nexus_hdmi_output.h"
#else
typedef void *NEXUS_HdmiOutputHandle;
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
This server-side is semi-private. In multi-process systems, only server apps like nxserver will call it.
Client apps will not call it. Therefore, this API is subject to non-backward compatible change.
**/

#define NEXUS_MAX_SIMPLE_DECODER_SPDIF_OUTPUTS 2
#define NEXUS_MAX_SIMPLE_DECODER_HDMI_OUTPUTS 2

typedef struct NEXUS_SimpleAudioDecoderServerSettings
{
    bool enabled;
    NEXUS_SimpleDecoderDisableMode disableMode;
    NEXUS_AudioDecoderHandle primary;   /* for decode and simul */
    NEXUS_AudioDecoderHandle secondary; /* for compressed passthrough */
    NEXUS_AudioDecoderHandle description; /* for audio description */
    NEXUS_AudioPlaybackHandle passthroughPlayback; /* For passthroughBuffer mode */

    struct {
        NEXUS_AudioMixerHandle stereo, multichannel;
    } mixers;

    NEXUS_AudioConnectorType syncConnector; /* Connector to use for Sync Channel */
    
    struct {
        NEXUS_SpdifOutputHandle outputs[NEXUS_MAX_SIMPLE_DECODER_SPDIF_OUTPUTS];
        NEXUS_AudioInput input[NEXUS_AudioCodec_eMax]; /* per codec, specify the final stage to be connected to spdif. 
            use NEXUS_AudioCodec_eUnknown to specify default configuration for playback-only. */
    } spdif;
    struct {
        NEXUS_HdmiOutputHandle outputs[NEXUS_MAX_SIMPLE_DECODER_HDMI_OUTPUTS];
        NEXUS_AudioInput input[NEXUS_AudioCodec_eMax]; /* per codec, specify the final stage to be connected to hdmi. 
            use NEXUS_AudioCodec_eUnknown to specify default configuration for playback-only. */
    } hdmi;
    int stcIndex; /* used for allocating stc channel once connected to simple stc channel */
} NEXUS_SimpleAudioDecoderServerSettings;

void NEXUS_SimpleAudioDecoder_GetDefaultServerSettings(
    NEXUS_SimpleAudioDecoderServerSettings *pSettings /* [out] */
    );

NEXUS_SimpleAudioDecoderHandle NEXUS_SimpleAudioDecoder_Create( /* attr{destructor=NEXUS_SimpleAudioDecoder_Destroy}  */
    unsigned index,
    const NEXUS_SimpleAudioDecoderServerSettings *pSettings /* attr{null_allowed=y} */
    );

void NEXUS_SimpleAudioDecoder_Destroy(
    NEXUS_SimpleAudioDecoderHandle handle
    );

void NEXUS_SimpleAudioDecoder_GetServerSettings(
    NEXUS_SimpleAudioDecoderHandle handle,
    NEXUS_SimpleAudioDecoderServerSettings *pSettings /* [out] */
    );

NEXUS_Error NEXUS_SimpleAudioDecoder_SetServerSettings(
    NEXUS_SimpleAudioDecoderHandle handle,
    const NEXUS_SimpleAudioDecoderServerSettings *pSettings
    );

typedef struct NEXUS_SimpleAudioPlaybackServerSettings
{
    NEXUS_SimpleAudioDecoderHandle decoder;
    NEXUS_AudioPlaybackHandle playback;
    NEXUS_I2sInputHandle i2sInput;

    struct {
        bool enabled;
        NEXUS_SpdifOutputHandle spdifOutputs[NEXUS_MAX_SIMPLE_DECODER_SPDIF_OUTPUTS];
        NEXUS_HdmiOutputHandle hdmiOutputs[NEXUS_MAX_SIMPLE_DECODER_HDMI_OUTPUTS];
    } compressed;
} NEXUS_SimpleAudioPlaybackServerSettings;

void NEXUS_SimpleAudioPlayback_GetDefaultServerSettings(
    NEXUS_SimpleAudioPlaybackServerSettings *pSettings /* [out] */
    );
    
NEXUS_SimpleAudioPlaybackHandle NEXUS_SimpleAudioPlayback_Create( /* attr{destructor=NEXUS_SimpleAudioPlayback_Destroy}  */
    unsigned index,
    const NEXUS_SimpleAudioPlaybackServerSettings *pSettings /* attr{null_allowed=y} */
    );

void NEXUS_SimpleAudioPlayback_Destroy(
    NEXUS_SimpleAudioPlaybackHandle handle
    );
    
void NEXUS_SimpleAudioPlayback_GetServerSettings(
    NEXUS_SimpleAudioPlaybackHandle handle,
    NEXUS_SimpleAudioPlaybackServerSettings *pSettings /* [out] */
    );

NEXUS_Error NEXUS_SimpleAudioPlayback_SetServerSettings(
    NEXUS_SimpleAudioPlaybackHandle handle,
    const NEXUS_SimpleAudioPlaybackServerSettings *pSettings
    );    
    

NEXUS_Error NEXUS_SimpleAudioDecoder_SwapServerSettings( 
    NEXUS_SimpleAudioDecoderHandle decoder1, 
    NEXUS_SimpleAudioDecoderHandle decoder2
    );

void NEXUS_SimpleAudioDecoderModule_LoadDefaultSettings(
    NEXUS_AudioDecoderHandle audio
    );

void NEXUS_SimpleAudioDecoder_GetStcIndex(
    NEXUS_SimpleAudioDecoderHandle handle,
    int *pStcIndex /* returns -1 if StcChannel is unused */
    );

#ifdef __cplusplus
}
#endif

#endif
