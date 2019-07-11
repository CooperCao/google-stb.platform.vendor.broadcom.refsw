/***************************************************************************
 * Copyright (C) 2018 Broadcom.
 * The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to
 * the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied),
 * right to use, or waiver of any kind with respect to the Software, and
 * Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 * THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 * IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use all
 * reasonable efforts to protect the confidentiality thereof, and to use this
 * information only in connection with your use of Broadcom integrated circuit
 * products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 * OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 * RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 * IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 * A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 * ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 * THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 * OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 * INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 * RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 * HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 * EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 * WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 * FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 * Module Description:
 *
 ***************************************************************************/
#include "bstd.h"
#include "bdbg.h"
#include "bvdc_priv.h"
#include "bvdc_adjrect_priv.h"

BDBG_MODULE(BVDC_ADJRECT);
BDBG_FILE_MODULE(BVDC_ASP_RAT);

/***************************************************************************
 * Utility function to set default value for unknown aspect ratio
 */
void BVDC_P_SetDefaultAspRatio_isrsafe(
    BFMT_AspectRatio                *peFullAspectRatio,
    uint32_t                         ulSampleAspectRatioX,
    uint32_t                         ulSampleAspectRatioY,
    uint32_t                         ulFullWidth,
    uint32_t                         ulFullHeight
)
{
    if(BVDC_P_IS_UNKNOWN_ASPR(*peFullAspectRatio, ulSampleAspectRatioX, ulSampleAspectRatioY))
    {
        uint32_t ulHVRatio = (ulFullWidth * 100) / ulFullHeight;
        *peFullAspectRatio = BVDC_P_EQ_DELTA(ulHVRatio, 130, 25)
            ? BFMT_AspectRatio_e4_3 : BFMT_AspectRatio_eSquarePxl;
    }
}

/***************************************************************************
 *
 * Utility function called by BVDC_P_AspectRatioCorrection_isrsafe and
 * BVDC_P_Window_CalcuUserDisplaySize_isr, and BVDC_P_Display_CalPixelAspectRatio_isr
 * to calculate the U4.16 fixed point format aspect ratio of a PIXEL
 *
 * note: pixel asp ratio range is well bounded ( <16, i.e. 4 int bits ), so
 * calcu it first, and also it could have more frac bits than the asp ratio
 * of a sub-rect (that is not well bounded).
 */
void BVDC_P_CalcuPixelAspectRatio_isrsafe(
    BFMT_AspectRatio                 eFullAspectRatio,     /* full asp ratio enum */
    uint32_t                         ulSampleAspectRatioX, /* width of one sampled src pixel */
    uint32_t                         ulSampleAspectRatioY, /* height of one sampled src pixel */
    uint32_t                         ulFullWidth,          /* full asp ratio width */
    uint32_t                         ulFullHeight,         /* full asp ratio height */
    const BVDC_P_ClipRect           *pAspRatCnvsClip,      /* asp rat cnvs clip */
    uintAR_t                        *pulPxlAspRatio,       /* PxlAspR_int.PxlAspR_frac */
    uint32_t                        *pulPxlAspRatio_x_y,   /* PxlAspR_x<<16 | PxlAspR_y */
    BFMT_Orientation                 eOrientation          /* orientation of the input stream  */
)
{
    uint32_t  ulAspRatCnvsWidth, ulAspRatCnvsHeight;
    uintAR_t  ulPixAspRatio = 0;
    uint16_t  uiPixAspR_x=0, uiPixAspR_y=0;
    uint32_t b=0, a=0, m=0, i=0;
    BDBG_ASSERT((NULL != pulPxlAspRatio) && (NULL != pulPxlAspRatio_x_y));
    BDBG_ASSERT((ulFullWidth != 0) && (ulFullHeight != 0));

    ulFullWidth  <<= (eOrientation == BFMT_Orientation_e3D_LeftRight);
    ulFullHeight <<= (eOrientation == BFMT_Orientation_e3D_OverUnder);
    if(NULL != pAspRatCnvsClip)
    {
        ulAspRatCnvsWidth  = ulFullWidth  - (pAspRatCnvsClip->ulLeft + pAspRatCnvsClip->ulRight);
        ulAspRatCnvsHeight = ulFullHeight - (pAspRatCnvsClip->ulTop  + pAspRatCnvsClip->ulBottom);
    }
    else
    {
        ulAspRatCnvsWidth  = ulFullWidth;
        ulAspRatCnvsHeight = ulFullHeight;
    }

    /* Set default value for unknown aspect ratio. */
    BVDC_P_SetDefaultAspRatio_isrsafe(&eFullAspectRatio, ulSampleAspectRatioX, ulSampleAspectRatioY, ulFullWidth, ulFullHeight);

    /* Pay attention to overflow, assuming ulAspRatCnvsHeight could be as big as 1080 */
    switch (eFullAspectRatio)
    {
    case BFMT_AspectRatio_eSquarePxl:
        BDBG_MSG(("BFMT_AspectRatio_eSquarePxl"));
        uiPixAspR_x = 1;
        uiPixAspR_y = 1;
        break;
    case BFMT_AspectRatio_e4_3:
        BDBG_MSG(("BFMT_AspectRatio_e4_3"));
        uiPixAspR_x = ulAspRatCnvsHeight * 4;
        uiPixAspR_y = ulAspRatCnvsWidth * 3;
        break;
    case BFMT_AspectRatio_e16_9:
        BDBG_MSG(("BFMT_AspectRatio_e16_9"));
        uiPixAspR_x = ulAspRatCnvsHeight * 16;
        uiPixAspR_y = ulAspRatCnvsWidth * 9;
        break;
    case BFMT_AspectRatio_e221_1:
        BDBG_MSG(("BFMT_AspectRatio_e221_1"));
        uiPixAspR_x = (ulAspRatCnvsHeight * 221) >> 3;
        uiPixAspR_y = (ulAspRatCnvsWidth  * 100) >> 3;
        ulPixAspRatio = ((uintAR_t)ulAspRatCnvsHeight << BVDC_P_ASPR_FRAC_BITS_NUM) * 2 / ulAspRatCnvsWidth +
                        ((uintAR_t)ulAspRatCnvsHeight << BVDC_P_ASPR_FRAC_BITS_NUM) * 21 / (100 * ulAspRatCnvsWidth);
        break;
    case BFMT_AspectRatio_e15_9:
        BDBG_MSG(("BFMT_AspectRatio_e15_9"));
        uiPixAspR_x = ulAspRatCnvsHeight * 15;
        uiPixAspR_y = ulAspRatCnvsWidth * 9;
        break;
    case BFMT_AspectRatio_eSAR:
        BDBG_MSG(("BFMT_AspectRatio_eSAR: %d, %d", ulSampleAspectRatioX, ulSampleAspectRatioY));
        uiPixAspR_x = ulSampleAspectRatioX <<(eOrientation == BFMT_Orientation_e3D_LeftRight);
        uiPixAspR_y = ulSampleAspectRatioY <<(eOrientation == BFMT_Orientation_e3D_OverUnder);
        break;
    default:
        uiPixAspR_x = 1;
        uiPixAspR_y = 1;
        BDBG_ERR(("Bad asp ratio enum %d", eFullAspectRatio));
        break;
    }

    if(uiPixAspR_y == uiPixAspR_x)
    {
        uiPixAspR_y = uiPixAspR_x = 1;
    }
    /* Euclidean gcd algorithm */
    else
    {
        a = uiPixAspR_y > uiPixAspR_x ? uiPixAspR_y:uiPixAspR_x;
        b = uiPixAspR_y > uiPixAspR_x ? uiPixAspR_x:uiPixAspR_y;

        while (b  && (i<10)) { m = a % b; a = b; b = m; i++;}

        if (i<10) {
            uiPixAspR_y/=a;
            uiPixAspR_x/=a;
        }
    }

    if (BFMT_AspectRatio_e221_1 != eFullAspectRatio)
    {
        /* use multiplication to avoid endian dependent error */
        ulPixAspRatio = ((uintAR_t)uiPixAspR_x << BVDC_P_ASPR_FRAC_BITS_NUM) / (uiPixAspR_y);
    }

    *pulPxlAspRatio = ulPixAspRatio;
    *pulPxlAspRatio_x_y = ((uint32_t)uiPixAspR_x <<16) | uiPixAspR_y;
}

#define BVDC_P_ROUND_CUT_OVER_ORIG        0
#if (BVDC_P_ROUND_CUT_OVER_ORIG == 1)
/***************************************************************************
 * {private}
 *
 * Utility function called by BVDC_P_AspectRatioCorrection_isrsafe to
 * round the (ulCutLen / ulFullLen) to i/16, iff the rounded ulCutLen really
 * makes the ratio exactly == i/16, and Src / Dst offset is inside
 * ulSclFctRndToler %
 */
static uint32_t BVDC_P_CutRounding_isrsafe(
      uint32_t                         ulSclFctRndToler,
      uint32_t                         ulCutLen,
      uint32_t                         ulFullLen
)
{
    uint32_t ul16TmpLen, ul16CutRatio;
    uint32_t ulOff, ulTmpLen, ulNewLen;

    ulNewLen = ulCutLen;

    if((0 != ulSclFctRndToler) && (ulCutLen < ulFullLen) && (0 != ulCutLen))
    {
        ul16CutRatio = BVDC_P_DIV_ROUND_NEAR(ulCutLen * 16, ulFullLen);
        ul16TmpLen = ul16CutRatio * ulFullLen;
        if(0 == (ul16TmpLen & 15)) /* fully devided by 16 */
        {
            ulTmpLen = ul16TmpLen / 16;
            ulOff = (ulTmpLen < ulCutLen)? (ulCutLen - ulTmpLen): (ulTmpLen - ulCutLen);
            if(200 * ulOff < ulCutLen * ulSclFctRndToler)
            {
                ulNewLen = ulTmpLen;
            }
        }
    }

    return BVDC_P_MIN(ulNewLen, ulFullLen);
}
#else
#define BVDC_P_CutRounding_isrsafe(t, c, f)  BVDC_P_MIN(c, f)
#endif


/***************************************************************************
 * {private}
 *
 * This function atomatically cut the window's content size (width or height)
 * or scl-out-rect size (width or height), to enforce correct aspect ratio.
 * It outputs the new content and scl-out-rect size
 *
 * If AspectRatioMode is Zoom, it will cut src rect to make it have the same
 * aspect ratio as the scaler-out-rect. When AspectRatioMode is Box, it will
 * cut the scaler-out-rect to make the src aspect ration unchanged after
 * scaling.
 *
 * When this func is called, it is called after box auto cut, pan scan, and
 * user set src/dst clipping. It assumes that aspect-ratio mode is set to
 * either box or zoom, and that it is not in non-linear scale mode.
 *
 * Theoretically, if the src is mpeg, hddvi, or if letter box auto back cut
 * is enabled, it should be called at every vsync when RUL is built.
 * Otherwise, it should be called only once at the first vsync after
 * ApplyChanges.
 *
 * Optimize: aspect ratio correctio and scale factor rounding needs re-do
 * only if values of SrcCut changed after box cut, pan scan, and user clip,
 * or right after ApplyChanges has been called.
 */
void BVDC_P_AspectRatioCorrection_isrsafe(
    uintAR_t                         ulSrcPxlAspRatio,    /* U4.16 value */
    uintAR_t                         ulDspPxlAspRatio,    /* U4.16 value */
    BVDC_AspectRatioMode             eAspectRatioMode,    /* aspect ratio correction mode */
    uint32_t                         ulHrzSclFctRndToler, /* horizontal scale factor rounding tolerance */
    uint32_t                         ulVrtSclFctRndToler, /* vertical scale factor rounding tolerance */
    BVDC_P_AutoAdj                  *pAutoAdj             /* structure for adjusted width and height */
)
{
    uintAR_t  ulCntAspR, ulDspAspR;
    uint32_t  ulTmpWidth, ulTmpHeight;
    uint32_t  ulSrcWidth;
    uint32_t  ulSrcHeight;
    uint32_t  ulDspWidth;
    uint32_t  ulDspHeight;

    BDBG_ENTER(BVDC_P_AspectRatioCorrection_isrsafe);
    BDBG_ASSERT((BVDC_AspectRatioMode_eUseAllDestination == eAspectRatioMode) ||
                (BVDC_AspectRatioMode_eUseAllSource == eAspectRatioMode));

    ulSrcWidth  = pAutoAdj->ulCntWidth;
    ulSrcHeight = pAutoAdj->ulCntHeight;
    ulDspWidth  = pAutoAdj->ulOutWidth;
    ulDspHeight = pAutoAdj->ulOutHeight;

    if((0 != ulDspWidth)  &&
       (0 != ulDspHeight) &&
       (0 != ulSrcWidth)  &&
       (0 != ulSrcHeight))
    {
        /* Lets use notation as the following
         * cw:  content width
         * ch:  content height
         * spar: src pixel aspect ratio value in U4.16
         * dw:  scaler output width
         * dh:  scaler output height
         * dpar: display pixel aspect ratio value in U4.16
         * sar: aspect ratio of the content rect in U10.11
         * dar: aspect ratio of the scaler-output rect in U10.11
         *
         * Then
         *        cw
        *  sar = -- * spar >> (BVDC_P_ASPR_FRAC_BITS_NUM - BVDC_P_SUB_ASPR_FRAC_BITS_NUM)
         *        ch
         *
         *        dw
        *  dar = -- * dpar >> (BVDC_P_ASPR_FRAC_BITS_NUM - BVDC_P_SUB_ASPR_FRAC_BITS_NUM)
         *        dh
         * or
         *        dw
         *  dar = --    if hWindow->hCompositor->stCurInfo.eAspectRatio IS eSquarePxl
         *        dh
         *
         * If AspectRatioMode is Zoom, our goal is to cut cw or ch to make sar to be the same as dar,
         * When AspectRatioMode is Box, our goal is to cut dw or dh to make dar to be the same as sar.
         */

        /* calculate the aspect ratio of the scalerOutput, in U10.11 fix pt */
        ulDspAspR = (BVDC_P_DIV_ROUND_NEAR(ulDspWidth * ulDspPxlAspRatio, ulDspHeight) >>
                     (BVDC_P_ASPR_FRAC_BITS_NUM - BVDC_P_SUB_ASPR_FRAC_BITS_NUM));
        ulDspAspR = BVDC_P_MIN(ulDspAspR, (((uintAR_t)1<<BVDC_P_SUB_ASPR_ALL_BITS_NUM) - 1));

        /* calculate the aspect ratio of the content rect, in U10.11 fix pt */
        ulCntAspR = (BVDC_P_DIV_ROUND_NEAR(ulSrcWidth * ulSrcPxlAspRatio, ulSrcHeight) >>
                     (BVDC_P_ASPR_FRAC_BITS_NUM - BVDC_P_SUB_ASPR_FRAC_BITS_NUM));
        ulCntAspR = BVDC_P_MIN(ulCntAspR, (((uintAR_t)1<<BVDC_P_SUB_ASPR_ALL_BITS_NUM) - 1));

        if(BVDC_AspectRatioMode_eUseAllDestination == eAspectRatioMode)
        {
            if( ulCntAspR > ulDspAspR )
            {
                /* needs cut cw: new cw = cw * dar / sar */
                ulTmpWidth = BVDC_P_DIV_ROUND_NEAR(ulSrcWidth * ulDspAspR, ulCntAspR);
                ulTmpWidth = BVDC_P_MIN(ulTmpWidth, ulSrcWidth);
                /* round cut/orig ratio to i/16 if it make sense and
                 * does not break the scale factor rounding tolerance */
                pAutoAdj->ulCntWidth = BVDC_P_CutRounding_isrsafe(
                    ulHrzSclFctRndToler, ulTmpWidth, ulSrcWidth);
                pAutoAdj->ulAdjFlags |= (BVDC_P_ADJ_CNT_WIDTH | BVDC_P_ADJ_HRZ_SRC_STEP);
                BDBG_MODULE_MSG(BVDC_ASP_RAT,("AspR correct: cut content W: new width %d", pAutoAdj->ulCntWidth));
#ifndef BVDC_UINT32_ONLY
                BDBG_MODULE_MSG(BVDC_ASP_RAT,("New equation cnt side " BDBG_UINT64_FMT ", disp side " BDBG_UINT64_FMT,
                    BDBG_UINT64_ARG((pAutoAdj->ulCntWidth * ulCntAspR) / ulSrcWidth), BDBG_UINT64_ARG(ulDspAspR)));
#else
                BDBG_MODULE_MSG(BVDC_ASP_RAT,("New equation cnt side 0x%Lx, disp side 0x%Lx",
                    (pAutoAdj->ulCntWidth * ulCntAspR) / ulSrcWidth, ulDspAspR));
#endif
            }
            else if( ulCntAspR < ulDspAspR )
            {
                /* needs cut ch: new ch = ch * sar / dar */
                ulTmpHeight = BVDC_P_DIV_ROUND_NEAR(ulSrcHeight * ulCntAspR, ulDspAspR);
                /* round cut/orig ratio to i/16 if it make sense and
                 * does not break the scale factor rounding tolerance */
                pAutoAdj->ulCntHeight = BVDC_P_CutRounding_isrsafe(
                    ulVrtSclFctRndToler, ulTmpHeight, ulSrcHeight);
                pAutoAdj->ulAdjFlags |= (BVDC_P_ADJ_CNT_HEIGHT | BVDC_P_ADJ_VRT_SRC_STEP);
                BDBG_MODULE_MSG(BVDC_ASP_RAT,("AspR correct: cut content H: new height %d", pAutoAdj->ulCntHeight));
#ifndef BVDC_UINT32_ONLY
                BDBG_MODULE_MSG(BVDC_ASP_RAT,("New equation cnt side " BDBG_UINT64_FMT ", disp side " BDBG_UINT64_FMT,
                    BDBG_UINT64_ARG((ulSrcHeight * ulCntAspR) / pAutoAdj->ulCntHeight), BDBG_UINT64_ARG(ulDspAspR)));
#else
                BDBG_MODULE_MSG(BVDC_ASP_RAT,("New equation cnt side 0x%Lx, disp side 0x%Lx",
                    (ulSrcHeight * ulCntAspR) / pAutoAdj->ulCntHeight, ulDspAspR));
#endif
            }
            /* else: no cut is needed */
        }

        else if(BVDC_AspectRatioMode_eUseAllSource == eAspectRatioMode)
        {
            if( ulCntAspR > ulDspAspR )
            {
                /* needs cut dh: new dh = dh * dar / sar */
                ulTmpHeight = BVDC_P_DIV_ROUND_NEAR(ulDspHeight * ulDspAspR, ulCntAspR);
                /* round cut/orig ratio to i/16 if it make sense and
                 * does not break the scale factor rounding tolerance */
                pAutoAdj->ulOutHeight = BVDC_P_CutRounding_isrsafe(
                    ulVrtSclFctRndToler, ulTmpHeight, ulDspHeight);
                pAutoAdj->ulAdjFlags |= (BVDC_P_ADJ_OUT_HEIGHT | BVDC_P_ADJ_VRT_SRC_STEP);
                BDBG_MODULE_MSG(BVDC_ASP_RAT,("AspR correct: cut out H: new height %d", pAutoAdj->ulOutHeight));
#ifndef BVDC_UINT32_ONLY
                BDBG_MODULE_MSG(BVDC_ASP_RAT,("New equation cnt side " BDBG_UINT64_FMT ", disp side " BDBG_UINT64_FMT,
                    BDBG_UINT64_ARG(ulCntAspR), BDBG_UINT64_ARG((ulDspHeight * ulDspAspR) / pAutoAdj->ulOutHeight)));
#else
                BDBG_MODULE_MSG(BVDC_ASP_RAT,("New equation cnt side 0x%Lx, disp side 0x%Lx",
                    ulCntAspR, (ulDspHeight * ulDspAspR) / pAutoAdj->ulOutHeight));
#endif
            }
            else if( ulCntAspR < ulDspAspR )
            {
                /* needs cut dw: new dw = dw * sar / dar */
                ulTmpWidth = BVDC_P_DIV_ROUND_NEAR(ulDspWidth * ulCntAspR, ulDspAspR);
                /* round cut/orig ratio to i/16 if it make sense and
                 * does not break the scale factor rounding tolerance */
                pAutoAdj->ulOutWidth = BVDC_P_CutRounding_isrsafe(
                    ulHrzSclFctRndToler, ulTmpWidth, ulDspWidth);
                pAutoAdj->ulAdjFlags |= (BVDC_P_ADJ_OUT_WIDTH | BVDC_P_ADJ_HRZ_SRC_STEP);
                BDBG_MODULE_MSG(BVDC_ASP_RAT,("AspR correct: cut out W: new width %d", pAutoAdj->ulOutWidth));
#ifndef BVDC_UINT32_ONLY
                BDBG_MODULE_MSG(BVDC_ASP_RAT,("New equation cnt side " BDBG_UINT64_FMT ", disp side " BDBG_UINT64_FMT,
                    BDBG_UINT64_ARG(ulCntAspR), BDBG_UINT64_ARG((pAutoAdj->ulOutWidth * ulDspAspR) / ulDspWidth)));
#else
                BDBG_MODULE_MSG(BVDC_ASP_RAT,("New equation cnt side 0x%Lx, disp side 0x%Lx",
                    ulCntAspR, (pAutoAdj->ulOutWidth * ulDspAspR) / ulDspWidth));
#endif
            }
            /* else: no cut is needed */
        }
        /* else: should not call this func */
    }
    else
    {
        BDBG_WRN(("Zero Rect Edge, Non-Linear Horizontal Scaling, or AspR unknown."));
    }

#if (BVDC_P_ROUND_CUT_OVER_ORIG != 1)
    BSTD_UNUSED(ulHrzSclFctRndToler);
    BSTD_UNUSED(ulVrtSclFctRndToler);
#endif

    BDBG_LEAVE(BVDC_P_AspectRatioCorrection_isrsafe);
    return;
}
/* End of file. */
