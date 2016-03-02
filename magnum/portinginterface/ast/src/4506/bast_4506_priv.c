/***************************************************************************
 *     Copyright (c) 2007-2013, Broadcom Corporation
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
#include "bchp_4506.h"
#include "bi2c.h"
#include "bkni_multi.h"
#include "bast.h"
#include "bast_priv.h"
#include "bast_4506_priv.h"

#ifndef BAST_4506_VER
#error "BAST_4506_VER needs to be defined"
#endif

#if BAST_4506_VER==BCHP_VER_A0
#error "unsupported version specified in BAST_4506_VER"
#define BAST_RAM_SIZE 0xE000
#define BAST_JDEC BCM4506_JDEC_56K_RAM
#elif BAST_4506_VER==BCHP_VER_A1
#define BAST_RAM_SIZE 0xE000
#define BAST_JDEC BCM4506_JDEC_56K_RAM
#elif BAST_4506_VER==BCHP_VER_B0
#define BAST_RAM_SIZE 0xE000
#define BAST_JDEC BCM4506_JDEC_56K_RAM
#else
#error "unsupported version specified in BAST_4506_VER"
#endif

BDBG_MODULE(bast_4506_priv);


#define BAST_MBOX_RETRIES 10

/******************************************************************************
 BAST_4506_P_Open()
******************************************************************************/
BERR_Code BAST_4506_P_Open(
   BAST_Handle *h,                   /* [out] BAST handle */
   BCHP_Handle hChip,                /* [in] chip handle */
   void        *pReg,                /* [in] pointer to register or i2c handle */
   BINT_Handle hInterrupt,           /* [in] Interrupt handle */
   const BAST_Settings *pDefSettings /* [in] default settings */
)
{
   BERR_Code retCode;
   BAST_Handle hDev;
   BAST_4506_P_Handle *h4506Dev;
   uint8_t i;

   BSTD_UNUSED(hInterrupt);
   BSTD_UNUSED(hChip);

   BDBG_ASSERT(pDefSettings->i2c.interruptEnableFunc);

   /* allocate memory for the handle */
   hDev = (BAST_Handle)BKNI_Malloc(sizeof(BAST_P_Handle));
   BDBG_ASSERT(hDev);
   h4506Dev = (BAST_4506_P_Handle *)BKNI_Malloc(sizeof(BAST_4506_P_Handle));
   BDBG_ASSERT(h4506Dev);
   hDev->pImpl = (void*)h4506Dev;

   hDev->pChannels = (BAST_P_ChannelHandle **)BKNI_Malloc(BAST_4506_MAX_CHANNELS * sizeof(BAST_P_ChannelHandle *));
   BDBG_ASSERT(hDev->pChannels);

   /* initialize our handle */
   h4506Dev->hRegister = (BREG_I2C_Handle)pReg;
   BKNI_Memcpy((void*)(&(hDev->settings)), (void*)pDefSettings, sizeof(BAST_Settings));
   hDev->totalChannels = BAST_4506_MAX_CHANNELS;
   for (i = 0; i < BAST_4506_MAX_CHANNELS; i++)
      hDev->pChannels[i] = NULL;

   /* create events */
   retCode = BKNI_CreateEvent(&(h4506Dev->hInterruptEvent));
   if (retCode != BERR_SUCCESS)
      goto done_0;
   retCode = BKNI_CreateEvent(&(h4506Dev->hApiEvent));
   if (retCode != BERR_SUCCESS)
      goto done_1;
   retCode = BKNI_CreateEvent(&(h4506Dev->hInitDoneEvent));
   if (retCode != BERR_SUCCESS)
      goto done_2;
   retCode = BKNI_CreateEvent(&(h4506Dev->hHabDoneEvent));
   if (retCode != BERR_SUCCESS)
      goto done_3;
   retCode = BKNI_CreateEvent(&(h4506Dev->hFtmEvent));
   if (retCode != BERR_SUCCESS)
   {
      BKNI_DestroyEvent(h4506Dev->hHabDoneEvent);

      done_3:
      BKNI_DestroyEvent(h4506Dev->hInitDoneEvent);

      done_2:
      BKNI_DestroyEvent(h4506Dev->hApiEvent);

      done_1:
      BKNI_DestroyEvent(h4506Dev->hInterruptEvent);

      done_0:
      BKNI_Free((void*)hDev);
      BKNI_Free((void*)h4506Dev);

      *h = NULL;
      return retCode;
   }

   h4506Dev->last_page_16_15 = 0xFF;
   h4506Dev->last_page_14_7 = 0xFF;
   h4506Dev->last_mbox_15_8 = 0xFF;
   h4506Dev->last_bsc_slave_addr = 0xFF;
   h4506Dev->xtal = 27000000;
   h4506Dev->bInit = false;

   *h = hDev;

   return retCode;
}


/******************************************************************************
 BAST_4506_P_Close()
******************************************************************************/
BERR_Code BAST_4506_P_Close(BAST_Handle h)
{
   BERR_Code retCode;
   BDBG_ASSERT(h);

   retCode = BAST_4506_P_DisableInterrupts(h);

   /* reset the AP */
   if (retCode == BERR_SUCCESS)
      retCode = BAST_4506_P_ResetAp(h);

   BKNI_DestroyEvent(((BAST_4506_P_Handle *)(h->pImpl))->hInterruptEvent);
   BKNI_DestroyEvent(((BAST_4506_P_Handle *)(h->pImpl))->hApiEvent);
   BKNI_DestroyEvent(((BAST_4506_P_Handle *)(h->pImpl))->hInitDoneEvent);
   BKNI_DestroyEvent(((BAST_4506_P_Handle *)(h->pImpl))->hHabDoneEvent);
   BKNI_DestroyEvent(((BAST_4506_P_Handle *)(h->pImpl))->hFtmEvent);
   BKNI_Free((void*)h->pChannels);
   BKNI_Free((void*)h->pImpl);
   BKNI_Free((void*)h);
   h = NULL;

   return retCode;
}


/******************************************************************************
 BAST_4506_P_GetTotalChannels()
******************************************************************************/
BERR_Code BAST_4506_P_GetTotalChannels(
   BAST_Handle  h,             /* [in] BAST handle */
   uint32_t     *totalChannels /* [out] number of channels supported */
)
{
   BAST_4506_P_Handle *p4506 = (BAST_4506_P_Handle *)(h->pImpl);

   if (p4506->bInit)
      *totalChannels = p4506->numTuners;
   else
      *totalChannels = BAST_4506_MAX_CHANNELS;
   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_4506_P_OpenChannel()
******************************************************************************/
BERR_Code BAST_4506_P_OpenChannel(
   BAST_Handle                 h,              /* [in] BAST handle */
   BAST_ChannelHandle         *pChannelHandle, /* [out] BAST channel handle */
   uint32_t                   chnNo,           /* [in] channel number */
   const BAST_ChannelSettings *pSettings       /* [in] channel settings */
)
{
   BERR_Code retCode;
   BAST_ChannelSettings cs;
   BAST_P_ChannelHandle *ch;
   BAST_4506_P_ChannelHandle *ch4506;

   BDBG_ASSERT(chnNo < BAST_4506_MAX_CHANNELS);

   if (pSettings == NULL)
      BAST_4506_GetChannelDefaultSettings(h, chnNo, &cs);
   else
      cs = *pSettings;

   /* allocate memory for the channel handle */
   ch = (BAST_P_ChannelHandle *)BKNI_Malloc(sizeof(BAST_P_ChannelHandle));
   BDBG_ASSERT(ch);
   BKNI_Memcpy((void*)(&ch->settings), (void*)&cs, sizeof(BAST_ChannelSettings));
   ch->channel = (uint8_t)chnNo;
   ch->pDevice = h;
   h->pChannels[chnNo] = ch;

   ch4506 = (BAST_4506_P_ChannelHandle *)BKNI_Malloc(sizeof(BAST_4506_P_ChannelHandle));
   BDBG_ASSERT(ch4506);

   retCode = BKNI_CreateEvent(&(ch4506->hLockChangeEvent));
   if (retCode != BERR_SUCCESS)
      goto done_0;
   retCode = BKNI_CreateEvent(&(ch4506->hDiseqcEvent));
   if (retCode != BERR_SUCCESS)
      goto done_1;
   retCode = BKNI_CreateEvent(&(ch4506->hTunerEvent));
   if (retCode != BERR_SUCCESS)
      goto done_2;
   retCode = BKNI_CreateEvent(&(ch4506->hStatusEvent));
   if (retCode != BERR_SUCCESS)
      goto done_3;
   retCode = BKNI_CreateEvent(&(ch4506->hPeakScanEvent));
   if (retCode != BERR_SUCCESS)
   {
      BKNI_DestroyEvent(ch4506->hStatusEvent);

      done_3:
      BKNI_DestroyEvent(ch4506->hTunerEvent);

      done_2:
      BKNI_DestroyEvent(ch4506->hDiseqcEvent);

      done_1:
      BKNI_DestroyEvent(ch4506->hLockChangeEvent);

      done_0:
      BKNI_Free((void*)ch4506);
      BKNI_Free((void*)ch);

      *pChannelHandle = NULL;
      return retCode;
   }

   ch->pImpl = (void*)ch4506;
   *pChannelHandle = ch;

   return retCode;
}


/******************************************************************************
 BAST_4506_P_CloseChannel()
******************************************************************************/
BERR_Code BAST_4506_P_CloseChannel(
   BAST_ChannelHandle h  /* [in] BAST channel handle */
)
{
   BAST_4506_P_ChannelHandle *ch4506;

   ch4506 = (BAST_4506_P_ChannelHandle *)(h->pImpl);
   BKNI_DestroyEvent(ch4506->hLockChangeEvent);
   BKNI_DestroyEvent(ch4506->hDiseqcEvent);
   BKNI_DestroyEvent(ch4506->hTunerEvent);
   BKNI_DestroyEvent(ch4506->hStatusEvent);
   BKNI_DestroyEvent(ch4506->hPeakScanEvent);
   BKNI_Free((void*)ch4506);
   BKNI_Free((void*)h);
   h = NULL;

   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_4506_P_GetDevice()
******************************************************************************/
BERR_Code BAST_4506_P_GetDevice(
   BAST_ChannelHandle h,  /* [in] BAST channel handle */
   BAST_Handle *pDev      /* [out] BAST handle */
)
{
   *pDev = h->pDevice;
   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_4506_P_InitAp()
******************************************************************************/
BERR_Code BAST_4506_P_InitAp(
   BAST_Handle h,            /* [in] BAST handle */
   const uint8_t *pHexImage  /* [in] pointer to BCM4506 microcode image */
)
{
   BAST_4506_P_Handle *p4506 = (BAST_4506_P_Handle *)(h->pImpl);
   BERR_Code retCode = BERR_SUCCESS;
   uint16_t n, addr;
   const uint8_t *pImage;
   bool bPage3Loaded = false;
   uint8_t sb, retries, buf[16];
#ifdef BAST_VERIFY_DOWNLOAD
   static uint8_t pVerifyBuf[BAST_RAM_SIZE];
#endif

   /* disable host interrupts */
   BAST_CHK_RETCODE(BAST_4506_P_EnableHostInterrupt(h, false));

   /* reset the AP before downloading the microcode */
   BAST_CHK_RETCODE(BAST_4506_P_ResetAp(h));

   /* disable interrupts */
   BAST_CHK_RETCODE(BAST_4506_P_DisableInterrupts(h));

   /* download to RAM */
   pImage = pHexImage;
   while (1)
   {
      n = (*pImage++ << 8);
      n |= *pImage++;

      if (n == 0)
         break;

      addr = (*pImage++ << 8);
      addr |= *pImage++;

      if ((addr >= 0xF000) && (addr < 0xF800))
      {
         sb = 0x0C;
         BAST_CHK_RETCODE(BAST_WriteHostRegister(h, BCM4506_SH_AP_SFR_XDATA_PAGE, &sb));
         addr += 0x0800;
         bPage3Loaded = true;
      }
      else if (addr >= 0xF800)
      {
         sb = (bPage3Loaded ? 0x0B : 0x0C);
         BAST_CHK_RETCODE(BAST_WriteHostRegister(h, BCM4506_SH_AP_SFR_XDATA_PAGE, &sb));
      }

      for (retries = 0; retries < 3; retries++)
      {
         /* BDBG_MSG(("writing %d bytes from 0x%04X", n, addr)); */
         if (BAST_4506_P_WriteMemory(h, addr, pImage, n) == BERR_SUCCESS)
         {
#ifdef BAST_VERIFY_DOWNLOAD
            BDBG_MSG(("verifying %d bytes from 0x%04X", n, addr));
            BAST_CHK_RETCODE(BAST_4506_P_ReadMemory(h, addr, (uint8_t *)pVerifyBuf, n));
            if (BKNI_Memcmp(pImage, pVerifyBuf, n) == 0)
               break;
            BDBG_WRN(("data read back does not match\n"));
#else
            /* check for host transfer error */
            BAST_CHK_RETCODE(BAST_ReadHostRegister(h, BCM4506_SH_AP_SFR_H_STAT1, &sb));
            if ((sb & BCM4506_STAT1_H_ER) == 0)
               break;

            BDBG_WRN(("host transfer error\n"));
            BAST_CHK_RETCODE(BAST_WriteHostRegister(h, BCM4506_SH_AP_SFR_H_STAT1, &sb));
#endif
         }
      }
      if (retries >= 3)
      {
         BERR_TRACE(retCode = BAST_ERR_HOST_XFER);
         goto done;
      }

      pImage += n;
   }

   /* restore XDATA_PAGE to ROM */
   sb = 0x08;
   BAST_CHK_RETCODE(BAST_WriteHostRegister(h, BCM4506_SH_AP_SFR_XDATA_PAGE, &sb));

#ifndef BAST_DONT_USE_INTERRUPT
   /* enable init done interrupt */
   sb = BCM4506_STAT2_INIT_DONE;
   BAST_CHK_RETCODE(BAST_WriteHostRegister(h, BCM4506_SH_AP_SFR_H_IE2, &sb));
   BKNI_WaitForEvent(p4506->hInitDoneEvent, 0);
#endif

   /* start running the AP */
   if ((retCode = BAST_4506_P_RunAp(h)) != BERR_SUCCESS)
      goto done;

#ifdef BAST_DONT_USE_INTERRUPT
   /* poll for AP init done */
   for (n = 0; n < 11111; n++)
   {
      BAST_CHK_RETCODE(BAST_ReadHostRegister(h, BCM4506_SH_AP_SFR_H_STAT2, &sb));
      if (sb & BCM4506_STAT2_INIT_DONE)
         break;
   }
   if ((sb & BCM4506_STAT2_INIT_DONE) == 0)
   {
      BDBG_ERR(("AP initialization timeout\n"));
      BERR_TRACE(retCode = BAST_ERR_AP_NOT_INIT);
   }
#else
   /* wait for init done interrupt */
   if (BAST_4506_P_WaitForEvent(h, p4506->hInitDoneEvent, 1000) != BERR_SUCCESS)
   {
      BDBG_ERR(("AP initialization timeout\n"));
      BERR_TRACE(retCode = BAST_ERR_AP_NOT_INIT);
   }
#endif

   if (retCode == BERR_SUCCESS)
   {
      sb = (uint8_t)(h->settings.networkSpec);
      retCode = BAST_WriteConfig(h->pChannels[0], BCM4506_CONFIG_NETWORK_SPEC, &sb, BCM4506_CONFIG_LEN_NETWORK_SPEC);
      if (retCode != BERR_SUCCESS)
      {
         BDBG_ERR(("unable to set NETWORK_SPEC"));
      }
   }

   if (retCode == BERR_SUCCESS)
   {
      /* determine chip id */
      buf[0] = 0x0C;
      BAST_CHK_RETCODE(BAST_4506_P_SendHabCommand(h, buf, 11, buf, true));
      p4506->chipId = ((buf[1] << 8) | buf[2]);
      if ((buf[7] & 0x20) || (buf[2] == 0x05) || (buf[2] == 0x15))
         p4506->numTuners = 1;
      else
         p4506->numTuners = 2;
      BDBG_MSG(("numTuners=%d", p4506->numTuners));
      p4506->bInit = true;
   }

   done:
   return retCode;
}


/******************************************************************************
 BAST_4506_P_SoftReset()
******************************************************************************/
BERR_Code BAST_4506_P_SoftReset(
   BAST_Handle h           /* [in] BAST device handle */
)
{
   BERR_Code retCode;
   uint8_t buf[3];

   buf[0] = 0x2D;
   buf[1] = 0x03;
   BAST_CHK_RETCODE(BAST_4506_P_SendHabCommand(h, buf, 3, buf, true));

   BKNI_Sleep(7); /* wait for soft reset to complete */

   done:
   return retCode;
}


/******************************************************************************
 BAST_4506_P_GetApStatus()
******************************************************************************/
BERR_Code BAST_4506_P_GetApStatus(
   BAST_Handle h,           /* [in] BAST device handle */
   BAST_ApStatus *pStatus   /* [out] AP status */
)
{
   BERR_Code retCode;
   uint8_t buf[4];

   *pStatus = 0;
   BAST_CHK_RETCODE(BREG_I2C_Read(((BAST_4506_P_Handle *)(h->pImpl))->hRegister, h->settings.i2c.chipAddr, BCM4506_SH_AP_SFR_H_CTL1, buf, 4));

   /*
      format is:
      buf[3] (bits 31:24) = H_STAT3
      buf[2] (bits 23:16) = H_STAT2
      buf[1] (bits 15:8) = H_STAT1
      buf[0] (bits 7:0) = H_CTL1
   */
   *pStatus = buf[0] | (buf[1] << 8) | (buf[2] << 16) | (buf[3] << 24);

   done:
   return retCode;
}


/******************************************************************************
 BAST_4506_P_GetApVersion()
******************************************************************************/
BERR_Code BAST_4506_P_GetApVersion(
   BAST_Handle h,          /* [in] BAST handle */
   uint16_t    *pChipId,   /* [out] BAST chip ID */
   uint8_t     *pChipVer,  /* [out] chip revision number */
   uint32_t    *pBondId,   /* [out] chip bonding option */
   uint8_t     *pApVer,    /* [out] AP microcode version */
   uint8_t     *pCfgVer    /* [out] host configuration version */
)
{
   BERR_Code retCode;
   uint8_t buf[16];

   buf[0] = 0x0C;
   BAST_CHK_RETCODE(BAST_4506_P_SendHabCommand(h, buf, 11, buf, true));

   *pChipId = ((buf[1] << 8) | buf[2]);
   *pChipVer = buf[3];
   *pBondId = (buf[4] << 24) | (buf[5] << 16) | (buf[6] << 8) | buf[7];
   *pApVer = buf[8];
   *pCfgVer = buf[9];

   done:
   return retCode;
}


/******************************************************************************
 BERR_Code BAST_4506_P_ConfigGpio(
******************************************************************************/
BERR_Code BAST_4506_P_ConfigGpio(
   BAST_Handle h,           /* [in] BAST handle */
   uint32_t    write_mask,  /* [in] selects which GPIO pins are output */
   uint32_t    read_mask    /* [in] selects which GPIO pins are input */
)
{
   BERR_Code retCode;

   uint32_t own_mask = (write_mask | read_mask) & 0x0000FFFF;
   uint8_t sb;

   sb = (uint8_t)((own_mask >> 8) & 0xFF);
   BAST_CHK_RETCODE(BAST_WriteHostRegister(h, BCM4506_SH_AP_SFR_GPIO_OW_1, &sb));
   sb = (uint8_t)(own_mask & 0xFF);
   BAST_CHK_RETCODE(BAST_WriteHostRegister(h, BCM4506_SH_AP_SFR_GPIO_OW_0, &sb));
   sb = (uint8_t)((read_mask >> 8) & 0xFF);
   BAST_CHK_RETCODE(BAST_WriteHostRegister(h, BCM4506_SH_AP_SFR_GPIOENB_1, &sb));
   sb = (uint8_t)(read_mask & 0xFF);
   BAST_CHK_RETCODE(BAST_WriteHostRegister(h, BCM4506_SH_AP_SFR_GPIOENB_0, &sb));

   done:
   return retCode;
}


/******************************************************************************
 BERR_Code BAST_4506_P_SetGpio(
******************************************************************************/
BERR_Code BAST_4506_P_SetGpio(
   BAST_Handle h,     /* [in] BAST handle */
   uint32_t    mask,  /* [in] selects which GPIO pins to set */
   uint32_t    state  /* [in] state of GPIO pins */
)
{
   BERR_Code retCode = BERR_SUCCESS;
   uint32_t x;
   uint8_t hi, lo, sb;

   mask &= 0x0000FFFF;

   BAST_CHK_RETCODE(BAST_ReadHostRegister(h, BCM4506_SH_AP_SFR_GPOUT_1, &hi));
   BAST_CHK_RETCODE(BAST_ReadHostRegister(h, BCM4506_SH_AP_SFR_GPOUT_0, &lo));
   x = (hi << 8) | lo;
   x &= ~mask;
   x |= (mask & state);

   sb = (uint8_t)(x >> 8);
   BAST_CHK_RETCODE(BAST_WriteHostRegister(h, BCM4506_SH_AP_SFR_GPOUT_1, &sb));
   sb = (uint8_t)(x & 0xFF);
   BAST_CHK_RETCODE(BAST_WriteHostRegister(h, BCM4506_SH_AP_SFR_GPOUT_0, &sb));

   done:
   return retCode;
}


/******************************************************************************
 BERR_Code BAST_4506_P_GetGpio(
******************************************************************************/
BERR_Code BAST_4506_P_GetGpio(
   BAST_Handle h,     /* [in] BAST handle */
   uint32_t    mask,  /* [in] selects which GPIO pins to read */
   uint32_t    *state  /* [out] state of GPIO pins */
)
{
   BERR_Code retCode = BERR_SUCCESS;
   uint8_t hi, lo;

   BAST_CHK_RETCODE(BAST_ReadHostRegister(h, BCM4506_SH_AP_SFR_GPIN_1, &hi));
   BAST_CHK_RETCODE(BAST_ReadHostRegister(h, BCM4506_SH_AP_SFR_GPIN_0, &lo));
   mask &= 0x0000FFFF;
   *state = (hi << 8) | lo;
   *state &= mask;

   done:
   return retCode;
}


/******************************************************************************
 BAST_4506_P_TuneAcquire()
******************************************************************************/
BERR_Code BAST_4506_P_TuneAcquire(
   BAST_ChannelHandle h,            /* [in] BAST channel handle */
   const uint32_t     freq,         /* [in] RF tuner frequency in Hz */
   const BAST_AcqSettings *pParams  /* [in] acquisition parameters */
)
{
   BERR_Code retCode;
   uint32_t acq_ctl, tunerFreq = freq;
   uint8_t buf[20], mode;

   if ((pParams->symbolRate < 256000UL) || (pParams->symbolRate > 45000000UL))
      return (BERR_TRACE(BERR_INVALID_PARAMETER));

   if ((freq < 200000000UL) || (freq > 2350000000UL))
      return (BERR_TRACE(BERR_INVALID_PARAMETER));

   mode = (uint8_t)pParams->mode;
   if ((mode == BAST_Mode_eUnknown) ||
       ((mode > BAST_Mode_eDcii_4_5) && (mode < BAST_Mode_eLdpc_Qpsk_1_2)) ||
       (mode == 0x2F) || (mode > BAST_Mode_eBlindScan) ||
       ((mode > BAST_Mode_eTurbo_8psk_8_9) && (mode < BAST_Mode_eBlindScan)))
      return (BERR_TRACE(BERR_INVALID_PARAMETER));

   BAST_CHK_RETCODE(BAST_4506_P_EnableLockInterrupt(h, false));

   /* mode enum values changed slightly from 4501 */
   if (mode >= BAST_Mode_eLdpc_Qpsk_1_2)
   {
      if (mode == BAST_Mode_eLdpc_scan)
         mode = 0x20;
      else if (mode < BAST_Mode_eLdpc_scan)
         mode++;
   }

   /* acq_ctl values changed from 4501 */
   acq_ctl = 0;
   if (pParams->acq_ctl & BAST_ACQSETTINGS_PN)
      acq_ctl |= 0x00000001;
   if (pParams->acq_ctl & BAST_ACQSETTINGS_BERT_ENABLE)
      acq_ctl |= 0x00000002;
   if (pParams->acq_ctl & BAST_ACQSETTINGS_PRBS15)
      acq_ctl |= 0x00000004;
   if (pParams->acq_ctl & BAST_ACQSETTINGS_PN_INVERT)
      acq_ctl |= 0x00000008;
   if (pParams->acq_ctl & BAST_ACQSETTINGS_BERT_RESYNC_DISABLE)
      acq_ctl |= 0x00000010;
   if (pParams->acq_ctl & BAST_ACQSETTINGS_TUNER_TEST_MODE)
      acq_ctl |= 0x00000020;
   if (pParams->acq_ctl & BAST_ACQSETTINGS_REACQ_DISABLE)
      acq_ctl |= 0x00000040;
   if (pParams->acq_ctl & BAST_ACQSETTINGS_RS_DISABLE)
      acq_ctl |= 0x00000080;
   if ((pParams->acq_ctl & BAST_ACQSETTINGS_NYQUIST_20) == 0)
      acq_ctl |= 0x00000100;
   acq_ctl |= ((pParams->acq_ctl & BAST_ACQSETTINGS_SPINV_IQ_SCAN) >> 4);
   if (pParams->acq_ctl & BAST_ACQSETTINGS_LDPC_PILOT)
      acq_ctl |= 0x00001000;
   if (pParams->acq_ctl & BAST_ACQSETTINGS_LDPC_PILOT_PLL)
      acq_ctl |= 0x00002000;
   if (pParams->acq_ctl & BAST_ACQSETTINGS_LDPC_PILOT_SCAN)
      acq_ctl |= 0x00010000;

   if (pParams->acq_ctl & BAST_ACQSETTINGS_DCII_SPLIT)
      acq_ctl |= 0x08000000;
   if (pParams->acq_ctl & BAST_ACQSETTINGS_DCII_SPLIT_Q)
      acq_ctl |= 0x10000000;
   if (pParams->acq_ctl & BAST_ACQSETTINGS_OQPSK)
      acq_ctl |= 0x20000000;
   if (pParams->acq_ctl & BAST_ACQSETTINGS_SIGNAL_DETECT_MODE)
      acq_ctl |= 0x00008000;
   acq_ctl |= (pParams->acq_ctl & 0xC0000000);

   buf[0] = (uint8_t)(0x24 | (h->channel ? 0x40 : 0x00));
   buf[1] = (uint8_t)((tunerFreq >> 24) & 0xFF);
   buf[2] = (uint8_t)((tunerFreq >> 16) & 0xFF);
   buf[3] = (uint8_t)((tunerFreq >> 8) & 0xFF);
   buf[4] = (uint8_t)(tunerFreq & 0xFF);
   buf[5] = mode;
   buf[6] = (uint8_t)((pParams->symbolRate >> 24) & 0xFF);
   buf[7] = (uint8_t)((pParams->symbolRate >> 16) & 0xFF);
   buf[8] = (uint8_t)((pParams->symbolRate >> 8) & 0xFF);
   buf[9] = (uint8_t)(pParams->symbolRate & 0xFF);
   buf[10] = (uint8_t)((pParams->carrierFreqOffset >> 24) & 0xFF);
   buf[11] = (uint8_t)((pParams->carrierFreqOffset >> 16) & 0xFF);
   buf[12] = (uint8_t)((pParams->carrierFreqOffset >> 8) & 0xFF);
   buf[13] = (uint8_t)(pParams->carrierFreqOffset & 0xFF);
   buf[14] = (uint8_t)((acq_ctl >> 24) & 0xFF);
   buf[15] = (uint8_t)((acq_ctl >> 16) & 0xFF);
   buf[16] = (uint8_t)((acq_ctl >> 8) & 0xFF);
   buf[17] = (uint8_t)(acq_ctl & 0xFF);
   BAST_CHK_RETCODE(BAST_4506_P_SendHabCommand(h->pDevice, buf, 19, buf, true));

   BAST_CHK_RETCODE(BAST_4506_P_EnableLockInterrupt(h, true));

   done:
   return retCode;
}


/******************************************************************************
 BAST_4506_P_GetChannelStatus()
******************************************************************************/
BERR_Code BAST_4506_P_GetChannelStatus(
   BAST_ChannelHandle h,        /* [in] BAST handle */
   BAST_ChannelStatus *pStatus  /* [out] pointer to status structure */
)
{
   BERR_Code retCode;
   uint32_t lpf;
   uint8_t buf[128], mode_buf[128];

   buf[0] = 0x26 | (h->channel ? 0x40 : 0x00);
   BAST_CHK_RETCODE(BAST_4506_P_SendHabCommand(h->pDevice, buf, 64, buf, true));

   pStatus->bBertLocked = buf[1] & 0x40 ? true : false;
   pStatus->bDemodLocked = (buf[1] & 0x02) ? true : false;
   pStatus->bTunerLocked = (buf[1] & 0x08) ? true : false;

   switch (buf[2])
   {
      case 0:
         pStatus->mode = BAST_Mode_eDvb_1_2;
         break;
      case 1:
         pStatus->mode = BAST_Mode_eDvb_2_3;
         break;
      case 2:
         pStatus->mode = BAST_Mode_eDvb_3_4;
         break;
      case 3:
         pStatus->mode = BAST_Mode_eDvb_5_6;
         break;
      case 4:
         pStatus->mode = BAST_Mode_eDvb_7_8;
         break;
      case 5:
         pStatus->mode = BAST_Mode_eDss_1_2;
         break;
      case 6:
         pStatus->mode = BAST_Mode_eDss_2_3;
         break;
      case 7:
         pStatus->mode = BAST_Mode_eDss_6_7;
         break;
      case 8:
         pStatus->mode = BAST_Mode_eDcii_1_2;
         break;
      case 9:
         pStatus->mode = BAST_Mode_eDcii_2_3;
         break;
      case 0x0A:
         pStatus->mode = BAST_Mode_eDcii_3_4;
         break;
      case 0x0B:
         pStatus->mode = BAST_Mode_eDcii_5_6;
         break;
      case 0x0C:
         pStatus->mode = BAST_Mode_eDcii_7_8;
         break;
      case 0x0D:
         pStatus->mode = BAST_Mode_eDcii_5_11;
         break;
      case 0x0E:
         pStatus->mode = BAST_Mode_eDcii_3_5;
         break;
      case 0x0F:
         pStatus->mode = BAST_Mode_eDcii_4_5;
         break;
      case 0x10:
         pStatus->mode = BAST_Mode_eLdpc_Qpsk_1_2;
         break;
      case 0x11:
         pStatus->mode = BAST_Mode_eLdpc_Qpsk_3_5;
         break;
      case 0x12:
         pStatus->mode = BAST_Mode_eLdpc_Qpsk_2_3;
         break;
      case 0x13:
         pStatus->mode = BAST_Mode_eLdpc_Qpsk_3_4;
         break;
      case 0x14:
         pStatus->mode = BAST_Mode_eLdpc_Qpsk_4_5;
         break;
      case 0x15:
         pStatus->mode = BAST_Mode_eLdpc_Qpsk_5_6;
         break;
      case 0x16:
         pStatus->mode = BAST_Mode_eLdpc_Qpsk_8_9;
         break;
      case 0x17:
         pStatus->mode = BAST_Mode_eLdpc_Qpsk_9_10;
         break;
      case 0x18:
         pStatus->mode = BAST_Mode_eLdpc_8psk_3_5;
         break;
      case 0x19:
         pStatus->mode = BAST_Mode_eLdpc_8psk_2_3;
         break;
      case 0x1A:
         pStatus->mode = BAST_Mode_eLdpc_8psk_3_4;
         break;
      case 0x1B:
         pStatus->mode = BAST_Mode_eLdpc_8psk_5_6;
         break;
      case 0x1C:
         pStatus->mode = BAST_Mode_eLdpc_8psk_8_9;
         break;
      case 0x1D:
         pStatus->mode = BAST_Mode_eLdpc_8psk_9_10;
         break;
      case 0x1E:
         pStatus->mode = BAST_Mode_eTurbo_Qpsk_1_2;
         break;
      case 0x1F:
         pStatus->mode = BAST_Mode_eTurbo_Qpsk_2_3;
         break;
      case 0x20:
         pStatus->mode = BAST_Mode_eTurbo_Qpsk_3_4;
         break;
      case 0x21:
         pStatus->mode = BAST_Mode_eTurbo_Qpsk_5_6;
         break;
      case 0x22:
         pStatus->mode = BAST_Mode_eTurbo_Qpsk_7_8;
         break;
      case 0x23:
         pStatus->mode = BAST_Mode_eTurbo_8psk_2_3;
         break;
      case 0x24:
         pStatus->mode = BAST_Mode_eTurbo_8psk_3_4;
         break;
      case 0x25:
         pStatus->mode = BAST_Mode_eTurbo_8psk_4_5;
         break;
      case 0x26:
         pStatus->mode = BAST_Mode_eTurbo_8psk_5_6;
         break;
      case 0x27:
         pStatus->mode = BAST_Mode_eTurbo_8psk_8_9;
         break;
      default:
         pStatus->mode = BAST_Mode_eUnknown;
         break;
   }

   pStatus->symbolRate = (buf[3] << 24) | (buf[4] << 16) | (buf[5] << 8) | buf[6];
   pStatus->carrierOffset = (int32_t)((buf[7] << 24) | (buf[8] << 16) | (buf[9] << 8) | buf[10]);
   pStatus->tunerFreq = (buf[11] << 24) | (buf[12] << 16) | (buf[13] << 8) | buf[14];
   pStatus->outputBitrate = (buf[16] << 24) | (buf[17] << 16) | (buf[18] << 8) | buf[19];
   pStatus->symbolRateError = (int32_t)((buf[20] << 24) | (buf[21] << 16) | (buf[22] << 8) | buf[23]);
   pStatus->carrierError = (int32_t)((buf[24] << 24) | (buf[25] << 16) | (buf[26] << 8) | buf[27]);
   pStatus->snrEstimate = (buf[28] << 8) | buf[29];
   pStatus->IFagc = (buf[31] << 24) | (buf[32] << 16) | (buf[33] << 8) | buf[34];
   pStatus->RFagc = (buf[35] << 24) | (buf[36] << 16) | (buf[37] << 8) | buf[38];
   pStatus->berErrors = (buf[39] << 24) | (buf[40] << 16) | (buf[41] << 8) | buf[42];
   pStatus->mpegErrors = (buf[43] << 24) | (buf[44] << 16) | (buf[45] << 8) | buf[46];
   pStatus->mpegCount = (buf[47] << 24) | (buf[48] << 16) | (buf[49] << 8) | buf[50];
   pStatus->reacqCount = (buf[51] << 24) | (buf[52] << 16) | (buf[53] << 8) | buf[54];
   pStatus->agf = (buf[55] << 24) | (buf[56] << 16) | (buf[57] << 8) | buf[58];
   pStatus->sample_clock = (buf[59] << 24) | (buf[60] << 16) | (buf[61] << 8) | buf[62];
   lpf = buf[15];
   pStatus->tunerFilter = lpf * 1000000;

   if (BAST_MODE_IS_LDPC(pStatus->mode))
   {
      /* send LDPC_STATUS HAB command */
      mode_buf[0] = 0x13 | (h->channel ? 0x40 : 0x00);
      BAST_CHK_RETCODE(BAST_4506_P_SendHabCommand(h->pDevice, mode_buf, 22, mode_buf, true));

      pStatus->modeStatus.ldpc.totalBlocks = (mode_buf[1] << 24) | (mode_buf[2] << 16) | (mode_buf[3] << 8) | mode_buf[4];
      pStatus->modeStatus.ldpc.corrBlocks = (mode_buf[5] << 24) | (mode_buf[6] << 16) | (mode_buf[7] << 8) | mode_buf[8];
      pStatus->modeStatus.ldpc.badBlocks = (mode_buf[9] << 24) | (mode_buf[10] << 16) | (mode_buf[11] << 8) | mode_buf[12];
      pStatus->modeStatus.ldpc.hpstatus = (mode_buf[13] << 24) | (mode_buf[14] << 16) | (mode_buf[15] << 8) | mode_buf[16];
      pStatus->modeStatus.ldpc.fec_clock = (mode_buf[17] << 24) | (mode_buf[18] << 16) | (mode_buf[19] << 8) | mode_buf[20];
   }
   else if (BAST_MODE_IS_TURBO(pStatus->mode))
   {
      /* send TURBO_STATUS HAB command */
      mode_buf[0] = 0x29 | (h->channel ? 0x40 : 0x00);
      BAST_CHK_RETCODE(BAST_4506_P_SendHabCommand(h->pDevice, mode_buf, 22, mode_buf, true));

      pStatus->modeStatus.turbo.totalBlocks = (mode_buf[1] << 24) | (mode_buf[2] << 16) | (mode_buf[3] << 8) | mode_buf[4];
      pStatus->modeStatus.turbo.corrBlocks = (mode_buf[5] << 24) | (mode_buf[6] << 16) | (mode_buf[7] << 8) | mode_buf[8];
      pStatus->modeStatus.turbo.badBlocks = (mode_buf[9] << 24) | (mode_buf[10] << 16) | (mode_buf[11] << 8) | mode_buf[12];
      pStatus->modeStatus.turbo.hpstatus = (mode_buf[13] << 24) | (mode_buf[14] << 16) | (mode_buf[15] << 8) | mode_buf[16];
      pStatus->modeStatus.turbo.fec_clock = (mode_buf[17] << 24) | (mode_buf[18] << 16) | (mode_buf[19] << 8) | mode_buf[20];
   }
   else if (pStatus->mode != BAST_Mode_eUnknown)
   {
      /* send LEGACY_QPSK HAB command */
      mode_buf[0] = 0x12 | (h->channel ? 0x40 : 0x00);
      BAST_CHK_RETCODE(BAST_4506_P_SendHabCommand(h->pDevice, mode_buf, 15, mode_buf, true));

      pStatus->modeStatus.legacy.rsCorrCount = (mode_buf[1] << 24) | (mode_buf[2] << 16) | (mode_buf[3] << 8) | mode_buf[4];
      pStatus->modeStatus.legacy.rsUncorrCount = (mode_buf[5] << 24) | (mode_buf[6] << 16) | (mode_buf[7] << 8) | mode_buf[8];
      pStatus->modeStatus.legacy.preVitErrCount = (mode_buf[9] << 24) | (mode_buf[10] << 16) | (mode_buf[11] << 8) | mode_buf[12];
      pStatus->modeStatus.legacy.spinv = (BAST_PhaseRotation)((mode_buf[13] >> 4) & 0x03);
      pStatus->modeStatus.legacy.phase = (BAST_PhaseRotation)((mode_buf[13] >> 1) & 0x03);
   }

   done:
   return retCode;
}


/******************************************************************************
 BAST_4506_P_GetLockStatus()
******************************************************************************/
BERR_Code BAST_4506_P_GetLockStatus(
   BAST_ChannelHandle h,  /* [in] BAST handle */
   bool *bLocked          /* [out] pointer to status structure */
)
{
   BERR_Code retCode;
   BAST_ApStatus status, mask;

   BAST_CHK_RETCODE(BAST_4506_P_GetApStatus(h->pDevice, &status));

   mask = h->channel ? BAST_APSTATUS_LOCK_1 : BAST_APSTATUS_LOCK_0;
   *bLocked = (mask & status) ? true : false;

   done:
   return retCode;
}

/******************************************************************************
 BAST_4506_P_ResetStatus()
******************************************************************************/
BERR_Code BAST_4506_P_ResetStatus(
   BAST_ChannelHandle h  /* [in] BAST channel handle */
)
{
   BERR_Code retCode;
   uint8_t buf[4];

   buf[0] = 0x1B | (h->channel ? 0x40 : 0x00);
   buf[1] = 0xFF;
   BAST_CHK_RETCODE(BAST_4506_P_SendHabCommand(h->pDevice, buf, 3, buf, true));

   done:
   return retCode;
}


/******************************************************************************
 BAST_4506_P_SetDiseqcTone()
******************************************************************************/
BERR_Code BAST_4506_P_SetDiseqcTone(
   BAST_ChannelHandle h,  /* [in] BAST channel handle */
   bool bTone             /* [in] true = tone on, false = tone off */
)
{
   BERR_Code retCode;
   uint8_t buf[3];

   buf[0] = 0x17 | (h->channel ? 0x40 : 0x00);
   buf[1] = bTone ? 0x22 : 0x20;
   BAST_CHK_RETCODE(BAST_4506_P_SendHabCommand(h->pDevice, buf, 3, buf, true));

   done:
   return retCode;
}


/******************************************************************************
 BAST_4506_P_GetDiseqcTone()
******************************************************************************/
BERR_Code BAST_4506_P_GetDiseqcTone(
   BAST_ChannelHandle h,  /* [in] BAST channel handle */
   bool *bTone            /* [out] true = tone present, false = tone absent */
)
{
   BERR_Code retCode;
   uint8_t buf[3];

   buf[0] = 0x1A | (h->channel ? 0x40 : 0x00);
   BAST_CHK_RETCODE(BAST_4506_P_SendHabCommand(h->pDevice, buf, 3, buf, true));

   *bTone = (buf[1] ? true : false);

   done:
   return retCode;
}


/******************************************************************************
 BAST_4506_P_SetDiseqcVoltage()
******************************************************************************/
BERR_Code BAST_4506_P_SetDiseqcVoltage(
   BAST_ChannelHandle h, /* [in] BAST channel handle */
   bool bVtop            /* [in] true = VTOP, false = VBOT */
)
{
   BERR_Code retCode;
   uint8_t buf[3];

   buf[0] = 0x17 | (h->channel ? 0x40 : 0x00);
   buf[1] = bVtop ? 0x11 : 0x10;
   BAST_CHK_RETCODE(BAST_4506_P_SendHabCommand(h->pDevice, buf, 3, buf, true));

   done:
   return retCode;
}


/******************************************************************************
 BAST_4506_P_GetDiseqcVoltage()
******************************************************************************/
BERR_Code BAST_4506_P_GetDiseqcVoltage(
   BAST_ChannelHandle h, /* [in] BAST channel handle */
   uint8_t *pVoltage     /* [out] voltage estimation */
)
{
   BERR_Code retCode;
   uint8_t buf[3];

   buf[0] = 0x34 | (h->channel ? 0x40 : 0x00);
   BAST_CHK_RETCODE(BAST_4506_P_SendHabCommand(h->pDevice, buf, 3, buf, true));

   *pVoltage = buf[1];

   done:
   return retCode;
}


/******************************************************************************
 BAST_4506_P_EnableVsenseInterrupts()
******************************************************************************/
BERR_Code BAST_4506_P_EnableVsenseInterrupts(
   BAST_ChannelHandle h,
   bool bEnable
)
{
   BSTD_UNUSED(h);
   BSTD_UNUSED(bEnable);

   return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BAST_4506_P_SendACW()
******************************************************************************/
BERR_Code BAST_4506_P_SendACW(
   BAST_ChannelHandle h,  /* [in] BAST channel handle */
   uint8_t            acw /* [in] auto control word */
)
{
   BERR_Code retCode;
   uint8_t buf[3], mask, ie;

   buf[0] = 0x27 | (h->channel ? 0x40 : 0x00);
   buf[1] = acw;

   /* set up diseqc irq */
   mask = h->channel ? BCM4506_STAT3_CH1_DS_DONE : BCM4506_STAT3_CH0_DS_DONE;
   BAST_CHK_RETCODE(BAST_WriteHostRegister(h->pDevice, BCM4506_SH_AP_SFR_H_FSTAT3, &mask));
   BAST_CHK_RETCODE(BAST_ReadHostRegister(h->pDevice, BCM4506_SH_AP_SFR_H_IE3, &ie));
   ie |= mask;
   BAST_CHK_RETCODE(BAST_WriteHostRegister(h->pDevice, BCM4506_SH_AP_SFR_H_IE3, &ie));

   /* send DISEQC_ACW HAB command */
   BAST_CHK_RETCODE(BAST_4506_P_SendHabCommand(h->pDevice, buf, 3, buf, true));

   done:
   return retCode;
}


/******************************************************************************
 BAST_4506_P_SendDiseqcCommand()
******************************************************************************/
BERR_Code BAST_4506_P_SendDiseqcCommand(
   BAST_ChannelHandle h,    /* [in] BAST channel handle */
   const uint8_t *pSendBuf, /* [in] contains data to send */
   uint8_t sendBufLen       /* [in] number of bytes to send */
)
{
   BERR_Code retCode;
   uint8_t ie, bytes_left, bytes_written;
   uint8_t buf[128], i, mask;

   BDBG_ASSERT(sendBufLen <= 128);

   if (sendBufLen > 16)
   {
      bytes_left = sendBufLen;
      bytes_written = 0;
      while (bytes_left)
      {
         /* set up the diseqc tx buffer */
         buf[0] = 0x2B | (h->channel ? 0x40 : 0x00);
         buf[1] = bytes_written;
         if (bytes_left < 64)
            buf[2] = bytes_left;
         else
            buf[2] = 64;
         for (i = 0; i < buf[2]; i++)
            buf[3+i] = pSendBuf[bytes_written + i];

         bytes_written += buf[2];
         bytes_left -= buf[2];

         BAST_CHK_RETCODE(BAST_4506_P_SendHabCommand(h->pDevice, buf, (uint8_t)(4+buf[2]), buf, true));
      }
   }

   /* set up DISEQC_SEND HAB command */
   buf[0] = 0x18 | (h->channel ? 0x40 : 0x00);
   buf[1] = sendBufLen;
   bytes_written = 3;
   if (sendBufLen <= 16)
   {
      for (i = 0; i < sendBufLen; i++)
      {
         buf[2+i] = pSendBuf[i];
         bytes_written++;
      }
   }

   /* set up diseqc irq */
   mask = h->channel ? BCM4506_STAT3_CH1_DS_DONE : BCM4506_STAT3_CH0_DS_DONE;
   BAST_CHK_RETCODE(BAST_WriteHostRegister(h->pDevice, BCM4506_SH_AP_SFR_H_FSTAT3, &mask));
   BAST_CHK_RETCODE(BAST_ReadHostRegister(h->pDevice, BCM4506_SH_AP_SFR_H_IE3, &ie));
   ie |= mask;
   BAST_CHK_RETCODE(BAST_WriteHostRegister(h->pDevice, BCM4506_SH_AP_SFR_H_IE3, &ie));

   /* send DISEQC_SEND HAB command */
   BAST_CHK_RETCODE(BAST_4506_P_SendHabCommand(h->pDevice, buf, bytes_written, buf, true));

   done:
   return retCode;
}


/******************************************************************************
 BAST_4506_P_GetDiseqcStatus()
******************************************************************************/
BERR_Code BAST_4506_P_GetDiseqcStatus(
   BAST_ChannelHandle h,       /* [in] BAST channel handle */
   BAST_DiseqcStatus  *pStatus /* [out] status of last transaction */
)
{
   BERR_Code retCode;
   uint8_t buf[32], i;

   /* send DISEQC_READ_RECEIVE_BUFFER */
   buf[0] = 0x19 | (h->channel ? 0x40 : 0x00);
   BAST_CHK_RETCODE(BAST_4506_P_SendHabCommand(h->pDevice, buf, 22, buf, true));

   switch (buf[1] & 0x0F)
   {
      case 1:
         pStatus->status = BAST_DiseqcSendStatus_eRxOverflow;
         break;

      case 2:
         pStatus->status = BAST_DiseqcSendStatus_eRxReplyTimeout;
         break;

      case 3:
         pStatus->status = BAST_DiseqcSendStatus_eRxParityError;
         break;

      case 4:
         pStatus->status = BAST_DiseqcSendStatus_eBusy;
         break;

      case 5:
         pStatus->status = BAST_DiseqcSendStatus_eAcwTimeout;
         break;

      default:
         pStatus->status = BAST_DiseqcSendStatus_eSuccess;
         break;
   }

   pStatus->bReplyExpected = (buf[1] & 0x10) ? true : false;
   pStatus->nReplyBytes = buf[4];
   if (pStatus->nReplyBytes > 16)
      pStatus->nReplyBytes = 16;
   for (i = 0; i < pStatus->nReplyBytes; i++)
      pStatus->replyBuffer[i] = buf[5+i];

   done:
   return retCode;
}


/******************************************************************************
 BAST_ResetDiseqc()
******************************************************************************/
BERR_Code BAST_4506_P_ResetDiseqc(
   BAST_ChannelHandle h,    /* [in] BAST channel handle */
   uint8_t options          /* [in] reset options */
)
{
   BERR_Code retCode;
   uint8_t buf[4];

   buf[0] = 0x16 | (h->channel ? 0x40 : 0x00);
   buf[1] = options;
   BAST_CHK_RETCODE(BAST_4506_P_SendHabCommand(h->pDevice, buf, 3, buf, true));

   done:
   return retCode;
}


/******************************************************************************
 BAST_ResetFtm()
******************************************************************************/
BERR_Code BAST_4506_P_ResetFtm(
   BAST_Handle h    /* [in] BAST channel handle */
)
{
   BERR_Code retCode;
   uint8_t buf[2];

   /* turn on ftm interrupts */
   BAST_CHK_RETCODE(BAST_4506_P_EnableFtmInterrupt(h, true));

   buf[0] = 0x23;
   BAST_CHK_RETCODE(BAST_4506_P_SendHabCommand(h, buf, 2, buf, true));

   done:
   return retCode;
}


/******************************************************************************
 BERR_Code BAST_4506_P_ReadFtm()
******************************************************************************/
BERR_Code BAST_4506_P_ReadFtm(
   BAST_Handle h,  /* [in] BAST handle */
   uint8_t *pBuf,  /* [out] data read */
   uint8_t *n      /* [out] length of data read */
)
{
   BERR_Code retCode;
   uint8_t buf[130], i, len, checksum;

   *n = 0;

   buf[0] = 0x20;
   buf[1] = 0x20;
   BAST_CHK_RETCODE(BAST_4506_P_SendHabCommand(h, buf, 2, buf, false));

   checksum = buf[0] + buf[1];
   len = buf[1] & 0x7F;

   if (len)
   {
      /* read the next len bytes in the HAB */
      BAST_CHK_RETCODE(BAST_4506_P_ReadHab(h, 2, &buf[2], (uint8_t)(len+1)));

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
         pBuf[i] = buf[i+1];
   }

   done:
   return retCode;
}


/******************************************************************************
 BERR_Code BAST_4506_P_WriteFtm()
******************************************************************************/
BERR_Code BAST_4506_P_WriteFtm(
   BAST_Handle h,  /* [in] BAST handle */
   uint8_t *pBuf,  /* [in] data to write */
   uint8_t n       /* [in] length of data to write */
)
{
   BERR_Code retCode;
   uint8_t buf[128], i;

#ifndef BAST_FTM_DEBUG
   /* Per DirecTV, dont allow debug options in production code */
   if (n == 4)
   {
      if ((pBuf[0] == 0x83) && (pBuf[1] == 0x04))
      {
         /* this is the Set Options local message */
         /* dont allow option bits 1, 3, 4, and 14 */
         if ((pBuf[2] & 0x40) || (pBuf[3] & 0x1A))
         {
            BDBG_ERR(("specified FTM option bit(s) are not allowed in production code"));
            return BERR_NOT_SUPPORTED;
         }
      }
      else if ((pBuf[0] == 0x83) && (pBuf[1] == 0x02))
      {
         /* this is a "Set Rx Bit Mask" message */
         BDBG_ERR(("Set Rx Bit Mask FTM command not allowed in production code"));
         return BERR_NOT_SUPPORTED;
      }
   }
#endif

   buf[0] = 0x1F;
   for (i = 0; i < n; i++)
   {
      /* BDBG_MSG(("WriteFtm(): 0x%02X", pBuf[i])); */
      buf[1+i] = pBuf[i];
   }
   BAST_CHK_RETCODE(BAST_4506_P_SendHabCommand(h, buf, (uint8_t)(2+n), buf, true));

   done:
   return retCode;
}


/******************************************************************************
 BAST_4506_P_PowerDownFtm()
******************************************************************************/
BERR_Code BAST_4506_P_PowerDownFtm(
   BAST_Handle h  /* [in] BAST handle */
)
{
#if 0
   BERR_Code retCode;
   uint8_t sb, buf[4];

   buf[0] = 0x22;
   buf[1] = 0x10;

   BAST_CHK_RETCODE(BAST_4506_P_SendHabCommand(h, buf, 3, buf, true));

   /* wait for power down task to complete */
   BKNI_Sleep(5);

   /* power down ftm channels of pll */
   sb = BCM4506_SH_IND_PLL_PWRDN0;
   BAST_CHK_RETCODE(BAST_WriteHostRegister(h, BCM4506_SH_SFR_H_LOCAL_ADR, &sb));
   sb = 0x18;
   BAST_CHK_RETCODE(BAST_WriteHostRegister(h, BCM4506_SH_SFR_H_LOCAL_DAT, &sb));

   done:
   return retCode;
#else
   BSTD_UNUSED(h);
   return BERR_SUCCESS;
#endif
}


/******************************************************************************
 BAST_4506_P_PowerUpFtm()
******************************************************************************/
BERR_Code BAST_4506_P_PowerUpFtm(
   BAST_Handle h  /* [in] BAST handle */
)
{
#if 0
   BERR_Code retCode;
   uint8_t sb, buf[4];

   buf[0] = 0x25;
   buf[1] = 0x10;

   BAST_CHK_RETCODE(BAST_4506_P_SendHabCommand(h, buf, 3, buf, true));

   /* wait for power down task to complete */
   BKNI_Sleep(5);

   /* power up ftm channels of pll */
   sb = BCM4506_SH_IND_PLL_PWRDN0;
   BAST_CHK_RETCODE(BAST_WriteHostRegister(h, BCM4506_SH_SFR_H_LOCAL_ADR, &sb));
   sb = 0xC0;
   BAST_CHK_RETCODE(BAST_WriteHostRegister(h, BCM4506_SH_SFR_H_LOCAL_DAT, &sb));

   done:
   return retCode;
#else
   BSTD_UNUSED(h);
   return BERR_SUCCESS;
#endif
}


/******************************************************************************
 BAST_4506_P_WriteMi2c()
******************************************************************************/
BERR_Code BAST_4506_P_WriteMi2c(
   BAST_ChannelHandle h, /* [in] BAST channel handle */
   uint8_t slave_addr,   /* [in] address of the i2c slave device */
   uint8_t *i2c_buf,     /* [in] specifies the data to transmit */
   uint8_t n             /* [in] number of bytes to transmit after the i2c slave address */
)
{
   BERR_Code retCode;
   uint8_t buf[16], i;

   BDBG_ASSERT((n <= 8) && (n > 0));

   buf[0] = 0x14 | (h->channel ? 0x40 : 0x00);
   buf[1] = n - 1;
   buf[2] = slave_addr;

   for (i = 0; i < n; i++)
      buf[3+i] = i2c_buf[i];

   BAST_CHK_RETCODE(BAST_4506_P_SendHabCommand(h->pDevice, buf, (uint8_t)(4+n), buf, true));

   if ((buf[1] & 0x80) == 0)
      retCode = BAST_ERR_MI2C_NO_ACK;

   done:
   return retCode;
}


/******************************************************************************
 BAST_4506_P_ReadMi2c()
******************************************************************************/
BERR_Code BAST_4506_P_ReadMi2c(
   BAST_ChannelHandle h, /* [in] BAST channel handle */
   uint8_t slave_addr,   /* [in] address of the i2c slave device */
   uint8_t *out_buf,     /* [in] specifies the data to transmit before the i2c restart condition */
   uint8_t out_n,        /* [in] number of bytes to transmit before the i2c restart condition not including the i2c slave address */
   uint8_t *in_buf,      /* [out] holds the data read */
   uint8_t in_n          /* [in] number of bytes to read after the i2c restart condition not including the i2c slave address */
)
{
   BERR_Code retCode;
   uint8_t buf[20], i;

   BDBG_ASSERT(out_n <= 8);
   BDBG_ASSERT((in_n > 0) && (in_n <= 8));
   BDBG_ASSERT(in_buf);

   buf[0] = 0x15 | (h->channel ? 0x40 : 0x00);
   buf[1] = (out_n | ((in_n - 1) << 4));
   buf[2] = slave_addr;

   for (i = 0; i < out_n; i++)
      buf[3+i] = out_buf[i];

   BAST_CHK_RETCODE(BAST_4506_P_SendHabCommand(h->pDevice, buf, (uint8_t)(4+out_n+in_n), buf, true));

   if ((buf[1] & 0x80) == 0)
      retCode = BAST_ERR_MI2C_NO_ACK;
   else
   {
      for (i = 0; i < in_n; i++)
         in_buf[i] = buf[i + out_n + 3];
   }

   done:
   return retCode;
}


/******************************************************************************
 BAST_4506_P_GetSoftDecisionBuf()
******************************************************************************/
BERR_Code BAST_4506_P_GetSoftDecisionBuf(
   BAST_ChannelHandle h,  /* [in] BAST channel handle */
   int16_t *pI,           /* [out] 15 I-values */
   int16_t *pQ            /* [out] 15 Q-values */
)
{
   BERR_Code retCode;
   uint8_t i;

#if 0
   uint32_t val32;

   /* use IO MBOX to read soft decisions */
   for (i = 0; i < 15; i++)
   {
      BAST_CHK_RETCODE(BAST_4506_P_ReadRegister(h, BCM4506_SDS_EQSFT, &val32));
      if (retCode != BERR_SUCCESS)
         break;

      pI[i] = (val32 >> 24) & 0xFF;
      if (pI[i] & 0x80)
         pI[i] -= 0x100;
      pQ[i] = (val32 >> 16) & 0xFF;
      if (pQ[i] & 0x80)
         pQ[i] -= 0x100;
   }
#else
   /* use HAB to get soft decisions */
   uint8_t buf[128], addr_hi, addr_lo, cs;

   buf[0] = 0x07;
   addr_hi = h->channel ? 0x65 : 0x45;
   addr_lo = 0x50;
   cs = h->channel ? 0xB8 : 0x98;

   for (i = 0; i < 15; i++)
   {
      buf[i*8 + 0] = 0x07;
      buf[i*8 + 1] = addr_hi;
      buf[i*8 + 2] = addr_lo;
      buf[i*8 + 3] = 0xFF;
      buf[i*8 + 4] = 0xFF;
      buf[i*8 + 5] = 0xFF;
      buf[i*8 + 6] = 0xFF;
      buf[i*8 + 7] = cs;
   }
   BAST_CHK_RETCODE(BAST_4506_P_SendHabCommand(h->pDevice, buf, 120, buf, false));
   for (i = 0; i < 15; i++)
   {
      pI[i] = buf[i*8 + 3];
      if (pI[i] & 0x80)
         pI[i] -= 0x100;
      pQ[i] = buf[i*8 + 4];
      if (pQ[i] & 0x80)
         pQ[i] -= 0x100;
   }
#endif

   done:
   return retCode;
}


/******************************************************************************
 BERR_Code BAST_4506_P_ReadAgc()
******************************************************************************/
BERR_Code BAST_4506_P_ReadAgc(
   BAST_ChannelHandle h,  /* [in] BAST channel handle */
   BAST_Agc which_agc,    /* [in] AGC select */
   uint32_t *pAgc         /* [out] integrator value */
)
{
   uint8_t buf[8];
   BERR_Code retCode;

   buf[0] = 0x07;
   buf[1] = (h->channel ? 0x64 : 0x44);
   buf[2] = (which_agc == BAST_Agc_eIF ? 0x88 : 0x90);

   BAST_CHK_RETCODE(BAST_4506_P_SendHabCommand(h->pDevice, buf, 8, buf, true));

   *pAgc = (buf[3] << 24) | (buf[4] << 16) | (buf[5] << 8) | buf[6];

   done:
   return retCode;
}


/******************************************************************************
 BERR_Code BAST_4506_P_WriteAgc()
******************************************************************************/
BERR_Code BAST_4506_P_WriteAgc(
   BAST_ChannelHandle h,  /* [in] BAST channel handle */
   BAST_Agc which_agc,    /* [in] AGC select */
   uint32_t *pAgc         /* [in] integrator value */
)
{
   uint8_t buf[8];

   buf[0] = 0x08;
   buf[1] = (h->channel ? 0x64 : 0x44);
   buf[2] = (which_agc == BAST_Agc_eIF ? 0x88 : 0x90);
   buf[3] = (uint8_t)(*pAgc >> 24);
   buf[4] = (uint8_t)(*pAgc >> 16);
   buf[5] = (uint8_t)(*pAgc >> 8);
   buf[6] = (uint8_t)(*pAgc & 0xFF);

   return BAST_4506_P_SendHabCommand(h->pDevice, buf, 8, buf, true);
}


/******************************************************************************
 BERR_Code BAST_4506_P_ConfigAgc()
******************************************************************************/
BERR_Code BAST_4506_P_ConfigAgc(
   BAST_Handle h,   /* [in] BAST device handle */
   uint32_t    ctl  /* [in] integrator value */
)
{
   uint8_t buf[4];

   buf[0] = 0x28;
   buf[1] = (uint8_t)(ctl & 0xFF);
   return BAST_4506_P_SendHabCommand(h, buf, 3, buf, true);
}


/******************************************************************************
 BERR_Code BAST_4506_P_HabIsbEdit()
******************************************************************************/
BERR_Code BAST_4506_P_HabIsbEdit(
   BAST_ChannelHandle h, /* [in] BAST channel handle */
   uint16_t isb_addr,    /* [in] ISB address */
   uint32_t and_mask,    /* [in] AND mask */
   uint32_t or_mask      /* [in] OR mask */
)
{
   uint8_t buf[12];

   buf[0] = 0x21 | (h->channel ? 0x40 : 0x00);
   buf[1] = isb_addr >> 8;
   buf[2] = isb_addr & 0xFF;
   buf[3] = (uint8_t)(and_mask >> 24);
   buf[4] = (uint8_t)(and_mask >> 16);
   buf[5] = (uint8_t)(and_mask >> 8);
   buf[6] = (uint8_t)(and_mask & 0xFF);
   buf[7] = (uint8_t)(or_mask >> 24);
   buf[8] = (uint8_t)(or_mask >> 16);
   buf[9] = (uint8_t)(or_mask >> 8);
   buf[10] = (uint8_t)(or_mask & 0xFF);

   return BAST_4506_P_SendHabCommand(h->pDevice, buf, 12, buf, true);
}


/******************************************************************************
 BERR_Code BAST_4506_P_FreezeAgc()
******************************************************************************/
BERR_Code BAST_4506_P_FreezeAgc(
   BAST_ChannelHandle h,  /* [in] BAST channel handle */
   BAST_Agc which_agc,    /* [in] AGC select */
   bool bFreeze           /* [in] true = freeze, false = unfreeze */
)
{
   uint32_t and_mask, or_mask;
   uint16_t addr;

   BAST_4506_P_AbortAcq(h);

   addr = h->channel ? 0x6000 : 0x4000;
   addr |= which_agc == BAST_Agc_eIF ? 0x90 : 0x94;
   and_mask = 0x000000FC;
   or_mask = bFreeze ? 0x00000002 : 0x00000000;
   return BAST_4506_P_HabIsbEdit(h, addr, and_mask, or_mask);
}


/******************************************************************************
 BERR_Code BAST_4506_P_FreezeEq()
******************************************************************************/
BERR_Code BAST_4506_P_FreezeEq(
   BAST_ChannelHandle h,  /* [in] BAST handle */
   bool bFreeze           /* [in] true = freeze, false = unfreeze */
)
{
   uint32_t and_mask, or_mask;
   uint16_t addr;

   addr = h->channel ? 0x60EC : 0x40EC;
   and_mask = 0x000000F8;
   or_mask = bFreeze ? 0x00000007 : 0x00000002;
   return BAST_4506_P_HabIsbEdit(h, addr, and_mask, or_mask);
}


/******************************************************************************
 BAST_4506_P_PowerDown()
******************************************************************************/
BERR_Code BAST_4506_P_PowerDown(
   BAST_ChannelHandle h,  /* [in] BAST handle */
   uint32_t options       /* [in] see BAST_CORE_* macros */
)
{
#if 0
   uint8_t buf[4];

   /* abort acquisition before power down */
   BAST_4506_P_AbortAcq(h);

   buf[0] = 0x22;
   buf[1] = 0;
   if (options & BAST_CORE_SDS)
      buf[1] |= (h->channel ? 0x02 : 0x01);
   if (options & BAST_CORE_DISEQC)
      buf[1] |= (h->channel ? 0x08 : 0x04);

   return BAST_4506_P_SendHabCommand(h->pDevice, buf, 3, buf, true);
#else
   BSTD_UNUSED(h);
   BSTD_UNUSED(options);
   return BERR_SUCCESS;
#endif
}


/******************************************************************************
 BAST_4506_P_PowerUp()
******************************************************************************/
BERR_Code BAST_4506_P_PowerUp(
   BAST_ChannelHandle h,  /* [in] BAST channel handle */
   uint32_t options       /* [in] see BAST_CORE_* macros */
)
{
#if 0
   uint8_t buf[4];

   buf[0] = 0x25;
   buf[1] = 0;
   if (options & BAST_CORE_SDS)
      buf[1] |= (h->channel ? 0x02 : 0x01);
   if (options & BAST_CORE_DISEQC)
      buf[1] |= (h->channel ? 0x08 : 0x04);

   return BAST_4506_P_SendHabCommand(h->pDevice, buf, 3, buf, true);
#else
   BSTD_UNUSED(h);
   BSTD_UNUSED(options);
   return BERR_SUCCESS;
#endif
}


/******************************************************************************
 BAST_4506_P_ReadRegister()
******************************************************************************/
BERR_Code BAST_4506_P_ReadRegister(
   BAST_ChannelHandle h,     /* [in] BAST channel handle */
   uint32_t           reg,   /* [in] address of register to read */
   uint32_t           *val   /* [in] contains data that was read */
)
{
   BERR_Code retCode = BERR_SUCCESS;
   int retry;

   /* accessing internal registers through io_mbox */
   if ((reg >= 0x4000) && (reg < 0x6000) && (h->channel == 1))
      reg |= 0x2000;
   else if ((reg >= 0x6000) && (reg < 0x8000) && (h->channel == 0))
      reg &= ~0x2000;
   for (retry = 0; retry < 5; retry++)
   {
      retCode = BAST_4506_P_ReadMbox(h->pDevice, (uint16_t)reg, val);
      if (retCode == BERR_SUCCESS)
         break;
   }

   return retCode;
}


/******************************************************************************
 BAST_4506_P_WriteRegister()
******************************************************************************/
BERR_Code BAST_4506_P_WriteRegister(
   BAST_ChannelHandle h,     /* [in] BAST channel handle */
   uint32_t           reg,   /* [in] address of register to read */
   uint32_t           *val   /* [in] contains data that was read */
)
{
   BERR_Code retCode;

   /* accessing internal registers through io_mbox */
   if ((reg >= 0x4000) && (reg < 0x6000) && (h->channel == 1))
      reg |= 0x2000;
   else if ((reg >= 0x6000) && (reg < 0x8000) && (h->channel == 0))
      reg &= ~0x2000;
   BAST_CHK_RETCODE(BAST_4506_P_WriteMbox(h->pDevice, (uint16_t)reg, val));

   done:
   return retCode;
}


/******************************************************************************
 BAST_4506_P_GetInterruptEventHandle()
******************************************************************************/
BERR_Code BAST_4506_P_GetInterruptEventHandle(
   BAST_Handle h,            /* [in] BAST handle */
   BKNI_EventHandle *hEvent  /* [out] interrupt event handle */
)
{
   *hEvent = ((BAST_4506_P_Handle *)(h->pImpl))->hInterruptEvent;
   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_4506_P_GetLockStateChangeEventHandle()
******************************************************************************/
BERR_Code BAST_4506_P_GetLockStateChangeEventHandle(
   BAST_ChannelHandle h,       /* [in] BAST channel handle */
   BKNI_EventHandle   *hEvent  /* [out] lock event handle */
)
{
   *hEvent = ((BAST_4506_P_ChannelHandle *)(h->pImpl))->hLockChangeEvent;
   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_4506_P_GetFtmEventHandle()
******************************************************************************/
BERR_Code BAST_4506_P_GetFtmEventHandle(
   BAST_Handle h,            /* [in] BAST handle */
   BKNI_EventHandle *hEvent  /* [out] lock event handle */
)
{
   *hEvent = ((BAST_4506_P_Handle *)(h->pImpl))->hFtmEvent;
   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_4506_P_GetDiseqcEventHandle()
******************************************************************************/
BERR_Code BAST_4506_P_GetDiseqcEventHandle(
   BAST_ChannelHandle h,     /* [in] BAST channel handle */
   BKNI_EventHandle *hEvent  /* [out] lock event handle */
)
{
   *hEvent = ((BAST_4506_P_ChannelHandle *)(h->pImpl))->hDiseqcEvent;
   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_4506_P_GetDiseqcVsenseEventHandle()
******************************************************************************/
BERR_Code BAST_4506_P_GetDiseqcVsenseEventHandles(
   BAST_ChannelHandle h,
   BKNI_EventHandle *hDiseqcOverVoltageEvent,
   BKNI_EventHandle *hDiseqcUnderVoltageEvent)
{
   BSTD_UNUSED(h);
   BSTD_UNUSED(hDiseqcOverVoltageEvent);
   BSTD_UNUSED(hDiseqcUnderVoltageEvent);

   return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BAST_4506_P_ReadMbox()
******************************************************************************/
BERR_Code BAST_4506_P_ReadMbox(
   BAST_Handle h,    /* [in] BAST handle */
   uint16_t    reg,  /* [in] RBUS register address */
   uint32_t    *val  /* [out] value read from register */
)
{
   BAST_4506_P_Handle *p4506 = (BAST_4506_P_Handle *)(h->pImpl);
   BERR_Code retCode;
   uint8_t sb, i, buf[4];

   /* update bits 15:8 of mbox address if changed */
   sb = reg >> 8;
   if (sb != p4506->last_mbox_15_8)
   {
      BAST_CHK_RETCODE(BAST_WriteHostRegister(h, BCM4506_SH_SFR_IO_MBOX_A_15_8, &sb));
      p4506->last_mbox_15_8 = sb;
   }

   /* check for mbox transfer complete */
   for (i = 0; i < BAST_MBOX_RETRIES; i++)
   {
      /* read from mbox */
      sb = reg & 0xFC;
      BAST_CHK_RETCODE(BAST_WriteHostRegister(h, BCM4506_SH_SFR_IO_MBOX_CMD, &sb));

      BAST_CHK_RETCODE(BAST_ReadHostRegister(h, BCM4506_SH_SFR_IO_MBOX_STATUS, &sb));
      if (((sb & 0xC0) == 0) || (reg == 0x4A74) || (reg == 0x6A74))
      {
         /* transfer completed - now get the data */
         BAST_CHK_RETCODE(BREG_I2C_Read(p4506->hRegister, h->settings.i2c.chipAddr, BCM4506_SH_SFR_IO_MBOX_D_31_24, buf, 4));
         *val = (buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3];
         goto done;
      }
   }

   if (sb & 0x80)
   {
      BERR_TRACE(retCode = BAST_ERR_IOMB_BUSY);
      BDBG_ERR(("BAST_4506_P_ReadMbox(0x%X): IO_MBOX busy", reg));
   }
   else if (sb & 0x40)
   {
      BERR_TRACE(retCode = BAST_ERR_IOMB_XFER);
      BDBG_ERR(("BAST_4506_P_ReadMbox(0x%X): IO_MBOX xfer error", reg));
   }

   done:
   return retCode;
}


/******************************************************************************
 BAST_4506_P_WriteMbox()
******************************************************************************/
BERR_Code BAST_4506_P_WriteMbox(
   BAST_Handle h,    /* [in] BAST handle */
   uint16_t    reg,  /* [in] RBUS register address */
   uint32_t    *val  /* [in] value to write */
)
{
   BAST_4506_P_Handle *p4506 = (BAST_4506_P_Handle *)(h->pImpl);
   BERR_Code retCode;
   uint8_t buf[6], sb, i;

   /* write addr, data, and cmd in one i2c transaction */
   buf[0] = (uint8_t)(reg >> 8);
   buf[1] = (uint8_t)(*val >> 24);
   buf[2] = (uint8_t)(*val >> 16);
   buf[3] = (uint8_t)(*val >> 8);
   buf[4] = (uint8_t)(*val & 0xFF);
   buf[5] = (uint8_t)((reg & 0xFC) | 0x01);
   i = (buf[0] != p4506->last_mbox_15_8) ? 0 : 1;
   p4506->last_mbox_15_8 = buf[0];
   BAST_CHK_RETCODE(BREG_I2C_Write(p4506->hRegister, h->settings.i2c.chipAddr, (uint8_t)(i ? BCM4506_SH_SFR_IO_MBOX_D_31_24 : BCM4506_SH_SFR_IO_MBOX_A_15_8), &buf[i], 6-i));

   for (i = 0; i < BAST_MBOX_RETRIES; i++)
   {
      /* check for mbox transfer complete */
      BAST_CHK_RETCODE(BAST_ReadHostRegister(h, BCM4506_SH_SFR_IO_MBOX_STATUS, &sb));
      if ((sb & 0xC0) == 0)
         goto done;
   }

   if (sb & 0x80)
   {
      BERR_TRACE(retCode = BAST_ERR_IOMB_BUSY);
      BDBG_ERR(("BAST_4506_P_WriteMbox(): IO_MBOX busy\n"));
   }
   else if (sb & 0x40)
   {
      BERR_TRACE(retCode = BAST_ERR_IOMB_XFER);
      BDBG_ERR(("BAST_4506_P_WriteMbox(): IO_MBOX xfer error\n"));
   }

   done:
   return retCode;
}


/******************************************************************************
 BAST_4506_P_SetApWindow()
******************************************************************************/
BERR_Code BAST_4506_P_SetApWindow(
   BAST_Handle h,    /* [in] BAST handle */
   uint32_t window   /* [in] base address of the 128-byte window */
)
{
   BAST_4506_P_Handle *p4506 = (BAST_4506_P_Handle *)(h->pImpl);
   BERR_Code retCode = BERR_SUCCESS;
   uint8_t   haddr_16_15 = (uint8_t)((window >> 15) & 0x03);
   uint8_t   haddr_14_7 = (uint8_t)((window >> 7) & 0xFF);
   uint8_t   buf[2];

   if (p4506->last_page_16_15 != haddr_16_15)
   {
      buf[0] = haddr_16_15;
      buf[1] = haddr_14_7;
      BAST_CHK_RETCODE(BREG_I2C_Write(p4506->hRegister, h->settings.i2c.chipAddr, BCM4506_SH_SFR_H_ADR_16_15, buf, 2));
      p4506->last_page_16_15 = haddr_16_15;
      p4506->last_page_14_7 = haddr_14_7;
   }
   else if (p4506->last_page_14_7 != haddr_14_7)
   {
      BAST_CHK_RETCODE(BAST_WriteHostRegister(h, BCM4506_SH_SFR_H_ADR_14_7, &haddr_14_7));
      p4506->last_page_14_7 = haddr_14_7;
   }

   done:
   return retCode;
}


/******************************************************************************
 BAST_4506_P_ReadHab()
******************************************************************************/
BERR_Code BAST_4506_P_ReadHab(BAST_Handle h, uint8_t addr, uint8_t *buf, uint8_t n)
{
   BAST_4506_P_Handle *p4506 = (BAST_4506_P_Handle *)(h->pImpl);
   BERR_Code retCode;

   if ((addr & 0x80) || (n & 0x80))
      return BERR_TRACE(BERR_INVALID_PARAMETER);

   if ((addr + n) > 0x80)
      return BERR_TRACE(BERR_INVALID_PARAMETER);

   BAST_CHK_RETCODE(BAST_4506_P_SetApWindow(h, BAST_WINDOW_HAB));
   BAST_CHK_RETCODE(BREG_I2C_Read(p4506->hRegister, h->settings.i2c.chipAddr, addr, buf, n));

   done:
   return retCode;
}


/******************************************************************************
 BAST_4506_P_WriteHab()
******************************************************************************/
BERR_Code BAST_4506_P_WriteHab(BAST_Handle h, uint8_t addr, uint8_t *buf, uint8_t n)
{
   BAST_4506_P_Handle *p4506 = (BAST_4506_P_Handle *)(h->pImpl);
   BERR_Code retCode;

   if ((addr & 0x80) || (n & 0x80))
      return BERR_TRACE(BERR_INVALID_PARAMETER);

   if ((addr + n) > 0x80)
      return BERR_TRACE(BERR_INVALID_PARAMETER);

   BAST_CHK_RETCODE(BAST_4506_P_SetApWindow(h, BAST_WINDOW_HAB));
   BAST_CHK_RETCODE(BREG_I2C_Write(p4506->hRegister, h->settings.i2c.chipAddr, addr, buf, n));

   done:
   return retCode;
}


/******************************************************************************
 BAST_4506_P_ReadMemory()
******************************************************************************/
BERR_Code BAST_4506_P_ReadMemory(BAST_Handle h, uint16_t addr, uint8_t *buf, uint16_t n)
{
   BAST_4506_P_Handle *p4506 = (BAST_4506_P_Handle *)(h->pImpl);
   BAST_ApStatus status;
   BERR_Code retCode;

   if (((uint32_t)addr + (uint32_t)n) > 0x10000)
      return BERR_TRACE(BERR_INVALID_PARAMETER);

   BAST_CHK_RETCODE(BAST_4506_P_GetApStatus(h, &status));
   if ((status & BAST_APSTATUS_RUN_MASK) == BAST_APSTATUS_RUN)
   {
      uint16_t bytes_to_read, i, bytes_left = n;
      uint8_t hab[128];

      while (bytes_left > 0)
      {
         if (bytes_left <= 0x7A)
            bytes_to_read = bytes_left;
         else
            bytes_to_read = 0x7A;
         bytes_left -= bytes_to_read;

         hab[0] = 0x01;
         hab[1] = (addr >> 8) & 0xFF;
         hab[2] = addr & 0xFF;
         hab[3] = (uint8_t)bytes_to_read;
         BAST_CHK_RETCODE(BAST_4506_P_SendHabCommand(h, hab, (uint8_t)(5+bytes_to_read), hab, true));
         for (i = 0; i < bytes_to_read; i++)
            buf[i] = hab[4+i];
         buf += bytes_to_read;
         addr += bytes_to_read;
      }
   }
   else
   {
      BAST_CHK_RETCODE(BAST_4506_P_SetApWindow(h, BAST_WINDOW_IRAM + addr));
      BAST_CHK_RETCODE(BREG_I2C_Read(p4506->hRegister, h->settings.i2c.chipAddr, (uint8_t)(addr & 0x7F), buf, n));
      p4506->last_page_16_15 = p4506->last_page_14_7 = 0xFF;
   }

   done:
   return retCode;
}


/******************************************************************************
 BAST_4506_P_WriteMemory()
******************************************************************************/
BERR_Code BAST_4506_P_WriteMemory(BAST_Handle h, uint16_t addr, const uint8_t *buf, uint16_t n)
{
   BAST_4506_P_Handle *p4506 = (BAST_4506_P_Handle *)(h->pImpl);
   BAST_ApStatus status;
   BERR_Code retCode = BERR_SUCCESS;
   uint16_t  curr_addr, nbytes, bytes_left;

   BAST_CHK_RETCODE(BAST_4506_P_GetApStatus(h, &status));
   if ((status & BAST_APSTATUS_RUN_MASK) == BAST_APSTATUS_RUN)
   {
      uint16_t bytes_to_write, bytes_left = n, i, idx;
      uint8_t hab[128];

      idx = 0;
      while (bytes_left > 0)
      {
         if (bytes_left <= 0x7B)
            bytes_to_write = bytes_left;
         else
            bytes_to_write = 0x7B;
         bytes_left -= bytes_to_write;

         hab[0] = 0x02;
         hab[1] = (addr >> 8) & 0xFF;
         hab[2] = addr & 0xFF;
         hab[3] = (uint8_t)bytes_to_write;

         for (i = 0; i < bytes_to_write; i++)
         {
            hab[4+i] = buf[idx++];
         }

         BAST_CHK_RETCODE(BAST_4506_P_SendHabCommand(h, hab, (uint8_t)(5+bytes_to_write), hab, true));
         buf += bytes_to_write;
         addr += bytes_to_write;
      }
   }
   else
   {
      curr_addr = addr;
      bytes_left = n;
      while (bytes_left > 0)
      {
         BAST_CHK_RETCODE(BAST_4506_P_SetApWindow(h, BAST_WINDOW_IRAM + curr_addr));
         nbytes = 128 - (curr_addr % 128);
         if (nbytes > bytes_left)
            nbytes = bytes_left;
         bytes_left -= nbytes;
         BAST_CHK_RETCODE(BREG_I2C_Write(p4506->hRegister, h->settings.i2c.chipAddr, (uint8_t)(curr_addr & 0x7F), buf, nbytes));
         curr_addr += nbytes;
         buf += nbytes;
      }
   }

   done:
   return retCode;
}


/******************************************************************************
 BAST_4506_P_ReadConfig()
******************************************************************************/
BERR_Code BAST_4506_P_ReadConfig(
   BAST_ChannelHandle h,   /* [in] BAST channel handle */
   uint16_t id,            /* [in] configuration parameter ID */
   uint8_t *p,             /* [out] buffer to hold the read data */
   uint8_t len             /* [in] number of bytes to read */
)
{
   BERR_Code retCode;
   uint16_t  addr = id;
   uint8_t   hab[32], i;

   BDBG_ASSERT((len == 1) || (len == 2) || (len == 4) || (len == 16));
   BDBG_ASSERT((len+id) < 0x180);

   switch (id)
   {
      case BCM4506_CONFIG_TUNER_CTL:
      case BCM4506_CONFIG_TUNER_CUTOFF:
      case BCM4506_CONFIG_XPORT_CTL:
      case BCM4506_CONFIG_LOCK_INDICATOR_PIN:
      case BCM4506_CONFIG_DISEQC_CTL1:
      case BCM4506_CONFIG_DISEQC_CTL2:
      case BCM4506_CONFIG_RRTO_USEC:
      case BCM4506_CONFIG_LDPC_SCRAMBLING_SEQ:
      case BCM4506_CONFIG_RT_STATUS_INDICATORS:
      case BCM4506_CONFIG_XPORT_CLOCK_ADJUST:
      case BCM4506_CONFIG_DTV_SCAN_CODE_RATES:
      case BCM4506_CONFIG_DVB_SCAN_CODE_RATES:
      case BCM4506_CONFIG_TURBO_SCAN_MODES:
      case BCM4506_CONFIG_ACQ_TIME:
      case BCM4506_CONFIG_STUFF_BYTES:
      case BCM4506_CONFIG_TIMING_LOOP_LOCK:
      case BCM4506_CONFIG_CNR_THRESH1:
      case BCM4506_CONFIG_LDPC_ALT_TRK_PLC_DAMP:
      case BCM4506_CONFIG_LDPC_ALT_TRK_PLC_BW:
      case BCM4506_CONFIG_LDPC_ALT_ACQ_PLC_DAMP:
      case BCM4506_CONFIG_LDPC_ALT_ACQ_PLC_BW:
         addr += (h->channel ? len : 0);
         break;
   }

   hab[0] = 0x09;
   hab[1] = (addr >> 8);
   hab[2] = (addr & 0xFF);
   hab[3] = len;
   BAST_CHK_RETCODE(BAST_4506_P_SendHabCommand(h->pDevice, hab, (uint8_t)(5+len), hab, true));

   for (i = 0; i < len; i++)
      p[i] = hab[i+4];

   done:
   return retCode;
}


/******************************************************************************
 BAST_4506_P_WriteConfig()
******************************************************************************/
BERR_Code BAST_4506_P_WriteConfig(
   BAST_ChannelHandle h,   /* [in] BAST channel handle */
   uint16_t id,            /* [in] configuration parameter ID */
   uint8_t *p,             /* [in] data to write */
   uint8_t len             /* [in] number of bytes to write */
)
{
   BERR_Code retCode;
   uint16_t  addr = id;
   uint8_t   i, hab[32];

   BDBG_ASSERT((len == 1) || (len == 2) || (len == 4) || (len == 16));
   BDBG_ASSERT((len+id) < 0x180);

   switch (id)
   {
      case BCM4506_CONFIG_TUNER_CTL:
      case BCM4506_CONFIG_TUNER_CUTOFF:
      case BCM4506_CONFIG_XPORT_CTL:
      case BCM4506_CONFIG_LOCK_INDICATOR_PIN:
      case BCM4506_CONFIG_DISEQC_CTL1:
      case BCM4506_CONFIG_DISEQC_CTL2:
      case BCM4506_CONFIG_RRTO_USEC:
      case BCM4506_CONFIG_LDPC_SCRAMBLING_SEQ:
      case BCM4506_CONFIG_RT_STATUS_INDICATORS:
      case BCM4506_CONFIG_XPORT_CLOCK_ADJUST:
      case BCM4506_CONFIG_DTV_SCAN_CODE_RATES:
      case BCM4506_CONFIG_DVB_SCAN_CODE_RATES:
      case BCM4506_CONFIG_TURBO_SCAN_MODES:
      case BCM4506_CONFIG_ACQ_TIME:
      case BCM4506_CONFIG_STUFF_BYTES:
      case BCM4506_CONFIG_TIMING_LOOP_LOCK:
      case BCM4506_CONFIG_CNR_THRESH1:
      case BCM4506_CONFIG_LDPC_ALT_TRK_PLC_DAMP:
      case BCM4506_CONFIG_LDPC_ALT_TRK_PLC_BW:
      case BCM4506_CONFIG_LDPC_ALT_ACQ_PLC_DAMP:
      case BCM4506_CONFIG_LDPC_ALT_ACQ_PLC_BW:
         addr += (h->channel ? len : 0);
         break;
      case BCM4506_CONFIG_FTM_CH_SELECT:
         if (*p == BAST_FskChannelConfig_eCh1Tx_Ch0Rx)
            return BERR_NOT_SUPPORTED;
         else if (*p > BAST_FskChannelConfig_eCh1Tx_Ch1Rx)
            *p = BAST_FskChannelConfig_eCh0Tx_Ch0Rx;
         break;
   }

   hab[0] = 0x0A;
   hab[1] = (addr >> 8);
   hab[2] = (addr & 0xFF);
   hab[3] = len;

   for (i = 0; i < len; i++)
      hab[i+4] = p[i];

   BAST_CHK_RETCODE(BAST_4506_P_SendHabCommand(h->pDevice, hab, (uint8_t)(5+len), hab, true));

   done:
   return retCode;
}


/******************************************************************************
 BAST_4506_P_ResetAp()
******************************************************************************/
BERR_Code BAST_4506_P_ResetAp(BAST_Handle h)
{
   BAST_4506_P_Handle *p4506 = (BAST_4506_P_Handle *)(h->pImpl);
   BERR_Code retCode;
   uint8_t sb;

#if (BAST_4506_VER==BCHP_VER_A0)
   uint8_t i;

   /* S/W workaround for BCM4506-A0: do an extra i2c access */
   BAST_ReadHostRegister(h, BCM4506_SH_AP_SFR_JDEC, &sb);

   /* enable i2c glitch filter to resolve i2c issues */
   for (i = 0; i < 10; i++)
   {
      /* write to indirect delay control register */
      sb = BCM4506_SH_IND_I2C_DLY_CONTROL;
      BAST_WriteHostRegister(h, BCM4506_SH_SFR_H_LOCAL_ADR, &sb);
      sb = 0x20;
      BAST_WriteHostRegister(h, BCM4506_SH_SFR_H_LOCAL_DAT, &sb);

      /* read back to verify write */
      sb = BCM4506_SH_IND_I2C_DLY_CONTROL;
      BAST_WriteHostRegister(h, BCM4506_SH_SFR_H_LOCAL_ADR, &sb);
      BAST_ReadHostRegister(h, BCM4506_SH_SFR_H_LOCAL_DAT, &sb);

      /* continue after successful write */
      if (sb == 0x20)
         break;

      /* abort if repeated writes unsuccessful */
      if ((i >= 9) && (sb != 0x20))
      {
         BERR_TRACE(retCode = BAST_ERR_AP_FAIL);
         goto done;
      }
   }
#endif

#ifdef BAST_4506_54MHZ_XTAL
   /* program AP PLL divider for 54MHz before AP reset */
   BDBG_MSG(("54 MHz crystal mode"));
   ((BAST_4506_P_Handle *)(h->pImpl))->xtal = 54000000;
   sb = 0x21;
   BAST_CHK_RETCODE(BAST_4506_P_WriteHostIndirectRegister(h, BCM4506_SH_IND_PLL_P1_2DIV, &sb));
#endif

   sb = 0x04; /* Enable amplitude limiter */
   BAST_CHK_RETCODE(BAST_4506_P_WriteHostIndirectRegister(h, BCM4506_SH_IND_OSC_CNTRL1, &sb));

#if 0 /* do not toggle power on oscillator */
   sb = 0x05; /* Power down doubler */
   BAST_CHK_RETCODE(BAST_4506_P_WriteHostIndirectRegister(h, BCM4506_SH_IND_OSC_CNTRL0, &sb));

   sb = 0x01; /* Power down oscillator */
   BAST_CHK_RETCODE(BAST_4506_P_WriteHostIndirectRegister(h, BCM4506_SH_IND_XTAL_PWRDN, &sb));

   BKNI_Sleep(1);

   sb = 0x00; /* Power up oscillator */
   BAST_CHK_RETCODE(BAST_4506_P_WriteHostIndirectRegister(h, BCM4506_SH_IND_XTAL_PWRDN, &sb));

   BKNI_Sleep(1);
#endif

   sb = 0x10 | BAST_JDEC; /* use HOST_JDEC value and not the default JDEC pin strap value when global reset goes away */
   BAST_CHK_RETCODE(BAST_4506_P_WriteHostIndirectRegister(h, BCM4506_SH_IND_HOST_JDEC_OVRD, &sb));

   sb = 0x01; /* Reset entire chip except main PLL and HOST I2C/SPI slave */
   BAST_CHK_RETCODE(BAST_4506_P_WriteHostIndirectRegister(h, BCM4506_SH_IND_GLOBAL_RESET, &sb));

   BKNI_Sleep(1);

   sb = 0x0F; /* Assert areset and dreset to main PLL */
   BAST_CHK_RETCODE(BAST_4506_P_WriteHostIndirectRegister(h, BCM4506_SH_IND_PLL_RST, &sb));

   BKNI_Sleep(1);

   sb = 0x0C; /* Deassert areset and dreset to main PLL */
   BAST_CHK_RETCODE(BAST_4506_P_WriteHostIndirectRegister(h, BCM4506_SH_IND_PLL_RST, &sb));

   sb = 0x00; /* release block reset */
   BAST_CHK_RETCODE(BAST_4506_P_WriteHostIndirectRegister(h, BCM4506_SH_IND_GLOBAL_RESET, &sb));

   /* initialize JDEC */
   sb = BAST_JDEC;
   BAST_CHK_RETCODE(BAST_WriteHostRegister(h, BCM4506_SH_AP_SFR_JDEC, &sb));

   /* reset the AP */
   sb = BCM4506_AP_RESET;
   BAST_CHK_RETCODE(BAST_WriteHostRegister(h, BCM4506_SH_AP_SFR_H_CTL1, &sb));

   /* reset AP status */
   sb = 0x0A;
   BAST_CHK_RETCODE(BAST_WriteHostRegister(h, BCM4506_SH_AP_SFR_AP_CMD, &sb));

   sb = 0x00;
   BAST_CHK_RETCODE(BAST_WriteHostRegister(h, BCM4506_SH_AP_SFR_H_MSG1, &sb));
#ifdef BAST_4506_54MHZ_XTAL
   sb = 0x80;
#endif
   BAST_CHK_RETCODE(BAST_WriteHostRegister(h, BCM4506_SH_AP_SFR_H_MSG2, &sb));

   /* verify that AP is reset */
   BAST_CHK_RETCODE(BAST_ReadHostRegister(h, BCM4506_SH_AP_SFR_H_CTL1, &sb));
   if ((sb & BCM4506_AP_MASK) != BCM4506_AP_RESET)
   {
      BDBG_ERR(("unable to reset the AP\n"));
      BERR_TRACE(retCode = BAST_ERR_AP_FAIL);
   }

   done:
   p4506->last_page_16_15 = 0xFF;
   p4506->last_page_14_7 = 0xFF;
   return retCode;
}


/******************************************************************************
 BAST_4506_P_RunAp()
******************************************************************************/
BERR_Code BAST_4506_P_RunAp(BAST_Handle h)
{
   BERR_Code retCode;
   uint8_t   sb, sb2;

   /* check if the AP is currently running */
   BAST_CHK_RETCODE(BAST_ReadHostRegister(h, BCM4506_SH_AP_SFR_H_CTL1, &sb));

   if ((sb & BCM4506_AP_MASK) != BCM4506_AP_RUN)
   {
      /* start running the AP */
      sb2 = BCM4506_AP_RUN;
      BAST_CHK_RETCODE(BAST_WriteHostRegister(h, BCM4506_SH_AP_SFR_H_CTL1, &sb2));

      /* verify that the AP is running */
      BAST_CHK_RETCODE(BAST_ReadHostRegister(h, BCM4506_SH_AP_SFR_H_CTL1, &sb));
      if ((sb & BCM4506_AP_MASK) != BCM4506_AP_RUN)
      {
         BDBG_ERR(("unable to run the AP\n"));
         BERR_TRACE(retCode = BAST_ERR_AP_FAIL);
         goto done;
      }

      /* clear AP_change state bit */
      sb2 = BCM4506_STAT1_AP_OP_CHG;
      BAST_CHK_RETCODE(BAST_WriteHostRegister(h, BCM4506_SH_AP_SFR_H_FSTAT1, &sb2));
   }
   else
   {
      BDBG_WRN(("BAST_4506_P_RunAp(): AP already running\n"));
   }

   done:
   return retCode;
}


/******************************************************************************
 BAST_4506_P_CheckHab()
******************************************************************************/
BERR_Code BAST_4506_P_CheckHab(
   BAST_Handle h    /* [in] BAST handle */
)
{
   BERR_Code retCode, apError;
   BAST_ApStatus status;

   BAST_CHK_RETCODE(BAST_GetApStatus(h, &status));

   apError = BAST_4506_P_DecodeError(h, &status);
#if 0
   if (apError)
   {
      /* report potential AP error */
      BDBG_WRN(("@@@ AP status=0x%08X | error=0x%X", (uint32_t)status, apError));
   }
#endif

   if ((status & BAST_APSTATUS_HAB_MASK) == BAST_APSTATUS_HAB_READY)
      retCode = BERR_SUCCESS;
   else if (apError)
      retCode = apError;
   else
      retCode = BAST_ERR_AP_UNKNOWN;   /* AP hang */

   done:
   return retCode;
}


/******************************************************************************
 BAST_4506_P_DecodeError()
******************************************************************************/
BERR_Code BAST_4506_P_DecodeError(
   BAST_Handle h,           /* [in] BAST handle */
   BAST_ApStatus *pApStatus /* [in] AP status returned by BAST_GetApStatus */
)
{
   BERR_Code retCode = BERR_SUCCESS;
   uint8_t sb;

   if (*pApStatus & BAST_APSTATUS_HAB_ERR)
      retCode = BAST_ERR_HABAV;
   else if (*pApStatus & BAST_APSTATUS_MEM_ERR)
      retCode = BAST_ERR_MEMAV;
   else if (*pApStatus & BAST_APSTATUS_H_ERR)
      retCode = BAST_ERR_HOST_XFER;
   else if (*pApStatus & BAST_APSTATUS_IOMB_ERR)
      retCode = BAST_ERR_IOMB_XFER;
   else if (*pApStatus & BAST_APSTATUS_HABCMD_ERR)
      retCode = BAST_ERR_HAB_ERR;
   else if ((*pApStatus & BAST_APSTATUS_AP_ERR) || (*pApStatus & BAST_APSTATUS_FTM_ERR))
   {
      BAST_CHK_RETCODE(BAST_ReadHostRegister(h, BCM4506_SH_AP_SFR_H_MSG1, &sb));
      switch (sb)
      {
         case 1:
            retCode = BAST_ERR_AP_HABAV;
            break;

         case 2:
            retCode = BAST_ERR_AP_STACK;
            break;

         case 3:
            retCode = BAST_ERR_AP_WD;
            break;

         case 4:
            retCode = BAST_ERR_AP_ISB;
            break;

         case 5:
            retCode = BAST_ERR_AP_SCR;
            break;

         case 6:
         case 7:
            retCode = BAST_ERR_AP_IRQ;
            break;

         case 8:
            retCode = BAST_ERR_AP_COPY;
            break;

         case 0x0A:
            retCode = BAST_ERR_HAB_CHECKSUM;
            break;

         case 0x0B:
            retCode = BAST_ERR_HAB_BAD_CMD;
            break;

         case 0x0C:
            retCode = BAST_ERR_HAB_NOT_SUPP;
            break;

         case 0x0D:
            retCode = BAST_ERR_HAB_BAD_PARAM;
            break;

         case 0x0E:
            retCode = BAST_ERR_HAB_CMD_FAIL;
            break;

         case 0x0F:
            retCode = BAST_ERR_DISEQC_BUSY;
            break;

         case 0x10:
            retCode = BAST_ERR_FTM_TX_BUSY;
            break;

         case 0x11:
            retCode = BAST_ERR_FTM_RX_FULL;
            break;

         case 0x12:
            retCode = BAST_ERR_FTM_MSG_DROPPED;
            break;

         case 0x14:
            retCode = BAST_ERR_POWERED_DOWN;
            break;

         default:
            BDBG_ERR(("unknown MSG1 (=0x%02X)\n", sb));
            retCode = BAST_ERR_AP_UNKNOWN;
            break;
      }
   }

   done:
   return retCode;
}


/******************************************************************************
 BAST_4506_P_SendHabCommand()
******************************************************************************/
BERR_Code BAST_4506_P_SendHabCommand(
   BAST_Handle h,      /* [in] BAST handle */
   uint8_t *write_buf, /* [in] specifies the HAB command to send (including space for checksum byte) */
   uint8_t write_len,  /* [in] number of bytes in the HAB command (excluding terminator byte) */
   uint8_t *read_buf,  /* [out] holds the data read from the HAB (must be at least write_len bytes long) */
   bool    bChecksum   /* [in] true = automatically compute checksum */
)
{
   BERR_Code retCode;
   uint8_t sb, i, cs;

   if ((write_len > 127) || (write_len < 2))
      return (BERR_TRACE(BERR_INVALID_PARAMETER));

   BAST_CHK_RETCODE(BAST_4506_P_CheckHab(h));

   /* zero out the HAB */
   sb = 0x0C;
   BAST_CHK_RETCODE(BAST_WriteHostRegister(h, BCM4506_SH_AP_SFR_AP_CMD, &sb));

   /* compute checksum */
   if (bChecksum)
   {
      for (i = cs = 0; i < (write_len - 1); i++)
         cs += write_buf[i];
      write_buf[write_len-1] = cs;
   }

#if 0
   for (i = 0; i < write_len; i++)
   {
      BDBG_MSG(("WriteHab[%d]: 0x%02X", i, write_buf[i]));
   }
#endif

   /* write the command to the HAB */
   BAST_CHK_RETCODE(BAST_4506_P_WriteHab(h, 0, write_buf, write_len));

   /* wait for the AP to service the HAB, and then read any return data */
   BAST_CHK_RETCODE(BAST_4506_P_ServiceHab(h, read_buf, write_len, (uint8_t)(write_buf[0] | 0x80), bChecksum));

   done:
   return retCode;
}


/******************************************************************************
 BAST_4506_P_ServiceHab()
******************************************************************************/
BERR_Code BAST_4506_P_ServiceHab(
   BAST_Handle h,   /* [in] BAST handle */
   uint8_t *read_buf,  /* [out] holds the data read from the HAB */
   uint8_t read_len,   /* [in] number of bytes to read from the HAB (including the checksum byte) */
   uint8_t ack_byte,   /* [in] value of the ack byte to expect */
   bool    bChecksum   /* [in] true = automatically compute checksum */
)
{
   BAST_4506_P_Handle *p4506 = (BAST_4506_P_Handle *)(h->pImpl);
   BERR_Code retCode;
#ifdef BAST_DONT_USE_INTERRUPT
   uint16_t  i;
#endif
   uint8_t   idx, cs, sb;

   /* clear the HAB_DONE interrupt */
   sb = BCM4506_STAT1_HAB_DONE;
   BAST_CHK_RETCODE(BAST_WriteHostRegister(h, BCM4506_SH_AP_SFR_H_FSTAT1, &sb));
   BKNI_WaitForEvent(p4506->hHabDoneEvent, 0);

#ifndef BAST_DONT_USE_INTERRUPT
   /* enable HAB_DONE interrupt */
   BAST_CHK_RETCODE(BAST_ReadHostRegister(h, BCM4506_SH_AP_SFR_H_IE1, &sb));
   sb |= BCM4506_STAT1_HAB_DONE;
   BAST_CHK_RETCODE(BAST_WriteHostRegister(h, BCM4506_SH_AP_SFR_H_IE1, &sb));
#endif

   /* send the command */
   sb = BCM4506_AP_HABR;
   BAST_CHK_RETCODE(BAST_WriteHostRegister(h, BCM4506_SH_AP_SFR_H_CTL1, &sb));

#ifdef BAST_DONT_USE_INTERRUPT
   /* wait for HAB to be serviced (polling) */
   for (i = 0; i < 1111; i++)
   {
      BAST_CHK_RETCODE(BAST_ReadHostRegister(h, BCM4506_SH_AP_SFR_H_STAT1, &sb));
      if (sb & BCM4506_STAT1_HAB_DONE)
         break;
   }
   if ((sb & BCM4506_STAT1_HAB_DONE) == 0)
   {
      BERR_TRACE(retCode = BAST_ERR_HAB_TIMEOUT);
      goto done;
   }
#else
   /* wait for HAB done interrupt */
   if (BAST_4506_P_WaitForEvent(h, p4506->hHabDoneEvent, 500) == BERR_TIMEOUT)
   {
      uint8_t   h_stat[3], h_ie[3];

      BAST_CHK_RETCODE(BREG_I2C_Read(p4506->hRegister, h->settings.i2c.chipAddr, BCM4506_SH_AP_SFR_H_FSTAT1, h_stat, 3));
      BAST_CHK_RETCODE(BREG_I2C_Read(p4506->hRegister, h->settings.i2c.chipAddr, BCM4506_SH_AP_SFR_H_IE1, h_ie, 3));
      BDBG_ERR(("HAB timeout: fstat1=0x%02X, fstat2=%02X, fstat3=0x%02X, ie1=0x%02X, ie2=0x%02X, ie3=0x%02X", h_stat[0], h_stat[1], h_stat[2], h_ie[0], h_ie[1], h_ie[2]));
      BERR_TRACE(retCode = BAST_ERR_HAB_TIMEOUT);
      goto done;
   }
#endif

   /* get the HAB contents */
   BAST_CHK_RETCODE(BAST_4506_P_ReadHab(h, 0, read_buf, read_len));

   /* determine if the command was serviced */
   if (ack_byte != read_buf[0])
   {
      BDBG_ERR(("HAB command not serviced!\n"));
      BERR_TRACE(retCode = BAST_ERR_HAB_NO_ACK);
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
 BAST_4506_P_EnableHostInterrupt()
******************************************************************************/
BERR_Code BAST_4506_P_EnableHostInterrupt(
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
 BAST_4506_P_EnableLockInterrupt()
******************************************************************************/
BERR_Code BAST_4506_P_EnableLockInterrupt(
   BAST_ChannelHandle h, /* [in] BAST channel handle */
   bool bEnable          /* [in] true = enable lock interrupts, false = disables lock interrupts */
)
{
   /* BAST_4506_P_ChannelHandle *p4506 = (BAST_4506_P_ChannelHandle *)(h->pImpl); */
   BERR_Code retCode;
   uint8_t sb, h_stat3, h_ie3;

   /* BAST_4506_P_EnableHostInterrupt(h->pDevice, false); */

   sb = h->channel ? 0x60 : 0x06;
   BAST_CHK_RETCODE(BAST_WriteHostRegister(h->pDevice, BCM4506_SH_AP_SFR_H_FSTAT3, &sb));
   if (bEnable)
   {
      BAST_CHK_RETCODE(BAST_ReadHostRegister(h->pDevice, BCM4506_SH_AP_SFR_H_STAT3, &h_stat3));
      BAST_CHK_RETCODE(BAST_ReadHostRegister(h->pDevice, BCM4506_SH_AP_SFR_H_IE3, &h_ie3));
      if (h->channel)
         h_ie3 |= 0x60; /* ((h_stat3 & BCM4506_STAT3_CH1_RT_LOCK) ? BCM4506_STAT3_CH1_NOT_LOCK : BCM4506_STAT3_CH1_LOCK); */
      else
         h_ie3 |= 0x06; /* ((h_stat3 & BCM4506_STAT3_CH0_RT_LOCK) ? BCM4506_STAT3_CH0_NOT_LOCK : BCM4506_STAT3_CH0_LOCK); */
      BAST_CHK_RETCODE(BAST_WriteHostRegister(h->pDevice, BCM4506_SH_AP_SFR_H_IE3, &h_ie3));

      /* BKNI_WaitForEvent(p4506->hLockChangeEvent, 0); */
   }

   done:
   /* BAST_4506_P_EnableHostInterrupt(h->pDevice, true); */
   return retCode;
}


/******************************************************************************
 BAST_4506_P_EnableFtmInterrupt()
******************************************************************************/
BERR_Code BAST_4506_P_EnableFtmInterrupt(
   BAST_Handle h, /* [in] BAST handle */
   bool bEnable   /* [in] true = enable FTM interrupts, false = disables FTM interrupts */
)
{
   BERR_Code retCode;
   uint8_t sb;

   /* BAST_4506_P_EnableHostInterrupt(h, false); */

   sb = BCM4506_STAT2_FSK;
   BAST_CHK_RETCODE(BAST_WriteHostRegister(h, BCM4506_SH_AP_SFR_H_FSTAT2, &sb));
   if (bEnable)
   {
      BAST_CHK_RETCODE(BAST_ReadHostRegister(h, BCM4506_SH_AP_SFR_H_IE2, &sb));
      sb |= BCM4506_STAT2_FSK;
      BAST_CHK_RETCODE(BAST_WriteHostRegister(h, BCM4506_SH_AP_SFR_H_IE2, &sb));
   }

   done:
   /* BAST_4506_P_EnableHostInterrupt(h, true); */
   return retCode;
}


/******************************************************************************
 BAST_4506_P_DecodeInterrupt()
******************************************************************************/
BERR_Code BAST_4506_P_DecodeInterrupt(BAST_Handle h)
{
   BAST_4506_P_Handle *p4506 = (BAST_4506_P_Handle *)(h->pImpl);
   BAST_4506_P_ChannelHandle *pChn;
   BERR_Code retCode;
   uint8_t   h_fstat[4], h_ie[3], retries = 0, bInitDone = 0;

   read_again:
   BAST_CHK_RETCODE(BREG_I2C_Read(p4506->hRegister, h->settings.i2c.chipAddr, BCM4506_SH_AP_SFR_H_FSTAT1, h_fstat, 4));
   BAST_CHK_RETCODE(BREG_I2C_Read(p4506->hRegister, h->settings.i2c.chipAddr, BCM4506_SH_AP_SFR_H_IE2, h_ie, 3));

   if (!h_fstat[0] && !h_fstat[1] && !h_fstat[2] && !h_fstat[3])
      return BERR_SUCCESS;

   /* sanity check: there shouldn't be any bits in FSTATx that isn't set in IEx */
   if (((h_ie[0] & h_fstat[1]) != h_fstat[1]) || ((h_ie[1] & h_fstat[2]) != h_fstat[2]) || ((h_ie[2] & h_fstat[3]) != h_fstat[3]))
   {
      BDBG_ERR(("BAST_4506_P_DecodeInterrupt() sanity check failed"));
      BDBG_ERR(("FSTAT[1..4]= %02X %02X %02X %02X", h_fstat[0], h_fstat[1], h_fstat[2], h_fstat[3]));
      BDBG_ERR(("IE[2..4]= %02X %02X %02X", h_ie[0], h_ie[1], h_ie[2]));

      if (++retries < 10)
         goto read_again;

      BDBG_ERR(("BAST_4506_P_DecodeInterrupt() aborting"));
      return BERR_SUCCESS;
   }

#if 0
   BDBG_MSG(("FSTAT[1..4]= %02X %02X %02X %02X", h_fstat[0], h_fstat[1], h_fstat[2], h_fstat[3]));
   BDBG_MSG(("IE[2..4]= %02X %02X %02X", h_ie[0], h_ie[1], h_ie[2]));
#endif

   if (h_fstat[0] & BCM4506_STAT1_HAB_DONE)
      BKNI_SetEvent(p4506->hHabDoneEvent);

   if (h_fstat[1] & BCM4506_STAT2_INIT_DONE)
   {
      BKNI_SetEvent(p4506->hInitDoneEvent);
      h_ie[0] &= ~BCM4506_STAT2_INIT_DONE;
      h_fstat[1] &= ~BCM4506_STAT2_INIT_DONE;
      bInitDone = 1;
   }

   if (h_fstat[1] & BCM4506_STAT2_FSK)
   {
      BKNI_SetEvent(p4506->hFtmEvent);
      h_ie[0] |= BCM4506_STAT2_FSK;
   }

   if (h_fstat[2] & 0x07)
   {
      /* channel 0 interrupt */
      pChn = (BAST_4506_P_ChannelHandle *)(h->pChannels[0]->pImpl);
      if (h_fstat[2] & BCM4506_STAT3_CH0_DS_DONE)
      {
         BKNI_SetEvent(pChn->hDiseqcEvent);
         h_ie[1] &= ~BCM4506_STAT3_CH0_DS_DONE;
      }
      if (h_fstat[2] & (BCM4506_STAT3_CH0_LOCK | BCM4506_STAT3_CH0_NOT_LOCK))
      {
         h_ie[1] |= (BCM4506_STAT3_CH0_LOCK | BCM4506_STAT3_CH0_NOT_LOCK);
         BKNI_SetEvent(pChn->hLockChangeEvent);
      }
   }

   if (h_fstat[2] & 0x70)
   {
      /* channel 1 interrupt */
      pChn = (BAST_4506_P_ChannelHandle *)(h->pChannels[1]->pImpl);
      if (h_fstat[2] & BCM4506_STAT3_CH1_DS_DONE)
      {
         BKNI_SetEvent(pChn->hDiseqcEvent);
         h_ie[1] &= ~BCM4506_STAT3_CH1_DS_DONE;
      }
      if (h_fstat[2] & (BCM4506_STAT3_CH1_LOCK | BCM4506_STAT3_CH1_NOT_LOCK))
      {
         h_ie[1] |= (BCM4506_STAT3_CH1_LOCK | BCM4506_STAT3_CH1_NOT_LOCK);
         BKNI_SetEvent(pChn->hLockChangeEvent);
      }
   }

   h_fstat[2] &= ~0x88;

   if (h_fstat[3] & 0x0F)
   {
      /* channel 0 status interrupt */
      pChn = (BAST_4506_P_ChannelHandle *)(h->pChannels[0]->pImpl);
      BKNI_SetEvent(pChn->hStatusEvent);
   }
   else if (h_fstat[3] & 0xF0)
   {
      /* channel 1 status interrupt */
      pChn = (BAST_4506_P_ChannelHandle *)(h->pChannels[1]->pImpl);
      BKNI_SetEvent(pChn->hStatusEvent);
   }

   if (h_fstat[1] & BCM4506_STAT2_PEAK_SCAN_1)
   {
      /* channel 1 peak scan interrupt */
      pChn = (BAST_4506_P_ChannelHandle *)(h->pChannels[1]->pImpl);
      BKNI_SetEvent(pChn->hPeakScanEvent);
      h_ie[0] &= ~BCM4506_STAT2_PEAK_SCAN_1;
   }

   if (h_fstat[1] & BCM4506_STAT2_PEAK_SCAN_0)
   {
      /* channel 0 peak scan interrupt */
      pChn = (BAST_4506_P_ChannelHandle *)(h->pChannels[0]->pImpl);
      BKNI_SetEvent(pChn->hPeakScanEvent);
      h_ie[0] &= ~BCM4506_STAT2_PEAK_SCAN_0;
   }

   /* clear the interrupt status */
   BAST_CHK_RETCODE(BREG_I2C_Write(p4506->hRegister, h->settings.i2c.chipAddr, BCM4506_SH_AP_SFR_H_FSTAT1, h_fstat, 4));

   if (h_ie[0] || h_ie[1] || h_ie[2] || bInitDone)
   {
      BAST_CHK_RETCODE(BREG_I2C_Write(p4506->hRegister, h->settings.i2c.chipAddr, BCM4506_SH_AP_SFR_H_IE2, h_ie, 3));
   }

   done:
   return retCode;
}


/******************************************************************************
 BAST_4506_P_HandleInterrupt_isr()
******************************************************************************/
BERR_Code BAST_4506_P_HandleInterrupt_isr(
   BAST_Handle h /* [in] BAST handle */
)
{
   BDBG_ASSERT(h);

   h->settings.i2c.interruptEnableFunc(false, h->settings.i2c.interruptEnableFuncParam);
   BKNI_SetEvent(((BAST_4506_P_Handle *)(h->pImpl))->hApiEvent);
   BKNI_SetEvent(((BAST_4506_P_Handle *)(h->pImpl))->hInterruptEvent);
   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_4506_P_ProcessInterruptEvent()
******************************************************************************/
BERR_Code BAST_4506_P_ProcessInterruptEvent(
   BAST_Handle h  /* [in] BAST handle */
)
{
   BERR_Code retCode;

   BDBG_ASSERT(h);
   BAST_CHK_RETCODE(BAST_4506_P_DecodeInterrupt(h));
   BAST_4506_P_EnableHostInterrupt(h, true);

   done:
   return retCode;
}


/******************************************************************************
 BAST_4506_P_DisableInterrupts()
******************************************************************************/
BERR_Code BAST_4506_P_DisableInterrupts(
   BAST_Handle h   /* [in] BAST handle */
)
{
   BAST_4506_P_Handle *p4506 = (BAST_4506_P_Handle *)(h->pImpl);
   BERR_Code err;
   const uint8_t val[4] = {0, 0, 0, 0};

   /* clear IEx registers */
   err = BREG_I2C_Write(p4506->hRegister, h->settings.i2c.chipAddr, BCM4506_SH_AP_SFR_H_IE1, val, 4);
   return err;
}



/******************************************************************************
 BERR_Code BAST_4506_P_WaitForEvent()
******************************************************************************/
BERR_Code BAST_4506_P_WaitForEvent(
   BAST_Handle h,             /* [in] BAST handle */
   BKNI_EventHandle hEvent,   /* [in] event to wait on */
   int timeoutMsec            /* [in] timeout in milliseconds */
)
{
   BAST_4506_P_Handle *p4506 = (BAST_4506_P_Handle *)(h->pImpl);
   BERR_Code retCode = BERR_SUCCESS;

#ifndef BAST_ALT_IMPL
   BAST_4506_P_EnableHostInterrupt(h, true);
   while (1)
   {
      retCode = BKNI_WaitForEvent(p4506->hApiEvent, timeoutMsec);

      if ((retCode == BERR_TIMEOUT) || (retCode == BERR_OS_ERROR)) {
         break;
      }
      else if (retCode == BERR_SUCCESS)
      {
         BAST_4506_P_DecodeInterrupt(h);
         if ((retCode = BKNI_WaitForEvent(hEvent, 0)) == BERR_SUCCESS)
            break;
      }
      BAST_4506_P_EnableHostInterrupt(h, true);
   }

   BAST_4506_P_EnableHostInterrupt(h, true);
#else
   /* First of all, verify whether we already got a message */
   BAST_4506_P_DecodeInterrupt(h);
   BAST_4506_P_EnableHostInterrupt(h, true);

   if ((retCode = BKNI_WaitForEvent(hEvent, 0)) != BERR_SUCCESS)
   {
      /* It wasn't the expected message. Wait until timeout */
      while (1)
      {
         retCode = BKNI_WaitForEvent(p4506->hApiEvent, timeoutMsec);

         if ((retCode == BERR_TIMEOUT) || (retCode == BERR_OS_ERROR)) {
             break;
         }
         else if (retCode == BERR_SUCCESS)
         {
            BAST_4506_P_DecodeInterrupt(h);
            BAST_4506_P_EnableHostInterrupt(h, true);
            if ((retCode = BKNI_WaitForEvent(hEvent, 0)) == BERR_SUCCESS)
               break;
         }
      }
   }
#endif
   return retCode;
}


/******************************************************************************
 BAST_4506_P_AbortAcq()
******************************************************************************/
BERR_Code BAST_4506_P_AbortAcq(
   BAST_ChannelHandle h  /* [in] BAST channel handle */
)
{
   uint8_t buf[4];

   buf[0] = 0x10 | (h->channel ? 0x40 : 0x00);
   buf[1] = buf[0];

   return BAST_4506_P_SendHabCommand(h->pDevice, buf, 2, buf, true);
}


/******************************************************************************
 BAST_4506_P_ConfigLna()
******************************************************************************/
BERR_Code BAST_4506_P_ConfigLna(
   BAST_Handle    h,             /* [in] BAST handle */
   BAST_LnaIntConfig int_config, /* [in] BCM3445 internal switch configuration */
   BAST_LnaExtConfig ext_config  /* [in] BCM3445 output mapping to SDS channels */
)
{
#if 0
   uint8_t buf[4];

   if (int_config == BAST_LnaIntConfig_eIn2Out1_In2Out2_DaisyOff)
      return BERR_NOT_SUPPORTED;

   buf[0] = 0x11;
   buf[1] = (uint8_t)(int_config & 0xF);
   buf[1] |= (uint8_t)((ext_config << 4) & 0xF0);

   return BAST_4506_P_SendHabCommand(h, buf, 3, buf, true);
#else
   BERR_Code retCode;
   BAST_Bcm3445Settings settings;
   BAST_Bcm3445OutputChannel out;
   BAST_ChannelHandle hChn0 = h->pChannels[0];
   BAST_ChannelHandle hChn1 = h->pChannels[1];

   BDBG_ASSERT(hChn0);
   BDBG_ASSERT(hChn1);

   settings.mi2c = BAST_Mi2cChannel_e0;
   switch (int_config)
   {
      case BAST_LnaIntConfig_eIn1Out1_In1Out2:
         settings.out1 = BAST_Bcm3445OutputConfig_eIn1Vga;
         settings.out2 = BAST_Bcm3445OutputConfig_eIn1Vga;
         settings.daisy = BAST_Bcm3445OutputConfig_eIn1Vga;
         break;
      case BAST_LnaIntConfig_eIn1Out1_In2Out2:
         settings.out1 = BAST_Bcm3445OutputConfig_eIn1Vga;
         settings.out2 = BAST_Bcm3445OutputConfig_eIn2Vga;
         settings.daisy = BAST_Bcm3445OutputConfig_eIn2Vga;
         break;
      case BAST_LnaIntConfig_eIn2Out1_In2Out2:
         settings.out1 = BAST_Bcm3445OutputConfig_eIn2Vga;
         settings.out2 = BAST_Bcm3445OutputConfig_eIn2Vga;
         settings.daisy = BAST_Bcm3445OutputConfig_eIn2Vga;
         break;
      case BAST_LnaIntConfig_eIn1DbOut1_In2Out2:
         settings.out1 = BAST_Bcm3445OutputConfig_eIn1Db;
         settings.out2 = BAST_Bcm3445OutputConfig_eIn2Vga;
         settings.daisy = BAST_Bcm3445OutputConfig_eIn2Vga;
         break;
      case BAST_LnaIntConfig_eOff:
         settings.out1 = BAST_Bcm3445OutputConfig_eOff;
         settings.out2 = BAST_Bcm3445OutputConfig_eOff;
         settings.daisy = BAST_Bcm3445OutputConfig_eOff;
         break;
      case BAST_LnaIntConfig_eIn2Out1_In1Out2:
         settings.out1 = BAST_Bcm3445OutputConfig_eIn2Vga;
         settings.out2 = BAST_Bcm3445OutputConfig_eIn1Vga;
         settings.daisy = BAST_Bcm3445OutputConfig_eIn1Vga;
         break;
      case BAST_LnaIntConfig_eIn2Out1_In2Out2_DaisyOff:
         settings.out1 = BAST_Bcm3445OutputConfig_eIn2Vga;
         settings.out2 = BAST_Bcm3445OutputConfig_eIn2Vga;
         settings.daisy = BAST_Bcm3445OutputConfig_eOff;
         break;
      default:
         retCode = BERR_NOT_SUPPORTED;

         exit_error:
         return BERR_TRACE(retCode);
   }
   retCode = BAST_4506_P_ConfigBcm3445(h, &settings);
   if (retCode != BERR_SUCCESS)
      goto exit_error;

   /* map tuner0 input */
   if ((ext_config == BAST_LnaExtConfig_eOut1Sds0_Out2Sds1) || (ext_config == BAST_LnaExtConfig_eOut1Sds0_DaisySds1))
      out = BAST_Bcm3445OutputChannel_eOut1;
   else if (ext_config == BAST_LnaExtConfig_eOut1Sds1_Out2Sds0)
      out = BAST_Bcm3445OutputChannel_eOut2;
   else
   {
      retCode = BERR_INVALID_PARAMETER;
      goto exit_error;
   }
   retCode = BAST_4506_P_MapBcm3445ToTuner(hChn0, BAST_Mi2cChannel_e0, out);
   if (retCode != BERR_SUCCESS)
      goto exit_error;

   /* map tuner1 input */
   if ((ext_config == BAST_LnaExtConfig_eOut1Sds0_Out2Sds1) || (ext_config == BAST_LnaExtConfig_eOut1Sds0_DaisySds1))
      out = BAST_Bcm3445OutputChannel_eOut2;
   else if (ext_config == BAST_LnaExtConfig_eOut1Sds1_Out2Sds0)
      out = BAST_Bcm3445OutputChannel_eOut1;
   retCode = BAST_4506_P_MapBcm3445ToTuner(hChn1, BAST_Mi2cChannel_e0, out);
   if (retCode != BERR_SUCCESS)
      goto exit_error;

   return retCode;
#endif
}


/******************************************************************************
 BAST_GetLnaStatus()
******************************************************************************/
BERR_Code BAST_4506_P_GetLnaStatus(
   BAST_Handle h,        /* [in] BAST handle */
   BAST_LnaStatus  *pStatus  /* [out] chip-specific status struct */
)
{
#if 0
   BERR_Code retCode;
   BAST_LnaStatus *pLnaStatus = (BAST_LnaStatus *)pStatus;
   uint8_t buf[8];

   if (pStatus == NULL)
      return (BERR_TRACE(BERR_INVALID_PARAMETER));

   buf[0] = 0x2C;
   BAST_CHK_RETCODE(BAST_4506_P_SendHabCommand(h, buf, 6, buf, true));

   pLnaStatus->agc0 = buf[3];
   pLnaStatus->agc1 = buf[4];
   pLnaStatus->version = buf[2];
   pLnaStatus->int_config = (BAST_LnaIntConfig)(buf[1] & 0x0F);
   pLnaStatus->ext_config = (BAST_LnaExtConfig)((buf[1] >> 4) & 0x0F);

   done:
   return retCode;
#else
   BERR_Code retCode;
   BAST_Bcm3445Status chnStatus;
   BAST_Bcm3445OutputConfig out0, out1;
   BAST_Bcm3445OutputChannel tuner0, tuner1;
   BAST_ChannelHandle hChn0 = h->pChannels[0];
   BAST_ChannelHandle hChn1 = h->pChannels[1];
   BAST_4506_P_Handle *p4506 = (BAST_4506_P_Handle *)(h->pImpl);

   /* initialize struct elements to avoid compiler warnings */
   chnStatus.tuner_input = BAST_Bcm3445OutputChannel_eNone;
   chnStatus.out_cfg = BAST_Bcm3445OutputConfig_eOff;
   chnStatus.version = 0;
   chnStatus.agc = 0;

   retCode = BAST_4506_P_GetBcm3445Status(hChn0, &chnStatus);
   if (retCode != BERR_SUCCESS)
   {
      exit_error:
      return BERR_TRACE(retCode);
   }

   tuner0 = chnStatus.tuner_input;
   out0 = chnStatus.out_cfg;

   pStatus->version = chnStatus.version;
   pStatus->agc0 = chnStatus.agc;

   /* check second channel only if dual tuner chip */
   if (p4506->numTuners > 1)
   {
      retCode = BAST_4506_P_GetBcm3445Status(hChn1, &chnStatus);
      if (retCode != BERR_SUCCESS)
         goto exit_error;

      tuner1 = chnStatus.tuner_input;
      out1 = chnStatus.out_cfg;
      pStatus->agc1 = chnStatus.agc;
   }
   else
   {
      tuner1 = BAST_Bcm3445OutputChannel_eNone;
      out1 = BAST_Bcm3445OutputConfig_eOff;
      pStatus->agc1 = 0;
   }
   /* BDBG_MSG(("BAST_4506_P_GetLnaStatus(): tuner0=%d, out0=%d, tuner1=%d, out1=%d", tuner0, out0, tuner1, out1)); */

   /* determine external config based on 3445 settings */
   if (tuner0 == BAST_Bcm3445OutputChannel_eOut1)
   {
      if (tuner1 == BAST_Bcm3445OutputChannel_eDaisy)
         pStatus->ext_config = BAST_LnaExtConfig_eOut1Sds0_DaisySds1;
      else  /* tuner1 == BAST_Bcm3445OutputChannel_eOut2 */
         pStatus->ext_config = BAST_LnaExtConfig_eOut1Sds0_Out2Sds1;
   }
   else
      pStatus->ext_config = BAST_LnaExtConfig_eOut1Sds1_Out2Sds0;

   /* determine internal config based on 3445 settings */
   if (out0 == BAST_Bcm3445OutputConfig_eOff)
      pStatus->int_config = BAST_LnaIntConfig_eOff;
   else if (out0 == BAST_Bcm3445OutputConfig_eIn1Vga)
   {
      if (out1 == BAST_Bcm3445OutputConfig_eIn1Vga)
         pStatus->int_config = BAST_LnaIntConfig_eIn1Out1_In1Out2;
      else  /* out1 == BAST_Bcm3445OutputConfig_eIn2Vga */
         pStatus->int_config = BAST_LnaIntConfig_eIn1Out1_In2Out2;
   }
   else if (out0 == BAST_Bcm3445OutputConfig_eIn1Db)
      pStatus->int_config = BAST_LnaIntConfig_eIn1DbOut1_In2Out2;
   else  /* out0 = BAST_Bcm3445OutputConfig_eIn2Vga */
   {
      if (out1 == BAST_Bcm3445OutputConfig_eIn1Vga)
         pStatus->int_config = BAST_LnaIntConfig_eIn2Out1_In1Out2;
      else if (out1 == BAST_Bcm3445OutputConfig_eIn2Vga)
         pStatus->int_config = BAST_LnaIntConfig_eIn2Out1_In2Out2;
      else
         pStatus->int_config = BAST_LnaIntConfig_eIn2Out1_In2Out2_DaisyOff;
   }

   return retCode;
#endif
}


/******************************************************************************
 BAST_4506_P_WriteHostIndirectRegister()
******************************************************************************/
BERR_Code BAST_4506_P_WriteHostIndirectRegister(
   BAST_Handle h,    /* [in] BAST handle */
   uint8_t     reg,  /* [in] address of indirect register */
   uint8_t     *pVal /* [in] value to write */
)
{
   BERR_Code retCode;
   uint8_t sb;

   sb = reg;
   BAST_CHK_RETCODE(BAST_WriteHostRegister(h, BCM4506_SH_SFR_H_LOCAL_ADR, &sb));

   sb = *pVal;
   BAST_CHK_RETCODE(BAST_WriteHostRegister(h, BCM4506_SH_SFR_H_LOCAL_DAT, &sb));

   done:
   return retCode;
}


/******************************************************************************
 BAST_4506_P_ReadHostIndirectRegister()
******************************************************************************/
BERR_Code BAST_4506_P_ReadHostIndirectRegister(
   BAST_Handle h,    /* [in] BAST handle */
   uint8_t     reg,  /* [in] address of indirect register */
   uint8_t     *pVal /* [out] value read */
)
{
   BERR_Code retCode;
   uint8_t sb;

   sb = reg;
   BAST_CHK_RETCODE(BAST_WriteHostRegister(h, BCM4506_SH_SFR_H_LOCAL_ADR, &sb));

   BAST_CHK_RETCODE(BAST_ReadHostRegister(h, BCM4506_SH_SFR_H_LOCAL_DAT, pVal));

   done:
   return retCode;
}


/******************************************************************************
 BAST_4506_P_PeakScan()
******************************************************************************/
BERR_Code BAST_4506_P_PeakScan(
   BAST_ChannelHandle h,           /* [in] BAST channel handle */
   uint32_t           tunerFreq    /* [in] frequency in Hz */
)
{
   BERR_Code retCode;
   uint8_t buf[8], ie, sb;

   /* enable peak scan interrupt */
   sb = h->channel ? BCM4506_STAT2_PEAK_SCAN_1 : BCM4506_STAT2_PEAK_SCAN_0;
   BAST_CHK_RETCODE(BAST_WriteHostRegister(h->pDevice, BCM4506_SH_AP_SFR_H_FSTAT2, &sb));
   BAST_CHK_RETCODE(BAST_ReadHostRegister(h->pDevice, BCM4506_SH_AP_SFR_H_IE2, &ie));
   ie |= sb;
   BAST_CHK_RETCODE(BAST_WriteHostRegister(h->pDevice, BCM4506_SH_AP_SFR_H_IE2, &ie));

   buf[0] = 0x2E | (h->channel ? 0x40 : 0x00);
   buf[1] = (uint8_t)((tunerFreq >> 24) & 0xFF);
   buf[2] = (uint8_t)((tunerFreq >> 16) & 0xFF);
   buf[3] = (uint8_t)((tunerFreq >> 8) & 0xFF);
   buf[4] = (uint8_t)(tunerFreq & 0xFF);
   retCode = BAST_4506_P_SendHabCommand(h->pDevice, buf, 6, buf, true);

   done:
   return retCode;
}


/******************************************************************************
 BAST_4506_P_GetPeakScanStatus()
******************************************************************************/
BERR_Code BAST_4506_P_GetPeakScanStatus(
   BAST_ChannelHandle  h,          /* [in] BAST channel handle */
   BAST_PeakScanStatus *pStatus    /* [out] Diseqc event handle */
)
{
   uint8_t buf[16];
   BERR_Code retCode;

   buf[0] = 0x2F | (h->channel ? 0x40 : 0x00);
   retCode = BAST_4506_P_SendHabCommand(h->pDevice, buf, 15, buf, true);
   if (retCode != BERR_SUCCESS)
      return retCode;

   pStatus->status = (uint32_t)buf[1];
   pStatus->tunerFreq = (buf[2] << 24) | (buf[3] << 16) | (buf[4] << 8) | buf[5];
   pStatus->out = (buf[6] << 24) | (buf[7] << 16) | (buf[8] << 8) | buf[9];
   pStatus->peakPower = (buf[10] << 24) | (buf[11] << 16) | (buf[12] << 8) | buf[13];
   return retCode;
}


/******************************************************************************
 BAST_4506_P_GetPeakScanEventHandle()
******************************************************************************/
BERR_Code BAST_4506_P_GetPeakScanEventHandle(
   BAST_ChannelHandle h,                /* [in] BAST channel handle */
   BKNI_EventHandle  *hPeakScanEvent    /* [out] Peak scan done event handle */
)
{
   *hPeakScanEvent = ((BAST_4506_P_ChannelHandle *)(h->pImpl))->hPeakScanEvent;
   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_4506_P_EnableStatusInterrupts()
******************************************************************************/
BERR_Code BAST_4506_P_EnableStatusInterrupts(
   BAST_ChannelHandle h,
   bool bEnable
)
{
   BERR_Code retCode;
   uint8_t sb, h_ie4;

   sb = h->channel ? 0xF0 : 0x0F;
   BAST_CHK_RETCODE(BAST_WriteHostRegister(h->pDevice, BCM4506_SH_AP_SFR_H_FSTAT4, &sb));
   if (bEnable)
   {
      BAST_CHK_RETCODE(BAST_ReadHostRegister(h->pDevice, BCM4506_SH_AP_SFR_H_IE4, &h_ie4));
      h_ie4 |= sb;
      BAST_CHK_RETCODE(BAST_WriteHostRegister(h->pDevice, BCM4506_SH_AP_SFR_H_IE4, &h_ie4));
   }

   done:
   return retCode;
}


/******************************************************************************
 BAST_4506_P_GetStatusEventHandle()
******************************************************************************/
BERR_Code BAST_4506_P_GetStatusEventHandle(
   BAST_ChannelHandle h,        /* [in] BAST channel handle */
   BKNI_EventHandle  *hEvent    /* [out] status event handle */
)
{
   *hEvent = ((BAST_4506_P_ChannelHandle *)(h->pImpl))->hStatusEvent;
   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_4506_P_ConfigBcm3445()
******************************************************************************/
BERR_Code BAST_4506_P_ConfigBcm3445(
   BAST_Handle h,                  /* [in] BAST device handle */
   BAST_Bcm3445Settings *pSettings /* [in] BCM3445 configuration settings */
)
{
   uint8_t buf[8];

   BDBG_ASSERT(pSettings);
   if (pSettings->mi2c == BAST_Mi2cChannel_e2)
      return (BERR_TRACE(BERR_INVALID_PARAMETER));
   if (pSettings->daisy == BAST_Bcm3445OutputConfig_eDaisy)
      return (BERR_TRACE(BERR_INVALID_PARAMETER));

   buf[0] = 0x30 | (pSettings->mi2c == BAST_Mi2cChannel_e1 ? 0x40 : 0x00);
   buf[1] = (uint8_t)(pSettings->out1);
   buf[2] = (uint8_t)(pSettings->out2);
   buf[3] = (uint8_t)(pSettings->daisy);
   /* BDBG_MSG(("BAST_4506_P_ConfigBcm3445(): mi2c=%d, out1=%d, out2=%d, daisy=%d", pSettings->mi2c, buf[1], buf[2], buf[3])); */
   return BAST_4506_P_SendHabCommand(h, buf, 5, buf, true);
}


/******************************************************************************
 BAST_4506_P_MapBcm3445ToTuner()
******************************************************************************/
BERR_Code BAST_4506_P_MapBcm3445ToTuner(
   BAST_ChannelHandle        h,    /* [in] BAST channel handle */
   BAST_Mi2cChannel          mi2c, /* [in] specifies which BCM3445 */
   BAST_Bcm3445OutputChannel out   /* [in] BCM3445 output channel */
)
{
   uint8_t buf[4];

   BDBG_ASSERT(h);
   if (mi2c == BAST_Mi2cChannel_e2)
      return (BERR_TRACE(BERR_INVALID_PARAMETER));

   buf[0] = 0x31 | (h->channel ? 0x40 : 0x00);
   buf[1] = (uint8_t)mi2c;
   buf[2] = (uint8_t)out;
   /* BDBG_MSG(("BAST_4506_P_MapBcm3445ToTuner(): chn=%d, port=%d", h->channel, x)); */
   return BAST_4506_P_SendHabCommand(h->pDevice, buf, 4, buf, true);
}


/******************************************************************************
 BAST_4506_P_GetBcm3445Status()
******************************************************************************/
BERR_Code BAST_4506_P_GetBcm3445Status(
   BAST_ChannelHandle h,       /* [in] BAST channel handle */
   BAST_Bcm3445Status *pStatus /* [out] BCM3445 status struct */
)
{
   BERR_Code retCode;
   uint8_t buf[8];

   BDBG_ASSERT(h);
   if (pStatus == NULL)
      return (BERR_TRACE(BERR_INVALID_PARAMETER));

   buf[0] = 0x32 | (h->channel ? 0x40 : 0x00);
   BAST_CHK_RETCODE(BAST_4506_P_SendHabCommand(h->pDevice, buf, 8, buf, true));

   /* no status if outputs not mapped */
   if (buf[2] == 0)
      return BERR_TRACE(BERR_NOT_INITIALIZED);

   pStatus->mi2c = buf[1];
   pStatus->tuner_input = buf[2];
   pStatus->out_cfg = buf[3];
   pStatus->status = buf[4];
   pStatus->version = buf[5];
   pStatus->agc = buf[6];

   done:
   return retCode;
}


/******************************************************************************
 BAST_4501_P_ReadIdata()
******************************************************************************/
BERR_Code BAST_4506_P_ReadIdata(BAST_Handle h, uint8_t page, uint8_t addr, uint8_t *buf, uint8_t n)
{
   BERR_Code retCode;
   BAST_ApStatus status;
   uint8_t hab[128], i;

   if ((n == 0) || (n > 0x7B))
      return BERR_TRACE(BERR_INVALID_PARAMETER);

   BAST_CHK_RETCODE(BAST_4506_P_GetApStatus(h, &status));
   if ((status & BAST_APSTATUS_RUN_MASK) != BAST_APSTATUS_RUN)
      return BERR_TRACE(BERR_NOT_INITIALIZED); /* AP is not running */

   hab[0] = 0x03;
   hab[1] = addr;
   hab[2] = page;
   hab[3] = n;
   BAST_CHK_RETCODE(BAST_4506_P_SendHabCommand(h, hab, (uint8_t)(5+n), hab, true));
   for (i = 0; i < n; i++)
      buf[i] = hab[4+i];

   done:
   return retCode;
}


/******************************************************************************
 BAST_4501_P_ReadXdataPage()
******************************************************************************/
BERR_Code BAST_4506_P_ReadXdataPage(BAST_Handle h, uint8_t page, uint16_t offset, uint8_t *buf, uint8_t n)
{
   BERR_Code retCode;
   BAST_ApStatus status;
   uint8_t hab[128], i;

   if ((n == 0) || (n > 0x7A))
      return BERR_TRACE(BERR_INVALID_PARAMETER);

   BAST_CHK_RETCODE(BAST_4506_P_GetApStatus(h, &status));
   if ((status & BAST_APSTATUS_RUN_MASK) != BAST_APSTATUS_RUN)
      return BERR_TRACE(BERR_NOT_INITIALIZED); /* AP is not running */

   hab[0] = 0x1D;
   hab[1] = page;
   hab[2] = offset >> 8;
   hab[3] = (offset & 0xFF);
   hab[4] = n;
   for (i = 0; i < n; i++)
      hab[5+i] = 0;

   BAST_CHK_RETCODE(BAST_4506_P_SendHabCommand(h, hab, (uint8_t)(6+n), hab, true));
   for (i = 0; i < n; i++)
      buf[i] = hab[5+i];

   done:
   return retCode;
}


#ifdef BAST_DEBUG
/******************************************************************************
 BAST_4506_P_CallApFunction()
******************************************************************************/
BERR_Code BAST_4506_P_CallApFunction(BAST_ChannelHandle h, uint8_t funct_id)
{
   uint8_t buf[4];

   if (funct_id > 2)
      return BERR_TRACE(BERR_INVALID_PARAMETER);

   buf[0] = 0x11 | (h->channel ? 0x40 : 0x00);
   buf[1] = funct_id;
   return BAST_4506_P_SendHabCommand(h->pDevice, buf, 3, buf, true);
}
#endif


/******************************************************************************
 BAST_4506_P_SetSearchRange()
******************************************************************************/
BERR_Code BAST_4506_P_SetSearchRange(BAST_Handle h, uint32_t searchRange)
{
   uint8_t buf[4];

   buf[0] = (uint8_t)((searchRange >> 24) & 0xFF);
   buf[1] = (uint8_t)((searchRange >> 16) & 0xFF);
   buf[2] = (uint8_t)((searchRange >> 8) & 0xFF);
   buf[3] = (uint8_t)(searchRange & 0xFF);
   return BAST_4506_P_WriteConfig(h->pChannels[0], BCM4506_CONFIG_SEARCH_RANGE, buf, BCM4506_CONFIG_LEN_SEARCH_RANGE);
}


/******************************************************************************
 BAST_4506_P_GetSearchRange()
******************************************************************************/
BERR_Code BAST_4506_P_GetSearchRange(BAST_Handle h, uint32_t *pSearchRange)
{
   BERR_Code retCode;
   uint8_t buf[4];

   retCode = BAST_4506_P_ReadConfig(h->pChannels[0], BCM4506_CONFIG_SEARCH_RANGE, buf, BCM4506_CONFIG_LEN_SEARCH_RANGE);
   if (retCode == BERR_SUCCESS)
      *pSearchRange = (uint32_t)((buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3]);
   else
      *pSearchRange = 0;
   return retCode;
}


/******************************************************************************
 BAST_4506_P_SetAmcScramblingSeq()
******************************************************************************/
BERR_Code BAST_4506_P_SetAmcScramblingSeq(BAST_ChannelHandle h, uint32_t xseed, uint32_t plhdrscr1, uint32_t plhdrscr2, uint32_t plhdrscr3)
{
   uint8_t buf[16];

   buf[0] = (uint8_t)((xseed >> 24) & 0xFF);
   buf[1] = (uint8_t)((xseed >> 16) & 0xFF);
   buf[2] = (uint8_t)((xseed >> 8) & 0xFF);
   buf[3] = (uint8_t)(xseed & 0xFF);
   buf[4] = (uint8_t)((plhdrscr1 >> 24) & 0xFF);
   buf[5] = (uint8_t)((plhdrscr1 >> 16) & 0xFF);
   buf[6] = (uint8_t)((plhdrscr1 >> 8) & 0xFF);
   buf[7] = (uint8_t)(plhdrscr1 & 0xFF);
   buf[8] = (uint8_t)((plhdrscr2 >> 24) & 0xFF);
   buf[9] = (uint8_t)((plhdrscr2 >> 16) & 0xFF);
   buf[10] = (uint8_t)((plhdrscr2 >> 8) & 0xFF);
   buf[11] = (uint8_t)(plhdrscr2 & 0xFF);
   buf[12] = (uint8_t)((plhdrscr3 >> 24) & 0xFF);
   buf[13] = (uint8_t)((plhdrscr3 >> 16) & 0xFF);
   buf[14] = (uint8_t)((plhdrscr3 >> 8) & 0xFF);
   buf[15] = (uint8_t)(plhdrscr3 & 0xFF);
   return BAST_4506_P_WriteConfig(h, BCM4506_CONFIG_LDPC_SCRAMBLING_SEQ, buf, BCM4506_CONFIG_LEN_LDPC_SCRAMBLING_SEQ);
}


/******************************************************************************
 BAST_4506_P_SetTunerFilter()
******************************************************************************/
BERR_Code BAST_4506_P_SetTunerFilter(BAST_ChannelHandle h, uint32_t cutoffHz)
{
   BERR_Code retCode;
   bool bAuto = false;
   uint8_t tuner_ctl, cutoffMhz = 0;

   if (cutoffHz == 0)
      bAuto = true;
   else
   {
      cutoffMhz = (uint8_t)((cutoffHz + 500000) / 1000000); /* round to nearest MHz */
      retCode = BAST_4506_P_WriteConfig(h, BCM4506_CONFIG_TUNER_CUTOFF, &cutoffMhz, BCM4506_CONFIG_LEN_TUNER_CUTOFF);
      if (retCode)
         goto done;
   }

   retCode = BAST_4506_P_ReadConfig(h, BCM4506_CONFIG_TUNER_CTL, &tuner_ctl, BCM4506_CONFIG_LEN_TUNER_CTL);
   if (retCode)
      goto done;
   if (bAuto)
      tuner_ctl &= ~0x08;
   else
      tuner_ctl |= 0x08;
   retCode = BAST_4506_P_WriteConfig(h, BCM4506_CONFIG_TUNER_CTL, &tuner_ctl, BCM4506_CONFIG_LEN_TUNER_CTL);

   done:
   return retCode;
}


/******************************************************************************
 BAST_4506_P_GetSignalDetectStatus()
******************************************************************************/
BERR_Code BAST_4506_P_GetSignalDetectStatus(BAST_ChannelHandle h, BAST_SignalDetectStatus *pStatus)
{
   BERR_Code retCode;
   uint8_t sb;

   retCode = BAST_4506_P_ReadConfig(h, BCM4506_CONFIG_TIMING_LOOP_LOCK, &sb, BCM4506_CONFIG_LEN_TIMING_LOOP_LOCK);
   if (retCode != BERR_SUCCESS)
      return retCode;

   pStatus->bDone = pStatus->bEnabled = pStatus->bTimingLoopLocked = false;
   if (sb & 0x80)
   {
      pStatus->bEnabled = true;
      if ((sb & 0x40) == 0)
      {
         pStatus->bDone = true;
         if (sb & 0x01)
            pStatus->bTimingLoopLocked = true;
      }
   }
   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_4506_P_SetOutputTransportSettings()
******************************************************************************/
BERR_Code BAST_4506_P_SetOutputTransportSettings(BAST_ChannelHandle h, BAST_OutputTransportSettings *pSettings)
{
   uint16_t xportCtl = 0;
   uint8_t buf[2];

   if (pSettings->bSerial)
      xportCtl |= BCM4506_XPORT_CTL_SERIAL;
   if (pSettings->bClkInv)
      xportCtl |= BCM4506_XPORT_CTL_CLKINV;
   if (pSettings->bClkSup)
      xportCtl |= BCM4506_XPORT_CTL_CLKSUP;
   if (pSettings->bVldInv)
      xportCtl |= BCM4506_XPORT_CTL_VLDINV;
   if (pSettings->bSyncInv)
      xportCtl |= BCM4506_XPORT_CTL_SYNCINV;
   if (pSettings->bErrInv)
      xportCtl |= BCM4506_XPORT_CTL_ERRINV;
   if (pSettings->bXbert)
      xportCtl |= BCM4506_XPORT_CTL_XBERT;
   if (pSettings->bTei)
      xportCtl |= BCM4506_XPORT_CTL_TEI;
   if (pSettings->bDelay)
      xportCtl |= BCM4506_XPORT_CTL_DELAY;
   if (pSettings->bSync1)
      xportCtl |= BCM4506_XPORT_CTL_SYNC1;
   if (pSettings->bHead4)
      xportCtl |= BCM4506_XPORT_CTL_HEAD4;
   if (pSettings->bDelHeader)
      xportCtl |= BCM4506_XPORT_CTL_DELH;
   if (pSettings->bOpllBypass)
      xportCtl |= BCM4506_XPORT_CTL_BYPASS_OPLL;
   if (pSettings->bchMpegErrorMode == BAST_BchMpegErrorMode_eBchOrCrc8)
      xportCtl |= 0;
   else if (pSettings->bchMpegErrorMode == BAST_BchMpegErrorMode_eCrc8)
      xportCtl |= BCM4506_XPORT_CTL_CRC8_CHECK;
   else if (pSettings->bchMpegErrorMode == BAST_BchMpegErrorMode_eBch)
      xportCtl |= BCM4506_XPORT_CTL_BCH_CHECK;
   else
      xportCtl |= (BCM4506_XPORT_CTL_CRC8_CHECK | BCM4506_XPORT_CTL_BCH_CHECK);

   buf[0] = (xportCtl >> 8) & 0xFF;
   buf[1] = (xportCtl & 0xFF);
   return BAST_4506_P_WriteConfig(h, BCM4506_CONFIG_XPORT_CTL, buf, BCM4506_CONFIG_LEN_XPORT_CTL);
}


/******************************************************************************
 BAST_4506_P_GetOutputTransportSettings()
******************************************************************************/
BERR_Code BAST_4506_P_GetOutputTransportSettings(BAST_ChannelHandle h, BAST_OutputTransportSettings *pSettings)
{
   BERR_Code retCode;
   uint16_t xportCtl;
   uint8_t buf[2];

   retCode = BAST_4506_P_ReadConfig(h, BCM4506_CONFIG_XPORT_CTL, buf, BCM4506_CONFIG_LEN_XPORT_CTL);;
   if (retCode != BERR_SUCCESS)
      return retCode;

   xportCtl = (buf[0] << 8) | buf[1];

   if (xportCtl & BCM4506_XPORT_CTL_SERIAL)
      pSettings->bSerial = true;
   else
      pSettings->bSerial = false;
   if (xportCtl & BCM4506_XPORT_CTL_CLKINV)
      pSettings->bClkInv = true;
   else
      pSettings->bClkInv = false;
   if (xportCtl & BCM4506_XPORT_CTL_CLKSUP)
      pSettings->bClkSup = true;
   else
      pSettings->bClkSup = false;
   if (xportCtl & BCM4506_XPORT_CTL_VLDINV)
      pSettings->bVldInv = true;
   else
      pSettings->bVldInv = false;
   if (xportCtl & BCM4506_XPORT_CTL_SYNCINV)
      pSettings->bSyncInv = true;
   else
      pSettings->bSyncInv = false;
   if (xportCtl & BCM4506_XPORT_CTL_ERRINV)
      pSettings->bErrInv = true;
   else
      pSettings->bErrInv = false;
   if (xportCtl & BCM4506_XPORT_CTL_XBERT)
      pSettings->bXbert = true;
   else
      pSettings->bXbert = false;
   if (xportCtl & BCM4506_XPORT_CTL_TEI)
      pSettings->bTei = true;
   else
      pSettings->bTei = false;
   if (xportCtl & BCM4506_XPORT_CTL_DELAY)
      pSettings->bDelay = true;
   else
      pSettings->bDelay = false;
   if (xportCtl & BCM4506_XPORT_CTL_SYNC1)
      pSettings->bSync1 = true;
   else
      pSettings->bSync1 = false;
   if (xportCtl & BCM4506_XPORT_CTL_HEAD4)
      pSettings->bHead4 = true;
   else
      pSettings->bHead4 = false;
   if (xportCtl & BCM4506_XPORT_CTL_DELH)
      pSettings->bDelHeader = true;
   else
      pSettings->bDelHeader = false;
   if (xportCtl & BCM4506_XPORT_CTL_BYPASS_OPLL)
      pSettings->bOpllBypass = true;
   else
      pSettings->bOpllBypass = false;

   if ((xportCtl & BCM4506_XPORT_CTL_CRC8_CHECK) &&  ((xportCtl & BCM4506_XPORT_CTL_BCH_CHECK)==0))
      pSettings->bchMpegErrorMode = BAST_BchMpegErrorMode_eCrc8;
   else if ((xportCtl & BCM4506_XPORT_CTL_BCH_CHECK) &&  ((xportCtl & BCM4506_XPORT_CTL_CRC8_CHECK)==0))
      pSettings->bchMpegErrorMode = BAST_BchMpegErrorMode_eBch;
   else if ((xportCtl & BCM4506_XPORT_CTL_CRC8_CHECK) &&  (xportCtl & BCM4506_XPORT_CTL_BCH_CHECK))
      pSettings->bchMpegErrorMode = BAST_BchMpegErrorMode_eBchAndCrc8;
   else
      pSettings->bchMpegErrorMode = BAST_BchMpegErrorMode_eBchOrCrc8;

   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_4506_P_SetDiseqcSettings()
******************************************************************************/
BERR_Code BAST_4506_P_SetDiseqcSettings(BAST_ChannelHandle h, BAST_DiseqcSettings *pSettings)
{
   BERR_Code retCode;
   uint8_t diseqcCtl = 0;
   uint8_t rrto[4];

   if (pSettings->bEnvelope)
      diseqcCtl |= BCM4506_DISEQC_CTL2_ENVELOPE;
   if (pSettings->bToneAlign)
      diseqcCtl |= BCM4506_DISEQC_CTL2_TONE_ALIGN;
   if (pSettings->bDisableRRTO)
      diseqcCtl |= BCM4506_DISEQC_CTL2_DISABLE_RRTO;
   if (pSettings->bEnableToneburst)
      diseqcCtl |= BCM4506_DISEQC_CTL2_TB_ENABLE;
   if (pSettings->bToneburstB)
      diseqcCtl |= BCM4506_DISEQC_CTL2_TB_B;
   if (pSettings->bOverrideFraming)
   {
      if (pSettings->bExpectReply)
         diseqcCtl |= BCM4506_DISEQC_CTL2_OVERRIDE_REPLY_BIT;
      else
         diseqcCtl |= BCM4506_DISEQC_CTL2_EXP_REPLY_DISABLE;
   }
   if (pSettings->bEnableLNBPU)
      pSettings->bEnableLNBPU = false; /* not implemented */
   if (pSettings->bDisableRxOnly)
      diseqcCtl |= BCM4506_DISEQC_CTL2_DISABLE_RX_ONLY;
   BAST_CHK_RETCODE(BAST_4506_P_WriteConfig(h, BCM4506_CONFIG_DISEQC_CTL2, &diseqcCtl, BCM4506_CONFIG_LEN_DISEQC_CTL2));

   /* set rrto */
   rrto[0] = (uint8_t)(pSettings->rrtoUsec >> 24);
   rrto[1] = (uint8_t)(pSettings->rrtoUsec >> 16);
   rrto[2] = (uint8_t)(pSettings->rrtoUsec >> 8);
   rrto[3] = (uint8_t)(pSettings->rrtoUsec & 0xFF);
   BAST_CHK_RETCODE(BAST_4506_P_WriteConfig(h, BCM4506_CONFIG_RRTO_USEC, rrto, BCM4506_CONFIG_LEN_RRTO_USEC));

   /* set rx bit threshold */
   pSettings->bitThreshold = 0;  /* not implemented */

   /* set tone detect threshold */
   BAST_CHK_RETCODE(BAST_4506_P_WriteConfig(h, BCM4506_CONFIG_DISEQC_TONE_THRESHOLD, &(pSettings->toneThreshold), BCM4506_CONFIG_LEN_DISEQC_TONE_THRESHOLD));

   /* set pretx delay */
   BAST_CHK_RETCODE(BAST_4506_P_WriteConfig(h, BCM4506_CONFIG_DISEQC_PRETX_DELAY, &(pSettings->preTxDelay), BCM4506_CONFIG_LEN_DISEQC_PRETX_DELAY));

   /* set vsense thresholds */
   pSettings->vsenseThresholdHi = 0;   /* not implemented */
   pSettings->vsenseThresholdLo = 0;   /* not implemented */

   done:
   return retCode;
}


/******************************************************************************
 BAST_4506_P_GetDiseqcSettings()
******************************************************************************/
BERR_Code BAST_4506_P_GetDiseqcSettings(BAST_ChannelHandle h, BAST_DiseqcSettings *pSettings)
{
   BERR_Code retCode;
   uint8_t diseqcCtl;
   uint8_t buf[4];

   BAST_CHK_RETCODE(BAST_4506_P_ReadConfig(h, BCM4506_CONFIG_DISEQC_CTL2, &diseqcCtl, BCM4506_CONFIG_LEN_DISEQC_CTL2));
   if (diseqcCtl & BCM4506_DISEQC_CTL2_ENVELOPE)
      pSettings->bEnvelope = true;
   else
      pSettings->bEnvelope = false;
   if (diseqcCtl & BCM4506_DISEQC_CTL2_TONE_ALIGN)
      pSettings->bToneAlign = true;
   else
      pSettings->bToneAlign = false;
   if (diseqcCtl & BCM4506_DISEQC_CTL2_DISABLE_RRTO)
      pSettings->bDisableRRTO = true;
   else
      pSettings->bDisableRRTO = false;
   if (diseqcCtl & BCM4506_DISEQC_CTL2_TB_ENABLE)
      pSettings->bEnableToneburst = true;
   else
      pSettings->bEnableToneburst = false;
   if (diseqcCtl & BCM4506_DISEQC_CTL2_TB_B)
      pSettings->bToneburstB = true;
   else
      pSettings->bToneburstB = false;
   if (diseqcCtl & BCM4506_DISEQC_CTL2_EXP_REPLY_DISABLE)
   {
      pSettings->bOverrideFraming = true;
      pSettings->bExpectReply = false;
   }
   else if (diseqcCtl & BCM4506_DISEQC_CTL2_OVERRIDE_REPLY_BIT)
   {
      pSettings->bOverrideFraming = true;
      pSettings->bExpectReply = true;
   }
   else
   {
      pSettings->bOverrideFraming = false;
      pSettings->bExpectReply = false;
   }
   pSettings->bEnableLNBPU = false; /* not implemented */
   if (diseqcCtl & BCM4506_DISEQC_CTL2_DISABLE_RX_ONLY)
      pSettings->bDisableRxOnly = true;
   else
      pSettings->bDisableRxOnly = false;

   /* get rrto */
   BAST_CHK_RETCODE(BAST_4506_P_ReadConfig(h, BCM4506_CONFIG_RRTO_USEC, buf, BCM4506_CONFIG_LEN_RRTO_USEC));
   pSettings->rrtoUsec = (buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3];
   pSettings->rrtoUsec &= 0xFFFFF;

   /* get rx bit threshold */
   pSettings->bitThreshold = 0;  /* not implemented */

   /* get tone detect threshold */
   BAST_CHK_RETCODE(BAST_4506_P_ReadConfig(h, BCM4506_CONFIG_DISEQC_TONE_THRESHOLD, &(pSettings->toneThreshold), BCM4506_CONFIG_LEN_DISEQC_TONE_THRESHOLD));

   /* get pretx delay */
   BAST_CHK_RETCODE(BAST_4506_P_ReadConfig(h, BCM4506_CONFIG_DISEQC_PRETX_DELAY, &(pSettings->preTxDelay), BCM4506_CONFIG_LEN_DISEQC_PRETX_DELAY));

   /* get vsense thresholds */
   pSettings->vsenseThresholdHi = 0;   /* not implemented */
   pSettings->vsenseThresholdLo = 0;   /* not implemented */

   done:
   return retCode;
}


/******************************************************************************
 BAST_4506_P_SetNetworkSpec()
******************************************************************************/
BERR_Code BAST_4506_P_SetNetworkSpec(BAST_Handle h, BAST_NetworkSpec s)
{
   uint8_t sb;

   if (s > BAST_NetworkSpec_eEuro)
      return BERR_INVALID_PARAMETER;

   h->settings.networkSpec = s;
   sb = (uint8_t)s;
   return BAST_WriteConfig(h->pChannels[0], BCM4506_CONFIG_NETWORK_SPEC, &sb, BCM4506_CONFIG_LEN_NETWORK_SPEC);
}


/******************************************************************************
 BAST_4506_P_GetNetworkSpec()
******************************************************************************/
BERR_Code BAST_4506_P_GetNetworkSpec(BAST_Handle h, BAST_NetworkSpec *p)
{
   BERR_Code retCode;
   uint8_t sb;

   retCode = BAST_ReadConfig(h->pChannels[0], BCM4506_CONFIG_NETWORK_SPEC, &sb, BCM4506_CONFIG_LEN_NETWORK_SPEC);
   if (retCode == BERR_SUCCESS)
   {
      h->settings.networkSpec = (BAST_NetworkSpec)sb;
      *p = h->settings.networkSpec;
   }
   return retCode;
}


/******************************************************************************
 BAST_4506_P_SetFskChannel()
******************************************************************************/
BERR_Code BAST_4506_P_SetFskChannel(BAST_Handle h, BAST_FskChannel fskTxChannel, BAST_FskChannel fskRxChannel)
{
   uint8_t sb;

   if ((fskTxChannel > BAST_FskChannel_e1) || (fskRxChannel > BAST_FskChannel_e1))
      return BERR_INVALID_PARAMETER;

   /* the following configuration is not supported */
   if ((fskTxChannel == BAST_FskChannel_e1) && (fskRxChannel == BAST_FskChannel_e0))
      return BERR_NOT_SUPPORTED;

   if ((fskTxChannel == BAST_FskChannel_e0) && (fskRxChannel == BAST_FskChannel_e0))
      sb = BAST_FskChannelConfig_eCh0Tx_Ch0Rx;
   else if ((fskTxChannel == BAST_FskChannel_e0) && (fskRxChannel == BAST_FskChannel_e1))
      sb = BAST_FskChannelConfig_eCh0Tx_Ch1Rx;
   else
      sb = BAST_FskChannelConfig_eCh1Tx_Ch1Rx;

   return BAST_WriteConfig(h->pChannels[0], BCM4506_CONFIG_FTM_CH_SELECT, &sb, BCM4506_CONFIG_LEN_FTM_CH_SELECT);
}


/******************************************************************************
 BAST_4506_P_GetFskChannel()
******************************************************************************/
BERR_Code BAST_4506_P_GetFskChannel(BAST_Handle h, BAST_FskChannel *fskTxChannel, BAST_FskChannel *fskRxChannel)
{
   BERR_Code retCode;
   uint8_t sb;

   retCode = BAST_ReadConfig(h->pChannels[0], BCM4506_CONFIG_FTM_CH_SELECT, &sb, BCM4506_CONFIG_LEN_FTM_CH_SELECT);
   if (retCode == BERR_SUCCESS)
   {
      if (sb == BAST_FskChannelConfig_eCh0Tx_Ch0Rx)
      {
         *fskTxChannel = BAST_FskChannel_e0;
         *fskRxChannel = BAST_FskChannel_e0;
      }
      else if (sb == BAST_FskChannelConfig_eCh0Tx_Ch1Rx)
      {
         *fskTxChannel = BAST_FskChannel_e0;
         *fskRxChannel = BAST_FskChannel_e1;
      }
      else
      {
         *fskTxChannel = BAST_FskChannel_e1;
         *fskRxChannel = BAST_FskChannel_e1;
      }
   }
   return retCode;
}


/******************************************************************************
 BAST_4506_P_SetPeakScanSymbolRateRange()
******************************************************************************/
BERR_Code BAST_4506_P_SetPeakScanSymbolRateRange(
   BAST_ChannelHandle h,             /* [in] BAST channel handle */
   uint32_t           minSymbolRate, /* [in] minimum symbol rate in sym/sec */
   uint32_t           maxSymbolRate  /* [in] maximum symbol rate in sym/sec */
)
{
   BERR_Code retCode;
   uint8_t buf[4];

   if ((minSymbolRate == 0) || (maxSymbolRate == 0))
   {
      minSymbolRate = 0;
      maxSymbolRate = 0;
   }
   else if ((minSymbolRate < 1000000) || (maxSymbolRate < 1000000) ||
       (minSymbolRate > 45000000) || (maxSymbolRate > 45000000) ||
       (minSymbolRate > maxSymbolRate))
   {
      return BERR_TRACE(BERR_INVALID_PARAMETER);
   }

   buf[0] = (uint8_t)((minSymbolRate >> 24) & 0xFF);
   buf[1] = (uint8_t)((minSymbolRate >> 16) & 0xFF);
   buf[2] = (uint8_t)((minSymbolRate >> 8) & 0xFF);
   buf[3] = (uint8_t)(minSymbolRate & 0xFF);
   retCode = BAST_4506_P_WriteConfig(h, BCM4506_CONFIG_PEAK_SCAN_SYM_RATE_MIN, buf, BCM4506_CONFIG_LEN_PEAK_SCAN_SYM_RATE_MIN);
   if (retCode)
      goto done;

   buf[0] = (uint8_t)((maxSymbolRate >> 24) & 0xFF);
   buf[1] = (uint8_t)((maxSymbolRate >> 16) & 0xFF);
   buf[2] = (uint8_t)((maxSymbolRate >> 8) & 0xFF);
   buf[3] = (uint8_t)(maxSymbolRate & 0xFF);
   retCode = BAST_4506_P_WriteConfig(h, BCM4506_CONFIG_PEAK_SCAN_SYM_RATE_MAX, buf, BCM4506_CONFIG_LEN_PEAK_SCAN_SYM_RATE_MAX);

   done:
   if (retCode != BERR_SUCCESS)
   {
      return BERR_TRACE(retCode);
   }
   return retCode;
}


/******************************************************************************
 BAST_4506_P_GetPeakScanSymbolRateRange()
******************************************************************************/
BERR_Code BAST_4506_P_GetPeakScanSymbolRateRange(
   BAST_ChannelHandle h,               /* [in] BAST channel handle */
   uint32_t           *pMinSymbolRate, /* [out] minimum symbol rate in sym/sec */
   uint32_t           *pMaxSymbolRate  /* [out] maximum symbol rate in sym/sec */
)
{
   BERR_Code retCode;
   uint8_t buf[4];

   if ((pMinSymbolRate == NULL) || (pMaxSymbolRate == NULL))
   {
      return BERR_TRACE(BERR_INVALID_PARAMETER);
   }

   retCode = BAST_4506_P_ReadConfig(h, BCM4506_CONFIG_PEAK_SCAN_SYM_RATE_MIN, buf, BCM4506_CONFIG_LEN_PEAK_SCAN_SYM_RATE_MIN);
   if (retCode != BERR_SUCCESS)
      goto done;
   *pMinSymbolRate = (uint32_t)((buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3]);

   retCode = BAST_4506_P_ReadConfig(h, BCM4506_CONFIG_PEAK_SCAN_SYM_RATE_MAX, buf, BCM4506_CONFIG_LEN_PEAK_SCAN_SYM_RATE_MAX);
   if (retCode != BERR_SUCCESS)
      goto done;
   *pMaxSymbolRate = (uint32_t)((buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3]);

   done:
   return retCode;
}


