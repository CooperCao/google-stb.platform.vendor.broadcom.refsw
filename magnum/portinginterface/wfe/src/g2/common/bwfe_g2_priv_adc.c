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

BDBG_MODULE(bwfe_g2_priv_adc);


#define BWFE_ACC_INTERVAL 255   /* interval is 0 to 255, 4096 samples per step */
#define BWFE_DAC_SEARCH_THRESH 2*4096*(BWFE_ACC_INTERVAL + 1)

#define BWFE_DEBUG_INL(x) /* x */
#define BWFE_DEBUG_EQU(x) /* x */
#define BWFE_DEBUG_DAC(x) /* x */
#define BWFE_DEBUG_PHASE_CAL(x) /* x */


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
 BWFE_g2_Adc_P_IsAdcOn()
******************************************************************************/
bool BWFE_g2_Adc_P_IsAdcOn(BWFE_ChannelHandle h)
{
   uint32_t val;

   BWFE_P_ReadRegister(h, BCHP_WFE_ANA_WBADC_SYS_CNTL_IN, &val);
   if (val & 0x1)
      h->bEnabled = true;
   else
      h->bEnabled = false;

   return h->bEnabled;
}


/******************************************************************************
 BWFE_g2_Adc_P_PowerUp()
******************************************************************************/
BERR_Code BWFE_g2_Adc_P_PowerUp(BWFE_ChannelHandle h)
{
   BWFE_g2_P_ChannelHandle *hChn = (BWFE_g2_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode = BERR_SUCCESS;
   uint8_t i;

   BDBG_MSG(("ADC%d pwrup", h->channel));
   h->bEnabled = true;

   /* chip specific adc intialization */
   BWFE_P_InitAdc(h);

   /* same phase for output clock */
   BWFE_P_AndRegister(h, BCHP_WFE_ANA_ADC_CNTL0, ~0x00000001);

   /* reset lut and eq taps */
   BWFE_g2_Adc_P_ResetLutAll(h);
   BWFE_g2_Adc_P_ResetEquTaps(h);

   /* enable RFAGC after ADC power up */
   if (!h->bReference)
   {
      BWFE_P_OrRegister(h, BCHP_WFE_CORE_AGCCTL2, 0x00000001);    /* enable PN sequence */
      BWFE_P_ToggleBit(h, BCHP_WFE_CORE_AGCCTL1, 0x00010000);     /* toggle agc sync reset */
      BWFE_P_ReadModifyWriteRegister(h, BCHP_WFE_CORE_AGCCTL1, ~0x01FCFFFF, (hChn->rfAgcBeta << 22) | (hChn->rfAgcK1 << 18) | hChn->rfAgcThresh);
      BWFE_P_AndRegister(h, BCHP_WFE_CORE_AGCCTL1, ~0x04020000);  /* unfreeze agc */
   }

   /* enable digisum2 for ping and pong lanes over all slices */
   for (i = 0; i < h->totalSlices; i++)
   {
      BWFE_P_OrSliceReg(h, i, BCHP_WFE_CORE_DGSCTL_PI_SLC, 0x00200000);
      BWFE_P_OrSliceReg(h, i, BCHP_WFE_CORE_DGSCTL_PO_SLC, 0x00200000);
   }

#ifdef BWFE_BYPASS_DGS
   /* bypass dgs lanes for bringup */
   BWFE_P_OrRegister(h, BCHP_WFE_ANA_ADC_CNTL0, 0x00800100);   /* legacy 10-bit summed mode for bypass */

   for (i = 0; i < h->totalSlices; i++)
   {
      BDBG_MSG(("bypass digisum2 for slice%d", i));
      BWFE_P_OrSliceReg(h, i, BCHP_WFE_CORE_DGSCTL_PI_SLC, 0x00100000);   /* bypass digisum2 for ping lane */
      BWFE_P_OrSliceReg(h, i, BCHP_WFE_CORE_DGSCTL_PO_SLC, 0x00100000);   /* bypass digisum2 for pong lane */
   }
#endif

   return retCode;
}


/******************************************************************************
 BWFE_g2_Adc_P_PowerDown()
******************************************************************************/
BERR_Code BWFE_g2_Adc_P_PowerDown(BWFE_ChannelHandle h)
{
   BERR_Code retCode = BERR_SUCCESS;

   BDBG_MSG(("ADC%d pwrdown", h->channel));
   h->bEnabled = false;

   /* freeze RFAGC before ADC power down */
   BWFE_P_OrRegister(h, BCHP_WFE_CORE_AGCCTL1, 0x04020000);

   /* power down ADC */
   retCode = BWFE_P_ShutdownAdc(h);

   return retCode;
}


#ifdef BWFE_USE_ADC_PHASE_CAL_V2
#include "bchp_stb_chan_ctrl.h"
#include "bchp_stb_chan_ch0.h"
/******************************************************************************
 BWFE_g2_Adc_P_FindUnusedChan()
******************************************************************************/
BERR_Code BWFE_g2_Adc_P_FindUnusedChan(BWFE_ChannelHandle h, uint8_t *unusedChan)
{
   uint32_t val;
   uint8_t i;

   *unusedChan = 0xFF;

   /* turn on the register clock for channelizers */
   BWFE_P_OrRegister(h, BCHP_TM_REG_CLK_EN, BCHP_TM_REG_CLK_EN_CHAN_MASK);

   /* find first unused channelizer */
   BWFE_P_ReadRegister(h, BCHP_STB_CHAN_CTRL_PWRDN, &val);
   for (i = 0; i < 8; i++)
   {
      if ((val >> i) & 1)
      {
         *unusedChan = i;
         return BERR_SUCCESS;
      }
   }
   return BERR_NOT_AVAILABLE;
}


/******************************************************************************
 BWFE_g2_Adc_P_ConfigChan()
******************************************************************************/
BERR_Code BWFE_g2_Adc_P_ConfigChan(BWFE_ChannelHandle h, uint8_t chan)
{
   uint32_t val;
   uint8_t shift, i;

   /* default aci coeff for 1MHz bw */
   static const int16_t aciCoeff[36] =
   {
      -10, -13, -20, -30, -42, -56, -72, -90, -108, -125, -141, -153,
      -160, -159, -150, -130, -97, -50, 13, 92, 186, 297, 422, 560,
      707, 862, 1020, 1177, 1330, 1473, 1603, 1715, 1807, 1874, 1916, 1929
   };

   /* select current adc from chan xbar */
   shift = chan << 1;   /* chan x 2 */
   BWFE_P_ReadModifyWriteRegister(h, BCHP_STB_CHAN_CTRL_XBAR_SEL, ~(0x3 << shift), h->channel << shift);

   /* program aci coeff */
   for (i = 0; i < 36; i++)
   {
      val = (uint32_t)aciCoeff[i];
      BWFE_P_WriteRegister(h, BCHP_STB_CHAN_CH0_ACI_H0 + (i*4) + chan*0x100, val);
   }

   /* disable notch0 */
   BWFE_P_WriteRegister(h, BCHP_STB_CHAN_CH0_NOTCH_0_CTRL + chan*0x100, 0x30000000);
   BWFE_P_WriteRegister(h, BCHP_STB_CHAN_CH0_NOTCH_0_FCW + chan*0x100, 0);
   BWFE_P_WriteRegister(h, BCHP_STB_CHAN_CH0_NOTCH_0_INT_I + chan*0x100, 0);
   BWFE_P_WriteRegister(h, BCHP_STB_CHAN_CH0_NOTCH_0_INT_Q + chan*0x100, 0);
   BWFE_P_WriteRegister(h, BCHP_STB_CHAN_CH0_NOTCH_0_INT_LF + chan*0x100, 0);

   /* disable notch1 */
   BWFE_P_WriteRegister(h, BCHP_STB_CHAN_CH0_NOTCH_1_CTRL + chan*0x100, 0x30000000);
   BWFE_P_WriteRegister(h, BCHP_STB_CHAN_CH0_NOTCH_1_FCW + chan*0x100, 0);
   BWFE_P_WriteRegister(h, BCHP_STB_CHAN_CH0_NOTCH_1_INT_I + chan*0x100, 0);
   BWFE_P_WriteRegister(h, BCHP_STB_CHAN_CH0_NOTCH_1_INT_Q + chan*0x100, 0);
   BWFE_P_WriteRegister(h, BCHP_STB_CHAN_CH0_NOTCH_1_INT_LF + chan*0x100, 0);

   /* enable agc, bypass la, set k1=2^-5, thr=0x2822 */
   BWFE_P_WriteRegister(h, BCHP_STB_CHAN_CH0_AGC_CTRL + chan*0x100, 0x04502822);

   return BERR_SUCCESS;
}


/******************************************************************************
 BWFE_g2_Adc_P_ChanIntToGain() - returns unsigned gain scaled as 9.19
******************************************************************************/
uint32_t BWFE_g2_Adc_P_ChanIntToGain(BWFE_ChannelHandle h, uint32_t chanInt)
{
   /* chan agc format 9.19 */
   chanInt &= 0x0FFFFFFF;

   /* invert msb to convert 2's comp to offset binary */
   return chanInt ^ 0x08000000;
}


/******************************************************************************
 BWFE_g2_Adc_P_CalibratePhaseV2()
******************************************************************************/
#define BWFE_PHASE_PWR_SCALE  524288   /* 2^19 */
#include "bchp_leap_ctrl.h"
BERR_Code BWFE_g2_Adc_P_CalibratePhaseV2(BWFE_ChannelHandle h, uint8_t chan)
{
#if (BCHP_CHIP==4538)
   BWFE_g2_P_ChannelHandle *hChn = (BWFE_g2_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode = BERR_SUCCESS;

   /* DPM freq at 1498.5 MHz */
   /* FCW for corresponding images at 1498500000, -256500000, 985500000, 2227500000 Hz */
   static const uint32_t fcw[4] = {0x4D37A6F5, 0xF2C8590B, 0x32C8590B, 0x72C8590B};

   uint32_t val, chanGain, P_hi, P_lo, imgPower;
   int32_t mainPower, imgRatio;
   int32_t maxMainPower, maxImgRatio;
   int32_t maxImgRatio2, maxImgRatio3;

   static const uint8_t p0[8] = {0, 7, 6, 1, 2, 3, 4, 5};
   static const uint8_t p1[8] = {1, 0, 7, 2, 3, 4, 5, 6};
   static const uint8_t p2[8] = {2, 1, 0, 3, 4, 5, 6, 7};
   static const uint8_t p3[5] = {3, 2, 1, 4, 5};   /* fixed p3=3 */

   uint8_t phase[4] = {0, 1, 2, 3}; /* default phase */
   uint8_t i, j, k, img;

   /* initialize main power and cumulative image ratio */
   mainPower = imgRatio = 0;
   maxMainPower = maxImgRatio = 0x80000000;  /* initialize to minimum */
   maxImgRatio2 = maxImgRatio3 = 0x80000000;

#if 1
   BREG_Write32(h->pDevice->hRegister, BCHP_LEAP_CTRL_GP20, 0);   /* max img ratio */
   BREG_Write32(h->pDevice->hRegister, BCHP_LEAP_CTRL_GP21, 0);   /* max phase */
   BREG_Write32(h->pDevice->hRegister, BCHP_LEAP_CTRL_GP22, 0);   /* 2nd img ratio */
   BREG_Write32(h->pDevice->hRegister, BCHP_LEAP_CTRL_GP23, 0);   /* 2nd phase */
   BREG_Write32(h->pDevice->hRegister, BCHP_LEAP_CTRL_GP24, 0);   /* 3rd img ratio */
   BREG_Write32(h->pDevice->hRegister, BCHP_LEAP_CTRL_GP25, 0);   /* 3rd phase */
#endif

   /* reset analog delays to nominal */
   BWFE_P_WriteRegister(h, BCHP_WFE_ANA_WBADC_TIMING_CNTL_IN, 0x28282828); /* coarse delays */
   BWFE_P_WriteRegister(h, BCHP_WFE_ANA_WBADC_TIMING_CNTL1_IN, 0);         /* fine delays */
   BWFE_P_ToggleBit(h, BCHP_WFE_ANA_WBADC_TIMING_CNTL_IN, 0x80808080);  /* latch in delays */
   BWFE_P_ToggleBit(h, BCHP_WFE_ANA_WBADC_TIMING_CNTL1_IN, 0x80808080); /* latch in fine delays */

   /* power up target channelizer */
   BWFE_P_AndRegister(h, BCHP_STB_CHAN_CTRL_PWRDN, ~(1 << chan));

   /* disable channelizer notches */
   BWFE_P_WriteRegister(h, BCHP_STB_CHAN_CH0_NOTCH_0_FCW + chan*0x100, 0x00000000);
   BWFE_P_WriteRegister(h, BCHP_STB_CHAN_CH0_NOTCH_1_FCW + chan*0x100, 0x00000000);
   BWFE_P_WriteRegister(h, BCHP_STB_CHAN_CH0_NOTCH_0_CTRL + chan*0x100, 0x30000000);
   BWFE_P_WriteRegister(h, BCHP_STB_CHAN_CH0_NOTCH_1_CTRL + chan*0x100, 0x30000000);

   /* search through p0, p1, p2, leave p3=3 fixed */
   for (i = 0; i < 8; i++)
   {
      for (j = 0; j < 8; j++)
      {
         for (k = 0; k < 8; k++)
         {
            val = (p3[0] << 28) | (p2[k] << 25) | (p1[j] << 22) | (p0[i] << 19);
            BWFE_P_ReadModifyWriteRegister(h, BCHP_WFE_ANA_WBADC_SYS_CNTL_IN, ~0x7FF80000, val);
            BWFE_DEBUG_PHASE_CAL(BDBG_MSG((">>>p%d/%d/%d/%d", p0[i], p1[j], p2[k], p3[0])));

            /* toggle dgs reset */
            BWFE_P_AndRegister(h, BCHP_WFE_ANA_WBADC_SYS_CNTL_IN, ~0x00040000);
            BWFE_P_OrRegister(h, BCHP_WFE_ANA_WBADC_SYS_CNTL_IN, 0x00040000);

            /* toggle STB_CHAN_CTRL_XBAR_CTRL.IFIFOS_ON */
            BWFE_P_AndRegister(h, BCHP_STB_CHAN_CTRL_XBAR_CTRL, ~(1<<h->channel));
            BWFE_P_OrRegister(h, BCHP_STB_CHAN_CTRL_XBAR_CTRL, 1<<h->channel);

            /* reset accumulated image power and image ratio */
            imgPower = 0;
            imgRatio = 0x80000000;

            for (img = 0; img < BWFE_LIC_L/4; img++)
            {
               /* tune to image frequency */
               BWFE_P_WriteRegister(h, BCHP_STB_CHAN_CH0_DEC_FCW + chan*0x100, fcw[img]);

               if (img == 0)  /* main image */
                  BWFE_P_WriteRegister(h, BCHP_STB_CHAN_CH0_AGC_LF_INT + chan*0x100, 0x08400000); /* initial agc */
               else
                  BWFE_P_WriteRegister(h, BCHP_STB_CHAN_CH0_AGC_LF_INT + chan*0x100, 0x02000000); /* initial agc */

               /* wait for chan agc to settle */
               BKNI_Sleep(2);

               BWFE_P_ReadRegister(h, BCHP_STB_CHAN_CH0_AGC_LF_INT + chan*0x100, &val);
               chanGain = BWFE_g2_Adc_P_ChanIntToGain(h, val); /* 9.19 scale */
               //BWFE_DEBUG_PHASE_CAL(BKNI_Printf("   gain%d=%08X->%08X\n", img, val, chanGain));

               if (img == 0)
               {
                  /* calculate total gain of main image = 20log(x) */
                  /* subtract out chanGain 20log(2^19) scale */
                  mainPower = -((BMTH_2560log10(chanGain) << 12) - 59974037); /* 9.19 scale */
                  BWFE_DEBUG_PHASE_CAL(BKNI_Printf("i%d:mainPwr=%08X\n", img, mainPower));

                  /* store max main power */
                  if (mainPower > maxMainPower)
                     maxMainPower = mainPower;
               #if 1
                  else if (BWFE_ABS(mainPower - maxMainPower) > BWFE_PHASE_PWR_SCALE)
                     break;   /* terminate early if main power differs from max power by more than 1.0dB */
               #endif
               }
               else
               {
                  /* accumulate imgPower += chanGain^-2 */
                  /* let a=imgPower, b=chanGain, then a + 1/b^2 = (a*b^2 + 1)/b^2 */
                  BMTH_HILO_32TO64_Mul(chanGain, chanGain, &P_hi, &P_lo);  /* product is double-scaled at 18.38 */
                  chanGain = ((P_hi << 8) | (P_lo >> 24));  /* reduce 18.38 format to 18.14 */
                  BMTH_HILO_32TO64_Mul(imgPower, chanGain, &P_hi, &P_lo);  /* product is scaled up to 27.33 */
                  BMTH_HILO_64TO64_Add(P_hi, P_lo, 0x2, 0, &P_hi, &P_lo);  /* add one scaled at 27.33 = 0x2|0000|0000 */
                  BMTH_HILO_64TO64_Div32(P_hi, P_lo, chanGain, &P_hi, &P_lo); /* divide by 18.14 */
                  imgPower = P_lo;  /* back to 9.19 scaling */
                  BWFE_DEBUG_PHASE_CAL(BKNI_Printf("i%d: imgPwr=%08X", img, imgPower));

                  /* imgRatio = mainPower - 10log(imgPower) */
                  /* subtract out imgPower 10log(2^19) scale */
                  imgRatio = mainPower - ((BMTH_2560log10(imgPower) << 11) - 29987019);   /* 9.19 scale */
                  /*BWFE_DEBUG_PHASE_CAL(BDBG_MSG(("   imgRatio=%08X", imgRatio)));*/

               #if 1
                  /* terminate early if image ratio less than max ratio */
                  if (imgRatio < maxImgRatio)
                     break;
               #endif
               }
            }

            /* update phases if max image ratio exceeded */
            if (imgRatio > maxImgRatio)
            {
               phase[0] = p0[i];
               phase[1] = p1[j];
               phase[2] = p2[k];
               maxImgRatio = imgRatio;
               BWFE_DEBUG_PHASE_CAL(BDBG_MSG(("***maxImgRatio=%08X: %d/%d/%d/%d", maxImgRatio, phase[0], phase[1], phase[2], phase[3])));

            #if 1
               val = BREG_Read32(h->pDevice->hRegister, BCHP_LEAP_CTRL_GP22); /* move 2nd img ratio to 3rd */
               BREG_Write32(h->pDevice->hRegister, BCHP_LEAP_CTRL_GP24, val);
               val = BREG_Read32(h->pDevice->hRegister, BCHP_LEAP_CTRL_GP23);
               BREG_Write32(h->pDevice->hRegister, BCHP_LEAP_CTRL_GP25, val);

               val = BREG_Read32(h->pDevice->hRegister, BCHP_LEAP_CTRL_GP20); /* move max img ratio to 2nd */
               BREG_Write32(h->pDevice->hRegister, BCHP_LEAP_CTRL_GP22, val);
               val = BREG_Read32(h->pDevice->hRegister, BCHP_LEAP_CTRL_GP21);
               BREG_Write32(h->pDevice->hRegister, BCHP_LEAP_CTRL_GP23, val);
            #endif

               BREG_Write32(h->pDevice->hRegister, BCHP_LEAP_CTRL_GP10, maxImgRatio);
               BREG_Write32(h->pDevice->hRegister, BCHP_LEAP_CTRL_GP20, maxImgRatio);
               val = (phase[3] << 28) | (phase[2] << 25) | (phase[1] << 22) | (phase[0] << 19);
               BREG_Write32(h->pDevice->hRegister, BCHP_LEAP_CTRL_GP21, val);
            }
            else if (imgRatio > maxImgRatio2)
            {
               maxImgRatio2 = imgRatio;
               BWFE_DEBUG_PHASE_CAL(BDBG_MSG(("*maxRatio2=%08X: %d/%d/%d/3", maxImgRatio2, p0[i], p1[j], p2[k])));

               val = BREG_Read32(h->pDevice->hRegister, BCHP_LEAP_CTRL_GP22); /* move 2nd img ratio to 3rd */
               BREG_Write32(h->pDevice->hRegister, BCHP_LEAP_CTRL_GP24, val);
               val = BREG_Read32(h->pDevice->hRegister, BCHP_LEAP_CTRL_GP23);
               BREG_Write32(h->pDevice->hRegister, BCHP_LEAP_CTRL_GP25, val);

               BREG_Write32(h->pDevice->hRegister, BCHP_LEAP_CTRL_GP22, maxImgRatio2); /* 2nd img ratio */
               val = (3 << 28) | (p2[k] << 25) | (p1[j] << 22) | (p0[i] << 19);
               BREG_Write32(h->pDevice->hRegister, BCHP_LEAP_CTRL_GP23, val); /* 2nd phase */
            }
            else if (imgRatio > maxImgRatio3)
            {
               maxImgRatio3 = imgRatio;
               BWFE_DEBUG_PHASE_CAL(BDBG_MSG(("*maxRatio3=%08X: %d/%d/%d/3", maxImgRatio3, p0[i], p1[j], p2[k])));

               BREG_Write32(h->pDevice->hRegister, BCHP_LEAP_CTRL_GP24, maxImgRatio3); /* 3rd img ratio */
               val = (3 << 28) | (p2[k] << 25) | (p1[j] << 22) | (p0[i] << 19);
               BREG_Write32(h->pDevice->hRegister, BCHP_LEAP_CTRL_GP25, val); /* 3rd phase */
            }
         }
      }
   }

   /* set optimal phases */
   val = (phase[3] << 28) | (phase[2] << 25) | (phase[1] << 22) | (phase[0] << 19);
   BWFE_P_ReadModifyWriteRegister(h, BCHP_WFE_ANA_WBADC_SYS_CNTL_IN, ~0x7FF80000, val);

   /* toggle STB_CHAN_CTRL_XBAR_CTRL.IFIFOS_ON */
   BWFE_P_AndRegister(h, BCHP_STB_CHAN_CTRL_XBAR_CTRL, ~(1<<h->channel));
   BWFE_P_OrRegister(h, BCHP_STB_CHAN_CTRL_XBAR_CTRL, 1<<h->channel);

#if 1
   for (i = 0; i < 4; i++)
   {
      BDBG_MSG(("phase(ADC%d, slice%d)=%d", h->channel, i, phase[i]));
      BKNI_Printf("phase(ADC%d, slice%d)=%d\n", h->channel, i, phase[i]);
   }
#endif

   /* power down target channelizer */
   BWFE_P_OrRegister(h, BCHP_STB_CHAN_CTRL_PWRDN, 1 << chan);

#ifdef BWFE_EXCLUDE_LIC_TAP_COMPUTER
   /* signal ready event if no lic tap calculations */
   BKNI_SetEvent(hChn->hWfeReady);
#endif

   hChn->bPhaseCalComplete = true;
   return retCode;
#else
   BSTD_UNUSED(h);
   BSTD_UNUSED(chan);
   return BERR_NOT_SUPPORTED;
#endif
}

#else

/******************************************************************************
 BWFE_g2_Adc_P_CalibratePhase()
******************************************************************************/
BERR_Code BWFE_g2_Adc_P_CalibratePhase(BWFE_ChannelHandle h)
{
#if (BCHP_CHIP==4538)
   BWFE_g2_P_ChannelHandle *hChn = (BWFE_g2_P_ChannelHandle *)h->pImpl;
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
   retCode = BWFE_g2_P_EnableTimer(h, BWFE_g2_TimerSelect_e0, 5, BWFE_g2_Adc_P_CalibratePhase1);
   BKNI_LeaveCriticalSection();

   return retCode;
#else
   BSTD_UNUSED(h);
   return BERR_NOT_SUPPORTED;
#endif
}


/******************************************************************************
 BWFE_g2_Adc_P_CalibratePhase1() - ISR context
******************************************************************************/
BERR_Code BWFE_g2_Adc_P_CalibratePhase1(BWFE_ChannelHandle h)
{
#if (BCHP_CHIP==4538)
   BERR_Code retCode = BERR_SUCCESS;
   BWFE_g2_P_ChannelHandle *hChn = (BWFE_g2_P_ChannelHandle *)h->pImpl;
   BWFE_g2_P_Handle *hDev = h->pDevice->pImpl;
   uint8_t i, slice, phase, shift, minDelayPhase[BWFE_NUM_SLICES];
   int32_t minDelay[BWFE_NUM_SLICES];
   uint32_t val;

   while (1)
   {
      BDBG_MSG(("BWFE_g2_Adc_P_CalibratePhase1(): S%d", hChn->calState));
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
               hChn->postCalcDelayFunct = BWFE_g2_Adc_P_CalibratePhase1;
               hChn->bCorrInProgress = true;
               return BWFE_g2_Corr_P_StartCorrelator(h, hDev->dpmPilotFreqKhz * 1000, 0, BWFE_g2_Corr_P_CalcDelay);
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
                  for (phase = 0; phase < BWFE_NUM_ADC_PHASES/2; phase++)
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
#endif


/******************************************************************************
 BWFE_g2_Adc_P_Accumulate() - return total accumulated value scaled by 4096
******************************************************************************/
BERR_Code BWFE_g2_Adc_P_Accumulate(BWFE_ChannelHandle h, uint8_t slice, uint8_t lane, uint8_t interval, int32_t *acc)
{
   BERR_Code retCode = BERR_SUCCESS;
   uint32_t val = 0;

   /* reset accumulators, note all accumulators controlled by ping lane slice0 */
   BWFE_P_ToggleSliceBit(h, 0, BCHP_WFE_CORE_DGSCTL_PI_SLC, 0x00000001);

   /* set accumulation length: 4096 samples per unit */
   BWFE_P_ReadModifyWriteSliceReg(h, 0, BCHP_WFE_CORE_DGSCTL_PI_SLC, ~0x0000FF00, (interval & 0xFF) << 8);

   /* reset and start accumulator */
   BWFE_P_AndSliceReg(h, 0, BCHP_WFE_CORE_DGSCTL_PI_SLC, ~0x00000002);
   BWFE_P_OrSliceReg(h, 0, BCHP_WFE_CORE_DGSCTL_PI_SLC, 0x00000002);   /* edge detect, non self clearing */

   do {
      /* poll for dgsum2_accu_active = 0 */
      BWFE_P_ReadRegister(h, BCHP_WFE_CORE_HDOFFSTS, &val);
   }
   while (val & 0x2);

   /* stop accumulator */
   BWFE_P_AndSliceReg(h, 0, BCHP_WFE_CORE_DGSCTL_PI_SLC, ~0x00000002);

   /* read accumulator with lane context */
   switch (lane)
   {
      case 0:
         BWFE_P_ReadSliceRegister(h, slice, BCHP_WFE_CORE_DGSACCUM_DMX0_PI_SLC, &val);
         break;
      case 1:
         BWFE_P_ReadSliceRegister(h, slice, BCHP_WFE_CORE_DGSACCUM_DMX0_PO_SLC, &val);
         break;
      default:
         retCode = BERR_INVALID_PARAMETER;
   }

   /* note cannot scale down here, must maintain precision, scale down later */
   *acc = (int32_t)val / 4;  /* value still scaled up by 4096, also need to divide by accumulation interval+1 to get average */ /* drop extra 2 fractional bits for mdac eq */
   return retCode;
}


/******************************************************************************
 BWFE_g2_Adc_P_CalibrateINL() - integrated non-linearities algorithm (clocks on, RF off)
******************************************************************************/
BERR_Code BWFE_g2_Adc_P_CalibrateINL(BWFE_ChannelHandle h)
{
   BERR_Code retCode = BERR_NOT_SUPPORTED;

   if (!h->bEnabled)
      return BWFE_ERR_POWERED_DOWN;

   /* TBD */

   return retCode;
}


/******************************************************************************
 BWFE_g2_Adc_P_LutSigned2Tc()
******************************************************************************/
uint16_t BWFE_g2_Adc_P_LutSigned2Tc(uint8_t mdac, int32_t val)
{
   uint16_t uval = 0;

   BDBG_ASSERT(mdac <= BWFE_NUM_MDACS);

   if (mdac == 1)
   {
      /* convert to 12-bit register field */
      if (val < -2048)
         uval = 0x800;     /* saturate negative value */
      else if (val > 2047)
         uval = 0xFFF;     /* saturate positive value */
      else
      {
         if (val < 0)
            uval = val + 0x1000; /* truncate sign extension if negative */

         uval = val & 0xFFF;     /* 12-bit field */
      }
   }
   else if (mdac == 2)
   {
      /* convert to 11-bit register field */
      if (val < -1024)
         uval = 0x400;     /* saturate negative value */
      else if (val > 1023)
         uval = 0x7FF;     /* saturate positive value */
      else
      {
         if (val < 0)
            uval = val + 0x800;  /* truncate sign extension if negative */

         uval = val & 0x7FF;     /* 11-bit field */
      }
   }

   return uval;
}


/******************************************************************************
 BWFE_g2_Adc_P_CoeffTc2Signed() - scale 2^17 to maintain resolution
******************************************************************************/
int32_t BWFE_g2_Adc_P_CoeffTc2Signed(uint32_t tc)
{
   int32_t val;

   /* sign extension if negative */
	if (tc > 0x07FFFFFF)
      tc -= 0x10000000;

   val = (int32_t)tc;
	return val / 512; /* scale up 2^17 for resolution, then down 2^26 */
}


/******************************************************************************
 BWFE_g2_Adc_P_CoeffSigned2Tc() - unscale 2^17
******************************************************************************/
uint32_t BWFE_g2_Adc_P_CoeffSigned2Tc(int32_t val)
{
   /* scale back up 2^26, note val scaled up 2^17 */
   val *= 512;

   /* truncate sign extension if negative */
	if (val < 0)
      val += 0x10000000;

   return (uint32_t)val;
}


/******************************************************************************
 BWFE_g2_Adc_P_DacBinSearch() - DAC calibration with binary search
******************************************************************************/
BERR_Code BWFE_g2_Adc_P_DacBinSearch(BWFE_ChannelHandle h, uint8_t slice, uint8_t lane, int32_t target)
{
   BERR_Code retCode = BERR_SUCCESS;
   int32_t avgUpper, avgLower;
   uint8_t msbUpper, msbLower, lsbUpper, lsbLower;
   uint8_t msbFinal, lsbFinal;

   if (!h->bEnabled)
      return BWFE_ERR_POWERED_DOWN;

   BWFE_DEBUG_DAC(BDBG_MSG(("DacBinSearch(lane%d): target=%d", lane, target)));

   lsbUpper = msbUpper = 31;
   lsbLower = msbLower = 0;

   if (target >= 256)
   {
      /* skip calibration and set maximum code */
      BWFE_P_OrRegister(h, BCHP_WFE_ANA_WBADC_TOP_CNTL_IN, 0x003FF000);
      return retCode;
   }
   target *= (BWFE_ACC_INTERVAL + 1) * 4096;    /* scale target up by acc length and 4096 for comparison with accumulator */

   /* search msb */
   while (msbUpper != msbLower)
   {
      /* get upper average */
      BWFE_P_ReadModifyWriteRegister(h, BCHP_WFE_ANA_WBADC_TOP_CNTL_IN, ~0x003FF000, msbUpper << 17 | lsbUpper << 12);
      retCode = BWFE_g2_Adc_P_Accumulate(h, slice, lane, BWFE_ACC_INTERVAL, &avgUpper);   /* set maximum accumulation length, 4096 samples per unit */
      if (retCode)
      {
         BDBG_ERR(("DacBinSearch: lane%d/msb%d/lsb%d error 0x%X!", lane, msbUpper, lsbUpper, retCode));
      }

      /* get lower average */
      BWFE_P_ReadModifyWriteRegister(h, BCHP_WFE_ANA_WBADC_TOP_CNTL_IN, ~0x003FF000, msbLower << 17 | lsbLower << 12);
      retCode = BWFE_g2_Adc_P_Accumulate(h, slice, lane, BWFE_ACC_INTERVAL, &avgLower);   /* set maximum accumulation length, 4096 samples per unit */
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
   BWFE_P_ReadModifyWriteRegister(h, BCHP_WFE_ANA_WBADC_TOP_CNTL_IN, ~0x003E0000, msbFinal << 17);

   /* search lsb */
   while (lsbUpper != lsbLower)
   {
      /* get upper average */
      BWFE_P_ReadModifyWriteRegister(h, BCHP_WFE_ANA_WBADC_TOP_CNTL_IN, ~0x0001F000, lsbUpper << 12);
      retCode = BWFE_g2_Adc_P_Accumulate(h, slice, lane, BWFE_ACC_INTERVAL, &avgUpper);   /* set maximum accumulation length, 4096 samples per unit */
      if (retCode)
      {
         BDBG_ERR(("DacBinSearch: lane%d/msb%d/lsb%d error 0x%X!", lane, msbFinal, lsbUpper, retCode));
      }

      /* get lower average */
      BWFE_P_ReadModifyWriteRegister(h, BCHP_WFE_ANA_WBADC_TOP_CNTL_IN, ~0x0001F000, lsbLower << 12);
      retCode = BWFE_g2_Adc_P_Accumulate(h, slice, lane, BWFE_ACC_INTERVAL, &avgLower);   /* set maximum accumulation length, 4096 samples per unit */
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
   BWFE_P_ReadModifyWriteRegister(h, BCHP_WFE_ANA_WBADC_TOP_CNTL_IN, ~0x0001F000, lsbFinal << 12);

   /* get final average */
   retCode = BWFE_g2_Adc_P_Accumulate(h, slice, lane, BWFE_ACC_INTERVAL, &avgLower);   /* set maximum accumulation length, 4096 samples per unit */
   BWFE_DEBUG_DAC(BDBG_MSG(("avgFinal=%d", avgLower / ((BWFE_ACC_INTERVAL + 1) * 4096))));  /* div by acc length to get avg, scale back down by 4096 */

   if (BWFE_ABS(target - avgLower) > BWFE_DAC_SEARCH_THRESH)
      retCode = BWFE_ERR_DAC_BIN_SEARCH_FAIL;

   return retCode;
}


/******************************************************************************
 BWFE_g2_Adc_P_DacLinSearch() - DAC calibration with linear search
******************************************************************************/
BERR_Code BWFE_g2_Adc_P_DacLinSearch(BWFE_ChannelHandle h, uint8_t slice, uint8_t lane, int32_t target)
{
   BERR_Code retCode = BERR_SUCCESS;
   int32_t avgCurrent;
   uint16_t caldacInput, caldacStart, caldacDelta, i;
   bool bScanUpDir = true;   /* scan up initially */

   if (!h->bEnabled)
      return BWFE_ERR_POWERED_DOWN;

   BWFE_DEBUG_DAC(BDBG_MSG(("DacLinSearch(lane%d): target=%d", lane, target)));

   /* start search at midpoint */
   caldacStart = caldacInput = 0x200;
   caldacDelta = 1;

   if (target >= 256)
   {
      /* skip calibration and set maximum code */
      BWFE_P_OrRegister(h, BCHP_WFE_ANA_WBADC_TOP_CNTL_IN, 0x003FF000);
      return retCode;
   }
   target *= (BWFE_ACC_INTERVAL + 1) * 4096;    /* scale target up by acc length and 4096 for comparison with accumulator */

   for (i = 0; i < 1024; i++)
   {
      BWFE_P_ReadModifyWriteRegister(h, BCHP_WFE_ANA_WBADC_TOP_CNTL_IN, ~0x003FF000, caldacInput << 12);
      retCode = BWFE_g2_Adc_P_Accumulate(h, slice, lane, BWFE_ACC_INTERVAL, &avgCurrent); /* set maximum accumulation length, 4096 samples per unit */
      if (retCode)
      {
         BDBG_ERR(("DacLinSearch: lane%d/msb%d/lsb%d error 0x%X!", lane, caldacInput >> 5, caldacInput & 0x1F, retCode));
      }

      BWFE_DEBUG_DAC(BDBG_MSG(("caldacInput=%X", caldacInput)));

      if (BWFE_ABS(target - avgCurrent) < BWFE_DAC_SEARCH_THRESH)
      {
         /* stop linear search when accumulator is within target +/- thresh */
         break;
      }
      else if ((caldacInput >= 0x3FF) || (caldacInput == 0))
      {
         /* stop linear search when upper or lower bounds reached */
         break;
      }
      else
      {
         if (bScanUpDir)
         {
            caldacInput = caldacStart + caldacDelta;
         }
         else
         {
            caldacInput = caldacStart - caldacDelta;
            caldacDelta++;
         }

         /* reverse direction every step */
         bScanUpDir = !bScanUpDir;
      }
   }

   /* final msb, lsb */
   BWFE_DEBUG_DAC(BDBG_MSG(("msbLsbFinal=%d", caldacInput)));
   BWFE_DEBUG_DAC(BDBG_MSG(("avgFinal=%d", avgCurrent / ((BWFE_ACC_INTERVAL + 1) * 4096))));  /* div by acc length to get avg, scale back down by 4096 */

   return retCode;
}


/******************************************************************************
 BWFE_g2_Adc_P_GetGainError()
******************************************************************************/
BERR_Code BWFE_g2_Adc_P_GetGainError(BWFE_ChannelHandle h, uint8_t slice, uint8_t lane, uint8_t mdac, uint32_t *gainErr)
{
   BERR_Code retCode = BERR_SUCCESS;
   uint32_t err, scale;
   int32_t e1, e2;

   /* enable MDAC equ mode */
   BWFE_P_OrRegister(h, BCHP_WFE_ANA_WBADC_CAL_CNTL_IN, 1 << (31 - slice));
   BWFE_P_ReadModifyWriteRegister(h, BCHP_WFE_ANA_ADC_CNTL3, ~0x00003000, 1 << (mdac + 11));  /* enable mdac gain calibration */

   /* set output to mid-level and accumulate */
   BWFE_P_AndRegister(h, BCHP_WFE_ANA_WBADC_CAL_CNTL_IN, ~(0xF << ((mdac - 1) * 5)));
   BWFE_P_OrRegister(h, BCHP_WFE_ANA_WBADC_CAL_CNTL_IN, (0x8 << ((mdac - 1) * 5)));
   retCode = BWFE_g2_Adc_P_Accumulate(h, slice, lane, BWFE_ACC_INTERVAL, &e1);   /* set maximum accumulation length, 4096 samples per unit */

   /* set output to mid-level +1 and accumulate */
   BWFE_P_AndRegister(h, BCHP_WFE_ANA_WBADC_CAL_CNTL_IN, ~(0xF << ((mdac - 1) * 5)));
   BWFE_P_OrRegister(h, BCHP_WFE_ANA_WBADC_CAL_CNTL_IN, (0x4 << ((mdac - 1) * 5)));
   retCode = BWFE_g2_Adc_P_Accumulate(h, slice, lane, BWFE_ACC_INTERVAL, &e2);   /* set maximum accumulation length, 4096 samples per unit */

   /* e2 - e1 = MDAC EQ gain, e1 - e2 = INL gain */
   scale = (BWFE_NUM_MDACS - mdac + 1) << 2;
   scale = (scale * scale) << 1;    /* scale 32 for mdac2, 128 for mdac1 */
   err = (scale << 20) + e1 - e2;   /* scale up by 4096, multiply by acc length to match accumulated values */

   /* disable equ mode and PN sequence */
   BWFE_P_AndRegister(h, BCHP_WFE_ANA_WBADC_CAL_CNTL_IN, ~(1 << (31 - slice)));
   BWFE_P_AndRegister(h, BCHP_WFE_ANA_WBADC_CAL_CNTL_IN, ~(0xF << ((mdac - 1) * 5)));
   BWFE_P_AndRegister(h, BCHP_WFE_ANA_ADC_CNTL3, ~0x00003000); /* disable mdac gain calibration */

   /* normalize gain error */
   *gainErr = err / scale; /* note still scaled up by 4096 * 256 */

   BWFE_DEBUG_EQU(BDBG_MSG(("e1=%d", e1)));
   BWFE_DEBUG_EQU(BDBG_MSG(("e2=%d", e2)));
   BWFE_DEBUG_EQU(BDBG_MSG(("scale=%d", scale)));
   BWFE_DEBUG_EQU(BDBG_MSG(("gainErr=0x%08X(%d)", *gainErr, *gainErr)));
   return retCode;
}


/******************************************************************************
 BWFE_g2_Adc_P_CancelDcOffset()
******************************************************************************/
BERR_Code BWFE_g2_Adc_P_CancelDcOffset(BWFE_ChannelHandle h, uint8_t slice, uint8_t mdac)
{
   BERR_Code retCode = BERR_SUCCESS;
   int32_t diff, acc[2 * BWFE_NUM_LANES]; /* 2 levels x 2 lanes */
   uint32_t mask;
   uint16_t offset[3];
   uint8_t shift;

   BWFE_DEBUG_EQU(BDBG_MSG(("CANCEL DC OFFSET...")));

   /* enable MDAC equ mode */
   BWFE_P_OrRegister(h, BCHP_WFE_ANA_WBADC_CAL_CNTL_IN, 1 << (31 - slice));
   BWFE_P_ReadModifyWriteRegister(h, BCHP_WFE_ANA_ADC_CNTL3, ~0x00003000, 1 << (mdac + 11));  /* enable mdac gain calibration */

   /* set output to mid-level and accumulate */
   BWFE_P_AndRegister(h, BCHP_WFE_ANA_WBADC_CAL_CNTL_IN, ~(0xF << ((mdac - 1) * 5)));
   BWFE_P_OrRegister(h, BCHP_WFE_ANA_WBADC_CAL_CNTL_IN, (0x8 << ((mdac - 1) * 5)));
   retCode = BWFE_g2_Adc_P_Accumulate(h, slice, 0, BWFE_ACC_INTERVAL, &acc[0]);  /* set maximum accumulation length, 4096 samples per unit */
   retCode = BWFE_g2_Adc_P_Accumulate(h, slice, 1, BWFE_ACC_INTERVAL, &acc[2]);  /* set maximum accumulation length, 4096 samples per unit */

   /* set output to mid-level +1 and accumulate */
   BWFE_P_AndRegister(h, BCHP_WFE_ANA_WBADC_CAL_CNTL_IN, ~(0xF << ((mdac - 1) * 5)));
   BWFE_P_OrRegister(h, BCHP_WFE_ANA_WBADC_CAL_CNTL_IN, (0x4 << ((mdac - 1) * 5)));
   retCode = BWFE_g2_Adc_P_Accumulate(h, slice, 0, BWFE_ACC_INTERVAL, &acc[1]);  /* set maximum accumulation length, 4096 samples per unit */
   retCode = BWFE_g2_Adc_P_Accumulate(h, slice, 1, BWFE_ACC_INTERVAL, &acc[3]);  /* set maximum accumulation length, 4096 samples per unit */

   /* disable equ mode */
   BWFE_P_AndRegister(h, BCHP_WFE_ANA_WBADC_CAL_CNTL_IN, ~(1 << (31 - slice)));
   BWFE_P_AndRegister(h, BCHP_WFE_ANA_WBADC_CAL_CNTL_IN, ~(0xF << ((mdac - 1) * 5)));
   BWFE_P_AndRegister(h, BCHP_WFE_ANA_ADC_CNTL3, ~0x00003000); /* disable mdac gain calibration */

   BWFE_DEBUG_EQU(BDBG_MSG((" acc1=0x%08X(%d)", acc[0], acc[0])));
   BWFE_DEBUG_EQU(BDBG_MSG((" acc2=0x%08X(%d)", acc[1], acc[1])));
   BWFE_DEBUG_EQU(BDBG_MSG((" acc3=0x%08X(%d)", acc[2], acc[2])));
   BWFE_DEBUG_EQU(BDBG_MSG((" acc4=0x%08X(%d)", acc[3], acc[3])));

   /* signed to unsigned conversion */
   diff = (acc[0] - acc[1]) * 64;            /* multiply by 64 for 6 fractional LUT bits */
   diff /= (BWFE_ACC_INTERVAL + 1) * 4096;   /* div by acc length to get avg, scale back down by 4096 */
   offset[0] = BWFE_g2_Adc_P_LutSigned2Tc(mdac, diff);
   diff = (acc[0] - acc[2]) * 64;            /* multiply by 64 for 6 fractional LUT bits */
   diff /= (BWFE_ACC_INTERVAL + 1) * 4096;   /* div by acc length to get avg, scale back down by 4096 */
   offset[1] = BWFE_g2_Adc_P_LutSigned2Tc(mdac, diff);
   diff = (acc[0] - acc[3]) * 64;            /* multiply by 64 for 6 fractional LUT bits */
   diff /= (BWFE_ACC_INTERVAL + 1) * 4096;   /* div by acc length to get avg, scale back down by 4096 */
   offset[2] = BWFE_g2_Adc_P_LutSigned2Tc(mdac, diff);

   BWFE_DEBUG_EQU(BDBG_MSG(("offset1=0x%08X(%d)", offset[0], offset[0])));
   BWFE_DEBUG_EQU(BDBG_MSG(("offset2=0x%08X(%d)", offset[1], offset[1])));
   BWFE_DEBUG_EQU(BDBG_MSG(("offset3=0x%08X(%d)", offset[2], offset[2])));

   /* program LUT values to cancel DC offset */
   mask = 0xFFF >> (mdac - 1);
   shift = (BWFE_NUM_MDACS - mdac + 1) * 10 - (mdac - 1);
   BWFE_P_ReadModifyWriteSliceReg(h, slice, BCHP_WFE_CORE_DGSLUT001_PI_SLC, ~(mask << shift), offset[0] << shift);
   BWFE_P_ReadModifyWriteSliceReg(h, slice, BCHP_WFE_CORE_DGSLUT000_PO_SLC, ~(mask << shift), offset[1] << shift);
   BWFE_P_ReadModifyWriteSliceReg(h, slice, BCHP_WFE_CORE_DGSLUT001_PO_SLC, ~(mask << shift), offset[2] << shift);

#if 1
   BWFE_DEBUG_EQU(BDBG_MSG(("CHECK DC OFFSET...")));

   /* enable MDAC equ mode */
   BWFE_P_OrRegister(h, BCHP_WFE_ANA_WBADC_CAL_CNTL_IN, 1 << (31 - slice));
   BWFE_P_ReadModifyWriteRegister(h, BCHP_WFE_ANA_ADC_CNTL3, ~0x00003000, 1 << (mdac + 11));  /* enable mdac gain calibration */

   /* set output to mid-level and accumulate */
   BWFE_P_AndRegister(h, BCHP_WFE_ANA_WBADC_CAL_CNTL_IN, ~(0xF << ((mdac - 1) * 5)));
   BWFE_P_OrRegister(h, BCHP_WFE_ANA_WBADC_CAL_CNTL_IN, (0x8 << ((mdac - 1) * 5)));
   retCode = BWFE_g2_Adc_P_Accumulate(h, slice, 0, BWFE_ACC_INTERVAL, &acc[0]);  /* set maximum accumulation length, 4096 samples per unit */
   retCode = BWFE_g2_Adc_P_Accumulate(h, slice, 1, BWFE_ACC_INTERVAL, &acc[2]);  /* set maximum accumulation length, 4096 samples per unit */

   /* set output to mid-level +1 and accumulate */
   BWFE_P_AndRegister(h, BCHP_WFE_ANA_WBADC_CAL_CNTL_IN, ~(0xF << ((mdac - 1) * 5)));
   BWFE_P_OrRegister(h, BCHP_WFE_ANA_WBADC_CAL_CNTL_IN, (0x4 << ((mdac - 1) * 5)));
   retCode = BWFE_g2_Adc_P_Accumulate(h, slice, 0, BWFE_ACC_INTERVAL, &acc[1]);  /* set maximum accumulation length, 4096 samples per unit */
   retCode = BWFE_g2_Adc_P_Accumulate(h, slice, 1, BWFE_ACC_INTERVAL, &acc[3]);  /* set maximum accumulation length, 4096 samples per unit */

   /* disable equ mode */
   BWFE_P_AndRegister(h, BCHP_WFE_ANA_WBADC_CAL_CNTL_IN, ~(1 << (31 - slice)));
   BWFE_P_AndRegister(h, BCHP_WFE_ANA_WBADC_CAL_CNTL_IN, ~(0xF << ((mdac - 1) * 5)));
   BWFE_P_AndRegister(h, BCHP_WFE_ANA_ADC_CNTL3, ~0x00003000); /* disable mdac gain calibration */

   BWFE_DEBUG_EQU(BDBG_MSG((" acc1=0x%08X(%d)", acc[0], acc[0])));
   BWFE_DEBUG_EQU(BDBG_MSG((" acc2=0x%08X(%d)", acc[1], acc[1])));
   BWFE_DEBUG_EQU(BDBG_MSG((" acc3=0x%08X(%d)", acc[2], acc[2])));
   BWFE_DEBUG_EQU(BDBG_MSG((" acc4=0x%08X(%d)", acc[3], acc[3])));
#endif

   return retCode;
}

/******************************************************************************
 BWFE_g2_Adc_P_ScaleEqTaps()
******************************************************************************/
BERR_Code BWFE_g2_Adc_P_ScaleEqTaps(BWFE_ChannelHandle h, uint8_t slice, uint8_t lane, uint8_t mdac, uint32_t gainScale)
{
   BWFE_g2_P_ChannelHandle *hChn = (BWFE_g2_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode = BERR_SUCCESS;
   uint32_t val, P_hi, P_lo, Q_hi, Q_lo;
   bool bNeg = false;
   uint8_t tap;

   /* read eq taps */
   retCode = BWFE_g2_Adc_P_ReadEquTaps(h);
   if (retCode)
   {
      BDBG_ERR(("ScaleEqTaps: ERROR reading eq taps!"));
      return retCode;
   }

   /* scale down taps */
   for (tap = 0; tap < BWFE_NUM_EQ_TAPS; tap++)
   {
      if (hChn->equCoeff[slice][lane][mdac-1][tap] < 0)
      {
         /* take absolute value of tap for extended precision */
         val = -(hChn->equCoeff[slice][lane][mdac-1][tap]);
         bNeg = true;
      }
      else
      {
         /* positive case */
         val = hChn->equCoeff[slice][lane][mdac-1][tap];
         bNeg = false;
      }

      BMTH_HILO_32TO64_Mul(val, 1048576, &P_hi, &P_lo);  /* scale tap up by acc length and 4096 to match gainScale */
      BMTH_HILO_64TO64_Div32(P_hi, P_lo, gainScale, &Q_hi, &Q_lo);   /* divide out gainScale */

      /* reverse sign if initial tap was negative */
      hChn->equCoeff[slice][lane][mdac-1][tap] = bNeg ? -Q_lo : Q_lo;
   }

   /* update eq taps */
   retCode = BWFE_g2_Adc_P_WriteEquTaps(h);
   return retCode;
}


/******************************************************************************
 BWFE_g2_Adc_P_EqualizePipeline() - note RFFE should be off during equalization
******************************************************************************/
/* TBD convert to isr context using state machine and timers */
BERR_Code BWFE_g2_Adc_P_EqualizePipeline(BWFE_ChannelHandle h)
{
   BWFE_g2_P_ChannelHandle *hChn = (BWFE_g2_P_ChannelHandle *)h->pImpl;
   uint32_t gainErrPing, gainErrPong, val, mask;
   uint8_t mdac, slice, target, shift;

   if (!h->bEnabled)
      return BWFE_ERR_POWERED_DOWN;

   /* power down RFFE */
   BWFE_g2_Rffe_P_PowerDown(h);  /* TBD set sha output to zero instead of disabling rffe? */

   /* power up caldac */
#if (BCHP_CHIP==4550)
   BWFE_P_AndRegister(h, BCHP_WFE_ANA_WBADC_TOP_CNTL_IN, ~0x18000200);  /* also set termination resistance to 1K */
#elif (BCHP_CHIP==4538)
   BWFE_P_AndRegister(h, BCHP_WFE_ANA_WBADC_TOP_CNTL_IN, ~0x18000040);  /* also set termination resistance to 1K */
#endif
   BWFE_P_OrRegister(h, BCHP_WFE_ANA_WBADC_TOP_CNTL_IN, 0x01C00000);    /* set size control to 7 */

   for (slice = 0; slice < h->totalSlices; slice++)
   {
      /* set mu step size */
      val = (hChn->equMainMu1 << 20) | (hChn->equMu1 << 16) | (hChn->equMainMu2 << 12) | (hChn->equMu2 << 8);
      BWFE_P_WriteSliceRegister(h, slice, BCHP_WFE_CORE_DGSLMSMU_SLC, val);

      for (mdac = BWFE_NUM_MDACS; mdac > 0; mdac--)
      {
         /* bypass mdac if disabled */
         if ((hChn->equMdacEnable & (1 << (mdac - 1))) == 0)
            break;

         BWFE_DEBUG_EQU(BDBG_MSG(("\n>>slice%d: mdac%d", slice, mdac)));

         /* target 16 for mdac2, 64 for mdac1 */
         target = (BWFE_NUM_MDACS - mdac + 1) << 2;
         target *= target;

         /* get gain error for each lane */
         BWFE_g2_Adc_P_DacBinSearch(h, slice, 0, target);   /* dac binary search for ping */
         BWFE_g2_Adc_P_GetGainError(h, slice, 0, mdac, &gainErrPing);   /* note gain err scaled up by 4096 * 256 */
         BWFE_g2_Adc_P_DacBinSearch(h, slice, 1, target);   /* dac binary search for pong */
         BWFE_g2_Adc_P_GetGainError(h, slice, 1, mdac, &gainErrPong);   /* note gain err scaled up by 4096 * 256 */

         BWFE_DEBUG_EQU(BDBG_MSG(("PING gain err=%08X", gainErrPing)));
         BWFE_DEBUG_EQU(BDBG_MSG(("PONG gain err=%08X", gainErrPong)));

         /* cancel dc offset */
         BWFE_g2_Adc_P_ResetLut(h, slice, mdac);
         BWFE_g2_Adc_P_DacBinSearch(h, hChn->equCurrSlice, 0, target);  /* redo dac binary search for ping only */
         BWFE_g2_Adc_P_CancelDcOffset(h, slice, mdac);

         /* enable MDAC equ mode and training sequence */
         BWFE_P_OrRegister(h, BCHP_WFE_ANA_WBADC_CAL_CNTL_IN, 1 << (31 - slice));
         BWFE_P_ReadModifyWriteRegister(h, BCHP_WFE_ANA_ADC_CNTL3, ~0x00003000, 1 << (mdac + 11)); /* enable mdac gain calibration */
         shift = (mdac - 1) * 5;
         BWFE_P_ReadModifyWriteRegister(h, BCHP_WFE_ANA_WBADC_CAL_CNTL_IN, ~(0xF << shift), 0xA << shift);  /* use PN1 as training sequence */

         /* update ping and pong coefficients */
         BWFE_P_OrSliceReg(h, slice, BCHP_WFE_CORE_DGSLMS_SLC, 0x03000000);

         /* enable PN input for LMS and update coefficients for corresponding mdac */
         shift = 8 * (3 - mdac);
         mask = (hChn->equTapMask << 4) | 0x2;
         BWFE_P_ReadModifyWriteSliceReg(h, slice, BCHP_WFE_CORE_DGSLMS_SLC, ~(0xF3 << shift), mask << shift);

         /* run for specified time */
         BKNI_Sleep(hChn->equRuntimeMs);

         /* disable PN input and stop update */
         BWFE_P_AndSliceReg(h, slice, BCHP_WFE_CORE_DGSLMS_SLC, ~0x03F3F300);

         /* disable equ mode and PN sequence */
         BWFE_P_AndRegister(h, BCHP_WFE_ANA_WBADC_CAL_CNTL_IN, ~(1 << (31 - slice)));
         BWFE_P_AndRegister(h, BCHP_WFE_ANA_WBADC_CAL_CNTL_IN, ~0x000001EF);
         BWFE_P_AndRegister(h, BCHP_WFE_ANA_ADC_CNTL3, ~0x00003000); /* disable mdac gain calibration */

         /* reset lut for ping and pong */
         BWFE_g2_Adc_P_ResetLut(h, slice, mdac); /* since this dc offset only applies to equ mode */

         /* gain scale eq taps for both lanes */
         BWFE_g2_Adc_P_ScaleEqTaps(h, slice, 0, mdac, gainErrPing);
         BWFE_g2_Adc_P_ScaleEqTaps(h, slice, 1, mdac, gainErrPong);
      }
   }

   /* power down caldac */
#if (BCHP_CHIP==4550)
   BWFE_P_OrRegister(h, BCHP_WFE_ANA_WBADC_TOP_CNTL_IN, 0x18000200);    /* revert termination resistance to 50K */
#elif (BCHP_CHIP==4538)
   BWFE_P_OrRegister(h, BCHP_WFE_ANA_WBADC_TOP_CNTL_IN, 0x18000040);
#endif

   /* power up RFFE */
   return BWFE_g2_Rffe_P_PowerUp(h);
}


/******************************************************************************
 BWFE_g2_Adc_P_EqualizeMdac() - note RFFE should be off during equalization
******************************************************************************/
BERR_Code BWFE_g2_Adc_P_EqualizeMdac(BWFE_ChannelHandle h)
{
   BWFE_g2_P_ChannelHandle *hChn = (BWFE_g2_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode = BERR_SUCCESS;

   if (hChn->equState < 4)
      return BWFE_ERR_EQU_BUSY;

   /* mdac equalization entry point */
   hChn->equState = 0;

   /* enter EqualizeMdac1() in ISR context */
   BKNI_EnterCriticalSection();
   retCode = BWFE_g2_P_EnableTimer(h, BWFE_g2_TimerSelect_e0, 5, BWFE_g2_Adc_P_EqualizeMdac1);
   BKNI_LeaveCriticalSection();

   return retCode;
}


/******************************************************************************
 BWFE_g2_Adc_P_EqualizeMdac1() - ISR context
******************************************************************************/
//#define BWFE_LINEAR_SEARCH_BACKUP
BERR_Code BWFE_g2_Adc_P_EqualizeMdac1(BWFE_ChannelHandle h)
{
   BWFE_g2_P_ChannelHandle *hChn = (BWFE_g2_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode = BERR_SUCCESS;
   uint32_t mask, val;
   uint8_t target, shift;

   while (1)
   {
      BWFE_DEBUG_EQU(BDBG_MSG(("BWFE_g2_Adc_P_EqualizeMdac(): S%d", hChn->equState)));
      switch (hChn->equState)
      {
         case 0:
            /* power down RFFE if not reference adc */
            if (!h->bReference)
               BWFE_g2_Rffe_P_PowerDown(h);  /* TBD set sha output to zero instead of disabling rffe? */

            /* power up caldac, also set termination resistance to 1K */
         #if (BCHP_CHIP==4550)
            BWFE_P_AndRegister(h, BCHP_WFE_ANA_WBADC_TOP_CNTL_IN, ~0x18000200);
         #elif (BCHP_CHIP==4538)
            BWFE_P_AndRegister(h, BCHP_WFE_ANA_WBADC_TOP_CNTL_IN, ~0x18000040);
         #endif
            BWFE_P_OrRegister(h, BCHP_WFE_ANA_WBADC_TOP_CNTL_IN, 0x01C00000);    /* set size control to 7 */

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

               /* disable mdac initially */
               BWFE_P_AndRegister(h, BCHP_WFE_ANA_WBADC_CAL_CNTL_IN, ~0xF0000000);  /* disable MDAC equ mode */
               BWFE_P_AndRegister(h, BCHP_WFE_ANA_ADC_CNTL3, ~0x00003000);          /* disable mdac gain calibration */
               BWFE_P_AndRegister(h, BCHP_WFE_ANA_WBADC_CAL_CNTL_IN, ~0x000001EF);  /* disable training sequence */

               /* get gain error for each lane */
               retCode = BWFE_g2_Adc_P_DacBinSearch(h, hChn->equCurrSlice, 0, target);  /* dac binary search for ping */
            #ifdef BWFE_LINEAR_SEARCH_BACKUP
               if (retCode == BWFE_ERR_DAC_BIN_SEARCH_FAIL)
               {
                  BDBG_ERR(("***BINARY SEARCH FAILED!"));
                  BWFE_g2_Adc_P_DacLinSearch(h, hChn->equCurrSlice, 0, target);  /* dac linear search for ping if binary search fails */
               }
            #endif
               BWFE_g2_Adc_P_GetGainError(h, hChn->equCurrSlice, 0, hChn->equCurrMdac, &(hChn->equGainErrPing));  /* note gain err scaled up by 4096 * 256 */

               retCode = BWFE_g2_Adc_P_DacBinSearch(h, hChn->equCurrSlice, 1, target);  /* dac binary search for pong */
            #ifdef BWFE_LINEAR_SEARCH_BACKUP
               if (retCode == BWFE_ERR_DAC_BIN_SEARCH_FAIL)
               {
                  BDBG_ERR(("***BINARY SEARCH FAILED!"));
                  BWFE_g2_Adc_P_DacLinSearch(h, hChn->equCurrSlice, 1, target);  /* dac linear search for ping if binary search fails */
               }
            #endif
               BWFE_g2_Adc_P_GetGainError(h, hChn->equCurrSlice, 1, hChn->equCurrMdac, &(hChn->equGainErrPong));  /* note gain err scaled up by 4096 * 256 */

               BWFE_DEBUG_EQU(BDBG_MSG(("PING gain err=%08X", hChn->equGainErrPing)));
               BWFE_DEBUG_EQU(BDBG_MSG(("PONG gain err=%08X", hChn->equGainErrPong)));

               /* cancel dc offset */
               BWFE_g2_Adc_P_ResetLut(h, hChn->equCurrSlice, hChn->equCurrMdac);
               retCode = BWFE_g2_Adc_P_DacBinSearch(h, hChn->equCurrSlice, 0, target);  /* redo dac binary search for ping only */
            #ifdef BWFE_LINEAR_SEARCH_BACKUP
               if (retCode == BWFE_ERR_DAC_BIN_SEARCH_FAIL)
               {
                  BDBG_ERR(("***BINARY SEARCH FAILED!"));
                  BWFE_g2_Adc_P_DacLinSearch(h, hChn->equCurrSlice, 0, target);  /* dac linear search for ping if binary search fails */
               }
            #endif
               BWFE_g2_Adc_P_CancelDcOffset(h, hChn->equCurrSlice, hChn->equCurrMdac);

               /* enable MDAC equ mode and training sequence */
            #if (BCHP_CHIP==4550)
               BWFE_P_OrRegister(h, BCHP_WFE_ANA_WBADC_CAL_CNTL_IN, 0x80000000);
               BWFE_P_ReadModifyWriteRegister(h, BCHP_WFE_ANA_ADC_CNTL3, ~0x00003000, 1 << (hChn->equCurrMdac + 11));  /* enable mdac gain calibration */
            #elif (BCHP_CHIP==4538)
               BWFE_P_OrRegister(h, BCHP_WFE_ANA_WBADC_CAL_CNTL_IN, 1 << (28 + hChn->equCurrSlice));
               BWFE_P_ReadModifyWriteRegister(h, BCHP_WFE_ANA_ADC_CNTL3, ~0x00003000, 1 << (hChn->equCurrMdac + 11));  /* enable mdac gain calibration */
               BWFE_P_ReadModifyWriteRegister(h, BCHP_WFE_ANA_ADC_CNTL3, ~0x18000000, 1 << (29 - hChn->equCurrMdac));  /* release PN gen reset */
            #endif
               shift = (hChn->equCurrMdac - 1) * 5;
               BWFE_P_ReadModifyWriteRegister(h, BCHP_WFE_ANA_WBADC_CAL_CNTL_IN, ~(0xF << shift), 0xA << shift);  /* use PN1 as training sequence */

               /* update ping and pong coefficients */
               BWFE_P_OrSliceReg(h, hChn->equCurrSlice, BCHP_WFE_CORE_DGSLMS_SLC, 0x03000000);

               /* enable PN input for LMS and update coefficients for corresponding mdac */
               shift = 8 * (3 - hChn->equCurrMdac);
               mask = (hChn->equTapMask << 4) | 0x2;
               BWFE_P_ReadModifyWriteSliceReg(h, hChn->equCurrSlice, BCHP_WFE_CORE_DGSLMS_SLC, ~(0xF3 << shift), mask << shift);

               /* run for specified time */
               hChn->equState = 3;
               return BWFE_g2_P_EnableTimer(h, BWFE_g2_TimerSelect_e0, hChn->equRuntimeMs * 1000, BWFE_g2_Adc_P_EqualizeMdac1);
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

            /* disable equ mode and PN sequence */
            BWFE_P_AndRegister(h, BCHP_WFE_ANA_WBADC_CAL_CNTL_IN, ~0xF00001EF);  /* disable mdac gain calibration */
            BWFE_P_AndRegister(h, BCHP_WFE_ANA_ADC_CNTL3, ~0x18003000); /* re-assert PN gen reset */

            /* reset lut for ping and pong */
            BWFE_g2_Adc_P_ResetLut(h, hChn->equCurrSlice, hChn->equCurrMdac); /* since this dc offset only applies to equ mode */

            /* gain scale eq taps for both lanes */
            BWFE_g2_Adc_P_ScaleEqTaps(h, hChn->equCurrSlice, 0, hChn->equCurrMdac, hChn->equGainErrPing);
            BWFE_g2_Adc_P_ScaleEqTaps(h, hChn->equCurrSlice, 1, hChn->equCurrMdac, hChn->equGainErrPong);

            /* end of mdac loop */
            hChn->equCurrMdac--;
            hChn->equState = 2;
            break;

         case 4:
            /* power down caldac, revert termination resistance to 50K */
         #if (BCHP_CHIP==4550)
            BWFE_P_OrRegister(h, BCHP_WFE_ANA_WBADC_TOP_CNTL_IN, 0x18000200);
         #elif (BCHP_CHIP==4538)
            BWFE_P_OrRegister(h, BCHP_WFE_ANA_WBADC_TOP_CNTL_IN, 0x18000040);
         #endif

            /* power up RFFE if not reference adc */
            if (!h->bReference)
               retCode = BWFE_g2_Rffe_P_PowerUp(h);
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
 BWFE_g2_Adc_P_ResetLut()
******************************************************************************/
BERR_Code BWFE_g2_Adc_P_ResetLut(BWFE_ChannelHandle h, uint8_t slice, uint8_t mdac)
{
   uint32_t mask = 0xFFF >> (mdac - 1);;
   uint8_t shift = (BWFE_NUM_MDACS - mdac + 1) * 10 - (mdac - 1);;

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
 BWFE_g2_Adc_P_ResetLutAll()
******************************************************************************/
BERR_Code BWFE_g2_Adc_P_ResetLutAll(BWFE_ChannelHandle h)
{
   uint8_t slice, mdac;

   for (slice = 0; slice < h->totalSlices; slice++)
   {
      for (mdac = BWFE_NUM_MDACS; mdac > 0; mdac--)
         BWFE_g2_Adc_P_ResetLut(h, slice, mdac);
   }
   return BERR_SUCCESS;
}


/******************************************************************************
 BWFE_g2_Adc_P_ResetEquTaps()
******************************************************************************/
BERR_Code BWFE_g2_Adc_P_ResetEquTaps(BWFE_ChannelHandle h)
{
   BWFE_g2_P_ChannelHandle *hChn = (BWFE_g2_P_ChannelHandle *)h->pImpl;
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
   BWFE_g2_Adc_P_ReadEquTaps(h);

   return retCode;
}


/******************************************************************************
 BWFE_g2_Adc_P_ReadEquTaps()
******************************************************************************/
BERR_Code BWFE_g2_Adc_P_ReadEquTaps(BWFE_ChannelHandle h)
{
   BWFE_g2_P_ChannelHandle *hChn = (BWFE_g2_P_ChannelHandle *)h->pImpl;
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
            hChn->equCoeff[slice][0][mdac-1][tap] = BWFE_g2_Adc_P_CoeffTc2Signed(val);
         }

         /* select pong lane for dgs coeff */
         BWFE_P_ReadModifyWriteSliceReg(h, slice, BCHP_WFE_CORE_DGSCOEFEN_SLC, ~0x00000003, 0x00000002);
         BWFE_P_ToggleSliceBit(h, slice, BCHP_WFE_CORE_DGSCOEFEN_SLC, 0x00000008);  /* toggle capture bit */
         BWFE_P_WriteSliceRegister(h, slice, BCHP_WFE_CORE_DGSCOEFA_SLC, (mdac - 1) * 4);   /* auto-incrementing address */
         for (tap = 0; tap < BWFE_NUM_EQ_TAPS; tap++)
         {
            /* read equalizer coefficients */
            BWFE_P_ReadSliceRegister(h, slice, BCHP_WFE_CORE_DGSCOEFD_SLC, &val);
            hChn->equCoeff[slice][1][mdac-1][tap] = BWFE_g2_Adc_P_CoeffTc2Signed(val);
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
 BWFE_g2_Adc_P_WriteEquTaps()
******************************************************************************/
BERR_Code BWFE_g2_Adc_P_WriteEquTaps(BWFE_ChannelHandle h)
{
   BWFE_g2_P_ChannelHandle *hChn = (BWFE_g2_P_ChannelHandle *)h->pImpl;
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
            val = BWFE_g2_Adc_P_CoeffSigned2Tc(hChn->equCoeff[slice][0][mdac-1][tap]);
            BWFE_P_WriteSliceRegister(h, slice, BCHP_WFE_CORE_DGSCOEFD_SLC, val);
         }
         BWFE_P_ToggleSliceBit(h, slice, BCHP_WFE_CORE_DGSCOEFEN_SLC, 0x00000004);  /* toggle load bit */

         /* select pong lane for dgs coeff */
         BWFE_P_ReadModifyWriteSliceReg(h, slice, BCHP_WFE_CORE_DGSCOEFEN_SLC, ~0x00000003, 0x00000002);
         BWFE_P_WriteSliceRegister(h, slice, BCHP_WFE_CORE_DGSCOEFA_SLC, (mdac - 1) * 4);   /* auto-incrementing address */
         for (tap = 0; tap < BWFE_NUM_EQ_TAPS; tap++)
         {
            /* write equalizer coefficients */
            val = BWFE_g2_Adc_P_CoeffSigned2Tc(hChn->equCoeff[slice][1][mdac-1][tap]);
            BWFE_P_WriteSliceRegister(h, slice, BCHP_WFE_CORE_DGSCOEFD_SLC, val);
         }
         BWFE_P_ToggleSliceBit(h, slice, BCHP_WFE_CORE_DGSCOEFEN_SLC, 0x00000004);  /* toggle load bit */

         /* deselect lanes after write */
         BWFE_P_AndSliceReg(h, slice, BCHP_WFE_CORE_DGSCOEFEN_SLC, ~0x00000003);
      }
   }

   return retCode;
}

