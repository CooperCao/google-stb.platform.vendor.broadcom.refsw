/***************************************************************************
 *     Copyright (c) 2006-2010, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description: Audio Decoder Interface
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/

#ifndef BAPE_CODEC_TYPES_H_
#define BAPE_CODEC_TYPES_H_

/***************************************************************************
Summary:
AC3 DRC (Dynamic Range Compression) Modes
***************************************************************************/
typedef enum BAPE_Ac3DrcMode
{
    BAPE_Ac3DrcMode_eLine,
    BAPE_Ac3DrcMode_eRf,
    BAPE_Ac3DrcMode_eCustomA,
    BAPE_Ac3DrcMode_eCustomD,
    BAPE_Ac3DrcMode_eDisabled,
    BAPE_Ac3DrcMode_eMax
} BAPE_Ac3DrcMode;

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
    BAPE_Ac3DrcMode drcMode;    /* DRC (Dynamic Range Compression) Mode */
    uint16_t drcScaleHi;        /* In %, ranges from 0..100 */
    uint16_t drcScaleLow;       /* In %, ranges from 0..100 */

    BAPE_Ac3StereoMode stereoMode;  /* Stereo Downmix Mode */
} BAPE_Ac3Settings;

/***************************************************************************
Summary:
AAC Stereo Downmix Modes
***************************************************************************/
typedef enum BAPE_AacStereoMode
{
    BAPE_AacStereoMode_eMatrix,     /* Default */
    BAPE_AacStereoMode_eArib,       /* ARIB Compatible */
    BAPE_AacStereoMode_eMax
} BAPE_AacStereoMode;

/***************************************************************************
Summary:
AAC Codec Settings
***************************************************************************/
typedef struct BAPE_AacSettings
{
    uint16_t drcScaleHi;            /* In %, ranges from 0..100 */
    uint16_t drcScaleLow;           /* In %, ranges from 0..100 */
    uint8_t  drcTargetLevel;        /* DRC (Dynamic Range Compression) Target Level.  Ranges from 0..127  */
    
    BAPE_AacStereoMode downmixMode; /* AAC Downmix Mode */
} BAPE_AacSettings;

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
    BAVC_AudioSamplingRate  originalSampleRate;     /* Stream sample rate */
    uint32_t                sampleRateCode;         /* Sample Rate Code extracted from stream */
    unsigned                bitRate;                /* Bitrate in kbps.  0 if not specified. */
    uint32_t                bitRateCode;            /* Bit Rate Code extracted from stream */
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
    BAPE_Ac3Acmod_e1_1,     /* 1+1 Dual Mono */
    BAPE_Ac3Acmod_e1_0,     /* Center Mono */
    BAPE_Ac3Acmod_e2_0,     /* Left Right Stereo */
    BAPE_Ac3Acmod_e3_0,     /* Left, Center, Right */
    BAPE_Ac3Acmod_e2_1,     /* Left, Right, Surround */
    BAPE_Ac3Acmod_e3_1,     /* Left, Center, Right, Surround */
    BAPE_Ac3Acmod_e2_2,     /* Left, Right, LeftSurround, RightSurround */
    BAPE_Ac3Acmod_e3_2,     /* Left, Center, Right, LeftSurround, RightSurround */
    BAPE_Ac3Acmod_eMax
} BAPE_Ac3Acmod;

/***************************************************************************
Summary:
AC3 Dolby Surround Mode (dsurmod)
***************************************************************************/
typedef enum BAPE_Ac3DolbySurround
{
    BAPE_Ac3DolbySurround_eNotIndicated,
    BAPE_Ac3DolbySurround_eNotEncoded,
    BAPE_Ac3DolbySurround_eEncoded,
    BAPE_Ac3DolbySurround_eReserved,
    BAPE_Ac3DolbySurround_eMax
} BAPE_Ac3DolbySurround;

/***************************************************************************
Summary:
AC3 Codec Status
***************************************************************************/
typedef struct BAPE_Ac3Status
{
    BAVC_AudioSamplingRate  originalSampleRate;     /* Stream sample rate */
    uint8_t                 sampleRateCode;         /* fscod - Sample Rate Code */
    uint8_t                 frameSizeCode;          /* frmsizcod - Frame Size Code.  Can be used in conjunction with fscod/sampleRateCode 
                                                       to determine frame size */
    uint8_t                 bitstreamId;            /* bsid - Version of the standard the stream is compliant with */
    BAPE_Ac3Bsmod           bsmod;                  /* bsmod - indicates type of service conveyed according to the standard */
    BAPE_Ac3Acmod           acmod;                  /* acmod - Indicates channel layout */
    BAPE_ChannelMode        channelMode;            /* Codec-independent ACMOD value */
    BAPE_Ac3DolbySurround   dolbySurround;          /* dsurmod - AC3 Dolby Surround Mode */
    bool                    lfe;                    /* lfeon - If true, the LFE channel exists in the stream. */
    bool                    copyright;              /* copyrightb - If true, the stream is marked as copyrighted */
    bool                    original;               /* origbs - If true, the stream is marked as original */
} BAPE_Ac3Status;

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
    BAVC_AudioSamplingRate originalSampleRate;      /* Stream sample rate */
    BAPE_AacProfile profile;                        /* AAC Profile */
    BAPE_ChannelMode channelMode;                   /* Codec-independent ACMOD value */
    unsigned bitRate;                               /* Bitrate in kbps */
    bool lfe;                                       /* True if the LFE channel exists in the steram */
    bool pseudoSurround;                            /* True if pseudo-surround decoding is enabled */
    bool drc;                                       /* True if DRC information is present in the stream */
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
    bool     lfe;                   /* True if the LFE channel exists in the steram */
    BAPE_ChannelMode channelMode;   /* Codec-independent ACMOD value */
    unsigned          version;      /* Version of the stream (Currently 1 or 2) */
    BAVC_AudioSamplingRate  originalSampleRate;     /* Stream sample rate */
} BAPE_WmaStatus;

#endif /* #ifndef BAPE_CODEC_TYPES_H_ */

