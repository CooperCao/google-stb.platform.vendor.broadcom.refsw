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
#include "nexus_types.h"
#include "nexus_display.h"
#include "nexus_core_utils.h"
#include "nexus_component_output.h"
#include "nexus_composite_output.h"
#if NEXUS_HAS_HDMI_OUTPUT
#include "nexus_hdmi_output.h"
#endif
#include "nexus_video_output.h"
#include "nexus_video_window.h"
#include "nexus_video_image_input.h"
#include "nexus_video_input.h"
#include "nexus_graphics2d.h"
#include <stdlib.h>

BDBG_MODULE(linux_twoapp);

/**
This app is deprecated.
It requires verified mode because it shares the display handle between processes. this is not recommended.
It also requires kernel mode because it launches the same app binary as server and client, which is only
possible in kernel mode.
**/

static void complete(void *data, int unused)
{
    BSTD_UNUSED(unused);
    BKNI_SetEvent((BKNI_EventHandle)data);
}

static void rect_move(const NEXUS_Rect *disp, const NEXUS_Rect *size, NEXUS_Rect *rect, int x, int y)
{
    int next;

    next = rect->x + x;

    if(next>=0) {
        if((next+size->width)<=disp->width) {
            rect->x = next;
            rect->width = size->width;
        } else {
            if(next<=disp->width) {
                rect->x = next;
                rect->width = disp->width - next;
            } else {
                rect->x = next%disp->width;
                rect->width = size->width;
            }
        }
    } else /* if(next < 0) */ {
        if(rect->width > -next) {
            rect->x = 0;
            rect->width += next;
        } else {
            rect->x = disp->width-size->width;
            rect->width = size->width;
        }
    }
    next = rect->y + y;

    if(next>=0) {
        if((next+size->height)<=disp->height) {
            rect->y = next;
            rect->height = size->height;
        } else {
            if(next<=disp->height) {
                rect->y = next;
                rect->height = disp->height - next;
            } else {
                rect->y = next%disp->height;
                rect->height = size->height;
            }
        }
    } else /* if(next < 0) */ {
        if(rect->height> -next) {
            rect->y = 0;
            rect->height += next;
        } else {
            rect->y = disp->height-size->height;
            rect->height= size->height;
        }
    }
}

struct compositor {
    NEXUS_Rect region;
    NEXUS_Pixel background;
    NEXUS_DisplayHandle display;
    struct {
        NEXUS_SurfaceHandle frame;
    } buffers[2];
    struct {
        NEXUS_Rect prev[2];
        NEXUS_Rect rect;
        NEXUS_Rect size;
        int stepX, stepY;
        NEXUS_Pixel color;
    } rects[5];
    unsigned current;
};

static NEXUS_Graphics2DHandle gfx = NULL;
static BKNI_EventHandle checkpointEvent = NULL;

static void
simple_fill(NEXUS_SurfaceHandle surface, const NEXUS_Rect *rect, NEXUS_Pixel pixel)
{
    NEXUS_Graphics2DFillSettings fillSettings;
    int rc;

    /* must use blitter because CPU draw doesn't work in non-UMA systems */
    NEXUS_Graphics2D_GetDefaultFillSettings(&fillSettings);
    fillSettings.surface = surface;
    fillSettings.rect = *rect;
    fillSettings.color = pixel;
    rc = NEXUS_Graphics2D_Fill(gfx, &fillSettings);
    BDBG_ASSERT(!rc);
}

static void
checkpoint(NEXUS_Graphics2DHandle gfx, BKNI_EventHandle checkpointEvent)
{
    int rc;
    rc = NEXUS_Graphics2D_Checkpoint(gfx, NULL);
    if (rc == NEXUS_GRAPHICS2D_QUEUED) {
        rc = BKNI_WaitForEvent(checkpointEvent, BKNI_INFINITE);
    }
    BDBG_ASSERT(!rc);
}

static void
compositorbuffer_move(struct compositor *compositor, unsigned buffer)
{
    unsigned i;
    for(i=0;i<sizeof(compositor->rects)/sizeof(*compositor->rects);i++) {
        simple_fill(compositor->buffers[buffer].frame, &compositor->rects[i].prev[buffer], compositor->background);
    }
    for(i=0;i<sizeof(compositor->rects)/sizeof(*compositor->rects);i++) {
        rect_move(&compositor->region, &compositor->rects[i].size, &compositor->rects[i].rect, compositor->rects[i].stepX, compositor->rects[i].stepY);
        compositor->rects[i].prev[buffer] = compositor->rects[i].rect;
        simple_fill(compositor->buffers[buffer].frame, &compositor->rects[i].rect, compositor->rects[i].color);
    }
    return;
}

static void
compositorbuffer_next(void *fb, int unused)
{
    struct compositor *compositor = fb;
    NEXUS_Error errCode;
    unsigned current;

    BSTD_UNUSED(unused);

    if(compositor->current>=sizeof(compositor->buffers)/sizeof(*compositor->buffers)) {
        compositor->current = 0;
    }
    current = compositor->current;
    compositor->current++;
    compositorbuffer_move(compositor, current);
    /* checkpoint required before submitting surfaces to HW */
    checkpoint(gfx, checkpointEvent);

    BDBG_MSG(("set fb[%d] %p", current, (void*)compositor->buffers[current].frame));
    errCode = NEXUS_Display_SetGraphicsFramebuffer(compositor->display, compositor->buffers[current].frame);
    BDBG_ASSERT(!errCode);
    return;
}


int graphics_process(NEXUS_DisplayHandle display)
{
    NEXUS_Error errCode;
    NEXUS_GraphicsSettings graphicsCfg;
    NEXUS_SurfaceCreateSettings surfaceCfg;
    struct compositor compositor;
    unsigned i;
    NEXUS_Graphics2DSettings gfxSettings;
    int rc;

    errCode = NEXUS_Platform_Join();
    if ( errCode ) {
        fprintf(stderr, "Unable to initialize nexus, err %d\n", errCode);
        return -1;
    }

    gfx = NEXUS_Graphics2D_Open(0, NULL);
    BDBG_ASSERT(gfx);
    rc = BKNI_CreateEvent(&checkpointEvent);
    BDBG_ASSERT(!rc);
    NEXUS_Graphics2D_GetSettings(gfx, &gfxSettings);
    gfxSettings.checkpointCallback.callback = complete;
    gfxSettings.checkpointCallback.context = checkpointEvent;
    rc = NEXUS_Graphics2D_SetSettings(gfx, &gfxSettings);
    BDBG_ASSERT(!rc);

    NEXUS_Display_GetGraphicsSettings(display, &graphicsCfg);
#if 0
/* don't do gfx in the framebuffer callback because we must wait for checkpoint. the blit can be done, but not the waitforevent. */
    graphicsCfg.frameBufferCallback.callback = compositorbuffer_next;
    graphicsCfg.frameBufferCallback.context = &compositor;
#endif
    graphicsCfg.enabled = true;
    rc = NEXUS_Display_SetGraphicsSettings(display, &graphicsCfg);
    BDBG_ASSERT(!rc);
    for(i=0;i<sizeof(compositor.buffers)/sizeof(*compositor.buffers);i++) {
        NEXUS_Surface_GetDefaultCreateSettings(&surfaceCfg);
        surfaceCfg.width = graphicsCfg.clip.width;
        surfaceCfg.height = graphicsCfg.clip.height;
        surfaceCfg.heap = NEXUS_Platform_GetFramebufferHeap(0);
        compositor.buffers[i].frame = NEXUS_Surface_Create(&surfaceCfg);
        if(!compositor.buffers[i].frame ) {
            fprintf(stderr, "Can't create frame buffer\n");
            return -1;
        }
        simple_fill(compositor.buffers[i].frame, &graphicsCfg.clip, 0);
    }
    compositor.region = graphicsCfg.clip;
    compositor.background = 0;
    compositor.rects[0].size.x = 0;
    compositor.rects[0].size.y = 0;
    compositor.rects[0].size.width = 300;
    compositor.rects[0].size.height = 240;
    compositor.rects[0].rect = compositor.rects[0].size;
    for(i=0;i<sizeof(compositor.buffers)/sizeof(*compositor.buffers);i++) {
        compositor.rects[0].prev[i] = compositor.rects[0].size;
    }
    compositor.rects[0].stepX = 1;
    compositor.rects[0].stepY = 1;
    compositor.rects[0].color = 0x7FFFFF00;

    compositor.rects[1].size.x = 0;
    compositor.rects[1].size.y = 0;
    compositor.rects[1].size.width = 50;
    compositor.rects[1].size.height = 40;
    compositor.rects[1].rect = compositor.rects[1].size;
    for(i=0;i<sizeof(compositor.buffers)/sizeof(*compositor.buffers);i++) {
        compositor.rects[1].prev[i] = compositor.rects[1].size;
    }
    compositor.rects[1].stepX = -8;
    compositor.rects[1].stepY = -4;
    compositor.rects[1].color = 0x7F00FF00;

    compositor.rects[2].size.x = 0;
    compositor.rects[2].size.y = 200;
    compositor.rects[2].size.width = 70;
    compositor.rects[2].size.height = 50;
    compositor.rects[2].rect = compositor.rects[2].size;
    for(i=0;i<sizeof(compositor.buffers)/sizeof(*compositor.buffers);i++) {
        compositor.rects[2].prev[i] = compositor.rects[2].size;
    }
    compositor.rects[2].stepX = -8;
    compositor.rects[2].stepY = 7;
    compositor.rects[2].color = 0x7F404040;

    compositor.rects[3].size.x = 400;
    compositor.rects[3].size.y = 0;
    compositor.rects[3].size.width = 120;
    compositor.rects[3].size.height = 100;
    compositor.rects[3].rect = compositor.rects[3].size;
    for(i=0;i<sizeof(compositor.buffers)/sizeof(*compositor.buffers);i++) {
        compositor.rects[3].prev[i] = compositor.rects[3].size;
    }
    compositor.rects[3].stepX = 8;
    compositor.rects[3].stepY = -4;
    compositor.rects[3].color = 0x7F0000FF;

    compositor.rects[4].size.x = 0;
    compositor.rects[4].size.y = 0;
    compositor.rects[4].size.width = 100;
    compositor.rects[4].size.height = 80;
    compositor.rects[4].rect = compositor.rects[0].size;
    for(i=0;i<sizeof(compositor.buffers)/sizeof(*compositor.buffers);i++) {
        compositor.rects[4].prev[i] = compositor.rects[4].size;
    }
    compositor.rects[4].stepX = 5;
    compositor.rects[4].stepY = 3;
    compositor.rects[4].color = 0x7FFF0000;

    compositor.current = 0;
    compositor.display = display;

    checkpoint(gfx, checkpointEvent);

    BDBG_MSG(("set fb[%d] %p", 0, (void*)compositor.buffers[0].frame));
    errCode = NEXUS_Display_SetGraphicsFramebuffer(display, compositor.buffers[0].frame);
    BDBG_ASSERT(!errCode);

#if 0
    /* this would require more app rework */
    fprintf(stderr, "Press any key to exit from graphics app....\n");
    getchar();
#else
    while (1) {
        BKNI_Sleep(30); /* poor man's framebuffer callback */
        compositorbuffer_next(&compositor, 0);
    }
#endif

    NEXUS_Display_GetGraphicsSettings(display, &graphicsCfg);
    graphicsCfg.frameBufferCallback.callback = NULL;
    graphicsCfg.frameBufferCallback.context = NULL;
    graphicsCfg.enabled = false;
    NEXUS_Display_SetGraphicsSettings(display, &graphicsCfg);

    for(i=0;i<sizeof(compositor.buffers)/sizeof(*compositor.buffers);i++) {
        NEXUS_Surface_Destroy(compositor.buffers[i].frame);
    }
    NEXUS_Platform_Uninit();

    return 0;
}

int video_process(const char *cmd)
{
    NEXUS_Error errCode;
    NEXUS_DisplayHandle display;
    NEXUS_VideoWindowHandle window;
    NEXUS_VideoWindowSettings windowCfg;
    NEXUS_SurfaceHandle img[2]; /* use to surfaces for double buffering */
    NEXUS_SurfaceCreateSettings surfaceCfg;
    NEXUS_Rect fillRect;
    NEXUS_PlatformSettings platformSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_VideoImageInputHandle imageInput;
    NEXUS_VideoInput videoInput;
    NEXUS_DisplaySettings displaySettings;
    NEXUS_PlatformStartServerSettings serverSettings;
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_HdmiOutputStatus hdmiStatus;
#endif
    unsigned i;
    NEXUS_Graphics2DSettings gfxSettings;
    int rc;

    fprintf(stderr, "Ready to initialize\n");

    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    errCode = NEXUS_Platform_Init(&platformSettings);
    if ( errCode ) {
        fprintf(stderr, "Unable to initialize nexus, err %d\n", errCode);
        return -1;
    }

    NEXUS_Platform_GetConfiguration(&platformConfig);

    gfx = NEXUS_Graphics2D_Open(0, NULL);
    BDBG_ASSERT(gfx);
    rc = BKNI_CreateEvent(&checkpointEvent);
    BDBG_ASSERT(!rc);
    NEXUS_Graphics2D_GetSettings(gfx, &gfxSettings);
    gfxSettings.checkpointCallback.callback = complete;
    gfxSettings.checkpointCallback.context = checkpointEvent;
    rc = NEXUS_Graphics2D_SetSettings(gfx, &gfxSettings);
    BDBG_ASSERT(!rc);

    NEXUS_Platform_GetDefaultStartServerSettings(&serverSettings);
    serverSettings.allowUnauthenticatedClients = true;
    serverSettings.unauthenticatedConfiguration.mode = NEXUS_ClientMode_eVerified;
    errCode = NEXUS_Platform_StartServer(&serverSettings);
    BDBG_ASSERT(!errCode);

    NEXUS_Display_GetDefaultSettings(&displaySettings);
    displaySettings.displayType = NEXUS_DisplayType_eAuto;
    displaySettings.format = NEXUS_VideoFormat_eNtsc;
    display = NEXUS_Display_Open(0, &displaySettings);

#if NEXUS_NUM_COMPONENT_OUTPUTS
    errCode = NEXUS_Display_AddOutput(display, NEXUS_ComponentOutput_GetConnector(platformConfig.outputs.component[0]));
    BDBG_ASSERT(!errCode);
#endif
#if NEXUS_NUM_COMPOSITE_OUTPUTS
    errCode = NEXUS_Display_AddOutput(display, NEXUS_CompositeOutput_GetConnector(platformConfig.outputs.composite[0]));
    BDBG_ASSERT(!errCode);
#endif
#if NEXUS_NUM_HDMI_OUTPUTS
    errCode = NEXUS_Display_AddOutput(display, NEXUS_HdmiOutput_GetVideoConnector(platformConfig.outputs.hdmi[0]));
    BDBG_ASSERT(!errCode);
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

    window = NEXUS_VideoWindow_Open(display, 0);
    NEXUS_VideoWindow_GetSettings(window, &windowCfg);
    windowCfg.position.width/=2;
    windowCfg.position.height/=2;
    windowCfg.visible = true;
    errCode = NEXUS_VideoWindow_SetSettings(window, &windowCfg);
    BDBG_ASSERT(!errCode);

    NEXUS_Surface_GetDefaultCreateSettings(&surfaceCfg);
    surfaceCfg.width = 720;
    surfaceCfg.height = 480;
    surfaceCfg.pixelFormat = NEXUS_PixelFormat_eCr8_Y18_Cb8_Y08;
    /* the image input surface must be from the MEMC that MFD has access too.
    image input 0 uses MFD0. we have to use mfdMapping[] to learn which AVD, then learn the heap for that AVD.
    NEXUS_VideoDecoderModuleInternalSettings has that information after NEXUS_Platform_Init completes.
    hence the multiple levels of indirection. */
    NEXUS_Platform_GetSettings(&platformSettings);
    surfaceCfg.heap = platformConfig.heap[platformSettings.videoDecoderModuleSettings.avdHeapIndex[platformSettings.videoDecoderModuleSettings.mfdMapping[0]]];
    img[0] = NEXUS_Surface_Create(&surfaceCfg);
    img[1] = NEXUS_Surface_Create(&surfaceCfg);
    BDBG_ASSERT(img[0]);
    BDBG_ASSERT(img[1]);

    imageInput = NEXUS_VideoImageInput_Open(0, NULL);
    BDBG_ASSERT(imageInput);
    videoInput = NEXUS_VideoImageInput_GetConnector(imageInput);

    NEXUS_Surface_GetCreateSettings(img[0], &surfaceCfg);
    fillRect.width = surfaceCfg.width;
    fillRect.height = surfaceCfg.height;
    fillRect.x = 0;
    fillRect.y = 0;
    simple_fill(img[0], &fillRect, 0xFFFF8080); /* AYCrCb white */
    simple_fill(img[1], &fillRect, 0xFFFF8080); /* AYCrCb white */

    /* now fill left half w/ changing color */
    fillRect.width /= 2;

    errCode = NEXUS_VideoWindow_AddInput(window, videoInput);
    BDBG_ASSERT(!errCode);

#if 1
    fprintf(stderr, "Press Ctrl-C to exit from main app....\n");
    for(i=0;;i++) {
#else
    fprintf(stderr, "Main app will run for around 10 seconds....\n");
    for(i=0;i<100;i++) {
#endif
        NEXUS_Pixel luma=(i*8)&0xFF;
        NEXUS_SurfaceHandle pic = img[i%2];

        simple_fill(pic, &fillRect, 0x80008000 |luma|(luma<<16) ); /* shade of gray */
        checkpoint(gfx, checkpointEvent);

        BDBG_MSG(("%d: set pic %p", i, (void*)pic));
        errCode = NEXUS_VideoImageInput_SetSurface(imageInput, pic);
        BDBG_ASSERT(!errCode);

        NEXUS_VideoWindow_GetSettings(window, &windowCfg);
        windowCfg.position.x=(i*8)%windowCfg.position.width;
        windowCfg.position.y=(i*4)%windowCfg.position.height;
        errCode = NEXUS_VideoWindow_SetSettings(window, &windowCfg);
        BDBG_ASSERT(!errCode);

        BKNI_Sleep(130); /* simplistic flow control */
        if(i==10) {
            char args[128];
            snprintf(args, sizeof(args), "%s %#lx", cmd, (unsigned long)display);
            fprintf(stderr, "starting second app '%s'\n", args);
            popen(args, "r");
        } else if(i>10000) {
            i=20;
        }
    }

    NEXUS_VideoImageInput_SetSurface(imageInput, NULL);
    NEXUS_VideoWindow_Close(window);
    NEXUS_VideoImageInput_Close(imageInput);
    NEXUS_Display_Close(display);
    NEXUS_Surface_Destroy(img[0]);
    NEXUS_Surface_Destroy(img[1]);
    NEXUS_Platform_Uninit();

    return 0;
}

/* Usage: launch linux_twoapp with no parameters. it will internally launch a second process as a client. */
int main(int argc, const char *argv[]) {
    if(argc<2) {
        return video_process(argv[0]);
    } else {
        return graphics_process((void *)strtoul(argv[1], NULL, 0));
    }
}
#else
int main(void)
{
    printf("This application is not supported on this platform (needs video display)!\n");
    return 0;
}
#endif

