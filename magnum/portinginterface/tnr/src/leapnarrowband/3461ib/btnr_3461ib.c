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
 * Module Description:
 *
 * Revision History:  $
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/
#include "bstd.h"
#include "bkni.h"
#include "btnr.h"
#include "btnr_priv.h"
#include "btnr_3461ib.h"
#include "btnr_3461ib_priv.h"

BDBG_MODULE(btnr_3461);

#define DEV_MAGIC_ID                    ((BERR_TNR_ID<<16) | 0xFACE)
#define BTNR_3461_SETTINGS_I2CADDR    (0x66)          /* 7bit addr */

/*******************************************************************************/
BERR_Code BTNR_3461_Open(
    BTNR_Handle *phDev,                 /* [output] Returns handle */
    BTNR_3461_Settings *pSettings, /* [Input] settings structure */
    BHAB_Handle hHab                  /* [Input] Hab Handle */   
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BTNR_3461_Handle h3461Dev;
    BTNR_P_3461_Settings *pTnrImplData;
    BTNR_Handle hDev;


    BDBG_ENTER(BTNR_3461_Open);
    BDBG_ASSERT( hHab );
    
    hDev = NULL;
    /* Alloc memory from the system heap */
    h3461Dev = (BTNR_3461_Handle) BKNI_Malloc( sizeof( BTNR_P_3461_Handle ) );
    if( h3461Dev == NULL )
    {
        retCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        BDBG_ERR(("BTNR_3461_Open: BKNI_malloc() failed\n"));
        goto done;
    }
    BKNI_Memset( h3461Dev, 0x00, sizeof( BTNR_P_3461_Handle ) );

    hDev = (BTNR_Handle) BKNI_Malloc( sizeof( BTNR_P_Handle ) );
    if( hDev == NULL )
    {
        retCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        BDBG_ERR(("BTNR_3461_Open: BKNI_malloc() failed\n"));
        BKNI_Free( h3461Dev );
        goto done;
    }
    BKNI_Memset( hDev, 0x00, sizeof( BTNR_P_Handle ) );

    h3461Dev->magicId = DEV_MAGIC_ID;
    pTnrImplData = &h3461Dev->settings;
    pTnrImplData->rfFreq = 0;
    pTnrImplData->tunerMode = BTNR_TunerMode_eLast;
    h3461Dev->channelNo = pSettings->channelNo;
    h3461Dev->hHab = hHab;
    hDev->hDevImpl = (void *) h3461Dev;
    hDev->magicId = DEV_MAGIC_ID;
    hDev->pSetRfFreq = (BTNR_SetRfFreqFunc) BTNR_3461_SetRfFreq;
    hDev->pGetRfFreq = (BTNR_GetRfFreqFunc) BTNR_3461_GetRfFreq;
    hDev->pGetAgcRegVal = (BTNR_GetAgcRegValFunc) BTNR_P_3461_GetAgcRegVal;
    hDev->pSetAgcRegVal = (BTNR_SetAgcRegValFunc) BTNR_3461_SetAgcRegVal;
    hDev->pGetInfo = (BTNR_GetInfoFunc) BTNR_3461_GetInfo;
    hDev->pClose = (BTNR_CloseFunc) BTNR_3461_Close;
    hDev->pGetPowerSaver = (BTNR_GetPowerSaverFunc) BTNR_3461_GetPowerSaver;
    hDev->pSetPowerSaver = (BTNR_SetPowerSaverFunc) BTNR_3461_SetPowerSaver;
    hDev->pGetSettings = (BTNR_GetSettingsFunc) BTNR_3461_GetSettings;
    hDev->pSetSettings = (BTNR_SetSettingsFunc) BTNR_3461_SetSettings;
    hDev->pVersionInfo = (BTNR_GetVersionInfoFunc) BTNR_3461_GetVersionInfo;     
	
done:
    *phDev = hDev;
    BDBG_LEAVE(BTNR_3461_Open);
    return( retCode );
}

BERR_Code BTNR_3461_GetDefaultSettings(
    BTNR_3461_Settings *pDefSettings  /* [out] Returns default setting */
    )
{
    BDBG_ASSERT(NULL != pDefSettings);

    BKNI_Memset(pDefSettings, 0, sizeof(*pDefSettings));
    pDefSettings->i2cAddr = BTNR_3461_SETTINGS_I2CADDR;

    return BERR_SUCCESS;
}

