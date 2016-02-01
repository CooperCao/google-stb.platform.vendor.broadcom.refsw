/***************************************************************************
 *     Copyright (c) 2005-2013, Broadcom Corporation
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
#include "bwfe.h"
#include "bwfe_priv.h"
#include "bwfe_g2_priv.h"

BDBG_MODULE(bwfe_g2_priv_corr);


#define BWFE_DEBUG_CORR(x) /*x*/


/******************************************************************************
 BWFE_g2_Corr_P_CorrDone_isr() - ISR context
******************************************************************************/
void BWFE_g2_Corr_P_CorrDone_isr(void *p, int param)
{
   BWFE_ChannelHandle h = (BWFE_ChannelHandle)p;
   BWFE_g2_P_ChannelHandle *hChn = (BWFE_g2_P_ChannelHandle *)h->pImpl;
   BWFE_FUNCT funct;

   BSTD_UNUSED(param);
   BWFE_g2_P_DisableTimer(h, BWFE_g2_TimerSelect_e0);

   /* stop correlator, disable and clear interrupt */
   BWFE_P_AndRegister(h, BCHP_WFE_CORE_CORRCTL, ~0x00000002);
   BINT_DisableCallback_isr(hChn->hCorrDoneCb);
   BINT_ClearCallback_isr(hChn->hCorrDoneCb);

   funct = hChn->corrIdleIsr;
   if (funct)
      funct(h);
}


#if !defined(BWFE_EXCLUDE_SPECTRUM_ANALYZER) || !defined(BWFE_EXCLUDE_ANALOG_DELAY)
#define BWFE_DPM_CORR_LEN  134217728   /* rough correlator length = 2^27 */
/******************************************************************************
 BWFE_g2_Corr_P_CalcCorrParams()
******************************************************************************/
BERR_Code BWFE_g2_Corr_P_SetCorrParams(BWFE_ChannelHandle h, uint32_t freqHz, uint32_t binSizeHz)
{
   BWFE_g2_P_Handle *hDev = h->pDevice->pImpl;
   BWFE_g2_P_ChannelHandle *hChn = (BWFE_g2_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode = BERR_SUCCESS;
   uint32_t P_hi, P_lo, Q_hi, Q_lo;
   uint32_t fcw, thr, Q;
   uint8_t maxbitpos = 0;
   
   if (binSizeHz > 0)
   {
      /* calculate FCW = (2^32)*Fc/Fs_adc for SA mode */
      BMTH_HILO_32TO64_Mul(freqHz << 1, 2147483648UL, &P_hi, &P_lo);  /* 2*Fc*(2^31) */
      BMTH_HILO_64TO64_Div32(P_hi, P_lo, (hDev->adcSampleFreqKhz / BWFE_LIC_L) * 1000, &Q_hi, &Q_lo);  /* div by (Fs_adc/LIC_L) */
      fcw = Q_lo;
      
      /* zero threshold for SA mode */
      thr = 0;
      
      /* set correlator duration, correlator clock is Fadc/LIC_L */
      BMTH_HILO_32TO64_Mul(hDev->adcSampleFreqKhz, 4000 * BWFE_LIC_L, &P_hi, &P_lo);
      BMTH_HILO_64TO64_Div32(P_hi, P_lo, binSizeHz, &Q_hi, &Q_lo);   /* corr_len = (4*Fadc*LIC_L) / Fb */
   }
   else
   {
      BWFE_DEBUG_CORR(BDBG_MSG(("QDDFS_M=%d", hDev->dpmQddfsM)));
      BWFE_DEBUG_CORR(BDBG_MSG(("QDDFS_N=%d", hDev->dpmQddfsN)));
      
      BMTH_HILO_32TO64_Mul(2147483648UL, 2, &P_hi, &P_lo);    /* 2^intbw where intbw=32 matches rtl word length */
      BMTH_HILO_64TO64_Div32(P_hi, P_lo, hDev->dpmQddfsN, &Q_hi, &Q_lo);  /* QDDFS_N is positive */
      Q = Q_lo;   /* Q = floor(2^intbw / QDDFS_N) where intbw=32 */
      
      BMTH_HILO_32TO64_Mul(hDev->dpmQddfsM, Q, &P_hi, &P_lo);
      fcw = P_lo;   /* fcw = QDDFS_M * Q */
      
      BMTH_HILO_32TO64_Mul(hDev->dpmQddfsN, Q, &P_hi, &P_lo);
      thr = P_lo;   /* thr = QDDFS_N * Q mod 2^intbw where intbw=32 */
      
      /* corr_len = floor(2^27/N_Q) * N_Q */
      BMTH_HILO_32TO64_Mul(BWFE_DPM_CORR_LEN / hDev->dpmQddfsN, hDev->dpmQddfsN, &Q_hi, &Q_lo);
   }
   
   BWFE_P_WriteRegister(h, BCHP_WFE_CORE_CORRFCW, fcw);
   BWFE_P_WriteRegister(h, BCHP_WFE_CORE_CORRTHR, thr);
   /* BDBG_MSG(("corr: freq=%u, fcw=0x%08X, thresh=0x%08X\n", freqHz, fcw, thr)); */
   
   BWFE_P_WriteRegister(h, BCHP_WFE_CORE_CORRLEN1, Q_hi & 0xF);
   BWFE_P_WriteRegister(h, BCHP_WFE_CORE_CORRLEN0, Q_lo);
   /* BWFE_DEBUG_CORR(BDBG_MSG(("len_hi=0x%08X | len_lo=0x%08X", Q_hi, Q_lo))); */
   
   /* determine 2^k accumulation limit based on corr interval to simplify 64-bit correlator values to 32-bit */
	/* per Pete: for non-power-of-2 intervals, round up to the nearest 2^k */
   if (Q_hi > 0)
   {
      /* corr interval determined by corrlen1 */
      maxbitpos = 32 + BWFE_P_GetMsb(Q_hi);
   }
   else
   {
      /* corr interval determined by corrlen0 */
      maxbitpos = BWFE_P_GetMsb(Q_lo);
   }
   /* BWFE_DEBUG_CORR(BDBG_MSG(("maxbitpos=%d\n", maxbitpos))); */
   hChn->corrMaxBit = 23 + maxbitpos + 1;    /* add 1 to round up */
   
   BWFE_P_WriteRegister(h, BCHP_WFE_CORE_CORRFCWEN, 0x00000000);   /* load fcw and thr */
   BWFE_P_ToggleBit(h, BCHP_WFE_CORE_CORRFCWEN, 0x00000001);
   
   return retCode;
}


/******************************************************************************
 BWFE_g2_Corr_P_StartCorrelator()
******************************************************************************/
BERR_Code BWFE_g2_Corr_P_StartCorrelator(BWFE_ChannelHandle h, uint32_t freqHz, uint32_t binSizeHz, BWFE_FUNCT func)
{
   BWFE_g2_P_ChannelHandle *hChn = (BWFE_g2_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode = BERR_SUCCESS;
   
   /* avoid multiples of 27MHz */
   if (freqHz % (BWFE_XTAL_FREQ_KHZ * 1000 / 2) == 0)
      freqHz += 200000;
   
   hChn->corrFreqHz = freqHz;
   
   /* stop correlator */
   BWFE_P_WriteRegister(h, BCHP_WFE_CORE_CORRCTL, 0x00000000);
   BWFE_P_ToggleBit(h, BCHP_WFE_CORE_CORRCTL, 0x00000001);  /* correlator clear */
   
   /* configure required fcw, thresh, and length depending on mode */
   BWFE_g2_Corr_P_SetCorrParams(h, freqHz, binSizeHz);

   /* clear interrupt and enable */
   BINT_ClearCallback_isr(hChn->hCorrDoneCb);
   BINT_EnableCallback_isr(hChn->hCorrDoneCb);
   hChn->corrIdleIsr = func;

   /* workaround for missing corr done irq at certain corrlen's - execute callback in 50 us */
   retCode = BWFE_g2_P_EnableTimer(h, BWFE_g2_TimerSelect_e0, 50, (BWFE_FUNCT)BWFE_g2_Corr_P_CorrDone_isr);

   /* start correlator */
   BWFE_P_OrRegister(h, BCHP_WFE_CORE_CORRCTL, 0x00000002);
   return retCode;
}


/******************************************************************************
 BWFE_g2_Lic_P_ReadCorrelator()
******************************************************************************/
#if 0
static const uint32_t corrImsbOverride[BWFE_LIC_L1] = {0x00000021, 0x0FFFFFD9, 0x00000026, 0x0FFFFFD9, 0x00000024, 0x0FFFFFDC, 0x00000021, 0x0FFFFFE3};
static const uint32_t corrIlsbOverride[BWFE_LIC_L1] = {0xC57626D8, 0x5380DA40, 0x89C5CB98, 0xA0A430D0, 0x9E303174, 0x91FFDFE0, 0x0CD5C07C, 0x82187188};
static const uint32_t corrQmsbOverride[BWFE_LIC_L1] = {0x0FFFFFEF, 0x00000007, 0x0FFFFFFD, 0x0FFFFFFC, 0x00000007, 0x0FFFFFEF, 0x00000013, 0x0FFFFFE6};
static const uint32_t corrQlsbOverride[BWFE_LIC_L1] = {0x8FD70FC8, 0x913F7444, 0x20EB92B4, 0x617723B8, 0x15341AF8, 0x0E8ADE4C, 0xFF1E6A0C, 0x5E4A2868};
#endif
BERR_Code BWFE_g2_Corr_P_ReadCorrelator(BWFE_ChannelHandle h, uint8_t slice, uint32_t regMsb, uint32_t regLsb, int32_t *corr)
{
   BWFE_g2_P_ChannelHandle *hChn = (BWFE_g2_P_ChannelHandle *)h->pImpl;
   uint32_t corrMsb, corrLsb;
   
   BWFE_P_ReadSliceRegister(h, slice, regLsb, &corrLsb);    /* lsb32 */
   BWFE_P_ReadSliceRegister(h, slice, regMsb, &corrMsb);    /* msb28 */
   
#if 0
   /* override correlator with test values */
   if (regMsb == BCHP_WFE_CORE_CORRI1_DMX0_PI_SLC)
      corrMsb = corrImsbOverride[slice];
   else if (regMsb == BCHP_WFE_CORE_CORRI1_DMX0_PO_SLC)
      corrMsb = corrImsbOverride[4+slice];
   else if (regMsb == BCHP_WFE_CORE_CORRQ1_DMX0_PI_SLC)
      corrMsb = corrQmsbOverride[slice];
   else if (regMsb == BCHP_WFE_CORE_CORRQ1_DMX0_PO_SLC)
      corrMsb = corrQmsbOverride[4+slice];
   else
      corrMsb = 0;

   if (regLsb == BCHP_WFE_CORE_CORRI0_DMX0_PI_SLC)
      corrLsb = corrIlsbOverride[slice];
   else if (regLsb == BCHP_WFE_CORE_CORRI0_DMX0_PO_SLC)
      corrLsb = corrIlsbOverride[4+slice];
   else if (regLsb == BCHP_WFE_CORE_CORRQ0_DMX0_PI_SLC)
      corrLsb = corrQlsbOverride[slice];
   else if (regLsb == BCHP_WFE_CORE_CORRQ0_DMX0_PO_SLC)
      corrLsb = corrQlsbOverride[4+slice];
   else
      corrLsb = 0;
   /*BKNI_Printf("read corr: %08X %08X\n", corrMsb, corrLsb);*/
#endif

   /* take 32 MSB based on 2^k accumulation limit */
	if (hChn->corrMaxBit >= 32)
      *corr = (int32_t)((corrMsb << (63 - hChn->corrMaxBit)) | (corrLsb >> (hChn->corrMaxBit - 31)));
	else
      *corr = (int32_t)(corrLsb << (31 - hChn->corrMaxBit));
   
   /* BWFE_DEBUG_CORR(BDBG_MSG(("corr:MSB=%08X|LSB=%08X-(%d)->%08X", corrMsb, corrLsb, hChn->corrMaxBit, *corr))); */
   return BERR_SUCCESS;
}


/******************************************************************************
 BWFE_g2_Corr_P_GetCorrVals()
******************************************************************************/
BERR_Code BWFE_g2_Corr_P_GetCorrVals(BWFE_ChannelHandle h, int32_t *corrI, int32_t *corrQ)
{
   uint32_t slice;
   
   for (slice = 0; slice < BWFE_NUM_SLICES; slice++)
   {
      /* read demux0 correlators */
      BWFE_g2_Corr_P_ReadCorrelator(h, slice, BCHP_WFE_CORE_CORRI1_DMX0_PI_SLC, BCHP_WFE_CORE_CORRI0_DMX0_PI_SLC, &corrI[slice]);
      BWFE_g2_Corr_P_ReadCorrelator(h, slice, BCHP_WFE_CORE_CORRQ1_DMX0_PI_SLC, BCHP_WFE_CORE_CORRQ0_DMX0_PI_SLC, &corrQ[slice]);
      BWFE_g2_Corr_P_ReadCorrelator(h, slice, BCHP_WFE_CORE_CORRI1_DMX0_PO_SLC, BCHP_WFE_CORE_CORRI0_DMX0_PO_SLC, &corrI[slice + BWFE_NUM_SLICES]);
      BWFE_g2_Corr_P_ReadCorrelator(h, slice, BCHP_WFE_CORE_CORRQ1_DMX0_PO_SLC, BCHP_WFE_CORE_CORRQ0_DMX0_PO_SLC, &corrQ[slice + BWFE_NUM_SLICES]);
      
      /* read demux1 correlators */
      BWFE_g2_Corr_P_ReadCorrelator(h, slice, BCHP_WFE_CORE_CORRI1_DMX1_PI_SLC, BCHP_WFE_CORE_CORRI0_DMX1_PI_SLC, &corrI[slice + BWFE_NUM_SLICES * 2]);
      BWFE_g2_Corr_P_ReadCorrelator(h, slice, BCHP_WFE_CORE_CORRQ1_DMX1_PI_SLC, BCHP_WFE_CORE_CORRQ0_DMX1_PI_SLC, &corrQ[slice + BWFE_NUM_SLICES * 2]);
      BWFE_g2_Corr_P_ReadCorrelator(h, slice, BCHP_WFE_CORE_CORRI1_DMX1_PO_SLC, BCHP_WFE_CORE_CORRI0_DMX1_PO_SLC, &corrI[slice + BWFE_NUM_SLICES * 3]);
      BWFE_g2_Corr_P_ReadCorrelator(h, slice, BCHP_WFE_CORE_CORRQ1_DMX1_PO_SLC, BCHP_WFE_CORE_CORRQ0_DMX1_PO_SLC, &corrQ[slice + BWFE_NUM_SLICES * 3]);
   }
   
   return BERR_SUCCESS;
}
#endif


#ifndef BWFE_EXCLUDE_ANALOG_DELAY
/******************************************************************************
 BWFE_g2_Corr_P_CalcDelay() - computes delay & gain imbalance per lane from I,Q correlators
 
 correlator I format msb28 + lsb32 TC37.23
   BCHP_WFE_CORE_CORRI1_DMX0_PI_SLC | BCHP_WFE_CORE_CORRI0_DMX0_PI_SLC
   BCHP_WFE_CORE_CORRI1_DMX0_PO_SLC | BCHP_WFE_CORE_CORRI0_DMX0_PO_SLC
   BCHP_WFE_CORE_CORRI1_DMX1_PI_SLC | BCHP_WFE_CORE_CORRI0_DMX1_PI_SLC
   BCHP_WFE_CORE_CORRI1_DMX1_PO_SLC | BCHP_WFE_CORE_CORRI0_DMX1_PO_SLC
 
 correlator Q format msb28 + lsb32 TC37.23
   BCHP_WFE_CORE_CORRQ1_DMX0_PI_SLC | BCHP_WFE_CORE_CORRQ0_DMX0_PI_SLC
   BCHP_WFE_CORE_CORRQ1_DMX0_PO_SLC | BCHP_WFE_CORE_CORRQ0_DMX0_PO_SLC
   BCHP_WFE_CORE_CORRQ1_DMX1_PI_SLC | BCHP_WFE_CORE_CORRQ0_DMX1_PI_SLC
   BCHP_WFE_CORE_CORRQ1_DMX1_PO_SLC | BCHP_WFE_CORE_CORRQ0_DMX1_PO_SLC
******************************************************************************/
BERR_Code BWFE_g2_Corr_P_CalcDelay(BWFE_ChannelHandle h)
{
   BERR_Code retCode = BERR_SUCCESS;
   BWFE_g2_P_Handle *hDev = h->pDevice->pImpl;
   BWFE_g2_P_ChannelHandle *hChn = (BWFE_g2_P_ChannelHandle *)h->pImpl;
   uint32_t P_hi, P_lo, Q_hi, Q_lo;
   int32_t corrI[BWFE_LIC_L], corrQ[BWFE_LIC_L], gain[BWFE_LIC_L], angle[BWFE_LIC_L];
   int32_t phase, vec[2];
   uint8_t k;
   int8_t tmp;
   bool bNegative = false;
   
   /* read all correlator values: 4 slices x 2 dmx x 2 lanes = LIC_L = 16 */
   BWFE_g2_Corr_P_GetCorrVals(h, (int32_t *)corrI, (int32_t *)corrQ);
   hChn->bCorrInProgress = false;   /* finished with correlator */
   
   /* disable DPM tone since correlator done */
   if ((hChn->corrDpmDebug & 0x6) == 0)
      BWFE_g2_Rffe_P_DisableDpmMode(h);

#if 0
   for (k = 0; k < BWFE_LIC_L; k++)
   {
      BKNI_Printf("'%08X'", corrI[k]);
      if (k < BWFE_LIC_L - 1)
         BKNI_Printf(", ");
   }
   BKNI_Printf("\n");
   for (k = 0; k < BWFE_LIC_L; k++)
   {
      BKNI_Printf("'%08X'", corrQ[k]);
      if (k < BWFE_LIC_L - 1)
         BKNI_Printf(", ");
   }
   BKNI_Printf("\n");
#endif
   
   for (k = 0; k < BWFE_LIC_L; k++)
   {
      vec[0] = corrI[k];
      vec[1] = corrQ[k];
      phase = 0;
      
      /* calculate gain and angle using cordic */
      BWFE_P_CordicVectorMode(vec, &phase);
      gain[k] = vec[0];
      angle[k] = phase; /* scaled by 2^32 / 2*PI */
   }
   
   for (k = 0; k < BWFE_LIC_L; k++)
   {
      /* do scaling based on specified reference */
      if (hChn->corrRefSlice == 0)
         tmp = k;
      else
         tmp = (k - hChn->corrRefSlice) % BWFE_LIC_L;
      
      /* use specified reference for relative gain and angle offsets */
      if (k != hChn->corrRefSlice)
      {
         /* calculate phase = (Fc/Fs_adc) * k * 2 */
         BMTH_HILO_32TO64_Mul((hDev->dpmPilotFreqKhz << 1) * BWFE_ABS(tmp), 2147483648UL, &P_hi, &P_lo); /* Fc scaled up by 2^31 * k * 2 */
         BMTH_HILO_64TO64_Div32(P_hi, P_lo, hDev->adcSampleFreqKhz, &Q_hi, &Q_lo);           /* div by Fs_adc */
         phase = (int32_t)Q_lo;
         
         if (tmp < 0)
            phase = -phase;   /* recover sign */
         
         angle[k] += phase;   /* add phase due to sample delays */
         angle[k] -= angle[hChn->corrRefSlice];   /* get phase offset relative to reference */
      }
   }
   
   /* use specified reference for relative gain and angle offsets */
   hChn->corrAgc[hChn->corrRefSlice] = (uint32_t)(BWFE_AGC_SCALE);
   hChn->corrDelay[hChn->corrRefSlice] = 0;
   
   for (k = 1; k < BWFE_LIC_L; k++)
   {
   #if 0
      /* calculate agc[k] = gain[hChn->corrRefSlice] / gain[k] */
      BMTH_HILO_32TO64_Mul(BWFE_AGC_SCALE, gain[0], &P_hi, &P_lo);  /* gain[0] scaled by agc_scale */
      BMTH_HILO_64TO64_Div32(P_hi, P_lo, gain[k], &Q_hi, &Q_lo);    /* div by gain[k] */
      h->corrAgc[k] = Q_lo;
   #endif

      /* remove sign from angle since extended precision is unsigned */
      /*BKNI_Printf("angle[%d]=%08X -> ", k, angle[k]);*/
      if (angle[k] < 0)
      {
         bNegative = true;
         angle[k] = -angle[k];
      }
      else
         bNegative = false;
      
      /* calculate delay[k] = angle[k] * Fs_adc / (2 * Fc) */ /* TBD optional scaling for angle[k] */
      BMTH_HILO_32TO64_Mul((uint32_t)angle[k], hDev->adcSampleFreqKhz, &P_hi, &P_lo);  /* angle[k] * Fs_adc */
      BMTH_HILO_64TO64_Div32(P_hi, P_lo, hDev->dpmPilotFreqKhz << 1, &Q_hi, &Q_lo);    /* div by 2Fc */
      hChn->corrDelay[k] = (int32_t)Q_lo; /* TBD check for overflow */
      
      /* recover sign */
      if (bNegative)
         hChn->corrDelay[k] = -(hChn->corrDelay[k]);

      /* BKNI_Printf("corrDelay[%d]=%08X\n", k, hChn->corrDelay[k]); */
      /* BKNI_Printf("k=%d: AGC=%08X, DLY=0x%08X (%d)\n", k, hChn->corrAgc[k], hChn->corrDelay[k], hChn->corrDelay[k]); */
   }
   
#if 0
   /* TBD estimate delay across slices: [(D1 + D3) - (D0 + D2)] / 2 */
   hChn->sliceDelay[0] = ((hChn->corrDelay[1] + hChn->corrDelay[3]) - (hChn->corrDelay[0] + hChn->corrDelay[2])) >> 1;
   
   /* freeze and program LF AGC based on correlator results */
   BWFE_P_OrSliceReg(h, slice, BCHP_WFE_CORE_NRAGCCTL_PI_SLC, 0x00000004);
   BWFE_P_OrSliceReg(h, slice, BCHP_WFE_CORE_NRAGCCTL_PO_SLC, 0x00000004);
   BWFE_P_WriteSliceRegister(h, slice, BCHP_WFE_CORE_REG_NR_AGC_LF_INT_WDATA_PI_SLC, hChn->corrAgc[slice][0]);
   BWFE_P_WriteSliceRegister(h, slice, BCHP_WFE_CORE_REG_NR_AGC_LF_INT_WDATA_PO_SLC, hChn->corrAgc[slice][1]);
#endif

   /* continue with next function */
   if (hChn->postCalcDelayFunct != NULL)
      retCode = hChn->postCalcDelayFunct(h);
   
   return retCode;
}


/******************************************************************************
 BWFE_g2_Corr_P_AccumulateOffset() - ISR context
******************************************************************************/
BERR_Code BWFE_g2_Corr_P_AccumulateOffset(BWFE_ChannelHandle h)
{
   BWFE_g2_P_Handle *hDev = h->pDevice->pImpl;
   BWFE_g2_P_ChannelHandle *hChn = (BWFE_g2_P_ChannelHandle *)h->pImpl;
   uint32_t P_hi, P_lo, Q_hi, Q_lo;
   uint32_t avg_hi, avg_lo;
   int32_t avg, offset, delay[BWFE_NUM_SLICES];
   uint8_t k;
   bool bNegative = false;
   
   avg_hi = avg_lo = 0;
   
   /* calculate average delay */
   for (k = 0; k < BWFE_NUM_SLICES; k++)
   {
      /* sign extension */
      P_lo = hChn->corrDelay[k];
      if (hChn->corrDelay[k] < 0)
         P_hi = 0xFFFFFFFF;
      else
         P_hi = 0;
      
      /* accumulate for average */
      BMTH_HILO_64TO64_Add(avg_hi, avg_lo, P_hi, P_lo, &avg_hi, &avg_lo);
   }
   
   /* remove sign since 64-bit div is unsigned */
   if ((int32_t)avg_hi < 0)
   {
      bNegative = true;
      BMTH_HILO_64TO64_Neg(avg_hi, avg_lo, &avg_hi, &avg_lo);
   }
   else
      bNegative = false;
   
   /* divide to get average */
   BMTH_HILO_64TO64_Div32(avg_hi, avg_lo, BWFE_NUM_SLICES, &Q_hi, &Q_lo);
   avg = (int32_t)Q_lo;
   
   /* recover sign for average */
   if (bNegative)
      avg = -avg;
   
   for (k = 0; k < BWFE_NUM_SLICES; k++)
   {
      offset = hChn->corrDelay[k] - avg;
      if (offset < 0)
      {
         bNegative = true;
         offset = -offset;
      }
      else
         bNegative = false;
      
      /* offset = beta * (corrDelay - avg) / Fs_adc * 1.0e12 */
      BMTH_HILO_32TO64_Mul(offset, hChn->corrDelayBeta * 1000000, &P_hi, &P_lo);  /* (corrDelay - avg) * 1.0e6 * beta(in 0.05 steps) */
      BMTH_HILO_64TO64_Div32(P_hi, P_lo, (hDev->adcSampleFreqKhz / 1000) * 3, &Q_hi, &Q_lo);  /* div by Fs_adc in MHz, div by 0.15 */
      delay[k] = (Q_hi << 5) | (Q_lo >> 27); /* undo cordic phase scaling, keep 4-bit frac delay */
      if (bNegative)
         delay[k] = -delay[k];
      
      delay[k] += hChn->analogDelay[k];   /* accumulate offset to previous delay */
      /*BKNI_Printf("{%08X} delay[%d]=%08X\n", hChn->analogDelay[k], k, delay[k]);*/
      
      /* restrict delay to boundaries */
      if (delay[k] > (80 << 4)) delay[k] = (80 << 4);
      if (delay[k] < 0) delay[k] = 0;
      
      /* store current delay for next iteration */
      hChn->analogDelay[k] = delay[k];
   }
   
   /* analog delay compensation */
   return BWFE_g2_Corr_P_CompensateDelay(h, (int32_t *)delay);
}


/******************************************************************************
 BWFE_g2_Corr_P_CompensateDelay() - ISR context
******************************************************************************/
BERR_Code BWFE_g2_Corr_P_CompensateDelay(BWFE_ChannelHandle h, int32_t *delay)
{
#if (BCHP_CHIP==4538)
   BERR_Code retCode = BERR_SUCCESS;
   uint32_t val;
   uint8_t k, delay_c[BWFE_NUM_SLICES], delay_f[BWFE_NUM_SLICES];
   
   for (k = 0; k < BWFE_NUM_SLICES; k++)
   {
      delay_c[k] = delay[k] >> 4;   /* extract coarse delay */
      delay_f[k] = delay[k] & 0xF;  /* extract fine delay */
      /*BKNI_Printf("delay_c=%d, delay_f=%d\n", delay_c[k], delay_f[k]);*/
   }
   
   /* set analog delays */
   val = (delay_c[3] << 24) | (delay_c[2] << 16) | (delay_c[1] << 8) | delay_c[0];
   BWFE_P_WriteRegister(h, BCHP_WFE_ANA_WBADC_TIMING_CNTL_IN, val);     /* coarse delays */
   val = (delay_f[3] << 24) | (delay_f[2] << 16) | (delay_f[1] << 8) | delay_f[0];
   BWFE_P_WriteRegister(h, BCHP_WFE_ANA_WBADC_TIMING_CNTL1_IN, val);    /* fine delays */
   
   BWFE_P_ToggleBit(h, BCHP_WFE_ANA_WBADC_TIMING_CNTL_IN, 0x80808080);  /* latch in delays */
   BWFE_P_ToggleBit(h, BCHP_WFE_ANA_WBADC_TIMING_CNTL1_IN, 0x80808080); /* latch in fine delays */
   
   return retCode;
#else
   return BERR_NOT_SUPPORTED;
#endif
}
#endif


#ifndef BWFE_EXCLUDE_SPECTRUM_ANALYZER
/******************************************************************************
 BWFE_g2_Corr_P_ScanSpectrum() - task context
******************************************************************************/
BERR_Code BWFE_g2_Corr_P_ScanSpectrum(BWFE_ChannelHandle h)
{
   BERR_Code retCode = BERR_SUCCESS;
   BWFE_g2_P_Handle *hDev = h->pDevice->pImpl;
   BWFE_g2_P_ChannelHandle *hChn = (BWFE_g2_P_ChannelHandle *)h->pImpl;
   
#if 0
   /* TBD debug */
   h->saParams.numSamples = 1;
   h->saParams.freqStartHz = 1076250000;
   h->saParams.freqStopHz = 1353750000;
#endif
   
   /* calculate freq step size */
   if (hChn->saParams.numSamples > 1)
      hDev->saStep = (hChn->saParams.freqStopHz - hChn->saParams.freqStartHz) / (hChn->saParams.numSamples - 1);
   
   BDBG_MSG(("freqStart: %d", hChn->saParams.freqStartHz));
   BDBG_MSG(("freqStop: %d", hChn->saParams.freqStopHz));
   BDBG_MSG(("stepSize: %d", hDev->saStep));
   BDBG_MSG(("#samples: %d", hChn->saParams.numSamples));
   BDBG_MSG(("#sweeps: %d", hChn->saParams.numSweeps));

   /* select input to correlator */
   BWFE_P_WriteRegister(h, BCHP_WFE_CORE_CORRINSEL, hChn->corrInputSelect & 0x3);
   
   /* clear sa samples */
   BKNI_Memset((void*)hDev->saSamples, 0, BWFE_SA_MAX_SAMPLES);
   
   hDev->saSweep = hChn->saParams.numSweeps;
   hDev->saState = 0;
   
   /* enter spectrum scan in isr context */
   BKNI_EnterCriticalSection();
   retCode = BWFE_g2_P_EnableTimer(h, BWFE_g2_TimerSelect_e1, 5, BWFE_g2_Corr_P_ScanSpectrum1);
   BKNI_LeaveCriticalSection();
   
   return retCode;
}


/******************************************************************************
 BWFE_g2_Corr_P_ScanSpectrum1() - ISR context
******************************************************************************/
BERR_Code BWFE_g2_Corr_P_ScanSpectrum1(BWFE_ChannelHandle h)
{
   BERR_Code retCode = BERR_SUCCESS;
   BWFE_g2_P_Handle *hDev = h->pDevice->pImpl;
   BWFE_g2_P_ChannelHandle *hChn = (BWFE_g2_P_ChannelHandle *)h->pImpl;
   uint32_t corrPowerDb, prevPower, i;
   int32_t corrI[BWFE_LIC_L], corrQ[BWFE_LIC_L];
   
   while (1)
   {
      /* BDBG_MSG(("BWFE_Corr_P_ScanSpectrum(): state=%d, sweep=%d, idx=%d, freq=%u", hDev-saState, hDev->saSweep, hDev->saIdx, hDev->saFreq)); */
      switch (hDev->saState)
      {
         case 0:
            if (hDev->saSweep == 0)
            {
               /* set event to notify host when done */
               BKNI_SetEvent(hDev->hSaDoneEvent);
               return retCode;
            }
            
            /* perform remaining sweeps */
            hDev->saIdx = 0;
            hDev->saFreq = hChn->saParams.freqStartHz;
            hDev->saState = 1;
            break;
            
         case 1:
            if (hDev->saFreq <= hChn->saParams.freqStopHz)
            {
               if (hChn->bCorrInProgress)
               {
                  /* wait for correlator if in use */
                  return BWFE_g2_P_EnableTimer(h, BWFE_g2_TimerSelect_e1, 5000, BWFE_g2_Corr_P_ScanSpectrum1);
               }
               
               hDev->saState = 2;
               hChn->bCorrInProgress = true;
               return BWFE_g2_Corr_P_StartCorrelator(h, hDev->saFreq, hDev->saStep, BWFE_g2_Corr_P_ScanSpectrum1);
            }
            
            /* decrement sweeps remaining */
            hDev->saSweep--;
            hDev->saState = 0;
            break;
            
         case 2:
            /* read all correlator values: 4 slices x 2 dmx x 2 lanes = LIC_L = 16 */
            BWFE_g2_Corr_P_GetCorrVals(h, (int32_t *)corrI, (int32_t *)corrQ);
            hChn->bCorrInProgress = false;   /* finished with correlator */
            
            /* fixed point power calculations */
            BWFE_P_CalcCorrPower(h, (int32_t *)corrI, (int32_t *)corrQ, &corrPowerDb);
            
            /* save power value to array */
            if ((hDev->saSweep != hChn->saParams.numSweeps) && (hChn->saParams.numSweeps > 1))
            {
               /* calculate cumulative moving average if not the first sweep and more than one sweep */
               i = hChn->saParams.numSweeps - hDev->saSweep;
               prevPower = hDev->saSamples[hDev->saIdx];
               hDev->saSamples[hDev->saIdx] = (corrPowerDb + i * prevPower) / (i + 1);
               /*BDBG_MSG(("%d: %d*%08X|%08X->%08X", hDev->saIdx, i, prevPower, corrPowerDb, h->pDevice->saSamples[hDev->saIdx]));*/
            }
            else
            {
               /* case if one sweep only or initial sweep */
               hDev->saSamples[hDev->saIdx] = corrPowerDb;
               /*BDBG_MSG(("* %d: %08X", hDev->saIdx, h->pDevice->saSamples[hDev->saIdx]));*/
            }
            
            hDev->saState = 1;
            hDev->saFreq += hDev->saStep;
            hDev->saIdx++;
            break;
         
         default:
            BDBG_ERR(("invalid state"));
            BERR_TRACE(retCode = BERR_UNKNOWN);
            break;
      }
   }
}
#endif

