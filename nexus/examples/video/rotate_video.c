/******************************************************************************
 *    (c)2008-2013 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
******************************************************************************/
#include "nexus_platform.h"
#include <stdio.h>
#if NEXUS_HAS_DISPLAY
#include "nexus_playback.h"
#include "nexus_pid_channel.h"
#include "nexus_parser_band.h"
#include "nexus_video_decoder.h"
#include "nexus_video_decoder_extra.h"
#include "nexus_stc_channel.h"
#include "nexus_display.h"
#include "nexus_core_utils.h"
#include "nexus_component_output.h"
#include "nexus_composite_output.h"
#if NEXUS_HAS_HDMI_OUTPUT
#include "nexus_hdmi_output.h"
#endif
#include "nexus_graphics2d.h"
#include "bstd.h"
#include "bkni.h"
#include <stdlib.h>
#include <string.h>

BDBG_MODULE(rotate_video);

/* the following define the input and its characteristics -- these will vary by input */
#if 1
#define FILE_NAME "videos/cnnticker.mpg"
#define TRANSPORT_TYPE NEXUS_TransportType_eTs
#define VIDEO_CODEC NEXUS_VideoCodec_eMpeg2
#define VIDEO_PID 0x21
#else
#define FILE_NAME "videos/herbie2AvcHD.mpg"
#define TRANSPORT_TYPE NEXUS_TransportType_eTs
#define VIDEO_CODEC NEXUS_VideoCodec_eH264
#define VIDEO_PID 0x1222
#endif

/* several flavors of CPU-based rotation, from simplest to most complex. */
void cpu_rotate_90(const void *src, unsigned src_pitch, void *dest, unsigned dest_pitch, unsigned width, unsigned height)
{
    unsigned y;

    for (y=0;y<width;y++) {
        uint32_t *srcptr = (uint32_t *)((uint8_t*)src + src_pitch * y);
        unsigned x;
        for (x=0;x<height;x++) {
            uint32_t *destptr = (uint32_t *)((uint8_t*)dest + dest_pitch * x);
            destptr[y] = srcptr[x];
        }
    }
    return;
}

void cpu_rotate_90_transposed(const void *src, unsigned src_pitch, void *dest, unsigned dest_pitch, unsigned width, unsigned height)
{
    unsigned y;

    for (y=0;y<width;y++) {
        uint32_t *destptr = (uint32_t *)((uint8_t*)dest + dest_pitch * y);
        unsigned x;
        for (x=0;x<height;x++) {
            uint32_t *srcptr = (uint32_t *)((uint8_t*)src + src_pitch * x);
            destptr[x] = srcptr[y];
        }
    }
    return;
}

void cpu_rotate_90_transposed_optimized(const void *src, unsigned src_pitch, void *dest, unsigned dest_pitch, unsigned width, unsigned height)
{
    unsigned y;

    for (y=0;y<width;y++) {
        uint32_t *destptr = (uint32_t *)((uint8_t*)dest + dest_pitch * y);
        unsigned x;
        uint8_t *srcptr = (uint8_t *)((uint32_t*)src+y);
        for (x=0;x<height;x++) {
            destptr[x] = *((uint32_t *)srcptr);
            srcptr += src_pitch;
        }
    }
    return;
}

#ifdef _MIPS_ARCH
#define B_KERNEL_DCACHE_LINE_SIZE   64
#define b_prepare_for_store(base) __asm__ __volatile__( \
    ".set push      \n"\
    ".set noreorder \n"\
    "pref 30,(0)(%0);    \n"\
    ".set pop       \n"\
    : \
    : "r" (base))


#define PIXELS_IN_CACHE_LINE (B_KERNEL_DCACHE_LINE_SIZE/sizeof(uint32_t))
void cpu_rotate_90_transposed_optimized_cache(const void *src, unsigned src_pitch, void *dest, unsigned dest_pitch, unsigned width, unsigned height)
{
    unsigned y;

    for (y=0;y<width;y++) {
        uint32_t *destptr = (uint32_t *)((uint8_t*)dest + dest_pitch * y);
        unsigned x;
        uint8_t *srcptr = (uint8_t *)((uint32_t*)src+y);
        for(x=0;x<height;x+=PIXELS_IN_CACHE_LINE) {
            uint32_t pixel0,pixel1,pixel2,pixel3;

            b_prepare_for_store(destptr);

            pixel0 = *((uint32_t *)srcptr); srcptr += src_pitch;
            pixel1 = *((uint32_t *)srcptr); srcptr += src_pitch;
            pixel2 = *((uint32_t *)srcptr); srcptr += src_pitch;
            pixel3 = *((uint32_t *)srcptr); srcptr += src_pitch;
            destptr[0] = pixel0;
            destptr[1] = pixel1;
            destptr[2] = pixel2;
            destptr[3] = pixel3;

            pixel0 = *((uint32_t *)srcptr); srcptr += src_pitch;
            pixel1 = *((uint32_t *)srcptr); srcptr += src_pitch;
            pixel2 = *((uint32_t *)srcptr); srcptr += src_pitch;
            pixel3 = *((uint32_t *)srcptr); srcptr += src_pitch;
            destptr[4+0] = pixel0;
            destptr[4+1] = pixel1;
            destptr[4+2] = pixel2;
            destptr[4+3] = pixel3;

            pixel0 = *((uint32_t *)srcptr); srcptr += src_pitch;
            pixel1 = *((uint32_t *)srcptr); srcptr += src_pitch;
            pixel2 = *((uint32_t *)srcptr); srcptr += src_pitch;
            pixel3 = *((uint32_t *)srcptr); srcptr += src_pitch;
            destptr[8+0] = pixel0;
            destptr[8+1] = pixel1;
            destptr[8+2] = pixel2;
            destptr[8+3] = pixel3;

            pixel0 = *((uint32_t *)srcptr); srcptr += src_pitch;
            pixel1 = *((uint32_t *)srcptr); srcptr += src_pitch;
            pixel2 = *((uint32_t *)srcptr); srcptr += src_pitch;
            pixel3 = *((uint32_t *)srcptr); srcptr += src_pitch;
            destptr[12+0] = pixel0;
            destptr[12+1] = pixel1;
            destptr[12+2] = pixel2;
            destptr[12+3] = pixel3;

            destptr += PIXELS_IN_CACHE_LINE;
            
        }
        if(x>width) {
            x -= PIXELS_IN_CACHE_LINE;
            srcptr = (uint8_t *)((uint32_t*)src+y) + x*src_pitch;
            destptr = (uint32_t *)((uint8_t*)dest + dest_pitch * y);
            for (;x<width;x++) {
                destptr[x] = *((uint32_t *)srcptr);
                srcptr += src_pitch;
            }
        }
    }
    return;
}
#endif

void complete(void *data, int unused)
{
    BSTD_UNUSED(unused);
    BKNI_SetEvent((BKNI_EventHandle)data);
}

int main(int argc, char **argv)
{
    NEXUS_VideoDecoderHandle videoDecoder;
    NEXUS_VideoDecoderStartSettings videoProgram;
    NEXUS_PidChannelHandle videoPidChannel;
    NEXUS_FilePlayHandle file;
    NEXUS_PlaypumpHandle playpump;
    NEXUS_PlaybackHandle playback;
    NEXUS_PlaybackSettings playbackSettings;
    NEXUS_PlaybackPidChannelSettings playbackPidSettings;
    NEXUS_StcChannelSettings stcSettings;
    NEXUS_StcChannelHandle stcChannel;
    NEXUS_DisplayHandle display;
    NEXUS_DisplaySettings displaySettings;
    NEXUS_VideoFormatInfo formatInfo;
    NEXUS_PlatformSettings platformSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_Graphics2DHandle gfx;
    NEXUS_Graphics2DSettings gfxSettings;
    BKNI_EventHandle event;
    NEXUS_SurfaceCreateSettings surfaceCreateSettings;
    NEXUS_SurfaceHandle framebuffer[2], videoSurface, secondVideoSurface;
    NEXUS_SurfaceMemory mem[2], videoSurfaceMem, secondVideoSurfaceMem;
    NEXUS_GraphicsSettings graphicsSettings;
    NEXUS_Error rc;
    int curarg = 1;
    unsigned rotate = 90;
    unsigned current = 0;

    while (curarg < argc) {
        if (!strcmp(argv[curarg], "-h") || !strcmp(argv[curarg], "--help")) {
            printf("Usage: nexus rotate_video [-rotate {0,90,180,270}]\n");
            return 0;
        }
        else if (!strcmp(argv[curarg], "-rotate") && curarg+1 < argc) {
            rotate = atoi(argv[++curarg]);
        }
        curarg++;
    }

    /* Bring up all modules for a platform in a default configuraiton for this platform */
    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init(&platformSettings);
    NEXUS_Platform_GetConfiguration(&platformConfig);
    
#if NEXUS_HAS_GRAPHICSV3D
    BDBG_WRN(("The preferred way to rotate video is with the 3D graphics core. An example application is available."));
#endif

    playpump = NEXUS_Playpump_Open(0, NULL);
    BDBG_ASSERT(playpump);
    playback = NEXUS_Playback_Create();
    BDBG_ASSERT(playback);

    file = NEXUS_FilePlay_OpenPosix(FILE_NAME, FILE_NAME);
    if (!file) {
        fprintf(stderr, "can't open file:%s\n", FILE_NAME);
        return -1;
    }

    NEXUS_StcChannel_GetDefaultSettings(0, &stcSettings);
    stcSettings.timebase = NEXUS_Timebase_e0;
    stcSettings.mode = NEXUS_StcChannelMode_eAuto;
    stcSettings.modeSettings.Auto.transportType = TRANSPORT_TYPE;
    stcChannel = NEXUS_StcChannel_Open(0, &stcSettings);

    NEXUS_Playback_GetSettings(playback, &playbackSettings);
    playbackSettings.playpump = playpump;
    playbackSettings.playpumpSettings.transportType = TRANSPORT_TYPE;
    playbackSettings.stcChannel = stcChannel;
    NEXUS_Playback_SetSettings(playback, &playbackSettings);

    videoDecoder = NEXUS_VideoDecoder_Open(0, NULL); /* take default capabilities */
    
    NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
    playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eVideo;
    playbackPidSettings.pidTypeSettings.video.codec = VIDEO_CODEC; /* must be told codec for correct handling */
    playbackPidSettings.pidTypeSettings.video.index = true;
    playbackPidSettings.pidTypeSettings.video.decoder = videoDecoder;
    videoPidChannel = NEXUS_Playback_OpenPidChannel(playback, VIDEO_PID, &playbackPidSettings);
    
    NEXUS_VideoDecoder_GetDefaultStartSettings(&videoProgram);
    videoProgram.codec = VIDEO_CODEC;
    videoProgram.pidChannel = videoPidChannel;
    videoProgram.stcChannel = stcChannel;

    /* bring up display and graphics */
    NEXUS_Display_GetDefaultSettings(&displaySettings);
    displaySettings.format = NEXUS_VideoFormat_e720p;
    display = NEXUS_Display_Open(0, &displaySettings);

#if NEXUS_NUM_COMPONENT_OUTPUTS
    NEXUS_Display_AddOutput(display, NEXUS_ComponentOutput_GetConnector(platformConfig.outputs.component[0]));
#endif
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_Display_AddOutput(display, NEXUS_HdmiOutput_GetVideoConnector(platformConfig.outputs.hdmi[0]));
#endif

    NEXUS_VideoFormat_GetInfo(displaySettings.format, &formatInfo);

    BKNI_CreateEvent(&event);

    gfx = NEXUS_Graphics2D_Open(0, NULL);
    NEXUS_Graphics2D_GetSettings(gfx, &gfxSettings);
    gfxSettings.checkpointCallback.callback = complete;
    gfxSettings.checkpointCallback.context = event;
    NEXUS_Graphics2D_SetSettings(gfx, &gfxSettings);

    /* create the framebuffer */
    NEXUS_Surface_GetDefaultCreateSettings(&surfaceCreateSettings);
    surfaceCreateSettings.pixelFormat = NEXUS_PixelFormat_eA8_R8_G8_B8;
    surfaceCreateSettings.width = formatInfo.width;
    surfaceCreateSettings.height = formatInfo.height;
    surfaceCreateSettings.heap = NEXUS_Platform_GetFramebufferHeap(0);
    framebuffer[0] = NEXUS_Surface_Create(&surfaceCreateSettings);
    NEXUS_Surface_GetMemory(framebuffer[0], &mem[0]);
    framebuffer[1] = NEXUS_Surface_Create(&surfaceCreateSettings);
    NEXUS_Surface_GetMemory(framebuffer[1], &mem[1]);
    BKNI_Memset(mem[0].buffer, 0, mem[0].pitch * surfaceCreateSettings.height);
    BKNI_Memset(mem[1].buffer, 0, mem[1].pitch * surfaceCreateSettings.height);
    
    /* max video size */
    surfaceCreateSettings.width = 1920;
    surfaceCreateSettings.height = 1080;
    surfaceCreateSettings.heap = NEXUS_Platform_GetFramebufferHeap(0);
    videoSurface = NEXUS_Surface_Create(&surfaceCreateSettings);
    NEXUS_Surface_GetMemory(videoSurface, &videoSurfaceMem);

    /* enough to rotate video */
    surfaceCreateSettings.width = 720;
    surfaceCreateSettings.height = 720;
    surfaceCreateSettings.heap = NEXUS_Platform_GetFramebufferHeap(0);
    secondVideoSurface = NEXUS_Surface_Create(&surfaceCreateSettings);
    NEXUS_Surface_GetMemory(secondVideoSurface, &secondVideoSurfaceMem);
    
    /* Tell Display module to connect to the VideoDecoder module and supply the
    L1 INT id's from BVDC_Display_GetInterrupt. Display will not register for the data ready ISR callback. */
    NEXUS_Display_DriveVideoDecoder(display);

    NEXUS_Display_GetGraphicsSettings(display, &graphicsSettings);
    graphicsSettings.enabled = true;
    NEXUS_Display_SetGraphicsSettings(display, &graphicsSettings);
    NEXUS_Display_SetGraphicsFramebuffer(display, framebuffer[current]);
    current = 1 - current;

    NEXUS_VideoDecoder_Start(videoDecoder, &videoProgram);
    NEXUS_Playback_Start(playback, file, NULL);

    /* Monitor status */
    while (1) {
        NEXUS_StripedSurfaceHandle stripedSurface;

        /* Create a Nexus striped surface for the most recent picture reported by VideoDecoder */
        stripedSurface = NEXUS_VideoDecoder_CreateStripedSurface(videoDecoder);

        if (stripedSurface) {
            NEXUS_Graphics2DBlitSettings blitSettings;
            NEXUS_StripedSurfaceCreateSettings stripedSettings;
            NEXUS_Rect rect = {0,0,0,0};
            
            NEXUS_StripedSurface_GetCreateSettings(stripedSurface, &stripedSettings);
            rect.width = stripedSettings.imageWidth;
            rect.height = stripedSettings.imageHeight;
            
            if (rotate == 90 || rotate == 270) {
                /* downscale in destripe to no more than 720 wide so that CPU flip works better */
                if (rect.width > 960) {
                    rect.height = rect.height * 720 / rect.width;
                    rect.width = 720;
                }
            }
            
            rc = NEXUS_Graphics2D_DestripeToSurface(gfx, stripedSurface, videoSurface, &rect);
            BDBG_ASSERT(!rc);
            
            if (rotate == 90 || rotate == 270) {
                /* CPU rotation */
                unsigned temp;
                
                rc = NEXUS_Graphics2D_Checkpoint(gfx, NULL);
                if (rc==NEXUS_GRAPHICS2D_QUEUED) {
                    rc = BKNI_WaitForEvent(event, BKNI_INFINITE);
                    BDBG_ASSERT(!rc);
                }
                
                NEXUS_FlushCache(videoSurfaceMem.buffer, videoSurfaceMem.pitch * rect.height);
                NEXUS_FlushCache(secondVideoSurfaceMem.buffer, secondVideoSurfaceMem.pitch * rect.height);
#ifdef _MIPS_ARCH
                cpu_rotate_90_transposed_optimized_cache(videoSurfaceMem.buffer, videoSurfaceMem.pitch, secondVideoSurfaceMem.buffer, secondVideoSurfaceMem.pitch,rect.width,rect.height);
#else
                cpu_rotate_90_transposed_optimized(videoSurfaceMem.buffer, videoSurfaceMem.pitch, secondVideoSurfaceMem.buffer, secondVideoSurfaceMem.pitch,rect.width,rect.height);
#endif
                /* cpu_rotate_90(videoSurfaceMem.buffer, videoSurfaceMem.pitch, secondVideoSurfaceMem.buffer, secondVideoSurfaceMem.pitch,rect.width,rect.height); */
                NEXUS_Surface_Flush(secondVideoSurface);
                
                temp = rect.width;
                rect.width = rect.height;
                rect.height = temp;
            }

            NEXUS_Graphics2D_GetDefaultBlitSettings(&blitSettings);
            if (rotate == 90 || rotate == 270) {
                blitSettings.source.surface = secondVideoSurface;
            }
            else {
                blitSettings.source.surface = videoSurface;
            }
            blitSettings.source.rect  = rect;
            blitSettings.output.surface = framebuffer[current];
            
            /* simple box A/R correction on rotated video 
            TODO: This is based on source size, not source aspect ratio */
            if (rotate == 90 || rotate == 270) {
                blitSettings.output.rect.width = stripedSettings.imageHeight * formatInfo.height / stripedSettings.imageWidth;
                blitSettings.output.rect.height = formatInfo.height;
                blitSettings.output.rect.x = (formatInfo.width-blitSettings.output.rect.width)/2;
                blitSettings.output.rect.y = 0;
            }
            
            /* M2MC mirroring */
            if (rotate == 180 || rotate == 270) {
                blitSettings.mirrorOutputVertically = true;
            }
            else if (rotate == 90) {
                blitSettings.mirrorOutputHorizontally = true;
            }
            rc = NEXUS_Graphics2D_Blit(gfx, &blitSettings);
            BDBG_ASSERT(!rc);

            rc = NEXUS_Graphics2D_Checkpoint(gfx, NULL);
            if (rc==NEXUS_GRAPHICS2D_QUEUED) {
                rc = BKNI_WaitForEvent(event, BKNI_INFINITE);
                BDBG_ASSERT(!rc);
            }

            NEXUS_Display_SetGraphicsFramebuffer(display, framebuffer[current]);
            current = 1 - current;
            
            /* No additional wait because we are already paced by NEXUS_VideoDecoder_CreateStripedSurface. This
            would need to be more sophisticated for frame rate conversion. */
            
            NEXUS_VideoDecoder_DestroyStripedSurface(videoDecoder, stripedSurface);
        }
        else {
            BKNI_Sleep(1);
        }
    }

    NEXUS_VideoDecoder_Stop(videoDecoder);
    NEXUS_VideoDecoder_Close(videoDecoder);
    NEXUS_Surface_Destroy(framebuffer[0]);
    NEXUS_Surface_Destroy(framebuffer[1]);
    NEXUS_PidChannel_Close(videoProgram.pidChannel);
    NEXUS_StcChannel_Close(videoProgram.stcChannel);
    NEXUS_Display_Close(display);
    NEXUS_Graphics2D_Close(gfx);
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
