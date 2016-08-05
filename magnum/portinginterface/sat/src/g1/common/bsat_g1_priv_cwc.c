/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its
 * licensors, and may only be used, duplicated, modified or distributed pursuant
 * to the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and Broadcom
 * expressly reserves all rights in and to the Software and all intellectual
 * property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 *    constitutes the valuable trade secrets of Broadcom, and you shall use all
 *    reasonable efforts to protect the confidentiality thereof, and to use
 *    this information only in connection with your use of Broadcom integrated
 *    circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
 *    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
 *    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
 *    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
 *    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
 *    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
 *    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
 *    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
 *    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
 *    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ******************************************************************************/
#include "bstd.h"
#include "bmth.h"
#include "bsat.h"
#include "bsat_priv.h"
#include "bsat_g1_priv.h"


BDBG_MODULE(bsat_g1_priv_cwc);

#define BSAT_DEBUG_CWC(x) /* x */


#ifndef BSAT_EXCLUDE_SPUR_CANCELLER

#define BSAT_CWC_STEP_SIZE 3
#define BSAT_CWC_INIT_CINT 0x40
#define BSAT_CWC_INIT_CLIN 0x100
#define BSAT_CWC_TRK_CINT  0x40
#define BSAT_CWC_TRK_CLIN  0x10


/******************************************************************************
 BSAT_g1_P_CwcDisable_isr() - this function disables all 6 spur cancellers
******************************************************************************/
BERR_Code BSAT_g1_P_CwcDisable_isr(BSAT_ChannelHandle h)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   uint32_t i;

   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_CWC_CTRL1, 0x8);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_CWC_CTRL1, 0);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_CWC_CTRL2, 0x00FC0FC0);

   for (i = 0; i < 6; i++)
      hChn->bCwcActive[i] = false;

   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_CwcReset_isr()
******************************************************************************/
BERR_Code BSAT_g1_P_CwcReset_isr(BSAT_ChannelHandle h)
{
   BSAT_g1_P_OrRegister_isrsafe(h, BCHP_SDS_CWC_CTRL1, 0x08);
   BSAT_g1_P_AndRegister_isrsafe(h, BCHP_SDS_CWC_CTRL1, ~0x08);
   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_CwcNarrowBw_isr()
******************************************************************************/
BERR_Code BSAT_g1_P_CwcNarrowBw_isr(BSAT_ChannelHandle h)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   uint32_t i;

   uint32_t cint, clin, lfc;

   hChn->functState++;
   cint = BSAT_CWC_INIT_CINT - (hChn->functState * (BSAT_CWC_INIT_CINT-BSAT_CWC_TRK_CINT)>>2);
   clin = BSAT_CWC_INIT_CLIN - (hChn->functState * (BSAT_CWC_INIT_CLIN-BSAT_CWC_TRK_CLIN)>>2);
   lfc = cint << BCHP_SDS_CWC_0_LFC1_LF_CINT_SHIFT;
   lfc |= (clin << BCHP_SDS_CWC_0_LFC1_LF_CLIN_SHIFT);

   for (i = 0; i < 6; i++)
   {
      if (hChn->bCwcActive[i])
      {
         BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_CWC_LFC1 + (i<<2), lfc);
      }
   }

   if (hChn->functState < 4)
      return BSAT_g1_P_EnableTimer_isr(h, BSAT_TimerSelect_eBaudUsec, 1000, BSAT_g1_P_CwcNarrowBw_isr);

   return hChn->passFunct(h);
}


/******************************************************************************
 BSAT_g1_P_CwcInit_isr()
******************************************************************************/
BERR_Code BSAT_g1_P_CwcInit_isr(BSAT_ChannelHandle h, BSAT_g1_FUNCT funct)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   BSAT_g1_P_Handle *hDev = (BSAT_g1_P_Handle*)(h->pDevice->pImpl);
   uint32_t range, Fc, Fspur, i, Q_hi, Q_lo, P_hi, P_lo, val32;
   int32_t diff;
   bool bIsCwcActive = false;
   uint32_t i_cw, stepSize, ctrl1, ctrl2, fcw, lfc;

   hChn->passFunct = funct;
   BSAT_g1_P_CwcDisable_isr(h);

   range = hChn->acqSettings.symbolRate << 1;
#if 0
   BSAT_g1_P_TunerGetActualLOFreq(h, &Fc);
#else
   Fc = hChn->actualTunerFreq;
#endif

   for (i = 0, i_cw = 0; (i_cw < 6) && (i < 6); i++)
   {
      Fspur = hDev->cwcFreq[i];
      if (Fspur == 0)
         continue;
      diff = Fspur - Fc;
      if ((diff >= (int32_t)range) || (diff <= (int32_t)-range))
         continue;

      BSAT_DEBUG_CWC(BDBG_MSG(("enabling CWC for spur at %u Hz, Fc=%u, initFreqOffset=%d, diff=%d", Fspur, Fc, hChn->initFreqOffset, diff)));
      bIsCwcActive = true;

      /* set ctrl1 */
      ctrl1 = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_SDS_CWC_CTRL1);
      ctrl1 &= ~0x7;
      ctrl1 |= (i_cw+1);
      stepSize = BSAT_CWC_STEP_SIZE;
      ctrl1 &= ~(0x0000000F << ((i_cw+1)<<2));
      ctrl1 |= (stepSize << ((i_cw+1)<<2));

      /* set ctrl2 */
      ctrl2 = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_SDS_CWC_CTRL2);
      ctrl2 &= ~(1<<(i_cw+6));   /* unfreeze loop filter integrator */
      ctrl2 &= ~(1<<(i_cw+18));  /* unfreeze tap */

      /* set freq control word */
      if (diff >= 0)
         val32 = (uint32_t)diff;
      else
         val32 = (uint32_t)-diff;
      BMTH_HILO_32TO64_Mul(val32, 268435456, &P_hi, &P_lo);
      BMTH_HILO_64TO64_Div32(P_hi, P_lo, hChn->acqSettings.symbolRate, &Q_hi, &Q_lo);
      if (diff >= 0)
         fcw = Q_lo;
      else
         fcw = ~Q_lo + 1;
      fcw &= 0x1FFFFFFF;

      /* set the phase/frequency detector and loop filter coefficients */
      lfc = BSAT_CWC_INIT_CINT << BCHP_SDS_CWC_0_LFC1_LF_CINT_SHIFT;
      lfc |= (BSAT_CWC_INIT_CLIN << BCHP_SDS_CWC_0_LFC1_LF_CLIN_SHIFT);

      BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_CWC_CTRL1, ctrl1);
      BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_CWC_CTRL2, ctrl2);
      BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_CWC_SPUR_FCW1 + (i_cw<<2), fcw);
      BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_CWC_LFC1 + (i_cw<<2), lfc);
      BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_CWC_FOFS1 + (i_cw<<2), 0);

      BSAT_g1_P_ToggleBit_isrsafe(h, BCHP_SDS_CWC_CTRL1, 0x08); /* toggle reset */

      hChn->bCwcActive[i] = true;
      BSAT_DEBUG_CWC(BDBG_MSG(("CWC(%d): Fc=%u, ctrl1=0x%08X, ctrl2=0x%08X, fcw=0x%08X, lfc=0x%08X", i_cw, Fc, ctrl1, ctrl2, fcw, lfc)));
      i_cw++;
   }

   if (bIsCwcActive)
   {
      hChn->functState = 0;
      return BSAT_g1_P_EnableTimer_isr(h, BSAT_TimerSelect_eBaudUsec, 100, BSAT_g1_P_CwcNarrowBw_isr);
   }

   return hChn->passFunct(h);
}

#endif
