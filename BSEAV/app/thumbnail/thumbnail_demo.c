/******************************************************************************
 *    (c)2016 Broadcom Corporation
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
 *****************************************************************************/
#include "nexus_platform.h"
#include "nexus_display.h"
#include "nexus_video_window.h"
#include "nexus_component_output.h"
#include "nexus_video_decoder.h"
#include "nexus_surface.h"
#include "nexus_graphics2d.h"
#include "nexus_playpump.h"
#include "nexus_playback.h"
#include "nexus_types.h"
#include "nexus_video_types.h"
#include "nexus_core_utils.h"

#include "bfile_stdio.h"
#include "thumbnail.h"
#include "thumbdecoder.h"
#include "binput.h"
#include "bgui.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

BDBG_MODULE(thumbnail_demo);

static NEXUS_DisplayHandle display;
static NEXUS_VideoWindowHandle window;
static NEXUS_VideoDecoderHandle videoDecoder;
static NEXUS_PlaybackHandle playback;
static NEXUS_PlaypumpHandle playpump;
static NEXUS_FilePlayHandle file;
static binput_t binput;
static bgui_t gui;

#define THUMB_WIDTH 220
#define THUMB_HEIGHT 120
#define THUMB_GAP 10
#define THUMB_EDGE 50

static int thumbnail_border(NEXUS_SurfaceHandle stillSurface, unsigned color);

thumbnail_data g_data;

int thumbnail_demo_init(void)
{
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_DisplaySettings displaySettings;
    NEXUS_PlaybackSettings playbackSettings;
    NEXUS_VideoFormatInfo displayFormatInfo;
    struct bgui_settings gui_settings;

    NEXUS_Platform_GetConfiguration(&platformConfig);

    binput = binput_open(NULL);

    NEXUS_Display_GetDefaultSettings(&displaySettings);
    displaySettings.format = NEXUS_VideoFormat_e720p;
    display = NEXUS_Display_Open(0, &displaySettings);
    NEXUS_Display_AddOutput(display, NEXUS_ComponentOutput_GetConnector(platformConfig.outputs.component[0]));
    NEXUS_Display_AddOutput(display, NEXUS_HdmiOutput_GetVideoConnector(platformConfig.outputs.hdmi[0]));

    NEXUS_VideoFormat_GetInfo(displaySettings.format, &displayFormatInfo);

    window = NEXUS_VideoWindow_Open(display, 0);

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

    bgui_get_default_settings(&gui_settings);
    gui_settings.display = display;
    gui = bgui_create(&gui_settings);

    return 0;
}

void thumbnail_demo_uninit(void)
{
    unsigned i;
    binput_close(binput);
    NEXUS_VideoWindow_Close(window);
    NEXUS_Display_Close(display);
    NEXUS_Playback_Destroy(playback);
    NEXUS_Playpump_Close(playpump);
    NEXUS_VideoDecoder_Close(videoDecoder);
    for (i=0;i<DISPLAYED_THUMBNAILS;i++) {
        if (g_data.thumbnail[i].surface) {
            NEXUS_Surface_Destroy(g_data.thumbnail[i].surface);
        }
    }
    NEXUS_Playpump_Close(g_data.playpump);
    bgui_destroy(gui);
}

static thumbdecoder_t g_thumbdecoder;

static int thumbnail_show_stills(void)
{
    unsigned i;
    int rc;
    bgui_fill(gui, 0);
    for (i=0;i<DISPLAYED_THUMBNAILS;i++) {
        NEXUS_Rect rect;
        unsigned time;
        NEXUS_Graphics2DBlitSettings blitSettings;

        /* use i+1 because there is no thumbnail at time 0. */
        time = g_data.base_time + (i+1)*g_data.spacing;
        if (g_data.thumbnail[i].time != time ||
            !g_data.thumbnail[i].surface)
        {
            NEXUS_SurfaceCreateSettings surfaceCreateSettings;

            if (g_data.thumbnail[i].surface) {
                NEXUS_Surface_Destroy(g_data.thumbnail[i].surface);
                g_data.thumbnail[i].surface = NULL;
            }
            g_data.thumbnail[i].time = time;
            BDBG_MSG(("decode still[%d] from %d sec", i, time));

            NEXUS_Surface_GetDefaultCreateSettings(&surfaceCreateSettings);
            /* must be larger enough for <= 15x downscale from 4K resolution */
            surfaceCreateSettings.width = THUMB_WIDTH * 2;
            surfaceCreateSettings.height = THUMB_HEIGHT * 2;
            g_data.thumbnail[i].surface = NEXUS_Surface_Create(&surfaceCreateSettings);

            rc = thumbdecoder_decode_still(g_thumbdecoder, time*1000, g_data.thumbnail[i].surface);
            if (rc) return BERR_TRACE(rc);
            if (!g_data.thumbnail[i].surface) return BERR_TRACE(NEXUS_OUT_OF_DEVICE_MEMORY);
            BDBG_MSG(("got still[%d] from %d ms", i, time));
        }

        thumbnail_border(g_data.thumbnail[i].surface, time == g_data.current_time ? 0xFF00FF00 : 0xFF333333);

        rect.x = THUMB_EDGE + i*(THUMB_WIDTH + THUMB_GAP);
        rect.y = 550;
        rect.width = THUMB_WIDTH;
        rect.height = THUMB_HEIGHT;

        NEXUS_Graphics2D_GetDefaultBlitSettings(&blitSettings);
        blitSettings.colorOp = NEXUS_BlitColorOp_eCopySource;
        blitSettings.alphaOp = NEXUS_BlitAlphaOp_eCopyConstant; /* YCrCb has no alpha, so we must set 0xFF */
        blitSettings.constantColor = 0xFF000000; /* alpha is opaque */
        blitSettings.source.surface = g_data.thumbnail[i].surface;
        blitSettings.output.surface = bgui_surface(gui);
        blitSettings.output.rect = rect;
        rc = NEXUS_Graphics2D_Blit(bgui_blitter(gui), &blitSettings);
        if (rc) return BERR_TRACE(rc);
    }
    bgui_checkpoint(gui);
    bgui_submit(gui);
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
    NEXUS_VideoDecoderSettings videoDecoderSettings;

    file = NEXUS_FilePlay_OpenPosix(g_data.datafilename, g_data.indexfilename);
    if (!file) {
        fprintf(stderr, "can't open file:%s index:%s\n", g_data.datafilename, g_data.indexfilename);
        return -1;
    }

    NEXUS_VideoDecoder_GetSettings(videoDecoder, &videoDecoderSettings);
    videoDecoderSettings.maxWidth = g_data.probe_results.video[0].width;
    videoDecoderSettings.maxHeight = g_data.probe_results.video[0].height;
    rc = NEXUS_VideoDecoder_SetSettings(videoDecoder, &videoDecoderSettings);
    if (rc) return BERR_TRACE(rc);

    rc = NEXUS_VideoWindow_AddInput(window, NEXUS_VideoDecoder_GetConnector(videoDecoder));
    if (rc) return BERR_TRACE(rc);

    NEXUS_VideoDecoder_GetDefaultStartSettings(&startSettings);

    NEXUS_Playback_GetSettings(playback, &playbackSettings);
    playbackSettings.playpumpSettings.transportType = g_data.probe_results.transportType;
    playbackSettings.playpumpSettings.timestamp.type = g_data.probe_results.timestampType;
    rc = NEXUS_Playback_SetSettings(playback, &playbackSettings);
    BDBG_ASSERT(!rc);

    NEXUS_Playback_GetDefaultPidChannelSettings(&pidSettings);
    pidSettings.pidSettings.pidType = NEXUS_PidType_eVideo;
    pidSettings.pidTypeSettings.video.codec = g_data.probe_results.video[0].codec;
    pidSettings.pidTypeSettings.video.decoder = videoDecoder;
    pidSettings.pidTypeSettings.video.index = true;
    startSettings.pidChannel = NEXUS_Playback_OpenPidChannel(playback, g_data.probe_results.video[0].pid, &pidSettings);

    startSettings.stcChannel = NULL; /* TODO */
    startSettings.codec = g_data.probe_results.video[0].codec;

    rc = NEXUS_Playback_Start(playback, file, NULL);
    if (rc) return BERR_TRACE(rc);

    rc = NEXUS_VideoDecoder_Start(videoDecoder, &startSettings);
    if (rc) return BERR_TRACE(rc);

    return 0;
}

void thumbnail_stop_video(void)
{
    NEXUS_VideoWindow_RemoveAllInputs(window);
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
    int rc;

    thumbnail_play_video();

    g_thumbdecoder = thumbdecoder_open();
    rc = thumbdecoder_open_file(g_thumbdecoder, g_data.datafilename, g_data.indexfilename);
    if (rc) return BERR_TRACE(rc);

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
        b_remote_key key;
        NEXUS_Error rc;

        rc = binput_wait(binput, 1000);
        if (rc) continue; /* on timeout, try for another still */

        binput_read_no_repeat(binput, &key);

        switch (key) {
        case b_remote_key_right: thumbnail_scroll(+1); break;
        case b_remote_key_left: thumbnail_scroll(-1); break;
        case b_remote_key_select: thumbnail_jump(); break;
        case b_remote_key_stop:
        case b_remote_key_clear:
        case b_remote_key_back:
            done = true;
            break;
        default:
            break;
        }
    }

    thumbdecoder_close(g_thumbdecoder);
    thumbnail_stop_video();

    return 0;
}
