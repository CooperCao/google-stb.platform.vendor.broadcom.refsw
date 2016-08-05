/***************************************************************************
 *  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.
 *
 * Module Description:
 *
 **************************************************************************/
#include "bstd.h"
#include "bsynclib_state_machine.h"
#include "bsynclib_state_machine_priv.h"

BDBG_MODULE(synclib);

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

BERR_Code BSYNClib_StateMachine_Close(
	BSYNClib_StateMachine_Handle hMachine
)
{
	BERR_Code rc = BERR_SUCCESS;
	BDBG_ASSERT(hMachine);
	(void)BSYNClib_StateMachine_SendSignal(hMachine, BSYNClib_StateMachine_Signal_eStop);
	BKNI_Free(hMachine);
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
	}

	return rc;
}

BERR_Code BSYNClib_StateMachine_GetStatus(
	const BSYNClib_StateMachine_Handle hMachine,
	BSYNClib_StateMachine_Status * psStatus
)
{
	BDBG_ASSERT(hMachine);
	BDBG_ASSERT(psStatus);
	*psStatus = hMachine->sStatus;
	return BERR_SUCCESS;
}
