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
#include "bast_g3.h"
#include "bast_g3_priv.h"
#include "bast_4538_priv.h"
#include "bchp_tm.h"
#include "bchp_stb_chan_ch0.h"
#include "bchp_stb_chan_ctrl.h"


#if (BCHP_CHIP != 4538)
#error "This file is for BCM4538 firmware only (not for host software)"
#endif

BDBG_MODULE(bast_4538_priv);


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
      BCHP_INT_ID_DISEQC_TIMER1_0,
      BCHP_INT_ID_DISEQC_TIMER2_0,
      BCHP_INT_ID_SDS_sar_vol_gt_hi_thrsh_0,
      BCHP_INT_ID_SDS_sar_vol_lt_lo_thrsh_0,
      BCHP_INT_ID_DSDN_IS_0,
      BCHP_INT_ID_DISEQC_tx_fifo_a_empty_0,
      BCHP_INT_ID_SDS_HP_IS_0,
      BCHP_INT_ID_MI2C_IS_0,
      BCHP_INT_ID_TURBO_LOCK_IS_0,
      BCHP_INT_ID_TURBO_NOT_LOCK_IS_0,
      BCHP_INT_ID_TURBO_SYNC_0,
      BCHP_INT_ID_AFEC_LOCK_IS_0,
      BCHP_INT_ID_AFEC_NOT_LOCK_IS_0,
      BCHP_INT_ID_HP_FRAME_BOUNDARY_0,
      BCHP_INT_ID_DFT_DONE_0,
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
      BCHP_INT_ID_AFEC_LOCK_IS_1,
      BCHP_INT_ID_AFEC_NOT_LOCK_IS_1,
      BCHP_INT_ID_HP_FRAME_BOUNDARY_1,
      BCHP_INT_ID_DFT_DONE_1,
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
      BCHP_INT_ID_AFEC_LOCK_IS_2,
      BCHP_INT_ID_AFEC_NOT_LOCK_IS_2,
      BCHP_INT_ID_HP_FRAME_BOUNDARY_2,
      BCHP_INT_ID_DFT_DONE_2,
   },
   /* channel 3 interrupts */
   {
      BCHP_INT_ID_SDS_LOCK_IS_3,
      BCHP_INT_ID_SDS_NOT_LOCK_IS_3,
      BCHP_INT_ID_SDS_BTM_IS_3,
      BCHP_INT_ID_SDS_BRTM_IS_3,
      BCHP_INT_ID_SDS_GENTM_IS1_3,
      BCHP_INT_ID_SDS_GENTM_IS2_3,
      BCHP_INT_ID_SDS_GENTM_IS3_3,
      0,
      0,
      0,
      0,
      0,
      0,
      BCHP_INT_ID_SDS_HP_IS_3,
      BCHP_INT_ID_MI2C_IS_3,
      BCHP_INT_ID_TURBO_LOCK_IS_3,
      BCHP_INT_ID_TURBO_NOT_LOCK_IS_3,
      BCHP_INT_ID_TURBO_SYNC_3,
      BCHP_INT_ID_AFEC_LOCK_IS_3,
      BCHP_INT_ID_AFEC_NOT_LOCK_IS_3,
      BCHP_INT_ID_HP_FRAME_BOUNDARY_3,
      BCHP_INT_ID_DFT_DONE_3,
   },
   /* channel 4 interrupts */
   {
      BCHP_INT_ID_SDS_LOCK_IS_4,
      BCHP_INT_ID_SDS_NOT_LOCK_IS_4,
      BCHP_INT_ID_SDS_BTM_IS_4,
      BCHP_INT_ID_SDS_BRTM_IS_4,
      BCHP_INT_ID_SDS_GENTM_IS1_4,
      BCHP_INT_ID_SDS_GENTM_IS2_4,
      BCHP_INT_ID_SDS_GENTM_IS3_4,
      0,
      0,
      0,
      0,
      0,
      0,
      BCHP_INT_ID_SDS_HP_IS_4,
      BCHP_INT_ID_MI2C_IS_4,
      BCHP_INT_ID_TURBO_LOCK_IS_4,
      BCHP_INT_ID_TURBO_NOT_LOCK_IS_4,
      BCHP_INT_ID_TURBO_SYNC_4,
      BCHP_INT_ID_AFEC_LOCK_IS_4,
      BCHP_INT_ID_AFEC_NOT_LOCK_IS_4,
      BCHP_INT_ID_HP_FRAME_BOUNDARY_4,
      BCHP_INT_ID_DFT_DONE_4,
   },
   /* channel 5 interrupts */
   {
      BCHP_INT_ID_SDS_LOCK_IS_5,
      BCHP_INT_ID_SDS_NOT_LOCK_IS_5,
      BCHP_INT_ID_SDS_BTM_IS_5,
      BCHP_INT_ID_SDS_BRTM_IS_5,
      BCHP_INT_ID_SDS_GENTM_IS1_5,
      BCHP_INT_ID_SDS_GENTM_IS2_5,
      BCHP_INT_ID_SDS_GENTM_IS3_5,
      0,
      0,
      0,
      0,
      0,
      0,
      BCHP_INT_ID_SDS_HP_IS_5,
      BCHP_INT_ID_MI2C_IS_5,
      BCHP_INT_ID_TURBO_LOCK_IS_5,
      BCHP_INT_ID_TURBO_NOT_LOCK_IS_5,
      BCHP_INT_ID_TURBO_SYNC_5,
      BCHP_INT_ID_AFEC_LOCK_IS_5,
      BCHP_INT_ID_AFEC_NOT_LOCK_IS_5,
      BCHP_INT_ID_HP_FRAME_BOUNDARY_5,
      BCHP_INT_ID_DFT_DONE_5,
   },
   /* channel 6 interrupts */
   {
      BCHP_INT_ID_SDS_LOCK_IS_6,
      BCHP_INT_ID_SDS_NOT_LOCK_IS_6,
      BCHP_INT_ID_SDS_BTM_IS_6,
      BCHP_INT_ID_SDS_BRTM_IS_6,
      BCHP_INT_ID_SDS_GENTM_IS1_6,
      BCHP_INT_ID_SDS_GENTM_IS2_6,
      BCHP_INT_ID_SDS_GENTM_IS3_6,
      0,
      0,
      0,
      0,
      0,
      0,
      BCHP_INT_ID_SDS_HP_IS_6,
      BCHP_INT_ID_MI2C_IS_6,
      BCHP_INT_ID_TURBO_LOCK_IS_6,
      BCHP_INT_ID_TURBO_NOT_LOCK_IS_6,
      BCHP_INT_ID_TURBO_SYNC_6,
      BCHP_INT_ID_AFEC_LOCK_IS_6,
      BCHP_INT_ID_AFEC_NOT_LOCK_IS_6,
      BCHP_INT_ID_HP_FRAME_BOUNDARY_6,
      BCHP_INT_ID_DFT_DONE_6,
   },
   /* channel 7 interrupts */
   {
      BCHP_INT_ID_SDS_LOCK_IS_7,
      BCHP_INT_ID_SDS_NOT_LOCK_IS_7,
      BCHP_INT_ID_SDS_BTM_IS_7,
      BCHP_INT_ID_SDS_BRTM_IS_7,
      BCHP_INT_ID_SDS_GENTM_IS1_7,
      BCHP_INT_ID_SDS_GENTM_IS2_7,
      BCHP_INT_ID_SDS_GENTM_IS3_7,
      0,
      0,
      0,
      0,
      0,
      0,
      BCHP_INT_ID_SDS_HP_IS_7,
      BCHP_INT_ID_MI2C_IS_7,
      BCHP_INT_ID_TURBO_LOCK_IS_7,
      BCHP_INT_ID_TURBO_NOT_LOCK_IS_7,
      BCHP_INT_ID_TURBO_SYNC_7,
      BCHP_INT_ID_AFEC_LOCK_IS_7,
      BCHP_INT_ID_AFEC_NOT_LOCK_IS_7,
      BCHP_INT_ID_HP_FRAME_BOUNDARY_7,
      BCHP_INT_ID_DFT_DONE_7,
   }
};


/******************************************************************************
 BAST_g3_P_PostInitAp()
******************************************************************************/
BERR_Code BAST_g3_P_PostInitAp(BAST_Handle h)
{
   extern bool is_channel_disabled(uint8_t chan);
   uint8_t i;

   for (i = 0; i < BAST_G3_MAX_CHANNELS; i++)
   {
      if (is_channel_disabled(i) == false)
      {
         BAST_g3_P_DiseqcPowerDown(h->pChannels[i]);
         BAST_g3_P_SdsPowerDownOpll_isrsafe(h->pChannels[i]);
         BAST_g3_P_LdpcPowerDown(h->pChannels[i]);
         BAST_g3_P_TurboPowerDown(h->pChannels[i]);
      }
   }
   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_g3_P_InitHandle()
******************************************************************************/
BERR_Code BAST_g3_P_InitHandle(BAST_Handle h)
{
   BAST_g3_P_Handle *hDev = (BAST_g3_P_Handle *)(h->pImpl);

   hDev->postInitApFunct = BAST_g3_P_PostInitAp;
   hDev->xtalFreq = BAST_G3_XTAL_FREQ;
   hDev->numSpurs = 0;
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
#ifndef BAST_EXCLUDE_EXT_TUNER
      hChn->bExternalTuner = false;
#endif
      hChn->bHasAfec = true;
      hChn->bHasTfec = true;
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

      hChn->miscCtl = BAST_G3_CONFIG_MISC_CTL_DISABLE_SMART_TUNE;
      hChn->acqParams.mode = BAST_Mode_eDvb_scan;
      hChn->tunerFreq = 950000000UL;
      hChn->notchState = -1;
      hChn->ldpcCtl = 0; /* enable PSL by default */
      if (hDev->bOpen == false)
         hChn->sampleFreq = BAST_DEFAULT_SAMPLE_FREQ;
      hChn->bFsNotDefault = false;
   }
   return retCode;
}


/******************************************************************************
 BAST_g3_P_GetRegisterAddress()
******************************************************************************/
void BAST_g3_P_GetRegisterAddress_isrsafe(BAST_ChannelHandle h, uint32_t reg, uint32_t *pAddr)
{
   *pAddr = reg;

   if (h->channel > 0)
   {
      if ((reg >= 0xA8100) && (reg < 0xA8200))
      {
         /* STB_CHAN_CHx register access */
         *pAddr += (uint32_t)((uint32_t)h->channel * 0x100);
      }
      else if ((reg >= 0xC0000) && (reg < 0xC430F))
      {
         /* SDS/AFEC/TFEC register access */
         *pAddr += (uint32_t)((uint32_t)h->channel * 0x6000);
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
 BAST_g3_P_GetAfecClock_isrsafe()
******************************************************************************/
BERR_Code BAST_g3_P_GetAfecClock_isrsafe(BAST_ChannelHandle h, uint32_t *pFreq)
{
   uint32_t afec_div, Fclk_sys_pll_vco;

   /*
      AFEC clock is (Fclk_sys_pll_vco / afec_div) / 2
      Default AFEC clock is (432 MHz / 2) = 216 MHz
   */
   afec_div = BREG_Read32(0, BCHP_TM_SYS_PLL_CLK_AFEC) & BCHP_TM_SYS_PLL_CLK_AFEC_DIV_MASK;

   BAST_g3_P_GetSysPllVcoFreq_isrsafe(h->pDevice, &Fclk_sys_pll_vco);
   *pFreq = (Fclk_sys_pll_vco / afec_div) >> 1;
   /* BDBG_MSG(("AFEC clock=%u", *pFreq)); */
   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_g3_P_GetTfecClock_isrsafe() - updates fecFreq
******************************************************************************/
BERR_Code BAST_g3_P_GetTfecClock_isrsafe(BAST_ChannelHandle h, uint32_t *pFreq)
{
   uint32_t tfec_div, Fclk_sys_pll_vco;

   tfec_div = BREG_Read32(0, BCHP_TM_SYS_PLL_CLK_TURBO) & BCHP_TM_SYS_PLL_CLK_TURBO_DIV_MASK;

   BAST_g3_P_GetSysPllVcoFreq_isrsafe(h->pDevice, &Fclk_sys_pll_vco);
   *pFreq = Fclk_sys_pll_vco / tfec_div;
   /* BDBG_MSG(("TFEC clock=%u", *pFreq)); */
   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_g3_P_GetAdcClock()
******************************************************************************/
BERR_Code BAST_g3_P_GetAdcClock(BAST_Handle h, uint32_t *clk)
{
   BAST_g3_P_Handle *hDev = h->pImpl;

   uint32_t pdiv;
   uint32_t ndiv_int;
   uint32_t ndiv_frac;
   uint32_t P_hi, P_lo, Q_hi, Q_lo;

   pdiv = BREG_Read32(0, BCHP_TM_ANA_PLL_PDIV) & 0xF;
   ndiv_int = BREG_Read32(0, BCHP_TM_ANA_PLL_NDIV_INT) & 0xFF;
   ndiv_frac = BREG_Read32(0, BCHP_TM_ANA_PLL_NDIV_FRAC) & 0xFFFFF;

   /* Fclk_rx_pll_vco = 54 MHz * (2 / Pdiv) * (Ndiv_int + (Ndiv_frac / 2^20)) */
   /* equivalent to (54 MHz * 2) * (2^20 * Ndiv_int + Ndiv_frac) / (2^20 * Pdiv) */

   BMTH_HILO_32TO64_Mul(hDev->xtalFreq << 1, (ndiv_int << 20) + ndiv_frac, &P_hi, &P_lo);  /* (54 MHz * 2) * (2^20 * Ndiv_int + Ndiv_frac) */
   BMTH_HILO_64TO64_Div32(P_hi, P_lo, pdiv << 20, &Q_hi, &Q_lo);  /* div by (2^20 * Pdiv) */

   BDBG_MSG(("Fs_adc=%d", Q_lo));
   *clk = Q_lo;
   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_g3_P_SdsPowerUp() - power up the sds core
******************************************************************************/
BERR_Code BAST_g3_P_SdsPowerUp(BAST_ChannelHandle h)
{
   /* enable 108 MHz register clock */
   BAST_g3_P_OrRegister_isrsafe(h, BCHP_TM_REG_CLK_EN, (1 << (h->channel + 4)));

   /* power up SDS */
   BAST_g3_P_OrRegister_isrsafe(h, BCHP_TM_SYS_CLK_EN, (1 << (h->channel + 15)));

   /* power up SDS memories */
   BAST_g3_P_AndRegister_isrsafe(h, BCHP_TM_MEM_CTRL0, ~(0x7 << (h->channel * 3)));

   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_g3_P_IsSdsOn()
******************************************************************************/
bool BAST_g3_P_IsSdsOn(BAST_ChannelHandle h)
{
   uint32_t val;

   BAST_g3_P_ReadRegister_isrsafe(h, BCHP_TM_SYS_CLK_EN, &val);
   val = (val >> 15) & 0xFF;

   if (val & (1 << h->channel))
      return true;
   else
      return false;
}


/******************************************************************************
 BAST_g3_P_SdsPowerDown() - power down the sds core
******************************************************************************/
BERR_Code BAST_g3_P_SdsPowerDown(BAST_ChannelHandle h)
{
   BERR_Code retCode;

   /* turn off oif */
   retCode = BAST_g3_P_SdsDisableOif_isrsafe(h);

   /* power down SDS memories first before TM */
   BAST_g3_P_OrRegister_isrsafe(h, BCHP_TM_MEM_CTRL0, (0x7 << (h->channel * 3)));

   /* power down SDS */
   BAST_g3_P_AndRegister_isrsafe(h, BCHP_TM_SYS_CLK_EN, ~(1 << (h->channel + 15)));

   /* disable 108 MHz register clock */
   BAST_g3_P_AndRegister_isrsafe(h, BCHP_TM_REG_CLK_EN, ~(1 << (h->channel + 4)));

   return retCode;
}


/******************************************************************************
 BAST_g3_P_IsLdpcOn_isrsafe()
******************************************************************************/
bool BAST_g3_P_IsLdpcOn_isrsafe(BAST_ChannelHandle h)
{
   uint32_t val;

   BAST_g3_P_ReadRegister_isrsafe(h, BCHP_TM_SYS_CLK_EN, &val);
   val = (val >> 7) & 0xFF;

   if (val & (1 << h->channel))
      return true;
   else
      return false;
}


/******************************************************************************
 BAST_g3_P_LdpcPowerUp_isrsafe() - power up the afec core
******************************************************************************/
BERR_Code BAST_g3_P_LdpcPowerUp_isrsafe(BAST_ChannelHandle h)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode;

   /* make sure system pll clock for afec is powered up */
   if ((BREG_Read32(NULL, BCHP_TM_SYS_PLL_CLK_AFEC) & 0x300) != 0x300)
      BAST_g3_P_OrRegister_isrsafe(h, BCHP_TM_SYS_PLL_CLK_AFEC, 0x300);

   /* power up AFEC */
   BAST_g3_P_OrRegister_isrsafe(h, BCHP_TM_SYS_CLK_EN, (1 << (h->channel + 7)));

   /* power up AFEC memories */
   BAST_g3_P_AndRegister_isrsafe(h, BCHP_TM_MEM_CTRL1, ~(0x7 << (h->channel * 3)));

   if ((retCode = BAST_g3_P_GetAfecClock_isrsafe(h, &(hChn->fecFreq))) != BERR_SUCCESS)
   {
      BERR_TRACE(retCode = BAST_ERR_HAB_CMD_FAIL);
      BDBG_ERR(("BAST_g3_P_LdpcPowerUp_isrsafe() failed"));
   }

   return retCode;
}


/******************************************************************************
 BAST_g3_P_LdpcPowerDown() - power down the afec core
******************************************************************************/
BERR_Code BAST_g3_P_LdpcPowerDown(BAST_ChannelHandle h)
{
   BERR_Code retCode;
   uint32_t val;

#if (BCHP_CHIP==4528) || (BCHP_CHIP==4538)
   BAST_g3_P_LdpcEnableDynamicPowerShutDown_isrsafe(h, false);
#endif

   /* power down AFEC memories before TM */
   BAST_g3_P_OrRegister_isrsafe(h, BCHP_TM_MEM_CTRL1, (0x7 << (h->channel * 3)));

   /* power down AFEC */
   BAST_g3_P_AndRegister_isrsafe(h, BCHP_TM_SYS_CLK_EN, ~(1 << (h->channel + 7)));

   /* power down afec system pll clock if all AFEC channels are powered down */
   val = BREG_Read32(NULL, BCHP_TM_SYS_CLK_EN);
   if ((val & 0x00007F80) == 0)
   {
      BAST_g3_P_AndRegister_isrsafe(h, BCHP_TM_SYS_PLL_CLK_AFEC, ~0x300);
   }

   if (BAST_g3_P_IsLdpcOn_isrsafe(h) == false)
      retCode = BERR_SUCCESS;
   else
   {
      BERR_TRACE(retCode = BAST_ERR_HAB_CMD_FAIL);
      BDBG_ERR(("BAST_g3_P_LdpcPowerDown() failed"));
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
   val = (val >> 24) & 0xFF;

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
   if ((BREG_Read32(NULL, BCHP_TM_SYS_PLL_CLK_TURBO) & 0x300) != 0x300)
      BAST_g3_P_OrRegister_isrsafe(h, BCHP_TM_SYS_PLL_CLK_TURBO, 0x300);

   /* power up TFEC */
   BAST_g3_P_OrRegister_isrsafe(h, BCHP_TM_SYS_CLK_EN, (1 << (h->channel + 24)));

   /* power up AFEC memories */
   BAST_g3_P_AndRegister_isrsafe(h, BCHP_TM_MEM_CTRL2, ~(0x7 << (h->channel * 3)));

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
   BAST_g3_P_OrRegister_isrsafe(h, BCHP_TM_MEM_CTRL2, (0x7 << (h->channel * 3)));

   /* power down TFEC */
   BAST_g3_P_AndRegister_isrsafe(h, BCHP_TM_SYS_CLK_EN, ~(1 << (h->channel + 24)));

   /* power down system pll clock for turbo if all TFECs are powered down */
   val = BREG_Read32(NULL, BCHP_TM_SYS_CLK_EN);
   if ((val & 0xFF000000) == 0)
   {
      BAST_g3_P_AndRegister_isrsafe(h, BCHP_TM_SYS_PLL_CLK_TURBO, ~0x300);
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

