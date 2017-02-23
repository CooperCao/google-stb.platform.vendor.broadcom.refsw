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
 * [File Description:]
 *
 ***************************************************************************/

#ifndef BXDM_PP_TYPES_H_
#define BXDM_PP_TYPES_H_

#include "btmr.h"
#include "bxdm.h"
#include "bxdm_dih.h"

#ifdef __cplusplus
extern "C" {
#endif

#if 0
}
#endif

typedef struct BXDM_PictureProvider_Settings
{
   uint32_t uiMaxBuffersPerPicture; /* This value indicates how many video buffers are
                                     * linked together in a single BXDM_Picture entry.
                                     * E.g. for MVC 3D, there are 2 video buffers per
                                     * BXDM_Picture entry */

   BMMA_Heap_Handle hHeap;          /* SWSTB-1380: when using the debug fifo, need to pass in a
                                     * heap handle when calling BXDM_PictureProvider_Create.  */
} BXDM_PictureProvider_Settings;


typedef struct BXDM_PictureProvider_ChannelChangeSettings
{
   bool bHoldLastPicture;
   bool bFirstPicturePreview;
} BXDM_PictureProvider_ChannelChangeSettings;


/***************************************************************************

Below is an explanation of the new eDisplayFieldMode mode
"BXDM_PictureProvider_DisplayFieldMode_eAuto" and the supporting API
"BXDM_PictureProvider_SetDeinterlacerMaxHeight".

For normal play and fast forward, eDisplayFieldMode should be set to "eBothField".
The question is how to set eDisplayFieldMode for slow motion and pause.
If there is an "appropriate" deinterlacer in the output path, it should be set to
"eBothField" for the best results.  If there is NOT an "appropriate" deinterlacer,
it should be set to "eSingleField"; this will minimize jitter at the expense of a lower
resolution image. The new eDisplayFieldMode of "BXDM_PictureProvider_DisplayFieldMode_eAuto" solves this
problem by automatically selecting either "eBothField" or "eSingleField" during slow
motion and pause.

Having an "appropriate" deinterlacer is defined as:
- every output is driven by a deinterlacer
- AND the least capable deinterlacer can support the stream height
- AND the deinterlacer(s) are enabled

The API "BXDM_PictureProvider_SetDeinterlacerMaxHeight" is used to set the
variable "uiMaxHeightSupportedByDeinterlacer".
The application/middleware will use "uiMaxHeightSupportedByDeinterlacer" to indicate if there
is an "appropriate" deinterlacer.  If "uiMaxHeightSupportedByDeinterlacer" is set to "0",
there is NOT an "appropriate" deinterlacer and the scan out will be "eSingleField".
If "uiMaxHeightSupportedByDeinterlacer" is set to a non-zero value, the scan out will be "eBothField" if
the stream height is less than or equal to "uiMaxHeightSupportedByDeinterlacer".
If the height is greater than "uiMaxHeightSupportedByDeinterlacer", the scan out will be "eSingleField".

The following pseudo code summaries the behavior:

   if ( stGlobal.eDisplayFieldMode == BXDM_PictureProvider_DisplayFieldMode_eAuto )
   {
      // Default to eBothField

      stLocal.eDisplayFieldMode = BXDM_PictureProvider_DisplayFieldMode_eBothField;

      // When paused or in slow motion, revert to eSingleField if the deinterlacer cannot support the stream.

      if ( stSource.uiHeight > uiMaxHeightSupportedByDeinterlacer
            && true == bPausedOrSlowMotion
         )
      {
         stLocal.eDisplayFieldMode = BXDM_PictureProvider_DisplayFieldMode_eSingleField;
      }

   }

****************************************************************************/


typedef enum BXDM_PictureProvider_DisplayFieldMode
{
   BXDM_PictureProvider_DisplayFieldMode_eBothField,       /* regular both field display */
   BXDM_PictureProvider_DisplayFieldMode_eTopFieldOnly,    /* Display Top fld only */
   BXDM_PictureProvider_DisplayFieldMode_eBottomFieldOnly,  /* Display Bottom fld only */
   BXDM_PictureProvider_DisplayFieldMode_eSingleField, /* Display single field only.
                                                        * The field that is displayed
                                                        * is the same as the one that
                                                        * is currently being
                                                        * displayed. */
   BXDM_PictureProvider_DisplayFieldMode_eAuto,    /* SW7405-4117: select either eSingleField or eBothField
                                                    * based on the steam height during slow motion (and preroll). */
   BXDM_PictureProvider_DisplayFieldMode_eMax
} BXDM_PictureProvider_DisplayFieldMode;

typedef enum BXDM_PictureProvider_SourceFormatOverride
{
   BXDM_PictureProvider_SourceFormatOverride_eDefault,
   BXDM_PictureProvider_SourceFormatOverride_eInterlaced,
   BXDM_PictureProvider_SourceFormatOverride_eProgressive,

   BXDM_PictureProvider_SourceFormatOverride_eMax
} BXDM_PictureProvider_SourceFormatOverride;


typedef enum BXDM_PictureProvider_ScanModeOverride
{
   BXDM_PictureProvider_ScanModeOverride_eDefault,
   BXDM_PictureProvider_ScanModeOverride_eInterlaced,
   BXDM_PictureProvider_ScanModeOverride_eProgressive,

   BXDM_PictureProvider_ScanModeOverride_eMax
} BXDM_PictureProvider_ScanModeOverride;


typedef enum BXDM_PictureProvider_240iScanMode
{
   /* Assume non-AVC 240i/288i has pulldown="F" (default) */
   BXDM_PictureProvider_240iScanMode_eForceProgressive,

   /* Assume non-AVC 240i/288i is coded correctly */
   BXDM_PictureProvider_240iScanMode_eUseEncodedFormat,

   BXDM_PictureProvider_240iScanMode_eMax
} BXDM_PictureProvider_240iScanMode;


typedef enum BXDM_PictureProvider_PulldownMode
{
   BXDM_PictureProvider_PulldownMode_eTopBottom,
   BXDM_PictureProvider_PulldownMode_eBottomTop,
   BXDM_PictureProvider_PulldownMode_eUseEncodedFormat,

   BXDM_PictureProvider_PulldownMode_eMax
} BXDM_PictureProvider_PulldownMode;


typedef enum BXDM_PictureProvider_DisplayMode
{
   BXDM_PictureProvider_DisplayMode_eTSM,   /* Use PTS for display timing */
   BXDM_PictureProvider_DisplayMode_eVirtualTSM, /* Ignore PTS and follow pulldown and frame rate */

   BXDM_PictureProvider_DisplayMode_eMax
} BXDM_PictureProvider_DisplayMode;


typedef struct BXDM_PictureProvider_TSMThresholdSettings
{
   uint32_t uiTooEarlyThreshold;
   uint32_t uiTooLateThreshold;
   uint32_t uiDeltaStcPtsDiffThreshold;
} BXDM_PictureProvider_TSMThresholdSettings;


typedef enum BXDM_PictureProvider_ClipTimeType
{
   /* Disables all clip logic. If clipping is currently active, it
    * will be disabled and normal TSM will resume. If a clip time was
    * previously set (but not yet used), it will be reset */
   BXDM_PictureProvider_ClipTimeType_eDisable = 0,

   /* Both clip start and stop times are valid w/ DVD specific
    * functionality:
      - Pictures causing PTS Errors are displayed in vsync mode
      - Start Time of 0 means start immediately
      - Stop Time of 0xFFFFFFFF means stop on a marker
      - Stop Event:
         - TSM Mode: STC value is compared to stop time
         - vTSM Mode: PTS value is compared to stop time
         - EOC/VSYNC Mode: stop time is ignored.  Marker is used to
           indicate stop.
   */
   BXDM_PictureProvider_ClipTimeType_eEnableBothWithDVDCompatibility,

   /* Both clip start and stop times are valid */
   BXDM_PictureProvider_ClipTimeType_eEnableBoth,

   /* Only clip start time is valid */
   BXDM_PictureProvider_ClipTimeType_eClipStartOnly,

   /* Only clip stop time is valid */
   BXDM_PictureProvider_ClipTimeType_eClipStopOnly,

   BXDM_PictureProvider_ClipTimeType_eMax
} BXDM_PictureProvider_ClipTimeType;

typedef struct BXDM_PictureProvider_ClipTimeSettings
{
   bool bValid;

   BXDM_PictureProvider_ClipTimeType eType;
   uint32_t uiStart;
   uint32_t uiStop;
   uint32_t uiId;
} BXDM_PictureProvider_ClipTimeSettings;


typedef enum BXDM_PictureProvider_PictureDropMode
{
   BXDM_PictureProvider_PictureDropMode_eField,
   BXDM_PictureProvider_PictureDropMode_eFrame,

   /* Add more enums ABOVE this line */
   BXDM_PictureProvider_PictureDropMode_eMax
} BXDM_PictureProvider_PictureDropMode;


typedef enum BXDM_PictureProvider_PTSType
{
   BXDM_PictureProvider_PTSType_eCoded,
   BXDM_PictureProvider_PTSType_eInterpolatedFromValidPTS,
   BXDM_PictureProvider_PTSType_eHostProgrammedPTS_Deprecated,
   BXDM_PictureProvider_PTSType_eInterpolatedFromInvalidPTS,

   BXDM_PictureProvider_PTSType_eMax
} BXDM_PictureProvider_PTSType;

typedef struct BXDM_PictureProvider_PTSInfo
{
   /* generally the PTS of the picture being delivered to VDC. */
   uint32_t ui32RunningPTS;

   /* The effective PTS value (accounting for all offsets) that DM
    * uses for TSM evaluation */
   uint32_t ui32EffectivePTS;

   /* The PTS type tag */
   BXDM_PictureProvider_PTSType ePTSType;

   /* the STC delivered by the video decoder on this vsync. */
   uint32_t uiSTCSnapShot;

   /* PCR offset used by the TSM logic for the picture being delivered to VDC. */
   uint32_t uiPCROffset;

   /* Indicates the state of the 'uiPCROffset' field */
   bool bPcrOffsetValid;

   /* the PCR discontiniuty flag for the picture being delivered to VDC. */
   bool bPcrDiscontinuity;

   /*
    * A running count of the number of elements (fields/frames) dropped by
    * the "Display Manager" due to "Time Stamp Management" failures.
    * Reset upon calling "BXVD_StartDecode()".
    */
   uint32_t uiDisplayManagerDroppedCount;

   /*
    * A running count of the number of pictures the decoder delivered to
    * the "Display Manager". Reset upon calling "BXVD_StartDecode()".
    */
   uint32_t uiPicturesDecodedCount;

   /* PR50623: A programmable offset added to the STC prior to TSM evaluation. */
   uint32_t uiSwPcrOffset;

   /* PR50623: A programmable flag which indicates whether or not to include the
    * HW PCR offset in the TSM equation.
    */
   bool bUseHwPcrOffset;

   uint32_t uiIntraGOPIndex; /* SW7425-2686: position of the picture within the GOP, "1" based.*/

} BXDM_PictureProvider_PTSInfo;

typedef enum BXDM_PictureProvider_TSMResult
{
   BXDM_PictureProvider_TSMResult_eTooEarly, /* (PTS > STC)  and ((PTS - STC) > discard threshold) */
   BXDM_PictureProvider_TSMResult_eWait,     /* (PTS > STC)  and ((PTS - STC) <= discard threshold) */
   BXDM_PictureProvider_TSMResult_ePass,     /* (PTS <= STC) and ((STC - PTS) <= too late threshold) */
   BXDM_PictureProvider_TSMResult_eTooLate,  /* (PTS < STC)  and ((STC - PTS) > too late threshold) */
   BXDM_PictureProvider_TSMResult_eDrop,     /* Drop for non-TSM reason (freeze, pts filtering, invalid pts, invalid pcr, etc) */

   BXDM_PictureProvider_TSMResult_eMax
} BXDM_PictureProvider_TSMResult;

typedef enum BXDM_PictureProvider_PictureHandlingMode
{
   /* In Broadcast:
    * - TooEarly --> eDrop
    * - TooLate --> eHonorPTS
    * In Playback:
    * - TooEarly --> eHonorPTS for 3 vsyncs.  eDrop after 3 vsyncs.  Reset on non-TooEarly.
    * - TooLate --> eIgnorePTS for first picture.  eHonorPTS for susequent pictures.  Reset on non-TooLate.
    */
   BXDM_PictureProvider_PictureHandlingMode_eDefault,

   /* Honor the PTS of the picture and assume it is correct */
   BXDM_PictureProvider_PictureHandlingMode_eHonorPTS,

   /* Ignore the PTS of the picture and instead interpolate from the previous PTS */
   BXDM_PictureProvider_PictureHandlingMode_eIgnorePTS,

   /* Drop the picture immediately */
   BXDM_PictureProvider_PictureHandlingMode_eDrop,

   /* SW7635-51: give the middleware/application the option to hold off processing a picture. */
   BXDM_PictureProvider_PictureHandlingMode_eWait,

   BXDM_PictureProvider_PictureHandlingMode_eMax
} BXDM_PictureProvider_PictureHandlingMode;

typedef struct BXDM_PictureProvider_TSMInfo
{
   BXDM_PictureProvider_TSMResult eTSMResult; /* The TSM Result for the currently evaluated picture.  *read only* */
   BXDM_PictureProvider_PictureHandlingMode ePictureHandlingMode; /* The picture handling mode for the currently evaluated picture */
} BXDM_PictureProvider_TSMInfo;

typedef struct BXDM_PictureProvider_Counters
{
   uint32_t uiDisplayManagerDroppedCount;  /* Number of Elements (Fields/Frames) DM decided to drop because of TSM. Running Counter. */
   uint32_t uiDecoderDroppedCount;         /* Number of Elements (Fields/Frames) DM asked the Decoder to drop. Running Counter. */
   uint32_t uiDisplayedParityFailureCount; /* Number of Elements (Fields/Frames) DM displayed eventhough there was Parity Failure, under normal display condition. Running Counter. */
   uint32_t uiDisplayedCount;              /* Number of Elements (Fields/Frames) DM delevered to VDC under normal display conditions. Running Counter. */
   uint32_t uiPicturesDecodedCount;        /* Number of Entries (PPBs) DM received from the AVD FW. Running Counter. */
   uint32_t uiVsyncUnderflowCount;         /* Number of Vsync the DM had Underflow in the Disp FIFO. Running Counter. */
   uint32_t uiDecodeErrorCount;            /* Number of Errors Decoder encountered. DM only passes this info as recieved from the AVD through the PPB */
   uint32_t uiVsyncCount;                  /* Running count, number of times DM callback has been executed for this channel. */
   uint32_t uiIFrameCount;                 /* Number of I frames DM received (inclusive of errors). Running Counter. */
   uint32_t uiErrorIFrameCount;            /* Number of I frames DM received with the error bit set. Running Counter. */
   uint32_t uiPicturesReceivedCount;       /* Number of pictures received from the decoder. */
   uint32_t uiPicturesReleasedCount;       /* Number of pictures released by the DM. */
   uint32_t uiUnderflowCount;              /* Number of times the delivery queue has gotten into an underflow state. */
} BXDM_PictureProvider_Counters;

typedef enum BXDM_PictureProvider_FrameAdvanceMode
{
   BXDM_PictureProvider_FrameAdvanceMode_eOff,
   BXDM_PictureProvider_FrameAdvanceMode_eField,
   BXDM_PictureProvider_FrameAdvanceMode_eFrame,
   BXDM_PictureProvider_FrameAdvanceMode_eFrameByField,

   BXDM_PictureProvider_FrameAdvanceMode_eMax
} BXDM_PictureProvider_FrameAdvanceMode;


typedef struct BXDM_PictureProvider_PreserveStateSettings
{
   bool bDisplay;
   bool bCounters;
} BXDM_PictureProvider_PreserveStateSettings;


typedef enum BXDM_PictureProvider_MonitorRefreshRate
{
   BXDM_PictureProvider_MonitorRefreshRate_eUnknown = 0,
   BXDM_PictureProvider_MonitorRefreshRate_e7_493Hz,
   BXDM_PictureProvider_MonitorRefreshRate_e7_5Hz,
   BXDM_PictureProvider_MonitorRefreshRate_e9_99Hz,
   BXDM_PictureProvider_MonitorRefreshRate_e10Hz,
   BXDM_PictureProvider_MonitorRefreshRate_e11_988Hz,
   BXDM_PictureProvider_MonitorRefreshRate_e12Hz,
   BXDM_PictureProvider_MonitorRefreshRate_e12_5Hz,
   BXDM_PictureProvider_MonitorRefreshRate_e14_985Hz,
   BXDM_PictureProvider_MonitorRefreshRate_e15Hz,
   BXDM_PictureProvider_MonitorRefreshRate_e19_98Hz,
   BXDM_PictureProvider_MonitorRefreshRate_e20Hz,
   BXDM_PictureProvider_MonitorRefreshRate_e23_976Hz,
   BXDM_PictureProvider_MonitorRefreshRate_e24Hz,
   BXDM_PictureProvider_MonitorRefreshRate_e25Hz,
   BXDM_PictureProvider_MonitorRefreshRate_e29_97Hz,
   BXDM_PictureProvider_MonitorRefreshRate_e30Hz,
   BXDM_PictureProvider_MonitorRefreshRate_e48Hz,
   BXDM_PictureProvider_MonitorRefreshRate_e50Hz,
   BXDM_PictureProvider_MonitorRefreshRate_e59_94Hz,
   BXDM_PictureProvider_MonitorRefreshRate_e60Hz,
   BXDM_PictureProvider_MonitorRefreshRate_e100Hz,
   BXDM_PictureProvider_MonitorRefreshRate_e119_88Hz,
   BXDM_PictureProvider_MonitorRefreshRate_e120Hz,

   /* Add new enums above this line */
   BXDM_PictureProvider_MonitorRefreshRate_eMax
} BXDM_PictureProvider_MonitorRefreshRate;

typedef enum BXDM_PictureProvider_1080pScanMode
{
   /* eDefault: 1080p pass-thru, else 1080i
    *
    * 1080p pass-thru cases:
    *   - 1080p23.97/24 --> 23.97/24 Hz
    *   - 1080p25 --> 25 Hz
    *   - 1080p29.97/30 --> 29.97/30 Hz
    */
    BXDM_PictureProvider_1080pScanMode_eDefault = 0,

   /* eAdvanced: 1080p pass-thru + 1080p pulldown, else 1080i
    *
    * 1080p pulldown cases:
    *   - 1080p23.97/24 --> 59.94/60 Hz (3:2 frame cadence)
    *   - 1080p29.97/30 --> 59.94/60 Hz (2:2 frame cadence)
    *   - 1080p25 --> 50Hz (2:2 frame cadence)
    */
   BXDM_PictureProvider_1080pScanMode_eAdvanced,

   /* Add new enums ABOVE this line */
   BXDM_PictureProvider_1080pScanMode_eMax
} BXDM_PictureProvider_1080pScanMode;


typedef enum BXDM_PictureProvider_FrameRateDetectionMode
{
   /* Disable frame rate detection.  Will always return unknown */
   BXDM_PictureProvider_FrameRateDetectionMode_eOff,

   /* Fast frame rate detection.  Will return best guess as soon as
    * possible.  Result may oscillate initially for very close frame
    * rates (e.g. 29.97 vs 30 fps) */
   BXDM_PictureProvider_FrameRateDetectionMode_eFast,

   /* Stable frame rate detection.  Will return "unknown" until frame
    * rate stabilizes.  Good for differentiating between close frame
    * rates (e.g. 29.97 vs 30 fps) */
   BXDM_PictureProvider_FrameRateDetectionMode_eStable,

   BXDM_PictureProvider_FrameRateDetectionMode_eMax

} BXDM_PictureProvider_FrameRateDetectionMode;


typedef enum BXDM_PictureProvider_ErrorHandlingMode
{
   BXDM_PictureProvider_ErrorHandlingMode_eOff, /* display all pictures */
   BXDM_PictureProvider_ErrorHandlingMode_ePicture, /* drop pictures if Decode Error is set  */
   BXDM_PictureProvider_ErrorHandlingMode_ePrognostic, /* drop pictures from Decode Error to the next RAP */

   BXDM_PictureProvider_ErrorHandlingMode_eMax
} BXDM_PictureProvider_ErrorHandlingMode;


/* SW7405-4703: Add APIs for controlling Horizontal Overscan processing */
typedef enum BXDM_PictureProvider_HorizontalOverscanMode
{
   BXDM_PictureProvider_HorizontalOverscanMode_eAuto,
   BXDM_PictureProvider_HorizontalOverscanMode_eDisable,

   /* Add more enums ABOVE this line */
   BXDM_PictureProvider_HorizontalOverscanMode_eMax
} BXDM_PictureProvider_HorizontalOverscanMode;


/* SWDEPRECATED-1003: needed to turn off the FIC logic during certain trick modes */
typedef enum BXDM_PictureProvider_TrickMode
{
   BXDM_PictureProvider_TrickMode_eAuto=0,
   BXDM_PictureProvider_TrickMode_eNormal,
   BXDM_PictureProvider_TrickMode_eSparsePictures,
   BXDM_PictureProvider_TrickMode_ePause,
   BXDM_PictureProvider_TrickMode_eRewind,
   BXDM_PictureProvider_TrickMode_eFastForward,

   /* Add more enums ABOVE this line */
   BXDM_PictureProvider_TrickMode_eMax
} BXDM_PictureProvider_TrickMode;


/* SW7422-72: API's and constants to allow the middleware/application to specify
 * an orientation for each picture.  If "bOverrideOrientation" is set
 * in the BXDM_PictureProvider_3DSettings structure, the "eOrientation" field
 * will be used to fill in the MFD structure.
 * If "bOverrideOrientation" is not set, the MFD will be filled in based on
 * data received from AVD, i.e. SEI messages and/or PPB flags.
 * "bSetNextPointer" is to support MVC on the older DVD chips.  When true,
 * both of the MFD pointers "pNext" and "pEnhanced" will be set.
 */

typedef enum BXDM_PictureProvider_Orientation
{
   BXDM_PictureProvider_Orientation_e2D=0,
   BXDM_PictureProvider_Orientation_eLeftRight,
   BXDM_PictureProvider_Orientation_eOverUnder,
   BXDM_PictureProvider_Orientation_eLeftRightFullFrame,
   BXDM_PictureProvider_Orientation_eRightLeftFullFrame,
   BXDM_PictureProvider_Orientation_eLeftRightEnhancedResolution,

   BXDM_PictureProvider_Orientation_eMax

} BXDM_PictureProvider_Orientation;

typedef struct BXDM_PictureProvider_3DSettings
{
   bool bOverrideOrientation;
   BXDM_PictureProvider_Orientation eOrientation;

   bool bSetNextPointer;

} BXDM_PictureProvider_3DSettings;


/*
 * SW7425-1264: support for a synthesized SW STC, can create a clock internal
 * to XDM that run backwards.
 *
 * The clock override is set by calling "BXDM_PictureProvider_SetClockOverride_isr".
 * "BXDM_PictureProvider_GetClockOverride_isr" returns the most recently written values.
 *
 * When "bEnableClockOverride" is true, XDM will use an internal SW STC.
 *
 * When "bLoadSwStc" is true, the SW STC will be initialized with "uiStcValue".
 * This loading occurs at the beginning of XDM's execution.  If "bLoadSwStc"
 * is false, the SW STC will not be loaded.  This mechanism allows
 * "BXDM_PictureProvider_SetClockOverride_isr" to be called without changing the
 * current value of the SW STC.
 *
 * "iStcDelta" is added to the SW STC every vsync, effectively at the end of XDM's execution.
 * "iStcDelta" can be negative, this will cause the clock to run backwards.
 * The units of "iStcDelta" are clock ticks; for example to achieve a 60 Hz display rate,
 * "iStcDelta" would be set to 750.
 *
 * The current SW STC can be retrieved by calling BXDM_PictureProvider_GetCurrentPTSInfo_isr
 * or BXVD_GetPTS_isr with "bEnableClockOverride" set to true.  The SW STC will be returned in
 * "stPTSInfo.uiSTCSnapShot".
 */
typedef struct BXDM_PictureProvider_ClockOverride
{
   /* When true, XVD will use an internal SW STC */
   bool       bEnableClockOverride;

   /* Initial value of the internal SW STC */
   uint32_t   uiStcValue;

   /* When true, the internal SW STC will be loaded with uiStcValue.
    * If false, the uiStcValue will not be changed.
    */
   bool       bLoadSwStc;

   /* Added to the internal STC on each vsync.
    * A negative value will cause the STC to decrement
    */
   int32_t    iStcDelta;

} BXDM_PictureProvider_ClockOverride;

/*
 * SW7425-3358: FNRT support, set by calling BXDM_PictureProvider_SetFNRTSettings_isr.
 */
typedef struct BXDM_PictureProvider_FNRTSettings
{
   bool bEnabled;

   /* Indicates the number of pictures to drop at beginning of chunks 1..n and
    * the number of pictures to encode at the end of chunks 0..n-1. (this value is also known as "K").
    */
   uint32_t uiPreChargeCount;

} BXDM_PictureProvider_FNRTSettings;

/*
 * SW7425-3358: FNRT support, returned by BXDM_PictureProvider_Callback_eChunkDone callback.
 */
typedef struct BXDM_PictureProvider_Callback_ChunkDoneInfo
{
   uint32_t uiChunkId;  /* ID of the chunk that has just completed. */

} BXDM_PictureProvider_Callback_ChunkDoneInfo;

/*
 * SW7425-4630
 */
typedef struct BXDM_PictureProvider_FilterSettings
{
   uint32_t uiSimplyAPlaceHolder;   /* A hedge in case we want to pass settings in the future. */

}  BXDM_PictureProvider_FilterSettings;

/* SWSTB-68: how to handle discontinuous PCR offsets.
 * Currently this is only applied at the beginning of decode in live mode.
 *
 * BXDM_PictureProvider_PCRDiscontinuityMode_eDrop - drop the picture
 * BXDM_PictureProvider_PCRDiscontinuityMode_eVsync - evaluate the picture in vsync mode
 * BXDM_PictureProvider_PCRDiscontinuityMode_eLegacy - evaluate the picture in TSM mode */

typedef enum BXDM_PictureProvider_PCRDiscontinuityMode
{
   BXDM_PictureProvider_PCRDiscontinuityMode_eDrop=0,
   BXDM_PictureProvider_PCRDiscontinuityMode_eVsync,
   BXDM_PictureProvider_PCRDiscontinuityMode_eLegacy,

   BXDM_PictureProvider_PCRDiscontinuityMode_eMax

} BXDM_PictureProvider_PCRDiscontinuityMode;

typedef struct BXDM_PictureProvider_PCRModes
{
   /* Applies in live mode until a valid PCR offset is received. */
   BXDM_PictureProvider_PCRDiscontinuityMode ePCRDiscontinuityModeAtStartup;

}  BXDM_PictureProvider_PCRModes;


/*
 * SWSTB-1380: descriptor of the debug fifo.
 */
typedef struct BXDM_PictureProvider_DebugFifoInfo
{
   size_t uiElementSize;      /* Size of one element */
   BMMA_Block_Handle hBlock;  /* Memory Block containing the XDM internal BDBG_Fifo */
   unsigned uiOffset;         /* Offset from start of memory block where BDBG_Fifo begins */

} BXDM_PictureProvider_DebugFifoInfo;

/*
 * SWSTB-3450: support for overriding the color parameters.
 */

typedef enum BXDM_PictureProvider_ColorOverrideMode
{
   BXDM_PictureProvider_ColorOverrideMode_eNone=0,
   BXDM_PictureProvider_ColorOverrideMode_eForce,

   BXDM_PictureProvider_ColorOverrideMode_eMax
} BXDM_PictureProvider_ColorOverrideMode;


typedef struct BXDM_PictureProvider_HDRColorParameters
{
   uint32_t      ulAvgContentLight;       /* if non zero, indicates max average light level */
   uint32_t      ulMaxContentLight;       /* if non zero, indicates max light level */

   BAVC_Point    stDisplayPrimaries[3];   /* coordinates of the color primaries */
   BAVC_Point    stWhitePoint;            /* coordinates of the white point */
   uint32_t      ulMaxDispMasteringLuma;  /* nominal max display luminance */
   uint32_t      ulMinDispMasteringLuma;  /* nominal min display luminance */

   /* see bavc_types.h for definitions of the transfer characteristics. */
   BAVC_TransferCharacteristics eTransferCharacteristics;

} BXDM_PictureProvider_HDRColorParameters;

typedef struct BXDM_PictureProvider_SDRColorParameters
{
   /* see bavc_types.h for definitions of the transfer characteristics. */
   BAVC_TransferCharacteristics    eTransferCharacteristics;

} BXDM_PictureProvider_SDRColorParameters;

typedef struct BXDM_PictureProvider_ColorOverride
{
   BXDM_PictureProvider_ColorOverrideMode eOverrideMode;

   BXDM_PictureProvider_SDRColorParameters stSDR;
   BXDM_PictureProvider_HDRColorParameters stHDR;

} BXDM_PictureProvider_ColorOverride;

/*
 * SWSTB-3450: support for passing parameters directly to BXDM_PictureProvider_Start_isr
 */
typedef struct BXDM_PictureProvider_StartSettings
{
    BXDM_PictureProvider_ColorOverride stColorOverride;

} BXDM_PictureProvider_StartSettings;

/*
 * SWSTB-3450: support for passing parameters directly to BXDM_PictureProvider_Stop_isr
 */
typedef struct BXDM_PictureProvider_StopSettings
{
    uint32_t uiSimplyAPlaceHolder;   /* A hedge in case we want to pass settings in the future. */

} BXDM_PictureProvider_StopSettings;

#ifdef __cplusplus
}
#endif

#endif /* BXDM_PP_H_ */
