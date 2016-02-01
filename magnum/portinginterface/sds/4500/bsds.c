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
#include "bchp_4500.h"
#include "bsds.h"
#include "bsds_priv.h"

BDBG_MODULE(bsds);


static const BSDS_Settings defDevSettings =
{
   (uint16_t)0x20,
   NULL, 
   NULL
};


/******************************************************************************
 BSDS_Open()
******************************************************************************/
BERR_Code BSDS_Open(
   BSDS_Handle *hSDS,                /* [output] SDS device handle */
   BCHP_Handle hChip,                /* chip handle */
   BREG_I2C_Handle hReg,             /* register handle */
   const BSDS_Settings *pDefSettings /* default settings */
)
{
   BERR_Code retCode = BERR_SUCCESS;
   BSDS_Handle hDev;

   BSTD_UNUSED(hChip);
   BDBG_ASSERT(hSDS);
   BDBG_ASSERT(hReg);

   /* allocate memory for the handle */
   hDev = (BSDS_Handle)BKNI_Malloc(sizeof(BSDS_P_Handle));
   BDBG_ASSERT(hDev);

   /* initialize our handle */
   hDev->hRegister = hReg;
   if (pDefSettings)
      BKNI_Memcpy((void*)(&(hDev->settings)), (void*)pDefSettings, sizeof(BSDS_Settings));
   else
      BKNI_Memcpy((void*)(&(hDev->settings)), (void*)&defDevSettings, sizeof(BSDS_Settings));
   BDBG_ASSERT(hDev->settings.interruptEnableFunc);
   
   /* create events */
   retCode = BKNI_CreateEvent(&(hDev->hInterruptEvent));
   if (retCode != BERR_SUCCESS)
      goto done_0;
   retCode = BKNI_CreateEvent(&(hDev->hApiEvent));
   if (retCode != BERR_SUCCESS)
      goto done_1;
   retCode = BKNI_CreateEvent(&(hDev->hLockStateChangeEvent));
   if (retCode != BERR_SUCCESS)
      goto done_2;
   retCode = BKNI_CreateEvent(&(hDev->hHabEvent));
   if (retCode != BERR_SUCCESS)
      goto done_3;
   retCode = BKNI_CreateEvent(&(hDev->hDiseqcEvent));
   if (retCode != BERR_SUCCESS)
      goto done_4;
   retCode = BKNI_CreateEvent(&(hDev->hInitEvent));
   if (retCode != BERR_SUCCESS)
   {
      done_5:
      BKNI_DestroyEvent(hDev->hDiseqcEvent);
      done_4:
      BKNI_DestroyEvent(hDev->hHabEvent);
      done_3:
      BKNI_DestroyEvent(hDev->hLockStateChangeEvent);
      done_2:
      BKNI_DestroyEvent(hDev->hApiEvent);
      done_1:
      BKNI_DestroyEvent(hDev->hInterruptEvent);
      done_0:
      BKNI_Free((void*)hDev);

      *hSDS = NULL;
      return retCode;
   }

   retCode = BSDS_P_DisableAllInterrupts(hDev);
   if (retCode != BERR_SUCCESS)
   {
      BKNI_DestroyEvent(hDev->hInitEvent);
      goto done_5;
   }

   hDev->bLocked = false;
   hDev->acqParams.mode = BSDS_ModulationType_eDvbScan;
   hDev->acqParams.symbolRate = 4000000;
   hDev->acqParams.carrierOffset = 0;
   hDev->acqParams.symbolRateOffset = 0;
   hDev->acqParams.acqControl = (BSDS_MPEG | BSDS_PARALLEL | BSDS_CLKINV | BSDS_TEI);
   hDev->turboIdx9 = BSDS_ModulationType_eTurboQpsk7_8;

   *hSDS = hDev;

   return (retCode);
}


/******************************************************************************
 BSDS_Close()
******************************************************************************/
BERR_Code BSDS_Close(
   BSDS_Handle hSDS   /* SDS device handle */
)
{
   BERR_Code retCode = BERR_SUCCESS;

   BDBG_ASSERT(hSDS);

   retCode = BSDS_P_DisableAllInterrupts(hSDS);
   BKNI_DestroyEvent(hSDS->hInterruptEvent);
   BKNI_DestroyEvent(hSDS->hApiEvent);
   BKNI_DestroyEvent(hSDS->hHabEvent);
   BKNI_DestroyEvent(hSDS->hLockStateChangeEvent);
   BKNI_DestroyEvent(hSDS->hDiseqcEvent);
   BKNI_DestroyEvent(hSDS->hInitEvent);
   BKNI_Free((void*)hSDS);
   return retCode;
}


/******************************************************************************
 BSDS_GetDefaultSettings()
******************************************************************************/
BERR_Code BSDS_GetDefaultSettings(
   BSDS_Settings *pDefSettings   /* [output] default settings */
)
{
   BERR_Code retCode = BERR_SUCCESS;

   BDBG_ASSERT(pDefSettings);

   BKNI_Memcpy((void*)pDefSettings, (void*)&defDevSettings, sizeof(BSDS_Settings));

   return retCode;
}


/******************************************************************************
 BSDS_GetApStatus()
******************************************************************************/
BERR_Code BSDS_GetApStatus(
   BSDS_Handle hSDS,        /* SDS device handle */
   BSDS_ApStatus *pStatus   /* [output] AP status */
)
{
   BERR_Code retCode = BERR_SUCCESS;
   uint8_t   sb;

   BDBG_ASSERT(hSDS);
   BDBG_ASSERT(pStatus);

   *pStatus = 0;
   BSDS_CHK_RETCODE(BSDS_ReadRegister(hSDS, BCM4500_APSTAT1, &sb));
   *pStatus = sb;
   BSDS_CHK_RETCODE(BSDS_ReadRegister(hSDS, BCM4500_APSTAT2, &sb));
   *pStatus |= (sb << 8); 
   BSDS_CHK_RETCODE(BSDS_ReadRegister(hSDS, BCM4500_HABSTAT, &sb));
   *pStatus |= (sb << 16); 
  
   done:
   return retCode;
}


/******************************************************************************
 BSDS_InitAp()
******************************************************************************/
BERR_Code BSDS_InitAp(
   BSDS_Handle hSDS,       /* SDS device handle */
   const uint8_t *pImage   /* BCM4500 AP microcode image */
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
   
   BDBG_ASSERT(hSDS);

   /* reset interrupts */
   BSDS_CHK_RETCODE(BSDS_P_DisableAllInterrupts(hSDS));
   BSDS_P_EnableHostInterrupt(hSDS, false);

   /* reset the AP before downloading the microcode */
   BSDS_CHK_RETCODE(BSDS_P_ResetAp(hSDS));

   BDBG_MSG(("BSDS_InitAp(): downloading microcode"));

#if 0
   max_retries = 5;
   mdata_retry = 0;
   bSetAddr = true;
   bReadHeader = true;
   length = addr = 0;   
#endif
   
   hSDS->bLocked = false;
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
            if ((retCode = BSDS_WriteRegister(hSDS, BCM4500_MADRH, &madrh)) == BERR_SUCCESS)
            {
               if ((retCode = BSDS_WriteRegister(hSDS, BCM4500_MADRL, &madrl)) == BERR_SUCCESS)
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
      if ((retCode = BSDS_WriteRegister(hSDS, BCM4500_MDATA, &mdata)) == BERR_SUCCESS)
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
      BSDS_CHK_RETCODE(BSDS_WriteRegister(hSDS, BCM4500_MADRH, &pImage[2]));
      BSDS_CHK_RETCODE(BSDS_WriteRegister(hSDS, BCM4500_MADRL, &pImage[3]));
      
      /* program the data in one transaction */
      BSDS_CHK_RETCODE(BREG_I2C_Write(hSDS->hRegister, hSDS->settings.chipAddr, BCM4500_MDATA, &pImage[4], length));
      
      pImage += (4 + length);
#endif         
   };

   BDBG_MSG(("BSDS_InitAp(): running microcode"));

   /* enable the init done interrupt */
   BSDS_CHK_RETCODE(BSDS_P_EnableInitDoneInterrupt(hSDS));

   /* take the AP out of reset state */
   BKNI_WaitForEvent(hSDS->hInterruptEvent, 0);
   BKNI_WaitForEvent(hSDS->hApiEvent, 0);
   BKNI_WaitForEvent(hSDS->hInitEvent, 0);   
   BSDS_P_EnableHostInterrupt(hSDS, true);   
   BSDS_CHK_RETCODE(BSDS_P_RunAp(hSDS));

   /* wait for the AP to initialize */
   BDBG_MSG(("BSDS_InitAp(): waiting for AP to initialize"));
   if (BSDS_P_WaitForApiEvent(hSDS, hSDS->hInitEvent, 600) != BERR_SUCCESS)
   {
      BERR_TRACE(retCode = BSDS_ERR_AP_NOT_INIT);
   }
   
   done:
   BSDS_P_DisableAllInterrupts(hSDS);
   BSDS_P_EnableHostInterrupt(hSDS, true);      
   return retCode;
}


/******************************************************************************
 BSDS_GetApVersion()
******************************************************************************/
BERR_Code BSDS_GetApVersion(
   BSDS_Handle hSDS,  /* SDS device handle */
   uint8_t *chipId,   /* [output] chip revision ID */
   uint8_t *apId,     /* [output] AP microcode revision ID */
   uint8_t *acqId,    /* [output] acquisition script version ID */
   uint8_t *cfgId     /* [output] configuration version ID */
)
{
   BERR_Code retCode = BERR_SUCCESS;
   uint8_t   hab[7];

   BDBG_ASSERT(hSDS);
   BDBG_ASSERT(chipId);
   BDBG_ASSERT(apId);
   BDBG_ASSERT(acqId);
   BDBG_ASSERT(cfgId);

   hab[0] = 0x05;
   hab[1] = 0x12;
   hab[6] = 0x00;
   BSDS_CHK_RETCODE(BSDS_P_SendHabCommand(hSDS, 7, hab, 6, hab));

   *chipId = hab[2];
   *apId = hab[3];
   *acqId = hab[4];
   *cfgId = hab[5];

   done:
   return retCode;
}


/******************************************************************************
 BSDS_Tune()
******************************************************************************/
BERR_Code BSDS_Tune(
   BSDS_Handle hSDS,       /* SDS device handle */
   const uint32_t freqHz   /* tuner frequency in Hz */
)
{
   BERR_Code retCode = BERR_SUCCESS;
   uint8_t   hab[8];

   BDBG_ASSERT(hSDS);

   hab[0] = 0x06;
   hab[1] = 0x0B;
   hab[2] = 0x17;
   hab[3] = (uint8_t)((freqHz >> 24) & 0xFF);
   hab[4] = (uint8_t)((freqHz >> 16) & 0xFF);
   hab[5] = (uint8_t)((freqHz >> 8) & 0xFF);
   hab[6] = (uint8_t)(freqHz & 0xFF);
   hab[7] = 0x00;
   BSDS_CHK_RETCODE(BSDS_P_SendHabCommand(hSDS, 8, hab, 1, hab));

   done:
   return retCode;
}


/******************************************************************************
 BSDS_TuneTransponder()
******************************************************************************/
BERR_Code BSDS_TuneTransponder(
   BSDS_Handle hSDS,            /* SDS device handle */
   const uint8_t transponder    /* transponder number from 1 to 31 */
)
{
   BERR_Code retCode = BERR_SUCCESS;
   uint8_t   hab[5];

   BDBG_ASSERT(hSDS);

   hab[0] = 0x03;
   hab[1] = 0x0B;
   hab[2] = 0x1C;
   hab[3] = transponder & 0x1F;
   hab[4] = 0x00;
   BSDS_CHK_RETCODE(BSDS_P_SendHabCommand(hSDS, 5, hab, 1, hab));

   done:
   return retCode;
}


/******************************************************************************
 BSDS_Acquire()
******************************************************************************/
BERR_Code BSDS_Acquire(
   BSDS_Handle hSDS,                 /* SDS device handle */
   const BSDS_AcqParams *pAcqParams  /* acquisition parameters */
)
{
   BERR_Code retCode = BERR_SUCCESS;
   uint8_t   hab[128];

   BDBG_ASSERT(hSDS);

   BSDS_CHK_RETCODE(BSDS_P_DisableAllInterrupts(hSDS));
   BSDS_P_EnableHostInterrupt(hSDS, true);

   if (pAcqParams)
   {
      if (hSDS->acqParams.symbolRate != pAcqParams->symbolRate)
      {         
         /* program the symbol rate */
         hab[0] = 0x07;
         hab[1] = 0x09;
         hab[2] = 0x39;
         hab[3] = 0x67;
         hab[4] = (uint8_t)((pAcqParams->symbolRate >> 24) & 0xFF);
         hab[5] = (uint8_t)((pAcqParams->symbolRate >> 16) & 0xFF);
         hab[6] = (uint8_t)((pAcqParams->symbolRate >> 8) & 0xFF);
         hab[7] = (uint8_t)(pAcqParams->symbolRate & 0xFF);
         hab[8] = 0x00;
         BSDS_CHK_RETCODE(BSDS_P_SendHabCommand(hSDS, 9, hab, 1, hab));
      }

      if (((pAcqParams->mode == BSDS_ModulationType_eTurboQpsk7_8) || (pAcqParams->mode == BSDS_ModulationType_eTurbo8psk3_4_2_20)) &&
          (pAcqParams->mode != hSDS->turboIdx9))
      {
         BSDS_CHK_RETCODE(BSDS_P_LoadTurboCodeRate9(hSDS, pAcqParams->mode));
      }
      
      BKNI_Memcpy((void*)&(hSDS->acqParams), (const void *)pAcqParams, sizeof(BSDS_AcqParams));
   }

   /* send the Acquire command */
   hab[0] = 0x03;
   hab[1] = 0x0F;
   hab[2] = hSDS->acqParams.acqControl & BSDS_MPEG ? 0x04 : 0x00;
   hab[2] |= (hSDS->acqParams.acqControl & BSDS_PARALLEL ? 0x08 : 0x00);
   hab[3] = 0x0F;
   hab[4] = (hSDS->acqParams.symbolRateOffset >> 24) & 0xFF; 
   hab[5] = (hSDS->acqParams.symbolRateOffset >> 16) & 0xFF; 
   hab[6] = (hSDS->acqParams.symbolRateOffset >> 8) & 0xFF; 
   hab[7] = (hSDS->acqParams.symbolRateOffset & 0xFF); 
   hab[8] = (hSDS->acqParams.carrierOffset >> 24) & 0xFF; 
   hab[9] = (hSDS->acqParams.carrierOffset >> 16) & 0xFF; 
   hab[10] = (hSDS->acqParams.carrierOffset >> 8) & 0xFF; 
   hab[11] = (hSDS->acqParams.carrierOffset & 0xFF); 
   hab[12] = (hSDS->acqParams.acqControl & BSDS_ERRINV ? 0x80 : 0x00);
   hab[12] |= (hSDS->acqParams.acqControl & BSDS_SYNCINV ? 0x40 : 0x00);
   hab[12] |= (hSDS->acqParams.acqControl & BSDS_VLDINV ? 0x20 : 0x00);
   hab[12] |= (hSDS->acqParams.acqControl & BSDS_CLKSUP ? 0x10 : 0x00);
   hab[12] |= (hSDS->acqParams.acqControl & BSDS_CLKINV ? 0x08 : 0x00);
   hab[13] = 0x01;
   hab[13] |= (hSDS->acqParams.acqControl & BSDS_DELH ? 0x80 : 0x00);
   hab[13] |= (hSDS->acqParams.acqControl & BSDS_HEAD4 ? 0x40 : 0x00);
   hab[13] |= (hSDS->acqParams.acqControl & BSDS_SYNC1 ? 0x20 : 0x00);
   hab[13] |= (hSDS->acqParams.acqControl & BSDS_ENABLE_BERT ? 0x10 : 0x00);
   hab[13] |= (hSDS->acqParams.acqControl & BSDS_DELAY ? 0x04 : 0x00);
   hab[14] = 0xBB;
   hab[14] |= (hSDS->acqParams.acqControl & BSDS_TEI ? 0x40 : 0x00);
   hab[14] |= (hSDS->acqParams.acqControl & BSDS_XBERT ? 0x04 : 0x00);
   hab[15] = 0x00;

   switch (hSDS->acqParams.mode)
   {
      case BSDS_ModulationType_eDvbScan:
         break;
      case BSDS_ModulationType_eDvb1_2:
         hab[2] |= 0x10; 
         break;
      case BSDS_ModulationType_eDvb2_3:
         hab[2] |= 0x20; 
         break;
      case BSDS_ModulationType_eDvb3_4:
         hab[2] |= 0x30; 
         break;
      case BSDS_ModulationType_eDvb5_6:
         hab[2] |= 0x40; 
         break;
      case BSDS_ModulationType_eDvb6_7:
         hab[2] |= 0x50; 
         break;
      case BSDS_ModulationType_eDvb7_8:
         hab[2] |= 0x60; 
         break;
      case BSDS_ModulationType_eDssScan:
         hab[2] |= 0x80; 
         break;
      case BSDS_ModulationType_eDss1_2:
         hab[2] |= 0x90; 
         break;
      case BSDS_ModulationType_eDss2_3:
         hab[2] |= 0xA0; 
         break;
      case BSDS_ModulationType_eDss3_4:
         hab[2] |= 0xB0; 
         break;
      case BSDS_ModulationType_eDss5_6:
         hab[2] |= 0xC0; 
         break;
      case BSDS_ModulationType_eDss6_7:
         hab[2] |= 0xD0; 
         break;
      case BSDS_ModulationType_eDss7_8:
         hab[2] |= 0xE0; 
         break;
      case BSDS_ModulationType_eDCIIScan:
         hab[2] |= 0xF0; 
         break;
      case BSDS_ModulationType_eDCII5_11:
         hab[2] |= 0xF0; 
         hab[3] |= 0x10;
         break;
      case BSDS_ModulationType_eDCII1_2:
         hab[2] |= 0xF0; 
         hab[3] |= 0x20;
         break;
      case BSDS_ModulationType_eDCII3_5:
         hab[2] |= 0xF0; 
         hab[3] |= 0x30;
         break;
      case BSDS_ModulationType_eDCII2_3:
         hab[2] |= 0xF0; 
         hab[3] |= 0x40;
         break;
      case BSDS_ModulationType_eDCII3_4:
         hab[2] |= 0xF0; 
         hab[3] |= 0x50;
         break;
      case BSDS_ModulationType_eDCII4_5:
         hab[2] |= 0xF0; 
         hab[3] |= 0x60;
         break;
      case BSDS_ModulationType_eDCII5_6:
         hab[2] |= 0xF0; 
         hab[3] |= 0x70;
         break;
      case BSDS_ModulationType_eDCII7_8:
         hab[2] |= 0xF0; 
         hab[3] |= 0x80;
         break;
      case BSDS_ModulationType_eTurboScan:
         hab[2] |= 0xF1;
         break;
      case BSDS_ModulationType_eTurboQpsk1_2:
         hab[2] |= 0x01;
         break;
      case BSDS_ModulationType_eTurboQpsk2_3:
         hab[2] |= 0x61;
         break;
      case BSDS_ModulationType_eTurboQpsk3_4:
         hab[2] |= 0x11;
         break;
      case BSDS_ModulationType_eTurboQpsk5_6:
         hab[2] |= 0x81;
         break;
      case BSDS_ModulationType_eTurboQpsk7_8:
         hab[2] |= 0x91;
         break;
      case BSDS_ModulationType_eTurbo8psk2_3:
         hab[2] |= 0x21;
         break;
      case BSDS_ModulationType_eTurbo8psk5_6:
         hab[2] |= 0x31;
         break;
      case BSDS_ModulationType_eTurbo8psk8_9:
         hab[2] |= 0x41;
         break;
      case BSDS_ModulationType_eTurbo8psk3_4_2_05:
         hab[2] |= 0x51;
         break;
      case BSDS_ModulationType_eTurbo8psk3_4_2_10:
         hab[2] |= 0x71;
         break;
      default:
         BERR_TRACE(retCode = BERR_INVALID_PARAMETER);
         goto done;
   }

   BSDS_CHK_RETCODE(BSDS_P_SendHabCommand(hSDS, 16, hab, 1, hab));

   /* enable lock interrupt */
   BSDS_CHK_RETCODE(BSDS_P_EnableLockInterrupt(hSDS));

   done:
   return retCode;
}


/******************************************************************************
 BSDS_GetAcqParams()
******************************************************************************/
BERR_Code BSDS_GetAcqParams(
   BSDS_Handle hSDS,          /* SDS device handle */
   BSDS_AcqParams *pAcqParams /* [output] acquisition parameters */
)
{
   BERR_Code retCode = BERR_SUCCESS;

   BDBG_ASSERT(hSDS);
   BDBG_ASSERT(pAcqParams);

   BKNI_Memcpy((void*)pAcqParams, &(hSDS->acqParams), sizeof(BSDS_AcqParams));

   return retCode;
}


/******************************************************************************
 BSDS_GetQpskStatus()
******************************************************************************/
BERR_Code BSDS_GetQpskStatus(
   BSDS_Handle hSDS,          /* SDS device handle */
   BSDS_QpskStatus *pStatus   /* [output] QPSK channel status */
)
{
   BERR_Code retCode = BERR_SUCCESS;
   uint8_t   hab[128];

   BDBG_ASSERT(hSDS);

   hab[0] = 0x01;
   hab[1] = 0x10;
   hab[2] = 0x00;
   BSDS_CHK_RETCODE(BSDS_P_SendHabCommand(hSDS, 3, hab, 47, hab));

   if (hab[5] & 0x03)
   {
      /* user requested legacy qpsk status but we are in turbo mode */
      BERR_TRACE(retCode = BSDS_ERR_CMD_FAILED);
      goto done;
   }

   if ((hab[5] & 0xF0) == 0xF0)
   {
      /* digicipher II */
      switch (hab[32] & 0x0F)
      {
         case 0x08: /* DCII 5/11 */
            pStatus->mode = BSDS_ModulationType_eDCII5_11;
            break;

         case 0x09: /* DCII 1/2 */
            pStatus->mode = BSDS_ModulationType_eDCII1_2;
            break;

         case 0x0A: /* DCII 3/5 */
            pStatus->mode = BSDS_ModulationType_eDCII3_5;
            break;

         case 0x0B: /* DCII 2/3 */
            pStatus->mode = BSDS_ModulationType_eDCII2_3;
            break;

         case 0x0C: /* DCII 3/4 */
            pStatus->mode = BSDS_ModulationType_eDCII3_4;
            break;

         case 0x0D: /* DCII 4/5 */
            pStatus->mode = BSDS_ModulationType_eDCII4_5;
            break;

         case 0x0E: /* DCII 5/6 */
            pStatus->mode = BSDS_ModulationType_eDCII5_6;
            break;

         case 0x0F: /* DCII 7/8 */
            pStatus->mode = BSDS_ModulationType_eDCII7_8;
            break;

         default:
            pStatus->mode = BSDS_ModulationType_eDCIIScan;
            break;
      }
   }
   else
   {
      if (hab[5] & 0x80)
      {
         switch (hab[32] & 0x0F)
         {
            case 0x00: /* DSS 1/2 */
               pStatus->mode = BSDS_ModulationType_eDss1_2;
               break;

            case 0x01: /* DSS 2/3 */
               pStatus->mode = BSDS_ModulationType_eDss2_3;
               break;

            case 0x02: /* DSS 3/4 */
               pStatus->mode = BSDS_ModulationType_eDss3_4;
               break;

            case 0x03: /* DSS 5/6 */
               pStatus->mode = BSDS_ModulationType_eDss5_6;
               break;

            case 0x04: /* DSS 6/7 */
               pStatus->mode = BSDS_ModulationType_eDss6_7;
               break;

            case 0x05: /* DSS 7/8 */
               pStatus->mode = BSDS_ModulationType_eDss7_8;
               break;
               
            default:
               pStatus->mode = BSDS_ModulationType_eDssScan;
               break;
         }         
      }
      else
      {
         switch (hab[32] & 0x0F)
         {
            case 0x00: /* DVB 1/2 */
               pStatus->mode = BSDS_ModulationType_eDvb1_2;
               break;

            case 0x01: /* DVB 2/3 */
               pStatus->mode = BSDS_ModulationType_eDvb2_3;
               break;

            case 0x02: /* DVB 3/4 */
               pStatus->mode = BSDS_ModulationType_eDvb3_4;
               break;

            case 0x03: /* DVB 5/6 */
               pStatus->mode = BSDS_ModulationType_eDvb5_6;
               break;

            case 0x04: /* DVB 6/7 */
               pStatus->mode = BSDS_ModulationType_eDvb6_7;
               break;

            case 0x05: /* DVB 7/8 */
               pStatus->mode = BSDS_ModulationType_eDvb7_8;
               break;
               
            default:
               pStatus->mode = BSDS_ModulationType_eDvbScan;
               break;
         }         
      }
   }

   pStatus->bRsLocked = hab[2] & 0x01 ? true : false;
   pStatus->bVitLocked = hab[2] & 0x08 ? true : false;
   pStatus->bBertLocked = hab[2] & 0x10 ? true : false;
   pStatus->tunerFreq = (hab[6] << 24) | (hab[7] << 16) | (hab[8] << 8) | hab[9];
   pStatus->snrEstimate = ((hab[10] << 8) | hab[11]);
   pStatus->symbolRate = (hab[12] << 24) | (hab[13] << 16) | (hab[14] << 8) | hab[15];
   pStatus->symbolRateOffset = (int32_t)((hab[16] << 24) | (hab[17] << 16) | (hab[18] << 8) | hab[19]);
   pStatus->symbolRateError = (int32_t)((hab[20] << 24) | (hab[21] << 16) | (hab[22] << 8) | hab[23]);
   pStatus->carrierOffset = (int32_t)((hab[24] << 24) | (hab[25] << 16) | (hab[26] << 8) | hab[27]);
   pStatus->carrierError = (int32_t)((hab[28] << 24) | (hab[29] << 16) | (hab[30] << 8) | hab[31]);
   pStatus->rsCorrCount = (hab[33] << 24) | (hab[34] << 16) | (hab[35] << 8) | hab[36];
   pStatus->rsUncorrCount = (hab[37] << 24) | (hab[38] << 16) | (hab[39] << 8) | hab[40];
   pStatus->inputPower = ((hab[45] << 8) | hab[46]);

   done:
   return retCode;
}


/******************************************************************************
 BSDS_GetTurboStatus()
******************************************************************************/
BERR_Code BSDS_GetTurboStatus(
   BSDS_Handle hSDS,           /* SDS device handle */
   BSDS_TurboStatus *pStatus   /* [output] Turbo channel status */
)
{
   static const BSDS_ModulationType BSDS_ModulationType_StatusTurbo[9] = {
      BSDS_ModulationType_eTurboQpsk1_2,
      BSDS_ModulationType_eTurboQpsk3_4,
      BSDS_ModulationType_eTurbo8psk2_3,
      BSDS_ModulationType_eTurbo8psk5_6,
      BSDS_ModulationType_eTurbo8psk8_9,
      BSDS_ModulationType_eTurbo8psk3_4_2_05,
      BSDS_ModulationType_eTurboQpsk2_3,
      BSDS_ModulationType_eTurbo8psk3_4_2_10,
      BSDS_ModulationType_eTurboQpsk5_6
   };

   static const BSDS_ModulationType BSDS_ModulationType_StatusTurboScan[10] = {
      BSDS_ModulationType_eTurbo8psk2_3,
      BSDS_ModulationType_eTurbo8psk5_6,
      BSDS_ModulationType_eTurbo8psk8_9,
      BSDS_ModulationType_eTurboQpsk1_2,
      BSDS_ModulationType_eTurboQpsk2_3,
      BSDS_ModulationType_eTurbo8psk3_4_2_10,
      BSDS_ModulationType_eTurbo8psk3_4_2_05,
      BSDS_ModulationType_eTurboQpsk3_4,
      BSDS_ModulationType_eTurboQpsk5_6,
      BSDS_ModulationType_eTurboQpsk7_8
   };
   
   BERR_Code retCode = BERR_SUCCESS;
   uint8_t   hab[128], cr;

   BDBG_ASSERT(hSDS);

   hab[0] = 0x01;
   hab[1] = 0x11;
   hab[2] = 0x00;
   BSDS_CHK_RETCODE(BSDS_P_SendHabCommand(hSDS, 3, hab, 57, hab));

   cr = ((hab[5] >> 4) & 0x0F);
   if (cr == 0x0F)
   {
      cr = ((hab[3] >> 4) & 0x0F);
      pStatus->mode = BSDS_ModulationType_StatusTurboScan[cr];
   }
   else if (cr == 0x09)
      pStatus->mode = hSDS->acqParams.mode;
   else
      pStatus->mode = BSDS_ModulationType_StatusTurbo[cr];
      
   
   pStatus->bFecLocked = hab[2] & 0x04 ? true : false;
   pStatus->bRcvrLocked = hab[2] & 0x02 ? true : false;
   pStatus->bBertLocked = hab[2] & 0x10 ? true : false;
   pStatus->tunerFreq = (hab[6] << 24) | (hab[7] << 16) | (hab[8] << 8) | hab[9];
   pStatus->symbolRate = (hab[12] << 24) | (hab[13] << 16) | (hab[14] << 8) | hab[15];
   pStatus->symbolRateOffset = (int32_t)((hab[16] << 24) | (hab[17] << 16) | (hab[18] << 8) | hab[19]);
   pStatus->symbolRateError = (int32_t)((hab[20] << 24) | (hab[21] << 16) | (hab[22] << 8) | hab[23]);
   pStatus->carrierOffset = (int32_t)((hab[24] << 24) | (hab[25] << 16) | (hab[26] << 8) | hab[27]);
   pStatus->carrierError = (int32_t)((hab[28] << 24) | (hab[29] << 16) | (hab[30] << 8) | hab[31]);
   pStatus->badBlockCount = (hab[35] << 24) | (hab[36] << 16) | (hab[37] << 8) | hab[38];
   pStatus->cleanBlockCount = (hab[39] << 24) | (hab[40] << 16) | (hab[41] << 8) | hab[42];
   pStatus->corrBlockCount = (hab[43] << 24) | (hab[44] << 16) | (hab[45] << 8) | hab[46];
   pStatus->corrSymbolCount = (hab[47] << 24) | (hab[48] << 16) | (hab[49] << 8) | hab[50];
   pStatus->bitsPerSym = (hab[51] << 24) | (hab[52] << 16) | (hab[53] << 8) | hab[54];
   pStatus->snrEstimate = ((hab[10] << 8) | hab[11]);
   pStatus->inputPower = ((hab[55] << 8) | hab[56]);

   done:
   return retCode;
}


/******************************************************************************
 BSDS_GetLockStatus()
******************************************************************************/
BERR_Code BSDS_GetLockStatus(
   BSDS_Handle hSDS,  /* SDS device handle */
   bool *pLocked      /* [output] true if FEC is locked */
)
{
   BERR_Code retCode = BERR_SUCCESS;
   uint8_t   sb;

   BDBG_ASSERT(hSDS);

   BSDS_CHK_RETCODE(BSDS_ReadRegister(hSDS, BCM4500_APSTAT2, &sb));
   *pLocked = (sb & BCM4500_APSTAT2_FECL) ? true : false;

   done:
   return retCode;
}


/******************************************************************************
 BSDS_GetSoftDecision()
******************************************************************************/
BERR_Code BSDS_GetSoftDecision(
   BSDS_Handle hSDS,    /* SDS device handle */
   int8_t *iVal,        /* [output] I value */
   int8_t *qVal         /* [output] Q value */
)
{
   BERR_Code retCode = BERR_SUCCESS;
   uint8_t   hab[8];

   BDBG_ASSERT(hSDS);

   hab[0] = 0x06;
   hab[1] = 0x04;
   hab[2] = 0x43;
   hab[7] = 0x00;
   BSDS_CHK_RETCODE(BSDS_P_SendHabCommand(hSDS, 8, hab, 8, hab));

   *iVal = (int8_t)hab[4];
   *qVal = (int8_t)hab[3];

   done:
   return retCode;
}

/******************************************************************************
 BSDS_GetOutputBitRate()
******************************************************************************/
BERR_Code BSDS_GetOutputBitRate(
	BSDS_Handle hSDS,  /* SDS device handle */
	uint32_t *bitrate  /* [output] bitrate in bps */
)
{
   BERR_Code retCode = BERR_SUCCESS;
   uint8_t hab[8];

   hab[0] = 0x06;
   hab[1] = 0x08;
   hab[2] = 0x8F;
   hab[7] = 0x00;
   BSDS_CHK_RETCODE(BSDS_P_SendHabCommand(hSDS, 8, hab, 8, hab));
   
   /* bitrate is in bits/sec */
   *bitrate = (uint32_t)(hab[3] << 24) | (hab[4] << 16) | (hab[5] << 8) | hab[6];

   done:
   return retCode;
}

/******************************************************************************
 BSDS_GetBerErrors()
******************************************************************************/
BERR_Code BSDS_GetBerErrors(
	BSDS_Handle hSDS,     /* SDS device handle */
	uint32_t *num_errors  /* [output] number of errors */
) 
{
   BERR_Code retCode = BERR_SUCCESS;
   uint8_t hab[8];

   hab[0] = 0x06;
   hab[1] = 0x04;
   hab[2] = 0x81;
   hab[7] = 0x00;

   BSDS_CHK_RETCODE(BSDS_P_SendHabCommand(hSDS, 8, hab, 8, hab));
 
   *num_errors = (uint32_t)(hab[3] << 24) | (hab[4] << 16) | (hab[5] << 8) | hab[6];
   
   done:
   return retCode;
}



/******************************************************************************
 BSDS_GetSoftDecisionBuf()
******************************************************************************/
BERR_Code BSDS_GetSoftDecisionBuf(
   BSDS_Handle hSDS,    /* SDS device handle */
   int8_t *iVal,        /* [output] set of 25 I values */
   int8_t *qVal         /* [output] set of 25 Q values */
)
{
   BERR_Code retCode = BERR_SUCCESS;
   int       i;
   uint8_t   hab[128], n;

   BDBG_ASSERT(hSDS);

   for (n = 0; n <= 120; n += 5)
   {
      hab[n + 0] = 0x04;
      hab[n + 1] = 0x04;
      hab[n + 2] = 0x43;
   }
   hab[n] = 0;      
   BSDS_CHK_RETCODE(BSDS_P_SendHabCommand(hSDS, (uint8_t)(n+1), hab, n, hab));

   for (n = i = 0; n <= 120; n += 5, i++)
   {
      iVal[i] = (int8_t)hab[n + 4];
      qVal[i] = (int8_t)hab[n + 3];
   }

   done:
   return retCode;
}


/******************************************************************************
 BSDS_ResetDecoderStats()
******************************************************************************/
BERR_Code BSDS_ResetDecoderStats(
   BSDS_Handle hSDS       /* SDS device handle */
)
{
   BERR_Code retCode = BERR_SUCCESS;
   uint8_t   hab[3];

   BDBG_ASSERT(hSDS);

   hab[0] = 0x01;
   hab[1] = 0x15;
   hab[2] = 0x00;
   BSDS_CHK_RETCODE(BSDS_P_SendHabCommand(hSDS, 3, hab, 1, hab));

   done:
   return retCode;
}


/******************************************************************************
 BSDS_ConfigDiseqc()
******************************************************************************/
BERR_Code BSDS_ConfigDiseqc(
   BSDS_Handle hSDS,          /* SDS device handle */
   BSDS_DiseqcTone tone,      /* DISEQC tone */
   BSDS_DiseqcVoltage voltage /* DISEQC voltage level */
)
{
   BERR_Code retCode = BERR_SUCCESS;
   uint8_t   hab[4], ctl;

   BDBG_ASSERT(hSDS);

   ctl = 0;
   if (tone == BSDS_DiseqcTone_ePresent)
      ctl |= 0x20;

   if (voltage == BSDS_DiseqcVoltage_eTop)
      ctl |= 0x40;

   hab[0] = 0x02;
   hab[1] = 0x1E;
   hab[2] = ctl;
   hab[3] = 0x00;
   BSDS_CHK_RETCODE(BSDS_P_SendHabCommand(hSDS, 4, hab, 1, hab));

   done:
   return retCode;
}


/******************************************************************************
 BSDS_SendDiseqcCommand()
******************************************************************************/
BERR_Code BSDS_SendDiseqcCommand(
   BSDS_Handle hSDS,               /* SDS device handle */
   BSDS_DiseqcSendOptions options, /* Diseqc send options */
   const uint8_t *pSendBuf,        /* contains data to send */ 
   uint8_t sendBufLen,             /* size of send buffer (0 to 8 bytes) */ 
   uint8_t *pRcvBuf,        /* [output] contains any reply bytes received */
   uint8_t *pReplyLen       /* [output] number of reply bytes received (0 to 8 bytes) */
)
{
   BERR_Code retCode = BERR_SUCCESS;
   uint8_t   hab[16], i, apmsk2, sb;

   BDBG_ASSERT(hSDS);
   BDBG_ASSERT(pSendBuf);
   BDBG_ASSERT(pRcvBuf);
   BDBG_ASSERT(pReplyLen);

   if (sendBufLen > 8)
   {
      BERR_TRACE(retCode = BERR_INVALID_PARAMETER);
      return retCode;
   }

   hab[0] = 0x02 + sendBufLen;
   hab[1] = 0x1C;
   hab[2] = options;

   for (i = 0; i < sendBufLen; i++)
      hab[3+i] = pSendBuf[i];

   hab[3+sendBufLen] = 0x00;

   /* save interrupt mask */
   if ((retCode = BSDS_ReadRegister(hSDS, BCM4500_APMSK2, &apmsk2)) != BERR_SUCCESS)
      return retCode;
      
   if (apmsk2)
   {
      /* disable all interrupts on IRQSTAT2 */
      sb = 0;
      BSDS_CHK_RETCODE(BSDS_WriteRegister(hSDS, BCM4500_APMSK2, &sb));
   }

   BKNI_WaitForEvent(hSDS->hDiseqcEvent, 0);
   BKNI_WaitForEvent(hSDS->hApiEvent, 0);

   /* enable diseqc interrupt */
   BSDS_CHK_RETCODE(BSDS_P_EnableDiseqcInterrupt(hSDS));

   BSDS_CHK_RETCODE(BSDS_P_SendHabCommand(hSDS, (uint8_t)(4+sendBufLen), hab, 1, hab));

   /* wait for diseqc done */
   if ((retCode = BSDS_P_WaitForApiEvent(hSDS, hSDS->hDiseqcEvent, 500)) != BERR_SUCCESS)
   {   
      BERR_TRACE(retCode = BSDS_ERR_DISEQC_RRTO);
   }
   BSDS_P_DisableDiseqcInterrupt(hSDS); /* disable the diseqc irq */
   if (retCode != BERR_SUCCESS)
      return retCode;

   hab[0] = 0x0C;
   hab[1] = 0x1D;
   hab[13] = 0x00;
   BSDS_CHK_RETCODE(BSDS_P_SendHabCommand(hSDS, 14, hab, 14, hab));

   if (hab[2] & BCM4500_APSTAT3_DISEQC_BUSY)
   {
      BERR_TRACE(retCode = BSDS_ERR_DISEQC_BUSY);
   }
   else if (hab[3] & BCM4500_APSTAT4_DISEQC_RCV_OVERFLOW)
   {
      BERR_TRACE(retCode = BSDS_ERR_DISEQC_OVERFLOW);
   }
   else if (hab[3] & BCM4500_APSTAT4_DISEQC_RRTO_ERR)
   {
      BERR_TRACE(retCode = BSDS_ERR_DISEQC_RRTO);
   }
   else if (hab[3] & BCM4500_APSTAT4_DISEQC_DEMOD_ERR)
   {
      BERR_TRACE(retCode = BSDS_ERR_DISEQC_DEMOD);
   }
   else if (hab[3] & BCM4500_APSTAT4_DISEQC_PARITY_ERR)
   {
      BERR_TRACE(retCode = BSDS_ERR_DISEQC_PARITY);
   }
   
   if (hab[3] & BCM4500_APSTAT4_DISEQC_TRANS_DONE)
   {
      *pReplyLen = hab[2] & BCM4500_APSTAT3_DISEQC_REPLY_BYTES;

      if (*pReplyLen > 0)
      {
         for (i = 0; i < 8; i++)
            pRcvBuf[i] = hab[4+i];
      }
   }

   done:
   /* put back the other bcm4500 interrupts */
   if (apmsk2)
   {
      BSDS_CHK_RETCODE(BSDS_WriteRegister(hSDS, BCM4500_APMSK2, &apmsk2));
   }
   
   return retCode;
}


/******************************************************************************
 BSDS_SendDiseqcAcw()
******************************************************************************/
BERR_Code BSDS_SendDiseqcAcw(
   BSDS_Handle hSDS,   /* SDS device handle */
   uint8_t acw         /* auto control word */
)
{
   BERR_Code retCode = BERR_SUCCESS;
   uint8_t   hab[4];

   BDBG_ASSERT(hSDS);

   hab[0] = 0x02;
   hab[1] = 0x1F;
   hab[2] = acw;
   hab[3] = 0x00;
   
   BSDS_CHK_RETCODE(BSDS_P_SendHabCommand(hSDS, 4, hab, 1, hab));

   done:
   return retCode;
}


/******************************************************************************
 BSDS_SetDiseqc22KhzMode()
******************************************************************************/
BERR_Code BSDS_SetDiseqc22KhzMode(
   BSDS_Handle hSDS,  
   BSDS_Diseqc22KhzMode mode
)
{
   BERR_Code retCode = BERR_SUCCESS;
   uint8_t   hab[6];

   BDBG_ASSERT(hSDS);

   hab[0] = 0x04;
   hab[1] = 0x0D;
   hab[2] = 0xEA;
   hab[3] = 0xF8;
   hab[4] = ((mode == BSDS_Diseqc22KhzMode_eTone) ? 0x03 : 0x04);
   hab[5] = 0x00;
   
   BSDS_CHK_RETCODE(BSDS_P_SendHabCommand(hSDS, 6, hab, 1, hab));

   done:
   return retCode;   
}


/******************************************************************************
 BSDS_SetGpioMode()
******************************************************************************/
BERR_Code BSDS_SetGpioMode(
   BSDS_Handle hSDS,
   bool bManualGPIO0Control,
   bool bManualTESTIO,   
   bool bManualTESTIO1
)
{
   BERR_Code retCode = BERR_SUCCESS;
   uint8_t   hab[5];

   BDBG_ASSERT(hSDS);

   hab[0] = 0x03;
   hab[1] = 0x0B;
   hab[2] = 0x3F;
   hab[3] = bManualGPIO0Control ? 0x01 : 0x00;
   hab[3] |= bManualTESTIO ? 0x02 : 0x00;
   hab[3] |= bManualTESTIO1 ? 0x04 : 0x00;
   hab[4] = 0x00;
      
   BSDS_CHK_RETCODE(BSDS_P_SendHabCommand(hSDS, 5, hab, 1, hab));
   
   done:
   return retCode;   
}


/******************************************************************************
 BSDS_SetGpioState()
******************************************************************************/
BERR_Code BSDS_SetGpioState(
   BSDS_Handle hSDS,        
   uint8_t gpioClearMask,   
   uint8_t gpioSetMask,
   uint8_t testioClearMask,
   uint8_t testioSetMask
)
{
   BERR_Code retCode = BERR_SUCCESS;
   uint8_t   hab[8];

   BDBG_ASSERT(hSDS);

   hab[0] = 0x06;
   hab[1] = 0x0B;
   hab[2] = 0x40;   
   hab[3] = gpioClearMask | gpioSetMask;
   hab[4] = (testioClearMask | testioSetMask) & 0x03;
   hab[5] = gpioSetMask;
   hab[6] = testioSetMask;
   hab[7] = 0x00;
   
   BSDS_CHK_RETCODE(BSDS_P_SendHabCommand(hSDS, 8, hab, 1, hab));
   
   done:
   return retCode;
}

/******************************************************************************
 BSDS_ReadCoreRegister()
******************************************************************************/
BERR_Code BSDS_ReadCoreRegister(
   BSDS_Handle hSDS,       /* [in] SDS device handle */
   uint8_t     address,    /* [in] address of the register to read */
   bool        bMultibyte, /* [in] true=multi-byte register, false=single-byte register */
   uint32_t    *value      /* [out] register value */
)
{
   BERR_Code retCode = BERR_SUCCESS;
   uint8_t   hab[8], len;

   BDBG_ASSERT(hSDS);

   hab[0] = bMultibyte ? 0x06 : 0x03;
   hab[1] = bMultibyte ? 0x04 : 0x02;
   hab[2] = address;   
   hab[3] = 0;
   hab[4] = 0;
   hab[5] = 0;
   hab[6] = 0;
   hab[7] = 0;
   len = bMultibyte ? 8 : 4;
   BSDS_CHK_RETCODE(BSDS_P_SendHabCommand(hSDS, 8, hab, len, hab));
   
   if (bMultibyte)
      *value = (uint32_t)((hab[3] << 24) | (hab[4] << 16) | (hab[5] << 8) | hab[6]);
   else
      *value = (uint32_t)hab[3];

   done:
   return retCode;
}


/******************************************************************************
 BSDS_GetLockStateChangeEventHandle()
******************************************************************************/
BERR_Code BSDS_GetLockStateChangeEventHandle(
   BSDS_Handle hSDS,           /* SDS device handle */
   BKNI_EventHandle *phEvent   /* [output] event handle */
)
{
   BDBG_ASSERT(hSDS);
   
   *phEvent = hSDS->hLockStateChangeEvent;
   return BERR_SUCCESS;
}


/******************************************************************************
 BSDS_GetInterruptEventHandle()
******************************************************************************/
BERR_Code BSDS_GetInterruptEventHandle(
   BSDS_Handle hSDS,           /* SDS device handle */
   BKNI_EventHandle *phEvent   /* [output] event handle */
)
{
   BDBG_ASSERT(hSDS);

   *phEvent = hSDS->hInterruptEvent;
   return BERR_SUCCESS;
}


/******************************************************************************
 BSDS_ProcessInterruptEvent()
******************************************************************************/
BERR_Code BSDS_ProcessInterruptEvent(
   BSDS_Handle hSDS,
   BKNI_EventHandle *phEvent
)
{
   BERR_Code     retCode = BERR_SUCCESS;

   BSTD_UNUSED(phEvent);
   BDBG_ASSERT(hSDS);

   BSDS_CHK_RETCODE(BSDS_P_DecodeInterrupt(hSDS));
   BSDS_P_EnableHostInterrupt(hSDS, true);
   
   done:
   return retCode;
}


/******************************************************************************
 BSDS_HandleInterrupt_isr()
******************************************************************************/
void BSDS_HandleInterrupt_isr(
   BSDS_Handle hSDS          /* SDS device handle */
)
{
   BDBG_ASSERT(hSDS);
   hSDS->settings.interruptEnableFunc(false, hSDS->settings.interruptEnableFuncParam);   
   BKNI_SetEvent(hSDS->hApiEvent);
   BKNI_SetEvent(hSDS->hInterruptEvent);
}

