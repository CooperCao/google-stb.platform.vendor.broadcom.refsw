/******************************************************************************
 *    (c)2008 Broadcom Corporation
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
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *****************************************************************************/
#include "bstd.h"
#include "bkni.h"

#include "nexus_platform.h"
#include "nexus_display.h"
#include "nexus_video_window.h"
#include "nexus_component_output.h"
#include "nexus_video_decoder.h"
#include "nexus_still_decoder.h"
#include "nexus_ir_input.h"
#include "nexus_surface.h"
#include "nexus_graphics2d.h"
#include "nexus_playpump.h"
#include "nexus_playback.h"
#include "nexus_types.h"
#include "nexus_video_types.h"
#include "nexus_core_utils.h"

#include "bfile_stdio.h"
#include "thumbnail.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

BDBG_MODULE(thumbnail_demo);

BKNI_EventHandle irInputEvent;
NEXUS_DisplayHandle display;
NEXUS_SurfaceHandle framebufferSurface;
NEXUS_VideoWindowHandle window;
NEXUS_VideoDecoderHandle videoDecoder;
NEXUS_PlaybackHandle playback;
NEXUS_PlaypumpHandle playpump;
NEXUS_FilePlayHandle file;
NEXUS_IrInputHandle irInput;

static int thumbnail_show(const NEXUS_Rect *rect, NEXUS_SurfaceHandle stillSurface);
static int thumbnail_border(NEXUS_SurfaceHandle stillSurface, unsigned color);

static void ir_input_ready(void *context, int unused)
{
    BSTD_UNUSED(unused);
    BKNI_SetEvent((BKNI_EventHandle)context);
}

thumbnail_data g_data;

static void checkpoint_cb(void *context, int param)
{
    BSTD_UNUSED(param);
    BKNI_SetEvent((BKNI_EventHandle)context);
}

void checkpoint(void)
{
    int rc;
    rc = NEXUS_Graphics2D_Checkpoint(g_data.blitter, NULL);
    if (rc == NEXUS_GRAPHICS2D_QUEUED) {
        BKNI_WaitForEvent(g_data.blitterEvent, 0xffffffff);
    }
}

int thumbnail_demo_init(void)
{
    NEXUS_SurfaceCreateSettings createSettings;
    NEXUS_GraphicsSettings graphicsSettings;
    NEXUS_PlatformSettings platformSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_DisplaySettings displaySettings;
    NEXUS_Graphics2DFillSettings fillSettings;
    NEXUS_Graphics2DSettings graphics2DSettings;
    NEXUS_Error rc;
    NEXUS_PlaybackSettings playbackSettings;
    NEXUS_IrInputSettings irInputSettings;
    NEXUS_VideoFormatInfo displayFormatInfo;

    /* Bring up all modules for a platform in a default configuraiton for this platform */
    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init(&platformSettings);
    NEXUS_Platform_GetConfiguration(&platformConfig);

    BKNI_CreateEvent(&irInputEvent);
    NEXUS_IrInput_GetDefaultSettings(&irInputSettings);
    irInputSettings.dataReady.callback = ir_input_ready;
    irInputSettings.dataReady.context = irInputEvent;
    irInput = NEXUS_IrInput_Open(0, &irInputSettings);

    NEXUS_Display_GetDefaultSettings(&displaySettings);
    displaySettings.displayType = NEXUS_DisplayType_eAuto;
    displaySettings.format = g_data.display_format;
    display = NEXUS_Display_Open(0, &displaySettings);
    NEXUS_Display_AddOutput(display, NEXUS_ComponentOutput_GetConnector(platformConfig.outputs.component[0]));
    NEXUS_Display_AddOutput(display, NEXUS_HdmiOutput_GetVideoConnector(platformConfig.outputs.hdmi[0]));

    NEXUS_VideoFormat_GetInfo(displaySettings.format, &displayFormatInfo);

    window = NEXUS_VideoWindow_Open(display, 0);

    g_data.blitter = NEXUS_Graphics2D_Open(0, NULL);
    BKNI_CreateEvent(&g_data.blitterEvent);
    NEXUS_Graphics2D_GetSettings(g_data.blitter, &graphics2DSettings);
    graphics2DSettings.checkpointCallback.callback = checkpoint_cb;
    graphics2DSettings.checkpointCallback.context = g_data.blitterEvent;
    NEXUS_Graphics2D_SetSettings(g_data.blitter, &graphics2DSettings);

    /* use one playback for stills */
    g_data.playpump = NEXUS_Playpump_Open(0, NULL);
    BDBG_ASSERT(g_data.playpump);

    /* use second playback for PVR */
    playpump = NEXUS_Playpump_Open(1, NULL);
    BDBG_ASSERT(playpump);
    playback = NEXUS_Playback_Create();
    BDBG_ASSERT(playback);

    NEXUS_Playback_GetSettings(playback, &playbackSettings);
    playbackSettings.playpump = playpump;
    NEXUS_Playback_SetSettings(playback, &playbackSettings);

    videoDecoder = NEXUS_VideoDecoder_Open(0, NULL);
    NEXUS_VideoWindow_AddInput(window, NEXUS_VideoDecoder_GetConnector(videoDecoder));

    g_data.stillDecoder = NEXUS_StillDecoder_Open(videoDecoder, 0, NULL);

    /* create graphics framebuffer */
    NEXUS_Surface_GetDefaultCreateSettings(&createSettings);
    createSettings.pixelFormat = NEXUS_PixelFormat_eA8_R8_G8_B8;
    createSettings.width = displayFormatInfo.width;
    createSettings.height = displayFormatInfo.height;
    createSettings.heap = NEXUS_Platform_GetFramebufferHeap(0);
    framebufferSurface = NEXUS_Surface_Create(&createSettings);

    /* make fb transparent */
    NEXUS_Graphics2D_GetDefaultFillSettings(&fillSettings);
    fillSettings.surface = framebufferSurface;
    fillSettings.color = 0x00000000; /* transparent */
    rc = NEXUS_Graphics2D_Fill(g_data.blitter, &fillSettings);
    BDBG_ASSERT(!rc);
    checkpoint();

    /* force full screen framebuffer. no horizontal upscale. */
    NEXUS_Display_GetGraphicsSettings(display, &graphicsSettings);
    graphicsSettings.position.width = displayFormatInfo.width;
    graphicsSettings.position.height = displayFormatInfo.height;
    graphicsSettings.clip.width = displayFormatInfo.width;
    graphicsSettings.clip.height = displayFormatInfo.height;
    graphicsSettings.enabled = true;
    NEXUS_Display_SetGraphicsSettings(display, &graphicsSettings);
    NEXUS_Display_SetGraphicsFramebuffer(display, framebufferSurface);

    return 0;
}

void thumbnail_demo_uninit(void)
{
    NEXUS_IrInput_Close(irInput);
    NEXUS_Display_Close(display);
    NEXUS_VideoDecoder_Close(videoDecoder);
    NEXUS_Playback_Destroy(playback);
    BKNI_DestroyEvent(irInputEvent);
    NEXUS_Playpump_Close(playpump);

    NEXUS_Graphics2D_Close(g_data.blitter);
    BKNI_DestroyEvent(g_data.blitterEvent);
    NEXUS_StillDecoder_Close(g_data.stillDecoder);
    NEXUS_Playpump_Close(g_data.playpump);

    NEXUS_Platform_Uninit();
}

static int thumbnail_show_stills(void)
{
    unsigned i;
    for (i=0;i<DISPLAYED_THUMBNAILS;i++) {
        NEXUS_Rect rect;
        unsigned time;
        bthumbnail_extractor_status status;

        /* use i+1 because there is no thumbnail at time 0. */
        time = g_data.base_time + (i+1)*g_data.spacing;
        if (g_data.thumbnail[i].time != time ||
            !g_data.thumbnail[i].surface)
        {
            if (g_data.thumbnail[i].surface) {
                NEXUS_Surface_Destroy(g_data.thumbnail[i].surface);
                g_data.thumbnail[i].surface = NULL;
            }
            g_data.thumbnail[i].time = time;
            BDBG_MSG(("decode still[%d] from %d sec", i, time));
            g_data.thumbnail[i].surface = thumbnail_decode_still(time);
            if (!g_data.thumbnail[i].surface) {
                continue;
            }
            else {
                bthumbnail_extractor_get_status(g_data.thumbnail_extractor, &status);
                BDBG_MSG(("got still[%d] from %d ms", i, status.timestamp));
            }
        }

        thumbnail_border(g_data.thumbnail[i].surface, time == g_data.current_time ? 0xFF00FF00 : 0xFF333333);

        rect.x = THUMB_EDGE + i*(THUMB_WIDTH + THUMB_GAP);
        rect.y = 550;
        rect.width = THUMB_WIDTH;
        rect.height = THUMB_HEIGHT;

        thumbnail_show(&rect, g_data.thumbnail[i].surface);
    }
    return 0;
}

static int thumbnail_show(const NEXUS_Rect *rect, NEXUS_SurfaceHandle stillSurface)
{
    NEXUS_Graphics2DBlitSettings blitSettings;
    int rc;

    NEXUS_Graphics2D_GetDefaultBlitSettings(&blitSettings);
    blitSettings.colorOp = NEXUS_BlitColorOp_eCopySource;
    blitSettings.alphaOp = NEXUS_BlitAlphaOp_eCopyConstant; /* YCrCb has no alpha, so we must set 0xFF */
    blitSettings.constantColor = 0xFF000000; /* alpha is opaque */
    blitSettings.source.surface = stillSurface;
    blitSettings.output.surface = framebufferSurface;
    blitSettings.output.rect = *rect;
    rc = NEXUS_Graphics2D_Blit(g_data.blitter, &blitSettings);
    if (rc) return BERR_TRACE(rc);
    
    checkpoint();
    return 0;
}

#define BORDER_WIDTH 3

static int thumbnail_border(NEXUS_SurfaceHandle stillSurface, unsigned color)
{
    NEXUS_SurfaceMemory mem;
    NEXUS_SurfaceCreateSettings createSettings;
    unsigned x, y;

    NEXUS_Surface_GetMemory(stillSurface, &mem);
    NEXUS_Surface_GetCreateSettings(stillSurface, &createSettings);

    for (y=0;y<BORDER_WIDTH;y++) {
        for (x=0;x<createSettings.width;x++) {
            ((uint32_t*)((uint8_t*)mem.buffer+y*mem.pitch))[x] = color;
        }
    }
    for (y=createSettings.height-BORDER_WIDTH;(int)y<createSettings.height;y++) {
        for (x=0;x<createSettings.width;x++) {
            ((uint32_t*)((uint8_t*)mem.buffer+y*mem.pitch))[x] = color;
        }
    }
    for (y=0;y<createSettings.height;y++) {
        for (x=0;x<BORDER_WIDTH;x++) {
            ((uint32_t*)((uint8_t*)mem.buffer+y*mem.pitch))[x] = color;
        }
        for (x=createSettings.width-BORDER_WIDTH;(int)x<createSettings.width;x++) {
            ((uint32_t*)((uint8_t*)mem.buffer+y*mem.pitch))[x] = color;
        }
    }
    NEXUS_Surface_Flush(stillSurface);
    return 0;
}

int thumbnail_play_video(void)
{
    NEXUS_VideoDecoderStartSettings startSettings;
    NEXUS_PlaybackPidChannelSettings pidSettings;
    NEXUS_PlaybackSettings playbackSettings;
    NEXUS_Error rc;

    file = NEXUS_FilePlay_OpenPosix(g_data.datafilename, g_data.indexfilename);
    if (!file) {
        fprintf(stderr, "can't open file:%s index:%s\n", g_data.datafilename, g_data.indexfilename);
        return -1;
    }

    NEXUS_VideoDecoder_GetDefaultStartSettings(&startSettings);

    NEXUS_Playback_GetSettings(playback, &playbackSettings);
    playbackSettings.playpumpSettings.transportType = g_data.transportType;
    playbackSettings.playpumpSettings.timestamp.type = g_data.timestampType;
    rc = NEXUS_Playback_SetSettings(playback, &playbackSettings);
    BDBG_ASSERT(!rc);

    NEXUS_Playback_GetDefaultPidChannelSettings(&pidSettings);
    pidSettings.pidSettings.pidType = NEXUS_PidType_eVideo;
    pidSettings.pidTypeSettings.video.codec = g_data.videoCodec;
    pidSettings.pidTypeSettings.video.decoder = videoDecoder;
    pidSettings.pidTypeSettings.video.index = true;
    startSettings.pidChannel = NEXUS_Playback_OpenPidChannel(playback, g_data.pid, &pidSettings);

    startSettings.stcChannel = NULL; /* TODO */
    startSettings.codec = g_data.videoCodec;

    rc = NEXUS_Playback_Start(playback, file, NULL);
    BDBG_ASSERT(!rc);

    rc = NEXUS_VideoDecoder_Start(videoDecoder, &startSettings);
    BDBG_ASSERT(!rc);

    return 0;
}

void thumbnail_stop_video(void)
{
    NEXUS_VideoDecoder_Stop(videoDecoder);
    NEXUS_Playback_Stop(playback);
    NEXUS_FilePlay_Close(file);
}

void thumbnail_scroll(int dir)
{
    BDBG_MSG(("scroll %d: %d, %d", dir, g_data.base_time, g_data.current_time));
    if (g_data.current_time < g_data.spacing && dir < 0) {
        return;
    }

    g_data.current_time += dir * g_data.spacing;

    if (g_data.current_time < g_data.base_time)
        g_data.base_time -= g_data.spacing;
    else if (g_data.current_time >= g_data.base_time + g_data.spacing * DISPLAYED_THUMBNAILS)
        g_data.base_time += g_data.spacing;

    thumbnail_show_stills();
}

void thumbnail_jump(void)
{
    NEXUS_Playback_Seek(playback, g_data.current_time * 1000);
}

int thumbnail_demo_run(void)
{
    bool done = false;

    thumbnail_play_video();

    thumbnail_decode_stills_init();

    g_data.base_time = 0;

    thumbnail_show_stills();

    printf(
    "\n"
    "\n"
    "Press LEFT/RIGHT on IR remote to scroll through thumbnails.\n"
    "Press SELECT to jump to the position.\n"
    "Press EXIT to quit.\n"
    );

    while (!done) {
        NEXUS_IrInputEvent event;
        unsigned num;
        bool overflow;
        NEXUS_Error rc;

        rc = BKNI_WaitForEvent(irInputEvent, 1000);
        if (rc) continue; /* on timeout, try for another still */

        NEXUS_IrInput_GetEvents(irInput, &event, 1, &num, &overflow);
        if (event.repeat) continue;

        switch (event.code) {
        case 0x6037: thumbnail_scroll(+1); break;
        case 0x7036: thumbnail_scroll(-1); break;
        case 0xe011: thumbnail_jump(); break;
        case 0xd012: done = true; break;
        }
    }

    thumbnail_stop_video();

    return 0;
}

