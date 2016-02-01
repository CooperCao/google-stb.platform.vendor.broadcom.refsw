/***************************************************************************
 *     Copyright (c) 2005-2010, Broadcom Corporation
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
#include "btnr_dtt760x.h"

BDBG_MODULE(btnr_dtt760x);

#define	DEV_MAGIC_ID			((BERR_TNR_ID<<16) | 0xFACE)

#define	CHK_RETCODE( rc, func )			\
do {										\
	if( (rc = BERR_TRACE(func)) != BERR_SUCCESS ) \
	{										\
		goto done;							\
	}										\
} while(0)


/*******************************************************************************
*
*	Private Module Handles
*
*******************************************************************************/


typedef struct BTNR_P_DTT760X_Settings
{
	unsigned long ifFreq;
	BREG_I2C_Handle hI2CReg;
	unsigned short i2cAddr;

	unsigned long rfFreq;
	BTNR_TunerMode tunerMode;
	bool isInitialized;
} BTNR_P_DTT760X_Settings;

typedef struct BTNR_P_DTT760X_Handle		*BTNR_DTT760X_Handle;

typedef struct BTNR_P_DTT760X_Handle
{
	uint32_t magicId;					/* Used to check if structure is corrupt */

	BTNR_P_DTT760X_Settings settings;
} BTNR_P_DTT760X_Handle;



/*******************************************************************************
*
*	Default Module Settings
*
*******************************************************************************/
static const BTNR_DTT760X_Settings defDevSettings =
{
	BTNR_DTT760X_SETTINGS_IFFREQ,
	BTNR_DTT760X_SETTINGS_I2CADDR
};

/*******************************************************************************
*
*	Private Module Data
*
*******************************************************************************/
static BERR_Code BTNR_DTT760X_Close(
	BTNR_Handle hDev					/* [in] Device handle */
	)
{
	BERR_Code retCode = BERR_SUCCESS;


	BDBG_ENTER(BTNR_DTT760X_Close);
	BDBG_ASSERT( hDev );
	BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

	hDev->magicId = 0x00;		/* clear it to catch inproper use */
	BKNI_Free( (void *) hDev->hDevImpl );
	BKNI_Free( (void *) hDev );

	BDBG_LEAVE(BTNR_DTT760X_Close);
	return( retCode );
}

static BERR_Code BTNR_DTT760X_SetRfFreq(
	BTNR_DTT760X_Handle hDev,				/* [in] Device handle */
	uint32_t rfFreq,					/* [in] Requested tuner freq., in Hertz */
    BTNR_TunerMode tunerMode			/* [in] Requested tuner mode */
    )
{
	BERR_Code retCode = BERR_SUCCESS;
	BTNR_P_DTT760X_Settings *pTnrImplData;
	uint8_t tnr[4];
	unsigned long n_cnt;
	
	BDBG_ENTER(BTNR_DTT760X_SetRfFreq);
	BDBG_ASSERT( hDev );
	BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

	pTnrImplData = &hDev->settings;
	if( pTnrImplData->isInitialized == false )
	{
		/* If required */
		pTnrImplData->isInitialized = true;
	}

	if( rfFreq < 136250001 )
	{
		tnr[3] = 0x09;
	}
	else if( rfFreq < 417250001 )
	{
		tnr[3] = 0x0a;
	}
	else
	{
		tnr[3] = 0x0c;
	}

	n_cnt = (rfFreq + pTnrImplData->ifFreq) / (1000000 / 16);
	
	tnr[0] = 0x7F & (n_cnt >> 8);
	tnr[1] = 0xFF & n_cnt;
	tnr[2] = 0x8E;

	if( tunerMode == BTNR_TunerMode_eAnalog )
	{
		tnr[3] &= 0x07;
	}

 	CHK_RETCODE( retCode, BREG_I2C_WriteNoAddr( pTnrImplData->hI2CReg, pTnrImplData->i2cAddr, tnr, sizeof(tnr) ) );

	tnr[2] = 0x9E;
	tnr[3] = 0x40;

 	CHK_RETCODE( retCode, BREG_I2C_WriteNoAddr( pTnrImplData->hI2CReg, pTnrImplData->i2cAddr, tnr, sizeof(tnr) ) );

	pTnrImplData->rfFreq = rfFreq;
	pTnrImplData->tunerMode = tunerMode;

done:
	BDBG_LEAVE(BTNR_DTT760X_SetRfFreq);
	return( retCode );
}

static BERR_Code BTNR_DTT760X_GetRfFreq(
	BTNR_DTT760X_Handle hDev,				/* [in] Device handle */
	uint32_t *rfFreq,					/* [output] Returns tuner freq., in Hertz */
	BTNR_TunerMode *tunerMode			/* [output] Returns tuner mode */
    )
{
	BERR_Code retCode = BERR_SUCCESS;
	BTNR_P_DTT760X_Settings *pTnrImplData;


	BDBG_ENTER(BTNR_DTT760X_GetRfFreq);
	BDBG_ASSERT( hDev );
	BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

	pTnrImplData = &hDev->settings;

	*rfFreq = pTnrImplData->rfFreq;
	*tunerMode = pTnrImplData->tunerMode;

	BDBG_LEAVE(BTNR_DTT760X_GetRfFreq);
	return( retCode );
}

/*******************************************************************************
*
*	Public Module Functions
*
*******************************************************************************/

BERR_Code BTNR_DTT760X_Open(
	BTNR_Handle *phDev,					/* [output] Returns handle */
	BCHP_Handle hChip,					/* [in] Chip handle */
	BREG_I2C_Handle hI2CReg,			/* [in] I2C Register handle */
	const BTNR_DTT760X_Settings *pDefSettings /* [in] Default settings */
	)
{
	BERR_Code retCode = BERR_SUCCESS;
 	BTNR_DTT760X_Handle hDTT760XDev;
	BTNR_P_DTT760X_Settings *pTnrImplData;
	BTNR_Handle hDev;


	BDBG_ENTER(BTNR_DTT760X_Open);
	BDBG_ASSERT( hI2CReg );
	BSTD_UNUSED(hChip);

	hDev = NULL;
	/* Alloc memory from the system heap */
	hDTT760XDev = (BTNR_DTT760X_Handle) BKNI_Malloc( sizeof( BTNR_P_DTT760X_Handle ) );
	if( hDTT760XDev == NULL )
	{
		retCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
		BDBG_ERR(("BTNR_DTT760X_Open: BKNI_malloc() failed\n"));
		goto done;
	}
	BKNI_Memset( hDTT760XDev, 0x00, sizeof( BTNR_P_DTT760X_Handle ) );

	hDev = (BTNR_Handle) BKNI_Malloc( sizeof( BTNR_P_Handle ) );
	if( hDev == NULL )
	{
		retCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
		BDBG_ERR(("BTNR_DTT760X_Open: BKNI_malloc() failed\n"));
		BKNI_Free( hDTT760XDev );
		goto done;
	}
	BKNI_Memset( hDev, 0x00, sizeof( BTNR_P_Handle ) );

	hDTT760XDev->magicId = DEV_MAGIC_ID;
	pTnrImplData = &hDTT760XDev->settings;
	pTnrImplData->hI2CReg = hI2CReg;
	pTnrImplData->ifFreq = pDefSettings->ifFreq;
	pTnrImplData->i2cAddr = pDefSettings->i2cAddr;
	if( (pDefSettings->i2cAddr & 0x80) )
	{
		BDBG_ERR(("I2C Address must be 7 bit format"));
		BDBG_ASSERT( !(pDefSettings->i2cAddr & 0x80) );
	}

	pTnrImplData->rfFreq = 0;
	pTnrImplData->tunerMode = BTNR_TunerMode_eDigital;
	pTnrImplData->isInitialized = false;

	hDev->hDevImpl = (void *) hDTT760XDev;
	hDev->magicId = DEV_MAGIC_ID;
	hDev->pSetRfFreq = (BTNR_SetRfFreqFunc) BTNR_DTT760X_SetRfFreq;
	hDev->pGetRfFreq = (BTNR_GetRfFreqFunc) BTNR_DTT760X_GetRfFreq;
	hDev->pGetAgcRegVal = (BTNR_GetAgcRegValFunc) NULL;
	hDev->pSetAgcRegVal = (BTNR_SetAgcRegValFunc) NULL;
	hDev->pClose = (BTNR_CloseFunc) BTNR_DTT760X_Close;
	hDev->pGetPowerSaver = (BTNR_GetPowerSaverFunc)NULL;
	hDev->pSetPowerSaver = (BTNR_SetPowerSaverFunc)NULL;


done:
	*phDev = hDev;
	BDBG_LEAVE(BTNR_DTT760X_Open);
	return( retCode );
}

BERR_Code BTNR_DTT760X_GetDefaultSettings(
	BTNR_DTT760X_Settings *pDefSettings,	/* [output] Returns default setting */
	BCHP_Handle hChip					/* [in] Chip handle */
	)
{
	BERR_Code retCode = BERR_SUCCESS;

	
	BDBG_ENTER(BTNR_DTT760X_GetDefaultSettings);
	BSTD_UNUSED(hChip);

	*pDefSettings = defDevSettings;

	BDBG_LEAVE(BTNR_DTT760X_GetDefaultSettings);
	return( retCode );
}
