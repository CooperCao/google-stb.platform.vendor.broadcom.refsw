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

#include "bstd.h"
#include "bdbg.h"                /* Dbglib */
#include "bxdm_pp_fp.h"

BDBG_MODULE(BXDM_PPFP);

/* Fixed Point Arithmetic */
void BXDM_PPFP_P_FixPtAdd_isrsafe(
   const BXDM_PPFP_P_DataType* pstOperandA,
   const BXDM_PPFP_P_DataType* pstOperandB,
   BXDM_PPFP_P_DataType* pstResult
   )
{
   BXDM_PPFP_P_DataType stTempResult;

   BDBG_ASSERT(pstOperandA);
   BDBG_ASSERT(pstOperandB);
   BDBG_ASSERT(pstResult);

   /* TODO: Add overflow checking */
   stTempResult.uiWhole = pstOperandA->uiWhole + pstOperandB->uiWhole;
   stTempResult.uiFractional = pstOperandA->uiFractional + pstOperandB->uiFractional;

   if ( stTempResult.uiFractional >= BXDM_PictureProvider_P_FixedPoint_FractionalOverflow )
   {
      stTempResult.uiWhole += 1;
      stTempResult.uiFractional = stTempResult.uiFractional & (BXDM_PictureProvider_P_FixedPoint_FractionalOverflow - 1);
   }

   *pstResult = stTempResult;

   return;
}

void BXDM_PPFP_P_FixPtSub_isrsafe(
   const BXDM_PPFP_P_DataType* pstOperandA,
   const BXDM_PPFP_P_DataType* pstOperandB,
   BXDM_PPFP_P_DataType* pstResult
   )
{
   BXDM_PPFP_P_DataType stTempResult;

   BDBG_ASSERT(pstOperandA);
   BDBG_ASSERT(pstOperandB);
   BDBG_ASSERT(pstResult);

   /* TODO: Add underflow checking */
   stTempResult.uiWhole = pstOperandA->uiWhole - pstOperandB->uiWhole;
   if ( pstOperandA->uiFractional >= pstOperandB->uiFractional )
   {
      stTempResult.uiFractional = pstOperandA->uiFractional - pstOperandB->uiFractional;
   }
   else
   {
      /* Do the carry */
      stTempResult.uiWhole -= 1;
      stTempResult.uiFractional = (BXDM_PictureProvider_P_FixedPoint_FractionalOverflow + pstOperandA->uiFractional) - pstOperandB->uiFractional;
   }

   *pstResult = stTempResult;

   return;
}

void BXDM_PPFP_P_FixPtDiv_isrsafe(
   const BXDM_PPFP_P_DataType* pstOperand,
   const uint32_t uiDivisor,
   BXDM_PPFP_P_DataType* pstResult
   )
{
   uint32_t uiFractionalRemainder;

   BDBG_ASSERT(pstOperand);
   BDBG_ASSERT(pstResult);

   if ( uiDivisor == 0 )
   {
      BDBG_WRN(("Divide by zero!"));
      return;
   }

   /* Copy operand into result */
   *pstResult = *pstOperand;

   if ( uiDivisor == 1 )
   {
      return;
   }

   /* Do the divide */

   /* Carry a 1 into the fractional component */
   if (pstResult->uiWhole)
   {
      pstResult->uiWhole--;
      pstResult->uiFractional += BXDM_PictureProvider_P_FixedPoint_FractionalOverflow;
   }

   /* Divide the fractional component */
   uiFractionalRemainder = pstResult->uiFractional % uiDivisor;
   pstResult->uiFractional /= uiDivisor;

   if (pstResult->uiWhole)
   {
      /* Add the remainder from division of whole component to the
       * fractional component */
      pstResult->uiFractional += (((pstResult->uiWhole % uiDivisor) * BXDM_PictureProvider_P_FixedPoint_FractionalOverflow) + uiFractionalRemainder) / uiDivisor;

      /* Divide the whole component */
      pstResult->uiWhole /= uiDivisor;

      /* Normalize the improper fractional component */
      pstResult->uiWhole += pstResult->uiFractional / BXDM_PictureProvider_P_FixedPoint_FractionalOverflow;
      pstResult->uiFractional %= BXDM_PictureProvider_P_FixedPoint_FractionalOverflow;
   }

   return;
}

void BXDM_PPFP_P_FixPtMult_isrsafe(
   const BXDM_PPFP_P_DataType* pstOperand,
   const uint32_t uiMultiplier,
   BXDM_PPFP_P_DataType* pstResult
   )
{
   uint32_t i;
   BXDM_PPFP_P_DataType stLocalResult;
   BDBG_ASSERT(pstOperand);
   BDBG_ASSERT(pstResult);

   if (uiMultiplier == 0)
   {
      stLocalResult.uiWhole = 0;
      stLocalResult.uiFractional = 0;
   }
   else
   {
      stLocalResult = *pstOperand;

      for (i = 1; i < uiMultiplier; i++)
      {
         BXDM_PPFP_P_FixPtAdd_isrsafe(
            &stLocalResult,
            pstOperand,
            &stLocalResult
            );
      }
   }

   *pstResult = stLocalResult;

   return;
}

void BXDM_PPFP_P_FixPtFractionalMul_isrsafe(
   const BXDM_PPFP_P_DataType* pstOperand,
   const uint16_t uiNumerator,
   const uint16_t uiDenominator,
   BXDM_PPFP_P_DataType* pstResult
   )
{
   BDBG_ASSERT(pstOperand);
   BDBG_ASSERT(pstResult);

   if ( uiDenominator == uiNumerator )
   {
      *pstResult = *pstOperand;
   }
   else if ( 0 == uiNumerator )
   {
      pstResult->uiWhole = 0;
      pstResult->uiFractional = 0;
   }
   else
   {
      pstResult->uiFractional = ( pstOperand->uiFractional * uiNumerator ) / uiDenominator ;
      pstResult->uiFractional += ( pstOperand->uiWhole * uiNumerator ) % uiDenominator ;

      pstResult->uiWhole = ( pstOperand->uiWhole * uiNumerator ) / uiDenominator ;

      /* Check for overflow on the fractional component. */
      if ( pstResult->uiFractional >= BXDM_PictureProvider_P_FixedPoint_FractionalOverflow )
      {
         pstResult->uiWhole += ( pstResult->uiFractional >> BXDM_PictureProvider_P_FixedPoint_FractionalShift );

         pstResult->uiFractional = pstResult->uiFractional & BXDM_PictureProvider_P_FixedPoint_FractionalMask;
      }
   }

   return;
}

/* Convert the binary digits to the right of the decimal point to a base 10 number.
 * Recall that the values of the digits are, 1/2, 1/4, 1/8, 1/16....
 */
static const uint32_t s_auiBinaryDecimalValueToBase10LUT[BXDM_PictureProvider_P_FixedPoint_FractionalBitsOfPrecision] =
{
   500000,  /* 1/2 */
   250000,  /* 1/4 */
   125000,  /* 1/8 */
    62500,  /* 1/16 */
    31250,  /* 1/32 */
    15625,  /* 1/64 */
     7812,  /* 1/128 */
     3906,  /* 1/256 */
     1953,  /* 1/512 */
      976,  /* 1/1024 */
      488,  /* 1/2048 */
      244,  /* 1/4096 */
      122,  /* 1/8192 */
       61,  /* 1/16384 */
       30,  /* 1/32768 */
       15   /* 1/65536 */
};

/* Precision of the preceding table. */
#define BXDM_PictureProvider_P_FixedPoint_DecimalPrecision 6


void BXDM_PPFP_P_FixPtBinaryFractionToBase10_isrsafe(
   BXDM_PPFP_P_DataType* pstOperand,
   uint32_t uiDstPrecision,
   uint32_t * puiBase10Fraction
   )
{
   uint32_t uiFracBase10=0;
   uint32_t uiDivisor=1;

   BDBG_ASSERT(pstOperand);
   BDBG_ASSERT(puiBase10Fraction);

   if ( 0 != pstOperand->uiFractional )
   {
      uint32_t uiMask=BXDM_PictureProvider_P_FixedPoint_FractionalMsbMask;
      uint32_t i;

      /* For each bit set in the binary fraction, add the appropriate
       * base 10 value to the sum.
       */
      for ( i=0; i < BXDM_PictureProvider_P_FixedPoint_FractionalBitsOfPrecision ; i++ )
      {
         if ( pstOperand->uiFractional & uiMask  )
         {
            uiFracBase10 += s_auiBinaryDecimalValueToBase10LUT[ i ];
         }

         uiMask >>= 1;
      }

      /* Adjust the result to the desired number of places.
       */
      if ( uiDstPrecision < BXDM_PictureProvider_P_FixedPoint_DecimalPrecision )
      {
         i = BXDM_PictureProvider_P_FixedPoint_DecimalPrecision - uiDstPrecision;

         /* This switch statement is a bit clunky and limited, but is saves doing a
          * bunch of mulitplies in a loop to raise "uiDivisor" to the power of "i".
          */
         switch( i )
         {
            case 5:     uiDivisor = 100000;  break;
            case 4:     uiDivisor = 10000;   break;
            case 3:     uiDivisor = 1000;    break;
            case 2:     uiDivisor = 100;     break;
            case 1:     uiDivisor = 10;      break;
            default:    uiDivisor = 1;       break;
         }

         uiFracBase10 /= uiDivisor;

      }
   }

   *puiBase10Fraction = uiFracBase10;

   return;
}
