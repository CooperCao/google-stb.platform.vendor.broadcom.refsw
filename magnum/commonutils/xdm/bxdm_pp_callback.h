/***************************************************************************
 *     Copyright (c) 2003-2012, Broadcom Corporation
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
         BXDM_PictureProvider_Callback_StcPtsOffset_isr fCallback,
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
         BXDM_PictureProvider_Callback_FirstPTSReady_isr fCallback,
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
         BXDM_PictureProvider_Callback_FirstCodedPTSReady_isr fCallback,
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
         BXDM_PictureProvider_Callback_FirstPTSPassed_isr fCallback,
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
         BXDM_PictureProvider_Callback_PTSError_isr fCallback,
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
         BXDM_PictureProvider_Callback_IFrame_isr fCallback,
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
         BXDM_PictureProvider_Callback_PictureParameters_isr fCallback,
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
         BXDM_PictureProvider_Callback_TSMPassInASTMMode_isr fCallback,
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
         BXDM_PictureProvider_Callback_ClipStart_isr fCallback,
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
         BXDM_PictureProvider_Callback_ClipStop_isr fCallback,
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
         BXDM_PictureProvider_Callback_PictureMarker_isr fCallback,
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
         BXDM_PictureProvider_Callback_RequestSTC_isr fCallback,
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
         BXDM_PictureProvider_Callback_PictureUnderEvaluation_isr fCallback,
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
         BXDM_PictureProvider_Callback_TSMResult_isr fCallback,
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
         BXDM_PictureProvider_Callback_PictureExtensionData_isr fCallback,
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
         BXDM_PictureProvider_Callback_DecodeError_isr fCallback,
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
         BXDM_PictureProvider_Callback_ChunkDone_isr fCallback,
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
