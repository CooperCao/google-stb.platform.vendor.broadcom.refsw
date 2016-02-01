/***************************************************************************
 *     (c)2007-2008 Broadcom Corporation
 *
 *  This program is the proprietary software of Broadcom Corporation and/or its licensors,
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

	if (thread_info->thread_id== NULL) return -1;

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

	if (thread_info->thread_id==NULL) return -1;

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
	if (thread_info->thread_id==NULL) return -1;

	return pthread_join(thread_info->thread_id, NULL);
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
