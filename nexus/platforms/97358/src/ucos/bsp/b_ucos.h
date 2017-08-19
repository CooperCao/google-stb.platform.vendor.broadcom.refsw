/***************************************************************************
 *  Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 ***************************************************************************/

#ifndef _B_UCOS_H_
#define _B_UCOS_H_

/****************************************************************************
****************************************************************************/
int
b_ucos_main(int argc, char ** argv);

/****************************************************************************
****************************************************************************/
void
b_ucos_delay(unsigned int microsec);

/****************************************************************************
    The old BKNI interface for Tasks and Qs. We're still migrating away
    from this. 
****************************************************************************/

typedef struct BKNI_QObject * BKNI_QHandle;

/****************************************************************************
Summary:
	Task object initialized by BKNI_CreateTask.
****************************************************************************/
typedef struct BKNI_TaskObj *BKNI_TaskHandle;

/****************************************************************************
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

/***************************************************************************
Summary:
    Create an OS task and assign the priority

Description:

Input:
    ppbcmTask - 
    task_fn - task function pointer
    task_priority - task priority
    task_param - parameter to be passed to the task function

Returns:
    BERR_SUCCESS
    BERR_OUT_OF_SYSTEM_MEMORY
    BERR_INVALID_PARAMETER
    BERR_OS_ERROR
****************************************************************************/
BERR_Code BKNI_AddTask(BKNI_TaskHandle *task,
                       BKNI_TaskFunction task_fn,
                       uint32_t task_priority, 
                       void *task_param);

/***************************************************************************
Summary:
Changes the recommended priority for a task.

Returns:
	BERR_SUCCESS - The system accepted the priority. There is no guarantee that
		this task will execute before or more frequently than a lower priority
		task.
	BERR_OS_ERROR - The system was unable to accept the priority.
****************************************************************************/
BERR_Code BKNI_ChangeTaskPriority(BKNI_TaskHandle task,
                                  uint32_t task_priority);


/***************************************************************************
Summary:
    Create an Queue object

Description:

Input:
    ppbcmQ - location to store the queue
    ulLastCount - The size of message queue storage area.
    pcQName - The queue name.

Returns:
    BERR_SUCCESS
    BERR_OUT_OF_SYSTEM_MEMORY
    BERR_OS_ERROR
****************************************************************************/
BERR_Code BKNI_CreateQ (BKNI_QHandle *q, 
                        uint32_t     ulLastCount,
                        char         *qName);
/***************************************************************************
Summary:
    Send a message to message queue.

Description:

Input:
    pbcmQ - the queue
    msg - the message
    size - the message size in bytes

Returns:
    BERR_SUCCESS
    BERR_NOT_INITIALIZED
    BERR_OUT_OF_SYSTEM_MEMORY
    BERR_OS_ERROR
****************************************************************************/
BERR_Code BKNI_SendQ (BKNI_QHandle  q,
                      void          *msg,
                      uint32_t      size);

/***************************************************************************
Summary:
    Return when a message is sent into the message queue or when the 
    time-out interval elapses

Description:

Input:
    pbcmQ - The queue
    msg - 
    Timeout - 

Returns:
    BERR_SUCCESS
    BERR_NOT_INITIALIZED
    BERR_OS_ERROR
****************************************************************************/
BERR_Code BKNI_ReceiveQ(BKNI_QHandle q,
                        void         *msg,
                        uint32_t     Timeout);


#endif /* _B_UCOS_H_ */

