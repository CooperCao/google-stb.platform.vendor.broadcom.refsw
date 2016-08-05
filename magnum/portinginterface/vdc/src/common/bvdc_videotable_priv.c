/******************************************************************************
 *  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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

#include "bstd.h"
#include "bvdc_common_priv.h"
#include "bvdc_scaler_priv.h"
#include "bvdc_window_priv.h"
#include "bvdc_compositor_priv.h"
#include "bvdc_display_priv.h"
#include "bvdc_displayfmt_priv.h"
#include "bvdc_csc_priv.h"
#include "bchp_cmp_0.h"

BDBG_MODULE(BVDC_CSC);
BDBG_FILE_MODULE(BVDC_NLCSC);

#define BVDC_P_IS_XVYCC     (1)
#define BVDC_P_NOT_XVYCC    (0)

#define BVDC_P_IS_CL_IN     (1)
#define BVDC_P_NOT_CL_IN    (0)

#if ((BVDC_P_SUPPORT_CMP_NON_LINEAR_CSC!=0) && (BVDC_P_CMP_NON_LINEAR_CSC_VER >= BVDC_P_NL_CSC_VER_2))
/* 7271 A and up */

/***************************************************************************
 * Matrices for Non-Linear CSC version 2 and up
 *
 **************************************************************************/

/* MA: YCbCr -> R'G'B'
 */

/* BT709 YCbCr -> R'G'B' (typically HD) */
static const BVDC_P_CscAbCoeffs  s_CMP_BT709_YCbCr_to_RGB = BVDC_P_MAKE_CMP_CSC_AB
    (  1.168950,   0.000000,   1.799771,  -249.073939,
       1.168950,  -0.214085,  -0.534999,    77.179562,
       1.168950,   2.120686,   0.000000,  -290.150968 );

/* 170M YCbCr -> R'G'B' (typically Ntsc SD, or HDMI Pal SD) */
static const BVDC_P_CscAbCoeffs  s_CMP_170M_YCbCr_to_RGB = BVDC_P_MAKE_CMP_CSC_AB
    (  1.168950,   0.000000,   1.602286,  -223.795768,
       1.168950,  -0.393299,  -0.816156,   136.106963,
       1.168950,   2.025143,   0.000000,  -277.921482 );

/* BT470_2_BG YCbCr -> R'G'B' (typically analog Pal SD) */
static const BVDC_P_CscAbCoeffs  s_CMP_470_2_BG_YCbCr_to_RGB = BVDC_P_MAKE_CMP_CSC_AB
    (  1.168950,   0.000000,   1.602286,  -223.795768,
       1.168950,  -0.393299,  -0.816156,   136.106963,
       1.168950,   2.025143,   0.000000,  -277.921482 );

/* BT2020 YCbCr -> R'G'B' (typically UHD) */
static const BVDC_P_CscAbCoeffs  s_CMP_BT2020_YCbCr_to_RGB = BVDC_P_MAKE_CMP_CSC_AB
    (  1.168950,   0.000000,   1.685257,   -234.416111,
       1.168950,   -0.188061,   -0.652975,   88.949376,
       1.168950,   2.150171,   0.000000,   -293.925139 );


/* MC: R'G'B' -> YCbCr
 */

/* BT709 R'G'B' -> YCbCr (typically HD) */
static const BVDC_P_CscCoeffs  s_CMP_BT709_RGB_to_YCbCr = BVDC_P_MAKE_CMP_CSC
    (  0.181873,   0.611831,   0.061765,    16.000000,
      -0.100251,  -0.337249,   0.437500,   128.000000,
       0.437500,  -0.397384,  -0.040116,   128.000000 );

/* 170M R'G'B' -> YCbCr (typically Ntsc SD, or HDMI Pal SD) */
static const BVDC_P_CscCoeffs  s_CMP_170M_RGB_to_YCbCr = BVDC_P_MAKE_CMP_CSC
    (  0.255785,   0.502160,   0.097523,    16.000000,
      -0.147644,  -0.289856,   0.437500,   128.000000,
       0.437500,  -0.366352,  -0.071148,   128.000000 );

/* BT470_2_BG R'G'B' -> YCbCr (typically analog Pal SD ) */
static const BVDC_P_CscCoeffs  s_CMP_470_2_BG_RGB_to_YCbCr = BVDC_P_MAKE_CMP_CSC
    (  0.255785,   0.502160,   0.097523,    16.000000,
      -0.147644,  -0.289856,   0.437500,   128.000000,
       0.437500,  -0.366352,  -0.071148,   128.000000 );

/* BT2020 R'G'B' -> YCbCr (typically UHD) */
static const BVDC_P_CscCoeffs  s_CMP_BT2020_RGB_to_YCbCr = BVDC_P_MAKE_CMP_CSC
    (  0.224732,   0.580008,   0.050729,    16.000000,
      -0.122176,  -0.315324,   0.437500,   128.000000,
       0.437500,  -0.402312,  -0.035188,   128.000000 );


/* MB: RGB -> RGB
 */

/* BT709 -> 170M */
static const BVDC_P_CscAbCoeffs  s_CMP_RGB_BT709_to_170M = BVDC_P_MAKE_CMP_CSC_AB
    (  1.065379,  -0.055401,  -0.009978,   0.000000,
      -0.019633,   1.036363,  -0.016731,   0.000000,
       0.001632,   0.004412,   0.993956,   0.000000 );

/* BT709 -> 470_2_BG */
static const BVDC_P_CscAbCoeffs  s_CMP_RGB_BT709_to_470_2_BG = BVDC_P_MAKE_CMP_CSC_AB
    (  0.954734,   0.042050,   0.000000,   0.000000,
       0.000000,   1.000923,   0.000000,   0.000000,
       0.000000,  -0.011945,   1.012859,   0.000000 );

/* BT709 -> BT2020Ncl */
static const BVDC_P_CscAbCoeffs  s_CMP_RGB_BT709_to_BT2020Ncl = BVDC_P_MAKE_CMP_CSC_AB
    (  0.627404,   0.329283,   0.043313,   0.000000,
       0.069097,   0.919540,   0.011362,   0.000000,
       0.016391,   0.088013,   0.895595,   0.000000 );

#if 0
/* BT709 -> BT2020Cl */
static const BVDC_P_CscAbCoeffs  s_CMP_RGB_BT709_to_BT2020Cl = BVDC_P_MAKE_CMP_CSC_AB
    (  0.627404,   0.329283,   0.043313,   0.000000,
       0.212639,   0.715170,   0.072191,   0.000000,
       0.016391,   0.088013,   0.895595,   0.000000 );
#endif

/* 170M -> BT709 */
static const BVDC_P_CscAbCoeffs  s_CMP_RGB_170M_to_BT709 = BVDC_P_MAKE_CMP_CSC_AB
    (  0.939542,   0.050181,   0.010277,   0.000000,
       0.017772,   0.965793,   0.016435,   0.000000,
      -0.001622,  -0.004370,   1.005991,   0.000000 );

/* 170M -> 470_2_BG */
static const BVDC_P_CscAbCoeffs  s_CMP_RGB_170M_to_470_2_BG = BVDC_P_MAKE_CMP_CSC_AB
    (  0.897760,   0.088521,   0.010502,   0.000000,
       0.017789,   0.966684,   0.016450,   0.000000,
      -0.001855,  -0.015962,   1.018731,   0.000000 );

/* 170M -> BT2020Ncl */
static const BVDC_P_CscAbCoeffs  s_CMP_RGB_170M_to_BT2020Ncl = BVDC_P_MAKE_CMP_CSC_AB
    (  0.595254,   0.349314,   0.055432,   0.000000,
       0.081244,   0.891503,   0.027253,   0.000000,
       0.015512,   0.081912,   0.902576,   0.000000 );

#if 0
/* 170M -> BT2020Cl */
static const BVDC_P_CscAbCoeffs  s_CMP_RGB_170M_to_BT2020Cl = BVDC_P_MAKE_CMP_CSC_AB
    (  0.595254,   0.349314,   0.055432,   0.000000,
       0.212376,   0.701061,   0.086562,   0.000000,
       0.015512,   0.081912,   0.902576,   0.000000 );
#endif

/* 470_2_BG -> BT709 */
static const BVDC_P_CscAbCoeffs  s_CMP_RGB_470_2_BG_to_BT709 = BVDC_P_MAKE_CMP_CSC_AB
    (  1.047413,  -0.044003,   0.000000,   0.000000,
       0.000000,   0.999078,   0.000000,   0.000000,
       0.000000,   0.011783,   0.987304,   0.000000 );

/* 470_2_BG -> 170M */
static const BVDC_P_CscAbCoeffs  s_CMP_RGB_470_2_BG_to_170M = BVDC_P_MAKE_CMP_CSC_AB
    (  1.115891,  -0.102347,  -0.009851,   0.000000,
      -0.020563,   1.036075,  -0.016518,   0.000000,
       0.001709,   0.016048,   0.981337,   0.000000 );

/* 470_2_BG -> BT2020Ncl */
static const BVDC_P_CscAbCoeffs  s_CMP_RGB_470_2_BG_to_BT2020Ncl = BVDC_P_MAKE_CMP_CSC_AB
    (  0.657151,   0.301882,   0.042763,   0.000000,
       0.072373,   0.915786,   0.011218,   0.000000,
       0.017169,   0.097763,   0.884225,   0.000000 );

#if 0
/* 470_2_BG -> BT2020Cl */
static const BVDC_P_CscAbCoeffs  s_CMP_RGB_470_2_BG_to_BT2020Cl = BVDC_P_MAKE_CMP_CSC_AB
    (  0.657151,   0.301882,   0.042763,   0.000000,
       0.222721,   0.706005,   0.071274,   0.000000,
       0.017169,   0.097763,   0.884225,   0.000000 );
#endif

/* BT2020Ncl -> BT709 */
static const BVDC_P_CscAbCoeffs  s_CMP_RGB_BT2020Ncl_to_BT709 = BVDC_P_MAKE_CMP_CSC_AB
    (  1.660491,  -0.587641,  -0.072850,   0.000000,
      -0.124550,   1.132900,  -0.008349,   0.000000,
      -0.018151,  -0.100579,   1.118730,   0.000000 );

/* BT2020Ncl -> 170M */
static const BVDC_P_CscAbCoeffs  s_CMP_RGB_BT2020Ncl_to_170M = BVDC_P_MAKE_CMP_CSC_AB
    (  1.776134,  -0.687821,  -0.088313,   0.000000,
      -0.161376,   1.187315,  -0.025940,   0.000000,
      -0.015881,  -0.095931,   1.111812,   0.000000 );

/* BT2020Ncl -> 470_2_BG */
static const BVDC_P_CscAbCoeffs  s_CMP_RGB_BT2020Ncl_to_470_2_BG = BVDC_P_MAKE_CMP_CSC_AB
    (  1.580089,  -0.513403,  -0.069903,   0.000000,
      -0.124665,   1.133945,  -0.008357,   0.000000,
      -0.016896,  -0.115405,   1.133215,   0.000000 );

#if 0
/* BT2020Ncl -> BT2020Cl */
static const BVDC_P_CscAbCoeffs  s_CMP_RGB_BT2020Ncl_to_BT2020Cl = BVDC_P_MAKE_CMP_CSC_AB
    (  1.000000,   0.000000,   0.000000,   0.000000,
       0.262700,   0.678000,   0.059300,   0.000000,
       0.000000,   0.000000,   1.000000,   0.000000 );
#endif

/* BT2020Cl -> BT709 */
static const BVDC_P_CscAbCoeffs  s_CMP_RYB_BT2020Cl_to_RGB_BT709 = BVDC_P_MAKE_CMP_CSC_AB
    (  1.888180,  -0.866727,  -0.021453,   0.000000,
      -0.563507,   1.670944,  -0.107436,   0.000000,
       0.020820,  -0.148346,   1.127527,   0.000000 );

/* BT2020Cl -> 170M */
static const BVDC_P_CscAbCoeffs  s_CMP_RYB_BT2020Cl_to_RGB_170M = BVDC_P_MAKE_CMP_CSC_AB
    (  2.042639,  -1.014485,  -0.028154,   0.000000,
      -0.621416,   1.751202,  -0.129786,   0.000000,
       0.021289,  -0.141492,   1.120202,   0.000000 );

/* BT2020Cl -> 470_2_BG */
static const BVDC_P_CscAbCoeffs  s_CMP_RYB_BT2020Cl_to_RGB_470_2_BG = BVDC_P_MAKE_CMP_CSC_AB
    (  1.779014,  -0.757231,  -0.024999,   0.000000,
      -0.564027,   1.672485,  -0.107536,   0.000000,
       0.027819,  -0.170213,   1.143309,   0.000000 );

/* BT2020Cl -> BT2020Ncl */
static const BVDC_P_CscAbCoeffs  s_CMP_RYB_BT2020Cl_to_RGB_BT2020Ncl = BVDC_P_MAKE_CMP_CSC_AB
    (  1.000000,   0.000000,   0.000000,   0.000000,
      -0.387463,   1.474926,  -0.087463,   0.000000,
       0.000000,   0.000000,   1.000000,   0.000000 );

/* BT2020Cl -> BT2020Ncl */
const BVDC_P_CscAbCoeffs  s_CMP_AB_Identity = BVDC_P_MAKE_CMP_CSC_AB
    (  1.000000,  0.000000,  0.000000,  0.000000,
       0.000000,  1.000000,  0.000000,  0.000000,
       0.000000,  0.000000,  1.000000,  0.000000 );

/* for coding convenience */
#define s_CMP_RGB_BT709_to_BT709             s_CMP_AB_Identity
#define s_CMP_RGB_170M_to_170M               s_CMP_AB_Identity
#define s_CMP_RGB_470_2_BG_to_470_2_BG       s_CMP_AB_Identity
#define s_CMP_RGB_BT2020Ncl_to_BT2020Ncl     s_CMP_AB_Identity

/* TODO:  get real matrices */
#define s_CMP_XvYCC601_YCbCr_to_RGB          s_CMP_170M_YCbCr_to_RGB
#define s_CMP_XvYCC601_RGB_to_YCbCr          s_CMP_170M_RGB_to_YCbCr

#define s_CMP_RGB_XvYCC601_to_BT709          s_CMP_RGB_170M_to_BT709
#define s_CMP_RGB_XvYCC601_to_170M           s_CMP_RGB_170M_to_170M
#define s_CMP_RGB_XvYCC601_to_470_2_BG       s_CMP_RGB_170M_to_470_2_BG
#define s_CMP_RGB_XvYCC601_to_BT2020Ncl      s_CMP_RGB_170M_to_BT2020Ncl
#define s_CMP_RGB_XvYCC601_to_XvYCC601       s_CMP_RGB_170M_to_170M

#define s_CMP_RGB_BT709_to_XvYCC601          s_CMP_RGB_BT709_to_170M
#define s_CMP_RGB_170M_to_XvYCC601           s_CMP_RGB_170M_to_170M
#define s_CMP_RGB_470_2_BG_to_XvYCC601       s_CMP_RGB_470_2_BG_to_170M
#define s_CMP_RGB_BT2020Ncl_to_XvYCC601      s_CMP_RGB_BT2020Ncl_to_170M
#define s_CMP_RYB_BT2020Cl_to_RGB_XvYCC601   s_CMP_RYB_BT2020Cl_to_RGB_170M
#define s_CMP_RGB_BT709_to_XvYCC601          s_CMP_RGB_BT709_to_170M

#define s_CMP_FCC_YCbCr_to_RGB               s_CMP_170M_YCbCr_to_RGB
#define s_CMP_240M_YCbCr_to_RGB              s_CMP_170M_YCbCr_to_RGB

#define s_CMP_RGB_FCC_to_BT709               s_CMP_RGB_170M_to_BT709
#define s_CMP_RGB_FCC_to_170M                s_CMP_RGB_170M_to_170M
#define s_CMP_RGB_FCC_to_470_2_BG            s_CMP_RGB_170M_to_470_2_BG
#define s_CMP_RGB_FCC_to_BT2020Ncl           s_CMP_RGB_170M_to_BT2020Ncl
#define s_CMP_RGB_FCC_to_BT2020Ncl           s_CMP_RGB_170M_to_BT2020Ncl
#define s_CMP_RGB_FCC_to_XvYCC601            s_CMP_RGB_170M_to_XvYCC601
#define s_CMP_RGB_FCC_to_BT709               s_CMP_RGB_170M_to_BT709

#define s_CMP_RGB_240M_to_BT709              s_CMP_RGB_170M_to_BT709
#define s_CMP_RGB_240M_to_170M               s_CMP_RGB_170M_to_170M
#define s_CMP_RGB_240M_to_470_2_BG           s_CMP_RGB_170M_to_470_2_BG
#define s_CMP_RGB_240M_to_BT2020Ncl          s_CMP_RGB_170M_to_BT2020Ncl
#define s_CMP_RGB_240M_to_BT2020Ncl          s_CMP_RGB_170M_to_BT2020Ncl
#define s_CMP_RGB_240M_to_XvYCC601           s_CMP_RGB_170M_to_XvYCC601
#define s_CMP_RGB_240M_to_BT709              s_CMP_RGB_170M_to_BT709


/* L-Range Adjust:
 */

/* 1886 - > 2084 */
const BVDC_P_CscLRangeAdj s_CMP_LRangeAdj_1886_to_2084 = BVDC_P_MAKE_CMP_NL_LR_ADJ
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

/* identity */
const BVDC_P_CscLRangeAdj s_CMP_LRangeAdj_Identity = BVDC_P_MAKE_CMP_NL_LR_ADJ
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

/* 2084 -> 1886 */
const BVDC_P_CscLRangeAdj s_CMP_LRangeAdj_2084_to_1886 = BVDC_P_MAKE_CMP_NL_LR_ADJ
    (  8, /* number of points */
       /*     x,        y,            m,  e */
       0.000000, 0.000000, 0.5099759615,  7,
       0.006500, 0.424300, 0.7236519608,  5,
       0.016700, 0.660500, 0.5141267123,  4,
       0.031300, 0.780600, 0.6018518519,  3,
       0.050200, 0.871600, 0.7762557078,  2,
       0.072100, 0.939600, 0.5412186380,  2,
       0.100000, 1.000000, 0.0000000000,  0,
       1.000000, 1.000000, 0.5000000000,  1 );

const BVDC_P_CscAbCoeffs *const s_aCMP_MA_Tbl[] =
{
    &s_CMP_BT709_YCbCr_to_RGB,                /* BVDC_P_MatrixCoeffs_eBt709 */
    &s_CMP_170M_YCbCr_to_RGB,                 /* BVDC_P_MatrixCoeffs_eSmpte170M */
    &s_CMP_470_2_BG_YCbCr_to_RGB,             /* BVDC_P_MatrixCoeffs_eBt470_2_BG */
    &s_CMP_BT2020_YCbCr_to_RGB,               /* BVDC_P_MatrixCoeffs_eBt2020_NCL */
    &s_CMP_AB_Identity,                       /* BVDC_P_MatrixCoeffs_eBt2020_CL */
    &s_CMP_XvYCC601_YCbCr_to_RGB,             /* BVDC_P_MatrixCoeffs_eXvYcc601 */
    &s_CMP_BT709_YCbCr_to_RGB,                /* BVDC_P_MatrixCoeffs_eXvYcc709 */
    &s_CMP_FCC_YCbCr_to_RGB,                  /* BVDC_P_MatrixCoeffs_eFcc */
    &s_CMP_240M_YCbCr_to_RGB                  /* BVDC_P_MatrixCoeffs_eSmpte_240M */
};

const BVDC_P_CscCoeffs *const s_aCMP_MC_Tbl[] =
{
    &s_CMP_BT709_RGB_to_YCbCr,                /* BVDC_P_MatrixCoeffs_eBt709 */
    &s_CMP_170M_RGB_to_YCbCr,                 /* BVDC_P_MatrixCoeffs_eSmpte170M */
    &s_CMP_470_2_BG_RGB_to_YCbCr,             /* BVDC_P_MatrixCoeffs_eBt470_2_BG */
    &s_CMP_BT2020_RGB_to_YCbCr,               /* BVDC_P_MatrixCoeffs_eBt2020_NCL */
    &s_CMP_BT2020_RGB_to_YCbCr,               /* BVDC_P_MatrixCoeffs_eBt2020_CL */
    &s_CMP_XvYCC601_RGB_to_YCbCr,             /* BVDC_P_MatrixCoeffs_eXvYcc601 */
    &s_CMP_BT709_RGB_to_YCbCr                 /* BVDC_P_MatrixCoeffs_eXvYcc709 */
};

const BVDC_P_CscAbCoeffs * const s_aaCMP_MB_Tbl[][7] =
{
    /* BVDC_P_MatrixCoeffs_eBt709 */
    {
        &s_CMP_RGB_BT709_to_BT709,            /* BVDC_P_MatrixCoeffs_eBt709 */
        &s_CMP_RGB_BT709_to_170M,             /* BVDC_P_MatrixCoeffs_eSmpte170M */
        &s_CMP_RGB_BT709_to_470_2_BG,         /* BVDC_P_MatrixCoeffs_eBt470_2_BG */
        &s_CMP_RGB_BT709_to_BT2020Ncl,        /* BVDC_P_MatrixCoeffs_eBt2020_NCL */
        &s_CMP_RGB_BT709_to_BT2020Ncl,        /* BVDC_P_MatrixCoeffs_eBt2020_CL */
        &s_CMP_RGB_BT709_to_XvYCC601,         /* BVDC_P_MatrixCoeffs_eXvYcc601 */
        &s_CMP_RGB_BT709_to_BT709             /* BVDC_P_MatrixCoeffs_eXvYcc709 */
    },

    /* BVDC_P_MatrixCoeffs_eSmpte170M */
    {
        &s_CMP_RGB_170M_to_BT709,             /* BVDC_P_MatrixCoeffs_eBt709 */
        &s_CMP_RGB_170M_to_170M,              /* BVDC_P_MatrixCoeffs_eSmpte170M */
        &s_CMP_RGB_170M_to_470_2_BG,          /* BVDC_P_MatrixCoeffs_eBt470_2_BG */
        &s_CMP_RGB_170M_to_BT2020Ncl,         /* BVDC_P_MatrixCoeffs_eBt2020_NCL */
        &s_CMP_RGB_170M_to_BT2020Ncl,         /* BVDC_P_MatrixCoeffs_eBt2020_CL */
        &s_CMP_RGB_170M_to_XvYCC601,          /* BVDC_P_MatrixCoeffs_eXvYcc601 */
        &s_CMP_RGB_170M_to_BT709              /* BVDC_P_MatrixCoeffs_eXvYcc709 */
    },

    /* BVDC_P_MatrixCoeffs_eBt470_2_BG */
    {
        &s_CMP_RGB_470_2_BG_to_BT709,         /* BVDC_P_MatrixCoeffs_eBt709 */
        &s_CMP_RGB_470_2_BG_to_170M,          /* BVDC_P_MatrixCoeffs_eSmpte170M */
        &s_CMP_RGB_470_2_BG_to_470_2_BG,      /* BVDC_P_MatrixCoeffs_eBt470_2_BG */
        &s_CMP_RGB_470_2_BG_to_BT2020Ncl,     /* BVDC_P_MatrixCoeffs_eBt2020_NCL */
        &s_CMP_RGB_470_2_BG_to_BT2020Ncl,     /* BVDC_P_MatrixCoeffs_eBt2020_CL */
        &s_CMP_RGB_470_2_BG_to_XvYCC601,      /* BVDC_P_MatrixCoeffs_eXvYcc601 */
        &s_CMP_RGB_470_2_BG_to_BT709          /* BVDC_P_MatrixCoeffs_eXvYcc709 */
    },

    /* BVDC_P_MatrixCoeffs_eBt2020_NCL */
    {
        &s_CMP_RGB_BT2020Ncl_to_BT709,        /* BVDC_P_MatrixCoeffs_eBt709 */
        &s_CMP_RGB_BT2020Ncl_to_170M,         /* BVDC_P_MatrixCoeffs_eSmpte170M */
        &s_CMP_RGB_BT2020Ncl_to_470_2_BG,     /* BVDC_P_MatrixCoeffs_eBt470_2_BG */
        &s_CMP_RGB_BT2020Ncl_to_BT2020Ncl,    /* BVDC_P_MatrixCoeffs_eBt2020_NCL */
        &s_CMP_RGB_BT2020Ncl_to_BT2020Ncl,    /* BVDC_P_MatrixCoeffs_eBt2020_CL */
        &s_CMP_RGB_BT2020Ncl_to_XvYCC601,     /* BVDC_P_MatrixCoeffs_eXvYcc601 */
        &s_CMP_RGB_BT2020Ncl_to_BT709         /* BVDC_P_MatrixCoeffs_eXvYcc709 */
    },

    /* BVDC_P_MatrixCoeffs_eBt2020_CL */
    {
        &s_CMP_RYB_BT2020Cl_to_RGB_BT709,     /* BVDC_P_MatrixCoeffs_eBt709 */
        &s_CMP_RYB_BT2020Cl_to_RGB_170M,      /* BVDC_P_MatrixCoeffs_eSmpte170M */
        &s_CMP_RYB_BT2020Cl_to_RGB_470_2_BG,  /* BVDC_P_MatrixCoeffs_eBt470_2_BG */
        &s_CMP_RYB_BT2020Cl_to_RGB_BT2020Ncl, /* BVDC_P_MatrixCoeffs_eBt2020_NCL */
        &s_CMP_RYB_BT2020Cl_to_RGB_BT2020Ncl, /* BVDC_P_MatrixCoeffs_eBt2020_CL */
        &s_CMP_RYB_BT2020Cl_to_RGB_XvYCC601,  /* BVDC_P_MatrixCoeffs_eXvYcc601 */
        &s_CMP_RYB_BT2020Cl_to_RGB_BT709      /* BVDC_P_MatrixCoeffs_eXvYcc709 */
    },

    /* BVDC_P_MatrixCoeffs_eXvYcc601 */
    {
        &s_CMP_RGB_XvYCC601_to_BT709,         /* BVDC_P_MatrixCoeffs_eBt709 */
        &s_CMP_RGB_XvYCC601_to_170M,          /* BVDC_P_MatrixCoeffs_eSmpte170M */
        &s_CMP_RGB_XvYCC601_to_470_2_BG,      /* BVDC_P_MatrixCoeffs_eBt470_2_BG */
        &s_CMP_RGB_XvYCC601_to_BT2020Ncl,     /* BVDC_P_MatrixCoeffs_eBt2020_NCL */
        &s_CMP_RGB_XvYCC601_to_BT2020Ncl,     /* BVDC_P_MatrixCoeffs_eBt2020_CL */
        &s_CMP_RGB_XvYCC601_to_XvYCC601,      /* BVDC_P_MatrixCoeffs_eXvYcc601 */
        &s_CMP_RGB_XvYCC601_to_BT709          /* BVDC_P_MatrixCoeffs_eXvYcc709 */
    },

    /* BVDC_P_MatrixCoeffs_eXvYcc709 */
    {
        &s_CMP_RGB_BT709_to_BT709,            /* BVDC_P_MatrixCoeffs_eBt709 */
        &s_CMP_RGB_BT709_to_170M,             /* BVDC_P_MatrixCoeffs_eSmpte170M */
        &s_CMP_RGB_BT709_to_470_2_BG,         /* BVDC_P_MatrixCoeffs_eBt470_2_BG */
        &s_CMP_RGB_BT709_to_BT2020Ncl,        /* BVDC_P_MatrixCoeffs_eBt2020_NCL */
        &s_CMP_RGB_BT709_to_BT2020Ncl,        /* BVDC_P_MatrixCoeffs_eBt2020_CL */
        &s_CMP_RGB_BT709_to_XvYCC601,         /* BVDC_P_MatrixCoeffs_eXvYcc601 */
        &s_CMP_RGB_BT709_to_BT709             /* BVDC_P_MatrixCoeffs_eXvYcc709 */
    },

    /* BVDC_P_MatrixCoeffs_eFcc */
    {
        &s_CMP_RGB_FCC_to_BT709,            /* BVDC_P_MatrixCoeffs_eBt709 */
        &s_CMP_RGB_FCC_to_170M,             /* BVDC_P_MatrixCoeffs_eSmpte170M */
        &s_CMP_RGB_FCC_to_470_2_BG,         /* BVDC_P_MatrixCoeffs_eBt470_2_BG */
        &s_CMP_RGB_FCC_to_BT2020Ncl,        /* BVDC_P_MatrixCoeffs_eBt2020_NCL */
        &s_CMP_RGB_FCC_to_BT2020Ncl,        /* BVDC_P_MatrixCoeffs_eBt2020_CL */
        &s_CMP_RGB_FCC_to_XvYCC601,         /* BVDC_P_MatrixCoeffs_eXvYcc601 */
        &s_CMP_RGB_FCC_to_BT709             /* BVDC_P_MatrixCoeffs_eXvYcc709 */
    },

    /* BVDC_P_MatrixCoeffs_eSmpte240M */
    {
        &s_CMP_RGB_240M_to_BT709,            /* BVDC_P_MatrixCoeffs_eBt709 */
        &s_CMP_RGB_240M_to_170M,             /* BVDC_P_MatrixCoeffs_eSmpte170M */
        &s_CMP_RGB_240M_to_470_2_BG,         /* BVDC_P_MatrixCoeffs_eBt470_2_BG */
        &s_CMP_RGB_240M_to_BT2020Ncl,        /* BVDC_P_MatrixCoeffs_eBt2020_NCL */
        &s_CMP_RGB_240M_to_BT2020Ncl,        /* BVDC_P_MatrixCoeffs_eBt2020_CL */
        &s_CMP_RGB_240M_to_XvYCC601,         /* BVDC_P_MatrixCoeffs_eXvYcc601 */
        &s_CMP_RGB_240M_to_BT709             /* BVDC_P_MatrixCoeffs_eXvYcc709 */
    }
};

#if (BVDC_P_CMP_NON_LINEAR_CSC_VER == BVDC_P_NL_CSC_VER_2)
#define BVDC_P_NL2L_709     BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_RECT1_SEL_NL2L_709
#define BVDC_P_NL2L_1886    BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_RECT1_SEL_NL2L_1886
#define BVDC_P_NL2L_PQ      BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_RECT1_SEL_NL2L_PQ
#define BVDC_P_NL2L_BBC     BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_RECT1_SEL_NL2L_BBC
#define BVDC_P_NL2L_RAM     BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_RECT1_SEL_NL2L_RAM
#define BVDC_P_NL2L_BYPASS  BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_RECT1_SEL_NL2L_BYPASS

#define BVDC_P_L2NL_709     BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_RECT1_SEL_L2NL_709
#define BVDC_P_L2NL_1886    BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_RECT1_SEL_L2NL_1886
#define BVDC_P_L2NL_PQ      BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_RECT1_SEL_L2NL_PQ
#define BVDC_P_L2NL_BBC     BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_RECT1_SEL_L2NL_BBC
#define BVDC_P_L2NL_RAM     BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_RECT1_SEL_L2NL_RAM
#define BVDC_P_L2NL_BYPASS  BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_RECT1_SEL_L2NL_BYPASS
#else /* TODO: add new HDR support */
#define BVDC_P_NL2L_709     0
#define BVDC_P_NL2L_1886    0
#define BVDC_P_NL2L_PQ      0
#define BVDC_P_NL2L_BBC     0
#define BVDC_P_NL2L_RAM     0
#define BVDC_P_NL2L_BYPASS  0

#define BVDC_P_L2NL_709     0
#define BVDC_P_L2NL_1886    0
#define BVDC_P_L2NL_PQ      0
#define BVDC_P_L2NL_BBC     0
#define BVDC_P_L2NL_RAM     0
#define BVDC_P_L2NL_BYPASS  0
#endif

uint8_t s_aCMP_CSC_NL2L_Tbl[] =
{
    BVDC_P_NL2L_1886,                 /* BAVC_HDMI_DRM_EOTF_eSDR */
    BVDC_P_NL2L_PQ,                   /* BAVC_HDMI_DRM_EOTF_eHDR */
    BVDC_P_NL2L_PQ,                   /* BAVC_HDMI_DRM_EOTF_eSMPTE_ST_2084 */
    BVDC_P_NL2L_BBC,                  /* BAVC_HDMI_DRM_EOTF_eFuture */
    BVDC_P_NL2L_1886                  /* BAVC_HDMI_DRM_EOTF_eMax */
};

uint8_t s_aCMP_CSC_L2NL_Tbl[] =
{
    BVDC_P_L2NL_1886,                 /* BAVC_HDMI_DRM_EOTF_eSDR */
    BVDC_P_L2NL_PQ,                   /* BAVC_HDMI_DRM_EOTF_eHDR */
    BVDC_P_L2NL_PQ,                   /* BAVC_HDMI_DRM_EOTF_eSMPTE_ST_2084 */
    BVDC_P_L2NL_BBC,                  /* BAVC_HDMI_DRM_EOTF_eFuture */
    BVDC_P_L2NL_1886                  /* BAVC_HDMI_DRM_EOTF_eMax */
};

static const BVDC_P_CscLRangeAdj * const s_aaCMP_LRangeAdj_Tbl[][4] =
{
    /* BAVC_HDMI_DRM_EOTF_eSDR */
    {
        &s_CMP_LRangeAdj_Identity,        /* BAVC_HDMI_DRM_EOTF_eSDR */
        &s_CMP_LRangeAdj_1886_to_2084,    /* BAVC_HDMI_DRM_EOTF_eHDR */
        &s_CMP_LRangeAdj_1886_to_2084,    /* BAVC_HDMI_DRM_EOTF_eSMPTE_ST_2084 */
        &s_CMP_LRangeAdj_1886_to_2084,    /* BAVC_HDMI_DRM_EOTF_eFuture */
    },

    /* BAVC_HDMI_DRM_EOTF_eHDR */
    {
        &s_CMP_LRangeAdj_2084_to_1886,    /* BAVC_HDMI_DRM_EOTF_eSDR */
        &s_CMP_LRangeAdj_Identity,        /* BAVC_HDMI_DRM_EOTF_eHDR */
        &s_CMP_LRangeAdj_Identity,        /* BAVC_HDMI_DRM_EOTF_eSMPTE_ST_2084 */
        &s_CMP_LRangeAdj_Identity         /* BAVC_HDMI_DRM_EOTF_eFuture */
    },

    /* BAVC_HDMI_DRM_EOTF_eSMPTE_ST_2084 */
    {
        &s_CMP_LRangeAdj_2084_to_1886,    /* BAVC_HDMI_DRM_EOTF_eSDR */
        &s_CMP_LRangeAdj_Identity,        /* BAVC_HDMI_DRM_EOTF_eHDR */
        &s_CMP_LRangeAdj_Identity,        /* BAVC_HDMI_DRM_EOTF_eSMPTE_ST_2084 */
        &s_CMP_LRangeAdj_Identity         /* BAVC_HDMI_DRM_EOTF_eFuture */
    },

    /* BAVC_HDMI_DRM_EOTF_eFuture */
    {
        &s_CMP_LRangeAdj_2084_to_1886,    /* BAVC_HDMI_DRM_EOTF_eSDR */
        &s_CMP_LRangeAdj_Identity,        /* BAVC_HDMI_DRM_EOTF_eHDR */
        &s_CMP_LRangeAdj_Identity,        /* BAVC_HDMI_DRM_EOTF_eSMPTE_ST_2084 */
        &s_CMP_LRangeAdj_Identity         /* BAVC_HDMI_DRM_EOTF_eFuture */
    }
};

#endif /* #if ((BVDC_P_SUPPORT_CMP_NON_LINEAR_CSC!=0) && (BVDC_P_CMP_NON_LINEAR_CSC_VER >= BVDC_P_NL_CSC_VER_2)) */

/***************************************************************************
 * video surface color space conversion matrices Set I
 * Note:
 *  - these csc tables match the color primaries of different standards;
 *  - we do not do non-linear gamma correction;
 *  - the CMP matrix is for [YCbCr] -> [YCbCr];
 *  - PR10417: coeffs are in two's complement fixed point format;
 * -----------------------------------------------------
 * Color Space Standards:
 *  - ITU-R BT.709, i.e. ATSC HD or PAL HD;
 *  - FCC, i.e. NTSC SD 1953 standard, with ITU-R BT.470-2 System M color primaries;
 *  - ITU-R BT.470-2 System B, G, i.e. PAL SD;
 *  - SMPTE 170M, i.e. modern NTSC SD;
 *  - SMPTE 240M (1987 standard), the legacy ATSC HD;
 **************************************************************************/

/* Group 0: xyz -> BT.709 (i.e. HD) */
/* SMPTE 170M (i.e. modern SD NTSC) -> BT.709 (i.e. HD) */
static const BVDC_P_CscCoeffs  s_CMP_NtscSDYCbCr_to_HDYCbCr = BVDC_P_MAKE_CMP_CSC
    (  1.000000, -0.086918, -0.198602,  36.546531,
       0.000000,  1.008912,  0.109928, -15.211527,
      -0.000000,  0.057004,  0.942791,   0.026243 );

/* BT.470-2 System B, G (i.e. SD Pal) -> BT.709 (i.e. HD) */
static const BVDC_P_CscCoeffs  s_CMP_PalSDYCbCr_to_HDYCbCr = BVDC_P_MAKE_CMP_CSC
    (  1.000000, -0.115168, -0.189312,  38.973396,
       0.000000,  1.004499,  0.099809, -13.351437,
      -0.000000,  0.084468,  1.072557, -20.099241 );

/* SMPTE 240M (i.e. 1987 ATSC HD) -> BT.709 (i.e. HD) */
static const BVDC_P_CscCoeffs  s_CMP_240MHDYCbCr_to_HDYCbCr = BVDC_P_MAKE_CMP_CSC
    (  1.000000,  0.000000, -0.000000,  0.000000,
      -0.000000,  0.990940, -0.000253,  1.192039,
      -0.000000,  0.004734,  0.924649,  9.039049 );

/* FCC (i.e. 1953 NTSC) -> BT.709 (i.e. HD) */
static const BVDC_P_CscCoeffs  s_CMP_FccSDYCbCr_to_HDYCbCr = BVDC_P_MAKE_CMP_CSC
    (  1.000000, -0.000000,  0.000000,   0.0000000,
       0.054988,  1.121448, -0.005106, -15.771562,
       0.033634, -0.011863,  1.511262, -64.461172 );

/* Group 1: xyz -> SMPTE 170M (i.e. modern SD NTSC) */
/* BT.709 (i.e. HD) -> SMPTE 170M (i.e. modern SD NTSC) */
static const BVDC_P_CscCoeffs  s_CMP_HDYCbCr_to_NtscSDYCbCr = BVDC_P_MAKE_CMP_CSC
    (  1.000000,  0.074740,  0.201939, -35.414914,
      -0.000000,  0.997740, -0.116335,  15.180202,
       0.000000, -0.060327,  1.067715,  -0.945678 );

/* BT.470-2 System B, G (i.e. SD Pal) -> SMPTE 170M (i.e. modern SD NTSC) */
static const BVDC_P_CscCoeffs  s_CMP_PalSDYCbCr_to_NtscSDYCbCr = BVDC_P_MAKE_CMP_CSC
    (  1.000000, -0.023033,  0.034738,  -1.498223,
      -0.000000,  0.992402, -0.025193,   4.197192,
      -0.000000,  0.029590,  1.139164, -21.600485 );

/* SMPTE 240M (i.e. 1987 ATSC HD) -> SMPTE 170M (i.e. modern SD NTSC) */
static const BVDC_P_CscCoeffs  s_CMP_240MHDYCbCr_to_NtscSDYCbCr = BVDC_P_MAKE_CMP_CSC
    (  1.000000,  0.075019,  0.186703, -33.500488,
      -0.000000,  0.988150, -0.107822,  15.317986,
      -0.000000, -0.054726,  0.987276,   8.633535 );

/* FCC (i.e. 1953 NTSC) -> SMPTE 170M (i.e. modern SD NTSC) */
static const BVDC_P_CscCoeffs  s_CMP_FccSDYCbCr_to_NtscSDYCbCr = BVDC_P_MAKE_CMP_CSC
    (  1.010902,  0.081422,  0.304801, -49.610885,
       0.050951,  1.120293, -0.180907,   6.943399,
       0.032594, -0.080320,  1.613905, -68.820370 );

/* Group 2: xyz -> BT.470-2 System B, G (i.e. SD Pal) */
/* SMPTE 170M (i.e. modern SD NTSC) -> BT.470-2 System B, G (i.e. SD Pal) */
static const BVDC_P_CscCoeffs  s_CMP_NtscSDYCbCr_to_PalSDYCbCr = BVDC_P_MAKE_CMP_CSC
    (  1.000000,  0.024103, -0.029962,   0.749873,
       0.000000,  1.006992,  0.022270,  -3.745506,
       0.000000, -0.026157,  0.877258,  19.058994 );

/* BT.709 (i.e. HD) -> BT.470-2 System B, G (i.e. SD Pal) */
static const BVDC_P_CscCoeffs  s_CMP_HDYCbCr_to_PalSDYCbCr = BVDC_P_MAKE_CMP_CSC
    (  1.000000,  0.100597,  0.167144, -34.270816,
      -0.000000,  1.003373, -0.093371,  11.519780,
       0.000000, -0.079020,  0.939705,  17.832324 );

/* SMPTE 240M (i.e. 1987 ATSC HD) -> BT.470-2 System B, G (i.e. SD Pal) */
static const BVDC_P_CscCoeffs  s_CMP_240MHDYCbCr_to_PalSDYCbCr = BVDC_P_MAKE_CMP_CSC
    (  1.000000,  0.100476,  0.154524, -32.640077,
      -0.000000,  0.993840, -0.086589,  11.871852,
       0.000000, -0.073855,  0.868917,  26.232166 );

/* FCC (i.e. 1953 NTSC) -> BT.470-2 System B, G (i.e. SD Pal) */
static const BVDC_P_CscCoeffs  s_CMP_FccSDYCbCr_to_PalSDYCbCr = BVDC_P_MAKE_CMP_CSC
    (  1.011153,  0.110831,  0.252085, -46.631688,
       0.052033,  1.126338, -0.146231,   1.713843,
       0.027261, -0.099765,  1.420544, -41.495877 );

/* BT.709 (i.e. HD) -> to xvYCC BT.601 (i.e. SD) */
static const BVDC_P_CscCoeffs  s_CMP_HDYCbCr_to_XvYCCSDYCbCr = BVDC_P_MAKE_CMP_CSC
    (  1.000000,  0.100178,  0.191707, -37.361314,
       0.000000,  0.989849, -0.110711,  15.470362,
      -0.000000, -0.073079,  0.983251,  11.497916 );

/* FCC (i.e. 1953 NTSC) -> to xvYCC BT.601 (i.e. SD) */
static const BVDC_P_CscCoeffs  s_CMP_FccSDYCbCr_to_XvYCCSDYCbCr = BVDC_P_MAKE_CMP_CSC
    (  1.011956,  0.110070,  0.289208, -51.298935,
       0.050706,  1.111377, -0.172368,   6.995471,
       0.029052, -0.093619,  1.486324, -50.731057 );

/* BT.470-2 System B, G (i.e. SD Pal) -> xvYCC BT.601 (i.e. SD) */
static const BVDC_P_CscCoeffs  s_CMP_PalSDYCbCr_to_XvYCCSDYCbCr = BVDC_P_MAKE_CMP_CSC
    (  1.000000,  0.001655,  0.026303, -3.578607,
       0.000000,  0.984950, -0.019948,  4.479666,
      -0.000000,  0.009646,  1.047299, -7.288983 );

/* SMPTE 170M (i.e. modern SD NTSC)) -> xvYCC BT.601 (i.e. SD) */
static const BVDC_P_CscCoeffs  s_CMP_NtscSDYCbCr_to_XvYCCSDYCbCr = BVDC_P_MAKE_CMP_CSC
    (  1.000000,  0.025081, -0.006850,  -2.333618,
       0.000000,  0.992359,  0.004435,   0.410342,
      -0.000000, -0.017681,  0.918967,  12.635360 );

/* SMPTE 240M (i.e. 1987 ATSC HD) -> xvYCC BT.601 (i.e. SD) */
static const BVDC_P_CscCoeffs  s_CMP_240MHDYCbCr_to_XvYCCSDYCbCr = BVDC_P_MAKE_CMP_CSC
    (  1.000000,  0.100178,  0.177236, -35.509050,
      -0.000000,  0.980357, -0.102619,  15.649576,
      -0.000000, -0.067762,  0.909181,  20.298460 );

/* xvYCC BT.601 (i.e. SD) -> to BT.709 (i.e. HD) */
static const BVDC_P_CscCoeffs  s_CMP_XvYCCSDYCbCr_to_HDYCbCr = BVDC_P_MAKE_CMP_CSC
    (  1.000000, -0.116569, -0.208098,  41.557373,
       0.000000,  1.018724,  0.114705, -17.078894,
      -0.000000,  0.075715,  1.025559, -12.963136 );

/* xvYCC BT.601 (i.e. SD) -> SMPTE 170M (i.e. modern SD NTSC)) */
static const BVDC_P_CscCoeffs  s_CMP_XvYCCSDYCbCr_to_NtscSDYCbCr = BVDC_P_MAKE_CMP_CSC
    (  1.000000, -0.025140,  0.007575,   2.248217,
      -0.000000,  1.007613, -0.004863,  -0.352023,
       0.000000,  0.019386,  1.088085, -13.756298 );

/* xvYCC BT.601 (i.e. SD) -> BT.470-2 System B, G (i.e. SD Pal) */
static const BVDC_P_CscCoeffs  s_CMP_XvYCCSDYCbCr_to_PalSDYCbCr = BVDC_P_MAKE_CMP_CSC
    (  1.000000, -0.001434, -0.025143,  3.401765,
       0.000000,  1.015090,  0.019334, -4.406337,
       0.000000, -0.009349,  0.954659,  7.000373 );

/* 472M (i.e. SD Pal) -> BT2020 (i.e. UHD) */
static const BVDC_P_CscCoeffs s_CMP_PalSDYCbCr_to_UHDYCbCr = BVDC_P_MAKE_CMP_CSC
    (  1.000000, -0.114060, -0.220759,  42.856807,
      -0.000458,  0.876936,  0.091411,   4.058961,
       0.001246,  0.060051,  0.716182,  28.622240 );

/* 240M (i.e. 1987 ATSC HD) -> BT2020 (i.e. UHD) */
static const BVDC_P_CscCoeffs s_CMP_240MHDYCbCr_to_UHDYCbCr = BVDC_P_MAKE_CMP_CSC
    (  1.000000,  -0.000795,   0.000649,   0.018744,
       0.000000,   0.866564,  -0.009478,  18.292975,
       0.000000,   0.015509,   0.615175,  47.272494 );

/* BT601 (i.e. xvYCC BT.601) -> BT2020 (i.e. UHD) */
static const BVDC_P_CscCoeffs s_CMP_XvYCCSDYCbCr_to_UHDYCbCr = BVDC_P_MAKE_CMP_CSC
    (  1.000000,   0.001252,   0.000420,  -0.214032,
       0.048687,   0.982230,  -0.020016,   4.057565,
       0.019779,   0.006763,   1.005020,  -1.824650 );

/* 170M (i.e. modern SD NTSC) -> BT2020 (i.e. UHD) */
static const BVDC_P_CscCoeffs s_CMP_NtscSDYCbCr_to_UHDYCbCr = BVDC_P_MAKE_CMP_CSC
    (  1.000000,  -0.085911,  -0.233380,  40.869177,
       0.000000,   0.881816,   0.103899,   1.828532,
       0.000000,   0.044681,   0.628677,  41.810198 );

/* BT2020 (i.e. UHD) -> BT601 (i.e. xvYCC BT.601)*/
static const BVDC_P_CscCoeffs s_CMP_UHDYCbCr_to_XvYCCSDYCbCr = BVDC_P_MAKE_CMP_CSC
    (  1.000071,  -0.001272,  -0.000444,   0.218397,
      -0.049966,   1.018015,   0.020296,  -4.104323,
      -0.019346,  -0.006825,   0.994877,   1.838855 );

/* BT2020 (i.e. UHD) -> 170M (i.e. modern SD NTSC) */
static const BVDC_P_CscCoeffs s_CMP_UHDYCbCr_to_NtscSDYCbCr = BVDC_P_MAKE_CMP_CSC
    (  1.000000,   0.079279,   0.358121,  -55.987253,
       0.000000,   1.143600,  -0.188998,    5.810930,
       0.000000,  -0.081277,   1.604074,  -66.918015 );

/* BT2020 (i.e. UHD) -> 472M (i.e. SD Pal) */
static const BVDC_P_CscCoeffs s_CMP_UHDYCbCr_to_PalSDYCbCr = BVDC_P_MAKE_CMP_CSC
    (  0.999684,   0.109884,   0.294122,  -51.707697,
       0.000710,   1.150467,  -0.146623,   -0.503457,
      -0.001799,  -0.096657,   1.408076,  -39.832873 );

/* SDR BT2020 (i.e. UHD) -> BT709 (i.e. HD) */
static const BVDC_P_CscCoeffs s_CMP_UHDYCbCr_to_HDYCbCr = BVDC_P_MAKE_CMP_CSC
    (  1.000000,   0.000015, -0.000130,    0.014753,
       0.000000,   1.143195,  0.016617,  -20.455953,
       0.000000,  -0.021793,  1.502906,  -61.582478 );

/* SDR BT709 (i.e. HD) -> BT2020 (i.e. UHD) */
static const BVDC_P_CscCoeffs s_CMP_HDYCbCr_to_UHDYCbCr = BVDC_P_MAKE_CMP_CSC
    (  1.000000,  -0.000011,   0.000087,  -0.009648,
       0.000000,   0.874557,  -0.009669,  17.294425,
       0.000000,   0.012682,   0.665237,  41.226373 );

/* HDR BT2020 (i.e. UHD) -> BT709 (i.e. HD) */
static const BVDC_P_CscCoeffs s_CMP_HDR_UHDYCbCr_to_HDYCbCr = BVDC_P_MAKE_CMP_CSC
    (  1.000000,   0.000015,  -0.000130,   0.014753,
       0.000000,   1.143195,   0.016617, -20.455953,
       0.000000,  -0.025639,   1.768125, -95.038210 );

/* HDR BT709 (i.e. HD) -> BT2020 (i.e. UHD) */
static const BVDC_P_CscCoeffs s_CMP_HDR_HDYCbCr_to_UHDYCbCr = BVDC_P_MAKE_CMP_CSC
    (  1.000000,  -0.000011,  0.000074,   -0.007985,
       0.000000,   0.874557, -0.008219,   17.108772,
       0.000000,   0.012682,  0.565452,   53.998927 );

/* identity matrix */
static const BVDC_P_CscCoeffs s_CMP_Identity_YCbCr_to_YCbCr = BVDC_P_MAKE_CMP_CSC
    (  1.000000,  0.000000,  0.000000,  0.000000,
       0.000000,  1.000000,  0.000000,  0.000000,
       0.000000,  0.000000,  1.000000,  0.000000 );


/***************************************************************************
 * video surface color space conversion matrices Set II
 * Note:
 *  - these csc matrices DON'T MATCH the color primaries of different standards;
 *  - we do not do non-linear gamma correction;
 *  - the CMP matrix is for [YCbCr] -> [YCbCr];
 *  - PR10417: coeffs are in two's complement fixed point format;
 * -----------------------------------------------------
 * Color Space Standards:
 *  - ITU-R BT.709, i.e. ATSC HD or PAL HD;
 *  - FCC, i.e. NTSC SD 1953 standard, with ITU-R BT.470-2 System M color primaries;
 *  - ITU-R BT.470-2 System B, G, i.e. PAL SD;
 *  - SMPTE 170M, i.e. modern NTSC SD;
 *  - SMPTE 240M (1987 standard), the legacy ATSC HD;
 **************************************************************************/

/* Group 0: xyz -> BT.709 (i.e. HD) */
/* SMPTE 170M (i.e. modern SD NTSC) -> BT.709 (i.e. HD) */
static const BVDC_P_CscCoeffs  s_CMP_NtscSDYCbCr_to_HDYCbCr_II = BVDC_P_MAKE_CMP_CSC
    (  1.000000, -0.116569, -0.208098,  41.557373,
       0.000000,  1.018724,  0.114705, -17.078894,
       0.000000,  0.075715,  1.025559, -12.963136 );

/* BT.470-2 System B, G (i.e. SD Pal) -> BT.709 (i.e. HD),
   the same as  SMPTE 170M (i.e. modern SD NTSC) -> BT.709 (i.e. HD) */

/* SMPTE 240M (i.e. 1987 ATSC HD) -> BT.709 (i.e. HD) */
static const BVDC_P_CscCoeffs  s_CMP_240MHDYCbCr_to_HDYCbCr_II = BVDC_P_MAKE_CMP_CSC
    (  1.000000, -0.028780, -0.006178,  4.474660,
      -0.000000,  1.000374,  0.003405, -0.483788,
      -0.000000,  0.018694,  1.004346, -2.949129 );

/* FCC (i.e. 1953 NTSC) -> BT.709 (i.e. HD),
   the same as  SMPTE 170M (i.e. modern SD NTSC) -> BT.709 (i.e. HD) */

/* Group 1: xyz -> SMPTE 170M (i.e. modern SD NTSC) */
/* BT.709 (i.e. HD) -> SMPTE 170M (i.e. modern SD NTSC) */
static const BVDC_P_CscCoeffs  s_CMP_HDYCbCr_to_NtscSDYCbCr_II = BVDC_P_MAKE_CMP_CSC
    (  1.000000,  0.100178,  0.191707, -37.361314,
       0.000000,  0.989849, -0.110711,  15.470362,
       0.000000, -0.073079,  0.983251,  11.497916 );

/* BT.470-2 System B, G (i.e. SD Pal) -> SMPTE 170M (i.e. modern SD NTSC),
   the same as  SMPTE 170M (i.e. modern SD NTSC) -> SMPTE 170M (i.e. modern SD NTSC) */

/* SMPTE 240M (i.e. 1987 ATSC HD) -> SMPTE 170M (i.e. modern SD NTSC) */
static const BVDC_P_CscCoeffs  s_CMP_240MHDYCbCr_to_NtscSDYCbCr_II = BVDC_P_MAKE_CMP_CSC
    (  1.000000,  0.075019,  0.186703, -33.500488,
      -0.000000,  0.988150, -0.107822,  15.317986,
      -0.000000, -0.054726,  0.987276,   8.633535 );

/* FCC (i.e. 1953 NTSC) -> SMPTE 170M (i.e. modern SD NTSC),
   the same as  SMPTE 170M (i.e. modern SD NTSC) -> SMPTE 170M (i.e. modern SD NTSC) */

/* Group 2: xyz -> BT.470-2 System B, G (i.e. SD Pal) */
/* xyz -> BT.709 (i.e. HD),
   the same as xyz -> BT.470-2 System B, G (i.e. SD Pal) */

/* non color primary matching matrix should never be used when UHD is involved */
#if BVDC_P_TRY_MATRIX_II_FOR_UHD
/* 240M (i.e. 1987 ATSC HD) -> BT2020 (i.e. UHD) */
static const BVDC_P_CscCoeffs s_CMP_240MHDYCbCr_to_UHDYCbCr_II = BVDC_P_MAKE_CMP_CSC
    (  1.000000, -0.044355,  0.088837,  -5.693701,
       0.000000,  0.994668, -0.048297,   6.864513,
       0.000000,  0.030766,  1.007144,  -4.852506 );

/* BT601 (i.e. xvYCC BT.601) -> BT2020 (i.e. UHD) */
static const BVDC_P_CscCoeffs s_CMP_XvYCCSDYCbCr_to_UHDYCbCr_II = BVDC_P_MAKE_CMP_CSC
    (  1.000000, -0.125382, -0.113292,  30.550350,
       0.000000,  1.010016,  0.061592,  -9.165859,
       0.000000,  0.086969,  1.029350, -14.888815 );

/* BT 472M (i.e. SD Pal) -> BT2020 (i.e. UHD) */
static const BVDC_P_CscCoeffs s_CMP_PalSDYCbCr_to_UHDYCbCr_II = BVDC_P_MAKE_CMP_CSC
    (  1.000000, -0.125382, -0.113292,  30.550350,
      -0.000000,  1.010016,  0.061592,  -9.165859,
      -0.000000,  0.086969,  1.029350, -14.888815 );

/*170M (i.e moden SD) -> BT2020 (i.e. UHD) */
static const BVDC_P_CscCoeffs s_CMP_NtscSDYCbCr_to_UHDYCbCr_II = BVDC_P_MAKE_CMP_CSC
    (  1.000000, -0.125382, -0.113292,  30.550350,
       0.000000,  1.010016,  0.061592,  -9.165859,
       0.000000,  0.086969,  1.029350, -14.888815 );

/* BT709 (i.e HD) -> BT2020 (i.e. UHD) */
static const BVDC_P_CscCoeffs s_CMP_HDYCbCr_to_UHDYCbCr_II = BVDC_P_MAKE_CMP_CSC
    (  1.000000, -0.016590,  0.094162,  -9.929200,
       0.000000,  0.995306, -0.051192,   7.153340,
       0.000000,  0.011507,  1.002637,  -1.810461 );

/* BT2020 (i.e. UHD) -> 170M (i.e. modern SD NTSC) */
static const BVDC_P_CscCoeffs s_CMP_UHDYCbCr_to_NtscSDYCbCr_II = BVDC_P_MAKE_CMP_CSC
    (  1.000000,  0.115256,  0.103166, -27.957919,
       0.000000,  0.995211, -0.059549,   8.235343,
       0.000000, -0.084085,  0.976518,  13.768493 );

/*  BT2020 (i.e. UHD) -> BT601 (i.e. xvYCC BT.601) */
static const BVDC_P_CscCoeffs s_CMP_UHDYCbCr_to_XvYCCSDYCbCr_II = BVDC_P_MAKE_CMP_CSC
    (  1.000000,  0.115256,  0.103166, -27.957919,
       0.000000,  0.995211, -0.059549,   8.235343,
       0.000000, -0.084085,  0.976518,  13.768493 );

/*  BT2020 (i.e. UHD) -> 240M (i.e. 1987 ATSC HD) */
static const BVDC_P_CscCoeffs s_CMP_UHDYCbCr_to_240MHDYCbCr_II = BVDC_P_MAKE_CMP_CSC
    (  1.000000,  0.047251, -0.085941,   4.952314,
       0.000000,  1.003872,  0.048140,  -6.657492,
       0.000000, -0.030666,  0.991436,   5.021459 );

/*  BT2020 (i.e. UHD) -> 472M (SD Pal) */
static const BVDC_P_CscCoeffs s_CMP_UHDYCbCr_to_PalSDYCbCr_II = BVDC_P_MAKE_CMP_CSC
    (  1.000000,  0.115256,  0.103166,  -27.957919,
       0.000000,  0.995211, -0.059549,    8.235343,
       0.000000, -0.084085,  0.976518,   13.768493 );

/*  BT2020 (i.e. UHD) -> BT709 (i.e. HD) */
static const BVDC_P_CscCoeffs s_CMP_UHDYCbCr_to_HDYCbCr_II = BVDC_P_MAKE_CMP_CSC
    (  1.000000,  0.017744, -0.093008,  9.633887,
       0.000000,  1.004123,  0.051267, -7.090019,
       0.000000, -0.011524,  0.996782,  1.887072 );
#else /* #ifdef BVDC_P_TRY_MATRIX_II_FOR_UHD */
#define s_CMP_240MHDYCbCr_to_UHDYCbCr_II   s_CMP_240MHDYCbCr_to_UHDYCbCr
#define s_CMP_XvYCCSDYCbCr_to_UHDYCbCr_II  s_CMP_XvYCCSDYCbCr_to_UHDYCbCr
#define s_CMP_PalSDYCbCr_to_UHDYCbCr_II    s_CMP_PalSDYCbCr_to_UHDYCbCr
#define s_CMP_NtscSDYCbCr_to_UHDYCbCr_II   s_CMP_NtscSDYCbCr_to_UHDYCbCr
#define s_CMP_HDYCbCr_to_UHDYCbCr_II       s_CMP_HDYCbCr_to_UHDYCbCr
#define s_CMP_UHDYCbCr_to_NtscSDYCbCr_II   s_CMP_UHDYCbCr_to_NtscSDYCbCr
#define s_CMP_UHDYCbCr_to_XvYCCSDYCbCr_II  s_CMP_UHDYCbCr_to_XvYCCSDYCbCr
#define s_CMP_UHDYCbCr_to_240MHDYCbCr_II   s_CMP_UHDYCbCr_to_240MHDYCbCr
#define s_CMP_UHDYCbCr_to_PalSDYCbCr_II    s_CMP_UHDYCbCr_to_PalSDYCbCr
#define s_CMP_UHDYCbCr_to_HDYCbCr_II       s_CMP_UHDYCbCr_to_HDYCbCr
#endif /* #if BVDC_P_TRY_MATRIX_II_FOR_UHD */

#if (BVDC_P_SUPPORT_CMP_NON_LINEAR_CSC!=0)
/* 7271 A0 CMP_1, 7439 B0 B1 B2 C0 ... */

/***************************************************************************
 * video surface color space conversion matrices Set MA and MC for CMPs with
 * non-linear conversion HW
 * Note:
 *  - The color space standards are devided into 2 groups: the BT2020 group
 *    and the non BT2020 group.
 *  - The BT2020 group has NCL and CL. They have the same primary and white
 *    point. But the matrix between R'G'B' and YCbCr are derived differently.
 *  - The non Bt.2020 group has all standards before BT2020. Their primary
 *    and white point are all similar, conversion between them can be well
 *    approximated with a single matrix (i.e. the non-linear process can be
 *    ignored).
 *  - The Non-linear color space converter HW can transfer from BT709 R'G'B'
 *    to BT2020 R'G'B'; from NCL YCbCr, CL YCbCr, and BT2020 R'G'B' to BT709
 *    or BT2020 R'G'B'.
 *  - MA and MC work together with the non-linear converter HW to convert any
 *    color space to limited range BT2020 NCL YCbCr, or any limited range
 *    non-BT2020 YCbCr.
 *  - VEC will further convert from the output of MC to desired format.
 *  - When BT2020 CL YCbCr is needed, output of MC is limited range BT2020
 *    NCL YCbCr, DVI_CSC converts it to BT2020 R'G'B', and the following
 *    non-linear converter HW further transfers it to BT2020 CL YCbCr
 * -----------------------------------------------------
 * Color Space Standards:
 *  - ITU-R BT.709, i.e. ATSC HD or PAL HD;
 *  - FCC, i.e. NTSC SD 1953 standard, with ITU-R BT.470-2 System M color primaries;
 *  - ITU-R BT.470-2 System B, G, i.e. PAL SD;
 *  - SMPTE 170M, i.e. modern NTSC SD;
 *  - SMPTE 240M (1987 standard), the legacy ATSC HD;
 *  - BT2020 NCL, UHD
 *  - BT2020 CL, UHD
 **************************************************************************/

/***************************************************************************
 * MA
 */

/* BT.709 (i.e. HD) YCbCr -> RGB Primaries */
static const BVDC_P_CscCoeffs s_CMP_HdYCbCr_to_BT709RGB = BVDC_P_MAKE_CMP_CSC
    (  1.168950,  0.000000,  1.799682, -249.062527,
       1.168950, -0.214073, -0.535094,   77.190243,
       1.168950,  2.120703,  0.000000, -290.153216 );

/* 170M (i.e moden Ntsc SD) YCbCr -> BT.709 (i.e. HD) R'G'B' */
static const BVDC_P_CscCoeffs s_CMP_NtscSdYCbCr_to_BT709RGB = BVDC_P_MAKE_CMP_CSC
    (  1.168950,  0.001075,  1.464459, -206.291588,
       1.168950, -0.346562, -0.759761,  122.906169,
       1.168950,  2.038995,  0.000968, -279.818453  );

/* 472M (i.e Pal SD) YCbCr -> BT.709 (i.e. HD) R'G'B' */
static const BVDC_P_CscCoeffs s_CMP_PalSdYCbCr_to_BT709RGB = BVDC_P_MAKE_CMP_CSC
    (  1.172936,  0.017306,  1.714167, -240.395562,
       1.167872, -0.392936, -0.815404,  135.981513,
       1.167882,  1.994798, -0.009616, -272.789425 );

/* 240M (i.e. 1987 ATSC HD) YCbCr -> BT.709 (i.e. HD) R'G'B' */
static const BVDC_P_CscCoeffs s_CMP_240MHdYCbCr_to_BT709RGB = BVDC_P_MAKE_CMP_CSC
    (  1.168950,  0.008449,  1.664915, -232.893802,
       1.168950, -0.215840, -0.494068,   72.164925,
       1.168950,  2.100492, -0.000540, -287.496988 );

/* FCC (i.e. 1953 NTSC SD) YCbCr -> BT.709 (i.e. HD) R'G'B' */
static const BVDC_P_CscCoeffs s_CMP_FccSdYCbCr_to_BT709RGB = BVDC_P_MAKE_CMP_CSC
    (  1.229483, -0.021351,  2.719926, -365.089315,
       1.139184, -0.233738, -0.807431,  115.042689,
       1.285562,  2.378239, -0.010828, -323.597494 );

/* XvYCCSD (i.e. xvYCC BT.601) YCbCr -> BT.709 (i.e. HD) R'G'B' */
static const BVDC_P_CscCoeffs s_CMP_XvYCCSdYCbCr_to_BT709RGB = BVDC_P_MAKE_CMP_CSC
    (  1.168950,  0.000006,  1.602516, -223.826139,
       1.168950, -0.394864, -0.816486,  136.349644,
       1.168950,  2.024130, -0.000003, -277.791453 );

/* BT.2020 (i.e. UHD) YCbCr -> RGB Primaries */
const BVDC_P_CscCoeffs s_CMP_UhdYCbCr_to_BT2020RGB = BVDC_P_MAKE_CMP_CSC
    (  1.168950,  0.000000,  1.685257, -234.416111,
       1.168950, -0.188061, -0.652975,   88.949376,
       1.168950,  2.150171,  0.000000, -293.925139 );

/***************************************************************************
 * MC
 */

/* BT.709 (i.e. HD) R'G'B' ->  BT.709 (i.e. HD) YCbCr */
static const BVDC_P_CscCoeffs s_CMP_BT709RGB_to_HdYCbCr = BVDC_P_MAKE_CMP_CSC
    (  0.181906,  0.611804,  0.061758,   16.000000,
      -0.100268, -0.337232,  0.437500,  128.000000,
       0.437500, -0.397386, -0.040114,  128.000000 );

/* BT.709 (i.e. HD) R'G'B' -> 170M (i.e moden Ntsc SD) YCbCr */
static const BVDC_P_CscCoeffs s_CMP_BT709RGB_to_NtscSdYCbCr = BVDC_P_MAKE_CMP_CSC
    (  0.262197,  0.496284,  0.096988,   16.000000,
      -0.152227, -0.289903,  0.442130,  128.000000,
       0.473259, -0.395563, -0.077697,  128.000000 );

/* BT.709 (i.e. HD) R'G'B' -> 472M (i.e Pal SD) YCbCr */
static const BVDC_P_CscCoeffs s_CMP_BT709RGB_to_PalSdYCbCr = BVDC_P_MAKE_CMP_CSC
    (  0.243560,  0.501559,  0.110080,   16.000000,
      -0.142304, -0.300979,  0.443891,  128.000000,
       0.417802, -0.339846, -0.079766,  128.000000 );

/* BT.709 (i.e. HD) R'G'B' -> XvYCCSD (i.e. xvYCC BT.601) YCbCr */
static const BVDC_P_CscCoeffs s_CMP_BT709RGB_to_XvYCCSdYCbCr = BVDC_P_MAKE_CMP_CSC
    (  0.255702,  0.501865,  0.097902,   16.000013,
      -0.147669, -0.289831,  0.437500,  127.999975,
       0.437499, -0.366083, -0.071416,  128.000064 );

/* RGB Primaries -> BT.2020 (i.e. UHD) YCbCr */
static const BVDC_P_CscCoeffs s_CMP_BT2020RGB_to_UhdYCbCr = BVDC_P_MAKE_CMP_CSC
    (  0.224732,  0.580008,  0.050729,   16.000000,
      -0.122176, -0.315324,  0.437500,  128.000000,
       0.437500, -0.402312, -0.035188,  128.000000 );

/****************************** Non-linear Set *********************************/
static const BVDC_P_CscCoeffs *const s_aCMP_MA_TO_UHD_MatrixTbl[] =
{
    &s_CMP_HdYCbCr_to_BT709RGB,       /* BVDC_P_MatrixCoeffs_eBt709 */
    &s_CMP_NtscSdYCbCr_to_BT709RGB,   /* BVDC_P_MatrixCoeffs_eSmpte170M */
    &s_CMP_PalSdYCbCr_to_BT709RGB,    /* BVDC_P_MatrixCoeffs_eBt470_2_BG */
    &s_CMP_Identity_YCbCr_to_YCbCr,   /* BVDC_P_MatrixCoeffs_eBt2020_NCL */
    &s_CMP_Identity_YCbCr_to_YCbCr,   /* BVDC_P_MatrixCoeffs_eBt2020_CL */
    &s_CMP_XvYCCSdYCbCr_to_BT709RGB,  /* BVDC_P_MatrixCoeffs_eXvYcc601 */
    &s_CMP_HdYCbCr_to_BT709RGB,       /* BVDC_P_MatrixCoeffs_eXvYcc709 */
    &s_CMP_FccSdYCbCr_to_BT709RGB,    /* BVDC_P_MatrixCoeffs_eFcc */
    &s_CMP_240MHdYCbCr_to_BT709RGB,   /* BVDC_P_MatrixCoeffs_eSmpte240M */
};

static const BVDC_P_CscCoeffs *const s_aCMP_BT709RGB_to_YCbCr_MatrixTbl[] =
{
    &s_CMP_BT709RGB_to_HdYCbCr,       /* BVDC_P_MatrixCoeffs_eBt709 */
    &s_CMP_BT709RGB_to_NtscSdYCbCr,   /* BVDC_P_MatrixCoeffs_eSmpte170M */
    &s_CMP_BT709RGB_to_PalSdYCbCr,    /* BVDC_P_MatrixCoeffs_eBt470_2_BG */
    NULL,                             /* BVDC_P_MatrixCoeffs_eBt2020_NCL */
    NULL,                             /* BVDC_P_MatrixCoeffs_eBt2020_CL */
    &s_CMP_BT709RGB_to_XvYCCSdYCbCr   /* BVDC_P_MatrixCoeffs_eXvYcc601 */
};

#endif /* #if (BVDC_P_SUPPORT_CMP_NON_LINEAR_CSC!=0) */

/***************************************************************************
 * color matrix (YCbCr->RGB) for BVDC_P_Csc_ApplyAttenuationRGB_isr
 * -----------------------------------------------------
 * Color Space Standards:
 *  - ITU-R BT.709, i.e. ATSC HD or PAL HD;
 *  - FCC, i.e. NTSC SD 1953 standard, with ITU-R BT.470-2 System M color primaries;
 *  - ITU-R BT.470-2 System B, G, i.e. PAL SD;
 *  - SMPTE 170M, i.e. modern NTSC SD;
 *  - SMPTE 240M (1987 standard), the legacy ATSC HD;
 **************************************************************************/

/* BT.709 (i.e. HD) YCbCr -> RGB Primaries */
static const BVDC_P_CscCoeffs s_CMP_HDYCbCr_to_RGBPrim = BVDC_P_MAKE_CMP_CSC
    (  1.168950,  0.000000,  1.799682, -249.062527,
       1.168950, -0.214073, -0.535094,   77.190243,
       1.168950,  2.120703,  0.000000, -290.153216 );

/* FCC (i.e. 1953 NTSC) -> RGB Primaries,
   the same as  SMPTE 170M (i.e. modern SD NTSC) */

/* BT.470-2 System B, G (i.e. SD Pal) -> RGB Primaries,
   the same as  SMPTE 170M (i.e. modern SD NTSC) */

/* SMPTE 170M (i.e. modern SD NTSC) YCbCr -> RGB Primaries */
static const BVDC_P_CscCoeffs s_CMP_NtscSDYCbCr_to_RGBPrim = BVDC_P_MAKE_CMP_CSC
    (  1.168950,  0.000000,  1.602425, -223.813572,
       1.168950, -0.394860, -0.816582,  136.361359,
       1.168950,  2.024147, -0.000000, -277.794001 );

/* SMPTE 240M (i.e. 1987 ATSC HD) -> RGB Primaries */
static const BVDC_P_CscCoeffs s_CMP_240MHDYCbCr_to_RGBPrim = BVDC_P_MAKE_CMP_CSC
    (  1.168950,  0.000000,  1.800283, -249.139370,
       1.168950, -0.257799, -0.545371,   84.102524,
       1.168950,  2.087854,  0.000000, -285.948536 );

/* BT.2020 (i.e. UHD) YCbCr -> RGB Primaries */
static const BVDC_P_CscCoeffs s_CMP_UHDYCbCr_to_RGBPrim = BVDC_P_MAKE_CMP_CSC
    (  1.168950,  0.000000,  1.685257, -234.416111,
       1.168950, -0.188061, -0.652975,   88.949376,
       1.168950,  2.150171,  0.000000, -293.925139 );

/***************************************************************************
 * color matrix (RGB->YCbCr) for BVDC_P_Csc_ApplyAttenuationRGB_isr
 * -----------------------------------------------------
 * Color Space Standards:
 *  - ITU-R BT.709, i.e. ATSC HD or PAL HD;
 *  - ITU-R BT.470-2 System B, G, i.e. PAL SD;
 *  - SMPTE 170M, i.e. modern NTSC SD;
 *  - Custom Panel, with source white points for proper functionality of CCB;
 **************************************************************************/

/* RGB Primaries -> BT.709 (i.e. HD) YCbCr */
static const BVDC_P_CscCoeffs s_CMP_RGBPrim_to_HDYCbCr = BVDC_P_MAKE_CMP_CSC
    (  0.181906,  0.611804,  0.061758,   16.000000,
      -0.100268, -0.337232,  0.437500,  128.000000,
       0.437500, -0.397386, -0.040114,  128.000000 );

/* RGB Primaries -> SMPTE 170M (i.e. modern SD NTSC) YCbCr */
static const BVDC_P_CscCoeffs s_CMP_RGBPrim_to_NtscSDYCbCr = BVDC_P_MAKE_CMP_CSC
    (  0.255733,  0.501839,  0.097896,   16.000000,
      -0.147686, -0.289814,  0.437500,  128.000000,
       0.437500, -0.366086, -0.071414,  128.000000 );

/* RGB Primaries -> BT.470-2 System B, G (i.e. SD Pal) YCbCr,
   the same as  SMPTE 170M (i.e. modern SD NTSC) */

/* MC matrices for Custom Panels, using source white points,
   can be replaced with panel specific matrices. */
/* RGB Primaries -> Custom Panel YCbCr, using BT.709 white points */
static const BVDC_P_CscCoeffs s_CMP_RGBPrim_to_CustomYCbCr_HDWp = BVDC_P_MAKE_CMP_CSC
    (  0.208948,  0.595680,  0.050842,   16.000000,
      -0.113611, -0.323889,  0.437500,  128.000000,
       0.437500, -0.403096, -0.034404,  128.000000 );

/* RGB Primaries -> Custom Panel YCbCr, using SMPTE 170M (i.e. modern SD NTSC) white points */
static const BVDC_P_CscCoeffs s_CMP_RGBPrim_to_CustomYCbCr_NtscWp = BVDC_P_MAKE_CMP_CSC
    (  0.217468,  0.582272,  0.055729,   16.000000,
      -0.118966, -0.318534,  0.437500,  128.000000,
       0.437500, -0.399285, -0.038215,  128.000000 );

/* RGB Primaries -> BT.2020 (i.e. UHD) YCbCr */
static const BVDC_P_CscCoeffs s_CMP_RGBPrim_to_UHDYCbCr = BVDC_P_MAKE_CMP_CSC
    (  0.224732,  0.580008,  0.050729,   16.000000,
      -0.122176, -0.315324,  0.437500,  128.000000,
       0.437500, -0.402312, -0.035188,  128.000000 );

/* TODO:  get real matrix for UHD
 */
#define s_CMP_FccSDYCbCr_to_UHDYCbCr         s_CMP_FccSDYCbCr_to_HDYCbCr
#define s_CMP_FccSDRGBPrim_to_BT2020RGBPrim  s_CMP_FccSDRGBPrim_to_BT709RGBPrim
#define s_CMP_BT2020RGBPrim_to_CustomRGBPrim s_CMP_BT709RGBPrim_to_CustomRGBPrim
#define s_CMP_RGBPrim_to_CustomYCbCr_UHDWp   s_CMP_RGBPrim_to_CustomYCbCr_HDWp

/****************************** Set One *********************************/
/* the compositor color matrices table WITH Color Primaries Matching */
static const BVDC_P_CscCoeffs *const s_aaCMP_YCbCr_MatrixTbl[][7] =
{
    /* BVDC_P_MatrixCoeffs_eBt709  */
    {
        &s_CMP_Identity_YCbCr_to_YCbCr,    /* 709 -> 709 */
        &s_CMP_HDYCbCr_to_NtscSDYCbCr,     /* 709 -> 170M */
        &s_CMP_HDYCbCr_to_PalSDYCbCr,      /* 709 -> 470_2_BG */
        &s_CMP_HDYCbCr_to_UHDYCbCr,        /* 709 -> 2020 NCL */
        &s_CMP_HDYCbCr_to_UHDYCbCr,        /* 709 -> 2020 CL */
        &s_CMP_HDYCbCr_to_XvYCCSDYCbCr,    /* 709 -> XvYCC601 */
        &s_CMP_Identity_YCbCr_to_YCbCr     /* 709 -> XvYCC709 */
    },

    /* BVDC_P_MatrixCoeffs_eSmpte170M */
    {
        &s_CMP_NtscSDYCbCr_to_HDYCbCr,      /* 170M -> 709 */
        &s_CMP_Identity_YCbCr_to_YCbCr,     /* 170M -> 170M */
        &s_CMP_NtscSDYCbCr_to_PalSDYCbCr,   /* 170M -> 470_2_BG */
        &s_CMP_NtscSDYCbCr_to_UHDYCbCr,     /* 170M -> 2020 NCL */
        &s_CMP_NtscSDYCbCr_to_UHDYCbCr,     /* 170M -> 2020 CL */
        &s_CMP_NtscSDYCbCr_to_XvYCCSDYCbCr, /* 170M -> XvYCC601 */
        &s_CMP_NtscSDYCbCr_to_HDYCbCr       /* 170M -> XvYCC709 */
    },

    /* BVDC_P_MatrixCoeffs_e470_2_BG */
    {
        &s_CMP_PalSDYCbCr_to_HDYCbCr,       /* 470_2_BG -> 709 */
        &s_CMP_PalSDYCbCr_to_NtscSDYCbCr,   /* 470_2_BG > 170M */
        &s_CMP_Identity_YCbCr_to_YCbCr,     /* 470_2_BG > 470_2_BG */
        &s_CMP_PalSDYCbCr_to_UHDYCbCr,      /* 470_2_BG > 2020 NCL */
        &s_CMP_PalSDYCbCr_to_UHDYCbCr,      /* 470_2_BG > 2020 CL */
        &s_CMP_PalSDYCbCr_to_XvYCCSDYCbCr,  /* 470_2_BG > XvYCC601 */
        &s_CMP_PalSDYCbCr_to_HDYCbCr        /* 470_2_BG > XvYCC709 */
    },

    /* BVDC_P_MatrixCoeffs_eBt2020_NCL */
    {
        &s_CMP_UHDYCbCr_to_HDYCbCr,        /* 2020 NCL -> 709 */
        &s_CMP_UHDYCbCr_to_NtscSDYCbCr,    /* 2020 NCL -> 170M */
        &s_CMP_UHDYCbCr_to_PalSDYCbCr,     /* 2020 NCL -> 470_2_BG */
        &s_CMP_Identity_YCbCr_to_YCbCr,    /* 2020 NCL -> 2020 NCL */
        &s_CMP_Identity_YCbCr_to_YCbCr,    /* 2020 NCL -> 2020 CL */
        &s_CMP_UHDYCbCr_to_XvYCCSDYCbCr,   /* 2020 NCL -> XvYCC601 */
        &s_CMP_UHDYCbCr_to_HDYCbCr         /* 2020 NCL -> XvYCC709 */
    },

    /* BVDC_P_MatrixCoeffs_eBt2020_CL */
    {
        &s_CMP_UHDYCbCr_to_HDYCbCr,        /* 2020 CL -> 709 */
        &s_CMP_UHDYCbCr_to_NtscSDYCbCr,    /* 2020 CL -> 170M */
        &s_CMP_UHDYCbCr_to_PalSDYCbCr,     /* 2020 CL -> 470_2_BG */
        &s_CMP_Identity_YCbCr_to_YCbCr,    /* 2020 CL -> 2020 NCL */
        &s_CMP_Identity_YCbCr_to_YCbCr,    /* 2020 CL -> 2020 CL */
        &s_CMP_UHDYCbCr_to_XvYCCSDYCbCr,   /* 2020 CL -> XvYCC601 */
        &s_CMP_UHDYCbCr_to_HDYCbCr         /* 2020 CL -> XvYCC709 */
    },

    /* BVDC_P_MatrixCoeffs_eXvYCC601 */
    {
        &s_CMP_XvYCCSDYCbCr_to_HDYCbCr,     /* XvYCC601 -> 709 */
        &s_CMP_XvYCCSDYCbCr_to_NtscSDYCbCr, /* XvYCC601 -> 170M */
        &s_CMP_XvYCCSDYCbCr_to_PalSDYCbCr,  /* XvYCC601 -> 470_2_BG */
        &s_CMP_XvYCCSDYCbCr_to_UHDYCbCr,    /* XvYCC601 -> 2020 NCL */
        &s_CMP_XvYCCSDYCbCr_to_UHDYCbCr,    /* XvYCC601 -> 2020 CL */
        &s_CMP_Identity_YCbCr_to_YCbCr,     /* XvYCC601 -> XvYCC601 */
        &s_CMP_XvYCCSDYCbCr_to_HDYCbCr      /* XvYCC601 -> XvYCC709 */
    },

    /* BVDC_P_MatrixCoeffs_eXvYCC709 */
    {
        &s_CMP_Identity_YCbCr_to_YCbCr,    /* XvYCC709 -> 709 */
        &s_CMP_HDYCbCr_to_NtscSDYCbCr,     /* XvYCC709 -> 170M */
        &s_CMP_HDYCbCr_to_PalSDYCbCr,      /* XvYCC709 -> 470_2_BG */
        &s_CMP_HDYCbCr_to_UHDYCbCr,        /* XvYCC709 -> 2020 NCL */
        &s_CMP_HDYCbCr_to_UHDYCbCr,        /* XvYCC709 -> 2020 CL */
        &s_CMP_HDYCbCr_to_XvYCCSDYCbCr,    /* XvYCC709 -> XvYCC601 */
        &s_CMP_Identity_YCbCr_to_YCbCr     /* XvYCC709 -> XvYCC709 */
    },

    /* BVDC_P_MatrixCoeffs_eFcc */
    {
        &s_CMP_FccSDYCbCr_to_HDYCbCr,       /* FCC -> 709 */
        &s_CMP_FccSDYCbCr_to_NtscSDYCbCr,   /* FCC -> 170M */
        &s_CMP_FccSDYCbCr_to_PalSDYCbCr,    /* FCC -> 470_2_BG */
        &s_CMP_FccSDYCbCr_to_UHDYCbCr,      /* FCC -> 2020 NCL */
        &s_CMP_FccSDYCbCr_to_UHDYCbCr,      /* FCC -> 2020 CL */
        &s_CMP_FccSDYCbCr_to_XvYCCSDYCbCr,  /* FCC -> XvYCC601 */
        &s_CMP_FccSDYCbCr_to_HDYCbCr        /* FCC -> XvYCC709 */
    },

    /* BVDC_P_MatrixCoeffs_eSmpte240M */
    {
        &s_CMP_240MHDYCbCr_to_HDYCbCr,      /* 240M -> 709 */
        &s_CMP_240MHDYCbCr_to_NtscSDYCbCr,  /* 240M -> 170M */
        &s_CMP_240MHDYCbCr_to_PalSDYCbCr,   /* 240M -> 470_2_BG */
        &s_CMP_240MHDYCbCr_to_UHDYCbCr,     /* 240M -> 2020 NCL */
        &s_CMP_240MHDYCbCr_to_UHDYCbCr,     /* 240M -> 2020 CL */
        &s_CMP_240MHDYCbCr_to_XvYCCSDYCbCr, /* 240M -> XvYCC601 */
        &s_CMP_240MHDYCbCr_to_HDYCbCr       /* 240M -> XvYCC709 */
    }
};

/****************************** Set Two *********************************/
/* the compositor color matrices table WITHOUT Color Primaries Matching */
static const BVDC_P_CscCoeffs * const s_aaCMP_YCbCr_MatrixTbl_II[][7] =
{
    /* BVDC_P_MatrixCoeffs_eBt709 */
    {
        &s_CMP_Identity_YCbCr_to_YCbCr,     /* 709 -> 709 */
        &s_CMP_HDYCbCr_to_NtscSDYCbCr_II,   /* 709 -> 170M */
        &s_CMP_HDYCbCr_to_NtscSDYCbCr_II,   /* 709 -> 470_2_BG */
        &s_CMP_HDYCbCr_to_UHDYCbCr_II,      /* 709 -> 2020 NCL */
        &s_CMP_HDYCbCr_to_UHDYCbCr_II,      /* 709 -> 2020 CL */
        &s_CMP_HDYCbCr_to_NtscSDYCbCr_II,   /* 709 -> xvYCC601 */
        &s_CMP_Identity_YCbCr_to_YCbCr      /* 709 -> xvYCC709 */
    },

    /* BVDC_P_MatrixCoeffs_eSmpte170M */
    {
        &s_CMP_NtscSDYCbCr_to_HDYCbCr_II,    /* 170M -> 709 */
        &s_CMP_Identity_YCbCr_to_YCbCr,      /* 170M -> 170M */
        &s_CMP_Identity_YCbCr_to_YCbCr,      /* 170M -> 470_2_BG */
        &s_CMP_NtscSDYCbCr_to_UHDYCbCr_II,   /* 170M -> 2020 NCL */
        &s_CMP_NtscSDYCbCr_to_UHDYCbCr_II,   /* 170M -> 2020 CL */
        &s_CMP_Identity_YCbCr_to_YCbCr,      /* 170M -> xvYCC601 */
        &s_CMP_NtscSDYCbCr_to_HDYCbCr_II     /* 170M -> xvYCC709 */
    },

    /* BVDC_P_MatrixCoeffs_eBt470_2_BG */
    {
        &s_CMP_NtscSDYCbCr_to_HDYCbCr_II,    /* 470_2_BG -> 709 */
        &s_CMP_Identity_YCbCr_to_YCbCr,      /* 470_2_BG -> 170M */
        &s_CMP_Identity_YCbCr_to_YCbCr,      /* 470_2_BG -> 470_2_BG */
        &s_CMP_NtscSDYCbCr_to_UHDYCbCr_II,   /* 470_2_BG -> 2020 NCL */
        &s_CMP_NtscSDYCbCr_to_UHDYCbCr_II,   /* 470_2_BG -> 2020 CL */
        &s_CMP_Identity_YCbCr_to_YCbCr,      /* 470_2_BG -> xvYCC601 */
        &s_CMP_NtscSDYCbCr_to_HDYCbCr_II     /* 470_2_BG -> xvYCC709 */
    },

    /* BVDC_P_MatrixCoeffs_eBt2020_NCL  */
    {
        &s_CMP_UHDYCbCr_to_HDYCbCr_II,       /* 2020 NCL -> 709 */
        &s_CMP_UHDYCbCr_to_NtscSDYCbCr_II,   /* 2020 NCL -> 170M */
        &s_CMP_UHDYCbCr_to_NtscSDYCbCr_II,   /* 2020 NCL -> 470_2_BG */
        &s_CMP_Identity_YCbCr_to_YCbCr,      /* 2020 NCL -> 2020 NCL */
        &s_CMP_Identity_YCbCr_to_YCbCr,      /* 2020 NCL -> 2020 CL */
        &s_CMP_UHDYCbCr_to_NtscSDYCbCr_II,   /* 2020 NCL -> xvYCC601 */
        &s_CMP_UHDYCbCr_to_HDYCbCr_II        /* 2020 NCL -> xvYCC709 */
    },

    /* BVDC_P_MatrixCoeffs_eBt2020_CL  */
    {
        &s_CMP_UHDYCbCr_to_HDYCbCr_II,       /* 2020 CL -> 709 */
        &s_CMP_UHDYCbCr_to_NtscSDYCbCr_II,   /* 2020 CL -> 170M */
        &s_CMP_UHDYCbCr_to_NtscSDYCbCr_II,   /* 2020 CL -> 470_2_BG */
        &s_CMP_Identity_YCbCr_to_YCbCr,      /* 2020 CL -> 2020 NCL */
        &s_CMP_Identity_YCbCr_to_YCbCr,      /* 2020 CL -> 2020 CL */
        &s_CMP_UHDYCbCr_to_NtscSDYCbCr_II,   /* 2020 CL -> xvYCC601 */
        &s_CMP_UHDYCbCr_to_HDYCbCr_II        /* 2020 CL -> xvYCC709 */
    },

    /* BVDC_P_MatrixCoeffs_eXvYcc601  */
    {
        &s_CMP_NtscSDYCbCr_to_HDYCbCr_II,   /* XvYCC_SD -> HD, XvYCC_709 */
        &s_CMP_Identity_YCbCr_to_YCbCr,     /* XvYCC_SD -> 170M */
        &s_CMP_Identity_YCbCr_to_YCbCr,     /* XvYCC_SD -> 470_2_BG */
        &s_CMP_XvYCCSDYCbCr_to_UHDYCbCr_II, /* XvYCC_SD -> 2020 NCL */
        &s_CMP_XvYCCSDYCbCr_to_UHDYCbCr_II, /* XvYCC_SD -> 2020 CL */
        &s_CMP_Identity_YCbCr_to_YCbCr,     /* XvYCC_SD -> XvYCC601 */
        &s_CMP_NtscSDYCbCr_to_HDYCbCr_II    /* XvYCC_SD -> XvYCC709 */
    },

    /* BVDC_P_MatrixCoeffs_eXvYcc709 */
    {
        &s_CMP_Identity_YCbCr_to_YCbCr,     /* xvYCC709 -> 709 */
        &s_CMP_HDYCbCr_to_NtscSDYCbCr_II,   /* xvYCC709 -> 170M */
        &s_CMP_HDYCbCr_to_NtscSDYCbCr_II,   /* xvYCC709 -> 470_2_BG */
        &s_CMP_HDYCbCr_to_UHDYCbCr_II,      /* xvYCC709 -> 2020 NCL */
        &s_CMP_HDYCbCr_to_UHDYCbCr_II,      /* xvYCC709 -> 2020 CL */
        &s_CMP_HDYCbCr_to_NtscSDYCbCr_II,   /* xvYCC709 -> xvYCC601 */
        &s_CMP_Identity_YCbCr_to_YCbCr      /* xvYCC709 -> xvYCC709 */
    },

    /* BVDC_P_MatrixCoeffs_eFcc */
    {
        &s_CMP_NtscSDYCbCr_to_HDYCbCr_II,    /* FCC -> 709 */
        &s_CMP_Identity_YCbCr_to_YCbCr,      /* FCC -> 170M */
        &s_CMP_Identity_YCbCr_to_YCbCr,      /* FCC -> 470_2_BG */
        &s_CMP_NtscSDYCbCr_to_UHDYCbCr_II,   /* FCC -> 2020 NCL */
        &s_CMP_NtscSDYCbCr_to_UHDYCbCr_II,   /* FCC -> 2020 CL */
        &s_CMP_Identity_YCbCr_to_YCbCr,      /* FCC -> xvYCC601 */
        &s_CMP_NtscSDYCbCr_to_HDYCbCr_II     /* FCC -> xvYCC709 */
    },

    /* BVDC_P_MatrixCoeffs_eSmpte240M */
    {
        &s_CMP_240MHDYCbCr_to_HDYCbCr_II,    /* 240M -> 709 */
        &s_CMP_240MHDYCbCr_to_NtscSDYCbCr_II,/* 240M -> 170M */
        &s_CMP_240MHDYCbCr_to_NtscSDYCbCr_II,/* 240M -> 470_2_BG */
        &s_CMP_240MHDYCbCr_to_UHDYCbCr_II,   /* 240M -> 2020 NCL */
        &s_CMP_240MHDYCbCr_to_UHDYCbCr_II,   /* 240M -> 2020 CL */
        &s_CMP_240MHDYCbCr_to_NtscSDYCbCr_II,/* 240M -> xvYCC601 */
        &s_CMP_240MHDYCbCr_to_HDYCbCr_II     /* 240M -> xvYCC709 */
    }
};

/* color matrix (YCbCr->RGB) for BVDC_P_Csc_ApplyAttenuationRGB_isr */
static const BVDC_P_CscCoeffs *const s_aCMP_YCbCr_to_RGBPrim_MatrixTbl[] =
{
    &s_CMP_HDYCbCr_to_RGBPrim,        /* BVDC_P_MatrixCoeffs_eBt709 */
    &s_CMP_NtscSDYCbCr_to_RGBPrim,    /* BVDC_P_MatrixCoeffs_eSmpte170M */
    &s_CMP_NtscSDYCbCr_to_RGBPrim,    /* BVDC_P_MatrixCoeffs_eBt470_2_BG */
    &s_CMP_UHDYCbCr_to_RGBPrim,       /* BVDC_P_MatrixCoeffs_eBt2020_NCL */
    &s_CMP_UHDYCbCr_to_RGBPrim,       /* BVDC_P_MatrixCoeffs_eBt2020_CL */
    &s_CMP_NtscSDYCbCr_to_RGBPrim,    /* BVDC_P_MatrixCoeffs_eXvYcc601 */
    &s_CMP_HDYCbCr_to_RGBPrim         /* BVDC_P_MatrixCoeffs_eXvYcc709 */
};

/* color matrix (RGB->YCbCr) for BVDC_P_Csc_ApplyAttenuationRGB_isr */
/* XvYCC output is handled like non-XvYCC output, and conversions are */
/* only to HD and SD */
static const BVDC_P_CscCoeffs *const s_aCMP_RGBPrim_to_YCbCr_MatrixTbl[] =
{
    &s_CMP_RGBPrim_to_HDYCbCr,        /* BVDC_P_MatrixCoeffs_eBt709 */
    &s_CMP_RGBPrim_to_NtscSDYCbCr,    /* BVDC_P_MatrixCoeffs_eSmpte170M */
    &s_CMP_RGBPrim_to_NtscSDYCbCr,    /* BVDC_P_MatrixCoeffs_eBt470_2_BG */
    &s_CMP_RGBPrim_to_UHDYCbCr,       /* BVDC_P_MatrixCoeffs_eBt2020_NCL */
    &s_CMP_RGBPrim_to_UHDYCbCr,       /* BVDC_P_MatrixCoeffs_eBt2020_CL */
    &s_CMP_RGBPrim_to_NtscSDYCbCr,    /* BVDC_P_MatrixCoeffs_eXvYcc601 */
    &s_CMP_RGBPrim_to_HDYCbCr         /* BVDC_P_MatrixCoeffs_eBt709 */
};

/***************************************************************************
 * Return the desired color space conversion for CSC in compositor.
 *
 */
void BVDC_P_Window_GetCscTable_isrsafe
    ( BVDC_P_CscCfg                   *pCscCfg,
      uint8_t                          ucSlotIdx,
      BVDC_Window_Handle               hWindow,
      BVDC_P_MatrixCoeffs              eInMatrixCoeffs )
{
    BVDC_Compositor_Handle hCompositor;
    bool bCscRgbMatching;
    BVDC_P_MatrixCoeffs eOutMatrixCoeffs;
    BVDC_WindowId eWinInCmp;

    pCscCfg->ucSlotIdx = ucSlotIdx;
    pCscCfg->ucRulBuildCntr = BVDC_P_RUL_UPDATE_THRESHOLD;

    hCompositor = hWindow->hCompositor;
    eWinInCmp = hWindow->eId - BVDC_P_CMP_GET_V0ID(hCompositor);
    eOutMatrixCoeffs = hCompositor->eOutMatrixCoeffs;
    bCscRgbMatching = hWindow->stCurInfo.bCscRgbMatching;

#if (BVDC_P_SUPPORT_CMP_NON_LINEAR_CSC!=0)

#if (BVDC_P_CMP_NON_LINEAR_CSC_VER >= BVDC_P_NL_CSC_VER_2)
/* 7271 A and up */
    if (hCompositor->bSupportEotfConv[eWinInCmp])
    {
        pCscCfg->pCscAbMA = s_aCMP_MA_Tbl[eInMatrixCoeffs];
        pCscCfg->pCscAbMB = s_aaCMP_MB_Tbl[eInMatrixCoeffs][eOutMatrixCoeffs];
        pCscCfg->stCscMC = *(s_aCMP_MC_Tbl[eOutMatrixCoeffs]);
        pCscCfg->ucXvYcc = ((eInMatrixCoeffs  == BVDC_P_MatrixCoeffs_eXvYcc601) ||
                            (eInMatrixCoeffs  == BVDC_P_MatrixCoeffs_eXvYcc709) ||
                            (eOutMatrixCoeffs == BVDC_P_MatrixCoeffs_eXvYcc601) ||
                            (eOutMatrixCoeffs == BVDC_P_MatrixCoeffs_eXvYcc709))? BVDC_P_IS_XVYCC : BVDC_P_NOT_XVYCC;
        pCscCfg->ucInputCL = (eInMatrixCoeffs == BVDC_P_MatrixCoeffs_eBt2020_CL)? BVDC_P_IS_CL_IN : BVDC_P_NOT_CL_IN;
        BDBG_MODULE_MSG(BVDC_NLCSC, ("win[%d] NL EOTF/CSC: SLOT[%d] VDC MatrixCffs %d->%d", hWindow->eId, ucSlotIdx, eInMatrixCoeffs, eOutMatrixCoeffs));
    }
    else
#endif /* #if (BVDC_P_CMP_NON_LINEAR_CSC_VER >= BVDC_P_NL_CSC_VER_2) */

    if (hCompositor->bSupportNLCsc[eWinInCmp])
    {
        /* is it poosible that CMP_1 output BT2020? */
        if (BVDC_P_IS_BT2020(eOutMatrixCoeffs))
        {
            pCscCfg->pCscMA = s_aCMP_MA_TO_UHD_MatrixTbl[eInMatrixCoeffs];
            if ((pCscCfg->pCscMA != &s_CMP_Identity_YCbCr_to_YCbCr) && !hCompositor->bSupportMACsc[eWinInCmp])
            {
                /* this cmp support NL csc, but not MA. However this usage case needs MA if
                   we want to do NL csc, so we have to fall back to use MC only */
                pCscCfg->ulNLCnv = BVDC_P_NL_CSC_CTRL_SEL_BYPASS;
                pCscCfg->pCscMA = &s_CMP_Identity_YCbCr_to_YCbCr;
                pCscCfg->stCscMC = *(s_aaCMP_YCbCr_MatrixTbl[eInMatrixCoeffs][eOutMatrixCoeffs]);
            }
            else
            {
                if (eInMatrixCoeffs == BVDC_P_MatrixCoeffs_eBt2020_NCL)
                {
                    pCscCfg->ulNLCnv = BVDC_P_NL_CSC_CTRL_SEL_BYPASS;
                    pCscCfg->stCscMC = s_CMP_Identity_YCbCr_to_YCbCr;
                }
                else
                {
                    pCscCfg->ulNLCnv = (eInMatrixCoeffs == BVDC_P_MatrixCoeffs_eBt2020_CL)?
                        BCHP_CMP_0_V0_NL_CSC_CTRL_SEL_CONV_R0_CL_YCbCr_2_2020_RGB :
                        BCHP_CMP_0_V0_NL_CSC_CTRL_SEL_CONV_R0_709_RGB_2_2020_RGB;
                    pCscCfg->stCscMC = s_CMP_BT2020RGB_to_UhdYCbCr;
                }
            }
        }
        else /* output non-BT2020 */
        {
            pCscCfg->pCscMA = &s_CMP_Identity_YCbCr_to_YCbCr;
            if (BVDC_P_IS_BT2020(eInMatrixCoeffs))
            {
                pCscCfg->ulNLCnv = (eInMatrixCoeffs == BVDC_P_MatrixCoeffs_eBt2020_NCL)?
                    BCHP_CMP_0_V0_NL_CSC_CTRL_SEL_CONV_R0_NCL_YCbCr_2_709_RGB :
                    BCHP_CMP_0_V0_NL_CSC_CTRL_SEL_CONV_R0_CL_YCbCr_2_709_RGB;
                pCscCfg->stCscMC = *(s_aCMP_BT709RGB_to_YCbCr_MatrixTbl[eOutMatrixCoeffs]);
            }
            else
            {
                pCscCfg->ulNLCnv = BVDC_P_NL_CSC_CTRL_SEL_BYPASS;
                if(bCscRgbMatching)
                {
                    pCscCfg->stCscMC = *(s_aaCMP_YCbCr_MatrixTbl[eInMatrixCoeffs][eOutMatrixCoeffs]);
                }
                else
                {
                    pCscCfg->stCscMC = *(s_aaCMP_YCbCr_MatrixTbl_II[eInMatrixCoeffs][eOutMatrixCoeffs]);
                }
            }
        }
        pCscCfg->ucXvYcc = ((eInMatrixCoeffs  == BVDC_P_MatrixCoeffs_eXvYcc601) ||
                            (eInMatrixCoeffs  == BVDC_P_MatrixCoeffs_eXvYcc709) ||
                            (eOutMatrixCoeffs == BVDC_P_MatrixCoeffs_eXvYcc601) ||
                            (eOutMatrixCoeffs == BVDC_P_MatrixCoeffs_eXvYcc709))? BVDC_P_IS_XVYCC : BVDC_P_NOT_XVYCC;
        BDBG_MODULE_MSG(BVDC_NLCSC, ("win[%d] NL CSC: SLOT[%d] VDC MatrixCffs %d->%d, NLCnv %d", hWindow->eId, ucSlotIdx, eInMatrixCoeffs, eOutMatrixCoeffs, pCscCfg->ulNLCnv));
    }
    else
#else /* #if ((BVDC_P_SUPPORT_CMP_NON_LINEAR_CSC!=0) */
    BSTD_UNUSED(eWinInCmp);
#endif  /* #if ((BVDC_P_SUPPORT_CMP_NON_LINEAR_CSC!=0) */

    if(bCscRgbMatching)
    {
        pCscCfg->stCscMC = *(s_aaCMP_YCbCr_MatrixTbl[eInMatrixCoeffs][eOutMatrixCoeffs]);
        BDBG_MODULE_MSG(BVDC_NLCSC, ("win[%d] Linear CSC: SLOT[%d] VDC MatrixCffs %d->%d", hWindow->eId, ucSlotIdx, eInMatrixCoeffs, eOutMatrixCoeffs));
    }
    else
    {
        pCscCfg->stCscMC = *(s_aaCMP_YCbCr_MatrixTbl_II[eInMatrixCoeffs][eOutMatrixCoeffs]);
        BDBG_MODULE_MSG(BVDC_NLCSC, ("win[%d] Linear CSC: SLOT[%d] VDC MatrixCffs %d->%d", hWindow->eId, ucSlotIdx, eInMatrixCoeffs, eOutMatrixCoeffs));
    }

    return;
}

/***************************************************************************
 * Return the desired EOTF conversion for CSC in compositor.
 *
 */
void BVDC_P_Window_GetEotfConvTable_isrsafe
    ( BVDC_P_EotfConvCfg              *pEotfConvCfg,
      uint8_t                          ucSlotIdx,
      BVDC_Window_Handle               hWindow,
      BAVC_HDMI_DRM_EOTF               eInEotf )
{
#if ((BVDC_P_SUPPORT_CMP_NON_LINEAR_CSC!=0) && (BVDC_P_CMP_NON_LINEAR_CSC_VER >= BVDC_P_NL_CSC_VER_2))
    BAVC_HDMI_DRM_EOTF eOutEotf = hWindow->hCompositor->eOutEotf;

    pEotfConvCfg->ucSlotIdx = ucSlotIdx;
    pEotfConvCfg->ucRulBuildCntr = BVDC_P_RUL_UPDATE_THRESHOLD;

    if (hWindow->hCompositor->eId == BVDC_CompositorId_eCompositor0)
    {
        pEotfConvCfg->ucNL2L = s_aCMP_CSC_NL2L_Tbl[eInEotf];
        pEotfConvCfg->ucL2NL = s_aCMP_CSC_L2NL_Tbl[eOutEotf];
        pEotfConvCfg->pLRangeAdj = s_aaCMP_LRangeAdj_Tbl[eInEotf][eOutEotf];

        BDBG_MODULE_MSG(BVDC_NLCSC, ("SLOT[%d] AVC Eotf %d->%d, NL2L %d, L2NL %d", ucSlotIdx, eInEotf, eOutEotf,pEotfConvCfg->ucNL2L, pEotfConvCfg->ucL2NL));
    }
#else
    BSTD_UNUSED(pEotfConvCfg);
    BSTD_UNUSED(ucSlotIdx);
    BSTD_UNUSED(hWindow);
    BSTD_UNUSED(eInEotf);
#endif /* #if ((BVDC_P_SUPPORT_CMP_NON_LINEAR_CSC!=0) && (BVDC_P_CMP_NON_LINEAR_CSC_VER >= BVDC_P_NL_CSC_VER_2)) */

    return;
}

/***************************************************************************
 * Return the desired matrices for converting between YCbCr and R'G'B' for
 * BVDC_P_Csc_ApplyAttenuationRGB_isr
 *
 */
void BVDC_P_Window_GetCscToApplyAttenuationRGB_isr
    ( const BVDC_P_CscCoeffs         **ppYCbCrToRGB,
      const BVDC_P_CscCoeffs         **ppRGBToYCbCr,
      BVDC_P_MatrixCoeffs              eOutMatrixCoeffs )
{
    /* Ouptut debug msgs */
    BDBG_MSG(("BVDC_P_Window_GetCscTables_YCbCr_RGB_isr:"));
    BDBG_MSG(("eOutMatrixCoeffs         = %d", eOutMatrixCoeffs));

    *ppYCbCrToRGB = s_aCMP_YCbCr_to_RGBPrim_MatrixTbl[eOutMatrixCoeffs];
    BDBG_ASSERT(*ppYCbCrToRGB);
    *ppRGBToYCbCr = s_aCMP_RGBPrim_to_YCbCr_MatrixTbl[eOutMatrixCoeffs];
    BDBG_ASSERT(*ppRGBToYCbCr);

    return;
}

/* End of file */
