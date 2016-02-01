/***************************************************************************
 *     Copyright (c) 2003-2011, Broadcom Corporation
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
#include "bchp_hsi.h"
#include "bchp_leap_host_irq.h"
#include "bchp_leap_ctrl.h"
#include "bchp_leap_hab_mem.h"
#include "bchp_leap_l2.h"
#include "bi2c.h"
#include "bkni_multi.h"
#include "bast.h"
#include "bast_priv.h"
#include "bast_4528_priv.h"
#ifdef BAST_USE_HAB_PI
#include "bhab.h"
#include "bhab_4528.h"
#endif

#ifndef BAST_4528_VER
#error "BAST_4528_VER needs to be defined"
#endif

#if BAST_4528_VER!=BCHP_VER_A0
#error "unsupported version specified in BAST_4528_VER"
#endif

BDBG_MODULE(bast_4528_priv);

/* local functions */
BERR_Code BAST_4528_P_EnableLockInterrupt(BAST_ChannelHandle h, bool bEnable);
BERR_Code BAST_4528_P_EnableFskInterrupt(BAST_Handle h, bool bEnable);
void BAST_4528_P_GetRegisterAddress(BAST_ChannelHandle h, uint32_t reg, uint32_t *pAddr);
#ifdef BAST_USE_HAB_PI
BERR_Code BAST_4528_P_InterruptCallback(void *pParm1, int parm2);
#else
BERR_Code BAST_4528_P_ReadHab(BAST_Handle h, uint8_t offset, uint8_t *buf, uint8_t n);
BERR_Code BAST_4528_P_WriteHab(BAST_Handle h, uint8_t offset, uint8_t *buf, uint8_t n);
BERR_Code BAST_4528_P_RunAp(BAST_Handle h);
BERR_Code BAST_4528_P_DisableInterrupts(BAST_Handle h);
BERR_Code BAST_4528_P_ServiceHab(BAST_Handle h, uint8_t *read_buf, uint8_t read_len, uint8_t ack_byte, bool bChecksum);
BERR_Code BAST_4528_P_CheckHab(BAST_Handle h);
BERR_Code BAST_4528_P_EnableHostInterrupt(BAST_Handle h, bool bEnable);
BERR_Code BAST_4528_P_DecodeInterrupt(BAST_Handle h);
BERR_Code BAST_4528_P_WaitForEvent(BAST_Handle h, BKNI_EventHandle hEvent, int timeoutMsec);
#endif


/******************************************************************************
 BAST_4528_P_Open()
******************************************************************************/
BERR_Code BAST_4528_P_Open(
   BAST_Handle *h,                   /* [out] BAST handle */
   BCHP_Handle hChip,                /* [in] chip handle */
   void        *pReg,                /* [in] pointer to register or i2c handle */
   BINT_Handle hInterrupt,           /* [in] Interrupt handle */   
   const BAST_Settings *pDefSettings /* [in] default settings */
)
{
   BERR_Code retCode;
   BAST_Handle hDev;
   BAST_4528_P_Handle *hImplDev;
   uint8_t i;
#ifdef BAST_USE_HAB_PI
   BHAB_Handle hHab;
#endif

   BSTD_UNUSED(hInterrupt);
   BSTD_UNUSED(hChip);

#ifdef BAST_USE_HAB_PI
   BDBG_ASSERT(pReg);
   hHab = (BHAB_Handle)pReg;
#else
   BDBG_ASSERT(pDefSettings->i2c.interruptEnableFunc);
#endif

   /* allocate memory for the handle */
   hDev = (BAST_Handle)BKNI_Malloc(sizeof(BAST_P_Handle));
   BDBG_ASSERT(hDev);
   hImplDev = (BAST_4528_P_Handle *)BKNI_Malloc(sizeof(BAST_4528_P_Handle));
   BDBG_ASSERT(hImplDev);
   hDev->pImpl = (void*)hImplDev;

   hDev->pChannels = (BAST_P_ChannelHandle **)BKNI_Malloc(BAST_4528_MAX_CHANNELS * sizeof(BAST_P_ChannelHandle *));
   BDBG_ASSERT(hDev->pChannels);

   /* initialize our handle */
#ifdef BAST_USE_HAB_PI
   hImplDev->hHab = (BHAB_Handle)pReg;
#else
   hImplDev->hRegister = (BREG_I2C_Handle)pReg;
#endif
   BKNI_Memcpy((void*)(&(hDev->settings)), (void*)pDefSettings, sizeof(BAST_Settings));
   hDev->totalChannels = BAST_4528_MAX_CHANNELS;
   for (i = 0; i < hDev->totalChannels; i++)
      hDev->pChannels[i] = NULL;

   /* create events */
#ifdef BAST_USE_HAB_PI   
   retCode = BKNI_CreateEvent(&(hImplDev->hFskEvent));
   if (retCode != BERR_SUCCESS)
      goto done_0;
   retCode = BKNI_CreateEvent(&(hImplDev->hDiseqcEvent));
   if (retCode != BERR_SUCCESS)
      goto done_1;
   retCode = BKNI_CreateEvent(&(hImplDev->hDiseqcOverVoltageEvent));
   if (retCode != BERR_SUCCESS)
      goto done_2;
   retCode = BKNI_CreateEvent(&(hImplDev->hDiseqcUnderVoltageEvent));
   if (retCode != BERR_SUCCESS)
   {
      BKNI_DestroyEvent(hImplDev->hDiseqcOverVoltageEvent);

      done_2:
      BKNI_DestroyEvent(hImplDev->hDiseqcEvent);

      done_1:
      BKNI_DestroyEvent(hImplDev->hFskEvent);

      done_0:
      BKNI_Free((void*)hDev->pChannels);
      BKNI_Free((void*)hImplDev);
      BKNI_Free((void*)hDev);      
      
      *h = NULL;
      return retCode;
   }

   BHAB_InstallInterruptCallback(hHab, BHAB_DevId_eAST, BAST_4528_P_InterruptCallback, (void*)hDev, 0);
#else
   retCode = BKNI_CreateEvent(&(hImplDev->hInterruptEvent));
   if (retCode != BERR_SUCCESS)
      goto done_0;
   retCode = BKNI_CreateEvent(&(hImplDev->hApiEvent));
   if (retCode != BERR_SUCCESS)
      goto done_1;
   retCode = BKNI_CreateEvent(&(hImplDev->hInitDoneEvent));
   if (retCode != BERR_SUCCESS)
      goto done_2;
   retCode = BKNI_CreateEvent(&(hImplDev->hHabDoneEvent));
   if (retCode != BERR_SUCCESS)
      goto done_3;
   retCode = BKNI_CreateEvent(&(hImplDev->hFskEvent));
   if (retCode != BERR_SUCCESS)
      goto done_4;
   retCode = BKNI_CreateEvent(&(hImplDev->hDiseqcEvent));
   if (retCode != BERR_SUCCESS)
      goto done_5;
   retCode = BKNI_CreateEvent(&(hImplDev->hDiseqcOverVoltageEvent));
   if (retCode != BERR_SUCCESS)
      goto done_6;
   retCode = BKNI_CreateEvent(&(hImplDev->hDiseqcUnderVoltageEvent));
   if (retCode != BERR_SUCCESS)
   {
      BKNI_DestroyEvent(hImplDev->hDiseqcOverVoltageEvent);

      done_6:
      BKNI_DestroyEvent(hImplDev->hDiseqcEvent);

      done_5:
      BKNI_DestroyEvent(hImplDev->hFskEvent);

      done_4:
      BKNI_DestroyEvent(hImplDev->hHabDoneEvent);

      done_3:
      BKNI_DestroyEvent(hImplDev->hInitDoneEvent);

      done_2:
      BKNI_DestroyEvent(hImplDev->hApiEvent);

      done_1:
      BKNI_DestroyEvent(hImplDev->hInterruptEvent);

      done_0:
      BKNI_Free((void*)hDev->pChannels);
      BKNI_Free((void*)hImplDev);
      BKNI_Free((void*)hDev);      
      
      *h = NULL;
      return retCode;
   }
#endif

   *h = hDev;
   return retCode;
}


/******************************************************************************
 BAST_4528_P_Close()
******************************************************************************/
BERR_Code BAST_4528_P_Close(BAST_Handle h)
{
#ifdef BAST_USE_HAB_PI
   BAST_4528_P_Handle *pDevImpl = (BAST_4528_P_Handle *)(h->pImpl);
   uint32_t mask;
#endif      
   BERR_Code retCode;

   BDBG_ASSERT(h);

#ifdef BAST_USE_HAB_PI
   mask = 0xFFFFFFFF;
   BHAB_CHK_RETCODE(BHAB_WriteRegister(pDevImpl->hHab, BCHP_LEAP_HOST_IRQ_MASK_SET0, &mask));
   BHAB_CHK_RETCODE(BHAB_WriteRegister(pDevImpl->hHab, BCHP_LEAP_HOST_IRQ_MASK_SET1, &mask));

   retCode = BHAB_UnInstallInterruptCallback(pDevImpl->hHab, BHAB_DevId_eAST);      
#else
   retCode = BAST_4528_P_DisableInterrupts(h);

   /* reset the AP */
   if (retCode == BERR_SUCCESS)
      retCode = BAST_4528_P_ResetAp(h);

   BKNI_DestroyEvent(((BAST_4528_P_Handle *)(h->pImpl))->hInterruptEvent);
   BKNI_DestroyEvent(((BAST_4528_P_Handle *)(h->pImpl))->hApiEvent);
   BKNI_DestroyEvent(((BAST_4528_P_Handle *)(h->pImpl))->hInitDoneEvent);
   BKNI_DestroyEvent(((BAST_4528_P_Handle *)(h->pImpl))->hHabDoneEvent);
#endif
   
   BKNI_DestroyEvent(((BAST_4528_P_Handle *)(h->pImpl))->hFskEvent);
   BKNI_DestroyEvent(((BAST_4528_P_Handle *)(h->pImpl))->hDiseqcEvent);
   BKNI_DestroyEvent(((BAST_4528_P_Handle *)(h->pImpl))->hDiseqcOverVoltageEvent);
   BKNI_DestroyEvent(((BAST_4528_P_Handle *)(h->pImpl))->hDiseqcUnderVoltageEvent);
   BKNI_Free((void*)h->pChannels);
   BKNI_Free((void*)h->pImpl);
   BKNI_Free((void*)h);
   h = NULL;

#ifdef BAST_USE_HAB_PI
   done:
#endif
   return retCode;
}


/******************************************************************************
 BAST_4528_P_GetTotalChannels()
******************************************************************************/
BERR_Code BAST_4528_P_GetTotalChannels(
   BAST_Handle  h,             /* [in] BAST handle */
   uint32_t     *totalChannels /* [out] number of channels supported */
)
{
   BSTD_UNUSED(h);
   *totalChannels = BAST_4528_MAX_CHANNELS;
   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_4528_P_OpenChannel()
******************************************************************************/
BERR_Code BAST_4528_P_OpenChannel(
   BAST_Handle                 h,              /* [in] BAST handle */
   BAST_ChannelHandle         *pChannelHandle, /* [out] BAST channel handle */
   uint32_t                   chnNo,           /* [in] channel number */
   const BAST_ChannelSettings *pSettings       /* [in] channel settings */
)
{
   BERR_Code retCode;
   BAST_ChannelSettings cs;
   BAST_P_ChannelHandle *ch;
   BAST_4528_P_ChannelHandle *chImpl;

   BDBG_ASSERT(chnNo < BAST_4528_MAX_CHANNELS);

   if (pSettings == NULL)
      BAST_4528_GetChannelDefaultSettings(h, chnNo, &cs);
   else
      cs = *pSettings;

   /* allocate memory for the channel handle */
   ch = (BAST_P_ChannelHandle *)BKNI_Malloc(sizeof(BAST_P_ChannelHandle));
   BDBG_ASSERT(ch);
   BKNI_Memcpy((void*)(&ch->settings), (void*)&cs, sizeof(BAST_ChannelSettings)); 
   ch->channel = (uint8_t)chnNo;
   ch->pDevice = h;
   h->pChannels[chnNo] = ch;

   chImpl = (BAST_4528_P_ChannelHandle *)BKNI_Malloc(sizeof(BAST_4528_P_ChannelHandle));
   BDBG_ASSERT(chImpl);

   chImpl->lastMode = BAST_Mode_eUnknown;
   retCode = BKNI_CreateEvent(&(chImpl->hLockChangeEvent));
   if (retCode != BERR_SUCCESS)
      goto done_0;
   retCode = BKNI_CreateEvent(&(chImpl->hStatusEvent));
   if (retCode != BERR_SUCCESS)
      goto done_1;
   retCode = BKNI_CreateEvent(&(chImpl->hPeakScanEvent));
   if (retCode != BERR_SUCCESS)
   {
      BKNI_DestroyEvent(chImpl->hStatusEvent);

      done_1:
      BKNI_DestroyEvent(chImpl->hLockChangeEvent);

      done_0:
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
 BAST_4528_P_CloseChannel()
******************************************************************************/
BERR_Code BAST_4528_P_CloseChannel(
   BAST_ChannelHandle h  /* [in] BAST channel handle */
)
{
   BAST_4528_P_ChannelHandle *chImpl;

   chImpl = (BAST_4528_P_ChannelHandle *)(h->pImpl);
   BKNI_DestroyEvent(chImpl->hLockChangeEvent);
   BKNI_DestroyEvent(chImpl->hStatusEvent);
   BKNI_DestroyEvent(chImpl->hPeakScanEvent);
   BKNI_Free((void*)chImpl);
   BKNI_Free((void*)h);
   h = NULL;

   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_4528_P_GetDevice()
******************************************************************************/
BERR_Code BAST_4528_P_GetDevice(
   BAST_ChannelHandle h,  /* [in] BAST channel handle */
   BAST_Handle *pDev      /* [out] BAST handle */
)
{
   *pDev = h->pDevice;
   return BERR_SUCCESS; 
}


/******************************************************************************
 BAST_4528_P_InitAp()
******************************************************************************/
BERR_Code BAST_4528_P_InitAp(
   BAST_Handle h,            /* [in] BAST handle */
   const uint8_t *pHexImage  /* [in] pointer to BCM4528 firmware image */
)
{
   BAST_4528_P_Handle *pImpl = (BAST_4528_P_Handle *)(h->pImpl);
   
#ifdef BAST_USE_HAB_PI   
   return BHAB_InitAp(pImpl->hHab, pHexImage);
#else
   BERR_Code retCode = BERR_SUCCESS;
   uint32_t n, addr, init_done_mask, retries, strap, val32;
   const uint8_t *pImage;
#ifdef BAST_VERIFY_DOWNLOAD
   static uint8_t *pVerifyBuf = NULL;
#endif
   /* disable host interrupts */
   BAST_4528_CHK_RETCODE(BAST_4528_P_EnableHostInterrupt(h, false));
   
   /* determine boot mode */
   BAST_4528_CHK_RETCODE(BAST_4528_ReadRbus(h, BCHP_LEAP_CTRL_CPU_STRAPS, &strap, 1));
   if (strap == 0)
   {
      /* boot mode 0 */

      /* reset the AP before downloading the firmware */
      BAST_4528_CHK_RETCODE(BAST_4528_P_ResetAp(h));
      
      /* disable interrupts */
      BAST_4528_CHK_RETCODE(BAST_4528_P_DisableInterrupts(h));

      /* download to RAM */
      pImage = pHexImage;
      while (1)
      {
         n = (uint32_t)(*pImage++ << 16);
         n |= (uint32_t)(*pImage++ << 8);
         n |= (uint32_t)(*pImage++);

         if (n == 0)
            break;

         addr = (uint32_t)(*pImage++ << 16);
         addr |= (uint32_t)(*pImage++ << 8);
         addr |= (uint32_t)(*pImage++);

#ifdef BAST_VERIFY_DOWNLOAD
         if (pVerifyBuf)
            BKNI_Free((void*)pVerifyBuf);
         pVerifyBuf = (uint8_t*)BKNI_Malloc(n); 
         if (pVerifyBuf == NULL)
         {
            BDBG_ERR(("unable to allocate memory to verify code"));
         }
#endif

         for (retries = 0; retries < 3; retries++)
         {
            BDBG_MSG(("writing %d bytes starting from 0x%04X", n, addr));
            retCode = BAST_4528_WriteMemory(h, addr, (uint8_t*)pImage, n);
            if (retCode == BERR_SUCCESS)
            {
#ifdef BAST_VERIFY_DOWNLOAD
               if (pVerifyBuf)
               {
                  BDBG_MSG(("verifying %d bytes from 0x%04X", n, addr));
                  BAST_4528_CHK_RETCODE(BAST_4528_ReadMemory(h, addr, (uint8_t *)pVerifyBuf, n));
                  if (BKNI_Memcmp(pImage, pVerifyBuf, n) == 0)
                     break; 
                  BDBG_WRN(("data read back does not match\n"));
               }
#else
               break;
#endif
            }
            else
            {
               BDBG_ERR(("BAST_4528_WriteMemory() error 0x%X", retCode));
            }
         }
         if (retries >= 3)
         {
            BERR_TRACE(retCode = BAST_ERR_HOST_XFER);
            goto done;
         }

         pImage += n;
      }
   
#ifdef BAST_VERIFY_DOWNLOAD
      BDBG_MSG(("firmware loaded successfully in RAM"));
#endif

      /* clear init_done interrupt status */
      init_done_mask = BAST_4528_HIRQ0_INIT_DONE;
      BAST_4528_CHK_RETCODE(BAST_4528_WriteRbus(h, BCHP_LEAP_HOST_IRQ_CLEAR0, &init_done_mask, 1));

#ifndef BAST_DONT_USE_INTERRUPT
      /* enable init done interrupt */
      BAST_4528_CHK_RETCODE(BAST_4528_WriteRbus(h, BCHP_LEAP_HOST_IRQ_MASK_CLEAR0, &init_done_mask, 1));
      BKNI_WaitForEvent(pImpl->hInitDoneEvent, 0);
#endif

      /* start running the AP */
      BAST_4528_CHK_RETCODE(BAST_4528_P_RunAp(h));
      BDBG_MSG(("LEAP is running"));

#ifdef BAST_DONT_USE_INTERRUPT
      /* poll for AP init done */
      for (n = 0; n < 1000; n++) 
      {
         BAST_4528_CHK_RETCODE(BAST_4528_ReadRbus(h, BCHP_LEAP_HOST_IRQ_STATUS0, &val32, 1));
         if (val32 & BAST_4528_HIRQ0_INIT_DONE)
            break;
         BKNI_Sleep(1);
      }
      if ((val32 & BAST_4528_HIRQ0_INIT_DONE) == 0)
      {
         BDBG_ERR(("AP initialization timeout\n"));
         BERR_TRACE(retCode = BAST_ERR_AP_NOT_INIT);
         goto done;
      }
#else
      /* wait for init done interrupt */
      if (BAST_4528_P_WaitForEvent(h, pImpl->hInitDoneEvent, 1000) != BERR_SUCCESS)
      {
         BDBG_ERR(("AP initialization timeout\n")); 
         BERR_TRACE(retCode = BAST_ERR_AP_NOT_INIT);  
         goto done;
      }
      BAST_4528_CHK_RETCODE(BAST_4528_WriteRbus(h, BCHP_LEAP_HOST_IRQ_MASK_SET0, &init_done_mask, 1));
#endif
   }
   else if (strap == 1)
   {
      /* boot mode 1 */
      for (n = 0; n < 100; n++)
      {
         BAST_4528_CHK_RETCODE(BAST_4528_ReadRbus(h, BCHP_LEAP_HOST_IRQ_STATUS0, &val32, 1));
         if (val32 & BAST_4528_HIRQ0_INIT_DONE)
            break;
         BKNI_Sleep(1);
      }
      if ((val32 & BAST_4528_HIRQ0_INIT_DONE) == 0)
      {
         BDBG_ERR(("AP initialization timeout\n"));
         BERR_TRACE(retCode = BAST_ERR_AP_NOT_INIT);
         goto done;
      }      
   }
   else
   {
      BDBG_ERR(("Unsupported boot mode (%d)", strap));
      BERR_TRACE(retCode = BAST_ERR_AP_NOT_INIT);
      goto done;
   }
   
   retCode = BAST_SetNetworkSpec(h, h->settings.networkSpec);
   if (retCode != BERR_SUCCESS)
   {
      BDBG_ERR(("BAST_SetNetworkSpec() error 0x%X", retCode));
   }
   
   done:
#ifdef BAST_VERIFY_DOWNLOAD
   if (pVerifyBuf)
      BKNI_Free((void*)pVerifyBuf);
#endif
   if (retCode == BERR_SUCCESS)
   {
      BDBG_MSG(("LEAP initialized successfully"));
   }
   return retCode;
#endif /* BAST_USE_HAB_PI */   
}


/******************************************************************************
 BAST_4528_P_GetApStatus()
******************************************************************************/
BERR_Code BAST_4528_P_GetApStatus(
   BAST_Handle h,           /* [in] BAST device handle */
   BAST_ApStatus *pStatus   /* [out] AP status */
)
{
#ifdef BAST_USE_HAB_PI
   BAST_4528_P_Handle *pImpl = (BAST_4528_P_Handle *)(h->pImpl);
   return BHAB_GetApStatus(pImpl->hHab, pStatus);
#else
   BERR_Code retCode;
   uint32_t spare0, hab_request;
   uint8_t csr_status;

   *pStatus = 0;

   BAST_4528_CHK_RETCODE(BAST_4528_ReadBbsi(h,  BCHP_CSR_STATUS, &csr_status, 1));
   if ((csr_status & BCHP_CSR_STATUS_CPU_RUNNING_MASK) == 0)
      *pStatus |= BAST_APSTATUS_RESET;
   else
   {
      /* AP is running */
      BAST_4528_CHK_RETCODE(BAST_4528_ReadRbus(h, BCHP_LEAP_CTRL_SW_SPARE0, &spare0, 1));
      if (spare0 & BAST_4528_SPARE0_INIT_DONE) 
      {
         /* AP has initialized */
         *pStatus |= BAST_APSTATUS_INIT_DONE;

         BAST_4528_CHK_RETCODE(BAST_4528_ReadRbus(h, BCHP_LEAP_CTRL_HAB_REQUEST, &hab_request, 1));
         if ((hab_request & 1) == 0)
            *pStatus |= BAST_APSTATUS_HAB_READY;
      }
   }
   
   done:
   return retCode;
#endif   
}


/******************************************************************************
 BAST_4528_P_SoftReset()
******************************************************************************/
BERR_Code BAST_4528_P_SoftReset(
   BAST_Handle h           /* [in] BAST device handle */
)
{
#ifdef BAST_USE_HAB_PI
   BAST_4528_P_Handle *pImpl = (BAST_4528_P_Handle *)(h->pImpl);
#endif
   BERR_Code retCode;
   uint8_t buf[3];
   
   buf[0] = 0x05;
   buf[1] = 0x00;
#ifdef BAST_USE_HAB_PI
   BAST_4528_CHK_RETCODE(BHAB_SendHabCommand(pImpl->hHab, buf, 3, buf, 3, true, true, 3));
#else
   BAST_4528_CHK_RETCODE(BAST_4528_P_SendHabCommand(h, buf, 3, buf, true));
#endif

   BKNI_Sleep(10); /* wait for soft reset to complete */

   done:
   return retCode;
}


/******************************************************************************
 BAST_4528_P_GetApVersion()
******************************************************************************/
BERR_Code BAST_4528_P_GetApVersion(
   BAST_Handle h,          /* [in] BAST handle */
   uint16_t    *pChipId,   /* [out] BAST chip ID */
   uint8_t     *pChipVer,  /* [out] chip revision number */
   uint32_t    *pBondId,   /* [out] chip bonding option */
   uint8_t     *pApFwMajVer,   /* [out] AP firmware major version */
   uint8_t     *pApFwMinVer    /* [out] AP firmware minor version */
)
{
   BERR_Code retCode;
#ifdef BAST_USE_HAB_PI
   BAST_4528_P_Handle *pImpl = (BAST_4528_P_Handle *)(h->pImpl);
   uint32_t familyId, chipId;
   uint16_t chipVer;
   uint8_t majVer, minVer;
#else
   uint8_t buf[10];
#endif   

#ifdef BAST_USE_HAB_PI
   BAST_4528_CHK_RETCODE(BHAB_GetApVersion(pImpl->hHab, &familyId, &chipId, &chipVer, &majVer, &minVer));
   *pChipId = (uint16_t)familyId;
   *pChipVer = (uint8_t)chipVer;
   *pBondId = 0;
   *pApFwMajVer = majVer;
   *pApFwMinVer = minVer;
#else  
   buf[0] = 0x03;   
   BAST_4528_CHK_RETCODE(BAST_4528_P_SendHabCommand(h, buf, 10, buf, true));
   *pChipId = (uint16_t)((uint16_t)(buf[3] << 8) | buf[4]);
   *pChipVer = buf[6];
   *pBondId = 0;
   *pApFwMajVer = buf[7];
   *pApFwMinVer = buf[8];
#endif

   done:
   return retCode;
}


/******************************************************************************
 BERR_Code BAST_4528_P_ConfigGpio(
******************************************************************************/
BERR_Code BAST_4528_P_ConfigGpio(
   BAST_Handle h,           /* [in] BAST handle */
   uint32_t    write_mask,  /* [in] selects which GPIO pins are output */
   uint32_t    read_mask    /* [in] selects which GPIO pins are input */
)
{
#ifdef BAST_USE_HAB_PI
   BAST_4528_P_Handle *pImpl = (BAST_4528_P_Handle *)(h->pImpl);
#endif   
   BERR_Code retCode;
   uint8_t buf[10];
   
   buf[0] = 0x23;
   buf[1] = (uint8_t)((write_mask >> 24) & 0xFF);
   buf[2] = (uint8_t)((write_mask >> 16) & 0xFF);
   buf[3] = (uint8_t)((write_mask >> 8) & 0xFF);
   buf[4] = (uint8_t)(write_mask & 0xFF);
   buf[5] = (uint8_t)((read_mask >> 24) & 0xFF);
   buf[6] = (uint8_t)((read_mask >> 16) & 0xFF);
   buf[7] = (uint8_t)((read_mask >> 8) & 0xFF);
   buf[8] = (uint8_t)(read_mask & 0xFF);
#ifdef BAST_USE_HAB_PI
   BAST_4528_CHK_RETCODE(BHAB_SendHabCommand(pImpl->hHab, buf, 10, buf, 10, true, true, 10));
#else   
   BAST_4528_CHK_RETCODE(BAST_4528_P_SendHabCommand(h, buf, 10, buf, true));
#endif

   done:
   return retCode;
}


/******************************************************************************
 BERR_Code BAST_4528_P_SetGpio(
******************************************************************************/
BERR_Code BAST_4528_P_SetGpio(
   BAST_Handle h,     /* [in] BAST handle */
   uint32_t    mask,  /* [in] selects which GPIO pins to set */
   uint32_t    state  /* [in] state of GPIO pins */
)
{
#ifdef BAST_USE_HAB_PI
   BAST_4528_P_Handle *pImpl = (BAST_4528_P_Handle *)(h->pImpl);
#endif   
   BERR_Code retCode;
   uint8_t buf[10];
   
   buf[0] = 0x24;
   buf[1] = (uint8_t)((mask >> 24) & 0xFF);
   buf[2] = (uint8_t)((mask >> 16) & 0xFF);
   buf[3] = (uint8_t)((mask >> 8) & 0xFF);
   buf[4] = (uint8_t)(mask & 0xFF);
   buf[5] = (uint8_t)((state >> 24) & 0xFF);
   buf[6] = (uint8_t)((state >> 16) & 0xFF);
   buf[7] = (uint8_t)((state >> 8) & 0xFF);
   buf[8] = (uint8_t)(state & 0xFF);
#ifdef BAST_USE_HAB_PI
   BAST_4528_CHK_RETCODE(BHAB_SendHabCommand(pImpl->hHab, buf, 10, buf, 10, true, true, 10));
#else   
   BAST_4528_CHK_RETCODE(BAST_4528_P_SendHabCommand(h, buf, 10, buf, true));
#endif

   done:
   return retCode;
}


/******************************************************************************
 BERR_Code BAST_4528_P_GetGpio(
******************************************************************************/
BERR_Code BAST_4528_P_GetGpio(
   BAST_Handle h,     /* [in] BAST handle */
   uint32_t    mask,  /* [in] selects which GPIO pins to read */
   uint32_t    *state  /* [out] state of GPIO pins */
)
{
#ifdef BAST_USE_HAB_PI
   BAST_4528_P_Handle *pImpl = (BAST_4528_P_Handle *)(h->pImpl);
#endif   
   BERR_Code retCode;
   uint8_t buf[10];
   
   buf[0] = 0x25;
   buf[1] = (uint8_t)((mask >> 24) & 0xFF);
   buf[2] = (uint8_t)((mask >> 16) & 0xFF);
   buf[3] = (uint8_t)((mask >> 8) & 0xFF);
   buf[4] = (uint8_t)(mask & 0xFF);
#ifdef BAST_USE_HAB_PI
   BAST_4528_CHK_RETCODE(BHAB_SendHabCommand(pImpl->hHab, buf, 10, buf, 10, true, true, 10));
#else   
   BAST_4528_CHK_RETCODE(BAST_4528_P_SendHabCommand(h, buf, 10, buf, true));
#endif

   *state = (buf[5] << 24) | (buf[6] << 16) | (buf[7] << 8) | buf[8];

   done:
   return retCode;
}


/******************************************************************************
 BAST_4528_P_TuneAcquire()
******************************************************************************/
BERR_Code BAST_4528_P_TuneAcquire(
   BAST_ChannelHandle h,            /* [in] BAST channel handle */
   const uint32_t     freq,         /* [in] RF tuner frequency in Hz */
   const BAST_AcqSettings *pParams  /* [in] acquisition parameters */
)
{
#ifdef BAST_USE_HAB_PI
   BAST_4528_P_Handle *pDevImpl = (BAST_4528_P_Handle *)(h->pDevice->pImpl);
#endif      
   BAST_4528_P_ChannelHandle *pImpl = (BAST_4528_P_ChannelHandle *)(h->pImpl);   
   BERR_Code retCode;
   BAST_Mode mode;
   uint8_t buf[20];

   if ((pParams->symbolRate < 256000UL) || (pParams->symbolRate > 45000000UL))
      return (BERR_TRACE(BERR_INVALID_PARAMETER));

   if (freq > 2450000000UL)
      return (BERR_TRACE(BERR_INVALID_PARAMETER));

   mode = pParams->mode;
   if ((mode == BAST_Mode_eUnknown) ||
       ((mode > BAST_Mode_eDcii_4_5) && (mode < BAST_Mode_eLdpc_Qpsk_1_2)) ||
       (mode > BAST_Mode_eBlindScan) ||
       ((mode > BAST_Mode_eTurbo_8psk_8_9) && (mode < BAST_Mode_eBlindScan)))
      return (BERR_TRACE(BERR_INVALID_PARAMETER));

   if (BAST_MODE_IS_TURBO(mode))
      return (BERR_TRACE(BERR_NOT_SUPPORTED));

   BAST_4528_CHK_RETCODE(BAST_4528_P_EnableLockInterrupt(h, false));

   buf[0] = 0x0A;
   buf[1] = h->channel; 
   buf[2] = (uint8_t)mode;
   buf[3] = (uint8_t)((freq >> 24) & 0xFF); 
   buf[4] = (uint8_t)((freq >> 16) & 0xFF); 
   buf[5] = (uint8_t)((freq >> 8) & 0xFF); 
   buf[6] = (uint8_t)(freq & 0xFF); 
   buf[7] = (uint8_t)((pParams->symbolRate >> 24) & 0xFF);
   buf[8] = (uint8_t)((pParams->symbolRate >> 16) & 0xFF);
   buf[9] = (uint8_t)((pParams->symbolRate >> 8) & 0xFF);
   buf[10] = (uint8_t)(pParams->symbolRate & 0xFF);
   buf[11] = (uint8_t)((pParams->carrierFreqOffset >> 24) & 0xFF); 
   buf[12] = (uint8_t)((pParams->carrierFreqOffset >> 16) & 0xFF); 
   buf[13] = (uint8_t)((pParams->carrierFreqOffset >> 8) & 0xFF); 
   buf[14] = (uint8_t)(pParams->carrierFreqOffset & 0xFF); 
   buf[15] = (uint8_t)((pParams->acq_ctl >> 24) & 0xFF);
   buf[16] = (uint8_t)((pParams->acq_ctl >> 16) & 0xFF);
   buf[17] = (uint8_t)((pParams->acq_ctl >> 8) & 0xFF);
   buf[18] = (uint8_t)(pParams->acq_ctl & 0xFF);
#ifdef BAST_USE_HAB_PI
   BAST_4528_CHK_RETCODE(BHAB_SendHabCommand(pDevImpl->hHab, buf, 20, buf, 20, true, true, 20));
#else      
   BAST_4528_CHK_RETCODE(BAST_4528_P_SendHabCommand(h->pDevice, buf, 20, buf, true));
#endif

   pImpl->lastMode = mode;
   BAST_4528_CHK_RETCODE(BAST_4528_P_EnableLockInterrupt(h, true));

   done:
   return retCode;
}


/******************************************************************************
 BAST_4528_P_GetChannelStatus()
******************************************************************************/
BERR_Code BAST_4528_P_GetChannelStatus(
   BAST_ChannelHandle h,        /* [in] BAST handle */
   BAST_ChannelStatus *pStatus  /* [out] pointer to status structure */
)
{
#ifdef BAST_USE_HAB_PI
   BAST_4528_P_Handle *pDevImpl = (BAST_4528_P_Handle *)(h->pDevice->pImpl);
#endif         
   BAST_4528_P_ChannelHandle *pImpl = (BAST_4528_P_ChannelHandle *)(h->pImpl);
   BERR_Code retCode;
   BAST_Mode mode;
   bool bLdpcStatus, bRetry = false;
   uint8_t buf[128];
   
   if (BAST_MODE_IS_LDPC(pImpl->lastMode))
   {
      /* get ldpc status */
      get_ldpc_status:
      bLdpcStatus = true;
      buf[0] = 0x0D;
      buf[1] = h->channel;
#ifdef BAST_USE_HAB_PI
      BAST_4528_CHK_RETCODE(BHAB_SendHabCommand(pDevImpl->hHab, buf, 87, buf, 87, true, true, 87));
#else            
      BAST_4528_CHK_RETCODE(BAST_4528_P_SendHabCommand(h->pDevice, buf, 87, buf, true));
#endif
      mode = (BAST_Mode)buf[3];
      if (BAST_MODE_IS_LEGACY_QPSK(mode) && !bRetry)
      {
         bRetry = true;
         goto get_qpsk_status;
      }
   }
   else
   {
      /* get qpsk status */
      get_qpsk_status:
      bLdpcStatus = false;
      buf[0] = 0x0B;
      buf[1] = h->channel;
#ifdef BAST_USE_HAB_PI
      BAST_4528_CHK_RETCODE(BHAB_SendHabCommand(pDevImpl->hHab, buf, 81, buf, 81, true, true, 81));
#else                  
      BAST_4528_CHK_RETCODE(BAST_4528_P_SendHabCommand(h->pDevice, buf, 81, buf, true));
#endif
      mode = (BAST_Mode)buf[3];
      if (BAST_MODE_IS_LDPC(mode) && !bRetry)
      {
         bRetry = true;
         goto get_ldpc_status;
      }
   }

   pStatus->bTunerLocked = (buf[2] & 0x01) ? true : false;
   pStatus->bDemodLocked = (buf[2] & 0x02) ? true : false;
   pStatus->bBertLocked = (buf[2] & 0x04) ? true : false;
   pStatus->mode = mode;
   pStatus->symbolRate = (buf[4] << 24) | (buf[5] << 16) | (buf[6] << 8) | buf[7];
   pStatus->tunerFreq = (buf[8] << 24) | (buf[9] << 16) | (buf[10] << 8) | buf[11];
   pStatus->carrierOffset = (int32_t)((buf[12] << 24) | (buf[13] << 16) | (buf[14] << 8) | buf[15]);
   pStatus->carrierError = (int32_t)((buf[16] << 24) | (buf[17] << 16) | (buf[18] << 8) | buf[19]);
   pStatus->symbolRateError = (int32_t)((buf[20] << 24) | (buf[21] << 16) | (buf[22] << 8) | buf[23]);
   pStatus->snrEstimate = (buf[24] << 8) | buf[25];
   pStatus->mpegCount = (buf[26] << 24) | (buf[27] << 16) | (buf[28] << 8) | buf[29];
   pStatus->mpegErrors = (buf[30] << 24) | (buf[31] << 16) | (buf[32] << 8) | buf[33];
   pStatus->berErrors = (buf[34] << 24) | (buf[35] << 16) | (buf[36] << 8) | buf[37];
   pStatus->outputBitrate = (buf[38] << 24) | (buf[39] << 16) | (buf[40] << 8) | buf[41];
   pStatus->reacqCount = (buf[42] << 24) | (buf[43] << 16) | (buf[44] << 8) | buf[45];
   pStatus->sample_clock = (buf[46] << 24) | (buf[47] << 16) | (buf[48] << 8) | buf[49];

   /* TBD */
   pStatus->tunerFilter = 0;
   pStatus->IFagc = 0;
   pStatus->RFagc = 0;
   pStatus->agf = 0;

   if (bLdpcStatus && (BAST_MODE_IS_LDPC(mode)))
   {
      pStatus->modeStatus.ldpc.totalBlocks = (buf[66] << 24) | (buf[67] << 16) | (buf[68] << 8) | buf[69];
      pStatus->modeStatus.ldpc.corrBlocks = (buf[70] << 24) | (buf[71] << 16) | (buf[72] << 8) | buf[73];
      pStatus->modeStatus.ldpc.badBlocks = (buf[74] << 24) | (buf[75] << 16) | (buf[76] << 8) | buf[77];
      pStatus->modeStatus.ldpc.hpstatus = (buf[78] << 24) | (buf[79] << 16) | (buf[80] << 8) | buf[81];
      pStatus->modeStatus.ldpc.fec_clock = (buf[82] << 24) | (buf[83] << 16) | (buf[84] << 8) | buf[85];
   }
   else if (!bLdpcStatus && (BAST_MODE_IS_LEGACY_QPSK(mode)))
   {
      pStatus->modeStatus.legacy.rsCorrCount = (buf[66] << 24) | (buf[67] << 16) | (buf[68] << 8) | buf[69];
      pStatus->modeStatus.legacy.rsUncorrCount = (buf[70] << 24) | (buf[71] << 16) | (buf[72] << 8) | buf[73];
      pStatus->modeStatus.legacy.preVitErrCount = (buf[74] << 24) | (buf[75] << 16) | (buf[76] << 8) | buf[77];
      pStatus->modeStatus.legacy.spinv = (BAST_Spinv)buf[78];
      pStatus->modeStatus.legacy.phase = (BAST_PhaseRotation)buf[79];
   }
   else
   {
      BKNI_Memset(&(pStatus->modeStatus.ldpc), 0, sizeof(BAST_LdpcStatus));
   }

   done:
   return retCode;
}


/******************************************************************************
 BAST_4528_P_GetLockStatus()
******************************************************************************/
BERR_Code BAST_4528_P_GetLockStatus(
   BAST_ChannelHandle h,  /* [in] BAST handle */
   bool *bLocked          /* [out] pointer to status structure */
)
{
#ifdef BAST_USE_HAB_PI
   BAST_4528_P_Handle *pDevImpl = (BAST_4528_P_Handle *)(h->pDevice->pImpl);
#endif         
   BERR_Code retCode;
   uint32_t spare0, mask;

   mask = 1 << h->channel; 
#ifdef BAST_USE_HAB_PI
   BAST_4528_CHK_RETCODE(BHAB_ReadRegister(pDevImpl->hHab,  BCHP_LEAP_CTRL_SW_SPARE0, &spare0));
#else
   BAST_4528_CHK_RETCODE(BAST_4528_ReadRbus(h->pDevice,  BCHP_LEAP_CTRL_SW_SPARE0, &spare0, 1));
#endif
   if (spare0 & mask) 
      *bLocked = true;
   else
      *bLocked = false; 

   done:
   return retCode;
}


/******************************************************************************
 BAST_4528_P_ResetStatus()
******************************************************************************/
BERR_Code BAST_4528_P_ResetStatus(
   BAST_ChannelHandle h  /* [in] BAST channel handle */
)
{
#ifdef BAST_USE_HAB_PI
   BAST_4528_P_Handle *pDevImpl = (BAST_4528_P_Handle *)(h->pDevice->pImpl);
#endif         
   BERR_Code retCode;
   uint8_t buf[4];
   
   buf[0] = 0x0E;
   buf[1] = h->channel;
#ifdef BAST_USE_HAB_PI
   BAST_4528_CHK_RETCODE(BHAB_SendHabCommand(pDevImpl->hHab, buf, 3, buf, 3, true, true, 3));
#else                     
   BAST_4528_CHK_RETCODE(BAST_4528_P_SendHabCommand(h->pDevice, buf, 3, buf, true));
#endif

   done:
   return retCode;
}


/******************************************************************************
 BAST_4528_P_SetDiseqcTone()
******************************************************************************/
BERR_Code BAST_4528_P_SetDiseqcTone(
   BAST_ChannelHandle h,  /* [in] BAST channel handle */
   bool bTone             /* [in] true = tone on, false = tone off */
)
{
#ifdef BAST_USE_HAB_PI
   BAST_4528_P_Handle *pDevImpl = (BAST_4528_P_Handle *)(h->pDevice->pImpl);
#endif            
   BERR_Code retCode;
   uint8_t buf[3];
   
   buf[0] = 0x12;
   buf[1] = bTone ? 1 : 0;
#ifdef BAST_USE_HAB_PI
   BAST_4528_CHK_RETCODE(BHAB_SendHabCommand(pDevImpl->hHab, buf, 3, buf, 3, true, true, 3));
#else                        
   BAST_4528_CHK_RETCODE(BAST_4528_P_SendHabCommand(h->pDevice, buf, 3, buf, true));
#endif

   done:
   return retCode;
}


/******************************************************************************
 BAST_4528_P_GetDiseqcTone()
******************************************************************************/
BERR_Code BAST_4528_P_GetDiseqcTone(
   BAST_ChannelHandle h,  /* [in] BAST channel handle */
   bool *bTone            /* [out] true = tone present, false = tone absent */
)
{
#ifdef BAST_USE_HAB_PI
   BAST_4528_P_Handle *pDevImpl = (BAST_4528_P_Handle *)(h->pDevice->pImpl);
#endif               
   BERR_Code retCode;
   uint8_t buf[3];
   
   buf[0] = 0x13;
#ifdef BAST_USE_HAB_PI
   BAST_4528_CHK_RETCODE(BHAB_SendHabCommand(pDevImpl->hHab, buf, 3, buf, 3, true, true, 3));
#else                           
   BAST_4528_CHK_RETCODE(BAST_4528_P_SendHabCommand(h->pDevice, buf, 3, buf, true));
#endif

   *bTone = (buf[1] & 0x01) ? true : false;

   done:
   return retCode;
}


/******************************************************************************
 BAST_4528_P_SetDiseqcVoltage()
******************************************************************************/
BERR_Code BAST_4528_P_SetDiseqcVoltage(
   BAST_ChannelHandle h, /* [in] BAST channel handle */
   bool bVtop            /* [in] true = VTOP, false = VBOT */
)
{
#ifdef BAST_USE_HAB_PI
   BAST_4528_P_Handle *pDevImpl = (BAST_4528_P_Handle *)(h->pDevice->pImpl);
#endif                  
   BERR_Code retCode;
   uint8_t buf[3];
   
   buf[0] = 0x14;
   buf[1] = bVtop ? 1 : 0;
#ifdef BAST_USE_HAB_PI
   BAST_4528_CHK_RETCODE(BHAB_SendHabCommand(pDevImpl->hHab, buf, 3, buf, 3, true, true, 3));
#else                              
   BAST_4528_CHK_RETCODE(BAST_4528_P_SendHabCommand(h->pDevice, buf, 3, buf, true));
#endif

   done:
   return retCode;
}


/******************************************************************************
 BAST_4528_P_GetDiseqcVoltage()
******************************************************************************/
BERR_Code BAST_4528_P_GetDiseqcVoltage(
   BAST_ChannelHandle h, /* [in] BAST channel handle */
   uint8_t *pVoltage     /* [out] voltage estimation */
)
{
#ifdef BAST_USE_HAB_PI
   BAST_4528_P_Handle *pDevImpl = (BAST_4528_P_Handle *)(h->pDevice->pImpl);
#endif                     
   BERR_Code retCode;
   uint8_t buf[3];
   
   buf[0] = 0x15;
#ifdef BAST_USE_HAB_PI
   BAST_4528_CHK_RETCODE(BHAB_SendHabCommand(pDevImpl->hHab, buf, 3, buf, 3, true, true, 3));
#else                                 
   BAST_4528_CHK_RETCODE(BAST_4528_P_SendHabCommand(h->pDevice, buf, 3, buf, true));
#endif

   *pVoltage = buf[1];
   
   done:
   return retCode;
}


/******************************************************************************
 BAST_4528_P_EnableVsenseInterrupts()
******************************************************************************/
BERR_Code BAST_4528_P_EnableVsenseInterrupts(
   BAST_ChannelHandle h, 
   bool bEnable
)
{
#ifdef BAST_USE_HAB_PI
   BAST_4528_P_Handle *pDevImpl = (BAST_4528_P_Handle *)(h->pDevice->pImpl);
#endif                        
   BERR_Code retCode;
   uint32_t val32;
   uint8_t buf[3];
   
   val32 = BAST_4528_HIRQ0_DISEQC_OVERVOLTAGE | BAST_4528_HIRQ0_DISEQC_UNDERVOLTAGE; 
#ifdef BAST_USE_HAB_PI  
   BAST_4528_CHK_RETCODE(BHAB_WriteRegister(pDevImpl->hHab, BCHP_LEAP_HOST_IRQ_CLEAR0, &val32));  
   if (bEnable)
   {
      BAST_4528_CHK_RETCODE(BHAB_WriteRegister(pDevImpl->hHab, BCHP_LEAP_HOST_IRQ_MASK_CLEAR0, &val32));  
   }
   else
   {
      BAST_4528_CHK_RETCODE(BHAB_WriteRegister(pDevImpl->hHab, BCHP_LEAP_HOST_IRQ_MASK_SET0, &val32));  
   }
#else
   BAST_4528_CHK_RETCODE(BAST_4528_WriteRbus(h->pDevice, BCHP_LEAP_HOST_IRQ_CLEAR0, &val32, 1));  
   if (bEnable)
   {
      BAST_4528_CHK_RETCODE(BAST_4528_WriteRbus(h->pDevice, BCHP_LEAP_HOST_IRQ_MASK_CLEAR0, &val32, 1));  
   }
   else
   {
      BAST_4528_CHK_RETCODE(BAST_4528_WriteRbus(h->pDevice, BCHP_LEAP_HOST_IRQ_MASK_SET0, &val32, 1));  
   }
#endif
   /* enable/disable vsense interrupts in the LEAP */
   buf[0] = 0x1F;
   buf[1] = bEnable ? 1 : 0;
#ifdef BAST_USE_HAB_PI
   BAST_4528_CHK_RETCODE(BHAB_SendHabCommand(pDevImpl->hHab, buf, 3, buf, 3, true, true, 3));
#else                                    
   BAST_4528_CHK_RETCODE(BAST_4528_P_SendHabCommand(h->pDevice, buf, 3, buf, true));
#endif

   done:
   return retCode;
}


/******************************************************************************
 BAST_4528_P_SendDiseqcCommand()
******************************************************************************/
BERR_Code BAST_4528_P_SendDiseqcCommand(
   BAST_ChannelHandle h,    /* [in] BAST channel handle */
   const uint8_t *pSendBuf, /* [in] contains data to send */
   uint8_t sendBufLen       /* [in] number of bytes to send */
)
{
#ifdef BAST_USE_HAB_PI
   BAST_4528_P_Handle *pDevImpl = (BAST_4528_P_Handle *)(h->pDevice->pImpl);
#endif                           
   BERR_Code retCode = BERR_SUCCESS;
   uint8_t buf[128],i,len;
   uint32_t mask=0x80;

   buf[0] = 0x16;
   len=sendBufLen+3;
   if(sendBufLen > 124)
   {
      retCode = BERR_INVALID_PARAMETER;
      goto done;
   }
   buf[1] = sendBufLen & 0xFF;
   for(i=0;i<sendBufLen;i++)
   {
      buf[2+i]=pSendBuf[i];
   }
   buf[len-1] = 0x00;
   
   /* set up diseqc irq */
   BAST_4528_CHK_RETCODE(BAST_4528_P_WriteRegister(h, BCHP_LEAP_HOST_IRQ_MASK_CLEAR0, &mask));     
#ifdef BAST_USE_HAB_PI
   BAST_4528_CHK_RETCODE(BHAB_SendHabCommand(pDevImpl->hHab, buf, len, buf, len, true, true, len));
#else
   BAST_4528_CHK_RETCODE(BAST_4528_P_SendHabCommand(h->pDevice, buf, len, buf, true));
#endif

   done:
   return retCode;
}


/******************************************************************************
 BAST_4528_P_SetDiseqcSettings()
******************************************************************************/
BERR_Code BAST_4528_P_SetDiseqcSettings(BAST_ChannelHandle h, BAST_DiseqcSettings *pSettings)
{
#ifdef BAST_USE_HAB_PI
   BAST_4528_P_Handle *pDevImpl = (BAST_4528_P_Handle *)(h->pDevice->pImpl);
#endif                           
   BERR_Code retCode=BERR_SUCCESS;
   uint8_t buf[128], len;
   
   buf[0] = 0x1B;
   buf[1] = 0x00;
   buf[1] |= pSettings->bEnvelope?0x01:0;
   buf[1] |= pSettings->bToneAlign?0x02:0;
   buf[1] |= pSettings->bDisableRRTO?0x04:0;
   buf[1] |= pSettings->bEnableToneburst?0x08:0;
   buf[1] |= pSettings->bToneburstB?0x10:0;
   buf[1] |= pSettings->bOverrideFraming?0x20:0;
   buf[1] |= pSettings->bExpectReply?0x40:0;
   buf[1] |= pSettings->bEnableLNBPU?0x80:0;
   buf[2] = 0x00;
   buf[2] |= pSettings->bDisableRxOnly?0x01:0;
   buf[3] = (uint8_t)((pSettings->rrtoUsec >> 24) & 0xFF);
   buf[4] = (uint8_t)((pSettings->rrtoUsec >> 16) & 0xFF);
   buf[5] = (uint8_t)((pSettings->rrtoUsec >> 8) & 0xFF);
   buf[6] = (uint8_t)((pSettings->rrtoUsec) & 0xFF);
   buf[7] = (uint8_t)((pSettings->bitThreshold >> 8) & 0xFF);
   buf[8] = (uint8_t)((pSettings->bitThreshold) & 0xFF);
   buf[9] = (uint8_t)((pSettings->toneThreshold) & 0xFF);
   buf[10] = (uint8_t)((pSettings->preTxDelay) & 0xFF);
   buf[11] = (uint8_t)((pSettings->vsenseThresholdHi) & 0xFF);
   buf[12] = (uint8_t)((pSettings->vsenseThresholdLo) & 0xFF);
   buf[13] = 0x00;
   len=14;

#ifdef BAST_USE_HAB_PI
   BAST_4528_CHK_RETCODE(BHAB_SendHabCommand(pDevImpl->hHab, buf, len, buf, len, true, true, len));
#else                                       
   BAST_4528_CHK_RETCODE(BAST_4528_P_SendHabCommand(h->pDevice, buf, len, buf, true));
#endif

   done:
   return retCode;
}


/******************************************************************************
 BAST_4528_P_GetDiseqcSettings()
******************************************************************************/
BERR_Code BAST_4528_P_GetDiseqcSettings(BAST_ChannelHandle h, BAST_DiseqcSettings *pSettings)
{
#ifdef BAST_USE_HAB_PI
   BAST_4528_P_Handle *pDevImpl = (BAST_4528_P_Handle *)(h->pDevice->pImpl);
#endif                              
   BERR_Code retCode = BERR_SUCCESS;
   uint8_t buf[128], len;
   
   buf[0] = 0x1C;
   len = 14;

#ifdef BAST_USE_HAB_PI
   BAST_4528_CHK_RETCODE(BHAB_SendHabCommand(pDevImpl->hHab, buf, len, buf, len, true, true, len));
#else                                          
   BAST_4528_CHK_RETCODE(BAST_4528_P_SendHabCommand(h->pDevice, buf, len, buf, true));
#endif

   pSettings->bEnvelope=(buf[1] & 0x01)?true:false;
   pSettings->bToneAlign=(buf[1] & 0x02)?true:false;
   pSettings->bDisableRRTO=(buf[1] & 0x04)?true:false;
   pSettings->bEnableToneburst=(buf[1] & 0x08)?true:false;
   pSettings->bToneburstB=(buf[1] & 0x18)?true:false;
   pSettings->bOverrideFraming=(buf[1] & 0x20)?true:false;
   pSettings->bExpectReply=(buf[1] & 0x60)?true:false;
   pSettings->bEnableLNBPU=(buf[1] & 0x80)?true:false;
   pSettings->bDisableRxOnly=(buf[2] & 0x01)?true:false;

   pSettings->rrtoUsec=0;
   pSettings->rrtoUsec|=buf[3];
   pSettings->rrtoUsec<<=8;
   pSettings->rrtoUsec|=buf[4];
   pSettings->rrtoUsec<<=8;
   pSettings->rrtoUsec|=buf[5];
   pSettings->rrtoUsec<<=8;
   pSettings->rrtoUsec|=buf[6];
   pSettings->bitThreshold=0;
   pSettings->bitThreshold|=buf[7];
   pSettings->bitThreshold<<=8;
   pSettings->bitThreshold|=buf[8];
   pSettings->toneThreshold=buf[9];
   pSettings->preTxDelay=buf[10];
   pSettings->vsenseThresholdHi=buf[11];
   pSettings->vsenseThresholdLo=buf[12];
   
   done:
   return retCode;
}


/******************************************************************************
 BAST_4528_P_GetDiseqcStatus()
******************************************************************************/
BERR_Code BAST_4528_P_GetDiseqcStatus(
   BAST_ChannelHandle h,       /* [in] BAST channel handle */
   BAST_DiseqcStatus  *pStatus /* [out] status of last transaction */
)
{
#ifdef BAST_USE_HAB_PI
   BAST_4528_P_Handle *pDevImpl = (BAST_4528_P_Handle *)(h->pDevice->pImpl);
#endif                           
   BERR_Code retCode;
   uint8_t buf[32], i;

   /* send DISEQC_READ_RECEIVE_BUFFER */
   buf[0] = 0x17;
#ifdef BAST_USE_HAB_PI
   BAST_4528_CHK_RETCODE(BHAB_SendHabCommand(pDevImpl->hHab, buf, 21, buf, 21, true, true, 21));
#else                                          
   BAST_4528_CHK_RETCODE(BAST_4528_P_SendHabCommand(h->pDevice, buf, 21, buf, true));
#endif

   pStatus->status = (BAST_DiseqcSendStatus)buf[1];
   pStatus->bReplyExpected = buf[2] ? true : false;
   pStatus->nReplyBytes = buf[3];
   if (pStatus->nReplyBytes > 16)
      pStatus->nReplyBytes = 16;
   for (i = 0; i < 16; i++)
      pStatus->replyBuffer[i] = buf[4+i];

   done:
   return retCode;
}


/******************************************************************************
 BAST_ResetDiseqc()
******************************************************************************/ 
BERR_Code BAST_4528_P_ResetDiseqc(
   BAST_ChannelHandle h,    /* [in] BAST channel handle */
   uint8_t options          /* [in] reset options */
)
{
#ifdef BAST_USE_HAB_PI
   BAST_4528_P_Handle *pDevImpl = (BAST_4528_P_Handle *)(h->pDevice->pImpl);
#endif                           
   BERR_Code retCode;
   uint8_t buf[4];

   BSTD_UNUSED(options);

   buf[0] = 0x11;
   buf[1] = 0;
#ifdef BAST_USE_HAB_PI
   BAST_4528_CHK_RETCODE(BHAB_SendHabCommand(pDevImpl->hHab, buf, 3, buf, 3, true, true, 3));
#else                                          
   BAST_4528_CHK_RETCODE(BAST_4528_P_SendHabCommand(h->pDevice, buf, 3, buf, true));
#endif

   done:
   return retCode;
}


/******************************************************************************
 BAST_ResetFsk()
******************************************************************************/ 
BERR_Code BAST_4528_P_ResetFsk(
   BAST_Handle h    /* [in] BAST channel handle */
)
{
#ifdef BAST_USE_HAB_PI
   BAST_4528_P_Handle *pDevImpl = (BAST_4528_P_Handle *)(h->pImpl);
#endif             
   BERR_Code retCode;
   uint8_t buf[2];

   buf[0] = 0x18;
#ifdef BAST_USE_HAB_PI
   BAST_4528_CHK_RETCODE(BHAB_SendHabCommand(pDevImpl->hHab, buf, 2, buf, 2, true, true, 2));
#else                                          
   BAST_4528_CHK_RETCODE(BAST_4528_P_SendHabCommand(h, buf, 2, buf, true));
#endif

   /* turn on fsk interrupts */
   BAST_4528_CHK_RETCODE(BAST_4528_P_EnableFskInterrupt(h, true));

   done:
   return retCode;
}


/******************************************************************************
 BERR_Code BAST_4528_P_ReadFsk()
******************************************************************************/
BERR_Code BAST_4528_P_ReadFsk(
   BAST_Handle h,  /* [in] BAST handle */
   uint8_t *pBuf,  /* [out] data read */
   uint8_t *n      /* [out] length of data read */
)
{
#ifdef BAST_USE_HAB_PI
   BAST_4528_P_Handle *pDevImpl = (BAST_4528_P_Handle *)(h->pImpl);
#endif                              
   BERR_Code retCode;
   uint8_t buf[130], i, len, checksum;

   *n = 0;

   buf[0] = 0x19;
   buf[1] = 0x19;
#ifdef BAST_USE_HAB_PI
   BAST_4528_CHK_RETCODE(BHAB_SendHabCommand(pDevImpl->hHab, buf, 2, buf, 2, true, false, 2));
#else                                             
   BAST_4528_CHK_RETCODE(BAST_4528_P_SendHabCommand(h, buf, 2, buf, false));
#endif
   checksum = buf[0] + buf[1];
   len = buf[1] & 0x7F;

   if (len)
   {
      /* read the next len bytes in the HAB */
#ifdef BAST_USE_HAB_PI
      BAST_4528_CHK_RETCODE(BHAB_ReadMemory(pDevImpl->hHab, BCHP_LEAP_HAB_MEM_WORDi_ARRAY_BASE+2, &buf[2], (uint32_t)(len+1)));
#else
      BAST_4528_CHK_RETCODE(BAST_4528_P_ReadHab(h, 2, &buf[2], (uint8_t)(len+1)));
#endif

      for (i = 0; i < len; i++)
         checksum += buf[2+i];

      if (checksum != buf[2+len])
      {
         BDBG_ERR(("Bad HAB checksum!\n"));
         BERR_TRACE(retCode = BAST_ERR_HAB_CHECKSUM);
         goto done;
      }

      *n = len + 1;
      for (i = 0; i < *n; i++)
         pBuf[i] = buf[i+2]; 
   }

   done:
   return retCode;
}


/******************************************************************************
 BERR_Code BAST_4528_P_WriteFsk()
******************************************************************************/
BERR_Code BAST_4528_P_WriteFsk(
   BAST_Handle h,  /* [in] BAST handle */
   uint8_t *pBuf,  /* [in] data to write */
   uint8_t n       /* [in] length of data to write */
)
{
#ifdef BAST_USE_HAB_PI
   BAST_4528_P_Handle *pDevImpl = (BAST_4528_P_Handle *)(h->pImpl);
#endif                                 
   BERR_Code retCode;
   uint8_t buf[128], i, len;

#ifndef BAST_SWM_DEBUG
   /* Per DirecTV, dont allow debug options in production code */
   if (n == 4)
   {
      if ((pBuf[0] == 0x83) && (pBuf[1] == 0x04))
      {
         /* this is the Set Options local message */
         /* dont allow option bits 1, 3, 4, and 14 */
         if ((pBuf[2] & 0x40) || (pBuf[3] & 0x1A))
         {
            BDBG_ERR(("specified SWM option bit(s) are not allowed in production code"));
            return BERR_NOT_SUPPORTED;
         }
      }
      else if ((pBuf[0] == 0x83) && (pBuf[1] == 0x02))
      {
         /* this is a "Set Rx Bit Mask" message */
         BDBG_ERR(("Set Rx Bit Mask SWM command not allowed in production code"));
         return BERR_NOT_SUPPORTED;
      }
   }
#endif

   buf[0] = 0x1A;
   for (i = 0; i < n; i++)
   {
      /* BDBG_MSG(("WriteFsk(): 0x%02X", pBuf[i])); */
      buf[1+i] = pBuf[i];
   }
   len = (uint8_t)(2+n);
#ifdef BAST_USE_HAB_PI
   BAST_4528_CHK_RETCODE(BHAB_SendHabCommand(pDevImpl->hHab, buf, len, buf, len, true, true, len));
#else                                                
   BAST_4528_CHK_RETCODE(BAST_4528_P_SendHabCommand(h, buf, len, buf, true));
#endif

   done:
   return retCode;
}


/******************************************************************************
 BAST_4528_P_PowerDownFsk()
******************************************************************************/
BERR_Code BAST_4528_P_PowerDownFsk(
   BAST_Handle h  /* [in] BAST handle */
)
{
   /* TBD */
   BSTD_UNUSED(h);
   return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BAST_4528_P_PowerUpFsk()
******************************************************************************/
BERR_Code BAST_4528_P_PowerUpFsk(
   BAST_Handle h  /* [in] BAST handle */
)
{
   /* TBD */
   BSTD_UNUSED(h);
   return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BAST_4528_P_WriteMi2c()
******************************************************************************/ 
BERR_Code BAST_4528_P_WriteMi2c(
   BAST_ChannelHandle h, /* [in] BAST channel handle */
   uint8_t slave_addr,   /* [in] address of the i2c slave device */
   uint8_t *i2c_buf,     /* [in] specifies the data to transmit */
   uint8_t n             /* [in] number of bytes to transmit after the i2c slave address */
)
{
#ifdef BAST_USE_HAB_PI
   BAST_4528_P_Handle *pDevImpl = (BAST_4528_P_Handle *)(h->pDevice->pImpl);
#endif               
   BERR_Code retCode;
   uint8_t buf[128], i, len;
   
   if ((n == 0) || (n > 8))
      return BERR_INVALID_PARAMETER;

   buf[0] = 0x26;
   buf[1] = h->channel;
   buf[2] = 0;
   buf[3] = n;
   buf[4] = slave_addr & 0xFE;
   for (i = 0; i < n; i++)
      buf[5+i] = i2c_buf[i];
   len = (uint8_t)(6+n);
#ifdef BAST_USE_HAB_PI
   BAST_4528_CHK_RETCODE(BHAB_SendHabCommand(pDevImpl->hHab, buf, len, buf, len, true, true, len));
#else                                                   
   BAST_4528_CHK_RETCODE(BAST_4528_P_SendHabCommand(h->pDevice, buf, len, buf, true));
#endif

   switch (buf[2])
   {
      case 0:
         retCode = BERR_SUCCESS;
         break;

      case 1:
         retCode = BAST_ERR_MI2C_NO_ACK;
         break;

      case 2:
         retCode = BAST_ERR_MI2C_BUSY;
         break;

      default:
         retCode = BAST_ERR_HAB_FAIL;
         break;
   }

   done:
   return retCode;
}


/******************************************************************************
 BAST_4528_P_ReadMi2c()
******************************************************************************/
BERR_Code BAST_4528_P_ReadMi2c(
   BAST_ChannelHandle h, /* [in] BAST channel handle */
   uint8_t slave_addr,   /* [in] address of the i2c slave device */
   uint8_t *out_buf,     /* [in] specifies the data to transmit before the i2c restart condition */
   uint8_t out_n,        /* [in] number of bytes to transmit before the i2c restart condition not including the i2c slave address */
   uint8_t *in_buf,      /* [out] holds the data read */
   uint8_t in_n          /* [in] number of bytes to read after the i2c restart condition not including the i2c slave address */
)
{
#ifdef BAST_USE_HAB_PI
   BAST_4528_P_Handle *pDevImpl = (BAST_4528_P_Handle *)(h->pDevice->pImpl);
#endif                  
   BERR_Code retCode;
   uint8_t buf[128], i, len;
   
   if ((out_n > 8) || (in_n > 8) || (in_n == 0))
      return BERR_INVALID_PARAMETER;

   buf[0] = 0x27;
   buf[1] = h->channel;
   buf[2] = 0;
   buf[3] = out_n;
   buf[4] = in_n;
   buf[5] = slave_addr & 0xFE;
   for (i = 0; i < out_n; i++)
      buf[6+i] = out_buf[i];

   len = 7 + out_n + in_n;
#ifdef BAST_USE_HAB_PI
   BAST_4528_CHK_RETCODE(BHAB_SendHabCommand(pDevImpl->hHab, buf, len, buf, len, true, true, len));
#else                                                      
   BAST_4528_CHK_RETCODE(BAST_4528_P_SendHabCommand(h->pDevice, buf, len, buf, true));
#endif

   for (i = 0; i < in_n; i++)
      in_buf[i] = buf[6+out_n+i];

   switch (buf[2])
   {
      case 0:
         retCode = BERR_SUCCESS;
         break;

      case 1:
         retCode = BAST_ERR_MI2C_NO_ACK;
         break;

      case 2:
         retCode = BAST_ERR_MI2C_BUSY;
         break;

      default:
         retCode = BAST_ERR_HAB_FAIL;
         break;
   }
   done:
   return retCode;
}


/******************************************************************************
 BAST_4528_P_GetSoftDecisionBuf()
******************************************************************************/
BERR_Code BAST_4528_P_GetSoftDecisionBuf(
   BAST_ChannelHandle h,  /* [in] BAST channel handle */
   int16_t *pI,           /* [out] 15 I-values */
   int16_t *pQ            /* [out] 15 Q-values */
)
{
#ifdef BAST_USE_HAB_PI
   BAST_4528_P_Handle *pDevImpl = (BAST_4528_P_Handle *)(h->pDevice->pImpl);
#endif                     
   BERR_Code retCode;
   uint32_t i;
   uint8_t buf[128];

   buf[0] = 0x20;
   buf[1] = h->channel;
#ifdef BAST_USE_HAB_PI
   BAST_4528_CHK_RETCODE(BHAB_SendHabCommand(pDevImpl->hHab, buf, 63, buf, 63, true, true, 63));
#else                                                         
   BAST_4528_CHK_RETCODE(BAST_4528_P_SendHabCommand(h->pDevice, buf, 63, buf, true));
#endif

   for (i = 0; i < 15; i++)
   {
      pI[i] = (int16_t)((buf[i*4+2] << 8) | buf[i*4+3]);
      pQ[i] = (int16_t)((buf[i*4+4] << 8) | buf[i*4+5]);
   }
   done:
   return retCode;
}


/******************************************************************************
 BERR_Code BAST_4528_P_FreezeEq()
******************************************************************************/
BERR_Code BAST_4528_P_FreezeEq(
   BAST_ChannelHandle h,  /* [in] BAST handle */
   bool bFreeze           /* [in] true = freeze, false = unfreeze */
)
{
   /* TBD */
   BSTD_UNUSED(h);
   BSTD_UNUSED(bFreeze);
   return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BAST_4528_P_DeepSleep()
******************************************************************************/
BERR_Code BAST_4528_P_DeepSleep(
   BAST_Handle h  /* [in] BAST handle */
)
{
   /* TBD */
   BSTD_UNUSED(h);
   return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BAST_4528_P_PowerDown()
******************************************************************************/
BERR_Code BAST_4528_P_PowerDown(
   BAST_ChannelHandle h,  /* [in] BAST handle */
   uint32_t options       /* [in] see BAST_CORE_* macros */
)
{
   /* TBD */
   BSTD_UNUSED(h);
   BSTD_UNUSED(options);
   return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BAST_4528_P_PowerUp()
******************************************************************************/
BERR_Code BAST_4528_P_PowerUp(
   BAST_ChannelHandle h,  /* [in] BAST channel handle */
   uint32_t options       /* [in] see BAST_CORE_* macros */
)
{
   /* TBD */
   BSTD_UNUSED(h);
   BSTD_UNUSED(options);
   return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BAST_4528_P_GetLockStateChangeEventHandle()
******************************************************************************/
BERR_Code BAST_4528_P_GetLockStateChangeEventHandle(
   BAST_ChannelHandle h,       /* [in] BAST channel handle */
   BKNI_EventHandle   *hEvent  /* [out] lock event handle */
)
{
   *hEvent = ((BAST_4528_P_ChannelHandle *)(h->pImpl))->hLockChangeEvent;
   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_4528_P_GetFskEventHandle()
******************************************************************************/
BERR_Code BAST_4528_P_GetFskEventHandle(
   BAST_Handle h,            /* [in] BAST handle */
   BKNI_EventHandle *hEvent  /* [out] lock event handle */
)
{
   *hEvent = ((BAST_4528_P_Handle *)(h->pImpl))->hFskEvent;
   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_4528_P_GetDiseqcEventHandle()
******************************************************************************/
BERR_Code BAST_4528_P_GetDiseqcEventHandle(
   BAST_ChannelHandle h,     /* [in] BAST channel handle */
   BKNI_EventHandle *hEvent  /* [out] lock event handle */
)
{
   *hEvent = ((BAST_4528_P_Handle *)(h->pDevice->pImpl))->hDiseqcEvent;
   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_4528_P_GetDiseqcVsenseEventHandle()
******************************************************************************/
BERR_Code BAST_4528_P_GetDiseqcVsenseEventHandles(
   BAST_ChannelHandle h, 
   BKNI_EventHandle *hDiseqcOverVoltageEvent, 
   BKNI_EventHandle *hDiseqcUnderVoltageEvent)
{
   *hDiseqcOverVoltageEvent = ((BAST_4528_P_Handle *)(h->pDevice->pImpl))->hDiseqcOverVoltageEvent;
   *hDiseqcUnderVoltageEvent = ((BAST_4528_P_Handle *)(h->pDevice->pImpl))->hDiseqcUnderVoltageEvent;   
   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_4528_P_ReadConfig()
******************************************************************************/
BERR_Code BAST_4528_P_ReadConfig(
   BAST_ChannelHandle h,   /* [in] BAST channel handle */
   uint16_t id,            /* [in] configuration parameter ID */
   uint8_t *p,             /* [out] buffer to hold the read data */
   uint8_t len             /* [in] number of bytes to read */
)
{
#ifdef BAST_USE_HAB_PI
   BAST_4528_P_Handle *pDevImpl = (BAST_4528_P_Handle *)(h->pDevice->pImpl);
#endif                        
   BERR_Code retCode;
   uint8_t   hab[32], i, n;

   if ((len != 1) && (len != 2) && (len != 4))
   {
      return BERR_TRACE(BERR_INVALID_PARAMETER);
   }

   hab[0] = 0x06;
   hab[1] = h->channel;
   hab[2] = (uint8_t)((id >> 8) & 0xFF);
   hab[3] = (uint8_t)(id & 0xFF);
   hab[4] = len;
   n = (uint8_t)(6+len);
#ifdef BAST_USE_HAB_PI
   BAST_4528_CHK_RETCODE(BHAB_SendHabCommand(pDevImpl->hHab, hab, n, hab, n, true, true, n));
#else                                                         
   BAST_4528_CHK_RETCODE(BAST_4528_P_SendHabCommand(h->pDevice, hab, n, hab, true));
#endif

   for (i = 0; i < len; i++)
      p[i] = hab[i+5];

   done:
   return retCode;
}


/******************************************************************************
 BAST_4528_P_WriteConfig()
******************************************************************************/
BERR_Code BAST_4528_P_WriteConfig(
   BAST_ChannelHandle h,   /* [in] BAST channel handle */
   uint16_t id,            /* [in] configuration parameter ID */
   uint8_t *p,             /* [in] data to write */
   uint8_t len             /* [in] number of bytes to write */
)
{
#ifdef BAST_USE_HAB_PI
   BAST_4528_P_Handle *pDevImpl = (BAST_4528_P_Handle *)(h->pDevice->pImpl);
#endif                           
   BERR_Code retCode;
   uint8_t   i, hab[32], n;

   if ((len != 1) && (len != 2) && (len != 4))
   {
      return BERR_TRACE(BERR_INVALID_PARAMETER);
   }

   hab[0] = 0x07;
   hab[1] = h->channel;
   hab[2] = (uint8_t)((id >> 8) & 0xFF);
   hab[3] = (uint8_t)(id & 0xFF);
   hab[4] = len;
   for (i = 0; i < len; i++)
      hab[i+5] = p[i];

   n = (uint8_t)(6+len);
#ifdef BAST_USE_HAB_PI
   BAST_4528_CHK_RETCODE(BHAB_SendHabCommand(pDevImpl->hHab, hab, n, hab, n, true, true, n));
#else                                                         
   BAST_4528_CHK_RETCODE(BAST_4528_P_SendHabCommand(h->pDevice, hab, n, hab, true));
#endif

   done:
   return retCode;
}


/******************************************************************************
 BAST_4528_P_AbortAcq()
******************************************************************************/
BERR_Code BAST_4528_P_AbortAcq(
   BAST_ChannelHandle h  /* [in] BAST channel handle */
)
{
#ifdef BAST_USE_HAB_PI
   BAST_4528_P_Handle *pDevImpl = (BAST_4528_P_Handle *)(h->pDevice->pImpl);
#endif                              
   uint8_t buf[4];

   buf[0] = 0x0F;
   buf[1] = h->channel;
#ifdef BAST_USE_HAB_PI
   return BHAB_SendHabCommand(pDevImpl->hHab, buf, 3, buf, 3, true, true, 3);
#else                                                            
   return BAST_4528_P_SendHabCommand(h->pDevice, buf, 3, buf, true);
#endif
}


/******************************************************************************
 BAST_4528_P_PeakScan()
******************************************************************************/
BERR_Code BAST_4528_P_PeakScan(
   BAST_ChannelHandle h,           /* [in] BAST channel handle */
   uint32_t           tunerFreq    /* [in] frequency in Hz */
)
{
   /* TBD */
   BSTD_UNUSED(h);
   BSTD_UNUSED(tunerFreq);
   return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BAST_4528_P_GetPeakScanStatus()
******************************************************************************/
BERR_Code BAST_4528_P_GetPeakScanStatus(
   BAST_ChannelHandle  h,          /* [in] BAST channel handle */
   BAST_PeakScanStatus *pStatus    /* [out] Diseqc event handle */
)
{
   /* TBD */
   BSTD_UNUSED(h);
   BSTD_UNUSED(pStatus);
   return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BAST_4528_P_GetPeakScanEventHandle()
******************************************************************************/
BERR_Code BAST_4528_P_GetPeakScanEventHandle(
   BAST_ChannelHandle h,                /* [in] BAST channel handle */
   BKNI_EventHandle  *hPeakScanEvent    /* [out] Peak scan done event handle */
)
{
   *hPeakScanEvent = ((BAST_4528_P_ChannelHandle *)(h->pImpl))->hPeakScanEvent;
   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_4528_P_EnableStatusInterrupts()
******************************************************************************/
BERR_Code BAST_4528_P_EnableStatusInterrupts(
   BAST_ChannelHandle h, 
   bool bEnable
)
{
#ifdef BAST_USE_HAB_PI
   BAST_4528_P_Handle *pDevImpl = (BAST_4528_P_Handle *)(h->pDevice->pImpl);
#endif                                 
   BERR_Code retCode;
   uint32_t mask, bit;

   bit = ((h->channel * 4) + 1);
   mask = 1 << bit;

#ifdef BAST_USE_HAB_PI
   BAST_4528_CHK_RETCODE(BHAB_WriteRegister(pDevImpl->hHab, BCHP_LEAP_HOST_IRQ_CLEAR1, &mask));
   if (bEnable)
   {
      BAST_4528_CHK_RETCODE(BHAB_WriteRegister(pDevImpl->hHab, BCHP_LEAP_HOST_IRQ_MASK_CLEAR1, &mask));
   }
   else
   {
      BAST_4528_CHK_RETCODE(BHAB_WriteRegister(pDevImpl->hHab, BCHP_LEAP_HOST_IRQ_MASK_SET1, &mask));
   }
#else
   BAST_4528_CHK_RETCODE(BAST_4528_WriteRbus(h->pDevice, BCHP_LEAP_HOST_IRQ_CLEAR1, &mask, 1));
   if (bEnable)
   {
      BAST_4528_CHK_RETCODE(BAST_4528_WriteRbus(h->pDevice, BCHP_LEAP_HOST_IRQ_MASK_CLEAR1, &mask, 1));
   }
   else
   {
      BAST_4528_CHK_RETCODE(BAST_4528_WriteRbus(h->pDevice, BCHP_LEAP_HOST_IRQ_MASK_SET1, &mask, 1));
   }
#endif   
   done:
   return retCode;
}


/******************************************************************************
 BAST_4528_P_GetStatusEventHandle()
******************************************************************************/
BERR_Code BAST_4528_P_GetStatusEventHandle(
   BAST_ChannelHandle h,        /* [in] BAST channel handle */
   BKNI_EventHandle  *hEvent    /* [out] status event handle */
)
{
   *hEvent = ((BAST_4528_P_ChannelHandle *)(h->pImpl))->hStatusEvent;
   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_4528_P_ConfigBcm3445()
******************************************************************************/
BERR_Code BAST_4528_P_ConfigBcm3445(
   BAST_Handle h,                  /* [in] BAST device handle */
   BAST_Bcm3445Settings *pSettings /* [in] BCM3445 configuration settings */
)
{
   /* TBD */
   BSTD_UNUSED(h);
   BSTD_UNUSED(pSettings);
   return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BAST_4528_P_MapBcm3445ToTuner()
******************************************************************************/
BERR_Code BAST_4528_P_MapBcm3445ToTuner(
   BAST_ChannelHandle        h,    /* [in] BAST channel handle */
   BAST_Mi2cChannel          mi2c, /* [in] specifies which BCM3445 */
   BAST_Bcm3445OutputChannel out   /* [in] BCM3445 output channel */
)
{
   /* TBD */
   BSTD_UNUSED(h);
   BSTD_UNUSED(mi2c);
   BSTD_UNUSED(out);
   return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BAST_4528_P_GetBcm3445Status()
******************************************************************************/
BERR_Code BAST_4528_P_GetBcm3445Status(
   BAST_ChannelHandle h,       /* [in] BAST channel handle */
   BAST_Bcm3445Status *pStatus /* [out] BCM3445 status struct */
)
{
   /* TBD */
   BSTD_UNUSED(h);
   BSTD_UNUSED(pStatus);
   return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BAST_4528_P_SetSearchRange()
******************************************************************************/
BERR_Code BAST_4528_P_SetSearchRange(BAST_Handle h, uint32_t searchRange)
{
#ifdef BAST_USE_HAB_PI
   BAST_4528_P_Handle *pDevImpl = (BAST_4528_P_Handle *)(h->pImpl);
#endif                                 
   BERR_Code retCode;
   uint8_t buf[12];
   
   buf[0] = 0x07;
   buf[1] = 0;
   buf[2] = 0;
   buf[3] = 1;
   buf[4] = 4;
   buf[5] = (uint8_t)((searchRange >> 24) & 0xFF);
   buf[6] = (uint8_t)((searchRange >> 16) & 0xFF);
   buf[7] = (uint8_t)((searchRange >> 8) & 0xFF); 
   buf[8] = (uint8_t)(searchRange & 0xFF);
#ifdef BAST_USE_HAB_PI
   BAST_4528_CHK_RETCODE(BHAB_SendHabCommand(pDevImpl->hHab, buf, 10, buf, 10, true, true, 10));
#else     
   BAST_4528_CHK_RETCODE(BAST_4528_P_SendHabCommand(h, buf, 10, buf, true));
#endif

   done:
   return retCode;
}


/******************************************************************************
 BAST_4528_P_GetSearchRange()
******************************************************************************/
BERR_Code BAST_4528_P_GetSearchRange(BAST_Handle h, uint32_t *pSearchRange)
{
#ifdef BAST_USE_HAB_PI
   BAST_4528_P_Handle *pDevImpl = (BAST_4528_P_Handle *)(h->pImpl);
#endif                                    
   BERR_Code retCode;
   uint8_t buf[12];

   *pSearchRange = 0;

   BKNI_Memset(buf, 0, 12);
   buf[0] = 0x06;
   buf[3] = 1;
   buf[4] = 4;
#ifdef BAST_USE_HAB_PI
   BAST_4528_CHK_RETCODE(BHAB_SendHabCommand(pDevImpl->hHab, buf, 10, buf, 10, true, true, 10));
#else        
   BAST_4528_CHK_RETCODE(BAST_4528_P_SendHabCommand(h, buf, 10, buf, true));
#endif

   *pSearchRange = (uint32_t)(buf[5] << 24) | (uint32_t)(buf[6] << 16) | (uint32_t)(buf[7] << 8) | (uint32_t)buf[8];

   done:
   return retCode;

}


/******************************************************************************
 BAST_4528_P_SetAmcScramblingSeq()
******************************************************************************/
BERR_Code BAST_4528_P_SetAmcScramblingSeq(BAST_ChannelHandle h, uint32_t xseed, uint32_t plhdrscr1, uint32_t plhdrscr2, uint32_t plhdrscr3)
{
#ifdef BAST_USE_HAB_PI
   BAST_4528_P_Handle *pDevImpl = (BAST_4528_P_Handle *)(h->pDevice->pImpl);
#endif                  
   BERR_Code retCode;
   uint8_t buf[20];

   buf[0] = 0x10;
   buf[1] = h->channel;
   buf[2] = (uint8_t)((xseed >> 24) & 0xFF);
   buf[3] = (uint8_t)((xseed >> 16) & 0xFF);
   buf[4] = (uint8_t)((xseed >> 8) & 0xFF);
   buf[5] = (uint8_t)(xseed & 0xFF);
   buf[6] = (uint8_t)((plhdrscr1 >> 24) & 0xFF);
   buf[7] = (uint8_t)((plhdrscr1 >> 16) & 0xFF);
   buf[8] = (uint8_t)((plhdrscr1 >> 8) & 0xFF);
   buf[9] = (uint8_t)(plhdrscr1 & 0xFF);
   buf[10] = (uint8_t)((plhdrscr2 >> 24) & 0xFF);
   buf[11] = (uint8_t)((plhdrscr2 >> 16) & 0xFF);
   buf[12] = (uint8_t)((plhdrscr2 >> 8) & 0xFF);
   buf[13] = (uint8_t)(plhdrscr2 & 0xFF);
   buf[14] = (uint8_t)((plhdrscr3 >> 24) & 0xFF);
   buf[15] = (uint8_t)((plhdrscr3 >> 16) & 0xFF);
   buf[16] = (uint8_t)((plhdrscr3 >> 8) & 0xFF);
   buf[17] = (uint8_t)(plhdrscr3 & 0xFF);
#ifdef BAST_USE_HAB_PI
   BAST_4528_CHK_RETCODE(BHAB_SendHabCommand(pDevImpl->hHab, buf, 19, buf, 19, true, true, 19));
#else           
   BAST_4528_CHK_RETCODE(BAST_4528_P_SendHabCommand(h->pDevice, buf, 19, buf, true));
#endif

   done:
   return retCode;
}


/******************************************************************************
 BAST_4528_P_SetTunerFilter()
******************************************************************************/
BERR_Code BAST_4528_P_SetTunerFilter(BAST_ChannelHandle h, uint32_t cutoffHz)
{
   /* TBD */
   BSTD_UNUSED(h);
   BSTD_UNUSED(cutoffHz);
   return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BAST_4528_P_GetSignalDetectStatus()
******************************************************************************/
BERR_Code BAST_4528_P_GetSignalDetectStatus(BAST_ChannelHandle h, BAST_SignalDetectStatus *pStatus)
{
   /* TBD */
   BSTD_UNUSED(h);
   BSTD_UNUSED(pStatus);
   return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BAST_4528_P_ResetChannel()
******************************************************************************/
BERR_Code BAST_4528_P_ResetChannel(BAST_ChannelHandle h, bool bForceReset)
{
#ifdef BAST_USE_HAB_PI
   BAST_4528_P_Handle *pDevImpl = (BAST_4528_P_Handle *)(h->pDevice->pImpl);
#endif                     
   uint8_t buf[4];

   buf[0] = 0x1D;
   buf[1] = h->channel;
   buf[2] = bForceReset ? 1: 0;
#ifdef BAST_USE_HAB_PI
   return BHAB_SendHabCommand(pDevImpl->hHab, buf, 4, buf, 4, true, true, 4);
#else              
   return BAST_4528_P_SendHabCommand(h->pDevice, buf, 4, buf, true);
#endif
}


/******************************************************************************
 BAST_4528_P_EnableDiseqcLnb()
******************************************************************************/
BERR_Code BAST_4528_P_EnableDiseqcLnb(BAST_ChannelHandle h, bool bEnable)
{
#ifdef BAST_USE_HAB_PI
   BAST_4528_P_Handle *pDevImpl = (BAST_4528_P_Handle *)(h->pDevice->pImpl);
#endif                        
   uint8_t buf[4];

   buf[0] = 0x14;
   buf[1] = bEnable ? 1 : 0;
#ifdef BAST_USE_HAB_PI
   return BHAB_SendHabCommand(pDevImpl->hHab, buf, 3, buf, 3, true, true, 3);
#else                 
   return BAST_4528_P_SendHabCommand(h->pDevice, buf, 3, buf, true);
#endif
}


/******************************************************************************
 BAST_4528_P_SetOutputTransportSettings()
******************************************************************************/
BERR_Code BAST_4528_P_SetOutputTransportSettings(BAST_ChannelHandle h, BAST_OutputTransportSettings *pSettings)
{
#ifdef BAST_USE_HAB_PI
   BAST_4528_P_Handle *pDevImpl = (BAST_4528_P_Handle *)(h->pDevice->pImpl);
#endif                           
   BERR_Code retCode;
   uint8_t buf[16];
   
   buf[0] = 0x07;
   buf[1] = h->channel;
   buf[2] = 0x00;
   buf[3] = 0x02;
   buf[4] = 4;
   buf[5] = buf[6] = buf[7] = buf[8] = 0;

   if (pSettings->bSerial)
      buf[8] |= 0x01;      
   if (pSettings->bClkInv)
      buf[8] |= 0x02; 
   if (pSettings->bClkSup)
      buf[8] |= 0x04; 
   if (pSettings->bVldInv)
      buf[8] |= 0x08;
   if (pSettings->bSyncInv)
      buf[8] |= 0x10;
   if (pSettings->bErrInv)
      buf[8] |= 0x20;
   if (pSettings->bXbert)
      buf[8] |= 0x40;
   if (pSettings->bTei)
      buf[8] |= 0x80;
   if (pSettings->bDelay)
      buf[7] |= 0x01;
   if (pSettings->bSync1)
      buf[7] |= 0x02;
   if (pSettings->bHead4)
      buf[7] |= 0x04;
   if (pSettings->bDelHeader)
      buf[7] |= 0x08;     
   if (pSettings->bOpllBypass)
      buf[7] |= 0x10;    
   buf[6] = (uint8_t)pSettings->bchMpegErrorMode;
#ifdef BAST_USE_HAB_PI
   BAST_4528_CHK_RETCODE(BHAB_SendHabCommand(pDevImpl->hHab, buf, 10, buf, 10, true, true, 10));
#else              
   BAST_4528_CHK_RETCODE(BAST_4528_P_SendHabCommand(h->pDevice, buf, 10, buf, true));
#endif

   done:
   return retCode;
}


/******************************************************************************
 BAST_4528_P_GetOutputTransportSettings()
******************************************************************************/
BERR_Code BAST_4528_P_GetOutputTransportSettings(BAST_ChannelHandle h, BAST_OutputTransportSettings *pSettings)
{
#ifdef BAST_USE_HAB_PI
   BAST_4528_P_Handle *pDevImpl = (BAST_4528_P_Handle *)(h->pDevice->pImpl);
#endif                              
   BERR_Code retCode;
   uint8_t buf[16];

   buf[0] = 0x06;
   buf[1] = h->channel;
   buf[2] = 0;
   buf[3] = 2;
   buf[4] = 4;
#ifdef BAST_USE_HAB_PI
   BAST_4528_CHK_RETCODE(BHAB_SendHabCommand(pDevImpl->hHab, buf, 10, buf, 10, true, true, 10));
#else                 
   BAST_4528_CHK_RETCODE(BAST_4528_P_SendHabCommand(h->pDevice, buf, 10, buf, true));
#endif

   if (buf[8] & 0x01)  
      pSettings->bSerial = true;
   else
      pSettings->bSerial = false;   
   if (buf[8] & 0x02) 
      pSettings->bClkInv = true; 
   else
      pSettings->bClkInv = false;
   if (buf[8] & 0x04)
      pSettings->bClkSup = true;
   else
      pSettings->bClkSup = false;   
   if (buf[8] & 0x08)
      pSettings->bVldInv = true;
   else
      pSettings->bVldInv = false;   
   if (buf[8] & 0x10)
      pSettings->bSyncInv = true;
   else
      pSettings->bSyncInv = false;   
   if (buf[8] & 0x20)
      pSettings->bErrInv = true;
   else
      pSettings->bErrInv = false;   
   if (buf[8] & 0x40)
      pSettings->bXbert = true;
   else
      pSettings->bXbert = false;   
   if (buf[8] & 0x80)
      pSettings->bTei = true;
   else
      pSettings->bTei = false;   
   if (buf[7] & 0x01)
      pSettings->bDelay = true;
   else
      pSettings->bDelay = false;   
   if (buf[7] & 0x02)
      pSettings->bSync1 = true;
   else
      pSettings->bSync1 = false;   
   if (buf[7] & 0x04)
      pSettings->bHead4 = true;
   else
      pSettings->bHead4 = false;   
   if (buf[7] & 0x08)
      pSettings->bDelHeader = true;    
   else
      pSettings->bDelHeader = false;   
   if (buf[7] & 0x10)
      pSettings->bOpllBypass = true;
   else
      pSettings->bOpllBypass = false; 

   buf[6] &= 0x03;
   pSettings->bchMpegErrorMode = (BAST_BchMpegErrorMode)buf[6];

   done:
   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_4528_P_SetNetworkSpec()
******************************************************************************/
BERR_Code BAST_4528_P_SetNetworkSpec(BAST_Handle h, BAST_NetworkSpec s)
{
#ifdef BAST_USE_HAB_PI
   BAST_4528_P_Handle *pDevImpl = (BAST_4528_P_Handle *)(h->pImpl);
#endif                                 
   BERR_Code retCode;
   uint8_t buf[8];
   
   buf[0] = 0x07;
   buf[1] = 0;
   buf[2] = 0;
   buf[3] = 0;
   buf[4] = 1;
   buf[5] = (uint8_t)s;
#ifdef BAST_USE_HAB_PI
   BAST_4528_CHK_RETCODE(BHAB_SendHabCommand(pDevImpl->hHab, buf, 7, buf, 7, true, true, 7));
#else                    
   BAST_4528_CHK_RETCODE(BAST_4528_P_SendHabCommand(h, buf, 7, buf, true));
#endif

   done:
   return retCode;
}


/******************************************************************************
 BAST_4528_P_GetNetworkSpec()
******************************************************************************/
BERR_Code BAST_4528_P_GetNetworkSpec(BAST_Handle h, BAST_NetworkSpec *p)
{
#ifdef BAST_USE_HAB_PI
   BAST_4528_P_Handle *pDevImpl = (BAST_4528_P_Handle *)(h->pImpl);
#endif                                    
   BERR_Code retCode;
   uint8_t buf[8];
   
   buf[0] = 0x06;
   buf[1] = 0;
   buf[2] = 0;
   buf[3] = 0;
   buf[4] = 1;
#ifdef BAST_USE_HAB_PI
   BAST_4528_CHK_RETCODE(BHAB_SendHabCommand(pDevImpl->hHab, buf, 7, buf, 7, true, true, 7));
#else                       
   BAST_4528_CHK_RETCODE(BAST_4528_P_SendHabCommand(h, buf, 7, buf, true));
#endif

   *p = (BAST_NetworkSpec)buf[5];

   done:
   return retCode;

}


/******************************************************************************
 BAST_4528_P_EnableLockInterrupt()
******************************************************************************/ 
BERR_Code BAST_4528_P_EnableLockInterrupt(
   BAST_ChannelHandle h, /* [in] BAST channel handle */
   bool bEnable          /* [in] true = enable lock interrupts, false = disables lock interrupts */
)
{
#ifdef BAST_USE_HAB_PI
   BAST_4528_P_Handle *pImpl = (BAST_4528_P_Handle *)(h->pDevice->pImpl);
#endif      
   BERR_Code retCode;
   uint32_t mask, bit;

   bit = h->channel * 4;
   mask = 1 << bit;

   /* clear the interrupt status */
#ifdef BAST_USE_HAB_PI
   BAST_4528_CHK_RETCODE(BHAB_WriteRegister(pImpl->hHab, BCHP_LEAP_HOST_IRQ_CLEAR1, &mask));
#else
   BAST_4528_CHK_RETCODE(BAST_4528_WriteRbus(h->pDevice, BCHP_LEAP_HOST_IRQ_CLEAR1, &mask, 1));
#endif

   if (bEnable)
   {
#ifdef BAST_USE_HAB_PI
      BAST_4528_CHK_RETCODE(BHAB_WriteRegister(pImpl->hHab, BCHP_LEAP_HOST_IRQ_MASK_CLEAR1, &mask));
#else      
      BAST_4528_CHK_RETCODE(BAST_4528_WriteRbus(h->pDevice, BCHP_LEAP_HOST_IRQ_MASK_CLEAR1, &mask, 1));
#endif
   }
   else
   {
#ifdef BAST_USE_HAB_PI
      BAST_4528_CHK_RETCODE(BHAB_WriteRegister(pImpl->hHab, BCHP_LEAP_HOST_IRQ_MASK_SET1, &mask));
#else            
      BAST_4528_CHK_RETCODE(BAST_4528_WriteRbus(h->pDevice, BCHP_LEAP_HOST_IRQ_MASK_SET1, &mask, 1));
#endif
   }

   done:
   return retCode;   
}


/******************************************************************************
 BAST_4528_P_EnableFskInterrupt()
******************************************************************************/
BERR_Code BAST_4528_P_EnableFskInterrupt(
   BAST_Handle h, /* [in] BAST handle */
   bool bEnable   /* [in] true = enable FTM interrupts, false = disables FTM interrupts */
)
{
#ifdef BAST_USE_HAB_PI
   BAST_4528_P_Handle *pImpl = (BAST_4528_P_Handle *)(h->pImpl);
#endif         
   BERR_Code retCode;
   uint32_t mask;

   mask = BAST_4528_HIRQ0_FTM_MSG_READY;
#ifdef BAST_USE_HAB_PI
   BAST_4528_CHK_RETCODE(BHAB_WriteRegister(pImpl->hHab, BCHP_LEAP_HOST_IRQ_CLEAR0, &mask));
   if (bEnable)
   {
      BAST_4528_CHK_RETCODE(BHAB_WriteRegister(pImpl->hHab, BCHP_LEAP_HOST_IRQ_MASK_CLEAR0, &mask));
   }
   else
   {
      BAST_4528_CHK_RETCODE(BHAB_WriteRegister(pImpl->hHab, BCHP_LEAP_HOST_IRQ_MASK_SET0, &mask));
   }
#else
   BAST_4528_CHK_RETCODE(BAST_4528_WriteRbus(h, BCHP_LEAP_HOST_IRQ_CLEAR0, &mask, 1));
   if (bEnable)
   {
      BAST_4528_CHK_RETCODE(BAST_4528_WriteRbus(h, BCHP_LEAP_HOST_IRQ_MASK_CLEAR0, &mask, 1));
   }
   else
   {
      BAST_4528_CHK_RETCODE(BAST_4528_WriteRbus(h, BCHP_LEAP_HOST_IRQ_MASK_SET0, &mask, 1));
   }
#endif

   done:
   return retCode; 
}


/******************************************************************************
 BAST_4528_P_GetRegisterAddress()
******************************************************************************/
void BAST_4528_P_GetRegisterAddress(BAST_ChannelHandle h, uint32_t reg, uint32_t *pAddr)
{
   *pAddr = reg;
   
   if (h->channel > 0)
   {
      if (((reg >= 0x000B0000) && (reg < 0x000B0D00)) || ((reg >= 0x000B4000) && (reg < 0x000B5000)))
      {
         /* SDS or AFEC register access */
         *pAddr = reg + (uint32_t)((uint32_t)h->channel * 0x8000);
      }
   }
}


/******************************************************************************
 BAST_4528_P_ReadRegister()
******************************************************************************/
BERR_Code BAST_4528_P_ReadRegister(
   BAST_ChannelHandle h,     /* [in] BAST channel handle */
   uint32_t           reg,   /* [in] address of register to read */
   uint32_t           *val   /* [in] contains data that was read */
)
{
#ifdef BAST_USE_HAB_PI
   BAST_4528_P_Handle *pDevImpl = (BAST_4528_P_Handle *)(h->pDevice->pImpl);
#endif                                 
   uint32_t addr;

   if (reg & 0x03)
      return BERR_INVALID_PARAMETER;
      
   BAST_4528_P_GetRegisterAddress(h, reg, &addr);
#ifdef BAST_USE_HAB_PI
   return BHAB_ReadRegister(pDevImpl->hHab, addr, val);
#else
   return BAST_4528_ReadRbus(h->pDevice, addr, val, 1);
#endif
}


/******************************************************************************
 BAST_4528_P_WriteRegister()
******************************************************************************/
BERR_Code BAST_4528_P_WriteRegister(
   BAST_ChannelHandle h,     /* [in] BAST channel handle */
   uint32_t           reg,   /* [in] address of register to read */
   uint32_t           *val   /* [in] contains data that was read */
)
{
#ifdef BAST_USE_HAB_PI
   BAST_4528_P_Handle *pDevImpl = (BAST_4528_P_Handle *)(h->pDevice->pImpl);
#endif                                    
   uint32_t addr;

   if (reg & 0x03)
      return BERR_INVALID_PARAMETER;

   BAST_4528_P_GetRegisterAddress(h, reg, &addr);
#ifdef BAST_USE_HAB_PI
   return BHAB_WriteRegister(pDevImpl->hHab, addr, val);
#else   
   return BAST_4528_WriteRbus(h->pDevice, addr, val, 1);
#endif
}


#ifdef BAST_USE_HAB_PI
/******************************************************************************
 BAST_4528_P_InterruptCallback()
******************************************************************************/
BERR_Code BAST_4528_P_InterruptCallback(void *pParm1, int parm2)
{
   BHAB_4528_IrqStatus *pParams = (BHAB_4528_IrqStatus *)pParm1;
   BAST_Handle h = (BAST_Handle)(pParams->pParm1);
   BAST_4528_P_Handle *pImpl = (BAST_4528_P_Handle *)(h->pImpl);   
   uint32_t fstatus0 = pParams->status0;
   uint32_t fstatus1 = pParams->status1;
   uint32_t channel, val32;
   BAST_4528_P_ChannelHandle *pChn;

   BSTD_UNUSED(parm2);
   if (fstatus0)
   {
      if (fstatus0 & BAST_4528_HIRQ0_FTM_MSG_READY)
         BKNI_SetEvent(pImpl->hFskEvent);
      
      if (fstatus0 & BAST_4528_HIRQ0_DISEQC_DONE)
         BKNI_SetEvent(pImpl->hDiseqcEvent);
      
      if (fstatus0 & BAST_4528_HIRQ0_DISEQC_OVERVOLTAGE)
         BKNI_SetEvent(pImpl->hDiseqcOverVoltageEvent);
      
      if (fstatus0 & BAST_4528_HIRQ0_DISEQC_UNDERVOLTAGE)
         BKNI_SetEvent(pImpl->hDiseqcUnderVoltageEvent);
   }

   if (fstatus1)
   {
      for (channel = 0; channel < BAST_4528_MAX_CHANNELS; channel++)
      {
         pChn = (BAST_4528_P_ChannelHandle *)(h->pChannels[channel]->pImpl);
         val32 = (fstatus1 >> (channel*4)) & 0x0F;
         if (val32 & BAST_4528_HIRQ1_CHAN0_LOCK_CHANGE) 
            BKNI_SetEvent(pChn->hLockChangeEvent);
         if (val32 & BAST_4528_HIRQ1_CHAN0_STATUS_EVENT) 
            BKNI_SetEvent(pChn->hStatusEvent);
         if (val32 & BAST_4528_HIRQ1_CHAN0_PEAK_SCAN_DONE) 
            BKNI_SetEvent(pChn->hPeakScanEvent);
      }
   }
   return BERR_SUCCESS;
}
#endif


#ifndef BAST_USE_HAB_PI
/******************************************************************************
 BAST_4528_P_GetInterruptEventHandle()
******************************************************************************/
BERR_Code BAST_4528_P_GetInterruptEventHandle(
   BAST_Handle h,            /* [in] BAST handle */
   BKNI_EventHandle *hEvent  /* [out] interrupt event handle */
)
{
   *hEvent = ((BAST_4528_P_Handle *)(h->pImpl))->hInterruptEvent;
   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_4528_P_WaitForBbsiDone()
******************************************************************************/
BERR_Code BAST_4528_P_WaitForBbsiDone(BAST_Handle h)
{
   BERR_Code retCode;
   uint32_t retry;
   uint8_t status;
   
   for (retry = 0; retry < BAST_MAX_BBSI_RETRIES; retry++)
   {
      BAST_4528_CHK_RETCODE(BAST_4528_ReadBbsi(h, BCHP_CSR_STATUS, &status, 1));
      if (((status & BCHP_CSR_STATUS_BUSY_MASK) == 0) || (status & 0x0F))
         break;
   }
   
   if (retry >= BAST_MAX_BBSI_RETRIES)
      retCode = BERR_TIMEOUT;
   else if (status & 0x0F)
   {
      BDBG_ERR(("CSR_STATUS error 0x%X", status));
      retCode = BAST_ERR_HOST_XFER; 
   }

   done:
   return retCode;
}


/******************************************************************************
 BAST_4528_P_CheckHab()
******************************************************************************/
BERR_Code BAST_4528_P_CheckHab(
   BAST_Handle h    /* [in] BAST handle */
)
{
   BERR_Code retCode = BAST_ERR_HAB_NOT_AVAIL;
   uint32_t spare0, hab_request;
   uint8_t csr_status;

   BAST_4528_CHK_RETCODE(BAST_4528_ReadBbsi(h,  BCHP_CSR_STATUS, &csr_status, 1));
   if (csr_status & BCHP_CSR_STATUS_CPU_RUNNING_MASK)
   {
      /* LEAP is running */
      BAST_4528_CHK_RETCODE(BAST_4528_ReadRbus(h, BCHP_LEAP_CTRL_SW_SPARE0, &spare0, 1));
      if (spare0 & BAST_4528_SPARE0_INIT_DONE) 
      {
         /* AP has initialized */
         BAST_4528_CHK_RETCODE(BAST_4528_ReadRbus(h, BCHP_LEAP_CTRL_HAB_REQUEST, &hab_request, 1));
         if ((hab_request & 1) == 0)
            retCode = BERR_SUCCESS; /* HAB is available */
      }
   }
   
   done:
   if (retCode != BERR_SUCCESS)
   {
      BERR_TRACE(retCode);
   }
   return retCode;
}


/******************************************************************************
 BAST_4528_P_ReadHab()
******************************************************************************/
BERR_Code BAST_4528_P_ReadHab(BAST_Handle h, uint8_t addr, uint8_t *buf, uint8_t n)
{
   if ((addr & 0x80) || (n & 0x80))
      return BERR_TRACE(BERR_INVALID_PARAMETER);

   if ((addr + n) > 0x80)
      return BERR_TRACE(BERR_INVALID_PARAMETER);

   return BAST_4528_ReadMemory(h, BCHP_LEAP_HAB_MEM_WORDi_ARRAY_BASE + (uint32_t)addr, buf, (uint32_t)n);
}


/******************************************************************************
 BAST_4528_P_ResetAp()
******************************************************************************/
BERR_Code BAST_4528_P_ResetAp(BAST_Handle h)
{
   BERR_Code retCode;
   uint32_t ctrl, val32;
   uint8_t status, buf;
   
   /* reset LEAP core, set to boot mode 0, reset the CPU */
   ctrl = 0x0000A04F;
   BAST_4528_CHK_RETCODE(BAST_4528_WriteRbus(h, BCHP_LEAP_CTRL_CTRL, &ctrl, 1));

   /* verify that AP is reset */
   BAST_4528_CHK_RETCODE(BAST_4528_ReadBbsi(h, BCHP_CSR_STATUS, &status, 1));
   if (status & BCHP_CSR_STATUS_CPU_RUNNING_MASK)
   {
      BDBG_ERR(("unable to reset the AP"));
      BERR_TRACE(retCode = BAST_ERR_AP_FAIL);
   }
   else
   {
      buf = 0x3B;
      BAST_4528_CHK_RETCODE(BAST_4528_WriteBbsi(h, BCHP_HIF_SFT_RST0, &buf, 1));
      buf = 0xFF;
      BAST_4528_CHK_RETCODE(BAST_4528_WriteBbsi(h, BCHP_HIF_SFT_RST1, &buf, 1));
      BAST_4528_CHK_RETCODE(BAST_4528_WriteBbsi(h, BCHP_HIF_SFT_RST2, &buf, 1));
      buf = 0x00;
      BAST_4528_CHK_RETCODE(BAST_4528_WriteBbsi(h, BCHP_HIF_SFT_RST0, &buf, 1));
      BAST_4528_CHK_RETCODE(BAST_4528_WriteBbsi(h, BCHP_HIF_SFT_RST1, &buf, 1));
      BAST_4528_CHK_RETCODE(BAST_4528_WriteBbsi(h, BCHP_HIF_SFT_RST2, &buf, 1));

      /* initialize some LEAP registers */
      val32 = 0;
      BAST_4528_CHK_RETCODE(BAST_4528_WriteRbus(h, BCHP_LEAP_CTRL_SW_SPARE0, &val32, 1));
      BAST_4528_CHK_RETCODE(BAST_4528_WriteRbus(h, BCHP_LEAP_CTRL_SW_SPARE1, &val32, 1));
      BAST_4528_CHK_RETCODE(BAST_4528_WriteRbus(h, BCHP_LEAP_CTRL_SW_SPARE2, &val32, 1));
      BAST_4528_CHK_RETCODE(BAST_4528_WriteRbus(h, BCHP_LEAP_CTRL_SW_SPARE3, &val32, 1));      
   }
   
   done:
   return retCode;
}


/******************************************************************************
 BAST_4528_P_RunAp()
******************************************************************************/
BERR_Code BAST_4528_P_RunAp(BAST_Handle h)
{
   BERR_Code retCode;
   uint32_t ctrl;
   uint8_t status;

   /* check if the AP is currently running */
   BAST_4528_CHK_RETCODE(BAST_4528_ReadBbsi(h, BCHP_CSR_STATUS, &status, 1));
   if (status & BCHP_CSR_STATUS_CPU_RUNNING_MASK)
   {
      BDBG_WRN(("BAST_4528_P_RunAp(): AP already running\n"));
   }
   else
   {
      BAST_4528_CHK_RETCODE(BAST_4528_ReadRbus(h, BCHP_LEAP_CTRL_CTRL, &ctrl, 1));
      ctrl &= ~BCHP_LEAP_CTRL_CTRL_CPU_RST_MASK;
      ctrl &= ~BCHP_LEAP_CTRL_CTRL_BSPI_RST_MASK;
      ctrl &= ~BCHP_LEAP_CTRL_CTRL_UART_RST_MASK;
      BAST_4528_CHK_RETCODE(BAST_4528_WriteRbus(h, BCHP_LEAP_CTRL_CTRL, &ctrl, 1));

      /* verify CPU is running */
      BAST_4528_CHK_RETCODE(BAST_4528_ReadBbsi(h, BCHP_CSR_STATUS, &status, 1));
      if ((status & BCHP_CSR_STATUS_CPU_RUNNING_MASK) == 0)
      {
         BDBG_ERR(("unable to run the AP\n"));
         BERR_TRACE(retCode = BAST_ERR_AP_FAIL);
      }
   }

   done:
   return retCode;
}


/******************************************************************************
 BAST_4528_P_ProcessInterruptEvent()
******************************************************************************/
BERR_Code BAST_4528_P_ProcessInterruptEvent(
   BAST_Handle h  /* [in] BAST handle */
)
{
   BERR_Code retCode;
   
   BDBG_ASSERT(h);
   BAST_4528_CHK_RETCODE(BAST_4528_P_DecodeInterrupt(h));
   BAST_4528_P_EnableHostInterrupt(h, true);
   
   done:
   return retCode;
}


/******************************************************************************
 BAST_4528_P_EnableHostInterrupt() 
******************************************************************************/
BERR_Code BAST_4528_P_EnableHostInterrupt(
   BAST_Handle h, /* [in] BAST PI handle */
   bool bEnable   /* [in] true=enables the L1 interrupt on the host processor */
)
{
   BKNI_EnterCriticalSection();
   h->settings.i2c.interruptEnableFunc(bEnable, h->settings.i2c.interruptEnableFuncParam);
   BKNI_LeaveCriticalSection();   

   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_4528_P_HandleInterrupt_isr()
******************************************************************************/
BERR_Code BAST_4528_P_HandleInterrupt_isr(
   BAST_Handle h /* [in] BAST handle */
)
{
   BDBG_ASSERT(h);
   
   h->settings.i2c.interruptEnableFunc(false, h->settings.i2c.interruptEnableFuncParam);
   BKNI_SetEvent(((BAST_4528_P_Handle *)(h->pImpl))->hApiEvent);   
   BKNI_SetEvent(((BAST_4528_P_Handle *)(h->pImpl))->hInterruptEvent);  
   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_4528_P_DisableInterrupts()
******************************************************************************/ 
BERR_Code BAST_4528_P_DisableInterrupts(
   BAST_Handle h   /* [in] BAST handle */
)
{
   BERR_Code retCode;
   uint32_t mask = 0xFFFFFFFF;

   BAST_4528_CHK_RETCODE(BAST_4528_WriteRbus(h, BCHP_LEAP_HOST_IRQ_MASK_SET0, &mask, 1));
   BAST_4528_CHK_RETCODE(BAST_4528_WriteRbus(h, BCHP_LEAP_HOST_IRQ_MASK_SET1, &mask, 1));

   done:
   return retCode;
}


/******************************************************************************
 BERR_Code BAST_4528_P_WaitForEvent()
******************************************************************************/
BERR_Code BAST_4528_P_WaitForEvent(
   BAST_Handle h,             /* [in] BAST handle */
   BKNI_EventHandle hEvent,   /* [in] event to wait on */
   int timeoutMsec            /* [in] timeout in milliseconds */
)
{
   BAST_4528_P_Handle *pImpl = (BAST_4528_P_Handle *)(h->pImpl);
   BERR_Code retCode = BERR_SUCCESS;

   BAST_4528_P_EnableHostInterrupt(h, true);
   while (1)
   {   
      retCode = BKNI_WaitForEvent(pImpl->hApiEvent, timeoutMsec);

      if ((retCode == BERR_TIMEOUT) || (retCode == BERR_OS_ERROR)) {
         break;
      } 
      else if (retCode == BERR_SUCCESS)
      {
         BAST_4528_P_DecodeInterrupt(h);
         if ((retCode = BKNI_WaitForEvent(hEvent, 0)) == BERR_SUCCESS)
            break;
      }
      BAST_4528_P_EnableHostInterrupt(h, true);
   }

   BAST_4528_P_EnableHostInterrupt(h, true);
   return retCode;
}


/******************************************************************************
 BAST_4528_P_SendHabCommand()
******************************************************************************/
BERR_Code BAST_4528_P_SendHabCommand(
   BAST_Handle h,      /* [in] BAST handle */
   uint8_t *write_buf, /* [in] specifies the HAB command to send (including space for checksum byte) */
   uint8_t write_len,  /* [in] number of bytes in the HAB command (excluding terminator byte) */ 
   uint8_t *read_buf,  /* [out] holds the data read from the HAB (must be at least write_len bytes long) */ 
   bool    bChecksum   /* [in] true = automatically compute checksum */
)
{
   BERR_Code retCode;
   uint8_t hab_buf[128], i, cs = 0;
   
   if ((write_len > 127) || (write_len < 2))
      return (BERR_TRACE(BERR_INVALID_PARAMETER));

   BAST_4528_CHK_RETCODE(BAST_4528_P_CheckHab(h));
  
   BKNI_Memcpy(hab_buf, write_buf, write_len);

   /* compute checksum */
   if (bChecksum)
   {
      for (i = cs = 0; i < (write_len - 1); i++)
         cs += hab_buf[i];
      hab_buf[write_len-1] = cs;
   }

#if 0
   for (i = 0; i < write_len; i++)
   {
      BDBG_MSG(("WriteHab[%d]: 0x%02X", i, hab_buf[i]));
   }
#endif

   /* write the command to the HAB */
   hab_buf[write_len] = 0; /* make sure there is a zero byte after the last byte of the command */
   BAST_4528_CHK_RETCODE(BAST_4528_WriteMemory(h, BCHP_LEAP_HAB_MEM_WORDi_ARRAY_BASE, hab_buf, (uint32_t)write_len+1));

   /* wait for the AP to service the HAB, and then read any return data */
   BAST_4528_CHK_RETCODE(BAST_4528_P_ServiceHab(h, read_buf, write_len, (uint8_t)(hab_buf[0] | 0x80), bChecksum));

   done:
   return retCode;
}


/******************************************************************************
 BAST_4528_P_DecodeHabError() - called when the command was not ack'd
******************************************************************************/
BERR_Code BAST_4528_P_DecodeHabError(
   BAST_Handle h     /* [in] BAST handle */
)
{
   BERR_Code retCode = BERR_SUCCESS;
   uint32_t status0, spare3, leap_status, clear0 = 0, hab_error, fw_error;

   BAST_4528_CHK_RETCODE(BAST_4528_ReadRbus(h,  BCHP_LEAP_CTRL_SW_SPARE3, &spare3, 1));
   BAST_4528_CHK_RETCODE(BAST_4528_ReadRbus(h,  BCHP_LEAP_HOST_IRQ_STATUS0, &status0, 1));
   BAST_4528_CHK_RETCODE(BAST_4528_ReadRbus(h,  BCHP_LEAP_L2_CPU_STATUS, &leap_status, 1));
   
   BDBG_ERR(("status0=0x%08X, spare3=0x%08X, leap_l2_status=0x%08X", status0, spare3, leap_status));

   if (status0 & BAST_4528_HIRQ0_HAB_ERROR)
   {
      clear0 |= BAST_4528_HIRQ0_HAB_ERROR;

      /* decode HAB error code */
      hab_error = spare3 & 0x07;
      switch (hab_error)
      {
         case 0: /* HAB_SUCCESS */
            retCode = BERR_SUCCESS;
            break;
         case 1: /* HAB_ERROR_COMMAND */
            retCode = BAST_ERR_HAB_BAD_CMD;
            break;
         case 2: /* HAB_ERROR_CHECKSUM */
            retCode = BAST_ERR_HAB_CHECKSUM;
            break;
         case 4: /* HAB_ERROR_PARAMETER */
            retCode = BAST_ERR_HAB_BAD_PARAM;
            break;
         case 3:  /* HAB_ERROR_NOT_IMPLEMENTED */
         case 5: /* HAB_ERROR_NOT_AVAILABLE */
            retCode = BAST_ERR_HAB_NOT_SUPP;
            break;
         case 6: /* HAB_ERROR_UNABLE_TO_SVC */
            retCode = BAST_ERR_HAB_CMD_FAIL;
            break;
         default:
            retCode = BAST_ERR_HAB_FAIL; /* should not get here */
            break;
      }
   }
   else if (status0 & BAST_4528_HIRQ0_FW_ERROR)
   {
      clear0 |= BAST_4528_HIRQ0_FW_ERROR;   

      /* decode FW error code */
      fw_error = (spare3 >> 3) & 0x03;
      /* TBD... */
      retCode = BAST_ERR_AP_UNKNOWN; /* do this for now */
   }

   if (clear0)
   {
      BAST_4528_CHK_RETCODE(BAST_4528_WriteRbus(h,  BCHP_LEAP_HOST_IRQ_CLEAR0, &clear0, 1));
   }

   done:
   return retCode;
}


/******************************************************************************
 BAST_4528_P_ServiceHab()
******************************************************************************/
BERR_Code BAST_4528_P_ServiceHab(
   BAST_Handle h,   /* [in] BAST handle */
   uint8_t *read_buf,  /* [out] holds the data read from the HAB */ 
   uint8_t read_len,   /* [in] number of bytes to read from the HAB (including the checksum byte) */
   uint8_t ack_byte,   /* [in] value of the ack byte to expect */
   bool    bChecksum   /* [in] true = automatically compute checksum */
)
{
   BAST_4528_P_Handle *pImpl = (BAST_4528_P_Handle *)(h->pImpl);
   BERR_Code retCode;
   uint32_t mask, val32;
#ifdef BAST_DONT_USE_INTERRUPT
   uint32_t  i;
#endif
   uint8_t   idx, cs;

   /* clear the HAB_DONE interrupt */
   mask = BAST_4528_HIRQ0_HAB_DONE;
   BAST_4528_CHK_RETCODE(BAST_4528_WriteRbus(h, BCHP_LEAP_HOST_IRQ_CLEAR0, &mask, 1));
   BKNI_WaitForEvent(pImpl->hHabDoneEvent, 0);

#ifndef BAST_DONT_USE_INTERRUPT
   /* enable HAB_DONE interrupt */
   BAST_4528_CHK_RETCODE(BAST_4528_WriteRbus(h, BCHP_LEAP_HOST_IRQ_MASK_CLEAR0, &mask, 1));
#endif
   
   /* send the command */
   val32 = BCHP_LEAP_CTRL_HAB_REQUEST_HAB_REQUEST_MASK;
   BAST_4528_CHK_RETCODE(BAST_4528_WriteRbus(h, BCHP_LEAP_CTRL_HAB_REQUEST, &val32, 1));
   
   /* assert the HAB interrupt on the LEAP */
   val32 = BCHP_LEAP_L2_CPU_SET_HAB_INTR_MASK;
   BAST_4528_CHK_RETCODE(BAST_4528_WriteRbus(h, BCHP_LEAP_L2_CPU_SET, &val32, 1));

#ifdef BAST_DONT_USE_INTERRUPT
   /* wait for HAB to be serviced (polling) */
   for (i = 0; i < 1200; i++)
   {
      BAST_4528_CHK_RETCODE(BAST_4528_ReadRbus(h, BCHP_LEAP_HOST_IRQ_STATUS0, &val32, 1));
      if (val32 & BAST_4528_HIRQ0_HAB_DONE)
         break;
   }
   if ((val32 & BAST_4528_HIRQ0_HAB_DONE) == 0)
   {
      BERR_TRACE(retCode = BAST_ERR_HAB_TIMEOUT);
      goto done;
   }
   else
   {
      /* clear the HAB_DONE interrupt status */
      BAST_4528_CHK_RETCODE(BAST_4528_WriteRbus(h, BCHP_LEAP_HOST_IRQ_CLEAR0, &mask, 1));
   }
#else
   /* wait for HAB done interrupt */  
   if (BAST_4528_P_WaitForEvent(h, pImpl->hHabDoneEvent, 500) == BERR_TIMEOUT)
   {
      uint32_t spare3, status0, mask0;
      BAST_4528_CHK_RETCODE(BAST_4528_ReadRbus(h, BCHP_LEAP_CTRL_SW_SPARE3, &spare3, 1));
      BAST_4528_CHK_RETCODE(BAST_4528_ReadRbus(h, BCHP_LEAP_HOST_IRQ_STATUS0, &status0, 1));
      BAST_4528_CHK_RETCODE(BAST_4528_ReadRbus(h, BCHP_LEAP_HOST_IRQ_MASK_STATUS0, &mask0, 1));
      BDBG_ERR(("HAB timeout: spare3=0x%08X, status0=0x%08X, mask0=0x%08X", spare3, status0, mask0));
      BERR_TRACE(retCode = BAST_ERR_HAB_TIMEOUT);
      goto done;
   }
#endif
   
   /* get the HAB contents */
   BAST_4528_CHK_RETCODE(BAST_4528_P_ReadHab(h, 0, read_buf, read_len));

   /* determine if the command was serviced */
   if (ack_byte != read_buf[0])
   {
      retCode = BAST_4528_P_DecodeHabError(h);
      if (retCode == BERR_SUCCESS)
      {
         BDBG_ERR(("HAB command not serviced!\n"));
         BERR_TRACE(retCode = BAST_ERR_HAB_NO_ACK);
      }
      goto done;
   }
   
   /* validate the checksum */
   if (bChecksum)
   {
      for (idx = cs = 0; idx < (read_len - 1); idx++)
      {
         cs += read_buf[idx];
#if 0
         BDBG_MSG(("ReadHab[%d] = 0x%02X", idx, read_buf[idx]));
#endif
      }
      if (cs != read_buf[read_len - 1])
      {
         BDBG_ERR(("Bad HAB checksum!\n"));
         BERR_TRACE(retCode = BAST_ERR_HAB_CHECKSUM);
      }
   }

   done:
   return retCode;
}


/******************************************************************************
 BAST_4528_P_DecodeInterrupt()
******************************************************************************/
BERR_Code BAST_4528_P_DecodeInterrupt(BAST_Handle h)
{
   BAST_4528_P_Handle *pImpl = (BAST_4528_P_Handle *)(h->pImpl);
   BAST_4528_P_ChannelHandle *pChn;
   BERR_Code retCode;
   uint32_t status0, status1, mask0, mask1, fstatus0, fstatus1, val32;
   uint32_t mask_set0 = 0, mask_set1 = 0;
   uint8_t channel;
   BAST_4528_CHK_RETCODE(BAST_4528_ReadRbus(h,  BCHP_LEAP_HOST_IRQ_STATUS0, &status0, 1));
   BAST_4528_CHK_RETCODE(BAST_4528_ReadRbus(h,  BCHP_LEAP_HOST_IRQ_STATUS1, &status1, 1));
   BAST_4528_CHK_RETCODE(BAST_4528_ReadRbus(h,  BCHP_LEAP_HOST_IRQ_MASK_STATUS0, &mask0, 1));
   BAST_4528_CHK_RETCODE(BAST_4528_ReadRbus(h,  BCHP_LEAP_HOST_IRQ_MASK_STATUS1, &mask1, 1));

   fstatus0 = (status0 & ~mask0);
   fstatus1 = (status1 & ~mask1); 
   if ((fstatus0 == 0) && (fstatus1 == 0))
      return BERR_SUCCESS;

#if 1
   BDBG_MSG(("status0, mask0 = 0x%08X, 0x%08X", status0, mask0));
   BDBG_MSG(("status1, mask1 = 0x%08X, 0x%08X", status1, mask1));
#endif
   if (fstatus0)
   {
      if (fstatus0 & BAST_4528_HIRQ0_INIT_DONE)
      {
         BKNI_SetEvent(pImpl->hInitDoneEvent);
         mask_set0 |= BAST_4528_HIRQ0_INIT_DONE;
      }

      if (fstatus0 & BAST_4528_HIRQ0_HAB_DONE)
      {
         BKNI_SetEvent(pImpl->hHabDoneEvent);
         mask_set0 |= BAST_4528_HIRQ0_HAB_DONE;
      }

      if (fstatus0 & BAST_4528_HIRQ0_FTM_MSG_READY)
      {
         BKNI_SetEvent(pImpl->hFskEvent);
      }

      if (fstatus0 & BAST_4528_HIRQ0_DISEQC_DONE)
      {
         BKNI_SetEvent(pImpl->hDiseqcEvent);
         mask_set0 |= BAST_4528_HIRQ0_DISEQC_DONE;
      }

      if (fstatus0 & BAST_4528_HIRQ0_DISEQC_OVERVOLTAGE)
      {
         BKNI_SetEvent(pImpl->hDiseqcOverVoltageEvent);
      }

      if (fstatus0 & BAST_4528_HIRQ0_DISEQC_UNDERVOLTAGE)
      {
         BKNI_SetEvent(pImpl->hDiseqcUnderVoltageEvent);
      }
   }
  
   if (fstatus1)
   {
      for (channel = 0; channel < BAST_4528_MAX_CHANNELS; channel++)
      {
         pChn = (BAST_4528_P_ChannelHandle *)(h->pChannels[channel]->pImpl);
         val32 = (fstatus1 >> (channel*4)) & 0x0F;
         if (val32 & BAST_4528_HIRQ1_CHAN0_LOCK_CHANGE) 
         {
            BKNI_SetEvent(pChn->hLockChangeEvent);
         }
         if (val32 & BAST_4528_HIRQ1_CHAN0_STATUS_EVENT) 
         {
            BKNI_SetEvent(pChn->hStatusEvent);
         }
         if (val32 & BAST_4528_HIRQ1_CHAN0_PEAK_SCAN_DONE) 
         {
            BKNI_SetEvent(pChn->hPeakScanEvent);
            mask_set1 |= (BAST_4528_HIRQ1_CHAN0_PEAK_SCAN_DONE << (channel*4));
         }
      }
   }

   /* clear the interrupt status */
   BAST_4528_CHK_RETCODE(BAST_4528_WriteRbus(h, BCHP_LEAP_HOST_IRQ_CLEAR0, &fstatus0, 1));
   BAST_4528_CHK_RETCODE(BAST_4528_WriteRbus(h, BCHP_LEAP_HOST_IRQ_CLEAR1, &fstatus1, 1));

   if (mask_set0)
   {
      BAST_4528_CHK_RETCODE(BAST_4528_WriteRbus(h,  BCHP_LEAP_HOST_IRQ_MASK_SET0, &mask_set0, 1));
   }
   if (mask_set1)
   {
      BAST_4528_CHK_RETCODE(BAST_4528_WriteRbus(h,  BCHP_LEAP_HOST_IRQ_MASK_SET1, &mask_set1, 1));
   }
 
   done:
   return retCode;
}

#endif /* BAST_USE_HAB_PI */

