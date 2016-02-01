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
#include "btnr_3418.h"
#include "btnr_3418_smarttune.h"
#include "btnr_3418_regio.h"

BDBG_MODULE(btnr_3418);

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

typedef struct BTNR_P_3418_Handle		*BTNR_3418_Handle;

typedef struct BTNR_P_3418_Handle
{
	uint32_t magicId;					/* Used to check if structure is corrupt */

	BTNR_P_3418_Settings settings;
} BTNR_P_3418_Handle;



/*******************************************************************************
*
*	Default Module Settings
*
*******************************************************************************/
static const BTNR_3418_Settings defDevSettings =
{
	BTNR_3418_SETTINGS_IFFREQ,
	BTNR_3418_SETTINGS_XTALFREQ,
	BTNR_3418_SETTINGS_I2CADDR,
	BTNR_3418_SETTINGS_ENABLEAGGAIN
};



/*******************************************************************************
*
*	Private Module Data
*
*******************************************************************************/
static BERR_Code BTNR_3418_Close(
	BTNR_Handle hDev					/* [in] Device handle */
	)
{
	BERR_Code retCode = BERR_SUCCESS;
	BTNR_3418_Handle btnr_3418_handle;
	BTNR_P_3418_Settings btnr_3418_settings;


	BDBG_ENTER(BTNR_3418_Close);
	BDBG_ASSERT( hDev );
	BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

	/* verify the handle is good before using it */
	btnr_3418_handle = (BTNR_3418_Handle) hDev->hDevImpl;
	btnr_3418_settings = btnr_3418_handle ->settings;

	hDev->magicId = 0x00;		/* clear it to catch inproper use */
	BKNI_Free((void *) btnr_3418_settings.register_shadow);
	BKNI_Free( (void *) hDev->hDevImpl );
	BKNI_Free( (void *) hDev );

	BDBG_LEAVE(BTNR_3418_Close);
	return( retCode );
}

static BERR_Code BTNR_3418_SetRfFreq(
	BTNR_3418_Handle hDev,				/* [in] Device handle */
	uint32_t rfFreq,					/* [in] Requested tuner freq., in Hertz */
    BTNR_TunerMode tunerMode			/* [in] Requested tuner mode */
    )
{
	BERR_Code retCode = BERR_SUCCESS;
	BTNR_P_3418_Settings *pTnrImplData;

	BDBG_ENTER(BTNR_3418_SetRfFreq);
	BDBG_ASSERT( hDev );
	BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

	
	pTnrImplData = &hDev->settings;
	if( pTnrImplData->isInitialized == false )
	{
		/* If the Bcm3418 has never initialized, initialize the H/W */
		BTNR_P_3418_initialize( pTnrImplData );
	}

	if( tunerMode == BTNR_TunerMode_eDigital)
		BTNR_P_3418_initDigital( pTnrImplData );
	else if( tunerMode == BTNR_TunerMode_eAnalog)
		BTNR_P_3418_initAnalog( pTnrImplData );
	
	BTNR_P_3418_tune( pTnrImplData, rfFreq );

	pTnrImplData->rfFreq = rfFreq;
	pTnrImplData->tunerMode = tunerMode;

	BDBG_LEAVE(BTNR_3418_SetRfFreq);
	return( retCode );
}

static BERR_Code BTNR_3418_GetRfFreq(
	BTNR_3418_Handle hDev,				/* [in] Device handle */
	uint32_t *rfFreq,					/* [output] Returns tuner freq., in Hertz */
	BTNR_TunerMode *tunerMode			/* [output] Returns tuner mode */
    )
{
	BERR_Code retCode = BERR_SUCCESS;
	BTNR_P_3418_Settings *pTnrImplData;


	BDBG_ENTER(BTNR_3418_GetRfFreq);
	BDBG_ASSERT( hDev );
	BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

	pTnrImplData = &hDev->settings;

	*rfFreq = pTnrImplData->rfFreq;
	*tunerMode = pTnrImplData->tunerMode;

	BDBG_LEAVE(BTNR_3418_GetRfFreq);
	return( retCode );
}

static BERR_Code BTNR_3418_GetAgcRegVal(
	BTNR_3418_Handle hDev,				/* [in] Device handle */
	uint32_t regOffset,					/* [in] AGC register offset */
	uint32_t *agcVal					/* [out] output value */
	)
{
	BERR_Code retCode = BERR_SUCCESS;
	BTNR_P_3418_Settings *pTnrImplData;
	uint8_t bVal;


	BDBG_ENTER(BTNR_3418_GetAgcRegVal);
	BDBG_ASSERT( hDev );
	BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

	pTnrImplData = &hDev->settings;

	BTNR_P_3418_ReadReg( pTnrImplData, regOffset, &bVal );
	*agcVal = bVal;

	BDBG_LEAVE(BTNR_3418_GetAgcRegVal);
	return( retCode );
}

static BERR_Code BTNR_3418_GetInfo(
	BTNR_3418_Handle hDev,				/* [in] Device handle */
	BTNR_TunerInfo *tnrInfo				/* [out] Tuner information */
	)
{	
	BTNR_P_3418_Settings *pTnrImplData;
	BERR_Code retCode = BERR_SUCCESS;

	BDBG_ENTER(BTNR_3418_GetInfo);
	BDBG_ASSERT( hDev );
	BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

	pTnrImplData = &hDev->settings;
	tnrInfo->tunerMaker = BRCM_TUNER_MAKER_ID;
	tnrInfo->tunerId = 3418;
	tnrInfo->tunerMajorVer = pTnrImplData->iRevLetter;
	tnrInfo->tunerMinorVer = pTnrImplData->iRevNumber;

	BDBG_LEAVE(BTNR_3418_GetInfo);
	return( retCode );
}

/*******************************************************************************
*
*	Public Module Functions
*
*******************************************************************************/

BERR_Code BTNR_3418_Open(
	BTNR_Handle *phDev,					/* [output] Returns handle */
	BCHP_Handle hChip,					/* [in] Chip handle */
	BREG_I2C_Handle hI2CReg,			/* [in] I2C Register handle */
	const BTNR_3418_Settings *pDefSettings /* [in] Default settings */
	)
{
	BERR_Code retCode = BERR_SUCCESS;
 	BTNR_3418_Handle h3418Dev;
	BTNR_P_3418_Settings *pTnrImplData;
	BTNR_Handle hDev;
	unsigned char * pShadow;


	BDBG_ENTER(BTNR_3418_Open);
	BDBG_ASSERT( hI2CReg );
	BSTD_UNUSED(hChip);

	hDev = NULL;
	/* Alloc memory from the system heap */
	h3418Dev = (BTNR_3418_Handle) BKNI_Malloc( sizeof( BTNR_P_3418_Handle ) );
	if( h3418Dev == NULL )
	{
		retCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
		BDBG_ERR(("BTNR_3418_Open: BKNI_malloc() failed\n"));
		*phDev = NULL;
		goto done;
	}
	BKNI_Memset( h3418Dev, 0x00, sizeof( BTNR_P_3418_Handle ) );

	hDev = (BTNR_Handle) BKNI_Malloc( sizeof( BTNR_P_Handle ) );
	if( hDev == NULL )
	{
		retCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
		BDBG_ERR(("BTNR_3418_Open: BKNI_malloc() failed\n"));
		BKNI_Free( h3418Dev );
		*phDev = NULL;
		goto done;
	}
	BKNI_Memset( hDev, 0x00, sizeof( BTNR_P_Handle ) );

	pShadow = (unsigned char *) BKNI_Malloc( (sizeof(unsigned char) * BTNR_P_3418_GetRegNumber()) );
	if( pShadow == NULL )
	{
		retCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
		BDBG_ERR(("BTNR_3418_Open: BKNI_malloc() failed\n"));
		BKNI_Free( h3418Dev );
		BKNI_Free( hDev );
		*phDev = NULL;
		goto done;
	}
	BKNI_Memset( pShadow, 0x00, (sizeof(unsigned char) * BTNR_P_3418_GetRegNumber()));

	h3418Dev->magicId = DEV_MAGIC_ID;
	pTnrImplData = &h3418Dev->settings;
	pTnrImplData->register_shadow = pShadow;
	pTnrImplData->hI2CReg = hI2CReg;
	pTnrImplData->ifFreq = pDefSettings->ifFreq;
	pTnrImplData->xtalFreq = pDefSettings->xtalFreq;
	pTnrImplData->i2cAddr = pDefSettings->i2cAddr;
	if( (pDefSettings->i2cAddr & 0x80) )
	{
		BDBG_ERR(("I2C Address must be 7 bit format"));
		BDBG_ASSERT( !(pDefSettings->i2cAddr & 0x80) );
	}
	pTnrImplData->enableAgcGain = pDefSettings->enableAgcGain;

	pTnrImplData->rfFreq = 0;
	pTnrImplData->tunerMode = BTNR_TunerMode_eDigital;
	pTnrImplData->isInitialized = false;

	hDev->hDevImpl = (void *) h3418Dev;
	hDev->magicId = DEV_MAGIC_ID;
	hDev->pSetRfFreq = (BTNR_SetRfFreqFunc) BTNR_3418_SetRfFreq;
	hDev->pGetRfFreq = (BTNR_GetRfFreqFunc) BTNR_3418_GetRfFreq;
	hDev->pGetAgcRegVal = (BTNR_GetAgcRegValFunc) BTNR_3418_GetAgcRegVal;
	hDev->pSetAgcRegVal = (BTNR_SetAgcRegValFunc) NULL;
	hDev->pGetInfo = (BTNR_GetInfoFunc) BTNR_3418_GetInfo;
	hDev->pClose = (BTNR_CloseFunc) BTNR_3418_Close;
	hDev->pGetPowerSaver = (BTNR_GetPowerSaverFunc)NULL;
	hDev->pSetPowerSaver = (BTNR_SetPowerSaverFunc)NULL;
	
	*phDev = hDev;
	
done:
	BDBG_LEAVE(BTNR_3418_Open);
	return( retCode );
}

BERR_Code BTNR_3418_GetDefaultSettings(
	BTNR_3418_Settings *pDefSettings,	/* [output] Returns default setting */
	BCHP_Handle hChip					/* [in] Chip handle */
	)
{
	BERR_Code retCode = BERR_SUCCESS;

	
	BDBG_ENTER(BTNR_3418_GetDefaultSettings);
	BSTD_UNUSED(hChip);

	*pDefSettings = defDevSettings;

	BDBG_LEAVE(BTNR_3418_GetDefaultSettings);
	return( retCode );
}

/* Private functions for Docsis use: Do not for general use, therefore not documented */
void BTNR_3418_SetIf1AgcForceMode(
	BTNR_Handle hDev					/* [in] Device handle */
	)
{
	BTNR_P_3418_Settings *pTnrImplData;
	BTNR_3418_Handle h3418Dev;

	BDBG_ENTER(BTNR_3418_SetIf1AgcForceMode);
	BDBG_ASSERT( hDev );
	BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

	h3418Dev = hDev->hDevImpl;
	pTnrImplData = &h3418Dev->settings;
	if( pTnrImplData->iRevLetter == 1 ) /* Only valid for 3418 B0 */
	{
		BTNR_P_3418_Freeze( pTnrImplData );
		BKNI_Sleep(100);
	}
	BDBG_LEAVE(BTNR_3418_SetIf1AgcForceMode);
}

/* Private functions for Docsis use: Do not for general use, therefore not documented */
void BTNR_3418_SetIf1AgcAutoMode(
	BTNR_Handle hDev					/* [in] Device handle */
	)
{
	BTNR_P_3418_Settings *pTnrImplData;
	BTNR_3418_Handle h3418Dev;

	BDBG_ENTER(BTNR_3418_SetIf1AgcAutoMode);
	BDBG_ASSERT( hDev );
	BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

	h3418Dev = hDev->hDevImpl;
	pTnrImplData = &h3418Dev->settings;
	if( pTnrImplData->iRevLetter == 1 ) /* Only valid for 3418 B0 */
	{
  		/* Set Auto mode */
        BTNR_P_3418_UnFreeze( pTnrImplData ); 
	}
	BDBG_LEAVE(BTNR_3418_SetIf1AgcAutoMode);
}

