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

#ifndef bxdm_pp_QM_H__
#define bxdm_pp_QM_H__

#include "bxdm_pp.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
** "public" queue management funcrtions.
*/
bool BXDM_PPQM_P_PeekAtNextPicture_isr(
   BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PictureProvider_P_LocalState * pLocalState,
   BXDM_PictureProvider_P_Picture_Context * pstNextPicCntxt,
   BXDM_PictureProvider_P_Picture_Context * pstPrevPicCntxt,
   BXDM_PictureProvider_P_Picture_Context * pstSelectedPicCntxt
   );

void BXDM_PPQM_P_GetNextPicture_isr(
   BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PictureProvider_P_LocalState * pLocalState,
   BXDM_PictureProvider_P_Picture_Context * pstNextPicCntxt
   );

void BXDM_PPQM_P_PromotePictureToDisplayStatus_isr(
   BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PictureProvider_P_LocalState * pLocalState,
   BXDM_PictureProvider_P_Picture_Context * pstPictureUnderEvaluation
   );

void BXDM_PPQM_P_ReleasePicture_isr(
   BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PictureProvider_P_Picture_Context * pstPicCntxt
   );

void BXDM_PPQM_P_ReleasePictureExt_isr(
   BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PictureProvider_P_LocalState* pLocalState,
   BXDM_PictureProvider_P_Picture_Context * pstPicCntxt
   );

void BXDM_PPQM_P_InvalidatePictureContext_isr(
   BXDM_PictureProvider_P_Picture_Context * pstPicCntxt
   );

void BXDM_PPQM_P_GetHwPcrOffset_isr(
   BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PictureProvider_P_Picture_Context * pstPicture,
   uint32_t * puiPcrOffset
   );

void BXDM_PPQM_P_GetSoftwarePcrOffset_isr(
   BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PictureProvider_P_Picture_Context * pstPicture,
   uint32_t * puiPcrOffset
   );

void BXDM_PPQM_P_GetCookedPcrOffset_isr(
   BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PictureProvider_P_Picture_Context * pstPicture,
   uint32_t * puiPcrOffset
   );

void BXDM_PPQM_P_GetPtsOffset_isr(
   BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PictureProvider_P_Picture_Context * pstPicture,
   uint32_t * puiPtsOffset
   );

void BXDM_PPQM_P_SetPtsWithFrac_isr(
   BXDM_PictureProvider_P_Picture_Context * pstPicture,
   BXDM_PictureProvider_P_PTSIndex ePTSIndex,
   uint32_t    uiIndex,
   BXDM_PPFP_P_DataType stPTS
   );

void BXDM_PPQM_P_GetPtsWithFrac_isr(
   BXDM_PictureProvider_P_Picture_Context * pstPicture,
   BXDM_PictureProvider_P_PTSIndex ePTSIndex,
   uint32_t    uiIndex,
   BXDM_PPFP_P_DataType *pstPTS
   );

void BXDM_PPQM_P_GetPts_isr(
   BXDM_PictureProvider_P_Picture_Context * pstPicture,
   BXDM_PictureProvider_P_PTSIndex ePTSIndex,
   uint32_t    uiIndex,
   uint32_t * puiPts
   );

void BXDM_PPQM_P_GetPtsUnfiltered_isr(
   BXDM_PictureProvider_P_Picture_Context * pstPicture,
   BXDM_PictureProvider_P_PTSIndex ePTSIndex,
   uint32_t    uiIndex,
   uint32_t * puiPts
   );

#if !B_REFSW_MINIMAL /* SWSTB-461 */
void BXDM_PPQM_P_GetLastPts_isr(
   BXDM_PictureProvider_P_Picture_Context * pstPicture,
   BXDM_PictureProvider_P_PTSIndex ePTSIndex,
   uint32_t * puiPts
   );
#endif

void BXDM_PPQM_P_GetPredictedPts_isr(
   BXDM_PictureProvider_P_Picture_Context * pstPicture,
   BXDM_PictureProvider_P_PTSIndex ePTSIndex,
   uint32_t * puiPts
   );

void BXDM_PPQM_P_GetPredictedPtsWithFrac_isr(
   BXDM_PictureProvider_P_Picture_Context * pstPicture,
   BXDM_PictureProvider_P_PTSIndex ePTSIndex,
   BXDM_PPFP_P_DataType *pstPTS
   );

void BXDM_PPQM_P_SetPredictedPtsWithFrac_isr(
   BXDM_PictureProvider_P_Picture_Context * pstPicture,
   BXDM_PictureProvider_P_PTSIndex ePTSIndex,
   BXDM_PPFP_P_DataType stPTS
   );

void BXDM_PPQM_P_GetPtsType_isr(
   BXDM_PictureProvider_P_Picture_Context * pstPicture,
   BXDM_PictureProvider_PTSType * pePtsType
   );

void BXDM_PPQM_P_GetPtsTypeUnfiltered_isr(
   BXDM_PictureProvider_P_Picture_Context * pstPicture,
   BXDM_PictureProvider_PTSType * pePtsType
   );

void BXDM_PPQM_P_SetElementPolarity_isr(
   BXDM_PictureProvider_P_Picture_Context *pstPicture,
   uint32_t uiElementIndex,
   BAVC_Polarity ePolarity);

void BXDM_PPQM_P_GetElementPolarity_isr(
   BXDM_PictureProvider_P_Picture_Context *pstPicture,
   uint32_t uiElementIndex,
   BAVC_Polarity *pePolarity);

BXDM_PictureProvider_P_Picture_Context * BXDM_PPQM_P_GetFirstPictureContext_isr(
   BXDM_PictureProvider_Handle hXdmPP
   );

/* SW7445-586: see the comment in the bxvd_pp_qm.c */
void BXDM_PPQM_P_GetPicturePulldown_isr(
   const BXDM_PictureProvider_P_Picture_Context *pstPicture,
   BXDM_Picture_PullDown * pePulldown
   );

void BXDM_PPQM_P_AppendField_isr(
   BXDM_PictureProvider_P_LocalState * pLocalState,
   BXDM_PictureProvider_P_Picture_Context *pstBasePicture,
   BXDM_PictureProvider_P_Picture_Context *pstBonusPicture
   );

void BXDM_PPQM_P_GetPictureTopBuffer_isr(
   const BXDM_PictureProvider_P_Picture_Context *pstPicture,
   const BXDM_Picture_BufferInfo ** pstBufferInfo
   );

void BXDM_PPQM_P_GetPictureBottomBuffer_isr(
   const BXDM_PictureProvider_P_Picture_Context *pstPicture,
   const BXDM_Picture_BufferInfo ** pstBufferInfo
   );

#ifdef __cplusplus
}
#endif

#endif /* #ifndef bxdm_pp_QM_H__ */

