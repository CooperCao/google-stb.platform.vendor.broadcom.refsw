/******************************************************************************
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
 ******************************************************************************/
#ifndef BVDC_ADJRECT_PRIV_H__
#define BVDC_ADJRECT_PRIV_H__

#include "bvdc.h"
#include "bvdc_common_priv.h"
#include "bvdc_priv.h"

#ifdef __cplusplus
extern "C" {
#endif

BDBG_OBJECT_ID_DECLARE(BVDC_ADJRECT);

#ifndef BVDC_UINT32_ONLY

#define BVDC_P_SUB_ASPR_INT_BITS_NUM       (12)
#define BVDC_P_SUB_ASPR_FRAC_BITS_NUM      (40)
#define BVDC_P_SUB_ASPR_ALL_BITS_NUM       (BVDC_P_SUB_ASPR_INT_BITS_NUM+BVDC_P_SUB_ASPR_FRAC_BITS_NUM)

#else

/* for NOT well bounded value such as sub-rect aspect ratio value */
#define BVDC_P_SUB_ASPR_INT_BITS_NUM       10
#define BVDC_P_SUB_ASPR_FRAC_BITS_NUM      11
#define BVDC_P_SUB_ASPR_ALL_BITS_NUM       (BVDC_P_SUB_ASPR_INT_BITS_NUM+BVDC_P_SUB_ASPR_FRAC_BITS_NUM)

#endif

#define BVDC_P_ADJ_CNT_CUT            (0x1)
#define BVDC_P_ADJ_OUT_CUT            (0x2)
#define BVDC_P_ADJ_SRC_STEP           (0x4)
#define BVDC_P_ADJ_FLAG_HRZ_SHIFT     (0)
#define BVDC_P_ADJ_FLAG_VRT_SHIFT     (4)

#define BVDC_P_ADJ_CNT_WIDTH          (BVDC_P_ADJ_CNT_CUT  << BVDC_P_ADJ_FLAG_HRZ_SHIFT)
#define BVDC_P_ADJ_OUT_WIDTH          (BVDC_P_ADJ_OUT_CUT  << BVDC_P_ADJ_FLAG_HRZ_SHIFT)
#define BVDC_P_ADJ_HRZ_SRC_STEP       (BVDC_P_ADJ_SRC_STEP << BVDC_P_ADJ_FLAG_HRZ_SHIFT)
#define BVDC_P_ADJ_CNT_HEIGHT         (BVDC_P_ADJ_CNT_CUT  << BVDC_P_ADJ_FLAG_VRT_SHIFT)
#define BVDC_P_ADJ_OUT_HEIGHT         (BVDC_P_ADJ_OUT_CUT  << BVDC_P_ADJ_FLAG_VRT_SHIFT)
#define BVDC_P_ADJ_VRT_SRC_STEP       (BVDC_P_ADJ_SRC_STEP << BVDC_P_ADJ_FLAG_VRT_SHIFT)


/* temporary struct for auto adjust status */
typedef struct
{
    uint32_t     ulAdjFlags;  /* what is adjusted */
    uint32_t     ulCntWidth;  /* new cntWidth */
    uint32_t     ulOutWidth;  /* new SclOutWidth */
    uint32_t     ulCntHeight; /* new cntHeight */
    uint32_t     ulOutHeight; /* new SclOutHeight */
} BVDC_P_AutoAdj;


void BVDC_P_SetDefaultAspRatio_isrsafe(
    BFMT_AspectRatio                *peFullAspectRatio,
    uint32_t                         ulSampleAspectRatioX,
    uint32_t                         ulSampleAspectRatioY,
    uint32_t                         ulFullWidth,
    uint32_t                         ulFullHeight);

void BVDC_P_CalcuPixelAspectRatio_isrsafe(
    BFMT_AspectRatio                 eFullAspectRatio,     /* full asp ratio enum */
    uint32_t                         ulSampleAspectRatioX, /* width of one sampled src pixel */
    uint32_t                         ulSampleAspectRatioY, /* height of one sampled src pixel */
    uint32_t                         ulFullWidth,          /* full asp ratio width */
    uint32_t                         ulFullHeight,         /* full asp ratio height */
    const BVDC_P_ClipRect*           pAspRatCnvsClip,      /* asp rat cnvs clip */
    uintAR_t *                       pulPxlAspRatio,       /* PxlAspR_int.PxlAspR_frac */
    uint32_t *                       pulPxlAspRatio_x_y,   /* PxlAspR_x<<16 | PxlAspR_y */
    BFMT_Orientation                 eOrientation );       /* orientation of the input stream  */

void BVDC_P_AspectRatioCorrection_isrsafe(
    uintAR_t                         ulSrcPxlAspRatio,    /* U4.16 value */
    uintAR_t                         ulDspPxlAspRatio,    /* U4.16 value */
    BVDC_AspectRatioMode             eAspectRatioMode,    /* aspect ratio correction mode */
    uint32_t                         ulHrzSclFctRndToler, /* horizontal scale factor rounding tolerance */
    uint32_t                         ulVrtSclFctRndToler, /* vertical scale factor rounding tolerance */
    BVDC_P_AutoAdj                  *pAutoAdj );          /* auto adj */

#ifdef __cplusplus
}
#endif

#endif /* #ifndef BVDC_ADJRECT_PRIV_H__ */
/* End of file. */
