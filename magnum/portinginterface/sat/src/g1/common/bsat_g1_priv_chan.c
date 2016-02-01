/******************************************************************************
*    (c)2011-2013 Broadcom Corporation
*
* This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
* $brcm_Workfile: $
* $brcm_Revision: $
* $brcm_Date: $
*
* Module Description:
*
* Revision History:
*
* $brcm_Log: $
*
*****************************************************************************/
#include "bstd.h"
#include "bmth.h"
#include "bsat.h"
#include "bsat_priv.h"
#include "bsat_g1_priv.h"
#if ((BCHP_CHIP==45216) || (BCHP_CHIP==45308))
#include "bchp_stb_chan_ctrl_0.h"
#else
#include "bchp_stb_chan_ctrl.h"
#endif

BDBG_MODULE(bsat_g1_priv_chan);

#define BSAT_DEBUG_NOTCH(x) /* x */


/* DCO notch filter selection */
typedef enum BSAT_NotchSelect
{
   BSAT_NotchSelect_e0 = 0,
   BSAT_NotchSelect_e1
} BSAT_NotchSelect;

static const uint8_t BSAT_Notch_beta[5] = {8, 9, 10, 11, 12};
/* static const uint8_t BSAT_Notch_delta[5] = {3, 3, 3, 3, 4}; */


/******************************************************************************
 BSAT_g1_P_TunerInit()
******************************************************************************/
BERR_Code BSAT_g1_P_TunerInit(BSAT_ChannelHandle h, BSAT_g1_FUNCT nextFunct)
{
   uint32_t mask;

   if (h->channel < 8)
      mask = (1 << h->channel);
   else
      mask = (1 << (8 - h->channel));

   BSAT_g1_P_ToggleBit_isrsafe(h, BCHP_STB_CHAN_CTRL_LOCAL_SW_RESET, mask);

   /* turn off test channel */
   BSAT_g1_P_OrRegister_isrsafe(h, BCHP_STB_CHAN_CTRL_PWRDN, BCHP_STB_CHAN_CTRL_PWRDN_TEST_MASK);

#if BCHP_CHIP!=7364
   BSAT_g1_P_AndRegister_isrsafe(h, BCHP_STB_CHAN_CTRL_XBAR_CTRL, ~0x0F); /* toggle all adc fifos */
   BSAT_g1_P_OrRegister_isrsafe(h, BCHP_STB_CHAN_CTRL_XBAR_CTRL, 0x0F);   /* turn on all adc fifos */
#endif

   if (nextFunct)
      return nextFunct(h);
   else
      return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_TunerUpdateActualTunerFreq_isr()
******************************************************************************/
BERR_Code BSAT_g1_P_TunerUpdateActualTunerFreq_isr(BSAT_ChannelHandle h)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;

   hChn->actualTunerFreq = hChn->acqSettings.freq + hChn->tunerIfStep;
   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_SetChanDecFcw_isr() - sets CHAN_DEC_FCW_CHx based on Fs and Fc
******************************************************************************/
BERR_Code BSAT_g1_P_SetChanDecFcw_isr(BSAT_ChannelHandle h)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   uint32_t P_hi, P_lo, Q_hi, Q_lo;

   /* fcw = (2^28)*Fc/Fadc */
   BMTH_HILO_32TO64_Mul(hChn->actualTunerFreq / 1000, 268435456, &P_hi, &P_lo);
   BMTH_HILO_64TO64_Div32(P_hi, P_lo, BSAT_ADC_FREQ_KHZ, &Q_hi, &Q_lo);  /* Fadc=4968MHz */
   Q_lo = -Q_lo;  /* program negative fcw for proper spectral inversion */
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_STB_CHAN_CHx_DEC_FCW, Q_lo & 0xFFFFFFF);
   /* BDBG_MSG(("BCHP_CHAN_DEC_FCW_CHx: Fc=%u, val=0x%X", hChn->actualTunerFreq, Q_lo)); */
   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_ConfigChanAgc_isr()
******************************************************************************/
BERR_Code BSAT_g1_P_ConfigChanAgc_isr(BSAT_ChannelHandle h, bool bTracking)
{
   uint32_t val, agc_thresh;

   /* agc_chx_byp = 0
      agc_chx_la_byp = 0
      agc_chx_la_frz = 0
      agc_chx_lf_frz = 0
      agc_chx_lf_k1 = 3
    */
#if BCHP_CHIP==45308
   val = (9 << BCHP_STB_CHAN_CH0_AGC_CTRL_AGC_LF_K1_SHIFT) & BCHP_STB_CHAN_CH0_AGC_CTRL_AGC_LF_K1_MASK;
#else
   val = (3 << BCHP_STB_CHAN_CH0_AGC_CTRL_AGC_LF_K1_SHIFT) & BCHP_STB_CHAN_CH0_AGC_CTRL_AGC_LF_K1_MASK;
#endif
   if (bTracking)
      agc_thresh = 7366246;
   else
      agc_thresh = 1074790;
   val |= agc_thresh << 2;
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_STB_CHAN_CHx_AGC_CTRL, val);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_STB_CHAN_CHx_AGC_LF_INT, 0);
   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_EnableNotch_isr()
******************************************************************************/
BERR_Code BSAT_g1_P_EnableNotch_isr(BSAT_ChannelHandle h, BSAT_NotchSelect n, int32_t freqHz, BSAT_NotchSettings *pSettings)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   uint32_t val, ctl_reg, fcw_reg, freq, P_hi, P_lo, Q_hi, Q_lo;
   uint8_t beta, delta;

   BDBG_ASSERT(h);
   BDBG_ASSERT(n <= BSAT_NotchSelect_e1);

   if (n == BSAT_NotchSelect_e0)
   {
      ctl_reg = BCHP_STB_CHAN_CHx_NOTCH_0_CTRL;
      fcw_reg = BCHP_STB_CHAN_CHx_NOTCH_0_FCW;
   }
   else
   {
      ctl_reg = BCHP_STB_CHAN_CHx_NOTCH_1_CTRL;
      fcw_reg = BCHP_STB_CHAN_CHx_NOTCH_1_FCW;
   }

   /* set baseband dco fcw = (2^31)*4*Fc/Fs */
   if (freqHz < 0)
      freq = -freqHz;
   else
      freq = freqHz;
   BMTH_HILO_32TO64_Mul(freq * 4, 2147483648UL, &P_hi, &P_lo);
   BMTH_HILO_64TO64_Div32(P_hi, P_lo, hChn->sampleFreq, &Q_hi, &Q_lo);  /* div by (Fs_adc/2) */
   if (freqHz < 0)
      Q_lo = -Q_lo;
   BSAT_g1_P_WriteRegister_isrsafe(h, fcw_reg, Q_lo);

   /* set notch_ctl */
   beta = BSAT_Notch_beta[0];
   delta = 0; /* BSAT_Notch_delta[0]; */
   val = ((delta & 0xF) << 16);
   val |= ((beta & 0x1F) << 8);
   val |= (pSettings->alpha & 0x1F);
   BSAT_DEBUG_NOTCH(BDBG_MSG(("notch%d: ctl=%08X, fcw=%08X", n, val, Q_lo)));
   BSAT_g1_P_WriteRegister_isrsafe(h, ctl_reg, val);

   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_DisableNotch_isrsafe()
******************************************************************************/
BERR_Code BSAT_g1_P_DisableNotch_isrsafe(BSAT_ChannelHandle h, BSAT_NotchSelect n)
{
   uint32_t ctl_reg, fcw_reg, int_reg;

   BDBG_ASSERT(h);
   BDBG_ASSERT(n <= BSAT_NotchSelect_e1);

   if (n == BSAT_NotchSelect_e0)
   {
      ctl_reg = BCHP_STB_CHAN_CHx_NOTCH_0_CTRL;
      fcw_reg = BCHP_STB_CHAN_CHx_NOTCH_0_FCW;
      int_reg = BCHP_STB_CHAN_CHx_NOTCH_0_INT_LF;
   }
   else
   {
      ctl_reg = BCHP_STB_CHAN_CHx_NOTCH_1_CTRL;
      fcw_reg = BCHP_STB_CHAN_CHx_NOTCH_1_FCW;
      int_reg = BCHP_STB_CHAN_CHx_NOTCH_1_INT_LF;
   }

   BSAT_g1_P_WriteRegister_isrsafe(h, ctl_reg, 0x30000000); /* freeze and bypass */
   BSAT_g1_P_WriteRegister_isrsafe(h, fcw_reg, 0) /* clear the fcw */;
   BSAT_g1_P_WriteRegister_isrsafe(h, int_reg, 0) /* clear the integrator */;
   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_DisableAllNotch_isrsafe()
******************************************************************************/
BERR_Code BSAT_g1_P_DisableAllNotch_isrsafe(BSAT_ChannelHandle h)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;

   BSAT_g1_P_DisableNotch_isrsafe(h, BSAT_NotchSelect_e0);
   BSAT_g1_P_DisableNotch_isrsafe(h, BSAT_NotchSelect_e1);
   hChn->notchState = -1;
   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_SetNotch_isr()
******************************************************************************/
BERR_Code BSAT_g1_P_SetNotch_isr(BSAT_ChannelHandle h)
{
   BERR_Code retCode = BERR_SUCCESS;
   BSAT_g1_P_Handle *hDev = (BSAT_g1_P_Handle *)(h->pDevice->pImpl);
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   uint32_t range, min_freq, max_freq, spurFreq, alpha;
   int32_t notchFreq;
   uint8_t numSpursFound, i, spur_index[2];

   BSAT_g1_P_DisableAllNotch_isrsafe(h);
   if ((hDev->nNotch== 0) || hChn->miscSettings.bDisableNotch)
      return BERR_SUCCESS;

   /* determine if spur is within Fb*Nyquist/2 */
   range = hChn->acqSettings.symbolRate;
   alpha = hChn->acqSettings.options & BSAT_ACQ_NYQUIST_MASK;
   if (alpha & BSAT_ACQ_NYQUIST_35)
      range = (range * 27) / 40;
   else if (alpha & BSAT_ACQ_NYQUIST_10)
      range = (range * 11) / 20;
   else
      range = (range * 3) / 5;
   min_freq = hChn->actualTunerFreq - range;
   max_freq = hChn->actualTunerFreq + range;

   for (i = 0, numSpursFound = 0; (numSpursFound < 2) && (i < hDev->nNotch); i++)
   {
      spurFreq = hDev->notchSettings[i].freq;
      if ((spurFreq >= min_freq) && (spurFreq <= max_freq))
      {
         spur_index[numSpursFound] = i;
         numSpursFound++;
      }
   }

   for (i = 0; i < numSpursFound; i++)
   {
      spurFreq = hDev->notchSettings[spur_index[i]].freq;
      notchFreq = hChn->actualTunerFreq - spurFreq;
      BSAT_DEBUG_NOTCH(BDBG_MSG(("notch%d: Fspur=%u, Ftuner=%u, Fnotch=%d", i, spurFreq, hChn->actualTunerFreq, notchFreq)));
      retCode = BSAT_g1_P_EnableNotch_isr(h, (BSAT_NotchSelect)i, notchFreq, &(hDev->notchSettings[spur_index[i]]));
      if (retCode)
      {
         BDBG_ERR(("BSAT_g1_P_EnableNotch_isr(%d) error 0x%X", i, retCode));
         goto done;
      }
      hChn->notchState = 0;
   }

   done:
   return retCode;
}


/******************************************************************************
 BSAT_g1_P_UpdateNotch_isr()
******************************************************************************/
BERR_Code BSAT_g1_P_UpdateNotch_isr(BSAT_ChannelHandle h)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   uint32_t val, ctl_reg;
   uint8_t beta, delta, i;

   if ((hChn->notchState < 0) || (hChn->notchState >= 4) || hChn->miscSettings.bDisableNotch)
      return BERR_SUCCESS;

   hChn->notchState++;
   beta = BSAT_Notch_beta[hChn->notchState];
   delta = 0; /* BSAT_Notch_delta[hChn->notchState]; */

   for (i = 0; i < 2; i++)
   {
      if (i == 0)
      {
         ctl_reg = BCHP_STB_CHAN_CHx_NOTCH_0_CTRL;
      }
      else
      {
         ctl_reg = BCHP_STB_CHAN_CHx_NOTCH_1_CTRL;
      }

      val = BSAT_g1_P_ReadRegister_isrsafe(h, ctl_reg);
      if (val & BCHP_STB_CHAN_CH0_NOTCH_0_CTRL_BYP_MASK)
         continue;

      val &= ~0x000FFF00;
      val = ((delta & 0xF) << 16);
      val |= ((beta & 0x1F) << 8);
      BSAT_DEBUG_NOTCH(BDBG_MSG(("notch%d: state=%d, delta=%d, beta=%d", i, hChn->notchState, delta, beta)));
      BSAT_g1_P_WriteRegister_isrsafe(h, ctl_reg, val);
   }
   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_TunerSetFreq_isr()
******************************************************************************/
BERR_Code BSAT_g1_P_TunerSetFreq_isr(BSAT_ChannelHandle h, BSAT_g1_FUNCT nextFunct)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;

   hChn->acqState = BSAT_AcqState_eTuning;

   BSAT_g1_P_DisableAllNotch_isrsafe(h);
   BSAT_g1_P_TunerUpdateActualTunerFreq_isr(h);
   BSAT_g1_P_SetChanDecFcw_isr(h);
   BSAT_g1_P_ChanSetAciCoeff_isr(h);
   BSAT_g1_P_ConfigChanAgc_isr(h, false); /* config agc for acquisition */

   return BSAT_g1_P_EnableTimer_isr(h, BSAT_TimerSelect_eBaudUsec, 1000, nextFunct);
}


/******************************************************************************
 BSAT_g1_P_TunerQuickTune_isr()
******************************************************************************/
BERR_Code BSAT_g1_P_TunerQuickTune_isr(BSAT_ChannelHandle h, BSAT_g1_FUNCT nextFunct)
{
   BSAT_g1_P_TunerUpdateActualTunerFreq_isr(h);
   BSAT_g1_P_SetChanDecFcw_isr(h);
   return nextFunct(h);
}


/******************************************************************************
 BSAT_g1_P_SetAdcSelect() - configures the crossbar to select the ADC for the channel
******************************************************************************/
BERR_Code BSAT_g1_P_SetAdcSelect(BSAT_ChannelHandle h, uint8_t adcSelect)
{
#ifdef BCHP_STB_CHAN_CTRL_XBAR_SEL
   BERR_Code retCode;
   uint32_t xbar, mask, shift;

   if (adcSelect > 3)
   {
      BERR_TRACE(retCode = BERR_INVALID_PARAMETER);
      return retCode;
   }

   xbar = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_STB_CHAN_CTRL_XBAR_SEL);

   if (h->channel < 8)
      shift = h->channel * 4;
   else
      shift = (h->channel - 8) * 4;

   mask = ~(0x7 << shift);
   xbar &= mask;
   xbar |= (((uint32_t)adcSelect & 0x07) << shift);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_STB_CHAN_CTRL_XBAR_SEL, xbar);
   return BERR_SUCCESS;
#else
   BSTD_UNUSED(h);
   if (adcSelect != 0)
      return BERR_NOT_AVAILABLE;
   else
      return BERR_SUCCESS;
#endif
}


/******************************************************************************
 BSAT_g1_P_GetAdcSelect() - returns the crossbar configuration for the channel
******************************************************************************/
BERR_Code BSAT_g1_P_GetAdcSelect(BSAT_ChannelHandle h, uint8_t *pAdcSelect)
{
#ifdef BCHP_STB_CHAN_CTRL_XBAR_SEL
   uint32_t xbar, shift;

   xbar = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_STB_CHAN_CTRL_XBAR_SEL);
   if (h->channel < 8)
      shift = h->channel * 4;
   else
      shift = (h->channel - 8) * 4;
   *pAdcSelect = (uint8_t)(xbar >> shift) & 0x07;
   return BERR_SUCCESS;
#else
   BSTD_UNUSED(h);
   *pAdcSelect = 0; /* only 1 ADC available */
   return BERR_SUCCESS;
#endif
}


/******************************************************************************
 BSAT_g1_ResetXbarFifo()
******************************************************************************/
BERR_Code BSAT_g1_ResetXbarFifo(BSAT_Handle h, uint8_t adcSelect)
{
   BERR_Code retCode;
   BSAT_ChannelHandle hChn = h->pChannels[0];

   if (adcSelect > 3)
   {
      BERR_TRACE(retCode = BERR_INVALID_PARAMETER);
      return retCode;
   }

   BSAT_g1_P_AndRegister_isrsafe(hChn, BCHP_STB_CHAN_CTRL_XBAR_CTRL, ~(1 << adcSelect)); /* toggle adc fifo */
   BSAT_g1_P_OrRegister_isrsafe(hChn, BCHP_STB_CHAN_CTRL_XBAR_CTRL, (1 << adcSelect));   /* turn on adc fifo */

   return BERR_SUCCESS;
}
