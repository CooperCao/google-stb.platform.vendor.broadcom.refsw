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
#include "bstd.h"
#include "bkni.h"
#include "bsynclib_video_format.h"

BDBG_MODULE(synclib);

void BSYNClib_VideoFormat_Init(BSYNClib_VideoFormat * psFormat)
{
	BDBG_ENTER(BSYNClib_VideoFormat_Reset_isr);
	BDBG_ASSERT(psFormat);
	BKNI_Memset(psFormat, 0, sizeof(BSYNClib_VideoFormat));
	psFormat->sData.eFrameRate = BAVC_FrameRateCode_eUnknown;
	BDBG_LEAVE(BSYNClib_VideoFormat_Reset_isr);
}

void BSYNClib_VideoFormat_Reset_isr(BSYNClib_VideoFormat * psFormat)
{
	BDBG_ENTER(BSYNClib_VideoFormat_Reset_isr);
	BDBG_ASSERT(psFormat);
	psFormat->sData.bValid = false;
	psFormat->sData.eFrameRate = BAVC_FrameRateCode_eUnknown;
	BDBG_LEAVE(BSYNClib_VideoFormat_Reset_isr);
}

void BSYNClib_VideoFormat_Diff_isr(
	BSYNClib_VideoFormat * psDesired,
	BSYNClib_VideoFormat * psCurrent,
	BSYNClib_VideoFormat_DiffResults * psResults
)
{
	BDBG_ENTER(BSYNClib_VideoFormat_Diff_isr);

	BDBG_ASSERT(psDesired);
	BDBG_ASSERT(psCurrent);
	BDBG_ASSERT(psResults);

	if 
	(
		(psDesired->sData.eFrameRate != psCurrent->sData.eFrameRate)
		|| (psDesired->sData.bInterlaced != psCurrent->sData.bInterlaced)
		|| (psDesired->sData.uiHeight != psCurrent->sData.uiHeight)
	)
	{
		psResults->bChanged = true;
	}

	BDBG_LEAVE(BSYNClib_VideoFormat_Diff_isr);
}

void BSYNClib_VideoFormat_Patch_isr(
	BSYNClib_VideoFormat * psDesired,
	BSYNClib_VideoFormat * psCurrent,
	BSYNClib_VideoFormat_DiffResults * psResults
)
{
	BDBG_ENTER(BSYNClib_VideoFormat_Patch_isr);

	BDBG_ASSERT(psDesired);
	BDBG_ASSERT(psCurrent);
	BDBG_ASSERT(psResults);

	if (psResults->bChanged)
	{
		psCurrent->sData = psDesired->sData;
		psCurrent->sData.bValid = true;
	}

	BDBG_LEAVE(BSYNClib_VideoFormat_Patch_isr);
}

void BSYNClib_VideoFormat_Snapshot_isr(BSYNClib_VideoFormat * psFormat)
{
	BDBG_ENTER(BSYNClib_VideoFormat_Snapshot_isr);
	BDBG_ASSERT(psFormat);
	psFormat->sSnapshot = psFormat->sData;
	BDBG_LEAVE(BSYNClib_VideoFormat_Snapshot_isr);
}

static const unsigned int gauiFramePeriods[BAVC_FrameRateCode_eMax + 1] =
	{
		0, /* Unknown */
		1126125, /* 23.976 */
		1125000, /* 24 */
		1080000, /* 25 */
		900900, /* 29.97 */
		900000, /* 30 */
		540000, /* 50 */
		450450, /* 59.94 */
		450000, /* 60 */
		1801800, /* 14.985 */
		3603600, /* 7.493 */
		2700000, /* 10 */
		1800000, /* 15 */
		1350000, /* 20 */
		2160000, /* 12.5 */
		270000, /* 100 */
		225225, /* 119.88 */
		225000,/* 120 */
		1351350, /* 19.98 */
		0 /* max */
	};

/* table of native frame periods * 1001/1000 */
static const unsigned int gauiUpconvertedFramePeriods[BAVC_FrameRateCode_eMax + 1] =
	{
		0, /* Unknown */
		1127251, /* 23.976 * 1000/1001 */
		1126125, /* 24 -> 23.976 */
		1081080, /* 25 * 1000/1001 */
		901801, /* 29.97 * 1000/1001 */
		900900, /* 30 -> 29.97 */
		540540, /* 50 * 1000/1001 */
		450900, /* 59.94 */
		450450, /* 60 -> 59.94 */
		1803602, /* 14.985 * 1000/1001 */
		3607204, /* 7.493 * 1000/1001 */
		2702700, /* 10 * 1000/1001 */
		1801800, /* 15 * 1000/1001 */
		1351350, /* 20 * 1000/1001 */
		2162160, /* 12.5 * 1000/1001 */
		270270, /* 100 * 1000/1001 */
		225450, /* 119.88 * 1000/1001 */
		225225, /* 120 * 1000/1001 */
		1352701, /* 19.98 * 1000/1001 */
		0 /* max */
	};

/* table of native frame periods * 1000/1001 */
static const unsigned int gauiDownconvertedFramePeriods[BAVC_FrameRateCode_eMax + 1] =
	{
		0, /* Unknown */
		1125000, /* 23.976 -> 24 */
		1123876, /* 24 * 1001/1000 */
		1078921, /* 25 * 1001/1000 */
		900000, /* 29.97 -> 30 */
		899101, /* 30 * 1001/1000 */
		539461, /* 50 * 1001/1000 */
		450000, /* 59.94 -> 60 */
		449550, /* 60 * 1001/1000 */
		1800000, /* 14.985 -> 15 */
		3600000, /* 7.493 -> 7.5 */
		2697303, /* 10 * 1001/1000 */
		1798202, /* 15 * 1001/1000 */
		1348651, /* 20 * 1001/1000 */
		2157842, /* 12.5 * 1001/1000 */
		269730, /* 100 * 1001/1000 */
		225000, /* 119.88 * 1001/1000 */
		224775, /* 120 * 1001/1000 */
		1350000, /* 19.98 * 1001/1000 */
		0 /* max */
	};

unsigned int BSYNClib_VideoFormat_P_GetUpconvertedFramePeriod_isrsafe(BSYNClib_VideoFormat * psFormat)
{
	unsigned int uiFramePeriod;

	BDBG_ENTER(BSYNClib_VideoFormat_P_GetUpconvertedFramePeriod_isrsafe);

	BDBG_ASSERT(psFormat);

	if (BSYNClib_VideoFormat_P_IsFrameRateSupported_isrsafe(psFormat->sData.eFrameRate))
	{
		uiFramePeriod = gauiUpconvertedFramePeriods[psFormat->sData.eFrameRate];
	}
	else
	{
		BDBG_WRN(("Unsupported sync frame rate; lipsync will suffer"));
		uiFramePeriod = gauiUpconvertedFramePeriods[0];
	}

	BDBG_LEAVE(BSYNClib_VideoFormat_P_GetUpconvertedFramePeriod_isrsafe);
	return uiFramePeriod;
}

unsigned int BSYNClib_VideoFormat_P_GetDownconvertedFramePeriod_isrsafe(BSYNClib_VideoFormat * psFormat)
{
	unsigned int uiFramePeriod;

	BDBG_ENTER(BSYNClib_VideoFormat_P_GetDownconvertedFramePeriod_isrsafe);

	BDBG_ASSERT(psFormat);

	if (BSYNClib_VideoFormat_P_IsFrameRateSupported_isrsafe(psFormat->sData.eFrameRate))
	{
		uiFramePeriod = gauiDownconvertedFramePeriods[psFormat->sData.eFrameRate];
	}
	else
	{
		BDBG_WRN(("Unsupported sync frame rate; lipsync will suffer"));
		uiFramePeriod = gauiDownconvertedFramePeriods[0];
	}

	BDBG_LEAVE(BSYNClib_VideoFormat_P_GetDownconvertedFramePeriod_isrsafe);
	return uiFramePeriod;
}


bool BSYNClib_VideoFormat_P_IsFrameRateSupported_isrsafe(BAVC_FrameRateCode eRate)
{
	bool bResult = false;

	BDBG_ENTER(BSYNClib_VideoFormat_P_IsFrameRateSupported_isrsafe);

	switch (eRate)
	{
		case BAVC_FrameRateCode_eUnknown:
		case BAVC_FrameRateCode_e23_976:
		case BAVC_FrameRateCode_e24:
		case BAVC_FrameRateCode_e25:
		case BAVC_FrameRateCode_e29_97:
		case BAVC_FrameRateCode_e30:
		case BAVC_FrameRateCode_e50:
		case BAVC_FrameRateCode_e59_94:
		case BAVC_FrameRateCode_e60:
		case BAVC_FrameRateCode_e14_985:
		case BAVC_FrameRateCode_e7_493:
		case BAVC_FrameRateCode_e10:
		case BAVC_FrameRateCode_e15:
		case BAVC_FrameRateCode_e20:
		case BAVC_FrameRateCode_e12_5:
		case BAVC_FrameRateCode_e100:
		case BAVC_FrameRateCode_e119_88:
		case BAVC_FrameRateCode_e120:
		case BAVC_FrameRateCode_e19_98:
			bResult = true;
			break;
		default:
			bResult = false;
			break;
	}

	BDBG_LEAVE(BSYNClib_VideoFormat_P_IsFrameRateSupported_isrsafe);

	return bResult;
}

unsigned int BSYNClib_VideoFormat_P_GetFramePeriod_isrsafe(BSYNClib_VideoFormat * psFormat)
{
	unsigned int uiFramePeriod;

	BDBG_ENTER(BSYNClib_VideoFormat_P_GetFramePeriod_isrsafe);

	BDBG_ASSERT(psFormat);

	if (BSYNClib_VideoFormat_P_IsFrameRateSupported_isrsafe(psFormat->sData.eFrameRate))
	{
		uiFramePeriod = gauiFramePeriods[psFormat->sData.eFrameRate];
	}
	else
	{
		BDBG_WRN(("Unsupported sync frame rate; lipsync may suffer"));
		uiFramePeriod = gauiFramePeriods[0];
	}

	BDBG_LEAVE(BSYNClib_VideoFormat_P_GetFramePeriod_isrsafe);
	return uiFramePeriod;
}

unsigned int BSYNClib_VideoFormat_P_GetVsyncPeriod_isrsafe(BSYNClib_VideoFormat * psFormat)
{
	unsigned int uiFieldPeriod;

	BDBG_ENTER(BSYNClib_VideoFormat_P_GetVsyncPeriod_isrsafe);

	BDBG_ASSERT(psFormat);

	uiFieldPeriod = BSYNClib_VideoFormat_P_GetFramePeriod_isrsafe(psFormat);

	if (psFormat->sData.bInterlaced)
	{
		uiFieldPeriod /= 2;
	}

	BDBG_LEAVE(BSYNClib_VideoFormat_P_GetVsyncPeriod_isrsafe);
	return uiFieldPeriod;	
}

#ifdef BDBG_DEBUG_BUILD
static const char * const gapcFrameRateNames[BAVC_FrameRateCode_eMax + 1] =
	{
		"Unknown",
		"23.976",
		"24",
		"25",
		"29.97",
		"30",
		"50",
		"59.94",
		"60",
		"14.985",
		"7.493",
		"10",
		"15",
		"20",
		NULL
	};

static const char * const gapcUnsupportedFrameRateName = "Unsupported";

const char * BSYNClib_VideoFormat_P_GetFrameRateName_isrsafe(BAVC_FrameRateCode eRate)
{
	const char * pcName = NULL;
	BDBG_ENTER(BSYNClib_VideoFormat_P_GetFrameRateName_isrsafe);

	if (BSYNClib_VideoFormat_P_IsFrameRateSupported_isrsafe(eRate))
	{
		pcName = gapcFrameRateNames[eRate];
	}
	else
	{
		BDBG_WRN(("Unsupported sync frame rate; lipsync will suffer"));
		pcName = gapcUnsupportedFrameRateName;
	}

	BDBG_LEAVE(BSYNClib_VideoFormat_P_GetFrameRateName_isrsafe);
	return pcName;
}
#endif /* BDBG_DEBUG_BUILD */
