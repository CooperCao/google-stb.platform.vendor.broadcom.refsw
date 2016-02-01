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

#ifndef bxdm_pp_CALLBACK_H__
#define bxdm_pp_CALLBACK_H__

#ifdef __cplusplus
extern "C" {
#endif

void BXDM_PPCB_P_EnableAutomaticCallbacks_isr(
   BXDM_PictureProvider_Handle hXdmPP
   );

void BXDM_PPCB_P_ExecuteSingleCallback_isr(
   BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PictureProvider_P_LocalState* pLocalState,
   BXDM_PictureProvider_Callback eCallbackName
   );

void BXDM_PPCB_P_ExecuteSingleCallbackExt_isr(
   BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PictureProvider_P_LocalState* pLocalState,
   BXDM_PictureProvider_P_Picture_Context * pstPicture,
   BXDM_PictureProvider_Callback eCallbackName
   );

void BXDM_PPCB_P_ExecuteCallbacks_isr(
   BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PictureProvider_P_LocalState* pLocalState
   );

void BXDM_PPCB_P_EvaluateCriteria_IFrame_isr(
   BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PictureProvider_P_LocalState* pLocalState,
   BXDM_PictureProvider_P_Picture_Context * pstSelectPic
   );

#ifdef __cplusplus
}
#endif

#endif /* #ifndef bxdm_pp_CALLBACK_H__ */

