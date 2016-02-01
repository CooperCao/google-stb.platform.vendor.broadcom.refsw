/***************************************************************************
 *     Copyright (c) 2003-2012, Broadcom Corporation
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
#include "bkni.h"
#include "bdbg.h"

#include "bxdm_pp_fix33.h"

BDBG_MODULE(BXDM_PP_FIX33);

#define BXDM_PP_Fix33_RADIX 31

/* Convert to BXDM_PP_Fix33_t */
BXDM_PP_Fix33_t BXDM_PP_Fix33_from_mixedfraction(const uint32_t uiWhole, const uint32_t uiNumerator, const uint32_t uiDenominator)
{
   BXDM_PP_Fix33_t fixTemp = 0;

   fixTemp = uiWhole;
   fixTemp <<= BXDM_PP_Fix33_RADIX;

   fixTemp |= (uint32_t) ( ( ( ((uint64_t) uiNumerator) * (((uint64_t)1) << (BXDM_PP_Fix33_RADIX)) ) / uiDenominator ) & 0x7FFFFFFF );

#if 0
   BKNI_Printf("%d %u/%u --> %016llx\n",
               iWhole,
               uiNumerator,
               uiDenominator,
               fixTemp
               );
#endif

   return fixTemp;
}

#if !B_REFSW_MINIMAL /* SWSTB-461 */
BXDM_PP_Fix33_t BXDM_PP_Fix33_from_int32(const int32_t iValue)
{
   return ((BXDM_PP_Fix33_t) iValue) << BXDM_PP_Fix33_RADIX;
}
#endif

BXDM_PP_Fix33_t BXDM_PP_Fix33_from_uint32(const uint32_t uiValue)
{
   return ((BXDM_PP_Fix33_t) uiValue) << BXDM_PP_Fix33_RADIX;
}

/* BXDM_PP_Fix33_t math operations */
BXDM_PP_Fix33_t BXDM_PP_Fix33_add(const BXDM_PP_Fix33_t fixOperand1, const BXDM_PP_Fix33_t fixOperand2)
{
   return fixOperand1 + fixOperand2;
}

BXDM_PP_Fix33_t BXDM_PP_Fix33_sub(const BXDM_PP_Fix33_t fixOperand1, const BXDM_PP_Fix33_t fixOperand2)
{
   return fixOperand1 - fixOperand2;
}

BXDM_PP_Fix33_t BXDM_PP_Fix33_mulu(const BXDM_PP_Fix33_t fixOperand1, const uint32_t uiOperand2)
{
   return fixOperand1 * uiOperand2;
}

/* udivide64 copied from /BSEAV/linux/driver/build/97038/bsettop_udivdi3.c */
static void udivide64(uint64_t n, uint64_t d, uint64_t *pResult, uint64_t *pRemainder)
{
        uint64_t r = 0;
        uint64_t rem = 0;

        if (n == 0) {
        /* If n == 0, special case to 0 (0/x == 0) */
                r = 0;
                rem = 0;
        } else if (d > n) {
        /* If d > n, special case to 0 (n would be the remainder) */
                r = 0;
                rem = n;
        } else if (n < (d*2)) {
        /* If n < d*2, special case to 1 */
                r = 1;
                rem = n-d;
        } else if ((d <= (uint64_t)0x00000000FFFFFFFF) && (n <= (uint64_t)0x00000000FFFFFFFF)) {
        /* If they are 32bit quantities, go ahead and perform a basic 32bit div instead */
                uint32_t n32 = (uint32_t)n;
                uint32_t d32 = (uint32_t)d;
                uint32_t r32 = n32/d32;

                r = r32;

                r32 = n32%d32;
                rem = r32;
        } else { /* Otherwise, complicated division time */

                /* This segment of code is based on the routine by Ian Kaplan at
                 * http://www.bearcave.com/software/divide.htm, which was licensed at
                 * the time as:
                 *
                 * "Use of this program, for any purpose, is granted the author, Ian
                 * Kaplan, as long as this copyright notice is included in the source code
                 * or any source code derived from this program.  The user assumes all
                 * responsibility for using this code.
                 *
                 * Ian Kaplan, October 1996
                 */
                uint64_t t = 0, tmp = 0;
                uint32_t bits = 64;
                uint32_t ix;

                r = 0;
                /* Figure out about what the order of the remainder would be, to shortcut
                 * a great deal of the math */
                while (rem < d) {
                        rem = (rem << 1) | (n>>63);
                        tmp = n;
                        n = n << 1;
                        bits--;
                }
                /* Undo the last step, since we just went one too far */
                n = tmp;
                rem = rem >> 1;
                bits++;
                /* And now, buckle down and do the rest of the work */
                for (ix=0; ix < bits; ix++) {
                        rem = (rem << 1) | (n>>63);
                        tmp = rem - d;
                        t = !(tmp>>63);
                        n = n << 1;
                        r = (r << 1) | t;
                        if (t) {
                                rem = tmp;
                        }
                }
                /* End of code based on Ian Kaplan's work. */
        }

    *pResult = r;
    *pRemainder = rem;
    return;
}

BXDM_PP_Fix33_t BXDM_PP_Fix33_divu(const BXDM_PP_Fix33_t fixOperand1, const uint32_t uiOperand2)
{
#if 1
   /* Avoid 64-bit divide because linux kernel doesn't implement it */
   bool bNegative = false;
   uint64_t uiNumerator;
   uint64_t uiResult;
   uint64_t uiRemainder;
   BXDM_PP_Fix33_t fixResult;

   if ( fixOperand1 < 0 )
   {
      bNegative = true;
      uiNumerator = BXDM_PP_Fix33_neg(fixOperand1);
   }
   else
   {
      uiNumerator = fixOperand1;
   }

   udivide64(uiNumerator, uiOperand2, &uiResult, &uiRemainder);

   fixResult = uiResult;

   if ( true == bNegative )
   {
      fixResult = BXDM_PP_Fix33_neg(fixResult);
   }

   return fixResult;
#else
   return fixOperand1 / uiOperand2;
#endif
}

BXDM_PP_Fix33_t BXDM_PP_Fix33_neg(const BXDM_PP_Fix33_t fixOperand)
{
   return -fixOperand;
}

/* Convert from BXDM_PP_Fix33_t */
int32_t BXDM_PP_Fix33_to_int32(const BXDM_PP_Fix33_t fixValue)
{
   if (fixValue >= 0)
   {
      return (((fixValue + (1 << (BXDM_PP_Fix33_RADIX-1))) << 1) >> 32);
   }
   else
   {
      return (((fixValue - (1 << (BXDM_PP_Fix33_RADIX-1))) << 1) >> 32);
   }
}

uint32_t BXDM_PP_Fix33_to_uint32(const BXDM_PP_Fix33_t fixValue)
{
   return ((fixValue + (1 << (BXDM_PP_Fix33_RADIX-1))) >> BXDM_PP_Fix33_RADIX);
}
