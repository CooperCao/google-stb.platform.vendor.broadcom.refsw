/***************************************************************************
*     Copyright (c) 2004-2012, Broadcom Corporation
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
#include "bkni.h"

#ifndef BSYSLIB_H__
#define BSYSLIB_H__

/* 20071214 bandrews - Removed.  Replace with scheduling a zero-length timer */
#ifdef BSYSLIB_TASK_SUPPORT

/*
Summary:
Delegate signature for a generic task function.  This is a task-time call,
not a new OS "task" / thread.
Description:
A task delegate to be run at task time
*/
typedef void (*BSYSlib_Task)
(
	void * pvParm1, /* user context param 1 */
	int iParm2 /* user context param 2 */
);

/*
Summary:
Settings for starting a task
*/
typedef struct
{
	BSYSlib_Task pfDoTask; /* the task to start */ 
	void * pvParm1; /* first user context parameter to the task delegate */ 
	int iParm2; /* second user context parameter to the task delegate */ 
} BSYSlib_Task_Settings;

/*
Summary:
Task-scheduling isr-time delegate signature
Description:
Schedules a task to be run out of isr context
*/
typedef BERR_Code (*BSYSlib_Task_Schedule_isr)
(
	void * pvParm1, /* first user context parameter to the schedule function [in] */ 
	int iParm2, /* second user context parameter to the schedule function [in] */ 
	const BSYSlib_Task_Settings * psSettings /* the task settings [in] */ 
);

/*
Summary:
Encapsulates the task callback function plus context
*/
typedef struct BSYSlib_TaskCallback
{
	BSYSlib_Task_Schedule_isr pfSchedule_isr; /* schedule signature */
	void * pvParm1; /* first user context parameter used for the task callback */
	int iParm; /* second user context parameter used for the task callback */
} BSYSlib_TaskCallback;

#endif

/*
Summary:
A task-time event-handling delegate signature
Description:
Called when the specified event this handler was registered for occurs
*/
typedef BERR_Code (*BSYSlib_Event_Handler)
(
	void * pvParm1, /* first user context parameter [in] */ 
	int iParm2, /* second user context parameter [in] */ 
	BKNI_EventHandle hEvent /* the event that occurred [in] */
);

/*
Summary:
Settings for listening to an event
*/
typedef struct
{
	BKNI_EventHandle hEvent; /* the event to be handled */
	BSYSlib_Event_Handler pfEventOccurred; /* the delegate to handle the event */
	void * pvParm1; /* user context param for the event handler */
	int iParm2; /* user context param2 for the event handler */
} BSYSlib_Event_Settings;

/*
Summary:
Adds an event handler for the specified event
Description:
Task-time delegate signature for adding event handlers to events
*/
typedef BERR_Code (*BSYSlib_Event_AddHandler)
(
	void * pvParm1, /* user context param for the add function [in] */
	int iParm2, /* user context param2 for the add function [in] */
	const BSYSlib_Event_Settings * psSettings /* the event handler settings for the handler to add [in] */
);

/*
Summary:
Removes an event handler for the specified event
Description:
Task-time delegate signature for removing event handlers from events
*/
typedef BERR_Code (*BSYSlib_Event_RemoveHandler)
(
	void * pvParm1, /* user context param for the remove function [in] */
	int iParm2, /* user context param2 for the remove function [in] */
	const BSYSlib_Event_Settings * psSettings /* the event handler settings for the handler to remove [in] */
);

/*
Summary:
Encapsulates all event-related callback functions plus context
*/
typedef struct BSYSlib_EventCallback
{
	BSYSlib_Event_AddHandler pfAdd; /* add signature */
	BSYSlib_Event_RemoveHandler pfRemove; /* remove signature */
	void * pvParm1; /* first user context parameter used for the event callbacks */
	int iParm2; /* second user context parameter used for the event callbacks */
} BSYSlib_EventCallback;

/*
Summary:
Generic timer handle 
*/ 
typedef void * BSYSlib_Timer_Handle;

/*
Summary:
Edge-triggered timer expiry notification function
Description:
Task-time timer expiry delegate signature 
*/ 
typedef BERR_Code (*BSYSlib_Timer_ExpiryHandler) 
( 
	void * pvParm1, /* first user context parameter [in] */ 
	int iParm2, /* second user context parameter [in] */ 
	BSYSlib_Timer_Handle hTimer /* the handle of the timer that expired [in] */ 
);

/*
Summary:
Settings for starting a timer
*/
typedef struct
{
	unsigned long ulTimeout; /* timeout of timer */
	BSYSlib_Timer_ExpiryHandler pfTimerExpired; /* the handler function to call back when the timer expires */
	void * pvParm1; /* first user context parameter to the expiry handler function */
	int iParm2; /* second user context parameter to the expiry handler function */ 
} BSYSlib_Timer_Settings;

/*
Summary:
Timer creation delegate.
Description:
The system will allocate resources for a timer.  The system should keep track of the timer through the specified handle. 
*/
typedef BERR_Code (*BSYSlib_Timer_Create) 
( 
	void * pvParm1, /* first user context parameter to the start function [in] */ 
	int iParm2, /* second user context parameter to the start function [in] */ 
	BSYSlib_Timer_Handle * phTimer /* handle of timer [out] */ 
);

/*
Summary:
Timer destruction delegate
Description:
The system will destroy resources associated with the specified timer handle.
*/
typedef void (*BSYSlib_Timer_Destroy) 
( 
	void * pvParm1, /* first user context parameter to the start function [in] */ 
	int iParm2, /* second user context parameter to the start function [in] */ 
	BSYSlib_Timer_Handle hTimer /* handle of timer [in] */ 
);

/*
Summary:
Timer start delegate.
Description:
The system will start the timer specified by handle (and created earlier) with the specified timeout and callback function. 
*/
typedef BERR_Code (*BSYSlib_Timer_Start_isr) 
( 
	void * pvParm1, /* first user context parameter to the start function [in] */ 
	int iParm2, /* second user context parameter to the start function [in] */ 
	BSYSlib_Timer_Handle hTimer, /* the handle of the timer */
	const BSYSlib_Timer_Settings * psSettings /* the timer settings [in] */
);

/*
Summary:
Timer cancel delegate.
Description:
The system will cancel the timer with the specified handle. 
*/
typedef BERR_Code (*BSYSlib_Timer_Cancel_isr)
(
	void * pvParm1, /* first user context parameter [in] */ 
	int iParm2, /* second user context parameter [in] */ 
	BSYSlib_Timer_Handle hTimer /* handle of timer [in] */ 
);

/*
Summary:
Encapsulates all timer-related callback functions plus context
*/
typedef struct BSYSlib_TimerCallback
{
	BSYSlib_Timer_Create pfCreate; /* create signature */
	BSYSlib_Timer_Destroy pfDestroy; /* destroy signature */
	BSYSlib_Timer_Start_isr pfStart_isr; /* start signature */
	BSYSlib_Timer_Cancel_isr pfCancel_isr; /* cancel signature */
	void * pvParm1; /* first user context parameter used for the timer callbacks */
	int iParm2; /* second user context parameter used for the timer callbacks */
} BSYSlib_TimerCallback;

/*
Summary:
Retrieves the current system time
*/
typedef BERR_Code (*BSYSlib_GetTime_isr)
(
	void * pvParm1, /* first user context parameter to the get time function [in] */ 
	int iParm2, /* second user context parameter to the get time function [in] */ 
	unsigned long * pulNow /* the current time in ms [out] */ 
);

/*
Summary:
Encapsulates system-time-related callback functions plus context
*/
typedef struct BSYSlib_TimeCallback
{
	BSYSlib_GetTime_isr pfGetTime_isr; /* get time signature */
	void * pvParm1; /* first user context parameter used for the get time callback */
	int iParm2; /* second user context parameter used for the get time callback */
} BSYSlib_TimeCallback;

#endif /* BSYSLIB_H__ */

