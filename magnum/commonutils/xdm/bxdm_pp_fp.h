/***************************************************************************
 *     Copyright (c) 2003-2011, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * [File Description:]
 *
 * Revision History:
 *
 * $brcm_Log: $
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

void BXDM_PPFP_P_FixPtAdd_isr(
   const BXDM_PPFP_P_DataType *pstOperandA,
   const BXDM_PPFP_P_DataType *pstOperandB,
   BXDM_PPFP_P_DataType *pstResult
   );

void BXDM_PPFP_P_FixPtSub_isr(
   const BXDM_PPFP_P_DataType *pstOperandA,
   const BXDM_PPFP_P_DataType *pstOperandB,
   BXDM_PPFP_P_DataType *pstResult
   );

void BXDM_PPFP_P_FixPtDiv_isr(
   const BXDM_PPFP_P_DataType *pstOperand,
   const uint32_t uiDivisor,
   BXDM_PPFP_P_DataType *pstResult
   );

void BXDM_PPFP_P_FixPtMult_isr(
   const BXDM_PPFP_P_DataType* pstOperand,
   const uint32_t uiMultiplier,
   BXDM_PPFP_P_DataType* pstResult
   );

void BXDM_PPFP_P_FixPtFractionalMul_isr(
   const BXDM_PPFP_P_DataType* pstOperand,
   const uint16_t uiNumerator,
   const uint16_t uiDenominator,
   BXDM_PPFP_P_DataType* pstResult
   );

void BXDM_PPFP_P_FixPtBinaryFractionToBase10_isr(
   BXDM_PPFP_P_DataType* pstOperand,
   uint32_t uiDstPrecision,
   uint32_t * puiBase10Fraction
   );

#ifdef __cplusplus
}
#endif

#endif /* #ifndef bxdm_pp_FP_H__ */
