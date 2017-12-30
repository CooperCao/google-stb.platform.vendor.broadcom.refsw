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

#ifndef BXDM_PP_DBG_FIFO_PRIV_H__
#define BXDM_PP_DBG_FIFO_PRIV_H__

#include "bxdm_pp.h"
#include "bxdm_pp_priv.h"
#include "bavc.h"
#include "bdbg_fifo.h"

#ifdef __cplusplus
extern "C" {
#endif

#if 0 /*needed by some editors */
}
#endif

/*
 * SWSTB-1380:
 */
/* #define BVCE_P_DEFAULT_DEBUG_LOG_SIZE (512*1024)*/  /* Is this needed? */

#define BXDM_P_MAX_DEBUG_FIFO_COUNT 128
#define BXDM_P_MAX_DEBUG_FIFO_STRING_LEN 128

typedef enum BXDM_DebugFifo_EntryType
{
   BXDM_DebugFifo_EntryType_eMFD,               /* MFD data send to XVD on this vsync. */
   BXDM_DebugFifo_EntryType_eUnifiedPicture,    /* Unified Pictures. */
   BXDM_DebugFifo_EntryType_eString,            /* Just a string */
   BXDM_DebugFifo_EntryType_eDebugInfo,         /* Info used by BXDM_PPDBG. */
   BXDM_DebugFifo_EntryType_eConfig,            /* Current XDM config data. */

   /* Add new enums ABOVE this line */
   BXDM_DebugFifo_EntryType_eMax

} BXDM_DebugFifo_EntryType;

typedef struct BXDM_DebugFifo_Metadata
{
   BXDM_DebugFifo_EntryType eType;
   unsigned uiInstanceID;  /* XDM instance ID, really a channel ID. */
   unsigned uiPPBIndex;    /* valid when a message is associated with a particular picture, otherwise -1 */
   unsigned uiVsyncCount;  /* modulo 12. */
   unsigned uiChannel;
   unsigned uiTimestamp;   /* (in milliseconds) */
} BXDM_DebugFifo_Metadata;

typedef struct BXDM_DebugFifo_String
{
   BXDM_Debug_MsgType eType;
   char szString[BXDM_P_MAX_DEBUG_FIFO_STRING_LEN];

} BXDM_DebugFifo_String;

typedef struct BXDM_DebugFifo_UnifiedPicture
{
   BXDM_Picture stUnifiedPicture;

   BXDM_Picture_PullDown ePulldown;
   char cSelectionMode;
   char cErrorOnThisPicture;
   BXDM_PictureProvider_DisplayMode eSelectionMode;
   BXDM_PictureProvider_TSMResult eTsmResult;
   bool bAppendedToPreviousPicture;
   unsigned uiDisplayOffset;
   int32_t iStcPtsDifferenceActual;

   /* Should the following two blocks be collapsed into a union? */
   /* For TSM mode. */
   unsigned uiPcrOffset;
   bool     bPcrOffsetValid;
   bool     bPcrOffsetDiscontinuity;
   unsigned uiStcSnapshot;
   int32_t  iPTSJitterCorrection;
   int32_t  iStcJitterCorrectionOffset;

   /* For vsync mode. */
   unsigned    uiVirtualStc;
   BXDM_PPFP_P_DataType stVirtualPts;

   /* Extra data. */
   bool bPrintAdditional;

   /* The following will be fill in when bPrintAdditional is true.
    * Fill it in all the time instead? */
   bool bProtocolValid;
   BAVC_FrameRateCode eFrameRate;
   BXDM_PPFP_P_DataType stDeltaPTSAvg;
   uint32_t uiAverageFractionBase10;
   BXDM_PictureProvider_P_FrameRateType eFrameRateType;
   uint32_t uiSwPcrOffsetUsedForEvaluation;
   /* collapse the following into a single field? */
   BXDM_PictureProvider_DisplayFieldMode eDisplayFieldMode;
   BXDM_PictureProvider_DisplayFieldMode ePictureDisplayFieldMode;
   bool bForceSingleFieldMode;


}  BXDM_DebugFifo_UnifiedPicture;

typedef struct BXDM_DebugFifo_DebugInfo
{
   unsigned uiStcSnapshot;
   int32_t  iStcJitterCorrectionOffset;
   BXDM_PictureProvider_MonitorRefreshRate eMonitorRefreshRate;
   BXDM_PPFP_P_DataType stSTCDelta;
   int32_t iAverageStcDelta;
   unsigned uiAverageFractionBase10;
   unsigned uiSlowMotionRate;
   BXDM_PictureProvider_P_STCTrickMode eSTCTrickMode;
   bool bPlayback;

} BXDM_DebugFifo_DebugInfo;

typedef struct BXDM_DebugFifo_Config
{
   bool bLastCall;
   BXDM_PictureProvider_P_Config stConfig;

} BXDM_DebugFifo_Config;

typedef struct BXDM_P_DebugFifo_Entry
{
   BXDM_DebugFifo_Metadata stMetadata;

   union
   {
      BAVC_MFD_Picture              stMFD;
      BXDM_DebugFifo_UnifiedPicture stUniPic;
      BXDM_DebugFifo_String         stString;
      BXDM_DebugFifo_DebugInfo      stDebugInfo;
      BXDM_DebugFifo_Config         stConfigInfo;

   } data;

} BXDM_P_DebugFifo_Entry;

/*
 * Functions
 */

void BXDM_PPDFIFO_P_QueString_isr(
   const BXDM_PictureProvider_Handle hXdmPP,
   const BXDM_Debug_MsgType eMessageType,
   const bool bNeedFormat,
   char * format,
   ...
   );

#if BDBG_DEBUG_BUILD

BERR_Code BXDM_PPDFIFO_P_QueDBG_isr(
   const BXDM_PictureProvider_Handle hXdmPP,
   const BXDM_DebugFifo_DebugInfo * pstDebugInfo
   );

void BXDM_PPDFIFO_P_QueDMConfig_isr(
   BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PictureProvider_P_LocalState * pLocalState,
   bool bLastCall
   );

void BXDM_PPDFIFO_P_QueMFD_isr(
   BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PictureProvider_P_LocalState* pLocalState,
   BAVC_MFD_Picture* pMFDPicture
   );

void BXDM_PPDFIFO_P_QueUnifiedPicture_isr(
   BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PictureProvider_P_LocalState * pLocalState,
   BXDM_PictureProvider_P_Picture_Context * pstPicture
   );

BERR_Code BXDM_PPDFIFO_P_Fifo_Create(
   BXDM_PictureProvider_Handle hXdmPP
   );

BERR_Code BXDM_PPDFIFO_P_Fifo_Destroy(
   BXDM_PictureProvider_Handle hXdmPP
   );

BERR_Code BXDM_PPDFIFO_P_Reader_Create(
   BXDM_PictureProvider_Handle hXdmPP
   );

BERR_Code BXDM_PPDFIFO_P_Reader_Destroy(
   BXDM_PictureProvider_Handle hXdmPP
   );

#else
/* Non-DEBUG build */

#define BXDM_PPDFIFO_P_QueDBG_isr( hXdmPP, pstDebugInfo )
#define BXDM_PPDFIFO_P_QueDMConfig_isr( hXdmPP, pLocalState, bLastCall )
#define BXDM_PPDFIFO_P_QueMFD_isr( hXdmPP, pLocalState, pMFDPicture )
#define BXDM_PPDFIFO_P_QueUnifiedPicture_isr( hXdmPP, pLocalState, pstPicture )

#define BXDM_PPDFIFO_P_Fifo_Create( hXdmPP )
#define BXDM_PPDFIFO_P_Fifo_Destroy( hXdmPP )
#define BXDM_PPDFIFO_P_Reader_Create( hXdmPP )
#define BXDM_PPDFIFO_P_Reader_Destroy( hXdmPP )

#endif

#ifdef __cplusplus
}
#endif

#endif /* #ifndef BXDM_PP_DBG_FIFO_PRIV_H__ */
