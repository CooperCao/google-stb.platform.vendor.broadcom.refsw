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
#ifndef BVDC_VFC_PRIV_H__
#define BVDC_VFC_PRIV_H__

#include "bvdc.h"
#include "bchp_common.h"
#include "bvdc_common_priv.h"
#include "bvdc_subrul_priv.h"
#include "bvdc_buffer_priv.h"
#include "bvdc_scaler_priv.h"

#if (BVDC_P_SUPPORT_VFC)
#include "bchp_vfc_0.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
 * Private register cracking macros
 ***************************************************************************/
#if (BVDC_P_SUPPORT_VFC)

#define BVDC_P_VFC_GET_REG_IDX(reg) \
    ((BCHP##_##reg - BCHP_VFC_0_REG_START) / sizeof(uint32_t))

/* Get/Set reg data */
#define BVDC_P_VFC_GET_REG_DATA(hxs, reg) \
    ((hxs)->aulRegs[BVDC_P_VFC_GET_REG_IDX(reg)])
#define BVDC_P_VFC_SET_REG_DATA(hxs, reg, data) \
    (BVDC_P_VFC_GET_REG_DATA((hxs), reg) = (uint32_t)(data))

/* Get with index. */
#define BVDC_P_VFC_GET_REG_DATA_I(hxs, idx, reg) \
    ((hxs)->aulRegs[BVDC_P_VFC_GET_REG_IDX(reg) + (idx)])

/* Get field */
#define BVDC_P_VFC_GET_FIELD_NAME(hxs, reg, field) \
    (BVDC_P_GET_FIELD(BVDC_P_VFC_GET_REG_DATA((hxs), reg), reg, field))

/* Compare field */
#define BVDC_P_VFC_COMPARE_FIELD_DATA(hxs, reg, field, data) \
    (BVDC_P_COMPARE_FIELD_DATA(BVDC_P_VFC_GET_REG_DATA((hxs), reg), reg, \
        field, (data)))
#define BVDC_P_VFC_COMPARE_FIELD_NAME(hxs, reg, field, name) \
    (BVDC_P_COMPARE_FIELD_NAME(BVDC_P_VFC_GET_REG_DATA((hxs), reg), reg, \
        field, name))

/* This macro does a write into RUL (write, addr, data). 3 dwords. */
#define BVDC_P_VFC_WRITE_TO_RUL(hxs, reg, addr_ptr) \
do { \
    *addr_ptr++ = BRDC_OP_IMM_TO_REG(); \
    *addr_ptr++ = BRDC_REGISTER(BCHP##_##reg + (hxs)->ulRegOffset); \
    *addr_ptr++ = BVDC_P_VFC_GET_REG_DATA((hxs), reg); \
} while (0)

/* This macro does a block write into RUL */
#define BVDC_P_VFC_BLOCK_WRITE_TO_RUL(hxs, from, to, pulCurrent) \
do { \
    uint32_t ulBlockSize = \
        BVDC_P_REGS_ENTRIES(from, to);\
    *pulCurrent++ = BRDC_OP_IMMS_TO_REGS( ulBlockSize ); \
    *pulCurrent++ = BRDC_REGISTER(BCHP##_##from + (hxs)->ulRegOffset); \
    BKNI_Memcpy((void*)pulCurrent, \
        (void*)&((hxs)->aulRegs[BVDC_P_VFC_GET_REG_IDX(from)]), \
        ulBlockSize * sizeof(uint32_t)); \
    pulCurrent += ulBlockSize; \
} while(0)

/* This macro does a scaler reset RUL. */
#define BVDC_P_VFC_RESET_RUL(hxs) \
do { \
    *pulCurrent++ = BRDC_OP_IMM_TO_REG(); \
    *pulCurrent++ = BRDC_REGISTER((hxs)->ulResetRegAddr); \
    *pulCurrent++ = (hxs)->ulResetMask; \
    *pulCurrent++ = BRDC_OP_IMM_TO_REG(); \
    *pulCurrent++ = BRDC_REGISTER((hxs)->ulResetRegAddr); \
    *pulCurrent++ = 0; \
} while (0)
/* TBD: This macro is unused. Pernaps it should be eliminated. */

/* number of registers in one block. */
#define BVDC_P_VFC_REGS_COUNT      \
    BVDC_P_REGS_ENTRIES(VFC_0_REG_START, VFC_0_REG_END)

#else
#define BVDC_P_VFC_REGS_COUNT     1
#ifndef BCHP_VNET_B_CAP_0_SRC_SOURCE_VFC_0
#define BCHP_VNET_B_CAP_0_SRC_SOURCE_VFC_0 0
#endif
#endif

/* VNET */
#if (BVDC_P_SUPPORT_VFC)
#define BVDC_P_Vfc_MuxAddr(VFC)       (BCHP_VNET_F_VFC_0_SRC + (VFC)->eId * sizeof(uint32_t))
#define BVDC_P_Vfc_PostMuxValue(VFC)  (BCHP_VNET_B_CAP_0_SRC_SOURCE_VFC_0 + (VFC)->eId)
#else
#define BVDC_P_Vfc_MuxAddr(VFC)       (0)
#define BVDC_P_Vfc_PostMuxValue(VFC)  (0)
#endif
#define BVDC_P_Vfc_SetVnet_isr(VFC, ulSrcMuxValue, eVnetPatchMode) \
   BVDC_P_SubRul_SetVnet_isr(&((VFC)->SubRul), ulSrcMuxValue, eVnetPatchMode)
#define BVDC_P_Vfc_UnsetVnet_isr(VFC) \
   BVDC_P_SubRul_UnsetVnet_isr(&((VFC)->SubRul))
#define BVDC_P_Vfc_SetRulBuildWinId_isr(hxs, eWinId) \
    BVDC_P_SubRul_SetRulBuildWinId_isr(&((hxs)->SubRul), eWinId)


/***************************************************************************
 * Vfc private data structures
 ***************************************************************************/
typedef struct BVDC_P_VfcContext
{
    BDBG_OBJECT(BVDC_VFC)

    /* flag initial state, requires reset; */
    bool                           bInitial;
    uint32_t                       ulResetRegAddr;
    uint32_t                       ulResetMask;
    uint32_t                       ulVnetResetAddr;
    uint32_t                       ulVnetResetMask;
    uint32_t                       ulVnetMuxAddr;
    uint32_t                       ulVnetMuxValue;

    /* private fields. */
    BVDC_P_VfcId                   eId;
    uint32_t                       ulRegOffset; /* VFC_0, VFC_1, and etc. */
    uint32_t                       aulRegs[BVDC_P_VFC_REGS_COUNT];
    BREG_Handle                    hReg;

    /* update flag. */
    uint32_t                       ulUpdateAll;

    BVDC_Window_Handle             hWindow;

    /* CFC */
    BVDC_P_CfcContext              stCfc;

    uint32_t                       ulPrevWidth;
    uint32_t                       ulPrevHeight;
    BFMT_Orientation               ePrevSrcOrientation;
    BFMT_Orientation               ePrevDispOrientation;
    bool                           bPrevSrc10Bit;
    bool                           bPrevEnableStg;
    BVDC_P_DitherSetting           stDither;

    /* sub-struct to manage vnet and rul build opreations */
    BVDC_P_SubRulContext           SubRul;
} BVDC_P_VfcContext;


/***************************************************************************
 * Vfc private functions
 ***************************************************************************/
BERR_Code BVDC_P_Vfc_Create
    ( BVDC_P_Vfc_Handle            *phVfc,
      BVDC_P_VfcId                  eVfcId,
      BVDC_P_Resource_Handle        hResource,
      BREG_Handle                   hReg );

void BVDC_P_Vfc_Destroy
    ( BVDC_P_Vfc_Handle             hVfc );

void BVDC_P_Vfc_Init_isrsafe
    ( BVDC_P_Vfc_Handle             hVfc );

void BVDC_P_Vfc_InitCfc_isrsafe
    ( BVDC_P_Vfc_Handle             hVfc);

BERR_Code BVDC_P_Vfc_AcquireConnect_isr
    ( BVDC_P_Vfc_Handle             hVfc,
      BVDC_Window_Handle            hWindow );

BERR_Code BVDC_P_Vfc_ReleaseConnect_isr
    ( BVDC_P_Vfc_Handle            *phVfc );

void BVDC_P_Vfc_BuildRul_SetEnable_isr
    ( BVDC_P_Vfc_Handle             hVfc,
      BVDC_P_ListInfo              *pList );

void BVDC_P_Vfc_BuildRul_isr
    ( const BVDC_P_Vfc_Handle       hVfc,
      BVDC_P_ListInfo              *pList,
      BVDC_P_State                  eVnetState,
      BVDC_P_PicComRulInfo         *pPicComRulInfo );

void BVDC_P_Vfc_BuildCfcRul_isr
    ( BVDC_P_Vfc_Handle                hVfc,
      BVDC_P_ListInfo                 *pList);

void BVDC_P_Vfc_SetInfo_isr
    ( BVDC_P_Vfc_Handle             hVfc,
      BVDC_Window_Handle            hWindow,
      const BVDC_P_PictureNodePtr   pPicture );

void BVDC_P_Vfc_SetEnable_isr
    ( BVDC_P_Vfc_Handle             hVfc,
      bool                          bEnable );

#ifdef __cplusplus
}
#endif

#endif /* #ifndef BVDC_VFC_PRIV_H__ */
/* End of file. */
