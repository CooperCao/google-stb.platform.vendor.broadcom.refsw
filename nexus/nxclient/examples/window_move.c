/******************************************************************************
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
 *****************************************************************************/
#include "nxclient.h"
#include <stdio.h>
#if NEXUS_HAS_PLAYBACK && NEXUS_HAS_SIMPLE_DECODER
#include "nexus_platform_client.h"
#include "nexus_surface_client.h"
#include "nexus_core_utils.h"
#include "nxclient_global.h"
#include "nexus_graphics2d.h"
#include "nexus_playback.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <pthread.h>
#include "bstd.h"
#include "bkni.h"

BDBG_MODULE(window_move);

/* the following define the input file and its characteristics -- these will vary by input file */
#define FILE_NAME "videos/cnnticker.mpg"
#define TRANSPORT_TYPE NEXUS_TransportType_eTs
#define VIDEO_CODEC NEXUS_VideoCodec_eMpeg2
#define VIDEO_PID 0x21

static void complete(void *context, int param)
{
    BSTD_UNUSED(param);
    BKNI_SetEvent((BKNI_EventHandle)context);
}

static void checkpoint(NEXUS_Graphics2DHandle gfx, BKNI_EventHandle checkpointEvent)
{
    int rc = NEXUS_Graphics2D_Checkpoint(gfx, NULL); /* require to execute queue */
    if (rc == NEXUS_GRAPHICS2D_QUEUED) {
        rc = BKNI_WaitForEvent(checkpointEvent, BKNI_INFINITE);
    }
    BDBG_ASSERT(!rc);
}

#include <sys/time.h>
static unsigned b_get_time(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec*1000 + tv.tv_usec/1000;
}

int main(int argc, const char **argv)
{
    NEXUS_Error rc = 0;
    int curarg = 1;
    NxClient_JoinSettings joinSettings;
    NxClient_AllocSettings allocSettings;
    NxClient_AllocResults allocResults;
    NxClient_ConnectSettings connectSettings;
    unsigned connectId;
    NxClient_DisplaySettings displaySettings;
    int inc = 10;
    NEXUS_SurfaceClientHandle video_sc = NULL;
    NEXUS_SurfaceClientHandle surfaceClient;
    NEXUS_SurfaceClientSettings surfaceClientSettings;
    NEXUS_Graphics2DHandle gfx;
    struct {
        NEXUS_SurfaceHandle handle;
        bool pushed;
    } surface[5]; /* 3 works if there are no extra prints to the console. with even slight delays, we need more. */
#define NUM_SURFACES (sizeof(surface)/sizeof(surface[0]))
    unsigned cur_surface;
    BKNI_EventHandle displayedEvent, recycledEvent, checkpointEvent, windowMovedEvent;
    NEXUS_Graphics2DSettings gfxSettings;
    NEXUS_SurfaceCreateSettings createSettings;
    NEXUS_Graphics2DFillSettings fillSettings;
    NEXUS_PlaypumpHandle playpump;
    NEXUS_PlaybackHandle playback;
    NEXUS_PlaybackSettings playbackSettings;
    NEXUS_PlaybackPidChannelSettings playbackPidSettings;
    NEXUS_FilePlayHandle file;
    NEXUS_SimpleVideoDecoderHandle videoDecoder;
    NEXUS_SimpleVideoDecoderStartSettings videoProgram;
    NEXUS_SimpleStcChannelHandle stcChannel;
    unsigned count = 0, starttime;
    bool graphics = true;
    bool window_move = true;
    bool push = false;
    bool resize = false;
    unsigned graphics_lag = 0;
    NEXUS_Rect pos_queue[3];
#define NUM_POS (sizeof(pos_queue)/sizeof(pos_queue[0]))
    unsigned cur_pos = 0;

    memset(pos_queue, 0, sizeof(pos_queue));
    while (argc > curarg) {
        if (!strcmp(argv[curarg],"-no_graphics")) {
            graphics = false;
        }
        else if (!strcmp(argv[curarg],"-no_window_move")) {
            window_move = false;
        }
        else if (!strcmp(argv[curarg],"-push")) {
            push = true;
        }
        else if (!strcmp(argv[curarg],"-resize")) {
            resize = true;
        }
        else if (!strcmp(argv[curarg],"-graphics_lag") && curarg+1 < argc) {
            graphics_lag = atoi(argv[++curarg]);
            if (graphics_lag >= NUM_POS) {
                fprintf(stderr, "max graphics_lag is %u\n", NUM_POS-1);
                return -1;
            }
        }
        else if (!strcmp(argv[curarg],"-inc")) {
            inc = atoi(argv[++curarg]);
        }
        curarg++;
    }

    NxClient_GetDefaultJoinSettings(&joinSettings);
    snprintf(joinSettings.name, NXCLIENT_MAX_NAME, "%s", argv[0]);
    rc = NxClient_Join(&joinSettings);
    if (rc) return -1;

    BKNI_CreateEvent(&displayedEvent);
    BKNI_CreateEvent(&recycledEvent);
    BKNI_CreateEvent(&windowMovedEvent);
    BKNI_CreateEvent(&checkpointEvent);

    /* make GFD translucent so we can see video/graphics overlap */
    NxClient_GetDisplaySettings(&displaySettings);
    displaySettings.graphicsSettings.alpha = 0x80;
    NxClient_SetDisplaySettings(&displaySettings);

    gfx = NEXUS_Graphics2D_Open(NEXUS_ANY_ID, NULL);
    NEXUS_Graphics2D_GetSettings(gfx, &gfxSettings);
    gfxSettings.checkpointCallback.callback = complete;
    gfxSettings.checkpointCallback.context = checkpointEvent;
    rc = NEXUS_Graphics2D_SetSettings(gfx, &gfxSettings);
    BDBG_ASSERT(!rc);

    NEXUS_Surface_GetDefaultCreateSettings(&createSettings);
    createSettings.pixelFormat = NEXUS_PixelFormat_eA8_R8_G8_B8;
    createSettings.width = 720;
    createSettings.height = 480;
    for (cur_surface=0;cur_surface<(push?NUM_SURFACES:1);cur_surface++) {
        surface[cur_surface].handle = NEXUS_Surface_Create(&createSettings);
        surface[cur_surface].pushed = false;
    }
    cur_surface = 0;

    NxClient_GetDefaultAllocSettings(&allocSettings);
    allocSettings.surfaceClient = 1;
    allocSettings.simpleVideoDecoder = 1;
    rc = NxClient_Alloc(&allocSettings, &allocResults);
    if (rc) BERR_TRACE(rc);

    file = NEXUS_FilePlay_OpenPosix(FILE_NAME, NULL);
    playpump = NEXUS_Playpump_Open(NEXUS_ANY_ID, NULL);
    playback = NEXUS_Playback_Create();
    stcChannel = NEXUS_SimpleStcChannel_Create(NULL);

    NEXUS_Playback_GetSettings(playback, &playbackSettings);
    playbackSettings.playpump = playpump;
    playbackSettings.simpleStcChannel = stcChannel;
    playbackSettings.playpumpSettings.transportType = TRANSPORT_TYPE;
    rc = NEXUS_Playback_SetSettings(playback, &playbackSettings);
    BDBG_ASSERT(!rc);

    videoDecoder = NEXUS_SimpleVideoDecoder_Acquire(allocResults.simpleVideoDecoder[0].id);

    NxClient_GetDefaultConnectSettings(&connectSettings);
    connectSettings.simpleVideoDecoder[0].id = allocResults.simpleVideoDecoder[0].id;
    connectSettings.simpleVideoDecoder[0].surfaceClientId = allocResults.surfaceClient[0].id;
    rc = NxClient_Connect(&connectSettings, &connectId);
    BDBG_ASSERT(!rc);

    NEXUS_SimpleVideoDecoder_GetDefaultStartSettings(&videoProgram);

    NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
    playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eVideo;
    playbackPidSettings.pidTypeSettings.video.codec = VIDEO_CODEC;
    playbackPidSettings.pidTypeSettings.video.index = true;
    playbackPidSettings.pidTypeSettings.video.simpleDecoder = videoDecoder;
    videoProgram.settings.pidChannel = NEXUS_Playback_OpenPidChannel(playback, VIDEO_PID, &playbackPidSettings);
    videoProgram.settings.codec = VIDEO_CODEC;

    surfaceClient = NEXUS_SurfaceClient_Acquire(allocResults.surfaceClient[0].id);
    NEXUS_SurfaceClient_GetSettings(surfaceClient, &surfaceClientSettings);
    surfaceClientSettings.displayed.callback = complete;
    surfaceClientSettings.displayed.context = displayedEvent;
    surfaceClientSettings.recycled.callback = complete;
    surfaceClientSettings.recycled.context = recycledEvent;
    surfaceClientSettings.windowMoved.callback = complete;
    surfaceClientSettings.windowMoved.context = windowMovedEvent;
    NEXUS_SurfaceClient_SetSettings(surfaceClient, &surfaceClientSettings);
    video_sc = NEXUS_SurfaceClient_AcquireVideoWindow(surfaceClient, 0);

    rc = NEXUS_SimpleVideoDecoder_SetStcChannel(videoDecoder, stcChannel);
    BDBG_ASSERT(!rc);
    rc = NEXUS_SimpleVideoDecoder_Start(videoDecoder, &videoProgram);
    BDBG_ASSERT(!rc);
    rc = NEXUS_Playback_Start(playback, file, NULL);
    BDBG_ASSERT(!rc);

    NEXUS_SurfaceClient_GetSettings(video_sc, &surfaceClientSettings);
    if (resize) {
        surfaceClientSettings.composition.position.x = 100;
        surfaceClientSettings.composition.position.y = 100;
    }
    else {
        surfaceClientSettings.composition.position.x = 0;
        surfaceClientSettings.composition.position.y = 100;
    }
    surfaceClientSettings.composition.position.width = 400;
    surfaceClientSettings.composition.position.height = 400/16*9;
    surfaceClientSettings.composition.contentMode = NEXUS_VideoWindowContentMode_eFull;
    NEXUS_SurfaceClient_SetSettings(video_sc, &surfaceClientSettings);

    starttime = b_get_time();
    /* assume default 1920,1080 virtual display */
    while (1) {
        if (resize) {
            if (surfaceClientSettings.composition.position.y + surfaceClientSettings.composition.position.height + inc > 1080 ||
                surfaceClientSettings.composition.position.y + inc < 0) {
                inc *= -1;
            }
            surfaceClientSettings.composition.position.width += inc;
            surfaceClientSettings.composition.position.height += inc;
        }
        else {
            if (surfaceClientSettings.composition.position.x + surfaceClientSettings.composition.position.width + inc > 1920 ||
                surfaceClientSettings.composition.position.x + inc < 0) {
                inc *= -1;
            }
            surfaceClientSettings.composition.position.x += inc;
        }
        pos_queue[cur_pos] = surfaceClientSettings.composition.position;

        if (graphics) {
            unsigned pos;
            const NEXUS_Rect *rect;
            if (push) {
                for (cur_surface=0;cur_surface<NUM_SURFACES;cur_surface++) {
                    if (!surface[cur_surface].pushed) break;
                }
                BDBG_ASSERT(cur_surface<NUM_SURFACES);
            }
            NEXUS_Graphics2D_GetDefaultFillSettings(&fillSettings);
            fillSettings.surface = surface[cur_surface].handle;
            fillSettings.color = 0xFFFFFFFF;
            rc = NEXUS_Graphics2D_Fill(gfx, &fillSettings);
            BDBG_ASSERT(!rc);
            fillSettings.color = 0x0;
            /* convert from virtual display 1920x1080 to surface 720x480 */
            pos = cur_pos + NUM_POS - graphics_lag;
            pos = pos % NUM_POS;
            if (pos_queue[pos].width == 0) pos = 0;
            rect = &pos_queue[pos];
            fillSettings.rect.x = rect->x*720/1920;
            fillSettings.rect.y = rect->y*480/1080;
            fillSettings.rect.width = rect->width*720/1920;
            fillSettings.rect.height = rect->height*480/1080;
            rc = NEXUS_Graphics2D_Fill(gfx, &fillSettings);
            BDBG_ASSERT(!rc);

            checkpoint(gfx, checkpointEvent);

            if (push) {
                if (count > 1) {
                    while (1) {
                        size_t n, i;
                        NEXUS_SurfaceHandle returned;
                        rc = NEXUS_SurfaceClient_RecycleSurface(surfaceClient, &returned, 1, &n);
                        BDBG_ASSERT(!rc);
                        if (!n) break;
                        for (i=0;surface[i].handle != returned;i++);
                        BDBG_ASSERT(i<NUM_SURFACES);
                        BDBG_ASSERT(surface[i].pushed);
                        surface[i].pushed = false;
                        BDBG_MSG(("recycle %u", i));
                    }
                }
                BDBG_MSG(("push %u", cur_surface));
                rc = NEXUS_SurfaceClient_PushSurface(surfaceClient, surface[cur_surface].handle, NULL, window_move);
                BDBG_ASSERT(!rc);
                surface[cur_surface].pushed = true;
            }
            else {
                rc = NEXUS_SurfaceClient_SetSurface(surfaceClient, surface[cur_surface].handle);
                BDBG_ASSERT(!rc);
            }
        }

        if (window_move) {
            NEXUS_SurfaceClient_SetSettings(video_sc, &surfaceClientSettings);
        }

        if (window_move && (!graphics || push)) {
            rc = BKNI_WaitForEvent(windowMovedEvent, 5000);
            BDBG_ASSERT(!rc);
        }
        else {
            if (!push) {
                rc = BKNI_WaitForEvent(displayedEvent, 5000);
                BDBG_ASSERT(!rc);
            }
        }

        if (++count % 100 == 0) {
            /* this drives need for more than 3 buffers */
            BDBG_LOG(("%u fps", count * 1000 / ((b_get_time() - starttime))));
            count = 0;
            starttime = b_get_time();
        }
        if (++cur_pos == NUM_POS) cur_pos = 0;
    }

    return 0;
}
#else /* #if NEXUS_HAS_PLAYBACK && NEXUS_HAS_SIMPLE_DECODER */
int main(void)
{
    printf("This application is not supported on this platform\n");
    return 0;
}
#endif
