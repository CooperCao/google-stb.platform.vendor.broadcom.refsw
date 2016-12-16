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
#ifndef BVDC_XSRC_PRIV_H__
#define BVDC_XSRC_PRIV_H__

#include "bvdc.h"
#include "bchp_common.h"
#include "bvdc_common_priv.h"
#include "bvdc_subrul_priv.h"
#include "bvdc_buffer_priv.h"
#include "bvdc_scaler_priv.h"

#if (BVDC_P_SUPPORT_XSRC)
#include "bchp_xsrc_0.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Core revs to track increatmental changes! */
#define BVDC_P_SUPPORT_XSRC_VER_1             (1) /* 7445Dx */

/* No more UPDATE_SEL. */
#define BVDC_P_SUPPORT_XSRC_VER_2             (2) /* 7250Bx, 7439Bx, 7364Ax */

/* xsrc hw configuration */
#define BVDC_P_XSRC_SUPPORT_HSCL              BCHP_XSRC_0_HW_CONFIGURATION_HSCL_CORE_DEFAULT
#define BVDC_P_XSRC_SUPPORT_DERINE            BCHP_XSRC_0_HW_CONFIGURATION_DERINGING_DEFAULT

/* Cb/Cr Amplifier Configuration */
#ifdef BCHP_XSRC_0_HW_CONFIGURATION_CCA_DEFAULT
#define BVDC_P_XSRC_SUPPORT_CCA               (1)
#else
#define BVDC_P_XSRC_SUPPORT_CCA               (0)
#endif

/***************************************************************************
 * Private register cracking macros
 ***************************************************************************/
#if (BVDC_P_SUPPORT_XSRC)

#define BVDC_P_XSRC_GET_REG_IDX(reg) \
    ((BCHP##_##reg - BCHP_XSRC_0_REG_START) / sizeof(uint32_t))

/* Get/Set reg data */
#define BVDC_P_XSRC_GET_REG_DATA(hxs, reg) \
    ((hxs)->aulRegs[BVDC_P_XSRC_GET_REG_IDX(reg)])
#define BVDC_P_XSRC_SET_REG_DATA(hxs, reg, data) \
    (BVDC_P_XSRC_GET_REG_DATA((hxs), reg) = (uint32_t)(data))

/* Get with index. */
#define BVDC_P_XSRC_GET_REG_DATA_I(hxs, idx, reg) \
    ((hxs)->aulRegs[BVDC_P_XSRC_GET_REG_IDX(reg) + (idx)])

/* Get field */
#define BVDC_P_XSRC_GET_FIELD_NAME(hxs, reg, field) \
    (BVDC_P_GET_FIELD(BVDC_P_XSRC_GET_REG_DATA((hxs), reg), reg, field))

/* Compare field */
#define BVDC_P_XSRC_COMPARE_FIELD_DATA(hxs, reg, field, data) \
    (BVDC_P_COMPARE_FIELD_DATA(BVDC_P_XSRC_GET_REG_DATA((hxs), reg), reg, \
        field, (data)))
#define BVDC_P_XSRC_COMPARE_FIELD_NAME(hxs, reg, field, name) \
    (BVDC_P_COMPARE_FIELD_NAME(BVDC_P_XSRC_GET_REG_DATA((hxs), reg), reg, \
        field, name))

/* This macro does a write into RUL (write, addr, data). 3 dwords. */
#define BVDC_P_XSRC_WRITE_TO_RUL(hxs, reg, addr_ptr) \
do { \
    *addr_ptr++ = BRDC_OP_IMM_TO_REG(); \
    *addr_ptr++ = BRDC_REGISTER(BCHP##_##reg + (hxs)->ulRegOffset); \
    *addr_ptr++ = BVDC_P_XSRC_GET_REG_DATA((hxs), reg); \
} while (0)

/* This macro does a block write into RUL */
#define BVDC_P_XSRC_BLOCK_WRITE_TO_RUL(hxs, from, to, pulCurrent) \
do { \
    uint32_t ulBlockSize = \
        BVDC_P_REGS_ENTRIES(from, to);\
    *pulCurrent++ = BRDC_OP_IMMS_TO_REGS( ulBlockSize ); \
    *pulCurrent++ = BRDC_REGISTER(BCHP##_##from + (hxs)->ulRegOffset); \
    BKNI_Memcpy((void*)pulCurrent, \
        (void*)&((hxs)->aulRegs[BVDC_P_XSRC_GET_REG_IDX(from)]), \
        ulBlockSize * sizeof(uint32_t)); \
    pulCurrent += ulBlockSize; \
} while(0)

/* This macro does a scaler reset RUL. */
#define BVDC_P_XSRC_RESET_RUL(hxs) \
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
#define BVDC_P_XSRC_REGS_COUNT      \
    BVDC_P_REGS_ENTRIES(XSRC_0_REG_START, XSRC_0_REG_END)

#if BVDC_P_XSRC_SUPPORT_HSCL

/* set scaling regions */
#define BVDC_P_XSRC_SET_HORZ_REGION02(hxs, region, end_pixel, step_inc) \
do { \
    BVDC_P_XSRC_GET_REG_DATA((hxs), XSRC_0_HORIZ_FIR_DEST_PIC_REGION_##region##_STEP_DELTA) &= ~( \
        BCHP_MASK(XSRC_0_HORIZ_FIR_DEST_PIC_REGION_0_STEP_DELTA, SIZE)); \
    BVDC_P_XSRC_GET_REG_DATA((hxs), XSRC_0_HORIZ_FIR_DEST_PIC_REGION_##region##_STEP_DELTA) |=  ( \
        BCHP_FIELD_DATA(XSRC_0_HORIZ_FIR_DEST_PIC_REGION_0_STEP_DELTA, SIZE, step_inc)); \
    BVDC_P_XSRC_GET_REG_DATA((hxs), XSRC_0_HORIZ_DEST_PIC_REGION_##region##_END) &= ~( \
        BCHP_MASK(XSRC_0_HORIZ_DEST_PIC_REGION_0_END, POSITION)); \
    BVDC_P_XSRC_GET_REG_DATA((hxs), XSRC_0_HORIZ_DEST_PIC_REGION_##region##_END) |=  ( \
        BCHP_FIELD_DATA(XSRC_0_HORIZ_DEST_PIC_REGION_0_END, POSITION, end_pixel)); \
} while (0)

/* Set horizontal scale down ratio */
#define BVDC_P_XSRC_HORZ_SCALE_REGION          (1)
#define BVDC_P_XSRC_SET_HORZ_RATIO(hxs, ratio) \
do { \
    BVDC_P_XSRC_GET_REG_DATA((hxs), XSRC_0_HORIZ_FIR_INIT_STEP_FRAC) &= ~( \
        BCHP_MASK(XSRC_0_HORIZ_FIR_INIT_STEP_FRAC, FRAC_SIZE)); \
    BVDC_P_XSRC_GET_REG_DATA((hxs), XSRC_0_HORIZ_FIR_INIT_STEP_FRAC) |=  ( \
        BCHP_FIELD_DATA(XSRC_0_HORIZ_FIR_INIT_STEP_FRAC, FRAC_SIZE, \
                        ratio & ~(1 << BVDC_P_SCL_H_RATIO_F_BITS))); \
    BVDC_P_XSRC_GET_REG_DATA((hxs), XSRC_0_HORIZ_FIR_INIT_STEP_INT) &= ~( \
        BCHP_MASK(XSRC_0_HORIZ_FIR_INIT_STEP_INT, INT_SIZE)); \
    BVDC_P_XSRC_GET_REG_DATA((hxs), XSRC_0_HORIZ_FIR_INIT_STEP_INT) |=  ( \
        BCHP_FIELD_DATA(XSRC_0_HORIZ_FIR_INIT_STEP_INT, INT_SIZE, \
                        ratio >> BVDC_P_SCL_H_RATIO_F_BITS)); \
} while (0)

#define BVDC_P_XSRC_SET_HORZ_REGION02(hxs, region, end_pixel, step_inc) \
do { \
    BVDC_P_XSRC_GET_REG_DATA((hxs), XSRC_0_HORIZ_FIR_DEST_PIC_REGION_##region##_STEP_DELTA) &= ~( \
        BCHP_MASK(XSRC_0_HORIZ_FIR_DEST_PIC_REGION_0_STEP_DELTA, SIZE)); \
    BVDC_P_XSRC_GET_REG_DATA((hxs), XSRC_0_HORIZ_FIR_DEST_PIC_REGION_##region##_STEP_DELTA) |=  ( \
        BCHP_FIELD_DATA(XSRC_0_HORIZ_FIR_DEST_PIC_REGION_0_STEP_DELTA, SIZE, step_inc)); \
    BVDC_P_XSRC_GET_REG_DATA((hxs), XSRC_0_HORIZ_DEST_PIC_REGION_##region##_END) &= ~( \
        BCHP_MASK(XSRC_0_HORIZ_DEST_PIC_REGION_0_END, POSITION)); \
    BVDC_P_XSRC_GET_REG_DATA((hxs), XSRC_0_HORIZ_DEST_PIC_REGION_##region##_END) |=  ( \
        BCHP_FIELD_DATA(XSRC_0_HORIZ_DEST_PIC_REGION_0_END, POSITION, end_pixel)); \
} while (0)

#define BVDC_P_XSRC_SET_HORZ_STEP_MISC(hxs, end_pixel_1, step_init) \
do { \
    BVDC_P_XSRC_SET_HORZ_RATIO((hxs), step_init); \
    BVDC_P_XSRC_GET_REG_DATA((hxs), XSRC_0_HORIZ_DEST_PIC_REGION_1_END) &= ~( \
        BCHP_MASK(XSRC_0_HORIZ_DEST_PIC_REGION_1_END, POSITION)); \
    BVDC_P_XSRC_GET_REG_DATA((hxs), XSRC_0_HORIZ_DEST_PIC_REGION_1_END) |=  ( \
        BCHP_FIELD_DATA(XSRC_0_HORIZ_DEST_PIC_REGION_1_END, POSITION, end_pixel_1)); \
} while (0)
/* TBD: this macro is unused. Perhaps it should be eliminated. */

#endif

/* fixed point stuffs */
#define BVDC_P_XSRC_H_PAN_SCAN_I_BITS          (1)
#define BVDC_P_XSRC_H_PAN_SCAN_F_BITS          (6)

#define BVDC_P_XSRC_H_RATIO_I_BITS             (4)
#define BVDC_P_XSRC_H_RATIO_F_BITS             (17)

#define BVDC_P_XSRC_LUMA_INIT_PHASE_I_BITS     (11)
#define BVDC_P_XSRC_LUMA_INIT_PHASE_F_BITS     (6)

#define BVDC_P_XSRC_CHROMA_INIT_PHASE_I_BITS   (11)
#define BVDC_P_XSRC_CHROMA_INIT_PHASE_F_BITS   (6)

#define BVDC_P_XSRC_COEFFS_I_BITS              (1)
#define BVDC_P_XSRC_COEFFS_F_BITS              (10)

#define BVDC_P_XSRC_LARGEST_F_BITS             (17)
#define BVDC_P_XSRC_ZERO_F_BITS                (0)

/* to normalize everything into S14.17 fixed format */
#define BVDC_P_XSRC_NORMALIZE(value, f_bit) ((value) << (BVDC_P_XSRC_LARGEST_F_BITS - (f_bit)))

/* to innormalize everything from S14.17 fixed format */
#define BVDC_P_XSRC_NORM_2_SPEC(value, f_bit) ((value) >> (BVDC_P_XSRC_LARGEST_F_BITS - (f_bit)))

/* Miscellaneous constants */
#define BVDC_P_XSRC_HORZ_REGIONS_COUNT         (1)

#define BVDC_P_XSRC_HORZ_FIR_TAP_COUNT         (8)
#define BVDC_P_XSRC_HORZ_FIR_PHASE_COUNT       (8)

#define BVDC_P_XSRC_FIR_TAP_COUNT_MAX          (8)
#define BVDC_P_XSRC_FIR_PHASE_COUNT_MAX        (8)

#define BVDC_P_XSRC_4TAP_HORZ_THRESHOLD_0      (1280)
#define BVDC_P_XSRC_4TAP_HORZ_THRESHOLD_1      (1024)

#define BVDC_P_XSRC_HORZ_HWF_FACTOR            (2)
#define BVDC_P_XSRC_SRC_HORZ_THRESHOLD         (960)

/* Make Horizontal ratio */
#define BVDC_P_XSRC_MAKE_H_RATIO(src, dst) \
    (BVDC_P_XSRC_NORM_2_SPEC((src), BVDC_P_XSRC_H_RATIO_F_BITS) / (dst))

#define BVDC_P_XSRC_HORZ_1_FIXED BVDC_P_FLOAT_TO_FIXED(1.000, \
    BVDC_P_XSRC_LUMA_INIT_PHASE_I_BITS, BVDC_P_XSRC_LUMA_INIT_PHASE_F_BITS)

#define BVDC_P_XSRC_FIR_COEFFS_MAX \
    (BVDC_P_XSRC_FIR_TAP_COUNT_MAX * BVDC_P_XSRC_FIR_PHASE_COUNT_MAX)

#define BVDC_P_XSRC_LAST UINT32_C(-1)
#else
#define BVDC_P_XSRC_REGS_COUNT     1
#ifndef BCHP_VNET_B_CAP_0_SRC_SOURCE_XSRC_0
#define BCHP_VNET_B_CAP_0_SRC_SOURCE_XSRC_0 0
#endif
#endif

/* VNET */
#if (BVDC_P_SUPPORT_XSRC)
#define BVDC_P_Xsrc_MuxAddr(XSRC)       (BCHP_VNET_F_XSRC_0_SRC + (XSRC)->eId * sizeof(uint32_t))
#define BVDC_P_Xsrc_PostMuxValue(XSRC)  (BCHP_VNET_B_CAP_0_SRC_SOURCE_XSRC_0 + (XSRC)->eId)
#else
#define BVDC_P_Xsrc_MuxAddr(XSRC)       (0)
#define BVDC_P_Xsrc_PostMuxValue(XSRC)  (0)
#endif
#define BVDC_P_Xsrc_SetVnet_isr(XSRC, ulSrcMuxValue, eVnetPatchMode) \
   BVDC_P_SubRul_SetVnet_isr(&((XSRC)->SubRul), ulSrcMuxValue, eVnetPatchMode)
#define BVDC_P_Xsrc_UnsetVnet_isr(XSRC) \
   BVDC_P_SubRul_UnsetVnet_isr(&((XSRC)->SubRul))
#define BVDC_P_Xsrc_SetRulBuildWinId_isr(hxs, eWinId) \
    BVDC_P_SubRul_SetRulBuildWinId_isr(&((hxs)->SubRul), eWinId)


/***************************************************************************
 * Xsrc private data structures
 ***************************************************************************/
typedef struct BVDC_P_XsrcContext
{
    BDBG_OBJECT(BVDC_XSRC)

    /* flag initial state, requires reset; */
    bool                           bInitial;
    uint32_t                       ulResetRegAddr;
    uint32_t                       ulResetMask;
    uint32_t                       ulVnetResetAddr;
    uint32_t                       ulVnetResetMask;
    uint32_t                       ulVnetMuxAddr;
    uint32_t                       ulVnetMuxValue;

    /* Fir coeff tables */
    const BVDC_P_FirCoeffTbl      *pHorzFirCoeffTbl;
    const BVDC_P_FirCoeffTbl      *pChromaHorzFirCoeffTbl;

    /* update flag. */
    uint32_t                       ulUpdateAll;

    BVDC_Source_Handle             hSource;

    uint32_t                       ulPrevSrcWidth;
    int32_t                        lPrevXsrcCutLeft;
    int32_t                        lPrevXsrcCutLeft_R;
    uint32_t                       ulPrevXsrcCutWidth;
    uint32_t                       ulPrevOutWidth;
    uint32_t                       ulPrevSrcHeight;
    uint32_t                       ulPrevCtIndexLuma;
    uint32_t                       ulPrevCtIndexChroma;
    BVDC_P_CtInput                 ePrevCtInputType;
    bool                           bPrevSrc10Bit;
    BVDC_P_DitherSetting           stDither;

    /* private fields. */
    BVDC_P_XsrcId                  eId;
    uint32_t                       ulRegOffset; /* XSRC_0, XSRC_1, and etc. */
    uint32_t                       aulRegs[BVDC_P_XSRC_REGS_COUNT];
    BREG_Handle                    hReg;

    /* sub-struct to manage vnet and rul build opreations */
    BVDC_P_SubRulContext           SubRul;

    uint32_t                       ulHorzTaps;

    bool                           bDeringing;
    bool                           bDithering;
    bool                           bHscl;
    bool                           bCca;

    BFMT_Orientation               ePrevSrcOrientation;
    BFMT_Orientation               ePrevDispOrientation;

    bool                           bChannelChange;
    /* src width alignment: 1 for 444 format internally, 2 for 422 format internally */
    uint32_t                       ulSrcHrzAlign;
} BVDC_P_XsrcContext;


/***************************************************************************
 * Xsrc private functions
 ***************************************************************************/
BERR_Code BVDC_P_Xsrc_Create
    ( BVDC_P_Xsrc_Handle           *phXsrc,
      BVDC_P_XsrcId                 eXsrcId,
      BVDC_P_Resource_Handle        hResource,
      BREG_Handle                   hReg );

void BVDC_P_Xsrc_Destroy
    ( BVDC_P_Xsrc_Handle            hXsrc );

void BVDC_P_Xsrc_Init_isr
    ( BVDC_P_Xsrc_Handle            hXsrc );

BERR_Code BVDC_P_Xsrc_AcquireConnect_isr
    ( BVDC_P_Xsrc_Handle            hXsrc,
      BVDC_Source_Handle            hSource );

BERR_Code BVDC_P_Xsrc_ReleaseConnect_isr
    ( BVDC_P_Xsrc_Handle           *phXsrc );

void BVDC_P_Xsrc_BuildRul_SetEnable_isr
    ( BVDC_P_Xsrc_Handle            hXsrc,
      BVDC_P_ListInfo              *pList );

void BVDC_P_Xsrc_BuildRul_isr
    ( const BVDC_P_Xsrc_Handle      hXsrc,
      BVDC_P_ListInfo              *pList,
      BVDC_P_State                  eVnetState,
      BVDC_P_PicComRulInfo         *pPicComRulInfo );

void BVDC_P_Xsrc_SetInfo_isr
    ( BVDC_P_Xsrc_Handle            hXsrc,
      BVDC_Window_Handle            hWindow,
      const BVDC_P_PictureNodePtr   pPicture );

void BVDC_P_Xsrc_SetEnable_isr
    ( BVDC_P_Xsrc_Handle            hXsrc,
      bool                          bEnable );

#ifdef __cplusplus
}
#endif

#endif /* #ifndef BVDC_XSRC_PRIV_H__ */
/* End of file. */
