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
 * [File Description:]
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 ***************************************************************************/

#include "bhab.h"
#include "bhab_priv.h"
#include "bhab_4517.h"
#include "bhab_4517_priv.h"

BDBG_MODULE(bhab_4517);


static const BHAB_Settings defDevSettings =
{
    0x68, /* chipAddr */
    NULL, /* interruptEnableFunc */
    NULL, /* interruptEnableFuncParam */
    /* API function table */
    {
        BHAB_4517_P_Open,
        BHAB_4517_P_Close,
        BHAB_4517_P_InitAp,
        BHAB_4517_P_GetApStatus,
        BHAB_4517_P_GetApVersion,
        BHAB_4517_P_GetVersionInfo,
        BHAB_4517_P_ReadRegister,
        BHAB_4517_P_WriteRegister,
        BHAB_4517_P_ReadMemory,
        BHAB_4517_P_WriteMemory,
        NULL, /* BHAB_4517_P_ReadMbox */
        NULL, /* BHAB_4517_P_WriteMbox */
        BHAB_4517_P_HandleInterrupt_isr,
        BHAB_4517_P_ProcessInterruptEvent,
        NULL, /* BHAB_4517_P_EnableLockInterrupt */
        BHAB_4517_P_InstallInterruptCallback,
        BHAB_4517_P_UnInstallInterruptCallback,
        BHAB_4517_P_SendHabCommand,
        BHAB_4517_P_GetInterruptEventHandle,
        NULL, /* BHAB_4517_P_GetWatchDogTimer */
        NULL, /* BHAB_4517_P_SetWatchDogTimer */
        NULL, /* BHAB_4517_P_GetNmiConfig */
        NULL, /* BHAB_4517_P_SetNmiConfig */
        NULL, /* BHAB_4517_P_GetConfigSettings */
        NULL, /* BHAB_4517_P_SetConfigSettings */
        NULL, /* BHAB_ReadSlave */
        NULL, /* BHAB_WriteSlave */
        NULL, /* BHAB_GetInternalGain */
        NULL, /* BHAB_GetExternalGain */
        NULL, /* BHAB_SetExternalGain */
        NULL, /* BHAB_GetAvsData */
        NULL, /* BHAB_GetTunerChannels */
        NULL, /* BHAB_GetCapabilities */
        BHAB_4517_P_Reset,
        NULL, /* BHAB_GetRecalibrateSettings */
        NULL, /* BHAB_SetRecalibrateSettings */
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
 BHAB_4517_GetDefaultSettings()
******************************************************************************/
BERR_Code BHAB_4517_GetDefaultSettings(
   BHAB_Settings *pDefSettings /* [out] default settings */
)
{
   *pDefSettings = defDevSettings;
   return BERR_SUCCESS;
}
