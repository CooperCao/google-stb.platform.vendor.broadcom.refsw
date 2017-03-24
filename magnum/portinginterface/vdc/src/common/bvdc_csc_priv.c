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
 *   VDEC/HD_DVI (or similiar hw) CSC matrix coefficients.
 *
 ***************************************************************************/
#include "bstd.h"
#include "bdbg.h"
#include "bvdc_csc_priv.h"
#include "bvdc_displayfmt_priv.h"

BDBG_MODULE(BVDC_CSC);

/* PR30702: Need to enlarge the CSC range for contrast/brightness/saturation/hue". */
#ifndef BVDC_SUPPORT_CONTRAST_WITH_CBCR
#define BVDC_SUPPORT_CONTRAST_WITH_CBCR          (0)
#endif

/* macros to select additive offset from alpha component or offset.*/
#define BVDC_P_CSC_USE_ALPHA(matrix)\
    ((matrix->usYAlpha || matrix->usCbAlpha || matrix->usCrAlpha) ? true : false)

#define BVDC_P_CSC_GET_YO(matrix) \
    (BVDC_P_CSC_USE_ALPHA(matrix) ? matrix->usYAlpha : \
                                    matrix->usYOffset)

#define BVDC_P_CSC_GET_CBO(matrix) \
    (BVDC_P_CSC_USE_ALPHA(matrix) ? matrix->usCbAlpha : \
                                    matrix->usCbOffset)

#define BVDC_P_CSC_GET_CRO(matrix) \
    (BVDC_P_CSC_USE_ALPHA(matrix) ? matrix->usCrAlpha : \
                                    matrix->usCrOffset)

/* alphas are stored as fractionals and are shifted to convert them to
   integers for calcuations */

#define BVDC_P_CSC_YOTOFIX(matrix) \
    (BVDC_P_CSC_USE_ALPHA(matrix) ? BVDC_P_CSC_CXTOFIX(BVDC_P_CSC_GET_YO(matrix)) << BVDC_P_CSC_VIDEO_DATA_BITS : \
                                    BVDC_P_CSC_COTOFIX(BVDC_P_CSC_GET_YO(matrix)))

#define BVDC_P_CSC_CBOTOFIX(matrix) \
    (BVDC_P_CSC_USE_ALPHA(matrix) ? BVDC_P_CSC_CXTOFIX(BVDC_P_CSC_GET_CBO(matrix)) << BVDC_P_CSC_VIDEO_DATA_BITS : \
                                    BVDC_P_CSC_COTOFIX(BVDC_P_CSC_GET_CBO(matrix)))

#define BVDC_P_CSC_CROTOFIX(matrix) \
    (BVDC_P_CSC_USE_ALPHA(matrix) ? BVDC_P_CSC_CXTOFIX(BVDC_P_CSC_GET_CRO(matrix)) << BVDC_P_CSC_VIDEO_DATA_BITS : \
                                    BVDC_P_CSC_COTOFIX(BVDC_P_CSC_GET_CRO(matrix)))


#define BVDC_P_CSC_FIXTOYO(offset, matrix) \
    if (BVDC_P_CSC_USE_ALPHA(matrix)) \
         matrix->usYAlpha  = BVDC_P_CSC_FIXTOCX(offset >> BVDC_P_CSC_VIDEO_DATA_BITS); \
    else matrix->usYOffset = BVDC_P_CSC_FIXTOCO(offset);

#define BVDC_P_CSC_FIXTOCBO(offset, matrix) \
    if (BVDC_P_CSC_USE_ALPHA(matrix)) \
         matrix->usCbAlpha  = BVDC_P_CSC_FIXTOCX(offset >> BVDC_P_CSC_VIDEO_DATA_BITS); \
    else matrix->usCbOffset = BVDC_P_CSC_FIXTOCO(offset);

#define BVDC_P_CSC_FIXTOCRO(offset, matrix) \
    if (BVDC_P_CSC_USE_ALPHA(matrix)) \
         matrix->usCrAlpha  = BVDC_P_CSC_FIXTOCX(offset >> BVDC_P_CSC_VIDEO_DATA_BITS); \
    else matrix->usCrOffset = BVDC_P_CSC_FIXTOCO(offset);

#ifndef BVDC_FOR_BOOTUPDATER

/* Populate with all necessary input <-> output color space conversions.
 * And any special matrices.  Should also include basic common CSC matrices
 * operations. */

/* HD_DVI, MFD, and Csc */
static const BVDC_P_CscCoeffs s_HdDvi_Identity = BVDC_P_MAKE_HDDVI_CSC
    ( 1.0000,  0.0000,  0.0000,   0.0000,
      0.0000,  1.0000,  0.0000,   0.0000,
      0.0000,  0.0000,  1.0000,   0.0000 );

/* The following matrix converts
   from [Limited Range RGB BT.601 / SMPTE 170M / xvYCC 601]
   to   [YCbCr BT.601 / SMPTE 170M / xvYCC 601 ] */
static const BVDC_P_CscCoeffs s_HdDvi_SdRgb_To_SdYCrCb = BVDC_P_MAKE_HDDVI_CSC
    ( 0.298939,  0.586625,  0.114436,   0.000000,
     -0.172638, -0.338777,  0.511416, 128.000000,
      0.511416, -0.427936, -0.083479, 128.000000 );

/* The following matrix converts
   from [Limited Range RGB BT.709 / xvYCC 709]
   to   [YCbCr BT.709 / xvYCC 709 ] */
static const BVDC_P_CscCoeffs s_HdDvi_HdRgb_To_HdYCrCb = BVDC_P_MAKE_HDDVI_CSC
    ( 0.212639,  0.715169,  0.072192,   0.000000,
     -0.117208, -0.394207,  0.511416, 128.000000,
      0.511416, -0.464524, -0.046891, 128.000000 );

/* The following matrix converts
   from [Full Range RGB BT.601 / SMPTE 170M / xvYCC 601]
   to   [YCbCr BT.601 / SMPTE 170M / xvYCC 601 ] */
static const BVDC_P_CscCoeffs s_HdDvi_SdRgb_To_SdYCrCb_FullRange = BVDC_P_MAKE_HDDVI_CSC
    ( 0.255733,  0.501839,  0.097896,  16.000000,
     -0.147686, -0.289814,  0.437500, 128.000000,
      0.437500, -0.366086, -0.071414, 128.000000 );

/* The following matrix converts
   from [Full Range RGB BT.709 / xvYCC 709]
   to   [YCbCr BT.709 / xvYCC 709 ] */
static const BVDC_P_CscCoeffs s_HdDvi_HdRgb_To_HdYCrCb_FullRange = BVDC_P_MAKE_HDDVI_CSC
    ( 0.181906,  0.611804,  0.061758,  16.000000,
     -0.100268, -0.337232,  0.437500, 128.000000,
      0.437500, -0.397386, -0.040114, 128.000000 );

/* Color Temperature - Piecewise Linear Model Parameters - (Assuming Temp is in 100s of K) */
/* Low Temp Model - Under 6500 K
    Atten_R = (0 * Temp + 100)/100
    Atten_G = (0.62695 * Temp + 59.223)/100
    Atten_B = (1.33301 * Temp + 13.333)/100      */
#define BVDC_P_MAKE_CLRTEMP_LMODEL_PARAML(f_bits)              \
    BVDC_P_MAKE_CLRTEMP_LMODEL                                 \
        (  0.00000, 100.000,                                   \
           0.62695,  59.223,                                   \
           1.33301,  13.333,                                   \
           BVDC_P_FIX_MAX_BITS - f_bits, f_bits)

/* High Temp Model - Over 6500 K
    Atten_R = (-0.57422 * Temp + 137.328)/100
    Atten_G = (-0.44727 * Temp + 129.134)/100
    Atten_B = (0 * Temp + 100)/100      */
#define BVDC_P_MAKE_CLRTEMP_LMODEL_PARAMH(f_bits)             \
    BVDC_P_MAKE_CLRTEMP_LMODEL                                \
        ( -0.57422, 137.328,                                  \
          -0.44727, 129.134,                                  \
           0.00000, 100.000,                                  \
           BVDC_P_FIX_MAX_BITS - f_bits, f_bits)

static const int32_t s_ClrTemp_LModel_ParamL_Cmp[3][2] =
    BVDC_P_MAKE_CLRTEMP_LMODEL_PARAML(BVDC_P_CSC_CMP_CX_F_BITS);

static const int32_t s_ClrTemp_LModel_ParamH_Cmp[3][2] =
    BVDC_P_MAKE_CLRTEMP_LMODEL_PARAMH(BVDC_P_CSC_CMP_CX_F_BITS);

static const int32_t s_ClrTemp_LModel_ParamL_Gfd[3][2] =
    BVDC_P_MAKE_CLRTEMP_LMODEL_PARAML(BVDC_P_CSC_GFD_CX_F_BITS);

static const int32_t s_ClrTemp_LModel_ParamH_Gfd[3][2] =
    BVDC_P_MAKE_CLRTEMP_LMODEL_PARAMH(BVDC_P_CSC_GFD_CX_F_BITS);

#define BVDC_P_CSC_FLOATING_POINT_MSG 0
/***************************************************************************
 * Print out what the CSC look like.
 */
void BVDC_P_Csc_Print_isr
    ( const BVDC_P_CscCoeffs          *pCscCoeffs )
{
#if BDBG_DEBUG_BUILD
    /* Let's what they look like */
    BDBG_MSG(("[cx_i=%d, cx_f=%d, co_i=%d, co_f=%d]:",
        pCscCoeffs->usCxIntBits, pCscCoeffs->usCxFractBits, pCscCoeffs->usCoIntBits, pCscCoeffs->usCoFractBits));
    BDBG_MSG(("[[0x%04x 0x%04x 0x%04x 0x%04x 0x%04x]",
        pCscCoeffs->usY0, pCscCoeffs->usY1, pCscCoeffs->usY2, pCscCoeffs->usYAlpha, pCscCoeffs->usYOffset));
    BDBG_MSG((" [0x%04x 0x%04x 0x%04x 0x%04x 0x%04x]",
        pCscCoeffs->usCb0, pCscCoeffs->usCb1, pCscCoeffs->usCb2, pCscCoeffs->usCbAlpha, pCscCoeffs->usCbOffset));
    BDBG_MSG((" [0x%04x 0x%04x 0x%04x 0x%04x 0x%04x]]",
        pCscCoeffs->usCr0, pCscCoeffs->usCr1, pCscCoeffs->usCr2, pCscCoeffs->usCrAlpha, pCscCoeffs->usCrOffset));

/* uses floats, build only for debugging purposes */
#if BVDC_P_CSC_FLOATING_POINT_MSG
    BDBG_MSG(("i.e."));
    BDBG_MSG(("[[%0f %04f %04f %04f %04f]",
        BVDC_P_CSC_CXTOF(pCscCoeffs->usY0), BVDC_P_CSC_CXTOF(pCscCoeffs->usY1), BVDC_P_CSC_CXTOF(pCscCoeffs->usY2), BVDC_P_CSC_CXTOF(pCscCoeffs->usYAlpha), BVDC_P_CSC_COTOF(pCscCoeffs->usYOffset)));
    BDBG_MSG((" [%04f %04f %04f %04f %04f]",
        BVDC_P_CSC_CXTOF(pCscCoeffs->usCb0), BVDC_P_CSC_CXTOF(pCscCoeffs->usCb1), BVDC_P_CSC_CXTOF(pCscCoeffs->usCb2), BVDC_P_CSC_CXTOF(pCscCoeffs->usCbAlpha), BVDC_P_CSC_COTOF(pCscCoeffs->usCbOffset)));
    BDBG_MSG((" [%04f %04f %04f %04f %04f]]",
        BVDC_P_CSC_CXTOF(pCscCoeffs->usCr0), BVDC_P_CSC_CXTOF(pCscCoeffs->usCr1), BVDC_P_CSC_CXTOF(pCscCoeffs->usCr2), BVDC_P_CSC_CXTOF(pCscCoeffs->usCrAlpha), BVDC_P_CSC_COTOF(pCscCoeffs->usCrOffset)));
#endif

#else
    BSTD_UNUSED(pCscCoeffs);
#endif
    return;
}

/***************************************************************************
 * Multiplies two csc matrices set up in 4x4 format.  pCscCoeffs contains
 * bit shift settings and alpha usage information.
 */
static void BVDC_P_Csc_Mult4X4_isr
    ( int32_t                          aalMatrixRet[4][4],
      int32_t                          aalMatrix1[4][4],
      int32_t                          aalMatrix2[4][4],
      BVDC_P_CscCoeffs                *pCscCoeffs)
{
    int i, j, k;

    for (i = 0; i < 4; i++)
    {
        for (j = 0; j < 4; j++)
        {
            aalMatrixRet[i][j] = 0;
            for (k = 0; k < 4; k++)
            {
                if (j == 3)
                {
                    aalMatrixRet[i][j] += BVDC_P_CSC_FIX_MUL_OFFSET(aalMatrix1[i][k], aalMatrix2[k][j], pCscCoeffs);
                }
                else
                {
                    aalMatrixRet[i][j] += BVDC_P_CSC_FIX_MUL(aalMatrix1[i][k], aalMatrix2[k][j]);
                }
            }
        }
    }
}

/* TODO: add real s_HdDvi_UhdRgb_To_UhdYCrCb_FullRange and s_HdDvi_UhdRgb_To_UhdYCrCb */
#define s_HdDvi_UhdRgb_To_UhdYCrCb_FullRange  s_HdDvi_HdRgb_To_HdYCrCb_FullRange
#define s_HdDvi_UhdRgb_To_UhdYCrCb            s_HdDvi_HdRgb_To_HdYCrCb
/***************************************************************************
 * Return the desired color space coverstion for CSC in HdDvi.
 */
void BVDC_P_Csc_GetHdDviTable_isr
    ( BVDC_P_CscCoeffs                *pCsc,
      BAVC_CscMode                     eCscMode )
{
    switch(eCscMode)
    {
    case BAVC_CscMode_e2020RgbFullRange:
        *pCsc = s_HdDvi_UhdRgb_To_UhdYCrCb_FullRange;
        break;

    case BAVC_CscMode_e2020RgbLimitedRange:
        *pCsc = s_HdDvi_UhdRgb_To_UhdYCrCb;
        break;

    case BAVC_CscMode_e709RgbFullRange:
        *pCsc = s_HdDvi_HdRgb_To_HdYCrCb_FullRange;
        break;

    case BAVC_CscMode_e709RgbLimitedRange:
        *pCsc = s_HdDvi_HdRgb_To_HdYCrCb;
        break;

    case BAVC_CscMode_e601RgbFullRange:
        *pCsc = s_HdDvi_SdRgb_To_SdYCrCb_FullRange;
        break;

    case BAVC_CscMode_e601RgbLimitedRange:
        *pCsc = s_HdDvi_SdRgb_To_SdYCrCb;
        break;

    case BAVC_CscMode_e709YCbCr:
    case BAVC_CscMode_e601YCbCr:
    case BAVC_CscMode_eMax:
    default:
        BDBG_MSG(("Unknown csc use identity: %d", eCscMode));
        *pCsc = s_HdDvi_Identity;
    }

    /* Let's what they look like */
    BDBG_MSG(("Hd Dvi Matrix:"));
    BVDC_P_Csc_Print_isr(pCsc);

    return;
}

/***************************************************************************
 * Return the color space converstion for CSC in HDDVI or CMP from user matrix.
 */
void BVDC_P_Csc_FromMatrix_isr
    ( BVDC_P_CscCoeffs                *pCsc,
      const int32_t                    pl32_Matrix[BVDC_CSC_COEFF_COUNT],
      uint32_t                         ulShift )
{
    pCsc->usY0       = BVDC_P_FR_USR_MATRIX(pl32_Matrix[0],  ulShift, pCsc->usCxIntBits, pCsc->usCxFractBits);
    pCsc->usY1       = BVDC_P_FR_USR_MATRIX(pl32_Matrix[1],  ulShift, pCsc->usCxIntBits, pCsc->usCxFractBits);
    pCsc->usY2       = BVDC_P_FR_USR_MATRIX(pl32_Matrix[2],  ulShift, pCsc->usCxIntBits, pCsc->usCxFractBits);
    pCsc->usYAlpha   = BVDC_P_FR_USR_MATRIX(pl32_Matrix[3],  ulShift, pCsc->usCxIntBits, pCsc->usCxFractBits);
    pCsc->usYOffset  = BVDC_P_FR_USR_MATRIX(pl32_Matrix[4],  ulShift, pCsc->usCoIntBits, pCsc->usCoFractBits);

    pCsc->usCb0      = BVDC_P_FR_USR_MATRIX(pl32_Matrix[5],  ulShift, pCsc->usCxIntBits, pCsc->usCxFractBits);
    pCsc->usCb1      = BVDC_P_FR_USR_MATRIX(pl32_Matrix[6],  ulShift, pCsc->usCxIntBits, pCsc->usCxFractBits);
    pCsc->usCb2      = BVDC_P_FR_USR_MATRIX(pl32_Matrix[7],  ulShift, pCsc->usCxIntBits, pCsc->usCxFractBits);
    pCsc->usCbAlpha  = BVDC_P_FR_USR_MATRIX(pl32_Matrix[8],  ulShift, pCsc->usCxIntBits, pCsc->usCxFractBits);
    pCsc->usCbOffset = BVDC_P_FR_USR_MATRIX(pl32_Matrix[9],  ulShift, pCsc->usCoIntBits, pCsc->usCoFractBits);

    pCsc->usCr0      = BVDC_P_FR_USR_MATRIX(pl32_Matrix[10], ulShift, pCsc->usCxIntBits, pCsc->usCxFractBits);
    pCsc->usCr1      = BVDC_P_FR_USR_MATRIX(pl32_Matrix[11], ulShift, pCsc->usCxIntBits, pCsc->usCxFractBits);
    pCsc->usCr2      = BVDC_P_FR_USR_MATRIX(pl32_Matrix[12], ulShift, pCsc->usCxIntBits, pCsc->usCxFractBits);
    pCsc->usCrAlpha  = BVDC_P_FR_USR_MATRIX(pl32_Matrix[13], ulShift, pCsc->usCxIntBits, pCsc->usCxFractBits);
    pCsc->usCrOffset = BVDC_P_FR_USR_MATRIX(pl32_Matrix[14], ulShift, pCsc->usCoIntBits, pCsc->usCoFractBits);

    /* Let's what they look like */
    BDBG_MSG(("User Matrix:"));
    BVDC_P_Csc_Print_isr(pCsc);

    return;
}


/***************************************************************************
 * Return the user matrix from vdec color space coverstion table.
 */
void BVDC_P_Csc_ToMatrix_isr
    ( int32_t                          pl32_Matrix[BVDC_CSC_COEFF_COUNT],
      const BVDC_P_CscCoeffs          *pCsc,
      uint32_t                         ulShift )
{
    pl32_Matrix[0]  = BVDC_P_TO_USR_MATRIX(pCsc->usY0,       pCsc->usCxIntBits, pCsc->usCxFractBits, ulShift);
    pl32_Matrix[1]  = BVDC_P_TO_USR_MATRIX(pCsc->usY1,       pCsc->usCxIntBits, pCsc->usCxFractBits, ulShift);
    pl32_Matrix[2]  = BVDC_P_TO_USR_MATRIX(pCsc->usY2,       pCsc->usCxIntBits, pCsc->usCxFractBits, ulShift);
    pl32_Matrix[3]  = BVDC_P_TO_USR_MATRIX(pCsc->usYAlpha,   pCsc->usCxIntBits, pCsc->usCxFractBits, ulShift);
    pl32_Matrix[4]  = BVDC_P_TO_USR_MATRIX(pCsc->usYOffset,  pCsc->usCoIntBits, pCsc->usCoFractBits, ulShift);

    pl32_Matrix[5]  = BVDC_P_TO_USR_MATRIX(pCsc->usCb0,      pCsc->usCxIntBits, pCsc->usCxFractBits, ulShift);
    pl32_Matrix[6]  = BVDC_P_TO_USR_MATRIX(pCsc->usCb1,      pCsc->usCxIntBits, pCsc->usCxFractBits, ulShift);
    pl32_Matrix[7]  = BVDC_P_TO_USR_MATRIX(pCsc->usCb2,      pCsc->usCxIntBits, pCsc->usCxFractBits, ulShift);
    pl32_Matrix[8]  = BVDC_P_TO_USR_MATRIX(pCsc->usCbAlpha,  pCsc->usCxIntBits, pCsc->usCxFractBits, ulShift);
    pl32_Matrix[9]  = BVDC_P_TO_USR_MATRIX(pCsc->usCbOffset, pCsc->usCoIntBits, pCsc->usCoFractBits, ulShift);

    pl32_Matrix[10] = BVDC_P_TO_USR_MATRIX(pCsc->usCr0,      pCsc->usCxIntBits, pCsc->usCxFractBits, ulShift);
    pl32_Matrix[11] = BVDC_P_TO_USR_MATRIX(pCsc->usCr1,      pCsc->usCxIntBits, pCsc->usCxFractBits, ulShift);
    pl32_Matrix[12] = BVDC_P_TO_USR_MATRIX(pCsc->usCr2,      pCsc->usCxIntBits, pCsc->usCxFractBits, ulShift);
    pl32_Matrix[13] = BVDC_P_TO_USR_MATRIX(pCsc->usCrAlpha,  pCsc->usCxIntBits, pCsc->usCxFractBits, ulShift);
    pl32_Matrix[14] = BVDC_P_TO_USR_MATRIX(pCsc->usCrOffset, pCsc->usCoIntBits, pCsc->usCoFractBits, ulShift);

    return;
}


/***************************************************************************
 * Return the color space converstion for CSC in HDDVI or CMP from user matrix.
 * User matrices are converted from user YCbCr->RGB format to Dvo hardware
 * csc format of CbYCr->GBR
 */
void BVDC_P_Csc_FromMatrixDvo_isr
    ( BVDC_P_CscCoeffs                *pCsc,
      const int32_t                    pl32_Matrix[BVDC_CSC_COEFF_COUNT],
      uint32_t                         ulShift,
      bool                             bRgb )
{
    if (bRgb)
    {
        /* R */
        pCsc->usCr0      = BVDC_P_FR_USR_MATRIX(pl32_Matrix[1], ulShift, pCsc->usCxIntBits, pCsc->usCxFractBits);
        pCsc->usCr1      = BVDC_P_FR_USR_MATRIX(pl32_Matrix[0], ulShift, pCsc->usCxIntBits, pCsc->usCxFractBits);
        pCsc->usCr2      = BVDC_P_FR_USR_MATRIX(pl32_Matrix[2], ulShift, pCsc->usCxIntBits, pCsc->usCxFractBits);
        pCsc->usCrAlpha  = BVDC_P_FR_USR_MATRIX(pl32_Matrix[3], ulShift, pCsc->usCxIntBits, pCsc->usCxFractBits);
        pCsc->usCrOffset = BVDC_P_FR_USR_MATRIX(pl32_Matrix[4], ulShift, pCsc->usCoIntBits, pCsc->usCoFractBits);

        /* G */
        pCsc->usY0       = BVDC_P_FR_USR_MATRIX(pl32_Matrix[6],  ulShift, pCsc->usCxIntBits, pCsc->usCxFractBits);
        pCsc->usY1       = BVDC_P_FR_USR_MATRIX(pl32_Matrix[5],  ulShift, pCsc->usCxIntBits, pCsc->usCxFractBits);
        pCsc->usY2       = BVDC_P_FR_USR_MATRIX(pl32_Matrix[7],  ulShift, pCsc->usCxIntBits, pCsc->usCxFractBits);
        pCsc->usYAlpha   = BVDC_P_FR_USR_MATRIX(pl32_Matrix[8],  ulShift, pCsc->usCxIntBits, pCsc->usCxFractBits);
        pCsc->usYOffset  = BVDC_P_FR_USR_MATRIX(pl32_Matrix[9],  ulShift, pCsc->usCoIntBits, pCsc->usCoFractBits);

        /* B */
        pCsc->usCb0      = BVDC_P_FR_USR_MATRIX(pl32_Matrix[11],  ulShift, pCsc->usCxIntBits, pCsc->usCxFractBits);
        pCsc->usCb1      = BVDC_P_FR_USR_MATRIX(pl32_Matrix[10],  ulShift, pCsc->usCxIntBits, pCsc->usCxFractBits);
        pCsc->usCb2      = BVDC_P_FR_USR_MATRIX(pl32_Matrix[12],  ulShift, pCsc->usCxIntBits, pCsc->usCxFractBits);
        pCsc->usCbAlpha  = BVDC_P_FR_USR_MATRIX(pl32_Matrix[13],  ulShift, pCsc->usCxIntBits, pCsc->usCxFractBits);
        pCsc->usCbOffset = BVDC_P_FR_USR_MATRIX(pl32_Matrix[14],  ulShift, pCsc->usCoIntBits, pCsc->usCoFractBits);
    }
    else
    {
        pCsc->usY0       = BVDC_P_FR_USR_MATRIX(pl32_Matrix[1],  ulShift, pCsc->usCxIntBits, pCsc->usCxFractBits);
        pCsc->usY1       = BVDC_P_FR_USR_MATRIX(pl32_Matrix[0],  ulShift, pCsc->usCxIntBits, pCsc->usCxFractBits);
        pCsc->usY2       = BVDC_P_FR_USR_MATRIX(pl32_Matrix[2],  ulShift, pCsc->usCxIntBits, pCsc->usCxFractBits);
        pCsc->usYAlpha   = BVDC_P_FR_USR_MATRIX(pl32_Matrix[3],  ulShift, pCsc->usCxIntBits, pCsc->usCxFractBits);
        pCsc->usYOffset  = BVDC_P_FR_USR_MATRIX(pl32_Matrix[4],  ulShift, pCsc->usCoIntBits, pCsc->usCoFractBits);

        pCsc->usCb0      = BVDC_P_FR_USR_MATRIX(pl32_Matrix[6],  ulShift, pCsc->usCxIntBits, pCsc->usCxFractBits);
        pCsc->usCb1      = BVDC_P_FR_USR_MATRIX(pl32_Matrix[5],  ulShift, pCsc->usCxIntBits, pCsc->usCxFractBits);
        pCsc->usCb2      = BVDC_P_FR_USR_MATRIX(pl32_Matrix[7],  ulShift, pCsc->usCxIntBits, pCsc->usCxFractBits);
        pCsc->usCbAlpha  = BVDC_P_FR_USR_MATRIX(pl32_Matrix[8],  ulShift, pCsc->usCxIntBits, pCsc->usCxFractBits);
        pCsc->usCbOffset = BVDC_P_FR_USR_MATRIX(pl32_Matrix[9],  ulShift, pCsc->usCoIntBits, pCsc->usCoFractBits);

        pCsc->usCr0      = BVDC_P_FR_USR_MATRIX(pl32_Matrix[11], ulShift, pCsc->usCxIntBits, pCsc->usCxFractBits);
        pCsc->usCr1      = BVDC_P_FR_USR_MATRIX(pl32_Matrix[10], ulShift, pCsc->usCxIntBits, pCsc->usCxFractBits);
        pCsc->usCr2      = BVDC_P_FR_USR_MATRIX(pl32_Matrix[12], ulShift, pCsc->usCxIntBits, pCsc->usCxFractBits);
        pCsc->usCrAlpha  = BVDC_P_FR_USR_MATRIX(pl32_Matrix[13], ulShift, pCsc->usCxIntBits, pCsc->usCxFractBits);
        pCsc->usCrOffset = BVDC_P_FR_USR_MATRIX(pl32_Matrix[14], ulShift, pCsc->usCoIntBits, pCsc->usCoFractBits);
    }

    /* Let's what they look like */
    BDBG_MSG(("User DVO Matrix:"));
    BVDC_P_Csc_Print_isr(pCsc);

    return;
}

/***************************************************************************
 * Apply the contrast calculation to the color matrix
 */
void BVDC_P_Csc_ApplyContrast_isr
    ( int16_t                          sContrast,
      BVDC_P_CscCoeffs                *pCscCoeffs )
{
    int32_t lFixK;
    int32_t lFixKMin = BVDC_P_CONTRAST_FIX_K_MIN;
    int32_t lFixKMax = BVDC_P_CONTRAST_FIX_K_MAX;
    int32_t lFixOne = BVDC_P_CSC_ITOFIX(1);

    int32_t lFixYOffset;
#if (BVDC_SUPPORT_CONTRAST_WITH_CBCR)
    int32_t lFixCbOffset;
    int32_t lFixCrOffset;
#endif

    BDBG_MSG(("Apply contrast = %d:", sContrast));
    BDBG_MSG(("Input CSC : %x %x %x %x %x, %x %x %x %x %x, %x %x %x %x %x",
        pCscCoeffs->usY0, pCscCoeffs->usY1, pCscCoeffs->usY2, pCscCoeffs->usYAlpha, pCscCoeffs->usYOffset,
        pCscCoeffs->usCb0, pCscCoeffs->usCb1, pCscCoeffs->usCb2, pCscCoeffs->usCbAlpha, pCscCoeffs->usCbOffset,
        pCscCoeffs->usCr0, pCscCoeffs->usCr1, pCscCoeffs->usCr2, pCscCoeffs->usCrAlpha, pCscCoeffs->usCrOffset));

    /* K of 1.0 is no contrast adjustment.
     * K changes linearly from Kmin to 1 with input contrast from -32768 to 0
     */
    if (sContrast <= 0)
    {
        lFixK = (((lFixOne - lFixKMin) * (sContrast - BVDC_P_CONTRAST_VAL_MIN)) /
                 -BVDC_P_CONTRAST_VAL_MIN
                )
                + lFixKMin;
    }
    /* K changes linearly from slightly greater than 1.0 to KMax with input contrast from 1 to 32767 */
    else
    {
        lFixK = (((lFixKMax - lFixOne) * sContrast) /
                 BVDC_P_CONTRAST_VAL_MAX
                )
                + lFixOne;
    }

    lFixYOffset  = BVDC_P_CSC_FIX_MUL_OFFSET(lFixK, BVDC_P_CSC_YOTOFIX(pCscCoeffs), pCscCoeffs) +
                   (BVDC_P_LUMA_BLACK_OFFSET * (lFixOne - lFixK));

#if (BVDC_SUPPORT_CONTRAST_WITH_CBCR)
    lFixCbOffset = BVDC_P_CSC_FIX_MUL_OFFSET(lFixK, BVDC_P_CSC_CBOTOFIX(pCscCoeffs), pCscCoeffs) +
                   (BVDC_P_CHROMA_BLACK_OFFSET * (lFixOne - lFixK));

    lFixCrOffset = BVDC_P_CSC_FIX_MUL_OFFSET(lFixK, BVDC_P_CSC_CROTOFIX(pCscCoeffs), pCscCoeffs) +
                   (BVDC_P_CHROMA_BLACK_OFFSET * (lFixOne - lFixK));
#endif

    /* Y */
    pCscCoeffs->usY0 = BVDC_P_CSC_FIXTOCX(
                                 BVDC_P_CSC_FIX_MUL(lFixK, BVDC_P_CSC_CXTOFIX(pCscCoeffs->usY0)));
    pCscCoeffs->usY1 = BVDC_P_CSC_FIXTOCX(
                                 BVDC_P_CSC_FIX_MUL(lFixK, BVDC_P_CSC_CXTOFIX(pCscCoeffs->usY1)));
    pCscCoeffs->usY2 = BVDC_P_CSC_FIXTOCX(
                                 BVDC_P_CSC_FIX_MUL(lFixK, BVDC_P_CSC_CXTOFIX(pCscCoeffs->usY2)));

    BVDC_P_CSC_FIXTOYO(lFixYOffset, pCscCoeffs);

#if (BVDC_SUPPORT_CONTRAST_WITH_CBCR)
    /* Cb */
    pCscCoeffs->usCb0 = BVDC_P_CSC_FIXTOCX(
                                 BVDC_P_CSC_FIX_MUL(lFixK, BVDC_P_CSC_CXTOFIX(pCscCoeffs->usCb0)));
    pCscCoeffs->usCb1 = BVDC_P_CSC_FIXTOCX(
                                 BVDC_P_CSC_FIX_MUL(lFixK, BVDC_P_CSC_CXTOFIX(pCscCoeffs->usCb1)));
    pCscCoeffs->usCb2 = BVDC_P_CSC_FIXTOCX(
                                 BVDC_P_CSC_FIX_MUL(lFixK, BVDC_P_CSC_CXTOFIX(pCscCoeffs->usCb2)));

    BVDC_P_CSC_FIXTOCBO(lFixCbOffset, pCscCoeffs);

    /* Cr */
    pCscCoeffs->usCr0 = BVDC_P_CSC_FIXTOCX(
                                 BVDC_P_CSC_FIX_MUL(lFixK, BVDC_P_CSC_CXTOFIX(pCscCoeffs->usCr0)));
    pCscCoeffs->usCr1 = BVDC_P_CSC_FIXTOCX(
                                 BVDC_P_CSC_FIX_MUL(lFixK, BVDC_P_CSC_CXTOFIX(pCscCoeffs->usCr1)));
    pCscCoeffs->usCr2 = BVDC_P_CSC_FIXTOCX(
                                 BVDC_P_CSC_FIX_MUL(lFixK, BVDC_P_CSC_CXTOFIX(pCscCoeffs->usCr2)));

    BVDC_P_CSC_FIXTOCRO(lFixCrOffset, pCscCoeffs);
#endif

    BDBG_MSG(("Output CSC: %x %x %x %x %x, %x %x %x %x %x, %x %x %x %x %x",
        pCscCoeffs->usY0, pCscCoeffs->usY1, pCscCoeffs->usY2, pCscCoeffs->usYAlpha, pCscCoeffs->usYOffset,
        pCscCoeffs->usCb0, pCscCoeffs->usCb1, pCscCoeffs->usCb2, pCscCoeffs->usCbAlpha, pCscCoeffs->usCbOffset,
        pCscCoeffs->usCr0, pCscCoeffs->usCr1, pCscCoeffs->usCr2, pCscCoeffs->usCrAlpha, pCscCoeffs->usCrOffset));

    return;
}


/***************************************************************************
 * Apply the Saturation and Hue calculation to color matrix
 */
void BVDC_P_Csc_ApplySaturationAndHue_isr
    ( int16_t                          sSaturation,
      int16_t                          sHue,
      BVDC_P_CscCoeffs                *pCscCoeffs )
{
    int32_t lTmpCb0;
    int32_t lTmpCb1;
    int32_t lTmpCb2;
    int32_t lTmpCbOffset;

    int32_t lTmpCr0;
    int32_t lTmpCr1;
    int32_t lTmpCr2;
    int32_t lTmpCrOffset;

    int32_t lFixKa;
    int32_t lFixKt;

    int32_t lFixKaMax = BVDC_P_SATURATION_FIX_KA_MAX;
    int32_t lFixKhMax = BVDC_P_HUE_FIX_KH_MAX;

    int32_t lFixKSinKt;
    int32_t lFixKCosKt;
    int32_t lFixC0;
    int32_t lFixC1;

    int32_t lFixOne = BVDC_P_CSC_ITOFIX(1);

    BDBG_MSG(("Apply sat = %d, hue = %d:", sSaturation, sHue));
    BDBG_MSG(("Input CSC : %x %x %x %x %x, %x %x %x %x %x, %x %x %x %x %x",
        pCscCoeffs->usY0, pCscCoeffs->usY1, pCscCoeffs->usY2, pCscCoeffs->usYAlpha, pCscCoeffs->usYOffset,
        pCscCoeffs->usCb0, pCscCoeffs->usCb1, pCscCoeffs->usCb2, pCscCoeffs->usCbAlpha, pCscCoeffs->usCbOffset,
        pCscCoeffs->usCr0, pCscCoeffs->usCr1, pCscCoeffs->usCr2, pCscCoeffs->usCrAlpha, pCscCoeffs->usCrOffset));

    /**
     * Ka of 1.0 is no saturation adjustment.
     * Ka changes linearly with input saturation value, from 0 to 1.0 when saturation
     * is negative, and from 1.0 to 4 when saturation is positive.
     *
     * With KaMax = 4, minimum saturation = -32768, maximum saturation = 32767:
     * -32768 input saturation equals Ka of 0
     *      0 input saturation equals Ka of 1
     *  32767 input saturation equals Ka of 4
     */

    if (sSaturation <= 0)
    {
        lFixKa = (lFixOne * (sSaturation - BVDC_P_CONTRAST_VAL_MIN)) /
                 -BVDC_P_SATURATION_VAL_MIN;
    }
    else
    {
        lFixKa = ((((lFixKaMax - lFixOne) * sSaturation) /
                   BVDC_P_SATURATION_VAL_MAX))
                 + lFixOne;
    }

    /**
     * Kt of 0 is no hue adjustment.
     * Kt changes linearly with input hue value, bounded by +/- Khmax.
     * hue of -32768 is clamped to -32767.
     *
     * With KhMax = pi, minimum hue = -32767, maximum hue = 32767:
     * -32767 input hue equals Kt of -pi
     *      0 input hue equals Kt of 0
     *  32767 input hue equals Kt of pi
     */
    if (sHue < -BVDC_P_HUE_VAL_MAX)
    {
        sHue = -BVDC_P_HUE_VAL_MAX;
    }

    lFixKt = (lFixKhMax * sHue) / BVDC_P_HUE_VAL_MAX;

    lFixKSinKt = BVDC_P_CSC_FIX_MUL(lFixKa , BVDC_P_CSC_FIX_SIN(lFixKt));
    lFixKCosKt = BVDC_P_CSC_FIX_MUL(lFixKa , BVDC_P_CSC_FIX_COS(lFixKt));

    /* offset coefficient is stored in integer format */
    lFixC0 = ((1 << (BVDC_P_CSC_VIDEO_DATA_BITS - 1)) * (lFixKSinKt - lFixKCosKt + lFixOne));
    lFixC1 = ((1 << (BVDC_P_CSC_VIDEO_DATA_BITS - 1)) * (-lFixKSinKt - lFixKCosKt + lFixOne));

    lTmpCb0      = BVDC_P_CSC_FIX_MUL(lFixKCosKt, BVDC_P_CSC_CXTOFIX(pCscCoeffs->usCb0)) -
                   BVDC_P_CSC_FIX_MUL(lFixKSinKt, BVDC_P_CSC_CXTOFIX(pCscCoeffs->usCr0));
    lTmpCb1      = BVDC_P_CSC_FIX_MUL(lFixKCosKt, BVDC_P_CSC_CXTOFIX(pCscCoeffs->usCb1)) -
                   BVDC_P_CSC_FIX_MUL(lFixKSinKt, BVDC_P_CSC_CXTOFIX(pCscCoeffs->usCr1));
    lTmpCb2      = BVDC_P_CSC_FIX_MUL(lFixKCosKt, BVDC_P_CSC_CXTOFIX(pCscCoeffs->usCb2)) -
                   BVDC_P_CSC_FIX_MUL(lFixKSinKt, BVDC_P_CSC_CXTOFIX(pCscCoeffs->usCr2));
    lTmpCbOffset = BVDC_P_CSC_FIX_MUL_OFFSET(lFixKCosKt, BVDC_P_CSC_CBOTOFIX(pCscCoeffs), pCscCoeffs) -
                   BVDC_P_CSC_FIX_MUL_OFFSET(lFixKSinKt, BVDC_P_CSC_CROTOFIX(pCscCoeffs), pCscCoeffs) +
                   lFixC0;

    lTmpCr0      = BVDC_P_CSC_FIX_MUL(lFixKSinKt, BVDC_P_CSC_CXTOFIX(pCscCoeffs->usCb0)) +
                   BVDC_P_CSC_FIX_MUL(lFixKCosKt, BVDC_P_CSC_CXTOFIX(pCscCoeffs->usCr0));
    lTmpCr1      = BVDC_P_CSC_FIX_MUL(lFixKSinKt, BVDC_P_CSC_CXTOFIX(pCscCoeffs->usCb1)) +
                   BVDC_P_CSC_FIX_MUL(lFixKCosKt, BVDC_P_CSC_CXTOFIX(pCscCoeffs->usCr1));
    lTmpCr2      = BVDC_P_CSC_FIX_MUL(lFixKSinKt, BVDC_P_CSC_CXTOFIX(pCscCoeffs->usCb2)) +
                   BVDC_P_CSC_FIX_MUL(lFixKCosKt, BVDC_P_CSC_CXTOFIX(pCscCoeffs->usCr2));
    lTmpCrOffset = BVDC_P_CSC_FIX_MUL_OFFSET(lFixKSinKt, BVDC_P_CSC_CBOTOFIX(pCscCoeffs), pCscCoeffs) +
                   BVDC_P_CSC_FIX_MUL_OFFSET(lFixKCosKt, BVDC_P_CSC_CROTOFIX(pCscCoeffs), pCscCoeffs) +
                   lFixC1;

    pCscCoeffs->usCb0      = BVDC_P_CSC_FIXTOCX(lTmpCb0);
    pCscCoeffs->usCb1      = BVDC_P_CSC_FIXTOCX(lTmpCb1);
    pCscCoeffs->usCb2      = BVDC_P_CSC_FIXTOCX(lTmpCb2);
    BVDC_P_CSC_FIXTOCBO(lTmpCbOffset, pCscCoeffs);

    pCscCoeffs->usCr0      = BVDC_P_CSC_FIXTOCX(lTmpCr0);
    pCscCoeffs->usCr1      = BVDC_P_CSC_FIXTOCX(lTmpCr1);
    pCscCoeffs->usCr2      = BVDC_P_CSC_FIXTOCX(lTmpCr2);
    BVDC_P_CSC_FIXTOCRO(lTmpCrOffset, pCscCoeffs);

    BDBG_MSG(("Output CSC: %x %x %x %x %x, %x %x %x %x %x, %x %x %x %x %x",
        pCscCoeffs->usY0, pCscCoeffs->usY1, pCscCoeffs->usY2, pCscCoeffs->usYAlpha, pCscCoeffs->usYOffset,
        pCscCoeffs->usCb0, pCscCoeffs->usCb1, pCscCoeffs->usCb2, pCscCoeffs->usCbAlpha, pCscCoeffs->usCbOffset,
        pCscCoeffs->usCr0, pCscCoeffs->usCr1, pCscCoeffs->usCr2, pCscCoeffs->usCrAlpha, pCscCoeffs->usCrOffset));

    return;
}


/***************************************************************************
 * Apply brightness calculation to color matrix
 */
void BVDC_P_Csc_ApplyBrightness_isr
    ( int16_t                          sBrightness,
      BVDC_P_CscCoeffs                *pCscCoeffs )
{
    int16_t sK;

    BDBG_MSG(("Apply brightness = %d:", sBrightness));
    BDBG_MSG(("Input CSC : %x %x %x %x %x, %x %x %x %x %x, %x %x %x %x %x",
        pCscCoeffs->usY0, pCscCoeffs->usY1, pCscCoeffs->usY2, pCscCoeffs->usYAlpha, pCscCoeffs->usYOffset,
        pCscCoeffs->usCb0, pCscCoeffs->usCb1, pCscCoeffs->usCb2, pCscCoeffs->usCbAlpha, pCscCoeffs->usCbOffset,
        pCscCoeffs->usCr0, pCscCoeffs->usCr1, pCscCoeffs->usCr2, pCscCoeffs->usCrAlpha, pCscCoeffs->usCrOffset));

    /* brightness of -32768 clamped to -32767. */
    if (sBrightness < -BVDC_P_BRIGHTNESS_VAL_MAX)
    {
        sBrightness = -BVDC_P_BRIGHTNESS_VAL_MAX;
    }

    /* sK varies linearly from -KMax to KMax based on input brightness */
    if (BVDC_P_CSC_USE_ALPHA(pCscCoeffs))
    {
        sK = ((sBrightness << (BVDC_P_CX_FRACTION_BITS - BVDC_P_CSC_VIDEO_DATA_BITS)) *
              BVDC_P_BRIGHTNESS_K_MAX) / BVDC_P_BRIGHTNESS_VAL_MAX;

        pCscCoeffs->usYAlpha += sK;
        pCscCoeffs->usYAlpha &= BVDC_P_CX_MASK;
    }
    else
    {
        sK = ((sBrightness << BVDC_P_CO_FRACTION_BITS) *
              BVDC_P_BRIGHTNESS_K_MAX) / BVDC_P_BRIGHTNESS_VAL_MAX;

        pCscCoeffs->usYOffset += sK;
        pCscCoeffs->usYOffset &= BVDC_P_CO_MASK;
    }

    BDBG_MSG(("Output CSC: %x %x %x %x %x, %x %x %x %x %x, %x %x %x %x %x",
        pCscCoeffs->usY0, pCscCoeffs->usY1, pCscCoeffs->usY2, pCscCoeffs->usYAlpha, pCscCoeffs->usYOffset,
        pCscCoeffs->usCb0, pCscCoeffs->usCb1, pCscCoeffs->usCb2, pCscCoeffs->usCbAlpha, pCscCoeffs->usCbOffset,
        pCscCoeffs->usCr0, pCscCoeffs->usCr1, pCscCoeffs->usCr2, pCscCoeffs->usCrAlpha, pCscCoeffs->usCrOffset));

    return;
}


/***************************************************************************
 * Apply RGB attenuation calculation to color matrix
 */
void BVDC_P_Csc_ApplyAttenuationRGB_isr
    ( int32_t                          lAttenuationR,
      int32_t                          lAttenuationG,
      int32_t                          lAttenuationB,
      int32_t                          lOffsetR,
      int32_t                          lOffsetG,
      int32_t                          lOffsetB,
      BVDC_P_CscCoeffs                *pCscCoeffs,
      const BVDC_P_CscCoeffs          *pYCbCrToRGB,
      const BVDC_P_CscCoeffs          *pRGBToYCbCr,
      bool                             bUserCsc)
{
    int32_t M0[4][4];
    int32_t M1[4][4];
    int32_t M2[4][4];
    int32_t MTmp[4][4];

    BDBG_ASSERT(pCscCoeffs);
    BDBG_ASSERT(pYCbCrToRGB);
    BDBG_ASSERT(pRGBToYCbCr);

    BDBG_MSG(("Apply RGB Attenuation"));
    BDBG_MSG(("Attenuation R=%d, G=%d, B=%d, Offset R=%d, G=%d, B=%d:",
        lAttenuationR, lAttenuationG, lAttenuationB, lOffsetR, lOffsetG, lOffsetB));
    BDBG_MSG(("YCbCrToRGB matrix:"));
    BVDC_P_Csc_Print_isr(pYCbCrToRGB);
    BDBG_MSG(("RGBToYCbCr matrix:"));
    BVDC_P_Csc_Print_isr(pRGBToYCbCr);
    BDBG_MSG(("Input CSC : %x %x %x %x %x, %x %x %x %x %x, %x %x %x %x %x",
        pCscCoeffs->usY0, pCscCoeffs->usY1, pCscCoeffs->usY2, pCscCoeffs->usYAlpha, pCscCoeffs->usYOffset,
        pCscCoeffs->usCb0, pCscCoeffs->usCb1, pCscCoeffs->usCb2, pCscCoeffs->usCbAlpha, pCscCoeffs->usCbOffset,
        pCscCoeffs->usCr0, pCscCoeffs->usCr1, pCscCoeffs->usCr2, pCscCoeffs->usCrAlpha, pCscCoeffs->usCrOffset));
    BVDC_P_Csc_Print_isr(pCscCoeffs);

    if(lAttenuationB == (int32_t)BVDC_P_CSC_ITOFIX(1) &&
       lAttenuationG == (int32_t)BVDC_P_CSC_ITOFIX(1) &&
       lAttenuationR == (int32_t)BVDC_P_CSC_ITOFIX(1) &&
       lOffsetB == 0 &&
       lOffsetG == 0 &&
       lOffsetR == 0)
    {
        BDBG_MSG(("Output CSC: %x %x %x %x %x, %x %x %x %x %x, %x %x %x %x %x",
            pCscCoeffs->usY0, pCscCoeffs->usY1, pCscCoeffs->usY2, pCscCoeffs->usYAlpha, pCscCoeffs->usYOffset,
            pCscCoeffs->usCb0, pCscCoeffs->usCb1, pCscCoeffs->usCb2, pCscCoeffs->usCbAlpha, pCscCoeffs->usCbOffset,
            pCscCoeffs->usCr0, pCscCoeffs->usCr1, pCscCoeffs->usCr2, pCscCoeffs->usCrAlpha, pCscCoeffs->usCrOffset));

        BVDC_P_Csc_Print_isr(pCscCoeffs);

        return;
    }
    else if (bUserCsc)
    {
        BDBG_ERR(("Color Temp or RGB Attenuation adjustment requires BVDC_Window_SetColorMatrixNonLinearC to be set when BVDC_Window_SetColorMatrix is used."));
        return;
    }

    /* M0 = Original CSC Matrix */
    BVDC_P_CSC_MAKE4X4(M0, pCscCoeffs);

    /* M1 = YCrCb to RGB Matrix */
    BVDC_P_CSC_MAKE4X4(M1, pYCbCrToRGB);

    M1[0][0] = BVDC_P_CSC_FIX_MUL(lAttenuationR, M1[0][0]);
    M1[0][1] = BVDC_P_CSC_FIX_MUL(lAttenuationR, M1[0][1]);
    M1[0][2] = BVDC_P_CSC_FIX_MUL(lAttenuationR, M1[0][2]);
    M1[0][3] = BVDC_P_CSC_FIX_MUL_OFFSET(lAttenuationR, M1[0][3], pCscCoeffs) + lOffsetR ;

    M1[1][0] = BVDC_P_CSC_FIX_MUL(lAttenuationG, M1[1][0]);
    M1[1][1] = BVDC_P_CSC_FIX_MUL(lAttenuationG, M1[1][1]);
    M1[1][2] = BVDC_P_CSC_FIX_MUL(lAttenuationG, M1[1][2]);
    M1[1][3] = BVDC_P_CSC_FIX_MUL_OFFSET(lAttenuationG, M1[1][3], pCscCoeffs) + lOffsetG ;

    M1[2][0] = BVDC_P_CSC_FIX_MUL(lAttenuationB, M1[2][0]);
    M1[2][1] = BVDC_P_CSC_FIX_MUL(lAttenuationB, M1[2][1]);
    M1[2][2] = BVDC_P_CSC_FIX_MUL(lAttenuationB, M1[2][2]);
    M1[2][3] = BVDC_P_CSC_FIX_MUL_OFFSET(lAttenuationB, M1[2][3], pCscCoeffs) + lOffsetB ;

    /* M2 = RGB to YCrCb Matrix */
    BVDC_P_CSC_MAKE4X4(M2, pRGBToYCbCr);

    /* Multiply M2*M1 -> Store in MTmp  */
    BVDC_P_Csc_Mult4X4_isr(MTmp, M2, M1, pCscCoeffs);

    /* Multiply MTmp*M0 -> store in M1 */
    BVDC_P_Csc_Mult4X4_isr(M1, MTmp, M0, pCscCoeffs);

    pCscCoeffs->usY0  = BVDC_P_CSC_FIXTOCX(M1[0][0]);
    pCscCoeffs->usY1  = BVDC_P_CSC_FIXTOCX(M1[0][1]);
    pCscCoeffs->usY2  = BVDC_P_CSC_FIXTOCX(M1[0][2]);
    BVDC_P_CSC_FIXTOYO(M1[0][3], pCscCoeffs);

    pCscCoeffs->usCb0 = BVDC_P_CSC_FIXTOCX(M1[1][0]);
    pCscCoeffs->usCb1 = BVDC_P_CSC_FIXTOCX(M1[1][1]);
    pCscCoeffs->usCb2 = BVDC_P_CSC_FIXTOCX(M1[1][2]);
    BVDC_P_CSC_FIXTOCBO(M1[1][3], pCscCoeffs);

    pCscCoeffs->usCr0 = BVDC_P_CSC_FIXTOCX(M1[2][0]);
    pCscCoeffs->usCr1 = BVDC_P_CSC_FIXTOCX(M1[2][1]);
    pCscCoeffs->usCr2 = BVDC_P_CSC_FIXTOCX(M1[2][2]);
    BVDC_P_CSC_FIXTOCRO(M1[2][3], pCscCoeffs);

    BDBG_MSG(("Output CSC: %x %x %x %x %x, %x %x %x %x %x, %x %x %x %x %x",
        pCscCoeffs->usY0, pCscCoeffs->usY1, pCscCoeffs->usY2, pCscCoeffs->usYAlpha, pCscCoeffs->usYOffset,
        pCscCoeffs->usCb0, pCscCoeffs->usCb1, pCscCoeffs->usCb2, pCscCoeffs->usCbAlpha, pCscCoeffs->usCbOffset,
        pCscCoeffs->usCr0, pCscCoeffs->usCr1, pCscCoeffs->usCr2, pCscCoeffs->usCrAlpha, pCscCoeffs->usCrOffset));

    BVDC_P_Csc_Print_isr(pCscCoeffs);

    return;
}


/***************************************************************************
 * Convert color temperature to attenuation RGB
 */
BERR_Code BVDC_P_Csc_ColorTempToAttenuationRGB
    ( int16_t                          sColorTemp,
      int32_t                         *plAttenuationR,
      int32_t                         *plAttenuationG,
      int32_t                         *plAttenuationB,
      BVDC_P_CscCoeffs                *pCscCoeffs )

{
    int32_t lSlope, lKelvin;
    int32_t lAttenuationR;
    int32_t lAttenuationG;
    int32_t lAttenuationB;

    /* Maximum and Minimum Values of Brightness Slider
      (Presumably these would be BVDC_P_BRIGHTNESS_VAL_MAX and BVDC_P_BRIGHTNESS_VAL_MIN,
       however this is not the case at the moment) */

    int32_t lColorTempMax = BVDC_P_COLORTEMP_VAL_MAX;
    int32_t lColorTempMin = BVDC_P_COLORTEMP_VAL_MIN;
    int32_t lColorTempCenter;

    /* Maximum, Minimum, and Center Values of Color Temperature in 100s of Kelvin */
    int32_t lKelvinMax    = BVDC_P_COLORTEMP_KELVIN_MAX;
    int32_t lKelvinMin    = BVDC_P_COLORTEMP_KELVIN_MIN;
    int32_t lKelvinCenter = BVDC_P_COLORTEMP_KELVIN_CENTER;


    /* Color Temperature - Piecewise Linear Model Parameters - (Assuming Temp is in 100s of K) */
    int32_t lModel_ParamL[3][2];
    int32_t lModel_ParamH[3][2];
    int i, j;

    for (i = 0; i < 3; i++)
    {
        for (j = 0; j < 2; j++)
        {
            if ((BVDC_P_CX_FRACTION_BITS == BVDC_P_CSC_CMP_CX_F_BITS) &&
                (BVDC_P_CX_INT_BITS == BVDC_P_CSC_CMP_CX_I_BITS))
            {
                lModel_ParamL[i][j] = s_ClrTemp_LModel_ParamL_Cmp[i][j];
                lModel_ParamH[i][j] = s_ClrTemp_LModel_ParamH_Cmp[i][j];
            }
            else if ((BVDC_P_CX_FRACTION_BITS == BVDC_P_CSC_GFD_CX_F_BITS) &&
                     (BVDC_P_CX_INT_BITS == BVDC_P_CSC_GFD_CX_I_BITS))
            {
                lModel_ParamL[i][j] = s_ClrTemp_LModel_ParamL_Gfd[i][j];
                lModel_ParamH[i][j] = s_ClrTemp_LModel_ParamH_Gfd[i][j];
            }
            else
            {
                BDBG_MSG(("Colortemp only supported for CMP and GFD."));
                return BERR_INVALID_PARAMETER;
            }
        }
    }

    BDBG_ENTER(BVDC_P_Csc_ColorTempToAttenuationRGB);

    lColorTempCenter = (lColorTempMin + lColorTempMax)/2;

    if(sColorTemp < lColorTempCenter) {
        lSlope = (lColorTempCenter - lColorTempMin)/(lKelvinCenter - lKelvinMin);
    }
    else {
        lSlope = (BVDC_P_COLORTEMP_VAL_MAX - lColorTempCenter)/(lKelvinMax - lKelvinCenter);
    }

    lKelvin = sColorTemp/lSlope - lColorTempCenter/lSlope + lKelvinCenter;

    /* Determine Attenuation Factors Using Piecewise Linear Model of Color Temperature */
    if(lKelvin < lKelvinCenter) {
        lAttenuationR = ((lModel_ParamL[0][0] * lKelvin) + lModel_ParamL[0][1]) / 100;
        lAttenuationG = ((lModel_ParamL[1][0] * lKelvin) + lModel_ParamL[1][1]) / 100;
        lAttenuationB = ((lModel_ParamL[2][0] * lKelvin) + lModel_ParamL[2][1]) / 100;
    }
    else
    {
        lAttenuationR = ((lModel_ParamH[0][0] * lKelvin) + lModel_ParamH[0][1]) / 100;
        lAttenuationG = ((lModel_ParamH[1][0] * lKelvin) + lModel_ParamH[1][1]) / 100;
        lAttenuationB = ((lModel_ParamH[2][0] * lKelvin) + lModel_ParamH[2][1]) / 100;
    }

    /* Ensure Attenuation Factors are Between 0 and 1 */
    lAttenuationR = (lAttenuationR < 0) ? 0 : (lAttenuationR > (int32_t)BVDC_P_CSC_ITOFIX(1)) ? (int32_t)BVDC_P_CSC_ITOFIX(1) : lAttenuationR;
    lAttenuationG = (lAttenuationG < 0) ? 0 : (lAttenuationG > (int32_t)BVDC_P_CSC_ITOFIX(1)) ? (int32_t)BVDC_P_CSC_ITOFIX(1) : lAttenuationG;
    lAttenuationB = (lAttenuationB < 0) ? 0 : (lAttenuationB > (int32_t)BVDC_P_CSC_ITOFIX(1)) ? (int32_t)BVDC_P_CSC_ITOFIX(1) : lAttenuationB;

    *plAttenuationR = lAttenuationR;
    *plAttenuationG = lAttenuationG;
    *plAttenuationB = lAttenuationB;

    BDBG_LEAVE(BVDC_P_Csc_ColorTempToAttenuationRGB);
    return BERR_SUCCESS;
}


/***************************************************************************
 * Apply RGB attenuation calculation to color matrix, performed in RGB
 * colorspace
 */
void BVDC_P_Csc_DvoApplyAttenuationRGB_isr
    ( int32_t                          lAttenuationR,
      int32_t                          lAttenuationG,
      int32_t                          lAttenuationB,
      int32_t                          lOffsetR,
      int32_t                          lOffsetG,
      int32_t                          lOffsetB,
      BVDC_P_CscCoeffs                *pCscCoeffs )
{
    int32_t lNewOffsetR;
    int32_t lNewOffsetG;
    int32_t lNewOffsetB;

    BDBG_MSG(("Apply Dvo RGB Attenuation"));
    BDBG_MSG(("Attenuation R=%d, G=%d, B=%d, Offset R=%d, G=%d, B=%d:",
        lAttenuationR, lAttenuationG, lAttenuationB, lOffsetR, lOffsetG, lOffsetB));
    BDBG_MSG(("Input CSC : %x %x %x %x %x, %x %x %x %x %x, %x %x %x %x %x",
        pCscCoeffs->usY0, pCscCoeffs->usY1, pCscCoeffs->usY2, pCscCoeffs->usYAlpha, pCscCoeffs->usYOffset,
        pCscCoeffs->usCb0, pCscCoeffs->usCb1, pCscCoeffs->usCb2, pCscCoeffs->usCbAlpha, pCscCoeffs->usCbOffset,
        pCscCoeffs->usCr0, pCscCoeffs->usCr1, pCscCoeffs->usCr2, pCscCoeffs->usCrAlpha, pCscCoeffs->usCrOffset));

    if(lAttenuationB == (int32_t)BVDC_P_CSC_ITOFIX(1) &&
       lAttenuationG == (int32_t)BVDC_P_CSC_ITOFIX(1) &&
       lAttenuationR == (int32_t)BVDC_P_CSC_ITOFIX(1) &&
       lOffsetB == 0 &&
       lOffsetG == 0 &&
       lOffsetR == 0)
    {
        BDBG_MSG(("Output CSC: %x %x %x %x %x, %x %x %x %x %x, %x %x %x %x %x",
            pCscCoeffs->usY0, pCscCoeffs->usY1, pCscCoeffs->usY2, pCscCoeffs->usYAlpha, pCscCoeffs->usYOffset,
            pCscCoeffs->usCb0, pCscCoeffs->usCb1, pCscCoeffs->usCb2, pCscCoeffs->usCbAlpha, pCscCoeffs->usCbOffset,
            pCscCoeffs->usCr0, pCscCoeffs->usCr1, pCscCoeffs->usCr2, pCscCoeffs->usCrAlpha, pCscCoeffs->usCrOffset));

        return;
    }

    lNewOffsetR = BVDC_P_CSC_FIX_MUL_OFFSET(lAttenuationR, BVDC_P_CSC_CROTOFIX(pCscCoeffs), pCscCoeffs) + lOffsetR;
    lNewOffsetG = BVDC_P_CSC_FIX_MUL_OFFSET(lAttenuationG, BVDC_P_CSC_YOTOFIX(pCscCoeffs),  pCscCoeffs) + lOffsetG;
    lNewOffsetB = BVDC_P_CSC_FIX_MUL_OFFSET(lAttenuationB, BVDC_P_CSC_CBOTOFIX(pCscCoeffs), pCscCoeffs) + lOffsetB;

    /* R */
    pCscCoeffs->usCr0 = BVDC_P_CSC_FIXTOCX(BVDC_P_CSC_FIX_MUL(lAttenuationR, BVDC_P_CSC_CXTOFIX(pCscCoeffs->usCr0)));
    pCscCoeffs->usCr1 = BVDC_P_CSC_FIXTOCX(BVDC_P_CSC_FIX_MUL(lAttenuationR, BVDC_P_CSC_CXTOFIX(pCscCoeffs->usCr1)));
    pCscCoeffs->usCr2 = BVDC_P_CSC_FIXTOCX(BVDC_P_CSC_FIX_MUL(lAttenuationR, BVDC_P_CSC_CXTOFIX(pCscCoeffs->usCr2)));
    BVDC_P_CSC_FIXTOCRO(lNewOffsetR, pCscCoeffs);

    /* G */
    pCscCoeffs->usY0  = BVDC_P_CSC_FIXTOCX(BVDC_P_CSC_FIX_MUL(lAttenuationG, BVDC_P_CSC_CXTOFIX(pCscCoeffs->usY0)));
    pCscCoeffs->usY1  = BVDC_P_CSC_FIXTOCX(BVDC_P_CSC_FIX_MUL(lAttenuationG, BVDC_P_CSC_CXTOFIX(pCscCoeffs->usY1)));
    pCscCoeffs->usY2  = BVDC_P_CSC_FIXTOCX(BVDC_P_CSC_FIX_MUL(lAttenuationG, BVDC_P_CSC_CXTOFIX(pCscCoeffs->usY2)));
    BVDC_P_CSC_FIXTOYO(lNewOffsetG, pCscCoeffs);

    /* B */
    pCscCoeffs->usCb0 = BVDC_P_CSC_FIXTOCX(BVDC_P_CSC_FIX_MUL(lAttenuationB, BVDC_P_CSC_CXTOFIX(pCscCoeffs->usCb0)));
    pCscCoeffs->usCb1 = BVDC_P_CSC_FIXTOCX(BVDC_P_CSC_FIX_MUL(lAttenuationB, BVDC_P_CSC_CXTOFIX(pCscCoeffs->usCb1)));
    pCscCoeffs->usCb2 = BVDC_P_CSC_FIXTOCX(BVDC_P_CSC_FIX_MUL(lAttenuationB, BVDC_P_CSC_CXTOFIX(pCscCoeffs->usCb2)));
    BVDC_P_CSC_FIXTOCBO(lNewOffsetB, pCscCoeffs);

    BDBG_MSG(("Output CSC: %x %x %x %x %x, %x %x %x %x %x, %x %x %x %x %x",
        pCscCoeffs->usY0, pCscCoeffs->usY1, pCscCoeffs->usY2, pCscCoeffs->usYAlpha, pCscCoeffs->usYOffset,
        pCscCoeffs->usCb0, pCscCoeffs->usCb1, pCscCoeffs->usCb2, pCscCoeffs->usCbAlpha, pCscCoeffs->usCbOffset,
        pCscCoeffs->usCr0, pCscCoeffs->usCr1, pCscCoeffs->usCr2, pCscCoeffs->usCrAlpha, pCscCoeffs->usCrOffset));

    return;
}


#if 0
/***************************************************************************
 * Calculate inverse of matrix
 */
void BVDC_P_Csc_MatrixInverse
    ( BVDC_P_CscCoeffs                *pCscCoeffs,
      BVDC_P_CscCoeffs                *pRetInvCscCoeffs )
{
    BMTH_FIX_Matrix stMatrix;
    BMTH_FIX_Matrix stInvMatrix;

    BDBG_MSG(("Calculate Inverse Matrix"));
    BDBG_MSG(("Input Matrix:"));
    BVDC_P_Csc_Print_isr(pCscCoeffs);

    /* Convert matrices to BMTH_FIX_Matrix format */
    BVDC_P_CSC_MAKE4X4_MTH(stMatrix, pCscCoeffs);

    stInvMatrix.ulSize = 4;
    stInvMatrix.ulFractBits = BVDC_P_CX_FRACTION_BITS;
    BKNI_Memset(stInvMatrix.data, 0, sizeof(stInvMatrix.data));

    BMTH_FIX_Matrix_Inverse(&stMatrix, &stInvMatrix);

    /* Convert result to csc coeff matrix format */
    pRetInvCscCoeffs->usY0  = BVDC_P_CSC_FIXTOCX(stInvMatrix.data[0][0]);
    pRetInvCscCoeffs->usY1  = BVDC_P_CSC_FIXTOCX(stInvMatrix.data[0][1]);
    pRetInvCscCoeffs->usY2  = BVDC_P_CSC_FIXTOCX(stInvMatrix.data[0][2]);
    BVDC_P_CSC_FIXTOYO(stInvMatrix.data[0][3], pRetInvCscCoeffs);

    pRetInvCscCoeffs->usCb0 = BVDC_P_CSC_FIXTOCX(stInvMatrix.data[1][0]);
    pRetInvCscCoeffs->usCb1 = BVDC_P_CSC_FIXTOCX(stInvMatrix.data[1][1]);
    pRetInvCscCoeffs->usCb2 = BVDC_P_CSC_FIXTOCX(stInvMatrix.data[1][2]);
    BVDC_P_CSC_FIXTOCBO(stInvMatrix.data[1][3], pRetInvCscCoeffs);

    pRetInvCscCoeffs->usCr0 = BVDC_P_CSC_FIXTOCX(stInvMatrix.data[2][0]);
    pRetInvCscCoeffs->usCr1 = BVDC_P_CSC_FIXTOCX(stInvMatrix.data[2][1]);
    pRetInvCscCoeffs->usCr2 = BVDC_P_CSC_FIXTOCX(stInvMatrix.data[2][2]);
    BVDC_P_CSC_FIXTOCRO(stInvMatrix.data[2][3], pRetInvCscCoeffs);

    BDBG_MSG(("Inverse Matrix:"));
    BVDC_P_Csc_Print_isr(pRetInvCscCoeffs);

    return;
}
#endif

#endif /* #ifndef BVDC_FOR_BOOTUPDATER */

static void BVDC_P_Csc_MatrixMultVector_isr
    ( BVDC_P_CscCoeffs                *pCscCoeffs,
      uint32_t                         aVector[4],
      uint32_t                         aRetVector[4])
{
    BMTH_FIX_Vector stVector;
    BMTH_FIX_Vector stRetVector;
    BMTH_FIX_Matrix stMatrix;
    uint32_t i;

    BVDC_P_CSC_MAKE4X4_MTH(stMatrix, pCscCoeffs);

    stVector.ulSize = 4;
    stVector.ulFractBits = BVDC_P_CX_FRACTION_BITS;

    for (i = 0; i < 4; i++)
    {
        stVector.data[i] = aVector[i];
    }

    BMTH_FIX_Matrix_MultVector(&stMatrix, &stVector, &stRetVector);

    for (i=0; i < 4; i++)
    {
        aRetVector[i] = stRetVector.data[i];
    }
}

/***************************************************************************
 * Set a matrix to output specified color in its original colorspace.
 */
void BVDC_P_Csc_ApplyYCbCrColor_isr
    ( BVDC_P_CscCoeffs                *pCscCoeffs,
      uint32_t                         ulColor0,
      uint32_t                         ulColor1,
      uint32_t                         ulColor2 )
{
    uint32_t aulColorVector[4];
    uint32_t aulRetColorVector[4];

    BDBG_MSG(("Apply YCbCr Color"));
    BDBG_MSG(("ulColor0=%d, ulColor1=%d, ulColor2=%d", ulColor0, ulColor1, ulColor2));
    BDBG_MSG(("Input Matrix:"));
    BVDC_P_Csc_Print_isr(pCscCoeffs);

    aulColorVector[0] = ulColor0;
    aulColorVector[1] = ulColor1;
    aulColorVector[2] = ulColor2;
    aulColorVector[3] = 1;

    /* multiply before we shift to fract bits to avoid overflow */
    BVDC_P_Csc_MatrixMultVector_isr(pCscCoeffs, aulColorVector, aulRetColorVector);

    pCscCoeffs->usY0 = 0;
    pCscCoeffs->usY1 = 0;
    pCscCoeffs->usY2 = 0;
    pCscCoeffs->usYOffset = aulRetColorVector[0] << pCscCoeffs->usCoFractBits;
    pCscCoeffs->usCb0 = 0;
    pCscCoeffs->usCb1 = 0;
    pCscCoeffs->usCb2 = 0;
    pCscCoeffs->usCbOffset = aulRetColorVector[1] << pCscCoeffs->usCoFractBits;
    pCscCoeffs->usCr0 = 0;
    pCscCoeffs->usCr1 = 0;
    pCscCoeffs->usCr2 = 0;
    pCscCoeffs->usCrOffset = aulRetColorVector[2] << pCscCoeffs->usCoFractBits;

    BDBG_MSG(("Output Matrix:"));
    BVDC_P_Csc_Print_isr(pCscCoeffs);
}

/* End of file */
