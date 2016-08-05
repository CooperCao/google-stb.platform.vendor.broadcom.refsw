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
#ifndef BSYNCLIB_RATE_MISMATCH_DETECTOR_PRIV_H__
#define BSYNCLIB_RATE_MISMATCH_DETECTOR_PRIV_H__

#include "bsyslib.h"
#include "bsyslib_list.h"
#include "bsynclib_priv.h"
#include "bsynclib_rate_mismatch_detector.h"
#include "bsynclib_timer.h"

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
