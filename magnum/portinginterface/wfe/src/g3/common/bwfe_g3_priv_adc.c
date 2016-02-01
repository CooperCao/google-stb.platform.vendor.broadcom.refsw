/***************************************************************************
 *     Copyright (c) 2005-2014, Broadcom Corporation
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
#include "bwfe_g3_priv.h"

BDBG_MODULE(bwfe_g3_priv_adc);


#define BWFE_ACC_INTERVAL 255   /* interval is 0 to 255, 4096 samples per step */
#define BWFE_DAC_SEARCH_THRESH 2*4096*(BWFE_ACC_INTERVAL + 1)

#define BWFE_DEBUG_INL(x) /* x */
#define BWFE_DEBUG_EQU(x) /* x */
#define BWFE_DEBUG_DAC(x) /* x */
#define BWFE_DEBUG_CVR(x) /* x */


#if 0
#define BWFE_INL_MAX_ERR_DIFF 8
#define BWFE_INL_ERR6 6

/* E1 and E2 vectors containing capswap values */
static uint8_t capSwap[5][2] =
{
   {0x08, 0x01},
   {0x11, 0x03},
   {0x22, 0x07},
   {0x0C, 0x06},
   {0x10, 0x04}
};

/* Z = M * inv(A), scale by 30 for fixed point */
static int8_t zMatrix[7][6] =
{
   { 0,  0,  0,  0,  0, -30},
   {25, 20, 15, 10,  5, -20},
   {20, 40, 30, 20, 10, -10},
   {15, 30, 45, 30, 15,   0},
   {10, 20, 30, 40, 20,  10},
   { 5, 10, 15, 20, 25,  20},
   { 0,  0,  0,  0,  0,  30}
};

static uint8_t mdacShift[3] = {8, 6, 4};
static int8_t offsetT[7] = {3, 2, 1, 0, -1, -2, -3};


/* E1 and E2 vectors containing capswap values */
static uint8_t capSwapV2[5][2] =
{
   {0x10, 0x04},
   {0x0C, 0x06},
   {0x22, 0x07},
   {0x11, 0x03},
   {0x08, 0x01}
};

static uint8_t mdacShiftV2[3] = {9, 7, 5};
#endif


/******************************************************************************
 BWFE_g3_Adc_P_PowerUp()
******************************************************************************/
BERR_Code BWFE_g3_Adc_P_PowerUp(BWFE_ChannelHandle h)
{
   BWFE_g3_P_ChannelHandle *hChn = (BWFE_g3_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode = BERR_SUCCESS;
#ifdef BWFE_BYPASS_DGS
   uint8_t slice;
#endif
   
   BDBG_MSG(("ADC%d pwrup", h->channel));
   
   /* chip specific adc intialization */
   BWFE_P_InitAdc(h);
   h->bEnabled = true;
   
   /* reset lut and eq taps */
   BWFE_g3_Adc_P_ResetLutAll(h);
   BWFE_g3_Adc_P_ResetEquTaps(h);
   
   /* enable RFAGC after ADC power up */
   if (!h->bReference)
   {
      BWFE_P_OrRegister(h, BCHP_WFE_CORE_AGCCTL2, 0x00000001);    /* enable PN sequence */
      BWFE_P_ToggleBit(h, BCHP_WFE_CORE_AGCCTL1, 0x00010000);     /* toggle agc sync reset */   
      BWFE_P_ReadModifyWriteRegister(h, BCHP_WFE_CORE_AGCCTL1, ~0x01FCFFFF, (hChn->rfAgcBeta << 22) | (hChn->rfAgcK1 << 18) | hChn->rfAgcThresh);
      BWFE_P_AndRegister(h, BCHP_WFE_CORE_AGCCTL1, ~0x04020000);  /* unfreeze agc */
   }
   
   /* enable digisum2 for ping and pong lanes over all slices */
   BWFE_P_OrRegister(h, BCHP_WFE_CORE_DGSCTL2, 0x00020000);
   
#ifdef BWFE_BYPASS_DGS
   #ifdef BWFE_HYBRID_ADC
   BWFE_P_ReadModifyWriteRegister(h, BCHP_WFE_ANA_ADC_CNTL0, ~0x00802300, 0x00000000); /* 12 bit raw data output (default) + 2 pn bits  */

   /* need dgs2 for hybrid mdac calibration */
   for (slice = 0; slice < h->totalSlices; slice++)
   {
      BDBG_MSG(("unbypass digisum2 for slice%d", slice));
      BWFE_P_AndSliceReg(h, slice, BCHP_WFE_CORE_DGSCTL_PI_SLC, ~0x00000010);   /* unbypass digisum2 for ping lane */
      BWFE_P_AndSliceReg(h, slice, BCHP_WFE_CORE_DGSCTL_PO_SLC, ~0x00000010);   /* unbypass digisum2 for pong lane */
   }
   #else
   /* bypass dgs lanes */
   BWFE_P_ReadModifyWriteRegister(h, BCHP_WFE_ANA_ADC_CNTL0, ~0x00802300, 0x00000300); /* legacy 10-bit summed mode msb aligned for bypass */

   for (slice = 0; slice < h->totalSlices; slice++)
   {
      BDBG_MSG(("bypass digisum2 for slice%d", slice));
      BWFE_P_OrSliceReg(h, slice, BCHP_WFE_CORE_DGSCTL_PI_SLC, 0x00000010);   /* bypass digisum2 for ping lane */
      BWFE_P_OrSliceReg(h, slice, BCHP_WFE_CORE_DGSCTL_PO_SLC, 0x00000010);   /* bypass digisum2 for pong lane */
   }
   #endif
#endif
   
   return retCode;
}


/******************************************************************************
 BWFE_g3_Adc_P_PowerDown()
******************************************************************************/
BERR_Code BWFE_g3_Adc_P_PowerDown(BWFE_ChannelHandle h)
{
   BERR_Code retCode = BERR_SUCCESS;

   BDBG_MSG(("ADC%d pwrdown", h->channel));

   /* freeze RFAGC before ADC power down */
   if (BWFE_P_IsAdcOn(h))
      BWFE_P_OrRegister(h, BCHP_WFE_CORE_AGCCTL1, 0x04020000);

   /* power down ADC */
   retCode = BWFE_P_ShutdownAdc(h);
   h->bEnabled = false;

   return retCode;
}


/******************************************************************************
 BWFE_g3_Adc_P_CalibratePhase()
******************************************************************************/
BERR_Code BWFE_g3_Adc_P_CalibratePhase(BWFE_ChannelHandle h)
{
#if (BCHP_CHIP==4538)
   BWFE_g3_P_ChannelHandle *hChn = (BWFE_g3_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode = BERR_SUCCESS;
   bool bBusy;
   
   /* check if correlator in use */
   BKNI_EnterCriticalSection();
   bBusy = hChn->bCorrInProgress;
   if (!bBusy)
      hChn->bCorrInProgress = true;
   BKNI_LeaveCriticalSection();
     
   if (bBusy)
      return BWFE_ERR_CORR_BUSY;
   
   /* reset compensation delays */
   BWFE_P_WriteRegister(h, BCHP_WFE_ANA_WBADC_TIMING_CNTL_IN, 0x00000000);    /* reset coarse delays */
   BWFE_P_WriteRegister(h, BCHP_WFE_ANA_WBADC_TIMING_CNTL1_IN, 0x00000000);   /* reset fine delays */
   BWFE_P_ToggleBit(h, BCHP_WFE_ANA_WBADC_TIMING_CNTL_IN, 0x80808080);        /* latch in delays */
   BWFE_P_ToggleBit(h, BCHP_WFE_ANA_WBADC_TIMING_CNTL1_IN, 0x80808080);       /* latch in fine delays */
   
   /* use slice3 as reference */
   hChn->corrRefSlice = 3;
   
   /* enter CalibratePhase1() in ISR context */
   hChn->calState = 0;
   BKNI_EnterCriticalSection();
   retCode = BWFE_g3_P_EnableTimer(h, BWFE_g3_TimerSelect_e0, 5, BWFE_g3_Adc_P_CalibratePhase1);
   BKNI_LeaveCriticalSection();
   
   return retCode;
#else
   BSTD_UNUSED(h);
   return BERR_NOT_SUPPORTED;
#endif
}


/******************************************************************************
 BWFE_g3_Adc_P_CalibratePhase1() - ISR context
******************************************************************************/
BERR_Code BWFE_g3_Adc_P_CalibratePhase1(BWFE_ChannelHandle h)
{
#if (BCHP_CHIP==4538)
   BERR_Code retCode = BERR_SUCCESS;
   BWFE_g3_P_ChannelHandle *hChn = (BWFE_g3_P_ChannelHandle *)h->pImpl;
   BWFE_g3_P_Handle *hDev = h->pDevice->pImpl;
   uint8_t i, slice, phase, shift, minDelayPhase[BWFE_NUM_SLICES];
   int32_t minDelay[BWFE_NUM_SLICES];
   uint32_t val;
   
   while (1)
   {
      BDBG_MSG(("BWFE_g3_Adc_P_CalibratePhase1(): S%d", hChn->calState));
      switch (hChn->calState)
      {
         case 0:
            /* select correlator input */
            BWFE_P_WriteRegister(h, BCHP_WFE_CORE_CORRINSEL, hChn->corrInputSelect & 0x3);
            
            /* initialize state machine */
            hChn->calCurrPhase = 0;
            hChn->calState = 1;
            break;
         
         case 1:
             /* start of phase loop */
            if (hChn->calCurrPhase < BWFE_NUM_ADC_PHASES)
            {
               /* set slices 0,1,2 to current phase, leave slice3 as is */
               BWFE_P_AndRegister(h, BCHP_WFE_ANA_WBADC_SYS_CNTL_IN, ~0x0FF80000);
               val = (hChn->calCurrPhase << 25) | (hChn->calCurrPhase << 22) | (hChn->calCurrPhase << 19);
               BWFE_P_OrRegister(h, BCHP_WFE_ANA_WBADC_SYS_CNTL_IN, val);
               
               /* run delay calibration for each phase */
               hChn->calState = 2;
               hChn->postCalcDelayFunct = BWFE_g3_Adc_P_CalibratePhase1;
               hChn->bCorrInProgress = true;
               return BWFE_g3_Corr_P_StartCorrelator(h, hDev->dpmPilotFreqKhz * 1000, 0, BWFE_g3_Corr_P_CalcDelay);
            }
            else
            {
               /* exit phase loop */
               hChn->calState = 3;
            }
            break;
         
         case 2:
            /* store delays */
            for (i = 0; i < BWFE_NUM_SLICES; i++)
               hChn->calDelay[hChn->calCurrPhase][i] = hChn->corrDelay[i];
            
            hChn->calCurrPhase++;
            hChn->calState = 1;
            break;
         
         case 3:
            for (slice = 0; slice < BWFE_NUM_SLICES; slice++)
            {
               if (slice == hChn->corrRefSlice)
               {
                  minDelay[slice] = 0;
                  minDelayPhase[slice] = hChn->corrRefSlice;
               }
               else
               {
                  /* initialize minimum delays to max */
                  minDelay[slice] = 2147483647;
                  minDelayPhase[slice] = 0;
               }
            }
            
            /* find minimum delay and corresponding phase */
            for (slice = 0; slice < BWFE_NUM_SLICES; slice++)
            {
               if (slice != hChn->corrRefSlice)
               {
                  for (phase = 0; phase < BWFE_NUM_ADC_PHASES; phase++)
                  {
                     if (BWFE_ABS(hChn->calDelay[phase][slice]) < minDelay[slice])
                     {
                        minDelay[slice] = BWFE_ABS(hChn->calDelay[phase][slice]);
                        minDelayPhase[slice] = phase; /* save min index */
                     }
                  }
                  
                  /* program appropriate phase sequence */
                  shift = 19 + slice * 3;
                  BWFE_P_AndRegister(h, BCHP_WFE_ANA_WBADC_SYS_CNTL_IN, ~(0x7 << shift));
                  BWFE_P_OrRegister(h, BCHP_WFE_ANA_WBADC_SYS_CNTL_IN, (minDelayPhase[slice] << shift));
               }
               
               BDBG_MSG(("phase(ADC%d, slice%d)=%d", h->channel, slice, minDelayPhase[slice]));
            }
            
            /* signal ready event */
            BKNI_SetEvent(hChn->hWfeReady);
            return retCode;
         
         default:
            BDBG_ERR(("invalid state"));
            BERR_TRACE(retCode = BERR_UNKNOWN);
            break;
      }
   }
   
   return retCode;
#else
   BSTD_UNUSED(h);
   return BERR_NOT_SUPPORTED;
#endif
}


/******************************************************************************
 BWFE_g3_Adc_P_Accumulate() - return total accumulated value scaled by 4096
******************************************************************************/
BERR_Code BWFE_g3_Adc_P_Accumulate(BWFE_ChannelHandle h, uint8_t slice, uint8_t lane, uint8_t interval, int32_t *acc)
{
   BERR_Code retCode = BERR_SUCCESS;
   uint32_t val = 0;

   /* reset accumulators, note all accumulators controlled by ping lane slice0 */
   BWFE_P_ToggleBit(h, BCHP_WFE_CORE_DGSCTL2, 0x00000001);

   /* set accumulation length: 4096 samples per unit */
   BWFE_P_ReadModifyWriteRegister(h, BCHP_WFE_CORE_DGSCTL2, ~0x0000FF00, (interval & 0xFF) << 8);

   /* reset and start accumulator */
   BWFE_P_AndRegister(h, BCHP_WFE_CORE_DGSCTL2, ~0x00000002);
   BWFE_P_OrRegister(h, BCHP_WFE_CORE_DGSCTL2, 0x00000002);   /* edge detect, non self clearing */

   do {
      /* poll for dgsum2_accu_active = 0 */
      BWFE_P_ReadRegister(h, BCHP_WFE_CORE_HDOFFSTS, &val);
   }
   while (val & 0x2);

   /* stop accumulator */
   BWFE_P_AndRegister(h, BCHP_WFE_CORE_DGSCTL2, ~0x00000002);

   /* read accumulator with lane context */
   switch (lane)
   {
      case 0:
         BWFE_P_ReadSliceRegister(h, slice, BCHP_WFE_CORE_DGSACCUM_PI_SLC, &val);
         break;
      case 1:
         BWFE_P_ReadSliceRegister(h, slice, BCHP_WFE_CORE_DGSACCUM_PO_SLC, &val);
         break;
      default:
         retCode = BERR_INVALID_PARAMETER;
   }

   /* note cannot scale down here, must maintain precision, scale down later */
#ifdef BWFE_HYBRID_ADC
   *acc = (int32_t)val / 16;  /* value still scaled up by 4096, also need to divide by accumulation interval+1 to get average */ /* drop extra 4 fractional bits for mdac eq */
#else
   *acc = (int32_t)val / 4;   /* value still scaled up by 4096, also need to divide by accumulation interval+1 to get average */ /* drop extra 2 fractional bits for mdac eq */
#endif
   return retCode;
}


/******************************************************************************
 BWFE_g3_Adc_P_CalibrateINL() - integrated non-linearities algorithm (clocks on, RF off)
******************************************************************************/
BERR_Code BWFE_g3_Adc_P_CalibrateINL(BWFE_ChannelHandle h)
{
   BERR_Code retCode = BERR_NOT_SUPPORTED;
   
   if (!h->bEnabled)
      return BWFE_ERR_POWERED_DOWN;
   
   /* TBD */

   return retCode;
}


/******************************************************************************
 BWFE_g3_Adc_P_CoeffTc2Signed() - scale 2^17 to maintain resolution
******************************************************************************/
int32_t BWFE_g3_Adc_P_CoeffTc2Signed(uint32_t tc)
{
   int32_t val;
   
   /* sign extension if negative */
	if (tc > 0x07FFFFFF)
      tc -= 0x10000000;
   
   val = (int32_t)tc;
	return val / 512; /* scale up 2^17 for resolution, then down 2^26 */
}


/******************************************************************************
 BWFE_g3_Adc_P_CoeffSigned2Tc() - unscale 2^17
******************************************************************************/
uint32_t BWFE_g3_Adc_P_CoeffSigned2Tc(int32_t val)
{
   /* scale back up 2^26, note val scaled up 2^17 */
   val *= 512;
   
   /* truncate sign extension if negative */
	if (val < 0)
      val += 0x10000000;
   
   return (uint32_t)val;
}


#ifdef BWFE_HYBRID_ADC
/******************************************************************************
 BWFE_g3_Adc_P_CvarBinSearch() - Cap variance calibration with binary search
******************************************************************************/
#define BWFE_HALF_ACC_STEP 524288
BERR_Code BWFE_g3_Adc_P_CvarBinSearch(BWFE_ChannelHandle h, uint8_t slice, uint8_t lane, int32_t target)
{
   BERR_Code retCode = BERR_SUCCESS;
   uint32_t val, slcRegCvar;
   int32_t avgUpper, avgLower;
   uint8_t cvrUpper, cvrLower, cvrFinal, count;

   if (!h->bEnabled)
      return BWFE_ERR_POWERED_DOWN;

   BWFE_DEBUG_CVR(BDBG_MSG(("CvarBinSearch(slice%d,lane%d): target=%d", slice, lane, target)));

   count = 0;
   cvrUpper = 127;
   cvrLower = 0;

   if (slice == 0)
      slcRegCvar = BCHP_WFE_ANA_ADC_CNTL8;
   else
      slcRegCvar = BCHP_WFE_ANA_ADC_CNTL13;

   /* upper limit */
   BWFE_P_ReadModifyWriteRegister(h, slcRegCvar, ~0x0000007F, cvrUpper);
   retCode = BWFE_g3_Adc_P_Accumulate(h, slice, lane, BWFE_ACC_INTERVAL, &avgUpper);   /* set maximum accumulation length, 4096 samples per unit */
   if (retCode)
   {
      BDBG_ERR(("CvarBinSearch: slice%d/lane%d error 0x%X!", slice, lane, retCode));
   }

   /* lower limit */
   BWFE_P_ReadModifyWriteRegister(h, slcRegCvar, ~0x0000007F, cvrLower);
   retCode = BWFE_g3_Adc_P_Accumulate(h, slice, lane, BWFE_ACC_INTERVAL, &avgLower);   /* set maximum accumulation length, 4096 samples per unit */
   if (retCode)
   {
      BDBG_ERR(("CvarBinSearch: slice%d/lane%d error 0x%X!", slice, lane, retCode));
   }

   if (target > avgUpper)
   {
      cvrFinal = cvrUpper;
      BWFE_P_ReadModifyWriteRegister(h, slcRegCvar, ~0x0000007F, cvrUpper);
   }
   else if (target < avgLower)
   {
      cvrFinal = cvrLower;
      BWFE_P_ReadModifyWriteRegister(h, slcRegCvar, ~0x0000007F, cvrLower);
   }
   else
   {
      for (count = 0; count < 100; count++)
      {
         /* do until avgUpper within +/-0.5 of avgLower */
         if ((avgUpper > (avgLower - BWFE_HALF_ACC_STEP)) && (avgUpper < (avgLower + BWFE_HALF_ACC_STEP)))
            break;

         /* upper limit */
         BWFE_P_ReadModifyWriteRegister(h, slcRegCvar, ~0x0000007F, cvrUpper);
         retCode = BWFE_g3_Adc_P_Accumulate(h, slice, lane, BWFE_ACC_INTERVAL, &avgUpper);

         /* lower limit */
         BWFE_P_ReadModifyWriteRegister(h, slcRegCvar, ~0x0000007F, cvrLower);
         retCode = BWFE_g3_Adc_P_Accumulate(h, slice, lane, BWFE_ACC_INTERVAL, &avgLower);

         BWFE_DEBUG_CVR(BDBG_MSG(("cvrUpper=%d, cvrLower=%d | avgUpper=%d, avgLower=%d", cvrUpper, cvrLower, avgUpper, avgLower)));

         /* continue binary search */
         if (target > (avgUpper + avgLower) / 2)   /* note values scaled up by by acc length and 4096 */
            cvrLower = (cvrLower + cvrUpper) / 2 + 1;
         else
            cvrUpper = (cvrLower + cvrUpper) / 2;
      }

      if (count >= 100)
      {
         BDBG_ERR(("CvarBinSearch: Failed to converge!"));
         retCode = BERR_TIMEOUT;
      }
   }
   BWFE_DEBUG_CVR(BDBG_MSG(("[cvrLower=%d, cvrUpper=%d]", cvrLower, cvrUpper)));

   /* update final cvar value */
   BWFE_P_ReadRegister(h, slcRegCvar, &val);
   val &= 0x7F;
   val = (val << 21) | (val << 14) | (val << 7) | val;
   BWFE_P_WriteRegister(h, slcRegCvar, val);

   BWFE_DEBUG_CVR(BDBG_MSG(("Final cvar: 0x%08X", val)));
   return retCode;
}


/******************************************************************************
 BWFE_g3_Adc_P_EqualizeMdac() - note RFFE should be off during equalization
******************************************************************************/
BERR_Code BWFE_g3_Adc_P_EqualizeMdac(BWFE_ChannelHandle h)
{
   BWFE_g3_P_ChannelHandle *hChn = (BWFE_g3_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode = BERR_SUCCESS;

   if (hChn->equState < 6)
      return BWFE_ERR_EQU_BUSY;

   /* mdac equalization entry point */
   hChn->equState = 0;

   /* enter EqualizeMdac1() in ISR context */
   BKNI_EnterCriticalSection();
   retCode = BWFE_g3_P_EnableTimer(h, BWFE_g3_TimerSelect_e1, 5, BWFE_g3_Adc_P_EqualizeMdac1);
   BKNI_LeaveCriticalSection();

   return retCode;
}


/******************************************************************************
 BWFE_g3_Adc_P_EqualizeMdac1() - ISR context
******************************************************************************/
BERR_Code BWFE_g3_Adc_P_EqualizeMdac1(BWFE_ChannelHandle h)
{
   BWFE_g3_P_ChannelHandle *hChn = (BWFE_g3_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode = BERR_SUCCESS;
   uint32_t mask, val, slcRegCvar, slcRegCaldac, slice;
   int32_t offset, target;

   while (1)
   {
      BWFE_DEBUG_EQU(BDBG_MSG(("BWFE_g3_Adc_P_EqualizeMdac(): S%d", hChn->equState)));
      switch (hChn->equState)
      {
         case 0:
            /* power down RFFE if not reference adc */
            if (!h->bReference)
               BWFE_g3_Rffe_P_PowerDown(h);  /* TBD set sha output to zero instead of disabling rffe? */

            /* after delay, run cvar on both slices before mdac cal */
            hChn->equState = 1;
            return BWFE_g3_P_EnableTimer(h, BWFE_g3_TimerSelect_e1, 2000, BWFE_g3_Adc_P_EqualizeMdac1);

         case 1:
            /* reset cvar to default midpoint */
            BWFE_P_WriteRegister(h, BCHP_WFE_ANA_ADC_CNTL8, 0x08102040);
            BWFE_P_WriteRegister(h, BCHP_WFE_ANA_ADC_CNTL13, 0x08102040);

            /* disable caldac for both slices */
            BWFE_P_AndRegister(h, BCHP_WFE_ANA_ADC_CNTL6, ~0x10000000);
            BWFE_P_AndRegister(h, BCHP_WFE_ANA_ADC_CNTL9, ~0x10000000);

            /* determine cvar target */
            retCode = BWFE_g3_Adc_P_Accumulate(h, 0, 0, BWFE_ACC_INTERVAL, &offset); /* set maximum accumulation length, 4096 samples per unit */
            target = 64 * (BWFE_ACC_INTERVAL + 1) * 4096; /* scale target up by acc length and 4096 for comparison with accumulator */
            target += offset;    /* add offset to target */

            /* enable caldac for both slices */
            BWFE_P_OrRegister(h, BCHP_WFE_ANA_ADC_CNTL6, 0x10000000);
            BWFE_P_OrRegister(h, BCHP_WFE_ANA_ADC_CNTL9, 0x10000000);

            /* run cvar bin search on slice0 ping lane only */
            BWFE_g3_Adc_P_CvarBinSearch(h, 0, 0, target);

            /* copy cvar values from slice0 */
            BWFE_P_ReadRegister(h, BCHP_WFE_ANA_ADC_CNTL8, &val);
            BWFE_P_WriteRegister(h, BCHP_WFE_ANA_ADC_CNTL13, val);

            /* after delay, proceed to mdac cal */
            hChn->equCurrSlice = 0;
            hChn->equState = 2;
            return BWFE_g3_P_EnableTimer(h, BWFE_g3_TimerSelect_e1, 2000, BWFE_g3_Adc_P_EqualizeMdac1);

         case 2:
            /* start of slice loop */
            if (hChn->equCurrSlice < h->totalSlices)
            {
               if (hChn->equCurrSlice == 0)
               {
                  slcRegCvar = BCHP_WFE_ANA_ADC_CNTL8;
                  slcRegCaldac = BCHP_WFE_ANA_ADC_CNTL6;
               }
               else
               {
                  slcRegCvar = BCHP_WFE_ANA_ADC_CNTL13;
                  slcRegCaldac = BCHP_WFE_ANA_ADC_CNTL9;
               }
            #if 0
               /* set mu step size */
               val = (hChn->equMainMu1 << 20) | (hChn->equMu1 << 16) | (hChn->equMainMu2 << 12) | (hChn->equMu2 << 8);
               BWFE_P_WriteSliceRegister(h, hChn->equCurrSlice, BCHP_WFE_CORE_DGSLMSMU_SLC, val);
            #endif

               /* set mdac1 to calibration and turn on PN for current slice */
               BWFE_P_ReadModifyWriteRegister(h, slcRegCaldac, ~0x0000FFFF, 0x00004A7F);

            #if 0
               /* TBD skip mdac */
               hChn->equCurrMdac = 0;
            #else
               /* proceed to mdac loop */
               hChn->equCurrMdac = BWFE_NUM_MDACS;
               hChn->equState = 3;
            #endif
            }
            else
            {
               /* exit slice loop */
               hChn->equState = 5;
            }
            break;

         case 3:
            /* start of mdac loop nested in slice loop */
            if (hChn->equCurrMdac > 0)
            {
               /* bypass mdac if disabled */
               if ((hChn->equMdacEnable & (1 << (hChn->equCurrMdac - 1))) == 0)
               {
                  /* bypass mdac loop */
                  hChn->equCurrMdac--;
                  break;
               }

               BWFE_DEBUG_EQU(BDBG_MSG(("\n>>slice%d: mdac%d", hChn->equCurrSlice, hChn->equCurrMdac)));

               BWFE_P_OrSliceReg(h, hChn->equCurrSlice, BCHP_WFE_CORE_DGSLMS_SLC, 0x04000000);  /* no differential */
               BWFE_P_OrSliceReg(h, hChn->equCurrSlice, BCHP_WFE_CORE_DGSLMS_SLC, 0x03000000);  /* update ping and pong coefficients */

               /* enable PN input for LMS and update coefficients for mdac1 */
               mask = (hChn->equTapMask << 4) | 0x2;
               BWFE_P_ReadModifyWriteSliceReg(h, hChn->equCurrSlice, BCHP_WFE_CORE_DGSLMS_SLC, ~(0xF3 << 16), mask << 16);

               /* run for specified time */
               hChn->equState = 4;
               return BWFE_g3_P_EnableTimer(h, BWFE_g3_TimerSelect_e1, hChn->equRuntimeMs * 1000, BWFE_g3_Adc_P_EqualizeMdac1);
            }
            else
            {
               /* end of slice loop */
               hChn->equCurrSlice++;
               hChn->equState = 2;
            }
            break;

         case 4:
            /* disable PN input and stop update for mdac1 */
            BWFE_P_AndSliceReg(h, hChn->equCurrSlice, BCHP_WFE_CORE_DGSLMS_SLC, ~0x03F30000);

            /* revert to normal and disable PN sequence */
            BWFE_P_AndRegister(h, BCHP_WFE_ANA_ADC_CNTL6, ~0x0000FFFF);    /* slice 0 */
            BWFE_P_AndRegister(h, BCHP_WFE_ANA_ADC_CNTL9, ~0x0000FFFF);    /* slice 1 */

            /* end of mdac loop */
            hChn->equCurrMdac--;
            hChn->equState = 3;
            break;

         case 5:
            /* disable caldac for both slices */
            BWFE_P_AndRegister(h, BCHP_WFE_ANA_ADC_CNTL6, ~0x10000000); /* slice0 */
            BWFE_P_AndRegister(h, BCHP_WFE_ANA_ADC_CNTL9, ~0x10000000); /* slice1 */

            /* power up RFFE if not reference adc */
            if (!h->bReference)
               BWFE_g3_Rffe_P_PowerUp(h);

         #if 0
            /* TBD skip PN/BG cal */
            return retCode;
         #else
            hChn->equState = 6;
            break;
         #endif

         case 6:
            /* calibrate pn gain */
            BWFE_DEBUG_EQU(BDBG_MSG(("PN gain calibration...")));
            for (slice = 0; slice < BWFE_NUM_SLICES; slice++)
            {
               /* disable auto dc-tap reset, dc tap mode, 1 pre-main tap, 2 post-main taps */
               BWFE_P_ReadModifyWriteSliceReg(h, slice, BCHP_WFE_CORE_DGSLMS_SLC, ~0x1C0C0000, 0x00040000);
               BWFE_P_OrSliceReg(h, slice, BCHP_WFE_CORE_DGSLMS_SLC, 0x03000000);   /* update ping and pong coefficients */

               BWFE_P_WriteSliceRegister(h, slice, BCHP_WFE_CORE_DGSLMSMU_SLC, 0x00220000);  /* set mu step sizes */

               /* set background interval, disable background cal */
               BWFE_P_ReadModifyWriteSliceReg(h, slice, BCHP_WFE_CORE_DGSBGLMS_SLC, ~0x0000007D, 0x19 << 2);

               /* eq error power controls, beta=1, unbypass, unfreeze */
               BWFE_P_ReadModifyWriteSliceReg(h, slice, BCHP_WFE_CORE_DGSEPCTL_SLC, ~0x000000FB, 0x00000010);
               BWFE_P_ToggleSliceBit(h, slice, BCHP_WFE_CORE_DGSEPCTL_SLC, 0x00000004);   /* toggle integrator reset */

               /* set mdac1 to pn gain calibration and turn on PN for current slice */
               if (slice == 0)
                  BWFE_P_ReadModifyWriteRegister(h, BCHP_WFE_ANA_ADC_CNTL6, ~0x0000FFFF, 0x0000E000);    /* slice 0 */
               else
                  BWFE_P_ReadModifyWriteRegister(h, BCHP_WFE_ANA_ADC_CNTL9, ~0x0000FFFF, 0x0000E000);    /* slice 1 */

               /* enable pn gain calibration */
               BWFE_P_OrSliceReg(h, slice, BCHP_WFE_CORE_DGSBGLMS_SLC, 0x00000080);
            }

            /* enable mdac1 for pn gain calibration */
            BWFE_P_OrSliceReg(h, 0, BCHP_WFE_CORE_DGSLMS_SLC, 0x00020000);
            BWFE_P_OrSliceReg(h, 1, BCHP_WFE_CORE_DGSLMS_SLC, 0x00020000);

            /* reset mu step sizes */
            BWFE_P_WriteSliceRegister(h, 0, BCHP_WFE_CORE_DGSLMSMU_SLC, 0x00000000);   /* slice 0 */
            BWFE_P_WriteSliceRegister(h, 1, BCHP_WFE_CORE_DGSLMSMU_SLC, 0x00000000);   /* slice 1 */

            /* run for 3400ms */
            hChn->equState = 7;
            return BWFE_g3_P_EnableTimer(h, BWFE_g3_TimerSelect_e1, 3400000, BWFE_g3_Adc_P_EqualizeMdac1);

         case 7:
            BWFE_DEBUG_EQU(BDBG_MSG(("disable PN gain calibration")));
            for (slice = 0; slice < BWFE_NUM_SLICES; slice++)
            {
               BWFE_P_AndSliceReg(h, slice, BCHP_WFE_CORE_DGSLMS_SLC, ~0x03F30000);    /* disable pn input and stop update for mdac1 */
               BWFE_P_AndSliceReg(h, slice, BCHP_WFE_CORE_DGSBGLMS_SLC, ~0x00000080);  /* disable pn gain calibration */
            }

            BWFE_DEBUG_EQU(BDBG_MSG(("enable BG mdac cal!")));
            for (slice = 0; slice < BWFE_NUM_SLICES; slice++)
            {
               /* disable auto dc-tap reset, dc tap mode, 1 pre-main tap, 2 post-main taps */
               BWFE_P_ReadModifyWriteSliceReg(h, slice, BCHP_WFE_CORE_DGSLMS_SLC, ~0x1C0C0000, 0x00040000);
               BWFE_P_OrSliceReg(h, slice, BCHP_WFE_CORE_DGSLMS_SLC, 0x03000000);   /* update ping and pong coefficients */

               /* set mu step sizes */
               BWFE_P_WriteSliceRegister(h, slice, BCHP_WFE_CORE_DGSLMSMU_SLC, 0x00220000);

               /* eq error power controls, beta=1, unbypass, unfreeze */
               BWFE_P_ReadModifyWriteSliceReg(h, slice, BCHP_WFE_CORE_DGSEPCTL_SLC, ~0x000000FB, 0x00000010);
               BWFE_P_ToggleSliceBit(h, slice, BCHP_WFE_CORE_DGSEPCTL_SLC, 0x00000004);    /* toggle integrator reset */

               /* set background interval, disable background cal */
               BWFE_P_ReadModifyWriteSliceReg(h, slice, BCHP_WFE_CORE_DGSBGLMS_SLC, ~0x0000007D, 0x14 << 2);
               BWFE_P_OrSliceReg(h, slice, BCHP_WFE_CORE_DGSBGLMS_SLC, 0x00000001);    /*  enable background cal */

               /* enable PN input for LMS and update coefficients for mdac1 */
               mask = (hChn->equTapMask << 4) | 0x2;
               BWFE_P_ReadModifyWriteSliceReg(h, slice, BCHP_WFE_CORE_DGSLMS_SLC, ~(0xF3 << 16), mask << 16);
            }
            return retCode;

         default:
            BDBG_ERR(("invalid state"));
            BERR_TRACE(retCode = BERR_UNKNOWN);
            break;
      }
   }
   return retCode;
}

#else

/******************************************************************************
 BWFE_g3_Adc_P_DacBinSearch() - DAC calibration with binary search
******************************************************************************/
BERR_Code BWFE_g3_Adc_P_DacBinSearch(BWFE_ChannelHandle h, uint8_t slice, uint8_t lane, int32_t target)
{
   BERR_Code retCode = BERR_SUCCESS;
   int32_t avgUpper, avgLower;
   uint8_t msbUpper, msbLower, lsbUpper, lsbLower;
   uint8_t msbFinal, lsbFinal;
   
   if (!h->bEnabled)
      return BWFE_ERR_POWERED_DOWN;
   
   BWFE_DEBUG_DAC(BDBG_MSG(("DacBinSearch(lane%d): target=%d", lane, target)));
   
   msbFinal = lsbFinal = 0;
   lsbUpper = msbUpper = 31;
   lsbLower = msbLower = 0;
   
   if (target >= 256)
   {
      /* skip calibration and set maximum code */
      BWFE_P_OrRegister(h, BCHP_WFE_ANA_ADC_CNTL7, 0x0FFC0000);
      return retCode;
   }
   target *= (BWFE_ACC_INTERVAL + 1) * 4096;    /* scale target up by acc length and 4096 for comparison with accumulator */

   /* search msb */
   while (msbUpper != msbLower)
   {
      /* get upper average */
      BWFE_P_ReadModifyWriteRegister(h, BCHP_WFE_ANA_ADC_CNTL7, ~0x0FFC0000, msbUpper << 23 | lsbUpper << 18);
      retCode = BWFE_g3_Adc_P_Accumulate(h, slice, lane, BWFE_ACC_INTERVAL, &avgUpper);   /* set maximum accumulation length, 4096 samples per unit */
      if (retCode)
      {
         BDBG_ERR(("DacBinSearch: lane%d/msb%d/lsb%d error 0x%X!", lane, msbUpper, lsbUpper, retCode));
      }
      
      /* get lower average */
      BWFE_P_ReadModifyWriteRegister(h, BCHP_WFE_ANA_ADC_CNTL7, ~0x0FFC0000, msbLower << 23 | lsbLower << 18);
      retCode = BWFE_g3_Adc_P_Accumulate(h, slice, lane, BWFE_ACC_INTERVAL, &avgLower);   /* set maximum accumulation length, 4096 samples per unit */
      if (retCode)
      {
         BDBG_ERR(("DacBinSearch: lane%d/msb%d/lsb%d error 0x%X!", lane, msbLower, lsbLower, retCode));
      }
      
      BWFE_DEBUG_DAC(BDBG_MSG(("msbUpper=%d, msbLower=%d | avgUpper=%d, avgLower=%d", msbUpper, msbLower, avgUpper, avgLower)));
      
      /* continue binary search */
      if (target > ((avgUpper + avgLower) / 2))    /* note values scaled up by by acc length and 4096 */
         msbFinal = msbLower = (msbLower + msbUpper) / 2 + 1;
      else
         msbFinal = msbUpper = (msbLower + msbUpper) / 2;
   }
   
   /* set final msb */
   BWFE_DEBUG_DAC(BDBG_MSG(("msbFinal=%d", msbFinal)));
   BWFE_P_ReadModifyWriteRegister(h, BCHP_WFE_ANA_ADC_CNTL7, ~0x0F800000, msbFinal << 23);
   
   /* search lsb */
   while (lsbUpper != lsbLower)
   {
      /* get upper average */
      BWFE_P_ReadModifyWriteRegister(h, BCHP_WFE_ANA_ADC_CNTL7, ~0x007C0000, lsbUpper << 18);
      retCode = BWFE_g3_Adc_P_Accumulate(h, slice, lane, BWFE_ACC_INTERVAL, &avgUpper);   /* set maximum accumulation length, 4096 samples per unit */
      if (retCode)
      {
         BDBG_ERR(("DacBinSearch: lane%d/msb%d/lsb%d error 0x%X!", lane, msbFinal, lsbUpper, retCode));
      }
      
      /* get lower average */
      BWFE_P_ReadModifyWriteRegister(h, BCHP_WFE_ANA_ADC_CNTL7, ~0x007C0000, lsbLower << 18);
      retCode = BWFE_g3_Adc_P_Accumulate(h, slice, lane, BWFE_ACC_INTERVAL, &avgLower);   /* set maximum accumulation length, 4096 samples per unit */
      if (retCode)
      {
         BDBG_ERR(("DacBinSearch: lane%d/msb%d/lsb%d error 0x%X!", lane, msbFinal, lsbLower, retCode));
      }
      
      BWFE_DEBUG_DAC(BDBG_MSG(("lsbUpper=%d, lsbLower=%d | avgUpper=%d, avgLower=%d", lsbUpper, lsbLower, avgUpper, avgLower)));
      
      /* exit condition */
      if (target == avgUpper)
         lsbFinal = lsbLower = lsbUpper;
      else if (target == avgLower)
         lsbFinal = lsbUpper = lsbLower;
      else
      {
         /* otherwise continue binary search */
         if (target > (avgUpper + avgLower) / 2)   /* note values scaled up by by acc length and 4096 */
            lsbFinal = lsbLower = (lsbLower + lsbUpper) / 2 + 1;
         else
            lsbFinal = lsbUpper = (lsbLower + lsbUpper) / 2;
      }
   }
   
   /* set final lsb */
   BWFE_DEBUG_DAC(BDBG_MSG(("lsbFinal=%d", lsbFinal)));
   BWFE_P_ReadModifyWriteRegister(h, BCHP_WFE_ANA_ADC_CNTL7, ~0x007C0000, lsbFinal << 18);
   
   /* get final average */
   retCode = BWFE_g3_Adc_P_Accumulate(h, slice, lane, BWFE_ACC_INTERVAL, &avgLower);   /* set maximum accumulation length, 4096 samples per unit */
   BWFE_DEBUG_DAC(BDBG_MSG(("avgFinal=%d", avgLower / ((BWFE_ACC_INTERVAL + 1) * 4096))));  /* div by acc length to get avg, scale back down by 4096 */
   
   if (BWFE_ABS(target - avgLower) > BWFE_DAC_SEARCH_THRESH)
      retCode = BWFE_ERR_DAC_BIN_SEARCH_FAIL;
   
   return retCode;
}


/******************************************************************************
 BWFE_g3_Adc_P_EqualizeMdac() - note RFFE should be off during equalization
******************************************************************************/
BERR_Code BWFE_g3_Adc_P_EqualizeMdac(BWFE_ChannelHandle h)
{
   BWFE_g3_P_ChannelHandle *hChn = (BWFE_g3_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode = BERR_SUCCESS;
   
   if (hChn->equState < 4)
      return BWFE_ERR_EQU_BUSY;
   
   /* mdac equalization entry point */
   hChn->equState = 0;
   
   /* enter EqualizeMdac1() in ISR context */
   BKNI_EnterCriticalSection();
   retCode = BWFE_g3_P_EnableTimer(h, BWFE_g3_TimerSelect_e1, 5, BWFE_g3_Adc_P_EqualizeMdac1);
   BKNI_LeaveCriticalSection();
   
   return retCode;
}


/******************************************************************************
 BWFE_g3_Adc_P_EqualizeMdac1() - ISR context
******************************************************************************/
BERR_Code BWFE_g3_Adc_P_EqualizeMdac1(BWFE_ChannelHandle h)
{
   BWFE_g3_P_ChannelHandle *hChn = (BWFE_g3_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode = BERR_SUCCESS;
   uint32_t mask, val;
   uint8_t target, shift;
   
   while (1)
   {
      BWFE_DEBUG_EQU(BDBG_MSG(("BWFE_g3_Adc_P_EqualizeMdac(): S%d", hChn->equState)));
      switch (hChn->equState)
      {
         case 0:
            /* power down RFFE if not reference adc */
            if (!h->bReference)
               BWFE_g3_Rffe_P_PowerDown(h);  /* TBD set sha output to zero instead of disabling rffe? */
            
            /* power up caldac, max current */
            BWFE_P_OrRegister(h, BCHP_WFE_ANA_ADC_CNTL7, 0x10030000);
            BWFE_P_OrRegister(h, BCHP_WFE_ANA_ADC_CNTL4, 0x0E000000);
            
            hChn->equCurrSlice = 0;
            hChn->equState = 1;
            break;
         
         case 1:
            /* start of slice loop */
            if (hChn->equCurrSlice < h->totalSlices)
            {
               /* set mu step size */
               val = (hChn->equMainMu1 << 20) | (hChn->equMu1 << 16) | (hChn->equMainMu2 << 12) | (hChn->equMu2 << 8);
               BWFE_P_WriteSliceRegister(h, hChn->equCurrSlice, BCHP_WFE_CORE_DGSLMSMU_SLC, val);
               
               /* proceed to mdac loop */
               hChn->equCurrMdac = BWFE_NUM_MDACS;
               hChn->equState = 2;
            }
            else
            {
               /* exit slice loop */
               hChn->equState = 4;
            }
            break;
         
         case 2:
            /* start of mdac loop nested in slice loop */
            if (hChn->equCurrMdac > 0)
            {
               /* bypass mdac if disabled */
               if ((hChn->equMdacEnable & (1 << (hChn->equCurrMdac - 1))) == 0)
               {
                  /* bypass mdac loop */
                  hChn->equCurrMdac--;
                  break;
               }
               
               BWFE_DEBUG_EQU(BDBG_MSG(("\n>>slice%d: mdac%d", hChn->equCurrSlice, hChn->equCurrMdac)));
               
               /* target 16 for mdac2, 64 for mdac1 */
               target = (BWFE_NUM_MDACS - hChn->equCurrMdac + 1) << 2;
               target *= target;
               
               BWFE_g3_Adc_P_DacBinSearch(h, hChn->equCurrSlice, 0, target);  /* dac binary search for ping */
               
               if (hChn->equCurrMdac == 1)
               {
                  /* set first mdac to calibration and turn on PN */
                  BWFE_P_ReadModifyWriteRegister(h, BCHP_WFE_ANA_ADC_CNTL6, ~0xFFFF0000, 0x8A7F0000);
               }
               else
               {
                  /* set second mdac to calibration and turn on PN */
                  BWFE_P_ReadModifyWriteRegister(h, BCHP_WFE_ANA_ADC_CNTL7, ~0x0000FFFF, 0x00008A7F);
               }
               
               BWFE_P_OrSliceReg(h, hChn->equCurrSlice, BCHP_WFE_CORE_DGSLMS_SLC, 0x0C000000);  /* seperate differential seperately for ping and pong lane */
               BWFE_P_OrSliceReg(h, hChn->equCurrSlice, BCHP_WFE_CORE_DGSLMS_SLC, 0x03000000);  /* update ping and pong coefficients */
               
               /* enable PN input for LMS and update coefficients for corresponding mdac */
               shift = 8 * (3 - hChn->equCurrMdac);
               mask = (hChn->equTapMask << 4) | 0x2;
               BWFE_P_ReadModifyWriteSliceReg(h, hChn->equCurrSlice, BCHP_WFE_CORE_DGSLMS_SLC, ~(0xF3 << shift), mask << shift);
               
               /* run for specified time */
               hChn->equState = 3;
               return BWFE_g3_P_EnableTimer(h, BWFE_g3_TimerSelect_e1, hChn->equRuntimeMs * 1000, BWFE_g3_Adc_P_EqualizeMdac1);
            }
            else
            {
               /* end of slice loop */
               hChn->equCurrSlice++;
               hChn->equState = 1;
            }
            break;
         
         case 3:
            /* disable PN input and stop update for both mdacs */
            BWFE_P_AndSliceReg(h, hChn->equCurrSlice, BCHP_WFE_CORE_DGSLMS_SLC, ~0x03F3F300);
            
            /* revert to normal and disable PN sequence */
            BWFE_P_AndRegister(h, BCHP_WFE_ANA_ADC_CNTL6, ~0xFFFF0000); /* set first mdac back to normal and turn off PN */
            BWFE_P_AndRegister(h, BCHP_WFE_ANA_ADC_CNTL7, ~0x0000FFFF); /* set second mdac back to normal and turn off PN */
            
            /* end of mdac loop */
            hChn->equCurrMdac--;
            hChn->equState = 2;
            break;
         
         case 4:
            /* power down caldac */
            BWFE_P_AndRegister(h, BCHP_WFE_ANA_ADC_CNTL7, ~0x10030000);
            
            /* power up RFFE if not reference adc */
            if (!h->bReference)
               BWFE_g3_Rffe_P_PowerUp(h);
            return retCode;
         
         default:
            BDBG_ERR(("invalid state"));
            BERR_TRACE(retCode = BERR_UNKNOWN);
            break;
      }
   }
   return retCode;
}
#endif


/******************************************************************************
 BWFE_g3_Adc_P_ResetLut()
******************************************************************************/
BERR_Code BWFE_g3_Adc_P_ResetLut(BWFE_ChannelHandle h, uint8_t slice, uint8_t mdac)
{
   uint32_t mask = 0xFFF >> (mdac - 1);
   uint8_t shift = (BWFE_NUM_MDACS - mdac + 1) * 10 - (mdac - 1);
   
   /* clear LUT values for ping lane for specified mdac */
   BWFE_P_AndSliceReg(h, slice, BCHP_WFE_CORE_DGSLUT011_PI_SLC, ~(mask << shift));
   BWFE_P_AndSliceReg(h, slice, BCHP_WFE_CORE_DGSLUT010_PI_SLC, ~(mask << shift));
   BWFE_P_AndSliceReg(h, slice, BCHP_WFE_CORE_DGSLUT001_PI_SLC, ~(mask << shift));
   BWFE_P_AndSliceReg(h, slice, BCHP_WFE_CORE_DGSLUT000_PI_SLC, ~(mask << shift));
   BWFE_P_AndSliceReg(h, slice, BCHP_WFE_CORE_DGSLUT111_PI_SLC, ~(mask << shift));
   BWFE_P_AndSliceReg(h, slice, BCHP_WFE_CORE_DGSLUT110_PI_SLC, ~(mask << shift));
   BWFE_P_AndSliceReg(h, slice, BCHP_WFE_CORE_DGSLUT101_PI_SLC, ~(mask << shift));
   BWFE_P_AndSliceReg(h, slice, BCHP_WFE_CORE_DGSLUT100_PI_SLC, ~(mask << shift));
   
   /* clear LUT values for pong lane for specified mdac */
   BWFE_P_AndSliceReg(h, slice, BCHP_WFE_CORE_DGSLUT011_PO_SLC, ~(mask << shift));
   BWFE_P_AndSliceReg(h, slice, BCHP_WFE_CORE_DGSLUT010_PO_SLC, ~(mask << shift));
   BWFE_P_AndSliceReg(h, slice, BCHP_WFE_CORE_DGSLUT001_PO_SLC, ~(mask << shift));
   BWFE_P_AndSliceReg(h, slice, BCHP_WFE_CORE_DGSLUT000_PO_SLC, ~(mask << shift));
   BWFE_P_AndSliceReg(h, slice, BCHP_WFE_CORE_DGSLUT111_PO_SLC, ~(mask << shift));
   BWFE_P_AndSliceReg(h, slice, BCHP_WFE_CORE_DGSLUT110_PO_SLC, ~(mask << shift));
   BWFE_P_AndSliceReg(h, slice, BCHP_WFE_CORE_DGSLUT101_PO_SLC, ~(mask << shift));
   BWFE_P_AndSliceReg(h, slice, BCHP_WFE_CORE_DGSLUT100_PO_SLC, ~(mask << shift));
   
   return BERR_SUCCESS;
}


/******************************************************************************
 BWFE_g3_Adc_P_ResetLutAll()
******************************************************************************/
BERR_Code BWFE_g3_Adc_P_ResetLutAll(BWFE_ChannelHandle h)
{
   uint8_t slice, mdac;
   
   for (slice = 0; slice < h->totalSlices; slice++)
   {
      for (mdac = BWFE_NUM_MDACS; mdac > 0; mdac--)
         BWFE_g3_Adc_P_ResetLut(h, slice, mdac);
   }
   return BERR_SUCCESS;
}


/******************************************************************************
 BWFE_g3_Adc_P_ResetEquTaps()
******************************************************************************/
BERR_Code BWFE_g3_Adc_P_ResetEquTaps(BWFE_ChannelHandle h)
{
   BWFE_g3_P_ChannelHandle *hChn = (BWFE_g3_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode = BERR_SUCCESS;
   uint32_t val = 0;
   uint8_t slice;
 
   for (slice = 0; slice < h->totalSlices; slice++)
   {
      /* reset stage 1 and 2 LMS coefficients */
      BWFE_P_ToggleSliceBit(h, slice, BCHP_WFE_CORE_DGSLMS_SLC, 0x00010100);
      
      /* set mu step size */
      val = (hChn->equMainMu1 << 20) | (hChn->equMu1 << 16) | (hChn->equMainMu2 << 12) | (hChn->equMu2 << 8);
      BWFE_P_WriteSliceRegister(h, slice, BCHP_WFE_CORE_DGSLMSMU_SLC, val);
   }
   
   /* refresh eq taps after reset */
   BWFE_g3_Adc_P_ReadEquTaps(h);
   
   return retCode;
}


/******************************************************************************
 BWFE_g3_Adc_P_ReadEquTaps()
******************************************************************************/
BERR_Code BWFE_g3_Adc_P_ReadEquTaps(BWFE_ChannelHandle h)
{
   BWFE_g3_P_ChannelHandle *hChn = (BWFE_g3_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode = BERR_SUCCESS;
   uint8_t slice, mdac, tap;
   uint32_t val;
 
   for (slice = 0; slice < h->totalSlices; slice++)
   {
      for (mdac = BWFE_NUM_MDACS; mdac > 0; mdac--)
      {
         /* select ping lane for dgs coeff */
         BWFE_P_ReadModifyWriteSliceReg(h, slice, BCHP_WFE_CORE_DGSCOEFEN_SLC, ~0x00000003, 0x00000001);
         BWFE_P_ToggleSliceBit(h, slice, BCHP_WFE_CORE_DGSCOEFEN_SLC, 0x00000008);  /* toggle capture bit */
         BWFE_P_WriteSliceRegister(h, slice, BCHP_WFE_CORE_DGSCOEFA_SLC, (mdac - 1) * 4);   /* auto-incrementing address */
         for (tap = 0; tap < BWFE_NUM_EQ_TAPS; tap++)
         {
            /* read equalizer coefficients */
            BWFE_P_ReadSliceRegister(h, slice, BCHP_WFE_CORE_DGSCOEFD_SLC, &val);
            hChn->equCoeff[slice][0][mdac-1][tap] = BWFE_g3_Adc_P_CoeffTc2Signed(val);
         }
         
         /* select pong lane for dgs coeff */
         BWFE_P_ReadModifyWriteSliceReg(h, slice, BCHP_WFE_CORE_DGSCOEFEN_SLC, ~0x00000003, 0x00000002);
         BWFE_P_ToggleSliceBit(h, slice, BCHP_WFE_CORE_DGSCOEFEN_SLC, 0x00000008);  /* toggle capture bit */
         BWFE_P_WriteSliceRegister(h, slice, BCHP_WFE_CORE_DGSCOEFA_SLC, (mdac - 1) * 4);   /* auto-incrementing address */
         for (tap = 0; tap < BWFE_NUM_EQ_TAPS; tap++)
         {
            /* read equalizer coefficients */
            BWFE_P_ReadSliceRegister(h, slice, BCHP_WFE_CORE_DGSCOEFD_SLC, &val);
            hChn->equCoeff[slice][1][mdac-1][tap] = BWFE_g3_Adc_P_CoeffTc2Signed(val);
         }
         
         /* deselect lanes after read */
         BWFE_P_AndSliceReg(h, slice, BCHP_WFE_CORE_DGSCOEFEN_SLC, ~0x00000003);
      }
   }
   
#if (BCHP_CHIP==4538)
   /* propagate ref taps to all slices */
   if (h->bReference)
   {
      for (slice = 1; slice < BWFE_NUM_SLICES; slice++)
      {
         for (mdac = BWFE_NUM_MDACS; mdac > 0; mdac--)
         {
            for (tap = 0; tap < BWFE_NUM_EQ_TAPS; tap++)
            {
               hChn->equCoeff[slice][0][mdac-1][tap] = hChn->equCoeff[0][0][mdac-1][tap];
               hChn->equCoeff[slice][1][mdac-1][tap] = hChn->equCoeff[0][1][mdac-1][tap];
            }
         }
      }
   }
#endif
   
   return retCode;
}


/******************************************************************************
 BWFE_g3_Adc_P_WriteEquTaps()
******************************************************************************/
BERR_Code BWFE_g3_Adc_P_WriteEquTaps(BWFE_ChannelHandle h)
{
   BWFE_g3_P_ChannelHandle *hChn = (BWFE_g3_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode = BERR_SUCCESS;
   uint8_t slice, mdac, tap;
   uint32_t val;
 
   for (slice = 0; slice < h->totalSlices; slice++)
   {
      for (mdac = BWFE_NUM_MDACS; mdac > 0; mdac--)
      {
         /* select ping lane for dgs coeff */
         BWFE_P_ReadModifyWriteSliceReg(h, slice, BCHP_WFE_CORE_DGSCOEFEN_SLC, ~0x00000003, 0x00000001);
         BWFE_P_WriteSliceRegister(h, slice, BCHP_WFE_CORE_DGSCOEFA_SLC, (mdac - 1) * 4);   /* auto-incrementing address */
         for (tap = 0; tap < BWFE_NUM_EQ_TAPS; tap++)
         {
            /* write equalizer coefficients */
            val = BWFE_g3_Adc_P_CoeffSigned2Tc(hChn->equCoeff[slice][0][mdac-1][tap]);
            BWFE_P_WriteSliceRegister(h, slice, BCHP_WFE_CORE_DGSCOEFD_SLC, val);
         }
         BWFE_P_ToggleSliceBit(h, slice, BCHP_WFE_CORE_DGSCOEFEN_SLC, 0x00000004);  /* toggle load bit */
         
         /* select pong lane for dgs coeff */
         BWFE_P_ReadModifyWriteSliceReg(h, slice, BCHP_WFE_CORE_DGSCOEFEN_SLC, ~0x00000003, 0x00000002);
         BWFE_P_WriteSliceRegister(h, slice, BCHP_WFE_CORE_DGSCOEFA_SLC, (mdac - 1) * 4);   /* auto-incrementing address */
         for (tap = 0; tap < BWFE_NUM_EQ_TAPS; tap++)
         {
            /* write equalizer coefficients */
            val = BWFE_g3_Adc_P_CoeffSigned2Tc(hChn->equCoeff[slice][1][mdac-1][tap]);
            BWFE_P_WriteSliceRegister(h, slice, BCHP_WFE_CORE_DGSCOEFD_SLC, val);
         }
         BWFE_P_ToggleSliceBit(h, slice, BCHP_WFE_CORE_DGSCOEFEN_SLC, 0x00000004);  /* toggle load bit */
         
         /* deselect lanes after write */
         BWFE_P_AndSliceReg(h, slice, BCHP_WFE_CORE_DGSCOEFEN_SLC, ~0x00000003);
      }
   }
   
   return retCode;
}

