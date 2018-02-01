/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.

 ******************************************************************************/
#include "bstd.h"
#include "bkni.h"
#include "bhab_45308.h"
#include "bdsq.h"
#include "bdsq_priv.h"
#include "bdsq_45308.h"
#include "bdsq_45308_priv.h"
#include "bchp_45308_leap_host_l2.h"


BDBG_MODULE(bdsq_45308_priv);

#define BDSQ_DEBUG(x) /* x */


/* local functions */
static BERR_Code BDSQ_45308_P_InterruptCallback(void *pParm1, int parm2);
static BERR_Code BDSQ_45308_P_SendCommand(BHAB_Handle h, uint32_t *pBuf, uint32_t n);
static BERR_Code BDSQ_45308_P_EnableChannelInterrupt(BDSQ_ChannelHandle h, bool bEnable, uint32_t mask);


/******************************************************************************
 BDSQ_45308_P_Open
******************************************************************************/
BERR_Code BDSQ_45308_P_Open(
   BDSQ_Handle *h,                /* [out] BDSQ handle */
   BCHP_Handle hChip,             /* [in] chip handle */
   void *pReg,                    /* [in] register or hab handle */
   BINT_Handle hInt,              /* [in] BINT handle */
   const BDSQ_Settings *pSettings /* [in] default settings */
)
{
   BERR_Code retCode = BERR_SUCCESS;
   BDSQ_Handle hDev;
   BDSQ_45308_P_Handle *pDevImpl;
   uint32_t numChannels, i;
   BHAB_Handle hHab;

   BSTD_UNUSED(hInt);
   BSTD_UNUSED(hChip);

   BDBG_ASSERT(pReg);
   hHab = (BHAB_Handle)pReg;

   /* allocate heap memory for device handle */
   hDev = (BDSQ_Handle)BKNI_Malloc(sizeof(BDSQ_P_Handle));
   BDBG_ASSERT(hDev);
   BKNI_Memset((void*)hDev, 0, sizeof(BDSQ_P_Handle));
   pDevImpl = (BDSQ_45308_P_Handle *)BKNI_Malloc(sizeof(BDSQ_45308_P_Handle));
   BDBG_ASSERT(pDevImpl);
   BKNI_Memset((void*)pDevImpl, 0, sizeof(BDSQ_45308_P_Handle));
   hDev->pImpl = (void*)pDevImpl;

   /* allocate heap memory for channel handle pointer */
   hDev->pChannels = (BDSQ_P_ChannelHandle **)BKNI_Malloc(BDSQ_45308_MAX_CHANNELS * sizeof(BDSQ_P_ChannelHandle *));
   BDBG_ASSERT(hDev->pChannels);
   BKNI_Memset((void*)hDev->pChannels, 0, BDSQ_45308_MAX_CHANNELS * sizeof(BDSQ_P_ChannelHandle *));

   /* initialize device handle */
   BKNI_Memcpy((void*)(&(hDev->settings)), (void*)pSettings, sizeof(BDSQ_Settings));
   pDevImpl->hHab = (BHAB_Handle)pReg;

   if (BDSQ_45308_P_GetTotalChannels(hDev, &numChannels) == BERR_SUCCESS)
      hDev->totalChannels = (uint8_t)numChannels;
   else
      hDev->totalChannels = BDSQ_45308_MAX_CHANNELS;
   for (i = 0; i < hDev->totalChannels; i++)
      hDev->pChannels[i] = NULL;

   /* install callback */
   BHAB_InstallInterruptCallback(hHab, BHAB_DevId_eDSQ, BDSQ_45308_P_InterruptCallback, (void*)hDev, 0);

   *h = hDev;
   return retCode;
}


/******************************************************************************
 BDSQ_45308_P_Close
******************************************************************************/
BERR_Code BDSQ_45308_P_Close(
   BDSQ_Handle h   /* [in] BDSQ handle */
)
{
   BDSQ_45308_P_Handle *pDevImpl;
   uint32_t mask;
   BERR_Code retCode;

   BDBG_ASSERT(h);
   pDevImpl = (BDSQ_45308_P_Handle *)(h->pImpl);

   /* disable dsq host interrupts */
   mask = BHAB_45308_HIRQ0_DSQ_MASK;
   BDSQ_45308_CHK_RETCODE(BHAB_WriteRegister(pDevImpl->hHab, BCHP_LEAP_HOST_L2_MASK_SET0, &mask));

   retCode = BHAB_UnInstallInterruptCallback(pDevImpl->hHab, BHAB_DevId_eDSQ);
   BKNI_Free((void*)h->pChannels);
   BKNI_Free((void*)h->pImpl);
   BKNI_Free((void*)h);
   h = NULL;

   done:
   return retCode;
}


/******************************************************************************
 BDSQ_45308_P_GetTotalChannels
******************************************************************************/
BERR_Code BDSQ_45308_P_GetTotalChannels(
   BDSQ_Handle h,             /* [in] BDSQ handle */
   uint32_t    *totalChannels /* [out] number of satellite downstream channels supported by this chip */
)
{
   BSTD_UNUSED(h);
   *totalChannels = BDSQ_45308_MAX_CHANNELS;
   return BERR_SUCCESS;
}


/******************************************************************************
 BDSQ_45308_P_OpenChannel
******************************************************************************/
BERR_Code BDSQ_45308_P_OpenChannel(
   BDSQ_Handle                h,                /* [in] BDSQ handle */
   BDSQ_ChannelHandle         *pChannelHandle,  /* [out] BDSQ channel handle */
   uint32_t                   chanNum,          /* [in] channel index (0 based) */
   const BDSQ_ChannelSettings *pSettings        /* [in] channel settings */
)
{
   BERR_Code retCode;
   BDSQ_ChannelSettings cs;
   BDSQ_P_ChannelHandle *ch;
   BDSQ_45308_P_ChannelHandle *chImpl;

   BDBG_ASSERT(h);
   BDBG_ASSERT(h->totalChannels <= BDSQ_45308_MAX_CHANNELS);
   BDBG_ASSERT(chanNum < h->totalChannels);

   if (pSettings == NULL)
      BDSQ_45308_GetChannelDefaultSettings(h, chanNum, &cs);
   else
      cs = *pSettings;

   /* allocate memory for the channel handle */
   ch = (BDSQ_P_ChannelHandle *)BKNI_Malloc(sizeof(BDSQ_P_ChannelHandle));
   BDBG_ASSERT(ch);
   BKNI_Memcpy((void*)(&ch->settings), (void*)&cs, sizeof(BDSQ_ChannelSettings));
   ch->channel = (uint8_t)chanNum;
   ch->pDevice = h;
   h->pChannels[chanNum] = ch;

   chImpl = (BDSQ_45308_P_ChannelHandle *)BKNI_Malloc(sizeof(BDSQ_45308_P_ChannelHandle));
   BDBG_ASSERT(chImpl);

   retCode = BKNI_CreateEvent(&(chImpl->hTxEvent));
   if (retCode != BERR_SUCCESS)
   {
      BKNI_Free((void*)chImpl);
      BKNI_Free((void*)ch);

      *pChannelHandle = NULL;
      return retCode;
   }

   retCode = BKNI_CreateEvent(&(chImpl->hRxEvent));
   if (retCode != BERR_SUCCESS)
   {
      BKNI_Free((void*)chImpl);
      BKNI_Free((void*)ch);

      *pChannelHandle = NULL;
      return retCode;
   }

   retCode = BKNI_CreateEvent(&(chImpl->hVsenseEvent));
   if (retCode != BERR_SUCCESS)
   {
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
 BDSQ_45308_P_CloseChannel
******************************************************************************/
BERR_Code BDSQ_45308_P_CloseChannel(
   BDSQ_ChannelHandle h  /* [in] BDSQ channel handle */
)
{
   BDSQ_45308_P_ChannelHandle *chImpl;

   chImpl = (BDSQ_45308_P_ChannelHandle *)(h->pImpl);
   BKNI_DestroyEvent(chImpl->hTxEvent);
   BKNI_DestroyEvent(chImpl->hRxEvent);
   BKNI_DestroyEvent(chImpl->hVsenseEvent);
   BKNI_Free((void*)chImpl);
   BKNI_Free((void*)h);
   h = NULL;

   return BERR_SUCCESS;
}


/******************************************************************************
 BDSQ_45308_P_GetDevice
******************************************************************************/
BERR_Code BDSQ_45308_P_GetDevice(
   BDSQ_ChannelHandle h,      /* [in] BDSQ channel handle */
   BDSQ_Handle        *pDev   /* [out] BDSQhandle */
)
{
   *pDev = h->pDevice;
   return BERR_SUCCESS;
}


/******************************************************************************
 BDSQ_45308_P_GetVersionInfo
******************************************************************************/
BERR_Code BDSQ_45308_P_GetVersionInfo(
   BDSQ_Handle h,                 /* [in] BDSQ handle */
   BFEC_VersionInfo *pVersionInfo /* [out] version information */
)
{
   BDBG_ENTER(BDSQ_45308_P_GetVersionInfo);

   BSTD_UNUSED(h);

   pVersionInfo->majorVersion = BDSQ_API_VERSION;
   pVersionInfo->minorVersion = BDSQ_45308_RELEASE_VERSION;
   pVersionInfo->buildType = 0;
   pVersionInfo->buildId = 0;
   return BERR_SUCCESS;

   BDBG_LEAVE(BDSQ_45308_P_GetVersionInfo);
   return BERR_SUCCESS;
}


/******************************************************************************
 BDSQ_45308_P_Reset
******************************************************************************/
BERR_Code BDSQ_45308_P_Reset(
   BDSQ_Handle h    /* [in] BDSQ handle */
)
{
   BDSQ_45308_P_Handle *pImpl = (BDSQ_45308_P_Handle *)(h->pImpl);
   BERR_Code retCode;
   uint32_t buf[2], irq_mask;

   BDBG_ENTER(BDSQ_45308_P_Reset);

   /* disable dsq host interrupts */
   irq_mask = BHAB_45308_HIRQ0_DSQ_MASK;
   BDSQ_45308_CHK_RETCODE(BHAB_WriteRegister(pImpl->hHab, BCHP_LEAP_HOST_L2_MASK_SET0, &irq_mask));

   buf[0] = BHAB_45308_InitHeader(0x09, 0, 0, BHAB_45308_MODULE_DSQ);
   BDSQ_45308_CHK_RETCODE(BDSQ_45308_P_SendCommand(pImpl->hHab, buf, 2));

   BKNI_Sleep(1); /* wait for soft reset to complete */

   done:
   BDBG_LEAVE(BDSQ_45308_P_Reset);
   return retCode;
}


/******************************************************************************
 BDSQ_45308_P_ResetChannel
******************************************************************************/
BERR_Code BDSQ_45308_P_ResetChannel(
   BDSQ_ChannelHandle h    /* [in] BDSQ channel handle */
)
{
   BDSQ_45308_P_Handle *pDevImpl = (BDSQ_45308_P_Handle *)(h->pDevice->pImpl);
   BERR_Code retCode;
   uint32_t buf[3];

   BDBG_ENTER(BDSQ_45308_P_ResetChannel);

   /* disable dsq host interrupts for channel */
   BDSQ_45308_P_EnableChannelInterrupt(h, false, BHAB_45308_DSEC_IRQ_ALL);

   /* TBD hab command */
   buf[0] = BHAB_45308_InitHeader(0x34, h->channel, 0, BHAB_45308_MODULE_DSQ);
   buf[1] = 0;
   BDSQ_45308_CHK_RETCODE(BDSQ_45308_P_SendCommand(pDevImpl->hHab, buf, 3));

   done:
   BDBG_LEAVE(BDSQ_45308_P_ResetChannel);
   return retCode;
}


/******************************************************************************
 BDSQ_45308_P_PowerDownChannel
******************************************************************************/
BERR_Code BDSQ_45308_P_PowerDownChannel(
   BDSQ_ChannelHandle h    /* [in] BDSQ channel handle */
)
{
   BDSQ_45308_P_Handle *pDevImpl = (BDSQ_45308_P_Handle *)(h->pDevice->pImpl);
   BERR_Code retCode;
   uint32_t buf[3];

   BDBG_ENTER(BDSQ_45308_P_PowerDownChannel);

   buf[0] = BHAB_45308_InitHeader(0x25, h->channel, 0, BHAB_45308_MODULE_DSQ);
   buf[1] = 1; /* power down */
   BDSQ_45308_CHK_RETCODE(BDSQ_45308_P_SendCommand(pDevImpl->hHab, buf, 3));

   done:
   BDBG_LEAVE(BDSQ_45308_P_PowerDownChannel);
   return retCode;
}


/******************************************************************************
 BDSQ_45308_P_PowerUpChannel
******************************************************************************/
BERR_Code BDSQ_45308_P_PowerUpChannel(
   BDSQ_ChannelHandle h  /* [in] BDSQ channel handle */
)
{
   BDSQ_45308_P_Handle *pDevImpl = (BDSQ_45308_P_Handle *)(h->pDevice->pImpl);
   BERR_Code retCode;
   uint32_t buf[3];

   BDBG_ENTER(BDSQ_45308_P_PowerUpChannel);

   buf[0] = BHAB_45308_InitHeader(0x25, h->channel, 0, BHAB_45308_MODULE_DSQ);
   buf[1] = 0; /* power up */
   BDSQ_45308_CHK_RETCODE(BDSQ_45308_P_SendCommand(pDevImpl->hHab, buf, 3));

   done:
   BDBG_LEAVE(BDSQ_45308_P_PowerUpChannel);
   return retCode;
}


/******************************************************************************
 BDSQ_45308_P_IsChannelOn()
******************************************************************************/
BERR_Code BDSQ_45308_P_IsChannelOn(BDSQ_ChannelHandle h, bool *pbOn)
{
   BERR_Code retCode;
   uint32_t buf[3];
   BDSQ_45308_P_Handle *pImpl = (BDSQ_45308_P_Handle *)(h->pDevice->pImpl);

   BDBG_ENTER(BDSQ_45308_P_IsChannelOn);

   buf[0] = BHAB_45308_InitHeader(0x32, h->channel, 0, BHAB_45308_MODULE_DSQ);
   buf[1] = 0;
   BDSQ_45308_CHK_RETCODE(BDSQ_45308_P_SendCommand(pImpl->hHab, buf, 3));

   *pbOn = (buf[1] == 1) ? true : false;

   done:
   BDBG_LEAVE(BDSQ_45308_P_IsChannelOn);
   return retCode;
}


/******************************************************************************
 BDSQ_45308_P_PowerDownVsense
******************************************************************************/
BERR_Code BDSQ_45308_P_PowerDownVsense(
   BDSQ_ChannelHandle h    /* [in] BDSQ channel handle */
)
{
   BDSQ_45308_P_Handle *pDevImpl = (BDSQ_45308_P_Handle *)(h->pDevice->pImpl);
   BERR_Code retCode;
   uint32_t buf[3];

   BDBG_ENTER(BDSQ_45308_P_PowerDownChannel);

   buf[0] = BHAB_45308_InitHeader(0x25, h->channel, 0, BHAB_45308_MODULE_VSN);
   buf[1] = 1; /* power down */
   BDSQ_45308_CHK_RETCODE(BDSQ_45308_P_SendCommand(pDevImpl->hHab, buf, 3));

   done:
   BDBG_LEAVE(BDSQ_45308_P_PowerDownChannel);
   return retCode;
}


/******************************************************************************
 BDSQ_45308_P_PowerUpVsense
******************************************************************************/
BERR_Code BDSQ_45308_P_PowerUpVsense(
   BDSQ_ChannelHandle h  /* [in] BDSQ channel handle */
)
{
   BDSQ_45308_P_Handle *pDevImpl = (BDSQ_45308_P_Handle *)(h->pDevice->pImpl);
   BERR_Code retCode;
   uint32_t buf[3];

   BDBG_ENTER(BDSQ_45308_P_PowerUpChannel);

   buf[0] = BHAB_45308_InitHeader(0x25, h->channel, 0, BHAB_45308_MODULE_VSN);
   buf[1] = 0; /* power up */
   BDSQ_45308_CHK_RETCODE(BDSQ_45308_P_SendCommand(pDevImpl->hHab, buf, 3));

   done:
   BDBG_LEAVE(BDSQ_45308_P_PowerUpChannel);
   return retCode;
}


/******************************************************************************
 BDSQ_45308_P_IsVsenseOn()
******************************************************************************/
BERR_Code BDSQ_45308_P_IsVsenseOn(BDSQ_ChannelHandle h, bool *pbOn)
{
   BERR_Code retCode;
   uint32_t buf[3];
   BDSQ_45308_P_Handle *pImpl = (BDSQ_45308_P_Handle *)(h->pDevice->pImpl);

   BDBG_ENTER(BDSQ_45308_P_IsChannelOn);

   buf[0] = BHAB_45308_InitHeader(0x32, h->channel, 0, BHAB_45308_MODULE_DSQ);
   buf[1] = 0;
   BDSQ_45308_CHK_RETCODE(BDSQ_45308_P_SendCommand(pImpl->hHab, buf, 3));

   *pbOn = (buf[1] == 1) ? true : false;

   done:
   BDBG_LEAVE(BDSQ_45308_P_IsChannelOn);
   return retCode;
}


/******************************************************************************
 BDSQ_45308_P_SetTone
******************************************************************************/
BERR_Code BDSQ_45308_P_SetTone(
   BDSQ_ChannelHandle h,  /* [in] BDSQ channel handle */
   bool bEnable           /* [in] true = tone on, false = tone off */
)
{
   BDSQ_45308_P_Handle *pDevImpl = (BDSQ_45308_P_Handle *)(h->pDevice->pImpl);
   BERR_Code retCode;
   uint32_t buf[3];

   BDBG_ENTER(BDSQ_45308_P_SetTone);

   buf[0] = BHAB_45308_InitHeader(0x0A, h->channel, BHAB_45308_WRITE, 0);
   buf[1] = bEnable;
   BDSQ_45308_CHK_RETCODE(BDSQ_45308_P_SendCommand(pDevImpl->hHab, buf, 3));

   done:
   BDBG_LEAVE(BDSQ_45308_P_SetTone);
   return retCode;
}


/******************************************************************************
 BDSQ_45308_P_GetTone
******************************************************************************/
BERR_Code BDSQ_45308_P_GetTone(
   BDSQ_ChannelHandle h,  /* [in] BDSQ channel handle */
   bool *pbTone           /* [out] true = tone is present, false = tone absent */
)
{
   BDSQ_45308_P_Handle *pDevImpl = (BDSQ_45308_P_Handle *)(h->pDevice->pImpl);
   BERR_Code retCode;
   uint32_t buf[3];

   BDBG_ENTER(BDSQ_45308_P_GetTone);

   buf[0] = BHAB_45308_InitHeader(0x0A, h->channel, BHAB_45308_READ, 0);
   BDSQ_45308_CHK_RETCODE(BDSQ_45308_P_SendCommand(pDevImpl->hHab, buf, 3));

   *pbTone = buf[1] ? true : false;

   done:
   BDBG_LEAVE(BDSQ_45308_P_GetTone);
   return retCode;
}


/******************************************************************************
 BDSQ_45308_P_SetVoltage
******************************************************************************/
BERR_Code BDSQ_45308_P_SetVoltage(
   BDSQ_ChannelHandle h, /* [in] BDSQ channel handle */
   bool bVtop            /* [in] true = VTOP, false = VBOT */
)
{
   BDSQ_45308_P_Handle *pDevImpl = (BDSQ_45308_P_Handle *)(h->pDevice->pImpl);
   BERR_Code retCode;
   uint32_t buf[3];

   BDBG_ENTER(BDSQ_45308_P_SetVoltage);

   buf[0] = BHAB_45308_InitHeader(0x0B, h->channel, BHAB_45308_WRITE, 0);
   buf[1] = bVtop;
   BDSQ_45308_CHK_RETCODE(BDSQ_45308_P_SendCommand(pDevImpl->hHab, buf, 3));

   done:
   BDBG_LEAVE(BDSQ_45308_P_SetVoltage);
   return retCode;
}


/******************************************************************************
 BDSQ_45308_P_GetVoltage
******************************************************************************/
BERR_Code BDSQ_45308_P_GetVoltage(
   BDSQ_ChannelHandle h,   /* [in] BDSQ channel handle */
   uint16_t *pVoltage       /* [out] voltage estimation */
)
{
   BDSQ_45308_P_Handle *pDevImpl = (BDSQ_45308_P_Handle *)(h->pDevice->pImpl);
   BERR_Code retCode;
   uint32_t buf[3];

   BDBG_ENTER(BDSQ_45308_P_GetVoltage);

   buf[0] = BHAB_45308_InitHeader(0x0B, h->channel, BHAB_45308_READ, 0);
   BDSQ_45308_CHK_RETCODE(BDSQ_45308_P_SendCommand(pDevImpl->hHab, buf, 3));

   *pVoltage = (uint16_t)(buf[1] & 0xFFFF);

   done:
   BDBG_LEAVE(BDSQ_45308_P_GetVoltage);
   return retCode;
}


/******************************************************************************
 BDSQ_45308_P_EnableVsenseInterrupt
******************************************************************************/
BERR_Code BDSQ_45308_P_EnableVsenseInterrupt(
   BDSQ_ChannelHandle h,   /* [in] BDSQ channel handle */
   bool bEnable            /* [in] true = enable, false = disable */
)
{
   BDSQ_45308_P_Handle *pDevImpl = (BDSQ_45308_P_Handle *)(h->pDevice->pImpl);
   BERR_Code retCode;
   uint32_t buf[3];

   BDBG_ENTER(BDSQ_45308_P_EnableVsenseInterrupt);

   buf[0] = BHAB_45308_InitHeader(0x28, h->channel, 0, 0);
   buf[1] = bEnable;

   /* clear and enable diseqc_vsense irq */
   BDSQ_45308_P_EnableChannelInterrupt(h, true, BHAB_45308_DSEC_IRQ_VSENSE);

   /* send hab command */
   BDSQ_45308_CHK_RETCODE(BDSQ_45308_P_SendCommand(pDevImpl->hHab, buf, 3));

   done:
   BDBG_LEAVE(BDSQ_45308_P_EnableVsenseInterrupt);
   return retCode;
}


/******************************************************************************
 BDSQ_45308_P_Write
******************************************************************************/
BERR_Code BDSQ_45308_P_Write(
   BDSQ_ChannelHandle h,   /* [in] BDSQ channel handle */
   const uint8_t *pBuf,    /* [in] contains data to transmit */
   uint8_t n               /* [in] number of bytes to transmit */
)
{
   BDSQ_45308_P_Handle *pDevImpl = (BDSQ_45308_P_Handle *)(h->pDevice->pImpl);
   BERR_Code retCode;
   uint32_t buf[128], i;

   BDBG_ENTER(BDSQ_45308_P_Write);

   buf[0] = BHAB_45308_InitHeader(0x0D, h->channel, 0, 0);
   buf[1] = n;

   /* copy packet data */
   for (i = 0; i < n; i++)
      buf[2+i] = pBuf[i];

   /* clear and enable diseqc irqs */
   BDSQ_45308_P_EnableChannelInterrupt(h, true, BHAB_45308_DSEC_IRQ_TX_DONE);
   BDSQ_45308_P_EnableChannelInterrupt(h, true, BHAB_45308_DSEC_IRQ_RX);

   /* send hab command */
   BDSQ_45308_CHK_RETCODE(BDSQ_45308_P_SendCommand(pDevImpl->hHab, buf, n+3));

   done:
   BDBG_LEAVE(BDSQ_45308_P_Write);
   return retCode;
}


/******************************************************************************
 BDSQ_45308_P_EnableRx
******************************************************************************/
BERR_Code BDSQ_45308_P_EnableRx(
   BDSQ_ChannelHandle h,   /* [in] BDSQ channel handle */
   bool bEnable            /* [in] enable/disable Rx */
)
{
   BDSQ_45308_P_Handle *pDevImpl = (BDSQ_45308_P_Handle *)(h->pDevice->pImpl);
   BERR_Code retCode;
   uint32_t buf[3];

   BDBG_ENTER(BDSQ_45308_P_EnableRx);

   buf[0] = BHAB_45308_InitHeader(0x08, h->channel, 0, 0);
   buf[1] = bEnable;

   /* clear and enable diseqc_rx irq */
   BDSQ_45308_P_EnableChannelInterrupt(h, true, BHAB_45308_DSEC_IRQ_RX);

   /* send hab command */
   BDSQ_45308_CHK_RETCODE(BDSQ_45308_P_SendCommand(pDevImpl->hHab, buf, 3));

   done:
   BDBG_LEAVE(BDSQ_45308_P_EnableRx);
   return retCode;
}


/******************************************************************************
 BDSQ_45308_P_Read
******************************************************************************/
BERR_Code BDSQ_45308_P_Read(
   BDSQ_ChannelHandle h,  /* [in] BDSQ handle */
   uint8_t     *pBuf,     /* [out] message data */
   uint8_t     nBufMax,   /* [in] maximum number of bytes that can be written into pBuf */
   uint8_t     *n,        /* [out] number of bytes received */
   uint8_t     *nNotRead  /* [out] number of bytes remaining in the rx fifo */
)
{
   BDSQ_45308_P_Handle *pDevImpl = (BDSQ_45308_P_Handle *)(h->pDevice->pImpl);
   BERR_Code retCode;
   uint32_t buf[21], i;
   uint8_t len;

   BDBG_ENTER(BDSQ_45308_P_Read);

   if (nBufMax > BDSQ_MAX_RX_LEN)
      return BERR_INVALID_PARAMETER;

   buf[0] = BHAB_45308_InitHeader(0x0E, h->channel, 0, 0);
   buf[1] = nBufMax;
   BDSQ_45308_CHK_RETCODE(BDSQ_45308_P_SendCommand(pDevImpl->hHab, buf, nBufMax+5));

   len = (uint8_t)(buf[2] & 0xFF);
   *nNotRead = (uint8_t)(buf[3] & 0xFF);

   /* copy data to buffer */
   for (i = 0; i < len; i++)
      pBuf[i] = (uint8_t)(buf[4+i] & 0xFF);
   *n = len;

   done:
   BDBG_LEAVE(BDSQ_45308_P_Read);
   return retCode;
}


/******************************************************************************
 BDSQ_45308_P_SendAcw
******************************************************************************/
BERR_Code BDSQ_45308_P_SendAcw(
   BDSQ_ChannelHandle h, /* [in] BDSQ channel handle */
   uint8_t acw           /* [in] auto control word to send */
)
{
   BSTD_UNUSED(h);
   BSTD_UNUSED(acw);
   return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BDSQ_45308_P_GetStatus
******************************************************************************/
BERR_Code BDSQ_45308_P_GetStatus(
   BDSQ_ChannelHandle h,   /* [in] BDSQ channel handle */
   BDSQ_Status *pStatus    /* [out] status of most recent transaction */
)
{
   BDSQ_45308_P_Handle *pDevImpl = (BDSQ_45308_P_Handle *)(h->pDevice->pImpl);
   BERR_Code retCode;
   uint32_t buf[5];

   BDBG_ENTER(BDSQ_45308_P_GetStatus);

   buf[0] = BHAB_45308_InitHeader(0x27, h->channel, 0, 0);
   BDSQ_45308_CHK_RETCODE(BDSQ_45308_P_SendCommand(pDevImpl->hHab, buf, 5));

   pStatus->status = (BDSQ_SendStatus)buf[1];
   pStatus->bRxExpected = buf[2] ? true : false;
   pStatus->nRxBytes = (uint8_t)(buf[3] & 0xFF);

   done:
   BDBG_LEAVE(BDSQ_45308_P_GetStatus);
   return retCode;
}


/******************************************************************************
 BDSQ_45308_P_SetChannelSettings
******************************************************************************/
BERR_Code BDSQ_45308_P_SetChannelSettings(
   BDSQ_ChannelHandle h,      /* [in] BDSQ channel handle */
   BDSQ_ChannelSettings *pSettings  /* [in] DiSEqC channel settings */
)
{
   BDSQ_45308_P_Handle *pDevImpl = (BDSQ_45308_P_Handle *)(h->pDevice->pImpl);
   BERR_Code retCode;
   uint32_t buf[9];

   BDBG_ENTER(BDSQ_45308_P_SetChannelSettings);

   buf[0] = BHAB_45308_InitHeader(0x26, h->channel, BHAB_45308_WRITE, 0);
   buf[1] = pSettings->bEnvelope ? 1 : 0;
   buf[1] |= pSettings->bToneAlign ? 0x2 : 0;
   buf[1] |= pSettings->bDisableRRTO ? 0x4 : 0;
   buf[1] |= pSettings->bEnableToneburst ? 0x8 : 0;
   buf[1] |= pSettings->bToneburstB ? 0x10 : 0;
   buf[1] |= pSettings->bOverrideFraming ? 0x20 : 0;
   buf[1] |= pSettings->bExpectReply ? 0x40 : 0;
   buf[1] |= pSettings->bAdaptiveRxSlice ? 0x100 : 0;
   BDSQ_45308_CHK_RETCODE(BDSQ_45308_P_SendCommand(pDevImpl->hHab, buf, 9));

   done:
   BDBG_LEAVE(BDSQ_45308_P_SetChannelSettings);
   return retCode;
}


/******************************************************************************
 BDSQ_45308_P_GetChannelSettings
******************************************************************************/
BERR_Code BDSQ_45308_P_GetChannelSettings(
   BDSQ_ChannelHandle h,      /* [in] BDSQ channel handle */
   BDSQ_ChannelSettings *pSettings  /* [in] DiSEqC channel settings */
)
{
   BDSQ_45308_P_Handle *pDevImpl = (BDSQ_45308_P_Handle *)(h->pDevice->pImpl);
   BERR_Code retCode;
   uint32_t buf[9];

   BDBG_ENTER(BDSQ_45308_P_GetChannelSettings);

   buf[0] = BHAB_45308_InitHeader(0x26, h->channel, BHAB_45308_READ, 0);
   BDSQ_45308_CHK_RETCODE(BDSQ_45308_P_SendCommand(pDevImpl->hHab, buf, 9));

   pSettings->bEnvelope = buf[1] & 0x1 ? true : false;
   pSettings->bToneAlign = buf[1] & 0x2 ? true : false;
   pSettings->bDisableRRTO = buf[1] & 0x4 ? true : false;
   pSettings->bEnableToneburst = buf[1] & 0x8 ? true : false;
   pSettings->bToneburstB = buf[1] & 0x10 ? true : false;
   pSettings->bOverrideFraming = buf[1] & 0x20 ? true : false;
   pSettings->bExpectReply = buf[1] & 0x40 ? true : false;
   pSettings->bAdaptiveRxSlice = buf[1] & 0x100 ? true : false;

   done:
   BDBG_LEAVE(BDSQ_45308_P_GetChannelSettings);
   return retCode;
}


/******************************************************************************
 BDSQ_45308_P_SetConfig
******************************************************************************/
BERR_Code BDSQ_45308_P_SetConfig(
   BDSQ_Handle h,    /* [in] BDSQ device handle */
   uint32_t    addr, /* [in] configuration parameter address */
   uint32_t    val   /* [in] configuration parameter value */
)
{
   BSTD_UNUSED(h);
   BSTD_UNUSED(addr);
   BSTD_UNUSED(val);
   return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BDSQ_45308_P_GetConfig
******************************************************************************/
BERR_Code BDSQ_45308_P_GetConfig(
   BDSQ_Handle h,    /* [in] BDSQ device handle */
   uint32_t    addr, /* [in] configuration parameter address */
   uint32_t    *pVal /* [out] value of the configuration parameter */
)
{
   BSTD_UNUSED(h);
   BSTD_UNUSED(addr);
   BSTD_UNUSED(pVal);
   return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BDSQ_45308_P_SetChannelConfig
******************************************************************************/
BERR_Code BDSQ_45308_P_SetChannelConfig(
   BDSQ_ChannelHandle h,    /* [in] BDSQ channel handle */
   uint32_t           addr, /* [in] configuration parameter address */
   uint32_t           val   /* [in] configuration parameter value */
)
{
   BDSQ_45308_P_Handle *pDevImpl = (BDSQ_45308_P_Handle *)(h->pDevice->pImpl);
   BERR_Code retCode;
   uint32_t hab[4];

   BDBG_ENTER(BDSQ_45308_P_SetChannelConfig);

   if (addr >= BDSQ_45308_CHAN_CONFIG_MAX)
      return BERR_INVALID_PARAMETER;

   hab[0] = BHAB_45308_InitHeader(0x05, h->channel, BHAB_45308_WRITE, BHAB_45308_MODULE_DSQ);
   hab[1] = addr;
   hab[2] = val;
   BDSQ_45308_CHK_RETCODE(BDSQ_45308_P_SendCommand(pDevImpl->hHab, hab, 4));

   done:
   BDBG_LEAVE(BDSQ_45308_P_SetChannelConfig);
   return retCode;
}


/******************************************************************************
 BDSQ_45308_P_GetChannelConfig
******************************************************************************/
BERR_Code BDSQ_45308_P_GetChannelConfig(
   BDSQ_ChannelHandle h,    /* [in] BDSQ channel handle */
   uint32_t           addr, /* [in] configuration parameter address */
   uint32_t           *pVal /* [out] value of the configuration parameter */
)
{
   BDSQ_45308_P_Handle *pDevImpl = (BDSQ_45308_P_Handle *)(h->pDevice->pImpl);
   BERR_Code retCode;
   uint32_t hab[4];

   BDBG_ENTER(BDSQ_45308_P_GetChannelConfig);

   if (addr >= BDSQ_45308_CHAN_CONFIG_MAX)
      return BERR_INVALID_PARAMETER;

   hab[0] = BHAB_45308_InitHeader(0x05, h->channel, BHAB_45308_READ, BHAB_45308_MODULE_DSQ);
   hab[1] = addr;
   BDSQ_45308_CHK_RETCODE(BDSQ_45308_P_SendCommand(pDevImpl->hHab, hab, 4));

   *pVal = hab[2];

   done:
   BDBG_LEAVE(BDSQ_45308_P_GetChannelConfig);
   return retCode;
}


/******************************************************************************
 BDSQ_45308_P_GetTxEventHandle
******************************************************************************/
BERR_Code BDSQ_45308_P_GetTxEventHandle(
   BDSQ_ChannelHandle h,       /* [in] BDSQ channel handle */
   BKNI_EventHandle   *hEvent  /* [out] DiSEqC tx event handle */
)
{
   BDBG_ENTER(BDSQ_45308_P_GetTxEventHandle);

   *hEvent = ((BDSQ_45308_P_ChannelHandle *)(h->pImpl))->hTxEvent;

   BDBG_LEAVE(BDSQ_45308_P_GetTxEventHandle);
   return BERR_SUCCESS;
}


/******************************************************************************
 BDSQ_45308_P_GetRxEventHandle
******************************************************************************/
BERR_Code BDSQ_45308_P_GetRxEventHandle(
   BDSQ_ChannelHandle h,       /* [in] BDSQ channel handle */
   BKNI_EventHandle   *hEvent  /* [out] DiSEqC rx event handle */
)
{
   BDBG_ENTER(BDSQ_45308_P_GetRxEventHandle);

   *hEvent = ((BDSQ_45308_P_ChannelHandle *)(h->pImpl))->hRxEvent;

   BDBG_LEAVE(BDSQ_45308_P_GetRxEventHandle);
   return BERR_SUCCESS;
}


/******************************************************************************
 BDSQ_45308_P_GetVsenseEventHandle
******************************************************************************/
BERR_Code BDSQ_45308_P_GetVsenseEventHandle(
   BDSQ_ChannelHandle h,             /* [in] BDSQ channel handle */
   BKNI_EventHandle *hVsenseEvent    /* [out] DiSEqC vsense event handle */
)
{
   BDBG_ENTER(BDSQ_45308_P_GetVsenseEventHandle);

   *hVsenseEvent = ((BDSQ_45308_P_ChannelHandle *)(h->pImpl))->hVsenseEvent;

   BDBG_LEAVE(BDSQ_45308_P_GetVsenseEventHandle);
   return BERR_SUCCESS;
}


/******************************************************************************
 BDSQ_45308_P_InterruptCallback()
******************************************************************************/
BERR_Code BDSQ_45308_P_InterruptCallback(void *pParm1, int parm2)
{
   BHAB_45308_IrqStatus *pParams = (BHAB_45308_IrqStatus *)pParm1;
   BDSQ_Handle h = (BDSQ_Handle)(pParams->pParm1);
   BDSQ_45308_P_Handle *pImpl = (BDSQ_45308_P_Handle *)(h->pImpl);
   uint32_t fstatus0 = pParams->status0;
   uint32_t fstatus1 = pParams->status1;
   uint32_t channel, val32;
   BDSQ_45308_P_ChannelHandle *pChn;

   BSTD_UNUSED(parm2);
   BSTD_UNUSED(pImpl);
   BSTD_UNUSED(fstatus1);

   BDSQ_DEBUG(BDBG_MSG(("BDSQ_45308_P_InterruptCallback(): fstatus0=0x%X, fstatus1=0x%X", fstatus0, fstatus1)));

   if (fstatus0)
   {
      for (channel = 0; channel < h->totalChannels; channel++)
      {
         if(!h->pChannels[channel])
            continue;

         pChn = (BDSQ_45308_P_ChannelHandle *)(h->pChannels[channel]->pImpl);
         val32 = (fstatus0 >> (channel*3 + 10)) & 0x7;
         if (val32 & BHAB_45308_DSEC_IRQ_TX_DONE)
         {
            BDSQ_DEBUG(BDBG_MSG(("DSQ%d: tx event", channel)));
            BKNI_SetEvent(pChn->hTxEvent);
         }
         if (val32 & BHAB_45308_DSEC_IRQ_RX)
         {
            BDSQ_DEBUG(BDBG_MSG(("DSQ%d: rx event", channel)));
            BKNI_SetEvent(pChn->hRxEvent);
         }
         if (val32 & BHAB_45308_DSEC_IRQ_VSENSE)
         {
            BDSQ_DEBUG(BDBG_MSG(("DSQ%d: vsense event", channel)));
            BKNI_SetEvent(pChn->hVsenseEvent);
         }
      }
   }

   return BERR_SUCCESS;
}


/******************************************************************************
 BDSQ_45308_P_SendCommand()
******************************************************************************/
BERR_Code BDSQ_45308_P_SendCommand(BHAB_Handle h, uint32_t *pBuf, uint32_t n)
{
   return BHAB_SendHabCommand(h, (uint8_t*)pBuf, (uint16_t)n, (uint8_t*)pBuf, (uint16_t)n, true, true, 0);
}


/******************************************************************************
 BDSQ_45308_P_EnableChannelInterrupt()
******************************************************************************/
BERR_Code BDSQ_45308_P_EnableChannelInterrupt(
   BDSQ_ChannelHandle h, /* [in] BDSQ channel handle */
   bool bEnable,         /* [in] true = enable diseqc interrupts, false = disables diseqc interrupts */
   uint32_t mask         /* [in] specifies which diseqc interrupt(s) to enable/disable */
)
{
   BDSQ_45308_P_Handle *pImpl = (BDSQ_45308_P_Handle *)(h->pDevice->pImpl);
   BERR_Code retCode;
   uint32_t irq_mask, shift;

   shift = (h->channel * 3) + 10;
   irq_mask = (mask & BHAB_45308_DSEC_IRQ_ALL) << shift;

   BDSQ_45308_CHK_RETCODE(BHAB_WriteRegister(pImpl->hHab, BCHP_LEAP_HOST_L2_CLEAR0, &irq_mask));

   if (bEnable)
   {
      BDSQ_45308_CHK_RETCODE(BHAB_WriteRegister(pImpl->hHab, BCHP_LEAP_HOST_L2_MASK_CLEAR0, &irq_mask));
   }
   else
   {
      BDSQ_45308_CHK_RETCODE(BHAB_WriteRegister(pImpl->hHab, BCHP_LEAP_HOST_L2_MASK_SET0, &irq_mask));
   }

   done:
   return retCode;
}
