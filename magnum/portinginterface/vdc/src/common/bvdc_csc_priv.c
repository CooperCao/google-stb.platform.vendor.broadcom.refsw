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
#if (BVDC_P_SUPPORT_HDDVI)
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
#endif

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

#if (BVDC_P_SUPPORT_HDDVI)
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
#endif

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
