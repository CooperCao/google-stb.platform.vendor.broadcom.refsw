/***************************************************************************
 *  Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 *
 * Module Description:
 *      Audio Module Global Routines
 *
 **************************************************************************/
#ifndef NEXUS_AUDIO_H__
#define NEXUS_AUDIO_H__

#include "nexus_types.h"
#include "nexus_audio_output.h"
#include "nexus_audio_dsp.h"

#ifdef __cplusplus
extern "C" {
#endif

/*************************************************************************** 
Summary: 
    Nexus audio module versions 
***************************************************************************/
#define NEXUS_AUDIO_MODULE_FAMILY_APE_RAAGA (7422)
#define NEXUS_AUDIO_MODULE_FAMILY_APE_MIPS  (7408)
#define NEXUS_AUDIO_MODULE_FAMILY_RAP_V3    (7405)
#define NEXUS_AUDIO_MODULE_FAMILY_RAP_V1    (7400)

/***************************************************************************
Summary: 
Audio hardware ramp step values. 
 
Description: 
When a hardware block is ramping from one volume value to another, it 
will change the linear volume value by the ramp step size on each sample. 
***************************************************************************/
typedef struct NEXUS_AudioRampStepSettings
{
    unsigned int mixerRampStep;
    unsigned int srcRampStep;
} NEXUS_AudioRampStepSettings;

/***************************************************************************
Summary: 
Get the audio hardware ramp step values 
***************************************************************************/
void NEXUS_AudioModule_GetRampStepSettings(
    NEXUS_AudioRampStepSettings *pRampStepSettings      /* [out] ramping step size for scale change for all output ports */
    );

/***************************************************************************
Summary: 
Set the audio hardware ramp step values 
***************************************************************************/
NEXUS_Error NEXUS_AudioModule_SetRampStepSettings(
    const NEXUS_AudioRampStepSettings *pRampStepSettings  /* ramping step size for scale change for all output ports */
    );

/***************************************************************************
Summary:
External MCLK rates.  Specified as a multiple of the sampling frequency 
the PLL is programmed for. 
***************************************************************************/
typedef enum NEXUS_ExternalMclkRate
{
    NEXUS_ExternalMclkRate_e128Fs,
    NEXUS_ExternalMclkRate_e256Fs,
    NEXUS_ExternalMclkRate_e384Fs,
    NEXUS_ExternalMclkRate_e512Fs,
    NEXUS_ExternalMclkRate_eMax
} NEXUS_ExternalMclkRate;

/***************************************************************************
Summary:
Enable an external MCLK at the rate specified.  
***************************************************************************/
NEXUS_Error NEXUS_AudioModule_EnableExternalMclk(
    unsigned mclkIndex,                 /* Which external MCLK to enable.  For chips with 1 external 
                                           MCLK line, this should be specified as 0.  For chips with
                                           more than one MCLK signal/pin, this ranges from 0 to the
                                           number of MCLK signals/pins - 1 */
    NEXUS_AudioOutputPll pll,           /* The PLL that will drive this MCLK */
    NEXUS_ExternalMclkRate mclkRate     /* The multiplier of the sample rate to be transmitted.  For
                                           example, if 128Fs is specified, the output clock rate will be 
                                           128 * sample_rate_in_hz */
    );

/***************************************************************************
Summary:
Audio PLL Modes
***************************************************************************/
 typedef enum NEXUS_AudioOutputPllMode
 {
     NEXUS_AudioOutputPllMode_eVcxo,           /* The input source is a VCXO Rate Manager */
     NEXUS_AudioOutputPllMode_eCustom,         /* Custom mode  */
     NEXUS_AudioOutputPllMode_eMax
 } NEXUS_AudioOutputPllMode;

/***************************************************************************
Summary:
Audio PLL Settings
***************************************************************************/
typedef struct NEXUS_AudioOutputPllSettings
{
    NEXUS_AudioOutputPllMode mode;
     struct
     {
         struct
         {
             NEXUS_Vcxo vcxo;    /* Specifies the VCXO-PLL driving this Audio PLL. */
         } vcxo;
         struct
         {
             unsigned value[2];  /* not for general use */
         } custom;
    } modeSettings;
} NEXUS_AudioOutputPllSettings;   
    
/***************************************************************************
Summary:
Get Audio PLL Settings
***************************************************************************/
void NEXUS_AudioOutputPll_GetSettings(
    NEXUS_AudioOutputPll pll,
    NEXUS_AudioOutputPllSettings *pSettings       /* [out] Current Settings */
    );
    
/***************************************************************************
Summary:
Set Audio PLL Settings
***************************************************************************/
NEXUS_Error NEXUS_AudioOutputPll_SetSettings(
    NEXUS_AudioOutputPll pll,
    const NEXUS_AudioOutputPllSettings *pSettings
    );

/***************************************************************************
Summary:
Get Loudness Equivalence Mode Settings
***************************************************************************/
void NEXUS_AudioModule_GetLoudnessSettings(
    NEXUS_AudioLoudnessSettings *pSettings
    );

/***************************************************************************
Summary:
Set Loudness Equivalence Mode Settings
***************************************************************************/
NEXUS_Error NEXUS_AudioModule_SetLoudnessSettings(
    const NEXUS_AudioLoudnessSettings *pSettings
    );
    
/***************************************************************************
Summary:
Audio HW/FW Capabilities
***************************************************************************/
typedef struct NEXUS_AudioCapabilities
{
    struct
    {
        unsigned hdmi;                  /* Number of HDMI audio inputs */
        unsigned i2s;                   /* Number of I2S inputs */
        unsigned spdif;                 /* Number of SPDIF inputs */
    } numInputs;

    struct
    {
        unsigned audioReturnChannel;    /* Number of ARC outputs */
        unsigned capture;               /* Maximum number of OutputCapture objects */
        unsigned dac;                   /* Number of DAC outputs */
        unsigned dummy;                 /* Maximum number of DummyOutput objects */
        unsigned hdmi;                  /* Number of HDMI audio outputs */
        unsigned i2s;                   /* Number of I2S outputs */
        unsigned loopback;              /* Maximum number of Loopback objects */
        unsigned rfmod;                 /* Number of RF Mod outputs */
        unsigned spdif;                 /* Number of SPDIF outputs */
    } numOutputs;

    unsigned numDecoders;               /* Maximum number of Decoders */
    unsigned numPlaybacks;              /* Maximum number of Playbacks */
    unsigned numInputCaptures;          /* Maximum number of InputCaptures */
    unsigned numVcxos;                  /* Number of VCXO PLLs that feed audio PLLs */
    unsigned numPlls;                   /* Number of output PLL clocks */
    unsigned numNcos;                   /* Number of output NCO clocks */
    unsigned numStcs;                   /* Number of STC that audio can use. Will be <= NEXUS_TransportCapabilities.numStcs. */
    unsigned numMixers;                 /* Number of Mixers */

    unsigned numDsps;                   /* Number of audio DSPs */
    unsigned numSoftAudioCores;         /* Number of Soft Audio Cores */
    unsigned numCrcs;                   /* Number of audio CRCs */
    unsigned numSrcs;                   /* Number of audio SRCs (Sample Rate Convertors) */
    unsigned numEqualizerStages;        /* Max Number of Equalizer stages that can be active */
    struct
    {
        struct
        {
            bool decode;                /* True if this codec can be decoded */
            bool passthrough;           /* True if this codec can be passthroughed */
            bool encode;                /* True if this codec can be encoded */
        } codecs[NEXUS_AudioCodec_eMax];

        bool audysseyAbx;               /* True if Audyssey ABX is supported */
        bool audysseyAdv;               /* True if Audyssey ADV is supported */
        bool autoVolumeLevel;           /* True if AutoVolumeLevel is supported */
        bool _3dSurround;               /* True if 3dSurround is supported */
        bool decodeRateControl;         /* True if DSOLA audio rate control is supported */
        bool dolbyDigitalReencode;      /* True if Dolby MS11 is supported */
        bool dolbyVolume;               /* True if DolbyVolume processing is supported */
        bool dolbyVolume258;            /* True if DolbyVolume258 processing is supported */
        bool karaoke;                   /* True if Karaoke post processing is supported */

        struct
        {
            bool supported;             /* True if echo cancellation is supported */
            bool algorithms[NEXUS_EchoCancellerAlgorithm_eMax];  /* Each supported algorithm will be set to true */
        } echoCanceller;

        bool encoder;                   /* True if encoder is supported */
        bool mixer;                     /* True if DSP mixers are supported */
        bool muxOutput;                 /* True if MuxOutput is supported for transcoding */

        struct
        {
            bool supported;             /* True if RF encoder is supported */
            bool encodings[NEXUS_RfAudioEncoding_eMax];  /* Each supported encoding will be set to true */
        } rfEncoder;

        bool studioSound;               /* True if SRS StudioSound processing is supported */
        bool truSurroundHd;             /* True if SRS TruSurround HD processing is supported */
        bool truVolume;                 /* True if SRS TruVolume processing is supported */
        bool processing[NEXUS_AudioPostProcessing_eMax]; /* True if specificed type is supported by NEXUS_AudioProcessor */
    } dsp;

    struct
    {
        bool supported;                 /* True if Equalizer support is available */
    } equalizer;

} NEXUS_AudioCapabilities;

/***************************************************************************
Summary:
Get Audio HW/FW Capabilities
***************************************************************************/
void NEXUS_GetAudioCapabilities(
    NEXUS_AudioCapabilities *pCaps        /* [out] */
    );

/***************************************************************************
Summary:
Audio FW Version
***************************************************************************/
typedef struct NEXUS_AudioFirmwareVersion
{
    unsigned major;                       /* Major Version of DSP Firmware */
    unsigned minor;                       /* Minor Version of DSP Firmware */
    unsigned branch[2];                   /* Branch Details */
} NEXUS_AudioFirmwareVersion;

/***************************************************************************
Summary:
Get Audio FW Version
***************************************************************************/
void NEXUS_GetAudioFirmwareVersion(
    NEXUS_AudioFirmwareVersion *pVersion /* [out] */
    );

/***************************************************************************
Summary:
Audio Buffer Structure
***************************************************************************/
typedef struct NEXUS_AudioBuffer
{
    void *pBuffer;     /* attr{memory=cached} Contiguous buffer base address */
    void *pWrapBuffer; /* attr{memory=cached} Buffer address after wraparound */
} NEXUS_AudioBuffer;

/***************************************************************************
Summary:
Audio Buffer Descriptor
***************************************************************************/
typedef struct NEXUS_AudioBufferDescriptor
{
    bool interleaved;               /* If true, every other channel will have valid pointers,
                                       e.g. left for L/R, leftSurround for L/R Surround, etc.  */
    unsigned numBuffers;            /* Number of buffers.  For mono/interleaved stereo this is 1.  For
                                       non-interleaved stereo, it's 2.  For non-interleaved 7.1 it's 8. */

    NEXUS_AudioBuffer left;         /* Left channel buffer.  Also carries mono data in mono configurations. */
    NEXUS_AudioBuffer right;        /* Right channel buffer (valid only if interleaved=false) */
    NEXUS_AudioBuffer leftSurround; /* Left surround channel buffer */
    NEXUS_AudioBuffer rightSurround;/* Right surround channel buffer (valid only if interleaved=false) */
    NEXUS_AudioBuffer center;       /* Center channel buffer */
    NEXUS_AudioBuffer lfe;          /* LFE channel buffer (valid only if interleaved=false) */
    NEXUS_AudioBuffer leftRear;     /* Left rear channel buffer */
    NEXUS_AudioBuffer rightRear;    /* Right rear channel buffer (valid only if interleaved=false) */

    unsigned bufferSize;            /* Buffer size before wraparound in bytes */
    unsigned wrapBufferSize;        /* Buffer size after wraparound in bytes */        
} NEXUS_AudioBufferDescriptor;


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
Audio Module Settings
**************************************************************************/
typedef struct NEXUS_AudioModuleSettings
{
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

#ifdef __cplusplus
}
#endif

#endif /* #ifndef NEXUS_AUDIO_H__ */
