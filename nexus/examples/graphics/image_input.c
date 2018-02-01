/******************************************************************************
 *  Copyright (C) 2016-2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

#if NEXUS_HAS_GRAPHICS2D
#include "bstd.h"
#include "bkni.h"
#include "nexus_types.h"
#include "nexus_platform.h"
#include "nexus_display.h"
#include "nexus_component_output.h"
#include "nexus_composite_output.h"
#if NEXUS_HAS_HDMI_OUTPUT
#include "nexus_hdmi_output.h"
#endif
#include "nexus_graphics2d.h"
#include "nexus_video_output.h"
#include "nexus_video_window.h"
#include "nexus_video_image_input.h"
#include "nexus_video_input.h"

#include <stdio.h>
#include <string.h>

BDBG_MODULE(image_input);
BDBG_FILE_MODULE(pxlval);

#if 0
#define NUM_SURFACES 1 /* single buffering with offscreen surface for CPU rendering */
#else
#define NUM_SURFACES 3  /* triple buffering for image update every FRAME and a full queue */
#endif

#define SCB_8_0    0 /* only 7271 etc newer chips has SCB 8.0 */
#define MAP_5_0    1 /* 7445,7439 etc 28nm chips have MAP 5.0 */


struct {
    NEXUS_SurfaceHandle surface;
    NEXUS_StripedSurfaceHandle stripedSurface;
    void *pLumaAddr, *pChromaAddr;
    bool submitted;
} g_surface[NUM_SURFACES];

static void complete(void *data, int unused)
{
    BSTD_UNUSED(unused);
    BKNI_SetEvent((BKNI_EventHandle)data);
}

static void imageBufferCallback( void *context, int param )
{
    BSTD_UNUSED(param);
    BKNI_SetEvent((BKNI_EventHandle)context);
}

void print_usage( void )
{
    printf(
    "Usage: imaeg_input OPTIONS\n"
    "  --help or -h for help\n"
    "  -vfd    use VFD input\n"
    "  -stripe use striped surface (invalid for vfd input)\n"
    "  -size W,H  set image size to WxH (default 720x480)\n"
    );
}

int main(int argc, char **argv)
{
    NEXUS_Error rc;
    bool mainHeapFound = false, chromaHeapFound = false;
    NEXUS_HeapHandle chromaHeap=NULL;
    NEXUS_DisplayHandle display;
    NEXUS_VideoWindowHandle window;
    NEXUS_VideoWindowSettings windowCfg;
    NEXUS_GraphicsSettings graphicsCfg;
    NEXUS_SurfaceCreateSettings surfaceCfg;
    NEXUS_StripedSurfaceCreateSettings stripeSettings;
    NEXUS_PlatformSettings platformSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_VideoImageInputHandle imageInput;
    NEXUS_VideoInput videoInput;
    NEXUS_DisplaySettings displaySettings;
    NEXUS_Graphics2DHandle gfx;
    NEXUS_Graphics2DSettings gfxSettings;
    BKNI_EventHandle event,imageEvent;
    NEXUS_VideoImageInputSurfaceSettings surfaceSettings;
    NEXUS_VideoImageInputSettings imageInputSettings;
    NEXUS_VideoImageInputStatus imageInputStatus;
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_HdmiOutputStatus hdmiStatus;
#endif
    int i = 0;
    bool moveWindow=false;
    bool vfd = false;
    bool stripe = false;
    unsigned w = 720, h = 480;

    for(i=0; i<argc; i++) {
        if(!strcmp("-h",argv[i]) || !strcmp("--help",argv[i])) {
            print_usage();
            return 0;
        }
        else if (!strcmp(argv[i], "-vfd")) {
            vfd = true;
            if(stripe) {
                stripe = false;
                BDBG_WRN(("VFD input cannot use stripe surface!"));
            }
        }
        else if (!strcmp(argv[i], "-stripe")) {
            stripe = true;
            if(vfd) {
                vfd = false;
                BDBG_WRN(("Stripe surface must be MFD input!"));
            }
        }
        else if (!strcmp(argv[i], "-size")) {
            sscanf(argv[++i], "%u,%u", &w, &h);
            printf("Image size: %ux%u\n", w, h);
            if(w*h > 3840*2160/2) {
                stripe = true;
                BDBG_WRN(("Enabled striping due to 4K image!"));
            }
        }
    }

    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;

    /* The following trick is complicated and might not be able to map memory for all cases; maybe it's easier to
       use "bounce buffer" technique to work around: do gfx stripe blit with surface in main heap;
       then dma to the image input picture heap for render; */
#if NEXUS_MEMC2_GRAPHICS_HEAP
    i = NEXUS_MEMC2_GRAPHICS_HEAP;
#endif
    if(stripe) { /* gfx stripe blit module might be in kernel mode, that currently needs cpu access to surface */
#if NEXUS_MEMC2_DRIVER_HEAP
        i = NEXUS_MEMC2_DRIVER_HEAP;
#endif
        platformSettings.heap[i].memoryType |= NEXUS_MemoryType_eDriver;
    }
    /* increase gfx heaps size to hold the image input surfaces */
    if(platformSettings.heap[i].size>0 &&
        platformSettings.heap[i].size < (int)(w*h*2*NUM_SURFACES*2)) {
        platformSettings.heap[i].size = (w*h*2*NUM_SURFACES*2);
    }
#if NEXUS_MEMC1_GRAPHICS_HEAP
    i = NEXUS_MEMC1_GRAPHICS_HEAP;
#endif
    if(stripe) { /* gfx stripe blit module might be in kernel mode, that currently needs cpu access to surface */
#if NEXUS_MEMC1_DRIVER_HEAP
        i = NEXUS_MEMC1_DRIVER_HEAP;
#endif
        platformSettings.heap[i].memoryType |= NEXUS_MemoryType_eDriver;
    }
    if(platformSettings.heap[i].size>0 &&
        platformSettings.heap[i].size < (int)(w*h*2*NUM_SURFACES*2)) {
        platformSettings.heap[i].size = (w*h*2*NUM_SURFACES*2);
    }
#if NEXUS_MEMC0_GRAPHICS_HEAP
    i = NEXUS_MEMC0_GRAPHICS_HEAP;
#endif
    if(stripe) { /* gfx stripe blit module might be in kernel mode, that currently needs cpu access to surface */
#if NEXUS_MEMC0_DRIVER_HEAP
        i = NEXUS_MEMC0_DRIVER_HEAP;
#endif
        platformSettings.heap[i].memoryType |= NEXUS_MemoryType_eDriver;
    }
    if(platformSettings.heap[i].size>0 &&
        platformSettings.heap[i].size < (int)(w*h*2*NUM_SURFACES*2)) {
        platformSettings.heap[i].size = (w*h*2*NUM_SURFACES*2);
    }
    rc = NEXUS_Platform_Init(&platformSettings);
    BDBG_ASSERT(!rc);
    NEXUS_Platform_GetConfiguration(&platformConfig);

    NEXUS_Display_GetDefaultSettings(&displaySettings);
    displaySettings.displayType = NEXUS_DisplayType_eAuto;
    displaySettings.format      = NEXUS_VideoFormat_eNtsc; /* change to xx_e720p to observe faster draw rate */
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

    window = NEXUS_VideoWindow_Open(display, 0);
    BDBG_ASSERT(window);

    if (moveWindow) {
        NEXUS_VideoWindow_GetSettings(window, &windowCfg);
        windowCfg.position.x = windowCfg.position.width/4;
        windowCfg.position.y = windowCfg.position.height/4;
        windowCfg.position.width/=2;
        windowCfg.position.height/=2;
        windowCfg.visible = true;
        rc = NEXUS_VideoWindow_SetSettings(window, &windowCfg);
        BDBG_ASSERT(!rc);
    }

    BKNI_CreateEvent(&event);
    gfx = NEXUS_Graphics2D_Open(0, NULL);
    NEXUS_Graphics2D_GetSettings(gfx, &gfxSettings);
    gfxSettings.checkpointCallback.callback = complete;
    gfxSettings.checkpointCallback.context = event;
    NEXUS_Graphics2D_SetSettings(gfx, &gfxSettings);

    /* Create a surface that will be used as a graphics framebuffer.
    This is used to show compositing the graphics (img[]) being fed through the video window
    with graphics (framebuffer) being fed through the graphics feeder. */
    NEXUS_Display_GetGraphicsSettings(display, &graphicsCfg);

    NEXUS_VideoImageInput_GetDefaultSettings(&imageInputSettings);
    imageInputSettings.type = vfd ? NEXUS_VideoImageInput_eVfd : NEXUS_VideoImageInput_eMfd;
    imageInput = NEXUS_VideoImageInput_Open(0, &imageInputSettings);
    BDBG_ASSERT(imageInput);

    NEXUS_VideoImageInput_GetStatus(imageInput, &imageInputStatus);

    /* Create surfaces that will be used with VideoImageInput */
    NEXUS_Surface_GetDefaultCreateSettings(&surfaceCfg);
    surfaceCfg.width  = w;
    surfaceCfg.height = h;
    surfaceCfg.pixelFormat = NEXUS_PixelFormat_eCr8_Y18_Cb8_Y08;
    for (i=0;i<NEXUS_MAX_HEAPS;i++) {
        NEXUS_MemoryStatus s;
        if (!platformConfig.heap[i] || NEXUS_Heap_GetStatus(platformConfig.heap[i], &s)) continue;
        /* use driver heap in case of kernel mode NEXUS */
        if (s.memcIndex == imageInputStatus.memcIndex && (s.memoryType & NEXUS_MemoryType_eDriver) && s.largestFreeBlock >= w*h*2 && !mainHeapFound) {
            surfaceCfg.heap = platformConfig.heap[i];
            mainHeapFound = true;
            BDBG_MSG(("main heap %d", i));
            if(chromaHeapFound || !stripe) {
                break;
            }
        }
        if (s.memcIndex == imageInputStatus.secondaryMemcIndex && (s.memoryType & NEXUS_MemoryType_eDriver) && s.largestFreeBlock >= w*h/2 && !chromaHeapFound) {
            chromaHeap = platformConfig.heap[i];
            chromaHeapFound = true;
            BDBG_MSG(("second heap %d", i));
            if(mainHeapFound) {
                break;
            }
        }
    }
    if (!surfaceCfg.heap) {
        BDBG_ERR(("no heap found. RTS failure likely."));
    }
    for( i=0; i < NUM_SURFACES ; i++ ) {
        g_surface[i].surface = NEXUS_Surface_Create(&surfaceCfg);
        BDBG_ASSERT(g_surface[i].surface);
        g_surface[i].submitted = false;
        if(stripe) {
            NEXUS_StripedSurface_GetDefaultCreateSettings(&stripeSettings);
            stripeSettings.lumaHeap = surfaceCfg.heap;
            BDBG_ASSERT(chromaHeap);
            stripeSettings.chromaHeap = chromaHeap;
            stripeSettings.imageWidth  = surfaceCfg.width;
            stripeSettings.imageHeight = surfaceCfg.height;
            g_surface[i].stripedSurface = NEXUS_StripedSurface_Create(&stripeSettings);
        }
    }

    videoInput = NEXUS_VideoImageInput_GetConnector(imageInput);
    BDBG_ASSERT(videoInput);

    rc = NEXUS_VideoWindow_AddInput(window, videoInput);
    BDBG_ASSERT(!rc);

    BKNI_CreateEvent(&imageEvent);
    NEXUS_VideoImageInput_GetSettings( imageInput, &imageInputSettings);
    imageInputSettings.imageCallback.callback = imageBufferCallback;
    imageInputSettings.imageCallback.context  = imageEvent;
    NEXUS_VideoImageInput_SetSettings( imageInput, &imageInputSettings);

#if NUM_SURFACES == 1
    {
        NEXUS_Graphics2DBlitSettings blitSettings;
        NEXUS_SurfaceHandle offscreen;
        unsigned x,y;
        NEXUS_SurfaceMemory mem;

        /* create a surface which is CPU accessible, then blit to to g_surface[0].surface, which is MFD accessible */
        NEXUS_Surface_GetDefaultCreateSettings(&surfaceCfg);
        surfaceCfg.width  = w;
        surfaceCfg.height = h;
        surfaceCfg.heap = NEXUS_Platform_GetFramebufferHeap(0); /* CPU and GFD accessible, even though we aren't sending to GFD */
        offscreen = NEXUS_Surface_Create(&surfaceCfg);

        /* draw using CPU */
        NEXUS_Surface_GetMemory(offscreen, &mem);
        for(y=0;y<surfaceCfg.height;y++) {
            uint32_t *buf = (uint32_t *)((uint8_t *)mem.buffer + mem.pitch*y);
            for(x=0;x<surfaceCfg.width;x++) {
                buf[x] = ((x/10)%2 != (y/10)%2) ? 0xFFAAAAAA : 0xFF0000FF; /* checker board */
            }
        }
        NEXUS_Surface_Flush(offscreen);

        NEXUS_Graphics2D_GetDefaultBlitSettings(&blitSettings);
        blitSettings.source.surface = offscreen;
        blitSettings.output.surface = g_surface[0].surface;
        rc = NEXUS_Graphics2D_Blit(gfx, &blitSettings);
        BDBG_ASSERT(!rc);
        rc = NEXUS_Graphics2D_Checkpoint(gfx, NULL);
        if (rc == NEXUS_GRAPHICS2D_QUEUED) {
            BKNI_WaitForEvent(event, 0xffffffff);
        }

        NEXUS_Surface_Destroy(offscreen);

        if(stripe) {
            NEXUS_Graphics2DStripeBlitSettings settings;
            NEXUS_Graphics2D_GetDefaultStripeBlitSettings(&settings);
            settings.source.surface = g_surface[0].surface;
            settings.output.stripedSurface = g_surface[0].stripedSurface;
            /* NOTE: must check point all pending blits prior to stripe blit! */
            NEXUS_Graphics2D_StripeBlit(gfx, &settings);
            rc = NEXUS_Graphics2D_Checkpoint(gfx, NULL);
            if (rc == NEXUS_GRAPHICS2D_QUEUED) {
                BKNI_WaitForEvent(event, 0xffffffff);
            }
            NEXUS_VideoImageInput_GetDefaultSurfaceSettings(&surfaceSettings);
            surfaceSettings.stripedSurface = g_surface[0].stripedSurface;
            NEXUS_VideoImageInput_PushSurface(imageInput, NULL, &surfaceSettings);
        } else {
            NEXUS_VideoImageInput_PushSurface(imageInput, g_surface[0].surface, NULL);
        }
    }
#else
    {
    unsigned submitIdx = 0,releaseIdx = 0;

    for(i=0;i<(int)w*10;i++) {
        NEXUS_Graphics2DFillSettings fillSettings;
        NEXUS_SurfaceHandle freeSurface=NULL;
        NEXUS_SurfaceHandle pic;
        size_t num_entries = 0;

        /* Make sure image surface is not in use by Video Output (VDC) */
        do {
            if ( g_surface[submitIdx].submitted ) {
                /* our queue is all used up, need to wait until VideoImageInput returns */
                /* a surface after it has been displayed                                */
                BDBG_MSG(("g_surface[submitIdx=%d].submitted in use, wait for recycle" , submitIdx ));
                BKNI_WaitForEvent( imageEvent, BKNI_INFINITE);
            }

            rc=NEXUS_VideoImageInput_RecycleSurface( imageInput, &freeSurface , 1, &num_entries );
            BDBG_ASSERT(!rc);
            if ( num_entries ) {
                /* our surface has been displayed, we can now re-use and re-queue it */
                BDBG_MSG(("g_surface[releaseIdx=%d].surface=%p  recycSurface=%p" , releaseIdx, (void*)g_surface[releaseIdx].surface , (void*)freeSurface ));
                BDBG_ASSERT( ((NEXUS_SurfaceHandle)g_surface[releaseIdx].stripedSurface==freeSurface) || (g_surface[releaseIdx].surface == freeSurface) );
                g_surface[releaseIdx].submitted = false;
                if ( ++releaseIdx == NUM_SURFACES ) releaseIdx=0;
            }

        } while ( num_entries || g_surface[submitIdx].submitted );

        g_surface[submitIdx].submitted = true; /* mark as inuse */
        pic = g_surface[submitIdx].surface;
        BDBG_MSG(("pic[%u]=%p" , submitIdx, (void*)pic ));

        /* must do M2MC fill. CPU may not have access to this surface on some non-UMA systems. */
        NEXUS_Graphics2D_GetDefaultFillSettings(&fillSettings);
        fillSettings.surface = pic;
        fillSettings.color = 0xFF00FF00; /* magenta (AYCbCr) */
        fillSettings.rect.x = 0;
        fillSettings.rect.width = w;
        fillSettings.rect.y = 0;
        fillSettings.rect.height = (i*4)%(2*h);
        if (fillSettings.rect.height > h) fillSettings.rect.height = 2*h - fillSettings.rect.height;
        if (fillSettings.rect.height) {
            rc = NEXUS_Graphics2D_Fill(gfx, &fillSettings);
            BDBG_ASSERT(!rc);
        }
        fillSettings.color = 0xFF00FFFF; /* blue (AYCbCr) */
        fillSettings.rect.y = fillSettings.rect.height;
        fillSettings.rect.height = h-fillSettings.rect.y;
        if (fillSettings.rect.height) {
            rc = NEXUS_Graphics2D_Fill(gfx, &fillSettings);
            BDBG_ASSERT(!rc);
        }
        rc = NEXUS_Graphics2D_Checkpoint(gfx, NULL);
        if (rc == NEXUS_GRAPHICS2D_QUEUED) {
            BKNI_WaitForEvent(event, 0xffffffff);
        }

        NEXUS_VideoImageInput_GetDefaultSurfaceSettings( &surfaceSettings );
        /* Submit surface to VideoImageInput, wait if queue to feed VDC is already full */
        do {
            /* 4K image must be striped; or interleaved striped and regular surface; */
            if(stripe && (w*h>3840*2160/2 || (i&1))) {
                NEXUS_Graphics2DStripeBlitSettings settings;
                NEXUS_Graphics2D_GetDefaultStripeBlitSettings(&settings);
                settings.source.surface = pic;
                settings.output.stripedSurface = g_surface[submitIdx].stripedSurface;
                /* NOTE: must check point all pending blits prior to stripe blit! */
                NEXUS_Graphics2D_StripeBlit(gfx, &settings);
                rc = NEXUS_Graphics2D_Checkpoint(gfx, NULL);
                if (rc == NEXUS_GRAPHICS2D_QUEUED) {
                    BKNI_WaitForEvent(event, 0xffffffff);
                }
                surfaceSettings.stripedSurface = g_surface[submitIdx].stripedSurface;
                rc = NEXUS_VideoImageInput_PushSurface(imageInput, NULL, &surfaceSettings);
            } else {/* default: regular surface only */
                rc = NEXUS_VideoImageInput_PushSurface(imageInput, pic , &surfaceSettings );
            }
            if(rc==NEXUS_IMAGEINPUT_QUEUE_FULL) {
                BKNI_WaitForEvent(imageEvent, BKNI_INFINITE);
            }
        } while ( rc );
        if ( ++submitIdx == NUM_SURFACES ) submitIdx=0;

        if ( moveWindow ) {
            /* Moving the window slows system throughput, set moveWindow to false to see higher throughput */
            NEXUS_VideoWindow_GetSettings(window, &windowCfg);
            windowCfg.position.x=(i*8)%windowCfg.position.width;
            windowCfg.position.y=(i*4)%windowCfg.position.height;
            rc = NEXUS_VideoWindow_SetSettings(window, &windowCfg);
            BDBG_ASSERT(!rc);
        }
    }
    }
#endif

    fprintf(stderr, "done. press enter to exit.\n");
    getchar();

    /* All work is done, but we must free any image displayed by passing NULL in to flush the pipeline */
    NEXUS_VideoImageInput_SetSurface(imageInput, NULL);
    NEXUS_VideoWindow_RemoveAllInputs(window);
    NEXUS_VideoInput_Shutdown(videoInput);
    NEXUS_VideoImageInput_Close(imageInput);

    BKNI_DestroyEvent(imageEvent);
    BKNI_DestroyEvent(event);

    if ( gfx ) NEXUS_Graphics2D_Close( gfx );
    NEXUS_VideoWindow_Close(window);
    NEXUS_Display_Close(display);
    for( i=NUM_SURFACES; i > 0 ; i-- ) {
        NEXUS_Surface_Destroy(g_surface[i-1].surface);
        if(g_surface[i-1].stripedSurface) NEXUS_StripedSurface_Destroy(g_surface[i-1].stripedSurface);
    }
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
