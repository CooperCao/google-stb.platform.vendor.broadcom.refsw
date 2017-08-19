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
*
***************************************************************************/
#ifndef BVDC_SCALER_PRIV_H__
#define BVDC_SCALER_PRIV_H__

#include "bvdc.h"
#include "bchp_common.h"
#include "bvdc_common_priv.h"
#include "bvdc_subrul_priv.h"
#include "bvdc_buffer_priv.h"
#include "bchp_scl_0.h"

#ifdef __cplusplus
extern "C" {
#endif

    /* Core revs to track increatmental changes! */
#if 0
/* The following are legacy chips */
#define BVDC_P_SUPPORT_SCL_VER_1              (1) /* 7400 - Ax, Bx, Cx */
#define BVDC_P_SUPPORT_SCL_VER_2              (2) /* 7400 - Dx, Ex, others */
#define BVDC_P_SUPPORT_SCL_VER_3              (3) /* 3548 Ax */
#define BVDC_P_SUPPORT_SCL_VER_4              (4) /* 7420 */
#define BVDC_P_SUPPORT_SCL_VER_5              (5) /* 3548 B0 */
#define BVDC_P_SUPPORT_SCL_VER_6              (6) /* 7550: no HORIZ_*_04_05 from ver_4*/
#endif
#define BVDC_P_SUPPORT_SCL_VER_7              (7) /* 7422 */
#define BVDC_P_SUPPORT_SCL_VER_8              (8) /* 7231B0, 7344B0, 7346B0, 7425B0, Added more derring knobs */
#define BVDC_P_SUPPORT_SCL_VER_9              (9) /* 7429B HW7420-976 fixed SCL rev equal to 0.3.0.5 */
#define BVDC_P_SUPPORT_SCL_VER_10             (10)/* 7445, 7145 */
#define BVDC_P_SUPPORT_SCL_VER_11             (11)/* 7364A, 7366B 7445D*/
#define BVDC_P_SUPPORT_SCL_VER_12             (12)/* 7278B, 7260B */


    /***************************************************************************
    * Private register cracking macros
    ***************************************************************************/
#define BVDC_P_SCL_GET_REG_IDX(reg) \
    ((BCHP##_##reg - BCHP_SCL_0_REG_START) / sizeof(uint32_t))

    /* Get/Set reg data */
#define BVDC_P_SCL_GET_REG_DATA(reg) \
    (hScaler->aulRegs[BVDC_P_SCL_GET_REG_IDX(reg)])
#define BVDC_P_SCL_SET_REG_DATA(reg, data) \
    (BVDC_P_SCL_GET_REG_DATA(reg) = (uint32_t)(data))

    /* Get with index. */
#define BVDC_P_SCL_GET_REG_DATA_I(idx, reg) \
    (hScaler->aulRegs[BVDC_P_SCL_GET_REG_IDX(reg) + (idx)])

    /* Get field */
#define BVDC_P_SCL_GET_FIELD_NAME(reg, field) \
    (BVDC_P_GET_FIELD(BVDC_P_SCL_GET_REG_DATA(reg), reg, field))

    /* Compare field */
#define BVDC_P_SCL_COMPARE_FIELD_DATA(reg, field, data) \
    (BVDC_P_COMPARE_FIELD_DATA(BVDC_P_SCL_GET_REG_DATA(reg), reg, field, (data)))
#define BVDC_P_SCL_COMPARE_FIELD_NAME(reg, field, name) \
    (BVDC_P_COMPARE_FIELD_NAME(BVDC_P_SCL_GET_REG_DATA(reg), reg, field, name))

    /* This macro does a write into RUL (write, addr, data). 3 dwords. */
#define BVDC_P_SCL_WRITE_TO_RUL(reg, addr_ptr) \
    { \
    *addr_ptr++ = BRDC_OP_IMM_TO_REG(); \
    *addr_ptr++ = BRDC_REGISTER(BCHP##_##reg + hScaler->ulRegOffset); \
    *addr_ptr++ = BVDC_P_SCL_GET_REG_DATA(reg); \
}

    /* This macro does a block write into RUL */
#define BVDC_P_SCL_BLOCK_WRITE_TO_RUL(from, to, pulCurrent) \
    do { \
    uint32_t ulBlockSize = \
    BVDC_P_REGS_ENTRIES(from, to);\
    *pulCurrent++ = BRDC_OP_IMMS_TO_REGS( ulBlockSize ); \
    *pulCurrent++ = BRDC_REGISTER(BCHP##_##from + hScaler->ulRegOffset); \
    BKNI_Memcpy((void*)pulCurrent, \
    (void*)&(hScaler->aulRegs[BVDC_P_SCL_GET_REG_IDX(from)]), \
    ulBlockSize * sizeof(uint32_t)); \
    pulCurrent += ulBlockSize; \
    } while(0)

    /* This macro does a scaler reset RUL. */
#define BVDC_P_SCL_RESET_RUL(hScaler) \
    { \
    *pulCurrent++ = BRDC_OP_IMM_TO_REG(); \
    *pulCurrent++ = BRDC_REGISTER(hScaler->ulResetRegAddr); \
    *pulCurrent++ = hScaler->ulResetMask; \
    *pulCurrent++ = BRDC_OP_IMM_TO_REG(); \
    *pulCurrent++ = BRDC_REGISTER(hScaler->ulResetRegAddr); \
    *pulCurrent++ = 0; \
}

    /* number of registers in one block. */
#define BVDC_P_SCL_REGS_COUNT      \
    BVDC_P_REGS_ENTRIES(SCL_0_REG_START, SCL_0_REG_END)


    /* PR48032: Account for Kell Factor.  In the case of P -> I. */
#define BVDC_P_SCL_KELL_FACTOR                (70) /* 0.70 */
#define BVDC_P_APPLY_KELL_FACTOR(ulFirStep)   \
    (((ulFirStep) * 100) / (2 * BVDC_P_SCL_KELL_FACTOR))


/* Fixed point defines */
/* Normalized fixed point precision for SCL PIC OFFSET and PAN-SCAN.
   Top component is S11.14 and left component is S11.6 */
#define BVDC_P_SCL_TOP_PIC_OFFSET_F_BITS                (14)
#define BVDC_P_SCL_TOP_PIC_OFFSET_F_MASK                (0x3FFF)
#define BVDC_P_SCL_TOP_PIC_OFFSET_SIGN_I_BITS           (12)

#define BVDC_P_SCL_LEFT_PIC_OFFSET_F_BITS               (6)
#define BVDC_P_SCL_LEFT_PIC_OFFSET_F_MASK               (0x3F)
#define BVDC_P_SCL_LEFT_PIC_OFFSET_SIGN_I_BITS          (12)

#if BVDC_P_SUPPORT_SCL_VER < BVDC_P_SUPPORT_SCL_VER_4
#define BVDC_P_SCL_H_NRM_I_BITS               (5)
#else
#define BVDC_P_SCL_H_NRM_I_BITS               (8)
#endif
#define BVDC_P_SCL_H_RATIO_I_BITS             (5)
#define BVDC_P_SCL_H_RATIO_F_BITS             (26)
#define BVDC_P_SCL_MAX_H_STEP                 (0x7fffffff)

#define BVDC_P_SCL_H_R_DLT_I_BITS             (1)
#define BVDC_P_SCL_H_R_DLT_F_BITS             (26)

#define BVDC_P_SCL_PHASE_ACCU_INIT_I_BITS     (5)
#define BVDC_P_SCL_PHASE_ACCU_INIT_F_BITS     (26)

#define BVDC_P_SCL_LUMA_INIT_PHASE_I_BITS     (11)
#define BVDC_P_SCL_LUMA_INIT_PHASE_F_BITS     (6)

#define BVDC_P_SCL_CHROMA_INIT_PHASE_I_BITS   (11)
#define BVDC_P_SCL_CHROMA_INIT_PHASE_F_BITS   (6)

#define BVDC_P_SCL_V_RATIO_I_BITS             (6)
#define BVDC_P_SCL_V_INIT_PHASE_I_BITS        (11)

#if BVDC_P_SUPPORT_SCL_VER > BVDC_P_SUPPORT_SCL_VER_2 /* 3548 */
#define BVDC_P_SCL_V_RATIO_F_BITS             (17)
#define BVDC_P_SCL_V_INIT_PHASE_F_BITS        (17)
#if BVDC_P_SUPPORT_SCL_VER >= BVDC_P_SUPPORT_SCL_VER_7 /* 7422 */
#define BVDC_P_SCL_V_RATIO_F_BITS_EXT         (26 - BVDC_P_SCL_V_RATIO_F_BITS)
#define BVDC_P_SCL_V_INIT_PHASE_F_BITS_EXT    (26 - BVDC_P_SCL_V_INIT_PHASE_F_BITS)
#endif
#else
#define BVDC_P_SCL_V_RATIO_F_BITS             (14)
#define BVDC_P_SCL_V_INIT_PHASE_F_BITS        (14)
#endif

#define BVDC_P_SCL_V_PAN_SCAN_I_BITS          (1)
#define BVDC_P_SCL_V_PAN_SCAN_F_BITS          (4)

#define BVDC_P_SCL_COEFFS_I_BITS              (1)
#define BVDC_P_SCL_COEFFS_F_BITS              (10)

#define BVDC_P_SCL_ZERO_F_BITS                (0)

#define BVDC_P_SCL_MAX_NRM_H_STEP  ((1 << (BVDC_P_SCL_H_NRM_I_BITS + BVDC_P_NRM_SRC_STEP_F_BITS))   - 1)
#define BVDC_P_SCL_MAX_NRM_H_R_DLT ((1 << (BVDC_P_SCL_H_R_DLT_I_BITS + BVDC_P_NRM_SRC_STEP_F_BITS))   - 1)

#define BVDC_P_SCL_H_RATIO_ONE                (1 << BVDC_P_SCL_H_RATIO_F_BITS)
#define BVDC_P_SCL_1ST_DECIMATION_THRESHOLD   (4 << BVDC_P_SCL_H_RATIO_F_BITS)
#define BVDC_P_SCL_2ND_DECIMATION_THRESHOLD   (8 << BVDC_P_SCL_H_RATIO_F_BITS)

#define BVDC_P_SCL_H_STEP_NRM_TO_SPEC(nrm_h_step)                            \
    ((nrm_h_step) < BVDC_P_SCL_MAX_NRM_H_STEP)?                              \
    ((nrm_h_step) << (BVDC_P_SCL_H_RATIO_F_BITS - BVDC_P_NRM_SRC_STEP_F_BITS)) : \
    (BVDC_P_SCL_MAX_H_STEP)

#define BVDC_P_SCL_V_RATIO_MAX   ((1 << (BVDC_P_SCL_V_RATIO_I_BITS+BVDC_P_SCL_V_RATIO_F_BITS)) - 1)
#define BVDC_P_SCL_V_RATIO_TRUNC(step)  ((step <= BVDC_P_SCL_V_RATIO_MAX)? step: BVDC_P_SCL_V_RATIO_MAX)
#define BVDC_P_SCL_V_RATIO_ONE BVDC_P_FLOAT_TO_FIXED(1.0, BVDC_P_SCL_V_RATIO_I_BITS, BVDC_P_SCL_V_RATIO_F_BITS)
#define BVDC_P_SCL_V_RATIO_KELL_RANGE BVDC_P_FLOAT_TO_FIXED(0.66667, BVDC_P_SCL_V_RATIO_I_BITS, BVDC_P_SCL_V_RATIO_F_BITS)

    /* Set horizontal scale down ratio */
#define BVDC_P_SCL_SET_HORZ_REGION02(region, end_pixel, step_inc) \
    { \
    BVDC_P_SCL_GET_REG_DATA(SCL_0_HORIZ_FIR_DEST_PIC_REGION_##region##_STEP_DELTA) &= ~( \
    BCHP_MASK(SCL_0_HORIZ_FIR_DEST_PIC_REGION_0_STEP_DELTA, SIZE)); \
    BVDC_P_SCL_GET_REG_DATA(SCL_0_HORIZ_FIR_DEST_PIC_REGION_##region##_STEP_DELTA) |=  ( \
    BCHP_FIELD_DATA(SCL_0_HORIZ_FIR_DEST_PIC_REGION_0_STEP_DELTA, SIZE, step_inc)); \
    BVDC_P_SCL_GET_REG_DATA(SCL_0_HORIZ_DEST_PIC_REGION_##region##_END) &= ~( \
    BCHP_MASK(SCL_0_HORIZ_DEST_PIC_REGION_0_END, POSITION)); \
    BVDC_P_SCL_GET_REG_DATA(SCL_0_HORIZ_DEST_PIC_REGION_##region##_END) |=  ( \
    BCHP_FIELD_DATA(SCL_0_HORIZ_DEST_PIC_REGION_0_END, POSITION, end_pixel)); \
}

#ifdef BCHP_SCL_0_HORIZ_DEST_PIC_REGION_N1_END
#define BVDC_P_SCL_SET_HORZ_REGION3N1(region, end_pixel)  \
    BVDC_P_SCL_GET_REG_DATA(SCL_0_HORIZ_DEST_PIC_REGION_##region##_END) &= ~( \
    BCHP_MASK(SCL_0_HORIZ_DEST_PIC_REGION_0_END, POSITION)); \
    BVDC_P_SCL_GET_REG_DATA(SCL_0_HORIZ_DEST_PIC_REGION_##region##_END) |=  ( \
    BCHP_FIELD_DATA(SCL_0_HORIZ_DEST_PIC_REGION_0_END, POSITION, end_pixel));
#endif

#define BVDC_P_SCL_SET_HORZ_RGN1(end_pixel_1) \
    { \
    BVDC_P_SCL_GET_REG_DATA(SCL_0_HORIZ_DEST_PIC_REGION_1_END) &= ~( \
    BCHP_MASK(SCL_0_HORIZ_DEST_PIC_REGION_1_END, POSITION)); \
    BVDC_P_SCL_GET_REG_DATA(SCL_0_HORIZ_DEST_PIC_REGION_1_END) |=  ( \
    BCHP_FIELD_DATA(SCL_0_HORIZ_DEST_PIC_REGION_1_END, POSITION, end_pixel_1)); \
}

#if BVDC_P_SUPPORT_SCL_VER < BVDC_P_SUPPORT_SCL_VER_4
#define BVDC_P_SCL_SET_HORZ_RGN1_STEP(step_int_size, step_frac_size)\
    { \
    BVDC_P_SCL_GET_REG_DATA(SCL_0_HORIZ_FIR_INIT_STEP) &= ~( \
    BCHP_MASK(SCL_0_HORIZ_FIR_INIT_STEP, SIZE)); \
    BVDC_P_SCL_GET_REG_DATA(SCL_0_HORIZ_FIR_INIT_STEP) |=  ( \
    BCHP_FIELD_DATA(SCL_0_HORIZ_FIR_INIT_STEP, SIZE, \
    step_int_size & BCHP_SCL_0_HORIZ_FIR_INIT_STEP_SIZE_MASK)); \
}
#define BVDC_P_SCL_SET_HORZ_RATIO(step) \
    { \
    BVDC_P_SCL_GET_REG_DATA(SCL_0_HORIZ_FIR_INIT_STEP) &= ~( \
    BCHP_MASK(SCL_0_HORIZ_FIR_INIT_STEP, SIZE)); \
    BVDC_P_SCL_GET_REG_DATA(SCL_0_HORIZ_FIR_INIT_STEP) |=  ( \
    BCHP_FIELD_DATA(SCL_0_HORIZ_FIR_INIT_STEP, SIZE, step)); \
}
#else
#define BVDC_P_SCL_SET_HORZ_RGN1_STEP(step_int_size, step_frac_size)\
    { \
    BVDC_P_SCL_GET_REG_DATA(SCL_0_HORIZ_FIR_INIT_STEP_FRAC) &= ~( \
    BCHP_MASK(SCL_0_HORIZ_FIR_INIT_STEP_FRAC, FRAC_SIZE)); \
    BVDC_P_SCL_GET_REG_DATA(SCL_0_HORIZ_FIR_INIT_STEP_FRAC) |=  ( \
    BCHP_FIELD_DATA(SCL_0_HORIZ_FIR_INIT_STEP_FRAC, FRAC_SIZE, \
    ((step_frac_size & ((1 << BVDC_P_SCL_H_RATIO_F_BITS)-1))))); \
    BVDC_P_SCL_GET_REG_DATA(SCL_0_HORIZ_FIR_INIT_STEP_INT) &= ~( \
    BCHP_MASK(SCL_0_HORIZ_FIR_INIT_STEP_INT, INT_SIZE)); \
    BVDC_P_SCL_GET_REG_DATA(SCL_0_HORIZ_FIR_INIT_STEP_INT) |=  ( \
    BCHP_FIELD_DATA(SCL_0_HORIZ_FIR_INIT_STEP_INT, INT_SIZE, \
    step_int_size & BCHP_SCL_0_HORIZ_FIR_INIT_STEP_INT_INT_SIZE_MASK)); \
}
#define BVDC_P_SCL_SET_HORZ_RATIO(step) \
    { \
    BVDC_P_SCL_GET_REG_DATA(SCL_0_HORIZ_FIR_INIT_STEP_FRAC) &= ~( \
    BCHP_MASK(SCL_0_HORIZ_FIR_INIT_STEP_FRAC, FRAC_SIZE)); \
    BVDC_P_SCL_GET_REG_DATA(SCL_0_HORIZ_FIR_INIT_STEP_FRAC) |=  ( \
    BCHP_FIELD_DATA(SCL_0_HORIZ_FIR_INIT_STEP_FRAC, FRAC_SIZE, \
    ((step & ((1 << BVDC_P_SCL_H_RATIO_F_BITS)-1))))); \
    BVDC_P_SCL_GET_REG_DATA(SCL_0_HORIZ_FIR_INIT_STEP_INT) &= ~( \
    BCHP_MASK(SCL_0_HORIZ_FIR_INIT_STEP_INT, INT_SIZE)); \
    BVDC_P_SCL_GET_REG_DATA(SCL_0_HORIZ_FIR_INIT_STEP_INT) |=  ( \
    BCHP_FIELD_DATA(SCL_0_HORIZ_FIR_INIT_STEP_INT, INT_SIZE, \
    step >> BVDC_P_SCL_H_RATIO_F_BITS)); \
}
#endif

#define BVDC_P_SCL_SET_HORZ_STEP_MISC(end_pixel_1, step_init) \
    { \
    BVDC_P_SCL_SET_HORZ_RATIO(step_init); \
    BVDC_P_SCL_GET_REG_DATA(SCL_0_HORIZ_DEST_PIC_REGION_1_END) &= ~( \
    BCHP_MASK(SCL_0_HORIZ_DEST_PIC_REGION_1_END, POSITION)); \
    BVDC_P_SCL_GET_REG_DATA(SCL_0_HORIZ_DEST_PIC_REGION_1_END) |=  ( \
    BCHP_FIELD_DATA(SCL_0_HORIZ_DEST_PIC_REGION_1_END, POSITION, end_pixel_1)); \
}

/* Set vertical scale down ratio */
#if (BVDC_P_SUPPORT_SCL_VER <= BVDC_P_SUPPORT_SCL_VER_6)
#define BVDC_P_SCL_SET_VERT_RATIO(ratio) \
{ \
    BVDC_P_SCL_GET_REG_DATA(SCL_0_VERT_FIR_SRC_PIC_STEP) &= ~( \
    BCHP_MASK(SCL_0_VERT_FIR_SRC_PIC_STEP, SIZE)); \
    BVDC_P_SCL_GET_REG_DATA(SCL_0_VERT_FIR_SRC_PIC_STEP) |=  ( \
    BCHP_FIELD_DATA(SCL_0_VERT_FIR_SRC_PIC_STEP, SIZE, ratio)); \
}
#else /* TODO: improve fixed point math to support higher precision SCL */
#define BVDC_P_SCL_SET_VERT_RATIO(ratio) \
    { \
    BVDC_P_SCL_GET_REG_DATA(SCL_0_VERT_FIR_INIT_PIC_STEP) &= ~( \
    BCHP_MASK(SCL_0_VERT_FIR_INIT_PIC_STEP, SIZE)); \
    BVDC_P_SCL_GET_REG_DATA(SCL_0_VERT_FIR_INIT_PIC_STEP) |=  ( \
    BCHP_FIELD_DATA(SCL_0_VERT_FIR_INIT_PIC_STEP, SIZE, (ratio << BVDC_P_SCL_V_RATIO_F_BITS_EXT))); \
}
#endif

    /* Initial phase values. */
#define BVDC_P_H_INIT_PHASE_0_POINT_5   BVDC_P_FLOAT_TO_FIXED(0.5, \
    BVDC_P_SCL_PHASE_ACCU_INIT_I_BITS, BVDC_P_SCL_PHASE_ACCU_INIT_F_BITS)
#define BVDC_P_H_INIT_PHASE_RATIO_ADJ(hsr) \
    (hsr >> (BVDC_P_SCL_H_RATIO_F_BITS - BVDC_P_SCL_PHASE_ACCU_INIT_F_BITS))
#define BVDC_P_V_INIT_PHASE_0_POINT_5   BVDC_P_FLOAT_TO_FIXED(0.5, \
    BVDC_P_SCL_V_INIT_PHASE_I_BITS, BVDC_P_SCL_V_INIT_PHASE_F_BITS)
#define BVDC_P_V_INIT_PHASE_1_POINT_0   BVDC_P_FLOAT_TO_FIXED(1.0, \
    BVDC_P_SCL_V_INIT_PHASE_I_BITS, BVDC_P_SCL_V_INIT_PHASE_F_BITS)
#define BVDC_P_V_INIT_PHASE_2_POINT_0   BVDC_P_FLOAT_TO_FIXED(2.0, \
    BVDC_P_SCL_V_INIT_PHASE_I_BITS, BVDC_P_SCL_V_INIT_PHASE_F_BITS)
#define BVDC_P_V_INIT_PHASE_0_POINT_25  BVDC_P_FLOAT_TO_FIXED(0.25, \
    BVDC_P_SCL_V_INIT_PHASE_I_BITS, BVDC_P_SCL_V_INIT_PHASE_F_BITS)

#define BVDC_P_V_INIT_PHASE_RATIO_ADJ(vsr) \
    (vsr >> (BVDC_P_SCL_V_RATIO_F_BITS - BVDC_P_SCL_V_INIT_PHASE_F_BITS))

/* Handy macros to build the table. */
#define BVDC_P_CT_UNUSED            (0)
#define BVDC_P_CT_P2I               (1)
#define BVDC_P_CT_2_TAP             (2)
#define BVDC_P_CT_4_TAP             (4)
#define BVDC_P_CT_6_TAP             (6)
#define BVDC_P_CT_8_TAP             (8)
#define BVDC_P_CT_12_TAP            (12)
#define BVDC_P_CT_16_TAP            (16)

/* horizontal scaling coeff table macroes */
#define BVDC_P_CT_H_RATIO_GENERIC(index, input, raster, operation, ratio, tap, table) \
{                                                                             \
    (index),                                                                  \
    BVDC_P_CtInput_e##input,                                                  \
    BVDC_P_CtOutput_eAny,                                                     \
    BVDC_P_CtRaster_e##raster,                                                \
    (operation),                                                              \
    BVDC_P_FLOAT_TO_FIXED(                                                    \
        (ratio), BVDC_P_SCL_H_RATIO_I_BITS, BVDC_P_SCL_H_RATIO_F_BITS),       \
    (tap),                                                                    \
    BVDC_P_CT_UNUSED,                                                         \
    BVDC_P_CT_UNUSED,                                                         \
    BVDC_P_CT_UNUSED,                                                         \
    (table),                                                                  \
    BDBG_STRING_INLINE(#table)                                                \
}

#define BVDC_P_CT_H_SIZE_INPUT(index, input, raster, srcsize, dstsize, delta, tap, table) \
    {                                                                             \
    (index),                                                                  \
    BVDC_P_CtInput_e##input,                                                  \
    BVDC_P_CtOutput_eAny,                                                     \
    BVDC_P_CtRaster_e##raster,                                                \
    BVDC_P_CtLutOp_eEqual,                                                    \
    BVDC_P_CT_UNUSED,                                                         \
    (tap),                                                                    \
    (srcsize),                                                                \
    (dstsize),                                                                \
    (delta),                                                                  \
    (table),                                                                  \
    BDBG_STRING_INLINE(#table)                                                       \
}

#define BVDC_P_CT_H_SIZE(srcsize, dstsize, delta, tap, table) \
    BVDC_P_CT_H_SIZE_INPUT(0, Any, Any, srcsize, dstsize, delta, tap, table)

#define BVDC_P_CT_H_RATIO_EQ(ratio, tap, table) \
    BVDC_P_CT_H_RATIO_GENERIC(0, Any, Any, BVDC_P_CtLutOp_eEqual, ratio, tap, table)

#define BVDC_P_CT_H_RATIO_GT(ratio, tap, table) \
    BVDC_P_CT_H_RATIO_GENERIC(0, Any, Any, BVDC_P_CtLutOp_eGreaterThan, ratio, tap, table)

#define BVDC_P_CT_H_RATIO_LT(ratio, tap, table) \
    BVDC_P_CT_H_RATIO_GENERIC(0, Any, Any, BVDC_P_CtLutOp_eLessThan, ratio, tap, table)

#define BVDC_P_CT_H_RATIO_GT_EQ(ratio, tap, table) \
    BVDC_P_CT_H_RATIO_GENERIC(0, Any, Any, BVDC_P_CtLutOp_eGreaterThanEqual, ratio, tap, table)

#define BVDC_P_CT_H_RATIO_LT_EQ(ratio, tap, table) \
    BVDC_P_CT_H_RATIO_GENERIC(0, Any, Any, BVDC_P_CtLutOp_eLessThanEqual, ratio, tap, table)


#define BVDC_P_CT_H_RATIO_EQ_INPUT(index, input, raster, ratio, tap, table) \
    BVDC_P_CT_H_RATIO_GENERIC(index, input, raster, BVDC_P_CtLutOp_eEqual, ratio, tap, table)

#define BVDC_P_CT_H_RATIO_GT_INPUT(index, input, raster, ratio, tap, table) \
    BVDC_P_CT_H_RATIO_GENERIC(index, input, raster, BVDC_P_CtLutOp_eGreaterThan, ratio, tap, table)

#define BVDC_P_CT_H_RATIO_LT_INPUT(index, input, raster, ratio, tap, table) \
    BVDC_P_CT_H_RATIO_GENERIC(index, input, raster, BVDC_P_CtLutOp_eLessThan, ratio, tap, table)

#define BVDC_P_CT_H_RATIO_GT_EQ_INPUT(index, input, raster, ratio, tap, table) \
    BVDC_P_CT_H_RATIO_GENERIC(index, input, raster, BVDC_P_CtLutOp_eGreaterThanEqual, ratio, tap, table)

#define BVDC_P_CT_H_RATIO_LT_EQ_INPUT(index, input, raster, ratio, tap, table) \
    BVDC_P_CT_H_RATIO_GENERIC(index, input, raster, BVDC_P_CtLutOp_eLessThanEqual, ratio, tap, table)


    /* vertical scaling coeff table macroes */
#define BVDC_P_CT_V_RATIO_GENERIC(index, input, output, raster, operation, ratio, tap, table) \
{                                                                             \
    (index),                                                                  \
    BVDC_P_CtInput_e##input,                                                  \
    BVDC_P_CtOutput_e##output,                                                \
    BVDC_P_CtRaster_e##raster,                                                \
    (operation),                                                              \
    BVDC_P_FLOAT_TO_FIXED((ratio), BVDC_P_SCL_V_RATIO_I_BITS, BVDC_P_SCL_V_RATIO_F_BITS), \
    (tap),                                                                    \
    BVDC_P_CT_UNUSED,                                                         \
    BVDC_P_CT_UNUSED,                                                         \
    BVDC_P_CT_UNUSED,                                                         \
    (table),                                                                  \
    BDBG_STRING_INLINE(#table)                                                       \
}

#define BVDC_P_CT_V_SIZE_INPUT(index, input, raster, srcsize, dstsize, delta, tap, table) \
{                                                                             \
    (index),                                                                  \
    BVDC_P_CtInput_e##input,                                                  \
    BVDC_P_CtOutput_eAny,                                                     \
    BVDC_P_CtRaster_e##raster,                                                \
    BVDC_P_CtLutOp_eEqual,                                                    \
    BVDC_P_CT_UNUSED,                                                         \
    (tap),                                                                    \
    (srcsize),                                                                \
    (dstsize),                                                                \
    (delta),                                                                  \
    (table),                                                                  \
    BDBG_STRING_INLINE(#table)                                                       \
}

#define BVDC_P_CT_V_SIZE(srcsize, dstsize, delta, tap, table) \
    BVDC_P_CT_V_SIZE_INPUT(0, Any, Any, srcsize, dstsize, delta, tap, table)

#define BVDC_P_CT_V_RATIO_EQ(ratio, tap, table) \
    BVDC_P_CT_V_RATIO_GENERIC(0, Any, Any, Any, BVDC_P_CtLutOp_eEqual, ratio, tap, table)

#define BVDC_P_CT_V_RATIO_GT(ratio, tap, table) \
    BVDC_P_CT_V_RATIO_GENERIC(0, Any, Any, Any, BVDC_P_CtLutOp_eGreaterThan, ratio, tap,  table)

#define BVDC_P_CT_V_RATIO_LT(ratio, tap, table) \
    BVDC_P_CT_V_RATIO_GENERIC(0, Any, Any, Any, BVDC_P_CtLutOp_eLessThan, ratio, tap, table)

#define BVDC_P_CT_V_RATIO_GT_EQ(ratio, tap, table) \
    BVDC_P_CT_V_RATIO_GENERIC(0, Any, Any, Any, BVDC_P_CtLutOp_eGreaterThanEqual, ratio, tap, table)

#define BVDC_P_CT_V_RATIO_LT_EQ(ratio, tap, table) \
    BVDC_P_CT_V_RATIO_GENERIC(0, Any, Any, Any, BVDC_P_CtLutOp_eLessThanEqual, ratio, tap, table)

/* output is regular display macro definition */
#define BVDC_P_CT_V_RATIO_EQ_INPUT(index, input, raster, ratio, tap, table) \
    BVDC_P_CT_V_RATIO_GENERIC(index, input, Any, raster, BVDC_P_CtLutOp_eEqual, ratio, tap, table)

#define BVDC_P_CT_V_RATIO_GT_INPUT(index, input, raster, ratio, tap, table) \
    BVDC_P_CT_V_RATIO_GENERIC(index, input, Any, raster, BVDC_P_CtLutOp_eGreaterThan, ratio, tap, table)

#define BVDC_P_CT_V_RATIO_LT_INPUT(index, input, raster, ratio, tap, table) \
    BVDC_P_CT_V_RATIO_GENERIC(index, input, Any, raster, BVDC_P_CtLutOp_eLessThan, ratio, tap, table)

#define BVDC_P_CT_V_RATIO_GT_EQ_INPUT(index, input, raster, ratio, tap, table) \
    BVDC_P_CT_V_RATIO_GENERIC(index, input, Any, raster, BVDC_P_CtLutOp_eGreaterThanEqual, ratio, tap, table)

#define BVDC_P_CT_V_RATIO_LT_EQ_INPUT(index, input, raster, ratio, tap, table) \
    BVDC_P_CT_V_RATIO_GENERIC(index, input, Any, raster, BVDC_P_CtLutOp_eLessThanEqual, ratio, tap, table)

/* input independent and output dependent macro definition */
#define BVDC_P_CT_V_RATIO_EQ_OUTPUT(index, output, raster, ratio, tap, table) \
        BVDC_P_CT_V_RATIO_GENERIC(index, Any, output, raster, BVDC_P_CtLutOp_eEqual, ratio, tap, table)

#define BVDC_P_CT_V_RATIO_GT_OUTPUT(index, output, raster, ratio, tap, table) \
        BVDC_P_CT_V_RATIO_GENERIC(index, Any, output, raster, BVDC_P_CtLutOp_eGreaterThan, ratio, tap, table)

#define BVDC_P_CT_V_RATIO_LT_OUTPUT(index, output, raster, ratio, tap, table) \
        BVDC_P_CT_V_RATIO_GENERIC(index, Any, output, raster, BVDC_P_CtLutOp_eLessThan, ratio, tap, table)

#define BVDC_P_CT_V_RATIO_GT_EQ_OUTPUT(index, output, raster, ratio, tap, table) \
        BVDC_P_CT_V_RATIO_GENERIC(index, Any, output, raster, BVDC_P_CtLutOp_eGreaterThanEqual, ratio, tap, table)

#define BVDC_P_CT_V_RATIO_LT_EQ_OUTPUT(index, output, raster, ratio, tap, table) \
        BVDC_P_CT_V_RATIO_GENERIC(index, Any, output, raster, BVDC_P_CtLutOp_eLessThanEqual, ratio, tap, table)

    /* User selectable */
#define BVDC_P_CT_USER_SELECTABLE(index, tap, table) \
    {(index), BVDC_P_CtInput_eAny, BVDC_P_CtOutput_eAny, BVDC_P_CtRaster_eAny, BVDC_P_CtLutOp_eUserSelectable, \
    BVDC_P_CT_UNUSED, (tap), BVDC_P_CT_UNUSED, \
    BVDC_P_CT_UNUSED, BVDC_P_CT_UNUSED, (table), BDBG_STRING_INLINE(#table)}

#define BVDC_P_CT_ALWAYS(table) \
    {0, BVDC_P_CtInput_eAny, BVDC_P_CtOutput_eAny, BVDC_P_CtRaster_eAny, BVDC_P_CtLutOp_eAlways,  \
    BVDC_P_CT_UNUSED, BVDC_P_CT_UNUSED, BVDC_P_CT_UNUSED, \
    BVDC_P_CT_UNUSED, BVDC_P_CT_UNUSED, (table), BDBG_STRING_INLINE(#table)}

#define BVDC_P_CT_ALWAYS_TAP(tap, table) \
    {0, BVDC_P_CtInput_eAny, BVDC_P_CtOutput_eAny, BVDC_P_CtRaster_eAny, BVDC_P_CtLutOp_eAlways, \
    BVDC_P_CT_UNUSED, (tap), BVDC_P_CT_UNUSED, \
    BVDC_P_CT_UNUSED, BVDC_P_CT_UNUSED, (table), BDBG_STRING_INLINE(#table)}

#define BVDC_P_CT_END \
    {0, BVDC_P_CtInput_eAny, BVDC_P_CtOutput_eAny, BVDC_P_CtRaster_eAny, BVDC_P_CtLutOp_eLast,    \
    BVDC_P_CT_UNUSED, BVDC_P_CT_UNUSED, BVDC_P_CT_UNUSED, \
    BVDC_P_CT_UNUSED, BVDC_P_CT_UNUSED, NULL, "NULL" }

    /* Miscellaneous constants */
#define BVDC_P_SCL_HORZ_REGIONS_COUNT         (9)
#define BVDC_P_SCL_VERT_IPC_COUNT             (20)

#define BVDC_P_SCL_VERT_FIR_TAP_COUNT         (4)
#define BVDC_P_SCL_VERT_FIR_PHASE_COUNT       (8)

#define BVDC_P_SCL_HORZ_FIR_TAP_COUNT         (8)
#define BVDC_P_SCL_HORZ_FIR_PHASE_COUNT       (8)

#define BVDC_P_SCL_FIR_TAP_COUNT_MAX          (8)
#define BVDC_P_SCL_FIR_PHASE_COUNT_MAX        (8)

#define BVDC_P_SCL_MAX_BLK_AVG                (31)

#if BVDC_P_SUPPORT_SCL_VER >= BVDC_P_SUPPORT_SCL_VER_4
#define BVDC_P_SCL_HD_SCL_LINE_BUFFER         (1920)
#else
#define BVDC_P_SCL_HD_SCL_LINE_BUFFER         (1280)
#endif
#define BVDC_P_SCL_SD_SCL_LINE_BUFFER         (720)

#define BVDC_P_SCL_HORZ_HWF_FACTOR            (2)

#define BVDC_P_SCL_HORZ_1_FIXED BVDC_P_FLOAT_TO_FIXED(1.000, \
    BVDC_P_SCL_LUMA_INIT_PHASE_I_BITS, BVDC_P_SCL_LUMA_INIT_PHASE_F_BITS)

#define BVDC_P_SCL_VERT_1_FIXED BVDC_P_FLOAT_TO_FIXED(1.000, \
    BVDC_P_SCL_V_INIT_PHASE_I_BITS, BVDC_P_SCL_V_INIT_PHASE_F_BITS)

#define BVDC_P_SCL_DEJAGGING_ON_THESHOLD BVDC_P_FLOAT_TO_FIXED(0.666, \
    BVDC_P_SCL_V_RATIO_I_BITS, BVDC_P_SCL_V_RATIO_F_BITS)

#define BVDC_P_SCL_8TAP_BLK_AVG_VERT_THRESHOLD BVDC_P_FLOAT_TO_FIXED(8, \
    BVDC_P_SCL_V_RATIO_I_BITS, BVDC_P_SCL_V_RATIO_F_BITS)

#define BVDC_P_SCL_6TAP_BLK_AVG_VERT_THRESHOLD BVDC_P_FLOAT_TO_FIXED(6, \
    BVDC_P_SCL_V_RATIO_I_BITS, BVDC_P_SCL_V_RATIO_F_BITS)

#define BVDC_P_SCL_4TAP_BLK_AVG_VERT_THRESHOLD BVDC_P_FLOAT_TO_FIXED(4, \
    BVDC_P_SCL_V_RATIO_I_BITS, BVDC_P_SCL_V_RATIO_F_BITS)

#define BVDC_P_SCL_2TAP_BLK_AVG_VERT_THRESHOLD BVDC_P_FLOAT_TO_FIXED(2, \
    BVDC_P_SCL_V_RATIO_I_BITS, BVDC_P_SCL_V_RATIO_F_BITS)

#define BVDC_P_SCL_FIR_COEFFS_MAX \
    (BVDC_P_SCL_FIR_TAP_COUNT_MAX * BVDC_P_SCL_FIR_PHASE_COUNT_MAX)

#if BVDC_P_SCL_V_STEP_SIZE_WORKAROUND
#define BVDC_P_CAL_VRT_SRC_STEP_BASE(InH, OutH, blk, f_bits) \
    BVDC_P_DIV_ROUND_UP(((InH + blk)/(blk+1) << f_bits), \
        BVDC_P_MAX((OutH) - ((InH) != (OutH) ? 2 : 0), 1))
#else
#define BVDC_P_CAL_VRT_SRC_STEP_BASE(InH, OutH, blk, f_bits) \
    BVDC_P_DIV_ROUND_UP(((InH + blk)/(blk+1) << f_bits), \
        BVDC_P_MAX(OutH, 1))
#endif

#define BVDC_P_CAL_BLK_VRT_SRC_STEP(InH, OutH, blk) \
    BVDC_P_CAL_VRT_SRC_STEP_BASE(InH, OutH, blk, BVDC_P_SCL_V_RATIO_F_BITS)

/* Rounding down OutH to even number when OutH is odd and output format is interlaced to avoid vstep accuracy loss */
#define BVDC_P_CAL_VRT_SRC_STEP(InH, OutH, bInterlaced) \
    BVDC_P_CAL_VRT_SRC_STEP_BASE(InH, (OutH -((bInterlaced) && (OutH &1))), 0, BVDC_P_NRM_SRC_STEP_F_BITS)

#define BVDC_P_SCL_LAST UINT32_C(-1)

#define BVDC_P_Scaler_MuxAddr(hScl)       (BCHP_VNET_F_SCL_0_SRC + (hScl)->eId * sizeof(uint32_t))
#define BVDC_P_Scaler_PostMuxValue(hScl)  \
(((hScl)->eId == BVDC_P_ScalerId_eScl4) ? BCHP_VNET_B_CAP_0_SRC_SOURCE_Scaler_4 : \
(BCHP_VNET_B_CAP_0_SRC_SOURCE_Scaler_0 + (hScl)->eId))
#define BVDC_P_Scaler_SetVnet_isr(hScl, ulSrcMuxValue, eVnetPatchMode) \
    BVDC_P_SubRul_SetVnet_isr(&((hScl)->SubRul), ulSrcMuxValue, eVnetPatchMode)
#define BVDC_P_Scaler_UnsetVnet_isr(hScl) \
    BVDC_P_SubRul_UnsetVnet_isr(&((hScl)->SubRul))

/***************************************************************************
* Scaler private data structures
***************************************************************************/
typedef enum
{
    BVDC_P_CtLutOp_eEqual = 0,
    BVDC_P_CtLutOp_eGreaterThan,
    BVDC_P_CtLutOp_eGreaterThanEqual,
    BVDC_P_CtLutOp_eLessThan,
    BVDC_P_CtLutOp_eLessThanEqual,
    BVDC_P_CtLutOp_eUserSelectable,
    BVDC_P_CtLutOp_eAlways,
    BVDC_P_CtLutOp_eLast
} BVDC_P_CtLut;

typedef enum
{
    BVDC_P_CtInput_eIfd = 0,
    BVDC_P_CtInput_eCvbs,
    BVDC_P_CtInput_eSvideo,
    BVDC_P_CtInput_eYPrPb,
    BVDC_P_CtInput_ePcIn,
    BVDC_P_CtInput_eScart,
    BVDC_P_CtInput_eHdDvi,
    BVDC_P_CtInput_eMpeg,
    BVDC_P_CtInput_eItu656,
    BVDC_P_CtInput_eAny,
    BVDC_P_CtInput_eUnknown
} BVDC_P_CtInput;

typedef enum
{
    BVDC_P_CtOutput_eAny = 0,
    BVDC_P_CtOutput_eDisp,
    BVDC_P_CtOutput_eStg,
    BVDC_P_CtOutput_eUnknown
} BVDC_P_CtOutput;

typedef enum
{
    BVDC_P_CtRaster_ePro = 0,
    BVDC_P_CtRaster_eInt,
    BVDC_P_CtRaster_eAny,
    BVDC_P_CtRaster_eUnknown
} BVDC_P_CtRaster;

/* Vert/Horz FIR coefficients. */
typedef struct
{
    uint32_t               ulCtIndex;
    BVDC_P_CtInput         eCtInputType;
    BVDC_P_CtOutput        eCtOutputType;
    BVDC_P_CtRaster        eCtRaster;
    BVDC_P_CtLut           eCtLutOp;
    uint32_t               ulDownRatio; /* ratio */
    uint32_t               ulTapMode;   /* 2-tap, 4-tap, 8-tap */
    uint32_t               ulSrcSize;
    uint32_t               ulDstSize;
    uint32_t               ulDelta;
    const uint32_t        *pulCoeffs;
    const char            *pchName;
} BVDC_P_FirCoeffTbl;

typedef struct BVDC_P_ScalerContext
{
    BDBG_OBJECT(BVDC_SCL)

    /* flag initial state, requires reset; */
    bool                           bInitial;
    uint32_t                       ulResetRegAddr;
    uint32_t                       ulResetMask;
    uint32_t                       ulVnetResetAddr;
    uint32_t                       ulVnetResetMask;

    /* Scaler internal line buffers! */
    uint32_t                       ulVertLineDepth;
    uint32_t                       ulVertBlkAvgThreshold;

    /* Fir coeff tables (root) */
    const BVDC_P_FirCoeffTbl      *pHorzFirCoeffTbl;
    const BVDC_P_FirCoeffTbl      *pVertFirCoeffTbl;
    const BVDC_P_FirCoeffTbl      *pChromaHorzFirCoeffTbl;
    const BVDC_P_FirCoeffTbl      *pChromaVertFirCoeffTbl;

    /* vert coeff */
    const BVDC_P_FirCoeffTbl      *pPrevVertFirCoeff;  /* coeff set into shadow reg */
    const BVDC_P_FirCoeffTbl      *pVertFirCoeff;  /* regular use */
    const BVDC_P_FirCoeffTbl      *pPrevChromaVertFirCoeff;  /* regular use */
    const BVDC_P_FirCoeffTbl      *pChromaVertFirCoeff;  /* regular use */
    /* forced smooth coeff for frac init phase  */
    const BVDC_P_FirCoeffTbl      *pFracInitPhaseVertFirCoeff;
    const BVDC_P_FirCoeffTbl      *pChromaFracInitPhaseVertFirCoeff;

    /* Pre-computed vertical initial phase for all fields from->to. */
    uint32_t                       aaulInitPhase[BVDC_P_MAX_POLARITY][BVDC_P_MAX_POLARITY];
    uint32_t                       aulBlkAvgSize[BVDC_P_MAX_POLARITY][BVDC_P_MAX_POLARITY];
    uint32_t                       ulUpdateAll;
    uint32_t                       ulUpdateCoeff;
    int32_t                        lPrevHPanScan;
    int32_t                        lPrevVPanScan;

    /* store previous scaler info */
    BVDC_P_Rect                    stPrevSclCut;
    BVDC_P_Rect                    stPrevSclOut;
    BVDC_P_Rect                    stPrevSclIn;
    uint32_t                       ulPrevNonlinearSrcWidth;
    uint32_t                       ulPrevNonlinearSclOutWidth;
    BVDC_CoefficientIndex          stCtIndex;
    BVDC_Scaler_Settings           stSclSettings;
    BAVC_Polarity                  ePrevSrcPolarity;
    BAVC_Polarity                  ePrevDstPolarity;
    BFMT_Orientation               ePrevSrcOrientation;
    BFMT_Orientation               ePrevOrigSrcOrientation;
    BFMT_Orientation               ePrevDispOrientation;
    BVDC_SplitScreenMode           ePrevDeJagging;
    BVDC_SplitScreenMode           ePrevDeRinging;
    BVDC_P_CtInput                 ePrevCtInputType;
    bool                           bHandleFldInv;

    /* Back pointer to window handle for  PEP to build rul */
    BVDC_Window_Handle             hWindow;
    BREG_Handle                    hReg;

    /* private fields. */
    BVDC_P_ScalerId                eId;
    uint32_t                       ulRegOffset; /* SCL_0, SCL_1, and etc. */
    uint32_t                       aulRegs[BVDC_P_SCL_REGS_COUNT];

    /* sub-struct to manage vnet and rul build opreations */
    BVDC_P_SubRulContext           SubRul;

    /* 422 to 444 up sampler setting */
    BVDC_422To444UpSampler         stUpSampler;
    /* 444 to 422 down sampler setting */
    BVDC_444To422DnSampler         stDnSampler;

    uint32_t                       ulHorzTaps;
    uint32_t                       ulVertTaps;

    bool                           bDeJagging;
    bool                           bDeRinging;

    bool                           bIs10BitCore;
    bool                           bIs2xClk;

    /* src width alignment: 1 for 444 format internally, 2 for 422 format internally */
    uint32_t                       ulSrcHrzAlign;

} BVDC_P_ScalerContext;


/***************************************************************************
* Scaler private functions
***************************************************************************/
BERR_Code BVDC_P_Scaler_Create
    ( BVDC_P_Scaler_Handle            *phScaler,
      BVDC_P_ScalerId                  eScalerId,
      BVDC_P_Resource_Handle           hResource,
      BREG_Handle                      hReg );

void BVDC_P_Scaler_Destroy
    ( BVDC_P_Scaler_Handle             hScaler );

void BVDC_P_Scaler_Init
    ( BVDC_P_Scaler_Handle             hScaler );

void BVDC_P_Scaler_Init_isr
    ( BVDC_P_Scaler_Handle             hScaler,
      BVDC_Window_Handle               hWindow );

void BVDC_P_Scaler_BuildRul_isr
    ( const BVDC_P_Scaler_Handle       hScaler,
      BVDC_P_ListInfo                 *pList,
      BVDC_P_State                     eVnetState,
      BVDC_P_PicComRulInfo            *pPicComRulInfo );

const BVDC_P_FirCoeffTbl* BVDC_P_SelectFirCoeff_isr
    ( const BVDC_P_FirCoeffTbl         *pFirstCoeffEntry,
      uint32_t                          ulCtIndex,
      BVDC_P_CtInput                    eCtInputType,
      BVDC_P_CtOutput                   eCtOutputType,
      BAVC_Polarity                     ePolarity,
      uint32_t                          ulDownRatio,
      uint32_t                          ulTapMode,
      uint32_t                          ulSrcSize,
      uint32_t                          ulDstSize );

void BVDC_P_Scaler_SetInfo_isr
    ( BVDC_P_Scaler_Handle             hScaler,
      const BVDC_P_PictureNodePtr      pPicture );

void BVDC_P_Scaler_SetEnable_isr
    ( BVDC_P_Scaler_Handle             hScaler,
      bool                             bEnable );

void BVDC_P_GetFirCoeffs_isr
    ( const BVDC_P_FirCoeffTbl       **ppHorzFirCoeffTbl,
    const BVDC_P_FirCoeffTbl       **ppVertFirCoeffTbl );

void BVDC_P_GetChromaFirCoeffs_isr
    ( const BVDC_P_FirCoeffTbl       **ppHorzFirCoeffTbl,
      const BVDC_P_FirCoeffTbl       **ppVertFirCoeffTbl );

void BVDC_P_Scaler_3ZoneNonLinear
    ( BVDC_P_Scaler_Handle             hScaler,
      const BVDC_P_PictureNodePtr      pPicture,
      uint32_t                        *pFirHrzStep,
      uint32_t                        *pHrzStep,
      uint32_t                        *pFirHrzStepInit );

void BVDC_P_Scaler_5ZoneNonLinear_isr
    ( BVDC_P_Scaler_Handle             hScaler,
      const BVDC_P_PictureNodePtr      pPicture,
      uint32_t                        *pFirHrzStep,
      uint32_t                        *pHrzStep,
      uint32_t                        *pFirHrzStepInit);

bool BVDC_P_Scaler_Validate_VertDepth_isr
    (BVDC_Window_Handle                 hWindow,
     const BVDC_P_Scaler_Handle         hScaler);
#ifdef __cplusplus
}
#endif

#endif /* #ifndef BVDC_SCALER_PRIV_H__ */
/* End of file. */
