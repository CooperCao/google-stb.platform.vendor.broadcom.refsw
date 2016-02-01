/***************************************************************************
 *     Copyright (c) 2005-2006, Broadcom Corporation
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
#include "bchp_ap_32.h"
#include "bchp_oob.h"
#include "bchp_ds_ter_eq.h"
#include "bvsb.h"
#include "bvsb_priv.h"
#include "bvsb_3560_priv.h"


BDBG_MODULE(bvsb_3560_priv);

/*#define BVSB_DEBUG*/
/*#define BVSB_DO_POLLING*/

#define BVSB_HAB_DONE_TIMEOUT 100

/* AP internal memory addresses */
#define BVSB_AP_INT_RAM 0x00600000
#define BVSB_AP_HAB     0x00640000


/******************************************************************************
 BVSB_3560_P_Open()
******************************************************************************/
BERR_Code BVSB_3560_P_Open(
   BVSB_Handle *h,       /* [out] BVSB handle */
   BCHP_Handle hChip,    /* [in] chip handle */
   void        *pReg,    /* [in] pointer to register or i2c handle */
   BINT_Handle hInterrupt, /* [in] interrupt handle */
   const BVSB_Settings *pDefSettings /* [in] default settings */
)
{
   BERR_Code retCode;
   BVSB_Handle hDev;
   BVSB_3560_P_Handle *p3560;

   BSTD_UNUSED(hInterrupt);
   BSTD_UNUSED(hChip);
   BDBG_ASSERT(pReg);
   BDBG_ASSERT(pDefSettings);

   /* allocate memory for the handle */
   hDev = (BVSB_Handle)BKNI_Malloc(sizeof(BVSB_P_Handle));
   BDBG_ASSERT(hDev);
   p3560 = (BVSB_3560_P_Handle *)BKNI_Malloc(sizeof(BVSB_3560_P_Handle));
   BDBG_ASSERT(p3560);
   hDev->pImpl = (void*)p3560;

   /* initialize our handle */
   p3560->hRegister = (BREG_Handle)pReg;
   BKNI_Memcpy((void*)(&(hDev->settings)), (void*)pDefSettings, sizeof(BVSB_Settings));

   /* create events */
   BVSB_CHK_RETCODE(BKNI_CreateEvent(&(p3560->hLockChangeEvent)));
   BVSB_CHK_RETCODE(BKNI_CreateEvent(&(p3560->hInitDoneEvent)));
   BVSB_CHK_RETCODE(BKNI_CreateEvent(&(p3560->hHabDoneEvent)));
   BVSB_CHK_RETCODE(BKNI_CreateEvent(&(p3560->hSignalLevelChangeEvent)));
   BVSB_CHK_RETCODE(BKNI_CreateEvent(&(p3560->hVideoKillEvent)));

   BVSB_CHK_RETCODE(BVSB_3560_P_DisableInterrupts(hDev));

   *h = hDev;

   done:
   return retCode;
}


/******************************************************************************
 BVSB_3560_P_Close()
******************************************************************************/
BERR_Code BVSB_3560_P_Close(BVSB_Handle h)
{
   BDBG_ASSERT(h);

   BVSB_3560_P_DisableInterrupts(h);
   BKNI_DestroyEvent(((BVSB_3560_P_Handle *)(h->pImpl))->hLockChangeEvent);
   BKNI_DestroyEvent(((BVSB_3560_P_Handle *)(h->pImpl))->hInitDoneEvent);
   BKNI_DestroyEvent(((BVSB_3560_P_Handle *)(h->pImpl))->hHabDoneEvent);
   BKNI_DestroyEvent(((BVSB_3560_P_Handle *)(h->pImpl))->hSignalLevelChangeEvent);
   BKNI_DestroyEvent(((BVSB_3560_P_Handle *)(h->pImpl))->hVideoKillEvent);
   BKNI_Free((void*)h->pImpl);
   BKNI_Free((void*)h);

   return BERR_SUCCESS;
}


#ifdef BVSB_DEBUG
void BVSB_3560_P_DebugDump(BVSB_Handle h)
{
   uint32_t val;

   BVSB_3560_P_Handle *p3560 = (BVSB_3560_P_Handle *)(h->pImpl);
   val = BREG_Read32(p3560->hRegister, BCHP_AP_32_H_CTL1);
   BDBG_MSG(("H_CTL1 = 0x%02X", val & 0xFF)); 
   val = BREG_Read32(p3560->hRegister, BCHP_AP_32_H_STAT2);
   BDBG_MSG(("H_STAT2 = 0x%02X", val & 0xFF)); 
   val = BREG_Read32(p3560->hRegister, BCHP_AP_32_H_STAT1);
   BDBG_MSG(("H_STAT1 = 0x%02X", val & 0xFF)); 
   val = BREG_Read32(p3560->hRegister, BCHP_AP_32_H_IE1);
   BDBG_MSG(("H_IE1 = 0x%02X", val & 0xFF)); 
   val = BREG_Read32(p3560->hRegister, BCHP_AP_32_H_IE2);
   BDBG_MSG(("H_IE2 = 0x%02X", val & 0xFF)); 
   val = BREG_Read32(p3560->hRegister, BCHP_AP_32_H_MSG1);
   BDBG_MSG(("H_MSG1 = 0x%02X", val & 0xFF)); 
   val = BREG_Read32(p3560->hRegister, BCHP_AP_32_H_MSG2);
   BDBG_MSG(("H_MSG2 = 0x%02X", val & 0xFF)); 
}
#endif


/******************************************************************************
 BVSB_3560_P_InitAp()
******************************************************************************/
BERR_Code BVSB_3560_P_InitAp(
   BVSB_Handle h,            /* [in] BVSB handle */
   const uint8_t *pHexImage  /* [in] pointer to BCM3560 microcode image */
)
{
   BVSB_3560_P_Handle *p3560 = (BVSB_3560_P_Handle *)(h->pImpl);
   BERR_Code retCode;
#ifdef BVSB_DO_POLLING
   uint32_t h_stat2, timeout;
#endif
   uint16_t n, addr;
   const uint8_t *pImage;

   /* disable interrupts */
   BVSB_CHK_RETCODE(BVSB_3560_P_DisableInterrupts(h));

   /* reset the AP before downloading the microcode */
   BVSB_CHK_RETCODE(BVSB_3560_P_ResetAp(h));

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

      /* write n bytes from pImage to addr */
      BVSB_3560_P_WriteMemory(h, addr, (uint8_t*)pImage, n);

      pImage += n;
   }
   
   /* enable init done interrupt */
   BREG_Write32(p3560->hRegister, BCHP_AP_32_H_STAT2, 0x80);
#ifndef BVSB_DO_POLLING  
   /* wait for init done interrupt */
   BREG_Write32(p3560->hRegister, BCHP_AP_32_H_IE2, 0x80);
#endif
   BKNI_WaitForEvent(p3560->hInitDoneEvent, 0);   

   /* clear H_STAT1 */
   BREG_Write32(p3560->hRegister, BCHP_AP_32_H_STAT1, 0xFF);

   /* start running the AP */
   if ((retCode = BVSB_3560_P_RunAp(h)) != BERR_SUCCESS)
      goto done;

#ifndef BVSB_DO_POLLING
   /* wait for init done interrupt */
   do
   {
      retCode = BKNI_WaitForEvent(p3560->hInitDoneEvent, 10);
   } while ((retCode != BERR_TIMEOUT) && (retCode != BERR_SUCCESS));
#else
   /* wait for init done status */
   
   for (timeout = 0; timeout < 10; timeout++)
   {
      h_stat2 = BREG_Read32(p3560->hRegister, BCHP_AP_32_H_STAT2);
      if (h_stat2 & 0x80)
         break;
      BKNI_Sleep(1);
   } 
   if (timeout >= 10)
      retCode = BERR_TIMEOUT;
#endif

   if (retCode != BERR_SUCCESS)
   {
      BDBG_ERR(("AP initialization timeout\n")); 
      BERR_TRACE(retCode = BVSB_ERR_AP_NOT_INIT);            
#ifdef BVSB_DEBUG
      BVSB_3560_P_DebugDump(h);
#endif
   }

   done:
   return retCode;
}


/******************************************************************************
 BVSB_3560_P_GetApStatus()
******************************************************************************/
BERR_Code BVSB_3560_P_GetApStatus(
   BVSB_Handle h,           /* [in] VSB device handle */
   BVSB_ApStatus *pStatus   /* [out] AP status */
)
{
   BVSB_3560_P_Handle *p3560 = (BVSB_3560_P_Handle *)(h->pImpl);
   uint32_t h_ctl1, h_stat1, h_stat2;

   h_ctl1 = BREG_Read32(p3560->hRegister, BCHP_AP_32_H_CTL1) & 0xFF;
   h_stat1 = BREG_Read32(p3560->hRegister, BCHP_AP_32_H_STAT1) & 0xFF;
   h_stat2 = BREG_Read32(p3560->hRegister, BCHP_AP_32_H_STAT2) & 0xFF;

   *pStatus = h_ctl1 | (h_stat1 << 8) | (h_stat2 << 16);
   
   done:
   return BERR_SUCCESS;
}


/******************************************************************************
 BVSB_3560_P_GetApVersion()
******************************************************************************/
BERR_Code BVSB_3560_P_GetApVersion(
   BVSB_Handle h,          /* [in] BVSB handle */
   uint16_t    *pChipId,   /* [out] BVSB chip ID */
   uint16_t    *pChipVer,  /* [out] chip revision number */
   uint8_t     *pApVer,    /* [out] AP microcode version */
   uint8_t     *pScrVer,   /* [out] acquisition script version */
   uint8_t     *pCfgVer    /* [out] host configuration version */
)
{
   BERR_Code retCode;
   uint8_t buf[16];

   buf[0] = 0x09;
   buf[14] = 0x00;
   BVSB_CHK_RETCODE(BVSB_3560_P_SendHabCommand(h, buf, 15, buf, 14, true));

   *pChipId = ((buf[1] << 8) | buf[2]);
   *pChipVer = ((buf[3] << 8) | buf[4]);
   *pApVer = buf[5];
   *pScrVer = buf[6];
   *pCfgVer = buf[7];

   done:
   return retCode;
}


/******************************************************************************
 BVSB_3560_P_ReadRegister() 
******************************************************************************/
BERR_Code BVSB_3560_P_ReadRegister(
   BVSB_Handle h,     /* [in] BVSB handle */
   uint32_t    reg,   /* [in] address of register to read */
   uint32_t    *val   /* [in] contains data that was read */
)
{
   BERR_Code retCode;
   BVSB_ApStatus status;
   BVSB_3560_P_Handle *p3560 = (BVSB_3560_P_Handle *)(h->pImpl);
   uint8_t buf[16];

   BVSB_CHK_RETCODE(BVSB_GetApStatus(h, &status));
   if ((status & 0x00800007) == 0x00800000)
   {
      buf[0] = 0x07;
      buf[1] = 0x10;
      buf[2] = (reg >> 16) & 0xFF;
      buf[3] = (reg >> 8) & 0xFF;
      buf[4] = reg & 0xFF;
      buf[9] = 0;
      BVSB_CHK_RETCODE(BVSB_3560_P_SendHabCommand(h, buf, 10, buf, 10, true));
      *val = (buf[5] << 24) | (buf[6] << 16) | (buf[7] << 8) | buf[8];
   }
   else
   {
      *val = BREG_Read32(p3560->hRegister, reg & 0x00FFFFFF);
      //*val = BREG_Read32(p3560->hRegister, reg);
   }

   done:
   return BERR_SUCCESS;
}


/******************************************************************************
 BVSB_3560_P_WriteRegister()
******************************************************************************/
BERR_Code BVSB_3560_P_WriteRegister(
   BVSB_Handle h,     /* [in] BVSB handle */
   uint32_t    reg,   /* [in] address of register to write */
   uint32_t    *val   /* [in] contains data to write */
)
{
   BERR_Code retCode;
   BVSB_ApStatus status;
   BVSB_3560_P_Handle *p3560 = (BVSB_3560_P_Handle *)(h->pImpl);
   uint8_t buf[16];

   BVSB_CHK_RETCODE(BVSB_GetApStatus(h, &status));
   if ((status & 0x00800007) == 0x00800000)
   {
      buf[0] = 0x08;
      buf[1] = 0x10;
      buf[2] = (reg >> 16) & 0xFF;
      buf[3] = (reg >> 8) & 0xFF;
      buf[4] = reg & 0xFF;
      buf[5] = (*val >> 24) & 0xFF;
      buf[6] = (*val >> 16) & 0xFF;
      buf[7] = (*val >> 8) & 0xFF;
      buf[8] = *val & 0xFF;
      buf[9] = 0;
      BVSB_CHK_RETCODE(BVSB_3560_P_SendHabCommand(h, buf, 10, buf, 1, true));
   }
   else
   {
      BREG_Write32(p3560->hRegister, reg & 0x00FFFFFF, *val);
   }

   done:
   return retCode;
}


/******************************************************************************
 BVSB_3560_P_AcquireInband()
******************************************************************************/
BERR_Code BVSB_3560_P_AcquireInband(
   BVSB_Handle h,                    /* [in] BVSB handle */
   const BVSB_InbandParams *pParams  /* [in] inband acquisition parameters */
)
{   
   BERR_Code retCode;

   BDBG_ASSERT(h);
   BDBG_ASSERT(pParams);

   BVSB_CHK_RETCODE(BVSB_3560_P_EnableLockInterrupt(h, false));
   
   switch (pParams->mode)
   {
      case BVSB_InbandMode_e8VSB:      /* 8-VSB */
      case BVSB_InbandMode_e16VSB:     /* 16-VSB */
         BVSB_CHK_RETCODE(BVSB_3560_P_AcquireVsb(h, pParams));
         break;

      case BVSB_InbandMode_e1024QAM_B: /* 1024-QAM Annex B */
      case BVSB_InbandMode_e256QAM_B:  /* 256-QAM Annex B */
      case BVSB_InbandMode_e64QAM_B:   /* 64-QAM Annex B  */
      case BVSB_InbandMode_e256QAM_A:  /* 256-QAM Annex A */
      case BVSB_InbandMode_e128QAM_A:  /* 128-QAM Annex A */
      case BVSB_InbandMode_e64QAM_A:   /* 64-QAM Annex A  */
      case BVSB_InbandMode_e32QAM_A:   /* 32-QAM Annex A  */
      case BVSB_InbandMode_e16QAM_A:   /* 16-QAM Annex A  */
         BVSB_CHK_RETCODE(BVSB_3560_P_AcquireQam(h, pParams));
         break;

      case BVSB_InbandMode_eNTSC:      /* NTSC */
         BVSB_CHK_RETCODE(BVSB_3560_P_AcquireNtsc(h, pParams));
         break;

      default:
         return (BERR_TRACE(BERR_INVALID_PARAMETER)); 
   }

   BVSB_CHK_RETCODE(BVSB_3560_P_EnableLockInterrupt(h, true));

   done:
   return retCode;
}


/******************************************************************************
 BVSB_3560_P_GetVsbStatus()
******************************************************************************/
BERR_Code BVSB_3560_P_GetVsbStatus(
   BVSB_Handle h,           /* [in] BVSB handle */
   BVSB_VsbStatus *pStatus  /* [out] VSB status */
)
{
   BERR_Code retCode;
   uint8_t   buf[128];

   if (pStatus == NULL)
      return (BERR_TRACE(BERR_INVALID_PARAMETER));

   BKNI_Memset(buf, 0, 128);
   buf[0] = 0x1E;
   BVSB_CHK_RETCODE(BVSB_3560_P_SendHabCommand(h, buf, 0x40, buf, 0x3D, true));

#if 0
   if ((retCode = BVSB_3560_P_CheckHab(h)) != BERR_SUCCESS)
   {
      BDBG_WRN(("HAB not available\n"));
      goto done;
   }
   BVSB_CHK_RETCODE(BVSB_3560_P_WriteHab(h, 0, buf, 1));
   BVSB_CHK_RETCODE(BVSB_3560_P_WriteHab(h, 0x3D, &buf[0x3D], 1));
   BVSB_CHK_RETCODE(BVSB_3560_P_ServiceHab(h, buf, 0x3D, true, 0x9E));
#endif

   switch ((buf[2] >> 4) & 0x07)
   {
      case 0:
         pStatus->acqParams.mode = BVSB_InbandMode_e8VSB;
         break;
         
      case 4:
         pStatus->acqParams.mode = BVSB_InbandMode_e16VSB;
         break;
         
      default:
         pStatus->acqParams.mode = BVSB_InbandMode_eUnknown;
         break;
   }
   
   switch ((buf[2] >> 1) & 0x03)
   {
      case 0:
         pStatus->acqSettings.bw = BVSB_PhaseLoopBw_eLow;
         break;
      case 1:
         pStatus->acqSettings.bw = BVSB_PhaseLoopBw_eMedium;
         break;
      default:
         pStatus->acqSettings.bw = BVSB_PhaseLoopBw_eHigh;
         break;
   }
   
   pStatus->acqParams.symbolRateOffset = (int32_t)((int16_t)((buf[0x08] << 8) + buf[0x09]));
   pStatus->acqParams.ifFreqOffset = (int32_t)((buf[0x04] << 24) + (buf[0x05] << 16) + (buf[0x06] << 8) + buf[0x07]);
   pStatus->acqSettings.bAutoAcq = buf[0x01] & 0x20 ? true : false;
   pStatus->acqSettings.bFastAcq = buf[0x01] & 0x01 ? true : false;
   pStatus->acqSettings.bTei = buf[0x02] & 0x01 ? true : false;
   pStatus->acqSettings.bTerr = buf[0x02] & 0x80 ? true : false;
   pStatus->acqSettings.bNtscSweep = buf[0x01] & 0x08 ? true : false;
   pStatus->acqSettings.bRfiSweep = buf[0x01] & 0x04 ? true : false;

   pStatus->bFecLocked = buf[0x12] & 0x04 ? true : false;
   pStatus->bPllLocked = buf[0x12] & 0x02 ? true : false;
   pStatus->bNtscEngaged = buf[0x12] & 0x20 ? true : false;
   pStatus->bRfiEngaged = buf[0x12] & 0x10 ? true : false;
   pStatus->ifFreq = (buf[0x0A] << 24) + (buf[0x0B] << 16) + (buf[0x0C] << 8) + buf[0x0D];
   pStatus->ifFreqError = (int32_t)((buf[0x17] << 24) + (buf[0x18] << 16) + (buf[0x19] << 8) + buf[0x1A]);
   pStatus->symbolRate = (int32_t)((buf[0x0E] << 24) + (buf[0x0F] << 16) + (buf[0x10] << 8) + buf[0x11]);
   pStatus->symbolRateError = (int32_t)((int16_t)((buf[0x1B] << 8) + buf[0x1C]));
   pStatus->corrErrCount = (buf[0x29] << 24) + (buf[0x2A] << 16) + (buf[0x2B] << 8) + buf[0x2C];
   pStatus->corrBlockCount = (buf[0x2D] << 24) + (buf[0x2E] << 16) + (buf[0x2F] << 8) + buf[0x30];
   pStatus->ucorrBlockCount = (buf[0x31] << 24) + (buf[0x32] << 16) + (buf[0x33] << 8) + buf[0x34];
   pStatus->cleanBlockCount = (buf[0x35] << 24) + (buf[0x36] << 16) + (buf[0x37] << 8) + buf[0x38];
   pStatus->berCount = (buf[0x25] << 24) + (buf[0x26] << 16) + (buf[0x27] << 8) + buf[0x28];
   pStatus->snr = (buf[0x13] << 8) + buf[0x14];
   pStatus->agf = (buf[0x1D] << 8) + buf[0x1E];
   pStatus->rf_agc = (((buf[0x1F] << 8) + buf[0x20]) * 1000) / 65535;
   pStatus->if_agc = (((buf[0x21] << 8) + buf[0x22]) * 1000) / 65535;
   pStatus->reacqCount = (buf[0x39] << 24) + (buf[0x3A] << 16) + (buf[0x3B] << 8) + buf[0x3C];

   done:
   return retCode;
}


/******************************************************************************
 BVSB_3560_P_GetQamStatus()
******************************************************************************/
BERR_Code BVSB_3560_P_GetQamStatus(
   BVSB_Handle h,           /* [in] BVSB handle */
   BVSB_QamStatus *pStatus  /* [out] QAM status   */
)
{
   BERR_Code retCode;
   uint8_t   buf[128];

   if (pStatus == NULL)
      return (BERR_TRACE(BERR_INVALID_PARAMETER));
      
   BKNI_Memset(buf, 0, 128);
   buf[0] = 0x20;
   BVSB_CHK_RETCODE(BVSB_3560_P_SendHabCommand(h, buf, 0x40, buf, 0x3D, true));

#if 0
   if ((retCode = BVSB_3560_P_CheckHab(h)) != BERR_SUCCESS)
   {
      BDBG_WRN(("HAB not available\n"));
      goto done;
   }

   buf[0] = 0x20;
   buf[0x3D] = 0x00;
   BVSB_CHK_RETCODE(BVSB_3560_P_WriteHab(h, 0, buf, 1));
   BVSB_CHK_RETCODE(BVSB_3560_P_WriteHab(h, 0x3D, &buf[0x3D], 1));
   BVSB_CHK_RETCODE(BVSB_3560_P_ServiceHab(h, buf, 0x3D, true, 0xA0));
#endif

   if (buf[2] & 0x08)
   {
      /* annex B */
      switch ((buf[2] >> 4) & 0x07)
      {
         case 3:
            pStatus->acqParams.mode = BVSB_InbandMode_e64QAM_B;
            break;
         
         case 5:
            pStatus->acqParams.mode = BVSB_InbandMode_e256QAM_B;
            break;
         
         default:
            pStatus->acqParams.mode = BVSB_InbandMode_e1024QAM_B;
            break;
      }      
   }
   else
   {
      /* annex A */
      switch ((buf[2] >> 4) & 0x07)
      {
         case 1:
            pStatus->acqParams.mode = BVSB_InbandMode_e16QAM_A;
            break;
         
         case 2:
            pStatus->acqParams.mode = BVSB_InbandMode_e32QAM_A;
            break;
         
         case 3:
            pStatus->acqParams.mode = BVSB_InbandMode_e64QAM_A;
            break;
         
         case 4:
            pStatus->acqParams.mode = BVSB_InbandMode_e128QAM_A;
            break;
         
         default:
            pStatus->acqParams.mode = BVSB_InbandMode_e256QAM_A;
            break;
      }            
   }
   
   switch ((buf[1] >> 6) & 0x03)
   {
      case 0:
         pStatus->acqSettings.nyquist = BVSB_NyquistFilter_e12;
         break;

      case 1:
         pStatus->acqSettings.nyquist = BVSB_NyquistFilter_e15;
         break;

      default:
         pStatus->acqSettings.nyquist = BVSB_NyquistFilter_e18;
         break;
   }

   switch ((buf[2] >> 1) & 0x03)
   {
      case 0:
         pStatus->acqSettings.bw = BVSB_PhaseLoopBw_eLow;
         break;
      case 1:
         pStatus->acqSettings.bw = BVSB_PhaseLoopBw_eMedium;
         break;
      default:
         pStatus->acqSettings.bw = BVSB_PhaseLoopBw_eHigh;
         break;
   }
   
   switch ((buf[3] >> 4) & 0x0F)
   {
      case 0:
         pStatus->acqSettings.idepth = BVSB_IDepth_e204_1;
         break;
         
      case 1:
         pStatus->acqSettings.idepth = BVSB_IDepth_e102_2;
         break;
         
      case 2:
         pStatus->acqSettings.idepth = BVSB_IDepth_e68_3;
         break;
         
      case 3:
         pStatus->acqSettings.idepth = BVSB_IDepth_e51_4;
         break;
         
      case 4:
         pStatus->acqSettings.idepth = BVSB_IDepth_e34_6;
         break;
         
      case 5:
         pStatus->acqSettings.idepth = BVSB_IDepth_e17_12;
         break;
         
      case 6:
         pStatus->acqSettings.idepth = BVSB_IDepth_e12_17;
         break;
         
      case 7:
         pStatus->acqSettings.idepth = BVSB_IDepth_e6_34;
         break;
         
      case 8:
         pStatus->acqSettings.idepth = BVSB_IDepth_e4_51;
         break;
         
      case 9:
         pStatus->acqSettings.idepth = BVSB_IDepth_e3_68;
         break;
         
      case 10:
         pStatus->acqSettings.idepth = BVSB_IDepth_e2_102;
         break;
         
      default:
         pStatus->acqSettings.idepth = BVSB_IDepth_e1_204;
         break;
   }
   
   pStatus->acqParams.symbolRateOffset = (int32_t)((int16_t)((buf[0x08] << 8) + buf[0x09]));
   pStatus->acqParams.ifFreqOffset = (int32_t)((buf[0x04] << 24) + (buf[0x05] << 16) + (buf[0x06] << 8) + buf[0x07]);
   pStatus->acqSettings.bAutoAcq = buf[0x01] & 0x20 ? true : false;
   pStatus->acqSettings.bFastAcq = buf[0x01] & 0x01 ? true : false;
   pStatus->acqSettings.bTei = buf[0x02] & 0x01 ? true : false;
   pStatus->acqSettings.bTerr = buf[0x02] & 0x80 ? true : false;
   pStatus->acqSettings.bSpinv = buf[0x01] & 0x10 ? true : false;
   pStatus->acqSettings.bEq = buf[0x01] & 0x04 ? true : false;
   pStatus->acqSettings.bCh = buf[0x01] & 0x02 ? true : false;
   pStatus->acqSettings.bDavic = buf[0x03] & 0x08 ? true : false;
   pStatus->bFecLocked = buf[0x12] & 0x04 ? true : false;
   pStatus->bPllLocked = buf[0x12] & 0x02 ? true : false;
   pStatus->bSpinv = (buf[0x12] & 0x01 ? true : false);
   pStatus->ifFreq = (buf[0x0A] << 24) + (buf[0x0B] << 16) + (buf[0x0C] << 8) + buf[0x0D];
   pStatus->ifFreqError = (int32_t)((buf[0x17] << 24) + (buf[0x18] << 16) + (buf[0x19] << 8) + buf[0x1A]);
   pStatus->symbolRate = (int32_t)((buf[0x0E] << 24) + (buf[0x0F] << 16) + (buf[0x10] << 8) + buf[0x11]);
   pStatus->symbolRateError = (int32_t)((int16_t)((buf[0x1B] << 8) + buf[0x1C]));
   pStatus->corrErrCount = (buf[0x29] << 24) + (buf[0x2A] << 16) + (buf[0x2B] << 8) + buf[0x2C];
   pStatus->corrBlockCount = (buf[0x2D] << 24) + (buf[0x2E] << 16) + (buf[0x2F] << 8) + buf[0x30];
   pStatus->ucorrBlockCount = (buf[0x31] << 24) + (buf[0x32] << 16) + (buf[0x33] << 8) + buf[0x34];
   pStatus->cleanBlockCount = (buf[0x35] << 24) + (buf[0x36] << 16) + (buf[0x37] << 8) + buf[0x38];
   pStatus->berCount = (buf[0x25] << 24) + (buf[0x26] << 16) + (buf[0x27] << 8) + buf[0x28];
   pStatus->snr = (buf[0x13] << 8) + buf[0x14];
   pStatus->agf = (buf[0x1D] << 8) + buf[0x1E];
   pStatus->rf_agc = (((buf[0x1F] << 8) + buf[0x20]) * 1000) / 65535;
   pStatus->if_agc = (((buf[0x21] << 8) + buf[0x22]) * 1000) / 65535;
   pStatus->reacqCount = (buf[0x39] << 24) + (buf[0x3A] << 16) + (buf[0x3B] << 8) + buf[0x3C];

   done:
   return retCode;
}


/******************************************************************************
 BVSB_3560_P_GetNtscStatus()
******************************************************************************/
BERR_Code BVSB_3560_P_GetNtscStatus(
   BVSB_Handle h,           /* [in] BVSB handle */
   BVSB_NtscStatus *pStatus /* [out] NTSC status */
)
{
   BERR_Code retCode;
   uint8_t buf[128];

   if (pStatus == NULL)
      return (BERR_TRACE(BERR_INVALID_PARAMETER));
      
   if ((retCode = BVSB_3560_P_CheckHab(h)) != BERR_SUCCESS)
   {
      BDBG_WRN(("HAB not available\n"));
      goto done;
   }
   
   buf[0] = 0x1C;
   buf[0x1B] = 0x00;
   BVSB_CHK_RETCODE(BVSB_3560_P_WriteHab(h, 0, buf, 1));
   BVSB_CHK_RETCODE(BVSB_3560_P_WriteHab(h, 0x1B, &buf[0x1B], 1));
   BVSB_CHK_RETCODE(BVSB_3560_P_ServiceHab(h, buf, 0x1B, true, 0x9C));

   switch ((buf[2] >> 1) & 0x03)
   {
      case 0:
         pStatus->acqSettings.bw = BVSB_PhaseLoopBw_eLow;
         break;
      case 1:
         pStatus->acqSettings.bw = BVSB_PhaseLoopBw_eMedium;
         break;
      default:
         pStatus->acqSettings.bw = BVSB_PhaseLoopBw_eHigh;
         break;
   }
   
   pStatus->acqParams.ifFreqOffset = (int32_t)((buf[0x04] << 24) + (buf[0x05] << 16) + (buf[0x06] << 8) + buf[0x07]);
   pStatus->acqParams.symbolRateOffset = 0;
   pStatus->acqParams.mode = BVSB_InbandMode_eNTSC;
   pStatus->acqSettings.bFastAcq = buf[0x01] & 0x01 ? true : false;
   pStatus->acqSettings.bTerr = buf[0x02] & 0x80 ? true : false;
   pStatus->acqSettings.bAutoAcq = buf[0x01] & 0x20 ? true : false;
   pStatus->pixCarrFreq = (int32_t)((buf[0x08] << 24) + (buf[0x09] << 16) + (buf[0x0A] << 8) + buf[0x0B]);
   pStatus->pixCarrFreqError = (int32_t)((buf[0x0D] << 24) + (buf[0x0E] << 16) + (buf[0x0F] << 8) + buf[0x10]);
   pStatus->rf_agc = (((buf[0x11] << 8) + buf[0x12]) * 1000) / 65535;
   pStatus->if_agc = (((buf[0x13] << 8) + buf[0x14]) * 1000) / 65535;
   pStatus->bHsyncLocked = buf[0x0C] & 0x08 ? true : false;
   pStatus->bAP1_WeakCarr= buf[0x20] & 0x80 ? true : false;
   pStatus->bAP2_WeakCarr= buf[0x21] & 0x80 ? true : false;
   pStatus->bAP1_NoisyCarr= buf[0x20] & 0x40 ? true : false;
   pStatus->bAP2_NoisyCarr= buf[0x21] & 0x40 ? true : false;
   pStatus->aud_noise_level = buf[0x22];
   pStatus->aud_noise_level_ap2 = buf[0x23];
   pStatus->aud_gain = (int16_t)((buf[0x24] << 8) | (buf[0x25]));
   pStatus->aud_gain_ap2 = (int16_t)((buf[0x26] << 8) | (buf[0x27]));     

   done:
   return retCode;
}


/******************************************************************************
 BVSB_3560_P_ResetInbandStatus()
******************************************************************************/
BERR_Code BVSB_3560_P_ResetInbandStatus(
   BVSB_Handle h /* [in] BVSB handle */
)
{
   BERR_Code retCode;
   uint8_t buf[3];

   buf[0] = 0x1F;
   buf[1] = 0x04;
   buf[2] = 0;
   BVSB_CHK_RETCODE(BVSB_3560_P_SendHabCommand(h, buf, 3, buf, 1, true));
   
   done:
   return retCode;
}


/******************************************************************************
 BVSB_3560_P_AcquireOob()
******************************************************************************/
BERR_Code BVSB_3560_P_AcquireOob(
   BVSB_Handle h,           /* [in] BVSB handle */
   const BVSB_OobParams *pParams  /* [in] acquisition parameters */
)
{
   BERR_Code retCode;
   uint8_t   hab[5];

   hab[0] = 0x0D;
   hab[1] = hab[2] = hab[3] = hab[4] = 0x00;
   
   hab[1] |= ((h->settings.oob.bSpinv) ? 0x10 : 0x00);   
   hab[1] |= ((h->settings.oob.bAutoAcq) ? 0x20 : 0x00);
   hab[2] |= ((h->settings.oob.bBypassFEC) ? 0x80 : 0x00);
   switch (pParams->mode)
   {
      case BVSB_OobMode_eDs178:
         /*hab[2] |= 0x00;*/
         break;
         
      case BVSB_OobMode_eDs167_A:
         hab[2] |= 0x18;
         break;
         
      case BVSB_OobMode_eDs167_B:
         hab[2] |= 0x28;  
         break;
         
      default:
         return (BERR_TRACE(BERR_INVALID_PARAMETER));
   }
   
   switch (h->settings.oob.bw)
   {
      case BVSB_PhaseLoopBw_eLow:
         /*hab[2] |= 0x00;*/
         break;
         
      case BVSB_PhaseLoopBw_eMedium:
         hab[2] |= 0x02;
         break;
         
      case BVSB_PhaseLoopBw_eHigh:
         hab[2] |= 0x04;
         break;
         
      default:
         return (BERR_TRACE(BERR_INVALID_PARAMETER));
   }
   
   BVSB_CHK_RETCODE(BVSB_3560_P_SendHabCommand(h, hab, 5, hab, 1, true));

   done:
   return retCode;
}


/******************************************************************************
 BVSB_3560_P_GetOobStatus()
******************************************************************************/
BERR_Code BVSB_3560_P_GetOobStatus(
   BVSB_Handle h,           /* [in] BVSB handle */
   BVSB_OobStatus *pStatus  /* [out] OOB status   */
)
{
   BERR_Code retCode;
   uint8_t   buf[128];

   if ((retCode = BVSB_3560_P_CheckHab(h)) != BERR_SUCCESS)
   {
      BDBG_WRN(("HAB not available\n"));
      goto done;
   }

   buf[0] = 0x19;
   buf[0x37] = 0x00;
   BVSB_CHK_RETCODE(BVSB_3560_P_WriteHab(h, 0, buf, 1));
   BVSB_CHK_RETCODE(BVSB_3560_P_WriteHab(h, 0x37, &buf[0x37], 1));
   BVSB_CHK_RETCODE(BVSB_3560_P_ServiceHab(h, buf, 0x37, true, 0x99));

   switch ((buf[2] >> 4) & 0x07)
   {
      case 0:
         pStatus->acqParams.mode = BVSB_OobMode_eDs178;
         break;
         
      case 1:
         pStatus->acqParams.mode = BVSB_OobMode_eDs167_A;
         break;
         
      case 2:
         pStatus->acqParams.mode = BVSB_OobMode_eDs167_B;
         break;
         
      default:
         pStatus->acqParams.mode = BVSB_OobMode_eUnknown;
         break;
   }
   
   switch ((buf[2] >> 1) & 0x03)
   {
      case 0:
         pStatus->acqSettings.bw = BVSB_PhaseLoopBw_eLow;
         break;
      case 1:
         pStatus->acqSettings.bw = BVSB_PhaseLoopBw_eMedium;
         break;
      default:
         pStatus->acqSettings.bw = BVSB_PhaseLoopBw_eHigh;
         break;
   }
   
   pStatus->acqSettings.bAutoAcq = buf[0x01] & 0x20 ? true : false;
   pStatus->acqSettings.bSpinv = buf[0x01] & 0x10 ? true : false;
   pStatus->acqSettings.bBypassFEC = buf[0x02] & 0x80 ? true : false;
   pStatus->bRcvrLocked = buf[0x10] & 0x08 ? true : false;
   pStatus->bFecLocked = buf[0x10] & 0x04 ? true : false;
   pStatus->bSpinv = buf[0x10] & 0x01 ? true : false;
   pStatus->loFreq = (buf[0x04] << 24) + (buf[0x05] << 16) + (buf[0x06] << 8) + buf[0x07];
   pStatus->ifFreq = (buf[0x08] << 24) + (buf[0x09] << 16) + (buf[0x0A] << 8) + buf[0x0B];
   pStatus->ifFreqError = (int32_t)((buf[0x15] << 24) + (buf[0x16] << 16) + (buf[0x17] << 8) + buf[0x18]);
   pStatus->symbolRate = (int32_t)((buf[0x0C] << 24) + (buf[0x0D] << 16) + (buf[0x0E] << 8) + buf[0x0F]);
   pStatus->symbolRateError = (int32_t)((int16_t)((buf[0x19] << 8) + buf[0x1A]));
   pStatus->corrBlockCount = (buf[0x23] << 24) + (buf[0x24] << 16) + (buf[0x25] << 8) + buf[0x26];
   pStatus->ucorrBlockCount = (buf[0x27] << 24) + (buf[0x28] << 16) + (buf[0x29] << 8) + buf[0x2A];
   pStatus->berCount = (buf[0x1F] << 24) + (buf[0x20] << 16) + (buf[0x21] << 8) + buf[0x22];
   pStatus->snr = (buf[0x11] << 8) + buf[0x12];
   pStatus->oob_agc = (buf[0x1B] << 8) + buf[0x1C];
   pStatus->ext_agc = (buf[0x1D] << 8) + buf[0x1E];   
   pStatus->reacqCount = (buf[0x2B] << 24) + (buf[0x2C] << 16) + (buf[0x2D] << 8) + buf[0x2E];
   pStatus->atm_cell_total_count = (buf[0x2F] << 24) + (buf[0x30] << 16) + (buf[0x31] << 8) + buf[0x32];
   pStatus->atm_cell_loss_count = (buf[0x33] << 24) + (buf[0x34] << 16) + (buf[0x35] << 8) + buf[0x36];

   done:
   return retCode;
}


/******************************************************************************
 BVSB_3560_P_ResetOobStatus()
******************************************************************************/
BERR_Code BVSB_3560_P_ResetOobStatus(
   BVSB_Handle h /* [in] BVSB handle */
)
{
   BERR_Code retCode;
   uint8_t buf[3];

   buf[0] = 0x21;
   buf[1] = 0x04;
   buf[2] = 0;
   BVSB_CHK_RETCODE(BVSB_3560_P_SendHabCommand(h, buf, 3, buf, 1, true));
   
   done:
   return retCode;
}


/******************************************************************************
 BVSB_3560_P_SetInbandOi()
******************************************************************************/
BERR_Code BVSB_3560_P_SetInbandOi(
   BVSB_Handle           h,         /* [in] BVSB handle */
   BVSB_InbandOiSettings *pInbandOi /* [in] inband transport output interface settings */
)
{
   BERR_Code retCode;
   uint8_t buf[4];

   buf[0] = 0x0F;
   buf[1] = buf[2] = buf[3] = 0;
   buf[1] |= (pInbandOi->bHead4 ? 0x40 : 0x00); 
   buf[1] |= (pInbandOi->bSync1 ? 0x20 : 0x00); 
   buf[1] |= (pInbandOi->bXBERT ? 0x04 : 0x00); 
   buf[2] |= (pInbandOi->bErrinv ? 0x80 : 0x00); 
   buf[2] |= (pInbandOi->bSyncinv ? 0x40 : 0x00); 
   buf[2] |= (pInbandOi->bVldinv ? 0x20 : 0x00); 
   buf[2] |= (pInbandOi->bClksup ? 0x10 : 0x00); 
   buf[2] |= (pInbandOi->bClkinv ? 0x08 : 0x00); 
   buf[2] |= (pInbandOi->bSerial ? 0x02 : 0x00); 

   BVSB_CHK_RETCODE(BVSB_3560_P_SendHabCommand(h, buf, 4, buf, 1, true));
 
   done:
   return retCode;
}


/******************************************************************************
 BVSB_3560_P_GetSoftDecisionBuf()
******************************************************************************/
BERR_Code BVSB_3560_P_GetSoftDecisionBuf(
   BVSB_Handle h,  /* [in] BVSB handle */
   int16_t *pI,    /* [out] 30 I-values */
   int16_t *pQ     /* [out] 30 Q-values */
)
{
#if 0
   BERR_Code retCode;
   uint8_t   hab[128], i;
   
   if ((retCode = BVSB_3560_P_CheckHab(h)) != BERR_SUCCESS)
   {
      BDBG_WRN(("HAB not available\n"));
      goto done;
   }

   hab[0] = 0x15;
   hab[1] = 0x10;
   hab[2] = 0x6A;
   hab[3] = 0x20;
   hab[4] = 0x48;
   hab[5] = 0x1E;
   hab[0x7E] = hab[0x7F] = 0x00;
   BVSB_CHK_RETCODE(BVSB_3560_P_WriteHab(h, 0, hab, 6));
   BVSB_CHK_RETCODE(BVSB_3560_P_WriteHab(h, 0x7E, &hab[0x7E], 2));
   BVSB_CHK_RETCODE(BVSB_3560_P_ServiceHab(h, hab, 0x7F, true, 0x95));
   
   for (i = 0; i < 30; i++)
   {
      pI[i] = (hab[6+(4*i)] << 4) | ((hab[7+(4*i)] >> 4) & 0x0F);
      pQ[i] = (hab[8+(4*i)] << 4) | ((hab[9+(4*i)] >> 4) & 0x0F);
      
      if (pI[i] & 0x800)
         pI[i] -= 0x1000;
      if (pQ[i] & 0x800)
         pQ[i] -= 0x1000;
   }
   
   done:
   return retCode;
#else
   BVSB_3560_P_Handle *p3560 = (BVSB_3560_P_Handle *)(h->pImpl);
   uint32_t regval;
   uint8_t  i;

   for (i = 0; i < 30; i++)
   {
      regval = BREG_Read32(p3560->hRegister, BCHP_DS_TER_EQ_SOFT);
      pI[i] = (regval >> 20) & 0x0FFF;
      pQ[i] = (regval >> 4) & 0x0FFF;
      if (pI[i] & 0x800)
         pI[i] -= 0x1000;
      if (pQ[i] & 0x800)
         pQ[i] -= 0x1000;
   }

   return BERR_SUCCESS;
#endif
}


/******************************************************************************
 BVSB_3560_P_GetOobSoftDecisionBuf()
******************************************************************************/
BERR_Code BVSB_3560_P_GetOobSoftDecisionBuf(
   BVSB_Handle h,  /* [in] BVSB handle */
   int16_t *pI,    /* [out] 30 I-values */
   int16_t *pQ     /* [out] 30 Q-values */
)
{
#if 0
   BERR_Code retCode;
   uint8_t   hab[128], i;
   
   if ((retCode = BVSB_3560_P_CheckHab(h)) != BERR_SUCCESS)
   {
      BDBG_WRN(("HAB not available\n"));
      goto done;
   }

   hab[0] = 0x15;
   hab[1] = 0x10;
   hab[2] = 0x6C;
   hab[3] = 0x00;
   hab[4] = 0x4C;
   hab[5] = 0x1E;
   hab[0x7E] = hab[0x7F] = 0x00;
   BVSB_CHK_RETCODE(BVSB_3560_P_WriteHab(h, 0, hab, 6));
   BVSB_CHK_RETCODE(BVSB_3560_P_WriteHab(h, 0x7E, &hab[0x7E], 2));
   BVSB_CHK_RETCODE(BVSB_3560_P_ServiceHab(h, hab, 0x7F, true, 0x95));
   
   for (i = 0; i < 30; i++)
   {
      pI[i] = (int16_t)(hab[6+(4*i)]);
      pQ[i] = (int16_t)(hab[7+(4*i)]);
   }
   
   done:
   return retCode;
#else
   BVSB_3560_P_Handle *p3560 = (BVSB_3560_P_Handle *)(h->pImpl);
   uint32_t regval;
   uint8_t  i;

   for (i = 0; i < 30; i++)
   {
      regval = BREG_Read32(p3560->hRegister, BCHP_OOB_LDSFT);
      pI[i] = (regval >> 24) & 0x0FF;
      pQ[i] = (regval >> 16) & 0x0FF;
      if (pI[i] & 0x80)
         pI[i] -= 0x100;
      if (pQ[i] & 0x80)
         pQ[i] -= 0x100;
   }

   return BERR_SUCCESS;
#endif
}


/******************************************************************************
 BVSB_3560_P_WriteConfig()
******************************************************************************/
BERR_Code BVSB_3560_P_WriteConfig(
   BVSB_Handle h,   /* [in] BVSB handle */
   uint16_t offset, /* [in] 16-bit offset within the host configuration space */
   uint8_t *buf,    /* [in] data to write */
   uint8_t len      /* [in] number of bytes to write */
)
{
   BERR_Code retCode;
   uint8_t   hab[128], i;

   if ((len < 1) || (len > 121))
      return (BERR_TRACE(BERR_INVALID_PARAMETER));

   hab[0] = 0x13;
   hab[1] = (offset >> 8);
   hab[2] = (offset & 0xFF);   
   hab[3] = len;
   hab[4+len] = 0x00;

   for (i = 0; i < len; i++)
      hab[4+i] = buf[i];

   BVSB_CHK_RETCODE(BVSB_3560_P_SendHabCommand(h, hab, 5+len, hab, 1, true));
 
   done:
   return retCode;
}


/******************************************************************************
 BVSB_3560_P_ReadConfig()
******************************************************************************/
BERR_Code BVSB_3560_P_ReadConfig(
   BVSB_Handle h,  /* [in] BVSB handle */
   uint16_t offset,   /* [in] 16-bit offset within the host configuration space */
   uint8_t *buf,      /* [in] buffer to hold the read data */
   uint8_t len        /* [in] number of bytes to read */
)
{
   BERR_Code retCode;
   uint8_t   hab[128], i;

   if ((len < 1) || (len > 121))
      return (BERR_TRACE(BERR_INVALID_PARAMETER));

   hab[0] = 0x12;
   hab[1] = (offset >> 8);
   hab[2] = (offset & 0xFF);   
   hab[3] = len;
   hab[4+len] = 0;

   BVSB_CHK_RETCODE(BVSB_3560_P_SendHabCommand(h, hab, 5+len, hab, 5+len, true));

   for (i = 0; i < len; i++)
      buf[i] = hab[4+i];

   done:
   return retCode;
}

/******************************************************************************
 BVSB_3560_P_GetVideoKillEventHandle()
******************************************************************************/
BERR_Code BVSB_3560_P_GetVideoKillEventHandle(
   BVSB_Handle h,            /* [in] BVSB handle */
   BKNI_EventHandle *hEvent  /* [out] lock event handle */
)
{
   *hEvent = ((BVSB_3560_P_Handle *)(h->pImpl))->hVideoKillEvent;
   return BERR_SUCCESS;
}

/******************************************************************************
 BVSB_3560_P_GetSignalChangeEventHandle()
******************************************************************************/
BERR_Code BVSB_3560_P_GetSignalChangeEventHandle(
   BVSB_Handle h,            /* [in] BVSB handle */
   BKNI_EventHandle *hEvent  /* [out] lock event handle */
)
{
   *hEvent = ((BVSB_3560_P_Handle *)(h->pImpl))->hSignalLevelChangeEvent;
   return BERR_SUCCESS;
}

/******************************************************************************
 BVSB_3560_P_GetLockStateChangeEventHandle()
******************************************************************************/
BERR_Code BVSB_3560_P_GetLockStateChangeEventHandle(
   BVSB_Handle h,            /* [in] BVSB handle */
   BKNI_EventHandle *hEvent  /* [out] lock event handle */
)
{
   *hEvent = ((BVSB_3560_P_Handle *)(h->pImpl))->hLockChangeEvent;
   return BERR_SUCCESS;
}



/******************************************************************************
 BVSB_3560_P_ReadHab()
******************************************************************************/
BERR_Code BVSB_3560_P_ReadHab(BVSB_Handle h, uint8_t addr, uint8_t *buf, uint8_t n)
{
   BVSB_3560_P_Handle *p3560 = (BVSB_3560_P_Handle *)(h->pImpl);
   uint8_t i;

   if ((addr & 0x80) || (n > 128))
      return BERR_TRACE(BERR_INVALID_PARAMETER);

   if ((addr + n) > 0x80)
      return BERR_TRACE(BERR_INVALID_PARAMETER);

   for (i = addr; i < (addr+n); i++)
      buf[i] = (uint8_t)(BREG_Read32(p3560->hRegister, BVSB_AP_HAB+(i*4)) & 0x000000FF);

   done:
   return BERR_SUCCESS;
}


/******************************************************************************
 BVSB_3560_P_WriteHab()
******************************************************************************/
BERR_Code BVSB_3560_P_WriteHab(BVSB_Handle h, uint8_t addr, uint8_t *buf, uint8_t n)
{
   BVSB_3560_P_Handle *p3560 = (BVSB_3560_P_Handle *)(h->pImpl);
   uint8_t i;

   if ((addr & 0x80) || (n & 0x80))
      return BERR_TRACE(BERR_INVALID_PARAMETER);

   if ((addr + n) > 0x80)
      return BERR_TRACE(BERR_INVALID_PARAMETER);

   for (i = addr; i < (addr+n); i++)
      BREG_Write32(p3560->hRegister, BVSB_AP_HAB+(i*4), (uint32_t)(buf[i]));

   done:
   return BERR_SUCCESS;
}


/******************************************************************************
 BVSB_3560_P_ResetAp()
******************************************************************************/
BERR_Code BVSB_3560_P_ResetAp(BVSB_Handle h)
{
   BVSB_3560_P_Handle *p3560 = (BVSB_3560_P_Handle *)(h->pImpl);
   BERR_Code retCode = BERR_SUCCESS;
   uint32_t regval;

   /* initialize JDEC to boot from internal RAM */
   BREG_Write32(p3560->hRegister, BCHP_AP_32_JDEC, 2);

   /* reset the AP */
   BREG_Write32(p3560->hRegister, BCHP_AP_32_H_CTL1, 0x01);

   /* reset AP status */
   BREG_Write32(p3560->hRegister, BCHP_AP_32_AP_CMD, 0xB);

   /* reset all interrupt status */
   BREG_Write32(p3560->hRegister, BCHP_AP_32_H_STAT1, 0xFF);
   BREG_Write32(p3560->hRegister, BCHP_AP_32_H_STAT2, 0xFF);
   BREG_Write32(p3560->hRegister, BCHP_AP_32_H_STAT3, 0xFF);
   BREG_Write32(p3560->hRegister, BCHP_AP_32_H_STAT4, 0xFF);

   /* clear MSGx registers */
   BREG_Write32(p3560->hRegister, BCHP_AP_32_H_MSG1, 0);
   BREG_Write32(p3560->hRegister, BCHP_AP_32_H_MSG2, 0);

   /* verify that AP is reset */
   regval = BREG_Read32(p3560->hRegister, BCHP_AP_32_H_CTL1);
   if ((regval & 0x03) != 1)
   {
      BDBG_ERR(("unable to reset the AP\n"));
      BERR_TRACE(retCode = BVSB_ERR_AP_FAIL);
   }

   done:
   return retCode;
}


/******************************************************************************
 BVSB_3560_P_RunAp()
******************************************************************************/
BERR_Code BVSB_3560_P_RunAp(BVSB_Handle h)
{
   BVSB_3560_P_Handle *p3560 = (BVSB_3560_P_Handle *)(h->pImpl);
   BERR_Code retCode = BERR_SUCCESS;
   uint32_t  regval;

   /* check if the AP is currently running */
   regval = BREG_Read32(p3560->hRegister, BCHP_AP_32_H_CTL1); 

   if ((regval & 0x03) != 0)
   {
      /* start running the AP */
      BREG_Write32(p3560->hRegister, BCHP_AP_32_H_CTL1, 0x60);

      /* verify that the AP is running */
      regval = BREG_Read32(p3560->hRegister, BCHP_AP_32_H_CTL1);
      if ((regval & 0x03) != 0)
      {
         BDBG_ERR(("unable to run the AP\n"));
         BERR_TRACE(retCode = BVSB_ERR_AP_FAIL);
         goto done;
      }

      /* clear AP_change_state bit */
      BREG_Write32(p3560->hRegister, BCHP_AP_32_H_STAT1, 1); 
   }
   else
   {
      BDBG_WRN(("BVSB_3560_P_RunAp(): AP already running\n"));
   }

   done:
   return retCode;
}


/******************************************************************************
 BVSB_3560_P_DisableInterrupts()
******************************************************************************/ 
BERR_Code BVSB_3560_P_DisableInterrupts(
   BVSB_Handle h   /* [in] BVSB PI Handle */
)
{
   BVSB_3560_P_Handle *p3560 = (BVSB_3560_P_Handle *)(h->pImpl);
   
   /* clear IEx registers */
   BREG_Write32(p3560->hRegister, BCHP_AP_32_H_IE1, 0);
   BREG_Write32(p3560->hRegister, BCHP_AP_32_H_IE2, 0);
   return BERR_SUCCESS;
}


/******************************************************************************
 BVSB_3560_P_DecodeError()
******************************************************************************/
BERR_Code BVSB_3560_P_DecodeError(
   BVSB_Handle h,           /* [in] BVSB PI Handle */
   BVSB_ApStatus *pApStatus /* [in] AP status returned by BVSB_GetApStatus */
)
{
   BVSB_3560_P_Handle *p3560 = (BVSB_3560_P_Handle *)(h->pImpl);
   BERR_Code retCode = BERR_SUCCESS;
   uint32_t mb;

   if (*pApStatus & BVSB_APSTATUS_HAB_ERR)
      retCode = BVSB_ERR_HABAV;
   else if (*pApStatus & BVSB_APSTATUS_MEM_ERR)
      retCode = BVSB_ERR_MEMAV;
   else if (*pApStatus & BVSB_APSTATUS_H_ERR)
      retCode = BVSB_ERR_HOST_XFER;
   else if (*pApStatus & BVSB_APSTATUS_IOMB_ERR)
      retCode = BVSB_ERR_IOMB_XFER;
   else if (*pApStatus & BVSB_APSTATUS_HABCMD_ERR)
      retCode = BVSB_ERR_HAB_ERR;
   else if (*pApStatus & BVSB_APSTATUS_AP_ERR)
   {
      mb = BREG_Read32(p3560->hRegister, BCHP_AP_32_H_MSG1);
      switch (mb)
      {
         case 1:
            retCode = BVSB_ERR_AP_BSC;
            break;

         case 2:
            retCode = BVSB_ERR_AP_STACK;
            break;

         case 3:
            retCode = BVSB_ERR_AP_WD;
            break;

         case 4:
            retCode = BVSB_ERR_AP_ISB;
            break;

         case 5:
            retCode = BVSB_ERR_AP_SCR;
            break;

         case 6:
            retCode = BVSB_ERR_AP_IRQ;
            break;

         case 7:
            retCode = BVSB_ERR_AP_COPY;
            break;

         case 8:
            retCode = BVSB_ERR_AP_EEPROM;
            break;

         case 9:
            retCode = BVSB_ERR_AP_HABAV;
            break;

         default:
            BDBG_ERR(("unknown MSG1 (=0x%02X)\n", mb));
            retCode = BVSB_ERR_AP_UNKNOWN;
            break;
      }
   }

   done:
   return retCode;
}


/******************************************************************************
 BVSB_3560_P_SendHabCommand()
******************************************************************************/
BERR_Code BVSB_3560_P_SendHabCommand(
   BVSB_Handle h,      /* [in] BVSB PI Handle */
   uint8_t *write_buf, /* [in] specifies the HAB command to send */
   uint8_t write_len,  /* [in] number of bytes in the HAB command */ 
   uint8_t *read_buf,  /* [out] holds the data read from the HAB */ 
   uint8_t read_len,   /* [in] number of bytes to read from the HAB */
   bool bCheckForAck   /* [in] true = determine if the AP has serviced the command */
)
{
   BERR_Code retCode;
   
   if ((write_len > 127) || (read_len > 127) || (write_len == 0))
      return (BERR_TRACE(BERR_INVALID_PARAMETER));

   BVSB_CHK_RETCODE(BVSB_3560_P_CheckHab(h));
  
   /* write the command to the HAB */
   BVSB_CHK_RETCODE(BVSB_3560_P_WriteHab(h, 0, write_buf, write_len));

   /* wait for the AP to service the HAB, and then read any return data */
   BVSB_CHK_RETCODE(BVSB_3560_P_ServiceHab(h, read_buf, read_len, bCheckForAck, write_buf[0] | 0x80));
 
   done:
   return retCode;
}


/******************************************************************************
 BVSB_3560_P_ServiceHab()
******************************************************************************/
BERR_Code BVSB_3560_P_ServiceHab(
   BVSB_Handle h,   /* [in] BVSB PI Handle */
   uint8_t *read_buf,  /* [out] holds the data read from the HAB */ 
   uint8_t read_len,   /* [in] number of bytes to read from the HAB */
   bool bCheckForAck,  /* [in] true = determine if the AP has serviced the command */
   uint8_t ack_byte    /* [in] value of the ack byte to expect */
)
{
   BVSB_3560_P_Handle *p3560 = (BVSB_3560_P_Handle *)(h->pImpl);
   BERR_Code retCode;
   uint32_t  ie2;
#ifdef BVSB_DO_POLLING
   uint32_t  val, timeout;
#endif

   /* save ie2 */
   ie2 = BREG_Read32(p3560->hRegister, BCHP_AP_32_H_IE2);
   
   /* clear ie1 and ie2 */
   BVSB_3560_P_DisableInterrupts(h);

   /* clear and enable HAB done interrupt status */
   BREG_Write32(p3560->hRegister, BCHP_AP_32_H_STAT1, 4);
#ifndef BVSB_DO_POLLING
   BREG_Write32(p3560->hRegister, BCHP_AP_32_H_IE1, 4);
#endif
   BKNI_WaitForEvent(p3560->hHabDoneEvent, 0);      
   
   /* send the command */
   BREG_Write32(p3560->hRegister, BCHP_AP_32_H_CTL1, 0x64);
   
#ifndef BVSB_DO_POLLING
   /* wait for HAB done interrupt */  
   do
   {
      retCode = BKNI_WaitForEvent(p3560->hHabDoneEvent, BVSB_HAB_DONE_TIMEOUT);
   } while ((retCode != BERR_TIMEOUT) && (retCode != BERR_SUCCESS));
#else
   /* poll for HAB done */
   for (timeout = 0; timeout < BVSB_HAB_DONE_TIMEOUT; timeout++)
   {
      val = BREG_Read32(p3560->hRegister, BCHP_AP_32_H_STAT1);
      if (val & 0x04)
         break;
      BKNI_Sleep(1);
   }
   if (timeout >= BVSB_HAB_DONE_TIMEOUT)
      retCode = BERR_TIMEOUT; 
#endif

   if (retCode == BERR_TIMEOUT)
   {
      BDBG_ERR(("HAB timeout\n"));   
      BREG_Write32(p3560->hRegister, BCHP_AP_32_H_IE1, 0);
      BERR_TRACE(retCode = BVSB_ERR_HAB_TIMEOUT);
      ie2 = 0;
      goto done;
   }

   if (read_len > 0)
   {
      BVSB_CHK_RETCODE(BVSB_3560_P_ReadHab(h, 0, read_buf, read_len));
      if (bCheckForAck)
      {
         if (ack_byte != read_buf[0])
         {
            BDBG_ERR(("HAB command not serviced!, got 0x%02X, expected 0x%02X\n", read_buf[0], ack_byte));
            BERR_TRACE(retCode = BVSB_ERR_HAB_NO_ACK);
#if 0 /* debug */
{
   int i, j;
   uint8_t buf[128];
   for (j = 0; j < 2; j++)
   { 
   BVSB_3560_P_ReadHab(h, 0, buf, 128);
   for (i = 0; i < 128; i++)
      printf("hab[%d] = 0x%02X\n", i, buf[i]);
   printf("------\n");
   }
}
#endif
         }
      }
   }

   done:
   BREG_Write32(p3560->hRegister, BCHP_AP_32_H_IE2, ie2);
   return retCode;
}


/******************************************************************************
 BVSB_3560_P_CheckHab()
******************************************************************************/
BERR_Code BVSB_3560_P_CheckHab(
   BVSB_Handle h    /* [in] BVSB PI Handle */
)
{
   BERR_Code retCode;
   BVSB_ApStatus status;

   BVSB_CHK_RETCODE(BVSB_GetApStatus(h, &status));
   /*if ((status & BVSB_APSTATUS_HAB_MASK) == BVSB_APSTATUS_HAB_READY)*/
   if ((status & 0x00800007) == 0x00800000)
      retCode = BERR_SUCCESS;
   else
   {
      retCode = BVSB_3560_P_DecodeError(h, &status);
      BERR_TRACE(retCode);
#ifdef BVSB_DEBUG
      BVSB_3560_P_DebugDump(h);
#endif
   }

   done:
   return retCode;
}

/******************************************************************************
 BVSB_3560_P_EnableVideoKillDetectInterrupt()
******************************************************************************/ 
BERR_Code BVSB_3560_P_EnableVideoKillDetectInterrupt(
    BVSB_Handle h, /* [in] BVSB PI Handle */
    bool bEnable /* [in] true = enable video kill detect interrupts, false = disable video kill detect*/
)
{
   BVSB_3560_P_Handle *p3560 = (BVSB_3560_P_Handle *)(h->pImpl);
   uint32_t regval;
   
   BREG_Write32(p3560->hRegister, BCHP_AP_32_H_STAT3, 0x18);
   regval = BREG_Read32(p3560->hRegister, BCHP_AP_32_H_FSTAT1);
   regval &= ~0x18; //clear any existing enable flag
   regval |= bEnable ? 0x08 : 0;
   BREG_Write32(p3560->hRegister, BCHP_AP_32_H_IE3, regval);
   BKNI_WaitForEvent(p3560->hVideoKillEvent, 0);   
   return BERR_SUCCESS;
}
/******************************************************************************
 BVSB_3560_P_EnableLowSignalDetectInterrupt()
******************************************************************************/ 
BERR_Code BVSB_3560_P_EnableLowSignalDetectInterrupt(
    BVSB_Handle h, /* [in] BVSB PI Handle */
    bool bEnable /* [in] true = enable low signal detect interrupts, false = disable low signal detect*/
)
{
   BVSB_3560_P_Handle *p3560 = (BVSB_3560_P_Handle *)(h->pImpl);
   uint32_t regval;
   
   BREG_Write32(p3560->hRegister, BCHP_AP_32_H_STAT3, 0x03);
   regval = BREG_Read32(p3560->hRegister, BCHP_AP_32_H_FSTAT1);
   regval &= ~0x3; //clear any existing enable flag
   regval = bEnable ? 0x01 : 0;
   BREG_Write32(p3560->hRegister, BCHP_AP_32_H_IE3, regval);
   BKNI_WaitForEvent(p3560->hSignalLevelChangeEvent, 0);   
   return BERR_SUCCESS;
}

/******************************************************************************
 BVSB_3560_P_EnableLockInterrupt()
******************************************************************************/ 
BERR_Code BVSB_3560_P_EnableLockInterrupt(
   BVSB_Handle h, /* [in] BVSB PI Handle */
   bool bEnable   /* [in] true = enable lock interrupts, false = disables lock interrupts */
)
{
   BVSB_3560_P_Handle *p3560 = (BVSB_3560_P_Handle *)(h->pImpl);
   uint32_t regval;
   
   BREG_Write32(p3560->hRegister, BCHP_AP_32_H_STAT2, 0x30);
   regval = bEnable ? 0x20 : 0;
   BREG_Write32(p3560->hRegister, BCHP_AP_32_H_IE2, regval);
   BKNI_WaitForEvent(p3560->hLockChangeEvent, 0);   
   return BERR_SUCCESS;
}


/******************************************************************************
 BVSB_3560_P_HandleInterrupt_isr() - This is the interrupt handler for 
                                     AP_CPU_INTR interrupt.
******************************************************************************/
BERR_Code BVSB_3560_P_HandleInterrupt_isr(
   BVSB_Handle h /* [in] BVSB handle */
)
{
   BVSB_3560_P_Handle *p3560 = (BVSB_3560_P_Handle *)(h->pImpl);
   uint32_t fstat1, fstat2, fstat3, ie1, ie2, ie3, new_stat2;

   fstat1 = BREG_Read32(p3560->hRegister, BCHP_AP_32_H_FSTAT1);
   fstat2 = BREG_Read32(p3560->hRegister, BCHP_AP_32_H_FSTAT2);
   fstat3 = BREG_Read32(p3560->hRegister, BCHP_AP_32_H_FSTAT3);
   ie1 = BREG_Read32(p3560->hRegister, BCHP_AP_32_H_IE1);
   ie2 = BREG_Read32(p3560->hRegister, BCHP_AP_32_H_IE2);
   ie3 = BREG_Read32(p3560->hRegister, BCHP_AP_32_H_IE3);
   
   new_stat2 = 0;
   
   if (!fstat1 && !fstat2 && !fstat3)
      return BERR_SUCCESS;
   
   if (fstat1 & 4) /* HAB_DONE */
   {
      ie1 = 0;
      BKNI_SetEvent(p3560->hHabDoneEvent);
   }
   
   if (fstat2 & 0x80) /* INIT_DONE */
   {
      ie2 &= ~0x80;
      BKNI_SetEvent(p3560->hInitDoneEvent);
   }
      
   if (fstat2 & 0x30) /* lock status change interrupt */
   {
      new_stat2 = 0x30; /* LOCK MASK */
      ie2 &= ~0x30;
      BKNI_SetEvent(p3560->hLockChangeEvent);
      if (fstat2 & 0x20) /* IN_LOCK */
      {
         ie2 |= 0x10; /* set OUT_OF_LOCK interrupt */
         /*BDBG_MSG(("locked"));*/
      }
      else
      {
         ie2 |= 0x20; /* set IN_LOCK interrupt */
         /*BDBG_MSG(("not locked"));*/
      }
   }
   
   if (fstat3 & 0x03) /* signal level changed detected */
   {
      if ( fstat3 & 0x1 ) 
      {
          ie3 |= 0x02;
          ie3 &= ~0x01;
      }
      else 
      {
          ie3 |= 0x01;
          ie3 &= ~0x02;
      }
      BREG_Write32(p3560->hRegister, BCHP_AP_32_H_STAT3, 0x03);   
      BREG_Write32(p3560->hRegister, BCHP_AP_32_H_IE3, ie3);   
      BKNI_SetEvent(p3560->hSignalLevelChangeEvent);
   }
 
   if (fstat3 & 0x18) /* signal level changed detected */
   {
      if ( fstat3 & 0x8 ) 
      {
          ie3 |= 0x10;
          ie3 &= ~0x08;
      }
      else 
      {
          ie3 |= 0x08;
          ie3 &= ~0x10;
      }
      BREG_Write32(p3560->hRegister, BCHP_AP_32_H_STAT3, 0x18);   
      BREG_Write32(p3560->hRegister, BCHP_AP_32_H_IE3, ie3);   
      BKNI_SetEvent(p3560->hVideoKillEvent);
   }
         
   /* clear the interrupt status */
   BREG_Write32(p3560->hRegister, BCHP_AP_32_H_IE1, ie1);   
   BREG_Write32(p3560->hRegister, BCHP_AP_32_H_IE2, ie2);   
   BREG_Write32(p3560->hRegister, BCHP_AP_32_H_STAT2, new_stat2);             
    
   done:
   return BERR_SUCCESS;
}


/******************************************************************************
 BVSB_3560_P_AcquireVsb()
******************************************************************************/
BERR_Code BVSB_3560_P_AcquireVsb(
   BVSB_Handle h,                    /* [in] BVSB handle */
   const BVSB_InbandParams *pParams  /* [in] inband acquisition parameters */
)
{
   BERR_Code retCode;
   uint8_t   hab[16];

   hab[0] = 0x0A;
   hab[1] = hab[2] = hab[10] = 0x00;
   hab[3] = 0x01;
   
   hab[1] |= ((h->settings.vsb.bAutoAcq) ? 0x20 : 0x00);
   hab[1] |= ((h->settings.vsb.bFastAcq) ? 0x01 : 0x00);
   hab[1] |= ((h->settings.vsb.bNtscSweep) ? 0x08 : 0x00);
   hab[1] |= ((h->settings.vsb.bRfiSweep) ? 0x04 : 0x00);
   hab[2] |= ((h->settings.vsb.bTei) ? 0x01 : 0x00);
   hab[2] |= ((h->settings.vsb.bTerr) ? 0x80 : 0x00);

   switch (pParams->mode)
   {
      case BVSB_InbandMode_e8VSB:
         /*hab[2] |= 0x00;*/
         break;

      case BVSB_InbandMode_e16VSB:
         hab[2] |= 0x40;
         break;

      default:
         return (BERR_TRACE(BERR_INVALID_PARAMETER));
   }

   switch (h->settings.vsb.bw)
   {
      case BVSB_PhaseLoopBw_eLow:
         /*hab[2] |= 0x00;*/
         break;
         
      case BVSB_PhaseLoopBw_eMedium:
         hab[2] |= 0x02;
         break;
         
      case BVSB_PhaseLoopBw_eHigh:
         hab[2] |= 0x04;
         break;
         
      default:
         return (BERR_TRACE(BERR_INVALID_PARAMETER));
   }
   
   if ((pParams->ifFreqOffset < -2097152) || (pParams->ifFreqOffset > 2097151))
      return (BERR_TRACE(BERR_INVALID_PARAMETER));
      
   if ((pParams->symbolRateOffset < -32768) || (pParams->symbolRateOffset > 32767))
      return (BERR_TRACE(BERR_INVALID_PARAMETER));
   
   hab[4] = (pParams->ifFreqOffset >> 24);
   hab[5] = (pParams->ifFreqOffset >> 16);
   hab[6] = (pParams->ifFreqOffset >> 8);
   hab[7] = (pParams->ifFreqOffset & 0xFF);
   
   hab[8] = (pParams->symbolRateOffset >> 8);
   hab[9] = (pParams->symbolRateOffset & 0xFF);

   BVSB_CHK_RETCODE(BVSB_3560_P_SendHabCommand(h, hab, 11, hab, 1, true));

   done:
   return retCode;
}


/******************************************************************************
 BVSB_3560_P_AcquireQam()
******************************************************************************/
BERR_Code BVSB_3560_P_AcquireQam(
   BVSB_Handle h,                    /* [in] BVSB handle */
   const BVSB_InbandParams *pParams  /* [in] inband acquisition parameters */
)
{
   BERR_Code retCode;
   uint8_t   hab[16];

   hab[0] = 0x0B;
   hab[1] = hab[2] = hab[0x0A] = 0x00;
   hab[3] = 0x03;
   
   hab[1] |= ((h->settings.qam.bAutoAcq) ? 0x20 : 0x00);
   hab[1] |= ((h->settings.qam.bFastAcq) ? 0x01 : 0x00);
   hab[1] |= ((h->settings.qam.bSpinv) ? 0x10 : 0x00);
   hab[1] |= ((h->settings.qam.bEq) ? 0x04 : 0x00);
   hab[1] |= ((h->settings.qam.bCh) ? 0x02 : 0x00);
   hab[2] |= ((h->settings.qam.bTei) ? 0x01 : 0x00);
   hab[2] |= ((h->settings.qam.bTerr) ? 0x80 : 0x00);
   hab[2] |= ((pParams->mode >= BVSB_InbandMode_e256QAM_A) ? 0x00 : 0x08);
   hab[3] |= ((h->settings.qam.bDavic) ? 0x08 : 0x00);
   hab[3] |= (((uint8_t)(h->settings.qam.idepth) << 4) & 0xF0);

   switch (pParams->mode)
   {
      case BVSB_InbandMode_e1024QAM_B:
         hab[2] |= 0x70;
         break;
            
      case BVSB_InbandMode_e256QAM_B:
         hab[2] |= 0x50;
         break;

      case BVSB_InbandMode_e64QAM_B:
         hab[2] |= 0x30;
         break;

      case BVSB_InbandMode_e256QAM_A:
         hab[2] |= 0x50;
         break;

      case BVSB_InbandMode_e128QAM_A:
         hab[2] |= 0x40;
         break;

      case BVSB_InbandMode_e64QAM_A:
         hab[2] |= 0x30;
         break;

      case BVSB_InbandMode_e32QAM_A:
         hab[2] |= 0x20;
         break;

      case BVSB_InbandMode_e16QAM_A:
         hab[2] |= 0x10;
         break;

      default:
         return (BERR_TRACE(BERR_INVALID_PARAMETER));
   }

   switch (h->settings.qam.bw)
   {
      case BVSB_PhaseLoopBw_eLow:
         /*hab[2] |= 0x00;*/
         break;
         
      case BVSB_PhaseLoopBw_eMedium:
         hab[2] |= 0x02;
         break;
         
      case BVSB_PhaseLoopBw_eHigh:
         hab[2] |= 0x04;
         break;
         
      default:
         return (BERR_TRACE(BERR_INVALID_PARAMETER));
   }
   
   switch (h->settings.qam.nyquist)
   {
      case BVSB_NyquistFilter_e12:
         /*hab[1] |= 0x00;*/
         break;

      case BVSB_NyquistFilter_e15:
         hab[1] |= 0x40;
         break;

      case BVSB_NyquistFilter_e18:
         hab[1] |= 0x80;
         break;

      default:
         return (BERR_TRACE(BERR_INVALID_PARAMETER));
   }

   if ((pParams->ifFreqOffset < -2097152) || (pParams->ifFreqOffset > 2097151))
      return (BERR_TRACE(BERR_INVALID_PARAMETER));
      
   if ((pParams->symbolRateOffset < -32768) || (pParams->symbolRateOffset > 32767))
      return (BERR_TRACE(BERR_INVALID_PARAMETER));
   
   hab[4] = (pParams->ifFreqOffset >> 24);
   hab[5] = (pParams->ifFreqOffset >> 16);
   hab[6] = (pParams->ifFreqOffset >> 8);
   hab[7] = (pParams->ifFreqOffset & 0xFF);
   
   hab[8] = (pParams->symbolRateOffset >> 8);
   hab[9] = (pParams->symbolRateOffset & 0xFF);

   BVSB_CHK_RETCODE(BVSB_3560_P_SendHabCommand(h, hab, 11, hab, 1, true));

   done:
   return retCode;
}


/******************************************************************************
 BVSB_3560_P_AcquireNtsc()
******************************************************************************/
BERR_Code BVSB_3560_P_AcquireNtsc(
   BVSB_Handle h,                    /* [in] BVSB handle */
   const BVSB_InbandParams *pParams  /* [in] inband acquisition parameters */
)
{
   BERR_Code retCode;
   uint8_t   hab[9];

   hab[0] = 0x0C;
   hab[1] = hab[2] = hab[8] = 0x00;
   hab[3] = 0x01;
   
   hab[1] |= ((h->settings.ntsc.bFastAcq) ? 0x01 : 0x00);
   hab[1] |= ((h->settings.ntsc.bAutoAcq) ? 0x20 : 0x00);
   hab[2] |= ((h->settings.ntsc.bTerr) ? 0x80 : 0x00);

   switch (h->settings.ntsc.bw)
   {
      case BVSB_PhaseLoopBw_eLow:
         /*hab[2] |= 0x00;*/
         break;
         
      case BVSB_PhaseLoopBw_eMedium:
         hab[2] |= 0x02;
         break;
         
      case BVSB_PhaseLoopBw_eHigh:
         hab[2] |= 0x04;
         break;
         
      default:
         return (BERR_TRACE(BERR_INVALID_PARAMETER));
   }
   
   if ((pParams->ifFreqOffset < -2097152) || (pParams->ifFreqOffset > 2097151))
      return BERR_INVALID_PARAMETER;
      
   hab[4] = (pParams->ifFreqOffset >> 24);
   hab[5] = (pParams->ifFreqOffset >> 16);
   hab[6] = (pParams->ifFreqOffset >> 8);
   hab[7] = (pParams->ifFreqOffset & 0xFF);
   
   BVSB_CHK_RETCODE(BVSB_3560_P_SendHabCommand(h, hab, 9, hab, 1, true));

   done:
   return retCode;
}


/******************************************************************************
 BVSB_3560_P_ReadMemory()
******************************************************************************/
BERR_Code BVSB_3560_P_ReadMemory(BVSB_Handle h, uint16_t addr, uint8_t *buf, uint16_t n)
{
   BVSB_3560_P_Handle *p3560 = (BVSB_3560_P_Handle *)(h->pImpl);
   uint16_t i;

   if (((uint32_t)addr + (uint32_t)n) > 0x10000)
      return BERR_TRACE(BERR_INVALID_PARAMETER);

   for (i = 0; i < n; i++)
   {
      buf[i] = (uint8_t)(BREG_Read32(p3560->hRegister, BVSB_AP_INT_RAM+((i+addr)*4)));
   }

   return BERR_SUCCESS;
}


/******************************************************************************
 BVSB_3560_P_WriteMemory()
******************************************************************************/
BERR_Code BVSB_3560_P_WriteMemory(BVSB_Handle h, uint16_t addr, uint8_t *buf, uint16_t n)
{
   BVSB_3560_P_Handle *p3560 = (BVSB_3560_P_Handle *)(h->pImpl);
   uint16_t i;

   if ((addr >= 0x8000) || (n >= 0x8000))
      return BERR_TRACE(BERR_INVALID_PARAMETER);
   if ((addr + n) > 0x8000)
      return BERR_TRACE(BERR_INVALID_PARAMETER);

   for (i = 0; i < n; i++)
   {
      BREG_Write32(p3560->hRegister, BVSB_AP_INT_RAM+((i+addr)*4), (uint32_t)(buf[i]));
   }

   return BERR_SUCCESS;
}

