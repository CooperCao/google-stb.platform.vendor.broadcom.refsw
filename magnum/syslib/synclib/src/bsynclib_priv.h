/***************************************************************************
*     Copyright (c) 2004-2012, Broadcom Corporation
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

#include "bsyslib_list.h"
#include "bsynclib.h"

#ifndef BSYNCLIB_PRIV_H__
#define BSYNCLIB_PRIV_H__

/*
 * 20151208 bandrews - some customers prefer video to have an initial delay and
 * then go down, rather than having a pause when we apply it, because they
 * are not using mute control
 */
/* default video priming delay */
#define BSYNCLIB_VIDEO_INITIAL_DELAY 150       /* ms */
#define BSYNCLIB_VIDEO_TSM_LOCK_TIMER_DEFAULT_TIMEOUT 100 /* ms */

/* set to zero means shut it off */
#define BSYNCLIB_VIDEO_RATE_MISMATCH_DETECTION_TIMER_DEFAULT_TIMEOUT 0 /* ms */
#define BSYNCLIB_VIDEO_RATE_REMATCH_DETECTION_TIMER_DEFAULT_TIMEOUT 0 /* ms */
#define BSYNCLIB_VIDEO_RATE_MISMATCH_DETECTION_DEFAULT_ACCEPTABLE_MTBC_LOWER 300
#define BSYNCLIB_VIDEO_RATE_MISMATCH_DETECTION_DEFAULT_ACCEPTABLE_MTBC_UPPER 1500
#define BSYNCLIB_VIDEO_RATE_MISMATCH_DETECTION_DEFAULT_ACCEPTABLE_TTLC 1500
#define BSYNCLIB_VIDEO_RATE_REMATCH_DETECTION_DEFAULT_ACCEPTABLE_TTLC 9000

#define BSYNCLIB_AUDIO_UNMUTE_DEFAULT_TIMEOUT 200 /* ms */
#define BSYNCLIB_VIDEO_UNMUTE_DEFAULT_TIMEOUT 133 /* ms */

/* 20070406 bandrews - compensate for external audio receiver on SPDIF output */
#define BSYNCLIB_AUDIO_RECEIVER_DELAY_COMPENSATION_DEFAULT 0 /* ms */

#define BSYNCLIB_UNCONDITIONAL_VIDEO_UNMUTE_SUPPORT 1
#define BSYNCLIB_VIDEO_UNCONDITIONAL_UNMUTE_DEFAULT_TIMEOUT 5000
#define BSYNCLIB_UNCONDITIONAL_AUDIO_UNMUTE_SUPPORT 1
#define BSYNCLIB_AUDIO_UNCONDITIONAL_UNMUTE_DEFAULT_TIMEOUT 5000

/*
Summary:
*/
struct BSYNClib_Impl
{
	BSYNClib_Settings sSettings;
	BSYSlib_List_Handle hChannels;
};

bool BSYNClib_P_Enabled(BSYNClib_Handle hSync);

unsigned int BSYNClib_P_Convert(unsigned int uiValue, BSYNClib_Units eFromUnits, BSYNClib_Units eToUnits);
unsigned int BSYNClib_P_Convert_isr(unsigned int uiValue, BSYNClib_Units eFromUnits, BSYNClib_Units eToUnits);
int BSYNClib_P_ConvertSigned(int iValue, BSYNClib_Units eFromUnits, BSYNClib_Units eToUnits);
int BSYNClib_P_ConvertSigned_isr(int iValue, BSYNClib_Units eFromUnits, BSYNClib_Units eToUnits);

#if BDBG_DEBUG_BUILD
extern const char * const BSYNClib_P_UnitsStrings[];
#endif

#endif /* BSYNCLIB_PRIV_H__ */
