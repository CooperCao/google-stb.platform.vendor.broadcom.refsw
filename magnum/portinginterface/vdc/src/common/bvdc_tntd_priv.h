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
#ifndef BVDC_TNTD_PRIV_H__
#define BVDC_TNTD_PRIV_H__

#include "bvdc.h"
#include "bchp_common.h"
#include "bvdc_common_priv.h"
#include "bvdc_subrul_priv.h"
#include "bvdc_buffer_priv.h"
#include "bvdc_scaler_priv.h"

#if (BVDC_P_SUPPORT_TNTD)
#include "bchp_tntd_0.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Core revs to track increatmental changes! */
#define BVDC_P_SUPPORT_TNTD_VER_1             (1) /* 7445 D0 */

#define BVDC_P_TNTD_SML_CONFIG_THRESH BVDC_P_FLOAT_TO_FIXED(1.3, BVDC_P_SCL_V_RATIO_I_BITS, BVDC_P_NRM_SRC_STEP_F_BITS)
#define BVDC_P_TNTD_MED_CONFIG_THRESH BVDC_P_FLOAT_TO_FIXED(1.8, BVDC_P_SCL_V_RATIO_I_BITS, BVDC_P_NRM_SRC_STEP_F_BITS)
#define BVDC_P_TNTD_LRG_CONFIG_THRESH BVDC_P_FLOAT_TO_FIXED(2.5, BVDC_P_SCL_V_RATIO_I_BITS, BVDC_P_NRM_SRC_STEP_F_BITS)
#define BVDC_P_TNTD_BEFORE_SCL_THRESH BVDC_P_TNTD_LRG_CONFIG_THRESH

/***************************************************************************
 * Private register cracking macros
 ***************************************************************************/
#if (BVDC_P_SUPPORT_TNTD)

#define BVDC_P_TNTD_GET_REG_IDX(reg) \
    ((BCHP##_##reg - BCHP_TNTD_0_REG_START) / sizeof(uint32_t))

/* Get/Set reg data */
#define BVDC_P_TNTD_GET_REG_DATA(reg) \
    (hTntd->aulRegs[BVDC_P_TNTD_GET_REG_IDX(reg)])
#define BVDC_P_TNTD_SET_REG_DATA(reg, data) \
    (BVDC_P_TNTD_GET_REG_DATA(reg) = (uint32_t)(data))

/* Get with index. */
#define BVDC_P_TNTD_GET_REG_DATA_I(idx, reg) \
    (hTntd->aulRegs[BVDC_P_TNTD_GET_REG_IDX(reg) + (idx)])

/* Get field */
#define BVDC_P_TNTD_GET_FIELD_NAME(reg, field) \
    (BVDC_P_GET_FIELD(BVDC_P_TNTD_GET_REG_DATA(reg), reg, field))

/* Compare field */
#define BVDC_P_TNTD_COMPARE_FIELD_DATA(reg, field, data) \
    (BVDC_P_COMPARE_FIELD_DATA(BVDC_P_TNTD_GET_REG_DATA(reg), reg, field, (data)))
#define BVDC_P_TNTD_COMPARE_FIELD_NAME(reg, field, name) \
    (BVDC_P_COMPARE_FIELD_NAME(BVDC_P_TNTD_GET_REG_DATA(reg), reg, field, name))

/* This macro does a write into RUL (write, addr, data). 3 dwords. */
#define BVDC_P_TNTD_WRITE_TO_RUL(reg, addr_ptr) \
{ \
    *addr_ptr++ = BRDC_OP_IMM_TO_REG(); \
    *addr_ptr++ = BRDC_REGISTER(BCHP##_##reg + hTntd->ulRegOffset); \
    *addr_ptr++ = BVDC_P_TNTD_GET_REG_DATA(reg); \
}

/* This macro does a block write into RUL */
#define BVDC_P_TNTD_BLOCK_WRITE_TO_RUL(from, to, pulCurrent) \
do { \
    uint32_t ulBlockSize = \
        BVDC_P_REGS_ENTRIES(from, to);\
    *pulCurrent++ = BRDC_OP_IMMS_TO_REGS( ulBlockSize ); \
    *pulCurrent++ = BRDC_REGISTER(BCHP##_##from + hTntd->ulRegOffset); \
    BKNI_Memcpy((void*)pulCurrent, \
        (void*)&(hTntd->aulRegs[BVDC_P_TNTD_GET_REG_IDX(from)]), \
        ulBlockSize * sizeof(uint32_t)); \
    pulCurrent += ulBlockSize; \
} while(0)

/* This macro does a block write  of n registers into RUL */
#define BVDC_P_TNTD_BLOCK_WRITE_nREG_TO_RUL(from, howMany, pulCurrent) \
do { \
    uint32_t ulBlockSize = howMany; \
    *pulCurrent++ = BRDC_OP_IMMS_TO_REGS( ulBlockSize ); \
    *pulCurrent++ = BRDC_REGISTER(BCHP##_##from + hTntd->ulRegOffset); \
    BKNI_Memcpy((void*)pulCurrent, \
        (void*)&(hTntd->aulRegs[BVDC_P_TNTD_GET_REG_IDX(from)]), \
        ulBlockSize * sizeof(uint32_t)); \
    pulCurrent += ulBlockSize; \
} while(0)

/* This macro does a scaler reset RUL. */
#define BVDC_P_TNTD_RESET_RUL(hTntd) \
{ \
    *pulCurrent++ = BRDC_OP_IMM_TO_REG(); \
    *pulCurrent++ = BRDC_REGISTER(hTntd->ulResetRegAddr); \
    *pulCurrent++ = hTntd->ulResetMask; \
    *pulCurrent++ = BRDC_OP_IMM_TO_REG(); \
    *pulCurrent++ = BRDC_REGISTER(hTntd->ulResetRegAddr); \
    *pulCurrent++ = 0; \
}

/* number of registers in one block. */
#define BVDC_P_TNTD_REGS_COUNT      \
    BVDC_P_REGS_ENTRIES(TNTD_0_REG_START, TNTD_0_REG_END)

#define BVDC_P_TNTD_LAST UINT32_C(-1)
#else
#define BVDC_P_TNTD_REGS_COUNT     1
#ifndef BCHP_VNET_B_CAP_0_SRC_SOURCE_TNTD_0
#define BCHP_VNET_B_CAP_0_SRC_SOURCE_TNTD_0 0
#endif
#endif

/* VNET */
#if (BVDC_P_SUPPORT_TNTD)
#define BVDC_P_Tntd_MuxAddr(TNTD)       (BCHP_VNET_F_TNTD_0_SRC + (TNTD)->eId * sizeof(uint32_t))
#define BVDC_P_Tntd_PostMuxValue(TNTD)  (BCHP_VNET_B_CAP_0_SRC_SOURCE_TNTD_0 + (TNTD)->eId)
#else
#define BVDC_P_Tntd_MuxAddr(TNTD)       (0)
#define BVDC_P_Tntd_PostMuxValue(TNTD)  (0)
#endif
#define BVDC_P_Tntd_SetVnet_isr(TNTD, ulSrcMuxValue, eVnetPatchMode) \
   BVDC_P_SubRul_SetVnet_isr(&((TNTD)->SubRul), ulSrcMuxValue, eVnetPatchMode)
#define BVDC_P_Tntd_UnsetVnet_isr(TNTD) \
   BVDC_P_SubRul_UnsetVnet_isr(&((TNTD)->SubRul))

/***************************************************************************
 * Tntd private data structures
 ***************************************************************************/
typedef struct BVDC_P_TntdConfigTbl
{
    const char *pchConfig;
    uint32_t    ulLPeakKernelCtrlDir5Blur;
    uint32_t    ulLPeakKernelCtrlDir5WideBlur;
    uint32_t    ulLPeakKernelCtrlDir5Scale;
    uint32_t    ulLPeakKernelCtrlDir5Mono;
    int32_t     alLPeakGainPos[15];
    int32_t     alLPeakGainNeg[15];
    int32_t     lLPeakIncoreGoffLowOffset1;
    int32_t     lLPeakIncoreGoffLowOffset2;
    int32_t     lLPeakIncoreGoffLowOffset3;
    int32_t     lLPeakIncoreGoffLowOffset4;
    int32_t     lLPeakIncoreGoffHighOffset1;
    int32_t     lLPeakIncoreGoffHighOffset2;
    int32_t     lLPeakIncoreGoffHighOffset3;
    int32_t     lLPeakIncoreGoffHighOffset4;
    int32_t     lLPeakIncoreGoffDir3Offset1;
    int32_t     lLPeakIncoreGoffDir3Offset2;
    int32_t     lLPeakIncoreGoffDir3Offset3;
    int32_t     lLPeakIncoreGoffDir3Offset4;
    int32_t     lLPeakIncoreGoffDir5Offset1;
    int32_t     lLPeakIncoreGoffDir5Offset2;
    int32_t     lLPeakIncoreGoffDir5Offset3;
    int32_t     lLPeakIncoreGoffDir5Offset4;
    uint32_t    ulLPeakIncoreThrHighT1;
    uint32_t    ulLPeakIncoreThrHighT2;
    uint32_t    ulLPeakIncoreDivHigh0OneOverT;
    uint32_t    ulLPeakIncoreDivHigh1OneOverT;
    uint32_t    ulLtiFilterHFilterSel;
    uint32_t    ulLtiFilterVFilterSel;
    uint32_t    ulLtiFilterBlurEn;
} BVDC_P_TntdConfigTbl;

typedef struct BVDC_P_TntdContext
{
    BDBG_OBJECT(BVDC_TNTD)

    /* flag initial state, requires reset; */
    bool                           bInitial;
    uint32_t                       ulResetRegAddr;
    uint32_t                       ulResetMask;
    uint32_t                       ulVnetResetAddr;
    uint32_t                       ulVnetResetMask;
    uint32_t                       ulVnetMuxAddr;
    uint32_t                       ulVnetMuxValue;

    /* update flag. */
    uint32_t                       ulUpdateAll;

    /* private fields. */
    BVDC_P_TntdId                  eId;
    uint32_t                       ulRegOffset; /* TNTD_0, and etc. */
    uint32_t                       aulRegs[BVDC_P_TNTD_REGS_COUNT];
    BREG_Handle                    hReg;
    BVDC_Window_Handle             hWindow;

    /* sub-struct to manage vnet and rul build opreations */
    BVDC_P_SubRulContext           SubRul;

    bool                           bSharpnessEnable;
    int16_t                        sSharpness;
    BVDC_SplitScreenMode           eDemoMode;
    uint32_t                       ulPrevWidth;
    uint32_t                       ulPrevHeight;
    bool                           bPrevTntdBeforeScl;
    BFMT_Orientation               ePrevSrcOrientation;
    BFMT_Orientation               ePrevDispOrientation;
    const BVDC_P_TntdConfigTbl    *pConfigTbl;
} BVDC_P_TntdContext;


/***************************************************************************
 * Tntd private functions
 ***************************************************************************/
BERR_Code BVDC_P_Tntd_Create
    ( BVDC_P_Tntd_Handle           *phTntd,
      BVDC_P_TntdId                 eTntdId,
      BVDC_P_Resource_Handle        hResource,
      BREG_Handle                   hReg );

void BVDC_P_Tntd_Destroy
    ( BVDC_P_Tntd_Handle            hTntd );

void BVDC_P_Tntd_Init
    ( BVDC_P_Tntd_Handle            hTntd,
      BVDC_Window_Handle            hWindow);

BERR_Code BVDC_P_Tntd_AcquireConnect_isr
    ( BVDC_P_Tntd_Handle            hTntd,
      BVDC_Window_Handle            hWindow );

BERR_Code BVDC_P_Tntd_ReleaseConnect_isr
    ( BVDC_P_Tntd_Handle           *phTntd );

void BVDC_P_Tntd_BuildRul_SetEnable_isr
    ( BVDC_P_Tntd_Handle            hTntd,
      BVDC_P_ListInfo              *pList );

void BVDC_P_Tntd_BuildRul_isr
    ( const BVDC_P_Tntd_Handle      hTntd,
      BVDC_P_ListInfo              *pList,
      BVDC_P_State                  eVnetState,
      BVDC_P_PicComRulInfo         *pPicComRulInfo );

void BVDC_P_Tntd_SetInfo_isr
    ( BVDC_P_Tntd_Handle            hTntd,
      const BVDC_P_PictureNodePtr   pPicture );

void BVDC_P_Tntd_SetEnable_isr
    ( BVDC_P_Tntd_Handle            hTntd,
      bool                          bEnable );

uint32_t BVDC_P_Tntd_CalcVertSclRatio_isr
    ( uint32_t                      ulInV,
      bool                          bInInterlaced,
      uint32_t                      ulOutV,
      bool                          bOutInterlaced );

#ifdef __cplusplus
}
#endif

#endif /* #ifndef BVDC_TNTD_PRIV_H__ */
/* End of file. */
