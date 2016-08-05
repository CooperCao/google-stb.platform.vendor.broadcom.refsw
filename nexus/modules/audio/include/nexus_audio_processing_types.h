/***************************************************************************
*  Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
***************************************************************************/

#ifndef NEXUS_AUDIO_PROCESSING_TYPES_H__
#define NEXUS_AUDIO_PROCESSING_TYPES_H__

#include "nexus_types.h"
#include "nexus_audio_decoder_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
Summary:
Auto Volume Level Settings
***************************************************************************/
typedef struct NEXUS_AutoVolumeLevelSettings
{
    bool enabled;                   /* If true, processing is enabled.  Otherwise this stage is bypassed */
    bool loudnessEqualization;      /* If true, loudness level equivalence processing is enabled. */

    /* The settings below are typically used at defaults, but may be modified if desired.
       Careful study and analysis is recommended before adjusting these values.  */

    int target;                     /* Target level of output signal in terms of dBFs */
    int lowerBound;                 /* Lower bound for primary range of volume control */
    unsigned fixedBoost;            /* Amount of fixed boost (in dB) if level < LOWERBOUND */
    unsigned ref;                   /* Reference logarithmic gain value of 0 dBFs */
    unsigned alpha;                 /* Attenuation factor for Level Maximum */
    unsigned beta;                  /* Attenuation factor for Level Mean */
    unsigned threshold;             /* Threshold for detecting active portion of signal . Input value should be multiplied by factor of 10 */
    unsigned dtfPercent;            /* Decay Time for DTF Percent delay (in seconds) */
    unsigned alpha2;                /* Attenuation factor for Level Minimum */
    unsigned fastGainRamp;          /* Fast Gain Ramp in milliseconds. Input value should be multiplied by factor of 10 */
    unsigned dtf;                   /* Decay Time Fraction. Input value should be multiplied by factor of 10 */
} NEXUS_AutoVolumeLevelSettings;

/***************************************************************************
Summary:
TruVolume Block Size
***************************************************************************/
typedef enum NEXUS_TruVolumeBlockSize
{
    NEXUS_TruVolumeBlockSize_e256,
    NEXUS_TruVolumeBlockSize_e512,
    NEXUS_TruVolumeBlockSize_e768,
    NEXUS_TruVolumeBlockSize_e1024,
    NEXUS_TruVolumeBlockSize_eMax
} NEXUS_TruVolumeBlockSize;

/***************************************************************************
Summary:
TruVolume Mode
***************************************************************************/
typedef enum NEXUS_TruVolumeMode
{
    NEXUS_TruVolumeMode_eLight,
    NEXUS_TruVolumeMode_eNormal,
    NEXUS_TruVolumeMode_eHeavy,
    NEXUS_TruVolumeMode_eMax
} NEXUS_TruVolumeMode;

/***************************************************************************
Summary:
TruVolume Speaker Resolution
***************************************************************************/
typedef enum NEXUS_TruVolumeSpeakerResolution
{
    NEXUS_TruVolumeSpeakerResolution_e20Hz,
    NEXUS_TruVolumeSpeakerResolution_e40Hz,
    NEXUS_TruVolumeSpeakerResolution_e110Hz,
    NEXUS_TruVolumeSpeakerResolution_e200Hz,
    NEXUS_TruVolumeSpeakerResolution_e315Hz,
    NEXUS_TruVolumeSpeakerResolution_e410Hz,
    NEXUS_TruVolumeSpeakerResolution_eMax
} NEXUS_TruVolumeSpeakerResolution;

/***************************************************************************
Summary:
SRS Filter Coefficient Generation Modes. Also required by StudioSound.
***************************************************************************/
typedef enum NEXUS_SrsFilterCoefGenMode
{
    NEXUS_SrsFilterCoefGenMode_eFilterUser,  /* Use user specified coefficients */
    NEXUS_SrsFilterCoefGenMode_eFilterSpec,  /* Use Filter Specification to generate filter coeffcients */
    NEXUS_SrsFilterCoefGenMode_eMax
} NEXUS_SrsFilterCoefGenMode;

/***************************************************************************
Summary:
SRS High Pass Filter Order. Also required by StudioSound.
***************************************************************************/
typedef enum NEXUS_SrsFilterOrder
{
    NEXUS_SrsFilterOrder_eOrder0 = 0,
    NEXUS_SrsFilterOrder_eOrder2 = 2,
    NEXUS_SrsFilterOrder_eOrder4 = 4,
    NEXUS_SrsFilterOrder_eOrder6 = 6,
    NEXUS_SrsFilterOrder_eMax
} NEXUS_SrsFilterOrder;

/***************************************************************************
Summary:
SRS High Pass Filter Coefficients. Also required by StudioSound.
***************************************************************************/
typedef struct NEXUS_SrsHighPassFilterCoef
{
    unsigned scale;    /* Specified q-format of the coefficients. (1+scale).(31-scale) format
                                             Default: 1.  scale=1 implies a q-format of 2.30 */
    int coefficientB0;
    int coefficientB1;
    int coefficientB2;
    int coefficientA1;
    int coefficientA2;
} NEXUS_SrsHighPassFilterCoef;

/***************************************************************************
Summary:
SRS High Pass Filter parameter for coefficients specified by user. Also required by StudioSound.
***************************************************************************/
typedef struct NEXUS_SrsHighPassFilterCoefUserParam
{
    NEXUS_SrsFilterOrder filterOrder;            /* HighPass Filter order. Default NEXUS_SrsFilterOrder_eOrder4 */
    NEXUS_SrsHighPassFilterCoef coefficients[3];  /* Number of active biquads filter decided by filterOrder,  N = filterOrder/2 */
} NEXUS_SrsHighPassFilterCoefUserParam;


/***************************************************************************
Summary:
SRS High Pass Filter parameter for coefficients generated by specification. Also required by StudioSound.
***************************************************************************/
typedef struct NEXUS_SrsHighPassFilterCoefSpecParam
{
    unsigned cutOffFrequency;           /* Range: 20 to 1000 hz, Default: 180 Hz */
    NEXUS_SrsFilterOrder filterOrder;   /* HighPass Filter order. Default NEXUS_SrsFilterOrder_eOrder4 */
} NEXUS_SrsHighPassFilterCoefSpecParam;


/***************************************************************************
Summary:
SRS High Pass Filter Settings. Also required by StudioSound.
***************************************************************************/
typedef struct NEXUS_SrsHighPassFilterSettings
{
    bool enabled;                       /* If true, High Pass Filter processing is enabled. Default: true */
    NEXUS_SrsFilterCoefGenMode coefGenMode;         /* Coefficients generation mode. Default: NEXUS_SrsHighPassFilterCoefGenMode_eFilterUser */

    union
    {
        NEXUS_SrsHighPassFilterCoefUserParam highPassFilterCoefUser[3];  /* Array size 3, representing supported sampling rates 32, 44.1 and 48kHz,
            will be used if coefMode is NEXUS_SrsFilterCoefGenMode_eFilterUser */
        NEXUS_SrsHighPassFilterCoefSpecParam highPassFilterCoefSpec;      /* Filter specification for generating filter coeffcients,
            will be used if coefMode is NEXUS_SrsFilterCoefGenMode_eFilterSpec */
    } coefParam;
} NEXUS_SrsHighPassFilterSettings;

/***************************************************************************
Summary:
TruVolume Settings
***************************************************************************/
typedef struct NEXUS_TruVolumeSettings
{
    bool enabled;               /* If true, processing is enabled.  Otherwise this stage is bypassed */

    NEXUS_TruVolumeBlockSize blockSize;    /* Block Size for processing */

    bool enableNormalGain;      /* If true, normal processing will be performed.  If false, only bypassGain is applied. */

    unsigned inputGain;         /* Input gain ranges from 0 to 3200 in %. Default: 100 (1.0) */
    unsigned outputGain;        /* Output gain ranges from 0 to 400 in %. Default: 100 (1.0) */
    unsigned bypassGain;        /* Bypass gain (In %).  Ranges from 0 to 100.  Default: 100 (1.0) */

    unsigned referenceLevel;    /* Reference level.  Ranges from 0 to 100 in %.  Default: 50 (0.5) */

    NEXUS_TruVolumeMode mode;    /* Processing Mode. */

    NEXUS_TruVolumeSpeakerResolution speakerResolution;

    unsigned maxGain;           /* Max gain control (In %).  Ranges from 25 to 25600.  Default: 1600 (16.0) */

    bool enableDcNotchFilter;   /* If true, the DC notch filter will be enabled */

    bool enableNoiseManager;
    int noiseManagerThreshold;  /* Noise manager threshold.  Ranges from 0 to 100 in %.  Default: 10 (0.10) */

    bool enableNormalizer;      /* If true, the normalizer is enabled.  */

    unsigned calibrate;         /* Ranges from 0..25600 in %.  Default=100 (1.0) */

    NEXUS_SrsHighPassFilterSettings highPassFilter; /* High Pass Filter Settings applied prior TruVolume */
} NEXUS_TruVolumeSettings;

/***************************************************************************
Summary:
Dolby Digital Reencode Downmix Modes
***************************************************************************/
typedef enum NEXUS_DolbyDigitalReencodeDownmixMode
{
    NEXUS_DolbyDigitalReencodeDownmixMode_eLtRt,
    NEXUS_DolbyDigitalReencodeDownmixMode_eLoRo,
    NEXUS_DolbyDigitalReencodeDownmixMode_eArib,
    NEXUS_DolbyDigitalReencodeDownmixMode_eMax
} NEXUS_DolbyDigitalReencodeDownmixMode;

/***************************************************************************
Summary:
Dolby Digital Reencode Profile
***************************************************************************/
typedef enum NEXUS_DolbyDigitalReencodeProfile
{
    NEXUS_DolbyDigitalReencodeProfile_eNoCompression,
    NEXUS_DolbyDigitalReencodeProfile_eFilmStandardCompression,
    NEXUS_DolbyDigitalReencodeProfile_eFilmLightCompression,
    NEXUS_DolbyDigitalReencodeProfile_eMusicStandardCompression,
    NEXUS_DolbyDigitalReencodeProfile_eMusicLightCompression,
    NEXUS_DolbyDigitalReencodeProfile_eSpeechCompression,
    NEXUS_DolbyDigitalReencodeProfile_eMax
} NEXUS_DolbyDigitalReencodeProfile;

/***************************************************************************
Summary:
AC3 Encode Settings
***************************************************************************/
typedef struct NEXUS_Ac3EncodeSettings
{
    bool spdifHeaderEnabled;    /* If true, SPDIF header generation is enabled.  */
    bool certificationMode;     /* If true, run in certification mode.  Set to false for normal operation. */
} NEXUS_Ac3EncodeSettings;

/***************************************************************************
Summary:
Dolby Digital Reencode Settings
***************************************************************************/
typedef struct NEXUS_DolbyDigitalReencodeSettings
{
    NEXUS_Ac3EncodeSettings encodeSettings; /* Settings for the AC3/Dolby Digital encoder */

    NEXUS_AudioDecoderDualMonoMode          dualMonoMode;   /* Mode for streams coded as dual-mono */
    NEXUS_AudioDecoderOutputMode            outputMode;     /* DDRE output mode.  In general, eAuto is correct, but this may be overridden
                                                               for testing and/or certification purposes. */
    NEXUS_AudioDecoderOutputLfeMode         outputLfeMode;  /* LFE output flag.  In general, eAuto is correct, but this may be overridden
                                                               for testing and/or certification purposes. */
    NEXUS_AudioMultichannelFormat           multichannelFormat; /* specify the multichannel output format desired.
                                                                   Valid settings are 5.1 or 7.1.
                                                                   7.1ch output is only supported for MS12 Config A supported products */

    NEXUS_AudioDecoderDolbyDrcMode          drcMode;    /* DRC Mode for multichannel connector.  This will be the DRC mode applied for multichannel outputs */
    NEXUS_AudioDecoderDolbyDrcMode          drcModeDownmix; /* DRC Mode for stereo connector.   This will be the DRC mode applied for stereo outputs */
    NEXUS_DolbyDigitalReencodeDownmixMode   stereoDownmixMode;
    uint16_t                                cut;                           /* DRC Cut factor */
    uint16_t                                boost;                         /* DRC Boost factor */

    bool loudnessEquivalenceEnabled;    /* This value has been depracated.  Loudness equivalence is enabled via NEXUS_AudioModuleSettings.loudnessMode. */

    bool externalPcmMode;               /* If true, operate with input from non-MS decoders.
                                           Should be false if any input is from a dolby MS decoder (AC3/AC3+/AAC). */
    bool certificationMode;             /* If true, operate in certification mode.  This also implies externalPcmMode=true. */

    /* The following settings are only applied if externalPcmMode = true or certificationMode = true. */
    NEXUS_DolbyDigitalReencodeProfile     profile;              /* Compression Profile */
    NEXUS_AudioAc3CenterMixLevel          centerMixLevel;       /* cmixlev - Center Mix Level */
    NEXUS_AudioAc3SurroundMixLevel        surroundMixLevel;     /* surmixlev - Surround Mix Level */
    NEXUS_AudioAc3DolbySurround           dolbySurround;        /* dsurmod - AC3 Dolby Surround Mode */
    unsigned                              dialogLevel;          /* Dialog level of incoming PCM in dB.  Ranges from 0..31. */

    /* The following settings are only applied if certificationMode = true. */
    NEXUS_AudioAc3Acmod                   inputAcmod;           /* ACMOD of the incoming stream */
    bool                                  inputHasLfe;          /* True if the incoming stream has LFE data */
    bool                                  inputHasDv258Applied; /* True if the input has DV258 processing applied. */

} NEXUS_DolbyDigitalReencodeSettings;

/***************************************************************************
Summary:
DolbyVolume258 Settings
***************************************************************************/
typedef struct NEXUS_DolbyVolume258Settings
{
    bool enabled;       /* If true, processing is enabled.  Otherwise, the stage is bypassed. */

    /* System Settings*/
    int preGain;                    /* Gain applied to the signal prior to entering DV258.  Ranges from -480 to 480 (-30..30 in steps of 0.0625 dB) */
    unsigned inputReferenceLevel;   /* Input reference level.  Ranges from 0 to 2080 (0..130 in steps of 0.0625 dB) */
    unsigned outputReferenceLevel;  /* Input reference level.  Ranges from 0 to 2080 (0..130 in steps of 0.0625 dB) */
    int calibrationOffset;          /* Calibration offset.  Ranges from -480 to 480 (-30..30 in steps of 0.0625 dB) */
    bool reset;                     /* Forces a reset if true. */

    /*Volume Modeler Settings*/
    bool volumeModelerEnabled;      /* If true, the volume modeler is enabled. */
    int digitalVolumeLevel;         /* Volume level gain applied by Dolby Volume.  Ranges from -1536 to 480 (-96..30 in steps of 0.0625 dB) */
    int analogVolumeLevel;          /* Volume level gain applied after Dolby Volume.  Ranges from -1536 to 480 (-96..30 in steps of 0.0625 dB) */

    /*Volume Leveler Settings */
    bool volumeLevelerEnabled;      /* If true, the volume leveler is enabled */
    bool midsideProcessingEnabled;  /* If true, midside processing is enabled */
    bool halfModeEnabled;           /* If true, half mode will be enabled */
    unsigned volumeLevelerAmount;   /* Ranges from 0 to 10 */

    /* Limiter Settings */
    bool limiterEnabled;            /* If true, the limiter is enabled */

} NEXUS_DolbyVolume258Settings;

/***************************************************************************
Summary:
KaraokeVocal Settings
***************************************************************************/
typedef struct NEXUS_KaraokeVocalSettings
{
    /* Karaoke Vocal Echo Effect */
    struct
    {
        bool enabled;               /* Enable Echo effect. Default is false. */
        unsigned attenuation;       /* Percentage of attenuation of Echo Effect, specified in Q31 format.
                                       Valid Values 0-100. Default is 20. */
        unsigned delay;             /* Delay of Echo effect, specified in milliseconds.
                                       Valid values are 0-200. Default is 200 */
    } echo;
} NEXUS_KaraokeVocalSettings;

/***************************************************************************
Summary:
Audio Fade Settings
***************************************************************************/
typedef struct NEXUS_AudioFadeSettings
{
    unsigned level;             /* Percentage representing the volume level.
                                   0 is muted, 100 is full volume. Default is 100. */
    unsigned duration;          /* duration in milliseconds it will take to change
                                   to a new level. Valid values are 3 - 1000 */
    unsigned type;              /* specifies the type of fade -
                                   0- Linear (Default), 1-Cubic-In, 2-Cubic-Out. */
} NEXUS_AudioFadeSettings;

/***************************************************************************
Summary:
Audio Fade Status
***************************************************************************/
typedef struct NEXUS_AudioFadeStatus
{
    bool active;
    unsigned remaining;         /* MilliSeconds remaining in the current active fade */
    unsigned level;             /* Percentage representing the current volume level.
                                   0 is muted, 100 is full volume. Default is 100. */
} NEXUS_AudioFadeStatus;

#ifdef __cplusplus
}
#endif

#endif /* #ifndef NEXUS_AUTO_VOLUME_LEVEL_H__ */
