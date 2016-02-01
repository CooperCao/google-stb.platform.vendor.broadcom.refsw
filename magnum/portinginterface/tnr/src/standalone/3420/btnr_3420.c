/***************************************************************************
 *     Copyright (c) 2003-2011, Broadcom Corporation
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
#include "btnr_3420.h"
#include "btnr_3420_priv.h"

BDBG_MODULE(btnr_3420);

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

typedef struct BTNR_P_3420_Handle		*BTNR_3420_Handle;

typedef struct BTNR_P_3420_Handle
{
	uint32_t magicId;					/* Used to check if structure is corrupt */

	BTNR_P_3420_Settings settings;
} BTNR_P_3420_Handle;




/*******************************************************************************
*
*	Default Module Settings
*
*******************************************************************************/
static const BTNR_3420_Settings defDevSettings =
{
	BTNR_3420_SETTINGS_ANNEXB_IFFREQ,
	BTNR_3420_SETTINGS_XTALFREQ,
	BTNR_3420_SETTINGS_I2CADDR,
	BTNR_3420_SETTINGS_ENABLEAGGAIN,
};



/*******************************************************************************
*
*	Private Module Data
*
*******************************************************************************/



static BERR_Code BTNR_3420_Close(
	BTNR_Handle hDev					/* [in] Device handle */
	)
{
	BERR_Code retCode = BERR_SUCCESS;
	BTNR_3420_Handle btnr_3420_handle;
	BTNR_P_3420_Settings btnr_3420_settings;


	BDBG_ENTER(BTNR_3420_Close);
	BDBG_ASSERT( hDev );
	BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

	/* verify the handle is good before using it */
	btnr_3420_handle = (BTNR_3420_Handle) hDev->hDevImpl;
	btnr_3420_settings = btnr_3420_handle ->settings;

	hDev->magicId = 0x00;		/* clear it to catch inproper use */
	BKNI_Free( (void *) hDev->hDevImpl );
	BKNI_Free( (void *) hDev );

	BDBG_LEAVE(BTNR_3420_Close);
	return( retCode );
}

static BERR_Code BTNR_3420_SetRfFreq(
	BTNR_3420_Handle hDev,				/* [in] Device handle */
	uint32_t rfFreq,					/* [in] Requested tuner freq., in Hertz */
    BTNR_TunerMode tunerMode			/* [in] Requested tuner mode */
    )
{
	BERR_Code retCode = BERR_SUCCESS;
	BTNR_P_3420_Settings *pTnrImplData;
	
	BDBG_ENTER(BTNR_3420_SetRfFreq);
	BDBG_ASSERT( hDev );
	BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

	pTnrImplData = &hDev->settings;
	if( pTnrImplData->isInitialized == false )
	{
		/* If the Bcm3420 has never initialized, initialize the H/W */
		retCode = BTNR_P_3420_initialize( pTnrImplData );
		if( retCode != BERR_SUCCESS) goto done;
	}

	/* 3420 is a digital only tuner */
	if(tunerMode == BTNR_TunerMode_eDigital)
	{
		
		retCode = BTNR_P_3420_tune( pTnrImplData, rfFreq );
		if( retCode != BERR_SUCCESS) goto done;	

		pTnrImplData->rfFreq = rfFreq;
		pTnrImplData->tunerMode = tunerMode;
	}
	else
	{
		retCode = BERR_INVALID_PARAMETER;
	}

done:
	BDBG_LEAVE(BTNR_3420_SetRfFreq);
	return( retCode );
}
			
static BERR_Code BTNR_3420_GetRfFreq(
	BTNR_3420_Handle hDev,				/* [in] Device handle */
	uint32_t *rfFreq,					/* [output] Returns tuner freq., in Hertz */
	BTNR_TunerMode *tunerMode			/* [output] Returns tuner mode */
    )
{
	BERR_Code retCode = BERR_SUCCESS;
	BTNR_P_3420_Settings *pTnrImplData;


	BDBG_ENTER(BTNR_3420_GetRfFreq);
	BDBG_ASSERT( hDev );
	BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

	pTnrImplData = &hDev->settings;

	*rfFreq = pTnrImplData->rfFreq;
	*tunerMode = pTnrImplData->tunerMode;

	BDBG_LEAVE(BTNR_3420_GetRfFreq);
	return( retCode );
}

static BERR_Code BTNR_P_3420_GetAgcRegVal(
	BTNR_3420_Handle hDev,				/* [in] Device handle */
	uint32_t regOffset,					/* [in] AGC register offset */
	uint32_t *agcVal					/* [out] output value */
	)
{
	BERR_Code retCode = BERR_SUCCESS;
	BTNR_P_3420_Settings *pTnrImplData;


	BDBG_ENTER(BTNR_3420_GetAgcRegVal);
	BDBG_ASSERT( hDev );
	BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

	BSTD_UNUSED(regOffset);

	pTnrImplData = &hDev->settings;

	BDBG_ERR(("Not Implemented on this tuner"));
	*agcVal = 0;

	BDBG_LEAVE(BTNR_3420_GetAgcRegVal);
	return( retCode );
}

static BERR_Code BTNR_3420_GetInfo(
	BTNR_3420_Handle hDev,				/* [in] Device handle */
	BTNR_TunerInfo *tnrInfo				/* [out] Tuner information */
	)
{	
	BTNR_P_3420_Settings *pTnrImplData;
	BERR_Code retCode = BERR_SUCCESS;

	BDBG_ENTER(BTNR_3420_GetInfo);
	BDBG_ASSERT( hDev );
	BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

	pTnrImplData = &hDev->settings;
	tnrInfo->tunerMaker = BRCM_TUNER_MAKER_ID;
	tnrInfo->tunerId = 3420;
	tnrInfo->tunerMajorVer = pTnrImplData->iRevLetter;
	tnrInfo->tunerMinorVer = pTnrImplData->iRevNumber;

	BDBG_LEAVE(BTNR_3420_GetInfo);
	return( retCode );
}

/*******************************************************************************
*
*	Public Module Functions
*
*******************************************************************************/

BERR_Code BTNR_3420_Open(
	BTNR_Handle *phDev,					/* [output] Returns handle */
	BCHP_Handle hChip,					/* [in] Chip handle */
	BREG_I2C_Handle hI2CReg,			/* [in] I2C Register handle */
	const BTNR_3420_Settings *pDefSettings /* [in] Default settings */
	)
{
	BERR_Code retCode = BERR_SUCCESS;
 	BTNR_3420_Handle h3420Dev;
	BTNR_P_3420_Settings *pTnrImplData;
	BTNR_Handle hDev;


	BDBG_ENTER(BTNR_3420_Open);
	BDBG_ASSERT( hI2CReg );
	BSTD_UNUSED(hChip);

	hDev = NULL;
	/* Alloc memory from the system heap */
	h3420Dev = (BTNR_3420_Handle) BKNI_Malloc( sizeof( BTNR_P_3420_Handle ) );
	if( h3420Dev == NULL )
	{
		retCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
		BDBG_ERR(("BTNR_3420_Open: BKNI_malloc() failed\n"));
		goto done;
	}
	BKNI_Memset( h3420Dev, 0x00, sizeof( BTNR_P_3420_Handle ) );

	hDev = (BTNR_Handle) BKNI_Malloc( sizeof( BTNR_P_Handle ) );
	if( hDev == NULL )
	{
		retCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
		BDBG_ERR(("BTNR_3420_Open: BKNI_malloc() failed\n"));
		BKNI_Free( h3420Dev );
		goto done;
	}
	BKNI_Memset( hDev, 0x00, sizeof( BTNR_P_Handle ) );

	h3420Dev->magicId = DEV_MAGIC_ID;
	pTnrImplData = &h3420Dev->settings;
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
	pTnrImplData->tunerMode = BTNR_TunerMode_eLast;

	hDev->hDevImpl = (void *) h3420Dev;
	hDev->magicId = DEV_MAGIC_ID;
	hDev->pSetRfFreq = (BTNR_SetRfFreqFunc) BTNR_3420_SetRfFreq;
	hDev->pGetRfFreq = (BTNR_GetRfFreqFunc) BTNR_3420_GetRfFreq;
	hDev->pGetAgcRegVal = (BTNR_GetAgcRegValFunc) BTNR_P_3420_GetAgcRegVal;
	hDev->pSetAgcRegVal = (BTNR_SetAgcRegValFunc) NULL;
	hDev->pGetInfo = (BTNR_GetInfoFunc) BTNR_3420_GetInfo;
	hDev->pClose = (BTNR_CloseFunc) BTNR_3420_Close;
	hDev->pGetPowerSaver = (BTNR_GetPowerSaverFunc)NULL;
	hDev->pSetPowerSaver = (BTNR_SetPowerSaverFunc)NULL;


done:
	*phDev = hDev;
	BDBG_LEAVE(BTNR_3420_Open);
	return( retCode );
}

BERR_Code BTNR_3420_GetDefaultSettings(
	BTNR_3420_Settings *pDefSettings,	/* [output] Returns default setting */
	BCHP_Handle hChip					/* [in] Chip handle */
	)
{
	BERR_Code retCode = BERR_SUCCESS;

	
	BDBG_ENTER(BTNR_3420_GetDefaultSettings);
	BSTD_UNUSED(hChip);

	*pDefSettings = defDevSettings;

	BDBG_LEAVE(BTNR_3420_GetDefaultSettings);
	return( retCode );
}


