/******************************************************************************
 *  Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#include "bvdc_gfxfeeder_priv.h"
#include "bvdc_window_priv.h"
#include "bvdc_compositor_priv.h"
#include "bchp.h"
#include "bchp_gfd_0.h"

BDBG_MODULE(BVDC_GFX_CSC);


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
static const BVDC_P_CscAbCoeffs  s_GFD_BT709_YCbCr_to_RGB_AlphaBlend = BVDC_P_MAKE_GFD_CSC_AB
    (  1.168950,   0.000000,   1.799771,  -249.073939,   0.000000,
       1.168950,  -0.214085,  -0.534999,    77.179562,   0.000000,
       1.168950,   2.120686,   0.000000,  -290.150968,   0.000000 );
#if 0
/* 170M YCbCr -> R'G'B' (typically Ntsc SD, or HDMI Pal SD) */
static const BVDC_P_CscAbCoeffs  s_GFD_170M_YCbCr_to_RGB_AlphaBlend = BVDC_P_MAKE_GFD_CSC_AB
    (  1.168950,   0.000000,   1.602286,  -223.795768,   0.000000,
       1.168950,  -0.393299,  -0.816156,   136.106963,   0.000000,
       1.168950,   2.025143,   0.000000,  -277.921482,   0.000000 );

/* BT470_2_BG YCbCr -> R'G'B' (typically analog Pal SD) */
static const BVDC_P_CscAbCoeffs  s_GFD_470_2_BG_YCbCr_to_RGB_AlphaBlend = BVDC_P_MAKE_GFD_CSC_AB
    (  1.168950,   0.000000,   1.602286,  -223.795768,   0.000000,
       1.168950,  -0.393299,  -0.816156,   136.106963,   0.000000,
       1.168950,   2.025143,   0.000000,  -277.921482,   0.000000 );

/* BT2020 YCbCr -> R'G'B' (typically UHD) */
static const BVDC_P_CscAbCoeffs  s_GFD_BT2020_YCbCr_to_RGB_AlphaBlend = BVDC_P_MAKE_GFD_CSC_AB
    (  1.168950,   0.000000,   1.685257,   -234.416111,   0.000000,
       1.168950,   -0.188061,   -0.652975,   88.949376,   0.000000,
       1.168950,   2.150171,   0.000000,   -293.925139,   0.000000 );
#endif

/* MC: R'G'B' -> YCbCr
 */

/* BT709 R'G'B' -> YCbCr (typically HD) */
static const BVDC_P_CscCoeffs  s_GFD_BT709_RGB_to_YCbCr_AlphaBlend = BVDC_P_MAKE_GFD_CSC
    (  0.181873,   0.611831,   0.061765,    16.000000,   0.000000,
      -0.100251,  -0.337249,   0.437500,   128.000000,   0.000000,
       0.437500,  -0.397384,  -0.040116,   128.000000,   0.000000 );

/* 170M R'G'B' -> YCbCr (typically Ntsc SD, or HDMI Pal SD) */
static const BVDC_P_CscCoeffs  s_GFD_170M_RGB_to_YCbCr_AlphaBlend = BVDC_P_MAKE_GFD_CSC
    (  0.255785,   0.502160,   0.097523,    16.000000,   0.000000,
      -0.147644,  -0.289856,   0.437500,   128.000000,   0.000000,
       0.437500,  -0.366352,  -0.071148,   128.000000,   0.000000 );

/* BT470_2_BG R'G'B' -> YCbCr (typically analog Pal SD ) */
static const BVDC_P_CscCoeffs  s_GFD_470_2_BG_RGB_to_YCbCr_AlphaBlend = BVDC_P_MAKE_GFD_CSC
    (  0.255785,   0.502160,   0.097523,    16.000000,   0.000000,
      -0.147644,  -0.289856,   0.437500,   128.000000,   0.000000,
       0.437500,  -0.366352,  -0.071148,   128.000000,   0.000000 );

/* BT2020 R'G'B' -> YCbCr (typically UHD) */
static const BVDC_P_CscCoeffs  s_GFD_BT2020_RGB_to_YCbCr_AlphaBlend = BVDC_P_MAKE_GFD_CSC
    (  0.224732,   0.580008,   0.050729,    16.000000,   0.000000,
      -0.122176,  -0.315324,   0.437500,   128.000000,   0.000000,
       0.437500,  -0.402312,  -0.035188,   128.000000,   0.000000 );

/* TODO:  get real matrices */
#define s_GFD_XvYCC601_RGB_to_YCbCr_AlphaBlend   s_GFD_170M_RGB_to_YCbCr_AlphaBlend

static const BVDC_P_CscCoeffs *const s_aGFD_MC_AlphaBlend_Tbl[] =
{
    &s_GFD_BT709_RGB_to_YCbCr_AlphaBlend,     /* BVDC_P_MatrixCoeffs_eBt709 */
    &s_GFD_170M_RGB_to_YCbCr_AlphaBlend,      /* BVDC_P_MatrixCoeffs_eSmpte170M */
    &s_GFD_470_2_BG_RGB_to_YCbCr_AlphaBlend,  /* BVDC_P_MatrixCoeffs_eBt470_2_BG */
    &s_GFD_BT2020_RGB_to_YCbCr_AlphaBlend,    /* BVDC_P_MatrixCoeffs_eBt2020_NCL */
    &s_GFD_BT2020_RGB_to_YCbCr_AlphaBlend,    /* BVDC_P_MatrixCoeffs_eBt2020_CL */
    &s_GFD_XvYCC601_RGB_to_YCbCr_AlphaBlend,  /* BVDC_P_MatrixCoeffs_eXvYcc601 */
    &s_GFD_BT709_RGB_to_YCbCr_AlphaBlend      /* BVDC_P_MatrixCoeffs_eXvYcc709 */
};

/* GFX 1886 - > 2084 */
const BVDC_P_CscLRangeAdj s_CMP_LRangeAdj_Gfx_1886_to_2084 = BVDC_P_MAKE_CMP_NL_LR_ADJ
    (  3, /* number of points */
       /*     x,        y,            m,  e */
       0.000000, 0.000000, 0.6400000000, -5,
       1.000000, 0.020000, 0.5000000000,  1,
       1.000000, 1.000000, 0.5000000000,  1,
       1.000000, 1.000000, 0.5000000000,  1,
       1.000000, 1.000000, 0.5000000000,  1,
       1.000000, 1.000000, 0.5000000000,  1,
       1.000000, 1.000000, 0.5000000000,  1,
       1.000000, 1.000000, 0.5000000000,  1 );

extern const BVDC_P_CscLRangeAdj s_CMP_LRangeAdj_Identity;

const BVDC_P_CscLRangeAdj * const s_aaCMP_LRangeAdj_Gfx_Tbl[4] =
{
    /* BAVC_HDMI_DRM_EOTF_eSDR */
    &s_CMP_LRangeAdj_Identity,            /* BAVC_HDMI_DRM_EOTF_eSDR */
    &s_CMP_LRangeAdj_Gfx_1886_to_2084,    /* BAVC_HDMI_DRM_EOTF_eHDR */
    &s_CMP_LRangeAdj_Gfx_1886_to_2084,    /* BAVC_HDMI_DRM_EOTF_eSMPTE_ST_2084 */
    &s_CMP_LRangeAdj_Identity             /* BAVC_HDMI_DRM_EOTF_eFuture */
};

extern const BVDC_P_CscAbCoeffs  s_CMP_AB_Identity;

extern const BVDC_P_CscAbCoeffs *const s_aCMP_MA_Tbl[];
extern const BVDC_P_CscCoeffs *const s_aCMP_MC_Tbl[];
extern const BVDC_P_CscAbCoeffs * const s_aaCMP_MB_Tbl[][7];

extern const uint8_t s_aCMP_CSC_NL2L_Tbl[];
extern const uint8_t s_aCMP_CSC_L2NL_Tbl[];

#endif /* #if ((BVDC_P_SUPPORT_CMP_NON_LINEAR_CSC!=0) && (BVDC_P_CMP_NON_LINEAR_CSC_VER >= BVDC_P_NL_CSC_VER_2)) */


/****************************************************************
 *  Color Conversion Matrix
 ****************************************************************/
/*--------------------------------------------------------------------*/
static const BVDC_P_CscCoeffs  s_RGBA_to_SdYCrCb_AlphaBlend = BVDC_P_MAKE_GFD_CSC
    (  0.2570,  0.5040,  0.0980,  16.0000,  0.0000,
      -0.1480, -0.2910,  0.4390, 128.0000,  0.0000,
       0.4390, -0.3680, -0.0710, 128.0000,  0.0000 );

/*--------------------------------------------------------------------*/
static const BVDC_P_CscCoeffs  s_RGBA_to_HdYCrCb_AlphaBlend = BVDC_P_MAKE_GFD_CSC
    (  0.1830,  0.6140,  0.0620,  16.0000,  0.0000,
      -0.1010, -0.3380,  0.4390, 128.0000,  0.0000,
       0.4390, -0.3990, -0.0400, 128.0000,  0.0000 );

/*--------------------------------------------------------------------*/
static const BVDC_P_CscCoeffs  s_HdRGBA_to_UhdYCrCb_AlphaBlend = BVDC_P_MAKE_GFD_CSC
    (  0.181912,   0.611800,   0.061757,   16.000000,0.000000,
      -0.091906,  -0.291101,   0.383006,  128.000000,0.000000,
       0.289770,  -0.268632,  -0.021138,  128.000000,0.000000 );

/*--------------------------------------------------------------------*/
static const BVDC_P_CscCoeffs  s_BT2020RGBA_to_UhdYCrCb_AlphaBlend = BVDC_P_MAKE_GFD_CSC
    (  0.224732,  0.580008,  0.050729,   16.000000, 0.000000,
      -0.122176, -0.315324,  0.437500,  128.000000, 0.000000,
       0.437500, -0.402312, -0.035188,  128.000000, 0.000000 );

/*--------------------------------------------------------------------*/
static const BVDC_P_CscCoeffs s_SdYCbCr_to_RGBA_AlphaBlend = BVDC_P_MAKE_GFD_CSC
    (  1.168950,  0.000000,  1.602425, -223.813572,  0.0000,
       1.168950, -0.394860, -0.816582,  136.361359,  0.0000,
       1.168950,  2.024147, -0.000000, -277.794001,  0.0000 );

/*--------------------------------------------------------------------*/
static const BVDC_P_CscCoeffs s_HdYCbCr_to_RGBA_AlphaBlend = BVDC_P_MAKE_GFD_CSC
    (  1.168950,  0.000000,  1.799682, -249.062527,  0.0000,
       1.168950, -0.214073, -0.535094,   77.190243,  0.0000,
       1.168950,  2.120703,  0.000000, -290.153216,  0.0000 );

/*--------------------------------------------------------------------*/
static const BVDC_P_CscCoeffs s_HdYCbCr_to_UhdYCbCr_AlphaBlend = BVDC_P_MAKE_GFD_CSC
    (  1.000000,  -0.000011,   0.000087,  -0.009648, 0.0000,
       0.000000,   0.874557,  -0.009669,  17.294425, 0.0000,
       0.000000,   0.012682,   0.665237,  41.226373, 0.0000 );

/*--------------------------------------------------------------------*/
static const BVDC_P_CscCoeffs  s_RGBA_to_SdYCrCb_ConstBlend = BVDC_P_MAKE_GFD_CSC
    (  0.2570,  0.5040,  0.0980,   0.0000,  16.0000,
      -0.1480, -0.2910,  0.4390,   0.0000, 128.0000,
       0.4390, -0.3680, -0.0710,   0.0000, 128.0000 );

/*--------------------------------------------------------------------*/
static const BVDC_P_CscCoeffs  s_RGBA_to_HdYCrCb_ConstBlend = BVDC_P_MAKE_GFD_CSC
    (  0.1830,  0.6140,  0.0620,   0.0000,  16.0000,
      -0.1010, -0.3380,  0.4390,   0.0000, 128.0000,
       0.4390, -0.3990, -0.0400,   0.0000, 128.0000 );

/*--------------------------------------------------------------------*/
static const BVDC_P_CscCoeffs  s_HdRGBA_to_UhdYCrCb_ConstBlend = BVDC_P_MAKE_GFD_CSC
    (  0.181912,   0.611800,   0.061757,  0.0000,   16.000000,
      -0.091906,  -0.291101,   0.383006,  0.0000,  128.000000,
       0.289770,  -0.268632,  -0.021138,  0.0000,  128.000000 );

/*--------------------------------------------------------------------*/
static const BVDC_P_CscCoeffs  s_BT2020RGBA_to_UhdYCrCb_ConstBlend = BVDC_P_MAKE_GFD_CSC
    (  0.224732,  0.580008,  0.050729,  0.0000,   16.000000,
      -0.122176, -0.315324,  0.437500,  0.0000,  128.000000,
       0.437500, -0.402312, -0.035188,  0.0000,  128.000000 );

/*--------------------------------------------------------------------*/
static const BVDC_P_CscCoeffs s_SdYCbCr_to_RGBA_ConstBlend = BVDC_P_MAKE_GFD_CSC
    (  1.168950,  0.000000,  1.602425,  0.0000, -223.813572,
       1.168950, -0.394860, -0.816582,  0.0000,  136.361359,
       1.168950,  2.024147, -0.000000,  0.0000, -277.794001 );

/*--------------------------------------------------------------------*/
static const BVDC_P_CscCoeffs s_HdYCbCr_to_RGBA_ConstBlend = BVDC_P_MAKE_GFD_CSC
    (  1.168950,  0.000000,  1.799682,  0.0000, -249.062527,
       1.168950, -0.214073, -0.535094,  0.0000,   77.190243,
       1.168950,  2.120703,  0.000000,  0.0000, -290.153216 );

/*--------------------------------------------------------------------*/
static const BVDC_P_CscCoeffs s_Identity = BVDC_P_MAKE_GFD_CSC
    (  1.000000,  0.000000,  0.000000,  0.000000,  0.000000,
       0.000000,  1.000000,  0.000000,  0.000000,  0.000000,
       0.000000,  0.000000,  1.000000,  0.000000,  0.000000 );

/*--------------------------------------------------------------------*/
static const BVDC_P_CscCoeffs s_HdYCbCr_to_UhdYCbCr_ConstBlend = BVDC_P_MAKE_GFD_CSC
    (  1.000000,  -0.000011,   0.000087,  0.0000, -0.009648,
       0.000000,   0.874557,  -0.009669,  0.0000, 17.294425,
       0.000000,   0.012682,   0.665237,  0.0000, 41.226373 );

#if 0
/* not used yet, because we don't have Uhd gfx surface yet */
/*--------------------------------------------------------------------*/
static const BVDC_P_CscCoeffs  s_RGBA_to_UhdYCrCb_AlphaBlend = BVDC_P_MAKE_GFD_CSC
    (  0.1830,  0.6140,  0.0620,  16.0000,  0.0000,
      -0.1010, -0.3380,  0.4390, 128.0000,  0.0000,
       0.4390, -0.3990, -0.0400, 128.0000,  0.0000 );

/*--------------------------------------------------------------------*/
static const BVDC_P_CscCoeffs s_UhdYCbCr_to_RGBA_AlphaBlend = BVDC_P_MAKE_GFD_CSC
    (  1.168950,  0.000000,  1.799682, -249.062527,  0.0000,
       1.168950, -0.214073, -0.535094,   77.190243,  0.0000,
       1.168950,  2.120703,  0.000000, -290.153216,  0.0000 );

/*--------------------------------------------------------------------*/
static const BVDC_P_CscCoeffs  s_RGBA_to_UhdYCrCb_ConstBlend = BVDC_P_MAKE_GFD_CSC
    (  0.1830,  0.6140,  0.0620,   0.0000,  16.0000,
      -0.1010, -0.3380,  0.4390,   0.0000, 128.0000,
       0.4390, -0.3990, -0.0400,   0.0000, 128.0000 );

/*--------------------------------------------------------------------*/
static const BVDC_P_CscCoeffs s_UhdYCbCr_to_RGBA_ConstBlend = BVDC_P_MAKE_GFD_CSC
    (  1.168950,  0.000000,  1.799682,  0.0000, -249.062527,
       1.168950, -0.214073, -0.535094,  0.0000,   77.190243,
       1.168950,  2.120703,  0.000000,  0.0000, -290.153216 );
#endif

/*---------------------------------------------------------------------
The orginal instruction from Richard W:
                                         R'_709        16
HDR10_Y'CbCr = step3 * step2 * step1 * [ G'_709 ] + [ 128 ]
                                         B'_709       128

step1 =

      0.000833725490196079       0.00280470588235294      0.000283137254901961
     -0.000449302376695451      -0.00151148193703004       0.00196078431372549
       0.00196078431372549      -0.00178099179727771     -0.000179792516447778

          luma_scale_factor         0               0
step2 = [         0         Cb_scale_factor         0       ]
                  0                 0       Cr_scale_factor

luma_scale_factor = 0.000304541078031848 * slider + 0.440165761116297

slider ranges from 0 to 1023.

slide_position_for_default =

   223

Default Cb_scale_factor = 0.65
Default Cr_scale_factor = 0.85

step3 =

                       219      -0.00252115460170543        0.0164946438489872
       2.8421709430404e-14          195.900707915213         -1.84105828906519
       6.3948846218409e-14          2.84071541916847          126.661162200751

---------------------------------------------------------------------------*/

#define DR_I_BITS   1
#define DR_F_BITS  30

#define DR_MAKE_E(e) \
   (int32_t) BMTH_FIX_SIGNED_FTOFIX(e, DR_I_BITS, DR_F_BITS)

#define DR_MAKE_M(m000, m001, m002, m010, m011, m012, m020, m021, m022, \
                  m100, m101, m102, m110, m111, m112, m120, m121, m122, \
                  m200, m201, m202, m210, m211, m212, m220, m221, m222) \
{\
    {\
        {DR_MAKE_E(m000), DR_MAKE_E(m001), DR_MAKE_E(m002)}, \
        {DR_MAKE_E(m010), DR_MAKE_E(m011), DR_MAKE_E(m012)}, \
        {DR_MAKE_E(m020), DR_MAKE_E(m021), DR_MAKE_E(m022)}  \
    },\
    {\
        {DR_MAKE_E(m100), DR_MAKE_E(m101), DR_MAKE_E(m102)}, \
        {DR_MAKE_E(m110), DR_MAKE_E(m111), DR_MAKE_E(m112)}, \
        {DR_MAKE_E(m120), DR_MAKE_E(m121), DR_MAKE_E(m122)}  \
    },\
    {\
        {DR_MAKE_E(m200), DR_MAKE_E(m201), DR_MAKE_E(m202)}, \
        {DR_MAKE_E(m210), DR_MAKE_E(m211), DR_MAKE_E(m212)}, \
        {DR_MAKE_E(m220), DR_MAKE_E(m221), DR_MAKE_E(m222)}  \
    }\
}

/* M[i][k][j] = S3[i][k] * S1[k][j] */
static const int32_t s_iM[3][3][3] = DR_MAKE_M(
     0.182585882352941320,  0.614230588235293862,  0.062007058823529464,
     0.000001132760754563,  0.000003810679640938, -0.000004943440395501,
     0.000032342438919583, -0.000029376825394063, -0.000002965613525519,

     0.000000000000000024,  0.000000000000000080,  0.000000000000000008,
    -0.088018653662626539, -0.296100381465242202,  0.384119035127868602,
    -0.003609918213853314,  0.003278909711135239,  0.000331008502718071,

     0.000000000000000053,  0.000000000000000179,  0.000000000000000018,
    -0.001276340189347808, -0.004293690044315861,  0.005570030233663667,
     0.248355220001472532, -0.225582490913199069, -0.022772729088273200);

#define DR_MAX_SHORT    32768
#define DR_MID_SLIDER   223
#define DR_MAX_SLIDER   1023

#define DR_ACCU_BITS    8
#define SCL_MUL(s, x) \
    BMTH_FIX_SIGNED_MUL(s, x, DR_I_BITS, DR_F_BITS, DR_I_BITS, DR_F_BITS, DR_I_BITS, DR_F_BITS)
#define DR_MUL(s, x) \
    BMTH_FIX_SIGNED_MUL(s, x, DR_I_BITS, DR_F_BITS, DR_I_BITS, DR_F_BITS, BVDC_P_CSC_GFD_CX_I_BITS, BVDC_P_CSC_GFD_CX_F_BITS + DR_ACCU_BITS)
#define DR_MAKE_CX(x) \
    (uint16_t)BMTH_FIX_SIGNED_CONVERT(x, BVDC_P_CSC_GFD_CX_I_BITS, BVDC_P_CSC_GFD_CX_F_BITS + DR_ACCU_BITS, BVDC_P_CSC_GFD_CX_I_BITS, BVDC_P_CSC_GFD_CX_F_BITS)

#define DR_MAKE_A(x) \
    BVDC_P_MAKE_CSC_CX(x / (1 << BVDC_P_CSC_GFD_CO_VID_BITS), BVDC_P_CSC_GFD_CX_I_BITS, BVDC_P_CSC_GFD_CX_F_BITS)
#define DR_MAKE_O(x) \
    BVDC_P_MAKE_CSC_CO(x, BVDC_P_CSC_GFD_CO_I_BITS, BVDC_P_CSC_GFD_CO_F_BITS, BVDC_P_CSC_GFD_CO_VID_BITS, BVDC_P_CSC_GFD_CO_VID_TBL_BITS)

void BVDC_P_GfxFeeder_CalculateSdr2HdrCsc_isr
    ( BVDC_P_GfxFeeder_Handle      hGfxFeeder,
      bool                         bConstantBlend )
{
    int32_t iSlider;
    int32_t iYScl, iCbScl, iCrScl;
    BVDC_P_GfxFeederCfgInfo *pCurCfg = &(hGfxFeeder->stCurCfgInfo);
    BVDC_P_CscCoeffs *pCsc = &hGfxFeeder->stCscCoeffSdr2Hdr;

    /* YScl range: [0.440165761116297, 0.751711283942877504], default: 0.508078421517399104
     *   0.000304541078031848 * 1023 + 0.440165761116297 = 0.751711283942877504
     *   0.000304541078031848 * 223 + 0.440165761116297 = 0.508078421517399104
     * Y positive step:
     *   0.751711283942877504 - 0.508078421517399104 = 0.2436328624254784
     *   step = (0.2436328624254784 / 0x7FFF) * 0x8000 = 0.24364029773729899628284554582354
     * Y negative step:
     *   step = ((0.000304541078031848 * 223) / 0x8000) * 0x8000 = 0.067912660401102104
     * where "* 0x8000" is for more accuracy, it is compensated by "-15" in sYSlider << (30 - 15);
     */
    if (pCurCfg->stSdrGfx2HdrAdj.sYSlider < 0)
    {
        /* (0x8000 - sYSlider) / 0x8000 in fixed point with 30 frac bits */
        iSlider = ((int32_t)0x8000 + pCurCfg->stSdrGfx2HdrAdj.sYSlider) << (30-15);
        iYScl = SCL_MUL(DR_MAKE_E(0.067912660401102104), iSlider) + DR_MAKE_E(0.440165761116297);
    }
    else
    {
        /* sYSlider / 0x8000 in fixed point with 30 frac bits */
        iSlider = pCurCfg->stSdrGfx2HdrAdj.sYSlider << (30-15);
        iYScl = SCL_MUL(DR_MAKE_E(0.24364029773729899628284554582354), iSlider) + DR_MAKE_E(0.508078421517399104);
    }

    /* CbScl range: [0.4, 1.0], default: 0.65
     * Cb positive step:
     *   step = ((1.0 - 0.65) / 0x7FFF) * 0x8000 = 0.35001068147831659901730399487289
     * Cb negative step:
     *   step = ((0.65 - 0.4) / 0x8000) * 0x8000 = 0.25
     * where "* 0x8000" is for more accuracy, it is compensated by "-15" in sCbSlider << (30 - 15);
     */
    if (pCurCfg->stSdrGfx2HdrAdj.sCbSlider < 0)
    {
        /* (0x8000 - sCbSlider) / 0x8000 in fixed point with 30 frac bits */
        iSlider = ((int32_t)0x8000 + pCurCfg->stSdrGfx2HdrAdj.sCbSlider) << (30-15);
        iCbScl = SCL_MUL(DR_MAKE_E(0.25), iSlider) + DR_MAKE_E(0.4);
    }
    else
    {
        /* sCbSlider / 0x8000 in fixed point with 30 frac bits */
        iSlider = pCurCfg->stSdrGfx2HdrAdj.sCbSlider << (30-15);
        iCbScl = SCL_MUL(DR_MAKE_E(0.35001068147831659901730399487289), iSlider) + DR_MAKE_E(0.65);
    }

    /* CrScl range: [0.5, 1.0]. default: 0.85
     * Cr positive step:
     *   step = ((1.0 - 0.85) / 0x7FFF) * 0x8000 = 0.15000457777642139957884456923124
     * Cr negative step:
     *   step = ((0.85 - 0.5) / 0x8000) * 0x8000 = 0.35
     * where "* 0x8000" is for more accuracy, it is compensated by "-15" in sCrSlider << (30 - 15);
     */
    if (pCurCfg->stSdrGfx2HdrAdj.sCrSlider < 0)
    {
        /* (0x8000 + sCrSlider) / 0x8000 in fixed point with 30 frac bits */
        iSlider = ((int32_t)0x8000 + pCurCfg->stSdrGfx2HdrAdj.sCrSlider) << (30-15);
        iCrScl = SCL_MUL(DR_MAKE_E(0.35), iSlider) + DR_MAKE_E(0.5);
    }
    else
    {
        /* sCrSlider / 0x8000 in fixed point with 30 frac bits */
        iSlider = pCurCfg->stSdrGfx2HdrAdj.sCrSlider << (30-15);
        iCrScl = SCL_MUL(DR_MAKE_E(0.15000457777642139957884456923124), iSlider) + DR_MAKE_E(0.85);
    }

    pCsc->usY0  = DR_MAKE_CX(DR_MUL(iYScl,s_iM[0][0][0]) + DR_MUL(iCbScl,s_iM[0][1][0]) + DR_MUL(iCrScl,s_iM[0][2][0]));
    pCsc->usY1  = DR_MAKE_CX(DR_MUL(iYScl,s_iM[0][0][1]) + DR_MUL(iCbScl,s_iM[0][1][1]) + DR_MUL(iCrScl,s_iM[0][2][1]));
    pCsc->usY2  = DR_MAKE_CX(DR_MUL(iYScl,s_iM[0][0][2]) + DR_MUL(iCbScl,s_iM[0][1][2]) + DR_MUL(iCrScl,s_iM[0][2][2]));

    pCsc->usCb0 = DR_MAKE_CX(DR_MUL(iYScl,s_iM[1][0][0]) + DR_MUL(iCbScl,s_iM[1][1][0]) + DR_MUL(iCrScl,s_iM[1][2][0]));
    pCsc->usCb1 = DR_MAKE_CX(DR_MUL(iYScl,s_iM[1][0][1]) + DR_MUL(iCbScl,s_iM[1][1][1]) + DR_MUL(iCrScl,s_iM[1][2][1]));
    pCsc->usCb2 = DR_MAKE_CX(DR_MUL(iYScl,s_iM[1][0][2]) + DR_MUL(iCbScl,s_iM[1][1][2]) + DR_MUL(iCrScl,s_iM[1][2][2]));

    pCsc->usCr0 = DR_MAKE_CX(DR_MUL(iYScl,s_iM[2][0][0]) + DR_MUL(iCbScl,s_iM[2][1][0]) + DR_MUL(iCrScl,s_iM[2][2][0]));

    pCsc->usCr1 = DR_MAKE_CX(DR_MUL(iYScl,s_iM[2][0][1]) + DR_MUL(iCbScl,s_iM[2][1][1]) + DR_MUL(iCrScl,s_iM[2][2][1]));

    pCsc->usCr2 = DR_MAKE_CX(DR_MUL(iYScl,s_iM[2][0][2]) + DR_MUL(iCbScl,s_iM[2][1][2]) + DR_MUL(iCrScl,s_iM[2][2][2]));

    if (bConstantBlend)
    {
        pCsc->usYAlpha = 0;
        pCsc->usCbAlpha = 0;
        pCsc->usCrAlpha = 0;
        pCsc->usYOffset = DR_MAKE_O(16.0);
        pCsc->usCbOffset = DR_MAKE_O(128.0);
        pCsc->usCrOffset = DR_MAKE_O(128.0);
    }
    else
    {
        pCsc->usYAlpha = DR_MAKE_A(16.0);
        pCsc->usCbAlpha = DR_MAKE_A(128.0);
        pCsc->usCrAlpha = DR_MAKE_A(128.0);
        pCsc->usYOffset = 0;
        pCsc->usCbOffset = 0;
        pCsc->usCrOffset = 0;
    }
    pCsc->usCxIntBits = BVDC_P_CSC_GFD_CX_I_BITS;
    pCsc->usCxFractBits = BVDC_P_CSC_GFD_CX_F_BITS;
    pCsc->usCoIntBits = BVDC_P_CSC_GFD_CO_I_BITS;
    pCsc->usCoFractBits = BVDC_P_CSC_GFD_CO_F_BITS;
    pCsc->usCoVideoBits = BVDC_P_CSC_GFD_CO_VID_BITS;

    BDBG_MSG(("GFD matrix for SDR gfx to HDR:"));
    BVDC_P_Csc_Print_isr(pCsc);
}

/*------------------------------------------------------------------------
 * {private}
 * BVDC_P_GfxFeeder_DecideColorMatrix_isr
 *
 * output: color matrix to convert from active pixel format to output
 *         color primary (main video window's color primary)
 *
 * Note: Because of gamma effect, of not knowing how user treated alpha
 * when the src gfx surface was created, and of diff between Bt601 and
 * Bt709 is not very noticable for gfx, we decide to use idendity matrix
 * to convert between Bt601 and Bt709 (i.e. not conv).
 * Note: If display is Uhd, we assume gfx surface is Bt 709 color, we do
 * convert from BT 709 to BT 2020
 */
BERR_Code BVDC_P_GfxFeeder_DecideColorMatrix_isr
    ( BPXL_Format                  eActivePxlFmt,
      BVDC_P_GfxFeeder_Handle      hGfxFeeder,
      const BVDC_P_CscCoeffs     **ppaulRGBToYCbCr,
      const BVDC_P_CscCoeffs     **ppaulYCbCrToRGB )
{
    BVDC_P_MatrixCoeffs eOutMatrixCoeffs;
    BAVC_HDMI_DRM_EOTF eOutEotf;
    bool bConstantBlend = hGfxFeeder->stCurCfgInfo.stFlags.bConstantBlending;
    BVDC_P_CscCfg *pCscCfg = &(hGfxFeeder->stGfxCsc);

    /* TODO: in the future, we should distinquish current hd/sd standard
     * from legacy hd/sd, and NTSC sd from PAL sd */
    BVDC_P_Window_GetCurrentOutMatrixCoeffs_isr( hGfxFeeder->hWindow, &eOutMatrixCoeffs );
    BVDC_P_Window_GetCurrentOutEotf_isr( hGfxFeeder->hWindow, &eOutEotf );

    if ( (BVDC_P_MatrixCoeffs_eBt2020_NCL == eOutMatrixCoeffs) ||
         (BVDC_P_MatrixCoeffs_eBt709 == eOutMatrixCoeffs) )
    {
        *ppaulRGBToYCbCr = (bConstantBlend)?
            &s_RGBA_to_HdYCrCb_ConstBlend : &s_RGBA_to_HdYCrCb_AlphaBlend;

        *ppaulYCbCrToRGB = (bConstantBlend)?
            &s_HdYCbCr_to_RGBA_ConstBlend : &s_HdYCbCr_to_RGBA_AlphaBlend;
    }
    else
    {
        *ppaulRGBToYCbCr = (bConstantBlend)?
            &s_RGBA_to_SdYCrCb_ConstBlend : &s_RGBA_to_SdYCrCb_AlphaBlend;

        *ppaulYCbCrToRGB = (bConstantBlend)?
            &s_SdYCbCr_to_RGBA_ConstBlend : &s_SdYCbCr_to_RGBA_AlphaBlend;
    }

#ifndef BVDC_FOR_BOOTUPDATER

    if (hGfxFeeder->stCurCfgInfo.stDirty.stBits.bSdrGfx2HdrAdj)
    {
        BVDC_P_GfxFeeder_CalculateSdr2HdrCsc_isr(hGfxFeeder, bConstantBlend);
    }

#if (BVDC_P_SUPPORT_CMP_NON_LINEAR_CSC!=0)
    pCscCfg->ucXvYcc = ((eOutMatrixCoeffs == BVDC_P_MatrixCoeffs_eXvYcc601) ||
                        (eOutMatrixCoeffs == BVDC_P_MatrixCoeffs_eXvYcc709))? BVDC_P_IS_XVYCC : BVDC_P_NOT_XVYCC;
    pCscCfg->ucInputCL = BVDC_P_NOT_XVYCC;
    pCscCfg->pCscMA = &s_Identity;
    pCscCfg->ulNLCnv = BVDC_P_NL_CSC_CTRL_SEL_BYPASS;

#if (BVDC_P_CMP_NON_LINEAR_CSC_VER >= BVDC_P_NL_CSC_VER_2)
    if (hGfxFeeder->bSupportEotfConv)
    {
        BVDC_P_EotfConvCfg *pEotfConvCfg = &(hGfxFeeder->stEotfConv);
        BVDC_P_MatrixCoeffs eInMatrixCoeffs = BVDC_P_MatrixCoeffs_eBt709;
        if (bConstantBlend)
        {
            pCscCfg->pCscAbMA = (BPXL_IS_RGB_FORMAT(eActivePxlFmt))? &s_CMP_AB_Identity : s_aCMP_MA_Tbl[eInMatrixCoeffs];
            pCscCfg->pCscAbMB = s_aaCMP_MB_Tbl[eInMatrixCoeffs][eOutMatrixCoeffs];
            pCscCfg->stCscMC = *(s_aCMP_MC_Tbl[eOutMatrixCoeffs]);
        }
        else
        {
            pCscCfg->pCscAbMA = (BPXL_IS_RGB_FORMAT(eActivePxlFmt))? &s_CMP_AB_Identity : &s_GFD_BT709_YCbCr_to_RGB_AlphaBlend;
            pCscCfg->pCscAbMB = s_aaCMP_MB_Tbl[eInMatrixCoeffs][eOutMatrixCoeffs];
            pCscCfg->stCscMC = *(s_aGFD_MC_AlphaBlend_Tbl[eOutMatrixCoeffs]);
        }
        pEotfConvCfg->ucNL2L = s_aCMP_CSC_NL2L_Tbl[BAVC_HDMI_DRM_EOTF_eSDR];
        pEotfConvCfg->ucL2NL = s_aCMP_CSC_L2NL_Tbl[eOutEotf];
        pEotfConvCfg->pLRangeAdj = s_aaCMP_LRangeAdj_Gfx_Tbl[eOutEotf];
    }
    else

/* #if (BVDC_P_CMP_NON_LINEAR_CSC_VER >= BVDC_P_NL_CSC_VER_2) */
#elif defined(BCHP_GFD_0_NL_CSC_CTRL_SEL_CONV_R0_709_RGB_2_2020_RGB)

    if ((BVDC_P_MatrixCoeffs_eBt2020_NCL == eOutMatrixCoeffs) &&
        (hGfxFeeder->bSupportNLCsc) &&
        (hGfxFeeder->bSupportMACsc || (BPXL_IS_RGB_FORMAT(eActivePxlFmt))) &&
        ((BAVC_HDMI_DRM_EOTF_eSDR == eOutEotf) || (!BPXL_IS_RGB_FORMAT(eActivePxlFmt))))
    {
        if (!BPXL_IS_RGB_FORMAT(eActivePxlFmt))
        {
            pCscCfg->pCscMA = &s_HdYCbCr_to_RGBA_ConstBlend;
        }
        pCscCfg->ulNLCnv = BCHP_GFD_0_NL_CSC_CTRL_SEL_CONV_R0_709_RGB_2_2020_RGB;
        pCscCfg->stCscMC = (bConstantBlend)?
            s_BT2020RGBA_to_UhdYCrCb_ConstBlend : s_BT2020RGBA_to_UhdYCrCb_AlphaBlend;
    }

    else
#endif /* #if (BVDC_P_CMP_NON_LINEAR_CSC_VER >= BVDC_P_NL_CSC_VER_2) */

#endif /* #if (BVDC_P_SUPPORT_CMP_NON_LINEAR_CSC!=0) */

    if( hGfxFeeder->hWindow->stCurInfo.bUserCsc )
    {
        BDBG_MSG(("Using User WIN CSC for GFX CSC Matrix"));

        if( hGfxFeeder->stCurCfgInfo.stDirty.stBits.bCsc )
        {
            BVDC_P_Csc_FromMatrix_isr( &pCscCfg->stCscMC,
                hGfxFeeder->hWindow->stCurInfo.pl32_Matrix, hGfxFeeder->hWindow->stCurInfo.ulUserShift);
        }
    }
    else
#endif /* #ifndef BVDC_FOR_BOOTUPDATER */

    if ( true == BPXL_IS_RGB_FORMAT(eActivePxlFmt) )
    {
        if (BAVC_HDMI_DRM_EOTF_eSDR != eOutEotf)
        {
            pCscCfg->stCscMC = hGfxFeeder->stCscCoeffSdr2Hdr;
#if (BDBG_DEBUG_BUILD)
            if (hGfxFeeder->stCurCfgInfo.stDirty.stBits.bSdrGfx2HdrAdj)
            {
                BDBG_MSG(("Using HDR CSC for GFD"));
            }
#endif
        }
        else if (BVDC_P_MatrixCoeffs_eBt2020_NCL == eOutMatrixCoeffs)
        {
            pCscCfg->stCscMC = (bConstantBlend)?
                s_HdRGBA_to_UhdYCrCb_ConstBlend : s_HdRGBA_to_UhdYCrCb_AlphaBlend;
        }
        else
        {
            pCscCfg->stCscMC = **ppaulRGBToYCbCr;
        }
        /* this makes BVDC_P_Csc_ApplyAttenuationRGB_isr work */
        *ppaulYCbCrToRGB = &s_Identity;
        *ppaulRGBToYCbCr = &s_Identity;
    }
    else
    {
#ifndef BVDC_FOR_BOOTUPDATER
        if (BVDC_P_MatrixCoeffs_eBt2020_NCL == eOutMatrixCoeffs)
        {
            pCscCfg->stCscMC = (bConstantBlend)?
                s_HdYCbCr_to_UhdYCbCr_ConstBlend : s_HdYCbCr_to_UhdYCbCr_AlphaBlend;
        }
        else
#endif /* #ifndef BVDC_FOR_BOOTUPDATER */
        {
            pCscCfg->stCscMC = s_Identity;
        }
    }

    return BERR_SUCCESS;
}

BERR_Code BVDC_P_GfxFeeder_InitColorMatrix
    ( BVDC_P_GfxFeeder_Handle      hGfxFeeder )
{
    BVDC_P_CscCfg *pCscCfg = &(hGfxFeeder->stGfxCsc);

#ifndef BVDC_FOR_BOOTUPDATER

#if (BVDC_P_SUPPORT_CMP_NON_LINEAR_CSC!=0)
    pCscCfg->pCscMA = &s_Identity;
    pCscCfg->ulNLCnv = BVDC_P_NL_CSC_CTRL_SEL_BYPASS;

#if (BVDC_P_CMP_NON_LINEAR_CSC_VER >= BVDC_P_NL_CSC_VER_2)
    if (hGfxFeeder->bSupportEotfConv)
    {
        pCscCfg->pCscAbMA = &s_CMP_AB_Identity;
        pCscCfg->pCscAbMB = &s_CMP_AB_Identity;
        hGfxFeeder->stEotfConv.pLRangeAdj = s_aaCMP_LRangeAdj_Gfx_Tbl[BAVC_HDMI_DRM_EOTF_eSDR];
    }
#endif /* #if (BVDC_P_CMP_NON_LINEAR_CSC_VER >= BVDC_P_NL_CSC_VER_2) */
#endif /* #if (BVDC_P_SUPPORT_CMP_NON_LINEAR_CSC!=0) */
#endif /* #ifndef BVDC_FOR_BOOTUPDATER */

    pCscCfg->stCscMC = s_Identity;

    return BERR_SUCCESS;
}

/****************************************************************
 *  GFD HSCL coefficients
 ****************************************************************/

static const uint32_t s_aulFirCoeff_PointSample[] =
{
    BCHP_FIELD_DATA( GFD_0_HORIZ_FIR_COEFF_PHASE0_00_01, COEFF_0, 0x0000 ) |
    BCHP_FIELD_DATA( GFD_0_HORIZ_FIR_COEFF_PHASE0_00_01, COEFF_1, 0x0000 ),
    BCHP_FIELD_DATA( GFD_0_HORIZ_FIR_COEFF_PHASE0_02,    COEFF_2, 0x0100 ),

    BCHP_FIELD_DATA( GFD_0_HORIZ_FIR_COEFF_PHASE1_00_01, COEFF_0, 0x0000 ) |
    BCHP_FIELD_DATA( GFD_0_HORIZ_FIR_COEFF_PHASE1_00_01, COEFF_1, 0x0000 ),
    BCHP_FIELD_DATA( GFD_0_HORIZ_FIR_COEFF_PHASE1_02,    COEFF_2, 0x0000 ),
};

static const uint32_t s_aulFirCoeff_Bilinear[] =
{
    BCHP_FIELD_DATA( GFD_0_HORIZ_FIR_COEFF_PHASE0_00_01, COEFF_0, 0x0000 ) |
    BCHP_FIELD_DATA( GFD_0_HORIZ_FIR_COEFF_PHASE0_00_01, COEFF_1, 0x0000 ),
    BCHP_FIELD_DATA( GFD_0_HORIZ_FIR_COEFF_PHASE0_02,    COEFF_2, 0x0100 ),

    BCHP_FIELD_DATA( GFD_0_HORIZ_FIR_COEFF_PHASE1_00_01, COEFF_0, 0x0000 ) |
    BCHP_FIELD_DATA( GFD_0_HORIZ_FIR_COEFF_PHASE1_00_01, COEFF_1, 0x0000 ),
    BCHP_FIELD_DATA( GFD_0_HORIZ_FIR_COEFF_PHASE1_02,    COEFF_2, 0x0080 ),
};

static const uint32_t s_aulFirCoeff_Sharp_1toN[] =
{
    BCHP_FIELD_DATA( GFD_0_HORIZ_FIR_COEFF_PHASE0_00_01, COEFF_0, 0x0000 ) |
    BCHP_FIELD_DATA( GFD_0_HORIZ_FIR_COEFF_PHASE0_00_01, COEFF_1, 0x0000 ),
    BCHP_FIELD_DATA( GFD_0_HORIZ_FIR_COEFF_PHASE0_02,    COEFF_2, 0x0100 ),

    BCHP_FIELD_DATA( GFD_0_HORIZ_FIR_COEFF_PHASE1_00_01, COEFF_0, 0x000e ) |
    BCHP_FIELD_DATA( GFD_0_HORIZ_FIR_COEFF_PHASE1_00_01, COEFF_1, 0xffd7 ),
    BCHP_FIELD_DATA( GFD_0_HORIZ_FIR_COEFF_PHASE1_02,    COEFF_2, 0x009b ),
};

static const uint32_t s_aulFirCoeff_Smooth_1toN[] =
{
    BCHP_FIELD_DATA( GFD_0_HORIZ_FIR_COEFF_PHASE0_00_01, COEFF_0, 0x0000 ) |
    BCHP_FIELD_DATA( GFD_0_HORIZ_FIR_COEFF_PHASE0_00_01, COEFF_1, 0x0000 ),
    BCHP_FIELD_DATA( GFD_0_HORIZ_FIR_COEFF_PHASE0_02,    COEFF_2, 0x0100 ),

    BCHP_FIELD_DATA( GFD_0_HORIZ_FIR_COEFF_PHASE1_00_01, COEFF_0, 0x0004 ) |
    BCHP_FIELD_DATA( GFD_0_HORIZ_FIR_COEFF_PHASE1_00_01, COEFF_1, 0xffe5 ),
    BCHP_FIELD_DATA( GFD_0_HORIZ_FIR_COEFF_PHASE1_02,    COEFF_2, 0x0097 ),
};

static const uint32_t s_aulFirCoeff_Sharp_16to9[] =
{
    BCHP_FIELD_DATA( GFD_0_HORIZ_FIR_COEFF_PHASE0_00_01, COEFF_0, 0xfff0 ) |
    BCHP_FIELD_DATA( GFD_0_HORIZ_FIR_COEFF_PHASE0_00_01, COEFF_1, 0x0044 ),
    BCHP_FIELD_DATA( GFD_0_HORIZ_FIR_COEFF_PHASE0_02,    COEFF_2, 0x0098 ),

    BCHP_FIELD_DATA( GFD_0_HORIZ_FIR_COEFF_PHASE1_00_01, COEFF_0, 0xfff0 ) |
    BCHP_FIELD_DATA( GFD_0_HORIZ_FIR_COEFF_PHASE1_00_01, COEFF_1, 0x000c ),
    BCHP_FIELD_DATA( GFD_0_HORIZ_FIR_COEFF_PHASE1_02,    COEFF_2, 0x0084 ),
};

static const uint32_t s_aulFirCoeff_Smooth_16to9[] =
{
    BCHP_FIELD_DATA( GFD_0_HORIZ_FIR_COEFF_PHASE0_00_01, COEFF_0, 0xfffc ) |
    BCHP_FIELD_DATA( GFD_0_HORIZ_FIR_COEFF_PHASE0_00_01, COEFF_1, 0x003d ),
    BCHP_FIELD_DATA( GFD_0_HORIZ_FIR_COEFF_PHASE0_02,    COEFF_2, 0x008e ),

    BCHP_FIELD_DATA( GFD_0_HORIZ_FIR_COEFF_PHASE1_00_01, COEFF_0, 0xfffc ) |
    BCHP_FIELD_DATA( GFD_0_HORIZ_FIR_COEFF_PHASE1_00_01, COEFF_1, 0x000e ),
    BCHP_FIELD_DATA( GFD_0_HORIZ_FIR_COEFF_PHASE1_02,    COEFF_2, 0x0076 ),
};

static const uint32_t s_aulFirCoeff_Sharp_3to1[] =
{
    BCHP_FIELD_DATA( GFD_0_HORIZ_FIR_COEFF_PHASE0_00_01, COEFF_0, 0x0017 ) |
    BCHP_FIELD_DATA( GFD_0_HORIZ_FIR_COEFF_PHASE0_00_01, COEFF_1, 0x003f ),
    BCHP_FIELD_DATA( GFD_0_HORIZ_FIR_COEFF_PHASE0_02,    COEFF_2, 0x0054 ),

    BCHP_FIELD_DATA( GFD_0_HORIZ_FIR_COEFF_PHASE1_00_01, COEFF_0, 0x000a ) |
    BCHP_FIELD_DATA( GFD_0_HORIZ_FIR_COEFF_PHASE1_00_01, COEFF_1, 0x002a ),
    BCHP_FIELD_DATA( GFD_0_HORIZ_FIR_COEFF_PHASE1_02,    COEFF_2, 0x004c ),
};

static const uint32_t s_aulFirCoeff_Smooth_3to1[] =
{
    BCHP_FIELD_DATA( GFD_0_HORIZ_FIR_COEFF_PHASE0_00_01, COEFF_0, 0x001d ) |
    BCHP_FIELD_DATA( GFD_0_HORIZ_FIR_COEFF_PHASE0_00_01, COEFF_1, 0x003d ),
    BCHP_FIELD_DATA( GFD_0_HORIZ_FIR_COEFF_PHASE0_02,    COEFF_2, 0x004c ),

    BCHP_FIELD_DATA( GFD_0_HORIZ_FIR_COEFF_PHASE1_00_01, COEFF_0, 0x000e ) |
    BCHP_FIELD_DATA( GFD_0_HORIZ_FIR_COEFF_PHASE1_00_01, COEFF_1, 0x002c ),
    BCHP_FIELD_DATA( GFD_0_HORIZ_FIR_COEFF_PHASE1_02,    COEFF_2, 0x0046 ),
};


#define BVDC_P_GFX_FIRCOEFF_IDX_START (101)
#define BVDC_P_GFX_FIRCOEFF_IDX_END   (126)

/****************************************************************
 *  Indexed GFD HSCL coefficients
 ****************************************************************/
static const uint32_t s_paulFirCoeffTbl[][GFD_NUM_REGS_HSCL_COEFF] =
{
    { 0x01000400, 0x06000000, 0x00400260, 0x05600000 },  /* 1 */
    { 0x00B00400, 0x06A00000, 0x00200210, 0x05D00000 },  /* 2 */
    { 0x00800400, 0x07000000, 0x000001E0, 0x06200000 },  /* 3 */
    { 0x00500400, 0x07600000, 0x3FF001A0, 0x06700000 },  /* 4 */
    { 0x002003F0, 0x07E00000, 0x3FE00160, 0x06C00000 },  /* 5 */
    { 0x3FF003E0, 0x08600000, 0x3FE00120, 0x07000000 },  /* 6 */
    { 0x3FC003C0, 0x09000000, 0x3FE000D0, 0x07500000 },  /* 7 */
    { 0x3FA003A0, 0x09800000, 0x3FD00090, 0x07A00000 },  /* 8 */
    { 0x3F800370, 0x0A200000, 0x3FE00040, 0x07E00000 },  /* 9 */
    { 0x3F700330, 0x0AC00000, 0x3FE03FF0, 0x08300000 },  /* 10 */
    { 0x3F6002F0, 0x0B600000, 0x3FE03FA0, 0x08800000 },  /* 11 */
    { 0x3F6002A0, 0x0C000000, 0x3FF03F50, 0x08C00000 },  /* 12 */
    { 0x3F600240, 0x0CC00000, 0x00003F00, 0x09000000 },  /* 13 */
    { 0x3F7001D0, 0x0D800000, 0x00103ED0, 0x09200000 },  /* 14 */
    { 0x3F900170, 0x0E000000, 0x00103E90, 0x09600000 },  /* 15 */
    { 0x3FB000F0, 0x0EC00000, 0x00203E70, 0x09700000 },  /* 16 */
    { 0x3FD00090, 0x0F400000, 0x00203E60, 0x09800000 },  /* 17 */
    { 0x00000010, 0x0FE00000, 0x00303E50, 0x09800000 },  /* 18 */
    { 0x00203FA0, 0x10800000, 0x00303E50, 0x09800000 },  /* 19 */
    { 0x00503F10, 0x11400000, 0x00203E70, 0x09700000 },  /* 20 */
    { 0x00703EB0, 0x11C00000, 0x00203E90, 0x09500000 },  /* 21 */
    { 0x00903E40, 0x12600000, 0x00103EC0, 0x09300000 },  /* 22 */
    { 0x00A03DE0, 0x13000000, 0x00003EF0, 0x09100000 },  /* 23 */
    { 0x00A03D70, 0x13E00000, 0x3FF03F40, 0x08D00000 },  /* 24 */
    { 0x00A03D20, 0x14800000, 0x3FF03F90, 0x08800000 },  /* 25 */
    { 0x00A03CD0, 0x15200000, 0x3FE03FE0, 0x08400000 },  /* 26 */
};

/*--------------------------------------------------------------------*/
static uint32_t *BVDC_P_GetFilterCoefficients_Auto_isr(
    uint32_t ulSrcSize,
    uint32_t ulOutSize )
{
    if( ulOutSize == ulSrcSize )
        return (uint32_t *) s_aulFirCoeff_PointSample;
    else if( ulOutSize >= 3 * ulSrcSize ) /* 1to3: 720p -> 4k */
        return (uint32_t *) &s_paulFirCoeffTbl[11];
    else if( ulOutSize >= 2 * ulSrcSize ) /* 1to2: 1080p -> 4k */
        return (uint32_t *) &s_paulFirCoeffTbl[11];
    else if( ulOutSize >= ulSrcSize )     /* 1toN: Any other scaler up */
        return (uint32_t *) s_aulFirCoeff_Smooth_1toN;
    else if( ulOutSize * 12 >= ulSrcSize * 16 )  /* rounding to 9/16 */
        return (uint32_t *) s_aulFirCoeff_Smooth_16to9;
    else
        return (uint32_t *) s_aulFirCoeff_Smooth_3to1;
}


/*--------------------------------------------------------------------*/
static uint32_t *BVDC_P_GetFilterCoefficients_PointSample_isr(
    uint32_t ulSrcSize,
    uint32_t ulOutSize )
{
    BSTD_UNUSED(ulSrcSize);
    BSTD_UNUSED(ulOutSize);
    return (uint32_t *) s_aulFirCoeff_PointSample;
}

/*--------------------------------------------------------------------*/
static uint32_t *BVDC_P_GetFilterCoefficients_Bilinear_isr(
    uint32_t ulSrcSize,
    uint32_t ulOutSize )
{
    BSTD_UNUSED(ulSrcSize);
    BSTD_UNUSED(ulOutSize);
    return (uint32_t *) s_aulFirCoeff_Bilinear;
}

/*--------------------------------------------------------------------*/
static uint32_t *BVDC_P_GetFilterCoefficients_Sharp_isr(
    uint32_t ulSrcSize,
    uint32_t ulOutSize )
{
    if( ulOutSize == ulSrcSize )
        return (uint32_t *) s_aulFirCoeff_PointSample;
    else if( ulOutSize >= ulSrcSize )
        return (uint32_t *) s_aulFirCoeff_Sharp_1toN;
    else if( ulOutSize * 12 >= ulSrcSize * 16 )  /* rounding to 9/16 */
        return (uint32_t *) s_aulFirCoeff_Sharp_16to9;
    else
        return (uint32_t *) s_aulFirCoeff_Sharp_3to1;
}

/*--------------------------------------------------------------------*/
static uint32_t *BVDC_P_GetFilterCoefficients_Smooth_isr(
    uint32_t ulSrcSize,
    uint32_t ulOutSize )
{
    if( ulOutSize == ulSrcSize )
        return (uint32_t *) s_aulFirCoeff_PointSample;
    else if( ulOutSize >= ulSrcSize )
        return (uint32_t *) s_aulFirCoeff_Smooth_1toN;
    else if( ulOutSize * 12 >= ulSrcSize * 16 )  /* rounding to 9/16 */
        return (uint32_t *) s_aulFirCoeff_Smooth_16to9;
    else
        return (uint32_t *) s_aulFirCoeff_Smooth_3to1;
}

/*--------------------------------------------------------------------
 * {private}
 * Description:
 * This is the private type for coefficient functions.
*--------------------------------------------------------------------*/
typedef uint32_t *(* BVDC_P_GetFilterCoefFunc)( uint32_t ulSrcSize, uint32_t ulOutSize );

/*--------------------------------------------------------------------
 * {private}
 * Note: The order has to match the def of BVDC_FilterCoeffs
 *-------------------------------------------------------------------*/
static const BVDC_P_GetFilterCoefFunc s_pfnGetFilterCoefficients[] =
{
    BVDC_P_GetFilterCoefficients_Auto_isr,
    BVDC_P_GetFilterCoefficients_PointSample_isr,
    BVDC_P_GetFilterCoefficients_Bilinear_isr,
    BVDC_P_GetFilterCoefficients_Smooth_isr,
    BVDC_P_GetFilterCoefficients_Sharp_isr
};

#define BVDC_P_GFX_PF_TABLE_COUNT \
    (sizeof(s_pfnGetFilterCoefficients)/sizeof(BVDC_P_GetFilterCoefFunc))

/*------------------------------------------------------------------------
 * {private}
 * BVDC_P_GfxFeeder_DecideFilterCoeff_isr
 *
 * output: Hscl filter coeff
 */
BERR_Code BVDC_P_GfxFeeder_DecideFilterCoeff_isr
    ( BVDC_FilterCoeffs     eCoeffs,
      uint32_t              ulCtIndex,
      uint32_t              ulSrcSize,
      uint32_t              ulOutSize,
      uint32_t **           paulCoeff )
{
    if ((eCoeffs <= BVDC_FilterCoeffs_eSharp) && (NULL != paulCoeff))
    {
        if (eCoeffs == BVDC_FilterCoeffs_eSharp && ulCtIndex != 0)
        {
            if (ulCtIndex < BVDC_P_GFX_FIRCOEFF_IDX_START || ulCtIndex > BVDC_P_GFX_FIRCOEFF_IDX_END)
            {
                return BERR_TRACE(BERR_INVALID_PARAMETER);
            }

            *paulCoeff = (uint32_t *)s_paulFirCoeffTbl[ulCtIndex - BVDC_P_GFX_FIRCOEFF_IDX_START];
        }
        else
        {
            *paulCoeff = (* s_pfnGetFilterCoefficients[eCoeffs])(ulSrcSize, ulOutSize);
        }
        return BERR_SUCCESS;
    }
    else
    {
        BDBG_ASSERT( NULL != paulCoeff );
        BDBG_ASSERT( eCoeffs <= BVDC_FilterCoeffs_eSharp );
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
}

#if (BVDC_P_SUPPORT_GFD_VER >= BVDC_P_SUPPORT_GFD_VER_3)
/****************************************************************
 *  GFD VSCL coefficients
 ****************************************************************/

static const uint32_t s_aulVsclFirCoeff_PointSample[] =
{
    BCHP_FIELD_DATA( GFD_0_VERT_FIR_COEFF_PHASE0_00_01, COEFF_0, 0x0000 ) |
    BCHP_FIELD_DATA( GFD_0_VERT_FIR_COEFF_PHASE0_00_01, COEFF_1, 0x0100 ),

    BCHP_FIELD_DATA( GFD_0_VERT_FIR_COEFF_PHASE1_00_01, COEFF_0, 0x0000 ) |
    BCHP_FIELD_DATA( GFD_0_VERT_FIR_COEFF_PHASE1_00_01, COEFF_1, 0x0000 ),
    BVDC_P_SCL_LAST
};

static const uint32_t s_aulVsclFirCoeff_Bilinear[] =
{
    BCHP_FIELD_DATA( GFD_0_VERT_FIR_COEFF_PHASE0_00_01, COEFF_0, 0x0000 ) |
    BCHP_FIELD_DATA( GFD_0_VERT_FIR_COEFF_PHASE0_00_01, COEFF_1, 0x0100 ),

    BCHP_FIELD_DATA( GFD_0_VERT_FIR_COEFF_PHASE1_00_01, COEFF_0, 0x0000 ) |
    BCHP_FIELD_DATA( GFD_0_VERT_FIR_COEFF_PHASE1_00_01, COEFF_1, 0x0080 ),
    BVDC_P_SCL_LAST
};

static const uint32_t s_aulVsclFirCoeff_Sharp_1toN[] =
{
    0x09200370, 0x076000A0,
    BVDC_P_SCL_LAST
};

static const uint32_t s_aulVsclFirCoeff_Sharp_16to9[] =
{
    0x08E00390, 0x075000B0,
    BVDC_P_SCL_LAST
};

static const uint32_t s_aulVsclFirCoeff_Sharp_3to1[] =
{
    0x08E00390, 0x075000B0,
    BVDC_P_SCL_LAST
};


static const uint32_t s_aulVsclFirCoeff_Smooth_1toN[] =
{
    BCHP_FIELD_DATA( GFD_0_VERT_FIR_COEFF_PHASE0_00_01, COEFF_0, 0x0037 ) |
    BCHP_FIELD_DATA( GFD_0_VERT_FIR_COEFF_PHASE0_00_01, COEFF_1, 0x0092 ),

    BCHP_FIELD_DATA( GFD_0_VERT_FIR_COEFF_PHASE1_00_01, COEFF_0, 0x0009 ) |
    BCHP_FIELD_DATA( GFD_0_VERT_FIR_COEFF_PHASE1_00_01, COEFF_1, 0x0077 ),
    BVDC_P_SCL_LAST
};

static const uint32_t s_aulVsclFirCoeff_Smooth_16to9[] =
{
    0x05800540, 0x042003E0,
    BVDC_P_SCL_LAST
};

static const uint32_t s_aulVsclFirCoeff_Smooth_3to1[] =
{
    0x05600550, 0x041003F0,
    BVDC_P_SCL_LAST
};


/****************************************************************
 *  Indexed GFD VSCL coefficients
 ****************************************************************/
static const uint32_t s_paulVsclFirCoeffTbl[][GFD_NUM_REGS_VSCL_COEFF] =
{
    { 0x0A000300, 0x07A00060 },  /* 1 */
    { 0x0A6002D0, 0x07B00050 },  /* 2 */
    { 0x0AA002B0, 0x07C00040 },  /* 3 */
    { 0x0B000280, 0x07D00030 },  /* 4 */
    { 0x0B400260, 0x07E00020 },  /* 5 */
    { 0x0BA00230, 0x07F00010 },  /* 6 */
    { 0x0C000200, 0x08000000 },  /* 7 */
    { 0x0C6001D0, 0x08103FF0 },  /* 8 */
    { 0x0CE00190, 0x08203FE0 },  /* 9 */
    { 0x0D600150, 0x08303FD0 },  /* 10 */
    { 0x0DE00110, 0x08303FD0 },  /* 11 */
    { 0x0E6000D0, 0x08403FC0 },  /* 12 */
    { 0x0EE00090, 0x08403FC0 },  /* 13 */
    { 0x0F600050, 0x08403FC0 },  /* 14 */
    { 0x10000000, 0x08403FC0 },  /* 15 */
    { 0x10603FD0, 0x08403FC0 },  /* 16 */
    { 0x11003F80, 0x08403FC0 },  /* 17 */
    { 0x11603F50, 0x08403FC0 },  /* 18 */
    { 0x11E03F10, 0x08303FD0 },  /* 19 */
    { 0x12403EE0, 0x08303FD0 },  /* 20 */
    { 0x12C03EA0, 0x08203FE0 },  /* 21 */
    { 0x13003E80, 0x08103FF0 },  /* 22 */
    { 0x13403E60, 0x08000000 },  /* 23 */
    { 0x13603E50, 0x08000000 },  /* 24 */
    { 0x13803E40, 0x07F00010 },  /* 25 */
    { 0x13803E40, 0x07E00020 },  /* 26 */
};


/*--------------------------------------------------------------------*/
static uint32_t *BVDC_P_GetVsclFilterCoefficients_Auto_isr(
    uint32_t ulSrcSize,
    uint32_t ulOutSize )
{
    if( ulOutSize == ulSrcSize )
        return (uint32_t *) s_aulVsclFirCoeff_PointSample;
    else if( ulOutSize >= 3 * ulSrcSize ) /* 1to3: 720p -> 4k */
        return (uint32_t *) &s_paulVsclFirCoeffTbl[11];
    else if( ulOutSize >= 2 * ulSrcSize ) /* 1to2: 1080p -> 4k */
        return (uint32_t *) s_paulVsclFirCoeffTbl[11];
    else if( ulOutSize >= ulSrcSize )
        return (uint32_t *) s_aulVsclFirCoeff_Smooth_1toN;
    else if( ulOutSize * 12 >= ulSrcSize * 16 )  /* rounding to 9/16 */
        return (uint32_t *) s_aulVsclFirCoeff_Smooth_16to9;
    else
        return (uint32_t *) s_aulVsclFirCoeff_Smooth_3to1;
}

/*--------------------------------------------------------------------*/
static uint32_t *BVDC_P_GetVsclFilterCoefficients_PointSample_isr(
    uint32_t ulSrcSize,
    uint32_t ulOutSize )
{
    BSTD_UNUSED(ulSrcSize);
    BSTD_UNUSED(ulOutSize);
    return (uint32_t *) s_aulVsclFirCoeff_PointSample;
}

/*--------------------------------------------------------------------*/
static uint32_t *BVDC_P_GetVsclFilterCoefficients_Bilinear_isr(
    uint32_t ulSrcSize,
    uint32_t ulOutSize )
{
    BSTD_UNUSED(ulSrcSize);
    BSTD_UNUSED(ulOutSize);
    return (uint32_t *) s_aulVsclFirCoeff_Bilinear;
}

/*--------------------------------------------------------------------*/
static uint32_t *BVDC_P_GetVsclFilterCoefficients_Sharp_isr(
    uint32_t ulSrcSize,
    uint32_t ulOutSize )
{
    if( ulOutSize == ulSrcSize )
        return (uint32_t *) s_aulVsclFirCoeff_PointSample;
    else if( ulOutSize >= ulSrcSize )
        return (uint32_t *) s_aulVsclFirCoeff_Sharp_1toN;
    else if( ulOutSize * 12 >= ulSrcSize * 16 )  /* rounding to 9/16 */
        return (uint32_t *) s_aulVsclFirCoeff_Sharp_16to9;
    else
        return (uint32_t *) s_aulVsclFirCoeff_Sharp_3to1;
}

/*--------------------------------------------------------------------*/
static uint32_t *BVDC_P_GetVsclFilterCoefficients_Smooth_isr(
    uint32_t ulSrcSize,
    uint32_t ulOutSize )
{
    if( ulOutSize == ulSrcSize )
        return (uint32_t *) s_aulVsclFirCoeff_PointSample;
    else if( ulOutSize >= ulSrcSize )
        return (uint32_t *) s_aulVsclFirCoeff_Smooth_1toN;
    else if( ulOutSize * 12 >= ulSrcSize * 16 )  /* rounding to 9/16 */
        return (uint32_t *) s_aulVsclFirCoeff_Smooth_16to9;
    else
        return (uint32_t *) s_aulVsclFirCoeff_Smooth_3to1;
}

/*--------------------------------------------------------------------
 * {private}
 * Description:
 * This is the private type for coefficient functions.
*--------------------------------------------------------------------*/
typedef uint32_t *(* BVDC_P_GetVsclFilterCoefFunc)( uint32_t ulSrcSize, uint32_t ulOutSize );

/*--------------------------------------------------------------------
 * {private}
 * Note: The order has to match the def of BVDC_FilterCoeffs
 *-------------------------------------------------------------------*/
static const BVDC_P_GetVsclFilterCoefFunc s_pfnGetVsclFirCoefficients[] =
{
    BVDC_P_GetVsclFilterCoefficients_Auto_isr,
    BVDC_P_GetVsclFilterCoefficients_PointSample_isr,
    BVDC_P_GetVsclFilterCoefficients_Bilinear_isr,
    BVDC_P_GetVsclFilterCoefficients_Smooth_isr,
    BVDC_P_GetVsclFilterCoefficients_Sharp_isr
};

#define BVDC_P_GFX_VSCL_PF_TABLE_COUNT \
    (sizeof(s_pfnGetVsclFirCoefficients)/sizeof(BVDC_P_GetVsclFilterCoefFunc))

/*------------------------------------------------------------------------
 * {private}
 * BVDC_P_GfxFeeder_DecideVsclFirCoeff_isr
 *
 * output: filter coeff
 *
 * Note: This implementation is originally copied from bgrc, we should watch
 * bgrc's update and update this code accordingly.
 */
BERR_Code BVDC_P_GfxFeeder_DecideVsclFirCoeff_isr
    ( BVDC_FilterCoeffs     eCoeffs,
      uint32_t              ulCtIndex,
      uint32_t              ulSrcSize,
      uint32_t              ulOutSize,
      uint32_t **           paulCoeff )
{
    if ((eCoeffs <= BVDC_FilterCoeffs_eSharp) && (NULL != paulCoeff))
    {
        if (eCoeffs == BVDC_FilterCoeffs_eSharp && ulCtIndex != 0)
        {
            if (ulCtIndex < BVDC_P_GFX_FIRCOEFF_IDX_START || ulCtIndex > BVDC_P_GFX_FIRCOEFF_IDX_END)
            {
                return BERR_TRACE(BERR_INVALID_PARAMETER);
            }

            *paulCoeff = (uint32_t *)s_paulVsclFirCoeffTbl[ulCtIndex - BVDC_P_GFX_FIRCOEFF_IDX_START];
        }
        else
        {
            *paulCoeff = (*s_pfnGetVsclFirCoefficients[eCoeffs])(ulSrcSize, ulOutSize);
        }
        return BERR_SUCCESS;
    }
    else
    {
        BDBG_ASSERT( NULL != paulCoeff );
        BDBG_ASSERT( eCoeffs <= BVDC_FilterCoeffs_eSharp );
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
}
#endif  /* #if (BVDC_P_SUPPORT_GFD_VER >= BVDC_P_SUPPORT_GFD_VER_3) */

/* End of File */
