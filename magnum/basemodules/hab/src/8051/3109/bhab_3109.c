/***************************************************************************
 *     Copyright (c) 2003-2013, Broadcom Corporation
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
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 ***************************************************************************/

#include "bhab.h"
#include "bhab_priv.h"
#include "bhab_3109.h"
#include "bhab_31xx_priv.h"

BDBG_MODULE(bhab_3109);


static const BHAB_Settings defDevSettings =
  {
    0x66, /* chipAddr */
    NULL, /* interruptEnableFunc */
    NULL, /* interruptEnableFuncParam */
    /* API function table */
    {
        BHAB_31xx_Open,
        BHAB_31xx_Close,
        BHAB_31xx_InitAp,
        BHAB_31xx_GetApStatus,
        BHAB_31xx_GetApVersion,
        NULL, /* BHAB_GetVersionInfo */
        BHAB_31xx_ReadRegister,
        BHAB_31xx_WriteRegister,
        BHAB_31xx_ReadMemory,
        BHAB_31xx_WriteMemory,
        BHAB_31xx_ReadMbox,
        BHAB_31xx_WriteMbox,
        BHAB_31xx_HandleInterrupt_isr,
        BHAB_31xx_ProcessInterruptEvent,
        BHAB_31xx_EnableLockInterrupt,
        BHAB_31xx_InstallInterruptCallback,
        BHAB_31xx_UnInstallInterruptCallback,
        BHAB_31xx_SendHabCommand,
        BHAB_31xx_GetInterruptEventHandle,
        BHAB_31xx_GetWatchDogTimer,
        BHAB_31xx_SetWatchDogTimer,
        BHAB_31xx_GetNmiConfig,
        BHAB_31xx_SetNmiConfig,
        BHAB_31xx_GetConfigSettings,
        BHAB_31xx_SetConfigSettings,
        NULL, /* BHAB_ReadSlave */
        NULL, /* BHAB_WriteSlave */
        NULL, /* BHAB_GetInternalGain */
        NULL, /* BHAB_GetExternalGain */
        NULL, /* BHAB_SetExternalGain */
        NULL, /* BHAB_GetAvsData */
        NULL, /* BHAB_GetTunerChannels */
        NULL, /* BHAB_GetCapabilities */
        BHAB_31xx_Reset,
        NULL, /* BHAB_GetRecalibrateSettings */
        NULL, /* BHAB_SetRecalibrateSettings */
        NULL /* BHAB_GetLnaStatus */
        },
        0x60, /* slaveChipAddr */
        false, /* isSpi */
        false, /* isMtsif */
        NULL, /* pImgInterface */
        NULL, /* pImgContext */
        NULL  /* pChp */
  };


/******************************************************************************
 BHAB_3109_GetDefaultSettings()
******************************************************************************/
BERR_Code BHAB_3109_GetDefaultSettings(
	BHAB_Settings *pDefSettings /* [out] default settings */
)
{
	*pDefSettings = defDevSettings;
	return BERR_SUCCESS;
}
