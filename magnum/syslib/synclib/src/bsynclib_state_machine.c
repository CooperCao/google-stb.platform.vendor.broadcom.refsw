/***************************************************************************
*     Copyright (c) 2004-2010, Broadcom Corporation
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
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/

#include "bstd.h"
#include "bsynclib_state_machine.h"
#include "bsynclib_state_machine_priv.h"

BDBG_MODULE(synclib);

/*
Summary:
Gets the default settings
*/
BERR_Code BSYNClib_StateMachine_GetDefaultSettings(
	BSYNClib_StateMachine_Settings * psSettings /* [out] */
)
{
	BERR_Code rc = BERR_SUCCESS;

	BDBG_ASSERT(psSettings);

	psSettings->cbStateChange.pfStateChange = NULL;
	psSettings->cbStateChange.pvParm1 = NULL;
	psSettings->cbStateChange.iParm2 = 0;

	return rc;
}

/*
Summary:
Opens a state machine handle
*/
BERR_Code BSYNClib_StateMachine_Open(
	const BSYNClib_StateMachine_Settings * psSettings,
	BSYNClib_StateMachine_Handle * phMachine
)
{
	BERR_Code rc = BERR_SUCCESS;
	BSYNClib_StateMachine_Handle hMachine;

	BDBG_ASSERT(phMachine);

	hMachine = (BSYNClib_StateMachine_Handle)BKNI_Malloc(sizeof(struct BSYNClib_StateMachine_Impl));
	if (!hMachine)
	{
		rc = BERR_OUT_OF_SYSTEM_MEMORY;
		goto error;
	}

	BKNI_Memset(hMachine, 0, sizeof(struct BSYNClib_StateMachine_Impl));

	if (psSettings)
	{
		hMachine->sSettings = *psSettings;
	}

	*phMachine = hMachine;

	goto end;

error:

	*phMachine = NULL;

end:
	return rc;
}

/*
Summary:
Closes a state machine handle
*/
BERR_Code BSYNClib_StateMachine_Close(
	BSYNClib_StateMachine_Handle hMachine
)
{
	BERR_Code rc = BERR_SUCCESS;

	BDBG_ASSERT(hMachine);

	rc = BSYNClib_StateMachine_SendSignal(hMachine, BSYNClib_StateMachine_Signal_eStop);
	if (rc) goto error;

	BKNI_Free(hMachine);

	goto end;
	
error:

end:
	return rc;
}

BERR_Code BSYNClib_StateMachine_SendSignal(
	BSYNClib_StateMachine_Handle hMachine,
	BSYNClib_StateMachine_Signal eSignal
)
{
	BERR_Code rc = BERR_SUCCESS;

	BDBG_ASSERT(hMachine);

	switch (hMachine->sData.eState)
	{
		case BSYNClib_StateMachine_State_eAcquire:
			switch (eSignal)
			{
				case BSYNClib_StateMachine_Signal_eStart:
					break;
				case BSYNClib_StateMachine_Signal_eCheckPassed:
					hMachine->sData.eState = BSYNClib_StateMachine_State_eSync;
					BSYNClib_StateMachine_FireStateChangeCallback(hMachine);
					break;
				case BSYNClib_StateMachine_Signal_eCheckFailed:
					break;
				case BSYNClib_StateMachine_Signal_eDataChanged:
					break;
				case BSYNClib_StateMachine_Signal_eDelayApplied:
					break;
				case BSYNClib_StateMachine_Signal_eStop:
					hMachine->sData.eState = BSYNClib_StateMachine_State_eStopped;
					BSYNClib_StateMachine_FireStateChangeCallback(hMachine);
					break;
				default:
					break;
			}
			break;
		case BSYNClib_StateMachine_State_eSync:
			switch (eSignal)
			{
				case BSYNClib_StateMachine_Signal_eStart:
					break;
				case BSYNClib_StateMachine_Signal_eCheckPassed:
					break;
				case BSYNClib_StateMachine_Signal_eCheckFailed:
					break;
				case BSYNClib_StateMachine_Signal_eDataChanged:
					break;
				case BSYNClib_StateMachine_Signal_eDelayApplied:
					hMachine->sData.eState = BSYNClib_StateMachine_State_eTrack;
					BSYNClib_StateMachine_FireStateChangeCallback(hMachine);
					break;
				case BSYNClib_StateMachine_Signal_eStop:
					hMachine->sData.eState = BSYNClib_StateMachine_State_eStopped;
					BSYNClib_StateMachine_FireStateChangeCallback(hMachine);
					break;
				default:
					break;
			}
			break;
		case BSYNClib_StateMachine_State_eTrack:
			switch (eSignal)
			{
				case BSYNClib_StateMachine_Signal_eStart:
					break;
				case BSYNClib_StateMachine_Signal_eCheckPassed:
					break;
				case BSYNClib_StateMachine_Signal_eCheckFailed:
					hMachine->sData.eState = BSYNClib_StateMachine_State_eAcquire;
					BSYNClib_StateMachine_FireStateChangeCallback(hMachine);
					break;
				case BSYNClib_StateMachine_Signal_eDataChanged:
					hMachine->sData.eState = BSYNClib_StateMachine_State_eSync;
					BSYNClib_StateMachine_FireStateChangeCallback(hMachine);
					break;
				case BSYNClib_StateMachine_Signal_eDelayApplied:
					break;
				case BSYNClib_StateMachine_Signal_eStop:
					hMachine->sData.eState = BSYNClib_StateMachine_State_eStopped;
					BSYNClib_StateMachine_FireStateChangeCallback(hMachine);
					break;
				default:
					break;
			}
			break;
		case BSYNClib_StateMachine_State_eStopped:
			switch (eSignal)
			{
				case BSYNClib_StateMachine_Signal_eStart:
					hMachine->sData.eState = BSYNClib_StateMachine_State_eAcquire;
					BSYNClib_StateMachine_FireStateChangeCallback(hMachine);
					break;
				case BSYNClib_StateMachine_Signal_eCheckPassed:
					break;
				case BSYNClib_StateMachine_Signal_eCheckFailed:
					break;
				case BSYNClib_StateMachine_Signal_eDataChanged:
					break;
				case BSYNClib_StateMachine_Signal_eDelayApplied:
					break;
				case BSYNClib_StateMachine_Signal_eStop:
					break;
				default:
					break;
			}
			break;
		default:
			break;
	}

	hMachine->sStatus.eState = hMachine->sData.eState;
	
	return rc;
}

BERR_Code BSYNClib_StateMachine_FireStateChangeCallback(
	BSYNClib_StateMachine_Handle hMachine
)
{
	BERR_Code rc = BERR_SUCCESS;
	BSYNClib_StateMachine_StateChangeCallback * pcbStateChange;

	BDBG_ASSERT(hMachine);

	pcbStateChange = &hMachine->sSettings.cbStateChange;

	if (pcbStateChange->pfStateChange)
	{
		rc = pcbStateChange->pfStateChange(pcbStateChange->pvParm1, pcbStateChange->iParm2, hMachine->sData.eState);
		if (rc) goto error;
	}

	goto end;
	
error:

end:
	return rc;
}

/*
Summary:
*/
BERR_Code BSYNClib_StateMachine_GetStatus(
	const BSYNClib_StateMachine_Handle hMachine,
	BSYNClib_StateMachine_Status * psStatus
)
{
	BERR_Code rc = BERR_SUCCESS;

	BDBG_ASSERT(hMachine);
	BDBG_ASSERT(psStatus);

	*psStatus = hMachine->sStatus;

	return rc;
}

