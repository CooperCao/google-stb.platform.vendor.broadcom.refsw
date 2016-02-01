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

unsigned int BSYNClib_VideoFormat_P_GetUpconvertedFramePeriod(BSYNClib_VideoFormat * psFormat)
{
	unsigned int uiFramePeriod;
	
	BDBG_ENTER(BSYNClib_VideoFormat_P_GetUpconvertedFramePeriod);

	BDBG_ASSERT(psFormat);

	if (BSYNClib_VideoFormat_P_IsFrameRateSupported(psFormat->sSnapshot.eFrameRate))
	{
		uiFramePeriod = gauiUpconvertedFramePeriods[psFormat->sSnapshot.eFrameRate];
	}
	else
	{
		BDBG_WRN(("Unsupported sync frame rate; lipsync will suffer"));
		uiFramePeriod = gauiUpconvertedFramePeriods[0];
	}

	BDBG_LEAVE(BSYNClib_VideoFormat_P_GetUpconvertedFramePeriod);
	return uiFramePeriod;
}

unsigned int BSYNClib_VideoFormat_P_GetUpconvertedFramePeriod_isr(BSYNClib_VideoFormat * psFormat)
{
	unsigned int uiFramePeriod;
	
	BDBG_ENTER(BSYNClib_VideoFormat_P_GetUpconvertedFramePeriod_isr);

	BDBG_ASSERT(psFormat);

	if (BSYNClib_VideoFormat_P_IsFrameRateSupported_isr(psFormat->sData.eFrameRate))
	{
		uiFramePeriod = gauiUpconvertedFramePeriods[psFormat->sData.eFrameRate];
	}
	else
	{
		BDBG_WRN(("Unsupported sync frame rate; lipsync will suffer"));
		uiFramePeriod = gauiUpconvertedFramePeriods[0];
	}

	BDBG_LEAVE(BSYNClib_VideoFormat_P_GetUpconvertedFramePeriod_isr);
	return uiFramePeriod;
}

unsigned int BSYNClib_VideoFormat_P_GetDownconvertedFramePeriod(BSYNClib_VideoFormat * psFormat)
{
	unsigned int uiFramePeriod;
	
	BDBG_ENTER(BSYNClib_VideoFormat_P_GetDownconvertedFramePeriod);

	BDBG_ASSERT(psFormat);

	if (BSYNClib_VideoFormat_P_IsFrameRateSupported(psFormat->sSnapshot.eFrameRate))
	{
		uiFramePeriod = gauiDownconvertedFramePeriods[psFormat->sSnapshot.eFrameRate];
	}
	else
	{
		BDBG_WRN(("Unsupported sync frame rate; lipsync will suffer"));
		uiFramePeriod = gauiDownconvertedFramePeriods[0];
	}

	BDBG_LEAVE(BSYNClib_VideoFormat_P_GetDownconvertedFramePeriod);
	return uiFramePeriod;
}

unsigned int BSYNClib_VideoFormat_P_GetDownconvertedFramePeriod_isr(BSYNClib_VideoFormat * psFormat)
{
	unsigned int uiFramePeriod;
	
	BDBG_ENTER(BSYNClib_VideoFormat_P_GetDownconvertedFramePeriod_isr);

	BDBG_ASSERT(psFormat);

	if (BSYNClib_VideoFormat_P_IsFrameRateSupported_isr(psFormat->sData.eFrameRate))
	{
		uiFramePeriod = gauiDownconvertedFramePeriods[psFormat->sData.eFrameRate];
	}
	else
	{
		BDBG_WRN(("Unsupported sync frame rate; lipsync will suffer"));
		uiFramePeriod = gauiDownconvertedFramePeriods[0];
	}

	BDBG_LEAVE(BSYNClib_VideoFormat_P_GetDownconvertedFramePeriod_isr);
	return uiFramePeriod;
}


bool BSYNClib_VideoFormat_P_IsFrameRateSupported(BAVC_FrameRateCode eRate)
{
	bool bResult = false;
	
	BDBG_ENTER(BSYNClib_VideoFormat_P_IsFrameRateSupported);
	
	BKNI_EnterCriticalSection();
	bResult = BSYNClib_VideoFormat_P_IsFrameRateSupported_isr(eRate);
	BKNI_LeaveCriticalSection();

	BDBG_LEAVE(BSYNClib_VideoFormat_P_IsFrameRateSupported);

	return bResult;
}

bool BSYNClib_VideoFormat_P_IsFrameRateSupported_isr(BAVC_FrameRateCode eRate)
{
	bool bResult = false;
	
	BDBG_ENTER(BSYNClib_VideoFormat_P_IsFrameRateSupported_isr);

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

	BDBG_LEAVE(BSYNClib_VideoFormat_P_IsFrameRateSupported_isr);

	return bResult;
}

unsigned int BSYNClib_VideoFormat_P_GetFramePeriod(BSYNClib_VideoFormat * psFormat)
{
	unsigned int uiFramePeriod;
	
	BDBG_ENTER(BSYNClib_VideoFormat_P_GetFramePeriod);

	BDBG_ASSERT(psFormat);

	if (BSYNClib_VideoFormat_P_IsFrameRateSupported(psFormat->sSnapshot.eFrameRate))
	{
		uiFramePeriod = gauiFramePeriods[psFormat->sSnapshot.eFrameRate];
	}
	else
	{
		BDBG_WRN(("Unsupported sync frame rate; lipsync may suffer"));
		uiFramePeriod = gauiFramePeriods[0];
	}

	BDBG_LEAVE(BSYNClib_VideoFormat_P_GetFramePeriod);
	return uiFramePeriod;
}

unsigned int BSYNClib_VideoFormat_P_GetFramePeriod_isr(BSYNClib_VideoFormat * psFormat)
{
	unsigned int uiFramePeriod;
	
	BDBG_ENTER(BSYNClib_VideoFormat_P_GetFramePeriod_isr);

	BDBG_ASSERT(psFormat);

	if (BSYNClib_VideoFormat_P_IsFrameRateSupported_isr(psFormat->sData.eFrameRate))
	{
		uiFramePeriod = gauiFramePeriods[psFormat->sData.eFrameRate];
	}
	else
	{
		BDBG_WRN(("Unsupported sync frame rate; lipsync may suffer"));
		uiFramePeriod = gauiFramePeriods[0];
	}

	BDBG_LEAVE(BSYNClib_VideoFormat_P_GetFramePeriod_isr);
	return uiFramePeriod;
}

unsigned int BSYNClib_VideoFormat_P_GetVsyncPeriod(BSYNClib_VideoFormat * psFormat)
{
	unsigned int uiFieldPeriod;

	BDBG_ENTER(BSYNClib_VideoFormat_P_GetVsyncPeriod);

	BDBG_ASSERT(psFormat);

	uiFieldPeriod = BSYNClib_VideoFormat_P_GetFramePeriod(psFormat);
	
	if (psFormat->sSnapshot.bInterlaced)
	{
		uiFieldPeriod /= 2;
	}

	BDBG_LEAVE(BSYNClib_VideoFormat_P_GetVsyncPeriod);
	return uiFieldPeriod;	
}

unsigned int BSYNClib_VideoFormat_P_GetVsyncPeriod_isr(BSYNClib_VideoFormat * psFormat)
{
	unsigned int uiFieldPeriod;

	BDBG_ENTER(BSYNClib_VideoFormat_P_GetVsyncPeriod_isr);

	BDBG_ASSERT(psFormat);

	uiFieldPeriod = BSYNClib_VideoFormat_P_GetFramePeriod_isr(psFormat);
	
	if (psFormat->sData.bInterlaced)
	{
		uiFieldPeriod /= 2;
	}

	BDBG_LEAVE(BSYNClib_VideoFormat_P_GetVsyncPeriod_isr);
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

const char * BSYNClib_VideoFormat_P_GetFrameRateName(BAVC_FrameRateCode eRate)
{
	const char * pcName = NULL;
	BDBG_ENTER(BSYNClib_VideoFormat_P_GetFrameRateName);

	BKNI_EnterCriticalSection();
	pcName = BSYNClib_VideoFormat_P_GetFrameRateName_isr(eRate);
	BKNI_LeaveCriticalSection();

	BDBG_LEAVE(BSYNClib_VideoFormat_P_GetFrameRateName);
	return pcName;
}

const char * BSYNClib_VideoFormat_P_GetFrameRateName_isr(BAVC_FrameRateCode eRate)
{
	const char * pcName = NULL;
	BDBG_ENTER(BSYNClib_VideoFormat_P_GetFrameRateName_isr);

	if (BSYNClib_VideoFormat_P_IsFrameRateSupported_isr(eRate))
	{
		pcName = gapcFrameRateNames[eRate];
	}
	else
	{
		BDBG_WRN(("Unsupported sync frame rate; lipsync will suffer"));
		pcName = gapcUnsupportedFrameRateName;
	}

	BDBG_LEAVE(BSYNClib_VideoFormat_P_GetFrameRateName_isr);
	return pcName;
}
#endif

