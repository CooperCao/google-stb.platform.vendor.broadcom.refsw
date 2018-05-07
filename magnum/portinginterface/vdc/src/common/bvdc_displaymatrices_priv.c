/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

#include "bchp.h"
#include "bvdc_display_priv.h"
#include "bvdc_displayfmt_priv.h"
#include "bvdc_csc_priv.h"

BDBG_MODULE(BVDC_CSC);

/* --------------------------------------------------------
 * this file now only contains matrices for analogue display
 * output;  HDMI output now uses unified CFC code.
*/


/**********************************************************
 * static tables
 **********************************************************/

/************* the followings are the original settings *****************/
/* SDYCrCb_to_SDRGB_480i */
static const BVDC_P_DisplayCscMatrix s_SDYCbCr_to_RGB = BVDC_P_MAKE_VEC_CSC_MATRIX
(
    0,
    4095,
    BVDC_P_MAKE_VEC_RGB_CSC
    (  0.6256, -0.0016,  0.8543, -119.3583,
       0.6256, -0.2087, -0.4378,   72.4649,
       0.6256,  1.0834, -0.0038, -148.6812 )
);

/* calculated by hand since scripts introduce roundoff errors */
static const BVDC_P_DisplayCscMatrix s_SDYCbCr_to_SDYPbPr = BVDC_P_MAKE_VEC_CSC_MATRIX
(
    -4096,
    4095,
    BVDC_P_MAKE_VEC_CSC
    (  0.6267,  0.0000,  0.0000,  -10.0269,
       0.0000,  0.6127,  0.0000,  -78.4286,
       0.0000,  0.0000,  0.6127,  -78.4300 )
);

/* HDYCrCb_to_HDRGB */
static const BVDC_P_DisplayCscMatrix s_HDYCbCr_to_RGB = BVDC_P_MAKE_VEC_CSC_MATRIX
(
    0,
    4095,
    BVDC_P_MAKE_VEC_RGB_CSC
    (  0.6267,  0.0000,  0.9649, -133.5367,
       0.6267, -0.1148, -0.2868,   41.3785,
       0.6267,  1.1370,  0.0000, -155.5594 )
);

/* HDYCrCb_to_HDYPrPb_1080i720p */
#define s_HDYCbCr_to_HDYPbPr s_SDYCbCr_to_SDYPbPr

/* For SECAM Composite/SVideo (SDYCrCb_to_secamYDrDb) */
static const BVDC_P_DisplayCscMatrix s_SDYCbCr_to_YDbDr = BVDC_P_MAKE_VEC_CSC_MATRIX
(
    -4096,
    4095,
    BVDC_P_MAKE_VEC_CSC
    (  0.6267,  0.0000,  0.0000,  -10.0274,
       0.0000,  0.8238,  0.0000, -103.1917,
       0.0000,  0.0000, -1.0040,  128.4742 )
);

#if 0 /* CMP_CSC outputs Smpte_170M for PAL formats due to HDMI out requirement */
/* For PAL Composite/SVideo (SMPTE 170M SDYCrCb to PAL YUV) */
static const BVDC_P_DisplayCscMatrix s_SDYCbCr_to_YUV = BVDC_P_MAKE_VEC_CSC_MATRIX
(
    -4096,
    4095,
    BVDC_P_MAKE_VEC_CSC
    ( 0.6267,  0.01510535,  -0.01877718,  -9.55745459,
      0.0000,  0.53571974,   0.01184764,  -70.0821091,
      0.0000, -0.01962559,   0.65820667,  -81.7247368 )
);

/* For PAL_M Composite/SVideo (SDYCrCb_to_?)
 * I don't have a reference for this matrix.
 */
static const BVDC_P_DisplayCscMatrix s_SDYCbCr_to_YUV_M = BVDC_P_MAKE_VEC_CSC_MATRIX
(
    -4096,
    4095,
    BVDC_P_MAKE_VEC_CSC
    ( 0.5886,  0.014187026, -0.017635633,  1.480175248,
      0.0000,  0.546403929,  0.012083925, -71.48604901,
      0.0000, -0.020015336,  0.671277822, -83.36475779 )
);

/* For PAL_N/NC Composite/SVideo (SDYCrCb_to_nYUV)
   The color space conversion matrix for pal_n is a little
   different from other pal modes, there is a 7.5IRE pedestal
 */
static const BVDC_P_DisplayCscMatrix s_SDYCbCr_to_YUV_N = BVDC_P_MAKE_VEC_CSC_MATRIX
(
    -4096,
    4095,
    BVDC_P_MAKE_VEC_CSC
    ( 0.5886,  0.014187026, -0.017635633,  1.480175248,
      0.0000,  0.546403929,  0.012083925, -71.48604901,
      0.0000, -0.020015336,  0.671277822, -83.36475779 )
);

#else
/* For PAL Composite/SVideo (SDYCrCb_to_YUV) */
static const BVDC_P_DisplayCscMatrix s_SDYCbCr_to_YUV = BVDC_P_MAKE_VEC_CSC_MATRIX
(
    -4096,
    4095,
    BVDC_P_MAKE_VEC_CSC
    ( 0.6267, 0.0000, 0.0000, -10.0274,
      0.0000, 0.5320, 0.0000, -68.0895,
      0.0000, 0.0000, 0.7503, -96.0247 )
);

/* For PAL_M Composite/SVideo (SDYCrCb_to_?)
 * I don't have a reference for this matrix.
 */
static const BVDC_P_DisplayCscMatrix s_SDYCbCr_to_YUV_M = BVDC_P_MAKE_VEC_CSC_MATRIX
(
    -4096,
    4095,
    BVDC_P_MAKE_VEC_CSC
    (  0.5886,  0.0000,  0.0000,   1.0388,
       0.0000,  0.5426,  0.0000, -69.4537,
       0.0000,  0.0000,  0.7652, -97.9487 )
);

/* For PAL_N/NC Composite/SVideo (SDYCrCb_to_nYUV)
   The color space conversion matrix for pal_n is a little
   different from other pal modes, there is a 7.5IRE pedestal
 */
static const BVDC_P_DisplayCscMatrix s_SDYCbCr_to_YUV_N = BVDC_P_MAKE_VEC_CSC_MATRIX
(
    -4096,
    4095,
    BVDC_P_MAKE_VEC_CSC
      ( 0.5886,  0,          0,           1.0388,
        0,       0.54261,    0,         -69.4537,
        0,       0,          0.7652,    -97.9487 )

);
#endif

/* For NTSC Composite/SVideo (SDYCrCb_to_ntscYIQ)
   Color space convert CbYCr to YIQ + 7.5 IRE pedestal on Y
*/
static const BVDC_P_DisplayCscMatrix s_SDYCbCr_to_YIQ = BVDC_P_MAKE_VEC_CSC_MATRIX
(
    -4096,
    4095,
    BVDC_P_MAKE_VEC_YIQ_CSC
    (  0.5913,  0.0000,  0.0000,    1.0388,
       0.0000, -0.2766,  0.5998,  -41.3690,
       0.0000,  0.4200,  0.3891, -103.5615 )
);

/* For NTSC_J Composite/SVideo (SDYCrCb_to_mYIQ)*/
static const BVDC_P_DisplayCscMatrix s_SDYCbCr_to_YIQ_M = BVDC_P_MAKE_VEC_CSC_MATRIX
(
    -4096,
    4095,
    BVDC_P_MAKE_VEC_YIQ_CSC
    (  0.6393,  0.0000,  0.0000,  -10.2283,
       0.0000, -0.2990,  0.6484,  -44.7232,
       0.0000,  0.4541,  0.4206, -111.9584 )
);

#if (BVDC_P_SUPPORT_ITU656_OUT)
/* For 656 output. */
static const BVDC_P_DisplayCscMatrix s_HDYCbCr_to_SDYCbCr_656 = BVDC_P_MAKE_VEC_CSC_MATRIX
(
    -4096,
    4095,
    BVDC_P_MAKE_656_CSC
    (  1.000102,  0.099579,  0.191999, -37.323650,
       0.001012,  0.989664, -0.110798,  15.488960,
      -0.000073, -0.071500,  0.983585,  11.254214 )
);

static const BVDC_P_DisplayCscMatrix s_HD240MYCbCr_to_SDYCbCr_656 = BVDC_P_MAKE_VEC_CSC_MATRIX
(
    -4096,
    4095,
    BVDC_P_MAKE_656_CSC
    (  1.000000,  0.073380,  0.187140, -33.346601,
      -0.000000,  0.988035, -0.108776,  15.454889,
       0.000000, -0.053362,  0.987620,   8.414941 )
);

static const BVDC_P_DisplayCscMatrix s_Identity_656 = BVDC_P_MAKE_VEC_CSC_MATRIX
(
    8,
    1016,
    BVDC_P_MAKE_656_CSC
    (  1.0000,  0.0000,  0.0000,  0.0000,
       0.0000,  1.0000,  0.0000,  0.0000,
       0.0000,  0.0000,  1.0000,  0.0000 )
);
#endif

/* RGB + Hsync matrix */
static const BVDC_P_DisplayCscMatrix s_HsyncMatrix = BVDC_P_MAKE_VEC_CSC_MATRIX
(
    -4096,
    4095,
    BVDC_P_MAKE_VEC_CSC
    (  0.0000,  0.0000,  0.0000,   0.0000,
       0.0000,  0.0000,  0.0000,  16.0000,
       0.0000,  0.0000,  0.0000,  16.0000 )
);

/* XvYCC to no XvYCC matrices for analog output.

Calculated by post-multiplying SDYCbCr_to_XXX matrix by the inverse
of s_CMP_NtscSDYCbCr_to_XvYCCSDYCbCr which strips XvYCC from the source */

#ifndef BVDC_FOR_BOOTUPDATER
/* SDYCrCb_to_SDRGB_480i */
static const BVDC_P_DisplayCscMatrix s_XvYCC_SDYCbCr_to_RGB = BVDC_P_MAKE_VEC_CSC_MATRIX
(
    0,
    4095,
    BVDC_P_MAKE_VEC_RGB_CSC
    (  0.6267, -0.0008,  0.9360, -129.9406,
       0.6267, -0.2350, -0.4715,   80.1133,
       0.6267,  1.0778,  0.0047, -147.8734 )
);

/* SDYCrCb_to_SDYPrPb_480i */
static const BVDC_P_DisplayCscMatrix s_XvYCC_SDYCbCr_to_SDYPbPr = BVDC_P_MAKE_VEC_CSC_MATRIX
(
    -4096,
    4095,
    BVDC_P_MAKE_VEC_CSC
    (  0.6267, -0.0158,  0.0047,  -8.5615,
       0.0000,  0.6176,  0.0025, -78.6586,
       0.0000,  0.0115,  0.6667, -86.8115 )
);

/* For SECAM Composite/SVideo (SDYCrCb_to_secamYDrDb) */
static const BVDC_P_DisplayCscMatrix s_XvYCC_SDYCbCr_to_YDbDr = BVDC_P_MAKE_VEC_CSC_MATRIX
(
    -4096,
    4095,
    BVDC_P_MAKE_VEC_CSC
    (  0.6267, -0.0158,  0.0047,   -8.6184,
       0.0000,  0.8301, -0.0040, -103.4817,
       0.0000, -0.0195, -1.0924,  142.2855 )
);

/* For PAL, PAL_NC Composite/SVideo (SDYCrCb_to_YUV) */
static const BVDC_P_DisplayCscMatrix s_XvYCC_SDYCbCr_to_YUV = BVDC_P_MAKE_VEC_CSC_MATRIX
(
    -4096,
    4095,
    BVDC_P_MAKE_VEC_CSC
    (  0.6267, -0.0158,  0.0047,   -8.6184,
       0.0000,  0.5361, -0.0026,  -68.2768,
       0.0000,  0.0145,  0.8164, -106.3461 )
);

/* For PAL_N/M Composite/SVideo (SDYCrCb_to_nYUV)
   The color space conversion matrix for pal_n is a little
   different from other pal modes, there is a 7.5IRE pedestal
 */
static const BVDC_P_DisplayCscMatrix s_XvYCC_SDYCbCr_to_YUV_N = BVDC_P_MAKE_VEC_CSC_MATRIX
(
    -4096,
    4095,
    BVDC_P_MAKE_VEC_CSC
    (  0.5886, -0.0148,  0.0045,    2.3621,
       0.0000,  0.5467, -0.0026,  -69.6447,
       0.0000,  0.0148,  0.8326, -108.4750 )
);

/* For NTSC Composite/SVideo (SDYCrCb_to_ntscYIQ)
   Color space convert CbYCr to YIQ + 7.5 IRE pedestal on Y
*/
static const BVDC_P_DisplayCscMatrix s_XvYCC_SDYCbCr_to_YIQ = BVDC_P_MAKE_VEC_CSC_MATRIX
(
    -4096,
    4095,
    BVDC_P_MAKE_VEC_YIQ_CSC
    (  0.5913, -0.0149,  0.0045,    2.3682,
       0.0000, -0.2671,  0.6540,  -49.5227,
       0.0000,  0.4307,  0.4213, -109.0619 )
);

/* For NTSC_J Composite/SVideo (SDYCrCb_to_mYIQ)*/
static const BVDC_P_DisplayCscMatrix s_XvYCC_SDYCbCr_to_YIQ_M = BVDC_P_MAKE_VEC_CSC_MATRIX
(
    -4096,
    4095,
    BVDC_P_MAKE_VEC_YIQ_CSC
    (  0.6393, -0.0161,  0.0048,   -8.7910,
       0.0000, -0.2887,  0.7070,  -53.5375,
       0.0000,  0.4657,  0.4554, -117.9042 )
);
#endif


#if (BVDC_P_SUPPORT_ITU656_OUT)
static const BVDC_P_DisplayCscMatrix s_XvYCC_SDYCbCr_to_SDYCbCr_656 = BVDC_P_MAKE_VEC_CSC_MATRIX
(
    8,
    1016,
    BVDC_P_MAKE_656_CSC
    (  1.0000, -0.0251,  0.0076,   2.2482,
       0.0000,  1.0076, -0.0049,  -0.3520,
       0.0000,  0.0194,  1.0881, -13.7563 )
);
#endif

/****************************************************************
 *  Global Tables
 *  3560 might not need to compensate CMP truncation error;
 ****************************************************************/

/* PAL specializations go here: */

/* use normal PAL matrix for PAL-NC: */
#define s_SDYCbCr_to_YUV_NC s_SDYCbCr_to_YUV

/* SDYCbCr CSC table, based on BVDC_P_Output */
static const BVDC_P_DisplayCscMatrix* const s_apSDYCbCr_MatrixTbl[BVDC_P_Output_eMax] =
{
    &s_SDYCbCr_to_YIQ,       /* SVideo, Composite, SC (for NTSC) */
    &s_SDYCbCr_to_YIQ_M,     /* SVideo, Composite, SC (for NTSC_J) */
    &s_SDYCbCr_to_YUV,       /* SVideo, Composite, SC (for generic Pal) */
    &s_SDYCbCr_to_YUV_M,     /* SVideo, Composite, SC (for Pal_M) */
    &s_SDYCbCr_to_YUV_N,     /* SVideo, Composite, SC (for Pal_N) */
    &s_SDYCbCr_to_YUV_NC,    /* SVideo, Composite, SC (for PAL_NC) */
    &s_SDYCbCr_to_YDbDr,     /* SVideo, Composite, SC (for SECAM) */
    &s_SDYCbCr_to_YDbDr,     /* SVideo, Composite, SC (for SECAM) */
    &s_SDYCbCr_to_YDbDr,     /* SVideo, Composite, SC (for SECAM) */
    &s_SDYCbCr_to_SDYPbPr,   /* SYPbPr, CYPbPr, SCYPbPr, SDYPbPr */
    &s_SDYCbCr_to_RGB,       /* RGBSRGB, CRGB, SCRGB, RGB */
};

#ifndef BVDC_FOR_BOOTUPDATER
/* xvYCC SDYCbCr CSC table, based on BVDC_P_Output */
static const BVDC_P_DisplayCscMatrix* const s_apXvYCC_SDYCbCr_MatrixTbl[BVDC_P_Output_eMax] =
{
    &s_XvYCC_SDYCbCr_to_YIQ,       /* SVideo, Composite, SC (for NTSC) */
    &s_XvYCC_SDYCbCr_to_YIQ_M,     /* SVideo, Composite, SC (for NTSC_J) */
    &s_XvYCC_SDYCbCr_to_YUV,       /* SVideo, Composite, SC (for generic Pal) */
    &s_XvYCC_SDYCbCr_to_YUV_N,     /* SVideo, Composite, SC (for Pal_N/M) */
    &s_XvYCC_SDYCbCr_to_YUV,       /* SVideo, Composite, SC (for PAL_NC) */
    &s_XvYCC_SDYCbCr_to_YDbDr,     /* SVideo, Composite, SC (for SECAM) */
    &s_XvYCC_SDYCbCr_to_YDbDr,     /* SVideo, Composite, SC (for SECAM) */
    &s_XvYCC_SDYCbCr_to_YDbDr,     /* SVideo, Composite, SC (for SECAM) */
    &s_XvYCC_SDYCbCr_to_SDYPbPr,   /* SYPbPr, CYPbPr, SCYPbPr, SDYPbPr */
    &s_XvYCC_SDYCbCr_to_RGB,       /* RGBSRGB, CRGB, SCRGB, RGB */
};
#endif

static const BVDC_P_DisplayCscMatrix* const s_apHDYCbCr_MatrixTbl[] =
{
    &s_HDYCbCr_to_RGB,       /* RGBSRGB, CRGB, SCRGB, RGB */
    &s_HDYCbCr_to_HDYPbPr,   /* HDYPbPr */
};


#if (BVDC_P_SUPPORT_ITU656_OUT)
/* SDYCbCr 656 CSC table */
static const BVDC_P_DisplayCscMatrix* const s_apSDYCbCr_656_Bypass_MatrixTbl[] =
{
    &s_HDYCbCr_to_SDYCbCr_656,            /* from HD video source */
    &s_Identity_656,                      /* from NTSC SD video source */
    &s_Identity_656,                      /* from PAL SD video source */
    &s_Identity_656,
    &s_Identity_656,
    &s_Identity_656,
    &s_Identity_656,                      /* from FCC 1953 video source */
    &s_HD240MYCbCr_to_SDYCbCr_656         /* from SMPTE 240M HD video source */
};
#endif

/***************************************************************************
 *
 */
void BVDC_P_Display_GetCscTable_isr
    ( const BVDC_P_DisplayInfo        *pDispInfo,
      BVDC_P_Output                    eOutputColorSpace,
      const BVDC_P_DisplayCscMatrix  **ppCscMatrix )
{

    /* Component only CSC */
    /* Get proper color space conversion table.
     * Note: assume non-bypass displays only have HD color space intput! */
    if(BVDC_P_Output_eHsync == eOutputColorSpace)
    {
        /* RGB + Hsync + Vsync */
        *ppCscMatrix = &s_HsyncMatrix;
    }
    else if(BFMT_IS_27Mhz(pDispInfo->pFmtInfo->ulPxlFreqMask))
    {
        BDBG_ASSERT(eOutputColorSpace <= BVDC_P_Output_eSDRGB);

        /* NTSCs, PALs, 480p and 576p;
           TODO: differentiate NTSC SD and PAL SD color space. */
        *ppCscMatrix =
#ifndef BVDC_FOR_BOOTUPDATER
            (pDispInfo->bXvYcc) ? s_apXvYCC_SDYCbCr_MatrixTbl[eOutputColorSpace] :
#endif /* #ifndef BVDC_FOR_BOOTUPDATER */
            s_apSDYCbCr_MatrixTbl[eOutputColorSpace];
    }
    else
    {
        /* HD output */
        uint32_t   ulIndex = (eOutputColorSpace == BVDC_P_Output_eHDYPrPb) ? 1 : 0;
        if(pDispInfo->eCmpColorimetry <= BCFC_Colorimetry_eSmpte240M)
        {
            *ppCscMatrix = s_apHDYCbCr_MatrixTbl[ulIndex];
        }
        else
        {
            BDBG_ASSERT(pDispInfo->eCmpColorimetry <= BCFC_Colorimetry_eSmpte240M);
        }
    }

    return;
}

/***************************************************************************
 *
 */
#if (BVDC_P_SUPPORT_ITU656_OUT)
void BVDC_P_Display_Get656CscTable_isr
    ( const BVDC_P_DisplayInfo        *pDispInfo,
      bool                             bBypass,
      const BVDC_P_DisplayCscMatrix  **ppCscMatrix )
{
    /* Note the new color space conversion in compositor would always
     * output SD to 656 output. */
    if(pDispInfo->eCmpColorimetry <= BCFC_Colorimetry_eSmpte240M)
    {
        *ppCscMatrix = (bBypass) ?
            s_apSDYCbCr_656_Bypass_MatrixTbl[pDispInfo->eCmpColorimetry] :
            ((pDispInfo->bXvYcc) ? &s_XvYCC_SDYCbCr_to_SDYCbCr_656 :
                                   &s_Identity_656);
    }
    else
    {
        BDBG_ASSERT(pDispInfo->eCmpColorimetry <= BCFC_Colorimetry_eSmpte240M);
    }
    return;
}
#endif

/* End of file */
