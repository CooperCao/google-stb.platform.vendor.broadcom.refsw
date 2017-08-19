/***************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *
 * Module Description:
 *
 ***************************************************************************/

#include "bstd.h"
#include "baob.h"
#include "baob_priv.h"


BDBG_MODULE(baob);


#define sizeInLong(x)	(sizeof(x)/sizeof(uint32_t))
#define	CHK_RETCODE( rc, func )				\
do {										\
	if( (rc = BERR_TRACE(func)) != BERR_SUCCESS ) \
	{										\
		goto done;							\
	}										\
} while(0)


#define	DEV_MAGIC_ID			((BERR_AOB_ID<<16) | 0xFACE)


/*******************************************************************************
*
*	Default Module Settings
*
*******************************************************************************/
static const BAOB_Settings defDevSettings =
{
	BRPC_DevId_3255_OB0,					/* TODO: Is this required? */
	NULL,									/* BRPC handle, must be provided by application*/
	BAOB_SETTINGS_XTALFREQ,
	BAOB_SETTINGS_IFFREQ,
	false,
	BAOB_SpectrumMode_eNoInverted
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

/*******************************************************************************
*
*	Public Module Functions
*
*******************************************************************************/

BERR_Code BAOB_GetDefaultSettings(
	BAOB_Settings *pDefSettings,		/* [out] Returns default setting */
	BCHP_Handle hChip					/* [in] Chip handle */
	)
{
	BERR_Code retCode = BERR_SUCCESS;

	BSTD_UNUSED(hChip);
	BDBG_ENTER(BAOB_GetDefaultSettings);

	*pDefSettings = defDevSettings;

	BDBG_LEAVE(BAOB_GetDefaultSettings);
	return( retCode );
}



BERR_Code BAOB_Open(
	BAOB_Handle *pAob,					/* [out] Returns handle */
	BCHP_Handle hChip,					/* [in] Chip handle */
	BREG_Handle hRegister,				/* [in] Register handle */
	BINT_Handle hInterrupt,				/* [in] Interrupt handle */
	const BAOB_Settings *pDefSettings	/* [in] Default settings */
	)
{
	BERR_Code retCode = BERR_SUCCESS;
	BAOB_Handle hDev;
	BRPC_Param_AOB_Open	Param;
	BERR_Code retVal;

	BDBG_ENTER(BAOB_Open);

	/* Alloc memory from the system heap */
	hDev = (BAOB_Handle) BKNI_Malloc( sizeof( BAOB_P_Handle ) );
	if( hDev == NULL )
	{
		retCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
		BDBG_ERR(("BAOB_Open: BKNI_malloc() failed"));
		goto done;
	}
	BKNI_Memset( hDev, 0x00, sizeof( BAOB_P_Handle ) );

	hDev->magicId = DEV_MAGIC_ID;
	hDev->hChip = hChip;
	hDev->hRegister = hRegister;
	hDev->hInterrupt = hInterrupt;
	hDev->hRpc = pDefSettings->hGeneric;	/* For this device, we need the RPC handle */
	hDev->devId = pDefSettings->devId;
	hDev->enableFEC = pDefSettings->enableFEC;
	hDev->spectrum = pDefSettings->spectrum;

	Param.devId = hDev->devId;
	Param.enableFEC = pDefSettings->enableFEC;

	CHK_RETCODE( retCode, BRPC_CallProc(hDev->hRpc, BRPC_ProcId_AOB_Open, (const uint32_t *)&Param, sizeInLong(Param), NULL, 0, &retVal));
	CHK_RETCODE( retCode, retVal );

	/* set default spectrum setting*/
	CHK_RETCODE( retCode, BAOB_Set_Spectrum(hDev, hDev->spectrum));

	CHK_RETCODE(retCode, BKNI_CreateMutex(&hDev->mutex));

	*pAob = hDev;

done:
	if( retCode != BERR_SUCCESS )
	{
		if( hDev != NULL )
		{
			BKNI_Free( hDev );
		}
		*pAob = NULL;
	}

	BDBG_LEAVE(BAOB_Open);
	return( retCode );
}

BERR_Code BAOB_Close(
	BAOB_Handle hDev					/* [in] Device handle */
	)
{
	BERR_Code retCode = BERR_SUCCESS;
	BRPC_Param_AOB_Close Param;
	BERR_Code retVal;


	BDBG_ENTER(BAOB_Close);
	BDBG_ASSERT( hDev );
	BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );
	BDBG_ASSERT( hDev->hRpc );

	Param.devId = hDev->devId;
	CHK_RETCODE( retCode, BRPC_CallProc(hDev->hRpc, BRPC_ProcId_AOB_Close, (const uint32_t *)&Param, sizeInLong(Param), NULL, 0, &retVal));
	CHK_RETCODE( retCode, retVal );

	BKNI_DestroyMutex(hDev->mutex);
	hDev->magicId = 0x00;		/* clear it to catch inproper use */
	BKNI_Free( (void *) hDev );

done:
	BDBG_LEAVE(BAOB_Close);
	return( retCode );
}


BERR_Code BAOB_Acquire(
	BAOB_Handle hDev,					/* [in] Device handle */
	BAOB_ModulationType modType,		/* [in] Modulation type to use */
	uint32_t symbolRate					/* [in] Symbol rate to use */
	)
{
	BERR_Code retCode = BERR_SUCCESS;
	BRPC_Param_AOB_Acquire Param;
	BERR_Code retVal;


	BDBG_ENTER(BAOB_Acquire);
	BDBG_ASSERT( hDev );
	BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );
	BDBG_ASSERT( hDev->hRpc );

	BDBG_MSG(("%s: modType=%d, symbolRate=%d", BSTD_FUNCTION, modType, symbolRate));
	BKNI_AcquireMutex(hDev->mutex);
	hDev->isLock = false;
	BKNI_ReleaseMutex(hDev->mutex);

	Param.devId = hDev->devId;
	Param.modType = modType;
	Param.symbolRate = symbolRate;
	CHK_RETCODE( retCode, BRPC_CallProc(hDev->hRpc, BRPC_ProcId_AOB_Acquire, (const uint32_t *)&Param, sizeInLong(Param), NULL, 0, &retVal));
	CHK_RETCODE( retCode, retVal );

done:
	BDBG_LEAVE(BAOB_Acquire);
	return( retCode );
}

BERR_Code BAOB_GetStatus(
	BAOB_Handle hDev,					/* [in] Device handle */
	BAOB_Status *pStatus				/* [out] Returns status */
	)
{
	BERR_Code retCode = BERR_SUCCESS;
	BRPC_Param_AOB_GetStatus outParam;
	BRPC_Param_XXX_Get Param;
	BERR_Code retVal;


	BDBG_ENTER(BAOB_GetStatus);
	BDBG_ASSERT( hDev );
	BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );
	BDBG_ASSERT( hDev->hRpc );

	pStatus->isFecLock = pStatus->isQamLock = false;
	Param.devId = hDev->devId;

	CHK_RETCODE( retCode, BRPC_CallProc(hDev->hRpc, BRPC_ProcId_AOB_GetStatus, (const uint32_t *)&Param, sizeInLong(Param), (uint32_t *)&outParam, sizeInLong(outParam), &retVal) );
	CHK_RETCODE( retCode, retVal);

	pStatus->modType = outParam.modType;
	pStatus->symbolRate = outParam.symbolRate;
	pStatus->ifFreq = outParam.ifFreq;
	pStatus->loFreq = outParam.loFreq;
	pStatus->sysXtalFreq = outParam.sysXtalFreq;
	pStatus->isFecLock = outParam.isFecLock;
	pStatus->isQamLock = outParam.isQamLock;
	pStatus->snrEstimate = outParam.snrEstimate;
	pStatus->agcIntLevel = outParam.agcIntLevel;
	pStatus->agcExtLevel = outParam.agcExtLevel;
	pStatus->carrierFreqOffset = outParam.carrierFreqOffset;
	pStatus->carrierPhaseOffset = outParam.carrierPhaseOffset;
	pStatus->uncorrectedCount = outParam.uncorrectedCount;
	pStatus->correctedCount = outParam.correctedCount;
	pStatus->berErrorCount = outParam.berErrorCount;
	pStatus->fdcChannelPower= outParam.fdcChannelPower;

	BDBG_MSG((" outParam.modType%d  outParam.ifFreq%d outParam.symbolRate%d\
		outParam.fdcChannelPower%d outParam.snrEstimate%d\n", outParam.modType,
		outParam.ifFreq, outParam.symbolRate, outParam.fdcChannelPower, outParam.snrEstimate));
done:
	BDBG_LEAVE(BAOB_GetStatus);
	return( retCode );
}

BERR_Code BAOB_GetLockStatus(
	BAOB_Handle hDev,					/* [in] Device handle */
	bool *isLock						/* [out] Returns lock status, 0=not lock, 1=locked */
	)
{
	BERR_Code retCode = BERR_SUCCESS;
#ifdef USE_RPC_GET_LOCK_STATUS
	BRPC_Param_AOB_GetLockStatus outParam;
	BRPC_Param_XXX_Get Param;
    BERR_Code retVal;
#endif

	BDBG_ENTER(BAOB_GetLockStatus);
	BDBG_ASSERT( hDev );
	BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );
	BDBG_ASSERT( hDev->hRpc );

	*isLock = false;
#ifdef USE_RPC_GET_LOCK_STATUS
	Param.devId = hDev->devId;
	CHK_RETCODE( retCode, BRPC_CallProc(hDev->hRpc, BRPC_ProcId_AOB_GetLockStatus, (const uint32_t *)&Param, sizeInLong(Param), (uint32_t *)&outParam, sizeInLong(outParam), &retVal) );
	CHK_RETCODE( retCode, retVal );
	if (hDev->enableFEC)
		*isLock = (outParam.isQamLock == true) && (outParam.isFecLock == true);
	else
		*isLock = outParam.isQamLock;
#else
	BKNI_AcquireMutex(hDev->mutex);
	*isLock = hDev->isLock;
	BKNI_ReleaseMutex(hDev->mutex);
#endif

#ifdef USE_RPC_GET_LOCK_STATUS
done:
#endif
	BDBG_LEAVE(BAOB_GetLockStatus);
	return( retCode );
}

BERR_Code BAOB_GetSoftDecision(
	BAOB_Handle hDev,					/* [in] Device handle */
	int16_t nbrToGet,					/* [in] Number values to get */
	int16_t *iVal,						/* [out] Ptr to array to store output I soft decision */
	int16_t *qVal,						/* [out] Ptr to array to store output Q soft decision */
	int16_t *nbrGotten					/* [out] Number of values gotten/read */
	)
{
	BERR_Code retCode = BERR_NOT_SUPPORTED;

	BDBG_ENTER(BAOB_GetSoftDecision);
	BDBG_ASSERT( hDev );
	BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );
	BDBG_ASSERT( hDev->hRpc );

	BSTD_UNUSED(nbrToGet);
	BSTD_UNUSED(iVal);
	BSTD_UNUSED(qVal);
	BSTD_UNUSED(nbrGotten);
#if !(BDBG_DEBUG_BUILD)
	BSTD_UNUSED(hDev);
#endif
	BDBG_LEAVE(BAOB_GetSoftDecision);
	return( retCode );
}

BERR_Code BAOB_EnablePowerSaver(
	BAOB_Handle hDev					/* [in] Device handle */
	)
{
	BERR_Code retCode = BERR_SUCCESS;
	BRPC_Param_AOB_EnablePowerSaver Param;
	BERR_Code retVal;

	BDBG_ENTER(BAOB_EnablePowerSaver);
	BDBG_ASSERT( hDev );
	BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );
	BDBG_ASSERT( hDev->hRpc );

	Param.devId = hDev->devId;
	CHK_RETCODE( retCode, BRPC_CallProc(hDev->hRpc, BRPC_ProcId_AOB_EnablePowerSaver, (const uint32_t *)&Param, sizeInLong(Param), NULL, 0, &retVal));
	CHK_RETCODE( retCode, retVal );
done:
	BDBG_LEAVE(BAOB_EnablePowerSaver);
	return( retCode );
}

BERR_Code BAOB_Get_Spectrum(
	BAOB_Handle hDev,					/* [in] Device handle */
	BAOB_SpectrumMode *spectrum			/* [out]current spectrum setting*/
	)
{
	BERR_Code retCode = BERR_SUCCESS;

	BDBG_ENTER(BAOB_Get_Spectrum);
	BDBG_ASSERT( hDev );
	BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );
	BDBG_ASSERT( hDev->hRpc );

	*spectrum = hDev->spectrum;

	BDBG_LEAVE(BAOB_Get_Spectrum);
	return( retCode );
}


BERR_Code BAOB_Set_Spectrum(
	BAOB_Handle hDev,					/* [in] Device handle */
	BAOB_SpectrumMode spectrum			/* [in] spectrum setting to be set*/
	)
{
	BERR_Code retCode = BERR_SUCCESS;
	BRPC_Param_AOB_SetSpectrum Param;
	BERR_Code retVal;

	BDBG_ENTER(BAOB_Set_Spectrum);
	BDBG_ASSERT( hDev );
	BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );
	BDBG_ASSERT( hDev->hRpc );

	Param.devId = hDev->devId;
	Param.spectrum = spectrum;

	CHK_RETCODE( retCode, BRPC_CallProc(hDev->hRpc, BRPC_ProcId_AOB_SetSpectrum, (const uint32_t *)&Param, sizeInLong(Param), NULL, 0, &retVal));
	CHK_RETCODE( retCode, retVal );

	hDev->spectrum = spectrum;
done:
	BDBG_LEAVE(BAOB_Set_Spectrum);
	return( retCode );

}


BERR_Code BAOB_ProcessNotification(
	BAOB_Handle hDev,						/* [in] Device handle */
	unsigned int event						/* [in] Event code and event data*/
	)
{
	BERR_Code retCode = BERR_SUCCESS;
	BRPC_Notification_Event event_code;
	bool isFECLock, isQAMLock;

	BDBG_ENTER(BAOB_ProcessNotification);
	BDBG_ASSERT( hDev );
	BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

	event_code = event>>16;

	switch (event_code) {
		case BRPC_Notification_Event_LockStatusChanged:
			isQAMLock = BRPC_GET_AOB_QAM_LOCKSTATUS(event);
			isFECLock = BRPC_GET_AOB_FEC_LOCKSTATUS(event);
			BKNI_AcquireMutex(hDev->mutex);
			if (hDev->enableFEC){
				hDev->isLock = isQAMLock && isFECLock;
			} else {
				hDev->isLock = isQAMLock;
			}
			BKNI_ReleaseMutex(hDev->mutex);
			if( hDev->pCallback[BAOB_Callback_eLockChange] != NULL )
			{
				(hDev->pCallback[BAOB_Callback_eLockChange])(hDev->pCallbackParam[BAOB_Callback_eLockChange] );
			}
			BDBG_MSG((" AOB LockStatusChanged from 3255: isLock? %d", hDev->isLock));
			break;
		default:
			BDBG_WRN((" unknown event code from 3255 %x",event_code ));
			break;
	}

	BDBG_LEAVE(BAOB_ProcessNotification);
	return( retCode );
}

BERR_Code BAOB_InstallCallback(
	BAOB_Handle hDev,					/* [in] Device handle */
	BAOB_Callback callbackType,			/* [in] Type of callback */
	BAOB_CallbackFunc pCallback,		/* [in] Function Ptr to callback */
	void *pParam						/* [in] Generic parameter send on callback */
	)
{
	BERR_Code retCode = BERR_SUCCESS;


	BDBG_ENTER(BAOB_InstallCallback);
	BDBG_ASSERT( hDev );
	BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

	switch( callbackType )
	{
		case BAOB_Callback_eLockChange:
			hDev->pCallback[callbackType] = pCallback;
			hDev->pCallbackParam[callbackType] = pParam;
			break;
		default:
			retCode = BERR_TRACE(BERR_INVALID_PARAMETER);
			break;
	}

	BDBG_LEAVE(BAOB_InstallCallback);
	return( retCode );
}
