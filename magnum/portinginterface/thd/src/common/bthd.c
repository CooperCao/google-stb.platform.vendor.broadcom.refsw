/***************************************************************************
 * Copyright (C) 2005-2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 ***************************************************************************/
#include "bstd.h"
#include "bthd.h"
#include "bthd_priv.h"

BDBG_MODULE(bTHD);

/******************************************************************************
 BTHD_Open()
******************************************************************************/
BERR_Code BTHD_Open(
   BTHD_Handle *h,         /* [out] BTHD handle */
   BCHP_Handle hChip,      /* [in] chip handle */
   void        *pReg,      /* [in] pointer to register or i2c handle */
   BINT_Handle hInterrupt, /* [in] Interrupt handle */   
   const BTHD_Settings *pDefSettings /* [in] default settings */
)
{ 
   BDBG_ASSERT(h);
   BDBG_ASSERT(pReg);
   BDBG_ASSERT(pDefSettings);
   
   return (pDefSettings->api.Open(h, hChip, pReg, hInterrupt, pDefSettings));
}


/******************************************************************************
 BTHD_Close()
******************************************************************************/
BERR_Code BTHD_Close(
   BTHD_Handle h   /* [in] BTHD handle */
)
{
   BDBG_ASSERT(h);
   return (h->settings.api.Close(h));
}

										    
/******************************************************************************
 BTHD_Init()
******************************************************************************/
BERR_Code BTHD_Init(
   BTHD_Handle h,         /* [in] BTHD handle */
   const uint8_t *pImage, /* [in] pointer to the microcode image. Set to NULL to use default image */
   uint32_t imageLength   /* [in] length of microcode image. Set to 0 when using default image */
)
{
   BDBG_ASSERT(h);
   return (h->settings.api.Init(h, pImage, imageLength));
}

/******************************************************************************
 BTHD_ReadRegister()
******************************************************************************/
BERR_Code BTHD_ReadRegister(
   BTHD_Handle h,    /* [in] BTHD handle */
   uint32_t    reg,  /* [in] address of register to read */
   uint32_t    *val  /* [in] contains data that was read */
)
{
   BDBG_ASSERT(h);
   return (h->settings.api.ReadRegister(h, reg, val));
}


/******************************************************************************
 BTHD_WriteRegister()
******************************************************************************/
BERR_Code BTHD_WriteRegister(
   BTHD_Handle h,    /* [in] BTHD handle */
   uint32_t    reg,  /* [in] address of register to read */
   uint32_t    *val  /* [in] contains data that was read */
)
{
   BDBG_ASSERT(h);
   return (h->settings.api.WriteRegister(h, reg, val));
}


/******************************************************************************
 BTHD_Mi2cWrite()
******************************************************************************/ 
BERR_Code BTHD_Mi2cWrite(
   BTHD_Handle h,      /* [in] BTHD handle */
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
 BTHD_Mi2cRead()
******************************************************************************/ 
BERR_Code BTHD_Mi2cRead(
   BTHD_Handle h,      /* [in] BTHD handle */
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
 BTHD_AcquireIfd()
******************************************************************************/
BERR_Code BTHD_AcquireIfd(
	BTHD_Handle h,					/* [in] BTHD handle */
	const BTHD_IfdParams *pParams   /* [in] IFD acqusition parameters */
	)
{
	BDBG_ASSERT(h);
	return (h->settings.api.AcquireIfd(h, pParams));
}

/******************************************************************************
 BTHD_GetIfdStatus()
******************************************************************************/
BERR_Code BTHD_GetIfdStatus(
	BTHD_Handle h,                  /* [in] BTHD handle */
	BTHD_IfdStatus *pIfdStatus		/* [out] IFD Status structure */
	)
{
	BDBG_ASSERT(h);
	return (h->settings.api.GetIfdStatus(h, pIfdStatus));
}

/******************************************************************************
 BTHD_SetAcquireParams()
******************************************************************************/
BERR_Code BTHD_SetAcquireParams(
   BTHD_Handle h,      /* [in] BTHD handle */
   const BTHD_InbandParams *pParams  /* [in] inband acquisition parameters */
)
{   
   BDBG_ASSERT(h);
   return (h->settings.api.SetAcquireParams(h, pParams));
}

/******************************************************************************
 BTHD_GetAcquireParams()
******************************************************************************/
BERR_Code BTHD_GetAcquireParams(
   BTHD_Handle h,      /* [in] BTHD handle */
   BTHD_InbandParams *pParams  /* [out] inband acquisition parameters */  
)
{   
   BDBG_ASSERT(h);
   return (h->settings.api.GetAcquireParams(h, pParams));
}

/******************************************************************************
 BTHD_TuneAcquire()
******************************************************************************/
BERR_Code BTHD_TuneAcquire(
   BTHD_Handle h,                    /* [in] BTHD handle */
   const BTHD_InbandParams *pParams  /* [in] inband acquisition parameters */
)
{   
   BDBG_ASSERT(h);
   return (h->settings.api.TuneAcquire(h, pParams));
}

/******************************************************************************
 BTHD_RequestThdAsyncStatus()
******************************************************************************/
BERR_Code BTHD_RequestThdAsyncStatus(
   BTHD_Handle h /* [in] BTHD handle */
)
{
   BDBG_ASSERT(h);
   return (h->settings.api.RequestThdAsyncStatus(h));
}

/******************************************************************************
 BTHD_GetThdAsyncStatus()
******************************************************************************/
BERR_Code BTHD_GetThdAsyncStatus(
   BTHD_Handle h,           /* [in] BTHD handle */
   BTHD_THDStatus *pStatus  /* [out] THD status */
)
{
   BDBG_ASSERT(h);
   return (h->settings.api.GetThdAsyncStatus(h, pStatus));
}

/******************************************************************************
 BTHD_GetThdStatus()
******************************************************************************/
BERR_Code BTHD_GetThdStatus(
   BTHD_Handle h,           /* [in] BTHD handle */
   BTHD_THDStatus *pStatus  /* [out] THD status */
)
{
   BDBG_ASSERT(h);
   return (h->settings.api.GetThdStatus(h, pStatus));
}

/******************************************************************************
 BTHD_ResetInbandStatus()
******************************************************************************/
BERR_Code BTHD_ResetInbandStatus(
   BTHD_Handle h /* [in] BTHD handle */
)
{
   BDBG_ASSERT(h);
   return (h->settings.api.ResetInbandStatus(h));
}

/******************************************************************************
 BTHD_GetInterruptEventHandle()
******************************************************************************/
BERR_Code BTHD_GetInterruptEventHandle(
   BTHD_Handle h,            /* [in] BTHD handle */
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
 BTHD_GetInterruptEventHandle()
******************************************************************************/
BERR_Code BTHD_GetBBSInterruptEventHandle(
   BTHD_Handle h,            /* [in] BTHD handle */
   BKNI_EventHandle *hEvent  /* [out] interrupt event handle */
)
{
   BDBG_ASSERT(h);
   if (h->settings.api.GetBBSInterruptEventHandle)
      return (h->settings.api.GetBBSInterruptEventHandle(h, hEvent));
   else
      return BERR_NOT_SUPPORTED;            
}

/******************************************************************************
 BTHD_HandleInterrupt_isr()
******************************************************************************/
BERR_Code BTHD_HandleInterrupt_isr(
   BTHD_Handle h /* [in] BTHD handle */
)
{
   BDBG_ASSERT(h);
   return (h->settings.api.HandleInterrupt_isr(h));
}

/******************************************************************************
 BTHD_ProcessInterruptEvent()
******************************************************************************/
BERR_Code BTHD_ProcessInterruptEvent(
   BTHD_Handle h  /* [in] BTHD handle */
)
{
   BDBG_ASSERT(h);
   return (h->settings.api.ProcessInterruptEvent(h));
}


/******************************************************************************
 BTHD_ProcessInterruptEvent()
******************************************************************************/
BERR_Code BTHD_ProcessBBSInterruptEvent(
   BTHD_Handle h  /* [in] BTHD handle */
)
{
   BDBG_ASSERT(h);
   return (h->settings.api.ProcessBBSInterruptEvent(h));
}


/******************************************************************************
 BTHD_GetSettings()
******************************************************************************/
BERR_Code BTHD_GetSettings(
   BTHD_Handle h,           /* [in] BTHD handle */
   BTHD_Settings *pSettings /* [out] current BTHD settings */
)
{
   BDBG_ASSERT(pSettings);
   BKNI_Memcpy((void*)pSettings, (void*)&(h->settings), sizeof(BTHD_Settings));
   return BERR_SUCCESS;
}


/******************************************************************************
   BTHD_SetSettings()
******************************************************************************/
BERR_Code BTHD_SetSettings(
   BTHD_Handle h,           /* [in] BTHD handle */
   BTHD_Settings *pSettings /* [in] new BTHD settings */
)
{
   BDBG_ASSERT(pSettings);
   BKNI_Memcpy((void*)&(h->settings), (void*)pSettings, sizeof(BTHD_Settings));
   return BERR_SUCCESS;
}

/******************************************************************************
 BTHD_GetChipRevision
******************************************************************************/
BERR_Code BTHD_GetChipRevision(
   BTHD_Handle h,           /* [in] BTHD handle */
   uint8_t *revision        /* [out] chip revision*/
)
{
   BDBG_ASSERT(h);
   return (h->settings.api.GetChipRevision(h, revision));
}

/******************************************************************************
 BTHD_GetVersion
******************************************************************************/
BERR_Code BTHD_GetVersion(
   BTHD_Handle h,           /* [in] BTHD handle */
   uint32_t *revision,      /* [out] firmware revision */
   uint32_t *checksum       /* [out] firmware checksum */
)
{
   BDBG_ASSERT(h);
   return (h->settings.api.GetVersion(h, revision, checksum));
}

/******************************************************************************
 BTHD_GetVersionInfo
******************************************************************************/
BERR_Code BTHD_GetVersionInfo(
    BTHD_Handle h,                 /* [in] BTHD handle  */
    BFEC_VersionInfo *pVersionInfo /* [out] Returns version Info */
)
{
   BDBG_ASSERT(h);
   return (h->settings.api.GetVersionInfo(h, pVersionInfo));
}

/******************************************************************************
 BTHD_GetLockStateChangeEventHandle()
******************************************************************************/
BERR_Code BTHD_GetLockStateChangeEventHandle(
   BTHD_Handle h,            /* [in] BTHD handle */
   BKNI_EventHandle *hEvent  /* [out] lock event handle */
)
{
   BDBG_ASSERT(h);
   return (h->settings.api.GetLockStateChangeEventHandle(h, hEvent));
}
/******************************************************************************
 BTHD_GetSoftDecisionBuf()
******************************************************************************/
BERR_Code BTHD_GetSoftDecisionBuf(
   BTHD_Handle h,  /* [in] BTHD handle */
   int16_t *pI,    /* [out] 30 I-values */
   int16_t *pQ     /* [out] 30 Q-values */
)
{
	BDBG_ASSERT(h);
	return (h->settings.api.GetSoftDecisionBuf(h, pI, pQ));
}

/******************************************************************************
 BTHD_GetDefaultInbandParam()
******************************************************************************/
BERR_Code BTHD_GetDefaultInbandParams(
    BTHD_Handle h,            /* [in] BTHD handle */         
    BTHD_InbandParams* pDefInbandParam /* [out] default param */
)
{
    return (h->settings.api.GetDefaultInbandParams(pDefInbandParam));
}

/******************************************************************************
 BTHD_GetDefaultIfdParam()
******************************************************************************/
BERR_Code BTHD_GetDefaultIfdParams(
    BTHD_Handle h,            /* [in] BTHD handle */         
    BTHD_IfdParams* pDefIfdParam /* [out] default param */
)
{
    return (h->settings.api.GetDefaultIfdParams(pDefIfdParam));
}

/******************************************************************************
 BTHD_ResetInbandStatus()
******************************************************************************/
BERR_Code BTHD_ResetIfdStatus(
   BTHD_Handle h /* [in] BTHD handle */
)
{
   BDBG_ASSERT(h);
   return (h->settings.api.ResetIfdStatus(h));
}

/******************************************************************************
 BTHD_GetIfdAudioSettings()
******************************************************************************/
BERR_Code BTHD_GetIfdAudioSettings(
    BTHD_Handle h,                          /* [in] BTHD handle */
    BTHD_IfdAudioSettings* audioSettings    /* [out] Audio settings structure */
)
{
    BDBG_ASSERT(h);
    return (h->settings.api.GetIfdAudioSettings(h, audioSettings));
}

/******************************************************************************
 BTHD_GetIfdAudioSettings()
******************************************************************************/
BERR_Code BTHD_SetIfdAudioSettings(
    BTHD_Handle h,                         /* [in] BTHD handle */
    const BTHD_IfdAudioSettings* audioSettings    /* [in] Audio settings structure */
)
{
    BDBG_ASSERT(h);
    return (h->settings.api.SetIfdAudioSettings(h, audioSettings));
}

/******************************************************************************
 BTHD_PowerEnable()
******************************************************************************/
BERR_Code BTHD_PowerUp(
    BTHD_Handle h                         /* [in] BTHD handle */
)
{
    BDBG_ASSERT(h);
    return (h->settings.api.PowerUp(h));
}

/******************************************************************************
 BTHD_PowerDisable()
******************************************************************************/
BERR_Code BTHD_PowerDown(
    BTHD_Handle h                         /* [in] BTHD handle */
)
{
    BDBG_ASSERT(h);
    return (h->settings.api.PowerDown(h));
}

/******************************************************************************
 BTHD_GetEWSEventHandle()
******************************************************************************/
BERR_Code BTHD_GetEWSEventHandle(
   BTHD_Handle h,            /* [in] BTHD handle */
   BKNI_EventHandle *hEvent  /* [out] lock event handle */
)
{
   BDBG_ASSERT(h);
   return (h->settings.api.GetEWSEventHandle(h, hEvent));
}

/******************************************************************************
 BTHD_GetThdLockStatus()
******************************************************************************/
BERR_Code BTHD_GetThdLockStatus(
   BTHD_Handle h,            /* [in] BTHD handle */
   BTHD_LockStatus *pLockStatus  /* [out] THD status */
)
{
   BDBG_ASSERT(h);
   return (h->settings.api.GetThdLockStatus(h, pLockStatus));
}

/******************************************************************************
 BTHD_InstallCallback()
******************************************************************************/
BERR_Code BTHD_InstallCallback(
    BTHD_Handle h,                      /* [in] Device channel handle */
    BTHD_Callback callbackType,         /* [in] Type of callback */
    BTHD_CallbackFunc pCallback_isr,    /* [in] Function Ptr to callback */
    void *pParam                        /* [in] Generic parameter send on callback */
    )
{
    BERR_Code retCode = BERR_SUCCESS;


    BDBG_ENTER(BTHD_InstallCallback);
    BDBG_ASSERT( h );
    
    if( h->settings.api.InstallCallback != NULL )
    {
        retCode = h->settings.api.InstallCallback( h, callbackType, pCallback_isr, pParam );
    }
    else
    {
        BDBG_WRN(("BTHD_InstallCallback: Funtion Ptr is NULL"));
    }

    BDBG_LEAVE(BTHD_InstallCallback);
    return( retCode );
}
