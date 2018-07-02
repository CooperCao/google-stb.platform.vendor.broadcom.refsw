/***************************************************************************
 * Copyright (C) 2017-2018 Broadcom.  The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
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
#if NEXHS_HAS_PLAYBACL
#include "nexus_playback.h"
#include "nexus_surface_client.h"
#include "bstd.h"
#include "bkni.h"
#include "bkni_multi.h"
#include <stdlib.h>
#include <string.h>

BDBG_MODULE(decode_source_clip);

#define FILE_NAME "videos/format_change.mpg"
#define TRANSPORT_TYPE NEXUS_TransportType_eTs
#define VIDEO_CODEC NEXUS_VideoCodec_eMpeg2
#define VIDEO_PID 0x11

static void source_changed_callback(void *context, int param)
{
    BSTD_UNUSED(param);
    BDBG_LOG(("source changed"));
    BKNI_SetEvent((BKNI_EventHandle)context);
}

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
    NEXUS_VideoDecoderSettings videoDecoderSettings;
    NEXUS_SimpleStcChannelHandle stcChannel;
    NEXUS_Error rc;
    NEXUS_SurfaceClientHandle surfaceClient, videoSurfaceClient;
    NEXUS_SurfaceClientSettings surfaceClientSettings;
    BKNI_EventHandle sourceChangedEvent;

    rc = NxClient_Join(NULL);
    if (rc) return -1;

    NxClient_GetDefaultAllocSettings(&allocSettings);
    allocSettings.simpleVideoDecoder = 1;
    allocSettings.surfaceClient = 1;
    rc = NxClient_Alloc(&allocSettings, &allocResults);
    if (rc) return BERR_TRACE(rc);

    BKNI_CreateEvent(&sourceChangedEvent);
    file = NEXUS_FilePlay_OpenPosix(FILE_NAME, NULL);

    NEXUS_Playpump_GetDefaultOpenSettings(&playpumpOpenSettings);
    playpump = NEXUS_Playpump_Open(NEXUS_ANY_ID, &playpumpOpenSettings);
    playback = NEXUS_Playback_Create();
    BDBG_ASSERT(playback);
    stcChannel = NEXUS_SimpleStcChannel_Create(NULL);

    NEXUS_Playback_GetSettings(playback, &playbackSettings);
    playbackSettings.playpump = playpump;
    playbackSettings.simpleStcChannel = stcChannel;
    playbackSettings.playpumpSettings.transportType = TRANSPORT_TYPE;
    rc = NEXUS_Playback_SetSettings(playback, &playbackSettings);
    BDBG_ASSERT(!rc);

    videoDecoder = NEXUS_SimpleVideoDecoder_Acquire(allocResults.simpleVideoDecoder[0].id);

    surfaceClient = NEXUS_SurfaceClient_Acquire(allocResults.surfaceClient[0].id);
    videoSurfaceClient = NEXUS_SurfaceClient_AcquireVideoWindow(surfaceClient, 0);

    NxClient_GetDefaultConnectSettings(&connectSettings);
    connectSettings.simpleVideoDecoder[0].id = allocResults.simpleVideoDecoder[0].id;
    connectSettings.simpleVideoDecoder[0].surfaceClientId = allocResults.surfaceClient[0].id;
    rc = NxClient_Connect(&connectSettings, &connectId);
    if (rc) return BERR_TRACE(rc);

    NEXUS_SimpleVideoDecoder_GetDefaultStartSettings(&videoProgram);

    NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
    playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eVideo;
    playbackPidSettings.pidTypeSettings.video.codec = VIDEO_CODEC;
    playbackPidSettings.pidTypeSettings.video.index = true;
    playbackPidSettings.pidTypeSettings.video.simpleDecoder = videoDecoder;
    videoProgram.settings.pidChannel = NEXUS_Playback_OpenPidChannel(playback, VIDEO_PID, &playbackPidSettings);
    videoProgram.settings.codec = VIDEO_CODEC;

    NEXUS_SimpleVideoDecoder_SetStcChannel(videoDecoder, stcChannel);
    NEXUS_SimpleVideoDecoder_Start(videoDecoder, &videoProgram);

    BDBG_LOG(("starting normal decode"));
    NEXUS_Playback_Start(playback, file, NULL);
    getchar();

    /* source clipping requires no auto A/R correction, so we switch to this first. If you need it, you'll
    have to do manual A/R correction by resizing the window. */
    BDBG_LOG(("full a/r"));
    NEXUS_SurfaceClient_GetSettings(videoSurfaceClient, &surfaceClientSettings);
    surfaceClientSettings.composition.contentMode = NEXUS_VideoWindowContentMode_eFull;
    NEXUS_SurfaceClient_SetSettings(videoSurfaceClient, &surfaceClientSettings);
    getchar();

    BDBG_LOG(("source clip (10 lines from top and bottom if source is SD)"));
    NEXUS_SurfaceClient_GetSettings(videoSurfaceClient, &surfaceClientSettings);
    surfaceClientSettings.composition.clipRect.x = 0;
    surfaceClientSettings.composition.clipRect.y = 10;
    surfaceClientSettings.composition.clipRect.width = 720;
    surfaceClientSettings.composition.clipRect.height = 460;
    surfaceClientSettings.composition.clipBase.width = 720;
    surfaceClientSettings.composition.clipBase.height = 480;
    NEXUS_SurfaceClient_SetSettings(videoSurfaceClient, &surfaceClientSettings);
    getchar();

    /* finally, how fast can we change source clipping in response to a sourceChanged callback?
    This requires a stream which changes source format. */
    NEXUS_SimpleVideoDecoder_GetSettings(videoDecoder, &videoDecoderSettings);
    videoDecoderSettings.sourceChanged.callback = source_changed_callback;
    videoDecoderSettings.sourceChanged.context = sourceChangedEvent;
    NEXUS_SimpleVideoDecoder_SetSettings(videoDecoder, &videoDecoderSettings);

    while (1) {
        NEXUS_VideoDecoderStatus status;
        unsigned clip;
        BDBG_LOG(("wait for source changed"));
        BKNI_WaitForEvent(sourceChangedEvent, BKNI_INFINITE);

        NEXUS_SimpleVideoDecoder_GetStatus(videoDecoder, &status);
        NEXUS_SurfaceClient_GetSettings(videoSurfaceClient, &surfaceClientSettings);
        surfaceClientSettings.composition.clipBase.width = status.source.width;
        surfaceClientSettings.composition.clipBase.height = status.source.height;
        /* clip 2 lines from top and bottom of SD source, not HD source */
        clip = (status.source.height <= 576) ? 2 : 0;
        surfaceClientSettings.composition.clipRect.x = 0;
        surfaceClientSettings.composition.clipRect.y = clip;
        surfaceClientSettings.composition.clipRect.width = status.source.width;
        surfaceClientSettings.composition.clipRect.height = status.source.height - clip*2;
        NEXUS_SurfaceClient_SetSettings(videoSurfaceClient, &surfaceClientSettings);
        BDBG_LOG(("set source clip to %u,%u -> %u,%u,%u,%u",
            status.source.width, status.source.height,
            surfaceClientSettings.composition.clipRect.x, surfaceClientSettings.composition.clipRect.y,
            surfaceClientSettings.composition.clipRect.width, surfaceClientSettings.composition.clipRect.height));
    }

    NEXUS_Playback_Stop(playback);
    NEXUS_Playback_Destroy(playback);
    NEXUS_Playpump_Close(playpump);
    NEXUS_FilePlay_Close(file);
    NxClient_Disconnect(connectId);
    BKNI_DestroyEvent(sourceChangedEvent);
    NxClient_Free(&allocResults);
    NxClient_Uninit();
    return 0;
}
#else /* #if NEXUS_HAS_PLAYBACK */
int main(void)
{
    printf("This application is not supported on this platform\n");
    return 0;
}
#endif
