/***************************************************************************
*     Copyright (c) 2004-2008, Broadcom Corporation
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

#include "bsynclib_state_machine.h"

#ifndef BSYNCLIB_STATE_MACHINE_PRIV_H__
#define BSYNCLIB_STATE_MACHINE_PRIV_H__

typedef struct
{
	BSYNClib_StateMachine_State eState;
} BSYNClib_StateMachine_Data;

/*
Summary:
*/
struct BSYNClib_StateMachine_Impl
{
	BSYNClib_StateMachine_Settings sSettings;
	BSYNClib_StateMachine_Data sData;
	BSYNClib_StateMachine_Status sStatus;
};

/*
Summary:
*/
BERR_Code BSYNClib_StateMachine_FireStateChangeCallback(
	BSYNClib_StateMachine_Handle hMachine
);

#endif /* BSYNCLIB_STATE_MACHINE_PRIV_H__ */

