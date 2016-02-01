/***************************************************************************
 *     Copyright (c) 2004-2009, Broadcom Corporation
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
#include "bvsb.h"
#include "bvsb_priv.h"

BDBG_MODULE(bvsb);


/******************************************************************************
 BVSB_Open()
******************************************************************************/
BERR_Code BVSB_Open(
   BVSB_Handle *h,         /* [out] BVSB handle */
   BCHP_Handle hChip,      /* [in] chip handle */
   void        *pReg,      /* [in] pointer to register or i2c handle */
   BINT_Handle hInterrupt, /* [in] Interrupt handle */   
   const BVSB_Settings *pDefSettings /* [in] default settings */
)
{ 
   BDBG_ASSERT(h);
   BDBG_ASSERT(pReg);
   BDBG_ASSERT(pDefSettings);
   
   return (pDefSettings->api.Open(h, hChip, pReg, hInterrupt, pDefSettings));
}


/******************************************************************************
 BVSB_Close()
******************************************************************************/
BERR_Code BVSB_Close(
   BVSB_Handle h   /* [in] BVSB handle */
)
{
   BDBG_ASSERT(h);
   return (h->settings.api.Close(h));
}

										    
/******************************************************************************
 BVSB_InitAp()
******************************************************************************/
BERR_Code BVSB_InitAp(
   BVSB_Handle h,         /* [in] BVSB handle */
   const uint8_t *pImage  /* [in] pointer to the AP image */
)
{
   BDBG_ASSERT(h);
   return (h->settings.api.InitAp(h, pImage));
}


/******************************************************************************
 BVSB_GetApStatus()
******************************************************************************/
BERR_Code BVSB_GetApStatus(
   BVSB_Handle   h,        /* [in] VSB device handle */
   BVSB_ApStatus *pStatus  /* [out] AP status */
)
{
   BDBG_ASSERT(h);
   return (h->settings.api.GetApStatus(h, pStatus));
}


/******************************************************************************
 BVSB_GetApVersion()
******************************************************************************/
BERR_Code BVSB_GetApVersion(
   BVSB_Handle h,          /* [in] BVSB handle */
   uint16_t    *pChipId,   /* [out] VSB chip id */
   uint16_t    *pChipVer,  /* [out] chip revision number */
   uint8_t     *pApVer,    /* [out] AP microcode version */
   uint8_t     *pScrVer,   /* [out] acquisition script version */
   uint8_t     *pCfgVer    /* [out] host configuration version */
)
{
   BDBG_ASSERT(h);
   return (h->settings.api.GetApVersion(h, pChipId, pChipVer, pApVer, pScrVer, pCfgVer));
}


/******************************************************************************
 BVSB_ReadRegister()
******************************************************************************/
BERR_Code BVSB_ReadRegister(
   BVSB_Handle h,    /* [in] BVSB handle */
   uint32_t    reg,  /* [in] address of register to read */
   uint32_t    *val  /* [in] contains data that was read */
)
{
   BDBG_ASSERT(h);
   return (h->settings.api.ReadRegister(h, reg, val));
}


/******************************************************************************
 BVSB_WriteRegister()
******************************************************************************/
BERR_Code BVSB_WriteRegister(
   BVSB_Handle h,    /* [in] BVSB handle */
   uint32_t    reg,  /* [in] address of register to read */
   uint32_t    *val  /* [in] contains data that was read */
)
{
   BDBG_ASSERT(h);
   return (h->settings.api.WriteRegister(h, reg, val));
}


/******************************************************************************
 BVSB_Mi2cWrite()
******************************************************************************/ 
BERR_Code BVSB_Mi2cWrite(
   BVSB_Handle h,      /* [in] BVSB handle */
   uint8_t slave_addr, /* [in] address of the i2c slave device */
   uint8_t *buf,       /* [in] specifies the data to transmit */
   uint8_t n           /* [in] number of bytes to transmit after the i2c slave address */
)
{
   BDBG_ASSERT(h);
   if (h->settings.api.Mi2cWrite)
      return (h->settings.api.Mi2cWrite(h, slave_addr, buf, n));
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BVSB_Mi2cRead()
******************************************************************************/ 
BERR_Code BVSB_Mi2cRead(
   BVSB_Handle h,      /* [in] BVSB handle */
   uint8_t slave_addr, /* [in] address of the i2c slave device */
   uint8_t *out_buf,   /* [in] specifies the data to transmit before the i2c restart condition */
   uint8_t out_n,      /* [in] number of bytes to transmit before the i2c restart condition not including the i2c slave address */
   uint8_t *in_buf,    /* [out] holds the data read */
   uint8_t in_n        /* [in] number of bytes to read after the i2c restart condition not including the i2c slave address */
)
{
   BDBG_ASSERT(h);
   if (h->settings.api.Mi2cRead)
      return (h->settings.api.Mi2cRead(h, slave_addr, out_buf, out_n, in_buf, in_n));
   else
      return BERR_NOT_SUPPORTED;  
}


/******************************************************************************
 BVSB_AcquireInband()
******************************************************************************/
BERR_Code BVSB_AcquireInband(
   BVSB_Handle h,                    /* [in] BVSB handle */
   const BVSB_InbandParams *pParams  /* [in] inband acquisition parameters */
)
{   
   BDBG_ASSERT(h);
   return (h->settings.api.AcquireInband(h, pParams));
}


/******************************************************************************
 BVSB_GetVsbStatus()
******************************************************************************/
BERR_Code BVSB_GetVsbStatus(
   BVSB_Handle h,           /* [in] BVSB handle */
   BVSB_VsbStatus *pStatus  /* [out] VSB status */
)
{
   BDBG_ASSERT(h);
   return (h->settings.api.GetVsbStatus(h, pStatus));
}


/******************************************************************************
 BVSB_GetQamStatus()
******************************************************************************/
BERR_Code BVSB_GetQamStatus(
   BVSB_Handle h,           /* [in] BVSB handle */
   BVSB_QamStatus *pStatus  /* [out] QAM status   */
)
{
   BDBG_ASSERT(h);
   return (h->settings.api.GetQamStatus(h, pStatus));
}


/******************************************************************************
 BVSB_GetNtscStatus()
******************************************************************************/
BERR_Code BVSB_GetNtscStatus(
   BVSB_Handle h,           /* [in] BVSB handle */
   BVSB_NtscStatus *pStatus /* [out] NTSC status */
)
{
   BDBG_ASSERT(h);
   return (h->settings.api.GetNtscStatus(h, pStatus));
}


/******************************************************************************
 BVSB_ResetInbandStatus()
******************************************************************************/
BERR_Code BVSB_ResetInbandStatus(
   BVSB_Handle h /* [in] BVSB handle */
)
{
   BDBG_ASSERT(h);
   return (h->settings.api.ResetInbandStatus(h));
}


/******************************************************************************
 BVSB_AcquireOob()
******************************************************************************/
BERR_Code BVSB_AcquireOob(
   BVSB_Handle h,           /* [in] BVSB handle */
   BVSB_OobParams *pParams  /* [in] acquisition parameters */
)
{
   BDBG_ASSERT(h);
   return (h->settings.api.AcquireOob(h, pParams));
}


/******************************************************************************
 BVSB_GetOobStatus()
******************************************************************************/
BERR_Code BVSB_GetOobStatus(
   BVSB_Handle h,           /* [in] BVSB handle */
   BVSB_OobStatus *pStatus  /* [out] OOB status   */
)
{
   BDBG_ASSERT(h);
   return (h->settings.api.GetOobStatus(h, pStatus));
}


/******************************************************************************
 BVSB_ResetOobStatus()
******************************************************************************/
BERR_Code BVSB_ResetOobStatus(
   BVSB_Handle h /* [in] BVSB handle */
)
{
   BDBG_ASSERT(h);
   return (h->settings.api.ResetOobStatus(h));
}


/******************************************************************************
 BVSB_SetBtscConfig()
******************************************************************************/
BERR_Code BVSB_SetBtscConfig(
   BVSB_Handle h,             /* [in] BVSB handle */
   BVSB_BtscSettings *pParams /* [in] BTSC configuration parameters */
)
{
   BDBG_ASSERT(h);
   if (h->settings.api.SetBtscConfig)
      return (h->settings.api.SetBtscConfig(h, pParams));
   else
      return BERR_NOT_SUPPORTED;   
}


/******************************************************************************
 BVSB_SetBtscVolume()
******************************************************************************/
BERR_Code BVSB_SetBtscVolume(
   BVSB_Handle h,        /* [in] BVSB handle */
   uint8_t left_volume,  /* [in] left volume attenuation in dB (0 to 95 dB) */
   uint8_t right_volume, /* [in] left volume attenuation in dB (0 to 95 dB) */
   bool    bMute
)
{
   BDBG_ASSERT(h);
   if (h->settings.api.SetBtscVolume)
      return (h->settings.api.SetBtscVolume(h, left_volume, right_volume, bMute));
   else
      return BERR_NOT_SUPPORTED;     
}


/******************************************************************************
 BVSB_GetBtscStatus()
******************************************************************************/
BERR_Code BVSB_GetBtscStatus(
   BVSB_Handle h,           /* [in] BVSB handle */
   BVSB_BtscStatus *pStatus /* [out] BTSC status */
)
{
   BDBG_ASSERT(h);
   if (h->settings.api.GetBtscStatus)
      return (h->settings.api.GetBtscStatus(h, pStatus));
   else
      return BERR_NOT_SUPPORTED;        
}


/******************************************************************************
 BVSB_SetInbandO()
******************************************************************************/
BERR_Code BVSB_SetInbandOi(
   BVSB_Handle           h,         /* [in] BVSB handle */
   BVSB_InbandOiSettings *pInbandOi /* [in] inband transport output interface settings */
)
{
   BDBG_ASSERT(h);
   return (h->settings.api.SetInbandOi(h, pInbandOi));
}


/******************************************************************************
 BVSB_GetSoftDecisionBuf()
******************************************************************************/
BERR_Code BVSB_GetSoftDecisionBuf(
   BVSB_Handle h,  /* [in] BVSB handle */
   int16_t *pI,    /* [out] 30 I-values */
   int16_t *pQ     /* [out] 30 Q-values */
)
{
   BDBG_ASSERT(h);
   return (h->settings.api.GetSoftDecisionBuf(h, pI, pQ));
}


/******************************************************************************
 BVSB_GetOobSoftDecisionBuf()
******************************************************************************/
BERR_Code BVSB_GetOobSoftDecisionBuf(
   BVSB_Handle h,  /* [in] BVSB handle */
   int16_t *pI,    /* [out] 30 I-values */
   int16_t *pQ     /* [out] 30 Q-values */
)
{
   BDBG_ASSERT(h);
   return (h->settings.api.GetOobSoftDecisionBuf(h, pI, pQ));
}


/******************************************************************************
 BVSB_SetTmConfig()
******************************************************************************/
BERR_Code BVSB_SetTmConfig(
   BVSB_Handle h,     /* [in] BVSB handle */
   void        *pCfg  /* [in] pad configuration parameters */
)
{
   BDBG_ASSERT(h);
   if (h->settings.api.SetTmConfig)
      return (h->settings.api.SetTmConfig(h, pCfg));
   else
      return BERR_NOT_SUPPORTED;   
}


/******************************************************************************
 BVSB_GetTmConfig()
******************************************************************************/
BERR_Code BVSB_GetTmConfig(
   BVSB_Handle h,     /* [in] BVSB handle */
   void        *pCfg  /* [out] pad configuration parameters */
)
{
   BDBG_ASSERT(h);
   if (h->settings.api.GetTmConfig)
      return (h->settings.api.GetTmConfig(h, pCfg));
   else
      return BERR_NOT_SUPPORTED;      
}


/******************************************************************************
 BVSB_WriteConfig()
******************************************************************************/
BERR_Code BVSB_WriteConfig(
   BVSB_Handle h,   /* [in] BVSB handle */
   uint16_t offset, /* [in] 16-bit offset within the host configuration space */
   uint8_t *buf,    /* [in] data to write */
   uint8_t len      /* [in] number of bytes to write */
)
{
   BDBG_ASSERT(h);
   return (h->settings.api.WriteConfig(h, offset, buf, len));
}


/******************************************************************************
 BVSB_ReadConfig()
******************************************************************************/
BERR_Code BVSB_ReadConfig(
   BVSB_Handle h,  /* [in] BVSB handle */
   uint16_t offset,   /* [in] 16-bit offset within the host configuration space */
   uint8_t *buf,      /* [in] buffer to hold the read data */
   uint8_t len        /* [in] number of bytes to read */
)
{
   BDBG_ASSERT(h);
   return (h->settings.api.ReadConfig(h, offset, buf, len));
}

/******************************************************************************
 BVSB_GetunLockStateEventHandle()
******************************************************************************/
BERR_Code BVSB_GetUnlockEventHandle(
   BVSB_Handle h,            /* [in] BVSB handle */
   BKNI_EventHandle *hEvent  /* [out] lock event handle */
)
{
   BDBG_ASSERT(h);
   return (h->settings.api.GetUnlockstatusEventHandle(h, hEvent));
}

/******************************************************************************
 BVSB_GetLockStateEventHandle()
******************************************************************************/
BERR_Code BVSB_GetLockEventHandle(
   BVSB_Handle h,            /* [in] BVSB handle */
   BKNI_EventHandle *hEvent  /* [out] lock event handle */
)
{
   BDBG_ASSERT(h);
   return (h->settings.api.GetLockstatusEventHandle(h, hEvent));
}

/******************************************************************************
 BVSB_GetLockStateChangeEventHandle()
******************************************************************************/
BERR_Code BVSB_GetLockStateChangeEventHandle(
   BVSB_Handle h,            /* [in] BVSB handle */
   BKNI_EventHandle *hEvent  /* [out] lock event handle */
)
{
   BDBG_ASSERT(h);
   return (h->settings.api.GetLockStateChangeEventHandle(h, hEvent));
}
/******************************************************************************
 BVSB_GetOobLockStateChangeEventHandle()
******************************************************************************/
BERR_Code BVSB_GetOobLockStateChangeEventHandle(
   BVSB_Handle h,            /* [in] BVSB handle */
   BKNI_EventHandle *hEvent  /* [out] lock event handle */
)
{
   BDBG_ASSERT(h);
   return (h->settings.api.GetOobLockStateChangeEventHandle(h, hEvent));
}

/******************************************************************************
 BVSB_GetAntennaEventHandle()
******************************************************************************/
BERR_Code BVSB_GetAntennaEventHandle(
   BVSB_Handle h,            /* [in] BVSB handle */
   BKNI_EventHandle *hEvent  /* [out] antenna event handle */
)
{
   BDBG_ASSERT(h);
   if (h->settings.api.GetAntennaEventHandle)
      return (h->settings.api.GetAntennaEventHandle(h, hEvent));
   else
      return BERR_NOT_SUPPORTED;         
}


/******************************************************************************
 BVSB_GetInterruptEventHandle()
******************************************************************************/
BERR_Code BVSB_GetInterruptEventHandle(
   BVSB_Handle h,            /* [in] BVSB handle */
   BKNI_EventHandle *hEvent  /* [out] interrupt event handle */
)
{
   BDBG_ASSERT(h);
   if (h->settings.api.GetInterruptEventHandle)
      return (h->settings.api.GetInterruptEventHandle(h, hEvent));
   else
      return BERR_NOT_SUPPORTED;            
}


/******************************************************************************
 BVSB_HandleInterrupt_isr()
******************************************************************************/
BERR_Code BVSB_HandleInterrupt_isr(
   BVSB_Handle h /* [in] BVSB handle */
)
{
   BDBG_ASSERT(h);
   return (h->settings.api.HandleInterrupt_isr(h));
}


/******************************************************************************
 BVSB_ProcessInterruptEvent()
******************************************************************************/
BERR_Code BVSB_ProcessInterruptEvent(
   BVSB_Handle h  /* [in] BVSB handle */
)
{
   BDBG_ASSERT(h);
   return (h->settings.api.ProcessInterruptEvent(h));
}


/******************************************************************************
 BVSB_GetSettings()
******************************************************************************/
BERR_Code BVSB_GetSettings(
   BVSB_Handle h,           /* [in] BVSB handle */
   BVSB_Settings *pSettings /* [out] current BVSB settings */
)
{
   BDBG_ASSERT(pSettings);
   BKNI_Memcpy((void*)pSettings, (void*)&(h->settings), sizeof(BVSB_Settings));
   return BERR_SUCCESS;
}


/******************************************************************************
   BVSB_SetSettings()
******************************************************************************/
BERR_Code BVSB_SetSettings(
   BVSB_Handle h,           /* [in] BVSB handle */
   BVSB_Settings *pSettings /* [in] new BVSB settings */
)
{
   BDBG_ASSERT(pSettings);
   BKNI_Memcpy((void*)&(h->settings), (void*)pSettings, sizeof(BVSB_Settings));
   return BERR_SUCCESS;
}

/******************************************************************************
 BVSB_GetChipRevision
******************************************************************************/
BERR_Code BVSB_GetChipRevision(
   BVSB_Handle h,           /* [in] BVSB handle */
   uint8_t *revision        /* [out] chip revision*/
)
{
   BDBG_ASSERT(h);
   return (h->settings.api.GetChipRevision(h, revision));
}

/******************************************************************************
   BVSB_DetectChannelSignal()
******************************************************************************/
BERR_Code BVSB_DetectChannelSignal(
								   BVSB_Handle h,  /* [in] VSB device handle */
								   BVSB_DetectType eDetectType,   /* [in] signal detect type */
								   BVSB_SignalMode* peSignalMode /* [out] signal mode */
)
{
   BDBG_ASSERT(h);
   return (h->settings.api.DetectChannelSignal(h, eDetectType, peSignalMode));
}


/******************************************************************************
   BVSB_SetInbandIfFreq()
******************************************************************************/
BERR_Code BVSB_SetInbandIfFreq(
							   BVSB_Handle h,             /* [in] BVSB handle                */
							   uint32_t uIfFreq_hz /* [in] BTSC configuration parameters */
							   )
{
	BDBG_ASSERT(h);
	return (h->settings.api.SetInbandIfFreq(h, uIfFreq_hz));
}

/******************************************************************************
   BVSB_SetIfdPullInRange()
******************************************************************************/
BERR_Code BVSB_SetIfdPullInRange(
	BVSB_Handle h,      
	BVSB_Ifd_PullInRange if_pulling_range
)
{
	BDBG_ASSERT(h);
	return (h->settings.api.SetIfdPullInRange(h, if_pulling_range));
}
BERR_Code BVSB_PowerDown(
    BVSB_Handle h
)
{
	BDBG_ASSERT(h);
	return (h->settings.api.PowerDown(h));
}

/******************************************************************************
   BVSB_SetPgaGain()
******************************************************************************/
BERR_Code BVSB_SetPgaGain(
	BVSB_Handle h,      
    BVSB_InbandMode mode,
	BVSB_PgaGain PgaGain
)
{
	BDBG_ASSERT(h);
	return (h->settings.api.SetPgaGain(h, mode, PgaGain));
}

/******************************************************************************
   BVSB_SetOobInterfaceControl()
******************************************************************************/
BERR_Code BVSB_SetOobInterfaceControl(
   BVSB_Handle  h,    /* [in] BVSB handle */
   BVSB_OobClockSuppression clockSuppression,
   BVSB_OobClock clock
)
{
	BDBG_ASSERT(h);
	return (h->settings.api.SetOobInterfaceControl(h, clockSuppression, clock));
}

