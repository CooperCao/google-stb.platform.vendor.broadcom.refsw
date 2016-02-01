/***************************************************************************
 *     Copyright (c) 2003-2013, Broadcom Corporation
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
BERR_Code BAST_g3_P_NarrowCwcBw_isr(BAST_ChannelHandle h)
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
