/***************************************************************************
 * Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
***************************************************************************/
#ifndef BVDC_MCVP_PRIV_H__
#define BVDC_MCVP_PRIV_H__

#include "bavc.h"
#include "breg_mem.h"      /* Chip register access (memory mapped). */
#include "bvdc_common_priv.h"
#include "bvdc_bufferheap_priv.h"
#include "bvdc_buffer_priv.h"
#include "bvdc_subrul_priv.h"
#include "bvdc_window_priv.h"

#ifdef __cplusplus
extern "C" {
#endif

    BDBG_OBJECT_ID_DECLARE(BVDC_ANR);
    BDBG_OBJECT_ID_DECLARE(BVDC_MDI);

/***************************************************************************
* {private}
*
* Mcvp Sub-module Overview:
*
*/

/*-------------------------------------------------------------------------
* macro used by mcvp sub-module
*/
#define BVDC_P_MCVP_BUFFER_MAX_COUNT            (4)

/* MCVP Versions */
#define BVDC_P_MCVP_VER_2                       (2) /* 7422Ax/7425Ax */
#define BVDC_P_MCVP_VER_3                       (3) /* 7231Ax/7344Ax/7346Ax/7358Ax/7552Ax */
#define BVDC_P_MCVP_VER_4                       (4) /* 7366 */
#define BVDC_P_MCVP_VER_5                       (5) /* 7445 D0 multi context support DCX MADR/MCVP enum conflict*/
#define BVDC_P_MCVP_VER_6                       (6) /* 7364 Ax 7439 B0 MVP_TOP_1_DITHER_CTR*/


/*-------------------------------------------------------------------------
* mcvp main context
*/
typedef struct BVDC_P_McvpContext
{
    BDBG_OBJECT(BVDC_MVP)

    /* mcvp Id */
    BVDC_P_McvpId                      eId;
    uint32_t                           ulMaxWidth; /* max width limited by line buf size */
    uint32_t                           ulMaxHeight; /* max height limited by RTS */
    uint32_t                           ulHsclSizeThreshold; /* hsize that triggers use of HSCL before deinterlacing */
    uint32_t                           ulRegOffset;
    uint32_t                           ulRegOffset1;

    /* Core & Vnet Channel Reset */
    uint32_t                           ulCoreResetAddr;
    uint32_t                           ulCoreResetMask;
    uint32_t                           ulVnetResetAddr;
    uint32_t                           ulVnetResetMask;
    uint32_t                           ulVnetMuxAddr;
    uint32_t                           ulVnetMuxValue;
    uint32_t                           ulUpdateAll[BAVC_MOSAIC_MAX];

    /* static info from creating */
    BREG_Handle                        hRegister;

    /* from acquireConnect */
    BVDC_Heap_Handle                   hHeap;
    BVDC_Window_Handle                 hWindow;

    /* sub-modules */
    BVDC_P_Hscaler_Handle              hHscaler;
    BVDC_P_Anr_Handle                  hAnr;
    BVDC_P_Mcdi_Handle                 hMcdi;
    BVDC_P_MvpDcxCore                  eDcxCore;
    bool                               bAnr;

    /* stream properties */
    uint32_t                           ulHSize;
    uint32_t                           ulVSize;
    bool                               bChannelChange[BAVC_MOSAIC_MAX];

    /* user settings */
    BVDC_Mode                          ePqEnhancement;

    /* buffers */
    uint32_t                           ulPixelBufCnt;
    uint32_t                           ulQmBufCnt;

    /*compression*/
    BVDC_P_Compression_Settings        stMvpCompression;

    /* sub-struct to manage vnet and rul build opreations */
    BVDC_P_SubRulContext               SubRul;

    bool                               bPrevDitherEn;
    BVDC_P_DitherSetting               stDither;

    BVDC_P_MvpMode                    stMcvpMode[BAVC_MOSAIC_MAX];
} BVDC_P_McvpContext;


/***************************************************************************
* private functions
***************************************************************************/

#define BVDC_P_Mcvp_MuxAddr(hMcvp)      (hMcvp->ulVnetMuxAddr)
#define BVDC_P_Mcvp_PostMuxValue(hMcvp) (hMcvp->ulVnetMuxValue)
#define BVDC_P_Mcvp_SetVnet_isr(hMcvp, ulSrcMuxValue, eVnetPatchMode) \
    BVDC_P_SubRul_SetVnet_isr(&((hMcvp)->SubRul), ulSrcMuxValue, eVnetPatchMode)
#define BVDC_P_Mcvp_UnsetVnet_isr(hMcvp) \
    BVDC_P_SubRul_UnsetVnet_isr(&((hMcvp)->SubRul))

#if (BVDC_P_SUPPORT_MCVP)
/***************************************************************************
* {private}
*
* BVDC_P_Mcvp_Create
*
* called by BVDC_Open only
*/
BERR_Code BVDC_P_Mcvp_Create
    ( BVDC_P_Mcvp_Handle *           phMcvp,
    BVDC_P_McvpId                    eMvpId,
    BREG_Handle                      hRegister,
    BVDC_P_Resource_Handle           hResource );

/***************************************************************************
* {private}
*
* BVDC_P_Mcvp_Destroy
*
* called by BVDC_Close only
*/
BERR_Code BVDC_P_Mcvp_Destroy
    ( BVDC_P_Mcvp_Handle               hMcvp );

/***************************************************************************
* {private}
*
* BVDC_P_Mcvp_AcquireConnect_isr
*
* It is called by BVDC_Window_Validate after changing from disable mcvp to
* enable mcvp.
*/
BERR_Code BVDC_P_Mcvp_AcquireConnect_isr
    ( BVDC_P_Mcvp_Handle               hMcvp,
    BVDC_Heap_Handle                   hHeap,
    BVDC_Window_Handle                 hWindow);


/***************************************************************************
* {private}
*
* BVDC_P_Mcvp_ReleaseConnect_isr
*
* It is called after window decided that mcvp is no-longer used by HW in its
* vnet mode (i.e. it is really shut down and teared off from vnet).
*/
BERR_Code BVDC_P_Mcvp_ReleaseConnect_isr
    ( BVDC_P_Mcvp_Handle              *phMcvp );

/***************************************************************************
* {private}
*
* BVDC_P_Mcvp_SetVnetAllocBuf_isr
*
* Called by BVDC_P_*_BuildRul_isr to setup for joinning into vnet (including
* optionally acquiring loop-back) and allocate buffers
*/
void BVDC_P_Mcvp_SetVnetAllocBuf_isr
    ( BVDC_P_Mcvp_Handle               hMcvp,
      uint32_t                         ulSrcMuxValue,
      BVDC_P_VnetPatch                 eVnetPatchMode,
      bool                             bRfcgVnet);

/***************************************************************************
* {private}
*
* BVDC_P_Mcvp_UnsetVnetFreeBuf_isr
*
* called by BVDC_P_Window_UnsetWriter(Reader)Vnet_isr to to release the
* potentially used loop-back, and free buffers
*/
void BVDC_P_Mcvp_UnsetVnetFreeBuf_isr
    ( BVDC_P_Mcvp_Handle                hMcvp );

/***************************************************************************
* {private}
*
* BVDC_P_MCVP_SetInfo_isr
*
* called by BVDC_P_Window_Writer(Reader)_isr to to detect size difference between
* two continuous rul
*/
void BVDC_P_MCVP_SetInfo_isr
    (BVDC_P_Mcvp_Handle                 hMcvp,
    BVDC_Window_Handle                 hWindow,
    BVDC_P_PictureNode                *pPicture);

/***************************************************************************
* {private}
*
* BVDC_P_Mcvp_BuildRul_isr
*
* called by BVDC_Window_BuildRul_isr at every src or vec vsync (depending on
* whether reader side or writer side is using this module)
*
* Input:
*    eVnetState - reader or writer window/vnet state
*    pPicComRulInfo - the PicComRulInfo that is the shared Picture info by
*      all sub-modules when they build rul.
*/
void BVDC_P_Mcvp_BuildRul_isr(
    BVDC_P_Mcvp_Handle                 hMcvp,
    BVDC_P_ListInfo                   *pList,
    BVDC_P_State                       eVnetState,
    BVDC_P_WindowContext              *pWindow,
    BVDC_P_PictureNode                *pPicture );

/***************************************************************************
* Initialized back to default whatever user did not customized.
*
*/
void BVDC_P_Mvp_Init_Default
    ( BVDC_MadGameMode                   *peGameMode,
      BPXL_Format                        *pePxlFormat,
      BVDC_Mode                          *pePqEnhancement,
      bool                               *pbShrinkWidth,
      bool                               *pbReverse32Pulldown,
      bool                               *pbReverse22Pulldown,
      BVDC_Deinterlace_ChromaSettings    *pChromaSettings,
      BVDC_Deinterlace_MotionSettings    *pMotionSettings );

void BVDC_P_Mvp_Init_Custom
    ( BVDC_422To444UpSampler            *pUpSampler,
      BVDC_444To422DnSampler            *pDnSampler,
      BVDC_Deinterlace_LowAngleSettings *pLowAngles );
#else
#define BVDC_P_Mcvp_Create(hmcvp, eid, hreg, hres)               BDBG_ASSERT(0)
#define BVDC_P_Mcvp_Destroy(hmcvp)                               BDBG_ASSERT(0)
#define BVDC_P_Mcvp_BuildRul_isr(hmcvp, list, evnet, pwin, ppic) BDBG_ASSERT(0)
#define BVDC_P_Mcvp_AcquireConnect_isr(hmcvp, hheap, hwin)       BDBG_ASSERT(0)
#define BVDC_P_Mcvp_SetVnetAllocBuf_isr(hmcvp, mux, patch, bcfg) BDBG_ASSERT(0)
#define BVDC_P_Mcvp_UnsetVnetFreeBuf_isr(hmcvp)                  BDBG_ASSERT(0)
#define BVDC_P_Mcvp_ReleaseConnect_isr(hmcvp)                    BDBG_ASSERT(0)
#define BVDC_P_MCVP_SetInfo_isr(hmcvp, hwin, ppic)               BDBG_ASSERT(0)
#define BVDC_P_Mvp_Init_Custom(upsample, dnsample, lowangle)     BDBG_ASSERT(0)
#define BVDC_P_Mvp_Init_Default(egmode, epxl, epq, bshrink, \
    pd32, pd22, chroma, motion )                                 BDBG_ASSERT(0)
#endif /* (BVDC_P_SUPPORT_MCVP) */

#ifdef __cplusplus
}
#endif

#endif /* #ifndef BVDC_MCVP_PRIV_H__ */
/* End of file. */
