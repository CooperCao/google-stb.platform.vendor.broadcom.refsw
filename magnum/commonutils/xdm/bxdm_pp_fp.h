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
 * [File Description:]
 *
 ***************************************************************************/

#ifndef bxdm_pp_FP_H__
#define bxdm_pp_FP_H__

#ifdef __cplusplus
extern "C" {
#endif

#if 0
}
#endif

#define BXDM_PictureProvider_P_FixedPoint_FractionalOverflow 0x10000L
#define BXDM_PictureProvider_P_FixedPoint_FractionalShift 16
#define BXDM_PictureProvider_P_FixedPoint_FractionalMask 0xFFFF

/*
 * These constants are used to extract the digits to the right of the decimal point.
 */
#define BXDM_PictureProvider_P_FixedPoint_FractionalMsbMask 0x8000
#define BXDM_PictureProvider_P_FixedPoint_FractionalBitsOfPrecision 16

/* Fixed Point Artitmetic */
typedef struct BXDM_PPFP_P_DataType
{
      uint32_t uiWhole;
      uint32_t uiFractional;
} BXDM_PPFP_P_DataType;

void BXDM_PPFP_P_FixPtAdd_isrsafe(
   const BXDM_PPFP_P_DataType *pstOperandA,
   const BXDM_PPFP_P_DataType *pstOperandB,
   BXDM_PPFP_P_DataType *pstResult
   );

void BXDM_PPFP_P_FixPtSub_isrsafe(
   const BXDM_PPFP_P_DataType *pstOperandA,
   const BXDM_PPFP_P_DataType *pstOperandB,
   BXDM_PPFP_P_DataType *pstResult
   );

void BXDM_PPFP_P_FixPtDiv_isrsafe(
   const BXDM_PPFP_P_DataType *pstOperand,
   const uint32_t uiDivisor,
   BXDM_PPFP_P_DataType *pstResult
   );

void BXDM_PPFP_P_FixPtMult_isrsafe(
   const BXDM_PPFP_P_DataType* pstOperand,
   const uint32_t uiMultiplier,
   BXDM_PPFP_P_DataType* pstResult
   );

void BXDM_PPFP_P_FixPtFractionalMul_isrsafe(
   const BXDM_PPFP_P_DataType* pstOperand,
   const uint16_t uiNumerator,
   const uint16_t uiDenominator,
   BXDM_PPFP_P_DataType* pstResult
   );

void BXDM_PPFP_P_FixPtBinaryFractionToBase10_isrsafe(
   BXDM_PPFP_P_DataType* pstOperand,
   uint32_t uiDstPrecision,
   uint32_t * puiBase10Fraction
   );

#ifdef __cplusplus
}
#endif

#endif /* #ifndef bxdm_pp_FP_H__ */
