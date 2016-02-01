/******************************************************************************
 *    (c)2008-2012 Broadcom Corporation
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
#include <stdio.h>
#if NEXUS_HAS_DISPLAY
#include "nexus_types.h"
#include "nexus_platform.h"
#include "nexus_display.h"
#include "nexus_composite_output.h"
#include "nexus_component_output.h"
#if NEXUS_HAS_HDMI_OUTPUT
#include "nexus_hdmi_output.h"
#endif
#include "nexus_core_utils.h"
#include "nexus_graphics2d.h"
#include <string.h>
#include <stdlib.h>

BDBG_MODULE(multibuffering);

struct compositor {
    struct {
        NEXUS_SurfaceHandle surface;
        NEXUS_SurfaceMemory mem;
    } framebuffer[3]; /* app supports double or triple buffering */
    BKNI_EventHandle framebufferEvent;
    unsigned number; /* 2 or 3 */
    unsigned current; /* framebuffer[current] has just been submitted to nexus */
    NEXUS_Rect boxrect, fullscreen_rect;
};

static void framebuffer_callback(void *fb, int unused)
{
    struct compositor *compositor = fb;
    BSTD_UNUSED(unused);
    BKNI_SetEvent(compositor->framebufferEvent);
}

void checkpoint_callback(void *data, int unused)
{
    BSTD_UNUSED(unused);
    BKNI_SetEvent((BKNI_EventHandle)data);
}

static void
simple_fill(NEXUS_Graphics2DHandle gfx, NEXUS_SurfaceHandle surface, const NEXUS_Rect *rect, NEXUS_Pixel pixel)
{
    NEXUS_Graphics2DFillSettings fillSettings;
    int rc;

    NEXUS_Graphics2D_GetDefaultFillSettings(&fillSettings);
    fillSettings.surface = surface;
    fillSettings.rect = *rect;
    fillSettings.color = pixel;
    rc = NEXUS_Graphics2D_Fill(gfx, &fillSettings);
    BDBG_ASSERT(!rc);
    /* no check point here. you don't want to do a checkpoint after every blit. only do it when you're about
    to do a CPU draw or about to set the framebuffer. */
}

static void
simple_cpufill(const NEXUS_SurfaceMemory *compositorBufferMem, const NEXUS_Rect *rect, NEXUS_Pixel pixel)
{
    unsigned x,y;
    unsigned width = rect->width;
    /* 32 bpp algo */
    for(y=0;y<rect->height;y++) {
        uint32_t *buf = (uint32_t *)((uint8_t *)compositorBufferMem->buffer + compositorBufferMem->pitch*(y+rect->y))+rect->x;
        for(x=0;x<width;x++) {
            buf[x] = pixel;
        }
    }
    /* don't flush here. you don't want to flush after every cpu draw. only do it when you're about
    to blit or about to set the framebuffer. */
}

#include <sys/time.h>
static unsigned b_get_time(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec*1000 + tv.tv_usec/1000;
}

int main(int argc, char **argv)
{
    NEXUS_Error errCode;
    NEXUS_DisplayHandle display;
    NEXUS_DisplaySettings displaySettings;
    NEXUS_GraphicsSettings graphicsCfg;
    NEXUS_SurfaceCreateSettings surfaceCfg;
    NEXUS_PlatformSettings platformSettings;
    NEXUS_PlatformConfiguration platformConfig;
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_HdmiOutputStatus hdmiStatus;
#endif
    struct compositor compositor;
    unsigned i;
    NEXUS_Graphics2DHandle gfx;
    NEXUS_Graphics2DSettings graphics2DSettings;
    BKNI_EventHandle checkpointEvent;
    unsigned cnt = 0;
    int curarg = 1;
    unsigned rendertime = 0; /* milliesconds. 0 = render as fast as possible */
    unsigned starttime, endtime;

    compositor.current = 0;
    compositor.number = 2; /* default to double buffering */

    /* interesting combinations are:
    1) double buffering, fast rendering. this can do 60fps multibuffering.
        OPTIONS: <none>
    2) double buffering, slow rendering. this can do 30fps multibuffering.
        OPTIONS: -rendertime 16
    3) triple buffering, slow rendering. this can do 60fps multibuffering.
        OPTIONS: -rendertime 16 -triple
    4) triple buffering, very slow rendering. this is best-effort multibuffering.
        OPTIONS: -rendertime 50 -triple
    */
    while (curarg < argc) {
        if (!strcmp(argv[curarg], "-triple")) {
            /* triple buffering instead of double buffering */
            compositor.number = 3;
        }
        else if (!strcmp(argv[curarg], "-rendertime") && curarg+1 < argc) {
            /* add delay to simulate slower (more complex) rendering times */
            rendertime = atoi(argv[++curarg]);
        }
        curarg++;
    }

    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    errCode = NEXUS_Platform_Init(&platformSettings);
    if ( errCode ) {
        return -1;
    }
    NEXUS_Platform_GetConfiguration(&platformConfig);

    NEXUS_Display_GetDefaultSettings(&displaySettings);
    displaySettings.displayType = NEXUS_DisplayType_eAuto;
    displaySettings.format = NEXUS_VideoFormat_eNtsc;
    display = NEXUS_Display_Open(0, &displaySettings);

#if NEXUS_NUM_COMPONENT_OUTPUTS
    NEXUS_Display_AddOutput(display, NEXUS_ComponentOutput_GetConnector(platformConfig.outputs.component[0]));
#endif
#if NEXUS_NUM_COMPOSITE_OUTPUTS
    NEXUS_Display_AddOutput(display, NEXUS_CompositeOutput_GetConnector(platformConfig.outputs.composite[0]));
#endif
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_Display_AddOutput(display, NEXUS_HdmiOutput_GetVideoConnector(platformConfig.outputs.hdmi[0]));
    errCode = NEXUS_HdmiOutput_GetStatus(platformConfig.outputs.hdmi[0], &hdmiStatus);
    if ( !errCode && hdmiStatus.connected )
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

    gfx = NEXUS_Graphics2D_Open(0, NULL);
    BKNI_CreateEvent(&checkpointEvent);
    NEXUS_Graphics2D_GetSettings(gfx, &graphics2DSettings);
    graphics2DSettings.checkpointCallback.callback = checkpoint_callback;
    graphics2DSettings.checkpointCallback.context = checkpointEvent;
    NEXUS_Graphics2D_SetSettings(gfx, &graphics2DSettings);

    BKNI_CreateEvent(&compositor.framebufferEvent);
    NEXUS_Display_GetGraphicsSettings(display, &graphicsCfg);
    graphicsCfg.frameBufferCallback.callback = framebuffer_callback;
    graphicsCfg.frameBufferCallback.context = &compositor;
    errCode = NEXUS_Display_SetGraphicsSettings(display, &graphicsCfg);
    BDBG_ASSERT(!errCode);

    compositor.fullscreen_rect.x = 0;
    compositor.fullscreen_rect.y = 0;
    compositor.fullscreen_rect.width = graphicsCfg.clip.width;
    compositor.fullscreen_rect.height = graphicsCfg.clip.height;
    compositor.boxrect.x = 0;
    compositor.boxrect.y = 0;
    compositor.boxrect.width = 100;
    compositor.boxrect.height = 100;

    for(i=0;i<compositor.number;i++) {
        NEXUS_Surface_GetDefaultCreateSettings(&surfaceCfg);
        surfaceCfg.width = compositor.fullscreen_rect.width;
        surfaceCfg.height = compositor.fullscreen_rect.height;
        surfaceCfg.heap = NEXUS_Platform_GetFramebufferHeap(0); /* gets the heap that is accessible by display 0's GFD */
        compositor.framebuffer[i].surface = NEXUS_Surface_Create(&surfaceCfg);
        BDBG_ASSERT(compositor.framebuffer[i].surface);

        /* cache the surface's mem information. it is unchanging. */
        NEXUS_Surface_GetMemory(compositor.framebuffer[i].surface, &compositor.framebuffer[i].mem);
    }

    BDBG_WRN(("start: %s buffering, %d millisecond render time", compositor.number == 2 ? "double":"triple", rendertime));
    starttime = b_get_time();
    while (1) {
        NEXUS_Rect inside_boxrect;
        int rc;
        unsigned start_rendertime = b_get_time();;

        /* next framebuffer */
        if (++compositor.current == compositor.number) {
            compositor.current = 0;
        }

        /* two blits */
        /* 1. full screen black */
        simple_fill(gfx, compositor.framebuffer[compositor.current].surface, &compositor.fullscreen_rect, 0);
        /* 2. smoothly move the blue box using M2MC */
        if (++compositor.boxrect.x + compositor.boxrect.width == compositor.fullscreen_rect.width) {
            compositor.boxrect.x = 0;
        }
        if (++compositor.boxrect.y + compositor.boxrect.height == compositor.fullscreen_rect.height) {
            compositor.boxrect.y = 0;
        }
        simple_fill(gfx, compositor.framebuffer[compositor.current].surface, &compositor.boxrect, 0xFF0000FF);
        /* checkpoint before doing CPU draw (or NEXUS_Display_SetGraphicsFramebuffer) */
        rc = NEXUS_Graphics2D_Checkpoint(gfx, NULL);
        if (rc == NEXUS_GRAPHICS2D_QUEUED) {
            BKNI_WaitForEvent(checkpointEvent, 0xffffffff);
        }

        /* 3. draw a little green box inside the blue box using the CPU */
        inside_boxrect = compositor.boxrect;
        inside_boxrect.x += inside_boxrect.width/4;
        inside_boxrect.y += inside_boxrect.height/4;
        inside_boxrect.width /= 2;
        inside_boxrect.height /= 2;
        simple_cpufill(&compositor.framebuffer[compositor.current].mem, &inside_boxrect, 0xFF00FF00);
        /* flush required before NEXUS_Display_SetGraphicsFramebuffer (or blit) */
        NEXUS_Surface_Flush(compositor.framebuffer[compositor.current].surface);

        if (rendertime) {
            unsigned now = b_get_time();
            unsigned diff = (now>start_rendertime)?(now-start_rendertime):(start_rendertime-now);
            if (diff < rendertime) {
                /* sleep for the additional time needed to hit the 'rendertime' */
                BKNI_Sleep(rendertime - diff);
            }
        }

        if (cnt >= 1 && compositor.number == 3) {
            /* if triple buffering, don't wait for the framebuffer event until after you have rendered.
            but we must wait for the last call to complete because we don't want to overwrite the
            previous NEXUS_Display_SetGraphicsFramebuffer (nexus does not maintain a queue). */
            BKNI_WaitForEvent(compositor.framebufferEvent, BKNI_INFINITE);
        }

        errCode = NEXUS_Display_SetGraphicsFramebuffer(display, compositor.framebuffer[compositor.current].surface);
        BDBG_ASSERT(!errCode);

        if (compositor.number == 2) {
            /* if double buffering, you must wait for the framebuffer event before rendering
            into a previously set framebuffer */
            BKNI_WaitForEvent(compositor.framebufferEvent, BKNI_INFINITE);
        }

        /* to compute framerate, wait for 100 flips, then subtract time from "start" time. compute 1000 divided by that time. */
        if (++cnt % 100 == 0) {
            unsigned framerate;
            
            endtime = b_get_time();
            framerate = (1000*100*100) / (endtime - starttime);

            BDBG_WRN(("%d flips, rate=%d.%02d fps", cnt, framerate/100, framerate%100));
            starttime = endtime;
        }
    }

    return 0;
}
#else
int main(void)
{
    printf("This application is not supported on this platform\n");
    return 0;
}
#endif
