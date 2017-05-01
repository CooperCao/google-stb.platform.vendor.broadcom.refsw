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
 * [File Description:]
 *
 ***************************************************************************/
#include "bstd.h"
#include "bmth.h"
#include "bast.h"
#include "bast_priv.h"
#include "bast_g3_priv.h"


BDBG_MODULE(bast_g3_priv_cwc);

#define BAST_DEBUG_CWC(x) /* x */


#ifndef BAST_EXCLUDE_SPUR_CANCELLER

#define BAST_CWC_STEP_SIZE 3
#define BAST_CWC_INIT_CINT 0x40
#define BAST_CWC_INIT_CLIN 0x100
#define BAST_CWC_TRK_CINT  0x40
#define BAST_CWC_TRK_CLIN  0x10


/******************************************************************************
 BAST_g3_P_ClearSpurCancellerConfig() - this function clears the spur canceller
                                        configuration
******************************************************************************/
BERR_Code BAST_g3_P_ClearSpurCancellerConfig(BAST_ChannelHandle h)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   int i;

   for (i = 0; i < 6; i++)
   {
      hChn->spurConfig[i].freq = 0;
      hChn->bCwcActive[i] = false;
   }
   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_g3_P_DisableSpurCanceller_isr() - this function disables all 6 spur cancellers
******************************************************************************/
BERR_Code BAST_g3_P_DisableSpurCanceller_isr(BAST_ChannelHandle h)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   uint32_t val32, i;

   val32 = 0x8;
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_CWC_CTRL1, &val32);

   val32 = 0;
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_CWC_CTRL1, &val32);

   val32 = 0x00FC0FC0;
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_CWC_CTRL2, &val32);

   for (i = 0; i < 6; i++)
      hChn->bCwcActive[i] = false;

   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_g3_P_DisableSpurCanceller() - Non-ISR context
******************************************************************************/
BERR_Code BAST_g3_P_DisableSpurCanceller(BAST_ChannelHandle h)
{
   BERR_Code retCode;

   BKNI_EnterCriticalSection();
   retCode = BAST_g3_P_DisableSpurCanceller_isr(h);
   BKNI_LeaveCriticalSection();
   return retCode;
}


/******************************************************************************
 BAST_g3_P_ResetCWC_isr()
******************************************************************************/
BERR_Code BAST_g3_P_ResetCWC_isr(BAST_ChannelHandle h)
{
   BAST_g3_P_OrRegister_isrsafe(h, BCHP_SDS_CWC_CTRL1, 0x08);
   BAST_g3_P_AndRegister_isrsafe(h, BCHP_SDS_CWC_CTRL1, ~0x08);
   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_g3_P_NarrowCwcBw_isr()
******************************************************************************/
static BERR_Code BAST_g3_P_NarrowCwcBw_isr(BAST_ChannelHandle h)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   uint32_t i;

   uint32_t cint, clin, lfc;

   hChn->funct_state++;
   cint = BAST_CWC_INIT_CINT - (hChn->funct_state * (BAST_CWC_INIT_CINT-BAST_CWC_TRK_CINT)>>2);
   clin = BAST_CWC_INIT_CLIN - (hChn->funct_state * (BAST_CWC_INIT_CLIN-BAST_CWC_TRK_CLIN)>>2);
   lfc = cint << BCHP_SDS_CWC_LFC1_LF_CINT_SHIFT;
   lfc |= (clin << BCHP_SDS_CWC_LFC1_LF_CLIN_SHIFT);

   for (i = 0; i < 6; i++)
   {
      if (hChn->bCwcActive[i])
      {
         BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_CWC_LFC1 + (i<<2), &lfc);
      }
   }

   if (hChn->funct_state < 4)
      return BAST_g3_P_EnableTimer_isr(h, BAST_TimerSelect_eBaudUsec, 1000, BAST_g3_P_NarrowCwcBw_isr);

   return hChn->passFunct(h);
}


/******************************************************************************
 BAST_g3_P_InitCWC_isr()
******************************************************************************/
BERR_Code BAST_g3_P_InitCWC_isr(BAST_ChannelHandle h, BAST_g3_FUNCT funct)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   uint32_t range, Fc, Fspur, i, Q_hi, Q_lo, P_hi, P_lo, val32;
   int32_t diff;
   bool bIsCwcActive = false;
   uint32_t i_cw, stepSize, ctrl1, ctrl2, fcw, lfc;

   hChn->passFunct = funct;
   BAST_g3_P_DisableSpurCanceller_isr(h);

   range = hChn->acqParams.symbolRate << 1;
   BAST_g3_P_TunerGetActualLOFreq_isrsafe(h, &Fc);

   for (i = 0, i_cw = 0; (i_cw < 6) && (i < 6); i++)
   {
      Fspur = hChn->spurConfig[i].freq;
      if (Fspur == 0)
         continue;
      diff = Fspur - Fc;
      if ((diff >= (int32_t)range) || (diff <= (int32_t)-range))
         continue;

      BAST_DEBUG_CWC(BDBG_MSG(("enabling CWC for spur at %u Hz, Fc=%u, initFreqOffset=%d, diff=%d", Fspur, Fc, hChn->initFreqOffset, diff)));
      bIsCwcActive = true;

      /* set ctrl1 */
      BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_CWC_CTRL1, &ctrl1);
      ctrl1 &= ~0x7;
      ctrl1 |= (i_cw+1);
      stepSize = BAST_CWC_STEP_SIZE;
      ctrl1 &= ~(0x0000000F << ((i_cw+1)<<2));
      ctrl1 |= (stepSize << ((i_cw+1)<<2));

      /* set ctrl2 */
      BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_CWC_CTRL2, &ctrl2);
      ctrl2 &= ~(1<<(i_cw+6));   /* unfreeze loop filter integrator */
      ctrl2 &= ~(1<<(i_cw+18));  /* unfreeze tap */

      /* set freq control word */
      if (diff >= 0)
         val32 = (uint32_t)diff;
      else
         val32 = (uint32_t)-diff;
      BMTH_HILO_32TO64_Mul(val32, 268435456, &P_hi, &P_lo);
      BMTH_HILO_64TO64_Div32(P_hi, P_lo, hChn->acqParams.symbolRate, &Q_hi, &Q_lo);
      if (diff >= 0)
         fcw = Q_lo;
      else
         fcw = ~Q_lo + 1;
      fcw &= 0x1FFFFFFF;

      /* set the phase/frequency detector and loop filter coefficients */
      lfc = BAST_CWC_INIT_CINT << BCHP_SDS_CWC_LFC1_LF_CINT_SHIFT;
      lfc |= (BAST_CWC_INIT_CLIN << BCHP_SDS_CWC_LFC1_LF_CLIN_SHIFT);

      BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_CWC_CTRL1, &ctrl1);
      BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_CWC_CTRL2, &ctrl2);
      BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_CWC_SPUR_FCW1 + (i_cw<<2), &fcw);
      BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_CWC_LFC1 + (i_cw<<2), &lfc);

      val32 = 0;
      BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_CWC_FOFS1 + (i_cw<<2), &val32);

      BAST_g3_P_ToggleBit_isrsafe(h, BCHP_SDS_CWC_CTRL1, 0x08); /* toggle reset */

      hChn->bCwcActive[i] = true;
      BAST_DEBUG_CWC(BDBG_MSG(("CWC(%d): Fc=%u, ctrl1=0x%08X, ctrl2=0x%08X, fcw=0x%08X, lfc=0x%08X", i_cw, Fc, ctrl1, ctrl2, fcw, lfc)));
      i_cw++;
   }

   if (bIsCwcActive)
   {
      hChn->funct_state = 0;
      return BAST_g3_P_EnableTimer_isr(h, BAST_TimerSelect_eBaudUsec, 100, BAST_g3_P_NarrowCwcBw_isr);
   }

   return hChn->passFunct(h);
}

#endif
