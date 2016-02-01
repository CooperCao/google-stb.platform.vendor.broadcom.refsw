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
#include "bmth.h"
#include "bast.h"
#include "bast_priv.h"
#include "bast_g3_priv.h"


/* this file is used only for chips that have wideband tuner */
#ifdef BAST_HAS_WFE

#include "bchp_chan.h"

BDBG_MODULE(bast_4528_priv_chan);


/* virtual channelizer registers for use in BAST_g3_P_GetChanAddress() */
#define BCHP_CHAN_DEC_FCW_CHx BCHP_CHAN_DEC_FCW_CH0
#define BCHP_CHAN_ACI_CHx_H0 BCHP_CHAN_ACI_CH0_H0
#define BCHP_CHAN_AGC_CTRL_CHx BCHP_CHAN_AGC_CTRL_CH0
#define BCHP_CHAN_AGC_CHx_LA_INT BCHP_CHAN_AGC_CH0_LA_INT
#define BCHP_CHAN_AGC_CHx_LF_INT BCHP_CHAN_AGC_CH0_LF_INT


/******************************************************************************
 BAST_g3_P_SetChanDecFcw_isr() - sets CHAN_DEC_FCW_CHx based on Fs and Fc
******************************************************************************/
BERR_Code BAST_g3_P_SetChanDecFcw_isr(BAST_ChannelHandle h)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode;
   uint32_t addr, P_hi, P_lo, Q_hi, Q_lo;

   retCode = BAST_g3_P_GetChanAddress(h, BCHP_CHAN_DEC_FCW_CHx, &addr);
   BDBG_ASSERT(retCode == BERR_SUCCESS);

   /* fcw = (2^32)*Fc/Fadc */
   BMTH_HILO_32TO64_Mul(hChn->actualTunerFreq/1000, 2147483648UL, &P_hi, &P_lo);
   BMTH_HILO_64TO64_Div32(P_hi, P_lo, BAST_ADC_FREQ_KHZ>>1, &Q_hi, &Q_lo); /* Fadc/2==2484MHz */
   BAST_g3_P_WriteRegister_isrsafe(h, addr, &Q_lo);
   /* BDBG_MSG(("BCHP_CHAN_DEC_FCW_CHx: Fc=%u, val=0x%X", hChn->actualTunerFreq, Q_lo)); */
   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_g3_P_SetChanAciCoeff_isr()
******************************************************************************/
BERR_Code BAST_g3_P_SetChanAciCoeff_isr(BAST_ChannelHandle h, int16_t *pCoeff)
{
   uint32_t i, addr, val;

   BDBG_ASSERT(BAST_g3_P_GetChanAddress(h, BCHP_CHAN_ACI_CHx_H0, &addr) == BERR_SUCCESS);
   for (i = 0; i < 65; i++)
   {
      val = (uint32_t)pCoeff[i];
      BAST_g3_P_WriteRegister_isrsafe(h, addr, &val);
      addr += 4;
   }
   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_g3_P_GetChanAciCoeff_isr() - returns a pointer to an array of 65 coefficients
******************************************************************************/
BERR_Code BAST_g3_P_GetChanAciCoeff_isr(BAST_ChannelHandle h, int16_t **p)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   uint32_t Fb_mbaud = (hChn->acqParams.symbolRate + 500000) / 1000000;
   extern int16_t* BAST_AciCoeff_Table[];

   /* just do this for now */
   if (Fb_mbaud < 1)
      Fb_mbaud = 1;
   else if (Fb_mbaud > 45)
      Fb_mbaud = 45;
   *p = (int16_t *)BAST_AciCoeff_Table[Fb_mbaud-1];
   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_g3_P_ConfigChanAgc_isr()
******************************************************************************/
BERR_Code BAST_g3_P_ConfigChanAgc_isr(BAST_ChannelHandle h, bool bTracking)
{
   uint32_t agc_ctrl_addr, agc_lf_int_addr, val, agc_thresh;

   BDBG_ASSERT(BAST_g3_P_GetChanAddress(h, BCHP_CHAN_AGC_CTRL_CHx, &agc_ctrl_addr) == BERR_SUCCESS);
   BDBG_ASSERT(BAST_g3_P_GetChanAddress(h, BCHP_CHAN_AGC_CHx_LF_INT, &agc_lf_int_addr) == BERR_SUCCESS);

   /* agc_chx_byp = 0
      agc_chx_la_byp = 0
      agc_chx_la_frz = 0
      agc_chx_lf_frz = 0
      agc_chx_lf_k1 = 3
    */
   val = (3 << BCHP_CHAN_AGC_CTRL_CH0_AGC_CH0_LF_K1_SHIFT) & BCHP_CHAN_AGC_CTRL_CH0_AGC_CH0_LF_K1_MASK;
   if (bTracking)
      agc_thresh = 7366246;
   else
      agc_thresh = 1074790;
   val |= (agc_thresh & 0xFFFFF);
   BAST_g3_P_WriteRegister_isrsafe(h, agc_ctrl_addr, &val);

   val = 0;
   BAST_g3_P_WriteRegister_isrsafe(h, agc_lf_int_addr, &val);
   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_g3_P_TunerConfigLna()
******************************************************************************/
BERR_Code BAST_g3_P_TunerConfigLna(BAST_Handle h, BAST_TunerLnaSettings *pSettings)
{
   BSTD_UNUSED(h);
   BSTD_UNUSED(pSettings);
   return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BAST_g3_P_GetTunerLnaStatus()
******************************************************************************/
BERR_Code BAST_g3_P_TunerGetLnaStatus(BAST_ChannelHandle h, BAST_TunerLnaStatus *pStatus)
{
   BSTD_UNUSED(h);
   BSTD_UNUSED(pStatus);
   return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BAST_g3_P_TunerPowerUp()
******************************************************************************/
BERR_Code BAST_g3_P_TunerPowerUp(BAST_ChannelHandle h)
{
   uint32_t val;
   uint32_t mask = (1 << h->channel);

   BAST_g3_P_ReadRegister_isrsafe(h, BCHP_CHAN_SHUTOFF, &val);
   val &= ~mask;
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_CHAN_SHUTOFF, &val);
   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_g3_P_TunerPowerDown()
******************************************************************************/
BERR_Code BAST_g3_P_TunerPowerDown(BAST_ChannelHandle h)
{
   uint32_t val;
   uint32_t mask = (1 << h->channel);

   BAST_g3_P_ReadRegister_isrsafe(h, BCHP_CHAN_SHUTOFF, &val);
   val |= mask;
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_CHAN_SHUTOFF, &val);
   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_g3_P_TunerGetLockStatus_isrsafe()
******************************************************************************/
BERR_Code BAST_g3_P_TunerGetLockStatus_isrsafe(BAST_ChannelHandle h, bool *bRefPllLocked, bool *bMixPllLocked)
{
   BSTD_UNUSED(h);

   /* TBD */
   *bRefPllLocked = true;
   *bMixPllLocked = true;
   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_g3_P_TunerSetFilter_isr()
******************************************************************************/
BERR_Code BAST_g3_P_TunerSetFilter_isr(BAST_ChannelHandle h, bool bTracking)
{
   BSTD_UNUSED(h);
   BSTD_UNUSED(bTracking);
   /* TBD */
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
   int16_t *pCoeff;

   hChn->postTuneFunct = nextFunct;

   /* set acq state to tuning */
   if (hChn->acqState == BAST_AcqState_eIdle)
      hChn->acqState = BAST_AcqState_eTuning;

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
   uint32_t val;

   if (h->channel == 0)
   {
      /* channelizer reset */
      val = 1;
      BAST_g3_P_WriteRegister_isrsafe(h, BCHP_CHAN_LOCAL_SW_RESET, &val);
      val = 0;
      BAST_g3_P_WriteRegister_isrsafe(h, BCHP_CHAN_LOCAL_SW_RESET, &val);

      /* start channelizer */
      val = 1;
      BAST_g3_P_WriteRegister_isrsafe(h, BCHP_CHAN_START_CHAN, &val);
   }

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
BERR_Code BAST_g3_P_SetAdcSelect(BAST_ChannelHandle h, BAST_AdcSelect adcSelect)
{
   BERR_Code retCode;

   if (adcSelect > BAST_AdcSelect_e0)
   {
      BERR_TRACE(retCode = BERR_INVALID_PARAMETER);
      return retCode;
   }

   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_g3_P_GetAdcSelect() - returns the crossbar configuration for the channel
******************************************************************************/
BERR_Code BAST_g3_P_GetAdcSelect(BAST_ChannelHandle h, BAST_AdcSelect *pAdcSelect)
{
   *pAdcSelect = BAST_AdcSelect_e0;
   return BERR_SUCCESS;
}

#endif /* BAST_HAS_WFE */
