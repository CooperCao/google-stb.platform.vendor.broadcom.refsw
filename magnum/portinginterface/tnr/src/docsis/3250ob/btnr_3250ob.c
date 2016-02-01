/***************************************************************************
 *     Copyright (c) 2003-2009, Broadcom Corporation
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
#include "btnr_3250ob.h"

BDBG_MODULE(btnr_3250ob);

#define	DEV_MAGIC_ID			((BERR_TNR_ID<<16) | 0xFACE)


#define	CHK_RETCODE( rc, func )			\
do {										\
	if( (rc = BERR_TRACE(func)) != BERR_SUCCESS ) \
	{										\
		goto done;							\
	}										\
} while(0)


#define	HERTZ_TO_MHZ(x)			((float) ((float) x / 1000000.0))
#define	MHZ_TO_HERTZ(x)			(x * 1000000)





/*******************************************************************************
*
*	Private Module Handles
*
*******************************************************************************/
typedef struct BTNR_P_3250Ob_Handle		*BTNR_3250Ob_Handle;

typedef struct BTNR_P_3250Ob_Settings
{
	BQOB_Handle hQobDev;					/* Required to set LO Freq */
	unsigned long ifFreq;
	unsigned long rfFreq;
} BTNR_P_3250Ob_Settings;

typedef struct BTNR_P_3250Ob_Handle
{
	uint32_t magicId;					/* Used to check if structure is corrupt */

	BTNR_P_3250Ob_Settings settings;
} BTNR_P_3250Ob_Handle;



/*******************************************************************************
*
*	Default Module Settings
*
*******************************************************************************/
static const BTNR_3250Ob_Settings defDevSettings =
{
	BTNR_3250OB_SETTINGS_IFFREQ
};



/*******************************************************************************
*
*	Private Module Data
*
*******************************************************************************/



/*******************************************************************************
*
*	Private Module Functions
*
*******************************************************************************/
static BERR_Code BTNR_3250Ob_Close(
	BTNR_Handle hDev					/* [in] Device handle */
	)
{
	BERR_Code retCode = BERR_SUCCESS;


	BDBG_ENTER(BTNR_3250Ob_Close);
	BDBG_ASSERT( hDev );
	BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

	hDev->magicId = 0x00;		/* clear it to catch inproper use */
	BKNI_Free( (void *) hDev->hDevImpl );
	BKNI_Free( (void *) hDev );

	BDBG_LEAVE(BTNR_3250Ob_Close);
	return( retCode );
}

static BERR_Code BTNR_3250Ob_SetRfFreq(
	BTNR_3250Ob_Handle hDev,			/* [in] Device handle */
	uint32_t rfFreq,					/* [in] Requested tuner freq., in Hertz */
    BTNR_TunerMode tunerMode			/* [in] Requested tuner mode */
    )
{
	BERR_Code retCode = BERR_SUCCESS;
	unsigned long loFreq;


	BDBG_ENTER(BTNR_3250Ob_SetRfFreq);
	BDBG_ASSERT( hDev );
	BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );
	BSTD_UNUSED(tunerMode);

	loFreq = hDev->settings.ifFreq + rfFreq;
	/* Now serialize this call to BQOB code, just in case others may be
	   trying to access BQOB  */
	BKNI_EnterCriticalSection();
	CHK_RETCODE( retCode, BQOB_SetLoFreq( hDev->settings.hQobDev, loFreq ) );
	BKNI_LeaveCriticalSection();
	hDev->settings.rfFreq = rfFreq;

done:
	BDBG_LEAVE(BTNR_3250Ob_SetRfFreq);
	return( retCode );
}

static BERR_Code BTNR_3250Ob_GetRfFreq(
	BTNR_3250Ob_Handle hDev,			/* [in] Device handle */
	uint32_t *rfFreq,					/* [output] Returns tuner freq., in Hertz */
	BTNR_TunerMode *tunerMode			/* [output] Returns tuner mode */
    )
{
	BERR_Code retCode = BERR_SUCCESS;


	BDBG_ENTER(BTNR_3250Ob_GetRfFreq);
	BDBG_ASSERT( hDev );
	BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

	*rfFreq = hDev->settings.rfFreq;
	*tunerMode = BTNR_TunerMode_eDigital;

	BDBG_LEAVE(BTNR_3250Ob_GetRfFreq);
	return( retCode );
}




/*******************************************************************************
*
*	Public Module Functions
*
*******************************************************************************/
BERR_Code BTNR_3250Ob_Open(
	BTNR_Handle *phDev,					/* [output] Returns handle */
	BCHP_Handle hChip,					/* [in] Chip handle */
	BQOB_Handle hQob,					/* [in] QOB handle for Bcm3250 */
	const BTNR_3250Ob_Settings *pDefSettings /* [in] Default settings */
	)
{
	BERR_Code retCode = BERR_SUCCESS;
	BTNR_3250Ob_Handle h3250ObDev;
	BTNR_Handle hDev;


	BDBG_ENTER(BTNR_3250Ob_Open);
	BDBG_ASSERT( hQob );
	BSTD_UNUSED(hChip);

	hDev = NULL;
	/* Alloc memory from the system heap */
	h3250ObDev = (BTNR_3250Ob_Handle) BKNI_Malloc( sizeof( BTNR_P_3250Ob_Handle ) );
	if( h3250ObDev == NULL )
	{
		retCode = BERR_OUT_OF_SYSTEM_MEMORY;
		BDBG_ERR(("BTNR_3250Ob_Open: BKNI_malloc() failed\n"));
		goto done;
	}
	BKNI_Memset( h3250ObDev, 0x00, sizeof( BTNR_P_3250Ob_Handle ) );

	hDev = (BTNR_Handle) BKNI_Malloc( sizeof( BTNR_P_Handle ) );
	if( hDev == NULL )
	{
		retCode = BERR_OUT_OF_SYSTEM_MEMORY;
		BDBG_ERR(("BTNR_3250Ob_Open: BKNI_malloc() failed\n"));
		BKNI_Free( h3250ObDev );
		goto done;
	}

	h3250ObDev->magicId = DEV_MAGIC_ID;
	h3250ObDev->settings.ifFreq = pDefSettings->ifFreq;
	h3250ObDev->settings.hQobDev = hQob;
	h3250ObDev->settings.rfFreq = 0;

	hDev->hDevImpl = (void *) h3250ObDev;
	hDev->magicId = DEV_MAGIC_ID;
	hDev->pSetRfFreq = (BTNR_SetRfFreqFunc) BTNR_3250Ob_SetRfFreq;
	hDev->pGetRfFreq = (BTNR_GetRfFreqFunc) BTNR_3250Ob_GetRfFreq;
	hDev->pGetAgcRegVal = (BTNR_GetAgcRegValFunc) NULL;
	hDev->pClose = (BTNR_CloseFunc) BTNR_3250Ob_Close;
	hDev->pGetPowerSaver = (BTNR_GetPowerSaverFunc)NULL;
	hDev->pSetPowerSaver = (BTNR_SetPowerSaverFunc)NULL;

done:
	*phDev = hDev;
	BDBG_LEAVE(BTNR_3250Ob_Open);
	return( retCode );
}

BERR_Code BTNR_3250Ob_GetDefaultSettings(
	BTNR_3250Ob_Settings *pDefSettings,	/* [output] Returns default setting */
	BCHP_Handle hChip					/* [in] Chip handle */
	)
{
	BERR_Code retCode = BERR_SUCCESS;

	
	BDBG_ENTER(BTNR_3250Ob_GetDefaultSettings);
	BSTD_UNUSED(hChip);

	*pDefSettings = defDevSettings;

	BDBG_LEAVE(BTNR_3250Ob_GetDefaultSettings);
	return( retCode );
}

