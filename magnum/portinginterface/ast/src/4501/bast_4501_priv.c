/***************************************************************************
 *     Copyright (c) 2005-2012, Broadcom Corporation
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
#include "bchp_4501.h"
#include "bi2c.h"
#include "bkni_multi.h"
#include "bast.h"
#include "bast_priv.h"
#include "bast_4501_priv.h"


BDBG_MODULE(bast_4501_priv);


/******************************************************************************
 BAST_4501_P_Open()
******************************************************************************/
BERR_Code BAST_4501_P_Open(
   BAST_Handle *h,                   /* [out] BAST handle */
   BCHP_Handle hChip,                /* [in] chip handle */
   void        *pReg,                /* [in] pointer to register or i2c handle */
   BINT_Handle hInterrupt,           /* [in] Interrupt handle */   
   const BAST_Settings *pDefSettings /* [in] default settings */
)
{
   BERR_Code retCode;
   BAST_Handle hDev;
   BAST_4501_P_Handle *h4501Dev;
   uint8_t i;
   
   BSTD_UNUSED(hInterrupt);
   BSTD_UNUSED(hChip);

   BDBG_ASSERT(pDefSettings->i2c.interruptEnableFunc);

   /* allocate memory for the handle */
   hDev = (BAST_Handle)BKNI_Malloc(sizeof(BAST_P_Handle));
   BDBG_ASSERT(hDev);
   h4501Dev = (BAST_4501_P_Handle *)BKNI_Malloc(sizeof(BAST_4501_P_Handle));
   BDBG_ASSERT(h4501Dev);
   hDev->pImpl = (void*)h4501Dev;

   hDev->pChannels = (BAST_P_ChannelHandle **)BKNI_Malloc(BAST_4501_MAX_CHANNELS * sizeof(BAST_P_ChannelHandle *));
   BDBG_ASSERT(hDev->pChannels);

   /* initialize our handle */
   h4501Dev->hRegister = (BREG_I2C_Handle)pReg;
   BKNI_Memcpy((void*)(&(hDev->settings)), (void*)pDefSettings, sizeof(BAST_Settings));
   hDev->totalChannels = BAST_4501_MAX_CHANNELS;
   for (i = 0; i < BAST_4501_MAX_CHANNELS; i++)
      hDev->pChannels[i] = NULL;

   /* create events */
   retCode = BKNI_CreateEvent(&(h4501Dev->hInterruptEvent));
   if (retCode != BERR_SUCCESS)
      goto done_3;
   retCode = BKNI_CreateEvent(&(h4501Dev->hApiEvent));
   if (retCode != BERR_SUCCESS)
      goto done_0;
   retCode = BKNI_CreateEvent(&(h4501Dev->hInitDoneEvent));
   if (retCode != BERR_SUCCESS)
      goto done_1;
   retCode = BKNI_CreateEvent(&(h4501Dev->hHabDoneEvent));
   if (retCode != BERR_SUCCESS)
      goto done_2;
   retCode = BKNI_CreateEvent(&(h4501Dev->hFtmEvent));
   if (retCode != BERR_SUCCESS)
   {
      BKNI_DestroyEvent(h4501Dev->hHabDoneEvent);

      done_2:
      BKNI_DestroyEvent(h4501Dev->hInitDoneEvent);

      done_1:
      BKNI_DestroyEvent(h4501Dev->hApiEvent);

      done_0:
      BKNI_DestroyEvent(h4501Dev->hInterruptEvent);

      done_3:
      BKNI_Free((void*)hDev);
      BKNI_Free((void*)h4501Dev);
      
      *h = NULL;
      return retCode;
   }

   h4501Dev->last_page_16_15 = 0xFF;
   h4501Dev->last_page_14_7 = 0xFF;
   h4501Dev->last_mbox_15_8 = 0xFF;
   h4501Dev->last_bsc_slave_addr = 0xFF;

   *h = hDev;

   return retCode;
}


/******************************************************************************
 BAST_4501_P_Close()
******************************************************************************/
BERR_Code BAST_4501_P_Close(BAST_Handle h)
{
   BERR_Code retCode;
   uint8_t sb;

   BDBG_ASSERT(h);

   retCode = BAST_4501_P_DisableInterrupts(h);
   if (retCode != BERR_SUCCESS)
   {
      BDBG_ERR(("unable to disable BCM4501 interrupts"));
   }

   /* reset the AP */
   sb = BCM4501_AP_RESET;
   retCode = BAST_WriteHostRegister(h, BCM4501_SH_AP_SFR_H_CTL1, &sb);
   if (retCode != BERR_SUCCESS)
   {
      BDBG_ERR(("unable to reset the BCM4501 AP"));
   }

   BKNI_DestroyEvent(((BAST_4501_P_Handle *)(h->pImpl))->hInterruptEvent);
   BKNI_DestroyEvent(((BAST_4501_P_Handle *)(h->pImpl))->hApiEvent);
   BKNI_DestroyEvent(((BAST_4501_P_Handle *)(h->pImpl))->hInitDoneEvent);
   BKNI_DestroyEvent(((BAST_4501_P_Handle *)(h->pImpl))->hHabDoneEvent);
   BKNI_DestroyEvent(((BAST_4501_P_Handle *)(h->pImpl))->hFtmEvent);
   BKNI_Free((void*)h->pChannels);
   BKNI_Free((void*)h->pImpl);
   BKNI_Free((void*)h);

   return retCode;
}


/******************************************************************************
 BAST_4501_P_GetTotalChannels()
******************************************************************************/
BERR_Code BAST_4501_P_GetTotalChannels(
   BAST_Handle  h,             /* [in] BAST handle */
   uint32_t     *totalChannels /* [out] number of channels supported */
)
{
   BSTD_UNUSED(h);
   *totalChannels = BAST_4501_MAX_CHANNELS;
   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_4501_P_OpenChannel()
******************************************************************************/
BERR_Code BAST_4501_P_OpenChannel(
   BAST_Handle                 h,              /* [in] BAST handle */
   BAST_ChannelHandle         *pChannelHandle, /* [out] BAST channel handle */
   uint32_t                   chnNo,           /* [in] channel number */
   const BAST_ChannelSettings *pSettings       /* [in] channel settings */
)
{
   BERR_Code retCode;
   BAST_ChannelSettings cs;
   BAST_P_ChannelHandle *ch;
   BAST_4501_P_ChannelHandle *ch4501;

   BDBG_ASSERT(chnNo < BAST_4501_MAX_CHANNELS);

   if (pSettings == NULL)
      BAST_4501_GetChannelDefaultSettings(h, chnNo, &cs);
   else
      cs = *pSettings;

   /* allocate memory for the channel handle */
   ch = (BAST_P_ChannelHandle *)BKNI_Malloc(sizeof(BAST_P_ChannelHandle));
   BDBG_ASSERT(ch);
   BKNI_Memcpy((void*)(&ch->settings), (void*)&cs, sizeof(BAST_ChannelSettings)); 
   ch->channel = (uint8_t)chnNo;
   ch->pDevice = h;
   h->pChannels[chnNo] = ch;

   ch4501 = (BAST_4501_P_ChannelHandle *)BKNI_Malloc(sizeof(BAST_4501_P_ChannelHandle));
   BDBG_ASSERT(ch4501);

   retCode = BKNI_CreateEvent(&(ch4501->hLockChangeEvent));
   if (retCode != BERR_SUCCESS)
      goto done_0;
   retCode = BKNI_CreateEvent(&(ch4501->hDiseqcEvent));
   if (retCode != BERR_SUCCESS)
      goto done_1;
   retCode = BKNI_CreateEvent(&(ch4501->hTunerEvent));
   if (retCode != BERR_SUCCESS)
   {
      BKNI_DestroyEvent(ch4501->hDiseqcEvent);

      done_1:
      BKNI_DestroyEvent(ch4501->hLockChangeEvent);

      done_0:
      BKNI_Free((void*)ch4501);
      BKNI_Free((void*)ch);

      *pChannelHandle = NULL;
      return retCode;
   }

   ch->pImpl = (void*)ch4501;
   *pChannelHandle = ch;

   return retCode;
}


/******************************************************************************
 BAST_4501_P_CloseChannel()
******************************************************************************/
BERR_Code BAST_4501_P_CloseChannel(
   BAST_ChannelHandle h  /* [in] BAST channel handle */
)
{
   BAST_4501_P_ChannelHandle *ch4501;

   ch4501 = (BAST_4501_P_ChannelHandle *)(h->pImpl);
   BKNI_DestroyEvent(ch4501->hLockChangeEvent);
   BKNI_DestroyEvent(ch4501->hDiseqcEvent);
   BKNI_DestroyEvent(ch4501->hTunerEvent);
   BKNI_Free((void*)ch4501);
   BKNI_Free((void*)h);

   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_4501_P_GetDevice()
******************************************************************************/
BERR_Code BAST_4501_P_GetDevice(
   BAST_ChannelHandle h,  /* [in] BAST channel handle */
   BAST_Handle *pDev      /* [out] BAST handle */
)
{
   *pDev = h->pDevice;
   return BERR_SUCCESS; 
}


/******************************************************************************
 BAST_4501_P_InitAp()
******************************************************************************/
BERR_Code BAST_4501_P_InitAp(
   BAST_Handle h,            /* [in] BAST handle */
   const uint8_t *pHexImage  /* [in] pointer to BCM4501 microcode image */
)
{
#ifndef BAST_DONT_USE_INTERRUPT
   BAST_4501_P_Handle *p4501 = (BAST_4501_P_Handle *)(h->pImpl);
#endif
   BERR_Code retCode = BERR_SUCCESS;
   uint16_t n, addr;
   const uint8_t *pImage;
   uint8_t sb, retries;
#ifdef BAST_VERIFY_DOWNLOAD
   uint8_t pVerifyBuf[0xA000];
#endif

#if 0
   for (n = 0; n < 10; n++)
   {
      sb = 0;
      retCode = BAST_WriteHostRegister(h, BCM4501_SH_AP_SFR_H_IE1, &sb);
      if (retCode == BERR_SUCCESS)
         break;
   }
   if (retCode != BERR_SUCCESS)
      return retCode;
#endif

   /* disable interrupts */
   BAST_CHK_RETCODE(BAST_4501_P_DisableInterrupts(h));
   BAST_CHK_RETCODE(BAST_4501_P_EnableHostInterrupt(h, false));

   /* reset the AP before downloading the microcode */
   BAST_CHK_RETCODE(BAST_4501_P_ResetAp(h));

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

      for (retries = 0; retries < 3; retries++)
      {
         /* BDBG_MSG(("writing %d bytes from 0x%04X", n, addr)); */
         if (BAST_4501_P_WriteMemory(h, addr, pImage, n) == BERR_SUCCESS) 
         {
#ifdef BAST_VERIFY_DOWNLOAD
            BDBG_MSG(("verifying %d bytes from 0x%04X", n, addr));
            BAST_CHK_RETCODE(BAST_4501_P_ReadMemory(h, addr, (uint8_t *)pVerifyBuf, n));
            if (BKNI_Memcmp(pImage, pVerifyBuf, n) == 0)
               break; 
            BDBG_WRN(("data read back does not match\n"));
#else
            /* check for host transfer error */
            BAST_CHK_RETCODE(BAST_ReadHostRegister(h, BCM4501_SH_AP_SFR_H_STAT1, &sb));
            if ((sb & BCM4501_STAT1_H_ER) == 0)
               break;
            
            BDBG_WRN(("host transfer error\n"));
            BAST_CHK_RETCODE(BAST_WriteHostRegister(h, BCM4501_SH_AP_SFR_H_STAT1, &sb));
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
   
#ifndef BAST_DONT_USE_INTERRUPT
   /* enable init done interrupt */
   sb = BCM4501_STAT2_INIT_DONE;
   BAST_CHK_RETCODE(BAST_WriteHostRegister(h, BCM4501_SH_AP_SFR_H_IE2, &sb));
   BKNI_WaitForEvent(p4501->hInitDoneEvent, 0);   
#endif

   /* start running the AP */
   if ((retCode = BAST_4501_P_RunAp(h)) != BERR_SUCCESS)
      goto done;

#ifdef BAST_DONT_USE_INTERRUPT
   /* poll for AP init done */
   for (n = 0; n < 2222; n++)
   {
      BAST_CHK_RETCODE(BAST_ReadHostRegister(h, BCM4501_SH_AP_SFR_H_STAT2, &sb));
      if (sb & BCM4501_STAT2_INIT_DONE)
         break;
   }
   if ((sb & BCM4501_STAT2_INIT_DONE) == 0)
   {
      BDBG_ERR(("AP initialization timeout\n")); 
      BERR_TRACE(retCode = BAST_ERR_AP_NOT_INIT);            
   }
#else
   /* wait for init done interrupt */
   if (BAST_4501_P_WaitForEvent(h, p4501->hInitDoneEvent, 200) != BERR_SUCCESS)
   {
      BDBG_ERR(("AP initialization timeout\n")); 
      BERR_TRACE(retCode = BAST_ERR_AP_NOT_INIT);            
   }
#endif

   if (retCode == BERR_SUCCESS)
   {
      sb = (uint8_t)(h->settings.networkSpec);
      retCode = BAST_WriteConfig(h->pChannels[0], BCM4501_CONFIG_NETWORK_SPEC, &sb, BCM4501_CONFIG_LEN_NETWORK_SPEC);
      if (retCode != BERR_SUCCESS)
      {
         BDBG_ERR(("unable to set NETWORK_SPEC config parameter\n")); 
      }
   }

   done:
   return retCode;
}


/******************************************************************************
 BAST_4501_P_SoftReset()
******************************************************************************/
BERR_Code BAST_4501_P_SoftReset(
   BAST_Handle h           /* [in] BAST device handle */
)
{
   BERR_Code retCode;
   uint8_t sb;

   sb = BCM4501_STAT2_INIT_DONE;
   BAST_CHK_RETCODE(BAST_WriteHostRegister(h, BCM4501_SH_AP_SFR_H_STAT2, &sb));
   sb = BCM4501_AP_SOFT_RESET;
   BAST_CHK_RETCODE(BAST_WriteHostRegister(h, BCM4501_SH_AP_SFR_H_CTL1, &sb));

   done:
   return retCode;
}


/******************************************************************************
 BAST_4501_P_GetApStatus()
******************************************************************************/
BERR_Code BAST_4501_P_GetApStatus(
   BAST_Handle h,           /* [in] BAST device handle */
   BAST_ApStatus *pStatus   /* [out] AP status */
)
{
   BERR_Code retCode;
   uint8_t buf[4];

   *pStatus = 0;
   BAST_CHK_RETCODE(BREG_I2C_Read(((BAST_4501_P_Handle *)(h->pImpl))->hRegister, h->settings.i2c.chipAddr, BCM4501_SH_AP_SFR_H_CTL1, buf, 4));

   *pStatus = buf[0] | (buf[1] << 8) | (buf[2] << 16) | (buf[3] << 24);
   
   done:
   return retCode;
}


/******************************************************************************
 BAST_4501_P_GetApVersion()
******************************************************************************/
BERR_Code BAST_4501_P_GetApVersion(
   BAST_Handle h,          /* [in] BAST handle */
   uint16_t    *pChipId,   /* [out] BAST chip ID */
   uint8_t     *pChipVer,  /* [out] chip revision number */
   uint32_t    *pBondId,   /* [out] chip bonding option */
   uint8_t     *pApVer,    /* [out] AP microcode version */
   uint8_t     *pCfgVer    /* [out] host configuration version */
)
{
   BERR_Code retCode;
   uint8_t buf[8];

   buf[0] = 0x0C;
   BAST_CHK_RETCODE(BAST_4501_P_SendHabCommand(h, buf, 8, buf, true));

   *pChipId = ((buf[1] << 8) | buf[2]);
   *pChipVer = buf[3];
   *pBondId = (uint32_t)buf[4];
   *pApVer = buf[5];
   *pCfgVer = buf[6];

   done:
   return retCode;
}


/******************************************************************************
 BAST_4501_P_SetTmConfig()
******************************************************************************/
BERR_Code BAST_4501_P_SetTmConfig(
   BAST_Handle h,    /* [in] BAST handle */
   void        *p    /* [in] TM configuration parameters */
)
{
   BERR_Code retCode;
   BAST_4501_TmSettings *pTmSettings = (BAST_4501_TmSettings *)p;

   BAST_CHK_RETCODE(BAST_4501_P_WriteRegister(h->pChannels[0], BCM4501_TM_PAD_CTRL, &(pTmSettings->tm_pad_ctrl)));
   BAST_CHK_RETCODE(BAST_4501_P_WriteRegister(h->pChannels[0], BCM4501_TM_SDS_XPT_PAD_CTRL, &(pTmSettings->tm_sds_xpt_pad_ctrl)));
   BAST_CHK_RETCODE(BAST_4501_P_WriteRegister(h->pChannels[0], BCM4501_TM_VREG_CTRL, &(pTmSettings->tm_vreg_ctrl)));

   done:
   return retCode;
}


/******************************************************************************
 BAST_4501_P_GetTmConfig()
******************************************************************************/
BERR_Code BAST_4501_P_GetTmConfig(
   BAST_Handle h,  /* [in] BAST handle */
   void        *p  /* [out] pad configuration parameters */
)
{
   uint32_t tm_pad_ctrl = 0, tm_sds_xpt_pad_ctrl = 0, tm_vreg_ctrl = 0;
   BERR_Code retCode;
   BAST_4501_TmSettings *pTmSettings = (BAST_4501_TmSettings *)p;

   BAST_CHK_RETCODE(BAST_4501_P_ReadRegister(h->pChannels[0], BCM4501_TM_PAD_CTRL, &tm_pad_ctrl));
   BAST_CHK_RETCODE(BAST_4501_P_ReadRegister(h->pChannels[0], BCM4501_TM_SDS_XPT_PAD_CTRL, &tm_sds_xpt_pad_ctrl));
   BAST_CHK_RETCODE(BAST_4501_P_ReadRegister(h->pChannels[0], BCM4501_TM_VREG_CTRL, &tm_vreg_ctrl));

   pTmSettings->tm_pad_ctrl = tm_pad_ctrl;
   pTmSettings->tm_sds_xpt_pad_ctrl = tm_sds_xpt_pad_ctrl;
   pTmSettings->tm_vreg_ctrl = tm_vreg_ctrl;

   done:
   return retCode;
}


/******************************************************************************
 BERR_Code BAST_4501_P_ConfigGpio(
******************************************************************************/
BERR_Code BAST_4501_P_ConfigGpio(
   BAST_Handle h,           /* [in] BAST handle */
   uint32_t    write_mask,  /* [in] selects which GPIO pins are output */
   uint32_t    read_mask    /* [in] selects which GPIO pins are input */
)
{
   BERR_Code retCode;

   uint32_t own_mask = (write_mask | read_mask) & 0x0000FFFF;
   uint8_t sb;

   sb = (uint8_t)((own_mask >> 8) & 0xFF);
   BAST_CHK_RETCODE(BAST_WriteHostRegister(h, BCM4501_SH_AP_SFR_GPIO_OW_1, &sb));
   sb = (uint8_t)(own_mask & 0xFF);
   BAST_CHK_RETCODE(BAST_WriteHostRegister(h, BCM4501_SH_AP_SFR_GPIO_OW_0, &sb));
   sb = (uint8_t)((read_mask >> 8) & 0xFF); 
   BAST_CHK_RETCODE(BAST_WriteHostRegister(h, BCM4501_SH_AP_SFR_GPIOENB_1, &sb));
   sb = (uint8_t)(read_mask & 0xFF); 
   BAST_CHK_RETCODE(BAST_WriteHostRegister(h, BCM4501_SH_AP_SFR_GPIOENB_0, &sb));
   
   done:
   return retCode;
}


/******************************************************************************
 BERR_Code BAST_4501_P_SetGpio(
******************************************************************************/
BERR_Code BAST_4501_P_SetGpio(
   BAST_Handle h,     /* [in] BAST handle */
   uint32_t    mask,  /* [in] selects which GPIO pins to set */
   uint32_t    state  /* [in] state of GPIO pins */
)
{
   BERR_Code retCode = BERR_SUCCESS;
   uint32_t x;
   uint8_t hi, lo, sb;

   mask &= 0x0000FFFF;
   
   BAST_CHK_RETCODE(BAST_ReadHostRegister(h, BCM4501_SH_AP_SFR_GPOUT_1, &hi));
   BAST_CHK_RETCODE(BAST_ReadHostRegister(h, BCM4501_SH_AP_SFR_GPOUT_0, &lo));
   x = (hi << 8) | lo;
   x &= ~mask;
   x |= (mask & state);

   sb = (uint8_t)(x >> 8);
   BAST_CHK_RETCODE(BAST_WriteHostRegister(h, BCM4501_SH_AP_SFR_GPOUT_1, &sb));
   sb = (uint8_t)(x & 0xFF);
   BAST_CHK_RETCODE(BAST_WriteHostRegister(h, BCM4501_SH_AP_SFR_GPOUT_0, &sb));

   done:
   return retCode;
}


/******************************************************************************
 BERR_Code BAST_4501_P_GetGpio(
******************************************************************************/
BERR_Code BAST_4501_P_GetGpio(
   BAST_Handle h,     /* [in] BAST handle */
   uint32_t    mask,  /* [in] selects which GPIO pins to read */
   uint32_t    *state  /* [out] state of GPIO pins */
)
{
   BERR_Code retCode = BERR_SUCCESS;
   uint8_t hi, lo;

   BAST_CHK_RETCODE(BAST_ReadHostRegister(h, BCM4501_SH_AP_SFR_GPIN_1, &hi));
   BAST_CHK_RETCODE(BAST_ReadHostRegister(h, BCM4501_SH_AP_SFR_GPIN_0, &lo));
   mask &= 0x0000FFFF;
   *state = (hi << 8) | lo;
   *state &= mask;

   done:
   return retCode;
}


/******************************************************************************
 BAST_4501_P_TuneAcquire()
******************************************************************************/
BERR_Code BAST_4501_P_TuneAcquire(
   BAST_ChannelHandle h,            /* [in] BAST channel handle */
   const uint32_t     freq,         /* [in] RF tuner frequency in Hz */
   const BAST_AcqSettings *pParams  /* [in] acquisition parameters */
)
{
   BERR_Code retCode;
   uint8_t buf[16];

   if ((freq < 950000000UL) || (freq > 2150000000UL))
      return (BERR_TRACE(BERR_INVALID_PARAMETER));
   if ((pParams->symbolRate < 1000000) || (pParams->symbolRate > 45000000))
      return (BERR_TRACE(BERR_INVALID_PARAMETER));
   if ((pParams->mode > BAST_Mode_eLdpc_scan) || 
       ((pParams->mode >= BAST_Mode_eDcii_scan) && (pParams->mode < BAST_Mode_eLdpc_Qpsk_1_2)) ||
       (pParams->mode == BAST_Mode_eUnknown))
      return (BERR_TRACE(BERR_INVALID_PARAMETER));

   BAST_CHK_RETCODE(BAST_4501_P_EnableLockInterrupt(h, false));

   /* program symbol rate */
   buf[0] = (uint8_t)(pParams->symbolRate >> 24);
   buf[1] = (uint8_t)(pParams->symbolRate >> 16);
   buf[2] = (uint8_t)(pParams->symbolRate >> 8);
   buf[3] = (uint8_t)(pParams->symbolRate & 0xFF);
   BAST_CHK_RETCODE(BAST_4501_P_WriteConfig(h, BCM4501_CONFIG_SYMBOL_RATE, buf, BCM4501_CONFIG_LEN_SYMBOL_RATE));

   buf[0] = (uint8_t)(0x24 | (h->channel ? 0x40 : 0x00));
   buf[1] = (uint8_t)((freq >> 24) & 0xFF); 
   buf[2] = (uint8_t)((freq >> 16) & 0xFF); 
   buf[3] = (uint8_t)((freq >> 8) & 0xFF); 
   buf[4] = (uint8_t)(freq & 0xFF); 
   buf[5] = (uint8_t)(pParams->mode);
   buf[6] = (uint8_t)((pParams->carrierFreqOffset >> 24) & 0xFF); 
   buf[7] = (uint8_t)((pParams->carrierFreqOffset >> 16) & 0xFF); 
   buf[8] = (uint8_t)((pParams->carrierFreqOffset >> 8) & 0xFF); 
   buf[9] = (uint8_t)(pParams->carrierFreqOffset & 0xFF); 
   buf[10] = (uint8_t)((pParams->acq_ctl >> 24) & 0xFF);
   buf[11] = (uint8_t)((pParams->acq_ctl >> 16) & 0xFF);
   buf[12] = (uint8_t)((pParams->acq_ctl >> 8) & 0xFF);
   buf[13] = (uint8_t)(pParams->acq_ctl & 0xFF);
   BAST_CHK_RETCODE(BAST_4501_P_SendHabCommand(h->pDevice, buf, 15, buf, true));

#ifndef BAST_DISABLE_LOCKINT
   BAST_CHK_RETCODE(BAST_4501_P_EnableLockInterrupt(h, true));
#endif

   done:
   return retCode;
}


/******************************************************************************
 BAST_4501_P_GetChannelStatus()
******************************************************************************/
BERR_Code BAST_4501_P_GetChannelStatus(
   BAST_ChannelHandle h,        /* [in] BAST handle */
   BAST_ChannelStatus *pStatus  /* [out] pointer to status structure */
)
{
   BERR_Code retCode;
   uint8_t buf[128];
   BAST_Mode currentMode;
   
   send_hab_status:
   currentMode = ((BAST_4501_P_ChannelHandle *)(h->pImpl))->currentMode;
   if (BAST_MODE_IS_LDPC(currentMode))
   {
      /* send LDPC_STATUS HAB command */
      buf[0] = 0x13 | (h->channel ? 0x40 : 0x00);
      BAST_CHK_RETCODE(BAST_4501_P_SendHabCommand(h->pDevice, buf, 86, buf, true));
   }
   else
   {
      /* send LEGACY_QPSK HAB command */
      buf[0] = 0x12 | (h->channel ? 0x40 : 0x00);
      BAST_CHK_RETCODE(BAST_4501_P_SendHabCommand(h->pDevice, buf, 75, buf, true));
   }

   if (((BAST_MODE_IS_LDPC(buf[2])) && !(BAST_MODE_IS_LDPC(currentMode))) ||
       (!(BAST_MODE_IS_LDPC(buf[2])) && (BAST_MODE_IS_LDPC(currentMode))))
   {
      /* this should not happen */
      ((BAST_4501_P_ChannelHandle *)(h->pImpl))->currentMode = (BAST_Mode)buf[2];
      goto send_hab_status;
   }

   pStatus->mode = (BAST_Mode)(buf[2]);
   pStatus->bBertLocked = buf[1] & 0x80 ? true : false;
   pStatus->bDemodLocked = (buf[1] & 0x40) ? true : false;
   pStatus->bTunerLocked = (buf[1] & 0x04) ? true : false;
   pStatus->symbolRate = (buf[3] << 24) | (buf[4] << 16) | (buf[5] << 8) | buf[6];
   pStatus->symbolRateError = (int32_t)((buf[20] << 24) | (buf[21] << 16) | (buf[22] << 8) | buf[23]);
   pStatus->carrierOffset = (int32_t)((buf[7] << 24) | (buf[8] << 16) | (buf[9] << 8) | buf[10]);
   pStatus->carrierError = (int32_t)((buf[24] << 24) | (buf[25] << 16) | (buf[26] << 8) | buf[27]);
   pStatus->snrEstimate = (buf[28] << 24) | (buf[29] << 16) | (buf[30] << 8) | buf[31];
   pStatus->IFagc = (buf[32] << 24) | (buf[33] << 16) | (buf[34] << 8) | buf[35];
   pStatus->RFagc = (buf[36] << 24) | (buf[37] << 16) | (buf[38] << 8) | buf[39];
   pStatus->outputBitrate = (buf[16] << 24) | (buf[17] << 16) | (buf[18] << 8) | buf[19];
   pStatus->berErrors = (buf[40] << 24) | (buf[41] << 16) | (buf[42] << 8) | buf[43];
   pStatus->mpegErrors = (buf[44] << 24) | (buf[45] << 16) | (buf[46] << 8) | buf[47];
   pStatus->mpegCount = (buf[48] << 24) | (buf[49] << 16) | (buf[50] << 8) | buf[51];
   pStatus->reacqCount = (buf[52] << 24) | (buf[53] << 16) | (buf[54] << 8) | buf[55];
   pStatus->tunerFreq = (buf[11] << 24) | (buf[12] << 16) | (buf[13] << 8) | buf[14];
   pStatus->agf = (buf[57] << 24) | (buf[58] << 16) | (buf[59] << 8) | buf[60];
   pStatus->sample_clock = (buf[61] << 24) | (buf[62] << 16) | (buf[63] << 8) | buf[64];
   pStatus->tunerFilter = (uint32_t)buf[15] * 1000000;

   if (BAST_MODE_IS_LDPC(currentMode))
   {
      pStatus->modeStatus.ldpc.totalBlocks = (buf[65] << 24) | (buf[66] << 16) | (buf[67] << 8) | buf[68];
      pStatus->modeStatus.ldpc.corrBlocks = (buf[69] << 24) | (buf[70] << 16) | (buf[71] << 8) | buf[72];
      pStatus->modeStatus.ldpc.badBlocks = (buf[73] << 24) | (buf[74] << 16) | (buf[75] << 8) | buf[76];
      pStatus->modeStatus.ldpc.hpstatus = (buf[77] << 24) | (buf[78] << 16) | (buf[79] << 8) | buf[80];
      pStatus->modeStatus.ldpc.fec_clock = (buf[81] << 24) | (buf[82] << 16) | (buf[83] << 8) | buf[84];
   }
   else
   {
      pStatus->modeStatus.legacy.rsCorrCount = (buf[65] << 24) | (buf[66] << 16) | (buf[67] << 8) | buf[68];
      pStatus->modeStatus.legacy.rsUncorrCount = (buf[69] << 24) | (buf[70] << 16) | (buf[71] << 8) | buf[72];
      pStatus->modeStatus.legacy.spinv = (BAST_Spinv)((buf[73] >> 4) & 0x03);
      pStatus->modeStatus.legacy.phase = (BAST_PhaseRotation)((buf[73] >> 1) & 0x03);
      pStatus->modeStatus.legacy.preVitErrCount = 0xFFFFFFFF; /* not supported in 4501 */
   }

   done:
   return retCode;
}


/******************************************************************************
 BAST_4501_P_GetLockStatus()
******************************************************************************/
BERR_Code BAST_4501_P_GetLockStatus(
   BAST_ChannelHandle h,  /* [in] BAST handle */
   bool *bLocked          /* [out] pointer to status structure */
)
{
   BERR_Code retCode;

#if 1
   BAST_ApStatus status, mask;

   BAST_CHK_RETCODE(BAST_4501_P_GetApStatus(h->pDevice, &status));

   mask = h->channel ? BAST_APSTATUS_LOCK_1 : BAST_APSTATUS_LOCK_0;
   *bLocked = (mask & status) ? true : false;
#else
   BAST_ChannelStatus status;
   BAST_CHK_RETCODE(BAST_4501_P_GetChannelStatus(h, &status));
   *bLocked = status.bDemodLocked;
#endif

   done:
   return retCode;
}

/******************************************************************************
 BAST_4501_P_ResetStatus()
******************************************************************************/
BERR_Code BAST_4501_P_ResetStatus(
   BAST_ChannelHandle h  /* [in] BAST channel handle */
)
{
   BERR_Code retCode;
   uint8_t buf[2];
   
   buf[0] = 0x1B | (h->channel ? 0x40 : 0x00);
   BAST_CHK_RETCODE(BAST_4501_P_SendHabCommand(h->pDevice, buf, 2, buf, true));

   done:
   return retCode;
}


/******************************************************************************
 BAST_4501_P_SetDiseqcTone()
******************************************************************************/
BERR_Code BAST_4501_P_SetDiseqcTone(
   BAST_ChannelHandle h,  /* [in] BAST channel handle */
   bool bTone             /* [in] true = tone on, false = tone off */
)
{
   BERR_Code retCode;
   uint8_t buf[3];
   
   buf[0] = 0x17 | (h->channel ? 0x40 : 0x00);
   buf[1] = bTone ? 0x22 : 0x20;
   BAST_CHK_RETCODE(BAST_4501_P_SendHabCommand(h->pDevice, buf, 3, buf, true));

   done:
   return retCode;
}


/******************************************************************************
 BAST_4501_P_GetDiseqcTone()
******************************************************************************/
BERR_Code BAST_4501_P_GetDiseqcTone(
   BAST_ChannelHandle h,  /* [in] BAST channel handle */
   bool *bTone            /* [out] true = tone present, false = tone absent */
)
{
   BERR_Code retCode;
   uint8_t buf[3];
   
   buf[0] = 0x1A | (h->channel ? 0x40 : 0x00);
   BAST_CHK_RETCODE(BAST_4501_P_SendHabCommand(h->pDevice, buf, 3, buf, true));

   *bTone = (buf[1] ? true : false);

   done:
   return retCode;
}


/******************************************************************************
 BAST_4501_P_SetDiseqcVoltage()
******************************************************************************/
BERR_Code BAST_4501_P_SetDiseqcVoltage(
   BAST_ChannelHandle h, /* [in] BAST channel handle */
   bool bVtop            /* [in] true = VTOP, false = VBOT */
)
{
   BERR_Code retCode;
   uint8_t buf[3];
   
   buf[0] = 0x17 | (h->channel ? 0x40 : 0x00);
   buf[1] = bVtop ? 0x11 : 0x10;
   BAST_CHK_RETCODE(BAST_4501_P_SendHabCommand(h->pDevice, buf, 3, buf, true));

   done:
   return retCode;
}


/******************************************************************************
 BAST_4501_P_SendDiseqcCommand()
******************************************************************************/
BERR_Code BAST_4501_P_SendDiseqcCommand(
   BAST_ChannelHandle h,    /* [in] BAST channel handle */
   const uint8_t *pSendBuf, /* [in] contains data to send */
   uint8_t sendBufLen       /* [in] number of bytes to send */
)
{
   BERR_Code retCode;
   uint8_t ie;
   uint8_t buf[32], i, mask;
   
   BDBG_ASSERT(sendBufLen <= 16);

   /* set up DISEQC_SEND HAB command */
   buf[0] = 0x18 | (h->channel ? 0x40 : 0x00);
   buf[1] = sendBufLen;
   for (i = 0; i < sendBufLen; i++)
      buf[2+i] = pSendBuf[i];

   /* set up diseqc irq */
   mask = h->channel ? BCM4501_STAT3_CH1_DS_DONE : BCM4501_STAT3_CH0_DS_DONE;
   BAST_CHK_RETCODE(BAST_WriteHostRegister(h->pDevice, BCM4501_SH_AP_SFR_H_FSTAT3, &mask));
   BAST_CHK_RETCODE(BAST_ReadHostRegister(h->pDevice, BCM4501_SH_AP_SFR_H_IE3, &ie));
   ie |= mask;
   BAST_CHK_RETCODE(BAST_WriteHostRegister(h->pDevice, BCM4501_SH_AP_SFR_H_IE3, &ie));

   /* send DISEQC_SEND HAB command */
   BAST_CHK_RETCODE(BAST_4501_P_SendHabCommand(h->pDevice, buf, (uint8_t)(3+sendBufLen), buf, true));

   done:
   return retCode;
}


/******************************************************************************
 BAST_4501_P_GetDiseqcStatus()
******************************************************************************/
BERR_Code BAST_4501_P_GetDiseqcStatus(
   BAST_ChannelHandle h,       /* [in] BAST channel handle */
   BAST_DiseqcStatus  *pStatus /* [out] status of last transaction */
)
{
   BERR_Code retCode;
   uint8_t buf[32], i;

   /* send DISEQC_READ_RECEIVE_BUFFER */
   buf[0] = 0x19 | (h->channel ? 0x40 : 0x00);
   BAST_CHK_RETCODE(BAST_4501_P_SendHabCommand(h->pDevice, buf, 20, buf, true));

   switch (buf[1] & 0x07)
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

      default:
         pStatus->status = BAST_DiseqcSendStatus_eSuccess;
         break;
   }
   
   pStatus->bReplyExpected = (buf[1] & 0x08) ? true : false;
   pStatus->nReplyBytes = buf[2];
   if (pStatus->nReplyBytes > 16)
      pStatus->nReplyBytes = 16;
   for (i = 0; i < pStatus->nReplyBytes; i++)
      pStatus->replyBuffer[i] = buf[3+i];

   done:
   return retCode;
}


/******************************************************************************
 BAST_ResetDiseqc()
******************************************************************************/ 
BERR_Code BAST_4501_P_ResetDiseqc(
   BAST_ChannelHandle h,    /* [in] BAST channel handle */
   uint8_t options          /* [in] reset options */
)
{
   BERR_Code retCode;
   uint8_t buf[4];

   buf[0] = 0x16 | (h->channel ? 0x40 : 0x00);
   buf[1] = options;
   BAST_CHK_RETCODE(BAST_4501_P_SendHabCommand(h->pDevice, buf, 3, buf, true));

#if 0 /* comment out for dual diseqc/ftm support */
   /* turn off ftm interrupts */
   BAST_CHK_RETCODE(BAST_4501_P_EnableFtmInterrupt(h->pDevice, false));
#endif

   done:
   return retCode;
}


/******************************************************************************
 BAST_ResetFtm()
******************************************************************************/ 
BERR_Code BAST_4501_P_ResetFtm(
   BAST_Handle h    /* [in] BAST channel handle */
)
{
   BERR_Code retCode;
   uint8_t buf[2];

   /* turn on ftm interrupts */
   BAST_CHK_RETCODE(BAST_4501_P_EnableFtmInterrupt(h, true));

   buf[0] = 0x23;
   BAST_CHK_RETCODE(BAST_4501_P_SendHabCommand(h, buf, 2, buf, true));

   done:
   return retCode;
}


/******************************************************************************
 BERR_Code BAST_4501_P_ReadFtm()
******************************************************************************/
BERR_Code BAST_4501_P_ReadFtm(
   BAST_Handle h,  /* [in] BAST handle */
   uint8_t *pBuf,  /* [out] data read */
   uint8_t *n      /* [out] length of data read */
)
{
   BERR_Code retCode;
   uint8_t buf[128], i, len;

   *n = 0;

   buf[0] = 0x20;
   buf[1] = 0x20;
   BAST_CHK_RETCODE(BAST_4501_P_SendHabCommand(h, buf, 2, buf, false));

   len = buf[1] & 0x7F;
   if (len)
   {
      /* read the next len bytes in the HAB */
      BAST_CHK_RETCODE(BAST_4501_P_ReadHab(h, 2, &buf[2], len));

      *n = len + 1;
      for (i = 0; i < *n; i++)
         pBuf[i] = buf[i+1]; 
   }

   done:
   return retCode;
}


/******************************************************************************
 BERR_Code BAST_4501_P_WriteFtm()
******************************************************************************/
BERR_Code BAST_4501_P_WriteFtm(
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
   BAST_CHK_RETCODE(BAST_4501_P_SendHabCommand(h, buf, (uint8_t)(2+n), buf, true));

   done:
   return retCode;
}


/******************************************************************************
 BAST_4501_P_PowerDownFtm()
******************************************************************************/
BERR_Code BAST_4501_P_PowerDownFtm(
   BAST_Handle h  /* [in] BAST handle */
)
{
   uint8_t buf[4];

   buf[0] = 0x22;
   buf[1] = 0x04;
  
   return BAST_4501_P_SendHabCommand(h, buf, 3, buf, true);
}


/******************************************************************************
 BAST_4501_P_PowerUpFtm()
******************************************************************************/
BERR_Code BAST_4501_P_PowerUpFtm(
   BAST_Handle h  /* [in] BAST handle */
)
{
   uint8_t buf[4];

   buf[0] = 0x25;
   buf[1] = 0x04;
  
   return BAST_4501_P_SendHabCommand(h, buf, 3, buf, true);
}


/******************************************************************************
 BAST_4501_P_WriteMi2c()
******************************************************************************/ 
BERR_Code BAST_4501_P_WriteMi2c(
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

   BAST_CHK_RETCODE(BAST_4501_P_SendHabCommand(h->pDevice, buf, (uint8_t)(4+n), buf, true));

   if ((buf[1] & 0x80) == 0)
      retCode = BAST_ERR_MI2C_NO_ACK;

   done:
   return retCode;
}


/******************************************************************************
 BAST_4501_P_ReadMi2c()
******************************************************************************/
BERR_Code BAST_4501_P_ReadMi2c(
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

   BAST_CHK_RETCODE(BAST_4501_P_SendHabCommand(h->pDevice, buf, (uint8_t)(4+out_n+in_n), buf, true));

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
 BAST_4501_P_GetSoftDecisionBuf()
******************************************************************************/
BERR_Code BAST_4501_P_GetSoftDecisionBuf(
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
      BAST_CHK_RETCODE(BAST_4501_P_ReadRegister(h, BCM4501_SDS_EQSFT, &val32));
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
   addr_hi = h->channel ? 0x25 : 0x15;
   addr_lo = 0x50;
   cs = h->channel ? 0x78 : 0x68;

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
   BAST_CHK_RETCODE(BAST_4501_P_SendHabCommand(h->pDevice, buf, 120, buf, false));
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
 BERR_Code BAST_4501_P_ReadAgc()
******************************************************************************/
BERR_Code BAST_4501_P_ReadAgc(
   BAST_ChannelHandle h,  /* [in] BAST channel handle */
   BAST_Agc which_agc,    /* [in] AGC select */
   uint32_t *pAgc         /* [out] integrator value */
)
{
   uint8_t buf[8];
   BERR_Code retCode;

   buf[0] = 0x07;
   buf[1] = (h->channel == 0 ? 0x14 : 0x24);
   buf[2] = (which_agc == BAST_Agc_eIF ? 0x88 : 0x90);

   BAST_CHK_RETCODE(BAST_4501_P_SendHabCommand(h->pDevice, buf, 8, buf, true));

   *pAgc = (buf[3] << 24) | (buf[4] << 16) | (buf[5] << 8) | buf[6];

   done:
   return retCode;
}


/******************************************************************************
 BERR_Code BAST_4501_P_WriteAgc()
******************************************************************************/
BERR_Code BAST_4501_P_WriteAgc(
   BAST_ChannelHandle h,  /* [in] BAST channel handle */
   BAST_Agc which_agc,    /* [in] AGC select */
   uint32_t *pAgc         /* [in] integrator value */
)
{
   uint8_t buf[8];

   buf[0] = 0x08;
   buf[1] = (h->channel == 0 ? 0x14 : 0x24);
   buf[2] = (which_agc == BAST_Agc_eIF ? 0x88 : 0x90);
   buf[3] = (uint8_t)(*pAgc >> 24);
   buf[4] = (uint8_t)(*pAgc >> 16);
   buf[5] = (uint8_t)(*pAgc >> 8);
   buf[6] = (uint8_t)(*pAgc & 0xFF);

   return BAST_4501_P_SendHabCommand(h->pDevice, buf, 8, buf, true);
}


/******************************************************************************
 BERR_Code BAST_4501_P_HabIsbEdit()
******************************************************************************/
BERR_Code BAST_4501_P_HabIsbEdit(
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
  
   return BAST_4501_P_SendHabCommand(h->pDevice, buf, 12, buf, true);
}


/******************************************************************************
 BERR_Code BAST_4501_P_FreezeAgc()
******************************************************************************/
BERR_Code BAST_4501_P_FreezeAgc(
   BAST_ChannelHandle h,  /* [in] BAST channel handle */
   BAST_Agc which_agc,    /* [in] AGC select */
   bool bFreeze           /* [in] true = freeze, false = unfreeze */
)
{
   uint32_t and_mask, or_mask;
   uint16_t addr;

   addr = h->channel == 0 ? 0x1000 : 0x2000;
   addr |= which_agc == BAST_Agc_eIF ? 0x90 : 0x94;
   and_mask = 0x000000FC;
   or_mask = bFreeze ? 0x00000002 : 0x00000000;
   return BAST_4501_P_HabIsbEdit(h, addr, and_mask, or_mask);
}


/******************************************************************************
 BERR_Code BAST_4501_P_FreezeEq()
******************************************************************************/
BERR_Code BAST_4501_P_FreezeEq(
   BAST_ChannelHandle h,  /* [in] BAST handle */
   bool bFreeze           /* [in] true = freeze, false = unfreeze */
)
{
   uint32_t and_mask, or_mask;
   uint16_t addr;

   addr = h->channel == 0 ? 0x10EC : 0x20EC;
   and_mask = 0x000000F8;
   or_mask = bFreeze ? 0x00000007 : 0x00000002;
   return BAST_4501_P_HabIsbEdit(h, addr, and_mask, or_mask);
}


/******************************************************************************
 BAST_4501_P_PowerDown()
******************************************************************************/
BERR_Code BAST_4501_P_PowerDown(
   BAST_ChannelHandle h,  /* [in] BAST handle */
   uint32_t options       /* [in] see BAST_CORE_* macros */
)
{
   uint8_t buf[4];

   buf[0] = 0x22;
   buf[1] = 0;
   if (options & BAST_CORE_SDS)
      buf[1] |= (h->channel ? 0x02 : 0x01);
   if (options & BAST_CORE_DISEQC)
      buf[1] |= (h->channel ? 0x08 : 0x04);

   return BAST_4501_P_SendHabCommand(h->pDevice, buf, 3, buf, true);
}


/******************************************************************************
 BAST_4501_P_PowerUp()
******************************************************************************/
BERR_Code BAST_4501_P_PowerUp(
   BAST_ChannelHandle h,  /* [in] BAST channel handle */
   uint32_t options       /* [in] see BAST_CORE_* macros */
)
{
   uint8_t buf[4];

   buf[0] = 0x25;
   buf[1] = 0;
   if (options & BAST_CORE_SDS)
      buf[1] |= (h->channel ? 0x02 : 0x01);
   if (options & BAST_CORE_DISEQC)
      buf[1] |= (h->channel ? 0x08 : 0x04);
  
   return BAST_4501_P_SendHabCommand(h->pDevice, buf, 3, buf, true);
}


/******************************************************************************
 BAST_4501_P_ReadRegister()
******************************************************************************/
BERR_Code BAST_4501_P_ReadRegister(
   BAST_ChannelHandle h,     /* [in] BAST channel handle */
   uint32_t           reg,   /* [in] address of register to read */
   uint32_t           *val   /* [in] contains data that was read */
)
{
   BERR_Code retCode;
#if 0
   uint8_t sb;

   if ((reg >= 0x80) && (reg <= 0xFF))
   {
      /* accessing host register space */
      BAST_CHK_RETCODE(BAST_ReadHostRegister(h->pDevice, reg, &sb));
      *val = (uint32_t)sb;
   }
   else
#endif
   {
      /* accessing internal registers through io_mbox */
      if ((reg >= 0x1000) && (reg < 0x2000) && (h->channel == 1))
         reg = (reg & 0x00000FFF) | 0x2000;
      else if ((reg >= 0x2000) && (reg < 0x3000) && (h->channel == 0))
         reg = (reg & 0x00000FFF) | 0x1000;
      BAST_CHK_RETCODE(BAST_4501_P_ReadMbox(h->pDevice, (uint16_t)reg, val));
   }

   done:
   return retCode;
}


/******************************************************************************
 BAST_4501_P_WriteRegister()
******************************************************************************/
BERR_Code BAST_4501_P_WriteRegister(
   BAST_ChannelHandle h,     /* [in] BAST channel handle */
   uint32_t           reg,   /* [in] address of register to read */
   uint32_t           *val   /* [in] contains data that was read */
)
{
   BERR_Code retCode;
#if 0
   uint8_t sb;

   if ((reg >= 0x80) && (reg <= 0xFF))
   {
      /* accessing host register space */
      sb = (uint8_t)(*val);
      BAST_CHK_RETCODE(BAST_WriteHostRegister(h->pDevice, reg, &sb));
   }
   else
#endif
   {
      /* accessing internal registers through io_mbox */
      if ((reg >= 0x1000) && (reg < 0x2000) && (h->channel == 1))
         reg = (reg & 0x00000FFF) | 0x2000;
      else if ((reg >= 0x2000) && (reg < 0x3000) && (h->channel == 0))
         reg = (reg & 0x00000FFF) | 0x1000;
      BAST_CHK_RETCODE(BAST_4501_P_WriteMbox(h->pDevice, (uint16_t)reg, val));
   }

   done:
   return retCode;
}


/******************************************************************************
 BAST_4501_P_GetInterruptEventHandle()
******************************************************************************/
BERR_Code BAST_4501_P_GetInterruptEventHandle(
   BAST_Handle h,            /* [in] BAST handle */
   BKNI_EventHandle *hEvent  /* [out] interrupt event handle */
)
{
   *hEvent = ((BAST_4501_P_Handle *)(h->pImpl))->hInterruptEvent;
   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_4501_P_GetLockStateChangeEventHandle()
******************************************************************************/
BERR_Code BAST_4501_P_GetLockStateChangeEventHandle(
   BAST_ChannelHandle h,       /* [in] BAST channel handle */
   BKNI_EventHandle   *hEvent  /* [out] lock event handle */
)
{
   *hEvent = ((BAST_4501_P_ChannelHandle *)(h->pImpl))->hLockChangeEvent;
   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_4501_P_GetFtmEventHandle()
******************************************************************************/
BERR_Code BAST_4501_P_GetFtmEventHandle(
   BAST_Handle h,            /* [in] BAST handle */
   BKNI_EventHandle *hEvent  /* [out] lock event handle */
)
{
   *hEvent = ((BAST_4501_P_Handle *)(h->pImpl))->hFtmEvent;
   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_4501_P_GetDiseqcEventHandle()
******************************************************************************/
BERR_Code BAST_4501_P_GetDiseqcEventHandle(
   BAST_ChannelHandle h,     /* [in] BAST channel handle */
   BKNI_EventHandle *hEvent  /* [out] lock event handle */
)
{
   *hEvent = ((BAST_4501_P_ChannelHandle *)(h->pImpl))->hDiseqcEvent;
   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_4501_P_ReadMbox()
******************************************************************************/
BERR_Code BAST_4501_P_ReadMbox(
   BAST_Handle h,    /* [in] BAST handle */
   uint16_t    reg,  /* [in] RBUS register address */
   uint32_t    *val  /* [out] value read from register */
)
{
   BAST_4501_P_Handle *p4501 = (BAST_4501_P_Handle *)(h->pImpl);
   BERR_Code retCode;
   uint8_t sb, i, buf[4];

   /* update bits 15:8 of mbox address if changed */
   sb = reg >> 8;
   if (sb != p4501->last_mbox_15_8)
   {
      BAST_CHK_RETCODE(BAST_WriteHostRegister(h, BCM4501_SH_SFR_IO_MBOX_A_15_8, &sb));
      p4501->last_mbox_15_8 = sb;
   }

   /* read from mbox */
   sb = reg & 0xFC;
   BAST_CHK_RETCODE(BAST_WriteHostRegister(h, BCM4501_SH_SFR_IO_MBOX_CMD, &sb));

   /* check for mbox transfer complete */
   for (i = 0; i < 3; i++)
   {
      BAST_CHK_RETCODE(BAST_ReadHostRegister(h, BCM4501_SH_SFR_IO_MBOX_STATUS, &sb));
      if ((sb & 0x80) == 0)
      {
         if (sb & 0x40)
         {
            BERR_TRACE(retCode = BAST_ERR_IOMB_XFER);
            goto done;
         }
         else
         {
            /* transfer completed - now get the data */
            BAST_CHK_RETCODE(BREG_I2C_Read(p4501->hRegister, h->settings.i2c.chipAddr, BCM4501_SH_SFR_IO_MBOX_D_31_24, buf, 4));
            *val = (buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3];
         }
         break;
      }
   }

   if (i >= 3)
   {
      /* this should not happen */
      BERR_TRACE(retCode = BAST_ERR_IOMB_BUSY);
      BDBG_ERR(("IO_MBOX busy\n"));
   }

   done:
   return retCode;
}


/******************************************************************************
 BAST_4501_P_WriteMbox()
******************************************************************************/
BERR_Code BAST_4501_P_WriteMbox(
   BAST_Handle h,    /* [in] BAST handle */
   uint16_t    reg,  /* [in] RBUS register address */
   uint32_t    *val  /* [in] value to write */
)
{
   BAST_4501_P_Handle *p4501 = (BAST_4501_P_Handle *)(h->pImpl);
   BERR_Code retCode;
   uint8_t buf[6], sb, i;

   /* write addr, data, and cmd in one i2c transaction */
   buf[0] = (uint8_t)(reg >> 8);
   buf[1] = (uint8_t)(*val >> 24);
   buf[2] = (uint8_t)(*val >> 16);
   buf[3] = (uint8_t)(*val >> 8);
   buf[4] = (uint8_t)(*val & 0xFF); 
   buf[5] = (uint8_t)((reg & 0xFC) | 0x01);
   i = (buf[0] != p4501->last_mbox_15_8) ? 0 : 1;
   p4501->last_mbox_15_8 = buf[0];
   BAST_CHK_RETCODE(BREG_I2C_Write(p4501->hRegister, h->settings.i2c.chipAddr, (uint8_t)(i ? BCM4501_SH_SFR_IO_MBOX_D_31_24 : BCM4501_SH_SFR_IO_MBOX_A_15_8), &buf[i], 6-i));

   for (i = 0; i < 3; i++)
   {
      /* check for mbox transfer complete */
      BAST_CHK_RETCODE(BAST_ReadHostRegister(h, BCM4501_SH_SFR_IO_MBOX_STATUS, &sb));
      if ((sb & 0x80) == 0)
      {
         if (sb & 0x40)
         {
            BERR_TRACE(retCode = BAST_ERR_IOMB_XFER);
         }
         goto done;
      }
   }

   if (sb & 0x80)
   {
      /* this should not happen */
      BERR_TRACE(retCode = BAST_ERR_IOMB_BUSY);
      BDBG_ERR(("IO_MBOX busy\n"));
   }

   done:
   return retCode;
}


/******************************************************************************
 BAST_4501_P_SetApWindow()
******************************************************************************/
BERR_Code BAST_4501_P_SetApWindow(
   BAST_Handle h,    /* [in] BAST handle */
   uint32_t window   /* [in] base address of the 128-byte window */
)
{
   BAST_4501_P_Handle *p4501 = (BAST_4501_P_Handle *)(h->pImpl);
   BERR_Code retCode = BERR_SUCCESS;
   uint8_t   haddr_16_15 = (uint8_t)((window >> 15) & 0x03);
   uint8_t   haddr_14_7 = (uint8_t)((window >> 7) & 0xFF);
   uint8_t   buf[2];

   if (p4501->last_page_16_15 != haddr_16_15)
   {
      buf[0] = haddr_16_15;
      buf[1] = haddr_14_7;
      BAST_CHK_RETCODE(BREG_I2C_Write(p4501->hRegister, h->settings.i2c.chipAddr, BCM4501_SH_SFR_H_ADR_16_15, buf, 2));
      p4501->last_page_16_15 = haddr_16_15;
      p4501->last_page_14_7 = haddr_14_7;
   }
   else if (p4501->last_page_14_7 != haddr_14_7)
   {
      BAST_CHK_RETCODE(BAST_WriteHostRegister(h, BCM4501_SH_SFR_H_ADR_14_7, &haddr_14_7));
      p4501->last_page_14_7 = haddr_14_7;
   }

   done:
   return retCode;
}


/******************************************************************************
 BAST_4501_P_ReadHab()
******************************************************************************/
BERR_Code BAST_4501_P_ReadHab(BAST_Handle h, uint8_t addr, uint8_t *buf, uint8_t n)
{
   BAST_4501_P_Handle *p4501 = (BAST_4501_P_Handle *)(h->pImpl);
   BERR_Code retCode;

   if ((addr & 0x80) || (n & 0x80))
      return BERR_TRACE(BERR_INVALID_PARAMETER);

   if ((addr + n) > 0x80)
      return BERR_TRACE(BERR_INVALID_PARAMETER);

   BAST_CHK_RETCODE(BAST_4501_P_SetApWindow(h, BAST_WINDOW_HAB));
   BAST_CHK_RETCODE(BREG_I2C_Read(p4501->hRegister, h->settings.i2c.chipAddr, addr, buf, n));

   done:
   return retCode;
}


/******************************************************************************
 BAST_4501_P_WriteHab()
******************************************************************************/
BERR_Code BAST_4501_P_WriteHab(BAST_Handle h, uint8_t addr, uint8_t *buf, uint8_t n)
{
   BAST_4501_P_Handle *p4501 = (BAST_4501_P_Handle *)(h->pImpl);
   BERR_Code retCode;

   if ((addr & 0x80) || (n & 0x80))
      return BERR_TRACE(BERR_INVALID_PARAMETER);

   if ((addr + n) > 0x80)
      return BERR_TRACE(BERR_INVALID_PARAMETER);

   BAST_CHK_RETCODE(BAST_4501_P_SetApWindow(h, BAST_WINDOW_HAB));
   BAST_CHK_RETCODE(BREG_I2C_Write(p4501->hRegister, h->settings.i2c.chipAddr, addr, buf, n));

   done:
   return retCode;
}


/******************************************************************************
 BAST_4501_P_ReadMemory()
******************************************************************************/
BERR_Code BAST_4501_P_ReadMemory(BAST_Handle h, uint16_t addr, uint8_t *buf, uint16_t n)
{
   BAST_4501_P_Handle *p4501 = (BAST_4501_P_Handle *)(h->pImpl);
   BAST_ApStatus status;
   BERR_Code retCode;

   if (((uint32_t)addr + (uint32_t)n) > 0x10000)
      return BERR_TRACE(BERR_INVALID_PARAMETER);

   BAST_CHK_RETCODE(BAST_4501_P_GetApStatus(h, &status));
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
         BAST_CHK_RETCODE(BAST_4501_P_SendHabCommand(h, hab, (uint8_t)(5+bytes_to_read), hab, true));
         for (i = 0; i < bytes_to_read; i++)
            buf[i] = hab[4+i]; 
         buf += bytes_to_read;
         addr += bytes_to_read;
      }
   }
   else
   {
      BAST_CHK_RETCODE(BAST_4501_P_SetApWindow(h, BAST_WINDOW_IRAM + addr));
      BAST_CHK_RETCODE(BREG_I2C_Read(p4501->hRegister, h->settings.i2c.chipAddr, (uint8_t)(addr & 0x7F), buf, n));
      p4501->last_page_16_15 = p4501->last_page_14_7 = 0xFF; 
   }

   done:
   return retCode;
}


/******************************************************************************
 BAST_4501_P_WriteMemory()
******************************************************************************/
BERR_Code BAST_4501_P_WriteMemory(BAST_Handle h, uint16_t addr, const uint8_t *buf, uint16_t n)
{
   BAST_4501_P_Handle *p4501 = (BAST_4501_P_Handle *)(h->pImpl);
   BAST_ApStatus status;
   BERR_Code retCode = BERR_SUCCESS;
   uint16_t  curr_addr, nbytes, bytes_left;

#ifndef BAST_DEBUG 
   if ((addr >= 0xA000) || (n > 0xA000))
      return BERR_TRACE(BERR_INVALID_PARAMETER);
   if ((addr + n) > 0xA000)
      return BERR_TRACE(BERR_INVALID_PARAMETER);
#endif

   BAST_CHK_RETCODE(BAST_4501_P_GetApStatus(h, &status));
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

         BAST_CHK_RETCODE(BAST_4501_P_SendHabCommand(h, hab, (uint8_t)(5+bytes_to_write), hab, true));
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
         BAST_CHK_RETCODE(BAST_4501_P_SetApWindow(h, BAST_WINDOW_IRAM + curr_addr));
         nbytes = 128 - (curr_addr % 128);
         if (nbytes > bytes_left)
            nbytes = bytes_left;
         bytes_left -= nbytes;
         BAST_CHK_RETCODE(BREG_I2C_Write(p4501->hRegister, h->settings.i2c.chipAddr, (uint8_t)(curr_addr & 0x7F), buf, nbytes));
         curr_addr += nbytes;
         buf += nbytes;
      }
   }

   done:
   return retCode;
}


/******************************************************************************
 BAST_4501_P_ReadConfig()
******************************************************************************/
BERR_Code BAST_4501_P_ReadConfig(
   BAST_ChannelHandle h,   /* [in] BAST channel handle */
   uint16_t id,            /* [in] configuration parameter ID */
   uint8_t *p,                /* [in] buffer to hold the read data */
   uint8_t len             /* [in] number of bytes to read */
)
{
   BERR_Code retCode;
   uint16_t  addr = id;
   uint8_t   hab[32], i;

   BDBG_ASSERT((len == 1) || (len == 2) || (len == 4) || (len == 16));
   BDBG_ASSERT((len+id) < 0x27F);

   switch (id)
   {
      case BCM4501_CONFIG_DISEQC_CTL1:
      case BCM4501_CONFIG_DISEQC_CTL2:
      case BCM4501_CONFIG_RRTO_USEC:
      case BCM4501_CONFIG_TUNER_CUTOFF:
      case BCM4501_CONFIG_SYMBOL_RATE:
      case BCM4501_CONFIG_ACQ_CTL:
      case BCM4501_CONFIG_XPORT_CTL:
      case BCM4501_CONFIG_BCM3440_ADDRESS:
      case BCM4501_CONFIG_TUNER_FLAGS:
      case BCM4501_CONFIG_ALT_FLIF:
      case BCM4501_CONFIG_LDPC_SCRAMBLING_SEQ:
      case BCM4501_CONFIG_XPORT_CLOCK_ADJUST:
      case BCM4501_CONFIG_STUFF_BYTES:
      case BCM4501_CONFIG_DISEQC_CMD_COUNT:
      case BCM4501_CONFIG_DISEQC_IRQ_COUNT:
         addr += (h->channel ? len : 0);
         break;
   }

   hab[0] = 0x09;
   hab[1] = (addr >> 8);
   hab[2] = (addr & 0xFF);   
   hab[3] = len;
   BAST_CHK_RETCODE(BAST_4501_P_SendHabCommand(h->pDevice, hab, (uint8_t)(5+len), hab, true));

   for (i = 0; i < len; i++)
      p[i] = hab[i+4];

   done:
   return retCode;
}


/******************************************************************************
 BAST_4501_P_WriteConfig()
******************************************************************************/
BERR_Code BAST_4501_P_WriteConfig(
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
   BDBG_ASSERT((len+id) < 0x27F);

   switch (id)
   {
      case BCM4501_CONFIG_DISEQC_CTL1:
      case BCM4501_CONFIG_DISEQC_CTL2:
      case BCM4501_CONFIG_RRTO_USEC:
      case BCM4501_CONFIG_TUNER_CUTOFF:
      case BCM4501_CONFIG_SYMBOL_RATE:
      case BCM4501_CONFIG_ACQ_CTL:
      case BCM4501_CONFIG_XPORT_CTL:
      case BCM4501_CONFIG_BCM3440_ADDRESS:
      case BCM4501_CONFIG_TUNER_FLAGS:
      case BCM4501_CONFIG_ALT_FLIF:
      case BCM4501_CONFIG_LDPC_SCRAMBLING_SEQ:
      case BCM4501_CONFIG_XPORT_CLOCK_ADJUST:
      case BCM4501_CONFIG_STUFF_BYTES:
      case BCM4501_CONFIG_DISEQC_CMD_COUNT:
      case BCM4501_CONFIG_DISEQC_IRQ_COUNT:
         addr += (h->channel ? len : 0);
         break;
   }

   hab[0] = 0x0A;
   hab[1] = (addr >> 8);
   hab[2] = (addr & 0xFF);
   hab[3] = len;

   for (i = 0; i < len; i++)
      hab[i+4] = p[i];

   BAST_CHK_RETCODE(BAST_4501_P_SendHabCommand(h->pDevice, hab, (uint8_t)(5+len), hab, true));
 
   done:
   return retCode;
}


/******************************************************************************
 BAST_4501_P_ResetAp()
******************************************************************************/
BERR_Code BAST_4501_P_ResetAp(BAST_Handle h)
{
   BAST_4501_P_Handle *p4501 = (BAST_4501_P_Handle *)(h->pImpl);
   BERR_Code retCode;
   uint32_t tm_pin_strap_ovrd = 0;
   uint8_t sb;

   BAST_CHK_RETCODE(BAST_4501_P_ReadRegister(h->pChannels[0], BCM4501_TM_PIN_STRAP_OVRD, &tm_pin_strap_ovrd));
   tm_pin_strap_ovrd |= 0x04000000;
   tm_pin_strap_ovrd &= ~0x00020000;
   BAST_CHK_RETCODE(BAST_4501_P_WriteRegister(h->pChannels[0], BCM4501_TM_PIN_STRAP_OVRD, &tm_pin_strap_ovrd));

   /* SW workaround to delay after switching tm_pin_strap_ovrd[17] */
   for (sb = 0; sb < 3; sb++)
   {
      BAST_CHK_RETCODE(BAST_4501_P_ReadRegister(h->pChannels[0], BCM4501_TM_PIN_STRAP_OVRD, &tm_pin_strap_ovrd));
   }

   /* initialize JDEC */
   sb = BCM4501_JDEC_RAM2;
   BAST_CHK_RETCODE(BAST_WriteHostRegister(h, BCM4501_SH_AP_SFR_JDEC, &sb));

   /* reset the AP */
   sb = BCM4501_AP_RESET;
   BAST_CHK_RETCODE(BAST_WriteHostRegister(h, BCM4501_SH_AP_SFR_H_CTL1, &sb));

   /* reset AP status */
   sb = 0x0A;
   BAST_CHK_RETCODE(BAST_WriteHostRegister(h, BCM4501_SH_AP_SFR_AP_CMD, &sb));

   sb = 0x00;
   BAST_CHK_RETCODE(BAST_WriteHostRegister(h, BCM4501_SH_AP_SFR_H_MSG1, &sb));
   BAST_CHK_RETCODE(BAST_WriteHostRegister(h, BCM4501_SH_AP_SFR_H_MSG2, &sb));

   /* verify that AP is reset */
   BAST_CHK_RETCODE(BAST_ReadHostRegister(h, BCM4501_SH_AP_SFR_H_CTL1, &sb));
   if ((sb & BCM4501_AP_MASK) != BCM4501_AP_RESET)
   {
      BDBG_ERR(("unable to reset the AP\n"));
      BERR_TRACE(retCode = BAST_ERR_AP_FAIL);
   }

   done:
   p4501->last_page_16_15 = 0xFF;
   p4501->last_page_14_7 = 0xFF;
   return retCode;
}


/******************************************************************************
 BAST_4501_P_RunAp()
******************************************************************************/
BERR_Code BAST_4501_P_RunAp(BAST_Handle h)
{
   BERR_Code retCode;
   uint8_t   sb, sb2;

   /* check if the AP is currently running */
   BAST_CHK_RETCODE(BAST_ReadHostRegister(h, BCM4501_SH_AP_SFR_H_CTL1, &sb)); 

   if ((sb & BCM4501_AP_MASK) != BCM4501_AP_RUN)
   {
      /* start running the AP */
      sb2 = BCM4501_AP_RUN;
      BAST_CHK_RETCODE(BAST_WriteHostRegister(h, BCM4501_SH_AP_SFR_H_CTL1, &sb2));

      /* verify that the AP is running */
      BAST_CHK_RETCODE(BAST_ReadHostRegister(h, BCM4501_SH_AP_SFR_H_CTL1, &sb));
      if ((sb & BCM4501_AP_MASK) != BCM4501_AP_RUN)
      {
         BDBG_ERR(("unable to run the AP\n"));
         BERR_TRACE(retCode = BAST_ERR_AP_FAIL);
         goto done;
      }

      /* clear AP_change state bit */
      sb2 = BCM4501_STAT1_AP_OP_CHG;
      BAST_CHK_RETCODE(BAST_WriteHostRegister(h, BCM4501_SH_AP_SFR_H_FSTAT1, &sb2)); 
   }
   else
   {
      BDBG_WRN(("BAST_4501_P_RunAp(): AP already running\n"));
   }

   done:
   return retCode;
}


/******************************************************************************
 BAST_4501_P_CheckHab()
******************************************************************************/
BERR_Code BAST_4501_P_CheckHab(
   BAST_Handle h    /* [in] BAST handle */
)
{
   BERR_Code retCode, apError;
   BAST_ApStatus status;
   
   BAST_CHK_RETCODE(BAST_GetApStatus(h, &status));
   apError = BAST_4501_P_DecodeError(h, &status);
   if (apError)
   {
      /* report potential AP error */
      BDBG_WRN(("@@@ AP status=0x%08X | error=0x%X", (uint32_t)status, apError));
   }
   
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
 BAST_4501_P_DecodeError()
******************************************************************************/
BERR_Code BAST_4501_P_DecodeError(
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
      BAST_CHK_RETCODE(BAST_ReadHostRegister(h, BCM4501_SH_AP_SFR_H_MSG1, &sb));
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

         case 9:
            retCode = BAST_ERR_POWERED_DOWN;
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

         /* non-critical errors */
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

         default:
            BDBG_ERR(("unknown MSG1 (=0x%02X)\n", sb));
            retCode = BAST_ERR_AP_UNKNOWN;
            break;
      }
      
      /* clear H_MSG1 and H_STAT2[0] if non-critical error */
      if (sb >= 0x0F)
      {
         sb = 0x00;
         BAST_WriteHostRegister(h, BCM4501_SH_AP_SFR_H_MSG1, &sb);
         sb = 0x01;
         BAST_WriteHostRegister(h, BCM4501_SH_AP_SFR_H_STAT2, &sb);
      }
   }

   done:
   return retCode;
}


/******************************************************************************
 BAST_4501_P_SendHabCommand()
******************************************************************************/
BERR_Code BAST_4501_P_SendHabCommand(
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

   BAST_CHK_RETCODE(BAST_4501_P_CheckHab(h));
  
   /* zero out the HAB */
   sb = 0x0C;
   BAST_CHK_RETCODE(BAST_WriteHostRegister(h, BCM4501_SH_AP_SFR_AP_CMD, &sb));

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
   BAST_CHK_RETCODE(BAST_4501_P_WriteHab(h, 0, write_buf, write_len));

   /* wait for the AP to service the HAB, and then read any return data */
   BAST_CHK_RETCODE(BAST_4501_P_ServiceHab(h, read_buf, write_len, (uint8_t)(write_buf[0] | 0x80), bChecksum));
 
   done:
   return retCode;
}


/******************************************************************************
 BAST_4501_P_ServiceHab()
******************************************************************************/
BERR_Code BAST_4501_P_ServiceHab(
   BAST_Handle h,   /* [in] BAST handle */
   uint8_t *read_buf,  /* [out] holds the data read from the HAB */ 
   uint8_t read_len,   /* [in] number of bytes to read from the HAB (including the checksum byte) */
   uint8_t ack_byte,   /* [in] value of the ack byte to expect */
   bool    bChecksum   /* [in] true = automatically compute checksum */
)
{
   BAST_4501_P_Handle *p4501 = (BAST_4501_P_Handle *)(h->pImpl);
   BERR_Code retCode;
#ifdef BAST_DONT_USE_INTERRUPT
   uint16_t  i;
#endif
   uint8_t   idx, cs, sb;

   /* clear the HAB_DONE interrupt */
   sb = BCM4501_STAT1_HAB_DONE;
   BAST_CHK_RETCODE(BAST_WriteHostRegister(h, BCM4501_SH_AP_SFR_H_FSTAT1, &sb));
   BKNI_WaitForEvent(p4501->hHabDoneEvent, 0);

#ifndef BAST_DONT_USE_INTERRUPT
   /* enable HAB_DONE interrupt */
   BAST_CHK_RETCODE(BAST_ReadHostRegister(h, BCM4501_SH_AP_SFR_H_IE1, &sb));
   sb |= BCM4501_STAT1_HAB_DONE;
   BAST_CHK_RETCODE(BAST_WriteHostRegister(h, BCM4501_SH_AP_SFR_H_IE1, &sb));
#endif
   
   /* send the command */
   sb = BCM4501_AP_HABR;
   BAST_CHK_RETCODE(BAST_WriteHostRegister(h, BCM4501_SH_AP_SFR_H_CTL1, &sb));
   
#ifdef BAST_DONT_USE_INTERRUPT
   /* wait for HAB to be serviced (polling) */
   for (i = 0; i < 1111; i++)
   {
      BAST_CHK_RETCODE(BAST_ReadHostRegister(h, BCM4501_SH_AP_SFR_H_STAT1, &sb));
      if (sb & BCM4501_STAT1_HAB_DONE)
         break;
   }
   if ((sb & BCM4501_STAT1_HAB_DONE) == 0)
   {
      BERR_TRACE(retCode = BAST_ERR_HAB_TIMEOUT);
      goto done;
   }
#else
   /* wait for HAB done interrupt */  
   if (BAST_4501_P_WaitForEvent(h, p4501->hHabDoneEvent, 500) == BERR_TIMEOUT)
   {
      uint8_t   h_stat[3], h_ie[3];

      BAST_CHK_RETCODE(BREG_I2C_Read(p4501->hRegister, h->settings.i2c.chipAddr, BCM4501_SH_AP_SFR_H_FSTAT1, h_stat, 3));
      BAST_CHK_RETCODE(BREG_I2C_Read(p4501->hRegister, h->settings.i2c.chipAddr, BCM4501_SH_AP_SFR_H_IE1, h_ie, 3));
      BDBG_ERR(("HAB timeout: fstat1=0x%02X, fstat2=%02X, fstat3=0x%02X, ie1=0x%02X, ie2=0x%02X, ie3=0x%02X", h_stat[0], h_stat[1], h_stat[2], h_ie[0], h_ie[1], h_ie[2]));   
      BERR_TRACE(retCode = BAST_ERR_HAB_TIMEOUT);
      goto done;
   }
#endif
   
   /* get the HAB contents */
   BAST_CHK_RETCODE(BAST_4501_P_ReadHab(h, 0, read_buf, read_len));

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
#if 0
         for (idx = 0; idx < read_len; idx++)
         {
            BDBG_ERR(("read_buf[%d] = 0x%02X\n", idx, read_buf[idx]));
         }
#endif
         BERR_TRACE(retCode = BAST_ERR_HAB_CHECKSUM);
      }
   }

   done:
   return retCode;
}


/******************************************************************************
 BAST_4501_P_EnableHostInterrupt() 
******************************************************************************/
BERR_Code BAST_4501_P_EnableHostInterrupt(
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
 BAST_4501_P_EnableLockInterrupt()
******************************************************************************/ 
BERR_Code BAST_4501_P_EnableLockInterrupt(
   BAST_ChannelHandle h, /* [in] BAST channel handle */
   bool bEnable          /* [in] true = enable lock interrupts, false = disables lock interrupts */
)
{
   /* BAST_4501_P_ChannelHandle *p4501 = (BAST_4501_P_ChannelHandle *)(h->pImpl); */
   BERR_Code retCode;
   uint8_t sb, h_stat3, h_ie3;
   
   /* BAST_4501_P_EnableHostInterrupt(h->pDevice, false); */

   sb = h->channel ? 0x60 : 0x06;
   BAST_CHK_RETCODE(BAST_WriteHostRegister(h->pDevice, BCM4501_SH_AP_SFR_H_FSTAT3, &sb));
   if (bEnable)
   {
      BAST_CHK_RETCODE(BAST_ReadHostRegister(h->pDevice, BCM4501_SH_AP_SFR_H_STAT3, &h_stat3));
      BAST_CHK_RETCODE(BAST_ReadHostRegister(h->pDevice, BCM4501_SH_AP_SFR_H_IE3, &h_ie3));
      if (h->channel)
         h_ie3 |= 0x60; /* ((h_stat3 & BCM4501_STAT3_CH1_RT_LOCK) ? BCM4501_STAT3_CH1_NOT_LOCK : BCM4501_STAT3_CH1_LOCK); */
      else
         h_ie3 |= 0x06; /* ((h_stat3 & BCM4501_STAT3_CH0_RT_LOCK) ? BCM4501_STAT3_CH0_NOT_LOCK : BCM4501_STAT3_CH0_LOCK); */
      BAST_CHK_RETCODE(BAST_WriteHostRegister(h->pDevice, BCM4501_SH_AP_SFR_H_IE3, &h_ie3));

      /* BKNI_WaitForEvent(p4501->hLockChangeEvent, 0); */
   }

   done:
   /* BAST_4501_P_EnableHostInterrupt(h->pDevice, true); */
   return retCode;   
}


/******************************************************************************
 BAST_4501_P_EnableFtmInterrupt()
******************************************************************************/
BERR_Code BAST_4501_P_EnableFtmInterrupt(
   BAST_Handle h, /* [in] BAST handle */
   bool bEnable   /* [in] true = enable FTM interrupts, false = disables FTM interrupts */
)
{
   BERR_Code retCode;
   uint8_t sb;

   /* BAST_4501_P_EnableHostInterrupt(h, false); */

   sb = BCM4501_STAT2_FTM;
   BAST_CHK_RETCODE(BAST_WriteHostRegister(h, BCM4501_SH_AP_SFR_H_FSTAT2, &sb));
   if (bEnable)
   {
      BAST_CHK_RETCODE(BAST_ReadHostRegister(h, BCM4501_SH_AP_SFR_H_IE2, &sb));
      sb |= BCM4501_STAT2_FTM;
      BAST_CHK_RETCODE(BAST_WriteHostRegister(h, BCM4501_SH_AP_SFR_H_IE2, &sb));
   }

   done:
   /* BAST_4501_P_EnableHostInterrupt(h, true); */
   return retCode; 
}


/******************************************************************************
 BAST_4501_P_DecodeInterrupt()
******************************************************************************/
BERR_Code BAST_4501_P_DecodeInterrupt(BAST_Handle h)
{
   BAST_4501_P_Handle *p4501 = (BAST_4501_P_Handle *)(h->pImpl);
   BAST_4501_P_ChannelHandle *pChn;
   BERR_Code retCode;
   uint8_t   h_fstat[4], h_ie[2], bInitDone = 0;

   BAST_CHK_RETCODE(BREG_I2C_Read(p4501->hRegister, h->settings.i2c.chipAddr, BCM4501_SH_AP_SFR_H_FSTAT1, h_fstat, 4));
   BAST_CHK_RETCODE(BREG_I2C_Read(p4501->hRegister, h->settings.i2c.chipAddr, BCM4501_SH_AP_SFR_H_IE2, h_ie, 2));
   
   if (!h_fstat[0] && !h_fstat[1] && !h_fstat[2] && !h_fstat[3])
      return BERR_SUCCESS;
   
   if (h_fstat[0] & BCM4501_STAT1_HAB_DONE)
      BKNI_SetEvent(p4501->hHabDoneEvent);
   
   if (h_fstat[1] & BCM4501_STAT2_INIT_DONE)
   {
      BKNI_SetEvent(p4501->hInitDoneEvent);
      h_ie[0] &= ~BCM4501_STAT2_INIT_DONE;
      h_fstat[1] &= ~BCM4501_STAT2_INIT_DONE;
      bInitDone = 1;
   }
      
   if (h_fstat[1] & BCM4501_STAT2_FTM)
   {
      BKNI_SetEvent(p4501->hFtmEvent);
      h_ie[0] |= BCM4501_STAT2_FTM;
   }

   if (h_fstat[2] & 0x07)
   {
      /* channel 0 interrupt */
      pChn = (BAST_4501_P_ChannelHandle *)(h->pChannels[0]->pImpl);
      if (h_fstat[2] & BCM4501_STAT3_CH0_DS_DONE)
      {
         BKNI_SetEvent(pChn->hDiseqcEvent);
         h_ie[1] &= ~BCM4501_STAT3_CH0_DS_DONE;
      }
      if (h_fstat[2] & (BCM4501_STAT3_CH0_LOCK | BCM4501_STAT3_CH0_NOT_LOCK))
      {
         h_ie[1] |= (BCM4501_STAT3_CH0_LOCK | BCM4501_STAT3_CH0_NOT_LOCK);
         BKNI_SetEvent(pChn->hLockChangeEvent);
      }
   }

   if (h_fstat[2] & 0x70)
   {
      /* channel 1 interrupt */
      pChn = (BAST_4501_P_ChannelHandle *)(h->pChannels[1]->pImpl);
      if (h_fstat[2] & BCM4501_STAT3_CH1_DS_DONE)
      {
         BKNI_SetEvent(pChn->hDiseqcEvent);
         h_ie[1] &= ~BCM4501_STAT3_CH1_DS_DONE;
      }
      if (h_fstat[2] & (BCM4501_STAT3_CH1_LOCK | BCM4501_STAT3_CH1_NOT_LOCK))
      {
         h_ie[1] |= (BCM4501_STAT3_CH1_LOCK | BCM4501_STAT3_CH1_NOT_LOCK);
         BKNI_SetEvent(pChn->hLockChangeEvent);
      }
   }

   h_fstat[2] &= ~0x88;

   if (h_fstat[1] & BCM4501_STAT2_TUNER_0_DONE)
   {
      BKNI_SetEvent(((BAST_4501_P_ChannelHandle *)(h->pChannels[0]->pImpl))->hTunerEvent);
      h_ie[0] &= ~BCM4501_STAT2_TUNER_0_DONE;
   }

   if (h_fstat[1] & BCM4501_STAT2_TUNER_1_DONE)
   {
      BKNI_SetEvent(((BAST_4501_P_ChannelHandle *)(h->pChannels[1]->pImpl))->hTunerEvent);
      h_ie[0] &= ~BCM4501_STAT2_TUNER_1_DONE;
   }

#if 0
   if (h_fstat[3] & BCM4501_STAT4_MI2C_ERROR_0)
      BKNI_SetEvent(((BAST_4501_P_ChannelHandle *)(h->pChannels[0]->pImpl))->hMi2cEvent);

   if (h_fstat[3] & BCM4501_STAT4_MI2C_ERROR_1)
      BKNI_SetEvent(((BAST_4501_P_ChannelHandle *)(h->pChannels[1]->pImpl))->hMi2cEvent);
#endif

   /* clear the interrupt status */
   BAST_CHK_RETCODE(BREG_I2C_Write(p4501->hRegister, h->settings.i2c.chipAddr, BCM4501_SH_AP_SFR_H_FSTAT1, h_fstat, 3));   

   if (h_ie[0] || h_ie[1] || bInitDone)
   {
      BAST_CHK_RETCODE(BREG_I2C_Write(p4501->hRegister, h->settings.i2c.chipAddr, BCM4501_SH_AP_SFR_H_IE2, h_ie, 2));
   }

   done:
   return retCode;
}


/******************************************************************************
 BAST_4501_P_HandleInterrupt_isr()
******************************************************************************/
BERR_Code BAST_4501_P_HandleInterrupt_isr(
   BAST_Handle h /* [in] BAST handle */
)
{
   BDBG_ASSERT(h);
   
   h->settings.i2c.interruptEnableFunc(false, h->settings.i2c.interruptEnableFuncParam);
   BKNI_SetEvent(((BAST_4501_P_Handle *)(h->pImpl))->hApiEvent);   
   BKNI_SetEvent(((BAST_4501_P_Handle *)(h->pImpl))->hInterruptEvent);  
   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_4501_P_ProcessInterruptEvent()
******************************************************************************/
BERR_Code BAST_4501_P_ProcessInterruptEvent(
   BAST_Handle h  /* [in] BAST handle */
)
{
   BERR_Code retCode;
   
   BDBG_ASSERT(h);
   BAST_CHK_RETCODE(BAST_4501_P_DecodeInterrupt(h));
   BAST_4501_P_EnableHostInterrupt(h, true);
   
   done:
   return retCode;
}


/******************************************************************************
 BAST_4501_P_DisableInterrupts()
******************************************************************************/ 
BERR_Code BAST_4501_P_DisableInterrupts(
   BAST_Handle h   /* [in] BAST handle */
)
{
   BAST_4501_P_Handle *p4501 = (BAST_4501_P_Handle *)(h->pImpl);
   BERR_Code err;
   const uint8_t val[4] = {0, 0, 0, 0};
   
   /* clear IEx registers */
   err = BREG_I2C_Write(p4501->hRegister, h->settings.i2c.chipAddr, BCM4501_SH_AP_SFR_H_IE1, val, 4);
   return err;   
}



/******************************************************************************
 BERR_Code BAST_4501_P_WaitForEvent()
******************************************************************************/
BERR_Code BAST_4501_P_WaitForEvent(
   BAST_Handle h,             /* [in] BAST handle */
   BKNI_EventHandle hEvent,   /* [in] event to wait on */
   int timeoutMsec            /* [in] timeout in milliseconds */
)
{
   BAST_4501_P_Handle *p4501 = (BAST_4501_P_Handle *)(h->pImpl);
   BERR_Code retCode = BERR_SUCCESS;

#ifndef BAST_ALT_IMPL
   BAST_4501_P_EnableHostInterrupt(h, true);
   while (1)
   {   
      retCode = BKNI_WaitForEvent(p4501->hApiEvent, timeoutMsec);
      if ((retCode == BERR_TIMEOUT) || (retCode == BERR_OS_ERROR))
         break;
      else if (retCode == BERR_SUCCESS)
      {
         BAST_4501_P_DecodeInterrupt(h);
         if ((retCode = BKNI_WaitForEvent(hEvent, 0)) == BERR_SUCCESS)
            break;
      }
      BAST_4501_P_EnableHostInterrupt(h, true);
   }

   BAST_4501_P_EnableHostInterrupt(h, true);
#else
   /* First of all, verify whether we already got a message */     
   BAST_4501_P_DecodeInterrupt(h);
   BAST_4501_P_EnableHostInterrupt(h, true);

   if ((retCode = BKNI_WaitForEvent(hEvent, 0)) != BERR_SUCCESS)
   {
      /* It wasn't the expected message. Wait until timeout */
      while (1)
      {
         retCode = BKNI_WaitForEvent(p4501->hApiEvent, timeoutMsec);
         if ((retCode == BERR_TIMEOUT) || (retCode == BERR_OS_ERROR))
            break;
         else if (retCode == BERR_SUCCESS)
         {
            BAST_4501_P_DecodeInterrupt(h);
            BAST_4501_P_EnableHostInterrupt(h, true);
            if ((retCode = BKNI_WaitForEvent(hEvent, 0)) == BERR_SUCCESS)
               break;
         }
      }
   }
#endif
   return retCode;
}


/******************************************************************************
 BAST_4501_P_AbortAcq()
******************************************************************************/
BERR_Code BAST_4501_P_AbortAcq(
   BAST_ChannelHandle h  /* [in] BAST channel handle */
)
{
   uint8_t buf[4];

   buf[0] = 0x10 | (h->channel ? 0x40 : 0x00);
   buf[1] = buf[0];
  
   return BAST_4501_P_SendHabCommand(h->pDevice, buf, 2, buf, true);
}


/******************************************************************************
 BAST_4501_P_ConfigLna()
******************************************************************************/
BERR_Code BAST_4501_P_ConfigLna(
   BAST_Handle    h,             /* [in] BAST handle */
   BAST_LnaIntConfig int_config, /* [in] BCM3445 internal switch configuration */
   BAST_LnaExtConfig ext_config  /* [in] BCM3445 output mapping to SDS channels */
)
{
   uint8_t buf[4];
   
   if ((int_config == BAST_LnaIntConfig_eIn2Out1_In1Out2) || 
      (int_config == BAST_LnaIntConfig_eIn2Out1_In2Out2_DaisyOff) ||
      (ext_config == BAST_LnaExtConfig_eOut1Sds1_Out2Sds0))
      return BERR_NOT_SUPPORTED;

   buf[0] = 0x11;
   buf[1] = (uint8_t)(int_config & 0xF);
   buf[1] |= (uint8_t)((ext_config << 4) & 0xF0);
  
   return BAST_4501_P_SendHabCommand(h, buf, 3, buf, true);
}


/******************************************************************************
 BAST_GetLnaStatus()
******************************************************************************/
BERR_Code BAST_4501_P_GetLnaStatus(
   BAST_Handle    h,        /* [in] BAST handle */
   BAST_LnaStatus *pStatus  /* [out] lna status struct */
)
{
   BERR_Code retCode;
   uint8_t buf[8];

   if (pStatus == NULL)
      return (BERR_TRACE(BERR_INVALID_PARAMETER));

   buf[0] = 0x27;
   BAST_CHK_RETCODE(BAST_4501_P_SendHabCommand(h, buf, 6, buf, true));

   pStatus->agc0 = buf[3];
   pStatus->agc1 = buf[4];
   pStatus->version = buf[2];
   pStatus->int_config = (BAST_LnaIntConfig)(buf[1] & 0x0F);
   pStatus->ext_config = (BAST_LnaExtConfig)((buf[1] >> 4) & 0x0F);

   done:
   return retCode;
}


/******************************************************************************
 BAST_4501_P_ConfigAgc()
******************************************************************************/
BERR_Code BAST_4501_P_ConfigAgc(
   BAST_Handle h,             /* [in] BAST handle */
   uint32_t    ctl            /* [in] control word */
)
{
   uint8_t buf[4];

   buf[0] = 0x26;
   buf[1] = (uint8_t)(ctl & 0xFF);
  
   return BAST_4501_P_SendHabCommand(h, buf, 3, buf, true);
}


/******************************************************************************
 BAST_4501_P_ReadIdata()
******************************************************************************/
BERR_Code BAST_4501_P_ReadIdata(BAST_Handle h, uint8_t page, uint8_t addr, uint8_t *buf, uint8_t n)
{
   BERR_Code retCode;
   BAST_ApStatus status;
   uint8_t hab[128], i;

   if ((n == 0) || (n > 0x7B))
      return BERR_TRACE(BERR_INVALID_PARAMETER);

   BAST_CHK_RETCODE(BAST_4501_P_GetApStatus(h, &status));
   if ((status & BAST_APSTATUS_RUN_MASK) != BAST_APSTATUS_RUN)
      return BERR_TRACE(BERR_NOT_INITIALIZED); /* AP is not running */

   hab[0] = 0x03;
   hab[1] = addr; 
   hab[2] = page;
   hab[3] = n;
   BAST_CHK_RETCODE(BAST_4501_P_SendHabCommand(h, hab, (uint8_t)(5+n), hab, true));
   for (i = 0; i < n; i++)
      buf[i] = hab[4+i];
   
   done:
   return retCode; 
}


/******************************************************************************
 BAST_4501_P_ReadXdataPage()
******************************************************************************/
BERR_Code BAST_4501_P_ReadXdataPage(BAST_Handle h, uint8_t page, uint16_t offset, uint8_t *buf, uint8_t n)
{
   BERR_Code retCode;
   BAST_ApStatus status;
   uint8_t hab[128], i;
 
   if ((n == 0) || ((n+offset) > 255))
      return BERR_TRACE(BERR_INVALID_PARAMETER);

   BAST_CHK_RETCODE(BAST_4501_P_GetApStatus(h, &status));
   if ((status & BAST_APSTATUS_RUN_MASK) != BAST_APSTATUS_RUN)
      return BERR_TRACE(BERR_NOT_INITIALIZED); /* AP is not running */

   hab[0] = 0x1D;
   hab[1] = page;
   hab[2] = (uint8_t)offset;
   hab[3] = n;
   for (i = 0; i < n; i++)
      hab[4+i] = 0;

   retCode = BAST_4501_P_SendHabCommand(h, hab, (uint8_t)(5+n), hab, true);
   if (retCode == BERR_SUCCESS)
   {
      for (i = 0; i < n; i++)
         buf[i] = hab[4+i];
   }

   done:
   return retCode;
}


/******************************************************************************
 BAST_4501_P_WriteXdataPage()
******************************************************************************/
BERR_Code BAST_4501_P_WriteXdataPage(BAST_Handle h, uint8_t page, uint16_t offset, uint8_t *buf, uint8_t n)
{
   BERR_Code retCode;
   BAST_ApStatus status;
   uint8_t hab[128], i;
 
   if ((n == 0) || ((n+offset) > 255) || (buf == (uint8_t*)0))
      return BERR_TRACE(BERR_INVALID_PARAMETER);

   BAST_CHK_RETCODE(BAST_4501_P_GetApStatus(h, &status));
   if ((status & BAST_APSTATUS_RUN_MASK) != BAST_APSTATUS_RUN)
      return BERR_TRACE(BERR_NOT_INITIALIZED); /* AP is not running */

   hab[0] = 0x1E;
   hab[1] = page;
   hab[2] = (uint8_t)offset;
   hab[3] = n;
   for (i = 0; i < n; i++)
      hab[4+i] = buf[i];

   retCode = BAST_4501_P_SendHabCommand(h, hab, (uint8_t)(5+n), hab, true);

   done:
   return retCode;
}


/******************************************************************************
 BAST_4501_P_SetSearchRange()
******************************************************************************/
BERR_Code BAST_4501_P_SetSearchRange(BAST_Handle h, uint32_t searchRange)
{
   uint8_t buf[4];

   buf[0] = (uint8_t)((searchRange >> 24) & 0xFF);
   buf[1] = (uint8_t)((searchRange >> 16) & 0xFF);
   buf[2] = (uint8_t)((searchRange >> 8) & 0xFF);
   buf[3] = (uint8_t)(searchRange & 0xFF);
   return BAST_4501_P_WriteConfig(h->pChannels[0], BCM4501_CONFIG_SEARCH_RANGE, buf, BCM4501_CONFIG_LEN_SEARCH_RANGE);
}


/******************************************************************************
 BAST_4501_P_GetSearchRange()
******************************************************************************/
BERR_Code BAST_4501_P_GetSearchRange(BAST_Handle h, uint32_t *pSearchRange)
{
   BERR_Code retCode;
   uint8_t buf[4];

   retCode = BAST_4501_P_ReadConfig(h->pChannels[0], BCM4501_CONFIG_SEARCH_RANGE, buf, BCM4501_CONFIG_LEN_SEARCH_RANGE);
   if (retCode == BERR_SUCCESS)
      *pSearchRange = (uint32_t)((buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3]);
   else
      *pSearchRange = 0;
   return retCode;
}


/******************************************************************************
 BAST_4501_P_SetAmcScramblingSeq()
******************************************************************************/
BERR_Code BAST_4501_P_SetAmcScramblingSeq(BAST_ChannelHandle h, uint32_t xseed, uint32_t plhdrscr1, uint32_t plhdrscr2, uint32_t plhdrscr3)
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
   return BAST_4501_P_WriteConfig(h, BCM4501_CONFIG_LDPC_SCRAMBLING_SEQ, buf, BCM4501_CONFIG_LEN_LDPC_SCRAMBLING_SEQ);
}


/******************************************************************************
 BAST_4501_P_SetTunerFilter()
******************************************************************************/
BERR_Code BAST_4501_P_SetTunerFilter(BAST_ChannelHandle h, uint32_t cutoffHz)
{
   BERR_Code retCode;
   bool bAuto = false;
   uint8_t tuner_ctl, cutoffMhz = 0;

   if (cutoffHz == 0)
      bAuto = true;
   else
   {
      cutoffMhz = (uint8_t)((cutoffHz + 500000) / 1000000); /* round to nearest MHz */
      retCode = BAST_4501_P_WriteConfig(h, BCM4501_CONFIG_TUNER_CUTOFF, &cutoffMhz, BCM4501_CONFIG_LEN_TUNER_CUTOFF);
      if (retCode)
         goto done;
   }

   retCode = BAST_4501_P_ReadConfig(h, BCM4501_CONFIG_TUNER_FLAGS, &tuner_ctl, BCM4501_CONFIG_LEN_TUNER_FLAGS);
   if (retCode)
      goto done;
   if (bAuto)
      tuner_ctl &= ~0x08;
   else
      tuner_ctl |= 0x08;
   retCode = BAST_4501_P_WriteConfig(h, BCM4501_CONFIG_TUNER_FLAGS, &tuner_ctl, BCM4501_CONFIG_LEN_TUNER_FLAGS);
  
   done:
   return retCode;
}


/******************************************************************************
 BAST_4501_P_SetOutputTransportSettings()
******************************************************************************/
BERR_Code BAST_4501_P_SetOutputTransportSettings(BAST_ChannelHandle h, BAST_OutputTransportSettings *pSettings)
{
   uint16_t xportCtl = 0;
   uint8_t buf[2];
   
   if (pSettings->bSerial)
      xportCtl |= BCM4501_XPORT_CTL_SERIAL;      
   if (pSettings->bClkInv)
      xportCtl |= BCM4501_XPORT_CTL_CLKINV;      
   if (pSettings->bClkSup)
      xportCtl |= BCM4501_XPORT_CTL_CLKSUP;
   if (pSettings->bVldInv)
      xportCtl |= BCM4501_XPORT_CTL_VLDINV;
   if (pSettings->bSyncInv)
      xportCtl |= BCM4501_XPORT_CTL_SYNCINV;
   if (pSettings->bErrInv)
      xportCtl |= BCM4501_XPORT_CTL_ERRINV;
   if (pSettings->bXbert)
      xportCtl |= BCM4501_XPORT_CTL_XBERT;
   if (pSettings->bTei)
      xportCtl |= BCM4501_XPORT_CTL_TEI;
   if (pSettings->bDelay)
      xportCtl |= BCM4501_XPORT_CTL_DELAY;
   if (pSettings->bSync1)
      xportCtl |= BCM4501_XPORT_CTL_SYNC1;
   if (pSettings->bHead4)
      xportCtl |= BCM4501_XPORT_CTL_HEAD4;
   if (pSettings->bDelHeader)
      xportCtl |= BCM4501_XPORT_CTL_DELH;     
   if (pSettings->bchMpegErrorMode == BAST_BchMpegErrorMode_eBchOrCrc8)
      xportCtl |= 0;
   else if (pSettings->bchMpegErrorMode == BAST_BchMpegErrorMode_eCrc8)
      xportCtl |= BCM4501_XPORT_CTL_CRC8_CHECK;
   else if (pSettings->bchMpegErrorMode == BAST_BchMpegErrorMode_eBch)  
      xportCtl |= BCM4501_XPORT_CTL_BCH_CHECK;
   else 
      xportCtl |= (BCM4501_XPORT_CTL_CRC8_CHECK | BCM4501_XPORT_CTL_BCH_CHECK);

   buf[0] = (xportCtl >> 8) & 0xFF;
   buf[1] = (xportCtl & 0xFF);
   return BAST_4501_P_WriteConfig(h, BCM4501_CONFIG_XPORT_CTL, buf, BCM4501_CONFIG_LEN_XPORT_CTL);;
}


/******************************************************************************
 BAST_4501_P_GetOutputTransportSettings()
******************************************************************************/
BERR_Code BAST_4501_P_GetOutputTransportSettings(BAST_ChannelHandle h, BAST_OutputTransportSettings *pSettings)
{
   BERR_Code retCode;
   uint16_t xportCtl;
   uint8_t buf[2];

   retCode = BAST_4501_P_ReadConfig(h, BCM4501_CONFIG_XPORT_CTL, buf, BCM4501_CONFIG_LEN_XPORT_CTL);;
   if (retCode != BERR_SUCCESS)
      return retCode;

   xportCtl = (buf[0] << 8) | buf[1];

   if (xportCtl & BCM4501_XPORT_CTL_SERIAL)  
      pSettings->bSerial = true;
   else
      pSettings->bSerial = false;   
   if (xportCtl & BCM4501_XPORT_CTL_CLKINV) 
      pSettings->bClkInv = true; 
   else
      pSettings->bClkInv = false;
   if (xportCtl & BCM4501_XPORT_CTL_CLKSUP)
      pSettings->bClkSup = true;
   else
      pSettings->bClkSup = false;   
   if (xportCtl & BCM4501_XPORT_CTL_VLDINV)
      pSettings->bVldInv = true;
   else
      pSettings->bVldInv = false;   
   if (xportCtl & BCM4501_XPORT_CTL_SYNCINV)
      pSettings->bSyncInv = true;
   else
      pSettings->bSyncInv = false;   
   if (xportCtl & BCM4501_XPORT_CTL_ERRINV)
      pSettings->bErrInv = true;
   else
      pSettings->bErrInv = false;   
   if (xportCtl & BCM4501_XPORT_CTL_XBERT)
      pSettings->bXbert = true;
   else
      pSettings->bXbert = false;   
   if (xportCtl & BCM4501_XPORT_CTL_TEI)
      pSettings->bTei = true;
   else
      pSettings->bTei = false;   
   if (xportCtl & BCM4501_XPORT_CTL_DELAY)
      pSettings->bDelay = true;
   else
      pSettings->bDelay = false;   
   if (xportCtl & BCM4501_XPORT_CTL_SYNC1)
      pSettings->bSync1 = true;
   else
      pSettings->bSync1 = false;   
   if (xportCtl & BCM4501_XPORT_CTL_HEAD4)
      pSettings->bHead4 = true;
   else
      pSettings->bHead4 = false;   
   if (xportCtl & BCM4501_XPORT_CTL_DELH)
      pSettings->bDelHeader = true;    
   else
      pSettings->bDelHeader = false;   
   pSettings->bOpllBypass = false; 

   if ((xportCtl & BCM4501_XPORT_CTL_CRC8_CHECK) &&  ((xportCtl & BCM4501_XPORT_CTL_BCH_CHECK)==0))
      pSettings->bchMpegErrorMode = BAST_BchMpegErrorMode_eCrc8;
   else if ((xportCtl & BCM4501_XPORT_CTL_BCH_CHECK) &&  ((xportCtl & BCM4501_XPORT_CTL_CRC8_CHECK)==0))
      pSettings->bchMpegErrorMode = BAST_BchMpegErrorMode_eBch;
   else if ((xportCtl & BCM4501_XPORT_CTL_CRC8_CHECK) &&  (xportCtl & BCM4501_XPORT_CTL_BCH_CHECK))
      pSettings->bchMpegErrorMode = BAST_BchMpegErrorMode_eBchAndCrc8;
   else
      pSettings->bchMpegErrorMode = BAST_BchMpegErrorMode_eBchOrCrc8;
      
   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_4501_P_SetDiseqcSettings()
******************************************************************************/
BERR_Code BAST_4501_P_SetDiseqcSettings(BAST_ChannelHandle h, BAST_DiseqcSettings *pSettings)
{
   BERR_Code retCode;
   uint16_t diseqcCtl = 0;
   uint8_t buf[2], rrto[4];
   
   if (pSettings->bEnvelope)
      diseqcCtl |= BCM4501_DISEQC_CTL2_ENVELOPE;
   if (pSettings->bToneAlign)
      diseqcCtl |= BCM4501_DISEQC_CTL2_TONE_ALIGN;
   if (pSettings->bDisableRRTO)
      diseqcCtl |= BCM4501_DISEQC_CTL2_DISABLE_RRTO;
   if (pSettings->bEnableToneburst)
      diseqcCtl |= BCM4501_DISEQC_CTL2_TB_ENABLE;
   if (pSettings->bToneburstB)
      diseqcCtl |= BCM4501_DISEQC_CTL2_TB_B;
   if (pSettings->bOverrideFraming)
   {
      if (pSettings->bExpectReply)
         pSettings->bExpectReply = false; /* not implemented */
      else
         diseqcCtl |= BCM4501_DISEQC_CTL2_EXP_REPLY_DISABLE;
   }
   if (pSettings->bEnableLNBPU)
      pSettings->bEnableLNBPU = false;    /* not implemented */
   if (pSettings->bDisableRxOnly)
      pSettings->bDisableRxOnly = false;  /* not implemented */
   
   buf[0] = (diseqcCtl >> 8) & 0xFF;
   buf[1] = (diseqcCtl & 0xFF);
   BAST_CHK_RETCODE(BAST_4501_P_WriteConfig(h, BCM4501_CONFIG_DISEQC_CTL2, buf, BCM4501_CONFIG_LEN_DISEQC_CTL2));
   
   /* set rrto */
   rrto[0] = (uint8_t)(pSettings->rrtoUsec >> 24);
   rrto[1] = (uint8_t)(pSettings->rrtoUsec >> 16);
   rrto[2] = (uint8_t)(pSettings->rrtoUsec >> 8);
   rrto[3] = (uint8_t)(pSettings->rrtoUsec & 0xFF);
   BAST_CHK_RETCODE(BAST_4501_P_WriteConfig(h, BCM4501_CONFIG_RRTO_USEC, rrto, BCM4501_CONFIG_LEN_RRTO_USEC));
   
   /* set rx bit threshold */
   pSettings->bitThreshold = 0;  /* not implemented */
   
   /* set tone detect threshold */
   pSettings->toneThreshold = 0; /* not implemented */
   
   /* set pretx delay */
   BAST_CHK_RETCODE(BAST_4501_P_WriteConfig(h, BCM4501_CONFIG_DISEQC_PRETX_DELAY, &(pSettings->preTxDelay), BCM4501_CONFIG_LEN_DISEQC_PRETX_DELAY));
   
   /* set vsense thresholds */
   pSettings->vsenseThresholdHi = 0;   /* not implemented */
   pSettings->vsenseThresholdLo = 0;   /* not implemented */
   
   done:
   return retCode;
}


/******************************************************************************
 BAST_4501_P_GetDiseqcSettings()
******************************************************************************/
BERR_Code BAST_4501_P_GetDiseqcSettings(BAST_ChannelHandle h, BAST_DiseqcSettings *pSettings)
{
   BERR_Code retCode;
   uint8_t diseqcCtl;
   uint8_t buf[4];
   
   BAST_CHK_RETCODE(BAST_4501_P_ReadConfig(h, BCM4501_CONFIG_DISEQC_CTL2, &diseqcCtl, BCM4501_CONFIG_LEN_DISEQC_CTL2));
   if (diseqcCtl & BCM4501_DISEQC_CTL2_ENVELOPE)  
      pSettings->bEnvelope = true;
   else
      pSettings->bEnvelope = false;
   if (diseqcCtl & BCM4501_DISEQC_CTL2_TONE_ALIGN) 
      pSettings->bToneAlign = true; 
   else
      pSettings->bToneAlign = false;
   if (diseqcCtl & BCM4501_DISEQC_CTL2_DISABLE_RRTO)
      pSettings->bDisableRRTO = true;
   else
      pSettings->bDisableRRTO = false;
   if (diseqcCtl & BCM4501_DISEQC_CTL2_TB_ENABLE)
      pSettings->bEnableToneburst = true;
   else
      pSettings->bEnableToneburst = false;
   if (diseqcCtl & BCM4501_DISEQC_CTL2_TB_B)
      pSettings->bToneburstB = true;
   else
      pSettings->bToneburstB = false;
   if (diseqcCtl & BCM4501_DISEQC_CTL2_EXP_REPLY_DISABLE)
   {
      pSettings->bOverrideFraming = true;
      pSettings->bExpectReply = false;
   }
   else
   {
      pSettings->bOverrideFraming = false;
      pSettings->bExpectReply = false;
   }
   pSettings->bEnableLNBPU = false;    /* not implemented */
   pSettings->bDisableRxOnly = false;  /* not implemented */
   
   /* get rrto */
   BAST_CHK_RETCODE(BAST_4501_P_ReadConfig(h, BCM4501_CONFIG_RRTO_USEC, buf, BCM4501_CONFIG_LEN_RRTO_USEC));
   pSettings->rrtoUsec = (buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3];
   pSettings->rrtoUsec &= 0xFFFFF;
   
   /* get rx bit threshold */
   pSettings->bitThreshold = 0;  /* not implemented */
   
   /* get tone detect threshold */
   pSettings->toneThreshold = 0; /* not implemented */
   
   /* get pretx delay */
   BAST_CHK_RETCODE(BAST_4501_P_ReadConfig(h, BCM4501_CONFIG_DISEQC_PRETX_DELAY, &(pSettings->preTxDelay), BCM4501_CONFIG_LEN_DISEQC_PRETX_DELAY));
   
   /* get vsense thresholds */
   pSettings->vsenseThresholdHi = 0;   /* not implemented */
   pSettings->vsenseThresholdLo = 0;   /* not implemented */
   
   done:
   return retCode;
}


/******************************************************************************
 BAST_4501_P_SetNetworkSpec()
******************************************************************************/
BERR_Code BAST_4501_P_SetNetworkSpec(BAST_Handle h, BAST_NetworkSpec s)
{
   uint8_t sb;

   if (s > BAST_NetworkSpec_eEuro)
      return BERR_INVALID_PARAMETER;

   h->settings.networkSpec = s;
   sb = (uint8_t)s;
   return BAST_WriteConfig(h->pChannels[0], BCM4501_CONFIG_NETWORK_SPEC, &sb, BCM4501_CONFIG_LEN_NETWORK_SPEC);
}


/******************************************************************************
 BAST_4501_P_GetNetworkSpec()
******************************************************************************/
BERR_Code BAST_4501_P_GetNetworkSpec(BAST_Handle h, BAST_NetworkSpec *p)
{
   BERR_Code retCode;
   uint8_t sb;

   retCode = BAST_ReadConfig(h->pChannels[0], BCM4501_CONFIG_NETWORK_SPEC, &sb, BCM4501_CONFIG_LEN_NETWORK_SPEC);
   if (retCode == BERR_SUCCESS)
   {
      h->settings.networkSpec = (BAST_NetworkSpec)sb;
      *p = h->settings.networkSpec;
   }
   return retCode;
}
