/***************************************************************************
 *     Copyright (c) 2006-2013, Broadcom Corporation
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
#include <sys/time.h>
#include <pthread.h>

static struct timeval initTimeStamp;
static pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;

void
BDBG_P_InitializeTimeStamp(void)
{
    gettimeofday(&initTimeStamp, NULL);
    return;
}

void
BDBG_P_GetTimeStamp(char *timeStamp, int size)
{
    struct timeval currentTime;
    int hours, minutes, seconds;
    int milliseconds;

    gettimeofday(&currentTime, NULL);

    if (currentTime.tv_usec < initTimeStamp.tv_usec)
    {
        milliseconds = (currentTime.tv_usec - initTimeStamp.tv_usec + 1000000)/1000;
        currentTime.tv_sec--;
    }
    else    {
        milliseconds = (currentTime.tv_usec - initTimeStamp.tv_usec)/1000;
    }

    /* Calculate the time   */
    hours = (currentTime.tv_sec - initTimeStamp.tv_sec)/3600;
    minutes = (((currentTime.tv_sec - initTimeStamp.tv_sec)/60))%60;
    seconds = (currentTime.tv_sec - initTimeStamp.tv_sec)%60;

    /* print the formatted time including the milliseconds  */
    BKNI_Snprintf(timeStamp, size, "%02u:%02u:%02u.%03u", hours, minutes, seconds, milliseconds);
    return;

}

BERR_Code BDBG_P_OsInit(void)
{
    /* g_mutex is statically initialized */
    return BERR_SUCCESS;
}

void BDBG_P_OsUninit(void)
{
    return;
}

/* NOTE: this function is called from both magnum task and isr context */
void BDBG_P_Lock(void)
{
    pthread_mutex_lock(&g_mutex); /* don't add BDBG_ASSERT, this is a low level BDBG functions that shouldn't use BDBG facilities */
    return;
}

/* NOTE: this function is called from both magnum task and isr context */
void BDBG_P_Unlock(void)
{
    pthread_mutex_unlock(&g_mutex);
    return;
}

/* End of file */
