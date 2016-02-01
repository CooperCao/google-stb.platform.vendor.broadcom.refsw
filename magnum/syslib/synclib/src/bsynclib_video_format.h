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
#include "bavc.h"

#ifndef BSYNCLIB_VIDEO_FORMAT_H__
#define BSYNCLIB_VIDEO_FORMAT_H__

typedef struct
{
	bool bValid;
	unsigned int uiHeight; /* height of format, required to predict VDC MAD state changes */
	bool bInterlaced; /* whether the format is interlaced */
	BAVC_FrameRateCode eFrameRate; /* the frame rate of the format */
} BSYNClib_VideoFormat_Data;

typedef struct
{
	bool bChanged;
} BSYNClib_VideoFormat_DiffResults;

typedef struct
{
	BSYNClib_VideoFormat_Data sData;
	BSYNClib_VideoFormat_Data sSnapshot;
} BSYNClib_VideoFormat;

void BSYNClib_VideoFormat_Init(BSYNClib_VideoFormat * psFormat);

void BSYNClib_VideoFormat_Reset_isr(BSYNClib_VideoFormat * psFormat);

void BSYNClib_VideoFormat_Diff_isr(
	BSYNClib_VideoFormat * psDesired,
	BSYNClib_VideoFormat * psCurrent,
	BSYNClib_VideoFormat_DiffResults * psResults
);

void BSYNClib_VideoFormat_Patch_isr(
	BSYNClib_VideoFormat * psDesired,
	BSYNClib_VideoFormat * psCurrent,
	BSYNClib_VideoFormat_DiffResults * psResults
);

void BSYNClib_VideoFormat_Snapshot_isr(BSYNClib_VideoFormat * psFormat);

unsigned int BSYNClib_VideoFormat_P_GetDownconvertedFramePeriod(BSYNClib_VideoFormat * psFormat);
unsigned int BSYNClib_VideoFormat_P_GetDownconvertedFramePeriod_isr(BSYNClib_VideoFormat * psFormat);

unsigned int BSYNClib_VideoFormat_P_GetUpconvertedFramePeriod(BSYNClib_VideoFormat * psFormat);
unsigned int BSYNClib_VideoFormat_P_GetUpconvertedFramePeriod_isr(BSYNClib_VideoFormat * psFormat);

unsigned int BSYNClib_VideoFormat_P_GetFramePeriod(BSYNClib_VideoFormat * psFormat);
unsigned int BSYNClib_VideoFormat_P_GetFramePeriod_isr(BSYNClib_VideoFormat * psFormat);

unsigned int BSYNClib_VideoFormat_P_GetVsyncPeriod(BSYNClib_VideoFormat * psFormat);
unsigned int BSYNClib_VideoFormat_P_GetVsyncPeriod_isr(BSYNClib_VideoFormat * psFormat);

bool BSYNClib_VideoFormat_P_IsFrameRateSupported(BAVC_FrameRateCode eRate);
bool BSYNClib_VideoFormat_P_IsFrameRateSupported_isr(BAVC_FrameRateCode eRate);

#ifdef BDBG_DEBUG_BUILD
const char * BSYNClib_VideoFormat_P_GetFrameRateName(BAVC_FrameRateCode eRate);
const char * BSYNClib_VideoFormat_P_GetFrameRateName_isr(BAVC_FrameRateCode eRate);
#endif

#endif /* BSYNCLIB_VIDEO_FORMAT_H__ */

