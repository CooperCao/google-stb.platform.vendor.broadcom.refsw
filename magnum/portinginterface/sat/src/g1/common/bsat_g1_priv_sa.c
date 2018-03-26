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
*
* Module Description:
*
*****************************************************************************/
#include "bstd.h"
#include "bmth.h"
#include "bsat.h"
#include "bsat_priv.h"
#include "bsat_g1_priv.h"


BDBG_MODULE(bsat_g1_priv_sa);


#define BSAT_DEBUG_SA 0


#if BSAT_DEBUG_SA
#ifdef BSAT_HAS_LEAP
#include "bchp_aif_wb_sat_core0.h"
#include "bos.h"
#endif
#endif


/* ACI for Spectrum Analyzer RBW=3MHz, fs = 4968.000000  */
static const int16_t BSAT_AciCoeff_3MHz_RBW[36] = {
   -20,
   -18,
   -26,
   -35,
   -44,
   -54,
   -64,
   -73,
   -81,
   -85,
   -86,
   -83,
   -73,
   -55,
   -30,
   5,
   50,
   106,
   172,
   249,
   336,
   432,
   536,
   647,
   763,
   880,
   998,
   1112,
   1221,
   1322,
   1413,
   1490,
   1553,
   1599,
   1627,
   1636
};


/* ACI for Spectrum Analyzer RBW=10MHz, fs = 4968.000000  */
static const int16_t BSAT_AciCoeff_10MHz_RBW[36] = {
   20,
   26,
   36,
   43,
   42,
   29,
   4,
   -33,
   -76,
   -116,
   -142,
   -144,
   -113,
   -47,
   49,
   160,
   264,
   336,
   350,
   288,
   143,
   -73,
   -332,
   -585,
   -777,
   -847,
   -744,
   -435,
   84,
   788,
   1620,
   2495,
   3319,
   3992,
   4434,
   4588
};

#if 0
/* ACI for Spectrum Analyzer RBW=30MHz, fs = 4968.000000  */
static const int16_t BSAT_AciCoeff_30MHz_RBW[36] = {
   -29,
   -35,
   -3,
   47,
   50,
   -17,
   -72,
   -23,
   89,
   106,
   -31,
   -156,
   -69,
   158,
   209,
   -46,
   -296,
   -151,
   269,
   389,
   -59,
   -530,
   -303,
   459,
   721,
   -69,
   -988,
   -630,
   871,
   1538,
   -76,
   -2440,
   -1950,
   3111,
   9881,
   13029
};
#endif

/******************************************************************************
 BSAT_g1_P_SaSetMixer_isr()
******************************************************************************/
static void BSAT_g1_P_SaSetMixer_isr(BSAT_ChannelHandle h)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   uint64_t Fs_ADC = (uint64_t)((uint64_t)BSAT_ADC_FREQ_KHZ * 1000);

   uint32_t val = (uint32_t)(((uint64_t)(1<<28) * (uint64_t)(Fs_ADC - (uint64_t)(hChn->acqSettings.freq))) / Fs_ADC);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_STB_CHAN_CHx_DEC_FCW, val);
}


/******************************************************************************
 BSAT_g1_P_log2Approx_isr() - return log2, which is approximately log2(xint) * 16
******************************************************************************/
static uint32_t BSAT_g1_P_log2Approx_isr(int xint)
{
   int msb = 0;
   int log2Result;

   int OutputShift = 4;    /* Log2 output has 4 LSB after the point */

   if (xint <= 0)
      return 0;
   else
   {
      while (xint >> msb) msb++;
      msb--;

      if(msb-OutputShift > 0)
         log2Result = (xint>>(msb-OutputShift))+((msb-1)<<OutputShift);
      else
         log2Result = (xint<<(OutputShift-msb))+((msb-1)<<OutputShift);

      return (log2Result);
   }
}


/******************************************************************************
 BSAT_g1_P_SaStateMachine_isr()
******************************************************************************/
BERR_Code BSAT_g1_P_SaStateMachine_isr(BSAT_ChannelHandle h)
{
#if (BSAT_DEBUG_SA && defined(BSAT_HAS_LEAP))
   static uint32_t t0;
#endif
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   BSAT_g1_P_Handle *hDev = (BSAT_g1_P_Handle *)h->pDevice->pImpl;
   uint64_t val64, val64_var2;
   uint32_t *pSaBuf = hDev->pSaBuf;
   uint8_t *pSaBuf8 = (uint8_t*)((void*)pSaBuf);
   bool bDone = false;
   uint32_t val, val2, chan_agc, displayDecimRate, j;
   int32_t sval, k, i;
   uint8_t val8;

   while (!bDone)
   {
      switch (hChn->functState)
      {
         case 0:
#if (BSAT_DEBUG_SA && defined(BSAT_HAS_LEAP))
            t0 = bos_get_microticks();
#endif
            BSAT_g1_P_ToggleBit_isrsafe(h, BCHP_SDS_CG_RSTCTL, 1); /* SDS rcvr datapath reset */
            BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_STB_CHAN_CHx_AGC_CTRL, 0x02019998);
            BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_FE_MIXCTL, 0x00000011);
            BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_FE_AGFCTL, 0);
            BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_FE_AGF, 0x0F100000); /* orig:0x0F200000 */

            BSAT_g1_P_FreezeChanAgc_isr(h, false); /* enable SDS_CHAN AGC */

            hChn->acqSettings.freq = hChn->saSettings.startFreq; /* sweepFreq */
            hChn->count1 = 0; /* count1 = index in saBuf */

            if (hChn->saSettings.resBw <= BSAT_ResBw_e300khz)
            {
               /* RBW is 100KHz or 300KHz */
               /* set ACI filter to 1 MBaud */
               BSAT_g1_P_ChanSetAciCoeff_isr(h);

               /* bypass the sds mixer so that the carrier loop does not affect the psd function */
               BSAT_g1_P_OrRegister_isrsafe(h, BCHP_SDS_FE_MIXCTL, BCHP_SDS_FE_0_MIXCTL_mixer_byp_MASK);

               /* we have to enable at least 4 SDS_FE decimation filters for the SDS_DFT to work, use quarterband to provide sharper filtering */
               BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_FE_FILTCTL, 0xD0);

               /* set up the dft block */
               BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_DFT_CTRL0, 2); /* dft soft reset */
               BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_DFT_CTRL0, 0);

               val = 0x3DE; /* dft_in_sel=1, bypass the bandedge tone part */
#ifdef BCHP_SDS_DFT_0_CTRL1_dft_avg_mode_MASK
               BCHP_SET_FIELD_DATA(val, SDS_DFT_0_CTRL1, dft_avg_mode, 0); /* average power mode */
               BCHP_SET_FIELD_DATA(val, SDS_DFT_0_CTRL1, dft_avg_num, 0); /* avgNumber=1 */
#endif
               BCHP_SET_FIELD_DATA(val, SDS_DFT_0_CTRL1, dft_size, 1); /* dft_size=512 */
               BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_DFT_CTRL1, val);

               BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_DFT_RANGE_START, 1); /* to avoid the dc bin at bin 0 */
               BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_DFT_RANGE_END, 16);
               BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_DFT_DDFS_FCW, 0);

               hChn->saStatus.freqStep = (BSAT_DEFAULT_SAMPLE_FREQ >> 13); /* sweepFreqStep = Fs/512 */
               hChn->functState = 1;
            }
            else if (hChn->saSettings.resBw == BSAT_ResBw_e1mhz)
            {
               /* RBW is 1 MHz */
               BSAT_g1_P_LoadAciFilterCoeff_isr(h, BSAT_AciCoeff_3MHz_RBW);

               /* enable 5 decimation filters, select quarterband */
               BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_FE_FILTCTL, 0xC0);

               hChn->saStatus.freqStep = 250000;
               hChn->functState = 1;
            }
            else if (hChn->saSettings.resBw == BSAT_ResBw_e3mhz)
            {
               /* RBW is 3 MHz */
               BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_FE_FILTCTL, 0x80DF); /* bypass all decimation filters, non-decimating halfband */
               BSAT_g1_P_LoadAciFilterCoeff_isr(h, BSAT_AciCoeff_3MHz_RBW);

               hChn->saStatus.freqStep = 1000000;
               hChn->functState = 1;
            }
            else
            {
               /* RBW is 10MHz */
               BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_FE_FILTCTL, 0x80DF); /* bypass all decimation filters, non-decimating halfband */
               BSAT_g1_P_LoadAciFilterCoeff_isr(h, BSAT_AciCoeff_10MHz_RBW);

               hChn->saStatus.freqStep = 2500000;
               hChn->functState = 1;
            }
            break;

         case 1:
            BSAT_g1_P_SaSetMixer_isr(h); /* set channelizer mixer */
            hChn->functState = 2;
            return BSAT_g1_P_EnableTimer_isr(h, BSAT_TimerSelect_eBaudUsec, 50, BSAT_g1_P_SaStateMachine_isr); /* wait for STB_CHAN AGC to converge */

         case 2:
            BSAT_g1_P_FreezeChanAgc_isr(h, true);
            chan_agc = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_STB_CHAN_CHx_AGC_LF_INT);
            val = (1<<27);
            if (chan_agc <= val)
            {
               val2 = 0; /* set agc gain to 2^9 or 54dB, so the (>>AGC^2) will need to shift 18 bits */
               hChn->count2 = 9 * 2;
            }
            else
            {
               /* set AGC gain to 2^K where K=1:9 */
               sval = 0;
               while ((chan_agc - val) >> sval) sval++;

               if(sval < 19)
                  sval = 0;
               else
                  sval -= 19;

               hChn->count2 = (sval * 2); /* count2 = right shift amount */
               val2 = val + (1<<(18+sval)); /* the (>>AGC^2) will need to shift 2*sval bits */
            }
            BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_STB_CHAN_CHx_AGC_LF_INT, val2);

            if (hChn->saSettings.resBw <= BSAT_ResBw_e300khz)
            {
               /* RBW=100KHz or 300KHz */
               /* start the dft */
               hChn->passFunct = BSAT_g1_P_SaStateMachine_isr;
               hChn->functState = 3;
               BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_DFT_CTRL0, 0);
               BSAT_g1_P_DftEnableInterrupt_isr(h);
               BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_DFT_CTRL0, 1);
               return BERR_SUCCESS;
            }
            else /* RBW >= 1MHz */
            {
               hChn->functState = 5;
               return BSAT_g1_P_EnableTimer_isr(h, BSAT_TimerSelect_eBaudUsec, 50, BSAT_g1_P_SaStateMachine_isr); /* wait for SDS_FE AGC to converge */
            }
            break;

         case 3:
            /* dft done */
            BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_DFT_MEM_RADDR, 0);
            val64 = 0;
            for (val = 0; val < 16; val++)
            {
               val2 = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_SDS_DFT_MEM_RDATA);

               if (hChn->saSettings.resBw == BSAT_ResBw_e300khz)
               {
                  val64 += val2;
                  if((val & 3) == 3) /* 3, 7, 11, 15 */
                  {
                     val2 = (uint32_t)(val64 >> 2);
                     val64 = 0;
                  }
                  else
                  {
                     continue; /* Update the pSaBuf with every 4th DFT data for RBW=300KHz */
                  }
               }

               if (hChn->count2 >= 10)
                  val2 = val2 >> (hChn->count2 - 10);
               else
                  val2 = val2 << (10 - hChn->count2);

               pSaBuf[hChn->count1++] = val2;
            }

            BSAT_g1_P_FreezeChanAgc_isr(h, false);

            if (hChn->count1 > (4096-16))
               hChn->functState = 100; /* out of memory */
            else
            {
               hChn->acqSettings.freq += (16 * hChn->saStatus.freqStep);
               if (hChn->acqSettings.freq > hChn->saSettings.stopFreq)
                  hChn->functState = 4;
               else
                  hChn->functState = 1;
            }
            break;

         case 4:
            if (hChn->saSettings.resBw <= BSAT_ResBw_e300khz)
            {
               if (hChn->saSettings.resBw == BSAT_ResBw_e100khz)
                  val2 = 5; /* moving average for RBW=100KHz */
               else
                  val2 = 4; /* moving average for RBW=300KHz */

               /* moving sum */
               val64  = (uint64_t)pSaBuf[hChn->count1-1];
               for (j = 2; j <= val2; j++)
                  val64 += (uint64_t)pSaBuf[hChn->count1-j];

               /* Filter From right to the left */
               for (i = (int32_t)hChn->count1 - 1; i >= (int32_t)val2; i--)
               {
                  /* saturation to 31-bit integer */
                  /* get the filter output at current sample */
                  if (hChn->saSettings.resBw == BSAT_ResBw_e100khz)
                     val64_var2 = val64 >> 3;
                  else
                     val64_var2 = val64 >> 2;

                  if (val64_var2 > 0x7FFFFFFF)
                     val = 0x7FFFFFFF; /* (1<<31)-1 */
                  else
                     val = (uint32_t)val64_var2;  /* Filter's new output */

                  /* Update the moving sum for the next sample, subtract the current one, then add the new one */
                  val64 -=  pSaBuf[i];
                  val64 +=  pSaBuf[i-val2];

                  /* Update the buffer with the current sample filter output */
                  pSaBuf[i] = val;
               }

               /* saturation to 31-bit integer */
               if (hChn->saSettings.resBw == BSAT_ResBw_e100khz)
                  val64_var2 = val64 >> 3;
               else
                  val64_var2 = val64 >> 2;
               if (val64_var2 > 0x7FFFFFFF)
                  val = 0x7FFFFFFF; /* (1<<31)-1 */
               else
                  val = (uint32_t)val64_var2;

               /* write the first val2 elements with the same value */
               for (j = 0; j <= val2; j++)
                  pSaBuf[j] = val;
            }

            /* VBW low pass filtering; done for all RBWs */
            k = (int32_t)hChn->saSettings.vidBw;
            if ((k > 0) && (hChn->count1 > 1))
            {
               /* at this point, k is 1 or 2 */
               k = k * k;   /* k is now 1 or 4 */
               /* forward filtering */
               for (val = 1; val <= hChn->count1; val++)
                  pSaBuf[val] = (uint32_t)((int32_t)pSaBuf[val-1] + (int32_t)((int32_t)(pSaBuf[val] - pSaBuf[val-1]) / k));

               /* backward filtering */
               for (val = (hChn->count1 - 1); ; val--)
               {
                  pSaBuf[val] = (uint32_t)((int32_t)pSaBuf[val+1] + (int32_t)((int32_t)(pSaBuf[val] - pSaBuf[val+1]) / k));
                  if (val == 0)
                     break;
               }
            }

            /* display detector */
            if (hChn->saSettings.resBw == BSAT_ResBw_e100khz)
               displayDecimRate = 5;
            else if (hChn->saSettings.resBw == BSAT_ResBw_e300khz)
               displayDecimRate = 4;
            else
               displayDecimRate = 1;

            if (displayDecimRate > 1)
            {
               i = 0;
               for (val = 0; val <= (hChn->count1-displayDecimRate); val += displayDecimRate)
               {

#if 0
                  val2 = pSaBuf[val];
                  val64 = (uint64_t)val2;

                  if (hChn->saSettings.dispDetectorMode != BSAT_DisplayDetectorMode_eSample)
                  {
                     for (j = (val+1); j < (val+displayDecimRate); j++)
                     {
                        if (hChn->saSettings.dispDetectorMode == BSAT_DisplayDetectorMode_eMin)
                        {
                           if (pSaBuf[j] < val2)
                              val2 = pSaBuf[j];
                        }
                        else if (hChn->saSettings.dispDetectorMode == BSAT_DisplayDetectorMode_eMax)
                        {
                           if (pSaBuf[j] > val2)
                              val2 = pSaBuf[j];
                        }
                        else /* BSAT_DisplayDetectorMode_eMean */
                        {
                           val64 += (uint64_t)pSaBuf[j];
                        }
                     }
                  }

                  if (hChn->saSettings.dispDetectorMode == BSAT_DisplayDetectorMode_eMean)
                     val2 = (uint32_t)(val64 / (uint64_t)displayDecimRate);
#else
                  val64 = 0;
                  for (j = val; j < (val+displayDecimRate); j++)
                     val64 += (uint64_t)pSaBuf[j];

                  val2 = (uint32_t)(val64 / (uint64_t)displayDecimRate);
#endif

                  pSaBuf[i] = val2;
                  i++;
               }
               hChn->count1 = i;
               hChn->saStatus.freqStep *= displayDecimRate;

               if (hChn->saSettings.resBw == BSAT_ResBw_e300khz)
                  hChn->saStatus.freqStep *= 4;
            }

            /* take log2 of each sample */
            for (val = 0; val < hChn->count1; val++)
            {
               /* keep 3 fractional bits out of log2 function */
               pSaBuf[val] = BSAT_g1_P_log2Approx_isr(pSaBuf[val]) >> 1;
            }

            /* compact the SA data by storing each data sample as 8-bits */
            for (val = 0; val < hChn->count1; val++)
            {
               val8 = (uint8_t)pSaBuf[val];
               pSaBuf8[val] = val8;
#if 0
printf("%u) %u %u 0x%X\n", val, hChn->saSettings.startFreq + (val*hChn->saStatus.freqStep), pSaBuf8[val], pSaBuf[val]);
#endif
            }

            hChn->saStatus.status = BERR_SUCCESS;
            hChn->saStatus.bValid = true;
            hChn->saStatus.numSamples = hChn->count1;
            hChn->functState = 100;
            break;

         case 5: /* RBW >= 1 MHz */
            val = (BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_SDS_FE_AIF) >> 8); /* agf_int_val */
            val = (val >> 8) & 0x0000FFFF;
            val2 = val * val;
            val64 = (uint64_t)0x80000000;
            val64 *= (uint64_t)524288;
            val64 /= (uint64_t)val2;  /* val64=2^50/val^2 */
            val = (uint32_t)val64 >> hChn->count2;
            if ((int32_t)val < 1)
               val = 1;
            pSaBuf[hChn->count1++] = val;
            BSAT_g1_P_FreezeChanAgc_isr(h, false);
            if (hChn->count1 > (4096-1))
               hChn->functState = 100; /* out of memory */
            else
            {
               hChn->acqSettings.freq += hChn->saStatus.freqStep;
               if (hChn->acqSettings.freq > hChn->saSettings.stopFreq)
                  hChn->functState = 4;
               else
                  hChn->functState = 1;
            }
            break;

         case 100:
#if BSAT_DEBUG_SA
#if defined(BSAT_HAS_LEAP)
            BKNI_Printf("sa done: n=%u, t=%u, freqStep=%u\n", hChn->count1, bos_get_usecs_elapsed(t0), hChn->saStatus.freqStep);
#else
            BKNI_Printf("sa done: n=%u, freqStep=%u\n", hChn->count1, hChn->saStatus.freqStep);
#endif
#endif
            bDone = true;
            hChn->functState = -1;
            BSAT_g1_P_IndicateAcqDone_isr(h);
            break;

         default:
            bDone = true;
            break;
      }
   }

   return BERR_SUCCESS;
}
