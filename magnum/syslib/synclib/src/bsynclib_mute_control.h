/***************************************************************************
*     Copyright (c) 2004-2010, Broadcom Corporation
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

#include "bsynclib.h"

#ifndef BSYNCLIB_MUTE_CONTROL_H__
#define BSYNCLIB_MUTE_CONTROL_H__

BERR_Code BSYNClib_MuteControl_ScheduleTask_isr(BSYNClib_Channel_Handle hChn);
BERR_Code BSYNClib_MuteControl_StartUnmuteTimers(BSYNClib_Channel_Handle hChn);
BERR_Code BSYNClib_MuteControl_CancelUnmuteTimers_isr(BSYNClib_Channel_Handle hChn);

#endif /*BSYNCLIB_MUTE_CONTROL_H__ */

