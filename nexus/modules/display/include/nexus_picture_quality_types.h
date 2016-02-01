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
#ifndef NEXUS_PICTURE_QUALITY_TYPES_H__
#define NEXUS_PICTURE_QUALITY_TYPES_H__

#include "nexus_types.h"
#include "nexus_display_types.h"

#ifdef __cplusplus
extern "C" {
#endif


/*=************************************************************************
The PictureCtrl interface manipulates the backend picture enhancement units.
in future Nexus releases.
**************************************************************************/


/***************************************************************************
Summary:
    Dither settings

Description:
    Used in NEXUS_PictureCtrl_SetDitherSettings
**************************************************************************/
typedef struct NEXUS_PictureCtrlDitherSettings
{
    bool      reduceSmooth;
    bool      smoothEnable;
    uint32_t  smoothLimit;
} NEXUS_PictureCtrlDitherSettings;

/***************************************************************************
Summary:
    CMS parameters used in NEXUS_PictureCtrlCmsSettings
**************************************************************************/
typedef struct NEXUS_PictureCtrlCmsParameters
{
    uint32_t red;
    uint32_t green;
    uint32_t blue;
    uint32_t cyan;
    uint32_t magenta;
    uint32_t yellow;
} NEXUS_PictureCtrlCmsParameters;

/***************************************************************************
Summary:
    CMS settings

Description:
    Used in NEXUS_PictureCtrl_SetCmsSettings
**************************************************************************/
typedef struct NEXUS_PictureCtrlCmsSettings
{
    NEXUS_PictureCtrlCmsParameters saturationGain;
    NEXUS_PictureCtrlCmsParameters hueGain;
} NEXUS_PictureCtrlCmsSettings;

/***************************************************************************
Deprecated. Use NEXUS_PictureCtrlCommonSettings.sharpness instead.
**************************************************************************/
typedef struct NEXUS_PictureCtrlSharpnessValue
{
    bool        enable;
    uint32_t    lumaControlCore;
    uint32_t    lumaControlGain;
    uint32_t    lumaControlBlur;
    bool        lumaCtrlSoften;
    bool        lumaCtrlHOnly;
    uint32_t    lumaPeakingHAvoid;
    uint32_t    lumaPeakingVAvoid;
    uint32_t    lumaPeakingPeakLimit;
    uint32_t    lumaPeakingPeakValue;
    uint32_t    chromaControlCore;
    bool        chromaCtrlWideChroma;
    uint32_t    chromaControlFalseColor;
    uint32_t    chromaControlGain;
    bool        chromaCtrlHOnly;
    uint32_t    wideLumaControlCore;
    uint32_t    wideLumaControlMode;
} NEXUS_PictureCtrlSharpnessValue;


#define NEXUS_DC_TABLE_ROWS              (15)
#define NEXUS_DC_TABLE_COLS              (17)

/***************************************************************************
Summary:
    This structure describes the settings used the contrast stretch algorithm.

Description:
    The fields are for the dynamic contrast stretch algorithm. These,
    however, do not allow the user direct control of the LAB table.
    The algorithm just uses these fields as basis for calculating
    the contrast stretch.

    Warning: These can change in future chip revisions.
**************************************************************************/
typedef struct NEXUS_PictureCtrlContrastStretch
{
	bool            enabled;                        /* enable contrast stretch. if false, all other values in this structure are ignored. */
    int             gain;                           /* the amount of stretch towards min and max
                                                       (recommended value of 1 in fixed point format) */
    int             gainShift;                      /* fractional number of gain */

    int             filterNum;                      /* unused: the numerator of the min, mid, max low pass filter */
    int             filterDenom;                    /* unused: the denomerator of the min, mid, max low pass filter */
    int             dynamicContrastBlackGain;       /* unused: gain for black stretch side in Dynamic Contrast (in fixed point format) */
    int             dynamicContrastWhiteGain;       /* unused: gain for white stretch side in Dynamic Contrast (in fixed point format) */
    uint16_t        dynamicContrastBlackLimit;      /* unused: limit for black stretch in Dynamic Contrast */
    uint16_t        dynamicContrastWhiteLimit;      /* unused: limit for white stretch in Dynamic Contrast */
    uint16_t        dynamicContrastEstCurMaxPt;     /* unused: point to estimate current max luma in dynamic contrast */
    uint16_t        dynamicContrastEstCurMinPt;     /* unused: point to estimate current min luma in dynamic contrast */
    bool            dynamicBacklightControl;        /* unused */

    int32_t         ireTable[NEXUS_DC_TABLE_COLS -1];
    uint32_t        dcTable1[NEXUS_DC_TABLE_ROWS * NEXUS_DC_TABLE_COLS];
    uint32_t        dcTable2[NEXUS_DC_TABLE_ROWS * NEXUS_DC_TABLE_COLS];
} NEXUS_PictureCtrlContrastStretch;

/***************************************************************************
Summary:
Array sizes used in NEXUS_LumaStatistics
**************************************************************************/
#define NEXUS_LUMA_HISTOGRAM_COUNT  (16)
#define NEXUS_LUMA_HISTOGRAM_LEVELS (4)

/***************************************************************************
Summary:
    This structure describes the luma statistics.

Description:
NEXUS_LumaStatistics contains luma statistics for the current picture coming out of the window.

See NEXUS_PictureCtrl_GetLumaStatistics
***************************************************************************/
typedef struct NEXUS_LumaStatistics
{
    uint32_t average;   /* the average of all luma in the specified region */
    uint32_t min;       /* the min of all luma in the specified region */
    uint32_t max;       /* the min of all luma in the specified region */
    uint32_t histogram[NEXUS_LUMA_HISTOGRAM_COUNT];  /* histogram data of luma */
    uint32_t levelStats[NEXUS_LUMA_HISTOGRAM_LEVELS]; /* only valid on some chips */
} NEXUS_LumaStatistics;

/***************************************************************************
Summary:
    Common picture control settings

Description:
    All int16_t values in this structure range between -32768 and 32767. The default is 0.
    Used in NEXUS_PictureCtrl_SetCommonSettings
**************************************************************************/
typedef struct NEXUS_PictureCtrlCommonSettings
{
    int16_t contrast;
    int16_t saturation;
    int16_t hue;
    int16_t brightness;

    bool    colorTempEnabled; /* If true, set colorTemp. This will be overridden if NEXUS_PictureCtrlAdvColorSettings.attenuationRbgEnabled is true. */
    int16_t colorTemp;

    bool    sharpnessEnable; /* enable or disable sharpness adjustment */
    int16_t sharpness;
} NEXUS_PictureCtrlCommonSettings;

/***************************************************************************
Summary:
    Attenutation base
**************************************************************************/
#define NEXUS_PICTURE_ATTENUATION_BASE  (2048)

/***************************************************************************
Summary:
    Blue stretch settings
**************************************************************************/
typedef struct
{
    uint16_t blueStretchOffset;
    uint16_t blueStretchSlope;
} NEXUS_BlueStretchSettings;

/***************************************************************************
Summary:
    Enhanced picture control color settings

Description:
    See NEXUS_PictureCtrl_GetAdvColorSettings
**************************************************************************/
typedef struct NEXUS_PictureCtrlAdvColorSettings
{
    bool attenuationRbgEnabled; /* If true, apply the following attenuation RGB convert matrix settings.
                                   This will be overridden if NEXUS_PictureCtrlCommonSettings.colorTempEnabled is true. */
    unsigned attenuationR;      /* attenuation_R in convert matrix */
    unsigned attenuationG;      /* attenuation_G in convert matrix */
    unsigned attenuationB;      /* attenuation_B in convert matrix */
    int16_t  offsetR;           /* offset_R in convert matrix */
    int16_t  offsetG;           /* offset_G in convert matrix */
    int16_t  offsetB;           /* offset_B in convert matrix */

    int16_t  fleshTone;          /* value for Flesh Tone */
    int16_t  greenBoost;         /* value for Green Boost */
    int16_t  blueBoost;          /* value for Blue Boost */
    NEXUS_BlueStretchSettings blueStretchSettings;  /* settings for Blue Stretch */
} NEXUS_PictureCtrlAdvColorSettings;

/***************************************************************************
Summary:
    Color Correction Settings

Description:
    See NEXUS_PictureCtrl_SetColorCorrectionTable
**************************************************************************/
typedef struct NEXUS_PictureControlColorCorrectionSettings
{
    bool enabled;
    unsigned gammaId;  /* identifier for gamma  */
    unsigned colorId;  /* identifier for the color temperature */
} NEXUS_PictureControlColorCorrectionSettings;

/***************************************************************************
Summary:
Color clip modes used in NEXUS_PictureCtrlColorClipSettings
***************************************************************************/
typedef enum NEXUS_ColorClipMode
{
    NEXUS_ColorClipMode_eNone = 0,     /* Disable color clip */
    NEXUS_ColorClipMode_eWhite,        /* Enable color clip for white */
    NEXUS_ColorClipMode_eBlack,        /* Enable color clip for black */
    NEXUS_ColorClipMode_eBoth,         /* Enable color clip for both */
    NEXUS_ColorClipMode_eMax
} NEXUS_ColorClipMode;

/***************************************************************************
Summary:
    This structure describes Color Clip slopes/joints configurations.

Description:
    The job of the color clip is to gracefully remap out of range RGB values in a
    graceful way before hardware does the clipping to 0 or 1023.  What gets clipped
    is purely a function of the matrix which converts display-colorspace YCbCr to
    display-colorspace RGB.  This matrix is completely determined by the display's
    primaries and white point.
    It only depends on the primaries and white point of the display, which is fixed.
    Therefore, the slopes and joints can be computed offline and stored for each
    TV model or individual unit.

    Note: slopes are unsigned and the derivation should follow the provided
    programming guide.
***************************************************************************/
typedef struct NEXUS_PictureCtrlColorClipSettings
{
    uint32_t            crYSlopeA; /* Chroma versus (219-Luma) Slope; in U16.16 fixed point format. */
    uint32_t            crYSlopeB; /* Chroma versus (219-Luma) Slope; in U16.16 fixed point format. */
    uint32_t            cbYSlopeA; /* Chroma versus (219-Luma) Slope; in U16.16 fixed point format. */
    uint32_t            cbYSlopeB; /* Chroma versus (219-Luma) Slope; in U16.16 fixed point format. */
    uint32_t            crJoint;   /* Joint of Cr Slope_A and Slope_B. */
    uint32_t            cbJoint;   /* Joint of Cb Slope_A and Slope_B. */
    bool                extendedWhite;
    bool                extendedBlack;
    NEXUS_ColorClipMode colorClipMode;
} NEXUS_PictureCtrlColorClipSettings;

#ifdef __cplusplus
}
#endif

#endif /* NEXUS_PICTURE_QUALITY_TYPES_H__ */
