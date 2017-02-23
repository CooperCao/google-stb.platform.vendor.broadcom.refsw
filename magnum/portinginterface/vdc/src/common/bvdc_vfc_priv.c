/***************************************************************************
 * Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#include "bvdc_vfc_priv.h"

#if (BVDC_P_SUPPORT_VFC)
#include "bchp_mmisc.h"
#include "bvdc_source_priv.h"
#include "bvdc_window_priv.h"
#include "bvdc_vnet_priv.h"
#include "bvdc_compositor_priv.h"


BDBG_MODULE(BVDC_VFC);
BDBG_FILE_MODULE(BVDC_DITHER);
BDBG_OBJECT_ID(BVDC_VFC);


#define BVDC_P_MAKE_VFC(pVfc, id, channel_init)                                 \
{                                                                               \
    (pVfc)->ulResetRegAddr  = BCHP_MMISC_SW_INIT;                               \
    (pVfc)->ulResetMask     = BCHP_MMISC_SW_INIT_VFC_##id##_MASK;               \
    (pVfc)->ulVnetResetAddr = BCHP_##channel_init;                              \
    (pVfc)->ulVnetResetMask = BCHP_##channel_init##_VFC_##id##_MASK;            \
    (pVfc)->ulVnetMuxAddr   = BCHP_VNET_F_VFC_##id##_SRC;                       \
    (pVfc)->ulVnetMuxValue  = BCHP_VNET_B_CAP_0_SRC_SOURCE_VFC_##id;            \
    (pVfc)->ulRegOffset     = BCHP_VFC_##id##_REG_START - BCHP_VFC_0_REG_START; \
}

/***************************************************************************
 * {private}
 *
 */
BERR_Code BVDC_P_Vfc_Create
    ( BVDC_P_Vfc_Handle             *phVfc,
      BVDC_P_VfcId                   eVfcId,
      BVDC_P_Resource_Handle         hResource,
      BREG_Handle                    hReg
)
{
    BVDC_P_VfcContext *pVfc;

    BDBG_ENTER(BVDC_P_Vfc_Create);

    BDBG_ASSERT(phVfc);

    /* Use: to see messages */
    /* BDBG_SetModuleLevel("BVDC_VFC", BDBG_eMsg); */

    /* The handle will be NULL if create fails. */
    *phVfc = NULL;

    /* Alloc the context. */
    pVfc = (BVDC_P_VfcContext*)
        (BKNI_Malloc(sizeof(BVDC_P_VfcContext)));
    if(!pVfc)
    {
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }

    /* Clear out the context and set defaults. */
    BKNI_Memset((void*)pVfc, 0x0, sizeof(BVDC_P_VfcContext));
    BDBG_OBJECT_SET(pVfc, BVDC_VFC);

    pVfc->eId          = eVfcId;
    pVfc->hReg         = hReg;
    pVfc->hWindow      = NULL;

    switch(eVfcId)
    {
        case BVDC_P_VfcId_eVfc0:
#ifdef BCHP_MMISC_VNET_B_CHANNEL_SW_INIT_VFC_0_MASK
            BVDC_P_MAKE_VFC(pVfc, 0, MMISC_VNET_B_CHANNEL_SW_INIT);
#elif BCHP_MMISC_VNET_B_CHANNEL_SW_INIT_1_VFC_0_MASK
            BVDC_P_MAKE_VFC(pVfc, 0, MMISC_VNET_B_CHANNEL_SW_INIT_1);
#endif
            break;
        case BVDC_P_VfcId_eVfc1:
#ifdef BCHP_MMISC_VNET_B_CHANNEL_SW_INIT_VFC_1_MASK
            BVDC_P_MAKE_VFC(pVfc, 1, MMISC_VNET_B_CHANNEL_SW_INIT);
#elif BCHP_MMISC_VNET_B_CHANNEL_SW_INIT_1_VFC_1_MASK
            BVDC_P_MAKE_VFC(pVfc, 1, MMISC_VNET_B_CHANNEL_SW_INIT_1);
#endif
            break;
        case BVDC_P_VfcId_eVfc2:
#ifdef BCHP_MMISC_VNET_B_CHANNEL_SW_INIT_VFC_2_MASK
            BVDC_P_MAKE_VFC(pVfc, 2, MMISC_VNET_B_CHANNEL_SW_INIT);
#elif BCHP_MMISC_VNET_B_CHANNEL_SW_INIT_1_VFC_2_MASK
            BVDC_P_MAKE_VFC(pVfc, 2, MMISC_VNET_B_CHANNEL_SW_INIT_1);
#endif
            break;
        case BVDC_P_VfcId_eUnknown:
        default:
            BDBG_ERR(("Need to handle BVDC_P_Vfc_eVfc%d", eVfcId));
            BDBG_ASSERT(0);
    }

    /* init the SubRul sub-module */
    BVDC_P_SubRul_Init(&(pVfc->SubRul), pVfc->ulVnetMuxAddr,
        pVfc->ulVnetMuxValue, BVDC_P_DrainMode_eBack, 0, hResource);

    /* All done. now return the new fresh context to user. */
    *phVfc = (BVDC_P_Vfc_Handle)pVfc;

    BDBG_LEAVE(BVDC_P_Vfc_Create);
    return BERR_SUCCESS;
}


/***************************************************************************
 * {private}
 *
 */
void BVDC_P_Vfc_Destroy
    ( BVDC_P_Vfc_Handle             hVfc )
{
    BDBG_ENTER(BVDC_P_Vfc_Destroy);
    BDBG_OBJECT_ASSERT(hVfc, BVDC_VFC);

    BDBG_OBJECT_DESTROY(hVfc, BVDC_VFC);
    /* Release context in system memory */
    BKNI_Free((void*)hVfc);

    BDBG_LEAVE(BVDC_P_Vfc_Destroy);
    return;
}


/***************************************************************************
 * {private}
 *
 */
void BVDC_P_Vfc_Init_isr
    ( BVDC_P_Vfc_Handle             hVfc )
{
    BDBG_ENTER(BVDC_P_Vfc_Init_isr);
    BDBG_OBJECT_ASSERT(hVfc, BVDC_VFC);

    hVfc->ulUpdateAll = BVDC_P_RUL_UPDATE_THRESHOLD;

    /* Clear out shadow registers. */
    BKNI_Memset((void*)hVfc->aulRegs, 0x0, sizeof(hVfc->aulRegs));

    hVfc->ulPrevWidth = 0xffffffff;

    /* Initialize state. */
    hVfc->bInitial = true;

    BDBG_LEAVE(BVDC_P_Vfc_Init_isr);
    return;
}


/***************************************************************************
 * {private}
 *
 */
void BVDC_P_Vfc_BuildRul_SetEnable_isr
    ( BVDC_P_Vfc_Handle             hVfc,
      BVDC_P_ListInfo              *pList )
{
    BDBG_OBJECT_ASSERT(hVfc, BVDC_VFC);

    /* Add scaler registers to RUL using block write */
    if(hVfc->ulUpdateAll)
    {
        hVfc->ulUpdateAll--;
        /* optimize scaler mute RUL */

        BVDC_P_VFC_WRITE_TO_RUL(
            hVfc, VFC_0_VIDEO_3D_MODE, pList->pulCurrent);
        BVDC_P_VFC_WRITE_TO_RUL(
            hVfc, VFC_0_PIC_SIZE, pList->pulCurrent);

        BVDC_P_VFC_BLOCK_WRITE_TO_RUL(
            hVfc, VFC_0_BYPASS_DITHER_422_CTRL, VFC_0_BYPASS_DITHER_LFSR_CTRL,
            pList->pulCurrent);

        BVDC_P_VFC_WRITE_TO_RUL(
            hVfc, VFC_0_OUT_MUX_CTRL, pList->pulCurrent);
    }

    BVDC_P_VFC_WRITE_TO_RUL(
        hVfc, VFC_0_ENABLE, pList->pulCurrent);
}



/***************************************************************************
 * {private}
 *
 * BVDC_P_Vfc_AcquireConnect_isr
 *
 * It is called by BVDC_Window_Validate after changing from diable mad to
 * enable mad, .
 */
BERR_Code BVDC_P_Vfc_AcquireConnect_isr
    ( BVDC_P_Vfc_Handle             hVfc,
      BVDC_Window_Handle            hWindow )
{
    BERR_Code  eResult = BERR_SUCCESS;

    BDBG_ENTER(BVDC_P_Vfc_AcquireConnect_isr);

    hVfc->hWindow = hWindow;

    /* init Vfc */
    BVDC_P_Vfc_Init_isr(hVfc);

    BDBG_LEAVE(BVDC_P_Vfc_AcquireConnect_isr);
    return BERR_TRACE(eResult);
}

/***************************************************************************
 * {private}
 *
 * BVDC_P_Vfc_ReleaseConnect_isr
 *
 * It is called after window decided that mad is no-longer used by HW in its
 * vnet mode (i.e. it is really shut down and teared off from vnet).
 */
BERR_Code BVDC_P_Vfc_ReleaseConnect_isr
    ( BVDC_P_Vfc_Handle        *phVfc )
{
    BERR_Code  eResult = BERR_SUCCESS;

    BDBG_ENTER(BVDC_P_Vfc_ReleaseConnect_isr);

    /* handle validation */
    if (NULL != *phVfc)
    {
        BDBG_OBJECT_ASSERT(*phVfc, BVDC_VFC);

        BVDC_P_Resource_ReleaseHandle_isr(
            BVDC_P_SubRul_GetResourceHandle_isr(&(*phVfc)->SubRul),
            BVDC_P_ResourceType_eVfc, (void *)(*phVfc));

        /* this makes win to stop calling Vfc code */
        *phVfc = NULL;
    }

    BDBG_LEAVE(BVDC_P_Vfc_ReleaseConnect_isr);
    return BERR_TRACE(eResult);
}

/***************************************************************************
 * {private}
 *
 * BVDC_P_Vfc_BuildRul_DrainVnet_isr
 *
 * called by BVDC_P_Vfc_BuildRul_isr after resetting to drain the module and
 * its pre-patch FreeCh or LpBack
 */
static void BVDC_P_Vfc_BuildRul_DrainVnet_isr
    ( BVDC_P_Vfc_Handle           hVfc,
      BVDC_P_ListInfo            *pList,
      bool                        bNoCoreReset)
{
    BDBG_OBJECT_ASSERT(hVfc, BVDC_VFC);
    /* reset sub and connect the module to a drain */
    BVDC_P_SubRul_Drain_isr(&(hVfc->SubRul), pList,
        bNoCoreReset?0:hVfc->ulResetRegAddr,
        bNoCoreReset?0:hVfc->ulResetMask,
        bNoCoreReset?0:hVfc->ulVnetResetAddr,
        bNoCoreReset?0:hVfc->ulVnetResetMask);
}

/***************************************************************************
 * {private}
 *
 */
void BVDC_P_Vfc_BuildRul_isr
    ( const BVDC_P_Vfc_Handle       hVfc,
      BVDC_P_ListInfo              *pList,
      BVDC_P_State                  eVnetState,
      BVDC_P_PicComRulInfo         *pPicComRulInfo )
{
    uint32_t  ulRulOpsFlags;

    BDBG_ENTER(BVDC_P_Vfc_BuildRul_isr);
    BDBG_OBJECT_ASSERT(hVfc, BVDC_VFC);

    /* currently this is only for vnet building / tearing-off*/

    ulRulOpsFlags = BVDC_P_SubRul_GetOps_isr(
        &(hVfc->SubRul), pPicComRulInfo->eWin, eVnetState, pList->bLastExecuted);

    if ((0 == ulRulOpsFlags) ||
        (ulRulOpsFlags & BVDC_P_RulOp_eReleaseHandle))
        return;
    else if (ulRulOpsFlags & BVDC_P_RulOp_eDisable)
    {
        BVDC_P_SubRul_DropOffVnet_isr(&(hVfc->SubRul), pList);
        BVDC_P_Vfc_SetEnable_isr(hVfc, false);
        BVDC_P_VFC_WRITE_TO_RUL(
            hVfc, VFC_0_ENABLE, pList->pulCurrent);
    }

    /* If rul failed to execute last time we'd re reprogrammed possible
     * missing registers. */
    if((!pList->bLastExecuted)|| (hVfc->bInitial))
    {
        hVfc->ulUpdateAll = BVDC_P_RUL_UPDATE_THRESHOLD;
    }

    /* reset */
    if(hVfc->bInitial)
    {
        hVfc->bInitial = false;
    }

    if (ulRulOpsFlags & BVDC_P_RulOp_eEnable)
    {
        BVDC_P_Vfc_BuildRul_SetEnable_isr(hVfc, pList);

        /* join in vnet after enable. note: its src mux is initialed as disabled */
        if (ulRulOpsFlags & BVDC_P_RulOp_eVnetInit)
        {
            BVDC_P_SubRul_JoinInVnet_isr(&(hVfc->SubRul), pList);
        }
    }

    if (ulRulOpsFlags & BVDC_P_RulOp_eDrainVnet)
    {
        BVDC_P_Vfc_BuildRul_DrainVnet_isr(hVfc, pList, pPicComRulInfo->bNoCoreReset);
    }

    BDBG_LEAVE(BVDC_P_Vfc_BuildRul_isr);
    return;
}


/***************************************************************************
 * {private}
 *
 */
void BVDC_P_Vfc_SetInfo_isr
    ( BVDC_P_Vfc_Handle             hVfc,
      BVDC_Window_Handle            hWindow,
      const BVDC_P_PictureNodePtr   pPicture )
{
    uint32_t ulDstVSize;               /* Dst height, in row unit */
    uint32_t ulOutMuxCtrl;

    BDBG_ENTER(BVDC_P_Vfc_SetInfo_isr);
    BDBG_OBJECT_ASSERT(hVfc, BVDC_VFC);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    ulDstVSize = pPicture->pVfcIn->ulHeight >> (BAVC_Polarity_eFrame!=pPicture->eSrcPolarity);
    if((hVfc->ulPrevWidth  != pPicture->pVfcIn->ulWidth) ||
       (hVfc->ulPrevHeight != ulDstVSize) ||  /* no vrt scl */
       (pPicture->eOrigSrcOrientation != hVfc->ePrevSrcOrientation)    ||
       (pPicture->eDispOrientation    != hVfc->ePrevDispOrientation)   ||
       (pPicture->bSrc10Bit != hVfc->bPrevSrc10Bit) ||
       (hWindow->hCompositor->hDisplay->stCurInfo.bEnableStg != hVfc->bPrevEnableStg) ||
       !BVDC_P_VFC_COMPARE_FIELD_DATA(hVfc, VFC_0_ENABLE, ENABLE, 1))
    {
        /* for next "dirty" check */
        hVfc->ulPrevWidth = pPicture->pVfcIn->ulWidth;
        hVfc->ulPrevHeight = ulDstVSize;
        hVfc->ePrevSrcOrientation  = pPicture->eOrigSrcOrientation;
        hVfc->ePrevDispOrientation = pPicture->eDispOrientation;
        hVfc->bPrevSrc10Bit = pPicture->bSrc10Bit;
        hVfc->bPrevEnableStg = hWindow->hCompositor->hDisplay->stCurInfo.bEnableStg;
        hVfc->ulUpdateAll = BVDC_P_RUL_UPDATE_THRESHOLD;

        BVDC_P_VFC_GET_REG_DATA(hVfc, VFC_0_ENABLE) =
            BCHP_FIELD_ENUM(VFC_0_ENABLE, ENABLE, ENABLE);

        BVDC_P_VFC_GET_REG_DATA(hVfc, VFC_0_VIDEO_3D_MODE) =
            BCHP_FIELD_DATA(VFC_0_VIDEO_3D_MODE, BVB_VIDEO,
            BVDC_P_VNET_USED_VFC_AT_WRITER(pPicture->stVnetMode) ?
            pPicture->eOrigSrcOrientation : pPicture->eDispOrientation);

        BVDC_P_VFC_GET_REG_DATA(hVfc, VFC_0_PIC_SIZE) =  (
            BCHP_FIELD_DATA(VFC_0_PIC_SIZE, HSIZE, pPicture->pVfcIn->ulWidth) |
            BCHP_FIELD_DATA(VFC_0_PIC_SIZE, VSIZE, ulDstVSize));

        ulOutMuxCtrl = hWindow->hCompositor->bIs10BitCore ?
            BCHP_FIELD_ENUM(VFC_0_OUT_MUX_CTRL, OUT_MUX_CTRL, BYPASS_HDR) :
            BCHP_FIELD_ENUM(VFC_0_OUT_MUX_CTRL, OUT_MUX_CTRL, BYPASS_HDR_10BIT_TO_8BIT);
        BVDC_P_VFC_GET_REG_DATA(hVfc, VFC_0_OUT_MUX_CTRL) =  (
            BCHP_FIELD_ENUM(VFC_0_OUT_MUX_CTRL, VFC_BYPASS,   DISABLE) |
            ulOutMuxCtrl);

        /* only enable dithering if VFC is connected to a 8-bit non transcode */
        /* window path from a 10-bit source */
        if(!hWindow->bIs10BitCore)
        {
            bool bDitherEn =
                (pPicture->bSrc10Bit || !hWindow->hCompositor->hDisplay->stCurInfo.bEnableStg) ?
                true : false;
            BDBG_MODULE_MSG(BVDC_DITHER,("VFC%d DITHER: %s", hVfc->eId,
                (bDitherEn) ? "ENABLE" : "DISABLE"));

            BVDC_P_Dither_Setting_isr(&hVfc->stDither, bDitherEn, 0xFFC, 0x1);

            BVDC_P_VFC_GET_REG_DATA(hVfc, VFC_0_BYPASS_DITHER_422_CTRL) = hVfc->stDither.ulCtrlReg;
            if(bDitherEn)
            {
                BVDC_P_VFC_GET_REG_DATA(hVfc, VFC_0_BYPASS_DITHER_LFSR_INIT) = hVfc->stDither.ulLfsrInitReg;
                BVDC_P_VFC_GET_REG_DATA(hVfc, VFC_0_BYPASS_DITHER_LFSR_CTRL) = hVfc->stDither.ulLfsrCtrlReg;
            }
        }
    }
    BDBG_LEAVE(BVDC_P_Vfc_SetInfo_isr);
    return;
}

/***************************************************************************
 * {private}
 *
 */
void BVDC_P_Vfc_SetEnable_isr
    ( BVDC_P_Vfc_Handle             hVfc,
      bool                          bEnable )
{
    BDBG_OBJECT_ASSERT(hVfc, BVDC_VFC);

    if(!BVDC_P_VFC_COMPARE_FIELD_DATA(
        hVfc, VFC_0_ENABLE, ENABLE, bEnable))
    {
        hVfc->ulUpdateAll = BVDC_P_RUL_UPDATE_THRESHOLD;
    }

    /* Turn on/off the scaler. */
    BVDC_P_VFC_GET_REG_DATA(hVfc, VFC_0_ENABLE) &= ~(
        BCHP_MASK(VFC_0_ENABLE, ENABLE));

    BVDC_P_VFC_GET_REG_DATA(hVfc, VFC_0_ENABLE) |=  (bEnable
        ? BCHP_FIELD_ENUM(VFC_0_ENABLE, ENABLE, ENABLE)
        : BCHP_FIELD_ENUM(VFC_0_ENABLE, ENABLE, DISABLE));

    return;
}
#else
BERR_Code BVDC_P_Vfc_Create
    ( BVDC_P_Vfc_Handle            *phVfc,
      BVDC_P_VfcId                  eVfcId,
      BVDC_P_Resource_Handle        hResource,
      BREG_Handle                   hReg )
{
    BSTD_UNUSED(phVfc);
    BSTD_UNUSED(eVfcId);
    BSTD_UNUSED(hResource);
    BSTD_UNUSED(hReg);
    return BERR_SUCCESS;
}

void BVDC_P_Vfc_Destroy
    ( BVDC_P_Vfc_Handle             hVfc )
{
    BSTD_UNUSED(hVfc);
    return;
}

void BVDC_P_Vfc_Init_isr
    ( BVDC_P_Vfc_Handle             hVfc )
{
    BSTD_UNUSED(hVfc);
    return;
}

void BVDC_P_Vfc_BuildRul_SetEnable_isr
    ( BVDC_P_Vfc_Handle             hVfc,
      BVDC_P_ListInfo              *pList )
{
    BSTD_UNUSED(hVfc);
    BSTD_UNUSED(pList);
}

void BVDC_P_Vfc_SetInfo_isr
    ( BVDC_P_Vfc_Handle             hVfc,
      BVDC_Window_Handle            hWindow,
      const BVDC_P_PictureNodePtr   pPicture )
{
    BSTD_UNUSED(hVfc);
    BSTD_UNUSED(hWindow);
    BSTD_UNUSED(pPicture);
    return;
}

void BVDC_P_Vfc_SetEnable_isr
    ( BVDC_P_Vfc_Handle             hVfc,
      bool                          bEnable )
{
    BSTD_UNUSED(hVfc);
    BSTD_UNUSED(bEnable);
    return;
}
#endif  /* #if (BVDC_P_SUPPORT_VFC) */

#if !(BVDC_P_SUPPORT_VFC)

BERR_Code BVDC_P_Vfc_AcquireConnect_isr
    ( BVDC_P_Vfc_Handle             hVfc,
      BVDC_Window_Handle            hWindow )
{
    BSTD_UNUSED(hVfc);
    BSTD_UNUSED(hWindow);
    return BERR_SUCCESS;
}

BERR_Code BVDC_P_Vfc_ReleaseConnect_isr
    ( BVDC_P_Vfc_Handle            *phVfc )
{
    BSTD_UNUSED(phVfc);
    return BERR_SUCCESS;
}

void BVDC_P_Vfc_BuildRul_isr
    ( const BVDC_P_Vfc_Handle       hVfc,
      BVDC_P_ListInfo              *pList,
      BVDC_P_State                  eVnetState,
      BVDC_P_PicComRulInfo         *pPicComRulInfo )
{
    BSTD_UNUSED(hVfc);
    BSTD_UNUSED(pList);
    BSTD_UNUSED(eVnetState);
    BSTD_UNUSED(pPicComRulInfo);
    return;
}

#endif /* #if !(BVDC_P_SUPPORT_VFC) */

/* End of file. */
