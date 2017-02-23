/***************************************************************************
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
 *
 * Module Description: Audio Decoder Interface
 *
 ***************************************************************************/

#ifndef BAPE_CODEC_TYPES_H_
#define BAPE_CODEC_TYPES_H_

/***************************************************************************
Summary:
AC3 DRC (Dynamic Range Compression) Modes
***************************************************************************/
typedef enum BAPE_DolbyDrcMode
{
    BAPE_DolbyDrcMode_eLine,
    BAPE_DolbyDrcMode_eRf,
    BAPE_DolbyDrcMode_eCustomA,
    BAPE_DolbyDrcMode_eCustomD,
    BAPE_DolbyDrcMode_eDisabled,
    BAPE_DolbyDrcMode_eCustomTarget,
    BAPE_DolbyDrcMode_eMax
} BAPE_DolbyDrcMode;

typedef BAPE_DolbyDrcMode BAPE_Ac3DrcMode;
#define BAPE_Ac3DrcMode_eLine BAPE_DolbyDrcMode_eLine
#define BAPE_Ac3DrcMode_eRf BAPE_DolbyDrcMode_eRf
#define BAPE_Ac3DrcMode_eCustomA BAPE_DolbyDrcMode_eCustomA
#define BAPE_Ac3DrcMode_eCustomD BAPE_DolbyDrcMode_eCustomD
#define BAPE_Ac3DrcMode_eDisabled BAPE_DolbyDrcMode_eDisabled
#define BAPE_Ac3DrcMode_eCustomTarget BAPE_DolbyDrcMode_eCustomTarget
#define BAPE_Ac3DrcMode_eMax BAPE_DolbyDrcMode_eMax

/***************************************************************************
Summary:
AC3 Stereo Downmix Modes
***************************************************************************/
typedef enum BAPE_Ac3StereoMode
{
    BAPE_Ac3StereoMode_eAuto,   /* Automatically select based on bitstream */
    BAPE_Ac3StereoMode_eLtRt,   /* Dolby ProLogic-II Compatible */
    BAPE_Ac3StereoMode_eLoRo,   /* Stereo Only */
    BAPE_Ac3StereoMode_eMax
} BAPE_Ac3StereoMode;

/***************************************************************************
Summary:
AC3 Codec Settings
***************************************************************************/
typedef struct BAPE_Ac3Settings
{
    BAPE_Ac3DrcMode drcMode;        /* DRC (Dynamic Range Compression) Mode */
    BAPE_Ac3DrcMode drcModeDownmix; /* DRC (Dynamic Range Compression) Mode for stereo downmixed data*/
    unsigned customTargetLevel; /* Used with drcMode set to NEXUS_AudioDecoderDolbyDrcMode_eCustomTarget.  Valid values are 0, which will result in DRC disabled,
                                   23 and 24 which result in outputs of -23 and -24dB */
    unsigned customTargetLevelDownmix; /* Used with drcModeDownmix set to NEXUS_AudioDecoderDolbyDrcMode_eCustomTarget.  Valid values are 0, which will result in DRC disabled,
                                          23 and 24 which result in outputs of -23 and -24dB */

    BAPE_Ac3StereoMode stereoMode;  /* Stereo Downmix Mode */

    uint16_t drcScaleHi;            /* In %, ranges from 0..100 */
    uint16_t drcScaleLow;           /* In %, ranges from 0..100 */
    uint16_t drcScaleHiDownmix;     /* In %, ranges from 0..100 */
    uint16_t drcScaleLowDownmix;    /* In %, ranges from 0..100 */

    uint16_t scale;                 /* PCM Scale factor */
    uint16_t scaleDownmix;          /* PCM Scale factor for stereo downmixed data only.  Applicable to MS11 licensed decoder only. */

    bool dialogNormalization;       /* Use dialog normalization values from the stream.  This should be true for normal operation. */
    unsigned dialogNormalizationValue; /* User-specified dialog normalization value.  Used only if dialogNormalization = false. Specified in dB, 0..31 */

    unsigned substreamId;           /* Applicable only to MS10/MS11 licensed decoders */
    bool enableAtmosProcessing;     /* Applicable only to MS12 licensed decoders */
    /*bool enableDecorrelation;*/       /* Applicable only to MS12 licensed decoders */
    unsigned certificationMode;     /* for internal use only */
} BAPE_Ac3Settings;

/***************************************************************************
Summary:
Dolby Stereo Downmix Modes
***************************************************************************/
typedef enum BAPE_DolbyStereoMode
{
    BAPE_DolbyStereoMode_eLoRo,   /* LoRo */
    BAPE_DolbyStereoMode_eLtRt,   /* Pro-Logic/Pro-Logic II */
    BAPE_DolbyStereoMode_eMax
} BAPE_DolbyStereoMode;

/***************************************************************************
Summary:
    This enum describes how the decoder will select the AC4 presentation
***************************************************************************/
typedef enum BAPE_Ac4PresentationSelectionMode
{
    BAPE_Ac4PresentationSelectionMode_eAuto,
    BAPE_Ac4PresentationSelectionMode_ePresentationIndex,
    BAPE_Ac4PresentationSelectionMode_ePresentationIdentifier,
    BAPE_Ac4PresentationSelectionMode_eMax
} BAPE_Ac4PresentationSelectionMode;

/***************************************************************************
Summary:
AC4 Associate type
***************************************************************************/
typedef enum BAPE_Ac4AssociateType
{
    BAPE_Ac4AssociateType_eNotSpecified,
    BAPE_Ac4AssociateType_eVisuallyImpaired,
    BAPE_Ac4AssociateType_eHearingImpaired,
    BAPE_Ac4AssociateType_eCommentary,
    BAPE_Ac4AssociateType_eMax
} BAPE_Ac4AssociateType;

/***************************************************************************
Summary:
AC4 Codec Settings
***************************************************************************/
#define BAPE_AC4_PRESENTATION_NAME_LENGTH  36
#define BAPE_AC4_PRESENTATION_ID_LENGTH    20
#define BAPE_AC4_LANGUAGE_NAME_LENGTH      8
#define BAPE_AC4_NUM_LANGUAGES             2
typedef struct BAPE_Ac4Settings
{
    BAPE_DolbyDrcMode drcMode;      /* DRC (Dynamic Range Compression) Mode */
    BAPE_DolbyDrcMode drcModeDownmix;/* DRC (Dynamic Range Compression) Mode for stereo downmix path */

    uint16_t drcScaleHi;            /* In %, ranges from 0..100 */
    uint16_t drcScaleLow;           /* In %, ranges from 0..100 */
    uint16_t drcScaleHiDownmix;     /* In %, ranges from 0..100 */
    uint16_t drcScaleLowDownmix;    /* In %, ranges from 0..100 */

    BAPE_DolbyStereoMode stereoMode;/* Stereo Downmix Mode */

    unsigned programSelection;      /* Program Selection for current presentation.
                                       0 (Default) - decode main + associate program,
                                       1 - decode main program only,
                                       2 - decode associate program only. */

    #if 0
    unsigned decodeMode;            /* decode mode describtes the program allocation from one or more sources
                                       0 - single program, one decode (either main OR associate)
                                       1 - single program, single decoder instance main + associate decode
                                       2 - single program, dual decoder instance main + associate decode
                                       3 (default) - dual program, dual decoder instance main + associate decode */
    #endif

    int programBalance;             /* Program balance adjusts the balance between the main and description
                                       programs. This control is for embedded description program only.
                                       Valid values are -32 to 32. -32 is main only, 32 is description only.
                                       Default is -32 */

    BAPE_Ac4PresentationSelectionMode selectionMode;   /* Specifies how the AC4 decoder selects the presentation -
                                                          Default setting is eAuto, allowing the decoder to choose based on the presence or absense
                                                          of Presentation Index or Id, followed by the various personalization parameters.;
                                                          eAuto setting should be used for certification testing */

    unsigned presentationIndex;        /* Multiple "presentation" groups can exist within a single program.
                                          To select by presentation index, set selectionMode = BAPE_Ac4PresentationSelectionMode_ePresentationIndex
                                          Valid values are 0 - 511. Default value is 0.
                                          See BAPE_DecoderStatus/BAPE_DecoderAc4PresentationInfo for more information. */

    char presentationId[BAPE_AC4_PRESENTATION_ID_LENGTH]; /* Multiple "presentation" groups can exist within a single program.
                                                             To select by presentation Id, set selectionMode = BAPE_Ac4PresentationSelectionMode_ePresentationIdentifier
                                                             This unique id can come in short or long varieties, per the Dolby AC4 spec.
                                                             Presentation Ids are obtained from the Stream Status info.
                                                             See BAPE_DecoderStatus/BAPE_DecoderAc4PresentationInfo for more information */

    int dialogEnhancerAmount;       /* Valid values are -12 to +12, in 1dB steps. Default value is 0 */

    unsigned certificationMode;     /* for internal use only */

    /* optional personalization parameters */
    bool preferLanguageOverAssociateType; /* correlates to Dolby AC4 preference for language over associate type -
                                             Default setting is false (Associate type is prioritized over Language) */

    struct {
        char selection[BAPE_AC4_LANGUAGE_NAME_LENGTH];   /* IETF BCP 47 language code. Codes that are longer than
                                                            8 characters should be truncated. */
    } languagePreference[BAPE_AC4_NUM_LANGUAGES];

    BAPE_Ac4AssociateType preferredAssociateType;
    bool enableAssociateMixing;     /* Enable mixing of associate program */

} BAPE_Ac4Settings;

/***************************************************************************
Summary:
AAC Stereo Downmix Modes
***************************************************************************/
typedef enum BAPE_AacStereoMode
{
    BAPE_AacStereoMode_eMatrix,     /* Default (Not applicable with Dolby Pulse) */
    BAPE_AacStereoMode_eArib,       /* ARIB Compatible (Not applicable with Dolby Pulse) */
    BAPE_AacStereoMode_eLtRt,       /* LtRt (Dolby Pulse Only) */
    BAPE_AacStereoMode_eLoRo,       /* LoRo (Dolby Pulse Only) */
    BAPE_AacStereoMode_eMax
} BAPE_AacStereoMode;

/***************************************************************************
Summary:
Dolby Pulse Dynamic Range Compression Mode
***************************************************************************/
typedef enum BAPE_DolbyPulseDrcMode
{
    BAPE_DolbyPulseDrcMode_eLine,
    BAPE_DolbyPulseDrcMode_eRf,
    BAPE_DolbyPulseDrcMode_eOff, /* Not used in Dolby Pulse, forces to Line Mode */
    BAPE_DolbyPulseDrcMode_eMax
} BAPE_DolbyPulseDrcMode;

/***************************************************************************
Summary:
AAC Codec Settings
***************************************************************************/
typedef struct BAPE_AacSettings
{
    uint16_t drcScaleHi;            /* In %, ranges from 0..100 */
    uint16_t drcScaleLow;           /* In %, ranges from 0..100 */
    uint16_t drcReferenceLevel;     /* DRC (Dynamic Range Compression) Reference Level.  Ranges from 0..127  in -0.25dB units (e.g. program 80 for -20dB) */

    BAPE_AacStereoMode downmixMode; /* AAC Downmix Mode */

    bool enableGainFactor;      /* If true, the gain factor below is applied (Used in Dolby Pulse only) */
    uint32_t gainFactor;        /* Gain factor in 8.24 format - 0x00800000=unity, 0x00B504F3=-3dB (Used in Dolby Pulse only) */

    uint16_t drcDefaultLevel;   /* Dynamic range compression default level.  Ranges from 0..127 (Used in Dolby Pulse only) */

    BAPE_DolbyPulseDrcMode drcMode; /* Dynamic Range Compression Mode (Used in Dolby Pulse only) */

    bool enableDownmixCoefficients; /* If true, downmix coefficients below are applied (Not applicable with Dolby Pulse) */

    bool mpegConformanceMode;       /* If true, enable MPEG conformance testing for Dolby Pulse certification. */

    bool enableSbrDecoding;         /* If set to true, AAC HE decoding is enabled.
                                       If set to false, AAC HE decoding is disabled,
                                       allowing only AAC LC decoding */

    uint16_t downmixCoefficients[6][6]; /* User defined downmix (or karaoke) coefficients.
                                           Valid only when enableDownmixCoefficients is true (Not Applicable with Dolby Pulse) */

    uint32_t downmixCoefScaleIndex;      /* Default = 0,  0 -> 0dB, 1 -> -0.5dB, 2 -> -1dB, ... , 23 -> -11.5dB, 24 -> -12dB; all values beyond 24 map to 50
                                            50 -> Decoder default settings (existing normalization) (Used in NON Dolby Pulse Only) */
    bool ignoreEmbeddedPrl;              /* If enabled we will ignore the embedded Program Reference Level (Used in NON Dolby Pulse Only)*/
} BAPE_AacSettings;

/***************************************************************************
Summary:
MPEG Codec Settings
***************************************************************************/
typedef struct BAPE_MpegSettings
{
    int inputReferenceLevel;            /*  The input level to the decoder in dB. Valid values
                                            are 0 to -31.
                                            Defaults to -24 if Loundness mode ATSC is enabled and
                                            -23 if Loudness mode EBU is enabled */
    bool attenuateMonoToStereo;         /* If enabled we will attenuate 3dB when converting a mono
                                           stream to stereo outputs.  Default is enabled */
} BAPE_MpegSettings;

/***************************************************************************
Summary:
WMA Pro Dynamic Range Control modes
***************************************************************************/
typedef enum BAPE_WmaProDrcMode
{
    BAPE_WmaProDrcMode_eDisabled,   /* DRC is disabled */
    BAPE_WmaProDrcMode_eHigh,       /* No scaling is applied.  Content is played in it's original form. */
    BAPE_WmaProDrcMode_eMedium,     /* drc_frame_scale_factor is applied along with a scale factor to make
                                       the output rms of the content rms_amplitude_target_dB, and the peak
                                       of the content be peak_amplitude_target_med_dB. */
    BAPE_WmaProDrcMode_eLow,        /* drc_frame_scale_factor is applied along with a scale factor to make
                                       the output rms of the content rms_amplitude_target_dB, and the peak
                                       of the content be peak_amplitude_target_low_dB.*/
    BAPE_WmaProDrcMode_eMax

} BAPE_WmaProDrcMode;

/***************************************************************************
Summary:
WMA Pro Stereo Downmix Modes
***************************************************************************/
typedef enum BAPE_WmaProStereoMode
{
    BAPE_WmaProStereoMode_eAuto,
    BAPE_WmaProStereoMode_eLtRt,
    BAPE_WmaProStereoMode_eLoRo,
    BAPE_WmaProStereoMode_eMax
} BAPE_WmaProStereoMode;

/***************************************************************************
Summary:
WMA Pro Codec Settings
***************************************************************************/
typedef struct BAPE_WmaProSettings
{
    BAPE_WmaProStereoMode stereoMode;
    BAPE_WmaProDrcMode drcMode;
    unsigned rmsAmplitudeReference;
    unsigned peakAmplitudeReference;
    unsigned desiredRms;        /* desired rmsDb for normalization */
    unsigned desiredPeak;       /* desired peakDb for normalization */
} BAPE_WmaProSettings;

/***************************************************************************
Summary:
DTS Dynamic Range Compression Mode
***************************************************************************/
typedef enum BAPE_DtsDrcMode
{
    BAPE_DtsDrcMode_eDisabled,
    BAPE_DtsDrcMode_eEnabled,
    BAPE_DtsDrcMode_eMax
} BAPE_DtsDrcMode;

/***************************************************************************
Summary:
DTS Stereo Downmix Modes
***************************************************************************/
typedef enum BAPE_DtsStereoMode
{
    BAPE_DtsStereoMode_eAuto,   /* Automatically select based on bitstream */
    BAPE_DtsStereoMode_eLtRt,
    BAPE_DtsStereoMode_eLoRo,
    BAPE_DtsStereoMode_eMax
} BAPE_DtsStereoMode;

/***************************************************************************
Summary:
DTS Codec Settings
***************************************************************************/
typedef struct BAPE_DtsSettings
{
    bool littleEndian;              /* Set to true if the data is little-endian as opposed to a bytestream (e.g. .wav content) */
    BAPE_DtsDrcMode drcMode;        /* Dynamic Range Compression Mode */
    uint16_t drcScaleHi;            /* In %, ranges from 0..100 */
    uint16_t drcScaleLow;           /* In %, ranges from 0..100 */

    BAPE_DtsStereoMode stereoMode;  /* Stereo Downmix Mode */
    bool mixLfeToPrimary;           /* Mix LFE to primary while downmixing, when Lfe output is disabled */
} BAPE_DtsSettings;

/***************************************************************************
Summary:
ADPCM Codec Settings
***************************************************************************/
typedef struct BAPE_AdpcmSettings
{
    struct
    {
        bool enabled;       /* If true, gain processing is enabled */
        uint32_t factor;    /* Ranges from BAPE_VOLUME_MAX to BAPE_VOLUME_MIN.  Default is BAPE_VOLUME_NORMAL. */
    } gain;
} BAPE_AdpcmSettings;

/***************************************************************************
Summary:
iLbc Codec Settings
***************************************************************************/
typedef struct BAPE_IlbcSettings
{
    bool packetLoss;            /* notify decoder if expected to tolerate packet loss */
    unsigned frameLength;       /* frame length in ms.  valid values 20, 30 */
    struct
    {
        bool enabled;           /* If true, gain processing is enabled */
        unsigned factor;        /* Specified in 1/100 dB.  Valid values for this codec
                                   are: 300 (+3dB), 600 (+6dB), 900 (+9dB) 1200 (+12dB) */
    } gain;
} BAPE_IlbcSettings;

/***************************************************************************
Summary:
iSac Coding Mode
***************************************************************************/
typedef enum BAPE_IsacCodingMode
{
    BAPE_IsacCodingMode_eAdaptive,
    BAPE_IsacCodingMode_eIndependent,
    BAPE_IsacCodingMode_eMax
} BAPE_IsacCodingMode;

/***************************************************************************
Summary:
iSac Band Mode
***************************************************************************/
typedef enum BAPE_IsacBandMode
{
    BAPE_IsacBandMode_eWide,
    BAPE_IsacBandMode_eNarrow,
    BAPE_IsacBandMode_eMax
} BAPE_IsacBandMode;

/***************************************************************************
Summary:
iSac Codec Settings
***************************************************************************/
typedef struct BAPE_IsacSettings
{
    bool packetLoss;            /* notify decoder if expected to tolerate packet loss */
    BAPE_IsacBandMode bandMode; /* 0 - Wide, 1 - Narrow */
    struct
    {
        bool enabled;           /* If true, gain processing is enabled */
        unsigned factor;        /* Specified in 1/100 dB.  Valid values for this codec
                                   are: 300 (+3dB), 600 (+6dB), 900 (+9dB) 1200 (+12dB) */
    } gain;
} BAPE_IsacSettings;


/***************************************************************************
Summary:
ALS Stereo Downmix Modes
***************************************************************************/
typedef enum BAPE_AlsStereoMode
{
    BAPE_AlsStereoMode_eArib,       /* ARIB Compatible */
    BAPE_AlsStereoMode_eLtRt,       /* LtRt */
    BAPE_AlsStereoMode_eMax
} BAPE_AlsStereoMode;

/***************************************************************************
Summary:
ALS Codec Settings
***************************************************************************/
typedef struct BAPE_AlsSettings
{
    BAPE_AlsStereoMode stereoMode;  /* ALS Downmix Mode.  Default is ARIB. */
    uint8_t aribMatrixMixdownIndex; /* Arib Matrix downmix index.  Valid values 1-3. Default Value 1.*/
} BAPE_AlsSettings;

/***************************************************************************
Summary:
MPEG Channel Modes
***************************************************************************/
typedef enum BAPE_MpegChannelMode
{
    BAPE_MpegChannelMode_eStereo,
    BAPE_MpegChannelMode_eIntensityStereo,
    BAPE_MpegChannelMode_eDualChannel,
    BAPE_MpegChannelMode_eSingleChannel,
    BAPE_MpegChannelMode_eMax
} BAPE_MpegChannelMode;

/***************************************************************************
Summary:
MPEG Emphasis Modes
***************************************************************************/
typedef enum BAPE_MpegEmphasisMode
{
    BAPE_MpegEmphasisMode_eNone,        /* No emphasis */
    BAPE_MpegEmphasisMode_e50_15ms,     /* 50/15 ms */
    BAPE_MpegEmphasisMode_eReserved,    /* Reserved */
    BAPE_MpegEmphasisMode_eCcit_J17,    /* CCIT J.17 */
    BAPE_MpegEmphasisMode_eMax
} BAPE_MpegEmphasisMode;

/***************************************************************************
Summary:
MPEG Codec Status
***************************************************************************/
typedef struct BAPE_MpegStatus
{
    bool original;                                  /* true if the stream is marked as original */
    bool copyright;                                 /* true if copyright is indicated in the stream */
    bool crcPresent;                                /* true if CRC's are present */
    unsigned samplingFrequency;
    unsigned                bitRate;                /* Bitrate in kbps.  0 if not specified. */
    unsigned                layer;                  /* 1 for Layer 1, 2 for Layer 2, etc. */
    BAPE_ChannelMode        channelMode;            /* Codec-independent channel mode */
    BAPE_MpegChannelMode    mpegChannelMode;        /* Stream Channel Mode */
    BAPE_MpegEmphasisMode   emphasisMode;           /* Emphasis mode extracted from the stream. */
} BAPE_MpegStatus;

/***************************************************************************
Summary:
AC3 Bitstream Mode
***************************************************************************/
typedef enum BAPE_Ac3Bsmod
{
    BAPE_Ac3Bsmod_eMainAudioCM,         /* Main Audio Service - Complete Main */
    BAPE_Ac3Bsmod_eMainAudioME,         /* Main Audio Service - Music and Effects */
    BAPE_Ac3Bsmod_eAssociatedVI,        /* Associated Service - Visually Impaired */
    BAPE_Ac3Bsmod_eAssociatedHI,        /* Associated Service - Hearing Impaired */
    BAPE_Ac3Bsmod_eAssociatedD,         /* Associated Service - Dialog */
    BAPE_Ac3Bsmod_eAssociatedC,         /* Associated Service - Commentary */
    BAPE_Ac3Bsmod_eAssociatedE,         /* Associated Service - Emergency */
    BAPE_Ac3Bsmod_eAcmodDepenedent,     /* Stream Identified by ACMOD.  If ACMOD = 001 (BAPE_ChannelMode_e1_0) is associated voice-over.  Anything else is Karaoke. */
    BAPE_Ac3Bsmod_eMax
} BAPE_Ac3Bsmod;

/***************************************************************************
Summary:
AC3 Channel Coding Mode
***************************************************************************/
typedef enum BAPE_Ac3Acmod
{
    BAPE_Ac3Acmod_e1_ch1_ch2,
    BAPE_Ac3Acmod_e1_0_C,
    BAPE_Ac3Acmod_e2_0_L_R,
    BAPE_Ac3Acmod_e3_0_L_C_R,
    BAPE_Ac3Acmod_e2_1_L_R_S,
    BAPE_Ac3Acmod_e3_1_L_C_R_S,
    BAPE_Ac3Acmod_e2_2_L_R_LS_RS,
    BAPE_Ac3Acmod_e3_2_L_C_R_LS_RS,
    BAPE_Ac3Acmod_e4_0_L_C_R_CVH,
    BAPE_Ac3Acmod_e2_3_L_R_LS_RS_TS,
    BAPE_Ac3Acmod_e3_3_L_R_C_LS_RS_TS,
    BAPE_Ac3Acmod_e4_2_L_R_C_LS_RS_CVH,
    BAPE_Ac3Acmod_e5_0_L_R_C_LC_RC,
    BAPE_Ac3Acmod_e2_4_L_R_LS_RS_LW_RW,
    BAPE_Ac3Acmod_e4_2_L_R_LS_RS_LVH_RVH,
    BAPE_Ac3Acmod_e2_4_L_R_LS_RS_LSD_RSD,
    BAPE_Ac3Acmod_e2_4_L_R_LS_RS_LRS_RRS,
    BAPE_Ac3Acmod_e5_2_L_R_C_LS_RS_LC_RC,
    BAPE_Ac3Acmod_e3_4_L_R_C_LS_RS_LW_RW,
    BAPE_Ac3Acmod_e5_2_L_R_C_LS_RS_LVH_RVH,
    BAPE_Ac3Acmod_e3_4_L_R_C_LS_RS_LSD_RSD,
    BAPE_Ac3Acmod_e3_4_L_R_C_LS_RS_LRS_RRS,
    BAPE_Ac3Acmod_e4_3_L_R_C_LS_RS_TS_CVH,
    BAPE_Ac3Acmod_eMax
} BAPE_Ac3Acmod;

/***************************************************************************
Summary:
AC3 Center Mix Level (cmixlev)

Description:
Expressed as dB attenuation
***************************************************************************/
typedef enum BAPE_Ac3CenterMixLevel
{
    BAPE_Ac3CenterMixLevel_e3,
    BAPE_Ac3CenterMixLevel_e4_5,
    BAPE_Ac3CenterMixLevel_e6,
    BAPE_Ac3CenterMixLevel_eReserved
} BAPE_Ac3CenterMixLevel;

/***************************************************************************
Summary:
AC3 Surround Mix Level (surmixlev)

Description:
Expressed as dB attenuation
***************************************************************************/
typedef enum BAPE_Ac3SurroundMixLevel
{
    BAPE_Ac3SurroundMixLevel_e3,
    BAPE_Ac3SurroundMixLevel_e6,
    BAPE_Ac3SurroundMixLevel_e0,
    BAPE_Ac3SurroundMixLevel_eReserved
} BAPE_Ac3SurroundMixLevel;

/***************************************************************************
Summary:
AC3 Dolby Surround Mode (dsurmod)
***************************************************************************/
typedef enum BAPE_Ac3DolbySurround
{
    BAPE_Ac3DolbySurround_eNotIndicated,
    BAPE_Ac3DolbySurround_eNotEncoded,
    BAPE_Ac3DolbySurround_eEncoded,
    BAPE_Ac3DolbySurround_eReserved
} BAPE_Ac3DolbySurround;

/***************************************************************************
Summary:
AC3 Dependent Frame Channel Map Mode
***************************************************************************/
typedef enum BAPE_Ac3DependentFrameChannelMap
{
    BAPE_Ac3DependentFrameChannelMap_eReserved,
    BAPE_Ac3DependentFrameChannelMap_eC,
    BAPE_Ac3DependentFrameChannelMap_eL_R,
    BAPE_Ac3DependentFrameChannelMap_eL_C_R,
    BAPE_Ac3DependentFrameChannelMap_eL_R_l,
    BAPE_Ac3DependentFrameChannelMap_eL_C_R_l,
    BAPE_Ac3DependentFrameChannelMap_eL_R_l_r,
    BAPE_Ac3DependentFrameChannelMap_eL_C_R_l_r,
    BAPE_Ac3DependentFrameChannelMap_eL_C_R_Cvh,
    BAPE_Ac3DependentFrameChannelMap_eL_R_l_r_Ts,
    BAPE_Ac3DependentFrameChannelMap_eL_C_R_l_r_Ts,
    BAPE_Ac3DependentFrameChannelMap_eL_C_R_l_r_Cvh,
    BAPE_Ac3DependentFrameChannelMap_eL_C_R_Lc_Rc,
    BAPE_Ac3DependentFrameChannelMap_eL_R_l_r_Lw_Rw,
    BAPE_Ac3DependentFrameChannelMap_eL_R_l_r_Lvh_Rvh,
    BAPE_Ac3DependentFrameChannelMap_eL_R_l_r_Lsd_rsd,
    BAPE_Ac3DependentFrameChannelMap_eL_R_l_r_Lrs_Rrs,
    BAPE_Ac3DependentFrameChannelMap_eL_C_R_l_r_Lc_Rc,
    BAPE_Ac3DependentFrameChannelMap_eL_C_R_l_r_Lw_Rw,
    BAPE_Ac3DependentFrameChannelMap_eL_C_R_l_r_Lvh_Rvh,
    BAPE_Ac3DependentFrameChannelMap_eL_C_R_l_r_Lsg_Rsd,
    BAPE_Ac3DependentFrameChannelMap_eL_C_R_l_r_Lrs_Rrs,
    BAPE_Ac3DependentFrameChannelMap_eL_C_R_l_r_Ts_Cvh,
    BAPE_Ac3DependentFrameChannelMap_eL_R_l_r_Cs,
    BAPE_Ac3DependentFrameChannelMap_eL_C_R_l_r_Cs,
    BAPE_Ac3DependentFrameChannelMap_eL_R_l_r_Cs_Ts,
    BAPE_Ac3DependentFrameChannelMap_eL_C_R_l_r_Cs_Cvh,
    BAPE_Ac3DependentFrameChannelMap_eL_C_R_l_r_Cs_Ts,
    BAPE_Ac3DependentFrameChannelMap_eMax
} BAPE_Ac3DependentFrameChannelMap;

/***************************************************************************
Summary:
AC3 Codec Status
***************************************************************************/
typedef struct BAPE_Ac3Status
{
    unsigned                    samplingFrequency;
    uint8_t                     frameSizeCode;          /* frmsizcod - Frame Size Code.  Can be used in conjunction with fscod/sampleRateCode
                                                           to determine frame size */
    uint8_t                     bitstreamId;            /* bsid - Version of the standard the stream is compliant with */
    BAPE_Ac3Bsmod               bsmod;                  /* bsmod - indicates type of service conveyed according to the standard */
    BAPE_Ac3Acmod               acmod;                  /* acmod - Indicates channel layout */
    BAPE_ChannelMode            channelMode;            /* Codec-independent ACMOD value */
    BAPE_Ac3CenterMixLevel      centerMixLevel;         /* cmixlev - Center Mix Level */
    BAPE_Ac3SurroundMixLevel    surroundMixLevel;       /* surmixlev - Surround Mix Level */
    BAPE_Ac3DolbySurround       dolbySurround;          /* dsurmod - AC3 Dolby Surround Mode */
    bool                        lfe;                    /* lfeon - If true, the LFE channel exists in the stream. */
    bool                        copyright;              /* copyrightb - If true, the stream is marked as copyrighted */
    bool                        original;               /* origbs - If true, the stream is marked as original */
    unsigned                    bitrate;                /* Bitrate in kbps */
    BAPE_Ac3DependentFrameChannelMap dependentFrameChannelMap;  /* Dependent Frame channel map for 7.1 streams */
    unsigned                    dialnorm;               /* Current Dialog Normalization value - Possible range 0 to 31 which corresponds to 0 to -31 dB level. */
    unsigned                    previousDialnorm;       /* Previous Dialog Normalization value - Possible range 0 to 31 which corresponds to 0 to -31 dB level. */
} BAPE_Ac3Status;

typedef enum BAPE_Ac4Acmod
{
    BAPE_Ac4Acmod_e1_ch1_ch2,
    BAPE_Ac4Acmod_e1_0_C,
    BAPE_Ac4Acmod_e2_0_L_R,
    BAPE_Ac4Acmod_e3_0_L_C_R,
    BAPE_Ac4Acmod_e2_1_L_R_S,
    BAPE_Ac4Acmod_e3_1_L_C_R_S,
    BAPE_Ac4Acmod_e2_2_L_R_LS_RS,
    BAPE_Ac4Acmod_e3_2_L_C_R_LS_RS,
    BAPE_Ac4Acmod_e2_2_L_C_R_CVH,
    BAPE_Ac4Acmod_e2_3_L_R_LS_RS_TS,
    BAPE_Ac4Acmod_e3_3_L_R_C_LS_RS_TS,
    BAPE_Ac4Acmod_e3_3_L_R_C_LS_RS_CVH,
    BAPE_Ac4Acmod_e3_2_L_R_C_LC_RC,
    BAPE_Ac4Acmod_e2_4_L_R_LS_RS_LW_RW,
    BAPE_Ac4Acmod_e2_4_L_R_LS_RS_LVH_RVH,
    BAPE_Ac4Acmod_e2_4_L_R_LS_RS_LSD_RSD,
    BAPE_Ac4Acmod_e2_4_L_R_LS_RS_LRS_RRS,
    BAPE_Ac4Acmod_e3_4_L_R_C_LS_RS_LC_RC,
    BAPE_Ac4Acmod_e3_4_L_R_C_LS_RS_LW_RW,
    BAPE_Ac4Acmod_e3_4_L_R_C_LS_RS_LVH_RVH,
    BAPE_Ac4Acmod_e3_4_L_R_C_LS_RS_LSD_RSD,
    BAPE_Ac4Acmod_e3_4_L_R_C_LS_RS_LRS_RRS,
    BAPE_Ac4Acmod_e3_4_L_R_C_LS_RS_TS_CVH,
    BAPE_Ac4Acmod_eMax
} BAPE_Ac4Acmod;

/***************************************************************************
Summary:
AC4 Codec Status
***************************************************************************/
typedef struct BAPE_Ac4Status
{
    unsigned                    samplingFrequency;
    uint8_t                     bitstreamId;            /* bsid - Version of the standard the stream is compliant with */
    BAPE_Ac4Acmod               acmod;                  /* acmod - Indicates channel layout */
    BAPE_ChannelMode            channelMode;            /* Codec-independent ACMOD value */
    bool                        lfe;                    /* lfeon - If true, the LFE channel exists in the stream. */
    unsigned                    bitrate;                /* Bitrate in kbps */
    unsigned                    dialnorm;               /* Current Dialog Normalization value - Possible range 0 to 31 which corresponds to 0 to -31 dB level. */
    unsigned                    previousDialnorm;       /* Previous Dialog Normalization value - Possible range 0 to 31 which corresponds to 0 to -31 dB level. */

    /* AC-4 specific stream info */
    unsigned streamInfoVersion;                                        /* Identifies which version of the decoder stream info is being provided */
    unsigned numPresentations;                                         /* Identifies the number of presentations present in compressed bitstream.
                                                                           Values greater than NEXUS_AUDIO_AC4_MAX_PRESENTATIONS should be ignored. */
    unsigned currentPresentationIndex;                                 /* Index to the current Presentation that is being decoded. */
    char currentPresentationId[BAPE_AC4_PRESENTATION_ID_LENGTH];    /* Id of the current Presentation that is being decoded. */
    unsigned dialogEnhanceMax;                                         /* Specifies the maximum value that will be honored as
                                                                          a Dialog Enhance Amount Value. Possible range 0 to 12.
                                                                          Values outside of this range should be ignored */
} BAPE_Ac4Status;

/***************************************************************************
Summary:
AAC Profiles
***************************************************************************/
typedef enum BAPE_AacProfile
{
    BAPE_AacProfile_eMain,
    BAPE_AacProfile_eLowComplexity,
    BAPE_AacProfile_eScalableSamplingRate,
    BAPE_AacProfile_eMax
} BAPE_AacProfile;

/***************************************************************************
Summary:
AAC Codec Status
***************************************************************************/
typedef struct BAPE_AacStatus
{
    unsigned samplingFrequency;                     /* Stream sample rate */
    BAPE_AacProfile profile;                        /* AAC Profile */
    BAPE_ChannelMode channelMode;                   /* Codec-independent ACMOD value */
    unsigned bitRate;                               /* Bitrate in kbps */
    bool lfe;                                       /* True if the LFE channel exists in the steram */
    bool pseudoSurround;                            /* True if pseudo-surround decoding is enabled */
    bool drc;                                       /* True if DRC information is present in the stream */
    bool stereoMatrix;                              /* True if a stereo downmix matrix is present */
    unsigned matrixIndex;                           /* A two-bit value indicating the coefficient to be used in a stereo downmix */
    unsigned numLfeChannels;
    unsigned numBackChannels;
    unsigned numSideChannels;
    unsigned numFrontChannels;
    unsigned dialnorm;                              /* Current Dialog Normalization value - Dialnorm value in steps of 0.25 dB. Range: 0-127 (0 to -31.75dB). */
    unsigned previousDialnorm;                      /* Previous Dialog Normalization value - Dialnorm value in steps of 0.25 dB. Range: 0-127 (0 to -31.75dB). */
} BAPE_AacStatus;

/***************************************************************************
Summary:
WMA Codec Status
***************************************************************************/
typedef struct BAPE_WmaStatus
{
    unsigned bitRate;               /* Bitrate in kbps */
    bool     copyright;             /* If true, the stream is marked as copyrighted */
    bool     original;              /* If true, the stream is marked as original */
    bool     crc;                   /* True if CRC is present in the stream */
    BAPE_ChannelMode channelMode;   /* Codec-independent ACMOD value */
    unsigned          version;      /* Version of the stream (Currently 1 or 2) */
} BAPE_WmaStatus;

/***************************************************************************
Summary:
WMA Pro Codec Status
***************************************************************************/
typedef struct BAPE_WmaProStatus
{
    unsigned bitRate;           /* In kbps */
    bool original;
    bool copyright;
    bool crc;
    bool lfe;
    unsigned version;           /* Version - 1 or 2 is WMA Standard, 3 is WMA Pro */
    BAPE_WmaProStereoMode stereoMode;
    BAPE_ChannelMode channelMode;           /* Codec-independent ACMOD value */
} BAPE_WmaProStatus;

/***************************************************************************
Summary:
DTS Audio Coding Mode (AMODE) values
***************************************************************************/
typedef enum BAPE_DtsAmode
{
    BAPE_DtsAmode_eOneCh_A,
    BAPE_DtsAmode_eTwoCh_A_B,
    BAPE_DtsAmode_eTwoCh_L_R,
    BAPE_DtsAmode_eTwoCh_LpR_LmR,
    BAPE_DtsAmode_eTwoCh_LT_RT,
    BAPE_DtsAmode_eThreeCh_C_L_R,
    BAPE_DtsAmode_eThreeCh_L_R_S,
    BAPE_DtsAmode_eFourCh_C_L_R_S,
    BAPE_DtsAmode_eFourCh_L_R_SL_SR,
    BAPE_DtsAmode_eFiveCh_C_L_R_SL_SR,
    BAPE_DtsAmode_eSixCh_CL_CR_L_R_SL_SR,
    BAPE_DtsAmode_eSixCh_C_L_R_LR_RR_OV,
    BAPE_DtsAmode_eSixCh_CF_CR_LF_RF_Lr_Rr,
    BAPE_DtsAmode_eSevenCh_CL_C_CR_L_R_SL_SR,
    BAPE_DtsAmode_eEightCh_CL_CR_L_R_SL1_SL2_SR1_SR2,
    BAPE_DtsAmode_eEightCh_CL_C_CR_L_R_SL_S_SR,
    BAPE_DtsAmode_eUserDefined,
    BAPE_DtsAmode_eMax
}BAPE_DtsAmode;

/***************************************************************************
Summary:
DTS stream copy history
***************************************************************************/
typedef enum BAPE_DtsCopyHistory
{
    BAPE_DtsCopyHistory_eCopyProhibited,
    BAPE_DtsCopyHistory_eFirstGeneration,
    BAPE_DtsCopyHistory_eSecondGeneration,
    BAPE_DtsCopyHistory_eOriginal,
    BAPE_DtsCopyHistory_eMax
} BAPE_DtsCopyHistory;

/***************************************************************************
Summary:
DTS extension types
***************************************************************************/
typedef enum BAPE_DtsExtension
{
    BAPE_DtsExtension_eXCh,       /* Channel Extension supporting 5.1 plus center surround channel */
    BAPE_DtsExtension_eXXCh,      /* Channel Extension supporting additional channels beyond 5.1 */
    BAPE_DtsExtension_eX96k,      /* Frequency Extension */
    BAPE_DtsExtension_eReserved,
    BAPE_DtsExtension_eMax
} BAPE_DtsExtension;

/***************************************************************************
Summary:
DTS Codec Status
***************************************************************************/
typedef struct BAPE_DtsStatus
{
    BAPE_DtsAmode amode;                    /* DTS audio channel arrangement */
    unsigned pcmResolution;                 /* Quantization resolution of source PCM samples (in bits) */
    BAPE_DtsCopyHistory copyHistory;        /* Copy history of the stream */
    BAPE_DtsExtension extensionDescriptor;  /* Extension descriptor code from the stream header. Valid if extensionPresent is true */
    unsigned bitRate;                       /* Bitrate in kbps.  0 for Open, Variable, or Lossless coded streams (bitRateCode=29..31) */
    unsigned version;                       /* Version Number */
    bool esFormat;                          /* If true, the left and right surround channels are mastered in ES format */
    bool lfe;                               /* True if LFE is present */
    bool extensionPresent;                  /* True if extended audio coding data is present in the stream */
    bool crc;                               /* True if a CRC is present */
    bool hdcdFormat;                        /* True if the source content is mastered in HDCD format */
    bool drc;                               /* True if DRC (dynamic range compression) is present */
    bool downmixCoefficients;               /* True if downmix coefficients are present in the stream */
    bool neo;                               /* True if DTS NEO is enabled */
    unsigned frameSize;                     /* Total bytes in the current frame including primary and extension data */
    unsigned numChannels;                   /* Total number of channels (primary+LFE) supported by the decoder depending on user configuration */
    unsigned pcmFrameSize;                  /* Total size of the current PCM frame in bytes (valid if frameSize ranges between 95..16383) */
    unsigned numPcmBlocks;                  /* Number of coded PCM blocks */
    BAPE_ChannelMode channelMode;           /* Codec-independent ACMOD value */
    unsigned samplingFrequency;            /* Stream sample rate */
} BAPE_DtsStatus;

/***************************************************************************
Summary:
PCMWAV Codec Status
***************************************************************************/
typedef struct BAPE_PcmWavStatus
{
    unsigned numChannels;
    unsigned samplingFrequency;
} BAPE_PcmWavStatus;

/***************************************************************************
Summary:
AMR Bitrate values
***************************************************************************/
typedef enum BAPE_AmrBitrate
{
    BAPE_AmrBitrate_e4_75kbps,
    BAPE_AmrBitrate_e5_15kbps,
    BAPE_AmrBitrate_e5_90kbps,
    BAPE_AmrBitrate_e6_70kbps,
    BAPE_AmrBitrate_e7_40kbps,
    BAPE_AmrBitrate_e7_95kbps,
    BAPE_AmrBitrate_e10_20kbps,
    BAPE_AmrBitrate_e12_20kbps,
    BAPE_AmrBitrate_eSilence,
    BAPE_AmrBitrate_eMax
} BAPE_AmrBitrate;

/***************************************************************************
Summary:
AMR Codec Status
***************************************************************************/
typedef struct BAPE_AmrStatus
{
    BAPE_AmrBitrate bitRate;
} BAPE_AmrStatus;

/***************************************************************************
Summary:
DRA Audio Coding Mode (ACMOD) values
***************************************************************************/
typedef enum BAPE_DraAcmod
{
    BAPE_DraAcmod_e1_0_C,
    BAPE_DraAcmod_e2_0_LR,
    BAPE_DraAcmod_e2_1_LRS,
    BAPE_DraAcmod_e2_2_LRLrRr,
    BAPE_DraAcmod_e3_2_LRLrRrC,
    BAPE_DraAcmod_e3_3_LRLrRrCrC,
    BAPE_DraAcmod_e5_2_LRLrRrLsRsC,
    BAPE_DraAcmod_e5_3_LRLrRrLsRsCrC,
    BAPE_DraAcmod_eMax
} BAPE_DraAcmod;

/***************************************************************************
Summary:
DRA Stereo Modes
***************************************************************************/
typedef enum BAPE_DraStereoMode
{
    BAPE_DraStereoMode_eLoRo,
    BAPE_DraStereoMode_eLtRt,
    BAPE_DraStereoMode_eMax
} BAPE_DraStereoMode;

/***************************************************************************
Summary:
DRA Codec Status
***************************************************************************/
typedef struct BAPE_DraStatus
{
    unsigned frameSize;             /* In bytes */
    unsigned numBlocks;             /* Number of short window mdct blocks in the frame.  There are 128 samples per block. */
    BAPE_DraAcmod acmod;            /* Channel coding mode */
    bool lfe;
    BAPE_ChannelMode channelMode;   /* Codec-independent ACMOD value */
    BAPE_DraStereoMode stereoMode;
    unsigned samplingFrequency;
} BAPE_DraStatus;

/***************************************************************************
Summary:
Cook Codec Status
***************************************************************************/
typedef struct BAPE_CookStatus
{
    bool stereo;        /* True if stereo, false if mono */
    unsigned frameSize; /* Frame size in bytes */
    unsigned samplingFrequency; /* The sampling frequency value, 0 in case of error, otherwise sampling frequency as it's */
} BAPE_CookStatus;

/***************************************************************************
Summary:
ALS Codec Status
***************************************************************************/
typedef struct BAPE_AlsStatus
{
    unsigned samplingFrequency; /* Stream sample rate */
    unsigned bitsPerSample; /* input sample bit width */
    BAPE_ChannelMode channelMode;   /* Codec-independent ACMOD value */
} BAPE_AlsStatus;

#endif /* #ifndef BAPE_CODEC_TYPES_H_ */
