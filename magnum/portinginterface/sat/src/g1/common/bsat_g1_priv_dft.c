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


BDBG_MODULE(bsat_g1_priv_dft);

#define BSAT_DEBUG_DFT 0
#define BSAT_DEBUG_PEAK_SCAN 0

/* local functions */
BERR_Code BSAT_g1_P_DftSearchCarrierStateMachine_isr(BSAT_ChannelHandle h);
BERR_Code BSAT_g1_P_DftOqpskSearchCarrierStateMachine_isr(BSAT_ChannelHandle h);
BERR_Code BSAT_g1_P_DftSetDdfsFcw_isr(BSAT_ChannelHandle h);
BERR_Code BSAT_g1_P_DftStartAndWaitForDone_isr(BSAT_ChannelHandle h, BSAT_g1_FUNCT funct);
BERR_Code BSAT_g1_P_DftSymbolRateScanStateMachine_isr(BSAT_ChannelHandle h);
BERR_Code BSAT_g1_P_DftPsdScan1_isr(BSAT_ChannelHandle h);
BERR_Code BSAT_g1_P_DftPsdScan2_isr(BSAT_ChannelHandle h);
BERR_Code BSAT_g1_P_DftToneDetect1_isr(BSAT_ChannelHandle h);


/******************************************************************************
 BSAT_g1_P_DftEnableInterrupt_isr()
******************************************************************************/
BERR_Code BSAT_g1_P_DftEnableInterrupt_isr(BSAT_ChannelHandle h)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   BINT_ClearCallback_isr(hChn->hDftDoneCb);
   return BINT_EnableCallback_isr(hChn->hDftDoneCb);

}


/******************************************************************************
 BSAT_g1_P_DftDisableInterrupt_isr()
******************************************************************************/
BERR_Code BSAT_g1_P_DftDisableInterrupt_isr(BSAT_ChannelHandle h)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   BINT_DisableCallback_isr(hChn->hDftDoneCb);
   return BINT_ClearCallback_isr(hChn->hDftDoneCb);
}


/******************************************************************************
 BSAT_g1_P_DftSetState_isr()
******************************************************************************/
void BSAT_g1_P_DftSetState_isr(BSAT_ChannelHandle h, uint8_t state)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   hChn->configParam[BSAT_g1_CONFIG_DFT_STATUS] &= ~BSAT_g1_CONFIG_DFT_STATUS_STATE;
   hChn->configParam[BSAT_g1_CONFIG_DFT_STATUS] |= state;
}


/******************************************************************************
 BSAT_g1_P_DftSearchCarrier_isr()
******************************************************************************/
BERR_Code BSAT_g1_P_DftSearchCarrier_isr(BSAT_ChannelHandle h, BSAT_g1_FUNCT funct)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;

   hChn->nextFunct = funct;

   hChn->configParam[BSAT_g1_CONFIG_DFT_FREQ_ESTIMATE] = 0;
   hChn->configParam[BSAT_g1_CONFIG_DFT_STATUS] = BSAT_g1_CONFIG_DFT_STATUS_START;

   if (hChn->miscSettings.bBypassDft)
      BSAT_g1_P_DftSetState_isr(h, 7);

   if ((hChn->acqSettings.options & BSAT_ACQ_OQPSK) && (BSAT_MODE_IS_DCII(hChn->acqSettings.mode)))
      return BSAT_g1_P_DftOqpskSearchCarrierStateMachine_isr(h);
   else
      return BSAT_g1_P_DftSearchCarrierStateMachine_isr(h);
}


/******************************************************************************
 BSAT_g1_P_DftSearchCarrierStateMachine_isr()
******************************************************************************/
BERR_Code BSAT_g1_P_DftSearchCarrierStateMachine_isr(BSAT_ChannelHandle h)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   BSAT_g1_P_Handle *hDev = (BSAT_g1_P_Handle*)h->pDevice->pImpl;
   BERR_Code retCode = BERR_SUCCESS;
   uint32_t total_pow, peak_pow, val, hb;
#if BSAT_DEBUG_DFT
   uint32_t start, stop, peak_bin, chan_lf_int;
#endif
   bool bDone = false;
   uint8_t state;

   while (!bDone)
   {
      state = (uint8_t)(hChn->configParam[BSAT_g1_CONFIG_DFT_STATUS] & BSAT_g1_CONFIG_DFT_STATUS_STATE);
      switch (state)
      {
         case 0xFF:
            bDone = true;
            break;

         case 0:
            /* coarse freq step is 10% of Fb */
            hChn->configParam[BSAT_g1_CONFIG_DFT_STATUS] |= BSAT_g1_CONFIG_DFT_STATUS_COARSE;
            hChn->tunerIfStepSize = hChn->acqSettings.symbolRate / 10;
            hChn->tunerIfStep = (int32_t)(-(hChn->searchRange) - hChn->tunerIfStepSize);
            hChn->tunerIfStepMax = (int32_t)(hChn->searchRange);
            hChn->maxCount2 = 0; /* maxCount2 = dft_current_max_pow */

            if (hChn->tunerIfStepSize > hChn->searchRange)
               BSAT_g1_P_DftSetState_isr(h, 6); /* do fine search */
            else
            {
               BSAT_g1_P_DftSetState_isr(h, 1);
#if BSAT_DEBUG_DFT
               BDBG_ERR(("\nDFT(coarse): StepSize=%d", hChn->tunerIfStepSize));
#endif
            }
            break;

         case 1:
            if (hChn->tunerIfStep <= hChn->tunerIfStepMax)
            {
               hChn->tunerIfStep += (int32_t)hChn->tunerIfStepSize;
               BSAT_g1_P_DftSetState_isr(h, 2);
               return BSAT_g1_P_TunerQuickTune_isr(h, BSAT_g1_P_DftSearchCarrierStateMachine_isr);
            }
            else
               BSAT_g1_P_DftSetState_isr(h, 4);
            break;

         case 2:
            BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_DFT_RANGE_START, 0x7F8);
            BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_DFT_RANGE_END, 0x007);
            BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_DFT_CTRL0, 2);
            BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_DFT_CTRL0, 0);
            BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_DFT_CTRL1, 0x2F7);

            BSAT_CHK_RETCODE(BSAT_g1_P_DftSetDdfsFcw_isr(h));
            BSAT_g1_P_DftSetState_isr(h, 3);
            return BSAT_g1_P_DftStartAndWaitForDone_isr(h, BSAT_g1_P_DftSearchCarrierStateMachine_isr);

         case 3:
            peak_pow = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_SDS_DFT_PEAK_POW);
            total_pow = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_SDS_DFT_TOTAL_POW);
#if BSAT_DEBUG_DFT
            peak_bin = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_SDS_DFT_PEAK_BIN);
            start = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_SDS_DFT_RANGE_START);
            stop = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_SDS_DFT_RANGE_END);
            chan_lf_int = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_STB_CHAN_CHx_AGC_LF_INT);
#endif

            /* software workaround for bug in total_pow */
            if (hDev->sdsRevId <= 0x71)
            {
               hb = BSAT_g1_P_GetNumDecimatingFilters_isr(h);
               val = 0;
               if (hb == 3)
                  val = 1;
               else if (hb == 4)
                  val = 2;
               else if (hb == 5)
                  val = 3;
               if (val)
                  total_pow = total_pow >> val;
            }

#if BSAT_DEBUG_DFT
            BDBG_ERR(("%d Hz: [%x,%x] %08X %08X %08X %08X %08X", hChn->tunerIfStep, start, stop, peak_pow, total_pow, peak_bin, chan_lf_int));
#endif

            if (peak_pow > (total_pow>>2))
            {
               val = peak_pow - ((total_pow - peak_pow) >> 5);
               if (val & 0x80000000) /* check for overflow */
                  val = 0;
            }
            else
               val = 0;
#if BSAT_DEBUG_DFT
            BDBG_MSG(("%d Hz: max_peak=0x%08X, sum_total=0x%08X, pow=%u", hChn->tunerIfStep, hChn->maxCount1, hChn->count2, val));
#endif
            if (val > hChn->maxCount2)
            {
               hChn->maxCount2 = val;
               hChn->configParam[BSAT_g1_CONFIG_DFT_FREQ_ESTIMATE] = hChn->tunerIfStep;
            }
            BSAT_g1_P_DftSetState_isr(h, 1);
            break;

         case 4:
            if (hChn->configParam[BSAT_g1_CONFIG_DFT_STATUS] & BSAT_g1_CONFIG_DFT_STATUS_FINE)
               BSAT_g1_P_DftSetState_isr(h, 5);
            else
            {
               BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_BL_BRI, 0); /* reset baud loop integrator */

               hChn->configParam[BSAT_g1_CONFIG_DFT_STATUS] |= BSAT_g1_CONFIG_DFT_STATUS_FINE;
               hChn->tunerIfStep = (int32_t)(hChn->configParam[BSAT_g1_CONFIG_DFT_FREQ_ESTIMATE]) - (int32_t)hChn->tunerIfStepSize;
               if (hChn->tunerIfStep < (int32_t)-(hChn->searchRange))
                  hChn->tunerIfStep = -(hChn->searchRange);
               hChn->tunerIfStepMax = (int32_t)(hChn->configParam[BSAT_g1_CONFIG_DFT_FREQ_ESTIMATE]) + (int32_t)hChn->tunerIfStepSize;
               if (hChn->tunerIfStepMax > (int32_t)hChn->searchRange)
                  hChn->tunerIfStepMax = (int32_t)(hChn->searchRange);
               hChn->tunerIfStepSize = hChn->acqSettings.symbolRate / 20;
               if (hChn->tunerIfStepSize > 1000000)
                  hChn->tunerIfStepSize = 1000000; /* fine step should be no more than 1MHz */
               hChn->tunerIfStep -= hChn->tunerIfStepSize;

               hChn->maxCount2 = 0;
               BSAT_g1_P_DftSetState_isr(h, 1);
#if BSAT_DEBUG_DFT
               BDBG_ERR(("DFT(fine): coarse_est=%d, StepSize=%d", hChn->configParam[BSAT_g1_CONFIG_DFT_FREQ_ESTIMATE], hChn->tunerIfStepSize));
#endif
            }
            break;

         case 5:
            hChn->configParam[BSAT_g1_CONFIG_DFT_STATUS] |= BSAT_g1_CONFIG_DFT_STATUS_DONE;
            BSAT_CHK_RETCODE(BSAT_g1_P_LogTraceBuffer_isr(h, BSAT_TraceEvent_eFreqEstDone));
            BDBG_MSG(("DFT freq estimate (chan %d) = %d", h->channel, hChn->configParam[BSAT_g1_CONFIG_DFT_FREQ_ESTIMATE]));
            hChn->tunerIfStep = 0;
            if (hChn->miscSettings.bBypassDft)
            {
               BDBG_WRN(("DFT: did not transfer freq offset to the LO"));
            }
            else if (hChn->miscSettings.bPreserveCommandedTunerFreq)
            {
               BDBG_MSG(("DFT: transferring freq offset to FLIF"));
               BSAT_CHK_RETCODE(BSAT_g1_P_SetFlifOffset_isr(h, (int32_t)(hChn->configParam[BSAT_g1_CONFIG_DFT_FREQ_ESTIMATE])));
            }
            else
               hChn->tunerIfStep = (int32_t)(hChn->configParam[BSAT_g1_CONFIG_DFT_FREQ_ESTIMATE]);

            BSAT_g1_P_DftSetState_isr(h, 6);
            return BSAT_g1_P_TunerQuickTune_isr(h, BSAT_g1_P_DftSearchCarrierStateMachine_isr);

         case 6:
            BSAT_g1_P_DftSetState_isr(h, 7);
            /* tuner settling delay */
            return BSAT_g1_P_EnableTimer_isr(h, BSAT_TimerSelect_eBaudUsec, 1000 /*20000*/, BSAT_g1_P_DftSearchCarrierStateMachine_isr);

         case 7:
            BSAT_CHK_RETCODE(BSAT_g1_P_LogTraceBuffer_isr(h, BSAT_TraceEvent_eRetuneDone));

#ifdef BSAT_HAS_WFE
            BSAT_g1_P_SetNotch_isr(h);
#endif

            if (hChn->miscSettings.bPreserveCommandedTunerFreq == false)
               BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_CL_FLIF, 0);
            BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_CL_FLI, 0);
            BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_CL_PLI, 0);
            BSAT_g1_P_DftSetState_isr(h, 0xFF);
            break;

         default:
            /* should never get here */
            BDBG_ERR(("BSAT_g1_P_DftSearchCarrierStateMachine(): invalid funct_state (%d)", state));
            BDBG_ASSERT(0);
      }
   }

   BSAT_CHK_RETCODE(hChn->nextFunct(h));

   done:
   return retCode;
}


/******************************************************************************
 BSAT_g1_P_DftOqpskSearchCarrierStateMachine_isr()
******************************************************************************/
BERR_Code BSAT_g1_P_DftOqpskSearchCarrierStateMachine_isr(BSAT_ChannelHandle h)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode;
   uint32_t peak_pow, total_pow, peak_bin, P_hi, P_lo, Q_hi, Q_lo;
   uint8_t state;

   while (1)
   {
      state = (uint8_t)(hChn->configParam[BSAT_g1_CONFIG_DFT_STATUS] & BSAT_g1_CONFIG_DFT_STATUS_STATE);
      if (state > 3)
         break;

      switch (state)
      {
         case 0:
            hChn->configParam[BSAT_g1_CONFIG_DFT_STATUS] |= BSAT_g1_CONFIG_DFT_STATUS_OQPSK;

            if (hChn->acqSettings.symbolRate > 11000000)
               hChn->tunerIfStepSize = 3000000; /* 1.5MHz filter * 2 */
            else if (hChn->acqSettings.symbolRate > 4800000)
               hChn->tunerIfStepSize = 2600000; /* 1.3MHz filter * 2 */
            else
               hChn->tunerIfStepSize = 2000000; /* 1.0MHz filter * 2 */

            hChn->count3 = 0; /* count3 = if step num */
            hChn->count2 = 0; /* count2 = max snr */
            BSAT_g1_P_DftSetState_isr(h, 1);
            break;

         case 1:
            if (hChn->count3)
               hChn->tunerIfStep = (hChn->tunerIfStepSize >> 1) + (((hChn->count3 - 1) >> 1) * hChn->tunerIfStepSize);
            else
               hChn->tunerIfStep = 0;

            /* check for done */
            if (hChn->tunerIfStep > (int32_t)(hChn->searchRange))
               BSAT_g1_P_DftSetState_isr(h, 7);
            else
            {
               if ((hChn->count3 & 1) == 0)
                  hChn->tunerIfStep = -(hChn->tunerIfStep);

               BSAT_CHK_RETCODE(BSAT_g1_P_SetFlifOffset_isr(h, hChn->tunerIfStep));
               BSAT_g1_P_DftSetState_isr(h, 2);
            }
            break;

         case 2:
            BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_DFT_RANGE_START, 0);
            BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_DFT_RANGE_END, 0x7FF);
            BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_DFT_CTRL0, 0x02);
            BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_DFT_CTRL0, 0);
            BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_DFT_CTRL1, 0xD37); /* input to DFT is ADC, offset QPSK */
            BSAT_CHK_RETCODE(BSAT_g1_P_DftSetDdfsFcw_isr(h));
            BSAT_g1_P_DftSetState_isr(h, 3);
            return BSAT_g1_P_DftStartAndWaitForDone_isr(h, BSAT_g1_P_DftOqpskSearchCarrierStateMachine_isr);

         case 3:
            peak_pow = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_SDS_DFT_PEAK_POW);
            total_pow = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_SDS_DFT_TOTAL_POW);
#if BSAT_DEBUG_DFT
            BDBG_MSG(("DFT OQPSK: %d Hz: peak_pow=%u, total_pow=%u", hChn->tunerIfStep, peak_pow, total_pow));
#endif
            BMTH_HILO_32TO64_Mul(peak_pow, 2048, &P_hi, &P_lo);
            BMTH_HILO_64TO64_Div32(P_hi, P_lo, total_pow, &Q_hi, &Q_lo);

            if (Q_lo > hChn->count2)
            {
               hChn->count2 = Q_lo;
               peak_bin = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_SDS_DFT_PEAK_BIN);
               peak_bin &= 0x00000FFF;
               if (peak_bin < 0x400)
               {
                  BMTH_HILO_32TO64_Mul(hChn->sampleFreq, peak_bin, &P_hi, &P_lo);
                  BMTH_HILO_64TO64_Div32(P_hi, P_lo, 65536, &Q_hi, &Q_lo);
                  hChn->configParam[BSAT_g1_CONFIG_DFT_FREQ_ESTIMATE] = Q_lo;
               }
               else
               {
                  BMTH_HILO_32TO64_Mul(hChn->sampleFreq, 2048 - peak_bin, &P_hi, &P_lo);
                  BMTH_HILO_64TO64_Div32(P_hi, P_lo, 65536, &Q_hi, &Q_lo);
                  hChn->configParam[BSAT_g1_CONFIG_DFT_FREQ_ESTIMATE] = (int32_t)-Q_lo;
               }
#if BSAT_DEBUG_DFT
               BDBG_MSG(("   -> count2=%u, bin=%u, Q_lo=%u, delta=%d", hChn->count2, peak_bin, Q_lo, hChn->configParam[BSAT_g1_CONFIG_DFT_FREQ_ESTIMATE]));
#endif
               hChn->configParam[BSAT_g1_CONFIG_DFT_FREQ_ESTIMATE] += hChn->tunerIfStep;
            }
            hChn->count3++;
            BSAT_g1_P_DftSetState_isr(h, 1);
            break;

         default:
            /* should never get here */
            BDBG_ERR(("BSAT_g1_P_DftOqpskSearchCarrierStateMachine(): invalid funct_state (%d)", state));
            BDBG_ASSERT(0);
      }
   }

   /* state should be 7 at this point */
   BDBG_ASSERT(state == 7);
   retCode = BSAT_g1_P_DftSearchCarrierStateMachine_isr(h);

   done:
   return retCode;
}


/******************************************************************************
 BSAT_g1_P_SetDftDdfsFcw_isr()
******************************************************************************/
BERR_Code BSAT_g1_P_DftSetDdfsFcw_isr(BSAT_ChannelHandle h)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   uint32_t P_hi, P_lo, Q_hi, Q_lo;

   BMTH_HILO_32TO64_Mul(hChn->acqSettings.symbolRate, 32768 << 1, &P_hi, &P_lo);
   BMTH_HILO_64TO64_Div32(P_hi, P_lo, hChn->sampleFreq, &Q_hi, &Q_lo);
   Q_lo = (Q_lo + 1) >> 1;
   /* BDBG_MSG(("DFT_DDFS_FCW=0x%X", Q_lo)); */
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_DFT_DDFS_FCW, Q_lo);
   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_DftDone_isr() - callback routine for DFT done interrupt
******************************************************************************/
void BSAT_g1_P_DftDone_isr(void *p, int int_id)
{
   BSAT_ChannelHandle h = (BSAT_ChannelHandle)p;
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   uint32_t val;

   BSAT_g1_P_IncrementInterruptCounter_isr(h, int_id);

   val = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_SDS_DFT_STATUS);
   if ((val & 0x06) == 0x06)
   {
      BSAT_g1_P_DftDisableInterrupt_isr(h);
      BSAT_g1_P_DisableTimer_isr(h, BSAT_TimerSelect_eBaudUsec);
      hChn->passFunct(h);
   }
   else
   {
      BSAT_g1_P_DftEnableInterrupt_isr(h); /* false trigger, keep this interrupt on */
      BDBG_MSG(("BSAT_g1_P_DftDone_isr: invalid DFT_STATUS=0x%X", val));
   }
}


/******************************************************************************
 BAST_g1_P_DftDoneTimeout_isr()
******************************************************************************/
BERR_Code BAST_g1_P_DftDoneTimeout_isr(BSAT_ChannelHandle h)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   uint32_t val;

   BSAT_g1_P_DftDisableInterrupt_isr(h);

   val = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_SDS_DFT_STATUS);
   if ((val & 0x06) == 0x06)
   {
      BDBG_ERR(("BAST_g1_P_DftDoneTimeout(): missed irq, DFT_STATUS=0x%X", val));
      return hChn->passFunct(h);
   }

   BDBG_ERR(("DFT timeout! (dft_status=0x%X)", val));
   hChn->reacqCause = BSAT_ReacqCause_eDftTimeout;
   if (hChn->operation == BSAT_Operation_eAcq)
      return BSAT_g1_P_GetModeFunct_isrsafe(hChn->acqType)->Reacquire(h);

   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_DftStartAndWaitForDone_isr()
******************************************************************************/
BERR_Code BSAT_g1_P_DftStartAndWaitForDone_isr(BSAT_ChannelHandle h, BSAT_g1_FUNCT funct)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   uint32_t i, val, retry_count = 0, timeout = 5000;

   hChn->passFunct = funct;

#ifdef BCHP_SDS_DFT_0_CTRL1_dft_avg_mode_MASK
   /* adjust the timeout based on the average number */
   val = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_SDS_DFT_CTRL1);
   i = (val & BCHP_SDS_DFT_0_CTRL1_dft_avg_num_MASK) >> BCHP_SDS_DFT_0_CTRL1_dft_avg_num_SHIFT;
   if (i)
      timeout = 5000 << i;
#endif

   /* start the DFT engine */
   retry:
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_DFT_CTRL0, 0);
   BSAT_g1_P_DftEnableInterrupt_isr(h);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_DFT_CTRL0, 1);

   for (i = 0; i < 50; i++)
   {
      val = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_SDS_DFT_STATUS);
      if (val & 0x07)
         break;
   }
   if ((val & 0x07) == 0)
   {
      BDBG_WRN(("dft_status should not be 0, trying again..."));
      if (retry_count++ < 10)
         goto retry;

      BDBG_ERR(("ERROR: dft_status=0!"));
      hChn->reacqCause = BSAT_ReacqCause_eDftTimeout;
      return BSAT_g1_P_GetModeFunct_isrsafe(hChn->acqType)->Reacquire(h);
   }

   return BSAT_g1_P_EnableTimer_isr(h, BSAT_TimerSelect_eBaudUsec, timeout, BAST_g1_P_DftDoneTimeout_isr);
}


/******************************************************************************
 BSAT_g1_P_DftSymbolRateScan_isr()
******************************************************************************/
#define BSAT_PEAK_SCAN_DFT_SIZE 2048
BERR_Code BSAT_g1_P_DftSymbolRateScan_isr(BSAT_ChannelHandle h)
{
   extern BERR_Code BSAT_g1_P_SetBfos_isr(BSAT_ChannelHandle h);

   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   uint32_t hb, i, val;

   /* reset the DFT */
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_DFT_CTRL0, 2);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_DFT_CTRL0, 0);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_DFT_RANGE_START, 0);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_DFT_RANGE_END, 0x0F);
   val = 0x3F5;
#ifdef BCHP_SDS_DFT_0_CTRL1_dft_avg_mode_MASK
   val |= (1 << BCHP_SDS_DFT_0_CTRL1_dft_avg_mode_SHIFT); /* minimum power mode */
   val |= (2 << BCHP_SDS_DFT_0_CTRL1_dft_avg_num_SHIFT);  /* average number = 4 */
#endif
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_DFT_CTRL1, val);

   hChn->acqSettings.symbolRate = hChn->symbolRateScanStatus.minSymbolRate;
   BSAT_g1_P_SetBfos_isr(h);

   hb = BSAT_g1_P_GetNumDecimatingFilters_isr(h);
   hChn->tunerIfStepSize = (uint32_t)(hChn->sampleFreq >> (9+hb)); /* Fs/(512*2^hb) */
   hChn->count1 = 0; /* writer index */
   hChn->count2 = 0; /* reader index */
   hChn->maxCount2 = 0; /* maxCount2 = peak scan max power */
   hChn->maxCount1 = 0; /* maxCount1 = bin at which max peak power occurs */
   hChn->count3 = 0; /* count3 = number of decimation filters at which max peak power occurs */
   for (i = 0; i < 32; i++)
      hChn->dftBinPower[i] = 0;
   hChn->configParam[BSAT_g1_CONFIG_DFT_STATUS] = BSAT_g1_CONFIG_DFT_STATUS_PEAK_SCAN;
   return BSAT_g1_P_DftSymbolRateScanStateMachine_isr(h);
}


/******************************************************************************
 BSAT_g1_P_DftSymbolRateScanStateMachine_isr()
******************************************************************************/
BERR_Code BSAT_g1_P_DftSymbolRateScanStateMachine_isr(BSAT_ChannelHandle h)
{
#define NEW_PEAK_POWER_RATIO 6
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   uint32_t val, P_hi, P_lo, Q_hi, Q_lo, i, j, bin_pow, sym_rate_est, sum, peak_pow, dftSize, hb;
   uint8_t state;

   next_state:
   state = (uint8_t)(hChn->configParam[BSAT_g1_CONFIG_DFT_STATUS] & BSAT_g1_CONFIG_DFT_STATUS_STATE);
#if 0 /* BSAT_DEBUG_PEAK_SCAN */
   BKNI_Printf("BSAT_g1_P_DftSymbolRateScanStateMachine_isr(): state=%d", state);
#endif
   switch (state)
   {
      case 0:
         BSAT_g1_P_DftSetState_isr(h, 1);
         BSAT_g1_P_DftSetDdfsFcw_isr(h);
         return BSAT_g1_P_DftStartAndWaitForDone_isr(h, BSAT_g1_P_DftSymbolRateScanStateMachine_isr);

      case 1:
      case 2:
      case 3:
      case 4:
#if 0 /* BSAT_DEBUG_PEAK_SCAN */
         BKNI_Printf("Fb=%u: ", hChn->acqSettings.symbolRate);
#endif
         BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_DFT_MEM_RADDR, 0);
         for (i = 0; i < 16; i++)
         {
            val = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_SDS_DFT_MEM_RDATA);
#if 0 /* BSAT_DEBUG_PEAK_SCAN */
         BKNI_Printf("%u ", val);
#endif
            if ((state == 1) || (val < hChn->dftBinPower[(hChn->count1+i)%32]))
               hChn->dftBinPower[(hChn->count1+i)%32] = val;
         }
#if 0 /* BSAT_DEBUG_PEAK_SCAN */
         BKNI_Printf("\n");
#endif
#ifndef BCHP_SDS_DFT_0_CTRL1_dft_avg_mode_MASK
         if (state != 4)
         {
            BSAT_g1_P_DftSetState_isr(h, state+1);
            return BSAT_g1_P_DftStartAndWaitForDone_isr(h, BSAT_g1_P_DftSymbolRateScanStateMachine_isr);
         }
#endif

         hChn->count1 += 16; /* count1 is always a multiple of 16 */
         while ((hChn->count1 - hChn->count2) > 9)
         {
            i = hChn->count2;
            bin_pow = hChn->dftBinPower[i%32];

            /* determine symbol rate associated with count2 */
            if ((i % 16) <= 6)
               sym_rate_est = hChn->acqSettings.symbolRate; /* current symbol rate */
            else
               sym_rate_est = hChn->acqSettings.symbolRate - hChn->tunerIfStepSize; /* previous symbol rate */

            if (bin_pow > (NEW_PEAK_POWER_RATIO*hChn->dftBinPower[(i+2)%32]))
            {
               for (j = sum = 0; j <= 9; j++)
               {
                  sum += hChn->dftBinPower[(i+2+j)%32];
               }
               if ((bin_pow > sum) && (sum > 0))
               {
                  BMTH_HILO_32TO64_Mul(bin_pow, bin_pow, &P_hi, &P_lo);
                  BMTH_HILO_64TO64_Div32(P_hi, P_lo, sum, &Q_hi, &peak_pow);
                  if (peak_pow > hChn->maxCount2)
                  {
                     hChn->maxCount2 = peak_pow; /* save peak power */
                     hChn->maxCount1 = i % 16; /* save peak bin */
                     hChn->symbolRateScanStatus.symbolRate = sym_rate_est;
                     hChn->count3 = BSAT_g1_P_GetNumDecimatingFilters_isr(h);
#if BSAT_DEBUG_PEAK_SCAN
                     BKNI_Printf("new peak: freq=%u, Fb=%u, bin=%d, pow=%u, ratio=", hChn->acqSettings.freq, sym_rate_est, hChn->maxCount1, hChn->maxCount2);
                     if (hChn->dftBinPower[(i+2)%32] > 0)
                        BKNI_Printf("%u\n", bin_pow / hChn->dftBinPower[(i+2)%32]);
                     else
                        BKNI_Printf("-1\n");
#endif
                  }
               }
            }

            hChn->count2++;
         }

         hChn->acqSettings.symbolRate += hChn->tunerIfStepSize;
         if (hChn->acqSettings.symbolRate <= hChn->symbolRateScanStatus.maxSymbolRate)
         {
            BSAT_g1_P_SetBfos_isr(h);
            BSAT_g1_P_DftSetState_isr(h, 0);
            goto next_state;
         }

         dftSize = 256 << (BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_SDS_DFT_CTRL1) & 0x03);
         hb = hChn->count3;

         if (hChn->maxCount1 > (dftSize >> 1))
         {
            BMTH_HILO_32TO64_Mul((uint32_t)(dftSize - hChn->maxCount1), hChn->sampleFreq, &P_hi, &P_lo);
            BMTH_HILO_64TO64_Div32(P_hi, P_lo, dftSize * 8, &Q_hi, &Q_lo);
            Q_lo = (Q_lo + 1) >> 1;
            val = hChn->symbolRateScanStatus.symbolRate - (Q_lo >> hb);
         }
         else  /* positive frequency */
         {
            BMTH_HILO_32TO64_Mul(hChn->maxCount1, hChn->sampleFreq, &P_hi, &P_lo);
            BMTH_HILO_64TO64_Div32(P_hi, P_lo, dftSize * 8, &Q_hi, &Q_lo);
            Q_lo = (Q_lo + 1) >> 1;
            val = hChn->symbolRateScanStatus.symbolRate + (Q_lo >> hb);
         }
         hChn->symbolRateScanStatus.symbolRate = val;
         hChn->symbolRateScanStatus.peakPower = hChn->maxCount2;
         hChn->symbolRateScanStatus.bScanDone = true;
         hChn->symbolRateScanStatus.status = BERR_SUCCESS;
         BSAT_g1_P_DisableChannelInterrupts_isr(h);
         hChn->configParam[BSAT_g1_CONFIG_DFT_STATUS] |=  BSAT_g1_CONFIG_DFT_STATUS_DONE;
         hChn->acqState = BSAT_AcqState_eIdle;
         BSAT_g1_P_IndicateAcqDone_isr(h);
#if BSAT_DEBUG_PEAK_SCAN
         BKNI_Printf("SR Scan Results: Fb=%u, pow=%d\n", hChn->symbolRateScanStatus.symbolRate, hChn->symbolRateScanStatus.peakPower);
#endif
         break;

      default:
         break;
   }

   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_DftPsdScan_isr()
******************************************************************************/
BERR_Code BSAT_g1_P_DftPsdScan_isr(BSAT_ChannelHandle h)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;

   /* reset the DFT */
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_DFT_CTRL0, 0x02);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_DFT_CTRL0, 0);

   hChn->count1 = 0;
   hChn->count2 = 0;
   return BSAT_g1_P_DftPsdScan1_isr(h);
}


/******************************************************************************
 BSAT_g1_P_DftPsdScanPsd1_isr()
******************************************************************************/
BERR_Code BSAT_g1_P_DftPsdScan1_isr(BSAT_ChannelHandle h)
{
   uint32_t val;

   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_DFT_CTRL0, 0);
   BKNI_Delay(2);
   val = 0x257;
#ifdef BCHP_SDS_DFT_0_CTRL1_dft_avg_mode_MASK
      val |= (3 << BCHP_SDS_DFT_0_CTRL1_dft_avg_num_SHIFT); /* average number = 8 */
#endif
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_DFT_CTRL1, val);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_DFT_RANGE_START, 0);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_DFT_RANGE_END, 0x0F);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_DFT_DDFS_FCW, 0);
   return BSAT_g1_P_DftStartAndWaitForDone_isr(h, BSAT_g1_P_DftPsdScan2_isr);
}


/******************************************************************************
 BSAT_g1_P_DftPsdScan2_isr()
******************************************************************************/
BERR_Code BSAT_g1_P_DftPsdScan2_isr(BSAT_ChannelHandle h)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   uint32_t val;

   val = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_SDS_DFT_PEAK_POW);
#ifndef BCHP_SDS_DFT_0_CTRL1_dft_avg_mode_MASK
   hChn->count1++;
   hChn->count2 += val;
   if (hChn->count1 < 8)
      return BSAT_g1_P_DftPsdScan1_isr(h);
#endif

   BSAT_g1_P_DisableChannelInterrupts_isr(h);
   hChn->psdScanStatus.bScanDone = true;
   hChn->psdScanStatus.status = BERR_SUCCESS;
#ifdef BCHP_SDS_DFT_0_CTRL1_dft_avg_mode_MASK
   hChn->psdScanStatus.power = val;
#else
   hChn->psdScanStatus.power = hChn->count2 / 8;
#endif

   hChn->configParam[BSAT_g1_CONFIG_DFT_STATUS] |=  BSAT_g1_CONFIG_DFT_STATUS_DONE;
   hChn->acqState = BSAT_AcqState_eIdle;
   BSAT_g1_P_IndicateAcqDone_isr(h);
   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_DftToneDetect_isr()
******************************************************************************/
BERR_Code BSAT_g1_P_DftToneDetect_isr(BSAT_ChannelHandle h)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;

   /* reset the DFT */
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_DFT_CTRL0, 2);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_DFT_CTRL0, 0);

   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_DFT_RANGE_START, 0);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_DFT_RANGE_END, 0xFF);

   /* set to 6 HB */
   BSAT_g1_P_ReadModifyWriteRegister_isrsafe(h, BCHP_SDS_FE_FILTCTL, 0xFFFFFF00, 0x60);

   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_DFT_CTRL1, 0x013D);

   hChn->configParam[BSAT_g1_CONFIG_DFT_STATUS] = BSAT_g1_CONFIG_DFT_STATUS_PEAK_SCAN;
   BSAT_g1_P_DftSetDdfsFcw_isr(h);
   return BSAT_g1_P_DftStartAndWaitForDone_isr(h, BSAT_g1_P_DftToneDetect1_isr);
}


/******************************************************************************
 BSAT_g1_P_DftToneDetect1_isr()
******************************************************************************/
BERR_Code BSAT_g1_P_DftToneDetect1_isr(BSAT_ChannelHandle h)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   uint32_t peak_bin, peak_pow;

   peak_pow = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_SDS_DFT_PEAK_POW);
   peak_bin = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_SDS_DFT_PEAK_BIN) & 0x00000FFF;
   hChn->toneDetectStatus.peakPower = peak_pow;
   hChn->toneDetectStatus.peakBin = peak_bin;

   hChn->toneDetectStatus.bScanDone = true;
   BSAT_g1_P_DisableChannelInterrupts_isr(h);
   hChn->configParam[BSAT_g1_CONFIG_DFT_STATUS] |=  BSAT_g1_CONFIG_DFT_STATUS_DONE;
   hChn->acqState = BSAT_AcqState_eIdle;
   BSAT_g1_P_IndicateAcqDone_isr(h);
   return BERR_SUCCESS;
}
