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
#ifndef BTST_KNI_H__
#define BTST_KNI_H__


#ifdef __cplusplus
extern "C" {
#endif

typedef struct BTST_TaskObj *BTST_TaskHandle;

/***************************************************************************
Summary:
	Function which will be called by the newly created task. When this
	function exits, the task is done executing.
****************************************************************************/
typedef void (*BTST_TaskFunction)(BTST_TaskHandle task, void *data);

typedef struct
{
	uint32_t sec;			/* in Seconds */
	uint32_t usec;			/* in MicroSeconds, guarantee to be less then 1000000 */
} BTST_Time;


/***************************************************************************
Summary:
	This function returns the current time in seconds and microseconds.
	
Description:
	This function is a platform/OS specific function.  It is responsible
	for returning the current time from the power-on, in seconds and 
	microseconds.  The microseconds value is guarantee to be less than
	one second or 1000000.
	
Input:
	pTime - ptr to returning time
	
Returns:
 	None

****************************************************************************/
void BTST_GetTime(BTST_Time *pTime);


/***************************************************************************
Summary:
	This function returns the delta time, difference for T2 and T1.
	
Description:
	This function returns the difference between T2 and T1, (T2-T1).
	If T1 > T2, then error is returned.
	
Input:
	pT2 - ptr to T2 time
	pT1 - ptr to T1 time
	pDeltaTime - ptr to returning delta time
	
Returns:
 	BERR_SUCCESS - T2 >= T1, delta time was calculated
 	BERR_UNKNOWN - T1 > T1, delta time is 0, error

****************************************************************************/
BERR_Code BTST_CalcDeltaTime(BTST_Time *pT2, BTST_Time *pT1, BTST_Time *pDeltaTime);


/***************************************************************************
Summary:
	This function creates a task.

Description:
	This is an OS specific specific function.  It is responsible for
	creating a task.

Input:
	phTask - ptr to returning task handle
	pTaskFn - ptr to function, entry point for task
	pData - ptr to void data, which is passed to pTaskFn

Returns:
	BERR_SUCCESS - The task was destroyed and the returnCode is correct.
	BERR_OS_ERROR - The task may or may not have been destroyed.
****************************************************************************/
BERR_Code BTST_CreateTask(BTST_TaskHandle *phTask, BTST_TaskFunction pTaskFn, void *pData);


/***************************************************************************
Summary:
	This function destroys a task.

Description:
	This is an OS specific specific function.  It is responsible for
	destroying a task.

Input:
	task - handle to an existing task

Returns:
	BERR_SUCCESS - The task was destroyed and the returnCode is correct.
	BERR_OS_ERROR - The task may or may not have been destroyed.
****************************************************************************/
BERR_Code BTST_DestroyTask(BTST_TaskHandle task);


#ifdef __cplusplus
}
#endif

#endif  /* BTST_KNI_H__ */
