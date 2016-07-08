/***************************************************************************
 *     Copyright (c) 2003-2013, Broadcom Corporation
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
#include "bchp_4538_leap_host_irq.h"
#include "bwfe.h"
#include "bwfe_priv.h"
#include "bwfe_4538_priv.h"
#include "bhab.h"
#include "bhab_4538.h"


BDBG_MODULE(bwfe_4538_priv);

/* local functions */
BERR_Code BWFE_4538_P_EnableSaDoneInterrupt(BWFE_Handle h, bool bEnable);
BERR_Code BWFE_4538_P_EnableReadyInterrupt(BWFE_ChannelHandle h, bool bEnable);
BERR_Code BWFE_4538_P_InterruptCallback(void *pParm1, int parm2);


/******************************************************************************
 BWFE_4538_P_Open()
******************************************************************************/
BERR_Code BWFE_4538_P_Open(
   BWFE_Handle *h,      /* [out] BWFE handle */
   BCHP_Handle hChip,   /* [in] chip handle */
   void *pReg,          /* [in] pointer to hab handle */
   BINT_Handle hInt,    /* [in] interrupt handle */
   const BWFE_Settings *pDefSettings   /* [in] default settings */
)
{
   BERR_Code retCode = BERR_SUCCESS;
   BWFE_Handle hDev;
   BWFE_4538_P_Handle *hImplDev;
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
   hImplDev = (BWFE_4538_P_Handle *)BKNI_Malloc(sizeof(BWFE_4538_P_Handle));
   BDBG_ASSERT(hImplDev);
   BKNI_Memset((void*)hImplDev, 0, sizeof(BWFE_4538_P_Handle));
   hDev->pImpl = (void*)hImplDev;

   /* allocate heap memory for channel handle pointer */
   hDev->pChannels = (BWFE_P_ChannelHandle **)BKNI_Malloc(BWFE_4538_MAX_CHANNELS * sizeof(BWFE_P_ChannelHandle *));
   BDBG_ASSERT(hDev->pChannels);
   BKNI_Memset((void*)hDev->pChannels, 0, BWFE_4538_MAX_CHANNELS * sizeof(BWFE_P_ChannelHandle *));

   /* reference channel handle pointer not used */
   hDev->pRefChannels = NULL;

   /* initialize device handle */
   BKNI_Memcpy((void*)(&(hDev->settings)), (void*)pDefSettings, sizeof(BWFE_Settings));
   hImplDev->hHab = (BHAB_Handle)pReg;
   hImplDev->numSamples = 0;

   if (BWFE_4538_P_GetTotalChannels(hDev, &chInfo) == BERR_SUCCESS)
      hDev->totalChannels = chInfo.numChannels;
   else
      hDev->totalChannels = BWFE_4538_MAX_CHANNELS;
   for (i = 0; i < BWFE_4538_MAX_CHANNELS; i++)
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
   BHAB_InstallInterruptCallback(hHab, BHAB_DevId_eWFE, BWFE_4538_P_InterruptCallback, (void*)hDev, 0);

   *h = hDev;
   return retCode;
}


/******************************************************************************
 BWFE_4538_P_Close()
******************************************************************************/
BERR_Code BWFE_4538_P_Close(BWFE_Handle h)
{
   BWFE_4538_P_Handle *pDevImpl = (BWFE_4538_P_Handle *)(h->pImpl);
   uint32_t mask;
   BERR_Code retCode;

   BDBG_ASSERT(h);

   /* disable wfe host interrupts */
   mask = 0x0000F800;
   BHAB_CHK_RETCODE(BHAB_WriteRegister(pDevImpl->hHab, BCHP_LEAP_HOST_IRQ_MASK_SET0, &mask));

   retCode = BHAB_UnInstallInterruptCallback(pDevImpl->hHab, BHAB_DevId_eWFE);
   BKNI_DestroyEvent(((BWFE_4538_P_Handle *)(h->pImpl))->hSaDoneEvent);
   BKNI_Free((void*)h->pChannels);
   BKNI_Free((void*)h->pImpl);
   BKNI_Free((void*)h);
   h = NULL;

   done:
   return retCode;
}


/******************************************************************************
 BWFE_4538_P_GetTotalChannels()
******************************************************************************/
BERR_Code BWFE_4538_P_GetTotalChannels(
   BWFE_Handle h,             /* [in] BWFE handle */
   BWFE_ChannelInfo  *pInfo   /* [out] channel availability info */
)
{
   #define BCHP_4538_JTAG_OTP_GENERAL_STATUS_5  0x00090828

   BERR_Code retCode;
   BWFE_4538_P_Handle *pDevImpl = (BWFE_4538_P_Handle *)(h->pImpl);
   uint32_t otp, disable_input;
   uint8_t i;

   pInfo->maxChannels = BWFE_4538_MAX_CHANNELS;
   pInfo->numChannels = BWFE_4538_MAX_CHANNELS;
   pInfo->availChannelsMask = 0xF;

   BWFE_4538_CHK_RETCODE(BHAB_ReadRegister(pDevImpl->hHab, BCHP_4538_JTAG_OTP_GENERAL_STATUS_5, &otp));

   disable_input = (otp >> 20) & 0x0F;
   if (disable_input)
   {
      for (i = 0; i < BWFE_4538_MAX_CHANNELS; i++)
      {
         if (disable_input & (1 << i))
            break;
      }
   }
   else
      i = BWFE_4538_MAX_CHANNELS;

   pInfo->numChannels = i;
   pInfo->availChannelsMask = ~disable_input & 0xF;

   done:
   return retCode;
}


/******************************************************************************
 BWFE_4538_P_OpenChannel()
******************************************************************************/
BERR_Code BWFE_4538_P_OpenChannel(
   BWFE_Handle                h,               /* [in] BWFE handle */
   BWFE_ChannelHandle         *pChannelHandle, /* [out] BWFE channel handle */
   uint8_t                    chanNum,         /* [in] channel number */
   const BWFE_ChannelSettings *pSettings       /* [in] channel settings */
)
{
   BERR_Code retCode;
   BWFE_ChannelSettings cs;
   BWFE_P_ChannelHandle *ch;
   BWFE_4538_P_ChannelHandle *chImpl;

   BDBG_ASSERT(h);
   BDBG_ASSERT(h->totalChannels <= BWFE_4538_MAX_CHANNELS);
   BDBG_ASSERT(chanNum < h->totalChannels);

   if (pSettings == NULL)
      BWFE_4538_GetChannelDefaultSettings(h, chanNum, &cs);
   else
      cs = *pSettings;

   /* allocate memory for the channel handle */
   ch = (BWFE_P_ChannelHandle *)BKNI_Malloc(sizeof(BWFE_P_ChannelHandle));
   BDBG_ASSERT(ch);
   BKNI_Memcpy((void*)(&ch->settings), (void*)&cs, sizeof(BWFE_ChannelSettings));
   ch->channel = (uint8_t)chanNum;
   ch->pDevice = h;
   h->pChannels[chanNum] = ch;

   chImpl = (BWFE_4538_P_ChannelHandle *)BKNI_Malloc(sizeof(BWFE_4538_P_ChannelHandle));
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
 BWFE_4538_P_CloseChannel()
******************************************************************************/
BERR_Code BWFE_4538_P_CloseChannel(BWFE_ChannelHandle h)
{
   BWFE_4538_P_ChannelHandle *chImpl;

   chImpl = (BWFE_4538_P_ChannelHandle *)(h->pImpl);
   BKNI_DestroyEvent(chImpl->hWfeReady);
   BKNI_Free((void*)chImpl);
   BKNI_Free((void*)h);
   h = NULL;

   return BERR_SUCCESS;
}


/******************************************************************************
 BWFE_4538_P_Reset()
******************************************************************************/
BERR_Code BWFE_4538_P_Reset(BWFE_Handle h)
{
   BWFE_4538_P_Handle *pImpl = (BWFE_4538_P_Handle *)(h->pImpl);
   BERR_Code retCode;
   uint8_t buf[3];

   buf[0] = 0x05;
   buf[1] = 0x00;
   BWFE_4538_CHK_RETCODE(BHAB_SendHabCommand(pImpl->hHab, buf, 3, buf, 3, true, true, 3));

   BKNI_Sleep(10); /* wait for soft reset to complete */

   done:
   return retCode;
}


/******************************************************************************
 BWFE_4538_P_GetVersion()
******************************************************************************/
BERR_Code BWFE_4538_P_GetVersion(BWFE_Handle h, BFEC_VersionInfo *pVersionInfo)
{
   BSTD_UNUSED(h);
   pVersionInfo->majorVersion = BWFE_API_VERSION;
   pVersionInfo->minorVersion = BWFE_4538_RELEASE_VERSION;
   pVersionInfo->buildType = 0;
   pVersionInfo->buildId = 0;
   return BERR_SUCCESS;
}


/******************************************************************************
 BWFE_4538_P_EnableInput()
******************************************************************************/
BERR_Code BWFE_4538_P_IsInputEnabled(BWFE_ChannelHandle h, bool *pbEnabled)
{
   BWFE_4538_P_Handle *pDevImpl = (BWFE_4538_P_Handle *)(h->pDevice->pImpl);
   BERR_Code retCode;
   uint8_t buf[4];

   buf[0] = 0x2D;
   buf[1] = h->channel;
   buf[2] = 0;
   BWFE_4538_CHK_RETCODE(BHAB_SendHabCommand(pDevImpl->hHab, buf, 4, buf, 4, true, true, 4));

   *pbEnabled = buf[2] ? true : false;

   done:
   return retCode;
}


/******************************************************************************
 BWFE_4538_P_EnableInput()
******************************************************************************/
BERR_Code BWFE_4538_P_EnableInput(BWFE_ChannelHandle h)
{
   BWFE_4538_P_Handle *pDevImpl = (BWFE_4538_P_Handle *)(h->pDevice->pImpl);
   BERR_Code retCode;
   uint8_t buf[4];

   BWFE_4538_CHK_RETCODE(BWFE_4538_P_EnableReadyInterrupt(h, true));

   buf[0] = 0x4F;
   buf[1] = h->channel;
   buf[2] = 1;
   BWFE_4538_CHK_RETCODE(BHAB_SendHabCommand(pDevImpl->hHab, buf, 4, buf, 4, true, true, 4));

   done:
   return retCode;
}


/******************************************************************************
 BWFE_4538_P_DisableInput()
******************************************************************************/
BERR_Code BWFE_4538_P_DisableInput(BWFE_ChannelHandle h)
{
   BWFE_4538_P_Handle *pDevImpl = (BWFE_4538_P_Handle *)(h->pDevice->pImpl);
   BERR_Code retCode;
   uint8_t buf[4];

   buf[0] = 0x4F;
   buf[1] = h->channel;
   buf[2] = 0;
   BWFE_4538_CHK_RETCODE(BHAB_SendHabCommand(pDevImpl->hHab, buf, 4, buf, 4, true, true, 4));

   done:
   return retCode;
}


/******************************************************************************
 BWFE_4538_P_GetWfeReadyEventHandle()
******************************************************************************/
BERR_Code BWFE_4538_P_GetWfeReadyEventHandle(BWFE_ChannelHandle h, BKNI_EventHandle *hEvent)
{
   *hEvent = ((BWFE_4538_P_ChannelHandle *)(h->pImpl))->hWfeReady;
   return BERR_SUCCESS;
}


/******************************************************************************
 BWFE_4538_P_GetSaDoneEventHandle()
******************************************************************************/
BERR_Code BWFE_4538_P_GetSaDoneEventHandle(
   BWFE_Handle h,            /* [in] BWFE handle */
   BKNI_EventHandle *hEvent  /* [out] SA done event handle */
)
{
   *hEvent = ((BWFE_4538_P_Handle *)(h->pImpl))->hSaDoneEvent;
   return BERR_SUCCESS;
}


/******************************************************************************
 BWFE_4538_P_ScanSpectrum()
******************************************************************************/
BERR_Code BWFE_4538_P_ScanSpectrum(BWFE_ChannelHandle h, BWFE_SpecAnalyzerParams *pParams)
{
   BWFE_4538_P_Handle *pDevImpl = (BWFE_4538_P_Handle *)(h->pDevice->pImpl);
   BERR_Code retCode;
   uint32_t freqStart, freqStop;
   uint16_t numSamples;
   uint8_t buf[13];

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

   BWFE_4538_CHK_RETCODE(BWFE_4538_P_EnableSaDoneInterrupt(h->pDevice, false));

   buf[0] = 0x47;
   buf[1] = h->channel;
   buf[2] = (uint8_t)(freqStart >> 24);
   buf[3] = (uint8_t)(freqStart >> 16);
   buf[4] = (uint8_t)(freqStart >> 8);
   buf[5] = (uint8_t)(freqStart & 0xFF);
   buf[6] = (uint8_t)(freqStop >> 24);
   buf[7] = (uint8_t)(freqStop >> 16);
   buf[8] = (uint8_t)(freqStop >> 8);
   buf[9] = (uint8_t)(freqStop & 0xFF);
   buf[10] = (uint8_t)(numSamples >> 8);
   buf[11] = (uint8_t)(numSamples & 0xFF);
   BWFE_4538_CHK_RETCODE(BHAB_SendHabCommand(pDevImpl->hHab, buf, 13, buf, 13, true, true, 13));

   BWFE_4538_CHK_RETCODE(BWFE_4538_P_EnableSaDoneInterrupt(h->pDevice, true));

   done:
   if (retCode == BERR_SUCCESS)
      pDevImpl->numSamples = numSamples;  /* save requested num samples */
   return retCode;
}


/******************************************************************************
 BWFE_4538_P_GetSaSamples()
******************************************************************************/
BERR_Code BWFE_4538_P_GetSaSamples(BWFE_Handle h, uint32_t *pSaSamples)
{
   BWFE_4538_P_Handle *pDevImpl = (BWFE_4538_P_Handle *)(h->pImpl);
   BERR_Code retCode;
   uint32_t i, addr;
   uint8_t buf[6];
   uint8_t *pSaSamplesByte = (uint8_t *)pSaSamples;

   buf[0] = 0x48;
   buf[1] = 0;
   buf[2] = 0;
   buf[3] = 0;
   buf[4] = 0;
   BWFE_4538_CHK_RETCODE(BHAB_SendHabCommand(pDevImpl->hHab, buf, 6, buf, 6, true, true, 6));

   /* read samples starting from returned address */
   addr = (buf[1] << 24) | (buf[2] << 16) | (buf[3] << 8) | buf[4];
   BWFE_4538_CHK_RETCODE(BHAB_ReadMemory(pDevImpl->hHab, addr, pSaSamplesByte, pDevImpl->numSamples * 4));

   for (i = 0; i < pDevImpl->numSamples; i++)
   {
      /* order bytes from read memory */
      pSaSamples[i] = (pSaSamplesByte[4*i] << 24) | (pSaSamplesByte[4*i+1] << 16) | (pSaSamplesByte[4*i+2] << 8) | pSaSamplesByte[4*i+3];
   }

   done:
   return retCode;
}


/******************************************************************************
 BWFE_4538_P_EnableSaDoneInterrupt()
******************************************************************************/
BERR_Code BWFE_4538_P_EnableSaDoneInterrupt(
   BWFE_Handle h,    /* [in] BWFE channel handle */
   bool bEnable      /* [in] true = enable sa interrupt, false = disables sa interrupt */
)
{
   BWFE_4538_P_Handle *pImpl = (BWFE_4538_P_Handle *)(h->pImpl);
   BERR_Code retCode;
   uint32_t mask;

   mask = BHAB_4538_HIRQ0_WFE_SA_DONE;

   /* clear the interrupt status */
   BWFE_4538_CHK_RETCODE(BHAB_WriteRegister(pImpl->hHab, BCHP_LEAP_HOST_IRQ_CLEAR0, &mask));

   if (bEnable)
   {
      BWFE_4538_CHK_RETCODE(BHAB_WriteRegister(pImpl->hHab, BCHP_LEAP_HOST_IRQ_MASK_CLEAR0, &mask));
   }
   else
   {
      BWFE_4538_CHK_RETCODE(BHAB_WriteRegister(pImpl->hHab, BCHP_LEAP_HOST_IRQ_MASK_SET0, &mask));
   }

   done:
   return retCode;
}


/******************************************************************************
 BWFE_4538_P_EnableReadyInterrupt()
******************************************************************************/
BERR_Code BWFE_4538_P_EnableReadyInterrupt(
   BWFE_ChannelHandle h,    /* [in] BWFE channel handle */
   bool bEnable      /* [in] true = enable sa interrupt, false = disables sa interrupt */
)
{
   BWFE_4538_P_Handle *pDevImpl = (BWFE_4538_P_Handle *)(h->pDevice->pImpl);
   BERR_Code retCode;
   uint32_t mask;

   mask = BHAB_4538_HIRQ0_WFE0_READY << h->channel;

   /* clear the interrupt status */
   BWFE_4538_CHK_RETCODE(BHAB_WriteRegister(pDevImpl->hHab, BCHP_LEAP_HOST_IRQ_CLEAR0, &mask));

   if (bEnable)
   {
      BWFE_4538_CHK_RETCODE(BHAB_WriteRegister(pDevImpl->hHab, BCHP_LEAP_HOST_IRQ_MASK_CLEAR0, &mask));
   }
   else
   {
      BWFE_4538_CHK_RETCODE(BHAB_WriteRegister(pDevImpl->hHab, BCHP_LEAP_HOST_IRQ_MASK_SET0, &mask));
   }

   done:
   return retCode;
}


/******************************************************************************
 BWFE_4538_P_InterruptCallback()
******************************************************************************/
BERR_Code BWFE_4538_P_InterruptCallback(void *pParm1, int parm2)
{
   BHAB_4538_IrqStatus *pParams = (BHAB_4538_IrqStatus *)pParm1;
   BWFE_Handle h = (BWFE_Handle)(pParams->pParm1);
   BWFE_4538_P_Handle *pImpl = (BWFE_4538_P_Handle *)(h->pImpl);
   uint32_t fstatus0 = pParams->status0;
   uint32_t fstatus1 = pParams->status1;
   uint32_t channel, val32;
   BWFE_4538_P_ChannelHandle *pChn;

   BSTD_UNUSED(parm2);
   BSTD_UNUSED(fstatus1);

   if (fstatus0)
   {
      if (fstatus0 & BHAB_4538_HIRQ0_WFE_SA_DONE)
         BKNI_SetEvent(pImpl->hSaDoneEvent);

      for (channel = 0; channel < h->totalChannels; channel++)
      {
         if (h->pChannels[channel] == NULL)
            continue;

         pChn = (BWFE_4538_P_ChannelHandle *)(h->pChannels[channel]->pImpl);
         val32 = (fstatus0 >> channel);
         if (val32 & BHAB_4538_HIRQ0_WFE0_READY)
         {
            BKNI_SetEvent(pChn->hWfeReady);
         }
      }
   }

   return BERR_SUCCESS;
}

