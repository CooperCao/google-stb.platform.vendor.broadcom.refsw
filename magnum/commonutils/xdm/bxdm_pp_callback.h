/***************************************************************************
 * Copyright (C) 2003-2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 ***************************************************************************/

#ifndef BXDM_PP_CALLBACK_H_
#define BXDM_PP_CALLBACK_H_

#ifdef __cplusplus
extern "C" {
#endif

#if 0
}
#endif

/*************/
/* Callbacks */
/*************/

typedef enum BXDM_PictureProvider_Callback
{
      BXDM_PictureProvider_Callback_eStcPtsOffset,            /* STC offset changed interrupt */
      BXDM_PictureProvider_Callback_eFirstPTSReady,           /* First Picture Ready */
      BXDM_PictureProvider_Callback_eFirstCodedPTSReady,      /* First Picture w/ Coded PTS Ready */
      BXDM_PictureProvider_Callback_eFirstPTSPassed,          /* First PTS Passed */
      BXDM_PictureProvider_Callback_ePTSError,                /* PTS mismatch */
      BXDM_PictureProvider_Callback_eIFrame,                  /* I-Frame detect event occured notification */
      BXDM_PictureProvider_Callback_ePictureParameters,       /* Called when first picture is queued to the display manager from
                                                                 the firmware to inform application of size, framerate, etc. */
      BXDM_PictureProvider_Callback_eTSMPassInASTMMode,       /* Called when DM is in
                                                               * vsync mode, configured
                                                               * for ASTM mode, and DM
                                                               * sees a TSM pass */
      BXDM_PictureProvider_Callback_eClipStart,               /* ClipStart event occured notification */
      BXDM_PictureProvider_Callback_eClipStop,                /* ClipStop event occured notification */
      BXDM_PictureProvider_Callback_ePictureMarker,           /* PictureMarker event occurred notification */
      BXDM_PictureProvider_Callback_eRequestSTC,              /* Called when DM needs the application to provide a valid STC before proceeding with TSM calculations */

      BXDM_PictureProvider_Callback_ePictureUnderEvaluation,           /* Called when a PPB is first evaluated for display */

      BXDM_PictureProvider_Callback_eTSMResult,               /* Called for each TSM evaluation before a picture is displayed */

      BXDM_PictureProvider_Callback_ePictureExtensionData,    /* Generic data callback mechanism, called when a picture has extension data. */

      BXDM_PictureProvider_Callback_eDecodeError,             /* Called when the decode error bit is set in the picture */

      BXDM_PictureProvider_Callback_eChunkDone,               /* SW7425-3358: FNRT, executed when the last picture of a
                                                                 chunk is sent to VDC */

      BXDM_PictureProvider_Callback_eMax                      /* Not a Real interrupt just the max no of XVD interrupts */
} BXDM_PictureProvider_Callback;

BERR_Code
BXDM_PictureProvider_Callback_SetEnable_isr(
         BXDM_PictureProvider_Handle hXdmPP,
         BXDM_PictureProvider_Callback,
         bool bEnable
         );

#if !B_REFSW_MINIMAL /* SWSTB-461 */
BERR_Code
BXDM_PictureProvider_Callback_GetEnable_isr(
         BXDM_PictureProvider_Handle hXdmPP,
         BXDM_PictureProvider_Callback,
         bool *pbEnable
         );
#endif

/* STC PTS Offset */
typedef void (*BXDM_PictureProvider_Callback_StcPtsOffset_isr)(
         void *pPrivateContext,
         int32_t iPrivateParam,
         const uint32_t *puiStcPtsOffset );

BERR_Code
BXDM_PictureProvider_Callback_Install_StcPtsOffset_isr(
         BXDM_PictureProvider_Handle hXdmPP,
         BXDM_PictureProvider_Callback_StcPtsOffset_isr fCallback_isr,
         void *pPrivateContext,
         int32_t iPrivateParam
         );

BERR_Code
BXDM_PictureProvider_Callback_UnInstall_StcPtsOffset_isr(
         BXDM_PictureProvider_Handle hXdmPP
         );

/* First PTS Ready */
typedef void (*BXDM_PictureProvider_Callback_FirstPTSReady_isr)(
         void *pPrivateContext,
         int32_t iPrivateParam,
         const BXDM_PictureProvider_PTSInfo *pstPTSInfo );

BERR_Code
BXDM_PictureProvider_Callback_Install_FirstPTSReady_isr(
         BXDM_PictureProvider_Handle hXdmPP,
         BXDM_PictureProvider_Callback_FirstPTSReady_isr fCallback_isr,
         void *pPrivateContext,
         int32_t iPrivateParam
         );

BERR_Code
BXDM_PictureProvider_Callback_UnInstall_FirstPTSReady_isr(
         BXDM_PictureProvider_Handle hXdmPP
         );

BERR_Code
BXDM_PictureProvider_Callback_UnInstall_FirstCodedPTSReady_isr(
         BXDM_PictureProvider_Handle hXdmPP
         );

/* First Coded PTS Ready */
typedef void (*BXDM_PictureProvider_Callback_FirstCodedPTSReady_isr)(
         void *pPrivateContext,
         int32_t iPrivateParam,
         const BXDM_PictureProvider_PTSInfo *pstPTSInfo );

BERR_Code
BXDM_PictureProvider_Callback_Install_FirstCodedPTSReady_isr(
         BXDM_PictureProvider_Handle hXdmPP,
         BXDM_PictureProvider_Callback_FirstCodedPTSReady_isr fCallback_isr,
         void *pPrivateContext,
         int32_t iPrivateParam
         );

/* First PTS Passed */
typedef void (*BXDM_PictureProvider_Callback_FirstPTSPassed_isr)(
         void *pPrivateContext,
         int32_t iPrivateParam,
         const BXDM_PictureProvider_PTSInfo *pstPTSInfo );

BERR_Code
BXDM_PictureProvider_Callback_Install_FirstPTSPassed_isr(
         BXDM_PictureProvider_Handle hXdmPP,
         BXDM_PictureProvider_Callback_FirstPTSPassed_isr fCallback_isr,
         void *pPrivateContext,
         int32_t iPrivateParam
         );

BERR_Code
BXDM_PictureProvider_Callback_UnInstall_FirstPTSPassed_isr(
         BXDM_PictureProvider_Handle hXdmPP
         );

/* PTS  Error */
typedef void (*BXDM_PictureProvider_Callback_PTSError_isr)(
         void *pPrivateContext,
         int32_t iPrivateParam,
         const BXDM_PictureProvider_PTSInfo *pstPTSInfo );

BERR_Code
BXDM_PictureProvider_Callback_Install_PTSError_isr(
         BXDM_PictureProvider_Handle hXdmPP,
         BXDM_PictureProvider_Callback_PTSError_isr fCallback_isr,
         void *pPrivateContext,
         int32_t iPrivateParam
         );

BERR_Code
BXDM_PictureProvider_Callback_UnInstall_PTSError_isr(
         BXDM_PictureProvider_Handle hXdmPP
         );

/* I Frame */
typedef void (*BXDM_PictureProvider_Callback_IFrame_isr)(
         void *pPrivateContext,
         int32_t iPrivateParam,
         const bool *pbFound );

BERR_Code
BXDM_PictureProvider_Callback_Install_IFrame_isr(
         BXDM_PictureProvider_Handle hXdmPP,
         BXDM_PictureProvider_Callback_IFrame_isr fCallback_isr,
         void *pPrivateContext,
         int32_t iPrivateParam
         );

BERR_Code
BXDM_PictureProvider_Callback_UnInstall_IFrame_isr(
         BXDM_PictureProvider_Handle hXdmPP
         );

typedef struct BXDM_PictureProvider_Callback_PictureParameterInfo
{
   const BXDM_Picture *pstUnifiedPicture;
   const BAVC_MFD_Picture *pstMFDPicture;
} BXDM_PictureProvider_Callback_PictureParameterInfo;

typedef void (*BXDM_PictureProvider_Callback_PictureParameters_isr)(
         void *pPrivateContext,
         int32_t iPrivateParam,
         const BXDM_PictureProvider_Callback_PictureParameterInfo *pstPictureParameterInfo );

BERR_Code
BXDM_PictureProvider_Callback_Install_PictureParameters_isr(
         BXDM_PictureProvider_Handle hXdmPP,
         BXDM_PictureProvider_Callback_PictureParameters_isr fCallback_isr,
         void *pPrivateContext,
         int32_t iPrivateParam
         );

BERR_Code
BXDM_PictureProvider_Callback_UnInstall_PictureParameters_isr(
         BXDM_PictureProvider_Handle hXdmPP
         );

typedef void (*BXDM_PictureProvider_Callback_TSMPassInASTMMode_isr)(
         void *pPrivateContext,
         int32_t iPrivateParam,
         const BXDM_PictureProvider_PTSInfo *pstPTSInfo );

BERR_Code
BXDM_PictureProvider_Callback_Install_TSMPassInASTMMode_isr(
         BXDM_PictureProvider_Handle hXdmPP,
         BXDM_PictureProvider_Callback_TSMPassInASTMMode_isr fCallback_isr,
         void *pPrivateContext,
         int32_t iPrivateParam
         );

BERR_Code
BXDM_PictureProvider_Callback_UnInstall_TSMPassInASTMMode_isr(
         BXDM_PictureProvider_Handle hXdmPP
         );

typedef struct BXDM_PictureProvider_Callback_ClipEventInfo
{
      uint32_t uiPTS;
      uint32_t uiClipId;
} BXDM_PictureProvider_Callback_ClipEventInfo;

typedef void (*BXDM_PictureProvider_Callback_ClipStart_isr)(
         void *pPrivateContext,
         int32_t iPrivateParam,
         const BXDM_PictureProvider_Callback_ClipEventInfo *pstClipEventInfo
         );

BERR_Code
BXDM_PictureProvider_Callback_Install_ClipStart_isr(
         BXDM_PictureProvider_Handle hXdmPP,
         BXDM_PictureProvider_Callback_ClipStart_isr fCallback_isr,
         void *pPrivateContext,
         int32_t iPrivateParam
         );

BERR_Code
BXDM_PictureProvider_Callback_UnInstall_ClipStart_isr(
         BXDM_PictureProvider_Handle hXdmPP
         );

typedef void (*BXDM_PictureProvider_Callback_ClipStop_isr)(
         void *pPrivateContext,
         int32_t iPrivateParam,
         const BXDM_PictureProvider_Callback_ClipEventInfo *pstClipEventInfo );

BERR_Code
BXDM_PictureProvider_Callback_Install_ClipStop_isr(
         BXDM_PictureProvider_Handle hXdmPP,
         BXDM_PictureProvider_Callback_ClipStop_isr fCallback_isr,
         void *pPrivateContext,
         int32_t iPrivateParam
         );

BERR_Code
BXDM_PictureProvider_Callback_UnInstall_ClipStop_isr(
         BXDM_PictureProvider_Handle hXdmPP
         );

typedef void (*BXDM_PictureProvider_Callback_PictureMarker_isr)(
         void *pPrivateContext,
         int32_t iPrivateParam,
         const BXDM_PictureProvider_Callback_ClipEventInfo *pstClipEventInfo );

BERR_Code
BXDM_PictureProvider_Callback_Install_PictureMarker_isr(
         BXDM_PictureProvider_Handle hXdmPP,
         BXDM_PictureProvider_Callback_PictureMarker_isr fCallback_isr,
         void *pPrivateContext,
         int32_t iPrivateParam
         );

BERR_Code
BXDM_PictureProvider_Callback_UnInstall_PictureMarker_isr(
         BXDM_PictureProvider_Handle hXdmPP
         );

typedef void (*BXDM_PictureProvider_Callback_RequestSTC_isr)(
         void *pPrivateContext,
         int32_t iPrivateParam,
         const BXDM_PictureProvider_PTSInfo *pstPTSInfo  );

BERR_Code
BXDM_PictureProvider_Callback_Install_RequestSTC_isr(
         BXDM_PictureProvider_Handle hXdmPP,
         BXDM_PictureProvider_Callback_RequestSTC_isr fCallback_isr,
         void *pPrivateContext,
         int32_t iPrivateParam
         );

BERR_Code
BXDM_PictureProvider_Callback_UnInstall_RequestSTC_isr(
         BXDM_PictureProvider_Handle hXdmPP
         );

typedef void (*BXDM_PictureProvider_Callback_PictureUnderEvaluation_isr)(
         void *pPrivateContext,
         int32_t iPrivateParam,
         const BXDM_Picture *pstPicture );

BERR_Code
BXDM_PictureProvider_Callback_Install_PictureUnderEvaluation_isr(
         BXDM_PictureProvider_Handle hXdmPP,
         BXDM_PictureProvider_Callback_PictureUnderEvaluation_isr fCallback_isr,
         void *pPrivateContext,
         int32_t iPrivateParam
         );

BERR_Code
BXDM_PictureProvider_Callback_UnInstall_PictureUnderEvaluation_isr(
         BXDM_PictureProvider_Handle hXdmPP
         );

typedef void (*BXDM_PictureProvider_Callback_TSMResult_isr)(
         void *pPrivateContext,
         int32_t iPrivateParam,
         BXDM_PictureProvider_TSMInfo *pstTSMInfo );

BERR_Code
BXDM_PictureProvider_Callback_Install_TSMResult_isr(
         BXDM_PictureProvider_Handle hXdmPP,
         BXDM_PictureProvider_Callback_TSMResult_isr fCallback_isr,
         void *pPrivateContext,
         int32_t iPrivateParam
         );

BERR_Code
BXDM_PictureProvider_Callback_UnInstall_TSMResult_isr(
         BXDM_PictureProvider_Handle hXdmPP
         );

typedef void (*BXDM_PictureProvider_Callback_PictureExtensionData_isr)(
         void *pPrivateContext,
         int32_t iPrivateParam,
         const BXDM_Picture_ExtensionInfo *pstExtensionInfo );

BERR_Code
BXDM_PictureProvider_Callback_Install_PictureExtensionData_isr(
         BXDM_PictureProvider_Handle hXdmPP,
         BXDM_PictureProvider_Callback_PictureExtensionData_isr fCallback_isr,
         void *pPrivateContext,
         int32_t iPrivateParam
         );

BERR_Code
BXDM_PictureProvider_Callback_UnInstall_PictureExtensionData_isr(
         BXDM_PictureProvider_Handle hXdmPP
         );

typedef struct BXDM_PictureProvider_Callback_DecodeErrorInfo
{
      uint32_t uiDecodeErrorCount;
} BXDM_PictureProvider_Callback_DecodeErrorInfo;

typedef void (*BXDM_PictureProvider_Callback_DecodeError_isr)(
         void *pPrivateContext,
         int32_t iPrivateParam,
         const BXDM_PictureProvider_Callback_DecodeErrorInfo *pstDecodeErrorInfo );

BERR_Code
BXDM_PictureProvider_Callback_Install_DecodeError_isr(
         BXDM_PictureProvider_Handle hXdmPP,
         BXDM_PictureProvider_Callback_DecodeError_isr fCallback_isr,
         void *pPrivateContext,
         int32_t iPrivateParam
         );

BERR_Code
BXDM_PictureProvider_Callback_UnInstall_DecodeError_isr(
         BXDM_PictureProvider_Handle hXdmPP
         );

/* SW7425-3358: FNRT support, callback is executed on the same vsync
 * that the last picture of a chunk is delivered to VDC.
 */
typedef void (*BXDM_PictureProvider_Callback_ChunkDone_isr)(
         void *pPrivateContext,
         int32_t iPrivateParam,
         const BXDM_PictureProvider_Callback_ChunkDoneInfo *pstChunkDoneInfo );

BERR_Code
BXDM_PictureProvider_Callback_Install_ChunkDone_isr(
         BXDM_PictureProvider_Handle hXdmPP,
         BXDM_PictureProvider_Callback_ChunkDone_isr fCallback_isr,
         void *pPrivateContext,
         int32_t iPrivateParam
         );

BERR_Code
BXDM_PictureProvider_Callback_UnInstall_ChunkDone_isr(
         BXDM_PictureProvider_Handle hXdmPP
         );


#ifdef __cplusplus
}
#endif

#endif /* BXDM_PP_CALLBACK_H_ */
