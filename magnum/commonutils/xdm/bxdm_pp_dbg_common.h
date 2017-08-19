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

#ifndef bxdm_pp_DBG_COMMON_H__
#define bxdm_pp_DBG_COMMON_H__

#if 0
#include "bxdm_pp.h"
#include "bxdm_pp_priv.h"
#endif
#include "bavc.h"

#ifdef __cplusplus
extern "C" {
#endif

#if 0 /* to keep some editors happy */
}
#endif

#define BXDM_PPDBG_FORMAT_INSTANCE_ID( _hXdmPP_ )   \
   _hXdmPP_->stDMConfig.uiInstanceID & 0xFF        \


#define BXDM_PPDBG_P_MAX_VSYNC_DEPTH 12
#define BXDM_PPDBG_P_MAX_DBG_LENGTH 1024

typedef enum BXDM_PPDBG_Selection
{
   BXDM_PPDBG_Selection_ePPBNotFound,
   BXDM_PPDBG_Selection_ePPBFound,
   BXDM_PPDBG_Selection_ePass,
   BXDM_PPDBG_Selection_eForce,
   BXDM_PPDBG_Selection_eLate,
   BXDM_PPDBG_Selection_eWait,
   BXDM_PPDBG_Selection_eFreeze,
   BXDM_PPDBG_Selection_eTooEarly,
   BXDM_PPDBG_Selection_eDrop,
   BXDM_PPDBG_Selection_eDelay,
   BXDM_PPDBG_Selection_eDependentPicture,
#if 0
   BXDM_PPDBG_Selection_PolarityOverride_eBothField,
   BXDM_PPDBG_Selection_PolarityOverride_eProgressive,
#endif
   BXDM_PPDBG_Selection_PolarityOverride_e1stTo2ndSlot,
   BXDM_PPDBG_Selection_PolarityOverride_e2ndTo1stSlot,
   BXDM_PPDBG_Selection_PolarityOverride_e2ndSlotNextElement,
   BXDM_PPDBG_Selection_PolarityOverride_eSelectPrevious,
   BXDM_PPDBG_Selection_PolarityOverride_eRepeatPrevious,
   BXDM_PPDBG_Selection_PolarityOverride_eFICReset,
   BXDM_PPDBG_Selection_PolarityOverride_eFICForceWait,
   BXDM_PPDBG_Selection_eStcInvalidForceWait,
   BXDM_PPDBG_Selection_eMulliganLogicForceWait,
   BXDM_PPDBG_Selection_eTSMResultCallbackForceWait,

   /* Add new enums above this line */
   BXDM_PPDBG_Selection_eMax
} BXDM_PPDBG_Selection;

/*
 * Defines and macro(s) for building the debug messages.
 */
#define BXDM_PPDBG_P_STR_PREFIX     5     /* All the strings start with a five character prefix. */
#define BXDM_PPDBG_P_STR_SUFFIX     1     /* Always want a NULL terminator at the end of the string. */
#define BXDM_PPDBG_P_STR_PER_VSYNC  15    /* Except for the "stTSMString" string, 8 characters will be appended on each vsync.
                                           * Typically 5 or 7 characters will be appended to "stTSMString" on each vsync, but it
                                           * could be more than that if pictures are dropped.  A value of 15 will give us a little
                                           * wiggle room. */

#define BXDM_PPDBG_P_STR_LENGTH  BXDM_PPDBG_P_STR_PREFIX                                           \
                                 + ( BXDM_PPDBG_P_STR_PER_VSYNC *  BXDM_PPDBG_P_MAX_VSYNC_DEPTH )  \
                                 + BXDM_PPDBG_P_STR_SUFFIX                                         \


#define BXDM_PPDBG_P_APPEND_CHAR( _pStr, _char )                  \
   if ( _pStr->uiDebugStrOffset < BXDM_PPDBG_P_STR_LENGTH - 2 )   \
   {                                                              \
      _pStr->szDebugStr[ _pStr->uiDebugStrOffset++ ] = _char;     \
   }                                                              \


typedef struct BXDM_PPDBG_P_String
{
   char szDebugStr[BXDM_PPDBG_P_MAX_DBG_LENGTH];
   uint32_t uiDebugStrOffset;
} BXDM_PPDBG_P_String;

typedef struct BXDM_PPDBG_P_Info
{
   uint32_t uiVsyncCount;

   BXDM_PPDBG_P_String stHeaderString;
   BXDM_PPDBG_P_String stTSMString;
   BXDM_PPDBG_P_String stPendingDropString;
   BXDM_PPDBG_P_String stInterruptString;
   BXDM_PPDBG_P_String stSourcePolarityOverrideString;
   BXDM_PPDBG_P_String stCallbackString;
   BXDM_PPDBG_P_String stStateString;
   BXDM_PPDBG_P_String stState2String;
   BXDM_PPDBG_P_String stStcDeltaString;

   bool bPrintSPO;
   bool bPrintCallbacks;
   bool bPrintState2;
   bool bPrintDropCount;

   uint32_t abSelectionLogHeader[BXDM_PPDBG_P_MAX_VSYNC_DEPTH];
} BXDM_PPDBG_P_Info;

#define BXDM_PPDBG_Output_SPO_UNUSED_0       0x0001
#define BXDM_PPDBG_Output_SPO_UNUSED_1       0x0002
#define BXDM_PPDBG_Output_SPO_UNUSED_2       0x0004
#define BXDM_PPDBG_Output_SPO_UNUSED_4       0x0008

#define BXDM_PPDBG_Output_SPO_TopField       0x0010
#define BXDM_PPDBG_Output_SPO_BottomField    0x0020
#define BXDM_PPDBG_Output_SPO_SingleField    0x0040

#define BXDM_PPDBG_Output_SPO_Interlaced     0x0100
#define BXDM_PPDBG_Output_SPO_Progressive    0x0200
#define BXDM_PPDBG_Output_SPO_pToi           0x0400
#define BXDM_PPDBG_Output_SPO_iTop           0x0800

#define BXDM_PPDBG_Output_SPO_ProgBothField  0x1000
#define BXDM_PPDBG_Output_SPO_UNUSED_5       0x2000
#define BXDM_PPDBG_Output_SPO_UNUSED_6       0x4000
#define BXDM_PPDBG_Output_SPO_ProgRepeat     0x8000

#define BXDM_PPDBG_Callback_FirstPtsReady      0x00001
#define BXDM_PPDBG_Callback_FirstCodedPtsReady 0x00002
#define BXDM_PPDBG_Callback_FirstPtsPassed     0x00004
#define BXDM_PPDBG_Callback_PtsError           0x00008

#define BXDM_PPDBG_Callback_PtsStcOffset       0x00010
#define BXDM_PPDBG_Callback_IFrame             0x00020
#define BXDM_PPDBG_Callback_PictureParameters  0x00040
#define BXDM_PPDBG_Callback_TsmPassInASTMMode  0x00080

#define BXDM_PPDBG_Callback_RequestSTC         0x00100
#define BXDM_PPDBG_Callback_ClipStart          0x00200
#define BXDM_PPDBG_Callback_ClipStop           0x00400
#define BXDM_PPDBG_Callback_PictureMarker      0x00800

#define BXDM_PPDBG_Callback_PPBReceived        0x01000
#define BXDM_PPDBG_Callback_PictureUnderEvaluation      0x02000
#define BXDM_PPDBG_Callback_DecodeError        0x04000
#define BXDM_PPDBG_Callback_TSMResult          0x08000

#define BXDM_PPDBG_Callback_ExtensionData      0x10000
#define BXDM_PPDBG_Callback_ChunkDone          0x20000

#define BXDM_PPDBG_State_SelectionMode_TSM       0x00001
#define BXDM_PPDBG_State_SelectionMode_VSYNC     0x00002
#define BXDM_PPDBG_State_DisplayMode_TSM         0x00004
#define BXDM_PPDBG_State_DisplayMode_VSYNC       0x00008

#define BXDM_PPDBG_State_IgnoreCadenceMatch      0x00010
#define BXDM_PPDBG_State_PCRDiscontinuity        0x00020
#define BXDM_PPDBG_State_PCRPresent              0x00040
#define BXDM_PPDBG_State_STCInvalid              0x00080

#define BXDM_PPDBG_State_ProgressiveFramePulldown       0x00100
#define BXDM_PPDBG_State_ProgressiveStreamPulldown      0x00200
#define BXDM_PPDBG_State_ProgressiveSourceFormat        0x00400
#define BXDM_PPDBG_State_ProgressiveSequence            0x00800

#define BXDM_PPDBG_State_UseHwPcrOffset            0x01000
#define BXDM_PPDBG_State_480pTo480i                0x02000
#define BXDM_PPDBG_State_1080pTo1080i              0x04000
#define BXDM_PPDBG_State_60iTo60p                  0x08000
#define BXDM_PPDBG_State_24iTo24p                  0x10000
#define BXDM_PPDBG_State_240iTo240p                0x20000
#define BXDM_PPDBG_State_ForceSingleFieldMode      0x40000

/* Bit definitions for the second state word. */

#define BXDM_PPDBG_State2_StcStalled            0x00001
#define BXDM_PPDBG_State2_RequestedStcStall     0x00002
#define BXDM_PPDBG_State2_IgnorePicture         0x00004
#define BXDM_PPDBG_State2_NRTStallDetected      0x00008

#define BXDM_PPDBG_State2_FrameAdvance          0x00010
#define BXDM_PPDBG_State2_PreviousVsyncTrick    0x00020     /* bPreviousVsyncWasTrickMode */
#define BXDM_PPDBG_State2_TrickModeTransition   0x00040     /* bTrickModeTransition */
#define BXDM_PPDBG_State2_InitialSTCInvalid     0x00080

#define BXDM_P_MAX_INTERRUPT_POLARITY  BAVC_Polarity_eFrame + 1
#define BXDM_P_MAX_VIDEO_PROTOCOL 22

extern const char BXDM_P_InterruptPolarityToStrLUT[];
extern const char BXDM_P_PicturePolaritytoStrLUT[][BXDM_P_MAX_INTERRUPT_POLARITY];
extern const char BXDM_P_HexToCharLUT[];
extern const char BXDM_P_PictureSelectionLUT[];
extern const char * const BXDM_P_PolarityToStrLUT[];
extern const char * const BXDM_P_BAVCFrameRateToStrLUT[];
extern const char * const BXDM_P_FrameRateTypeToStrLUT[];
extern const char * const BXDM_P_OrientationToStrLUT[];
extern const char * const BXDM_P_BAVCPictureCodingToStrLUT[];
extern const char * const BXDM_P_BXDMPictureCodingToStrLUT[];
extern const char * const BXDM_P_AspectRatioToStrLUT[];
extern const char * const BXDM_P_MonitorRefreshRateToStrLUT[];
extern const char * const BXDM_P_STCTrickModeToStrLUT[];
extern const char * const BXDM_P_TSMResultToStrLUT[];
extern const char * const BXDM_P_PullDownEnumToStrLUT[];
extern const char * const BXDM_P_SiPullDownEnumToStrLUT[];
extern const char * const BXDM_P_SiRepeatPullDownEnumToStrLUT[];
extern const char * const BXDM_P_DisplayFieldModeToStrLUT[];
extern const char * const BXDM_P_PPOrientationToStrLUT[];
extern const char * const BXDM_P_PulldownModeToStrLUT[];
extern const char * const BXDM_P_FrameAdvanceModeToStrLUT[];
extern const char * const BXDM_P_VideoCompressionStdToStrLUT[];
extern const char * const BXDM_P_TrickModeToStrLUT[];
extern const char * const BXDM_P_FrameRateDetectionModeToStrLUT[];
extern const char * const BXDM_P_ErrorHandlingModeToStrLUT[];
extern const char * const BXDM_P_BFMTRefreshRateToStrLUT[];

typedef enum BXDM_Debug_MsgType
{
   BXDM_Debug_MsgType_eUnKnown=0,
   BXDM_Debug_MsgType_eQM,
   BXDM_Debug_MsgType_eDBG,
   BXDM_Debug_MsgType_eDBG2,
   BXDM_Debug_MsgType_eDBGC,
   BXDM_Debug_MsgType_eMFD1,
   BXDM_Debug_MsgType_eMFD2,
   BXDM_Debug_MsgType_eMFD3,
   BXDM_Debug_MsgType_eCFG,
   BXDM_Debug_MsgType_eFRD,
   BXDM_Debug_MsgType_eFIC,
   BXDM_Debug_MsgType_eCB,
   BXDM_Debug_MsgType_eCLIP,
   BXDM_Debug_MsgType_eOUT,
   BXDM_Debug_MsgType_eTSM,
   BXDM_Debug_MsgType_eVTSM,
   BXDM_Debug_MsgType_ePPV2,

   /* Add new enums ABOVE this line */
   BXDM_Debug_MsgType_eMax
} BXDM_Debug_MsgType;


#ifdef __cplusplus
}
#endif

#endif /* #ifndef bxdm_pp_DBG_COMMON_H__ */
