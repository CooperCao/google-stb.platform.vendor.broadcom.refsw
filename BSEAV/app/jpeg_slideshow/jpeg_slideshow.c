/***************************************************************************
 *     Copyright (c) 2004-2010, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *    Reference JPEG Decode into YUYV or RGB Color spaces with slideshow
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *************************************************************************/
#include <stdio.h>
#include <string.h>
#include <sys/times.h>
#include <sys/time.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

#include "nexus_platform.h"
#include "nexus_display.h"
#include "nexus_graphics2d.h"
#include "nexus_core_utils.h"
#include "b_decode_jpeg.h"
#if NEXUS_DTV_PLATFORM
#include "nexus_platform_boardcfg.h"
#endif

int b_surface_copy(NEXUS_Graphics2DHandle gfx, NEXUS_SurfaceHandle dest, const NEXUS_Rect *pDestRect, NEXUS_SurfaceHandle src, const NEXUS_Rect *pSrcRect)
{
    NEXUS_Graphics2DBlitSettings blitSettings;
    NEXUS_Error rc;

    NEXUS_Graphics2D_GetDefaultBlitSettings(&blitSettings);
    blitSettings.source.surface = src;
    if (pSrcRect) {
        blitSettings.source.rect = *pSrcRect;
    }
    blitSettings.output.surface = dest;
    if (pDestRect) {
        blitSettings.output.rect = *pDestRect;
    }
    rc = NEXUS_Graphics2D_Blit(gfx, &blitSettings);
    BDBG_ASSERT(!rc);

    /* must know the blit has completed before we can destroy the surface.
    just do a simple polling loop. */
    while (NEXUS_Graphics2D_Checkpoint(gfx, NULL)) {
        BKNI_Sleep(1);
    }
    return 0;
}

int b_surface_fill(NEXUS_Graphics2DHandle gfx, NEXUS_SurfaceHandle sur, const NEXUS_Rect *pRect, NEXUS_Pixel pixel)
{
    NEXUS_Graphics2DFillSettings fillSettings;
    NEXUS_Error rc;

    NEXUS_Graphics2D_GetDefaultFillSettings(&fillSettings);
    fillSettings.surface = sur;
    if (pRect) {
        fillSettings.rect = *pRect;
    }
    fillSettings.color = pixel;
    rc = NEXUS_Graphics2D_Fill(gfx, &fillSettings);
    BDBG_ASSERT(!rc);

    /* must know the blit has completed before we can destroy the surface.
    just do a simple polling loop. */
    while (NEXUS_Graphics2D_Checkpoint(gfx, NULL)) {
        BKNI_Sleep(1);
    }
    return 0;
}

/* global state */
NEXUS_DisplayHandle display;
NEXUS_SurfaceHandle framebuffer[2];
unsigned cur_fb = 0; /* current drawing fb (i.e. the one that's not visible) */
NEXUS_Graphics2DHandle gfx;
BKNI_EventHandle fb_callback_event;

static void flip(void)
{
    NEXUS_Error rc;

    /* blits must be finished */
    while (NEXUS_Graphics2D_Checkpoint(gfx, NULL)) {
        BKNI_Sleep(1);
    }

    BKNI_ResetEvent(fb_callback_event);

    rc = NEXUS_Display_SetGraphicsFramebuffer(display, framebuffer[cur_fb]);
    BDBG_ASSERT(!rc);

    cur_fb = 1 - cur_fb;

    /* wait for flip to complete */
    BKNI_WaitForEvent(fb_callback_event, 1000);
}

static void fb_callback(void *context, int param)
{
    BSTD_UNUSED(param);
    BKNI_SetEvent((BKNI_EventHandle)context);
}

int main(int argc, char **argv)
{
    NEXUS_SurfaceCreateSettings surfaceCreateSettings;
    NEXUS_VideoFormatInfo videoFormatInfo;
    NEXUS_GraphicsSettings graphicsSettings;
    NEXUS_PlatformSettings platformSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_DisplaySettings displaySettings;
    NEXUS_Error rc;
    NEXUS_SurfaceHandle newSurface=NULL;
    NEXUS_SurfaceHandle oldSurface=NULL;
    const NEXUS_Rect destRect = {0, 0, 960, 1080};
    int i, total, first;
    struct timeval timeStart, timeEnd;
    NEXUS_Graphics2DSettings graphics2DSettings;

    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init(&platformSettings);
    NEXUS_Platform_GetConfiguration(&platformConfig);

    BKNI_CreateEvent(&fb_callback_event);

#if NEXUS_DTV_PLATFORM
    NEXUS_Display_GetDefaultSettings(&displaySettings);
    display = NEXUS_Display_Open(0, &displaySettings);
    NEXUS_Display_AddOutput(display, NEXUS_PanelOutput_GetConnector(platformConfig.outputs.panel[0]));
    NEXUS_BoardCfg_ConfigurePanel(true, true, true);
    fprintf(stderr, "Panel output ready\n");
#else
    NEXUS_Display_GetDefaultSettings(&displaySettings);
    displaySettings.format = NEXUS_VideoFormat_e1080i;
    display = NEXUS_Display_Open(0, &displaySettings);
    NEXUS_Display_AddOutput(display, NEXUS_ComponentOutput_GetConnector(platformConfig.outputs.component[0]));
#if NEXUS_NUM_HDMI_OUTPUTS
    if (platformConfig.outputs.hdmi[0]) {
        NEXUS_Display_AddOutput(display, NEXUS_HdmiOutput_GetVideoConnector(platformConfig.outputs.hdmi[0]));
    }
#endif
#endif

    NEXUS_VideoFormat_GetInfo(displaySettings.format, &videoFormatInfo);
    NEXUS_Surface_GetDefaultCreateSettings(&surfaceCreateSettings);
    surfaceCreateSettings.pixelFormat = NEXUS_PixelFormat_eY08_Cb8_Y18_Cr8;
    surfaceCreateSettings.width = destRect.width;
    surfaceCreateSettings.height = destRect.height;
#if BCHP_CHIP == 7420
    surfaceCreateSettings.heap = platformConfig.heap[1];
#endif
    framebuffer[0] = NEXUS_Surface_Create(&surfaceCreateSettings);
    framebuffer[1] = NEXUS_Surface_Create(&surfaceCreateSettings);
#if BCHP_CHIP == 7420
    surfaceCreateSettings.heap = NULL;
#endif

    NEXUS_Display_GetGraphicsSettings(display, &graphicsSettings);
    graphicsSettings.enabled = true;
    /* graphicsSettings.position will default to the display size */
    graphicsSettings.clip.width = surfaceCreateSettings.width;
    graphicsSettings.clip.height = surfaceCreateSettings.height;
    graphicsSettings.frameBufferCallback.callback = fb_callback;
    graphicsSettings.frameBufferCallback.context = fb_callback_event;
    rc = NEXUS_Display_SetGraphicsSettings(display, &graphicsSettings);
    BDBG_ASSERT(!rc);

    /* don't set fb until first picture is ready */

    gfx = NEXUS_Graphics2D_Open(0, NULL);

    NEXUS_Graphics2D_GetSettings(gfx, &graphics2DSettings);
    graphics2DSettings.pollingCheckpoint = true;
    rc = NEXUS_Graphics2D_SetSettings(gfx, &graphics2DSettings);
    BDBG_ASSERT(!rc);

    /* Create two pseudo-framebufffers for blending */
    newSurface = NEXUS_Surface_Create(&surfaceCreateSettings);
    oldSurface = NEXUS_Surface_Create(&surfaceCreateSettings);

    if ( strcmp(argv[1], "-auto") )
        first=1;
    else
        first=2;    /* Skip first arg, it's the -auto flag */

    for ( i = first, total=0; i < argc; total++ )
    {
        /* Decompress Surface */
        NEXUS_SurfaceHandle surface;

        gettimeofday(&timeStart, NULL);

        surface = b_decompress_jpeg(argv[i], surfaceCreateSettings.width, surfaceCreateSettings.height);

        if ( NULL != surface )
        {
            NEXUS_Rect copyrect;
            NEXUS_SurfaceCreateSettings surfaceCreateSetings;

            NEXUS_Surface_Flush(surface);

            NEXUS_Surface_GetCreateSettings(surface, &surfaceCreateSetings);

            if ( (((surfaceCreateSetings.width)*540)/surfaceCreateSetings.height) > 960 )
            {
                /* Image is too wide, letterbox */
                copyrect.x = 0;
                copyrect.width = destRect.width;
                copyrect.height = 2*((surfaceCreateSetings.height * 960)/surfaceCreateSetings.width);
                copyrect.y = ((destRect.height - copyrect.height)/2)&~1;
            }
            else
            {
                /* Image is too tall, pillarbox */
                copyrect.y = 0;
                copyrect.height = destRect.height;
                copyrect.width = ((surfaceCreateSetings.width * 540) / surfaceCreateSetings.height)&~1;
                copyrect.x = ((destRect.width - copyrect.width)/2)&~1;
            }
            /* Fill with black for letterbox */
            b_surface_fill(gfx, newSurface, &destRect, 0xff000000);
            b_surface_copy(gfx, newSurface, &copyrect, surface, NULL);
            NEXUS_Surface_Destroy(surface);

            if ( 0==total )
            {
                /* Full image copy */
                b_surface_copy(gfx, framebuffer[cur_fb], &destRect, newSurface, &destRect);
                flip();
            }
            else
            {
                #define TRANS_FIELDS 45 /* 1/2 second */
                int j, trans;
                trans=total%4;
                if ( first == 1 )
                {
                    /* Not auto mode... */
                    printf("Press ENTER to continue\n");
                    (void)getchar();
                }
                switch ( trans )
                {
                case 0:
                    /* Dissolve */
                    for ( j=1; j<=TRANS_FIELDS; j++ )
                    {
                        unsigned long alpha = ((255*j)/TRANS_FIELDS);
                        NEXUS_Graphics2DBlitSettings blitSettings;

                        NEXUS_Graphics2D_GetDefaultBlitSettings(&blitSettings);
                        blitSettings.source.surface = newSurface;
                        blitSettings.source.rect = destRect;
                        blitSettings.dest.surface = oldSurface;
                        blitSettings.output.surface = framebuffer[cur_fb];
                        blitSettings.output.rect = destRect;
                        blitSettings.colorOp = NEXUS_BlitColorOp_eUseConstantAlpha;
                        blitSettings.alphaOp = NEXUS_BlitAlphaOp_eCopyConstant;
                        blitSettings.constantColor = alpha << 24;
                        rc = NEXUS_Graphics2D_Blit(gfx, &blitSettings);
                        BDBG_ASSERT(!rc);
                        BKNI_Sleep(5); /* if we go too fast, we miss the effect */
                        flip();
                    }
                    break;
                case 1:
                    /* Wipe from right */
                    for ( j=1; j<=TRANS_FIELDS; j++ )
                    {
                        NEXUS_Rect rect=destRect;
                        rect.width = ((destRect.width*(TRANS_FIELDS-j))/TRANS_FIELDS)&~1;
                        if (rect.width > 0 )
                            b_surface_copy(gfx, framebuffer[cur_fb], &rect, oldSurface, &rect);
                        rect.x = rect.width;
                        rect.width = destRect.width - rect.x;
                        if ( rect.width > 0 )
                            b_surface_copy(gfx, framebuffer[cur_fb], &rect, newSurface, &rect);
                        flip();
                    }
                    break;
                case 3:
                    /* Split shutter horizontal reveal effect */
                    for ( j=1; j<=TRANS_FIELDS; j++ )
                    {
                        int k;
                        /* We're going to work in 24 steps, (1080/24)=45) */
                        b_surface_copy(gfx, framebuffer[cur_fb], &destRect, newSurface, &destRect);
                        for ( k=0; k<24; k+=2 )
                        {
                            /* Even pass, move to right side */
                            NEXUS_Rect rect=destRect;
                            NEXUS_Rect srcrect=destRect;
                            srcrect.width = rect.width = ((destRect.width*(TRANS_FIELDS-j))/TRANS_FIELDS)&~1;
                            rect.x = destRect.width - rect.width;
                            srcrect.height = rect.height = destRect.height/24;
                            srcrect.y = rect.y = (destRect.height*k)/24;
                            if ( srcrect.width > 0 )
                                b_surface_copy(gfx, framebuffer[cur_fb], &rect, oldSurface, &srcrect);
                        }
                        for ( k=1; k<24; k+=2 )
                        {
                            /* Odd pass, move to left side */
                            NEXUS_Rect rect=destRect;
                            NEXUS_Rect srcrect=destRect;
                            srcrect.width = rect.width = ((destRect.width*(TRANS_FIELDS-j))/TRANS_FIELDS)&~1;
                            srcrect.x = destRect.width - rect.width;
                            srcrect.height = rect.height = destRect.height/24;
                            srcrect.y = rect.y = (destRect.height*k)/24;
                            if ( srcrect.width > 0 )
                                b_surface_copy(gfx, framebuffer[cur_fb], &rect, oldSurface, &srcrect);
                        }
                        flip();
                    }
                    break;
                case 2:
                    /* Box wipe */
                    b_surface_copy(gfx, framebuffer[cur_fb], &destRect, oldSurface, &destRect);
                    for ( j=1; j<=TRANS_FIELDS; j++ )
                    {
                        NEXUS_Rect rect;
                        rect.width = ((destRect.width*j)/TRANS_FIELDS)&~1;
                        rect.x = ((destRect.width - rect.width)/2)&~1;
                        rect.height = ((destRect.height*j)/TRANS_FIELDS)&~1;
                        rect.y = ((destRect.height - rect.height)/2)&~1;
                        b_surface_copy(gfx, framebuffer[cur_fb], &rect, newSurface, &rect);
                        flip();
                    }
                    break;
                default:
                    break;
                }
            }
        }

        /* Loop after last image */
        if ( ++i >= argc )
            i=first;

        /* Rotate two surfaces */
        b_surface_copy(gfx, oldSurface, &destRect, newSurface, &destRect);

        /* Wait... */
        if ( first > 1 )
        {
            struct timespec sleepTime;
            gettimeofday(&timeEnd, NULL);

            sleepTime.tv_sec = 4 - (timeEnd.tv_sec - timeStart.tv_sec);
            sleepTime.tv_nsec = 1000000000 - (1000 *(timeEnd.tv_usec - timeStart.tv_usec));
            if ( sleepTime.tv_nsec > 1000000000 )
            {
                sleepTime.tv_sec++;
                sleepTime.tv_nsec -= 1000000000;
            }
            nanosleep(&sleepTime, NULL);
        }
    }

    return 0;
}

