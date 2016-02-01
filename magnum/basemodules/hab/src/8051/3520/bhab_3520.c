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
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/

#include "bhab.h"
#include "bhab_priv.h"
#include "bhab_3520.h"
#include "bhab_3520_priv.h"

BDBG_MODULE(bvsb_3520);


static const BHAB_Settings defDevSettings =
  {	
    0x0F, /* chipAddr */
    NULL, /* interruptEnableFunc */
    NULL, /* interruptEnableFuncParam */
    /* API function table */
    {
        BHAB_3520_Open,
        BHAB_3520_Close,
        BHAB_3520_InitAp, 
        BHAB_3520_GetApStatus,
        BHAB_3520_GetApVersion,
        NULL, /* BHAB_GetVersionInfo */
        BHAB_3520_ReadRegister,
        BHAB_3520_WriteRegister,
        BHAB_3520_ReadMemory,
        BHAB_3520_WriteMemory,
        BHAB_3520_ReadMbox,
        BHAB_3520_WriteMbox,
        BHAB_3520_HandleInterrupt_isr,
        BHAB_3520_ProcessInterruptEvent,
        BHAB_3520_EnableLockInterrupt, 
        BHAB_3520_InstallInterruptCallback, 
        BHAB_3520_UnInstallInterruptCallback,
        BHAB_3520_SendHabCommand,
        BHAB_3520_GetLockStateChangeEventHandle,
        BHAB_3520_GetInterruptEventHandle
    }
};


/******************************************************************************
 BHAB_3520_GetDefaultSettings()
******************************************************************************/
BERR_Code BHAB_3520_GetDefaultSettings(
	BHAB_Settings *pDefSettings /* [out] default settings */
)
{
	*pDefSettings = defDevSettings;
	return BERR_SUCCESS;
}
