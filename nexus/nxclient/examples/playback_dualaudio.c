/***************************************************************************
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
 **************************************************************************/
#include "nxclient.h"
#include "nexus_playback.h"
#include "nexus_surface_client.h"
#include "bstd.h"
#include "bkni.h"
#include "bkni_multi.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

BDBG_MODULE(playback);
#include "nxapp_prompt.inc"

/* This stream has two audio pids in a single program.
Stream is located at \\brcm-irv.broadcom.com\dfs\projects\stbdevstream_scratch\streams\playback inside Broadcom.
Can be replaced with any dual audio track stream. */
#define FILE_NAME "videos/clifford_dualaudio.mpg"
#define TRANSPORT_TYPE NEXUS_TransportType_eTs
#define VIDEO_CODEC NEXUS_VideoCodec_eMpeg2
#define AUDIO_CODEC NEXUS_AudioCodec_eAc3
#define VIDEO_PID 0x41
#define AUDIO_0_PID 0x44
#define AUDIO_1_PID 0x45

int main(void)
{
    NxClient_AllocSettings allocSettings;
    NxClient_ConnectSettings connectSettings;
    struct {
        NxClient_AllocResults allocResults;
        unsigned connectId;
    } track[3]; /* 0 is video, 1 and 2 are audio */
    NEXUS_PlaypumpHandle playpump;
    NEXUS_PlaypumpOpenSettings playpumpOpenSettings;
    NEXUS_PlaybackHandle playback;
    NEXUS_PlaybackSettings playbackSettings;
    NEXUS_PlaybackPidChannelSettings playbackPidSettings;
    NEXUS_FilePlayHandle file;
    NEXUS_SimpleVideoDecoderHandle videoDecoder;
    NEXUS_SimpleVideoDecoderStartSettings videoProgram;
    NEXUS_SimpleAudioDecoderHandle audioDecoder[2];
    NEXUS_SimpleAudioDecoderStartSettings audioProgram;
    NEXUS_PidChannelHandle audioPidChannel[2], videoPidChannel;
    NEXUS_SimpleStcChannelHandle stcChannel;
    NEXUS_Error rc;
    unsigned i, audioConnect;

    rc = NxClient_Join(NULL);
    if (rc) return -1;

    NxClient_GetDefaultAllocSettings(&allocSettings);
    allocSettings.simpleVideoDecoder = 1;
    rc = NxClient_Alloc(&allocSettings, &track[0].allocResults);
    if (rc) return BERR_TRACE(rc);
    NxClient_GetDefaultAllocSettings(&allocSettings);
    allocSettings.simpleAudioDecoder = 1;
    rc = NxClient_Alloc(&allocSettings, &track[1].allocResults);
    if (rc) return BERR_TRACE(rc);
    NxClient_GetDefaultAllocSettings(&allocSettings);
    allocSettings.simpleAudioDecoder = 1;
    rc = NxClient_Alloc(&allocSettings, &track[2].allocResults);
    if (rc) return BERR_TRACE(rc);

    file = NEXUS_FilePlay_OpenPosix(FILE_NAME, NULL);

    NEXUS_Playpump_GetDefaultOpenSettings(&playpumpOpenSettings);
    playpump = NEXUS_Playpump_Open(NEXUS_ANY_ID, &playpumpOpenSettings);
    playback = NEXUS_Playback_Create();
    BDBG_ASSERT(playback);
    stcChannel = NEXUS_SimpleStcChannel_Create(NULL);

    NEXUS_Playback_GetSettings(playback, &playbackSettings);
    playbackSettings.playpump = playpump;
    playbackSettings.simpleStcChannel = stcChannel;
    playbackSettings.stcTrick = true;
    playbackSettings.playpumpSettings.transportType = TRANSPORT_TYPE;
    rc = NEXUS_Playback_SetSettings(playback, &playbackSettings);
    BDBG_ASSERT(!rc);

    videoDecoder = NEXUS_SimpleVideoDecoder_Acquire(track[0].allocResults.simpleVideoDecoder[0].id);
    audioDecoder[0] = NEXUS_SimpleAudioDecoder_Acquire(track[1].allocResults.simpleAudioDecoder.id);
    audioDecoder[1] = NEXUS_SimpleAudioDecoder_Acquire(track[2].allocResults.simpleAudioDecoder.id);

    NxClient_GetDefaultConnectSettings(&connectSettings);
    connectSettings.simpleVideoDecoder[0].id = track[0].allocResults.simpleVideoDecoder[0].id;
    rc = NxClient_Connect(&connectSettings, &track[0].connectId);
    if (rc) return BERR_TRACE(rc);

    NxClient_GetDefaultConnectSettings(&connectSettings);
    connectSettings.simpleAudioDecoder.id = track[1].allocResults.simpleAudioDecoder.id;
    connectSettings.simpleAudioDecoder.primer = true;
    rc = NxClient_Connect(&connectSettings, &track[1].connectId);
    if (rc) return BERR_TRACE(rc);

    NxClient_GetDefaultConnectSettings(&connectSettings);
    connectSettings.simpleAudioDecoder.id = track[2].allocResults.simpleAudioDecoder.id;
    connectSettings.simpleAudioDecoder.primer = true;
    rc = NxClient_Connect(&connectSettings, &track[2].connectId);
    if (rc) return BERR_TRACE(rc);

    NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
    playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eAudio;
    playbackPidSettings.pidTypeSettings.audio.simpleDecoder = audioDecoder[0];
    audioPidChannel[0] = NEXUS_Playback_OpenPidChannel(playback, AUDIO_0_PID, &playbackPidSettings);
    playbackPidSettings.pidTypeSettings.audio.simpleDecoder = audioDecoder[1];
    audioPidChannel[1] = NEXUS_Playback_OpenPidChannel(playback, AUDIO_1_PID, &playbackPidSettings);

    NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
    playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eVideo;
    playbackPidSettings.pidTypeSettings.video.codec = VIDEO_CODEC;
    playbackPidSettings.pidTypeSettings.video.index = true;
    playbackPidSettings.pidTypeSettings.video.simpleDecoder = videoDecoder;
    videoPidChannel = NEXUS_Playback_OpenPidChannel(playback, VIDEO_PID, &playbackPidSettings);

    NEXUS_SimpleVideoDecoder_SetStcChannel(videoDecoder, stcChannel);
    NEXUS_SimpleAudioDecoder_SetStcChannel(audioDecoder[0], stcChannel);
    NEXUS_SimpleAudioDecoder_SetStcChannel(audioDecoder[1], stcChannel);

    NEXUS_SimpleVideoDecoder_GetDefaultStartSettings(&videoProgram);
    videoProgram.settings.pidChannel = videoPidChannel;
    videoProgram.settings.codec = VIDEO_CODEC;
    NEXUS_SimpleVideoDecoder_Start(videoDecoder, &videoProgram);

    NEXUS_SimpleAudioDecoder_GetDefaultStartSettings(&audioProgram);
    audioProgram.primary.pidChannel = audioPidChannel[0];
    audioProgram.primary.codec = AUDIO_CODEC;
    audioProgram.primer.pcm = true;
    audioProgram.primer.compressed = true;
    NEXUS_SimpleAudioDecoder_Start(audioDecoder[0], &audioProgram);

    NEXUS_SimpleAudioDecoder_GetDefaultStartSettings(&audioProgram);
    audioProgram.primary.pidChannel = audioPidChannel[1];
    audioProgram.primary.codec = AUDIO_CODEC;
    audioProgram.primer.pcm = true;
    audioProgram.primer.compressed = true;
    NEXUS_SimpleAudioDecoder_Start(audioDecoder[1], &audioProgram);

    NEXUS_Playback_Start(playback, file, NULL);

    audioConnect = 1;
    while (1) {
        NxClient_ReconfigSettings reconfig;

        nxapp_prompt("toggle");

        NxClient_GetDefaultReconfigSettings(&reconfig);
        reconfig.command[0].connectId1 = track[1].connectId;
        reconfig.command[0].connectId2 = track[2].connectId;
        reconfig.command[0].type = NxClient_ReconfigType_eRerouteAudio;
        NxClient_Reconfig(&reconfig);
        if (++audioConnect == 3) audioConnect = 1;
    }

    NEXUS_Playback_Stop(playback);
    NEXUS_SimpleAudioDecoder_Stop(audioDecoder[0]);
    NEXUS_SimpleAudioDecoder_Stop(audioDecoder[1]);
    NEXUS_SimpleVideoDecoder_Stop(videoDecoder);
    NEXUS_Playback_ClosePidChannel(playback, videoPidChannel);
    NEXUS_Playback_ClosePidChannel(playback, audioPidChannel[0]);
    NEXUS_Playback_ClosePidChannel(playback, audioPidChannel[1]);
    NEXUS_Playback_Destroy(playback);
    NEXUS_Playpump_Close(playpump);
    NEXUS_FilePlay_Close(file);
    for (i=0;i<3;i++) {
        NxClient_Disconnect(track[i].connectId);
        NxClient_Free(&track[i].allocResults);
    }
    NxClient_Uninit();
    return 0;
}
