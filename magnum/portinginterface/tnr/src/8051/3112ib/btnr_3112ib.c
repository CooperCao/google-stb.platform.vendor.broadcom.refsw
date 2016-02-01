/***************************************************************************
 *     Copyright (c) 2003-2010, Broadcom Corporation
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
 ***************************************************************************/
#include "bstd.h"
#include "bkni.h"
#include "btnr.h"
#include "btnr_priv.h"
#include "btnr_3112ib.h"
#include "btnr_31xxib_priv.h"

BDBG_MODULE(btnr_3112ib);

#define	DEV_MAGIC_ID					((BERR_TNR_ID<<16) | 0xFACE)
#define	BTNR_3112Ib_SETTINGS_I2CADDR	(0x66)			/* 7bit addr */

/*******************************************************************************/
BERR_Code BTNR_3112Ib_Open(
	BTNR_Handle *phDev,					/* [output] Returns handle */
	BTNR_3112Ib_Settings *pSettings, /* [Input] settings structure */
	BHAB_Handle hHab                  /* [Input] Hab Handle */   
	)
{
	BERR_Code retCode = BERR_SUCCESS;
 	BTNR_31xxIb_Handle h31xxIbDev;
	BTNR_P_31xxIb_Settings *pTnrImplData;
	BTNR_Handle hDev;


	BDBG_ENTER(BTNR_3112Ib_Open);
	BDBG_ASSERT( hHab );
	BSTD_UNUSED( pSettings );
	
	hDev = NULL;
	/* Alloc memory from the system heap */
	h31xxIbDev = (BTNR_31xxIb_Handle) BKNI_Malloc( sizeof( BTNR_P_31xxIb_Handle ) );
	if( h31xxIbDev == NULL )
	{
		retCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
		BDBG_ERR(("BTNR_3112Ib_Open: BKNI_malloc() failed\n"));
		goto done;
	}
	BKNI_Memset( h31xxIbDev, 0x00, sizeof( BTNR_P_31xxIb_Handle ) );

	hDev = (BTNR_Handle) BKNI_Malloc( sizeof( BTNR_P_Handle ) );
	if( hDev == NULL )
	{
		retCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
		BDBG_ERR(("BTNR_3112Ib_Open: BKNI_malloc() failed\n"));
		BKNI_Free( h31xxIbDev );
		goto done;
	}
	BKNI_Memset( hDev, 0x00, sizeof( BTNR_P_Handle ) );

	h31xxIbDev->magicId = DEV_MAGIC_ID;
	pTnrImplData = &h31xxIbDev->settings;
	pTnrImplData->rfFreq = 0;
	pTnrImplData->tunerMode = BTNR_TunerMode_eLast;

	h31xxIbDev->hHab = hHab;
	hDev->hDevImpl = (void *) h31xxIbDev;
	hDev->magicId = DEV_MAGIC_ID;
	hDev->pSetRfFreq = (BTNR_SetRfFreqFunc) BTNR_31xxIb_SetRfFreq;
	hDev->pGetRfFreq = (BTNR_GetRfFreqFunc) BTNR_31xxIb_GetRfFreq;
	hDev->pGetAgcRegVal = (BTNR_GetAgcRegValFunc) BTNR_P_31xxIb_GetAgcRegVal;
    hDev->pSetAgcRegVal = (BTNR_SetAgcRegValFunc) BTNR_31xxIb_SetAgcRegVal;
	hDev->pGetInfo = (BTNR_GetInfoFunc) BTNR_31xxIb_GetInfo;
	hDev->pClose = (BTNR_CloseFunc) BTNR_31xxIb_Close;
	hDev->pGetPowerSaver = (BTNR_GetPowerSaverFunc) BTNR_31xxIb_GetPowerSaver;
	hDev->pSetPowerSaver = (BTNR_SetPowerSaverFunc) BTNR_31xxIb_SetPowerSaver;


done:
	*phDev = hDev;
	BDBG_LEAVE(BTNR_3112Ib_Open);
	return( retCode );
}

BERR_Code BTNR_3112Ib_GetDefaultSettings(
	BTNR_3112Ib_Settings *pDefSettings	/* [out] Returns default setting */
	)
{
	BDBG_ASSERT(NULL != pDefSettings);

    BKNI_Memset(pDefSettings, 0, sizeof(*pDefSettings));
	pDefSettings->i2cAddr = BTNR_3112Ib_SETTINGS_I2CADDR;

	return BERR_SUCCESS;
}

