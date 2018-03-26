/******************************************************************************
 *  Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.
 ******************************************************************************/
#include "bstd.h"
#include "bkni.h"
#include "bvdc.h"
#include "brdc.h"
#include "bvdc_scaler_priv.h"
#include "bvdc_buffer_priv.h"
#include "bvdc_source_priv.h"
#include "bvdc_window_priv.h"
#include "bvdc_compositor_priv.h"
#include "bvdc_pep_priv.h"
#include "bvdc_vnet_priv.h"
#include "bchp_mmisc.h"



BDBG_MODULE(BVDC_SCL);
BDBG_FILE_MODULE(BVDC_SCL_VSTEP);
BDBG_FILE_MODULE(BVDC_FIR_BYPASS);
BDBG_OBJECT_ID(BVDC_SCL);

/* SW7420-560, SW7420-721: use smoothen vertical coefficient to improve weaving */

#define BVDC_P_MAKE_SCALER(pScaler, id)                                                  \
{                                                                                        \
    (pScaler)->ulRegOffset      = BCHP_SCL_##id##_REG_START - BCHP_SCL_0_REG_START;      \
    (pScaler)->ulResetMask      = BCHP_MMISC_SW_INIT_SCL_##id##_MASK;                    \
    (pScaler)->ulVnetResetMask  = BCHP_MMISC_VNET_B_CHANNEL_SW_INIT_SCL_##id##_MASK;     \
}


/***************************************************************************
 * {private}
 *
 */
BERR_Code BVDC_P_Scaler_Create
    ( BVDC_P_Scaler_Handle            *phScaler,
      BVDC_P_ScalerId                  eScalerId,
      BVDC_P_Resource_Handle           hResource,
      BREG_Handle                      hReg )
{
    BVDC_P_ScalerContext *pScaler;

    BDBG_ENTER(BVDC_P_Scaler_Create);

    BDBG_ASSERT(phScaler);

    /* Use: to see messages */
    /* BDBG_SetModuleLevel("BVDC_SCL", BDBG_eMsg); */

    /* The handle will be NULL if create fails. */
    *phScaler = NULL;

    /* Alloc the context. */
    pScaler = (BVDC_P_ScalerContext*)
        (BKNI_Malloc(sizeof(BVDC_P_ScalerContext)));
    if(!pScaler)
    {
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }

    /* Clear out the context and set defaults. */
    BKNI_Memset((void*)pScaler, 0x0, sizeof(BVDC_P_ScalerContext));
    BDBG_OBJECT_SET(pScaler, BVDC_SCL);

    pScaler->eId          = eScalerId;

    pScaler->hReg = hReg;

    /* Init to the default filter coeffficient tables. */
    BVDC_P_GetFirCoeffs_isrsafe(&pScaler->pHorzFirCoeffTbl, &pScaler->pVertFirCoeffTbl);
    BVDC_P_GetChromaFirCoeffs_isrsafe(&pScaler->pChromaHorzFirCoeffTbl, &pScaler->pChromaVertFirCoeffTbl);

    /* Scaler reset address */
    pScaler->ulResetRegAddr = BCHP_MMISC_SW_INIT;
    pScaler->ulVnetResetAddr  = BCHP_MMISC_VNET_B_CHANNEL_SW_INIT;
    switch(pScaler->eId)
    {
        case BVDC_P_ScalerId_eScl0:
            BVDC_P_MAKE_SCALER(pScaler, 0);
            break;
#ifdef BCHP_SCL_1_REG_START
        case BVDC_P_ScalerId_eScl1:
            BVDC_P_MAKE_SCALER(pScaler, 1);
            break;
#endif
#ifdef BCHP_SCL_2_REG_START
        case BVDC_P_ScalerId_eScl2:
            BVDC_P_MAKE_SCALER(pScaler, 2);
            break;
#endif
#ifdef BCHP_SCL_3_REG_START
        case BVDC_P_ScalerId_eScl3:
            BVDC_P_MAKE_SCALER(pScaler, 3);
            break;
#endif
#ifdef BCHP_SCL_4_REG_START
        case BVDC_P_ScalerId_eScl4:
            BVDC_P_MAKE_SCALER(pScaler, 4);
            break;
#endif
#ifdef BCHP_SCL_5_REG_START
        case BVDC_P_ScalerId_eScl5:
            BVDC_P_MAKE_SCALER(pScaler, 5);
            break;
#endif
#ifdef BCHP_SCL_6_REG_START
        case BVDC_P_ScalerId_eScl6:
            BVDC_P_MAKE_SCALER(pScaler, 6);
            break;
#endif
#ifdef BCHP_SCL_7_REG_START
        case BVDC_P_ScalerId_eScl7:
            BVDC_P_MAKE_SCALER(pScaler, 7);
            break;
#endif
        default:
            BDBG_ERR(("Need to handle BVDC_P_ScalerId_eScl%d", pScaler->eId));
            BDBG_ASSERT(0);
            break;
    }

    /* init the SubRul sub-module */
    BVDC_P_SubRul_Init(&(pScaler->SubRul), BVDC_P_Scaler_MuxAddr(pScaler),
        BVDC_P_Scaler_PostMuxValue(pScaler), BVDC_P_DrainMode_eBack, 0, hResource);

    BVDC_P_Scaler_Init(pScaler);

    /* All done. now return the new fresh context to user. */
    *phScaler = (BVDC_P_Scaler_Handle)pScaler;

    BDBG_LEAVE(BVDC_P_Scaler_Create);
    return BERR_SUCCESS;
}


/***************************************************************************
 * {private}
 *
 */
void BVDC_P_Scaler_Destroy
    ( BVDC_P_Scaler_Handle             hScaler )
{
    BDBG_ENTER(BVDC_P_Scaler_Destroy);
    BDBG_OBJECT_ASSERT(hScaler, BVDC_SCL);

    BDBG_OBJECT_DESTROY(hScaler, BVDC_SCL);
    /* Release context in system memory */
    BKNI_Free((void*)hScaler);

    BDBG_LEAVE(BVDC_P_Scaler_Destroy);
    return;
}


/***************************************************************************
 * {private}
 *
 */
void BVDC_P_Scaler_Init
    ( BVDC_P_Scaler_Handle             hScaler )
{
    uint32_t  ulReg;
    uint32_t  ulTaps;
    uint32_t  ulVertLineDepth;

    BDBG_ENTER(BVDC_P_Scaler_Init);
    BDBG_OBJECT_ASSERT(hScaler, BVDC_SCL);

    hScaler->ulUpdateAll   = BVDC_P_RUL_UPDATE_THRESHOLD;
    hScaler->ulUpdateCoeff = BVDC_P_RUL_UPDATE_THRESHOLD;

    /* Clear out shadow registers. */
    BKNI_Memset((void*)hScaler->aulRegs, 0x0, sizeof(hScaler->aulRegs));

    /* Initialize state. */
    hScaler->bInitial          = true;
    hScaler->pPrevVertFirCoeff = NULL;

    ulReg = BREG_Read32(hScaler->hReg, BCHP_SCL_0_HW_CONFIGURATION + hScaler->ulRegOffset);

    hScaler->bDeJagging = BCHP_GET_FIELD_DATA(ulReg, SCL_0_HW_CONFIGURATION, DEJAGGING) ? true : false;
    hScaler->bDeRinging = BCHP_GET_FIELD_DATA(ulReg, SCL_0_HW_CONFIGURATION, DERINGING) ? true : false;

    ulTaps = BCHP_GET_FIELD_DATA(ulReg, SCL_0_HW_CONFIGURATION, HORIZ_TAPS);
    switch(ulTaps)
    {
        case BCHP_SCL_0_HW_CONFIGURATION_HORIZ_TAPS_HORIZ_8_TAPS:
            hScaler->ulHorzTaps = BVDC_P_CT_8_TAP;
            break;
        case BCHP_SCL_0_HW_CONFIGURATION_HORIZ_TAPS_HORIZ_12_TAPS:
            hScaler->ulHorzTaps = BVDC_P_CT_12_TAP;
            break;
        case BCHP_SCL_0_HW_CONFIGURATION_HORIZ_TAPS_HORIZ_16_TAPS:
            hScaler->ulHorzTaps = BVDC_P_CT_16_TAP;
            break;
        default:
            BDBG_ERR(("SCL[%d] Unknown horizontal tap configuration detected: %d", hScaler->eId, ulTaps));
            BDBG_ASSERT(0);
            break;
    }

    ulTaps = BCHP_GET_FIELD_DATA(ulReg, SCL_0_HW_CONFIGURATION, VERT_TAPS);
    switch(ulTaps)
    {
        case BCHP_SCL_0_HW_CONFIGURATION_VERT_TAPS_VERT_4_TAPS:
            hScaler->ulVertTaps = BVDC_P_CT_4_TAP;
            hScaler->ulVertBlkAvgThreshold = BVDC_P_SCL_4TAP_BLK_AVG_VERT_THRESHOLD;
            break;
        case BCHP_SCL_0_HW_CONFIGURATION_VERT_TAPS_VERT_6_TAPS:
            hScaler->ulVertTaps = BVDC_P_CT_6_TAP;
            hScaler->ulVertBlkAvgThreshold = BVDC_P_SCL_6TAP_BLK_AVG_VERT_THRESHOLD;
            break;
        case BCHP_SCL_0_HW_CONFIGURATION_VERT_TAPS_VERT_8_TAPS:
            hScaler->ulVertTaps = BVDC_P_CT_8_TAP;
            hScaler->ulVertBlkAvgThreshold = BVDC_P_SCL_8TAP_BLK_AVG_VERT_THRESHOLD;
            break;
        default:
            BDBG_ERR(("SCL[%d] Unknown vertical tap configuration detected: %d", hScaler->eId, ulTaps));
            BDBG_ASSERT(0);
            break;
    }

    /* Scaler intial states */
    /* Scaler with SD line buffer */
    ulVertLineDepth = BCHP_GET_FIELD_DATA(ulReg, SCL_0_HW_CONFIGURATION, LINE_STORE_DEPTH);
#if (BVDC_P_SUPPORT_SCL_VER >= BVDC_P_SUPPORT_SCL_VER_10)
    switch(ulVertLineDepth)
    {
        case 0: hScaler->ulVertLineDepth = 1920; break;
        case 1: hScaler->ulVertLineDepth =  720; break;
        case 2: hScaler->ulVertLineDepth = 4096; break;
        default:
        BDBG_ERR(("SCL[%d] Unknown vertical line depth detected: %d", hScaler->eId, ulVertLineDepth));
        BDBG_ASSERT(0);
        break;
    }
#else
    hScaler->ulVertLineDepth = ulVertLineDepth;
#endif

    /* Scaler with 10 bit core */
#if (BCHP_SCL_0_HW_CONFIGURATION_MODE_10BIT_MASK)
    hScaler->bIs10BitCore = BCHP_GET_FIELD_DATA(ulReg, SCL_0_HW_CONFIGURATION, MODE_10BIT);
#endif
#if (BCHP_SCL_0_HW_CONFIGURATION_BVB2X_CLK_MASK)
    hScaler->bIs2xClk = BCHP_GET_FIELD_DATA(ulReg, SCL_0_HW_CONFIGURATION, BVB2X_CLK);
#endif
    /* default settings for up sampler and down sampler */
    hScaler->stUpSampler.bUnbiasedRound = false;
    hScaler->stUpSampler.eFilterType    = BVDC_422To444Filter_eTenTaps;
    hScaler->stUpSampler.eRingRemoval   = BVDC_RingSuppressionMode_eNormal;

    hScaler->stDnSampler.eFilterType    = BVDC_444To422Filter_eDecimate;
    hScaler->stDnSampler.eRingRemoval   = BVDC_RingSuppressionMode_eNormal;

    hScaler->ulSrcHrzAlign  = 2;

    BDBG_LEAVE(BVDC_P_Scaler_Init);
    return;
}


/***************************************************************************
 * {private}
 *
 */
void BVDC_P_Scaler_Init_isr
    ( BVDC_P_Scaler_Handle             hScaler,
      BVDC_Window_Handle               hWindow )
{
    BDBG_ENTER(BVDC_P_Scaler_Init_isr);
    BDBG_OBJECT_ASSERT(hScaler, BVDC_SCL);

    hScaler->hWindow = hWindow;

    hWindow->bIs10BitCore = hScaler->bIs10BitCore;
    hWindow->bIs2xClk = hScaler->bIs2xClk;

    BDBG_LEAVE(BVDC_P_Scaler_Init_isr);
    return;
}

/***************************************************************************
 * {private}
 *
 * BVDC_P_Scaler_BuildRul_DrainVnet_isr
 *
 * called by BVDC_P_Scaler_BuildRul_isr after resetting to drain the module and
 * its pre-patch FreeCh or LpBack
 */
static void BVDC_P_Scaler_BuildRul_DrainVnet_isr
    ( BVDC_P_Scaler_Handle           hScaler,
      BVDC_P_ListInfo               *pList,
      bool                           bNoCoreReset)
{
    BDBG_OBJECT_ASSERT(hScaler, BVDC_SCL);
#if (BVDC_P_SUPPORT_SCL_VER < BVDC_P_SUPPORT_SCL_VER_7)
    /* drain */
    BVDC_P_SubRul_Drain_isr(&(hScaler->SubRul), pList,
    hScaler->ulResetRegAddr, hScaler->ulResetMask,
    hScaler->ulVnetResetAddr, hScaler->ulVnetResetMask);
    BSTD_UNUSED(bNoCoreReset);
#else
    /* drain */
    BVDC_P_SubRul_Drain_isr(&(hScaler->SubRul), pList,
    bNoCoreReset?0:hScaler->ulResetRegAddr,
    bNoCoreReset?0:hScaler->ulResetMask,
    bNoCoreReset?0:hScaler->ulVnetResetAddr,
    bNoCoreReset?0:hScaler->ulVnetResetMask);
#endif
}

/***************************************************************************
 * {private}
 *
 */
void BVDC_P_Scaler_BuildRul_isr
    ( const BVDC_P_Scaler_Handle       hScaler,
      BVDC_P_ListInfo                 *pList,
      BVDC_P_State                     eVnetState,
      BVDC_P_PicComRulInfo            *pPicComRulInfo )
{
    uint32_t  ulRulOpsFlags;

    BDBG_ENTER(BVDC_P_Scaler_BuildRul_isr);
    BDBG_OBJECT_ASSERT(hScaler, BVDC_SCL);
    BDBG_OBJECT_ASSERT(hScaler->hWindow, BVDC_WIN);

    /* currently this is only for vnet building / tearing-off*/

    ulRulOpsFlags = BVDC_P_SubRul_GetOps_isr(
        &(hScaler->SubRul), pPicComRulInfo->eWin, eVnetState, pList->bLastExecuted);

    if ((0 == ulRulOpsFlags) ||
        (ulRulOpsFlags & BVDC_P_RulOp_eReleaseHandle))
        return;
    else if (ulRulOpsFlags & BVDC_P_RulOp_eDisable)
    {
        BVDC_P_SubRul_DropOffVnet_isr(&(hScaler->SubRul), pList);
        BVDC_P_Scaler_SetEnable_isr(hScaler, false);
        BVDC_P_SCL_WRITE_TO_RUL(SCL_0_ENABLE, pList->pulCurrent);
    }

    /* If rul failed to execute last time we'd re reprogrammed possible
     * missing registers. */
    if((!pList->bLastExecuted)|| (hScaler->bInitial))
    {
        hScaler->ulUpdateAll = BVDC_P_RUL_UPDATE_THRESHOLD;
    }

#ifdef BCHP_RDC_EOP_ID_256_eop_id_scl_0
    /* NRT mode source cropping at SCL should wait for EOP */
    if(hScaler->hWindow->hCompositor->hDisplay->stCurInfo.bStgNonRealTime) {
        *pList->pulCurrent++ = BRDC_OP_WAIT_EOP(BCHP_RDC_EOP_ID_256_eop_id_scl_0 + hScaler->eId);
        /*BDBG_MSG(("SCL%u wait for eop!", hScaler->eId));*/
    }
#endif

    /* reset */
    if(hScaler->bInitial)
    {
        hScaler->bInitial = false;
    }

    if (ulRulOpsFlags & BVDC_P_RulOp_eEnable)
    {
        /* Add scaler registers to RUL using block write */
        if(hScaler->ulUpdateAll)
        {
            hScaler->ulUpdateAll--;
            hScaler->ulUpdateCoeff = 0; /* no need to update coeff alone */

            /* optimize scaler mute RUL */
            BVDC_P_SCL_BLOCK_WRITE_TO_RUL(SCL_0_VERT_FIR_LUMA_COEFF_PHASE0_00_01,  SCL_0_VERT_FIR_CHROMA_COEFF_PHASE7_00_01, pList->pulCurrent);
            BVDC_P_SCL_BLOCK_WRITE_TO_RUL(SCL_0_HORIZ_FIR_LUMA_COEFF_PHASE0_00_01, SCL_0_HORIZ_FIR_CHROMA_COEFF_PHASE7_04_05, pList->pulCurrent);

            if(hScaler->bDeRinging)
            {
                BVDC_P_SCL_BLOCK_WRITE_TO_RUL(SCL_0_DERINGING, SCL_0_DERING_DEMO_SETTING,
                    pList->pulCurrent);
            }

            if(hScaler->bDeJagging)
            {
                BVDC_P_SCL_BLOCK_WRITE_TO_RUL(SCL_0_DEJAGGING, SCL_0_DEJAGGING_DEMO_SETTING,
                    pList->pulCurrent);
            }

#if (BVDC_P_SUPPORT_SCL_VER < BVDC_P_SUPPORT_SCL_VER_12)
            BVDC_P_SCL_BLOCK_WRITE_TO_RUL(SCL_0_BVB_IN_SIZE, SCL_0_HORIZ_DEST_PIC_REGION_3_END, pList->pulCurrent);
            BVDC_P_SCL_WRITE_TO_RUL(SCL_0_VIDEO_3D_MODE, pList->pulCurrent);
            BVDC_P_SCL_BLOCK_WRITE_TO_RUL(SCL_0_TOP_CONTROL, SCL_0_HORIZ_CONTROL, pList->pulCurrent);
#else
            BVDC_P_SCL_BLOCK_WRITE_TO_RUL(SCL_0_VIDEO_3D_MODE, SCL_0_HORIZ_FIR_COEFF_00, pList->pulCurrent);
            BVDC_P_SCL_BLOCK_WRITE_TO_RUL(SCL_0_TOP_CONTROL, SCL_0_VERT_FIR_INIT_PIC_STEP, pList->pulCurrent);
#endif
        }
        else
        {
            /* update coeff alone, likely due a tmp field inversion */
            if (hScaler->ulUpdateCoeff)
            {
                hScaler->ulUpdateCoeff--;
                BVDC_P_SCL_BLOCK_WRITE_TO_RUL(SCL_0_VERT_FIR_LUMA_COEFF_PHASE0_00_01, SCL_0_VERT_FIR_CHROMA_COEFF_PHASE7_00_01, pList->pulCurrent);
            }

            /* Update these register on every vsync. */
            BVDC_P_SCL_BLOCK_WRITE_TO_RUL(SCL_0_VERT_FIR_SRC_PIC_OFFSET_INT, SCL_0_VERT_FIR_INIT_PIC_STEP, pList->pulCurrent);
            BVDC_P_SCL_WRITE_TO_RUL(SCL_0_TOP_CONTROL, pList->pulCurrent);
        }

        BVDC_P_SCL_WRITE_TO_RUL(SCL_0_ENABLE, pList->pulCurrent);

        /* join in vnet after enable. note: its src mux is initialed as disabled */
        if (ulRulOpsFlags & BVDC_P_RulOp_eVnetInit)
        {
            BVDC_P_SubRul_JoinInVnet_isr(&(hScaler->SubRul), pList);
        }
    }

    else if (ulRulOpsFlags & BVDC_P_RulOp_eDrainVnet)
    {
        BVDC_P_Scaler_BuildRul_DrainVnet_isr(hScaler, pList, pPicComRulInfo->bNoCoreReset);
    }

    BDBG_LEAVE(BVDC_P_Scaler_BuildRul_isr);
    return;
}


/***************************************************************************
 * {private}
 *
 */
static void BVDC_P_Scaler_SetHorizFirCoeff_isr
    ( BVDC_P_Scaler_Handle             hScaler,
      const uint32_t                  *pulHorzFirCoeff )
{
    int i;
    int j = 0;

    BDBG_OBJECT_ASSERT(hScaler, BVDC_SCL);
    /* write 32 hor entries into registers */
    for(i = 0; (pulHorzFirCoeff) && (*pulHorzFirCoeff != BVDC_P_SCL_LAST); i++)
    {
        hScaler->aulRegs[BVDC_P_SCL_GET_REG_IDX(SCL_0_HORIZ_FIR_LUMA_COEFF_PHASE0_00_01) + i] =
            *pulHorzFirCoeff;
        pulHorzFirCoeff++;

        j++;
        if (j == 2 && hScaler->ulHorzTaps == BVDC_P_CT_8_TAP)
        {
            i++;   /* padding 0x00000000 to table */
            j = 0;
        }
    }
}


/***************************************************************************
 * {private}
 *
 */
static void BVDC_P_Scaler_SetChromaHorizFirCoeff_isr
    ( BVDC_P_Scaler_Handle             hScaler,
      const uint32_t                  *pulHorzFirCoeff )
{
    int i;
    int j = 0;

    BDBG_OBJECT_ASSERT(hScaler, BVDC_SCL);
    /* write 32 hor entries into registers */
    for(i = 0; (pulHorzFirCoeff) && (*pulHorzFirCoeff != BVDC_P_SCL_LAST); i++)
    {
        hScaler->aulRegs[BVDC_P_SCL_GET_REG_IDX(SCL_0_HORIZ_FIR_CHROMA_COEFF_PHASE0_00_01) + i] =
            *pulHorzFirCoeff;
        pulHorzFirCoeff++;

        j++;
        if (j == 2 && hScaler->ulHorzTaps == BVDC_P_CT_8_TAP)
        {
            i++;   /* padding 0x00000000 to table */
            j = 0;
        }
    }
}

/***************************************************************************
 * {private}
 *
 */
static void BVDC_P_Scaler_SetVertFirCoeff_isr
    ( BVDC_P_Scaler_Handle             hScaler,
      const uint32_t                  *pulVertFirCoeff )
{
    int i;

    BDBG_OBJECT_ASSERT(hScaler, BVDC_SCL);
    /* write ver entries into registers */
    for(i = 0; (pulVertFirCoeff) && (*pulVertFirCoeff != BVDC_P_SCL_LAST); i++)
    {
        hScaler->aulRegs[BVDC_P_SCL_GET_REG_IDX(SCL_0_VERT_FIR_LUMA_COEFF_PHASE0_00_01) + i] =
            *pulVertFirCoeff;
        pulVertFirCoeff++;
    }
}


/***************************************************************************
 * {private}
 *
 */
static void BVDC_P_Scaler_SetChromaVertFirCoeff_isr
    ( BVDC_P_Scaler_Handle             hScaler,
      const uint32_t                  *pulVertFirCoeff )
{
    int i;

    BDBG_OBJECT_ASSERT(hScaler, BVDC_SCL);
    /* write ver entries into registers */
    for(i = 0; (pulVertFirCoeff) && (*pulVertFirCoeff != BVDC_P_SCL_LAST); i++)
    {
        hScaler->aulRegs[BVDC_P_SCL_GET_REG_IDX(SCL_0_VERT_FIR_CHROMA_COEFF_PHASE0_00_01) + i] =
            *pulVertFirCoeff;
        pulVertFirCoeff++;
    }
}


/***************************************************************************
 * {private}
 * The effect of (bPrgrssvIntrlcChange==true) is that point sampling coeff
 * will not be choosen.
 */
const BVDC_P_FirCoeffTbl* BVDC_P_SelectFirCoeff_isr
    ( const BVDC_P_FirCoeffTbl         *pFirstCoeffEntry,
      uint32_t                          ulCtIndex,
      BVDC_P_CtInput                    eCtInputType,
      BVDC_P_CtOutput                   eCtOutputType,
      BAVC_Polarity                     eSrcOrigPolarity,
      uint32_t                          ulDownRatio,
      uint32_t                          ulTapMode,
      uint32_t                          ulSrcSize,
      uint32_t                          ulDstSize )
{
    const BVDC_P_FirCoeffTbl *pCoeffs = NULL; /* Return this */
    const BVDC_P_FirCoeffTbl *pCurTable = pFirstCoeffEntry;
    BVDC_P_CtRaster eCtRaster = (BAVC_Polarity_eFrame == eSrcOrigPolarity)
        ? BVDC_P_CtRaster_ePro : BVDC_P_CtRaster_eInt;

    /* Iterate thru to find first matching table */
    while((BVDC_P_CtLutOp_eLast != pCurTable->eCtLutOp) && (!pCoeffs))
    {
        if((pCurTable->eCtLutOp == BVDC_P_CtLutOp_eAlways) &&
           (pCurTable->ulTapMode == ulTapMode))
        {
            BDBG_WRN(("No matching rule for coeffs selecting default!"));
            pCoeffs = pCurTable;
        }
        else if((pCurTable->eCtLutOp == BVDC_P_CtLutOp_eUserSelectable) &&
                (pCurTable->ulTapMode == ulTapMode) &&
                (pCurTable->ulCtIndex == ulCtIndex))
        {
            pCoeffs = pCurTable;
        }
        else if ((pCurTable->ulCtIndex == ulCtIndex) &&
                 (pCurTable->ulTapMode == ulTapMode) &&
                 ((pCurTable->eCtInputType == eCtInputType) ||             /* Matched input type or any. */
                  (pCurTable->eCtInputType == BVDC_P_CtInput_eAny)) &&
                 ((pCurTable->eCtOutputType == eCtOutputType) ||             /* Matched output type or any. */
                  (pCurTable->eCtOutputType == BVDC_P_CtOutput_eAny)) &&
                 ((pCurTable->eCtRaster == eCtRaster) ||                   /* Matched raster type or any. */
                  (pCurTable->eCtRaster == BVDC_P_CtRaster_eAny)))
        {
            switch(pCurTable->eCtLutOp)
            {
            case BVDC_P_CtLutOp_eGreaterThan:
                if(ulDownRatio > pCurTable->ulDownRatio)
                {
                    pCoeffs = pCurTable;
                }
                break;

            case BVDC_P_CtLutOp_eGreaterThanEqual:
                if(ulDownRatio >= pCurTable->ulDownRatio)
                {
                    pCoeffs = pCurTable;
                }
                break;

            case BVDC_P_CtLutOp_eLessThan:
                if(ulDownRatio < pCurTable->ulDownRatio)
                {
                    pCoeffs = pCurTable;
                }
                break;

            case BVDC_P_CtLutOp_eEqual:
                /* If using size match, not ratio */
                if(pCurTable->ulDownRatio == BVDC_P_CT_UNUSED)
                {
                    if((ulSrcSize <= pCurTable->ulSrcSize + pCurTable->ulDelta) &&
                        (ulSrcSize >= pCurTable->ulSrcSize - pCurTable->ulDelta) &&
                        (ulDstSize <= pCurTable->ulDstSize + pCurTable->ulDelta) &&
                        (ulDstSize >= pCurTable->ulDstSize - pCurTable->ulDelta))
                    {
                        pCoeffs = pCurTable;
                    }
                }
                else
                {
                    if(ulDownRatio == pCurTable->ulDownRatio)
                    {
                        pCoeffs = pCurTable;
                    }
                }
                break;

            case BVDC_P_CtLutOp_eLessThanEqual:
                if(ulDownRatio <= pCurTable->ulDownRatio)
                {
                    pCoeffs = pCurTable;
                }
                break;

            default:
                pCoeffs = NULL;
                break;
            }
        }
        ++pCurTable;
    }

    BDBG_ASSERT(pCoeffs);

    return pCoeffs;
}


/***************************************************************************
 * {private}
 *
 * The following notes illustrate the capabilities of the scaler.  It
 * mainly shows different modes scaler operates to achieve the desire
 * scaling ratio with the best quality.
 *
 * Sx = in/out.  For example src=1920 dst=720 then Sx = 1920/720 or 2.66.
 * Hence this is what Sx means:
 *   Sx >  1 Scale down
 *   Sx <  1 Scale up
 *   Sx == 1 Unity.
 *
 * Actuall Sx tells how much we're scaling down.  For example
 * Sx = 4 means we're scaling down 4 times.
 *
 * [[ Horizontal Scaling ]]
 *
 * {BVB_in} --> [ HWF_0 ] --> [ HWF_1 ] --> [ FIR ] --> {BVB_out}
 *
 *  in:out        HWF_0         HWF_1         FIR
 *
 * Sx <= 4         1:1           1:1          Sx' = Sx / (1 * 1)
 * Sx >  4         2:1           1:1          Sx' = Sx / (2 * 1)
 * Sx >  8         2:1           2:1          Sx' = Sx / (2 * 2)
 *
 * Sx' is the new horizontal ratio, and this is the value that goes into
 * horizontal FIR ratio register.
 *
 * Sx' must be [32.0, 0.125].
 *
 *
 * [[ Vertical Scaling ]]
 *
 * {BVB_in} --> [ BLK_AVG ]      -->        [ FIR ] --> {BVB_out}
 *
 *  in:out        BLK_AVG                     FIR
 *
 * L >= 1280 (2-tap)
 *  Sy <= 2       1:1                         Sy' = Sy
 *  Sy >  2       (n+1):1                     Sy' = Sy / (n+1)
 *
 * L < 1280  (4-tap)
 *  Sy <= 4       1:1                         Sy' = Sy
 *  Sy >  4   (n+1):1                         Sy' = Sy / (n+1)
 *
 * Sy' is the new vertical ratio, and this is the value that goes into
 *  vertical FIR ratio register.
 *
 * Sy' must be [2, 0.013125] (for 2-tap)
 * Sy' must be [4, 0.013125] (for 4-tap)
 *
 * L is input lenght to vertical scaler in pixels.  Notes that if the
 *  horzontal scaler is before vertical scaler L is equal to the output of
 *  horzontal.  (if horizontal is downscale it will be first).
 *
 * n is the BAVG_BLK_SIZE.  n must be [0, 15].
 *
 *
 * [[ Conclusion ]]
 *  With the above information the theoretical scaling capacities are:
 *
 *  Sx = 32:1 to 1:32
 *
 *  Sy = 32:1 to 1:32 (for 2-tap)
 *     = 64:1 to 1:32 (for 4-tap)
 */
static BERR_Code BVDC_P_Scaler_CalVertInitPhase_isr
    ( BVDC_P_Scaler_Handle             hScaler,
      const BAVC_Polarity              eSrcPolarity,
      const BAVC_Polarity              eDstPolarity,
      uint32_t                         ulPanScanTop,
      uint32_t                        *pulLargestBlkAvgSize,
      uint32_t                        *pulVertStep,
      uint32_t                         ulVSclIn,
      uint32_t                         ulVSclOut)
{
    uint32_t ulVertInitPhase; /* FIXED: S6.6 */
    uint32_t ulFirVrtStep;
    uint32_t ulANom      = (eDstPolarity == BAVC_Polarity_eBotField) ? 3 : 1;
    uint32_t ulADenom    = (eDstPolarity == BAVC_Polarity_eFrame) ? 2 : 4;
    uint32_t ulBNom      = (eSrcPolarity == BAVC_Polarity_eBotField) ? 3 : 1;
    uint32_t ulBDenom    = (eSrcPolarity == BAVC_Polarity_eFrame) ? 2 : 4;
    bool     bPhaseShift = ((hScaler->bHandleFldInv) &&
                            (eSrcPolarity != BAVC_Polarity_eFrame) &&
                            (eDstPolarity != BAVC_Polarity_eFrame));
    bool bSclVertPhaseIgnore = hScaler->hWindow->stCurInfo.stSclSettings.bSclVertPhaseIgnore;
    uint32_t ulInitPhase = 0, ulBlkAvgSize = 0;
    uint32_t ulThreshold = 0;

    BDBG_MODULE_MSG(BVDC_SCL_VSTEP, (" Scl[%d] in: %d out: %d blkavg %d ", hScaler->eId, ulVSclIn, ulVSclOut, *pulLargestBlkAvgSize));

    /* Compute Vertical FIR initial phase.
     * Do we want scaler to invert the field?  To invert field we will
     * need to change the vertical initial phase a according to the given
     * formula for doing TF2BF, and BF2TF.
     * NOTE:
     *   - this formula is based on field middle line alignment; */

    ulFirVrtStep = BVDC_P_CAL_BLK_VRT_SRC_STEP(ulVSclIn, ulVSclOut,0);
    ulFirVrtStep = BVDC_P_SCL_V_RATIO_TRUNC(ulFirVrtStep);
    ulFirVrtStep = BVDC_P_V_INIT_PHASE_RATIO_ADJ(ulFirVrtStep);
    ulInitPhase =
        BVDC_P_FIXED_A_MINUS_FIXED_B(ulFirVrtStep * ulANom / ulADenom, BVDC_P_V_INIT_PHASE_1_POINT_0 * ulBNom / ulBDenom) + ulPanScanTop;
    if(bPhaseShift && (0 == (ulFirVrtStep & (BVDC_P_SCL_V_RATIO_ONE - 1))))
        ulInitPhase -= BVDC_P_V_INIT_PHASE_0_POINT_25;
    ulVertInitPhase = bSclVertPhaseIgnore? 0 : ulInitPhase;
    ulThreshold = hScaler->ulVertBlkAvgThreshold / 2 + (1 << BVDC_P_SCL_V_RATIO_F_BITS);

    /* The new equation to avoid SCL hang: VSCL_STEP <= (VSCL_TAP/2) + V_Init_phase + 1 */
    while((ulBlkAvgSize <= BVDC_P_SCL_MAX_BLK_AVG) &&
          (ulFirVrtStep > (ulThreshold + ulVertInitPhase )))
    {
        ulBlkAvgSize++;
        ulFirVrtStep = BVDC_P_CAL_BLK_VRT_SRC_STEP(ulVSclIn, ulVSclOut, ulBlkAvgSize);
        BDBG_MODULE_MSG(BVDC_SCL_VSTEP,("step %x blkavg %d max %x", ulFirVrtStep, hScaler->aulBlkAvgSize[eSrcPolarity][eDstPolarity], BVDC_P_SCL_V_RATIO_MAX));
        ulFirVrtStep = BVDC_P_SCL_V_RATIO_TRUNC(ulFirVrtStep);
        ulFirVrtStep = BVDC_P_V_INIT_PHASE_RATIO_ADJ(ulFirVrtStep);

        ulInitPhase =
            BVDC_P_FIXED_A_MINUS_FIXED_B(ulFirVrtStep* ulANom / ulADenom, BVDC_P_V_INIT_PHASE_1_POINT_0 * ulBNom / ulBDenom) +
            (ulPanScanTop / (ulBlkAvgSize + 1));
        if(bPhaseShift && (0 == (ulFirVrtStep & (BVDC_P_SCL_V_RATIO_ONE - 1))))
            ulInitPhase -= BVDC_P_V_INIT_PHASE_0_POINT_25;
        ulVertInitPhase = bSclVertPhaseIgnore? 0 : ulInitPhase ;

        BDBG_MODULE_MSG(BVDC_SCL_VSTEP,("step %x blkavg %d ", ulFirVrtStep, ulBlkAvgSize));
    }

    hScaler->aulBlkAvgSize[eSrcPolarity][eDstPolarity] = ulBlkAvgSize;

    if(*pulLargestBlkAvgSize < ulBlkAvgSize)
    {
        *pulLargestBlkAvgSize = ulBlkAvgSize;
        *pulVertStep = ulFirVrtStep;
    }

    if(ulBlkAvgSize > BVDC_P_SCL_MAX_BLK_AVG &&
       (ulFirVrtStep > (ulThreshold + ulVertInitPhase )))
    {
        return BERR_UNKNOWN;
    }

    return BERR_SUCCESS;
}


void BVDC_P_Scaler_SetInfo_isr
    ( BVDC_P_Scaler_Handle             hScaler,
      const BVDC_P_PictureNodePtr      pPicture )
{
    uint32_t ulSrcHSize;               /* really scaled src width in pixel unit */
    uint32_t ulSrcVSize;               /* really scaled src height, in row unit */
    uint32_t ulDstHSize;               /* Dst width in pixel unit */
    uint32_t ulDstVSize;               /* Dst height, in row unit */
    uint32_t ulAlgnSrcHSize;           /* src width into the 1st one of half band or FIR, pixel unit */
    uint32_t ulAlgnSrcVSize;           /* src height into the 1st one of block avrg or FIR, row unit */
    uint32_t ulBvbInHSize;             /* input bvb width in pixel unit */
    uint32_t ulBvbInVSize;             /* input bvb height, in row unit */
    uint32_t ulPicOffsetLeft;          /* horizontal Pan/Scan part cut by PIC_OFFSET, pixel unit */
#if (BVDC_P_SUPPORT_SCL_VER >= BVDC_P_SUPPORT_SCL_VER_8)
    uint32_t ulPicOffsetLeft_R;         /* horizontal Pan/Scan part cut by PIC_OFFSET_R, pixel unit */
#endif
    uint32_t ulPicOffsetTop;           /* vertical Pan/Scan part cut by PIC_OFFSET, row unit */
    uint32_t ulPanScanLeft;            /* horizontal Pan/Scan vector in S11.6 format */
    uint32_t ulPanScanTop;             /* Vertical Pan/Scan vector in S11.14 format */
    uint32_t ulFirSrcHSize;            /* FIR Horizontal input size for size matching to select coeff */
    uint32_t ulFirDstHSize;            /* FIR Horizontal output size for size matching to select coeff */
    uint32_t ulFirSrcVSize;            /* FIR Vertical input size for size matching to select coeff */
    uint32_t ulFirDstVSize;            /* FIR Vertical output size for size matching to select coeff */
    const BVDC_P_FirCoeffTbl *pHorzFirCoeff;
    uint32_t ulVertStepRoundoff = 0;
    uint32_t ulNrmHrzStep;              /* Total horizontal src step per dest pixel, U12.20 */
    uint32_t ulNrmVrtSrcStep;
    uint32_t ulHrzStep = 0;             /* Total horizontal src step per dest pixel, HW reg format */
    uint32_t ulVrtStep = 0;             /* Total vertical src step per dest pixel, HW reg format */
    uint32_t ulFirHrzStep = 0;          /* FIR hrz src step per dest pixel, HW reg fmt, for coeff select */
    uint32_t ulFirVrtStep = 0;          /* FIR vertical src step per dest pixel */
    uint32_t ulFirHrzStepInit = 0;      /* FIR hrz src step per dest pixel, HW reg fmt, for init phase */
    uint32_t ulVertSclSrcWidth = 0;     /* Adjusted src width after horz scaler, in pixel unit */
    uint32_t ulVertInitPhase       = 0; /* FIXED: S6.6 */
    uint32_t ulBlkAvgSize = 0;
    int32_t  lHrzPhsAccInit = 0;
    uint32_t ulMaxX, ulMaxY;
    uint32_t ulVsr = 0;
    BVDC_P_CtOutput  eOutputType;
    BVDC_P_Rect  *pSclIn, *pSclOut, *pSclCut;
    bool bPqNcl;

    BDBG_ENTER(BVDC_P_Scaler_SetInfo_isr);
    BDBG_OBJECT_ASSERT(hScaler, BVDC_SCL);
    BDBG_OBJECT_ASSERT(hScaler->hWindow, BVDC_WIN);
    BDBG_OBJECT_ASSERT(hScaler->hWindow->stCurInfo.hSource, BVDC_SRC);
    pSclIn = pPicture->pSclIn;
    pSclCut = &pPicture->stSclCut;
    pSclOut = pPicture->pSclOut;

    bPqNcl = (pPicture->astMosaicColorSpace[pPicture->ulPictureIdx].eColorTF == BCFC_ColorTF_eBt2100Pq &&
              pPicture->astMosaicColorSpace[pPicture->ulPictureIdx].eColorFmt == BCFC_ColorFormat_eYCbCr) ? true : false;

    /* any following info changed -> re-calculate SCL settings */
    /* TODO: This need optimization */
    if(!BVDC_P_RECT_CMP_EQ(pSclOut, &hScaler->stPrevSclOut)   ||
       !BVDC_P_RECT_CMP_EQ(pSclCut, &hScaler->stPrevSclCut)   ||
       !BVDC_P_RECT_CMP_EQ(pSclIn,  &hScaler->stPrevSclIn)    ||
       (hScaler->hWindow->stCurInfo.stSclSettings.bSclVertDejagging       != hScaler->stSclSettings.bSclVertDejagging      ) ||
       (hScaler->hWindow->stCurInfo.stSclSettings.ulSclDejaggingCore      != hScaler->stSclSettings.ulSclDejaggingCore     ) ||
       (hScaler->hWindow->stCurInfo.stSclSettings.ulSclDejaggingGain      != hScaler->stSclSettings.ulSclDejaggingGain     ) ||
       (hScaler->hWindow->stCurInfo.stSclSettings.ulSclDejaggingHorz      != hScaler->stSclSettings.ulSclDejaggingHorz     ) ||
       (hScaler->hWindow->stCurInfo.stCtIndex.ulSclHorzLuma   != hScaler->stCtIndex.ulSclHorzLuma   ) ||
       (hScaler->hWindow->stCurInfo.stCtIndex.ulSclHorzChroma != hScaler->stCtIndex.ulSclHorzChroma ) ||
       (hScaler->hWindow->stCurInfo.stCtIndex.ulSclVertLuma   != hScaler->stCtIndex.ulSclVertLuma   ) ||
       (hScaler->hWindow->stCurInfo.stCtIndex.ulSclVertChroma != hScaler->stCtIndex.ulSclVertChroma ) ||
       (hScaler->hWindow->stCurInfo.stSplitScreenSetting.eDeJagging != hScaler->ePrevDeJagging) ||
       (hScaler->hWindow->stCurInfo.stSplitScreenSetting.eDeRinging != hScaler->ePrevDeRinging) ||
       (hScaler->hWindow->stCurInfo.hSource->stCurInfo.eCtInputType  != hScaler->ePrevCtInputType)   ||
       (pPicture->ulNonlinearSrcWidth    != hScaler->ulPrevNonlinearSrcWidth)    ||
       (pPicture->ulNonlinearSclOutWidth != hScaler->ulPrevNonlinearSclOutWidth) ||
       (pPicture->eSrcOrientation        != hScaler->ePrevSrcOrientation)    ||
       (pPicture->eOrigSrcOrientation    != hScaler->ePrevOrigSrcOrientation)    ||
       (pPicture->eDispOrientation       != hScaler->ePrevDispOrientation)   ||
       (bPqNcl != hScaler->bPqNcl) ||
       ((pPicture->eSrcPolarity == BAVC_Polarity_eFrame) !=
        (hScaler->ePrevSrcPolarity == BAVC_Polarity_eFrame)) ||
       ((pPicture->eDstPolarity == BAVC_Polarity_eFrame) !=
        (hScaler->ePrevDstPolarity == BAVC_Polarity_eFrame)) ||
       !BVDC_P_SCL_COMPARE_FIELD_DATA(SCL_0_ENABLE, SCALER_ENABLE, 1) ||
       (pPicture->stFlags.bHandleFldInv != hScaler->bHandleFldInv)||
       (pPicture->bMosaicIntra))
    {
        uint32_t ulFirVrtStepWithKellFactor;

        /* for next "dirty" check */
        hScaler->stPrevSclIn  = *pSclIn;
        hScaler->stPrevSclOut = *pSclOut;
        hScaler->stPrevSclCut = *pSclCut;
        hScaler->stCtIndex    = hScaler->hWindow->stCurInfo.stCtIndex;
        hScaler->stSclSettings = hScaler->hWindow->stCurInfo.stSclSettings;
        hScaler->ulPrevNonlinearSrcWidth = pPicture->ulNonlinearSrcWidth;
        hScaler->ulPrevNonlinearSclOutWidth = pPicture->ulNonlinearSclOutWidth;
        hScaler->ePrevSrcPolarity = pPicture->eSrcPolarity;
        hScaler->ePrevDstPolarity = pPicture->eDstPolarity;
        hScaler->ePrevSrcOrientation  = pPicture->eSrcOrientation;
        hScaler->ePrevOrigSrcOrientation = pPicture->eOrigSrcOrientation;
        hScaler->ePrevDispOrientation = pPicture->eDispOrientation;
        hScaler->ePrevDeJagging = hScaler->hWindow->stCurInfo.stSplitScreenSetting.eDeJagging;
        hScaler->ePrevDeRinging = hScaler->hWindow->stCurInfo.stSplitScreenSetting.eDeRinging;
        hScaler->ePrevCtInputType = hScaler->hWindow->stCurInfo.hSource->stCurInfo.eCtInputType;
        hScaler->ulUpdateAll = BVDC_P_RUL_UPDATE_THRESHOLD;
        hScaler->bHandleFldInv = pPicture->stFlags.bHandleFldInv;
        hScaler->bPqNcl = bPqNcl;

        /* -----------------------------------------------------------------------
         * 1). Init some regitsers first, they might be modified later basing on
         * specific conditions
         */

        /* scaler panscan will be combined with init phase */
        BVDC_P_SCL_GET_REG_DATA(SCL_0_SRC_PIC_HORIZ_PAN_SCAN) &= ~(
            BCHP_MASK(SCL_0_SRC_PIC_HORIZ_PAN_SCAN, OFFSET));
        BVDC_P_SCL_GET_REG_DATA(SCL_0_SRC_PIC_VERT_PAN_SCAN) &= ~(
            BCHP_MASK(SCL_0_SRC_PIC_VERT_PAN_SCAN, OFFSET));

        /* Horizontal scaler settings (and top control)!  Choose scaling order,
         * and how much to decimate data. */
        BVDC_P_SCL_GET_REG_DATA(SCL_0_TOP_CONTROL) &= ~(
            BCHP_MASK(SCL_0_TOP_CONTROL, FILTER_ORDER ));

        BVDC_P_SCL_GET_REG_DATA(SCL_0_TOP_CONTROL) |=  (
#if (BVDC_P_SUPPORT_SCL_VER < BVDC_P_SUPPORT_SCL_VER_10)
            BCHP_FIELD_ENUM(SCL_0_TOP_CONTROL, UPDATE_SEL, UPDATE_BY_PICTURE) |
#endif
            /* disable it for now due to robustness issue for 3548 Ax */
            BCHP_FIELD_ENUM(SCL_0_TOP_CONTROL, ENABLE_CTRL,  ENABLE_BY_PICTURE) |
            BCHP_FIELD_ENUM(SCL_0_TOP_CONTROL, FILTER_ORDER, VERT_FIRST));

#if (BVDC_P_SUPPORT_SCL_VER > BVDC_P_SUPPORT_SCL_VER_1)
        BVDC_P_SCL_GET_REG_DATA(SCL_0_ENABLE) &= ~(
            BCHP_MASK(SCL_0_ENABLE, SCALER_ENABLE));
        BVDC_P_SCL_GET_REG_DATA(SCL_0_ENABLE) |=
            BCHP_FIELD_ENUM(SCL_0_ENABLE, SCALER_ENABLE, ON);
#endif

        /* HW half band filters are initialized as OFF */
        BVDC_P_SCL_GET_REG_DATA(SCL_0_HORIZ_CONTROL) &= ~(
            BCHP_MASK(SCL_0_HORIZ_CONTROL, HWF0_ENABLE) |
            BCHP_MASK(SCL_0_HORIZ_CONTROL, HWF1_ENABLE) |
            BCHP_MASK(SCL_0_HORIZ_CONTROL, FIR_ENABLE)  |
            BCHP_MASK(SCL_0_HORIZ_CONTROL, MASK_HSCL_LONG_LINE)  |
            BCHP_MASK(SCL_0_HORIZ_CONTROL, MASK_HSCL_SHORT_LINE) |
            BCHP_MASK(SCL_0_HORIZ_CONTROL, STALL_DRAIN_ENABLE));
        BVDC_P_SCL_GET_REG_DATA(SCL_0_HORIZ_CONTROL) |=  (
            BCHP_FIELD_ENUM(SCL_0_HORIZ_CONTROL, HWF0_ENABLE, OFF) |
            BCHP_FIELD_ENUM(SCL_0_HORIZ_CONTROL, HWF1_ENABLE, OFF) |
            BCHP_FIELD_ENUM(SCL_0_HORIZ_CONTROL, FIR_ENABLE,  ON ) |
            BCHP_FIELD_ENUM(SCL_0_HORIZ_CONTROL, MASK_HSCL_LONG_LINE,  ON) |
            BCHP_FIELD_ENUM(SCL_0_HORIZ_CONTROL, MASK_HSCL_SHORT_LINE, ON) |
            BCHP_FIELD_ENUM(SCL_0_HORIZ_CONTROL, STALL_DRAIN_ENABLE, OFF));

        /* -----------------------------------------------------------------------
         * 2). Need to calculate the horizontal scaling factors before src width
         * alignment and init phase can be decided
         */

        /* output size */
        ulDstHSize = pSclOut->ulWidth;
        ulDstVSize = (BAVC_Polarity_eFrame==pPicture->eDstPolarity
#if BFMT_LEGACY_3DTV_SUPPORT
            && (!BFMT_IS_CUSTOM_1080P3D(pPicture->hBuffer->hWindow->hCompositor->stCurInfo.pFmtInfo->eVideoFmt))
#endif
            )? pSclOut->ulHeight : pSclOut->ulHeight / BVDC_P_FIELD_PER_FRAME;

        BVDC_P_SCL_GET_REG_DATA(SCL_0_DEST_PIC_SIZE) &= ~(
            BCHP_MASK(SCL_0_DEST_PIC_SIZE, HSIZE) |
            BCHP_MASK(SCL_0_DEST_PIC_SIZE, VSIZE));
        BVDC_P_SCL_GET_REG_DATA(SCL_0_DEST_PIC_SIZE) |=  (
            BCHP_FIELD_DATA(SCL_0_DEST_PIC_SIZE, HSIZE, ulDstHSize) |
            BCHP_FIELD_DATA(SCL_0_DEST_PIC_SIZE, VSIZE, ulDstVSize));

        /* the src size really scaled and output */
        ulSrcHSize = pSclCut->ulWidth;
        ulSrcVSize = (BAVC_Polarity_eFrame == pPicture->eSrcPolarity)?
            pSclCut->ulHeight :
            pSclCut->ulHeight / BVDC_P_FIELD_PER_FRAME;

        /* pan scan:  left format S11.6, top format S11.14 */
        ulPanScanTop  = (BAVC_Polarity_eFrame == pPicture->eSrcPolarity)?
            pSclCut->lTop :
            pSclCut->lTop / BVDC_P_FIELD_PER_FRAME;
        ulPanScanLeft = pSclCut->lLeft;

        /* separate the amount cut by SCL_0_PIC_OFFSET and FIR_LUMA_SRC_PIC_OFFSET */
        ulPicOffsetLeft = (ulPanScanLeft >> BVDC_P_SCL_LEFT_PIC_OFFSET_F_BITS) & ~(hScaler->ulSrcHrzAlign - 1);
        ulPicOffsetTop  = (ulPanScanTop  >> BVDC_P_SCL_TOP_PIC_OFFSET_F_BITS);
#if (BVDC_P_SUPPORT_SCL_VER >= BVDC_P_SUPPORT_SCL_VER_8)
        ulPicOffsetLeft_R = (pSclCut->lLeft_R >> BVDC_P_SCL_LEFT_PIC_OFFSET_F_BITS) & ~(hScaler->ulSrcHrzAlign - 1);
#endif

        ulPanScanLeft -= (ulPicOffsetLeft << BVDC_P_SCL_LEFT_PIC_OFFSET_F_BITS);
        ulPanScanTop  -= (ulPicOffsetTop  << BVDC_P_SCL_TOP_PIC_OFFSET_F_BITS);

        /* the src size that get into the first scaler sub-modules (e.g. HW half-band
         * filter if it is scaled down a lot): it includes the FIR_LUMA_SRC_PIC_OFFSET,
         * but not the SCL_0_PIC_OFFSET, it has to be rounded-up for alignment */
        ulMaxX = ulPanScanLeft + (ulSrcHSize << BVDC_P_SCL_LEFT_PIC_OFFSET_F_BITS);
        ulMaxY = ulPanScanTop  + (ulSrcVSize << BVDC_P_SCL_TOP_PIC_OFFSET_F_BITS);
        ulAlgnSrcHSize = ((ulMaxX + ((1<< BVDC_P_SCL_LEFT_PIC_OFFSET_F_BITS) - 1)) >>  BVDC_P_SCL_LEFT_PIC_OFFSET_F_BITS);
        ulAlgnSrcVSize = ((ulMaxY + ((1<<BVDC_P_SCL_TOP_PIC_OFFSET_F_BITS) - 1)) >> BVDC_P_SCL_TOP_PIC_OFFSET_F_BITS);
        ulAlgnSrcHSize = BVDC_P_ALIGN_DN(ulAlgnSrcHSize, hScaler->ulSrcHrzAlign);

        /* Init the input/output horizontal/vertical size of FIRs */
        ulFirSrcHSize = ulSrcHSize;
        ulFirDstHSize = ulDstHSize;
        ulFirSrcVSize = ulSrcVSize;
        ulFirDstVSize = ulDstVSize;

        /* this is the src width into vertical scaler */
        ulVertSclSrcWidth = ulSrcHSize;

        /* for linear scaling mode, horizontal scaling may turn on HW filters */
        if(0 == pPicture->ulNonlinearSclOutWidth)
        {
            bool bStgTrigger = BVDC_P_DISPLAY_USED_STG(hScaler->hWindow->hCompositor->hDisplay->eMasterTg);
            /* Horizantal step HW reg uses U5.17 in older arch, U5.26 after smooth non-linear is
             * suported. Since CPU uses 32 bits int, calc step with 26 bits frac needs special
             * handling (see the delta calcu in the case of nonlinear scaling). It is the step
             * delta and internal step accum reg, not the initial step value, that really need 26
             * frac bits, therefore we use 20 bits for trade off */
            ulNrmHrzStep = pPicture->ulNrmHrzSrcStep;    /* U12.20 */
            ulFirHrzStep = ulHrzStep = BVDC_P_SCL_H_STEP_NRM_TO_SPEC(ulNrmHrzStep); /* U4.17, U5.17, U5.26 */

            /* Use Hard-Wired-Filters to assist FIR extreme scale down */
            if(( (ulHrzStep  > BVDC_P_SCL_1ST_DECIMATION_THRESHOLD) &&(!bStgTrigger)) ||
                ((ulHrzStep >= BVDC_P_SCL_1ST_DECIMATION_THRESHOLD) &&( bStgTrigger)))
            {
                BVDC_P_SCL_GET_REG_DATA(SCL_0_HORIZ_CONTROL) &= ~(
                    BCHP_MASK(SCL_0_HORIZ_CONTROL, HWF0_ENABLE));
                BVDC_P_SCL_GET_REG_DATA(SCL_0_HORIZ_CONTROL) |=  (
                    BCHP_FIELD_ENUM(SCL_0_HORIZ_CONTROL, HWF0_ENABLE, ON));
                ulFirHrzStep /= BVDC_P_SCL_HORZ_HWF_FACTOR;
                ulFirSrcHSize /= BVDC_P_SCL_HORZ_HWF_FACTOR;
                ulPanScanLeft /= BVDC_P_SCL_HORZ_HWF_FACTOR;
                if (1 != hScaler->ulSrcHrzAlign)
                    ulAlgnSrcHSize = BVDC_P_ALIGN_DN(ulAlgnSrcHSize, 2 * hScaler->ulSrcHrzAlign);

                if(ulHrzStep > BVDC_P_SCL_2ND_DECIMATION_THRESHOLD)
                {
                    BVDC_P_SCL_GET_REG_DATA(SCL_0_HORIZ_CONTROL) &= ~(
                        BCHP_MASK(SCL_0_HORIZ_CONTROL, HWF1_ENABLE));
                    BVDC_P_SCL_GET_REG_DATA(SCL_0_HORIZ_CONTROL) |=  (
                        BCHP_FIELD_ENUM(SCL_0_HORIZ_CONTROL, HWF1_ENABLE, ON));
                    ulFirHrzStep /= BVDC_P_SCL_HORZ_HWF_FACTOR;
                    ulFirSrcHSize /= BVDC_P_SCL_HORZ_HWF_FACTOR;
                    ulPanScanLeft /= BVDC_P_SCL_HORZ_HWF_FACTOR;
                    if (1 != hScaler->ulSrcHrzAlign)
                        ulAlgnSrcHSize = BVDC_P_ALIGN_DN(ulAlgnSrcHSize, 4 * hScaler->ulSrcHrzAlign);
                }
            }
            else if (((1<<BVDC_P_NRM_SRC_STEP_F_BITS) == ulNrmHrzStep) &&
                     (0 == ulPanScanLeft) &&
                     (ulSrcHSize == ulDstHSize) &&
                     (!hScaler->stCtIndex.ulSclHorzLuma) &&
                     (!hScaler->stCtIndex.ulSclHorzChroma))
            {
                /* unity scale and no phase shift, turn off FIR for accuracy */
                BVDC_P_SCL_GET_REG_DATA(SCL_0_HORIZ_CONTROL) &= ~(
                    BCHP_MASK(SCL_0_HORIZ_CONTROL, FIR_ENABLE));
                BVDC_P_SCL_GET_REG_DATA(SCL_0_HORIZ_CONTROL) |=  (
                    BCHP_FIELD_ENUM(SCL_0_HORIZ_CONTROL, FIR_ENABLE,  OFF));
                BDBG_MODULE_MSG(BVDC_FIR_BYPASS, ("scl[%d] HFIR_ENABLE OFF", hScaler->eId));
            }

            ulFirHrzStepInit = ulFirHrzStep;

            /* set step size and region_0 end */
            BVDC_P_SCL_SET_HORZ_RATIO(ulFirHrzStep);
            BVDC_P_SCL_SET_HORZ_REGION02(0, ulDstHSize, 0)
        }
        else
        {
            /* 5 Zone nonlinear function, backwards compatible with 3-Zone nonlinear*/
            BVDC_P_Scaler_5ZoneNonLinear_isr(
                hScaler, pPicture, &ulFirHrzStep, &ulHrzStep,
                &ulFirHrzStepInit);
        }

        /* -----------------------------------------------------------------------
         * 3). Now we can set src size, offset and bvb size
         */
        ulBvbInHSize = pSclIn->ulWidth;
        ulBvbInVSize = (BAVC_Polarity_eFrame==pPicture->eSrcPolarity)?
            (pSclIn->ulHeight) :
            (pSclIn->ulHeight) / BVDC_P_FIELD_PER_FRAME;

        /* in older chips, align ulBvbInHSize up if ulAlgnSrcHSize has been aligned
         * up due to half-band.
         * note: align ulBvbInHSize up might cause "short line" error, that is OK
         * and scl input module would patch. If we don't align up, SCL might hang */
        if (1 != hScaler->ulSrcHrzAlign)
            ulBvbInHSize  = BVDC_P_MAX(ulBvbInHSize, ulAlgnSrcHSize + ulPicOffsetLeft);
        else
            ulAlgnSrcHSize = BVDC_P_MIN(ulAlgnSrcHSize, ulBvbInHSize - ulPicOffsetLeft);

        /* without this we might see flash when we move up with dst cut if MAD is disabled? */
        /*ulBvbInVSize  = BVDC_P_MAX(ulBvbInVSize, ulAlgnSrcVSize + ulPicOffsetTop);*/

        /* Make sure scaler BVN in V size, picture offset and src picture size are inline.
         * Otherwise we may run into short field error --- a long delay to patch and to
         * cause.enable_error with next enabling.
         */
        ulAlgnSrcVSize = BVDC_P_MIN(ulAlgnSrcVSize, ulBvbInVSize - ulPicOffsetTop);

        BVDC_P_SCL_GET_REG_DATA(SCL_0_PIC_OFFSET) &= ~(
            BCHP_MASK(SCL_0_PIC_OFFSET, HSIZE) |
            BCHP_MASK(SCL_0_PIC_OFFSET, VSIZE));
        BVDC_P_SCL_GET_REG_DATA(SCL_0_PIC_OFFSET) |=  (
            BCHP_FIELD_DATA(SCL_0_PIC_OFFSET, HSIZE, ulPicOffsetLeft) |
            BCHP_FIELD_DATA(SCL_0_PIC_OFFSET, VSIZE, ulPicOffsetTop));

#if (BVDC_P_SUPPORT_SCL_VER >= BVDC_P_SUPPORT_SCL_VER_8)
        BVDC_P_SCL_GET_REG_DATA(SCL_0_PIC_OFFSET_R) &= ~(
            BCHP_MASK(SCL_0_PIC_OFFSET_R, ENABLE) |
            BCHP_MASK(SCL_0_PIC_OFFSET_R, HSIZE));
        BVDC_P_SCL_GET_REG_DATA(SCL_0_PIC_OFFSET_R) |=  (
            BCHP_FIELD_DATA(SCL_0_PIC_OFFSET_R, ENABLE, (ulPicOffsetLeft != ulPicOffsetLeft_R)) |
            BCHP_FIELD_DATA(SCL_0_PIC_OFFSET_R, HSIZE, ulPicOffsetLeft_R));
#endif

        if(pPicture->eSrcOrientation != pPicture->eOrigSrcOrientation)
        {
            ulBvbInHSize <<=
                ((pPicture->eOrigSrcOrientation == BFMT_Orientation_e3D_LeftRight) &&
                 (pPicture->eSrcOrientation == BFMT_Orientation_e2D));
        }
        BVDC_P_SCL_GET_REG_DATA(SCL_0_BVB_IN_SIZE) &= ~(
            BCHP_MASK(SCL_0_BVB_IN_SIZE, HSIZE) |
            BCHP_MASK(SCL_0_BVB_IN_SIZE, VSIZE));
        BVDC_P_SCL_GET_REG_DATA(SCL_0_BVB_IN_SIZE) |=  (
            BCHP_FIELD_DATA(SCL_0_BVB_IN_SIZE, HSIZE, ulBvbInHSize) |
            BCHP_FIELD_DATA(SCL_0_BVB_IN_SIZE, VSIZE, ulBvbInVSize));

        /* SRC_PIC_SIZE should include FIR_LUMA_SRC_PIC_OFFSET and align */
        BVDC_P_SCL_GET_REG_DATA(SCL_0_SRC_PIC_SIZE) &= ~(
            BCHP_MASK(SCL_0_SRC_PIC_SIZE, HSIZE) |
            BCHP_MASK(SCL_0_SRC_PIC_SIZE, VSIZE));
        BVDC_P_SCL_GET_REG_DATA(SCL_0_SRC_PIC_SIZE) |=  (
            BCHP_FIELD_DATA(SCL_0_SRC_PIC_SIZE, HSIZE, ulAlgnSrcHSize) |
            BCHP_FIELD_DATA(SCL_0_SRC_PIC_SIZE, VSIZE, ulAlgnSrcVSize));

        /* If it is horizontally scaled down, we do horizontal scale first */
        if(ulHrzStep >= BVDC_P_SCL_H_RATIO_ONE)
        {
            BVDC_P_SCL_GET_REG_DATA(SCL_0_TOP_CONTROL) &= ~(
                BCHP_MASK(SCL_0_TOP_CONTROL, FILTER_ORDER ));
            BVDC_P_SCL_GET_REG_DATA(SCL_0_TOP_CONTROL) |=  (
                BCHP_FIELD_ENUM(SCL_0_TOP_CONTROL, FILTER_ORDER, HORIZ_FIRST));

            ulVertSclSrcWidth = ulDstHSize;
        }

        /* -----------------------------------------------------------------------
         * 4). Now we compute vertical scale factor
         */
        /* calculate the overall Scaling Factors for V */
        /* NOTE: the scaling source should have clipped the pan/scan offset. */
        ulNrmVrtSrcStep = (BAVC_Polarity_eFrame == pPicture->eSrcPolarity)?
            pPicture->ulNrmVrtSrcStep : (pPicture->ulNrmVrtSrcStep >> 1);  /* U12.20 */
        ulNrmVrtSrcStep = (BAVC_Polarity_eFrame == pPicture->eDstPolarity
#if BFMT_LEGACY_3DTV_SUPPORT
            && (!BFMT_IS_CUSTOM_1080P3D(pPicture->hBuffer->hWindow->hCompositor->stCurInfo.pFmtInfo->eVideoFmt))
#endif
            )? ulNrmVrtSrcStep : (ulNrmVrtSrcStep << 1); /* U12.20 */

        /* round up to avoid accuracy loss */
        ulVertStepRoundoff = (1 << (BVDC_P_NRM_SRC_STEP_F_BITS - BVDC_P_SCL_V_RATIO_F_BITS)) - 1;
        ulFirVrtStep = ulVrtStep = (ulNrmVrtSrcStep + ulVertStepRoundoff) >>
            (BVDC_P_NRM_SRC_STEP_F_BITS - BVDC_P_SCL_V_RATIO_F_BITS);    /* U12.14 */


        if (((1<<BVDC_P_NRM_SRC_STEP_F_BITS) == ulNrmVrtSrcStep) &&
                     (0 == ulPanScanTop) &&
                     (ulSrcVSize == ulDstVSize) &&
                     (!hScaler->stCtIndex.ulSclVertLuma) &&
                     (!hScaler->stCtIndex.ulSclVertChroma))
        {
            /* unity scale and no phase shift, turn off FIR for accuracy */
            BVDC_P_SCL_GET_REG_DATA(SCL_0_VERT_CONTROL) &= ~(
                           BCHP_MASK(SCL_0_VERT_CONTROL, SEL_4TAP_IN_FIR8 ) |
                           BCHP_MASK(SCL_0_VERT_CONTROL, FIR_ENABLE       ));
            BVDC_P_SCL_GET_REG_DATA(SCL_0_VERT_CONTROL) |=  (
                BCHP_FIELD_ENUM(SCL_0_VERT_CONTROL, SEL_4TAP_IN_FIR8, DISABLE) |
                BCHP_FIELD_ENUM(SCL_0_VERT_CONTROL, FIR_ENABLE,       OFF    ));

            BDBG_MODULE_MSG(BVDC_FIR_BYPASS, ("scl[%d] VFIR_ENABLE OFF", hScaler->eId));
        }
        else
        {
            BVDC_P_SCL_GET_REG_DATA(SCL_0_VERT_CONTROL) &= ~(
                BCHP_MASK(SCL_0_VERT_CONTROL, SEL_4TAP_IN_FIR8 ) |
                BCHP_MASK(SCL_0_VERT_CONTROL, FIR_ENABLE       ));
            BVDC_P_SCL_GET_REG_DATA(SCL_0_VERT_CONTROL) |=  (
                BCHP_FIELD_ENUM(SCL_0_VERT_CONTROL, SEL_4TAP_IN_FIR8, DISABLE) |
                BCHP_FIELD_ENUM(SCL_0_VERT_CONTROL, FIR_ENABLE,       ON    ));
        }

        {

            if(pPicture->eSrcPolarity != BAVC_Polarity_eFrame)
            {
                /* src = T/B */
                if(pPicture->eDstPolarity == BAVC_Polarity_eFrame)
                {
                    /* src = T/B, dst = F => pick max out of the 2 */
                    if(BERR_SUCCESS != BVDC_P_Scaler_CalVertInitPhase_isr(hScaler,
                        BAVC_Polarity_eTopField, BAVC_Polarity_eFrame,    ulPanScanTop, &ulBlkAvgSize,&ulFirVrtStep, ulAlgnSrcVSize, ulDstVSize))
                        BDBG_MSG(("Max BlkAvg but fail constraint %dx%d(T) => %dx%d(F)",
                            pSclIn->ulWidth,  pSclIn->ulHeight,
                            pSclOut->ulWidth, pSclOut->ulHeight));
                    if(BERR_SUCCESS != BVDC_P_Scaler_CalVertInitPhase_isr(hScaler,
                        BAVC_Polarity_eBotField, BAVC_Polarity_eFrame,    ulPanScanTop, &ulBlkAvgSize,&ulFirVrtStep, ulAlgnSrcVSize, ulDstVSize))
                        BDBG_MSG(("Max BlkAvg but fail constraint %dx%d(B) => %dx%d(F)",
                            pSclIn->ulWidth,  pSclIn->ulHeight,
                            pSclOut->ulWidth, pSclOut->ulHeight));
                    BDBG_MSG(("T/B => F: largest ulBlkAvgSize = %d", ulBlkAvgSize));
                }
                else
                {
                    /* src = T/B, dst = T/B => pick max out of the 4 */
                    if(BERR_SUCCESS != BVDC_P_Scaler_CalVertInitPhase_isr(hScaler,
                        BAVC_Polarity_eTopField, BAVC_Polarity_eTopField, ulPanScanTop, &ulBlkAvgSize,&ulFirVrtStep, ulAlgnSrcVSize, ulDstVSize))
                        BDBG_MSG(("Max BlkAvg but fail constraint %dx%d(T) => %dx%d(T)",
                            pSclIn->ulWidth,  pSclIn->ulHeight,
                            pSclOut->ulWidth, pSclOut->ulHeight));
                    if(BERR_SUCCESS != BVDC_P_Scaler_CalVertInitPhase_isr(hScaler,
                        BAVC_Polarity_eBotField, BAVC_Polarity_eTopField, ulPanScanTop, &ulBlkAvgSize,&ulFirVrtStep, ulAlgnSrcVSize, ulDstVSize))
                        BDBG_MSG(("Max BlkAvg but fail constraint %dx%d(B) => %dx%d(T)",
                            pSclIn->ulWidth,  pSclIn->ulHeight,
                            pSclOut->ulWidth, pSclOut->ulHeight));
                    if(BERR_SUCCESS != BVDC_P_Scaler_CalVertInitPhase_isr(hScaler,
                        BAVC_Polarity_eTopField, BAVC_Polarity_eBotField, ulPanScanTop, &ulBlkAvgSize,&ulFirVrtStep, ulAlgnSrcVSize, ulDstVSize))
                        BDBG_MSG(("Max BlkAvg but fail constraint %dx%d(T) => %dx%d(B)",
                            pSclIn->ulWidth,  pSclIn->ulHeight,
                            pSclOut->ulWidth, pSclOut->ulHeight));
                    if(BERR_SUCCESS != BVDC_P_Scaler_CalVertInitPhase_isr(hScaler,
                        BAVC_Polarity_eBotField, BAVC_Polarity_eBotField,  ulPanScanTop, &ulBlkAvgSize,&ulFirVrtStep, ulAlgnSrcVSize, ulDstVSize))
                        BDBG_MSG(("Max BlkAvg but fail constraint %dx%d(B) => %dx%d(B)",
                            pSclIn->ulWidth,  pSclIn->ulHeight,
                            pSclOut->ulWidth, pSclOut->ulHeight));
                    BDBG_MSG(("T/B => T/B: largest ulBlkAvgSize = %d", ulBlkAvgSize));
                }
            }
            else
            {
                /* src = F */
                if(pPicture->eDstPolarity != BAVC_Polarity_eFrame)
                {
                    /* src = F, dst = T/B => pick max out of the 2 */
                    if(BERR_SUCCESS != BVDC_P_Scaler_CalVertInitPhase_isr(hScaler,
                        BAVC_Polarity_eFrame,    BAVC_Polarity_eTopField, ulPanScanTop, &ulBlkAvgSize,&ulFirVrtStep, ulAlgnSrcVSize, ulDstVSize))
                        BDBG_MSG(("Max BlkAvg but fail constraint %dx%d(F) => %dx%d(T)",
                            pSclIn->ulWidth,  pSclIn->ulHeight,
                            pSclOut->ulWidth, pSclOut->ulHeight));
                    if(BERR_SUCCESS != BVDC_P_Scaler_CalVertInitPhase_isr(hScaler,
                        BAVC_Polarity_eFrame,    BAVC_Polarity_eBotField, ulPanScanTop, &ulBlkAvgSize,&ulFirVrtStep, ulAlgnSrcVSize, ulDstVSize))
                        BDBG_MSG(("Max BlkAvg but fail constraint %dx%d(F) => %dx%d(B)",
                            pSclIn->ulWidth,  pSclIn->ulHeight,
                            pSclOut->ulWidth, pSclOut->ulHeight));
                    BDBG_MSG(("F => T/B: largest ulBlkAvgSize = %d", ulBlkAvgSize));
                }
                else
                {
                    if(BERR_SUCCESS != BVDC_P_Scaler_CalVertInitPhase_isr(hScaler,
                        BAVC_Polarity_eFrame,    BAVC_Polarity_eFrame,    ulPanScanTop, &ulBlkAvgSize,&ulFirVrtStep, ulAlgnSrcVSize, ulDstVSize))
                        BDBG_MSG(("Max BlkAvg but fail constraint %dx%d(F) => %dx%d(F)",
                            pSclIn->ulWidth,  pSclIn->ulHeight,
                            pSclOut->ulWidth, pSclOut->ulHeight));
                    BDBG_MSG(("F => F: largest ulBlkAvgSize = %d", ulBlkAvgSize));
                }
            }

            /* Block averaging is another cheap way of downscaling. */
            ulPanScanTop  /= (ulBlkAvgSize + 1);
            ulFirSrcVSize /= (ulBlkAvgSize + 1);
        }

        BVDC_P_SCL_GET_REG_DATA(SCL_0_VERT_CONTROL) &= ~(
            BCHP_MASK(SCL_0_VERT_CONTROL, BAVG_BLK_SIZE));
        BVDC_P_SCL_GET_REG_DATA(SCL_0_VERT_CONTROL) |=  (
            BCHP_FIELD_DATA(SCL_0_VERT_CONTROL, BAVG_BLK_SIZE, ulBlkAvgSize));

        ulFirVrtStep = BVDC_P_SCL_V_RATIO_TRUNC(ulFirVrtStep);
        BVDC_P_SCL_SET_VERT_RATIO(ulFirVrtStep);

        /* -----------------------------------------------------------------------
         * 5). set coeffs for both horizontal and vertical
         */
        pHorzFirCoeff = BVDC_P_SelectFirCoeff_isr(hScaler->pHorzFirCoeffTbl,
            hScaler->hWindow->stCurInfo.stCtIndex.ulSclHorzLuma,
            hScaler->hWindow->stCurInfo.hSource->stCurInfo.eCtInputType,
            BVDC_P_CtOutput_eAny,
            pPicture->PicComRulInfo.eSrcOrigPolarity, ulFirHrzStep,
            hScaler->ulHorzTaps, ulFirSrcHSize, ulFirDstHSize);
        BDBG_MSG(("Luma H Coeffs  : %s", pHorzFirCoeff->pchName));
        BVDC_P_Scaler_SetHorizFirCoeff_isr(hScaler, pHorzFirCoeff->pulCoeffs);

        pHorzFirCoeff = BVDC_P_SelectFirCoeff_isr(hScaler->pChromaHorzFirCoeffTbl,
            hScaler->hWindow->stCurInfo.stCtIndex.ulSclHorzChroma,
            hScaler->hWindow->stCurInfo.hSource->stCurInfo.eCtInputType,
            BVDC_P_CtOutput_eAny,
            pPicture->PicComRulInfo.eSrcOrigPolarity, ulFirHrzStep,
            hScaler->ulHorzTaps, ulFirSrcHSize, ulFirDstHSize);
        BDBG_MSG(("Chroma H Coeffs: %s", pHorzFirCoeff->pchName));
        BVDC_P_Scaler_SetChromaHorizFirCoeff_isr
            (hScaler, pHorzFirCoeff->pulCoeffs);

        /* PR48032: Account for Kell Factor.  In the case of P -> I.
         * Treat Destination as (KELL_FACTOR * Destination_lines * 2).  For
         * example desintaion is:
         *  1280x720p -> 1920x1080i, then it's (540 lines * 2 * KELL_FACTOR).
         * = (540 * 2 * 0.70).  In PI term it's
         * Sy = in_y / out_y;
         * Sy = 1280 / (540 * 2 * KELL_FACTOR).  So to account for KELL factor
         * We needed
         */
        /* If scale factor (Sy = DstH / SrcH) is within range (Sy <= 1.5 and
         * Sy != 1), use Kell factor filter, otherwise no filter (PR36666).
         * Internally, Sy = in_y / out_y = SrcH / DstH * 2 (for interlace case)
         * therefore the range is Sy <= 1/1.5 or Sy <= 0.6667 and Sy != 1
         */
        ulFirVrtStepWithKellFactor = (
            (BAVC_Polarity_eFrame == pPicture->eSrcPolarity) &&
            (BAVC_Polarity_eFrame != pPicture->eDstPolarity) &&
            ((ulVrtStep / 2) >= BVDC_P_SCL_V_RATIO_KELL_RANGE &&
             (ulVrtStep / 2) != BVDC_P_SCL_V_RATIO_ONE))
            ? BVDC_P_APPLY_KELL_FACTOR(ulFirVrtStep)     /* P -> I w/ kell */
            : ulFirVrtStep;                              /* normal */

        eOutputType = BVDC_P_DISPLAY_USED_STG(hScaler->hWindow->hCompositor->hDisplay->eMasterTg)?
            BVDC_P_CtOutput_eStg : BVDC_P_CtOutput_eDisp;
        /* We need to choose good fir coefficients base on adjusted
         * fir scale down ratio, src, dst size (of FIR). */
        hScaler->pVertFirCoeff = BVDC_P_SelectFirCoeff_isr(hScaler->pVertFirCoeffTbl,
            hScaler->hWindow->stCurInfo.stCtIndex.ulSclVertLuma,
            hScaler->hWindow->stCurInfo.hSource->stCurInfo.eCtInputType,
            eOutputType,
            pPicture->PicComRulInfo.eSrcOrigPolarity, ulFirVrtStepWithKellFactor,
            hScaler->ulVertTaps, ulFirSrcVSize, ulFirDstVSize);
        hScaler->pChromaVertFirCoeff = BVDC_P_SelectFirCoeff_isr(hScaler->pChromaVertFirCoeffTbl,
            hScaler->hWindow->stCurInfo.stCtIndex.ulSclVertChroma,
            hScaler->hWindow->stCurInfo.hSource->stCurInfo.eCtInputType,
            eOutputType,
            pPicture->PicComRulInfo.eSrcOrigPolarity, ulFirVrtStepWithKellFactor,
            hScaler->ulVertTaps, ulFirSrcVSize, ulFirDstVSize);

        /* to void point sampling if init phase has fraction */
        if((BVDC_P_SCL_V_RATIO_ONE == ulFirVrtStepWithKellFactor) &&
           (!hScaler->hWindow->stCurInfo.stCtIndex.ulSclVertLuma)&&
           (!hScaler->hWindow->stCurInfo.stCtIndex.ulSclVertChroma))
        {
            /* using "ulFirVrtStepWithKellFactor - 1" leads to use a 1toN coeffient */
            hScaler->pFracInitPhaseVertFirCoeff = BVDC_P_SelectFirCoeff_isr(hScaler->pVertFirCoeffTbl,
                hScaler->hWindow->stCurInfo.stCtIndex.ulSclVertLuma,
                hScaler->hWindow->stCurInfo.hSource->stCurInfo.eCtInputType,
                eOutputType,
                pPicture->PicComRulInfo.eSrcOrigPolarity, ulFirVrtStepWithKellFactor - 1,
                hScaler->ulVertTaps, ulFirSrcVSize, ulFirDstVSize);
            hScaler->pChromaFracInitPhaseVertFirCoeff = BVDC_P_SelectFirCoeff_isr(hScaler->pChromaVertFirCoeffTbl,
                hScaler->hWindow->stCurInfo.stCtIndex.ulSclVertChroma,
                hScaler->hWindow->stCurInfo.hSource->stCurInfo.eCtInputType,
                eOutputType,
                pPicture->PicComRulInfo.eSrcOrigPolarity, ulFirVrtStepWithKellFactor - 1,
                hScaler->ulVertTaps, ulFirSrcVSize, ulFirDstVSize);

            /* if field inversion, therefore sub-pixel phase, is expected, point sampling
             * should be completely avoided. This is true src and display are interlaced and
             * "50 to 60Hz conversion is performed, or 60 to 50Hz conversion is performed */
            if(hScaler->bHandleFldInv)
            {
                hScaler->pVertFirCoeff = hScaler->pFracInitPhaseVertFirCoeff;
                hScaler->pChromaVertFirCoeff = hScaler->pChromaFracInitPhaseVertFirCoeff;
            }
        }
        else
        {
            /* in this case regular coeffs are not point sampling */
            hScaler->pFracInitPhaseVertFirCoeff = hScaler->pVertFirCoeff;
            hScaler->pChromaFracInitPhaseVertFirCoeff = hScaler->pChromaVertFirCoeff;
        }

        /* SW7420-560, SW7420-721: use smoothen vertical coefficient to improve weaving */

        /* DEJAGGING/DERINGING settigns */
        if(hScaler->bDeJagging)
        {
            bool bDeJagging = (
                (hScaler->stSclSettings.bSclVertDejagging) &&
                (ulFirVrtStepWithKellFactor < BVDC_P_SCL_DEJAGGING_ON_THESHOLD));

            BVDC_P_SCL_GET_REG_DATA(SCL_0_DEJAGGING) = (
#if (BVDC_P_SUPPORT_SCL_VER >= BVDC_P_SUPPORT_SCL_VER_8)
                BCHP_FIELD_DATA(SCL_0_DEJAGGING, CORN, 8                                        ) |
#endif
                BCHP_FIELD_DATA(SCL_0_DEJAGGING, HORIZ,hScaler->stSclSettings.ulSclDejaggingHorz) | /* U2.3 */
                BCHP_FIELD_DATA(SCL_0_DEJAGGING, GAIN, hScaler->stSclSettings.ulSclDejaggingGain) | /* U2.3 */
                BCHP_FIELD_DATA(SCL_0_DEJAGGING, CORE, hScaler->stSclSettings.ulSclDejaggingCore) |
                BCHP_FIELD_DATA(SCL_0_DEJAGGING, DEMO_MODE,
                    (BVDC_SplitScreenMode_eDisable != hScaler->hWindow->stCurInfo.stSplitScreenSetting.eDeJagging)) |
                ((bDeJagging)
                ? BCHP_FIELD_ENUM(SCL_0_DEJAGGING, VERT_DEJAGGING, ON)
                : BCHP_FIELD_ENUM(SCL_0_DEJAGGING, VERT_DEJAGGING, OFF))                         );

            BVDC_P_SCL_GET_REG_DATA(SCL_0_DEJAGGING_DEMO_SETTING) = (
                ((BVDC_SplitScreenMode_eLeft == hScaler->hWindow->stCurInfo.stSplitScreenSetting.eDeJagging)
                ? BCHP_FIELD_ENUM(SCL_0_DEJAGGING_DEMO_SETTING, DEMO_L_R, LEFT )
                : BCHP_FIELD_ENUM(SCL_0_DEJAGGING_DEMO_SETTING, DEMO_L_R, RIGHT)         )|
                BCHP_FIELD_DATA(SCL_0_DEJAGGING_DEMO_SETTING, DEMO_BOUNDARY, ulDstHSize/2));
        }

        if(hScaler->bDeRinging)
        {
            BVDC_P_SCL_GET_REG_DATA(SCL_0_DERINGING) = (
                BCHP_FIELD_DATA(SCL_0_DERINGING, DEMO_MODE,
                    (BVDC_SplitScreenMode_eDisable != hScaler->hWindow->stCurInfo.stSplitScreenSetting.eDeRinging)) |
#if (BVDC_P_SUPPORT_SCL_VER >= BVDC_P_SUPPORT_SCL_VER_8)
                ((hScaler->bPqNcl)
                ? BCHP_FIELD_ENUM(SCL_0_DERINGING, VERT_CHROMA_PASS_FIRST_RING, DISABLE)
                : BCHP_FIELD_ENUM(SCL_0_DERINGING, VERT_CHROMA_PASS_FIRST_RING, ENABLE) ) |
                ((hScaler->bPqNcl)
                ? BCHP_FIELD_ENUM(SCL_0_DERINGING, VERT_LUMA_PASS_FIRST_RING, DISABLE)
                : BCHP_FIELD_ENUM(SCL_0_DERINGING, VERT_LUMA_PASS_FIRST_RING, ENABLE)   ) |
                ((hScaler->bPqNcl)
                ? BCHP_FIELD_ENUM(SCL_0_DERINGING, HORIZ_CHROMA_PASS_FIRST_RING, DISABLE)
                : BCHP_FIELD_ENUM(SCL_0_DERINGING, HORIZ_CHROMA_PASS_FIRST_RING, ENABLE)) |
                ((hScaler->bPqNcl)
                ? BCHP_FIELD_ENUM(SCL_0_DERINGING, HORIZ_LUMA_PASS_FIRST_RING, DISABLE)
                : BCHP_FIELD_ENUM(SCL_0_DERINGING, HORIZ_LUMA_PASS_FIRST_RING, ENABLE)  ) |
#endif
                BCHP_FIELD_ENUM(SCL_0_DERINGING, VERT_CHROMA_DERINGING,  ON) |
                BCHP_FIELD_ENUM(SCL_0_DERINGING, VERT_LUMA_DERINGING,    ON) |
                BCHP_FIELD_ENUM(SCL_0_DERINGING, HORIZ_CHROMA_DERINGING, ON) |
                BCHP_FIELD_ENUM(SCL_0_DERINGING, HORIZ_LUMA_DERINGING,   ON) );

            BVDC_P_SCL_GET_REG_DATA(SCL_0_DERING_DEMO_SETTING) =  (
                ((BVDC_SplitScreenMode_eLeft == hScaler->hWindow->stCurInfo.stSplitScreenSetting.eDeRinging)
                ? BCHP_FIELD_ENUM(SCL_0_DERING_DEMO_SETTING, DEMO_L_R, LEFT )
                : BCHP_FIELD_ENUM(SCL_0_DERING_DEMO_SETTING, DEMO_L_R, RIGHT)  )|
                BCHP_FIELD_DATA(SCL_0_DERING_DEMO_SETTING, DEMO_BOUNDARY, ulDstHSize/2));
        }

        /* -----------------------------------------------------------------------
         * 6). set init phase for both horizontal and vertical
         */
        /* Compute the phase accumulate intial value in S11.6 or S5.26 */
        lHrzPhsAccInit = BVDC_P_FIXED_A_MINUS_FIXED_B(
            BVDC_P_H_INIT_PHASE_RATIO_ADJ(ulFirHrzStepInit) / 2,
            BVDC_P_H_INIT_PHASE_0_POINT_5);

        BVDC_P_SCL_GET_REG_DATA(SCL_0_HORIZ_FIR_LUMA_SRC_PIC_OFFSET) &= ~(
            BCHP_MASK(SCL_0_HORIZ_FIR_LUMA_SRC_PIC_OFFSET, VALUE));
        BVDC_P_SCL_GET_REG_DATA(SCL_0_HORIZ_FIR_CHROMA_SRC_PIC_OFFSET) &= ~(
            BCHP_MASK(SCL_0_HORIZ_FIR_CHROMA_SRC_PIC_OFFSET, VALUE));
        BVDC_P_SCL_GET_REG_DATA(SCL_0_HORIZ_FIR_LUMA_SRC_PIC_OFFSET) |=  (
            BCHP_FIELD_DATA(SCL_0_HORIZ_FIR_LUMA_SRC_PIC_OFFSET, VALUE,
            ulPanScanLeft));
        BVDC_P_SCL_GET_REG_DATA(SCL_0_HORIZ_FIR_CHROMA_SRC_PIC_OFFSET) |=  (
            BCHP_FIELD_DATA(SCL_0_HORIZ_FIR_CHROMA_SRC_PIC_OFFSET, VALUE,
            ulPanScanLeft));

        BVDC_P_SCL_GET_REG_DATA(SCL_0_HORIZ_FIR_INIT_PHASE_ACC) &= ~(
            BCHP_MASK(SCL_0_HORIZ_FIR_INIT_PHASE_ACC, SIZE));
        BVDC_P_SCL_GET_REG_DATA(SCL_0_HORIZ_FIR_INIT_PHASE_ACC) |=  (
            BCHP_FIELD_DATA(SCL_0_HORIZ_FIR_INIT_PHASE_ACC, SIZE,
            lHrzPhsAccInit));

        /* Compute Vertical FIR initial phase.
         * Do we want scaler to invert the field?  To invert field we will
         * need to change the vertical initial phase a according to the given
         * formula for doing TF2BF, and BF2TF.
         * NOTE:
         *   - this formula is based on field middle line alignment; */
        ulVsr = BVDC_P_V_INIT_PHASE_RATIO_ADJ(ulFirVrtStep);

        /* to FRM */
        hScaler->aaulInitPhase[BAVC_Polarity_eFrame   ][BAVC_Polarity_eFrame   ] =
            BVDC_P_FIXED_A_MINUS_FIXED_B(ulVsr / 2, BVDC_P_V_INIT_PHASE_1_POINT_0 / 2) + ulPanScanTop;

        hScaler->aaulInitPhase[BAVC_Polarity_eTopField][BAVC_Polarity_eFrame   ] =
            BVDC_P_FIXED_A_MINUS_FIXED_B(ulVsr / 2, BVDC_P_V_INIT_PHASE_1_POINT_0 / 4) + ulPanScanTop;

        hScaler->aaulInitPhase[BAVC_Polarity_eBotField][BAVC_Polarity_eFrame   ] =
            BVDC_P_FIXED_A_MINUS_FIXED_B(ulVsr / 2, BVDC_P_V_INIT_PHASE_1_POINT_0 * 3 / 4) + ulPanScanTop;

        /* to TF */
        hScaler->aaulInitPhase[BAVC_Polarity_eFrame   ][BAVC_Polarity_eTopField] =
            BVDC_P_FIXED_A_MINUS_FIXED_B(ulVsr / 4, BVDC_P_V_INIT_PHASE_1_POINT_0 / 2) + ulPanScanTop;

        hScaler->aaulInitPhase[BAVC_Polarity_eTopField][BAVC_Polarity_eTopField] =
            BVDC_P_FIXED_A_MINUS_FIXED_B(ulVsr / 4, BVDC_P_V_INIT_PHASE_1_POINT_0 / 4) + ulPanScanTop;

        hScaler->aaulInitPhase[BAVC_Polarity_eBotField][BAVC_Polarity_eTopField] =
            BVDC_P_FIXED_A_MINUS_FIXED_B(ulVsr / 4, BVDC_P_V_INIT_PHASE_1_POINT_0 * 3 / 4) + ulPanScanTop;

        /* to BF */
        hScaler->aaulInitPhase[BAVC_Polarity_eFrame   ][BAVC_Polarity_eBotField] =
            BVDC_P_FIXED_A_MINUS_FIXED_B(ulVsr * 3 / 4, BVDC_P_V_INIT_PHASE_1_POINT_0 / 2) + ulPanScanTop;

        hScaler->aaulInitPhase[BAVC_Polarity_eTopField][BAVC_Polarity_eBotField] =
            BVDC_P_FIXED_A_MINUS_FIXED_B(ulVsr * 3 / 4, BVDC_P_V_INIT_PHASE_1_POINT_0 / 4) + ulPanScanTop;

        hScaler->aaulInitPhase[BAVC_Polarity_eBotField][BAVC_Polarity_eBotField] =
            BVDC_P_FIXED_A_MINUS_FIXED_B(ulVsr * 3 / 4, BVDC_P_V_INIT_PHASE_1_POINT_0 * 3 / 4) + ulPanScanTop;

        /*
         * Shift vertical init phase by 1/4 line when:
         * 1) it is a 50Hz to 60Hz conversion and
         * 2) input and output are both interlaced without MAD and
         * 3) vertical scaling src_step is integer number
         * note: interger ratio scale up might need similar treat to avoid periodic
         * integer initial phase, but interger ratio scale up is unlikely practically
         */
        if ((hScaler->bHandleFldInv) &&
            (0 == (ulFirVrtStep & (BVDC_P_SCL_V_RATIO_ONE - 1))))
        {
            BDBG_MSG(("shift phase -0.25"));
            hScaler->aaulInitPhase[BAVC_Polarity_eTopField][BAVC_Polarity_eTopField] -= BVDC_P_V_INIT_PHASE_0_POINT_25;
            hScaler->aaulInitPhase[BAVC_Polarity_eTopField][BAVC_Polarity_eBotField] -= BVDC_P_V_INIT_PHASE_0_POINT_25;
            hScaler->aaulInitPhase[BAVC_Polarity_eBotField][BAVC_Polarity_eTopField] -= BVDC_P_V_INIT_PHASE_0_POINT_25;
            hScaler->aaulInitPhase[BAVC_Polarity_eBotField][BAVC_Polarity_eBotField] -= BVDC_P_V_INIT_PHASE_0_POINT_25;
        }
    }

    /* Vertical intial phase */
    ulVertInitPhase = hScaler->hWindow->stCurInfo.stSclSettings.bSclVertPhaseIgnore? 0 :
        hScaler->aaulInitPhase[pPicture->eSrcPolarity][pPicture->eDstPolarity];
    /* integer part of the offset */
    BVDC_P_SCL_GET_REG_DATA(SCL_0_VERT_FIR_SRC_PIC_OFFSET_INT) &= ~(
        BCHP_MASK(SCL_0_VERT_FIR_SRC_PIC_OFFSET_INT, VALUE));
    BVDC_P_SCL_GET_REG_DATA(SCL_0_VERT_FIR_SRC_PIC_OFFSET_INT) |=  (
        BCHP_FIELD_DATA(SCL_0_VERT_FIR_SRC_PIC_OFFSET_INT, VALUE,
        (ulVertInitPhase >> BVDC_P_SCL_V_INIT_PHASE_F_BITS)));

    /* fractional part;
       TODO: improve fixed point math to support higher precision offset */
    BVDC_P_SCL_GET_REG_DATA(SCL_0_VERT_FIR_SRC_PIC_OFFSET_FRAC) &= ~(
        BCHP_MASK(SCL_0_VERT_FIR_SRC_PIC_OFFSET_FRAC, VALUE));
    BVDC_P_SCL_GET_REG_DATA(SCL_0_VERT_FIR_SRC_PIC_OFFSET_FRAC) |=  (
        BCHP_FIELD_DATA(SCL_0_VERT_FIR_SRC_PIC_OFFSET_FRAC, VALUE,
        (ulVertInitPhase << BVDC_P_SCL_V_RATIO_F_BITS_EXT)));

    /* set vertical coeff change */
    if ((0 == (ulVertInitPhase & (BVDC_P_V_INIT_PHASE_1_POINT_0 - 1))) &&
        ((hScaler->pPrevVertFirCoeff != hScaler->pVertFirCoeff) ||
         (hScaler->pPrevChromaVertFirCoeff != hScaler->pChromaVertFirCoeff)))
    {
        /* init phase does not have fraction, use regular coeff */
        hScaler->ulUpdateCoeff = BVDC_P_RUL_UPDATE_THRESHOLD;
        hScaler->pPrevVertFirCoeff = hScaler->pVertFirCoeff;
        hScaler->pPrevChromaVertFirCoeff = hScaler->pChromaVertFirCoeff;
        BVDC_P_Scaler_SetVertFirCoeff_isr(
            hScaler, hScaler->pVertFirCoeff->pulCoeffs);
        BVDC_P_Scaler_SetChromaVertFirCoeff_isr(
            hScaler, hScaler->pChromaVertFirCoeff->pulCoeffs);
        BDBG_MSG(("Luma V Coeffs  : %s", hScaler->pVertFirCoeff->pchName));
        BDBG_MSG(("Chroma V Coeffs: %s", hScaler->pChromaVertFirCoeff->pchName));
    }
    else if ((0 != (ulVertInitPhase & (BVDC_P_V_INIT_PHASE_1_POINT_0 - 1))) &&
             ((hScaler->pPrevVertFirCoeff != hScaler->pFracInitPhaseVertFirCoeff) ||
              (hScaler->pPrevChromaVertFirCoeff != hScaler->pChromaFracInitPhaseVertFirCoeff)))
    {
        /* init phase has fraction, use special coeff for init-phase with fraction
         * to avoid point sampling */
        hScaler->ulUpdateCoeff = BVDC_P_RUL_UPDATE_THRESHOLD;
        hScaler->pPrevVertFirCoeff = hScaler->pFracInitPhaseVertFirCoeff;
        hScaler->pPrevChromaVertFirCoeff = hScaler->pChromaFracInitPhaseVertFirCoeff;
        BVDC_P_Scaler_SetVertFirCoeff_isr(
            hScaler, hScaler->pFracInitPhaseVertFirCoeff->pulCoeffs);
        BVDC_P_Scaler_SetChromaVertFirCoeff_isr(
            hScaler, hScaler->pChromaFracInitPhaseVertFirCoeff->pulCoeffs);
        BDBG_MSG(("Luma V Coeffs  : %s", hScaler->pFracInitPhaseVertFirCoeff->pchName));
        BDBG_MSG(("Chroma V Coeffs: %s", hScaler->pChromaFracInitPhaseVertFirCoeff->pchName));
    }

    BDBG_ASSERT((hScaler->pPrevVertFirCoeff == hScaler->pVertFirCoeff) ||
                (hScaler->pPrevVertFirCoeff == hScaler->pFracInitPhaseVertFirCoeff));

    BDBG_ASSERT(hScaler->pPrevVertFirCoeff);

    /* 3D support */
#if (BVDC_P_SUPPORT_SCL_VER >= BVDC_P_SUPPORT_SCL_VER_7)
    BVDC_P_SCL_GET_REG_DATA(SCL_0_VIDEO_3D_MODE) &= ~(
        BCHP_MASK(SCL_0_VIDEO_3D_MODE, BVB_VIDEO));
    BVDC_P_SCL_GET_REG_DATA(SCL_0_VIDEO_3D_MODE) |=  (
        BVDC_P_VNET_USED_SCALER_AT_WRITER(pPicture->stVnetMode) ?
        BCHP_FIELD_DATA(SCL_0_VIDEO_3D_MODE, BVB_VIDEO, pPicture->eSrcOrientation) :
        BCHP_FIELD_DATA(SCL_0_VIDEO_3D_MODE, BVB_VIDEO, pPicture->eDispOrientation));
#endif

    /* Printing out ratio in float format would be nice, but PI
    * code does permit float. */
    if(BVDC_P_RUL_UPDATE_THRESHOLD == hScaler->ulUpdateAll)
    {
        BDBG_MSG(("-------------------------"));
        BDBG_MSG(("Scaler[%d]chan[%d]  : %dx%d to %dx%d", hScaler->eId,pPicture->ulPictureIdx,
            pSclIn->ulWidth,  pSclIn->ulHeight,
            pSclOut->ulWidth, pSclOut->ulHeight));
        BDBG_MSG(("Scaler[%d]'clip    : %dx%d to %dx%d", hScaler->eId,
            pSclCut->ulWidth,  pSclCut->ulHeight,
            pSclOut->ulWidth, pSclOut->ulHeight));
        BDBG_MSG(("ulHrzStep         : %-8x", ulHrzStep));
        BDBG_MSG(("ulVrtStep         : %-8x", ulVrtStep<<BVDC_P_SCL_V_RATIO_F_BITS_EXT));
        BDBG_MSG(("ulFirHrzStep      : %-8x", ulFirHrzStep));
        BDBG_MSG(("ulFirVrtStep      : %-8x", ulFirVrtStep<<BVDC_P_SCL_V_RATIO_F_BITS_EXT));
        BDBG_MSG(("ulBlkAvgSize      : %d", ulBlkAvgSize));
        BDBG_MSG(("ulVertSclSrcWidth : %d", ulVertSclSrcWidth));
        BDBG_MSG(("HWF_0             : %s",
            BVDC_P_SCL_COMPARE_FIELD_NAME(SCL_0_HORIZ_CONTROL, HWF0_ENABLE, ON) ? "On" : "Off"));
        BDBG_MSG(("HWF_1             : %s",
            BVDC_P_SCL_COMPARE_FIELD_NAME(SCL_0_HORIZ_CONTROL, HWF1_ENABLE, ON) ? "On" : "Off"));
        BDBG_MSG(("H_PanScan         : %-8x", pSclCut->lLeft));
        BDBG_MSG(("V_PanScan         : %-8x", pSclCut->lTop));
        BDBG_MSG(("H_InitPhase       : %-8x", lHrzPhsAccInit));
        BDBG_MSG(("V_InitPhase       : %-8x", ulVertInitPhase));
        BDBG_MSG(("SrcPolarity       : %d", pPicture->eSrcPolarity));
        BDBG_MSG(("DstPolarity       : %d", pPicture->eDstPolarity));
    }

    BSTD_UNUSED(ulVertSclSrcWidth);
    BDBG_LEAVE(BVDC_P_Scaler_SetInfo_isr);
    return;
}

/***************************************************************************
 * {private}
 *
 */
void BVDC_P_Scaler_SetEnable_isr
    ( BVDC_P_Scaler_Handle             hScaler,
      bool                             bEnable )
{
    BDBG_OBJECT_ASSERT(hScaler, BVDC_SCL);

    if(!BVDC_P_SCL_COMPARE_FIELD_DATA(SCL_0_ENABLE, SCALER_ENABLE, bEnable))
    {
        hScaler->ulUpdateAll = BVDC_P_RUL_UPDATE_THRESHOLD;
    }

    /* Turn on/off the scaler. */
    BVDC_P_SCL_GET_REG_DATA(SCL_0_ENABLE) &= ~(
        BCHP_MASK(SCL_0_ENABLE, SCALER_ENABLE));

    BVDC_P_SCL_GET_REG_DATA(SCL_0_ENABLE) |=  (bEnable
        ? BCHP_FIELD_ENUM(SCL_0_ENABLE, SCALER_ENABLE, ON)
        : BCHP_FIELD_ENUM(SCL_0_ENABLE, SCALER_ENABLE, OFF));

    return;
}


/***************************************************************************
* {private}
*
*/
void BVDC_P_Scaler_5ZoneNonLinear_isr
    ( BVDC_P_Scaler_Handle             hScaler,
      const BVDC_P_PictureNodePtr      pPicture,
      uint32_t                         *pFirHrzStep,
      uint32_t                         *pHrzStep,
      uint32_t                         *pFirHrzStepInit)
{
    /* 5-region smooth non-liear horizontal scaling:
    *  0. we handle both aspect ratio expand and shrink, also between 480i 4:3 and 16:9
    *  1. horizontal divide 5-region in DEST domain;
    *  2. central region might not be aspect ratio correct;
    *  3. scale factor is constant in central region, then linearly changes to edge
    *  4. scaling coeffs are selected by central region;
    *  5. no extreme horizontal scaledown (h_ratio < 8) to avoid hw filtering;
    *  6. no scale-factor rounding;
    *  7. no overall aspect ratio correction or source cliping;
    *  8. no vbi pass-through;
    */
    uint32_t ulDstWidthRgn02, ulSrcWidthRgn02;
    uint32_t ulDstWidthRgn1 , ulSrcWidthRgn1;
    uint32_t ulDstWidthRgn3, ulSrcWidthRgn3;
    uint32_t ulSrcWidth, ulSclSrcWidth, ulDstWidth;
    uint32_t ulNumer, ulDenom;
    uint32_t ulDlt, ulDltInt, ulDltFrac, ulDltRem;
    uint32_t ulStep1, ulStepInit, ulStepRem, ulStepFrac, ulStep;
    uint32_t r1;
    bool bDltNeg, bNegUnderflow;

    /* 1) region sizes: dst region ends must be even number according to HW */
    ulSrcWidth      = pPicture->pSclIn->ulWidth;
    ulDstWidth      = pPicture->pSclOut->ulWidth;
    ulSrcWidthRgn02 = pPicture->ulNonlinearSrcWidth;
    ulDstWidthRgn02 = pPicture->ulNonlinearSclOutWidth;
    ulSrcWidthRgn1  = ulSrcWidth - (ulSrcWidthRgn02 << 1);
    ulDstWidthRgn1  = pPicture->ulCentralRegionSclOutWidth;

#define PIXELMASK                        (0xFFFFFFFE)
    ulDstWidthRgn3  = (ulDstWidthRgn02>>2) & PIXELMASK ;
    ulSrcWidthRgn3  = (ulSrcWidthRgn02>>2) & PIXELMASK;
    ulDstWidthRgn02 = ulDstWidthRgn02 - ulDstWidthRgn3;
    ulSrcWidthRgn02 = ulSrcWidthRgn02 - ulSrcWidthRgn3;
    BDBG_MSG(("Src Width %4d 0: %4d 1: %4d 3 %4d", ulSrcWidth, ulSrcWidthRgn02, ulSrcWidthRgn1, ulSrcWidthRgn3));
    BDBG_MSG(("Des Width %4d 0: %4d 1: %4d 3 %4d", ulDstWidth, ulDstWidthRgn02, ulDstWidthRgn1, ulDstWidthRgn3));

    /*@@@ Any HW limitation to be validated here */
    if (ulDstWidthRgn02 < 3)
        {
            ulDstWidthRgn1 =
                BVDC_P_MAX((3 - ulDstWidthRgn02), ulDstWidthRgn1) - (3 - ulDstWidthRgn02);
            ulDstWidthRgn02 = 3;
        }

    /* 2). calculate the delta: src_step's step increase/decrease in region 0 and 2.
    *
    * Lets use notation as the following
    * sw:       src width
    * dw:       scaler output width
    * sw n1/0/1/2/3:    src width of region n1 0, 1, 2 3
    * dw n1/0/1/2/3:    scaler output width of region n1, 0, 1, 2, 3
    * r1:       U5.26, scale ratio (really the src step) in region 1, i.e. ulNrmHrzStep
    * ri:       U5.26  scale ration in region n1 and 3.
    * dlt:      S1.26, delta, src_step's 2-step inc/dec at even pixels in region 0 and 2
    *
    * Since step increases on even pixel only in HW, we assume dw0 is even (enforced by win code),
    *   ri + dlt * (dw0/2) = r1
    * and
    *   ri * ulDstWidth + (r1 - ri) * (ulDstWidthRg1 + ulDstWidthRgn02)
    *   = dw*AvgScaleFactor = sw.
    * i.e
    *           (sw - r1*(ulDstWidthRgn02 + ulDstWidthRgn1))
    *   ri = --------------------------------------------------
    *           2*ulDstWidthRgn3 + ulDstWidthRgn02
    *
    * Therefore
    *          2*(r1* ulDstWidth - sw)
    *   dlt =  ---------------------------------------------------------------------
    *             ulDstWidthRgn02 * (2*ulDstWidthRgn3 + ulDstWidthRgn02)
    *
    * Note that dlt could be negative, but we must have ri >= 0. This leads some user's
    * set to non-linear src/sclOut region invalid.
    */
    ulSrcWidth    = (ulSrcWidth     << BVDC_P_NRM_SRC_STEP_F_BITS);
    r1            = (ulSrcWidthRgn1 << BVDC_P_NRM_SRC_STEP_F_BITS)/ulDstWidthRgn1;

    /*scale ratio ri of region 0 and 2*/
    ulSclSrcWidth = r1 * (ulDstWidthRgn1 + ulDstWidthRgn02);
    ulDenom       = 2* ulDstWidthRgn3 + ulDstWidthRgn02;
    ulNumer       = ulSrcWidth > ulSclSrcWidth ?
        (ulSrcWidth - ulSclSrcWidth):
        (ulSclSrcWidth - ulSrcWidth);

    ulStepInit    = ulNumer /ulDenom;
    ulStepRem     = ulNumer - ulStepInit*ulDenom;
    ulStepFrac    = (ulStepInit &((1 << BVDC_P_NRM_SRC_STEP_F_BITS) - 1))
        <<(BVDC_P_SCL_H_R_DLT_F_BITS - BVDC_P_NRM_SRC_STEP_F_BITS);
    ulStepInit    = ulStepInit >> BVDC_P_NRM_SRC_STEP_F_BITS;
    ulStepFrac    = ulStepFrac |((ulStepRem << (BVDC_P_SCL_H_R_DLT_F_BITS - BVDC_P_NRM_SRC_STEP_F_BITS))/ulDenom);
    ulStep        = (ulStepInit << BVDC_P_SCL_H_R_DLT_F_BITS) | ulStepFrac;

    /*delta calculation*/
    ulSclSrcWidth = ulDstWidth * r1;
    ulDenom       = (ulDstWidthRgn02*(2*ulDstWidthRgn3 + ulDstWidthRgn02));

    bDltNeg       = ulSclSrcWidth < ulSrcWidth;

    ulNumer       = bDltNeg? (ulSrcWidth - ulSclSrcWidth): (ulSclSrcWidth - ulSrcWidth);
    ulNumer       = (ulNumer <<1);
    ulDltInt      = ulNumer/ulDenom;            /*Int*2^20 to avoid accuracy loss*/
    ulDltRem      = ulNumer - ulDltInt*ulDenom;
    ulDltFrac     = ulDltInt & ((1 << BVDC_P_NRM_SRC_STEP_F_BITS) - 1);
    ulDltInt      = ulDltInt >> BVDC_P_NRM_SRC_STEP_F_BITS;
    ulDltRem      = ulDltRem << (BVDC_P_SCL_H_R_DLT_F_BITS - BVDC_P_NRM_SRC_STEP_F_BITS);
    ulDltFrac     = (ulDltRem / ulDenom)|(ulDltFrac <<(BVDC_P_SCL_H_R_DLT_F_BITS - BVDC_P_NRM_SRC_STEP_F_BITS));
    ulDlt         = (ulDltInt << (BVDC_P_SCL_H_R_DLT_F_BITS + BCHP_SCL_0_HORIZ_FIR_DEST_PIC_REGION_0_STEP_DELTA_SIZE_SHIFT))
        + ulDltFrac;
    /* Underflow control */
    ulStep1 = pPicture->ulNrmHrzSrcStep << (BVDC_P_SCL_H_RATIO_F_BITS - BVDC_P_NRM_SRC_STEP_F_BITS);
    r1 <<= (BVDC_P_SCL_H_R_DLT_F_BITS - BVDC_P_NRM_SRC_STEP_F_BITS);
    bNegUnderflow = (!bDltNeg) && (r1 <(ulDlt*ulDstWidthRgn02/2));
    BDBG_MSG(("Underflow %1d Step1 %8x Thd %8x %8x", bNegUnderflow, ulStep1, ulDstWidthRgn02, (ulDlt*ulDstWidthRgn02/2)));

    if(bNegUnderflow )
    {
        ulDlt  = 0;
        ulStep = ulSrcWidth /ulDstWidth;
        ulStep <<= (BVDC_P_SCL_H_R_DLT_F_BITS - BVDC_P_NRM_SRC_STEP_F_BITS);
        ulStep1 = ulStep;
        BDBG_ERR(("Need to set non-linear area bigger to avoid underflow %8d %d Step1 %8d ",
            (ulSrcWidth>>BVDC_P_NRM_SRC_STEP_F_BITS), ulDstWidth, (ulStep1>>BVDC_P_SCL_H_RATIO_F_BITS) ));
    }

    BDBG_MSG(("r1  %8x Step: %8x Int: %8x Frac %8x", r1, ulStep, ulStepInit, ulStepFrac));
    BDBG_MSG(("Delta: %8x Int: %8x Frac %8x bDltNeg %1d", ulDlt, ulDltInt, ulDltFrac, bDltNeg));

    /*set register*/
    BVDC_P_SCL_SET_HORZ_STEP_MISC(ulDstWidthRgn02 + ulDstWidthRgn1 + ulDstWidthRgn3, ulStep);

    BVDC_P_SCL_SET_HORZ_REGION3N1(N1,  ulDstWidthRgn3);
    BVDC_P_SCL_SET_HORZ_REGION3N1(3,   ulDstWidth);

    if(!bDltNeg)
    {
        BVDC_P_SCL_SET_HORZ_REGION02 (0,   ulDstWidthRgn3 + ulDstWidthRgn02,                     ulDlt);
        BVDC_P_SCL_SET_HORZ_REGION02 (2,   ulDstWidthRgn3 + ulDstWidthRgn1 + 2*ulDstWidthRgn02, -ulDlt);
    }
    else
    {
        BVDC_P_SCL_SET_HORZ_REGION02 (0,   ulDstWidthRgn3 + ulDstWidthRgn02,                    -ulDlt);
        BVDC_P_SCL_SET_HORZ_REGION02 (2,   ulDstWidthRgn3 + ulDstWidthRgn1 + 2*ulDstWidthRgn02,  ulDlt);
    }

    /* used outside */
    *pFirHrzStep = *pHrzStep = ulStep1;
    *pFirHrzStepInit = (ulStepInit << BVDC_P_SCL_H_R_DLT_F_BITS) | ulStepFrac;
}

/***************************************************************************
* {private}
*
* This funtion decide whether to Overwrite Src orientation if input
* buffer length surpass the vertical scaler line buffer depth
*/

bool BVDC_P_Scaler_Validate_VertDepth_isr
    (BVDC_Window_Handle                 hWindow,
     const BVDC_P_Scaler_Handle         hScaler)
{
    BFMT_Orientation                   eSrcOrientation;
    BFMT_Orientation                   eDstOrientation;
    BFMT_Orientation                   eOrientation;
    bool                               bOrientationLR;
    uint32_t                           ulSclVertDepth;
    uint32_t                           ulSclSrcWidth, ulSclDstWidth;
    BVDC_P_PictureNode                *pPicture;


    ulSclDstWidth = hWindow->stCurInfo.stScalerOutput.ulWidth;
    ulSclSrcWidth = hWindow->stSrcCnt.ulWidth;

    pPicture = hWindow->pCurWriterNode;

    eSrcOrientation = pPicture->eSrcOrientation;
    eDstOrientation = pPicture->eDispOrientation;
    eOrientation =
        BVDC_P_VNET_USED_SCALER_AT_WRITER(hWindow->stVnetMode) ?
        eSrcOrientation : eDstOrientation;


    bOrientationLR = (eOrientation == BFMT_Orientation_e3D_LeftRight);
    ulSclVertDepth = hScaler->ulVertLineDepth;

    if(((ulSclSrcWidth << bOrientationLR) > ulSclVertDepth) &&
        ((ulSclDstWidth<< bOrientationLR) > ulSclVertDepth))
    {
        BDBG_MSG(("Overwrite SrcOrientation 3D -> 2D: Scl [%d] line buffer length cannot be larger than %d ",
            hScaler->eId, hScaler->ulVertLineDepth));
        return (false);
    }
    return (true);
}

/* End of file. */
