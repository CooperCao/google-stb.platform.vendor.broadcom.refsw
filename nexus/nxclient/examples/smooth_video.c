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
#include "nexus_core_utils.h"
#include "nexus_surface_client.h"
#include "bstd.h"
#include "bkni.h"
#include "bkni_multi.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

BDBG_MODULE(smooth_video);

static struct {
    const char *filename;
    NEXUS_VideoCodec codec;
    unsigned pid;
} g_streams[] = {
    {"videos/cnnticker.mpg", NEXUS_VideoCodec_eMpeg2, 0x21},
    {"videos/japan480p.mpg", NEXUS_VideoCodec_eMpeg2, 0x11}
};

static void print_usage(void)
{
    printf(
    "Usage: smooth_video OPTIONS\n"
    "-inc X       - set x/y increment. default is 1.\n"
    "-resize      - test smooth scaling instead of smooth movement\n"
    "\n"
    "Suboptimal test options:\n"
    "-no_graphics - don't use graphics for video flow control\n"
    "-set         - use less optimal SetSurface\n"
    );
}

static void complete2(void *context, int param)
{
    BSTD_UNUSED(param);
    BKNI_SetEvent((BKNI_EventHandle)context);
}

#include <sys/time.h>
static unsigned b_get_time(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec*1000 + tv.tv_usec/1000;
}

#define START_BLOCK_CHECK(PSTARTTIME) \
    do { starttime = b_get_time(); } while(0)
#define END_BLOCK_CHECK(FUNCTION,STARTTIME,MAX) \
    do { unsigned endtime = b_get_time(); if (endtime - (STARTTIME) > (MAX)) { \
        BDBG_WRN(("blocking %s: %d", #FUNCTION, endtime - (STARTTIME))); } } while(0)

int main(int argc, char **argv)
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
    NEXUS_SurfaceClientHandle surfaceClient, videoSurfaceClient;
    int x_dir = 1, y_dir = 1;
    int curarg = 1;
    bool graphics = true;
    bool push = true;
    struct {
        unsigned cnt, time;
    } fps = {0,0};
#define MAX_SURFACES 3
    NEXUS_SurfaceHandle surface[MAX_SURFACES];
    unsigned next_surface = 0;
    unsigned total = 0;
    BKNI_EventHandle nscEvent;
    NEXUS_SurfaceClientSettings settings;
    bool resize = false;
    unsigned video = 0;
    unsigned msPerVsync;
    unsigned starttime;
    unsigned i;

    memset(surface, 0, sizeof(surface));

    while (curarg < argc) {
        if (!strcmp(argv[curarg], "-h") || !strcmp(argv[curarg], "--help")) {
            print_usage();
            return 0;
        }
        else if (!strcmp(argv[curarg], "-inc") && curarg+1<argc) {
            x_dir = y_dir = atoi(argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-no_graphics")) {
            graphics = false;
        }
        else if (!strcmp(argv[curarg], "-set")) {
            push = false;
        }
        else if (!strcmp(argv[curarg], "-resize")) {
            resize = true;
        }
        else if (!strcmp(argv[curarg], "-video") && curarg+1<argc) {
            video = atoi(argv[++curarg]);
        }
        else {
            print_usage();
            return -1;
        }
        curarg++;
    }

    rc = NxClient_Join(NULL);
    if (rc) return -1;

    NxClient_GetDefaultAllocSettings(&allocSettings);
    allocSettings.simpleVideoDecoder = 1;
    allocSettings.surfaceClient = 1;
    rc = NxClient_Alloc(&allocSettings, &allocResults);
    if (rc) return BERR_TRACE(rc);

    file = NEXUS_FilePlay_OpenPosix(g_streams[video].filename, NULL);

    NEXUS_Playpump_GetDefaultOpenSettings(&playpumpOpenSettings);
    playpump = NEXUS_Playpump_Open(NEXUS_ANY_ID, &playpumpOpenSettings);
    playback = NEXUS_Playback_Create();
    BDBG_ASSERT(playback);
    stcChannel = NEXUS_SimpleStcChannel_Create(NULL);

    NEXUS_Playback_GetSettings(playback, &playbackSettings);
    playbackSettings.playpump = playpump;
    playbackSettings.simpleStcChannel = stcChannel;
    playbackSettings.stcTrick = true;
    rc = NEXUS_Playback_SetSettings(playback, &playbackSettings);
    BDBG_ASSERT(!rc);

    videoDecoder = NEXUS_SimpleVideoDecoder_Acquire(allocResults.simpleVideoDecoder[0].id);

    surfaceClient = NEXUS_SurfaceClient_Acquire(allocResults.surfaceClient[0].id);
    videoSurfaceClient = NEXUS_SurfaceClient_AcquireVideoWindow(surfaceClient, 0);

    if (graphics) {
        /* push an alpha 0 surface */
        NEXUS_SurfaceCreateSettings surfaceCreateSettings;
        NEXUS_SurfaceMemory mem;
        NEXUS_Surface_GetDefaultCreateSettings(&surfaceCreateSettings);
        surfaceCreateSettings.width = 100;
        surfaceCreateSettings.height = 100;
        for (i=0;i<MAX_SURFACES;i++) {
            surface[i] = NEXUS_Surface_Create(&surfaceCreateSettings);
            NEXUS_Surface_GetMemory(surface[i], &mem);
            BKNI_Memset(mem.buffer, 0, surfaceCreateSettings.height * mem.pitch);
            NEXUS_Surface_Flush(surface[i]);
        }

        BKNI_CreateEvent(&nscEvent);
        NEXUS_SurfaceClient_GetSettings(surfaceClient, &settings);
        if (push) {
            settings.recycled.callback = complete2;
            settings.recycled.context = nscEvent;
        }
        else {
            settings.displayed.callback = complete2;
            settings.displayed.context = nscEvent;
        }
        NEXUS_SurfaceClient_SetSettings(surfaceClient, &settings);
    }

    NxClient_GetDefaultConnectSettings(&connectSettings);
    connectSettings.simpleVideoDecoder[0].id = allocResults.simpleVideoDecoder[0].id;
    connectSettings.simpleVideoDecoder[0].surfaceClientId = allocResults.surfaceClient[0].id;
    rc = NxClient_Connect(&connectSettings, &connectId);
    if (rc) return BERR_TRACE(rc);

    NEXUS_SimpleVideoDecoder_GetDefaultStartSettings(&videoProgram);

    NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
    playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eVideo;
    playbackPidSettings.pidTypeSettings.video.codec = g_streams[video].codec;
    playbackPidSettings.pidTypeSettings.video.index = true;
    playbackPidSettings.pidTypeSettings.video.simpleDecoder = videoDecoder;
    videoProgram.settings.pidChannel = NEXUS_Playback_OpenPidChannel(playback, g_streams[video].pid, &playbackPidSettings);
    videoProgram.settings.codec = g_streams[video].codec;

    if (videoProgram.settings.pidChannel) {
        NEXUS_SimpleVideoDecoder_SetStcChannel(videoDecoder, stcChannel);
    }
    if (videoProgram.settings.pidChannel) {
        NEXUS_SimpleVideoDecoder_Start(videoDecoder, &videoProgram);
    }
    NEXUS_Playback_Start(playback, file, NULL);

    {
    NxClient_DisplaySettings displaySettings;
    NEXUS_VideoFormatInfo videoInfo;
    NxClient_GetDisplaySettings(&displaySettings);
    NEXUS_VideoFormat_GetInfo(displaySettings.format, &videoInfo);
    msPerVsync = 1000 * 100 / videoInfo.verticalFreq;
    msPerVsync = msPerVsync * 4 / 3; /* give 33% fudge. on 16 msec system, we can see occasional 20 msec delay but still hit 60fps. */
    }

    /* To get 60fps graphics and video changes, we must do flow control with PushSurface graphics.
    Update video first (which should be non-blocking, provided another video update is not pending).
    Then update graphics and wait for it to complete. With PushSurface, the graphics update will be 60fps,
    even with other clients running, and video will get applied with no extra blocking.

    Run this app with no options to get this mode.

    Things to avoid:

    If graphics is not used for flow control (-no_graphics), the second video update will block inside VDC. And because the whole
    state machine waits, other clients will slow down.

    Also, if SetSurface is used for graphics (-set), it can do 60fps on its own, but with another graphics client it
    will slow down to 30fps. The other PushSurface client will be driving its own composition while this SetSurface
    client waits, so it gets left out of half of the compositions.
    */

    BDBG_WRN(("start"));
    while (1) {
        NEXUS_SurfaceClient_GetSettings(videoSurfaceClient, &settings);
        settings.composition.contentMode = NEXUS_VideoWindowContentMode_eFull;
        settings.composition.virtualDisplay.width = 1920;
        settings.composition.virtualDisplay.height = 1080;
        if (resize) {
            if (settings.composition.position.width == 1920) {
                settings.composition.position.width = 960;
                settings.composition.position.height = 540;
            }
            else {
                settings.composition.position.width += x_dir;
                settings.composition.position.height += y_dir;
            }
        }
        else {
            settings.composition.position.width = 960;
            settings.composition.position.height = 540;
            settings.composition.position.x += x_dir;
            settings.composition.position.y += y_dir;
        }

        START_BLOCK_CHECK(&starttime);
        NEXUS_SurfaceClient_SetSettings(videoSurfaceClient, &settings);
        END_BLOCK_CHECK(NEXUS_SurfaceClient_SetSettings, starttime, 5);
        if (graphics) {
            if (push) {
                if (total > 1) {
                    NEXUS_SurfaceHandle recycle[MAX_SURFACES];
                    size_t num;

                    rc = NEXUS_SurfaceClient_RecycleSurface(surfaceClient, recycle, MAX_SURFACES, &num);
                    BDBG_ASSERT(!rc);

                    START_BLOCK_CHECK(&starttime);
                    rc = BKNI_WaitForEvent(nscEvent, 5000);
                    BDBG_ASSERT(!rc);
                    END_BLOCK_CHECK(BKNI_WaitForEvent, starttime, msPerVsync);
                }
                START_BLOCK_CHECK(&starttime);
                rc = NEXUS_SurfaceClient_PushSurface(surfaceClient, surface[next_surface], NULL, false);
                BDBG_ASSERT(!rc);
                END_BLOCK_CHECK(NEXUS_SurfaceClient_UpdateSurface, starttime, 5);
                if (++next_surface == MAX_SURFACES) {
                    next_surface = 0;
                }
            }
            else {
                if (total) {
                    START_BLOCK_CHECK(&starttime);
                    rc = BKNI_WaitForEvent(nscEvent, 5000);
                    BDBG_ASSERT(!rc);
                    END_BLOCK_CHECK(BKNI_WaitForEvent, starttime, msPerVsync);
                }
                START_BLOCK_CHECK(&starttime);
                rc = NEXUS_SurfaceClient_SetSurface(surfaceClient, surface[0]);
                BDBG_ASSERT(!rc);
                END_BLOCK_CHECK(NEXUS_SurfaceClient_SetSurface, starttime, 5);
            }
            total++;
        }
        if (resize) {
            if (settings.composition.position.height >= 1000 || settings.composition.position.height < 200) {
                x_dir *= -1;
                y_dir *= -1;
            }
        }
        else {
            if (settings.composition.position.x + settings.composition.position.width >= 1920 ||
                settings.composition.position.x <= 0) x_dir *= -1;
            if (settings.composition.position.y + settings.composition.position.height >= 1080 ||
                settings.composition.position.y <= 0) y_dir *= -1;
        }

        if (++fps.cnt % 100 == 0) {
            unsigned now = b_get_time();
            if (fps.time) {
                BDBG_WRN(("%d fps", 1000*fps.cnt/(now-fps.time)));
            }
            fps.cnt = 0;
            fps.time = now;
        }
    }

    NEXUS_Playback_Stop(playback);
    NEXUS_Playback_Destroy(playback);
    NEXUS_Playpump_Close(playpump);
    NEXUS_FilePlay_Close(file);
    NxClient_Disconnect(connectId);
    NxClient_Free(&allocResults);
    for (i=0;i<MAX_SURFACES;i++) {
        if (surface[i]) {
            NEXUS_Surface_Destroy(surface[i]);
        }
    }
    NxClient_Uninit();
    return 0;
}
