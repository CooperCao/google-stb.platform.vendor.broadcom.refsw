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
#include "bchp_4538_hif.h"
#include "bast_priv.h"
#include "bast_4538.h"
#include "bast_4538_priv.h"


BDBG_MODULE(bast_4538);

#define BAST_BCM3447_I2C_ADDRESS 0x60


static const BAST_Settings defDevSettings =
{
   {  /* host i2c settings */
      0x6C, /* chipAddr */
      NULL, /* interruptEnableFunc */
      NULL  /* interruptEnableFuncParam */
   },
   { /* API function table */
      BAST_4538_P_Open,
      BAST_4538_P_Close,
      BAST_4538_P_GetTotalChannels,
      BAST_4538_GetChannelDefaultSettings,
      BAST_4538_P_OpenChannel,
      BAST_4538_P_CloseChannel,
      BAST_4538_P_GetDevice,
      BAST_4538_P_InitAp,
      BAST_4538_P_SoftReset,
      BAST_4538_P_GetApStatus,
      BAST_4538_P_GetApVersion,
      NULL, /* BAST_SetTmConfig, */
      NULL, /* BAST_GetTmConfig, */
      BAST_4538_P_ConfigGpio,
      BAST_4538_P_SetGpio,
      BAST_4538_P_GetGpio,
      BAST_4538_P_TuneAcquire,
      BAST_4538_P_GetChannelStatus,
      BAST_4538_P_GetLockStatus,
      BAST_4538_P_ResetStatus,
      BAST_4538_P_SetDiseqcTone,
      BAST_4538_P_GetDiseqcTone,
      BAST_4538_P_SetDiseqcVoltage,
      BAST_4538_P_SendDiseqcCommand,
      BAST_4538_P_GetDiseqcStatus,
      BAST_4538_P_ResetDiseqc,
      BAST_4538_P_ResetFsk, 
      BAST_4538_P_ReadFsk, 
      BAST_4538_P_WriteFsk, 
      BAST_4538_P_PowerDownFsk,
      BAST_4538_P_PowerUpFsk,
      NULL, /* BAST_4538_P_WriteMi2c, */
      NULL, /* BAST_4538_P_ReadMi2c, */
      BAST_4538_P_GetSoftDecisionBuf,
      NULL, /* BAST_4538_P_ReadAgc, */
      NULL, /* BAST_4538_P_WriteAgc, */
      NULL, /* BAST_4538_P_FreezeAgc, */
      BAST_4538_P_FreezeEq,
      BAST_4538_P_PowerDown,
      BAST_4538_P_PowerUp,     
      BAST_4538_P_ReadRegister,
      BAST_4538_P_WriteRegister,
      BAST_4538_P_ReadConfig,
      BAST_4538_P_WriteConfig,
      NULL, /* BAST_GetInterruptEventHandle */
      BAST_4538_P_GetLockStateChangeEventHandle,
      BAST_4538_P_GetFskEventHandle,
      BAST_4538_P_GetDiseqcEventHandle,
      NULL, /* BAST_HandleInterrupt_isr */
      NULL, /* BAST_ProcessInterruptEvent */
      BAST_4538_P_AbortAcq,
      NULL, /* BAST_ConfigLna */
      NULL, /* BAST_GetLnaStatus */
      NULL, /* BAST_ConfigAgc */
      BAST_4538_P_SendACW,
      BAST_4538_P_GetDiseqcVoltage,
      BAST_4538_P_GetDiseqcVsenseEventHandles,
      BAST_4538_P_EnableVsenseInterrupts,
      BAST_4538_P_PeakScan,
      BAST_4538_P_GetPeakScanStatus,
      BAST_4538_P_GetPeakScanEventHandle,
      BAST_4538_P_EnableStatusInterrupts,
      BAST_4538_P_GetStatusEventHandle,
      NULL, /* BAST_4538_P_ConfigBcm3445, */
      NULL, /* BAST_4538_P_MapBcm3445ToTuner, */
      NULL, /* BAST_4538_P_GetBcm3445Status, */
      BAST_4538_P_EnableSpurCanceller, 
      BAST_4538_P_ResetChannel, 
      BAST_4538_P_EnableDiseqcLnb, 
      BAST_4538_P_SetSearchRange,
      BAST_4538_P_GetSearchRange,
      BAST_4538_P_SetAmcScramblingSeq, 
      BAST_4538_P_SetTunerFilter,
      BAST_4538_P_GetSignalDetectStatus,
      BAST_4538_P_SetOutputTransportSettings,
      BAST_4538_P_GetOutputTransportSettings,
      BAST_4538_P_SetDiseqcSettings,
      BAST_4538_P_GetDiseqcSettings, 
      BAST_4538_P_SetNetworkSpec,
      BAST_4538_P_GetNetworkSpec,
      NULL, /* BAST_4538_P_TunerConfigLna, */
      NULL, /* BAST_4538_P_GetTunerLnaStatus, */
      NULL, /* BAST_SetFskChannel */
      NULL, /* BAST_GetFskChannel */
      BAST_4538_P_SetPeakScanSymbolRateRange,
      BAST_4538_P_GetPeakScanSymbolRateRange,
      BAST_4538_P_SetAdcSelect,
      BAST_4538_P_GetAdcSelect,
      BAST_4538_P_GetVersionInfo,
   },
   BAST_NetworkSpec_eDefault
};


static const BAST_ChannelSettings defChnSettings =
{
   (uint8_t)0
};


/******************************************************************************
 BAST_4538_GetDefaultSettings()
******************************************************************************/
BERR_Code BAST_4538_GetDefaultSettings(
   BAST_Settings *pDefSettings /* [out] default settings */
)
{
   *pDefSettings = defDevSettings;
   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_4538_P_GetChannelDefaultSettings()
******************************************************************************/
BERR_Code BAST_4538_GetChannelDefaultSettings(
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
 BAST_4538_PrintUart()
******************************************************************************/
BERR_Code BAST_4538_PrintUart(BAST_Handle h, char *pBuf)
{
   BAST_4538_P_Handle *pImpl = (BAST_4538_P_Handle *)(h->pImpl);
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
 BAST_4538_EnableAvs()
******************************************************************************/
BERR_Code BAST_4538_EnableAvs(BAST_Handle h, bool bEnable)
{
   BAST_4538_P_Handle *pImpl = (BAST_4538_P_Handle *)(h->pImpl);
   uint8_t buf[4];

   buf[0] = 0x2A;
   buf[1] = bEnable ? 1 : 0;
   return BHAB_SendHabCommand(pImpl->hHab, buf, 3, buf, 3, true, true, 3);
}


/******************************************************************************
 BAST_4538_WriteBsc()
******************************************************************************/ 
BERR_Code BAST_4538_WriteBsc(
   BAST_Handle h,        /* [in] BAST device handle */
   uint8_t channel,      /* [in] BSC channel select */
   uint8_t slave_addr,   /* [in] address of the i2c slave device */
   uint8_t *i2c_buf,     /* [in] specifies the data to transmit */
   uint8_t n             /* [in] number of bytes to transmit after the i2c slave address */
)
{
   BAST_4538_P_Handle *pImpl = (BAST_4538_P_Handle *)(h->pImpl);
   BERR_Code retCode;
   uint8_t buf[128], i, len;
   
   if (channel > 2)
      return BERR_INVALID_PARAMETER;

   if ((n == 0) || (n > 8))
      return BERR_INVALID_PARAMETER;

   buf[0] = 0x26;
   buf[1] = channel;
   buf[2] = 0;
   buf[3] = n;
   buf[4] = slave_addr & 0xFE;
   for (i = 0; i < n; i++)
      buf[5+i] = i2c_buf[i];
   len = (uint8_t)(6+n);
   BAST_4538_CHK_RETCODE(BHAB_SendHabCommand(pImpl->hHab, buf, len, buf, len, true, true, len));

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
 BAST_4538_ReadBsc()
******************************************************************************/ 
BERR_Code BAST_4538_ReadBsc(
   BAST_Handle h,        /* [in] BAST device handle */
   uint8_t channel,      /* [in] bsc channel select */
   uint8_t slave_addr,   /* [in] address of the i2c slave device */
   uint8_t *out_buf,     /* [in] specifies the data to transmit before the i2c restart condition */
   uint8_t out_n,        /* [in] number of bytes to transmit before the i2c restart condition not including the i2c slave address */
   uint8_t *in_buf,      /* [out] holds the data read */
   uint8_t in_n          /* [in] number of bytes to read after the i2c restart condition not including the i2c slave address */
)
{
   BAST_4538_P_Handle *pImpl = (BAST_4538_P_Handle *)(h->pImpl);
   BERR_Code retCode;
   uint8_t buf[128], i, len;
   
   if ((out_n == 0) || (out_n > 8) || (in_n == 0) || (in_n > 8) || (in_buf == NULL) || (channel > 2))
      return BERR_INVALID_PARAMETER;

   buf[0] = 0x27;
   buf[1] = channel;
   buf[2] = 0;
   buf[3] = out_n;
   buf[4] = in_n;
   buf[5] = slave_addr & 0xFE;
   for (i = 0; i < out_n; i++)
      buf[6+i] = out_buf[i];
   len = (uint8_t)(7+out_n+in_n);
   BAST_4538_CHK_RETCODE(BHAB_SendHabCommand(pImpl->hHab, buf, len, buf, len, true, true, len));

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
 BAST_4538_WriteGpo()
******************************************************************************/ 
BERR_Code BAST_4538_WriteGpo(
   BAST_Handle h,        /* [in] BAST device handle */
   uint32_t mask,        /* [in] specifies which GPO pins to write */
   uint32_t data         /* [in] specifies the state of the GPO pins to be written */
)
{
   BAST_4538_P_Handle *pImpl = (BAST_4538_P_Handle *)(h->pImpl);
   BERR_Code retCode;
   uint8_t buf[10];
   
   buf[0] = 0x28;
   buf[1] = (uint8_t)((mask >> 24) & 0xFF);
   buf[2] = (uint8_t)((mask >> 16) & 0xFF);
   buf[3] = (uint8_t)((mask >> 8) & 0xFF);
   buf[4] = (uint8_t)(mask & 0xFF);
   buf[5] = (uint8_t)((data >> 24) & 0xFF);
   buf[6] = (uint8_t)((data >> 16) & 0xFF);
   buf[7] = (uint8_t)((data >> 8) & 0xFF);
   buf[8] = (uint8_t)(data & 0xFF);
   BAST_4538_CHK_RETCODE(BHAB_SendHabCommand(pImpl->hHab, buf, 10, buf, 10, true, true, 10));

   done:
   return retCode;
}


/******************************************************************************
 BAST_4538_WriteFlashSector()
******************************************************************************/
BERR_Code BAST_4538_WriteFlashSector(BAST_Handle h, uint32_t addr)
{
   BAST_4538_P_Handle *pImpl = (BAST_4538_P_Handle *)(h->pImpl);             
   BERR_Code retCode;
   uint8_t buf[8];

   buf[0] = 0x4D;
   buf[1] = (uint8_t)((addr >> 24) & 0xFF);
   buf[2] = (uint8_t)((addr >> 16) & 0xFF);
   buf[3] = (uint8_t)((addr >> 8) & 0xFF);
   buf[4] = (uint8_t)(addr & 0xFF);
   BAST_4538_CHK_RETCODE(BHAB_SendHabCommand(pImpl->hHab, buf, 6, buf, 6, true, true, 6));

   done:
   return retCode;
}


#if 0
/******************************************************************************
 BAST_4538_PowerDownMtsif()
******************************************************************************/
BERR_Code BAST_4538_PowerDownMtsif(BAST_Handle h, bool bPowerDown)
{
   BAST_4538_P_Handle *pImpl = (BAST_4538_P_Handle *)(h->pImpl);             
   BERR_Code retCode;
   uint8_t buf[4];

   buf[0] = 0x4E;
   buf[1] = bPowerDown ? 1 : 0;
   BAST_4538_CHK_RETCODE(BHAB_SendHabCommand(pImpl->hHab, buf, 3, buf, 3, true, true, 3));

   done:
   return retCode;
}
#endif

/******************************************************************************
 BAST_4538_SetBertInterface()
******************************************************************************/
BERR_Code BAST_4538_SetBertInterface(BAST_Handle h, bool bEnable, bool bSerial, bool bClkinv, uint8_t channel)
{
   BAST_4538_P_Handle *pImpl = (BAST_4538_P_Handle *)(h->pImpl);             
   BERR_Code retCode;
   uint8_t buf[16];

   buf[0] = 0x09;
   buf[1] = 0;
   buf[2] = 0x09;
   buf[3] = 0x02;
   buf[4] = 0x74;
   buf[5] = 0;
   buf[6] = 0;
   buf[7] = 0;
   buf[8] = (bEnable ? 0x08 : 0x00);
   buf[8] |= (channel & 0x07);
   buf[8] |= (bSerial ? 0 : 0x10);
   buf[8] |= (bClkinv ? 0x20 : 0);
   BAST_4538_CHK_RETCODE(BHAB_SendHabCommand(pImpl->hHab, buf, 10, buf, 10, true, true, 10));

   done:
   return retCode;
}


/******************************************************************************
 BAST_4538_GetBertInterface()
******************************************************************************/
BERR_Code BAST_4538_GetBertInterface(BAST_Handle h, bool *pbEnable, bool *pbSerial, bool *pbClkinv, uint8_t *pChannel)
{
   BAST_4538_P_Handle *pImpl = (BAST_4538_P_Handle *)(h->pImpl);             
   BERR_Code retCode;
   uint8_t buf[16];

   buf[0] = 0x08;
   buf[1] = 0;
   buf[2] = 0x09;
   buf[3] = 0x02;
   buf[4] = 0x74;
   buf[5] = 1;
   buf[6] = 0;
   buf[7] = 0;
   buf[8] = 0;
   buf[9] = 0;
   BAST_4538_CHK_RETCODE(BHAB_SendHabCommand(pImpl->hHab, buf, 11, buf, 11, true, true, 11));

   *pbEnable = (buf[9] & 0x08) ? true : false;
   *pChannel = buf[9] & 0x07;
   *pbSerial = (buf[9] & 0x10) ? false : true;
   *pbClkinv = (buf[9] & 0x20) ? true : false;

   done:
   return retCode;
}


/******************************************************************************
 BAST_4538_GetAgcStatus()
******************************************************************************/
BERR_Code BAST_4538_GetAgcStatus(BAST_ChannelHandle h, BAST_4538_AgcStatus *pStatus)
{
   BAST_4538_P_Handle *pImpl = (BAST_4538_P_Handle *)(h->pDevice->pImpl);
   BERR_Code retCode;
   uint8_t buf[16];

   buf[0] = 0x2C;
   buf[1] = h->channel;
   BAST_4538_CHK_RETCODE(BHAB_SendHabCommand(pImpl->hHab, buf, 14, buf, 14, true, true, 14));

   pStatus->bLnaGainValid = (buf[2] & 1) ? true : false;
   pStatus->bChanAgcValid = (buf[2] & 2) ? true : false;
   pStatus->adcSelect = (buf[2] >> 6) & 0x03;
   pStatus->lnaGain = (buf[3] << 8) | buf[4];
   pStatus->chanAgc = (buf[5] << 24) | (buf[6] << 16) | (buf[7] << 8) | buf[8];
   pStatus->tunerFreq = (buf[9] << 24) | (buf[10] << 16) | (buf[11] << 8) | buf[12];

   done:
   return retCode;
}


/******************************************************************************
 BAST_4538_ReadBcm3447Register()
******************************************************************************/
BERR_Code BAST_4538_ReadBcm3447Register(BAST_Handle h, uint32_t reg, uint32_t *pVal)
{
   BERR_Code retCode;
   uint8_t addr = (uint8_t)(reg >> 2);
   uint8_t buf[4];

   retCode = BAST_4538_ReadBsc(h, 2, BAST_BCM3447_I2C_ADDRESS, &addr, 1, buf, 4);
   if (retCode == BERR_SUCCESS)
      *pVal = (buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3];
   else
      *pVal = 0;
   return retCode;
}


/******************************************************************************
 BAST_4538_WriteBcm3447Register()
******************************************************************************/
BERR_Code BAST_4538_WriteBcm3447Register(BAST_Handle h, uint32_t reg, uint32_t val)
{
   uint32_t addr = (reg >> 2);
   uint8_t buf[8];

   buf[0] = (uint8_t)(addr & 0xFF);
   buf[1] = (val >> 24) & 0xFF;
   buf[2] = (val >> 16) & 0xFF;
   buf[3] = (val >> 8) & 0xFF;
   buf[4] = (val & 0xFF);
   return BAST_4538_WriteBsc(h, 2, BAST_BCM3447_I2C_ADDRESS, buf, 5);
}


/******************************************************************************
 BERR_Code BAST_4538_ListenFsk()
******************************************************************************/
BERR_Code BAST_4538_ListenFsk(
   BAST_Handle h,  /* [in] BAST handle */
   uint8_t n       /* [in] length of data expected */
)
{
   BAST_4538_P_Handle *pImpl = (BAST_4538_P_Handle *)(h->pImpl);
   BERR_Code retCode;
   uint8_t buf[3];

   buf[0] = 0x53;
   buf[1] = n;
   BAST_4538_CHK_RETCODE(BHAB_SendHabCommand(pImpl->hHab, buf, 3, buf, 3, true, true, 3));

   done:
   return retCode;
}


/******************************************************************************
 BERR_Code BAST_4538_EnableFskCarrier()
******************************************************************************/
BERR_Code BAST_4538_EnableFskCarrier(
   BAST_Handle h,  /* [in] BAST handle */
   bool bEnable    /* [in] enable/disable carrier */
)
{
   BAST_4538_P_Handle *pImpl = (BAST_4538_P_Handle *)(h->pImpl);
   BERR_Code retCode;
   uint8_t buf[3];

   buf[0] = 0x54;
   buf[1] = bEnable;
   BAST_4538_CHK_RETCODE(BHAB_SendHabCommand(pImpl->hHab, buf, 3, buf, 3, true, true, 3));

   done:
   return retCode;
}

