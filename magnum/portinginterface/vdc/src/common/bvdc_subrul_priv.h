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

#ifndef BVDC_SUBRUL_PRIV_H__
#define BVDC_SUBRUL_PRIV_H__

#include "bstd.h"
#include "bvdc.h"
#include "bvdc_common_priv.h"
#include "bvdc_resource_priv.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
 * {private}
 *
 * SubRul Utility Sub-module Overview:
 *
 * In VDC there are quite amount of HW sub-modules such as vfd, cap, mad, anr,
 * boxDetect, scl, cmp, mfd, dnr, .... Window code uses eVnetMode to connect
 * them together. Since some modules (such as BoxDetect, anr, dnr) could be
 * shared by more than one windows, one window shuts down vnet might not cause
 * the shared module to shut down.
 *
 * This utility sub-module is designed provide utilities for those sub-modules
 * to build rul at each source or display vsync. That includes a func to decide
 * what operations are needed with each BVDC_P_*_BuildRul_isr call (mainly for
 * shuting-down process of the shared sub-modules), and funcs to join-into and
 * drop-off from vnet.
 *
 * For a typical usage, the vnet used sub-module should have a sub-struct
 * BVDC_P_SubRul_Context, call BVDC_P_SubRul_Init in BVDC_P_*_Create, and then
 * call BVDC_P_SubRul_GetOps_isr in each BVDC_P_*_BuildRul_isr. Here is an
 * example:
 *
 *  ulRulOps = BVDC_P_SubRul_GetOps_isr(
 *      &(pAnr->SubRul), pPicComRulInfo->eWin, eVnetState, pList->bLastRulExecuted);
 *
 *  // if we re-set vnet, or if src video format changes (DdDvi), we need to
 *  // do SrcInit, such as reset buffer size and stride
 *  if ((ulRulOps & BVDC_P_RulOp_eVnetInit) || (pPicComRulInfo->stSrcDirty.bInputFormat))
 *      BVDC_P_Anr_BuildRul_SrcInit_isr(pAnr, pList, pSrcFmtInfo);
 *
 *  // periodic statistics init
 *  if (ulRulOps & BVDC_P_RulOp_eStatisInit)
 *      BVDC_P_Anr_BuildRul_StatisInit_isr(pAnr, pList, pSrcFmtInfo);
 *
 *  if (ulRulOps & BVDC_P_RulOp_eEnable)
 *  {
 *      BVDC_P_Anr_BuildRul_SetEnable_isr(pAnr, pList, true,
 *          pPicComRulInfo->eSrcOrigPolarity, pSrcFmtInfo->bInterlaced);
 *
 *      // join in vnet after enable. note: its src mux is initialed as disabled
 *      if (ulRulOps & BVDC_P_RulOp_eVnetInit)
 *      {
 *          BVDC_P_SubRul_JoinInVnet(&(pAnr->SubRul), pList, pAnr->ulRegOffset);
 *      }
 *  }
 *  else if (ulRulOps & BVDC_P_RulOp_eDisable)
 *  {
 *      BVDC_P_SubRul_DropOffVnet_isr(pAnr, pList, pAnr->ulRegOffset);
 *      BVDC_P_Anr_BuildRul_SetEnable_isr(pAnr, pList, false, pPicComRulInfo->eSrcOrigPolarity);
 *  }
 *  else if (ulRulOpsFlags & BVDC_P_RulOp_eDrainVnet)
 *  {
 *      BVDC_P_Anr_BuildRul_DrainVnet_isr(pAnr, pList);
 *  }
 */

#define BVDC_P_SUBRUL_VNET_INIT_MARK                      (-20)
#define BVDC_P_SUBRUL_VNET_INIT_CHECK                     (BVDC_P_SUBRUL_VNET_INIT_MARK+1)
#define BVDC_P_SUBRUL_STATIS_INIT_MARK                    (-10)
#define BVDC_P_SUBRUL_STATIS_INIT_CHECK                   (BVDC_P_SUBRUL_STATIS_INIT_MARK+1)
#define BVDC_P_SUBRUL_STATE_STATIS_INIT(lAccumCntr)       (BVDC_P_SUBRUL_STATIS_INIT_MARK >= (lAccumCntr))
#define BVDC_P_SUBRUL_STATE_STATIS_INIT_CHECK(lAccumCntr) (BVDC_P_SUBRUL_STATIS_INIT_CHECK  == (lAccumCntr))
#define BVDC_P_SUBRUL_STATE_VNET_INIT(lAccumCntr)         (BVDC_P_SUBRUL_VNET_INIT_MARK == (lAccumCntr))
#define BVDC_P_SUBRUL_STATE_VNET_INIT_CHECK(lAccumCntr)   (BVDC_P_SUBRUL_VNET_INIT_CHECK  == (lAccumCntr))

/* This macro does a single reg write into RUL (write, addr, data). 3 dwords. */
#define BVDC_P_SUBRUL_ONE_REG(pList, reg, offs, value) \
{ \
    *(pList)->pulCurrent++ = BRDC_OP_IMM_TO_REG(); \
    *(pList)->pulCurrent++ = BRDC_REGISTER(reg + offs); \
    *(pList)->pulCurrent++ = value; \
}

/* This macro start a reg block write into RUL (write_num, addr). 2 dwords. */
#define BVDC_P_SUBRUL_START_BLOCK(pList, reg, offs, num) \
{ \
    *(pList)->pulCurrent++ = BRDC_OP_IMMS_TO_REGS(num); \
    *(pList)->pulCurrent++ = BRDC_REGISTER(reg + offs); \
}

/* all BCHP_VNET_F_*_SRC_SOURCE_Output_Disabled is '15' */
#define BVDC_P_MuxValue_SrcOutputDisabled      BCHP_VNET_F_SCL_0_SRC_SOURCE_Output_Disabled

/* from HwId to get the post mux value as this sub is its src: for lpback and FreeCh */
#if (BVDC_P_SUPPORT_LOOP_BACK > 4)
#define BVDC_P_LpBack_eId_To_PostMuxValue(eId) \
   ((0 == (eId))? BCHP_VNET_F_SCL_0_SRC_SOURCE_Loopback_0 : \
    (1 == (eId))? BCHP_VNET_F_SCL_0_SRC_SOURCE_Loopback_1 : \
    (2 == (eId))? BCHP_VNET_F_SCL_0_SRC_SOURCE_Loopback_2 : \
    (3 == (eId))? BCHP_VNET_F_SCL_0_SRC_SOURCE_Loopback_3 : \
                  BCHP_VNET_F_SCL_0_SRC_SOURCE_Loopback_4 + (eId) - BVDC_P_LpBckId_eLp4)
#elif (BVDC_P_SUPPORT_LOOP_BACK > 2)
#define BVDC_P_LpBack_eId_To_PostMuxValue(eId) \
   ((0 == (eId))? BCHP_VNET_F_SCL_0_SRC_SOURCE_Loopback_0 : \
    (1 == (eId))? BCHP_VNET_F_SCL_0_SRC_SOURCE_Loopback_1 : \
                  BCHP_VNET_F_SCL_0_SRC_SOURCE_Loopback_2 + (eId) - BVDC_P_LpBckId_eLp2)
#else
#define BVDC_P_LpBack_eId_To_PostMuxValue(eId) \
   ((0 == (eId))? BCHP_VNET_F_SCL_0_SRC_SOURCE_Loopback_0 : \
                  BCHP_VNET_F_SCL_0_SRC_SOURCE_Loopback_1)
#endif
/* There is a hole between BCHP_VNET_B_CAP_0_SRC_SOURCE_Free_Ch_3 and BCHP_VNET_B_CAP_0_SRC_SOURCE_Free_Ch_4 */
#if (BVDC_P_SUPPORT_FREE_CHANNEL > 4)
#define BVDC_P_FreeCh_eId_To_PostMuxValue(eId)   \
    ((eId > 3) ? (BCHP_VNET_B_CAP_0_SRC_SOURCE_Free_Ch_4 + (eId-4)) \
    : (BCHP_VNET_B_CAP_0_SRC_SOURCE_Free_Ch_0 + (eId)))
#else
#define BVDC_P_FreeCh_eId_To_PostMuxValue(eId) (BCHP_VNET_B_CAP_0_SRC_SOURCE_Free_Ch_0 + (eId))
#endif

/* from HwId to get this module's mux addr: for lpback, FreeCh and front-drain */
#define BVDC_P_FreeCh_eId_To_MuxAddr(eId)      (BCHP_VNET_F_FCH_0_SRC + (eId) * sizeof(uint32_t))
#define BVDC_P_FrnDrain_eId_To_MuxAddr(eId)    (BCHP_VNET_F_DRAIN_0_SRC + (eId) * sizeof(uint32_t))
#define BVDC_P_LpBack_eId_To_MuxAddr(eId)      (BCHP_VNET_B_LOOPBACK_0_SRC + (eId) * sizeof(uint32_t))

#define BVDC_P_Bp_eId_To_MuxAddr(eId)          (BCHP_VNET_B_BP_0_SRC + (eId) * sizeof(uint32_t))
#define BVDC_P_Crc_eId_To_MuxAddr(eId)         (BCHP_VNET_B_CRC_SRC + (eId) * sizeof(uint32_t))
#define BVDC_P_CrcCtrl_eId_To_MuxAddr(eId)     (BCHP_VNET_B_CRC_CTRL + (eId) * sizeof(uint32_t))
#define BVDC_P_CrcYStatus_eId_To_MuxAddr(eId)  (BCHP_VNET_B_CRC_Y_STATUS + (eId) * sizeof(uint32_t))
#define BVDC_P_CrcCbStatus_eId_To_MuxAddr(eId) (BCHP_VNET_B_CRC_CB_STATUS + (eId) * sizeof(uint32_t))
#define BVDC_P_CrcCrStatus_eId_To_MuxAddr(eId) (BCHP_VNET_B_CRC_CR_STATUS + (eId) * sizeof(uint32_t))

/* BCHP_VNET_B_CAP_0_SRC_SOURCE_Output_Disabled is fixed as 15 */
#define BVDC_P_FreeCh_MuxAddr_To_PostMuxValue(ulMuxAddr, ulPostMuxValue)  \
{ \
    ulPostMuxValue = BCHP_VNET_B_CAP_0_SRC_SOURCE_Free_Ch_0 + ((ulMuxAddr) - BCHP_VNET_F_FCH_0_SRC) / sizeof(uint32_t) ; \
    ulPostMuxValue = ulPostMuxValue + (ulPostMuxValue >=BCHP_VNET_B_CAP_0_SRC_SOURCE_Output_Disabled) - \
    (BCHP_VNET_B_CAP_0_SRC_SOURCE_Free_Ch_0 > BCHP_VNET_B_CAP_0_SRC_SOURCE_Output_Disabled); \
}

/* from mux addr to HwId as this sub is its src: for LpBack and FreeCh */
#if (BVDC_P_SUPPORT_LOOP_BACK > 2)
#define BVDC_P_LpBack_MuxAddr_To_HwId(ulMuxAddr) \
   ((BCHP_VNET_B_LOOPBACK_0_SRC == (ulMuxAddr))? BVDC_P_LpBckId_eLp0 : \
    (BCHP_VNET_B_LOOPBACK_1_SRC == (ulMuxAddr))? BVDC_P_LpBckId_eLp1 : \
     BVDC_P_LpBckId_eLp2 + ((ulMuxAddr) - BCHP_VNET_B_LOOPBACK_2_SRC) / sizeof(uint32_t))
#else
#define BVDC_P_LpBack_MuxAddr_To_HwId(ulMuxAddr) \
   ((BCHP_VNET_B_LOOPBACK_0_SRC == (ulMuxAddr))? BVDC_P_LpBckId_eLp0 : \
                                                 BVDC_P_LpBckId_eLp1)
#endif

/* from mux addr to post mux value as this sub is its src: for LpBack and FreeCh */
#define BVDC_P_LpBack_MuxAddr_To_PostMuxValue(ulMuxAddr) \
   BVDC_P_LpBack_eId_To_PostMuxValue(BVDC_P_LpBack_MuxAddr_To_HwId(ulMuxAddr))

#define BVDC_P_FreeCh_MuxAddr_To_HwId(ulMuxAddr)  \
   (BVDC_P_FreeChId_eCh0 + ((ulMuxAddr) - BCHP_VNET_F_FCH_0_SRC) / sizeof(uint32_t))

/* from mux addr to channel reset mask: for LpBack and FreeCh */

#define BVDC_P_LpBack_MuxAddr_To_ChnResetMask(ulMuxAddr, ulLoop0Mask) \
    ((ulLoop0Mask) << ((ulMuxAddr) - BCHP_VNET_B_LOOPBACK_0_SRC) / sizeof(uint32_t))
#define BVDC_P_FreeCh_MuxAddr_To_ChnResetMask(ulMuxAddr, ulFch0Mask)  \
   ((ulFch0Mask) << ((ulMuxAddr) - BCHP_VNET_F_FCH_0_SRC) / sizeof(uint32_t))


/***************************************************************************
 * Ops are or-ed before passed to each module to tell what operations are
 * needed with the current RUL build cycle
 */
typedef enum
{
    BVDC_P_RulOp_eVnetInit =         (1<<0),
    BVDC_P_RulOp_eStatisInit =       (1<<1),
    BVDC_P_RulOp_eEnable =           (1<<2),
    BVDC_P_RulOp_eDisable =          (1<<3),
    BVDC_P_RulOp_eDrainVnet =        (1<<4),
    BVDC_P_RulOp_eReleaseHandle =    (1<<5)
} BVDC_P_RulOp;

/***************************************************************************
 * module vnet patch of free-channel or loop-back
 */
typedef enum BVDC_P_VnetPatch
{
   BVDC_P_VnetPatch_eNone,    /* no need to patch */
   BVDC_P_VnetPatch_eFreeCh,  /* patch a FreeChannel in front */
   BVDC_P_VnetPatch_eLpBack   /* patch a loop back in front */
} BVDC_P_VnetPatch;


/***************************************************************************
 * module vnet drain mode
 */
typedef enum BVDC_P_DrainMode
{
   BVDC_P_DrainMode_eNone,     /* no need to drain this module */
   BVDC_P_DrainMode_eFront,    /* drain by front drain */
   BVDC_P_DrainMode_eBack      /* drain by front drain */
} BVDC_P_DrainMode;


/***************************************************************************
 * SubRul private data structures
 ***************************************************************************/
typedef struct BVDC_P_SubRulContext
{
    BVDC_P_Resource_Handle         hResource;

    /* cntr for statistics and vnet init process */
    int32_t          lAccumCntr;
    int32_t          lStatisReadMark;

    /* its bit (1<<eWin) indicates if eWin is actively using this module */
    uint32_t         ulWinsActFlags;

    /* the window that builds RUL */
    BVDC_P_WindowId  eBldWin;

    /* for vnet join-in, tear-off, and drain.
     * note: feeder module does not need to set vnet, middle and back module
     * need to have a pre-loop-back if it is after another middle module, and
     * need to have a pre-free-channel if it is directly connected to a
     * front module
     */

    /* set with BVDC_P_SubRul_SetVnet */
    BVDC_P_VnetPatch eVnetPatchMode;
    uint32_t         ulMuxValue;     /* this module's current src mux value */
    uint32_t         ulPatchMuxAddr; /* src mux addr of the pre-freeCh or pre-
                                      * Lpback if needed, 0 if not needed */
    /* set with BVDC_P_SubRul_Init */
    uint32_t         ulMuxAddr;      /* this module's src mux addr */
    uint32_t         ulPostMuxValue; /* the value for the post mux if this
                                      * module is its src */
    /* not need to drain, need front or back drainer */
    BVDC_P_DrainMode eDrainMode;

} BVDC_P_SubRulContext;


#define BVDC_P_SubRul_GetWinsActFlags(pSubRul)   \
        ((pSubRul)->ulWinsActFlags)

#define BVDC_P_SubRul_SetStatisReadPeriod(pSubRul, lReadPeriod)   \
        ((pSubRul)->lStatisReadMark = (lReadPeriod))

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
    BVDC_P_Resource_Handle         hResource );

/***************************************************************************
 * {private}
 *
 * BVDC_P_SubRul_ResetAccumCntr_isr
 *
 * Called by a module to reset AccumCntr to 0 for statistics cntr reset.
 */
void BVDC_P_SubRul_ResetAccumCntr_isr(
    BVDC_P_SubRulContext          *pSubRul );

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
    BVDC_P_WindowId                eWin );

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
    bool                           bLastRulExecuted );

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
    BVDC_P_VnetPatch               eVnetPatchMode);

/***************************************************************************
 * {private}
 *
 * BVDC_P_SubRul_UnsetVnet
 *
 * Called by BVDC_P_*_UnsetVnet_isr to release the free-channel or
 * loop-back used to to patch the vnet.  It should be called only after
 * shut-down RUL is executed.
 */
void BVDC_P_SubRul_UnsetVnet_isr(
    BVDC_P_SubRulContext          *pSubRul);

/***************************************************************************
 * {private}
 *
 * BVDC_P_SubRul_JoinInVnet
 *
 * Called by BVDC_P_*_BuildRul_isr to build RUL for joinning into vnet
 */
void BVDC_P_SubRul_JoinInVnet_isr(
    BVDC_P_SubRulContext          *pSubRul,
    BVDC_P_ListInfo               *pList );

/***************************************************************************
 * {private}
 *
 * BVDC_P_SubRul_DropOffVnet
 *
 * Called by BVDC_P_*_BuildRul_isr to build RUL for droping off from vnet
 */
void BVDC_P_SubRul_DropOffVnet_isr(
    BVDC_P_SubRulContext          *pSubRul,
    BVDC_P_ListInfo               *pList );

/***************************************************************************
 * {private}
 *
 * BVDC_P_SubRul_Drain
 *
 * Called by BVDC_P_*_BuildRul_DrainVnet_isr to reset sub and connect the
 * module to a drain, reset its channel ping-pong buf; and finally
 * drain the free-channel or loop-back used to to patch the vnet, and reset
 * its channel ping-pong buf too.
 */

void BVDC_P_SubRul_Drain_isr(
    BVDC_P_SubRulContext          *pSubRul,
    BVDC_P_ListInfo               *pList,
    uint32_t                       ulResetReg,
    uint32_t                       ulResetMask,
    uint32_t                       ulChnResetReg,
    uint32_t                       ulChnResetMask );

/***************************************************************************
 * {private}
 *
 * BVDC_P_SubRul_GetResourceHandle_isr
 *
 * Called by module to get the resource handle
 */
#define BVDC_P_SubRul_GetResourceHandle_isr(pSubRul)  \
    ((pSubRul)->hResource)

#ifdef __cplusplus
}
#endif

#endif /* #ifndef BVDC_SUBRUL_PRIV_H__ */

/* End of file. */
