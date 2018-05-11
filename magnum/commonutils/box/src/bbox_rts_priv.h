/******************************************************************************
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
* Module Description:
*
******************************************************************************/
#ifndef BBOX_RTS_PRIV_H__
#define BBOX_RTS_PRIV_H__

#include "bstd.h"
#include "berr_ids.h"    /* Error codes */
#include "bavc_types.h"
#include "bbox.h"

#ifdef __cplusplus
extern "C" {
#endif


void BBOX_P_SetDefaultMemConfig
    ( BBOX_MemConfig     *pMemConfig );

void BBOX_P_SetRdcMemc
    ( BBOX_MemConfig     *pBoxMemConfig,
      BBOX_MemcIndex      eMemcIndex );

void BBOX_P_SetDviCfcMemc
    ( BBOX_MemConfig     *pBoxMemConfig,
      BBOX_MemcIndex      eMemcIndex );

void BBOX_P_SetVideoWinMemc
    ( BBOX_MemConfig     *pBoxMemConfig,
      BBOX_Vdc_DisplayId  display,
      BBOX_Vdc_WindowId   window,
      BBOX_MemcIndex      eWinMemcIndex,
      BBOX_MemcIndex      eDeinterlacerMemcIndex );

void BBOX_P_SetGfxWinMemc
    ( BBOX_MemConfig     *pBoxMemConfig,
      BBOX_Vdc_DisplayId  display,
      BBOX_Vdc_WindowId   window,
      BBOX_MemcIndex      eGfdMemcIndex );

void BBOX_P_SetHdrVideoAndGfxMemc
    ( BBOX_MemConfig     *pBoxMemConfig,
      BBOX_Vdc_DisplayId  display,
      BBOX_MemcIndex      eCmpCfcMemcIndex,
      BBOX_MemcIndex      eGfdCfcMemcIndex );

void BBOX_P_SetNumMemc
    ( BBOX_MemConfig     *pBoxMemConfig,
      uint32_t            ulNumMemc );

void BBOX_P_SetDramRefreshRate
    ( BBOX_MemConfig          *pBoxMemConfig,
      BBOX_DramRefreshRate     eRefreshRate );



#define BBOX_P_SET_LEGACY_RDC_MEMC( memconfig )  BBOX_P_SetRdcMemc( \
    memconfig, \
    0 )

#define BBOX_P_SET_LEGACY_DVI_CFC_MEMC( memconfig ) BBOX_P_SetDviCfcMemc( \
    memconfig, \
    BBOX_MemcIndex_Invalid )

#define BBOX_P_SET_LEGACY_VIDEO_WIN_MEMC( memconfig, display, window, idx, deinterlacer_idx )  BBOX_P_SetVideoWinMemc( \
    memconfig, \
    BBOX_Vdc_Display_e##display, \
    BBOX_Vdc_Window_e##window, \
    idx, \
    deinterlacer_idx )

#define BBOX_P_SET_LEGACY_HDR_VIDEO_AND_GFX_MEMC( memconfig, display )   BBOX_P_SetHdrVideoAndGfxMemc( \
    memconfig, \
    BBOX_Vdc_Display_e##display, \
    BBOX_MemcIndex_Invalid, \
    BBOX_MemcIndex_Invalid )

#define BBOX_P_SET_LEGACY_NUM_MEMC( memconfig )  BBOX_P_SetNumMemc( \
    memconfig, \
    1 )

#define BBOX_P_SET_LEGACY_DRAM_REFRESH_RATE( memconfig )  BBOX_P_SetDramRefreshRate( \
    memconfig, \
    BBOX_DramRefreshRate_e1x )

#define BBOX_P_SET_RDC_MEMC( memconfig, idx ) BBOX_P_SetRdcMemc( \
    memconfig, \
    BBOX_MemcIndex_##idx )

#define BBOX_P_SET_DVI_CFC_MEMC( memconfig, idx )  BBOX_P_SetDviCfcMemc( \
    memconfig, \
    BBOX_MemcIndex_##idx )

#define BBOX_P_SET_VIDEO_WIN_MEMC( memconfig, display, window, idx, deinterlacer_idx )  BBOX_P_SetVideoWinMemc( \
    memconfig, \
    BBOX_Vdc_Display_e##display, \
    BBOX_Vdc_Window_e##window, \
    BBOX_MemcIndex_##idx, \
    BBOX_MemcIndex_##deinterlacer_idx )

#define BBOX_P_SET_GFX_WIN_MEMC( memconfig, display, window, idx )  BBOX_P_SetGfxWinMemc( \
    memconfig, \
    BBOX_Vdc_Display_e##display, \
    BBOX_Vdc_Window_e##window, \
    BBOX_MemcIndex_##idx )

#define BBOX_P_SET_HDR_VIDEO_AND_GFX_MEMC( memconfig, display, video_index, gfx_index )  BBOX_P_SetHdrVideoAndGfxMemc( \
    memconfig, \
    BBOX_Vdc_Display_e##display, \
    BBOX_MemcIndex_##video_index, \
    BBOX_MemcIndex_##gfx_index )

#define BBOX_P_SET_NUM_MEMC( memconfig, num_memc ) BBOX_P_SetNumMemc( \
    memconfig, \
    num_memc )

#define BBOX_P_SET_DRAM_REFRESH_RATE( memconfig, rate )  BBOX_P_SetDramRefreshRate( \
    memconfig, \
    BBOX_DramRefreshRate_e##rate )

#ifdef __cplusplus
}
#endif

#endif/* BBOX_RTS_PRIV_H__ */
