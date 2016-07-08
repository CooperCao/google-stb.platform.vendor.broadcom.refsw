/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
#ifndef BVDC_CSC_PRIV_H__
#define BVDC_CSC_PRIV_H__

#include "bmth_fix.h"
#include "bmth_fix_matrix.h"
#include "bvdc.h"
#include "bvdc_common_priv.h"
#include "bvdc_gfxfeeder_priv.h"

#ifdef __cplusplus
extern "C" {
#endif

#define BVDC_P_FIX_MAX_BITS                 (31)

/* Default FIX point shift */
#define BVDC_P_FIX_POINT_SHIFT              (16)

/* CSC Precision notation SI.F or S4.11, etc */
#if (BVDC_P_SUPPORT_GFD_VER >= BVDC_P_SUPPORT_GFD_VER_3)
#define BVDC_P_CSC_GFD_CX_I_BITS             (2)
#define BVDC_P_CSC_GFD_CX_F_BITS            (13)
#define BVDC_P_CSC_GFD_CO_I_BITS             (9)
#define BVDC_P_CSC_GFD_CO_F_BITS             (6)
#define BVDC_P_CSC_GFD_CO_VID_BITS           (8)
#else
#define BVDC_P_CSC_GFD_CX_I_BITS             (2)
#define BVDC_P_CSC_GFD_CX_F_BITS            (13)
#define BVDC_P_CSC_GFD_CO_I_BITS            (11)
#define BVDC_P_CSC_GFD_CO_F_BITS             (4)
#define BVDC_P_CSC_GFD_CO_VID_BITS           (8)
#endif

#define BVDC_P_CSC_GFD_MA_CX_I_BITS          (2)
#define BVDC_P_CSC_GFD_MA_CX_F_BITS         (22)
#define BVDC_P_CSC_GFD_MA_CO_I_BITS          (9)
#define BVDC_P_CSC_GFD_MA_CO_F_BITS         (15)
#define BVDC_P_CSC_GFD_MA_CO_VID_BITS        (8)

#define BVDC_P_CSC_CMP_CX_I_BITS             (2)
#define BVDC_P_CSC_CMP_CX_F_BITS            (13)
#define BVDC_P_CSC_CMP_CO_I_BITS             (9)
#define BVDC_P_CSC_CMP_CO_F_BITS             (6)
#define BVDC_P_CSC_CMP_CO_VID_BITS           (8)

#define BVDC_P_CSC_CMP_AB_CX_I_BITS          (2)
#define BVDC_P_CSC_CMP_AB_CX_F_BITS         (22)
#define BVDC_P_CSC_CMP_AB_CO_I_BITS          (9)
#define BVDC_P_CSC_CMP_AB_CO_F_BITS         (15)
#define BVDC_P_CSC_CMP_AB_CO_VID_BITS        (8)

#define BVDC_P_CSC_CMP_LR_XY_I_BITS          (1)
#define BVDC_P_CSC_CMP_LR_XY_F_BITS         (15)
#define BVDC_P_CSC_CMP_LR_SLP_M_I_BITS       (0)
#define BVDC_P_CSC_CMP_LR_SLP_M_F_BITS      (15)
#define BVDC_P_CSC_CMP_LR_SLP_E_I_BITS       (4)
#define BVDC_P_CSC_CMP_LR_SLP_E_F_BITS       (0)

#define BVDC_P_CSC_CMP_LR_X_SHIFT           BCHP_HDR_CMP_0_V0_R0_NL_LR_XY_0i_LRA_X_SHIFT
#define BVDC_P_CSC_CMP_LR_Y_SHIFT           BCHP_HDR_CMP_0_V0_R0_NL_LR_XY_0i_LRA_Y_SHIFT
#define BVDC_P_CSC_CMP_LR_SLP_M_SHIFT       BCHP_HDR_CMP_0_V0_R0_NL_LR_SLOPEi_SLOPE_M_SHIFT
#define BVDC_P_CSC_CMP_LR_SLP_E_SHIFT       BCHP_HDR_CMP_0_V0_R0_NL_LR_SLOPEi_SLOPE_E_SHIFT

#define BVDC_P_CSC_VDEC_CX_I_BITS            (2)
#define BVDC_P_CSC_VDEC_CX_F_BITS           (13)
#define BVDC_P_CSC_VDEC_CO_I_BITS            (9)
#define BVDC_P_CSC_VDEC_CO_F_BITS            (6)
#define BVDC_P_CSC_VDEC_CO_VID_BITS          (8)

#define BVDC_P_CSC_HDDVI_CX_I_BITS           (2)  /* HD-DVI, MFD */
#define BVDC_P_CSC_HDDVI_CX_F_BITS          (13)
#define BVDC_P_CSC_HDDVI_CO_I_BITS           (9)
#define BVDC_P_CSC_HDDVI_CO_F_BITS           (6)
#define BVDC_P_CSC_HDDVI_CO_VID_BITS         (8)

#define BVDC_P_CSC_VEC_CX_I_BITS             (2)
#define BVDC_P_CSC_VEC_CX_F_BITS            (13)
#define BVDC_P_CSC_VEC_CO_I_BITS             (9)
#define BVDC_P_CSC_VEC_CO_F_BITS             (6)
#define BVDC_P_CSC_VEC_CO_VID_BITS           (8)

#define BVDC_P_CSC_656_CX_I_BITS             (4)
#define BVDC_P_CSC_656_CX_F_BITS            (11)
#define BVDC_P_CSC_656_CO_I_BITS            (11)
#define BVDC_P_CSC_656_CO_F_BITS             (4)
#define BVDC_P_CSC_656_CO_VID_BITS           (8)

#define BVDC_P_CSC_DVO_CX_I_BITS             (2)
#define BVDC_P_CSC_DVO_CX_F_BITS            (13)
#define BVDC_P_CSC_DVO_CO_I_BITS             (9)
#define BVDC_P_CSC_DVO_CO_F_BITS             (6)
#define BVDC_P_CSC_DVO_CO_VID_BITS           (8)

/* video bit format of static tables for each module */
#define BVDC_P_CSC_CMP_CO_VID_TBL_BITS       (8)
#define BVDC_P_CSC_CMP_AB_CO_VID_TBL_BITS    (8)
#define BVDC_P_CSC_VDEC_CO_VID_TBL_BITS      (8)
#define BVDC_P_CSC_HDDVI_CO_VID_TBL_BITS     (8)
#define BVDC_P_CSC_GFD_CO_VID_TBL_BITS       (8)
#define BVDC_P_CSC_GFD_MA_CO_VID_TBL_BITS    (8)
#define BVDC_P_CSC_656_CO_VID_TBL_BITS       (8)
#define BVDC_P_CSC_DVO_CO_VID_TBL_BITS       (8)
#define BVDC_P_CSC_VEC_CO_VID_TBL_BITS       (8)

#define BVDC_P_CSC_VEC_RANGE_VID_TBL_BITS   (12)

/* vec csc range bits */
#define BVDC_P_CSC_VEC_RANGE_BITS           (12)

/* ranges are specified in signed 12 bit dither values */
#define BVDC_P_CSC_VEC_RANGE_SHIFT (BVDC_P_CSC_VEC_RANGE_VID_TBL_BITS - BVDC_P_CSC_VEC_RANGE_BITS)
#define BVDC_P_CSC_VEC_RANGE_MASK  (0xFFFFFFFF >> (BVDC_P_FIX_MAX_BITS - BVDC_P_CSC_VEC_RANGE_VID_TBL_BITS))


/* Color Space Conversion Matrix */
#define BVDC_P_MAKE_CSC_CX(cx, i_bits, f_bits)                                           \
    (uint16_t)BMTH_FIX_SIGNED_FTOFIX(cx, i_bits, f_bits)

#define BVDC_P_MAKE_CSC_CO(co, i_bits, f_bits, co_vid_bits, co_vid_tbl_bits)             \
    (uint16_t)BMTH_FIX_SIGNED_FTOFIX((co_vid_tbl_bits > co_vid_bits) ?                   \
                                     ((co) / (1 << (co_vid_tbl_bits - co_vid_bits))) :   \
                                     ((co) * (1 << (co_vid_bits - co_vid_tbl_bits))),    \
                                     i_bits, f_bits)

#define BVDC_P_FR_USR_MATRIX(cx, usr_shift, i_bits, f_bits)            \
    (uint16_t)BMTH_FIX_SIGNED_CONVERT(cx, BVDC_P_FIX_MAX_BITS - usr_shift, usr_shift, i_bits, f_bits)

#define BVDC_P_TO_USR_MATRIX(cx, i_bits, f_bits, usr_shift)            \
    BMTH_FIX_SIGNED_CONVERT(cx, i_bits, f_bits, BVDC_P_FIX_MAX_BITS - usr_shift, usr_shift)

/* An entry */
#define BVDC_P_MAKE_CSC(Y0, Y1, Y2, YAlpha, YOffset,                \
                        Cb0, Cb1, Cb2, CbAlpha, CbOffset,           \
                        Cr0, Cr1, Cr2, CrAlpha, CrOffset,           \
                        cx_i_bits, cx_f_bits, co_i_bits, co_f_bits, \
                        co_vid_bits, co_vid_tbl_bits)               \
{                                                                   \
    BVDC_P_MAKE_CSC_CX(Y0,       cx_i_bits, cx_f_bits),             \
    BVDC_P_MAKE_CSC_CX(Y1,       cx_i_bits, cx_f_bits),             \
    BVDC_P_MAKE_CSC_CX(Y2,       cx_i_bits, cx_f_bits),             \
    BVDC_P_MAKE_CSC_CX(YAlpha,   cx_i_bits, cx_f_bits),             \
    BVDC_P_MAKE_CSC_CO(YOffset,  co_i_bits, co_f_bits,              \
                       co_vid_bits, co_vid_tbl_bits),               \
                                                                    \
    BVDC_P_MAKE_CSC_CX(Cb0,      cx_i_bits, cx_f_bits),             \
    BVDC_P_MAKE_CSC_CX(Cb1,      cx_i_bits, cx_f_bits),             \
    BVDC_P_MAKE_CSC_CX(Cb2,      cx_i_bits, cx_f_bits),             \
    BVDC_P_MAKE_CSC_CX(CbAlpha,  cx_i_bits, cx_f_bits),             \
    BVDC_P_MAKE_CSC_CO(CbOffset, co_i_bits, co_f_bits,              \
                       co_vid_bits, co_vid_tbl_bits),               \
                                                                    \
    BVDC_P_MAKE_CSC_CX(Cr0,      cx_i_bits, cx_f_bits),             \
    BVDC_P_MAKE_CSC_CX(Cr1,      cx_i_bits, cx_f_bits),             \
    BVDC_P_MAKE_CSC_CX(Cr2,      cx_i_bits, cx_f_bits),             \
    BVDC_P_MAKE_CSC_CX(CrAlpha,  cx_i_bits, cx_f_bits),             \
    BVDC_P_MAKE_CSC_CO(CrOffset, co_i_bits, co_f_bits,              \
                       co_vid_bits, co_vid_tbl_bits),               \
                                                                    \
    cx_i_bits,                                                      \
    cx_f_bits,                                                      \
    co_i_bits,                                                      \
    co_f_bits,                                                      \
    co_vid_bits                                                     \
}

/* CMP's Csc */
#define BVDC_P_MAKE_CMP_CSC(Y0, Y1, Y2, YOffset,                    \
                            Cb0, Cb1, Cb2, CbOffset,                \
                            Cr0, Cr1, Cr2, CrOffset)                \
    BVDC_P_MAKE_CSC(                                                \
    Y0, Y1, Y2, 0, YOffset,                                         \
    Cb0, Cb1, Cb2, 0, CbOffset,                                     \
    Cr0, Cr1, Cr2, 0, CrOffset,                                     \
    BVDC_P_CSC_CMP_CX_I_BITS,                                       \
    BVDC_P_CSC_CMP_CX_F_BITS,                                       \
    BVDC_P_CSC_CMP_CO_I_BITS,                                       \
    BVDC_P_CSC_CMP_CO_F_BITS,                                       \
    BVDC_P_CSC_CMP_CO_VID_BITS,                                     \
    BVDC_P_CSC_CMP_CO_VID_TBL_BITS)

/* VDEC's Csc */
#define BVDC_P_MAKE_VDEC_CSC(Y0, Y1, Y2, YOffset,                   \
                             Cb0, Cb1, Cb2, CbOffset,               \
                             Cr0, Cr1, Cr2, CrOffset)               \
    BVDC_P_MAKE_CSC(                                                \
    Y0, Y1, Y2, 0, YOffset,                                         \
    Cb0, Cb1, Cb2, 0, CbOffset,                                     \
    Cr0, Cr1, Cr2, 0, CrOffset,                                     \
    BVDC_P_CSC_VDEC_CX_I_BITS,                                      \
    BVDC_P_CSC_VDEC_CX_F_BITS,                                      \
    BVDC_P_CSC_VDEC_CO_I_BITS,                                      \
    BVDC_P_CSC_VDEC_CO_F_BITS,                                      \
    BVDC_P_CSC_VDEC_CO_VID_BITS,                                    \
    BVDC_P_CSC_VDEC_CO_VID_TBL_BITS)

/* HD-DVI's Csc */
#define BVDC_P_MAKE_HDDVI_CSC(Y0, Y1, Y2, YOffset,                  \
                              Cb0, Cb1, Cb2, CbOffset,              \
                              Cr0, Cr1, Cr2, CrOffset)              \
    BVDC_P_MAKE_CSC(                                                \
    Y0, Y1, Y2, 0, YOffset,                                         \
    Cb0, Cb1, Cb2, 0, CbOffset,                                     \
    Cr0, Cr1, Cr2, 0, CrOffset,                                     \
    BVDC_P_CSC_HDDVI_CX_I_BITS,                                     \
    BVDC_P_CSC_HDDVI_CX_F_BITS,                                     \
    BVDC_P_CSC_HDDVI_CO_I_BITS,                                     \
    BVDC_P_CSC_HDDVI_CO_F_BITS,                                     \
    BVDC_P_CSC_HDDVI_CO_VID_BITS,                                   \
    BVDC_P_CSC_HDDVI_CO_VID_TBL_BITS)

/* GFD's Csc */
#define BVDC_P_MAKE_GFD_CSC(Y0, Y1, Y2, YAlpha, YOffset,                        \
                            Cb0, Cb1, Cb2, CbAlpha, CbOffset,                   \
                            Cr0, Cr1, Cr2, CrAlpha, CrOffset)                   \
    BVDC_P_MAKE_CSC(                                                            \
    Y0, Y1, Y2, (YAlpha / (1 << BVDC_P_CSC_GFD_CO_VID_BITS)), YOffset,          \
    Cb0, Cb1, Cb2, (CbAlpha / (1 << BVDC_P_CSC_GFD_CO_VID_BITS)), CbOffset,     \
    Cr0, Cr1, Cr2, (CrAlpha / (1 << BVDC_P_CSC_GFD_CO_VID_BITS)), CrOffset,     \
    BVDC_P_CSC_GFD_CX_I_BITS,                                                   \
    BVDC_P_CSC_GFD_CX_F_BITS,                                                   \
    BVDC_P_CSC_GFD_CO_I_BITS,                                                   \
    BVDC_P_CSC_GFD_CO_F_BITS,                                                   \
    BVDC_P_CSC_GFD_CO_VID_BITS,                                                 \
    BVDC_P_CSC_GFD_CO_VID_TBL_BITS)

/* VEC's Csc */
/* converts inputs from YCbCr to CbYCr format */
#define BVDC_P_MAKE_VEC_CSC(Y0, Y1, Y2, YOffset,                    \
                            Cb0, Cb1, Cb2, CbOffset,                \
                            Cr0, Cr1, Cr2, CrOffset)                \
    BVDC_P_MAKE_CSC(                                                \
    Y1, Y0, Y2, 0, YOffset,                                         \
    Cb1, Cb0, Cb2, 0, CbOffset,                                     \
    Cr1, Cr0, Cr2, 0, CrOffset,                                     \
    BVDC_P_CSC_VEC_CX_I_BITS,                                       \
    BVDC_P_CSC_VEC_CX_F_BITS,                                       \
    BVDC_P_CSC_VEC_CO_I_BITS,                                       \
    BVDC_P_CSC_VEC_CO_F_BITS,                                       \
    BVDC_P_CSC_VEC_CO_VID_BITS,                                     \
    BVDC_P_CSC_VEC_CO_VID_TBL_BITS)

/* VEC's Csc for RGB */
/* converts YCbCr->RGB matrix to CbYCr->GBR format */
#define BVDC_P_MAKE_VEC_RGB_CSC(R0, R1, R2, ROffset,                \
                                G0, G1, G2, GOffset,                \
                                B0, B1, B2, BOffset)                \
    BVDC_P_MAKE_VEC_CSC(                                            \
    G0, G1, G2, GOffset,                                            \
    B0, B1, B2, BOffset,                                            \
    R0, R1, R2, ROffset)                                            \

/* VEC's Csc for YIQ */
/* converts YCbCr->YIQ matrix to CbYCr->YQI format */
#define BVDC_P_MAKE_VEC_YIQ_CSC(Y0, Y1, Y2, YOffset,                \
                                I0, I1, I2, IOffset,                \
                                Q0, Q1, Q2, QOffset)                \
    BVDC_P_MAKE_VEC_CSC(                                            \
    Y0, Y1, Y2, YOffset,                                            \
    Q0, Q1, Q2, QOffset,                                            \
    I0, I1, I2, IOffset)                                            \

/* 656's Csc */
/* converts YCbCr->YCbCr matrix to CbYCr->YCrCb format */
#define BVDC_P_MAKE_656_CSC(Y0, Y1, Y2, YOffset,                    \
                            Cb0, Cb1, Cb2, CbOffset,                \
                            Cr0, Cr1, Cr2, CrOffset)                \
    BVDC_P_MAKE_CSC(                                                \
    Y1, Y0, Y2, 0, YOffset,                                         \
    Cr1, Cr0, Cr2, 0, CrOffset,                                     \
    Cb1, Cb0, Cb2, 0, CbOffset,                                     \
    BVDC_P_CSC_656_CX_I_BITS,                                       \
    BVDC_P_CSC_656_CX_F_BITS,                                       \
    BVDC_P_CSC_656_CO_I_BITS,                                       \
    BVDC_P_CSC_656_CO_F_BITS,                                       \
    BVDC_P_CSC_656_CO_VID_BITS,                                     \
    BVDC_P_CSC_656_CO_VID_TBL_BITS)

/* DVO's Csc */
/* converts inputs from YCbCr to CbYCr format */
#define BVDC_P_MAKE_DVO_CSC(Y0, Y1, Y2, YOffset,                    \
                            Cb0, Cb1, Cb2, CbOffset,                \
                            Cr0, Cr1, Cr2, CrOffset)                \
    BVDC_P_MAKE_CSC(                                                \
    Y1, Y0, Y2, 0, YOffset,                                         \
    Cb1, Cb0, Cb2, 0, CbOffset,                                     \
    Cr1, Cr0, Cr2, 0, CrOffset,                                     \
    BVDC_P_CSC_DVO_CX_I_BITS,                                       \
    BVDC_P_CSC_DVO_CX_F_BITS,                                       \
    BVDC_P_CSC_DVO_CO_I_BITS,                                       \
    BVDC_P_CSC_DVO_CO_F_BITS,                                       \
    BVDC_P_CSC_DVO_CO_VID_BITS,                                     \
    BVDC_P_CSC_DVO_CO_VID_TBL_BITS)

/* converts YCbCr->RGB matrix to CbYCr->GBR format */
#define BVDC_P_MAKE_DVO_RGB_CSC(R0, R1, R2, ROffset,                \
                                G0, G1, G2, GOffset,                \
                                B0, B1, B2, BOffset)                \
    BVDC_P_MAKE_DVO_CSC(                                            \
    G0, G1, G2, GOffset,                                            \
    B0, B1, B2, BOffset,                                            \
    R0, R1, R2, ROffset)                                            \

/* ColorTemp calculation Csc */
#define BVDC_P_MAKE_CLRTEMP_CSC(Y0, Y1, Y2, YOffset,                \
                                Cb0, Cb1, Cb2, CbOffset,            \
                                Cr0, Cr1, Cr2, CrOffset,            \
                                cx_i_bits, cx_f_bits)               \
{                                                                   \
    {                                                               \
        BMTH_FIX_SIGNED_FTOFIX(Y0,       cx_i_bits, cx_f_bits),     \
        BMTH_FIX_SIGNED_FTOFIX(Y1,       cx_i_bits, cx_f_bits),     \
        BMTH_FIX_SIGNED_FTOFIX(Y2,       cx_i_bits, cx_f_bits),     \
        BMTH_FIX_SIGNED_FTOFIX(YOffset,  cx_i_bits, cx_f_bits)      \
    },                                                              \
    {                                                               \
        BMTH_FIX_SIGNED_FTOFIX(Cb0,      cx_i_bits, cx_f_bits),     \
        BMTH_FIX_SIGNED_FTOFIX(Cb1,      cx_i_bits, cx_f_bits),     \
        BMTH_FIX_SIGNED_FTOFIX(Cb2,      cx_i_bits, cx_f_bits),     \
        BMTH_FIX_SIGNED_FTOFIX(CbOffset, cx_i_bits, cx_f_bits),     \
    },                                                              \
    {                                                               \
        BMTH_FIX_SIGNED_FTOFIX(Cr0,      cx_i_bits, cx_f_bits),     \
        BMTH_FIX_SIGNED_FTOFIX(Cr1,      cx_i_bits, cx_f_bits),     \
        BMTH_FIX_SIGNED_FTOFIX(Cr2,      cx_i_bits, cx_f_bits),     \
        BMTH_FIX_SIGNED_FTOFIX(CrOffset, cx_i_bits, cx_f_bits),     \
    }                                                               \
}

/* ColorTemp calculation Linear Model */
#define BVDC_P_MAKE_CLRTEMP_LMODEL(R0, R1,                          \
                                   G0, G1,                          \
                                   B0, B1,                          \
                                   cx_i_bits, cx_f_bits)            \
{                                                                   \
    {                                                               \
        BMTH_FIX_SIGNED_FTOFIX(R0, cx_i_bits, cx_f_bits),           \
        BMTH_FIX_SIGNED_FTOFIX(R1, cx_i_bits, cx_f_bits)            \
    },                                                              \
    {                                                               \
        BMTH_FIX_SIGNED_FTOFIX(G0, cx_i_bits, cx_f_bits),           \
        BMTH_FIX_SIGNED_FTOFIX(G1, cx_i_bits, cx_f_bits)            \
    },                                                              \
    {                                                               \
        BMTH_FIX_SIGNED_FTOFIX(B0, cx_i_bits, cx_f_bits),           \
        BMTH_FIX_SIGNED_FTOFIX(B1, cx_i_bits, cx_f_bits)            \
    }                                                               \
}

/* macro to build BVDC_P_DisplayCscMatrix */
#define BVDC_P_MAKE_VEC_CSC_MATRIX(min, max, csc_coeff)             \
{                                                                   \
    (min & BVDC_P_CSC_VEC_RANGE_MASK) >> BVDC_P_CSC_VEC_RANGE_SHIFT,\
    (max & BVDC_P_CSC_VEC_RANGE_MASK) >> BVDC_P_CSC_VEC_RANGE_SHIFT,\
    csc_coeff                                                       \
}

/************************************
 * MACRO for nonlinear HDR conversion
 */

#define BVDC_P_MAKE_CSC_AB_CX(cx, i_bits, f_bits)                                           \
    (uint32_t)BMTH_FIX_SIGNED_FTOFIX(cx, i_bits, f_bits)

#define BVDC_P_MAKE_CSC_AB_CO(co, i_bits, f_bits, co_vid_bits, co_vid_tbl_bits)             \
    (uint32_t)BMTH_FIX_SIGNED_FTOFIX((co_vid_tbl_bits > co_vid_bits) ?                   \
                                     ((co) / (1 << (co_vid_tbl_bits - co_vid_bits))) :   \
                                     ((co) * (1 << (co_vid_bits - co_vid_tbl_bits))),    \
                                     i_bits, f_bits)

/* An entry */
#define BVDC_P_MAKE_CSC_AB(Y0, Y1, Y2, YAlpha, YOffset,                \
                           Cb0, Cb1, Cb2, CbAlpha, CbOffset,           \
                           Cr0, Cr1, Cr2, CrAlpha, CrOffset,           \
                           cx_i_bits, cx_f_bits, co_i_bits, co_f_bits, \
                           co_vid_bits, co_vid_tbl_bits)               \
{                                                                      \
    BVDC_P_MAKE_CSC_AB_CX(Y0,       cx_i_bits, cx_f_bits),             \
    BVDC_P_MAKE_CSC_AB_CX(Y1,       cx_i_bits, cx_f_bits),             \
    BVDC_P_MAKE_CSC_AB_CX(Y2,       cx_i_bits, cx_f_bits),             \
    BVDC_P_MAKE_CSC_AB_CX(YAlpha,   cx_i_bits, cx_f_bits),             \
    BVDC_P_MAKE_CSC_AB_CO(YOffset,  co_i_bits, co_f_bits,              \
                          co_vid_bits, co_vid_tbl_bits),               \
                                                                       \
    BVDC_P_MAKE_CSC_AB_CX(Cb0,      cx_i_bits, cx_f_bits),             \
    BVDC_P_MAKE_CSC_AB_CX(Cb1,      cx_i_bits, cx_f_bits),             \
    BVDC_P_MAKE_CSC_AB_CX(Cb2,      cx_i_bits, cx_f_bits),             \
    BVDC_P_MAKE_CSC_AB_CX(CbAlpha,  cx_i_bits, cx_f_bits),             \
    BVDC_P_MAKE_CSC_AB_CO(CbOffset, co_i_bits, co_f_bits,              \
                          co_vid_bits, co_vid_tbl_bits),               \
                                                                       \
    BVDC_P_MAKE_CSC_AB_CX(Cr0,      cx_i_bits, cx_f_bits),             \
    BVDC_P_MAKE_CSC_AB_CX(Cr1,      cx_i_bits, cx_f_bits),             \
    BVDC_P_MAKE_CSC_AB_CX(Cr2,      cx_i_bits, cx_f_bits),             \
    BVDC_P_MAKE_CSC_AB_CX(CrAlpha,  cx_i_bits, cx_f_bits),             \
    BVDC_P_MAKE_CSC_AB_CO(CrOffset, co_i_bits, co_f_bits,              \
                          co_vid_bits, co_vid_tbl_bits),               \
                                                                       \
    cx_i_bits,                                                         \
    cx_f_bits,                                                         \
    co_i_bits,                                                         \
    co_f_bits,                                                         \
    co_vid_bits                                                        \
}

/* CMP's Csc */
#define BVDC_P_MAKE_CMP_CSC_AB(Y0, Y1, Y2, YOffset,                    \
                               Cb0, Cb1, Cb2, CbOffset,                \
                               Cr0, Cr1, Cr2, CrOffset)                \
    BVDC_P_MAKE_CSC_AB(                                                \
    Y0, Y1, Y2, 0, YOffset,                                            \
    Cb0, Cb1, Cb2, 0, CbOffset,                                        \
    Cr0, Cr1, Cr2, 0, CrOffset,                                        \
    BVDC_P_CSC_CMP_AB_CX_I_BITS,                                       \
    BVDC_P_CSC_CMP_AB_CX_F_BITS,                                       \
    BVDC_P_CSC_CMP_AB_CO_I_BITS,                                       \
    BVDC_P_CSC_CMP_AB_CO_F_BITS,                                       \
    BVDC_P_CSC_CMP_AB_CO_VID_BITS,                                     \
    BVDC_P_CSC_CMP_AB_CO_VID_TBL_BITS)

/* GFD's Csc */
#define BVDC_P_MAKE_GFD_CSC_AB(Y0, Y1, Y2, YAlpha, YOffset,                     \
                               Cb0, Cb1, Cb2, CbAlpha, CbOffset,                \
                               Cr0, Cr1, Cr2, CrAlpha, CrOffset)                \
    BVDC_P_MAKE_CSC_AB(                                                         \
    Y0, Y1, Y2, (YAlpha / (1 << BVDC_P_CSC_GFD_MA_CO_VID_BITS)), YOffset,       \
    Cb0, Cb1, Cb2, (CbAlpha / (1 << BVDC_P_CSC_GFD_MA_CO_VID_BITS)), CbOffset,  \
    Cr0, Cr1, Cr2, (CrAlpha / (1 << BVDC_P_CSC_GFD_MA_CO_VID_BITS)), CrOffset,  \
    BVDC_P_CSC_GFD_MA_CX_I_BITS,                                                \
    BVDC_P_CSC_GFD_MA_CX_F_BITS,                                                \
    BVDC_P_CSC_GFD_MA_CO_I_BITS,                                                \
    BVDC_P_CSC_GFD_MA_CO_F_BITS,                                                \
    BVDC_P_CSC_GFD_MA_CO_VID_BITS,                                              \
    BVDC_P_CSC_GFD_MA_CO_VID_TBL_BITS)

#define BVDC_P_MAKE_CMP_NL_LR_XY(x, y) \
    (((uint16_t)BMTH_FIX_SIGNED_FTOFIX(x, BVDC_P_CSC_CMP_LR_XY_I_BITS, BVDC_P_CSC_CMP_LR_XY_F_BITS)) << BVDC_P_CSC_CMP_LR_X_SHIFT) | \
    (((uint16_t)BMTH_FIX_SIGNED_FTOFIX(y, BVDC_P_CSC_CMP_LR_XY_I_BITS, BVDC_P_CSC_CMP_LR_XY_F_BITS)) << BVDC_P_CSC_CMP_LR_Y_SHIFT)

#define BVDC_P_MAKE_CMP_NL_LR_SLOPE(m,e) \
    (((uint16_t)BMTH_FIX_SIGNED_FTOFIX(m, BVDC_P_CSC_CMP_LR_SLP_M_I_BITS, BVDC_P_CSC_CMP_LR_SLP_M_F_BITS)) << BVDC_P_CSC_CMP_LR_SLP_M_SHIFT) | \
    (((uint16_t)BMTH_FIX_SIGNED_FTOFIX(e, BVDC_P_CSC_CMP_LR_SLP_E_I_BITS, BVDC_P_CSC_CMP_LR_SLP_E_F_BITS)) << BVDC_P_CSC_CMP_LR_SLP_E_SHIFT)

#define BVDC_P_MAKE_CMP_NL_LR_ADJ(num_pts,                                      \
                                  x0, y0, mantissa0, exp0,                      \
                                  x1, y1, mantissa1, exp1,                      \
                                  x2, y2, mantissa2, exp2,                      \
                                  x3, y3, mantissa3, exp3,                      \
                                  x4, y4, mantissa4, exp4,                      \
                                  x5, y5, mantissa5, exp5,                      \
                                  x6, y6, mantissa6, exp6,                      \
                                  x7, y7, mantissa7, exp7)                      \
{                                                                               \
    num_pts,                                                                    \
    {                                                                           \
        BVDC_P_MAKE_CMP_NL_LR_SLOPE(mantissa0, exp0),                           \
        BVDC_P_MAKE_CMP_NL_LR_SLOPE(mantissa1, exp1),                           \
        BVDC_P_MAKE_CMP_NL_LR_SLOPE(mantissa2, exp2),                           \
        BVDC_P_MAKE_CMP_NL_LR_SLOPE(mantissa3, exp3),                           \
        BVDC_P_MAKE_CMP_NL_LR_SLOPE(mantissa4, exp4),                           \
        BVDC_P_MAKE_CMP_NL_LR_SLOPE(mantissa5, exp5),                           \
        BVDC_P_MAKE_CMP_NL_LR_SLOPE(mantissa6, exp6),                           \
        BVDC_P_MAKE_CMP_NL_LR_SLOPE(mantissa7, exp7)                            \
    },                                                                          \
    {                                                                           \
        BVDC_P_MAKE_CMP_NL_LR_XY(x0, y0),                                       \
        BVDC_P_MAKE_CMP_NL_LR_XY(x1, y1),                                       \
        BVDC_P_MAKE_CMP_NL_LR_XY(x2, y2),                                       \
        BVDC_P_MAKE_CMP_NL_LR_XY(x3, y3),                                       \
        BVDC_P_MAKE_CMP_NL_LR_XY(x4, y4),                                       \
        BVDC_P_MAKE_CMP_NL_LR_XY(x5, y5),                                       \
        BVDC_P_MAKE_CMP_NL_LR_XY(x6, y6),                                       \
        BVDC_P_MAKE_CMP_NL_LR_XY(x7, y7)                                        \
    }                                                                           \
}

#define BVDC_P_CSC_VIDEO_DATA_BITS      (pCscCoeffs->usCoVideoBits)

/* Color adjustment values */
#define BVDC_P_CONTRAST_VAL_MIN INT16_MIN
#define BVDC_P_CONTRAST_VAL_MAX INT16_MAX
#define BVDC_P_BRIGHTNESS_VAL_MIN INT16_MIN
#define BVDC_P_BRIGHTNESS_VAL_MAX INT16_MAX
#define BVDC_P_SATURATION_VAL_MIN INT16_MIN
#define BVDC_P_SATURATION_VAL_MAX INT16_MAX
#define BVDC_P_HUE_VAL_MIN INT16_MIN
#define BVDC_P_HUE_VAL_MAX INT16_MAX
#define BVDC_P_COLORTEMP_VAL_MIN  INT16_MIN
#define BVDC_P_COLORTEMP_VAL_MAX  INT16_MAX

#define BVDC_P_BRIGHTNESS_MAX      (1 << BVDC_P_CSC_VIDEO_DATA_BITS) /* 1024 on 3563 CMP, 256 on others */
#define BVDC_P_LUMA_BLACK_OFFSET   (64  >> (10 - BVDC_P_CSC_VIDEO_DATA_BITS))
#define BVDC_P_CHROMA_BLACK_OFFSET (512 >> (10 - BVDC_P_CSC_VIDEO_DATA_BITS))

/* Maximum, Minimum, and Center Values of Color Temperature in 100s of Kelvin */
#define BVDC_P_COLORTEMP_KELVIN_MAX    150
#define BVDC_P_COLORTEMP_KELVIN_MIN     15
#define BVDC_P_COLORTEMP_KELVIN_CENTER  65

/* Color adjustment range coefficients */
/* These values have been changed several times to comply with different
 * requests.  The following describes how to convert contrast, saturation,
 * hue, and brightness values between code with different minimums and
 * maximum values.
 *
 * Saturation and Contrast
 *
 * for values < 0:  new value = old value(1 - old min/1 - new min)
 * for values > 0:  new value = old value(old max - 1/new max - 1)
 *
 * for saturation,  min = (1 - BVDC_P_SATURATION_FIX_KA_RANGE) or 0 if it does not exist.
 *                  max = (1 + BVDC_P_SATURATION_FIX_KA_RANGE) or old BVDC_P_SATURATION_FIX_KA_MAX.
 *
 * for contrast,    min = BVDC_P_CONTRAST_FIX_K_MIN
 *                  max = BVDC_P_CONTRAST_FIX_K_MAX
 *
 *
 * Hue and Brightness
 *
 * new value = old value(old max/new max)
 *
 * for hue,         max = BVDC_P_HUE_FIX_KH_MAX
 *
 * for brightness,  max = BVDC_P_BRIGHTNESS_K_MAX
 */
#define BVDC_P_CONTRAST_FIX_K_MIN    0x0 /* 0.0 */
#define BVDC_P_CONTRAST_FIX_K_MAX    BMTH_FIX_SIGNED_ITOFIX(4, BVDC_P_FIX_INT_BITS, BVDC_P_FIX_FRACTION_BITS) /* 4.0 */
#define BVDC_P_SATURATION_FIX_KA_MAX BMTH_FIX_SIGNED_ITOFIX(4, BVDC_P_FIX_INT_BITS, BVDC_P_FIX_FRACTION_BITS) /* 4.0 */
#define BVDC_P_HUE_FIX_KH_MAX        BVDC_P_FIX_PI /* 180 degrees */
#define BVDC_P_BRIGHTNESS_K_MAX      BVDC_P_BRIGHTNESS_MAX /* 1024 on 3563 CMP, 256 on others */

#define BVDC_P_FIX_PI                BMTH_FIX_SIGNED_GET_PI(BVDC_P_FIX_INT_BITS, BVDC_P_FIX_FRACTION_BITS) /* pi */

#define BVDC_P_FIX_INT_BITS         (BVDC_P_FIX_MAX_BITS - BVDC_P_FIX_FRACTION_BITS)
#define BVDC_P_FIX_FRACTION_BITS    BVDC_P_CX_FRACTION_BITS

#define BVDC_P_CX_INT_BITS          (pCscCoeffs->usCxIntBits)
#define BVDC_P_CX_FRACTION_BITS     (pCscCoeffs->usCxFractBits)
#define BVDC_P_CX_MASK              (0xFFFFFFFF >> (BVDC_P_FIX_MAX_BITS - (BVDC_P_CX_INT_BITS + BVDC_P_CX_FRACTION_BITS)))
#define BVDC_P_CX_SIGN_MASK         (1 << (BVDC_P_CX_INT_BITS + BVDC_P_CX_FRACTION_BITS))

#define BVDC_P_CO_INT_BITS          (pCscCoeffs->usCoIntBits)
#define BVDC_P_CO_FRACTION_BITS     (pCscCoeffs->usCoFractBits)
#define BVDC_P_CO_MASK              (0xFFFFFFFF >> (BVDC_P_FIX_MAX_BITS - (BVDC_P_CO_INT_BITS + BVDC_P_CO_FRACTION_BITS)))
#define BVDC_P_CO_SIGN_MASK         (1 << (BVDC_P_CO_INT_BITS + BVDC_P_CO_FRACTION_BITS))

/* fixed point conversion macros */

/* fixed to integer */
#define BVDC_P_CSC_FIXTOI(x) \
    (BMTH_FIX_SIGNED_FIXTOI(x, BVDC_P_FIX_INT_BITS, BVDC_P_FIX_FRACTION_BITS))

/* integer to fixed */
#define BVDC_P_CSC_ITOFIX(x) \
    (BMTH_FIX_SIGNED_ITOFIX(x, BVDC_P_FIX_INT_BITS, BVDC_P_FIX_FRACTION_BITS))

/* fixed to float */
/* Useful for debugging.  Comment out uses of these macros when checking in due to lack of
 * float support on some platforms. */
#define BVDC_P_CSC_CXTOF(x) \
    (((int32_t)((BVDC_P_CX_SIGN_MASK & x) ? -((BVDC_P_CX_MASK & ~x) + 1) : x) / (float)(1 << BVDC_P_CX_FRACTION_BITS)))

#define BVDC_P_CSC_COTOF(x) \
    (((int32_t)((BVDC_P_CO_SIGN_MASK & x) ? -((BVDC_P_CO_MASK & ~x) + 1) : x) / (float)(1 << BVDC_P_CO_FRACTION_BITS)))

/* fixed point operation multiply */
#define BVDC_P_CSC_FIX_MUL(x, y) \
    (BMTH_FIX_SIGNED_MUL(x, y, BVDC_P_FIX_INT_BITS, BVDC_P_FIX_FRACTION_BITS, \
                               BVDC_P_FIX_INT_BITS, BVDC_P_FIX_FRACTION_BITS, \
                               BVDC_P_FIX_INT_BITS, BVDC_P_FIX_FRACTION_BITS))

/* alphas are stored as fractionals and need to be converted to integers
 * for calculations.  Instead of converting to integers by shifting
 * BVDC_P_CSC_VIDEO_DATA_BITS up here, we perform multiplication as if
 * alpha had BVDC_P_CSC_VIDEO_DATA_BITS less fractional bits, which is
 * equivalent and avoids overflow.
 *
 * we also reduce non-alpha offsets to their native fixed-point precision
 * before multiplying to avoid overflow.
 */
#define BVDC_P_CSC_FIX_MUL_OFFSET(x, y, matrix) \
    (BVDC_P_CSC_USE_ALPHA(matrix) ? \
     BMTH_FIX_SIGNED_MUL(x, y >> BVDC_P_CSC_VIDEO_DATA_BITS, \
                         BVDC_P_FIX_INT_BITS, BVDC_P_FIX_FRACTION_BITS,   \
                         BVDC_P_FIX_INT_BITS + BVDC_P_CSC_VIDEO_DATA_BITS, BVDC_P_FIX_FRACTION_BITS - BVDC_P_CSC_VIDEO_DATA_BITS,   \
                         BVDC_P_FIX_INT_BITS, BVDC_P_FIX_FRACTION_BITS) : \
     BMTH_FIX_SIGNED_MUL(x, BVDC_P_CSC_FIXTOCO32(y), \
                         BVDC_P_FIX_INT_BITS, BVDC_P_FIX_FRACTION_BITS,   \
                         BVDC_P_FIX_MAX_BITS - BVDC_P_CO_FRACTION_BITS, BVDC_P_CO_FRACTION_BITS,   \
                         BVDC_P_FIX_INT_BITS, BVDC_P_FIX_FRACTION_BITS))

/* clamp internal fixed point value to fit within values supported by hardware bits */
#define BVDC_P_CSC_FIX_CLAMPTOCX(x) \
    (((int32_t)x >= (1 << (BVDC_P_CX_INT_BITS + BVDC_P_FIX_FRACTION_BITS)))? \
        ((1 << (BVDC_P_CX_INT_BITS + BVDC_P_FIX_FRACTION_BITS)) - 1) : \
     (((int32_t)x <= ~(1 << (BVDC_P_CX_INT_BITS + BVDC_P_FIX_FRACTION_BITS)))? \
        ~((1 << (BVDC_P_CX_INT_BITS + BVDC_P_FIX_FRACTION_BITS)) - 1) : \
      (int32_t)x))

#define BVDC_P_CSC_FIX_CLAMPTOCO(x) \
    (((int32_t)x >= (1 << (BVDC_P_CO_INT_BITS + BVDC_P_FIX_FRACTION_BITS)))? \
        ((1 << (BVDC_P_CO_INT_BITS + BVDC_P_FIX_FRACTION_BITS)) - 1) : \
     (((int32_t)x <= ~(1 << (BVDC_P_CO_INT_BITS + BVDC_P_FIX_FRACTION_BITS)))? \
        ~((1 << (BVDC_P_CO_INT_BITS + BVDC_P_FIX_FRACTION_BITS)) - 1) : \
      (int32_t)x))

/* fixed point operation divide*/
#define BVDC_P_CSC_FIX_DIV(x, y) \
    (BMTH_FIX_SIGNED_DIV(x, y, BVDC_P_FIX_INT_BITS, BVDC_P_FIX_FRACTION_BITS, \
                               BVDC_P_FIX_INT_BITS, BVDC_P_FIX_FRACTION_BITS, \
                               BVDC_P_FIX_INT_BITS, BVDC_P_FIX_FRACTION_BITS))

/* convert CX coeffs to common fixed notation */
#define BVDC_P_CSC_CXTOFIX(x) \
    (BMTH_FIX_SIGNED_CONVERT(x, BVDC_P_CX_INT_BITS, BVDC_P_CX_FRACTION_BITS, \
                                BVDC_P_FIX_INT_BITS, BVDC_P_FIX_FRACTION_BITS))

/* convert common fixed notation to CX coeffs */
#define BVDC_P_CSC_FIXTOCX(x) \
    (BMTH_FIX_SIGNED_CONVERT(BVDC_P_CSC_FIX_CLAMPTOCX(x), BVDC_P_FIX_INT_BITS, BVDC_P_FIX_FRACTION_BITS, \
                                BVDC_P_CX_INT_BITS, BVDC_P_CX_FRACTION_BITS))

/* convert CO offsets to common fixed notation */
#define BVDC_P_CSC_COTOFIX(x) \
    (BMTH_FIX_SIGNED_CONVERT(x, BVDC_P_CO_INT_BITS, BVDC_P_CO_FRACTION_BITS, \
                                BVDC_P_FIX_INT_BITS, BVDC_P_FIX_FRACTION_BITS))

/* convert common fixed notation to CO offsets */
#define BVDC_P_CSC_FIXTOCO(x) \
    (BMTH_FIX_SIGNED_CONVERT(BVDC_P_CSC_FIX_CLAMPTOCO(x), BVDC_P_FIX_INT_BITS, BVDC_P_FIX_FRACTION_BITS, \
                                BVDC_P_CO_INT_BITS, BVDC_P_CO_FRACTION_BITS))

#define BVDC_P_CSC_FIXTOCO32(x) \
    (BMTH_FIX_SIGNED_CONVERT(x, BVDC_P_FIX_INT_BITS, BVDC_P_FIX_FRACTION_BITS, \
                                BVDC_P_FIX_MAX_BITS - BVDC_P_CO_FRACTION_BITS, BVDC_P_CO_FRACTION_BITS))

/* convert CO offsets to integer representation */
#define BVDC_P_CSC_COTOI(x) \
    (BMTH_FIXTOI(x, BVDC_P_CO_INT_BITS, BVDC_P_CO_FRACTION_BITS))

/* sin, with linear interpolation */
#define BVDC_P_CSC_FIX_SIN(x) \
    (BMTH_FIX_SIGNED_SIN(x, BVDC_P_FIX_INT_BITS, BVDC_P_FIX_FRACTION_BITS, \
                            BVDC_P_FIX_INT_BITS, BVDC_P_FIX_FRACTION_BITS))

/* cos, with linear interpolation */
#define BVDC_P_CSC_FIX_COS(x) \
    (BMTH_FIX_SIGNED_COS(x, BVDC_P_FIX_INT_BITS, BVDC_P_FIX_FRACTION_BITS, \
                            BVDC_P_FIX_INT_BITS, BVDC_P_FIX_FRACTION_BITS))

/* Convert csc matrix object to 4x4 matrix of fixed point values */
#define BVDC_P_CSC_MAKE4X4(matrix4x4, cscmatrix)                 \
    matrix4x4[0][0] = BVDC_P_CSC_CXTOFIX(cscmatrix->usY0);     \
    matrix4x4[0][1] = BVDC_P_CSC_CXTOFIX(cscmatrix->usY1);     \
    matrix4x4[0][2] = BVDC_P_CSC_CXTOFIX(cscmatrix->usY2);     \
    matrix4x4[0][3] = BVDC_P_CSC_YOTOFIX(cscmatrix);           \
                                                                 \
    matrix4x4[1][0] = BVDC_P_CSC_CXTOFIX(cscmatrix->usCb0);    \
    matrix4x4[1][1] = BVDC_P_CSC_CXTOFIX(cscmatrix->usCb1);    \
    matrix4x4[1][2] = BVDC_P_CSC_CXTOFIX(cscmatrix->usCb2);    \
    matrix4x4[1][3] = BVDC_P_CSC_CBOTOFIX(cscmatrix);          \
                                                                 \
    matrix4x4[2][0] = BVDC_P_CSC_CXTOFIX(cscmatrix->usCr0);    \
    matrix4x4[2][1] = BVDC_P_CSC_CXTOFIX(cscmatrix->usCr1);    \
    matrix4x4[2][2] = BVDC_P_CSC_CXTOFIX(cscmatrix->usCr2);    \
    matrix4x4[2][3] = BVDC_P_CSC_CROTOFIX(cscmatrix);          \
                                                                 \
    matrix4x4[3][0] = 0;                                         \
    matrix4x4[3][1] = 0;                                         \
    matrix4x4[3][2] = 0;                                         \
    matrix4x4[3][3] = BVDC_P_CSC_ITOFIX(1)

#define BVDC_P_CSC_MAKE4X4_MTH(matrix4x4_mth, cscmatrix)         \
    matrix4x4_mth.ulSize = 4;                                    \
    matrix4x4_mth.ulFractBits = cscmatrix->usCxFractBits;        \
    BVDC_P_CSC_MAKE4X4(matrix4x4_mth.data, cscmatrix)

#define BVDC_P_IS_SDR(eotf) \
    ((eotf)==BAVC_HDMI_DRM_EOTF_eSDR || (eotf)==BAVC_HDMI_DRM_EOTF_eFuture)

#if ((BVDC_P_SUPPORT_CMP_NON_LINEAR_CSC==0) || (BVDC_P_CMP_NON_LINEAR_CSC_VER <= BVDC_P_NL_CSC_VER_1))
#define BVDC_P_BYPASS_COLOR_CONV(eInEotf, eOutEotf) \
    (BVDC_P_IS_SDR(eInEotf) != BVDC_P_IS_SDR(eOutEotf))
#endif

#define BVDC_P_IS_BT2020(mc) \
    (((mc) == BVDC_P_MatrixCoeffs_eBt2020_NCL) || ((mc) == BVDC_P_MatrixCoeffs_eBt2020_CL))

void BVDC_P_Csc_GetHdDviTable_isr
    ( BVDC_P_CscCoeffs                *pCsc,
      BAVC_CscMode                     eCscMode );

void BVDC_P_Csc_ToMatrix_isr
    ( int32_t                          pl32_Matrix[BVDC_CSC_COEFF_COUNT],
      const BVDC_P_CscCoeffs          *pCsc,
      uint32_t                         ulShift );

void BVDC_P_Csc_FromMatrix_isr
    ( BVDC_P_CscCoeffs                *pCsc,
      const int32_t                    pl32_Matrix[BVDC_CSC_COEFF_COUNT],
      uint32_t                         ulShift );

void BVDC_P_Csc_ToMatrixDvo_isr
    ( int32_t                          pl32_Matrix[BVDC_CSC_COEFF_COUNT],
      const BVDC_P_CscCoeffs          *pCsc,
      uint32_t                         ulShift,
      bool                             bRgb );

void BVDC_P_Csc_FromMatrixDvo_isr
    ( BVDC_P_CscCoeffs                *pCsc,
      const int32_t                    pl32_Matrix[BVDC_CSC_COEFF_COUNT],
      uint32_t                         ulShift,
      bool                             bRgb );

void BVDC_P_Csc_ApplyContrast_isr
    ( int16_t                          sContrast,
      BVDC_P_CscCoeffs                *pCscCoeffs );

void BVDC_P_Csc_ApplySaturationAndHue_isr
    ( int16_t                          sSaturation,
      int16_t                          sHue,
      BVDC_P_CscCoeffs                *pCscCoeffs );

void BVDC_P_Csc_ApplyBrightness_isr
    ( int16_t                          sBrightness,
      BVDC_P_CscCoeffs                *pCscCoeffs );

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
      bool                             bUserCsc);

void BVDC_P_Csc_DvoApplyAttenuationRGB_isr
    ( int32_t                          lAttenuationR,
      int32_t                          lAttenuationG,
      int32_t                          lAttenuationB,
      int32_t                          lOffsetR,
      int32_t                          lOffsetG,
      int32_t                          lOffsetB,
      BVDC_P_CscCoeffs                *pCscCoeffs );

BERR_Code BVDC_P_Csc_ColorTempToAttenuationRGB
    ( int16_t                          sColorTemp,
      int32_t                         *plAttenuationR,
      int32_t                         *plAttenuationG,
      int32_t                         *plAttenuationB,
      BVDC_P_CscCoeffs                *pCscCoeffs );

void BVDC_P_Csc_MatrixInverse
    ( BVDC_P_CscCoeffs                *pCscCoeffs,
      BVDC_P_CscCoeffs                *pRetInvCscCoeffs );

void BVDC_P_Csc_ApplyYCbCrColor_isr
    ( BVDC_P_CscCoeffs                *pCscCoeffs,
      uint32_t                         ulColor0,
      uint32_t                         ulColor1,
      uint32_t                         ulColor2 );
void BVDC_P_Csc_Print_isr
    ( const BVDC_P_CscCoeffs          *pCsc );

BVDC_P_MatrixCoeffs BVDC_P_MatrixCoeffs_BAVC_to_BVDC_P
    ( BAVC_MatrixCoefficients          eMatrixCoeffs,
      bool                             bXvYcc);

#ifdef __cplusplus
}
#endif

#endif /* #ifndef BVDC_CSC_PRIV_H__ */
/* End of file. */
