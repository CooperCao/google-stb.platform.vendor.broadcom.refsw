/***************************************************************************
 *  Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#ifndef NEXUS_SIMPLE_ENCODER_SERVER_H__
#define NEXUS_SIMPLE_ENCODER_SERVER_H__

#include "nexus_simple_encoder.h"
#include "nexus_playpump.h"
#ifdef NEXUS_HAS_STREAM_MUX
#include "nexus_video_encoder.h"
#include "nexus_stream_mux.h"
#endif
#if NEXUS_HAS_AUDIO
#include "nexus_audio_mixer.h"
#endif
#include "nexus_core_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
This server-side API is semi-private. In multi-process systems, only server apps like nxserver will call it.
Client apps will not call it. Therefore, this API is subject to non-backward compatible change.
**/

typedef struct NEXUS_SimpleEncoderServer *NEXUS_SimpleEncoderServerHandle;

NEXUS_SimpleEncoderServerHandle NEXUS_SimpleEncoderServer_Create( /* attr{destructor=NEXUS_SimpleEncoderServer_Destroy}  */
    void
    );

void NEXUS_SimpleEncoderServer_Destroy(
    NEXUS_SimpleEncoderServerHandle handle
    );

/**
Summary:
**/
NEXUS_SimpleEncoderHandle NEXUS_SimpleEncoder_Create( /* attr{destructor=NEXUS_SimpleEncoder_Destroy} */
    NEXUS_SimpleEncoderServerHandle server,
    unsigned id
    );

/**
Summary:
**/
void NEXUS_SimpleEncoder_Destroy(
    NEXUS_SimpleEncoderHandle handle
    );

/**
Video and audio encode resources are opened externally so they can be passed between instances of SimpleEncoder. 
**/
typedef struct NEXUS_SimpleEncoderServerSettings
{
    unsigned transcodeDisplayIndex;
    bool headless;
    NEXUS_AudioMuxOutputHandle audioMuxOutput;
    NEXUS_VideoEncoderHandle videoEncoder;
#define NEXUS_SIMPLE_ENCODER_NUM_PLAYPUMPS 3
    NEXUS_PlaypumpHandle playpump[NEXUS_SIMPLE_ENCODER_NUM_PLAYPUMPS];
    NEXUS_StcChannelHandle stcChannelTranscode; /* only needed for realtime */
    NEXUS_Timebase timebase;
    NEXUS_AudioMixerHandle mixer; /* must be dsp mixer */
    bool nonRealTime;
    
    NEXUS_SimpleDecoderDisableMode disableMode;

    /* with display encode, encode starts before any decode is known. */
    struct {
        NEXUS_DisplayHandle display;
        NEXUS_SimpleAudioDecoderHandle masterAudio; /* simple decoder for this encoder */
        NEXUS_SimpleAudioDecoderHandle slaveAudio; /* simple decoder which currently has audio */
    } displayEncode;
} NEXUS_SimpleEncoderServerSettings;

/**
Summary:
**/
void NEXUS_SimpleEncoder_GetServerSettings(
    NEXUS_SimpleEncoderServerHandle server,
    NEXUS_SimpleEncoderHandle handle,
    NEXUS_SimpleEncoderServerSettings *pSettings
    );

/**
Summary:
**/
NEXUS_Error NEXUS_SimpleEncoder_SetServerSettings(
    NEXUS_SimpleEncoderServerHandle server,
    NEXUS_SimpleEncoderHandle handle,
    const NEXUS_SimpleEncoderServerSettings *pSettings
    );

#ifdef __cplusplus
}
#endif

#endif
