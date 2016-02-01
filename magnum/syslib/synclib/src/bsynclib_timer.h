/***************************************************************************
*     Copyright (c) 2004-2008, Broadcom Corporation
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
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/

#include "bstd.h"
#include "bsyslib.h"
#include "bsynclib.h"

#ifndef BSYNCLIB_TIMER_H__
#define BSYNCLIB_TIMER_H__

typedef struct
{
	BSYNClib_Channel_Handle hOwner;
	bool bScheduled; /* is the timer scheduled? */
	BSYSlib_Timer_Handle hTimer;
} BSYNClib_Timer;

void BSYNClib_Timer_Init(BSYNClib_Timer * psTimer);
void BSYNClib_Timer_Reset_isr(BSYNClib_Timer * psTimer);

#endif /* BSYNCLIB_TIMER_H__ */

