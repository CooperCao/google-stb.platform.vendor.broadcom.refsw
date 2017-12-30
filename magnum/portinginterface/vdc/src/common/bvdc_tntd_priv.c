/***************************************************************************
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
 ***************************************************************************/
#include "bstd.h"
#include "bkni.h"
#include "bvdc.h"
#include "brdc.h"
#include "bvdc_source_priv.h"

#if (BVDC_P_SUPPORT_TNTD)
#include "bvdc_tntd_priv.h"
#include "bvdc_vnet_priv.h"
#include "bvdc_window_priv.h"
#include "bchp_mmisc.h"


BDBG_MODULE(BVDC_TNTD);
BDBG_OBJECT_ID(BVDC_TNTD);

#define BVDC_P_MAKE_TNTD(pTntd, id, channel_init)                                 \
{                                                                                 \
    (pTntd)->ulResetRegAddr  = BCHP_MMISC_SW_INIT;                                \
    (pTntd)->ulResetMask     = BCHP_MMISC_SW_INIT_TNTD_##id##_MASK;               \
    (pTntd)->ulVnetResetAddr = BCHP_##channel_init;                               \
    (pTntd)->ulVnetResetMask = BCHP_##channel_init##_TNTD_##id##_MASK;            \
    (pTntd)->ulVnetMuxAddr   = BCHP_VNET_F_TNTD_##id##_SRC;                       \
    (pTntd)->ulVnetMuxValue  = BCHP_VNET_B_CAP_0_SRC_SOURCE_TNTD_##id;            \
}


/***************************************************************************
 * {private}
 *
 */
BERR_Code BVDC_P_Tntd_Create
    ( BVDC_P_Tntd_Handle            *phTntd,
      BVDC_P_TntdId                  eTntdId,
      BVDC_P_Resource_Handle         hResource,
      BREG_Handle                    hReg
)
{
    BVDC_P_TntdContext *pTntd;

    BDBG_ENTER(BVDC_P_Tntd_Create);

    BDBG_ASSERT(phTntd);

    /* Use: to see messages */
    /* BDBG_SetModuleLevel("BVDC_TNTD", BDBG_eMsg); */

    /* The handle will be NULL if create fails. */
    *phTntd = NULL;

    /* Alloc the context. */
    pTntd = (BVDC_P_TntdContext*)
        (BKNI_Malloc(sizeof(BVDC_P_TntdContext)));
    if(!pTntd)
    {
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }

    /* Clear out the context and set defaults. */
    BKNI_Memset((void*)pTntd, 0x0, sizeof(BVDC_P_TntdContext));
    BDBG_OBJECT_SET(pTntd, BVDC_TNTD);

    pTntd->eId          = eTntdId;
    pTntd->hReg         = hReg;
    pTntd->ulRegOffset  = 0;

    /* Tntd reset address */
    switch(eTntdId)
    {
        case BVDC_P_TntdId_eTntd0:
            BVDC_P_MAKE_TNTD(pTntd, 0, MMISC_VNET_B_CHANNEL_SW_INIT_1);
            break;
        case BVDC_P_TntdId_eUnknown:
        default:
            BDBG_ERR(("Need to handle BVDC_P_Tntd_eTntd%d", eTntdId));
            BDBG_ASSERT(0);
    }

    /* init the SubRul sub-module */
    BVDC_P_SubRul_Init(&(pTntd->SubRul), pTntd->ulVnetMuxAddr,
        pTntd->ulVnetMuxValue, BVDC_P_DrainMode_eBack, 0, hResource);

    /* All done. now return the new fresh context to user. */
    *phTntd = (BVDC_P_Tntd_Handle)pTntd;

    BDBG_LEAVE(BVDC_P_Tntd_Create);
    return BERR_SUCCESS;
}


/***************************************************************************
 * {private}
 *
 */
void BVDC_P_Tntd_Destroy
    ( BVDC_P_Tntd_Handle             hTntd )
{
    BDBG_ENTER(BVDC_P_Tntd_Destroy);
    BDBG_OBJECT_ASSERT(hTntd, BVDC_TNTD);

    BDBG_OBJECT_DESTROY(hTntd, BVDC_TNTD);
    /* Release context in system memory */
    BKNI_Free((void*)hTntd);

    BDBG_LEAVE(BVDC_P_Tntd_Destroy);
    return;
}


/***************************************************************************
 * {private}
 *
 */
static void BVDC_P_Tntd_Init_isr
    ( BVDC_P_Tntd_Handle            hTntd,
      BVDC_Window_Handle            hWindow )
{
    BDBG_ENTER(BVDC_P_Tntd_Init_isr);
    BDBG_OBJECT_ASSERT(hTntd, BVDC_TNTD);

    hTntd->ulUpdateAll = BVDC_P_RUL_UPDATE_THRESHOLD;
    hTntd->hWindow = hWindow;

    /* Clear out shadow registers. */
    BKNI_Memset_isr((void*)hTntd->aulRegs, 0x0, sizeof(hTntd->aulRegs));

    BVDC_P_TNTD_GET_REG_DATA(TNTD_0_LPEAK_KERNEL_CONTROL_DIR3) &= ~(
        BCHP_MASK(TNTD_0_LPEAK_KERNEL_CONTROL_DIR3, MONO)  |
        BCHP_MASK(TNTD_0_LPEAK_KERNEL_CONTROL_DIR3, SCALE) |
        BCHP_MASK(TNTD_0_LPEAK_KERNEL_CONTROL_DIR3, BLUR));
    BVDC_P_TNTD_GET_REG_DATA(TNTD_0_LPEAK_KERNEL_CONTROL_DIR3) |=  (
        BCHP_FIELD_ENUM(TNTD_0_LPEAK_KERNEL_CONTROL_DIR3, MONO,  ENABLE) |
        BCHP_FIELD_DATA(TNTD_0_LPEAK_KERNEL_CONTROL_DIR3, SCALE, 2     ) |
        BCHP_FIELD_DATA(TNTD_0_LPEAK_KERNEL_CONTROL_DIR3, BLUR,  4     ));

    BVDC_P_TNTD_GET_REG_DATA(TNTD_0_LPEAK_INCORE_THR_LOW) &= ~(
        BCHP_MASK(TNTD_0_LPEAK_INCORE_THR_LOW, T2) |
        BCHP_MASK(TNTD_0_LPEAK_INCORE_THR_LOW, T1));
    BVDC_P_TNTD_GET_REG_DATA(TNTD_0_LPEAK_INCORE_THR_LOW) |=  (
        BCHP_FIELD_DATA(TNTD_0_LPEAK_INCORE_THR_LOW, T2, 400) |
        BCHP_FIELD_DATA(TNTD_0_LPEAK_INCORE_THR_LOW, T1, 200));

    BVDC_P_TNTD_GET_REG_DATA(TNTD_0_LPEAK_INCORE_THR_DIR3) &= ~(
        BCHP_MASK(TNTD_0_LPEAK_INCORE_THR_DIR3, T2) |
        BCHP_MASK(TNTD_0_LPEAK_INCORE_THR_DIR3, T1));
    BVDC_P_TNTD_GET_REG_DATA(TNTD_0_LPEAK_INCORE_THR_DIR3) |=  (
        BCHP_FIELD_DATA(TNTD_0_LPEAK_INCORE_THR_DIR3, T2, 400) |
        BCHP_FIELD_DATA(TNTD_0_LPEAK_INCORE_THR_DIR3, T1, 200));

    BVDC_P_TNTD_GET_REG_DATA(TNTD_0_LPEAK_INCORE_THR_DIR5) &= ~(
        BCHP_MASK(TNTD_0_LPEAK_INCORE_THR_DIR5, T2) |
        BCHP_MASK(TNTD_0_LPEAK_INCORE_THR_DIR5, T1));
    BVDC_P_TNTD_GET_REG_DATA(TNTD_0_LPEAK_INCORE_THR_DIR5) |=  (
        BCHP_FIELD_DATA(TNTD_0_LPEAK_INCORE_THR_DIR5, T2, 400) |
        BCHP_FIELD_DATA(TNTD_0_LPEAK_INCORE_THR_DIR5, T1, 200));

    BVDC_P_TNTD_GET_REG_DATA(TNTD_0_LPEAK_OUT_CORE_MODE) &= ~(
        BCHP_MASK(TNTD_0_LPEAK_OUT_CORE_MODE, MODE));
    BVDC_P_TNTD_GET_REG_DATA(TNTD_0_LPEAK_OUT_CORE_MODE) |=  (
        BCHP_FIELD_ENUM(TNTD_0_LPEAK_OUT_CORE_MODE, MODE, DIRECTIONAL));

    BVDC_P_TNTD_GET_REG_DATA(TNTD_0_LPEAK_OUT_CORE) &= ~(
        BCHP_MASK(TNTD_0_LPEAK_OUT_CORE, LCORE_BAND3) |
        BCHP_MASK(TNTD_0_LPEAK_OUT_CORE, LCORE_BAND2) |
        BCHP_MASK(TNTD_0_LPEAK_OUT_CORE, LCORE_BAND1));
    BVDC_P_TNTD_GET_REG_DATA(TNTD_0_LPEAK_OUT_CORE) |=  (
        BCHP_FIELD_DATA(TNTD_0_LPEAK_OUT_CORE, LCORE_BAND3,  5) |
        BCHP_FIELD_DATA(TNTD_0_LPEAK_OUT_CORE, LCORE_BAND2, 10) |
        BCHP_FIELD_DATA(TNTD_0_LPEAK_OUT_CORE, LCORE_BAND1, 10));

    BVDC_P_TNTD_GET_REG_DATA(TNTD_0_LPEAK_CLIP_AVOID) &= ~(
        BCHP_MASK(TNTD_0_LPEAK_CLIP_AVOID, CLIPAVOID_EN) |
        BCHP_MASK(TNTD_0_LPEAK_CLIP_AVOID, SLOPE2      ) |
        BCHP_MASK(TNTD_0_LPEAK_CLIP_AVOID, SLOPE1      ) |
        BCHP_MASK(TNTD_0_LPEAK_CLIP_AVOID, CLIPVAL2    ) |
        BCHP_MASK(TNTD_0_LPEAK_CLIP_AVOID, CLIPVAL1    ));
    BVDC_P_TNTD_GET_REG_DATA(TNTD_0_LPEAK_CLIP_AVOID) |=  (
        BCHP_FIELD_ENUM(TNTD_0_LPEAK_CLIP_AVOID, CLIPAVOID_EN, DISABLE) |
        BCHP_FIELD_DATA(TNTD_0_LPEAK_CLIP_AVOID, SLOPE2,       2      ) |
        BCHP_FIELD_DATA(TNTD_0_LPEAK_CLIP_AVOID, SLOPE1,       2      ) |
        BCHP_FIELD_DATA(TNTD_0_LPEAK_CLIP_AVOID, CLIPVAL2,     970    ) |
        BCHP_FIELD_DATA(TNTD_0_LPEAK_CLIP_AVOID, CLIPVAL1,     50     ));

    BVDC_P_TNTD_GET_REG_DATA(TNTD_0_LTI_CONTROL) &= ~(
        BCHP_MASK(TNTD_0_LTI_CONTROL, LTI_INCORE_EN) |
        BCHP_MASK(TNTD_0_LTI_CONTROL, GAIN      ) |
        BCHP_MASK(TNTD_0_LTI_CONTROL, HAVOID      ) |
        BCHP_MASK(TNTD_0_LTI_CONTROL, VAVOID    ) |
        BCHP_MASK(TNTD_0_LTI_CONTROL, CORE_LEVEL    ));
    BVDC_P_TNTD_GET_REG_DATA(TNTD_0_LTI_CONTROL) |=  (
        BCHP_FIELD_ENUM(TNTD_0_LTI_CONTROL, LTI_INCORE_EN, DISABLE) |
        BCHP_FIELD_DATA(TNTD_0_LTI_CONTROL, GAIN,          8      ) |
        BCHP_FIELD_DATA(TNTD_0_LTI_CONTROL, HAVOID,        12     ) |
        BCHP_FIELD_DATA(TNTD_0_LTI_CONTROL, VAVOID,        12     ) |
        BCHP_FIELD_DATA(TNTD_0_LTI_CONTROL, CORE_LEVEL,    4      ));

    BVDC_P_TNTD_GET_REG_DATA(TNTD_0_LTI_INCORE_THR) &= ~(
        BCHP_MASK(TNTD_0_LTI_INCORE_THR, T2) |
        BCHP_MASK(TNTD_0_LTI_INCORE_THR, T1));
    BVDC_P_TNTD_GET_REG_DATA(TNTD_0_LTI_INCORE_THR) |=  (
        BCHP_FIELD_DATA(TNTD_0_LTI_INCORE_THR, T2, 200) |
        BCHP_FIELD_DATA(TNTD_0_LTI_INCORE_THR, T1, 100));

    /* TODO */
    hTntd->aulRegs[BVDC_P_TNTD_GET_REG_IDX(TNTD_0_LPEAK_GAINSi_ARRAY_BASE)] =
        BCHP_FIELD_DATA(TNTD_0_LPEAK_GAINSi, GAIN_NEG, 0) |
        BCHP_FIELD_DATA(TNTD_0_LPEAK_GAINSi, GAIN_POS, 0);
    hTntd->aulRegs[BVDC_P_TNTD_GET_REG_IDX(TNTD_0_LPEAK_INCORE_DIV_LOWi_ARRAY_BASE)  + 0] = 327;
    hTntd->aulRegs[BVDC_P_TNTD_GET_REG_IDX(TNTD_0_LPEAK_INCORE_DIV_LOWi_ARRAY_BASE)  + 1] = 327;
    hTntd->aulRegs[BVDC_P_TNTD_GET_REG_IDX(TNTD_0_LPEAK_INCORE_DIV_LOWi_ARRAY_BASE)  + 2] = 105;
    hTntd->aulRegs[BVDC_P_TNTD_GET_REG_IDX(TNTD_0_LPEAK_INCORE_DIV_HIGHi_ARRAY_BASE) + 2] = 105;
    hTntd->aulRegs[BVDC_P_TNTD_GET_REG_IDX(TNTD_0_LPEAK_INCORE_DIV_DIR3i_ARRAY_BASE) + 0] = 327;
    hTntd->aulRegs[BVDC_P_TNTD_GET_REG_IDX(TNTD_0_LPEAK_INCORE_DIV_DIR3i_ARRAY_BASE) + 1] = 327;
    hTntd->aulRegs[BVDC_P_TNTD_GET_REG_IDX(TNTD_0_LPEAK_INCORE_DIV_DIR3i_ARRAY_BASE) + 2] = 105;
    hTntd->aulRegs[BVDC_P_TNTD_GET_REG_IDX(TNTD_0_LPEAK_INCORE_DIV_DIR5i_ARRAY_BASE) + 0] = 327;
    hTntd->aulRegs[BVDC_P_TNTD_GET_REG_IDX(TNTD_0_LPEAK_INCORE_DIV_DIR5i_ARRAY_BASE) + 1] = 327;
    hTntd->aulRegs[BVDC_P_TNTD_GET_REG_IDX(TNTD_0_LPEAK_INCORE_DIV_DIR5i_ARRAY_BASE) + 2] = 105;
    hTntd->aulRegs[BVDC_P_TNTD_GET_REG_IDX(TNTD_0_LTI_INCORE_DIVi_ARRAY_BASE)        + 0] = 655;
    hTntd->aulRegs[BVDC_P_TNTD_GET_REG_IDX(TNTD_0_LTI_INCORE_DIVi_ARRAY_BASE)        + 1] = 655;
    hTntd->aulRegs[BVDC_P_TNTD_GET_REG_IDX(TNTD_0_LTI_INCORE_DIVi_ARRAY_BASE)        + 2] =  79;

    /* Initialize state. */
    hTntd->bInitial = true;

    BDBG_LEAVE(BVDC_P_Tntd_Init_isr);
    return;
}


/***************************************************************************
 * {private}
 *
 */
void BVDC_P_Tntd_BuildRul_SetEnable_isr
    ( BVDC_P_Tntd_Handle            hTntd,
      BVDC_P_ListInfo              *pList )
{
    BDBG_OBJECT_ASSERT(hTntd, BVDC_TNTD);

    /* Add scaler registers to RUL using block write */
    if(hTntd->ulUpdateAll)
    {
        hTntd->ulUpdateAll--;

        BVDC_P_TNTD_BLOCK_WRITE_TO_RUL(TNTD_0_TOP_CONFIG, TNTD_0_LTI_INCORE_GOFF, pList->pulCurrent);
        BVDC_P_TNTD_BLOCK_WRITE_nREG_TO_RUL(TNTD_0_LTI_INCORE_DIVi_ARRAY_BASE, 3, pList->pulCurrent);
        BVDC_P_TNTD_WRITE_TO_RUL(TNTD_0_TNTD_VIDEO_MODE, pList->pulCurrent);
    }

    BVDC_P_TNTD_WRITE_TO_RUL(TNTD_0_TOP_CONTROL, pList->pulCurrent);
}



/***************************************************************************
 * {private}
 *
 * BVDC_P_Tntd_AcquireConnect_isr
 *
 * It is called by BVDC_Window_Validate after changing from diable mad to
 * enable mad, .
 */
BERR_Code BVDC_P_Tntd_AcquireConnect_isr
    ( BVDC_P_Tntd_Handle            hTntd,
      BVDC_Window_Handle            hWindow )
{
    BERR_Code  eResult = BERR_SUCCESS;

    BDBG_ENTER(BVDC_P_Tntd_AcquireConnect_isr);

    /* init Tntd */
    BVDC_P_Tntd_Init_isr(hTntd, hWindow);

    BDBG_LEAVE(BVDC_P_Tntd_AcquireConnect_isr);
    return BERR_TRACE(eResult);
}

/***************************************************************************
 * {private}
 *
 * BVDC_P_Tntd_ReleaseConnect_isr
 *
 * It is called after window decided that mad is no-longer used by HW in its
 * vnet mode (i.e. it is really shut down and teared off from vnet).
 */
BERR_Code BVDC_P_Tntd_ReleaseConnect_isr
    ( BVDC_P_Tntd_Handle        *phTntd )
{
    BERR_Code  eResult = BERR_SUCCESS;

    BDBG_ENTER(BVDC_P_Tntd_ReleaseConnect_isr);

    /* handle validation */
    if (NULL != *phTntd)
    {
        BDBG_OBJECT_ASSERT(*phTntd, BVDC_TNTD);
        BDBG_ASSERT(0 == (*phTntd)->SubRul.ulWinsActFlags);

        BVDC_P_Resource_ReleaseHandle_isr(
            BVDC_P_SubRul_GetResourceHandle_isr(&(*phTntd)->SubRul),
            BVDC_P_ResourceType_eTntd, (void *)(*phTntd));

        /* this makes win to stop calling Tntd code */
        *phTntd = NULL;
    }

    BDBG_LEAVE(BVDC_P_Tntd_ReleaseConnect_isr);
    return BERR_TRACE(eResult);
}

/***************************************************************************
 * {private}
 *
 * BVDC_P_Tntd_BuildRul_DrainVnet_isr
 *
 * called by BVDC_P_Tntd_BuildRul_isr after resetting to drain the module and
 * its pre-patch FreeCh or LpBack
 */
static void BVDC_P_Tntd_BuildRul_DrainVnet_isr
    ( BVDC_P_Tntd_Handle          hTntd,
      BVDC_P_ListInfo            *pList,
      bool                        bNoCoreReset)
{
    BDBG_OBJECT_ASSERT(hTntd, BVDC_TNTD);
    /* reset sub and connect the module to a drain */
    BVDC_P_SubRul_Drain_isr(&(hTntd->SubRul), pList,
        bNoCoreReset? 0 : hTntd->ulResetRegAddr,
        bNoCoreReset? 0 : hTntd->ulResetMask,
        bNoCoreReset? 0 : hTntd->ulVnetResetAddr,
        bNoCoreReset? 0 : hTntd->ulVnetResetMask);
}

/***************************************************************************
 * {private}
 *
 */
void BVDC_P_Tntd_BuildRul_isr
    ( const BVDC_P_Tntd_Handle      hTntd,
      BVDC_P_ListInfo              *pList,
      BVDC_P_State                  eVnetState,
      BVDC_P_PicComRulInfo         *pPicComRulInfo )
{
    uint32_t  ulRulOpsFlags;

    BDBG_ENTER(BVDC_P_Tntd_BuildRul_isr);
    BDBG_OBJECT_ASSERT(hTntd, BVDC_TNTD);

    /* currently this is only for vnet building / tearing-off*/

    ulRulOpsFlags = BVDC_P_SubRul_GetOps_isr(
        &(hTntd->SubRul), pPicComRulInfo->eWin, eVnetState, pList->bLastExecuted);

    if ((0 == ulRulOpsFlags) ||
        (ulRulOpsFlags & BVDC_P_RulOp_eReleaseHandle))
        return;
    else if (ulRulOpsFlags & BVDC_P_RulOp_eDisable)
    {
        BVDC_P_SubRul_DropOffVnet_isr(&(hTntd->SubRul), pList);
        BVDC_P_Tntd_SetEnable_isr(hTntd, false);
        BVDC_P_TNTD_WRITE_TO_RUL(TNTD_0_TOP_CONTROL, pList->pulCurrent);
    }

    /* If rul failed to execute last time we'd re reprogrammed possible
     * missing registers. */
    if((!pList->bLastExecuted)|| (hTntd->bInitial))
    {
        hTntd->ulUpdateAll = BVDC_P_RUL_UPDATE_THRESHOLD;
    }

    /* reset */
    if(hTntd->bInitial)
    {
        hTntd->bInitial = false;
    }

    if (ulRulOpsFlags & BVDC_P_RulOp_eEnable)
    {
        BVDC_P_Tntd_BuildRul_SetEnable_isr(hTntd, pList);

        /* join in vnet after enable. note: its src mux is initialed as disabled */
        if (ulRulOpsFlags & BVDC_P_RulOp_eVnetInit)
        {
            BVDC_P_SubRul_JoinInVnet_isr(&(hTntd->SubRul), pList);
        }
    }

    if (ulRulOpsFlags & BVDC_P_RulOp_eDrainVnet)
    {
        BVDC_P_Tntd_BuildRul_DrainVnet_isr(hTntd, pList, pPicComRulInfo->bNoCoreReset);
    }

    BDBG_LEAVE(BVDC_P_Tntd_BuildRul_isr);
    return;
}

/***************************************************************************
 * {private}
 *
 */
#define BVDC_P_TNTD_ADJUST_VAL(val) (val * ulSharpness / 16)

static const BVDC_P_TntdConfigTbl s_aConfigTbl[] =
{
    {BDBG_STRING("ConfigSmall"),  4, 0, 2, 1, { 0,  0, 12,  0,  0,  0,  0, 12, 12, 12, 12, 12, 12, 16,  4}, { 0,  0, 12,  0,  0,  0,  0, 12, 12, 12, 12, 12, 12, 16,  4}, 0, 0, 0,   0, 0, 0, 0, -12, 0, 0, 0, -16, 0, 0, 0,  -4,  50, 400, 1310, 187, 2, 2, 1},
    {BDBG_STRING("ConfigMedium"), 4, 1, 2, 1, { 7,  7,  6,  7,  7,  7,  7,  6,  6,  6,  6,  6,  6,  8,  9}, { 7,  7,  6,  7,  7,  7,  7,  6,  6,  6,  6,  6,  6,  8,  9}, 0, 0, 0,  -6, 0, 0, 0,  -7, 0, 0, 0,  -8, 0, 0, 0,  -9, 200, 400,  327, 327, 0, 0, 1},
    {BDBG_STRING("ConfigLarge"),  4, 1, 2, 1, {15, 15,  0, 15, 15, 15, 15,  0,  0,  0,  0,  0,  0,  0, 15}, {15, 15,  0, 15, 15, 15, 15,  0,  0,  0,  0,  0,  0,  0, 15}, 0, 0, 0, -15, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0, -15, 200, 400,  327, 327, 0, 0, 1},
};

/*
    The TNTD document provides recommended settings for:
    1) Small scale (or downscale) for vertical scale factors < 1.3X.
    2) Medium scale for vertical scale factors >= 1.3X vertical upscale.
    3) Large scale for vertical scale factors >= 1.8X vertical upscale.
    4) For vertical scale factors >= 2.5X, we recommend sharpening before scaling.
       When sharpening before scaling, use the "small" scale settings.
*/
static void BVDC_P_Tntd_SelectConfigTable_isr
    ( uint32_t                      ulVertRatio,
      const BVDC_P_TntdConfigTbl  **pConfigTbl )
{
    if(ulVertRatio <  BVDC_P_TNTD_SML_CONFIG_THRESH)
    {
        *pConfigTbl = &s_aConfigTbl[0];
    }
    else if(ulVertRatio < BVDC_P_TNTD_MED_CONFIG_THRESH)
    {
        *pConfigTbl = &s_aConfigTbl[1];
    }
    else if(ulVertRatio < BVDC_P_TNTD_LRG_CONFIG_THRESH)
    {
        *pConfigTbl = &s_aConfigTbl[2];
    }
    else
    {
        *pConfigTbl = &s_aConfigTbl[0];
    }
}

uint32_t BVDC_P_Tntd_CalcVertSclRatio_isr
    ( uint32_t                      ulInV,
      bool                          bInInterlaced,
      uint32_t                      ulOutV,
      bool                          bOutInterlaced )
{
    uint32_t ulIn = (bInInterlaced) ? ulInV / BVDC_P_FIELD_PER_FRAME : ulInV;
    uint32_t ulOut = (bOutInterlaced) ? ulOutV / BVDC_P_FIELD_PER_FRAME : ulOutV;
    /* BDBG_MSG(("In: %d(%s), Out: %d(%s)", ulInV, bInInterlaced ? "I" : "P", ulOutV, bOutInterlaced ? "I" : "P"));*/
    return BVDC_P_DIV_ROUND_NEAR(ulOut << BVDC_P_NRM_SRC_STEP_F_BITS, ulIn);
}

/***************************************************************************
 * {private}
 *
 */
int16_t BVDC_P_Tntd_MapSharpnessForHdr_isr
    ( int16_t                      sSharpness )
{
    int16_t sImplSharpness;

    if(sSharpness <= -2048)
    {
        sImplSharpness = -32768 +(int16_t)((sSharpness + 32768) * 8 / 10);
    }
    else if(sSharpness <= 2048)
    {
        sImplSharpness = -8192 + sSharpness + 2048;
    }
    else if(sSharpness < 32767)
    {
        sImplSharpness = -4096 + (int16_t)((sSharpness - 2048) * 12 / 10);
    }
    else
    {
        sImplSharpness = 32767;
    }
    return sImplSharpness;
}

/***************************************************************************
 * {private}
 *
 */
void BVDC_P_Tntd_SetInfo_isr
    ( BVDC_P_Tntd_Handle            hTntd,
      const BVDC_P_PictureNodePtr   pPicture )
{
    uint32_t i;
    uint32_t ulWidth  = 0;
    uint32_t ulHeight = 0;
    uint32_t ulVertRatio = 0;
    uint32_t ulSharpness = 0;
    BVDC_P_Rect  *pSclIn;
    BVDC_P_Rect  *pSclOut;
    bool bPqNcl;
    int16_t sImplSharpness = hTntd->hWindow->stCurInfo.sSharpness;

    BDBG_ENTER(BVDC_P_Tntd_SetInfo_isr);
    BDBG_OBJECT_ASSERT(hTntd, BVDC_TNTD);
    BDBG_OBJECT_ASSERT(hTntd->hWindow, BVDC_WIN);
    pSclIn  = pPicture->pSclIn;
    pSclOut = pPicture->pSclOut;

    /* calculate the vertical Scaling Factor */
    ulVertRatio = BVDC_P_Tntd_CalcVertSclRatio_isr(
        pSclIn->ulHeight,
        (BAVC_Polarity_eFrame==pPicture->eSrcPolarity) ? false : true,
        pSclOut->ulHeight,
        (BAVC_Polarity_eFrame==pPicture->eDstPolarity)? false : true);

    /* If vert ratio >= 2.5 => TNTD before SCL, otherwise TNTD after scaler */
    /*if(ulVertRatio >= BVDC_P_FLOAT_TO_FIXED(2.5, BVDC_P_SCL_V_RATIO_I_BITS, BVDC_P_NRM_SRC_STEP_F_BITS))*/
    if(pPicture->stVnetMode.stBits.bTntdBeforeScl)
    {
        ulWidth = pSclIn->ulWidth;
        ulHeight = (BAVC_Polarity_eFrame==pPicture->eSrcPolarity)?
            (pSclIn->ulHeight) :
            (pSclIn->ulHeight) / BVDC_P_FIELD_PER_FRAME;
    }
    else
    {
        ulWidth = pSclOut->ulWidth;
        ulHeight = (BAVC_Polarity_eFrame==pPicture->eDstPolarity)?
            (pSclOut->ulHeight) :
            (pSclOut->ulHeight) / BVDC_P_FIELD_PER_FRAME;
    }

    bPqNcl = (pPicture->astMosaicColorSpace[pPicture->ulPictureIdx].eColorTF == BAVC_P_ColorTF_eBt2100Pq &&
              pPicture->astMosaicColorSpace[pPicture->ulPictureIdx].eColorFmt == BAVC_P_ColorFormat_eYCbCr) ? true : false;

    if((hTntd->hWindow->stCurInfo.bSharpnessEnable != hTntd->bSharpnessEnable) ||
       (hTntd->hWindow->stCurInfo.sSharpness != hTntd->sSharpness) ||
       (hTntd->hWindow->stCurInfo.stSplitScreenSetting.eSharpness != hTntd->eDemoMode) ||
       (ulWidth  != hTntd->ulPrevWidth) ||
       (ulHeight != hTntd->ulPrevHeight) ||
       (bPqNcl != hTntd->bPqNcl) ||
       (pPicture->stVnetMode.stBits.bTntdBeforeScl != hTntd->bPrevTntdBeforeScl) ||
       !BVDC_P_TNTD_COMPARE_FIELD_DATA(TNTD_0_TOP_CONTROL, TNTD_ENABLE, 1) ||
       (pPicture->eSrcOrientation        != hTntd->ePrevSrcOrientation) ||
       (pPicture->eDispOrientation       != hTntd->ePrevDispOrientation))
    {
        /* for next dirty check */
        hTntd->bSharpnessEnable = hTntd->hWindow->stCurInfo.bSharpnessEnable;
        hTntd->sSharpness = hTntd->hWindow->stCurInfo.sSharpness;
        hTntd->eDemoMode = hTntd->hWindow->stCurInfo.stSplitScreenSetting.eSharpness;
        hTntd->ulPrevWidth  = ulWidth;
        hTntd->ulPrevHeight = ulHeight;
        hTntd->bPrevTntdBeforeScl = pPicture->stVnetMode.stBits.bTntdBeforeScl;
        hTntd->ePrevSrcOrientation = pPicture->eSrcOrientation;
        hTntd->ePrevDispOrientation = pPicture->eDispOrientation;
        hTntd->bPqNcl = bPqNcl;
        hTntd->ulUpdateAll = BVDC_P_RUL_UPDATE_THRESHOLD;

        if(bPqNcl)
        {
            sImplSharpness = BVDC_P_Tntd_MapSharpnessForHdr_isr(hTntd->hWindow->stCurInfo.sSharpness);
        }

        BVDC_P_Tntd_SelectConfigTable_isr(ulVertRatio, &hTntd->pConfigTbl);

        /* TNTD_0_TOP_CONTROL */
        BVDC_P_TNTD_GET_REG_DATA(TNTD_0_TOP_CONTROL) &= ~(
            BCHP_MASK(TNTD_0_TOP_CONTROL, ENABLE_CTRL    ) |
            BCHP_MASK(TNTD_0_TOP_CONTROL, TNTD_ENABLE    ));
        BVDC_P_TNTD_GET_REG_DATA(TNTD_0_TOP_CONTROL) |=  (
            BCHP_FIELD_ENUM(TNTD_0_TOP_CONTROL, ENABLE_CTRL, DISABLE) |
            BCHP_FIELD_ENUM(TNTD_0_TOP_CONTROL, TNTD_ENABLE, ENABLE ));

        /* TNTD_0_TOP_CONFIG */
        BVDC_P_TNTD_GET_REG_DATA(TNTD_0_TOP_CONFIG) &= ~(
            BCHP_MASK(TNTD_0_TOP_CONFIG, UPSCALEX       ) |
            BCHP_MASK(TNTD_0_TOP_CONFIG, LPEAK_INCORE_EN) |
            BCHP_MASK(TNTD_0_TOP_CONFIG, DEMO_MODE      ) |
            BCHP_MASK(TNTD_0_TOP_CONFIG, H_WINDOW       ) |
            BCHP_MASK(TNTD_0_TOP_CONFIG, BYPASS_MODE    ));
        BVDC_P_TNTD_GET_REG_DATA(TNTD_0_TOP_CONFIG) |=  (
            BCHP_FIELD_ENUM(TNTD_0_TOP_CONFIG, UPSCALEX,        DISABLE) |
            BCHP_FIELD_ENUM(TNTD_0_TOP_CONFIG, LPEAK_INCORE_EN, ENABLE ) |
            ((hTntd->hWindow->stCurInfo.stSplitScreenSetting.eSharpness == BVDC_SplitScreenMode_eDisable) ?
                BCHP_FIELD_ENUM(TNTD_0_TOP_CONFIG, DEMO_MODE,   DISABLE) :
                BCHP_FIELD_ENUM(TNTD_0_TOP_CONFIG, DEMO_MODE,   ENABLE ))|
            BCHP_FIELD_DATA(TNTD_0_TOP_CONFIG, H_WINDOW,        0      ) |
            BCHP_FIELD_ENUM(TNTD_0_TOP_CONFIG, BYPASS_MODE,     DISABLE));

        /* TNTD_0_SRC_PIC_SIZE */
        BVDC_P_TNTD_GET_REG_DATA(TNTD_0_SRC_PIC_SIZE) &= ~(
            BCHP_MASK(TNTD_0_SRC_PIC_SIZE, HSIZE) |
            BCHP_MASK(TNTD_0_SRC_PIC_SIZE, VSIZE));
        BVDC_P_TNTD_GET_REG_DATA(TNTD_0_SRC_PIC_SIZE) |=  (
            BCHP_FIELD_DATA(TNTD_0_SRC_PIC_SIZE, HSIZE, ulWidth ) |
            BCHP_FIELD_DATA(TNTD_0_SRC_PIC_SIZE, VSIZE, ulHeight));

        /* TNTD_0_DEMO_SETTING */
        BVDC_P_TNTD_GET_REG_DATA(TNTD_0_DEMO_SETTING) &= ~(
            BCHP_MASK(TNTD_0_DEMO_SETTING, DEMO_L_R) |
            BCHP_MASK(TNTD_0_DEMO_SETTING, DEMO_BOUNDARY));
        BVDC_P_TNTD_GET_REG_DATA(TNTD_0_DEMO_SETTING) |=  (
            ((hTntd->hWindow->stCurInfo.stSplitScreenSetting.eSharpness == BVDC_SplitScreenMode_eLeft) ?
                BCHP_FIELD_ENUM(TNTD_0_DEMO_SETTING, DEMO_L_R, LEFT ) :
                BCHP_FIELD_ENUM(TNTD_0_DEMO_SETTING, DEMO_L_R, RIGHT))|
            BCHP_FIELD_DATA(TNTD_0_DEMO_SETTING, DEMO_BOUNDARY, ulWidth/2));

        /* TNTD_0_TNTD_VIDEO_MODE */
        BVDC_P_TNTD_GET_REG_DATA(TNTD_0_TNTD_VIDEO_MODE) &= ~(
            BCHP_MASK(TNTD_0_TNTD_VIDEO_MODE, BVB_VIDEO));
        BVDC_P_TNTD_GET_REG_DATA(TNTD_0_TNTD_VIDEO_MODE) |=  (
            BVDC_P_VNET_USED_SCALER_AT_WRITER(pPicture->stVnetMode) ?
            BCHP_FIELD_DATA(TNTD_0_TNTD_VIDEO_MODE, BVB_VIDEO, pPicture->eSrcOrientation) :
            BCHP_FIELD_DATA(TNTD_0_TNTD_VIDEO_MODE, BVB_VIDEO, pPicture->eDispOrientation));

        /* calculate sharpness value [1, 16] */
        BVDC_P_Sharpness_Calculate_Gain_Value_isr(
            sImplSharpness, 0, 8, 16, &ulSharpness);

        BVDC_P_TNTD_GET_REG_DATA(TNTD_0_LPEAK_KERNEL_CONTROL_DIR5) &= ~(
            BCHP_MASK(TNTD_0_LPEAK_KERNEL_CONTROL_DIR5, MONO     ) |
            BCHP_MASK(TNTD_0_LPEAK_KERNEL_CONTROL_DIR5, SCALE    ) |
            BCHP_MASK(TNTD_0_LPEAK_KERNEL_CONTROL_DIR5, WIDE_BLUR) |
            BCHP_MASK(TNTD_0_LPEAK_KERNEL_CONTROL_DIR5, BLUR     ));
        BVDC_P_TNTD_GET_REG_DATA(TNTD_0_LPEAK_KERNEL_CONTROL_DIR5) |=  (
            BCHP_FIELD_DATA(TNTD_0_LPEAK_KERNEL_CONTROL_DIR5, MONO,      hTntd->pConfigTbl->ulLPeakKernelCtrlDir5Mono    ) |
            BCHP_FIELD_DATA(TNTD_0_LPEAK_KERNEL_CONTROL_DIR5, SCALE,     hTntd->pConfigTbl->ulLPeakKernelCtrlDir5Scale   ) |
            BCHP_FIELD_DATA(TNTD_0_LPEAK_KERNEL_CONTROL_DIR5, WIDE_BLUR, hTntd->pConfigTbl->ulLPeakKernelCtrlDir5WideBlur) |
            BCHP_FIELD_DATA(TNTD_0_LPEAK_KERNEL_CONTROL_DIR5, BLUR,      hTntd->pConfigTbl->ulLPeakKernelCtrlDir5Blur    ));

        for(i = 0; i < 15; i++)
        {
            hTntd->aulRegs[BVDC_P_TNTD_GET_REG_IDX(TNTD_0_LPEAK_GAINSi_ARRAY_BASE) + i + 1] =
                BCHP_FIELD_DATA(TNTD_0_LPEAK_GAINSi, GAIN_NEG, BVDC_P_TNTD_ADJUST_VAL(hTntd->pConfigTbl->alLPeakGainNeg[i])) |
                BCHP_FIELD_DATA(TNTD_0_LPEAK_GAINSi, GAIN_POS, BVDC_P_TNTD_ADJUST_VAL(hTntd->pConfigTbl->alLPeakGainPos[i]));
        }

        BVDC_P_TNTD_GET_REG_DATA(TNTD_0_LPEAK_INCORE_GOFF_LOW) &= ~(
            BCHP_MASK(TNTD_0_LPEAK_INCORE_GOFF_LOW, OFFSET4) |
            BCHP_MASK(TNTD_0_LPEAK_INCORE_GOFF_LOW, OFFSET3) |
            BCHP_MASK(TNTD_0_LPEAK_INCORE_GOFF_LOW, OFFSET2) |
            BCHP_MASK(TNTD_0_LPEAK_INCORE_GOFF_LOW, OFFSET1));
        BVDC_P_TNTD_GET_REG_DATA(TNTD_0_LPEAK_INCORE_GOFF_LOW) |=  (
            BCHP_FIELD_DATA(TNTD_0_LPEAK_INCORE_GOFF_LOW, OFFSET4, BVDC_P_TNTD_ADJUST_VAL(hTntd->pConfigTbl->lLPeakIncoreGoffLowOffset4)) |
            BCHP_FIELD_DATA(TNTD_0_LPEAK_INCORE_GOFF_LOW, OFFSET3, BVDC_P_TNTD_ADJUST_VAL(hTntd->pConfigTbl->lLPeakIncoreGoffLowOffset3)) |
            BCHP_FIELD_DATA(TNTD_0_LPEAK_INCORE_GOFF_LOW, OFFSET2, BVDC_P_TNTD_ADJUST_VAL(hTntd->pConfigTbl->lLPeakIncoreGoffLowOffset2)) |
            BCHP_FIELD_DATA(TNTD_0_LPEAK_INCORE_GOFF_LOW, OFFSET1, BVDC_P_TNTD_ADJUST_VAL(hTntd->pConfigTbl->lLPeakIncoreGoffLowOffset1)));

        BVDC_P_TNTD_GET_REG_DATA(TNTD_0_LPEAK_INCORE_GOFF_HIGH) &= ~(
            BCHP_MASK(TNTD_0_LPEAK_INCORE_GOFF_HIGH, OFFSET4) |
            BCHP_MASK(TNTD_0_LPEAK_INCORE_GOFF_HIGH, OFFSET3) |
            BCHP_MASK(TNTD_0_LPEAK_INCORE_GOFF_HIGH, OFFSET2) |
            BCHP_MASK(TNTD_0_LPEAK_INCORE_GOFF_HIGH, OFFSET1));
        BVDC_P_TNTD_GET_REG_DATA(TNTD_0_LPEAK_INCORE_GOFF_HIGH) |=  (
            BCHP_FIELD_DATA(TNTD_0_LPEAK_INCORE_GOFF_HIGH, OFFSET4, BVDC_P_TNTD_ADJUST_VAL(hTntd->pConfigTbl->lLPeakIncoreGoffHighOffset4)) |
            BCHP_FIELD_DATA(TNTD_0_LPEAK_INCORE_GOFF_HIGH, OFFSET3, BVDC_P_TNTD_ADJUST_VAL(hTntd->pConfigTbl->lLPeakIncoreGoffHighOffset3)) |
            BCHP_FIELD_DATA(TNTD_0_LPEAK_INCORE_GOFF_HIGH, OFFSET2, BVDC_P_TNTD_ADJUST_VAL(hTntd->pConfigTbl->lLPeakIncoreGoffHighOffset2)) |
            BCHP_FIELD_DATA(TNTD_0_LPEAK_INCORE_GOFF_HIGH, OFFSET1, BVDC_P_TNTD_ADJUST_VAL(hTntd->pConfigTbl->lLPeakIncoreGoffHighOffset1)));

        BVDC_P_TNTD_GET_REG_DATA(TNTD_0_LPEAK_INCORE_GOFF_DIR3) &= ~(
            BCHP_MASK(TNTD_0_LPEAK_INCORE_GOFF_DIR3, OFFSET4) |
            BCHP_MASK(TNTD_0_LPEAK_INCORE_GOFF_DIR3, OFFSET3) |
            BCHP_MASK(TNTD_0_LPEAK_INCORE_GOFF_DIR3, OFFSET2) |
            BCHP_MASK(TNTD_0_LPEAK_INCORE_GOFF_DIR3, OFFSET1));
        BVDC_P_TNTD_GET_REG_DATA(TNTD_0_LPEAK_INCORE_GOFF_DIR3) |=  (
            BCHP_FIELD_DATA(TNTD_0_LPEAK_INCORE_GOFF_DIR3, OFFSET4, BVDC_P_TNTD_ADJUST_VAL(hTntd->pConfigTbl->lLPeakIncoreGoffDir3Offset4)) |
            BCHP_FIELD_DATA(TNTD_0_LPEAK_INCORE_GOFF_DIR3, OFFSET3, BVDC_P_TNTD_ADJUST_VAL(hTntd->pConfigTbl->lLPeakIncoreGoffDir3Offset3)) |
            BCHP_FIELD_DATA(TNTD_0_LPEAK_INCORE_GOFF_DIR3, OFFSET2, BVDC_P_TNTD_ADJUST_VAL(hTntd->pConfigTbl->lLPeakIncoreGoffDir3Offset2)) |
            BCHP_FIELD_DATA(TNTD_0_LPEAK_INCORE_GOFF_DIR3, OFFSET1, BVDC_P_TNTD_ADJUST_VAL(hTntd->pConfigTbl->lLPeakIncoreGoffDir3Offset1)));

        BVDC_P_TNTD_GET_REG_DATA(TNTD_0_LPEAK_INCORE_GOFF_DIR5) &= ~(
            BCHP_MASK(TNTD_0_LPEAK_INCORE_GOFF_DIR5, OFFSET4) |
            BCHP_MASK(TNTD_0_LPEAK_INCORE_GOFF_DIR5, OFFSET3) |
            BCHP_MASK(TNTD_0_LPEAK_INCORE_GOFF_DIR5, OFFSET2) |
            BCHP_MASK(TNTD_0_LPEAK_INCORE_GOFF_DIR5, OFFSET1));
        BVDC_P_TNTD_GET_REG_DATA(TNTD_0_LPEAK_INCORE_GOFF_DIR5) |=  (
            BCHP_FIELD_DATA(TNTD_0_LPEAK_INCORE_GOFF_DIR5, OFFSET4, BVDC_P_TNTD_ADJUST_VAL(hTntd->pConfigTbl->lLPeakIncoreGoffDir5Offset4)) |
            BCHP_FIELD_DATA(TNTD_0_LPEAK_INCORE_GOFF_DIR5, OFFSET3, BVDC_P_TNTD_ADJUST_VAL(hTntd->pConfigTbl->lLPeakIncoreGoffDir5Offset3)) |
            BCHP_FIELD_DATA(TNTD_0_LPEAK_INCORE_GOFF_DIR5, OFFSET2, BVDC_P_TNTD_ADJUST_VAL(hTntd->pConfigTbl->lLPeakIncoreGoffDir5Offset2)) |
            BCHP_FIELD_DATA(TNTD_0_LPEAK_INCORE_GOFF_DIR5, OFFSET1, BVDC_P_TNTD_ADJUST_VAL(hTntd->pConfigTbl->lLPeakIncoreGoffDir5Offset1)));

        BVDC_P_TNTD_GET_REG_DATA(TNTD_0_LPEAK_INCORE_THR_HIGH) &= ~(
            BCHP_MASK(TNTD_0_LPEAK_INCORE_THR_HIGH, T2) |
            BCHP_MASK(TNTD_0_LPEAK_INCORE_THR_HIGH, T1));
        BVDC_P_TNTD_GET_REG_DATA(TNTD_0_LPEAK_INCORE_THR_HIGH) |=  (
            BCHP_FIELD_DATA(TNTD_0_LPEAK_INCORE_THR_HIGH, T2, hTntd->pConfigTbl->ulLPeakIncoreThrHighT2) |
            BCHP_FIELD_DATA(TNTD_0_LPEAK_INCORE_THR_HIGH, T1, hTntd->pConfigTbl->ulLPeakIncoreThrHighT1));

        hTntd->aulRegs[BVDC_P_TNTD_GET_REG_IDX(TNTD_0_LPEAK_INCORE_DIV_HIGHi_ARRAY_BASE) + 0] =
            BCHP_FIELD_DATA(TNTD_0_LPEAK_INCORE_DIV_HIGHi, ONE_OVER_T, hTntd->pConfigTbl->ulLPeakIncoreDivHigh0OneOverT);
        hTntd->aulRegs[BVDC_P_TNTD_GET_REG_IDX(TNTD_0_LPEAK_INCORE_DIV_HIGHi_ARRAY_BASE) + 1] =
            BCHP_FIELD_DATA(TNTD_0_LPEAK_INCORE_DIV_HIGHi, ONE_OVER_T, hTntd->pConfigTbl->ulLPeakIncoreDivHigh1OneOverT);

        BVDC_P_TNTD_GET_REG_DATA(TNTD_0_LTI_FILTER) &= ~(
            BCHP_MASK(TNTD_0_LTI_FILTER, BLUR_EN)    |
            BCHP_MASK(TNTD_0_LTI_FILTER, V_FILT_SEL) |
            BCHP_MASK(TNTD_0_LTI_FILTER, H_FILT_SEL));
        BVDC_P_TNTD_GET_REG_DATA(TNTD_0_LTI_FILTER) |=  (
            BCHP_FIELD_DATA(TNTD_0_LTI_FILTER, BLUR_EN,    hTntd->pConfigTbl->ulLtiFilterBlurEn    ) |
            BCHP_FIELD_DATA(TNTD_0_LTI_FILTER, V_FILT_SEL, hTntd->pConfigTbl->ulLtiFilterVFilterSel) |
            BCHP_FIELD_DATA(TNTD_0_LTI_FILTER, H_FILT_SEL, hTntd->pConfigTbl->ulLtiFilterHFilterSel));

        BVDC_P_TNTD_GET_REG_DATA(TNTD_0_LTI_INCORE_GOFF) &= ~(
            BCHP_MASK(TNTD_0_LTI_INCORE_GOFF, OFFSET4) |
            BCHP_MASK(TNTD_0_LTI_INCORE_GOFF, OFFSET3) |
            BCHP_MASK(TNTD_0_LTI_INCORE_GOFF, OFFSET2) |
            BCHP_MASK(TNTD_0_LTI_INCORE_GOFF, OFFSET1));
        BVDC_P_TNTD_GET_REG_DATA(TNTD_0_LTI_INCORE_GOFF) |=  (
            BCHP_FIELD_DATA(TNTD_0_LTI_INCORE_GOFF, OFFSET4, BVDC_P_TNTD_ADJUST_VAL(0) ) |
            BCHP_FIELD_DATA(TNTD_0_LTI_INCORE_GOFF, OFFSET3, BVDC_P_TNTD_ADJUST_VAL(0) ) |
            BCHP_FIELD_DATA(TNTD_0_LTI_INCORE_GOFF, OFFSET2, BVDC_P_TNTD_ADJUST_VAL(-8)) |
            BCHP_FIELD_DATA(TNTD_0_LTI_INCORE_GOFF, OFFSET1, BVDC_P_TNTD_ADJUST_VAL(-8)));
    }

    /* Printing out ratio in float format would be nice, but PI
     * code does permit float. */
    if(BVDC_P_RUL_UPDATE_THRESHOLD == hTntd->ulUpdateAll)
    {
        BDBG_MSG(("-------------------------"));
        BDBG_MSG(("sSharpness=%d && bPqNcl =%d => sImplSharpness=%d => ulSharpness=%d",
            hTntd->hWindow->stCurInfo.sSharpness, bPqNcl, sImplSharpness, ulSharpness));
        BDBG_MSG(("SCL in: %dx%d(%d) - SCL out: %dx%d(%d) => TNTD: %dx%d",
            pSclIn->ulWidth, pSclIn->ulHeight, pPicture->eSrcPolarity,
            pSclOut->ulWidth, pSclOut->ulHeight, pPicture->eDstPolarity,
            ulWidth, ulHeight));
        BDBG_MSG(("Verticle ratio: 0x%08x => Config: %s",
            ulVertRatio, hTntd->pConfigTbl->pchConfig));
    }

    BDBG_LEAVE(BVDC_P_Tntd_SetInfo_isr);
    return;
}

/***************************************************************************
 * {private}
 *
 */
void BVDC_P_Tntd_SetEnable_isr
    ( BVDC_P_Tntd_Handle            hTntd,
      bool                          bEnable )
{
    BDBG_OBJECT_ASSERT(hTntd, BVDC_TNTD);

    if(!BVDC_P_TNTD_COMPARE_FIELD_DATA(TNTD_0_TOP_CONTROL, TNTD_ENABLE, bEnable))
    {
        hTntd->ulUpdateAll = BVDC_P_RUL_UPDATE_THRESHOLD;
    }

    /* Turn on/off the scaler. */
    BVDC_P_TNTD_GET_REG_DATA(TNTD_0_TOP_CONTROL) &= ~(
        BCHP_MASK(TNTD_0_TOP_CONTROL, TNTD_ENABLE));

    BVDC_P_TNTD_GET_REG_DATA(TNTD_0_TOP_CONTROL) |=  (bEnable
        ? BCHP_FIELD_ENUM(TNTD_0_TOP_CONTROL, TNTD_ENABLE, ENABLE)
        : BCHP_FIELD_ENUM(TNTD_0_TOP_CONTROL, TNTD_ENABLE, DISABLE));

    return;
}
#else
BERR_Code BVDC_P_Tntd_Create
    ( BVDC_P_Tntd_Handle           *phTntd,
      BVDC_P_TntdId                 eTntdId,
      BVDC_P_Resource_Handle        hResource,
      BREG_Handle                   hReg )
{
    BSTD_UNUSED(phTntd);
    BSTD_UNUSED(eTntdId);
    BSTD_UNUSED(hResource);
    BSTD_UNUSED(hReg);
    return BERR_SUCCESS;
}

void BVDC_P_Tntd_Destroy
    ( BVDC_P_Tntd_Handle            hTntd )
{
    BSTD_UNUSED(hTntd);
    return;
}

void BVDC_P_Tntd_BuildRul_SetEnable_isr
    ( BVDC_P_Tntd_Handle            hTntd,
      BVDC_P_ListInfo              *pList )
{
    BSTD_UNUSED(hTntd);
    BSTD_UNUSED(pList);
}

BERR_Code BVDC_P_Tntd_AcquireConnect_isr
    ( BVDC_P_Tntd_Handle            hTntd,
      BVDC_Window_Handle            hWindow )
{
    BSTD_UNUSED(hTntd);
    BSTD_UNUSED(hWindow);
    return BERR_SUCCESS;
}

BERR_Code BVDC_P_Tntd_ReleaseConnect_isr
    ( BVDC_P_Tntd_Handle           *phTntd )
{
    BSTD_UNUSED(phTntd);
    return BERR_SUCCESS;
}

void BVDC_P_Tntd_BuildRul_isr
    ( const BVDC_P_Tntd_Handle      hTntd,
      BVDC_P_ListInfo              *pList,
      BVDC_P_State                  eVnetState,
      BVDC_P_PicComRulInfo         *pPicComRulInfo )
{
    BSTD_UNUSED(hTntd);
    BSTD_UNUSED(pList);
    BSTD_UNUSED(eVnetState);
    BSTD_UNUSED(pPicComRulInfo);
    return;
}

uint32_t BVDC_P_Tntd_CalcVertSclRatio_isr
    ( uint32_t                      ulInV,
      bool                          bInInterlaced,
      uint32_t                      ulOutV,
      bool                          bOutInterlaced )
{
    BSTD_UNUSED(ulInV);
    BSTD_UNUSED(bInInterlaced);
    BSTD_UNUSED(ulOutV);
    BSTD_UNUSED(bOutInterlaced);
    return 0;
}

void BVDC_P_Tntd_SetInfo_isr
    ( BVDC_P_Tntd_Handle            hTntd,
      const BVDC_P_PictureNodePtr   pPicture )
{
    BSTD_UNUSED(hTntd);
    BSTD_UNUSED(pPicture);
    return;
}

void BVDC_P_Tntd_SetEnable_isr
    ( BVDC_P_Tntd_Handle            hTntd,
      bool                          bEnable )
{
    BSTD_UNUSED(hTntd);
    BSTD_UNUSED(bEnable);
    return;
}

#endif /* #if !(BVDC_P_SUPPORT_TNTD) */

/* End of file. */
