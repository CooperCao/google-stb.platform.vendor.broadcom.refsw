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
 **************************************************************************/
#ifndef NEXUS_SIMPLE_STC_CHANNEL_H__
#define NEXUS_SIMPLE_STC_CHANNEL_H__

#include "nexus_types.h"
#include "nexus_stc_channel.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
Summary:
SimpleStcChannel is a virtualization of StcChannel
Client cannot choose timebase or stcIndex.
Underlying StcChannel may be removed.
**/
typedef struct NEXUS_SimpleStcChannel *NEXUS_SimpleStcChannelHandle;

/**
Summary:
Modes for handling high jitter PCR's in IP streaming 
**/
typedef enum NEXUS_SimpleStcChannelHighJitterMode
{
    NEXUS_SimpleStcChannelHighJitterMode_eNone,      /* Not high jitter */
    NEXUS_SimpleStcChannelHighJitterMode_eDirect,    /* High jitter goes directly to a DPCR, but that DPCR is separate from the display. */
    NEXUS_SimpleStcChannelHighJitterMode_eTtsPacing, /* SW Buffering & Content Paced by local Playback Hardware using TTS Timestamps */
    NEXUS_SimpleStcChannelHighJitterMode_ePcrPacing,  /* SW Buffering & Content Paced by local Playback Hardware using PCRs as Timestamps */
    NEXUS_SimpleStcChannelHighJitterMode_eMax
} NEXUS_SimpleStcChannelHighJitterMode;

/**
Summary:
Settings for NEXUS_SimpleStcChannelHighJitterSettings
**/
typedef struct NEXUS_SimpleStcChannelHighJitterSettings
{
    NEXUS_SimpleStcChannelHighJitterMode mode;
    unsigned threshold;      /* In msec, threshold for filtering PCR change notifications to decoders. Only applies for NEXUS_SimpleStcChannelHighJitterMode_eDirect. */
} NEXUS_SimpleStcChannelHighJitterSettings;

/**
Summary:
Sync channel behavioral modes
**/
typedef enum NEXUS_SimpleStcChannelSyncMode
{
    NEXUS_SimpleStcChannelSyncMode_eOff, /* Do not perform basic or precision delay equalization. TSM is still performed. Fastest time to a/v presentation, but
        will not meet several industry a/v sync specs. */
    NEXUS_SimpleStcChannelSyncMode_eDefaultAdjustmentConcealment, /* Perform basic and possibly precision delay equalization.  Keeps decoders muted to avoid
        showing adjustment artifacts. Mute time depends on stream muxing and can be up to 10 seconds. */
    NEXUS_SimpleStcChannelSyncMode_eNoAdjustmentConcealment, /* Perform basic and possibly precision delay equalization.  Does not attempt to conceal adjustment
        artifacts.  Faster time to a/v presentation, but adjustment artifacts will likely be observable. */
    NEXUS_SimpleStcChannelSyncMode_eAudioAdjustmentConcealment, /* Perform basic and possibly precision delay equalization. Attempts to conceal audio adjustment
        artifacts via sample-level adjustments over a long period of time.  Fast time to audio presentation, but A/V sync may not be on target for up to 10 seconds.  */
    NEXUS_SimpleStcChannelSyncMode_eMax
} NEXUS_SimpleStcChannelSyncMode;

/**
Summary:
Settings for NEXUS_SimpleStcChannelSettings
**/
typedef struct NEXUS_SimpleStcChannelSettings
{
    NEXUS_StcChannelMode mode;
    struct
    {
        NEXUS_StcChannelPcrModeSettings pcr; /* A higher-level settings struct */
        NEXUS_SimpleStcChannelHighJitterSettings highJitter; /* only applies for ePcr mode */
        NEXUS_StcChannelAutoModeSettings Auto;
        NEXUS_StcChannelHostModeSettings host;
    } modeSettings;
    bool astm; /* If true, astm is enabled for this decode session. Defaults to false. */
    NEXUS_SimpleStcChannelSyncMode sync; /* Controls path delay equalization (sync_channel) behavior. Defaults to eDefaultAdjustmentConcealment. */
    NEXUS_StcChannelUnderflowHandling underflowHandling; /* select how underflows are handled in NRT mode */
    NEXUS_Timebase timebase; /* Optional user-owned and configured timebase. Must be opened with NEXUS_Timebase_Open(NEXUS_ANY_ID).
                                Internal StcChannel will be set to autoConfigTimebase = false. */
} NEXUS_SimpleStcChannelSettings;

void NEXUS_SimpleStcChannel_GetDefaultSettings(
    NEXUS_SimpleStcChannelSettings *pSettings
    );

NEXUS_SimpleStcChannelHandle NEXUS_SimpleStcChannel_Create( /* attr{destructor=NEXUS_SimpleStcChannel_Destroy}  */
    const NEXUS_SimpleStcChannelSettings *pSettings /* attr{null_allowed=y} */
    );

void NEXUS_SimpleStcChannel_Destroy(
    NEXUS_SimpleStcChannelHandle handle
    );
    
void NEXUS_SimpleStcChannel_GetSettings(
    NEXUS_SimpleStcChannelHandle handle,
    NEXUS_SimpleStcChannelSettings *pSettings
    );

NEXUS_Error NEXUS_SimpleStcChannel_SetSettings(
    NEXUS_SimpleStcChannelHandle handle,
    const NEXUS_SimpleStcChannelSettings *pSettings
    );

NEXUS_Error NEXUS_SimpleStcChannel_Invalidate(
    NEXUS_SimpleStcChannelHandle handle
    );

void NEXUS_SimpleStcChannel_GetStc(
    NEXUS_SimpleStcChannelHandle handle,
    uint32_t *pStc /* [out] Current Stc */
    );

NEXUS_Error NEXUS_SimpleStcChannel_SetStc(
    NEXUS_SimpleStcChannelHandle handle,
    uint32_t stc /* New STC */
    );

NEXUS_Error NEXUS_SimpleStcChannel_Freeze(
    NEXUS_SimpleStcChannelHandle handle,
    bool frozen /* if true, the STC stops. if false, the STC starts. */
    );

NEXUS_Error NEXUS_SimpleStcChannel_SetRate(
    NEXUS_SimpleStcChannelHandle handle,
    unsigned increment,
    unsigned prescale
    );
    
#ifdef __cplusplus
}
#endif

#endif
