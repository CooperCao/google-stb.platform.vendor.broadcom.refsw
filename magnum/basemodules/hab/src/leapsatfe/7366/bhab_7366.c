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
#include "bhab_7366.h"
#include "bhab_7366_priv.h"
#include "bchp_leap_ctrl.h"
#include "bchp_leap_hab_mem.h"


BDBG_MODULE(bhab_7366);


static const BHAB_Settings defDevSettings =
{
    0,    /* chipAddr */
    NULL, /* interruptEnableFunc */
    NULL, /* interruptEnableFuncParam */
    /* API function table */
    {
        BHAB_7366_P_Open,
        BHAB_7366_P_Close,
        BHAB_7366_P_InitAp,
        BHAB_7366_P_GetApStatus,
        NULL, /* BHAB_7366_P_GetApVersion */
        BHAB_7366_P_GetVersionInfo,
#if BHAB_CHIP==4548
        BHAB_4548_P_ReadRegister,
        BHAB_4548_P_WriteRegister,
#else
        BHAB_7366_P_ReadRegister_isrsafe,
        BHAB_7366_P_WriteRegister_isrsafe,
#endif
        BHAB_7366_P_ReadMemory,
        BHAB_7366_P_WriteMemory,
        NULL, /* BHAB_ReadMbox */
        NULL, /* BHAB_WriteMbox */
        BHAB_7366_P_HandleInterrupt_isr,
#if BHAB_CHIP==4548
        BHAB_4548_P_ProcessInterruptEvent,
#else
        NULL,
#endif
        NULL, /* BHAB_EnableLockInterrupt */
        BHAB_7366_P_InstallInterruptCallback,
        BHAB_7366_P_UnInstallInterruptCallback,
        BHAB_7366_P_SendHabCommand,
#if BHAB_CHIP==4548
        BHAB_7366_P_GetInterruptEventHandle,
#else
        NULL,
#endif
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
        NULL, /* BHAB_7366_P_GetAvsData */
        NULL, /* BHAB_GetTunerChannels */
        NULL, /* BHAB_GetCapabilities */
        BHAB_7366_P_Reset,
        NULL,
        NULL,
        NULL /* BHAB_GetLnaStatus */
    },
    0,     /* slaveChipAddr */
    false, /* isSpi */
    false, /* isMtsif */
    NULL, /* pImgInterface */
    NULL, /* pImgContext */
    NULL  /* pChp */
};


/******************************************************************************
 BHAB_7366_GetDefaultSettings()
******************************************************************************/
BERR_Code BHAB_7366_GetDefaultSettings(
   BHAB_Settings *pDefSettings /* [out] default settings */
)
{
   *pDefSettings = defDevSettings;
   return BERR_SUCCESS;
}


/******************************************************************************
 BHAB_7366_P_InitHeader()
******************************************************************************/
uint32_t BHAB_7366_InitHeader(uint8_t cmd, uint8_t chn, uint8_t dir, uint8_t module)
{
   uint32_t header = (uint32_t)cmd;
   header |= (uint32_t)(chn & 0xF) << 8;
   header |= (uint32_t)(dir & 1) << 12;
   header |= (uint32_t)(module & 0x7) << 13;
   return header;
}


/******************************************************************************
 BHAB_7366_PrintUart()
******************************************************************************/
BERR_Code BHAB_7366_PrintUart(BHAB_Handle h, char *pStr)
{
   uint32_t hab[128], i, n;

   for (n = 0; n < 124; n++)
   {
      if (pStr[n] == 0)
         break;
   }

   if ((n == 0) || (n >= 124))
      return BERR_INVALID_PARAMETER;

   hab[0] = BHAB_7366_InitHeader(0x03, 0, 0, 0);
   hab[1] = n;
   hab[1] |= 0x80000000;
   for (i = 0; i < n; i++)
      hab[2+i] = (uint32_t)pStr[i];
   return BHAB_7366_P_SendCommand(h, hab, 3+n);
}


/******************************************************************************
 BHAB_7366_Configure()
******************************************************************************/
BERR_Code BHAB_7366_Configure(BHAB_Handle h, BHAB_7366_Settings *pSettings)
{
   BHAB_7366_P_Handle *pImpl = (BHAB_7366_P_Handle *)(h->pImpl);

   if (pSettings->bUseInternalMemory == false)
   {
      if ((pSettings->physAddr & 0xFFFFF) || (pSettings->pRam == NULL))
         return BERR_INVALID_PARAMETER;
   }

   BKNI_Memcpy((void*)&(pImpl->settings), (void*)pSettings, sizeof(BHAB_7366_Settings));

   pImpl->habBaseAddr = BCHP_LEAP_HAB_MEM_WORDi_ARRAY_BASE;

#if BHAB_CHIP!=4548
   if (pImpl->settings.bUseInternalMemory)
   {
      BDBG_MSG(("LEAP: using internal memory"));
   }
   else
   {
#if BCHP_VER==BCHP_VER_A0
      pImpl->habBaseAddr = (uint32_t)(pImpl->settings.pRam) + 0x40000;
#endif
      BDBG_MSG(("LEAP RAM: physAddr=0x%08X, virtAddr=0x%08X", pImpl->settings.physAddr, pImpl->settings.pRam));
      BREG_Write32(pImpl->hReg, BCHP_LEAP_CTRL_ADDR_TRANS, (pImpl->settings.physAddr & 0xFFF00000) | 0x1);
   }
#endif

   return BERR_SUCCESS;
}


