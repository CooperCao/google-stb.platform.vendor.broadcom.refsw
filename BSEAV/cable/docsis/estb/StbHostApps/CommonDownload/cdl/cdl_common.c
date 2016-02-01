/***************************************************************************
 *     Copyright (c) 2007-2014, Broadcom Corporation
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
 * Module Description:  common routines
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/

#include "cdl_common.h"

BDBG_MODULE(cdl_common);

int cdl_create_thread
(
	cdl_thread_info *thread_info,
	void        *thread_fn,
	void        *thread_param
)
{
	int err;
		
	/* Init Vital Stats */
	thread_info->thread_fn       = thread_fn;
	thread_info->thread_param    = thread_param;
	thread_info->private_info  = 0;
	thread_info->priority      = PRI_NORMAL;
	thread_info->exit          = 0;
	
	/*
	**  Initialize the thread attributes to default values.
	**  Then modify the attributes to make a real-time thread.
	*/
	err = pthread_attr_init( &(thread_info->attr) );
	if ( err )
	{
		BDBG_ERR(("pthread_attr_init failed %x!\n", err));
		return(-1);
	}
	
	err = pthread_attr_setschedpolicy( &(thread_info->attr), SCHED_OTHER);
	if ( err )
	{
		BDBG_ERR(("pthread_attr_setschedpolicy failed %x!\n", err));
		return(-1);
	}


	err = pthread_attr_getschedparam( &(thread_info->attr), &(thread_info->param) );
	if ( err )
	{
		BDBG_ERR(("pthread_attr_getschedparam failed %x!\n", err));
		return(-1);
	}
	
	/* Create thread */
	err = pthread_create(&thread_info->thread_id, &(thread_info->attr),
		(void *) thread_info->thread_fn, 
		(void*)thread_info->thread_param);
	
	if ( err )
	{
		BDBG_ERR(("pthread_create failed %x!\n", err));
		return(-1);
	}
	
	
	return(0);
}



int cdl_kill_thread( cdl_thread_info *thread_info, int wait)
{
	void * ret;
                                 
	/* cancel a thread */
	pthread_cancel(thread_info->thread_id);
	thread_info->exit = 1;
	if (wait) {
		/* wait for thread to return */
		pthread_join(thread_info->thread_id, &ret);
	}
	return 0;
}

int cdl_detach_thread( cdl_thread_info *thread_info )
{
	int err;

	err = pthread_detach(thread_info->thread_id);

	if ( err )
	{
		BDBG_ERR(("pthread_detach failed %x!\n", err));
		return(-1);
	}

	return(0);
}

int cdl_wait_thread_done (cdl_thread_info *thread_info)
{
	pthread_join(thread_info->thread_id, NULL);
}

int cdl_set_thread_priority
(
	cdl_thread_info      *thread_info,
	cdl_thread_priority priority
)
{
	int  sched_priority;
	int     ret;
	
	
	
	/* Set priority level for this process */
	/* for now it is hard coded and assumed the range of FIFO scheduler
	   is from 1 to 99 (1 for lowest and 99 for highest)  */
	switch( priority )
	{
	case PRI_NORMAL:
		sched_priority = 50;
		break;
	case PRI_LOWEST:
		sched_priority = 1;
		break;
	case PRI_BELOW_NORMAL:
		sched_priority = 25;
		break;
	case PRI_ABOVE_NORMAL:
		sched_priority = 75;
		break;
	case PRI_HIGHEST:
		sched_priority = 99;
		break;
	default:
		sched_priority = 50;
		break;
	}
	
	(thread_info->param).sched_priority = sched_priority;
	ret =pthread_attr_setschedparam( &(thread_info->attr), &(thread_info->param) );
	if ( ret != 0 )
	{
		BDBG_ERR(("pthread_attr_setschedparam failed %x!\n", ret));
		return(-1);
	}
	else
	{
		thread_info->priority = priority;
	}
	
    return (0);
}


