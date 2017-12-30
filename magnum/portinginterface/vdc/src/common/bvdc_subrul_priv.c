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
 ***************************************************************************/
#include "bvdc_subrul_priv.h"
#include "bchp_vnet_f.h"
#include "bchp_vnet_b.h"
#include "bchp_mmisc.h"
#if (BVDC_P_SUPPORT_DMISC)
#include "bchp_dmisc.h"
#endif

BDBG_MODULE(VNET);

/***************************************************************************
 * {private}
 *
 * BVDC_P_SubRul_Init
 *
 * called by BVDC_P_*_Create to init BVDC_P_SubRul_Context
 */
void  BVDC_P_SubRul_Init(
    BVDC_P_SubRulContext          *pSubRul,
    uint32_t                       ulMuxAddr,
    uint32_t                       ulPostMuxValue,
    BVDC_P_DrainMode               eDrainMode,
    int32_t                        lStatisReadMark,
    BVDC_P_Resource_Handle         hResource )
{
    /* init the context */
    pSubRul->hResource = hResource;
    pSubRul->lAccumCntr = BVDC_P_SUBRUL_VNET_INIT_MARK;
    pSubRul->eBldWin = BVDC_P_WindowId_eUnknown;
    pSubRul->ulMuxAddr = ulMuxAddr;
    pSubRul->ulPostMuxValue = ulPostMuxValue;
    pSubRul->eDrainMode = eDrainMode;
    pSubRul->lStatisReadMark =
        (lStatisReadMark)? lStatisReadMark : 0x7ffffff0;

    /* all src mux will be set to be disabled by resetting bit VNET_B/F
     * in BCHP_MMISC_SOFT_RESET in BVDC_P_ResetBvn(pVdc) */
}


#if (BVDC_P_SUPPORT_BOX_DETECT)
/***************************************************************************
 * {private}
 *
 * BVDC_P_SubRul_ResetAccumCntr_isr
 *
 * Called by a module to reset AccumCntr to 0 for statistics cntr reset.
 */
void BVDC_P_SubRul_ResetAccumCntr_isr(
    BVDC_P_SubRulContext          *pSubRul )
{
    pSubRul->lAccumCntr = BVDC_P_MIN(0, pSubRul->lAccumCntr);
}
#endif


/***************************************************************************
 * {private}
 *
 * BVDC_P_SubRul_SetRulBuildWinId_isr
 *
 * Called by BVDC_P_Window_Writer_isr to set MFD's RUL BuildWinId to be the
 * last win connected to the MFD. Otherwise Mfd might start to feed right
 * after the 1st win gets enabled, then the following win might join in the
 * vnet after feeding started.
 */
void BVDC_P_SubRul_SetRulBuildWinId_isr(
    BVDC_P_SubRulContext          *pSubRul,
    BVDC_P_WindowId                eWin)
{
    pSubRul->eBldWin = eWin;
}

/***************************************************************************
 * {private}
 *
 * BVDC_P_SubRul_GetOps_isr
 *
 * Called by BVDC_P_*_BuildRul_isr to decide what operations to take with
 * this BVDC_P_*_BuildRul_isr call.
 *
 * IMPORTANT: we assume source calls windows to build RUL in the order from
 * smaller WinId to bigger WinId.
 *
 * Input:
 *   eVnetState - the window's reader or writer state
 */
uint32_t BVDC_P_SubRul_GetOps_isr(
    BVDC_P_SubRulContext          *pSubRul,
    BVDC_P_WindowId                eWin,
    BVDC_P_State                   eVnetState,
    bool                           bLastRulExecuted)
{
    uint32_t ulRulOpsFlags = 0;

    BDBG_ASSERT((BVDC_P_State_eCreate   != eVnetState) &&
                (BVDC_P_State_eDestroy  != eVnetState));

    if (BVDC_P_State_eInactive == eVnetState)
    {
        /* for window init stage, this could be WinOut or SCL */
        BDBG_MSG(("RulOp 0: pSubRul->ulMuxAddr %x, ->ulMuxValue %d, ->ulPatchMuxAddr: %x",
            pSubRul->ulMuxAddr, pSubRul->ulMuxValue, pSubRul->ulPatchMuxAddr));
        return ulRulOpsFlags;
    }

    if ((BVDC_P_State_eActive           == eVnetState) ||
        ((BVDC_P_State_eShutDownPending == eVnetState) &&
         (pSubRul->ulWinsActFlags & (1<<eWin))))  /* such as writer wait for reader down */
    {
        /* bit eWin in ulWinsActFlags indicates whether eWin is actively using this module */
        pSubRul->ulWinsActFlags |= (1<<eWin);

        /* it is the 1st active window that should build the active RUL */
        if ((BVDC_P_WindowId_eUnknown == pSubRul->eBldWin) || /* 1st, or RUL build win is off */
            (eWin == pSubRul->eBldWin))                       /* it built RUL last time */
        {
            pSubRul->eBldWin = eWin;

            if (BVDC_P_SUBRUL_STATE_VNET_INIT_CHECK(pSubRul->lAccumCntr)||
                BVDC_P_SUBRUL_STATE_STATIS_INIT_CHECK(pSubRul->lAccumCntr))
            {
                /* last time we build into RUL a complete hw init or a hw reset, if
                 * the RUL did get executed, we set lAccumCntr to 0 to indicate a
                 * fresh new subsub statistics, otherwise we need to bulid the complete
                 * thing (same as last time) into RUL again */
                if ( bLastRulExecuted )
                    pSubRul->lAccumCntr = 0;
                else
                    pSubRul->lAccumCntr --; /* return to init or reset state */
            }

            if ( BVDC_P_SUBRUL_STATE_STATIS_INIT(pSubRul->lAccumCntr) )
            {
                if ( BVDC_P_SUBRUL_STATE_VNET_INIT(pSubRul->lAccumCntr) )
                {
                    /* inform next GetOps_isr call to check whether init is sent to HW */
                    pSubRul->lAccumCntr = BVDC_P_SUBRUL_VNET_INIT_CHECK;

                    /* instruct to init for joinning in vnet, src size, ... */
                    ulRulOpsFlags |= BVDC_P_RulOp_eVnetInit;
                    BDBG_MSG(("RulOp Init: pSubRul->ulMuxAddr %x, ->ulMuxValue %d, ->ulPatchMuxAddr: %x",
                              pSubRul->ulMuxAddr, pSubRul->ulMuxValue, pSubRul->ulPatchMuxAddr));
                }
                else
                {
                    /* inform next SubRul_isr call to check whether reset is sent to HW,
                     * however, if statis should be read every vsync, then there is no
                     * need to check that, just inform next vsync to read again */
                    pSubRul->lAccumCntr = (1 < pSubRul->lStatisReadMark)?
                        BVDC_P_SUBRUL_STATIS_INIT_CHECK : BVDC_P_SUBRUL_STATIS_INIT_MARK;
                }

                /* instruct to reset statistics */
                ulRulOpsFlags |= BVDC_P_RulOp_eStatisInit;
            }
            else
            {
                /* normal case */
                pSubRul->lAccumCntr ++;

                /* check if neext vsync should read and reset statistics */
                pSubRul->lAccumCntr =
                    (pSubRul->lAccumCntr >= pSubRul->lStatisReadMark)?
                    BVDC_P_SUBRUL_STATIS_INIT_MARK : pSubRul->lAccumCntr;
            }

            /* instruct to enable the hw to accept pixel data */
            ulRulOpsFlags |= BVDC_P_RulOp_eEnable;
        }
    }
    else if (BVDC_P_State_eShutDownRul == eVnetState)
    {
        /* Note: It is the last shuting-down win that should build the shut-down RUL,
         * even if this is the 2nd time to build shut down RUL in the case that last
         * one did not execute. However, regardless of whether this module is still
         * used by other windows, this win will release this module when its related
         * reader or writer vnet are shut-down and drained.
         */

#if BDBG_DEBUG_BUILD
        if ((0 == (pSubRul->ulWinsActFlags & (1<<eWin))) && bLastRulExecuted)
        {
            /* eReaderState could be reset to eShutDownPending if ReConfig is requested
             * again when window just did a shut down for old vnet mode and have not
             * brought up the new vnet mode yet. This Reconfig request could be due to
             * a quick following ApplyChange or a src dynamic format change. If we do
             * darin as join-in vnet, we might be able to get rid of this situation */
            BDBG_MSG(("Repeated ShutDownRul state: pSubRul->ulMuxAddr %x, ->ulMuxValue %d, ->ulPatchMuxAddr: %x",
                      pSubRul->ulMuxAddr, pSubRul->ulMuxValue, pSubRul->ulPatchMuxAddr));
        }
#endif
        /* this hanldle is for modules such as BoxDetect that could be disabled without
         * vnet reconfigure */
        if ((0 == (pSubRul->ulWinsActFlags & (1 << eWin))) && (bLastRulExecuted))
        {
            /* disable rul executed for this win, instruct to release handle inside
             * BVDC_P_*_BuildRul_isr */
            ulRulOpsFlags |= BVDC_P_RulOp_eReleaseHandle;
            BDBG_MSG(("RulOp ReleaseHandle: pSubRul->ulMuxAddr %x, ->ulMuxValue %d, ->ulPatchMuxAddr: %x",
                      pSubRul->ulMuxAddr, pSubRul->ulMuxValue, pSubRul->ulPatchMuxAddr));
        }
        else
        {
            /* vnet for this win is in shuting down process */
            pSubRul->ulWinsActFlags &= ~(1<<eWin);

            if ((0 == pSubRul->ulWinsActFlags) &&
                ((BVDC_P_WindowId_eUnknown == pSubRul->eBldWin) || (eWin == pSubRul->eBldWin)))
            {
                /* this win is the last win that shuts down, build shut down and
                 * drain RUL.
                 * Note: if a module could be shared by multiple wins, SetRulBuildWinId
                 * in the setInfo round is needed. If a window with bigger winId is
                 * enabling it at this vsync, we don't want to disable the module
                 * here. */

                /* instruct to shut down this module and drain vnet */
                ulRulOpsFlags |= BVDC_P_RulOp_eDisable;

                /* if it is enabled again, need to re-init src */
                pSubRul->lAccumCntr = BVDC_P_SUBRUL_VNET_INIT_MARK;
                pSubRul->eBldWin = BVDC_P_WindowId_eUnknown;
                BDBG_MSG(("RulOp Disable: pSubRul->ulMuxAddr %x, ->ulMuxValue %d, ->ulPatchMuxAddr: %x",
                          pSubRul->ulMuxAddr, pSubRul->ulMuxValue, pSubRul->ulPatchMuxAddr));
            }
            else if ((0 != pSubRul->ulWinsActFlags) &&
                     ((BVDC_P_WindowId_eUnknown == pSubRul->eBldWin) || (eWin == pSubRul->eBldWin)))
            {
                if (0 == (pSubRul->ulWinsActFlags & ~((1 << eWin) - 1)))
                {
                    /* there is some window with smaller winId, but no more win with bigger winId,
                     * that are still actively using this module, this win should build an active
                     * RUL for the last time.
                     *
                     * IMPORTANT: we assume source calls windows to build RUL in an oder from
                     * small WinId to bigger WinId
                     */

                    /* instruct to enable the hw to accept pixel data */
                    ulRulOpsFlags |= BVDC_P_RulOp_eEnable;
                    pSubRul->lAccumCntr ++;
                }

                /* this win no-longer uses the model, but there is other win that still uses it,
                 * therefore this win should no longer drive the RUL build */
                pSubRul->eBldWin = BVDC_P_WindowId_eUnknown;
            }
        }

        /* else: ((0 == pSubRul->ulWinsActFlags) &&
         *        ((BVDC_P_WindowId_eUnknown != pSubRul->eBldWin) && (eWin != pSubRul->eBldWin))) ||
         *       ((0 != pSubRul->ulWinsActFlags) &&
         *        ((BVDC_P_WindowId_eUnknown != pSubRul->eBldWin) && (eWin != pSubRul->eBldWin)))
         * don't need to do anything here
         */
    }

    else if ((0 == pSubRul->ulWinsActFlags) &&
             (BVDC_P_State_eDrainVnet == eVnetState))
    {
        ulRulOpsFlags |= BVDC_P_RulOp_eDrainVnet;
        BDBG_MSG(("RulOp DrainVnet: pSubRul->ulMuxAddr %x, ->ulMuxValue %d, ->ulPatchMuxAddr: %x",
                  pSubRul->ulMuxAddr, pSubRul->ulMuxValue, pSubRul->ulPatchMuxAddr));
    }

    /* else
        ((BVDC_P_State_eShutDownPending == eVnetState) && !(pSubRul->ulWinsActFlags & (1<<eWin))) ||
        (((BVDC_P_State_eDrainVnet == eVnetState) || (BVDC_P_State_eShutDown == eVnetState)) &&
         (0 != pSubRul->ulWinsActFlags))
     * don't need to do anything here
     */

    return ulRulOpsFlags;
}


/***************************************************************************
 * {private}
 *
 * BVDC_P_SubRul_SetVnet_isr
 *
 * Called by BVDC_P_*_BuildRul_isr to setup for joinning into vnet
 *
 * input:
 *   ulSrcMuxValue - this module's new src mux value
 *   eVnetPatchMode - specify need pre-freeCh or pre-Lpback
 */
BERR_Code  BVDC_P_SubRul_SetVnet_isr(
    BVDC_P_SubRulContext          *pSubRul,
    uint32_t                       ulSrcMuxValue,
    BVDC_P_VnetPatch               eVnetPatchMode)
{
    uint32_t  ulFreeChHwId, ulLpBackHwId;
    BERR_Code  eResult;

    /* if the module is already active, we can not change its up-stream
     * without a shuting down process */
    BDBG_ASSERT((BVDC_P_SUBRUL_STATE_VNET_INIT(pSubRul->lAccumCntr)) ||
                ((eVnetPatchMode == pSubRul->eVnetPatchMode) &&
                 (ulSrcMuxValue == pSubRul->ulMuxValue)));

    /* don't need to do anything if this module already sets-up to join-in vnet
     * by another wiondow */
    if(BVDC_P_SUBRUL_STATE_VNET_INIT(pSubRul->lAccumCntr) &&
       (0 == pSubRul->ulPatchMuxAddr))
    {
        BDBG_ASSERT(BVDC_P_VnetPatch_eNone == pSubRul->eVnetPatchMode);
        if (BVDC_P_VnetPatch_eFreeCh == eVnetPatchMode)
        {
            /* acquire a free channel */
            eResult = BVDC_P_Resource_AcquireHwId_isr(pSubRul->hResource,
                BVDC_P_ResourceType_eFreeCh, 0, ulSrcMuxValue, &ulFreeChHwId, false);
            BDBG_MSG(("AcquireFreeCh: eType=%d, ulHwId=%d, Cntr=%d",
                BVDC_P_ResourceType_eFreeCh, ulFreeChHwId,
                BVDC_P_Resource_GetHwIdAcquireCntr_isr(pSubRul->hResource, BVDC_P_ResourceType_eFreeCh, ulFreeChHwId)));
            BDBG_ASSERT(BERR_SUCCESS == eResult);
            if (BERR_SUCCESS != eResult)
                return BERR_TRACE(eResult);

            pSubRul->ulPatchMuxAddr = BVDC_P_FreeCh_eId_To_MuxAddr(ulFreeChHwId);
        }
#ifdef BCHP_VNET_B_LOOPBACK_0_SRC
        else if (BVDC_P_VnetPatch_eLpBack == eVnetPatchMode)
        {
            /* acquire a loop back */
            eResult = BVDC_P_Resource_AcquireHwId_isr(pSubRul->hResource,
                BVDC_P_ResourceType_eLpBck, 0, ulSrcMuxValue, &ulLpBackHwId, false);
            BDBG_MSG(("AcquireLpBck: eType=%d, ulHwId=%d, Cntr=%d",
                BVDC_P_ResourceType_eLpBck, ulLpBackHwId,
                BVDC_P_Resource_GetHwIdAcquireCntr_isr(pSubRul->hResource, BVDC_P_ResourceType_eLpBck, ulLpBackHwId)));
            BDBG_ASSERT(BERR_SUCCESS == eResult);
            if (BERR_SUCCESS != eResult)
                return BERR_TRACE(eResult);

            pSubRul->ulPatchMuxAddr = BVDC_P_LpBack_eId_To_MuxAddr(ulLpBackHwId);
        }
#else
        BSTD_UNUSED(ulLpBackHwId);
#endif
        pSubRul->eVnetPatchMode = eVnetPatchMode;
        pSubRul->ulMuxValue = ulSrcMuxValue;
    }

    return BERR_SUCCESS;
}

/***************************************************************************
 * {private}
 *
 * BVDC_P_SubRul_UnsetVnet_isr
 *
 * Called by BVDC_P_*_UnsetVnet_isr to release the free-channel or
 * loop-back used to to patch the vnet.  It should be called only after
 * shut-down RUL is executed.
 */
void BVDC_P_SubRul_UnsetVnet_isr(
    BVDC_P_SubRulContext          *pSubRul)
{
    /* free the free-channel or loop-back that is used to patch the vnet */
    if ((0 == pSubRul->ulWinsActFlags) &&   /* no win is using this module */
        (BVDC_P_WindowId_eUnknown == pSubRul->eBldWin) &&
        (0 != pSubRul->ulPatchMuxAddr))        /* not freed yet */
    {
        BDBG_ASSERT(BVDC_P_SUBRUL_STATE_VNET_INIT(pSubRul->lAccumCntr));
        BDBG_ASSERT(BVDC_P_VnetPatch_eNone != pSubRul->eVnetPatchMode);

        if (BVDC_P_VnetPatch_eFreeCh == pSubRul->eVnetPatchMode)
        {
            BVDC_P_Resource_ReleaseHwId_isr(pSubRul->hResource, BVDC_P_ResourceType_eFreeCh,
                BVDC_P_FreeCh_MuxAddr_To_HwId(pSubRul->ulPatchMuxAddr));
            BDBG_MSG(("ReleaseHwIdFch: eType=%d, ulHwId=%u, Cntr=%d",
                BVDC_P_ResourceType_eFreeCh,
                (unsigned int)BVDC_P_FreeCh_MuxAddr_To_HwId(pSubRul->ulPatchMuxAddr),
                BVDC_P_Resource_GetHwIdAcquireCntr_isr(pSubRul->hResource,
                BVDC_P_ResourceType_eFreeCh,
                BVDC_P_FreeCh_MuxAddr_To_HwId(pSubRul->ulPatchMuxAddr))));
        }
#ifdef BCHP_VNET_B_LOOPBACK_0_SRC
        else if (BVDC_P_VnetPatch_eLpBack == pSubRul->eVnetPatchMode)
        {
            BVDC_P_Resource_ReleaseHwId_isr(pSubRul->hResource, BVDC_P_ResourceType_eLpBck,
                BVDC_P_LpBack_MuxAddr_To_HwId(pSubRul->ulPatchMuxAddr));
            BDBG_MSG(("ReleaseHwIdLp: eType=%d, ulHwId=%u, Cntr=%d",
                BVDC_P_ResourceType_eLpBck,
                (unsigned int)BVDC_P_LpBack_MuxAddr_To_HwId(pSubRul->ulPatchMuxAddr),
                BVDC_P_Resource_GetHwIdAcquireCntr_isr(pSubRul->hResource,
                BVDC_P_ResourceType_eLpBck,
                BVDC_P_LpBack_MuxAddr_To_HwId(pSubRul->ulPatchMuxAddr))));
        }
#endif
        pSubRul->ulPatchMuxAddr = 0;
        pSubRul->eVnetPatchMode = BVDC_P_VnetPatch_eNone;
        pSubRul->ulMuxValue = BVDC_P_MuxValue_SrcOutputDisabled;
    }

    return;
}


/***************************************************************************
 * {private}
 *
 * BVDC_P_SubRul_JoinInVnet
 *
 * Called by BVDC_P_*_BuildRul_isr to build RUL for joinning into vnet
 */
void BVDC_P_SubRul_JoinInVnet_isr(
    BVDC_P_SubRulContext          *pSubRul,
    BVDC_P_ListInfo               *pList )
{
    /* join in vnet from back to front. Join in vnet after enable module */
    BDBG_ASSERT(pSubRul->ulMuxAddr);
    if (BVDC_P_VnetPatch_eFreeCh == pSubRul->eVnetPatchMode)
    {
        uint32_t ulPostMuxValue;
        BDBG_ASSERT(pSubRul->ulPatchMuxAddr);
        BVDC_P_FreeCh_MuxAddr_To_PostMuxValue(pSubRul->ulPatchMuxAddr, ulPostMuxValue);
        BVDC_P_SUBRUL_ONE_REG(pList, pSubRul->ulMuxAddr, 0, ulPostMuxValue);
        BDBG_ASSERT(ulPostMuxValue != BCHP_VNET_B_CAP_0_SRC_SOURCE_Output_Disabled);
        BVDC_P_SUBRUL_ONE_REG(pList, pSubRul->ulPatchMuxAddr, 0, pSubRul->ulMuxValue);
    }
#ifdef BCHP_VNET_B_LOOPBACK_0_SRC
    else if (BVDC_P_VnetPatch_eLpBack == pSubRul->eVnetPatchMode)
    {
        uint32_t ulPostMuxValue;
        BDBG_ASSERT(pSubRul->ulPatchMuxAddr);
        ulPostMuxValue = BVDC_P_LpBack_MuxAddr_To_PostMuxValue(pSubRul->ulPatchMuxAddr);
        BVDC_P_SUBRUL_ONE_REG(pList, pSubRul->ulMuxAddr, 0, ulPostMuxValue);
        BDBG_ASSERT(ulPostMuxValue!= BCHP_VNET_F_SCL_0_SRC_SOURCE_Output_Disabled);
        BVDC_P_SUBRUL_ONE_REG(pList, pSubRul->ulPatchMuxAddr, 0, pSubRul->ulMuxValue);
    }
#endif
    else
    {
        BVDC_P_SUBRUL_ONE_REG(pList, pSubRul->ulMuxAddr, 0, pSubRul->ulMuxValue);
    }

    return;
}

/***************************************************************************
 * {private}
 *
 * BVDC_P_SubRul_DropOffVnet
 *
 * Called by BVDC_P_*_BuildRul_isr to build RUL for droping off from vnet
 */
static uint32_t BVDC_P_SubRul_GetVnetRefCnt_isr(
    BVDC_P_SubRulContext          *pSubRul )
{
    BVDC_P_ResourceType            eType;
    uint32_t                       ulHwId;
    uint32_t                       ulRefCnt = 0;

    /* drop off vnet fron front to back. Drop off before disable module */
    BDBG_ASSERT(pSubRul->ulMuxAddr);
    if ((BVDC_P_VnetPatch_eFreeCh == pSubRul->eVnetPatchMode) ||
        (BVDC_P_VnetPatch_eLpBack == pSubRul->eVnetPatchMode))
    {
        BDBG_ASSERT(pSubRul->ulPatchMuxAddr);
#ifdef BCHP_VNET_B_LOOPBACK_0_SRC
        ulHwId = (BVDC_P_VnetPatch_eLpBack == pSubRul->eVnetPatchMode) ?
            BVDC_P_LpBack_MuxAddr_To_HwId(pSubRul->ulPatchMuxAddr) :
            BVDC_P_FreeCh_MuxAddr_To_HwId(pSubRul->ulPatchMuxAddr);
        eType = (BVDC_P_VnetPatch_eLpBack == pSubRul->eVnetPatchMode) ?
            BVDC_P_ResourceType_eLpBck : BVDC_P_ResourceType_eFreeCh;
#else
        ulHwId = BVDC_P_FreeCh_MuxAddr_To_HwId(pSubRul->ulPatchMuxAddr);
        eType = BVDC_P_ResourceType_eFreeCh;
#endif
        BDBG_MSG(("LpBack/FreeCh DropOffVnet: eType=%d, ulHwId=%d, Cntr=%d", eType, ulHwId,
            BVDC_P_Resource_GetHwIdAcquireCntr_isr(pSubRul->hResource, eType, ulHwId)));
        /* Only disable LPB or FCH if it is not shared by any other module */
        /* Since acquired counter is decremented to 0 when HW is released  */
        /* which is happenning later than this, at this point, if acquired */
        /* counter is 1, that indicates that it can be disabled */
        ulRefCnt = BVDC_P_Resource_GetHwIdAcquireCntr_isr(pSubRul->hResource, eType, ulHwId);
    }

    return ulRefCnt;
}

/***************************************************************************
 * {private}
 *
 * BVDC_P_SubRul_DropOffVnet
 *
 * Called by BVDC_P_*_BuildRul_isr to build RUL for droping off from vnet
 */
void BVDC_P_SubRul_DropOffVnet_isr(
    BVDC_P_SubRulContext          *pSubRul,
    BVDC_P_ListInfo               *pList )
{
    /* drop off vnet fron front to back. Drop off before disable module */
    BDBG_ASSERT(pSubRul->ulMuxAddr);
    if ((BVDC_P_VnetPatch_eFreeCh == pSubRul->eVnetPatchMode) ||
        (BVDC_P_VnetPatch_eLpBack == pSubRul->eVnetPatchMode))
    {
        /* Only disable LPB or FCH if it is not shared by any other module */
        /* Since acquired counter is decremented to 0 when HW is released  */
        /* which is happenning later than this, at this point, if acquired */
        /* counter is 1, that indicates that it can be disabled */
        if(BVDC_P_SubRul_GetVnetRefCnt_isr(pSubRul) <= 1)
        {
            BVDC_P_SUBRUL_ONE_REG(pList, pSubRul->ulPatchMuxAddr, 0, BVDC_P_MuxValue_SrcOutputDisabled);
        }
        else
        {
            BVDC_P_SubRul_UnsetVnet_isr(pSubRul);
        }
    }

    BVDC_P_SUBRUL_ONE_REG(pList, pSubRul->ulMuxAddr, 0, BVDC_P_MuxValue_SrcOutputDisabled);

    return;
}

/***************************************************************************
 * {private}
 *
 * BVDC_P_SubRul_StartDrain
 *
 * Called by BVDC_P_*_BuildRul_DrainVnet_isr to reset sub and connect the
 * module to a drain.
 */
void BVDC_P_SubRul_Drain_isr(
    BVDC_P_SubRulContext          *pSubRul,
    BVDC_P_ListInfo               *pList,
    uint32_t                       ulResetReg,
    uint32_t                       ulResetMask,
    uint32_t                       ulChnResetReg,
    uint32_t                       ulChnResetMask )
{
    uint32_t  ulDrainMuxAddr, ulDrainSrcAddr;
    uint32_t  ulVnetBChannelReset=0, ulVnetFChannelReset=0;
    uint32_t  ulVnetBChanFch0ResetMask =0, ulVnetFChanLp0ResetMask=0;

    /* Saved Drains' current mux value */
    *pList->pulCurrent++ = BRDC_OP_REG_TO_VAR(BRDC_Variable_0);
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_VNET_F_DRAIN_0_SRC);
    *pList->pulCurrent++ = BRDC_OP_REG_TO_VAR(BRDC_Variable_1);
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_VNET_B_DRAIN_0_SRC);

    ulDrainSrcAddr = (BVDC_P_DrainMode_eFront == pSubRul->eDrainMode)?
        BCHP_VNET_F_DRAIN_0_SRC : BCHP_VNET_B_DRAIN_0_SRC;
    ulDrainMuxAddr = BVDC_P_VnetPatch_eFreeCh == pSubRul->eVnetPatchMode?
        BCHP_VNET_B_DRAIN_0_SRC : BCHP_VNET_F_DRAIN_0_SRC;


#if BCHP_MMISC_VNET_B_CHANNEL_RESET
    ulVnetBChannelReset = BCHP_MMISC_VNET_B_CHANNEL_RESET;
    ulVnetFChannelReset = BCHP_MMISC_VNET_F_CHANNEL_RESET;
#if BCHP_MMISC_VNET_B_CHANNEL_RESET
    ulVnetBChanFch0ResetMask = BCHP_MMISC_VNET_B_CHANNEL_RESET_FCH_0_RESET_MASK;
    ulVnetFChanLp0ResetMask  = BCHP_MMISC_VNET_F_CHANNEL_RESET_LOOP_0_RESET_MASK;
#endif
#elif BCHP_MMISC_VNET_B_CHANNEL_SW_INIT
    ulVnetBChannelReset = BCHP_MMISC_VNET_B_CHANNEL_SW_INIT;
    ulVnetFChannelReset = BCHP_MMISC_VNET_F_CHANNEL_SW_INIT;
    ulVnetBChanFch0ResetMask = BCHP_MMISC_VNET_B_CHANNEL_SW_INIT_FCH_0_MASK;
    ulVnetFChanLp0ResetMask  = BCHP_MMISC_VNET_F_CHANNEL_SW_INIT_LOOP_0_MASK;
#endif

    /* start to set 1 to core/channel sw_init */
    if (0 != ulResetMask)
    {
        /* reset before drain. capture only need reset */
        BVDC_P_SUBRUL_ONE_REG(pList, ulResetReg, 0, ulResetMask);
    }

    if (BVDC_P_DrainMode_eNone != pSubRul->eDrainMode)
    {
        /* connect the module to the drain. will not call this func if drain
         * is not needed */
        BVDC_P_SUBRUL_ONE_REG(pList, ulDrainSrcAddr, 0, pSubRul->ulPostMuxValue);
    }

    if (0 != ulChnResetReg)
    {
        BVDC_P_SUBRUL_ONE_REG(pList, ulChnResetReg, 0, ulChnResetMask);

        /* drain the free-channel or loop-back that is used to patch the vnet */
        if (BVDC_P_VnetPatch_eFreeCh == pSubRul->eVnetPatchMode)
        {
            uint32_t ulPostMuxValue;

            BVDC_P_FreeCh_MuxAddr_To_PostMuxValue(pSubRul->ulPatchMuxAddr, ulPostMuxValue)
            BVDC_P_SUBRUL_ONE_REG(pList, ulDrainMuxAddr, 0, ulPostMuxValue);
            /* reset channel ping-pong after drain the free channel */
            BVDC_P_SUBRUL_ONE_REG(pList, ulDrainMuxAddr, 0, BVDC_P_MuxValue_SrcOutputDisabled);
            if(ulVnetBChannelReset)
            {
                BVDC_P_SUBRUL_ONE_REG(pList, ulVnetBChannelReset, 0,
                    BVDC_P_FreeCh_MuxAddr_To_ChnResetMask(pSubRul->ulPatchMuxAddr, ulVnetBChanFch0ResetMask));
                BVDC_P_SUBRUL_ONE_REG(pList, ulVnetBChannelReset, 0, 0);
            }
        }
#ifdef BCHP_VNET_B_LOOPBACK_0_SRC
        else if (BVDC_P_VnetPatch_eLpBack == pSubRul->eVnetPatchMode)
        {
            BVDC_P_SUBRUL_ONE_REG(pList, ulDrainMuxAddr, 0,
                BVDC_P_LpBack_MuxAddr_To_PostMuxValue(pSubRul->ulPatchMuxAddr));
            BVDC_P_SUBRUL_ONE_REG(pList, ulDrainMuxAddr, 0, BVDC_P_MuxValue_SrcOutputDisabled);

            /* reset channel ping-pong after drain the loop back if not shared by another active window */
            if(ulVnetFChannelReset && (BVDC_P_SubRul_GetVnetRefCnt_isr(pSubRul) <= 1))
            {
                BVDC_P_SUBRUL_ONE_REG(pList, ulVnetFChannelReset, 0,
                    BVDC_P_LpBack_MuxAddr_To_ChnResetMask(pSubRul->ulPatchMuxAddr, ulVnetFChanLp0ResetMask));
                BVDC_P_SUBRUL_ONE_REG(pList, ulVnetFChannelReset, 0, 0);
            }
        }
#endif
        BVDC_P_SUBRUL_ONE_REG(pList, ulChnResetReg, 0, 0);
    }

    /* start to clear 0 to core/channel sw_init */
    if (BVDC_P_DrainMode_eNone != pSubRul->eDrainMode)
    {
        /* connect the module to the drain. will not call this func if drain
         * is not needed */
        BVDC_P_SUBRUL_ONE_REG(pList, ulDrainSrcAddr, 0, BVDC_P_MuxValue_SrcOutputDisabled);

    }

    if (0 != ulResetMask)
    {
        /* reset before drain. capture only need reset */
        BVDC_P_SUBRUL_ONE_REG(pList, ulResetReg, 0, 0);
    }


    /* Restore drains' current mux value. */
    *pList->pulCurrent++ = BRDC_OP_VAR_TO_REG(BRDC_Variable_0);
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_VNET_F_DRAIN_0_SRC);
    *pList->pulCurrent++ = BRDC_OP_VAR_TO_REG(BRDC_Variable_1);
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_VNET_B_DRAIN_0_SRC);

}


/* End of file. */
