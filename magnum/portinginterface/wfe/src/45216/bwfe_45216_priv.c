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
#include "bstd.h"
#include "bchp_45216_leap_host_l2.h"
#include "bchp_45216_tm.h"
#include "bwfe.h"
#include "bwfe_priv.h"
#include "bwfe_45216_priv.h"
#include "bhab.h"
#include "bhab_45216.h"


BDBG_MODULE(bwfe_45216_priv);

/* local functions */
BERR_Code BWFE_45216_P_EnableSaDoneInterrupt(BWFE_Handle h, bool bEnable);
BERR_Code BWFE_45216_P_EnableReadyInterrupt(BWFE_ChannelHandle h, bool bEnable);
BERR_Code BWFE_45216_P_InterruptCallback(void *pParm1, int parm2);
BERR_Code BWFE_45216_P_SendCommand(BHAB_Handle h, uint32_t *pBuf, uint32_t n);


/******************************************************************************
 BWFE_45216_P_Open()
******************************************************************************/
BERR_Code BWFE_45216_P_Open(
   BWFE_Handle *h,      /* [out] BWFE handle */
   BCHP_Handle hChip,   /* [in] chip handle */
   void *pReg,          /* [in] pointer to hab handle */
   BINT_Handle hInt,    /* [in] interrupt handle */
   const BWFE_Settings *pDefSettings   /* [in] default settings */
)
{
   BERR_Code retCode = BERR_SUCCESS;
   BWFE_Handle hDev;
   BWFE_45216_P_Handle *hImplDev;
   BWFE_ChannelInfo chInfo;
   uint8_t i;
   BHAB_Handle hHab;

   BSTD_UNUSED(hInt);
   BSTD_UNUSED(hChip);

   BDBG_ASSERT(pReg);
   hHab = (BHAB_Handle)pReg;

   /* allocate heap memory for device handle */
   hDev = (BWFE_Handle)BKNI_Malloc(sizeof(BWFE_P_Handle));
   BDBG_ASSERT(hDev);
   BKNI_Memset((void*)hDev, 0, sizeof(BWFE_P_Handle));
   hImplDev = (BWFE_45216_P_Handle *)BKNI_Malloc(sizeof(BWFE_45216_P_Handle));
   BDBG_ASSERT(hImplDev);
   BKNI_Memset((void*)hImplDev, 0, sizeof(BWFE_45216_P_Handle));
   hDev->pImpl = (void*)hImplDev;

   /* allocate heap memory for channel handle pointer */
   hDev->pChannels = (BWFE_P_ChannelHandle **)BKNI_Malloc(BWFE_45216_MAX_CHANNELS * sizeof(BWFE_P_ChannelHandle *));
   BDBG_ASSERT(hDev->pChannels);
   BKNI_Memset((void*)hDev->pChannels, 0, BWFE_45216_MAX_CHANNELS * sizeof(BWFE_P_ChannelHandle *));

   /* reference channel handle pointer not used */
   hDev->pRefChannels = NULL;

   /* initialize device handle */
   BKNI_Memcpy((void*)(&(hDev->settings)), (void*)pDefSettings, sizeof(BWFE_Settings));
   hImplDev->hHab = (BHAB_Handle)pReg;
   hImplDev->numSamples = 0;

   if (BWFE_45216_P_GetTotalChannels(hDev, &chInfo) == BERR_SUCCESS)
      hDev->totalChannels = chInfo.maxChannels;
   else
      hDev->totalChannels = BWFE_45216_MAX_CHANNELS;
   for (i = 0; i < hDev->totalChannels; i++)
      hDev->pChannels[i] = NULL;

   /* create events */
   retCode = BKNI_CreateEvent(&(hImplDev->hSaDoneEvent));
   if (retCode != BERR_SUCCESS)
   {
      BKNI_Free((void*)hDev->pChannels);
      BKNI_Free((void*)hImplDev);
      BKNI_Free((void*)hDev);

      *h = NULL;
      return retCode;
   }

   /* install callback */
   BHAB_InstallInterruptCallback(hHab, BHAB_DevId_eWFE, BWFE_45216_P_InterruptCallback, (void*)hDev, 0);

   *h = hDev;
   return retCode;
}


/******************************************************************************
 BWFE_45216_P_Close()
******************************************************************************/
BERR_Code BWFE_45216_P_Close(BWFE_Handle h)
{
   BWFE_45216_P_Handle *pDevImpl = (BWFE_45216_P_Handle *)(h->pImpl);
   uint32_t mask;
   BERR_Code retCode;

   BDBG_ASSERT(h);

   /* disable wfe host interrupts */
   mask = BHAB_45216_HIRQ0_WFE_MASK;
   BHAB_CHK_RETCODE(BHAB_WriteRegister(pDevImpl->hHab, BCHP_LEAP_HOST_L2_MASK_SET0, &mask));

   retCode = BHAB_UnInstallInterruptCallback(pDevImpl->hHab, BHAB_DevId_eWFE);
   BKNI_DestroyEvent(((BWFE_45216_P_Handle *)(h->pImpl))->hSaDoneEvent);
   BKNI_Free((void*)h->pChannels);
   BKNI_Free((void*)h->pImpl);
   BKNI_Free((void*)h);
   h = NULL;

   done:
   return retCode;
}


/******************************************************************************
 BWFE_45216_P_GetTotalChannels()
******************************************************************************/
BERR_Code BWFE_45216_P_GetTotalChannels(
   BWFE_Handle h,             /* [in] BWFE handle */
   BWFE_ChannelInfo  *pInfo   /* [out] channel availability info */
)
{
   BWFE_45216_P_Handle *pImpl = (BWFE_45216_P_Handle *)(h->pImpl);
   BERR_Code retCode;
   uint32_t product_id;

   BWFE_45216_CHK_RETCODE(BHAB_ReadRegister(pImpl->hHab, BCHP_TM_PRODUCT_ID, &product_id));
   product_id >>= 8;

   pInfo->maxChannels = BWFE_45216_MAX_CHANNELS;
   pInfo->numChannels = (product_id >> 8) & 0x3;
	if (pInfo->numChannels == 0) /* assume 45208 in case product id is 0 */
	   pInfo->numChannels = 2;
   pInfo->availChannelsMask = (1 << pInfo->numChannels) - 1;

   done:
   return retCode;
}


/******************************************************************************
 BWFE_45216_P_OpenChannel()
******************************************************************************/
BERR_Code BWFE_45216_P_OpenChannel(
   BWFE_Handle                h,               /* [in] BWFE handle */
   BWFE_ChannelHandle         *pChannelHandle, /* [out] BWFE channel handle */
   uint8_t                    chanNum,         /* [in] channel number */
   const BWFE_ChannelSettings *pSettings       /* [in] channel settings */
)
{
   BERR_Code retCode;
   BWFE_ChannelSettings cs;
   BWFE_P_ChannelHandle *ch;
   BWFE_45216_P_ChannelHandle *chImpl;

   BDBG_ASSERT(h);
   BDBG_ASSERT(h->totalChannels <= BWFE_45216_MAX_CHANNELS);
   BDBG_ASSERT(chanNum < h->totalChannels);

   if (pSettings == NULL)
      BWFE_45216_GetChannelDefaultSettings(h, chanNum, &cs);
   else
      cs = *pSettings;

   /* allocate memory for the channel handle */
   ch = (BWFE_P_ChannelHandle *)BKNI_Malloc(sizeof(BWFE_P_ChannelHandle));
   BDBG_ASSERT(ch);
   BKNI_Memcpy((void*)(&ch->settings), (void*)&cs, sizeof(BWFE_ChannelSettings));
   ch->channel = (uint8_t)chanNum;
   ch->pDevice = h;
   h->pChannels[chanNum] = ch;

   chImpl = (BWFE_45216_P_ChannelHandle *)BKNI_Malloc(sizeof(BWFE_45216_P_ChannelHandle));
   BDBG_ASSERT(chImpl);

   retCode = BKNI_CreateEvent(&(chImpl->hWfeReady));
   if (retCode != BERR_SUCCESS)
   {
      BKNI_DestroyEvent(chImpl->hWfeReady);

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
 BWFE_45216_P_CloseChannel()
******************************************************************************/
BERR_Code BWFE_45216_P_CloseChannel(BWFE_ChannelHandle h)
{
   BWFE_45216_P_ChannelHandle *chImpl;

   chImpl = (BWFE_45216_P_ChannelHandle *)(h->pImpl);
   BKNI_DestroyEvent(chImpl->hWfeReady);
   BKNI_Free((void*)chImpl);
   BKNI_Free((void*)h);
   h = NULL;

   return BERR_SUCCESS;
}


/******************************************************************************
 BWFE_45216_P_Reset()
******************************************************************************/
BERR_Code BWFE_45216_P_Reset(BWFE_Handle h)
{
   BWFE_45216_P_Handle *pImpl = (BWFE_45216_P_Handle *)(h->pImpl);
   BERR_Code retCode;
   uint32_t buf[2], irq_mask;

   BDBG_ENTER(BWFE_45216_P_Reset);

   /* disable wfe host interrupts */
   irq_mask = BHAB_45216_HIRQ0_WFE_MASK;
   BHAB_CHK_RETCODE(BHAB_WriteRegister(pImpl->hHab, BCHP_LEAP_HOST_L2_MASK_SET0, &irq_mask));

   buf[0] = BHAB_45216_InitHeader(0x09, 0, 0, BHAB_45216_MODULE_WFE);
   BWFE_45216_CHK_RETCODE(BWFE_45216_P_SendCommand(pImpl->hHab, buf, 2));

   BKNI_Sleep(10); /* wait for soft reset to complete */

   done:
   BDBG_LEAVE(BWFE_45216_P_Reset);
   return retCode;

}


/******************************************************************************
 BWFE_45216_P_GetVersion()
******************************************************************************/
BERR_Code BWFE_45216_P_GetVersion(BWFE_Handle h, BFEC_VersionInfo *pVersionInfo)
{
   BSTD_UNUSED(h);
   pVersionInfo->majorVersion = BWFE_API_VERSION;
   pVersionInfo->minorVersion = BWFE_45216_RELEASE_VERSION;
   pVersionInfo->buildType = 0;
   pVersionInfo->buildId = 0;
   return BERR_SUCCESS;
}


/******************************************************************************
 BWFE_45216_P_EnableInput()
******************************************************************************/
BERR_Code BWFE_45216_P_IsInputEnabled(BWFE_ChannelHandle h, bool *pbEnabled)
{
   BERR_Code retCode;
   uint32_t buf[3];
   BWFE_45216_P_Handle *pImpl = (BWFE_45216_P_Handle *)(h->pDevice->pImpl);

   BDBG_ENTER(BWFE_45216_P_IsInputEnabled);

   buf[0] = BHAB_45216_InitHeader(0x32, h->channel, 0, BHAB_45216_MODULE_WFE);
   buf[1] = 0;
   BWFE_45216_CHK_RETCODE(BWFE_45216_P_SendCommand(pImpl->hHab, buf, 3));

   *pbEnabled = (buf[1] == 1) ? true : false;

   done:
   BDBG_LEAVE(BWFE_45216_P_IsInputEnabled);
   return retCode;
}


/******************************************************************************
 BWFE_45216_P_EnableInput()
******************************************************************************/
BERR_Code BWFE_45216_P_EnableInput(BWFE_ChannelHandle h)
{
   BWFE_45216_P_Handle *pDevImpl = (BWFE_45216_P_Handle *)(h->pDevice->pImpl);
   BERR_Code retCode;
   uint32_t buf[3];

   BDBG_ENTER(BWFE_45216_P_EnableInput);

   BWFE_45216_CHK_RETCODE(BWFE_45216_P_EnableReadyInterrupt(h, true));

   buf[0] = BHAB_45216_InitHeader(0x25, h->channel, 0, BHAB_45216_MODULE_WFE);
   buf[1] = 0; /* power up */
   BWFE_45216_CHK_RETCODE(BWFE_45216_P_SendCommand(pDevImpl->hHab, buf, 3));

   done:
   BDBG_LEAVE(BWFE_45216_P_EnableInput);
   return retCode;
}


/******************************************************************************
 BWFE_45216_P_DisableInput()
******************************************************************************/
BERR_Code BWFE_45216_P_DisableInput(BWFE_ChannelHandle h)
{
   BWFE_45216_P_Handle *pDevImpl = (BWFE_45216_P_Handle *)(h->pDevice->pImpl);
   BERR_Code retCode;
   uint32_t buf[3];

   BDBG_ENTER(BWFE_45216_P_DisableInput);

   buf[0] = BHAB_45216_InitHeader(0x25, h->channel, 0, BHAB_45216_MODULE_WFE);
   buf[1] = 1; /* power down */
   BWFE_45216_CHK_RETCODE(BWFE_45216_P_SendCommand(pDevImpl->hHab, buf, 3));

   done:
   BDBG_LEAVE(BWFE_45216_P_DisableInput);
   return retCode;
}


/******************************************************************************
 BWFE_45216_P_GetWfeReadyEventHandle()
******************************************************************************/
BERR_Code BWFE_45216_P_GetWfeReadyEventHandle(BWFE_ChannelHandle h, BKNI_EventHandle *hEvent)
{
   *hEvent = ((BWFE_45216_P_ChannelHandle *)(h->pImpl))->hWfeReady;
   return BERR_SUCCESS;
}


/******************************************************************************
 BWFE_45216_P_GetSaDoneEventHandle()
******************************************************************************/
BERR_Code BWFE_45216_P_GetSaDoneEventHandle(
   BWFE_Handle h,            /* [in] BWFE handle */
   BKNI_EventHandle *hEvent  /* [out] SA done event handle */
)
{
   *hEvent = ((BWFE_45216_P_Handle *)(h->pImpl))->hSaDoneEvent;
   return BERR_SUCCESS;
}


/******************************************************************************
 BWFE_45216_P_ScanSpectrum()
******************************************************************************/
BERR_Code BWFE_45216_P_ScanSpectrum(BWFE_ChannelHandle h, BWFE_SpecAnalyzerParams *pParams)
{
   BWFE_45216_P_Handle *pDevImpl = (BWFE_45216_P_Handle *)(h->pDevice->pImpl);
   BERR_Code retCode;
   uint32_t freqStart, freqStop;
   uint16_t numSamples;
   uint32_t buf[5];

   BDBG_ENTER(BWFE_45216_P_ScanSpectrum);

   freqStart = pParams->freqStartHz;
   freqStop = pParams->freqStopHz;
   numSamples = pParams->numSamples;

   /* validate spec analyzer parameters */
   if (freqStart >= freqStop)
   {
      BDBG_ERR(("freqStart: %d >= freqStop: %d!", freqStart, freqStop));
      return BERR_INVALID_PARAMETER;
   }
   else if ((numSamples > BWFE_SA_MAX_SAMPLES) || (numSamples == 0))
   {
      BDBG_ERR(("numSamples must be between [1,%d]!", BWFE_SA_MAX_SAMPLES));
      return BERR_INVALID_PARAMETER;
   }

   BWFE_45216_CHK_RETCODE(BWFE_45216_P_EnableSaDoneInterrupt(h->pDevice, true));

   buf[0] = BHAB_45216_InitHeader(0x35, h->channel, 0, 0);
   buf[1] = freqStart;
   buf[2] = freqStop;
   buf[3] = numSamples;
   BWFE_45216_CHK_RETCODE(BWFE_45216_P_SendCommand(pDevImpl->hHab, buf, 5));

   done:
   BDBG_LEAVE(BWFE_45216_P_ScanSpectrum);
   if (retCode == BERR_SUCCESS)
      pDevImpl->numSamples = numSamples;  /* save requested num samples */
   return retCode;
}


/******************************************************************************
 BWFE_45216_P_GetSaSamples()
******************************************************************************/
BERR_Code BWFE_45216_P_GetSaSamples(BWFE_Handle h, uint32_t *pSaSamples)
{
   BWFE_45216_P_Handle *pDevImpl = (BWFE_45216_P_Handle *)(h->pImpl);
   BERR_Code retCode;
   uint32_t addr, buf[3];

   buf[0] = BHAB_45216_InitHeader(0x36, 0, 0, 0);;
   BWFE_45216_CHK_RETCODE(BWFE_45216_P_SendCommand(pDevImpl->hHab, buf, 3));

   /* read samples starting from returned address */
   addr = buf[1];
   BWFE_45216_CHK_RETCODE(BHAB_ReadMemory(pDevImpl->hHab, addr, (uint8_t *)pSaSamples, pDevImpl->numSamples * 4));

   done:
   return retCode;
}


/******************************************************************************
 BWFE_45216_P_EnableSaDoneInterrupt()
******************************************************************************/
BERR_Code BWFE_45216_P_EnableSaDoneInterrupt(
   BWFE_Handle h,    /* [in] BWFE channel handle */
   bool bEnable      /* [in] true = enable sa interrupt, false = disables sa interrupt */
)
{
   BWFE_45216_P_Handle *pImpl = (BWFE_45216_P_Handle *)(h->pImpl);
   BERR_Code retCode;
   uint32_t mask;

   mask = BHAB_45216_HIRQ0_WFE_SA_DONE;

   /* clear the interrupt status */
   BWFE_45216_CHK_RETCODE(BHAB_WriteRegister(pImpl->hHab, BCHP_LEAP_HOST_L2_CLEAR0, &mask));

   if (bEnable)
   {
      BWFE_45216_CHK_RETCODE(BHAB_WriteRegister(pImpl->hHab, BCHP_LEAP_HOST_L2_MASK_CLEAR0, &mask));
   }
   else
   {
      BWFE_45216_CHK_RETCODE(BHAB_WriteRegister(pImpl->hHab, BCHP_LEAP_HOST_L2_MASK_SET0, &mask));
   }

   done:
   return retCode;
}


/******************************************************************************
 BWFE_45216_P_EnableReadyInterrupt()
******************************************************************************/
BERR_Code BWFE_45216_P_EnableReadyInterrupt(
   BWFE_ChannelHandle h,    /* [in] BWFE channel handle */
   bool bEnable      /* [in] true = enable sa interrupt, false = disables sa interrupt */
)
{
   BWFE_45216_P_Handle *pDevImpl = (BWFE_45216_P_Handle *)(h->pDevice->pImpl);
   BERR_Code retCode;
   uint32_t mask;

   mask = BHAB_45216_HIRQ0_WFE0_READY << h->channel;

   /* clear the interrupt status */
   BWFE_45216_CHK_RETCODE(BHAB_WriteRegister(pDevImpl->hHab, BCHP_LEAP_HOST_L2_CLEAR0, &mask));

   if (bEnable)
   {
      BWFE_45216_CHK_RETCODE(BHAB_WriteRegister(pDevImpl->hHab, BCHP_LEAP_HOST_L2_MASK_CLEAR0, &mask));
   }
   else
   {
      BWFE_45216_CHK_RETCODE(BHAB_WriteRegister(pDevImpl->hHab, BCHP_LEAP_HOST_L2_MASK_SET0, &mask));
   }

   done:
   return retCode;
}


/******************************************************************************
 BWFE_45216_P_InterruptCallback()
******************************************************************************/
BERR_Code BWFE_45216_P_InterruptCallback(void *pParm1, int parm2)
{
   BHAB_45216_IrqStatus *pParams = (BHAB_45216_IrqStatus *)pParm1;
   BWFE_Handle h = (BWFE_Handle)(pParams->pParm1);
   BWFE_45216_P_Handle *pImpl = (BWFE_45216_P_Handle *)(h->pImpl);
   uint32_t fstatus0 = pParams->status0;
   uint32_t fstatus1 = pParams->status1;
   uint32_t channel, val32;
   BWFE_45216_P_ChannelHandle *pChn;

   BSTD_UNUSED(parm2);
   BSTD_UNUSED(fstatus1);

   if (fstatus0)
   {
      if (fstatus0 & BHAB_45216_HIRQ0_WFE_SA_DONE)
         BKNI_SetEvent(pImpl->hSaDoneEvent);

      for (channel = 0; channel < h->totalChannels; channel++)
      {
         if (h->pChannels[channel] == NULL)
            continue;

         pChn = (BWFE_45216_P_ChannelHandle *)(h->pChannels[channel]->pImpl);
         val32 = (fstatus0 >> channel);
         if (val32 & BHAB_45216_HIRQ0_WFE0_READY)
         {
            BKNI_SetEvent(pChn->hWfeReady);
         }
      }
   }

   return BERR_SUCCESS;
}


/******************************************************************************
 BWFE_45216_P_SendCommand()
******************************************************************************/
BERR_Code BWFE_45216_P_SendCommand(BHAB_Handle h, uint32_t *pBuf, uint32_t n)
{
   return BHAB_SendHabCommand(h, (uint8_t*)pBuf, (uint16_t)n, (uint8_t*)pBuf, (uint16_t)n, true, true, 0);
}
