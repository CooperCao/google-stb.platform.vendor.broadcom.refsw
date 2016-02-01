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
#include "bchp_4501.h"
#include "bast.h"
#include "bast_priv.h"
#include "bast_4501.h"
#include "bast_4501_priv.h"


BDBG_MODULE(bast_4501);


static const BAST_Settings defDevSettings =
{
   {  /* host i2c settings */
      0x69, /* chipAddr */
      NULL, /* interruptEnableFunc */
      NULL  /* interruptEnableFuncParam */
   },
   { /* API function table */
      BAST_4501_P_Open,
      BAST_4501_P_Close,
      BAST_4501_P_GetTotalChannels,
      BAST_4501_GetChannelDefaultSettings,
      BAST_4501_P_OpenChannel,
      BAST_4501_P_CloseChannel,
      BAST_4501_P_GetDevice,
      BAST_4501_P_InitAp,
      BAST_4501_P_SoftReset,
      BAST_4501_P_GetApStatus,
      BAST_4501_P_GetApVersion,
      BAST_4501_P_SetTmConfig,
      BAST_4501_P_GetTmConfig,
      BAST_4501_P_ConfigGpio,
      BAST_4501_P_SetGpio,
      BAST_4501_P_GetGpio,
      BAST_4501_P_TuneAcquire,
      BAST_4501_P_GetChannelStatus,
      BAST_4501_P_GetLockStatus,
      BAST_4501_P_ResetStatus,
      BAST_4501_P_SetDiseqcTone,
      BAST_4501_P_GetDiseqcTone,
      BAST_4501_P_SetDiseqcVoltage,
      BAST_4501_P_SendDiseqcCommand,
      BAST_4501_P_GetDiseqcStatus,
      BAST_4501_P_ResetDiseqc,
      BAST_4501_P_ResetFtm,
      BAST_4501_P_ReadFtm,
      BAST_4501_P_WriteFtm,
      BAST_4501_P_PowerDownFtm,
      BAST_4501_P_PowerUpFtm,
      BAST_4501_P_WriteMi2c,
      BAST_4501_P_ReadMi2c,
      BAST_4501_P_GetSoftDecisionBuf,
      BAST_4501_P_ReadAgc,
      BAST_4501_P_WriteAgc,
      BAST_4501_P_FreezeAgc,
      BAST_4501_P_FreezeEq,
      BAST_4501_P_PowerDown,
      BAST_4501_P_PowerUp,
      BAST_4501_P_ReadRegister,
      BAST_4501_P_WriteRegister,
      BAST_4501_P_ReadConfig,
      BAST_4501_P_WriteConfig,
      BAST_4501_P_GetInterruptEventHandle,
      BAST_4501_P_GetLockStateChangeEventHandle,
      BAST_4501_P_GetFtmEventHandle,
      BAST_4501_P_GetDiseqcEventHandle,
      BAST_4501_P_HandleInterrupt_isr,
      BAST_4501_P_ProcessInterruptEvent,
      BAST_4501_P_AbortAcq,
      BAST_4501_P_ConfigLna,
      BAST_4501_P_GetLnaStatus,
      BAST_4501_P_ConfigAgc,
      NULL, /* BAST_SendACW not available on BCM4501 */
      NULL, /* BAST_GetDiseqcVoltage not implemented */
      NULL, /* BAST_GetDiseqcVsenseEventHandles not implemented */
      NULL, /* BAST_EnableVsenseInterrupts not implemented */
      NULL, /* BAST_PeakScan not supported */
      NULL, /* BAST_GetPeakScanStatus not supported */
      NULL, /* BAST_GetPeakScanEventHandle not supported */
      NULL, /* BAST_EnableStatusInterrupts not supported */
      NULL, /* BAST_GetStatusEventHandle not supported */
      NULL, /* BAST_ConfigBcm3445 not supported */
      NULL, /* BAST_MapBcm3445ToTuner not supported */
      NULL, /* BAST_GetBcm3445Status not supported */
      NULL, /* BAST_EnableSpurCanceller not supported */
      NULL, /* BAST_ResetChannel not supported */
      NULL, /* BAST_EnableDiseqcLnb not supported */
      BAST_4501_P_SetSearchRange,
      BAST_4501_P_GetSearchRange,
      BAST_4501_P_SetAmcScramblingSeq,
      BAST_4501_P_SetTunerFilter,
      NULL,  /* BAST_GetSignalDetectStatus not supported */
      BAST_4501_P_SetOutputTransportSettings,
      BAST_4501_P_GetOutputTransportSettings,
      BAST_4501_P_SetDiseqcSettings,
      BAST_4501_P_GetDiseqcSettings,
      BAST_4501_P_SetNetworkSpec,
      BAST_4501_P_GetNetworkSpec,
      NULL, /* BAST_ConfigTunerLna */
      NULL, /* BAST_GetTunerLnaStatus */
      NULL, /* BAST_SetFskChannel */
      NULL, /* BAST_GetFskChannel */
      NULL, /* BAST_SetPeakScanSymbolRateRange */
      NULL, /* BAST_GetPeakScanSymbolRateRange */
      NULL, /* BAST_SetAdcSelect */
      NULL, /* BAST_GetAdcSelect */
      NULL, /* BAST_GetVersionInfo */
   },
   BAST_NetworkSpec_eDefault /* enable internal tuner */
};


static const BAST_ChannelSettings defChnSettings =
{
   (uint8_t)0 /* use internal tuner */
};


/******************************************************************************
 BAST_4501_GetDefaultSettings()
******************************************************************************/
BERR_Code BAST_4501_GetDefaultSettings(
   BAST_Settings *pDefSettings /* [out] default settings */
)
{
   *pDefSettings = defDevSettings;
   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_4501_P_GetChannelDefaultSettings()
******************************************************************************/
BERR_Code BAST_4501_GetChannelDefaultSettings(
   BAST_Handle   h,                      /* [in] BAST handle */
   uint32_t      chnNo,                      /* [in] channel number */
   BAST_ChannelSettings *pChnDefSettings /* [out] default channel settings */
)
{
   BSTD_UNUSED(h);
   BSTD_UNUSED(chnNo);
   *pChnDefSettings = defChnSettings;
   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_4501_WriteHostRegister()
******************************************************************************/
BERR_Code BAST_4501_WriteHostRegister(BAST_Handle h, uint8_t address, uint8_t *data)
{
   return (BAST_WriteHostRegister(h, address, data));
}


/******************************************************************************
 BAST_4501_ReadHostRegister()
******************************************************************************/
BERR_Code BAST_4501_ReadHostRegister(BAST_Handle h, uint8_t address, uint8_t *data)
{
   return (BAST_ReadHostRegister(h, address, data));
}

