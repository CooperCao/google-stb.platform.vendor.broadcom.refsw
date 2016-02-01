/***************************************************************************
 *     Copyright (c) 2005-2013, Broadcom Corporation
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
 * $brcm_Log: $
 *
 ***************************************************************************/

#include "bhab.h"
#include "bhab_priv.h"
#include "bhab_3461_priv.h"

BDBG_MODULE(bhab_3461);


static const BHAB_Settings defDevSettings =
{
    0x6c, /* chipAddr */
    NULL, /* interruptEnableFunc */
    NULL, /* interruptEnableFuncParam */
    /* API function table */
    {
        BHAB_3461_Open,
        BHAB_3461_Close,
        BHAB_3461_InitAp,
        BHAB_3461_GetApStatus,
        BHAB_3461_GetApVersion,
        BHAB_3461_GetVersionInfo,
        BHAB_3461_ReadRegister,
        BHAB_3461_WriteRegister,
        BHAB_3461_ReadMemory,
        BHAB_3461_WriteMemory,
        BHAB_3461_ReadMbox,
        BHAB_3461_WriteMbox,
        BHAB_3461_HandleInterrupt_isr,
        BHAB_3461_ProcessInterruptEvent,
        BHAB_3461_EnableLockInterrupt,
        BHAB_3461_InstallInterruptCallback,
        BHAB_3461_UnInstallInterruptCallback,
        BHAB_3461_SendHabCommand,
        BHAB_3461_GetInterruptEventHandle,
        BHAB_3461_GetWatchDogTimer,
        BHAB_3461_SetWatchDogTimer,
        BHAB_3461_GetNmiConfig,
        BHAB_3461_SetNmiConfig,
        BHAB_3461_GetConfigSettings,
        BHAB_3461_SetConfigSettings,
        NULL, /* BHAB_ReadSlave */
        NULL, /* BHAB_WriteSlave */
        BHAB_3461_GetInternalGain,
        BHAB_3461_GetExternalGain,
        BHAB_3461_SetExternalGain,
        BHAB_3461_GetAvsData,
        BHAB_3461_GetTunerChannels,
        BHAB_3461_GetCapabilities,
        BHAB_3461_Reset,
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
 BHAB_3461_GetDefaultSettings()
******************************************************************************/
BERR_Code BHAB_3461_GetDefaultSettings(
	BHAB_Settings *pDefSettings /* [out] default settings */
)
{
	*pDefSettings = defDevSettings;
	return BERR_SUCCESS;
}
