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
#include "bvdc_compositor_priv.h"
#include "bvdc_source_priv.h"
#include "bvdc_pep_priv.h"
#include "bvdc_tnt_priv.h"

BDBG_MODULE(BVDC_PEP);
BDBG_OBJECT_ID(BVDC_PEP);

#if (BVDC_P_SUPPORT_MASK_DITHER)
#include "bchp_mask_0.h"
#endif

#if(BVDC_P_SUPPORT_HIST)
/*************************************************************************
 *  {secret}
 *  BVDC_P_Histo_BuildRul_isr
 *  Builds Histogram block
 **************************************************************************/
static void BVDC_P_Histo_BuildRul_isr
    ( const BVDC_Window_Handle     hWindow,
      bool                         bInterlaced,
      BVDC_P_ListInfo             *pList )
{
    const BVDC_P_Window_Info    *pCurInfo;
    BVDC_P_Rect                  stRect;

    BDBG_ENTER(BVDC_P_Histo_BuildRul_isr);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    /* Get current pointer to RUL */
    pCurInfo = &hWindow->stCurInfo;

    BVDC_P_CalculateRect_isr(&pCurInfo->stLumaRect.stRegion,
                             pCurInfo->stDstRect.ulWidth,
                             pCurInfo->stDstRect.ulHeight,
                             bInterlaced, &stRect);
    BDBG_MSG(("Dst Rect: %d x %d => Hist Rec: %d x %d",
        pCurInfo->stDstRect.ulWidth, pCurInfo->stDstRect.ulHeight,
        stRect.ulWidth, stRect.ulHeight));

    *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_PEP_CMP_0_V0_HISTO_WIN_OFFSET);
    *pList->pulCurrent++ =
        BCHP_FIELD_DATA(PEP_CMP_0_V0_HISTO_WIN_OFFSET, X_OFFSET, stRect.lLeft) |
        BCHP_FIELD_DATA(PEP_CMP_0_V0_HISTO_WIN_OFFSET, Y_OFFSET, stRect.lTop);

    *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_PEP_CMP_0_V0_HISTO_WIN_SIZE);
    *pList->pulCurrent++ =
        BCHP_FIELD_DATA(PEP_CMP_0_V0_HISTO_WIN_SIZE, HSIZE, stRect.ulWidth) |
        BCHP_FIELD_DATA(PEP_CMP_0_V0_HISTO_WIN_SIZE, VSIZE, stRect.ulHeight);

    BDBG_LEAVE(BVDC_P_Histo_BuildRul_isr);
    return;
}

/*************************************************************************
 *  {secret}
 *  BVDC_P_Histo_UpdateHistoData_isr
 *  Reading the histogram data every vysnc, called from Window Reader isr
 **************************************************************************/
void BVDC_P_Histo_UpdateHistoData_isr
    ( BVDC_P_Pep_Handle            hPep )
{
    uint32_t id;
    uint32_t ulReg;

    BDBG_ENTER(BVDC_P_Histo_UpdateHistoData_isr);
    BDBG_OBJECT_ASSERT(hPep, BVDC_PEP);

    for(id = 0; id < hPep->ulHistSize; id++)
    {
        hPep->stTmpHistoData.aulHistogram[id] = BREG_Read32(hPep->hReg,
            BCHP_PEP_CMP_0_V0_HISTO_DATA_COUNT_i_ARRAY_BASE + (id * 4));
    }

    ulReg = BREG_Read32(hPep->hReg, BCHP_PEP_CMP_0_V0_HISTO_MIN_MAX);
    hPep->stTmpHistoData.ulMin = BCHP_GET_FIELD_DATA(ulReg, PEP_CMP_0_V0_HISTO_MIN_MAX, MIN);
    hPep->stTmpHistoData.ulMax = BCHP_GET_FIELD_DATA(ulReg, PEP_CMP_0_V0_HISTO_MIN_MAX, MAX);

    BDBG_LEAVE(BVDC_P_Histo_UpdateHistoData_isr);
}

/*************************************************************************
 *  {secret}
 *  BVDC_P_Pep_ReadHistoData_isr
 *  Reading the histogram data
 **************************************************************************/
static void BVDC_P_Pep_ReadHistoData_isr
    ( const BVDC_Window_Handle     hWindow,
      BVDC_P_Pep_Handle            hPep )
{
    uint32_t id;

    BSTD_UNUSED(hWindow);
    BDBG_ENTER(BVDC_P_Pep_ReadHistoData_isr);
    BDBG_OBJECT_ASSERT(hPep, BVDC_PEP);

    for(id = 0; id < hPep->ulHistSize; id++)
    {
        /* Keeping the last 3 histo counts used by contrast stretch algorithm */
        hPep->aulLastLastBin[id] = hPep->aulLastBin[id];
        hPep->aulLastBin[id] = hPep->stHistoData.aulHistogram[id];
        hPep->stHistoData.aulHistogram[id] = hPep->stTmpHistoData.aulHistogram[id];
    }

    hPep->stHistoData.ulMin = hPep->stTmpHistoData.ulMin;
    hPep->stHistoData.ulMax = hPep->stTmpHistoData.ulMax;

    BDBG_LEAVE(BVDC_P_Pep_ReadHistoData_isr);
}
#endif /* BVDC_P_SUPPORT_HIST */


/***************************************************************************
 * {private}
 *
 */
BERR_Code BVDC_P_Pep_Create
    ( BVDC_P_Pep_Handle            *phPep,
      const BVDC_P_WindowId         eWinId,
      const BREG_Handle             hReg )
{
    BVDC_P_PepContext *pPep;

    BDBG_ENTER(BVDC_P_Pep_Create);

    BDBG_ASSERT(phPep);

    /* (1) Alloc the context. */
    pPep = (BVDC_P_PepContext*)
        (BKNI_Malloc(sizeof(BVDC_P_PepContext)));
    if(!pPep)
    {
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }

    /* Clear out the context and set defaults. */
    BKNI_Memset((void*)pPep, 0x0, sizeof(BVDC_P_PepContext));
    BDBG_OBJECT_SET(pPep, BVDC_PEP);

    pPep->eId              = eWinId;
    pPep->hReg             = hReg;

    /* All done. now return the new fresh context to user. */
    *phPep = (BVDC_P_Pep_Handle)pPep;

    BDBG_LEAVE(BVDC_P_Pep_Create);
    return BERR_SUCCESS;
}


/***************************************************************************
 * {private}
 *
 */
void BVDC_P_Pep_Destroy
    ( BVDC_P_Pep_Handle             hPep )
{
    BDBG_ENTER(BVDC_P_Pep_Destroy);

    BDBG_OBJECT_ASSERT(hPep, BVDC_PEP);

    BDBG_OBJECT_DESTROY(hPep, BVDC_PEP);
    /* [1] Free the context. */
    BKNI_Free((void*)hPep);

    BDBG_LEAVE(BVDC_P_Pep_Destroy);
    return;
}


/***************************************************************************
 * {private}
 *
 */
static void BVDC_P_Pep_DcInit_isr
    ( const BVDC_P_Pep_Handle       hPep )
{
    BDBG_ENTER(BVDC_P_Pep_DcInit_isr);
    BDBG_OBJECT_ASSERT(hPep, BVDC_PEP);

    /* Clear out the context and set defaults. */
    BKNI_Memset((void*)hPep->aulLastBin,       0x0, sizeof(hPep->aulLastBin));
    BKNI_Memset((void*)hPep->aulLastLastBin,   0x0, sizeof(hPep->aulLastLastBin));
    BKNI_Memset((void*)&hPep->stHistoData,     0x0, sizeof(BVDC_LumaStatus));
    BKNI_Memset((void*)&hPep->stTmpHistoData,  0x0, sizeof(BVDC_LumaStatus));
    BKNI_Memset((void*)&hPep->ulAvgLevelStats, 0x0, sizeof(hPep->ulAvgLevelStats));

    hPep->bLabTableValid = false;
    hPep->bProcessHist   = false;
    hPep->bLoadCabTable  = false;
    hPep->bLoadLabTable  = false;
    hPep->bProcessCab    = true;

    hPep->ulAvgAPL       = 0;
    hPep->ulFiltAPL      = 0;
    hPep->lLastGain      = 0;

    BDBG_LEAVE(BVDC_P_Pep_DcInit_isr);
    return;
}

void BVDC_P_Pep_Init
    ( const BVDC_P_Pep_Handle       hPep )
{
    BDBG_ENTER(BVDC_P_Pep_Init);
    BDBG_OBJECT_ASSERT(hPep, BVDC_PEP);

    hPep->bInitial = true;

    BKNI_EnterCriticalSection();
    BVDC_P_Pep_DcInit_isr(hPep);
    BKNI_LeaveCriticalSection();

    hPep->bLabCtrlPending= false;

    hPep->ulHistSize       = BVDC_P_HISTO_TABLE_SIZE;

    hPep->stCallbackData.ulShift = 8;
    hPep->stCallbackData.iScalingFactor = 0;

    BDBG_LEAVE(BVDC_P_Pep_Init);
    return;
}


/*************************************************************************
 *  {secret}
 *  BVDC_P_Dither_BuildRul_isr
 *  Builds MASK (Dithering) block
 **************************************************************************/
static void BVDC_P_Dither_BuildRul_isr
    ( const BVDC_Window_Handle     hWindow,
      BVDC_P_ListInfo             *pList )
{
#if (BVDC_P_SUPPORT_MASK_DITHER)
    const BVDC_DitherSettings *pDither;
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);
    pDither = &hWindow->stCurInfo.stDither;

    /* Update the dithering registers */
    *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_MASK_0_CONTROL + hWindow->ulMaskRegOffset);
    *pList->pulCurrent++ =
        BCHP_FIELD_ENUM(MASK_0_CONTROL, TEXTURE_ENABLE, DISABLE ) |
        BCHP_FIELD_DATA(MASK_0_CONTROL, REDUCE_SMOOTH,
            pDither->bReduceSmooth                              ) |
        BCHP_FIELD_DATA(MASK_0_CONTROL, SMOOTH_ENABLE,
            pDither->bSmoothEnable                              );

    *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_MASK_0_SMOOTH_LIMIT + hWindow->ulMaskRegOffset);
    *pList->pulCurrent++ =
        BCHP_FIELD_DATA(MASK_0_SMOOTH_LIMIT, VALUE,
            pDither->ulSmoothLimit                              );

    *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_MASK_0_TEXTURE_FREQ + hWindow->ulMaskRegOffset);
    *pList->pulCurrent++ =
        BCHP_FIELD_DATA(MASK_0_TEXTURE_FREQ, HORIZ_FREQ, 8 ) |
        BCHP_FIELD_DATA(MASK_0_TEXTURE_FREQ, VERT_FREQ,  8 );

    *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_MASK_0_RANDOM_PATTERN + hWindow->ulMaskRegOffset);
    *pList->pulCurrent++ =
        BCHP_FIELD_ENUM(MASK_0_RANDOM_PATTERN, RNG_MODE, RUN  ) |
        BCHP_FIELD_DATA(MASK_0_RANDOM_PATTERN, RNG_SEED, 4369 );

    *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_MASK_0_SCALE_0_1_2_3 + hWindow->ulMaskRegOffset);
    *pList->pulCurrent++ =
        BCHP_FIELD_DATA(MASK_0_SCALE_0_1_2_3, MULT_0,  0 ) |
        BCHP_FIELD_DATA(MASK_0_SCALE_0_1_2_3, SHIFT_0, 4 ) |
        BCHP_FIELD_DATA(MASK_0_SCALE_0_1_2_3, MULT_1,  0 ) |
        BCHP_FIELD_DATA(MASK_0_SCALE_0_1_2_3, SHIFT_1, 4 ) |
        BCHP_FIELD_DATA(MASK_0_SCALE_0_1_2_3, MULT_2,  0 ) |
        BCHP_FIELD_DATA(MASK_0_SCALE_0_1_2_3, SHIFT_2, 4 ) |
        BCHP_FIELD_DATA(MASK_0_SCALE_0_1_2_3, MULT_3,  0 ) |
        BCHP_FIELD_DATA(MASK_0_SCALE_0_1_2_3, SHIFT_3, 4 );

    *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_MASK_0_SCALE_4_5_6_7 + hWindow->ulMaskRegOffset);
    *pList->pulCurrent++ =
        BCHP_FIELD_DATA(MASK_0_SCALE_4_5_6_7, MULT_4,  0 ) |
        BCHP_FIELD_DATA(MASK_0_SCALE_4_5_6_7, SHIFT_4, 4 ) |
        BCHP_FIELD_DATA(MASK_0_SCALE_4_5_6_7, MULT_5,  0 ) |
        BCHP_FIELD_DATA(MASK_0_SCALE_4_5_6_7, SHIFT_5, 4 ) |
        BCHP_FIELD_DATA(MASK_0_SCALE_4_5_6_7, MULT_6,  0 ) |
        BCHP_FIELD_DATA(MASK_0_SCALE_4_5_6_7, SHIFT_6, 4 ) |
        BCHP_FIELD_DATA(MASK_0_SCALE_4_5_6_7, MULT_7,  0 ) |
        BCHP_FIELD_DATA(MASK_0_SCALE_4_5_6_7, SHIFT_7, 4 );
#else
    BSTD_UNUSED(hWindow);
    BSTD_UNUSED(pList);
#endif

    return;
}


/*************************************************************************
 *  {secret}
 *  BVDC_P_Pep_BuildVsyncRul_isr
 *  Builds LAB contrast stretch every vsync
 **************************************************************************/
static void BVDC_P_Pep_BuildVsyncRul_isr
    ( const BVDC_Window_Handle     hWindow,
      BVDC_P_Pep_Handle            hPep,
      BVDC_P_ListInfo             *pList )
{
    BDBG_ENTER(BVDC_P_Pep_BuildVsyncRul_isr);

    if(!hWindow)
    {
        BDBG_LEAVE(BVDC_P_Pep_BuildVsyncRul_isr);
        return;
    }

    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    if(hWindow->bTntAvail)
    {
        BVDC_P_Tnt_BuildVysncRul_isr(hWindow, pList);
    }

    if(hWindow->eId != BVDC_P_WindowId_eComp0_V0)
    {
        /* Only C0_V0 has PEP */
        BDBG_LEAVE(BVDC_P_Pep_BuildVsyncRul_isr);
        return;
    }
    BDBG_OBJECT_ASSERT(hPep, BVDC_PEP);

#if(BVDC_P_SUPPORT_HIST)
    /* Process HIST data here */
    /* Old PEP arch, histogram data is read every frame */
    if(hPep->bProcessHist == true)
    {
        /* Build RUL to reset histogram every other field and collect  */
        /* the histogram the alternate field */
        BVDC_P_Pep_ReadHistoData_isr(hWindow, hPep);

        /* Reset histogram and min_max */
        *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
        *pList->pulCurrent++ = BRDC_REGISTER(BCHP_PEP_CMP_0_V0_HISTO_RESET);
        *pList->pulCurrent++ = 1;

        *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
        *pList->pulCurrent++ = BRDC_REGISTER(BCHP_PEP_CMP_0_V0_MIN_MAX_RESET);
        *pList->pulCurrent++ = 1;

        /* Enable HISTO_CTRL and MIN_MAX_CTRL every other vsync since */
        /* these registers are cleared automatically */
        *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
        *pList->pulCurrent++ = BRDC_REGISTER(BCHP_PEP_CMP_0_V0_HISTO_CTRL);
        *pList->pulCurrent++ = BCHP_FIELD_ENUM(PEP_CMP_0_V0_HISTO_CTRL, HISTO_CTRL, SINGLE_PICTURE);

        *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
        *pList->pulCurrent++ = BRDC_REGISTER(BCHP_PEP_CMP_0_V0_MIN_MAX_CTRL);
        *pList->pulCurrent++ = BCHP_FIELD_ENUM(PEP_CMP_0_V0_MIN_MAX_CTRL, MIN_MAX_CTRL, SINGLE_PICTURE);

        hPep->bProcessHist = false;
    }
    else
    {
        hPep->bProcessHist = true;
    }
#else
    BSTD_UNUSED(hPep);
    BSTD_UNUSED(pList);
#endif /* BVDC_P_SUPPORT_HIST */

    BDBG_LEAVE(BVDC_P_Pep_BuildVsyncRul_isr);
    return;
}

/*************************************************************************
 *  {secret}
 *  BVDC_P_Pep_BuildRul_isr
 *  Builds PEP block
 **************************************************************************/
void BVDC_P_Pep_BuildRul_isr
    ( const BVDC_Window_Handle     hWindow,
      BVDC_P_ListInfo             *pList,
      bool                         bInitial )
{
    BVDC_P_Window_DirtyBits       *pCurDirty;

    BDBG_ENTER(BVDC_P_Pep_BuildRul_isr);
    if(!hWindow)
    {
        BDBG_LEAVE(BVDC_P_Pep_BuildRul_isr);
        return;
    }

    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);
    pCurDirty = &hWindow->stCurInfo.stDirty;

    if(hWindow->eId == BVDC_P_WindowId_eComp0_V0)
    {
        /* Only C0_V0 has PEP */
        BDBG_OBJECT_ASSERT(hWindow->stCurResource.hPep, BVDC_PEP);
        bInitial |= hWindow->stCurResource.hPep->bInitial;

#if (BVDC_P_SUPPORT_HIST)
        if(pCurDirty->stBits.bHistoRect || bInitial)
        {
            BVDC_P_Histo_BuildRul_isr(hWindow, hWindow->hCompositor->stCurInfo.pFmtInfo->bInterlaced, pList);
            pCurDirty->stBits.bHistoRect = BVDC_P_CLEAN;
        }
#endif

        /* never dirty since hardware doesn't exist */
        pCurDirty->stBits.bLabAdjust = BVDC_P_CLEAN;
        pCurDirty->stBits.bCabAdjust = BVDC_P_CLEAN;
    }

    if(hWindow->bTntAvail && (pCurDirty->stBits.bTntAdjust|| bInitial))
    {
        if(bInitial)
        {
            BVDC_P_Tnt_BuildInit_isr(hWindow, pList);
        }
        BVDC_P_Tnt_BuildRul_isr(hWindow, pList);
        pCurDirty->stBits.bTntAdjust = BVDC_P_CLEAN;
    }

    if(hWindow->bMaskAvail && (pCurDirty->stBits.bDitAdjust || bInitial))
    {
        BVDC_P_Dither_BuildRul_isr(hWindow, pList);
        pCurDirty->stBits.bDitAdjust = BVDC_P_CLEAN;
    }

    BVDC_P_Pep_BuildVsyncRul_isr(hWindow, hWindow->stCurResource.hPep, pList);
    if(hWindow->eId == BVDC_P_WindowId_eComp0_V0)
    {
        hWindow->stCurResource.hPep->bInitial   = false;
        hWindow->stCurResource.hPep->bHardStart = false;
    }

    BDBG_LEAVE(BVDC_P_Pep_BuildRul_isr);
    return;
}


/*************************************************************************
 *  {secret}
 *  BVDC_P_Pep_SetInfo_isr
 *  Set info for PEP block
 **************************************************************************/
void BVDC_P_Pep_SetInfo_isr
    ( BVDC_P_Pep_Handle            hPep,
      BVDC_P_PictureNode          *pPicture )
{
    uint32_t               ulHSize, ulVSize;

    BDBG_OBJECT_ASSERT(hPep, BVDC_PEP);
    ulHSize = pPicture->stSclCut.ulWidth;
    ulVSize = (BAVC_Polarity_eFrame == pPicture->PicComRulInfo.eSrcOrigPolarity)?
        pPicture->stSclCut.ulHeight: pPicture->stSclCut.ulHeight / 2;

    if ((ulHSize != hPep->ulPrevHSize) || (ulVSize != hPep->ulPrevVSize) ||
        (pPicture->eSrcOrientation  != hPep->ePrevSrcOrientation) ||
        (pPicture->eDispOrientation != hPep->ePrevDispOrientation))
    {
        hPep->ulPrevHSize = ulHSize;
        hPep->ulPrevVSize = ulVSize;
        hPep->ePrevSrcOrientation  = pPicture->eSrcOrientation;
        hPep->ePrevDispOrientation = pPicture->eDispOrientation;

        hPep->bHardStart = true;
    }
}

/* End of File */
