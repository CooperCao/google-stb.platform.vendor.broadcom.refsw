/***************************************************************************
 *     Copyright (c) 2003-2012, Broadcom Corporation
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
#include "bast_priv.h"
#include "bast_g2.h"
#include "bast_g2_priv.h"


BDBG_MODULE(bast_g2);


static const BAST_Settings defDevSettings =
{
   {  /* host i2c settings */
      0x00, /* chipAddr */
      NULL, /* interruptEnableFunc */
      NULL  /* interruptEnableFuncParam */
   },
   { /* API function table */
      BAST_g2_P_Open,
      BAST_g2_P_Close,
      BAST_g2_P_GetTotalChannels,
      BAST_g2_GetChannelDefaultSettings,
      BAST_g2_P_OpenChannel,
      BAST_g2_P_CloseChannel,
      BAST_g2_P_GetDevice,
      BAST_g2_P_InitAp,
      BAST_g2_P_SoftReset,
      BAST_g2_P_GetApStatus,
      BAST_g2_P_GetApVersion,
      NULL, /* BAST_SetTmConfig, */
      NULL, /* BAST_GetTmConfig, */
      NULL, /* BAST_ConfigGpio */
      NULL, /* BAST_SetGpio */
      NULL, /* BAST_GetGpio */
      BAST_g2_P_TuneAcquire,
      BAST_g2_P_GetChannelStatus,
      BAST_g2_P_GetLockStatus,
      BAST_g2_P_ResetStatus,
      BAST_g2_P_SetDiseqcTone,
      BAST_g2_P_GetDiseqcTone,
      BAST_g2_P_SetDiseqcVoltage,
      BAST_g2_P_SendDiseqcCommand,
      BAST_g2_P_GetDiseqcStatus,
      BAST_g2_P_ResetDiseqc,
      BAST_g2_P_ResetFtm,
      BAST_g2_P_ReadFtm,
      BAST_g2_P_WriteFtm,
      BAST_g2_P_PowerDownFtm,
      BAST_g2_P_PowerUpFtm,
      BAST_g2_P_WriteMi2c,
      BAST_g2_P_ReadMi2c,
      BAST_g2_P_GetSoftDecisionBuf,
      BAST_g2_P_ReadAgc,
      BAST_g2_P_WriteAgc,
      BAST_g2_P_FreezeAgc,
      BAST_g2_P_FreezeEq,
      BAST_g2_P_PowerDown,
      BAST_g2_P_PowerUp,
      BAST_g2_P_ReadRegister_isrsafe,
      BAST_g2_P_WriteRegister_isrsafe,
      BAST_g2_P_ReadConfig,
      BAST_g2_P_WriteConfig,
      NULL, /* BAST_GetInterruptEventHandle */
      BAST_g2_P_GetLockChangeEventHandle,
      BAST_g2_P_GetFtmEventHandle,
      BAST_g2_P_GetDiseqcEventHandle,
      NULL, /*  BAST_HandleInterrupt_isr */
      NULL, /* BAST_ProcessInterruptEvent */
      BAST_g2_P_AbortAcq,
      BAST_g2_P_ConfigLna,
      BAST_g2_P_GetLnaStatus,
      NULL, /* BAST_ConfigAgc */
      BAST_g2_P_SendACW,
      BAST_g2_P_GetDiseqcVoltage,
      BAST_g2_P_GetDiseqcVsenseEventHandles,
      BAST_g2_P_EnableVsenseInterrupts,
      BAST_g2_P_PeakScan,
      BAST_g2_P_GetPeakScanStatus,
      BAST_g2_P_GetPeakScanEventHandle,
      BAST_g2_P_EnableStatusInterrupts,
      BAST_g2_P_GetStatusEventHandle,
      BAST_g2_P_ConfigBcm3445,
      BAST_g2_P_MapBcm3445ToTuner,
      BAST_g2_P_GetBcm3445Status,
      NULL, /* BAST_EnableSpurCanceller */
      BAST_g2_P_ResetChannel,
      BAST_g2_P_EnableDiseqcLnb,
      BAST_g2_P_SetSearchRange,
      BAST_g2_P_GetSearchRange,
      BAST_g2_P_SetAmcScramblingSeq,
      BAST_g2_P_SetTunerFilter,
      NULL,  /* BAST_GetSignalDetectStatus */
      BAST_g2_P_SetOutputTransportSettings,
      BAST_g2_P_GetOutputTransportSettings,
      BAST_g2_P_SetDiseqcSettings,
      BAST_g2_P_GetDiseqcSettings,
      BAST_g2_P_SetNetworkSpec,
      BAST_g2_P_GetNetworkSpec,
      NULL, /* BAST_ConfigTunerLna */
      NULL, /* BAST_GetTunerLnaStatus */
      BAST_g2_P_SetFskChannel,
      BAST_g2_P_GetFskChannel,
      BAST_g2_P_SetPeakScanSymbolRateRange,
      BAST_g2_P_GetPeakScanSymbolRateRange,
      NULL, /* BAST_SetAdcSelect */
      NULL, /* BAST_GetAdcSelect */
      BAST_g2_P_GetVersionInfo
   },
   BAST_NetworkSpec_eDefault
};


static const BAST_ChannelSettings defChnSettings =
{
   (uint8_t)0
};


/******************************************************************************
 BAST_g2_GetDefaultSettings()
******************************************************************************/
BERR_Code BAST_g2_GetDefaultSettings(
   BAST_Settings *pDefSettings /* [out] default settings */
)
{
   *pDefSettings = defDevSettings;
   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_g2_P_GetChannelDefaultSettings()
******************************************************************************/
BERR_Code BAST_g2_GetChannelDefaultSettings(
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

