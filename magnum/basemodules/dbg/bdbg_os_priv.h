/***************************************************************************
 * Copyright (C) 2006-2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
void BDBG_P_Lock_isrsafe(void);


/****************************************************************
Summary:
Release BDBG module from a BDBG_P_Lock state.
****************************************************************/
void BDBG_P_Unlock_isrsafe(void);


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
void BDBG_P_GetTimeStamp_isrsafe(
    char *timeStamp,    /* size of the string containing the formatted timestamp */
    int size          /* the formatted timestamp to attach to debug messages */
    );


#ifdef __cplusplus
}
#endif

#endif
