/***************************************************************************
 *  Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 **************************************************************************/
#ifndef NEXUS_SIMPLE_AUDIO_PLAYBACK_SERVER_H__
#define NEXUS_SIMPLE_AUDIO_PLAYBACK_SERVER_H__

#include "nexus_types.h"
#include "nexus_simple_audio_playback.h"
#if NEXUS_HAS_AUDIO
#include "nexus_audio_playback.h"
#include "nexus_i2s_input.h"
#include "nexus_spdif_output.h"
#endif
#if NEXUS_HAS_HDMI_OUTPUT
#include "nexus_hdmi_output.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define NEXUS_MAX_AUDIO_PLAYBACK_SPDIF_OUTPUTS 2
#define NEXUS_MAX_SIMPLE_AUDIO_PLAYBACK_HDMI_OUTPUTS 2

/**
This server-side API is semi-private. In multi-process systems, only server apps like nxserver will call it.
Client apps will not call it. Therefore, this API is subject to non-backward compatible change.
**/

typedef struct NEXUS_SimpleAudioPlaybackServer *NEXUS_SimpleAudioPlaybackServerHandle;

NEXUS_SimpleAudioPlaybackServerHandle NEXUS_SimpleAudioPlaybackServer_Create( /* attr{destructor=NEXUS_SimpleAudioPlaybackServer_Destroy}  */
                                                                              void
                                                                            );
void NEXUS_SimpleAudioPlaybackServer_Destroy(
    NEXUS_SimpleAudioPlaybackServerHandle handle
    );

typedef struct NEXUS_SimpleAudioPlaybackServerSettings
{
    /* NEXUS_SimpleAudioDecoderHandle decoder; */
    NEXUS_AudioPlaybackHandle playback;
    NEXUS_I2sInputHandle i2sInput;

    struct {
        bool enabled;
        NEXUS_SpdifOutputHandle spdifOutputs[NEXUS_MAX_AUDIO_PLAYBACK_SPDIF_OUTPUTS];
        NEXUS_HdmiOutputHandle hdmiOutputs[NEXUS_MAX_SIMPLE_AUDIO_PLAYBACK_HDMI_OUTPUTS];
    } compressed;
} NEXUS_SimpleAudioPlaybackServerSettings;

void NEXUS_SimpleAudioPlayback_GetDefaultServerSettings(
    NEXUS_SimpleAudioPlaybackServerSettings *pSettings /* [out] */
    );

NEXUS_SimpleAudioPlaybackHandle NEXUS_SimpleAudioPlayback_Create( /* attr{destructor=NEXUS_SimpleAudioPlayback_Destroy}  */
    NEXUS_SimpleAudioPlaybackServerHandle server,
    unsigned index,
    const NEXUS_SimpleAudioPlaybackServerSettings *pSettings /* attr{null_allowed=y} */
    );

void NEXUS_SimpleAudioPlayback_Destroy(
    NEXUS_SimpleAudioPlaybackHandle handle
    );

void NEXUS_SimpleAudioPlayback_GetServerSettings(
    NEXUS_SimpleAudioPlaybackServerHandle server,
    NEXUS_SimpleAudioPlaybackHandle handle,
    NEXUS_SimpleAudioPlaybackServerSettings *pSettings /* [out] */
    );

NEXUS_Error NEXUS_SimpleAudioPlayback_SetServerSettings(
    NEXUS_SimpleAudioPlaybackServerHandle server,
    NEXUS_SimpleAudioPlaybackHandle handle,
    const NEXUS_SimpleAudioPlaybackServerSettings *pSettings
    );

#ifdef __cplusplus
}
#endif

#endif
