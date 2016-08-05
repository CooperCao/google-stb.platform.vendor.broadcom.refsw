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
#ifndef NEXUS_SYNC_CHANNEL_H__
#define NEXUS_SYNC_CHANNEL_H__

/*=***********************************
SyncChannel provides high-precision lipsync by making fine tuned adjustments
to audio and video inputs and outputs.

First, you must use StcChannel to provide basic lipsync using TSM.
Then, you can add SyncChannel to fine tune the lipsync.

The information SyncChannel uses includes, but is not limited to:
o Total delay in Display video path (which varies for HD and SD paths based on # of capture buffers)
o PTS/STC phase offset in video decoder
o Total delay in Audio path (due to offset in the PCM ring buffers)

See nexus/examples/sync_channel.c for a sample app.

*************************************/

#include "nexus_video_input.h"
#include "nexus_audio_input.h"
#include "nexus_audio_output.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
Summary:
Handle for SyncChannel interface
**/
typedef struct NEXUS_SyncChannel *NEXUS_SyncChannelHandle;

/**
Summary:
Supported number of audio inputs for sync channel.  This can differ from num inputs
defined in nexus_platform_features.h.
**/
#define NEXUS_SYNC_CHANNEL_NUM_AUDIO_INPUTS 6

/**
Summary:
Settings for NEXUS_SyncChannel

Description:
All audio and video input handles used to present a single program should be specified in NEXUS_SyncChannelSettings.
Internally, SyncChannel will determine which outputs are tied to those inputs.
Then, SyncChannel will monitor those inputs and outputs and make necessary adjustments to them in order to achieve precision lipsync.

If an input is no longer used (e.g. channel change from digital to analog, system shutdown, etc.),
the app should NULL out those handles as follows:

    NEXUS_SyncChannel_GetSettings(syncChannel, &settings);
    settings.videoInput = NULL;
    settings.audioInput[0] = NULL;
    settings.audioInput[1] = NULL;
    NEXUS_SyncChannel_SetSettings(syncChannel, &settings);

If an output belonging to a sync-managed input is removed from that input, or a new output is added to a sync-managed input,
(e.g., a window is disconnected/reconnected, audio output is switched between inputs) then SyncChannel must be told to rediscover the new
input-output path connections. You should call NEXUS_SyncChannel_SetSettings with all NULL's, then call it again with the inputs you intend
to use for the next decode session. This will force sync to rediscover the new input-output connections.  This forced rediscovery of connections
is not required for cases where an output's properties change without changing its connection to the input (e.g., display format change, audio volume
change).

SyncChannel provides two types of lipsync improvements to TSM. They are:
1) basic lipsync - sync multiple displays and multiple audio outputs together. also, sync audio and video within +/- 1 video frame.
2) precision lipsync - adds subframe audio adjustments based on video feedback
**/
typedef struct NEXUS_SyncChannelSettings
{
    NEXUS_VideoInput videoInput; /* SyncChannel supports one video input */
    NEXUS_AudioInputHandle audioInput[NEXUS_SYNC_CHANNEL_NUM_AUDIO_INPUTS]; /* Both
        audio inputs must be for the same program. Two audio inputs are used
        for PCM and compressed-pass through configurations */
    bool enablePrecisionLipsync; /* enables subframe audio adjustments based on
        video feedback */
    bool enableMuteControl; /* Enables control over video and audio muting to
        ensure we remain muted until adjustments are completed */
    bool allowIncrementalStart; /* defaults false, which requires all audio and
        video sync channel configuration before start. If true, you can
        incrementally configure sync for audio and video, but audio-only or
        video-only streams will be muted for 5 second timeout, which is not
        recommended. */
    bool simultaneousUnmute; /* This will cause all devices to unmute at the
        same time.  This time will be the largest of all of the unmute timout
        values set amongst the devices (excluding the unconditional unmutes).
        Defaults to false. */
    unsigned adjustmentThreshold; /* threshold below which no further attempts
        to adjust lipsync occur, in ms. This maps to
        VideoSourceConfig.sJitterToleranceImprovementThreshold.uiValue in
        synclib, which is already in ms */
    unsigned customVideoDelay; /* in ms, this maps to
        VideoSourceConfig.sDelay.sCustom.uiValue in synclib, which is already
        in ms */
    unsigned customAudioDelay[NEXUS_SYNC_CHANNEL_NUM_AUDIO_INPUTS]; /* in ms,
        this maps to AudioSourceConfig.sDelay.sCustom.uiValue in synclib, which
        is already in ms */
} NEXUS_SyncChannelSettings;

/**
Summary:
Get default settings
**/
void NEXUS_SyncChannel_GetDefaultSettings(
    NEXUS_SyncChannelSettings *pSettings /* [out] */
    );

/**
Summary:
Create a new SyncChannel.

Description:
You will need one SyncChannel for every program for which you want high-precision lipsync.

You cannot attach an input to more than one SyncChannel at a time.
**/
NEXUS_SyncChannelHandle NEXUS_SyncChannel_Create( /* attr{destructor=NEXUS_SyncChannel_Destroy}  */
    const NEXUS_SyncChannelSettings *pSettings /* attr{null_allowed=y} */
    );

/**
Summary:
Destroy a SyncChannel.
**/
void NEXUS_SyncChannel_Destroy(
    NEXUS_SyncChannelHandle handle
    );

/**
Summary:
Get current settings.
**/
void NEXUS_SyncChannel_GetSettings(
    NEXUS_SyncChannelHandle    handle,
    NEXUS_SyncChannelSettings *pSettings /* [out] */
    );

/**
Summary:
Apply new settings.

Description:
See NEXUS_SyncChannelSettings for usage requirements
**/
NEXUS_Error NEXUS_SyncChannel_SetSettings(
    NEXUS_SyncChannelHandle          handle,
    const NEXUS_SyncChannelSettings *pSettings
    );

#ifdef __cplusplus
}
#endif

#endif
