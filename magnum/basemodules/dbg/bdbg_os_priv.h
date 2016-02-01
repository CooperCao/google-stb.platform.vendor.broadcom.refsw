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


#ifndef BDBG_OS_PRIV_H__
#define BDBG_OS_PRIV_H__

#ifdef __cplusplus
extern "C" {
#endif

/* The functions below are OS specific and must exist for OS using BDBG module  */

/****************************************************************
Summary:
Initialize the OS-specific layer in the DBG basemodule
****************************************************************/
BERR_Code BDBG_P_OsInit(void);

/****************************************************************
Summary:
Uninitialize the OS-specific layer in the DBG basemodule
****************************************************************/
void BDBG_P_OsUninit(void);

/****************************************************************
Summary:
Lock BDBG module from multiple accesses.

Description:
DBG requires that BDBG_P_Lock be implemented per OS.
BDBG_P_Lock must work in both task and isr modes.
Therefore, BDBG_P_Lock cannot be implemented by calling BKNI_AcquireMutex or BKNI_EnterCriticalSection which only work in task mode.

Your OS's implementation of BDBG_P_Lock will likely look similar to your OS's implementation of BKNI_EnterCriticalSection.
For example, in linux user mode, BDBG_P_Lock acquries a pthread mutex.
For another example, in linux kernel mode, BDBG_P_Lock acquires a spin lock.
****************************************************************/
void BDBG_P_Lock(void);


/****************************************************************
Summary:
Release BDBG module from a BDBG_P_Lock state.
****************************************************************/
void BDBG_P_Unlock(void);


/****************************************************************
Summary:
This function initializes the initial timestamp when BDBG module
is initialize. This initial timestamp will be used as reference
for determining the run time thus far.
****************************************************************/
void BDBG_P_InitializeTimeStamp(void);


/****************************************************************
Summary:
This function calculates the run time and convert it into a
more readable form so that it can be attach to debug messages.
*****************************************************************/
void BDBG_P_GetTimeStamp(
    char *timeStamp,    /* size of the string containing the formatted timestamp */
    int size          /* the formatted timestamp to attach to debug messages */
    );


#ifdef __cplusplus
}
#endif

#endif
