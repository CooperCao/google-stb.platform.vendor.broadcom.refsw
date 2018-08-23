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

#include "bstd.h"
#include "bdbg.h"                /* Dbglib */
#include "bxdm_pp_fp.h"

BDBG_MODULE(BXDM_PPFP);

/* Fixed Point Arithmetic */
void BXDM_PPFP_P_FixPtAdd_isr(
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

void BXDM_PPFP_P_FixPtSub_isr(
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

void BXDM_PPFP_P_FixPtDiv_isr(
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

void BXDM_PPFP_P_FixPtMult_isr(
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
         BXDM_PPFP_P_FixPtAdd_isr(
            &stLocalResult,
            pstOperand,
            &stLocalResult
            );
      }
   }

   *pstResult = stLocalResult;

   return;
}

void BXDM_PPFP_P_FixPtFractionalMul_isr(
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


void BXDM_PPFP_P_FixPtBinaryFractionToBase10_isr(
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

