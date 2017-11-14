/***************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *
 * Module Description:
 *
 ***************************************************************************/

/* uCOS include files */
#include "ucos_ii.h" 
#include "bstd.h"
#include "bkni.h"
#include "bdbg_os_priv.h"
#include <sys/time.h>

/****************************************************************************
    Global functions
****************************************************************************/
/* tap into bkni's function to check for a BKNI critical section. It's ugly,
   but there is no BKNI standard interface to do this.
*/
/* 
	JPF - The PI often calls BDBG_MSG which calls BDBG_P_Lock_isrsafe in critical sections
	BDBG_P_Lock_isrsafe uses a mutex which causes scheduling which is not legal in a real
	critical section.  One possible workaround is to not do real critical sections.
	The current workaround is to have CHECK_CRITICAL return true when uCos 
	OSLockNesting > 0.
  */
extern unsigned char OSLockNesting;
extern unsigned char OSIntNesting;
extern uint32_t total_ticks;

#define CHECK_CRITICAL() ( bos_in_interrupt() || (OSLockNesting > 0) || (OSIntNesting > 0))
#define TICKS_TO_MS(ticks)	(ticks * 1000/OS_TICKS_PER_SEC)
#define MS_TO_TICKS(x)      ((x * OS_TICKS_PER_SEC)/ 1000)
#define BOS_PEND_FOREVER	(-1)

/****************************************************************************
    Static data
****************************************************************************/
typedef unsigned int b_task_t;
typedef unsigned int b_queue_t;
typedef unsigned int b_event_t;
typedef struct b_mutex_t
{
	b_queue_t queue;
	b_event_t event_queue;
	b_event_t event;
}b_mutex_t;

/* platform spefic type for timestamps */
static unsigned int initTicks;
/* OS-specific mutex */
static b_mutex_t g_BdbgMutex;
static b_mutex_t *g_pBdbgMutex = NULL;

/****************************************************************************
****************************************************************************/
void
BDBG_P_InitializeTimeStamp(void)
{
    initTicks = OSTimeGet();
}

/****************************************************************************
****************************************************************************/
void
BDBG_P_GetTimeStamp_isrsafe(char *timeStamp, int size_t)
{
    unsigned int deltaTicks;
    unsigned int milliseconds;

    deltaTicks = OSTimeGet() - initTicks;
    milliseconds = TICKS_TO_MS(deltaTicks);

    /* print the formatted time including the milliseconds  */
    BKNI_Snprintf(timeStamp, size_t, "%08u", milliseconds);
    return;
}

/*****************************************************************************
 gettimeofday()
*****************************************************************************/
int gettimeofday(struct timeval *t, struct timezone *tz)
{
    t->tv_sec = total_ticks/100;
    t->tv_usec = (total_ticks%100)*10000;
	return 0;
}

/***************************************************************************
Summary:
    Macro to write a cp0 register.

Description:
    asm macro to write a cp0 register given the register, select and value. (MIPS32)

See Also:
    bcm_read_cp0
***************************************************************************/
#define bcm_write_cp0(reg, sel, value)                  \
{       __asm__ __volatile__(".set\tpush\n\t"           \
            ".set\tmips32\n\t"                          \
            "mtc0\t%z0, " #reg ", " #sel "\n\t" \
            ".set\tpop\n\t"                         \
            : /* none */                                \
            : "r" ((unsigned int)value));               \
}

/***************************************************************************
Summary:
    Macro to read a cp0 register.

Description:
    asm macro to read a cp0 register given the register and select. (MIPS32)

See Also:
    bcm_read_cp0
***************************************************************************/
#define bcm_read_cp0(reg, sel)                          \
({ unsigned int bcm_read_cp0_res;                       \
        __asm__ __volatile__(   ".set\tpush\n\t"        \
            ".set\tmips32\n\t"                          \
            "mfc0\t%0, " #reg ", " #sel "\n\t"          \
            ".set\tpop\n\t"                         \
            : "=r" (bcm_read_cp0_res));                 \
    bcm_read_cp0_res;                                   \
})

/*****************************************************************************
 bos_in_interrupt()
*****************************************************************************/
bool bos_in_interrupt()
{
   uint32_t status;

#ifdef GHS
   status = __MFC0(12, 0);
#else
   status = bcm_read_cp0($12, 0);
#endif   
   return (status & 0x00000002) ? true : false;
}

/******************************************************************************
* Post and event to the queue..
******************************************************************************/
BERR_Code bos_post_event(
					  b_queue_t handle,	  /* event queue */
					  b_event_t *event		  /* event to post */
					  )
{
	INT8U err;
	if ((err = OSQPost((OS_EVENT*)handle,event)) != OS_ERR_NONE)
	{
		/*BKNI_Printf("OSQPost err = %d\n",err);*/

		return BERR_OS_ERROR;
	}
	return BERR_SUCCESS;
}

/******************************************************************************
* Wait for an event..
******************************************************************************/
b_event_t *bos_pend_event(
						 b_queue_t handle,	 /* event queue */
						 int timeout_ms	 /* timeout in milliseconds */
						 )
{
	INT8U err;
	if (!handle)
	{
		volatile int *null=0;*null=*null;
	}

	if (timeout_ms == 0)
	{
		return(b_event_t*)OSQAccept((OS_EVENT*)handle,&err);
	}
	else if (timeout_ms < 0)
	{
		b_event_t *p_evt;
		while ((p_evt = (b_event_t*)OSQPend((OS_EVENT*)handle,0xFFFF,&err)) == NULL)
		{
			if ((err != OS_ERR_NONE) && (err != OS_ERR_TIMEOUT))
			{
				/*BKNI_Printf("OSQPend err = %d\n",err);*/
				/*__asm__("sdbbp");*/
				p_evt = NULL;

				break;
			}
		}
		return p_evt;
	}
	else if (timeout_ms < TICKS_TO_MS(1))
	{ /* set timeout as 1 tick if timeout_ms is lower than 1 tick time */
		timeout_ms = TICKS_TO_MS(1);    
	}
	return(b_event_t*)OSQPend((OS_EVENT*)handle,MS_TO_TICKS(timeout_ms),&err);
}

/******************************************************************************
* create an event queue using the even pool provided.
******************************************************************************/
BERR_Code bos_create_mutex(
						b_mutex_t *handle		/* mutex reference */
						)
{
	OS_EVENT* p_os_event = OSQCreate((void**)&handle->event_queue,1);
	if (p_os_event)
	{
		handle->queue = (b_queue_t)p_os_event;
		return bos_post_event(handle->queue,&(handle->event));
	}
	return BERR_OS_ERROR;
}

/******************************************************************************
* delete an event queue..
******************************************************************************/
void bos_delete_mutex(
					 b_mutex_t *handle	 /* mutex reference */
					 )
{
#if OS_Q_DEL_EN > 0
	INT8U           err;
	OSQDel((OS_EVENT*)handle->event_queue,OS_DEL_ALWAYS,&err);
#endif
}

/******************************************************************************
* Post and event to the queue..
******************************************************************************/
BERR_Code bos_acquire_mutex(
						 b_mutex_t *handle,	 /* mutex reference */
						 int timeout_ms	 /* timeout in milliseconds */
						 )
{
	return(bos_pend_event(handle->queue,timeout_ms) != NULL) ? BERR_SUCCESS : BERR_TIMEOUT;
}

/******************************************************************************
* Wait for an event..
******************************************************************************/
BERR_Code bos_release_mutex(
						 b_mutex_t *handle	 /* event queue */
						 )
{
	return bos_post_event(handle->queue,&handle->event);
}

/****************************************************************************
****************************************************************************/
BERR_Code 
BDBG_P_OsInit(void)
{
    BERR_Code rc;
    
    /* This lock needs to work between both ISR and Task contexts. We're using
       a mutex because we only have a tiny amount of code running in ISR 
       context -- the L1 handler -- and we don't expect it to need the BDBG 
       lock.
       
       The lock is only used when a debug module is registered for the first
       time.
       
       We need to go directly to the OS for our mutex because we have code
       in BKNI which checks to make sure no one is trying to call AcquireMutex
       from within a Magnum ISR context. 
    */
    if (g_pBdbgMutex != NULL) {
#if 0
        rc = BERR_TRACE(BERR_UNKNOWN);
        BKNI_Fail();
#else
		return BERR_SUCCESS;
#endif
    }

	if (bos_create_mutex(&g_BdbgMutex) != BERR_SUCCESS)
	{
        rc = BERR_TRACE(BERR_OS_ERROR);
        BKNI_Fail();
	}

    g_pBdbgMutex = &g_BdbgMutex;
    
    return BERR_SUCCESS;
}

/****************************************************************************
****************************************************************************/
void 
BDBG_P_OsUninit(void)
{
    /* No way to release kernel object in uCOS */
    BKNI_Printf("%s: uCOS Event Leak - no method to destroy semaphore\n", BSTD_FUNCTION);
    if (g_pBdbgMutex) {
		bos_delete_mutex(g_pBdbgMutex);
		g_pBdbgMutex = NULL;
	}
}

/****************************************************************************
****************************************************************************/
void 
BDBG_P_Lock_isrsafe(void)
{
    unsigned char ucosError;

    if (g_pBdbgMutex == NULL) {
        ucosError = BERR_TRACE(BERR_NOT_INITIALIZED);
        return;
    }
    
    if (CHECK_CRITICAL()) {
        return;
    }
    
    /* Wait forever on this mutex */
    bos_acquire_mutex(g_pBdbgMutex,BOS_PEND_FOREVER); 
}

/****************************************************************************
****************************************************************************/
void 
BDBG_P_Unlock_isrsafe(void
{
    BERR_Code rc;

    if (g_pBdbgMutex == NULL) {
        rc = BERR_TRACE(BERR_NOT_INITIALIZED);
        return;
    }

    if (CHECK_CRITICAL()) {
        return;
    }

    bos_release_mutex(g_pBdbgMutex);
}

/* End of file */
