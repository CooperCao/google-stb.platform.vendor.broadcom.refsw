/***************************************************************************
 *     Copyright (c) 2006-2011, Broadcom Corporation
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
#include "b_bare_os.h"

b_bare_os_tick g_init_timestamp;
b_bare_os_lock g_mutex;

void
BDBG_P_InitializeTimeStamp(void)
{
    g_init_timestamp = pb_bare_os->current_tick();
}

void
BDBG_P_GetTimeStamp(char *timeStamp, int size_t)
{
    b_bare_os_tick currentTime;
    int hours, minutes, seconds;
    int milliseconds;
    int rc;

    currentTime = pb_bare_os->current_tick();
    if (currentTime > g_init_timestamp) {
        milliseconds = pb_bare_os->tick_diff(currentTime, g_init_timestamp);
    }
    else {
        milliseconds = pb_bare_os->tick_diff(g_init_timestamp, currentTime);
    }

    /* Calculate the time   */
    hours = milliseconds/(1000*60*60);
    minutes = milliseconds/(1000*60)%60;
    seconds = milliseconds/1000%60;

    /* print the formatted time including the milliseconds  */
    rc = BKNI_Snprintf(timeStamp, size_t, "%02u:%02u:%02u.%03u", hours, minutes, seconds, milliseconds);
    return;

}

BERR_Code BDBG_P_OsInit(void)
{
    g_mutex = pb_bare_os->lock_create();
    return 0;
}

void BDBG_P_OsUninit(void)
{
    pb_bare_os->lock_destroy(g_mutex);
}

/* NOTE: this function is called from both magnum task and isr context */
void BDBG_P_Lock(void)
{
    int rc;
    rc = pb_bare_os->lock_acquire(g_mutex);
    BDBG_ASSERT(0 == rc);
    return;
}

/* NOTE: this function is called from both magnum task and isr context */
void BDBG_P_Unlock(void)
{
    pb_bare_os->lock_release(g_mutex);
    return;
}

/* End of file */
