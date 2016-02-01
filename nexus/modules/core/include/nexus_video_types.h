/***************************************************************************
*     (c)2004-2013 Broadcom Corporation
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
* API Description:
*   API name: Base
*
* Revision History:
*
* $brcm_Log: $
*
***************************************************************************/
#ifndef NEXUS_VIDEO_TYPES_H
#define NEXUS_VIDEO_TYPES_H

#ifdef __cplusplus
extern "C"
{
#endif

/*
Summary:
Rate measured in frames per second.

Description:
23_976 means 23.976 frames per second.

See Also:
NEXUS_VideoDecoderStatus
*/
typedef enum NEXUS_VideoFrameRate {
    NEXUS_VideoFrameRate_eUnknown = 0,
    NEXUS_VideoFrameRate_e23_976,
    NEXUS_VideoFrameRate_e24,
    NEXUS_VideoFrameRate_e25,
    NEXUS_VideoFrameRate_e29_97,
    NEXUS_VideoFrameRate_e30,
    NEXUS_VideoFrameRate_e50,
    NEXUS_VideoFrameRate_e59_94,
    NEXUS_VideoFrameRate_e60,
    NEXUS_VideoFrameRate_e14_985,
    NEXUS_VideoFrameRate_e7_493,
    NEXUS_VideoFrameRate_e10,
    NEXUS_VideoFrameRate_e15,
    NEXUS_VideoFrameRate_e20,
    NEXUS_VideoFrameRate_e12_5,
    NEXUS_VideoFrameRate_e100,
    NEXUS_VideoFrameRate_e119_88,
    NEXUS_VideoFrameRate_e120,
    NEXUS_VideoFrameRate_e19_98,
    NEXUS_VideoFrameRate_eMax
} NEXUS_VideoFrameRate;

/*
Summary:
Video signal polarity

Description:
Used in NEXUS_AnalogVideoStatus
*/
typedef enum NEXUS_VideoPolarity {
    NEXUS_VideoPolarity_ePositive,
    NEXUS_VideoPolarity_eNegative
} NEXUS_VideoPolarity ;

/*
Summary:
Video buffer type

Description:
Used in NEXUS_StripedSurfaceCreateSettings
*/
typedef enum NEXUS_VideoBufferType
{
   NEXUS_VideoBufferType_eFrame = 1,
   NEXUS_VideoBufferType_eFieldPair,
   NEXUS_VideoBufferType_eTopField,
   NEXUS_VideoBufferType_eBotField,
   NEXUS_VideoBufferType_eMax
} NEXUS_VideoBufferType;

/*
Summary:
Scanout for a picture

Description:
Used in NEXUS_VideoImageInputSurfaceSettings
*/
typedef enum NEXUS_PicturePullDown
{
   NEXUS_PicturePullDown_eFrame,
   NEXUS_PicturePullDown_eTop,
   NEXUS_PicturePullDown_eBottom,
   NEXUS_PicturePullDown_eTopBottom,
   NEXUS_PicturePullDown_eBottomTop,
   NEXUS_PicturePullDown_eTopBottomTop,
   NEXUS_PicturePullDown_eBottomTopBottom,
   NEXUS_PicturePullDown_eMax
} NEXUS_PicturePullDown;

/*
Summary:
Codec used for video compression.

Description:
See Also:
NEXUS_VideoDecoderStartSettings
NEXUS_VideoDecoderModuleSettings
*/
typedef enum NEXUS_VideoCodec {
    NEXUS_VideoCodec_eUnknown = 0,     /* unknown/not supported video codec */
    NEXUS_VideoCodec_eNone = 0,        /* unknown/not supported video codec */
    NEXUS_VideoCodec_eMpeg1,           /* MPEG-1 Video (ISO/IEC 11172-2) */
    NEXUS_VideoCodec_eMpeg2,           /* MPEG-2 Video (ISO/IEC 13818-2) */
    NEXUS_VideoCodec_eMpeg4Part2,      /* MPEG-4 Part 2 Video */
    NEXUS_VideoCodec_eH263,            /* H.263 Video. The value of the enum is not based on PSI standards. */
    NEXUS_VideoCodec_eH264,            /* H.264 (ITU-T) or ISO/IEC 14496-10/MPEG-4 AVC */
    NEXUS_VideoCodec_eH264_Svc,        /* Deprecated: Scalable Video Codec extension of H.264 */
    NEXUS_VideoCodec_eH264_Mvc,        /* Multi View Coding extension of H.264 */
    NEXUS_VideoCodec_eVc1,             /* VC-1 Advanced Profile */
    NEXUS_VideoCodec_eVc1SimpleMain,   /* VC-1 Simple & Main Profile */
    NEXUS_VideoCodec_eDivx311,         /* DivX 3.11 coded video */
    NEXUS_VideoCodec_eAvs,             /* AVS coded video */
    NEXUS_VideoCodec_eRv40,            /* RV 4.0 coded video */
    NEXUS_VideoCodec_eVp6,             /* VP6 coded video */
    NEXUS_VideoCodec_eVp7,             /* VP7 coded video */
    NEXUS_VideoCodec_eVp8,             /* VP8 coded video */
    NEXUS_VideoCodec_eVp9,             /* VP9 coded video */
    NEXUS_VideoCodec_eSpark,           /* H.263 Sorenson Spark coded video */
    NEXUS_VideoCodec_eMotionJpeg,      /* Motion Jpeg video codec */
    NEXUS_VideoCodec_eH265,            /* HEVC,H.265 ITU-T SG16 WP3 and ISO/IEC JTC1/SC29/WG11 */
    NEXUS_VideoCodec_eMax
} NEXUS_VideoCodec;

/*
Summary:
Video formats

Description:
See Also:
NEXUS_DisplaySettings
*/
typedef enum NEXUS_VideoFormat {
    NEXUS_VideoFormat_eUnknown = 0,     /* unknown/not supported video format */
    NEXUS_VideoFormat_eNtsc,            /* 480i, NTSC-M for North America */
    NEXUS_VideoFormat_eNtsc443,         /* NTSC encoding with the PAL color carrier frequency. */
    NEXUS_VideoFormat_eNtscJapan,       /* Japan NTSC, no pedestal level */
    NEXUS_VideoFormat_ePalM,            /* PAL Brazil */
    NEXUS_VideoFormat_ePalN,            /* PAL_N */
    NEXUS_VideoFormat_ePalNc,           /* PAL_N, Argentina */
    NEXUS_VideoFormat_ePalB,            /* Australia */
    NEXUS_VideoFormat_ePalB1,           /* Hungary */
    NEXUS_VideoFormat_ePalD,            /* China */
    NEXUS_VideoFormat_ePalD1,           /* Poland */
    NEXUS_VideoFormat_ePalDK1=NEXUS_VideoFormat_ePalD1,          /* Eastern Europe */
    NEXUS_VideoFormat_ePalDK2,          /* Eastern Europe */
    NEXUS_VideoFormat_ePalDK3,          /* Eastern Europe */
    NEXUS_VideoFormat_ePalG,            /* Europe. Same as NEXUS_VideoFormat_ePal. */
    NEXUS_VideoFormat_ePal = NEXUS_VideoFormat_ePalG,     /* PAL Europe */
    NEXUS_VideoFormat_ePalH,            /* Europe */
    NEXUS_VideoFormat_ePalK,            /* Europe */
    NEXUS_VideoFormat_ePalI,            /* U.K. */
    NEXUS_VideoFormat_ePal60hz,         /* 60Hz PAL */
    NEXUS_VideoFormat_eSecamL,          /* France */
    NEXUS_VideoFormat_eSecam = NEXUS_VideoFormat_eSecamL, /* Backward compatibility */
    NEXUS_VideoFormat_eSecamB,          /* Middle East */
    NEXUS_VideoFormat_eSecamG,          /* Middle East */
    NEXUS_VideoFormat_eSecamD,          /* Eastern Europe */
    NEXUS_VideoFormat_eSecamK,          /* Eastern Europe */
    NEXUS_VideoFormat_eSecamH,          /* Line SECAM */
    NEXUS_VideoFormat_e480p,            /* NTSC Progressive (27Mhz) */
    NEXUS_VideoFormat_e576p,            /* HD PAL Progressive 50hz for Australia */
    NEXUS_VideoFormat_e1080i,           /* HD 1080 Interlaced */
    NEXUS_VideoFormat_e1080i50hz,       /* European 50hz HD 1080 */
    NEXUS_VideoFormat_e1080p24hz,       /* HD 1080 Progressive, 24hz */
    NEXUS_VideoFormat_e1080p25hz,       /* HD 1080 Progressive, 25hz */
    NEXUS_VideoFormat_e1080p30hz,       /* HD 1080 Progressive, 30hz */
    NEXUS_VideoFormat_e1080p50hz,       /* HD 1080 Progressive, 50hz. */
    NEXUS_VideoFormat_e1080p60hz,       /* HD 1080 Progressive, 60hz */
    NEXUS_VideoFormat_e1080p = NEXUS_VideoFormat_e1080p60hz,
    NEXUS_VideoFormat_e1080p100hz,      /* HD 1080 Progressive, 100hz */
    NEXUS_VideoFormat_e1080p120hz,      /* HD 1080 Progressive, 120hz */
    NEXUS_VideoFormat_e1250i50hz,       /* HD 1250 Interlaced, 50hz */
    NEXUS_VideoFormat_e720p,            /* HD 720 Progressive */
    NEXUS_VideoFormat_e720p50hz,        /* HD 720p 50hz for Australia */
    NEXUS_VideoFormat_e720p24hz,        /* HD 720p 24hz */
    NEXUS_VideoFormat_e720p25hz,        /* HD 720p 25hz */
    NEXUS_VideoFormat_e720p30hz,        /* HD 720p 30hz */
    NEXUS_VideoFormat_e240p60hz,        /* NTSC 240p */
    NEXUS_VideoFormat_e288p50hz,        /* PAL 288p */
    NEXUS_VideoFormat_e1440x480p60hz,   /* CEA861B */
    NEXUS_VideoFormat_e1440x576p50hz,   /* CEA861B */
    NEXUS_VideoFormat_e3840x2160p24hz,  /* UHD 3840x2160 24Hz */
    NEXUS_VideoFormat_e3840x2160p25hz,  /* UHD 3840x2160 25Hz */
    NEXUS_VideoFormat_e3840x2160p30hz,  /* UHD 3840x2160 30Hz */
    NEXUS_VideoFormat_e3840x2160p50hz,  /* UHD 3840x2160 50Hz */
    NEXUS_VideoFormat_e3840x2160p60hz,  /* UHD 3840x2160 60Hz */
    NEXUS_VideoFormat_e4096x2160p24hz,  /* UHD 4096x2160 24Hz */
    NEXUS_VideoFormat_e4096x2160p25hz,  /* UHD 4096x2160 25Hz */
    NEXUS_VideoFormat_e4096x2160p30hz,  /* UHD 4096x2160 30Hz */
    NEXUS_VideoFormat_e4096x2160p50hz,  /* UHD 4096x2160 50Hz */
    NEXUS_VideoFormat_e4096x2160p60hz,  /* UHD 4096x2160 60Hz */
    NEXUS_VideoFormat_eCustomer1440x240p60hz,             /* 240p 60hz 7411 custom format. */
    NEXUS_VideoFormat_eCustomer1440x288p50hz,             /* 288p 50hz 7411 custom format. */
    NEXUS_VideoFormat_eCustomer1366x768p60hz,             /* Custom 1366x768 mode */
    NEXUS_VideoFormat_eCustomer1366x768p50hz,             /* Custom 1366x768 50hz mode */
    NEXUS_VideoFormat_eVesa640x480p60hz,                  /* DVI Safe mode for computer monitors */
    NEXUS_VideoFormat_eVesa800x600p60hz,                  /* DVI VESA mode for computer monitors */
    NEXUS_VideoFormat_eVesa1024x768p60hz,                 /* DVI VESA mode for computer monitors */
    NEXUS_VideoFormat_eVesa1280x768p60hz,                 /* DVI VESA mode for computer monitors */
    NEXUS_VideoFormat_eVesa1280x768p60hzRed,              /* DVI VESA mode for computer monitors */
    NEXUS_VideoFormat_eVesa1280x720p50hz,                 /* DVI VESA mode for computer monitors */
    NEXUS_VideoFormat_eVesa1280x720p60hz,                 /* DVI VESA mode for computer monitors */
    NEXUS_VideoFormat_eVesa1280x720pReducedBlank,         /* DVI VESA mode for computer monitors */
    NEXUS_VideoFormat_eVesa640x350p60hz,                  /* DVI VESA mode for computer monitors */
    NEXUS_VideoFormat_eVesa640x350p70hz,                  /* DVI VESA mode for computer monitors */
    NEXUS_VideoFormat_eVesa640x350p72hz,                  /* DVI VESA mode for computer monitors */
    NEXUS_VideoFormat_eVesa640x350p75hz,                  /* DVI VESA mode for computer monitors */
    NEXUS_VideoFormat_eVesa640x350p85hz,                  /* DVI VESA mode for computer monitors */
    NEXUS_VideoFormat_eVesa640x400p60hz,                  /* DVI VESA mode for computer monitors */
    NEXUS_VideoFormat_eVesa640x400p70hz,                  /* DVI VESA mode for computer monitors */
    NEXUS_VideoFormat_eVesa640x400p72hz,                  /* DVI VESA mode for computer monitors */
    NEXUS_VideoFormat_eVesa640x400p75hz,                  /* DVI VESA mode for computer monitors */
    NEXUS_VideoFormat_eVesa640x400p85hz,                  /* DVI VESA mode for computer monitors */
    NEXUS_VideoFormat_eVesa640x480p66hz,                  /* DVI VESA mode for computer monitors */
    NEXUS_VideoFormat_eVesa640x480p70hz,                  /* DVI VESA mode for computer monitors */
    NEXUS_VideoFormat_eVesa640x480p72hz,                  /* DVI VESA mode for computer monitors */
    NEXUS_VideoFormat_eVesa640x480p75hz,                  /* DVI VESA mode for computer monitors */
    NEXUS_VideoFormat_eVesa640x480p85hz,                  /* DVI VESA mode for computer monitors */
    NEXUS_VideoFormat_eVesa720x400p60hz,                  /* DVI VESA mode for computer monitors */
    NEXUS_VideoFormat_eVesa720x400p70hz,                  /* DVI VESA mode for computer monitors */
    NEXUS_VideoFormat_eVesa720x400p72hz,                  /* DVI VESA mode for computer monitors */
    NEXUS_VideoFormat_eVesa720x400p75hz,                  /* DVI VESA mode for computer monitors */
    NEXUS_VideoFormat_eVesa720x400p85hz,                  /* DVI VESA mode for computer monitors */
    NEXUS_VideoFormat_eVesa800x600p56hz,                  /* DVI VESA mode for computer monitors */
    NEXUS_VideoFormat_eVesa800x600p59hzRed,               /* DVI VESA mode for computer monitors */
    NEXUS_VideoFormat_eVesa800x600p70hz,                  /* DVI VESA mode for computer monitors */
    NEXUS_VideoFormat_eVesa800x600p72hz,                  /* DVI VESA mode for computer monitors */
    NEXUS_VideoFormat_eVesa800x600p75hz,                  /* DVI VESA mode for computer monitors */
    NEXUS_VideoFormat_eVesa800x600p85hz,                  /* DVI VESA mode for computer monitors */
    NEXUS_VideoFormat_eVesa848x480p60hz,                  /* DVI VESA mode for computer monitors */
    NEXUS_VideoFormat_eVesa1024x768p66hz,                 /* DVI VESA mode for computer monitors */
    NEXUS_VideoFormat_eVesa1024x768p70hz,                 /* DVI VESA mode for computer monitors */
    NEXUS_VideoFormat_eVesa1024x768p72hz,                 /* DVI VESA mode for computer monitors */
    NEXUS_VideoFormat_eVesa1024x768p75hz,                 /* DVI VESA mode for computer monitors */
    NEXUS_VideoFormat_eVesa1024x768p85hz,                 /* DVI VESA mode for computer monitors */
    NEXUS_VideoFormat_eVesa1064x600p60hz,                 /* DVI VESA mode for computer monitors */
    NEXUS_VideoFormat_eVesa1280x720p70hz,                 /* DVI VESA mode for computer monitors */
    NEXUS_VideoFormat_eVesa1280x720p72hz,                 /* DVI VESA mode for computer monitors */
    NEXUS_VideoFormat_eVesa1280x720p75hz,                 /* DVI VESA mode for computer monitors */
    NEXUS_VideoFormat_eVesa1280x720p85hz,                 /* DVI VESA mode for computer monitors */
    NEXUS_VideoFormat_eVesa1024x768i87hz,                 /* DVI VESA mode for computer monitors */
    NEXUS_VideoFormat_eVesa1152x864p75hz,                 /* DVI VESA mode for computer monitors */
    NEXUS_VideoFormat_eVesa1280x768p75hz,                 /* DVI VESA mode for computer monitors */
    NEXUS_VideoFormat_eVesa1280x768p85hz,                 /* DVI VESA mode for computer monitors */
    NEXUS_VideoFormat_eVesa1280x800p_60Hz,                /* DVI VESA mode for computer monitors */
    NEXUS_VideoFormat_eVesa1280x960p60hz,                 /* DVI VESA mode for computer monitors */
    NEXUS_VideoFormat_eVesa1280x960p85hz,                 /* DVI VESA mode for computer monitors */
    NEXUS_VideoFormat_eVesa1280x1024p60hz,                /* DVI VESA mode for computer monitors */
    NEXUS_VideoFormat_eVesa1280x1024p69hz,                /* DVI VESA mode for computer monitors */
    NEXUS_VideoFormat_eVesa1280x1024p75hz,                /* DVI VESA mode for computer monitors */
    NEXUS_VideoFormat_eVesa1280x1024p85hz,                /* DVI VESA mode for computer monitors */
    NEXUS_VideoFormat_eVesa832x624p75hz,                  /* MAC-16 */
    NEXUS_VideoFormat_eVesa1360x768p60hz,                 /* DVI VESA mode for computer monitors */
    NEXUS_VideoFormat_eVesa1366x768p60hz,                 /* DVI VESA mode for computer monitors */
    NEXUS_VideoFormat_eVesa1400x1050p60hz,                /* DVI VESA mode for computer monitors */
    NEXUS_VideoFormat_eVesa1400x1050p60hzReducedBlank,    /* DVI VESA mode for computer monitors */
    NEXUS_VideoFormat_eVesa1400x1050p75hz,                /* DVI VESA mode for computer monitors */
    NEXUS_VideoFormat_eVesa1440x900p60hz,                 /* DVI VESA mode for computer monitors */
    NEXUS_VideoFormat_eVesa1600x1200p60hz,                /* DVI VESA mode for computer monitors */
    NEXUS_VideoFormat_eVesa1920x1080p60hzReducedBlank,    /* DVI VESA mode for computer monitors */

    /* 3D source & display formats. These are all full-res O/U 3D formats. Half-res does not require a special 3D format. */
    NEXUS_VideoFormat_e720p_3DOU_AS,                      /* HD 3D 720p */
    NEXUS_VideoFormat_e720p50hz_3DOU_AS,                  /* HD 3D 720p50 */
    NEXUS_VideoFormat_e720p30hz_3DOU_AS,                  /* HD 3D 720p30 */
    NEXUS_VideoFormat_e720p24hz_3DOU_AS,                  /* HD 3D 720p24 */
    NEXUS_VideoFormat_e1080p24hz_3DOU_AS,                 /* HD 1080p 24Hz, 2750 sample per line, SMPTE 274M-1998 */
    NEXUS_VideoFormat_e1080p30hz_3DOU_AS,                 /* HD 1080p 30Hz, 2200 sample per line, SMPTE 274M-1998 */

    NEXUS_VideoFormat_eVesa1680x1050p_60Hz,               /* DVI VESA mode for computer monitors */
    NEXUS_VideoFormat_eVesa1280x800p_60Hz_Red,            /* DVI VESA mode for computer monitors */
    NEXUS_VideoFormat_eVesa1600x1200p_75Hz,               /* DVI VESA mode for computer monitors */
    NEXUS_VideoFormat_eVesa1600x900p_60Hz_Red,            /* DVI VESA mode for computer monitors */
    NEXUS_VideoFormat_eVesa1680x1050p_60Hz_Red,           /* DVI VESA mode for computer monitors */
    NEXUS_VideoFormat_eVesa1920x1200p_60Hz,               /* DVI VESA mode for computer monitors */
    NEXUS_VideoFormat_eVesa1920x1200p_60Hz_Red,           /* DVI VESA mode for computer monitors */
    NEXUS_VideoFormat_eVesa1152x720p_60Hz,                /* DVI VESA mode for computer monitors */
    NEXUS_VideoFormat_eVesa1152x720p_75Hz,                /* DVI VESA mode for computer monitors */
    NEXUS_VideoFormat_eVesa1152x720p_85Hz,                /* DVI VESA mode for computer monitors */
    NEXUS_VideoFormat_eVesa1152x864p_60Hz,                /* DVI VESA mode for computer monitors */
    NEXUS_VideoFormat_eVesa1152x864p_85Hz,                /* DVI VESA mode for computer monitors */
    NEXUS_VideoFormat_eVesa1152x870p_75Hz,                /* DVI VESA mode for computer monitors */
    NEXUS_VideoFormat_eVesa1152x900p_66Hz,                /* DVI VESA mode for computer monitors */
    NEXUS_VideoFormat_eVesa1152x900p_76Hz,                /* DVI VESA mode for computer monitors */
    NEXUS_VideoFormat_eVesa1170x584p_50Hz,                /* DVI VESA mode for computer monitors */
    NEXUS_VideoFormat_eVesa1280x1024p_70Hz,               /* DVI VESA mode for computer monitors */
    NEXUS_VideoFormat_eVesa1280x1024p_72Hz,               /* DVI VESA mode for computer monitors */
    NEXUS_VideoFormat_eVesa1280x1024p_76Hz,               /* DVI VESA mode for computer monitors */
    NEXUS_VideoFormat_eVesa1280x768p_50Hz,                /* DVI VESA mode for computer monitors */
    NEXUS_VideoFormat_eVesa1280x960p_75Hz,                /* DVI VESA mode for computer monitors */
    NEXUS_VideoFormat_eVesa1600x1024p_60Hz,               /* DVI VESA mode for computer monitors */
    NEXUS_VideoFormat_eVesa1600x1024p_76Hz,               /* DVI VESA mode for computer monitors */
    NEXUS_VideoFormat_eVesa1728x1080p_60Hz,               /* DVI VESA mode for computer monitors */
    NEXUS_VideoFormat_eVesa800x600p_100Hz,                /* DVI VESA mode for computer monitors */
    NEXUS_VideoFormat_eVesa800x600p_90Hz,                 /* DVI VESA mode for computer monitors */
    NEXUS_VideoFormat_eVesa848x480p_75Hz,                 /* DVI VESA mode for computer monitors */
    NEXUS_VideoFormat_eVesa848x480p_85Hz,                 /* DVI VESA mode for computer monitors */
    NEXUS_VideoFormat_eVesa852x480p_60Hz,                 /* DVI VESA mode for computer monitors */
    NEXUS_VideoFormat_e720x482_NTSC,                      /* 720x482i NTSC-M for North America */
    NEXUS_VideoFormat_e720x482_NTSC_J,                    /* 720x482i Japan */
    NEXUS_VideoFormat_e720x483p,                          /* 720x483p */

    /* dynamics: custom format */
    NEXUS_VideoFormat_eCustom0,         /* deprecated */
    NEXUS_VideoFormat_eCustom1,         /* deprecated */
    NEXUS_VideoFormat_eCustom2,         /* eCustom2 is defined at run time by calling NEXUS_Display_SetCustomFormatSettings.
                                           This runtime modification cannot be done for eCustom0 and eCustom1.
                                           Typically, apps set eCustom2 to be a 48Hz format, which also supports 24Hz content. */

    NEXUS_VideoFormat_eCustom1920x2160i_48Hz, /* deprecated */
    NEXUS_VideoFormat_eCustom_3D_720p,        /* deprecated */
    NEXUS_VideoFormat_eCustom_3D_720p_50hz,   /* deprecated */
    NEXUS_VideoFormat_eCustom_3D_720p_30hz,   /* deprecated */
    NEXUS_VideoFormat_eCustom_3D_720p_24hz,   /* deprecated */
    NEXUS_VideoFormat_eCustom_3D_1080p_24hz,  /* deprecated */
    NEXUS_VideoFormat_eCustom_3D_1080p_30hz,  /* deprecated */

    NEXUS_VideoFormat_eMax              /* Total number of video formats */
} NEXUS_VideoFormat;

#define NEXUS_VideoFormat_e3D_720p       NEXUS_VideoFormat_e720p_3DOU_AS       /* HD 3D 720p */
#define NEXUS_VideoFormat_e3D_720p_24Hz  NEXUS_VideoFormat_e720p24hz_3DOU_AS   /* HD 3D 720p24 */
#define NEXUS_VideoFormat_e3D_720p_30Hz  NEXUS_VideoFormat_e720p30hz_3DOU_AS   /* HD 3D 720p30 */
#define NEXUS_VideoFormat_e3D_720p_50Hz  NEXUS_VideoFormat_e720p50hz_3DOU_AS   /* HD 3D 720p50 */
#define NEXUS_VideoFormat_e3D_1080p_24Hz NEXUS_VideoFormat_e1080p24hz_3DOU_AS  /* HD 1080p 24Hz, 2750 sample per line, SMPTE 274M-1998 */
#define NEXUS_VideoFormat_e3D_1080p_30Hz NEXUS_VideoFormat_e1080p30hz_3DOU_AS  /* HD 1080p 30Hz, 2200 sample per line, SMPTE 274M-1998 */

/**
Summary:
Video aspect ratio

Description:
See Also:
NEXUS_PcInputStatus
NEXUS_VideoDecoderStatus
**/
typedef enum NEXUS_AspectRatio {
    NEXUS_AspectRatio_eUnknown   = 0, /* Unknown/Reserved */
    NEXUS_AspectRatio_eSquarePixel,   /* Square pixel. This is equivalent to NEXUS_AspectRatio_eSar 1:1. */
    NEXUS_AspectRatio_e4x3,           /* 4:3 */
    NEXUS_AspectRatio_e16x9,          /* 16:9 */
    NEXUS_AspectRatio_e221x1,         /* 2.21:1 */
    NEXUS_AspectRatio_e15x9,          /* 15:9 */
    NEXUS_AspectRatio_eSar,           /* Sample aspect ratio - aspect ratio of the source calculated as the ratio of two numbers reported by the decoder.
                                         See NEXUS_VideoDecoderStreamInformation.sampleAspectRatioX and sampleAspectRatioY for an example.
                                         This aspect ratio is applied to the picture's source size (i.e. coded size), not the picture's display size. */
    NEXUS_AspectRatio_eMax
} NEXUS_AspectRatio;


/**
Summary:
Video color space

Description:
See Also:
NEXUS_MatrixCoefficients, NEXUS_ColorRange.
**/
typedef enum NEXUS_ColorSpace {
    NEXUS_ColorSpace_eAuto,
    NEXUS_ColorSpace_eRgb,
    NEXUS_ColorSpace_eYCbCr422,   /* 2 */
    NEXUS_ColorSpace_eYCbCr444,
    NEXUS_ColorSpace_eYCbCr420,   /* 4 */
    NEXUS_ColorSpace_eMax
} NEXUS_ColorSpace;

/**
Summary:
Video color range

Description:
See Also:
NEXUS_MatrixCoefficients, NEXUS_ColorSpace.
**/
typedef enum NEXUS_ColorRange {
    NEXUS_ColorRange_eLimited,
    NEXUS_ColorRange_eFull,
    NEXUS_ColorRange_eMax
} NEXUS_ColorRange;

/* for backward compatibility */
typedef unsigned NEXUS_HdmiColorDepth;
/* HDMI color depth of 0 means "auto". Nexus will select the highest quality supported color depth. */
#define NEXUS_HdmiColorDepth_e8bit  8
#define NEXUS_HdmiColorDepth_e10bit 10
#define NEXUS_HdmiColorDepth_e12bit 12
#define NEXUS_HdmiColorDepth_e16bit 16
#define NEXUS_HdmiColorDepth_eMax   17

/**
Summary:
NEXUS_VideoOutput is an abstract connector token for routing video from to a destination.

Description:
It is used to connect specific VideoOutputs to the Display. It's also used for generic video settings and status.

All VideoOutputs also have a specific input (e.g. ComponentVideo, HdmiVideo).
This can be optionally used to control specific settings and to retrieve specific status.

See Also:
NEXUS_ComponentOutput_GetConnector
NEXUS_CompositeOutput_GetConnector
NEXUS_SvideoOutput_GetConnector
NEXUS_HdmiOutput_GetConnector
NEXUS_RfOutput_GetConnector
**/
#ifdef __cplusplus
/* in C++  typeded struct A *A; is invalid, e.g. there is no separate namespace for structures */
typedef struct NEXUS_VideoOutputObject *NEXUS_VideoOutput;
#else
typedef struct NEXUS_VideoOutput *NEXUS_VideoOutput;
#endif
/**
Summary:
NEXUS_VideoOutputHandle is synonym for NEXUS_VideoOutput
**/
typedef NEXUS_VideoOutput NEXUS_VideoOutputHandle;

/**
Summary:
NEXUS_VideoInput is an abstract connector token for routing video from a source.

Description:
It is used to connect VideoSources to VideoWindows. It's also used for generic video settings and status.

All VideoInputs also have a specific input (e.g. ComponentVideo, HdmiVideo).
This can be optionally used to control specific settings and to retrieve specific status.

See Also:
NEXUS_VideoDecoder_GetConnector
NEXUS_HdmiInput_GetConnector
NEXUS_AnalogVideo_GetConnector
**/
#ifdef __cplusplus
/* in C++  typeded struct A *A; is invalid, e.g. there is no separate namespace for structures */
typedef struct NEXUS_VideoInputObject *NEXUS_VideoInput;
#else
typedef struct NEXUS_VideoInput *NEXUS_VideoInput;
#endif

/**
Summary:
NEXUS_VideoInputHandle is synonym for NEXUS_VideoInput
**/
typedef NEXUS_VideoInput NEXUS_VideoInputHandle;

#define NEXUS_COLOR_MATRIX_COEFF_COUNT           (15)

/***************************************************************************
Summary:
Coefficient matrix for color space convertor

Description:
This is a 3x5 matrix for programming color space convertors (CSC) in various places in the video or graphics pipeline.
The following explanation comes from BVDC_Window_SetColorMatrix:

    [Y_out ]   [M[0]  M[1]  M[2]  M[3] [M[4] ]   [Y_in ]
    [Cb_out]   [M[5]  M[6]  M[7]  M[8] [M[9] ]   [Cb_in]
    [Cr_out] = [M[10] M[11] M[12] M[13][M[14]] * [Cr_in]
    [A_out ]   [0     0     0     1     0    ]   [A_in ]
    [  1   ]   [0     0     0     0     1    ]   [  1  ]

    {Y_in, Cb_in, Cr_in, A_in}     = Input pixel before apply M matrix.
    {Y_out, Cb_Out, Cr_out, A_out} = Output pixel after apply M matrix.

    Note: The last two rows of the above matrix are used to facilitate
    computation only. It is not implemented physically in hardware.
    Columns 0 to 2 are multiplicative coefficients. Columns 4 and 5 are
    additive coefficients.  Alpha multiply (4th column) is only supported
    graphics.

"shift" is a user-specified bit shift of the coeffMatrix values. This allows the user to specify
fixed point numbers in whatever precision they want. The actual precision supported by the hardware
block is use can vary.
Example:
    shift of 0 -> use the whole coeffMatrix[] value as an integer
    shift of 1 -> use (coeffMatrix[]>>1) as integer and (coeffMatrix[]&0x1)/2 as fractional part.
    shift of 4 -> use (coeffMatrix[]>>4) as integer and (coeffMatrix[]&0xF)/16 as fractional part.

See Also:
NEXUS_VideoInput_SetColorMatrix
NEXUS_VideoWindow_SetColorMatrix
NEXUS_Display_SetGraphicsColorMatrix
****************************************************************************/
typedef struct NEXUS_ColorMatrix
{
    uint32_t shift;
    int32_t  coeffMatrix[NEXUS_COLOR_MATRIX_COEFF_COUNT];
    bool bypass; /* This sets the CSC into bypass mode. It will ignore any values in this
                    NEXUS_ColorMatrix struct, and also bypass any internal color matrix defaults.
                    For graphics, the bypass must be set or cleared before NEXUS_GraphicsSettings.enabled is changed to true.
                    */
} NEXUS_ColorMatrix;

/***************************************************************************
Summary:
Used to enumerate the possible color space standards

Description:
This enum order does not follow the MPEG-2 standard Video-spec, and it could
change in the future.

See Also:
NEXUS_ColorSpace, NEXUS_ColorRange.
NEXUS_ColorPrimaries, NEXUS_TransferCharacteristics.
**/
typedef enum NEXUS_MatrixCoefficients
{
    /* To be deprecated. Use NEXUS_ColorSpace_eRgb and NEXUS_ColorRange_eLimited instead */
    NEXUS_MatrixCoefficients_eHdmi_RGB,

    /* Recommendation ITU-R BT.709. Typical video format: ATSC HD, PAL HD, UHD */
    NEXUS_MatrixCoefficients_eItu_R_BT_709,

    /* To be deprecated */
    NEXUS_MatrixCoefficients_eUnknown,

    /* To be deprecated. Use NEXUS_ColorSpace_eRgb and NEXUS_ColorRange_eFull instead */
    NEXUS_MatrixCoefficients_eDvi_Full_Range_RGB,

    /* FCC. Typical video format: Obsolete 1953 NTSC SD */
    NEXUS_MatrixCoefficients_eFCC,

    /* Recommendation ITU-R BT.470-2 System B, G. Typical video format: PAL SD */
    NEXUS_MatrixCoefficients_eItu_R_BT_470_2_BG,

    /* SMPTE 170M. Typical video format: NTSC SD */
    NEXUS_MatrixCoefficients_eSmpte_170M,

   /* SMPTE 240M. Typical video format: Obsolete 1987 ATSC HD */
    NEXUS_MatrixCoefficients_eSmpte_240M,

    /* HDMI 1.3 xvYCC709. Typical video format: HD */
    NEXUS_MatrixCoefficients_eXvYCC_709,

    /* HDMI 1.3 xvYCC601. Typical video format: SD */
    NEXUS_MatrixCoefficients_eXvYCC_601,

    /* Rec ITU-R BT.2020 non-constant luminance: Typical video format: UHD */
    NEXUS_MatrixCoefficients_eItu_R_BT_2020_NCL,

    /* Rec ITU-R BT.2020 constant luminance: Typical video format: UHD */
    NEXUS_MatrixCoefficients_eItu_R_BT_2020_CL,

    /* To be deprecated. Use NEXUS_ColorSpace_eYCbCr4** and NEXUS_ColorRange_eFull instead */
    NEXUS_MatrixCoefficients_eHdmi_Full_Range_YCbCr,

    NEXUS_MatrixCoefficients_eMax
} NEXUS_MatrixCoefficients;

/***************************************************************************
Summary:
    Used to specify the possible color primaries.

Description:
    This enum order does not follow the MPEG-2 standard Video-spec, and it
    could change in the future.

See Also:
    NEXUS_MatrixCoefficients, NEXUS_TransferCharacteristics.
****************************************************************************/
typedef enum NEXUS_ColorPrimaries
{
    /* Recommendation ITU-R BT.709;
       (ATSC HD or PAL HD) */
    NEXUS_ColorPrimaries_eItu_R_BT_709 = 1,

    /* Unspecified Video: Image characteristics are unknown;
       VDC would handle 'Unknown' case as follows, i.e.
       if the decoded picture is in HD format(size is larger than
       720x576), then take default HD color matrix; else take default SD color
       matrix. */
    NEXUS_ColorPrimaries_eUnknown,

    /* Recommendation ITU-R BT.470-2 System M;
       (NTSC SD 1953, not the same as PAL SD nor SMPTE170) */
    NEXUS_ColorPrimaries_eItu_R_BT_470_2_M = 4,

    /* Recommendation ITU-R BT.470-2 System B, G;
       (PAL SD, similar to SMPTE170) */
    NEXUS_ColorPrimaries_eItu_R_BT_470_2_BG,

    /* SMPTE 170M; (NTSC SD) */
    NEXUS_ColorPrimaries_eSmpte_170M,

    /* SMPTE 240M (1987);
       (ATSC HD; same as SMPTE170) */
    NEXUS_ColorPrimaries_eSmpte_240M,

    /* Generic file
       AVC specification ??? */
    NEXUS_ColorPrimaries_eGenericFilm,

    /* Rec. ITU-R BT. 2020
       (UHDTV) */
    NEXUS_ColorPrimaries_eItu_R_BT_2020

} NEXUS_ColorPrimaries;

/***************************************************************************
Summary:
    Used to specify the possible transfer characteristics.

Description:
    This enum order does not follow the MPEG-2 standard Video-spec, and it
    could change in the future.

See Also:
    NEXUS_MatrixCoefficients, NEXUS_ColorPrimaries.
****************************************************************************/
typedef enum NEXUS_TransferCharacteristics
{
    /* Recommendation ITU-R BT.709;
       (ATSC HD or PAL HD) */
    NEXUS_TransferCharacteristics_eItu_R_BT_709 = 1,

    /* Unspecified Video: Image characteristics are unknown;
       VDC would handle 'Unknown' case as follows, i.e.
       if the decoded picture is in HD format(size is larger than
       720x576), then take default HD color matrix; else take default
       SD color. */
    NEXUS_TransferCharacteristics_eUnknown,

    /* FCC, or Recommendation ITU-R BT.470-2 System M;
       (NTSC SD 1953, assumed display gamma 2.2) */
    NEXUS_TransferCharacteristics_eItu_R_BT_470_2_M = 4,

    /* Recommendation ITU-R BT.470-2 System B, G;
       (PAL SD, assumed display gamma 2.8) */
    NEXUS_TransferCharacteristics_eItu_R_BT_470_2_BG,

    /* SMPTE 170M; (NTSC SD) */
    NEXUS_TransferCharacteristics_eSmpte_170M,

    /* SMPTE 240M (1987); (ATSC HD) */
    NEXUS_TransferCharacteristics_eSmpte_240M,

    /* Linear Transfer Characteristics */
    NEXUS_TransferCharacteristics_eLinear,

    /* Recommendation ITU-T H.262, H.264; (IEC 61966-2-4 gamma, xvYCC) */
    NEXUS_TransferCharacteristics_eIec_61966_2_4 = 11,

    /* Recommendation ITU-R BT.2020 10 bit system;
       (Ultra HD) */
    NEXUS_TransferCharacteristics_eItu_R_BT_2020_10bit,

    /* Recommendation ITU-R BT.709 12 bit system;
       (Ultra HD) */
    NEXUS_TransferCharacteristics_eItu_R_BT_2020_12bit,

    /* SMPTE ST 2084 */
    NEXUS_TransferCharacteristics_eSmpte_ST_2084,

    /* ARIB STD-B67 */
    NEXUS_TransferCharacteristics_eArib_STD_B67,

    /* Enum terminator */
    NEXUS_TransferCharacteristics_eMax

} NEXUS_TransferCharacteristics;

/***************************************************************************
Summary:
    Enumerated type that specifies the structure of a 3D source
***************************************************************************/
typedef enum NEXUS_Video3DStructure
{
    NEXUS_Video3DStructure_eFramePacking,
    NEXUS_Video3DStructure_eFieldAlternative,
    NEXUS_Video3DStructure_eLineAlternative,
    NEXUS_Video3DStructure_eSidexSideFull,
    NEXUS_Video3DStructure_eLDepth,
    NEXUS_Video3DStructure_eLDepthGraphics,
    NEXUS_Video3DStructure_eTopAndBottom,
    NEXUS_Video3DStructure_eReserved,
    NEXUS_Video3DStructure_eSidexSideHalf, /* 0x8 1000 */
    NEXUS_Video3DStructure_eMax
} NEXUS_Video3DStructure;

/***************************************************************************
Summary:
    Enumerated type that specifies the sub-sampling method of a 3D source
***************************************************************************/
typedef enum NEXUS_Video3DSubSample
{
    NEXUS_Video3DSubSample_eNone,
    NEXUS_Video3DSubSample_eHorzOLOR = 0 ,
    NEXUS_Video3DSubSample_eHorzOLER,
    NEXUS_Video3DSubSample_eHorzELOR,
    NEXUS_Video3DSubSample_eHorzELER,
    NEXUS_Video3DSubSample_eQuinOLOR,
    NEXUS_Video3DSubSample_eQuinOLER,
    NEXUS_Video3DSubSample_eQuinELOR,
    NEXUS_Video3DSubSample_eQuinELER,
    NEXUS_Video3DSubSample_eMax
} NEXUS_Video3DSubSample;

/**
Summary:
Orientation of video

Description:
See Also:
NEXUS_VideoFormat
**/
typedef enum NEXUS_VideoOrientation {
    NEXUS_VideoOrientation_e2D,             /* 2D */
    NEXUS_VideoOrientation_e3D_LeftRight,   /* 3D left right */
    NEXUS_VideoOrientation_e3D_OverUnder,   /* 3D over under */
    NEXUS_VideoOrientation_eMax
} NEXUS_VideoOrientation;

/**
Summary:
Picture coding for MPEG video
**/
typedef enum NEXUS_PictureCoding
{
    NEXUS_PictureCoding_eUnknown = 0,
    NEXUS_PictureCoding_eI,
    NEXUS_PictureCoding_eP,
    NEXUS_PictureCoding_eB,
    NEXUS_PictureCoding_eMax
} NEXUS_PictureCoding;

/**
Summary:
Video Electro-Optical Transfer Function (EOTF) types for dynamic range signaling
**/
typedef enum NEXUS_VideoEotf
{
    NEXUS_VideoEotf_eSdr,
    NEXUS_VideoEotf_eHdr,
    NEXUS_VideoEotf_eSmpteSt2084,
    NEXUS_VideoEotf_eFuture,
    NEXUS_VideoEotf_eMax
} NEXUS_VideoEotf;

/**
Summary:
Picture Scan
*/
typedef enum NEXUS_PictureScan
{
   NEXUS_PictureScan_eUnknown,
   NEXUS_PictureScan_eInterlaced,
   NEXUS_PictureScan_eProgressive,
   NEXUS_PictureScan_eMax
} NEXUS_PictureScan;

/**
Summary:
SVP picture buffer support for decoder and window
*/
typedef enum NEXUS_SecureVideo
{
    NEXUS_SecureVideo_eUnsecure, /* decoder/window is only unsecure */
    NEXUS_SecureVideo_eSecure,   /* decoder/window is only secure */
    NEXUS_SecureVideo_eBoth,     /* decoder/window may switch between secure and unsecure */
    NEXUS_SecureVideo_eMax
} NEXUS_SecureVideo;

#ifdef __cplusplus
}
#endif

#endif /* NEXUS_VIDEO_TYPES_H */
