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
#include "bast_g3.h"
#include "bast_g3_priv.h"


static const BAST_Settings defDevSettings =
{
   {  /* host i2c settings */
      0x00, /* chipAddr */
      NULL, /* interruptEnableFunc */
      NULL  /* interruptEnableFuncParam */
   },
   { /* API function table */
      BAST_g3_P_Open,
      BAST_g3_P_Close,
      BAST_g3_P_GetTotalChannels,
      BAST_g3_GetChannelDefaultSettings,
      BAST_g3_P_OpenChannel,
      BAST_g3_P_CloseChannel,
      BAST_g3_P_GetDevice,
      BAST_g3_P_InitAp,
      BAST_g3_P_SoftReset,
      NULL, /* BAST_g3_P_GetApStatus, */
#ifdef BAST_HAS_LEAP
      NULL,
#else
      BAST_g3_P_GetApVersion,
#endif
      NULL, /* SetTmConfig, */
      NULL, /* GetTmConfig, */
      NULL, /* ConfigGpio */
      NULL, /* SetGpio */
      NULL, /* GetGpio */
      BAST_g3_P_TuneAcquire,
      BAST_g3_P_GetChannelStatus,
      BAST_g3_P_GetLockStatus,
      BAST_g3_P_ResetStatus,
      BAST_g3_P_SetDiseqcTone,
      BAST_g3_P_GetDiseqcTone,
      BAST_g3_P_SetDiseqcVoltage,
      BAST_g3_P_SendDiseqcCommand,
      BAST_g3_P_GetDiseqcStatus,
      BAST_g3_P_ResetDiseqc,
#ifndef BAST_EXCLUDE_FTM
      BAST_g3_P_ResetFtm,
      BAST_g3_P_ReadFtm,
      BAST_g3_P_WriteFtm,
      BAST_g3_P_PowerDownFtm,
      BAST_g3_P_PowerUpFtm,
#else
      NULL,
      NULL,
      NULL,
      NULL,
      NULL,
#endif
#ifndef BAST_EXCLUDE_MI2C
      BAST_g3_P_WriteMi2c,
      BAST_g3_P_ReadMi2c,
#else
      NULL,
      NULL,
#endif
      BAST_g3_P_GetSoftDecisionBuf,
#ifndef BAST_HAS_WFE
      BAST_g3_P_ReadAgc,
      BAST_g3_P_WriteAgc,
      BAST_g3_P_FreezeAgc,
#else
      NULL,
      NULL,
      NULL,
#endif
      BAST_g3_P_FreezeEq,
      BAST_g3_P_PowerDown,
      BAST_g3_P_PowerUp,
      BAST_g3_P_ReadRegister_isrsafe,
      BAST_g3_P_WriteRegister_isrsafe,
      BAST_g3_P_ReadConfig,
      BAST_g3_P_WriteConfig,
      NULL, /* GetInterruptEventHandle */
      BAST_g3_P_GetLockChangeEventHandle,
#ifndef BAST_EXCLUDE_FTM
      BAST_g3_P_GetFtmEventHandle,
#else
      NULL,
#endif
      BAST_g3_P_GetDiseqcEventHandle,
      NULL, /*  HandleInterrupt_isr */
      NULL, /* ProcessInterruptEvent */
      BAST_g3_P_AbortAcq,
      NULL, /* BAST_g3_P_ConfigLna, */
      NULL, /* BAST_g3_P_GetLnaStatus, */
      NULL, /* ConfigAgc */
#ifndef BAST_EXCLUDE_ACW
      BAST_g3_P_SendACW,
#else
      NULL,
#endif
      BAST_g3_P_GetDiseqcVoltage,
      BAST_g3_P_GetDiseqcVsenseEventHandles,
      BAST_g3_P_EnableVsenseInterrupts,
      BAST_g3_P_PeakScan,
      BAST_g3_P_GetPeakScanStatus,
      BAST_g3_P_GetPeakScanEventHandle,
#ifndef BAST_EXCLUDE_STATUS_EVENTS
      BAST_g3_P_EnableStatusInterrupts,
      BAST_g3_P_GetStatusEventHandle,
#else
      NULL,
      NULL,
#endif
#ifndef BAST_EXCLUDE_BCM3445
      BAST_g3_P_ConfigBcm3445,
      BAST_g3_P_MapBcm3445ToTuner,
      BAST_g3_P_GetBcm3445Status,
#else
      NULL,
      NULL,
      NULL,
#endif
      BAST_g3_P_EnableSpurCanceller,
      BAST_g3_P_ResetChannel,
      BAST_g3_P_EnableDiseqcLnb,
      BAST_g3_P_SetSearchRange,
      BAST_g3_P_GetSearchRange,
#ifndef BAST_EXCLUDE_LDPC
      BAST_g3_P_SetAmcScramblingSeq,
#else
      NULL,
#endif
      BAST_g3_P_SetTunerFilter,
      BAST_g3_P_GetSignalDetectStatus,
      BAST_g3_P_SetOutputTransportSettings,
      BAST_g3_P_GetOutputTransportSettings,
      BAST_g3_P_SetDiseqcSettings,
      BAST_g3_P_GetDiseqcSettings,
      BAST_g3_P_SetNetworkSpec,
      BAST_g3_P_GetNetworkSpec,
#ifndef BAST_HAS_WFE
      BAST_g3_P_TunerConfigLna,
      BAST_g3_P_TunerGetLnaStatus,
#else
      NULL,
      NULL,
#endif
      BAST_g3_P_SetFskChannel,
      BAST_g3_P_GetFskChannel,
      BAST_g3_P_SetPeakScanSymbolRateRange,
      BAST_g3_P_GetPeakScanSymbolRateRange,
#ifdef BAST_HAS_WFE
      BAST_g3_P_SetAdcSelect,
      BAST_g3_P_GetAdcSelect,
#else
      NULL,
      NULL,
#endif
#ifdef BAST_HAS_LEAP
      NULL,
#else
      BAST_g3_P_GetVersionInfo,
#endif
   },
   BAST_NetworkSpec_eDefault
};


static const BAST_ChannelSettings defChnSettings =
{
   (uint8_t)0
};


/******************************************************************************
 BAST_g3_GetDefaultSettings()
******************************************************************************/
BERR_Code BAST_g3_GetDefaultSettings(
   BAST_Settings *pDefSettings /* [out] default settings */
)
{
   *pDefSettings = defDevSettings;
   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_g3_P_GetChannelDefaultSettings()
******************************************************************************/
BERR_Code BAST_g3_GetChannelDefaultSettings(
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

