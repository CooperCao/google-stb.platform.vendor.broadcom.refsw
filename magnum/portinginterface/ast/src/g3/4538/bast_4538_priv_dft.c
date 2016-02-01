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
#include "bast.h"
#include "bast_priv.h"
#include "bast_g3_priv.h"

BDBG_MODULE(bast_4538_priv_dft);


#define BAST_DEBUG_DFT(x) /* x */
#define BAST_DEBUG_PEAK_SCAN(x) /* x */
/* #define BAST_DONT_USE_DFT_IRQ */

/* local functions */
BERR_Code BAST_g3_P_DftPeakScanStateMachine(BAST_ChannelHandle h);


/******************************************************************************
 BAST_g3_P_SetDftDdfsFcw()
******************************************************************************/
BERR_Code BAST_g3_P_DftSetDdfsFcw_isr(BAST_ChannelHandle h)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   uint32_t P_hi, P_lo, Q_hi, Q_lo;

   BMTH_HILO_32TO64_Mul(hChn->acqParams.symbolRate, 32768 << 1, &P_hi, &P_lo);
   BMTH_HILO_64TO64_Div32(P_hi, P_lo, hChn->sampleFreq, &Q_hi, &Q_lo);
   Q_lo = (Q_lo + 1) >> 1;
   /* BDBG_MSG(("DFT_DDFS_FCW=0x%X", Q_lo)); */
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_DFT_DDFS_FCW, &Q_lo);
   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_g3_P_DftDone_isr()
******************************************************************************/
void BAST_g3_P_DftDone_isr(void *p, int param)
{
#ifndef BAST_DONT_USE_DFT_IRQ
   BAST_ChannelHandle h = (BAST_ChannelHandle)p;
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   uint32_t val;

   BINT_DisableCallback_isr(hChn->hDftDoneCb);
   BINT_ClearCallback_isr(hChn->hDftDoneCb);

   BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_DFT_STATUS, &val);
   if ((val & 0x06) == 0x06)
   {
      hChn->failFunct(h);
   }
   else
   {
      BDBG_ERR(("BAST_g3_P_DftDone_isr: invalid DFT_STATUS=0x%X\n", val));
   }
#endif /* BAST_DONT_USE_DFT_IRQ */
}


/******************************************************************************
 BAST_g3_P_DftWaitForDone_isr()
******************************************************************************/
BERR_Code BAST_g3_P_DftWaitForDone_isr(BAST_ChannelHandle h)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   uint32_t val;

   BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_DFT_STATUS, &val);
   if (((val & 0x06) == 0x06) || (hChn->dft_done_timeout > 3))
   {
      if (hChn->dft_done_timeout > 1)
      {
         BDBG_ERR(("dft_done_timeout=%d (DFT_STATUS=0x%X)", hChn->dft_done_timeout, val));
      }
      return hChn->failFunct(h);
   }

   hChn->dft_done_timeout++;
   BDBG_ERR(("DFT not yet ready (DFT_STATUS=0x%X)...", val));
   return BAST_g3_P_EnableTimer_isr(h, BAST_TimerSelect_eBaudUsec, 100, BAST_g3_P_DftWaitForDone_isr);
}


/******************************************************************************
 BAST_g3_P_DftStartAndWaitForDone_isr()
******************************************************************************/
BERR_Code BAST_g3_P_DftStartAndWaitForDone_isr(BAST_ChannelHandle h, BAST_g3_FUNCT funct)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   uint32_t range_start, range_end, P_hi, P_lo, Q_hi, Q_lo, i, n, retry_count = 0;
   uint32_t val;

   hChn->failFunct = funct;

   /* start the DFT engine */
   retry:
   val = 1;
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_DFT_CTRL0, &val);

   /* time it takes for DFT to run is 33.4K Fs cycles * 2^N with N decimation filters enabled */
   n = BAST_g3_P_GetNumDecimatingFilters_isr(h);

   BMTH_HILO_32TO64_Mul(33400 * (1 << n), 2000000, &P_hi, &P_lo);
   BMTH_HILO_64TO64_Div32(P_hi, P_lo, hChn->sampleFreq, &Q_hi, &Q_lo);
   Q_lo = (Q_lo + 1) >> 1;
   BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_DFT_RANGE_START, &range_start);
   BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_DFT_RANGE_END, &range_end);
   val = (range_end - range_start) >> 4;
   if (val > 0)
      Q_lo *= (val+1);
   if (Q_lo == 0)
      Q_lo = 400;
   /* BAST_DEBUG_DFT(BDBG_MSG(("%d decimation filters, waiting %u usecs", n, Q_lo))); */

   /* wait at least 70 Fs clocks from DFT_CTRL0.0=0 to DFT_CTRL.0=1 */
   val = 0;
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_DFT_CTRL0, &val);

   for (i = 0; i < 50; i++)
   {
      BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_DFT_STATUS, &val);
      if (val & 0x07)
         break;
   }
   if ((val & 0x07) == 0)
   {
      BDBG_WRN(("dft_status should not be 0, trying again..."));
      if (retry_count++ < 10)
         goto retry;
      BDBG_ERR(("ERROR: dft_status=0!"));
      return BERR_TRACE(BAST_ERR_HAB_CMD_FAIL);
   }

#ifdef BAST_DONT_USE_DFT_IRQ
   hChn->dft_done_timeout = 0;
   return BAST_g3_P_EnableTimer_isr(h, BAST_TimerSelect_eBaudUsec, Q_lo, BAST_g3_P_DftWaitForDone_isr);
#else
   BINT_ClearCallback_isr(hChn->hDftDoneCb);
   BINT_EnableCallback_isr(hChn->hDftDoneCb);
   return BERR_SUCCESS;
#endif
}


/******************************************************************************
 BAST_g3_P_DftSearchCarrierStateMachine_isr()
******************************************************************************/
BERR_Code BAST_g3_P_DftSearchCarrierStateMachine_isr(BAST_ChannelHandle h)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   BAST_g3_P_Handle *hDev = h->pDevice->pImpl;
   BERR_Code retCode = BERR_SUCCESS;
   uint32_t val, hb, peak_pow, total_pow;
   uint32_t peak_bin, start, stop;

   while (hChn->dft_funct_state != 0xFFFFFFFF)
   {
      switch (hChn->dft_funct_state)
      {
         case 0:
            /* coarse freq step is 10% of Fb */
            hChn->dftFreqEstimateStatus |= BAST_G3_CONFIG_FREQ_ESTIMATE_STATUS_COARSE;
            hChn->tunerIfStepSize = hChn->acqParams.symbolRate / 10;
            hChn->tunerIfStep = (int32_t)(-(hDev->searchRange) - hChn->tunerIfStepSize);
            hChn->tunerIfStepMax = (int32_t)(hDev->searchRange);
            hChn->dft_current_max_pow = 0;

            if (hChn->tunerIfStepSize > hDev->searchRange)
               hChn->dft_funct_state = 6; /* do fine search */
            else
            {
               hChn->dft_funct_state = 1;
               BAST_DEBUG_DFT(BDBG_MSG(("\nDFT(coarse): StepSize=%d", hChn->tunerIfStepSize)));
            }
            break;

         case 1:
            if (hChn->tunerIfStep <= hChn->tunerIfStepMax)
            {
               hChn->tunerIfStep += (int32_t)hChn->tunerIfStepSize;
               hChn->dft_funct_state = 2;
               return BAST_g3_P_TunerQuickTune_isr(h, BAST_g3_P_DftSearchCarrierStateMachine_isr);
            }
            else
               hChn->dft_funct_state = 6;
            break;

         case 2:
            hChn->count2 = 0; /* count2 = sum of total_pow */
            hChn->dft_funct_state = 3;
            break;

         case 3:
            val = 0x7F8;
            BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_DFT_RANGE_START, &val);

            val = 0x07;
            BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_DFT_RANGE_END, &val);

            val = 0x02;
            BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_DFT_CTRL0, &val);

            val = 0x00;
            BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_DFT_CTRL0, &val);

            hb = BAST_g3_P_GetNumDecimatingFilters_isr(h);
#ifndef BAST_HAS_WFE
            if (hb <= 2)
               val = 0x2F7;
            else if (hb <= 3)
               val = 0x217;
            else
               val = 0x317;
#else
            val = 0x2F7;
#endif
            BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_DFT_CTRL1, &val);

            BAST_CHK_RETCODE(BAST_g3_P_DftSetDdfsFcw_isr(h));
            hChn->dft_funct_state = 4;
            return BAST_g3_P_DftStartAndWaitForDone_isr(h, BAST_g3_P_DftSearchCarrierStateMachine_isr);

         case 4:
            BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_DFT_PEAK_POW, &peak_pow);
            BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_DFT_TOTAL_POW, &total_pow);
            BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_DFT_PEAK_BIN, &peak_bin);
            BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_DFT_RANGE_START, &start);
            BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_DFT_RANGE_END, &stop);

            /* software workaround for bug in total_pow */
            hb = BAST_g3_P_GetNumDecimatingFilters_isr(h);
            val = 0;
            if (hb == 3)
               val = 1;
            else if (hb == 4)
               val = 2;
            else if (hb == 5)
               val = 3;
            if (val)
               total_pow = total_pow >> val;

            BAST_DEBUG_DFT(BDBG_MSG(("%d Hz: [%x,%x] %08X %08X %08X", hChn->tunerIfStep, start, stop, peak_pow, total_pow, peak_bin)));

#if 0
            if (peak_pow > (total_pow>>1))
            {
#endif
               val = peak_pow - ((total_pow - peak_pow) >> 5);
               if (val & 0x80000000) /* check for overflow */
                  val = 0;
#if 0
            }
            else
               val = 0;
#endif
            BAST_DEBUG_DFT(BDBG_MSG(("%d Hz: max_peak=0x%08X, sum_total=0x%08X, pow=%u", hChn->tunerIfStep, hChn->maxCount1, hChn->count2, val)));
            if (val > hChn->dft_current_max_pow)
            {
               hChn->dft_current_max_pow = val;
               hChn->dftFreqEstimate = hChn->tunerIfStep;
            }
            hChn->dft_funct_state = 1;
            break;

         case 6:
            if (hChn->dftFreqEstimateStatus & BAST_G3_CONFIG_FREQ_ESTIMATE_STATUS_FINE)
               hChn->dft_funct_state = 7;
            else
            {
               /* reset baud loop integrator */
               val = 0;
               BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_BL_BRI, &val);

               hChn->dftFreqEstimateStatus |= BAST_G3_CONFIG_FREQ_ESTIMATE_STATUS_FINE;
               hChn->tunerIfStep = hChn->dftFreqEstimate - (int32_t)hChn->tunerIfStepSize;
               if (hChn->tunerIfStep < (int32_t)-(hDev->searchRange))
                  hChn->tunerIfStep = -(hDev->searchRange);
               hChn->tunerIfStepMax = hChn->dftFreqEstimate + (int32_t)hChn->tunerIfStepSize;
               if (hChn->tunerIfStepMax > (int32_t)hDev->searchRange)
                  hChn->tunerIfStepMax = (int32_t)(hDev->searchRange);
               hChn->tunerIfStepSize = hChn->acqParams.symbolRate / 20;
               if (hChn->tunerIfStepSize > 1000000)
                  hChn->tunerIfStepSize = 1000000;
               hChn->tunerIfStep -= hChn->tunerIfStepSize;

               hChn->dft_current_max_pow = 0;
               hChn->dft_funct_state = 1;
               BAST_DEBUG_DFT(BDBG_MSG(("DFT(fine): coarse_est=%d, StepSize=%d", hChn->dftFreqEstimate, hChn->tunerIfStepSize)));
            }
            break;

         case 7:
            hChn->dftFreqEstimateStatus |= BAST_G3_CONFIG_FREQ_ESTIMATE_STATUS_DONE;
            BAST_CHK_RETCODE(BAST_g3_P_LogTraceBuffer_isr(h, BAST_TraceEvent_eFreqEstDone));
            BDBG_MSG(("DFT freq estimate (chan %d) = %d", h->channel, hChn->dftFreqEstimate));
            hChn->tunerIfStep = 0;
            if (hChn->tunerCtl & BAST_G3_CONFIG_TUNER_CTL_BYPASS_DFT_FREQ_EST)
            {
               BDBG_WRN(("DFT: did not transfer freq offset to the LO"));
            }
            else if (hChn->tunerCtl & BAST_G3_CONFIG_TUNER_CTL_PRESERVE_COMMANDED_FREQ)
            {
               BDBG_MSG(("DFT: transferring freq offset to FLIF"));
               BAST_CHK_RETCODE(BAST_g3_P_SetFlifOffset_isr(h, hChn->dftFreqEstimate));
            }
            else
               hChn->tunerIfStep = hChn->dftFreqEstimate;

            hChn->dft_funct_state = 8;
            return BAST_g3_P_TunerQuickTune_isr(h, BAST_g3_P_DftSearchCarrierStateMachine_isr);

         case 8:
            hChn->dft_funct_state = 9;
            /* tuner settling delay */
            return BAST_g3_P_EnableTimer_isr(h, BAST_TimerSelect_eBaudUsec, 20000, BAST_g3_P_DftSearchCarrierStateMachine_isr);

         case 9:
            BAST_CHK_RETCODE(BAST_g3_P_LogTraceBuffer_isr(h, BAST_TraceEvent_eRetuneDone));

#ifdef BAST_HAS_WFE
            BAST_g3_P_SetNotch_isr(h);
#endif

            val = 0;
            if ((hChn->tunerCtl & BAST_G3_CONFIG_TUNER_CTL_PRESERVE_COMMANDED_FREQ) == 0)
               BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_CL_FLIF, &val);
            BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_CL_FLI, &val);
            BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_CL_PLI, &val);
            hChn->dft_funct_state = 0xFFFFFFFF;
            break;

         default:
            /* should never get here */
            BDBG_ERR(("BAST_g3_P_DftSearchCarrierStateMachine_isr(): invalid funct_state (%d)", hChn->dft_funct_state));
            BDBG_ASSERT(0);
      }
   }

   BAST_CHK_RETCODE(hChn->passFunct(h));

   done:
   return retCode;
}


/******************************************************************************
 BAST_g3_P_DftOqpskSearchCarrierStateMachine_isr()
******************************************************************************/
BERR_Code BAST_g3_P_DftOqpskSearchCarrierStateMachine_isr(BAST_ChannelHandle h)
{
   static const uint32_t script_dft_oqpsk_2[] =
   {
      BAST_SCRIPT_WRITE(BCHP_SDS_DFT_RANGE_START, 0),
      BAST_SCRIPT_WRITE(BCHP_SDS_DFT_RANGE_END, 0x7FF),
      BAST_SCRIPT_WRITE(BCHP_SDS_DFT_CTRL0, 0x02),
      BAST_SCRIPT_WRITE(BCHP_SDS_DFT_CTRL0, 0),
      BAST_SCRIPT_WRITE(BCHP_SDS_DFT_CTRL1, 0xD37), /* input to DFT is ADC, offset QPSK */
      BAST_SCRIPT_EXIT
   };

   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   BAST_g3_P_Handle *hDev = h->pDevice->pImpl;
   BERR_Code retCode;
   uint32_t peak_pow, total_pow, peak_bin, P_hi, P_lo, Q_hi, Q_lo;

   while (hChn->dft_funct_state <= 3)
   {
      switch (hChn->dft_funct_state)
      {
         case 0:
            hChn->dftFreqEstimateStatus |= BAST_G3_CONFIG_FREQ_ESTIMATE_STATUS_OQPSK;

            if (hChn->acqParams.symbolRate > 11000000)
               hChn->tunerIfStepSize = 3000000; /* 1.5MHz filter * 2 */
            else if (hChn->acqParams.symbolRate > 4800000)
               hChn->tunerIfStepSize = 2600000; /* 1.3MHz filter * 2 */
            else
               hChn->tunerIfStepSize = 2000000; /* 1.0MHz filter * 2 */

            hChn->dft_count1 = 0; /* dft_count1 = if step num */
            hChn->count2 = 0; /* count2 = max snr */
            hChn->dft_funct_state = 1;
            break;

         case 1:
            if (hChn->dft_count1)
               hChn->tunerIfStep = (hChn->tunerIfStepSize >> 1) + (((hChn->dft_count1 - 1) >> 1) * hChn->tunerIfStepSize);
            else
               hChn->tunerIfStep = 0;

            /* check for done */
            if (hChn->tunerIfStep > (int32_t)(hDev->searchRange))
               hChn->dft_funct_state = 7;
            else
            {
               if ((hChn->dft_count1 & 1) == 0)
                  hChn->tunerIfStep = -(hChn->tunerIfStep);

               BAST_CHK_RETCODE(BAST_g3_P_SetFlifOffset_isr(h, hChn->tunerIfStep));
               hChn->dft_funct_state = 2;
            }
            break;

         case 2:
            BAST_CHK_RETCODE(BAST_g3_P_ProcessScript_isrsafe(h, script_dft_oqpsk_2));
            BAST_CHK_RETCODE(BAST_g3_P_DftSetDdfsFcw_isr(h));
            hChn->dft_funct_state = 3;
            return BAST_g3_P_DftStartAndWaitForDone_isr(h, BAST_g3_P_DftOqpskSearchCarrierStateMachine_isr);

         case 3:
            BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_DFT_PEAK_POW, &peak_pow);
            BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_DFT_TOTAL_POW, &total_pow);
            BAST_DEBUG_DFT(BDBG_MSG(("DFT OQPSK: %d Hz: peak_pow=%u, total_pow=%u", hChn->tunerIfStep, peak_pow, total_pow)));
            BMTH_HILO_32TO64_Mul(peak_pow, 2048, &P_hi, &P_lo);
            BMTH_HILO_64TO64_Div32(P_hi, P_lo, total_pow, &Q_hi, &Q_lo);

            if (Q_lo > hChn->count2)
            {
               hChn->count2 = Q_lo;
               BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_DFT_PEAK_BIN, &peak_bin);
               peak_bin &= 0x00000FFF;
               if (peak_bin < 0x400)
               {
                  BMTH_HILO_32TO64_Mul(hChn->sampleFreq, peak_bin, &P_hi, &P_lo);
                  BMTH_HILO_64TO64_Div32(P_hi, P_lo, 65536, &Q_hi, &Q_lo);
                  hChn->dftFreqEstimate = Q_lo;
               }
               else
               {
                  BMTH_HILO_32TO64_Mul(hChn->sampleFreq, 2048 - peak_bin, &P_hi, &P_lo);
                  BMTH_HILO_64TO64_Div32(P_hi, P_lo, 65536, &Q_hi, &Q_lo);
                  hChn->dftFreqEstimate = (int32_t)-Q_lo;
               }
               BAST_DEBUG_DFT(BDBG_MSG(("   -> count2=%u, bin=%u, Q_lo=%u, delta=%d", hChn->count2, peak_bin, Q_lo, hChn->dftFreqEstimate)));
               hChn->dftFreqEstimate += hChn->tunerIfStep;
            }
            hChn->dft_count1++;
            hChn->dft_funct_state = 1;
            break;

         default:
            /* should never get here */
            BDBG_ERR(("BAST_g3_P_DftOqpskSearchCarrierStateMachine_isr(): invalid funct_state (%d)", hChn->dft_funct_state));
            BDBG_ASSERT(0);
      }
   }

   BDBG_ASSERT(hChn->dft_funct_state == 7);
   retCode = BAST_g3_P_DftSearchCarrierStateMachine_isr(h);

   done:
   return retCode;
}


/******************************************************************************
 BAST_g3_P_DftSearchCarrier_isr()
******************************************************************************/
BERR_Code BAST_g3_P_DftSearchCarrier_isr(BAST_ChannelHandle h, BAST_g3_FUNCT funct)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   uint32_t nDecFilters;
#ifndef BAST_HAS_WFE
   uint32_t mdiv, ndiv;
#endif

   hChn->passFunct = funct; /* passFunct = function to call after DFT freq estimate */

   hChn->dftFreqEstimate = 0;
   hChn->dftFreqEstimateStatus = BAST_G3_CONFIG_FREQ_ESTIMATE_STATUS_START;

   if ((nDecFilters = BAST_g3_P_GetNumDecimatingFilters_isr(h)) >= 4)
   {
      /* dft h/w problem with 4 or more decimation filters enabled */
#ifndef BAST_HAS_WFE
      while (nDecFilters >= 4)
      {
         /* divide Fs by 2 */
         BAST_g3_P_GetSampleFreqMN_isr(h, &ndiv, &mdiv);
         mdiv = mdiv << 1;
         BAST_g3_P_SetSampleFreq_isr(h, ndiv, mdiv);

         BAST_g3_P_ConfigBl_isr(h);
         nDecFilters = BAST_g3_P_GetNumDecimatingFilters_isr(h);
         BDBG_MSG(("Change Fs to %u, nDecFilters=%d", hChn->sampleFreq, nDecFilters));
      }
#endif
   }
   BDBG_MSG(("%d decimation filters enabled", nDecFilters));

#ifndef BAST_EXCLUDE_EXT_TUNER
   if (hChn->bExternalTuner)
      hChn->dft_funct_state = 6;
   else
#endif
   {
      hChn->dft_funct_state = 0;
      if ((hChn->acqParams.carrierFreqOffset) ||
          (hChn->tunerCtl & BAST_G3_CONFIG_TUNER_CTL_BYPASS_DFT_FREQ_EST))
      {
         if (hChn->acqCount == 0)
         {
            /* warm start */
            hChn->dftFreqEstimate = hChn->acqParams.carrierFreqOffset;
            hChn->dft_funct_state = 7;
         }
         else
            hChn->acqParams.carrierFreqOffset = 0; /* warm start failed previously, so do cold start */
      }

      if (hChn->acqParams.acq_ctl & BAST_ACQSETTINGS_OQPSK)
         return BAST_g3_P_DftOqpskSearchCarrierStateMachine_isr(h);
   }
   return BAST_g3_P_DftSearchCarrierStateMachine_isr(h);
}


/******************************************************************************
 BAST_g3_P_DftPeakScanStateMachine()
******************************************************************************/
BERR_Code BAST_g3_P_DftPeakScanStateMachine(BAST_ChannelHandle h)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   uint32_t val, P_hi, P_lo, Q_hi, Q_lo, total_pow, peak_pow;

   next_state:
   BAST_DEBUG_PEAK_SCAN(BDBG_MSG(("BAST_g3_P_DftPeakScanStateMachine(): state=%d", hChn->dft_funct_state)));
   switch (hChn->dft_funct_state)
   {
      case 0:
         hChn->dft_funct_state = 1;
         BAST_g3_P_DftSetDdfsFcw_isr(h);
         return BAST_g3_P_DftStartAndWaitForDone_isr(h, BAST_g3_P_DftPeakScanStateMachine);

      case 1:
         BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_DFT_PEAK_POW, &peak_pow);
         BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_DFT_TOTAL_POW, &total_pow);

         BAST_DEBUG_PEAK_SCAN(BDBG_MSG(("PeakScan: %d sym/s: peak_pow=0x%08X, total_pow=0x%08X", hChn->acqParams.symbolRate, peak_pow, total_pow)));
         if (peak_pow > hChn->peakScanMaxPower)
         {
            hChn->peakScanMaxPower = peak_pow;
            BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_DFT_PEAK_BIN, &val);
            hChn->peakScanMaxPeakBin = val & 0x00000FFF;
            BAST_DEBUG_PEAK_SCAN(BDBG_MSG(("PeakScan: peak_bin=0x%X", hChn->peakScanMaxPeakBin)));
            hChn->peakScanSymRateEst = hChn->acqParams.symbolRate;
         }
         hChn->acqParams.symbolRate += hChn->peakScanIfStepSize;
         if (hChn->acqParams.symbolRate <= hChn->peakScanSymRateMax)
         {
            hChn->dft_funct_state = 0;
            goto next_state;
         }

         if (hChn->peakScanSymRateMax)
         {
            if (hChn->peakScanMaxPeakBin > (hChn->peakScanDftSize >> 1))
            {
               BMTH_HILO_32TO64_Mul((uint32_t)(hChn->peakScanDftSize - hChn->peakScanMaxPeakBin), hChn->sampleFreq, &P_hi, &P_lo);
               BMTH_HILO_64TO64_Div32(P_hi, P_lo, (uint32_t)hChn->peakScanDftSize * 8, &Q_hi, &Q_lo);
               Q_lo = (Q_lo + 1) >> 1;
               hChn->peakScanOutput = hChn->peakScanSymRateEst - Q_lo;
            }
            else  /* positive frequency */
            {
               BMTH_HILO_32TO64_Mul((uint32_t)hChn->peakScanMaxPeakBin, hChn->sampleFreq, &P_hi, &P_lo);
               BMTH_HILO_64TO64_Div32(P_hi, P_lo, (uint32_t)hChn->peakScanDftSize * 8, &Q_hi, &Q_lo);
               Q_lo = (Q_lo + 1) >> 1;
               hChn->peakScanOutput = hChn->peakScanSymRateEst + Q_lo;
            }
         }
         else
         {
            hChn->peakScanOutput = hChn->peakScanMaxPeakBin;
         }
         BAST_DEBUG_PEAK_SCAN(BDBG_MSG(("peak_bin=%d, peak_symrate=%d, output=%d", hChn->peakScanMaxPeakBin, hChn->peakScanSymRateEst, hChn->peakScanOutput)));

         BAST_g3_P_DisableChannelInterrupts_isr(h, false, false);

         hChn->peakScanStatus &= ~BAST_PEAK_SCAN_STATUS_ERROR;
         hChn->peakScanStatus |= BAST_PEAK_SCAN_STATUS_DONE;
         hChn->acqState = BAST_AcqState_eIdle;
         BKNI_SetEvent(hChn->hPeakScanEvent);
         break;

      default:
         break;
   }
   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_g3_P_DftPeakScan_isr()
******************************************************************************/
BERR_Code BAST_g3_P_DftPeakScan_isr(BAST_ChannelHandle h)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   uint32_t val, ctrl1;

   /* reset the DFT */
   val = 0x02;
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_DFT_CTRL0, &val);
   val = 0x00;
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_DFT_CTRL0, &val);

   /* configure the DFT */
   if ((hChn->peakScanSymRateMin == 0) || (hChn->peakScanSymRateMax == 0))
   {
      hChn->peakScanSymRateMax = hChn->peakScanSymRateMin = 0;
      val = (uint32_t)(hChn->dftRangeStart & 0x7FF);
      BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_DFT_RANGE_START, &val);
      val = (uint32_t)(hChn->dftRangeEnd & 0x7FF);
      BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_DFT_RANGE_END, &val);

      /* set to 6 HB */
      BAST_g3_P_ReadModifyWriteRegister_isrsafe(h, BCHP_SDS_FE_FILTCTL, 0xFFFFFF00, 0x60);

      ctrl1 = 0x013D;
   }
   else
   {
      val = 0;
      BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_DFT_RANGE_START, &val);
      val = 0xFF;
      BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_DFT_RANGE_END, &val);

      ctrl1 = 0x0197;
   }

   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_DFT_CTRL1, &ctrl1);

   hChn->acqParams.symbolRate = hChn->peakScanSymRateMin;
   hChn->peakScanDftSize = 256 << (val & 0x03);
   hChn->peakScanIfStepSize = (uint32_t)((hChn->sampleFreq + 64) >> 7); /* Fs/128 */
   hChn->peakScanMaxPower = 0;
   hChn->peakScanMaxPeakBin = 0;
   hChn->peakScanSymRateEst = hChn->peakScanSymRateMin;
   hChn->dft_funct_state = 0;
   return BAST_g3_P_DftPeakScanStateMachine(h);
}
