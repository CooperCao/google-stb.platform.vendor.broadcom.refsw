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
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 **************************************************************************/
#ifndef NEXUS_DISPLAY_TYPES_H__
#define NEXUS_DISPLAY_TYPES_H__

#include "nexus_base_types.h"
#include "nexus_video_types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define NEXUS_MAX_VIDEO_DACS 7
#define NEXUS_MAX_DISPLAYS 7
#define NEXUS_MAX_VIDEO_WINDOWS 2
#define NEXUS_MAX_VIDEO_ENCODERS 6

/**
Summary:
Types of the display aspect ratio
**/
typedef enum NEXUS_DisplayAspectRatio
{
    NEXUS_DisplayAspectRatio_eAuto, /* 4x3 for SD and 480p, 16x9 for HD (including 720p, 1080i, etc.) */
    NEXUS_DisplayAspectRatio_e4x3,
    NEXUS_DisplayAspectRatio_e16x9,
    NEXUS_DisplayAspectRatio_eSar,  /* sample aspect ratio. must also set sampleAspectRatio.x and .y. */
    NEXUS_DisplayAspectRatio_eMax
} NEXUS_DisplayAspectRatio;

/**
Summary:
DAC's avaliable for the analog outputs
**/
typedef enum NEXUS_VideoDac
{
    NEXUS_VideoDac_eNone, /* disconnected */
    NEXUS_VideoDac_e0,
    NEXUS_VideoDac_e1,
    NEXUS_VideoDac_e2,
    NEXUS_VideoDac_e3,
    NEXUS_VideoDac_e4,
    NEXUS_VideoDac_e5,
    NEXUS_VideoDac_e6,
    NEXUS_VideoDac_eMax
} NEXUS_VideoDac;

/**
Summary:
Display handle obtained from NEXUS_Display_Open
**/
typedef struct NEXUS_Display *NEXUS_DisplayHandle;

/**
Summary:
Video window handle obtained from NEXUS_Display_GetWindow
**/
typedef struct NEXUS_VideoWindow *NEXUS_VideoWindowHandle;

/**
deprecated
**/
typedef enum NEXUS_AnalogVideoChannel
{
    NEXUS_AnalogVideoChannel_eY,
    NEXUS_AnalogVideoChannel_ePr,
    NEXUS_AnalogVideoChannel_ePb,
    NEXUS_AnalogVideoChannel_eR,
    NEXUS_AnalogVideoChannel_eG,
    NEXUS_AnalogVideoChannel_eB,
    NEXUS_AnalogVideoChannel_eLuma,
    NEXUS_AnalogVideoChannel_eChroma,
    NEXUS_AnalogVideoChannel_eComposite,
    NEXUS_AnalogVideoChannel_eScartR,
    NEXUS_AnalogVideoChannel_eScartG,
    NEXUS_AnalogVideoChannel_eScartB,
    NEXUS_AnalogVideoChannel_eScartY,
    NEXUS_AnalogVideoChannel_eScartPr,
    NEXUS_AnalogVideoChannel_eScartPb,
    NEXUS_AnalogVideoChannel_eScartSync,
    NEXUS_AnalogVideoChannel_eScartComposite,
    NEXUS_AnalogVideoChannel_eScartLuma,
    NEXUS_AnalogVideoChannel_eScartChroma
} NEXUS_AnalogVideoChannel;

/**
Summary:
Factors used to blend video windows and graphics in the compositor

Description:
Used in NEXUS_VideoWindowSettings and NEXUS_GraphicsSettings.

See BVDC_Window_SetBlendFactor for a thorough explanation of video and graphics blending equations.

Compare with NEXUS_BlendFactor in nexus_graphics2d.h. There are similar, but map to separate blocks of hardware.
**/
typedef enum NEXUS_CompositorBlendFactor
{
    NEXUS_CompositorBlendFactor_eZero = 0,                /* 0 */
    NEXUS_CompositorBlendFactor_eOne,                     /* 1 */
    NEXUS_CompositorBlendFactor_eSourceAlpha,             /* alpha source */
    NEXUS_CompositorBlendFactor_eInverseSourceAlpha,      /* 1 - (alpha source) */
    NEXUS_CompositorBlendFactor_eConstantAlpha,           /* constant alpha */
    NEXUS_CompositorBlendFactor_eInverseConstantAlpha,    /* 1 - (constant alpha) */
    NEXUS_CompositorBlendFactor_eMax
} NEXUS_CompositorBlendFactor;

/***************************************************************************
Summary:
Conversion matrix settings

Description:
Parameters in the convert matrix are in RGB domain in 20.11 signed fixed point.
For example, 1 is represented by 0x800 and -1 is 0xffffffff.

Used by NEXUS_VideoWindow_SetAttenuationRgb

**************************************************************************/
typedef struct NEXUS_AttenuationRgbSettings
{
    int32_t attenuationR;      /* attenuation_R in convert matrix */
    int32_t attenuationG;      /* attenuation_G in convert matrix */
    int32_t attenuationB;      /* attenuation_B in convert matrix */
    int32_t offsetR;           /* offset_R in convert matrix */
    int32_t offsetG;           /* offset_G in convert matrix */
    int32_t offsetB;           /* offset_B in convert matrix */
} NEXUS_AttenuationRgbSettings;

/**
Summary:
Options for converting the source to the video window rectangle based on both of their aspect ratios (A/R).

Description:
The behavior of this enum depends on the aspect ratio of the video window and the source.
If the aspect ratio of the display and content match, this enum has no effect.
**/
typedef enum NEXUS_VideoWindowContentMode
{
    NEXUS_VideoWindowContentMode_eZoom,      /* cut off content to preserve aspect ratio.
        If you want to achieve different levels of zoom, you will need to use eFull and set your own manual source clipping
        using NEXUS_VideoWindowSettings. This eZoom enum implements only one flavor of zoom, which is minimal clipping. */
    NEXUS_VideoWindowContentMode_eBox,       /* add either a window box (also known as pillar box or sidebars) or a letter box to preserve aspect ratio.
        The type of box depends on the aspect ratio of the source and display.
        If source is 4x3 and display is 16x9, eBox will result in window box.
        If source is 16x9 and display is 4x3, eBox will result in letter box. */
    NEXUS_VideoWindowContentMode_ePanScan,   /* Used for 16x9 source with pan scan vectors on 4x3 display only, otherwise same as zoom.
                                                Additional A/R correct is applied after pan scan vectors. */
    NEXUS_VideoWindowContentMode_eFull,      /* distort aspect ratio but see all the content and no windowbox or letterbox. */
    NEXUS_VideoWindowContentMode_eFullNonLinear,/* Non-linear upscaling to full screen where
                                                  the edge of the content will have more distorted aspect ratio.
                                                  See NEXUS_VideoWindow_SetNonLinearScalerSettings (for some chips) for an enable boolean. */
    NEXUS_VideoWindowContentMode_ePanScanWithoutCorrection, /* Same as ePanScan, but don't apply additional A/R correction afterwards. */
    NEXUS_VideoWindowContentMode_eMax
} NEXUS_VideoWindowContentMode;

/**
Summary:
Set color control settings for graphics feeder (GFD).

Description:
This will override anything set by NEXUS_Display_SetGraphicsColorMatrix.

int16_t values range between -32768 and 32767.
**/
typedef struct NEXUS_GraphicsColorSettings
{
    int16_t contrast;
    int16_t saturation;
    int16_t hue;
    int16_t brightness;
} NEXUS_GraphicsColorSettings;

/**
Summary:
Mode used in NEXUS_VideoWindowFilterSettings
**/
typedef enum NEXUS_VideoWindowFilterMode
{
    NEXUS_VideoWindowFilterMode_eDisable,
    NEXUS_VideoWindowFilterMode_eBypass,
    NEXUS_VideoWindowFilterMode_eEnable,
    NEXUS_VideoWindowFilterMode_eMax
} NEXUS_VideoWindowFilterMode;

/**
Summary:
Filter settings used in NEXUS_VideoWindowDnrSettings and NEXUS_VideoWindowAnrSettings.
**/
typedef struct NEXUS_VideoWindowFilterSettings
{
    NEXUS_VideoWindowFilterMode mode; /* mode of noise reduction */
    int level; /* level of noise reduction, valid effective range -100 ...200 */
} NEXUS_VideoWindowFilterSettings;

/**
Summary:
Digital Noise Reduction (DNR) settings

Description:
See BVDC_Dnr_Settings for detailed information
**/
typedef struct NEXUS_VideoWindowDnrSettings
{
    NEXUS_VideoWindowFilterSettings mnr; /* Mosquito Noise Reduction. See BVDC_Dnr_Settings for mnr.level range. */
    NEXUS_VideoWindowFilterSettings bnr; /* Block Noise Reduction. See BVDC_Dnr_Settings for bnr.level range. */
    NEXUS_VideoWindowFilterSettings dcr; /* Digital Contour Reduction. See BVDC_Dnr_Settings for dcr.level range. */
    unsigned qp; /* non zero value is used to force constant QP (0 is default and used to derive QP  from the decoded stream) */
} NEXUS_VideoWindowDnrSettings;

/**
Summary:
Analog Noise Reduction (ANR) settings

Description:
anr.level is used for user adjustment to S/N db number.
If S/N is around 60 db, the video signal is very clean and ANR
will be configured to perform very little filtering. When S/N db
number become smaller, the video signal is more noisy and ANR will
be configured to perform stronger filtering. As it reaches to
about 25 db, ANR filtering reaches the strongest.
**/
typedef struct NEXUS_VideoWindowAnrSettings
{
    NEXUS_VideoWindowFilterSettings anr; /* Analog Noise Reduction */

    NEXUS_PixelFormat pixelFormat;       /* pixel format of ANR buffer */
} NEXUS_VideoWindowAnrSettings;

/***************************************************************************
Summary:
Specifies various modes for handling AFD (Active Format Descriptor).

Description:
This enumeration specifies different modes for handling AFD (Active
Format Descriptor).  AFD is value transmitted in the stream's user data
that can be use to clip away unwanted content such as black vertical or
horizontal bars; or even non-important contents.  The specification of
clipping is base on AFD specs.
***************************************************************************/
typedef enum NEXUS_AfdMode {
    NEXUS_AfdMode_eDisabled = 0, /* No clipping will be performed even if the stream contains the AFD value. */
    NEXUS_AfdMode_eStream,       /* Clipping will be performed if the stream contains the AFD value. */
    NEXUS_AfdMode_eUser,         /* Clipping will be performed with NEXUS_VideoWindowAfdSettings.userValue. Any AFD value in the stream will be ignored. */
    NEXUS_AfdMode_eMax
} NEXUS_AfdMode;

/***************************************************************************
Summary:
Specifies various clipping modes to be use with AFD.
***************************************************************************/
typedef enum NEXUS_AfdClip
{
    NEXUS_AfdClip_eNominal = 0,    /* This mode clips nominal. This will clip away the black content in the stream base on AFD value. */
    NEXUS_AfdClip_eOptionalLevel1, /* This mode clips away the NEXUS_AfdClip_eNominal content + optional contents at level1. */
    NEXUS_AfdClip_eOptionalLevel2, /* This mode clips away the NEXUS_AfdClip_eNominal content + optional contents at level2. */
    NEXUS_AfdClip_eMax
} NEXUS_AfdClip;

/***************************************************************************
Summary:
Active Format Descriptor settings used in NEXUS_VideoWindow_SetAfdSettings
***************************************************************************/
typedef struct NEXUS_VideoWindowAfdSettings
{
    NEXUS_AfdMode mode;
    NEXUS_AfdClip clip;
    uint32_t      userValue; /* only used if mode = NEXUS_AfdMode_eUser */
} NEXUS_VideoWindowAfdSettings;

/**
Summary:
Graphics feeder (GFD) scaler filtering options used in NEXUS_GraphicsSettings
**/
typedef enum NEXUS_GraphicsFilterCoeffs
{
    NEXUS_GraphicsFilterCoeffs_eAuto,             /* Select by internal algorithm */
    NEXUS_GraphicsFilterCoeffs_ePointSample,      /* Point sampled filtering */
    NEXUS_GraphicsFilterCoeffs_eBilinear,         /* Bilinear filtering */
    NEXUS_GraphicsFilterCoeffs_eAnisotropic,      /* Anisotropic filtering */
    NEXUS_GraphicsFilterCoeffs_eSharp,            /* Tabled sin(x)/x filtering. Also, see comments for horizontalCoeffIndex and verticalCoeffIndex */
    NEXUS_GraphicsFilterCoeffs_eMax
} NEXUS_GraphicsFilterCoeffs;

/*
Summary:
Macrovision types used in NEXUS_Display_SetMacrovision
*/
typedef enum NEXUS_DisplayMacrovisionType
{
    NEXUS_DisplayMacrovisionType_eNone,         /* No macrovision on outputs. */
    NEXUS_DisplayMacrovisionType_eAgcOnly,      /* AGC only. */
    NEXUS_DisplayMacrovisionType_eAgc2Lines,    /* AGC + 2 Line color stripe. */
    NEXUS_DisplayMacrovisionType_eAgc4Lines,    /* AGC + 4 Line color stripe. */
    NEXUS_DisplayMacrovisionType_eCustom,       /* User-provided tables. */
    NEXUS_DisplayMacrovisionType_eAgcOnlyRgb,   /* PAL MV cert test AGC only with MV on RGB. */
    NEXUS_DisplayMacrovisionType_eAgc2LinesRgb, /* NTSC MV cert test AGC + 2 Line color stripe, with MV on RGB. */
    NEXUS_DisplayMacrovisionType_eAgc4LinesRgb, /* NTSC MV cert test AGC + 4 Line color stripe. */
    NEXUS_DisplayMacrovisionType_eTest01,       /* MV certification test 01. */
    NEXUS_DisplayMacrovisionType_eTest02,       /* MV certification test 02. */
    NEXUS_DisplayMacrovisionType_eMax           /* MV certification test 02. */
} NEXUS_DisplayMacrovisionType;

/*
Summary:
Macrovision table structure used in NEXUS_Display_SetMacrovision

Description:
See magnum/portinginterface/vdc/BCHP_CHIP/bvdc_macrovision.h for the source of the numbers for this structure.
*/
typedef struct NEXUS_DisplayMacrovisionTables
{
    uint8_t cpcTable[2];
    uint8_t cpsTable[33];
} NEXUS_DisplayMacrovisionTables;

/**
Summary:
CRC data for video at the MFD (MPEG feeder)

Description:
This API is used for test and is subject to change.
**/
typedef struct NEXUS_VideoInputCrcData
{
    uint32_t idrPictureId;
    uint32_t pictureOrderCount;
    uint32_t isField;
    uint32_t crc[6]; /* Meaning of CRC fields is NEXUS_VideoInputCrcType specific. Current mapping is:
                        crc[0] = luma
                        crc[1] = chroma for eCrc32, Cb for eCrc16 and eChecksum
                        crc[2] = unused for eCrc32, Cr for eCrc16 and eChecksum
                        crc[3..5] = right eye equivalents of crc[0..2]
                        Unused crc fields are 0xFFFFFFFF. */
} NEXUS_VideoInputCrcData;

/**
Summary:
Display CRC data
**/
typedef struct NEXUS_DisplayCrcData
{
    struct {
        uint32_t luma;
        uint32_t cb;
        uint32_t cr;
    } cmp; /* compositor (CMP) CRC */
} NEXUS_DisplayCrcData;

/**
Summary:
Multi-buffering game delay mode settings

Description:
See BVDC_Window_GameModeSettings in bvdc.h for details.
**/
typedef struct NEXUS_VideoWindowGameModeDelay
{
    bool enable;
    bool forceCoarseTrack;
    uint32_t bufferDelayTarget;
    uint32_t bufferDelayTolerance;
} NEXUS_VideoWindowGameModeDelay;

/**
Summary:
MAD game mode options

Description:
Used in NEXUS_VideoWindowMadSettings. Not related to NEXUS_VideoWindowGameModeDelay.
See BVDC_MadGameMode in bvdc.h for details.
**/
typedef enum NEXUS_VideoWindowGameMode
{
    NEXUS_VideoWindowGameMode_eOff,
    NEXUS_VideoWindowGameMode_e5Fields_2Delay,
    NEXUS_VideoWindowGameMode_e5Fields_1Delay,
    NEXUS_VideoWindowGameMode_e5Fields_0Delay,
    NEXUS_VideoWindowGameMode_e5Fields_ForceSpatial,
    NEXUS_VideoWindowGameMode_e4Fields_2Delay,
    NEXUS_VideoWindowGameMode_e4Fields_1Delay,
    NEXUS_VideoWindowGameMode_e4Fields_0Delay,
    NEXUS_VideoWindowGameMode_e4Fields_ForceSpatial,
    NEXUS_VideoWindowGameMode_e3Fields_2Delay,
    NEXUS_VideoWindowGameMode_e3Fields_1Delay,
    NEXUS_VideoWindowGameMode_e3Fields_0Delay,
    NEXUS_VideoWindowGameMode_e3Fields_ForceSpatial,
    NEXUS_VideoWindowGameMode_eMax
} NEXUS_VideoWindowGameMode;

/**
Summary:
Enabled general picture quality enhancement features for deinterlacer
**/
typedef enum NEXUS_MadPqEnhancement
{
    NEXUS_MadPqEnhancement_eAuto, /* internal decision */
    NEXUS_MadPqEnhancement_eOff,  /* force off */
    NEXUS_MadPqEnhancement_eOn,   /* force on */
    NEXUS_MadPqEnhancement_eMax
} NEXUS_MadPqEnhancement;

/**
Summary:
Motion Adaptive Deinterlacer (MAD) settings
**/
typedef struct NEXUS_VideoWindowMadSettings
{
    bool deinterlace;                   /* If true, MAD is allowed to be used if available and appropriate. */
    bool enable32Pulldown;              /* enable reverse 3:2 pulldown. defaults to false.
                                           this feature enables the deinterlacer's detection/recovery of progressive picture from 3:2 pulled-down contents. */
    bool enable22Pulldown;              /* enable reverse 2:2 pulldown. defaults to false.
                                           this feature enables the deinterlacer's detection/recovery of progressive picture from 2:2 pulled-down contents. */
    NEXUS_VideoWindowGameMode gameMode; /* Include game mode */
    NEXUS_PixelFormat pixelFormat;      /* pixel format of MAD buffer */
    bool shrinkWidth;                   /* Internally scale down the width of the input source so that we can deinterlace a source whose width exceeds
                                           the MAD HW limitation. This sacrifices horizontal resolution and requires additional memory allocation and bandwidth.
                                           Ideally, this setting should be applied before calling NEXUS_VideoWindow_AddInput. Otherwise, it will cause a momentary black frame. */
    NEXUS_MadPqEnhancement pqEnhancement;
} NEXUS_VideoWindowMadSettings;

/**
Deprecated. Only available for systems without box modes.
**/
typedef enum NEXUS_ScalerCaptureBias
{
    NEXUS_ScalerCaptureBias_eAuto,
    NEXUS_ScalerCaptureBias_eScalerBeforeCapture,
    NEXUS_ScalerCaptureBias_eScalerAfterCapture
} NEXUS_ScalerCaptureBias;

/**
Summary:
Scaler settings
**/
typedef struct NEXUS_VideoWindowScalerSettings
{
    bool        nonLinearScaling;           /* Set true to enable non-linear scaling. */
    uint32_t    nonLinearSourceWidth;       /* Applied if nonLinearScaling is true */
    uint32_t    nonLinearScalerOutWidth;    /* Applied if nonLinearScaling is true */

    /* Deprecated. Only available for systems without box modes. */
    struct {
        NEXUS_ScalerCaptureBias bias;
        uint32_t delta;
    } bandwidthEquationParams;

    bool verticalDejagging;         /* set true to enable, defaults true */
    bool horizontalLumaDeringing;   /* set true to enable, defaults true */
    bool verticalLumaDeringing;     /* set true to enable, defaults true */
    bool horizontalChromaDeringing; /* set true to enable, defaults true */
    bool verticalChromaDeringing;   /* set true to enable, defaults true */
} NEXUS_VideoWindowScalerSettings;

/* Deprecated name */
#define NEXUS_VideoWindowSclSettings NEXUS_VideoWindowScalerSettings

#ifdef __cplusplus
}
#endif

#endif /* NEXUS_VIDEO_DISPLAY_H__ */
