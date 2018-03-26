/******************************************************************************
 *  Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.
 ******************************************************************************/

#include "bcfc.h"
#include "bchp_gfd_0.h"
#ifdef BCHP_CMP_1_REG_START
#include "bchp_cmp_1.h"
#endif

BDBG_MODULE(BVDC_CFC);
BDBG_FILE_MODULE(BCFC_1); /* print CFC in and out color space info */
BDBG_FILE_MODULE(BCFC_3); /* print CFC matrix and LRAdjust value */
BDBG_FILE_MODULE(BCFC_4); /* print more implementation detail info */
BDBG_FILE_MODULE(BCFC_VFC_1); /* print VFC in and out color space info */
BDBG_FILE_MODULE(BCFC_VFC_4); /* print more implementation detail info */

/* --------------------------------------------------------------------
 * misc matrix
 */

static const BCFC_Csc3x4  s_Csc3x4Identity = BCFC_MAKE_CSC_3x4
    (  1.000000,   0.000000,   0.000000,     0.00000000,
       0.000000,   1.000000,   0.000000,     0.00000000,
       0.000000,   0.000000,   1.000000,     0.00000000 );

static const BCFC_Csc3x3  s_Csc3x3Identity = BCFC_MAKE_CSC_3x3
    (  1.000000,   0.000000,   0.000000,
       0.000000,   1.000000,   0.000000,
       0.000000,   0.000000,   1.000000 );

static const BCFC_Csc3x4  s_CscCr0p85Adj_BT2020_to_NonBT2020 = BCFC_MAKE_CSC_3x4
    (  1.000000,   0.000000,   0.000000,     0.00000000,
       0.000000,   1.000000,   0.000000,     0.00000000,
       0.000000,   0.000000,   0.850000,    19.20000000 );

static const BCFC_Csc3x4  s_CscCr0p85Adj_NonBT2020_to_BT2020 = BCFC_MAKE_CSC_3x4
    (  1.000000,   0.000000,   0.000000000,     0.00000000,
       0.000000,   1.000000,   0.000000000,     0.00000000,
       0.000000,   0.000000,   1.176470588,   -22.58823529 );

static const BCFC_Csc3x4  s_CscYCbCr_Limited_to_Full = BCFC_MAKE_CSC_3x4
    (  1.164383562,    0.000000000,    0.000000000,    -18.63013699,
       0.000000000,    1.138392857,    0.000000000,    -17.71428571,
       0.000000000,    0.000000000,    1.138392857,    -17.71428571 );

static const BCFC_Csc3x4  s_CscYCbCr_Full_to_Limited = BCFC_MAKE_CSC_3x4
    (  0.858823529,    0.000000000,    0.000000000,     16.00000000,
       0.000000000,    0.878431373,    0.000000000,     15.56078431,
       0.000000000,    0.000000000,    0.878431373,     15.56078431 );

static const BCFC_Csc3x4  s_CscRGB_Full_to_Limited = BCFC_MAKE_CSC_3x4
    (  0.858824,    0.000000,    0.000000,    16.000000,
       0.000000,    0.858824,    0.000000,    16.000000,
       0.000000,    0.000000,    0.858824,    16.000000 );

static const BCFC_Csc3x4  s_CscRGB_Limited_to_Full = BCFC_MAKE_CSC_3x4
    (  1.164384,    0.000000,    0.000000,    -18.630137,
       0.000000,    1.164384,    0.000000,    -18.630137,
       0.000000,    0.000000,    1.164384,    -18.630137 );

/* --------------------------------------------------------------------
 * MA: YCbCr -> R'G'B'
 */

/* BT709 YCbCr -> R'G'B' (typically HD) */
static const BCFC_Csc3x4 s_BT709_YCbCr_to_RGB = BCFC_MAKE_CSC_3x4
    (  1.164384,      0.000000,      1.792741,     -248.100994,
       1.164384,     -0.213249,     -0.532909,       76.878080,
       1.164384,      2.112402,      0.000000,     -289.017566 );

/* 170M YCbCr -> R'G'B' (typically Ntsc SD, or HDMI Pal SD) */
static const BCFC_Csc3x4 s_170M_YCbCr_to_RGB = BCFC_MAKE_CSC_3x4
    (  1.164384,      0.000000,      1.596027,     -222.921566,
       1.164384,     -0.391762,     -0.812968,      135.575295,
       1.164384,      2.017232,      0.000000,     -276.835851 );

/* BT470_2_BG YCbCr -> R'G'B' (typically analog Pal SD) */
static const BCFC_Csc3x4 s_BT470_BG_YCbCr_to_RGB = BCFC_MAKE_CSC_3x4
    (  1.164384,      0.000000,      1.596027,     -222.921566,
       1.164384,     -0.391762,     -0.812968,      135.575295,
       1.164384,      2.017232,      0.000000,     -276.835851 );

/* BT2020 NCL YCbCr -> R'G'B' (typically UHD) */
static const BCFC_Csc3x4 s_BT2020NCL_YCbCr_to_RGB = BCFC_MAKE_CSC_3x4
    (  1.164384,      0.000000,      1.678674,     -233.500423,
       1.164384,     -0.187326,     -0.650424,       88.601917,
       1.164384,      2.141772,      0.000000,     -292.776994 );

/* SMPTE240 YCbCr -> R'G'B' (obsolete HD) */
static const BCFC_Csc3x4 s_240M_YCbCr_to_RGB = BCFC_MAKE_CSC_3x4
    (  1.164384,      0.000000,      1.794107,     -248.275851,
       1.164384,     -0.257985,     -0.542583,       83.842551,
       1.164384,      2.078705,      0.000000,     -284.704423 );

/* FCC / NTSC 1953 YCbCr -> R'G'B' */
static const BCFC_Csc3x4 s_FCC_YCbCr_to_RGB = BCFC_MAKE_CSC_3x4
    (  1.164384,      0.000000,      1.596027,     -222.921566,
       1.164384,     -0.391762,     -0.812968,      135.575295,
       1.164384,      2.017232,      0.000000,     -276.835851 );

/* BT2020 CL YCbCr -> R'YB' (typically UHD) with TF 1886 */
static const BCFC_Csc3x4 s_BT2020CL_YCbCr_to_RYB_Bt1886_Negative = BCFC_MAKE_CSC_3x4
    (  1.164384,      0.000000,      1.956214,     -269.025566,   /* R */
       1.164384,      0.000000,      0.000000,      -18.630137,   /* Y */
       1.164384,      2.208938,      0.000000,     -301.374137 ); /* B */

/* BT2020 CL YCbCr -> R'YB' (typically UHD) with TF 1886 */
/* note: swap row 0 and 1 to satisfy the needs of BVDC_P_Cfc_BuildRulForCscRx4_isr */
static const BCFC_Csc3x4 s_BT2020CL_YCbCr_to_RYB_Bt1886_Positive = BCFC_MAKE_CSC_3x4
    (  1.164384,      0.000000,      0.000000,      -18.630137,   /* Y */
       1.164384,      0.000000,      1.131107,     -163.411851,   /* R */
       1.164384,      1.800482,      0.000000,     -249.091851 ); /* B */

#define s_XvYCC601_YCbCr_to_RGB          s_170M_YCbCr_to_RGB

static const BCFC_Csc3x4 *const s_aCFC_MA_Tbl[] =
{
    &s_BT709_YCbCr_to_RGB,                /* BCFC_Colorimetry_eBt709 */
    &s_170M_YCbCr_to_RGB,                 /* BCFC_Colorimetry_eSmpte170M */
    &s_BT470_BG_YCbCr_to_RGB,             /* BCFC_Colorimetry_eBt470_BG */
    &s_BT2020NCL_YCbCr_to_RGB,            /* BCFC_Colorimetry_eBt2020 */
    &s_XvYCC601_YCbCr_to_RGB,             /* BCFC_Colorimetry_eXvYcc601 */
    &s_BT709_YCbCr_to_RGB,                /* BCFC_Colorimetry_eXvYcc709 */
    &s_FCC_YCbCr_to_RGB,                  /* BCFC_Colorimetry_eFcc */
    &s_240M_YCbCr_to_RGB                  /* BCFC_Colorimetry_eSmpte_240M */
};

/* --------------------------------------------------------------------
 * MC: R'G'B' -> YCbCr
 */

/* BT709 R'G'B' -> YCbCr (typically HD) */
static const BCFC_Csc3x4 s_BT709_RGB_to_YCbCr = BCFC_MAKE_CSC_3x4
    (  0.182586,      0.614231,      0.062007,       16.000000,
      -0.100644,     -0.338572,      0.439216,      128.000000,
       0.439216,     -0.398942,     -0.040274,      128.000000 );

/* 170M R'G'B' -> YCbCr (typically Ntsc SD, or HDMI Pal SD) */
static const BCFC_Csc3x4 s_170M_RGB_to_YCbCr = BCFC_MAKE_CSC_3x4
    (  0.256788,      0.504129,      0.097906,       16.000000,
      -0.148223,     -0.290993,      0.439216,      128.000000,
       0.439216,     -0.367788,     -0.071427,      128.000000 );

/* BT470_2_BG R'G'B' -> YCbCr (typically analog Pal SD ) */
static const BCFC_Csc3x4 s_BT470_BG_RGB_to_YCbCr = BCFC_MAKE_CSC_3x4
    (  0.256788,      0.504129,      0.097906,       16.000000,
      -0.148223,     -0.290993,      0.439216,      128.000000,
       0.439216,     -0.367788,     -0.071427,      128.000000 );

/* BT2020 NCL R'G'B' -> YCbCr (typically UHD) */
static const BCFC_Csc3x4 s_BT2020NCL_RGB_to_YCbCr = BCFC_MAKE_CSC_3x4
    (  0.225613,      0.582282,      0.050928,       16.000000,
      -0.122655,     -0.316560,      0.439216,      128.000000,
       0.439216,     -0.403890,     -0.035325,      128.000000 );

/* BT2020 CL R'Y'B' -> YCbCr (typically UHD) for negative CbCr with TF 1886 */
static const BCFC_Csc3x4 s_BT2020CL_RYB_to_YCbCr_Bt1886_Negative = BCFC_MAKE_CSC_3x4
    (  0.000000,      0.858824,      0.000000,       16.000000,
       0.000000,     -0.452706,      0.452706,      128.000000,
       0.511191,     -0.511191,      0.000000,      128.000000 );

/* BT2020 CL R'Y'B' -> YCbCr (typically UHD) for positive CbCr with TF 1886 */
static const BCFC_Csc3x4 s_BT2020CL_RYB_to_YCbCr_Bt1886_Positive = BCFC_MAKE_CSC_3x4
    (  0.000000,      0.858824,      0.000000,       16.000000,
       0.000000,     -0.555407,      0.555407,      128.000000,
       0.884090,     -0.884090,      0.000000,      128.000000 );

#define s_XvYCC601_RGB_to_YCbCr          s_170M_RGB_to_YCbCr

static const BCFC_Csc3x4 *const s_aCFC_MC_Tbl[] =
{
    &s_BT709_RGB_to_YCbCr,                /* BCFC_Colorimetry_eBt709 */
    &s_170M_RGB_to_YCbCr,                 /* BCFC_Colorimetry_eSmpte170M */
    &s_BT470_BG_RGB_to_YCbCr,             /* BCFC_Colorimetry_eBt470_BG */
    &s_BT2020NCL_RGB_to_YCbCr,            /* BCFC_Colorimetry_eBt2020 */
    &s_XvYCC601_RGB_to_YCbCr,             /* BCFC_Colorimetry_eXvYcc601 */
    &s_BT709_RGB_to_YCbCr                 /* BCFC_Colorimetry_eXvYcc709 */
};

/* --------------------------------------------------------------------
 * MB input: RGB -> XYZ
 */

static const BCFC_Csc3x3 s_BT709_RGB_to_XYZ = BCFC_MAKE_CSC_3x3
    (  0.412391,      0.357584,      0.180481,
       0.212639,      0.715169,      0.072192,
       0.019331,      0.119195,      0.950532 );

static const BCFC_Csc3x3 s_170M_RGB_to_XYZ = BCFC_MAKE_CSC_3x3
    (  0.393521,      0.365258,      0.191677,
       0.212376,      0.701060,      0.086564,
       0.018739,      0.111934,      0.958385 );

static const BCFC_Csc3x3 s_BT470_BG_RGB_to_XYZ = BCFC_MAKE_CSC_3x3
    (  0.431943,      0.341235,      0.178189,
       0.222721,      0.706003,      0.071276,
       0.020247,      0.129434,      0.938465 );

static const BCFC_Csc3x3 s_240M_RGB_to_XYZ = BCFC_MAKE_CSC_3x3
    (  0.393521,      0.365258,      0.191677,
       0.212376,      0.701060,      0.086564,
       0.018739,      0.111934,      0.958385 );

static const BCFC_Csc3x3 s_FCC_RGB_to_XYZ = BCFC_MAKE_CSC_3x3
    (  0.606993,      0.173449,      0.200571,
       0.298967,      0.586421,      0.114612,
       0.000000,      0.066076,      1.117469 );

static const BCFC_Csc3x3 s_BT2020NCL_RGB_to_XYZ = BCFC_MAKE_CSC_3x3
    (  0.636958,      0.144617,      0.168881,
       0.262700,      0.677998,      0.059302,
       0.000000,      0.028073,      1.060985);
#if 0
static const BCFC_Csc3x3 s_BT2020CL_RYB_to_XYZ = BCFC_MAKE_CSC_3x3
    (  0.580924,      0.213299,      0.156232,
       0.000001,      0.999997,      0.000002,
      -0.010877,      0.041405,      1.058530 );
#endif
#if (BCFC_VERSION >= BCFC_VER_1) /* #if (BVDC_P_CMP_CFC_VER >= BVDC_P_CFC_VER_1) */
static const BCFC_Csc3x3 s_BT2020NCL_RGB_to_RYB = BCFC_MAKE_CSC_3x3
    (  1.000000,      0.000000,     0.000000,
       0.262700,      0.678000,     0.059300,
       0.000000,      0.000000,     1.000000 );
#endif

static const BCFC_Csc3x3 s_BT2020NCL_RYB_to_RGB = BCFC_MAKE_CSC_3x3
    (  1.000000,      0.000000,     0.000000,
      -0.387463,      1.474926,    -0.087463,
       0.000000,      0.000000,     1.000000 );

#define s_XvYCC601_RGB_to_XYZ   s_170M_RGB_to_XYZ
#define s_XvYCC601_XYZ_to_RGB   s_170M_XYZ_to_RGB
static const BCFC_Csc3x3 *const s_aCFC_MB_IN_Tbl[] =
{
    &s_BT709_RGB_to_XYZ,                /* BCFC_Colorimetry_eBt709 */
    &s_170M_RGB_to_XYZ,                 /* BCFC_Colorimetry_eSmpte170M */
    &s_BT470_BG_RGB_to_XYZ,             /* BCFC_Colorimetry_eBt470_BG */
    &s_BT2020NCL_RGB_to_XYZ,            /* BCFC_Colorimetry_eBt2020 */
    &s_XvYCC601_RGB_to_XYZ,             /* BCFC_Colorimetry_eXvYcc601 */
    &s_BT709_RGB_to_XYZ,                /* BCFC_Colorimetry_eXvYcc709 */
    &s_FCC_RGB_to_XYZ,                  /* BCFC_Colorimetry_eFcc */
    &s_240M_RGB_to_XYZ                  /* BCFC_Colorimetry_eSmpte_240M */
};

/* --------------------------------------------------------------------
 * MB output: XYZ -> RGB
 */

static const BCFC_Csc3x3 s_BT709_XYZ_to_RGB = BCFC_MAKE_CSC_3x3
    (  3.240970,      -1.537383,     -0.498611,
      -0.969244,       1.875968,      0.041555,
       0.055630,      -0.203977,      1.056972 );

static const BCFC_Csc3x3 s_170M_XYZ_to_RGB = BCFC_MAKE_CSC_3x3
    (  3.506003,     -1.739791,     -0.544058,
      -1.069048,      1.977779,      0.035171,
       0.056307,     -0.196976,      1.049952 );

static const BCFC_Csc3x3 s_BT470_BG_XYZ_to_RGB = BCFC_MAKE_CSC_3x3
    (  3.053507,      -1.388908,     -0.474293,
      -0.970138,      1.877698,      0.041593,
       0.067923,     -0.229008,      1.070067 );

static const BCFC_Csc3x3 s_BT2020NCL_XYZ_to_RGB = BCFC_MAKE_CSC_3x3
    (  1.716651,     -0.355671,     -0.253366,
      -0.666684,      1.616481,      0.015769,
       0.017640,     -0.042771,      0.942103 );
#if 0
static const BCFC_Csc3x3 s_BT2020CL_XYZ_to_RYB = BCFC_MAKE_CSC_3x3
    (  1.716651,     -0.355671,     -0.253366,
      -0.000002,      1.000003,     -0.000002,
       0.017640,     -0.042771,      0.942103 );
#endif
static const BCFC_Csc3x3 *const s_aCFC_MB_OUT_Tbl[] =
{
    &s_BT709_XYZ_to_RGB,                /* BCFC_Colorimetry_eBt709 */
    &s_170M_XYZ_to_RGB,                 /* BCFC_Colorimetry_eSmpte170M */
    &s_BT470_BG_XYZ_to_RGB,             /* BCFC_Colorimetry_eBt470_BG */
    &s_BT2020NCL_XYZ_to_RGB,            /* BCFC_Colorimetry_eBt2020 */
    &s_XvYCC601_XYZ_to_RGB,             /* BCFC_Colorimetry_eXvYcc601 */
    &s_BT709_XYZ_to_RGB                 /* BCFC_Colorimetry_eXvYcc709 */
};

/* --------------------------------------------------------------------
 * utility funcs to return basic matrix MA, MB_IN, MB_OUT and MC
 *
 */
const BCFC_Csc3x4 *BCFC_GetCsc3x4_Ma_isrsafe(BCFC_Colorimetry eColorimetry)
{
    return s_aCFC_MA_Tbl[eColorimetry];
}

const BCFC_Csc3x4 *BCFC_GetCsc3x4_Mc_isrsafe(BCFC_Colorimetry eColorimetry)
{
    return s_aCFC_MC_Tbl[eColorimetry];
}

const BCFC_Csc3x3 *BCFC_GetCsc3x3_MbIn_isrsafe(BCFC_Colorimetry eColorimetry)
{
    return s_aCFC_MB_IN_Tbl[eColorimetry];
}

const BCFC_Csc3x3 *BCFC_GetCsc3x3_MbOut_isrsafe(BCFC_Colorimetry eColorimetry)
{
    return s_aCFC_MB_OUT_Tbl[eColorimetry];
}

const BCFC_Csc3x4 *BCFC_GetCsc3x4_Cr0p85Adj_NonBT2020_to_BT2020_isrsafe(void)
{
    return &s_CscCr0p85Adj_NonBT2020_to_BT2020;
}

const BCFC_Csc3x4 *BCFC_GetCsc3x4_Cr0p85Adj_BT2020_to_NonBT2020_isrsafe(void)
{
    return &s_CscCr0p85Adj_BT2020_to_NonBT2020;
}

const BCFC_Csc3x4 *BCFC_GetCsc3x4_YCbCr_Limited_to_Full_isrsafe(void)
{
    return &s_CscYCbCr_Limited_to_Full;
}

const BCFC_Csc3x4 *BCFC_GetCsc3x4_Identity_isrsafe(void)
{
    return &s_Csc3x4Identity;
}

const BCFC_Csc3x4 *BCFC_GetCsc3x4_BT709_YCbCrtoRGB_isrsafe(void)
{
    return &s_BT709_YCbCr_to_RGB;
}

/* --------------------------------------------------------------------
 * Generic CFC matrix multiply M = A * B.
 *
 * Matrix M, A and B must be 3 * 3 or 3 * 4 matrices. During matrix
 * multiplication a 3 * 4 matrix B is considered as a 4 * 4 matrix
 * with last row being (0, 0, 0, 1.0)
 *
 *   [A00   A01]    [B00   B01]    [A00*B00   A00*B01+A01]
 *   |         |  * |         | =  |                     |
 *   [  0     1]    [  0     1]    [      0             1]
 *
 * If A has 4 column and B is a 3 * 3 matrix, B is also considered a 4 * 4 matrix
 * with last row and last column being (0, 0, 0, 1.0)
 *
 *   [A00   A01]    [B00   B01]    [A00*B00   A00*B01+A01]
 *   |         |  * |         | =  |                     |
 *   [  0     1]    [  0     1]    [      0             1]
 *
 * All matrices' CX elements have int/frac bits as BCFC_CSC_SW_CX_I_BITS
 * /BCFC_CSC_SW_CX_F_BITS, and CO elements have int/frac bits
 * BCFC_CSC_SW_CO_I_BITS / BCFC_CSC_SW_CO_F_BITS.
*/
void BCFC_Csc_Mult_isrsafe
    ( const int32_t                        *pA,  /* matrix A element buf ptr */
      int                                   iAc, /* matrxi A's number of columns */
      const int32_t                        *pB,  /* matrix B element buf ptr */
      int                                   iBc, /* matrxi B's number of columns */
      int32_t                              *pM ) /* matrix M element buf ptr */
{
    int32_t ii, jj, iMc;

    BDBG_ASSERT(((iAc == 3) || (iAc == 4)) && ((iBc == 3) || (iBc == 4)));

    iMc = (iAc > iBc)? iAc : iBc;
    for (ii=0; ii<3; ii++)
    {
        for (jj=0; jj<iBc; jj++)
        {
            uint32_t t;
            t = (uint32_t)(0x00000000FFFFFFFF &
                           (((int64_t)(*(pA + ii * iAc + 0)) * (*(pB + 0 * iBc + jj)) +
                             (int64_t)(*(pA + ii * iAc + 1)) * (*(pB + 1 * iBc + jj)) +
                             (int64_t)(*(pA + ii * iAc + 2)) * (*(pB + 2 * iBc + jj)) +
                             (1 << (BCFC_CSC_SW_CX_F_BITS - 1))) >> BCFC_CSC_SW_CX_F_BITS));
            *(pM + ii * iMc + jj) = *(int32_t *)(&t);
        }

        if ((iBc == 4) && (iAc == 4))
        {
            *(pM + ii * iMc + 3) += *(pA + ii * iAc + 3);
        }
        else if (iAc == 4) /* (iBc == 3) */
        {
            *(pM + ii * iMc + 3) = *(pA + ii * iAc + 3);
        }
    }
}

/* --------------------------------------------------------------------
 * get the 4x5 matrix to convert gfx YCbCr to RGB
 */
#define BVDC_P_MAKE_GFX_CX(a) ((a)>>(BCFC_CSC_SW_CX_F_BITS - ulShift))
#define BVDC_P_MAKE_GFX_CO(o) ((o)>>(BCFC_CSC_SW_CO_F_BITS - ulShift))
void BCFC_GetMatrixForGfxYCbCr2Rgb_isrsafe
    ( BAVC_MatrixCoefficients          eMatrixCoeffs,
      uint32_t                         ulShift,
      int32_t                         *pulCoeffs)
{
    int ii, jj;
    BCFC_Colorimetry eColorimetry = BCFC_AvcColorInfoToColorimetry_isrsafe(BAVC_ColorPrimaries_eUnknown, eMatrixCoeffs, false);
    const BCFC_Csc3x4 *pMatrix = s_aCFC_MA_Tbl[eColorimetry];

    for (ii=0; ii<3; ii++)
    {
        for (jj=0; jj<3; jj++)
        {
            *(pulCoeffs + ii*5 + jj) = BVDC_P_MAKE_GFX_CX(pMatrix->m[ii][jj]);
        }
        *(pulCoeffs + ii*5 + 3) = 0;
        *(pulCoeffs + ii*5 + 4) = BVDC_P_MAKE_GFX_CO(pMatrix->m[ii][3]);
    }

    for (jj=0; jj<5; jj++)
    {
        *(pulCoeffs + 3*5 + jj) = 0;
    }
    *(pulCoeffs + 3*5 + 3) = 1 << ulShift;

    return;
}

/* --------------------------------------------------------------------
 * utilty functons to print matrix
 */

#define BVDC_P_CSC_FLOATING_POINT_MSG     0

#define BVDC_P_CX_TO_FLOAT(x) \
    (((int32_t)((BCFC_CSC_SW_SIGN_MASK & x) ? -((BCFC_CSC_SW_MASK & ~x) + 1) : x) / (float)(1 << BCFC_CSC_SW_CX_F_BITS)))
#define BVDC_P_CO_TO_FLOAT(x) \
    (((int32_t)((BCFC_CSC_SW_SIGN_MASK & x) ? -((BCFC_CSC_SW_MASK & ~x) + 1) : x) / (float)(1 << BCFC_CSC_SW_CO_F_BITS)))

#if (BDBG_DEBUG_BUILD)
static const char *const s_MatrixName[] = {
    "MA",              /* 0 */
    "MB",              /* 1 */
    "MC",              /* 2 */
    "MblendIn",        /* 3 */
    "MblendOut",       /* 4 */
    "MB2",             /* 5 */
    "MA-ConstBlend",   /* 6 */
    "MA-AlphaBlend",   /* 7 */
    "MC-ConstBlend",   /* 8 */
    "MC-AlphaBlend",   /* 9 */
    "MA-ConstBlend",   /* 10 */
    "MA-AlphaBlend"    /* 11 */
};
#endif /* #if (BDBG_DEBUG_BUILD) */

/* */
void BCFC_PrintCscRx4_isrsafe(const uint32_t *pCur, uint32_t ulCfg, bool bUseAlt)
{
#if (BDBG_DEBUG_BUILD)
    BCFC_CscType eCscType;
    BCFC_LeftShift eLeftShift;
    uint32_t ulLeftShiftBits;
    uint32_t ulRows, ulColumns;
    int ii;

    eCscType = BCFC_GET_CSC_TYPE(ulCfg);
    eLeftShift = BCFC_GET_CSC_LSHIFT(ulCfg);

    ulRows = BCFC_CSC_ROWS(eCscType);
    ulColumns = BCFC_CSC_COLUMS(eCscType);
    ulLeftShiftBits = BCFC_LSHIFT_BITS(eLeftShift);
    if ((ulRows > 3) && !bUseAlt)
    {
        ulRows = 3;
    }

    BDBG_MODULE_MSG(BCFC_3,("      %s, lShift %d:", s_MatrixName[BCFC_CSC_NAME_IDX(eCscType)], ulLeftShiftBits));
    if (ulColumns == 4)
    {
        for (ii=0; ii<(int)ulRows; ii++)
        {
            BDBG_MODULE_MSG(BCFC_3,("       [   0x%08x    0x%08x    0x%08x    0x%08x]",
                *(pCur + ii*4 + 0), *(pCur + ii*4 + 1), *(pCur + ii*4 + 2), *(pCur + ii*4 + 3)));
        }
    }
    else
    {
        if (BCFC_CscType_eMcPacked == eCscType)
        {
            BDBG_ASSERT(ulColumns == 2);
            for (ii=0; ii<(int)ulRows; ii++)
            {
                BDBG_MODULE_MSG(BCFC_3,("       [   0x%08x    0x%08x", *(pCur + ii*2 + 0), *(pCur + ii*2 + 1)));
            }
        }
        else
        {
            BDBG_ASSERT(ulColumns == 5);
            for (ii=0; ii<(int)ulRows; ii++)
            {
                BDBG_MODULE_MSG(BCFC_3,("       [   0x%08x    0x%08x    0x%08x    0x%08x    0x%08x]",
                                            *(pCur + ii*5 + 0), *(pCur + ii*5 + 1), *(pCur + ii*5 + 2), *(pCur + ii*5 + 3), *(pCur + ii*5 + 4)));
            }
        }
    }
#else
    BSTD_UNUSED(pCur);
    BSTD_UNUSED(ulCfg);
    BSTD_UNUSED(bUseAlt);
#endif
}

/* */
void BCFC_PrintFloatCscRx4_isrsafe(const BCFC_Csc3x4 *pCsc, const BCFC_Csc3x4 *pAlt)
{
#if ((BVDC_P_CSC_FLOATING_POINT_MSG) && (BDBG_DEBUG_BUILD))
    int ii;

    for(ii=0; ii<3; ii++)
    {
        BDBG_MODULE_MSG(BCFC_3,("       [%13.8f %13.8f %13.8f %13.8f]",
            BVDC_P_CX_TO_FLOAT(pCsc->m[ii][0]), BVDC_P_CX_TO_FLOAT(pCsc->m[ii][1]), BVDC_P_CX_TO_FLOAT(pCsc->m[ii][2]), BVDC_P_CO_TO_FLOAT(pCsc->m[ii][3])));
    }

    if (pAlt==NULL)
        return;

    for(ii=1; ii<3; ii++)
    {
        BDBG_MODULE_MSG(BCFC_3,("       [%13.8f %13.8f %13.8f %13.8f]",
            BVDC_P_CX_TO_FLOAT(pAlt->m[ii][0]), BVDC_P_CX_TO_FLOAT(pAlt->m[ii][1]), BVDC_P_CX_TO_FLOAT(pAlt->m[ii][2]), BVDC_P_CO_TO_FLOAT(pAlt->m[ii][3])));
    }
#else
    BSTD_UNUSED(pCsc);
    BSTD_UNUSED(pAlt);
#endif
}

/*  */
void BCFC_PrintCsc3x3_isrsafe(const uint32_t *pCur, uint32_t ulCfg)
{
#if (BDBG_DEBUG_BUILD)
    int ii;
    BCFC_LeftShift eLeftShift;
    uint32_t ulLeftShiftBits;

    eLeftShift = BCFC_GET_CSC_LSHIFT(ulCfg);
    ulLeftShiftBits = BCFC_LSHIFT_BITS(eLeftShift);

    BDBG_MODULE_MSG(BCFC_3,("      MB, LShift %d:", ulLeftShiftBits));
    for (ii=0; ii<3; ii++)
    {
        BDBG_MODULE_MSG(BCFC_3,("       [   0x%08x    0x%08x    0x%08x    0x%08x]",
            *(pCur + ii*4 + 0), *(pCur + ii*4 + 1), *(pCur + ii*4 + 2), *(pCur + ii*4 + 3)));
    }
#else
    BSTD_UNUSED(pCur);
    BSTD_UNUSED(ulCfg);
#endif
}

/*  */
void BCFC_PrintFloatCsc3x3_isrsafe(const BCFC_Csc3x3 *pCsc)
{
#if ((BVDC_P_CSC_FLOATING_POINT_MSG) && (BDBG_DEBUG_BUILD))
    int ii;
    for (ii=0; ii<3; ii++)
    {
        BDBG_MODULE_MSG(BCFC_3,("       [%13.8f %13.8f %13.8f]",
            BVDC_P_CX_TO_FLOAT(pCsc->m[ii][0]), BVDC_P_CX_TO_FLOAT(pCsc->m[ii][1]), BVDC_P_CX_TO_FLOAT(pCsc->m[ii][2])));
    }
#else
    BSTD_UNUSED(pCsc);
#endif
}

/*------------------------------------------------------------------------
 * convert BAVC transferChracteristics to color info to BCFC_ColorTF
 */
BCFC_ColorTF BCFC_AvcTransferCharacteristicsToTF_isrsafe(BAVC_TransferCharacteristics eTransChar)
{

    BCFC_ColorTF eTF = BCFC_ColorTF_eBt1886;

    switch (eTransChar)
    {
        case BAVC_TransferCharacteristics_eSmpte_ST_2084:
            eTF = BCFC_ColorTF_eBt2100Pq;
            break;

        case BAVC_TransferCharacteristics_eArib_STD_B67:
            eTF = BCFC_ColorTF_eHlg;
            break;

        case BAVC_TransferCharacteristics_eUnknown:
        case BAVC_TransferCharacteristics_eItu_R_BT_709:
        case BAVC_TransferCharacteristics_eItu_R_BT_470_2_M:
        case BAVC_TransferCharacteristics_eItu_R_BT_470_2_BG:
        case BAVC_TransferCharacteristics_eSmpte_170M:
        case BAVC_TransferCharacteristics_eSmpte_240M:
        case BAVC_TransferCharacteristics_eLinear:
        case BAVC_TransferCharacteristics_eIec_61966_2_4:
        case BAVC_TransferCharacteristics_eItu_R_BT_2020_10bit:
        case BAVC_TransferCharacteristics_eItu_R_BT_2020_12bit:
            eTF = BCFC_ColorTF_eBt1886;
            break;

        default:
            BDBG_MSG(("Unsupported TransChar %d.  Assuming SDR.", eTransChar));
            break;
    }

    return eTF;
}

/* note: if we really treat BAVC_MatrixCoefficients_eFCC input as BVDC_P_Colorimetry_eFcc, some popular
 * contents such as cnnticker.mpg will look too red.  Those contents might have indeed been coded to
 * eSmpte170M, but the MatrixCoefficients info in mega data passed to us is wrongly put as eFCC. We have
 * always treated BAVC_MatrixCoefficients_eFCC input as BVDC_P_Colorimetry_eSmpte170M, and we have getton
 * used to the display effect, so lets keep the same
 */
static const BCFC_Colorimetry s_aAvcMatrixCoeffs_to_Colorimetry[] =
{
    BCFC_Colorimetry_eBt709,               /* BAVC_MatrixCoefficients_eHdmi_RGB = 0 */
    BCFC_Colorimetry_eBt709,               /* BAVC_MatrixCoefficients_eItu_R_BT_709 = 1, */
    BCFC_Colorimetry_eBt709,               /* BAVC_MatrixCoefficients_eUnknown = 2 */
    BCFC_Colorimetry_eBt709,               /* BAVC_MatrixCoefficients_eDvi_Full_Range_RGB = 3: Forbidden */
    BCFC_Colorimetry_eSmpte170M,           /* BAVC_MatrixCoefficients_eFCC = 4 */
    BCFC_Colorimetry_eBt470_BG,            /* BAVC_MatrixCoefficients_eItu_R_BT_470_2_BG = 5 */
    BCFC_Colorimetry_eSmpte170M,           /* BAVC_MatrixCoefficients_eSmpte_170M = 6 */
    BCFC_Colorimetry_eSmpte240M,           /* BAVC_MatrixCoefficients_eSmpte_240M = 7 */
    BCFC_Colorimetry_eBt709,               /* unused 8 */
    BCFC_Colorimetry_eBt2020,              /* BAVC_MatrixCoefficients_eItu_R_BT_2020_NCL = 9, */
    BCFC_Colorimetry_eBt2020,              /* BAVC_MatrixCoefficients_eItu_R_BT_2020_CL = 10, */
    BCFC_Colorimetry_eXvYcc709,            /* BAVC_MatrixCoefficients_eXvYCC_709 = 11 */
    BCFC_Colorimetry_eXvYcc601,            /* BAVC_MatrixCoefficients_eXvYCC_601 = 12 */
    BCFC_Colorimetry_eBt709                /* BAVC_MatrixCoefficients_eHdmi_Full_Range_YCbCr = 13 */
};

static const BCFC_Colorimetry s_aAvcColorPrimaries_to_Colorimetry[] =
{
    BCFC_Colorimetry_eBt709,               /* forbidden */
    BCFC_Colorimetry_eBt709,               /* BAVC_ColorPrimaries_eItu_R_BT_709 = 1 */
    BCFC_Colorimetry_eBt709,               /* BAVC_ColorPrimaries_eUnknown = 2 */
    BCFC_Colorimetry_eBt709,               /* reserved */
    BCFC_Colorimetry_eSmpte170M,           /* BAVC_ColorPrimaries_eItu_R_BT_470_2_M = 4 */
    BCFC_Colorimetry_eBt470_BG,            /* BAVC_ColorPrimaries_eItu_R_BT_470_2_BG = 5 */
    BCFC_Colorimetry_eSmpte170M,           /* BAVC_ColorPrimaries_eSmpte_170M = 6 */
    BCFC_Colorimetry_eSmpte240M,           /* BAVC_ColorPrimaries_eSmpte_240M = 7 */
    BCFC_Colorimetry_eBt709,               /* BAVC_ColorPrimaries_eGenericFilm = 8 */
    BCFC_Colorimetry_eBt2020,              /* BAVC_ColorPrimaries_eItu_R_BT_2020 = 9 */
};

/*------------------------------------------------------------------------
 * convert BAVC color info to BCFC_Colorimetry
 */
BCFC_Colorimetry BCFC_AvcColorInfoToColorimetry_isrsafe
    ( BAVC_ColorPrimaries              eColorPrimaries,
      BAVC_MatrixCoefficients          eMatrixCoeffs,
      bool                             bXvYcc)
{
    BCFC_Colorimetry eColorimetry;

    if ((eColorPrimaries == BAVC_ColorPrimaries_eItu_R_BT_709) ||
        ((eColorPrimaries <= BAVC_ColorPrimaries_eItu_R_BT_2020) && (eColorPrimaries >= BAVC_ColorPrimaries_eItu_R_BT_470_2_M)))
    {
        eColorimetry = s_aAvcColorPrimaries_to_Colorimetry[eColorPrimaries];
        if(eMatrixCoeffs == BAVC_MatrixCoefficients_eXvYCC_709)
        {
            eColorimetry = BCFC_Colorimetry_eXvYcc709;
        }
        else if(eMatrixCoeffs == BAVC_MatrixCoefficients_eXvYCC_601)
        {
            eColorimetry = BCFC_Colorimetry_eXvYcc601;
        }
    }
    else if (eMatrixCoeffs <= BAVC_MatrixCoefficients_eHdmi_Full_Range_YCbCr)
    {
        eColorimetry = s_aAvcMatrixCoeffs_to_Colorimetry[eMatrixCoeffs];
    }
    else
    {
        BDBG_WRN(("unsupported colorimetry %d, assumed BT709", eMatrixCoeffs));
        eColorimetry = BCFC_Colorimetry_eBt709;
    }

    if(bXvYcc)
    {
        if (eColorimetry == BCFC_Colorimetry_eBt709)
        {
            eColorimetry = BCFC_Colorimetry_eXvYcc709;
        }
        else if(eColorimetry == BCFC_Colorimetry_eSmpte170M)
        {
            eColorimetry = BCFC_Colorimetry_eXvYcc601;
        }
        else
        {
            BDBG_WRN(("bXvYacc = true with non XvYcc capable MatrixCoeffs"));
        }
    }

    return eColorimetry;
}

/* --------------------------------------------------------------------
 * TF (Transfer Function) related
 */

static const uint8_t s_CFC_NL2L_Tbl[] =
{
    BCFC_NL2L_1886,                 /* BCFC_ColorTF_eBt1886 */
    BCFC_NL2L_PQ,                   /* BCFC_ColorTF_eBt2100Pq */
    BCFC_NL2L_1886,                 /* BCFC_ColorTF_eBtHlg */
    BCFC_NL2L_1886                  /* BCFC_ColorTF_Max */
};

static const uint8_t s_CFC_L2NL_Tbl[] =
{
    BCFC_L2NL_1886,                 /* BCFC_ColorTF_eBt1886 */
    BCFC_L2NL_PQ,                   /* BCFC_ColorTF_eBt2100Pq */
    BCFC_L2NL_1886,                 /* BCFC_ColorTF_eHlg */
    BCFC_L2NL_1886                  /* BCFC_ColorTF_eMax */
};

#if 0 /* replaced by s_LRangeAdj_V0_1886_to_2084 */
/* 1886 - > 2084 */
static const BCFC_LRangeAdjTable s_LRangeAdj_1886_to_2084 = BCFC_MAKE_LR_ADJ
    (  3, /* number of points */
       /*     x,        y,            m,  e */
       0.000000, 0.000000, 0.9600000000, -5,
       1.000000, 0.030000, 0.5000000000,  1,
       1.000000, 1.000000, 0.5000000000,  1,
       1.000000, 1.000000, 0.5000000000,  1,
       1.000000, 1.000000, 0.5000000000,  1,
       1.000000, 1.000000, 0.5000000000,  1,
       1.000000, 1.000000, 0.5000000000,  1,
       1.000000, 1.000000, 0.5000000000,  1 );
#endif
/* identity */
static const BCFC_LRangeAdjTable s_LRangeAdj_Identity = BCFC_MAKE_LR_ADJ
    (  2, /* number of points */
       /*     x,        y,            m,  e */
       0.000000, 0.000000, 0.5000000000,  1,
       1.000000, 1.000000, 0.5000000000,  1,
       1.000000, 1.000000, 0.5000000000,  1,
       1.000000, 1.000000, 0.5000000000,  1,
       1.000000, 1.000000, 0.5000000000,  1,
       1.000000, 1.000000, 0.5000000000,  1,
       1.000000, 1.000000, 0.5000000000,  1,
       1.000000, 1.000000, 0.5000000000,  1 );

const BCFC_LRangeAdjTable *BCFC_GetLRangeAdjIdentity_isrsafe(void)
{
    return &s_LRangeAdj_Identity;
}

#if (BCFC_VERSION >= BCFC_VER_2) /* #if (BVDC_P_CMP_CFC_VER >= BVDC_P_CFC_VER_2) */

/* this table is copied from auto-generated v2/bvdc_cfcramlut_v0_sdr_to_hdr10.c */
static const BCFC_LRangeAdjTable s_LRangeAdj_V0_1886_to_2084 = BCFC_MAKE_LR_ADJ
(   8, /* number of PWL points */
    /*     x         y         m     e              (U0.15)x,   (U0.15)y,   (S0.15)m,   (S4)e   */
    0.000000, 0.000000, 0.639648,   -5,   /* PWL 0, 0x00000000, 0x00000000, 0x000051e0, 0xfffffffb */
    1.000000, 0.019989, 0.000000,    0,   /* PWL 1, 0x00008000, 0x0000028f, 0x00000000, 0x00000000 */
    1.000000, 0.019989, 0.000000,    0,   /* PWL 2, 0x00008000, 0x0000028f, 0x00000000, 0x00000000 */
    1.000000, 0.019989, 0.000000,    0,   /* PWL 3, 0x00008000, 0x0000028f, 0x00000000, 0x00000000 */
    1.000000, 0.019989, 0.000000,    0,   /* PWL 4, 0x00008000, 0x0000028f, 0x00000000, 0x00000000 */
    1.000000, 0.019989, 0.000000,    0,   /* PWL 5, 0x00008000, 0x0000028f, 0x00000000, 0x00000000 */
    1.000000, 0.019989, 0.000000,    0,   /* PWL 6, 0x00008000, 0x0000028f, 0x00000000, 0x00000000 */
    1.000000, 0.019989, 0.000000,    0    /* PWL 7, 0x00008000, 0x0000028f, 0x00000000, 0x00000000 */
);

/* this table is copied from auto-generated v2/bvdc_cfcramlut_v1_sdr_to_hlg_v10.c */
static const BCFC_LRangeAdjTable s_LRangeAdj_PIP_1886_to_hlg_v10 = BCFC_MAKE_LR_ADJ
(   8, /* number of PWL points */
    /*     x         y         m     e              (U0.15)x,   (U0.15)y,   (S0.15)m,   (S4)e   */
    0.000000, 0.000000, 0.758606,    0,   /* PWL 0, 0x00000000, 0x00000000, 0x0000611a, 0x00000000 */
    0.200012, 0.151733, 0.732910,    0,   /* PWL 1, 0x0000199a, 0x0000136c, 0x00005dd0, 0x00000000 */
    0.299988, 0.225006, 0.590790,    0,   /* PWL 2, 0x00002666, 0x00001ccd, 0x00004b9f, 0x00000000 */
    0.399994, 0.284088, 0.974060,   -1,   /* PWL 3, 0x00003333, 0x0000245d, 0x00007cae, 0xffffffff */
    0.500000, 0.332794, 0.807739,   -1,   /* PWL 4, 0x00004000, 0x00002a99, 0x00006764, 0xffffffff */
    0.649994, 0.393372, 0.675476,   -1,   /* PWL 5, 0x00005333, 0x0000325a, 0x00005676, 0xffffffff */
    0.799988, 0.444031, 0.571259,   -1,   /* PWL 6, 0x00006666, 0x000038d6, 0x0000491f, 0xffffffff */
    1.000000, 0.501160, 0.000000,    0    /* PWL 7, 0x00008000, 0x00004026, 0x00000000, 0x00000000 */
);

/* this table is copied from auto-generated v3/bvdc_cfcramlut_vfc_hdr10_to_sdr.c */
static const BCFC_LRangeAdjTable s_LRangeAdj_VFC_2084_to_1886 = BCFC_MAKE_LR_ADJ
(   8, /* number of PWL points */
    /*     x         y         m     e              (U0.15)x,   (U0.15)y,   (S0.15)m,   (S4)e   */
    0.000000, 0.000000, 0.565857,    6,   /* PWL 0, 0x00000000, 0x00000000, 0x0000486e, 0x00000006 */
    0.006989, 0.253082, 0.718597,    5,   /* PWL 1, 0x000000e5, 0x00002065, 0x00005bfb, 0x00000005 */
    0.012695, 0.384308, 0.927673,    4,   /* PWL 2, 0x000001a0, 0x00003131, 0x000076be, 0x00000004 */
    0.022003, 0.522461, 0.609619,    4,   /* PWL 3, 0x000002d1, 0x000042e0, 0x00004e08, 0x00000004 */
    0.037506, 0.673676, 0.811951,    3,   /* PWL 4, 0x000004cd, 0x0000563b, 0x000067ee, 0x00000003 */
    0.062012, 0.832855, 0.549896,    3,   /* PWL 5, 0x000007f0, 0x00006a9b, 0x00004663, 0x00000003 */
    0.100006, 1.000000, 0.000000,    0,   /* PWL 6, 0x00000ccd, 0x00008000, 0x00000000, 0x00000000 */
    1.000000, 1.000000, 0.000000,    0    /* PWL 7, 0x00008000, 0x00008000, 0x00000000, 0x00000000 */
);

/* this table is copied from auto-generated cfclut_ver2/bvdc_cfcramlut_v1_hdr10_to_hlg_v10.c */
static const BCFC_LRangeAdjTable s_LRangeAdj_PIP_2084_to_hlg_v10 = BCFC_MAKE_LR_ADJ
(   8, /* number of PWL points */
    /*     x         y         m     e              (U0.15)x,   (U0.15)y,   (S0.15)m,   (S4)e   */
    0.000000, 0.000000, 0.584351,    6,   /* PWL 0, 0x00000000, 0x00000000, 0x00004acc, 0x00000006 */
    0.005066, 0.189453, 0.576569,    5,   /* PWL 1, 0x000000a6, 0x00001840, 0x000049cd, 0x00000005 */
    0.024994, 0.557129, 0.582153,    4,   /* PWL 2, 0x00000333, 0x00004750, 0x00004a84, 0x00000004 */
    0.037506, 0.673676, 0.891815,    3,   /* PWL 3, 0x000004cd, 0x0000563b, 0x00007227, 0x00000003 */
    0.049988, 0.762726, 0.727142,    3,   /* PWL 4, 0x00000666, 0x000061a1, 0x00005d13, 0x00000003 */
    0.062500, 0.835510, 0.581207,    3,   /* PWL 5, 0x00000800, 0x00006af2, 0x00004a65, 0x00000003 */
    0.087494, 0.951721, 0.964630,    2,   /* PWL 6, 0x00000b33, 0x000079d2, 0x00007b79, 0x00000002 */
    0.100006, 1.000000, 0.000000,    0    /* PWL 7, 0x00000ccd, 0x00008000, 0x00000000, 0x00000000 */
);

/* this table is copied from auto-generated cfclut_ver3/bvdc_cfcramlut_gfd_sdr_to_hdr10.c */
static const BCFC_LRangeAdjTable s_LRangeAdj_GFD_1886_to_2084 = BCFC_MAKE_LR_ADJ
(   8, /* number of PWL points */
    /*     x         y         m     e              (U0.15)x,   (U0.15)y,   (S0.15)m,   (S4)e   */
    0.000000, 0.000000, 0.639648,   -5,   /* PWL 0, 0x00000000, 0x00000000, 0x000051e0, 0xfffffffb */
    1.000000, 0.019989, 0.000000,    0,   /* PWL 1, 0x00008000, 0x0000028f, 0x00000000, 0x00000000 */
    1.000000, 0.019989, 0.000000,    0,   /* PWL 2, 0x00008000, 0x0000028f, 0x00000000, 0x00000000 */
    1.000000, 0.019989, 0.000000,    0,   /* PWL 3, 0x00008000, 0x0000028f, 0x00000000, 0x00000000 */
    1.000000, 0.019989, 0.000000,    0,   /* PWL 4, 0x00008000, 0x0000028f, 0x00000000, 0x00000000 */
    1.000000, 0.019989, 0.000000,    0,   /* PWL 5, 0x00008000, 0x0000028f, 0x00000000, 0x00000000 */
    1.000000, 0.019989, 0.000000,    0,   /* PWL 6, 0x00008000, 0x0000028f, 0x00000000, 0x00000000 */
    1.000000, 0.019989, 0.000000,    0    /* PWL 7, 0x00008000, 0x0000028f, 0x00000000, 0x00000000 */
);

static const BCFC_LRangeAdjTable * const s_aaLRangeAdj_Tbl[][3] =
{
    /* input BVDC_P_ColorTF_eBt1886 */
    {
        &s_LRangeAdj_Identity,            /* output BVDC_P_ColorTF_eBt1886 */
        &s_LRangeAdj_V0_1886_to_2084,     /* output BVDC_P_ColorTF_eBt2100Pq */
        &s_LRangeAdj_PIP_1886_to_hlg_v10  /* output BVDC_P_ColorTF_eHlg */
    },

    /* input BVDC_P_ColorTF_eBt2100Pq */
    {
        &s_LRangeAdj_VFC_2084_to_1886,    /* output BVDC_P_ColorTF_eBt1886 */
        &s_LRangeAdj_Identity,            /* output BVDC_P_ColorTF_eBt2100Pq */
        &s_LRangeAdj_PIP_2084_to_hlg_v10  /* output BVDC_P_ColorTF_eHlg */
    },

    /* input BVDC_P_ColorTF_eHlg */
    {
        &s_LRangeAdj_Identity,            /* output BVDC_P_ColorTF_eBt1886 */
        &s_LRangeAdj_V0_1886_to_2084,     /* output BVDC_P_ColorTF_eBt2100Pq */
        &s_LRangeAdj_Identity             /* output BVDC_P_ColorTF_eHlg */
    }
};

#endif /* #if (BCFC_VERSION >= BCFC_VER_2) *//* #if (BVDC_P_CMP_CFC_VER >= BVDC_P_CFC_VER_2) */

static const BCFC_TfConvRamLuts s_TfConvRamLutsBypass =
{
    NULL,  NULL,  NULL,  &s_LRangeAdj_Identity, NULL
};

/*
 *
 */
const BCFC_TfConvRamLuts * BCFC_GetTfConvRamLutsBypass_isrsafe(void)
{
    return &s_TfConvRamLutsBypass;
}

/* --------------------------------------------------------------------
 * CFC basic implementations
 */

/* generic cfc color space init
 */
void BCFC_InitCfcColorSpace(
    BCFC_ColorSpace          *pColorSpace )
{
    uint32_t ii;

    /* init as NTSC */
    pColorSpace->eColorFmt = BCFC_ColorFormat_eYCbCr;
    pColorSpace->eColorimetry = BCFC_Colorimetry_eSmpte170M;
    pColorSpace->eColorTF = BCFC_ColorTF_eBt1886;
    pColorSpace->eColorDepth = BCFC_ColorDepth_e8Bit;
    pColorSpace->eColorRange = BCFC_ColorRange_eLimited;

    /* Content Light SEI message */
    pColorSpace->stHdrParm.ulAvgContentLight = 0;
    pColorSpace->stHdrParm.ulMaxContentLight = 0;

    /* Mastering Display Colour Volume */
    for (ii=0; ii<3; ii++)
    {
        pColorSpace->stHdrParm.stDisplayPrimaries[ii].ulX = 0;
        pColorSpace->stHdrParm.stDisplayPrimaries[ii].ulY = 0;
    }
    pColorSpace->stHdrParm.stWhitePoint.ulX = 0;
    pColorSpace->stHdrParm.stWhitePoint.ulY = 0;

    pColorSpace->stHdrParm.ulMaxDispMasteringLuma = 0;
    pColorSpace->stHdrParm.ulMinDispMasteringLuma = 0;

    pColorSpace->pMetaData = NULL;
}

/* generic cfc color space copy
 */
void BCFC_CopyColorSpace_isrsafe(
    BCFC_ColorSpace          *pDstColorSpace,
    BCFC_ColorSpace          *pSrcColorSpace )
{
    void *pMetaDataSave = pDstColorSpace->pMetaData;
    BKNI_Memcpy_isr(pDstColorSpace, pSrcColorSpace, sizeof(BCFC_ColorSpace));
    pDstColorSpace->pMetaData = pMetaDataSave;
}

/* generic cfc init
 */
void BCFC_InitCfc_isrsafe(
    BCFC_Context          *pCfc )
{
    /* mark as not used yet */
    pCfc->stColorSpaceExtIn.stCfg.stBits.bDirty = BCFC_P_CLEAN;
    pCfc->stColorSpaceExtIn.stColorSpace.eColorFmt = BCFC_ColorFormat_eInvalid;
    pCfc->stColorSpaceExtIn.stColorSpace.pMetaData = NULL;
    pCfc->bBypassCfc = true;
    pCfc->ucSelBlackBoxNL = BCFC_NL_SEL_BYPASS;
    pCfc->pMa = &s_Csc3x4Identity;
    pCfc->stMb = s_Csc3x3Identity;
    pCfc->stMc = s_Csc3x4Identity;

#if (BCFC_VERSION >= BCFC_VER_2) /* #if (BVDC_P_CMP_CFC_VER >= BVDC_P_CFC_VER_2) */
    pCfc->stLRangeAdj.pTable = &s_LRangeAdj_Identity;
#if (BCFC_VERSION >= BCFC_VER_3) /* #if (BVDC_P_CMP_CFC_VER >= BVDC_P_CFC_VER_3) */
    pCfc->pTfConvRamLuts = &s_TfConvRamLutsBypass;
    pCfc->ucRamNL2LRulBuildCntr = 0;
    pCfc->ucRamL2NLRulBuildCntr = 0;
    pCfc->ucRamLMRRulBuildCntr  = 0;
#endif
#endif
}

/* Generic configure of a CFC according to its input and output color space
 *
 * return true if further ramLut configuring is needed
 *
 * CFC capabilites:
 *
 * 7439 B0:  cmp0_V0  6 bMa && bBlackBoxNLConv                   && bMc
 *           cmp0_V1  6 bMa && bBlackBoxNLConv                   && bMc
 *
 *           GF0      1 bMa && bBlackBoxNLConv                   && bMc
 *
 *           DVI_CSC  1                                             bMc   (CL Output -> CL2020_CONTROL.CTRL = 1)

 *
 *           cmp1_V0  6        bBlackBoxNLConv                   && bMc   (SelNLConv and addr match with CMP0)
 *           cmp1_V1  6        bBlackBoxNLConv                   && bMc
 *
 *           (MA and CL2020_CONTROL_CTRL.ENABLE for CL display)
 *
 * 7271 A0:  cmp0_V0  6 bMa && bNL2L && bMb && bLRngAdj && bL2Nl && bMc && bRamNL2L && bRamL2NL (CL input -> Ma=Identity/disabled, SEL_CL_IN=1)
 *           cmp0_V1  6 bMa && bNL2L && bMb && bLRngAdj && bL2Nl && bMc                         (CL input -> Ma=Identity/disabled, SEL_CL_IN=1)
 *
 *           GF0      1 bMa && bNL2L && bMb && bLRngAdj && bL2Nl && bMc && bRamNL2L && bRamL2NL (CL input -> Ma=Identity/disabled, SEL_CL_IN=1)
 *
 *           DVI_CSC  1                                             bMc                         (CL Output -> CL2020_CONTROL.CTRL = 1)
 *
 *           cmp1_V0  6        bBlackBoxNLConv                   && bMc                         (Mc addr matches with CMP0)
 *           cmp1_V1  6        bBlackBoxNLConv                   && bMc
 *
 *           (MA and CL2020_CONTROL_CTRL.ENABLE for CL display)
 *           1). where is Ma / Mc selection ?  CMP_0_V0_RECT_CSC_INDEX_0 ?
 *
 * 7271 B0:  cmp0_V0  8/4 bDbvCmp && bTpToneMapping && bMa && bNL2L && bMb && bLMR && bLRngAdj && bL2Nl && bMc && bDbvToneMapping && bRamNL2L && bRamL2NL && bRamLutScb
 *                        (CL input -> Ma5x4, SEL_CL_IN=1)
 *           cmp0_V1  8/4                              bMa && bNL2L && bMb && bLMR && bLRngAdj && bL2Nl && bMc                    && bRamNL2L && bRamL2NL && bRamLutScb
 *                        (CL input -> Ma5x4, SEL_CL_IN=1)
 *
 *           GF0      1                                bMa && bNL2L && bMb && bLMR && bLRngAdj && bL2Nl && bMc && bDbvToneMapping && bRamNL2L && bRamL2NL && bRamLutScb
 *                        (CL input -> Ma5x4, SEL_CL_IN=1)
 *
 *           DVI_CSC  1                                bMa && bNL2L && bMb         && bLRngAdj && bL2Nl && bMc                    && bRamNL2L && bRamL2NL && bRamLutScb
 *                        (CL output -> Mc5x4, SEL_CL_OUT=1)
 *
 *           cmp1_V0  6        bBlackBoxNLConv                                                          && bMc (Mc addr match CMP0)
 *           cmp1_V1  no V1
 *
 *           VFC      1                                bMa && bNL2L && bMb         && bLRngAdj && bL2Nl && bMc                    && bRamNL2L && bRamL2NL
 *                        (CL input -> Ma5x4, SEL_CL_IN=1)
 *
 * CFC RAM LUT size:
 *
 *          NL2L               LMR             L2NL         CVM        DLBV_CMP       TP
 *
 * 7271 a0:
 *      v0   1201                               276
 *      v1
 *    gfd0    304                                72
 *     dvi
 *
 * 7271 b0:
 *      v0   1201               256             400         4*513      yes            4*65
 *      v1   1201               256             400
 *    gfd0    513               256              72         4*513
 *     vfc    513                                72
 *     dvi   1201                 0             400
 *
 * YCbCr and YCbCr_CL in CMP (both in and out) are limted range, all RGB in CMP (both in, out and middle) are full range
 */
bool BCFC_UpdateCfg_isr
    ( BCFC_Context        *pCfc,
      bool                 bMosaicMode,
      bool                 bTchInput,
      bool                 bForceDirty)
{
    bool bSupportTfConv = pCfc->stCapability.stBits.bLRngAdj;
    BCFC_ColorSpaceExt *pColorSpaceExtIn = &(pCfc->stColorSpaceExtIn);
    BCFC_ColorSpaceExt *pColorSpaceExtOut = pCfc->pColorSpaceExtOut;
    BCFC_ColorSpace *pColorSpaceIn = &(pColorSpaceExtIn->stColorSpace);
    BCFC_ColorSpace *pColorSpaceOut = &(pColorSpaceExtOut->stColorSpace);
    BCFC_Csc3x4  stTmp3x4;
    BCFC_Csc3x4  *pTmp3x4 = &stTmp3x4;
    bool bDone = false;
    bool bForceNotBypass;
    bool bRamLutCfgDirty = false;

    /* check for GFD and DVI_CSC: CL input with SDR/HDR, CL display with SDR/HDR ??? */

    /* update config decided by input color space */
    if (pColorSpaceExtIn->stCfg.stBits.bDirty)
    {
        pColorSpaceExtIn->stCfg.stBits.SelTF = s_CFC_NL2L_Tbl[pColorSpaceIn->eColorTF];
        pColorSpaceExtIn->stCfg.stBits.bSelXvYcc = (BCFC_Colorimetry_eXvYcc601 == pColorSpaceIn->eColorimetry ||
                                                 BCFC_Colorimetry_eXvYcc709 == pColorSpaceIn->eColorimetry)? 1 : 0;
        pColorSpaceExtIn->stCfg.stBits.bSelCL = (pColorSpaceIn->eColorFmt == BCFC_ColorFormat_eYCbCr_CL)? 1 : 0;
        pColorSpaceExtIn->stCfg.stBits.bEnTpToneMap =
            bTchInput && (!pCfc->stForceCfg.stBits.bDisableTch) &&
            (BCFC_IS_SDR_TO_HDR10(pColorSpaceIn->eColorTF, pColorSpaceOut->eColorTF) ||
             BCFC_IS_HDR10_TO_SDR(pColorSpaceIn->eColorTF, pColorSpaceOut->eColorTF));
        if (BCFC_ColorFormat_eYCbCr_CL == pColorSpaceIn->eColorFmt)
        {
            pColorSpaceExtIn->stM3x4 = s_BT2020CL_YCbCr_to_RYB_Bt1886_Negative;
            pColorSpaceExtIn->stMalt = s_BT2020CL_YCbCr_to_RYB_Bt1886_Positive;
            BCFC_Csc_Mult_isrsafe(&(s_aCFC_MB_IN_Tbl[pColorSpaceIn->eColorimetry]->m[0][0]), 3, &(s_BT2020NCL_RYB_to_RGB.m[0][0]), 3, &(pColorSpaceExtIn->stMb.m[0][0]));
        }
        else
        {
            pColorSpaceExtIn->stM3x4 = (BCFC_ColorFormat_eRGB == pColorSpaceIn->eColorFmt)?
                s_Csc3x4Identity : *(s_aCFC_MA_Tbl[pColorSpaceIn->eColorimetry]);
            pColorSpaceExtIn->stMb = *(s_aCFC_MB_IN_Tbl[pColorSpaceIn->eColorimetry]);
        }

        /* all pre-generated matrices assume YCbCr limted range and RGB full range */
        if ((pColorSpaceIn->eColorRange == BCFC_ColorRange_eFull) && (pColorSpaceIn->eColorFmt == BCFC_ColorFormat_eYCbCr))
        {
            /* Tmp = Ma * s_CscYCbCr_Full_to_Limited */
            BCFC_Csc_Mult_isrsafe(&(pColorSpaceExtIn->stM3x4.m[0][0]), 4, &(s_CscYCbCr_Full_to_Limited.m[0][0]), 4, &(pTmp3x4->m[0][0]));
            pColorSpaceExtIn->stM3x4 = *pTmp3x4;
            if (BCFC_ColorFormat_eYCbCr_CL == pColorSpaceIn->eColorFmt)
            {
                BCFC_Csc_Mult_isrsafe(&(pColorSpaceExtIn->stMalt.m[0][0]), 4, &(s_CscYCbCr_Full_to_Limited.m[0][0]), 4, &(pTmp3x4->m[0][0]));
                pColorSpaceExtIn->stMalt = *pTmp3x4;
            }
        }
        else if ((pColorSpaceIn->eColorRange == BCFC_ColorRange_eLimited) && (pColorSpaceIn->eColorFmt == BCFC_ColorFormat_eRGB))
        {
            /* Tmp = Ma * s_CscRGB_Limited_to_Full */
            BCFC_Csc_Mult_isrsafe(&(pColorSpaceExtIn->stM3x4.m[0][0]), 4, &(s_CscRGB_Limited_to_Full.m[0][0]), 4, &(pTmp3x4->m[0][0]));
            pColorSpaceExtIn->stM3x4 = *pTmp3x4;
        }

        if(bTchInput && (!pCfc->stForceCfg.stBits.bDisableTch) &&
           (BCFC_IS_SDR_TO_HDR10(pColorSpaceIn->eColorTF, pColorSpaceOut->eColorTF) ||
            BCFC_IS_HDR10_TO_SDR(pColorSpaceIn->eColorTF, pColorSpaceOut->eColorTF)))
        {
            pColorSpaceExtIn->stM3x4 = s_Csc3x4Identity;
        }
    }

    /* update config decided by output color space */
    if (pColorSpaceExtOut->stCfg.stBits.bDirty)
    {
        pColorSpaceExtOut->stCfg.stBits.SelTF = s_CFC_L2NL_Tbl[pColorSpaceOut->eColorTF];
        pColorSpaceExtOut->stCfg.stBits.bSelXvYcc = (BCFC_Colorimetry_eXvYcc601 == pColorSpaceOut->eColorimetry ||
                                                  BCFC_Colorimetry_eXvYcc709 == pColorSpaceOut->eColorimetry)? 1 : 0;
        pColorSpaceExtOut->stCfg.stBits.bSelCL = (pColorSpaceOut->eColorFmt == BCFC_ColorFormat_eYCbCr_CL)? 1 : 0;
        if (BCFC_ColorFormat_eYCbCr_CL == pColorSpaceOut->eColorFmt)
        {
            /* #if BVDC_P_CFC_SWAP_NEG_POS_MC , else swap pos and neg matrix */
            pColorSpaceExtOut->stM3x4 = s_BT2020CL_RYB_to_YCbCr_Bt1886_Positive;
            pColorSpaceExtOut->stMalt = s_BT2020CL_RYB_to_YCbCr_Bt1886_Negative;
        }
        else
        {
            pColorSpaceExtOut->stM3x4 = (BCFC_ColorFormat_eRGB == pColorSpaceOut->eColorFmt)?
                s_Csc3x4Identity :   /* must be dvi_csc */
                *(s_aCFC_MC_Tbl[pColorSpaceOut->eColorimetry]);   /* could be gfd, cmp, or dvi_csc */
        }
        pColorSpaceExtOut->stMb = *(s_aCFC_MB_OUT_Tbl[pColorSpaceOut->eColorimetry]);

        /* all pre-generated matrices assume YCbCr limted range and RGB full range */
        if ((pColorSpaceOut->eColorRange == BCFC_ColorRange_eFull) && (pColorSpaceOut->eColorFmt == BCFC_ColorFormat_eYCbCr))
        {
            /* Tmp = s_CscYCbCr_Limited_to_Full * Mc */
            BCFC_Csc_Mult_isrsafe(&(s_CscYCbCr_Limited_to_Full.m[0][0]), 4, &(pColorSpaceExtOut->stM3x4.m[0][0]), 4, &(pTmp3x4->m[0][0]));
            pColorSpaceExtOut->stM3x4 = *pTmp3x4;
            if (BCFC_ColorFormat_eYCbCr_CL == pColorSpaceOut->eColorFmt)
            {
                BCFC_Csc_Mult_isrsafe(&(pColorSpaceExtOut->stMalt.m[0][0]), 4, &(s_CscYCbCr_Limited_to_Full.m[0][0]), 4, &(pTmp3x4->m[0][0]));
                pColorSpaceExtOut->stMalt = *pTmp3x4;
            }
        }
        else if ((pColorSpaceOut->eColorRange == BCFC_ColorRange_eLimited) && (pColorSpaceOut->eColorFmt == BCFC_ColorFormat_eRGB))
        {
            /* Tmp = Mc * s_CscRGB_Full_to_Limited */
            BCFC_Csc_Mult_isrsafe(&(s_CscRGB_Full_to_Limited.m[0][0]), 4, &(pColorSpaceExtOut->stM3x4.m[0][0]), 4, &(pTmp3x4->m[0][0]));
            pColorSpaceExtOut->stM3x4 = *pTmp3x4;
        }
    }

    /* pCfc->bDirty was set to true when display colorSpace changes */
    if ((pColorSpaceExtIn->stCfg.stBits.bDirty) || (pColorSpaceExtOut->stCfg.stBits.bDirty) || (bForceDirty))
    {
        if (!BCFC_IN_VFC(pCfc->eId)) /* VFC in mosaic mode print too much */
        {
#if (BDBG_DEBUG_BUILD)
            BDBG_MODULE_MSG(BCFC_1,("    %s-Cfc%d colorSpace in -> out:", BCFC_GetCfcName_isrsafe(pCfc->eId), pCfc->ucMosaicSlotIdx));
            BDBG_MODULE_MSG(BCFC_1,("       ColorFmt    %9s -> %-9s", BCFC_GetColorFormatName_isrsafe(pColorSpaceIn->eColorFmt),    BCFC_GetColorFormatName_isrsafe(pColorSpaceOut->eColorFmt)));
            BDBG_MODULE_MSG(BCFC_1,("       Colorimetry %9s -> %-9s", BCFC_GetColorimetryName_isrsafe(pColorSpaceIn->eColorimetry), BCFC_GetColorimetryName_isrsafe(pColorSpaceOut->eColorimetry)));
            BDBG_MODULE_MSG(BCFC_1,("       ColorRange  %9s -> %-9s", BCFC_GetColorRangeName_isrsafe(pColorSpaceIn->eColorRange),   BCFC_GetColorRangeName_isrsafe(pColorSpaceOut->eColorRange)));
            BDBG_MODULE_MSG(BCFC_1,("       ColorTF     %9s -> %-9s", BCFC_GetColorTfName_isrsafe(pColorSpaceIn->eColorTF),         BCFC_GetColorTfName_isrsafe(pColorSpaceOut->eColorTF)));
            BDBG_MODULE_MSG(BCFC_1,("       ColorDepth  %9s -> %-9s", BCFC_GetColorDepthName_isrsafe(pColorSpaceIn->eColorDepth),   BCFC_GetColorDepthName_isrsafe(pColorSpaceOut->eColorDepth)));
#endif
            BDBG_MODULE_MSG(BCFC_4,("       inDirtyIn %d, OutDirty %d || %d", pColorSpaceExtIn->stCfg.stBits.bDirty, bForceDirty, pColorSpaceExtOut->stCfg.stBits.bDirty));
        }
        else
        {
#if (BDBG_DEBUG_BUILD)
            BDBG_MODULE_MSG(BCFC_VFC_1,("%s-Cfc%d colorSpace in -> out:", BCFC_GetCfcName_isrsafe(pCfc->eId), pCfc->ucMosaicSlotIdx));
            BDBG_MODULE_MSG(BCFC_VFC_1,("   ColorFmt    %9s -> %-9s", BCFC_GetColorFormatName_isrsafe(pColorSpaceIn->eColorFmt),    BCFC_GetColorFormatName_isrsafe(pColorSpaceOut->eColorFmt)));
            BDBG_MODULE_MSG(BCFC_VFC_1,("   Colorimetry %9s -> %-9s", BCFC_GetColorimetryName_isrsafe(pColorSpaceIn->eColorimetry), BCFC_GetColorimetryName_isrsafe(pColorSpaceOut->eColorimetry)));
            BDBG_MODULE_MSG(BCFC_VFC_1,("   ColorRange  %9s -> %-9s", BCFC_GetColorRangeName_isrsafe(pColorSpaceIn->eColorRange),   BCFC_GetColorRangeName_isrsafe(pColorSpaceOut->eColorRange)));
            BDBG_MODULE_MSG(BCFC_VFC_1,("   ColorTF     %9s -> %-9s", BCFC_GetColorTfName_isrsafe(pColorSpaceIn->eColorTF),         BCFC_GetColorTfName_isrsafe(pColorSpaceOut->eColorTF)));
            BDBG_MODULE_MSG(BCFC_VFC_1,("   ColorDepth  %9s -> %-9s", BCFC_GetColorDepthName_isrsafe(pColorSpaceIn->eColorDepth),   BCFC_GetColorDepthName_isrsafe(pColorSpaceOut->eColorDepth)));
#endif
            BDBG_MODULE_MSG(BCFC_VFC_4,("   inDirtyIn %d, OutDirty %d || %d", pColorSpaceExtIn->stCfg.stBits.bDirty, bForceDirty, pColorSpaceExtOut->stCfg.stBits.bDirty));
        }

        /* note: for 7271 B0 and up, all mosaic CFCs share the same HDR_CMP_0_CMP_HDR_V0_CTRL.SEL_L2NL, therefore in mosaic mode
         * we cannot allow any mosaic CFC to bypass, otherwise it will conflict with other mosaic CFCs.
         */
      #if (BCFC_VERSION >= BCFC_VER_3) /* #if (BVDC_P_CMP_CFC_VER >= BVDC_P_CFC_VER_3) */
        bForceNotBypass = bMosaicMode;
        if(bTchInput && (!pCfc->stForceCfg.stBits.bDisableTch) &&
           (BCFC_IS_SDR_TO_HDR10(pColorSpaceIn->eColorTF, pColorSpaceOut->eColorTF) ||
            BCFC_IS_HDR10_TO_SDR(pColorSpaceIn->eColorTF, pColorSpaceOut->eColorTF)))
        {
            bForceNotBypass = true; /* TODO: need to check for TCH mode */
        }
      #else
        bForceNotBypass = false;
        BSTD_UNUSED(bMosaicMode);
      #endif /* #if (BCFC_VERSION >= BCFC_VER_3) */
        if ((!bForceNotBypass) &&
            ((!BCFC_COLOR_SPACE_DIFF(pColorSpaceIn, pColorSpaceOut)) ||
             (BCFC_NEED_TF_CONV(pColorSpaceIn->eColorTF, pColorSpaceOut->eColorTF) && !bSupportTfConv && !BCFC_IN_GFD(pCfc->eId))))
        {
            pCfc->bBypassCfc = true;
            pCfc->ucSelBlackBoxNL = BCFC_NL_SEL_BYPASS;
            pCfc->pMa = &s_Csc3x4Identity;
            pCfc->stMb = s_Csc3x3Identity;
            pCfc->stMc = s_Csc3x4Identity;
          #if (BCFC_VERSION >= BCFC_VER_2) /* #if (BVDC_P_CMP_CFC_VER >= BVDC_P_CFC_VER_2) */
            pCfc->stLRangeAdj.pTable = &s_LRangeAdj_Identity;
          #if (BCFC_VERSION >= BCFC_VER_3) /* #if (BVDC_P_CMP_CFC_VER >= BVDC_P_CFC_VER_3) */
            pCfc->pTfConvRamLuts = &s_TfConvRamLutsBypass;
          #endif
          #endif
            bDone = true;
        }
        else
        {
            pCfc->bBypassCfc = false;
            pCfc->ucSelBlackBoxNL = BCFC_NL_SEL_BYPASS;

          #if (defined(BCHP_CMP_1_V0_NL_CSC_CTRL) || defined(BCHP_GFD_0_NL_CSC_CTRL_SEL_CONV_R0_709_RGB_2_2020_RGB)) && !defined(BCHP_CMP_0_V0_NL_CSC_CTRL)
            BDBG_CASSERT(false);
          #endif

          #if (BCFC_VERSION >= BCFC_VER_1) /* #if (BVDC_P_CMP_CFC_VER >= BVDC_P_CFC_VER_1) */
          #if BCHP_CMP_0_V0_NL_CSC_CTRL
            /* BT709 CL input / output ??? */
            if (pCfc->stCapability.stBits.bBlackBoxNLConv &&
                (pCfc->stCapability.stBits.bMa || /* without Ma, input colorimetry / format must match BlackBoxNLConv's needs */
                 ((pColorSpaceIn->eColorimetry == BCFC_Colorimetry_eBt2020) &&
                  (pColorSpaceIn->eColorFmt != BCFC_ColorFormat_eRGB) &&
                  (pColorSpaceIn->eColorRange == BCFC_ColorRange_eLimited)) ||
                 ((pColorSpaceIn->eColorimetry != BCFC_Colorimetry_eBt2020) &&
                  (pColorSpaceIn->eColorFmt == BCFC_ColorFormat_eRGB) &&
                  (pColorSpaceIn->eColorRange == BCFC_ColorRange_eFull))))
            {
                /* DVI_CSC does not have bBlackBoxNLConv, will not be here,
                 * and cmp always output limted range NCL YCbCr */
                if (BCFC_Colorimetry_eBt2020 == pColorSpaceIn->eColorimetry)
                {
                    bDone = true;

                    /* decide Ma and ucSelBlackBoxNL */
                    if (BCFC_ColorFormat_eYCbCr_CL == pColorSpaceIn->eColorFmt)
                    {
                        pCfc->pMa = (pColorSpaceIn->eColorRange == BCFC_ColorRange_eFull)?
                            &(s_CscYCbCr_Full_to_Limited) : &(s_Csc3x4Identity);
                        pCfc->ucSelBlackBoxNL = (BCFC_Colorimetry_eBt2020 == pColorSpaceOut->eColorimetry)?
                            BCHP_CMP_0_V0_NL_CSC_CTRL_SEL_CONV_R0_CL_YCbCr_2_2020_RGB : BCHP_CMP_0_V0_NL_CSC_CTRL_SEL_CONV_R0_CL_YCbCr_2_709_RGB;
                    }
                    else if (BCFC_Colorimetry_eBt2020 == pColorSpaceOut->eColorimetry)
                    {
                        pCfc->ucSelBlackBoxNL = BCFC_NL_SEL_BYPASS;
                        pCfc->pMa = &(s_Csc3x4Identity);
                        bDone = false; /* using Mc only */
                    }
                    else
                    {
                        /* video/gfx win input could be full / limited range YCbCr / RGB */
                        BCFC_Csc_Mult_isrsafe(&(s_BT2020NCL_RGB_to_YCbCr.m[0][0]), 4, &(pColorSpaceExtIn->stM3x4.m[0][0]), 4, &(pColorSpaceExtIn->stMalt.m[0][0]));
                        pCfc->pMa = &(pColorSpaceExtIn->stMalt);
                        pCfc->ucSelBlackBoxNL = BCHP_CMP_0_V0_NL_CSC_CTRL_SEL_CONV_R0_NCL_YCbCr_2_709_RGB;
                    }

                    /* decide Mc */
                    if (bDone)
                    {
                        if (BCFC_Colorimetry_eBt2020 == pColorSpaceOut->eColorimetry)
                        {
                            pCfc->stMc = pColorSpaceExtOut->stM3x4;
                        }
                        else
                        {
                            /* for non-BT2020 output, blackBoxNLConv's output is BT709 full-range RGB */
                            BCFC_Csc_Mult_isrsafe(&(pColorSpaceExtOut->stMb.m[0][0]), 3, &(s_BT709_RGB_to_XYZ.m[0][0]), 3, &(pCfc->stMb.m[0][0]));
                            BCFC_Csc_Mult_isrsafe(&(pColorSpaceExtOut->stM3x4.m[0][0]), 4, &(pCfc->stMb.m[0][0]), 3, &(pCfc->stMc.m[0][0]));
                        }
                    }
                }
                else if ((BCFC_Colorimetry_eBt2020 == pColorSpaceOut->eColorimetry) && /* input is non_bt2020 */
                         (!BCFC_IN_GFD0(pCfc->eId) || !BCFC_IS_HDR10(pColorSpaceOut->eColorTF)))
                {
                    /* if input is not BT2020, then blockBoxNLConv's input should be BT709 full range RGB */
                    BCFC_Csc_Mult_isrsafe(&(s_BT709_XYZ_to_RGB.m[0][0]), 3, &(pColorSpaceExtIn->stMb.m[0][0]), 3, &(pCfc->stMb.m[0][0]));
                    BCFC_Csc_Mult_isrsafe(&(pCfc->stMb.m[0][0]), 3, &(pColorSpaceExtIn->stM3x4.m[0][0]), 4, &(pColorSpaceExtIn->stMalt.m[0][0]));
                    pCfc->pMa = &(pColorSpaceExtIn->stMalt);
                    pCfc->ucSelBlackBoxNL = BCHP_CMP_0_V0_NL_CSC_CTRL_SEL_CONV_R0_709_RGB_2_2020_RGB;
                    pCfc->stMc = pColorSpaceExtOut->stM3x4;
                    bDone = true;
                }
                else
                {
                    pCfc->ucSelBlackBoxNL = BCFC_NL_SEL_BYPASS;
                    pCfc->pMa = &(s_Csc3x4Identity);
                    /* bDone is false --> using Mc only */
                }
            }
            else
          #endif /* #if BCHP_CMP_0_V0_NL_CSC_CTRL */
            if (pCfc->stCapability.stBits.bMb)
            {
                /* note: if pCfc->stCapability.stBits.bMb=true, we should use pColorSpaceExtIn->stM3x4 for Ma and
                 * pColorSpaceExtOut->stM3x4 for Mc directly
                 */
                pCfc->pMa = &pColorSpaceExtIn->stM3x4;
                if (BCFC_ColorFormat_eYCbCr_CL == pColorSpaceOut->eColorFmt)
                {
                    /* must be display cfc */
                    pCfc->stMb = s_BT2020NCL_RGB_to_RYB;
                }
                    else if ((pColorSpaceIn->eColorimetry == pColorSpaceOut->eColorimetry) && (BCFC_ColorFormat_eYCbCr_CL != pColorSpaceIn->eColorFmt))
                {
                    pCfc->stMb = s_Csc3x3Identity;
                }
                else
                {
                    /* B = Bout  * Bin */
                    BCFC_Csc_Mult_isrsafe(&(pColorSpaceExtOut->stMb.m[0][0]), 3, &(pColorSpaceExtIn->stMb.m[0][0]), 3, &(pCfc->stMb.m[0][0]));
                }
                pCfc->stMc = pColorSpaceExtOut->stM3x4;

              #if (BCFC_VERSION >= BCFC_VER_2) /* #if (BVDC_P_CMP_CFC_VER >= BVDC_P_CFC_VER_2) */
                if (bSupportTfConv)
                {
                    if (BCFC_IN_GFD(pCfc->eId) &&
                        BCFC_IS_SDR_TO_HDR10(pColorSpaceIn->eColorTF, pColorSpaceOut->eColorTF))
                    {
                        pCfc->stLRangeAdj.pTable = &s_LRangeAdj_GFD_1886_to_2084;
                    }
                    else
                    {
                        pCfc->stLRangeAdj.pTable = s_aaLRangeAdj_Tbl[pColorSpaceIn->eColorTF][pColorSpaceOut->eColorTF];
                    }

                    /* need to further check ram lut configure for 7271 A0 and up */
                    bRamLutCfgDirty = true;
                }
              #endif

                /* if app wants to force certain sub-modules off */
                if (0 != pCfc->stForceCfg.ulInts)
                {
                    if (pCfc->stForceCfg.stBits.bDisableNl2l)
                    {
                        pColorSpaceExtIn->stCfg.stBits.SelTF = BCFC_NL2L_BYPASS;
                    }
                    if (pCfc->stForceCfg.stBits.bDisableL2nl)
                    {
                        pColorSpaceExtOut->stCfg.stBits.SelTF = BCFC_L2NL_BYPASS;
                    }
                  #if (BCFC_VERSION >= BCFC_VER_2) /* #if (BVDC_P_CMP_CFC_VER >= BVDC_P_CFC_VER_2) */
                    if (pCfc->stForceCfg.stBits.bDisableLRangeAdj)
                    {
                        pCfc->stLRangeAdj.pTable = &s_LRangeAdj_Identity;
                    }
                  #endif
                    if (pCfc->stForceCfg.stBits.bDisableMb)
                    {
                        pCfc->stMb = (BCFC_ColorFormat_eYCbCr_CL == pColorSpaceIn->eColorFmt)? s_BT2020NCL_RYB_to_RGB :  s_Csc3x3Identity;
                    }
                }

                bDone = true;
            }
            /* else bDone is false --> using Mc only */
          #endif /* #if (BCFC_VERSION >= BCFC_VER_1) */
        }

        /* we only have a Mc to mimic */
        if (!bDone)
        {
            if (pCfc->stCapability.stBits.bMa && (BCFC_ColorFormat_eYCbCr_CL == pColorSpaceOut->eColorFmt))
            {
                /* must be 97439 B0 or 97271 A0 DVI-CFC: pCfc->stCapability.stBits.bMa indicates it can output YCbCr_CL
                 * note: it can't output full range YcbCr_CL */
                pCfc->stMc = pColorSpaceExtIn->stM3x4;
            }
            else
            {
                /* B = Bout  * Bin */
                BCFC_Csc_Mult_isrsafe(&(pColorSpaceExtOut->stMb.m[0][0]), 3, &(pColorSpaceExtIn->stMb.m[0][0]), 3, &(pCfc->stMb.m[0][0]));

                if ((pColorSpaceIn->eColorimetry != BCFC_Colorimetry_eBt2020) && (pColorSpaceOut->eColorimetry == BCFC_Colorimetry_eBt2020))
                {
                    /* Mc is used for tmp first */
                    if (BCFC_ColorFormat_eRGB == pColorSpaceIn->eColorFmt)
                    {
                        /* non-BT2020 RGB -> same colorimetry non-BT2020 limited range YCbCr */
                        BCFC_Csc_Mult_isrsafe(&(s_aCFC_MC_Tbl[pColorSpaceIn->eColorimetry]->m[0][0]), 4, &(pColorSpaceExtIn->stM3x4.m[0][0]), 4, &(pCfc->stMc.m[0][0]));
                    }
                    else
                    {
                        /* non-BT2020 YCbCr -> same colorimetry non-BT2020 limited range YCbCr */
                        pCfc->stMc = (pColorSpaceIn->eColorRange == BCFC_ColorRange_eFull)? s_CscYCbCr_Full_to_Limited : s_Csc3x4Identity;
                    }
                    /* reverse 0.85 adj applied to non-BT2020 limited range YCbCr */
                    BCFC_Csc_Mult_isrsafe(&(s_CscCr0p85Adj_NonBT2020_to_BT2020.m[0][0]), 4, &(pCfc->stMc.m[0][0]), 4, &(pTmp3x4->m[0][0]));
                    /* non-BT2020 limited range YCbC -> non-BT2020 full range RGB */
                    BCFC_Csc_Mult_isrsafe(&(s_aCFC_MA_Tbl[pColorSpaceIn->eColorimetry]->m[0][0]), 4, &(pTmp3x4->m[0][0]), 4, &(pCfc->stMc.m[0][0]));
                    /* non-BT2020 full range RGB -> BT2020 full range RGB */
                    BCFC_Csc_Mult_isrsafe(&(pCfc->stMb.m[0][0]), 3, &(pCfc->stMc.m[0][0]), 4, &(pTmp3x4->m[0][0]));
                    /* BT2020 full range RGB -> BT2020 limted range YCbCr */
                    BCFC_Csc_Mult_isrsafe(&(pColorSpaceExtOut->stM3x4.m[0][0]), 4, &(pTmp3x4->m[0][0]), 4, &(pCfc->stMc.m[0][0]));
                }
                else
                {
                    /* note: this HW can not even mimic BT2020-CL input, nor output, the following code might lead to identity matrix, it is ok
                     */

                    /* Tmp = B * Ain*/
                    BCFC_Csc_Mult_isrsafe(&(pCfc->stMb.m[0][0]), 3, &(pColorSpaceExtIn->stM3x4.m[0][0]), 4, &(pTmp3x4->m[0][0]));
                    /* C = Cout * Tmp */
                    BCFC_Csc_Mult_isrsafe(&(pColorSpaceExtOut->stM3x4.m[0][0]), 4, &(pTmp3x4->m[0][0]), 4, &(pCfc->stMc.m[0][0]));

                    /* refer to BVDC_P_Cfc_UpdateOutColorSpace_isr, if input eColorimetry != output eColorimetry, this cfc must be in cmp
                     * so the output is always limited range YCbCr */
                    if ((pColorSpaceIn->eColorimetry == BCFC_Colorimetry_eBt2020) && (pColorSpaceOut->eColorimetry != BCFC_Colorimetry_eBt2020))
                    {
                        /* BT2020 -> non-BT2020: Tmp = s_CscCr0p85Adj_BT2020_to_NonBT2020 * C */
                        BCFC_Csc_Mult_isrsafe(&(s_CscCr0p85Adj_BT2020_to_NonBT2020.m[0][0]), 4, &(pCfc->stMc.m[0][0]), 4, &(pTmp3x4->m[0][0]));
                        pCfc->stMc = *pTmp3x4;
                    }
                }
            }
        }

        pColorSpaceExtIn->stCfg.stBits.bDirty = BCFC_P_CLEAN;
        pColorSpaceExtOut->stCfg.stBits.bDirty = BCFC_P_CLEAN;
        pCfc->ucRulBuildCntr = BCFC_RUL_UPDATE_THRESHOLD;
    }

    return bRamLutCfgDirty;
}

/*
 * utility functions for debug messages
 */

#if (BDBG_DEBUG_BUILD)
static const char *const s_CfcName[] = {
    "Cmp0_V0",   /* BCFC_Id_eComp0_V0 */
    "Cmp0_V1",   /* BCFC_Id_eComp0_V1 */
    "Cmp1_V0",   /* BCFC_Id_eComp1_V0 */
    "Cmp1_V1",   /* BCFC_Id_eComp1_V1 */
    "Cmp2_V0",   /* BCFC_Id_eComp2_V0 */
    "Cmp3_V0",   /* BCFC_Id_eComp3_V0 */
    "Cmp4_V0",   /* BCFC_Id_eComp4_V0 */
    "Cmp5_V0",   /* BCFC_Id_eComp5_V0 */
    "Cmp6_V0",   /* BCFC_Id_eComp6_V0 */

    "Cmp0_G0",   /* BCFC_Id_eComp0_G0 */
    "Cmp1_G0",   /* BCFC_Id_eComp1_G0 */
    "Cmp2_G0",   /* BCFC_Id_eComp2_G0 */
    "Cmp3_G0",   /* BCFC_Id_eComp3_G0 */
    "Cmp4_G0",   /* BCFC_Id_eComp4_G0 */
    "Cmp5_G0",   /* BCFC_Id_eComp5_G0 */
    "Cmp6_G0",   /* BCFC_Id_eComp6_G0 */

    "Display0",  /* BCFC_Id_eDisplay0 */

    "Vfc0",      /* BCFC_Id_eVfc0 */
    "Vfc1",      /* BCFC_Id_eVfc1 */
    "Vfc2",      /* BCFC_Id_eVfc2 */
    "unknown"    /* BCFC_Id_eUnknown */
};

static const char *const s_ClrFmtName[] = {
    "RGB",        /* BCFC_ColorFormat_eRGB */
    "YCbCr",      /* BCFC_ColorFormat_eYCbCr */
    "YCbCr_CL",   /* BCFC_ColorFormat_eYCbCr_CL */
    "LMS",        /* BCFC_ColorFormat_eLMS */
    "ICtCp",      /* BCFC_ColorFormat_eICtCp */
    "Invalid"     /* BCFC_ColorFormat_eMax */
};

static const char *const s_ClrmtrName[] = {
    "Bt709",      /* BCFC_Colorimetry_eBt709 */
    "Smpte170M",  /* BCFC_Colorimetry_eSmpte170M */
    "Bt470_BG",   /* BCFC_Colorimetry_eBt470_BG */
    "Bt2020",     /* BCFC_Colorimetry_eBt2020 */
    "XvYcc601",   /* BCFC_Colorimetry_eXvYcc601 */
    "XvYcc709",   /* BCFC_Colorimetry_eXvYcc709 */
    "Fcc",        /* BCFC_Colorimetry_eFcc */
    "Smpte240M",  /* BCFC_Colorimetry_eSmpte240M */
    "Invalid"     /* BCFC_Colorimetry_eMax */
};

static const char *const s_ClrRngName[] = {
    "Limited",    /* BCFC_ColorRange_eLimited */
    "Full",       /* BCFC_ColorRange_eFull */
    "Invalid"     /* BCFC_ColorRange_eMax */
};

static const char *const s_ClrTfName[] = {
    "Bt1886",     /* ColorTF_eBt1886 */
    "Bt2100Pq",   /* ColorTF_eBt2100Pq */
    "Hlg",        /* ColorTF_eHlg */
    "Dbv",        /* ColorTF_eDbv */
    "Invalid"     /* ColorTF_eMax */
};

static const char *const s_ClrDptName[] = {
    "8Bit",       /* BCFC_ColorDepth_e8Bit */
    "10Bit",      /* BCFC_ColorDepth_e10Bit */
    "12Bit",      /* BCFC_ColorDepth_e12Bit */
    "14Bit",      /* BCFC_ColorDepth_e14Bit */
    "Invalid"     /* BCFC_ColorDepth_eMax */
};

const char *BCFC_GetCfcName_isrsafe(BCFC_Id eId)
{
    return s_CfcName[eId];
}

const char *BCFC_GetColorFormatName_isrsafe(BCFC_ColorFormat eFmt)
{
    return s_ClrFmtName[eFmt];
}

const char *BCFC_GetColorimetryName_isrsafe(BCFC_Colorimetry eClrmtr)
{
    return s_ClrmtrName[eClrmtr];
}

const char *BCFC_GetColorRangeName_isrsafe(BCFC_ColorRange eClrRng)
{
    return s_ClrRngName[eClrRng];
}

const char *BCFC_GetColorTfName_isrsafe(BCFC_ColorTF eClrTf)
{
    return s_ClrTfName[eClrTf];
}

const char *BCFC_GetColorDepthName_isrsafe(BCFC_ColorDepth eClrDpt)
{
    return s_ClrDptName[eClrDpt];
}

#endif /* #if (BDBG_DEBUG_BUILD) */

/* End of file */
