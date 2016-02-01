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
******************************************************************************/
#include "nexus_platform.h"
#include <stdio.h>

#if NEXUS_HAS_DISPLAY
#include "nexus_types.h"
#include "nexus_display.h"
#include "nexus_composite_output.h"
#include "nexus_component_output.h"
#if NEXUS_HAS_HDMI_OUTPUT
#include "nexus_hdmi_output.h"
#endif
#include "nexus_core_utils.h"

BDBG_MODULE(framebuffer);

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
        NEXUS_SurfaceMemory mem;
    } buffers[2]; /* double-buffered system. because render time is far less than vsync time,
                     we can achieve 60fps updates */
    struct {
        NEXUS_Rect prev[2];
        NEXUS_Rect rect;
        NEXUS_Rect size;
        int stepX, stepY;
        NEXUS_Pixel color;
    } rects[5];
    unsigned current;
};


static void
simple_fill(const NEXUS_SurfaceMemory *compositorBufferMem, const NEXUS_Rect *rect, NEXUS_Pixel pixel)
{
    unsigned x,y;
    unsigned width = rect->width;
    for(y=0;y<rect->height;y++) {
        uint32_t *buf = (uint32_t *)((uint8_t *)compositorBufferMem->buffer + compositorBufferMem->pitch*(y+rect->y))+rect->x;
        for(x=0;x<width;x++) {
            buf[x] = pixel;
        }
    }
}


static void
compositorbuffer_move(struct compositor *compositor, unsigned buffer)
{
    unsigned i;
    for(i=0;i<sizeof(compositor->rects)/sizeof(*compositor->rects);i++) {
        simple_fill(&compositor->buffers[buffer].mem, &compositor->rects[i].prev[buffer], compositor->background);
    }
    for(i=0;i<sizeof(compositor->rects)/sizeof(*compositor->rects);i++) {
        rect_move(&compositor->region, &compositor->rects[i].size, &compositor->rects[i].rect, compositor->rects[i].stepX, compositor->rects[i].stepY);
        compositor->rects[i].prev[buffer] = compositor->rects[i].rect;
        simple_fill(&compositor->buffers[buffer].mem, &compositor->rects[i].rect, compositor->rects[i].color);
    }
    NEXUS_Surface_Flush(compositor->buffers[buffer].frame);
    return;
}

static void
compositorbuffer_next(void *fb, int unused)
{
    struct compositor *compositor = fb;
    NEXUS_Error errCode;
    unsigned current;
    static unsigned cnt = 0;

    BSTD_UNUSED(unused);

    if(compositor->current>=sizeof(compositor->buffers)/sizeof(*compositor->buffers)) {
        compositor->current = 0;
    }
    current = compositor->current;
    compositor->current++;
    compositorbuffer_move(compositor, current);

    /* By calling NEXUS_Display_SetGraphicsFramebuffer directly from the framebufferCallback, we avoid an extra
    context switch in the application. this provides a small performance improvement.
    however, you should not wait on a NEXUS_Graphics2D checkpoint event here. If your app must wait on a checkpoint before
    flipping, you need to set an event here. */
    errCode = NEXUS_Display_SetGraphicsFramebuffer(compositor->display, compositor->buffers[current].frame);
    BDBG_ASSERT(!errCode);

    if (++cnt % 100 == 0) BDBG_WRN(("%d flips", cnt));
}


int main(void)
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

    fprintf(stderr, "Ready to initialize\n");

    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    errCode = NEXUS_Platform_Init(&platformSettings);
    if ( errCode ) {
        fprintf(stderr, "Unable to initialize nexus, err %d\n", errCode);
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

    NEXUS_Display_GetGraphicsSettings(display, &graphicsCfg);
    graphicsCfg.frameBufferCallback.callback = compositorbuffer_next;
    graphicsCfg.frameBufferCallback.context = &compositor;
    errCode = NEXUS_Display_SetGraphicsSettings(display, &graphicsCfg);
    BDBG_ASSERT(!errCode);

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
        NEXUS_Surface_GetMemory(compositor.buffers[i].frame , &compositor.buffers[i].mem);
        simple_fill(&compositor.buffers[i].mem, &graphicsCfg.clip, 0);
    }
    compositor.region = graphicsCfg.clip;
    compositor.background = 0;
    NEXUS_Surface_Flush(compositor.buffers[0].frame);
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
    compositor.rects[0].color = 0xFFFFFF00;

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
    compositor.rects[1].color = 0xFF00FF00;

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
    compositor.rects[2].color = 0xFF404040;

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
    compositor.rects[3].color = 0xFF0000FF;

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
    compositor.rects[4].color = 0xFFFF0000;

    compositor.current = 0;
    compositor.display = display;

    BDBG_WRN(("start"));
    errCode = NEXUS_Display_SetGraphicsFramebuffer(display, compositor.buffers[0].frame);
    BDBG_ASSERT(!errCode);

    /* the rest of the state machine is driven by the framebufferCallback */

    fprintf(stderr, "Press ENTER to exit....\n");
    getchar();

    NEXUS_StopCallbacks(display);

    NEXUS_Display_RemoveAllOutputs(display);

    NEXUS_Display_Close(display);
    for(i=0;i<sizeof(compositor.buffers)/sizeof(*compositor.buffers);i++) {
        NEXUS_Surface_Destroy(compositor.buffers[i].frame);
    }
    NEXUS_Platform_Uninit();

    return 0;
}
#else /* NEXUS_HAS_DISPLAY */
int main(void)
{
    printf("This application is not supported on this platform (needs display)!\n");
    return 0;
}
#endif
