/***************************************************************************
 *     Copyright (c) 2005-2012, Broadcom Corporation
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

BDBG_MODULE(bwfe_priv_adc);

#define BWFE_INL_ACC_INTERVAL 255   /* interval is 0 to 255, 4096 samples per step */
#define BWFE_INL_ACC_SATURATION 0x40000000
#define BWFE_INL_MAX_ERR_DIFF 8
#define BWFE_INL_ERR6 6

#define BWFE_DCO_ACC_INTERVAL 255
#define BWFE_DCO_ACC_COUNT 2

#define BWFE_DEBUG_INL(x) /* x */
#define BWFE_DEBUG_EQU(x) /* x */
#define BWFE_DEBUG_DCO(x) x


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



/******************************************************************************
 BWFE_Adc_P_PowerUp()
******************************************************************************/
BERR_Code BWFE_Adc_P_PowerUp(BWFE_ChannelHandle h)
{
   BERR_Code retCode = BERR_SUCCESS;
   uint8_t i;
   
   BDBG_MSG(("ADC%d pwrup", h->channel));
   h->bEnabled = true;
   
   BWFE_P_SetAdcCurrent(h);   /* chip specific current settings */
   BWFE_P_OrRegister(h, BCHP_WFE_ANA_U7_WBADC_SYS_CNTL_IN, 0x00000001); /* power up ADC */
   
   /* wait for ADC power up */
   BKNI_Sleep(1);
   
   /* turn on reference lane and reset ADC */
   BWFE_P_PowerUpRefLane(h);     /* wake ADC2 for 4550, ADC4 for 4528 */
   BWFE_P_ResetAdc(h);           /* reference lane must be on for ADC reset */
   BWFE_P_PowerDownRefLane(h);   /* sleep ADC2 for 4550, ADC4 for 4528 */
   
   /* enable ADC output, use negative edge */
   BWFE_P_OrRegister(h, BCHP_WFE_CORE_HDOFFCTL0, 0x002003FF);
   
   /* same phase for output clock */
   BWFE_P_AndRegister(h, BCHP_WFE_ANA_U7_ADC_CNTL0, ~0x00000001);
   
   /* reset lut and eq taps */
   BWFE_Adc_P_ResetLut(h);
   BWFE_Adc_P_ResetEqTaps(h);
   
   /* enable RFAGC after ADC power up */
   BWFE_P_OrRegister(h, BCHP_WFE_CORE_AGCCTL2, 0x00000001);    /* enable PN sequence */
   BWFE_P_AndRegister(h, BCHP_WFE_CORE_AGCCTL1, ~0x02E10000);  /* unfreeze, leaky averager beta=9 */
   
   /* enable digisum2 for each lane including reference lanes */
   for (i = 0; i < h->totalLanes + 2; i++)
      BWFE_P_OrLaneReg(h, i, BCHP_WFE_CORE_DGSCTL_LN, 0x00200000);
   
#ifdef BWFE_BYPASS_DGS
   /* bypass dgs lanes for bringup */
   BWFE_P_OrRegister(h, BCHP_WFE_ANA_U7_ADC_CNTL0, 0x00800000);   /* legacy 12-bit output for bypass mode */
   BDBG_MSG(("bypass lanes 0 1 2 3 ref0 ref1"));
   for (i = 0; i < h->totalLanes + 2; i++)
      BWFE_P_OrLaneReg(h, i, BCHP_WFE_CORE_DGSCTL_LN, 0x00100000);   /* bypass digisum2 for all lanes */
#endif
   
   return retCode;
}


/******************************************************************************
 BWFE_Adc_P_PowerDown()
******************************************************************************/
BERR_Code BWFE_Adc_P_PowerDown(BWFE_ChannelHandle h)
{
   BERR_Code retCode = BERR_SUCCESS;
   
   BDBG_MSG(("ADC%d pwrdown", h->channel));
   h->bEnabled = false;
   
   /* freeze RFAGC before ADC power down */
   BWFE_P_OrRegister(h, BCHP_WFE_CORE_AGCCTL1, 0x02010000);
   
   /* power down ADC */
   BWFE_P_AndRegister(h, BCHP_WFE_ANA_U7_WBADC_SYS_CNTL_IN, ~0x00000001);
   
   return retCode;
}


/******************************************************************************
 BWFE_Adc_P_ReadAccumulator() - return total accumulated value scaled by 4096
******************************************************************************/
BERR_Code BWFE_Adc_P_ReadAccumulator(BWFE_ChannelHandle h, uint8_t lane, uint8_t accLength, int32_t *acc)
{
   BERR_Code retCode = BERR_SUCCESS;
   uint32_t val = 0;
   
   /* reset accumulators, note all accumulators controlled by lane 0 */
   BWFE_P_OrLaneReg(h, 0, BCHP_WFE_CORE_DGSCTL_LN, 0x00000001);
   BWFE_P_AndLaneReg(h, 0, BCHP_WFE_CORE_DGSCTL_LN, ~0x00000001);
   
   /* set accumulation length: 4096 samples per unit */
   BWFE_P_ReadModifyWriteLaneReg(h, 0, BCHP_WFE_CORE_DGSCTL_LN, ~0x0000FF00, (accLength & 0xFF) << 8);
   
   /* start accumulator */
   BWFE_P_AndLaneReg(h, 0, BCHP_WFE_CORE_DGSCTL_LN, ~0x00000002);
   BWFE_P_OrLaneReg(h, 0, BCHP_WFE_CORE_DGSCTL_LN, 0x00000002);   /* edge detect, non self clearing */
   
   do {
      /* poll for dgsum2_accu_active = 0 */
      BWFE_P_ReadRegister(h, BCHP_WFE_CORE_HDOFFSTS, &val);
   }
   while (val & 0x2);
   
   /* stop accumulator */
   BWFE_P_AndLaneReg(h, 0, BCHP_WFE_CORE_DGSCTL_LN, ~0x00000002);
   
   /* read accumulator with lane context */
   BWFE_P_ReadLaneRegister(h, lane, BCHP_WFE_CORE_DGSACCUM_LN, &val);  
   *acc = (int32_t)val;
   
#if 0
   /* limit accumulators if saturated */
   if ((*acc < -BWFE_INL_ACC_SATURATION) || (*acc > BWFE_INL_ACC_SATURATION))
   {
      *acc = (*acc > 0) ? BWFE_INL_ACC_SATURATION : -BWFE_INL_ACC_SATURATION;
      h->accSaturateCount++;
      retCode = BWFE_ERR_ACC_SATURATED;
   }
#endif
   
   /* note value is scaled up by 4096, need to divide by accumulation ticks to get average */
   return retCode;
}


/******************************************************************************
 BWFE_Adc_P_ReadAccumulatorAllLanes() - return total accumulated value scaled by 4096
******************************************************************************/
BERR_Code BWFE_Adc_P_ReadAccumulatorAllLanes(BWFE_ChannelHandle h, uint8_t accLength, int32_t *pAcc)
{
   BERR_Code retCode = BERR_SUCCESS;
   uint32_t val = 0;
   uint8_t lane;
   
   /* reset accumulators, note all accumulators controlled by lane 0 */
   BWFE_P_OrLaneReg(h, 0, BCHP_WFE_CORE_DGSCTL_LN, 0x00000001);
   BWFE_P_AndLaneReg(h, 0, BCHP_WFE_CORE_DGSCTL_LN, ~0x00000001);
   
   /* set accumulation length: 4096 samples per unit */
   BWFE_P_ReadModifyWriteLaneReg(h, 0, BCHP_WFE_CORE_DGSCTL_LN, ~0x0000FF00, (accLength & 0xFF) << 8);
   
   /* start accumulator */
   BWFE_P_AndLaneReg(h, 0, BCHP_WFE_CORE_DGSCTL_LN, ~0x00000002);
   BWFE_P_OrLaneReg(h, 0, BCHP_WFE_CORE_DGSCTL_LN, 0x00000002);   /* edge detect, non self clearing */
   
   do {
      /* sleep until dgsum2_accu_active = 0 */
      BKNI_Sleep(3);
      BWFE_P_ReadRegister(h, BCHP_WFE_CORE_HDOFFSTS, &val);
   }
   while (val & 0x2);
   
   /* stop accumulator */
   BWFE_P_AndLaneReg(h, 0, BCHP_WFE_CORE_DGSCTL_LN, ~0x00000002);
   
   /* read accumulators */
   for (lane = 0; lane < h->totalLanes; lane++)
   {
      BWFE_P_ReadLaneRegister(h, lane, BCHP_WFE_CORE_DGSACCUM_LN, &val);
      pAcc[lane] = val;
      //printf("acc: %08X?=%08X\n", val, pAcc[lane]);
   }
   
   /* note value is scaled up by 4096, need to divide by accumulation ticks to get average */
   return retCode;
}


/******************************************************************************
 BWFE_Adc_P_CalibrateINL() - integrated non-linearities algorithm (clocks on, RF off)
******************************************************************************/
/* The INL algorithm will calibrate ADC cap mismatch due to process
 1) Accumulate errors for each capswap value for each MDAC(3 to 1) for each lane
 2) e(6x1) = E1(6x1) - E2(6x1) where e6=6 then invert sign and divide by 2^MDAC_shift
      MDAC_shift = 4, 6, 8 for MDAC = 3, 2, 1 respectively
 3) Solve for J(6x1) given A(6x6) and e(6x1) e.g. A(6x6) * J(6x1) = e(6x1) 
      A(6x6) = {
      {2, -2, 0, 0, 0, 0},
      {0, 2, -2, 0, 0, 0},
      {0, 0, 2, -2, 0, 0},
      {0, 0, 0, 2, -2, 0},
      {0, 0, 0, 0, 2, -2},
      {1, 1, 1, 1, 1, 1}}
 4) LUT(7x1) = M(7x6) * J(6x1) e.g. LUT(7x1) = M(7x6) * Inv(A)(6x6) * e(6x1)
      M(7x6) = {
      {-1, -1, -1, -1, -1, -1},
      {1, -1, -1, -1, -1, -1},
      {1, 1, -1, -1, -1, -1},
      {1, 1, 1, -1, -1, -1},
      {1, 1, 1, 1, -1, -1},
      {1, 1, 1, 1, 1, -1},
      {1, 1, 1, 1, 1, 1}}
 5) Convert real LUT values to fixed point integer LUT_int = round(((0.5*LUT) + T) * 2^(MDAC_shift+5))
      T(7x1) = {3, 2, 1, 0 -1, -2, -3}'
*/
BERR_Code BWFE_Adc_P_CalibrateINL(BWFE_ChannelHandle h)
{
   BERR_Code retCode = BERR_SUCCESS;
   int32_t acc, maxCapDiff, errAccumDiff[6], dgsLut[7];
   uint32_t offset;
   uint8_t lane, mdac, err, cap, z, e;
   
   if (!h->bEnabled)
      return BWFE_ERR_POWERED_DOWN;
   
   /* power up caldac */
   BWFE_P_AndRegister(h, BCHP_WFE_ANA_U7_WBADC_TOP_CNTL_IN, ~0x10000200);  /* also set termination resistance to 1K */
   
   /* turn on reference lane */
   BWFE_P_PowerUpRefLane(h);  /* wake ADC2 for 4550, ADC4 for 4528 */
   h->accSaturateCount = 0;
   h->lutSaturateCount = 0;
   
   /* accumulate MDACs for each lane including 2 reference lanes */
   for (lane = 0; lane < h->totalLanes + 2; lane++)
   {
      /* clear digisum LUT table */
      BKNI_Memset((void*)dgsLut, 0, sizeof(dgsLut));
      
      /* calibrate DC offset to zero before accumulation */
      BWFE_P_CalibrateDAC(h, lane, 0);
      
      /* start with MDAC3 first */
      for (mdac = 3; mdac > 0; mdac--)
      {
         /* clear delta values for each MDAC pass */
         BKNI_Memset((void*)errAccumDiff, 0, 6 * 4);
         
         /* enable MDAC gain calibration*/
         BWFE_P_ReadModifyWriteRegister(h, BCHP_WFE_ANA_U7_ADC_CNTL3, ~0x00007000, (1 << (mdac-1)) << 12);
         
         /* accumulate E1x followed by E2x for each row to minimize time-dependent noise */
         for (cap = 0; cap < 5; cap++)
         {
            for (err = 0; err < 2; err++)
            {
               switch (mdac)
               {
                  case 3:
                     BWFE_P_ReadModifyWriteRegister(h, BCHP_WFE_ANA_U7_ADC_CNTL0, ~0x3F000000, (capSwap[cap][err] & 0x3F) << 24);
                     break;
                  case 2:
                     BWFE_P_ReadModifyWriteRegister(h, BCHP_WFE_ANA_U7_ADC_CNTL3, ~0x00000FC0, (capSwap[cap][err] & 0x3F) << 6);
                     break;
                  case 1:
                     BWFE_P_ReadModifyWriteRegister(h, BCHP_WFE_ANA_U7_ADC_CNTL3, ~0x0000003F, (capSwap[cap][err] & 0x3F));
                     break;
               }
               
               retCode = BWFE_Adc_P_ReadAccumulator(h, lane, BWFE_INL_ACC_INTERVAL, &acc);   /* need to div by acc interval for avg, and also scale back down by 4096 */
               //BWFE_DEBUG_INL(BDBG_MSG(("acc=%d(%08X)", acc, acc)));
               if (retCode == BWFE_ERR_ACC_SATURATED)
               {
                  BDBG_MSG(("CalibrateINL: lane%d/mdac%/capswap(%d,%d) accumulator saturated, value clipped!", lane, mdac, cap, err));
               }
               
               /* -e(x) = E2x - E1x */
               errAccumDiff[cap] += err ? acc : -acc;
            }
            
            /* -e(x) / 2^MDAC_shift */
            errAccumDiff[cap] >>= mdacShift[mdac-1];
            
            maxCapDiff = BWFE_INL_MAX_ERR_DIFF * (BWFE_INL_ACC_INTERVAL + 1);    /* scale up by acc interval */
            maxCapDiff <<= 12;   /* scale up by 4096 */
            if ((errAccumDiff[cap] < -maxCapDiff) || (errAccumDiff[cap] > maxCapDiff))
            {
               BDBG_MSG(("CalibrateINL: lane%d/mdac%/cap(%d) difference overflow - %d", lane, mdac, cap, errAccumDiff[cap]));
            }
         }
         
         /* disable MDAC gain calibration*/
         BWFE_P_AndRegister(h, BCHP_WFE_ANA_U7_ADC_CNTL3, ~0x00007000);
         
         /* fixed value e(6) = 6 */
         errAccumDiff[5] = (BWFE_INL_ERR6 << 12) * (BWFE_INL_ACC_INTERVAL + 1);
         
         /* print errAccumDiff for debug */
         BWFE_DEBUG_INL(BDBG_MSG(("e[]={%d %d %d %d %d %d}", errAccumDiff[0], errAccumDiff[1], errAccumDiff[2], errAccumDiff[3], errAccumDiff[4], errAccumDiff[5])));
         
         /* generate LUT(7x1) = M(7x6) * Inv(A)(6x6) * e(6x1) = Z(7x6) * e(6x1) */
         for (z = 0; z < 7; z++)
         {
            for (e = 0; e < 6; e++)
            {
               /* matrix multiplication LUT(7x1) = Z(7x6) * e(6x1), note Z is scaled by 30 */
               dgsLut[z] += zMatrix[z][e] * errAccumDiff[e];
            }
            
            /* add offset compensation and convert LUT value to integer */
            /* LUT_int = round(((0.5*LUT) + T) * 2^(MDAC_shift+5)) */
            offset = 30 * offsetT[z] * (BWFE_INL_ACC_INTERVAL + 1);  /* scale by 30 since added to Z-matrix, also scale by acc interval */
            offset <<= 13;    /* scale offset by 4096, then by 2 to account for 0.5 factor */
            dgsLut[z] += offset;
            dgsLut[z] <<= (mdacShift[mdac-1] + 4);      /* div by 2 */
            dgsLut[z] /= 30 * (BWFE_INL_ACC_INTERVAL + 1);  /* undo Z-matrix scaling */
            dgsLut[z] >>= 12;  /* scale back down by 4096 for average */
            
            /* limit LUT value if saturated*/
            BWFE_DEBUG_INL(BDBG_MSG(("z[%d]=%d", z, dgsLut[z])));
            if ((dgsLut[z] < -128) || (dgsLut[z] > 127))
            {
               BDBG_MSG(("CalibrateINL: lane%d mdac%d lut(%d) saturated, value clipped!", lane, mdac, z));
               dgsLut[z] = (dgsLut[z] > 0) ? 127 : -128;
               h->lutSaturateCount++;
            }
         }
         
         BWFE_DEBUG_INL(BDBG_MSG(("dgsLut[]={%d %d %d %d %d %d %d}", dgsLut[0], dgsLut[1], dgsLut[2], dgsLut[3], dgsLut[4], dgsLut[5], dgsLut[6])));
         
         /* program  LUT values for each lane */
         BWFE_P_OrLaneReg(h, lane, BCHP_WFE_CORE_DGSLUT011_LN, (dgsLut[6] & 0xFF) << (8 * (3-mdac)));
         BWFE_P_OrLaneReg(h, lane, BCHP_WFE_CORE_DGSLUT010_LN, (dgsLut[5] & 0xFF) << (8 * (3-mdac)));
         BWFE_P_OrLaneReg(h, lane, BCHP_WFE_CORE_DGSLUT001_LN, (dgsLut[4] & 0xFF) << (8 * (3-mdac)));
         BWFE_P_OrLaneReg(h, lane, BCHP_WFE_CORE_DGSLUT000_LN, (dgsLut[3] & 0xFF) << (8 * (3-mdac)));
         BWFE_P_OrLaneReg(h, lane, BCHP_WFE_CORE_DGSLUT111_LN, (dgsLut[2] & 0xFF) << (8 * (3-mdac)));
         BWFE_P_OrLaneReg(h, lane, BCHP_WFE_CORE_DGSLUT110_LN, (dgsLut[1] & 0xFF) << (8 * (3-mdac)));
         BWFE_P_OrLaneReg(h, lane, BCHP_WFE_CORE_DGSLUT101_LN, (dgsLut[0] & 0xFF) << (8 * (3-mdac)));
      }
   }
   
   /* power down caldac */
   BWFE_P_OrRegister(h, BCHP_WFE_ANA_U7_WBADC_TOP_CNTL_IN, 0x10000200);    /* revert termination resistance to 10K */
   
   /* turn off reference lane */
   BWFE_P_PowerDownRefLane(h);   /* sleep ADC2 for 4550, ADC4 for 4528 */
   
   return retCode;
}


/******************************************************************************
 BWFE_Adc_P_CalibrateINLv2() - integrated non-linearities algorithm (clocks on, RF off)
******************************************************************************/
BERR_Code BWFE_Adc_P_CalibrateINLv2(BWFE_ChannelHandle h)
{
   BERR_Code retCode = BERR_SUCCESS;
   uint32_t val, dgsLut[7];
   int32_t acc, maxCapDiff, offset, errAccumDiff[6], J[6], mdacLut[7];
   int16_t target = 0;
   uint8_t lane, mdac, slice, err, cap, i;
   
   if (!h->bEnabled)
      return BWFE_ERR_POWERED_DOWN;
   
   /* power up caldac */
   BWFE_P_AndRegister(h, BCHP_WFE_ANA_U7_WBADC_TOP_CNTL_IN, ~0x10000200);  /* also set termination resistance to 1K */
   
   /* turn on reference lane */
   BWFE_P_PowerUpRefLane(h);  /* wake ADC2 for 4550, ADC4 for 4528 */
   h->accSaturateCount = 0;
   h->lutSaturateCount = 0;
   
   /* accumulate MDACs for each lane including 2 reference lanes */
   for (lane = 0; lane < h->totalLanes + 2; lane++)
   {
      /* clear digisum LUT table */
      BKNI_Memset((void*)dgsLut, 0, sizeof(dgsLut));
   
      /* start with MDAC3 first */
      for (mdac = 3; mdac > 0; mdac--)
      {
         /* calibrate DC offset to zero before accumulation */
         BWFE_P_CalibrateDAC(h, lane, 0);
         
         /* clear delta values for each MDAC pass */
         BKNI_Memset((void*)errAccumDiff, 0, 6 * 4);
         
         /* enable MDAC mismatch calibration*/
         BWFE_P_ReadModifyWriteRegister(h, BCHP_WFE_ANA_U7_ADC_CNTL3, ~0x00007000, (1 << (mdac-1)) << 12);
         
         /* accumulate E1x followed by E2x for each row to minimize time-dependent noise */
         for (cap = 0; cap < 5; cap++)
         {
            for (err = 0; err < 2; err++)
            {
               switch (mdac)
               {
                  case 3:
                     BWFE_P_ReadModifyWriteRegister(h, BCHP_WFE_ANA_U7_ADC_CNTL0, ~0x3F000000, (capSwapV2[cap][err] & 0x3F) << 24);
                     break;
                  case 2:
                     BWFE_P_ReadModifyWriteRegister(h, BCHP_WFE_ANA_U7_ADC_CNTL3, ~0x00000FC0, (capSwapV2[cap][err] & 0x3F) << 6);
                     break;
                  case 1:
                     BWFE_P_ReadModifyWriteRegister(h, BCHP_WFE_ANA_U7_ADC_CNTL3, ~0x0000003F, (capSwapV2[cap][err] & 0x3F));
                     break;
               }
               
               retCode = BWFE_Adc_P_ReadAccumulator(h, lane, BWFE_INL_ACC_INTERVAL, &acc);   /* need to div by acc interval for avg, and also scale back down by 4096 */
               //BWFE_DEBUG_INL(BDBG_MSG(("acc=%d(%08X)", acc, acc)));
               if (retCode == BWFE_ERR_ACC_SATURATED)
               {
                  BWFE_DEBUG_INL(BDBG_MSG(("CalibrateINL: lane%d/mdac%/capswap(%d,%d) accumulator saturated, value clipped!", lane, mdac, cap, err)));
               }
               
               /* e(x) = E1x - E2x */
               errAccumDiff[cap] += err ? -acc : acc;
            }
            
            /* e(x) / 2^MDAC_shift */
            errAccumDiff[cap] >>= mdacShiftV2[mdac-1];
            
            maxCapDiff = BWFE_INL_MAX_ERR_DIFF * (BWFE_INL_ACC_INTERVAL + 1);    /* scale up by acc interval */
            maxCapDiff <<= 12;   /* scale up by 4096 */
            if ((errAccumDiff[cap] < -maxCapDiff) || (errAccumDiff[cap] > maxCapDiff))
            {
               BWFE_DEBUG_INL(BDBG_MSG(("CalibrateINL: lane%d/mdac%/cap(%d) difference overflow -> %d", lane, mdac, cap, errAccumDiff[cap])));
            }
         }
         
         /* disable MDAC mismatch calibration*/
         BWFE_P_AndRegister(h, BCHP_WFE_ANA_U7_ADC_CNTL3, ~0x00007000);
         
         /* calibrate DC offset to specified targets before err6 accumulation */
         target = 1 << mdacShift[mdac-1];
         BWFE_P_CalibrateDAC(h, lane, target);
         
         /* determine slice from lane */
         if (lane < BWFE_NUM_LANES)
            slice = lane % (BWFE_NUM_LANES / 2);
         else
            slice = BWFE_NUM_LANES / 2;
         
         /* run equalizer for err6 term */
         BWFE_P_OrRegister(h, BCHP_WFE_ANA_U7_WBADC_CAL_CNTL_IN, 1 << (27 + slice));         /* enable ADCx to MDAC equ mode */
         BWFE_P_OrRegister(h, BCHP_WFE_ANA_U7_WBADC_CAL_CNTL_IN, (0x1 << ((mdac-1) * 5)));   /* set cap3 low for E1 */
         
         BWFE_P_ReadRegister(h, BCHP_WFE_ANA_U7_WBADC_CAL_CNTL_IN, &val);
         
         retCode = BWFE_Adc_P_ReadAccumulator(h, lane, BWFE_INL_ACC_INTERVAL, &acc);   /* need to div by acc interval for avg, and also scale back down by 4096 */
         errAccumDiff[5] = (1 << mdacShiftV2[mdac-1]) * (BWFE_INL_ACC_INTERVAL + 1);   /* scale up by acc interval */
         errAccumDiff[5] <<= 12;    /* scale offset by 4096 */
         //BDBG_MSG(("E6_1=%d", acc));
         if (mdac == 1)
            errAccumDiff[5] += acc;
         else
            errAccumDiff[5] -= acc;
         
         BWFE_P_AndRegister(h, BCHP_WFE_ANA_U7_WBADC_CAL_CNTL_IN, ~(0x1F << ((mdac-1) * 5)));   /* disable cap3 */
         BWFE_P_OrRegister(h, BCHP_WFE_ANA_U7_WBADC_CAL_CNTL_IN, (0x5 << ((mdac-1) * 5)));      /* set cap3 high for E2 */
         retCode = BWFE_Adc_P_ReadAccumulator(h, lane, BWFE_INL_ACC_INTERVAL, &acc);   /* need to div by acc interval for avg, and also scale back down by 4096 */
         //BDBG_MSG(("E6_2=%d", acc));
         if (mdac == 1)
            errAccumDiff[5] -= acc;
         else
            errAccumDiff[5] += acc;
            
         BWFE_P_AndRegister(h, BCHP_WFE_ANA_U7_WBADC_CAL_CNTL_IN, ~(0x1F << ((mdac-1) * 5)));   /* disable cap3 */
         BWFE_P_AndRegister(h, BCHP_WFE_ANA_U7_WBADC_CAL_CNTL_IN, ~(1 << (27 + slice)));        /* disable ADCx for MDAC equ mode */
         
         /* e(6) / 2^MDAC_shift */
         errAccumDiff[5] >>= mdacShiftV2[mdac-1];
         
         /* print errAccumDiff for debug */
         BWFE_DEBUG_INL(BDBG_MSG(("e[]={%d %d %d %d %d %d}", errAccumDiff[0], errAccumDiff[1], errAccumDiff[2], errAccumDiff[3], errAccumDiff[4], errAccumDiff[5])));
         
         /* calculate J(6x1) vector */
         J[3] = errAccumDiff[5];
         J[2] = J[3] - errAccumDiff[2];
         J[1] = J[2] - errAccumDiff[3];
         J[0] = J[1] - errAccumDiff[4];
         J[4] = J[3] + errAccumDiff[1];
         J[5] = J[4] + errAccumDiff[0];
         
         BWFE_DEBUG_INL(BDBG_MSG(("J[]={%d %d %d %d %d %d}", J[0], J[1], J[2], J[3], J[4], J[5])));
         
         /* calculate LUTs for each mdac */
         mdacLut[6] = J[0] + J[1] + J[2] + J[3] + J[4] + J[5];
         mdacLut[5] = mdacLut[6] - (J[5] << 1);
         mdacLut[4] = mdacLut[5] - (J[4] << 1);
         mdacLut[3] = mdacLut[4] - (J[3] << 1);
         mdacLut[2] = mdacLut[3] - (J[2] << 1);
         mdacLut[1] = mdacLut[2] - (J[1] << 1);
         mdacLut[0] = -mdacLut[6];
         
         BWFE_DEBUG_INL(BDBG_MSG(("mdacLut[]={%d %d %d %d %d %d %d}", mdacLut[0], mdacLut[1], mdacLut[2], mdacLut[3], mdacLut[4], mdacLut[5], mdacLut[6])));
         
         for (i = 0; i < 7; i++)
         {
            offset = offsetT[i] * (BWFE_INL_ACC_INTERVAL + 1);  /* scale offset by acc interval */
            offset <<= 13;    /* scale offset by 4096, then by 2 to account for 0.5 factor */
            mdacLut[i] += offset;
            mdacLut[i] <<= (mdacShiftV2[mdac-1] + 3);    /* div by 2 */
            mdacLut[i] /= (BWFE_INL_ACC_INTERVAL + 1);   /* div by acc interval for avg */
            mdacLut[i] >>= 12;  /* also scale back down by 4096 */
            
            /* limit LUT value if saturated*/
            BWFE_DEBUG_INL(BDBG_MSG(("mdacLut[%d]=%d", i, mdacLut[i])));
            if ((mdacLut[i] < -128) || (mdacLut[i] > 127))
            {
               BWFE_DEBUG_INL(BDBG_MSG(("CalibrateINL: lane%d mdac%d lut(%d) saturated, value clipped!", lane, mdac, i)));
               mdacLut[i] = (mdacLut[i] > 0) ? 127 : -128;
               h->lutSaturateCount++;
            }
         }
         
         BWFE_DEBUG_INL(BDBG_MSG(("mdacLut[]={%d %d %d %d %d %d %d}", mdacLut[0], mdacLut[1], mdacLut[2], mdacLut[3], mdacLut[4], mdacLut[5], mdacLut[6])));
         
         /* generate LUT values for each lane but do not program LUT yet */
         dgsLut[0] |= (mdacLut[0] & 0xFF) << (8 * (3-mdac));
         dgsLut[1] |= (mdacLut[1] & 0xFF) << (8 * (3-mdac));
         dgsLut[2] |= (mdacLut[2] & 0xFF) << (8 * (3-mdac));
         dgsLut[3] |= (mdacLut[3] & 0xFF) << (8 * (3-mdac));
         dgsLut[4] |= (mdacLut[4] & 0xFF) << (8 * (3-mdac));
         dgsLut[5] |= (mdacLut[5] & 0xFF) << (8 * (3-mdac));
         dgsLut[6] |= (mdacLut[6] & 0xFF) << (8 * (3-mdac));
      }
      
      BWFE_DEBUG_INL(BDBG_MSG(("dgsLut[]={%d %d %d %d %d %d %d}", dgsLut[0], dgsLut[1], dgsLut[2], dgsLut[3], dgsLut[4], dgsLut[5], dgsLut[6])));
      
      /* program  LUT values for each lane */
      BWFE_P_WriteLaneRegister(h, lane, BCHP_WFE_CORE_DGSLUT011_LN, dgsLut[6]);
      BWFE_P_WriteLaneRegister(h, lane, BCHP_WFE_CORE_DGSLUT010_LN, dgsLut[5]);
      BWFE_P_WriteLaneRegister(h, lane, BCHP_WFE_CORE_DGSLUT001_LN, dgsLut[4]);
      BWFE_P_WriteLaneRegister(h, lane, BCHP_WFE_CORE_DGSLUT000_LN, dgsLut[3]);
      BWFE_P_WriteLaneRegister(h, lane, BCHP_WFE_CORE_DGSLUT111_LN, dgsLut[2]);
      BWFE_P_WriteLaneRegister(h, lane, BCHP_WFE_CORE_DGSLUT110_LN, dgsLut[1]);
      BWFE_P_WriteLaneRegister(h, lane, BCHP_WFE_CORE_DGSLUT101_LN, dgsLut[0]);
   }
   
   /* power down caldac */
   BWFE_P_OrRegister(h, BCHP_WFE_ANA_U7_WBADC_TOP_CNTL_IN, 0x10000200);    /* revert termination resistance to 10K */
   
   /* turn off reference lane */
   BWFE_P_PowerDownRefLane(h);   /* sleep ADC2 for 4550, ADC4 for 4528 */
   
   return retCode;
}


/******************************************************************************
 BWFE_Adc_P_EqualizePipeline()
******************************************************************************/
BERR_Code BWFE_Adc_P_EqualizePipeline(BWFE_ChannelHandle h)
{
   BERR_Code retCode = BERR_SUCCESS;
   uint8_t slice, mdac, laneA, laneB, tap;
   uint32_t mask, val;
   int16_t offset = 0, target = 0;
   
   if (!h->bEnabled)
      return BWFE_ERR_POWERED_DOWN;
   
   /* power up caldac */
   BWFE_P_AndRegister(h, BCHP_WFE_ANA_U7_WBADC_TOP_CNTL_IN, ~0x10000200);  /* also set termination resistance to 1K */
   
   /* turn on reference lane */
   BWFE_P_PowerUpRefLane(h);  /* wake ADC2 for 4550, ADC4 for 4528 */
   
   /* select ADC slice */
   for (slice = 0; slice < BWFE_NUM_SLICES; slice++)
   {
      /* enable ADCx to MDAC equ mode */
      BWFE_DEBUG_EQU(BDBG_MSG((" slice%d", slice)));
      BWFE_P_OrRegister(h, BCHP_WFE_ANA_U7_WBADC_CAL_CNTL_IN, 1 << (27 + slice));
      
      /* select MDAC */
      for (mdac = 3; mdac > 0; mdac--)
      {
         if (slice == BWFE_NUM_SLICES - 1)
         {
            /* use reference lanes for reference slice */
            laneA = BWFE_REF_LANE0;
            laneB = BWFE_REF_LANE1;
         }
         else
         {
            /* determine lane pairing for current slice */
            laneA = slice;
            laneB = (BWFE_NUM_LANES / 2) + slice;
         }
         
         /* calibrate DAC to hit ADC target output for lane (applies to lane pair) */
         target = 1 << mdacShift[mdac-1];
         BWFE_P_CalibrateDAC(h, laneA, offset + target);
         
         /* enable PN generator for adaptation on MDACx */
         BWFE_P_AndRegister(h, BCHP_WFE_ANA_U7_WBADC_CAL_CNTL_IN, ~(0x1F << ((mdac - 1) * 5)));
         BWFE_P_OrRegister(h, BCHP_WFE_ANA_U7_WBADC_CAL_CNTL_IN, (0x3 << ((mdac - 1) * 5)));
         
         /* enable PN input for LMS and update coefficents */
         mask = 0x78 << (8 * (3 - mdac));
         BWFE_P_OrRegister(h, BCHP_WFE_CORE_DGSLMS_PAIR0 + (4 * slice), mask);
         
         /* run for 1ms default */
         BKNI_Sleep(h->equRuntimeMs);
         
         /* disable PN input and stop update */
         BWFE_P_AndRegister(h, BCHP_WFE_ANA_U7_WBADC_CAL_CNTL_IN, ~(0x1F << ((mdac - 1) * 5)));
         BWFE_P_AndRegister(h, BCHP_WFE_CORE_DGSLMS_PAIR0 + (4 * slice), ~mask);
         
         /* read equalizer coefficients */
         BWFE_DEBUG_EQU(BDBG_MSG(("  mdac%d", mdac)));
         BWFE_P_WriteRegister(h, BCHP_WFE_CORE_DGSCOEFA_PAIR0 + (8 * slice), (mdac - 1) * 3);   /* auto-incrementing address */
         for (tap = 0; tap < BWFE_NUM_EQ_TAPS; tap++)
         {
            BWFE_P_ReadRegister(h, BCHP_WFE_CORE_DGSCOEFD_PAIR0 + (8 * slice), &val);
            BWFE_DEBUG_EQU(BDBG_MSG(("   tap%d=%08X", tap, val)));
            h->equCoeff[slice][mdac-1][tap] = val;  /* 3D array of eq taps (slices x mdacs x taps) */
         }
      }
      
      /* disable ADCx for MDAC equ mode */
      BWFE_P_AndRegister(h, BCHP_WFE_ANA_U7_WBADC_CAL_CNTL_IN, ~(1 << (27 + slice)));
   }
   
   /* power down caldac */
   BWFE_P_OrRegister(h, BCHP_WFE_ANA_U7_WBADC_TOP_CNTL_IN, 0x10000200);    /* revert termination resistance to 10K */
   
   /* turn off reference lane */
   BWFE_P_PowerDownRefLane(h);   /* sleep ADC2 for 4550, ADC4 for 4528 */
   
   return retCode;
}


/******************************************************************************
 BWFE_Adc_P_ResetLut()
******************************************************************************/
BERR_Code BWFE_Adc_P_ResetLut(BWFE_ChannelHandle h)
{
   uint8_t i;
   
   /* reset lut for all lanes including 2 reference lanes */
   for (i = 0; i < h->totalLanes + 2; i++)
   {
      /* clear LUT values for each lane */
      BWFE_P_WriteLaneRegister(h, i, BCHP_WFE_CORE_DGSLUT011_LN, 0);
      BWFE_P_WriteLaneRegister(h, i, BCHP_WFE_CORE_DGSLUT010_LN, 0);
      BWFE_P_WriteLaneRegister(h, i, BCHP_WFE_CORE_DGSLUT001_LN, 0);
      BWFE_P_WriteLaneRegister(h, i, BCHP_WFE_CORE_DGSLUT000_LN, 0);
      BWFE_P_WriteLaneRegister(h, i, BCHP_WFE_CORE_DGSLUT111_LN, 0);
      BWFE_P_WriteLaneRegister(h, i, BCHP_WFE_CORE_DGSLUT110_LN, 0);
      BWFE_P_WriteLaneRegister(h, i, BCHP_WFE_CORE_DGSLUT101_LN, 0);
   }
   
   return BERR_SUCCESS;
}

/******************************************************************************
 BWFE_Adc_P_ResetEqTaps()
******************************************************************************/
BERR_Code BWFE_Adc_P_ResetEqTaps(BWFE_ChannelHandle h)
{
   BERR_Code retCode = BERR_SUCCESS;
   uint8_t slice, mdac, tap, mu;
   uint32_t mask, val;
 
   for (slice = 0; slice < BWFE_NUM_SLICES; slice++)
   {
      /* reset each LMS pair */
      BWFE_P_ToggleBit(h, BCHP_WFE_CORE_DGSLMS_PAIR0 + (4 * slice), 0x00808080);
      
      for (mdac = 3; mdac > 0; mdac--)
      {
         /* set mu=4 e.g. 2^-9 */
         mask = 0x07 << (8 * (3 - mdac));
         BWFE_P_AndRegister(h, BCHP_WFE_CORE_DGSLMS_PAIR0 + (4 * slice), ~mask);
         if (mdac == 1)
            mu = h->dgsMu1;
         else if (mdac == 2)
            mu = h->dgsMu2;
         else
            mu = h->dgsMu3;
         mask = mu << (8 * (3 - mdac));
         BWFE_P_OrRegister(h, BCHP_WFE_CORE_DGSLMS_PAIR0 + (4 * slice), mask);
         
         /* read default equalizer coefficients */
         BWFE_P_WriteRegister(h, BCHP_WFE_CORE_DGSCOEFA_PAIR0 + (8 * slice), (mdac - 1) * 3);   /* auto-incrementing address */
         for (tap = 0; tap < BWFE_NUM_EQ_TAPS; tap++)
         {
            BWFE_P_ReadRegister(h, BCHP_WFE_CORE_DGSCOEFD_PAIR0 + (8 * slice), &val);
            h->equCoeff[slice][mdac-1][tap] = val;
         }
      }
   }
   
   return retCode;
}


/******************************************************************************
 BWFE_Adc_P_UpdateEqTaps()
******************************************************************************/
BERR_Code BWFE_Adc_P_UpdateEqTaps(BWFE_ChannelHandle h)
{
   BERR_Code retCode = BERR_SUCCESS;
   uint8_t slice, mdac, tap;
   uint32_t val;
 
   for (slice = 0; slice < BWFE_NUM_SLICES; slice++)
   {
      for (mdac = 3; mdac > 0; mdac--)
      { 
         /* read default equalizer coefficients */
         BWFE_P_WriteRegister(h, BCHP_WFE_CORE_DGSCOEFA_PAIR0 + (8 * slice), (mdac - 1) * 3);   /* auto-incrementing address */
         for (tap = 0; tap < BWFE_NUM_EQ_TAPS; tap++)
         {
            BWFE_P_ReadRegister(h, BCHP_WFE_CORE_DGSCOEFD_PAIR0 + (8 * slice), &val);
            h->equCoeff[slice][mdac-1][tap] = val;
         }
      }
   }
   
   return retCode;
}


/******************************************************************************
 BWFE_Adc_P_CancelDCOffset()
******************************************************************************/
#ifdef BWFE_USE_ORIG_DCO_CANCEL
BERR_Code BWFE_Adc_P_CancelDCOffset(BWFE_ChannelHandle h)
{
   BERR_Code retCode = BERR_SUCCESS;
   uint8_t i, lane, mdac;
   int32_t acc, accum[BWFE_NUM_LANES], offset[BWFE_NUM_LANES], average = 0;
   
   /* reset dgs lookup table */
   BWFE_Adc_P_ResetLut(h);
   
   BDBG_MSG(("Before..."));
   for (lane = 0; lane < BWFE_NUM_LANES; lane++)
   {
      accum[lane] = 0;
      
      /* read accumulator */
      for (i = 0; i < BWFE_DCO_ACC_COUNT; i++)
      {
         /* set maximum accumulation length: 4096 samples per unit */
         BWFE_Adc_P_ReadAccumulator(h, lane, BWFE_DCO_ACC_INTERVAL, &acc);
         accum[lane] += acc;
      }
      
      /* calculate average for lane */
      accum[lane] /= BWFE_DCO_ACC_COUNT;
      average += accum[lane];
      BDBG_MSG(("LANE%d: accum=%d(%08X)", lane, accum[lane], accum[lane]));
   }
   
   /* average across all lanes */
   average /= BWFE_NUM_LANES;
   BDBG_MSG(("average=%d(%08X)\n", average, average));
   
   /* start with MDAC3 first */
   for (mdac = 3; mdac > 0; mdac--)
   {
      BDBG_MSG(("MDAC%d", mdac));
      
      /* calculate offset from average */
      for (lane = 0; lane < BWFE_NUM_LANES; lane++)
      {
         /* read accumulator */
         accum[lane] = 0;
         for (i = 0; i < BWFE_DCO_ACC_COUNT; i++)
         {
            /* set maximum accumulation length: 4096 samples per unit */
            BWFE_Adc_P_ReadAccumulator(h, lane, BWFE_DCO_ACC_INTERVAL, &acc);
            accum[lane] += acc;
         }
         accum[lane] /= BWFE_DCO_ACC_COUNT; /* calculate average for lane */
         
         /* update offset */
         offset[lane] = (average - accum[lane]);
         BDBG_MSG(("LANE%d: accum=%d(%08X), offset=%d(%08X)", lane, accum[lane], accum[lane], offset[lane], offset[lane]));
         offset[lane] >>= 16;    /* div by acc length to get avg, scale back down by 4096, scale up by 16 to account for fractional bits in LUT (8 + 12 - 4) */
         BDBG_MSG(("        offset>>16=%d(%08X)", offset[lane]));
         
         /* program  LUT values for each lane */
         BWFE_P_WriteLaneRegister(h, lane, BCHP_WFE_CORE_DGSLUT011_LN, (offset[lane] & 0xFF) << (8 * (3-mdac)));
         BWFE_P_WriteLaneRegister(h, lane, BCHP_WFE_CORE_DGSLUT010_LN, (offset[lane] & 0xFF) << (8 * (3-mdac)));
         BWFE_P_WriteLaneRegister(h, lane, BCHP_WFE_CORE_DGSLUT001_LN, (offset[lane] & 0xFF) << (8 * (3-mdac)));
         BWFE_P_WriteLaneRegister(h, lane, BCHP_WFE_CORE_DGSLUT000_LN, (offset[lane] & 0xFF) << (8 * (3-mdac)));
         BWFE_P_WriteLaneRegister(h, lane, BCHP_WFE_CORE_DGSLUT111_LN, (offset[lane] & 0xFF) << (8 * (3-mdac)));
         BWFE_P_WriteLaneRegister(h, lane, BCHP_WFE_CORE_DGSLUT110_LN, (offset[lane] & 0xFF) << (8 * (3-mdac)));
         BWFE_P_WriteLaneRegister(h, lane, BCHP_WFE_CORE_DGSLUT101_LN, (offset[lane] & 0xFF) << (8 * (3-mdac)));
      }
   }
   
   /* check if accumulator is same after */
   BDBG_MSG(("\nAfter..."));
   for (lane = 0; lane < BWFE_NUM_LANES; lane++)
   {
      accum[lane] = 0;
      
      /* read accumulator */
      for (i = 0; i < BWFE_DCO_ACC_COUNT; i++)
      {
         /* set maximum accumulation length: 4096 samples per unit */
         BWFE_Adc_P_ReadAccumulator(h, lane, BWFE_DCO_ACC_INTERVAL, &acc);
         accum[lane] += acc;
      }
      
      /* calculate average for lane */
      accum[lane] /= BWFE_DCO_ACC_COUNT;
      BDBG_MSG(("LANE%d: accum=%d(%08X)", lane, accum[lane], accum[lane]));
   }
   
   return retCode;
}
#else
BERR_Code BWFE_Adc_P_CancelDCOffset(BWFE_ChannelHandle h)
{
   BERR_Code retCode = BERR_SUCCESS;
   int32_t acc[BWFE_NUM_LANES];
   uint8_t i, lane;
   
   for (i = 0; i < h->totalLanes / 2; i++)
   {
      /* set dco level for lane pairs */
      BWFE_P_WriteRegister(h, BCHP_WFE_CORE_LICDCO0 + (4 * i), 0x0FC00FC0);
   }
   
   BWFE_Adc_P_ReadAccumulatorAllLanes(h, BWFE_DCO_ACC_INTERVAL, acc);
   
   for (lane = 0; lane < BWFE_NUM_LANES; lane++)
   {
      /* lane selection */
      BWFE_P_ReadModifyWriteRegister(h, BCHP_WFE_CORE_LICCTL1, ~0x0000FFFF, 1 << lane);
      
      /* write lic dco tap */
      BWFE_P_WriteRegister(h, BCHP_WFE_CORE_LICCOEFA, 16);
      BWFE_P_WriteRegister(h, BCHP_WFE_CORE_LICCOEFD, acc[lane]);
   }
   
   return retCode;
}
#endif
