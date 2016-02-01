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
#include "btnr_3255ob.h"
#include "brpc.h"
#include "brpc_3255.h"

BDBG_MODULE(btnr_3255ob);

#define sizeInLong(x)	(sizeof(x)/sizeof(long))

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
typedef struct BTNR_P_3255Ob_Handle		*BTNR_3255Ob_Handle;

typedef struct BTNR_P_3255Ob_Settings
{
	unsigned long ifFreq;
	unsigned long rfFreq;
} BTNR_P_3255Ob_Settings;

typedef struct BTNR_P_3255Ob_Handle
{
	uint32_t magicId;					/* Used to check if structure is corrupt */
	BTNR_P_3255Ob_Settings settings;
	BRPC_DevId devId;
	BRPC_Handle hRpc;
} BTNR_P_3255Ob_Handle;



/*******************************************************************************
*
*	Default Module Settings
*
*******************************************************************************/
static const BTNR_3255Ob_Settings defDevSettings =
{
	BRPC_DevId_3255_TNR0_OOB,				/* We need this to explicitly define which tuner we are using */
	NULL,									/* BRPC handle, must be provided by application*/
	BTNR_3255OB_SETTINGS_IFFREQ
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
static BERR_Code BTNR_3255Ob_Close(
	BTNR_Handle hDev					/* [in] Device handle */
	)
{
	BERR_Code retCode = BERR_SUCCESS;
	BTNR_3255Ob_Handle hImplDev;
	BRPC_Param_TNR_OOB_Close Param;
	BERR_Code retVal;

	BDBG_ENTER(BTNR_3255Ob_Close);
	BDBG_ASSERT( hDev );
	BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

	hImplDev = (BTNR_3255Ob_Handle) hDev->hDevImpl;
	BDBG_ASSERT( hImplDev->hRpc );

	Param.devId = hImplDev->devId;
	CHK_RETCODE( retCode, BRPC_CallProc(hImplDev->hRpc, BRPC_ProcId_TNR_Oob_Close, (const uint32_t *)&Param, sizeInLong(Param), NULL, 0, &retVal) );
	CHK_RETCODE( retCode, retVal );

	hDev->magicId = 0x00;		/* clear it to catch inproper use */
	BKNI_Free( (void *) hDev->hDevImpl );
	BKNI_Free( (void *) hDev );

done:
	BDBG_LEAVE(BTNR_3255Ob_Close);
	return( retCode );
}

static BERR_Code BTNR_3255Ob_SetRfFreq(
	BTNR_3255Ob_Handle hDev,			/* [in] Device handle */
	uint32_t rfFreq,					/* [in] Requested tuner freq., in Hertz */
    BTNR_TunerMode tunerMode			/* [in] Requested tuner mode */
    )
{
	BERR_Code retCode = BERR_SUCCESS;
	BRPC_Param_TNR_OOB_SetRfFreq Param;
	BERR_Code retVal;

	BDBG_ENTER(BTNR_3255Ib_SetRfFreq);
	BDBG_ASSERT( hDev );
	BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );
	BSTD_UNUSED(tunerMode);

	BDBG_MSG(("%s: enter, freq=%d", __FUNCTION__, rfFreq));

	BDBG_ASSERT( hDev->hRpc );
	Param.devId = hDev->devId;
	Param.rfFreq = rfFreq;

	CHK_RETCODE( retCode, BRPC_CallProc(hDev->hRpc, BRPC_ProcId_TNR_Oob_SetRfFreq, (const uint32_t *)&Param, sizeInLong(Param), NULL, 0, &retVal) );
	CHK_RETCODE( retCode, retVal);

	hDev->settings.rfFreq = rfFreq;
done:
	BDBG_LEAVE(BTNR_3255Ib_SetRfFreq);
	return( retCode );

}

static BERR_Code BTNR_3255Ob_GetRfFreq(
	BTNR_3255Ob_Handle hDev,			/* [in] Device handle */
	uint32_t *rfFreq,					/* [output] Returns tuner freq., in Hertz */
	BTNR_TunerMode *tunerMode			/* [output] Returns tuner mode */
    )
{
	BERR_Code retCode = BERR_SUCCESS;

	BDBG_ENTER(BTNR_3255Ob_GetRfFreq);
	BDBG_ASSERT( hDev );
	BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

	*rfFreq = hDev->settings.rfFreq;
	*tunerMode = BTNR_TunerMode_eDigital;

	BDBG_LEAVE(BTNR_3255Ob_GetRfFreq);
	return( retCode );
}

/*******************************************************************************
*
*	Public Module Functions
*
*******************************************************************************/
BERR_Code BTNR_3255Ob_Open(
	BTNR_Handle *phDev,					/* [output] Returns handle */
	BCHP_Handle hChip,					/* [in] Chip handle */
	const BTNR_3255Ob_Settings *pDefSettings /* [in] Default settings */
	)
{
	BERR_Code retCode = BERR_SUCCESS;
	BTNR_3255Ob_Handle h3255ObDev;
	BTNR_Handle hDev;
	BRPC_Param_TNR_OOB_Open Param;
	BERR_Code retVal;

	BDBG_ENTER(BTNR_3255Ob_Open);
	BSTD_UNUSED(hChip);

	hDev = NULL;
	/* Alloc memory from the system heap */
	h3255ObDev = (BTNR_3255Ob_Handle) BKNI_Malloc( sizeof( BTNR_P_3255Ob_Handle ) );
	if( h3255ObDev == NULL )
	{
		retCode = BERR_OUT_OF_SYSTEM_MEMORY;
		BDBG_ERR(("BTNR_3255Ob_Open: BKNI_malloc() failed\n"));
		goto done;
	}
	BKNI_Memset( h3255ObDev, 0x00, sizeof( BTNR_P_3255Ob_Handle ) );

	hDev = (BTNR_Handle) BKNI_Malloc( sizeof( BTNR_P_Handle ) );
	if( hDev == NULL )
	{
		retCode = BERR_OUT_OF_SYSTEM_MEMORY;
		BDBG_ERR(("BTNR_3255Ob_Open: BKNI_malloc() failed\n"));
		goto done;
	}

	h3255ObDev->magicId = DEV_MAGIC_ID;
	h3255ObDev->settings.ifFreq = pDefSettings->ifFreq;
	h3255ObDev->settings.rfFreq = 0;
	h3255ObDev->hRpc = pDefSettings->hGeneric;
	h3255ObDev->devId = pDefSettings->devId;
	BDBG_ASSERT( h3255ObDev->hRpc );

	Param.devId = h3255ObDev->devId;
	Param.ifFreq = pDefSettings->ifFreq;
	CHK_RETCODE( retCode, BRPC_CallProc(h3255ObDev->hRpc, BRPC_ProcId_TNR_Oob_Open, (const uint32_t *)&Param, sizeInLong(Param), NULL, 0, &retVal) );
	CHK_RETCODE( retCode, retVal );

	hDev->hDevImpl = (void *) h3255ObDev;
	hDev->magicId = DEV_MAGIC_ID;
	hDev->pSetRfFreq = (BTNR_SetRfFreqFunc) BTNR_3255Ob_SetRfFreq;
	hDev->pGetRfFreq = (BTNR_GetRfFreqFunc) BTNR_3255Ob_GetRfFreq;
	hDev->pGetAgcRegVal = (BTNR_GetAgcRegValFunc) NULL;
	hDev->pSetAgcRegVal = (BTNR_SetAgcRegValFunc) NULL;
	hDev->pClose = (BTNR_CloseFunc) BTNR_3255Ob_Close;
	hDev->pGetPowerSaver = (BTNR_GetPowerSaverFunc)NULL;
	hDev->pSetPowerSaver = (BTNR_SetPowerSaverFunc)NULL;

	*phDev = hDev;
    BDBG_LEAVE(BTNR_3255Ob_Open);
    return( retCode );

done:
    if (h3255ObDev) BKNI_Free( h3255ObDev );
    if (hDev) BKNI_Free(hDev);
	BDBG_LEAVE(BTNR_3255Ob_Open);
	return( retCode );
}

BERR_Code BTNR_3255Ob_GetDefaultSettings(
	BTNR_3255Ob_Settings *pDefSettings,	/* [output] Returns default setting */
	BCHP_Handle hChip					/* [in] Chip handle */
	)
{
	BERR_Code retCode = BERR_SUCCESS;


	BDBG_ENTER(BTNR_3255Ob_GetDefaultSettings);
	BSTD_UNUSED(hChip);

	*pDefSettings = defDevSettings;

	BDBG_LEAVE(BTNR_3255Ob_GetDefaultSettings);
	return( retCode );
}

