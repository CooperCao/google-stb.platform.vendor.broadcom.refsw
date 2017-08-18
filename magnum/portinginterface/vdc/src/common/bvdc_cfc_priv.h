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
#ifndef BVDC_CFC_PRIV_H__
#define BVDC_CFC_PRIV_H__

#include "bmth_fix.h"
#include "bmth_fix_matrix.h"
#include "bvdc.h"
#include "bvdc_common_priv.h"
#include "bvdc_cfc_types_priv.h"
#if BVDC_P_DBV_SUPPORT
#include "bvdc_cfc_dbv_priv.h"
#endif
#if BVDC_P_TCH_SUPPORT
#include "bvdc_cfc_tch_priv.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif


#define BVDC_P_IS_SDR(tf) \
    ((tf)==BAVC_P_ColorTF_eBt1886 || (tf)==BAVC_P_ColorTF_eHlg)

#define BVDC_P_IS_TCH(dataType) \
    ((dataType)==BAVC_HdrMetadataType_eTch_Cvri || (dataType)==BAVC_HdrMetadataType_eTch_Cri)

#define BVDC_P_NEED_TF_CONV(i, o) \
    !(((i) == (o)) || \
      (((i)==BAVC_P_ColorTF_eHlg) && (o)== BAVC_P_ColorTF_eBt1886))

#define BVDC_P_IS_BT2020(c) \
    (BAVC_P_Colorimetry_eBt2020 == (c))

#define BVDC_P_IS_HLG_TO_HDR10(i, o) \
    ((i == BAVC_P_ColorTF_eHlg) && (o == BAVC_P_ColorTF_eBt2100Pq))

#define BVDC_P_IS_SDR_TO_HDR10(i, o) \
    ((i == BAVC_P_ColorTF_eBt1886) && (o == BAVC_P_ColorTF_eBt2100Pq))

#define BVDC_P_IS_HLG_TO_SDR(i, o) \
    ((i == BAVC_P_ColorTF_eHlg) && (o == BAVC_P_ColorTF_eBt1886))

#define BVDC_P_IS_HDR10_TO_SDR(i, o) \
    ((i == BAVC_P_ColorTF_eBt2100Pq) && (o == BAVC_P_ColorTF_eBt1886))

#define BVDC_P_IS_SDR_TO_HLG(i, o) \
    ((i == BAVC_P_ColorTF_eBt1886) && (o == BAVC_P_ColorTF_eHlg))

#define BVDC_P_IS_HDR10_TO_HLG(i, o) \
    ((i == BAVC_P_ColorTF_eBt2100Pq) && (o == BAVC_P_ColorTF_eHlg))

#define BVDC_P_NEED_BLEND_MATRIX(hCompositor) \
    ((hCompositor)->abBlenderUsed[0] && \
     (hCompositor)->stCfcCapability[0].stBits.bCscBlendOut && \
     !BVDC_P_IS_SDR((hCompositor)->stOutColorSpace.stAvcColorSpace.eColorTF))

#define BVDC_P_CFC_IN_CMP0V0(id)   ((id) == BVDC_P_CfcId_eComp0_V0)

#define BVDC_P_CFC_IN_CMP0(id)     ((id) <= BVDC_P_CfcId_eComp0_V1)

#define BVDC_P_CFC_IN_CMP(id)      ((id) <= BVDC_P_CfcId_eComp6_V0)

#define BVDC_P_CFC_IN_GFD0(id)     ((id) == BVDC_P_CfcId_eComp0_G0)

#define BVDC_P_CFC_IN_GFD(id)      (((id) >= BVDC_P_CfcId_eComp0_G0) && ((id) <= BVDC_P_CfcId_eComp6_G0))

#define BVDC_P_CFC_IN_VFC(id)      (((id) >= BVDC_P_CfcId_eVfc0) && ((id) <= BVDC_P_CfcId_eVfcMax))

#define BVDC_P_CFC_IN_DVI(id)      ((id) == BVDC_P_CfcId_eDisplay0)

#define BVDC_P_CSC_SW_MAX_BITS             (31)
#define BVDC_P_CSC_SW_MASK         (0x7FFFFFFF)
#define BVDC_P_CSC_SW_SIGN_MASK    (0x80000000)
#define BVDC_P_CSC_SW_CX_I_BITS             (2)
#define BVDC_P_CSC_SW_CX_F_BITS            (29)
#define BVDC_P_CSC_SW_CO_I_BITS             (9)
#define BVDC_P_CSC_SW_CO_F_BITS            (22)
#define BVDC_P_CFC_FIX_MAX_BITS            (63)
#define BVDC_P_CFC_FIX_FRACTION_BITS  (BVDC_P_CSC_SW_CX_F_BITS)
#define BVDC_P_CFC_FIX_INT_BITS       (BVDC_P_CFC_FIX_MAX_BITS - BVDC_P_CFC_FIX_FRACTION_BITS)
#define BVDC_P_CFC_SW_F_CX_CO_DIFF_BITS      (BVDC_P_CSC_SW_CX_F_BITS - BVDC_P_CSC_SW_CO_F_BITS)

/* integer to fixed */
#define BVDC_P_CFC_ITOFIX(x) \
    BMTH_FIX_SIGNED_ITOFIX(x, BVDC_P_CSC_SW_CX_I_BITS, BVDC_P_CSC_SW_CX_F_BITS)

#define BVDC_P_CFC_NL_SEL_BYPASS         (0xFF)

/****************************************************************************
 * ColorSpace
 *
 */

/* Software copy of 3x4 Color Conversion Matrix, such as Ma for YCbCr2Rgb,
 * Ipt2Lsm, .... ; or Mc (Rgb2YCbCr, Lms2Ipt, ...), or blend_in, blend_out,
 * or dvi_csc ...
 */
typedef struct
{
    int32_t     m[3][4];
} BVDC_P_Csc3x4;

/* Software copy of 2x4 Color Conversion Matrix, for CL Cr/Cb positive
 * alternative for input Ma in CMP and display Mc in DVI_CSC
 */
typedef struct
{
    int32_t     m[2][4];
} BVDC_P_Csc2x4;

/* Software copy of 3x3 Matrix, such as Mb for input (Rgb2Xyz,  Lms2Xyz, ..)
 * and output (Xyz2Rgb, Xyz2Lms, ...).
 * The final Matrix Mb will be the product of input Mb and output Mb.
 */
typedef struct
{
    int32_t     m[3][3];
} BVDC_P_Csc3x3;

/* HW matrix types with diff int / fraction bits, row/colum numbers, or other formats
 */

#define BVDC_P_CSC_GFD_WITH_BLEND(t) \
    ((t) >= (6<<16))
#define BVDC_P_CSC_GFD_CONST_BLEND(t) \
    (BVDC_P_CSC_GFD_WITH_BLEND(t) && (((t) & (1<<16)) == 0))
#define BVDC_P_CSC_GFD_ALPHA_BLEND(t) \
    (BVDC_P_CSC_GFD_WITH_BLEND(t) && (((t) & (1<<16)) != 0))

#define BVDC_P_CSC_ROWS(t) \
    (((t) >> 12) & 0xF)
#define BVDC_P_CSC_COLUMS(t) \
    (((t) >> 8) & 0xF)

#define BVDC_P_CSC_CX_F_BITS(t) \
    ((t) & 0xFF)
#define BVDC_P_CSC_CO_F_BITS(t) \
    (BVDC_P_CSC_CX_F_BITS(t) - 7)
#define BVDC_P_CSC_MASK(t) \
    ((1 << (1 + 2 + BVDC_P_CSC_CX_F_BITS(t))) - 1)

#define BVDC_P_CSC_NAME_IDX(t) \
    ((t) >> 16)

typedef enum
{
    /* name convention: the last number in name is the total number of bits of one element */
    BVDC_P_CscType_eMa3x4_16 =    (13) | (0x34<<8) | (0<<16),  /* s2.13 / s9.6:  CMP MA in 7439 B0 */
    BVDC_P_CscType_eMa3x4_25 =    (22) | (0x34<<8) | (0<<16),  /* s2.22 / s9.15: CMP MA in 7271 A0, DVI MA in 7271 B0/up */
    BVDC_P_CscType_eMa5x4_25 =    (22) | (0x54<<8) | (0<<16),  /* s2.22 / s9.15: CMP/VFC MA in 7271 B0 and up */

    BVDC_P_CscType_eMb3x4_25 =    (22) | (0x34<<8) | (1<<16),  /* s2.22 / s9.15: CMP/GFD/VFC MB in 7271 A0/up, DVI MB in 7271 B0/up */

    BVDC_P_CscType_eMc3x4_16 =    (13) | (0x34<<8) | (2<<16),  /* s2.13 / s9.6:  CMP/VFC MC */
    BVDC_P_CscType_eMc5x4_16 =    (13) | (0x54<<8) | (2<<16),  /* s2.13 / s9.6:  DVI MC in 7271 B0 and up */
    BVDC_P_CscType_eMcPacked =    (13) | (0x32<<8) | (2<<16),  /* s2.13 / s9.6:  DVI MC with 2 elemnts in one reg, 7271 A0 / older */

    BVDC_P_CscType_eMbi3x4_16 =   (13) | (0x34<<8) | (3<<16),  /* s2.13 / s9.6:  CMP blend-in,  7271 A0 and up */
    BVDC_P_CscType_eMbo3x4_16 =   (13) | (0x34<<8) | (4<<16),  /* s2.13 / s9.6:  CMP blend-out, 7271 A0 and up */

    BVDC_P_CscType_eMb1x4_25 =    (22) | (0x14<<8) | (5<<16),  /* s2.22 / s9.15: CMP / GFD MB2 for 7271 B0 and up */

    BVDC_P_CscType_eMa3x5_16CB =  (13) | (0x35<<8) | (6<<16),  /* s2.22 / s9.15: GFD MA with constant blending, 7439 B0 */
    BVDC_P_CscType_eMa3x5_16AB =  (13) | (0x35<<8) | (7<<16),  /* s2.22 / s9.15: GFD MA with alpha blending, 7439 B0 */
    BVDC_P_CscType_eMa3x5_25CB =  (22) | (0x35<<8) | (6<<16),  /* s2.22 / s9.15: GFD MA with constant blending, 7271 A0/up */
    BVDC_P_CscType_eMa3x5_25AB =  (22) | (0x35<<8) | (7<<16),  /* s2.22 / s9.15: GFD MA with alpha blending, 7271 A0/up */

    BVDC_P_CscType_eMc3x5_16CB =  (13) | (0x35<<8) | (8<<16),  /* s2.13 / s9.6:  GFD MC with constant blending, 7271 A0/up */
    BVDC_P_CscType_eMc3x5_16AB =  (13) | (0x35<<8) | (9<<16),  /* s2.13 / s9.6:  GFD MC with alpha blending,    7271 A0/up */

    BVDC_P_CscType_eMbi3x5_16CB = (13) | (0x35<<8) | (10<<16), /* s2.13 / s9.6:  GFD Blend-in with constant blending, 7271 B0/up */
    BVDC_P_CscType_eMbi3x5_16AB = (13) | (0x35<<8) | (11<<16), /* s2.13 / s9.6:  GFD Blend-in with alpha blending,    7271 B0/up */
    BVDC_P_CscType_eInvalid = 0
} BVDC_P_CscType;

/* HW matrix left shift handling
 */
#define BVDC_P_LSHIFT_BITS(s) \
    ((s) >> 4)
#define BVDC_P_WRITE_LSHIFT_REG(s) \
    ((s) & 0x1)

typedef enum
{
    BVDC_P_LeftShift_eNotExist =     0, /* matrix in old chips does not have left shift */
    BVDC_P_LeftShift_eOff = (0<<4) | 1, /* not shift, but program LSHIFT reg */
    BVDC_P_LeftShift_e1_1 = (1<<4) | 1, /* lshift 1 bit, program LSHIFT reg */
    BVDC_P_LeftShift_e2_1 = (2<<4) | 1, /* lshift 2 bit, program LSHIFT reg */
    BVDC_P_LeftShift_e1_0 = (1<<4) | 0, /* lshift 1 bit, NOT program LSHIFT reg */
    BVDC_P_LeftShift_e2_0 = (2<<4) | 0, /* lshift 2 bit, NOT program LSHIFT reg */
    BVDC_P_LeftShift_eExist = BVDC_P_LeftShift_eOff, /* for dvs, program LSHIFT reg with BDVS_Control_Csc.uiLshf */
    BVDC_P_LeftShift_eMax
} BVDC_P_LeftShift;

/* we combine BVDC_P_CscType and BVDC_P_LeftShift into one uint32_t to pass one less parameter
 * in BVDC_P_Cfc_BuildCscRul
 */
#define BVDC_P_MAKE_CSC_CFG(t, s) \
    ((t) | ((s) << 24))
#define BVDC_P_GET_CSC_TYPE(c) \
    ((c) & 0x00FFFFFF)
#define BVDC_P_GET_CSC_LSHIFT(c) \
    ((c) >> 24)

/* --------------------------------------------------------------------
 * L-Range Adjust MACRO:
 */
#define BVDC_P_LR_XY_I_BITS          (1)
#define BVDC_P_LR_XY_F_BITS         (15)
#define BVDC_P_LR_SLP_M_I_BITS       (0)
#define BVDC_P_LR_SLP_M_F_BITS      (15)
#define BVDC_P_LR_SLP_E_I_BITS       (4)
#define BVDC_P_LR_SLP_E_F_BITS       (0)

#if (BVDC_P_CMP_CFC_VER >= BVDC_P_CFC_VER_3)
#define BVDC_P_LR_X_SHIFT           BCHP_HDR_CMP_0_V0_R0_LR_XY_0i_LRA_X_SHIFT
#define BVDC_P_LR_Y_SHIFT           BCHP_HDR_CMP_0_V0_R0_LR_XY_0i_LRA_Y_SHIFT
#define BVDC_P_LR_SLP_M_SHIFT       BCHP_HDR_CMP_0_V0_R0_LR_SLOPEi_SLOPE_M_SHIFT
#define BVDC_P_LR_SLP_E_SHIFT       BCHP_HDR_CMP_0_V0_R0_LR_SLOPEi_SLOPE_E_SHIFT

#elif (BVDC_P_CMP_CFC_VER == BVDC_P_CFC_VER_2)
#define BVDC_P_LR_X_SHIFT           BCHP_HDR_CMP_0_V0_R0_NL_LR_XY_0i_LRA_X_SHIFT
#define BVDC_P_LR_Y_SHIFT           BCHP_HDR_CMP_0_V0_R0_NL_LR_XY_0i_LRA_Y_SHIFT
#define BVDC_P_LR_SLP_M_SHIFT       BCHP_HDR_CMP_0_V0_R0_NL_LR_SLOPEi_SLOPE_M_SHIFT
#define BVDC_P_LR_SLP_E_SHIFT       BCHP_HDR_CMP_0_V0_R0_NL_LR_SLOPEi_SLOPE_E_SHIFT

#else /* TODO: add support */
#define BVDC_P_LR_X_SHIFT           0
#define BVDC_P_LR_Y_SHIFT           0
#define BVDC_P_LR_SLP_M_SHIFT       0
#define BVDC_P_LR_SLP_E_SHIFT       0
#endif

#define BVDC_P_MAKE_LR_XY(x, y) \
    (((uint16_t)BMTH_FIX_SIGNED_FTOFIX(x, BVDC_P_LR_XY_I_BITS, BVDC_P_LR_XY_F_BITS)) << BVDC_P_LR_X_SHIFT) | \
    (((uint16_t)BMTH_FIX_SIGNED_FTOFIX(y, BVDC_P_LR_XY_I_BITS, BVDC_P_LR_XY_F_BITS)) << BVDC_P_LR_Y_SHIFT)

#define BVDC_P_MAKE_LR_SLOPE(m,e) \
    (((uint16_t)BMTH_FIX_SIGNED_FTOFIX(m, BVDC_P_LR_SLP_M_I_BITS, BVDC_P_LR_SLP_M_F_BITS)) << BVDC_P_LR_SLP_M_SHIFT) | \
    (((uint16_t)BMTH_FIX_SIGNED_FTOFIX(e, BVDC_P_LR_SLP_E_I_BITS, BVDC_P_LR_SLP_E_F_BITS)) << BVDC_P_LR_SLP_E_SHIFT)

#define BVDC_P_MAKE_LR_ADJ(num_pts,                               \
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
        BVDC_P_MAKE_LR_SLOPE(mantissa0, exp0),                    \
        BVDC_P_MAKE_LR_SLOPE(mantissa1, exp1),                    \
        BVDC_P_MAKE_LR_SLOPE(mantissa2, exp2),                    \
        BVDC_P_MAKE_LR_SLOPE(mantissa3, exp3),                    \
        BVDC_P_MAKE_LR_SLOPE(mantissa4, exp4),                    \
        BVDC_P_MAKE_LR_SLOPE(mantissa5, exp5),                    \
        BVDC_P_MAKE_LR_SLOPE(mantissa6, exp6),                    \
        BVDC_P_MAKE_LR_SLOPE(mantissa7, exp7)                     \
    },                                                            \
    {                                                             \
        BVDC_P_MAKE_LR_XY(x0, y0),                                \
        BVDC_P_MAKE_LR_XY(x1, y1),                                \
        BVDC_P_MAKE_LR_XY(x2, y2),                                \
        BVDC_P_MAKE_LR_XY(x3, y3),                                \
        BVDC_P_MAKE_LR_XY(x4, y4),                                \
        BVDC_P_MAKE_LR_XY(x5, y5),                                \
        BVDC_P_MAKE_LR_XY(x6, y6),                                \
        BVDC_P_MAKE_LR_XY(x7, y7)                                 \
    }                                                             \
}

/*
 * AVC ColorSpace (public)
 *
 */
typedef struct BAVC_P_ColorSpace
{
    /* likely the follwoing five will be BAVC_ColorSpace */
    BAVC_P_ColorFormat          eColorFmt;
    BAVC_P_Colorimetry          eColorimetry;
    BAVC_P_ColorRange           eColorRange;
    BAVC_P_ColorTF              eColorTF;
    BAVC_P_ColorDepth           eColorDepth;

#if BVDC_P_DBV_SUPPORT || BVDC_P_TCH_SUPPORT
    union {
        /* DBV support */
        #if BVDC_P_DBV_SUPPORT
        BVDC_P_DBV_Input stDbvInput;
        #endif
        #if BVDC_P_TCH_SUPPORT
        BVDC_P_TCH_Input stTchInput;
        #endif
    } stMetaData;
#endif /* BVDC_P_DBV_SUPPORT || BVDC_P_TCH_SUPPORT */
} BAVC_P_ColorSpace;

#define BAVC_P_COLOR_SPACE_DIFF(c1, c2) \
    (((c1)->eColorFmt    != (c2)->eColorFmt) || \
     ((c1)->eColorimetry != (c2)->eColorimetry) || \
     ((c1)->eColorTF     != (c2)->eColorTF) || \
     /*((c1)->eColorDepth  != (c2)->eColorDepth) ||*/ \
     ((c1)->eColorRange  != (c2)->eColorRange))

/*
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
} BVDC_P_ColorSpace_Config;

/*
 * Color Space
 *
 */
typedef struct BVDC_P_ColorSpace
{
    BAVC_P_ColorSpace           stAvcColorSpace;

    BVDC_P_ColorSpace_Config    stCfg;

    /* non dynamically changing matrix might be a ptr in future ??? */
    BVDC_P_Csc3x4               stM3x4; /* ColorRange adjusted Ma for input, Mc for display */
    BVDC_P_Csc3x3               stMb;   /* 2XYZ for input, fromXYZ for output */
    BVDC_P_Csc3x4               stMalt; /* CL pos alt, Ma with input, Mc with display */

} BVDC_P_ColorSpace;

/* --------------------------------------------------------------------
 * PWL table for dbv/tch background color nl2l&l2nl adjustment:
 */
#define BVDC_P_PWL_XY_I_BITS          (1)
#define BVDC_P_PWL_XY_F_BITS         (24)
#define BVDC_P_PWL_SLP_M_I_BITS       (0)
#define BVDC_P_PWL_SLP_M_F_BITS      (24)
#define BVDC_P_PWL_SLP_E_I_BITS       (4)
#define BVDC_P_PWL_SLP_E_F_BITS       (0)

#define BVDC_P_MAKE_PWL_XY(x, y) \
    {BMTH_FIX_SIGNED_FTOFIX(x, BVDC_P_PWL_XY_I_BITS, BVDC_P_PWL_XY_F_BITS), \
     BMTH_FIX_SIGNED_FTOFIX(y, BVDC_P_PWL_XY_I_BITS, BVDC_P_PWL_XY_F_BITS)}

#define BVDC_P_MAKE_PWL_SLOPE(m,e) \
    {BMTH_FIX_SIGNED_FTOFIX(m, BVDC_P_PWL_SLP_M_I_BITS, BVDC_P_PWL_SLP_M_F_BITS), \
     BMTH_FIX_SIGNED_FTOFIX(e, BVDC_P_PWL_SLP_E_I_BITS, BVDC_P_PWL_SLP_E_F_BITS)}

#define BVDC_P_MAKE_NL_PWL(num_pts,                              \
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
        BVDC_P_MAKE_PWL_XY(x0, y0),                               \
        BVDC_P_MAKE_PWL_XY(x1, y1),                               \
        BVDC_P_MAKE_PWL_XY(x2, y2),                               \
        BVDC_P_MAKE_PWL_XY(x3, y3),                               \
        BVDC_P_MAKE_PWL_XY(x4, y4),                               \
        BVDC_P_MAKE_PWL_XY(x5, y5),                               \
        BVDC_P_MAKE_PWL_XY(x6, y6),                               \
        BVDC_P_MAKE_PWL_XY(x7, y7)                                \
    },                                                            \
    {                                                             \
        BVDC_P_MAKE_PWL_SLOPE(mantissa0, exp0),                   \
        BVDC_P_MAKE_PWL_SLOPE(mantissa1, exp1),                   \
        BVDC_P_MAKE_PWL_SLOPE(mantissa2, exp2),                   \
        BVDC_P_MAKE_PWL_SLOPE(mantissa3, exp3),                   \
        BVDC_P_MAKE_PWL_SLOPE(mantissa4, exp4),                   \
        BVDC_P_MAKE_PWL_SLOPE(mantissa5, exp5),                   \
        BVDC_P_MAKE_PWL_SLOPE(mantissa6, exp6),                   \
        BVDC_P_MAKE_PWL_SLOPE(mantissa7, exp7)                    \
    }                                                             \
}

typedef struct BVDC_P_NL_PwlSegments
{
    unsigned num;/* number of points/segments */
    struct {
        uint32_t x;/* U1.24 */
        uint32_t y;/* U1.24 */
    } point[BVDC_P_CMP_LR_ADJ_PTS];
    struct
    {
        int32_t man;/* S0.24 */
        int32_t exp;/* S4.0 */
    } slope[BVDC_P_CMP_LR_ADJ_PTS];
} BVDC_P_NL_PwlSegments;

/* --------------------------------------------------------------------
 * matrix
 */
#define BVDC_P_MAKE_CFC_CX(m) \
    BMTH_FIX_SIGNED_FTOFIX(m, BVDC_P_CSC_SW_CX_I_BITS, BVDC_P_CSC_SW_CX_F_BITS)
#define BVDC_P_MAKE_CFC_CO(m) \
    BMTH_FIX_SIGNED_FTOFIX(m, BVDC_P_CSC_SW_CO_I_BITS, BVDC_P_CSC_SW_CO_F_BITS)

#define BVDC_P_MAKE_CFC_CX_FR_USR(m, usr_shift) \
    BMTH_FIX_SIGNED_CONVERT_isrsafe(m, BVDC_P_CFC_FIX_MAX_BITS - usr_shift, usr_shift, BVDC_P_CSC_SW_CX_I_BITS, BVDC_P_CSC_SW_CX_F_BITS)
#define BVDC_P_MAKE_CFC_CO_FR_USR(m, usr_shift) \
    BMTH_FIX_SIGNED_CONVERT_isrsafe(m, BVDC_P_CFC_FIX_MAX_BITS - usr_shift, usr_shift, BVDC_P_CSC_SW_CO_I_BITS, BVDC_P_CSC_SW_CO_F_BITS)

#define BVDC_P_MAKE_CFC_CX_TO_USR(m, usr_shift) \
    BMTH_FIX_SIGNED_CONVERT_isrsafe(m, BVDC_P_CSC_SW_CX_I_BITS, BVDC_P_CSC_SW_CX_F_BITS, BVDC_P_CFC_FIX_MAX_BITS - usr_shift, usr_shift)
#define BVDC_P_MAKE_CFC_CO_TO_USR(m, usr_shift) \
    BMTH_FIX_SIGNED_CONVERT_isrsafe(m, BVDC_P_CSC_SW_CO_I_BITS, BVDC_P_CSC_SW_CO_F_BITS, BVDC_P_CFC_FIX_MAX_BITS - usr_shift, usr_shift)

/* 3x4 matrix, for A or C */
#define BVDC_P_MAKE_CSC_3x4(m00, m01, m02, m03,  \
                            m10, m11, m12, m13,  \
                            m20, m21, m22, m23 ) \
{{                                               \
     { BVDC_P_MAKE_CFC_CX(m00),   \
       BVDC_P_MAKE_CFC_CX(m01),   \
       BVDC_P_MAKE_CFC_CX(m02),   \
       BVDC_P_MAKE_CFC_CO(m03) }, \
                                  \
     { BVDC_P_MAKE_CFC_CX(m10),   \
       BVDC_P_MAKE_CFC_CX(m11),   \
       BVDC_P_MAKE_CFC_CX(m12),   \
       BVDC_P_MAKE_CFC_CO(m13) }, \
                                  \
     { BVDC_P_MAKE_CFC_CX(m20),   \
       BVDC_P_MAKE_CFC_CX(m21),   \
       BVDC_P_MAKE_CFC_CX(m22),   \
       BVDC_P_MAKE_CFC_CO(m23) }  \
}}

/* 2x4 matrix, for A or C positive Cr/Cb ALT in CL case */
#define BVDC_P_MAKE_CSC_2x4(m00, m01, m02, m03,   \
                            m10, m11, m12, m13 )  \
{{                                                \
     { BVDC_P_MAKE_CFC_CX(m00),   \
       BVDC_P_MAKE_CFC_CX(m01),   \
       BVDC_P_MAKE_CFC_CX(m02),   \
       BVDC_P_MAKE_CFC_CO(m03) }, \
                                  \
     { BVDC_P_MAKE_CFC_CX(m10),   \
       BVDC_P_MAKE_CFC_CX(m11),   \
       BVDC_P_MAKE_CFC_CX(m12),   \
       BVDC_P_MAKE_CFC_CO(m13) }  \
}}

/* 3x4 matrix, for B */
#define BVDC_P_MAKE_CSC_3x3(m00, m01, m02,  \
                            m10, m11, m12,  \
                            m20, m21, m22 ) \
{{                                \
     { BVDC_P_MAKE_CFC_CX(m00),   \
       BVDC_P_MAKE_CFC_CX(m01),   \
       BVDC_P_MAKE_CFC_CX(m02) }, \
                                  \
     { BVDC_P_MAKE_CFC_CX(m10),   \
       BVDC_P_MAKE_CFC_CX(m11),   \
       BVDC_P_MAKE_CFC_CX(m12) }, \
                                  \
     { BVDC_P_MAKE_CFC_CX(m20),   \
       BVDC_P_MAKE_CFC_CX(m21),   \
       BVDC_P_MAKE_CFC_CX(m22) }  \
}}

/*
 * LRange Adjust
 *
 */
#define BVDC_P_LR_ADJ_LIMIT_NOT_EXIST   0   /* for 7270 A0 and older */
#define BVDC_P_LR_ADJ_LIMIT_ALWAYS_ON   1   /* for DVI-CFC 7271 B0 and up */
#define BVDC_P_LR_ADJ_LIMIT_DISABLE     2   /* for CMP/GFD 7270 B0 and up */
#define BVDC_P_LR_ADJ_LIMIT_ENABLE      3   /* for CMP/GFD 7270 B0 and up */
#define BVDC_P_LR_ADJ_LIMIT_I_BITS      1
#define BVDC_P_LR_ADJ_LIMIT_F_BITS     26
typedef struct
{
    uint32_t   ulLRangeAdjNumPoints;
    uint32_t   aulLRangeAdjSlope[BVDC_P_CMP_LR_ADJ_PTS];
    uint32_t   aulLRangeAdjXY[BVDC_P_CMP_LR_ADJ_PTS];

} BVDC_P_LRangeAdjTable;

typedef struct BVDC_P_CfcLRangeAdj
{
    uint32_t                    ulLRangeAdjCtrl;
    uint32_t                    ulMin;
    uint32_t                    ulMax;

    const BVDC_P_LRangeAdjTable *pTable;

} BVDC_P_CfcLRangeAdj;

/*
 * Ram LUT
 *
 */
#define BVDC_P_MAX_LUT_SEGS   8
typedef struct
{
    uint8_t    ucNumSeg;
    uint8_t    ucXScale;  /* U1.2 */
    uint8_t    ucOutIntBits;
    uint8_t    ucOutFracBits;

    uint16_t   usSegBinEnd[BVDC_P_MAX_LUT_SEGS];
    uint16_t   usSegOffset[BVDC_P_MAX_LUT_SEGS];
    uint16_t   usSegIntBits[BVDC_P_MAX_LUT_SEGS];

    const uint32_t  *pulTable;

} BVDC_P_RamLut;

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
    uint32_t           *pulStart;
    uint32_t           *pulCurrent;
    BMMA_DeviceOffset   ullStartDeviceAddr;

    BMMA_Block_Handle   hMmaBlock;
    uint32_t            ulRulBuildCntr;
} BVDC_P_CfcLutLoadListInfo;

#if (BVDC_P_CMP_CFC_VER >= BVDC_P_CFC_VER_3)
/* ram lut cfg tables
 */
typedef struct
{
    const BVDC_P_RamLut         *pRamLutNL2L;   /* must be NULL if not used */
    const BVDC_P_RamLut         *pRamLutLMR;    /* must be NULL if not used */
    const uint32_t              *pulLmrAdj;     /* LMR_A/B/C, must not be NULL if pRamLutLMR is not */
    const BVDC_P_LRangeAdjTable *pLRngAdjTable; /* cp to pCfc->stLRangeAdj.pTable, not directly used to build */
    const BVDC_P_RamLut         *pRamLutL2NL;   /* must be NULL if not used */
} BVDC_P_TfConvRamLuts;
#endif

/* CFC (color format conversion) ID
 */
typedef enum
{
    /* Video Window 0 CFC in compositor 0 (Primary) */
    BVDC_P_CfcId_eComp0_V0 = 0,
    BVDC_P_CfcId_eComp0_V1,

    /* Video Window 1 CFC in compositor 1 (Secondary) */
    BVDC_P_CfcId_eComp1_V0,
    BVDC_P_CfcId_eComp1_V1,

    /* Video Window 0 CFC in Compositor Bypass */
    BVDC_P_CfcId_eComp2_V0,

    /* Video Window 0 CFC in Vice */
    BVDC_P_CfcId_eComp3_V0,
    BVDC_P_CfcId_eComp4_V0,
    BVDC_P_CfcId_eComp5_V0,
    BVDC_P_CfcId_eComp6_V0,

    /* Gfx CFC in compositor 0 (Primary) */
    BVDC_P_CfcId_eComp0_G0,

    /* Gfx CFC in compositor 1 (Secondary) */
    BVDC_P_CfcId_eComp1_G0,

    /* Gfx CFC in compositor 2 (Tertiary) */
    BVDC_P_CfcId_eComp2_G0,

    /* Gfx CFC in compositor 3/4/5/6 (Vice) */
    BVDC_P_CfcId_eComp3_G0,
    BVDC_P_CfcId_eComp4_G0,
    BVDC_P_CfcId_eComp5_G0,
    BVDC_P_CfcId_eComp6_G0,

    /* DVI-CSC CFC */
    BVDC_P_CfcId_eDisplay0,

    /* CFC in VFC 0/1/2 */
    BVDC_P_CfcId_eVfc0,
    BVDC_P_CfcId_eVfc1,
    BVDC_P_CfcId_eVfc2,
    BVDC_P_CfcId_eVfcMax = BVDC_P_CfcId_eVfc2,

    /* Must be last */
    BVDC_P_CfcId_eUnknown
} BVDC_P_CfcId;

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
} BVDC_P_Cfc_Capability;

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
} BVDC_P_Cfc_ForceCfg;

/*
 * CFC (color format conversion) context
 * Note: Each mosaic csc configure slot is represented by a CFC handle
 */
typedef struct BVDC_P_CfcContext
{
    BDBG_OBJECT(BVDC_CFC)

    BVDC_P_CfcId                eId;
    uint8_t                     ucMosaicSlotIdx;  /* used for CFC in CMP only */

    BVDC_P_Cfc_Capability       stCapability;
    BVDC_P_Cfc_ForceCfg         stForceCfg;

    BVDC_P_ColorSpace           stColorSpaceIn;
    BVDC_P_ColorSpace          *pColorSpaceOut;  /* e.g. ptr to struct in hCompositor or hDisplay */

    uint8_t                     ucRulBuildCntr;  /* for RUL build */

    uint8_t                     ucSelBlackBoxNL;

    bool                        bBlendInMatrixOn;
    bool                        bBypassCfc;

    const BVDC_P_Csc3x4        *pMa;
    BVDC_P_Csc3x3               stMb;  /* MbOut * MbIn */
    BVDC_P_Csc3x4               stMc;  /* McOut, or McOut * MbOut * MbIn * MaIn, or identity */
#if (BVDC_P_CMP_CFC_VER >= BVDC_P_CFC_VER_2)
    BVDC_P_CfcLRangeAdj         stLRangeAdj;
#if (BVDC_P_CMP_CFC_VER >= BVDC_P_CFC_VER_3)
    const BVDC_P_TfConvRamLuts *pTfConvRamLuts;
    uint8_t                     ucRamNL2LRulBuildCntr;  /* too expensive, so extra build optimization */
    uint8_t                     ucRamLMRRulBuildCntr;   /* too expensive, so extra build optimization */
    uint8_t                     ucRamL2NLRulBuildCntr;  /* too expensive, so extra build optimization */
    BVDC_P_CfcLutLoadListInfo  *pLutList;               /* ram lut loading cmd list */
#endif /* #if (BVDC_P_CMP_CFC_VER >= BVDC_P_CFC_VER_3) */
#endif /* #if (BVDC_P_CMP_CFC_VER >= BVDC_P_CFC_VER_2) */
} BVDC_P_CfcContext;

/*
 * Color clamp in DVI CFC
 */
typedef struct
{
    uint16_t                    ulYMin;
    uint16_t                    ulYMax;
    uint16_t                    ulCbMin;
    uint16_t                    ulCbMax;
    uint16_t                    ulCrMin;
    uint16_t                    ulCrMax;
} BVDC_P_CscClamp;

/* convert BAVC_MatrixCoefficients to BAVC_P_Colorimetry
 */
BAVC_P_Colorimetry BVDC_P_AvcMatrixCoeffs_to_Colorimetry_isr
    ( BAVC_MatrixCoefficients  eMatrixCoeffs,
      BAVC_ColorPrimaries      ePrimaries,
      bool                     bXvYcc);

/* generic avc color space init
 */
void BVDC_P_Cfc_InitAvcColorSpace(
    BAVC_P_ColorSpace          *pColorSpace );

/* Configure a CFC according to its input and output color space
 *
 */
void BVDC_P_Cfc_UpdateCfg_isr
    ( BVDC_P_CfcContext        *pCfc,
      bool                      bMosaicMode,
      bool                      bForceDirty);

/* Build RDC RUL for ram LUT loading
 */
#if (BVDC_P_CMP_CFC_VER >= BVDC_P_CFC_VER_3)
void BVDC_P_Cfc_PrintCscRx4_isr(const uint32_t *pCur, uint32_t ulCfg, bool bUseAlt);

/* this function could be called to build CMP/GFD/VEC LRange adjust
 */
void BVDC_P_Cfc_BuildRulForLRAdj_isr(
    const BVDC_P_CfcLRangeAdj       *pLRangeAdj,
    uint32_t                         ulStartReg,
    BVDC_P_ListInfo                 *pList);

/* Build LUT RUL for RAM (NL2L/L2NL/LMR) loading and RDC RUL for usage control
 */
void BVDC_P_Cfc_BuildRulForRamLut_isr
    ( const BVDC_P_RamLut             *pRamLut,
      uint32_t                         ulStartReg,
      uint32_t                         ulLutId,
      BVDC_P_CfcLutLoadListInfo       *pLutList,
      BVDC_P_ListInfo                 *pList);

/* Build RDC RUL for ram LUT loading
 */
void BVDC_P_Cfc_BuildRulForLutLoading_isr
    ( BVDC_P_CfcLutLoadListInfo *pLutList,
      uint32_t                   ulStartReg, /* *_LUT_DESC_ADDR */
      BVDC_P_ListInfo           *pList);

#if BVDC_P_DBV_SUPPORT
void BVDC_P_Dbv_UpdateVideoInputColorSpace_isr(
    BAVC_P_ColorSpace            *pColorSpace,
    const BAVC_MVD_Field         *pMvdFieldData );
void BVDC_P_Dbv_UpdateGfxInputColorSpace_isr(
    BVDC_Compositor_Handle        hCompositor,
    const BAVC_P_ColorSpace      *pColorSpace );
#endif
#if BVDC_P_TCH_SUPPORT
void BVDC_P_Tch_UpdateVideoInputColorSpace_isr(
    BAVC_P_ColorSpace            *pColorSpace,
    const BAVC_MVD_Field         *pMvdFieldData );
#endif
#endif

void BVDC_P_Csc_Mult_isr
    ( const int32_t                        *pA,  /* matrix A element buf ptr */
      int                                   iAc, /* matrxi A's number of columns */
      const int32_t                        *pB,  /* matrix B element buf ptr */
      int                                   iBc, /* matrxi B's number of columns */
      int32_t                              *pM ); /* matrix M element buf ptr */

void BVDC_P_Cfc_FromMatrix_isr
    ( BVDC_P_CfcContext        *pCfc,
      const int32_t             pl32_Matrix[BVDC_CSC_COEFF_COUNT],
      uint32_t                  ulShift );

void BVDC_P_Cfc_ToMatrix_isr
    ( int32_t                   pl32_Matrix[BVDC_CSC_COEFF_COUNT],
      const BVDC_P_Csc3x4      *pCsc,
      uint32_t                  ulShift );

void BVDC_P_Cfc_GetCfcToApplyAttenuationRGB_isr
    ( BAVC_P_Colorimetry        eColorimetry,
      const BVDC_P_Csc3x4     **ppYCbCrToRGB,
      const BVDC_P_Csc3x4     **ppRGBToYCbCr );

BERR_Code BVDC_P_Cfc_ColorTempToAttenuationRGB
    ( int16_t                          sColorTemp,
      int32_t                         *plAttenuationR,
      int32_t                         *plAttenuationG,
      int32_t                         *plAttenuationB,
      BVDC_P_Csc3x4                   *pCscCoeffs );
uint32_t BVDC_P_Compositor_Update_Canvas_Background_isrsafe
    ( BVDC_Compositor_Handle           hCompositor,
      uint8_t                          ucRed,
      uint8_t                          ucGreen,
      uint8_t                          ucBlue);

void BVDC_P_Cfc_ApplyContrast_isr
    ( int16_t                          sContrast,
      BVDC_P_Csc3x4                   *pCscCoeffs );

void BVDC_P_Cfc_ApplyBrightness_isr
    ( int16_t                          sBrightness,
      BVDC_P_Csc3x4                   *pCscCoeffs );

void BVDC_P_Cfc_ApplySaturationAndHue_isr
    ( int16_t                          sSaturation,
      int16_t                          sHue,
      BVDC_P_Csc3x4                   *pCscCoeffs );

void BVDC_P_Cfc_ApplyAttenuationRGB_isr
    ( int32_t                          lAttenuationR,
      int32_t                          lAttenuationG,
      int32_t                          lAttenuationB,
      int32_t                          lOffsetR,
      int32_t                          lOffsetG,
      int32_t                          lOffsetB,
      BVDC_P_Csc3x4                   *pCscCoeffs,
      const BVDC_P_Csc3x4             *pYCbCrToRGB,
      const BVDC_P_Csc3x4             *pRGBToYCbCr,
      bool                             bUserCsc);

void BVDC_P_Cfc_DvoApplyAttenuationRGB_isr
    ( int32_t                          lAttenuationR,
      int32_t                          lAttenuationG,
      int32_t                          lAttenuationB,
      int32_t                          lOffsetR,
      int32_t                          lOffsetG,
      int32_t                          lOffsetB,
      BVDC_P_Csc3x4                   *pCscCoeffs );

void BVDC_P_Cfc_ApplyYCbCrColor_isr
    ( BVDC_P_Csc3x4                   *pCscCoeffs,
      uint32_t                         ulColor0,
      uint32_t                         ulColor1,
      uint32_t                         ulColor2 );

#ifdef __cplusplus
}
#endif

#endif /* #ifndef BVDC_CFC_PRIV_H__ */
/* End of file. */
