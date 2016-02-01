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

#include "bdsq.h"
#include "bdsq_priv.h"
#include "bdsq_g1_priv.h"
#include "bdsq_4550_priv.h"

BDBG_MODULE(bdsq_4550_priv);


uint32_t BDSQ_g1_ChannelIntrID[BDSQ_NUM_CHANNELS][BDSQ_g1_MaxIntID] = {
   {
      /* DSEC channel 0 interrupts */
      BCHP_INT_ID_DSEC_TIMER1_0,
      BCHP_INT_ID_DSEC_TIMER2_0,
      BCHP_INT_ID_DSEC_DONE_0,
      BCHP_INT_ID_DSEC_TX_A_EMPTY_0,
      BCHP_INT_ID_DSEC_RX_A_EMPTY_0,
      BCHP_INT_ID_DSEC_ACW_DONE_0,
      BCHP_INT_ID_DSEC_VOL_LT_THRESH_0,
      BCHP_INT_ID_DSEC_VOL_GT_THRESH_0,
      BCHP_INT_ID_DSEC_TONE_RISE_0,
      BCHP_INT_ID_DSEC_TONE_FALL_0,
      BCHP_INT_ID_DSEC_IDLE_TIMEOUT_0
   }
};


/******************************************************************************
 BDSQ_P_GetRegisterAddress()
******************************************************************************/
BERR_Code BDSQ_P_GetRegisterAddress(BDSQ_ChannelHandle h, uint32_t reg, uint32_t *pAddr)
{
   BERR_Code retCode = BERR_SUCCESS;
   *pAddr = reg;
   
   if ((reg < 0xA4000) || (reg > 0xA4154))
      return BERR_INVALID_PARAMETER;
   
   /*BDBG_MSG(("GetRegisterAddress(%d): %08X -> %08X", h->channel, reg, *pAddr)); */
   return retCode;
}


/******************************************************************************
 BDSQ_P_ReadRegister_isrsafe()
******************************************************************************/
BERR_Code BDSQ_P_ReadRegister_isrsafe(
   BDSQ_ChannelHandle h,      /* [in] BDSQ channel handle */
   uint32_t           reg,    /* [in] address of register to read */
   uint32_t           *val    /* [out] contains data that was read */
)
{
   BERR_Code retCode = BERR_SUCCESS;
   uint32_t addr;
   
   retCode = BDSQ_P_GetRegisterAddress(h, reg, &addr);
   *val = BREG_Read32(h->pDevice->hRegister, addr); 
   
   return retCode;
}


/******************************************************************************
 BDSQ_P_WriteRegister_isrsafe()
******************************************************************************/
BERR_Code BDSQ_P_WriteRegister_isrsafe(
   BDSQ_ChannelHandle h,      /* [in] BDSQ channel handle */
   uint32_t           reg,    /* [in] address of register to write */
   uint32_t           val     /* [in] contains data to be written */
)
{
   BERR_Code retCode = BERR_SUCCESS;
   uint32_t addr;

   retCode = BDSQ_P_GetRegisterAddress(h, reg, &addr);
   BREG_Write32(h->pDevice->hRegister, addr, val);
   
   return retCode;
}


/******************************************************************************
 BDSQ_P_PowerDownDsecPhy()
******************************************************************************/
BERR_Code BDSQ_P_PowerDownDsecPhy(BDSQ_ChannelHandle h)
{
   /* power down vsense and rx phy */
   BDSQ_P_OrRegister(h, BCHP_SDS_DSEC_DS_COMMON_CONTROL, 0x00000002);

	return BERR_SUCCESS;
}


/******************************************************************************
 BDSQ_P_PowerUpDsecPhy()
******************************************************************************/
BERR_Code BDSQ_P_PowerUpDsecPhy(BDSQ_ChannelHandle h)
{
   /* configure fsk phy */
	BDSQ_P_WriteRegister_isrsafe(h, BCHP_FTM_PHY_ANA_CTL0_1, 0x006C0000);

   /* update pga gain */
   BDSQ_P_ReadModifyWriteRegister(h, BCHP_FTM_PHY_ANA_CTL0_1, ~0x00780000, (hChn->configParam[BDSQ_g1_CONFIG_PGA_GAIN] & 0xF) << 19);

   /* power up vsense and rx phy */
   BDSQ_P_AndRegister(h, BCHP_SDS_DSEC_DS_COMMON_CONTROL, ~0x00000002);

   /* default to AFE mode 12MHz */
   BDSQ_P_WriteRegister_isrsafe(h, BCHP_SDS_DSEC_DSCGDIV, 0x0000096C);

   BDSQ_P_WriteRegister_isrsafe(h, BCHP_SDS_DSEC_DS_COMMON_CONTROL, 0x00000028);   /* enable diseqc input, select adc for diseqc rx */
   BDSQ_P_ToggleBit(h, BCHP_SDS_DSEC_DS_COMMON_CONTROL, 0x00000001);       /* reset vsense and rx phy  */

	return BERR_SUCCESS;
}


/******************************************************************************
 BDSQ_P_PowerDownVsensePhy()
******************************************************************************/
BERR_Code BDSQ_P_PowerDownVsensePhy(BDSQ_ChannelHandle h)
{
   /* vsense phy coupled with rx phy */
   BDSQ_P_PowerDownDsecPhy(h);

   hChn->bVsenseEnabled = false;
	return retCode;
}


/******************************************************************************
 BDSQ_P_PowerUpVsensePhy()
******************************************************************************/
BERR_Code BDSQ_P_PowerUpVsensePhy(BDSQ_ChannelHandle h)
{
   BDSQ_g1_P_ChannelHandle *hChn = (BDSQ_g1_P_ChannelHandle *)(h->pImpl);
   BERR_Code retCode = BERR_SUCCESS;
   uint32_t val;

   /* vsense phy coupled with rx phy */
   BDSQ_P_PowerUpDsecPhy(h);

   /* update vsense hi threshold */
   val = hChn->configParam[BDSQ_g1_CONFIG_VSENSE_THRESHOLD_HI] + 0x80;  /* convert to 2's comp */
   BDSQ_P_ReadModifyWriteRegister(h, BCHP_SDS_DSEC_DS_SAR_THRSH, ~0x0000FF00, (val & 0xFF) << 8);

   /* update vsense lo threshold */
   val = hChn->configParam[BDSQ_g1_CONFIG_VSENSE_THRESHOLD_LO] + 0x80;  /* convert to 2's comp */
   BDSQ_P_ReadModifyWriteRegister(h, BCHP_SDS_DSEC_DS_SAR_THRSH, ~0x000000FF, val & 0xFF);

   /* set lpf alpha for vsense operation, configure sar format to offset binary */
   BDSQ_P_ReadModifyWriteRegister(h, BCHP_SDS_DSEC_DS_SAR_CONTROL, ~0x00000F10, 0x00000500);

   /* enable vsense input */
   BDSQ_P_OrRegister(h, BCHP_SDS_DSEC_DS_COMMON_CONTROL, 0x00000080);

   /* start vsense adc */
   BDSQ_P_ReadModifyWriteRegister(h, BCHP_SDS_DSEC_DS_COMMON_CONTROL, ~0x00000200, 0x00000100);

   hChn->bVsenseEnabled = true;
	return retCode;
}


/******************************************************************************
 BDSQ_P_ConfigAdcMode()
******************************************************************************/
BERR_Code BDSQ_P_ConfigAdcMode(BDSQ_ChannelHandle h, BDSQ_AdcMode mode)
{
   BDSQ_g1_P_ChannelHandle *hChn = (BDSQ_g1_P_ChannelHandle *)h->pImpl;

   switch (mode)
   {
      case BDSQ_AdcMode_eRx:
         BDSQ_P_ReadModifyWriteRegister(h, BCHP_SDS_DSEC_DS_COMMON_CONTROL, ~0x00000040, 0x00000080);    /* stop vsense */
         BDSQ_P_ReadModifyWriteRegister(h, BCHP_SDS_DSEC_DSCGDIV, ~0x0000FF00, 0x00000900);     /* AFE mode 12MHz */
         BDSQ_P_OrRegister(h, BCHP_SDS_DSEC_DS_COMMON_CONTROL, 0x00000028);   /* enable diseqc input, select adc for diseqc rx */
         break;
      
      case BDSQ_AdcMode_eVsense:
         BDSQ_P_AndRegister(h, BCHP_SDS_DSEC_DS_COMMON_CONTROL, ~0x00000028);    /* disable diseqc input, select adc for vsense */
         BDSQ_P_ReadModifyWriteRegister(h, BCHP_SDS_DSEC_DSCGDIV, ~0x0000FF00, 0x00002400);  /* vsense mode 3MHz */
         BDSQ_P_ReadModifyWriteRegister(h, BCHP_SDS_DSEC_DS_COMMON_CONTROL, ~0x00000080, 0x00000040);   /* start vsense */
         break;
      
      default:
         BDBG_ERR(("invalid phy config!"));
         return BERR_TRACE(BERR_INVALID_PARAMETER);
   }
   
	return BERR_SUCCESS;
}


/******************************************************************************
 BDSQ_P_ResetVsense()
******************************************************************************/
BERR_Code BDSQ_P_ResetVsense(BDSQ_ChannelHandle h)
{
   BDSQ_g1_P_ChannelHandle *hChn = (BDSQ_g1_P_ChannelHandle *)h->pImpl;
   uint32_t mb;

   /* reset lpf integrator, then clear bits */
   BDSQ_P_ReadModifyWriteRegister(h, BCHP_SDS_DSEC_DS_SAR_CONTROL, ~0x00, 0x2000);
   BDSQ_P_ReadModifyWriteRegister(h, BCHP_SDS_DSEC_DS_SAR_CONTROL, ~0x2000, 0x00);

   /* reset max/min values, then clear bits */
   BDSQ_P_ReadModifyWriteRegister(h, BCHP_SDS_DSEC_DS_SAR_CONTROL, ~0x00, 0xC0);
   BDSQ_P_ReadModifyWriteRegister(h, BCHP_SDS_DSEC_DS_SAR_CONTROL, ~0xC0, 0x00);

   /* re-enable interrupts if previously on */
   BDSQ_P_ReadRegister_isrsafe(h, BCHP_SDS_DSEC_DS_SAR_THRSH, &mb);
   if ((mb & 0x10000) == 0x10000)
   {
      BINT_EnableCallback_isr(hChn->hOverVoltageCb);
      BINT_EnableCallback_isr(hChn->hUnderVoltageCb);
   }

   return BERR_SUCCESS;
}


/******************************************************************************
 BDSQ_P_CalibrateDcOffset() - ISR context
******************************************************************************/
BERR_Code BDSQ_P_CalibrateDcOffset(BDSQ_ChannelHandle h)
{
   uint32_t mb;

   /* configure DC offset */
   BDSQ_P_ReadRegister_isrsafe(h, BCHP_SDS_DSEC_DS_SAR_DATA_OUT, &mb);
   BDSQ_P_WriteRegister_isrsafe(h, BCHP_SDS_DSEC_DS_SAR_DC_OFFSET, mb);

   /* set lpf alpha for vsense operation */
   BDSQ_P_ReadModifyWriteRegister(h, BCHP_SDS_DSEC_DS_SAR_CONTROL, ~0xF00, 0x0B00);

   /* configure adc for diseqc rx */
   BDSQ_P_ConfigAdcMode(h, BDSQ_AdcMode_eRx);

   /* soft reset after calibration */
   return BDSQ_P_ResetVsense(h);
}

/******************************************************************************
 BDSQ_P_CalibrateVsense() 
******************************************************************************/
BERR_Code BDSQ_P_CalibrateVsense(BDSQ_ChannelHandle h)
{
   BDSQ_g1_P_ChannelHandle *hChn = (BDSQ_g1_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode = BERR_SUCCESS;
   
   /* disable vsense interrupts for calibration */
   BINT_DisableCallback(hChn->hOverVoltageCb);
   BINT_DisableCallback(hChn->hUnderVoltageCb);
   
   /* set lpf alpha for vsense calibration */
   BDSQ_P_ReadModifyWriteRegister(h, BCHP_SDS_DSEC_DS_SAR_CONTROL, ~0xF00, 0x500);

   /* wait for filter value to converge */
   BKNI_EnterCriticalSection();
   retCode = BDSQ_g1_P_EnableTimer(h, BDSQ_g1_TimerSelect_e2, 50, BDSQ_P_CalibrateDcOffset);
   BKNI_LeaveCriticalSection();
   
   return retCode;
}


/******************************************************************************
 BDSQ_P_ReadVsenseAdc()
******************************************************************************/
BERR_Code BDSQ_P_ReadVsenseAdc(BDSQ_ChannelHandle h, uint16_t *pVal)
{
   uint32_t val;

   BDSQ_P_ConfigAdcMode(h, BDSQ_AdcMode_eVsense);
   BKNI_Delay(10);

   BDSQ_P_ReadRegister_isrsafe(h, BCHP_SDS_DSEC_DS_SAR_DATA_OUT, &val);

   /* 8-bit vsense adc, offset binary format */
   *pVal = (uint16_t)(val & 0xFF);

   BDSQ_P_CalibrateVsense(h);

   return BERR_SUCCESS;
}

