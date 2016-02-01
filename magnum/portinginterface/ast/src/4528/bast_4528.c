/***************************************************************************
 *     Copyright (c) 2003-2012, Broadcom Corporation
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
#include "bast.h"
#include "bchp_hsi.h"
#include "bchp_leap_prog_mem.h"
#include "bast_priv.h"
#include "bast_4528.h"
#include "bast_4528_priv.h"


BDBG_MODULE(bast_4528);


static const BAST_Settings defDevSettings =
{
   {  /* host i2c settings */
      0x69, /* chipAddr */
      NULL, /* interruptEnableFunc */
      NULL  /* interruptEnableFuncParam */
   },
   { /* API function table */
      BAST_4528_P_Open,
      BAST_4528_P_Close,
      BAST_4528_P_GetTotalChannels,
      BAST_4528_GetChannelDefaultSettings,
      BAST_4528_P_OpenChannel,
      BAST_4528_P_CloseChannel,
      BAST_4528_P_GetDevice,
      BAST_4528_P_InitAp,
      BAST_4528_P_SoftReset,
      BAST_4528_P_GetApStatus,
      BAST_4528_P_GetApVersion,
      NULL, /* BAST_SetTmConfig, */
      NULL, /* BAST_GetTmConfig, */
      BAST_4528_P_ConfigGpio,
      BAST_4528_P_SetGpio,
      BAST_4528_P_GetGpio,
      BAST_4528_P_TuneAcquire,
      BAST_4528_P_GetChannelStatus,
      BAST_4528_P_GetLockStatus,
      BAST_4528_P_ResetStatus,
      BAST_4528_P_SetDiseqcTone,
      BAST_4528_P_GetDiseqcTone,
      BAST_4528_P_SetDiseqcVoltage,
      BAST_4528_P_SendDiseqcCommand,
      BAST_4528_P_GetDiseqcStatus,
      BAST_4528_P_ResetDiseqc,
      BAST_4528_P_ResetFsk,
      BAST_4528_P_ReadFsk,
      BAST_4528_P_WriteFsk,
      BAST_4528_P_PowerDownFsk,
      BAST_4528_P_PowerUpFsk,
      BAST_4528_P_WriteMi2c,
      BAST_4528_P_ReadMi2c,
      BAST_4528_P_GetSoftDecisionBuf,
      NULL, /* BAST_4528_P_ReadAgc, */
      NULL, /* BAST_4528_P_WriteAgc, */
      NULL, /* BAST_4528_P_FreezeAgc, */
      BAST_4528_P_FreezeEq,
      BAST_4528_P_PowerDown,
      BAST_4528_P_PowerUp,     
      BAST_4528_P_ReadRegister,
      BAST_4528_P_WriteRegister,
      BAST_4528_P_ReadConfig,
      BAST_4528_P_WriteConfig,
#ifdef BAST_USE_HAB_PI
      NULL,
#else
      BAST_4528_P_GetInterruptEventHandle,
#endif
      BAST_4528_P_GetLockStateChangeEventHandle,
      BAST_4528_P_GetFskEventHandle,
      BAST_4528_P_GetDiseqcEventHandle,
#ifdef BAST_USE_HAB_PI
      NULL,
      NULL,
#else
      BAST_4528_P_HandleInterrupt_isr,
      BAST_4528_P_ProcessInterruptEvent,
#endif
      BAST_4528_P_AbortAcq,
      NULL, /* BAST_ConfigLna */
      NULL, /* BAST_GetLnaStatus */
      NULL, /* BAST_ConfigAgc */
      NULL, /* BAST_4528_P_SendACW */
      BAST_4528_P_GetDiseqcVoltage,
      BAST_4528_P_GetDiseqcVsenseEventHandles,
      BAST_4528_P_EnableVsenseInterrupts,
      BAST_4528_P_PeakScan,
      BAST_4528_P_GetPeakScanStatus,
      BAST_4528_P_GetPeakScanEventHandle,
      BAST_4528_P_EnableStatusInterrupts,
      BAST_4528_P_GetStatusEventHandle,
      BAST_4528_P_ConfigBcm3445,
      BAST_4528_P_MapBcm3445ToTuner,
      BAST_4528_P_GetBcm3445Status,
      NULL, /* BAST_4528_P_EnableSpurCanceller */
      BAST_4528_P_ResetChannel, 
      BAST_4528_P_EnableDiseqcLnb, 
      BAST_4528_P_SetSearchRange,
      BAST_4528_P_GetSearchRange,
      BAST_4528_P_SetAmcScramblingSeq,
      BAST_4528_P_SetTunerFilter,
      BAST_4528_P_GetSignalDetectStatus,
      BAST_4528_P_SetOutputTransportSettings,
      BAST_4528_P_GetOutputTransportSettings,
      BAST_4528_P_SetDiseqcSettings,
      BAST_4528_P_GetDiseqcSettings, 
      BAST_4528_P_SetNetworkSpec,
      BAST_4528_P_GetNetworkSpec,
      NULL, /* BAST_TunerConfigLna */
      NULL, /* BAST_TunerGetLnaStatus */
      NULL, /* BAST_SetFskChannel */
      NULL, /* BAST_GetFskChannel */
      NULL, /* BAST_4528_P_SetPeakScanSymbolRateRange, */
      NULL, /* BAST_4528_P_GetPeakScanSymbolRateRange */
      NULL, /* BAST_SetAdcSelect */
      NULL, /* BAST_GetAdcSelect */
      NULL  /* BAST_GetVersionInfo */
   },
   BAST_NetworkSpec_eDefault
};


static const BAST_ChannelSettings defChnSettings =
{
   (uint8_t)0
};


/******************************************************************************
 BAST_4528_GetDefaultSettings()
******************************************************************************/
BERR_Code BAST_4528_GetDefaultSettings(
   BAST_Settings *pDefSettings /* [out] default settings */
)
{
   *pDefSettings = defDevSettings;
   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_4528_P_GetChannelDefaultSettings()
******************************************************************************/
BERR_Code BAST_4528_GetChannelDefaultSettings(
   BAST_Handle   h,                      /* [in] BAST handle */
   uint32_t      chnNo,                      /* [in] channel number */
   BAST_ChannelSettings *pChnDefSettings /* [out] default channel settings */
)
{
   BSTD_UNUSED(h);
   BSTD_UNUSED(chnNo);
   *pChnDefSettings = defChnSettings;
   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_4528_PrintUart()
******************************************************************************/
BERR_Code BAST_4528_PrintUart(BAST_Handle h, char *pBuf)
{
#ifdef BAST_USE_HAB_PI
   BAST_4528_P_Handle *pImpl = (BAST_4528_P_Handle *)(h->pImpl);
#endif      
   uint8_t len, buf[128], n;

   buf[0] = 0x0C;
   for (len = 0; len < 124; len++)
   {
      buf[len+2] = pBuf[len];
      if (pBuf[len] == 0)
         break;
   }
   buf[1] = len; 

   n = (uint8_t)(3+len);
#ifdef BAST_USE_HAB_PI
   return BHAB_SendHabCommand(pImpl->hHab, buf, n, buf, n, true, true, n);
#else      
   return BAST_4528_P_SendHabCommand(h, buf, n, buf, true);
#endif
}


/******************************************************************************
 BAST_4528_ConfigXbert()
******************************************************************************/
BERR_Code BAST_4528_ConfigXbert(BAST_Handle h, uint8_t ctrl)
{
#ifdef BAST_USE_HAB_PI
   BAST_4528_P_Handle *pImpl = (BAST_4528_P_Handle *)(h->pImpl);
#endif         
   uint8_t buf[4];

   buf[0] = 0x28;
   buf[1] = ctrl;   
#ifdef BAST_USE_HAB_PI
   return BHAB_SendHabCommand(pImpl->hHab, buf, 3, buf, 3, true, true, 3);
#else         
   return BAST_4528_P_SendHabCommand(h, buf, 3, buf, true);
#endif
}


/******************************************************************************
 BAST_4528_EnableAvs()
******************************************************************************/
BERR_Code BAST_4528_EnableAvs(BAST_Handle h, bool bEnable)
{
#ifdef BAST_USE_HAB_PI
   BAST_4528_P_Handle *pImpl = (BAST_4528_P_Handle *)(h->pImpl);
#endif            
   uint8_t buf[4];

   buf[0] = 0x36;
   buf[1] = bEnable ? 1 : 0;
#ifdef BAST_USE_HAB_PI
      return BHAB_SendHabCommand(pImpl->hHab, buf, 3, buf, 3, true, true, 3);
#else            
   return (BAST_4528_P_SendHabCommand(h, buf, 3, buf, true));
#endif
}


/******************************************************************************
 BAST_4528_GetAvsStatus()
******************************************************************************/
BERR_Code BAST_4528_GetAvsStatus(BAST_Handle h, BAST_AvsStatus *pStatus)
{
#ifdef BAST_USE_HAB_PI
   BAST_4528_P_Handle *pImpl = (BAST_4528_P_Handle *)(h->pImpl);
#endif               
   BERR_Code retCode;
   uint8_t buf[12];

   buf[0] = 0x29;
#ifdef BAST_USE_HAB_PI
   BAST_4528_CHK_RETCODE(BHAB_SendHabCommand(pImpl->hHab, buf, 11, buf, 11, true, true, 11));
#else               
   BAST_4528_CHK_RETCODE(BAST_4528_P_SendHabCommand(h, buf, 11, buf, true));
#endif

   pStatus->bEnabled = (buf[1] & 0x01) ? true : false;
   pStatus->temperature = (uint32_t)((buf[2] << 24) | (buf[3] << 16) | (buf[4] << 8) | buf[5]);
   pStatus->voltage_1p1_0 = (uint32_t)((buf[6] << 24) | (buf[7] << 16) | (buf[8] << 8) | buf[9]);

   done:
   return retCode;
}


/******************************************************************************
 BAST_4528_WriteFlashSector()
******************************************************************************/
BERR_Code BAST_4528_WriteFlashSector(BAST_Handle h, uint32_t addr)
{
#ifdef BAST_USE_HAB_PI
   BAST_4528_P_Handle *pImpl = (BAST_4528_P_Handle *)(h->pImpl);
#endif               
   BERR_Code retCode;
   uint8_t buf[12];

   buf[0] = 0x37;
   buf[1] = (uint8_t)((addr >> 24) & 0xFF);
   buf[2] = (uint8_t)((addr >> 16) & 0xFF);
   buf[3] = (uint8_t)((addr >> 8) & 0xFF);
   buf[4] = (uint8_t)(addr & 0xFF);
#ifdef BAST_USE_HAB_PI
   BAST_4528_CHK_RETCODE(BHAB_SendHabCommand(pImpl->hHab, buf, 6, buf, 6, true, true, 6));
#else               
   BAST_4528_CHK_RETCODE(BAST_4528_P_SendHabCommand(h, buf, 6, buf, true));
#endif

   done:
   return retCode;
}


#ifndef BAST_USE_HAB_PI
/******************************************************************************
 BAST_4528_WriteMemory() 
******************************************************************************/
BERR_Code BAST_4528_WriteMemory(BAST_Handle h, uint32_t start_addr, uint8_t *buf, uint32_t n)
{
   BERR_Code retCode = BERR_SUCCESS;
   uint32_t i, addr = start_addr, nWords, leftover;
   bool bIsRunning;
   uint8_t i2c_buf[5], csr_status;

   /* make sure memory address is 32-bit aligned */
   if ((n == 0) || (start_addr & 0x03))
      retCode = BERR_INVALID_PARAMETER;

   BAST_4528_CHK_RETCODE(BAST_4528_ReadBbsi(h,  BCHP_CSR_STATUS, &csr_status, 1));
   bIsRunning = (csr_status & BCHP_CSR_STATUS_CPU_RUNNING_MASK) ? true : false;
   if (bIsRunning && (addr <= (BCHP_LEAP_PROG_MEM_WORDi_ARRAY_BASE+(BCHP_LEAP_PROG_MEM_WORDi_ARRAY_END<<2))))
      return BAST_ERR_MEMAV; /* cannot write to instruction memory while running */

   leftover = n % 4;
   nWords = (n >> 2);
   
   /* wait for !busy or error */
   BAST_4528_CHK_RETCODE(BAST_4528_P_WaitForBbsiDone(h));
     
   i2c_buf[0] = 0x00; /* BCHP_CSR_CONFIG: 32-bit writes */
   i2c_buf[1] = (uint8_t)((start_addr >> 24) & 0xFF);
   i2c_buf[2] = (uint8_t)((start_addr >> 16) & 0xFF);
   i2c_buf[3] = (uint8_t)((start_addr >> 8) & 0xFF);
   i2c_buf[4] = (uint8_t)(start_addr & 0xFF);
   BAST_4528_CHK_RETCODE(BAST_4528_WriteBbsi(h, BCHP_CSR_CONFIG, i2c_buf, 5));

   for (i = 0; i < nWords; i++)
   {
      /* wait for !busy or error */
      BAST_4528_CHK_RETCODE(BAST_4528_P_WaitForBbsiDone(h));

      if (bIsRunning)
      {
         i2c_buf[0] = buf[i<<2];
         i2c_buf[1] = buf[(i<<2)+1];
         i2c_buf[2] = buf[(i<<2)+2];
         i2c_buf[3] = buf[(i<<2)+3];
         BAST_4528_CHK_RETCODE(BAST_4528_WriteBbsi(h, BCHP_CSR_RBUS_DATA0, i2c_buf, 4));
      }
      else
      {
         /* we can write entire buf in one transaction if CPU not running */
         if (nWords > 0)
         {
            BAST_4528_CHK_RETCODE(BAST_4528_WriteBbsi(h, BCHP_CSR_RBUS_DATA0, buf, nWords<<2));
         }
         break;
      }
   }

   if (leftover)
   {
      /* pad with zeros */
      i2c_buf[0] = buf[(nWords<<2)];
      i2c_buf[1] = (leftover > 1) ? buf[(nWords<<2)+1] : 0;
      i2c_buf[2] = (leftover > 2) ? buf[(nWords<<2)+2] : 0;
      i2c_buf[3] = 0;
      BAST_4528_CHK_RETCODE(BAST_4528_WriteBbsi(h, BCHP_CSR_RBUS_DATA0, i2c_buf, 4));      
   }
   
   done:
   return retCode;
}


/******************************************************************************
 BAST_4528_ReadMemory() 
******************************************************************************/
BERR_Code BAST_4528_ReadMemory(BAST_Handle h, uint32_t start_addr, uint8_t *buf, uint32_t n)
{
   BERR_Code retCode;
   uint32_t i, j, buf_idx, addr, end_addr, nReads, x, val32;
   bool bIsRunning, bCsrAddrSet = false;
   uint8_t i2c_buf[5], csr_status = 0;

   if (n == 0)
      return BERR_INVALID_PARAMETER;

   BAST_4528_CHK_RETCODE(BAST_4528_ReadBbsi(h, BCHP_CSR_STATUS, &csr_status, 1));
   bIsRunning = (csr_status & BCHP_CSR_STATUS_CPU_RUNNING_MASK) ? true : false;

   addr = start_addr & ~0x03; /* actual starting address is 32-bit aligned */
   end_addr = start_addr + n;
   if (end_addr & 0x03)
      end_addr = (end_addr & ~0x03) + 4;
   nReads = (end_addr - addr) >> 2; /* total number of 32-bit reads */
   
   for (i = buf_idx = 0; i < nReads; i++)
   {
      /* wait for !busy or error */
      BAST_4528_CHK_RETCODE(BAST_4528_P_WaitForBbsiDone(h));

      if (i == 0)
      {
         x = start_addr & 0x03;
         if (x)
         {
            /* starting address is not 32-bit aligned */
            BAST_4528_CHK_RETCODE(BAST_4528_ReadRbus(h, addr, &val32, 1));
            if (x == 3)
               buf[buf_idx++] = (uint8_t)(val32 & 0xFF);
            else if (x == 2)
            {
               buf[buf_idx++] = (uint8_t)((val32 >> 8) & 0xFF);
               buf[buf_idx++] = (uint8_t)(val32 & 0xFF);
            }
            else /* x==1 */
            {
               buf[buf_idx++] = (uint8_t)((val32 >> 16) & 0xFF);
               buf[buf_idx++] = (uint8_t)((val32 >> 8) & 0xFF);
               buf[buf_idx++] = (uint8_t)(val32 & 0xFF);
            }
            addr += 4;
            continue;
         }
      }

      if (!bCsrAddrSet)
      {
         i2c_buf[0] = (i == (nReads-1)) ? 0x01 : 0x03; /* BCHP_CSR_CONFIG: 32-bit reads */
         i2c_buf[1] = (uint8_t)((addr >> 24) & 0xFF);
         i2c_buf[2] = (uint8_t)((addr >> 16) & 0xFF);
         i2c_buf[3] = (uint8_t)((addr >> 8) & 0xFF);
         i2c_buf[4] = (uint8_t)(addr & 0xFF);
         BAST_4528_CHK_RETCODE(BAST_4528_WriteBbsi(h, BCHP_CSR_CONFIG, i2c_buf, 5));
         bCsrAddrSet = true;
      }

      BAST_4528_CHK_RETCODE(BAST_4528_ReadBbsi(h, BCHP_CSR_RBUS_DATA0, i2c_buf, 4));
      for (j = 0; (j < 4) && (buf_idx < n); j++)
         buf[buf_idx++] = i2c_buf[j];
   }

   done:
   return retCode;
}


/******************************************************************************
 BAST_4528_WriteBbsi()
******************************************************************************/
BERR_Code BAST_4528_WriteBbsi(BAST_Handle h, uint8_t addr, uint8_t *buf, uint32_t n)
{
   return BREG_I2C_Write(((BAST_4528_P_Handle *)(h->pImpl))->hRegister, h->settings.i2c.chipAddr, addr, buf, n);
}


/******************************************************************************
 BAST_4528_ReadBbsi()
******************************************************************************/
BERR_Code BAST_4528_ReadBbsi(BAST_Handle h, uint8_t addr, uint8_t *buf, uint32_t n)
{
   return BREG_I2C_Read(((BAST_4528_P_Handle *)(h->pImpl))->hRegister, h->settings.i2c.chipAddr, addr, buf, n);
}


/******************************************************************************
 BAST_4528_ReadRbus()
******************************************************************************/
BERR_Code BAST_4528_ReadRbus(BAST_Handle h, uint32_t addr, uint32_t *buf, uint32_t n)
{
   BERR_Code retCode = BERR_SUCCESS;
   uint32_t i;
   uint8_t i2c_buf[5];

   if (n == 0)
      return BERR_INVALID_PARAMETER;

   for (i = 0; i < n; i++)
   {
      /* set up the starting address to read */
      if (i == 0)
      {
         i2c_buf[0] = (n > 1) ? 0x03 : 0x05; /* BCHP_CSR_CONFIG */
         i2c_buf[1] = (uint8_t)((addr >> 24) & 0xFF);
         i2c_buf[2] = (uint8_t)((addr >> 16) & 0xFF);
         i2c_buf[3] = (uint8_t)((addr >> 8) & 0xFF);
         i2c_buf[4] = (uint8_t)(addr & 0xFF);
         BAST_4528_CHK_RETCODE(BAST_4528_WriteBbsi(h, BCHP_CSR_CONFIG, i2c_buf, 5));
      }

      /* wait for !busy or error */
      BAST_4528_CHK_RETCODE(BAST_4528_P_WaitForBbsiDone(h));

      BAST_4528_CHK_RETCODE(BAST_4528_ReadBbsi(h, BCHP_CSR_RBUS_DATA0, i2c_buf, 4));
      buf[i] = (uint32_t)((i2c_buf[0] << 24) | (i2c_buf[1] << 16) | (i2c_buf[2] << 8) | i2c_buf[3]);
   }

   done:
   return retCode;
}


/******************************************************************************
 BAST_4528_WriteRbus()
******************************************************************************/
BERR_Code BAST_4528_WriteRbus(BAST_Handle h, uint32_t addr, uint32_t *buf, uint32_t n)
{
   BERR_Code retCode = BERR_SUCCESS; 
   uint32_t i, data_idx;
   uint8_t i2c_buf[9];

   if (n == 0)
      retCode = BERR_INVALID_PARAMETER;

   for (i = 0; i < n; i++)
   {
      /* wait for !busy or error */
      BAST_4528_CHK_RETCODE(BAST_4528_P_WaitForBbsiDone(h));      
   
      if (i == 0)
      {
         i2c_buf[0] = 0; /* BCHP_CSR_CONFIG */
         i2c_buf[1] = (uint8_t)((addr >> 24) & 0xFF);
         i2c_buf[2] = (uint8_t)((addr >> 16) & 0xFF);
         i2c_buf[3] = (uint8_t)((addr >> 8) & 0xFF);
         i2c_buf[4] = (uint8_t)(addr & 0xFF);
         data_idx = 5;
      }
      else
         data_idx = 0;

      i2c_buf[data_idx++] = (uint8_t)((buf[i] >> 24) & 0xFF);
      i2c_buf[data_idx++] = (uint8_t)((buf[i] >> 16) & 0xFF);
      i2c_buf[data_idx++] = (uint8_t)((buf[i] >> 8) & 0xFF);
      i2c_buf[data_idx++] = (uint8_t)(buf[i] & 0xFF);

      BAST_4528_CHK_RETCODE(BAST_4528_WriteBbsi(h, (uint8_t)((data_idx == 4) ? BCHP_CSR_RBUS_DATA0 : BCHP_CSR_CONFIG), i2c_buf, data_idx));
   }

   done:
   return retCode;
}
#endif

