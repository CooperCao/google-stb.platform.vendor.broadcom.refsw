/***************************************************************************
 *     Copyright (c) 2003-2013, Broadcom Corporation
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
 * [File Description:]
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 ***************************************************************************/

#include "bstd.h"
#include "bkni.h"
#include "bdbg.h"

#include "bxdm_pp.h"
#include "bxdm_pp_priv.h"
#include "bxdm_pp_dbg.h"
#include "bxdm_pp_qm.h"


BDBG_MODULE(BXDM_PPDBG);
BDBG_FILE_MODULE(BXDM_MFD1);
BDBG_FILE_MODULE(BXDM_MFD2);
BDBG_FILE_MODULE(BXDM_MFD3);
BDBG_FILE_MODULE(BXDM_PPQM);
BDBG_FILE_MODULE(BXDM_CFG);
BDBG_FILE_MODULE(BXDM_PPDBG2);
BDBG_FILE_MODULE(BXDM_PPDBC);

#if BDBG_DEBUG_BUILD

extern uint32_t BXDM_PPTMR_lutVsyncsPersSecond[];

/*
 * Lookup tables mapping variables to strings.
 */
static const char sInterruptPolarityNameLUT[BAVC_Polarity_eFrame + 1] =
{
  't', /* BAVC_Polarity_eTopField */
  'b', /* BAVC_Polarity_eBotField */
  'f'  /* BAVC_Polarity_eFrame */
};

static const char sPicturePolarityNameLUT[BXDM_PictureProvider_P_InterruptType_eMax][BAVC_Polarity_eFrame + 1] =
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


static const char sHexToCharLUT[16] =
{
   '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
};

static const char sSelectionLUT[BXDM_PPDBG_Selection_eMax] =
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

static const char * const s_aPolarityToStrLUT[BAVC_Polarity_eFrame+1] =
{
   "T",  /* Top field */
   "B",  /* Bottom field */
   "F",  /* Progressive frame */
};

static const char * const s_aBAVCFrameRateToStrLUT[BXDM_PictureProvider_P_MAX_FRAMERATE] =
{
   "ukn",      /* Unknown */
   "23.97",    /* 23.976 */
   "24",       /* 24 */
   "25",       /* 25 */
   "29.97",    /* 29.97 */
   "30",       /* 30 */
   "50",       /* 50 */
   "59.94",    /* 59.94 */
   "60",       /* 60 */
   "14.98",    /* 14.985 */
   "7.49",     /* 7.493 */
   "10",       /* 10 */
   "15",       /* 15 */
   "20",       /* 20 */
   "12.5",     /* SW7584-331: add support for BAVC_FrameRateCode_e12_5 */
   "100",
   "119.88",
   "120",
   "19.98",    /* SWSTB-378: add support for BAVC_FrameRateCode_e19_98 */
};

static const char * const s_aFrameRateTypeToStrLUT[BXDM_PictureProvider_P_FrameRateType_eMax] =
{
   "cod",   /* coded in the stream */
   "def",   /* as specified by BXDM_PictureProvider_SetDefaultFrameRate_isr   */
   "ovr",   /* as specified by BXDM_PictureProvider_SetFrameRateOverride_isr  */
   "frd",   /* calculated in the FRD code using the PTS values                */
   "hcd"    /* using the values hardcoded in BXDM_PPTSM_P_PtsCalculateParameters_isr */
};

static const char * const s_aOrientationToStrLUT[BFMT_Orientation_eLeftRight_Enhanced+1] =
{
   "2D",       /* 2D */
   "LftRgt",   /* 3D left right */
   "OvrUnd",   /* 3D over under */
   "FF-Lf",    /* 3D left */
   "FF-Rt",    /* 3D right */
   "LR-En"     /* 3D left right, enhancement picture */
};

static const char * const s_aBAVCPictureCodingToStrLUT[BAVC_PictureCoding_eMax] =
{
   "u",     /* Picture Coding Type Unknown */
   "I",     /* Picture Coding Type I */
   "P",     /* Picture Coding Type P */
   "B"      /* Picture Coding Type B */
};

static const char * const s_aBXDMPictureCodingToStrLUT[BXDM_Picture_Coding_eMax] =
{
   "u",     /* BXDM_Picture_Coding_eUnknown */
   "I",     /* BXDM_Picture_Coding_eI */
   "P",     /* BXDM_Picture_Coding_eP */
   "B"      /* BXDM_Picture_Coding_eB */
};

static const char * const s_aAspectRatioToStrLUT[BFMT_AspectRatio_eSAR+1] =
{
  "Ukn",    /* Unkown/Reserved */
  "Sqr",    /* square pixel */
  "4x3",    /* 4:3 */
  "16x9",   /* 16:9 */
  "221x1",  /* 2.21:1 */
  "15x9",   /* 15:9 */
  "SAR"     /* no DAR available, use SAR instead */
};

static const char * const s_aMonitorRefreshRateToStrLUT[BXDM_PictureProvider_MonitorRefreshRate_eMax] =
{
   "Ukn",      /* BXDM_PictureProvider_MonitorRefreshRate_eUnknown */
   "12.5",    /* BXDM_PictureProvider_MonitorRefreshRate_e12_5Hz */
   "14.985",    /* BXDM_PictureProvider_MonitorRefreshRate_e14_985Hz */
   "15",    /* BXDM_PictureProvider_MonitorRefreshRate_e15Hz */
   "20",    /* BXDM_PictureProvider_MonitorRefreshRate_e20Hz */
   "23.97",    /* BXDM_PictureProvider_MonitorRefreshRate_e23_976Hz */
   "24",       /* BXDM_PictureProvider_MonitorRefreshRate_e24Hz */
   "25",       /* BXDM_PictureProvider_MonitorRefreshRate_e25Hz */
   "29.97",    /* BXDM_PictureProvider_MonitorRefreshRate_e29_97Hz */
   "30",       /* BXDM_PictureProvider_MonitorRefreshRate_e30Hz */
   "50",       /* BXDM_PictureProvider_MonitorRefreshRate_e50Hz */
   "59.94",    /* BXDM_PictureProvider_MonitorRefreshRate_e59_94Hz */
   "60",       /* BXDM_PictureProvider_MonitorRefreshRate_e60Hz */
   "48",       /* BXDM_PictureProvider_MonitorRefreshRate_e48Hz */
   "100",      /* BXDM_PictureProvider_MonitorRefreshRate_e100Hz */
   "119.88",   /* BXDM_PictureProvider_MonitorRefreshRate_e119_88Hz */
   "120"       /* BXDM_PictureProvider_MonitorRefreshRate_e120Hz */
};

static const char * const s_aSTCTrickModeToStrLUT[BXDM_PictureProvider_P_STCTrickMode_eMax] =
{
   "Off",   /* BXDM_PictureProvider_P_STCTrickMode_eOff */
   "FstFwd",   /* BXDM_PictureProvider_P_STCTrickMode_eFastForward */
   "SlwFwd",   /* BXDM_PictureProvider_P_STCTrickMode_eSlowMotion */
   "Pause",   /* BXDM_PictureProvider_P_STCTrickMode_ePause */
   "FstRwd",   /* BXDM_PictureProvider_P_STCTrickMode_eFastRewind */
   "SlwRwd"    /* BXDM_PictureProvider_P_STCTrickMode_eSlowRewind */
};

static const char * const s_aTSMResultToStrLUT[BXDM_PictureProvider_TSMResult_eMax] =
{
   "e",  /* BXDM_PictureProvider_TSMResult_eTooEarly */
   "w",  /* BXDM_PictureProvider_TSMResult_eWait */
   "p",  /* BXDM_PictureProvider_TSMResult_ePass */
   "l",  /* BXDM_PictureProvider_TSMResult_eTooLate */
   "d"   /* BXDM_PictureProvider_TSMResult_eDrop */
};


static const char * const s_aPullDownEnumToStrLUT[BXDM_Picture_PullDown_eMax] =
{
   "ukn",   /* x */
   "T  ",   /* BXDM_Picture_PullDown_eTop */
   "B  ",   /* BXDM_Picture_PullDown_eBottom*/
   "TB ",   /* BXDM_Picture_PullDown_eTopBottom */
   "BT ",   /* BXDM_Picture_PullDown_eBottomTop*/
   "TBT",   /* BXDM_Picture_PullDown_eTopBottomTop */
   "BTB",   /* BXDM_Picture_PullDown_eBottomTopBottom */
   "X2 ",   /* BXDM_Picture_PullDown_eFrameX2 */
   "X3 ",   /* BXDM_Picture_PullDown_eFrameX3 */
   "X1 ",   /* BXDM_Picture_PullDown_eFrameX1 */
   "X4 "    /* BXDM_Picture_PullDown_eFrameX4 */
};

/* SW7445-586: H265/HEVC split interlaced. Use an "s" to highlight that there are
 * two separate picture buffers.  Should only see Ts, Bs, TBs or BTs. */
static const char * const s_aSiPullDownEnumToStrLUT[BXDM_Picture_PullDown_eMax] =
{
   "ukn",    /* x */
   "Ts  ",   /* BXDM_Picture_PullDown_eTop */
   "Bs  ",   /* BXDM_Picture_PullDown_eBottom*/
   "TBs ",   /* BXDM_Picture_PullDown_eTopBottom */
   "BTs ",   /* BXDM_Picture_PullDown_eBottomTop*/
   "TBTs",   /* BXDM_Picture_PullDown_eTopBottomTop */
   "BTBs",   /* BXDM_Picture_PullDown_eBottomTopBottom */
   "X2s ",   /* BXDM_Picture_PullDown_eFrameX2 */
   "X3s ",   /* BXDM_Picture_PullDown_eFrameX3 */
   "X1s ",   /* BXDM_Picture_PullDown_eFrameX1 */
   "X4s "    /* BXDM_Picture_PullDown_eFrameX4 */
};

/* SW7445-1638: for split interlaced, highlight when the repeat flag is
 * set to aid with debug. */
static const char * const s_aSiRepeatPullDownEnumToStrLUT[BXDM_Picture_PullDown_eMax] =
{
   "ukn",    /* x */
   "Tsr ",   /* BXDM_Picture_PullDown_eTop */
   "Bsr ",   /* BXDM_Picture_PullDown_eBottom*/
   "TBsr",   /* BXDM_Picture_PullDown_eTopBottom */
   "BTsr",   /* BXDM_Picture_PullDown_eBottomTop*/
   "TBTsr",   /* BXDM_Picture_PullDown_eTopBottomTop */
   "BTBsr",   /* BXDM_Picture_PullDown_eBottomTopBottom */
   "X2sr",   /* BXDM_Picture_PullDown_eFrameX2 */
   "X3sr",   /* BXDM_Picture_PullDown_eFrameX3 */
   "X1sr",   /* BXDM_Picture_PullDown_eFrameX1 */
   "X4sr"    /* BXDM_Picture_PullDown_eFrameX4 */
};

static const char * const s_aDisplayFieldModeToStrLUT[BXDM_PictureProvider_DisplayFieldMode_eMax]=
{
   "bf",       /* BXDM_PictureProvider_DisplayFieldMode_eBothField */
   "to",       /* BXDM_PictureProvider_DisplayFieldMode_eTopFieldOnly */
   "bo",       /* BXDM_PictureProvider_DisplayFieldMode_eBottomFieldOnly */
   "sf",       /* BXDM_PictureProvider_DisplayFieldMode_eSingleField */
   "at",       /* BXDM_PictureProvider_DisplayFieldMode_eAuto */
};

static const char * const s_aPPOrientationToStrLUT[BXDM_PictureProvider_Orientation_eMax]=
{
   "2D",
   "LfRt",
   "OvUn",
   "LRff",
   "RLff"
};

static const char * const s_aPulldownModeToStrLUT[BXDM_PictureProvider_PulldownMode_eMax] =
{
   "TB",    /* BXDM_PictureProvider_PulldownMode_eTopBottom */
   "BT",    /* BXDM_PictureProvider_PulldownMode_eBottomTop */
   "En"     /* BXDM_PictureProvider_PulldownMode_eUseEncodedFormat */
};

static const char * const s_aFrameAdvanceModeToStrLUT[BXDM_PictureProvider_FrameAdvanceMode_eMax]=
{
   "Off",      /* BXDM_PictureProvider_FrameAdvanceMode_eOff */
   "Fld",      /* BXDM_PictureProvider_FrameAdvanceMode_eField */
   "Frm",      /* BXDM_PictureProvider_FrameAdvanceMode_eFrame */
   "FbF"       /* BXDM_PictureProvider_FrameAdvanceMode_eFrameByField */
};

#define BXDM_PPDBG_S_MAX_VIDEO_PROTOCOL 21

static const char * const s_aVideoCompressionStdToStrLUT[BXDM_PPDBG_S_MAX_VIDEO_PROTOCOL]=
{
   "H264",           /* H.264 */
   "MPEG2",          /* MPEG-2 */
   "H261",           /* H.261 */
   "H263",           /* H.263 */
   "VC1",            /* VC1 Advanced profile */
   "MPEG1",          /* MPEG-1 */
   "MPEG2DTV",       /* MPEG-2 DirecTV DSS ES */
   "VC1SimpleMain",  /* VC1 Simple & Main profile */
   "MPEG4Part2",     /* MPEG 4, Part 2. */
   "AVS",            /* AVS Jinzhun profile. */
   "MPEG2_DSS_PES",  /* MPEG-2 DirecTV DSS PES */
   "SVC",            /* Scalable Video Codec */
   "SVC_BL",         /* Scalable Video Codec Base Layer */
   "MVC",            /* MVC Multi View Coding */
   "VP6",            /* VP6 */
   "VP7",            /* VP7 */
   "VP8",            /* VP8 */
   "RV9",            /* Real Video 9 */
   "SPARK",          /* Sorenson Spark */
   "MJPEG",          /* Motion Jpeg */
   "HEVC"            /* H.265 */
};

static const char * const s_aTrickModeToStrLUT[BXDM_PictureProvider_TrickMode_eMax]=
{
   "auto",     /* BXDM_PictureProvider_TrickMode_eAuto */
   "normal",   /* BXDM_PictureProvider_TrickMode_eNormal */
   "sparse",   /* BXDM_PictureProvider_TrickMode_eSparsePictures */
   "pause",    /* BXDM_PictureProvider_TrickMode_ePause */
   "rew",      /* BXDM_PictureProvider_TrickMode_eRewind */
   "ff"        /* BXDM_PictureProvider_TrickMode_eFastForward */

};

static const char * const s_aFrameRateDetectionModeToStrLUT[BXDM_PictureProvider_FrameRateDetectionMode_eMax]=
{
   "off",   /* BXDM_PictureProvider_FrameRateDetectionMode_eOff */
   "fast",  /* BXDM_PictureProvider_FrameRateDetectionMode_eFast */
   "stable" /* BXDM_PictureProvider_FrameRateDetectionMode_eStable */
};

static const char * const s_aErrorHandlingModeToStrLUT[BXDM_PictureProvider_ErrorHandlingMode_eMax]=
{
   "off",      /* BXDM_PictureProvider_ErrorHandlingMode_eOff */
   "pic",      /* BXDM_PictureProvider_ErrorHandlingMode_ePicture */
   "prog"      /* BXDM_PictureProvider_ErrorHandlingMode_ePrognostic */
};

static const char * const s_aBFMTRefreshRateToStrLUT[BFMT_Vert_eLast]=
{
   "Ukn",      /* BFMT_Vert_eInvalid */
   "12.5",     /* BFMT_Vert_e12_5Hz */
   "14.985",   /* BFMT_Vert_e14_985Hz */
   "15",       /* BFMT_Vert_e15Hz */
   "20",       /* BFMT_Vert_e20Hz */
   "23.97",    /* BFMT_Vert_e23_976Hz */
   "24",       /* BFMT_Vert_e24Hz */
   "25",       /* BFMT_Vert_e25Hz */
   "29.97",    /* BFMT_Vert_e29_97Hz */
   "30",       /* BFMT_Vert_e30Hz */
   "48",       /* BFMT_Vert_e48Hz */
   "50",       /* BFMT_Vert_e50Hz */
   "59.94",    /* BFMT_Vert_e59_94Hz */
   "60",       /* BFMT_Vert_e60Hz */
   "100",
   "119.88",
   "120"
};

/*
 * Functions
 */
static void BXDM_PPDBG_S_AppendChar_isrsafe(
   BXDM_PPDBG_P_String * pstString,
   uint32_t uiCount,
   ...
   )
{
   uint32_t i;

   va_list args;

   va_start(args, uiCount);

   for( i=0; i < uiCount; i++ )
   {
      BXDM_PPDBG_P_APPEND_CHAR( pstString, va_arg(args, uint32_t) );
   }

   va_end(args);

   return;
}


#if 0
BERR_Code BXDM_PPDBG_P_ResetString(
   BXDM_PPDBG_P_String *pStringInfo
   )
{
   pStringInfo->szDebugStr[0] = '\0';
   pStringInfo->uiDebugStrOffset = 0;

   return BERR_SUCCESS;
}

BERR_Code BXDM_PPDBG_P_AppendString(
   BXDM_PPDBG_P_String *pStringInfo,
   const char *fmt,
   ...
   )
{
   BERR_Code rc = BERR_SUCCESS;

   if (pStringInfo->uiDebugStrOffset < sizeof(pStringInfo->szDebugStr))
   {
      va_list args;

      va_start(args, fmt);
      pStringInfo->uiDebugStrOffset += BKNI_Vsnprintf(pStringInfo->szDebugStr +
                                                     pStringInfo->uiDebugStrOffset,
                                                     sizeof(pStringInfo->szDebugStr) -
                                                     pStringInfo->uiDebugStrOffset,
                                                     fmt,
                                                     args);
      va_end(args);
   }
   else
   {
      rc = BERR_OUT_OF_DEVICE_MEMORY;
   }

   return rc;
}

BERR_Code BXDM_PPDBG_P_PrintString(
   const BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PPDBG_P_String *pStringInfo
   )
{
   BERR_Code rc = BERR_SUCCESS;

   BXVD_DBG_MSG(hXdmPP, ("%s", pStringInfo->szDebugStr));

   if (pStringInfo->uiDebugStrOffset >= sizeof(pStringInfo->szDebugStr))
   {
      BXVD_DBG_ERR(hXdmPP, ("Debug Buffer Overflow!"));
      rc = BERR_OUT_OF_DEVICE_MEMORY;
   }

   BXDM_PPDBG_P_ResetString(pStringInfo);

   return rc;
}
#endif
BERR_Code BXDM_PPDBG_P_OutputLog_isr(
   const BXDM_PictureProvider_Handle hXdmPP,
   const BXDM_PictureProvider_P_LocalState *pLocalState,
   const BAVC_MFD_Picture *pMFDPicture
   )
{
   BXDM_PPDBG_P_Info *pDebugInfo = &hXdmPP->stDMState.stDecode.stDebug;
   uint32_t uiPPBIndex;
   char iInfoFlag;

   /* Gather Stats for this VSYNC */
   uiPPBIndex = hXdmPP->stDMState.stChannel.stSelectedPicture.stPicParms.uiPPBIndex & 0xFFF;

   if (pMFDPicture->bMute)
   {
      iInfoFlag = 'M';
   }
   else if ((pMFDPicture->eInterruptPolarity != BAVC_Polarity_eFrame) &&
            (pMFDPicture->eSourcePolarity != BAVC_Polarity_eFrame) &&
            (pMFDPicture->eInterruptPolarity != pMFDPicture->eSourcePolarity) &&
            (BXDM_PictureProvider_P_InterruptType_eSingle == pLocalState->eInterruptType)
            )
   {
      iInfoFlag = '*';
   }
   else if (pMFDPicture->bPictureRepeatFlag)
   {
      iInfoFlag = 'R';
   }
   else
   {
      iInfoFlag = ' ';
   }

   if (pDebugInfo->uiVsyncCount == 0)
   {
      BKNI_Memset( &(pDebugInfo->stInterruptString), 0, sizeof( BXDM_PPDBG_P_String ) );
      BXDM_PPDBG_S_AppendChar_isrsafe( &(pDebugInfo->stInterruptString), 4, 'P', 'S', ':', ' ' );
   }

   BXDM_PPDBG_S_AppendChar_isrsafe(
         &(pDebugInfo->stInterruptString),
         8,
         sInterruptPolarityNameLUT[pMFDPicture->eInterruptPolarity],
         sPicturePolarityNameLUT[pLocalState->eInterruptType][pMFDPicture->eSourcePolarity],
         ':',
         sHexToCharLUT[ (uiPPBIndex >> 8) & 0xF ],
         sHexToCharLUT[ (uiPPBIndex >> 4) & 0xF ],
         sHexToCharLUT[ uiPPBIndex & 0xF ],
         iInfoFlag,
         ' ' );

   return BERR_SUCCESS;
}

BERR_Code BXDM_PPDBG_P_OutputSPOLog_isr(
   const BXDM_PictureProvider_Handle hXdmPP,
   const uint32_t uiOverrideBits
   )
{
   BXDM_PPDBG_P_Info *pDebugInfo = &hXdmPP->stDMState.stDecode.stDebug;
   BXDM_PPDBG_P_String * pStr = &pDebugInfo->stSourcePolarityOverrideString;

   if (pDebugInfo->uiVsyncCount == 0)
   {
      BKNI_Memset( pStr, 0, sizeof( BXDM_PPDBG_P_String ) );
      BXDM_PPDBG_S_AppendChar_isrsafe( pStr, 4, 'O', 'V', ':', ' ' );
   }

   if ( uiOverrideBits )
   {
      BXDM_PPDBG_S_AppendChar_isrsafe(
         pStr,
         8,
         sHexToCharLUT[ pDebugInfo->uiVsyncCount ],
         ':',
         sHexToCharLUT[ (uiOverrideBits >> 16) & 0xF ],
         sHexToCharLUT[ (uiOverrideBits >> 12) & 0xF ],
         sHexToCharLUT[ (uiOverrideBits >> 8) & 0xF ],
         sHexToCharLUT[ (uiOverrideBits >> 4) & 0xF ],
         sHexToCharLUT[ uiOverrideBits & 0xF ],
         ' ' );
   }
   else
   {
      /* If none of the bits are set, print spaces to make it
       * more obvious when something is set.
       */
      BXDM_PPDBG_S_AppendChar_isrsafe(
         pStr,
         8,
         sHexToCharLUT[ pDebugInfo->uiVsyncCount ],
         ':',
         ' ' ,
         ' ' ,
         ' ' ,
         ' ' ,
         ' ' ,
         ' ' );
   }

   if ( 0 != uiOverrideBits )
   {
      pDebugInfo->bPrintSPO = true;
   }

   return BERR_SUCCESS;
}

BERR_Code BXDM_PPDBG_P_SelectionLog_isr(
   const BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PPDBG_Selection eSelectionInfo
   )
{
   BXDM_PPDBG_P_Info *pDebugInfo = &hXdmPP->stDMState.stDecode.stDebug;
   BXDM_PPDBG_P_String * pStr = &pDebugInfo->stTSMString;

   if (pDebugInfo->abSelectionLogHeader[pDebugInfo->uiVsyncCount] == false)
   {
      if (pDebugInfo->uiVsyncCount == 0)
      {
         BKNI_Memset( pStr, 0, sizeof( BXDM_PPDBG_P_String ) );
         BXDM_PPDBG_S_AppendChar_isrsafe( pStr, 4, 'T', 'M', ':', ' ' );
      }
      else
      {
         BXDM_PPDBG_S_AppendChar_isrsafe( pStr, 1, ' ' );
      }

      BXDM_PPDBG_S_AppendChar_isrsafe(
            pStr,
            2,
            sHexToCharLUT[ pDebugInfo->uiVsyncCount & 0xF ],
            ':' );

      pDebugInfo->abSelectionLogHeader[pDebugInfo->uiVsyncCount] = true;
   }
   if ( BXDM_PPDBG_Selection_PolarityOverride_eSelectPrevious == eSelectionInfo
        || BXDM_PPDBG_Selection_PolarityOverride_eRepeatPrevious == eSelectionInfo
        || BXDM_PPDBG_Selection_PolarityOverride_eFICForceWait == eSelectionInfo
        || BXDM_PPDBG_Selection_PolarityOverride_e2ndSlotNextElement == eSelectionInfo )
   {
      BXDM_PPDBG_S_AppendChar_isrsafe( pStr, 1, '(' );
   }

   BXDM_PPDBG_S_AppendChar_isrsafe( pStr, 1, sSelectionLUT[eSelectionInfo] );

   if ( BXDM_PPDBG_Selection_PolarityOverride_eSelectPrevious == eSelectionInfo
        || BXDM_PPDBG_Selection_PolarityOverride_eRepeatPrevious == eSelectionInfo
        || BXDM_PPDBG_Selection_PolarityOverride_eFICForceWait == eSelectionInfo
        || BXDM_PPDBG_Selection_PolarityOverride_e2ndSlotNextElement == eSelectionInfo )
   {
      BXDM_PPDBG_S_AppendChar_isrsafe( pStr, 1, ')' );
   }


   return BERR_SUCCESS;
}

BERR_Code BXDM_PPDBG_P_CallbackTriggeredLog_isr(
   const BXDM_PictureProvider_Handle hXdmPP,
   const uint32_t uiCallbackTriggeredBits
   )
{
   BXDM_PPDBG_P_Info *pDebugInfo = &hXdmPP->stDMState.stDecode.stDebug;
   BXDM_PPDBG_P_String * pStr = &pDebugInfo->stCallbackString;

   if (pDebugInfo->uiVsyncCount == 0)
   {
      BKNI_Memset( pStr, 0, sizeof( BXDM_PPDBG_P_String ) );
      BXDM_PPDBG_S_AppendChar_isrsafe( pStr, 4, 'C', 'B', ':', ' ' );
   }


   if ( uiCallbackTriggeredBits )
   {
      BXDM_PPDBG_S_AppendChar_isrsafe(
         pStr,
         8,
         sHexToCharLUT[ pDebugInfo->uiVsyncCount ],
         ':',
         sHexToCharLUT[ (uiCallbackTriggeredBits >> 16) & 0xF ],
         sHexToCharLUT[ (uiCallbackTriggeredBits >> 12) & 0xF ],
         sHexToCharLUT[ (uiCallbackTriggeredBits >> 8) & 0xF ],
         sHexToCharLUT[ (uiCallbackTriggeredBits >> 4) & 0xF ],
         sHexToCharLUT[ uiCallbackTriggeredBits & 0xF ],
         ' ' );
   }
   else
   {
      /* If none of the bits are set, print spaces to make
       * it more obvious when something is set.
       */
      BXDM_PPDBG_S_AppendChar_isrsafe(
         pStr,
         8,
         sHexToCharLUT[ pDebugInfo->uiVsyncCount ],
         ':',
         ' ' ,
         ' ' ,
         ' ' ,
         ' ' ,
         ' ' ,
         ' ' );
   }

   if ( 0 != uiCallbackTriggeredBits )
   {
      pDebugInfo->bPrintCallbacks = true;
   }

   return BERR_SUCCESS;
}

BERR_Code BXDM_PPDBG_P_StateLog_isr(
   const BXDM_PictureProvider_Handle hXdmPP,
   const uint32_t uiStateBits
   )
{
   BXDM_PPDBG_P_Info *pDebugInfo = &hXdmPP->stDMState.stDecode.stDebug;
   BXDM_PPDBG_P_String * pStr = &pDebugInfo->stStateString;

   if (pDebugInfo->uiVsyncCount == 0)
   {
      BKNI_Memset( pStr, 0, sizeof( BXDM_PPDBG_P_String ) );
      BXDM_PPDBG_S_AppendChar_isrsafe( pStr, 4, 'S', 'T', ':', ' ' );
   }

   BXDM_PPDBG_S_AppendChar_isrsafe(
         pStr,
         8,
         sHexToCharLUT[ pDebugInfo->uiVsyncCount ],
         ':',
         sHexToCharLUT[ (uiStateBits >> 16) & 0xF ],
         sHexToCharLUT[ (uiStateBits >> 12) & 0xF ],
         sHexToCharLUT[ (uiStateBits >> 8) & 0xF ],
         sHexToCharLUT[ (uiStateBits >> 4) & 0xF ],
         sHexToCharLUT[ uiStateBits & 0xF ],
         ' ' );

   return BERR_SUCCESS;
}

BERR_Code BXDM_PPDBG_P_State2Log_isr(
   const BXDM_PictureProvider_Handle hXdmPP,
   const uint32_t uiStateBits
   )
{
   BXDM_PPDBG_P_Info *pDebugInfo = &hXdmPP->stDMState.stDecode.stDebug;
   BXDM_PPDBG_P_String * pStr = &pDebugInfo->stState2String;

   if (pDebugInfo->uiVsyncCount == 0)
   {
      BKNI_Memset( pStr, 0, sizeof( BXDM_PPDBG_P_String ) );
      BXDM_PPDBG_S_AppendChar_isrsafe( pStr, 3, 'S', '2', ':' );
   }


   if ( uiStateBits )
   {
      BXDM_PPDBG_S_AppendChar_isrsafe(
         pStr,
         8,
         sHexToCharLUT[ pDebugInfo->uiVsyncCount ],
         ':',
         sHexToCharLUT[ (uiStateBits >> 16) & 0xF ],
         sHexToCharLUT[ (uiStateBits >> 12) & 0xF ],
         sHexToCharLUT[ (uiStateBits >> 8) & 0xF ],
         sHexToCharLUT[ (uiStateBits >> 4) & 0xF ],
         sHexToCharLUT[ uiStateBits & 0xF ],
         ' ' );
   }
   else
   {
      /* If none of the bits are set, print spaces to make
       * it more obvious when something is set.
       */
      BXDM_PPDBG_S_AppendChar_isrsafe(
         pStr,
         8,
         sHexToCharLUT[ pDebugInfo->uiVsyncCount ],
         ':',
         ' ' ,
         ' ' ,
         ' ' ,
         ' ' ,
         ' ' ,
         ' ' );
   }

   if ( 0 != uiStateBits )
   {
      pDebugInfo->bPrintState2 = true;
   }

   return BERR_SUCCESS;
}


BERR_Code BXDM_PPDBG_P_StcDeltaLog_isr(
   const BXDM_PictureProvider_Handle hXdmPP,
   const BXDM_PictureProvider_P_LocalState * pLocalState
   )
{
   BXDM_PPDBG_P_Info *pDebugInfo = &hXdmPP->stDMState.stDecode.stDebug;
   BXDM_PPDBG_P_String * pStr = &pDebugInfo->stStcDeltaString;

   /*uint32_t uiStcDelta = pLocalState->uiStcSnapshot - hXdmPP->stDMState.stDecode.uiLastStcSnapshot;*/
   uint32_t uiStcDelta = pLocalState->stEffectiveSTCDelta[BXDM_PictureProvider_DisplayMode_eTSM].uiWhole;

   if (pDebugInfo->uiVsyncCount == 0)
   {
      BKNI_Memset( pStr, 0, sizeof( BXDM_PPDBG_P_String ) );
      BXDM_PPDBG_S_AppendChar_isrsafe( pStr, 4, 'C', 'D', ':', ' ' );
   }

   BXDM_PPDBG_S_AppendChar_isrsafe(
         pStr,
         8,
         sHexToCharLUT[ pDebugInfo->uiVsyncCount ],
         ':',
         sHexToCharLUT[ (uiStcDelta >> 16) & 0xF ],
         sHexToCharLUT[ (uiStcDelta >> 12) & 0xF ],
         sHexToCharLUT[ (uiStcDelta >> 8) & 0xF ],
         sHexToCharLUT[ (uiStcDelta >> 4) & 0xF ],
         sHexToCharLUT[ uiStcDelta & 0xF ],
         ' ' );

   return BERR_SUCCESS;
}


BERR_Code BXDM_PPDBG_P_DecoderDropLog_isr(
   const BXDM_PictureProvider_Handle hXdmPP,
   const uint32_t uiPendingDrop
   )
{
   BXDM_PPDBG_P_Info *pDebugInfo = &hXdmPP->stDMState.stDecode.stDebug;
   BXDM_PPDBG_P_String * pStr = &pDebugInfo->stPendingDropString;

   if (pDebugInfo->uiVsyncCount == 0)
   {
      BKNI_Memset( pStr, 0, sizeof( BXDM_PPDBG_P_String ) );
      BXDM_PPDBG_S_AppendChar_isrsafe( pStr, 4, 'D', 'R', ':', ' ' );
   }

   if ( uiPendingDrop )
   {
      BXDM_PPDBG_S_AppendChar_isrsafe(
         pStr,
         8,
         sHexToCharLUT[ pDebugInfo->uiVsyncCount ],
         ':',
         sHexToCharLUT[ (uiPendingDrop >> 16) & 0xF ],
         sHexToCharLUT[ (uiPendingDrop >> 12) & 0xF ],
         sHexToCharLUT[ (uiPendingDrop >> 8) & 0xF ],
         sHexToCharLUT[ (uiPendingDrop >> 4) & 0xF ],
         sHexToCharLUT[ uiPendingDrop & 0xF ],
         ' ' );
   }
   else
   {
      /* If none of the bits are set, print spaces to make it
       * more obvious when something is set.
       */
      BXDM_PPDBG_S_AppendChar_isrsafe(
         pStr,
         8,
         sHexToCharLUT[ pDebugInfo->uiVsyncCount ],
         ':',
         ' ' ,
         ' ' ,
         ' ' ,
         ' ' ,
         ' ' ,
         ' ' );
   }

   if ( 0 != uiPendingDrop )
   {
      pDebugInfo->bPrintDropCount = true;
   }

   return BERR_SUCCESS;
}

BERR_Code BXDM_PPDBG_P_Print_isr(
   const BXDM_PictureProvider_Handle hXdmPP,
   const BXDM_PictureProvider_P_LocalState* pLocalState,
   bool bForcePrint
   )
{
   BXDM_PPDBG_P_Info *pDebugInfo = &hXdmPP->stDMState.stDecode.stDebug;

   /* See if we need to print out stats */
   if (pDebugInfo->uiVsyncCount == (BXDM_PPDBG_P_MAX_VSYNC_DEPTH - 1)
       || true == bForcePrint )
   {
      int32_t iAverageStcDelta;
      uint32_t uiAverageFractionBase10;

      BXDM_PictureProvider_MonitorRefreshRate eMonitorRefreshRate = hXdmPP->stDMConfig.eMonitorRefreshRate;

      if ( eMonitorRefreshRate >= BXDM_PictureProvider_MonitorRefreshRate_eMax )
      {
         eMonitorRefreshRate = BXDM_PictureProvider_MonitorRefreshRate_eUnknown;
      }

      /* Figure out the STC delta (from the previous vsync) based on assorted state.
       */
      if ( true == hXdmPP->stDMConfig.stClockOverride.bEnableClockOverride )
      {
         iAverageStcDelta = hXdmPP->stDMConfig.stClockOverride.iStcDelta;
         uiAverageFractionBase10 = 0;
      }
      else if ( true == hXdmPP->stDMConfig.bSTCValid )
      {
         iAverageStcDelta = pLocalState->stDeltaSTCAvg.uiWhole;

         /* Convert the fractional part of stDeltaSTCAvg to a base 10 value for display. */
         BXDM_PPFP_P_FixPtBinaryFractionToBase10_isr(
                  (BXDM_PPFP_P_DataType *)&(pLocalState->stDeltaSTCAvg),
                  2,
                  &(uiAverageFractionBase10)
                  );
      }
      else
      {
         iAverageStcDelta = pLocalState->stEffectiveSTCDelta[BXDM_PictureProvider_DisplayMode_eTSM].uiWhole;
         uiAverageFractionBase10 = 0;
      }


      BXVD_DBG_MSG(hXdmPP, (" DM Log (ch:%02x stc:%08x%c mr:%sHz edSTC:%u adSTC:%d.%u pbr:%d%c tm:%s %s)",
                                BXDM_PPDBG_FORMAT_INSTANCE_ID( hXdmPP ),
                                pLocalState->uiStcSnapshot,
                                (hXdmPP->stDMState.stDecode.iStcJitterCorrectionOffset != 0)?'*':' ',
                                s_aMonitorRefreshRateToStrLUT[eMonitorRefreshRate],
                                pLocalState->stSTCDelta.uiWhole,
                                iAverageStcDelta,
                                uiAverageFractionBase10,
                                ( pLocalState->uiSlowMotionRate / BXDM_PICTUREPROVIDER_NORMAL_PLAYBACK_RATE_EXTRA_DECIMALS ),
                                '%',
                                ( pLocalState->eSTCTrickMode < BXDM_PictureProvider_P_STCTrickMode_eMax) ?
                                          s_aSTCTrickModeToStrLUT[ pLocalState->eSTCTrickMode ] : "error",
                                hXdmPP->stDMConfig.bPlayback?"pb":"lv"
                                ));

      /* Print TSM Logs */
      BXVD_DBG_MSG(hXdmPP, ("%s", (char *)&pDebugInfo->stTSMString.szDebugStr));

      /* Print Decoder Drop Logs */
      if ( true ==  pDebugInfo->bPrintDropCount )
      {
         BXVD_DBG_MSG(hXdmPP, ("%s", (char *)&pDebugInfo->stPendingDropString.szDebugStr));
      }

      /* Print picture selecton log */
      BXVD_DBG_MSG(hXdmPP, ("%s", (char *)&pDebugInfo->stInterruptString.szDebugStr));

      /* Print stats for Source Polarity Override */
      if ( true == pDebugInfo->bPrintSPO )
      {
         BXVD_DBG_MSG(hXdmPP, ("%s", (char *)&pDebugInfo->stSourcePolarityOverrideString.szDebugStr));
      }

      /* Print stats for Callbacks */
      if ( true == pDebugInfo->bPrintCallbacks )
      {
         BXVD_DBG_MSG(hXdmPP, ("%s", (char *)&pDebugInfo->stCallbackString.szDebugStr));
      }

      /* Print State Logs: */
      BXVD_DBG_MSG(hXdmPP, ("%s", (char *)&pDebugInfo->stStateString.szDebugStr));

      /* Print stats for state 2 */
      if ( true == pDebugInfo->bPrintState2 )
      {
         BDBG_MODULE_MSG( BXDM_PPDBG2, ("%s", (char *)&pDebugInfo->stState2String.szDebugStr));
      }

      BDBG_MODULE_MSG( BXDM_PPDBC, ("%s", (char *)&pDebugInfo->stStcDeltaString.szDebugStr));

      BKNI_Memset(pDebugInfo, 0, sizeof(BXDM_PPDBG_P_Info));
   }
   else
   {
      pDebugInfo->uiVsyncCount++;
   }

   return BERR_SUCCESS;
}

void BXDM_PPDBG_P_PrintStartDecode_isr(
   const BXDM_PictureProvider_Handle hXdmPP
   )
{
   /* SW7445-1259: reduce messages printed at start decode time */
   BXDM_PictureProvider_P_State_Decode *pDecode = &hXdmPP->stDMState.stDecode;
   BXDM_PPDBG_P_Info *pDebug = &pDecode->stDebug;

   BXVD_DBG_MSG(hXdmPP, ("--- %x:[%02x.xxx] BXDM_PictureProvider_StartDecode_isr has been called ---",
                              pDebug->uiVsyncCount,
                              BXDM_PPDBG_FORMAT_INSTANCE_ID( hXdmPP )
                              ));
}


void BXDM_PPDBG_P_PrintStopDecode_isr(
   const BXDM_PictureProvider_Handle hXdmPP
   )
{
   BXDM_PictureProvider_P_State_Decode *pDecode = &hXdmPP->stDMState.stDecode;
   BXDM_PPDBG_P_Info *pDebug = &pDecode->stDebug;

   BXVD_DBG_MSG(hXdmPP, ("--- %x:[%02x.xxx] BXDM_PictureProvider_StopDecode_isr has been called ---",
                              pDebug->uiVsyncCount,
                              BXDM_PPDBG_FORMAT_INSTANCE_ID( hXdmPP )
                              ));
}



/*
** SW7335-781: Output warning when forcing picture selection override
** Output Selection Mode override message only once, if:
** an override to vTSM mode occurred
** the override was not the same as the previous Override
** function assumes new selection mode is VSYNC (vTSM) mode
*/
void BXDM_PPDBG_P_PrintSelectionModeOverride_isr(
   char *pMsg,
   const BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PictureProvider_P_Picture_Context *pPicture)
{
   BXDM_PictureProvider_P_State_Decode *pDecode = &hXdmPP->stDMState.stDecode;
   BXDM_PPDBG_P_Info *pDebug = &pDecode->stDebug;

   if ((BXDM_PictureProvider_DisplayMode_eVirtualTSM != pPicture->stPicParms.stTSM.stDynamic.eSelectionMode)
     && (BXDM_PictureProvider_DisplayMode_eVirtualTSM != pDecode->eLastSelectionModeOverride))
   {
      BXVD_DBG_WRN(hXdmPP, (" %x:[%02x.%03x] %s: Selection Mode Override (TSM -> vTSM)",
                              pDebug->uiVsyncCount,
                              BXDM_PPDBG_FORMAT_INSTANCE_ID( hXdmPP ),
                              pPicture->stPicParms.uiPPBIndex & 0xFFF,
                              pMsg));

      /* prevent repeated override messages */
      pDecode->eLastSelectionModeOverride = BXDM_PictureProvider_DisplayMode_eVirtualTSM;
   }
}

/*
** SW7335-781: Output warning when forcing picture selection override
** Indicate when the override transitions back to TSM mode
*/
void BXDM_PPDBG_P_PrintEndSelectionModeOverride_isr(
   const BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PictureProvider_P_Picture_Context *pPicture)
{
   BXDM_PictureProvider_P_State_Decode *pDecode = &hXdmPP->stDMState.stDecode;
   BXDM_PPDBG_P_Info *pDebug = &pDecode->stDebug;

   /* if selected picture's selection mode was TSM and last selection mode override was VSYNC
      then output the selection mode change message */
   if ((BXDM_PictureProvider_DisplayMode_eTSM == pPicture->stPicParms.stTSM.stDynamic.eSelectionMode)
      && (BXDM_PictureProvider_DisplayMode_eVirtualTSM == pDecode->eLastSelectionModeOverride))
   {
      BXVD_DBG_WRN(hXdmPP, (" %x:[%02x.%03x] Selection Mode Override: Return to TSM",
                                 pDebug->uiVsyncCount,
                                 BXDM_PPDBG_FORMAT_INSTANCE_ID( hXdmPP ),
                                 pPicture->stPicParms.uiPPBIndex & 0xFFF
                                 ));

      pDecode->eLastSelectionModeOverride = BXDM_PictureProvider_DisplayMode_eTSM;
   }
}

/*
 * SW7405-4736: conditionally print the MFD structure.
 */
void BXDM_PPDBG_P_PrintMFD_isr(
   BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PictureProvider_P_LocalState* pLocalState,
   BAVC_MFD_Picture* pMFDPicture
   )
{
   BAVC_MFD_Picture * pMFD;
   BXDM_PPDBG_P_Info * pDebug;
   BXDM_PictureProvider_P_Picture_Context * pstSelectedPicture;
   int32_t  i=0;
   char cBarDataType;

   BDBG_ENTER( BXDM_PPDBG_P_PrintMFD_isr );

   BDBG_ASSERT( hXdmPP );
   BDBG_ASSERT( pLocalState );
   BDBG_ASSERT( pMFDPicture );

   pMFD = pMFDPicture;
   pDebug = &(hXdmPP->stDMState.stDecode.stDebug);
   pstSelectedPicture = &(hXdmPP->stDMState.stChannel.stSelectedPicture);


   while ( NULL != pMFD )
   {
      BFMT_AspectRatio eAspectRatio=BFMT_AspectRatio_eUnknown;

      /* Range check */

      if ( pMFD->eAspectRatio <= BFMT_AspectRatio_eSAR )
      {
         eAspectRatio = pMFD->eAspectRatio;
      }

      BDBG_MODULE_MSG( BXDM_MFD1, ("%c%x:[%02x.%03x] id:%03x pts:%08x %s->%s %dx%d->%dx%d %s:%dx%d AQP:%02d fr:%s mr:%s %s%s%s%s%s%s%s%s%s%s%s",
                                 ( pMFD->bMute ) ? 'M' : ' ',
                                 pDebug->uiVsyncCount,
                                 BXDM_PPDBG_FORMAT_INSTANCE_ID( hXdmPP ),
                                 pstSelectedPicture->stPicParms.uiPPBIndex & 0xFFF,
                                 pMFD->ulDecodePictureId,
                                 pMFD->ulOrigPTS,
                                 s_aPolarityToStrLUT[ pMFD->eSourcePolarity ],
                                 s_aPolarityToStrLUT[ pMFD->eInterruptPolarity ],
                                 pMFD->ulSourceHorizontalSize,
                                 pMFD->ulSourceVerticalSize,
                                 pMFD->ulDisplayHorizontalSize,
                                 pMFD->ulDisplayVerticalSize,
                                 s_aAspectRatioToStrLUT[ eAspectRatio ],
                                 pMFD->uiSampleAspectRatioX,
                                 pMFD->uiSampleAspectRatioY,
                                 pMFD->ulAdjQp,
                                 s_aBAVCFrameRateToStrLUT[ pMFD->eFrameRateCode ],
                                 s_aBFMTRefreshRateToStrLUT[ pMFD->eInterruptRefreshRate],
                                 s_aOrientationToStrLUT[ pMFD->eOrientation ],
                                 ( pMFD->bPictureRepeatFlag ) ? " rp" : " ",
                                 ( pMFD->bRepeatField ) ? " rf" : " ",
                                 ( pMFD->bIgnoreCadenceMatch ) ? " ic" : " ",
                                 ( pMFD->bIgnorePicture ) ? " ip" : " ",
                                 ( pMFD->bStallStc ) ? " ss" : " ",
                                 ( pMFD->bLast ) ? " lst" : " ",
                                 ( pMFD->bChannelChange) ? " chg" : " ",
                                 ( pMFD->bPreChargePicture) ? " pcp" : " ",
                                 ( pMFD->bEndOfChunk) ? " eoc" : " ",
                                 ( 0 != i ) ? " en" : " "
                                 ));

      BDBG_MODULE_MSG( BXDM_MFD3, ("%c%x:[%02x.%03x] %s top::lb:0x%lu lo:%08x cb:0x%lu co:%08x bot::lb:0x%lu lo:%08x cb:0x%lu co:%08x",
                                 ( pMFD->bMute ) ? 'M' : ' ',
                                 pDebug->uiVsyncCount,
                                 BXDM_PPDBG_FORMAT_INSTANCE_ID( hXdmPP ),
                                 pstSelectedPicture->stPicParms.uiPPBIndex & 0xFFF,
                                 ( pMFD->eBufferFormat ) ? "fp" : "Fr" ,
                                 (unsigned long)pMFD->hLuminanceFrameBufferBlock,
                                 pMFD->ulLuminanceFrameBufferBlockOffset,
                                 (unsigned long)pMFD->hChrominanceFrameBufferBlock,
                                 pMFD->ulChrominanceFrameBufferBlockOffset,
                                 (unsigned long)pMFD->hLuminanceBotFieldBufferBlock,
                                 pMFD->ulLuminanceBotFieldBufferBlockOffset,
                                 (unsigned long)pMFD->hChrominanceBotFieldBufferBlock,
                                 pMFD->ulChrominanceBotFieldBufferBlockOffset
                          ));

      pMFD = pMFD->pEnhanced;
      i++;
   }

   pMFD = pMFDPicture; /* reset to the orignal value. */

   /* SW7405-4736: Ideally the following would be printed out once per second or when any of the values
    * change, this would save extraneous prints to the console.  The values could be tracked by keeping
    * a local version of the MFD structure, but that would involve copying the data an extra time.
    * The memory pointed to by pMFD is allocated by XDM, i.e. "hXdmPP->astMFDPicture"; essentially XDM
    * already has a local copy.  However the "astMFDPicture" elements are set throughout
    * "BXDM_PPOUT_P_CalculateVdcData_isr".  The code to check for a change in value would have to be added
    * in multiple locations, a task for another day.
    */

   switch( pMFD->eBarDataType )
   {
      case BAVC_BarDataType_eTopBottom:   cBarDataType = 'T';     break;
      case BAVC_BarDataType_eLeftRight:   cBarDataType = 'L';     break;
      default:                            cBarDataType = 'i';     break;
   }

   BDBG_MODULE_MSG( BXDM_MFD2, ("%c%x:[%02x.%03x] clp:%dx%d afd:%d(%d) bar:%dx%d(%c) pan:%dx%d loc:%d ci:%s cp:%d tc:%d mc:%d %s dp:%s chk:%08x %s%s",
                                 ( pMFD->bMute ) ? 'M' : ' ',
                                 pDebug->uiVsyncCount,
                                 BXDM_PPDBG_FORMAT_INSTANCE_ID( hXdmPP ),
                                 pstSelectedPicture->stPicParms.uiPPBIndex & 0xFFF,
                                 pMFD->ulSourceClipLeft,
                                 pMFD->ulSourceClipTop,
                                 pMFD->ulAfd,
                                 pMFD->bValidAfd,
                                 pMFD->ulTopLeftBarValue,
                                 pMFD->ulBotRightBarValue,
                                 cBarDataType,
                                 pMFD->i32_HorizontalPanScan,
                                 pMFD->i32_VerticalPanScan,
                                 pMFD->eMpegType,
                                 ( pMFD->eChrominanceInterpolationMode ) ? "Fr" : "Fld",
                                 pMFD->eColorPrimaries,
                                 pMFD->eTransferCharacteristics,
                                 pMFD->eMatrixCoefficients,
                                 s_aBAVCPictureCodingToStrLUT[ pMFD->ePictureType ],
                                 ( BAVC_VideoBitDepth_e10Bit == pMFDPicture->eBitDepth ) ? "10" : "8",
                                 pMFD->ulChunkId,
                                 ( pMFD->bFrameProgressive ) ? " fp" : " ",
                                 ( pMFD->bStreamProgressive ) ? " sp" : " "
                                 ));


   BDBG_LEAVE( BXDM_PPDBG_P_PrintMFD_isr );

   return;

}  /* end of BXDM_PPDBG_P_PrintMFD */

void BXDM_PPDBG_P_PrintUnifiedPicture_isr(
   BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PictureProvider_P_LocalState * pLocalState,
   BXDM_PictureProvider_P_Picture_Context * pstPicture
   )
{
   BXDM_PictureProvider_P_Picture_Context *  pstSelectedPicture;
   const BXDM_Picture * pstUnified;
   const BXDM_Picture * pstSelectedUnified;

   bool  bPrintAdditional;
   BXDM_Picture_Coding eCoding;
   BXDM_Picture_PullDown ePulldown;
   char  cSourceFormat, cProgressiveSequence, cSelectionMode, cErrorOnThisPicture;

   BDBG_ENTER( BXDM_PPDBG_P_PrintUnifiedPicture_isr );

   BSTD_UNUSED(pLocalState);

   pstSelectedPicture = &(hXdmPP->stDMState.stChannel.stSelectedPicture);
   pstSelectedUnified = pstSelectedPicture->pstUnifiedPicture;

   pstUnified = pstPicture->pstUnifiedPicture;

   /* SW7405-4736: now that this routine is called from BXDM_PP_S_SelectPicture, there is a
    * need to verify that the unified picture is still available.  In the scenario where a
    * picture is being redisplayed while subsequent ones are dropped, "pstSelectedUnified"
    * will be valid and "pstUnified" can be NULL.  This can occur when playing a DVD clip
    * where video data continues after the clip stop time.  The picture just prior to the
    * clip stop time will be repeated, the pictures after the clip stop time will be dropped.
    */
   if ( NULL == pstUnified )
   {
      goto Done;
   }

   /* Print once per second or when any of the parameters change. */

   bPrintAdditional = ( hXdmPP->stDMState.stDecode.uiVsyncCountQM >= BXDM_PPTMR_lutVsyncsPersSecond[ hXdmPP->stDMConfig.eMonitorRefreshRate ] );

   if ( pstSelectedUnified && true == pstSelectedPicture->bValidated )
   {
      bPrintAdditional |= ( pstUnified->stBufferInfo.stSource.uiWidth != pstSelectedUnified->stBufferInfo.stSource.uiWidth ) ;
      bPrintAdditional |= ( pstUnified->stBufferInfo.stSource.uiHeight != pstSelectedUnified->stBufferInfo.stSource.uiHeight ) ;
      bPrintAdditional |= ( pstUnified->stProtocol.eProtocol != pstSelectedUnified->stProtocol.eProtocol ) ;
      bPrintAdditional |= ( pstUnified->stBufferInfo.eBufferFormat != pstSelectedUnified->stBufferInfo.eBufferFormat ) ;
      bPrintAdditional |= ( pstUnified->stFrameRate.stRate.uiNumerator != pstSelectedUnified->stFrameRate.stRate.uiNumerator ) ;
      bPrintAdditional |= ( pstUnified->stFrameRate.stRate.uiDenominator != pstSelectedUnified->stFrameRate.stRate.uiDenominator ) ;
      bPrintAdditional |= ( pstPicture->stPicParms.stTSM.stStatic.eFrameRateXVD != pstSelectedPicture->stPicParms.stTSM.stStatic.eFrameRateXVD ) ;
      bPrintAdditional |= ( pstUnified->stAspectRatio.eAspectRatio != pstSelectedUnified->stAspectRatio.eAspectRatio ) ;
      bPrintAdditional |= ( pstUnified->stAspectRatio.uiSampleAspectRatioX != pstSelectedUnified->stAspectRatio.uiSampleAspectRatioX ) ;
      bPrintAdditional |= ( pstUnified->stAspectRatio.uiSampleAspectRatioY != pstSelectedUnified->stAspectRatio.uiSampleAspectRatioY ) ;
      bPrintAdditional |= ( hXdmPP->stDMStatus.stCurrentPTS.uiSwPcrOffset != hXdmPP->stDMConfig.uiSoftwarePCROffset );
      bPrintAdditional |= ( hXdmPP->stDMConfig.uiDirtyBits_1 & BXDM_PictureProvider_P_DIRTY_1_DISPLAY_FIELD_MODE );
   }
   else
   {
      /* Results in printing the parameters for the first picture after start decode. */
      bPrintAdditional = true;
   }

   if ( true == bPrintAdditional )
   {
      bool bProtocolValid;
      BAVC_FrameRateCode eFrameRate = BAVC_FrameRateCode_eUnknown;
      BFMT_AspectRatio eAspectRatio = BFMT_AspectRatio_eUnknown;
      BXDM_PPFP_P_DataType stDeltaPTSAvg;
      uint32_t uiAverageFractionBase10;

      if ( 0 !=  hXdmPP->stDMState.stDecode.stFRDStats.uiDeltaPTSCount )
      {
         /* determine Average dPTS (using FRD parameters) ... */
         BXDM_PPFP_P_FixPtDiv_isr(
               &hXdmPP->stDMState.stDecode.stFRDStats.stDeltaPTSRunningSum,
               hXdmPP->stDMState.stDecode.stFRDStats.uiDeltaPTSCount,
               &stDeltaPTSAvg
               );

         /* Convert the fractional part of stDeltaPTSAvg to a base 10 value for display. */
         BXDM_PPFP_P_FixPtBinaryFractionToBase10_isr( &stDeltaPTSAvg, 2, &uiAverageFractionBase10 );
      }
      else
      {
         stDeltaPTSAvg.uiFractional = 0;
         stDeltaPTSAvg.uiWhole = 0;
         uiAverageFractionBase10 = 0;
      }

      /* Range check the variables used to index lookup tables. */

      bProtocolValid = ( pstUnified->stProtocol.eProtocol < BXDM_PPDBG_S_MAX_VIDEO_PROTOCOL ) ? true : false ;

      if ( pstPicture->stPicParms.stTSM.stStatic.eFrameRateXVD < BXDM_PictureProvider_P_MAX_FRAMERATE )
      {
         eFrameRate = pstPicture->stPicParms.stTSM.stStatic.eFrameRateXVD;
      }

      if (  pstUnified->stAspectRatio.eAspectRatio <= BFMT_AspectRatio_eSAR )
      {
         eAspectRatio =  pstUnified->stAspectRatio.eAspectRatio;
      }

      hXdmPP->stDMState.stDecode.uiVsyncCountQM = 0;

      BDBG_MODULE_MSG( BXDM_PPQM, (" %x:[%02x.%03x] %4dx%3d %s%scfr:%d/%d afr:%sHz(%s),ar:%s(%d,%d)(%d) swOff:%x avPts:%d.%u %s%s",
                             hXdmPP->stDMState.stDecode.stDebug.uiVsyncCount,
                             BXDM_PPDBG_FORMAT_INSTANCE_ID( hXdmPP ),
                             pstPicture->stPicParms.uiPPBIndex & 0xFFF,
                             pstUnified->stBufferInfo.stSource.uiWidth,
                             pstUnified->stBufferInfo.stSource.uiHeight,
                             ( bProtocolValid ) ? s_aVideoCompressionStdToStrLUT[ pstUnified->stProtocol.eProtocol ] : "ukn" ,
                             ( pstUnified->stBufferInfo.eBufferFormat == BXDM_Picture_BufferFormat_eSplitInterlaced ) ?  "-si," : "," ,
                             pstUnified->stFrameRate.stRate.uiNumerator,
                             pstUnified->stFrameRate.stRate.uiDenominator,
                             s_aBAVCFrameRateToStrLUT[ eFrameRate ],
                             s_aFrameRateTypeToStrLUT[ pstPicture->stPicParms.stTSM.stStatic.eFrameRateType ],
                             s_aAspectRatioToStrLUT[ eAspectRatio ],
                             pstUnified->stAspectRatio.uiSampleAspectRatioX,
                             pstUnified->stAspectRatio.uiSampleAspectRatioY,
                             pstUnified->stAspectRatio.bValid,
                             pstPicture->stPicParms.stTSM.stDynamic.uiSwPcrOffsetUsedForEvaluation,
                             stDeltaPTSAvg.uiWhole,
                             uiAverageFractionBase10,
                             s_aDisplayFieldModeToStrLUT[ pstPicture->stPicParms.stDisplay.stDynamic.eDisplayFieldMode ],
                             ( pstPicture->stPicParms.stDisplay.stDynamic.bForceSingleFieldMode ) ? "-f" :
                                 ( BXDM_PictureProvider_DisplayFieldMode_eAuto == hXdmPP->stDMConfig.eDisplayFieldMode ) ? "-a" : ""
                             ));
   }


   /* The following values from the Unified Picture are set outside of XDM.  Range check the values to avoid
    * stepping off the end of lookup tables, i.e. "s_aXXXToStrLUT[]"
    */
   eCoding = ( pstUnified->stPictureType.eCoding < BXDM_Picture_Coding_eMax ) ? pstUnified->stPictureType.eCoding : BXDM_Picture_Coding_eUnknown ;

   /* SW7445-586: use accessor method to retrieve pulldown */
   BXDM_PPQM_P_GetPicturePulldown_isr( pstPicture, &ePulldown );

   if ( ePulldown >= BXDM_Picture_PullDown_eMax ) ePulldown = 0;

   switch ( pstUnified->stBufferInfo.eSourceFormat )
   {
      case BXDM_Picture_SourceFormat_eInterlaced:     cSourceFormat = 'I';    break;
      case BXDM_Picture_SourceFormat_eProgressive:    cSourceFormat = 'P';    break;
      case BXDM_Picture_SourceFormat_eUnknown:
      default:                                        cSourceFormat = 'U';    break;
   }


   switch ( pstUnified->stPictureType.eSequence )
   {
      case BXDM_Picture_Sequence_eInterlaced:   cProgressiveSequence = 'I';    break;
      case BXDM_Picture_Sequence_eProgressive:  cProgressiveSequence = 'P';    break;
      case BXDM_Picture_Sequence_eUnknown:
      default:                                  cProgressiveSequence = 'U';    break;
   }

   switch ( pstPicture->stPicParms.stTSM.stDynamic.ePictureHandlingMode )
   {
      case BXDM_PictureProvider_PictureHandlingMode_eHonorPTS:    cSelectionMode = 'h';   break;
      case BXDM_PictureProvider_PictureHandlingMode_eIgnorePTS:   cSelectionMode = 'i';   break;
      case BXDM_PictureProvider_PictureHandlingMode_eDrop:        cSelectionMode = 'd';   break;
      case BXDM_PictureProvider_PictureHandlingMode_eWait:
         cSelectionMode = (BXDM_PictureProvider_DisplayMode_eTSM == pstPicture->stPicParms.stTSM.stDynamic.eSelectionMode) ? 'w' : 'v' ;
         break;

      case BXDM_PictureProvider_PictureHandlingMode_eDefault:
      default:
         if ( BXDM_PictureProvider_DisplayMode_eTSM == pstPicture->stPicParms.stTSM.stDynamic.eSelectionMode )
         {
            cSelectionMode = ( pstPicture->stPicParms.stTSM.stDynamic.bEvaluatedWithSwStc ) ? 's' : 't';
         }
         else
         {
            cSelectionMode = ( pstPicture->stPicParms.stTSM.stDynamic.bSelectedInPrerollMode ) ? 'p' : 'v' ;
         }
         break;
   }

   /* A picture-less picture, it will simply be dropped. */
   if ( false == pstPicture->pstUnifiedPicture->stBufferInfo.bValid )
   {
      cSelectionMode = 'x';
   }

   /* SWSTB-439: if the error threshold is non-zero, use "uiPercentError" from the picture structure
    * to determine if the picture has an error.  Otherwise use the error flag. */

   if ( 0 != hXdmPP->stDMConfig.uiErrorThreshold )
   {
      cErrorOnThisPicture = ( pstUnified->stError.uiPercentError == 0 ) ? ' ' :
                                    ( pstUnified->stError.uiPercentError >= hXdmPP->stDMConfig.uiErrorThreshold ) ? 'e' : 'p';
   }
   else
   {
      cErrorOnThisPicture = ( pstUnified->stError.bThisPicture == true )  ? 'E' : ' ';
   }


   if ( BXDM_PictureProvider_DisplayMode_eTSM == pstPicture->stPicParms.stTSM.stDynamic.eSelectionMode )
   {
      uint32_t uiPcrOffset;
      uint32_t uiDisplayOffset;
      bool     bPcrOffsetValid;
      bool     bPcrOffsetDiscontinuity;

      BXDM_PPQM_P_GetHwPcrOffset_isr( hXdmPP, pstPicture, &uiPcrOffset );

      uiPcrOffset += pstPicture->stPicParms.stTSM.stDynamic.uiSwPcrOffsetUsedForEvaluation;

      BXDM_PPQM_P_GetPtsOffset_isr( hXdmPP, pstPicture, &uiDisplayOffset );

      if (( false == hXdmPP->stDMConfig.bUseHardwarePCROffset )
            || ( true == hXdmPP->stDMConfig.bPlayback))
      {
         bPcrOffsetValid = true;
         bPcrOffsetDiscontinuity = false;
      }
      else
      {
         bPcrOffsetValid = pstPicture->stPicParms.stTSM.stStatic.stPCROffsetXDM.bValidOverloaded;
         bPcrOffsetDiscontinuity = pstPicture->stPicParms.stTSM.stStatic.stPCROffsetXDM.bDiscontinuityOverloaded;
      }

      BDBG_MODULE_MSG( BXDM_PPQM, (" %x:[%02x.%03x]%c%c%c%s,%s,fs:%c%c %s,pts:%08x(%d),off:%04x,j:%3d,pcr:%08x(%d,%d),stc:%08x%c,d:%08x",
                             hXdmPP->stDMState.stDecode.stDebug.uiVsyncCount,
                             BXDM_PPDBG_FORMAT_INSTANCE_ID( hXdmPP ),
                             pstPicture->stPicParms.uiPPBIndex & 0xFFF,
                             cErrorOnThisPicture,
                             cSelectionMode,
                             ( true == pstPicture->stPicParms.stDisplay.stDynamic.bAppendedToPreviousPicture ) ? '^' : ':',
                             s_aTSMResultToStrLUT[ pstPicture->stPicParms.stTSM.stDynamic.eTsmResult],
                             ( BXDM_Picture_BufferFormat_eSplitInterlaced == pstUnified->stBufferInfo.eBufferFormat ) ?
                                ( BXDM_Picture_BufferHandlingMode_eSiRepeat == pstUnified->stBufferInfo.eBufferHandlingMode ) ?
                                   s_aSiRepeatPullDownEnumToStrLUT[ ePulldown ] : s_aSiPullDownEnumToStrLUT[ ePulldown ] :
                                 s_aPullDownEnumToStrLUT[ ePulldown ],
                             cSourceFormat,
                             cProgressiveSequence,
                             s_aBXDMPictureCodingToStrLUT[ eCoding ],
                             pstUnified->stPTS.uiValue,
                             pstUnified->stPTS.bValid,
                             uiDisplayOffset,
                             pstPicture->stPicParms.stTSM.stStatic.iPTSJitterCorrection,
                             uiPcrOffset,
                             bPcrOffsetValid,
                             bPcrOffsetDiscontinuity,
                             pLocalState->uiStcSnapshot,
                             (hXdmPP->stDMState.stDecode.iStcJitterCorrectionOffset != 0)?'*':' ',
                             pstPicture->stPicParms.stTSM.stDynamic.iStcPtsDifferenceActual
                             ));
   }
   else
   {
      uint32_t    uiVirtualStc;
      uint32_t    uiDisplayOffset;

      BXDM_PPFP_P_DataType stVirtualPts;

      BXDM_PPQM_P_GetPtsOffset_isr( hXdmPP, pstPicture, &uiDisplayOffset );

      BXDM_PPVTSM_P_VirtualStcGet_isr( hXdmPP, &uiVirtualStc );

      uiVirtualStc += pstPicture->stPicParms.stTSM.stDynamic.uiSwPcrOffsetUsedForEvaluation;

      BXDM_PPQM_P_GetPtsWithFrac_isr(
            pstPicture,
            BXDM_PictureProvider_P_PTSIndex_eVirtual,
            pstPicture->stPicParms.stDisplay.stDynamic.uiSelectedElement,
            &stVirtualPts
            );

      BDBG_MODULE_MSG( BXDM_PPQM, (" %x:[%02x.%03x]%c%c%c%s,%s,fs:%c%c %s,pts:%08x(%d),off:%04x,j:   ,vpts:%08x   ,vstc:%08x ,d:%08x",
                             hXdmPP->stDMState.stDecode.stDebug.uiVsyncCount,
                             BXDM_PPDBG_FORMAT_INSTANCE_ID( hXdmPP ),
                             pstPicture->stPicParms.uiPPBIndex & 0xFFF,
                             cErrorOnThisPicture,
                             cSelectionMode,
                             ( true == pstPicture->stPicParms.stDisplay.stDynamic.bAppendedToPreviousPicture ) ? '^' : ':',
                             s_aTSMResultToStrLUT[ pstPicture->stPicParms.stTSM.stDynamic.eTsmResult],
                             ( BXDM_Picture_BufferFormat_eSplitInterlaced == pstUnified->stBufferInfo.eBufferFormat ) ?
                                ( BXDM_Picture_BufferHandlingMode_eSiRepeat == pstUnified->stBufferInfo.eBufferHandlingMode ) ?
                                   s_aSiRepeatPullDownEnumToStrLUT[ ePulldown ] : s_aSiPullDownEnumToStrLUT[ ePulldown ] :
                                 s_aPullDownEnumToStrLUT[ ePulldown ],
                             cSourceFormat,
                             cProgressiveSequence,
                             s_aBXDMPictureCodingToStrLUT[ eCoding ],
                             pstUnified->stPTS.uiValue,
                             pstUnified->stPTS.bValid,
                             uiDisplayOffset,
                             stVirtualPts.uiWhole,
                             uiVirtualStc,
                             pstPicture->stPicParms.stTSM.stDynamic.iStcPtsDifferenceActual
                             ));

   }

Done:

   BDBG_LEAVE( BXDM_PPDBG_P_PrintUnifiedPicture_isr );

   return;

}  /* end of BXDM_PPDBG_P_PrintUnifiedPicture */

void BXDM_PPDBG_P_PrintDMConfig_isr(
   BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PictureProvider_P_LocalState * pLocalState,
   bool bLastCall
   )
{
   BDBG_ENTER( BXDM_PPDBG_P_PrintDMConfig_isr );

   BSTD_UNUSED(pLocalState);

   /* Print when any of the parameters change. */

   if (  hXdmPP->stDMConfig.uiDirtyBits_1 || hXdmPP->stDMConfig.uiDirtyBits_2 )
   {
      uint32_t uiDirtyBitsGroup_1_1, uiDirtyBitsGroup_1_2, uiDirtyBitsGroup_1_3, uiDirtyBitsGroup_1_4;
      uint32_t uiDirtyBitsGroup_2_1, uiDirtyBitsGroup_2_2, uiDirtyBitsGroup_2_3, uiDirtyBitsGroup_2_4 ;

      uiDirtyBitsGroup_1_1 = uiDirtyBitsGroup_1_2 = uiDirtyBitsGroup_1_3 = uiDirtyBitsGroup_1_4 = hXdmPP->stDMConfig.uiDirtyBits_1;

      uiDirtyBitsGroup_2_1 = uiDirtyBitsGroup_2_2 = uiDirtyBitsGroup_2_3 = uiDirtyBitsGroup_2_4 = hXdmPP->stDMConfig.uiDirtyBits_2;

      uiDirtyBitsGroup_1_1 &= BXDM_PictureProvider_P_DIRTY_1_STC_VALID
                              | BXDM_PictureProvider_P_DIRTY_1_PTS_OFFSET
                              | BXDM_PictureProvider_P_DIRTY_1_SOFTWARE_PCR_OFFSET
                              | BXDM_PictureProvider_P_DIRTY_1_USE_HW_PCR_OFFSET
                              | BXDM_PictureProvider_P_DIRTY_1_PLAYBACK_RATE
                              | BXDM_PictureProvider_P_DIRTY_1_DISPLAY_FIELD_MODE;

      uiDirtyBitsGroup_2_1 &= BXDM_PictureProvider_P_DIRTY_2_MUTE;

      if ( uiDirtyBitsGroup_1_1 || uiDirtyBitsGroup_2_1 )
      {
         BDBG_MODULE_MSG( BXDM_CFG, ("%c%x:[%02x.xxx]1:%c stc:%c(%d) PTSoff:%08x(%d) STCoff:%08x(%d) usePCR:%c(%d) pbr:%d/%d(%d) dfm:%s(%d)",
                  ( bLastCall ) ? '*' : ' ',
                  hXdmPP->stDMState.stDecode.stDebug.uiVsyncCount,
                  BXDM_PPDBG_FORMAT_INSTANCE_ID( hXdmPP ),
                  ( hXdmPP->stDMConfig.bMute == true ) ? 'M' : ' ',
/*                  ( hXdmPP->stDMConfig.uiDirtyBits_2 & BXDM_PictureProvider_P_DIRTY_2_MUTE ) ? 1 : 0,*/
                  ( hXdmPP->stDMConfig.bSTCValid == true ) ? 'v' : 'i',
                  ( hXdmPP->stDMConfig.uiDirtyBits_1 & BXDM_PictureProvider_P_DIRTY_1_STC_VALID ) ? 1 : 0,
                  hXdmPP->stDMConfig.uiPTSOffset,
                  ( hXdmPP->stDMConfig.uiDirtyBits_1 & BXDM_PictureProvider_P_DIRTY_1_PTS_OFFSET ) ? 1 : 0,
                  hXdmPP->stDMConfig.uiSoftwarePCROffset,
                  ( hXdmPP->stDMConfig.uiDirtyBits_1 & BXDM_PictureProvider_P_DIRTY_1_SOFTWARE_PCR_OFFSET ) ? 1 : 0,
                  ( hXdmPP->stDMConfig.bUseHardwarePCROffset == true) ? 't' : 'f',
                  ( hXdmPP->stDMConfig.uiDirtyBits_1 & BXDM_PictureProvider_P_DIRTY_1_USE_HW_PCR_OFFSET ) ? 1 : 0,
                  hXdmPP->stDMConfig.stPlaybackRate.uiNumerator,
                  hXdmPP->stDMConfig.stPlaybackRate.uiDenominator,
                  ( hXdmPP->stDMConfig.uiDirtyBits_1 & BXDM_PictureProvider_P_DIRTY_1_PLAYBACK_RATE ) ? 1 : 0,
                  s_aDisplayFieldModeToStrLUT[ hXdmPP->stDMConfig.eDisplayFieldMode ],
                  ( hXdmPP->stDMConfig.uiDirtyBits_1 & BXDM_PictureProvider_P_DIRTY_1_DISPLAY_FIELD_MODE ) ? 1 : 0
                  ));

      }     /* end of  if ( uiDirtyBitsGroup_1_1 || uiDirtyBitsGroup_2_1 ) */


      uiDirtyBitsGroup_1_2 &= BXDM_PictureProvider_P_DIRTY_1_PLAYBACK
                              | BXDM_PictureProvider_P_DIRTY_1_PROTOCOL
                              | BXDM_PictureProvider_P_DIRTY_1_MONITOR_REFRESH_RATE
                              | BXDM_PictureProvider_P_DIRTY_1_DEFAULT_FRAME_RATE
                              | BXDM_PictureProvider_P_DIRTY_1_FRAMERATE_OVERRIDE
                              | BXDM_PictureProvider_P_DIRTY_1_TRICK_MODE
                              | BXDM_PictureProvider_P_DIRTY_1_FREEZE ;

      uiDirtyBitsGroup_2_2 &= BXDM_PictureProvider_P_DIRTY_2_DISPLAY_MODE;

      if ( uiDirtyBitsGroup_1_2 || uiDirtyBitsGroup_2_2 )
      {

         BDBG_MODULE_MSG( BXDM_CFG, ("%c%x:[%02x.xxx]2:%s(%d) %s(%d) %s(%d) mr:%s(%d) dfr:%s(%d) fro:%d/%d(%d,%d)(%d) tm:%s(%d) frz:%c(%d)",
                  ( bLastCall ) ? '*' : ' ',
                  hXdmPP->stDMState.stDecode.stDebug.uiVsyncCount,
                  BXDM_PPDBG_FORMAT_INSTANCE_ID( hXdmPP ),
                  ( hXdmPP->stDMConfig.eDisplayMode == BXDM_PictureProvider_DisplayMode_eTSM ) ? "TSM" : "vsync",
                  ( hXdmPP->stDMConfig.uiDirtyBits_2 & BXDM_PictureProvider_P_DIRTY_2_DISPLAY_MODE ) ? 1 : 0,
                  ( hXdmPP->stDMConfig.bPlayback == true ) ? "pb" : "lv",
                  ( hXdmPP->stDMConfig.uiDirtyBits_1 & BXDM_PictureProvider_P_DIRTY_1_PLAYBACK ) ? 1 : 0,
                  s_aVideoCompressionStdToStrLUT[ hXdmPP->stDMConfig.eProtocol ],
                  ( hXdmPP->stDMConfig.uiDirtyBits_2 & BXDM_PictureProvider_P_DIRTY_1_PROTOCOL ) ? 1 : 0,
                  s_aMonitorRefreshRateToStrLUT[ hXdmPP->stDMConfig.eMonitorRefreshRate ],
                  ( hXdmPP->stDMConfig.uiDirtyBits_1 & BXDM_PictureProvider_P_DIRTY_1_MONITOR_REFRESH_RATE ) ? 1 : 0,
                  s_aBAVCFrameRateToStrLUT[ hXdmPP->stDMConfig.eDefaultFrameRate ],
                  ( hXdmPP->stDMConfig.uiDirtyBits_1 & BXDM_PictureProvider_P_DIRTY_1_DEFAULT_FRAME_RATE ) ? 1 : 0,
                  hXdmPP->stDMConfig.stFrameRateOverride.stRate.uiNumerator,
                  hXdmPP->stDMConfig.stFrameRateOverride.stRate.uiDenominator,
                  hXdmPP->stDMConfig.stFrameRateOverride.bValid,
                  hXdmPP->stDMConfig.stFrameRateOverride.bTreatAsSingleElement,
                  ( hXdmPP->stDMConfig.uiDirtyBits_1 & BXDM_PictureProvider_P_DIRTY_1_FRAMERATE_OVERRIDE ) ? 1 : 0,
                  s_aTrickModeToStrLUT[ hXdmPP->stDMConfig.eTrickMode ],
                  ( hXdmPP->stDMConfig.uiDirtyBits_1 & BXDM_PictureProvider_P_DIRTY_1_TRICK_MODE ) ? 1 : 0,
                  ( hXdmPP->stDMConfig.bFreeze == true ) ? 't' : 'f',
                  ( hXdmPP->stDMConfig.uiDirtyBits_2 & BXDM_PictureProvider_P_DIRTY_1_FREEZE ) ? 1 : 0
                  ));

      } /* end of if ( uiDirtyBitsGroup_1_2 || uiDirtyBitsGroup_2_2 ) */


      uiDirtyBitsGroup_1_3 &= BXDM_PictureProvider_P_DIRTY_1_SRC_FORMAT_OVERRIDE
                              | BXDM_PictureProvider_P_DIRTY_1_SCAN_MODE_OVERRIDE
                              | BXDM_PictureProvider_P_DIRTY_1_1080P_SCAN_MODE
                              | BXDM_PictureProvider_P_DIRTY_1_PICTURE_DROP_MODE
                              | BXDM_PictureProvider_P_DIRTY_1_480P_PULLDOWN_MODE
                              | BXDM_PictureProvider_P_DIRTY_1_1080P_PULLDOWN_MODE
                              | BXDM_PictureProvider_P_DIRTY_1_240I_SCAN_MODE
                              | BXDM_PictureProvider_P_DIRTY_1_HORIZON_OVERSCAN_MODE;


      if ( uiDirtyBitsGroup_1_3 )
      {
         char cSFO, cSMO;

         switch( hXdmPP->stDMConfig.eSourceFormatOverride )
         {
            case BXDM_PictureProvider_SourceFormatOverride_eInterlaced:    cSFO = 'P';    break;
            case BXDM_PictureProvider_SourceFormatOverride_eProgressive:   cSFO = 'I';    break;
            case BXDM_PictureProvider_SourceFormatOverride_eDefault:
            default:                                                       cSFO = 'D';    break;
         }

         switch( hXdmPP->stDMConfig.eScanModeOverride )
         {
            case BXDM_PictureProvider_ScanModeOverride_eInterlaced:     cSMO = 'P';    break;
            case BXDM_PictureProvider_ScanModeOverride_eProgressive:    cSMO = 'I';    break;
            case BXDM_PictureProvider_ScanModeOverride_eDefault:
            default:                                                    cSMO = 'D';    break;
         }

         BDBG_MODULE_MSG( BXDM_CFG, ("%c%x:[%02x.xxx]3:sfo:%c(%d) smo:%c(%d) 1080sm:%c(%d) pdm:%s(%d) 480pdm:%s(%d) 1080pdm:%s(%d) 240sm:%s(%d) hom:%s(%d)",
                  ( bLastCall ) ? '*' : ' ',
                  hXdmPP->stDMState.stDecode.stDebug.uiVsyncCount,
                  BXDM_PPDBG_FORMAT_INSTANCE_ID( hXdmPP ),
                  cSFO,
                  ( hXdmPP->stDMConfig.uiDirtyBits_1 & BXDM_PictureProvider_P_DIRTY_1_SRC_FORMAT_OVERRIDE ) ? 1 : 0,
                  cSMO,
                  ( hXdmPP->stDMConfig.uiDirtyBits_1 & BXDM_PictureProvider_P_DIRTY_1_SCAN_MODE_OVERRIDE ) ? 1 : 0,
                  ( hXdmPP->stDMConfig.e1080pScanMode == BXDM_PictureProvider_1080pScanMode_eDefault ) ? 'D' : 'A' ,
                  ( hXdmPP->stDMConfig.uiDirtyBits_1 & BXDM_PictureProvider_P_DIRTY_1_1080P_SCAN_MODE ) ? 1 : 0,
                  ( hXdmPP->stDMConfig.ePictureDropMode == BXDM_PictureProvider_PictureDropMode_eField ) ? "fld" : "Frm" ,
                  ( hXdmPP->stDMConfig.uiDirtyBits_1 & BXDM_PictureProvider_P_DIRTY_1_PICTURE_DROP_MODE ) ? 1 : 0,
                  s_aPulldownModeToStrLUT[ hXdmPP->stDMConfig.e480pPulldownMode ],
                  ( hXdmPP->stDMConfig.uiDirtyBits_1 & BXDM_PictureProvider_P_DIRTY_1_480P_PULLDOWN_MODE ) ? 1 : 0,
                  s_aPulldownModeToStrLUT[ hXdmPP->stDMConfig.e1080pPulldownMode ],
                  ( hXdmPP->stDMConfig.uiDirtyBits_1 & BXDM_PictureProvider_P_DIRTY_1_1080P_PULLDOWN_MODE ) ? 1 : 0,
                  ( hXdmPP->stDMConfig.e240iScanMode == BXDM_PictureProvider_240iScanMode_eForceProgressive ) ? "fp" : "en" ,
                  ( hXdmPP->stDMConfig.uiDirtyBits_1 & BXDM_PictureProvider_P_DIRTY_1_240I_SCAN_MODE ) ? 1 : 0,
                  ( hXdmPP->stDMConfig.eHorizontalOverscanMode == BXDM_PictureProvider_HorizontalOverscanMode_eAuto ) ? "auto" : "dis" ,
                  ( hXdmPP->stDMConfig.uiDirtyBits_1 & BXDM_PictureProvider_P_DIRTY_1_HORIZON_OVERSCAN_MODE ) ? 1 : 0
                  ));

      }     /* end of if ( uiDirtyBitsGroup_1_3 ) */


      uiDirtyBitsGroup_1_4 &= BXDM_PictureProvider_P_DIRTY_1_FRAME_ADVANCE_MODE
                              | BXDM_PictureProvider_P_DIRTY_1_REVERSE_FIELDS
                              | BXDM_PictureProvider_P_DIRTY_1_AUTO_VALIDATE_ON_PAUSE
                              | BXDM_PictureProvider_P_DIRTY_1_3D_SETTINGS
                              | BXDM_PictureProvider_P_DIRTY_1_JITTER_TOLERANCE
                              | BXDM_PictureProvider_P_DIRTY_1_FRAME_RATE_DETECTION_MODE
                              | BXDM_PictureProvider_P_DIRTY_1_ERROR_HANDLING_MODE
                              | BXDM_PictureProvider_P_DIRTY_1_ATSM_MODE
                              | BXDM_PictureProvider_P_DIRTY_1_VTSM_ON_PCR_DISCON ;


      uiDirtyBitsGroup_2_4 &= BXDM_PictureProvider_P_DIRTY_2_ERROR_THRESHOLD;

      if ( uiDirtyBitsGroup_1_4 || uiDirtyBitsGroup_2_4 )
      {
         BDBG_MODULE_MSG( BXDM_CFG, ("%c%x:[%02x.xxx]4:fra:%s(%d) rvf:%c(%d) avop:%c(%d) 3Do:%s(%d)(%d) jti:%s(%d) frd:%s(%d) ehm:%s(%d) el:%d(%d) astm:%c(%d) avsync:%c(%d) ",
                  ( bLastCall ) ? '*' : ' ',
                  hXdmPP->stDMState.stDecode.stDebug.uiVsyncCount,
                  BXDM_PPDBG_FORMAT_INSTANCE_ID( hXdmPP ),
                  s_aFrameAdvanceModeToStrLUT[hXdmPP->stDMConfig.eFrameAdvanceMode],
                  ( hXdmPP->stDMConfig.uiDirtyBits_1 & BXDM_PictureProvider_P_DIRTY_1_FRAME_ADVANCE_MODE ) ? 1 : 0,
                  ( hXdmPP->stDMConfig.bReverseFields == true ) ? 't' : 'f' ,
                  ( hXdmPP->stDMConfig.uiDirtyBits_1 & BXDM_PictureProvider_P_DIRTY_1_REVERSE_FIELDS ) ? 1 : 0,
                  ( hXdmPP->stDMConfig.bAutoValidateStcOnPause == true ) ? 't' : 'f' ,
                  ( hXdmPP->stDMConfig.uiDirtyBits_1 & BXDM_PictureProvider_P_DIRTY_1_AUTO_VALIDATE_ON_PAUSE ) ? 1 : 0,
                  s_aPPOrientationToStrLUT[ hXdmPP->stDMConfig.st3DSettings.eOrientation ],
                  hXdmPP->stDMConfig.st3DSettings.bOverrideOrientation,
                  ( hXdmPP->stDMConfig.uiDirtyBits_1 & BXDM_PictureProvider_P_DIRTY_1_3D_SETTINGS ) ? 1 : 0,
                  ( hXdmPP->stDMConfig.bJitterToleranceImprovement == true ) ? "on" : "off" ,
                  ( hXdmPP->stDMConfig.uiDirtyBits_1 & BXDM_PictureProvider_P_DIRTY_1_JITTER_TOLERANCE ) ? 1 : 0,
                  s_aFrameRateDetectionModeToStrLUT[ hXdmPP->stDMConfig.eFrameRateDetectionMode ],
                  ( hXdmPP->stDMConfig.uiDirtyBits_1 & BXDM_PictureProvider_P_DIRTY_1_FRAME_RATE_DETECTION_MODE ) ? 1 : 0,
                  s_aErrorHandlingModeToStrLUT[ hXdmPP->stDMConfig.eErrorHandlingMode ],
                  ( hXdmPP->stDMConfig.uiDirtyBits_1 & BXDM_PictureProvider_P_DIRTY_1_ERROR_HANDLING_MODE ) ? 1 : 0,
                  hXdmPP->stDMConfig.uiErrorThreshold,
                  ( hXdmPP->stDMConfig.uiDirtyBits_1 & BXDM_PictureProvider_P_DIRTY_2_ERROR_THRESHOLD ) ? 1 : 0,
                  ( hXdmPP->stDMConfig.bAstmMode == true ) ? 't' : 'f' ,
                  ( hXdmPP->stDMConfig.uiDirtyBits_1 & BXDM_PictureProvider_P_DIRTY_1_ATSM_MODE ) ? 1 : 0,
                  ( hXdmPP->stDMConfig.bVirtualTSMOnPCRDiscontinuity == true ) ? 't' : 'f' ,
                  ( hXdmPP->stDMConfig.uiDirtyBits_1 & BXDM_PictureProvider_P_DIRTY_1_VTSM_ON_PCR_DISCON ) ? 1 : 0
                  ));

      }     /* end of if ( uiDirtyBitsGroup_1_4 || uiDirtyBitsGroup_2_4 ) */

      /* The following are not currently printed.
       *
       * uint32_t uiRemovalDelay;
       * BXDM_Picture_Rate stPreRollRate;
       * uint32_t uiMaxHeightSupportedByDeinterlacer;
       * BXDM_PictureProvider_TSMThresholdSettings stTSMThresholdSettings;
       * uint32_t uiSTCIndex;
       * BXDM_PictureProvider_ClipTimeSettings stClipTimeSettings;
       */

      hXdmPP->stDMConfig.uiDirtyBits_1 = BXDM_PictureProvider_P_DIRTY_NONE;
      hXdmPP->stDMConfig.uiDirtyBits_2 = BXDM_PictureProvider_P_DIRTY_NONE;

   }       /* end of  if ( hXdmPP->stDMConfig.uiDirtyBits_1 || hXdmPP->stDMConfig.uiDirtyBits_2 ) */

   BDBG_LEAVE( BXDM_PPDBG_P_PrintDMConfig_isr );

   return;

}  /* end of BXDM_PPDBG_P_PrintDMConfig */


#endif
