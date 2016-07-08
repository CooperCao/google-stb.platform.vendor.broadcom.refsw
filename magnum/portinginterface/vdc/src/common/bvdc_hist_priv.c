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
#include "bstd.h"
#include "bkni.h"
#include "brdc.h"
#include "bvdc.h"
#include "bchp_vnet_b.h"
#include "bvdc_resource_priv.h"
#include "bvdc_source_priv.h"
#include "bvdc_compositor_priv.h"

#if (BVDC_P_SUPPORT_HIST_VER >= BVDC_P_SUPPORT_HIST_VER_2)
#include "bchp_hist.h"

BDBG_MODULE(BVDC_HIST);
BDBG_OBJECT_ID(BVDC_HST);

/* This table mapped from VDC bin selects to HW numbins */
static const BVDC_P_Hist_NumBins s_aHistNumBins[] =
{
    { BCHP_HIST_NUMBINS_NUMBINS_BIN_16, 16 },
    { BCHP_HIST_NUMBINS_NUMBINS_BIN_32, 32 },
    { BCHP_HIST_NUMBINS_NUMBINS_BIN_64, 64 },
};

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
      BVDC_P_Resource_Handle            hResource )
{
    BVDC_P_HistContext *pHist;
    BERR_Code  eResult = BERR_SUCCESS;

    BDBG_ENTER(BVDC_P_Hist_Create);

    /* in case creation failed */
    BDBG_ASSERT(phHist);
    *phHist = NULL;

    pHist = (BVDC_P_HistContext *)
        (BKNI_Malloc(sizeof(BVDC_P_HistContext)));
    if( pHist )
    {
        /* init the context */
        BKNI_Memset((void*)pHist, 0x0, sizeof(BVDC_P_HistContext));
        BDBG_OBJECT_SET(pHist, BVDC_HST);
        pHist->eId          = eHistId;
        pHist->hRegister    = hRegister;
        pHist->ulRegOffset  = 0;
        pHist->hWindow      = NULL;
        pHist->bInitial     = true;

        /* init the SubRul sub-module */
        BVDC_P_SubRul_Init(&(pHist->SubRul), BVDC_P_Hist_MuxAddr(pHist),
            0, BVDC_P_DrainMode_eNone, 0, hResource);

        *phHist = pHist;
    }

    BDBG_LEAVE(BVDC_P_Hist_Create);
    return BERR_TRACE(eResult);
}

/***************************************************************************
 * {private}
 *
 * BVDC_P_Hist_Destroy
 *
 * called by BVDC_Close only
 */
BERR_Code BVDC_P_Hist_Destroy
    ( BVDC_P_Hist_Handle                hHist )
{
    BERR_Code  eResult = BERR_SUCCESS;

    BDBG_ENTER(BVDC_P_Hist_Destroy);
    BDBG_OBJECT_ASSERT(hHist, BVDC_HST);

    BDBG_OBJECT_DESTROY(hHist, BVDC_HST);
    /* it is gone afterwards !!! */
    BKNI_Free((void*)hHist);

    BDBG_LEAVE(BVDC_P_Hist_Destroy);
    return BERR_TRACE(eResult);
}


/***************************************************************************
 * {private}
 *
 * BVDC_P_Hist_AcquireConnect_isr
 *
 * It is called by BVDC_Window_Validate after changing from diableBox to
 * enablingBox .
 */
BERR_Code BVDC_P_Hist_AcquireConnect_isr
    ( BVDC_P_Hist_Handle                hHist,
      BVDC_Window_Handle                hWindow)
{
    BERR_Code  eResult = BERR_SUCCESS;

    BDBG_ENTER(BVDC_P_Hist_AcquireConnect_isr);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    hHist->hWindow = hWindow;

    BDBG_LEAVE(BVDC_P_Hist_AcquireConnect_isr);
    return BERR_TRACE(eResult);
}


/***************************************************************************
 * {private}
 *
 * BVDC_P_Hist_ReleaseConnect_isr
 *
 * It is called after window decided that hist is no-longer used by HW in
 * its vnet mode (i.e. it is really shut down and teared off from vnet).
 */
BERR_Code BVDC_P_Hist_ReleaseConnect_isr
    ( BVDC_P_Hist_Handle               *phHist )
{
    BERR_Code  eResult = BERR_SUCCESS;

    BDBG_ENTER(BVDC_P_Hist_ReleaseConnect_isr);

    /* handle validation */
    BDBG_OBJECT_ASSERT(*phHist, BVDC_HST);

    BVDC_P_SubRul_UnsetVnet_isr(&((*phHist)->SubRul));
    BVDC_P_Resource_ReleaseHandle_isr(
        BVDC_P_SubRul_GetResourceHandle_isr(&(*phHist)->SubRul),
        BVDC_P_ResourceType_eHist, (void *)(*phHist));

    /* this makes win to stop calling hist code */
    *phHist = NULL;

    BDBG_LEAVE(BVDC_P_Hist_ReleaseConnect_isr);
    return BERR_TRACE(eResult);
}

/***************************************************************************
 * {private}
 *
 * BVDC_P_Hist_UpdateHistData_isr
 *
 * called by BVDC_Window_BuildRul_isr at every src vsync. It reads histogram
 * data from HW into hist structure
 */
void BVDC_P_Hist_UpdateHistData_isr
    ( BVDC_P_Hist_Handle                hHist )
{
    uint32_t id;
    uint32_t ulReg;
    BVDC_LumaStatus *pStats = &hHist->stHistData;

    BDBG_OBJECT_ASSERT(hHist, BVDC_HST);

    ulReg = BCHP_FIELD_ENUM(HIST_SW_READ, READING, READ);
    BREG_Write32(hHist->hRegister, BCHP_HIST_SW_READ, ulReg);

    ulReg = BREG_Read32(hHist->hRegister, BCHP_HIST_RD_MIN);
    if((BCHP_GET_FIELD_DATA(ulReg, HIST_RD_MIN, INVALID)  == BCHP_HIST_RD_MIN_INVALID_INVALID) ||
       (BCHP_GET_FIELD_DATA(ulReg, HIST_RD_MIN, NOTFOUND) == BCHP_HIST_RD_MIN_NOTFOUND_NF))
    {
        pStats->ulMin = 0;
    }
    else
    {
        pStats->ulMin = BCHP_GET_FIELD_DATA(ulReg, HIST_RD_MIN, MIN);
    }

    ulReg = BREG_Read32(hHist->hRegister, BCHP_HIST_RD_MAX);
    if((BCHP_GET_FIELD_DATA(ulReg, HIST_RD_MAX, INVALID)  == BCHP_HIST_RD_MAX_INVALID_INVALID) ||
       (BCHP_GET_FIELD_DATA(ulReg, HIST_RD_MAX, NOTFOUND) == BCHP_HIST_RD_MAX_NOTFOUND_NF))
    {
        pStats->ulMax = 0;
    }
    else
    {
        pStats->ulMax = BCHP_GET_FIELD_DATA(ulReg, HIST_RD_MAX, MAX);
    }

    ulReg = BREG_Read32(hHist->hRegister, BCHP_HIST_STATUS);
    if(BCHP_GET_FIELD_DATA(ulReg, HIST_STATUS, INVALID) == BCHP_HIST_STATUS_INVALID_VALID)
    {
        pStats->ulSum = BREG_Read32(hHist->hRegister, BCHP_HIST_RD_APL);
    }
    else
    {
        pStats->ulSum = 0;
    }

    for(id = 0; id < BVDC_LUMA_HISTOGRAM_COUNT; id++)
    {
        ulReg = BREG_Read32(hHist->hRegister, BCHP_HIST_RD_BINi_ARRAY_BASE + (id * 4));
        if((BCHP_GET_FIELD_DATA(ulReg, HIST_RD_BINi, INVALID) == BCHP_HIST_RD_BINi_INVALID_INVALID) ||
           (BCHP_GET_FIELD_DATA(ulReg, HIST_RD_BINi, NOT_USED) == BCHP_HIST_RD_BINi_NOT_USED_NOT_USED))
        {
            pStats->aulHistogram[id] = 0;
        }
        else
        {
            pStats->aulHistogram[id] = BCHP_GET_FIELD_DATA(ulReg, HIST_RD_BINi, COUNT);
        }
    }

    for(id = 0; id < BVDC_LUMA_HISTOGRAM_LEVELS; id++)
    {
        ulReg = BREG_Read32(hHist->hRegister, BCHP_HIST_RD_LEVELi_ARRAY_BASE + (id * 4));
        if((BCHP_GET_FIELD_DATA(ulReg, HIST_RD_LEVELi, INVALID) == BCHP_HIST_RD_LEVELi_INVALID_INVALID) ||
           (BCHP_GET_FIELD_DATA(ulReg, HIST_RD_LEVELi, NOTFOUND) == BCHP_HIST_RD_LEVELi_NOTFOUND_NF))
        {
            pStats->aulLevelStats[id] = 0;
        }
        else
        {
            pStats->aulLevelStats[id] = BCHP_GET_FIELD_DATA(ulReg, HIST_RD_LEVELi, LEVEL);
        }
    }

    ulReg = BCHP_FIELD_ENUM(HIST_SW_READ, READING, IDLE);
    BREG_Write32(hHist->hRegister, BCHP_HIST_SW_READ, ulReg);
}

/***************************************************************************
 *
 */
BERR_Code BVDC_P_Hist_SetEnable_isr
    ( BVDC_P_ListInfo                  *pList,
      bool                              bEnable )
{
    BDBG_ENTER(BVDC_P_Hist_SetEnable_isr);

    *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_HIST_EN);
    *pList->pulCurrent++ = BCHP_FIELD_DATA(HIST_EN, ENABLE, bEnable);

    BDBG_MSG(("Set Hist %s", bEnable ? "true" : "false"));

    BDBG_LEAVE(BVDC_P_Hist_SetEnable_isr);
    return BERR_SUCCESS;
}

/***************************************************************************
 * {private}
 *
 * BVDC_P_Hist_BuildCfgRul_isr
 *
 * called by BVDC_Window_BuildRul_isr once dirty bit is set. It builds RUL for
 * Hist HW module.
 */
static void BVDC_P_Hist_BuildCfgRul_isr
    ( BVDC_P_Hist_Handle                hHist,
      const BVDC_Window_Handle          hWindow,
      BVDC_P_ListInfo                  *pList )
{
    const BVDC_P_Window_Info *pCurInfo;
    uint32_t ulRulOffset;
    const BVDC_P_Hist_NumBins *pNumBin;
    BVDC_ClipRect stUsrClipRect;
    BVDC_P_Rect stHistoRect;
    bool bInterlaced;
    uint32_t aulLevelThres[BVDC_LUMA_HISTOGRAM_LEVELS];
    uint32_t id;
    uint32_t ulWidth, ulHeight;

    BDBG_ENTER(BVDC_P_Hist_BuildCfgRul_isr);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);
    BDBG_OBJECT_ASSERT(hHist, BVDC_HST);
    pCurInfo = &hWindow->stCurInfo;
    BDBG_ASSERT(pCurInfo->stLumaRect.eNumBins < (sizeof(s_aHistNumBins) / sizeof(BVDC_P_Hist_NumBins)));

    pNumBin = &s_aHistNumBins[pCurInfo->stLumaRect.eNumBins];

    ulRulOffset = hHist->ulRegOffset;

    if(hWindow->stCurInfo.bHistAtSrc)
    {
        ulWidth  = BVDC_P_OVER_SAMPLE(hWindow->stCurInfo.hSource->ulSampleFactor,
            hWindow->stCurInfo.hSource->stCurInfo.pFmtInfo->ulWidth);
        ulHeight = hWindow->stCurInfo.hSource->stCurInfo.pFmtInfo->ulHeight;
        bInterlaced = hWindow->stCurInfo.hSource->stCurInfo.pFmtInfo->bInterlaced;
    }
    else
    {
        ulWidth = hWindow->stCurInfo.stDstRect.ulWidth;
        ulHeight = hWindow->stCurInfo.stDstRect.ulHeight;
        bInterlaced = hWindow->hCompositor->stCurInfo.pFmtInfo->bInterlaced;
    }

    if(hWindow->stCurInfo.bLumaRectUserSet)
    {
        stUsrClipRect = pCurInfo->stLumaRect.stRegion;
    }
    else
    {
        /* If usr region is not set, HIST can be enabled by dynamic contrast */
        /* therefore, use default clipping of 0, meaning full source size */
        stUsrClipRect.ulLeft   = 0;
        stUsrClipRect.ulRight  = 0;
        stUsrClipRect.ulTop    = 0;
        stUsrClipRect.ulBottom = 0;
    }
    BVDC_P_CalculateRect_isr(&stUsrClipRect, ulWidth, ulHeight, bInterlaced, &stHistoRect);

    for(id = 0; id < BVDC_LUMA_HISTOGRAM_LEVELS; id++)
    {
        aulLevelThres[id] = ((pCurInfo->stLumaRect.aulLevelThres[id] *
            stHistoRect.ulWidth /100) * stHistoRect.ulHeight / 100);
    }

    if(hWindow->stCurInfo.bLumaRectUserSet)
    {
        BDBG_MSG(("ClipRect: (%4d, %4d, %4d, %4d)", stUsrClipRect.ulLeft,
            stUsrClipRect.ulRight, stUsrClipRect.ulTop, stUsrClipRect.ulBottom));
    }
    BVDC_P_PRINT_RECT("HistoRect", &stHistoRect, false);
    BDBG_MSG(("HistEnable = %s, UserCfg = %s, HistAtSrc = %s, Hist Size: %d, Interlace = %s",
        pCurInfo->bHistEnable ? "true" :"false",
        hWindow->stCurInfo.bLumaRectUserSet ? "true" : "false",
        hWindow->stCurInfo.bHistAtSrc ? "true" : "false",
        pNumBin->ulHistSize,
        bInterlaced ? "true" : "false"));
    BDBG_MSG(("Threshold[(%d/100)%%] = %d", pCurInfo->stLumaRect.aulLevelThres[0], aulLevelThres[0]));
    BDBG_MSG(("Threshold[(%d/100)%%] = %d", pCurInfo->stLumaRect.aulLevelThres[1], aulLevelThres[1]));
    BDBG_MSG(("Threshold[(%d/100)%%] = %d", pCurInfo->stLumaRect.aulLevelThres[2], aulLevelThres[2]));
    BDBG_MSG(("Threshold[(%d/100)%%] = %d", pCurInfo->stLumaRect.aulLevelThres[3], aulLevelThres[3]));

    *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_HIST_WIN_OFFSET);
    *pList->pulCurrent++ =
        BCHP_FIELD_DATA(HIST_WIN_OFFSET, H_OFFSET, stHistoRect.lLeft) |
        BCHP_FIELD_DATA(HIST_WIN_OFFSET, V_OFFSET, stHistoRect.lTop);

    *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_HIST_WIN_SIZE);
    *pList->pulCurrent++ =
        BCHP_FIELD_DATA(HIST_WIN_SIZE, H_SIZE, stHistoRect.ulWidth) |
        BCHP_FIELD_DATA(HIST_WIN_SIZE, V_SIZE, stHistoRect.ulHeight);

    *pList->pulCurrent++ = BRDC_OP_IMMS_TO_REGS(BVDC_LUMA_HISTOGRAM_LEVELS);
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_HIST_LVL_THRESHi_ARRAY_BASE);
    BKNI_Memcpy((void*)pList->pulCurrent, (void*)&aulLevelThres[0], BVDC_LUMA_HISTOGRAM_LEVELS * sizeof(uint32_t));
    pList->pulCurrent += BVDC_LUMA_HISTOGRAM_LEVELS;

    *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_HIST_NUMBINS);
    *pList->pulCurrent++ =
        BCHP_FIELD_DATA(HIST_NUMBINS, NUMBINS, pNumBin->ulHwNumBin);

    BVDC_P_Hist_SetEnable_isr(pList, pCurInfo->bHistEnable);
    hHist->ulHistSize = pNumBin->ulHistSize;
    hHist->stHistData.ulPixelCnt = stHistoRect.ulWidth * stHistoRect.ulHeight;
    BDBG_LEAVE(BVDC_P_Hist_BuildCfgRul_isr);
}

/***************************************************************************
 * {private}
 *
 * BVDC_P_Hist_BuildVsyncRul_isr
 *
 * called by BVDC_Window_BuildRul_isr every vsync
 */
static void BVDC_P_Hist_BuildVsyncRul_isr
    ( BVDC_P_ListInfo                  *pList )
{
    *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_HIST_POP);
    *pList->pulCurrent++ =
        BCHP_FIELD_ENUM(HIST_POP, POP, POP);
}

/***************************************************************************
 * {private}
 *
 * BVDC_P_Hist_BuildRul_isr
 *
 * called by BVDC_Window_BuildRul_isr at every src vsync. It builds RUL for
 * Hist HW module.
 *
 * It will reset *phHist to NULL if the HW module is no longer used by
 * any window.
 */
void BVDC_P_Hist_BuildRul_isr
    ( BVDC_P_Hist_Handle                hHist,
      BVDC_P_ListInfo                  *pList,
      BVDC_P_State                      eVnetState,
      BVDC_P_PicComRulInfo             *pPicComRulInfo )
{
    uint32_t                 ulRulOpsFlags;
    BVDC_P_Window_Info      *pCurInfo;
    BVDC_P_Window_DirtyBits *pCurDirty;

    /* handle validation */
    BDBG_OBJECT_ASSERT(hHist, BVDC_HST);
    BDBG_OBJECT_ASSERT(hHist->hWindow, BVDC_WIN);

    pCurInfo  = &hHist->hWindow->stCurInfo;
    pCurDirty = &pCurInfo->stDirty;


    ulRulOpsFlags = BVDC_P_SubRul_GetOps_isr(
        &(hHist->SubRul), pPicComRulInfo->eWin, eVnetState, pList->bLastExecuted);

    if ((0 == ulRulOpsFlags) || (ulRulOpsFlags & BVDC_P_RulOp_eReleaseHandle))
    {
        if(pCurDirty->stBits.bHistoRect)
        {
            /* Defer setting hist configuration until vnet stage is active */
            hHist->bInitial = true;
            pCurDirty->stBits.bHistoRect = BVDC_P_CLEAN;
        }
        return;
    }

    if(!pList->bLastExecuted || pCurDirty->stBits.bHistoRect || hHist->bInitial)
    {
        BVDC_P_Hist_BuildCfgRul_isr(hHist, hHist->hWindow, pList);

        hHist->bInitial = false;
        pCurDirty->stBits.bHistoRect = BVDC_P_CLEAN;
    }

    if(pCurInfo->bHistEnable)
    {
        BVDC_P_Hist_BuildVsyncRul_isr(pList);
    }

    if (ulRulOpsFlags & BVDC_P_RulOp_eEnable)
    {
        /* join in vnet after enable. note: its src mux is initialed as disabled */
        if (ulRulOpsFlags & BVDC_P_RulOp_eVnetInit)
        {
            BVDC_P_SubRul_JoinInVnet_isr(&(hHist->SubRul), pList);
        }
    }
    else if (ulRulOpsFlags & BVDC_P_RulOp_eDisable)
    {
        BVDC_P_SubRul_DropOffVnet_isr(&(hHist->SubRul), pList);
        BVDC_P_Hist_SetEnable_isr(pList, false);
        hHist->bInitial = true;
    }
}

#else
/***************************************************************************/
/* No support for any hist block */

#include "bvdc_errors.h"

BDBG_MODULE(BVDC_HIST);
BDBG_OBJECT_ID(BVDC_HST);

BERR_Code BVDC_P_Hist_Create
    ( BVDC_P_Hist_Handle *              phHist,
      BVDC_P_HistId                     eHistId,
      BREG_Handle                       hRegister,
      BVDC_P_Resource_Handle            hResource )
{
    BDBG_ASSERT(phHist);
    *phHist = NULL;
    BSTD_UNUSED(eHistId);
    BSTD_UNUSED(hRegister);
    BSTD_UNUSED(hResource);
    return BERR_SUCCESS;
}

BERR_Code BVDC_P_Hist_Destroy
    ( BVDC_P_Hist_Handle                hHist )
{
    BSTD_UNUSED(hHist);
    return BERR_SUCCESS;
}

BERR_Code BVDC_P_Hist_AcquireConnect_isr
    ( BVDC_P_Hist_Handle                hHist,
      BVDC_Window_Handle                hWindow)
{
    BSTD_UNUSED(hHist);
    BSTD_UNUSED(hWindow);
    return BERR_SUCCESS;
}

BERR_Code BVDC_P_Hist_ReleaseConnect_isr
    ( BVDC_P_Hist_Handle               *phHist )
{
    BSTD_UNUSED(phHist);
    return BERR_SUCCESS;
}

void BVDC_P_Hist_BuildRul_isr
    ( BVDC_P_Hist_Handle                hHist,
      BVDC_P_ListInfo                  *pList,
      BVDC_P_State                      eVnetState,
      BVDC_P_PicComRulInfo             *pPicComRulInfo )
{
    BSTD_UNUSED(hHist);
    BSTD_UNUSED(pList);
    BSTD_UNUSED(eVnetState);
    BSTD_UNUSED(pPicComRulInfo);
    return;
}

void BVDC_P_Hist_UpdateHistData_isr
    ( BVDC_P_Hist_Handle                hHist )
{
    BSTD_UNUSED(hHist);
    return;
}

#endif  /* #if (BVDC_P_SUPPORT_HIST_VER >= BVDC_P_SUPPORT_HIST_VER_2) */

void BVDC_P_Hist_GetHistogramData
    ( const BVDC_Window_Handle          hWindow,
      BVDC_LumaStatus                  *pLumaStatus )
{
    BVDC_Source_Handle            hSource;

    BDBG_ENTER(BVDC_P_Hist_GetHistogramData);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);
    BDBG_OBJECT_ASSERT(hWindow->stCurResource.hHist, BVDC_HST);
    hSource = hWindow->stCurInfo.hSource;
    BDBG_OBJECT_ASSERT(hSource, BVDC_SRC);

    BKNI_EnterCriticalSection();

    if(pLumaStatus)
    {
        if(hSource->stCurInfo.eMuteMode == BVDC_MuteMode_eDisable)
        {
            *pLumaStatus = hWindow->stCurResource.hHist->stHistData;
        }
        else
        {
            *pLumaStatus = hWindow->stCurResource.hHist->stFreezedHistData;
        }
    }

    BKNI_LeaveCriticalSection();
    BDBG_LEAVE(BVDC_P_Hist_GetHistogramData);

    return;
}

/* End of file. */
