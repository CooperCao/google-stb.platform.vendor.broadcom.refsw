/***************************************************************************
*     Copyright (c) 2004-2013, Broadcom Corporation
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

#include "bsyslib.h"
#include "bsyslib_callback.h"
#include "bsynclib_video_format.h"

#ifndef BSYNCLIB_RATE_MISMATCH_DETECTOR_H__
#define BSYNCLIB_RATE_MISMATCH_DETECTOR_H__

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

