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
 * [File Description:]
 *
 ***************************************************************************/
#include "bvdc_compositor_priv.h"
#include "bvdc_source_priv.h"
#include "bvdc_pep_priv.h"
#include "bvdc_tnt_priv.h"
#include "bvdc_hist_priv.h"

BDBG_MODULE(BVDC_PEP);
BDBG_OBJECT_ID(BVDC_PEP);

#if (BVDC_P_SUPPORT_MASK_DITHER)
#include "bchp_mask_0.h"
#endif

#if (BVDC_P_SUPPORT_PEP)

/*************************************************************************
 *  {secret}
 *  BVDC_P_Cab_BuildRul_isr
 *  Builds CAB block
 **************************************************************************/
static void BVDC_P_Cab_BuildRul_isr
    ( const BVDC_Window_Handle     hWindow,
      BVDC_P_Pep_Handle            hPep,
      BVDC_P_ListInfo             *pList )
{
    const BVDC_P_Window_Info    *pCurInfo;
    bool                         bCabEnable = false;
    bool                         bDemoModeEnable = false;

    BDBG_ENTER(BVDC_P_Cab_BuildRul_isr);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);
    BDBG_OBJECT_ASSERT(hPep, BVDC_PEP);

    /* Get current pointer to RUL */
    pCurInfo = &hWindow->stCurInfo;

    /* Only enable CAB if there is CAB table available */
    /* right now only 10 bits CAB tables are supported */
    bCabEnable = ((pCurInfo->ulFleshtone    != 0) ||
                  (pCurInfo->ulBlueBoost  != 0) ||
                  (pCurInfo->ulGreenBoost != 0) ||
                  (BVDC_P_PEP_CMS_IS_ENABLE(&pCurInfo->stSatGain, &pCurInfo->stHueGain)) ||
                  (pCurInfo->bUserCabEnable)) ? true : false;
    bDemoModeEnable = (((pCurInfo->ulFleshtone    != 0 ||
                         pCurInfo->ulBlueBoost  != 0 ||
                         pCurInfo->ulGreenBoost != 0) &&
                        (pCurInfo->stSplitScreenSetting.eAutoFlesh != BVDC_SplitScreenMode_eDisable)) ||
                       (BVDC_P_PEP_CMS_IS_ENABLE(&pCurInfo->stSatGain, &pCurInfo->stHueGain) &&
                        pCurInfo->stSplitScreenSetting.eCms != BVDC_SplitScreenMode_eDisable));
    BDBG_MSG(("User CAB table = %s, CAB_ENABLE = %s, DEMO_MODE = %s",
        (pCurInfo->bUserCabEnable == true) ? "true" : "false",
        (bCabEnable == true) ? "true" : "false",
        (bDemoModeEnable == true) ? "true" : "false"));

    BDBG_MSG(("ulFleshtone %d", pCurInfo->ulFleshtone));
    BDBG_MSG(("ulBlueBoost %d", pCurInfo->ulBlueBoost));
    BDBG_MSG(("ulGreenBoost %d", pCurInfo->ulGreenBoost));
    BDBG_MSG(("stSatGain B %d C %d Y %d G %d M %d R %d ", pCurInfo->stSatGain.lBlue, pCurInfo->stSatGain.lCyan, pCurInfo->stSatGain.lYellow, pCurInfo->stSatGain.lGreen, pCurInfo->stSatGain.lMagenta, pCurInfo->stSatGain.lRed));
    BDBG_MSG(("stHueGain B %d C %d Y %d G %d M %d R %d ", pCurInfo->stHueGain.lBlue, pCurInfo->stHueGain.lCyan, pCurInfo->stHueGain.lYellow, pCurInfo->stHueGain.lGreen, pCurInfo->stHueGain.lMagenta, pCurInfo->stHueGain.lRed));
    BDBG_MSG(("eAutoFlesh %d", pCurInfo->stSplitScreenSetting.eAutoFlesh));
    BDBG_MSG(("stSplitScreenSetting.eCms  %d", pCurInfo->stSplitScreenSetting.eCms));
    BDBG_MSG(("ulFleshtone %d", pCurInfo->ulFleshtone));


    /* Demo mode setting for Auto Flesh, Blue Boost and Green Boost */
    /* has to be the same, since they are applied to the same CAB table */
    BDBG_ASSERT(pCurInfo->stSplitScreenSetting.eAutoFlesh == pCurInfo->stSplitScreenSetting.eGreenBoost);
    BDBG_ASSERT(pCurInfo->stSplitScreenSetting.eAutoFlesh == pCurInfo->stSplitScreenSetting.eBlueBoost);

    /* Build RUL */
    *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_PEP_CMP_0_V0_CAB_CTRL);
    *pList->pulCurrent++ =
        BCHP_FIELD_DATA(PEP_CMP_0_V0_CAB_CTRL, CAB_DEMO_ENABLE, bDemoModeEnable) |
        BCHP_FIELD_DATA(PEP_CMP_0_V0_CAB_CTRL, CAB_ENABLE, bCabEnable) |
        BCHP_FIELD_DATA(PEP_CMP_0_V0_CAB_CTRL, LUMA_OFFSET_ENABLE, 0);

    /* Setting up the demo mode register */
    if(bDemoModeEnable)
    {
        uint32_t ulBoundary = hWindow->stAdjDstRect.ulWidth / 2;
        uint32_t ulDemoSide;

        if(BVDC_P_PEP_CMS_IS_ENABLE(&pCurInfo->stSatGain, &pCurInfo->stHueGain) &&
           pCurInfo->stSplitScreenSetting.eCms != BVDC_SplitScreenMode_eDisable)
        {
            ulDemoSide = (pCurInfo->stSplitScreenSetting.eCms == BVDC_SplitScreenMode_eLeft) ?
                               BCHP_PEP_CMP_0_V0_CAB_DEMO_SETTING_DEMO_L_R_LEFT :
                               BCHP_PEP_CMP_0_V0_CAB_DEMO_SETTING_DEMO_L_R_RIGHT;
        }
        else
        {
            ulDemoSide = (pCurInfo->stSplitScreenSetting.eAutoFlesh == BVDC_SplitScreenMode_eLeft) ?
                               BCHP_PEP_CMP_0_V0_CAB_DEMO_SETTING_DEMO_L_R_LEFT :
                               BCHP_PEP_CMP_0_V0_CAB_DEMO_SETTING_DEMO_L_R_RIGHT;
        }

        *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
        *pList->pulCurrent++ = BRDC_REGISTER(BCHP_PEP_CMP_0_V0_CAB_DEMO_SETTING);
        *pList->pulCurrent++ =
            BCHP_FIELD_DATA(PEP_CMP_0_V0_CAB_DEMO_SETTING, DEMO_L_R, ulDemoSide) |
            BCHP_FIELD_DATA(PEP_CMP_0_V0_CAB_DEMO_SETTING, DEMO_BOUNDARY, ulBoundary) ;

        BDBG_MSG(("CAB Demo Mode: L_R = %s, BOUNDARY = %d",
                  ((ulDemoSide == BCHP_PEP_CMP_0_V0_CAB_DEMO_SETTING_DEMO_L_R_LEFT) ? "L" : "R"),
                  ulBoundary));
    }

    if(bCabEnable)
    {
        /* Set flag to load user CAB table to the HW when the slot is avaiable */
        hPep->bLoadCabTable = true;
        /* Force CAB table to be loaded in same vsync cycle with CAB enable */
        hPep->bProcessCab   = true;
    }

    BDBG_LEAVE(BVDC_P_Cab_BuildRul_isr);
    return;
}


/*************************************************************************
 *  {secret}
 *  BVDC_P_Lab_BuildRul_isr
 *  Builds LAB block
 **************************************************************************/
static void BVDC_P_Lab_BuildRul_isr
    ( const BVDC_Window_Handle     hWindow,
      BVDC_P_Pep_Handle            hPep,
      BVDC_P_ListInfo             *pList )
{
    const BVDC_P_Window_Info    *pCurInfo;

    BDBG_ENTER(BVDC_P_Lab_BuildRul_isr);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);
    BDBG_OBJECT_ASSERT(hPep, BVDC_PEP);

    /* Get current pointer to RUL */
    pCurInfo = &hWindow->stCurInfo;

    /* Setting up the demo mode register */
    if(pCurInfo->stSplitScreenSetting.eContrastStretch != BVDC_SplitScreenMode_eDisable)
    {
        *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
        *pList->pulCurrent++ = BRDC_REGISTER(BCHP_PEP_CMP_0_V0_LAB_DEMO_SETTING);
        *pList->pulCurrent++ =
            BCHP_FIELD_DATA(PEP_CMP_0_V0_LAB_DEMO_SETTING, DEMO_L_R,
                           (pCurInfo->stSplitScreenSetting.eContrastStretch == BVDC_SplitScreenMode_eLeft) ?
                            BCHP_PEP_CMP_0_V0_LAB_DEMO_SETTING_DEMO_L_R_LEFT :
                            BCHP_PEP_CMP_0_V0_LAB_DEMO_SETTING_DEMO_L_R_RIGHT) |
            BCHP_FIELD_DATA(PEP_CMP_0_V0_LAB_DEMO_SETTING, DEMO_BOUNDARY,
                            hWindow->stAdjDstRect.ulWidth / 2) ;

        BDBG_MSG(("LAB Demo Mode: L_R = %s, BOUNDARY = %d",
                 (pCurInfo->stSplitScreenSetting.eContrastStretch == BVDC_SplitScreenMode_eLeft) ? "L" : "R",
                  hWindow->stAdjDstRect.ulWidth / 2));
    }

    /* Defer LAB enable if CAB table is being processed this vsync */
    hPep->bLabCtrlPending = ((pCurInfo->bContrastStretch || pCurInfo->bUserLabLuma) &&
                              hPep->bProcessCab) ? true : false;

    if(!hPep->bLabCtrlPending)
    {
        *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
        *pList->pulCurrent++ = BRDC_REGISTER(BCHP_PEP_CMP_0_V0_LAB_CTRL);
        *pList->pulCurrent++ =
            ((pCurInfo->stSplitScreenSetting.eContrastStretch == BVDC_SplitScreenMode_eDisable) ?
              BCHP_FIELD_DATA(PEP_CMP_0_V0_LAB_CTRL, LAB_DEMO_ENABLE, 0) :
              BCHP_FIELD_DATA(PEP_CMP_0_V0_LAB_CTRL, LAB_DEMO_ENABLE, 1)) |
            ((pCurInfo->bContrastStretch || pCurInfo->bBlueStretch ||
              pCurInfo->bUserLabLuma || pCurInfo->bUserLabCbCr) ?
              BCHP_FIELD_DATA(PEP_CMP_0_V0_LAB_CTRL, ENABLE, 1):
              BCHP_FIELD_DATA(PEP_CMP_0_V0_LAB_CTRL, ENABLE, 0));

        if(pCurInfo->bUserLabLuma || pCurInfo->bUserLabCbCr ||
           (pCurInfo->bBlueStretch && !pCurInfo->bUserLabCbCr))
        {
            /* Set flag to load user LAB table to the HW when the slot is avaiable */
            hPep->bLoadLabTable = true;
            /* Set flag to delay CAB table loaded one vsync so that the first LAB */
            /* table can be loaded in one vsync with LAB enable */
            hPep->bProcessCab   = false;
        }

        BDBG_MSG(("Contrast %s, Blue %s, Usr luma %s, Usr CbCr %s, load tbl %s",
            (pCurInfo->bContrastStretch) ? "enabled" : "disabled",
            (pCurInfo->bBlueStretch) ? "enabled" : "disabled",
            (pCurInfo->bUserLabLuma) ? "enabled" : "disabled",
            (pCurInfo->bUserLabCbCr) ? "enabled" : "disabled",
            (hPep->bLoadLabTable) ? "enabled" : "disabled"));
    }

    BDBG_LEAVE(BVDC_P_Lab_BuildRul_isr);
    return;
}

#endif /* (BVDC_P_SUPPORT_PEP) */


#if(BVDC_P_SUPPORT_HIST)
#if(BVDC_P_SUPPORT_HIST_VER == BVDC_P_SUPPORT_HIST_VER_1)
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
      BVDC_P_Pep_Handle            hPep,
      const BVDC_P_HistContext    *pHist )
{
    uint32_t id;

    BSTD_UNUSED(pHist);
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

#endif /* BVDC_P_SUPPORT_HIST_VER */
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
    BKNI_Memset((void*)hPep->aulLabTable,      0x0, sizeof(hPep->aulLabTable));
    BKNI_Memset((void*)hPep->aulLastBin,       0x0, sizeof(hPep->aulLastBin));
    BKNI_Memset((void*)hPep->aulLastLastBin,   0x0, sizeof(hPep->aulLastLastBin));
    BKNI_Memset((void*)hPep->lFixEstLuma,      0x0, sizeof(hPep->lFixEstLuma));
    BKNI_Memset((void*)hPep->lFixHist_out,     0x0, sizeof(hPep->lFixHist_out));
    BKNI_Memset((void*)&hPep->stHistoData,     0x0, sizeof(BVDC_LumaStatus));
    BKNI_Memset((void*)&hPep->stTmpHistoData,  0x0, sizeof(BVDC_LumaStatus));
    BKNI_Memset((void*)&hPep->ulAvgLevelStats, 0x0, sizeof(hPep->ulAvgLevelStats));

    hPep->bLabTableValid = false;
    hPep->bProcessHist   = false;
    hPep->bLoadCabTable  = false;
    hPep->bLoadLabTable  = false;
    hPep->bProcessCab    = true;
    hPep->lFixLastMin    = 0;
    hPep->lFixLastMax    = 0;
    hPep->lFixLastMid    = 0;
    hPep->lFixBrtCur     = 0;
    hPep->lFixBrtLast    = 0;

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
    hPep->ulLumaChromaGain = 0x3f;

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
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_MASK_0_CONTROL);
    *pList->pulCurrent++ =
        BCHP_FIELD_ENUM(MASK_0_CONTROL, TEXTURE_ENABLE, DISABLE ) |
        BCHP_FIELD_DATA(MASK_0_CONTROL, REDUCE_SMOOTH,
            pDither->bReduceSmooth                              ) |
        BCHP_FIELD_DATA(MASK_0_CONTROL, SMOOTH_ENABLE,
            pDither->bSmoothEnable                              );

    *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_MASK_0_SMOOTH_LIMIT);
    *pList->pulCurrent++ =
        BCHP_FIELD_DATA(MASK_0_SMOOTH_LIMIT, VALUE,
            pDither->ulSmoothLimit                              );

    *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_MASK_0_TEXTURE_FREQ);
    *pList->pulCurrent++ =
        BCHP_FIELD_DATA(MASK_0_TEXTURE_FREQ, HORIZ_FREQ, 8 ) |
        BCHP_FIELD_DATA(MASK_0_TEXTURE_FREQ, VERT_FREQ,  8 );

    *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_MASK_0_RANDOM_PATTERN);
    *pList->pulCurrent++ =
        BCHP_FIELD_ENUM(MASK_0_RANDOM_PATTERN, RNG_MODE, RUN  ) |
        BCHP_FIELD_DATA(MASK_0_RANDOM_PATTERN, RNG_SEED, 4369 );

    *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_MASK_0_SCALE_0_1_2_3);
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
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_MASK_0_SCALE_4_5_6_7);
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
    const BVDC_P_Window_Info *pCurInfo;

    BDBG_ENTER(BVDC_P_Pep_BuildVsyncRul_isr);

    if(!hWindow)
    {
        BDBG_LEAVE(BVDC_P_Pep_BuildVsyncRul_isr);
        return;
    }

    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);
    BDBG_OBJECT_ASSERT(hPep, BVDC_PEP);

    /* Get current pointer to RUL */
    pCurInfo = &hWindow->stCurInfo;

    BVDC_P_Tnt_BuildVysncRul_isr(hWindow, pList);


#if(BVDC_P_SUPPORT_HIST)
#if(BVDC_P_SUPPORT_HIST_VER <= BVDC_P_SUPPORT_HIST_VER_1)
    /* Process HIST data here */
    /* Old PEP arch, histogram data is read every frame */
    if(hPep->bProcessHist == true)
    {
        /* Build RUL to reset histogram every other field and collect  */
        /* the histogram the alternate field */
        BVDC_P_Pep_ReadHistoData_isr(hWindow, hPep, NULL);

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
#endif /* BVDC_P_SUPPORT_HIST_VER */
#endif /* BVDC_P_SUPPORT_HIST */

#if(BVDC_P_SUPPORT_PEP)
    /* We are alternating the service slot for CAB and LAB table every other */
    /* field to avoid overloading the RUL, so at the max each Vsync, there   */
    /* will be 1024 entries from ther LAB or CAB table loaded */
    if(hPep->bProcessCab == true)
    {
        hPep->bProcessCab = false;
        if(hPep->bLoadCabTable)
        {
            BDBG_MSG(("Build RUL to load CAB table"));
            *pList->pulCurrent++ = BRDC_OP_IMMS_TO_REGS(BVDC_P_CAB_TABLE_SIZE);
            *pList->pulCurrent++ = BRDC_REGISTER(BCHP_PEP_CMP_0_V0_CAB_LUT_DATA_i_ARRAY_BASE);
            BKNI_Memcpy((void*)pList->pulCurrent, (void*)&pCurInfo->pulCabTable, BVDC_P_CAB_TABLE_SIZE * sizeof(uint32_t));
            pList->pulCurrent += BVDC_P_CAB_TABLE_SIZE;
            hPep->bLoadCabTable = false;
        }
    }
    else
    {
        uint32_t id = 0;

        if(pCurInfo->bContrastStretch || hPep->bLoadLabTable)
        {
            *pList->pulCurrent++ = BRDC_OP_IMMS_TO_REGS(BVDC_P_LAB_TABLE_SIZE);
            *pList->pulCurrent++ = BRDC_REGISTER(BCHP_PEP_CMP_0_V0_LAB_LUT_DATA_i_ARRAY_BASE);
            if(pCurInfo->bContrastStretch == true)
            {
                /* compute LAB table if contrast stretch is enabled */
                BVDC_P_Pep_DynamicContrast_isr(&pCurInfo->stContrastStretch,
                                               hPep,
                                               &hPep->aulLabTable[0]);

                if(pCurInfo->bUserLabCbCr || pCurInfo->bBlueStretch)
                {
                    for(id = 0; id < BVDC_P_LAB_TABLE_SIZE; id++)
                    {
#if (BVDC_P_SUPPORT_PEP_VER_5>BVDC_P_SUPPORT_PEP_VER)
                        hPep->aulLabTable[id] &= ~(
                            BCHP_MASK(PEP_CMP_0_V0_LAB_LUT_DATA_i, CB_OFFSET) |
                            BCHP_MASK(PEP_CMP_0_V0_LAB_LUT_DATA_i, CR_OFFSET));
                        if(pCurInfo->bUserLabCbCr)
                        {
                            /* Merge with user Cb and Cr tables */
                            hPep->aulLabTable[id] |= (
                                BCHP_FIELD_DATA(PEP_CMP_0_V0_LAB_LUT_DATA_i, CB_OFFSET, *(pCurInfo->pulLabCbTbl + id)) |
                                BCHP_FIELD_DATA(PEP_CMP_0_V0_LAB_LUT_DATA_i, CR_OFFSET, *(pCurInfo->pulLabCrTbl + id)));
                        }
#endif
                    }
                }
            } else if(hPep->bLoadLabTable)
            {
                BDBG_MSG(("Build RUL to load LAB table"));

                for(id = 0; id < BVDC_P_LAB_TABLE_SIZE; id++)
                {
                    if(pCurInfo->bUserLabLuma)
                    {
                        /* Merge with user luma table */
                        hPep->aulLabTable[id] &= ~(
                            BCHP_MASK(PEP_CMP_0_V0_LAB_LUT_DATA_i, LUMA_DATA));
                        hPep->aulLabTable[id] = (
                            BCHP_FIELD_DATA(PEP_CMP_0_V0_LAB_LUT_DATA_i, LUMA_DATA, *(pCurInfo->pulLabLumaTbl + id)));
                    }
                    else if(pCurInfo->bBlueStretch)
                    {
                        /* use identity for luma table */
                        hPep->aulLabTable[id] &= ~(
                            BCHP_MASK(PEP_CMP_0_V0_LAB_LUT_DATA_i, LUMA_DATA));
                        hPep->aulLabTable[id] = (
                            BCHP_FIELD_DATA(PEP_CMP_0_V0_LAB_LUT_DATA_i, LUMA_DATA, id));
                    }
                    if(pCurInfo->bUserLabCbCr)
                    {
#if (BVDC_P_SUPPORT_PEP_VER_5>BVDC_P_SUPPORT_PEP_VER)
                        /* Merge with user Cb and Cr tables */
                        hPep->aulLabTable[id] &= ~(
                            BCHP_MASK(PEP_CMP_0_V0_LAB_LUT_DATA_i, CB_OFFSET) |
                            BCHP_MASK(PEP_CMP_0_V0_LAB_LUT_DATA_i, CR_OFFSET));
                        hPep->aulLabTable[id] |= (
                            BCHP_FIELD_DATA(PEP_CMP_0_V0_LAB_LUT_DATA_i, CB_OFFSET, *(pCurInfo->pulLabCbTbl + id)) |
                            BCHP_FIELD_DATA(PEP_CMP_0_V0_LAB_LUT_DATA_i, CR_OFFSET, *(pCurInfo->pulLabCrTbl + id)));
#endif
                    }
                }
                hPep->bLoadLabTable = false;
            }

            BKNI_Memcpy((void*)pList->pulCurrent, (void*)&hPep->aulLabTable[0], BVDC_P_LAB_TABLE_SIZE * sizeof(uint32_t));
            pList->pulCurrent += BVDC_P_LAB_TABLE_SIZE;
        }

        hPep->bProcessCab = true;
    }

#else
    BSTD_UNUSED(hPep);
    BSTD_UNUSED(pList);
#endif /* BVDC_P_SUPPORT_PEP */

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

    /* Only C0_V0 has PEP */
    BDBG_ASSERT(BVDC_P_WindowId_eComp0_V0 == hWindow->eId);

    BDBG_OBJECT_ASSERT(hWindow->stCurResource.hPep, BVDC_PEP);
    bInitial |= hWindow->stCurResource.hPep->bInitial;

#if (BVDC_P_SUPPORT_HIST && BVDC_P_SUPPORT_HIST_VER <= BVDC_P_SUPPORT_HIST_VER_1)
    if(pCurDirty->stBits.bHistoRect || bInitial)
    {
        BVDC_P_Histo_BuildRul_isr(hWindow, hWindow->hCompositor->stCurInfo.pFmtInfo->bInterlaced, pList);
        pCurDirty->stBits.bHistoRect = BVDC_P_CLEAN;
    }
#endif

#if (BVDC_P_SUPPORT_PEP)
    if(hWindow->stCurResource.hPep->bHardStart)
    {
        BVDC_P_Pep_DcInit_isr(hWindow->stCurResource.hPep);
    }

    if(pCurDirty->stBits.bCabAdjust || bInitial)
    {
        BVDC_P_Cab_BuildRul_isr(hWindow, hWindow->stCurResource.hPep, pList);
        pCurDirty->stBits.bCabAdjust = BVDC_P_CLEAN;
    }

    if(pCurDirty->stBits.bLabAdjust || bInitial || hWindow->stCurResource.hPep->bLabCtrlPending)
    {
        BVDC_P_Lab_BuildRul_isr(hWindow, hWindow->stCurResource.hPep, pList);
        pCurDirty->stBits.bLabAdjust = BVDC_P_CLEAN;
    }

#else
    /* never dirty since hardware doesn't exist */
    pCurDirty->stBits.bLabAdjust = BVDC_P_CLEAN;
    pCurDirty->stBits.bCabAdjust = BVDC_P_CLEAN;
#endif

    if(pCurDirty->stBits.bTntAdjust|| bInitial)
    {
        if(bInitial)
        {
            BVDC_P_Tnt_BuildInit_isr(hWindow, pList);
        }
        BVDC_P_Tnt_BuildRul_isr(hWindow, pList);
        pCurDirty->stBits.bTntAdjust = BVDC_P_CLEAN;
    }

    if(pCurDirty->stBits.bDitAdjust || bInitial)
    {
        BVDC_P_Dither_BuildRul_isr(hWindow, pList);
        pCurDirty->stBits.bDitAdjust = BVDC_P_CLEAN;
    }

    BVDC_P_Pep_BuildVsyncRul_isr(hWindow, hWindow->stCurResource.hPep, pList);
    hWindow->stCurResource.hPep->bInitial   = false;
    hWindow->stCurResource.hPep->bHardStart = false;

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

/*************************************************************************
 *  {secret}
 *  BVDC_P_Pep_GetLumaStatus
 *  Get Luma status from PEP block
 **************************************************************************/
void BVDC_P_Pep_GetLumaStatus
    ( const BVDC_Window_Handle     hWindow,
      BVDC_LumaStatus             *pLumaStatus )
{
    BDBG_ENTER(BVDC_P_Pep_GetLumaStatus);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);
    BDBG_OBJECT_ASSERT(hWindow->stCurResource.hPep, BVDC_PEP);

    BKNI_EnterCriticalSection();
    *pLumaStatus = hWindow->stCurResource.hPep->stHistoData;
    BKNI_LeaveCriticalSection();

    BDBG_LEAVE(BVDC_P_Pep_GetLumaStatus);
    return;
}


/*************************************************************************
 *  {secret}
 *  BVDC_P_Tab_BuildRul_isr
 *  Builds TAB block
 **************************************************************************/
void BVDC_P_Tab_BuildRul_isr
    ( const BVDC_Window_Handle     hWindow,
      BVDC_P_ListInfo             *pList )
{
    BVDC_P_Window_Info    *pCurInfo;

    BDBG_ENTER(BVDC_P_Tab_BuildRul_isr);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    /* Only C0_V0 has PEP */
    BDBG_ASSERT(BVDC_P_WindowId_eComp0_V0 == hWindow->eId);

    /* Get current pointer to RUL */
    pCurInfo = &hWindow->stCurInfo;

#if (BVDC_P_SUPPORT_TAB)
    if(pCurInfo->bSharpnessEnable)
    {
        BDBG_MSG(("TAB Sharpness = %d => Peak Setting = %d, Peak Scale = %d",
                   pCurInfo->sSharpness, pCurInfo->ulSharpnessPeakSetting,
                   pCurInfo->ulSharpnessPeakScale));

        /* Build RUL */
        *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
        *pList->pulCurrent++ = BRDC_REGISTER(BCHP_TAB_0_CONFIGURATION_REG);
        *pList->pulCurrent++ =
            ((pCurInfo->stSplitScreenSetting.eSharpness == BVDC_SplitScreenMode_eDisable) ?
            BCHP_FIELD_ENUM(TAB_0_CONFIGURATION_REG, DEMO_ENABLE, DISABLE) :
            BCHP_FIELD_ENUM(TAB_0_CONFIGURATION_REG, DEMO_ENABLE, ENABLE)) |
            BCHP_FIELD_ENUM(TAB_0_CONFIGURATION_REG, TAB_ENABLE, NORMAL) |
            BCHP_FIELD_ENUM(TAB_0_CONFIGURATION_REG, LUMA_MEDIAN, ONE_TAP_LUMA_MEDIAN) |
            BCHP_FIELD_ENUM(TAB_0_CONFIGURATION_REG, CHROMA_MEDIAN, THREE_TAP_CHROMA_MEDIAN) |
            BCHP_FIELD_ENUM(TAB_0_CONFIGURATION_REG, DISABLE_CHROMA_LUMA, DISABLE) |
            BCHP_FIELD_DATA(TAB_0_CONFIGURATION_REG, CHROMA_LUMA_SETTING, 0x0) |
            BCHP_FIELD_ENUM(TAB_0_CONFIGURATION_REG, DISABLE_CHROMA_CHROMA, DISABLE) |
            BCHP_FIELD_DATA(TAB_0_CONFIGURATION_REG, CHROMA_CHROMA_SETTING, 0x0);

        *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
        *pList->pulCurrent++ = BRDC_REGISTER(BCHP_TAB_0_LUMA_PEAK_CORRECTION_REG);
        *pList->pulCurrent++ =
            BCHP_FIELD_DATA(TAB_0_LUMA_PEAK_CORRECTION_REG, OLD_CORE_FUNCTION, 0)    |
            BCHP_FIELD_DATA(TAB_0_LUMA_PEAK_CORRECTION_REG, OVERSHOOT,         0xFF) |
            BCHP_FIELD_DATA(TAB_0_LUMA_PEAK_CORRECTION_REG, PEAKCORE,          0x10) |
            BCHP_FIELD_DATA(TAB_0_LUMA_PEAK_CORRECTION_REG, PEAK_SCALE,        pCurInfo->ulSharpnessPeakScale) |
            BCHP_FIELD_DATA(TAB_0_LUMA_PEAK_CORRECTION_REG, PEAK_SETTING,      pCurInfo->ulSharpnessPeakSetting);

        *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
        *pList->pulCurrent++ = BRDC_REGISTER(BCHP_TAB_0_LUMA_EDGE_ENHANCEMENT_REG);
        *pList->pulCurrent++ =
            BCHP_FIELD_DATA(TAB_0_LUMA_EDGE_ENHANCEMENT_REG, EDGECORE, 0x3) |
            BCHP_FIELD_DATA(TAB_0_LUMA_EDGE_ENHANCEMENT_REG, EDGEMAX, 0x14) |
            BCHP_FIELD_DATA(TAB_0_LUMA_EDGE_ENHANCEMENT_REG, EDGE_SCALE, 0x6) |
            BCHP_FIELD_DATA(TAB_0_LUMA_EDGE_ENHANCEMENT_REG, EDGE_SETTING, 0x0);

        *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
        *pList->pulCurrent++ = BRDC_REGISTER(BCHP_TAB_0_CHROMA_PEAKING_REG);
        *pList->pulCurrent++ =
            BCHP_FIELD_DATA(TAB_0_CHROMA_PEAKING_REG, SATCORE, 0x0) |
            BCHP_FIELD_DATA(TAB_0_CHROMA_PEAKING_REG, SATMAX, 0x14) |
            BCHP_FIELD_ENUM(TAB_0_CHROMA_PEAKING_REG, SAT_SETTING, DISABLE) |
            BCHP_FIELD_DATA(TAB_0_CHROMA_PEAKING_REG, SATSCALE, 0x6);

        *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
        *pList->pulCurrent++ = BRDC_REGISTER(BCHP_TAB_0_CHROMA_MODULATED_FILTER_REG);
        *pList->pulCurrent++ = 0x55150501;

        /* Setting up the demo mode register */
        if(pCurInfo->stSplitScreenSetting.eSharpness != BVDC_SplitScreenMode_eDisable)
        {
            uint32_t ulBoundary = (hWindow->stSrcCnt.ulWidth < hWindow->stAdjSclOut.ulWidth) ?
                                  (hWindow->stSrcCnt.ulWidth / 2) :
                                  (hWindow->stAdjSclOut.ulWidth / 2);
            uint32_t ulDemoSide = (pCurInfo->stSplitScreenSetting.eSharpness == BVDC_SplitScreenMode_eLeft) ?
                                   BCHP_TAB_0_DEMO_SETTING_REG_DEMO_L_R_LEFT :
                                   BCHP_TAB_0_DEMO_SETTING_REG_DEMO_L_R_RIGHT;

            *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
            *pList->pulCurrent++ = BRDC_REGISTER(BCHP_TAB_0_DEMO_SETTING_REG);
            *pList->pulCurrent++ =
                BCHP_FIELD_DATA(TAB_0_DEMO_SETTING_REG, DEMO_L_R, ulDemoSide) |
                BCHP_FIELD_DATA(TAB_0_DEMO_SETTING_REG, DEMO_BOUNDARY, ulBoundary);

            BDBG_MSG(("TAB Demo Mode: L_R = %s, BOUNDARY = %d",
                      ((ulDemoSide == BCHP_TAB_0_DEMO_SETTING_REG_DEMO_L_R_LEFT) ? "L" : "R"),
                      ulBoundary));
        }
    }
    else
    {
        /* Bypass TAB */
        *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
        *pList->pulCurrent++ = BRDC_REGISTER(BCHP_TAB_0_CONFIGURATION_REG);
        *pList->pulCurrent++ = (
            BCHP_FIELD_ENUM(TAB_0_CONFIGURATION_REG, DEMO_ENABLE, DISABLE) |
            BCHP_FIELD_ENUM(TAB_0_CONFIGURATION_REG, TAB_ENABLE, BYPASS_IN_OUT) |
            BCHP_FIELD_ENUM(TAB_0_CONFIGURATION_REG, DISABLE_CHROMA_LUMA, DISABLE) |
            BCHP_FIELD_ENUM(TAB_0_CONFIGURATION_REG, DISABLE_CHROMA_CHROMA, DISABLE));

        *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
        *pList->pulCurrent++ = BRDC_REGISTER(BCHP_TAB_0_CHROMA_PEAKING_REG);
        *pList->pulCurrent++ = (
            BCHP_FIELD_ENUM(TAB_0_CHROMA_PEAKING_REG, SAT_SETTING, DISABLE));
    }

#else
    BSTD_UNUSED(pList);
#endif /* (BVDC_P_SUPPORT_TAB) */

    pCurInfo->stDirty.stBits.bTabAdjust = false;

    BDBG_LEAVE(BVDC_P_Tab_BuildRul_isr);
    return;
}

/* End of File */
