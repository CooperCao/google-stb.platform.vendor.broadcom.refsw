/***************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

#ifndef bxdm_pp_DBG_H__
#define bxdm_pp_DBG_H__

#include "bxdm_pp.h"
#include "bxdm_pp_priv.h"
#include "bavc.h"
#include "bxdm_pp_dbg_fifo_priv.h"

#ifdef __cplusplus
extern "C" {
#endif

#if 0
}
#endif

#if BDBG_DEBUG_BUILD

BERR_Code BXDM_PPDBG_P_OutputLog_isr(
   const BXDM_PictureProvider_Handle hXdmPP,
   const BXDM_PictureProvider_P_LocalState *pLocalState,
   const BAVC_MFD_Picture *pMFDPicture
   );

BERR_Code BXDM_PPDBG_P_Print_isr(
   const BXDM_PictureProvider_Handle hXdmPP,
   const BXDM_PictureProvider_P_LocalState* pLocalState,
   bool bForcePrint
   );

BERR_Code BXDM_PPDBG_P_SelectionLog_isr(
   const BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PPDBG_Selection eSelectionInfo
   );

BERR_Code BXDM_PPDBG_P_OutputSPOLog_isr(
   const BXDM_PictureProvider_Handle hXdmPP,
   const uint32_t uiOverrideBits
   );

BERR_Code BXDM_PPDBG_P_CallbackTriggeredLog_isr(
   const BXDM_PictureProvider_Handle hXdmPP,
   const uint32_t uiCallbackTriggeredBits
   );

BERR_Code BXDM_PPDBG_P_StateLog_isr(
   const BXDM_PictureProvider_Handle hXdmPP,
   const uint32_t uiStateBits
   );

BERR_Code BXDM_PPDBG_P_State2Log_isr(
   const BXDM_PictureProvider_Handle hXdmPP,
   const uint32_t uiStateBits
   );

BERR_Code BXDM_PPDBG_P_StcDeltaLog_isr(
   const BXDM_PictureProvider_Handle hXdmPP,
   const BXDM_PictureProvider_P_LocalState* pLocalState
   );

BERR_Code BXDM_PPDBG_P_DecoderDropLog_isr(
   const BXDM_PictureProvider_Handle hXdmPP,
   const uint32_t uiPendingDrop
   );

void BXDM_PPDBG_P_PrintStartDecode_isr(
   const BXDM_PictureProvider_Handle hXdmPP
   );

void BXDM_PPDBG_P_PrintStopDecode_isr(
   const BXDM_PictureProvider_Handle hXdmPP
   );

void BXDM_PPDBG_P_PrintSelectionModeOverride_isr(
   char *pMsg,
   const BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PictureProvider_P_Picture_Context *pPicture);

void BXDM_PPDBG_P_PrintEndSelectionModeOverride_isr(
   const BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PictureProvider_P_Picture_Context *pPicture);

void BXDM_PPDBG_P_PrintMFD_isr(
   BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PictureProvider_P_LocalState* pLocalState,
   BAVC_MFD_Picture* pMFDPicture
   );

void BXDM_PPDBG_P_PrintUnifiedPicture_isr(
   BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PictureProvider_P_LocalState * pLocalState,
   BXDM_PictureProvider_P_Picture_Context * pstPicture
   );

void BXDM_PPDBG_P_PrintDMConfig_isr(
   BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PictureProvider_P_LocalState * pLocalState,
   bool bLastCall
   );

#else
/* Non-DEBUG build */

#define BXDM_PPDBG_P_OutputLog_isr( hXdmPP, pLocalState, pMFDPicture )
#define BXDM_PPDBG_P_Print_isr( hXdmPP, pLocalState, bForcePrint )
#define BXDM_PPDBG_P_SelectionLog_isr( hXdmPP, eSelectionInfo )
#define BXDM_PPDBG_P_OutputSPOLog_isr( hXdmPP, uiOverrideBits )
#define BXDM_PPDBG_P_CallbackTriggeredLog_isr( hXdmPP, uiCallbackTriggeredBits )
#define BXDM_PPDBG_P_StateLog_isr( hXdmPP, uiStateBits )
#define BXDM_PPDBG_P_State2Log_isr( hXdmPP, uiStateBits )
#define BXDM_PPDBG_P_StcDeltaLog_isr( hXdmPP, pLocalState )
#define BXDM_PPDBG_P_DecoderDropLog_isr( hXdmPP, uiPendingDrop )
#define BXDM_PPDBG_P_PrintStartDecode_isr( hXdmPP )
#define BXDM_PPDBG_P_PrintStopDecode_isr( hXdmPP )
#define BXDM_PPDBG_P_PrintSelectionModeOverride_isr( pMsg, hXdmPP, pPicture )
#define BXDM_PPDBG_P_PrintEndSelectionModeOverride_isr( hXdmPP, pPicture )
#define BXDM_PPDBG_P_PrintMFD_isr( hXdmPP, pLocalState, pMFDPicture )
#define BXDM_PPDBG_P_PrintUnifiedPicture_isr( hXdmPP, pLocalState, pstPicture )
#define BXDM_PPDBG_P_PrintDMConfig_isr( hXdmPP, pLocalState, bLastCall )

#endif

#ifdef __cplusplus
}
#endif

#endif /* #ifndef bxdm_pp_DBG_H__ */
