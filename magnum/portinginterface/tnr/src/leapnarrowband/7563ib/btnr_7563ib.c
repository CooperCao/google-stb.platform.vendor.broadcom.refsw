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
 * Revision History: $
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/
#include "bstd.h"
#include "bkni.h"
#include "btnr.h"
#include "btnr_priv.h"
#include "btnr_7563ib.h"
#include "btnr_7563ib_priv.h"

BDBG_MODULE(btnr_7563);

#define DEV_MAGIC_ID                ((BERR_TNR_ID<<16) | 0xFACE)
#define BTNR_7563_SETTINGS_I2CADDR  (0x6C)          /* 7bit addr */
#define BTNR_7563_MAX_CHANNELS      2

/*******************************************************************************/
BERR_Code BTNR_7563_Open(
    BTNR_Handle *phDev,                 /* [output] Returns handle */
    BTNR_7563_Settings *pSettings, /* [Input] settings structure */
    BHAB_Handle hHab                  /* [Input] Hab Handle */   
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BTNR_7563_Handle h7563Dev;
    BTNR_P_7563_Settings *pTnrImplData;
    BTNR_Handle hDev;

    BDBG_ENTER(BTNR_7563_Open);
    BDBG_ASSERT( hHab );
    
    hDev = NULL;
    /* Alloc memory from the system heap */
    h7563Dev = (BTNR_7563_Handle) BKNI_Malloc( sizeof( BTNR_P_7563_Handle ) );
    if( h7563Dev == NULL )
    {
        retCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        BDBG_ERR(("BTNR_7563_Open: BKNI_malloc() failed\n"));
        goto done;
    }
    BKNI_Memset( h7563Dev, 0x00, sizeof( BTNR_P_7563_Handle ) );

    hDev = (BTNR_Handle) BKNI_Malloc( sizeof( BTNR_P_Handle ) );
    if( hDev == NULL )
    {
        retCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        BDBG_ERR(("BTNR_7563_Open: BKNI_malloc() failed\n"));
        BKNI_Free( h7563Dev );
        goto done;
    }
    BKNI_Memset( hDev, 0x00, sizeof( BTNR_P_Handle ) );

    h7563Dev->magicId = DEV_MAGIC_ID;
    pTnrImplData = &h7563Dev->settings;
    pTnrImplData->rfFreq = 0;
    pTnrImplData->tunerMode = BTNR_TunerMode_eLast;
    h7563Dev->channelNo = pSettings->channelNo;
    h7563Dev->hHab = hHab;
    hDev->hDevImpl = (void *) h7563Dev;
    hDev->magicId = DEV_MAGIC_ID;
    hDev->pSetRfFreq = (BTNR_SetRfFreqFunc) BTNR_7563_SetRfFreq;
    hDev->pGetRfFreq = (BTNR_GetRfFreqFunc) BTNR_7563_GetRfFreq;
    hDev->pGetAgcRegVal = (BTNR_GetAgcRegValFunc) BTNR_P_7563_GetAgcRegVal;
    hDev->pSetAgcRegVal = (BTNR_SetAgcRegValFunc) BTNR_7563_SetAgcRegVal;
    hDev->pGetInfo = (BTNR_GetInfoFunc) BTNR_7563_GetInfo;
    hDev->pClose = (BTNR_CloseFunc) BTNR_7563_Close;
    hDev->pGetPowerSaver = (BTNR_GetPowerSaverFunc) BTNR_7563_GetPowerSaver;
    hDev->pSetPowerSaver = (BTNR_SetPowerSaverFunc) BTNR_7563_SetPowerSaver;
    hDev->pGetSettings = (BTNR_GetSettingsFunc) BTNR_7563_GetSettings;
    hDev->pSetSettings = (BTNR_SetSettingsFunc) BTNR_7563_SetSettings;
    
    if(pSettings->channelNo >= pSettings->mxChnNo)
    {
        retCode = BERR_TRACE(BERR_NOT_SUPPORTED); 
        BDBG_ERR(("Only %d TNR channels supported", pSettings->mxChnNo)); 
    }
    else
        *phDev = hDev;    
done:
    if( retCode != BERR_SUCCESS )
    {
        if( hDev != NULL )
        {
            BKNI_Free( hDev );
        }
        *phDev = NULL;
    }
    BDBG_LEAVE(BTNR_7563_Open);
    return( retCode );
}

BERR_Code BTNR_7563_GetDefaultSettings(
    BTNR_7563_Settings *pDefSettings  /* [out] Returns default setting */
    )
{
    BDBG_ASSERT(NULL != pDefSettings);

    BKNI_Memset(pDefSettings, 0, sizeof(*pDefSettings));
    pDefSettings->i2cAddr = BTNR_7563_SETTINGS_I2CADDR;
    pDefSettings->mxChnNo = BTNR_7563_MAX_CHANNELS;

    return BERR_SUCCESS;
}

