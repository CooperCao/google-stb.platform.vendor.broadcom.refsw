/***************************************************************************
 *     Copyright (c) 2006-2012, Broadcom Corporation
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
#include <linux/version.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 37)
#include <generated/autoconf.h>
#else
#include <linux/autoconf.h>
#endif
#include <linux/time.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
#include <linux/hardirq.h>
#else
#include <linux/tqueue.h>
#include <asm/smplock.h>
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,17,0)
#include <linux/ktime.h>
#endif

static struct timeval initTimeStamp;
static spinlock_t g_lock = __SPIN_LOCK_UNLOCKED(bdbg_os_priv.lock);
static unsigned long g_lockFlags;

void
BDBG_P_InitializeTimeStamp(void)
{
    do_gettimeofday(&initTimeStamp);
}

void
BDBG_P_GetTimeStamp(char *timeStamp, int size_t)
{
    struct timeval currentTime;
    int hours, minutes, seconds;
    int milliseconds;
    int rc;

    do_gettimeofday(&currentTime);

    if (currentTime.tv_usec < initTimeStamp.tv_usec)
    {
        milliseconds = (currentTime.tv_usec - initTimeStamp.tv_usec + 1000000)/1000;
        currentTime.tv_sec--;
    }
    else{
        milliseconds = (currentTime.tv_usec - initTimeStamp.tv_usec)/1000;
    }

    /* Calculate the time   */
    hours = (currentTime.tv_sec - initTimeStamp.tv_sec)/3600;
    minutes = (((currentTime.tv_sec - initTimeStamp.tv_sec)/60))%60;
    seconds = (currentTime.tv_sec - initTimeStamp.tv_sec)%60;

    /* print the formatted time including the milliseconds  */
    rc = BKNI_Snprintf(timeStamp, size_t, "%02u:%02u:%02u.%03u", hours, minutes, seconds, milliseconds);
    return;

}

BERR_Code BDBG_P_OsInit(void)
{
    /* g_lock is statically initialized */
    return 0;
}

void BDBG_P_OsUninit(void)
{
}

/* Must protect against recursive entries with isr's and/or tasklets here */
/* This code should really all be in BKNI... and not here... */

/*******
 * WARNING: There may be a deadlock possibility using recursive spinlocks
 * if critical sections are implemented with them as well.  Because of this,
 * it is recommended that critical sections are implemented with tasklet_disable
 * and tasklet_enable rather than spin_lock_bh().
*******/
void BDBG_P_Lock(void)
{
    spin_lock_irqsave(&g_lock, g_lockFlags);
    return;
}

void BDBG_P_Unlock(void)
{
    spin_unlock_irqrestore(&g_lock, g_lockFlags);
    return;
}

/* End of file */
