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

#ifndef BXDM_PP_DBG_FIFO_H__
#define BXDM_PP_DBG_FIFO_H__

#include "bxdm_pp.h"
#include "bxdm_pp_priv.h"
#include "bxdm_pp_dbg_fifo_priv.h"
#include "bavc.h"

#ifdef __cplusplus
extern "C" {
#endif

#if 0 /* needed by some editors */
}
#endif

#if BDBG_DEBUG_BUILD

/*
 * The routines for pulling data off the debug fifo.
 */
void BXDM_PictureProvider_Debug_DumpFifo_isrsafe(
   BXDM_PictureProvider_Handle hXdmPP
   );

void BXDM_PictureProvider_Debug_PrintFifoEntry_isrsafe(
   BXDM_P_DebugFifo_Entry * pstEntry
   );

int BXDM_PictureProvider_Debug_FormatMsg_MFD1_isrsafe(
   const BXDM_P_DebugFifo_Entry * pstEntry,
   char *szMessage,
   size_t uiSize
   );

int BXDM_PictureProvider_Debug_FormatMsg_MFD2_isrsafe(
   const BXDM_P_DebugFifo_Entry * pstEntry,
   char *szMessage,
   size_t uiSize
   );

int BXDM_PictureProvider_Debug_FormatMsg_MFD3_isrsafe(
   const BXDM_P_DebugFifo_Entry * pstEntry,
   char *szMessage,
   size_t uiSize
   );

int BXDM_PictureProvider_Debug_FormatMsg_PPQM_isrsafe(
   const BXDM_P_DebugFifo_Entry * pstEntry,
   char *szMessage,
   size_t uiSize
   );

int BXDM_PictureProvider_Debug_FormatMsg_PPQM_Bonus_isrsafe(
   const BXDM_P_DebugFifo_Entry * pstEntry,
   char *szMessage,
   size_t uiSize
   );

int BXDM_PictureProvider_Debug_FormatMsg_PPDBG_isrsafe(
   const BXDM_P_DebugFifo_Entry * pstEntry,
   char *szMessage,
   size_t uiSize
   );

int BXDM_PictureProvider_Debug_FormatMsg_Config1_isrsafe(
   const BXDM_P_DebugFifo_Entry * pstEntry,
   char *szMessage,
   size_t uiSize
   );

int BXDM_PictureProvider_Debug_FormatMsg_Config2_isrsafe(
   const BXDM_P_DebugFifo_Entry * pstEntry,
   char *szMessage,
   size_t uiSize
   );

int BXDM_PictureProvider_Debug_FormatMsg_Config3_isrsafe(
   const BXDM_P_DebugFifo_Entry * pstEntry,
   char *szMessage,
   size_t uiSize
   );

int BXDM_PictureProvider_Debug_FormatMsg_Config4_isrsafe(
   const BXDM_P_DebugFifo_Entry * pstEntry,
   char *szMessage,
   size_t uiSize
   );

#else  /* Non-DEBUG build */

#define BXDM_PictureProvider_Debug_DumpFifo_isrsafe( hXdmPP )
#define BXDM_PictureProvider_Debug_PrintFifoEntry_isrsafe( pstEntry )
#define BXDM_PictureProvider_Debug_FormatMsg_PPDBG_isrsafe( pstEntry, szMessage, uiSize )
#define BXDM_PictureProvider_Debug_FormatMsg_MFD1_isrsafe( pstEntry, szMessage, uiSize )
#define BXDM_PictureProvider_Debug_FormatMsg_MFD2_isrsafe( pstEntry, szMessage, uiSize )
#define BXDM_PictureProvider_Debug_FormatMsg_MFD3_isrsafe( pstEntry, szMessage, uiSize )
#define BXDM_PictureProvider_Debug_FormatMsg_PPQM_isrsafe( pstEntry, szMessage, uiSize )
#define BXDM_PictureProvider_Debug_FormatMsg_PPQM_Bonus_isrsafe( pstEntry, szMessage, uiSize )
#define BXDM_PictureProvider_Debug_FormatMsg_Config1_isrsafe( pstEntry, szMessage, uiSize )
#define BXDM_PictureProvider_Debug_FormatMsg_Config2_isrsafe( pstEntry, szMessage, uiSize )
#define BXDM_PictureProvider_Debug_FormatMsg_Config3_isrsafe( pstEntry, szMessage, uiSize )
#define BXDM_PictureProvider_Debug_FormatMsg_Config4_isrsafe( pstEntry, szMessage, uiSize )

#endif

#ifdef __cplusplus
}
#endif

#endif /* #ifndef BXDM_PP_DBG_FIFO_H__ */
