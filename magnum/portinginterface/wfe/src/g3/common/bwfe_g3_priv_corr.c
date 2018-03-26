/******************************************************************************
 * Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 ******************************************************************************/

#include "bstd.h"
#include "bwfe.h"
#include "bwfe_priv.h"
#include "bwfe_g3_priv.h"

BDBG_MODULE(bwfe_g3_priv_corr);


#define BWFE_DEBUG_CORR(x) /*x*/


/******************************************************************************
 BWFE_g3_Corr_P_CorrDone_isr() - ISR context
******************************************************************************/
void BWFE_g3_Corr_P_CorrDone_isr(void *p, int param)
{
   BWFE_ChannelHandle h = (BWFE_ChannelHandle)p;
   BWFE_g3_P_ChannelHandle *hChn = (BWFE_g3_P_ChannelHandle *)h->pImpl;
   BWFE_FUNCT funct;

   BSTD_UNUSED(param);
   BWFE_g3_P_DisableTimer_isr(h, BWFE_g3_TimerSelect_e0);

   /* stop correlator, disable and clear interrupt */
   BWFE_P_AndRegister(h, BCHP_WFE_CORE_CORRCTL, ~0x00000002);
   BINT_DisableCallback_isr(hChn->hCorrDoneCb);
   BINT_ClearCallback_isr(hChn->hCorrDoneCb);
   
   funct = hChn->corrIdleIsr;
   if (funct)
      funct(h);
}


#if !defined(BWFE_EXCLUDE_SPECTRUM_ANALYZER) || !defined(BWFE_EXCLUDE_ANALOG_DELAY)
/******************************************************************************
 BWFE_g3_Corr_P_CalcCorrParams()
******************************************************************************/
/*#define BWFE_DPM_CORR_LEN  134217728*/   /* rough correlator length = 2^27 */
#define BWFE_DPM_CORR_LEN  16777216   /* rough correlator length = 2^24 */
BERR_Code BWFE_g3_Corr_P_SetCorrParams(BWFE_ChannelHandle h, uint32_t freqHz, uint32_t binSizeHz)
{
   BWFE_g3_P_ChannelHandle *hChn = (BWFE_g3_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode = BERR_SUCCESS;
   uint32_t P_hi, P_lo, Q_hi, Q_lo;
   uint32_t fcw, thr, Q;
   uint8_t maxbitpos = 0;

   if (binSizeHz > 0)
   {
      /* calculate FCW = (2^32)*Fc/Fs_adc for SA mode */
      BMTH_HILO_32TO64_Mul(freqHz, 2147483648UL, &P_hi, &P_lo);  /* Fc*(2^31) */
      BMTH_HILO_64TO64_Div32(P_hi, P_lo, (hChn->adcSampleFreqKhz / BWFE_LIC_L) * 500, &Q_hi, &Q_lo);  /* div by (Fs_adc/LIC_L) / 2 */
      fcw = Q_lo;

      /* zero threshold for SA mode */
      thr = 0;

      /* set correlator duration, correlator clock is Fadc/LIC_L */
      BMTH_HILO_32TO64_Mul(hChn->adcSampleFreqKhz, 1000, &P_hi, &P_lo);
      BMTH_HILO_64TO64_Div32(P_hi, P_lo, BWFE_LIC_L * binSizeHz, &Q_hi, &Q_lo);   /* corr_len = (Fadc/LIC_L) / Fb */
   }
   else
   {
      BWFE_DEBUG_CORR(BDBG_MSG(("QDDFS_M=%d", hChn->dpmQddfsM)));
      BWFE_DEBUG_CORR(BDBG_MSG(("QDDFS_N=%d", hChn->dpmQddfsN)));

      BMTH_HILO_32TO64_Mul(2147483648UL, 2, &P_hi, &P_lo);    /* 2^intbw where intbw=32 matches rtl word length */
      BMTH_HILO_64TO64_Div32(P_hi, P_lo, hChn->dpmQddfsN, &Q_hi, &Q_lo);  /* QDDFS_N is positive */
      Q = Q_lo;      /* Q = floor(2^intbw / QDDFS_N) where intbw=32 */

      BMTH_HILO_32TO64_Mul(hChn->dpmQddfsM, Q, &P_hi, &P_lo);
      fcw = P_lo;    /* fcw = QDDFS_M * Q */

      BMTH_HILO_32TO64_Mul(hChn->dpmQddfsN, Q, &P_hi, &P_lo);
      thr = P_lo;    /* thr = QDDFS_N * Q mod 2^intbw where intbw=32 */

      /* corr_len = floor(2^24/N_Q) * N_Q */
      BMTH_HILO_32TO64_Mul(BWFE_DPM_CORR_LEN / hChn->dpmQddfsN, hChn->dpmQddfsN, &Q_hi, &Q_lo);

   #if (BCHP_CHIP==45308)
      /* corr_len = floor(corr_ctl_len=4095/256)) * (floor(2^24/N_Q) * N_Q) */
      BMTH_HILO_64TO64_Mul(Q_hi, Q_lo, 0, 15, &Q_hi, &Q_lo);
   #endif
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
 BWFE_g3_Corr_P_StartCorrelator() - ISR context
******************************************************************************/
BERR_Code BWFE_g3_Corr_P_StartCorrelator(BWFE_ChannelHandle h, uint32_t freqHz, uint32_t binSizeHz, BWFE_FUNCT func)
{
   BWFE_g3_P_ChannelHandle *hChn = (BWFE_g3_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode = BERR_SUCCESS;
   
   hChn->corrFreqHz = freqHz;

   /* enable corr clock */
   BWFE_P_OrRegister(h, BCHP_WFE_ANA_CLK_CTRL, 0x00000008);

   /* stop correlator */
   BWFE_P_WriteRegister(h, BCHP_WFE_CORE_CORRCTL, 0x00000000);
   BWFE_P_ToggleBit(h, BCHP_WFE_CORE_CORRCTL, 0x00000001);  /* correlator clear */
   
   /* configure required fcw, thresh, and length depending on mode */
   BWFE_g3_Corr_P_SetCorrParams(h, freqHz, binSizeHz);

   /* clear interrupt and enable */
   BINT_ClearCallback_isr(hChn->hCorrDoneCb);
   BINT_EnableCallback_isr(hChn->hCorrDoneCb);
   hChn->corrIdleIsr = func;

   /* corr done timeout for spec analyzer only */
   if (binSizeHz > 0)
   {
      /* workaround for missing corr done irq at certain corrlen's - execute callback in 50 us */
      retCode = BWFE_g3_P_EnableTimer_isr(h, BWFE_g3_TimerSelect_e0, 50, (BWFE_FUNCT)BWFE_g3_Corr_P_CorrDone_isr);
   }

   /* start correlator */
   BWFE_P_OrRegister(h, BCHP_WFE_CORE_CORRCTL, 0x00000002);
   return retCode;
}


/******************************************************************************
 BWFE_g3_Lic_P_ReadCorrelator()
******************************************************************************/
BERR_Code BWFE_g3_Corr_P_ReadCorrelator(BWFE_ChannelHandle h, uint8_t slice, uint32_t regMsb, uint32_t regLsb, int32_t *corr)
{
   BWFE_g3_P_ChannelHandle *hChn = (BWFE_g3_P_ChannelHandle *)h->pImpl;
   uint32_t corrMsb, corrLsb;
   
   BWFE_P_ReadSliceRegister(h, slice, regLsb, &corrLsb);    /* lsb32 */
   BWFE_P_ReadSliceRegister(h, slice, regMsb, &corrMsb);    /* msb28 */
   
   /* take 32 MSB based on 2^k accumulation limit */
	if (hChn->corrMaxBit >= 32)
      *corr = (int32_t)((corrMsb << (63 - hChn->corrMaxBit)) | (corrLsb >> (hChn->corrMaxBit - 31)));
	else
      *corr = (int32_t)(corrLsb << (31 - hChn->corrMaxBit));
   
   /* BWFE_DEBUG_CORR(BDBG_MSG(("corr:MSB=%08X|LSB=%08X-(%d)->%08X", corrMsb, corrLsb, hChn->corrMaxBit, *corr))); */
   return BERR_SUCCESS;
}


/******************************************************************************
 BWFE_g3_Corr_P_GetCorrVals()
******************************************************************************/
BERR_Code BWFE_g3_Corr_P_GetCorrVals(BWFE_ChannelHandle h, int32_t *corrI, int32_t *corrQ)
{
   uint32_t slice;
   
   for (slice = 0; slice < BWFE_NUM_SLICES; slice++)
   {
      /* read demux0 correlators */
      BWFE_g3_Corr_P_ReadCorrelator(h, slice, BCHP_WFE_CORE_CORRI1_DMX0_PI_SLC, BCHP_WFE_CORE_CORRI0_DMX0_PI_SLC, &corrI[slice]);
      BWFE_g3_Corr_P_ReadCorrelator(h, slice, BCHP_WFE_CORE_CORRQ1_DMX0_PI_SLC, BCHP_WFE_CORE_CORRQ0_DMX0_PI_SLC, &corrQ[slice]);
      BWFE_g3_Corr_P_ReadCorrelator(h, slice, BCHP_WFE_CORE_CORRI1_DMX0_PO_SLC, BCHP_WFE_CORE_CORRI0_DMX0_PO_SLC, &corrI[slice + BWFE_NUM_SLICES]);
      BWFE_g3_Corr_P_ReadCorrelator(h, slice, BCHP_WFE_CORE_CORRQ1_DMX0_PO_SLC, BCHP_WFE_CORE_CORRQ0_DMX0_PO_SLC, &corrQ[slice + BWFE_NUM_SLICES]);
      
      /* read demux1 correlators */
      BWFE_g3_Corr_P_ReadCorrelator(h, slice, BCHP_WFE_CORE_CORRI1_DMX1_PI_SLC, BCHP_WFE_CORE_CORRI0_DMX1_PI_SLC, &corrI[slice + BWFE_NUM_SLICES * 2]);
      BWFE_g3_Corr_P_ReadCorrelator(h, slice, BCHP_WFE_CORE_CORRQ1_DMX1_PI_SLC, BCHP_WFE_CORE_CORRQ0_DMX1_PI_SLC, &corrQ[slice + BWFE_NUM_SLICES * 2]);
      BWFE_g3_Corr_P_ReadCorrelator(h, slice, BCHP_WFE_CORE_CORRI1_DMX1_PO_SLC, BCHP_WFE_CORE_CORRI0_DMX1_PO_SLC, &corrI[slice + BWFE_NUM_SLICES * 3]);
      BWFE_g3_Corr_P_ReadCorrelator(h, slice, BCHP_WFE_CORE_CORRQ1_DMX1_PO_SLC, BCHP_WFE_CORE_CORRQ0_DMX1_PO_SLC, &corrQ[slice + BWFE_NUM_SLICES * 3]);
      
      /* read demux2 correlators */
      BWFE_g3_Corr_P_ReadCorrelator(h, slice, BCHP_WFE_CORE_CORRI1_DMX2_PI_SLC, BCHP_WFE_CORE_CORRI0_DMX2_PI_SLC, &corrI[slice + BWFE_NUM_SLICES * 4]);
      BWFE_g3_Corr_P_ReadCorrelator(h, slice, BCHP_WFE_CORE_CORRQ1_DMX2_PI_SLC, BCHP_WFE_CORE_CORRQ0_DMX2_PI_SLC, &corrQ[slice + BWFE_NUM_SLICES * 4]);
      BWFE_g3_Corr_P_ReadCorrelator(h, slice, BCHP_WFE_CORE_CORRI1_DMX2_PO_SLC, BCHP_WFE_CORE_CORRI0_DMX2_PO_SLC, &corrI[slice + BWFE_NUM_SLICES * 5]);
      BWFE_g3_Corr_P_ReadCorrelator(h, slice, BCHP_WFE_CORE_CORRQ1_DMX2_PO_SLC, BCHP_WFE_CORE_CORRQ0_DMX2_PO_SLC, &corrQ[slice + BWFE_NUM_SLICES * 5]);
      
      /* read demux3 correlators */
      BWFE_g3_Corr_P_ReadCorrelator(h, slice, BCHP_WFE_CORE_CORRI1_DMX3_PI_SLC, BCHP_WFE_CORE_CORRI0_DMX3_PI_SLC, &corrI[slice + BWFE_NUM_SLICES * 6]);
      BWFE_g3_Corr_P_ReadCorrelator(h, slice, BCHP_WFE_CORE_CORRQ1_DMX3_PI_SLC, BCHP_WFE_CORE_CORRQ0_DMX3_PI_SLC, &corrQ[slice + BWFE_NUM_SLICES * 6]);
      BWFE_g3_Corr_P_ReadCorrelator(h, slice, BCHP_WFE_CORE_CORRI1_DMX3_PO_SLC, BCHP_WFE_CORE_CORRI0_DMX3_PO_SLC, &corrI[slice + BWFE_NUM_SLICES * 7]);
      BWFE_g3_Corr_P_ReadCorrelator(h, slice, BCHP_WFE_CORE_CORRQ1_DMX3_PO_SLC, BCHP_WFE_CORE_CORRQ0_DMX3_PO_SLC, &corrQ[slice + BWFE_NUM_SLICES * 7]);
   }
   
   return BERR_SUCCESS;
}
#endif


#ifndef BWFE_EXCLUDE_ANALOG_DELAY
/******************************************************************************
 BWFE_g3_Corr_P_CalcDelay() - computes delay & gain imbalance per lane from I,Q correlators
 
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
BERR_Code BWFE_g3_Corr_P_CalcDelay(BWFE_ChannelHandle h)
{
   BERR_Code retCode = BERR_SUCCESS;
   BWFE_g3_P_ChannelHandle *hChn = (BWFE_g3_P_ChannelHandle *)h->pImpl;
   uint32_t P_hi, P_lo, Q_hi, Q_lo;
   int32_t corrI[BWFE_LIC_L], corrQ[BWFE_LIC_L], gain[BWFE_LIC_L], angle[BWFE_LIC_L];
   int32_t phase, vec[2];
   uint8_t k;
   int8_t tmp;
   bool bNegative = false;
   
   /* read all correlator values: 4 slices x 2 dmx x 2 lanes = LIC_L = 16 */
   BWFE_g3_Corr_P_GetCorrVals(h, (int32_t *)corrI, (int32_t *)corrQ);
   hChn->bCorrInProgress = false;   /* finished with correlator */
   
   /* disable DPM tone since correlator done */
   if ((hChn->corrDpmDebug & 0x6) == 0)
      BWFE_P_DisableDpmPilot(h);
   
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
         BMTH_HILO_32TO64_Mul((hChn->dpmPilotFreqKhz << 1) * BWFE_ABS(tmp), 2147483648UL, &P_hi, &P_lo); /* Fc scaled up by 2^31 * k * 2 */
         BMTH_HILO_64TO64_Div32(P_hi, P_lo, hChn->adcSampleFreqKhz, &Q_hi, &Q_lo);           /* div by Fs_adc */
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
   
   for (k = 0; k < BWFE_LIC_L; k++)
   {
      if (k != hChn->corrRefSlice)
      {
         /* calculate agc[k] = gain[hChn->corrRefSlice] / gain[k] */
      #if 0
         BMTH_HILO_32TO64_Mul(BWFE_AGC_SCALE, gain[0], &P_hi, &P_lo);  /* gain[0] scaled by agc_scale */
         BMTH_HILO_64TO64_Div32(P_hi, P_lo, gain[k], &Q_hi, &Q_lo);    /* div by gain[k] */
         h->corrAgc[k] = Q_lo;
      #else
         BSTD_UNUSED(gain);
      #endif

         /* remove sign from angle since extended precision is unsigned */
         if (angle[k] < 0)
         {
            bNegative = true;
            angle[k] = -angle[k];
         }
         else
            bNegative = false;

         /* calculate delay[k] = angle[k] * Fs_adc / (2 * Fc) */ /* TBD optional scaling for angle[k] */
         BMTH_HILO_32TO64_Mul((uint32_t)angle[k], hChn->adcSampleFreqKhz, &P_hi, &P_lo);  /* angle[k] * Fs_adc */
         BMTH_HILO_64TO64_Div32(P_hi, P_lo, hChn->dpmPilotFreqKhz << 1, &Q_hi, &Q_lo);    /* div by 2Fc */
         hChn->corrDelay[k] = (int32_t)Q_lo;

         /* recover sign */
         if (bNegative)
            hChn->corrDelay[k] = -(hChn->corrDelay[k]);

         /*BDBG_MSG(("k%d: DLY=0x%08X (%d)\n", k, hChn->corrDelay[k], hChn->corrDelay[k]));*/
      }
   }

   /* TBD estimate delay across slices: [(D1 + D3 + ... + D_odd) - (D0 + D2 + ... + D_even)] / 8 */
   /*hChn->sliceDelay[0] = ((hChn->corrDelay[1] + hChn->corrDelay[3]) - (hChn->corrDelay[0] + hChn->corrDelay[2])) >> 1; */
   hChn->sliceDelay[0] = 0;
   for (k = 0; k < BWFE_LIC_L; k++)
   {
      if (k & 1)
         hChn->sliceDelay[0] += hChn->corrDelay[k];
      else
         hChn->sliceDelay[0] -= hChn->corrDelay[k];
   }
   hChn->sliceDelay[0] >>= 3;
   BWFE_DEBUG_CORR(BKNI_Printf("slc delay=%d\n", hChn->sliceDelay[0]));

   /* continue with next function */
   if (hChn->postCalcDelayFunct != NULL)
      retCode = hChn->postCalcDelayFunct(h);
   
   return retCode;
}


/******************************************************************************
 BWFE_g3_Corr_P_CoarseAdjust() - ISR context
******************************************************************************/
BERR_Code BWFE_g3_Corr_P_CoarseAdjust(BWFE_ChannelHandle h)
{
   BERR_Code retCode = BERR_SUCCESS;
   BWFE_g3_P_ChannelHandle *hChn = (BWFE_g3_P_ChannelHandle *)h->pImpl;
   uint32_t P_hi, P_lo, Q_hi, Q_lo;
   uint32_t absDelay;

   BWFE_DEBUG_CORR(BKNI_Printf("BWFE_g3_Corr_P_CoarseAdjust: measDelay=%d\n", hChn->sliceDelay[0]));

   hChn->adjRight = 7;
   hChn->adjLeft = 7;

   /* meas_delay = (delay / 2^32 * pi) / Fs_adc * 1e12(ps) */
   /* rt = rt +/- abs(int(meas_delay/0.2))+1 */
   /* lt = lt +/- abs(int(meas_delay/0.2))-1 */

   /* remove sign */
   if (hChn->sliceDelay[0] < 0)
      absDelay = -hChn->sliceDelay[0];
   else
      absDelay = hChn->sliceDelay[0];

   BMTH_HILO_32TO64_Mul(absDelay, 314159265 * 5, &P_hi, &P_lo);   /* pi * 1e8 / 0.2 */
#if 0
   BMTH_HILO_64TO64_Div32(P_hi, P_lo, 502232, &Q_hi, &Q_lo);      /* div by Fs_adc/10000 */
#else
   BMTH_HILO_64TO64_Div32(P_hi, P_lo, hChn->adcSampleFreqKhz / 10, &Q_hi, &Q_lo);      /* div by Fs_adc/10000 */
#endif
   BMTH_HILO_64TO64_Add(Q_hi, Q_lo, 0, 2147483647, &Q_hi, &Q_lo); /* ceil(result) */
   BWFE_DEBUG_CORR(BKNI_Printf("absDelay=%d -> Q=%08X %08X\n", absDelay, Q_hi, Q_lo));

   if (hChn->sliceDelay[0] < 0)
   {
      hChn->adjRight = hChn->adjRight - Q_hi + 1;
      hChn->adjLeft = hChn->adjLeft + Q_hi - 1;
   }
   else
   {
      hChn->adjRight = hChn->adjRight + Q_hi;
      hChn->adjLeft = hChn->adjLeft - Q_hi;
   }

   if ((hChn->adjRight > 15) || (hChn->adjLeft > 15))
   {
      BDBG_ERR(("delay adjust overflow!"));
      return BERR_UNKNOWN;
   }

   BWFE_DEBUG_CORR(BKNI_Printf("rt=%d/lt=%d\n", hChn->adjRight, hChn->adjLeft));
   BWFE_g3_Corr_P_CompensateDelay(h, hChn->adjRight, hChn->adjLeft);

   /* re-measure delay and do fine adjustments */
   BWFE_P_EnableDpmPilot(h);  /* enable DPM tone, disable DPM after delay calculations */
   hChn->postCalcDelayFunct = BWFE_g3_Corr_P_FineAdjust;    /* fine adjustment after delay measurement */
   BWFE_g3_Corr_P_StartCorrelator(h, hChn->dpmPilotFreqKhz * 1000, 0, BWFE_g3_Corr_P_CalcDelay);

   return retCode;
}


/******************************************************************************
 BWFE_g3_Corr_P_FineAdjust() - ISR context
******************************************************************************/
BERR_Code BWFE_g3_Corr_P_FineAdjust(BWFE_ChannelHandle h)
{
   BWFE_g3_P_ChannelHandle *hChn = (BWFE_g3_P_ChannelHandle *)h->pImpl;

   /* fine correction linear search */
   hChn->calState = 0;
   hChn->calCount = 1;
   hChn->initDelay = hChn->sliceDelay[0];
   return BWFE_g3_Corr_P_FineAdjust1(h);
}


/******************************************************************************
 BWFE_g3_Corr_P_FineAdjust1() - ISR context
******************************************************************************/
BERR_Code BWFE_g3_Corr_P_FineAdjust1(BWFE_ChannelHandle h)
{
   BERR_Code retCode = BERR_SUCCESS;
   BWFE_g3_P_ChannelHandle *hChn = (BWFE_g3_P_ChannelHandle *)h->pImpl;

   BWFE_DEBUG_CORR(BKNI_Printf("BWFE_g3_Corr_P_FineAdjust: measDelay=%d\n", hChn->sliceDelay[0]));

   while (1)
   {
      /*BKNI_Printf("BWFE_g3_Corr_P_FineAdjust(): state=%d\n", hChn->calState);*/
      switch (hChn->calState)
      {
         case 0:
            if (hChn->initDelay > 0)
            {
               BWFE_DEBUG_CORR(BKNI_Printf("R loop...\n"));
               if (hChn->calCount & 1)
                  hChn->adjRight++; /* right */
               else
                  hChn->adjLeft--;  /* left */
            }
            else
            {
               BWFE_DEBUG_CORR(BKNI_Printf("L loop...\n"));
               if (hChn->calCount & 1)
                  hChn->adjRight--; /* right */
               else
                  hChn->adjLeft++;  /* left */
            }

            BWFE_DEBUG_CORR(BKNI_Printf("rt=%d/lt=%d\n", hChn->adjRight, hChn->adjLeft));
            BWFE_g3_Corr_P_CompensateDelay(h, hChn->adjRight, hChn->adjLeft);
            hChn->calState = 1;
            break;

         case 1:
            /* re-measure delay */
            BWFE_P_EnableDpmPilot(h);  /* enable DPM tone, disable DPM after delay calculations */
            hChn->calState = 2;
            hChn->prevDelay = hChn->sliceDelay[0];
            hChn->postCalcDelayFunct = BWFE_g3_Corr_P_FineAdjust1;    /* continue linear search after delay measurement */
            return BWFE_g3_Corr_P_StartCorrelator(h, hChn->dpmPilotFreqKhz * 1000, 0, BWFE_g3_Corr_P_CalcDelay);

         case 2:
            BWFE_DEBUG_CORR(BKNI_Printf("%d: measDelay=%d | prevDelay=%d\n", hChn->calCount, hChn->sliceDelay[0], hChn->prevDelay));

            if ((hChn->prevDelay > 0) && (hChn->sliceDelay[0] < 0))
            {
               if (BWFE_ABS(hChn->sliceDelay[0]) < BWFE_ABS(hChn->prevDelay))
               {
                  /* exit if measDelay switched signs and is less than prevDelay */
                  BWFE_DEBUG_CORR(BKNI_Printf("exit R loop\n"));
               }
               else
               {
                  /* revert linear adjustments */
                  BWFE_DEBUG_CORR(BKNI_Printf("revert to prev R\n"));
                  hChn->sliceDelay[0] = hChn->prevDelay;
                  if (hChn->calCount & 1)
                     hChn->adjRight--; /* right */
                  else
                     hChn->adjLeft++;  /* left */
                  BWFE_g3_Corr_P_CompensateDelay(h, hChn->adjRight, hChn->adjLeft);
               }
               hChn->calState = 3;
            }
            else if ((hChn->prevDelay < 0) && (hChn->sliceDelay[0] > 0))
            {
               if (BWFE_ABS(hChn->sliceDelay[0]) < BWFE_ABS(hChn->prevDelay))
               {
                  /* exit if measDelay switched signs and is less than prevDelay */
                  BWFE_DEBUG_CORR(BKNI_Printf("exit L loop\n"));
               }
               else
               {
                  /* revert linear adjustments */
                  BWFE_DEBUG_CORR(BKNI_Printf("revert to prev L\n"));
                  hChn->sliceDelay[0] = hChn->prevDelay;
                  if (hChn->calCount & 1)
                     hChn->adjRight++; /* right */
                  else
                     hChn->adjLeft--;  /* left */
                  BWFE_g3_Corr_P_CompensateDelay(h, hChn->adjRight, hChn->adjLeft);
               }
               hChn->calState = 3;
            }
            else
            {
               hChn->calCount++;
               if (hChn->calCount < 16)
                  hChn->calState = 0;
               else
                  hChn->calState = 3;
            }
            break;

         case 3:
            /* finished linear search */
            BWFE_DEBUG_CORR(BKNI_Printf("rt=%d/lt=%d\n", hChn->adjRight, hChn->adjLeft));
            BWFE_DEBUG_CORR(BKNI_Printf("FINAL measDelay=%d\n", hChn->sliceDelay[0]));
            BKNI_SetEvent(hChn->hDelayCalDone);
            return retCode;

         default:
            BDBG_ERR(("invalid state"));
            BERR_TRACE(retCode = BERR_UNKNOWN);
            break;
      }
   }

   return retCode;
}


/******************************************************************************
 BWFE_g3_Corr_P_CompensateDelay()
******************************************************************************/
BERR_Code BWFE_g3_Corr_P_CompensateDelay(BWFE_ChannelHandle h, uint32_t adjRight, uint32_t adjLeft)
{
   BERR_Code retCode = BERR_SUCCESS;
   uint32_t val;

   /* cap adjustments */
   if (adjRight > 15)
      adjRight = 15;
   if (adjLeft > 15)
      adjLeft = 15;

   /* override coarse-fine timing */
   val = (adjLeft << 12) | (adjLeft << 8) | (adjLeft << 4) | adjLeft;   /* S0 left adjust */
   val |= (adjRight << 28) | (adjRight << 24) | (adjRight << 20) | (adjRight << 16);   /* S1 right adjust */
   BWFE_P_WriteRegister(h, BCHP_WFE_ANA_ADC_CNTL10, val);

   return retCode;
}
#endif


#ifndef BWFE_EXCLUDE_SPECTRUM_ANALYZER
/******************************************************************************
 BWFE_g3_Corr_P_ScanSpectrum() - task context
******************************************************************************/
BERR_Code BWFE_g3_Corr_P_ScanSpectrum(BWFE_ChannelHandle h)
{
   BERR_Code retCode = BERR_SUCCESS;
   BWFE_g3_P_Handle *hDev = h->pDevice->pImpl;
   BWFE_g3_P_ChannelHandle *hChn = (BWFE_g3_P_ChannelHandle *)h->pImpl;
   
#if 0
   /* TBD debug */
   h->saParams.numSamples = 1;
   h->saParams.freqStartHz = 1076250000;
   h->saParams.freqStopHz = 1353750000;
#endif
   
   /* calculate freq step size */
   if (hChn->saParams.numSamples > 0)
      hDev->saStep = (hChn->saParams.freqStopHz - hChn->saParams.freqStartHz) / hChn->saParams.numSamples;
   
   BWFE_DEBUG_CORR(BDBG_MSG(("freq[%u,%u]\n", hChn->saParams.freqStartHz, hChn->saParams.freqStopHz)));
   BWFE_DEBUG_CORR(BDBG_MSG(("step: %d | #samples: %d | #sweeps: %d\n", hDev->saStep, hChn->saParams.numSamples, hChn->saParams.numSweeps)));

   /* enable corr clock */
   BWFE_P_OrRegister(h, BCHP_WFE_ANA_CLK_CTRL, 0x00000008);

   /* select input to correlator */
   BWFE_P_WriteRegister(h, BCHP_WFE_CORE_CORRINSEL, hChn->corrInputSelect & 0x3);
   
   /* clear sa samples */
   BKNI_Memset((void*)hDev->saSamples, 0, BWFE_SA_MAX_SAMPLES);
   
   hDev->saSweep = hChn->saParams.numSweeps;
   hDev->saState = 0;
   
   /* enter spectrum scan in isr context */
   BKNI_EnterCriticalSection();
   retCode = BWFE_g3_P_EnableTimer_isr(h, BWFE_g3_TimerSelect_e0, 5, BWFE_g3_Corr_P_ScanSpectrum1);
   BKNI_LeaveCriticalSection();
   
   return retCode;
}


/******************************************************************************
 BWFE_g3_Corr_P_ScanSpectrum1() - ISR context
******************************************************************************/
BERR_Code BWFE_g3_Corr_P_ScanSpectrum1(BWFE_ChannelHandle h)
{
   BERR_Code retCode = BERR_SUCCESS;
   BWFE_g3_P_Handle *hDev = h->pDevice->pImpl;
   BWFE_g3_P_ChannelHandle *hChn = (BWFE_g3_P_ChannelHandle *)h->pImpl;
   uint32_t corrPowerDb, prevPower, i;
   int32_t corrI[BWFE_LIC_L], corrQ[BWFE_LIC_L];
   
   while (1)
   {
      /*BDBG_MSG(("BWFE_Corr_P_ScanSpectrum(): state=%d, sweep=%d, idx=%d, freq=%u", hDev->saState, hDev->saSweep, hDev->saIdx, hDev->saFreq));*/
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
                  return BWFE_g3_P_EnableTimer_isr(h, BWFE_g3_TimerSelect_e0, 5000, BWFE_g3_Corr_P_ScanSpectrum1);
               }
               
               hDev->saState = 2;
               hChn->bCorrInProgress = true;
               return BWFE_g3_Corr_P_StartCorrelator(h, hDev->saFreq, hDev->saStep, BWFE_g3_Corr_P_ScanSpectrum1);
            }
            
            /* decrement sweeps remaining */
            if (hDev->saSweep > 0)
               hDev->saSweep--;
            hDev->saState = 0;
            break;
            
         case 2:
            /* read all correlator values: 4 slices x 2 dmx x 2 lanes = LIC_L = 16 */
            BWFE_g3_Corr_P_GetCorrVals(h, (int32_t *)corrI, (int32_t *)corrQ);
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
               /*BDBG_MSG(("%d: %d*%08X|%08X->%08X", hDev->saIdx, i, prevPower, corrPowerDb, hDev->saSamples[hDev->saIdx]));*/
            }
            else
            {
               /* case if one sweep only or initial sweep */
               hDev->saSamples[hDev->saIdx] = corrPowerDb;
               /*BDBG_MSG(("* %d: %08X", hDev->saIdx, hDev->saSamples[hDev->saIdx]));*/
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

