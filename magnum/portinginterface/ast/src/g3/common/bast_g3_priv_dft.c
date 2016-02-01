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

BDBG_MODULE(bast_g3_priv_dft);


#define BAST_DEBUG_DFT(x) /* x */
#define BAST_DEBUG_PEAK_SCAN 0


/* local functions */
BERR_Code BAST_g3_P_DftPeakScanStateMachine_isr(BAST_ChannelHandle h);
BERR_Code BAST_g3_P_DftPeakScanPsd1_isr(BAST_ChannelHandle h);
BERR_Code BAST_g3_P_DftPeakScanPsd2_isr(BAST_ChannelHandle h);


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
   uint32_t range_start, range_end, val, P_hi, P_lo, Q_hi, Q_lo, i, n, retry_count = 0;

   hChn->failFunct = funct;

   /* start the DFT engine */
   retry:
   val = 1;
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_DFT_CTRL0, &val);

   n = BAST_g3_P_GetNumDecimatingFilters_isr(h);
   BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_DFT_RANGE_START, &range_start);
   BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_DFT_RANGE_END, &range_end);
#if 1
   /* time it takes for DFT to run is 33.4K Fs cycles * 2^N with N decimation filters enabled */
   BMTH_HILO_32TO64_Mul(33400 * (1 << n), 2000000, &P_hi, &P_lo);
   BMTH_HILO_64TO64_Div32(P_hi, P_lo, hChn->sampleFreq, &Q_hi, &Q_lo);
   Q_lo = (Q_lo + 1) >> 1;
   val = (range_end - range_start) >> 4;
   if (val > 0)
      Q_lo *= (val+1);
   if (Q_lo == 0)
      Q_lo = 400;
#else
   /* dft scan time = dft_size * 2^hb * 16 * (DFT_RANGE/16) / Fs */
   BMTH_HILO_32TO64_Mul((range_end - range_start + 1) * (1<<n), 2048000000, &P_hi, &P_lo);
   BMTH_HILO_64TO64_Div32(P_hi, P_lo, hChn->sampleFreq, &Q_hi, &Q_lo);
   Q_lo += 1000;
#endif
   /*BDBG_ERR(("%d decimation filters, waiting %u usecs", n, Q_lo));*/

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

   hChn->dft_done_timeout = 0;
   return BAST_g3_P_EnableTimer_isr(h, BAST_TimerSelect_eBaudUsec, Q_lo, BAST_g3_P_DftWaitForDone_isr);
}


/******************************************************************************
 BAST_g3_P_DftSearchCarrierStateMachine_isr()
******************************************************************************/
BERR_Code BAST_g3_P_DftSearchCarrierStateMachine_isr(BAST_ChannelHandle h)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   BAST_g3_P_Handle *hDev = h->pDevice->pImpl;
   BERR_Code retCode = BERR_SUCCESS;
   uint32_t peak_pow, total_pow, peak_bin, val, start, stop, hb;

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
#if 0          /* if LPF is narrowed, FLIF stepping is OK */
               BAST_CHK_RETCODE(BAST_g3_P_SetFlifOffset_isr(h, hChn->tunerIfStep));
#else
               return BAST_g3_P_TunerQuickTune_isr(h, BAST_g3_P_DftSearchCarrierStateMachine_isr);
#endif
            }
            else
               hChn->dft_funct_state = 6;
            break;

         case 2:
            hChn->dft_count1 = 0; /* dft_count1 = current number of passes */
            hChn->count2 = 0; /* count2 = sum of total_pow */
            hChn->dft_funct_state = 3;
            break;

         case 3:
            val = hChn->dft_count1 ? 0x7F0 : 0;
            BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_DFT_RANGE_START, &val);
            hChn->dft_funct_state = 31;
#if 0
            return BAST_g3_P_EnableTimer_isr(h, BAST_TimerSelect_eBaudUsec, 1, BAST_g3_P_DftSearchCarrierStateMachine_isr);
#else
            break;
#endif

         case 31:
            BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_DFT_RANGE_START, &val);
            val |= 0x0F;
            BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_DFT_RANGE_END, &val);

            val = 0x02;
            BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_DFT_CTRL0, &val);
            hChn->dft_funct_state = 32;
#if 0
            return BAST_g3_P_EnableTimer_isr(h, BAST_TimerSelect_eBaudUsec, 1, BAST_g3_P_DftSearchCarrierStateMachine_isr);
#else
            break;
#endif

         case 32:
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
            BAST_DEBUG_DFT(BDBG_MSG(("%d Hz: [%x,%x] %08X %08X %08X", hChn->tunerIfStep, start, stop, peak_pow, total_pow, peak_bin)));
            if (hChn->dft_count1)
            {
               if (peak_pow > hChn->maxCount1)
                  goto save_peak;
            }
            else
            {
               save_peak:
               hChn->maxCount1 = peak_pow;
            }

            hChn->count2 += total_pow;
            hChn->dft_count1++;
            if (hChn->dft_count1 == 1)
               hChn->dft_funct_state = 3;
            else
               hChn->dft_funct_state = 5;
            break;

         case 5:
               val = hChn->maxCount1 - ((hChn->count2 - hChn->maxCount1) >> 5);
               if (val & 0x80000000) /* check for overflow */
                  val = 0;
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
               hChn->dft_count1 = 0; /* dft_count1 = current max */
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
               BDBG_MSG(("DFT: did not transfer freq offset to the LO"));
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
#if BCHP_CHIP!=4517
   uint32_t nDecFilters;
#ifndef BAST_HAS_WFE
   uint32_t mdiv, ndiv;
#endif
#endif

   hChn->passFunct = funct; /* passFunct = function to call after DFT freq estimate */

   hChn->dftFreqEstimate = 0;
   hChn->dftFreqEstimateStatus = BAST_G3_CONFIG_FREQ_ESTIMATE_STATUS_START;

#if BCHP_CHIP!=4517
   if ((nDecFilters = BAST_g3_P_GetNumDecimatingFilters_isr(h)) >= 3)
   {
      /* dft h/w problem with 3 or more decimation filters enabled */
#ifndef BAST_HAS_WFE
      while (nDecFilters >= 3)
      {
         /* divide Fs by 2 */
         BAST_g3_P_GetSampleFreqMN_isr(h, &ndiv, &mdiv);
         mdiv = mdiv << 1;
         BAST_g3_P_SetSampleFreq_isr(h, ndiv, mdiv);

         BAST_g3_P_SetDecimationFilters_isr(h);
         BAST_g3_P_SetBfos_isr(h);
         nDecFilters = BAST_g3_P_GetNumDecimatingFilters_isr(h);
         BDBG_MSG(("Change Fs to %u, nDecFilters=%d", hChn->sampleFreq, nDecFilters));
      }
#endif
   }
   BDBG_MSG(("%d decimation filters enabled", nDecFilters));
#endif

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
 BAST_g3_P_DftPeakScanStateMachine_isr()
******************************************************************************/
BERR_Code BAST_g3_P_DftPeakScanStateMachine_isr(BAST_ChannelHandle h)
{
#define NEW_PEAK_POWER_RATIO 6
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   uint32_t val, P_hi, P_lo, Q_hi, Q_lo, total_pow, peak_pow, hb, peak_bin, sym_rate_est, ratio, i, j, sum, bin_pow;

   next_state:
   switch (hChn->dft_funct_state)
   {
      case 0:
         hChn->dft_funct_state = 1;
         BAST_g3_P_DftSetDdfsFcw_isr(h);
         return BAST_g3_P_DftStartAndWaitForDone_isr(h, BAST_g3_P_DftPeakScanStateMachine_isr);

      case 1:
      case 2:
      case 3:
      case 4:
         if ((hChn->peakScanSymRateMin != 0) && (hChn->peakScanSymRateMax != 0))
         {
            /* do this only for symbol rate scan */
            val = 0;
            BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_DFT_MEM_RADDR, &val);

            for (i = 0; i < 16; i++)
            {
               BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_DFT_MEM_RDATA, &val);
               if ((hChn->dft_funct_state == 1) || (val < hChn->dftBinPower[(hChn->dft_count1+i)%32]))
                  hChn->dftBinPower[(hChn->dft_count1+i)%32] = val;
            }
            if (hChn->dft_funct_state != 4)
            {
               hChn->dft_funct_state++;
               return BAST_g3_P_DftStartAndWaitForDone_isr(h, BAST_g3_P_DftPeakScanStateMachine_isr);
            }

            hChn->dft_count1 += 16; /* dft_count is a multiple of 16 */

            while ((hChn->dft_count1 - hChn->count1) > 9)
            {
               i = hChn->count1;
               bin_pow = hChn->dftBinPower[i%32];

               /* determine symbol rate associated with count1 */
               if ((i % 16) <= 6)
                  sym_rate_est = hChn->acqParams.symbolRate; /* current symbol rate */
               else
                  sym_rate_est = hChn->acqParams.symbolRate - hChn->peakScanIfStepSize; /* previous symbol rate */

               if (bin_pow > (NEW_PEAK_POWER_RATIO*hChn->dftBinPower[(i+2)%32]))
               {
                  for (j = sum = 0; j <= 9; j++)
                  {
                     sum += hChn->dftBinPower[(i+2+j)%32];
                  }
                  if (bin_pow > sum)
                  {
                     BMTH_HILO_32TO64_Mul(bin_pow, bin_pow, &P_hi, &P_lo);
                     BMTH_HILO_64TO64_Div32(P_hi, P_lo, sum, &Q_hi, &peak_pow);
                     if (peak_pow > hChn->peakScanMaxPower)
                     {
                        hChn->peakScanMaxPower = peak_pow;
                        hChn->peakScanMaxPeakBin = i % 16;
                        hChn->peakScanSymRateEst = sym_rate_est;
                        hChn->peakScanMaxPeakHb = BAST_g3_P_GetNumDecimatingFilters_isr(h);
#if BAST_DEBUG_PEAK_SCAN
                        BKNI_Printf("new peak: Fb=%u, bin=%d, pow=%u, ratio=", hChn->peakScanSymRateEst, hChn->peakScanMaxPeakBin, hChn->peakScanMaxPower);
                        if (hChn->dftBinPower[(i+2)%32] > 0)
                           BKNI_Printf("%u\n", bin_pow / hChn->dftBinPower[(i+2)%32]);
                        else
                           BKNI_Printf("-1\n");
#endif
                     }
                  }
               }

               hChn->count1++;
            }
         }
         else
         {
            BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_DFT_PEAK_POW, &peak_pow);
            BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_DFT_TOTAL_POW, &total_pow);
            BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_DFT_PEAK_BIN, &peak_bin);
            peak_bin &= 0x00000FFF;

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

#if 0 /*BAST_DEBUG_PEAK_SCAN*/
            BKNI_Printf("%d baud: hb=%d, peak=%08X, total=%08X, bin=%d", hChn->acqParams.symbolRate, hb, peak_pow, total_pow, peak_bin);
            if (total_pow != 0)
               BKNI_Printf(", peak/total=%f", (float)peak_pow/(float)total_pow);
#endif

            if (peak_pow > hChn->peakScanMaxPower)
            {
               hChn->peakScanMaxPower = peak_pow;
               hChn->peakScanMaxPeakBin = peak_bin;
               hChn->peakScanSymRateEst = hChn->acqParams.symbolRate;
               hChn->peakScanMaxPeakHb = hb;
            }

            if (total_pow > 0)
            {
               ratio = peak_pow / total_pow;
               if (ratio > hChn->peakScanMaxRatio)
               {
                  hChn->peakScanMaxRatioPower = peak_pow;
                  hChn->peakScanMaxRatio = ratio;
                  hChn->peakScanMaxRatioBin = peak_bin;
                  hChn->peakScanMaxRatioSymRate = hChn->acqParams.symbolRate;
                  hChn->peakScanMaxRatioHb = hb;
               }
            }
         }

         hChn->acqParams.symbolRate += hChn->peakScanIfStepSize;
         if (hChn->acqParams.symbolRate <= hChn->peakScanSymRateMax)
         {
            BAST_g3_P_SetBfos_isr(h);

            hChn->dft_funct_state = 0;
            goto next_state;
         }

         /* use_max_ratio */
         sym_rate_est = hChn->peakScanSymRateEst;
         peak_bin = hChn->peakScanMaxPeakBin;
         hb = hChn->peakScanMaxPeakHb;

         if (hChn->peakScanSymRateMax)
         {
            if (peak_bin > (hChn->peakScanDftSize >> 1))
            {
               BMTH_HILO_32TO64_Mul((uint32_t)(hChn->peakScanDftSize - peak_bin), hChn->sampleFreq, &P_hi, &P_lo);
               BMTH_HILO_64TO64_Div32(P_hi, P_lo, (uint32_t)hChn->peakScanDftSize * 8, &Q_hi, &Q_lo);
               Q_lo = (Q_lo + 1) >> 1;
               hChn->peakScanOutput = sym_rate_est - (Q_lo >> hb);
            }
            else  /* positive frequency */
            {
               BMTH_HILO_32TO64_Mul((uint32_t)peak_bin, hChn->sampleFreq, &P_hi, &P_lo);
               BMTH_HILO_64TO64_Div32(P_hi, P_lo, (uint32_t)hChn->peakScanDftSize * 8, &Q_hi, &Q_lo);
               Q_lo = (Q_lo + 1) >> 1;
               hChn->peakScanOutput = sym_rate_est + (Q_lo >> hb);
            }
         }
         else
         {
            hChn->peakScanOutput = peak_bin;
         }

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
 BAST_g3_P_DftPeakScanPsd2_isr()
******************************************************************************/
BERR_Code BAST_g3_P_DftPeakScanPsd2_isr(BAST_ChannelHandle h)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   uint32_t peak;

   hChn->dft_funct_state++;
   BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_DFT_PEAK_POW, &peak);
   hChn->dft_count1 += peak;
   if (hChn->dft_funct_state < 8)
      return BAST_g3_P_DftPeakScanPsd1_isr(h);

   BAST_g3_P_DisableChannelInterrupts_isr(h, false, false);
   hChn->peakScanStatus &= ~BAST_PEAK_SCAN_STATUS_ERROR;
   hChn->peakScanStatus |= BAST_PEAK_SCAN_STATUS_DONE;
   hChn->acqState = BAST_AcqState_eIdle;
   hChn->peakScanOutput = 0; /* sym rate estimate */
   hChn->peakScanMaxPower = hChn->dft_count1 / 8;
   BKNI_SetEvent(hChn->hPeakScanEvent);
   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_g3_P_DftPeakScanPsd1_isr()
******************************************************************************/
BERR_Code BAST_g3_P_DftPeakScanPsd1_isr(BAST_ChannelHandle h)
{
   uint32_t val;

   val = 0x00;
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_DFT_CTRL0, &val);
   BKNI_Delay(2);
   val = 0x257;
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_DFT_CTRL1, &val);
   val = 0;
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_DFT_RANGE_START, &val);
   val = 0x0F;
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_DFT_RANGE_END, &val);
   val = 0;
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_DFT_DDFS_FCW, &val);
   return BAST_g3_P_DftStartAndWaitForDone_isr(h, BAST_g3_P_DftPeakScanPsd2_isr);
}


/******************************************************************************
 BAST_g3_P_DftPeakScan_isr()
******************************************************************************/
BERR_Code BAST_g3_P_DftPeakScan_isr(BAST_ChannelHandle h)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   uint32_t val, ctrl1, ndiv, mdiv, nDecFilters;

   /* reset the DFT */
   val = 0x02;
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_DFT_CTRL0, &val);
   val = 0x00;
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_DFT_CTRL0, &val);

   /* configure the DFT */
   if ((hChn->peakScanSymRateMin == 0) || (hChn->peakScanSymRateMax == 0))
   {
      hChn->peakScanSymRateMax = hChn->peakScanSymRateMin = 0;
      if (hChn->miscCtl & BSAT_G3_CONFIG_MISC_CTL_PEAKSCAN_PSD)
      {
         /* divide Fs by 4 */
         BAST_g3_P_GetSampleFreqMN_isr(h, &ndiv, &mdiv);
         mdiv = mdiv << 2;
         BAST_g3_P_SetSampleFreq_isr(h, ndiv, mdiv);

         hChn->dft_funct_state = 0;
         hChn->dft_count1 = 0;
         return BAST_g3_P_DftPeakScanPsd1_isr(h);
      }
      else
      {
         val = (uint32_t)(hChn->dftRangeStart & 0x7FF);
         BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_DFT_RANGE_START, &val);
         val = (uint32_t)(hChn->dftRangeEnd & 0x7FF);
         BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_DFT_RANGE_END, &val);

         /* set to 6 HB */
         BAST_g3_P_ReadModifyWriteRegister_isrsafe(h, BCHP_SDS_FE_FILTCTL, 0xFFFFFF00, 0x60);

         ctrl1 = 0x013D;
      }

   }
   else
   {
      /* symbol rate scan */
      hChn->acqParams.symbolRate = hChn->peakScanSymRateMin * 2; /* double Fb when calculating filtctl */
      if (hChn->acqParams.symbolRate > 40000000)
         hChn->acqParams.symbolRate = 40000000;
      BAST_g3_P_SetDecimationFilters_isr(h);
      hChn->acqParams.symbolRate = hChn->peakScanSymRateMin;
      BAST_g3_P_SetBfos_isr(h);

#ifndef BAST_HAS_WFE
      if ((nDecFilters = BAST_g3_P_GetNumDecimatingFilters_isr(h)) >= 3)
      {
         /* dft h/w problem with 3 or more decimation filters enabled */
         BAST_g3_P_GetSampleFreqMN_isr(h, &ndiv, &mdiv);
         mdiv = mdiv << ((nDecFilters >= 4) ? 2 : 1);
         BAST_g3_P_SetSampleFreq_isr(h, ndiv, mdiv);

         BAST_g3_P_OrRegister_isrsafe(h, BCHP_SDS_FE_FILTCTL, 0x1C); /* set to 2 decimation filters */
         nDecFilters = BAST_g3_P_GetNumDecimatingFilters_isr(h);
#if BAST_DEBUG_PEAK_SCAN
         BKNI_Printf("Change Fs to %u, nDecFilters=%d\n", hChn->sampleFreq, nDecFilters);
#endif
      }
#endif

      val = 0;
      BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_DFT_RANGE_START, &val);
      val = 0x0F;
      BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_DFT_RANGE_END, &val);

      ctrl1 = 0x3F5;
   }
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_DFT_CTRL1, &ctrl1);

   hChn->acqParams.symbolRate = hChn->peakScanSymRateMin;
   hChn->peakScanDftSize = 256 << (ctrl1 & 0x03);
   hChn->peakScanIfStepSize = (uint32_t)(hChn->sampleFreq >> (9+BAST_g3_P_GetNumDecimatingFilters_isr(h))); /* Fs/(512*2^hb) */
   hChn->peakScanMaxPower = 0;
   hChn->peakScanMaxPeakBin = 0;
   hChn->dft_count1 = 0;
   hChn->count1 = 0;
   for (val = 0; val < 32; val++)
      hChn->dftBinPower[val] = 0;
   hChn->peakScanMaxPeakHb = 0;
   hChn->peakScanMaxRatioPower = 0;
   hChn->peakScanMaxRatio = 0;
   hChn->peakScanMaxRatioBin = 0;
   hChn->peakScanMaxRatioHb = 0;
   hChn->peakScanMaxRatioSymRate = hChn->peakScanSymRateMin;
   hChn->peakScanSymRateEst = hChn->peakScanSymRateMin;
   hChn->dft_funct_state = 0;
   return BAST_g3_P_DftPeakScanStateMachine_isr(h);
}


/******************************************************************************
 BAST_g3_P_DftDumpBins2_isr()
******************************************************************************/
BERR_Code BAST_g3_P_DftDumpBins2_isr(BAST_ChannelHandle h)
{
   BAST_g3_P_Handle *hDev = h->pDevice->pImpl;
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   uint32_t val;
   int i;

   next_state:
   if ((hChn->dft_funct_state) == 0)
   {
      hChn->dft_funct_state = 1;
      BAST_g3_P_DftSetDdfsFcw_isr(h);
      return BAST_g3_P_DftStartAndWaitForDone_isr(h, BAST_g3_P_DftDumpBins2_isr);
   }
   else
   {
      val = 0;
      BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_DFT_MEM_RADDR, &val);
      for (i = 0; i < 16; i++)
      {
         BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_DFT_MEM_RDATA, &val);
         hChn->dftBinPower[i] = val;
      }

      if (hChn->dft_count1 == 0)
         BKNI_Printf("%u %u ", hChn->actualTunerFreq, hChn->acqParams.symbolRate);
      for (i = 0; i < 16; i++)
         BKNI_Printf("%u ", hChn->dftBinPower[i]);

      hChn->dft_count1++;
      if (hChn->dft_count1 < hDev->dftMinN)
      {
         hChn->dft_funct_state = 0;
         goto next_state;
      }

      BKNI_Printf("\n");

      hChn->acqParams.symbolRate += hChn->peakScanIfStepSize;
      if (hChn->acqParams.symbolRate <= hChn->peakScanSymRateMax)
      {
         BAST_g3_P_SetBfos_isr(h);
         hChn->dft_funct_state = 0;
         hChn->dft_count1 = 0;
         goto next_state;
      }

      BAST_g3_P_DisableChannelInterrupts_isr(h, false, false);
      hChn->peakScanStatus &= ~BAST_PEAK_SCAN_STATUS_ERROR;
      hChn->peakScanStatus |= BAST_PEAK_SCAN_STATUS_DONE;
      hChn->acqState = BAST_AcqState_eIdle;
      BKNI_SetEvent(hChn->hPeakScanEvent);
   }
   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_g3_P_DftDumpBins_isr()
******************************************************************************/
BERR_Code BAST_g3_P_DftDumpBins_isr(BAST_ChannelHandle h)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   uint32_t val, ctrl1, ndiv, mdiv, nDecFilters;

   /* reset the DFT */
   val = 0x02;
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_DFT_CTRL0, &val);
   val = 0x00;
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_DFT_CTRL0, &val);

   /* symbol rate scan */
   /* divide Fs by 2 */
   BAST_g3_P_GetSampleFreqMN_isr(h, &ndiv, &mdiv);
   mdiv = mdiv << 1;
   BAST_g3_P_SetSampleFreq_isr(h, ndiv, mdiv);

   hChn->acqParams.symbolRate = hChn->peakScanSymRateMin;
   BAST_g3_P_SetDecimationFilters_isr(h);
   if ((nDecFilters = BAST_g3_P_GetNumDecimatingFilters_isr(h)) >= 3)
   {
      BAST_g3_P_OrRegister_isrsafe(h, BCHP_SDS_FE_FILTCTL, 0x1C); /* set to 2 decimation filters */
   }
   BAST_g3_P_SetBfos_isr(h);

   val = 0;
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_DFT_RANGE_START, &val);
   val = 0x0F;
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_DFT_RANGE_END, &val);

   ctrl1 = 0x3F5;
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_DFT_CTRL1, &ctrl1);

   hChn->peakScanDftSize = 256 << (ctrl1 & 0x03);
   hChn->peakScanIfStepSize = hChn->sampleFreq / 2048;
   hChn->dft_funct_state = 0;
   hChn->dft_count1 = 0;
   for (val = 0; val < 32; val++)
      hChn->dftBinPower[val] = 0;
   return BAST_g3_P_DftDumpBins2_isr(h);
}
