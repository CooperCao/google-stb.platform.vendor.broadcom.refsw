/******************************************************************************
* Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
*
* This program is the proprietary software of Broadcom and/or its licensors,
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
* Module Description:
*
*****************************************************************************/
#include "bhab.h"
#include "bhab_priv.h"
#include "bhab_45402.h"
#include "bhab_45402_priv.h"
#include "bchp_45402_cpu_mem.h"
#include "bchp_45402_leap_host_l2_0.h"
#include "bchp_45402_leap_host_l2_1.h"

BDBG_MODULE(bhab_45402);


static const BHAB_Settings defDevSettings =
{
    0x68, /* chipAddr */
    NULL, /* interruptEnableFunc */
    NULL, /* interruptEnableFuncParam */
    /* API function table */
    {
        BHAB_45402_P_Open,
        BHAB_45402_P_Close,
        BHAB_45402_P_InitAp,
        BHAB_45402_P_GetApStatus,
        NULL, /* BHAB_45402_P_GetApVersion */
        BHAB_45402_P_GetVersionInfo,
        BHAB_45402_P_ReadRegister,
        BHAB_45402_P_WriteRegister,
        BHAB_45402_P_ReadMemory,
        BHAB_45402_P_WriteMemory,
        NULL, /* BHAB_ReadMbox */
        NULL, /* BHAB_WriteMbox */
        BHAB_45402_P_HandleInterrupt_isr,
        BHAB_45402_P_ProcessInterruptEvent,
        NULL, /* BHAB_EnableLockInterrupt */
        BHAB_45402_P_InstallInterruptCallback,
        BHAB_45402_P_UnInstallInterruptCallback,
        BHAB_45402_P_SendHabCommand,
        BHAB_45402_P_GetInterruptEventHandle,
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
        NULL, /* BHAB_GetAvsData */
        NULL, /* BHAB_GetTunerChannels */
        NULL, /* BHAB_GetCapabilities */
        BHAB_45402_P_Reset,
        NULL,
        NULL,
        NULL /* BHAB_GetLnaStatus */
    },
    0,    /* slaveChipAddr */
    true, /* isSpi */
    true, /* isMtsif */
    NULL, /* pImgInterface */
    NULL, /* pImgContext */
    NULL  /* pChp */
};


/******************************************************************************
 BHAB_45402_GetDefaultSettings()
******************************************************************************/
BERR_Code BHAB_45402_GetDefaultSettings(
   BHAB_Settings *pDefSettings /* [out] default settings */
)
{
   *pDefSettings = defDevSettings;
   return BERR_SUCCESS;
}


/******************************************************************************
 BHAB_45402_P_InitHeader()
******************************************************************************/
uint32_t BHAB_45402_InitHeader(uint8_t cmd, uint8_t chn, uint8_t dir, uint8_t module)
{
   uint32_t header = (uint32_t)cmd & 0xFF;
   header |= (uint32_t)(chn & 0xFF) << 8;
   header |= (uint32_t)(module & 0x7F) << 16;
   header |= (uint32_t)(dir & 1) << 23;
   return header;
}


/******************************************************************************
 BHAB_45402_PrintUart()
******************************************************************************/
BERR_Code BHAB_45402_PrintUart(BHAB_Handle h, char *pStr)
{
   uint32_t hab[128], i, n;

   for (n = 0; n < 124; n++)
   {
      if (pStr[n] == 0)
         break;
   }

   if ((n == 0) || (n >= 124))
      return BERR_INVALID_PARAMETER;

   hab[0] = BHAB_45402_InitHeader(0x03, 0, 0, 0);
   hab[1] = n;
   hab[1] |= 0x80000000; /* insert NL/CR at end of string */
   for (i = 0; i < n; i++)
      hab[2+i] = (uint32_t)pStr[i];
   return BHAB_45402_P_SendCommand(h, hab, 3+n);
}


#if 0
/******************************************************************************
 BHAB_45402_BscWrite()
******************************************************************************/
BERR_Code BHAB_45402_BscWrite(
   BHAB_Handle h,        /* [in] BHAB handle */
   uint8_t channel,      /* [in] BSC channel, 0=BSCA, 1=BSCB, 2=BSCC */
   uint16_t slave_addr,  /* [in] for 7-bit address: bits[6:0]; for 10-bit address: bits[9:0], bit 15 is set  */
   uint8_t *i2c_buf,     /* [in] specifies the data to transmit */
   uint32_t n            /* [in] number of bytes to transmit after the i2c slave address, 0 to 8 */
)
{
   BERR_Code retCode;
   uint32_t hab[13], i;

   if ((n > 8) || (i2c_buf == NULL) || (channel > 2))
      return BERR_INVALID_PARAMETER;

   BKNI_Memset(hab, 0, 13*sizeof(uint32_t));
   hab[0] = BHAB_45402_InitHeader(0x40, channel, 0, 0);
   hab[2] = slave_addr | (n << 16);
   for (i = 0; i < n; i++)
      hab[3+i] = (uint32_t)i2c_buf[i];
   retCode = BHAB_45402_P_SendCommand(h, hab, 4+n);
   if (retCode == BERR_SUCCESS)
      retCode = hab[1];

   return retCode;
}


/******************************************************************************
 BHAB_45402_BscRead()
******************************************************************************/
BERR_Code BHAB_45402_BscRead(
   BHAB_Handle h,        /* [in] BHAB handle */
   uint8_t channel,      /* [in] BSC channel, 0=BSCA, 1=BSCB, 2=BSCC */
   uint16_t slave_addr,  /* [in] for 7-bit address: bits[6:0]; for 10-bit address: bits[9:0], bit 15 is set  */
   uint8_t *out_buf,     /* [in] specifies the data to transmit before the i2c restart condition */
   uint8_t out_n,        /* [in] number of bytes to transmit (<=8) before the i2c restart condition not including the i2c slave address */
   uint8_t *in_buf,      /* [out] stores the data read */
   uint32_t in_n         /* [in] number of bytes to read after the i2c restart condition */
)
{
   BERR_Code retCode = BERR_SUCCESS;
   uint32_t hab[21], i;

   if ((in_n > 8) || (in_n == 0) || (in_buf == NULL) || (out_n > 8) || ((out_buf == NULL) && out_n) || (channel > 2))
      return BERR_INVALID_PARAMETER;

   BKNI_Memset(hab, 0, 21*sizeof(uint32_t));
   hab[0] = BHAB_45402_InitHeader(0x41, channel, 0, 0);
   hab[2] = slave_addr | (out_n << 16) | (in_n << 24);
   for (i = 0; i < out_n; i++)
      hab[3+i] = (uint32_t)out_buf[i];

   retCode = BHAB_45402_P_SendCommand(h, hab, 4+in_n+out_n);
   if (retCode == BERR_SUCCESS)
   {
      retCode = hab[1];
      for (i = 0; i < in_n; i++)
         in_buf[i] = (uint8_t)(hab[3+out_n+i] & 0xFF);
   }

   return retCode;
}


/******************************************************************************
 BHAB_45402_GpioConfig()
******************************************************************************/
BERR_Code BHAB_45402_GpioConfig(BHAB_Handle h, uint32_t write_mask, uint32_t read_mask)
{
   uint32_t hab[4];

   if ((write_mask & ~0xFFFFF) || (read_mask & ~0xFFFFF))
      return BERR_INVALID_PARAMETER;

   hab[0] = BHAB_45402_InitHeader(0x43, 0, 0, 0);
   hab[1] = write_mask;
   hab[2] = read_mask;
   return BHAB_45402_P_SendCommand(h, hab, 4);
}


/******************************************************************************
 BHAB_45402_GpioWrite()
******************************************************************************/
BERR_Code BHAB_45402_GpioWrite(BHAB_Handle h, uint32_t pin_mask, uint32_t state_mask)
{
   uint32_t hab[4];

   if ((pin_mask & ~0xFFFFF) || (state_mask & ~0xFFFFF))
      return BERR_INVALID_PARAMETER;

   hab[0] = BHAB_45402_InitHeader(0x44, 0, 0, 0);
   hab[1] = pin_mask;
   hab[2] = state_mask;
   return BHAB_45402_P_SendCommand(h, hab, 4);
}


/******************************************************************************
 BHAB_45402_GpioRead()
******************************************************************************/
BERR_Code BHAB_45402_GpioRead(BHAB_Handle h, uint32_t pin_mask, uint32_t *pstate_mask)
{
   BERR_Code retCode;
   uint32_t hab[4];

   *pstate_mask = 0;
   if ((pin_mask & ~0xFFFFF) || (pstate_mask == NULL))
      return BERR_INVALID_PARAMETER;

   hab[0] = BHAB_45402_InitHeader(0x45, 0, 0, 0);
   hab[1] = pin_mask;
   retCode = BHAB_45402_P_SendCommand(h, hab, 4);
   if (retCode == BERR_SUCCESS)
      *pstate_mask = hab[2];

   return retCode;
}


/******************************************************************************
 BHAB_45402_GpoConfig()
******************************************************************************/
BERR_Code BHAB_45402_GpoConfig(BHAB_Handle h, uint32_t write_mask, uint32_t ctl_mask)
{
   uint32_t hab[4];

   if ((write_mask & ~0x7FFF) || (ctl_mask & ~0x7FFF))
      return BERR_INVALID_PARAMETER;

   hab[0] = BHAB_45402_InitHeader(0x46, 0, 0, 0);
   hab[1] = write_mask;
   hab[2] = ctl_mask;
   return BHAB_45402_P_SendCommand(h, hab, 4);
}


/******************************************************************************
 BHAB_45402_GpoWrite()
******************************************************************************/
BERR_Code BHAB_45402_GpoWrite(BHAB_Handle h, uint32_t pin_mask, uint32_t state_mask)
{
   uint32_t hab[4];

   if ((pin_mask & ~0x7FFF) || (state_mask & ~0x7FFF))
      return BERR_INVALID_PARAMETER;

   hab[0] = BHAB_45402_InitHeader(0x47, 0, 0, 0);
   hab[1] = pin_mask;
   hab[2] = state_mask;
   return BHAB_45402_P_SendCommand(h, hab, 4);
}
#endif
