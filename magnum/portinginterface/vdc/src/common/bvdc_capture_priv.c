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
#include "bvdc.h"
#include "brdc.h"
#include "bkni.h"
#include "bvdc_priv.h"
#include "bvdc_common_priv.h"
#include "bvdc_window_priv.h"
#include "bvdc_compositor_priv.h"
#include "bvdc_capture_priv.h"
#include "bvdc_buffer_priv.h"
#include "bvdc_vnet_priv.h"
#include "bvdc_window_priv.h"
#include "bvdc_source_priv.h"
#include "bvdc_feeder_priv.h"
#include "bchp_bmisc.h"
#include "bchp_timer.h"

#define BVDC_P_MAKE_CAP(pCapture, id)                                              \
{                                                                                  \
    (pCapture)->ulRegOffset = BCHP_CAP_##id##_REG_START - BCHP_CAP_0_REG_START;    \
    (pCapture)->ulResetMask = BCHP_BMISC_SW_INIT_CAP_##id##_MASK;                  \
}


BDBG_MODULE(BVDC_CAP);
BDBG_OBJECT_ID(BVDC_CAP);

/***************************************************************************
 * {private}
 *
 */
BERR_Code BVDC_P_Capture_Create
    ( BVDC_P_Capture_Handle           *phCapture,
      BRDC_Handle                      hRdc,
      BREG_Handle                      hRegister,
      BVDC_P_CaptureId                 eCaptureId,
#if (!BVDC_P_USE_RDC_TIMESTAMP)
      BTMR_TimerHandle                 hTimer,
#endif
      BVDC_P_Resource_Handle           hResource )
{
    BVDC_P_CaptureContext *pCapture;

    BDBG_ENTER(BVDC_P_Capture_Create);

    BDBG_ASSERT(phCapture);
    BDBG_ASSERT(hRegister);

    /* The handle will be NULL if create fails. */
    *phCapture = NULL;

    /* Alloc the context. */
    pCapture = (BVDC_P_CaptureContext*)
        (BKNI_Malloc(sizeof(BVDC_P_CaptureContext)));
    if(!pCapture)
    {
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }

    /* Clear out the context and set defaults. */
    BKNI_Memset((void*)pCapture, 0x0, sizeof(BVDC_P_CaptureContext));
    BDBG_OBJECT_SET(pCapture, BVDC_CAP);

    pCapture->eId                  = eCaptureId;
    pCapture->eTrig                = BRDC_Trigger_eCap0Trig0 + eCaptureId * 2;
    pCapture->hRegister            = hRegister;
    pCapture->hRdc                 = hRdc;
    pCapture->ulTimestamp          = 0;
#if (!BVDC_P_USE_RDC_TIMESTAMP)
    pCapture->hTimer               = hTimer;
    BTMR_GetTimerRegisters(pCapture->hTimer, &pCapture->stTimerReg);
#endif

    switch(eCaptureId)
    {
    case BVDC_P_CaptureId_eCap0:
        pCapture->ulRegOffset = 0;
        break;
#if BCHP_CAP_1_REG_START
    case BVDC_P_CaptureId_eCap1:
        BVDC_P_MAKE_CAP(pCapture, 1);
        break;
#endif

#if BCHP_CAP_2_REG_START
    case BVDC_P_CaptureId_eCap2:
        BVDC_P_MAKE_CAP(pCapture, 2);
        break;
#endif

#if BCHP_CAP_3_REG_START
    case BVDC_P_CaptureId_eCap3:
        BVDC_P_MAKE_CAP(pCapture, 3);
        break;
#endif

#if BCHP_CAP_4_REG_START
    case BVDC_P_CaptureId_eCap4:
        BVDC_P_MAKE_CAP(pCapture, 4);
        break;
#endif

#if BCHP_CAP_5_REG_START
    case BVDC_P_CaptureId_eCap5:
        BVDC_P_MAKE_CAP(pCapture, 5);
        break;
#endif

#if BCHP_CAP_6_REG_START
    case BVDC_P_CaptureId_eCap6:
        BVDC_P_MAKE_CAP(pCapture, 6);
        break;
#endif

#if BCHP_CAP_7_REG_START
    case BVDC_P_CaptureId_eCap7:
        BVDC_P_MAKE_CAP(pCapture, 7);
        break;
#endif
    default:
        BDBG_ASSERT(0);
    }

    /* Capture reset address */
    pCapture->ulResetRegAddr = BCHP_BMISC_SW_INIT;
    pCapture->ulResetMask    = BCHP_BMISC_SW_INIT_CAP_0_MASK << (pCapture->eId);

    /* init the SubRul sub-module */
    BVDC_P_SubRul_Init(&(pCapture->SubRul), BVDC_P_Capture_MuxAddr(pCapture),
        0, BVDC_P_DrainMode_eNone, 0, hResource);

    /* All done. now return the new fresh context to user. */
    *phCapture = (BVDC_P_Capture_Handle)pCapture;

    BDBG_LEAVE(BVDC_P_Capture_Create);
    return BERR_SUCCESS;
}


/***************************************************************************
 * {private}
 *
 */
void BVDC_P_Capture_Destroy
    ( BVDC_P_Capture_Handle            hCapture )
{
    BDBG_ENTER(BVDC_P_Capture_Destroy);
    BDBG_OBJECT_ASSERT(hCapture, BVDC_CAP);

    /* Turn off the capture. */
    hCapture->stRegs.ulCtrl &= ~(
#if (BVDC_P_SUPPORT_CAP_VER >= BVDC_P_CAP_VER_6)
        BCHP_MASK(CAP_0_CTRL, ENABLE_CTRL) |
#endif
        BCHP_MASK(CAP_0_CTRL, CAP_ENABLE));

    hCapture->stRegs.ulCtrl |=  (
#if (BVDC_P_SUPPORT_CAP_VER >= BVDC_P_CAP_VER_6)
        BCHP_FIELD_ENUM(CAP_0_CTRL, ENABLE_CTRL, ENABLE_BY_PICTURE) |
#endif
        BCHP_FIELD_ENUM(CAP_0_CTRL, CAP_ENABLE, DISABLE));

    BDBG_OBJECT_DESTROY(hCapture, BVDC_CAP);
    /* Release context in system memory */
    BKNI_Free((void*)hCapture);

    BDBG_LEAVE(BVDC_P_Capture_Destroy);
    return;
}


/***************************************************************************
 * {private}
 *
 */
void BVDC_P_Capture_Init
    ( BVDC_P_Capture_Handle            hCapture,
      BVDC_Window_Handle               hWindow )
{
#if (BVDC_P_SUPPORT_VIDEO_TESTFEATURE1_CAP_DCXM)
    uint32_t   ulReg;
#endif

    BDBG_ENTER(BVDC_P_Capture_Init);
    BDBG_OBJECT_ASSERT(hCapture, BVDC_CAP);

    hCapture->hWindow = hWindow;

    /* Clear out registers settings. */
    BKNI_Memset((void*)&hCapture->stRegs, 0x0, sizeof(BVDC_P_CaptureRegisterSetting));

    /* Initialize state. */
    hCapture->bInitial = true;
    hCapture->eCapDataMode = BVDC_P_Capture_DataMode_e8Bit422;

    hCapture->stDnSampler.eFilterType = BVDC_444To422Filter_eStandard;
    hCapture->stDnSampler.eRingRemoval = BVDC_RingSuppressionMode_eDisable;

#if (BVDC_P_SUPPORT_VIDEO_TESTFEATURE1_CAP_DCXM)
    ulReg = BREG_Read32(hCapture->hRegister, BCHP_CAP_0_HW_CONFIGURATION +
        hCapture->ulRegOffset);
    hWindow->bSupportDcxm = BCHP_GET_FIELD_DATA(ulReg, CAP_0_HW_CONFIGURATION, DCEM_EN);
    if(hWindow->bSupportDcxm)
    {
        /* CAP_0_DCEM_CFG */
        hCapture->stRegs.ulDcemCfg &= ~(
            BCHP_MASK(CAP_0_DCEM_CFG, ENABLE      ) |
#ifdef BCHP_CAP_0_DCEM_CFG_HALF_VBR_BFR_MODE_SHIFT
            BCHP_MASK(CAP_0_DCEM_CFG, HALF_VBR_BFR_MODE ) |
#endif
            BCHP_MASK(CAP_0_DCEM_CFG, APPLY_QERR  ) |
            BCHP_MASK(CAP_0_DCEM_CFG, FIXED_RATE  ) |
            BCHP_MASK(CAP_0_DCEM_CFG, COMPRESSION ));

        hCapture->stRegs.ulDcemCfg |=  (
            BCHP_FIELD_ENUM(CAP_0_DCEM_CFG, ENABLE,      Enable     ) |
#ifdef BCHP_CAP_0_DCEM_CFG_HALF_VBR_BFR_MODE_SHIFT
            BCHP_FIELD_ENUM(CAP_0_DCEM_CFG, HALF_VBR_BFR_MODE,  Disable) |
#endif
            BCHP_FIELD_ENUM(CAP_0_DCEM_CFG, APPLY_QERR,  Apply_Qerr ) |
            BCHP_FIELD_ENUM(CAP_0_DCEM_CFG, FIXED_RATE,  Fixed      ) |
            BCHP_FIELD_ENUM(CAP_0_DCEM_CFG, COMPRESSION, BPP_10     ));
    }
#else
        hWindow->bSupportDcxm = false;
#endif

    BDBG_LEAVE(BVDC_P_Capture_Init);
    return;
}

/***************************************************************************
 * {private}
 *
 * BVDC_P_Capture_BuildRul_DrainVnet_isr
 *
 * called by BVDC_P_Capture_BuildRul_isr after resetting to drain the module and
 * its pre-patch FreeCh or LpBack
 */
static void BVDC_P_Capture_BuildRul_DrainVnet_isr(
    BVDC_P_Capture_Handle              hCapture,
    BVDC_P_ListInfo                   *pList,
    bool                               bNoCoreReset)
{
    /* reset sub */
#if (BVDC_P_SUPPORT_CAP_VER >= BVDC_P_CAP_VER_6)
    BVDC_P_SubRul_Drain_isr(&(hCapture->SubRul), pList,
        bNoCoreReset?0:hCapture->ulResetRegAddr,
        bNoCoreReset?0:hCapture->ulResetMask,
        0, 0);
#else
    BVDC_P_SubRul_Drain_isr(&(hCapture->SubRul), pList,
        hCapture->ulResetRegAddr, hCapture->ulResetMask,
        0, 0);

    BSTD_UNUSED(bNoCoreReset);
#endif
}

/***************************************************************************
 * {private}
 *
 */
void BVDC_P_Capture_BuildRul_isr
    ( const BVDC_P_Capture_Handle      hCapture,
      BVDC_P_ListInfo                 *pList,
      BVDC_P_State                     eVnetState,
      const BVDC_P_PictureNodePtr      pPicture)
{
    uint32_t  ulRulOpsFlags;
    BVDC_P_PicComRulInfo  *pPicComRulInfo = &(pPicture->PicComRulInfo);

    BDBG_ENTER(BVDC_P_Capture_BuildRul_isr);
    BDBG_OBJECT_ASSERT(hCapture, BVDC_CAP);

    /* currently this is only for vnet building / tearing-off*/

    ulRulOpsFlags = BVDC_P_SubRul_GetOps_isr(
        &(hCapture->SubRul), pPicComRulInfo->eWin, eVnetState, pList->bLastExecuted);

    if ((0 == ulRulOpsFlags) ||
        (ulRulOpsFlags & BVDC_P_RulOp_eReleaseHandle))
        return;
    else if (ulRulOpsFlags & BVDC_P_RulOp_eDisable)
    {
        BVDC_P_SubRul_DropOffVnet_isr(&(hCapture->SubRul), pList);
        BVDC_P_Capture_SetEnable_isr(hCapture, false);

        /* Capture does not auto shut-off, need RDC to turn it off. */
        BVDC_P_CAP_WRITE_TO_RUL(CAP_0_CTRL, pList->pulCurrent, hCapture->stRegs.ulCtrl);
    }

    if (ulRulOpsFlags & BVDC_P_RulOp_eEnable)
    {
        /* Size */
#if (BVDC_P_SUPPORT_CAP_VER > BVDC_P_CAP_VER_4)
        BDBG_CASSERT(4 == (((BCHP_CAP_0_BVB_IN_SIZE - BCHP_CAP_0_PIC_SIZE) / sizeof(uint32_t)) + 1));
        *pList->pulCurrent++ = BRDC_OP_IMMS_TO_REGS(((BCHP_CAP_0_BVB_IN_SIZE - BCHP_CAP_0_PIC_SIZE) / sizeof(uint32_t)) + 1);
        *pList->pulCurrent++ = BRDC_REGISTER(BCHP_CAP_0_PIC_SIZE + hCapture->ulRegOffset);
        *pList->pulCurrent++ = hCapture->stRegs.ulPicSize;
        *pList->pulCurrent++ = hCapture->stRegs.ulPicOffset;
        *pList->pulCurrent++ = hCapture->stRegs.ulPicOffsetR;
        *pList->pulCurrent++ = hCapture->stRegs.ulBvbInSize;
#else
        BDBG_CASSERT(3 == (((BCHP_CAP_0_BVB_IN_SIZE - BCHP_CAP_0_PIC_SIZE) / sizeof(uint32_t)) + 1));
        *pList->pulCurrent++ = BRDC_OP_IMMS_TO_REGS(((BCHP_CAP_0_BVB_IN_SIZE - BCHP_CAP_0_PIC_SIZE) / sizeof(uint32_t)) + 1);
        *pList->pulCurrent++ = BRDC_REGISTER(BCHP_CAP_0_PIC_SIZE + hCapture->ulRegOffset);
        *pList->pulCurrent++ = hCapture->stRegs.ulPicSize;
        *pList->pulCurrent++ = hCapture->stRegs.ulPicOffset;
        *pList->pulCurrent++ = hCapture->stRegs.ulBvbInSize;
#endif

        /* Pitch */
        BVDC_P_CAP_WRITE_TO_RUL(CAP_0_PITCH, pList->pulCurrent, hCapture->stRegs.ulPitch);

        /* Address */
        BRDC_AddrRul_ImmToReg_isr(&pList->pulCurrent,
            BCHP_CAP_0_MSTART + hCapture->ulRegOffset, hCapture->stRegs.ullMStart);
        BRDC_AddrRul_ImmToReg_isr(&pList->pulCurrent,
            BCHP_CAP_0_MSTART_R + hCapture->ulRegOffset, hCapture->stRegs.ullMStartR);

        /* Mode etc */
        BDBG_CASSERT(6 == (((BCHP_CAP_0_BVB_TRIG_1_CFG - BCHP_CAP_0_MODE) / sizeof(uint32_t)) + 1));
        *pList->pulCurrent++ = BRDC_OP_IMMS_TO_REGS(((BCHP_CAP_0_BVB_TRIG_1_CFG - BCHP_CAP_0_MODE) / sizeof(uint32_t)) + 1);
        *pList->pulCurrent++ = BRDC_REGISTER(BCHP_CAP_0_MODE + hCapture->ulRegOffset);
        *pList->pulCurrent++ = hCapture->stRegs.ulMode;
        *pList->pulCurrent++ = hCapture->stRegs.ulCompOrder;
        *pList->pulCurrent++ = BCHP_FIELD_ENUM(CAP_0_RX_CTRL, PADDING_MODE, ENABLE);
        *pList->pulCurrent++ = hCapture->stRegs.ulTrigCtrl;
        *pList->pulCurrent++ = BCHP_FIELD_ENUM(CAP_0_BVB_TRIG_0_CFG, TRIG_CFG, END_OF_PICTURE);
        *pList->pulCurrent++ = BCHP_FIELD_DATA(CAP_0_BVB_TRIG_1_CFG, TRIG_CFG, 0);

#if (BVDC_P_SUPPORT_VIDEO_TESTFEATURE1_CAP_DCXM)
        /* Disable compression if not 10bit core. Default is enabled. */
        if(hCapture->hWindow->bSupportDcxm)
        {
#if (BVDC_P_SUPPORT_MOSAIC_MODE)
            uint32_t i, ulMosaicCount = pPicture->ulMosaicCount;
#endif

            BVDC_P_CAP_WRITE_TO_RUL(CAP_0_DCEM_CFG, pList->pulCurrent,
                hCapture->stRegs.ulDcemCfg);

#if (BVDC_P_SUPPORT_MOSAIC_MODE)
#if (!BVDC_P_CAP_SUPPORT_NEW_MEMORY_PITCH)
            if(hCapture->bEnableDcxm)
#endif
            {
                BDBG_CASSERT(3 == (((BCHP_CAP_0_DCEM_RECT_ID - BCHP_CAP_0_DCEM_RECT_CTRL) / sizeof(uint32_t)) + 1));
                *pList->pulCurrent++ = BRDC_OP_IMMS_TO_REGS(((BCHP_CAP_0_DCEM_RECT_ID - BCHP_CAP_0_DCEM_RECT_CTRL) / sizeof(uint32_t)) + 1);
                *pList->pulCurrent++ = BRDC_REGISTER(BCHP_CAP_0_DCEM_RECT_CTRL + hCapture->ulRegOffset);
                *pList->pulCurrent++ = hCapture->stRegs.ulDcemRectCtrl;
                *pList->pulCurrent++ = hCapture->stRegs.ulDcemRectMask;
                *pList->pulCurrent++ = hCapture->stRegs.ulDcemRectId;

                if(ulMosaicCount)  {
                    *pList->pulCurrent++ = BRDC_OP_IMMS_TO_REGS(ulMosaicCount);
                    *pList->pulCurrent++ = BRDC_REGISTER(
                        BCHP_CAP_0_DCEM_RECT_SIZEi_ARRAY_BASE + hCapture->ulRegOffset);
                    for(i = 0; i < ulMosaicCount; i++)
                    {
                        *pList->pulCurrent++ = hCapture->stRegs.ulDecmRect[i];
                    }

                    *pList->pulCurrent++ = BRDC_OP_IMMS_TO_REGS(ulMosaicCount);
                    *pList->pulCurrent++ = BRDC_REGISTER(
                        BCHP_CAP_0_DCEM_RECT_OFFSETi_ARRAY_BASE + hCapture->ulRegOffset);
                    for(i = 0; i < ulMosaicCount; i++)
                    {
                        *pList->pulCurrent++ =hCapture->stRegs.ulDecmRectOffset[i];
                    }

                }
            }
#endif
        }
#endif

#if (!BVDC_P_USE_RDC_TIMESTAMP)
        /* only the master RUL needs timestamp */
        if(pList->bMasterList)
        {
            /* Read and store timestamp */
            *pList->pulCurrent++ = BRDC_OP_REG_TO_VAR(BRDC_Variable_0);
            *pList->pulCurrent++ = BRDC_REGISTER(hCapture->stTimerReg.status);
            *pList->pulCurrent++ = BRDC_OP_IMM_TO_VAR(BRDC_Variable_1);
            *pList->pulCurrent++ = BCHP_TIMER_TIMER0_STAT_COUNTER_VAL_MASK;
            *pList->pulCurrent++ = BRDC_OP_VAR_AND_VAR_TO_VAR(BRDC_Variable_0, BRDC_Variable_1, BRDC_Variable_2);
            *pList->pulCurrent++ = BRDC_OP_VAR_TO_REG(BRDC_Variable_2);
            *pList->pulCurrent++ = BRDC_REGISTER(hCapture->ulTimestampRegAddr);
        }
#endif

        /* must be the last */
        BVDC_P_CAP_WRITE_TO_RUL(CAP_0_CTRL, pList->pulCurrent, hCapture->stRegs.ulCtrl);

        /* join in vnet after enable. note: its src mux is initialed as disabled */
        if (ulRulOpsFlags & BVDC_P_RulOp_eVnetInit)
        {
            BVDC_P_SubRul_JoinInVnet_isr(&(hCapture->SubRul), pList);
        }
    }

    else if (ulRulOpsFlags & BVDC_P_RulOp_eDrainVnet)
    {
        BVDC_P_Capture_BuildRul_DrainVnet_isr(hCapture, pList, pPicComRulInfo->bNoCoreReset);
    }

    BDBG_LEAVE(BVDC_P_Capture_BuildRul_isr);
    return;
}


/***************************************************************************
 * {private}
 *
 */
static BERR_Code BVDC_P_Capture_SetPictureRect_isr
    ( BVDC_P_Capture_Handle            hCapture,
      BVDC_P_Rect                     *pCapIn,
      BVDC_P_Rect                     *pCapOut,
      const BVDC_P_PictureNodePtr      pPicture,
      BAVC_Polarity                    eScanType )
{
    uint32_t ulWidth = pCapOut->ulWidth;
    uint32_t ulHeight = pCapOut->ulHeight;
    int32_t lLeft = pCapOut->lLeft;
    int32_t lTop = pCapOut->lTop;
    uint32_t ulCapInWidth = pCapIn->ulWidth;
    uint32_t ulCapInHeight = pCapIn->ulHeight;
#if (BVDC_P_SUPPORT_CAP_VER >= BVDC_P_CAP_VER_5)
    int32_t lLeft_R = pCapOut->lLeft_R;
#endif

    BDBG_ENTER(BVDC_P_Capture_SetPictureRect_isr);
    BDBG_OBJECT_ASSERT(hCapture, BVDC_CAP);

    /* Capture can not convert 3d to 2d, it's either in 3d mode or in 2d
     * mode. In the case eSrcOrientation is 3d, eDispOrientation is 2d,
     * set capture in 2d mode, and increase CAP_0_BVB_IN_SIZE to clip to
     * single view.
     */
    if(pPicture->eCapOrientation == BFMT_Orientation_e2D)
    {
        if(pPicture->eSrcOrientation == BFMT_Orientation_e3D_LeftRight)
            ulCapInWidth = 2*ulCapInWidth;
        else if(pPicture->eSrcOrientation == BFMT_Orientation_e3D_OverUnder)
            ulCapInHeight = 2*ulCapInHeight;
    }

    /* for even pixel align, we can only drop pixel, not expand pixel width */
    ulWidth = ulWidth & ~1;
    lLeft  = lLeft  & ~1;
    ulCapInWidth = ulCapInWidth & ~1;
#if (BVDC_P_SUPPORT_CAP_VER >= BVDC_P_CAP_VER_5)
    lLeft_R  = lLeft_R  & ~1;
#endif

    if(BAVC_Polarity_eFrame != eScanType)
    {
        ulHeight /= BVDC_P_FIELD_PER_FRAME;
        lTop /= BVDC_P_FIELD_PER_FRAME;
        ulCapInHeight /= BVDC_P_FIELD_PER_FRAME;
    }

    /* SW7445-2893/SW7445-2936 padding one line to delay eop. */
#if (BVDC_P_DCXM_CAP_PADDING_WORKAROUND)
    if(!pPicture->bMosaicMode && pPicture->bEnableDcxm)
    {
        ulCapInHeight += BVDC_P_DCXM_CAP_PADDING_WORKAROUND;
        ulHeight += BVDC_P_DCXM_CAP_PADDING_WORKAROUND;
    }
#endif

    /* set capture size */
    hCapture->stRegs.ulPicSize &= ~(
        BCHP_MASK(CAP_0_PIC_SIZE, HSIZE) |
        BCHP_MASK(CAP_0_PIC_SIZE, VSIZE));

    hCapture->stRegs.ulPicSize |=  (
        BCHP_FIELD_DATA(CAP_0_PIC_SIZE, HSIZE, ulWidth) |
        BCHP_FIELD_DATA(CAP_0_PIC_SIZE, VSIZE, ulHeight));

    hCapture->stRegs.ulPicOffset &= ~(
        BCHP_MASK(CAP_0_PIC_OFFSET, HSIZE) |
        BCHP_MASK(CAP_0_PIC_OFFSET, VSIZE));

    hCapture->stRegs.ulPicOffset |=  (
        BCHP_FIELD_DATA(CAP_0_PIC_OFFSET, HSIZE, lLeft) |
        BCHP_FIELD_DATA(CAP_0_PIC_OFFSET, VSIZE, lTop));

#if (BVDC_P_SUPPORT_CAP_VER >= BVDC_P_CAP_VER_5)
    hCapture->stRegs.ulPicOffsetR &= ~(
        BCHP_MASK(CAP_0_PIC_OFFSET_R, ENABLE) |
        BCHP_MASK(CAP_0_PIC_OFFSET_R, HSIZE));

    hCapture->stRegs.ulPicOffsetR |=  (
        BCHP_FIELD_DATA(CAP_0_PIC_OFFSET_R, ENABLE, (lLeft != lLeft_R)) |
        BCHP_FIELD_DATA(CAP_0_PIC_OFFSET_R, HSIZE,  lLeft_R));
#endif

    hCapture->stRegs.ulBvbInSize &= ~(
        BCHP_MASK(CAP_0_BVB_IN_SIZE, HSIZE) |
        BCHP_MASK(CAP_0_BVB_IN_SIZE, VSIZE));

    hCapture->stRegs.ulBvbInSize |=  (
        BCHP_FIELD_DATA(CAP_0_BVB_IN_SIZE, HSIZE, ulCapInWidth) |
        BCHP_FIELD_DATA(CAP_0_BVB_IN_SIZE, VSIZE, ulCapInHeight));

#if (BVDC_P_SUPPORT_VIDEO_TESTFEATURE1_CAP_DCXM && BVDC_P_SUPPORT_MOSAIC_MODE)
    /* Disable compression if not 10bit core. Default is enabled. */
    if(hCapture->hWindow->bSupportDcxm)
    {
        uint32_t i, ulMosaicCount = pPicture->ulMosaicCount;
        bool   bInterlaced;

        bInterlaced = (hCapture->eCapturePolarity != BAVC_Polarity_eFrame);

        for(i = 0; i < ulMosaicCount; i++)
        {
            hCapture->stRegs.ulDecmRect[i]=
                BCHP_FIELD_DATA(CAP_0_DCEM_RECT_SIZEi, HSIZE,
                    pPicture->astMosaicRect[i].ulWidth) |
                BCHP_FIELD_DATA(CAP_0_DCEM_RECT_SIZEi, VSIZE,
                    pPicture->astMosaicRect[i].ulHeight >> bInterlaced);

            hCapture->stRegs.ulDecmRectOffset[i] =
                 BCHP_FIELD_DATA(CAP_0_DCEM_RECT_OFFSETi, X_OFFSET,
                    pPicture->astMosaicRect[i].lLeft) |
                BCHP_FIELD_DATA(CAP_0_DCEM_RECT_OFFSETi, Y_OFFSET,
                    pPicture->astMosaicRect[i].lTop >> bInterlaced);
        }
    }
#endif
    BDBG_LEAVE(BVDC_P_Capture_SetPictureRect_isr);
    return BERR_SUCCESS;
}


/***************************************************************************
 * {private}
 *
 */
BERR_Code BVDC_P_Capture_SetBuffer_isr
    ( BVDC_P_Capture_Handle            hCapture,
      BMMA_DeviceOffset                ullDeviceAddr,
      BMMA_DeviceOffset                ullDeviceAddr_R,
      uint32_t                         ulPitch )
{
    BDBG_ENTER(BVDC_P_Capture_SetBuffer_isr);
    BDBG_OBJECT_ASSERT(hCapture, BVDC_CAP);

#if (BVDC_P_SUPPORT_CAP_VER < BVDC_P_CAP_VER_4)
    BSTD_UNUSED(ullDeviceAddr_R);
#endif

    /* This should always be true! */
#if (!BVDC_P_CAP_SUPPORT_NEW_MEMORY_PITCH)
    BDBG_ASSERT(BVDC_P_IS_ALIGN(ulPitch, BVDC_P_PITCH_ALIGN));
#endif
    BDBG_ASSERT(BVDC_P_IS_ALIGN(ullDeviceAddr, BVDC_P_BUFFER_ALIGN));

    /* set mstart */
    hCapture->stRegs.ullMStart &= ~(
        BCHP_MASK(CAP_0_MSTART, MSTART));

    hCapture->stRegs.ullMStart |=  (
        BCHP_FIELD_DATA(CAP_0_MSTART, MSTART, ullDeviceAddr));

#if (BVDC_P_SUPPORT_CAP_VER >= BVDC_P_CAP_VER_4)
    /* set mstart */
    hCapture->stRegs.ullMStartR &= ~(
        BCHP_MASK(CAP_0_MSTART_R, MSTART));

    hCapture->stRegs.ullMStartR |=  (
        BCHP_FIELD_DATA(CAP_0_MSTART_R, MSTART, ullDeviceAddr_R));
#endif

    /* set pitch */
    hCapture->stRegs.ulPitch &= ~(
        BCHP_MASK(CAP_0_PITCH, PITCH));

    hCapture->stRegs.ulPitch |=  (
        BCHP_FIELD_DATA(CAP_0_PITCH, PITCH, ulPitch));


    BDBG_LEAVE(BVDC_P_Capture_SetBuffer_isr);
    return BERR_SUCCESS;
}


/***************************************************************************
 * {private}
 * This replaces the deprecated BVDC_P_Capture_SetPacking_isr.
 */
static BERR_Code BVDC_P_Capture_SetPixelFormat_isr
    ( BVDC_P_Capture_Handle            hCapture,
      const BVDC_P_PictureNodePtr      pPicture )
{
    BPXL_Format    ePxlFormat;

    BDBG_ENTER(BVDC_P_Capture_SetPixelFormat_isr);
    BDBG_OBJECT_ASSERT(hCapture, BVDC_CAP);

    ePxlFormat = pPicture->ePixelFormat;

#if (BVDC_P_CAP_VER_7 > BVDC_P_SUPPORT_CAP_VER)
    /* set byte order */
    hCapture->stRegs.ulCompOrder &= ~(
        BCHP_MASK(CAP_0_BYTE_ORDER, BYTE_3_SEL) |
        BCHP_MASK(CAP_0_BYTE_ORDER, BYTE_2_SEL) |
        BCHP_MASK(CAP_0_BYTE_ORDER, BYTE_1_SEL) |
        BCHP_MASK(CAP_0_BYTE_ORDER, BYTE_0_SEL));

    /* Always program CAP_0_BYTE_ORDER according to pixel format */
    switch(ePxlFormat)
    {
    default:
    case BPXL_eY18_Cb8_Y08_Cr8:
        hCapture->stRegs.ulCompOrder |=  (
            BCHP_FIELD_ENUM(CAP_0_BYTE_ORDER, BYTE_3_SEL, CR) |
            BCHP_FIELD_ENUM(CAP_0_BYTE_ORDER, BYTE_2_SEL, Y0) |
            BCHP_FIELD_ENUM(CAP_0_BYTE_ORDER, BYTE_1_SEL, CB) |
            BCHP_FIELD_ENUM(CAP_0_BYTE_ORDER, BYTE_0_SEL, Y1));
        break;

    case BPXL_eY08_Cb8_Y18_Cr8:
        hCapture->stRegs.ulCompOrder |=  (
            BCHP_FIELD_ENUM(CAP_0_BYTE_ORDER, BYTE_3_SEL, CR) |
            BCHP_FIELD_ENUM(CAP_0_BYTE_ORDER, BYTE_2_SEL, Y1) |
            BCHP_FIELD_ENUM(CAP_0_BYTE_ORDER, BYTE_1_SEL, CB) |
            BCHP_FIELD_ENUM(CAP_0_BYTE_ORDER, BYTE_0_SEL, Y0));
        break;

    case BPXL_eY18_Cr8_Y08_Cb8:
        hCapture->stRegs.ulCompOrder |=  (
            BCHP_FIELD_ENUM(CAP_0_BYTE_ORDER, BYTE_3_SEL, CB) |
            BCHP_FIELD_ENUM(CAP_0_BYTE_ORDER, BYTE_2_SEL, Y0) |
            BCHP_FIELD_ENUM(CAP_0_BYTE_ORDER, BYTE_1_SEL, CR) |
            BCHP_FIELD_ENUM(CAP_0_BYTE_ORDER, BYTE_0_SEL, Y1));
        break;

    case BPXL_eY08_Cr8_Y18_Cb8:
        hCapture->stRegs.ulCompOrder |=  (
            BCHP_FIELD_ENUM(CAP_0_BYTE_ORDER, BYTE_3_SEL, CB) |
            BCHP_FIELD_ENUM(CAP_0_BYTE_ORDER, BYTE_2_SEL, Y1) |
            BCHP_FIELD_ENUM(CAP_0_BYTE_ORDER, BYTE_1_SEL, CR) |
            BCHP_FIELD_ENUM(CAP_0_BYTE_ORDER, BYTE_0_SEL, Y0));
            break;

    case BPXL_eCr8_Y18_Cb8_Y08:
        hCapture->stRegs.ulCompOrder |=  (
            BCHP_FIELD_ENUM(CAP_0_BYTE_ORDER, BYTE_3_SEL, Y0) |
            BCHP_FIELD_ENUM(CAP_0_BYTE_ORDER, BYTE_2_SEL, CB) |
            BCHP_FIELD_ENUM(CAP_0_BYTE_ORDER, BYTE_1_SEL, Y1) |
            BCHP_FIELD_ENUM(CAP_0_BYTE_ORDER, BYTE_0_SEL, CR));
        break;

    case BPXL_eCr8_Y08_Cb8_Y18:
        hCapture->stRegs.ulCompOrder |=  (
            BCHP_FIELD_ENUM(CAP_0_BYTE_ORDER, BYTE_3_SEL, Y1) |
            BCHP_FIELD_ENUM(CAP_0_BYTE_ORDER, BYTE_2_SEL, CB) |
            BCHP_FIELD_ENUM(CAP_0_BYTE_ORDER, BYTE_1_SEL, Y0) |
            BCHP_FIELD_ENUM(CAP_0_BYTE_ORDER, BYTE_0_SEL, CR));
            break;

    case BPXL_eCb8_Y18_Cr8_Y08:
        hCapture->stRegs.ulCompOrder |=  (
            BCHP_FIELD_ENUM(CAP_0_BYTE_ORDER, BYTE_3_SEL, Y0) |
            BCHP_FIELD_ENUM(CAP_0_BYTE_ORDER, BYTE_2_SEL, CR) |
            BCHP_FIELD_ENUM(CAP_0_BYTE_ORDER, BYTE_1_SEL, Y1) |
            BCHP_FIELD_ENUM(CAP_0_BYTE_ORDER, BYTE_0_SEL, CB));
        break;

    case BPXL_eCb8_Y08_Cr8_Y18:
        hCapture->stRegs.ulCompOrder |=  (
            BCHP_FIELD_ENUM(CAP_0_BYTE_ORDER, BYTE_3_SEL, Y1) |
            BCHP_FIELD_ENUM(CAP_0_BYTE_ORDER, BYTE_2_SEL, CR) |
            BCHP_FIELD_ENUM(CAP_0_BYTE_ORDER, BYTE_1_SEL, Y0) |
            BCHP_FIELD_ENUM(CAP_0_BYTE_ORDER, BYTE_0_SEL, CB));
        break;
    }
#else
    /* set byte order */
    hCapture->stRegs.ulCompOrder &= ~(
        BCHP_MASK(CAP_0_COMP_ORDER, COMP_3_SEL) |
        BCHP_MASK(CAP_0_COMP_ORDER, COMP_2_SEL) |
        BCHP_MASK(CAP_0_COMP_ORDER, COMP_1_SEL) |
        BCHP_MASK(CAP_0_COMP_ORDER, COMP_0_SEL));

    /* Always program CAP_0_COMP_ORDER according to pixel format */
    switch(ePxlFormat)
    {
    default:
    case BPXL_eY18_Cb8_Y08_Cr8:
        hCapture->stRegs.ulCompOrder |=  (
            BCHP_FIELD_ENUM(CAP_0_COMP_ORDER, COMP_3_SEL, CR) |
            BCHP_FIELD_ENUM(CAP_0_COMP_ORDER, COMP_2_SEL, Y0) |
            BCHP_FIELD_ENUM(CAP_0_COMP_ORDER, COMP_1_SEL, CB) |
            BCHP_FIELD_ENUM(CAP_0_COMP_ORDER, COMP_0_SEL, Y1));
        break;

    case BPXL_eY08_Cb8_Y18_Cr8:
        hCapture->stRegs.ulCompOrder |=  (
            BCHP_FIELD_ENUM(CAP_0_COMP_ORDER, COMP_3_SEL, CR) |
            BCHP_FIELD_ENUM(CAP_0_COMP_ORDER, COMP_2_SEL, Y1) |
            BCHP_FIELD_ENUM(CAP_0_COMP_ORDER, COMP_1_SEL, CB) |
            BCHP_FIELD_ENUM(CAP_0_COMP_ORDER, COMP_0_SEL, Y0));
        break;

    case BPXL_eY18_Cr8_Y08_Cb8:
        hCapture->stRegs.ulCompOrder |=  (
            BCHP_FIELD_ENUM(CAP_0_COMP_ORDER, COMP_3_SEL, CB) |
            BCHP_FIELD_ENUM(CAP_0_COMP_ORDER, COMP_2_SEL, Y0) |
            BCHP_FIELD_ENUM(CAP_0_COMP_ORDER, COMP_1_SEL, CR) |
            BCHP_FIELD_ENUM(CAP_0_COMP_ORDER, COMP_0_SEL, Y1));
        break;

    case BPXL_eY08_Cr8_Y18_Cb8:
        hCapture->stRegs.ulCompOrder |=  (
            BCHP_FIELD_ENUM(CAP_0_COMP_ORDER, COMP_3_SEL, CB) |
            BCHP_FIELD_ENUM(CAP_0_COMP_ORDER, COMP_2_SEL, Y1) |
            BCHP_FIELD_ENUM(CAP_0_COMP_ORDER, COMP_1_SEL, CR) |
            BCHP_FIELD_ENUM(CAP_0_COMP_ORDER, COMP_0_SEL, Y0));
            break;

    case BPXL_eCr8_Y18_Cb8_Y08:
        hCapture->stRegs.ulCompOrder |=  (
            BCHP_FIELD_ENUM(CAP_0_COMP_ORDER, COMP_3_SEL, Y0) |
            BCHP_FIELD_ENUM(CAP_0_COMP_ORDER, COMP_2_SEL, CB) |
            BCHP_FIELD_ENUM(CAP_0_COMP_ORDER, COMP_1_SEL, Y1) |
            BCHP_FIELD_ENUM(CAP_0_COMP_ORDER, COMP_0_SEL, CR));
        break;

    case BPXL_eCr8_Y08_Cb8_Y18:
        hCapture->stRegs.ulCompOrder |=  (
            BCHP_FIELD_ENUM(CAP_0_COMP_ORDER, COMP_3_SEL, Y1) |
            BCHP_FIELD_ENUM(CAP_0_COMP_ORDER, COMP_2_SEL, CB) |
            BCHP_FIELD_ENUM(CAP_0_COMP_ORDER, COMP_1_SEL, Y0) |
            BCHP_FIELD_ENUM(CAP_0_COMP_ORDER, COMP_0_SEL, CR));
            break;

    case BPXL_eCb8_Y18_Cr8_Y08:
        hCapture->stRegs.ulCompOrder |=  (
            BCHP_FIELD_ENUM(CAP_0_COMP_ORDER, COMP_3_SEL, Y0) |
            BCHP_FIELD_ENUM(CAP_0_COMP_ORDER, COMP_2_SEL, CR) |
            BCHP_FIELD_ENUM(CAP_0_COMP_ORDER, COMP_1_SEL, Y1) |
            BCHP_FIELD_ENUM(CAP_0_COMP_ORDER, COMP_0_SEL, CB));
        break;

    case BPXL_eCb8_Y08_Cr8_Y18:
        hCapture->stRegs.ulCompOrder |=  (
            BCHP_FIELD_ENUM(CAP_0_COMP_ORDER, COMP_3_SEL, Y1) |
            BCHP_FIELD_ENUM(CAP_0_COMP_ORDER, COMP_2_SEL, CR) |
            BCHP_FIELD_ENUM(CAP_0_COMP_ORDER, COMP_1_SEL, Y0) |
            BCHP_FIELD_ENUM(CAP_0_COMP_ORDER, COMP_0_SEL, CB));
        break;
    }
#endif

    BDBG_LEAVE(BVDC_P_Capture_SetPixelFormat_isr);
    return BERR_SUCCESS;
}


#if (BVDC_P_SUPPORT_CAP_VER >= BVDC_P_CAP_VER_4)
/***************************************************************************
 * {private}
 */
static BERR_Code BVDC_P_Capture_SetMode_isr
    ( BVDC_P_Capture_Handle            hCapture,
      const BVDC_P_PictureNodePtr      pPicture )
{
    BDBG_ENTER(BVDC_P_Capture_SetMode_isr);
    BDBG_OBJECT_ASSERT(hCapture, BVDC_CAP);

    /* CAP_0_MODE */
    hCapture->stRegs.ulMode &= ~(
#if (BVDC_P_SUPPORT_CAP_VER >= BVDC_P_CAP_VER_7)
        BCHP_MASK(CAP_0_MODE, PIXEL_MODE) |
#endif
        BCHP_MASK(CAP_0_MODE, MEM_VIDEO) |
        BCHP_MASK(CAP_0_MODE, BVB_VIDEO));

    /* Always use dual pointer for 3D mode */
    if(pPicture->eCapOrientation == BFMT_Orientation_e2D)
    {
        hCapture->stRegs.ulMode |=  (
#if (BVDC_P_SUPPORT_CAP_VER >= BVDC_P_CAP_VER_7)
            BCHP_FIELD_DATA(CAP_0_MODE, PIXEL_MODE, pPicture->bEnable10Bit) |
#endif
            BCHP_FIELD_ENUM(CAP_0_MODE, MEM_VIDEO, MODE_2D) |
            BCHP_FIELD_ENUM(CAP_0_MODE, BVB_VIDEO, MODE_2D));
    }
    else
    {
        hCapture->stRegs.ulMode |=  (
#if (BVDC_P_SUPPORT_CAP_VER >= BVDC_P_CAP_VER_7)
            BCHP_FIELD_DATA(CAP_0_MODE, PIXEL_MODE, pPicture->bEnable10Bit) |
#endif
            BCHP_FIELD_ENUM(CAP_0_MODE, MEM_VIDEO, MODE_3D_DUAL_POINTER)    |
            BCHP_FIELD_DATA(CAP_0_MODE, BVB_VIDEO, pPicture->eCapOrientation));
    }

    BDBG_LEAVE(BVDC_P_Capture_SetMode_isr);
    return BERR_SUCCESS;
}
#endif

/***************************************************************************
 * {private}
 * TODO: only support line_compare type non-mask mode now;
 *
 */
static BERR_Code BVDC_P_Capture_SetTrigger_isr
    ( BVDC_P_Capture_Handle            hCapture,
      BVDC_P_CapTriggerType            eTrigType )
{
    BDBG_ENTER(BVDC_P_Capture_SetTrigger_isr);
    BDBG_OBJECT_ASSERT(hCapture, BVDC_CAP);
#if (BVDC_P_CAP_VER_7 > BVDC_P_SUPPORT_CAP_VER)
    /* Turn off both capture trig0/1. */
    hCapture->stRegs.ulTrigCtrl &= ~(
        BCHP_MASK(CAP_0_TRIG_CTRL, TRIG_0_SEL) |
        BCHP_MASK(CAP_0_TRIG_CTRL, TRIG_1_SEL));
#else
    hCapture->stRegs.ulTrigCtrl &= ~(
        BCHP_MASK(CAP_0_TRIG_CTRL, TRIG_0) |
        BCHP_MASK(CAP_0_TRIG_CTRL, TRIG_1));
#endif
    if(BVDC_P_CapTriggerType_eBvbField == eTrigType)
    {
#if (BVDC_P_CAP_VER_7 > BVDC_P_SUPPORT_CAP_VER)
        /* turn on trig 0 */
        hCapture->stRegs.ulTrigCtrl |=  (
            BCHP_FIELD_DATA(CAP_0_TRIG_CTRL, TRIG_0_SEL, eTrigType));
#else
        hCapture->stRegs.ulTrigCtrl |=  (
            BCHP_FIELD_ENUM(CAP_0_TRIG_CTRL, TRIG_0, ENABLE));
#endif
    }

    BDBG_LEAVE(BVDC_P_Capture_SetTrigger_isr);
    return BERR_SUCCESS;
}

/***************************************************************************
 * {private}
 *
 */
BERR_Code BVDC_P_Capture_SetEnable_isr
    ( BVDC_P_Capture_Handle            hCapture,
      bool                             bEnable )
{
    BDBG_ENTER(BVDC_P_Capture_SetEnable_isr);
    BDBG_OBJECT_ASSERT(hCapture, BVDC_CAP);

    /* Turn on/off the capture. */
    hCapture->stRegs.ulCtrl &= ~(
#if (BVDC_P_SUPPORT_CAP_VER >= BVDC_P_CAP_VER_6)
        BCHP_MASK(CAP_0_CTRL, ENABLE_CTRL) |
#endif
        BCHP_MASK(CAP_0_CTRL, CAP_ENABLE));

    if(bEnable)
    {
        hCapture->stRegs.ulCtrl |=  (
#if (BVDC_P_SUPPORT_CAP_VER >= BVDC_P_CAP_VER_6)
            BCHP_FIELD_ENUM(CAP_0_CTRL, ENABLE_CTRL, ENABLE_BY_PICTURE) |
#endif
            BCHP_FIELD_ENUM(CAP_0_CTRL, CAP_ENABLE, ENABLE));
    }
    else
    {
        hCapture->stRegs.ulCtrl |=  (
#if (BVDC_P_SUPPORT_CAP_VER >= BVDC_P_CAP_VER_6)
            BCHP_FIELD_ENUM(CAP_0_CTRL, ENABLE_CTRL, ENABLE_BY_PICTURE) |
#endif
            BCHP_FIELD_ENUM(CAP_0_CTRL, CAP_ENABLE, DISABLE));
        BVDC_P_Capture_SetTrigger_isr(hCapture, BVDC_P_CapTriggerType_eDisable);
    }

    BDBG_LEAVE(BVDC_P_Capture_SetEnable_isr);
    return BERR_SUCCESS;
}

#if (BVDC_P_SUPPORT_MOSAIC_MODE)
#if (BVDC_P_SUPPORT_VIDEO_TESTFEATURE1_CAP_DCXM)
/***************************************************************************
 * {private}
 *
 */
static BERR_Code BVDC_P_Capture_SetDcxmMosaicRect_isr
    ( BVDC_P_Capture_Handle            hCapture,
      const BVDC_P_PictureNodePtr      pPicture,
      uint32_t                         ulMosaicIdx,
      bool                             bEnable )
{
    uint32_t ulMosaicCount = pPicture->ulMosaicCount;

    if(bEnable)
    {
        /* CAP_0_DCEM_RECT_CTRL */
        hCapture->stRegs.ulDcemRectCtrl &= ~(
            BCHP_MASK(CAP_0_DCEM_RECT_CTRL, RECT_ENABLE));
        hCapture->stRegs.ulDcemRectCtrl |= (
            BCHP_FIELD_ENUM(CAP_0_DCEM_RECT_CTRL, RECT_ENABLE, ENABLE));

        /* CAP_0_DCEM_RECT_ENABLE_MASK */
        hCapture->stRegs.ulDcemRectMask = (
            BCHP_FIELD_DATA(CAP_0_DCEM_RECT_ENABLE_MASK, RECT_ENABLE_MASK,
                (1<<ulMosaicCount) - 1));

        /* CAP_0_DCEM_RECT_ID */
        hCapture->stRegs.ulDcemRectId = (
            BCHP_FIELD_DATA(CAP_0_DCEM_RECT_ID, REC_CURR_ID, ulMosaicIdx));
    }
    else
    {
        /* CAP_0_DCEM_RECT_ID */
        hCapture->stRegs.ulDcemRectId = (
            BCHP_FIELD_DATA(CAP_0_DCEM_RECT_ID, REC_CURR_ID, 0));

        /* CAP_0_DCEM_RECT_ENABLE_MASK */
        hCapture->stRegs.ulDcemRectMask = 0;

        /* CAP_0_DCEM_RECT_CTRL */
        hCapture->stRegs.ulDcemRectCtrl = (
            BCHP_FIELD_ENUM(CAP_0_DCEM_RECT_CTRL, RECT_ENABLE, DISABLE));
    }

    return BERR_SUCCESS;
}
#endif

/***************************************************************************
 * {private}
 *
 */
static BERR_Code BVDC_P_Capture_GetMosaicRectAddr_isr
    ( BVDC_P_Capture_Handle            hCapture,
      const BVDC_P_PictureNodePtr      pPicture,
      uint32_t                         ulMosaicIdx,
      uint32_t                         ulPitch,
      BMMA_DeviceOffset               *pullStartAddr,
      BMMA_DeviceOffset               *pullStartAddr_R )
{
    BMMA_DeviceOffset      ullStartAddr, ullStartAddr_R;
    unsigned int  uiByteOffset;
    BAVC_Polarity   eCapturePolarity;

    BSTD_UNUSED(hCapture);

    BDBG_ASSERT(pullStartAddr);
    BDBG_ASSERT(pullStartAddr_R);

    eCapturePolarity = hCapture->eCapturePolarity;
    ullStartAddr = *pullStartAddr;
    ullStartAddr_R = *pullStartAddr_R;

    /* MosaicMode: calculate the starting address of sub-window capture; */
    BPXL_GetBytesPerNPixels_isr(pPicture->ePixelFormat,
        pPicture->astMosaicRect[ulMosaicIdx].lLeft,
        &uiByteOffset);

    ullStartAddr += ulPitch * (pPicture->astMosaicRect[ulMosaicIdx].lTop >>
         (eCapturePolarity != BAVC_Polarity_eFrame)) + uiByteOffset;
#if (BVDC_P_SUPPORT_CAP_VER >= BVDC_P_CAP_VER_4)
    ullStartAddr_R += ulPitch * (pPicture->astMosaicRect[ulMosaicIdx].lTop >>
         (eCapturePolarity != BAVC_Polarity_eFrame)) + uiByteOffset;
#endif

    *pullStartAddr = ullStartAddr;
    *pullStartAddr_R = ullStartAddr_R;

    return BERR_SUCCESS;
}
#endif

/***************************************************************************
 * {private}
 *
 */
BERR_Code BVDC_P_Capture_SetInfo_isr
    ( BVDC_P_Capture_Handle            hCapture,
      BVDC_Window_Handle               hWindow,
      const BVDC_P_PictureNodePtr      pPicture,
      uint32_t                         ulRectIdx,
      bool                             bLastPic )
{
    unsigned int uiPitch;
    uint32_t ulPitch, ulWidth;
    BMMA_DeviceOffset ullStartAddr, ullStartAddr_R = 0;
    BVDC_P_Rect  *pCapOut, *pCapIn;
    bool bDrain = false;
#if BVDC_P_SUPPORT_MOSAIC_MODE
    const BVDC_P_Window_Info *pUserInfo;
#endif

    BDBG_OBJECT_ASSERT(hCapture, BVDC_CAP);
    BDBG_ASSERT(BVDC_P_VNET_USED_CAPTURE(pPicture->stVnetMode));
    pCapIn  = pPicture->pCapIn;
    pCapOut = pPicture->pCapOut;
    BDBG_ASSERT(pCapIn->ulWidth  >= pCapOut->ulWidth  + pCapOut->lLeft);
    BDBG_ASSERT(pCapIn->ulHeight >= pCapOut->ulHeight + pCapOut->lTop);

#if BVDC_P_SUPPORT_MOSAIC_MODE
    pUserInfo = &hWindow->stCurInfo;
#endif
    hCapture->bEnableDcxm = pPicture->bEnableDcxm;

    /* compression is always enabled for 10bit */
    if(pPicture->bEnable10Bit)
        BDBG_ASSERT(pPicture->bEnableDcxm);

#if (BVDC_P_SUPPORT_VIDEO_TESTFEATURE1_CAP_DCXM)
    if(hCapture->hWindow->bSupportDcxm)
    {
#ifdef BCHP_CAP_0_DCEM_CFG_HALF_VBR_BFR_MODE_SHIFT
        bool  bHalfSizeBufMode;

        bHalfSizeBufMode =
            (pPicture->eSrcOrientation == BFMT_Orientation_e3D_LeftRight) ||
            (pPicture->eDispOrientation == BFMT_Orientation_e3D_LeftRight);
#endif

        /* CAP_0_DCEM_CFG */
        hCapture->stRegs.ulDcemCfg &= ~(
            BCHP_MASK(CAP_0_DCEM_CFG, ENABLE ) |
#ifdef BCHP_CAP_0_DCEM_CFG_HALF_VBR_BFR_MODE_SHIFT
            BCHP_MASK(CAP_0_DCEM_CFG, HALF_VBR_BFR_MODE ) |
#endif
            BCHP_MASK(CAP_0_DCEM_CFG, APPLY_QERR ) |
            BCHP_MASK(CAP_0_DCEM_CFG, FIXED_RATE ) |
            BCHP_MASK(CAP_0_DCEM_CFG, COMPRESSION ));
        hCapture->stRegs.ulDcemCfg |= (
            BCHP_FIELD_DATA(CAP_0_DCEM_CFG, ENABLE, pPicture->bEnableDcxm) |
#ifdef BCHP_CAP_0_DCEM_CFG_HALF_VBR_BFR_MODE_SHIFT
            BCHP_FIELD_DATA(CAP_0_DCEM_CFG, HALF_VBR_BFR_MODE, bHalfSizeBufMode) |
#endif
            BCHP_FIELD_ENUM(CAP_0_DCEM_CFG, APPLY_QERR,  Apply_Qerr) |
            BCHP_FIELD_DATA(CAP_0_DCEM_CFG, FIXED_RATE,  pPicture->bMosaicMode) |
            BCHP_FIELD_ENUM(CAP_0_DCEM_CFG, COMPRESSION, BPP_10 ));
    }
#endif

    /* Pitch is the total of bytes per line.
     * MosaicMode: the capture pitch is equal to the playback pitch! */
    ulWidth = pPicture->bMosaicMode ?
        ((pPicture->pVfdIn->ulWidth + 1) & ~0x1) : ((pCapOut->ulWidth + 1) & ~0x1);

#if (BVDC_P_CAP_SUPPORT_NEW_MEMORY_PITCH)
    if(pPicture->bMosaicMode)
    {
        if(pPicture->bEnableDcxm)
            ulPitch = (ulWidth * BVDC_P_DCXM_BITS_PER_PIXEL)/ 8;
        else
            BPXL_GetBytesPerNPixels_isr(pPicture->ePixelFormat, ulWidth, &ulPitch);
        ulPitch += (pPicture->eCapOrientation == BFMT_Orientation_e2D)
            ? BVDC_P_GUARD_MEMORY_2D(pPicture->ulMosaicCount)
            : BVDC_P_GUARD_MEMORY_3D(pPicture->ulMosaicCount);
    }
    else
    {
        BPXL_GetBytesPerNPixels_isr(pPicture->ePixelFormat, ulWidth, &uiPitch);
        ulPitch = BVDC_P_ALIGN_UP(uiPitch, BVDC_P_PITCH_ALIGN);
    }
#else
        BPXL_GetBytesPerNPixels_isr(pPicture->ePixelFormat, ulWidth, &uiPitch);
        ulPitch = BVDC_P_ALIGN_UP(uiPitch, BVDC_P_PITCH_ALIGN);
#endif

    hCapture->eCapturePolarity =
        (BVDC_P_VNET_USED_SCALER_AT_WRITER(pPicture->stVnetMode)
        ? pPicture->eDstPolarity : pPicture->eSrcPolarity);

#if BFMT_LEGACY_3DTV_SUPPORT
    /* SW workaround might forces eDstPolarity as eFrame in window for this 3D format */
    if(BFMT_IS_CUSTOM_1080P3D(hWindow->hCompositor->stCurInfo.pFmtInfo->eVideoFmt) &&
        BVDC_P_VNET_USED_SCALER_AT_WRITER(pPicture->stVnetMode))
    {
        hCapture->eCapturePolarity = BAVC_Polarity_eTopField;
    }
#endif

    BVDC_P_Capture_SetEnable_isr(hCapture, true);

    BVDC_P_Capture_SetPixelFormat_isr(hCapture, pPicture);

#if (BVDC_P_SUPPORT_CAP_VER >= BVDC_P_CAP_VER_4)
    BVDC_P_Capture_SetMode_isr(hCapture, pPicture);
#endif

    /* if source sends extra vbi pass-thru lines, we need to capture it also; */
#if (BVDC_P_CAP_SUPPORT_NEW_MEMORY_PITCH)
    if(hWindow->bSupportDcxm && pPicture->bMosaicMode)
    {
        BVDC_P_Capture_SetPictureRect_isr(hCapture, pCapIn,
            &hCapture->hWindow->stCurInfo.stScalerOutput, pPicture,
            hCapture->eCapturePolarity);
    }
    else
    {
        BVDC_P_Capture_SetPictureRect_isr(hCapture, pCapIn, pCapOut, pPicture,
            hCapture->eCapturePolarity);
    }
#elif (BVDC_P_SUPPORT_VIDEO_TESTFEATURE1_CAP_DCXM)
    if(pPicture->bEnableDcxm && pPicture->bMosaicMode)
    {
        BVDC_P_Capture_SetPictureRect_isr(hCapture, pCapIn,
            &hCapture->hWindow->stCurInfo.stScalerOutput, pPicture,
            hCapture->eCapturePolarity);
    }
    else
    {
        BVDC_P_Capture_SetPictureRect_isr(hCapture, pCapIn, pCapOut, pPicture, hCapture->eCapturePolarity);
    }
#else
    {
        BVDC_P_Capture_SetPictureRect_isr(hCapture, pCapIn, pCapOut, pPicture, hCapture->eCapturePolarity);
    }
#endif

    ullStartAddr = BVDC_P_Buffer_GetDeviceOffset(pPicture);

#if (BVDC_P_SUPPORT_CAP_VER >= BVDC_P_CAP_VER_4)
    if(hWindow->eBufAllocMode == BVDC_P_BufHeapAllocMode_eLRSeparate)
    {
        ullStartAddr_R = BVDC_P_Buffer_GetDeviceOffset_R(pPicture);
    }
    else if(hWindow->eBufAllocMode == BVDC_P_BufHeapAllocMode_eLRCombined)
    {
        uint32_t  ulBufHeapSize;

        BVDC_P_BufferHeap_GetHeapSizeById_isr(
            hWindow->hCapHeap,
            hWindow->eBufferHeapIdRequest, &ulBufHeapSize);

        ullStartAddr_R = ullStartAddr + ulBufHeapSize / 2;
    }
#endif

#if BVDC_P_SUPPORT_MOSAIC_MODE
    /* MosaicMode: calculate the starting address of sub-window capture; */
    if(pPicture->bMosaicMode)
    {
        /* for the chained sub-RULs, enable capture trigger; disable at last;
           note in simul mode, we still need to enable cap trig when current
           capture is the last mosaic for the current window in case the other
           window has more mosaics to capture, i.e. combo trig is enabled; */
        /* this is to assure the rest of the pictures list got drained; */
        /* don't overwrite captured window; drain the source to save bandwidth; */
        if(((ulRectIdx >= pPicture->ulMosaicCount) ||
           (!pPicture->abMosaicVisible[ulRectIdx])) && !pPicture->bEnableDcxm)
        {
            bDrain = true;
        }
        else
        {
            uint32_t  ulMosaicIdx = BVDC_P_MIN(ulRectIdx, pPicture->ulMosaicCount - 1);

#if (BVDC_P_CAP_SUPPORT_NEW_MEMORY_PITCH)
            if(hWindow->bSupportDcxm)
            {
                BVDC_P_Capture_SetDcxmMosaicRect_isr(hCapture,
                    pPicture, ulMosaicIdx, true);
            }
            else
            {
                BVDC_P_Capture_GetMosaicRectAddr_isr(hCapture, pPicture,
                    ulMosaicIdx, ulPitch, &ullStartAddr, &ullStartAddr_R);
            }

#elif (BVDC_P_SUPPORT_VIDEO_TESTFEATURE1_CAP_DCXM)
            if(pPicture->bEnableDcxm)
            {
                BVDC_P_Capture_SetDcxmMosaicRect_isr(hCapture,
                    pPicture, ulMosaicIdx, true);
            }
            else
            {
                BVDC_P_Capture_GetMosaicRectAddr_isr(hCapture, pPicture,
                    ulMosaicIdx, ulPitch, &ullStartAddr, &ullStartAddr_R);
            }
#else
            {
                BVDC_P_Capture_GetMosaicRectAddr_isr(hCapture, pPicture,
                    ulMosaicIdx, ulPitch, &ullStartAddr, &ullStartAddr_R);
            }
#endif
        }
    }
    else
    {
        /* this is to assure the rest of the pictures list got drained; */
        /* don't overwrite captured window; drain the source; */
#if (!BVDC_P_CAP_SUPPORT_NEW_MEMORY_PITCH)
        if(!pPicture->bMosaicMode && ulRectIdx && !pPicture->bEnableDcxm)
        {
            bDrain = true;
        }
#endif

#if (BVDC_P_SUPPORT_VIDEO_TESTFEATURE1_CAP_DCXM)
        if(pPicture->bEnableDcxm)
        {
            BVDC_P_Capture_SetDcxmMosaicRect_isr(hCapture,
                pPicture, ulRectIdx, false);
        }
#endif
    }

    if(pUserInfo->hSource->stCurInfo.bMosaicMode && !bLastPic)
    {
        BVDC_P_Capture_SetTrigger_isr(hCapture, BVDC_P_CapTriggerType_eBvbField);
    }
    else
    {
        BVDC_P_Capture_SetTrigger_isr(hCapture, BVDC_P_CapTriggerType_eDisable);
    }

    if(bDrain)
    {
        uint32_t   ulBvbHSize, ulBvbVSize;

        ullStartAddr   = hWindow->ullNullBufOffset;
        ullStartAddr_R = hWindow->ullNullBufOffset;

        hCapture->stRegs.ulPicSize =  (
            BCHP_FIELD_DATA(CAP_0_PIC_SIZE, HSIZE, 2) |
            BCHP_FIELD_DATA(CAP_0_PIC_SIZE, VSIZE, 1));
        /* clip the top/left region of input data to have capture engine to
           generate trigger at correct timing (end of input picture); */
        ulBvbHSize = BVDC_P_GET_FIELD(hCapture->stRegs.ulBvbInSize,
            CAP_0_BVB_IN_SIZE, HSIZE);
        ulBvbVSize = BVDC_P_GET_FIELD(hCapture->stRegs.ulBvbInSize,
            CAP_0_BVB_IN_SIZE, VSIZE);

        hCapture->stRegs.ulPicOffset =  (
            BCHP_FIELD_DATA(CAP_0_PIC_OFFSET, HSIZE, ulBvbHSize - 2) |
            BCHP_FIELD_DATA(CAP_0_PIC_OFFSET, VSIZE, ulBvbVSize - 1));
    }
#else
    BSTD_UNUSED(bLastPic);
    BSTD_UNUSED(ulRectIdx);
    BSTD_UNUSED(bDrain);
#endif

    BVDC_P_Capture_SetBuffer_isr(hCapture, ullStartAddr, ullStartAddr_R, ulPitch);

    return BERR_SUCCESS;
}

/* End of file. */
