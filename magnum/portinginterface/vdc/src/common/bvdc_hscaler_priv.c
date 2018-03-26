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
 * Module Description:
 *
 ***************************************************************************/
#include "bstd.h"
#include "bkni.h"
#include "bvdc.h"
#include "brdc.h"
#include "bvdc_hscaler_priv.h"
#include "bvdc_buffer_priv.h"
#include "bvdc_source_priv.h"
#include "bvdc_window_priv.h"
#include "bvdc_pep_priv.h"
#include "bvdc_vnet_priv.h"

#if BVDC_P_SUPPORT_HSCL_VER

#include "bchp_hscl_0.h"

#ifdef BCHP_HSCL_1_REG_START
#include "bchp_hscl_1.h"
#endif

BDBG_MODULE(BVDC_HSCL);
BDBG_FILE_MODULE(BVDC_FIR_BYPASS);
BDBG_OBJECT_ID(BVDC_HSL);

#define BVDC_P_MAKE_HSCALER(pHscaler, id)                                                    \
{                                                                                            \
    (pHscaler)->ulRegOffset      = BCHP_HSCL_##id##_REG_START - BCHP_HSCL_0_REG_START;       \
}
/***************************************************************************
 * {private}
 *
 */
BERR_Code BVDC_P_Hscaler_Create
    ( BVDC_P_Hscaler_Handle            *phHscaler,
      BVDC_P_HscalerId                  eHscalerId,
      BVDC_P_Resource_Handle            hResource,
      BREG_Handle                       hReg
)
{
    BVDC_P_HscalerContext *pHscaler;

    BDBG_ENTER(BVDC_P_Hscaler_Create);

    BDBG_ASSERT(phHscaler);

    /* Use: to see messages */
    /* BDBG_SetModuleLevel("BVDC_HSCL", BDBG_eMsg); */

    /* The handle will be NULL if create fails. */
    *phHscaler = NULL;

    /* Alloc the context. */
    pHscaler = (BVDC_P_HscalerContext*)
        (BKNI_Malloc(sizeof(BVDC_P_HscalerContext)));
    if(!pHscaler)
    {
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }

    /* Clear out the context and set defaults. */
    BKNI_Memset((void*)pHscaler, 0x0, sizeof(BVDC_P_HscalerContext));
    BDBG_OBJECT_SET(pHscaler, BVDC_HSL);

    pHscaler->eId          = eHscalerId;
    pHscaler->hReg         = hReg;
    pHscaler->ulRegOffset  = 0;

    switch(pHscaler->eId)
    {
        case BVDC_P_HscalerId_eHscl0:
            BVDC_P_MAKE_HSCALER(pHscaler, 0);
            break;
#ifdef BCHP_HSCL_1_REG_START
        case BVDC_P_HscalerId_eHscl1:
            BVDC_P_MAKE_HSCALER(pHscaler, 1);
            break;
#endif
#ifdef BCHP_HSCL_2_REG_START
        case BVDC_P_HscalerId_eHscl2:
            BVDC_P_MAKE_HSCALER(pHscaler, 2);
            break;
#endif
#ifdef BCHP_HSCL_3_REG_START
        case BVDC_P_HscalerId_eHscl3:
            BVDC_P_MAKE_HSCALER(pHscaler, 3);
            break;
#endif
#ifdef BCHP_HSCL_4_REG_START
        case BVDC_P_HscalerId_eHscl4:
            BVDC_P_MAKE_HSCALER(pHscaler, 4);
            break;
#endif
#ifdef BCHP_HSCL_5_REG_START
        case BVDC_P_HscalerId_eHscl5:
            BVDC_P_MAKE_HSCALER(pHscaler, 5);
            break;
#endif
        default:
            BDBG_ERR(("Need to handle BVDC_P_HscalerId_eHscl%d", pHscaler->eId));
            BDBG_ASSERT(0);
            break;
    }



    /* Init to the default filter coeffficient tables. */
    BVDC_P_GetFirCoeffs_isrsafe(&pHscaler->pHorzFirCoeffTbl, NULL);
    BVDC_P_GetChromaFirCoeffs_isrsafe(&pHscaler->pChromaHorzFirCoeffTbl, NULL);

    /* All done. now return the new fresh context to user. */
    *phHscaler = (BVDC_P_Hscaler_Handle)pHscaler;
    BSTD_UNUSED(hResource);

    BDBG_LEAVE(BVDC_P_Hscaler_Create);
    return BERR_SUCCESS;
}


/***************************************************************************
 * {private}
 *
 */
void BVDC_P_Hscaler_Destroy
    ( BVDC_P_Hscaler_Handle             hHscaler )
{
    BDBG_ENTER(BVDC_P_Hscaler_Destroy);
    BDBG_OBJECT_ASSERT(hHscaler, BVDC_HSL);

    BDBG_OBJECT_DESTROY(hHscaler, BVDC_HSL);
    /* Release context in system memory */
    BKNI_Free((void*)hHscaler);

    BDBG_LEAVE(BVDC_P_Hscaler_Destroy);
    return;
}


/***************************************************************************
 * {private}
 *
 */
void BVDC_P_Hscaler_Init_isr
    ( BVDC_P_Hscaler_Handle            hHscaler )
{
    uint32_t  ulReg;
    uint32_t  ulTaps;

    BDBG_ENTER(BVDC_P_Hscaler_Init_isr);
    BDBG_OBJECT_ASSERT(hHscaler, BVDC_HSL);

    /* Clear out shadow registers. */
    BKNI_Memset_isr((void*)hHscaler->aulRegs, 0x0, sizeof(hHscaler->aulRegs));

    hHscaler->ulPrevSrcWidth = 0xffffffff;
    /* one value is enough to cause re-setinfo
    hHscaler->lPrevHsclCutLeft = 0xffffffff;
    hHscaler->ulPrevHsclCutWidth = 0xffffffff;
    hHscaler->ulPrevOutWidth = 0xffffffff;
    hHscaler->ulPrevSrcHeight = 0xffffffff;*/

#if (BVDC_P_SUPPORT_HSCL_VER >= BVDC_P_SUPPORT_HSCL_VER_2)
    ulReg = BREG_Read32_isr(hHscaler->hReg, BCHP_HSCL_0_HW_CONFIGURATION + hHscaler->ulRegOffset);

    hHscaler->bDeringing = BCHP_GET_FIELD_DATA(ulReg, HSCL_0_HW_CONFIGURATION, DERINGING) ? true : false;
    ulTaps = BCHP_GET_FIELD_DATA(ulReg, HSCL_0_HW_CONFIGURATION, HORIZ_TAPS);
    switch(ulTaps)
    {
        case BCHP_HSCL_0_HW_CONFIGURATION_HORIZ_TAPS_HORIZ_8_TAPS:
            hHscaler->ulHorzTaps = BVDC_P_CT_8_TAP;
            break;
        case BCHP_HSCL_0_HW_CONFIGURATION_HORIZ_TAPS_HORIZ_12_TAPS:
            hHscaler->ulHorzTaps = BVDC_P_CT_12_TAP;
            break;
        case BCHP_HSCL_0_HW_CONFIGURATION_HORIZ_TAPS_HORIZ_16_TAPS:
            hHscaler->ulHorzTaps = BVDC_P_CT_16_TAP;
            break;
        default: break;
    }

#else
    hHscaler->ulHorzTaps = BVDC_P_CT_8_TAP;
    hHscaler->bDeringing = true;

    BSTD_UNUSED(ulReg);
    BSTD_UNUSED(ulTaps);
#endif

    if(hHscaler->bDeringing)
    {
        BVDC_P_HSCL_GET_REG_DATA(HSCL_0_DERINGING) &=  ~(
#if (BVDC_P_SUPPORT_HSCL_VER >= BVDC_P_SUPPORT_HSCL_VER_6)
            BCHP_MASK(HSCL_0_DERINGING, HORIZ_CHROMA_PASS_FIRST_RING ) |
            BCHP_MASK(HSCL_0_DERINGING, HORIZ_LUMA_PASS_FIRST_RING ) |
#endif
            BCHP_MASK(HSCL_0_DERINGING, HORIZ_CHROMA_DERINGING ) |
            BCHP_MASK(HSCL_0_DERINGING, HORIZ_LUMA_DERINGING ));

        BVDC_P_HSCL_GET_REG_DATA(HSCL_0_DERINGING) |=  (
#if (BVDC_P_SUPPORT_HSCL_VER >= BVDC_P_SUPPORT_HSCL_VER_6)
            BCHP_FIELD_ENUM(HSCL_0_DERINGING, HORIZ_CHROMA_PASS_FIRST_RING, ENABLE ) |
            BCHP_FIELD_ENUM(HSCL_0_DERINGING, HORIZ_LUMA_PASS_FIRST_RING, ENABLE ) |
#endif
            BCHP_FIELD_ENUM(HSCL_0_DERINGING, HORIZ_CHROMA_DERINGING, ON ) |
            BCHP_FIELD_ENUM(HSCL_0_DERINGING, HORIZ_LUMA_DERINGING,   ON ));
    }


    hHscaler->ulSrcHrzAlign  = 2;

    /* Initialize state. */
    hHscaler->bInitial = true;

    BDBG_LEAVE(BVDC_P_Hscaler_Init_isr);
    return;
}


/***************************************************************************
 * {private}
 *
 */
void BVDC_P_Hscaler_BuildRul_SrcInit_isr
    ( BVDC_P_Hscaler_Handle            hHscaler,
      BVDC_P_ListInfo                 *pList )
{
    BDBG_OBJECT_ASSERT(hHscaler, BVDC_HSL);

    if(hHscaler->ulUpdateAll)
    {
        hHscaler->ulUpdateAll--;
    /* optimize scaler mute RUL */
#if (BVDC_P_SUPPORT_HSCL_VER >= BVDC_P_SUPPORT_HSCL_VER_6)
        BDBG_CASSERT(6 == (((BCHP_HSCL_0_DEST_PIC_SIZE - BCHP_HSCL_0_HORIZ_CONTROL) / sizeof(uint32_t)) + 1));
        *pList->pulCurrent++ = BRDC_OP_IMMS_TO_REGS(((BCHP_HSCL_0_DEST_PIC_SIZE - BCHP_HSCL_0_HORIZ_CONTROL) / sizeof(uint32_t)) + 1);
        *pList->pulCurrent++ = BRDC_REGISTER(BCHP_HSCL_0_HORIZ_CONTROL + hHscaler->ulRegOffset);
        *pList->pulCurrent++ = BVDC_P_HSCL_GET_REG_DATA(HSCL_0_HORIZ_CONTROL);
        *pList->pulCurrent++ = BVDC_P_HSCL_GET_REG_DATA(HSCL_0_BVB_IN_SIZE);
        *pList->pulCurrent++ = BVDC_P_HSCL_GET_REG_DATA(HSCL_0_PIC_OFFSET);
        *pList->pulCurrent++ = BVDC_P_HSCL_GET_REG_DATA(HSCL_0_PIC_OFFSET_R);
        *pList->pulCurrent++ = BVDC_P_HSCL_GET_REG_DATA(HSCL_0_SRC_PIC_SIZE);
        *pList->pulCurrent++ = BVDC_P_HSCL_GET_REG_DATA(HSCL_0_DEST_PIC_SIZE);
#else
        BDBG_CASSERT(5 == (((BCHP_HSCL_0_DEST_PIC_SIZE - BCHP_HSCL_0_HORIZ_CONTROL) / sizeof(uint32_t)) + 1));
        *pList->pulCurrent++ = BRDC_OP_IMMS_TO_REGS(((BCHP_HSCL_0_DEST_PIC_SIZE - BCHP_HSCL_0_HORIZ_CONTROL) / sizeof(uint32_t)) + 1);
        *pList->pulCurrent++ = BRDC_REGISTER(BCHP_HSCL_0_HORIZ_CONTROL + hHscaler->ulRegOffset);
        *pList->pulCurrent++ = BVDC_P_HSCL_GET_REG_DATA(HSCL_0_HORIZ_CONTROL);
        *pList->pulCurrent++ = BVDC_P_HSCL_GET_REG_DATA(HSCL_0_BVB_IN_SIZE);
        *pList->pulCurrent++ = BVDC_P_HSCL_GET_REG_DATA(HSCL_0_PIC_OFFSET);
        *pList->pulCurrent++ = BVDC_P_HSCL_GET_REG_DATA(HSCL_0_SRC_PIC_SIZE);
        *pList->pulCurrent++ = BVDC_P_HSCL_GET_REG_DATA(HSCL_0_DEST_PIC_SIZE);
#endif

        BDBG_CASSERT(14 == (((BCHP_HSCL_0_HORIZ_DEST_PIC_REGION_3_END - BCHP_HSCL_0_SRC_PIC_HORIZ_PAN_SCAN) / sizeof(uint32_t)) + 1));
        *pList->pulCurrent++ = BRDC_OP_IMMS_TO_REGS(((BCHP_HSCL_0_HORIZ_DEST_PIC_REGION_3_END - BCHP_HSCL_0_SRC_PIC_HORIZ_PAN_SCAN) / sizeof(uint32_t)) + 1);
        *pList->pulCurrent++ = BRDC_REGISTER(BCHP_HSCL_0_SRC_PIC_HORIZ_PAN_SCAN + hHscaler->ulRegOffset);
        *pList->pulCurrent++ = BVDC_P_HSCL_GET_REG_DATA(HSCL_0_SRC_PIC_HORIZ_PAN_SCAN);
        *pList->pulCurrent++ = BVDC_P_HSCL_GET_REG_DATA(HSCL_0_HORIZ_FIR_LUMA_SRC_PIC_OFFSET);
        *pList->pulCurrent++ = BVDC_P_HSCL_GET_REG_DATA(HSCL_0_HORIZ_FIR_CHROMA_SRC_PIC_OFFSET);
        *pList->pulCurrent++ = BVDC_P_HSCL_GET_REG_DATA(HSCL_0_HORIZ_FIR_INIT_PHASE_ACC);
        *pList->pulCurrent++ = BVDC_P_HSCL_GET_REG_DATA(HSCL_0_HORIZ_FIR_INIT_PHASE_ACC_R);
        *pList->pulCurrent++ = BVDC_P_HSCL_GET_REG_DATA(HSCL_0_HORIZ_FIR_INIT_STEP_FRAC);
        *pList->pulCurrent++ = BVDC_P_HSCL_GET_REG_DATA(HSCL_0_HORIZ_FIR_INIT_STEP_INT);
        *pList->pulCurrent++ = BVDC_P_HSCL_GET_REG_DATA(HSCL_0_HORIZ_FIR_DEST_PIC_REGION_0_STEP_DELTA);
        *pList->pulCurrent++ = BVDC_P_HSCL_GET_REG_DATA(HSCL_0_HORIZ_FIR_DEST_PIC_REGION_2_STEP_DELTA);
        *pList->pulCurrent++ = BVDC_P_HSCL_GET_REG_DATA(HSCL_0_HORIZ_DEST_PIC_REGION_N1_END);
        *pList->pulCurrent++ = BVDC_P_HSCL_GET_REG_DATA(HSCL_0_HORIZ_DEST_PIC_REGION_0_END);
        *pList->pulCurrent++ = BVDC_P_HSCL_GET_REG_DATA(HSCL_0_HORIZ_DEST_PIC_REGION_1_END);
        *pList->pulCurrent++ = BVDC_P_HSCL_GET_REG_DATA(HSCL_0_HORIZ_DEST_PIC_REGION_2_END);
        *pList->pulCurrent++ = BVDC_P_HSCL_GET_REG_DATA(HSCL_0_HORIZ_DEST_PIC_REGION_3_END);

        BVDC_P_HSCL_WRITE_TO_RUL(HSCL_0_VIDEO_3D_MODE, pList->pulCurrent);

#ifdef BCHP_HSCL_0_DERINGING
        if(hHscaler->bDeringing)
        {
            BVDC_P_HSCL_WRITE_TO_RUL(HSCL_0_DERINGING, pList->pulCurrent);
        }
#endif

        BVDC_P_HSCL_BLOCK_WRITE_TO_RUL(HSCL_0_HORIZ_FIR_LUMA_COEFF_PHASE0_00_01,
            HSCL_0_HORIZ_FIR_CHROMA_COEFF_PHASE7_04_05, pList->pulCurrent);
        BVDC_P_HSCL_WRITE_TO_RUL(HSCL_0_TOP_CONTROL, pList->pulCurrent);
    }
}


/***************************************************************************
 * {private}
 *
 */
static void BVDC_P_Hscaler_SetFirCoeff_isr
    ( BVDC_P_Hscaler_Handle            hHscaler,
      const uint32_t                  *pulHorzFirCoeff )
{
    int i;

    BDBG_OBJECT_ASSERT(hHscaler, BVDC_HSL);
    /* write horiz entries into registers */
    for(i = 0; (pulHorzFirCoeff) && (*pulHorzFirCoeff != BVDC_P_HSCL_LAST); i++)
    {

        hHscaler->aulRegs[BVDC_P_HSCL_GET_REG_IDX(HSCL_0_HORIZ_FIR_LUMA_COEFF_PHASE0_00_01) + i] =
            *pulHorzFirCoeff;
        pulHorzFirCoeff++;
    }
}


/***************************************************************************
 * {private}
 *
 */
static void BVDC_P_Hscaler_SetChromaFirCoeff_isr
    ( BVDC_P_Hscaler_Handle            hHscaler,
      const uint32_t                  *pulHorzFirCoeff )
{
    int i;

    BDBG_OBJECT_ASSERT(hHscaler, BVDC_HSL);
    /* write 32 hor entries into registers */
    for(i = 0; (pulHorzFirCoeff) && (*pulHorzFirCoeff != BVDC_P_HSCL_LAST); i++)
    {
        hHscaler->aulRegs[BVDC_P_HSCL_GET_REG_IDX(HSCL_0_HORIZ_FIR_CHROMA_COEFF_PHASE0_00_01) + i] =
            *pulHorzFirCoeff;
        pulHorzFirCoeff++;
    }
}


/***************************************************************************
 * {private}
 *
 * The following notes illustrate the capabilities of the hscaler.  It
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
 * There is no hard-wired filter in HSCL, so Sx is the value that goes into
 * horizontal FIR ratio register.
 *
 * Sx must be [32.0, 0.125].
 *
 *
 * [[ Conclusion ]]
 *  With the above information the theoretical scaling capacities are:
 *
 *  Sx = 32:1 to 1:32
 */
void BVDC_P_Hscaler_SetInfo_isr
    ( BVDC_P_Hscaler_Handle            hHscaler,
      BVDC_Window_Handle               hWindow,
      const BVDC_P_PictureNodePtr      pPicture )
{
    uint32_t ulSrcHSize;               /* really scaled src width in pixel unit */
    uint32_t ulDstHSize;               /* Dst width in pixel unit */
    uint32_t ulDstVSize;               /* Dst height, in row unit */
    uint32_t ulAlgnSrcHSize;           /* src width into the 1st one of half band or FIR, pixel unit */
    uint32_t ulBvbInHSize;             /* input bvb width in pixel unit */
    uint32_t ulPicOffsetLeft = 0;      /* horizontal Pan/Scan part cut by PIC_OFFSET, pixel unit */
#if (BVDC_P_SUPPORT_HSCL_VER >= BVDC_P_SUPPORT_HSCL_VER_6)
    uint32_t ulPicOffsetLeft_R = 0;    /* horizontal Pan/Scan part cut by PIC_OFFSET_R, pixel unit */
#endif
    uint32_t ulPanScanLeft;            /* horizontal Pan/Scan vector in S11.6 format */
    const BVDC_P_FirCoeffTbl *pHorzFirCoeff;
    uint32_t ulNrmHrzStep;              /* Total horizontal src step per dest pixel, U11.21 */
    uint32_t ulFirHrzStep = 0;          /* FIR hrz src step per dest pixel, HW reg fmt, for coeff select */
    int32_t  lHrzPhsAccInit = 0;
    uint32_t ulMaxX;
    BVDC_P_Rect  *pHsclIn, *pHsclOut;
    bool     bHsclFirEnable;

    BDBG_ENTER(BVDC_P_Hscaler_SetInfo_isr);
    BDBG_OBJECT_ASSERT(hHscaler, BVDC_HSL);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);
    BDBG_OBJECT_ASSERT(hWindow->stCurInfo.hSource, BVDC_SRC);
    pHsclIn = pPicture->pHsclIn;
    pHsclOut = pPicture->pHsclOut;


    ulDstVSize = pHsclOut->ulHeight >> (BAVC_Polarity_eFrame!=pPicture->eSrcPolarity);
    /* any following info changed -> re-calculate SCL settings */
    if((hHscaler->ulPrevSrcWidth != pHsclIn->ulWidth) ||
       (hHscaler->lPrevHsclCutLeft != pPicture->stHsclCut.lLeft) ||
       (hHscaler->lPrevHsclCutLeft_R != pPicture->stHsclCut.lLeft_R) ||
       (hHscaler->ulPrevHsclCutWidth != pPicture->stHsclCut.ulWidth) ||
       (hHscaler->ulPrevOutWidth != pHsclOut->ulWidth) ||
       (hHscaler->ulPrevSrcHeight != ulDstVSize) ||  /* no vrt scl */
       (pPicture->eSrcOrientation != hHscaler->ePrevSrcOrientation)    ||
       (pPicture->eDispOrientation  != hHscaler->ePrevDispOrientation)   ||
       (hWindow->stCurInfo.stCtIndex.ulHsclHorzLuma != hHscaler->ulPrevCtIndexLuma) ||
       (hWindow->stCurInfo.stCtIndex.ulHsclHorzChroma != hHscaler->ulPrevCtIndexChroma) ||
       (hWindow->stCurInfo.hSource->stCurInfo.eCtInputType != hHscaler->ePrevCtInputType) ||
       !BVDC_P_HSCL_COMPARE_FIELD_DATA(HSCL_0_ENABLE, SCALER_ENABLE, 1) ||
       (hHscaler->bChannelChange != pPicture->bChannelChange)||
       (pPicture->bMosaicIntra))
    {
        /* for next "dirty" check */
        hHscaler->ulPrevSrcWidth = pHsclIn->ulWidth;
        hHscaler->lPrevHsclCutLeft = pPicture->stHsclCut.lLeft;
        hHscaler->lPrevHsclCutLeft_R = pPicture->stHsclCut.lLeft_R;
        hHscaler->ulPrevHsclCutWidth = pPicture->stHsclCut.ulWidth;
        hHscaler->ulPrevOutWidth = pHsclOut->ulWidth;
        hHscaler->ulPrevSrcHeight = pHsclIn->ulHeight >> (BAVC_Polarity_eFrame!=pPicture->eSrcPolarity);
        hHscaler->ulPrevCtIndexLuma = hWindow->stCurInfo.stCtIndex.ulHsclHorzLuma;
        hHscaler->ulPrevCtIndexChroma = hWindow->stCurInfo.stCtIndex.ulHsclHorzChroma;
        hHscaler->ePrevCtInputType = hWindow->stCurInfo.hSource->stCurInfo.eCtInputType;

        hHscaler->ePrevSrcOrientation  = pPicture->eSrcOrientation;
        hHscaler->ePrevDispOrientation = pPicture->eDispOrientation;
        hHscaler->bChannelChange = pPicture->bChannelChange;

        hHscaler->ulUpdateAll = BVDC_P_RUL_UPDATE_THRESHOLD;
        /* -----------------------------------------------------------------------
         * 1). Init some regitsers first, they might be modified later basing on
         * specific conditions
         */

        /* scaler panscan will be combined with init phase */
        BVDC_P_HSCL_GET_REG_DATA(HSCL_0_SRC_PIC_HORIZ_PAN_SCAN) &= ~(
            BCHP_MASK(HSCL_0_SRC_PIC_HORIZ_PAN_SCAN, OFFSET));

        /* Always re-enable after set info. */
        /* Horizontal scaler settings (and top control)!  Choose scaling order,
         * and how much to decimate data. */
#if (BVDC_P_SUPPORT_HSCL_VER >= BVDC_P_SUPPORT_HSCL_VER_7)
        BVDC_P_HSCL_GET_REG_DATA(HSCL_0_TOP_CONTROL) =
            BCHP_FIELD_ENUM(HSCL_0_TOP_CONTROL, ENABLE_CTRL,  ENABLE_BY_PICTURE);
#else
        BVDC_P_HSCL_GET_REG_DATA(HSCL_0_TOP_CONTROL) =  (
            BCHP_FIELD_ENUM(HSCL_0_TOP_CONTROL, UPDATE_SEL,   UPDATE_BY_PICTURE) |
            BCHP_FIELD_ENUM(HSCL_0_TOP_CONTROL, ENABLE_CTRL,  ENABLE_BY_PICTURE));
#endif


        BVDC_P_HSCL_GET_REG_DATA(HSCL_0_ENABLE) =
            BCHP_FIELD_ENUM(HSCL_0_ENABLE, SCALER_ENABLE, ON);

        BVDC_P_HSCL_GET_REG_DATA(HSCL_0_HORIZ_CONTROL) &= ~(
            BCHP_MASK(HSCL_0_HORIZ_CONTROL, FIR_ENABLE          ) |
            BCHP_MASK(HSCL_0_HORIZ_CONTROL, MASK_HSCL_LONG_LINE ) |
            BCHP_MASK(HSCL_0_HORIZ_CONTROL, MASK_HSCL_SHORT_LINE) |
            BCHP_MASK(HSCL_0_HORIZ_CONTROL, STALL_DRAIN_ENABLE  ));


        bHsclFirEnable =
            ((1 << BVDC_P_NRM_SRC_STEP_F_BITS) != pPicture->ulHsclNrmHrzSrcStep) ||
            (hWindow->stCurInfo.stCtIndex.ulHsclHorzLuma) ||
            (hWindow->stCurInfo.stCtIndex.ulHsclHorzChroma);
        BVDC_P_HSCL_GET_REG_DATA(HSCL_0_HORIZ_CONTROL) |=  (
            BCHP_FIELD_DATA(HSCL_0_HORIZ_CONTROL, FIR_ENABLE, bHsclFirEnable )|
            BCHP_FIELD_ENUM(HSCL_0_HORIZ_CONTROL, MASK_HSCL_LONG_LINE,  ON ) |
            BCHP_FIELD_ENUM(HSCL_0_HORIZ_CONTROL, MASK_HSCL_SHORT_LINE, ON ) |
            BCHP_FIELD_ENUM(HSCL_0_HORIZ_CONTROL, STALL_DRAIN_ENABLE,   OFF));

        BDBG_MODULE_MSG(BVDC_FIR_BYPASS, ("hscl[%d] HFIR_ENABLE %s", hHscaler->eId, bHsclFirEnable?"OFF":"ON"));

#if (BVDC_P_SUPPORT_HSCL_VER >= BVDC_P_SUPPORT_HSCL_VER_5)
        BVDC_P_HSCL_GET_REG_DATA(HSCL_0_VIDEO_3D_MODE) =
            BCHP_FIELD_DATA(HSCL_0_VIDEO_3D_MODE, BVB_VIDEO,
            BVDC_P_VNET_USED_MVP_AT_WRITER(pPicture->stVnetMode) ?
            pPicture->eSrcOrientation : pPicture->eDispOrientation);
#endif

        /* -----------------------------------------------------------------------
         * 2). Need to calculate the horizontal scaling factors before src width
         * alignment and init phase can be decided
         */

        /* output size */
        ulDstHSize = pHsclOut->ulWidth;

        BVDC_P_HSCL_GET_REG_DATA(HSCL_0_DEST_PIC_SIZE) &= ~(
            BCHP_MASK(HSCL_0_DEST_PIC_SIZE, HSIZE) |
            BCHP_MASK(HSCL_0_DEST_PIC_SIZE, VSIZE));
        BVDC_P_HSCL_GET_REG_DATA(HSCL_0_DEST_PIC_SIZE) |=  (
            BCHP_FIELD_DATA(HSCL_0_DEST_PIC_SIZE, HSIZE, ulDstHSize) |
            BCHP_FIELD_DATA(HSCL_0_DEST_PIC_SIZE, VSIZE, ulDstVSize));

        /* the src size really scaled and output */
        ulSrcHSize = pPicture->stHsclCut.ulWidth;

        /* pan scan:  left format S11.6, top format S11.14 */
        ulPanScanLeft = pPicture->stHsclCut.lLeft;

        /* separate the amount cut by HSCL_0_PIC_OFFSET and FIR_LUMA_SRC_PIC_OFFSET */
        ulPicOffsetLeft = (ulPanScanLeft >> BVDC_P_SCL_LEFT_PIC_OFFSET_F_BITS) & ~(hHscaler->ulSrcHrzAlign - 1);
        ulPanScanLeft -= (ulPicOffsetLeft << BVDC_P_SCL_LEFT_PIC_OFFSET_F_BITS);

#if (BVDC_P_SUPPORT_HSCL_VER >= BVDC_P_SUPPORT_HSCL_VER_6)
        ulPicOffsetLeft_R = (pPicture->stHsclCut.lLeft_R >> BVDC_P_SCL_LEFT_PIC_OFFSET_F_BITS) & ~(hHscaler->ulSrcHrzAlign - 1);
#endif
        /* the src size that get into the first scaler sub-modules (e.g. HW half-band
         * filter if it is scaled down a lot): it includes the FIR_LUMA_SRC_PIC_OFFSET,
         * but not the HSCL_0_PIC_OFFSET, it has to be rounded-up for alignment */
        ulMaxX = ulPanScanLeft + (ulSrcHSize << BVDC_P_SCL_LEFT_PIC_OFFSET_F_BITS);
        ulAlgnSrcHSize = ((ulMaxX + ((1<< BVDC_P_SCL_LEFT_PIC_OFFSET_F_BITS) - 1)) >>  BVDC_P_SCL_LEFT_PIC_OFFSET_F_BITS);
        ulAlgnSrcHSize = BVDC_P_ALIGN_UP(ulAlgnSrcHSize, hHscaler->ulSrcHrzAlign);

        /* Init the input/output horizontal size of FIRs */
        /*ulFirSrcHSize = ulSrcHSize;*/

        /* HSCL only do linear scaling */
        /*if(0 == pPicture->ulNonlinearSclOutWidth)*/
        {
            /* Horizantal step HW reg uses U5.17 in older arch, U5.26 after smooth non-linear is
             * suported. Since CPU uses 32 bits int, calc step with 26 bits frac needs special
             * handling (see the delta calcu in the case of nonlinear scaling). It is the step
             * delta and internal step accum reg, not the initial step value, that really need 26
             * frac bits, therefore we use 21 bits for trade off */
            ulNrmHrzStep = pPicture->ulHsclNrmHrzSrcStep;    /* U11.21 */
            ulFirHrzStep = /*ulHrzStep =*/ BVDC_P_SCL_H_STEP_NRM_TO_SPEC(ulNrmHrzStep); /* U4.17, U5.17, U5.26 */

            if (((1<<BVDC_P_NRM_SRC_STEP_F_BITS) == ulNrmHrzStep) &&
                (0 == ulPanScanLeft) && (ulSrcHSize == ulDstHSize))
            {
                /* unity scale and no phase shift, turn off FIR for accuracy */
                BVDC_P_HSCL_GET_REG_DATA(HSCL_0_HORIZ_CONTROL) &= ~(
                    BCHP_MASK(HSCL_0_HORIZ_CONTROL, FIR_ENABLE));
                BVDC_P_HSCL_GET_REG_DATA(HSCL_0_HORIZ_CONTROL) |=  (
                    BCHP_FIELD_ENUM(HSCL_0_HORIZ_CONTROL, FIR_ENABLE,  OFF));
            }
            /*ulFirHrzStepInit = ulFirHrzStep;*/

            /* set step size and region_0 end */
            BVDC_P_HSCL_SET_HORZ_RATIO(ulFirHrzStep);
            BVDC_P_HSCL_SET_HORZ_REGION02(0, ulDstHSize, 0)
        }

        /* -----------------------------------------------------------------------
         * 3). Now we can set src size, offset and bvb size
         */
        ulBvbInHSize = pHsclIn->ulWidth;

        /* in older chips, align ulBvbInHSize up if ulAlgnSrcHSize has been aligned
         * up due to half-band.
         * note: align ulBvbInHSize up might cause "short line" error, that is OK
         * and scl input module would patch. If we don't align up, SCL might hang */
        if (1 != hHscaler->ulSrcHrzAlign)
            ulBvbInHSize  = BVDC_P_MAX(ulBvbInHSize, ulAlgnSrcHSize + ulPicOffsetLeft);
        else
            ulAlgnSrcHSize = BVDC_P_MIN(ulAlgnSrcHSize, ulBvbInHSize - ulPicOffsetLeft);

        /* without this we might see flash when we move up with dst cut if MAD is disabled? */
        /*ulBvbInVSize  = BVDC_P_MAX(ulBvbInVSize, ulAlgnSrcVSize + ulPicOffsetTop);*/

        BVDC_P_HSCL_GET_REG_DATA(HSCL_0_PIC_OFFSET) &= ~(
            BCHP_MASK(HSCL_0_PIC_OFFSET, HSIZE) |
            BCHP_MASK(HSCL_0_PIC_OFFSET, VSIZE));
        BVDC_P_HSCL_GET_REG_DATA(HSCL_0_PIC_OFFSET) |=  (
            BCHP_FIELD_DATA(HSCL_0_PIC_OFFSET, HSIZE, ulPicOffsetLeft) |
            BCHP_FIELD_DATA(HSCL_0_PIC_OFFSET, VSIZE, 0));

#if (BVDC_P_SUPPORT_HSCL_VER >= BVDC_P_SUPPORT_HSCL_VER_6)
        BVDC_P_HSCL_GET_REG_DATA(HSCL_0_PIC_OFFSET_R) &= ~(
            BCHP_MASK(HSCL_0_PIC_OFFSET_R, ENABLE) |
            BCHP_MASK(HSCL_0_PIC_OFFSET_R, HSIZE));
        BVDC_P_HSCL_GET_REG_DATA(HSCL_0_PIC_OFFSET_R) |=  (
            BCHP_FIELD_DATA(HSCL_0_PIC_OFFSET_R, ENABLE, (ulPicOffsetLeft != ulPicOffsetLeft_R)) |
            BCHP_FIELD_DATA(HSCL_0_PIC_OFFSET_R, HSIZE, ulPicOffsetLeft_R));
#endif

        BVDC_P_HSCL_GET_REG_DATA(HSCL_0_BVB_IN_SIZE) &= ~(
            BCHP_MASK(HSCL_0_BVB_IN_SIZE, HSIZE) |
            BCHP_MASK(HSCL_0_BVB_IN_SIZE, VSIZE));
        BVDC_P_HSCL_GET_REG_DATA(HSCL_0_BVB_IN_SIZE) |=  (
            BCHP_FIELD_DATA(HSCL_0_BVB_IN_SIZE, HSIZE, ulBvbInHSize) |
            BCHP_FIELD_DATA(HSCL_0_BVB_IN_SIZE, VSIZE, ulDstVSize));

        /* SRC_PIC_SIZE should include FIR_LUMA_SRC_PIC_OFFSET and align */
        BVDC_P_HSCL_GET_REG_DATA(HSCL_0_SRC_PIC_SIZE) &= ~(
            BCHP_MASK(HSCL_0_SRC_PIC_SIZE, HSIZE) |
            BCHP_MASK(HSCL_0_SRC_PIC_SIZE, VSIZE));
        BVDC_P_HSCL_GET_REG_DATA(HSCL_0_SRC_PIC_SIZE) |=  (
            BCHP_FIELD_DATA(HSCL_0_SRC_PIC_SIZE, HSIZE, ulAlgnSrcHSize) |
            BCHP_FIELD_DATA(HSCL_0_SRC_PIC_SIZE, VSIZE, ulDstVSize));

        /* -----------------------------------------------------------------------
         * 4). set coeffs for both horizontal
         */
        pHorzFirCoeff = BVDC_P_SelectFirCoeff_isr(hHscaler->pHorzFirCoeffTbl,
            hWindow->stCurInfo.stCtIndex.ulHsclHorzLuma,
            hWindow->stCurInfo.hSource->stCurInfo.eCtInputType,
            BVDC_P_CtOutput_eAny,
            pPicture->PicComRulInfo.eSrcOrigPolarity, ulFirHrzStep,
            hHscaler->ulHorzTaps, ulSrcHSize, ulDstHSize);
        BDBG_MSG(("Luma H Coeffs  : %s", pHorzFirCoeff->pchName));
        BVDC_P_Hscaler_SetFirCoeff_isr(hHscaler, pHorzFirCoeff->pulCoeffs);

        pHorzFirCoeff = BVDC_P_SelectFirCoeff_isr(hHscaler->pChromaHorzFirCoeffTbl,
            hWindow->stCurInfo.stCtIndex.ulHsclHorzChroma,
            hWindow->stCurInfo.hSource->stCurInfo.eCtInputType,
            BVDC_P_CtOutput_eAny,
            pPicture->PicComRulInfo.eSrcOrigPolarity, ulFirHrzStep,
            hHscaler->ulHorzTaps, ulSrcHSize, ulDstHSize);
        BDBG_MSG(("Chroma H Coeffs: %s", pHorzFirCoeff->pchName));
        BVDC_P_Hscaler_SetChromaFirCoeff_isr(
            hHscaler, pHorzFirCoeff->pulCoeffs);

        /* -----------------------------------------------------------------------
         * 5). set init phase for horizontal
         */
        /* Compute the phase accumulate intial value in S11.6 or S5.26 */
        lHrzPhsAccInit = BVDC_P_FIXED_A_MINUS_FIXED_B(
            BVDC_P_H_INIT_PHASE_RATIO_ADJ(ulFirHrzStep) / 2,
            BVDC_P_H_INIT_PHASE_0_POINT_5);

        BVDC_P_HSCL_GET_REG_DATA(HSCL_0_HORIZ_FIR_LUMA_SRC_PIC_OFFSET) &= ~(
            BCHP_MASK(HSCL_0_HORIZ_FIR_LUMA_SRC_PIC_OFFSET, VALUE));
        BVDC_P_HSCL_GET_REG_DATA(HSCL_0_HORIZ_FIR_CHROMA_SRC_PIC_OFFSET) &= ~(
            BCHP_MASK(HSCL_0_HORIZ_FIR_CHROMA_SRC_PIC_OFFSET, VALUE));
        BVDC_P_HSCL_GET_REG_DATA(HSCL_0_HORIZ_FIR_LUMA_SRC_PIC_OFFSET) |=  (
            BCHP_FIELD_DATA(HSCL_0_HORIZ_FIR_LUMA_SRC_PIC_OFFSET, VALUE,
            ulPanScanLeft));
        BVDC_P_HSCL_GET_REG_DATA(HSCL_0_HORIZ_FIR_CHROMA_SRC_PIC_OFFSET) |=  (
            BCHP_FIELD_DATA(HSCL_0_HORIZ_FIR_CHROMA_SRC_PIC_OFFSET, VALUE,
            ulPanScanLeft));

        BVDC_P_HSCL_GET_REG_DATA(HSCL_0_HORIZ_FIR_INIT_PHASE_ACC) &= ~(
            BCHP_MASK(HSCL_0_HORIZ_FIR_INIT_PHASE_ACC, SIZE));
        BVDC_P_HSCL_GET_REG_DATA(HSCL_0_HORIZ_FIR_INIT_PHASE_ACC) |=  (
            BCHP_FIELD_DATA(HSCL_0_HORIZ_FIR_INIT_PHASE_ACC, SIZE,
            lHrzPhsAccInit));

        BVDC_P_HSCL_GET_REG_DATA(HSCL_0_HORIZ_FIR_INIT_PHASE_ACC_R) &= ~(
            BCHP_MASK(HSCL_0_HORIZ_FIR_INIT_PHASE_ACC_R, SIZE));
        BVDC_P_HSCL_GET_REG_DATA(HSCL_0_HORIZ_FIR_INIT_PHASE_ACC_R) |=  (
            BCHP_FIELD_DATA(HSCL_0_HORIZ_FIR_INIT_PHASE_ACC_R, SIZE,
            lHrzPhsAccInit));
    }

    /* Printing out ratio in float format would be nice, but PI
     * code does permit float. */
    if(BVDC_P_RUL_UPDATE_THRESHOLD == hHscaler->ulUpdateAll)
    {
        BDBG_MSG(("-------------------------"));
        BDBG_MSG(("Hscaler[%d]         : %dx%d to %dx%d", hHscaler->eId,
            pHsclIn->ulWidth,  pHsclIn->ulHeight,
            pHsclOut->ulWidth, pHsclOut->ulHeight));
        BDBG_MSG(("Hscaler[%d]'clip    : left %d, width %d, left_R %d",
                hHscaler->eId, pPicture->stHsclCut.lLeft, pPicture->stHsclCut.ulWidth,
                pPicture->stHsclCut.lLeft_R));
        BDBG_MSG(("ulFirHrzStep      : %-8x", ulFirHrzStep));
        BDBG_MSG(("PIC_OFFSET_LEFT   : %-8x", ulPicOffsetLeft));
        BDBG_MSG(("H_InitPhase       : %-8x", lHrzPhsAccInit));

    }

    BDBG_LEAVE(BVDC_P_Hscaler_SetInfo_isr);
    return;
}

/***************************************************************************
 * {private}
 *
 */
void BVDC_P_Hscaler_SetEnable_isr
    ( BVDC_P_Hscaler_Handle            hHscaler,
      bool                             bEnable,
      BVDC_P_ListInfo                  *pList)
{
    BDBG_OBJECT_ASSERT(hHscaler, BVDC_HSL);

    /* Turn on/off the scaler. */
    BVDC_P_HSCL_GET_REG_DATA(HSCL_0_ENABLE) &= ~(
        BCHP_MASK(HSCL_0_ENABLE, SCALER_ENABLE));

    BVDC_P_HSCL_GET_REG_DATA(HSCL_0_ENABLE) |=  (bEnable
        ? BCHP_FIELD_ENUM(HSCL_0_ENABLE, SCALER_ENABLE, ON)
        : BCHP_FIELD_ENUM(HSCL_0_ENABLE, SCALER_ENABLE, OFF));

    BVDC_P_HSCL_WRITE_TO_RUL(HSCL_0_ENABLE, pList->pulCurrent);

    return;
}

#else
BERR_Code BVDC_P_Hscaler_Create
    ( BVDC_P_Hscaler_Handle            *phHscaler,
      BVDC_P_HscalerId                  eHscalerId,
      BVDC_P_Resource_Handle            hResource,
      BREG_Handle                       hReg
)
{
    BSTD_UNUSED(phHscaler);
    BSTD_UNUSED(eHscalerId);
    BSTD_UNUSED(hResource);
    BSTD_UNUSED(hReg);
    return BERR_SUCCESS;
}

void BVDC_P_Hscaler_Init_isr
    ( BVDC_P_Hscaler_Handle            hHscaler )
{
    BSTD_UNUSED(hHscaler);
}

void BVDC_P_Hscaler_BuildRul_SrcInit_isr
    ( BVDC_P_Hscaler_Handle            hHscaler,
      BVDC_P_ListInfo                 *pList )
{
    BSTD_UNUSED(hHscaler);
    BSTD_UNUSED(pList);
}

void BVDC_P_Hscaler_SetInfo_isr
    ( BVDC_P_Hscaler_Handle            hHscaler,
      BVDC_Window_Handle               hWindow,
      const BVDC_P_PictureNodePtr      pPicture )
{
    BSTD_UNUSED(hHscaler);
    BSTD_UNUSED(hWindow);
    BSTD_UNUSED(pPicture);

}

void BVDC_P_Hscaler_Destroy
    ( BVDC_P_Hscaler_Handle             hHscaler )
{
    BSTD_UNUSED(hHscaler);
}

void BVDC_P_Hscaler_SetEnable_isr
    ( BVDC_P_Hscaler_Handle            hHscaler,
      bool                             bEnable,
      BVDC_P_ListInfo                  *pList)
{
    BSTD_UNUSED(hHscaler);
    BSTD_UNUSED(bEnable);
    BSTD_UNUSED(pList);
}
#endif
/* End of file. */
