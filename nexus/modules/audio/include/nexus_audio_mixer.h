/******************************************************************************
 * Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 *****************************************************************************/
#ifndef NEXUS_AUDIO_MIXER_H__
#define NEXUS_AUDIO_MIXER_H__

#include "nexus_types.h"
#include "nexus_audio_types.h"
#include "blst_slist.h"
#include "blst_queue.h"

#ifdef __cplusplus
extern "C" {
#endif

/*=************************************
Interface: AudioMixer

Header file: nexus_audio_mixer.h

Module: Audio

Description: Mixer two or more PCM NEXUS_AudioInputs into a single NEXUS_AudioOutput.

**************************************/

/**
Summary:
Handle for an Audio Mixer
**/
typedef struct NEXUS_AudioMixer *NEXUS_AudioMixerHandle;

typedef struct NEXUS_AudioMixerDolbySettings
{
    /* Applies to MS10+ */
    int multiStreamBalance;     /* If this is a DSP mixer and you are mixing multi-stream content, this
                                   value controls the relative volume between the main and associated
                                   audio content.  0 means no preference.  Positive values up to 32
                                   favor associated audio in dB steps.  32 will mute main audio.  Negative
                                   values down to -32 favor main audio in dB steps.  -32 will mute the
                                   associated audio. */

    /* The following fields apply MS12 mixing only */
    /* Start Dolby MS12 DAP features */
    unsigned certificationMode;         /* Dolby Certification mode - default is 0 */
    bool enablePostProcessing;          /* If enabled, DAPv2 processing will be enabled - default is false [OFF] */
    struct {
        bool enableVolumeLimiting;      /* Enables Dolby Volume Limiting. Must be set to true for the controls below
                                           to be honored. */
        bool enableIntelligentLoudness; /* Enables Intelligent Loudness Support.
                                           In most cases this should be set to true, allowing the
                                           DAP Mixer to regulate the amount of limiting. However, certain
                                           products will be required to customize volumeLimiterAmount and
                                           this field must be set to false in those cases. Default is true. */
        unsigned volumeLimiterAmount;   /* Controls the amount/profile of Volume Limiting. This control is only
                                           honored if enableIntelligentLoudness is set to false.
                                           Valid values 0-10. See "Dolby MS12 Multistream Decoder System Development Manual"
                                           for more information. Default is 0 */
    } volumeLimiter;
    struct {
        bool enableDialogEnhancer;      /* Enables Dolby Dialog Enhancer. Must be set to true for the controls below
                                           to be honored. */
        unsigned dialogEnhancerLevel;   /* Valid values 0-16. Each integer step is 0.0625dB of boost. Default is 0 */
        unsigned contentSuppressionLevel;/* Valid values 0-16. Each integer step is 0.0625dB of cut. Default is 0 */
    } dialogEnhancer;
    struct {
        bool enabled;                   /* Enables Dolby Intelligent EQ. Must be set to true for the controls below
                                           to be honored. */
        unsigned numBands;              /* Application specifies the number of EQ Band Entries. Valid values [1 - 20] */
        struct {
            unsigned frequency;         /* Center frequency of each eq band, specified in Hz. Valid values [20 - 20,000] */
            int gain;                   /* Gain in 0.0625 dB steps, achieving +/- 30dB of gain. Valid values [-480 - 480] */
        } band [20];
    } intelligentEqualizer;
    struct {
        bool continuousFading;          /* When true, fade operation will proceed even in the absence of active input data.
                                           Default is false. */
    } fade;
    /* End Dolby MS12 DAP features */
} NEXUS_AudioMixerDolbySettings;

/***************************************************************************
Summary:
Audio Mixer Settings
***************************************************************************/
typedef struct NEXUS_AudioMixerSettings
{
    bool mixUsingDsp;           /* If true, mix contents in the DSP as opposed to the mixer hardware.
                                   This requires the masterInput field to be set below prior to starting
                                   any input.  This field should be set prior to NEXUS_AudioMixer_Open
                                   and not changed on the fly. */

    bool intermediate;          /* if intermediate is true, an "intermediate" HW mixer will be created,
                                   to support custom routing through a HW mixer. This allows for special
                                   usage cases such as decoder/playback->hwMixer->loopback->dsp.
                                   This field is only valid for HW mixers (ignored if mixUsingDsp=true) */

    int multiStreamBalance;     /* If this is a DSP mixer and you are mixing multi-stream content, this
                                   value controls the relative volume between the main and associated
                                   audio content.  0 means no preference.  Positive values up to 32
                                   favor associated audio in dB steps.  32 will mute main audio.  Negative
                                   values down to -32 favor main audio in dB steps.  -32 will mute the
                                   associated audio. */

    unsigned dspIndex;          /* If mixUsingDsp is true, this selects the DSP to use.  When mixing decoder
                                   inputs from the DSP, all inputs must run on the same DSP as the mixer.
                                   This is not changeable on the fly. */

    NEXUS_AudioInputHandle master;/* This field will determine what input is deemed to be the master for mixing
                                   purposes.  The master will determine the output timing, and if mixing multi-stream
                                   content such as audio descriptors it will determine the primary trach as opposed to
                                   the description track. */

    unsigned outputSampleRate;  /* Output sample rate in Hz.  If 0 (default) the output rate will
                                   be automatically selected based on the input sample rate.  This will
                                   override the sample rate of a master input if specified in
                                   NEXUS_AudioMixerSettings.master */

    int32_t outputVolume[NEXUS_AudioChannel_eMax]; /* Output volume to be applied to downstream connections.
                                                      This control is valid ONLY for mixers with intermediate=true.
                                                      Valid values are NEXUS_AUDIO_VOLUME_LINEAR_MIN - NEXUS_AUDIO_VOLUME_LINEAR_MAX.
                                                      Default value is NEXUS_AUDIO_VOLUME_LINEAR_NORMAL (no attenuation or boost) */
    bool outputMuted;           /* Output mute control to be applied to downstream connections.
                                   This control is valid ONLY for mixers with intermediate=true */

    NEXUS_AudioFadeSettings mainDecodeFade; /* Fade control for main decode output mixer stage (Main+Associate or Dual Main)
                                               -- only supported for MS12 DSP Mixers */

    int32_t loopbackVolumeMatrix[NEXUS_AudioChannel_eMax][NEXUS_AudioChannel_eMax]; /* If this is a DSP mixer, and we have one
                                           or more PcmPlayback/FMM (loopback) inputs, apply these mixing coefficients.
                                           This can be used to mix across channel pairs, including subtraction.
                                           Common use case would be to map stereo into multichannel in a variety
                                           of different ways. For example, if content is mono, choose where that should
                                           be routed into the multichannel domain.
                                           Some example matrices:
                                           1. L->L, R->R (default)
                                              coeff[i][i] = NEXUS_AUDIO_VOLUME_LINEAR_NORMAL, zero all others
                                           2. L->C
                                              coeff[NEXUS_AudioChannel_eCenter][NEXUS_AudioChannel_eLeft] = NEXUS_AUDIO_VOLUME_LINEAR_NORMAL
                                           3. (L/2+R/2)->C, L-3(L+R)/8, R-3(L+R)/8 (upmix L and R to center, subtract 3/8 from L and R)
                                              coeff[NEXUS_AudioChannel_eLeft][NEXUS_AudioChannel_eLeft] = NEXUS_AUDIO_VOLUME_LINEAR_NORMAL - 3*NEXUS_AUDIO_VOLUME_LINEAR_NORMAL/8
                                              coeff[NEXUS_AudioChannel_eLeft][NEXUS_AudioChannel_eRight] = -3*NEXUS_AUDIO_VOLUME_LINEAR_NORMAL/8
                                              coeff[NEXUS_AudioChannel_eRight][NEXUS_AudioChannel_eLeft] = -3*NEXUS_AUDIO_VOLUME_LINEAR_NORMAL/8
                                              coeff[NEXUS_AudioChannel_eRight][NEXUS_AudioChannel_eRight] = NEXUS_AUDIO_VOLUME_LINEAR_NORMAL - 3*NEXUS_AUDIO_VOLUME_LINEAR_NORMAL/8
                                              coeff[NEXUS_AudioChannel_eCenter][NEXUS_AudioChannel_eLeft] = NEXUS_AUDIO_VOLUME_LINEAR_NORMAL/2
                                              coeff[NEXUS_AudioChannel_eCenter][NEXUS_AudioChannel_eRight] = NEXUS_AUDIO_VOLUME_LINEAR_NORMAL/2
                                              */

    bool loopbackMixerEnabled;          /* When set to true[Default], for DspMixers, we will create a stereo loopback mixer when
                                           required and aggregate any FMM source inputs into a single FMM path for the
                                           DSP to consume. If false, no such mixer will be created and only ONE FMM source
                                           will be allowed into the DspMixer. */

    NEXUS_AudioMixerDolbySettings dolby; /* Dolby Specific settings for Multi-Stream Mixing */

    /* Hardware Mixer controls (mixUsingDsp = false) */
    bool fixedOutputFormatEnabled; /* When set to false, Mixers adapt their output format to meet the needs of the inputs.
                                      In this mode, the output format will change as needed depending on the inputs
                                      connected. When set to true, the fixed output format below will be enforced,
                                      enabling upmixing or downmixing if needed. Default = false. */
    NEXUS_AudioMultichannelFormat fixedOutputFormat; /* Takes effect if fixedOutputFormatEnabled = true.
                                                        Currently only upmixing is supported, so the only valid values are
                                                        NEXUS_AudioMultichannelFormat_e5_1, NEXUS_AudioMultichannelFormat_e7_1 */
} NEXUS_AudioMixerSettings;

/***************************************************************************
Summary:
Get default settings for an audio mixer
***************************************************************************/
void NEXUS_AudioMixer_GetDefaultSettings(
    NEXUS_AudioMixerSettings *pSettings    /* [out] Default Settings */
    );

/***************************************************************************
Summary:
Open an audio mixer
***************************************************************************/
NEXUS_AudioMixerHandle NEXUS_AudioMixer_Open( /* attr{destructor=NEXUS_AudioMixer_Close}  */
    const NEXUS_AudioMixerSettings *pSettings /* attr{null_allowed=y} */
    );

/***************************************************************************
Summary:
Close an audio mixer
***************************************************************************/
void NEXUS_AudioMixer_Close(
    NEXUS_AudioMixerHandle handle
    );

/***************************************************************************
Summary:
Get current settings for an audio mixer
***************************************************************************/
void NEXUS_AudioMixer_GetSettings(
    NEXUS_AudioMixerHandle handle,
    NEXUS_AudioMixerSettings *pSettings      /* [out] Mixer Settings */
    );

/***************************************************************************
Summary:
Set settings of an audio mixer
***************************************************************************/
NEXUS_Error NEXUS_AudioMixer_SetSettings(
    NEXUS_AudioMixerHandle handle,
    const NEXUS_AudioMixerSettings *pSettings    /* Mixer Settings */
    );

/***************************************************************************
Summary:
Start a mixer

Description:
This call is optional.  By default, mixers will automatically start when
The first input starts, but if you want to explicitly enable the mixer
earlier call this function prior to starting any inputs.   Currently, this
is only supported if mixUsingDsp is set to true.
***************************************************************************/
NEXUS_Error NEXUS_AudioMixer_Start(
    NEXUS_AudioMixerHandle handle
    );

/***************************************************************************
Summary:
Stop a mixer

Description:
This call is required only if you call NEXUS_AudioMixer_Start().  By default,
mixers will automatically stop when the last input stops, but if you
have explicitly started the mixer via NEXUS_AudioMixer_Start() you must call
this routine to stop it after all inputs have stopped.
***************************************************************************/
void NEXUS_AudioMixer_Stop(
    NEXUS_AudioMixerHandle handle
    );

/***************************************************************************
Summary:
Add an audio input to a mixer
***************************************************************************/
NEXUS_Error NEXUS_AudioMixer_AddInput(
    NEXUS_AudioMixerHandle handle,
    NEXUS_AudioInputHandle input
    );

/***************************************************************************
Summary:
Remove an audio input from a mixer
***************************************************************************/
NEXUS_Error NEXUS_AudioMixer_RemoveInput(
    NEXUS_AudioMixerHandle handle,
    NEXUS_AudioInputHandle input
    );

/***************************************************************************
Summary:
Remove all audio inputs from a mixer
***************************************************************************/
NEXUS_Error NEXUS_AudioMixer_RemoveAllInputs(
    NEXUS_AudioMixerHandle handle
    );

/***************************************************************************
Summary:
Get the audio input connector for connection to outputs or post-processing
***************************************************************************/
NEXUS_AudioInputHandle NEXUS_AudioMixer_GetConnector(
    NEXUS_AudioMixerHandle mixer
    );

/***************************************************************************
Summary:
Get Mixer Input Settings
***************************************************************************/
NEXUS_Error NEXUS_AudioMixer_GetInputSettings(
    NEXUS_AudioMixerHandle handle,
    NEXUS_AudioInputHandle input,
    NEXUS_AudioMixerInputSettings *pSettings       /* [out] Mixer Input Settings */
    );

/***************************************************************************
Summary:
Set Mixer Input Settings
***************************************************************************/
NEXUS_Error NEXUS_AudioMixer_SetInputSettings(
    NEXUS_AudioMixerHandle handle,
    NEXUS_AudioInputHandle input,
    const NEXUS_AudioMixerInputSettings *pSettings
    );

/***************************************************************************
Summary:
Get Status of Mixer Input
***************************************************************************/
NEXUS_Error NEXUS_AudioMixer_GetInputStatus(
    NEXUS_AudioMixerHandle handle,
    NEXUS_AudioInputHandle input,
    NEXUS_AudioMixerInputStatus *pStatus       /* [out] Mixer Input Status */
    );

/***************************************************************************
Summary:
Get Status of Mixer
***************************************************************************/
NEXUS_Error NEXUS_AudioMixer_GetStatus(
    NEXUS_AudioMixerHandle handle,
    NEXUS_AudioMixerStatus *pStatus      /* [out] Mixer Status */
    );

#ifdef __cplusplus
}
#endif

#endif /* #ifndef NEXUS_AUDIO_MIXER_H__ */
