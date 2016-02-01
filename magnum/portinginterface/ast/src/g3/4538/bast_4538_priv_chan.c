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
#include "bchp_tm.h"


//#define BAST_DISABLE_DCO_NOTCH


#if (BCHP_CHIP != 4538)
#error "This file is for BCM4538 firmware only (not for host software)"
#endif


/* this file is used only for chips that have wideband tuner */
#ifdef BAST_HAS_WFE

#include "bchp_stb_chan_ctrl.h"
#include "bchp_stb_chan_ch0.h"


BDBG_MODULE(bast_4538_priv_chan);

#define BAST_DEBUG_NOTCH(x) /* x */
static uint8_t BAST_Notch_beta[5] = {8, 9, 10, 11, 12};
/* static uint8_t BAST_Notch_delta[5] = {3, 3, 3, 3, 4}; */


/******************************************************************************
 BAST_g3_P_SetChanDecFcw_isr() - sets CHAN_DEC_FCW_CHx based on Fs and Fc
******************************************************************************/
BERR_Code BAST_g3_P_SetChanDecFcw_isr(BAST_ChannelHandle h)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   uint32_t P_hi, P_lo, Q_hi, Q_lo;

   /* fcw = (2^32)*Fc/Fadc */
   BMTH_HILO_32TO64_Mul(hChn->actualTunerFreq/1000, 2147483648UL, &P_hi, &P_lo);
   BMTH_HILO_64TO64_Div32(P_hi, P_lo, BAST_ADC_FREQ_KHZ>>1, &Q_hi, &Q_lo); /* Fadc/2==2484MHz */
   Q_lo = -Q_lo;  /* program negative fcw for proper spectral inversion */
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_STB_CHAN_CHx_DEC_FCW, &Q_lo);
   /* BDBG_MSG(("BCHP_CHAN_DEC_FCW_CHx: Fc=%u, val=0x%X", hChn->actualTunerFreq, Q_lo)); */
   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_g3_P_ConfigChanAgc_isr()
******************************************************************************/
BERR_Code BAST_g3_P_ConfigChanAgc_isr(BAST_ChannelHandle h, bool bTracking)
{
   uint32_t val, agc_thresh;

   /* agc_chx_byp = 0
      agc_chx_la_byp = 0
      agc_chx_la_frz = 0
      agc_chx_lf_frz = 0
      agc_chx_lf_k1 = 3
    */
   val = (3 << BCHP_STB_CHAN_CH0_AGC_CTRL_AGC_LF_K1_SHIFT) & BCHP_STB_CHAN_CH0_AGC_CTRL_AGC_LF_K1_MASK;
   if (bTracking)
      agc_thresh = 7366246;
   else
      agc_thresh = 1074790;
   val |= agc_thresh;
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_STB_CHAN_CHx_AGC_CTRL, &val);

   val = 0;
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_STB_CHAN_CHx_AGC_LF_INT, &val);
   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_g3_P_TunerPowerUp()
******************************************************************************/
BERR_Code BAST_g3_P_TunerPowerUp(BAST_ChannelHandle h)
{
   uint32_t val;
   uint32_t mask = (1 << h->channel);

   /* turn on the channelizer register clock if not on */
   BAST_g3_P_ReadRegister_isrsafe(h, BCHP_TM_REG_CLK_EN, &val);
   if ((val & BCHP_TM_REG_CLK_EN_CHAN_MASK) == 0)
      BAST_g3_P_OrRegister_isrsafe(h, BCHP_TM_REG_CLK_EN, BCHP_TM_REG_CLK_EN_CHAN_MASK);

   BAST_g3_P_ReadRegister_isrsafe(h, BCHP_STB_CHAN_CTRL_PWRDN, &val);
   val &= ~mask;
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_STB_CHAN_CTRL_PWRDN, &val);

   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_g3_P_TunerPowerDown()
******************************************************************************/
BERR_Code BAST_g3_P_TunerPowerDown(BAST_ChannelHandle h)
{
   uint32_t val;
   uint32_t mask = (1 << h->channel);

   /* check if channelizer already off */
   BAST_g3_P_ReadRegister_isrsafe(h, BCHP_TM_REG_CLK_EN, &val);
   if ((val & BCHP_TM_REG_CLK_EN_CHAN_MASK) == 0)
      return BERR_SUCCESS;

   BAST_g3_P_ReadRegister_isrsafe(h, BCHP_STB_CHAN_CTRL_PWRDN, &val);
   val |= mask;
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_STB_CHAN_CTRL_PWRDN, &val);

   /* turn off the register clock if all channelizers are disabled */
   BAST_g3_P_ReadRegister_isrsafe(h, BCHP_STB_CHAN_CTRL_PWRDN, &val);
#if 0
   if ((val & 0xFF) == 0xFF)
      BAST_g3_P_AndRegister_isrsafe(h, BCHP_TM_REG_CLK_EN, ~BCHP_TM_REG_CLK_EN_CHAN_MASK);
#endif
   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_g3_P_TunerUpdateActualTunerFreq_isr()
******************************************************************************/
BERR_Code BAST_g3_P_TunerUpdateActualTunerFreq_isr(BAST_ChannelHandle h)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;

   hChn->actualTunerFreq = hChn->tunerFreq + hChn->tunerIfStep;
   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_g3_P_TunerSetFreq_isr()
******************************************************************************/
BERR_Code BAST_g3_P_TunerSetFreq_isr(BAST_ChannelHandle h, BAST_g3_FUNCT nextFunct)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   uint32_t mask = (1 << h->channel);
   int16_t *pCoeff;

   hChn->postTuneFunct = nextFunct;

   /* set acq state to tuning */
   if (hChn->acqState == BAST_AcqState_eIdle)
      hChn->acqState = BAST_AcqState_eTuning;

   /* dynamically power up channelizer */
   BAST_g3_P_AndRegister_isrsafe(h, BCHP_STB_CHAN_CTRL_PWRDN, ~mask);

   BAST_g3_P_DisableAllNotch_isr(h);
   BAST_g3_P_TunerUpdateActualTunerFreq_isr(h);
   BAST_g3_P_SetChanDecFcw_isr(h);
   BAST_g3_P_GetChanAciCoeff_isr(h, &pCoeff);
   BAST_g3_P_SetChanAciCoeff_isr(h, pCoeff);
   BAST_g3_P_ConfigChanAgc_isr(h, false); /* config agc for acquisition */

   return BAST_g3_P_EnableTimer_isr(h, BAST_TimerSelect_eBaudUsec, 1000, hChn->postTuneFunct);
}


/******************************************************************************
 BAST_g3_P_TunerInit()
******************************************************************************/
BERR_Code BAST_g3_P_TunerInit(BAST_ChannelHandle h, BAST_g3_FUNCT nextFunct)
{
   BERR_Code retCode;

   BKNI_EnterCriticalSection();
   retCode = BAST_g3_P_TunerInit_isr(h, nextFunct);
   BKNI_LeaveCriticalSection();
   return retCode;
}


/******************************************************************************
 BAST_g3_P_TunerInit_isr()
******************************************************************************/
BERR_Code BAST_g3_P_TunerInit_isr(BAST_ChannelHandle h, BAST_g3_FUNCT nextFunct)
{
   uint32_t mask = (1 << h->channel);

   BAST_g3_P_ToggleBit_isrsafe(h, BCHP_STB_CHAN_CTRL_LOCAL_SW_RESET, mask);
   BAST_g3_P_AndRegister_isrsafe(h, BCHP_STB_CHAN_CTRL_XBAR_CTRL, ~0x0F); /* toggle all adc fifos */
   BAST_g3_P_OrRegister_isrsafe(h, BCHP_STB_CHAN_CTRL_XBAR_CTRL, 0x0F);   /* turn on all adc fifos */

   /* init all channelizers to powered down state */
   BAST_g3_P_OrRegister_isrsafe(h, BCHP_STB_CHAN_CTRL_PWRDN, 0x1FF);

   if (nextFunct)
      return nextFunct(h);
   else
      return BERR_SUCCESS;
}


/******************************************************************************
 BAST_g3_P_TunerQuickTune_isr()
******************************************************************************/
BERR_Code BAST_g3_P_TunerQuickTune_isr(BAST_ChannelHandle h, BAST_g3_FUNCT nextFunct)
{
   BAST_g3_P_TunerUpdateActualTunerFreq_isr(h);
   BAST_g3_P_SetChanDecFcw_isr(h);
   return nextFunct(h);
}


/******************************************************************************
 BAST_g3_P_SetAdcSelect() - configures the crossbar to select the ADC for the channel
******************************************************************************/
BERR_Code BAST_g3_P_SetAdcSelect(BAST_ChannelHandle h, uint8_t adcSelect)
{
   BERR_Code retCode;
   uint32_t xbar, mask;

   if (adcSelect > 3)
   {
      BERR_TRACE(retCode = BERR_INVALID_PARAMETER);
      return retCode;
   }

   BAST_g3_P_ReadRegister_isrsafe(h, BCHP_STB_CHAN_CTRL_XBAR_SEL, &xbar);
   mask = ~(0x3 << (h->channel * 2));
   xbar &= mask;
   xbar |= (((uint32_t)adcSelect & 0x03) << (h->channel * 2));
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_STB_CHAN_CTRL_XBAR_SEL, &xbar);

   /* we dont need to turn on the adc fifo because it has already been turned on in TunerInit
   BAST_g3_P_OrRegister_isrsafe(h, BCHP_STB_CHAN_CTRL_XBAR_CTRL, 1 << adcSelect); */
   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_g3_P_GetAdcSelect() - returns the crossbar configuration for the channel
******************************************************************************/
BERR_Code BAST_g3_P_GetAdcSelect(BAST_ChannelHandle h, uint8_t *pAdcSelect, uint8_t *pNumAdc)
{
   uint32_t xbar;

   BAST_g3_P_ReadRegister_isrsafe(h, BCHP_STB_CHAN_CTRL_XBAR_SEL, &xbar);
   *pAdcSelect = (uint8_t)(xbar >> (h->channel * 2)) & 0x03;
   *pNumAdc = 4;
   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_g3_SetNotchSettings()
******************************************************************************/
BERR_Code BAST_g3_SetNotchSettings(BAST_Handle h, uint8_t numSpurs, BAST_NotchSettings *pSettings)
{
#ifndef BAST_DISABLE_DCO_NOTCH
   BAST_g3_P_Handle *pDev = (BAST_g3_P_Handle*)h->pImpl;
   BAST_g3_P_ChannelHandle *hChn;
   BAST_NotchSettings *p;
   uint8_t i;

   BDBG_ASSERT(h);

   if (numSpurs > BAST_MAX_SPURS)
      return BERR_INVALID_PARAMETER;

   pDev->numSpurs = numSpurs;
   for (i = 0; i < BAST_MAX_SPURS; i++)
   {
      if (i < numSpurs)
      {
         p = &pSettings[i];
         BKNI_Memcpy(&(pDev->spurs[i]), p, sizeof(BAST_NotchSettings));
      }
   }

   if (numSpurs == 0)
   {
      for (i = 0; i < BAST_G3_MAX_CHANNELS; i++)
      {
         if (h->pChannels[i] == NULL)
            continue;

         BAST_g3_P_DisableNotch_isrsafe(h->pChannels[i], BAST_NotchSelect_e0);
         BAST_g3_P_DisableNotch_isrsafe(h->pChannels[i], BAST_NotchSelect_e1);
         hChn = (BAST_g3_P_ChannelHandle *)(h->pChannels[i]->pImpl);
         hChn->notchState = -1;
      }
   }
#endif

   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_g3_GetNotchSettings()
******************************************************************************/
BERR_Code BAST_g3_GetNotchSettings(BAST_Handle h, uint8_t *pNumSpurs, BAST_NotchSettings *pSettings)
{
   BAST_g3_P_Handle *pDev = (BAST_g3_P_Handle*)h->pImpl;
   uint8_t i;

   BDBG_ASSERT(h);
   BDBG_ASSERT(pSettings);

   *pNumSpurs = pDev->numSpurs;
   if (pSettings)
   {
      for (i = 0; i < pDev->numSpurs; i++)
      {
         BKNI_Memcpy(&pSettings[i], &(pDev->spurs[i]), sizeof(BAST_NotchSettings));
      }
   }

   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_g3_P_EnableNotch_isr()
******************************************************************************/
BERR_Code BAST_g3_P_EnableNotch_isr(BAST_ChannelHandle h, BAST_NotchSelect n, int32_t freqHz, BAST_NotchSettings *pSettings)
{
#ifndef BAST_DISABLE_DCO_NOTCH
   uint32_t val, ctl_reg, fcw_reg, freq, P_hi, P_lo, Q_hi, Q_lo;
   uint8_t beta, delta;

   BDBG_ASSERT(h);
   BDBG_ASSERT(n <= BAST_NotchSelect_e1);

   if (n == BAST_NotchSelect_e0)
   {
      ctl_reg = BCHP_STB_CHAN_CHx_NOTCH_0_CTRL;
      fcw_reg = BCHP_STB_CHAN_CHx_NOTCH_0_FCW;
   }
   else
   {
      ctl_reg = BCHP_STB_CHAN_CHx_NOTCH_1_CTRL;
      fcw_reg = BCHP_STB_CHAN_CHx_NOTCH_1_FCW;
   }

   /* set baseband dco fcw = (2^31)*4*Fc/BAST_DEFAULT_SAMPLE_FREQ */
   if (freqHz < 0)
      freq = -freqHz;
   else
      freq = freqHz;
   BMTH_HILO_32TO64_Mul(freq * 4, 2147483648UL, &P_hi, &P_lo);
   BMTH_HILO_64TO64_Div32(P_hi, P_lo, BAST_DEFAULT_SAMPLE_FREQ, &Q_hi, &Q_lo);  /* div by (Fs_adc/2) */
   if (freqHz < 0)
      Q_lo = -Q_lo;
   BAST_g3_P_WriteRegister_isrsafe(h, fcw_reg, &Q_lo);

   /* set notch_ctl */
   beta = BAST_Notch_beta[0];
   delta = 0; /* BAST_Notch_delta[0]; */
   val = ((delta & 0xF) << 16);
   val |= ((beta & 0x1F) << 8);
   val |= (pSettings->alpha & 0x1F);
   BAST_DEBUG_NOTCH(BDBG_MSG(("notch%d: ctl=%08X, fcw=%08X", n, val, Q_lo)));
   BAST_g3_P_WriteRegister_isrsafe(h, ctl_reg, &val);
#endif

   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_g3_P_DisableNotch_isrsafe()
******************************************************************************/
BERR_Code BAST_g3_P_DisableNotch_isrsafe(BAST_ChannelHandle h, BAST_NotchSelect n)
{
#ifndef BAST_DISABLE_DCO_NOTCH
   uint32_t ctl_reg, fcw_reg, int_reg, val;

   BDBG_ASSERT(h);
   BDBG_ASSERT(n <= BAST_NotchSelect_e1);

   if (n == BAST_NotchSelect_e0)
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

   /* freeze and bypass */
   val = 0x30000000;
   BAST_g3_P_WriteRegister_isrsafe(h, ctl_reg, &val);

   /* clear the fcw */
   val = 0;
   BAST_g3_P_WriteRegister_isrsafe(h, fcw_reg, &val);

   /* clear the integrator */
   BAST_g3_P_WriteRegister_isrsafe(h, int_reg, &val);
#endif

   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_g3_P_DisableAllNotch_isr()
******************************************************************************/
BERR_Code BAST_g3_P_DisableAllNotch_isr(BAST_ChannelHandle h)
{
#ifndef BAST_DISABLE_DCO_NOTCH
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;

   BAST_g3_P_DisableNotch_isrsafe(h, BAST_NotchSelect_e0);
   BAST_g3_P_DisableNotch_isrsafe(h, BAST_NotchSelect_e1);
   hChn->notchState = -1;
#endif
   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_g3_P_SetNotch_isr()
******************************************************************************/
BERR_Code BAST_g3_P_SetNotch_isr(BAST_ChannelHandle h)
{
   BERR_Code retCode = BERR_SUCCESS;
#ifndef BAST_DISABLE_DCO_NOTCH
   BAST_g3_P_Handle *hDev = (BAST_g3_P_Handle *)(h->pDevice->pImpl);
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   uint32_t range, min_freq, max_freq, spurFreq;
   int32_t notchFreq;
   uint8_t numSpursFound, i, spur_index[2];

   BAST_g3_P_DisableAllNotch_isr(h);
   if ((hDev->numSpurs == 0) || (hChn->miscCtl & BAST_G3_CONFIG_MISC_CTL_DISABLE_NOTCH))
      return BERR_SUCCESS;

   /* determine if spur is within Fb*Nyquist/2 */
   range = hChn->acqParams.symbolRate;
   if (hChn->acqParams.acq_ctl & BAST_ACQSETTINGS_NYQUIST_20)
      range = (range * 3) / 5;
   else
      range = (range * 27) / 40;
   min_freq = hChn->actualTunerFreq - range;
   max_freq = hChn->actualTunerFreq + range;

   for (i = 0, numSpursFound = 0; (numSpursFound < 2) && (i < hDev->numSpurs); i++)
   {
      spurFreq = hDev->spurs[i].spurFreq;
      if ((spurFreq >= min_freq) && (spurFreq <= max_freq))
      {
         spur_index[numSpursFound] = i;
         numSpursFound++;
      }
   }

   for (i = 0; i < numSpursFound; i++)
   {
      spurFreq = hDev->spurs[spur_index[i]].spurFreq;
      notchFreq = hChn->actualTunerFreq - spurFreq;
      BAST_DEBUG_NOTCH(BDBG_MSG(("notch%d: Fspur=%u, Ftuner=%u, Fnotch=%d", i, spurFreq, hChn->actualTunerFreq, notchFreq)));
      retCode = BAST_g3_P_EnableNotch_isr(h, (BAST_NotchSelect)i, notchFreq, &(hDev->spurs[spur_index[i]]));
      if (retCode)
      {
         BDBG_ERR(("BAST_g3_P_EnableNotch_isr(%d) error 0x%X", i, retCode));
         goto done;
      }
      hChn->notchState = 0;
   }

   done:
#endif
   return retCode;
}


/******************************************************************************
 BAST_g3_P_UpdateNotch_isr()
******************************************************************************/
BERR_Code BAST_g3_P_UpdateNotch_isr(BAST_ChannelHandle h)
{
#ifndef BAST_DISABLE_DCO_NOTCH
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   uint32_t val, ctl_reg;
   uint8_t beta, delta, i;

   if ((hChn->notchState < 0) || (hChn->notchState >= 4) || (hChn->miscCtl & BAST_G3_CONFIG_MISC_CTL_DISABLE_NOTCH))
      return BERR_SUCCESS;

   hChn->notchState++;
   beta = BAST_Notch_beta[hChn->notchState];
   delta = 0; /* BAST_Notch_delta[hChn->notchState]; */

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

      BAST_g3_P_ReadRegister_isrsafe(h, ctl_reg, &val);
      if (val & BCHP_STB_CHAN_CH0_NOTCH_0_CTRL_BYP_MASK)
         continue;

      val &= ~0x000FFF00;
      val = ((delta & 0xF) << 16);
      val |= ((beta & 0x1F) << 8);
      BAST_DEBUG_NOTCH(BDBG_MSG(("notch%d: state=%d, delta=%d, beta=%d", i, hChn->notchState, delta, beta)));
      BAST_g3_P_WriteRegister_isrsafe(h, ctl_reg, &val);
   }
#endif
   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_g3_ResetXbarFifo()
******************************************************************************/
BERR_Code BAST_g3_ResetXbarFifo(BAST_Handle h, uint8_t adcSelect)
{
   BERR_Code retCode;
   BAST_ChannelHandle hChn = h->pChannels[0];

   if (adcSelect > 3)
   {
      BERR_TRACE(retCode = BERR_INVALID_PARAMETER);
      return retCode;
   }

   BAST_g3_P_AndRegister_isrsafe(hChn, BCHP_STB_CHAN_CTRL_XBAR_CTRL, ~(1 << adcSelect)); /* toggle adc fifo */
   BAST_g3_P_OrRegister_isrsafe(hChn, BCHP_STB_CHAN_CTRL_XBAR_CTRL, (1 << adcSelect));   /* turn on adc fifo */

   return BERR_SUCCESS;
}

#endif /* BAST_HAS_WFE */
