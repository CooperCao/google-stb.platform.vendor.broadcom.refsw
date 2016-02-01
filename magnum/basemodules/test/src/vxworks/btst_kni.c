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

/* VxWorks include files */
#include "vxWorks.h"
#include "fioLib.h"
#include "tickLib.h"
#include "taskLib.h"
#include "intLib.h"

BDBG_MODULE(test_kni);


#define CALCULATE_SEC(x)	((x) / 1000000)
#define	CALCULATE_USEC(x)	((x) % 1000000)

#define	DEFAULT_STACK_SZ		(0x1000)	/* Default: 4K Bytes */
#define	DEFAULT_TASK_PRIORITY	(150)		/* Default: 150, which is less then Tornoda shell launch priority */

#define	GET_CURRENT_TIMER_TICK()	0

typedef struct BTST_TaskObj
{
	int taskId;
} BTST_TaskObj;

void BTST_GetTime(
	BTST_Time *pTime)
{
	static uint32_t lastTimerTick = 0;
	static uint32_t overflowTimerCnt = 0;
	uint32_t curTimerTick;
	unsigned overflowCnt;

	curTimerTick = GET_CURRENT_TIMER_TICK();
	if( curTimerTick < lastTimerTick )
	{
		overflowTimerCnt++;
		lastTimerTick = curTimerTick;
		overflowCnt = overflowTimerCnt;
		pTime->sec = (CALCULATE_SEC(0xFFFFFFFF) * overflowCnt) +
			CALCULATE_SEC(curTimerTick + (CALCULATE_USEC(0xFFFFFFFF) * overflowCnt));
		pTime->usec = CALCULATE_USEC(curTimerTick + (CALCULATE_USEC(0xFFFFFFFF) * overflowCnt));
	}
	else
	{
		pTime->sec = CALCULATE_SEC(curTimerTick);
		pTime->usec = CALCULATE_USEC(curTimerTick);
	}
}

BERR_Code BTST_CalcDeltaTime(
	BTST_Time *pT2,
	BTST_Time *pT1,
	BTST_Time *pDeltaTime)
{
	BERR_Code retVal;


	retVal = BERR_UNKNOWN;
	pDeltaTime->sec = 0;
	pDeltaTime->usec = 0;
	if( pT2->usec < 1000000 && pT1->usec < 1000000 )
	{
		if( (pT2->sec == pT1->sec && pT2->usec >= pT1->usec) )
		{
			pDeltaTime->sec = 0;
			pDeltaTime->usec = pT2->usec - pT1->usec;
			retVal = BERR_SUCCESS;
		}
		else if( pT2->sec > pT1->sec )
		{
			pDeltaTime->sec = pT2->sec - pT1->sec;
			if( pT2->usec >= pT1->usec )
			{
				pDeltaTime->usec = pT2->usec - pT1->usec;
			}
			else
			{
				pDeltaTime->sec--;
				pDeltaTime->usec = 1000000 - pT1->usec + pT2->usec;
			}
			retVal = BERR_SUCCESS;
		}
	}
	return(retVal);
}


/***************************************************************************
 *	Task Functions
 ***************************************************************************/
static int BTST_TaskStart(
	int hTask, int pTaskFn, int taskData,
	int data3, int data4, int data5,
	int data6, int data7, int data8,
	int data9)
{
	int (*pFunc)(BTST_TaskHandle, void *);
	int retVal;

	/* Use taskSafe()/taskUnsafe() to protect against unsafe task deletion.
	   This is behavior that is specified by the Kernel Interface */
/*	taskSafe(); */
	pFunc = (int (*)()) pTaskFn;
	(*pFunc)((BTST_TaskHandle) hTask, (void *) taskData);
	retVal = 0;
/*	taskUnsafe(); */
	return(retVal);
}

BERR_Code BTST_CreateTask(
	BTST_TaskHandle *phTask,
	BTST_TaskFunction pTaskFn,
	void *pData)
{
	BERR_Code retVal;
	BTST_TaskObj *pTaskObj;


	retVal = BERR_OS_ERROR;
	*phTask = NULL;
	pTaskObj = (BTST_TaskObj *) BKNI_Malloc(sizeof(BTST_TaskObj));
	if( pTaskObj != NULL )
	{
		pTaskObj->taskId = taskSpawn((char *) NULL, DEFAULT_TASK_PRIORITY,
			VX_FP_TASK, DEFAULT_STACK_SZ, (FUNCPTR) BTST_TaskStart,
			(int) pTaskObj, (int) pTaskFn, (int) pData,
			0, 0, 0,
			0, 0, 0, 0);
		if( pTaskObj->taskId != ERROR )
		{
			*phTask = pTaskObj;
			retVal = BERR_SUCCESS;
		}
	}
	if( retVal == BERR_OS_ERROR )
	{
		if( pTaskObj != NULL )
		{
			BKNI_Free((void *) pTaskObj);
		}
	}
	return(retVal);
}

BERR_Code BTST_DestroyTask(
	BTST_TaskHandle hTask)
{
	BERR_Code retVal;
	BTST_TaskObj *pTaskObj;


	retVal = BERR_SUCCESS;
	pTaskObj = (BTST_TaskObj *) hTask;
	BDBG_ASSERT(pTaskObj);
	if( taskIdVerify(pTaskObj->taskId) == OK )
	{
		if( taskDelete(pTaskObj->taskId) == ERROR )
		{
			switch( errno )
			{
				case S_objLib_OBJ_DELETED:
					/* Not an error, since the task may have exited */
					break;
				case S_intLib_NOT_ISR_CALLABLE:
				case S_objLib_OBJ_ID_ERROR:
				case S_objLib_OBJ_UNAVAILABLE:
				default:
					retVal = BERR_OS_ERROR;
					break;
			}
		}
	}
	if( retVal == BERR_SUCCESS )
	{
		BKNI_Free((void *) pTaskObj);
	}
	return(retVal);
}

