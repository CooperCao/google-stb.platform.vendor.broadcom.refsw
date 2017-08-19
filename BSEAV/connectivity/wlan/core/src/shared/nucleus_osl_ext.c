/*
 * Nucleus OS Support Extension Layer
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 * $Id$
 */

/* ---- Include Files ---------------------------------------------------- */

#include "typedefs.h"
#include "bcmdefs.h"
#include "bcmendian.h"
#include "bcmutils.h"
#include "osl_ext.h"
#include "nucleus.h"
#include <stdlib.h>
#include "pkt_lbuf.h"

/* ---- Public Variables ------------------------------------------------- */
/* ---- Private Constants and Types -------------------------------------- */

/* Nucleus-specific task priorities. */
#ifndef NUCLEUS_TASK_IDLE_PRIORITY
#define NUCLEUS_TASK_IDLE_PRIORITY		60
#endif

#ifndef NUCLEUS_TASK_LOW_PRIORITY
#define NUCLEUS_TASK_LOW_PRIORITY		50
#endif

#ifndef NUCLEUS_TASK_LOW_NORMAL_PRIORITY
#define NUCLEUS_TASK_LOW_NORMAL_PRIORITY	40
#endif

#ifndef NUCLEUS_TASK_NORMAL_PRIORITY
#define NUCLEUS_TASK_NORMAL_PRIORITY		30
#endif

#ifndef NUCLEUS_TASK_HIGH_NORMAL_PRIORITY
#define NUCLEUS_TASK_HIGH_NORMAL_PRIORITY	20
#endif

#ifndef NUCLEUS_TASK_HIGHEST_PRIORITY
#define NUCLEUS_TASK_HIGHEST_PRIORITY		10
#endif

#ifndef NUCLEUS_TASK_TIME_CRITICAL_PRIORITY
#define NUCLEUS_TASK_TIME_CRITICAL_PRIORITY	5
#endif


#define OSL_LOG(a)	printf a


/* ---- Private Variables ------------------------------------------------ */
/* ---- Private Function Prototypes -------------------------------------- */

static osl_ext_task_t *g_terminator_task = 0;
static osl_ext_queue_t g_terminator_queue;

static osl_ext_status_t osl_ext_error_check_nu(STATUS nu_status);
static VOID osl_ext_nu_task_entry(UNSIGNED argc, VOID* argv);


/* ---- Functions -------------------------------------------------------- */

/****************************************************************************
* Function:   osl_ext_error_check_nu
*
* Purpose:    Maps from Nucleus specific error code to OSL generic error code.
*
* Parameters: nu_status (in) Nucleus specific error code.
*
* Returns:    OSL_EXT_SUCCESS on success, else error code.
*****************************************************************************
*/
static osl_ext_status_t
osl_ext_error_check_nu(STATUS nu_status)
{
	osl_ext_status_t osl_ext_status;

	if (nu_status == NU_SUCCESS)
		osl_ext_status = OSL_EXT_SUCCESS;
	else if (nu_status == NU_TIMEOUT)
		osl_ext_status = OSL_EXT_TIMEOUT;
	else
		osl_ext_status = OSL_EXT_ERROR;

	return (osl_ext_status);
}


/* --------------------------------------------------------------------------
** Semaphores
*/

/* ----------------------------------------------------------------------- */
osl_ext_status_t
osl_ext_sem_create(char *name, int init_cnt, osl_ext_sem_t *sem)
{
	STATUS status;

	memset(sem, 0, sizeof(osl_ext_sem_t));
	status = NU_Create_Semaphore(sem, name, init_cnt, NU_FIFO);

	return (osl_ext_error_check_nu(status));
}


/* ----------------------------------------------------------------------- */
osl_ext_status_t
osl_ext_sem_delete(osl_ext_sem_t *sem)
{
	STATUS status;

	status = NU_Delete_Semaphore(sem);

	return (osl_ext_error_check_nu(status));
}


/* ----------------------------------------------------------------------- */
osl_ext_status_t
osl_ext_sem_give(osl_ext_sem_t *sem)
{
	STATUS status;

	status = NU_Release_Semaphore(sem);

	return (osl_ext_error_check_nu(status));
}


/* ----------------------------------------------------------------------- */
osl_ext_status_t
osl_ext_sem_take(osl_ext_sem_t *sem, osl_ext_time_ms_t timeout_msec)
{
	STATUS 	status;
	UNSIGNED	timeout_ticks;

	if (timeout_msec == 0)
		timeout_ticks = NU_NO_SUSPEND;
	else if (timeout_msec == OSL_EXT_TIME_FOREVER)
		timeout_ticks = NU_SUSPEND;
	else
		timeout_ticks = OSL_MSEC_TO_TICKS(timeout_msec);

	status = NU_Obtain_Semaphore(sem, timeout_ticks);

	return (osl_ext_error_check_nu(status));
}


/* --------------------------------------------------------------------------
** Mutex
*/

/* ----------------------------------------------------------------------- */
osl_ext_status_t
osl_ext_mutex_create(char *name, osl_ext_mutex_t *mutex)
{
	/* Nucleus doesn't have a mutex privimite, just use a binary sema4. */
	return (osl_ext_sem_create(name, 1, mutex));
}


/* ----------------------------------------------------------------------- */
osl_ext_status_t
osl_ext_mutex_delete(osl_ext_mutex_t *mutex)
{
	return (osl_ext_sem_delete(mutex));
}


/* ----------------------------------------------------------------------- */
osl_ext_status_t
osl_ext_mutex_acquire(osl_ext_mutex_t *mutex, osl_ext_time_ms_t timeout_msec)
{
	return (osl_ext_sem_take(mutex, timeout_msec));
}


/* ----------------------------------------------------------------------- */
osl_ext_status_t
osl_ext_mutex_release(osl_ext_mutex_t *mutex)
{
	return (osl_ext_sem_give(mutex));
}


/* --------------------------------------------------------------------------
** Timers
*/

/* ----------------------------------------------------------------------- */
osl_ext_status_t
osl_ext_timer_create(char *name, osl_ext_time_ms_t timeout_msec, osl_ext_timer_mode_t mode,
                 osl_ext_timer_callback func, osl_ext_timer_arg_t arg, osl_ext_timer_t *timer)
{
	STATUS status;

	UNSIGNED init_ticks;
	UNSIGNED reschedule_ticks = 0;

	memset(timer, 0, sizeof(osl_ext_timer_t));
	init_ticks = OSL_MSEC_TO_TICKS(timeout_msec);
	if (mode == OSL_EXT_TIMER_MODE_REPEAT)
		reschedule_ticks = OSL_MSEC_TO_TICKS(timeout_msec);

	status = NU_Create_Timer(timer,
	                         name,
	                         (void (*)(UNSIGNED))func,
	                         (UNSIGNED) arg,
	                         init_ticks,
	                         reschedule_ticks,
	                         NU_ENABLE_TIMER);

	return (osl_ext_error_check_nu(status));
}


/* ----------------------------------------------------------------------- */
osl_ext_status_t
osl_ext_timer_delete(osl_ext_timer_t *timer)
{
	STATUS	status;

	status = NU_Control_Timer(timer,	NU_DISABLE_TIMER);

	if (status == NU_SUCCESS)
		status = NU_Delete_Timer(timer);

	return (osl_ext_error_check_nu(status));
}


/* ----------------------------------------------------------------------- */
osl_ext_time_ms_t
osl_ext_time_get(void)
{
	return OSL_TICKS_TO_MSEC(NU_Retrieve_Clock());
}


/* --------------------------------------------------------------------------
** Tasks
*/

/* Map generic task priorities to Nucleus-specific priorities. */
static OPTION g_task_priority_table[OSL_EXT_TASK_NUM_PRIORITES] =
{
	NUCLEUS_TASK_IDLE_PRIORITY,
	NUCLEUS_TASK_LOW_PRIORITY,
	NUCLEUS_TASK_LOW_NORMAL_PRIORITY,
	NUCLEUS_TASK_NORMAL_PRIORITY,
	NUCLEUS_TASK_HIGH_NORMAL_PRIORITY,
	NUCLEUS_TASK_HIGHEST_PRIORITY,
	NUCLEUS_TASK_TIME_CRITICAL_PRIORITY
};


/****************************************************************************
* Function:   osl_ext_nu_task_entry
*
* Purpose:    Nucleus specific task entry function. Invokes the user registered
*             task entry function.
*
* Parameters: argc, argv (in) Task entry arguments.
*
* Returns:    Nothing.
*****************************************************************************
*/
static VOID
osl_ext_nu_task_entry(UNSIGNED argc, VOID* argv)
{
	osl_ext_task_entry	func;
	osl_ext_task_arg_t 	arg;
	osl_ext_task_t 	*task;

	/* 'argv' is actually a pointer to an OSL task object. */
	task = (osl_ext_task_t*)argv;
	func = (osl_ext_task_entry) task->func;
	arg = task->arg;

	/* Invoke the user register task entry function. */
	(*func)(arg);
}


/* ----------------------------------------------------------------------- */
osl_ext_status_t osl_ext_task_create_ex(char* name,
	void *stack, unsigned int stack_size,
	osl_ext_task_priority_t priority,
	osl_ext_time_ms_t timeslice_msec,
	osl_ext_task_entry func, osl_ext_task_arg_t arg,
	osl_ext_task_t *task)
{
	STATUS status;
	OPTION nu_priority;
	UNSIGNED timeslice_ticks;

	if (timeslice_msec == 0)
		timeslice_ticks = 0;
	else
		timeslice_ticks = OSL_MSEC_TO_TICKS(timeslice_msec);

	memset(task, 0, sizeof(osl_ext_task_t));

	/* Map from the generic OSL priority to a Nucleus specific priority. */
	nu_priority = g_task_priority_table[priority];

	/* Save the user registered task entry function and argument. */
	task->func = (void*) func;
	task->arg = arg;

	/* Dynamically allocate a stack. */
	if (stack == NULL) {
		task->stack = MALLOC(NULL, stack_size);
		task->stack_size = stack_size;
		ASSERT(task->stack);
		stack = task->stack;
	}

	/* Create the task... */
	status = NU_Create_Task(&task->nu_task,
	                        name,
	                        osl_ext_nu_task_entry,
	                        0,
	                        (void*) task,
	                        stack,
	                        stack_size,
	                        nu_priority,
	                        timeslice_ticks,
	                        NU_PREEMPT,
	                        NU_START);


	return (osl_ext_error_check_nu(status));
}


/* ----------------------------------------------------------------------- */
static osl_ext_status_t task_delete(osl_ext_task_t *task)
{
	STATUS status;

	status = NU_Terminate_Task(&task->nu_task);

	if (status == NU_SUCCESS)
		status = NU_Delete_Task(&task->nu_task);

	if (task->stack != NULL)
		MFREE(NULL, task->stack, task->stack_size);

	return (osl_ext_error_check_nu(status));
}


/* ----------------------------------------------------------------------- */
static void terminator_task_entry(void *arg)
{
	while (1) {
		osl_ext_task_t *task;
		osl_ext_queue_receive(&g_terminator_queue,
			OSL_EXT_TIME_FOREVER, (void **)&task);
		task_delete(task);
	}
}


/* ----------------------------------------------------------------------- */
osl_ext_status_t osl_ext_task_delete(osl_ext_task_t *task)
{
	osl_ext_status_t status;

	/* delete task if invoked from another task */
	if (&task->nu_task != NU_Current_Task_Pointer())
		return (task_delete(task));

	/* invoked from task itself so must delete task from terminator task */
	if (g_terminator_task == 0) {
		g_terminator_task = MALLOC(NULL, sizeof(*g_terminator_task));
		status = osl_ext_task_create("term", 512, OSL_EXT_TASK_NORMAL_PRIORITY,
			terminator_task_entry, 0, g_terminator_task);
		ASSERT(status == OSL_EXT_SUCCESS);
		status = osl_ext_queue_create("term", 8, &g_terminator_queue);
		ASSERT(status == OSL_EXT_SUCCESS);
	}

	status = osl_ext_queue_send_synchronous(&g_terminator_queue, task);
	ASSERT(status == OSL_EXT_SUCCESS);

	return OSL_EXT_SUCCESS;
}


/* --------------------------------------------------------------------------
** Queue
*/

/* ----------------------------------------------------------------------- */
osl_ext_status_t
osl_ext_queue_create(char *name,
	void *queue_buffer, unsigned int size, osl_ext_queue_t *queue)
{
	STATUS status;
	UNSIGNED msg_size = 1; /* Number of UNSIGNED data elements in message. */

	/* The OSL extension API uses 'void *' messages for queues; whereas,
	 * Nucleus uses UNSIGNED data elements. For simplification, the
	 * implementation relies upon the fact that these types are
	 * the same size.
	 */
	ASSERT(sizeof(void*) == sizeof(UNSIGNED));

	memset(queue, 0, sizeof(osl_ext_queue_t));

	/* Dynamically allocate queue buffer. */
	if (queue_buffer == NULL) {
		queue->nu_buffer_size = sizeof(UNSIGNED) * size;
		queue->nu_buffer = MALLOC(NULL, queue->nu_buffer_size);
		ASSERT(queue->nu_buffer);
		queue_buffer = queue->nu_buffer;
	}

	/* Create queue. */
	status = NU_Create_Queue(&queue->nu_queue, name, queue_buffer,
		size, NU_FIXED_SIZE, msg_size, NU_FIFO);

	return (osl_ext_error_check_nu(status));
}


/* ----------------------------------------------------------------------- */
osl_ext_status_t
osl_ext_queue_delete(osl_ext_queue_t *queue)
{
	STATUS status;

	status = NU_Delete_Queue(&queue->nu_queue);

	if (queue->nu_buffer != NULL)
		MFREE(NULL, queue->nu_buffer, queue->nu_buffer_size);

	return (osl_ext_error_check_nu(status));
}


/* ----------------------------------------------------------------------- */
osl_ext_status_t
osl_ext_queue_bind(osl_ext_queue_t *queue, osl_ext_task_t *task)
{
	return OSL_EXT_SUCCESS;
}


/* ----------------------------------------------------------------------- */
osl_ext_status_t
osl_ext_queue_send(osl_ext_queue_t *queue, void *data)
{
	STATUS status;
	UNSIGNED msg_size = 1; /* Number of UNSIGNED data elements in message. */

	status = NU_Send_To_Queue(&queue->nu_queue, &data,
		msg_size, NU_NO_SUSPEND);

	return (osl_ext_error_check_nu(status));
}


/* ----------------------------------------------------------------------- */
osl_ext_status_t
osl_ext_queue_send_synchronous(osl_ext_queue_t *queue, void *data)
{
	STATUS status;
	UNSIGNED msg_size = 1; /* Number of UNSIGNED data elements in message. */

	status = NU_Send_To_Queue(&queue->nu_queue, &data,
		msg_size, NU_SUSPEND);

	return (osl_ext_error_check_nu(status));
}


/* ----------------------------------------------------------------------- */
osl_ext_status_t
osl_ext_queue_receive(osl_ext_queue_t *queue, osl_ext_time_ms_t timeout_msec, void **data)
{
	STATUS status;
	unsigned timeout_ticks;
	UNSIGNED size;
	UNSIGNED msg_size = 1; /* Number of UNSIGNED data elements in message. */

	if (timeout_msec == 0)
		timeout_ticks = NU_NO_SUSPEND;
	else if (timeout_msec == OSL_EXT_TIME_FOREVER)
		timeout_ticks = NU_SUSPEND;
	else
		timeout_ticks = OSL_MSEC_TO_TICKS(timeout_msec);

	status = NU_Receive_From_Queue(&queue->nu_queue, data,
		msg_size, &size, timeout_ticks);

	return (osl_ext_error_check_nu(status));
}


/* ----------------------------------------------------------------------- */
osl_ext_status_t osl_ext_task_yield(void)
{
	NU_Relinquish();

	return (OSL_EXT_SUCCESS);
}
