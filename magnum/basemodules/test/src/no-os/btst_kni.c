/***************************************************************************
 *     Copyright (c) 2003, Broadcom Corporation
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
#include "btst_kni.h"





typedef struct BTST_TaskObj
{
	/* TODO: Please insert your target OS requirements */
	int dummy;
} BTST_TaskObj;

void BTST_GetTime(
	BTST_Time *pTime)
{
	/* TODO: Please instead your platform specific code to get the current timer value */
}

BERR_Code BTST_CalcDeltaTime(
	BTST_Time *pT2,
	BTST_Time *pT1,
	BTST_Time *pDeltaTime)
{
	BERR_Code retVal = 0;
	return(retVal);
}


/***************************************************************************
 *	Task Functions
 ***************************************************************************/
BERR_Code BTST_CreateTask(
	BTST_TaskHandle *phTask,
	BTST_TaskFunction pTaskFn,
	void *pData)
{
	BERR_Code retVal = 0;
	return(retVal);
}

BERR_Code BTST_DestroyTask(
	BTST_TaskHandle hTask)
{
	BERR_Code retVal = 0;
	return(retVal);
}

