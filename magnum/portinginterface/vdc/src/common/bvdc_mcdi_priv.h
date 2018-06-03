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
#ifndef BVDC_MCDI_PRIV_H__
#define BVDC_MCDI_PRIV_H__

#include "bvdc.h"
#include "bavc.h"
#include "bchp_common.h"
#include "bvdc_common_priv.h"
#include "bvdc_scaler_priv.h"
#include "bvdc_subrul_priv.h"
#include "bvdc_buffer_priv.h"
#include "bvdc_window_priv.h"

/***************************************************************************
 * Private defines
 ***************************************************************************/
/***************************************************************************
 * MCDI
 ***************************************************************************/
/* 7420 */
/* 7422 */
#define BVDC_P_MCDI_VER_2                        (2)
/* 7425 */
#define BVDC_P_MCDI_VER_3                        (3)
/* 7425Bx */
#define BVDC_P_MCDI_VER_4                        (4)
/* 7435Ax */
#define BVDC_P_MCDI_VER_5                        (5)

/* 7445Ax
 * PPB_0_WEAVE_DETECT_CTRL new constant register */
#define BVDC_P_MCDI_VER_6                        (6)

/* 7445Cx/7439Ax
 * PPB_0_IT_PCC_CONTROL need to write register */
#define BVDC_P_MCDI_VER_7                        (7)

/* 7445Dx
 * MDI_TOP_0_MULTI_CONTEXT_TO/FROM  */
#define BVDC_P_MCDI_VER_8                        (8)

/* 7439Bx CRBVN-290  MDI_FCB_0 :: MODE_CONTROL_0 :: PIXEL_CAP_ENABLE [17:17] */
#define BVDC_P_MCDI_VER_9                        (9)

/* 7216Ax, 7211Bx: Removed MDI_FCB, MDI_PPB, MDI_MEMC */
#define BVDC_P_MCDI_VER_10                        (10)
/***************************************************************************
 * MADR
 ***************************************************************************/
/* 7358 7552 7231 7346 7344 */
#define BVDC_P_MADR_VER_2                        (2)

/* 7422Ax 7425Ax, needs BVDC_P_MADR_HSIZE_WORKAROUND, BVDC_P_MADR_PICSIZE_WORKAROUND */
#define BVDC_P_MADR_VER_3                        (3)

/* 7358A1:
 * fixed BVDC_P_MADR_HSIZE_WORKAROUND */
#define BVDC_P_MADR_VER_4                        (4)

/* 7231Bx, 7344Bx, 7346Bx, 7552Bx, 7425Bx, 7429Ax, 7435Ax
 * fixed BVDC_P_MADR_HSIZE_WORKAROUND and BVDC_P_MADR_PICSIZE_WORKAROUND,
 * WAIT_DMA_DONE, etc. */
#define BVDC_P_MADR_VER_5                        (5)
/* 7435Ax SUSPEND_EOP */
#define BVDC_P_MADR_VER_6                        (6)
/* 7435Bx/7429Bx/7584Ax Low delay from 1 field to zero */
#define BVDC_P_MADR_VER_7                        (7)
/* 7439/7366Ax default value as golden set */
#define BVDC_P_MADR_VER_8                        (8)
/* 7364A0 remove MDI_TOP_0_MODE_CONTROL_0/UPDATE_SEL field*/
#define BVDC_P_MADR_VER_9                        (9)
/* 7445Dx MDI_TOP_1_MULTI_CONTEXT_TO/FROM  */
#define BVDC_P_MADR_VER_10                       (10)
/* 7268/7271 SW7552-178/HW7425-1196 SW workaround removal  */
#define BVDC_P_MADR_VER_11                       (11)

#ifdef __cplusplus
extern "C" {
#endif

#if ((BVDC_P_SUPPORT_MCDI_VER >= BVDC_P_MCDI_VER_8) && \
     (BVDC_P_SUPPORT_MADR_VER == BVDC_P_MADR_VER_10))
#define BVDC_P_SUPPORT_MCDI_SUPERSET            (0)
#else
#define BVDC_P_SUPPORT_MCDI_SUPERSET            (1)
#endif
    /***************************************************************************
    * Private register cracking macros
    ***************************************************************************/

#define BVDC_P_MCDI_SRC_HORZ_THRESHOLD         (960)

    /***************************************************************************
    * Software 2:2 reverse pulldown
    ***************************************************************************/
    /*
    This value is the minimum PCC count required to use the PCC algorithm.
    If the PCC is below this, the hardware 2:2 algorithm is used
    (as implemented in software).
    */
#define BVDC_P_MCDI_MIN_USABLE_PCC           2100 /* PR38878 */

    /*
    This value is the multiplier used by the pixel weave check.
    */
#define BVDC_P_MCDI_PW_MATCH_MULTIPLIER      0x10

    /*
    Indicates how strong the non-weave PCC counter has to be in relation to
    the weave PCC counter. A larger value means that the algorithm would
    be more selective in declaring 2:2 lock.
    */
#define BVDC_P_MCDI_PCC_NONMATCH_MATCH_RATIO 8

    /*
    Multiplier to apply to histogram[4] (measure of motion) used in conjunction
    with HISTOGRAM4_OFFSET.
    */
#define BVDC_P_MCDI_HISTOGRAM4_RATIO_NTSC         8
#define BVDC_P_MCDI_HISTOGRAM4_RATIO_PAL          10

    /*
    Allowable PCC count in the weave direction assuming that histogram[4]
    (measure of motion) is zero.  A larger value means that the algorithm
    would be less likely to lose lock in low motion.
    */
#define BVDC_P_MCDI_HISTOGRAM4_OFFSET_NTSC        (0x2c0)
#define BVDC_P_MCDI_HISTOGRAM4_OFFSET_PAL         (0x300)
#define BVDC_P_MCDI_HISTOGRAM4_OFFSET_PAL_OVERSAMPLE         (0x900)

    /*
    Amount to decrease the phase counter if the motion-adjusted
    (histogram[4]) PCC weave is too high. Recommendation is to make
    the value at least equal to REV22_LOCK_SAT_LEVEL - REV22_EXIT_LOCK_LEVEL
    so that the check can take the chip out of lock immediately.
    */
#define BVDC_P_MCDI_HISTOCHECK_DEC           15

    /*
    Tolerable delta between PCC counters in the weave and nonweave direction
    before we decrement the phase counter.
    */
#define BVDC_P_MCDI_WEAVE_22_THRESHOLD       150
#define BVDC_P_MCDI_WEAVE_22_THRESHOLD_OVERSAMPLE       0

    /*
    (Emulates RDB register function when PCC counters do not
    exceed MIN_USABLE_PCC).
    */
#define BVDC_P_MCDI_UPPER_MATCH_THRESH       (625 << 5)
#define BVDC_P_MCDI_LOWER_NONMATCH_THRESH    (468 << 5)
#define BVDC_P_MCDI_NONMATCH_MATCH_RATIO     8
#define BVDC_P_MCDI_REV22_LOCK_SAT_LEVEL     32

    /*
    If both PCC counts are below this value, we're not getting enough
    information for the PCC method to be useful.
    */
#define BVDC_P_MCDI_REV22_DONTCARE           150

    /*
    Bad weave threshold for sudden increases in PCC value.
    */
#define BVDC_P_MCDI_MAX_PCC_CHANGE           7400

    /*
    If PCC in the weave direction is higher than this threshold,
    algorithm will perform a check on the PCC against the repf_motion.
    */
#define BVDC_P_MCDI_RM_CHECK_THRESH_NTSC     2000
#define BVDC_P_MCDI_RM_CHECK_THRESH_PAL      2750

    /*
    Multiplier used in repf_motion check.
    */
#define BVDC_P_MCDI_RM_CHECK_RATIO_NTSC      29
#define BVDC_P_MCDI_RM_CHECK_RATIO_PAL       28


    /* constant value of 0 and 1       */
#define BVDC_P_MCDI_ZERO                      0x0
#define BVDC_P_MCDI_ONE                       0x1

    /*
    MADR register golden set value
    */
    /*
    FCN Module
    */

    /* IT_FIELD_PHASE_CALC_CONTROL_0 */
#if (BVDC_P_SUPPORT_MADR_VER <= BVDC_P_MADR_VER_7)
#define BVDC_P_MADR_REV32_ENTER_LOCK_LEVEL                 0x8
#define BVDC_P_MADR_REV32_EXIT_LOCK_LEVEL                  0x7
#else
#define BVDC_P_MADR_REV32_ENTER_LOCK_LEVEL                 0x6
#define BVDC_P_MADR_REV32_EXIT_LOCK_LEVEL                  0x5
#endif
#define BVDC_P_MADR_REV32_PHASE_MATCH_THRESH               0x4

/* IT_FIELD_PHASE_CALC_CONTROL_3 */
#define BVDC_P_MADR_REV22_ENTER_LOCK_LEVEL                 0x19
#define BVDC_P_MADR_REV22_EXIT_LOCK_LEVEL                  0x14
#define BVDC_P_MADR_REV22_NONMATCH_MATCH_RATIO             0x6
    /* IT_FIELD_PHASE_CALC_CONTROL_1 */
#if (BVDC_P_SUPPORT_MADR_VER <= BVDC_P_MADR_VER_7)
#define BVDC_P_MADR_REV32_REPF_VETO_LEVEL_HD               0x910
#define BVDC_P_MADR_REV32_REPF_VETO_LEVEL_SD               0x1d0
#else
#define BVDC_P_MADR_REV32_REPF_VETO_LEVEL_HD               0x270
#define BVDC_P_MADR_REV32_REPF_VETO_LEVEL_SD               0x9c
#endif
    /* IT_FIELD_PHASE_CALC_CONTROL_2             */
#if (BVDC_P_SUPPORT_MADR_VER <= BVDC_P_MADR_VER_7)
#define BVDC_P_MADR_REV32_LOCK_SAT_LEVEL                   0x20
#else
#define BVDC_P_MADR_REV32_LOCK_SAT_LEVEL                   0xdc
#endif
#define BVDC_P_MADR_REV32_BAD_EDIT_LEVEL_SD                0x927
#define BVDC_P_MADR_REV32_BAD_EDIT_LEVEL_HD                0x249c

    /* IT_FIELD_PHASE_CALC_CONTROL_5 */
#define BVDC_P_MADR_REV22_LOWER_THRESHOLD_SD               0xea
#define BVDC_P_MADR_REV22_LOWER_THRESHOLD_HD               0x3a8
#define BVDC_P_MADR_REV22_HIGHER_THRESHOLD_SD              0x1ff
#define BVDC_P_MADR_REV22_HIGHER_THRESHOLD_HD              0x7fc

    /* IT_FIELD_PHASE_CALC_CONTROL_8 */
#define BVDC_P_MADR_REV32_LOCK_SAT_THRESH                  0x8
#define BVDC_P_MADR_REV32_MIN_SIGMA_RANGE_SD               0x0
#define BVDC_P_MADR_REV32_MIN_SIGMA_RANGE_HD               0x40


    /* IT_FIELD_PHASE_CALC_CONTROL_9 */
#define BVDC_P_MADR_REV32_P0_MAX_SD                        2
#if (BVDC_P_SUPPORT_MADR_VER < BVDC_P_MADR_VER_10)
#define BVDC_P_MADR_REV32_P0_MAX_HD                        9
#else
#define BVDC_P_MADR_REV32_P0_MAX_HD                        8
#endif
#define BVDC_P_MADR_REV32_T1_RATIO                         0xb
#define BVDC_P_MADR_REV32_T1_MIN_SD                        0x14
#define BVDC_P_MADR_REV32_T1_MIN_HD                        0x50

    /* IT_FIELD_PHASE_CALC_CONTROL_10 */
#define BVDC_P_MADR_REV32_BW_MAX_SD                        75
#define BVDC_P_MADR_REV32_BW_MAX_HD                        0x12c
#define BVDC_P_MADR_REV32_BW_FF_DIFF_SD                    50
#define BVDC_P_MADR_REV32_BW_FF_DIFF_HD                    0xc8

    /* IT_FIELD_PHASE_CALC_CONTROL_11 */
#define BVDC_P_MADR_LG_PCC_THRESHOLD_SD                    25
#if (BVDC_P_SUPPORT_MADR_VER < BVDC_P_MADR_VER_10)
#define BVDC_P_MADR_LG_PCC_THRESHOLD_HD                    0x64
#else
#define BVDC_P_MADR_LG_PCC_THRESHOLD_HD                    0xB0
#endif
#define BVDC_P_MADR_TICKER_THRESHOLD_SD                    19
#define BVDC_P_MADR_TICKER_THRESHOLD_HD                    0x4c

    /* IT_FIELD_PHASE_CALC_CONTROL_12 */
#define BVDC_P_MADR_REV22_SAMEF_THRESHOLD_SD               0x40
#define BVDC_P_MADR_REV22_SAMEF_THRESHOLD_HD               0x100
#define BVDC_P_MADR_REV22_SAMEF_DTHRESHOLD_SD              0x28
#define BVDC_P_MADR_REV22_SAMEF_DTHRESHOLD_HD              0xa0

    /* IT_FIELD_PHASE_CALC_CONTROL_13 */
#define BVDC_P_MADR_REV22_SAMEF_PCCVETO_SD                 0x1c2
#define BVDC_P_MADR_REV22_SAMEF_PCCVETO_HD                 0x708
#define BVDC_P_MADR_REV22_UNDO_SWAP_SD                     0x20
#define BVDC_P_MADR_REV22_UNDO_SWAP_HD                     0x80

    /* IT_FIELD_PHASE_CALC_CONTROL_14 */
#define BVDC_P_MADR_REV22_BW_STAIR_SD                      125
#define BVDC_P_MADR_REV22_BW_STAIR_HD                      0x1f4
#define BVDC_P_MADR_REV22_BW_PCC_MAX_SD                    312
#define BVDC_P_MADR_REV22_BW_PCC_MAX_HD                    0x4e0

    /* IT_FIELD_PHASE_CALC_CONTROL_15 */
#define BVDC_P_MADR_REV22_BW_MAX_SD                        55
#define BVDC_P_MADR_REV22_BW_MAX_HD                        0xdc
#define BVDC_P_MADR_REV22_BW_FF_DIFF_SD                    36
#define BVDC_P_MADR_REV22_BW_FF_DIFF_HD                    0x90

    /* IT_FIELD_PHASE_CALC_CONTROL_18 */
#define BVDC_P_MADR_REV22_P0_MAX                           16
#define BVDC_P_MADR_REV22_T1_MIN_SD                        15
#define BVDC_P_MADR_REV22_T1_MIN_HD                        0x3c


    /* OBTS_DECAY */
#define BVDC_P_MADR_OBTS_DECAY                             0x8

    /* OBTS_HOLDOFF */
#define BVDC_P_MADR_OBTS_HOLDOFF                           0x3840

    /* OBTS_MAX_HOLDOFF */
#define BVDC_P_MADR_OBTS_MAX_HOLDOFF                       0x23280


    /* OBTS_OBTS_CORE                            */
#define BVDC_P_MADR_OBTS_CTRL_CORE                         0x7080
    /*
    PPB Module
    */
    /* MDI_PPB_0_QM_MAPPING_RANGE                   */
#define BVDC_P_MADR_QM_MAPPING_RANGE_VALUE_1               0x20
#define BVDC_P_MADR_QM_MAPPING_RANGE_VALUE_2               0x40
#define BVDC_P_MADR_QM_MAPPING_RANGE_VALUE_3               0x60

    /* MDI_PPB_0_MOTION_CAL_CONTROL                 */
#define BVDC_P_MADR_WEAVE_DETECT_THR                       0x10
#define BVDC_P_MADR_WEAVE_MOTION_THR_LOW                   0x16

    /* XCHROMA_CONTROL_4                            */
#define BVDC_P_MADR_XCHROMA_CTRL_MAX                       0xff

    /* LA_SCALE_0                         */
#define BVDC_P_MADR_LA_SCALE_0_DEGREE                      8

    /* ITT_PCC_CONTROL */
#define BVDC_P_MADR_TKR_CORING_THRE                0x10
#define BVDC_P_MADR_LUMA_CORING_THRE               0x8
#define BVDC_P_MADR_XOMA_CORING_THRE               0x4


    /*
    MCDI register golden set value
    */

/*
    FCB Module
    */

    /*MC_DECISION_CONTROL_0                      */
#define BVDC_P_MCDI_FCB_MC_CTRL_PCC_TOTAL_THRESH           0x180

    /*IT_FIELD_PHASE_CALC_CONTROL_1*/
#if (BVDC_P_MADR_VER_6<=BVDC_P_SUPPORT_MADR_VER)
#define BVDC_P_MCDI_REV32_REPF_VETO_LEVEL_SD               0x120
#else
#define BVDC_P_MCDI_REV32_REPF_VETO_LEVEL_SD               0x300
#endif
#define BVDC_P_MCDI_REV32_REPF_VETO_LEVEL_HD               0xF00
#define BVDC_P_MCDI_REV32_REPF_PIX_LEVEL_SD                0x300
#define BVDC_P_MCDI_REV32_REPF_PIX_LEVEL_HD                0xF00

    /*IT_FIELD_PHASE_CALC_CONTROL_2*/
#if (BVDC_P_MADR_VER_6<=BVDC_P_SUPPORT_MADR_VER)
#define BVDC_P_MCDI_REV32_BAD_EDIT_LEVEL_SD                0x1000
#else
#define BVDC_P_MCDI_REV32_BAD_EDIT_LEVEL_SD                0xa00
#endif
#define BVDC_P_MCDI_REV32_BAD_EDIT_LEVEL_HD                0x3200

    /*REV32_IT_FIELD_PHASE_CALC_CONTROL_4*/
    /*REV32_IT_FIELD_PHASE_CALC_CONTROL_4_MCDI   */
#define BVDC_P_MCDI_REV32_REPF_FL_RATIO                    0x8
#define BVDC_P_MCDI_REV32_REPF_FL_MIN                      0x400

    /*REV32_IT_FIELD_PHASE_CALC_CONTROL_6*/
#if (BVDC_P_MADR_VER_6<=BVDC_P_SUPPORT_MADR_VER)
#define BVDC_P_MCDI_REV32_BW_LG_PCC_RATIO                  0x2a
#else
#define BVDC_P_MCDI_REV32_BW_LG_PCC_RATIO                  0x32
#endif
#define BVDC_P_MCDI_REV32_BW_LG_PCC_MIN                    0x28

#if (BVDC_P_MADR_VER_8<=BVDC_P_SUPPORT_MADR_VER)
    /*REV32_IT_FIELD_PHASE_CALC_CONTROL_7*/
#define BVDC_P_MCDI_REV32_BW_LG_FF_OFFSET_HD               0x1194

    /*REV32_IT_FIELD_PHASE_CALC_CONTROL_8*/
#define BVDC_P_MCDI_REV32_REV32_BW_FEATHERING_MAX_HD       0x80
#define BVDC_P_MCDI_REV32_REV32_BW_FEATHERING_FF_DIFF_HD   0x80

    /*REV32_IT_FIELD_PHASE_CALC_CONTROL_9*/
#define BVDC_P_MCDI_REV32_REV32_T_T1_FEATH_MIN_HD          0x64
#endif

    /*  REV22_IT_FIELD_PHASE_CALC_CONTROL_0.     */
#define BVDC_P_MCDI_REV22_NONMATCH_MATCH_RATIO             0x8
#if (BVDC_P_MADR_VER_6<=BVDC_P_SUPPORT_MADR_VER)
#define BVDC_P_MCDI_REV22_ENTER_LOCK_LEVEL                 0xf
#define BVDC_P_MCDI_REV22_EXIT_LOCK_LEVEL                  0xe
#else
#define BVDC_P_MCDI_REV22_ENTER_LOCK_LEVEL                 0x19
#define BVDC_P_MCDI_REV22_EXIT_LOCK_LEVEL                  0x14
#endif

    /*  REV22_IT_FIELD_PHASE_CALC_CONTROL_3.     */
#define BVDC_P_MCDI_REV22_MIN_USABLE_PCC                   0x514
#define BVDC_P_MCDI_REV22_PW_MATCH_MULTIPLIER              0x240

    /*  REV22_IT_FIELD_PHASE_CALC_CONTROL_3_MCDI.*/
#define BVDC_P_MCDI_REV22_MIN_USABLE_PCC_MCDI              0x514
#define BVDC_P_MCDI_REV22_PW_MATCH_MULTIPLIER_MCDI         0x3E8

    /*  REV22_IT_FIELD_PHASE_CALC_CONTROL_8.     */
#define BVDC_P_MCDI_REV22_ALMOST_LOCK_LEVEL                0x17

    /*  REV22_IT_FIELD_PHASE_CALC_CONTROL_9.     */
#define BVDC_P_MCDI_REV22_BW_LG_PCC_RATIO                  0xB0
#define BVDC_P_MCDI_REV22_BW_LG_PCC_MIN                    0xA

    /*  REV22_IT_FIELD_PHASE_CALC_CONTROL_11.    */
#define BVDC_P_MCDI_REV22_BW_LG_PCC_MAXIMUM                0x7D0

    /*  BCHP_MDI_FCB_0_BWV_CONTROL_1             */
#define BVDC_P_MCDI_BWV_LUMA32_THD_SD                      0x44c
#define BVDC_P_MCDI_BWV_LUMA32_THD_HD                      0x3a98
#define BVDC_P_MCDI_BWV_LUMA32_AVG_THD_SD                  0x7d0
#define BVDC_P_MCDI_BWV_LUMA32_AVG_THD_HD                  0x2710
    /*  BCHP_MDI_FCB_0_BWV_CONTROL_3             */
#define BVDC_P_MCDI_BWV_TKR_PCC_MULT                       0xffff
#define BVDC_P_MCDI_BWV_TKR_VETO_LEVEL_SD                  0x80
#define BVDC_P_MCDI_BWV_TKR_VETO_LEVEL_HD                  0x280

#define BVDC_P_MCDI_SC_THD                                 0x20
#define BVDC_P_MCDI_SC_PST_THD                             0x8
#define BVDC_P_MCDI_SADBYBUSY_THR                          0x50
    /*
    PPB Module
    */
    /* MC_BLEND                                  */
#define BVDC_P_MCDI_MC_BLEND_K0                            0x120

    /* MEMC_CTRL                                 */
#if (BVDC_P_MCDI_VER_3 >= BVDC_P_SUPPORT_MCDI_VER )
#define BVDC_P_MCDI_MEMC_CTRL_LAMDA                        0x4
#elif (BVDC_P_MCDI_VER_4 <= BVDC_P_SUPPORT_MCDI_VER )
#define BVDC_P_MCDI_MEMC_CTRL_LAMDA                        0x10
#endif

    /*MC_COST_CTRL_02                            */
#define BVDC_P_MCDI_PCC_CNT_THD_1                          0x5
#define BVDC_P_MCDI_PCC_CNT_THD_0                          0x3
#define BVDC_P_MCDI_PCC_CNT_THD                            0x4
#define BVDC_P_MCDI_ZERO_PCC_CORE_THD                      0x10
#define BVDC_P_MCDI_MC_PCC_CORE_THD                        0x4

    /*MC_COST_CTRL_04                            */
#define BVDC_P_MCDI_EDGE_K2                                0xe
#define BVDC_P_MCDI_EDGE_K1                                0x2
#define BVDC_P_MCDI_EDGE_K0                                0xc

    /* MC_MC_THD                                 */
#if (BVDC_P_MCDI_VER_3 >= BVDC_P_SUPPORT_MCDI_VER )
#define BVDC_P_MCDI_MC_MC_THD_HIGH                         0x28
#elif (BVDC_P_MCDI_VER_4 <= BVDC_P_SUPPORT_MCDI_VER )
#define BVDC_P_MCDI_MC_MC_THD_HIGH                         0x27
#endif
#define BVDC_P_MCDI_MC_MC_THD_LOW                          0x8

    /* MC_ZERO_THD_0                             */
#define BVDC_P_MCDI_ZERO_THD_LOW_1                         0xc
#define BVDC_P_MCDI_ZERO_THD_LOW_0                         0xc

    /* MC_ZERO_THD_1                             */
#if (BVDC_P_MCDI_VER_3 >= BVDC_P_SUPPORT_MCDI_VER )
#define BVDC_P_MCDI_ZERO_THD_HIGH                          0x14
#elif (BVDC_P_MCDI_VER_4 <= BVDC_P_SUPPORT_MCDI_VER )
#define BVDC_P_MCDI_ZERO_THD_HIGH                          0x13
#endif

    /* MC_SHIFT_MC_THD                           */
#define BVDC_P_MCDI_SHIFT_MC_THD_1                         0x74a
#if (BVDC_P_MCDI_VER_3 >= BVDC_P_SUPPORT_MCDI_VER )
#define BVDC_P_MCDI_SHIFT_MC_THD_0                         0x80
#elif (BVDC_P_MCDI_VER_4 <= BVDC_P_SUPPORT_MCDI_VER )
#define BVDC_P_MCDI_SHIFT_MC_THD_0                         0x81
#endif
    /* MC_EDGE_THD_0                             */
#define BVDC_P_MCDI_EDGE_THD_LOW_1                         0x2
#define BVDC_P_MCDI_EDGE_THD_LOW_0                         0x0

    /* MC_EDGE_THD_1                             */
#define BVDC_P_MCDI_EDGE_THD_HIGH_1                        0xfe

#if (BVDC_P_MCDI_VER_3 >= BVDC_P_SUPPORT_MCDI_VER )
#define BVDC_P_MCDI_EDGE_THD_HIGH_0                        0x64
    /* MC_EDGE_THD_2                             */
#define BVDC_P_MCDI_EDGE_THD_HIGH_2                        0xFA0
#elif (BVDC_P_MCDI_VER_4 <= BVDC_P_SUPPORT_MCDI_VER )
#define BVDC_P_MCDI_EDGE_THD_HIGH_0                        0x65
    /* MC_EDGE_THD_2                             */
#define BVDC_P_MCDI_EDGE_THD_HIGH_2                        0xFA1
#endif

    /* MC_VM_THD                                 */
#define BVDC_P_MCDI_VM_THD_1                               0x0
#define BVDC_P_MCDI_VM_THD_0                               0x0

    /* MC_SHIFT_ZERO_THD                         */
#define BVDC_P_MCDI_SHIFT_ZERO_THD                         0x3B8

    /* MOVIE_MC_THD                              */
#define BVDC_P_MCDI_MOVIE_MC_THD                           0xa8

    /* BCHP_MDI_PPB_0_MVD_K1_VALUE               */
#define BVDC_P_MVD_K1_VALUE                                0xc80

    /* XCHROMA_CONTROL_6                         */
#define BVDC_P_MCDI_MA_420422_BLEND_STRENGTH               0x8
#define BVDC_P_MCDI_IT_420422_BLEND_STRENGTH               0x8

    /* BCHP_MDI_PPB_0_GMV_NUM_THD                */
#define BVDC_P_MCDI_GMV_NUM_THD                            0x9c40

#if (BVDC_P_SUPPORT_MCDI_VER >= BVDC_P_MCDI_VER_8)
#define BVDC_P_MCDI_GMV_NUM_THD_HD                         196500
#define BVDC_P_MCDI_GMV_NUM_THD_SD                         31800
#define BVDC_P_MCDI_GMV_NUM_THD_PAL                        38040
#define BVDC_P_MCDI_RANGE_GMV_Y                            6
#define BVDC_P_MCDI_RANGE_GMV_X                            8
#endif

    /* MH_MAPPING_VALUE                          */
#define BVDC_P_MCDI_MH_MAPPING_VALUE_3                     0xFF
#define BVDC_P_MCDI_MH_MAPPING_VALUE_2                     0x60
#define BVDC_P_MCDI_MH_MAPPING_VALUE_1                     0x30

    /* MA_MC_BLEND_CTRL_0                        */
#define BVDC_P_MCDI_STAT_THRESH_GMV_CHOSEN                 0x3FF
#define BVDC_P_MCDI_STAT_THRESH                            0x17

    /* MA_MC_BLEND_CTRL_1                        */
#define BVDC_P_MCDI_STAT_SCALE_GMV_CHOSEN                  0x100
#define BVDC_P_MCDI_STAT_SCALE                             0x10

    /*PPB_0_CONST_BLEND_CTRL_MAD*/
#define BVDC_P_MCDI_CONST_BLEND                            0x100
    /*
    due to HW requirement, FIELD_FREEZE can not be set for the first 4 fields
    */
#define BVDC_P_MCDI_TRICK_MODE_START_DELAY                  4

#define BVDC_P_MAD_SPATIAL(eGameMode)                         \
    ((eGameMode == BVDC_MadGameMode_e5Fields_ForceSpatial) || \
     (eGameMode == BVDC_MadGameMode_e4Fields_ForceSpatial) || \
     (eGameMode == BVDC_MadGameMode_e3Fields_ForceSpatial) || \
     (eGameMode == BVDC_MadGameMode_eMinField_ForceSpatial))

#define BVDC_P_MAD_LOWDELAY(eGameMode)                         \
    ((eGameMode != BVDC_MadGameMode_e5Fields_2Delay) && \
     (eGameMode != BVDC_MadGameMode_e4Fields_2Delay) && \
     (eGameMode != BVDC_MadGameMode_e3Fields_2Delay) && \
     (eGameMode != BVDC_MadGameMode_eOff))

/***************************************************************************
* Mcdi private data structures
***************************************************************************/
typedef struct BVDC_P_McdiRev22Statistics
{
    uint32_t   ulMatchWeave;
    uint32_t   ulNonMatchWeave;
    uint32_t   ulMatchUM;
    uint32_t   ulNonMatchUM;
    uint32_t   ulAvgWeave;
    uint32_t   ulPixelWeave;
    uint32_t   ulRepfMotion;
} BVDC_P_McdiRev22Statistics;

typedef struct BVDC_P_McdiRev32Statistics
{
    uint32_t           ulBwvCtrl5;
    uint32_t           ulPhaseCalc0;
    uint32_t           ulPhaseCalc1;
    uint32_t           ulPhaseCalc2;
    uint32_t           ulPhaseCalc8;
    uint32_t           ulPccLumaPcc;
    uint32_t           ulPccChromaPcc;
    uint32_t           ulPrevLumaPcc;
    uint32_t           ulPrevChromaPcc;
    uint32_t           ulHistogramBin0Reg;
    uint32_t           ulHistogramBin1Reg;
    uint32_t           ulHistogramBin2Reg;
    uint32_t           ulHistogramBin3Reg;
    uint32_t           ulHistogramBin4Reg;
    uint32_t           ulWndBias;

    bool               abRev32Locked[5];
    uint32_t           aulSigma[5];
    uint32_t           aulX[5];
    uint32_t           aulP[5];
    uint32_t           aulV[5];
} BVDC_P_McdiRev32Statistics;

typedef struct BVDC_P_McdiGameModeInfo
{
    BVDC_MadGameMode               eMode;
    uint16_t                       usDelay;
    uint16_t                       usPixelBufferCnt;
    const char                    *pchModeName;

} BVDC_P_McdiGameModeInfo;

/****************************************************************************
* Mcdi dirty bits to makr RUL building and executing dirty.
*/
typedef union
{
    struct
    {
        uint32_t                   bSize           : 1;
        uint32_t                   bPxlFmt         : 1;
        uint32_t                   bModeSwitch     : 1;
        uint32_t                   bGameMode       : 1;
        uint32_t                   bChannelChange  : 1;
        uint32_t                   bBuffer         : 1;
    } stBits;

    uint32_t aulInts [BVDC_P_DIRTY_INT_ARRAY_SIZE];
} BVDC_P_McdiDirtyBits;

typedef struct BVDC_P_McdiContext
{
    BDBG_OBJECT(BVDC_MDI)

    BREG_Handle                    hRegister;
    BVDC_Heap_Handle               hHeap;
    BVDC_P_WindowId                eWinId;

    BVDC_P_Compression_Settings    *pstCompression;

    /* flag initial state, require reset; */
    bool                           bInitial;

    /* OSD feature for MCDI */
    bool                           bEnableOsd;
    uint32_t                       ulOsdHpos;
    uint32_t                       ulOsdVpos;

    /* flag for changes */
    uint32_t                       ulUpdateAll;

    /* Optimized */
    int32_t                        lMcdiCutLeft; /* S11.6, same fmt as SclCut->lLeft */

    /* private fields. */
    BVDC_P_McdiId                  eId;
    uint32_t                       ulRegOffset; /* MCDI_0, MCDI_1, and etc. */
    uint32_t                       ulRegOffset1;/* MCDI_1, MCDI_2, and etc. */
    bool                           bMadr;
    uint32_t                       ulMosaicMaxChannels;

    /* Pixel Field Memory Store */
#if (BVDC_P_MAX_MCDI_BUFFER_COUNT > 0)
    BVDC_P_HeapNodePtr             apHeapNode[BAVC_MOSAIC_MAX][BVDC_P_MAX_MCDI_BUFFER_COUNT];
#endif
    uint32_t                       ulPxlBufCnt[BAVC_MOSAIC_MAX];
    uint32_t                       ulPxlBufSize[BAVC_MOSAIC_MAX];

    /* QM Field Memory Store */
    BVDC_P_HeapNodePtr             apQmHeapNode[BAVC_MOSAIC_MAX][BVDC_P_MAX(BVDC_P_MCDI_QM_BUFFER_COUNT, 1)]; /*7420 has no qm */
    uint32_t                       ulQmBufCnt[BAVC_MOSAIC_MAX];
    uint32_t                       ulQmBufSize[BAVC_MOSAIC_MAX];

    /* set as change is marked, clear after built into RUL */
    BVDC_P_McdiDirtyBits           stSwDirty;

    /* set after built into RUL, clear after executed */
    BVDC_P_McdiDirtyBits           stHwDirty;

    uint32_t                       ulHSize;
    uint32_t                       ulVSize;

    /* values for MCDI game mode */
    uint16_t                       usFeedCapture;
    uint16_t                       usCurQm;
    uint16_t                       usGameModeStartDelay;
    uint16_t                       usGameModeQmDelay;
    uint16_t                       usAllocBufferCnt;
    bool                           bBufferCntChanged;

    /* video source and fmt */
    BVDC_Deinterlace_ChromaSettings *pChromaSettings;

    /* mcdi user setting */
    BVDC_MadGameMode               eGameMode;
    bool                           bRev32Pulldown;
    bool                           bRev22Pulldown;

    /* sub-struct to manage vnet and rul build opreations */
    BVDC_P_SubRulContext           SubRul;

    /* delay for MCDI freeze transition */
    uint16_t                       usTrickModeStartDelay[BAVC_MOSAIC_MAX];

    BFMT_Orientation               eDispOrientation;
    BFMT_Orientation               eSrcOrientation;
    BVDC_P_Rect                    astRect[BAVC_MOSAIC_MAX];
    uint32_t                       ulMosaicInit;

} BVDC_P_McdiContext;


/***************************************************************************
 * Mcdi private functions
 ***************************************************************************/
#define BVDC_P_MCDI_GET_REG_OFFSET(mad_id)     (0)
#define BVDC_P_Mcdi_MuxAddr(hMcdi)             (0)
#define BVDC_P_Mcdi_PostMuxValue(hMcdi)        (0)


#if (BVDC_P_SUPPORT_MCVP)
/***************************************************************************
* {private}
*/
BERR_Code BVDC_P_Mcdi_Create
    ( BVDC_P_Mcdi_Handle              *phMcdi,
      BVDC_P_McdiId                    eMcdiId,
      BREG_Handle                      hRegister,
      BVDC_P_Resource_Handle           hResource );

void BVDC_P_Mcdi_Destroy
    ( BVDC_P_Mcdi_Handle               hMcdi );

void BVDC_P_Mcdi_Init_isr
    ( BVDC_P_Mcdi_Handle               hMcdi,
      BVDC_Window_Handle               hWindow);

void BVDC_P_Mcdi_BuildRul_SrcInit_isr
    ( BVDC_P_McdiContext              *pMcdi,
      BVDC_P_ListInfo                 *pList,
      BVDC_P_PictureNode              *pPicture );

void BVDC_P_Mcdi_BuildRul_SetEnable_isr
    ( BVDC_P_Mcdi_Handle               hMcdi,
      BVDC_P_ListInfo                 *pList,
      bool                             bEnable,
      BVDC_P_PictureNode              *pPicture,
      bool                             bInit);

bool BVDC_P_Mcdi_BeHardStart_isr
    ( bool                             bInit,
      BVDC_P_Mcdi_Handle               hMcdi );

void BVDC_P_Mcdi_GetUserConf_isr
    ( BVDC_P_Mcdi_Handle               hMcdi,
      BVDC_P_Deinterlace_Settings     *pMadSettings);

void BVDC_P_Mcdi_Init_Chroma_DynamicDefault_isr
    ( BVDC_P_Mcdi_Handle               hMcdi,
      BVDC_Deinterlace_ChromaSettings *pChromaSettings,
      const BFMT_VideoInfo            *pFmtInfo,
      bool                             bMfdSrc);

uint16_t BVDC_P_Mcdi_GetVsyncDelayNum_isr
    ( BVDC_P_Mcdi_Handle               hMcdi,
      BVDC_MadGameMode                 eGameMode);

uint16_t BVDC_P_Mcdi_GetPixBufCnt_isr
    ( bool                             bMadr,
      BVDC_MadGameMode                 eGameMode);

void BVDC_P_Mcdi_GetDeinterlacerType_isr
    ( BVDC_P_Mcdi_Handle               hMcdi,
      bool                            *pbMadr);

#if (BVDC_P_SUPPORT_MTG)
void BVDC_P_Mcdi_ReadOutPhase_isr
    ( BVDC_P_Mcdi_Handle               hMcdi,
      BVDC_P_PictureNode              *pPicture);
#endif /* BVDC_P_SUPPORT_MTG */
#else
#define BVDC_P_Mcdi_Create(hmcdi, eid, hreg, hres)               BDBG_ASSERT(0)
#define BVDC_P_Mcdi_Destroy(hmcdi)                               BDBG_ASSERT(0)
#define BVDC_P_Mcdi_Init_isr(hmcdi, hwin)                        BDBG_ASSERT(0)
#define BVDC_P_Mcdi_BuildRul_SrcInit_isr(hmcdi, plist, ppic)     BDBG_ASSERT(0)
#define BVDC_P_Mcdi_BeHardStart_isr(init, hmcdi)                 (0)
#define BVDC_P_Mcdi_GetUserConf_isr(hmcdi, pmadsettings)         BDBG_ASSERT(0)
#define BVDC_P_Mcdi_GetVsyncDelayNum_isr(hmcdi, egmode)          (0)
#define BVDC_P_Mcdi_GetPixBufCnt_isr(bmadr, egmode)              (0)
#define BVDC_P_Mcdi_GetDeinterlacerType_isr(hmcdi, bmadr)        BDBG_ASSERT(0)
#define BVDC_P_Mcdi_BuildRul_SetEnable_isr(hmcdi, plist, \
    enable, ppic, init)                                          BDBG_ASSERT(0)
#define BVDC_P_Mcdi_Init_Chroma_DynamicDefault_isr(hmcdi, \
    pchroma, pfmt, bmfdsrc )                                     BDBG_ASSERT(0)
#if (BVDC_P_SUPPORT_MTG)
#define BVDC_P_Mcdi_ReadOutPhase_isr(hmcdi, ppic)                BDBG_ASSERT(0)
#endif /* BVDC_P_SUPPORT_MTG */
#endif /* (BVDC_P_SUPPORT_MCVP) */
#ifdef __cplusplus
}
#endif
#endif /* #ifndef BVDC_MCDI_PRIV_H__ */
/* End of file. */
