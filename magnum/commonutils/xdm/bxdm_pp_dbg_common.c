/***************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
 * [File Description:]
 *
 ***************************************************************************/

#include "bstd.h"
#include "bkni.h"
#include "bdbg.h"

#include "bxdm_pp.h"
#include "bxdm_pp_priv.h"
#include "bxdm_pp_dbg.h"
#include "bxdm_pp_qm.h"
#include "bxdm_pp_dbg_common.h"


BDBG_MODULE(BXDM_PPDBG_COMMON);

/*
 * Lookup tables mapping variables to strings.
 */

const char BXDM_P_InterruptPolarityToStrLUT[BXDM_P_MAX_INTERRUPT_POLARITY] =
{
  't', /* BAVC_Polarity_eTopField */
  'b', /* BAVC_Polarity_eBotField */
  'f'  /* BAVC_Polarity_eFrame */
};

const char BXDM_P_PicturePolaritytoStrLUT[BXDM_PictureProvider_P_InterruptType_eMax][BXDM_P_MAX_INTERRUPT_POLARITY] =
{
 /* BXDM_PictureProvider_P_InterruptType_eSingle */
 {
  'T', /* BAVC_Polarity_eTopField */
  'B', /* BAVC_Polarity_eBotField */
  'F'  /* BAVC_Polarity_eFrame */
 },

 /* BXDM_PictureProvider_P_InterruptType_eBase (or Primary) */
 {
  'p', /* BAVC_Polarity_eTopField */
  'p', /* BAVC_Polarity_eBotField */
  'p'  /* BAVC_Polarity_eFrame */
 },

 /* BXDM_PictureProvider_P_InterruptType_eDependent */
 {
  'd', /* BAVC_Polarity_eTopField */
  'd', /* BAVC_Polarity_eBotField */
  'd'  /* BAVC_Polarity_eFrame */
 }
};


const char BXDM_P_HexToCharLUT[16] =
{
   '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
};

const char BXDM_P_PictureSelectionLUT[BXDM_PPDBG_Selection_eMax] =
{
   '-', /* BXDM_PPDBG_Selection_ePPBNotFound */
   '+', /* BXDM_PPDBG_Selection_ePPBFound */
   'P', /* BXDM_PPDBG_Selection_ePass */
   'F', /* BXDM_PPDBG_Selection_eForce */
   'L', /* BXDM_PPDBG_Selection_eLate */
   'W', /* BXDM_PPDBG_Selection_eWait */
   'Z', /* BXDM_PPDBG_Selection_eFreeze */
   'E', /* BXDM_PPDBG_Selection_eTooEarly */
   'D', /* BXDM_PPDBG_Selection_eDrop */
   'R', /* BXDM_PPDBG_Selection_eDelay */
   'd', /* BXDM_PPDBG_Selection_eDependentPicture */

#if 0
   'b', /* BXDM_PPDBG_Selection_PolarityOverride_eBothField */
   'p', /* BXDM_PPDBG_Selection_PolarityOverride_eProgressive */
#endif

   '1', /* BXDM_PPDBG_Selection_PolarityOverride_e1stTo2ndSlot */
   '2', /* BXDM_PPDBG_Selection_PolarityOverride_e2ndTo1stSlot */
   'n', /* BXDM_PPDBG_Selection_PolarityOverride_e2ndSlotNextElement */
   'p', /* BXDM_PPDBG_Selection_PolarityOverride_eSelectPrevious */
   'r', /* BXDM_PPDBG_Selection_PolarityOverride_eRepeatPrevious */
   '0', /* BXDM_PPDBG_Selection_PolarityOverride_eFICReset */
   'w', /* BXDM_PPDBG_Selection_PolarityOverride_eFICForceWait */
   'i', /* BXDM_PPDBG_Selection_eStcInvalidForceWait */
   'm', /* BXDM_PPDBG_Selection_eMulliganLogicForceWait */
   't', /* BXDM_PPDBG_Selection_eTSMResultCallbackForceWait */
};

const char * const BXDM_P_PolarityToStrLUT[BAVC_Polarity_eFrame+1] =
{
   BDBG_STRING_INLINE("T"),  /* Top field */
   BDBG_STRING_INLINE("B"),  /* Bottom field */
   BDBG_STRING_INLINE("F"),  /* Progressive frame */
};

const char * const BXDM_P_BAVCFrameRateToStrLUT[BXDM_PictureProvider_P_MAX_FRAMERATE] =
{
   BDBG_STRING_INLINE("ukn"),      /* Unknown */
   BDBG_STRING_INLINE("23.97"),    /* 23.976 */
   BDBG_STRING_INLINE("24"),       /* 24 */
   BDBG_STRING_INLINE("25"),       /* 25 */
   BDBG_STRING_INLINE("29.97"),    /* 29.97 */
   BDBG_STRING_INLINE("30"),       /* 30 */
   BDBG_STRING_INLINE("50"),       /* 50 */
   BDBG_STRING_INLINE("59.94"),    /* 59.94 */
   BDBG_STRING_INLINE("60"),       /* 60 */
   BDBG_STRING_INLINE("14.98"),    /* 14.985 */
   BDBG_STRING_INLINE("7.49"),     /* 7.493 */
   BDBG_STRING_INLINE("10"),       /* 10 */
   BDBG_STRING_INLINE("15"),       /* 15 */
   BDBG_STRING_INLINE("20"),       /* 20 */
   BDBG_STRING_INLINE("12.5"),     /* SW7584-331: add support for BAVC_FrameRateCode_e12_5 */
   BDBG_STRING_INLINE("100"),
   BDBG_STRING_INLINE("119.88"),
   BDBG_STRING_INLINE("120"),
   BDBG_STRING_INLINE("19.98"),    /* SWSTB-378: add support for BAVC_FrameRateCode_e19_98 */
   BDBG_STRING_INLINE("7.5"),      /* SWSTB-1401: add support for BAVC_FrameRateCode_e7_5 */
   BDBG_STRING_INLINE("12"),       /* SWSTB-1401: add support for BAVC_FrameRateCode_e12 */
   BDBG_STRING_INLINE("11.988"),   /* SWSTB-1401: add support for BAVC_FrameRateCode_e11_988 */
   BDBG_STRING_INLINE("9.99"),     /* SWSTB-1401: add support for BAVC_FrameRateCode_e9_99 */
};

const char * const BXDM_P_FrameRateTypeToStrLUT[BXDM_PictureProvider_P_FrameRateType_eMax] =
{
   BDBG_STRING_INLINE("cod"),   /* coded in the stream */
   BDBG_STRING_INLINE("def"),   /* as specified by BXDM_PictureProvider_SetDefaultFrameRate_isr   */
   BDBG_STRING_INLINE("ovr"),   /* as specified by BXDM_PictureProvider_SetFrameRateOverride_isr  */
   BDBG_STRING_INLINE("frd"),   /* calculated in the FRD code using the PTS values                */
   BDBG_STRING_INLINE("hcd")    /* using the values hardcoded in BXDM_PPTSM_P_PtsCalculateParameters_isr */
};

const char * const BXDM_P_OrientationToStrLUT[BFMT_Orientation_eLeftRight_Enhanced+1] =
{
   BDBG_STRING_INLINE("2D"),       /* 2D */
   BDBG_STRING_INLINE("LftRgt"),   /* 3D left right */
   BDBG_STRING_INLINE("OvrUnd"),   /* 3D over under */
   BDBG_STRING_INLINE("FF-Lf"),    /* 3D left */
   BDBG_STRING_INLINE("FF-Rt"),    /* 3D right */
   BDBG_STRING_INLINE("LR-En")     /* 3D left right, enhancement picture */
};

const char * const BXDM_P_BAVCPictureCodingToStrLUT[BAVC_PictureCoding_eMax] =
{
   BDBG_STRING_INLINE("u"),     /* Picture Coding Type Unknown */
   BDBG_STRING_INLINE("I"),     /* Picture Coding Type I */
   BDBG_STRING_INLINE("P"),     /* Picture Coding Type P */
   BDBG_STRING_INLINE("B")      /* Picture Coding Type B */
};

const char * const BXDM_P_BXDMPictureCodingToStrLUT[BXDM_Picture_Coding_eMax] =
{
   BDBG_STRING_INLINE("u"),     /* BXDM_Picture_Coding_eUnknown */
   BDBG_STRING_INLINE("I"),     /* BXDM_Picture_Coding_eI */
   BDBG_STRING_INLINE("P"),     /* BXDM_Picture_Coding_eP */
   BDBG_STRING_INLINE("B")      /* BXDM_Picture_Coding_eB */
};

const char * const BXDM_P_AspectRatioToStrLUT[BFMT_AspectRatio_eSAR+1] =
{
  BDBG_STRING_INLINE("Ukn"),    /* Unkown/Reserved */
  BDBG_STRING_INLINE("Sqr"),    /* square pixel */
  BDBG_STRING_INLINE("4x3"),    /* 4:3 */
  BDBG_STRING_INLINE("16x9"),   /* 16:9 */
  BDBG_STRING_INLINE("221x1"),  /* 2.21:1 */
  BDBG_STRING_INLINE("15x9"),   /* 15:9 */
  BDBG_STRING_INLINE("SAR")     /* no DAR available, use SAR instead */
};

const char * const BXDM_P_MonitorRefreshRateToStrLUT[BXDM_PictureProvider_MonitorRefreshRate_eMax] =
{
   BDBG_STRING_INLINE("Ukn"),      /* BXDM_PictureProvider_MonitorRefreshRate_eUnknown */
   BDBG_STRING_INLINE("7.493"),    /* BXDM_PictureProvider_MonitorRefreshRate_e7_493Hz */
   BDBG_STRING_INLINE("7.5"),      /* BXDM_PictureProvider_MonitorRefreshRate_e7_5Hz */
   BDBG_STRING_INLINE("9.99"),     /* BXDM_PictureProvider_MonitorRefreshRate_e9_99Hz */
   BDBG_STRING_INLINE("10"),       /* BXDM_PictureProvider_MonitorRefreshRate_e10Hz */
   BDBG_STRING_INLINE("11.988"),   /* BXDM_PictureProvider_MonitorRefreshRate_e11_988Hz */
   BDBG_STRING_INLINE("12"),       /* BXDM_PictureProvider_MonitorRefreshRate_e12Hz */
   BDBG_STRING_INLINE("12.5"),     /* BXDM_PictureProvider_MonitorRefreshRate_e12_5Hz */
   BDBG_STRING_INLINE("14.985"),   /* BXDM_PictureProvider_MonitorRefreshRate_e14_985Hz */
   BDBG_STRING_INLINE("15"),       /* BXDM_PictureProvider_MonitorRefreshRate_e15Hz */
   BDBG_STRING_INLINE("19.98"),    /* BXDM_PictureProvider_MonitorRefreshRate_e19_98Hz */
   BDBG_STRING_INLINE("20"),       /* BXDM_PictureProvider_MonitorRefreshRate_e20Hz */
   BDBG_STRING_INLINE("23.97"),    /* BXDM_PictureProvider_MonitorRefreshRate_e23_976Hz */
   BDBG_STRING_INLINE("24"),       /* BXDM_PictureProvider_MonitorRefreshRate_e24Hz */
   BDBG_STRING_INLINE("25"),       /* BXDM_PictureProvider_MonitorRefreshRate_e25Hz */
   BDBG_STRING_INLINE("29.97"),    /* BXDM_PictureProvider_MonitorRefreshRate_e29_97Hz */
   BDBG_STRING_INLINE("30"),       /* BXDM_PictureProvider_MonitorRefreshRate_e30Hz */
   BDBG_STRING_INLINE("48"),       /* BXDM_PictureProvider_MonitorRefreshRate_e48Hz */
   BDBG_STRING_INLINE("50"),       /* BXDM_PictureProvider_MonitorRefreshRate_e50Hz */
   BDBG_STRING_INLINE("59.94"),    /* BXDM_PictureProvider_MonitorRefreshRate_e59_94Hz */
   BDBG_STRING_INLINE("60"),       /* BXDM_PictureProvider_MonitorRefreshRate_e60Hz */
   BDBG_STRING_INLINE("100"),      /* BXDM_PictureProvider_MonitorRefreshRate_e100Hz */
   BDBG_STRING_INLINE("119.88"),   /* BXDM_PictureProvider_MonitorRefreshRate_e119_88Hz */
   BDBG_STRING_INLINE("120")       /* BXDM_PictureProvider_MonitorRefreshRate_e120Hz */
};

const char * const BXDM_P_STCTrickModeToStrLUT[BXDM_PictureProvider_P_STCTrickMode_eMax] =
{
   BDBG_STRING_INLINE("Off"),   /* BXDM_PictureProvider_P_STCTrickMode_eOff */
   BDBG_STRING_INLINE("FstFwd"),   /* BXDM_PictureProvider_P_STCTrickMode_eFastForward */
   BDBG_STRING_INLINE("SlwFwd"),   /* BXDM_PictureProvider_P_STCTrickMode_eSlowMotion */
   BDBG_STRING_INLINE("Pause"),   /* BXDM_PictureProvider_P_STCTrickMode_ePause */
   BDBG_STRING_INLINE("FstRwd"),   /* BXDM_PictureProvider_P_STCTrickMode_eFastRewind */
   BDBG_STRING_INLINE("SlwRwd")    /* BXDM_PictureProvider_P_STCTrickMode_eSlowRewind */
};

const char * const BXDM_P_TSMResultToStrLUT[BXDM_PictureProvider_TSMResult_eMax] =
{
   BDBG_STRING_INLINE("e"),  /* BXDM_PictureProvider_TSMResult_eTooEarly */
   BDBG_STRING_INLINE("w"),  /* BXDM_PictureProvider_TSMResult_eWait */
   BDBG_STRING_INLINE("p"),  /* BXDM_PictureProvider_TSMResult_ePass */
   BDBG_STRING_INLINE("l"),  /* BXDM_PictureProvider_TSMResult_eTooLate */
   BDBG_STRING_INLINE("d")   /* BXDM_PictureProvider_TSMResult_eDrop */
};


const char * const BXDM_P_PullDownEnumToStrLUT[BXDM_Picture_PullDown_eMax] =
{
   BDBG_STRING_INLINE("ukn"),   /* x */
   BDBG_STRING_INLINE("T  "),   /* BXDM_Picture_PullDown_eTop */
   BDBG_STRING_INLINE("B  "),   /* BXDM_Picture_PullDown_eBottom*/
   BDBG_STRING_INLINE("TB "),   /* BXDM_Picture_PullDown_eTopBottom */
   BDBG_STRING_INLINE("BT "),   /* BXDM_Picture_PullDown_eBottomTop*/
   BDBG_STRING_INLINE("TBT"),   /* BXDM_Picture_PullDown_eTopBottomTop */
   BDBG_STRING_INLINE("BTB"),   /* BXDM_Picture_PullDown_eBottomTopBottom */
   BDBG_STRING_INLINE("X2 "),   /* BXDM_Picture_PullDown_eFrameX2 */
   BDBG_STRING_INLINE("X3 "),   /* BXDM_Picture_PullDown_eFrameX3 */
   BDBG_STRING_INLINE("X1 "),   /* BXDM_Picture_PullDown_eFrameX1 */
   BDBG_STRING_INLINE("X4 ")    /* BXDM_Picture_PullDown_eFrameX4 */
};

/* SW7445-586: H265/HEVC split interlaced. Use an "s" to highlight that there are
 * two separate picture buffers.  Should only see Ts, Bs, TBs or BTs. */
const char * const BXDM_P_SiPullDownEnumToStrLUT[BXDM_Picture_PullDown_eMax] =
{
   BDBG_STRING_INLINE("ukn"),    /* x */
   BDBG_STRING_INLINE("Ts  "),   /* BXDM_Picture_PullDown_eTop */
   BDBG_STRING_INLINE("Bs  "),   /* BXDM_Picture_PullDown_eBottom*/
   BDBG_STRING_INLINE("TBs "),   /* BXDM_Picture_PullDown_eTopBottom */
   BDBG_STRING_INLINE("BTs "),   /* BXDM_Picture_PullDown_eBottomTop*/
   BDBG_STRING_INLINE("TBTs"),   /* BXDM_Picture_PullDown_eTopBottomTop */
   BDBG_STRING_INLINE("BTBs"),   /* BXDM_Picture_PullDown_eBottomTopBottom */
   BDBG_STRING_INLINE("X2s "),   /* BXDM_Picture_PullDown_eFrameX2 */
   BDBG_STRING_INLINE("X3s "),   /* BXDM_Picture_PullDown_eFrameX3 */
   BDBG_STRING_INLINE("X1s "),   /* BXDM_Picture_PullDown_eFrameX1 */
   BDBG_STRING_INLINE("X4s ")    /* BXDM_Picture_PullDown_eFrameX4 */
};

/* SW7445-1638: for split interlaced, highlight when the repeat flag is
 * set to aid with debug. */
const char * const BXDM_P_SiRepeatPullDownEnumToStrLUT[BXDM_Picture_PullDown_eMax] =
{
   BDBG_STRING_INLINE("ukn"),    /* x */
   BDBG_STRING_INLINE("Tsr "),   /* BXDM_Picture_PullDown_eTop */
   BDBG_STRING_INLINE("Bsr "),   /* BXDM_Picture_PullDown_eBottom*/
   BDBG_STRING_INLINE("TBsr"),   /* BXDM_Picture_PullDown_eTopBottom */
   BDBG_STRING_INLINE("BTsr"),   /* BXDM_Picture_PullDown_eBottomTop*/
   BDBG_STRING_INLINE("TBTsr"),   /* BXDM_Picture_PullDown_eTopBottomTop */
   BDBG_STRING_INLINE("BTBsr"),   /* BXDM_Picture_PullDown_eBottomTopBottom */
   BDBG_STRING_INLINE("X2sr"),   /* BXDM_Picture_PullDown_eFrameX2 */
   BDBG_STRING_INLINE("X3sr"),   /* BXDM_Picture_PullDown_eFrameX3 */
   BDBG_STRING_INLINE("X1sr"),   /* BXDM_Picture_PullDown_eFrameX1 */
   BDBG_STRING_INLINE("X4sr")    /* BXDM_Picture_PullDown_eFrameX4 */
};

const char * const BXDM_P_DisplayFieldModeToStrLUT[BXDM_PictureProvider_DisplayFieldMode_eMax]=
{
   BDBG_STRING_INLINE("bf"),       /* BXDM_PictureProvider_DisplayFieldMode_eBothField */
   BDBG_STRING_INLINE("to"),       /* BXDM_PictureProvider_DisplayFieldMode_eTopFieldOnly */
   BDBG_STRING_INLINE("bo"),       /* BXDM_PictureProvider_DisplayFieldMode_eBottomFieldOnly */
   BDBG_STRING_INLINE("sf"),       /* BXDM_PictureProvider_DisplayFieldMode_eSingleField */
   BDBG_STRING_INLINE("at"),       /* BXDM_PictureProvider_DisplayFieldMode_eAuto */
};

const char * const BXDM_P_PPOrientationToStrLUT[BXDM_PictureProvider_Orientation_eMax]=
{
   BDBG_STRING_INLINE("2D"),
   BDBG_STRING_INLINE("LfRt"),
   BDBG_STRING_INLINE("OvUn"),
   BDBG_STRING_INLINE("LRff"),
   BDBG_STRING_INLINE("RLff")
};

const char * const BXDM_P_PulldownModeToStrLUT[BXDM_PictureProvider_PulldownMode_eMax] =
{
   BDBG_STRING_INLINE("TB"),    /* BXDM_PictureProvider_PulldownMode_eTopBottom */
   BDBG_STRING_INLINE("BT"),    /* BXDM_PictureProvider_PulldownMode_eBottomTop */
   BDBG_STRING_INLINE("En")     /* BXDM_PictureProvider_PulldownMode_eUseEncodedFormat */
};

const char * const BXDM_P_FrameAdvanceModeToStrLUT[BXDM_PictureProvider_FrameAdvanceMode_eMax]=
{
   BDBG_STRING_INLINE("Off"),      /* BXDM_PictureProvider_FrameAdvanceMode_eOff */
   BDBG_STRING_INLINE("Fld"),      /* BXDM_PictureProvider_FrameAdvanceMode_eField */
   BDBG_STRING_INLINE("Frm"),      /* BXDM_PictureProvider_FrameAdvanceMode_eFrame */
   BDBG_STRING_INLINE("FbF")       /* BXDM_PictureProvider_FrameAdvanceMode_eFrameByField */
};

/*#define BXDM_PPDBG_S_MAX_VIDEO_PROTOCOL 21*/

const char * const BXDM_P_VideoCompressionStdToStrLUT[BXDM_P_MAX_VIDEO_PROTOCOL]=
{
   BDBG_STRING_INLINE("H264"),           /* H.264 */
   BDBG_STRING_INLINE("MPEG2"),          /* MPEG-2 */
   BDBG_STRING_INLINE("H261"),           /* H.261 */
   BDBG_STRING_INLINE("H263"),           /* H.263 */
   BDBG_STRING_INLINE("VC1"),            /* VC1 Advanced profile */
   BDBG_STRING_INLINE("MPEG1"),          /* MPEG-1 */
   BDBG_STRING_INLINE("MPEG2DTV"),       /* MPEG-2 DirecTV DSS ES */
   BDBG_STRING_INLINE("VC1SimpleMain"),  /* VC1 Simple & Main profile */
   BDBG_STRING_INLINE("MPEG4Part2"),     /* MPEG 4, Part 2. */
   BDBG_STRING_INLINE("AVS"),            /* AVS Jinzhun profile. */
   BDBG_STRING_INLINE("MPEG2_DSS_PES"),  /* MPEG-2 DirecTV DSS PES */
   BDBG_STRING_INLINE("SVC"),            /* Scalable Video Codec */
   BDBG_STRING_INLINE("SVC_BL"),         /* Scalable Video Codec Base Layer */
   BDBG_STRING_INLINE("MVC"),            /* MVC Multi View Coding */
   BDBG_STRING_INLINE("VP6"),            /* VP6 */
   BDBG_STRING_INLINE("VP7"),            /* VP7 */
   BDBG_STRING_INLINE("VP8"),            /* VP8 */
   BDBG_STRING_INLINE("RV9"),            /* Real Video 9 */
   BDBG_STRING_INLINE("SPARK"),          /* Sorenson Spark */
   BDBG_STRING_INLINE("MJPEG"),          /* Motion Jpeg */
   BDBG_STRING_INLINE("HEVC")            /* H.265 */
};

const char * const BXDM_P_TrickModeToStrLUT[BXDM_PictureProvider_TrickMode_eMax]=
{
   BDBG_STRING_INLINE("auto"),     /* BXDM_PictureProvider_TrickMode_eAuto */
   BDBG_STRING_INLINE("normal"),   /* BXDM_PictureProvider_TrickMode_eNormal */
   BDBG_STRING_INLINE("sparse"),   /* BXDM_PictureProvider_TrickMode_eSparsePictures */
   BDBG_STRING_INLINE("pause"),    /* BXDM_PictureProvider_TrickMode_ePause */
   BDBG_STRING_INLINE("rew"),      /* BXDM_PictureProvider_TrickMode_eRewind */
   BDBG_STRING_INLINE("ff")        /* BXDM_PictureProvider_TrickMode_eFastForward */

};

const char * const BXDM_P_FrameRateDetectionModeToStrLUT[BXDM_PictureProvider_FrameRateDetectionMode_eMax]=
{
   BDBG_STRING_INLINE("off"),   /* BXDM_PictureProvider_FrameRateDetectionMode_eOff */
   BDBG_STRING_INLINE("fast"),  /* BXDM_PictureProvider_FrameRateDetectionMode_eFast */
   BDBG_STRING_INLINE("stable") /* BXDM_PictureProvider_FrameRateDetectionMode_eStable */
};

const char * const BXDM_P_ErrorHandlingModeToStrLUT[BXDM_PictureProvider_ErrorHandlingMode_eMax]=
{
   BDBG_STRING_INLINE("off"),      /* BXDM_PictureProvider_ErrorHandlingMode_eOff */
   BDBG_STRING_INLINE("pic"),      /* BXDM_PictureProvider_ErrorHandlingMode_ePicture */
   BDBG_STRING_INLINE("prog")      /* BXDM_PictureProvider_ErrorHandlingMode_ePrognostic */
};

const char * const BXDM_P_BFMTRefreshRateToStrLUT[BFMT_Vert_eLast]=
{
   BDBG_STRING_INLINE("Ukn"),      /* BFMT_Vert_eInvalid */
   BDBG_STRING_INLINE("12.5"),     /* BFMT_Vert_e12_5Hz */
   BDBG_STRING_INLINE("14.985"),   /* BFMT_Vert_e14_985Hz */
   BDBG_STRING_INLINE("15"),       /* BFMT_Vert_e15Hz */
   BDBG_STRING_INLINE("20"),       /* BFMT_Vert_e20Hz */
   BDBG_STRING_INLINE("23.97"),    /* BFMT_Vert_e23_976Hz */
   BDBG_STRING_INLINE("24"),       /* BFMT_Vert_e24Hz */
   BDBG_STRING_INLINE("25"),       /* BFMT_Vert_e25Hz */
   BDBG_STRING_INLINE("29.97"),    /* BFMT_Vert_e29_97Hz */
   BDBG_STRING_INLINE("30"),       /* BFMT_Vert_e30Hz */
   BDBG_STRING_INLINE("48"),       /* BFMT_Vert_e48Hz */
   BDBG_STRING_INLINE("50"),       /* BFMT_Vert_e50Hz */
   BDBG_STRING_INLINE("59.94"),    /* BFMT_Vert_e59_94Hz */
   BDBG_STRING_INLINE("60"),       /* BFMT_Vert_e60Hz */
   BDBG_STRING_INLINE("100"),
   BDBG_STRING_INLINE("119.88"),
   BDBG_STRING_INLINE("120")
   BDBG_STRING_INLINE("7.493"),    /* BFMT_Vert_e7_493Hz */
   BDBG_STRING_INLINE("7.5"),      /* BFMT_Vert_e7_5Hz */
   BDBG_STRING_INLINE("9.99"),     /* BFMT_Vert_e9_99Hz */
   BDBG_STRING_INLINE("10"),       /* BFMT_Vert_e10Hz */
   BDBG_STRING_INLINE("11.988"),   /* BFMT_Vert_e11_988Hz */
   BDBG_STRING_INLINE("12"),       /* BFMT_Vert_e12Hz */
   BDBG_STRING_INLINE("19.98"),    /* BFMT_Vert_e19_98Hz */
};
