/******************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *
 * Module Description:
 *  Example program to demonstrate receiving live or playback content over IP Channels (UDP/RTP/RTSP/HTTP based)
 *
 ******************************************************************************/
#ifndef MEDIA_PLAYER_PRIV_H__
#define MEDIA_PLAYER_PRIV_H__

#include "nexus_simple_video_decoder.h"
#include "nexus_simple_audio_decoder.h"
#include "nexus_simple_stc_channel.h"
#include "nexus_surface_client.h"
#include "nexus_playback.h"
#include "nexus_file.h"
#include "nexus_core_utils.h"
#include "namevalue.h"
#include "media_probe.h"
#include "nxclient.h"
#include "blst_queue.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include "bstd.h"
#include "bkni.h"

struct media_player
{
    BDBG_OBJECT(media_player)
    media_player_create_settings create_settings;
    media_player_start_settings start_settings;
    bool started;

    BLST_Q_ENTRY(media_player) link; /* for 'players' list */
    BLST_Q_HEAD(media_player_list, media_player) players; /* all players, including the master itself */
    media_player_t master; /* points to player[0], even for the master itself */
    unsigned mosaic_start_count;

    NEXUS_SimpleVideoDecoderHandle videoDecoder;
    NEXUS_VideoDecoderSettings videoDecoderSettings;
    NEXUS_SimpleAudioDecoderHandle audioDecoder;
    NEXUS_FilePlayHandle file;
    NEXUS_PlaypumpHandle playpump;
    NEXUS_PlaybackHandle playback;
    NEXUS_PidChannelHandle pcrPidChannel;
    NEXUS_SimpleStcChannelHandle stcChannel;
    NxClient_AllocResults allocResults;
    unsigned allocIndex;
    unsigned connectId;
    struct {
        NxClient_AllocResults allocResults;
        unsigned connectId;
        unsigned persistentMasterConnectId;
    } audio;
    NEXUS_SimpleVideoDecoderStartSettings videoProgram;
    NEXUS_SimpleAudioDecoderStartSettings audioProgram;
    media_player_settings settings;
    dvr_crypto_t crypto;
    unsigned colorDepth;
    bool stcTrick;
    NEXUS_PlaybackTrickModeSettings trickMode;
};

/*
syntax: "scheme://domain:port/path?query_string#fragment_id"
each char array is null-terminated
*/
struct url {
    char scheme[32];
    char domain[128];
    unsigned port;
    char path[256]; /* contains "/path?query_string#fragment_id" */
};

#endif /* MEDIA_PLAYER_PRIV_H__ */
