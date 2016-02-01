/***************************************************************************
 *     (c)2007-2012 Broadcom Corporation
 *
 *  This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 **************************************************************************/
#ifndef NEXUS_PID_CHANNEL_SCRAMBLING_H__
#define NEXUS_PID_CHANNEL_SCRAMBLING_H__

#include "nexus_types.h"
#include "nexus_rave.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
The transport RAVE hardware can capture MPEG2 TS scrambling control bits from incoming transport packets.
This can be used to determine if a stream is encrypted.
**/

/**
Summary:
Settings for a pid scrambling check
**/
typedef struct NEXUS_PidChannelScramblingSettings
{
    bool pusiOnly; /* If pusiOnly is true, then only the scrambling control bits in transport packets with the PUSI bit (packet unit start indicator) set are tested.
                      If pusiOnly is false, then scrambling control bits from any transport packet are captured. */
    NEXUS_RaveHandle raveContext; /* optional RAVE context temporarily obtained from decoder. See NEXUS_VideoDecoder_DetachRaveContext. 
                                     The decoder must be already started before calling NEXUS_PidChannel_StartScramblingCheck because the decoder must configure
                                     the RAVE context for simultaneous use. */
} NEXUS_PidChannelScramblingSettings;

/**
Summary:
Get default settings
**/
void NEXUS_PidChannel_GetDefaultScramblingSettings(
    NEXUS_PidChannelScramblingSettings *pSettings /* [out] */
    );

/**
Summary:
Start monitoring a PID to determine what, if any, scrambling is present.

Description:
After starting the check, call NEXUS_PidChannel_GetScramblingStatus to get results.
**/
NEXUS_Error NEXUS_PidChannel_StartScramblingCheck(
    NEXUS_PidChannelHandle pidChannel,
    const NEXUS_PidChannelScramblingSettings *pSettings /* attr{null_allowed=y} */
    );

/**
Summary:
Stop monitoring a pid for scrambling
**/
void NEXUS_PidChannel_StopScramblingCheck(
    NEXUS_PidChannelHandle pidChannel
    );

/**
Summary:
Status read from NEXUS_PidChannel_GetScramblingStatus
**/
typedef struct NEXUS_PidChannelScramblingStatus
{
    bool pidExists; /* If false, no packets for this PID were found and the other fields in this status structure are undefined.
                       If true, then at least one packet for this PID was found.
                       This field is reset to false when NEXUS_PidChannel_StartScramblingCheck is called. */
    bool scrambled; /* Ignore this field until pidExists is true.
                       Once pidExists is true, "scrambled" be set to true if any packet passes with the its scrambling control bits set.
                       If false, then no packet has been seen.
                       This field is reset to false when NEXUS_PidChannel_StartScramblingCheck is called. */
} NEXUS_PidChannelScramblingStatus;

/**
Summary:
Get status of a scrambling check.

Description:
Call this after NEXUS_PidChannel_StartScramblingCheck.
The application should poll this function for whatever amount of time it sees fit.
The amount of time and number of calls depends on the application's timeout requirements.
It could wait for a fix amount of time, then read it once. It could loop and read it multiple times.

There is no interrupt for the scrambling control capture hardware. Only polling is available.
**/
NEXUS_Error NEXUS_PidChannel_GetScramblingStatus(
    NEXUS_PidChannelHandle pidChannel,
    NEXUS_PidChannelScramblingStatus *pStatus /* [out] */
    );

/**
Summary:
Clear current status of a scrambling check.
**/
void NEXUS_PidChannel_ClearScramblingStatus(
    NEXUS_PidChannelHandle pidChannel
    );

#ifdef __cplusplus
}
#endif

#endif /* NEXUS_PID_CHANNEL_SCRAMBLING_H__ */
