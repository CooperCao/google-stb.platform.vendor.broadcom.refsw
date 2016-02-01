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
#ifndef BKNI_APP_H__
#define BKNI_APP_H__

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
Summary:
	Task object initialized by BKNI_CreateTask.
****************************************************************************/
typedef struct BKNI_TaskObj *BKNI_TaskHandle;

/***************************************************************************
Summary:
	Function which will be called by the newly created task. When this
	function exits, the task is done executing.
Returns:
	The return value will be captured and returned in the retcode parameter
	of BKNI_DestroyTask.
****************************************************************************/
typedef int (*BKNI_TaskFunction)(BKNI_TaskHandle task, void *data);


/***************************************************************************
Summary:
	Enum which defines the priority of the task. This priority is not guaranteed
	but is merely a recommendation to the platform scheduler.
****************************************************************************/
typedef enum {
  BKNI_HighPriority,
  BKNI_MedPriority,
  BKNI_LowPriority
} BKNI_TaskPriority;

/***************************************************************************
Summary:

Returns:

****************************************************************************/
BERR_Code BKNI_CreateTask(BKNI_TaskHandle *task,
  BKNI_TaskFunction task_fn, BKNI_TaskPriority priority, void *data);


/***************************************************************************
Summary:

Description:
	Wait for the task to exit, then return the BKNI_TaskFunction return code through the
	returnCode out parameter. If the task function has already returned, it returns the
	saved return code. BKNI_DestroyTask releases all resources associated with BKNI_Task,
	and that object can't be used any more.

	BKNI_DestroyTask doesn't force the task to exit - this is the user's
	responsibility. However BKNI_DestroyTask might interrupt any blocked
	kernel interface functions.

Returns:
	BERR_SUCCESS - The task was destroyed and the returnCode is correct.
	BERR_OS_ERROR - The task may or may not have been destroyed. The value
		pointed to by returnCode is invalid.
****************************************************************************/
BERR_Code BKNI_DestroyTask(BKNI_TaskHandle task, int *returnCode);


/***************************************************************************
Summary:
Changes the recommended priority for a task.

Returns:
	BERR_SUCCESS - The system accepted the priority. There is no guarantee that
		this task will execute before or more frequently than a lower priority
		task.
	BERR_OS_ERROR - The system was unable to accept the priority.
****************************************************************************/
BERR_Code BKNI_SetTaskPriority(BKNI_TaskHandle task, BKNI_TaskPriority priority);

#ifdef __cplusplus
}
#endif

#endif /* BKNI_APP_H__ */
