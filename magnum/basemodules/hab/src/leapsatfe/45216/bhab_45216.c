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
#include "bhab.h"
#include "bhab_priv.h"
#include "bhab_45216.h"
#include "bhab_45216_priv.h"
#include "bchp_45216_leap_ctrl.h"
#include "bchp_45216_leap_hab_mem.h"
#include "bchp_45216_leap_host_l2.h"
#include "bchp_45216_bspi.h"

BDBG_MODULE(bhab_45216);


static const BHAB_Settings defDevSettings =
{
    0x68, /* chipAddr */
    NULL, /* interruptEnableFunc */
    NULL, /* interruptEnableFuncParam */
    /* API function table */
    {
        BHAB_45216_P_Open,
        BHAB_45216_P_Close,
        BHAB_45216_P_InitAp,
        BHAB_45216_P_GetApStatus,
        NULL, /* BHAB_45216_P_GetApVersion */
        BHAB_45216_P_GetVersionInfo,
        BHAB_45216_P_ReadRegister,
        BHAB_45216_P_WriteRegister,
        BHAB_45216_P_ReadMemory,
        BHAB_45216_P_WriteMemory,
        NULL, /* BHAB_ReadMbox */
        NULL, /* BHAB_WriteMbox */
        BHAB_45216_P_HandleInterrupt_isr,
        BHAB_45216_P_ProcessInterruptEvent,
        NULL, /* BHAB_EnableLockInterrupt */
        BHAB_45216_P_InstallInterruptCallback,
        BHAB_45216_P_UnInstallInterruptCallback,
        BHAB_45216_P_SendHabCommand,
        BHAB_45216_P_GetInterruptEventHandle,
        NULL, /* BHAB_GetWatchDogTimer */
        NULL, /* BHAB_SetWatchDogTimer */
        NULL, /* BHAB_GetNmiConfig */
        NULL, /* BHAB_SetNmiConfig */
        NULL, /* BHAB_GetConfigSettings */
        NULL, /* BHAB_SetConfigSettings */
        NULL, /* BHAB_ReadSlave */
        NULL, /* BHAB_WriteSlave */
        NULL, /* BHAB_GetInternalGain */
        NULL, /* BHAB_GetExternalGain */
        NULL, /* BHAB_SetExternalGain */
        BHAB_45216_P_GetAvsData,
        NULL, /* BHAB_GetTunerChannels */
        NULL, /* BHAB_GetCapabilities */
        BHAB_45216_P_Reset,
        NULL,
        NULL,
        NULL /* BHAB_GetLnaStatus */
    },
    0,     /* slaveChipAddr */
    false, /* isSpi */
    true,  /* isMtsif */
    NULL, /* pImgInterface */
    NULL, /* pImgContext */
    NULL  /* pChp */
};


/******************************************************************************
 BHAB_45216_GetDefaultSettings()
******************************************************************************/
BERR_Code BHAB_45216_GetDefaultSettings(
   BHAB_Settings *pDefSettings /* [out] default settings */
)
{
   *pDefSettings = defDevSettings;
   return BERR_SUCCESS;
}


/******************************************************************************
 BHAB_45216_P_InitHeader()
******************************************************************************/
uint32_t BHAB_45216_InitHeader(uint8_t cmd, uint8_t chn, uint8_t dir, uint8_t module)
{
   uint32_t header = (uint32_t)cmd & 0xFF;
   header |= (uint32_t)(chn & 0xFF) << 8;
   header |= (uint32_t)(module & 0x7F) << 16;
   header |= (uint32_t)(dir & 1) << 23;
   return header;
}


/******************************************************************************
 BHAB_45216_PrintUart()
******************************************************************************/
BERR_Code BHAB_45216_PrintUart(BHAB_Handle h, char *pStr)
{
   uint32_t hab[128], i, n;

   for (n = 0; n < 124; n++)
   {
      if (pStr[n] == 0)
         break;
   }

   if ((n == 0) || (n >= 124))
      return BERR_INVALID_PARAMETER;

   hab[0] = BHAB_45216_InitHeader(0x03, 0, 0, 0);
   hab[1] = n;
   hab[1] |= 0x80000000; /* insert NL/CR at end of string */
   for (i = 0; i < n; i++)
      hab[2+i] = (uint32_t)pStr[i];
   return BHAB_45216_P_SendCommand(h, hab, 3+n);
}


/******************************************************************************
 BHAB_45216_GetFlashEventHandle()
******************************************************************************/
BERR_Code BHAB_45216_GetFlashEventHandle(
   BHAB_Handle handle,            /* [in] BHAB handle */
   BKNI_EventHandle *hEvent       /* [out] flash operation done event handle */
)
{
   *hEvent = ((BHAB_45216_P_Handle *)(handle->pImpl))->hFlashDoneEvent;
   return BERR_SUCCESS;
}


/******************************************************************************
 BHAB_45216_WriteFlashSector()
******************************************************************************/
BERR_Code BHAB_45216_WriteFlashSector(
   BHAB_Handle h,    /* [in] BHAB handle */
   uint32_t    addr, /* [in] sector address in flash to program */
   uint8_t     *pData /* [in] 4096 bytes of data to write to sector */
)
{
   BERR_Code retCode;
   uint32_t mask, hab[4], bufAddr, i;
   uint8_t *sectorBuf = NULL;

   if (addr >= (512<<10))
      return BERR_INVALID_PARAMETER;

   sectorBuf = (uint8_t*)BKNI_Malloc(4<<10);
   if (sectorBuf == NULL)
      return BERR_OUT_OF_SYSTEM_MEMORY;

   /* get the address of the flash sector buf in LEAP memory */
   hab[0] = BHAB_45216_InitHeader(0x05, 0, BHAB_45216_READ, BHAB_45216_MODULE_LEAP);
   hab[1] = 6; /* FLASH_BUF_ADDR */
   BHAB_CHK_RETCODE(BHAB_45216_P_SendCommand(h, hab, 4));
   bufAddr = hab[2];

   /* write sector data */
   for (i = 0; i < (4<<10); i += 4)
   {
      sectorBuf[i+0] = pData[i+3];
      sectorBuf[i+1] = pData[i+2];
      sectorBuf[i+2] = pData[i+1];
      sectorBuf[i+3] = pData[i+0];
   }
   BHAB_CHK_RETCODE(BHAB_45216_P_WriteMemory(h, bufAddr, (const uint8_t*)sectorBuf, (4<<10)));

   /* enable the flash done interrupt */
   mask = BHAB_45216_HIRQ0_FLASH_DONE;
   BHAB_CHK_RETCODE(BHAB_45216_P_WriteRegister(h, BCHP_LEAP_HOST_L2_MASK_CLEAR0, &mask));

   /* initiate write to flash */
   hab[0] = BHAB_45216_InitHeader(0x08, 0, 0, 0);
   hab[1] = addr;
   retCode = BHAB_45216_P_SendCommand(h, hab, 3);

   done:
   BKNI_Free((void*)sectorBuf);
   return retCode;
}


/******************************************************************************
 BHAB_45216_ReadFlash()
******************************************************************************/
BERR_Code BHAB_45216_ReadFlash(
   BHAB_Handle h,    /* [in] BHAB handle */
   uint32_t    addr, /* [in] offset in flash */
   uint32_t    n,    /* [in] number of bytes to read */
   uint8_t     *pBuf /* [out] bytes read from flash */
)
{
   BERR_Code retCode;
   uint32_t val, flashStatus;

   /* check if flash is busy */
   BHAB_CHK_RETCODE(BHAB_45216_P_ReadRegister(h, BCHP_LEAP_CTRL_GP27, &flashStatus));
   if (flashStatus == 0x02000008)
      return BHAB_ERR_HAB_RESOURCE_BUSY;

   /* set BSPI to control SPI bus */
   val = 0;
   BHAB_CHK_RETCODE(BHAB_45216_P_WriteRegister(h, BCHP_BSPI_MAST_N_BOOT_CTRL, &val));

   retCode = BHAB_45216_P_ReadMemory(h, 0xC0000000 | addr, pBuf, n);

   done:
   return retCode;
}
