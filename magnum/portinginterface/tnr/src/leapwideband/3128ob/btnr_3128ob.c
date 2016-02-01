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
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ****************************************************************************/
#include "bstd.h"
#include "bkni.h"
#include "btnr.h"
#include "btnr_priv.h"
#include "btnr_3128ob.h"
#include "btnr_312xob_priv.h"

BDBG_MODULE(btnr_3128ob);

#define	DEV_MAGIC_ID					((BERR_TNR_ID<<16) | 0xFACE)
#define	BTNR_3128Ob_SETTINGS_I2CADDR	(0x66)			/* 7bit addr */
#define	BTNR_3128Ob_IF_FREQ	            44000000			/* 7bit addr */

/*******************************************************************************
*
*	Public Module Functions
*
*******************************************************************************/

BERR_Code BTNR_3128Ob_Open(
	BTNR_Handle *phDev,					/* [output] Returns handle */
	BTNR_3128Ob_Settings *pSettings, /* [Input] settings structure */
	BHAB_Handle hHab                  /* [Input] Hab Handle */   
	)
{
    BERR_Code retCode = BERR_SUCCESS;
    BTNR_312xOb_Handle h312xObDev;
    BTNR_P_312xOb_Settings *pTnrImplData;
    BTNR_Handle hDev;  
    uint16_t chipVer;    
    uint32_t familyId, chipId;    
    uint8_t apVer, minApVer;
    uint8_t hab[9] = HAB_MSG_HDR(BTNR_OOB_RF_INPUT_MODE, 0x4, BTNR_CORE_TYPE, BTNR_CORE_ID);

    BDBG_ENTER(BTNR_312xOb_Open);
    BDBG_ASSERT( hHab );	

    hDev = NULL;
    /* Alloc memory from the system heap */
    h312xObDev = (BTNR_312xOb_Handle) BKNI_Malloc( sizeof( BTNR_P_312xOb_Handle ) );
    if( h312xObDev == NULL )
    {
        retCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        BDBG_ERR(("BTNR_312x_Open: BKNI_malloc() failed\n"));
        goto done;
    }
    BKNI_Memset( h312xObDev, 0x00, sizeof( BTNR_P_312xOb_Handle ) );

    hDev = (BTNR_Handle) BKNI_Malloc( sizeof( BTNR_P_Handle ) );
    if( hDev == NULL )
    {
        retCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        BDBG_ERR(("BTNR_312xOb_Open: BKNI_malloc() failed\n"));
        BKNI_Free( h312xObDev );
        goto done;
    }
    BKNI_Memset( hDev, 0x00, sizeof( BTNR_P_Handle ) );

    h312xObDev->magicId = DEV_MAGIC_ID;
    h312xObDev->bPowerdown = true;    
    pTnrImplData = &h312xObDev->settings;
    pTnrImplData->rfFreq = 0;
    pTnrImplData->tunerMode = BTNR_TunerMode_eDigital;
    pTnrImplData->ifFreq = pSettings->ifFreq;

    h312xObDev->hHab = hHab;
    hDev->hDevImpl = (void *) h312xObDev;
    hDev->magicId = DEV_MAGIC_ID;
    hDev->pSetRfFreq = (BTNR_SetRfFreqFunc) BTNR_312xOb_SetRfFreq;
    hDev->pGetRfFreq = (BTNR_GetRfFreqFunc) BTNR_312xOb_GetRfFreq;
    hDev->pGetAgcRegVal = (BTNR_GetAgcRegValFunc) BTNR_P_312xOb_GetAgcRegVal;
    hDev->pSetAgcRegVal = (BTNR_SetAgcRegValFunc) BTNR_312xOb_SetAgcRegVal;
    hDev->pGetInfo = (BTNR_GetInfoFunc) BTNR_312xOb_GetInfo;
    hDev->pClose = (BTNR_CloseFunc) BTNR_312xOb_Close;
    hDev->pGetPowerSaver = (BTNR_GetPowerSaverFunc) BTNR_312xOb_GetPowerSaver;
    hDev->pSetPowerSaver = (BTNR_SetPowerSaverFunc) BTNR_312xOb_SetPowerSaver;
    hDev->pGetSettings = (BTNR_GetSettingsFunc) BTNR_312xOb_GetSettings;
    hDev->pSetSettings = (BTNR_SetSettingsFunc) BTNR_312xOb_SetSettings;      
    hDev->pVersionInfo = (BTNR_GetVersionInfoFunc) BTNR_312xOb_GetVersionInfo;      

    hab[4] = pSettings->inputMode;
    CHK_RETCODE(retCode, BHAB_SendHabCommand(h312xObDev->hHab, hab, 9, hab, 0, false, true, 9));
    
    CHK_RETCODE(retCode, BHAB_GetApVersion(h312xObDev->hHab, &familyId, &chipId, &chipVer, &apVer, &minApVer));

    if((chipId == 0x00) && (familyId == 0x3128))
        chipId = 0x3128;
        
    if((chipId == 0x3128) || (chipId == 0x3127) ||(chipId == 0x3148) || (chipId == 0x3147))
        *phDev = hDev;
    else
    {
        retCode = BERR_TRACE(BERR_NOT_SUPPORTED);
        BDBG_ERR(("AOB not supported on %x", chipId));        
    }
   
done:
    if( retCode != BERR_SUCCESS )
    {
        if( hDev != NULL )
        {
            BKNI_Free( hDev );
        }
        *phDev = NULL;
    }
    BDBG_LEAVE(BTNR_312xOb_Open);
    return( retCode );
}

BERR_Code BTNR_3128Ob_GetDefaultSettings(
	BTNR_3128Ob_Settings *pDefSettings	/* [out] Returns default setting */
	)
{
	BDBG_ASSERT(NULL != pDefSettings);

    BKNI_Memset(pDefSettings, 0, sizeof(*pDefSettings));
	pDefSettings->i2cAddr = BTNR_3128Ob_SETTINGS_I2CADDR;
	pDefSettings->ifFreq = BTNR_3128Ob_IF_FREQ;

	return BERR_SUCCESS;
}

