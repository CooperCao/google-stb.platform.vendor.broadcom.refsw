 /***************************************************************************
 *     Copyright (c) 2003-2009, Broadcom Corporation
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
#include "bchp_3510.h"
#include "bvsb.h"
#include "bvsb_priv.h"

BDBG_MODULE(bvsb);


static const BVSB_Settings defDevSettings =
{
   (uint16_t)0x10,
   NULL, 
   NULL
};


/******************************************************************************
 BVSB_Open()
******************************************************************************/
BERR_Code BVSB_Open(
   BVSB_Handle *hVSB,                /* [output] VSB device handle */
   BCHP_Handle hChip,                /* chip handle */
   BREG_I2C_Handle hReg,             /* register handle */
   const BVSB_Settings *pDefSettings /* default settings */
)
{
   BERR_Code retCode = BERR_SUCCESS;
   BVSB_Handle hDev;

   BSTD_UNUSED(hChip);
   BDBG_ASSERT(hVSB);
   BDBG_ASSERT(hReg);

   /* allocate memory for the handle */
   hDev = (BVSB_Handle)BKNI_Malloc(sizeof(BVSB_P_Handle));
   BDBG_ASSERT(hDev);
   *hVSB = hDev;

   /* initialize our handle */
   hDev->hRegister = hReg;
   if (pDefSettings)
      BKNI_Memcpy((void*)(&(hDev->settings)), (void*)pDefSettings, sizeof(BVSB_Settings));
   else
      BKNI_Memcpy((void*)(&(hDev->settings)), (void*)&defDevSettings, sizeof(BVSB_Settings));
   BDBG_ASSERT(hDev->settings.interruptEnableFunc);
   
   /* create events */
   BVSB_CHK_RETCODE(BKNI_CreateEvent(&(hDev->hInterruptEvent)));
   BVSB_CHK_RETCODE(BKNI_CreateEvent(&(hDev->hApiEvent)));
   BVSB_CHK_RETCODE(BKNI_CreateEvent(&(hDev->hLockStateChangeEvent)));
   BVSB_CHK_RETCODE(BKNI_CreateEvent(&(hDev->hHabEvent)));
   BVSB_CHK_RETCODE(BKNI_CreateEvent(&(hDev->hInitEvent)));

   BVSB_CHK_RETCODE(BVSB_P_DisableAllInterrupts(hDev));

   hDev->bLocked = false;
   hDev->acqParams.mode = BVSB_ModulationType_eAnnexDVsb8;
   hDev->acqParams.symbolRate = 5381119;
   hDev->acqParams.ifFreq = 44000000;
   hDev->acqParams.symbolRateOffset = 0;
   hDev->acqParams.ifFreqOffset = 0;
   hDev->acqParams.ntscOffset = 0;


   done:
   return (retCode);
}


/******************************************************************************
 BVSB_Close()
******************************************************************************/
BERR_Code BVSB_Close(
   BVSB_Handle hVSB   /* VSB device handle */
)
{
   BERR_Code retCode;

   BDBG_ASSERT(hVSB);

   retCode = BVSB_P_DisableAllInterrupts(hVSB);
   BKNI_DestroyEvent(hVSB->hInterruptEvent);
   BKNI_DestroyEvent(hVSB->hApiEvent);
   BKNI_DestroyEvent(hVSB->hHabEvent);
   BKNI_DestroyEvent(hVSB->hLockStateChangeEvent);
   BKNI_DestroyEvent(hVSB->hInitEvent);
   BKNI_Free((void*)hVSB);

   return retCode;
}


/******************************************************************************
 BVSB_GetDefaultSettings()
******************************************************************************/
BERR_Code BVSB_GetDefaultSettings(
   BVSB_Settings *pDefSettings   /* [output] default settings */
)
{
   BDBG_ASSERT(pDefSettings);

   BKNI_Memcpy((void*)pDefSettings, (void*)&defDevSettings, sizeof(BVSB_Settings));
   return BERR_SUCCESS;
}


/******************************************************************************
 BVSB_GetApStatus()
******************************************************************************/
BERR_Code BVSB_GetApStatus(
   BVSB_Handle hVSB,        /* VSB device handle */
   BVSB_ApStatus *pStatus   /* [output] AP status */
)
{
   BERR_Code retCode = BERR_SUCCESS;
   uint8_t   sb;

   BDBG_ASSERT(hVSB);
   BDBG_ASSERT(pStatus);

   *pStatus = 0;
   BVSB_CHK_RETCODE(BVSB_ReadRegister(hVSB, BCM3510_APSTAT1, &sb));
   *pStatus = sb;
   BVSB_CHK_RETCODE(BVSB_ReadRegister(hVSB, BCM3510_APSTAT2, &sb));
   *pStatus |= (sb << 8);
   BVSB_CHK_RETCODE(BVSB_ReadRegister(hVSB, BCM3510_HABSTAT, &sb));
   *pStatus |= (sb << 16);

   done:
   return retCode;
}


/******************************************************************************
 BVSB_InitAp()
******************************************************************************/
BERR_Code BVSB_InitAp(
   BVSB_Handle hVSB,       /* VSB device handle */
   const uint8_t *pImage   /* BCM3510 AP microcode image */
)
{
   BERR_Code retCode = BERR_SUCCESS;
   uint16_t  length, addr;
#if 0   
   uint32_t  mdata_retry, max_retries;
   bool      bSetAddr, bReadHeader;
   uint16_t  i;
   uint8_t   madrh, madrl, mdata;
#endif
   uint8_t   sb;

   BDBG_ASSERT(hVSB);

   BDBG_MSG(("BVSB_InitAp(): enter\n"));
   
   /* reset all interrupts */
   BVSB_CHK_RETCODE(BVSB_P_DisableAllInterrupts(hVSB));

   /* reset the AP before downloading the microcode */
   BVSB_CHK_RETCODE(BVSB_P_ResetAp(hVSB));

   sb = 0;
   BVSB_CHK_RETCODE(BVSB_WriteRegister(hVSB, BCM3510_TSTCTL, &sb));
   sb = 0x07;
   BVSB_CHK_RETCODE(BVSB_WriteRegister(hVSB, BCM3510_JDEC, &sb));

#if 0
   max_retries = 5;
   mdata_retry = 0;
   bSetAddr = true;
   bReadHeader = true;
   length = addr = 0;
#endif
   
   hVSB->bLocked = false;
   BDBG_ASSERT(pImage);

   while (1)
   {
#if 0   
      if (bReadHeader)
      {
         length = (pImage[0] << 8) | pImage[1];
         if (length == 0)
            break;

         addr = (pImage[2] << 8) | pImage[3];
         pImage += 4;
         bReadHeader = false;
         bSetAddr = true;
      }

      if (bSetAddr)
      {
         madrh = (addr >> 8);
         madrl = (addr & 0xFF);

         for (i = 0; (bSetAddr && (i < max_retries)); i++)
         {
            if ((retCode = BVSB_WriteRegister(hVSB, BCM3510_MADRH, &madrh)) == BERR_SUCCESS)
            {
               if ((retCode = BVSB_WriteRegister(hVSB, BCM3510_MADRL, &madrl)) == BERR_SUCCESS)
               {
                  bSetAddr = false;
                  break;
               }
            }
         }
         if (bSetAddr == true)
            goto done;
      }

      mdata = *pImage;
      if ((retCode = BVSB_WriteRegister(hVSB, BCM3510_MDATA, &mdata)) == BERR_SUCCESS)
      {
         length--;
         pImage++;
         addr++;
         mdata_retry = 0;

         if (length == 0)
            bReadHeader = true;
      }
      else if (mdata_retry++ > max_retries)
         goto done;
      else
         bSetAddr = true;
#else
      /* this is the faster way to download the microcode */
      /* extract the header */
      length = (pImage[0] << 8) | pImage[1];
      if (length == 0)
         break;         
      addr = (pImage[2] << 8) | pImage[3];
      
      /* set the starting address */
      BVSB_CHK_RETCODE(BVSB_WriteRegister(hVSB, BCM3510_MADRH, &pImage[2]));
      BVSB_CHK_RETCODE(BVSB_WriteRegister(hVSB, BCM3510_MADRL, &pImage[3]));
      
      /* program the data in one transaction */
      BVSB_CHK_RETCODE(BREG_I2C_Write(hVSB->hRegister, hVSB->settings.chipAddr, BCM3510_MDATA, &pImage[4], length));
      
      pImage += (4 + length);
#endif         
   };

   /* enable the init done interrupt */
   BVSB_CHK_RETCODE(BVSB_P_EnableInitDoneInterrupt(hVSB));

   /* take the AP out of reset state */
   BKNI_WaitForEvent(hVSB->hInterruptEvent, 0);
   BKNI_WaitForEvent(hVSB->hApiEvent, 0);
   BKNI_WaitForEvent(hVSB->hInitEvent, 0); 
   BVSB_P_EnableHostInterrupt(hVSB, true);     
   BVSB_CHK_RETCODE(BVSB_P_RunAp(hVSB));

   /* wait for the AP to initialize */
   /*if (BKNI_WaitForEvent(hVSB->hInitEvent, 300) != BERR_SUCCESS) */
   if (BVSB_P_WaitForApiEvent(hVSB, hVSB->hInitEvent, 300) != BERR_SUCCESS)
   {
      BERR_TRACE(retCode = BVSB_ERR_AP_NOT_INIT);
   }

   done:
   BVSB_P_DisableAllInterrupts(hVSB);
   BDBG_MSG(("BVSB_InitAp(): exit\n"));

   return retCode;
}


/******************************************************************************
 BVSB_GetApVersion()
******************************************************************************/
BERR_Code BVSB_GetApVersion(
   BVSB_Handle hVSB,  /* VSB device handle */
   uint8_t *chipId,   /* [output] chip revision ID */
   uint8_t *apId,     /* [output] AP microcode revision ID */
   uint8_t *acqId,    /* [output] acquisition script version ID */
   uint8_t *cfgId     /* [output] configuration version ID */
)
{
   BERR_Code retCode = BERR_SUCCESS;
   uint8_t   hab[7];

   BDBG_ASSERT(hVSB);
   BDBG_ASSERT(chipId);
   BDBG_ASSERT(apId);
   BDBG_ASSERT(acqId);
   BDBG_ASSERT(cfgId);

   hab[0] = 0x3D;
   hab[1] = 0x15;
   hab[6] = 0x00;
   BVSB_CHK_RETCODE(BVSB_P_SendHabCommand(hVSB, 7, hab, 6, hab));

   *apId = hab[2];
   *acqId = hab[3];
   *cfgId = hab[4];
   *chipId = hab[5];

   done:
   return retCode;
}


/******************************************************************************
 BVSB_Acquire()
******************************************************************************/
BERR_Code BVSB_Acquire(
   BVSB_Handle hVSB,                 /* VSB device handle */
   const BVSB_AcqParams *pAcqParams  /* acquisition parameters */
)
{
   BERR_Code retCode = BERR_SUCCESS;
   uint8_t   hab[16], sym_if;
   uint32_t  offset;

   BDBG_ASSERT(hVSB);
   BDBG_ASSERT(pAcqParams);

   BVSB_CHK_RETCODE(BVSB_P_DisableAllInterrupts(hVSB));
   BVSB_P_EnableHostInterrupt(hVSB, true);

   if (pAcqParams->mode == BVSB_ModulationType_eAnnexDVsb8)
   {
      sym_if = 0x00; /* Auto selects 5381119 */
   }
   else if (pAcqParams->mode == BVSB_ModulationType_eAnnexBQam256)
   {
      sym_if = 0x10; /* Auto selects 5360537 */
   }
   else if (pAcqParams->mode == BVSB_ModulationType_eAnnexBQam64)
   {
      sym_if = 0x20; /* Auto selects 5056941 */
   }
   else
   {
      /* Annex A mode, so we let user input symbolrate */
      switch (pAcqParams->symbolRate)
      {
         case 5381119:
            sym_if = 0x00;
            break;
         case 5360537:
            sym_if = 0x10;
            break;
         case 5056941:
            sym_if = 0x20;
            break;
         default:
            sym_if = 0x70;
            if (pAcqParams->symbolRate != hVSB->acqParams.symbolRate)
            {
               /* program the symbol rate */
               hab[0] = 0x2D;
               hab[1] = 0x0C;
               hab[2] = (pAcqParams->symbolRate >> 24) & 0xFF;
               hab[3] = (pAcqParams->symbolRate >> 16) & 0xFF;
               hab[4] = (pAcqParams->symbolRate >> 8) & 0xFF;
               hab[5] = (pAcqParams->symbolRate & 0xFF);
               hab[6] = 0x00;
               BVSB_CHK_RETCODE(BVSB_P_SendHabCommand(hVSB, 7, hab, 1, hab));
            }
            break;
      }
   }

   switch (pAcqParams->ifFreq)
   {
      case 44000000:
         sym_if |= 0x00;
         break;
      case 43750000:
         sym_if |= 0x01;
         break;
      default:
         sym_if |= 0x07;
         if (pAcqParams->ifFreq != hVSB->acqParams.ifFreq)
         {
            /* program the symbol rate */
            hab[0] = 0x2D;
            hab[1] = 0x0D;
            hab[2] = (pAcqParams->ifFreq >> 24) & 0xFF;
            hab[3] = (pAcqParams->ifFreq >> 16) & 0xFF;
            hab[4] = (pAcqParams->ifFreq >> 8) & 0xFF;
            hab[5] = (pAcqParams->ifFreq & 0xFF);
            hab[6] = 0x00;
            BVSB_CHK_RETCODE(BVSB_P_SendHabCommand(hVSB, 7, hab, 1, hab));
         }
         break;
   }

   BKNI_Memcpy((void*)&(hVSB->acqParams), (const void *)pAcqParams, sizeof(BVSB_AcqParams));

   /* send the Acquire command */
   hab[0] = 0x38;
   hab[1] = 0x0A;
   hab[2] = 0xC0 | (((uint8_t)pAcqParams->mode & 0x0F) + 1);
   hab[3] = sym_if;
   offset = (hVSB->acqParams.ifFreqOffset / 10) & 0xFFFF;
   hab[4] = (offset >> 8) & 0xFF; 
   hab[5] = (offset & 0xFF);
   offset = (hVSB->acqParams.symbolRateOffset & 0xFFFF); 
   hab[6] = (offset >> 8) & 0xFF;
   hab[7] = (offset & 0xFF);
   offset = (hVSB->acqParams.ntscOffset / 10) & 0xFFFF;
   hab[8] = (offset >> 8) & 0xFF; 
   hab[9] = (offset & 0xFF);
   hab[10] = 0x00;
   BVSB_CHK_RETCODE(BVSB_P_SendHabCommand(hVSB, 11, hab, 1, hab));

   BVSB_ResetInbandStatus(hVSB);

   /* enable lock interrupt */
   BVSB_CHK_RETCODE(BVSB_P_EnableLockInterrupt(hVSB));

   done:
   return retCode;
}


/******************************************************************************
 BVSB_GetAcqParams()
******************************************************************************/
BERR_Code BVSB_GetAcqParams(
   BVSB_Handle hVSB,          /* VSB device handle */
   BVSB_AcqParams *pAcqParams /* [output] acquisition parameters */
)
{
   BDBG_ASSERT(hVSB);
   BDBG_ASSERT(pAcqParams);

   BKNI_Memcpy((void*)pAcqParams, &(hVSB->acqParams), sizeof(BVSB_AcqParams));
   return BERR_SUCCESS;
}


/******************************************************************************
 BVSB_GetStatus()
******************************************************************************/
BERR_Code BVSB_GetStatus(
   BVSB_Handle hVSB,      /* VSB device handle */
   BVSB_Status *pStatus   /* [output] channel status */
)
{
   BERR_Code retCode = BERR_SUCCESS;
   uint8_t   hab[128];

   BDBG_ASSERT(hVSB);

   hab[0] = 0x38;
   hab[1] = 0x08;
   hab[2] = 0x00;
   BVSB_CHK_RETCODE(BVSB_P_SendHabCommand(hVSB, 3, hab, 40, hab));

   pStatus->bRcvrLocked = hab[3] & 0x01 ? true : false;
   pStatus->bFecLocked = hab[3] & 0x02 ? true : false;
   pStatus->bOPllLocked = hab[3] & 0x04 ? true : false;
   pStatus->snrEstimate = ((hab[6] << 8) | hab[7]);
;
   pStatus->symbolRate = (hab[12] << 24) | (hab[13] << 16) | (hab[14] << 8) | hab[15];
      
   pStatus->symbolRateOffset = (int32_t)((hab[16] << 8) | hab[17]);
   if (pStatus->symbolRateOffset >= 0x8000)
      pStatus->symbolRateOffset -= 0x10000;
      
   pStatus->symbolRateError = (int32_t)((hab[18] << 8) | hab[19]); 
   if (pStatus->symbolRateError >= 0x8000)
      pStatus->symbolRateError -= 0x10000;
      
   pStatus->ifFreq = (hab[20] << 24) | (hab[21] << 16) | (hab[22] << 8) | hab[23];
   
   pStatus->ifFreqOffset = (int32_t)((hab[24] << 8) | hab[25]);
   if (pStatus->ifFreqOffset >= 0x8000)
      pStatus->ifFreqOffset -= 0x10000;
   pStatus->ifFreqOffset *= 10;
   
   pStatus->ifFreqError = (int32_t)((hab[26] << 8) | hab[27]);
   if (pStatus->ifFreqError >= 0x8000)
      pStatus->ifFreqError -= 0x10000;
   pStatus->ifFreqError *= 10;
   
   pStatus->ntscFilter = (hab[28] << 24) | (hab[29] << 16) | (hab[30] << 8) | hab[31];
   
   pStatus->ntscOffset = (int32_t)((hab[32] << 8) | hab[33]);
   if (pStatus->ntscOffset >= 0x8000)
      pStatus->ntscOffset -= 0x10000;
   pStatus->ntscOffset *= 10;
   
   pStatus->ntscError = (int32_t)((hab[34] << 8) | hab[35]);
   if (pStatus->ntscError >= 0x8000)
      pStatus->ntscError -= 0x10000;
   pStatus->ntscError *= 10;
   
   pStatus->intAgcLevel = ((hab[36] << 8) | hab[37]) * 1000 / 65535;
   pStatus->extAgcLevel = ((hab[38] << 8) | hab[39]) * 1000 / 65535;

   switch (hab[2] & 0x0F)
   {
      case 0x01: /* 16 QAM */
         pStatus->mode = BVSB_ModulationType_eAnnexAQam16;
         break;

      case 0x02: /* 32 QAM */
         pStatus->mode = BVSB_ModulationType_eAnnexAQam32;
         break;

      case 0x04: /* 128 QAM */
         pStatus->mode = BVSB_ModulationType_eAnnexAQam128;
         break;

      case 0x0B: /* 8 VSB */
         pStatus->mode = BVSB_ModulationType_eAnnexDVsb8;
         break;

      case 0x0D: /* 16 VSB */
         pStatus->mode = BVSB_ModulationType_eAnnexDVsb16;
         break;

      default: /* 64/256 QAM */
         hab[0] = 0x4C;
         hab[1] = 0x00;
         hab[2] = 0x38;
         hab[3] = 0x1E;
         hab[5] = 0x00;
         BVSB_CHK_RETCODE(BVSB_P_SendHabCommand(hVSB, 6, hab, 6, hab));
         hab[4] &= 0x0F;
         if (hab[4] == 0x01)
            pStatus->mode = BVSB_ModulationType_eAnnexBQam256;
         else if (hab[4] == 0x02)
            pStatus->mode = BVSB_ModulationType_eAnnexBQam64;
         else if (hab[4] == 0x03)
            pStatus->mode = BVSB_ModulationType_eAnnexAQam256;
         else if (hab[4] == 0x05)
            pStatus->mode = BVSB_ModulationType_eAnnexAQam64;
         else
         {
            /*BDBG_ERR(("BVSB_GetStatus(): unable to determine modulation type\n"));*/
           pStatus->mode = hVSB->acqParams.mode;
         }
         break;
   }

   hab[0] = 0x38;
   hab[1] = 0x14;
   hab[2] = 0x00;
   BVSB_CHK_RETCODE(BVSB_P_SendHabCommand(hVSB, 3, hab, 16, hab));

   hVSB->corrErrCount += ((hab[8] << 8) | hab[9]);
   hVSB->ucorrBlockCount += ((hab[12] << 8) | hab[13]);
   pStatus->corrErrCount = hVSB->corrErrCount;
   pStatus->ucorrBlockCount = hVSB->ucorrBlockCount;
   BDBG_MSG(("Corrected bits, Uncorrected blocks = %d,%d\n", hVSB->corrErrCount, hVSB->ucorrBlockCount));

   done:
   return retCode;
}

/******************************************************************************
 BVSB_ResetInbandStatus()
******************************************************************************/
BERR_Code BVSB_ResetInbandStatus(
   BVSB_Handle hVSB    /* [in] VSB device handle */
   )
{
    hVSB->corrErrCount = hVSB->ucorrBlockCount = 0;
    return BERR_SUCCESS;
}

/******************************************************************************
 BVSB_GetLockStatus()
******************************************************************************/
BERR_Code BVSB_GetLockStatus(
   BVSB_Handle hVSB,  /* VSB device handle */
   bool *pLocked      /* [output] true if FEC is locked */
)
{
   BERR_Code retCode = BERR_SUCCESS;
   uint8_t   sb;

   BDBG_ASSERT(hVSB);

   BVSB_CHK_RETCODE(BVSB_ReadRegister(hVSB, BCM3510_APSTAT2, &sb));
   *pLocked = (sb & BCM3510_APSTAT2_LOCK) ? true : false;

   done:
   return retCode;
}


/******************************************************************************
 BVSB_GetSoftDecision()
******************************************************************************/
BERR_Code BVSB_GetSoftDecision(
   BVSB_Handle hVSB,    /* VSB device handle */
   int8_t *iVal,        /* [output] I value */
   int8_t *qVal         /* [output] Q value */
)
{
   BERR_Code retCode = BERR_SUCCESS;
   uint8_t   hab[5];

   BDBG_ASSERT(hVSB);

   hab[0] = 0x73;
   hab[1] = 0x48;
   hab[4] = 0x00;
   BVSB_CHK_RETCODE(BVSB_P_SendHabCommand(hVSB, 5, hab, 5, hab));

   *iVal = (int8_t)hab[2];
   *qVal = (int8_t)hab[3];

   done:
   return retCode;
}


/******************************************************************************
 BVSB_GetSoftDecisionBuf()
******************************************************************************/
BERR_Code BVSB_GetSoftDecisionBuf(
   BVSB_Handle hVSB,    /* VSB device handle */
   int8_t *iVal,        /* [output] set of 31 I values */
   int8_t *qVal         /* [output] set of 31 Q values */
)
{
   BERR_Code retCode = BERR_SUCCESS;
   int       i;
   uint8_t   hab[128], n;

   BDBG_ASSERT(hVSB);

   for (n = 0; n < 124; n += 4)
   {
      hab[n + 0] = 0x73;
      hab[n + 1] = 0x48;
   }
   hab[n] = 0;      
   BVSB_CHK_RETCODE(BVSB_P_SendHabCommand(hVSB, n+1, hab, n, hab));

   for (n = i = 0; n < 124; n += 4, i++)
   {
      iVal[i] = (int8_t)hab[n + 2];
      qVal[i] = (int8_t)hab[n + 3];
   }

   done:
   return retCode;
}


/******************************************************************************
 BVSB_WriteI2c()
******************************************************************************/
BERR_Code BVSB_WriteI2c(
   BVSB_Handle hVSB,   /* VSB device handle */
   const uint8_t *buf,
   uint8_t n
)
{
   BERR_Code retCode = BERR_SUCCESS;
   uint8_t   i, hab[128];

   BDBG_ASSERT(hVSB);
   BDBG_ASSERT(buf);

   if ((n < 1) || (n > 31))
   {
      BERR_TRACE(retCode = BERR_INVALID_PARAMETER);
      goto done;
   }

   hab[0] = 0x38;
   hab[1] = 0x16;
   hab[2] = 0x80 | n;
   hab[3] = 0x00;
   hab[4] = 0x03;
   hab[5] = 0x40;
   for (i = 0; i < n; i++)
   {
      hab[6+(i*2)] = 0x07;
      hab[7+(i*2)] = buf[i];
   }
   hab[6+(n*2)] = 0x00;

   BVSB_CHK_RETCODE(BVSB_P_SendHabCommand(hVSB, 7+(n*2), hab, 1, hab));

   done:
   return retCode;
}


/******************************************************************************
 BVSB_GetLockStateChangeEventHandle()
******************************************************************************/
BERR_Code BVSB_GetLockStateChangeEventHandle(
   BVSB_Handle hVSB,           /* VSB device handle */
   BKNI_EventHandle *phEvent   /* [output] event handle */
)
{
   BDBG_ASSERT(hVSB);

   *phEvent = hVSB->hLockStateChangeEvent;
   return BERR_SUCCESS;
}


/******************************************************************************
 BVSB_GetInterruptEventHandle()
******************************************************************************/
BERR_Code BVSB_GetInterruptEventHandle(
   BVSB_Handle hVSB,           /* VSB device handle */
   BKNI_EventHandle *phEvent   /* [output] event handle */
)
{
   BDBG_ASSERT(hVSB);

   *phEvent = hVSB->hInterruptEvent;
   return BERR_SUCCESS;
}


/******************************************************************************
 BVSB_ProcessInterruptEvent()
******************************************************************************/
BERR_Code BVSB_ProcessInterruptEvent(
   BVSB_Handle hVSB,
   BKNI_EventHandle *phEvent
)
{
   BERR_Code     retCode = BERR_SUCCESS;

   BSTD_UNUSED(phEvent);
   BDBG_ASSERT(hVSB);

   BVSB_CHK_RETCODE(BVSB_P_DecodeInterrupt(hVSB));
   BVSB_P_EnableHostInterrupt(hVSB, true);

   done:
   return retCode;
}


/******************************************************************************
 BVSB_HandleInterrupt_isr()
******************************************************************************/
void BVSB_HandleInterrupt_isr(
   BVSB_Handle hVSB          /* VSB device handle */
)
{
   BDBG_ASSERT(hVSB);
   hVSB->settings.interruptEnableFunc(false, hVSB->settings.interruptEnableFuncParam);   
   BKNI_SetEvent(hVSB->hApiEvent);
   BKNI_SetEvent(hVSB->hInterruptEvent);
}

