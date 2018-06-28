/***************************************************************************
 * Copyright (C) 2017-2018 Broadcom. The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
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
#include <stdio.h>
#if NEXUS_HAS_PLAYBACK
#include "nexus_playback.h"
#include "nexus_surface_client.h"
#include "bstd.h"
#include "bkni.h"
#include "bkni_multi.h"
#include <stdlib.h>
#include <string.h>

BDBG_MODULE(playback);

/* the following define the input file and its characteristics -- these will vary by input file */
struct {
    const char *filename;
    unsigned pid;
} g_files[] = {
    {"videos/spider_cc.mpg", 0x11},
    {"videos/cnnticker.mpg", 0x21}};

#define TRANSPORT_TYPE NEXUS_TransportType_eTs
#define VIDEO_CODEC NEXUS_VideoCodec_eMpeg2

#if 0
static void first_pts(void *context, int param)
{
    NEXUS_SimpleVideoDecoderHandle videoDecoder = context;
    NEXUS_VideoDecoderStatus videoDecoderStatus;
    BSTD_UNUSED(param);
    NEXUS_SimpleVideoDecoder_GetStatus(videoDecoder, &videoDecoderStatus);
    BDBG_LOG(("first_pts: %x", videoDecoderStatus.pts));
}

static void first_pts_passed(void *context, int param)
{
    NEXUS_SimpleVideoDecoderHandle videoDecoder = context;
    NEXUS_VideoDecoderStatus videoDecoderStatus;
    BSTD_UNUSED(param);
    NEXUS_SimpleVideoDecoder_GetStatus(videoDecoder, &videoDecoderStatus);
    BDBG_LOG(("first_pts_passed: %x", videoDecoderStatus.pts));
}
#endif

int main(void)
{
    NxClient_AllocSettings allocSettings;
    NxClient_AllocResults allocResults;
    NxClient_ConnectSettings connectSettings;
    unsigned connectId;
    NEXUS_PlaypumpHandle playpump;
    NEXUS_PlaypumpOpenSettings playpumpOpenSettings;
    NEXUS_PlaybackHandle playback;
    NEXUS_PlaybackSettings playbackSettings;
    NEXUS_PlaybackPidChannelSettings playbackPidSettings;
    NEXUS_FilePlayHandle file;
    NEXUS_SimpleVideoDecoderHandle videoDecoder;
    NEXUS_SimpleVideoDecoderStartSettings videoProgram;
    NEXUS_SimpleStcChannelHandle stcChannel;
    NEXUS_Error rc;
    NEXUS_VideoDecoderSettings videoDecoderSettings;
    NEXUS_SimpleStcChannelSettings stcSettings;

    rc = NxClient_Join(NULL);
    if (rc) return -1;

    NxClient_GetDefaultAllocSettings(&allocSettings);
    allocSettings.simpleVideoDecoder = 1;
    rc = NxClient_Alloc(&allocSettings, &allocResults);
    if (rc) return BERR_TRACE(rc);

    NEXUS_Playpump_GetDefaultOpenSettings(&playpumpOpenSettings);
    playpump = NEXUS_Playpump_Open(NEXUS_ANY_ID, &playpumpOpenSettings);
    playback = NEXUS_Playback_Create();
    BDBG_ASSERT(playback);

    stcChannel = NEXUS_SimpleStcChannel_Create(NULL);
    /* NOTE: can't use high-precision lipsync (SyncChannel) with this technique because ptsOffsets get applied */
    NEXUS_SimpleStcChannel_GetSettings(stcChannel, &stcSettings);
    stcSettings.sync = NEXUS_SimpleStcChannelSyncMode_eOff;
    NEXUS_SimpleStcChannel_SetSettings(stcChannel, &stcSettings);

    NEXUS_Playback_GetSettings(playback, &playbackSettings);
    playbackSettings.playpump = playpump;
    playbackSettings.simpleStcChannel = stcChannel;
    playbackSettings.stcTrick = true;
    playbackSettings.playpumpSettings.transportType = TRANSPORT_TYPE;
    NEXUS_Playback_SetSettings(playback, &playbackSettings);

    videoDecoder = NEXUS_SimpleVideoDecoder_Acquire(allocResults.simpleVideoDecoder[0].id);

    NxClient_GetDefaultConnectSettings(&connectSettings);
    connectSettings.simpleVideoDecoder[0].id = allocResults.simpleVideoDecoder[0].id;
    connectSettings.simpleVideoDecoder[0].surfaceClientId = allocResults.surfaceClient[0].id;
    rc = NxClient_Connect(&connectSettings, &connectId);
    if (rc) return BERR_TRACE(rc);

    /* NOTE: we always set eHoldUntilTsmLock for this feature */
    NEXUS_SimpleVideoDecoder_GetSettings(videoDecoder, &videoDecoderSettings);
    videoDecoderSettings.channelChangeMode = NEXUS_VideoDecoder_ChannelChangeMode_eHoldUntilTsmLock;
#if 0
    videoDecoderSettings.firstPts.callback = first_pts;
    videoDecoderSettings.firstPts.context = videoDecoder;
    videoDecoderSettings.firstPtsPassed.callback = first_pts_passed;
    videoDecoderSettings.firstPtsPassed.context = videoDecoder;
#endif
    NEXUS_SimpleVideoDecoder_SetSettings(videoDecoder, &videoDecoderSettings);

    while (1) {
        NEXUS_VideoDecoderStatus videoDecoderStatus;
        uint32_t stc;
        uint32_t target_pts;
        unsigned index = 0;

        BDBG_LOG(("starting first stream so we have a picture to hold"));
        index = 0;
        file = NEXUS_FilePlay_OpenPosix(g_files[index].filename, NULL);
        NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
        playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eVideo;
        playbackPidSettings.pidTypeSettings.video.codec = VIDEO_CODEC;
        playbackPidSettings.pidTypeSettings.video.index = true;
        playbackPidSettings.pidTypeSettings.video.simpleDecoder = videoDecoder;
        NEXUS_SimpleVideoDecoder_GetDefaultStartSettings(&videoProgram);
        videoProgram.settings.pidChannel = NEXUS_Playback_OpenPidChannel(playback, g_files[index].pid, &playbackPidSettings);
        videoProgram.settings.codec = VIDEO_CODEC;

        NEXUS_SimpleVideoDecoder_SetStcChannel(videoDecoder, stcChannel);
        NEXUS_SimpleVideoDecoder_Start(videoDecoder, &videoProgram);
        NEXUS_Playback_Start(playback, file, NULL);
        BKNI_Sleep(2000);
        NEXUS_SimpleVideoDecoder_Stop(videoDecoder);
        NEXUS_Playback_Stop(playback);
        NEXUS_Playback_ClosePidChannel(playback, videoProgram.settings.pidChannel);
        NEXUS_FilePlay_Close(file);

        BDBG_LOG(("starting second stream"));
        index = 1;
        target_pts = 0x27d80; /* a few seconds into cnnticker.mpg */
        file = NEXUS_FilePlay_OpenPosix(g_files[index].filename, NULL);
        NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
        playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eVideo;
        playbackPidSettings.pidTypeSettings.video.codec = VIDEO_CODEC;
        playbackPidSettings.pidTypeSettings.video.index = true;
        playbackPidSettings.pidTypeSettings.video.simpleDecoder = videoDecoder;
        NEXUS_SimpleVideoDecoder_GetDefaultStartSettings(&videoProgram);
        videoProgram.settings.pidChannel = NEXUS_Playback_OpenPidChannel(playback, g_files[index].pid, &playbackPidSettings);
        videoProgram.settings.codec = VIDEO_CODEC;

        /* To queue:
        1) set STC in host control (so decoder doesn't set it)
        2) set STC value to be earlier than SetStartPts (so new picture is not displayed)
        3) freeze it (because we are going to pause there) */
        NEXUS_SimpleStcChannel_GetSettings(stcChannel, &stcSettings);
        stcSettings.mode = NEXUS_StcChannelMode_eHost;
        NEXUS_SimpleStcChannel_SetSettings(stcChannel, &stcSettings);
        NEXUS_SimpleStcChannel_Freeze(stcChannel, true);
        /* 10000 was chosen because it's early but within the "discard threshold"
        which (I believe) defaults to 2 seconds for MPEG and 10 seconds for other codecs */
        NEXUS_SimpleStcChannel_SetStc(stcChannel, target_pts - 10000);
        NEXUS_SimpleVideoDecoder_SetStcChannel(videoDecoder, stcChannel);

        NEXUS_SimpleVideoDecoder_SetStartPts(videoDecoder, target_pts);
        NEXUS_SimpleVideoDecoder_Start(videoDecoder, &videoProgram);
        NEXUS_Playback_Start(playback, file, NULL);

        BDBG_LOG(("  decoding to target PTS %x", target_pts));
        /* We poll to see when we've arrived. firstPts callback is not reliable for this. */
        do {
            NEXUS_SimpleVideoDecoder_GetStatus(videoDecoder, &videoDecoderStatus);
        } while (videoDecoderStatus.pts < target_pts);

        NEXUS_SimpleStcChannel_GetStc(stcChannel, &stc);
        BDBG_LOG(("Ready: PTS %x, STC %x", videoDecoderStatus.pts, stc));

        BDBG_LOG(("  Press ENTER to set stc to current PTS, which shows current picture"));
        getchar();

        /* TODO: without this sleep, and if you press ENTER earlier so the preceding getchar() takes no time,
        I sometimes will not see the picture even though decoder PTS == STC value. But a slight delay works. */
        BKNI_Sleep(30);

        /* To show: get current PTS from decoder, which may differ slightly from target_pts, then set the STC */
        NEXUS_SimpleVideoDecoder_GetStatus(videoDecoder, &videoDecoderStatus);
        NEXUS_SimpleStcChannel_SetStc(stcChannel, videoDecoderStatus.pts);

        NEXUS_SimpleStcChannel_GetStc(stcChannel, &stc);
        BDBG_LOG(("Set: PTS %x, STC %x", videoDecoderStatus.pts, stc));

        BDBG_LOG(("  Press ENTER to play"));
        getchar();

        /* To play:
        1) eAuto (so decoder will set the STC if we hit a discontinuity)
        2) unfreeze */
        NEXUS_SimpleStcChannel_GetSettings(stcChannel, &stcSettings);
        stcSettings.mode = NEXUS_StcChannelMode_eAuto;
        NEXUS_SimpleStcChannel_SetSettings(stcChannel, &stcSettings);
        NEXUS_SimpleStcChannel_Freeze(stcChannel, false);

        BDBG_LOG(("Go!"));
        BDBG_LOG(("  Press ENTER to repeat steps"));
        getchar();

        NEXUS_SimpleVideoDecoder_Stop(videoDecoder);
        NEXUS_Playback_Stop(playback);
        NEXUS_Playback_ClosePidChannel(playback, videoProgram.settings.pidChannel);
        NEXUS_FilePlay_Close(file);
    }

    NEXUS_Playback_Destroy(playback);
    NEXUS_Playpump_Close(playpump);
    NxClient_Disconnect(connectId);
    NxClient_Free(&allocResults);
    NxClient_Uninit();
    return 0;

#else /* #if NEXUS_HAS_PLAYBACK */
int main(void)
{
    printf("This application is not supported on this platform\n");
    return 0;
}
#endif
