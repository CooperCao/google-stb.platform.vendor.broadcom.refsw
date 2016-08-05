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
#ifndef BSYNCLIB_RATE_MISMATCH_DETECTOR_H__
#define BSYNCLIB_RATE_MISMATCH_DETECTOR_H__

#include "bsyslib.h"
#include "bsyslib_callback.h"
#include "bsynclib_video_format.h"

/*
Summary:
*/
typedef struct BSYNClib_RateMismatchDetector_Impl * BSYNClib_RateMismatchDetector_Handle;

/*
Summary:
*/
typedef struct
{
	bool bEnabled;

	BSYSlib_Callback cbStateChange;
	
	struct 
	{
		unsigned int uiTimeout; /* time to wait before checking the number of callbacks received vs the acceptable callback count for rate mismatch */
		unsigned int uiAcceptableMtbcLower; /* acceptable mean time between callbacks lower bound */
		unsigned int uiAcceptableMtbcUpper; /* acceptable mean time between callbacks upper bound */
		unsigned int uiAcceptableTtlc; /* acceptable time to last callback for mismatches */
	} sMismatch;
	struct
	{
		unsigned int uiTimeout; /* time to wait before checking the number of callbacks received vs the acceptable callback count for rate rematch */
		unsigned int uiAcceptableTtlc; /* acceptable time to last callback for rematches */
	} sRematch;
} BSYNClib_RateMismatchDetector_Settings;

/*
Summary:
*/
typedef struct
{
	unsigned long ulDelayNotificationTime; /* when was the notification received in ms */
} BSYNClib_RateMismatchDetector_DelayNotificationInfo;

/*
Summary:
*/
typedef struct
{
	bool bSourceSinkMatched; /* source 0 matches sink 0 strictly */
	bool bSinkSinkMatched; /* all sinks match each other loosely */
} BSYNClib_RateMismatchDetector_Status;

typedef struct
{
	unsigned int uiVideoSourceCount;
	unsigned int uiVideoSinkCount;
} BSYNClib_RateMismatchDetector_Config;

/*
Summary:
Gets the default settings
*/
void BSYNClib_RateMismatchDetector_GetDefaultSettings(
	BSYNClib_RateMismatchDetector_Settings * psSettings /* [out] */
);

/*
Summary:
Opens a state machine handle
*/
BERR_Code BSYNClib_RateMismatchDetector_Open(
	BSYNClib_Channel_Handle hParent,
	const BSYNClib_RateMismatchDetector_Settings * psSettings,
	BSYNClib_RateMismatchDetector_Handle * phDetector
);

/*
Summary:
Closes a state machine handle
*/
void BSYNClib_RateMismatchDetector_Close(
	BSYNClib_RateMismatchDetector_Handle hDetector
);

/*
Summary:
*/
BERR_Code BSYNClib_RateMismatchDetector_DelayNotificationHandler_isr(
	BSYNClib_RateMismatchDetector_Handle hDetector,
	unsigned int uiSource,
	const BSYNClib_RateMismatchDetector_DelayNotificationInfo * psInfo
);

/*
Summary:
*/
BERR_Code BSYNClib_RateMismatchDetector_ResetSourceMeasurements_isr(
	BSYNClib_RateMismatchDetector_Handle hDetector,
	unsigned int uiSource
);

/*
Summary:
*/
BERR_Code BSYNClib_RateMismatchDetector_ResetSourceData_isr(
	BSYNClib_RateMismatchDetector_Handle hDetector,
	unsigned int uiSource
);

/*
Summary:
*/
BERR_Code BSYNClib_RateMismatchDetector_SetVideoSourceFormat_isr(
	BSYNClib_RateMismatchDetector_Handle hSync,
	unsigned int uiSource,
	const BSYNClib_VideoFormat * psFormat
);

/*
Summary:
*/
BERR_Code BSYNClib_RateMismatchDetector_SetVideoSinkFormat_isr(
	BSYNClib_RateMismatchDetector_Handle hSync,
	unsigned int uiSink,
	const BSYNClib_VideoFormat * psFormat,
	bool bMasterFrameRateEnabled,
	bool bSyncLocked
);

/*
Summary:
*/
BERR_Code BSYNClib_RateMismatchDetector_GetStatus(
	BSYNClib_RateMismatchDetector_Handle hDetector,
	BSYNClib_RateMismatchDetector_Status * psStatus
);

void BSYNClib_RateMismatchDetector_GetConfig(
	BSYNClib_RateMismatchDetector_Handle hDetector,
	BSYNClib_RateMismatchDetector_Config * psConfig
);

BERR_Code BSYNClib_RateMismatchDetector_SetConfig(
	BSYNClib_RateMismatchDetector_Handle hDetector,
	const BSYNClib_RateMismatchDetector_Config * psConfig
);

#endif /* BSYNCLIB_RATE_MISMATCH_DETECTOR_H__ */

