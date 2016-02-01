/***************************************************************************
 *     Copyright (c) 2003-2014, Broadcom Corporation
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
#include "bhab_3128.h"
#include "bhab_312x_priv.h"

BDBG_MODULE(bhab_3128);


static const BHAB_Settings defDevSettings =
{
    0x66, /* chipAddr */
    NULL, /* interruptEnableFunc */
    NULL, /* interruptEnableFuncParam */
    /* API function table */
    {
        BHAB_312x_Open,
        BHAB_312x_Close,
        BHAB_312x_InitAp,
        BHAB_312x_GetApStatus,
        BHAB_312x_GetApVersion,
        BHAB_312x_GetVersionInfo,
        BHAB_312x_ReadRegister,
        BHAB_312x_WriteRegister,
        BHAB_312x_ReadMemory,
        BHAB_312x_WriteMemory,
        BHAB_312x_ReadMbox,
        BHAB_312x_WriteMbox,
        BHAB_312x_HandleInterrupt_isr,
        BHAB_312x_ProcessInterruptEvent,
        BHAB_312x_EnableLockInterrupt,
        BHAB_312x_InstallInterruptCallback,
        BHAB_312x_UnInstallInterruptCallback,
        BHAB_312x_SendHabCommand,
        BHAB_312x_GetInterruptEventHandle,
        BHAB_312x_GetWatchDogTimer,
        BHAB_312x_SetWatchDogTimer,
        BHAB_312x_GetNmiConfig,
        BHAB_312x_SetNmiConfig,
        BHAB_312x_GetConfigSettings,
        BHAB_312x_SetConfigSettings,
        BHAB_312x_ReadSlave,
        BHAB_312x_WriteSlave,
        BHAB_312x_GetInternalGain,
        BHAB_312x_GetExternalGain,
        BHAB_312x_SetExternalGain,
        BHAB_312x_GetAvsData,
        BHAB_312x_GetTunerChannels,
        BHAB_312x_GetCapabilities,
        BHAB_312x_Reset,
        BHAB_312x_GetRecalibrateSettings,
        BHAB_312x_SetRecalibrateSettings,
        BHAB_312x_GetLnaStatus
    },
    0x60, /* slaveChipAddr */
    false, /* isSpi */
    false, /* isMtsif */
    NULL, /* pImgInterface */
    NULL, /* pImgContext */
    NULL  /* pChp */
};

static const BHAB_RecalibrateSettings defRecalibrateSettings =
{
    {
        true,
        260,
        50
    }
};

/******************************************************************************
 BHAB_3128_GetDefaultSettings()
******************************************************************************/
BERR_Code BHAB_3128_GetDefaultSettings(
	BHAB_Settings *pDefSettings /* [out] default settings */
)
{
	*pDefSettings = defDevSettings;
	return BERR_SUCCESS;
}

/******************************************************************************
 BHAB_3128_GetDefaultRecalibrateSettings()
******************************************************************************/
BERR_Code BHAB_3128_GetDefaultRecalibrateSettings(
    BHAB_RecalibrateSettings *pRecalibrateSettings /* [out] default recalibrate settings */
)
{
	*pRecalibrateSettings = defRecalibrateSettings;
	return BERR_SUCCESS;
}
