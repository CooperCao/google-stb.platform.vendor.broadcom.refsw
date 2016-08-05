/***************************************************************************
 * Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

#define BXDM_P_MAX_DEBUG_FIFO_COUNT 1024
#define BXDM_P_MAX_DEBUG_FIFO_STRING_LEN 128

typedef enum BXDM_DebugFifo_EntryType
{
   BXDM_DebugFifo_EntryType_eMFD,                           /* MFD data send to XVD on this vsync. */
   BXDM_DebugFifo_EntryType_eUnifiedPicture,                /* Unified Pictures. */
   BXDM_DebugFifo_EntryType_eString,                /* Just a string */
   BXDM_DebugFifo_EntryType_eDebugInfo,               /* Info used by BXDM_PPDBG. */

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

typedef struct BXDM_DebugFifo_UnifiedPictureInfo
{
   bool  bPrintAdditional;

} BXDM_DebugFifo_UnifiedPictureInfo;

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

typedef struct BXDM_P_DebugFifo_Entry
{
   BXDM_DebugFifo_Metadata stMetadata;

   union
   {
      BAVC_MFD_Picture  stMFD;
      BXDM_DebugFifo_UnifiedPicture stUniPic;
      BXDM_DebugFifo_UnifiedPictureInfo stUniPicInfo;
      BXDM_DebugFifo_String stString;
      BXDM_DebugFifo_DebugInfo stDebugInfo;

   } data;

} BXDM_P_DebugFifo_Entry;


#if XDM_DEBUG_FIFO
typedef struct BVCE_P_CommandDebug
{
      uint32_t uiCommand;
      char *szCommandParameterName[HOST_CMD_BUFFER_SIZE/sizeof(uint32_t)];
      char *szResponseParameterName[HOST_CMD_BUFFER_SIZE/sizeof(uint32_t)];
      size_t uiCommandSize;
      size_t uiResponseSize;
} BVCE_P_CommandDebug;

typedef enum BVCE_DebugFifo_EntryType
{
   BVCE_DebugFifo_EntryType_eConfig, /* Encode Configuration */
   BVCE_DebugFifo_EntryType_eStatus, /* Encode Status */
   BVCE_DebugFifo_EntryType_eBufferDescriptor, /* Buffer Descriptor */
   BVCE_DebugFifo_EntryType_eMetadataDescriptor, /* Metadata Descriptor */
   BVCE_DebugFifo_EntryType_eITB, /* Raw ITB Descriptor */
   BVCE_DebugFifo_EntryType_eCommand, /* VCE FW Command */
   BVCE_DebugFifo_EntryType_eResponse, /* VCE FW Response */
   BVCE_DebugFifo_EntryType_eTrace0, /* VCE Function Trace 0 */
   BVCE_DebugFifo_EntryType_eTrace1, /* VCE Function Trace 1 */

   /* Add new enums ABOVE this line */
   BVCE_DebugFifo_EntryType_eMax
} BVCE_DebugFifo_EntryType;

typedef struct BVCE_DebugFifo_EntryMetadata
{
   BVCE_DebugFifo_EntryType eType;
   unsigned uiInstance;
   unsigned uiChannel;
   unsigned uiTimestamp; /* (in milliseconds) */
} BVCE_DebugFifo_EntryMetadata;

#define BVCE_P_FUNCTION_TRACE_LENGTH 64
#define BVCE_P_FUNCTION_TRACE_ENTER(_level, _hVce, _uiChannel) BVCE_P_FUNCTION_TRACE(_level, "Enter:", _hVce, _uiChannel);
#define BVCE_P_FUNCTION_TRACE_LEAVE(_level, _hVce, _uiChannel) BVCE_P_FUNCTION_TRACE(_level, "Leave:", _hVce, _uiChannel);
#define BVCE_P_FUNCTION_TRACE(_level, _szPrefix, _hVce, _uiChannel) \
if ( NULL != (_hVce)->stDebugFifo.hDebugFifo )\
{\
   BVCE_P_DebugFifo_Entry *pstEntry;\
   BDBG_Fifo_Token stToken;\
\
   pstEntry = (BVCE_P_DebugFifo_Entry *) BDBG_Fifo_GetBuffer( (_hVce)->stDebugFifo.hDebugFifo, &stToken );\
   if ( NULL != pstEntry )\
   {\
      pstEntry->stMetadata.eType = BVCE_DebugFifo_EntryType_eTrace##_level;\
      pstEntry->stMetadata.uiInstance = ((BVCE_Handle)(_hVce))->stOpenSettings.uiInstance;\
      pstEntry->stMetadata.uiChannel = _uiChannel;\
      pstEntry->stMetadata.uiTimestamp = 0;\
      ( NULL != (_hVce)->hTimer ) ? BTMR_ReadTimer( (_hVce)->hTimer, &pstEntry->stMetadata.uiTimestamp ) : 0;\
      BKNI_Snprintf(pstEntry->data.szFunctionTrace, BVCE_P_FUNCTION_TRACE_LENGTH, "%s%s",_szPrefix,__FUNCTION__);\
      BDBG_Fifo_CommitBuffer( &stToken );\
   }\
}

typedef struct BVCE_P_DebugFifo_Entry
{
   BVCE_DebugFifo_EntryMetadata stMetadata;

   union
   {
      struct
      {
         BVCE_Channel_StartEncodeSettings stStartEncodeSettings;
         BVCE_Channel_EncodeSettings stEncodeSettings;
         BVCE_P_SendCommand_ConfigChannel_Settings stSettingsModifiers;
      } stConfig;
      BVCE_Channel_Status stStatus;
      BVCE_P_Output_ITB_IndexEntry stITBDescriptor;
      BAVC_VideoBufferDescriptor stBufferDescriptor;
      BAVC_VideoMetadataDescriptor stMetadataDescriptor;
      uint8_t auiITB[16];
      BVCE_P_Command stCommand;
      BVCE_P_Response stResponse;
      char szFunctionTrace[BVCE_P_FUNCTION_TRACE_LENGTH];
   } data;
} BVCE_P_DebugFifo_Entry;

#endif

/*
 * SWSTB-1380: end of code
 */

#if BDBG_DEBUG_BUILD && BXDM_DEBUG_FIFO

BERR_Code BXDM_PPDFIFO_P_OutputLog_isr(
   const BXDM_PictureProvider_Handle hXdmPP,
   const BXDM_PictureProvider_P_LocalState *pLocalState,
   const BAVC_MFD_Picture *pMFDPicture
   );

BERR_Code BXDM_PPDFIFO_P_SelectionLog_isr(
   const BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PPDBG_Selection eSelectionInfo
   );

BERR_Code BXDM_PPDFIFO_P_OutputSPOLog_isr(
   const BXDM_PictureProvider_Handle hXdmPP,
   const uint32_t uiOverrideBits
   );

BERR_Code BXDM_PPDFIFO_P_CallbackTriggeredLog_isr(
   const BXDM_PictureProvider_Handle hXdmPP,
   const uint32_t uiCallbackTriggeredBits
   );

BERR_Code BXDM_PPDFIFO_P_StateLog_isr(
   const BXDM_PictureProvider_Handle hXdmPP,
   const uint32_t uiStateBits
   );

BERR_Code BXDM_PPDFIFO_P_State2Log_isr(
   const BXDM_PictureProvider_Handle hXdmPP,
   const uint32_t uiStateBits
   );

BERR_Code BXDM_PPDFIFO_P_StcDeltaLog_isr(
   const BXDM_PictureProvider_Handle hXdmPP,
   const BXDM_PictureProvider_P_LocalState* pLocalState
   );

BERR_Code BXDM_PPDFIFO_P_DecoderDropLog_isr(
   const BXDM_PictureProvider_Handle hXdmPP,
   const uint32_t uiPendingDrop
   );

void BXDM_PPDFIFO_P_QueStartDecode_isr(
   const BXDM_PictureProvider_Handle hXdmPP
   );

void BXDM_PPDFIFO_P_QueStopDecode_isr(
   const BXDM_PictureProvider_Handle hXdmPP
   );

void BXDM_PPDFIFO_P_PrintSelectionModeOverride_isr(
   char *pMsg,
   const BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PictureProvider_P_Picture_Context *pPicture);

void BXDM_PPDFIFO_P_PrintEndSelectionModeOverride_isr(
   const BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PictureProvider_P_Picture_Context *pPicture);

BERR_Code BXDM_PPDFIFO_P_QueDBG_isr(
   const BXDM_PictureProvider_Handle hXdmPP,
   const BXDM_PictureProvider_P_LocalState* pLocalState,
   bool bForcePrint
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

void BXDM_PPDFIFO_P_QueString_isr(
   const BXDM_PictureProvider_Handle hXdmPP,
   const BXDM_Debug_MsgType eMessageType,
   char * format,
   ...
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

#define BXDM_PPDFIFO_P_OutputLog_isr( hXdmPP, pLocalState, pMFDPicture )
#define BXDM_PPDFIFO_P_SelectionLog_isr( hXdmPP, eSelectionInfo )
#define BXDM_PPDFIFO_P_OutputSPOLog_isr( hXdmPP,uiOverrideBits )
#define BXDM_PPDFIFO_P_CallbackTriggeredLog_isr( hXdmPP, uiCallbackTriggeredBits )
#define BXDM_PPDFIFO_P_StateLog_isr( hXdmPP, uiStateBits )
#define BXDM_PPDFIFO_P_State2Log_isr( hXdmPP, uiStateBits )
#define BXDM_PPDFIFO_P_StcDeltaLog_isr( hXdmPP, pLocalState )
#define BXDM_PPDFIFO_P_DecoderDropLog_isr( hXdmPP, uiPendingDrop )

#define BXDM_PPDFIFO_P_QueDBG_isr( hXdmPP, pLocalState, bForcePrint )
#define BXDM_PPDFIFO_P_QueDMConfig_isr( hXdmPP, pLocalState, bLastCall )
#define BXDM_PPDFIFO_P_QueMFD_isr( hXdmPP, pLocalState, pMFDPicture )
#define BXDM_PPDFIFO_P_QueUnifiedPicture_isr( hXdmPP, pLocalState, pstPicture )
/*#define BXDM_PPDFIFO_P_QueString_isr( hXdmPP, pLocalState, format, ... )*/
#define BXDM_PPDFIFO_P_QueString_isr( hXdmPP, pLocalState, format )

#define BXDM_PPDFIFO_P_QueStartDecode_isr( hXdmPP )
#define BXDM_PPDFIFO_P_QueStopDecode_isr( hXdmPP )
#define BXDM_PPDFIFO_P_PrintSelectionModeOverride_isr( pMsg, hXdmPP, pPicture)
#define BXDM_PPDFIFO_P_PrintEndSelectionModeOverride_isr( hXdmPP, pPicture)

#define BXDM_PPDFIFO_P_Fifo_Create( hXdmPP )
#define BXDM_PPDFIFO_P_Fifo_Destroy( hXdmPP )
#define BXDM_PPDFIFO_P_Reader_Create( hXdmPP )
#define BXDM_PPDFIFO_P_Reader_Destroy( hXdmPP )

#endif

#ifdef __cplusplus
}
#endif

#endif /* #ifndef BXDM_PP_DBG_FIFO_PRIV_H__ */
