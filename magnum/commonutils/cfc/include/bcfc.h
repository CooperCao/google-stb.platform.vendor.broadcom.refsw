/******************************************************************************
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
 ******************************************************************************/
#ifndef BCFC_H__
#define BCFC_H__

/*=Module Overview: ********************************************************
There are CFC (Color Format Converter) hardware modules in window compositor,
vec, graphics feeder, and also m2mc. This module provides core functionality
for CFC.
****************************************************************************/

#include "bcfc_types.h"
#include "bavc_types.h"
#include "bmma.h"
#include "bmth_fix.h"
#include "bmth_fix_matrix.h"
#include "bchp_common.h"
#ifdef BCHP_HDR_CMP_0_REG_START
#include "bchp_hdr_cmp_0.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if defined(BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_SEL_LMR_USE_LMR_R1)
#define BCFC_VERSION     (4)
#elif defined(BCHP_HDR_CMP_0_LUT_DESC_CFG)
#define BCFC_VERSION     (3)
#elif defined(BCHP_HDR_CMP_0_REG_START)
#define BCFC_VERSION     (2)
#elif defined(BCHP_CMP_0_V0_NL_CSC_CTRL)
#define BCFC_VERSION     (1)
#else
#define BCFC_VERSION     (0)
#endif

#define BCFC_VER_4       (4)
#define BCFC_VER_3       (3)
#define BCFC_VER_2       (2)
#define BCFC_VER_1       (1)
#define BCFC_VER_0       (0)

#define BCFC_IS_SDR(tf) \
    ((tf)==BCFC_ColorTF_eBt1886)

#define BCFC_IS_HLG(tf) \
    ((tf)==BCFC_ColorTF_eHlg)

#define BCFC_IS_HDR10(tf) \
    ((tf)==BCFC_ColorTF_eBt2100Pq)

#define BCFC_IS_TCH(dataType) \
    ((dataType)==BAVC_HdrMetadataType_eTch_Slhdr)

#define BCFC_NEED_TF_CONV(i, o) \
    (!(((i) == (o)) || \
       (((i)==BCFC_ColorTF_eHlg) && (o)== BCFC_ColorTF_eBt1886)))

#define BCFC_IS_BT2020(c) \
    (BCFC_Colorimetry_eBt2020 == (c))

#define BCFC_IS_HLG_TO_HDR10(i, o) \
    ((i == BCFC_ColorTF_eHlg) && (o == BCFC_ColorTF_eBt2100Pq))

#define BCFC_IS_SDR_TO_HDR10(i, o) \
    ((i == BCFC_ColorTF_eBt1886) && (o == BCFC_ColorTF_eBt2100Pq))

#define BCFC_IS_HLG_TO_SDR(i, o) \
    ((i == BCFC_ColorTF_eHlg) && (o == BCFC_ColorTF_eBt1886))

#define BCFC_IS_HDR10_TO_SDR(i, o) \
    ((i == BCFC_ColorTF_eBt2100Pq) && (o == BCFC_ColorTF_eBt1886))

#define BCFC_IS_SDR_TO_HLG(i, o) \
    ((i == BCFC_ColorTF_eBt1886) && (o == BCFC_ColorTF_eHlg))

#define BCFC_IS_HDR10_TO_HLG(i, o) \
    ((i == BCFC_ColorTF_eBt2100Pq) && (o == BCFC_ColorTF_eHlg))

#define BCFC_IN_CMP0V0(id)   ((id) == BCFC_Id_eComp0_V0)

#define BCFC_IN_CMP0(id)     ((id) <= BCFC_Id_eComp0_V1)

#define BCFC_IN_CMP(id)      ((id) <= BCFC_Id_eComp6_V0)

#define BCFC_IN_GFD0(id)     ((id) == BCFC_Id_eComp0_G0)

#define BCFC_IN_GFD(id)      (((id) >= BCFC_Id_eComp0_G0) && ((id) <= BCFC_Id_eComp6_G0))

#define BCFC_IN_VFC(id)      (((id) >= BCFC_Id_eVfc0) && ((id) <= BCFC_Id_eVfcMax))

#define BCFC_IN_DVI(id)      ((id) == BCFC_Id_eDisplay0)

#define BCFC_CSC_SW_MAX_BITS             (31)
#define BCFC_CSC_SW_MASK         (0x7FFFFFFF)
#define BCFC_CSC_SW_SIGN_MASK    (0x80000000)
#define BCFC_CSC_SW_CX_I_BITS             (2)
#define BCFC_CSC_SW_CX_F_BITS            (29)
#define BCFC_CSC_SW_CO_I_BITS             (9)
#define BCFC_CSC_SW_CO_F_BITS            (22)
#define BCFC_CSC_SW_F_CX_CO_DIFF_BITS    (BCFC_CSC_SW_CX_F_BITS - BCFC_CSC_SW_CO_F_BITS)

#define BCFC_NL_SEL_BYPASS             (0xFF)

/*-------------------------------------------------------------------------
 * matrix
 */

/* Software copy of 3x4 Color Conversion Matrix, such as Ma for YCbCr2Rgb,
 * Ipt2Lsm, .... ; or Mc (Rgb2YCbCr, Lms2Ipt, ...), or blend_in, blend_out,
 * or dvi_csc ...
 */
typedef struct
{
    int32_t     m[3][4];
} BCFC_Csc3x4;

/* Software copy of 3x3 Matrix, such as Mb for input (Rgb2Xyz,  Lms2Xyz, ..)
 * and output (Xyz2Rgb, Xyz2Lms, ...).
 * The final Matrix Mb will be the product of input Mb and output Mb.
 */
typedef struct
{
    int32_t     m[3][3];
} BCFC_Csc3x3;

/* --------------------------------------------------------------------
 * make matrix
 */
#define BCFC_P_MAKE_CX(m) \
    BMTH_FIX_SIGNED_FTOFIX(m, BCFC_CSC_SW_CX_I_BITS, BCFC_CSC_SW_CX_F_BITS)
#define BCFC_P_MAKE_CO(m) \
    BMTH_FIX_SIGNED_FTOFIX(m, BCFC_CSC_SW_CO_I_BITS, BCFC_CSC_SW_CO_F_BITS)

/* 3x4 matrix, for A or C */
#define BCFC_MAKE_CSC_3x4(m00, m01, m02, m03,  \
                          m10, m11, m12, m13,  \
                          m20, m21, m22, m23 ) \
{{                                             \
     { BCFC_P_MAKE_CX(m00),   \
       BCFC_P_MAKE_CX(m01),   \
       BCFC_P_MAKE_CX(m02),   \
       BCFC_P_MAKE_CO(m03) }, \
                              \
     { BCFC_P_MAKE_CX(m10),   \
       BCFC_P_MAKE_CX(m11),   \
       BCFC_P_MAKE_CX(m12),   \
       BCFC_P_MAKE_CO(m13) }, \
                              \
     { BCFC_P_MAKE_CX(m20),   \
       BCFC_P_MAKE_CX(m21),   \
       BCFC_P_MAKE_CX(m22),   \
       BCFC_P_MAKE_CO(m23) }  \
}}

/* 3x4 matrix, for B */
#define BCFC_MAKE_CSC_3x3(m00, m01, m02,  \
                          m10, m11, m12,  \
                          m20, m21, m22 ) \
{{                            \
     { BCFC_P_MAKE_CX(m00),   \
       BCFC_P_MAKE_CX(m01),   \
       BCFC_P_MAKE_CX(m02) }, \
                              \
     { BCFC_P_MAKE_CX(m10),   \
       BCFC_P_MAKE_CX(m11),   \
       BCFC_P_MAKE_CX(m12) }, \
                              \
     { BCFC_P_MAKE_CX(m20),   \
       BCFC_P_MAKE_CX(m21),   \
       BCFC_P_MAKE_CX(m22) }  \
}}

/*-------------------------------------------------------------------------
 * HW matrix types: with info of int / fraction bits, row/colum numbers, packed or not, ...
 */

#define BCFC_P_CSC_GFD_WITH_BLEND(t) \
    ((t) >= (6<<16))
#define BCFC_CSC_GFD_CONST_BLEND(t) \
    (BCFC_P_CSC_GFD_WITH_BLEND(t) && (((t) & (1<<16)) == 0))
#define BCFC_CSC_GFD_ALPHA_BLEND(t) \
    (BCFC_P_CSC_GFD_WITH_BLEND(t) && (((t) & (1<<16)) != 0))

#define BCFC_CSC_ROWS(t) \
    (((t) >> 12) & 0xF)
#define BCFC_CSC_COLUMS(t) \
    (((t) >> 8) & 0xF)

#define BCFC_CSC_CX_F_BITS(t) \
    ((t) & 0xFF)
#define BCFC_CSC_CO_F_BITS(t) \
    (BCFC_CSC_CX_F_BITS(t) - 7)
#define BCFC_CSC_MASK(t) \
    ((1 << (1 + 2 + BCFC_CSC_CX_F_BITS(t))) - 1)

#define BCFC_CSC_NAME_IDX(t) \
    ((t) >> 16)

typedef enum
{
    /* name convention: the last number in name is the total number of bits of one element */
    /*                      CX F bits  row|column  name idx */
    BCFC_CscType_eMa3x4_16 =    (13) | (0x34<<8) | (0<<16),  /* s2.13 / s9.6:  CMP MA in 7439 B0 */
    BCFC_CscType_eMa3x4_25 =    (22) | (0x34<<8) | (0<<16),  /* s2.22 / s9.15: CMP MA in 7271 A0, DVI MA in 7271 B0/up */
    BCFC_CscType_eMa5x4_25 =    (22) | (0x54<<8) | (0<<16),  /* s2.22 / s9.15: CMP/VFC MA in 7271 B0 and up */

    BCFC_CscType_eMb3x4_25 =    (22) | (0x34<<8) | (1<<16),  /* s2.22 / s9.15: CMP/GFD/VFC MB in 7271 A0/up, DVI MB in 7271 B0/up */

    BCFC_CscType_eMc3x4_16 =    (13) | (0x34<<8) | (2<<16),  /* s2.13 / s9.6:  CMP/VFC MC */
    BCFC_CscType_eMc5x4_16 =    (13) | (0x54<<8) | (2<<16),  /* s2.13 / s9.6:  DVI MC in 7271 B0 and up */
    BCFC_CscType_eMcPacked =    (13) | (0x32<<8) | (2<<16),  /* s2.13 / s9.6:  DVI MC with 2 elemnts in one reg, 7271 A0 / older */

    BCFC_CscType_eMbi3x4_16 =   (13) | (0x34<<8) | (3<<16),  /* s2.13 / s9.6:  CMP blend-in,  7271 A0 and up */
    BCFC_CscType_eMbo3x4_16 =   (13) | (0x34<<8) | (4<<16),  /* s2.13 / s9.6:  CMP blend-out, 7271 A0 and up */

    BCFC_CscType_eMb1x4_25 =    (22) | (0x14<<8) | (5<<16),  /* s2.22 / s9.15: CMP / GFD MB2 for 7271 B0 and up */

    BCFC_CscType_eMa3x5_16CB =  (13) | (0x35<<8) | (6<<16),  /* s2.22 / s9.15: GFD MA with constant blending, 7439 B0 */
    BCFC_CscType_eMa3x5_16AB =  (13) | (0x35<<8) | (7<<16),  /* s2.22 / s9.15: GFD MA with alpha blending, 7439 B0 */
    BCFC_CscType_eMa3x5_25CB =  (22) | (0x35<<8) | (6<<16),  /* s2.22 / s9.15: GFD MA with constant blending, 7271 A0/up */
    BCFC_CscType_eMa3x5_25AB =  (22) | (0x35<<8) | (7<<16),  /* s2.22 / s9.15: GFD MA with alpha blending, 7271 A0/up */

    BCFC_CscType_eMc3x5_16CB =  (13) | (0x35<<8) | (8<<16),  /* s2.13 / s9.6:  GFD MC with constant blending, 7271 A0/up */
    BCFC_CscType_eMc3x5_16AB =  (13) | (0x35<<8) | (9<<16),  /* s2.13 / s9.6:  GFD MC with alpha blending,    7271 A0/up */

    BCFC_CscType_eMbi3x5_16CB = (13) | (0x35<<8) | (10<<16), /* s2.13 / s9.6:  GFD Blend-in with constant blending, 7271 B0/up */
    BCFC_CscType_eMbi3x5_16AB = (13) | (0x35<<8) | (11<<16), /* s2.13 / s9.6:  GFD Blend-in with alpha blending,    7271 B0/up */
    BCFC_CscType_eInvalid = 0
} BCFC_CscType;

/* HW matrix left shift handling
 */
#define BCFC_LSHIFT_BITS(s) \
    ((s) >> 4)
#define BCFC_NEED_WRITE_LSHIFT_REG(s) \
    ((s) & 0x1)

typedef enum
{
    BCFC_LeftShift_eNotExist =     0, /* matrix in old chips does not have left shift */
    BCFC_LeftShift_eOff = (0<<4) | 1, /* not shift, but program LSHIFT reg */
    BCFC_LeftShift_e1_1 = (1<<4) | 1, /* lshift 1 bit, program LSHIFT reg */
    BCFC_LeftShift_e2_1 = (2<<4) | 1, /* lshift 2 bit, program LSHIFT reg */
    BCFC_LeftShift_e1_0 = (1<<4) | 0, /* lshift 1 bit, NOT program LSHIFT reg */
    BCFC_LeftShift_e2_0 = (2<<4) | 0, /* lshift 2 bit, NOT program LSHIFT reg */
    BCFC_LeftShift_eExist = BCFC_LeftShift_eOff, /* for dvs, program LSHIFT reg with BDVS_Control_Csc.uiLshf */
    BCFC_LeftShift_eMax
} BCFC_LeftShift;

/* we combine BCFC_CscType and BCFC_LeftShift into one uint32_t to pass one less parameter
 * in BVDC_P_Cfc_BuildCscRul
 */
#define BCFC_MAKE_CSC_CFG(t, s) \
    ((t) | ((s) << 24))
#define BCFC_GET_CSC_TYPE(c) \
    ((c) & 0x00FFFFFF)
#define BCFC_GET_CSC_LSHIFT(c) \
    ((c) >> 24)

/*-------------------------------------------------------------------------
 * LRange Adjust
 */
#define BCFC_LR_ADJ_PTS             (8)

#define BCFC_LR_ADJ_LIMIT_NOT_EXIST   0   /* for 7270 A0 and older */
#define BCFC_LR_ADJ_LIMIT_ALWAYS_ON   1   /* for DVI-CFC 7271 B0 and up */
#define BCFC_LR_ADJ_LIMIT_DISABLE     2   /* for CMP/GFD 7270 B0 and up */
#define BCFC_LR_ADJ_LIMIT_ENABLE      3   /* for CMP/GFD 7270 B0 and up */
#define BCFC_LR_ADJ_LIMIT_I_BITS      1
#define BCFC_LR_ADJ_LIMIT_F_BITS     26
typedef struct
{
    uint32_t   ulLRangeAdjNumPoints;
    uint32_t   aulLRangeAdjSlope[BCFC_LR_ADJ_PTS];
    uint32_t   aulLRangeAdjXY[BCFC_LR_ADJ_PTS];

} BCFC_LRangeAdjTable;

typedef struct BCFC_LRangeAdj
{
    uint32_t                    ulLRangeAdjCtrl;
    uint32_t                    ulMin;
    uint32_t                    ulMax;

    const BCFC_LRangeAdjTable  *pTable;

} BCFC_LRangeAdj;

/* --------------------------------------------------------------------
 * make L-Range Adjust
 */
#define BCFC_P_LR_XY_I_BITS          (1)
#define BCFC_P_LR_XY_F_BITS         (15)
#define BCFC_P_LR_SLP_M_I_BITS       (0)
#define BCFC_P_LR_SLP_M_F_BITS      (15)
#define BCFC_P_LR_SLP_E_I_BITS       (4)
#define BCFC_P_LR_SLP_E_F_BITS       (0)

#define BCFC_LR_XY_F_BITS         BCFC_P_LR_XY_F_BITS

#ifdef BCHP_HDR_CMP_0_V0_R0_LR_XY_0i_LRA_X_SHIFT
#define BCFC_P_LR_X_SHIFT           BCHP_HDR_CMP_0_V0_R0_LR_XY_0i_LRA_X_SHIFT
#define BCFC_P_LR_Y_SHIFT           BCHP_HDR_CMP_0_V0_R0_LR_XY_0i_LRA_Y_SHIFT
#define BCFC_P_LR_SLP_M_SHIFT       BCHP_HDR_CMP_0_V0_R0_LR_SLOPEi_SLOPE_M_SHIFT
#define BCFC_P_LR_SLP_E_SHIFT       BCHP_HDR_CMP_0_V0_R0_LR_SLOPEi_SLOPE_E_SHIFT

#elif defined(BCHP_HDR_CMP_0_V0_R0_NL_LR_XY_0i_LRA_X_SHIFT)
#define BCFC_P_LR_X_SHIFT           BCHP_HDR_CMP_0_V0_R0_NL_LR_XY_0i_LRA_X_SHIFT
#define BCFC_P_LR_Y_SHIFT           BCHP_HDR_CMP_0_V0_R0_NL_LR_XY_0i_LRA_Y_SHIFT
#define BCFC_P_LR_SLP_M_SHIFT       BCHP_HDR_CMP_0_V0_R0_NL_LR_SLOPEi_SLOPE_M_SHIFT
#define BCFC_P_LR_SLP_E_SHIFT       BCHP_HDR_CMP_0_V0_R0_NL_LR_SLOPEi_SLOPE_E_SHIFT

#else /* TODO: add support */
#define BCFC_P_LR_X_SHIFT           0
#define BCFC_P_LR_Y_SHIFT           0
#define BCFC_P_LR_SLP_M_SHIFT       0
#define BCFC_P_LR_SLP_E_SHIFT       0
#endif

#define BCFC_P_MAKE_LR_XY(x, y) \
    (((uint16_t)BMTH_FIX_SIGNED_FTOFIX(x, BCFC_P_LR_XY_I_BITS, BCFC_P_LR_XY_F_BITS)) << BCFC_P_LR_X_SHIFT) | \
    (((uint16_t)BMTH_FIX_SIGNED_FTOFIX(y, BCFC_P_LR_XY_I_BITS, BCFC_P_LR_XY_F_BITS)) << BCFC_P_LR_Y_SHIFT)

#define BCFC_P_MAKE_LR_SLOPE(m,e) \
    (((uint16_t)BMTH_FIX_SIGNED_FTOFIX(m, BCFC_P_LR_SLP_M_I_BITS, BCFC_P_LR_SLP_M_F_BITS)) << BCFC_P_LR_SLP_M_SHIFT) | \
    (((uint16_t)BMTH_FIX_SIGNED_FTOFIX(e, BCFC_P_LR_SLP_E_I_BITS, BCFC_P_LR_SLP_E_F_BITS)) << BCFC_P_LR_SLP_E_SHIFT)

#define BCFC_MAKE_LR_ADJ(num_pts,                                 \
                           x0, y0, mantissa0, exp0,               \
                           x1, y1, mantissa1, exp1,               \
                           x2, y2, mantissa2, exp2,               \
                           x3, y3, mantissa3, exp3,               \
                           x4, y4, mantissa4, exp4,               \
                           x5, y5, mantissa5, exp5,               \
                           x6, y6, mantissa6, exp6,               \
                           x7, y7, mantissa7, exp7)               \
{                                                                 \
    num_pts,                                                      \
    {                                                             \
        BCFC_P_MAKE_LR_SLOPE(mantissa0, exp0),                    \
        BCFC_P_MAKE_LR_SLOPE(mantissa1, exp1),                    \
        BCFC_P_MAKE_LR_SLOPE(mantissa2, exp2),                    \
        BCFC_P_MAKE_LR_SLOPE(mantissa3, exp3),                    \
        BCFC_P_MAKE_LR_SLOPE(mantissa4, exp4),                    \
        BCFC_P_MAKE_LR_SLOPE(mantissa5, exp5),                    \
        BCFC_P_MAKE_LR_SLOPE(mantissa6, exp6),                    \
        BCFC_P_MAKE_LR_SLOPE(mantissa7, exp7)                     \
    },                                                            \
    {                                                             \
        BCFC_P_MAKE_LR_XY(x0, y0),                                \
        BCFC_P_MAKE_LR_XY(x1, y1),                                \
        BCFC_P_MAKE_LR_XY(x2, y2),                                \
        BCFC_P_MAKE_LR_XY(x3, y3),                                \
        BCFC_P_MAKE_LR_XY(x4, y4),                                \
        BCFC_P_MAKE_LR_XY(x5, y5),                                \
        BCFC_P_MAKE_LR_XY(x6, y6),                                \
        BCFC_P_MAKE_LR_XY(x7, y7)                                 \
    }                                                             \
}

/*-------------------------------------------------------------------------
 * Ram LUT
 */
#define BCFC_MAX_LUT_SEGS   8
typedef struct
{
    uint8_t    ucNumSeg;
    uint8_t    ucXScale;  /* U1.2 */
    uint8_t    ucOutIntBits;
    uint8_t    ucOutFracBits;

    uint16_t   usSegBinEnd[BCFC_MAX_LUT_SEGS];
    uint16_t   usSegOffset[BCFC_MAX_LUT_SEGS];
    uint16_t   usSegIntBits[BCFC_MAX_LUT_SEGS];

    const uint32_t  *pulTable;

} BCFC_RamLut;

/*-------------------------------------------------------------------------
 * CFC ColorSpace (public)
 */
typedef struct BCFC_ColorSpace
{
    BCFC_ColorFormat          eColorFmt;
    BCFC_Colorimetry          eColorimetry;
    BCFC_ColorRange           eColorRange;
    BCFC_ColorTF              eColorTF;
    BCFC_ColorDepth           eColorDepth;

    struct {
        /* Content Light SEI message */
        uint32_t              ulAvgContentLight; /* Pic Average Light level */
        uint32_t              ulMaxContentLight; /* Max Light level */

        /* Mastering Display Colour Volume */
        BAVC_Point            stDisplayPrimaries[3];
        BAVC_Point            stWhitePoint;
        uint32_t              ulMaxDispMasteringLuma; /* in 0.0001 nit */
        uint32_t              ulMinDispMasteringLuma; /* in 0.0001 nit */
    } stHdrParm;

    /* */
    void *                    pMetaData;
} BCFC_ColorSpace;

#define BCFC_COLOR_SPACE_DIFF(c1, c2) \
    (((c1)->eColorFmt    != (c2)->eColorFmt) || \
     ((c1)->eColorimetry != (c2)->eColorimetry) || \
     ((c1)->eColorTF     != (c2)->eColorTF) || \
     /*((c1)->eColorDepth  != (c2)->eColorDepth) ||*/ \
     ((c1)->eColorRange  != (c2)->eColorRange))

/*-------------------------------------------------------------------------
 * (input or output) one ColorSpace decided CFC configure
 *
 */
typedef union
{
    struct
    {
        uint32_t                 SelTF                    : 4;  /* 0 */
        uint32_t                 bEnDlbvCvm               : 1;  /* 4 */
        uint32_t                 bEnHlgAdj                : 1;
        uint32_t                 bEnTpToneMap             : 1;
        uint32_t                 bSelXvYcc                : 1;
        uint32_t                 bSelCL                   : 1;  /* 8 */
        uint32_t                 bSel14bit422To444        : 1;
        uint32_t                 bDirty                   : 1;  /* for software update */

    } stBits;

    uint32_t ulInts;
} BCFC_ColorSpace_Config;

/*-------------------------------------------------------------------------
 * Color Space externsion
 *
 */
typedef struct BCFC_ColorSpaceExt
{
    BCFC_ColorSpace           stColorSpace;

    BCFC_ColorSpace_Config    stCfg;

    /* non dynamically changing matrix might be a ptr in future ??? */
    BCFC_Csc3x4               stM3x4; /* ColorRange adjusted Ma for input, Mc for display */
    BCFC_Csc3x3               stMb;   /* 2XYZ for input, fromXYZ for output */
    BCFC_Csc3x4               stMalt; /* CL pos alt, Ma with input, Mc with display */

} BCFC_ColorSpaceExt;


/* --------------------------------------------------------------------
 * TF (Transfer Function) related
 */

#ifdef BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_SEL_NL2L_BT709
#define BCFC_NL2L_709     BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_SEL_NL2L_BT709
#define BCFC_NL2L_1886    BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_SEL_NL2L_BT1886
#define BCFC_NL2L_PQ      BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_SEL_NL2L_PQ
#define BCFC_NL2L_BBC     BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_SEL_NL2L_RESERVED
#define BCFC_NL2L_RAM     BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_SEL_NL2L_RAM
#define BCFC_NL2L_BYPASS  BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_SEL_NL2L_BYPASS

#define BCFC_L2NL_709     BCHP_HDR_CMP_0_CMP_HDR_V0_CTRL_SEL_L2NL_BT709
#define BCFC_L2NL_1886    BCHP_HDR_CMP_0_CMP_HDR_V0_CTRL_SEL_L2NL_BT1886
#define BCFC_L2NL_PQ      BCHP_HDR_CMP_0_CMP_HDR_V0_CTRL_SEL_L2NL_PQ
#define BCFC_L2NL_BBC     BCHP_HDR_CMP_0_CMP_HDR_V0_CTRL_SEL_L2NL_RESERVED
#define BCFC_L2NL_RAM     BCHP_HDR_CMP_0_CMP_HDR_V0_CTRL_SEL_L2NL_RAM
#define BCFC_L2NL_BYPASS  BCHP_HDR_CMP_0_CMP_HDR_V0_CTRL_SEL_L2NL_BYPASS

#elif defined(BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_RECT1_SEL_NL2L_709)
#define BCFC_NL2L_709     BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_RECT1_SEL_NL2L_709
#define BCFC_NL2L_1886    BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_RECT1_SEL_NL2L_1886
#define BCFC_NL2L_PQ      BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_RECT1_SEL_NL2L_PQ
#define BCFC_NL2L_BBC     BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_RECT1_SEL_NL2L_BBC
#define BCFC_NL2L_RAM     BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_RECT1_SEL_NL2L_RAM
#define BCFC_NL2L_BYPASS  BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_RECT1_SEL_NL2L_BYPASS

#define BCFC_L2NL_709     BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_RECT1_SEL_L2NL_709
#define BCFC_L2NL_1886    BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_RECT1_SEL_L2NL_1886
#define BCFC_L2NL_PQ      BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_RECT1_SEL_L2NL_PQ
#define BCFC_L2NL_BBC     BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_RECT1_SEL_L2NL_BBC
#define BCFC_L2NL_RAM     BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_RECT1_SEL_L2NL_RAM
#define BCFC_L2NL_BYPASS  BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_RECT1_SEL_L2NL_BYPASS
#else /* TODO: add new HDR support */
#define BCFC_NL2L_709     0
#define BCFC_NL2L_1886    0
#define BCFC_NL2L_PQ      0
#define BCFC_NL2L_BBC     0
#define BCFC_NL2L_RAM     0
#define BCFC_NL2L_BYPASS  0

#define BCFC_L2NL_709     0
#define BCFC_L2NL_1886    0
#define BCFC_L2NL_PQ      0
#define BCFC_L2NL_BBC     0
#define BCFC_L2NL_RAM     0
#define BCFC_L2NL_BYPASS  0
#endif

/* must be the same as in bvdc_common_priv.h */
#define BCFC_P_CLEAN                         0
#define BCFC_RUL_UPDATE_THRESHOLD           (2)
#define BCFC_MAX_MULTI_RUL_BUFFER_COUNT     (2)

/****************************************************************************
 * CFC (color format conversion)
 *
 */

/* CFC LUT loading command list information. Whenever a new LUT loadind cmd is
 * added pulCurrent is updated.  When all LUT loading cmds in this CMP / GFD /
 * DVI-CFC are put into this LUT loading cmd list, it is submitted to HW to
 * execute with regular RDC RUL */
typedef struct
{
    uint32_t           *pulStart[BCFC_MAX_MULTI_RUL_BUFFER_COUNT];
    BMMA_DeviceOffset   ullStartDeviceAddr[BCFC_MAX_MULTI_RUL_BUFFER_COUNT];
    BMMA_Block_Handle   hMmaBlock[BCFC_MAX_MULTI_RUL_BUFFER_COUNT];
    uint32_t           *pulCurrent;
    uint32_t            ulIndex;
} BCFC_LutLoadListInfo;

/* ram lut cfg tables
 */
typedef struct
{
    const BCFC_RamLut         *pRamLutNL2L;   /* must be NULL if not used */
    const BCFC_RamLut         *pRamLutLMR;    /* must be NULL if not used */
    const uint32_t            *pulLmrAdj;     /* LMR_A/B/C, must not be NULL if pRamLutLMR is not */
    const BCFC_LRangeAdjTable *pLRngAdjTable; /* cp to pCfc->stLRangeAdj.pTable, not directly used to build */
    const BCFC_RamLut         *pRamLutL2NL;   /* must be NULL if not used */
} BCFC_TfConvRamLuts;

/* CFC (color format conversion) ID
 */
typedef enum
{
    /* Video Window 0 CFC in compositor 0 (Primary) */
    BCFC_Id_eComp0_V0 = 0,
    BCFC_Id_eComp0_V1,

    /* Video Window 1 CFC in compositor 1 (Secondary) */
    BCFC_Id_eComp1_V0,
    BCFC_Id_eComp1_V1,

    /* Video Window 0 CFC in Compositor Bypass */
    BCFC_Id_eComp2_V0,

    /* Video Window 0 CFC in Vice */
    BCFC_Id_eComp3_V0,
    BCFC_Id_eComp4_V0,
    BCFC_Id_eComp5_V0,
    BCFC_Id_eComp6_V0,

    /* Gfx CFC in compositor 0 (Primary) */
    BCFC_Id_eComp0_G0,

    /* Gfx CFC in compositor 1 (Secondary) */
    BCFC_Id_eComp1_G0,

    /* Gfx CFC in compositor 2 (Tertiary) */
    BCFC_Id_eComp2_G0,

    /* Gfx CFC in compositor 3/4/5/6 (Vice) */
    BCFC_Id_eComp3_G0,
    BCFC_Id_eComp4_G0,
    BCFC_Id_eComp5_G0,
    BCFC_Id_eComp6_G0,

    /* DVI-CSC CFC */
    BCFC_Id_eDisplay0,

    /* CFC in VFC 0/1/2 */
    BCFC_Id_eVfc0,
    BCFC_Id_eVfc1,
    BCFC_Id_eVfc2,
    BCFC_Id_eVfcMax = BCFC_Id_eVfc2,

    /* Must be last */
    BCFC_Id_eUnknown
} BCFC_Id;

/*
 * CFC feature flag bits.  Some CFC module only have a sub-set of modules
 *
 */
typedef union
{
    struct
    {
        uint32_t                 bDbvCmp                  : 1;  /* 0 */
        uint32_t                 bTpToneMapping           : 1;
        uint32_t                 bMa                      : 1;
        uint32_t                 bNL2L                    : 1;
        uint32_t                 bMb                      : 1;  /* 4 */
        uint32_t                 bLMR                     : 1;
        uint32_t                 bLRngAdj                 : 1;
        uint32_t                 bL2NL                    : 1;
        uint32_t                 bMc                      : 1;  /* 8 */
        uint32_t                 bDbvToneMapping          : 1;
        uint32_t                 bRamNL2L                 : 1;
        uint32_t                 bRamL2NL                 : 1;
        uint32_t                 bRamLutScb               : 1;  /* 12 */
        uint32_t                 bCscBlendIn              : 1;
        uint32_t                 bCscBlendOut             : 1;
        uint32_t                 bAlphaDiv                : 1;
        uint32_t                 bBlackBoxNLConv          : 1;  /* 16 - in 7439 B0 and 7271 A0 CMP1 */
    } stBits;

    uint32_t ulInts;
} BCFC_Capability;

/*
 * CFC force Config. Test API. App uses it to force certain CFC sub-modules off
 *
 */
typedef union
{
    struct
    {
        uint32_t                 bDisableNl2l             : 1;  /* 0 */
        uint32_t                 bDisableLRangeAdj        : 1;
        uint32_t                 bDisableLmr              : 1;
        uint32_t                 bDisableL2nl             : 1;
        uint32_t                 bDisableRamLuts          : 1;  /* 4 */
        uint32_t                 bDisableMb               : 1;
        uint32_t                 bDisableDolby            : 1;
        uint32_t                 bDisableTch              : 1;
        uint32_t                 bDisableItm              : 1;  /* 8 */
    } stBits;

    uint32_t ulInts;
} BCFC_ForceCfg;

/*
 * CFC (color format conversion) context
 * Note: Each mosaic csc configure slot is represented by a CFC handle
 */
typedef struct BCFC_Context
{
    BDBG_OBJECT(BVDC_CFC)

    BCFC_Id                     eId;
    uint8_t                     ucMosaicSlotIdx;  /* used for CFC in CMP only */

    BCFC_Capability             stCapability;
    BCFC_ForceCfg               stForceCfg;

    BCFC_ColorSpaceExt          stColorSpaceExtIn;
    BCFC_ColorSpaceExt         *pColorSpaceExtOut;  /* e.g. ptr to struct in hCompositor or hDisplay */

    uint8_t                     ucRulBuildCntr;  /* for RUL build */

    uint8_t                     ucSelBlackBoxNL;

    bool                        bBlendInMatrixOn;
    bool                        bBypassCfc;

    const BCFC_Csc3x4          *pMa;
    BCFC_Csc3x3                 stMb;  /* MbOut * MbIn */
    BCFC_Csc3x4                 stMc;  /* McOut, or McOut * MbOut * MbIn * MaIn, or identity */
  #if (BCFC_VERSION >= BCFC_VER_2)  /* #if (BVDC_P_CMP_CFC_VER >= BVDC_P_CFC_VER_2) */
    BCFC_LRangeAdj              stLRangeAdj;
  #if (BCFC_VERSION >= BCFC_VER_3)  /* #if (BVDC_P_CMP_CFC_VER >= BVDC_P_CFC_VER_3) */
    const BCFC_TfConvRamLuts   *pTfConvRamLuts;
    uint8_t                     ucRamNL2LRulBuildCntr;  /* too expensive, so extra build optimization */
    uint8_t                     ucRamLMRRulBuildCntr;   /* too expensive, so extra build optimization */
    uint8_t                     ucRamL2NLRulBuildCntr;  /* too expensive, so extra build optimization */
    BCFC_LutLoadListInfo       *pLutList;               /* ram lut loading cmd list */
  #endif /* #if (BCFC_VERSION >= BCFC_VER_3) */
  #endif /* #if (BCFC_VERSION >= BCFC_VER_2) */
} BCFC_Context;

/* --------------------------------------------------------------------
 * utility funcs to return basic matrix MA, MB_IN, MB_OUT and MC
 *
 */
const BCFC_Csc3x4 *BCFC_GetCsc3x4_Ma_isrsafe(BCFC_Colorimetry eColorimetry);
const BCFC_Csc3x4 *BCFC_GetCsc3x4_Mc_isrsafe(BCFC_Colorimetry eColorimetry);
const BCFC_Csc3x3 *BCFC_GetCsc3x3_MbIn_isrsafe(BCFC_Colorimetry eColorimetry);
const BCFC_Csc3x3 *BCFC_GetCsc3x3_MbOut_isrsafe(BCFC_Colorimetry eColorimetry);
const BCFC_Csc3x4 *BCFC_GetCsc3x4_Cr0p85Adj_NonBT2020_to_BT2020_isrsafe(void);
const BCFC_Csc3x4 *BCFC_GetCsc3x4_YCbCr_Limited_to_Full_isrsafe(void);
const BCFC_Csc3x4 *BCFC_GetCsc3x4_Identity_isrsafe(void);
const BCFC_Csc3x4 *BCFC_GetCsc3x4_BT709_YCbCrtoRGB_isrsafe(void);

/* Generic CFC matrix multiply M = A * B. Refer to bcfc.c for detail
 */
void BCFC_Csc_Mult_isrsafe
    ( const int32_t              *pA,  /* matrix A element buf ptr */
      int                         iAc, /* matrxi A's number of columns */
      const int32_t              *pB,  /* matrix B element buf ptr */
      int                         iBc, /* matrxi B's number of columns */
      int32_t                    *pM ); /* matrix M element buf ptr */

/* get the 4x5 matrix to convert gfx YCbCr to RGB
 */
BERR_Code BCFC_GetMatrixForGfxYCbCr2Rgb_isrsafe
    ( BAVC_MatrixCoefficients          eMatrixCoeffs,
      uint32_t                         ulShift,
      int32_t                         *pulCoeffs);

/* convert BAVC color info to BCFC_Colorimetry
 */
BCFC_Colorimetry BCFC_AvcColorInfoToColorimetry_isrsafe
    ( BAVC_ColorPrimaries              eColorPrimaries,
      BAVC_MatrixCoefficients          eMatrixCoeffs,
      bool                             bXvYcc);

/* convert BAVC transferChracteristics to color info to BCFC_ColorTF
 */
BCFC_ColorTF BCFC_AvcTransferCharacteristicsToTF_isrsafe
    ( BAVC_TransferCharacteristics eTransChar);

/* generic avc color space init
 */
void BCFC_InitCfcColorSpace(
    BCFC_ColorSpace       *pColorSpace );

/* generic cfc color space copy
 */
void BCFC_CopyColorSpace_isrsafe(
    BCFC_ColorSpace          *pDstColorSpace,
    BCFC_ColorSpace          *pSrcColorSpace );

/* generic cfc init
 */
void BCFC_InitCfc_isrsafe(
    BCFC_Context          *pCfc );

/* return BCFC_TfConvRamLuts for bypass
 */
const BCFC_TfConvRamLuts * BCFC_GetTfConvRamLutsBypass_isrsafe(void);

/* generic configure of a CFC according to its input and output color space
 */
bool BCFC_UpdateCfg_isr
    ( BCFC_Context        *pCfc,
      bool                 bMosaicMode,
      bool                 bTchInput,
      bool                 bForceDirty);

/* utility functions to print matrix values
 */
void BCFC_PrintCscRx4_isrsafe(const uint32_t *pCur, uint32_t ulCfg, bool bUseAlt);
void BCFC_PrintFloatCscRx4_isrsafe(const BCFC_Csc3x4 *pCsc, const BCFC_Csc3x4 *pAlt);
void BCFC_PrintCsc3x3_isrsafe(const uint32_t *pCur, uint32_t ulCfg);
void BCFC_PrintFloatCsc3x3_isrsafe(const BCFC_Csc3x3 *pCsc);

#if (BDBG_DEBUG_BUILD)
/* utility functions for printing names
 */
const char *BCFC_GetCfcName_isrsafe(BCFC_Id eId);
const char *BCFC_GetColorFormatName_isrsafe(BCFC_ColorFormat eFmt);
const char *BCFC_GetColorimetryName_isrsafe(BCFC_Colorimetry eClrmtr);
const char *BCFC_GetColorRangeName_isrsafe(BCFC_ColorRange eClrRng);
const char *BCFC_GetColorTfName_isrsafe(BCFC_ColorTF eClrTf);
const char *BCFC_GetColorDepthName_isrsafe(BCFC_ColorDepth eClrDpt);
#endif

#ifdef __cplusplus
}
#endif

#endif /* #ifndef BCFC_H__ */
/* End of file. */
