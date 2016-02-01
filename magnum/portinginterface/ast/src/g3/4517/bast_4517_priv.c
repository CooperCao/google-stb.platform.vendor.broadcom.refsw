/***************************************************************************
 *     Copyright (c) 2003-2014, Broadcom Corporation
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
#include "bast_g3.h"
#include "bast_g3_priv.h"
#include "bast_4517_priv.h"
#include "bchp_tm.h"


BDBG_MODULE(bast_4517_priv);


/* local functions */
void BAST_g3_P_GetRegisterAddress(BAST_ChannelHandle h, uint32_t reg, uint32_t *pAddr);

const uint32_t BAST_g3_ChannelIntrID[BAST_G3_MAX_CHANNELS][BAST_g3_MaxIntID] = {
   /* channel 0 interrupts */
   {
      BCHP_INT_ID_SDS_LOCK_IS_0,
      BCHP_INT_ID_SDS_NOT_LOCK_IS_0,
      BCHP_INT_ID_SDS_BTM_IS_0,
      BCHP_INT_ID_SDS_BRTM_IS_0,
      BCHP_INT_ID_SDS_GENTM_IS1_0,
      BCHP_INT_ID_SDS_GENTM_IS2_0,
      BCHP_INT_ID_SDS_GENTM_IS3_0,
      BCHP_INT_ID_DISEQC_TIMER1,
      BCHP_INT_ID_DISEQC_TIMER2,
      BCHP_INT_ID_SDS_sar_vol_gt_hi_thrsh,
      BCHP_INT_ID_SDS_sar_vol_lt_lo_thrsh,
      BCHP_INT_ID_DSDN_IS,
      BCHP_INT_ID_DISEQC_tx_fifo_a_empty_0,
      BCHP_INT_ID_SDS_HP_IS_0,
      BCHP_INT_ID_MI2C_IS_0,
      BCHP_INT_ID_TURBO_LOCK_IS_0,
      BCHP_INT_ID_TURBO_NOT_LOCK_IS_0,
      BCHP_INT_ID_TURBO_SYNC_0,
      0,
      0,
      0,
      0
   },
   /* channel 1 interrupts */
   {
      BCHP_INT_ID_SDS_LOCK_IS_1,
      BCHP_INT_ID_SDS_NOT_LOCK_IS_1,
      BCHP_INT_ID_SDS_BTM_IS_1,
      BCHP_INT_ID_SDS_BRTM_IS_1,
      BCHP_INT_ID_SDS_GENTM_IS1_1,
      BCHP_INT_ID_SDS_GENTM_IS2_1,
      BCHP_INT_ID_SDS_GENTM_IS3_1,
      0,
      0,
      0,
      0,
      0,
      0,
      BCHP_INT_ID_SDS_HP_IS_1,
      BCHP_INT_ID_MI2C_IS_1,
      BCHP_INT_ID_TURBO_LOCK_IS_1,
      BCHP_INT_ID_TURBO_NOT_LOCK_IS_1,
      BCHP_INT_ID_TURBO_SYNC_1,
      0,
      0,
      0,
      0
   },
   /* channel 2 interrupts */
   {
      BCHP_INT_ID_SDS_LOCK_IS_2,
      BCHP_INT_ID_SDS_NOT_LOCK_IS_2,
      BCHP_INT_ID_SDS_BTM_IS_2,
      BCHP_INT_ID_SDS_BRTM_IS_2,
      BCHP_INT_ID_SDS_GENTM_IS1_2,
      BCHP_INT_ID_SDS_GENTM_IS2_2,
      BCHP_INT_ID_SDS_GENTM_IS3_2,
      0,
      0,
      0,
      0,
      0,
      0,
      BCHP_INT_ID_SDS_HP_IS_2,
      BCHP_INT_ID_MI2C_IS_2,
      BCHP_INT_ID_TURBO_LOCK_IS_2,
      BCHP_INT_ID_TURBO_NOT_LOCK_IS_2,
      BCHP_INT_ID_TURBO_SYNC_2,
      0,
      0,
      0,
      0
   }
};


/******************************************************************************
 BAST_g3_P_InitHandle()
******************************************************************************/
BERR_Code BAST_g3_P_InitHandle(BAST_Handle h)
{
   BAST_g3_P_Handle *hDev = (BAST_g3_P_Handle *)(h->pImpl);

   hDev->xtalFreq = BAST_G3_XTAL_FREQ;
   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_g3_P_InitConfig()
******************************************************************************/
BERR_Code BAST_g3_P_InitConfig(BAST_ChannelHandle h)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   BAST_g3_P_Handle *hDev = (BAST_g3_P_Handle *)h->pDevice->pImpl;
   BERR_Code retCode = BERR_SUCCESS;

   if (h->channel >= BAST_G3_MAX_CHANNELS)
   {
      BERR_TRACE(retCode = BERR_INVALID_PARAMETER);
      BDBG_ASSERT(0);
   }
   else
   {
      hChn->bHasDiseqc = (h->channel == 0) ? true : false;
      hChn->bExternalTuner = false;
      hChn->bHasTunerRefPll = (h->channel % 2) ? true : false;
      if (h->channel == 1)
         hChn->tunerRefPllChannel = 0;
      else
         hChn->tunerRefPllChannel = h->channel;
      hChn->bHasAfec = false;
      hChn->bHasTfec = true;

      if (h->channel == 2)
         hChn->tunerCtl = BAST_G3_CONFIG_TUNER_CTL_LNA_PGA_MODE;

      hChn->xportSettings.bSerial = true;
      hChn->xportSettings.bClkInv = true;
      hChn->xportSettings.bClkSup = true;
      hChn->xportSettings.bVldInv = false;
      hChn->xportSettings.bSyncInv = false;
      hChn->xportSettings.bErrInv = false;
      hChn->xportSettings.bXbert = false;
      hChn->xportSettings.bTei = true;
      hChn->xportSettings.bDelay = false;
      hChn->xportSettings.bSync1 = false;
      hChn->xportSettings.bHead4 = false;
      hChn->xportSettings.bDelHeader = false;
      hChn->xportSettings.bOpllBypass = false;
      hChn->xportSettings.bchMpegErrorMode = BAST_BchMpegErrorMode_eBchAndCrc8;

      hChn->acqParams.mode = BAST_Mode_eDvb_scan;
      hChn->tunerFreq = 950000000UL;
      hChn->miscCtl = 0; /*BAST_G3_CONFIG_MISC_CTL_DISABLE_SMART_TUNE;*/
      hChn->bFsNotDefault = false;
      if (hDev->bOpen == false)
         hChn->sampleFreq = BAST_DEFAULT_SAMPLE_FREQ;
   }
   return retCode;
}


/******************************************************************************
 BAST_g3_P_GetRegisterAddress_isrsafe()
******************************************************************************/
void BAST_g3_P_GetRegisterAddress_isrsafe(BAST_ChannelHandle h, uint32_t reg, uint32_t *pAddr)
{
   *pAddr = reg;

   if (h->channel > 0)
   {
      if ((reg >= 0xA8000) && (reg <= 0xA8CFF))
      {
         /* SDS register access */
         *pAddr += (h->channel * 0x8000);
      }
      else if ((reg >= 0xAC000) && (reg <= 0xAC3FF))
      {
         /* TFEC register access */
         *pAddr += (h->channel * 0x8000);

      }
   }
}



/******************************************************************************
 BAST_g3_P_ReadRegister_isrsafe()
******************************************************************************/
BERR_Code BAST_g3_P_ReadRegister_isrsafe(
   BAST_ChannelHandle h,     /* [in] BAST channel handle */
   uint32_t           reg,   /* [in] address of register to read */
   uint32_t           *val   /* [out] contains data that was read */
)
{
   uint32_t addr;

   BAST_g3_P_GetRegisterAddress_isrsafe(h, reg, &addr);
   *val = BREG_Read32(0, addr);
   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_g3_P_WriteRegister_isrsafe()
******************************************************************************/
BERR_Code BAST_g3_P_WriteRegister_isrsafe(
   BAST_ChannelHandle h,     /* [in] BAST channel handle */
   uint32_t           reg,   /* [in] address of register to write */
   uint32_t           *val   /* [in] contains data to be written */
)
{
   uint32_t addr, wait_time;

   BAST_g3_P_GetRegisterWriteWaitTime_isrsafe(h, reg, &wait_time);
   BAST_g3_P_GetRegisterAddress_isrsafe(h, reg, &addr);

   BREG_Write32(0, addr, *val);
   if (wait_time > 0)
      BKNI_Delay(wait_time);

   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_g3_P_GetSysPllVcoFreq_isrsafe()
******************************************************************************/
void BAST_g3_P_GetSysPllVcoFreq_isrsafe(BAST_Handle h, uint32_t *Fclk_sys_pll_vco)
{
   BAST_g3_P_Handle *hDev = (BAST_g3_P_Handle*)h->pImpl;
   uint32_t P_hi, P_lo, Q_hi, Pdiv, Ndiv_int, Ndiv_frac;

   /*
      Fclk_sys_pll_vco = Fclk_xtal * (1 / Pdiv) * (Ndiv_int + (Ndiv_frac / 2^20))
      where Fclk_xtal = 54MHz, Pdiv = 2 (default),
            Ndiv_int = 80 (default), and
            Ndiv_frac = 0 (default)
      So the default Fclk_sys_pll_vco freq is:
      Fclk_sys_pll_vco = 54 MHz * (1 / 2) * (80 + (0 / 2^20)) = 2160MHz
   */
   Pdiv = BREG_Read32(0, BCHP_TM_SYS_PLL_PDIV) & BCHP_TM_SYS_PLL_PDIV_DIV_MASK;
   Ndiv_int = BREG_Read32(0, BCHP_TM_SYS_PLL_NDIV_INT) & BCHP_TM_SYS_PLL_NDIV_INT_DIV_MASK;
   Ndiv_frac = BREG_Read32(0, BCHP_TM_SYS_PLL_NDIV_FRAC) & BCHP_TM_SYS_PLL_NDIV_FRAC_DIV_MASK;

   /* deal with fractional Ndiv later since it complicates the calculation */
   if (Ndiv_frac != 0)
   {
      BDBG_ERR(("TM_SYS_PLL_NDIV_FRAC is non-zero!"));
      BDBG_ASSERT(0);
   }

   BMTH_HILO_32TO64_Mul(hDev->xtalFreq, Ndiv_int, &P_hi, &P_lo);
   BMTH_HILO_64TO64_Div32(P_hi, P_lo, Pdiv, &Q_hi, Fclk_sys_pll_vco);
}


/******************************************************************************
 BAST_g3_P_GetAnaPllVcoFreq_isrsafe()
******************************************************************************/
void BAST_g3_P_GetAnaPllVcoFreq_isrsafe(BAST_Handle h, uint32_t *Fclk_ana_pll_vco)
{
   BAST_g3_P_Handle *hDev = (BAST_g3_P_Handle*)h->pImpl;
   uint32_t P_hi, P_lo, Q_hi, Pdiv, Ndiv_int, Ndiv_frac;

   /*
      Fclk_ana_pll_vco = Fclk_xtal * (1 / Pdiv) * (Ndiv_int + (Ndiv_frac / 2^20))
      where Fclk_xtal = 54MHz, Pdiv = 2 (default),
            Ndiv_int = 80 (default), and
            Ndiv_frac = 0 (default)
      So the default Fclk_sys_pll_vco freq is:
      Fclk_ana_pll_vco = 54 MHz * (1 / 2) * (82 + (0 / 2^20)) = 2214MHz
   */
   Pdiv = BREG_Read32(0, BCHP_TM_ANA_PLL_PDIV) & BCHP_TM_ANA_PLL_PDIV_DIV_MASK;
   Ndiv_int = BREG_Read32(0, BCHP_TM_ANA_PLL_NDIV_INT) & BCHP_TM_ANA_PLL_NDIV_INT_DIV_MASK;
   Ndiv_frac = BREG_Read32(0, BCHP_TM_ANA_PLL_NDIV_FRAC) & BCHP_TM_ANA_PLL_NDIV_FRAC_DIV_MASK;

   /* deal with fractional Ndiv later since it complicates the calculation */
   if (Ndiv_frac != 0)
   {
      BDBG_ERR(("TM_ANA_PLL_NDIV_FRAC is non-zero!"));
      BDBG_ASSERT(0);
   }

   BMTH_HILO_32TO64_Mul(hDev->xtalFreq, Ndiv_int, &P_hi, &P_lo);
   BMTH_HILO_64TO64_Div32(P_hi, P_lo, Pdiv, &Q_hi, Fclk_ana_pll_vco);
}


/******************************************************************************
 BAST_g3_P_GetTfecClock_isrsafe() - updates fecFreq
******************************************************************************/
BERR_Code BAST_g3_P_GetTfecClock_isrsafe(BAST_ChannelHandle h, uint32_t *pFreq)
{
   uint32_t tfec_div, Fclk_sys_pll_vco;

   tfec_div = BREG_Read32(0, BCHP_TM_SYS_PLL_CLK_TFEC) & BCHP_TM_SYS_PLL_CLK_TFEC_DIV_MASK;

   BAST_g3_P_GetSysPllVcoFreq_isrsafe(h->pDevice, &Fclk_sys_pll_vco);
   *pFreq = Fclk_sys_pll_vco / tfec_div;
   /* BDBG_MSG(("TFEC clock=%u", *pFreq)); */
   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_g3_P_IsSdsOn()
******************************************************************************/
bool BAST_g3_P_IsSdsOn(BAST_ChannelHandle h)
{
   uint32_t val;

   BAST_g3_P_ReadRegister_isrsafe(h, BCHP_TM_SYS_CLK_EN, &val);
   val = (val >> 10) & 0x7;

   if (val & (1 << h->channel))
      return true;
   else
      return false;
}


/******************************************************************************
 BAST_g3_P_SdsPowerUp() - power up the sds core
******************************************************************************/
BERR_Code BAST_g3_P_SdsPowerUp(BAST_ChannelHandle h)
{
   /* enable 108 MHz register clock */
   BAST_g3_P_OrRegister_isrsafe(h, BCHP_TM_REG_CLK_EN, (1 << (h->channel + 4)));

   /* power up SDS */
   BAST_g3_P_OrRegister_isrsafe(h, BCHP_TM_SYS_CLK_EN, (1 << (h->channel + 10)));

   /* power up SDS memories */
   BAST_g3_P_AndRegister_isrsafe(h, BCHP_TM_MEM_CTRL0, ~(0x5 << (h->channel * 3)));
   BKNI_Sleep(2);
   BAST_g3_P_AndRegister_isrsafe(h, BCHP_TM_MEM_CTRL0, ~(0x2 << (h->channel * 3)));

   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_g3_P_SdsPowerDown() - power down the sds core
******************************************************************************/
BERR_Code BAST_g3_P_SdsPowerDown(BAST_ChannelHandle h)
{
   BERR_Code retCode;
   uint32_t val;

   BAST_ChannelHandle hChn0 = (BAST_ChannelHandle)(h->pDevice->pChannels[0]);

   /* turn off oif */
   retCode = BAST_g3_P_SdsDisableOif_isrsafe(h);

   /* check tuner global power down */
   BAST_g3_P_ReadRegister_isrsafe(hChn0, BCHP_SDS_TUNER_PWRUP_COMMON_R01, &val);

   if ((h->channel != 0) || ((val & 1) == 0))
   {
      /* power down SDS memories first before TM */
      BAST_g3_P_OrRegister_isrsafe(h, BCHP_TM_MEM_CTRL0, (0x7 << (h->channel * 3)));

      /* power down SDS */
      BAST_g3_P_AndRegister_isrsafe(h, BCHP_TM_SYS_CLK_EN, ~(1 << (h->channel + 10)));

      /* disable 108 MHz register clock */
      BAST_g3_P_AndRegister_isrsafe(h, BCHP_TM_REG_CLK_EN, ~(1 << (h->channel + 4)));
   }

   if ((h->channel != 0) && ((val & 1) == 0))
   {
      /* power down channel 0 last */
      /* power down SDS memories first before TM */
      BAST_g3_P_OrRegister_isrsafe(h, BCHP_TM_MEM_CTRL0, (0x7 << (0 * 3)));

      /* power down SDS */
      BAST_g3_P_AndRegister_isrsafe(h, BCHP_TM_SYS_CLK_EN, ~(1 << (0 + 10)));

      /* disable 108 MHz register clock */
      BAST_g3_P_AndRegister_isrsafe(h, BCHP_TM_REG_CLK_EN, ~(1 << (0 + 4)));
   }

   return retCode;
}


/******************************************************************************
 BAST_g3_P_IsTurboOn_isrsafe()
******************************************************************************/
bool BAST_g3_P_IsTurboOn_isrsafe(BAST_ChannelHandle h)
{
   uint32_t val;

   BAST_g3_P_ReadRegister_isrsafe(h, BCHP_TM_SYS_CLK_EN, &val);
   val = (val >> 7) & 0x7;

   if (val & (1 << h->channel))
      return true;
   else
      return false;
}


/******************************************************************************
 BAST_g3_P_TurboPowerUp_isr() - power up the tfec core
******************************************************************************/
BERR_Code BAST_g3_P_TurboPowerUp_isr(BAST_ChannelHandle h)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode;

   /* make sure system pll clock for turbo is powered up */
   if ((BREG_Read32(NULL, BCHP_TM_SYS_PLL_CLK_TFEC) & 0x300) != 0x300)
      BAST_g3_P_OrRegister_isrsafe(h, BCHP_TM_SYS_PLL_CLK_TFEC, 0x300);

   /* power up TFEC */
   BAST_g3_P_OrRegister_isrsafe(h, BCHP_TM_SYS_CLK_EN, (1 << (h->channel + 7)));

   /* power up TFEC memories */
   BAST_g3_P_AndRegister_isrsafe(h, BCHP_TM_MEM_CTRL1, ~(0x5 << (h->channel * 3)));
   BKNI_Sleep(2);
   BAST_g3_P_AndRegister_isrsafe(h, BCHP_TM_MEM_CTRL1, ~(0x2 << (h->channel * 3)));

   if ((retCode = BAST_g3_P_GetTfecClock_isrsafe(h, &(hChn->fecFreq))) != BERR_SUCCESS)
   {
      BERR_TRACE(retCode = BAST_ERR_HAB_CMD_FAIL);
      BDBG_ERR(("BAST_g3_P_TurboPowerUp_isr() failed"));
   }

   return retCode;
}


/******************************************************************************
 BAST_g3_P_TurboPowerDown() - power down the tfec core
******************************************************************************/
BERR_Code BAST_g3_P_TurboPowerDown(BAST_ChannelHandle h)
{
   BERR_Code retCode;
   uint32_t val;

   /* power down TFEC memories before TM */
   BAST_g3_P_OrRegister_isrsafe(h, BCHP_TM_MEM_CTRL1, (0x7 << (h->channel * 3)));

   /* power down TFEC */
   BAST_g3_P_AndRegister_isrsafe(h, BCHP_TM_SYS_CLK_EN, ~(1 << (h->channel + 7)));

   /* power down system pll clock for turbo if all TFECs are powered down */
   val = BREG_Read32(NULL, BCHP_TM_SYS_CLK_EN);
   if ((val & 0x00000380) == 0)
   {
      BAST_g3_P_AndRegister_isrsafe(h, BCHP_TM_SYS_PLL_CLK_TFEC, ~0x300);
   }

   if (BAST_g3_P_IsTurboOn_isrsafe(h) == false)
      retCode = BERR_SUCCESS;
   else
   {
      BERR_TRACE(retCode = BAST_ERR_HAB_CMD_FAIL);
      BDBG_ERR(("BAST_g3_P_TurboPowerDown() failed"));
   }

   return retCode;
}
