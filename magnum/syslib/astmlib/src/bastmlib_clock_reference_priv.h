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

#include "bstd.h"
#include "bastmlib.h"

#ifndef BASTMLIB_CLOCK_REFERENCE_PRIV_H__
#define BASTMLIB_CLOCK_REFERENCE_PRIV_H__

/* TODO: tuning for these */
#define BASTMLIB_CLOCK_REFERENCE_P_DEFAULT_MIN_TIME_BETWEEN_EVENTS 20 /* ms */
#define BASTMLIB_CLOCK_REFERENCE_P_DEFAULT_EVENT_QUEUE_CAPACITY 100
#define BASTMLIB_CLOCK_REFERENCE_P_DEFAULT_DEVIANT_COUNT_THRESHOLD 2
#define BASTMLIB_CLOCK_REFERENCE_P_DEFAULT_IDEAL_COUNT_THRESHOLD 2
#define BASTMLIB_CLOCK_REFERENCE_P_DEFAULT_DEVIATION_THRESHOLD 1 /* 1 ms */

struct BASTMlib_ClockReference_Impl
{
	BASTMlib_Handle hAstm;

	BASTMlib_ClockReference_Config sConfig;

	unsigned int uiMaximumAcquisitionTime;
	unsigned int uiDeviantCount;
	unsigned int uiIdealCount;
	long lAverageDeviation;

	struct
	{
		BASTMlib_ClockReference_Event * asEvents;
		unsigned int uiSize;
		unsigned int uiCapacity;
		unsigned int uiWrite;
		unsigned int uiRead;
	} sEventQueue;
};

#endif /* BASTMLIB_CLOCK_REFERENCE_PRIV_H__ */

