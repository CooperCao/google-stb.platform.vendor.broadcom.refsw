/******************************************************************************
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
 ******************************************************************************/
#ifndef BVDC_CFC_PRIV_H__
#define BVDC_CFC_PRIV_H__

#include "bmth_fix.h"
#include "bmth_fix_matrix.h"

#ifdef __cplusplus
extern "C" {
#endif


#define BVDC_P_IS_SDR(tf) \
    ((tf)==BAVC_P_ColorTF_eBt1886 || (tf)==BAVC_P_ColorTF_eHlg)

#define BVDC_P_NEED_TF_CONV(i, o) \
    !(((i) == (o)) || \
      (((i)==BAVC_P_ColorTF_eHlg) && (o)== BAVC_P_ColorTF_eBt1886))

#define BVDC_P_IS_BT2020(c) \
    (BAVC_P_Colorimetry_eBt2020 == (c))

#define BVDC_P_NEED_BLEND_MATRIX(hCompositor) \
    ((hCompositor)->abBlenderUsed[0] && \
     (hCompositor)->stCfcCapability[0].stBits.bCscBlendOut && \
     !BVDC_P_IS_SDR((hCompositor)->stOutColorSpace.stAvcColorSpace.eColorTF))

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
    BVDC_P_CscType_eMa5x4_25 =    (22) | (0x54<<8) | (0<<16),  /* s2.22 / s9.15: CMP MA in 7271 B0 and up */

    BVDC_P_CscType_eMb3x4_25 =    (22) | (0x34<<8) | (1<<16),  /* s2.22 / s9.15: CMP/GFD MB in 7271 A0/up, DVI MB in 7271 B0/up */

    BVDC_P_CscType_eMc3x4_16 =    (13) | (0x34<<8) | (2<<16),  /* s2.13 / s9.6:  CMP MC */
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

/*
Summary:
    Pixel color format type.

Description:

See Also:

*/
typedef enum
{
    BAVC_P_ColorFormat_eRGB = 0,   /* non-linear */
    BAVC_P_ColorFormat_eYCbCr,     /* regular YCbCr, i.e. non-constant luminance */
    BAVC_P_ColorFormat_eYCbCr_CL,  /* constant luminance.  does bt709 have CL combination ??? */
    BAVC_P_ColorFormat_eMax,
    BAVC_P_ColorFormat_eInvalid = BAVC_P_ColorFormat_eMax

} BAVC_P_ColorFormat;

/*
Summary:
    Pixel colorimetry type.

Description:

See Also:
pColorSpace->eColorFormat
*/
typedef enum BAVC_P_Colorimetry
{
    BAVC_P_Colorimetry_eBt709,
    BAVC_P_Colorimetry_eSmpte170M,      /* ntsc / pal digital */
    BAVC_P_Colorimetry_eBt470_BG,       /* pal analog */
    BAVC_P_Colorimetry_eBt2020,
    BAVC_P_Colorimetry_eXvYcc601,
    BAVC_P_Colorimetry_eXvYcc709,       /* matrix is same as BT709 */
    BAVC_P_Colorimetry_eFcc,            /* only for input */
    BAVC_P_Colorimetry_eSmpte240M,      /* only for input */
    BAVC_P_Colorimetry_eMax,
    BAVC_P_Colorimetry_eInvalid = BAVC_P_Colorimetry_eMax
} BAVC_P_Colorimetry;

/*
Summary:
    Pixel color range

Description:
    Used to specify color component range

See Also:

*/
typedef enum BAVC_P_ColorRange {

    BAVC_P_ColorRange_eLimited,
    BAVC_P_ColorRange_eFull,
    BAVC_P_ColorRange_eMax
} BAVC_P_ColorRange;

/*
Summary:
    Video transfer function

Description:
    Used to specify transfer function, such as OETF for input, EOTF for display.

See Also:

*/
typedef enum BAVC_P_ColorTF {
    BAVC_P_ColorTF_eBt1886,      /* SDR */
    BAVC_P_ColorTF_eBt2100Pq,    /* i.e. HDR-PQ, or HDR10 */
    BAVC_P_ColorTF_eHlg,         /* Hybrid Log-Gamma */
    BAVC_P_ColorTF_eMax
} BAVC_P_ColorTF;

/*
Summary:
    Defines pixel sample's bit depth per component.

Description:
    This enum is used to report the pixel sample bit depth per component.
    Conventional video codecs support 8-bit per component video sample.
    while h265 video spec also supports 10-bit video.

See Also:
*/
typedef enum BAVC_P_ColorDepth
{
    BAVC_P_ColorDepth_e8Bit = 0,
    BAVC_P_ColorDepth_e10Bit,
    BAVC_P_ColorDepth_e12Bit,
    BAVC_P_ColorDepth_e16Bit,
    BAVC_P_ColorDepth_ePacked16,
    BAVC_P_ColorDepth_eMax

} BAVC_P_ColorDepth;

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
} BVDC_P_Cfc_Config;

/*
 * Color Space
 *
 */
typedef struct BVDC_P_ColorSpace
{
    BAVC_P_ColorSpace           stAvcColorSpace;

    BVDC_P_Cfc_Config           stCfg;

    /* non dynamically changing matrix might be a ptr in future ??? */
    BVDC_P_Csc3x4               stM3x4; /* ColorRange adjusted Ma for input, Mc for display */
    BVDC_P_Csc3x3               stMb;   /* 2XYZ for input, fromXYZ for output */
    BVDC_P_Csc3x4               stMalt; /* CL pos alt, Ma with input, Mc with display */

} BVDC_P_ColorSpace;

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

/****************************************************************************
 * CFC (color format conversion)
 *
 */

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

    /* Must be last */
    BVDC_P_CfcId_eUnknown
} BVDC_P_CfcId;

#define  BVDC_P_CFC_IN_GFD(i) \
    ((BVDC_P_CfcId_eComp0_G0 <= (i)) && ((i) <= BVDC_P_CfcId_eComp6_G0))

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
        uint32_t                 bHlgOotfAdj              : 1;
        uint32_t                 bLRngAdj                 : 1;
        uint32_t                 bL2NL                    : 1;
        uint32_t                 bMc                      : 1;  /* 8 */
        uint32_t                 bDbvToneMapping          : 1;
        uint32_t                 bCscBlendIn              : 1;
        uint32_t                 bCscBlendOut             : 1;
        uint32_t                 bAlphaDiv                : 1;  /* 12 */
        uint32_t                 bBlackBoxNLConv          : 1;  /* in 7439 B0 and 7271 A0 CMP1 */
    } stBits;

    uint32_t ulInts;
} BVDC_P_Cfc_Capability;

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

    BVDC_P_ColorSpace           stColorSpaceIn;
    BVDC_P_ColorSpace          *pColorSpaceOut;  /* e.g. ptr to struct in hCompositor or hDisplay */

    uint8_t                     ucRulBuildCntr;  /* for RUL build */

    uint8_t                     ucSelBlackBoxNL;

    bool                        bBlendInMatrixOn;
    bool                        bBypassCfc;

    BVDC_P_CfcLRangeAdj         stLRangeAdj;
    const BVDC_P_Csc3x4        *pMa;
    BVDC_P_Csc3x3               stMb;  /* MbOut * MbIn */
    BVDC_P_Csc3x4               stMc;  /* McOut, or McOut * MbOut * MbIn * MaIn, or identity */

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
