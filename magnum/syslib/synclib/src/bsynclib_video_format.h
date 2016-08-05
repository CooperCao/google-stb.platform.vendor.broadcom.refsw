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
#ifndef BSYNCLIB_VIDEO_FORMAT_H__
#define BSYNCLIB_VIDEO_FORMAT_H__

#include "bstd.h"
#include "bavc.h"

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
void BSYNClib_VideoFormat_Diff_isr(BSYNClib_VideoFormat * psDesired, BSYNClib_VideoFormat * psCurrent, BSYNClib_VideoFormat_DiffResults * psResults);
void BSYNClib_VideoFormat_Patch_isr(BSYNClib_VideoFormat * psDesired, BSYNClib_VideoFormat * psCurrent, BSYNClib_VideoFormat_DiffResults * psResults);
void BSYNClib_VideoFormat_Snapshot_isr(BSYNClib_VideoFormat * psFormat);
unsigned int BSYNClib_VideoFormat_P_GetDownconvertedFramePeriod_isrsafe(BSYNClib_VideoFormat * psFormat);
unsigned int BSYNClib_VideoFormat_P_GetUpconvertedFramePeriod_isrsafe(BSYNClib_VideoFormat * psFormat);
unsigned int BSYNClib_VideoFormat_P_GetFramePeriod_isrsafe(BSYNClib_VideoFormat * psFormat);
unsigned int BSYNClib_VideoFormat_P_GetVsyncPeriod_isrsafe(BSYNClib_VideoFormat * psFormat);
bool BSYNClib_VideoFormat_P_IsFrameRateSupported_isrsafe(BAVC_FrameRateCode eRate);
#ifdef BDBG_DEBUG_BUILD
const char * BSYNClib_VideoFormat_P_GetFrameRateName_isrsafe(BAVC_FrameRateCode eRate);
#endif

#endif /* BSYNCLIB_VIDEO_FORMAT_H__ */
