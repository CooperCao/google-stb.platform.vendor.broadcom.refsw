/***************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
#ifndef BVDC_HIST_PRIV_H__
#define BVDC_HIST_PRIV_H__

#include "breg_mem.h"      /* Chip register access (memory mapped). */
#include "bvdc_common_priv.h"
#include "bvdc_subrul_priv.h"
#include "bvdc_window_priv.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
 * {private}
 *
 */
#define BVDC_P_SUPPORT_HIST_VER_0                            (0) /* other */
#define BVDC_P_SUPPORT_HIST_VER_1                            (1) /* 7400, 3563, 7325, 7335 */
#define BVDC_P_SUPPORT_HIST_VER_2                            (2) /* 3548, 3556 */

/*-------------------------------------------------------------------------
 * macro used by Histogram sub-module
 */

#define BVDC_P_Hist_MuxAddr(hHist)   (BCHP_VNET_B_HISTOGRAM_0_SRC + (hHist)->eId * sizeof(uint32_t))

#define BVDC_P_Hist_SetVnet_isr(hHist, ulSrcMuxValue, eVnetPatchMode) \
   BVDC_P_SubRul_SetVnet_isr(&((hHist)->SubRul), ulSrcMuxValue, eVnetPatchMode)
#define BVDC_P_Hist_UnsetVnet_isr(hHist) \
   BVDC_P_SubRul_UnsetVnet_isr(&((hHist)->SubRul))

/*
 * This structure contains the actual HW value representing the number of
 * histogram bins and the actual number of histogram bins
 */
typedef struct BVDC_P_Hist_NumBins
{
    uint32_t                        ulHwNumBin;
    uint32_t                        ulHistSize;
} BVDC_P_Hist_NumBins;

/*-------------------------------------------------------------------------
 * Histogram main context
 */
typedef struct BVDC_P_HistContext
{
    BDBG_OBJECT(BVDC_HST)

    /* Hist Id */
    BVDC_P_HistId                    eId;
    uint32_t                         ulRegOffset;

    /* static info from creating */
    BREG_Handle                      hRegister;

    /* Which window it connect to */
    BVDC_Window_Handle               hWindow;

    /* sub-struct to manage vnet and rul build opreations */
    BVDC_P_SubRulContext             SubRul;

    bool                             bInitial;

    /* Histogram data */
    BVDC_LumaStatus                  stHistData;
    uint32_t                         ulHistSize;

    /* Freezed histogram data */
    BVDC_LumaStatus                  stFreezedHistData;

} BVDC_P_HistContext;


/***************************************************************************
 * private functions
***************************************************************************/
/***************************************************************************
 * {private}
 *
 * BVDC_P_Hist_Create
 *
 * called by BVDC_Open only
 */
BERR_Code BVDC_P_Hist_Create
    ( BVDC_P_Hist_Handle               *phHist,
      BVDC_P_HistId                     eHistId,
      BREG_Handle                       hRegister,
      BVDC_P_Resource_Handle            hResource );

/***************************************************************************
 * {private}
 *
 * BVDC_P_Hist_Destroy
 *
 * called by BVDC_Close only
 */
BERR_Code BVDC_P_Hist_Destroy
    ( BVDC_P_Hist_Handle                hHist );

/***************************************************************************
 * {private}
 *
 * BVDC_P_Hist_AcquireConnect_isr
 *
 * It is called by BVDC_Window_Validate after changing from diabling Hist to
 * enabling Hist.
 */
BERR_Code BVDC_P_Hist_AcquireConnect_isr
    ( BVDC_P_Hist_Handle                hHist,
      BVDC_Window_Handle                hWindow);

/***************************************************************************
 * {private}
 *
 * BVDC_P_Hist_ReleaseConnect_isr
 *
 * It is called after window decided that Histogram is no-longer used by HW in
 * its vnet mode (i.e. it is really shut down and teared off from vnet).
 */
BERR_Code BVDC_P_Hist_ReleaseConnect_isr
    ( BVDC_P_Hist_Handle               *phHist );

/***************************************************************************
 * {private}
 *
 * BVDC_P_Hist_BuildRul_isr
 *
 * called by BVDC_Window_BuildRul_isr at every src vsync. It builds RUL for
 * histogram HW module.
 */
void BVDC_P_Hist_BuildRul_isr
    ( BVDC_P_Hist_Handle                hHist,
      BVDC_P_ListInfo                  *pList,
      BVDC_P_State                      eVnetState,
      BVDC_P_PicComRulInfo             *pPicComRulInfo );

/***************************************************************************
 * {private}
 *
 * BVDC_P_Hist_UpdateHistData_isr
 *
 * called by BVDC_Window_Writer_isr at every src vsync. It samples the
 * Histogram registers.
 */
void BVDC_P_Hist_UpdateHistData_isr
    ( BVDC_P_Hist_Handle                hHist );

/***************************************************************************
 * {private}
 *
 * BVDC_P_Hist_GetHistogramData
 *
 * called by BVDC_Window_GetLumaStatus. It returns the histogram data
 * collected by the HIST block.
 */
void BVDC_P_Hist_GetHistogramData
    ( const BVDC_Window_Handle          hWindow,
      BVDC_LumaStatus                  *pLumaStatus );

#ifdef __cplusplus
}
#endif

#endif /* #ifndef BVDC_HIST_PRIV_H__ */
/* End of file. */
