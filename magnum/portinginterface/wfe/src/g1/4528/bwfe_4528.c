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

#include "bwfe.h"
#include "bwfe.h"
#include "bwfe_priv.h"
#include "bwfe_4528.h"

BDBG_MODULE(bwfe_4528);

#define BWFE_CALDAC_ACC_INTERVAL 255   /* interval is 0 to 255, 4096 samples per step */

#define BWFE_DEBUG_DAC(x) /* x */


/******************************************************************************
 BWFE_P_InitConfig()
******************************************************************************/
BERR_Code BWFE_P_InitConfig(BWFE_ChannelHandle h)
{
   BERR_Code retCode = BERR_SUCCESS;
   
   /* TBD */
   
   return retCode;
}


/******************************************************************************
 BWFE_P_ReadRegister()
******************************************************************************/
BERR_Code BWFE_P_ReadRegister(
   BWFE_ChannelHandle h,     /* [in] BWFE channel handle */
   uint32_t           reg,   /* [in] address of register to read */
   uint32_t           *val   /* [out] contains data that was read */
)
{
   *val = BREG_Read32(h->pDevice->hRegister, reg); 
   return BERR_SUCCESS;
}


/******************************************************************************
 BWFE_P_WriteRegister()
******************************************************************************/
BERR_Code BWFE_P_WriteRegister(
   BWFE_ChannelHandle h,     /* [in] BWFE channel handle */
   uint32_t           reg,   /* [in] address of register to write */
   uint32_t           val   /* [in] contains data to be written */
)
{
   BREG_Write32(h->pDevice->hRegister, reg, val);
   return BERR_SUCCESS;
}


/******************************************************************************
 BWFE_P_GetRegisterAddress()
******************************************************************************/
BERR_Code BWFE_P_GetLaneRegisterAddress(BWFE_ChannelHandle h, uint8_t lane, uint32_t reg, uint32_t *pAddr)
{
   *pAddr = reg;
   
   BSTD_UNUSED(h);
   BDBG_ASSERT(lane < BWFE_NUM_LANES+2);  /* 2 reference lanes */
   
   switch (reg)
   {
      case BCHP_WFE_CORE_DGSCTL_LN:
      case BCHP_WFE_CORE_DGSCLP_LN:
      case BCHP_WFE_CORE_DGSHIST_LN:
      case BCHP_WFE_CORE_DGSCLPCNT_LN:
      case BCHP_WFE_CORE_DGSACCUM_LN:
         *pAddr = reg + lane * 0x4;
         break;
      
      case BCHP_WFE_CORE_DGSLUT011_LN:
      case BCHP_WFE_CORE_DGSLUT010_LN:
      case BCHP_WFE_CORE_DGSLUT001_LN:
      case BCHP_WFE_CORE_DGSLUT000_LN:
      case BCHP_WFE_CORE_DGSLUT111_LN:
      case BCHP_WFE_CORE_DGSLUT110_LN:
      case BCHP_WFE_CORE_DGSLUT101_LN:
         *pAddr = reg + lane * 0x1C;
         break;
      
      default:
         return BERR_INVALID_PARAMETER;
   }
   
   return BERR_SUCCESS;
}


/******************************************************************************
 BWFE_P_ReadLaneRegister()
******************************************************************************/
BERR_Code BWFE_P_ReadLaneRegister(
   BWFE_ChannelHandle h,     /* [in] BWFE channel handle */
   uint8_t            lane,  /* [in] lane number */
   uint32_t           reg,   /* [in] address of register to read */
   uint32_t           *val   /* [out] contains data that was read */
)
{
   BERR_Code retCode = BERR_SUCCESS;
   uint32_t addr;
   
   if (lane >= BWFE_NUM_LANES+2)
      return BERR_INVALID_PARAMETER;
   
   /* look up lane register */
   BWFE_CHK_RETCODE(BWFE_P_GetLaneRegisterAddress(h, lane, reg, &addr));
   
   *val = BREG_Read32(h->pDevice->hRegister, addr); 
   
   done:
   return retCode;
}


/******************************************************************************
 BWFE_P_WriteLaneRegister()
******************************************************************************/
BERR_Code BWFE_P_WriteLaneRegister(
   BWFE_ChannelHandle h,     /* [in] BWFE channel handle */
   uint8_t            lane,  /* [in] lane number */
   uint32_t           reg,   /* [in] address of register to write */
   uint32_t           val    /* [in] contains data to be written */
)
{
   BERR_Code retCode = BERR_SUCCESS;
   uint32_t addr;
   
   if (lane >= BWFE_NUM_LANES+2)
      return BERR_INVALID_PARAMETER;
   
   /* look up lane register */
   BWFE_CHK_RETCODE(BWFE_P_GetLaneRegisterAddress(h, lane, reg, &addr));
   
   BREG_Write32(h->pDevice->hRegister, addr, val);
   
   done:
   return retCode;
}


/******************************************************************************
 BWFE_P_SetAdcClock()
******************************************************************************/
BERR_Code BWFE_P_SetAdcSampleFreq(BWFE_Handle h, uint32_t freqKhz)
{
   BERR_Code retCode = BERR_SUCCESS;
   uint32_t ndiv_int, val;
   
   /* limit frequencies range */
   if ((freqKhz < 4644000) || (freqKhz > 5076000))
      return BERR_INVALID_PARAMETER;
   
   /* limit possible frequencies based on 54MHz */
   if (freqKhz % (2 * BWFE_XTAL_FREQ_KHZ) != 0)
      return BERR_INVALID_PARAMETER;
   
   /* PLL initializiation sequence */
   /* 1) After power-up, make sure to reset the PLL by bit i_resetb and i_post_resetb. */
   /* 2) If post-divider channels are used, please use i_load_en_ch<2:0> to load the correct settings of the post-divider. */
   /* 3) Wait until the output bit o_lock becomes high  this indicates the PLL is locked. */
   
   BREG_Write32(h->hRegister, BCHP_TM_ANA_PLL_RST, 0x00000003);    /* pll global reset and post-div reset */
   BREG_Write32(h->hRegister, BCHP_TM_ANA_PLL_RST, 0x00000000);    /* release resets */
   
   /* calculate ndiv_int from freqKhz, ndiv_int = (Fs_adc) / (54MHz * 2) */
   ndiv_int = (freqKhz >> 1) / BWFE_XTAL_FREQ_KHZ;
   
   /* Fclk_ana_pll_vco = 54 MHz * (2 / Pdiv) * (Ndiv_int + (Ndiv_frac / 2^20)) */
   /* Example: Pdiv=1, Ndiv_int=46 -> Fclk_ana_pll_vco=4968Mhz */
   
   /* set pll dividers for rx clk */
   BREG_Write32(h->hRegister, BCHP_TM_ANA_PLL_NDIV_INT, ndiv_int & 0xFF);   /* Ndiv_int=46 default for 4968MHz */
   BREG_Write32(h->hRegister, BCHP_TM_ANA_PLL_NDIV_FRAC, 0x00000000);       /* Ndiv_frac=0 */
   
   /* wait for pll lock */
   BKNI_Sleep(5);
   
   /* check pll status */
   val = BREG_Read32(h->hRegister, BCHP_TM_ANA_PLL_STATUS);
   if ((val & 0x00010000) == 0)
      BDBG_ERR(("rx pll NOT locked!"));
   
   return retCode;
}


/******************************************************************************
 BWFE_P_GetAdcClock()
******************************************************************************/
BERR_Code BWFE_P_GetAdcSampleFreq(BWFE_Handle h, uint32_t *freqKhz)
{
   uint32_t pdiv;
   uint32_t ndiv_int;
   uint32_t ndiv_frac;
   uint32_t P_hi, P_lo, Q_hi, Q_lo;
   
   pdiv = BREG_Read32(h->hRegister, BCHP_TM_ANA_PLL_PDIV) & 0xF;
   ndiv_int = BREG_Read32(h->hRegister, BCHP_TM_ANA_PLL_NDIV_INT) & 0xFF;
   ndiv_frac = BREG_Read32(h->hRegister, BCHP_TM_ANA_PLL_NDIV_FRAC) & 0xFFFFF;
   
   /* Fclk_rx_pll_vco = 54 MHz * (2 / Pdiv) * (Ndiv_int + (Ndiv_frac / 2^20)) */
   /* equivalent to (54 MHz * 2) * (2^20 * Ndiv_int + Ndiv_frac) / (2^20 * Pdiv) */
   
   BMTH_HILO_32TO64_Mul(BWFE_XTAL_FREQ_KHZ << 1, (ndiv_int << 20) + ndiv_frac, &P_hi, &P_lo);  /* (54 MHz * 2) * (2^20 * Ndiv_int + Ndiv_frac) */
   BMTH_HILO_64TO64_Div32(P_hi, P_lo, pdiv << 20, &Q_hi, &Q_lo);  /* div by (2^20 * Pdiv) */
   
   BDBG_MSG(("Fs_adc=%d000", Q_lo));
   *freqKhz = Q_lo;
   return BERR_SUCCESS;
}


/******************************************************************************
 BWFE_P_ResetAdc()
******************************************************************************/
BERR_Code BWFE_P_ResetAdc(BWFE_ChannelHandle h)
{
   /* set phase for ADC0, 1, 2, 3 */
   BWFE_P_ReadModifyWriteRegister(h, BCHP_WFE_ANA_U7_WBADC_SYS_CNTL_IN, ~0x0007FF80, 0x00034400);
   
   BWFE_P_AndRegister(h, BCHP_WFE_ANA_U7_WBADC_SYS_CNTL_IN, ~0x00400000);  /* invert toggle reset */
   BWFE_P_OrRegister(h, BCHP_WFE_ANA_U7_WBADC_SYS_CNTL_IN, 0x00400000);    /* release reset */
   
   return BERR_SUCCESS;
}


/******************************************************************************
 BWFE_P_PowerDownRefLane()
******************************************************************************/
BERR_Code BWFE_P_PowerDownRefLane(BWFE_ChannelHandle h)
{
   BWFE_P_OrRegister(h, BCHP_WFE_ANA_U7_WBADC_SYS_CNTL_IN, 0x00000040);    /* ADC4 sleep mode */
   BWFE_P_AndRegister(h, BCHP_WFE_ANA_U7_WBADC_TOP_CNTL_IN, ~0x20000000);  /* disable averaging for ADC4 */
   
   return BERR_SUCCESS;
}


/******************************************************************************
 BWFE_P_PowerUpRefLane()
******************************************************************************/
BERR_Code BWFE_P_PowerUpRefLane(BWFE_ChannelHandle h)
{
   BWFE_P_AndRegister(h, BCHP_WFE_ANA_U7_WBADC_SYS_CNTL_IN, ~0x00000040);  /* ADC4 normal operation */
   BWFE_P_OrRegister(h, BCHP_WFE_ANA_U7_WBADC_TOP_CNTL_IN, 0x20000000);    /* enable averaging for ADC4 */
   
   return BERR_SUCCESS;
}


/******************************************************************************
 BWFE_P_SetRefPhase()
******************************************************************************/
BERR_Code BWFE_P_SetRefPhase(BWFE_ChannelHandle h, uint8_t lane)
{
   if (lane >= BWFE_NUM_LANES)
      return BERR_INVALID_PARAMETER;
   
   /* lane0: 0 deg, lane1: 45 deg, lane2: 90 deg, lane3: 135 deg, lane4: 180 deg, lane5: 225 deg, lane6: 270 deg, lane 7: 315 deg */
   BWFE_P_ReadModifyWriteRegister(h, BCHP_WFE_ANA_U7_WBADC_SYS_CNTL_IN, ~0x00380000, (lane & 0x7) << 19);   /* set ADC4 phase */

   /* skip one latency in ADC4 ping data for 180 or greater phase shift */
   if (lane < BWFE_NUM_LANES / 2)
      BWFE_P_AndRegister(h, BCHP_WFE_ANA_U7_WBADC_SYS_CNTL_IN, ~0x10000000);
   else
      BWFE_P_OrRegister(h, BCHP_WFE_ANA_U7_WBADC_SYS_CNTL_IN, 0x10000000);
   
   return BERR_SUCCESS;
}


/******************************************************************************
 BWFE_P_SetAdcCurrent()
******************************************************************************/
BERR_Code BWFE_P_SetAdcCurrent(BWFE_ChannelHandle h)
{
   /* use defaults for 4528 */
   return BERR_SUCCESS;
}


/******************************************************************************
 BWFE_P_CalibrateDAC()
******************************************************************************/
BERR_Code BWFE_P_CalibrateDAC(BWFE_ChannelHandle h, uint8_t lane, int32_t target)
{
   BERR_Code retCode = BERR_SUCCESS;
   int32_t avgUpper, avgLower;
   uint8_t msbUpper, msbLower, lsbUpper, lsbLower;
   uint8_t msbFinal, lsbFinal;
   
   BWFE_DEBUG_DAC(BDBG_MSG(("CalibrateDAC(lane%d): target=%d", lane, target)));
   
   lsbUpper = msbUpper = 31;
   lsbLower = msbLower = 0;
   
   /* search msb */
   while (msbUpper != msbLower)
   {
      /* get upper average */
      BWFE_P_ReadModifyWriteRegister(h, BCHP_WFE_ANA_U7_WBADC_TOP_CNTL_IN, ~0x003FF000, msbUpper << 17 | lsbUpper << 12);
      retCode = BWFE_Adc_P_ReadAccumulator(h, lane, BWFE_CALDAC_ACC_INTERVAL, &avgUpper);   /* set maximum accumulation length: 4096 samples per unit */
      avgUpper >>= 20;  /* div by acc length to get avg, scale back down by 4096 */
      if (retCode == BWFE_ERR_ACC_SATURATED)
      {
         BDBG_MSG(("CalibrateDAC: lane%d/msb%d/lsb%d accumulator saturated, value clipped!", lane, msbUpper, lsbUpper));
      }
      
      /* get lower average */
      BWFE_P_ReadModifyWriteRegister(h, BCHP_WFE_ANA_U7_WBADC_TOP_CNTL_IN, ~0x003FF000, msbLower << 17 | lsbLower << 12);
      retCode = BWFE_Adc_P_ReadAccumulator(h, lane, BWFE_CALDAC_ACC_INTERVAL, &avgLower);   /* set maximum accumulation length: 4096 samples per unit */
      avgLower >>= 20;  /* div by acc length to get avg, scale back down by 4096 */
      if (retCode == BWFE_ERR_ACC_SATURATED)
      {
         BDBG_MSG(("CalibrateDAC: lane%d/msb%d/lsb%d accumulator saturated, value clipped!", lane, msbLower, lsbLower));
      }
      
      BWFE_DEBUG_DAC(BDBG_MSG(("msbUpper=%d, msbLower=%d | avgUpper=%d, avgLower=%d", msbUpper, msbLower, avgUpper, avgLower)));
      
      /* continue binary search */
      if (target < ((avgUpper + avgLower) / 2))
         msbFinal = msbLower = (msbLower + msbUpper) / 2 + 1;
      else
         msbFinal = msbUpper = (msbLower + msbUpper) / 2;
   }
   
   /* set final msb */
   BWFE_DEBUG_DAC(BDBG_MSG(("msbFinal=%d", msbFinal)));
   BWFE_P_ReadModifyWriteRegister(h, BCHP_WFE_ANA_U7_WBADC_TOP_CNTL_IN, ~0x003E0000, msbFinal << 17);
   
   /* search lsb */
   while (lsbUpper != lsbLower)
   {
      /* get upper average */
      BWFE_P_ReadModifyWriteRegister(h, BCHP_WFE_ANA_U7_WBADC_TOP_CNTL_IN, ~0x0001F000, lsbUpper << 12);
      retCode = BWFE_Adc_P_ReadAccumulator(h, lane, BWFE_CALDAC_ACC_INTERVAL, &avgUpper);   /* set maximum accumulation length: 4096 samples per unit */
      avgUpper >>= 20;  /* div by acc length to get avg, scale back down by 4096 */
      if (retCode == BWFE_ERR_ACC_SATURATED)
      {
         BDBG_MSG(("CalibrateDAC: lane%d/msb%d/lsb%d accumulator saturated, value clipped!", lane, msbUpper, lsbUpper));
      }
      
      /* get lower average */
      BWFE_P_ReadModifyWriteRegister(h, BCHP_WFE_ANA_U7_WBADC_TOP_CNTL_IN, ~0x0001F000, lsbLower << 12);
      retCode = BWFE_Adc_P_ReadAccumulator(h, lane, BWFE_CALDAC_ACC_INTERVAL, &avgLower);   /* set maximum accumulation length: 4096 samples per unit */
      avgLower >>= 20;  /* div by acc length to get avg, scale back down by 4096 */
      if (retCode == BWFE_ERR_ACC_SATURATED)
      {
         BDBG_MSG(("CalibrateDAC: lane%d/msb%d/lsb%d accumulator saturated, value clipped!", lane, msbLower, lsbLower));
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
         if (target < (avgUpper + avgLower) / 2)
            lsbFinal = lsbLower = (lsbLower + lsbUpper) / 2 + 1;
         else
            lsbFinal = lsbUpper = (lsbLower + lsbUpper) / 2;
      }
   }
   
   /* set final lsb */
   BWFE_DEBUG_DAC(BDBG_MSG(("lsbFinal=%d", lsbFinal)));
   BWFE_P_ReadModifyWriteRegister(h, BCHP_WFE_ANA_U7_WBADC_TOP_CNTL_IN, ~0x0001F000, lsbFinal << 12);
   
   /* get final average */
   retCode = BWFE_Adc_P_ReadAccumulator(h, lane, BWFE_CALDAC_ACC_INTERVAL, &avgLower);
   avgLower >>= 20;  /* div by acc length to get avg, scale back down by 4096 */
   BWFE_DEBUG_DAC(BDBG_MSG(("avgFinal=%d", avgLower)));
   
   return retCode;
}

