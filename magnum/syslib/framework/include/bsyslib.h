/***************************************************************************
 *  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#include "bkni.h"

#ifndef BSYSLIB_H__
#define BSYSLIB_H__

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
typedef void (*BSYSlib_Timer_Cancel_isr)
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
typedef void (*BSYSlib_GetTime_isr)
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

