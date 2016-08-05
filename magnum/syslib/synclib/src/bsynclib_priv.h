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
#ifndef BSYNCLIB_PRIV_H__
#define BSYNCLIB_PRIV_H__

#include "bsyslib_list.h"
#include "bsynclib.h"

/*
 * 20151208 bandrews - some customers prefer video to have an initial delay and
 * then go down, rather than having a pause when we apply it, because they
 * are not using mute control
 */
/* default video priming delay */
#define BSYNCLIB_VIDEO_INITIAL_DELAY 150       /* ms */
#define BSYNCLIB_VIDEO_TSM_LOCK_TIMER_DEFAULT_TIMEOUT 100 /* ms */

/* set to zero means shut it off */
#define BSYNCLIB_VIDEO_RATE_MISMATCH_DETECTION_TIMER_DEFAULT_TIMEOUT 0 /* ms */
#define BSYNCLIB_VIDEO_RATE_REMATCH_DETECTION_TIMER_DEFAULT_TIMEOUT 0 /* ms */
#define BSYNCLIB_VIDEO_RATE_MISMATCH_DETECTION_DEFAULT_ACCEPTABLE_MTBC_LOWER 300
#define BSYNCLIB_VIDEO_RATE_MISMATCH_DETECTION_DEFAULT_ACCEPTABLE_MTBC_UPPER 1500
#define BSYNCLIB_VIDEO_RATE_MISMATCH_DETECTION_DEFAULT_ACCEPTABLE_TTLC 1500
#define BSYNCLIB_VIDEO_RATE_REMATCH_DETECTION_DEFAULT_ACCEPTABLE_TTLC 9000

#define BSYNCLIB_AUDIO_UNMUTE_DEFAULT_TIMEOUT 200 /* ms */
#define BSYNCLIB_VIDEO_UNMUTE_DEFAULT_TIMEOUT 133 /* ms */

/* 20070406 bandrews - compensate for external audio receiver on SPDIF output */
#define BSYNCLIB_AUDIO_RECEIVER_DELAY_COMPENSATION_DEFAULT 0 /* ms */

#define BSYNCLIB_UNCONDITIONAL_VIDEO_UNMUTE_SUPPORT 1
#define BSYNCLIB_VIDEO_UNCONDITIONAL_UNMUTE_DEFAULT_TIMEOUT 5000
#define BSYNCLIB_UNCONDITIONAL_AUDIO_UNMUTE_SUPPORT 1
#define BSYNCLIB_AUDIO_UNCONDITIONAL_UNMUTE_DEFAULT_TIMEOUT 5000

/*
Summary:
*/
struct BSYNClib_Impl
{
	BSYNClib_Settings sSettings;
	BSYSlib_List_Handle hChannels;
};

bool BSYNClib_P_Enabled_isrsafe(BSYNClib_Handle hSync);
unsigned int BSYNClib_P_Convert_isrsafe(unsigned int uiValue, BSYNClib_Units eFromUnits, BSYNClib_Units eToUnits);
int BSYNClib_P_ConvertSigned_isrsafe(int iValue, BSYNClib_Units eFromUnits, BSYNClib_Units eToUnits);

#if BDBG_DEBUG_BUILD
extern const char * const BSYNClib_P_UnitsStrings[];
#endif

#endif /* BSYNCLIB_PRIV_H__ */
