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

#ifndef bxdm_pp_VTSM_H__
#define bxdm_pp_VTSM_H__

#include "bxdm_pp.h"


#ifdef __cplusplus
extern "C" {
#endif

#if 0
}
#endif

typedef struct BXDM_PPVTSM_P_State
{
   BXDM_PPFP_P_DataType stVirtualSTC;
   bool bVirtualPTSInitialized;
   bool bTrickModeTransition;
   BXDM_PictureProvider_FrameAdvanceMode eFrameAdvanceMode;
} BXDM_PPVTSM_P_State;

void BXDM_PPVTSM_P_VirtualStcSet_isr(
   BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PictureProvider_P_LocalState* pLocalState
   );

void BXDM_PPVTSM_P_VirtualStcIncrement_isr(
   BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PictureProvider_P_LocalState* pLocalState
   );

void BXDM_PPVTSM_P_VirtualPtsInterpolate_isr(
   BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PictureProvider_P_LocalState* pLocalState,
   BXDM_PictureProvider_P_Picture_Context* pstPicture,
   BXDM_PictureProvider_P_Picture_Context* pstPrevPicture,
   BXDM_PictureProvider_P_Picture_Context* pstSelectedPicture
   );

void BXDM_PPVTSM_P_VirtualStcGet_isr(
   const BXDM_PictureProvider_Handle hXdmPP,
   uint32_t* puiStc
   );

void BXDM_PPVTSM_P_ClipTimeTrickModeTransitionHandler_isr(
   BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PictureProvider_P_LocalState * pLocalState
   );

#ifdef __cplusplus
}
#endif

#endif /* #ifndef bxdm_pp_VTSM_H__ */
