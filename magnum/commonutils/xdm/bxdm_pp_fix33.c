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
#include "bkni.h"
#include "bdbg.h"

#include "bxdm_pp_fix33.h"

BDBG_MODULE(BXDM_PP_FIX33);

#define BXDM_PP_Fix33_RADIX 31

/* Convert to BXDM_PP_Fix33_t */
BXDM_PP_Fix33_t BXDM_PP_Fix33_from_mixedfraction_isrsafe(const uint32_t uiWhole, const uint32_t uiNumerator, const uint32_t uiDenominator)
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
BXDM_PP_Fix33_t BXDM_PP_Fix33_from_int32_isrsafe(const int32_t iValue)
{
   return ((BXDM_PP_Fix33_t) iValue) << BXDM_PP_Fix33_RADIX;
}
#endif

BXDM_PP_Fix33_t BXDM_PP_Fix33_from_uint32_isrsafe(const uint32_t uiValue)
{
   return ((BXDM_PP_Fix33_t) uiValue) << BXDM_PP_Fix33_RADIX;
}

/* BXDM_PP_Fix33_t math operations */
BXDM_PP_Fix33_t BXDM_PP_Fix33_add_isrsafe(const BXDM_PP_Fix33_t fixOperand1, const BXDM_PP_Fix33_t fixOperand2)
{
   return fixOperand1 + fixOperand2;
}

BXDM_PP_Fix33_t BXDM_PP_Fix33_sub_isrsafe(const BXDM_PP_Fix33_t fixOperand1, const BXDM_PP_Fix33_t fixOperand2)
{
   return fixOperand1 - fixOperand2;
}

BXDM_PP_Fix33_t BXDM_PP_Fix33_mulu_isrsafe(const BXDM_PP_Fix33_t fixOperand1, const uint32_t uiOperand2)
{
   return fixOperand1 * uiOperand2;
}

/* udivide64 copied from /BSEAV/linux/driver/build/97038/bsettop_udivdi3.c */
static void BXDM_PP_Fix33_udivide64_isrsafe(uint64_t n, uint64_t d, uint64_t *pResult, uint64_t *pRemainder)
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

BXDM_PP_Fix33_t BXDM_PP_Fix33_divu_isrsafe(const BXDM_PP_Fix33_t fixOperand1, const uint32_t uiOperand2)
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
      uiNumerator = BXDM_PP_Fix33_neg_isrsafe(fixOperand1);
   }
   else
   {
      uiNumerator = fixOperand1;
   }

   BXDM_PP_Fix33_udivide64_isrsafe(uiNumerator, uiOperand2, &uiResult, &uiRemainder);

   fixResult = uiResult;

   if ( true == bNegative )
   {
      fixResult = BXDM_PP_Fix33_neg_isrsafe(fixResult);
   }

   return fixResult;
#else
   return fixOperand1 / uiOperand2;
#endif
}

BXDM_PP_Fix33_t BXDM_PP_Fix33_neg_isrsafe(const BXDM_PP_Fix33_t fixOperand)
{
   return -fixOperand;
}

/* Convert from BXDM_PP_Fix33_t */
int32_t BXDM_PP_Fix33_to_int32_isrsafe(const BXDM_PP_Fix33_t fixValue)
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

uint32_t BXDM_PP_Fix33_to_uint32_isrsafe(const BXDM_PP_Fix33_t fixValue)
{
   return ((fixValue + (1 << (BXDM_PP_Fix33_RADIX-1))) >> BXDM_PP_Fix33_RADIX);
}
