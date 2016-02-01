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
#include "bast.h"
#include "bast_priv.h"

BDBG_MODULE(bast);

#ifndef BAST_EXCLUDE_API_TABLE

/******************************************************************************
 BAST_Open()
******************************************************************************/
BERR_Code BAST_Open(
   BAST_Handle *h,         /* [out] BAST handle */
   BCHP_Handle hChip,      /* [in] chip handle */
   void        *pReg,      /* [in] pointer to register or i2c handle */
   BINT_Handle hInterrupt, /* [in] interrupt handle */
   const BAST_Settings *pDefSettings /* [in] default settings */
)
{
   BDBG_ASSERT(h);
   BDBG_ASSERT(pDefSettings);

   return (pDefSettings->api.Open(h, hChip, pReg, hInterrupt, pDefSettings));
}


/******************************************************************************
 BAST_Close()
******************************************************************************/
BERR_Code BAST_Close(
   BAST_Handle h   /* [in] BAST handle */
)
{
   BDBG_ASSERT(h);
   return (h->settings.api.Close(h));
}

/******************************************************************************
 BAST_GetTotalChannels()
******************************************************************************/
BERR_Code BAST_GetTotalChannels(
   BAST_Handle  h,             /* [in] BAST handle */
   uint32_t         *totalChannels /* [out] number of channels supported */
)
{
   BDBG_ASSERT(h);
   return (h->settings.api.GetTotalChannels(h, totalChannels));
}


/******************************************************************************
 BAST_GetChannelDefaultSettings()
******************************************************************************/
BERR_Code BAST_GetChannelDefaultSettings(
   BAST_Handle   h,                      /* [in] BAST handle */
   uint32_t      chnNo,                  /* [in] channel number */
   BAST_ChannelSettings *pChnDefSettings /* [out] default channel settings */
)
{
   BDBG_ASSERT(h);
   return (h->settings.api.GetChannelDefaultSettings(h, chnNo, pChnDefSettings));
}


/******************************************************************************
 BAST_OpenChannel()
******************************************************************************/
BERR_Code BAST_OpenChannel(
   BAST_Handle                h,               /* [in] BAST handle */
   BAST_ChannelHandle         *pChannelHandle, /* [out] BAST channel handle */
   uint32_t                   chnNo,           /* [in] channel number */
   const BAST_ChannelSettings *pSettings       /* [in] channel settings */
)
{
   BDBG_ASSERT(h);
   BDBG_ASSERT(pChannelHandle);
   return (h->settings.api.OpenChannel(h, pChannelHandle, chnNo, pSettings));
}


/******************************************************************************
 BAST_CloseChannel()
******************************************************************************/
BERR_Code BAST_CloseChannel(
   BAST_ChannelHandle h  /* [in] BAST channel handle */
)
{
   BDBG_ASSERT(h);
   return (h->pDevice->settings.api.CloseChannel(h));
}


/******************************************************************************
 BAST_GetDevice()
******************************************************************************/
BERR_Code BAST_GetDevice(
   BAST_ChannelHandle h,  /* [in] BAST channel handle */
   BAST_Handle *pDev      /* [out] BAST handle */
)
{
   BDBG_ASSERT(h);
   return (h->pDevice->settings.api.GetDevice(h, pDev));
}

/******************************************************************************
 BAST_SoftReset()
******************************************************************************/
BERR_Code BAST_SoftReset(
   BAST_Handle h           /* [in] BAST device handle */
)
{
   BDBG_ASSERT(h);
   return (h->settings.api.SoftReset(h));
}


/******************************************************************************
 BAST_ResetChannel()
******************************************************************************/
BERR_Code BAST_ResetChannel(
   BAST_ChannelHandle h,  /* [in] BAST channel handle */
   bool bForceReset       /* [in] true=force reset, false=abort when busy */
)
{
   BDBG_ASSERT(h);
   if (h->pDevice->settings.api.ResetChannel)
      return (h->pDevice->settings.api.ResetChannel(h, bForceReset));
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BAST_InitAp()
******************************************************************************/
BERR_Code BAST_InitAp(
   BAST_Handle   h,   /* [in] BAST handle */
   const uint8_t *pImage  /* [in] pointer to AP microcode image */
)
{
   BDBG_ASSERT(h);
   return (h->settings.api.InitAp(h, pImage));
}


/******************************************************************************
 BAST_GetApStatus()
******************************************************************************/
BERR_Code BAST_GetApStatus(
   BAST_Handle   h,         /* [in] BAST device handle */
   BAST_ApStatus *pStatus   /* [out] AP status */
)
{
   BDBG_ASSERT(h);
   return (h->settings.api.GetApStatus(h, pStatus));
}


/******************************************************************************
 BAST_GetApVersion()
******************************************************************************/
BERR_Code BAST_GetApVersion(
   BAST_Handle h,          /* [in] BAST handle */
   uint16_t    *pChipId,   /* [out] BAST chip id */
   uint8_t     *pChipVer,  /* [out] chip revision number */
   uint32_t    *pBondId,   /* [out] chip bonding option */
   uint8_t     *pApVer,    /* [out] AP microcode version */
   uint8_t     *pCfgVer    /* [out] host configuration version */
)
{
   BDBG_ASSERT(h);
   return (h->settings.api.GetApVersion(h, pChipId, pChipVer, pBondId, pApVer, pCfgVer));
}


/******************************************************************************
 BAST_SetTmConfig()
******************************************************************************/
BERR_Code BAST_SetTmConfig(
   BAST_Handle h,     /* [in] BAST handle */
   void        *pCfg  /* [in] TM configuration parameters */
)
{
   BDBG_ASSERT(h);
   if (h->settings.api.SetTmConfig)
      return (h->settings.api.SetTmConfig(h, pCfg));
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BAST_GetTmConfig()
******************************************************************************/
BERR_Code BAST_GetTmConfig(
   BAST_Handle h,     /* [in] BAST handle */
   void        *pCfg  /* [out] TM configuration parameters */
)
{
   BDBG_ASSERT(h);
   if (h->settings.api.GetTmConfig)
      return (h->settings.api.GetTmConfig(h, pCfg));
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BAST_ConfigGpio()
******************************************************************************/
BERR_Code BAST_ConfigGpio(
   BAST_Handle h,           /* [in] BAST handle */
   uint32_t    write_mask,  /* [in] selects which GPIO pins are output */
   uint32_t    read_mask    /* [in] selects which GPIO pins are input */
)
{
   BDBG_ASSERT(h);
   if (h->settings.api.ConfigGpio)
      return (h->settings.api.ConfigGpio(h, write_mask, read_mask));
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BAST_SetGpio()
******************************************************************************/
BERR_Code BAST_SetGpio(
   BAST_Handle h,     /* [in] BAST handle */
   uint32_t    mask,  /* [in] selects which GPIO pins to set */
   uint32_t    state  /* [in] state of GPIO pins */
)
{
   BDBG_ASSERT(h);
   if (h->settings.api.SetGpio)
      return (h->settings.api.SetGpio(h, mask, state));
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BAST_GetGpio()
******************************************************************************/
BERR_Code BAST_GetGpio(
   BAST_Handle h,      /* [in] BAST handle */
   uint32_t    mask,   /* [in] selects which GPIO pins to read */
   uint32_t    *state  /* [out] state of GPIO pins */
)
{
   BDBG_ASSERT(h);
   if (h->settings.api.GetGpio)
      return (h->settings.api.GetGpio(h, mask, state));
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BAST_TuneAcquire()
******************************************************************************/
BERR_Code BAST_TuneAcquire(
   BAST_ChannelHandle h,            /* [in] BAST channel handle */
   const uint32_t     freq,         /* [in] RF tuner frequency in Hz */
   const BAST_AcqSettings *pParams  /* [in] acquisition parameters */
)
{
   BDBG_ASSERT(h);
   BDBG_ASSERT(pParams);
   if (pParams == NULL)
      return BERR_TRACE(BERR_INVALID_PARAMETER);
   else
      return (h->pDevice->settings.api.TuneAcquire(h, freq, pParams));
}


/******************************************************************************
 BAST_GetChannelStatus()
******************************************************************************/
BERR_Code BAST_GetChannelStatus(
   BAST_ChannelHandle h,        /* [in] BAST handle */
   BAST_ChannelStatus *pStatus  /* [out] pointer to status structure */
)
{
   BDBG_ASSERT(h);
   BDBG_ASSERT(pStatus);
   return (h->pDevice->settings.api.GetChannelStatus(h, pStatus));
}


/******************************************************************************
 BAST_GetLockStatus()
******************************************************************************/
BERR_Code BAST_GetLockStatus(
   BAST_ChannelHandle h,  /* [in] BAST handle */
   bool *bLocked          /* [out] true if demod is locked */
)
{
   BDBG_ASSERT(h);
   BDBG_ASSERT(bLocked);
   return (h->pDevice->settings.api.GetLockStatus(h, bLocked));
}


/******************************************************************************
 BAST_ResetStatus()
******************************************************************************/
BERR_Code BAST_ResetStatus(
   BAST_ChannelHandle h  /* [in] BAST channel handle */
)
{
   BDBG_ASSERT(h);
   return (h->pDevice->settings.api.ResetStatus(h));
}


/******************************************************************************
 BAST_SetDiseqcTone()
******************************************************************************/
BERR_Code BAST_SetDiseqcTone(
   BAST_ChannelHandle h,  /* [in] BAST channel handle */
   bool bTone             /* [in] true = tone on, false = tone off */
)
{
   BDBG_ASSERT(h);
   return (h->pDevice->settings.api.SetDiseqcTone(h, bTone));
}


/******************************************************************************
 BAST_GetDiseqcTone()
******************************************************************************/
BERR_Code BAST_GetDiseqcTone(
   BAST_ChannelHandle h,  /* [in] BAST channel handle */
   bool *bTone            /* [out] true = tone present, false = tone absent */
)
{
   BDBG_ASSERT(h);
   return (h->pDevice->settings.api.GetDiseqcTone(h, bTone));
}


/******************************************************************************
 BAST_SetDiseqcVoltage()
******************************************************************************/
BERR_Code BAST_SetDiseqcVoltage(
   BAST_ChannelHandle h, /* [in] BAST channel handle */
   bool bVtop            /* [in] true = VTOP, false = VBOT */
)
{
   BDBG_ASSERT(h);
   return (h->pDevice->settings.api.SetDiseqcVoltage(h, bVtop));
}


/******************************************************************************
 BAST_GetDiseqcVoltage()
******************************************************************************/
BERR_Code BAST_GetDiseqcVoltage(
   BAST_ChannelHandle h,   /* [in] BAST channel handle */
   uint8_t *pVoltage       /* [out] voltage estimation */
)
{
   BDBG_ASSERT(h);
   if (h->pDevice->settings.api.GetDiseqcVoltage)
      return (h->pDevice->settings.api.GetDiseqcVoltage(h, pVoltage));
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BAST_EnableDiseqcLnb()
******************************************************************************/
BERR_Code BAST_EnableDiseqcLnb(
   BAST_ChannelHandle h,  /* [in] BAST channel handle */
   bool bEnable           /* [in] true = LNB on, false = LNB off */
)
{
   BDBG_ASSERT(h);
   if (h->pDevice->settings.api.EnableDiseqcLnb)
      return (h->pDevice->settings.api.EnableDiseqcLnb(h, bEnable));
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BAST_EnableVsenseInterrupts()
******************************************************************************/
BERR_Code BAST_EnableVsenseInterrupts(
   BAST_ChannelHandle h,   /* [in] BAST channel handle */
   bool bEnable            /* [in] true = enable, false = disable */
)
{
   BDBG_ASSERT(h);
   if (h->pDevice->settings.api.EnableVsenseInterrupts)
      return (h->pDevice->settings.api.EnableVsenseInterrupts(h, bEnable));
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BAST_SendDiseqcCommand()
******************************************************************************/
BERR_Code BAST_SendDiseqcCommand(
   BAST_ChannelHandle h,       /* [in] BAST channel handle */
   const uint8_t *pSendBuf,    /* [in] contains data to send */
   uint8_t sendBufLen          /* [in] number of bytes to send */
)
{
   BDBG_ASSERT(h);
   return (h->pDevice->settings.api.SendDiseqcCommand(h, pSendBuf, sendBufLen));
}


/******************************************************************************
 BAST_GetDiseqcStatus()
******************************************************************************/
BERR_Code BAST_GetDiseqcStatus(
   BAST_ChannelHandle h,       /* [in] BAST channel handle */
   BAST_DiseqcStatus *pStatus  /* [out] status of most recent transaction */
)
{
   BDBG_ASSERT(h);
   return (h->pDevice->settings.api.GetDiseqcStatus(h, pStatus));
}


/******************************************************************************
 BAST_ResetDiseqc()
******************************************************************************/
BERR_Code BAST_ResetDiseqc(
   BAST_ChannelHandle h,    /* [in] BAST channel handle */
   uint8_t options          /* [in] reset options */
)
{
   BDBG_ASSERT(h);
   return (h->pDevice->settings.api.ResetDiseqc(h, options));
}


/******************************************************************************
 BERR_Code BAST_ResetFtm()
******************************************************************************/
BERR_Code BAST_ResetFtm(
   BAST_Handle h  /* [in] BAST handle */
)
{
   BDBG_ASSERT(h);
   if (h->settings.api.ResetFtm)
      return (h->settings.api.ResetFtm(h));
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BERR_Code BAST_ReadFtm()
******************************************************************************/
BERR_Code BAST_ReadFtm(
   BAST_Handle h,  /* [in] BAST handle */
   uint8_t *pBuf,  /* [out] data read */
   uint8_t *n      /* [out] length of data read */
)
{
   BDBG_ASSERT(h);
   if (h->settings.api.ReadFtm)
      return (h->settings.api.ReadFtm(h, pBuf, n));
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BERR_Code BAST_WriteFtm()
******************************************************************************/
BERR_Code BAST_WriteFtm(
   BAST_Handle h,  /* [in] BAST handle */
   uint8_t *pBuf,  /* [in] data to write */
   uint8_t n       /* [in] length of data to write */
)
{
   BDBG_ASSERT(h);
   if (h->settings.api.WriteFtm)
      return (h->settings.api.WriteFtm(h, pBuf, n));
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BAST_PowerDownFtm()
******************************************************************************/
BERR_Code BAST_PowerDownFtm(
   BAST_Handle h  /* [in] BAST handle */
)
{
   BDBG_ASSERT(h);
   if (h->settings.api.PowerDownFtm)
      return (h->settings.api.PowerDownFtm(h));
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BAST_PowerUpFtm()
******************************************************************************/
BERR_Code BAST_PowerUpFtm(
   BAST_Handle h  /* [in] BAST handle */
)
{
   BDBG_ASSERT(h);
   if (h->settings.api.PowerUpFtm)
      return (h->settings.api.PowerUpFtm(h));
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BAST_WriteMi2c()
******************************************************************************/
BERR_Code BAST_WriteMi2c(
   BAST_ChannelHandle h, /* [in] BAST channel handle */
   uint8_t slave_addr,   /* [in] address of the i2c slave device */
   uint8_t *buf,         /* [in] specifies the data to transmit */
   uint8_t n             /* [in] number of bytes to transmit after the i2c slave address */
)
{
   BDBG_ASSERT(h);
   if (h->pDevice->settings.api.WriteMi2c)
      return (h->pDevice->settings.api.WriteMi2c(h, slave_addr, buf, n));
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BAST_ReadMi2c()
******************************************************************************/
BERR_Code BAST_ReadMi2c(
   BAST_ChannelHandle h, /* [in] BAST channel handle */
   uint8_t slave_addr,   /* [in] address of the i2c slave device */
   uint8_t *out_buf,     /* [in] specifies the data to transmit before the i2c restart condition */
   uint8_t out_n,        /* [in] number of bytes to transmit before the i2c restart condition not including the i2c slave address */
   uint8_t *in_buf,      /* [out] holds the data read */
   uint8_t in_n          /* [in] number of bytes to read after the i2c restart condition not including the i2c slave address */
)
{
   BDBG_ASSERT(h);
   if (h->pDevice->settings.api.ReadMi2c)
      return (h->pDevice->settings.api.ReadMi2c(h, slave_addr, out_buf, out_n, in_buf, in_n));
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BAST_GetSoftDecisionBuf()
******************************************************************************/
BERR_Code BAST_GetSoftDecisionBuf(
   BAST_ChannelHandle h,  /* [in] BAST channel handle */
   int16_t *pI,           /* [out] 30 I-values */
   int16_t *pQ            /* [out] 30 Q-values */
)
{
   BDBG_ASSERT(h);
   return (h->pDevice->settings.api.GetSoftDecisionBuf(h, pI, pQ));
}

/******************************************************************************
 BERR_Code BAST_ReadAgc()
******************************************************************************/
BERR_Code BAST_ReadAgc(
   BAST_ChannelHandle h,  /* [in] BAST handle */
   BAST_Agc which_agc,    /* [in] AGC select */
   uint32_t *pAgc         /* [out] integrator value */
)
{
   BDBG_ASSERT(h);
   if (h->pDevice->settings.api.ReadAgc)
      return (h->pDevice->settings.api.ReadAgc(h, which_agc, pAgc));
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BERR_Code BAST_WriteAgc()
******************************************************************************/
BERR_Code BAST_WriteAgc(
   BAST_ChannelHandle h,  /* [in] BAST handle */
   BAST_Agc which_agc,    /* [in] AGC select */
   uint32_t *pAgc         /* [in] integrator value */
)
{
   BDBG_ASSERT(h);
   if (h->pDevice->settings.api.WriteAgc)
      return (h->pDevice->settings.api.WriteAgc(h, which_agc, pAgc));
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BERR_Code BAST_FreezeAgc()
******************************************************************************/
BERR_Code BAST_FreezeAgc(
   BAST_ChannelHandle h,  /* [in] BAST handle */
   BAST_Agc which_agc,    /* [in] AGC select */
   bool bFreeze           /* [in] true = freeze, false = unfreeze */
)
{
   BDBG_ASSERT(h);
   if (h->pDevice->settings.api.FreezeAgc)
      return (h->pDevice->settings.api.FreezeAgc(h, which_agc, bFreeze));
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BERR_Code BAST_FreezeEq()
******************************************************************************/
BERR_Code BAST_FreezeEq(
   BAST_ChannelHandle h,  /* [in] BAST handle */
   bool bFreeze           /* [in] true = freeze, false = unfreeze */
)
{
   BDBG_ASSERT(h);
   if (h->pDevice->settings.api.FreezeEq)
      return (h->pDevice->settings.api.FreezeEq(h, bFreeze));
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BAST_PowerDown()
******************************************************************************/
BERR_Code BAST_PowerDown(
   BAST_ChannelHandle h,  /* [in] BAST handle */
   uint32_t options       /* [in] see BAST_CORE_* macros */
)
{
   BDBG_ASSERT(h);
   if (h->pDevice->settings.api.PowerDown)
      return (h->pDevice->settings.api.PowerDown(h, options));
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BAST_PowerUp()
******************************************************************************/
BERR_Code BAST_PowerUp(
   BAST_ChannelHandle h,  /* [in] BAST handle */
   uint32_t options       /* [in] see BAST_CORE_* macros */
)
{
   BDBG_ASSERT(h);
   if (h->pDevice->settings.api.PowerUp)
      return (h->pDevice->settings.api.PowerUp(h, options));
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BAST_ReadRegister()
******************************************************************************/
BERR_Code BAST_ReadRegister(
   BAST_ChannelHandle h,     /* [in] BAST channel handle */
   uint32_t           reg,   /* [in] address of register to read */
   uint32_t           *val   /* [in] contains data that was read */
)
{
   BDBG_ASSERT(h);
   return (h->pDevice->settings.api.ReadRegister(h, reg, val));
}


/******************************************************************************
 BAST_WriteRegister()
******************************************************************************/
BERR_Code BAST_WriteRegister(
   BAST_ChannelHandle h,     /* [in] BAST channel handle */
   uint32_t           reg,   /* [in] address of register to write */
   uint32_t           *val   /* [in] contains data to write */
)
{
   BDBG_ASSERT(h);
   return (h->pDevice->settings.api.WriteRegister(h, reg, val));
}


/******************************************************************************
 BAST_ReadConfig()
******************************************************************************/
BERR_Code BAST_ReadConfig(
   BAST_ChannelHandle h, /* [in] BAST channel handle */
   uint16_t id,          /* [in] configuration parameter ID */
   uint8_t *buf,         /* [out] bytes read */
   uint8_t n             /* [in] length of configuration parameter */
)
{
   BDBG_ASSERT(h);
   return (h->pDevice->settings.api.ReadConfig(h, id, buf, n));
}


/******************************************************************************
 BAST_WriteConfig()
******************************************************************************/
BERR_Code BAST_WriteConfig(
   BAST_ChannelHandle h, /* [in] BAST channel handle */
   uint16_t id,          /* [in] configuration parameter ID */
   uint8_t *buf,         /* [in] bytes to write */
   uint8_t n             /* [in] length of configuration parameter */
)
{
   BDBG_ASSERT(h);
   return (h->pDevice->settings.api.WriteConfig(h, id, buf, n));
}


/******************************************************************************
 BAST_GetInterruptEventHandle()
******************************************************************************/
BERR_Code BAST_GetInterruptEventHandle(
   BAST_Handle h,            /* [in] BAST handle */
   BKNI_EventHandle *phEvent /* [out] event handle */
)
{
   BDBG_ASSERT(h);
   if (h->settings.api.GetInterruptEventHandle)
      return (h->settings.api.GetInterruptEventHandle(h, phEvent));
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BAST_GetLockStateChangeEventHandle()
******************************************************************************/
BERR_Code BAST_GetLockStateChangeEventHandle(
   BAST_ChannelHandle h,     /* [in] BAST channel handle */
   BKNI_EventHandle *hEvent  /* [out] lock event handle */
)
{
   BDBG_ASSERT(h);
   return (h->pDevice->settings.api.GetLockStateChangeEventHandle(h, hEvent));
}


/******************************************************************************
 BAST_GetFtmEventHandle()
******************************************************************************/
BERR_Code BAST_GetFtmEventHandle(
   BAST_Handle      h,       /* [in] BAST handle */
   BKNI_EventHandle *hEvent  /* [out] FTM event handle */
)
{
   BDBG_ASSERT(h);
   if (h->settings.api.GetFtmEventHandle)
      return (h->settings.api.GetFtmEventHandle(h, hEvent));
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BAST_GetDiseqcEventHandle()
******************************************************************************/
BERR_Code BAST_GetDiseqcEventHandle(
   BAST_ChannelHandle h,       /* [in] BAST channel handle */
   BKNI_EventHandle   *hEvent  /* [out] Diseqc event handle */
)
{
   BDBG_ASSERT(h);
   if (h->pDevice->settings.api.GetDiseqcEventHandle)
      return (h->pDevice->settings.api.GetDiseqcEventHandle(h, hEvent));
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BAST_GetDiseqcVsenseEventHandles()
******************************************************************************/
BERR_Code BAST_GetDiseqcVsenseEventHandles(
   BAST_ChannelHandle h,                  /* [in] BAST channel handle */
   BKNI_EventHandle   *hOverVoltageEvent, /* [out] Diseqc event handle */
   BKNI_EventHandle   *hUnderVoltageEvent /* [out] Diseqc event handle */
)
{
   BDBG_ASSERT(h);
   if (h->pDevice->settings.api.GetDiseqcVsenseEventHandles)
      return (h->pDevice->settings.api.GetDiseqcVsenseEventHandles(h, hOverVoltageEvent, hUnderVoltageEvent));
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BAST_HandleInterrupt_isr()
******************************************************************************/
BERR_Code BAST_HandleInterrupt_isr(
   BAST_Handle h   /* [in] BAST handle */
)
{
   BDBG_ASSERT(h);
   return (h->settings.api.HandleInterrupt_isr(h));
}


/******************************************************************************
 BAST_ProcessInterruptEvent()
******************************************************************************/
BERR_Code BAST_ProcessInterruptEvent(
   BAST_Handle h  /* [in] BAST handle */
)
{
   BDBG_ASSERT(h);
   return (h->settings.api.ProcessInterruptEvent(h));
}


/******************************************************************************
 BAST_AbortAcq()
******************************************************************************/
BERR_Code BAST_AbortAcq(
   BAST_ChannelHandle h  /* [in] BAST channel handle */
)
{
   BDBG_ASSERT(h);
   if (h->pDevice->settings.api.AbortAcq)
      return (h->pDevice->settings.api.AbortAcq(h));
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BAST_ConfigLna()
******************************************************************************/
BERR_Code BAST_ConfigLna(
   BAST_Handle    h,              /* [in] BAST handle */
   BAST_LnaIntConfig int_config,  /* [in] internal switch configuration */
   BAST_LnaExtConfig ext_config   /* [in] external configuration */
)
{
   if (h->settings.api.ConfigLna)
      return (h->settings.api.ConfigLna(h, int_config, ext_config));
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BAST_GetLnaStatus()
******************************************************************************/
BERR_Code BAST_GetLnaStatus(
   BAST_Handle    h,        /* [in] BAST handle */
   BAST_LnaStatus *pStatus  /* [out] lna status struct */
)
{
   if (h->settings.api.GetLnaStatus)
      return (h->settings.api.GetLnaStatus(h, pStatus));
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BAST_ConfigAgc()
******************************************************************************/
BERR_Code BAST_ConfigAgc(
   BAST_Handle h,    /* [in] BAST handle */
   uint32_t    ctl   /* [in] control word */
)
{
   if (h->settings.api.ConfigAgc)
      return (h->settings.api.ConfigAgc(h, ctl));
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BAST_SendACW()
******************************************************************************/
BERR_Code BAST_SendACW(
   BAST_ChannelHandle h, /* [in] BAST channel handle */
   uint8_t acw           /* [in] auto control word to send */
)
{
   if (h->pDevice->settings.api.SendACW)
      return (h->pDevice->settings.api.SendACW(h, acw));
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BAST_PeakScan()
******************************************************************************/
BERR_Code BAST_PeakScan(
   BAST_ChannelHandle h,           /* [in] BAST channel handle */
   uint32_t           tunerFreq    /* [in] frequency in Hz */
)
{
   if (h->pDevice->settings.api.PeakScan)
      return (h->pDevice->settings.api.PeakScan(h, tunerFreq));
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BAST_GetPeakScanStatus()
******************************************************************************/
BERR_Code BAST_GetPeakScanStatus(
   BAST_ChannelHandle  h,          /* [in] BAST channel handle */
   BAST_PeakScanStatus *pStatus    /* [out] Diseqc event handle */
)
{
   if (h->pDevice->settings.api.GetPeakScanStatus)
      return (h->pDevice->settings.api.GetPeakScanStatus(h, pStatus));
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BAST_GetPeakScanEventHandle()
******************************************************************************/
BERR_Code BAST_GetPeakScanEventHandle(
   BAST_ChannelHandle h,                /* [in] BAST channel handle */
   BKNI_EventHandle  *hPeakScanEvent    /* [out] Peak scan done event handle */
)
{
   if (h->pDevice->settings.api.GetPeakScanEventHandle)
      return (h->pDevice->settings.api.GetPeakScanEventHandle(h, hPeakScanEvent));
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BAST_EnableStatusInterrupts()
******************************************************************************/
BERR_Code BAST_EnableStatusInterrupts(
   BAST_ChannelHandle h,   /* [in] BAST channel handle */
   bool bEnable            /* [in] true = enable, false = disable */
)
{
   if (h->pDevice->settings.api.EnableStatusInterrupts)
      return (h->pDevice->settings.api.EnableStatusInterrupts(h, bEnable));
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BAST_GetStatusEventHandle()
******************************************************************************/
BERR_Code BAST_GetStatusEventHandle(
   BAST_ChannelHandle h,        /* [in] BAST channel handle */
   BKNI_EventHandle  *hEvent    /* [out] status event handle */
)
{
   if (h->pDevice->settings.api.GetStatusEventHandle)
      return (h->pDevice->settings.api.GetStatusEventHandle(h, hEvent));
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BAST_ConfigBcm3445()
******************************************************************************/
BERR_Code BAST_ConfigBcm3445(
   BAST_Handle h,                  /* [in] BAST device handle */
   BAST_Bcm3445Settings *pSettings /* [in] BCM3445 configuration settings */
)
{
   if (h->settings.api.ConfigBcm3445)
      return (h->settings.api.ConfigBcm3445(h, pSettings));
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BAST_MapBcm3445ToTuner()
******************************************************************************/
BERR_Code BAST_MapBcm3445ToTuner(
   BAST_ChannelHandle        h,    /* [in] BAST channel handle */
   BAST_Mi2cChannel          mi2c, /* [in] specifies which BCM3445 */
   BAST_Bcm3445OutputChannel out   /* [in] BCM3445 output channel */
)
{
   if (h->pDevice->settings.api.MapBcm3445ToTuner)
      return (h->pDevice->settings.api.MapBcm3445ToTuner(h, mi2c, out));
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BAST_GetBcm3445Status()
******************************************************************************/
BERR_Code BAST_GetBcm3445Status(
   BAST_ChannelHandle h,       /* [in] BAST channel handle */
   BAST_Bcm3445Status *pStatus /* [out] BCM3445 status struct */
)
{
   if (h->pDevice->settings.api.GetBcm3445Status)
      return (h->pDevice->settings.api.GetBcm3445Status(h, pStatus));
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BAST_ConfigTunerLna()
******************************************************************************/
BERR_Code BAST_ConfigTunerLna(
   BAST_Handle h,                   /* [in] BAST device handle */
   BAST_TunerLnaSettings *pSettings /* [in] Tuner LNA configuration settings */
)
{
   if (h->settings.api.ConfigTunerLna)
      return (h->settings.api.ConfigTunerLna(h, pSettings));
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BAST_GetTunerLnaStatus()
******************************************************************************/
BERR_Code BAST_GetTunerLnaStatus(
   BAST_ChannelHandle h,         /* [in] BAST channel handle */
   BAST_TunerLnaStatus *pStatus  /* [out] Tuner LNA status struct */
)
{
   if (h->pDevice->settings.api.GetTunerLnaStatus)
      return (h->pDevice->settings.api.GetTunerLnaStatus(h, pStatus));
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BAST_EnableSpurCanceller()
******************************************************************************/
BERR_Code BAST_EnableSpurCanceller(
   BAST_ChannelHandle h,       /* [in] BAST channel handle */
   uint8_t            n,       /* [in] number of spurs to cancel (i.e. size of pConfig array) */
   BAST_SpurCancellerConfig *pConfig /* [in] array of notch filter settings */
)
{
   if (h->pDevice->settings.api.EnableSpurCanceller)
      return (h->pDevice->settings.api.EnableSpurCanceller(h, n, pConfig));
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BAST_SetSearchRange()
******************************************************************************/
BERR_Code BAST_SetSearchRange(
   BAST_Handle h,           /* [in] BAST device handle */
   uint32_t searchRange   /* [in] search range in Hz */
)
{
   if (h->settings.api.SetSearchRange)
      return (h->settings.api.SetSearchRange(h, searchRange));
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BAST_GetSearchRange()
******************************************************************************/
BERR_Code BAST_GetSearchRange(
   BAST_Handle h,           /* [in] BAST device handle */
   uint32_t *pSearchRange   /* [in] search range in Hz */
)
{
   if (h->settings.api.GetSearchRange)
      return (h->settings.api.GetSearchRange(h, pSearchRange));
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BAST_SetAmcScramblingSeq()
******************************************************************************/
BERR_Code BAST_SetAmcScramblingSeq(
   BAST_ChannelHandle h, /* [in] BAST channel handle */
   uint32_t xseed,       /* [in] physical layer scrambler seed */
   uint32_t plhdrscr1,   /* [in] PL Header Scrambling Sequence Bit[31:0] */
   uint32_t plhdrscr2,   /* [in] PL Header Scrambling Sequence Bit[63:32] */
   uint32_t plhdrscr3    /* [in] PL Header Scrambling Sequence Bit[89:64] */
)
{
   if (h->pDevice->settings.api.SetAmcScramblingSeq)
      return (h->pDevice->settings.api.SetAmcScramblingSeq(h, xseed, plhdrscr1, plhdrscr2, plhdrscr3));
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BAST_GetSignalDetectStatus()
******************************************************************************/
BERR_Code BAST_GetSignalDetectStatus(
   BAST_ChannelHandle      h,       /* [in] BAST channel handle */
   BAST_SignalDetectStatus *pStatus /* [out] returned status */
)
{
   if (h->pDevice->settings.api.GetSignalDetectStatus)
      return (h->pDevice->settings.api.GetSignalDetectStatus(h, pStatus));
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BAST_SetTunerFilter()
******************************************************************************/
BERR_Code BAST_SetTunerFilter(
   BAST_ChannelHandle h,       /* [in] BAST channel handle */
   uint32_t           cutoffHz /* [in] filter cutoff in Hz */
)
{
   if (h->pDevice->settings.api.SetTunerFilter)
      return (h->pDevice->settings.api.SetTunerFilter(h, cutoffHz));
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BAST_SetOutputTransportSettings()
******************************************************************************/
BERR_Code BAST_SetOutputTransportSettings(
   BAST_ChannelHandle h,                   /* [in] BAST channel handle */
   BAST_OutputTransportSettings *pSettings /* [in] transport settings */
)
{
   if (h->pDevice->settings.api.SetOutputTransportSettings)
      return (h->pDevice->settings.api.SetOutputTransportSettings(h, pSettings));
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BAST_GetOutputTransportSettings()
******************************************************************************/
BERR_Code BAST_GetOutputTransportSettings(
   BAST_ChannelHandle h,                   /* [in] BAST channel handle */
   BAST_OutputTransportSettings *pSettings /* [out] transport settings */
)
{
   if (h->pDevice->settings.api.GetOutputTransportSettings)
      return (h->pDevice->settings.api.GetOutputTransportSettings(h, pSettings));
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BAST_SetDiseqcSettings()
******************************************************************************/
BERR_Code BAST_SetDiseqcSettings(
   BAST_ChannelHandle h,            /* [in] BAST channel handle */
   BAST_DiseqcSettings *pSettings   /* [in] diseqc settings */
)
{
   if (h->pDevice->settings.api.SetDiseqcSettings)
      return (h->pDevice->settings.api.SetDiseqcSettings(h, pSettings));
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BAST_GetDiseqcSettings()
******************************************************************************/
BERR_Code BAST_GetDiseqcSettings(
   BAST_ChannelHandle h,            /* [in] BAST channel handle */
   BAST_DiseqcSettings *pSettings   /* [out] diseqc settings */
)
{
   if (h->pDevice->settings.api.GetDiseqcSettings)
      return (h->pDevice->settings.api.GetDiseqcSettings(h, pSettings));
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BAST_SetNetworkSpec()
******************************************************************************/
BERR_Code BAST_SetNetworkSpec(
   BAST_Handle      h,            /* [in] BAST channel handle */
   BAST_NetworkSpec networkSpec   /* [in] network spec */
)
{
   if (h->settings.api.SetNetworkSpec)
      return (h->settings.api.SetNetworkSpec(h, networkSpec));
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BAST_GetNetworkSpec()
******************************************************************************/
BERR_Code BAST_GetNetworkSpec(
   BAST_Handle      h,            /* [in] BAST channel handle */
   BAST_NetworkSpec *pNetworkSpec /* [out] network spec */
)
{
   if (h->settings.api.GetNetworkSpec)
      return (h->settings.api.GetNetworkSpec(h, pNetworkSpec));
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BAST_SetNetworkSpec()
******************************************************************************/
BERR_Code BAST_SetFskChannel(
   BAST_Handle     h,               /* [in] BAST device handle */
   BAST_FskChannel fskTxChannel,    /* [in] channel for fsk transmit */
   BAST_FskChannel fskRxChannel     /* [in] channel for fsk receive */
)
{
   if (h->settings.api.SetFskChannel)
      return (h->settings.api.SetFskChannel(h, fskTxChannel, fskRxChannel));
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BAST_GetNetworkSpec()
******************************************************************************/
BERR_Code BAST_GetFskChannel(
   BAST_Handle     h,               /* [in] BAST device handle */
   BAST_FskChannel *fskTxChannel,   /* [out] channel for fsk transmit */
   BAST_FskChannel *fskRxChannel    /* [out] channel for fsk receive */
)
{
   if (h->settings.api.GetFskChannel)
      return (h->settings.api.GetFskChannel(h, fskTxChannel, fskRxChannel));
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BAST_SetPeakScanSymbolRateRange()
******************************************************************************/  
BERR_Code BAST_SetPeakScanSymbolRateRange(
   BAST_ChannelHandle h,             /* [in] BAST channel handle */
   uint32_t           minSymbolRate, /* [in] minimum symbol rate in sym/sec */
   uint32_t           maxSymbolRate  /* [in] maximum symbol rate in sym/sec */
)
{
   if (h->pDevice->settings.api.SetPeakScanSymbolRateRange)
      return (h->pDevice->settings.api.SetPeakScanSymbolRateRange(h, minSymbolRate, maxSymbolRate));
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BAST_SetAdcSelect()
******************************************************************************/  
BERR_Code BAST_SetAdcSelect(BAST_ChannelHandle h, uint8_t adcSelect)
{
   if (h->pDevice->settings.api.SetAdcSelect)
      return (h->pDevice->settings.api.SetAdcSelect(h, adcSelect));
   else
      return BERR_NOT_SUPPORTED;   
}


/******************************************************************************
 BAST_GetAdcSelect()
******************************************************************************/  
BERR_Code BAST_GetAdcSelect(BAST_ChannelHandle h, uint8_t *pAdcSelect, uint8_t *pNumAdc)
{
   if (h->pDevice->settings.api.GetAdcSelect)
      return (h->pDevice->settings.api.GetAdcSelect(h, pAdcSelect, pNumAdc));
   else
      return BERR_NOT_SUPPORTED;   
}


/******************************************************************************
 BAST_GetPeakScanSymbolRateRange()
******************************************************************************/  
BERR_Code BAST_GetPeakScanSymbolRateRange(
   BAST_ChannelHandle h,               /* [in] BAST channel handle */
   uint32_t           *pMinSymbolRate, /* [out] minimum symbol rate in sym/sec */
   uint32_t           *pMaxSymbolRate  /* [out] maximum symbol rate in sym/sec */
)
{
   if (h->pDevice->settings.api.GetPeakScanSymbolRateRange)
      return (h->pDevice->settings.api.GetPeakScanSymbolRateRange(h, pMinSymbolRate, pMaxSymbolRate));
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BAST_GetVersionInfo()
******************************************************************************/  
BERR_Code BAST_GetVersionInfo(
   BAST_Handle h,             /* [in] BAST device handle */
   BFEC_VersionInfo *pVersion /* [out] version information */
)
{
   if (h->settings.api.GetVersionInfo)
      return (h->settings.api.GetVersionInfo(h, pVersion));
   else
      return BERR_NOT_SUPPORTED;   
}
#endif /* BAST_EXCLUDE_API_TABLE */


/******************************************************************************
 BAST_MultU32U32()
******************************************************************************/
void BAST_MultU32U32(uint32_t A, uint32_t B, uint32_t *P_hi, uint32_t *P_lo)
{
   uint32_t A_lo = A & 0xFFFF;
   uint32_t A_hi = (A >> 16) & 0xFFFF;
   uint32_t B_lo = B & 0xFFFF;
   uint32_t B_hi = (B >> 16) & 0xFFFF;
   uint32_t P, P0, P1, P2, P3, c;

   P = B_lo * A_lo;
   P0 = P & 0xFFFF;
   P1 = (P >> 16) & 0xFFFF;

   P = B_hi * A_hi;
   P2 = P & 0xFFFF;
   P3 = (P >> 16) & 0xFFFF;

   P = B_lo * A_hi;
   P1 += (P & 0xFFFF);
   P2 += ((P >> 16) & 0xFFFF);

   P = B_hi * A_lo;
   P1 += (P & 0xFFFF);
   P2 += ((P >> 16) & 0xFFFF);

   c = (P1 >> 16) & 0xFFFF;
   if (c)
   {
      P1 &= 0xFFFF;
      P2 += c;
   }

   c = (P2 >> 16) & 0xFFFF;
   if (c)
   {
      P2 &= 0xFFFF;
      P3 += c;
   }

   P3 &= 0xFFFF;
   *P_hi = P2 | (P3 << 16);
   *P_lo = P0 | (P1 << 16);
}


/******************************************************************************
 BAST_DivU64U32()
******************************************************************************/
void BAST_DivU64U32(uint32_t A_hi, uint32_t A_lo, uint32_t B, uint32_t *Q_hi, uint32_t *Q_lo)
{
   uint32_t X;
   int i;

   X = *Q_hi = *Q_lo = 0;
   for (i = 63; i >= 0; i--)
   {
      X = (X << 1);
      if (i >= 32)
      {
         *Q_hi = *Q_hi << 1;
         X |= ((A_hi & (1 << (i - 32))) ? 1 : 0);
      }
      else
      {
         *Q_lo = *Q_lo << 1;
         X |= ((A_lo & (1 << i)) ? 1 : 0);
      }

      if (X >= B)
      {
         if (i >= 32)
            *Q_hi |= 1;
         else
            *Q_lo |= 1;
         X -= B;
      }
   }   
}


