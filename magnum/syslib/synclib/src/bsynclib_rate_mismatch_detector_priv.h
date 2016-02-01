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
#include "bsyslib_list.h"
#include "bsynclib_priv.h"
#include "bsynclib_rate_mismatch_detector.h"
#include "bsynclib_timer.h"

#ifndef BSYNCLIB_RATE_MISMATCH_DETECTOR_PRIV_H__
#define BSYNCLIB_RATE_MISMATCH_DETECTOR_PRIV_H__

typedef struct BSYNClib_RateMismatchDetector_Sink
{
	unsigned int uiIndex;
	BSYNClib_VideoFormat sFormat;
	bool bMasterFrameRateEnabled;
	bool bSyncLocked;
} BSYNClib_RateMismatchDetector_Sink;

typedef struct BSYNClib_RateMismatchDetector_Source
{
	unsigned int uiIndex;
	BSYNClib_Timer * psTimer;

	BSYNClib_VideoFormat sFormat;

	unsigned int uiDelayCallbackCount;
	long lMeanTimeBetweenCallbacks;
	unsigned long ulLastCallbackTime;
} BSYNClib_RateMismatchDetector_Source;

typedef struct
{
	BSYSlib_List_Handle hSources;
	unsigned int uiSourceCount;
	
	BSYSlib_List_Handle hSinks;
	unsigned int uiSinkCount;
} BSYNClib_RateMismatchDetector_Data;

/*
Summary:
*/
struct BSYNClib_RateMismatchDetector_Impl
{
	/* TODO: change this so that I can schedule tasks instead of having a timer for each one */
	BSYNClib_Timer * psStaticRateMatchTimer;
	BSYNClib_Timer * psStaticRateMismatchTimer;
	BSYNClib_Channel_Handle hParent;
	BSYNClib_RateMismatchDetector_Settings sSettings;
	BSYNClib_RateMismatchDetector_Config sConfig;
	BSYNClib_RateMismatchDetector_Data sData;
	BSYNClib_RateMismatchDetector_Status sStatus;
};

BERR_Code BSYNClib_RateMismatchDetector_P_StaticRateMatchTimerExpired(void * pvParm1, int iParm2, BSYSlib_Timer_Handle hTimer);
BERR_Code BSYNClib_RateMismatchDetector_P_StaticRateMismatchTimerExpired(void * pvParm1, int iParm2, BSYSlib_Timer_Handle hTimer);
BERR_Code BSYNClib_RateMismatchDetector_MismatchTimerExpired(void * pvParm1, int iParm2, BSYSlib_Timer_Handle hTimer);
BERR_Code BSYNClib_RateMismatchDetector_RematchTimerExpired(void * pvParm1, int iParm2, BSYSlib_Timer_Handle hTimer);
BERR_Code BSYNClib_RateMismatchDetector_P_ProcessConfig(BSYNClib_RateMismatchDetector_Handle hDetector);

void BSYNClib_RateMismatchDetector_P_TestSourceAndSinkFormats_isr(
	BSYNClib_RateMismatchDetector_Handle hDetector, 
	BSYNClib_RateMismatchDetector_Source * psSource, 
	BSYNClib_RateMismatchDetector_Sink * psSink
);

void BSYNClib_RateMismatchDetector_P_CompareSinkFormats_isr(
	BSYNClib_RateMismatchDetector_Handle hDetector
);

bool BSYNClib_RateMismatchDetector_P_LooseRateMatchCheck_isr(
	BSYNClib_VideoFormat * psFormatA, 
	BSYNClib_VideoFormat * psFormatB
);

bool BSYNClib_RateMismatchDetector_P_StrictRateMatchCheck_isr(
	BSYNClib_VideoFormat * psFormatA, 
	BSYNClib_VideoFormat * psFormatB,
	bool bMasterFrameRateEnabled,
	bool bSyncLocked
);

#endif /* BSYNCLIB_RATE_MISMATCH_DETECTOR_PRIV_H__ */

