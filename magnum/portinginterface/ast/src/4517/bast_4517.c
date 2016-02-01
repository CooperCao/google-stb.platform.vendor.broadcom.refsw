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
#include "bast.h"
#include "bchp_hif.h"
#include "bchp_tm.h"
#include "bast_priv.h"
#include "bast_4517.h"
#include "bast_4517_priv.h"


BDBG_MODULE(bast_4517);


static const BAST_Settings defDevSettings =
{
   {  /* host i2c settings */
      0x6C, /* chipAddr */
      NULL, /* interruptEnableFunc */
      NULL  /* interruptEnableFuncParam */
   },
   { /* API function table */
      BAST_4517_P_Open,
      BAST_4517_P_Close,
      BAST_4517_P_GetTotalChannels,
      BAST_4517_GetChannelDefaultSettings,
      BAST_4517_P_OpenChannel,
      BAST_4517_P_CloseChannel,
      BAST_4517_P_GetDevice,
      BAST_4517_P_InitAp,
      BAST_4517_P_SoftReset,
      BAST_4517_P_GetApStatus,
      BAST_4517_P_GetApVersion,
      NULL, /* BAST_SetTmConfig, */
      NULL, /* BAST_GetTmConfig, */
      BAST_4517_P_ConfigGpio,
      BAST_4517_P_SetGpio,
      BAST_4517_P_GetGpio,
      BAST_4517_P_TuneAcquire,
      BAST_4517_P_GetChannelStatus,
      BAST_4517_P_GetLockStatus,
      BAST_4517_P_ResetStatus,
      BAST_4517_P_SetDiseqcTone,
      BAST_4517_P_GetDiseqcTone,
      BAST_4517_P_SetDiseqcVoltage,
      BAST_4517_P_SendDiseqcCommand,
      BAST_4517_P_GetDiseqcStatus,
      BAST_4517_P_ResetDiseqc,
      BAST_4517_P_ResetFsk, 
      BAST_4517_P_ReadFsk, 
      BAST_4517_P_WriteFsk, 
      BAST_4517_P_PowerDownFsk,
      BAST_4517_P_PowerUpFsk,
      BAST_4517_P_WriteMi2c,
      BAST_4517_P_ReadMi2c,
      BAST_4517_P_GetSoftDecisionBuf,
      BAST_4517_P_ReadAgc, 
      BAST_4517_P_WriteAgc,
      BAST_4517_P_FreezeAgc,
      BAST_4517_P_FreezeEq,
      BAST_4517_P_PowerDown,
      BAST_4517_P_PowerUp,     
      BAST_4517_P_ReadRegister,
      BAST_4517_P_WriteRegister,
      BAST_4517_P_ReadConfig,
      BAST_4517_P_WriteConfig,
      NULL, /* BAST_GetInterruptEventHandle */
      BAST_4517_P_GetLockStateChangeEventHandle,
      BAST_4517_P_GetFskEventHandle,
      BAST_4517_P_GetDiseqcEventHandle,
      NULL, /* BAST_HandleInterrupt_isr */
      NULL, /* BAST_ProcessInterruptEvent */
      BAST_4517_P_AbortAcq,
      NULL, /* BAST_ConfigLna */
      NULL, /* BAST_GetLnaStatus */
      NULL, /* BAST_ConfigAgc */
      BAST_4517_P_SendACW,
      BAST_4517_P_GetDiseqcVoltage,
      BAST_4517_P_GetDiseqcVsenseEventHandles,
      BAST_4517_P_EnableVsenseInterrupts,
      BAST_4517_P_PeakScan,
      BAST_4517_P_GetPeakScanStatus,
      BAST_4517_P_GetPeakScanEventHandle,
      BAST_4517_P_EnableStatusInterrupts,
      BAST_4517_P_GetStatusEventHandle,
      BAST_4517_P_ConfigBcm3445,
      BAST_4517_P_MapBcm3445ToTuner,
      BAST_4517_P_GetBcm3445Status,
      BAST_4517_P_EnableSpurCanceller, 
      BAST_4517_P_ResetChannel, 
      BAST_4517_P_EnableDiseqcLnb, 
      BAST_4517_P_SetSearchRange,
      BAST_4517_P_GetSearchRange,
      NULL, /* BAST_SetAmcScramblingSeq */
      BAST_4517_P_SetTunerFilter,
      BAST_4517_P_GetSignalDetectStatus,
      BAST_4517_P_SetOutputTransportSettings,
      BAST_4517_P_GetOutputTransportSettings,
      BAST_4517_P_SetDiseqcSettings,
      BAST_4517_P_GetDiseqcSettings, 
      BAST_4517_P_SetNetworkSpec,
      BAST_4517_P_GetNetworkSpec,
      BAST_4517_P_TunerConfigLna,
      BAST_4517_P_GetTunerLnaStatus,
      NULL, /* BAST_SetFskChannel */
      NULL, /* BAST_GetFskChannel */
      BAST_4517_P_SetPeakScanSymbolRateRange,
      BAST_4517_P_GetPeakScanSymbolRateRange,
      NULL, /* BAST_SetAdcSelect */
      NULL, /* BAST_GetAdcSelect */
      BAST_4517_P_GetVersionInfo
   },
   BAST_NetworkSpec_eEcho
};


static const BAST_ChannelSettings defChnSettings =
{
   (uint8_t)0
};


/******************************************************************************
 BAST_4517_GetDefaultSettings()
******************************************************************************/
BERR_Code BAST_4517_GetDefaultSettings(
   BAST_Settings *pDefSettings /* [out] default settings */
)
{
   *pDefSettings = defDevSettings;
   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_4517_P_GetChannelDefaultSettings()
******************************************************************************/
BERR_Code BAST_4517_GetChannelDefaultSettings(
   BAST_Handle   h,                      /* [in] BAST handle */
   uint32_t      chnNo,                  /* [in] channel number */
   BAST_ChannelSettings *pChnDefSettings /* [out] default channel settings */
)
{
   BSTD_UNUSED(h);
   BSTD_UNUSED(chnNo);
   *pChnDefSettings = defChnSettings;
   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_4517_PrintUart()
******************************************************************************/
BERR_Code BAST_4517_PrintUart(BAST_Handle h, char *pBuf)
{
   BAST_4517_P_Handle *pImpl = (BAST_4517_P_Handle *)(h->pImpl);
   uint8_t len, buf[128], n;

   buf[0] = 0x0C;
   for (len = 0; len < 124; len++)
   {
      buf[len+2] = pBuf[len];
      if (pBuf[len] == 0)
         break;
   }
   buf[1] = len; 

   n = (uint8_t)(3+len);
   return BHAB_SendHabCommand(pImpl->hHab, buf, n, buf, n, true, true, n);
}


/******************************************************************************
 BAST_4517_EnableAvs()
******************************************************************************/
BERR_Code BAST_4517_EnableAvs(BAST_Handle h, bool bEnable)
{
   BAST_4517_P_Handle *pImpl = (BAST_4517_P_Handle *)(h->pImpl);
   uint8_t buf[4];

   buf[0] = 0x36;
   buf[1] = bEnable ? 1 : 0;
   return BHAB_SendHabCommand(pImpl->hHab, buf, 3, buf, 3, true, true, 3);
}


/******************************************************************************
 BAST_4517_GetAvsStatus()
******************************************************************************/
BERR_Code BAST_4517_GetAvsStatus(BAST_Handle h, BAST_AvsStatus *pStatus)
{
   BAST_4517_P_Handle *pImpl = (BAST_4517_P_Handle *)(h->pImpl);
   BERR_Code retCode;
   uint8_t buf[12];

   buf[0] = 0x29;
   BAST_4517_CHK_RETCODE(BHAB_SendHabCommand(pImpl->hHab, buf, 11, buf, 11, true, true, 11));

   pStatus->bEnabled = (buf[1] & 0x01) ? true : false;
   pStatus->temperature = (uint32_t)((buf[2] << 24) | (buf[3] << 16) | (buf[4] << 8) | buf[5]);
   pStatus->voltage_1p1_0 = (uint32_t)((buf[6] << 24) | (buf[7] << 16) | (buf[8] << 8) | buf[9]);

   done:
   return retCode;
}


/******************************************************************************
 BAST_4517_GetTunerStatus()
******************************************************************************/
BERR_Code BAST_4517_GetTunerStatus(BAST_ChannelHandle h, BAST_4517_TunerStatus *pStatus)
{
   BAST_4517_P_Handle *pDevImpl = (BAST_4517_P_Handle *)(h->pDevice->pImpl);
   BERR_Code retCode;
   uint8_t buf[15];

   buf[0] = 0x36;
   buf[1] = h->channel;
   BAST_4517_CHK_RETCODE(BHAB_SendHabCommand(pDevImpl->hHab, buf, 15, buf, 15, true, true, 15));

   pStatus->Fddfs = (uint32_t)((buf[2] << 24) | (buf[3] << 16) | (buf[4] << 8) | buf[5]);
   pStatus->loDiv = (uint32_t)((buf[6] << 24) | (buf[7] << 16) | (buf[8] << 8) | buf[9]);
   pStatus->loFreq = (uint32_t)((buf[10] << 24) | (buf[11] << 16) | (buf[12] << 8) | buf[13]);

   done:
   return retCode;
}


/******************************************************************************
 BAST_4517_WriteBsc()
******************************************************************************/ 
BERR_Code BAST_4517_WriteBsc(
   BAST_Handle h,        /* [in] BAST device handle */
   uint8_t slave_addr,   /* [in] address of the i2c slave device */
   uint8_t *i2c_buf,     /* [in] specifies the data to transmit */
   uint8_t n             /* [in] number of bytes to transmit after the i2c slave address */
)
{
   BAST_4517_P_Handle *pImpl = (BAST_4517_P_Handle *)(h->pImpl);
   BERR_Code retCode;
   uint8_t buf[128], i, len;
   
   if ((n == 0) || (n > 8))
      return BERR_INVALID_PARAMETER;

   buf[0] = 0x26;
   buf[1] = 3;
   buf[2] = 0;
   buf[3] = n;
   buf[4] = slave_addr & 0xFE;
   for (i = 0; i < n; i++)
      buf[5+i] = i2c_buf[i];
   len = (uint8_t)(6+n);
   BAST_4517_CHK_RETCODE(BHAB_SendHabCommand(pImpl->hHab, buf, len, buf, len, true, true, len));

   switch (buf[2])
   {
      case 0:
         retCode = BERR_SUCCESS;
         break;

      case 1:
         retCode = BAST_ERR_MI2C_NO_ACK;
         BDBG_WRN(("no i2c ack"));
         break;

      case 2:
         retCode = BAST_ERR_MI2C_BUSY;
         BDBG_WRN(("mi2c busy"));
         break;

      default:
         retCode = BAST_ERR_HAB_FAIL;
         BDBG_WRN(("mi2c error"));
         break;
   }

   done:
   return retCode;
}


/******************************************************************************
 BAST_4517_ReadBsc()
******************************************************************************/ 
BERR_Code BAST_4517_ReadBsc(BAST_Handle h, uint8_t slave_addr, uint8_t *out_buf, uint8_t out_n, uint8_t *in_buf, uint8_t in_n)
{
   BAST_4517_P_Handle *pImpl = (BAST_4517_P_Handle *)(h->pImpl);
   BERR_Code retCode;
   uint8_t buf[128], i, len;
   
   if ((out_n == 0) || (out_n > 8) || (in_n == 0) || (in_n > 8) || (in_buf == NULL))
      return BERR_INVALID_PARAMETER;

   buf[0] = 0x27;
   buf[1] = 3;
   buf[2] = 0;
   buf[3] = out_n;
   buf[4] = in_n;
   buf[5] = slave_addr & 0xFE;
   for (i = 0; i < out_n; i++)
      buf[6+i] = out_buf[i];
   len = (uint8_t)(7+out_n+in_n);
   BAST_4517_CHK_RETCODE(BHAB_SendHabCommand(pImpl->hHab, buf, len, buf, len, true, true, len));

   switch (buf[2])
   {
      case 0:
         retCode = BERR_SUCCESS;
         for (i = 0; i < in_n; i++)
            in_buf[i] = buf[6+out_n+i];
         break;

      case 1:
         retCode = BAST_ERR_MI2C_NO_ACK;
         BDBG_WRN(("no i2c ack"));
         break;

      case 2:
         retCode = BAST_ERR_MI2C_BUSY;
         BDBG_WRN(("mi2c busy"));
         break;

      default:
         retCode = BAST_ERR_HAB_FAIL;
         BDBG_WRN(("mi2c error"));
         break;
   }

   done:
   return retCode;
}


/******************************************************************************
 BAST_4517_ReadTunerIndirectRegister()
******************************************************************************/
BERR_Code BAST_4517_ReadTunerIndirectRegister(BAST_ChannelHandle h, BAST_TunerIndirectRegGroup group, uint8_t addr, uint32_t *pVal)
{
   BAST_4517_P_Handle *pDevImpl = (BAST_4517_P_Handle *)(h->pDevice->pImpl);
   BERR_Code retCode;
   uint8_t buf[9];

   if ((group > 5) || (addr > 13) || (pVal == NULL))
      return BERR_TRACE(BERR_INVALID_PARAMETER);

   buf[0] = 0x39;
   buf[1] = h->channel;
   buf[2] = (uint8_t)group;
   buf[3] = addr;
   BAST_4517_CHK_RETCODE(BHAB_SendHabCommand(pDevImpl->hHab, buf, 9, buf, 9, true, true, 9));

   *pVal = (buf[4] << 24) | (buf[5] << 16) | (buf[6] << 8) | buf[7];

   done:
   return retCode;
}


/******************************************************************************
 BAST_4517_WriteTunerIndirectRegister()
******************************************************************************/
BERR_Code BAST_4517_WriteTunerIndirectRegister(BAST_ChannelHandle h, BAST_TunerIndirectRegGroup group, uint8_t addr, uint32_t val)
{
   BAST_4517_P_Handle *pDevImpl = (BAST_4517_P_Handle *)(h->pDevice->pImpl);
   BERR_Code retCode;
   uint8_t buf[9];

   if ((group > 5) || (addr > 13))
      return BERR_TRACE(BERR_INVALID_PARAMETER);

   buf[0] = 0x3A;
   buf[1] = h->channel;
   buf[2] = (uint8_t)group;
   buf[3] = addr;
   buf[4] = (uint8_t)((val >> 24) & 0xFF);
   buf[5] = (uint8_t)((val >> 16) & 0xFF);
   buf[6] = (uint8_t)((val >> 8) & 0xFF);
   buf[7] = (uint8_t)(val & 0xFF);
   
   BAST_4517_CHK_RETCODE(BHAB_SendHabCommand(pDevImpl->hHab, buf, 9, buf, 9, true, true, 9));
   
   done:
   return retCode;
}


/******************************************************************************
 BAST_4517_DeepSleep()
******************************************************************************/
BERR_Code BAST_4517_DeepSleep(
   BAST_Handle h  /* [in] BAST handle */
)
{
   BERR_Code retCode = BERR_SUCCESS;
   BAST_4517_P_Handle *pDevImpl = (BAST_4517_P_Handle *)(h->pImpl);
   uint32_t val;
   uint8_t i;

   /* power down ftm */
   BAST_4517_P_PowerDownFsk(h);
   
   for (i = 0; i < BAST_4517_MAX_CHANNELS; i++)
   {
      /* power down all channels */
      BAST_4517_P_PowerDown(h->pChannels[i], BAST_CORE_ALL);
   }
   
   /* wait for power down tasks to complete */
   BKNI_Sleep(10);
   
   /* power down sys, analog pll, cml */
   val = 0x00000007;
   BAST_4517_CHK_RETCODE(BHAB_WriteRegister(pDevImpl->hHab, BCHP_TM_PWRDN, &val));
   
#if 0
   /* power down leap memories */
   val = 0x0F;
   BHAB_CHK_RETCODE(BHAB_WriteRegister(pDevImpl->hHab, BCHP_HIF_MEM_CTRL, &val));
#endif
   
   /* power down reg pll and xtal */
   val = 0x03;
   BHAB_CHK_RETCODE(BHAB_WriteRegister(pDevImpl->hHab, BCHP_HIF_PWRDN, &val));
   
   done:
   return retCode;
}


/******************************************************************************
 BAST_4517_WakeUp()
******************************************************************************/
BERR_Code BAST_4517_WakeUp(
   BAST_Handle h  /* [in] BAST handle */
)
{
   BERR_Code retCode = BERR_SUCCESS;
   BAST_4517_P_Handle *pDevImpl = (BAST_4517_P_Handle *)(h->pImpl);
   uint32_t val;
   uint8_t i;
   
   /* power up reg pll and xtal */
   val = 0x00;
   BHAB_CHK_RETCODE(BHAB_WriteRegister(pDevImpl->hHab, BCHP_HIF_PWRDN, &val));
   
#if 0
   /* power up leap memories */
   val = 0x02;
   BHAB_CHK_RETCODE(BHAB_WriteRegister(pDevImpl->hHab, BCHP_HIF_MEM_CTRL, &val));
   BKNI_Sleep(2);
   val = 0x02;
   BHAB_CHK_RETCODE(BHAB_WriteRegister(pDevImpl->hHab, BCHP_HIF_MEM_CTRL, &val));
#endif
   
   /* power up sys, analog pll, leave cml off */
   val = 0x00000002;
   BAST_4517_CHK_RETCODE(BHAB_WriteRegister(pDevImpl->hHab, BCHP_TM_PWRDN, &val));

   /* power up ftm */
   BAST_4517_P_PowerUpFsk(h);
   
   for (i = 0; i < BAST_4517_MAX_CHANNELS; i++)
   {
      /* power down all channels */
      BAST_4517_P_PowerUp(h->pChannels[i], BAST_CORE_ALL);
   }
   
   done:
   return retCode;
}
