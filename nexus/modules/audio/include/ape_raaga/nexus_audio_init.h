/***************************************************************************
 *     (c)2007-2013 Broadcom Corporation
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
 *      Audio Initialization Routines
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 **************************************************************************/
#ifndef NEXUS_AUDIO_INIT_H__
#define NEXUS_AUDIO_INIT_H__

#include "nexus_types.h"
#include "nexus_audio.h"
#include "nexus_audio_output.h"
#include "nexus_audio_dsp.h"

#ifdef __cplusplus
extern "C" {
#endif

/*************************************************************************** 
Summary: 
    Nexus audio module version
***************************************************************************/
#define NEXUS_AUDIO_MODULE_FAMILY (NEXUS_AUDIO_MODULE_FAMILY_APE_RAAGA)

/***************************************************************************
Summary:
Audio Module Settings
**************************************************************************/
typedef struct NEXUS_AudioModuleSettings
{
    struct 
    {
        NEXUS_ModuleHandle transport;
        NEXUS_ModuleHandle hdmiInput;   /* Only required for platforms that support HDMI input */
        NEXUS_ModuleHandle hdmiOutput;  /* Only required for platforms that support HDMI output */
        NEXUS_ModuleHandle rfm;         /* Only required for platforms that support RFM */
        NEXUS_ModuleHandle frontend;    /* Only required for platforms that support RfAudioDecoder */
        NEXUS_ModuleHandle surface; 
        NEXUS_ModuleHandle core;        /* Handle to Core module. See NEXUS_Core_Init. */
        NEXUS_ModuleHandle security;
    } modules;

    unsigned maxAudioDspTasks;          /* Maximum number of audio tasks that will run on the DSP concurrently */
    unsigned maxIndependentDelay;       /* Maximum output delay value in ms.  */
    unsigned maxPcmSampleRate;          /* Maximum PCM output sample rate in Hz. */

    bool watchdogEnabled;               /* If true, watchdog recovery is enabled */
    bool independentDelay;              /* If true, independent delay is enabled */
    bool routeInputsToDsp;              /* If true, external inputs such as I2S, AnalogAudioDecoder, or RfAudioDecoder will
                                           route to the DSP if the input's Start() routine is called.  Otherwise, the input 
                                           will route to the audio hardware and bypass the DSP.  This defaults to true
                                           for DTV platforms and false for STB platforms.  If false, you can still route inputs
                                           to the DSP by passing the connector to NEXUS_AudioDecoder_Start(). */
    unsigned numPcmBuffers;             /* Number of PCM audio buffers required. */
    unsigned numCompressedBuffers;      /* Number of discrete compressed decoder output buffers required.  This is the
                                           number of outputs that will receive discrete compressed content running at up to 48kHz.
                                           Independent delay does not affect this number, only different content sources. */ 
    unsigned numCompressed4xBuffers;    /* Number of discrete compressed decoder output buffers required.  This is the
                                           number of outputs that will receive discrete compressed content running at up to 192kHz.
                                           Independent delay does not affect this number, only different content sources. */ 
    unsigned numCompressed16xBuffers;   /* Number of discrete compressed decoder output buffers required.  This is the
                                           number of outputs that will receive discrete compressed content running at up to 768kHz (HDMI HBR).
                                           Independent delay does not affect this number, only different content sources. */ 
    unsigned numRfEncodedPcmBuffers;    /* Number of discrete RF encoded PCM buffers required.  This is the
                                           number of outputs that will receive discrete RF encoded PCM content running at up to 192kHz (BTSC).
                                           Independent delay does not affect this number, only different content sources. */     

    NEXUS_AudioOutputPll defaultPll;    /* Default PLL for outputs.  Default=0.  */

    unsigned heapIndex;                 /* Optional.  If set, the audio buffers will be allocated in this heap by default. */
    unsigned firmwareHeapIndex;         /* Optional.  If set, the DSP Firmware images will be loaded in this heap. */

    NEXUS_AudioDspDebugSettings dspDebugSettings;   /* DSP Debug settings */

    NEXUS_AudioDspAlgorithmSettings dspAlgorithmSettings; /* DSP Algorithm settings */

    NEXUS_AudioLoudnessEquivalenceMode loudnessMode;    /* Loudness equivalence mode.  Default is 
                                                           NEXUS_AudioLoudnessEquivalenceMode_eNone */
    bool allowSpdif4xCompressed;        /* allow 61937x4 mode over SPDIF interface - used for special applications, many AVRs
                                            do not support this mode */
    bool allowI2sCompressed;            /* allow 61937 and 61937x4 mode over i2s interface - used for special applications,
                                            typically sending to another chip/dsp/fpga etc.  This should be disabled
                                            if i2s is connected to an external DAC */
} NEXUS_AudioModuleSettings;

struct NEXUS_Core_PreInitState;

void NEXUS_AudioModule_GetDefaultSettings(
    const struct NEXUS_Core_PreInitState *preInitState,
    NEXUS_AudioModuleSettings *pSettings    /* [out] */
    );

/**
Summary:
Initialize the audio module

Description:
This function is called by NEXUS_Platform_Init, not by applications.
If you want to modify these settings from your application, you can do this 
through NEXUS_PlatformSettings as follows:

    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.audioModuleSettings.xxx = xxx;
    NEXUS_Platform_Init(&platformSettings);

**/
NEXUS_ModuleHandle NEXUS_AudioModule_Init(
    const NEXUS_AudioModuleSettings *pSettings  /* NULL will use default settings */
    );

/**
Summary:
Un-Initialize the audio module
**/
void NEXUS_AudioModule_Uninit(void);

/***************************************************************************
Summary:
Audio Module Usage Settings
**************************************************************************/
typedef struct NEXUS_AudioModuleUsageSettings
{
    unsigned maxIndependentDelay;                     /* max independent output delay [0-500] milliseconds.
                                                         This value affects the size of the decoder's output buffers. */
    unsigned maxDecoderOutputChannels;                /* max decoder output channel count. specify 6 for 5.1 ch, 8 for 7.1 ch.
                                                         This value affects the size of the decoder's output buffers. */
    unsigned maxDecoderOutputSamplerate;              /* max decoder output samplerate [48000, 96000] Hz.
                                                         This value affects the size of the decoder's output buffers. */

    NEXUS_AudioDolbyCodecVersion dolbyCodecVersion;   /* several versions of Dolby codecs (AC3, AAC, DDP) exist on our
                                                         system, but only one type is used in a given product, specified
                                                         by compile time defines */

    bool decodeCodecEnabled [NEXUS_AudioCodec_eMax];  /* specify true for each decoder codec that will be enabled */
    bool encodeCodecEnabled [NEXUS_AudioCodec_eMax];  /* specify true for each encoder codec that will be enabled */
    bool postProcessingEnabled[NEXUS_AudioPostProcessing_eMax]; /* specify true for each post processing algo that will be enabled */
    unsigned numDecoders;                             /* specify the number of parallel audio decoders required.
                                                         Default will be NEXUS_NUM_AUDIO_DECODERS */
    unsigned numEncoders;                             /* specify the number of parallel audio encoders required.
                                                         Default will be NEXUS_NUM_VIDEO_ENCODERS */
    unsigned numPassthroughDecoders;                  /* number of IEC-61937 passthrough that will be supported over SPDIF and/or HDMI. */
    unsigned numHbrPassthroughDecoders;               /* number of HBR passthroughs enabled (Dolby TrueHD and/or
                                                         DTS HD MA passthrough over HDMI) */

    unsigned numDspMixers;                            /* specify the number of DSP mixers the system will use.
                                                         Examples of usage cases that require a dsp mixer - MS10/11 mixing, transcode.
                                                         Each transcode requires one dsp mixer.
                                                         Each MS10/11 instance requires one dsp mixer.
                                                         Ex: MS11 + 4 transcodes, where one transcode sources
                                                         the MS11 path would require 4 dsp mixers */
    unsigned numPostProcessing;                       /* specify the worst case number of post processing
                                                         stages that will run simultaneously */
    unsigned numEchoCancellers;                       /* specify the number of echo cancellers required */
} NEXUS_AudioModuleUsageSettings;

/***************************************************************************
Summary:
Audio Module Memory Estimate
**************************************************************************/
typedef struct NEXUS_AudioModuleMemoryEstimate
{
    struct {
        unsigned general; /* bytes allocated */
    } memc[NEXUS_MAX_MEMC];
} NEXUS_AudioModuleMemoryEstimate;

/**
Summary:
Get Default Usage Settings
**/
void NEXUS_AudioModule_GetDefaultUsageSettings(
    NEXUS_AudioModuleUsageSettings *pSettings   /* [out] */
    );

/**
Summary:
Get Memory Estimate

Description:
Get an estimated amount of memory required for specified usage
cases.
**/
NEXUS_Error NEXUS_AudioModule_GetMemoryEstimate(
    const struct NEXUS_Core_PreInitState *preInitState,
    const NEXUS_AudioModuleUsageSettings *pSettings,
    NEXUS_AudioModuleMemoryEstimate *pEstimate  /* [out] */
    );

#ifdef __cplusplus
}
#endif

#endif /* #ifndef NEXUS_AUDIO_INIT_H__ */

