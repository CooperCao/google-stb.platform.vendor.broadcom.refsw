/******************************************************************************
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
 ******************************************************************************/
#include "nexus_platform.h"
#include <stdio.h>
#if NEXUS_HAS_DISPLAY
#include "nexus_video_decoder.h"
#include "nexus_video_decoder_extra.h"
#include "nexus_stc_channel.h"
#include "nexus_display.h"
#include "nexus_video_window.h"
#include "nexus_video_input.h"
#include "nexus_component_output.h"
#include "nexus_composite_output.h"
#include "nexus_striped_surface.h"
#if NEXUS_HAS_HDMI_OUTPUT
#include "nexus_hdmi_output.h"
#endif
#include "nexus_playback.h"
#include "nexus_file.h"
#include "nexus_graphics2d.h"
#include "nexus_base_mmap.h"

#include <assert.h>
#include "bstd.h"
#include "bkni.h"

BDBG_MODULE(capture_yuv);

/* the following define the input file and its characteristics -- these will vary by input file */
#define FILE_NAME "videos/cnnticker.mpg"
#define TRANSPORT_TYPE NEXUS_TransportType_eTs
#define VIDEO_CODEC NEXUS_VideoCodec_eMpeg2
#define VIDEO_PID 0x21

static void checkpoint(NEXUS_Graphics2DHandle gfx, BKNI_EventHandle checkpointEvent)
{
    int rc;
    rc = NEXUS_Graphics2D_Checkpoint(gfx, NULL); /* require to execute queue */
    if (rc == NEXUS_GRAPHICS2D_QUEUED) {
        rc = BKNI_WaitForEvent(checkpointEvent, BKNI_INFINITE);
    }
    BDBG_ASSERT(!rc);
}
static void complete(void *data, int unused)
{
    BSTD_UNUSED(unused);
    BKNI_SetEvent((BKNI_EventHandle)data);
}

int main(void)
{
    NEXUS_PlatformSettings platformSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_PidChannelHandle videoPidChannel;
    NEXUS_DisplayHandle display;
    NEXUS_VideoWindowHandle window;
    NEXUS_VideoDecoderHandle videoDecoder;
    NEXUS_VideoDecoderStartSettings videoProgram;
    NEXUS_FilePlayHandle file;
    NEXUS_PlaypumpHandle playpump;
    NEXUS_PlaybackHandle playback;
    NEXUS_PlaybackSettings playbackSettings;
    NEXUS_PlaybackPidChannelSettings playbackPidSettings;
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_HdmiOutputStatus hdmiStatus;
    NEXUS_DisplaySettings displaySettings;
#endif
    NEXUS_Error rc;
    const char *fname = FILE_NAME;
    unsigned num_frames = 0;
    NEXUS_GraphicsSettings graphicsSettings;
    NEXUS_Graphics2DHandle gfx;
    NEXUS_Graphics2DSettings gfxSettings;
    NEXUS_Graphics2DFillSettings fillSettings;
    BKNI_EventHandle checkpointEvent;
    NEXUS_SurfaceCreateSettings framebufferCreateSettings;
    NEXUS_SurfaceHandle framebuffer;
    FILE *pixel_file, *luma_file, *chroma_file;

    pixel_file = fopen("capture_yuv.pixel.dat", "wb");
    luma_file = fopen("capture_yuv.luma.dat", "wb");
    chroma_file = fopen("capture_yuv.chroma.dat", "wb");

    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    /* ensure picture buffer heap has eApplication mapping for CPU/fwrite access */
    platformSettings.heap[platformSettings.videoDecoderModuleSettings.avdHeapIndex[0]].memoryType = NEXUS_MemoryType_eApplication;
    NEXUS_Platform_Init(&platformSettings);
    NEXUS_Platform_GetConfiguration(&platformConfig);

    playpump = NEXUS_Playpump_Open(0, NULL);
    assert(playpump);
    playback = NEXUS_Playback_Create();
    assert(playback);

    file = NEXUS_FilePlay_OpenPosix(fname, NULL);
    if (!file) {
        fprintf(stderr, "can't open file:%s\n", fname);
        return -1;
    }

    NEXUS_Playback_GetSettings(playback, &playbackSettings);
    playbackSettings.playpump = playpump;
    playbackSettings.playpumpSettings.transportType = TRANSPORT_TYPE;
    playbackSettings.endOfStreamAction = NEXUS_PlaybackLoopMode_ePause; /* play once */
#define USE_START_PAUSED 1
#if USE_START_PAUSED
    playbackSettings.startPaused = true;
#endif
    NEXUS_Playback_SetSettings(playback, &playbackSettings);

    /* Bring up video display and outputs */
    display = NEXUS_Display_Open(0, NULL);
    window = NEXUS_VideoWindow_Open(display, 0);

#if NEXUS_NUM_COMPONENT_OUTPUTS
    NEXUS_Display_AddOutput(display, NEXUS_ComponentOutput_GetConnector(platformConfig.outputs.component[0]));
#endif
#if NEXUS_NUM_COMPOSITE_OUTPUTS
    NEXUS_Display_AddOutput(display, NEXUS_CompositeOutput_GetConnector(platformConfig.outputs.composite[0]));
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

    /* bring up decoder and connect to display */
    videoDecoder = NEXUS_VideoDecoder_Open(0, NULL); /* take default capabilities */
    NEXUS_VideoWindow_AddInput(window, NEXUS_VideoDecoder_GetConnector(videoDecoder));

    NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
    playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eVideo;
    playbackPidSettings.pidTypeSettings.video.codec = VIDEO_CODEC; /* must be told codec for correct handling */
    playbackPidSettings.pidTypeSettings.video.index = true;
    playbackPidSettings.pidTypeSettings.video.decoder = videoDecoder;
    videoPidChannel = NEXUS_Playback_OpenPidChannel(playback, VIDEO_PID, &playbackPidSettings);

    /* Set up decoder Start structures now. We need to know the audio codec to properly set up the audio outputs. */
    NEXUS_VideoDecoder_GetDefaultStartSettings(&videoProgram);
    videoProgram.codec = VIDEO_CODEC;
    videoProgram.pidChannel = videoPidChannel;

    rc = NEXUS_VideoDecoder_Start(videoDecoder, &videoProgram);
    BDBG_ASSERT(!rc);
    rc = NEXUS_Playback_Start(playback, file, NULL);
    BDBG_ASSERT(!rc);
#if !USE_START_PAUSED
    rc = NEXUS_Playback_Pause(playback);
    BDBG_ASSERT(!rc);
#endif

    /* create framebuffer to display destriped luma surface */
    BKNI_CreateEvent(&checkpointEvent);
    gfx = NEXUS_Graphics2D_Open(NEXUS_ANY_ID, NULL);
    NEXUS_Graphics2D_GetSettings(gfx, &gfxSettings);
    gfxSettings.checkpointCallback.callback = complete;
    gfxSettings.checkpointCallback.context = checkpointEvent;
    rc = NEXUS_Graphics2D_SetSettings(gfx, &gfxSettings);
    BDBG_ASSERT(!rc);
    NEXUS_Surface_GetDefaultCreateSettings(&framebufferCreateSettings);
    framebufferCreateSettings.pixelFormat = NEXUS_PixelFormat_eA8_R8_G8_B8;
    framebufferCreateSettings.width = 720;
    framebufferCreateSettings.height = 480;
    framebufferCreateSettings.heap = NEXUS_Platform_GetFramebufferHeap(0);
    framebuffer = NEXUS_Surface_Create(&framebufferCreateSettings);
    NEXUS_Graphics2D_GetDefaultFillSettings(&fillSettings);
    fillSettings.surface = framebuffer;
    fillSettings.color = 0;
    rc = NEXUS_Graphics2D_Fill(gfx, &fillSettings);
    BDBG_ASSERT(!rc);
    checkpoint(gfx, checkpointEvent);
    NEXUS_Display_GetGraphicsSettings(display, &graphicsSettings);
    graphicsSettings.clip.width = 720;
    graphicsSettings.clip.height = 480;
    graphicsSettings.enabled = true;
    NEXUS_Display_SetGraphicsSettings(display, &graphicsSettings);
    NEXUS_Display_SetGraphicsFramebuffer(display, framebuffer);

    /* frame advance through the stream, capturing all frames as striped 4:2:0 surfaces. */
    while (1) {
        NEXUS_StripedSurfaceHandle stripedSurface;
        NEXUS_StripedSurfaceCreateSettings createSettings;
        NEXUS_PlaybackStatus pstatus;
        NEXUS_VideoDecoderStatus vstatus;
        NEXUS_Rect rect = {100,100,100,100};
        NEXUS_Addr lumaOffset, chromaOffset;

        rc = NEXUS_Playback_FrameAdvance(playback, true);
        BDBG_ASSERT(!rc);

        while (1) {
            stripedSurface = NEXUS_VideoDecoder_CreateStripedSurface(videoDecoder);
            if (stripedSurface) break;
            BKNI_Sleep(1);
        }

        num_frames++;
        NEXUS_StripedSurface_GetCreateSettings(stripedSurface, &createSettings);
        BDBG_LOG(("striped surface"));
        NEXUS_MemoryBlock_LockOffset(createSettings.lumaBuffer, &lumaOffset);
        BDBG_LOG(("  luma   0x%08x image=%ldx%ld, stripe %ldx%ld",
            (unsigned)lumaOffset + createSettings.lumaBufferOffset,
            createSettings.imageWidth, createSettings.imageHeight,
            createSettings.stripedWidth, createSettings.lumaStripedHeight));
        NEXUS_MemoryBlock_LockOffset(createSettings.chromaBuffer, &chromaOffset);
        BDBG_LOG(("  chroma 0x%08x image=%ldx%ld, stripe %ldx%ld",
            (unsigned)chromaOffset + createSettings.chromaBufferOffset,
            createSettings.imageWidth, createSettings.imageHeight,
            createSettings.stripedWidth, createSettings.chromaStripedHeight));

        {
        void *luma_ptr = NEXUS_OffsetToCachedAddr(lumaOffset + createSettings.lumaBufferOffset);
        void *chroma_ptr = NEXUS_OffsetToCachedAddr(chromaOffset + createSettings.lumaBufferOffset);
        fwrite(luma_ptr, 1, createSettings.stripedWidth * createSettings.lumaStripedHeight, luma_file);
        fwrite(chroma_ptr, 1, createSettings.stripedWidth * createSettings.chromaStripedHeight, chroma_file);
        }

        rc = NEXUS_Graphics2D_DestripeToSurface(gfx, stripedSurface, framebuffer, &rect);
        BDBG_ASSERT(!rc);
        checkpoint(gfx, checkpointEvent);

        {
        NEXUS_SurfaceMemory mem;
        NEXUS_Surface_GetMemory(framebuffer, &mem);
        NEXUS_Surface_Flush(framebuffer);
        fwrite(mem.buffer, 1, mem.pitch * framebufferCreateSettings.height, pixel_file); /* for pixel data */
        }

        NEXUS_VideoDecoder_DestroyStripedSurface(videoDecoder, stripedSurface);

        rc = NEXUS_Playback_GetStatus(playback, &pstatus);
        BDBG_ASSERT(!rc);
        rc = NEXUS_VideoDecoder_GetStatus(videoDecoder, &vstatus);
        BDBG_ASSERT(!rc);
        if (pstatus.last == pstatus.position && !vstatus.queueDepth) break;
    }

    printf("%d frames\n", num_frames);
    fclose(pixel_file);
    fclose(luma_file);
    fclose(chroma_file);

    /* Bring down system */
    NEXUS_VideoDecoder_Stop(videoDecoder);
    NEXUS_Playback_Stop(playback);
    NEXUS_FilePlay_Close(file);
    NEXUS_Playback_Destroy(playback);
    NEXUS_Playpump_Close(playpump);
    NEXUS_VideoDecoder_Close(videoDecoder);
    NEXUS_VideoWindow_Close(window);
    NEXUS_Display_Close(display);
    NEXUS_Graphics2D_Close(gfx);
    BKNI_DestroyEvent(checkpointEvent);
    NEXUS_Surface_Destroy(framebuffer);
    NEXUS_Platform_Uninit();
    return 0;
}
#else
int main(void)
{
    printf("This application is not supported on this platform\n");
    return 0;
}
#endif
