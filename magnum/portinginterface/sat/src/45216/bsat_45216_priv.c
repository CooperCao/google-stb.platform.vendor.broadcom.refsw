/******************************************************************************
* Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#include "bchp_45216_leap_host_l2.h"
#include "bchp_45216_leap_ctrl.h"
#include "bchp_45216_tm.h"
#include "bhab_45216.h"
#include "bsat.h"
#include "bsat_priv.h"
#include "bsat_45216.h"
#include "bsat_45216_priv.h"


BDBG_MODULE(bsat_45216_priv);

#define BSAT_DEBUG(x) /* x */


/* local functions */
static BERR_Code BSAT_45216_P_InterruptCallback(void *pParm1, int parm2);
static BERR_Code BSAT_45216_P_EnableChannelInterrupt(BSAT_ChannelHandle h, bool bEnable, uint32_t mask);


/******************************************************************************
 BSAT_45216_P_Open()
******************************************************************************/
BERR_Code BSAT_45216_P_Open(BSAT_Handle *h, BCHP_Handle hChip, void *pReg, BINT_Handle hInterrupt, const BSAT_Settings *pSettings)
{
   BERR_Code retCode;
   BSAT_Handle hDev;
   BSAT_45216_P_Handle *hImplDev;
   uint32_t numChannels;
   uint8_t i;
   BHAB_Handle hHab;

   BSTD_UNUSED(hInterrupt);
   BSTD_UNUSED(hChip);

   BDBG_ASSERT(pReg);
   hHab = (BHAB_Handle)pReg;

   /* allocate memory for the handle */
   hDev = (BSAT_Handle)BKNI_Malloc(sizeof(BSAT_P_Handle));
   BDBG_ASSERT(hDev);
   hImplDev = (BSAT_45216_P_Handle *)BKNI_Malloc(sizeof(BSAT_45216_P_Handle));
   BDBG_ASSERT(hImplDev);
   hDev->pImpl = (void*)hImplDev;
   hImplDev->hHab = (BHAB_Handle)pReg;

   retCode = BSAT_45216_P_GetTotalChannels(hDev, &numChannels);
   if (retCode != BERR_SUCCESS)
   {
      BKNI_Free(hImplDev);
      BKNI_Free(hDev);
      return retCode;
   }
   hDev->totalChannels = (uint8_t)numChannels;

   hDev->pChannels = (BSAT_P_ChannelHandle **)BKNI_Malloc(numChannels * sizeof(BSAT_P_ChannelHandle *));
   BDBG_ASSERT(hDev->pChannels);
   retCode = BKNI_CreateEvent(&(hImplDev->hInitDoneEvent));
   BDBG_ASSERT(retCode == BERR_SUCCESS);
   hImplDev->lastFastStatusCmd = 0;
   hImplDev->lastFastStatusCount = 0;

   BKNI_Memcpy((void*)(&(hDev->settings)), (void*)pSettings, sizeof(BSAT_Settings));

   for (i = 0; i < numChannels; i++)
      hDev->pChannels[i] = NULL;

   BHAB_InstallInterruptCallback(hHab, BHAB_DevId_eSAT, BSAT_45216_P_InterruptCallback, (void*)hDev, 0);

   *h = hDev;
   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_45216_P_Close()
******************************************************************************/
BERR_Code BSAT_45216_P_Close(BSAT_Handle h)
{
   BSAT_45216_P_Handle *pDevImpl = (BSAT_45216_P_Handle *)(h->pImpl);
   uint32_t mask;
   BERR_Code retCode;

   BDBG_ASSERT(h);

   mask = BHAB_45216_HIRQ0_SAT_MASK;
   BHAB_CHK_RETCODE(BHAB_WriteRegister(pDevImpl->hHab, BCHP_LEAP_HOST_L2_MASK_SET0, &mask));

   mask = 0xFFFFFFFF;
   BHAB_CHK_RETCODE(BHAB_WriteRegister(pDevImpl->hHab, BCHP_LEAP_HOST_L2_MASK_SET1, &mask));

   retCode = BHAB_UnInstallInterruptCallback(pDevImpl->hHab, BHAB_DevId_eSAT);
   BKNI_DestroyEvent(pDevImpl->hInitDoneEvent);
   BKNI_Free((void*)h->pChannels);
   BKNI_Free((void*)h->pImpl);
   BKNI_Free((void*)h);
   h = NULL;

   done:
   return retCode;
}


/******************************************************************************
 BSAT_45216_P_GetTotalChannels()
******************************************************************************/
BERR_Code BSAT_45216_P_GetTotalChannels(BSAT_Handle h, uint32_t *totalChannels)
{
   BSAT_45216_P_Handle *pDevImpl = (BSAT_45216_P_Handle *)(h->pImpl);
   uint32_t product_id, chip_id, n, retries;
   BERR_Code retCode;

   for (retries = 0; retries < 10; retries++)
   {
      retCode = BHAB_ReadRegister(pDevImpl->hHab,  BCHP_TM_PRODUCT_ID, &product_id);
      if (retCode == BERR_SUCCESS)
         break;
   }

   if (retCode == BERR_SUCCESS)
   {
      chip_id = product_id >> 8;
      n = chip_id & 0xFF;
      if (n == 0x16)
         *totalChannels = 16;
      else if (n == 0x04)
         *totalChannels = 4;
      else if (n == 0x06)
         *totalChannels = 6;
      else /* assume 45208 if product_id=0 */
         *totalChannels = 8;
   }

   return retCode;
}


/******************************************************************************
 BSAT_45216_P_OpenChannel()
******************************************************************************/
BERR_Code BSAT_45216_P_OpenChannel(BSAT_Handle h, BSAT_ChannelHandle *pChannelHandle, uint32_t chan, const BSAT_ChannelSettings *pSettings)
{
   BERR_Code retCode;
   BSAT_ChannelSettings cs;
   BSAT_P_ChannelHandle *ch;
   BSAT_45216_P_ChannelHandle *chImpl;

   BDBG_ENTER(BSAT_45216_P_OpenChannel);
   BDBG_ASSERT(h);
   BDBG_ASSERT(h->totalChannels <= BSAT_45216_MAX_CHANNELS);
   BDBG_ASSERT(chan < h->totalChannels);

   if (pSettings == NULL)
      BSAT_45216_GetChannelDefaultSettings(h, chan, &cs);
   else
      cs = *pSettings;

   /* allocate memory for the channel handle */
   ch = (BSAT_P_ChannelHandle *)BKNI_Malloc(sizeof(BSAT_P_ChannelHandle));
   BDBG_ASSERT(ch);
   BKNI_Memcpy((void*)(&ch->settings), (void*)&cs, sizeof(BSAT_ChannelSettings));
   ch->channel = (uint8_t)chan;
   ch->pDevice = h;
   h->pChannels[chan] = ch;

   chImpl = (BSAT_45216_P_ChannelHandle *)BKNI_Malloc(sizeof(BSAT_45216_P_ChannelHandle));
   BDBG_ASSERT(chImpl);

   chImpl->lastMode = BSAT_Mode_eUnknown;
   retCode = BKNI_CreateEvent(&(chImpl->hLockChangeEvent));
   if (retCode != BERR_SUCCESS)
      goto done_0;
   retCode = BKNI_CreateEvent(&(chImpl->hSignalNotificationEvent));
   if (retCode != BERR_SUCCESS)
      goto done_1;
   retCode = BKNI_CreateEvent(&(chImpl->hAcqDoneEvent));
   if (retCode != BERR_SUCCESS)
      goto done_2;
   retCode = BKNI_CreateEvent(&(chImpl->hReadyEvent));
   if (retCode != BERR_SUCCESS)
   {
      BKNI_DestroyEvent(chImpl->hAcqDoneEvent);

      done_2:
      BKNI_DestroyEvent(chImpl->hSignalNotificationEvent);

      done_1:
      BKNI_DestroyEvent(chImpl->hLockChangeEvent);

      done_0:
      BKNI_Free((void*)chImpl);
      BKNI_Free((void*)ch);

      *pChannelHandle = NULL;
      goto done;
   }
   else
   {
      ch->pImpl = (void*)chImpl;
      *pChannelHandle = ch;
   }

   done:
   BDBG_LEAVE(BSAT_45216_P_OpenChannel);
   return retCode;
}


/******************************************************************************
 BSAT_45216_P_CloseChannel()
******************************************************************************/
BERR_Code BSAT_45216_P_CloseChannel(BSAT_ChannelHandle h)
{
   BSAT_45216_P_ChannelHandle *chImpl;

   BDBG_ENTER(BSAT_45216_P_CloseChannel);

   chImpl = (BSAT_45216_P_ChannelHandle *)(h->pImpl);
   BKNI_DestroyEvent(chImpl->hLockChangeEvent);
   BKNI_DestroyEvent(chImpl->hAcqDoneEvent);
   BKNI_DestroyEvent(chImpl->hSignalNotificationEvent);
   BKNI_DestroyEvent(chImpl->hReadyEvent);
   BKNI_Free((void*)chImpl);
   BKNI_Free((void*)h);
   h = NULL;

   BDBG_LEAVE(BSAT_45216_P_CloseChannel);
   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_45216_P_GetDevice()
******************************************************************************/
BERR_Code BSAT_45216_P_GetDevice(BSAT_ChannelHandle h, BSAT_Handle *pDev)
{
   *pDev = h->pDevice;
   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_45216_P_GetVersionInfo()
******************************************************************************/
BERR_Code BSAT_45216_P_GetVersionInfo(BSAT_Handle h, BFEC_VersionInfo *pVersionInfo)
{
   BERR_Code retCode;
   uint32_t hab[6];
   BSAT_45216_P_Handle *pImpl = (BSAT_45216_P_Handle *)(h->pImpl);

   BDBG_ENTER(BSAT_45216_P_GetVersionInfo);

   hab[0] = BHAB_45216_InitHeader(0x2B, 0, 0, BHAB_45216_MODULE_SAT);
   hab[1] = hab[2] = hab[3] = hab[4] = 0;
   BSAT_45216_CHK_RETCODE(BSAT_45216_P_SendCommand(pImpl->hHab, hab, 6));

   pVersionInfo->majorVersion = hab[1];
   pVersionInfo->minorVersion = hab[2];
   pVersionInfo->buildType = hab[3];
   pVersionInfo->buildId = hab[4];

   done:
   BDBG_LEAVE(BSAT_45216_P_GetVersionInfo);
   return retCode;
}


/******************************************************************************
 BSAT_45216_P_Reset()
******************************************************************************/
BERR_Code BSAT_45216_P_Reset(BSAT_Handle h)
{
   BERR_Code retCode;
   uint32_t hab[2], irq_mask, i, n;
   BSAT_45216_P_Handle *pImpl = (BSAT_45216_P_Handle *)(h->pImpl);

   BDBG_ENTER(BSAT_45216_P_Reset);

   BSAT_45216_CHK_RETCODE(BSAT_45216_P_GetTotalChannels(h, &n));

   /* disable the channel interrupts */
   for (i = 0; i < n; i++)
   {
      if (h->pChannels[i])
      {
         BSAT_45216_CHK_RETCODE(BSAT_45216_P_EnableChannelInterrupt(h->pChannels[i], false, BHAB_45216_CHAN_IRQ_ALL));
      }
   }

   /* enable SAT reset done interrupt */
   irq_mask = BHAB_45216_HIRQ0_SAT_INIT_DONE;
   BSAT_45216_CHK_RETCODE(BHAB_WriteRegister(pImpl->hHab, BCHP_LEAP_HOST_L2_CLEAR0, &irq_mask));
   BSAT_45216_CHK_RETCODE(BHAB_WriteRegister(pImpl->hHab, BCHP_LEAP_HOST_L2_MASK_CLEAR0, &irq_mask));

   hab[0] = BHAB_45216_InitHeader(0x09, 0, 0, BHAB_45216_MODULE_SAT);
   BSAT_45216_CHK_RETCODE(BSAT_45216_P_SendCommand(pImpl->hHab, hab, 2));

   done:
   BDBG_LEAVE(BSAT_45216_P_Reset);
   return retCode;
}


/******************************************************************************
 BSAT_45216_P_PowerDownChannel()
******************************************************************************/
BERR_Code BSAT_45216_P_PowerDownChannel(BSAT_ChannelHandle h)
{
   BERR_Code retCode;
   uint32_t hab[3];
   BSAT_45216_P_Handle *pImpl = (BSAT_45216_P_Handle *)(h->pDevice->pImpl);

   BDBG_ENTER(BSAT_45216_P_PowerDownChannel);

   BSAT_45216_CHK_RETCODE(BSAT_45216_P_EnableChannelInterrupt(h, true, BHAB_45216_CHAN_IRQ_MISC));

   hab[0] = BHAB_45216_InitHeader(0x25, h->channel, 0, BHAB_45216_MODULE_SAT);
   hab[1] = 1; /* power down */
   BSAT_45216_CHK_RETCODE(BSAT_45216_P_SendCommand(pImpl->hHab, hab, 3));

   done:
   BDBG_LEAVE(BSAT_45216_P_PowerDownChannel);
   return retCode;
}


/******************************************************************************
 BSAT_45216_P_PowerUpChannel()
******************************************************************************/
BERR_Code BSAT_45216_P_PowerUpChannel(BSAT_ChannelHandle h)
{
   BERR_Code retCode;
   uint32_t hab[3];
   BSAT_45216_P_Handle *pImpl = (BSAT_45216_P_Handle *)(h->pDevice->pImpl);

   BDBG_ENTER(BSAT_45216_P_PowerUpChannel);

   BSAT_45216_CHK_RETCODE(BSAT_45216_P_EnableChannelInterrupt(h, true, BHAB_45216_CHAN_IRQ_MISC));

   hab[0] = BHAB_45216_InitHeader(0x25, h->channel, 0, BHAB_45216_MODULE_SAT);
   hab[1] = 0; /* power up */
   BSAT_45216_CHK_RETCODE(BSAT_45216_P_SendCommand(pImpl->hHab, hab, 3));

   done:
   BDBG_LEAVE(BSAT_45216_P_PowerUpChannel);
   return retCode;
}


/******************************************************************************
 BSAT_45216_P_IsChannelOn()
******************************************************************************/
BERR_Code BSAT_45216_P_IsChannelOn(BSAT_ChannelHandle h, bool *pbOn)
{
   BERR_Code retCode;
   uint32_t hab[3];
   BSAT_45216_P_Handle *pImpl = (BSAT_45216_P_Handle *)(h->pDevice->pImpl);

   BDBG_ENTER(BSAT_45216_P_IsChannelOn);

   hab[0] = BHAB_45216_InitHeader(0x32, h->channel, 0, BHAB_45216_MODULE_SAT);
   hab[1] = 0;
   BSAT_45216_CHK_RETCODE(BSAT_45216_P_SendCommand(pImpl->hHab, hab, 3));

   *pbOn = (hab[1] == 1) ? true : false;

   done:
   BDBG_LEAVE(BSAT_45216_P_IsChannelOn);
   return retCode;
}


/******************************************************************************
 BSAT_45216_P_Acquire()
******************************************************************************/
BERR_Code BSAT_45216_P_Acquire(BSAT_ChannelHandle h, BSAT_AcqSettings *pParams)
{
   BERR_Code retCode;
   uint32_t hab[7];
   BSAT_45216_P_Handle *pImpl = (BSAT_45216_P_Handle *)(h->pDevice->pImpl);
   BSAT_45216_P_ChannelHandle *hChn = (BSAT_45216_P_ChannelHandle *)(h->pImpl);

   BDBG_ENTER(BSAT_45216_P_Acquire);

   if (pParams->mode > BSAT_Mode_eBlindScan)
   {
      BDBG_WRN(("BSAT_45216_P_Acquire(): invalid mode (0x%X)", pParams->mode));
      goto invalid_parameter;
   }

#if 1
   if ((pParams->mode >= BSAT_Mode_eDvbs2_16apsk_2_3) && (pParams->mode <= BSAT_Mode_eDvbs2_32apsk_9_10))
   {
      BDBG_WRN(("BSAT_45216_P_Acquire(): unsupported mode (0x%X)", pParams->mode));
      retCode = BERR_TRACE(BERR_NOT_SUPPORTED);
      goto done;
   }
#endif

   if ((pParams->symbolRate < 1000000) || (pParams->symbolRate > 45000000))
   {
      BDBG_WRN(("BSAT_45216_P_Acquire(): invalid symbol rate (%d)", pParams->symbolRate));
      goto invalid_parameter;
   }

   if ((pParams->freq < 200000000UL) || (pParams->freq > 2450000000UL))
   {
      BDBG_WRN(("BSAT_45216_P_Acquire(): invalid frequency (%d)", pParams->freq));
      goto invalid_parameter;
   }

   if (pParams->adcSelect > 1)
   {
      BDBG_WRN(("BSAT_45216_P_Acquire(): invalid adcSelect (%d)", pParams->adcSelect));
      invalid_parameter:
      retCode = BERR_TRACE(BERR_INVALID_PARAMETER);
      goto done;
   }

   BSAT_45216_CHK_RETCODE(BSAT_45216_P_EnableChannelInterrupt(h, true, BHAB_45216_CHAN_IRQ_ACQ));

   hab[0] = BHAB_45216_InitHeader(0x0F, h->channel, 0, 0);
   hab[1] = pParams->mode;
   hab[2] = pParams->symbolRate;
   hab[3] = pParams->freq;
   hab[4] = pParams->options;
   hab[5] = pParams->adcSelect;
   BSAT_45216_CHK_RETCODE(BSAT_45216_P_SendCommand(pImpl->hHab, hab, 7));

   hChn->lastMode = pParams->mode;

   done:
   BDBG_LEAVE(BSAT_45216_P_Acquire);
   return retCode;
}


/******************************************************************************
 BSAT_45216_P_GetLockStatus()
******************************************************************************/
BERR_Code BSAT_45216_P_GetLockStatus(BSAT_ChannelHandle h, bool *pbLocked, bool *pbAcqDone)
{
   BSAT_45216_P_Handle *pDevImpl = (BSAT_45216_P_Handle *)(h->pDevice->pImpl);
   BERR_Code retCode;
   uint32_t spare0, mask;

   BDBG_ENTER(BSAT_45216_P_GetLockStatus);

   mask = 1 << h->channel;
   BSAT_45216_CHK_RETCODE(BHAB_ReadRegister(pDevImpl->hHab,  BCHP_LEAP_CTRL_SW_SPARE0, &spare0));
   mask = 1 << h->channel;
   if (spare0 & mask)
      *pbLocked = true;
   else
      *pbLocked = false;
   mask = 1 << (16 + h->channel);
   if (spare0 & mask)
      *pbAcqDone = true;
   else
      *pbAcqDone = false;

   done:
   BDBG_LEAVE(BSAT_45216_P_GetLockStatus);
   return retCode;
}


/******************************************************************************
 BSAT_45216_P_GetChannelStatus()
******************************************************************************/
BERR_Code BSAT_45216_P_GetChannelStatus(BSAT_ChannelHandle h, BSAT_ChannelStatus *pStatus)
{
   BSAT_45216_P_Handle *pDevImpl = (BSAT_45216_P_Handle *)(h->pDevice->pImpl);
   BSAT_45216_P_ChannelHandle *pImpl = (BSAT_45216_P_ChannelHandle *)(h->pImpl);
   BERR_Code retCode;
   BSAT_Mode mode = pImpl->lastMode;
   bool bRetry = false;
   uint32_t hab[29];

   BDBG_ENTER(BSAT_45216_P_GetChannelStatus);
   if (BSAT_MODE_IS_DVBS2(mode))
   {
      /* get dvb-s2 status */
      get_dvbs2_status:
      hab[0] = BHAB_45216_InitHeader(0x11, h->channel, 0, 0);
      BSAT_45216_CHK_RETCODE(BSAT_45216_P_SendCommand(pDevImpl->hHab, hab, 29));
      mode = (BSAT_Mode)hab[7];
      if (!bRetry)
      {
         bRetry = true;
         if (BSAT_MODE_IS_LEGACY_QPSK(mode))
            goto get_qpsk_status;
         if (BSAT_MODE_IS_TURBO(mode))
            goto get_turbo_status;
      }
   }
   else if (BSAT_MODE_IS_TURBO(mode))
   {
      /* get turbo status */
      get_turbo_status:
      hab[0] = BHAB_45216_InitHeader(0x12, h->channel, 0, 0);
      BSAT_45216_CHK_RETCODE(BSAT_45216_P_SendCommand(pDevImpl->hHab, hab, 29));
      mode = (BSAT_Mode)hab[7];
      if (!bRetry)
      {
         bRetry = true;
         if (BSAT_MODE_IS_LEGACY_QPSK(mode))
            goto get_qpsk_status;
         if (BSAT_MODE_IS_DVBS2(mode))
            goto get_dvbs2_status;
      }
   }
   else
   {
      /* get qpsk status */
      get_qpsk_status:
      hab[0] = BHAB_45216_InitHeader(0x10, h->channel, 0, 0);
      BSAT_45216_CHK_RETCODE(BSAT_45216_P_SendCommand(pDevImpl->hHab, hab, 27));
      mode = (BSAT_Mode)hab[7];
      if (!bRetry)
      {
         bRetry = true;
         if (BSAT_MODE_IS_TURBO(mode))
            goto get_turbo_status;
         if (BSAT_MODE_IS_DVBS2(mode))
            goto get_dvbs2_status;
      }
   }

   pStatus->bTunerLocked = hab[1] & 1 ? true : false;
   pStatus->bDemodLocked = hab[1] & 2 ? true : false;
   pStatus->acqSettings.mode = (BSAT_Mode)hab[2];
   pStatus->acqSettings.symbolRate = hab[3];
   pStatus->acqSettings.freq = hab[4];
   pStatus->acqSettings.options = hab[5];
   pStatus->acqSettings.adcSelect = hab[6];
   pStatus->mode = (BSAT_Mode)hab[7];
   pStatus->symbolRateError = (int32_t)hab[8];
   pStatus->carrierError = (int32_t)hab[9];
   pStatus->outputBitrate = hab[10];
   pStatus->mpegCount = hab[11];
   pStatus->mpegErrors = hab[12];
   pStatus->snr = hab[13];
   pStatus->reacqCount = hab[14];
   pStatus->reacqCause = hab[15];
   pStatus->sampleFreq = hab[16];
   pStatus->aciBandwidth = hab[17];
   pStatus->notification = hab[18] & 0x0F;
   pStatus->agc.flags = hab[19];
   pStatus->agc.value[0] = hab[20];
   pStatus->agc.value[1] = hab[21];
   pStatus->agc.value[2] = hab[22];
   pStatus->bSpinv = hab[18] & 0x10 ? true : false;

   if (BSAT_MODE_IS_LEGACY_QPSK(pStatus->mode))
   {
      pStatus->modeStatus.qpsk.bValid = true;
      pStatus->modeStatus.qpsk.rsCorrCount = hab[23];
      pStatus->modeStatus.qpsk.rsUncorrCount = hab[24];
      pStatus->modeStatus.qpsk.preVitErrCount = hab[25];
   }
   else if (BSAT_MODE_IS_DVBS2(pStatus->mode))
   {
      pStatus->modeStatus.dvbs2.bValid = hab[27] & 0x80000000 ? true : false;
      pStatus->modeStatus.dvbs2.totalBlocks = hab[23];
      pStatus->modeStatus.dvbs2.corrBlocks = hab[24];
      pStatus->modeStatus.dvbs2.badBlocks = hab[25];
      pStatus->modeStatus.dvbs2.fecFreq = hab[26];
      pStatus->modeStatus.dvbs2.bPilot = hab[27] & 1 ? true : false;
      pStatus->modeStatus.dvbs2.bShortFrame = hab[27] & 2 ? true : false;
   }
   else if (BSAT_MODE_IS_TURBO(pStatus->mode))
   {
      pStatus->modeStatus.turbo.bValid = hab[27] & 0x80000000 ? true : false;
      pStatus->modeStatus.turbo.totalBlocks = hab[23];
      pStatus->modeStatus.turbo.corrBlocks = hab[24];
      pStatus->modeStatus.turbo.badBlocks = hab[25];
      pStatus->modeStatus.turbo.fecFreq = hab[26];
   }
   else
   {
      BKNI_Memset(&(pStatus->modeStatus.turbo), 0, sizeof(BSAT_TurboStatus));
   }

   done:
   BDBG_LEAVE(BSAT_45216_P_GetChannelStatus);
   return retCode;
}


/******************************************************************************
 BSAT_45216_P_ResetChannelStatus()
******************************************************************************/
BERR_Code BSAT_45216_P_ResetChannelStatus(BSAT_ChannelHandle h)
{
   BERR_Code retCode;
   uint32_t hab[2];
   BSAT_45216_P_Handle *pImpl = (BSAT_45216_P_Handle *)(h->pDevice->pImpl);

   BDBG_ENTER(BSAT_45216_P_ResetChannelStatus);

   hab[0] = BHAB_45216_InitHeader(0x15, h->channel, 0, 0);
   BSAT_45216_CHK_RETCODE(BSAT_45216_P_SendCommand(pImpl->hHab, hab, 2));

   done:
   BDBG_LEAVE(BSAT_45216_P_ResetChannelStatus);
   return retCode;
}


/******************************************************************************
 BSAT_45216_P_GetSoftDecisions()
******************************************************************************/
BERR_Code BSAT_45216_P_GetSoftDecisions(BSAT_ChannelHandle h, uint32_t n, int16_t *pI, int16_t *pQ)
{
   BERR_Code retCode;
   uint32_t hab[128], i;
   BSAT_45216_P_Handle *pImpl = (BSAT_45216_P_Handle *)(h->pDevice->pImpl);

   BDBG_ENTER(BSAT_45216_P_GetSoftDecisions);

   if ((n < 1) || (n > 125))
   {
      BDBG_WRN(("BSAT_45216_P_GetSoftDecisions(): invalid number of I/Q pairs specified (%d)", n));
      retCode = BERR_TRACE(BERR_INVALID_PARAMETER);
      goto done;
   }

   hab[0] = BHAB_45216_InitHeader(0x18, h->channel, 0, 0);
   hab[1] = n;
   BSAT_45216_CHK_RETCODE(BSAT_45216_P_SendCommand(pImpl->hHab, hab, 3+n));

   for (i = 0; i < n; i++)
   {
      pI[i] = (int16_t)(hab[2+i] >> 16);
      pQ[i] = (int16_t)(hab[2+i] & 0xFFFF);
   }

   done:
   BDBG_LEAVE(BSAT_45216_P_GetSoftDecisions);
   return retCode;
}


/******************************************************************************
 BSAT_45216_P_ResetChannel()
******************************************************************************/
BERR_Code BSAT_45216_P_ResetChannel(BSAT_ChannelHandle h, bool bDisableDemod)
{
   BERR_Code retCode;
   uint32_t hab[3];
   BSAT_45216_P_Handle *pImpl = (BSAT_45216_P_Handle *)(h->pDevice->pImpl);

   BDBG_ENTER(BSAT_45216_P_ResetChannel);

   BSAT_45216_CHK_RETCODE(BSAT_45216_P_EnableChannelInterrupt(h, false, BHAB_45216_CHAN_IRQ_ALL));

   hab[0] = BHAB_45216_InitHeader(0x34, h->channel, 0, BHAB_45216_MODULE_SAT);
   hab[1] = bDisableDemod ? 1 : 0;
   BSAT_45216_CHK_RETCODE(BSAT_45216_P_SendCommand(pImpl->hHab, hab, 3));

   done:
   BDBG_LEAVE(BSAT_45216_P_ResetChannel);
   return retCode;
}


/******************************************************************************
 BSAT_45216_P_SetBertSettings()
******************************************************************************/
BERR_Code BSAT_45216_P_SetBertSettings(BSAT_ChannelHandle h, BSAT_BertSettings *pSettings)
{
   BSAT_45216_P_Handle *pDevImpl = (BSAT_45216_P_Handle *)(h->pDevice->pImpl);
   BERR_Code retCode;
   uint32_t hab[3];

   BDBG_ENTER(BSAT_45216_P_SetBertSettings);

   hab[0] = BHAB_45216_InitHeader(0x1B, h->channel, BHAB_45216_WRITE, 0);
   hab[1] = pSettings->bInvert ? 1 : 0;
   if (pSettings->mode == BSAT_BertMode_ePRBS15)
      hab[1] |= 0x2;
   else if (pSettings->mode == BSAT_BertMode_ePRBS31)
      hab[1] |= 0x4;
   BSAT_45216_CHK_RETCODE(BSAT_45216_P_SendCommand(pDevImpl->hHab, hab, 3));

   done:
   BDBG_LEAVE(BSAT_45216_P_SetBertSettings);
   return retCode;
}


/******************************************************************************
 BSAT_45216_P_GetBertSettings()
******************************************************************************/
BERR_Code BSAT_45216_P_GetBertSettings(BSAT_ChannelHandle h, BSAT_BertSettings *pSettings)
{
   BSAT_45216_P_Handle *pDevImpl = (BSAT_45216_P_Handle *)(h->pDevice->pImpl);
   BERR_Code retCode;
   uint32_t hab[3];

   BDBG_ENTER(BSAT_45216_P_GetBertSettings);

   hab[0] = BHAB_45216_InitHeader(0x1B, h->channel, BHAB_45216_READ, 0);
   hab[1] = 0;
   BSAT_45216_CHK_RETCODE(BSAT_45216_P_SendCommand(pDevImpl->hHab, hab, 3));

   pSettings->bInvert = hab[1] & 1 ? true : false;
   pSettings->mode = (BSAT_BertMode)((hab[1] >> 1) & 0x3);

   done:
   BDBG_LEAVE(BSAT_45216_P_GetBertSettings);
   return retCode;
}


/******************************************************************************
 BSAT_45216_P_GetBertStatus()
******************************************************************************/
BERR_Code BSAT_45216_P_GetBertStatus(BSAT_ChannelHandle h, BSAT_BertStatus *pStatus)
{
   BSAT_45216_P_Handle *pDevImpl = (BSAT_45216_P_Handle *)(h->pDevice->pImpl);
   BERR_Code retCode;
   uint32_t hab[7];

   BDBG_ENTER(BSAT_45216_P_GetBertStatus);

   hab[0] = BHAB_45216_InitHeader(0x1A, h->channel, 0, 0);
   BSAT_45216_CHK_RETCODE(BSAT_45216_P_SendCommand(pDevImpl->hHab, hab, 7));

   pStatus->bEnabled = hab[1] & 1 ? true : false;
   pStatus->bLock = hab[1] & 2 ? true : false;
   pStatus->bSyncLost = hab[1] & 4 ? true : false;
   pStatus->bSyncAcquired = hab[1] & 8 ? true : false;
   pStatus->bitCountHi = hab[2];
   pStatus->bitCountLo = hab[3];
   pStatus->errorCountHi = hab[4];
   pStatus->errorCountLo = hab[5];

   done:
   BDBG_LEAVE(BSAT_45216_P_GetBertStatus);
   return retCode;
}


/******************************************************************************
 BSAT_45216_P_SetSearchRange()
******************************************************************************/
BERR_Code BSAT_45216_P_SetSearchRange(BSAT_ChannelHandle h, uint32_t searchRange)
{
   BSAT_45216_P_Handle *pDevImpl = (BSAT_45216_P_Handle *)(h->pDevice->pImpl);
   BERR_Code retCode;
   uint32_t hab[3];

   BDBG_ENTER(BSAT_45216_P_SetSearchRange);

   if ((searchRange < 1000000) || (searchRange > 15000000))
      return BERR_INVALID_PARAMETER;

   hab[0] = BHAB_45216_InitHeader(0x1E, h->channel, BHAB_45216_WRITE, 0);
   hab[1] = searchRange;
   BSAT_45216_CHK_RETCODE(BSAT_45216_P_SendCommand(pDevImpl->hHab, hab, 3));

   done:
   BDBG_LEAVE(BSAT_45216_P_SetSearchRange);
   return retCode;
}


/******************************************************************************
 BSAT_45216_P_GetSearchRange()
******************************************************************************/
BERR_Code BSAT_45216_P_GetSearchRange(BSAT_ChannelHandle h, uint32_t *pSearchRange)
{
   BSAT_45216_P_Handle *pDevImpl = (BSAT_45216_P_Handle *)(h->pDevice->pImpl);
   BERR_Code retCode;
   uint32_t hab[3];

   BDBG_ENTER(BSAT_45216_P_GetSearchRange);

   hab[0] = BHAB_45216_InitHeader(0x1E, h->channel, BHAB_45216_READ, 0);
   hab[1] = 0;
   BSAT_45216_CHK_RETCODE(BSAT_45216_P_SendCommand(pDevImpl->hHab, hab, 3));

   *pSearchRange = hab[1];

   done:
   BDBG_LEAVE(BSAT_45216_P_GetSearchRange);
   return retCode;
}


#ifndef BSAT_EXCLUDE_AFEC
/******************************************************************************
 BSAT_45216_P_SetAmcScramblingSeq()
******************************************************************************/
BERR_Code BSAT_45216_P_SetAmcScramblingSeq(BSAT_ChannelHandle h, BSAT_AmcScramblingSeq *pSeq)
{
   BSAT_45216_P_Handle *pDevImpl = (BSAT_45216_P_Handle *)(h->pDevice->pImpl);
   BERR_Code retCode;
   uint32_t hab[6];

   BDBG_ENTER(BSAT_45216_P_SetAmcScramblingSeq);

   hab[0] = BHAB_45216_InitHeader(0x13, h->channel, 0, 0);
   hab[1] = pSeq->xseed;
   hab[2] = pSeq->plhdrscr1;
   hab[3] = pSeq->plhdrscr2;
   hab[4] = pSeq->plhdrscr3;
   BSAT_45216_CHK_RETCODE(BSAT_45216_P_SendCommand(pDevImpl->hHab, hab, 6));

   done:
   BDBG_LEAVE(BSAT_45216_P_SetAmcScramblingSeq);
   return retCode;
}
#endif


/******************************************************************************
 BSAT_45216_P_SetNetworkSpec()
******************************************************************************/
BERR_Code BSAT_45216_P_SetNetworkSpec(BSAT_Handle h, BSAT_NetworkSpec networkSpec)
{
   BSAT_45216_P_Handle *pDevImpl = (BSAT_45216_P_Handle *)(h->pImpl);
   BERR_Code retCode;
   uint32_t hab[3];

   BDBG_ENTER(BSAT_45216_P_SetNetworkSpec);

   if (networkSpec > BSAT_NetworkSpec_eEuro)
      return BERR_INVALID_PARAMETER;

   hab[0] = BHAB_45216_InitHeader(0x1D, 0, BHAB_45216_WRITE, 0);
   hab[1] = (uint32_t)networkSpec;
   BSAT_45216_CHK_RETCODE(BSAT_45216_P_SendCommand(pDevImpl->hHab, hab, 3));

   done:
   BDBG_LEAVE(BSAT_45216_P_SetNetworkSpec);
   return retCode;
}


/******************************************************************************
 BSAT_45216_P_GetNetworkSpec()
******************************************************************************/
BERR_Code BSAT_45216_P_GetNetworkSpec(BSAT_Handle h, BSAT_NetworkSpec *pNetworkSpec)
{
   BSAT_45216_P_Handle *pDevImpl = (BSAT_45216_P_Handle *)(h->pImpl);
   BERR_Code retCode;
   uint32_t hab[3];

   BDBG_ENTER(BSAT_45216_P_GetNetworkSpec);

   hab[0] = BHAB_45216_InitHeader(0x1D, 0, BHAB_45216_READ, 0);
   hab[1] = 0;
   BSAT_45216_CHK_RETCODE(BSAT_45216_P_SendCommand(pDevImpl->hHab, hab, 3));

   *pNetworkSpec = (BSAT_NetworkSpec)hab[1];

   done:
   BDBG_LEAVE(BSAT_45216_P_GetNetworkSpec);
   return retCode;
}


/******************************************************************************
 BSAT_45216_P_SetOutputTransportSettings()
******************************************************************************/
BERR_Code BSAT_45216_P_SetOutputTransportSettings(BSAT_ChannelHandle h, BSAT_OutputTransportSettings *pSettings)
{
   BSAT_45216_P_Handle *pDevImpl = (BSAT_45216_P_Handle *)(h->pDevice->pImpl);
   BERR_Code retCode;
   uint32_t hab[3];

   BDBG_ENTER(BSAT_45216_P_SetOutputTransportSettings);

   hab[0] = BHAB_45216_InitHeader(0x1C, h->channel, BHAB_45216_WRITE, 0);
   hab[1] = pSettings->bSerial ? 1 : 0;
   if (pSettings->bClkInv)
      hab[1] |= 0x2;
   if (pSettings->bClkSup)
      hab[1] |= 0x4;
   if (pSettings->bVldInv)
      hab[1] |= 0x8;
   if (pSettings->bSyncInv)
      hab[1] |= 0x10;
   if (pSettings->bErrInv)
      hab[1] |= 0x20;
   if (pSettings->bXbert)
      hab[1] |= 0x40;
   if (pSettings->bTei)
      hab[1] |= 0x80;
   if (pSettings->bDelay)
      hab[1] |= 0x100;
   if (pSettings->bSync1)
      hab[1] |= 0x200;
   if (pSettings->bHead4)
      hab[1] |= 0x400;
   if (pSettings->bDelHeader)
      hab[1] |= 0x800;
   if (pSettings->bOpllBypass)
      hab[1] |= 0x1000;
   if (pSettings->bDataInv)
      hab[1] |= 0x8000;
   hab[1] |= ((uint32_t)(pSettings->bchMpegErrorMode) & 0x3) << 13;
   BSAT_45216_CHK_RETCODE(BSAT_45216_P_SendCommand(pDevImpl->hHab, hab, 3));

   done:
   BDBG_LEAVE(BSAT_45216_P_SetOutputTransportSettings);
   return retCode;
}


/******************************************************************************
 BSAT_45216_P_GetOutputTransportSettings()
******************************************************************************/
BERR_Code BSAT_45216_P_GetOutputTransportSettings(BSAT_ChannelHandle h, BSAT_OutputTransportSettings *pSettings)
{
   BSAT_45216_P_Handle *pDevImpl = (BSAT_45216_P_Handle *)(h->pDevice->pImpl);
   BERR_Code retCode;
   uint32_t hab[3];

   BDBG_ENTER(BSAT_45216_P_GetOutputTransportSettings);

   hab[0] = BHAB_45216_InitHeader(0x1C, h->channel, BHAB_45216_READ, 0);
   hab[1] = 0;
   BSAT_45216_CHK_RETCODE(BSAT_45216_P_SendCommand(pDevImpl->hHab, hab, 3));

   pSettings->bSerial = hab[1] & 0x1 ? true : false;
   pSettings->bClkInv = hab[1] & 0x2 ? true : false;
   pSettings->bClkSup = hab[1] & 0x4 ? true : false;
   pSettings->bVldInv = hab[1] & 0x8 ? true : false;
   pSettings->bSyncInv = hab[1] & 0x10 ? true : false;
   pSettings->bErrInv = hab[1] & 0x20 ? true : false;
   pSettings->bXbert = hab[1] & 0x40 ? true : false;
   pSettings->bTei = hab[1] & 0x80 ? true : false;
   pSettings->bDelay = hab[1] & 0x100 ? true : false;
   pSettings->bSync1 = hab[1] & 0x200 ? true : false;
   pSettings->bHead4 = hab[1] & 0x400 ? true : false;
   pSettings->bDelHeader = hab[1] & 0x800 ? true : false;
   pSettings->bOpllBypass = hab[1] & 0x1000 ? true : false;
   pSettings->bDataInv = hab[1] & 0x8000 ? true : false;
   pSettings->bchMpegErrorMode = (BSAT_BchMpegErrorMode)((hab[1] >> 13) & 0x3);

   done:
   BDBG_LEAVE(BSAT_45216_P_GetOutputTransportSettings);
   return retCode;
}


/******************************************************************************
 BSAT_45216_P_GetInitDoneEventHandle()
******************************************************************************/
BERR_Code BSAT_45216_P_GetInitDoneEventHandle(BSAT_Handle h, BKNI_EventHandle *hEvent)
{
   BDBG_ENTER(BSAT_45216_P_GetInitDoneEventHandle);

   *hEvent = ((BSAT_45216_P_Handle *)(h->pImpl))->hInitDoneEvent;

   BDBG_LEAVE(BSAT_45216_P_GetInitDoneEventHandle);
   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_45216_P_GetLockStateChangeEventHandle()
******************************************************************************/
BERR_Code BSAT_45216_P_GetLockStateChangeEventHandle(BSAT_ChannelHandle h, BKNI_EventHandle *hEvent)
{
   BDBG_ENTER(BSAT_45216_P_GetLockStateChangeEventHandle);

   *hEvent = ((BSAT_45216_P_ChannelHandle *)(h->pImpl))->hLockChangeEvent;

   BDBG_LEAVE(BSAT_45216_P_GetLockStateChangeEventHandle);
   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_45216_P_GetAcqDoneEventHandle()
******************************************************************************/
BERR_Code BSAT_45216_P_GetAcqDoneEventHandle(BSAT_ChannelHandle h, BKNI_EventHandle *hEvent)
{
   BDBG_ENTER(BSAT_45216_P_GetAcqDoneEventHandle);

   *hEvent = ((BSAT_45216_P_ChannelHandle *)(h->pImpl))->hAcqDoneEvent;

   BDBG_LEAVE(BSAT_45216_P_GetAcqDoneEventHandle);
   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_45216_P_GetSignalNotificationEventHandle()
******************************************************************************/
BERR_Code BSAT_45216_P_GetSignalNotificationEventHandle(BSAT_ChannelHandle h, BKNI_EventHandle *hEvent)
{
   BDBG_ENTER(BSAT_45216_P_GetSignalNotificationEventHandle);

   *hEvent = ((BSAT_45216_P_ChannelHandle *)(h->pImpl))->hSignalNotificationEvent;

   BDBG_LEAVE(BSAT_45216_P_GetSignalNotificationEventHandle);
   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_45216_P_GetReadyEventHandle()
******************************************************************************/
BERR_Code BSAT_45216_P_GetReadyEventHandle(BSAT_ChannelHandle h, BKNI_EventHandle *hEvent)
{
   BDBG_ENTER(BSAT_45216_P_GetReadyEventHandle);

   *hEvent = ((BSAT_45216_P_ChannelHandle *)(h->pImpl))->hReadyEvent;

   BDBG_LEAVE(BSAT_45216_P_GetReadyEventHandle);
   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_45216_P_StartToneDetect()
******************************************************************************/
BERR_Code BSAT_45216_P_StartToneDetect(BSAT_ChannelHandle h, uint32_t freq, uint32_t adcSelect)
{
   BERR_Code retCode;
   uint32_t hab[4];
   BSAT_45216_P_Handle *pImpl = (BSAT_45216_P_Handle *)(h->pDevice->pImpl);
   BSAT_45216_P_ChannelHandle *hChn = (BSAT_45216_P_ChannelHandle *)(h->pImpl);

   BDBG_ENTER(BSAT_45216_P_StartToneDetect);

   if ((freq < 250000000UL) || (freq > 2300000000UL))
   {
      BDBG_WRN(("BSAT_45216_P_StartToneDetect(): invalid frequency (%d)", freq));
      goto invalid_parameter;
   }

   if (adcSelect > 3)
   {
      BDBG_WRN(("BSAT_45216_P_StartToneDetect(): invalid adcSelect (%d)", adcSelect));
      invalid_parameter:
      retCode = BERR_TRACE(BERR_INVALID_PARAMETER);
      goto done;
   }

   BSAT_45216_CHK_RETCODE(BSAT_45216_P_EnableChannelInterrupt(h, true, BHAB_45216_CHAN_IRQ_ACQ_DONE));

   hab[0] = BHAB_45216_InitHeader(0x29, h->channel, 0, 0);
   hab[1] = freq;
   hab[2] = adcSelect;
   BSAT_45216_CHK_RETCODE(BSAT_45216_P_SendCommand(pImpl->hHab, hab, 4));

   hChn->lastMode = BSAT_Mode_eUnknown;

   done:
   BDBG_LEAVE(BSAT_45216_P_StartToneDetect);
   return retCode;
}


/******************************************************************************
 BSAT_45216_P_GetToneDetectStatus()
******************************************************************************/
BERR_Code BSAT_45216_P_GetToneDetectStatus(BSAT_ChannelHandle h, BSAT_ToneDetectStatus *pStatus)
{
   BSAT_45216_P_Handle *pDevImpl = (BSAT_45216_P_Handle *)(h->pDevice->pImpl);
   BERR_Code retCode;
   uint32_t hab[7];

   BDBG_ENTER(BSAT_45216_P_GetToneDetectStatus);

   hab[0] = BHAB_45216_InitHeader(0x2A, h->channel, 0, 0);
   BSAT_45216_CHK_RETCODE(BSAT_45216_P_SendCommand(pDevImpl->hHab, hab, 7));

   pStatus->bEnabled = hab[1] & 1 ? true : false;
   pStatus->bScanDone = hab[1] & 2 ? true : false;
   pStatus->status = hab[2];
   pStatus->tunerFreq = hab[3];
   pStatus->peakPower = hab[4];
   pStatus->peakBin = hab[5];

   done:
   BDBG_LEAVE(BSAT_45216_P_GetToneDetectStatus);
   return retCode;
}


/******************************************************************************
 BSAT_45216_P_StartSymbolRateScan()
******************************************************************************/
BERR_Code BSAT_45216_P_StartSymbolRateScan(BSAT_ChannelHandle h, uint32_t freq, uint32_t minSymbolRate, uint32_t maxSymbolRate, uint32_t adcSelect)
{
   BERR_Code retCode;
   uint32_t hab[6];
   BSAT_45216_P_Handle *pImpl = (BSAT_45216_P_Handle *)(h->pDevice->pImpl);
   BSAT_45216_P_ChannelHandle *hChn = (BSAT_45216_P_ChannelHandle *)(h->pImpl);

   BDBG_ENTER(BSAT_45216_P_StartSymbolRateScan);

   if ((minSymbolRate < 1000000) || (minSymbolRate > 45000000))
   {
      BDBG_WRN(("BSAT_45216_P_StartSymbolRateScan(): invalid symbol rate (%d)", minSymbolRate));
      goto invalid_parameter;
   }

   if ((maxSymbolRate < 1000000) || (maxSymbolRate > 45000000))
   {
      BDBG_WRN(("BSAT_45216_P_StartSymbolRateScan(): invalid symbol rate (%d)", maxSymbolRate));
      goto invalid_parameter;
   }

   if (maxSymbolRate <= minSymbolRate)
   {
      BDBG_WRN(("BSAT_45216_P_StartSymbolRateScan(): maxSymbolRate should not be <= minSymbolRate"));
      goto invalid_parameter;
   }

   if ((freq < 250000000UL) || (freq > 2300000000UL))
   {
      BDBG_WRN(("BSAT_45216_P_StartSymbolRateScan(): invalid frequency (%d)", freq));
      goto invalid_parameter;
   }

   if (adcSelect > 3)
   {
      BDBG_WRN(("BSAT_45216_P_StartSymbolRateScan(): invalid adcSelect (%d)", adcSelect));
      invalid_parameter:
      retCode = BERR_TRACE(BERR_INVALID_PARAMETER);
      goto done;
   }

   BSAT_45216_CHK_RETCODE(BSAT_45216_P_EnableChannelInterrupt(h, true, BHAB_45216_CHAN_IRQ_ACQ_DONE));

   hab[0] = BHAB_45216_InitHeader(0x2C, h->channel, 0, 0);
   hab[1] = freq;
   hab[2] = minSymbolRate;
   hab[3] = maxSymbolRate;
   hab[4] = adcSelect;
   BSAT_45216_CHK_RETCODE(BSAT_45216_P_SendCommand(pImpl->hHab, hab, 6));

   hChn->lastMode = BSAT_Mode_eUnknown;

   done:
   BDBG_LEAVE(BSAT_45216_P_StartSymbolRateScan);
   return retCode;
}


/******************************************************************************
 BSAT_45216_P_GetSymbolRateScanStatus()
******************************************************************************/
BERR_Code BSAT_45216_P_GetSymbolRateScanStatus(BSAT_ChannelHandle h, BSAT_SymbolRateScanStatus *pStatus)
{
   BSAT_45216_P_Handle *pDevImpl = (BSAT_45216_P_Handle *)(h->pDevice->pImpl);
   BERR_Code retCode;
   uint32_t hab[9];

   BDBG_ENTER(BSAT_45216_P_GetSymbolRateScanStatus);

   hab[0] = BHAB_45216_InitHeader(0x2D, h->channel, 0, 0);
   BSAT_45216_CHK_RETCODE(BSAT_45216_P_SendCommand(pDevImpl->hHab, hab, 9));

   pStatus->bEnabled = hab[1] & 1 ? true : false;
   pStatus->bScanDone = hab[1] & 2 ? true : false;
   pStatus->status = hab[2];
   pStatus->tunerFreq = hab[3];
   pStatus->minSymbolRate = hab[4];
   pStatus->maxSymbolRate = hab[5];
   pStatus->peakPower = hab[6];
   pStatus->symbolRate = hab[7];

   done:
   BDBG_LEAVE(BSAT_45216_P_GetSymbolRateScanStatus);
   return retCode;
}


/******************************************************************************
 BSAT_45216_P_SetConfig()
******************************************************************************/
BERR_Code BSAT_45216_P_SetConfig(BSAT_Handle h, uint32_t addr, uint32_t val)
{
   BSAT_45216_P_Handle *pDevImpl = (BSAT_45216_P_Handle *)(h->pImpl);
   BERR_Code retCode;
   uint32_t hab[4];

   BDBG_ENTER(BSAT_45216_P_SetConfig);

   if (addr >= BSAT_45216_CONFIG_MAX)
      return BERR_INVALID_PARAMETER;

   hab[0] = BHAB_45216_InitHeader(0x05, 0, BHAB_45216_WRITE, BHAB_45216_MODULE_LEAP);
   hab[1] = addr;
   hab[2] = val;
   BSAT_45216_CHK_RETCODE(BSAT_45216_P_SendCommand(pDevImpl->hHab, hab, 4));

   done:
   BDBG_LEAVE(BSAT_45216_P_SetConfig);
   return retCode;
}


/******************************************************************************
 BSAT_45216_P_GetConfig()
******************************************************************************/
BERR_Code BSAT_45216_P_GetConfig(BSAT_Handle h, uint32_t addr, uint32_t *pVal)
{
   BSAT_45216_P_Handle *pDevImpl = (BSAT_45216_P_Handle *)(h->pImpl);
   BERR_Code retCode;
   uint32_t hab[4];

   BDBG_ENTER(BSAT_45216_P_GetConfig);

   if (addr >= BSAT_45216_CONFIG_MAX)
      return BERR_INVALID_PARAMETER;

   hab[0] = BHAB_45216_InitHeader(0x05, 0, BHAB_45216_READ, BHAB_45216_MODULE_LEAP);
   hab[1] = addr;
   BSAT_45216_CHK_RETCODE(BSAT_45216_P_SendCommand(pDevImpl->hHab, hab, 4));

   *pVal = hab[2];

   done:
   BDBG_LEAVE(BSAT_45216_P_GetConfig);
   return retCode;
}


/******************************************************************************
 BSAT_45216_P_SetChannelConfig()
******************************************************************************/
BERR_Code BSAT_45216_P_SetChannelConfig(BSAT_ChannelHandle h, uint32_t addr, uint32_t val)
{
   BSAT_45216_P_Handle *pDevImpl = (BSAT_45216_P_Handle *)(h->pDevice->pImpl);
   BERR_Code retCode;
   uint32_t hab[4];

   BDBG_ENTER(BSAT_45216_P_SetChannelConfig);

   if (addr >= BSAT_45216_CHAN_CONFIG_MAX)
      return BERR_INVALID_PARAMETER;

   hab[0] = BHAB_45216_InitHeader(0x05, h->channel, BHAB_45216_WRITE, BHAB_45216_MODULE_SAT);
   hab[1] = addr;
   hab[2] = val;
   BSAT_45216_CHK_RETCODE(BSAT_45216_P_SendCommand(pDevImpl->hHab, hab, 4));

   done:
   BDBG_LEAVE(BSAT_45216_P_SetChannelConfig);
   return retCode;
}


/******************************************************************************
 BSAT_45216_P_GetChannelConfig()
******************************************************************************/
BERR_Code BSAT_45216_P_GetChannelConfig(BSAT_ChannelHandle h, uint32_t addr, uint32_t *pVal)
{
   BSAT_45216_P_Handle *pDevImpl = (BSAT_45216_P_Handle *)(h->pDevice->pImpl);
   BERR_Code retCode;
   uint32_t hab[4];

   BDBG_ENTER(BSAT_45216_P_GetChannelConfig);

   if (addr >= BSAT_45216_CHAN_CONFIG_MAX)
      return BERR_INVALID_PARAMETER;

   hab[0] = BHAB_45216_InitHeader(0x05, h->channel, BHAB_45216_READ, BHAB_45216_MODULE_SAT);
   hab[1] = addr;
   BSAT_45216_CHK_RETCODE(BSAT_45216_P_SendCommand(pDevImpl->hHab, hab, 4));

   *pVal = hab[2];

   done:
   BDBG_LEAVE(BSAT_45216_P_GetChannelConfig);
   return retCode;
}


/******************************************************************************
 BSAT_45216_P_SetAcqDoneEventSettings()
******************************************************************************/
BERR_Code BSAT_45216_P_SetAcqDoneEventSettings(BSAT_Handle h, uint8_t reacqs)
{
   BSAT_45216_P_Handle *pDevImpl = (BSAT_45216_P_Handle *)(h->pImpl);
   BERR_Code retCode;
   uint32_t hab[3];

   BDBG_ENTER(BSAT_45216_P_SetAcqDoneEventSettings);



   hab[0] = BHAB_45216_InitHeader(0x31, 0, 0, 0);
   hab[1] = (uint32_t)reacqs;
   BSAT_45216_CHK_RETCODE(BSAT_45216_P_SendCommand(pDevImpl->hHab, hab, 3));

   done:
   BDBG_LEAVE(BSAT_45216_P_SetAcqDoneEventSettings);
   return retCode;
}


/******************************************************************************
 BSAT_45216_P_SetSignalNotificationSettings()
******************************************************************************/
BERR_Code BSAT_45216_P_SetSignalNotificationSettings(BSAT_ChannelHandle h, BSAT_SignalNotificationSettings *pSettings)
{
   BSAT_45216_P_Handle *pDevImpl = (BSAT_45216_P_Handle *)(h->pDevice->pImpl);
   BERR_Code retCode;
   uint32_t hab[5];

   BDBG_ENTER(BSAT_45216_P_SetSignalNotificationSettings);

   hab[0] = BHAB_45216_InitHeader(0x22, h->channel, BHAB_45216_WRITE, 0);
   hab[1] = pSettings->rainFadeThreshold;
   hab[2] = pSettings->rainFadeWindow;
   hab[3] = pSettings->freqDriftThreshold;
   BSAT_45216_CHK_RETCODE(BSAT_45216_P_SendCommand(pDevImpl->hHab, hab, 5));

   done:
   BDBG_LEAVE(BSAT_45216_P_SetSignalNotificationSettings);
   return retCode;
}


/******************************************************************************
 BSAT_45216_P_GetSignalNotificationSettings()
******************************************************************************/
BERR_Code BSAT_45216_P_GetSignalNotificationSettings(BSAT_ChannelHandle h, BSAT_SignalNotificationSettings *pSettings)
{
   BSAT_45216_P_Handle *pDevImpl = (BSAT_45216_P_Handle *)(h->pDevice->pImpl);
   BERR_Code retCode;
   uint32_t hab[5];

   BDBG_ENTER(BSAT_45216_P_GetSignalNotificationSettings);

   hab[0] = BHAB_45216_InitHeader(0x22, h->channel, BHAB_45216_READ, 0);
   BSAT_45216_CHK_RETCODE(BSAT_45216_P_SendCommand(pDevImpl->hHab, hab, 5));

   pSettings->rainFadeThreshold = hab[1];
   pSettings->rainFadeWindow = hab[2];
   pSettings->freqDriftThreshold = hab[3];

   done:
   BDBG_LEAVE(BSAT_45216_P_GetSignalNotificationSettings);
   return retCode;
}


/******************************************************************************
 BSAT_45216_P_SetACIBandwidth()
******************************************************************************/
BERR_Code BSAT_45216_P_SetACIBandwidth(BSAT_ChannelHandle h, uint32_t bw)
{
   BSAT_45216_P_Handle *pDevImpl = (BSAT_45216_P_Handle *)(h->pDevice->pImpl);
   BERR_Code retCode;
   uint32_t hab[3];

   BDBG_ENTER(BSAT_45216_P_SetACIBandwidth);

   if ((bw != 0) && ((bw < 1000000) || (bw > 45000000)))
      return BERR_INVALID_PARAMETER;

   hab[0] = BHAB_45216_InitHeader(0x19, h->channel, 0, 0);
   hab[1] = bw;
   BSAT_45216_CHK_RETCODE(BSAT_45216_P_SendCommand(pDevImpl->hHab, hab, 3));

   done:
   BDBG_LEAVE(BSAT_45216_P_SetACIBandwidth);
   return retCode;
}


/******************************************************************************
 BSAT_45216_P_StartSignalDetect()
******************************************************************************/
BERR_Code BSAT_45216_P_StartSignalDetect(BSAT_ChannelHandle h, uint32_t symbolRate, uint32_t freq, uint32_t adcSelect)
{
   BERR_Code retCode;
   uint32_t hab[5];
   BSAT_45216_P_Handle *pImpl = (BSAT_45216_P_Handle *)(h->pDevice->pImpl);
   BSAT_45216_P_ChannelHandle *hChn = (BSAT_45216_P_ChannelHandle *)(h->pImpl);

   BDBG_ENTER(BSAT_45216_P_StartSignalDetect);

   if ((symbolRate < 1000000) || (symbolRate > 45000000))
   {
      BDBG_WRN(("BSAT_45216_P_StartSignalDetect(): invalid symbol rate (%d)", symbolRate));
      goto invalid_parameter;
   }

   if ((freq < 250000000UL) || (freq > 2300000000UL))
   {
      BDBG_WRN(("BSAT_45216_P_StartSignalDetect(): invalid frequency (%d)", freq));
      goto invalid_parameter;
   }

   if (adcSelect > 3)
   {
      BDBG_WRN(("BSAT_45216_P_StartSignalDetect(): invalid adcSelect (%d)", adcSelect));
      invalid_parameter:
      retCode = BERR_TRACE(BERR_INVALID_PARAMETER);
      goto done;
   }

   BSAT_45216_CHK_RETCODE(BSAT_45216_P_EnableChannelInterrupt(h, true, BHAB_45216_CHAN_IRQ_ACQ_DONE));

   hab[0] = BHAB_45216_InitHeader(0x16, h->channel, 0, 0);
   hab[1] = symbolRate;
   hab[2] = freq;
   hab[3] = adcSelect;
   BSAT_45216_CHK_RETCODE(BSAT_45216_P_SendCommand(pImpl->hHab, hab, 5));

   hChn->lastMode = BSAT_Mode_eUnknown;

   done:
   BDBG_LEAVE(BSAT_45216_P_StartSignalDetect);
   return retCode;
}


/******************************************************************************
 BSAT_45216_P_GetSignalDetectStatus()
******************************************************************************/
BERR_Code BSAT_45216_P_GetSignalDetectStatus(BSAT_ChannelHandle h, BSAT_SignalDetectStatus *pStatus)
{
   BSAT_45216_P_Handle *pDevImpl = (BSAT_45216_P_Handle *)(h->pDevice->pImpl);
   BERR_Code retCode;
   uint32_t hab[3];

   BDBG_ENTER(BSAT_45216_P_GetSignalDetectStatus);

   hab[0] = BHAB_45216_InitHeader(0x17, h->channel, 0, 0);
   BSAT_45216_CHK_RETCODE(BSAT_45216_P_SendCommand(pDevImpl->hHab, hab, 3));

   pStatus->bEnabled = hab[1] & 1 ? true : false;
   pStatus->bDone = hab[1] & 2 ? true : false;
   pStatus->bTimingLoopLocked = hab[1] & 4 ? true : false;

   done:
   BDBG_LEAVE(BSAT_45216_P_GetSignalDetectStatus);
   return retCode;
}


/******************************************************************************
 BSAT_45216_P_GetLegacyQpskAcqSettings()
******************************************************************************/
BERR_Code BSAT_45216_P_GetLegacyQpskAcqSettings(BSAT_ChannelHandle h, BSAT_LegacyQpskAcqSettings *pSettings)
{
   BSAT_45216_P_Handle *pDevImpl = (BSAT_45216_P_Handle *)(h->pDevice->pImpl);
   BERR_Code retCode;
   uint32_t hab[4];

   BDBG_ENTER(BSAT_45216_P_GetLegacyQpskAcqSettings);

   hab[0] = BHAB_45216_InitHeader(0x1F, h->channel, BHAB_45216_READ, 0);
   BSAT_45216_CHK_RETCODE(BSAT_45216_P_SendCommand(pDevImpl->hHab, hab, 4));

   pSettings->dtvScanModes = (uint8_t)(hab[1] & 0xFF);
   pSettings->dvbsScanModes = (uint8_t)((hab[1] >> 8) & 0xFF);
   pSettings->dciiScanModes = (uint8_t)((hab[1] >> 16) & 0xFF);
   pSettings->stuffBytes = (uint8_t)(hab[2] & 0xFF);
   pSettings->bRsDisable = hab[2] & 0x100 ? true : false;

   done:
   BDBG_LEAVE(BSAT_45216_P_GetLegacyQpskAcqSettings);
   return retCode;
}


/******************************************************************************
 BSAT_45216_P_SetLegacyQpskAcqSettings()
******************************************************************************/
BERR_Code BSAT_45216_P_SetLegacyQpskAcqSettings(BSAT_ChannelHandle h, BSAT_LegacyQpskAcqSettings *pSettings)
{
   BSAT_45216_P_Handle *pDevImpl = (BSAT_45216_P_Handle *)(h->pDevice->pImpl);
   BERR_Code retCode;
   uint32_t hab[4];

   BDBG_ENTER(BSAT_45216_P_SetLegacyQpskAcqSettings);

   hab[0] = BHAB_45216_InitHeader(0x1F, h->channel, BHAB_45216_WRITE, 0);
   hab[1] = (uint32_t)pSettings->dtvScanModes;
   hab[1] |= ((uint32_t)pSettings->dvbsScanModes << 8);
   hab[1] |= ((uint32_t)pSettings->dciiScanModes << 16);
   hab[2] = (uint32_t)pSettings->stuffBytes;
   if (pSettings->bRsDisable)
      hab[2] |= 0x100;
   BSAT_45216_CHK_RETCODE(BSAT_45216_P_SendCommand(pDevImpl->hHab, hab, 4));

   done:
   BDBG_LEAVE(BSAT_45216_P_SetLegacyQpskAcqSettings);
   return retCode;
}


/******************************************************************************
 BSAT_45216_P_GetExtAcqSettings()
******************************************************************************/
BERR_Code BSAT_45216_P_GetExtAcqSettings(BSAT_ChannelHandle h, BSAT_ExtAcqSettings *pSettings)
{
   BSAT_45216_P_Handle *pDevImpl = (BSAT_45216_P_Handle *)(h->pDevice->pImpl);
   BERR_Code retCode;
   uint32_t hab[6];

   BDBG_ENTER(BSAT_45216_P_GetExtAcqSettings);

   hab[0] = BHAB_45216_InitHeader(0x24, h->channel, BHAB_45216_READ, 0);
   BSAT_45216_CHK_RETCODE(BSAT_45216_P_SendCommand(pDevImpl->hHab, hab, 6));

   pSettings->bForceUnlockOutsideSearchRange = hab[1] & 0x1 ? true : false;
   pSettings->bBypassDft = hab[1] & 0x2 ? true : false;
   pSettings->bDontRetuneOnReacquire = hab[1] & 0x4 ? true : false;
   pSettings->bPreserveState = hab[1] & 0x8 ? true : false;
   pSettings->bPreserveCommandedTunerFreq = hab[1] & 0x10 ? true : false;
   pSettings->bDisableNotch = hab[1] & 0x20 ? true : false;
   pSettings->bCheckTimingLoop = hab[1] & 0x40 ? true : false;
   pSettings->bOverrideFiltctl = hab[1] & 0x80 ? true : false;
   pSettings->blindScanModes = hab[2] & BSAT_BLIND_SCAN_MODE_MASK;
   pSettings->filtctlOverride = hab[3];
   pSettings->maxReacqs = hab[4];
   pSettings->snrEstMethod = (hab[1] >> 8) & 0x03;

   done:
   BDBG_LEAVE(BSAT_45216_P_GetExtAcqSettings);
   return retCode;
}


/******************************************************************************
 BSAT_45216_P_SetExtAcqSettings()
******************************************************************************/
BERR_Code BSAT_45216_P_SetExtAcqSettings(BSAT_ChannelHandle h, BSAT_ExtAcqSettings *pSettings)
{
   BSAT_45216_P_Handle *pDevImpl = (BSAT_45216_P_Handle *)(h->pDevice->pImpl);
   BERR_Code retCode;
   uint32_t hab[6];

   BDBG_ENTER(BSAT_45216_P_SetExtAcqSettings);

   hab[0] = BHAB_45216_InitHeader(0x24, h->channel, BHAB_45216_WRITE, 0);
   hab[1] = pSettings->bForceUnlockOutsideSearchRange ? 0x01 : 0;
   if (pSettings->bBypassDft)
      hab[1] |= 0x2;
   if (pSettings->bDontRetuneOnReacquire)
      hab[1] |= 0x4;
   if (pSettings->bPreserveState)
      hab[1] |= 0x8;
   if (pSettings->bPreserveCommandedTunerFreq)
      hab[1] |= 0x10;
   if (pSettings->bDisableNotch)
      hab[1] |= 0x20;
   if (pSettings->bCheckTimingLoop)
      hab[1] |= 0x40;
   if (pSettings->bOverrideFiltctl)
      hab[1] |= 0x80;
   hab[1] |= ((pSettings->snrEstMethod & 0x03) << 8);
   hab[2] = (uint32_t)(pSettings->blindScanModes & BSAT_BLIND_SCAN_MODE_MASK);
   hab[3] = pSettings->filtctlOverride;
   hab[4] = pSettings->maxReacqs;
   BSAT_45216_CHK_RETCODE(BSAT_45216_P_SendCommand(pDevImpl->hHab, hab, 6));

   done:
   BDBG_LEAVE(BSAT_45216_P_SetExtAcqSettings);
   return retCode;
}


/******************************************************************************
 BSAT_45216_P_GetDvbs2AcqSettings()
******************************************************************************/
BERR_Code BSAT_45216_P_GetDvbs2AcqSettings(BSAT_ChannelHandle h, BSAT_Dvbs2AcqSettings *pSettings)
{
   BSAT_45216_P_Handle *pDevImpl = (BSAT_45216_P_Handle *)(h->pDevice->pImpl);
   BERR_Code retCode;
   uint32_t hab[4];

   BDBG_ENTER(BSAT_45216_P_GetDvbs2AcqSettings);

   hab[0] = BHAB_45216_InitHeader(0x20, h->channel, BHAB_45216_READ, 0);
   BSAT_45216_CHK_RETCODE(BSAT_45216_P_SendCommand(pDevImpl->hHab, hab, 4));

   pSettings->scanModes = hab[1];
   pSettings->ctl = hab[2];

   done:
   BDBG_LEAVE(BSAT_45216_P_GetDvbs2AcqSettings);
   return retCode;
}


/******************************************************************************
 BSAT_45216_P_SetDvbs2AcqSettings()
******************************************************************************/
BERR_Code BSAT_45216_P_SetDvbs2AcqSettings(BSAT_ChannelHandle h, BSAT_Dvbs2AcqSettings *pSettings)
{
   BSAT_45216_P_Handle *pDevImpl = (BSAT_45216_P_Handle *)(h->pDevice->pImpl);
   BERR_Code retCode;
   uint32_t hab[4];

   BDBG_ENTER(BSAT_45216_P_SetDvbs2AcqSettings);

   hab[0] = BHAB_45216_InitHeader(0x20, h->channel, BHAB_45216_WRITE, 0);
   hab[1] = pSettings->scanModes;
   hab[2] = pSettings->ctl;

   BSAT_45216_CHK_RETCODE(BSAT_45216_P_SendCommand(pDevImpl->hHab, hab, 4));

   done:
   BDBG_LEAVE(BSAT_45216_P_SetDvbs2AcqSettings);
   return retCode;
}


/******************************************************************************
 BSAT_45216_P_GetTurboAcqSettings()
******************************************************************************/
BERR_Code BSAT_45216_P_GetTurboAcqSettings(BSAT_ChannelHandle h, BSAT_TurboAcqSettings *pSettings)
{
   BSAT_45216_P_Handle *pDevImpl = (BSAT_45216_P_Handle *)(h->pDevice->pImpl);
   BERR_Code retCode;
   uint32_t hab[5];

   BDBG_ENTER(BSAT_45216_P_GetTurboAcqSettings);

   hab[0] = BHAB_45216_InitHeader(0x21, h->channel, BHAB_45216_READ, 0);
   BSAT_45216_CHK_RETCODE(BSAT_45216_P_SendCommand(pDevImpl->hHab, hab, 5));

   pSettings->scanModes = hab[1];
   pSettings->ctl = hab[2];
   pSettings->tzsyOverride = hab[3];

   done:
   BDBG_LEAVE(BSAT_45216_P_GetTurboAcqSettings);
   return retCode;
}


/******************************************************************************
 BSAT_45216_P_SetTurboAcqSettings()
******************************************************************************/
BERR_Code BSAT_45216_P_SetTurboAcqSettings(BSAT_ChannelHandle h, BSAT_TurboAcqSettings *pSettings)
{
   BSAT_45216_P_Handle *pDevImpl = (BSAT_45216_P_Handle *)(h->pDevice->pImpl);
   BERR_Code retCode;
   uint32_t hab[5];

   BDBG_ENTER(BSAT_45216_P_SetTurboAcqSettings);

   hab[0] = BHAB_45216_InitHeader(0x21, h->channel, BHAB_45216_WRITE, 0);
   hab[1] = pSettings->scanModes;
   hab[2] = pSettings->ctl;
   hab[3] = pSettings->tzsyOverride;
   BSAT_45216_CHK_RETCODE(BSAT_45216_P_SendCommand(pDevImpl->hHab, hab, 5));

   done:
   BDBG_LEAVE(BSAT_45216_P_SetTurboAcqSettings);
   return retCode;
}


/******************************************************************************
 BSAT_45216_P_SetNotchSettings()
******************************************************************************/
BERR_Code BSAT_45216_P_SetNotchSettings(BSAT_Handle h, uint32_t n, BSAT_NotchSettings *pSettings)
{
   BDBG_ENTER(BSAT_45216_P_SetNotchSettings);

   BSTD_UNUSED(h);
   BSTD_UNUSED(n);
   BSTD_UNUSED(pSettings);

   BDBG_LEAVE(BSAT_45216_P_SetNotchSettings);
   return BSAT_ERR_NOT_IMPLEMENTED;
}


/******************************************************************************
 BSAT_45216_P_GetNotchSettings()
******************************************************************************/
BERR_Code BSAT_45216_P_GetNotchSettings(BSAT_Handle h, uint32_t *pNum, BSAT_NotchSettings *pSettings)
{
   BDBG_ENTER(BSAT_45216_P_GetNotchSettings);

   BSTD_UNUSED(h);
   BSTD_UNUSED(pNum);
   BSTD_UNUSED(pSettings);

   BDBG_LEAVE(BSAT_45216_P_GetNotchSettings);
   return BSAT_ERR_NOT_IMPLEMENTED;
}


/******************************************************************************
 BSAT_45216_P_InterruptCallback()
******************************************************************************/
static BERR_Code BSAT_45216_P_InterruptCallback(void *pParm1, int parm2)
{
   BERR_Code retCode = BERR_SUCCESS;
   BHAB_45216_IrqStatus *pParams = (BHAB_45216_IrqStatus *)pParm1;
   BSAT_Handle h = (BSAT_Handle)(pParams->pParm1);
   BSAT_45216_P_Handle *pImpl = (BSAT_45216_P_Handle *)(h->pImpl);
   uint32_t fstatus0 = pParams->status0;
   uint32_t fstatus1 = pParams->status1;
   uint32_t channel, val32, spare1;
   BSAT_45216_P_ChannelHandle *pChn;

   BSTD_UNUSED(parm2);

   BSAT_DEBUG(BDBG_MSG(("BSAT_45216_P_InterruptCallback(): fstatus0=0x%X, fstatus1=0x%X", fstatus0, fstatus1)));

   if (fstatus0 & BHAB_45216_HIRQ0_SAT_INIT_DONE)
   {
      BKNI_SetEvent(pImpl->hInitDoneEvent);
   }

   for (channel = 0; channel < h->totalChannels; channel++)
   {
      if (h->pChannels[channel] == NULL)
         continue;

      pChn = (BSAT_45216_P_ChannelHandle *)(h->pChannels[channel]->pImpl);
      if (channel == 10)
      {
         if (fstatus1 & BHAB_45216_HIRQ1_SAT10_LOCK_CHANGE)
         {
            BSAT_DEBUG(BDBG_MSG(("SAT%d: set lock change event", channel)));
            BKNI_SetEvent(pChn->hLockChangeEvent);
         }
         if (fstatus1 & BHAB_45216_HIRQ1_SAT10_ACQ_DONE)
         {
            BSAT_DEBUG(BDBG_MSG(("SAT%d: set acq done event", channel)));
            BKNI_SetEvent(pChn->hAcqDoneEvent);
         }
         if (fstatus0 & BHAB_45216_HIRQ0_SAT10_MISC)
            goto decode_misc_irq;
      }
      else
      {
         if (channel < 10)
            val32 = (fstatus1 >> (channel*3)) & 0x07;
         else /* channel > 10 */
            val32 = (fstatus0 >> ((channel-11)*3 + 17)) & 0x07;

         if ((val32 & BHAB_45216_CHAN_IRQ_ALL) == 0)
            continue;

         if (val32 & BHAB_45216_CHAN_IRQ_LOCK_STATUS_CHANGE)
         {
            BSAT_DEBUG(BDBG_MSG(("SAT%d: set lock change event", channel)));
            BKNI_SetEvent(pChn->hLockChangeEvent);
         }
         if (val32 & BHAB_45216_CHAN_IRQ_ACQ_DONE)
         {
            BSAT_DEBUG(BDBG_MSG(("SAT%d: set acq done event", channel)));
            BKNI_SetEvent(pChn->hAcqDoneEvent);
         }
         if (val32 & BHAB_45216_CHAN_IRQ_MISC)
         {
            decode_misc_irq:
            BSAT_45216_CHK_RETCODE(BHAB_ReadRegister(pImpl->hHab,  BCHP_LEAP_CTRL_SW_SPARE1, &spare1));
            spare1 = (spare1 >> (channel * 2)) & 0x3;
            if (spare1 & BHAB_45216_SPARE1_CH0_SIG_NOT)
            {
               BSAT_DEBUG(BDBG_MSG(("SAT%d: set signal notification event", channel)));
               BKNI_SetEvent(pChn->hSignalNotificationEvent);
            }
            if (spare1 & BHAB_45216_SPARE1_CH0_READY)
            {
               BSAT_DEBUG(BDBG_MSG(("SAT%d: set ready event", channel)));
               BKNI_SetEvent(pChn->hReadyEvent);
            }
         }
      }
   }

   done:
   return retCode;
}


/******************************************************************************
 BSAT_45216_P_SendCommand()
******************************************************************************/
BERR_Code BSAT_45216_P_SendCommand(BHAB_Handle h, uint32_t *pBuf, uint32_t n)
{
   return BHAB_SendHabCommand(h, (uint8_t*)pBuf, (uint16_t)n, (uint8_t*)pBuf, (uint16_t)n, true, true, 0);
}


/******************************************************************************
 BSAT_45216_P_EnableChannelInterrupt()
******************************************************************************/
static BERR_Code BSAT_45216_P_EnableChannelInterrupt(
   BSAT_ChannelHandle h, /* [in] BSAT channel handle */
   bool bEnable,         /* [in] true = enable lock interrupts, false = disables lock interrupts */
   uint32_t mask         /* [in] specifies which channel interrupt(s) to enable/disable */
)
{
   BERR_Code retCode = BERR_SUCCESS;
   BSAT_45216_P_Handle *pImpl = (BSAT_45216_P_Handle *)(h->pDevice->pImpl);
   uint32_t irq_mask, shift, clearReg, maskClearReg, maskSetReg;

   if (h->channel == 10)
   {
      if (mask & BHAB_45216_CHAN_IRQ_MISC)
      {
         irq_mask = BHAB_45216_HIRQ0_SAT10_MISC;
         BSAT_45216_CHK_RETCODE(BHAB_WriteRegister(pImpl->hHab, BCHP_LEAP_HOST_L2_CLEAR0, &irq_mask));

         if (bEnable)
         {
            BSAT_45216_CHK_RETCODE(BHAB_WriteRegister(pImpl->hHab, BCHP_LEAP_HOST_L2_MASK_CLEAR0, &irq_mask));
         }
         else
         {
            BSAT_45216_CHK_RETCODE(BHAB_WriteRegister(pImpl->hHab, BCHP_LEAP_HOST_L2_MASK_SET0, &irq_mask));
         }
      }

      irq_mask = ((mask & BHAB_45216_CHAN_IRQ_LOCK_STATUS_CHANGE) ? (1<<30) : 0);
      irq_mask |= ((mask & BHAB_45216_CHAN_IRQ_ACQ_DONE) ? (1<<31) : 0);
      if (irq_mask != 0)
      {
         if (bEnable)
         {
            BSAT_45216_CHK_RETCODE(BHAB_WriteRegister(pImpl->hHab, BCHP_LEAP_HOST_L2_MASK_CLEAR1, &irq_mask));
         }
         else
         {
            BSAT_45216_CHK_RETCODE(BHAB_WriteRegister(pImpl->hHab, BCHP_LEAP_HOST_L2_MASK_SET1, &irq_mask));
         }
      }
   }
   else
   {
      if (h->channel < 10)
      {
         clearReg = BCHP_LEAP_HOST_L2_CLEAR1;
         maskClearReg = BCHP_LEAP_HOST_L2_MASK_CLEAR1;
         maskSetReg = BCHP_LEAP_HOST_L2_MASK_SET1;
         shift = h->channel * 3;
      }
      else /* (h->channel > 10) */
      {
         clearReg = BCHP_LEAP_HOST_L2_CLEAR0;
         maskClearReg = BCHP_LEAP_HOST_L2_MASK_CLEAR0;
         maskSetReg = BCHP_LEAP_HOST_L2_MASK_SET0;
         shift = ((h->channel - 11) * 3) + 17;
      }

      irq_mask = (mask & 0x07) << shift;

      BSAT_45216_CHK_RETCODE(BHAB_WriteRegister(pImpl->hHab, clearReg, &irq_mask));

      if (bEnable)
      {
         BSAT_45216_CHK_RETCODE(BHAB_WriteRegister(pImpl->hHab, maskClearReg, &irq_mask));
      }
      else
      {
         BSAT_45216_CHK_RETCODE(BHAB_WriteRegister(pImpl->hHab, maskSetReg, &irq_mask));
      }
   }

   done:
   return retCode;
}


/******************************************************************************
 BSAT_45216_P_GetTraceInfo()
******************************************************************************/
BERR_Code BSAT_45216_P_GetTraceInfo(BSAT_ChannelHandle h, BSAT_TraceInfo *pInfo)
{
   BSAT_45216_P_Handle *pDevImpl = (BSAT_45216_P_Handle *)(h->pDevice->pImpl);
   BERR_Code retCode;
   uint32_t hab[11], i;

   BDBG_ENTER(BSAT_45216_P_GetTraceInfo);

   hab[0] = BHAB_45216_InitHeader(0x33, h->channel, 0, 0);
   BSAT_45216_CHK_RETCODE(BSAT_45216_P_SendCommand(pDevImpl->hHab, hab, 11));

   for (i = 0; i < 9; i++)
      pInfo->eventTime[i] = hab[i+1];

   done:
   BDBG_LEAVE(BSAT_45216_P_GetTraceInfo);
   return retCode;
}


/******************************************************************************
 BSAT_45216_P_SetExternalBertSettings()
******************************************************************************/
BERR_Code BSAT_45216_P_SetExternalBertSettings(BSAT_Handle h, BSAT_ExternalBertSettings *pSettings)
{
   BSAT_45216_P_Handle *pImpl = (BSAT_45216_P_Handle *)(h->pImpl);
   BERR_Code retCode;
   uint32_t hab[3];

   BDBG_ENTER(BSAT_45216_P_SetExternalBertSettings);

   hab[0] = BHAB_45216_InitHeader(0x37, 0, BHAB_45216_WRITE, 0);
   hab[1] = pSettings->channel;
   if (pSettings->bEnable)
      hab[1] |= 0x80000000;
   if (pSettings->bClkInv)
      hab[1] |= 0x40000000;

   BSAT_45216_CHK_RETCODE(BSAT_45216_P_SendCommand(pImpl->hHab, hab, 3));

   done:
   BDBG_LEAVE(BSAT_45216_P_SetExternalBertSettings);
   return retCode;
}


/******************************************************************************
 BSAT_45216_P_GetExternalBertSettings()
******************************************************************************/
BERR_Code BSAT_45216_P_GetExternalBertSettings(BSAT_Handle h, BSAT_ExternalBertSettings *pSettings)
{
   BSAT_45216_P_Handle *pImpl = (BSAT_45216_P_Handle *)(h->pImpl);
   BERR_Code retCode;
   uint32_t hab[3];

   BDBG_ENTER(BSAT_45216_P_GetExternalBertSettings);

   hab[0] = BHAB_45216_InitHeader(0x37, 0, BHAB_45216_READ, 0);
   BSAT_45216_CHK_RETCODE(BSAT_45216_P_SendCommand(pImpl->hHab, hab, 3));

   pSettings->channel = hab[1] & 0x0F;
   pSettings->bEnable = (hab[1] & 0x80000000) ? true : false;
   pSettings->bClkInv = (hab[1] & 0x40000000) ? true : false;

   done:
   BDBG_LEAVE(BSAT_45216_P_GetExternalBertSettings);
   return retCode;
}


/******************************************************************************
 BSAT_45216_P_StartPsdScan()
******************************************************************************/
BERR_Code BSAT_45216_P_StartPsdScan(BSAT_ChannelHandle h, uint32_t freq, uint32_t adcSelect)
{
   BERR_Code retCode;
   uint32_t hab[4];
   BSAT_45216_P_Handle *pImpl = (BSAT_45216_P_Handle *)(h->pDevice->pImpl);
   BSAT_45216_P_ChannelHandle *hChn = (BSAT_45216_P_ChannelHandle *)(h->pImpl);

   BDBG_ENTER(BSAT_45216_P_StartPsdScan);

   if ((freq < 250000000UL) || (freq > 2300000000UL))
   {
      BDBG_WRN(("BSAT_45216_P_StartPsdScan(): invalid frequency (%d)", freq));
      goto invalid_parameter;
   }

   if (adcSelect > 3)
   {
      BDBG_WRN(("BSAT_45216_P_StartPsdScan(): invalid adcSelect (%d)", adcSelect));
      invalid_parameter:
      retCode = BERR_TRACE(BERR_INVALID_PARAMETER);
      goto done;
   }

   BSAT_45216_CHK_RETCODE(BSAT_45216_P_EnableChannelInterrupt(h, true, BHAB_45216_CHAN_IRQ_ACQ_DONE));

   hab[0] = BHAB_45216_InitHeader(0x39, h->channel, 0, 0);
   hab[1] = freq;
   hab[2] = adcSelect;
   BSAT_45216_CHK_RETCODE(BSAT_45216_P_SendCommand(pImpl->hHab, hab, 4));

   hChn->lastMode = BSAT_Mode_eUnknown;

   done:
   BDBG_LEAVE(BSAT_45216_P_StartPsdScan);
   return retCode;
}


/******************************************************************************
 BSAT_45216_P_GetPsdScanStatus()
******************************************************************************/
BERR_Code BSAT_45216_P_GetPsdScanStatus(BSAT_ChannelHandle h, BSAT_PsdScanStatus *pStatus)
{
   BSAT_45216_P_Handle *pDevImpl = (BSAT_45216_P_Handle *)(h->pDevice->pImpl);
   BERR_Code retCode;
   uint32_t hab[9];

   BDBG_ENTER(BSAT_45216_P_GetPsdScanStatus);

   hab[0] = BHAB_45216_InitHeader(0x3A, h->channel, 0, 0);
   BSAT_45216_CHK_RETCODE(BSAT_45216_P_SendCommand(pDevImpl->hHab, hab, 6));

   pStatus->bEnabled = hab[1] & 1 ? true : false;
   pStatus->bScanDone = hab[1] & 2 ? true : false;
   pStatus->status = hab[2];
   pStatus->tunerFreq = hab[3];
   pStatus->power = hab[4];

   done:
   BDBG_LEAVE(BSAT_45216_P_GetPsdScanStatus);
   return retCode;
}


/******************************************************************************
 BSAT_45216_P_GetFastChannelStatus()
******************************************************************************/
BERR_Code BSAT_45216_P_GetFastChannelStatus(BSAT_ChannelHandle h, BSAT_FastStatusId *pIds, uint8_t n, BSAT_FastChannelStatus *pStatus)
{
#define LEAP_GP_STATUS_CMD    BCHP_LEAP_CTRL_GP51
#define LEAP_GP_STATUS_RESULT BCHP_LEAP_CTRL_GP52
#define LEAP_GP_STATUS0       BCHP_LEAP_CTRL_GP53
#define LEAP_GP_STATUS1       BCHP_LEAP_CTRL_GP54
#define LEAP_GP_STATUS2       BCHP_LEAP_CTRL_GP55
#define LEAP_GP_STATUS3       BCHP_LEAP_CTRL_GP56
#define LEAP_GP_STATUS4       BCHP_LEAP_CTRL_GP57
#define LEAP_GP_STATUS5       BCHP_LEAP_CTRL_GP58
#define LEAP_GP_STATUS6       BCHP_LEAP_CTRL_GP59

   BSAT_45216_P_Handle *pDevImpl = (BSAT_45216_P_Handle *)(h->pDevice->pImpl);
   BERR_Code retCode;
   uint32_t cmd = (h->channel << 28), result, counter, val;
   uint8_t i, statusID;

   BDBG_ENTER(BSAT_45216_P_GetFastChannelStatus);

   if ((n == 0) || (n > 7) || (pIds == NULL) || (pStatus == NULL))
      return BERR_INVALID_PARAMETER;

   for (i = 0; i < n; i++)
   {
      statusID = (uint8_t)pIds[i];
      if (statusID >= BSAT_FastStatusId_max)
         return BERR_INVALID_PARAMETER;
      pStatus->item[i].bValid = false;
      cmd |= ((statusID & 0x0F) << (i*4));
   }

   BSAT_45216_CHK_RETCODE(BHAB_ReadRegister(pDevImpl->hHab, LEAP_GP_STATUS_CMD, &val));
   if (val != cmd)
   {
      write_command:
      /* BDBG_WRN(("writing cmd 0x%X", cmd)); */
      BSAT_45216_CHK_RETCODE(BHAB_WriteRegister(pDevImpl->hHab, LEAP_GP_STATUS_CMD, &cmd));
      pDevImpl->lastFastStatusCmd = cmd;
      BSAT_45216_CHK_RETCODE(BHAB_ReadRegister(pDevImpl->hHab, LEAP_GP_STATUS_RESULT, &result));
      pDevImpl->lastFastStatusCount = result >> 8;
   }

   for (i = 0; i < 3; i++)
   {
      BSAT_45216_CHK_RETCODE(BHAB_ReadRegister(pDevImpl->hHab, LEAP_GP_STATUS_RESULT, &result));
      counter = result >> 8;
      if (counter != pDevImpl->lastFastStatusCount)
      {
         pDevImpl->lastFastStatusCount = counter;
         break;
      }
      BKNI_Sleep(1);
   }
   if (i == 3)
   {
      BSAT_45216_CHK_RETCODE(BHAB_ReadRegister(pDevImpl->hHab, LEAP_GP_STATUS_CMD, &val));
      if (val != pDevImpl->lastFastStatusCmd)
      {
         /* BDBG_WRN(("BSAT_45216_P_GetFastChannelStatus: cmd changed to 0x%X", val)); */
         goto write_command;
      }

      return BSAT_ERR_INVALID_STATE; /* should never happen */
   }

   pStatus->bDemodLocked = (result & 0x80) ? true : false;
   for (i = 0; i < n; i++)
   {
      if (result & (1<<i))
      {
         pStatus->item[i].bValid = true;
         BSAT_45216_CHK_RETCODE(BHAB_ReadRegister(pDevImpl->hHab, LEAP_GP_STATUS0 + (i*4), &val));
         pStatus->item[i].value.uint32 = val;
      }
   }

   done:
   BDBG_LEAVE(BSAT_45216_P_GetFastChannelStatus);
   return retCode;
}
