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
#ifndef NEXUS_SIMPLE_AUDIO_PRIV_H__
#define NEXUS_SIMPLE_AUDIO_PRIV_H__

#include "nexus_audio_decoder_primer.h"
#if NEXUS_HAS_HDMI_INPUT
#include "nexus_audio_input_capture.h"
#endif

#ifdef __cplusplus
#error
#endif

struct NEXUS_SimpleAudioDecoder_P_CodecSettings {
    NEXUS_AudioDecoderCodecSettings codecSettings[NEXUS_AudioCodec_eMax];
};

struct NEXUS_SimpleAudioDecoder_P_CodecSettingsDecoder {
    struct NEXUS_SimpleAudioDecoder_P_CodecSettings primary;
    struct NEXUS_SimpleAudioDecoder_P_CodecSettings secondary;
    struct NEXUS_SimpleAudioDecoder_P_CodecSettings description;
};

enum nexus_simpleaudiodecoder_state {
    state_stopped,
    state_started,
    state_suspended,
    state_priming,
    state_max
};

struct NEXUS_SimpleAudioDecoder
{
    NEXUS_OBJECT(NEXUS_SimpleAudioDecoder);
    BLST_S_ENTRY(NEXUS_SimpleAudioDecoder) link;
    unsigned index;
    bool acquired;
    struct {
        enum nexus_simpleaudiodecoder_state state;
        NEXUS_AudioDecoderPrimerHandle primer;
        NEXUS_AudioProcessorHandle processor[NEXUS_AudioPostProcessing_eMax];
        NEXUS_AudioProcessorSettings processorSettings[NEXUS_AudioPostProcessing_eMax];
    } decoders[NEXUS_SimpleAudioDecoderSelector_eMax], playback;
    bool clientStarted;
    bool suspended;
    NEXUS_SimpleAudioDecoderServerSettings serverSettings;
    NEXUS_SimpleAudioDecoderSettings settings;
    struct NEXUS_SimpleAudioDecoder_P_CodecSettingsDecoder codecSettings;
    NEXUS_SimpleAudioDecoderStartSettings startSettings;
    NEXUS_AudioDecoderTrickState trickState;
    NEXUS_AudioInput currentSpdifInput, currentHdmiInput;
    bool currentSpdifInputCompressed, currentHdmiInputCompressed;
    NEXUS_SimpleStcChannelHandle stcChannel;

    struct {
        NEXUS_SimpleEncoderHandle handle;
        NEXUS_AudioEncoderHandle audioEncoder;
        bool passthrough;
        NEXUS_AudioCodec codec;
        unsigned sampleRate;
        NEXUS_AudioMuxOutputHandle muxOutput;
        bool displayEncode;
        NEXUS_AudioMixerHandle audioMixer;
    } encoder;
    struct {
        NEXUS_SimpleAudioDecoderHandle master, slave;
    } displayEncode; /* never swapped */
    struct {
        NEXUS_HdmiInputHandle handle;
#if NEXUS_HAS_HDMI_INPUT
        NEXUS_AudioInputCaptureHandle inputCapture;
#endif
    } hdmiInput;
    struct {
        NEXUS_AudioMixerHandle stereo, multichannel;
        bool suspended;
    } mixers;
};

BDBG_OBJECT_ID_DECLARE(NEXUS_SimpleAudioPlayback);
struct NEXUS_SimpleAudioPlayback
{
    NEXUS_OBJECT(NEXUS_SimpleAudioPlayback);
    BLST_S_ENTRY(NEXUS_SimpleAudioPlayback) link; /* global list */
    BLST_S_ENTRY(NEXUS_SimpleAudioPlayback) decoderLink; /* per decoder list */
    unsigned index;
    bool acquired;
    bool started;
    bool clientStarted;
    bool suspended;
    NEXUS_SimpleAudioPlaybackServerSettings serverSettings;
    NEXUS_SimpleAudioPlaybackSettings settings;
    NEXUS_SimpleAudioPlaybackStartSettings startSettings;
};

void nexus_simpleaudiodecoder_p_add_playback(NEXUS_SimpleAudioPlaybackHandle audioPlayback);
void nexus_simpleaudiodecoder_p_remove_playback(NEXUS_SimpleAudioPlaybackHandle audioPlayback);

NEXUS_Error nexus_simpleaudioplayback_p_internal_start(NEXUS_SimpleAudioPlaybackHandle handle);
void nexus_simpleaudioplayback_p_internal_stop(NEXUS_SimpleAudioPlaybackHandle handle);
NEXUS_Error nexus_simpleaudioplayback_p_suspend(NEXUS_SimpleAudioPlaybackHandle handle);
void nexus_simpleaudioplayback_p_resume(NEXUS_SimpleAudioPlaybackHandle handle);

NEXUS_Error nexus_simpleaudiodecoder_p_add_encoder(NEXUS_SimpleAudioDecoderHandle handle, NEXUS_SimpleAudioDecoderHandle slave,
    NEXUS_AudioMuxOutputHandle muxOutput, NEXUS_AudioCodec codec, bool passthrough, unsigned sampleRate,
    NEXUS_SimpleEncoderHandle encoder, NEXUS_AudioMixerHandle audioMixer,
    bool displayEncode);
void nexus_simpleaudiodecoder_p_remove_encoder(NEXUS_SimpleAudioDecoderHandle handle);

#endif
