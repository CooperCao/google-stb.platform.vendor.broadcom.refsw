/***************************************************************************
 *     Copyright (c) 2003-2010, Broadcom Corporation
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

#ifndef bxdm_pp_CLIP_H__
#define bxdm_pp_CLIP_H__

#ifdef __cplusplus
extern "C" {
#endif

#if 0
}
#endif

void BXDM_PPCLIP_P_Reset_isr(
   BXDM_PictureProvider_Handle hXdmPP
   );

bool BXDM_PPCLIP_P_ClipTimeTSMHandler_isr(
   BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PictureProvider_P_LocalState* pLocalState,
   BXDM_PictureProvider_P_Picture_Context* pstPicture,
   bool bEvaluateActualPts,
   BXDM_PictureProvider_TSMResult *peTsmState
   );

void BXDM_PPCLIP_P_ClipTimeTSMTransitionHandler_isr(
   BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PictureProvider_P_LocalState* pLocalState,
   BXDM_PictureProvider_P_Picture_Context* pstPicture,
   BXDM_PictureProvider_TSMResult *peTsmState
   );

void BXDM_PPCLIP_P_ClipTimeQueueTransitionHandler_isr(
   BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PictureProvider_P_LocalState * pLocalState,
   BXDM_PictureProvider_P_Picture_Context * pstPicCntxt
   );

void BXDM_PPCLIP_P_ClipTimeTrickModeTransitionHandler_isr(
   BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PictureProvider_P_LocalState * pLocalState
   );

void BXDM_PPCLIP_P_ClipTimeCallbackTriggerHandler_isr(
   BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PictureProvider_P_LocalState * pLocalState,
   BXDM_PictureProvider_P_Picture_Context * pstPicture
   );

#ifdef __cplusplus
}
#endif

#endif /* #ifndef bxdm_pp_CLIP_H__ */
