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
 * Module Description:
 *
 * [File Description:]
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 ***************************************************************************/
#include "bhab.h"
#include "bhab_priv.h"
#include "bhab_34xx_priv.h"

BDBG_MODULE(bhab_3472);


static const BHAB_Settings defDevSettings =
{
    0x6c, /* chipAddr */
    NULL, /* interruptEnableFunc */
    NULL, /* interruptEnableFuncParam */
    /* API function table */
    {
        BHAB_34xx_Open,
        BHAB_34xx_Close,
        BHAB_34xx_InitAp,
        BHAB_34xx_GetApStatus,
        BHAB_34xx_GetApVersion,
        BHAB_34xx_GetVersionInfo,
        BHAB_34xx_ReadRegister,
        BHAB_34xx_WriteRegister,
        BHAB_34xx_ReadMemory,
        BHAB_34xx_WriteMemory,
        BHAB_34xx_ReadMbox,
        BHAB_34xx_WriteMbox,
        BHAB_34xx_HandleInterrupt_isr,
        BHAB_34xx_ProcessInterruptEvent,
        BHAB_34xx_EnableLockInterrupt,
        BHAB_34xx_InstallInterruptCallback,
        BHAB_34xx_UnInstallInterruptCallback,
        BHAB_34xx_SendHabCommand,
        BHAB_34xx_GetInterruptEventHandle,
        BHAB_34xx_GetWatchDogTimer,
        BHAB_34xx_SetWatchDogTimer,
        BHAB_34xx_GetNmiConfig,
        BHAB_34xx_SetNmiConfig,
        BHAB_34xx_GetConfigSettings,
        BHAB_34xx_SetConfigSettings,
        NULL, /* BHAB_ReadSlave */
        NULL, /* BHAB_WriteSlave */
        BHAB_34xx_GetInternalGain,
        BHAB_34xx_GetExternalGain,
        BHAB_34xx_SetExternalGain,
        BHAB_34xx_GetAvsData,
        BHAB_34xx_GetTunerChannels,
        BHAB_34xx_GetCapabilities,
        BHAB_34xx_Reset,
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
 BHAB_3472_GetDefaultSettings()
******************************************************************************/
BERR_Code BHAB_3472_GetDefaultSettings(
    BHAB_Settings *pDefSettings /* [out] default settings */
)
{
    *pDefSettings = defDevSettings;
    return BERR_SUCCESS;
}
