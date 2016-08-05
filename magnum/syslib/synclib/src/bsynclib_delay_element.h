/***************************************************************************
 *  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.
 *
 * Module Description:
 *
 **************************************************************************/
#ifndef BSYNCLIB_DELAY_ELEMENT_H__
#define BSYNCLIB_DELAY_ELEMENT_H__

#include "bstd.h"
#include "bsynclib.h"

/*
Summary:
*/
typedef struct
{
	bool bValid;
	bool bTimedOut;
	BSYNClib_Units ePreferredUnits;
	BSYNClib_Units eOriginalUnits;
	unsigned int uiMeasured;
	unsigned int uiCustom;
	unsigned int uiCapacity;
	unsigned int uiQuantizationLevel;
} BSYNClib_Delay_Data;

/*
Summary:
*/
typedef struct
{
	bool bAccepted;
	bool bEstimated;
	bool bGenerateCallback;
	unsigned int uiApplied;
	unsigned int uiDesired;
} BSYNClib_Delay_Results;

/*
Summary:
*/
typedef struct
{
	BSYNClib_Delay_Data sData;
	BSYNClib_Delay_Data sSnapshot;
	BSYNClib_Delay_Results sResults;
} BSYNClib_Delay;

/*
Summary:
*/
typedef struct
{
	bool bEnabled; /* TODO: is this being moved to NULLing out the callback function pointer? */
	bool bReceived;
	BSYNClib_Units ePreferredUnits;
} BSYNClib_DelayNotification_Data;

/*
Summary:
*/
typedef struct
{
	bool bEnabled;
	BSYNClib_UnsignedValue sThreshold;
	bool bGenerateCallback;
} BSYNClib_DelayNotification_Results;

/*
Summary:
*/
typedef struct
{
	BSYNClib_DelayNotification_Data sData;
	BSYNClib_DelayNotification_Data sSnapshot;
	BSYNClib_DelayNotification_Results sResults;
} BSYNClib_DelayNotification;

/*
Summary:
*/
typedef enum
{
	BSYNClib_DelayElement_LifecycleEvent_eNone,
	BSYNClib_DelayElement_LifecycleEvent_eStarted,
	BSYNClib_DelayElement_LifecycleEvent_eStopped
} BSYNClib_DelayElement_LifecycleEvent;

/*
Summary:
*/
typedef enum
{
	BSYNClib_DelayElement_SynchronizationEvent_eNone,
	BSYNClib_DelayElement_SynchronizationEvent_eSynchronized,
	BSYNClib_DelayElement_SynchronizationEvent_eIgnored
} BSYNClib_DelayElement_SynchronizationEvent;

#ifdef BDBG_DEBUG_BUILD
extern const char * const BSYNClib_DelayElement_LifecycleEventNames[];
extern const char * const BSYNClib_DelayElement_SynchronizationEventNames[];
#endif

/*
Summary:
*/
typedef struct
{
	bool bStarted;
	bool bSynchronize;
	bool bNonRealTime;
} BSYNClib_DelayElement_Data;

/*
Summary:
*/
typedef struct
{
	bool bChanged;
	BSYNClib_DelayElement_LifecycleEvent eLifecycleEvent;
	BSYNClib_DelayElement_SynchronizationEvent eSynchronizationEvent;
	bool bDelayReceived;
	bool bGenerateDelayCallback;
	bool bGenerateNotificationCallback;
} BSYNClib_DelayElement_DiffResults;

/*
Summary:
*/
typedef struct
{
	unsigned int uiIndex;
	BSYNClib_Channel_Handle hParent;

	BSYNClib_Delay sDelay;
	BSYNClib_DelayNotification sNotification;

	BSYNClib_DelayElement_Data sData;
	BSYNClib_DelayElement_Data sSnapshot;
} BSYNClib_DelayElement;

void BSYNClib_DelayElement_Init(BSYNClib_DelayElement * psElement);
void BSYNClib_DelayElement_Reset_isr(BSYNClib_DelayElement * psElement);
void BSYNClib_DelayElement_Snapshot_isr(BSYNClib_DelayElement * psElement);
void BSYNClib_Delay_Init(BSYNClib_Delay * psDelay);
void BSYNClib_Delay_Reset_isr(BSYNClib_Delay * psDelay);
void BSYNClib_Delay_Snapshot_isr(BSYNClib_Delay * psDelay);
void BSYNClib_DelayNotification_Init(BSYNClib_DelayNotification * psNotification);
void BSYNClib_DelayNotification_Reset_isr(BSYNClib_DelayNotification * psNotification);
void BSYNClib_DelayNotification_Snapshot_isr(BSYNClib_DelayNotification * psNotification);

void BSYNClib_DelayElement_CheckLifecycle_isr(
	bool bStarted,
	const BSYNClib_DelayElement * psCurrent,
	BSYNClib_DelayElement_DiffResults * psResults
);

void BSYNClib_DelayElement_Diff_isr(
	const BSYNClib_DelayElement * psDesired,
	const BSYNClib_DelayElement * psCurrent,
	BSYNClib_DelayElement_DiffResults * psResults
);

void BSYNClib_DelayElement_Patch_isr(
	const BSYNClib_DelayElement * psDesired,
	BSYNClib_DelayElement * psCurrent,
	BSYNClib_DelayElement_DiffResults * psResults
);

#endif /* BSYNCLIB_DELAY_ELEMENT_H__ */
