/******************************************************************************
*    (c)2011-2013 Broadcom Corporation
*
* This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
*
* $brcm_Workfile: $
* $brcm_Revision: $
* $brcm_Date: $
*
* Module Description:
*
* Revision History:
*
* $brcm_Log: $
* 
*****************************************************************************/
#include "bdsq.h"
#include "bdsq_priv.h"


BDBG_MODULE(bdsq);


#ifndef BDSQ_EXCLUDE_API_TABLE
/******************************************************************************
 BDSQ_Open()
******************************************************************************/
BERR_Code BDSQ_Open(
   BDSQ_Handle *h,                /* [out] BDSQ handle */
   BCHP_Handle hChip,             /* [in] chip handle */
   void *pReg,                    /* [in] register or hab handle */
   BINT_Handle hInterrupt,        /* [in] BINT handle */
   const BDSQ_Settings *pSettings /* [in] default settings */
)
{
   BDBG_ASSERT(h);
   BDBG_ASSERT(pSettings);
   /*BDBG_ASSERT(pReg); */
   return (pSettings->api.Open(h, hChip, pReg, hInterrupt, pSettings));
}


/******************************************************************************
 BDSQ_Close()
******************************************************************************/
BERR_Code BDSQ_Close(
   BDSQ_Handle h   /* [in] BDSQ handle */
)
{
   BDBG_ASSERT(h);
   return (h->settings.api.Close(h));
}


/******************************************************************************
 BDSQ_GetTotalChannels()
******************************************************************************/
BERR_Code BDSQ_GetTotalChannels(
   BDSQ_Handle h,             /* [in] BDSQ handle */
   uint32_t    *totalChannels /* [out] number of satellite downstream channels supported by this chip */
)
{
   BDBG_ASSERT(h);
   BDBG_ASSERT(totalChannels);
   return (h->settings.api.GetTotalChannels(h, totalChannels));
}


/******************************************************************************
 BDSQ_GetChannelDefaultSettings()
******************************************************************************/
BERR_Code BDSQ_GetChannelDefaultSettings(
   BDSQ_Handle h,                   /* [in] BDSQ handle */
   uint32_t    chan,                /* [in] channel index (0 based) */
   BDSQ_ChannelSettings *pSettings  /* [out] default channel settings */
)
{
   BDBG_ASSERT(h);
   BDBG_ASSERT(pSettings);
   return (h->settings.api.GetChannelDefaultSettings(h, chan, pSettings));
}


/******************************************************************************
 BDSQ_OpenChannel()
******************************************************************************/
BERR_Code BDSQ_OpenChannel(
   BDSQ_Handle                h,                /* [in] BDSQ handle */
   BDSQ_ChannelHandle         *pChannelHandle,  /* [out] BDSQ channel handle */
   uint32_t                   chan,             /* [in] channel index (0 based) */
   const BDSQ_ChannelSettings *pSettings        /* [in] channel settings */
)
{
   BDBG_ASSERT(h);
   BDBG_ASSERT(pChannelHandle);
   BDBG_ASSERT(pSettings);
   return (h->settings.api.OpenChannel(h, pChannelHandle, chan, pSettings));
}


/******************************************************************************
 BDSQ_CloseChannel()
******************************************************************************/
BERR_Code BDSQ_CloseChannel(
   BDSQ_ChannelHandle h  /* [in] BDSQ channel handle */
)
{
   BDBG_ASSERT(h);
   return (h->pDevice->settings.api.CloseChannel(h));
}


/******************************************************************************
 BDSQ_GetDevice()
******************************************************************************/
BERR_Code BDSQ_GetDevice(
   BDSQ_ChannelHandle h,      /* [in] BDSQ channel handle */
   BDSQ_Handle        *pDev   /* [out] BDSQhandle */
)
{
   BDBG_ASSERT(h);
   BDBG_ASSERT(pDev);
   return (h->pDevice->settings.api.GetDevice(h, pDev));
}


/******************************************************************************
 BDSQ_GetVersionInfo()
******************************************************************************/
BERR_Code BDSQ_GetVersionInfo(
   BDSQ_Handle h,             /* [in] BDSQ handle */
   BFEC_VersionInfo *pVersion /* [out] version information */
)
{
   BDBG_ASSERT(h);
   BDBG_ASSERT(pVersion);
   return (h->settings.api.GetVersionInfo(h, pVersion));
}


/******************************************************************************
 BDSQ_Reset()
******************************************************************************/
BERR_Code BDSQ_Reset(
   BDSQ_Handle h    /* [in] BDSQ handle */
)
{
   BDBG_ASSERT(h);
   if (h->settings.api.Reset)
      return h->settings.api.Reset(h);
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BDSQ_ResetChannel()
******************************************************************************/
BERR_Code BDSQ_ResetChannel(
   BDSQ_ChannelHandle h    /* [in] BDSQ handle */
)
{
   BDBG_ASSERT(h);
   if (h->pDevice->settings.api.ResetChannel)
      return h->pDevice->settings.api.ResetChannel(h);
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BDSQ_PowerDownChannel()
******************************************************************************/
BERR_Code BDSQ_PowerDownChannel(
   BDSQ_ChannelHandle h    /* [in] BDSQ channel handle */
)
{
   BDBG_ASSERT(h);
   if (h->pDevice->settings.api.PowerDownChannel)
      return h->pDevice->settings.api.PowerDownChannel(h);
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BDSQ_PowerUpChannel()
******************************************************************************/
BERR_Code BDSQ_PowerUpChannel(
   BDSQ_ChannelHandle h  /* [in] BDSQ channel handle */
)
{
   BDBG_ASSERT(h);
   if (h->pDevice->settings.api.PowerUpChannel)
      return h->pDevice->settings.api.PowerUpChannel(h);
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BDSQ_IsChannelOn()
******************************************************************************/
BERR_Code BDSQ_IsChannelOn(
   BDSQ_ChannelHandle h, /* [in] BDSQ channel handle */
   bool *pOn             /* [out] true=channel is powered */
)
{
   BDBG_ASSERT(h);
   if (h->pDevice->settings.api.IsChannelOn)
      return h->pDevice->settings.api.IsChannelOn(h, pOn);
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BDSQ_PowerDownVsense()
******************************************************************************/
BERR_Code BDSQ_PowerDownVsense(
   BDSQ_ChannelHandle h    /* [in] BDSQ channel handle */
)
{
   BDBG_ASSERT(h);
   if (h->pDevice->settings.api.PowerDownVsense)
      return h->pDevice->settings.api.PowerDownVsense(h);
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BDSQ_PowerUpVsense()
******************************************************************************/
BERR_Code BDSQ_PowerUpVsense(
   BDSQ_ChannelHandle h  /* [in] BDSQ channel handle */
)
{
   BDBG_ASSERT(h);
   if (h->pDevice->settings.api.PowerUpVsense)
      return h->pDevice->settings.api.PowerUpVsense(h);
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BDSQ_IsVsenseOn()
******************************************************************************/
BERR_Code BDSQ_IsVsenseOn(
   BDSQ_ChannelHandle h, /* [in] BDSQ channel handle */
   bool *pOn             /* [out] true=channel is powered */
)
{
   BDBG_ASSERT(h);
   if (h->pDevice->settings.api.IsVsenseOn)
      return h->pDevice->settings.api.IsVsenseOn(h, pOn);
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BDSQ_SetTone()
******************************************************************************/
BERR_Code BDSQ_SetTone(
   BDSQ_ChannelHandle h,  /* [in] BDSQ channel handle */
   bool bEnable           /* [in] true = tone on, false = tone off */
)
{
   BDBG_ASSERT(h);
   if (h->pDevice->settings.api.SetTone)
      return h->pDevice->settings.api.SetTone(h, bEnable);
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BDSQ_GetTone()
******************************************************************************/
BERR_Code BDSQ_GetTone(
   BDSQ_ChannelHandle h,  /* [in] BDSQ channel handle */
   bool *pbTone           /* [out] true = tone is present, false = tone absent */
)
{
   BDBG_ASSERT(h);
   BDBG_ASSERT(pbTone);
   if (h->pDevice->settings.api.GetTone)
      return h->pDevice->settings.api.GetTone(h, pbTone);
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BDSQ_SetVoltage()
******************************************************************************/
BERR_Code BDSQ_SetVoltage(
   BDSQ_ChannelHandle h, /* [in] BDSQ channel handle */
   bool bVtop            /* [in] true = VTOP, false = VBOT */
)
{
   BDBG_ASSERT(h);
   if (h->pDevice->settings.api.SetVoltage)
      return h->pDevice->settings.api.SetVoltage(h, bVtop);
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BDSQ_GetVoltage()
******************************************************************************/
BERR_Code BDSQ_GetVoltage(
   BDSQ_ChannelHandle h,   /* [in] BDSQ channel handle */
   uint16_t *pVoltage      /* [out] voltage estimation */
)
{
   BDBG_ASSERT(h);
   BDBG_ASSERT(pVoltage);
   if (h->pDevice->settings.api.GetVoltage)
      return h->pDevice->settings.api.GetVoltage(h, pVoltage);
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BDSQ_EnableVsenseInterrupt()
******************************************************************************/
BERR_Code BDSQ_EnableVsenseInterrupt(
   BDSQ_ChannelHandle h,   /* [in] BDSQ channel handle */
   bool bEnable            /* [in] true = enable, false = disable */
)
{
   BDBG_ASSERT(h);
   if (h->pDevice->settings.api.EnableVsenseInterrupt)
      return h->pDevice->settings.api.EnableVsenseInterrupt(h, bEnable);
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BDSQ_EnableActivityInterrupt()
******************************************************************************/
BERR_Code BDSQ_EnableActivityInterrupt(
   BDSQ_ChannelHandle h,   /* [in] BDSQ channel handle */
   bool bEnable            /* [in] true = enable, false = disable */
)
{
   BDBG_ASSERT(h);
   if (h->pDevice->settings.api.EnableActivityInterrupt)
      return h->pDevice->settings.api.EnableActivityInterrupt(h, bEnable);
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BDSQ_EnableIdleInterrupt()
******************************************************************************/
BERR_Code BDSQ_EnableIdleInterrupt(
   BDSQ_ChannelHandle h,   /* [in] BDSQ channel handle */
   bool bEnable            /* [in] true = enable, false = disable */
)
{
   BDBG_ASSERT(h);
   if (h->pDevice->settings.api.EnableIdleInterrupt)
      return h->pDevice->settings.api.EnableIdleInterrupt(h, bEnable);
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BDSQ_Write()
******************************************************************************/
BERR_Code BDSQ_Write(
   BDSQ_ChannelHandle h,   /* [in] BDSQ channel handle */
   const uint8_t *pBuf,    /* [in] contains data to transmit */
   uint8_t n               /* [in] number of bytes to transmit */
)
{
   BDBG_ASSERT(h);
   if (h->pDevice->settings.api.Write)
      return h->pDevice->settings.api.Write(h, pBuf, n);
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BDSQ_EnableRx()
******************************************************************************/
BERR_Code BDSQ_EnableRx(
   BDSQ_ChannelHandle h,   /* [in] BDSQ channel handle */
   bool bEnable            /* [in] enable/disable Rx */
)
{
   BDBG_ASSERT(h);
   if (h->pDevice->settings.api.EnableRx)
      return h->pDevice->settings.api.EnableRx(h, bEnable);
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BDSQ_EnableLnb()
******************************************************************************/
BERR_Code BDSQ_EnableLnb(
   BDSQ_ChannelHandle h,   /* [in] BDSQ channel handle */
   bool bEnable            /* [in] enable/disable LNB */
)
{
   BDBG_ASSERT(h);
   if (h->pDevice->settings.api.EnableLnb)
      return h->pDevice->settings.api.EnableLnb(h, bEnable);
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BDSQ_Read()
******************************************************************************/
BERR_Code BDSQ_Read(
   BDSQ_ChannelHandle h,  /* [in] BDSQ handle */
   uint8_t     *pBuf,     /* [out] message data */
   uint8_t     nBufMax,   /* [in] maximum number of bytes that can be written into pBuf */
   uint8_t     *n,        /* [out] number of bytes received */
   uint8_t     *nNotRead  /* [out] number of bytes remaining in the rx fifo */
)
{
   BDBG_ASSERT(h);
   BDBG_ASSERT(pBuf);
   BDBG_ASSERT(n);
   BDBG_ASSERT(nNotRead);
   if (h->pDevice->settings.api.Read)
      return h->pDevice->settings.api.Read(h, pBuf, nBufMax, n, nNotRead);
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BDSQ_SendACW()
******************************************************************************/
BERR_Code BDSQ_SendAcw(
   BDSQ_ChannelHandle h, /* [in] BDSQ channel handle */
   uint8_t acw           /* [in] auto control word to send */
)
{
   BDBG_ASSERT(h);
   if (h->pDevice->settings.api.SendAcw)
      return h->pDevice->settings.api.SendAcw(h, acw);
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BDSQ_GetStatus()
******************************************************************************/
BERR_Code BDSQ_GetStatus(
   BDSQ_ChannelHandle h,   /* [in] BDSQ channel handle */
   BDSQ_Status *pStatus    /* [out] status of most recent transaction */
)
{
   BDBG_ASSERT(h);
   BDBG_ASSERT(pStatus);
   if (h->pDevice->settings.api.GetStatus)
      return h->pDevice->settings.api.GetStatus(h, pStatus);
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BDSQ_GetActivityStatus()
******************************************************************************/
BERR_Code BDSQ_GetActivityStatus(
   BDSQ_ChannelHandle  h,                 /* [in] BDSQ channel handle */
   BDSQ_ActivityStatus *pActivityStatus   /* [out] status of bus activity */
)
{
   BDBG_ASSERT(h);
   BDBG_ASSERT(pActivityStatus);
   if (h->pDevice->settings.api.GetActivityStatus)
      return h->pDevice->settings.api.GetActivityStatus(h, pActivityStatus);
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BDSQ_ResetActivityStatus()
******************************************************************************/
BERR_Code BDSQ_ResetActivityStatus(
   BDSQ_ChannelHandle h    /* [in] BDSQ channel handle */
)
{
   BDBG_ASSERT(h);
   if (h->pDevice->settings.api.ResetActivityStatus)
      return h->pDevice->settings.api.ResetActivityStatus(h);
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BDSQ_SetChannelSettings()
******************************************************************************/   
BERR_Code BDSQ_SetChannelSettings(
   BDSQ_ChannelHandle h,      /* [in] BDSQ channel handle */
   BDSQ_ChannelSettings *pSettings  /* [in] DiSEqC channel settings */
)
{
   BDBG_ASSERT(h);
   BDBG_ASSERT(pSettings);
   if (h->pDevice->settings.api.SetChannelSettings)
      return h->pDevice->settings.api.SetChannelSettings(h, pSettings);
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BDSQ_GetChannelSettings()
******************************************************************************/
BERR_Code BDSQ_GetChannelSettings(
   BDSQ_ChannelHandle h,      /* [in] BDSQ channel handle */
   BDSQ_ChannelSettings *pSettings  /* [in] DiSEqC channel settings */
)
{
   BDBG_ASSERT(h);
   BDBG_ASSERT(pSettings);
   if (h->pDevice->settings.api.GetChannelSettings)
      return h->pDevice->settings.api.GetChannelSettings(h, pSettings);
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BDSQ_SetConfig()
******************************************************************************/
BERR_Code BDSQ_SetConfig(
   BDSQ_Handle h,    /* [in] BDSQ device handle */
   uint32_t    addr, /* [in] configuration parameter address */
   uint32_t    val   /* [in] configuration parameter value */
)
{
   BDBG_ASSERT(h);
   if (h->settings.api.SetConfig)
      return h->settings.api.SetConfig(h, addr, val);
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BDSQ_GetConfig()
******************************************************************************/
BERR_Code BDSQ_GetConfig(
   BDSQ_Handle h,    /* [in] BDSQ device handle */
   uint32_t    addr, /* [in] configuration parameter address */
   uint32_t    *pVal /* [out] value of the configuration parameter */
)
{
   BDBG_ASSERT(h);
   BDBG_ASSERT(pVal);
   if (h->settings.api.GetConfig)
      return h->settings.api.GetConfig(h, addr, pVal);
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BDSQ_SetChannelConfig()
******************************************************************************/
BERR_Code BDSQ_SetChannelConfig(
   BDSQ_ChannelHandle h,    /* [in] BDSQ channel handle */
   uint32_t           addr, /* [in] configuration parameter address */
   uint32_t           val   /* [in] configuration parameter value */
)
{
   BDBG_ASSERT(h);
   if (h->pDevice->settings.api.SetChannelConfig)
      return h->pDevice->settings.api.SetChannelConfig(h, addr, val);
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BDSQ_GetChannelConfig()
******************************************************************************/
BERR_Code BDSQ_GetChannelConfig(
   BDSQ_ChannelHandle h,    /* [in] BDSQ channel handle */
   uint32_t           addr, /* [in] configuration parameter address */
   uint32_t           *pVal /* [out] value of the configuration parameter */
)
{
   BDBG_ASSERT(h);
   BDBG_ASSERT(pVal);
   if (h->pDevice->settings.api.GetChannelConfig)
      return h->pDevice->settings.api.GetChannelConfig(h, addr, pVal);
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BDSQ_GetTxEventHandle()
******************************************************************************/
BERR_Code BDSQ_GetTxEventHandle(
   BDSQ_ChannelHandle h,       /* [in] BDSQ channel handle */
   BKNI_EventHandle   *hEvent  /* [out] DiSEqC Tx event handle */
)
{
   BDBG_ASSERT(h);
   BDBG_ASSERT(hEvent);
   if (h->pDevice->settings.api.GetTxEventHandle)
      return h->pDevice->settings.api.GetTxEventHandle(h, hEvent);
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BDSQ_GetRxEventHandle()
******************************************************************************/
BERR_Code BDSQ_GetRxEventHandle(
   BDSQ_ChannelHandle h,       /* [in] BDSQ channel handle */
   BKNI_EventHandle   *hEvent  /* [out] DiSEqC Rx event handle */
)
{
   BDBG_ASSERT(h);
   BDBG_ASSERT(hEvent);
   if (h->pDevice->settings.api.GetRxEventHandle)
      return h->pDevice->settings.api.GetRxEventHandle(h, hEvent);
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BDSQ_GetVsenseEventHandle()
******************************************************************************/
BERR_Code BDSQ_GetVsenseEventHandle(
   BDSQ_ChannelHandle h,             /* [in] BDSQ channel handle */
   BKNI_EventHandle *hVsenseEvent    /* [out] DiSEqC vsense event handle */
)
{
   BDBG_ASSERT(h);
   BDBG_ASSERT(hVsenseEvent);
   if (h->pDevice->settings.api.GetVsenseEventHandle)
      return h->pDevice->settings.api.GetVsenseEventHandle(h, hVsenseEvent);
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BDSQ_GetActivityEventHandle()
******************************************************************************/
BERR_Code BDSQ_GetActivityEventHandle(
   BDSQ_ChannelHandle h,             /* [in] BDSQ channel handle */
   BKNI_EventHandle *hActivityEvent  /* [out] DiSEqC activity event handle */
)
{
   BDBG_ASSERT(h);
   BDBG_ASSERT(hActivityEvent);
   if (h->pDevice->settings.api.GetActivityEventHandle)
      return h->pDevice->settings.api.GetActivityEventHandle(h, hActivityEvent);
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BDSQ_GetIdleEventHandle()
******************************************************************************/
BERR_Code BDSQ_GetIdleEventHandle(
   BDSQ_ChannelHandle h,             /* [in] BDSQ channel handle */
   BKNI_EventHandle *hIdleEvent  /* [out] DiSEqC idle event handle */
)
{
   BDBG_ASSERT(h);
   BDBG_ASSERT(hIdleEvent);
   if (h->pDevice->settings.api.GetIdleEventHandle)
      return h->pDevice->settings.api.GetIdleEventHandle(h, hIdleEvent);
   else
      return BERR_NOT_SUPPORTED;
}

#endif

