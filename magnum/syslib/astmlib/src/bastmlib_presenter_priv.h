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

#ifndef BASTMLIB_PRESENTER_PRIV_H__
#define BASTMLIB_PRESENTER_PRIV_H__

/* TODO: tuning for these */
#define BASTMLIB_PRESENTER_P_DEFAULT_MIN_TIME_BETWEEN_EVENTS 10
#define BASTMLIB_PRESENTER_P_DEFAULT_EVENT_QUEUE_CAPACITY 500
#define BASTMLIB_PRESENTER_P_DEFAULT_PASS_EVENT_COUNT_THRESHOLD 5
#define BASTMLIB_PRESENTER_P_DEFAULT_FAIL_EVENT_COUNT_THRESHOLD 10

struct BASTMlib_Presenter_Impl
{
	BASTMlib_Handle hAstm;
	char * pcName;
	unsigned int uiId;

	BASTMlib_Presenter_Settings sSettings;
	BASTMlib_Presenter_Config sConfig;

	unsigned int uiMaximumAcquisitionTime;
	unsigned int uiPassEventCount;
	unsigned int uiFailEventCount;

	struct
	{
		BASTMlib_Presenter_Event * asEvents;
		unsigned int uiSize;
		unsigned int uiCapacity;
		unsigned int uiWrite;
		unsigned int uiRead;
	} sEventQueue;
};

#endif /* BASTMLIB_PRESENTER_PRIV_H__ */

