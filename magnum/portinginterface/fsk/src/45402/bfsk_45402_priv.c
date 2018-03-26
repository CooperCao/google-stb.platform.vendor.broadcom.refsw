/******************************************************************************
* Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
*
* Module Description:
*
*****************************************************************************/
#include "bstd.h"
#include "bkni.h"
#include "bhab_45402.h"
#include "bfsk.h"
#include "bfsk_protocol.h"
#include "bfsk_priv.h"
#include "bfsk_45402.h"
#include "bfsk_45402_priv.h"
#include "bchp_45402_leap_host_l2_0.h"


BDBG_MODULE(bfsk_45402_priv);


/* local functions */
BERR_Code BFSK_45402_P_InterruptCallback(void *pParm1, int parm2);
BERR_Code BFSK_45402_P_SendCommand(BHAB_Handle h, uint32_t *pBuf, uint32_t n);


/******************************************************************************
 BFSK_45402_P_Open
******************************************************************************/
BERR_Code BFSK_45402_P_Open(
   BFSK_Handle *h,                /* [out] BFSK handle */
   BCHP_Handle hChip,             /* [in] chip handle */
   void *pReg,                    /* [in] register or hab handle */
   BINT_Handle hInt,              /* [in] BINT handle */
   const BFSK_Settings *pSettings /* [in] default settings */
)
{
   BERR_Code retCode = BERR_SUCCESS;
   BFSK_Handle hDev;
   BFSK_45402_P_Handle *hImplDev;
   uint32_t numChannels, i;
   BHAB_Handle hHab;

   BSTD_UNUSED(hInt);
   BSTD_UNUSED(hChip);

   BDBG_ASSERT(pReg);
   hHab = (BHAB_Handle)pReg;

   /* allocate heap memory for device handle */
   hDev = (BFSK_Handle)BKNI_Malloc(sizeof(BFSK_P_Handle));
   BDBG_ASSERT(hDev);
   BKNI_Memset((void*)hDev, 0, sizeof(BFSK_P_Handle));
   hImplDev = (BFSK_45402_P_Handle *)BKNI_Malloc(sizeof(BFSK_45402_P_Handle));
   BDBG_ASSERT(hImplDev);
   BKNI_Memset((void*)hImplDev, 0, sizeof(BFSK_45402_P_Handle));
   hDev->pImpl = (void*)hImplDev;

   /* allocate heap memory for channel handle pointer */
   hDev->pChannels = (BFSK_P_ChannelHandle **)BKNI_Malloc(BFSK_45402_MAX_CHANNELS * sizeof(BFSK_P_ChannelHandle *));
   BDBG_ASSERT(hDev->pChannels);
   BKNI_Memset((void*)hDev->pChannels, 0, BFSK_45402_MAX_CHANNELS * sizeof(BFSK_P_ChannelHandle *));

   /* initialize device handle */
   BKNI_Memcpy((void*)(&(hDev->settings)), (void*)pSettings, sizeof(BFSK_Settings));
   hImplDev->hHab = (BHAB_Handle)pReg;

   if (BFSK_45402_P_GetTotalChannels(hDev, &numChannels) == BERR_SUCCESS)
      hDev->totalChannels = (uint8_t)numChannels;
   else
      hDev->totalChannels = BFSK_45402_MAX_CHANNELS;
   for (i = 0; i < hDev->totalChannels; i++)
      hDev->pChannels[i] = NULL;

   /* install callback */
   BHAB_InstallInterruptCallback(hHab, BHAB_DevId_eFSK, BFSK_45402_P_InterruptCallback, (void*)hDev, 0);

   *h = hDev;
   return retCode;
}


/******************************************************************************
 BFSK_45402_P_Close
******************************************************************************/
BERR_Code BFSK_45402_P_Close(
   BFSK_Handle h   /* [in] BFSK handle */
)
{
   BFSK_45402_P_Handle *pDevImpl = (BFSK_45402_P_Handle *)(h->pImpl);
   uint32_t mask;
   BERR_Code retCode;

   BDBG_ASSERT(h);

   /* disable fsk host interrupts */
   mask = BHAB_45402_HIRQ0_FSK_MASK;
   BFSK_45402_CHK_RETCODE(BHAB_WriteRegister(pDevImpl->hHab, BCHP_LEAP_HOST_L2_0_MASK_SET0, &mask));

   retCode = BHAB_UnInstallInterruptCallback(pDevImpl->hHab, BHAB_DevId_eFSK);
   BKNI_Free((void*)h->pChannels);
   BKNI_Free((void*)h->pImpl);
   BKNI_Free((void*)h);
   h = NULL;

   done:
   return retCode;
}


/******************************************************************************
 BFSK_45402_P_GetTotalChannels
******************************************************************************/
BERR_Code BFSK_45402_P_GetTotalChannels(
   BFSK_Handle h,             /* [in] BFSK handle */
   uint32_t    *totalChannels /* [out] number of satellite downstream channels supported by this chip */
)
{
   BSTD_UNUSED(h);

   *totalChannels = BFSK_45402_MAX_CHANNELS;
   return BERR_SUCCESS;
}


/******************************************************************************
 BFSK_45402_P_OpenChannel
******************************************************************************/
BERR_Code BFSK_45402_P_OpenChannel(
   BFSK_Handle                h,                /* [in] BFSK handle */
   BFSK_ChannelHandle         *pChannelHandle,  /* [out] BFSK channel handle */
   uint32_t                   chanNum,          /* [in] channel index (0 based) */
   const BFSK_ChannelSettings *pSettings        /* [in] channel settings */
)
{
   BERR_Code retCode;
   BFSK_ChannelSettings cs;
   BFSK_P_ChannelHandle *ch;
   BFSK_45402_P_ChannelHandle *chImpl;

   BDBG_ASSERT(h);
   BDBG_ASSERT(h->totalChannels <= BFSK_45402_MAX_CHANNELS);
   BDBG_ASSERT(chanNum < h->totalChannels);

   if (pSettings == NULL)
      BFSK_45402_GetChannelDefaultSettings(h, chanNum, &cs);
   else
      cs = *pSettings;

   /* allocate memory for the channel handle */
   ch = (BFSK_P_ChannelHandle *)BKNI_Malloc(sizeof(BFSK_P_ChannelHandle));
   BDBG_ASSERT(ch);
   BKNI_Memcpy((void*)(&ch->settings), (void*)&cs, sizeof(BFSK_ChannelSettings));
   ch->channel = (uint8_t)chanNum;
   ch->pDevice = h;
   h->pChannels[chanNum] = ch;

   chImpl = (BFSK_45402_P_ChannelHandle *)BKNI_Malloc(sizeof(BFSK_45402_P_ChannelHandle));
   BDBG_ASSERT(chImpl);

   retCode = BKNI_CreateEvent(&(chImpl->hRxEvent));
   if (retCode != BERR_SUCCESS)
   {
      BKNI_DestroyEvent(chImpl->hRxEvent);

      BKNI_Free((void*)chImpl);
      BKNI_Free((void*)ch);

      *pChannelHandle = NULL;
      return retCode;
   }

   retCode = BKNI_CreateEvent(&(chImpl->hTxEvent));
   if (retCode != BERR_SUCCESS)
   {
      BKNI_DestroyEvent(chImpl->hRxEvent);
      BKNI_DestroyEvent(chImpl->hTxEvent);

      BKNI_Free((void*)chImpl);
      BKNI_Free((void*)ch);

      *pChannelHandle = NULL;
      return retCode;
   }

   ch->pImpl = (void*)chImpl;
   *pChannelHandle = ch;

   return retCode;
}


/******************************************************************************
 BFSK_45402_P_CloseChannel
******************************************************************************/
BERR_Code BFSK_45402_P_CloseChannel(
   BFSK_ChannelHandle h  /* [in] BFSK channel handle */
)
{
   BFSK_45402_P_ChannelHandle *chImpl;

   chImpl = (BFSK_45402_P_ChannelHandle *)(h->pImpl);
   BKNI_DestroyEvent(chImpl->hRxEvent);
   BKNI_DestroyEvent(chImpl->hTxEvent);
   BKNI_Free((void*)chImpl);
   BKNI_Free((void*)h);
   h = NULL;

   return BERR_SUCCESS;
}


/******************************************************************************
 BFSK_45402_P_GetDevice
******************************************************************************/
BERR_Code BFSK_45402_P_GetDevice(
   BFSK_ChannelHandle h,      /* [in] BFSK channel handle */
   BFSK_Handle        *pDev   /* [out] BFSKhandle */
)
{
   *pDev = h->pDevice;
   return BERR_SUCCESS;
}


/******************************************************************************
 BFSK_45402_P_GetVersionInfo
******************************************************************************/
BERR_Code BFSK_45402_P_GetVersionInfo(
   BFSK_Handle h,                 /* [in] BFSK handle */
   BFEC_VersionInfo *pVersionInfo /* [out] version information */
)
{
   BDBG_ENTER(BFSK_45402_P_GetVersionInfo);

   BSTD_UNUSED(h);

   pVersionInfo->majorVersion = BFSK_API_VERSION;
   pVersionInfo->minorVersion = BFSK_45402_RELEASE_VERSION;
   pVersionInfo->buildType = 0;
   pVersionInfo->buildId = 0;
   return BERR_SUCCESS;

   BDBG_LEAVE(BFSK_45402_P_GetVersionInfo);
   return BERR_SUCCESS;
}


/******************************************************************************
 BFSK_45402_P_Reset
******************************************************************************/
BERR_Code BFSK_45402_P_Reset(
   BFSK_Handle h    /* [in] BFSK handle */
)
{
   BFSK_45402_P_Handle *pImpl = (BFSK_45402_P_Handle *)(h->pImpl);
   BERR_Code retCode;
   uint32_t buf[2], irq_mask;

   BDBG_ENTER(BFSK_45402_P_Reset);

   /* disable fsk host interrupts */
   irq_mask = BHAB_45402_HIRQ0_FSK_MASK;
   BFSK_45402_CHK_RETCODE(BHAB_WriteRegister(pImpl->hHab, BCHP_LEAP_HOST_L2_0_MASK_SET0, &irq_mask));

   buf[0] = BHAB_45402_InitHeader(0x09, 0, 0, BHAB_45402_MODULE_FSK);
   BFSK_45402_CHK_RETCODE(BFSK_45402_P_SendCommand(pImpl->hHab, buf, 2));

   BKNI_Sleep(1); /* wait for soft reset to complete */

   done:
   BDBG_LEAVE(BFSK_45402_P_Reset);
   return retCode;
}


/******************************************************************************
 BFSK_45402_P_ResetChannel
******************************************************************************/
BERR_Code BFSK_45402_P_ResetChannel(
   BFSK_ChannelHandle h    /* [in] BFSK channel handle */
)
{
   BFSK_45402_P_Handle *pDevImpl = (BFSK_45402_P_Handle *)(h->pDevice->pImpl);
   BERR_Code retCode;
   uint32_t buf[3], irq_mask;

   BDBG_ENTER(BFSK_45402_P_ResetChannel);

   /* clear and enable fsk irq */
   irq_mask = BHAB_45402_HIRQ0_FSK_MASK;
   BFSK_45402_CHK_RETCODE(BHAB_WriteRegister(pDevImpl->hHab, BCHP_LEAP_HOST_L2_0_CLEAR0, &irq_mask));
   BFSK_45402_CHK_RETCODE(BHAB_WriteRegister(pDevImpl->hHab, BCHP_LEAP_HOST_L2_0_MASK_CLEAR0, &irq_mask));

   /* send hab command */
   buf[0] = BHAB_45402_InitHeader(0x34, h->channel, 0, BHAB_45402_MODULE_FSK);
   buf[1] = 0;
   BFSK_45402_CHK_RETCODE(BFSK_45402_P_SendCommand(pDevImpl->hHab, buf, 3));

   done:
   BDBG_LEAVE(BFSK_45402_P_ResetChannel);
   return retCode;
}


/******************************************************************************
 BFSK_45402_P_PowerDownChannel
******************************************************************************/
BERR_Code BFSK_45402_P_PowerDownChannel(
   BFSK_ChannelHandle h    /* [in] BFSK channel handle */
)
{
   BFSK_45402_P_Handle *pDevImpl = (BFSK_45402_P_Handle *)(h->pDevice->pImpl);
   BERR_Code retCode;
   uint32_t buf[3], irq_mask;

   BDBG_ENTER(BFSK_45402_P_PowerDownChannel);

   /* disable fsk host interrupts */
   irq_mask = BHAB_45402_HIRQ0_FSK_MASK;
   BFSK_45402_CHK_RETCODE(BHAB_WriteRegister(pDevImpl->hHab, BCHP_LEAP_HOST_L2_0_MASK_SET0, &irq_mask));

   buf[0] = BHAB_45402_InitHeader(0x25, h->channel, 0, BHAB_45402_MODULE_FSK);
   buf[1] = 1; /* power down */
   BFSK_45402_CHK_RETCODE(BFSK_45402_P_SendCommand(pDevImpl->hHab, buf, 3));

   done:
   BDBG_LEAVE(BFSK_45402_P_PowerDownChannel);
   return retCode;
}


/******************************************************************************
 BFSK_45402_P_PowerUpChannel
******************************************************************************/
BERR_Code BFSK_45402_P_PowerUpChannel(
   BFSK_ChannelHandle h  /* [in] BFSK channel handle */
)
{
   BFSK_45402_P_Handle *pDevImpl = (BFSK_45402_P_Handle *)(h->pDevice->pImpl);
   BERR_Code retCode;
   uint32_t buf[3], irq_mask;

   BDBG_ENTER(BFSK_45402_P_PowerUpChannel);

   /* clear and enable fsk irq */
   irq_mask = BHAB_45402_HIRQ0_FSK_MASK;
   BFSK_45402_CHK_RETCODE(BHAB_WriteRegister(pDevImpl->hHab, BCHP_LEAP_HOST_L2_0_CLEAR0, &irq_mask));
   BFSK_45402_CHK_RETCODE(BHAB_WriteRegister(pDevImpl->hHab, BCHP_LEAP_HOST_L2_0_MASK_CLEAR0, &irq_mask));

   buf[0] = BHAB_45402_InitHeader(0x25, h->channel, 0, BHAB_45402_MODULE_FSK);
   buf[1] = 0; /* power up */
   BFSK_45402_CHK_RETCODE(BFSK_45402_P_SendCommand(pDevImpl->hHab, buf, 3));

   done:
   BDBG_LEAVE(BFSK_45402_P_PowerUpChannel);
   return retCode;
}


/******************************************************************************
 BFSK_45402_P_IsChannelOn()
******************************************************************************/
BERR_Code BFSK_45402_P_IsChannelOn(BFSK_ChannelHandle h, bool *pbOn)
{
   BERR_Code retCode;
   uint32_t buf[3];
   BFSK_45402_P_Handle *pImpl = (BFSK_45402_P_Handle *)(h->pDevice->pImpl);

   BDBG_ENTER(BFSK_45402_P_IsChannelOn);

   buf[0] = BHAB_45402_InitHeader(0x32, h->channel, 0, BHAB_45402_MODULE_FSK);
   buf[1] = 0;
   BFSK_45402_CHK_RETCODE(BFSK_45402_P_SendCommand(pImpl->hHab, buf, 3));

   *pbOn = (buf[1] == 1) ? true : false;

   done:
   BDBG_LEAVE(BFSK_45402_P_IsChannelOn);
   return retCode;
}


/******************************************************************************
 BFSK_45402_P_EnableCarrier()
******************************************************************************/
BERR_Code BFSK_45402_P_EnableCarrier(BFSK_ChannelHandle h)
{
   BFSK_45402_P_Handle *pDevImpl = (BFSK_45402_P_Handle *)(h->pDevice->pImpl);
   BERR_Code retCode;
   uint32_t buf[3];

   BDBG_ENTER(BFSK_45402_P_EnableCarrier);

   buf[0] = BHAB_45402_InitHeader(0x38, h->channel, 0, BHAB_45402_MODULE_FSK);
   buf[1] = 1; /* enable carrier */
   BFSK_45402_CHK_RETCODE(BFSK_45402_P_SendCommand(pDevImpl->hHab, buf, 3));

   done:
   BDBG_LEAVE(BFSK_45402_P_EnableCarrier);
   return retCode;
}


/******************************************************************************
 BFSK_45402_P_DisableCarrier()
******************************************************************************/
BERR_Code BFSK_45402_P_DisableCarrier(BFSK_ChannelHandle h)
{
   BFSK_45402_P_Handle *pDevImpl = (BFSK_45402_P_Handle *)(h->pDevice->pImpl);
   BERR_Code retCode;
   uint32_t buf[3];

   BDBG_ENTER(BFSK_45402_P_DisableCarrier);

   buf[0] = BHAB_45402_InitHeader(0x38, h->channel, 0, BHAB_45402_MODULE_FSK);
   buf[1] = 0; /* disable carrier */
   BFSK_45402_CHK_RETCODE(BFSK_45402_P_SendCommand(pDevImpl->hHab, buf, 3));

   done:
   BDBG_LEAVE(BFSK_45402_P_DisableCarrier);
   return retCode;
}


/******************************************************************************
 BFSK_45402_P_Write
******************************************************************************/
BERR_Code BFSK_45402_P_Write(BFSK_ChannelHandle h, uint8_t *pBuf, uint8_t n, void *pParams)
{
   BFSK_45402_P_Handle *pDevImpl = (BFSK_45402_P_Handle *)(h->pDevice->pImpl);
   BERR_Code retCode;
   uint32_t buf[128], i, irq_mask;

#ifndef BFSK_PROTOCOL_ECHO
   BSTD_UNUSED(pParams);
#endif

   BDBG_ENTER(BFSK_45402_P_Write);

   if (n > 122)
      return BERR_INVALID_PARAMETER;

   buf[0] = BHAB_45402_InitHeader(0x30, h->channel, 0, 0);
   buf[1] = n;

   /* copy packet data */
   for (i = 0; i < n; i++)
      buf[2+i] = pBuf[i];

#ifdef BFSK_PROTOCOL_ECHO
   buf[2+n] = ((BFSK_P_TxParams *)pParams)->tdmaTxSlot;
#else
   buf[2+n] = 0;
#endif

   /* clear and enable fsk irq */
   irq_mask = BHAB_45402_HIRQ0_FSK_MASK;
   BFSK_45402_CHK_RETCODE(BHAB_WriteRegister(pDevImpl->hHab, BCHP_LEAP_HOST_L2_0_CLEAR0, &irq_mask));
   BFSK_45402_CHK_RETCODE(BHAB_WriteRegister(pDevImpl->hHab, BCHP_LEAP_HOST_L2_0_MASK_CLEAR0, &irq_mask));

   /* send hab command */
   BFSK_45402_CHK_RETCODE(BFSK_45402_P_SendCommand(pDevImpl->hHab, buf, n+4));

   done:
   BDBG_LEAVE(BFSK_45402_P_Write);
   return retCode;
}


/******************************************************************************
 BFSK_45402_P_SetRxCount
******************************************************************************/
BERR_Code BFSK_45402_P_SetRxCount(BFSK_ChannelHandle h, uint8_t n)
{
   BFSK_45402_P_Handle *pDevImpl = (BFSK_45402_P_Handle *)(h->pDevice->pImpl);
   BERR_Code retCode;
   uint32_t buf[3];

   BDBG_ENTER(BFSK_45402_P_SetRxCount);

   buf[0] = BHAB_45402_InitHeader(0x2E, h->channel, 0, 0);
   buf[1] = n; /* number of bytes to expect */
   BFSK_45402_CHK_RETCODE(BFSK_45402_P_SendCommand(pDevImpl->hHab, buf, 3));

   done:
   BDBG_LEAVE(BFSK_45402_P_SetRxCount);
   return retCode;
}


/******************************************************************************
 BFSK_45402_P_Read
******************************************************************************/
BERR_Code BFSK_45402_P_Read(BFSK_ChannelHandle h, uint8_t *pBuf, uint8_t nBufMax, uint8_t *n, uint8_t *nNotRead)
{
   BFSK_45402_P_Handle *pDevImpl = (BFSK_45402_P_Handle *)(h->pDevice->pImpl);
   BERR_Code retCode;
   uint32_t buf[128], i;
   uint8_t len;

   BDBG_ENTER(BFSK_45402_P_Read);

   if (nBufMax > 122)
      return BERR_INVALID_PARAMETER;

   buf[0] = BHAB_45402_InitHeader(0x2F, h->channel, 0, 0);
   buf[1] = nBufMax;
   BFSK_45402_CHK_RETCODE(BFSK_45402_P_SendCommand(pDevImpl->hHab, buf, nBufMax+5));

   len = buf[2] & 0xFF;
   *nNotRead = buf[3] & 0xFF;

   /* copy data to buffer */
   for (i = 0; i < len; i++)
      pBuf[i] = (uint8_t)(buf[4+i] & 0xFF);
   *n = len;

   done:
   BDBG_LEAVE(BFSK_45402_P_Read);
   return retCode;
}


/******************************************************************************
 BFSK_45402_P_GetRssiLevel
******************************************************************************/
BERR_Code BFSK_45402_P_GetRssiLevel(BFSK_ChannelHandle h, uint32_t *pLevel)
{
   BFSK_45402_P_Handle *pDevImpl = (BFSK_45402_P_Handle *)(h->pDevice->pImpl);
   BERR_Code retCode;
   uint32_t hab[3];

   BDBG_ENTER(BFSK_45402_P_GetRssiLevel);

   hab[0] = BHAB_45402_InitHeader(0x42, h->channel, 0, 0);
   BFSK_45402_CHK_RETCODE(BFSK_45402_P_SendCommand(pDevImpl->hHab, hab, 3));

   *pLevel = hab[1];

   done:
   BDBG_LEAVE(BFSK_45402_P_GetRssiLevel);
   return retCode;
}


/******************************************************************************
 BFSK_45402_P_GetRxEventHandle
******************************************************************************/
BERR_Code BFSK_45402_P_GetRxEventHandle(BFSK_ChannelHandle h, BKNI_EventHandle *hRxEvent)
{
   BDBG_ENTER(BFSK_45402_P_GetRxEventHandle);

   *hRxEvent = ((BFSK_45402_P_ChannelHandle *)(h->pImpl))->hRxEvent;

   BDBG_LEAVE(BFSK_45402_P_GetRxEventHandle);
   return BERR_SUCCESS;
}



/******************************************************************************
 BFSK_45402_P_GetTxEventHandle
******************************************************************************/
BERR_Code BFSK_45402_P_GetTxEventHandle(BFSK_ChannelHandle h, BKNI_EventHandle *hTxEvent)
{
   BDBG_ENTER(BFSK_45402_P_GetTxEventHandle);

   *hTxEvent = ((BFSK_45402_P_ChannelHandle *)(h->pImpl))->hTxEvent;

   BDBG_LEAVE(BFSK_45402_P_GetTxEventHandle);
   return BERR_SUCCESS;
}


/******************************************************************************
 BFSK_45402_P_SetConfig
******************************************************************************/
BERR_Code BFSK_45402_P_SetConfig(BFSK_Handle h, uint32_t addr, uint32_t val)
{
   BSTD_UNUSED(h);
   BSTD_UNUSED(addr);
   BSTD_UNUSED(val);
   return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BFSK_45402_P_GetConfig
******************************************************************************/
BERR_Code BFSK_45402_P_GetConfig(BFSK_Handle h, uint32_t addr, uint32_t *pVal)
{
   BSTD_UNUSED(h);
   BSTD_UNUSED(addr);
   BSTD_UNUSED(pVal);
   return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BFSK_45402_P_SetChannelConfig
******************************************************************************/
BERR_Code BFSK_45402_P_SetChannelConfig(BFSK_ChannelHandle h, uint32_t addr, uint32_t val)
{
   BFSK_45402_P_Handle *pDevImpl = (BFSK_45402_P_Handle *)(h->pDevice->pImpl);
   BERR_Code retCode;
   uint32_t hab[4];

   BDBG_ENTER(BFSK_45402_P_SetChannelConfig);

#ifndef BFSK_PROTOCOL_ECHO
   if (addr >= BFSK_45402_CHAN_CONFIG_MAX)
      return BERR_INVALID_PARAMETER;
#else
   if ((addr >= BFSK_45402_CHAN_CONFIG_MAX) && (addr < BFSK_PROTOCOL_CONFIG_OFFSET))
      retCode = BERR_INVALID_PARAMETER;
   else if (addr >= BFSK_PROTOCOL_CONFIG_OFFSET + BFSK_PROTOCOL_CONFIG_MAX)
      retCode = BERR_INVALID_PARAMETER;
#endif

   hab[0] = BHAB_45402_InitHeader(0x05, h->channel, BHAB_45402_WRITE, BHAB_45402_MODULE_FSK);
   hab[1] = addr;
   hab[2] = val;
   BFSK_45402_CHK_RETCODE(BFSK_45402_P_SendCommand(pDevImpl->hHab, hab, 4));

   done:
   BDBG_LEAVE(BFSK_45402_P_SetChannelConfig);
   return retCode;
}


/******************************************************************************
 BFSK_45402_P_GetChannelConfig
******************************************************************************/
BERR_Code BFSK_45402_P_GetChannelConfig(BFSK_ChannelHandle h, uint32_t addr, uint32_t *pVal)
{
   BFSK_45402_P_Handle *pDevImpl = (BFSK_45402_P_Handle *)(h->pDevice->pImpl);
   BERR_Code retCode;
   uint32_t hab[4];

   BDBG_ENTER(BFSK_45402_P_GetChannelConfig);

#ifndef BFSK_PROTOCOL_ECHO
   if (addr >= BFSK_45402_CHAN_CONFIG_MAX)
      return BERR_INVALID_PARAMETER;
#else
   if ((addr >= BFSK_45402_CHAN_CONFIG_MAX) && (addr < BFSK_PROTOCOL_CONFIG_OFFSET))
      retCode = BERR_INVALID_PARAMETER;
   else if (addr >= BFSK_PROTOCOL_CONFIG_OFFSET + BFSK_PROTOCOL_CONFIG_MAX)
      retCode = BERR_INVALID_PARAMETER;
#endif

   hab[0] = BHAB_45402_InitHeader(0x05, h->channel, BHAB_45402_READ, BHAB_45402_MODULE_FSK);
   hab[1] = addr;
   BFSK_45402_CHK_RETCODE(BFSK_45402_P_SendCommand(pDevImpl->hHab, hab, 4));

   *pVal = hab[2];

   done:
   BDBG_LEAVE(BFSK_45402_P_GetChannelConfig);
   return retCode;
}


/******************************************************************************
 BFSK_45402_P_InterruptCallback()
******************************************************************************/
BERR_Code BFSK_45402_P_InterruptCallback(void *pParm1, int parm2)
{
   BHAB_45402_IrqStatus *pParams = (BHAB_45402_IrqStatus *)pParm1;
   BFSK_Handle h = (BFSK_Handle)(pParams->pParm1);
   BFSK_45402_P_Handle *pImpl = (BFSK_45402_P_Handle *)(h->pImpl);
   uint32_t fstatus0 = pParams->status0;
   BFSK_45402_P_ChannelHandle *pChn;

   BSTD_UNUSED(parm2);
   BSTD_UNUSED(pImpl);

   if (fstatus0 & BHAB_45402_HIRQ0_FSK_MASK)
   {
      pChn = (BFSK_45402_P_ChannelHandle *)(h->pChannels[0]->pImpl);
      BDBG_ASSERT(pChn != NULL);
      if (fstatus0 & BHAB_45402_HIRQ0_FSK_RX)
      {
         BKNI_SetEvent(pChn->hRxEvent);
      }
      if (fstatus0 & BHAB_45402_HIRQ0_FSK_TX)
      {
         BKNI_SetEvent(pChn->hTxEvent);
      }
   }

   return BERR_SUCCESS;
}


/******************************************************************************
 BFSK_45402_P_SendCommand()
******************************************************************************/
BERR_Code BFSK_45402_P_SendCommand(BHAB_Handle h, uint32_t *pBuf, uint32_t n)
{
   return BHAB_SendHabCommand(h, (uint8_t*)pBuf, (uint16_t)n, (uint8_t*)pBuf, (uint16_t)n, true, true, 0);
}
