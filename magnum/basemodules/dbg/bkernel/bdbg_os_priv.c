/***************************************************************************
 *     Copyright (c) 2006-2009, Broadcom Corporation
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
#include "bkernel.h"

void
BDBG_P_InitializeTimeStamp(void)
{
}

void
BDBG_P_GetTimeStamp(char *timeStamp, int size)
{
    struct timeval currentTime;

    gettimeofday(&currentTime, NULL);

    BKNI_Snprintf(timeStamp, size, "%3u.%03u", currentTime.tv_sec, currentTime.tv_usec/1000);
    return;

}

BERR_Code BDBG_P_OsInit(void)
{
    return 0;
}

void BDBG_P_OsUninit(void)
{
}

/* NOTE: this function is called from both magnum task and isr context */
void BDBG_P_Lock(void)
{
}

/* NOTE: this function is called from both magnum task and isr context */
void BDBG_P_Unlock(void)
{
}

/* End of file */

