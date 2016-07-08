/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2014-2016 Broadcom. All rights reserved.
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

#include "nexus_platform.h"
#include <stdio.h>

#if NEXUS_HAS_GRAPHICS2D
#include "nexus_surface.h"
#include "nexus_graphics2d.h"
#include "nexus_display.h"
#include "nexus_composite_output.h"
#include "nexus_component_output.h"
#if NEXUS_HAS_HDMI_OUTPUT
#include "nexus_hdmi_output.h"
#endif
#include "nexus_core_utils.h"
#include "nexus_video_decoder.h"
#include "nexus_video_window.h"
#include "nexus_parser_band.h"
#include "bstd.h"
#include "bkni.h"
#include "blst_queue.h"
#include <stdlib.h>
#include <string.h>

BDBG_MODULE(gfxvideo);

/* the following define the input and its characteristics -- these will vary by input */
#define TRANSPORT_TYPE NEXUS_TransportType_eTs
#define VIDEO_CODEC NEXUS_VideoCodec_eMpeg2
#define VIDEO_PID 0x11

void complete(void *data, int unused)
{
    BSTD_UNUSED(unused);
    BKNI_SetEvent((BKNI_EventHandle)data);
}

struct render_element {
    BLST_Q_ENTRY(render_element) link;
    NEXUS_SurfaceHandle surface;
    NEXUS_Rect rect;
    NEXUS_Rect size;
    int stepX, stepY;
};

struct compositor {
    BLST_Q_HEAD(compositor_elements,render_element) elements;
    NEXUS_Graphics2DHandle graphics;
    NEXUS_Rect region;
    NEXUS_Pixel background;
    BKNI_EventHandle checkpointEvent, spaceAvailableEvent;
    size_t allocated;
};

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
    if(rect->width + rect->x >= disp->width) {
        rect->width -= (rect->width + rect->x) - disp->width;
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
                if(rect->y + size->height <= disp->height) {
                    rect->height = size->height;
                }
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
    if(rect->height + rect->y >= disp->height) {
        rect->height -= (rect->height + rect->y) - disp->height;
    }
}

static void
compositorbuffer_step(struct compositor *compositor, NEXUS_SurfaceHandle framebuffer)
{
    NEXUS_Error rc;
    struct render_element *element;
    NEXUS_Graphics2DFillSettings fillSettings;
    NEXUS_Graphics2DBlitSettings blitSettings;

    NEXUS_Graphics2D_GetDefaultFillSettings(&fillSettings);
    fillSettings.surface = framebuffer;
    fillSettings.color = compositor->background;
    fillSettings.rect = compositor->region;
    for(;;) {
        rc = NEXUS_Graphics2D_Fill(compositor->graphics, &fillSettings);
        if (rc != NEXUS_GRAPHICS2D_QUEUE_FULL) {
            break;
        }
        BKNI_WaitForEvent(compositor->spaceAvailableEvent, BKNI_INFINITE);
    }

#if 0

    for(element=BLST_Q_FIRST(&compositor->elements);element;element=BLST_Q_NEXT(element,link)) {
        fillSettings.rect = element->rect;
        for(;;) {
            rc = NEXUS_Graphics2D_Fill(compositor->graphics, &fillSettings);
            if (rc != NEXUS_GRAPHICS2D_QUEUE_FULL) {
                break;
            }
            BKNI_WaitForEvent(compositor->spaceAvailableEvent, BKNI_INFINITE);
        }
    }
#endif

    for(element=BLST_Q_FIRST(&compositor->elements);element;element=BLST_Q_NEXT(element,link)) {
        rect_move(&compositor->region, &element->size, &element->rect, element->stepX, element->stepY);
    }
    NEXUS_Graphics2D_GetDefaultBlitSettings(&blitSettings);
    blitSettings.output.surface = framebuffer;
    blitSettings.colorOp = NEXUS_BlitColorOp_eCopySource;
    blitSettings.alphaOp = NEXUS_BlitAlphaOp_eCopySource;

    for(element=BLST_Q_FIRST(&compositor->elements);element;element=BLST_Q_NEXT(element,link)) {
        if(element->rect.width==0 || element->rect.height==0) {
            continue;
        }
        blitSettings.output.rect = element->rect;
        blitSettings.source.rect = element->rect;
        blitSettings.source.rect.x = blitSettings.source.rect.y = 0;

        blitSettings.source.surface = element->surface;
        for(;;) {
            rc = NEXUS_Graphics2D_Blit(compositor->graphics, &blitSettings);
            if (rc != NEXUS_GRAPHICS2D_QUEUE_FULL) {
                break;
            }
            BKNI_WaitForEvent(compositor->spaceAvailableEvent, BKNI_INFINITE);
        }
    }
    rc = NEXUS_Graphics2D_Checkpoint(compositor->graphics, NULL); /* require to execute queue */
    if (rc == NEXUS_GRAPHICS2D_QUEUED) {
        rc = BKNI_WaitForEvent(compositor->checkpointEvent, BKNI_INFINITE);
    }
    return;
}

static void
compositorbuffer_add_element(struct compositor *compositor, NEXUS_HeapHandle heap, const NEXUS_Rect *position, NEXUS_Pixel color, int stepX, int stepY)
{
    struct render_element *element;
    NEXUS_SurfaceCreateSettings createSettings;
    NEXUS_Graphics2DFillSettings fillSettings;
    NEXUS_SurfaceStatus status;

    element = BKNI_Malloc(sizeof(*element));
    if(!element) {
        return;
    }
    NEXUS_Surface_GetDefaultCreateSettings(&createSettings);
    createSettings.pixelFormat = NEXUS_PixelFormat_eA8_R8_G8_B8;
    createSettings.width = position->width;
    createSettings.height = position->height;
    createSettings.heap = heap;
    for(;;) {
        NEXUS_Error rc;
        element->surface = NEXUS_Surface_Create(&createSettings);
        if(element->surface!=NULL) {
            break;
        }
        rc = NEXUS_Platform_GrowHeap(heap, 32*1024*1024);
        if(rc!=NEXUS_SUCCESS) {
            break;
        }
    }
    element->rect = *position;
    element->size = *position;
    element->size.x = element->size.y = 0;
    element->stepX = stepX;
    element->stepY = stepY;
    BLST_Q_INSERT_TAIL(&compositor->elements, element, link);

    NEXUS_Surface_GetStatus(element->surface, &status);
    compositor->allocated += status.height  * status.pitch;

    NEXUS_Graphics2D_GetDefaultFillSettings(&fillSettings);
    fillSettings.surface = element->surface;
    fillSettings.rect.width = position->width;
    fillSettings.rect.height = position->height;
    fillSettings.color = color;
    NEXUS_Graphics2D_Fill(compositor->graphics, &fillSettings);

    return;
}

static void
compositorbuffer_remove_element(struct compositor *compositor, struct render_element *element)
{
    NEXUS_SurfaceStatus status;

    BLST_Q_REMOVE(&compositor->elements, element, link);

    NEXUS_Surface_GetStatus(element->surface, &status);
    compositor->allocated -= status.height  * status.pitch;

    NEXUS_Surface_Destroy(element->surface);
    BKNI_Free(element);
    return;
}


int main(int argc, char **argv)
{
    NEXUS_SurfaceHandle surface, surface1;
    NEXUS_SurfaceCreateSettings createSettings;
    NEXUS_DisplayHandle display;
    NEXUS_DisplaySettings displaySettings;
    NEXUS_GraphicsSettings graphicsSettings;
    NEXUS_Graphics2DHandle gfx;
    NEXUS_Graphics2DFillSettings fillSettings;
    NEXUS_Graphics2DSettings gfxSettings;
    BKNI_EventHandle checkpointEvent, spaceAvailableEvent, framebufferApplied;
    NEXUS_PlatformSettings platformSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_VideoFormatInfo info;
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_HdmiOutputStatus hdmiStatus;
#endif
    NEXUS_Error rc;
    int i;
    NEXUS_HeapHandle heap;
    bool dynamic_cma = true;
    NEXUS_ParserBand parserBand = NEXUS_ParserBand_e0;
    NEXUS_ParserBandSettings parserBandSettings;
    NEXUS_VideoWindowHandle window;
    NEXUS_VideoDecoderOpenSettings openSettings;
    NEXUS_VideoDecoderHandle videoDecoder;
    NEXUS_VideoDecoderStartSettings videoProgram;
    NEXUS_MemoryBlockHandle seedBlock;
    size_t seed_size = 20*1024*1024;/* hardcoded for 7445, minimum size for v3d */
    struct compositor compositor;
    NEXUS_MemoryStatus heapStatus;

    if (argc > 1 && !strcmp(argv[1],"-off")) dynamic_cma = false;

    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    if (dynamic_cma) {
        platformSettings.heap[7].size = seed_size; /* hardcoded for 7445 */
        /* new heap */
        platformSettings.heap[10] = platformSettings.heap[7];
        platformSettings.heap[10].memoryType = NEXUS_MEMORY_TYPE_MANAGED|NEXUS_MEMORY_TYPE_ONDEMAND_MAPPED;
    }
    rc = NEXUS_Platform_Init(&platformSettings);
    if (rc) return rc;
    NEXUS_Platform_GetConfiguration(&platformConfig);

    heap = NEXUS_Platform_GetFramebufferHeap(0);
    if (dynamic_cma) {
        BDBG_ASSERT(platformConfig.heap[7] == heap); /* verify that 7 is the framebuffer heap */
        heap = platformConfig.heap[10];
        BDBG_ASSERT(heap);
    }

    /* used up all memory in the target heap */
    seedBlock = NEXUS_MemoryBlock_Allocate(heap, seed_size-1024*1024, 0, NULL);
    if (dynamic_cma) {
        /* and add some more memory */
        rc = NEXUS_Platform_GrowHeap(heap, 128*1024*1024);
        BDBG_ASSERT(rc==NEXUS_SUCCESS);
    }

    NEXUS_Display_GetDefaultSettings(&displaySettings);
    displaySettings.displayType = NEXUS_DisplayType_eAuto;
    displaySettings.format = NEXUS_VideoFormat_eNtsc;
    displaySettings.format = NEXUS_VideoFormat_e1080i;
    display = NEXUS_Display_Open(0, &displaySettings);

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

    NEXUS_VideoFormat_GetInfo(displaySettings.format, &info);

    NEXUS_Surface_GetDefaultCreateSettings(&createSettings);
    createSettings.pixelFormat = NEXUS_PixelFormat_eA8_R8_G8_B8;
    createSettings.width = info.width;
    createSettings.height = info.height;
    createSettings.heap = heap;
    surface = NEXUS_Surface_Create(&createSettings);
    if (dynamic_cma) {
        NEXUS_Heap_Dump(heap);
    }
    surface1 = NEXUS_Surface_Create(&createSettings);

    BKNI_CreateEvent(&checkpointEvent);
    BKNI_CreateEvent(&spaceAvailableEvent);
    BKNI_CreateEvent(&framebufferApplied);

    gfx = NEXUS_Graphics2D_Open(0, NULL);

    NEXUS_Graphics2D_GetSettings(gfx, &gfxSettings);
    gfxSettings.checkpointCallback.callback = complete;
    gfxSettings.checkpointCallback.context = checkpointEvent;
    gfxSettings.packetSpaceAvailable.callback = complete;
    gfxSettings.packetSpaceAvailable.context = spaceAvailableEvent;
    NEXUS_Graphics2D_SetSettings(gfx, &gfxSettings);

    BKNI_Memset(&compositor, 0, sizeof(compositor));
    BLST_Q_INIT(&compositor.elements);
    compositor.graphics = gfx;
    compositor.checkpointEvent = checkpointEvent;
    compositor.spaceAvailableEvent = spaceAvailableEvent;
    compositor.background = 0;
    compositor.allocated = 0;
    compositor.region.width = createSettings.width;
    compositor.region.height = createSettings.height;

    NEXUS_Heap_GetStatus(heap, &heapStatus);
    compositor.allocated = heapStatus.size - heapStatus.free;


    /* fill with black */
    NEXUS_Graphics2D_GetDefaultFillSettings(&fillSettings);
    fillSettings.surface = surface;
    fillSettings.rect.width = createSettings.width;
    fillSettings.rect.height = createSettings.height;
    fillSettings.color = 0;
    NEXUS_Graphics2D_Fill(gfx, &fillSettings);

    rc = NEXUS_Graphics2D_Checkpoint(gfx, NULL); /* require to execute queue */
    if (rc == NEXUS_GRAPHICS2D_QUEUED) {
        rc = BKNI_WaitForEvent(checkpointEvent, BKNI_INFINITE);
    }
    BDBG_ASSERT(!rc);

    NEXUS_Display_GetGraphicsSettings(display, &graphicsSettings);
    graphicsSettings.enabled = true;
    graphicsSettings.frameBufferCallback.callback = complete;
    graphicsSettings.frameBufferCallback.context = framebufferApplied;

    NEXUS_Display_SetGraphicsSettings(display, &graphicsSettings);
    NEXUS_Display_SetGraphicsFramebuffer(display, surface);


#if 1
    BDBG_WRN(("cpu test"));
    {
        NEXUS_SurfaceMemory mem;
        NEXUS_Surface_GetMemory(surface, &mem);
        for (i=0;i<10;i++) {
            uint32_t color = rand()|0xFF000000;
            unsigned x,y;
            for (y=0;y<createSettings.height;y++) {
                uint32_t *ptr = (void*)((uint8_t *)mem.buffer + mem.pitch*y);
                for (x=0;x<createSettings.width;x++) {
                    ptr[x] = color;
                }
            }
            NEXUS_Surface_Flush(surface);
            BKNI_Sleep(200);
        }
    }
#endif

    BDBG_WRN(("m2mc test"));
    NEXUS_Graphics2D_GetDefaultFillSettings(&fillSettings); /* don't get defaults more often than necessary */
    fillSettings.surface = surface;
    fillSettings.rect.width = 20;
    fillSettings.rect.height = 20;
    fillSettings.colorOp = NEXUS_FillOp_eCopy;
    fillSettings.alphaOp = NEXUS_FillOp_eCopy;
    for(i=0;i<5000*20;i++) {
        fillSettings.rect.x = rand() % (createSettings.width-20);
        fillSettings.rect.y = rand() % (createSettings.height-20);
        fillSettings.color = (rand()%0xFFFFFF) | ((i/200)%0xFF<<24);
        rc = NEXUS_Graphics2D_Fill(gfx, &fillSettings);
        BDBG_ASSERT(!rc);
        if (i && i%5000==0) {
            /* must do checkpoint before measurement */
            rc = NEXUS_Graphics2D_Checkpoint(gfx, NULL);
            if (rc == NEXUS_GRAPHICS2D_QUEUED) {
                rc = BKNI_WaitForEvent(checkpointEvent, BKNI_INFINITE);
            }
            BDBG_ASSERT(!rc);
        }
    }
    {
        NEXUS_Rect position;
        NEXUS_SurfaceHandle framebuffer = surface;
        unsigned count = 1;
        bool grow = true;

        position.x = position.y = 0;
        position.width = createSettings.width/2;
        position.height = createSettings.height/2;
        compositorbuffer_add_element(&compositor, heap, &position, 0x80FF00FFu, 1, 1);
        for(;;) {
            unsigned r = rand();
            if( (grow && r >= 3*(RAND_MAX/4)) || (r >= 31 *(RAND_MAX/32))) {
                uint32_t color = rand()|0xFF000000;
                int stepX, stepY;
                position.width = rand()%createSettings.width;
                position.height = rand()%createSettings.height;
                position.width = position.width < 16 ? 16 : position.width;
                position.height = position.height < 16 ? 16 : position.height;
                stepX = 1+rand()%16;
                stepY = 1+rand()%16;
                if(rand()>=RAND_MAX/2) {
                    stepX = -stepX;
                }
                if(rand()>=RAND_MAX/2) {
                    stepY = -stepY;
                }
                compositorbuffer_add_element(&compositor, heap, &position, color, stepX, stepY);
                count++;
                BDBG_LOG(("add %u:%u", count, compositor.allocated/1024));
                if(grow  && (count > 256 || compositor.allocated >= 384*1024*1024)) {
                    grow = false;
                }
            }
            if(count > 0) {
                if(
                   (count > 256 && r <= RAND_MAX/8) ||
                   (count > 512 && r <= RAND_MAX/4) ||
                   (count <= 256 && ((grow && r <= RAND_MAX/16) || (r <= RAND_MAX/8)))
                   ) {
                    struct render_element *element;
                    unsigned n = r % count;
                    for(element = BLST_Q_FIRST(&compositor.elements); n>0 ; n--) {
                        element = BLST_Q_NEXT(element, link);
                        BDBG_ASSERT(element);
                    }
                    compositorbuffer_remove_element(&compositor, element);
                    count--;
                    if(!grow && count < 64) {
                        NEXUS_Platform_ShrinkHeap(heap, 4*1024*1024, 32*1024*1024);
                        grow = true;
                    }
                    BDBG_LOG(("remove %u(%u)", count, compositor.allocated/1024));
                }
            }
            if(r%16 < 2) {
                compositorbuffer_step(&compositor, framebuffer);
                BKNI_WaitForEvent(framebufferApplied, BKNI_INFINITE);
                framebuffer = (framebuffer == surface) ? surface1 : surface;
                NEXUS_Display_SetGraphicsFramebuffer(display, framebuffer);
            }
        }
    }
    NEXUS_Display_GetGraphicsSettings(display, &graphicsSettings);
    graphicsSettings.enabled = false;
    NEXUS_Display_SetGraphicsSettings(display, &graphicsSettings);
    NEXUS_Graphics2D_Close(gfx);
    NEXUS_Surface_Destroy(surface);
    NEXUS_Surface_Destroy(surface1);
    if(dynamic_cma) {
        NEXUS_Platform_ShrinkHeap(heap, 64*1024*1024, 4096);
        NEXUS_Heap_Dump(heap);
    }
    NEXUS_MemoryBlock_Free(seedBlock);
    BKNI_DestroyEvent(checkpointEvent);
    BKNI_DestroyEvent(spaceAvailableEvent);

    BDBG_WRN(("decode test"));
    NEXUS_ParserBand_GetSettings(parserBand, &parserBandSettings);
    parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eInputBand;
    NEXUS_Platform_GetStreamerInputBand(0, &parserBandSettings.sourceTypeSettings.inputBand);
    parserBandSettings.transportType = TRANSPORT_TYPE;
    NEXUS_ParserBand_SetSettings(parserBand, &parserBandSettings);
    window = NEXUS_VideoWindow_Open(display, 0);
    NEXUS_VideoDecoder_GetDefaultOpenSettings(&openSettings);
    if (dynamic_cma) {
        openSettings.pictureHeap = heap;
    }
    /* opening 0, which uses HVD0 and MEMC2 on 97445. will vary for other platforms. */
    videoDecoder = NEXUS_VideoDecoder_Open(0, &openSettings);
    rc = NEXUS_VideoWindow_AddInput(window, NEXUS_VideoDecoder_GetConnector(videoDecoder));
    BDBG_ASSERT(!rc);
    NEXUS_VideoDecoder_GetDefaultStartSettings(&videoProgram);
    videoProgram.codec = VIDEO_CODEC;
    videoProgram.pidChannel = NEXUS_PidChannel_Open(parserBand, VIDEO_PID, NULL);
    rc = NEXUS_VideoDecoder_Start(videoDecoder, &videoProgram);
    BDBG_ASSERT(!rc);

    BDBG_WRN(("press enter to stop"));
    getchar();

    NEXUS_VideoDecoder_Stop(videoDecoder);
    NEXUS_VideoWindow_Close(window);
    NEXUS_VideoDecoder_Close(videoDecoder);
    NEXUS_Display_Close(display);
    NEXUS_Platform_Uninit();

    return 0;
}
#else /* NEXUS_HAS_GRAPHICS2D */
int main(void)
{
    printf("ERROR: NEXUS_Graphics2D not supported\n");
    return -1;
}
#endif
