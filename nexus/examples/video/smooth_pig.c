/******************************************************************************
 * Copyright (C) 2019 Broadcom.
 * The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to
 * the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied),
 * right to use, or waiver of any kind with respect to the Software, and
 * Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 * THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 * IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use all
 * reasonable efforts to protect the confidentiality thereof, and to use this
 * information only in connection with your use of Broadcom integrated circuit
 * products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 * OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 * RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 * IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 * A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 * ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 * THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 * OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 * INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 * RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 * HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 * EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 * WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 * FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
******************************************************************************/
#if NEXUS_HAS_DISPLAY && NEXUS_HAS_GRAPHICS2D
#include "nexus_platform.h"
#include "nexus_video_decoder.h"
#include "nexus_stc_channel.h"
#include "nexus_display.h"
#include "nexus_video_window.h"
#include "nexus_video_input.h"
#include "nexus_component_output.h"
#if NEXUS_HAS_HDMI_OUTPUT
#include "nexus_hdmi_output.h"
#endif
#if NEXUS_HAS_PLAYBACK
#include "nexus_playback.h"
#include "nexus_file.h"
#endif
#include "nexus_core_utils.h"
#include "nexus_graphics2d.h"

#include <assert.h>
#include "bstd.h"
#include "bkni.h"
#include <stdio.h>
#include <sys/time.h>
#include <stdlib.h>

BDBG_MODULE(smooth_pig);

#if 0
/* cnnticker is not good for this test because the video has black boundaries, so it's harder to see a gap between graphics and video */
#define FILE_NAME "videos/cnnticker.mpg"
#define TRANSPORT_TYPE NEXUS_TransportType_eTs
#define VIDEO_CODEC NEXUS_VideoCodec_eMpeg2
#define VIDEO_PID 0x21
#else
#define FILE_NAME "videos/japan480p.mpg"
#define TRANSPORT_TYPE NEXUS_TransportType_eTs
#define VIDEO_CODEC NEXUS_VideoCodec_eMpeg2
#define VIDEO_PID 0x11
#endif

void complete(void *data, int unused)
{
    BSTD_UNUSED(unused);
    BKNI_SetEvent((BKNI_EventHandle)data);
}

int main(void)
{
    NEXUS_StcChannelHandle stcChannel;
    NEXUS_PidChannelHandle videoPidChannel;
    NEXUS_VideoDecoderHandle videoDecoder;
    NEXUS_FilePlayHandle file;
    NEXUS_PlaypumpHandle playpump;
    NEXUS_PlaybackHandle playback;
    NEXUS_PlaybackSettings playbackSettings;
    NEXUS_PlaybackPidChannelSettings playbackPidSettings;
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_HdmiOutputStatus hdmiStatus;
#endif
    const char *fname = FILE_NAME;
    NEXUS_VideoDecoderStartSettings videoProgram;
    NEXUS_StcChannelSettings stcSettings;
    NEXUS_DisplayHandle display;
    NEXUS_VideoWindowHandle window;
    NEXUS_VideoWindowSettings windowSettings;
    NEXUS_PlatformSettings platformSettings;
    NEXUS_MemoryConfigurationSettings memConfigSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_DisplaySettings displaySettings;
    NEXUS_VideoFormatInfo formatInfo;
#define MAX_FRAMEBUFFERS 6
    NEXUS_SurfaceHandle framebuffer[MAX_FRAMEBUFFERS];
    NEXUS_SurfaceCreateSettings createSurfaceSettings;
    NEXUS_GraphicsSettings graphicsSettings;
    NEXUS_Graphics2DHandle gfx;
    NEXUS_Graphics2DSettings gfxSettings;
    NEXUS_Graphics2DFillSettings fillSettings;
    NEXUS_Error rc;
    unsigned cur = 0;
    unsigned cnt = 0;
    int incW;
    const unsigned smallest_width = 200;
    const unsigned animationTime = 2;
    const unsigned FPS = 60;
    BKNI_EventHandle checkpointEvent, framebufferEvent;
    struct timeval start_timeval;
    unsigned i;

    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    NEXUS_GetDefaultMemoryConfigurationSettings(&memConfigSettings);
    platformSettings.openFrontend = false;
    memConfigSettings.display[0].window[0].mtg = false; /* MTG limits to 30 fps */
    rc = NEXUS_Platform_MemConfigInit(&platformSettings, &memConfigSettings);
    BDBG_ASSERT(!rc);
    NEXUS_Platform_GetConfiguration(&platformConfig);
    BKNI_CreateEvent(&checkpointEvent);
    BKNI_CreateEvent(&framebufferEvent);

    playpump = NEXUS_Playpump_Open(0, NULL);
    assert(playpump);
    playback = NEXUS_Playback_Create();
    assert(playback);

    file = NEXUS_FilePlay_OpenPosix(fname, NULL);
    if (!file) {
        fprintf(stderr, "can't open file:%s\n", fname);
        return -1;
    }

    NEXUS_StcChannel_GetDefaultSettings(0, &stcSettings);
    stcSettings.timebase = NEXUS_Timebase_e0;
    stcSettings.mode = NEXUS_StcChannelMode_eAuto;
    stcChannel = NEXUS_StcChannel_Open(0, &stcSettings);

    NEXUS_Playback_GetSettings(playback, &playbackSettings);
    playbackSettings.playpump = playpump;
    playbackSettings.playpumpSettings.transportType = TRANSPORT_TYPE;
    playbackSettings.stcChannel = stcChannel;
    NEXUS_Playback_SetSettings(playback, &playbackSettings);

    NEXUS_Display_GetDefaultSettings(&displaySettings);
    displaySettings.format = NEXUS_VideoFormat_e720p; /* we want 60 fps format */
    display = NEXUS_Display_Open(0, &displaySettings);
    window = NEXUS_VideoWindow_Open(display, 0);

#if NEXUS_NUM_COMPONENT_OUTPUTS
    NEXUS_Display_AddOutput(display, NEXUS_ComponentOutput_GetConnector(platformConfig.outputs.component[0]));
#endif
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_Display_AddOutput(display, NEXUS_HdmiOutput_GetVideoConnector(platformConfig.outputs.hdmi[0]));
    rc = NEXUS_HdmiOutput_GetStatus(platformConfig.outputs.hdmi[0], &hdmiStatus);
    if ( !rc && hdmiStatus.connected )
    {
        /* If current display format is not supported by monitor, switch to monitor's preferred format. 
           If other connected outputs do not support the preferred format, a harmless error will occur. */
        NEXUS_Display_GetSettings(display, &displaySettings);
        if ( !hdmiStatus.videoFormatSupported[displaySettings.format] ) {
            displaySettings.format = hdmiStatus.preferredVideoFormat;
            NEXUS_Display_SetSettings(display, &displaySettings);
        }
    }
#endif
    
    videoDecoder = NEXUS_VideoDecoder_Open(0, NULL);

    NEXUS_VideoFormat_GetInfo(displaySettings.format, &formatInfo);
    incW = (formatInfo.width - smallest_width) / FPS / animationTime ;

    NEXUS_Surface_GetDefaultCreateSettings(&createSurfaceSettings);
    createSurfaceSettings.pixelFormat = NEXUS_PixelFormat_eA8_R8_G8_B8;
    createSurfaceSettings.width = formatInfo.width;
    createSurfaceSettings.height = formatInfo.height;
    createSurfaceSettings.heap = NEXUS_Platform_GetFramebufferHeap(0);
    for (i=0;i<MAX_FRAMEBUFFERS;i++) {
        framebuffer[i] = NEXUS_Surface_Create(&createSurfaceSettings);
    }

    gfx = NEXUS_Graphics2D_Open(0, NULL);
    NEXUS_Graphics2D_GetSettings(gfx, &gfxSettings);
    gfxSettings.checkpointCallback.callback = complete;
    gfxSettings.checkpointCallback.context = checkpointEvent;
    NEXUS_Graphics2D_SetSettings(gfx, &gfxSettings);

    NEXUS_Display_GetGraphicsSettings(display, &graphicsSettings);
    graphicsSettings.enabled = true;
    graphicsSettings.clip.width = formatInfo.width;
    graphicsSettings.clip.height = formatInfo.height;
    graphicsSettings.alpha = 0x80; /* semi-transparent framebuffer shows graphics/video overlap */
    graphicsSettings.frameBufferCallback.callback = complete;
    graphicsSettings.frameBufferCallback.context = framebufferEvent;
    graphicsSettings.synchronized = true;
    rc = NEXUS_Display_SetGraphicsSettings(display, &graphicsSettings);
    BDBG_ASSERT(!rc);
    
    NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
    playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eVideo;
    playbackPidSettings.pidTypeSettings.video.codec = VIDEO_CODEC;
    playbackPidSettings.pidTypeSettings.video.index = true;
    playbackPidSettings.pidTypeSettings.video.decoder = videoDecoder;
    videoPidChannel = NEXUS_Playback_OpenPidChannel(playback, VIDEO_PID, &playbackPidSettings);

    NEXUS_VideoDecoder_GetDefaultStartSettings(&videoProgram);
    videoProgram.codec = VIDEO_CODEC;
    videoProgram.pidChannel = videoPidChannel;
    videoProgram.stcChannel = stcChannel;

    NEXUS_VideoWindow_GetSettings(window, &windowSettings);
    windowSettings.position.x = 0;
    windowSettings.position.y = 0;
    windowSettings.position.width = smallest_width;
    windowSettings.position.height = smallest_width * 9 / 16;
    windowSettings.alpha = 0xFF;
    rc = NEXUS_VideoWindow_SetSettings(window, &windowSettings);
    BDBG_ASSERT(!rc);

    rc = NEXUS_VideoWindow_AddInput(window, NEXUS_VideoDecoder_GetConnector(videoDecoder));
    BDBG_ASSERT(!rc);
    
    NEXUS_VideoDecoder_Start(videoDecoder, &videoProgram);
    NEXUS_Playback_Start(playback, file, NULL);

    gettimeofday(&start_timeval, NULL);
    while (1) {
        unsigned display_gfx_index;
        NEXUS_VideoWindowStatus windowStatus;
        unsigned graphics_delay;

        /* adjust the video window first */
        NEXUS_VideoWindow_GetSettings(window, &windowSettings);
        windowSettings.position.width = windowSettings.position.width + incW;
        windowSettings.position.height = windowSettings.position.width * 9 / 16; /* preserve 16:9 aspect ratio */
        if(incW > 0 && windowSettings.position.width >= formatInfo.width) {
            windowSettings.position.width = formatInfo.width;
            windowSettings.position.height = formatInfo.height;
            incW *= -1;
        }
        else if (incW < 0 && windowSettings.position.width < smallest_width) {
            incW *= -1;
        }
        
        /* fill whole screen with blue */
        NEXUS_Graphics2D_GetDefaultFillSettings(&fillSettings);
        fillSettings.surface = framebuffer[cur];
        fillSettings.color = 0xFF0000FF;
        rc = NEXUS_Graphics2D_Fill(gfx, &fillSettings);
        BDBG_ASSERT(!rc);
        /* draw the alpha hole */
        NEXUS_Graphics2D_GetDefaultFillSettings(&fillSettings);
        fillSettings.surface = framebuffer[cur];
        fillSettings.rect = windowSettings.position;
        fillSettings.color = 0;
        rc = NEXUS_Graphics2D_Fill(gfx, &fillSettings);
        BDBG_ASSERT(!rc);
        rc = NEXUS_Graphics2D_Checkpoint(gfx, NULL);
        if (rc == NEXUS_GRAPHICS2D_QUEUED) {
            rc = BKNI_WaitForEvent(checkpointEvent, 5000);
            BDBG_ASSERT(!rc);
        }

#if 0
#define BDBG_MSG_TRACE(X) BDBG_LOG(X)
#else
#define BDBG_MSG_TRACE(X)
#endif

        /* calc which graphics fb to set now */
        display_gfx_index = cur;

        NEXUS_VideoWindow_GetStatus(window, &windowStatus);
        graphics_delay = windowStatus.delay;
        if (graphics_delay) graphics_delay--;
        if (graphics_delay && cnt > graphics_delay-1) {
            /* go back in time */
            display_gfx_index = (cnt + MAX_FRAMEBUFFERS - graphics_delay) % MAX_FRAMEBUFFERS;
        }

        rc = NEXUS_DisplayModule_SetUpdateMode(NEXUS_DisplayUpdateMode_eManual);
        BDBG_ASSERT(!rc);

        BDBG_MSG_TRACE((">NEXUS_VideoWindow_SetSettings"));
        rc = NEXUS_VideoWindow_SetSettings(window, &windowSettings);
        BDBG_ASSERT(!rc);

        BDBG_MSG_TRACE((">NEXUS_Display_SetGraphicsFramebuffer"));
        rc = NEXUS_Display_SetGraphicsFramebuffer(display, framebuffer[display_gfx_index]);
        BDBG_ASSERT(!rc);

        BDBG_MSG_TRACE((">NEXUS_DisplayUpdateMode_eAuto"));
        rc = NEXUS_DisplayModule_SetUpdateMode(NEXUS_DisplayUpdateMode_eAuto);
        BDBG_ASSERT(!rc);

        /* we can't wait on this because it's triggered by BVDC_Source_GetSurface_isr, and that doesn't
        work with VDC graphics-in-applychanges. We end up with good sync, but 30fps.
        Instead, we need to rely on ApplyChanges making it so on the next vsync. */
        BDBG_MSG_TRACE((">BKNI_WaitForEvent(framebufferEvent)"));
        rc = BKNI_WaitForEvent(framebufferEvent, 5000);
        BDBG_ASSERT(!rc);
        BDBG_MSG_TRACE(("done"));

        /* next framebuffer */
        if (++cur == MAX_FRAMEBUFFERS) {
            cur = 0;
        }

        /* print framerate */
        if (++cnt % 60 == 0) {
            unsigned diff;
            struct timeval current_timeval;
            gettimeofday(&current_timeval, NULL);
            diff = current_timeval.tv_sec - start_timeval.tv_sec;
            BDBG_WRN(("%u seconds, %u flips, %u fps", diff, cnt, diff ? cnt / diff : 0));
        }
    }
    return 0;
}
#else
#include <stdio.h>
int main(void)
{
    printf("This application is not supported on this platform\n");
    return 0;
}
#endif
