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
 *****************************************************************************/
/* Nexus example app: Perform both Porter-Duff Fill and Blit operations */

#include "nexus_platform.h"
#include <stdio.h>
#if NEXUS_HAS_GRAPHICS2D && NEXUS_HAS_DISPLAY
#include "nexus_surface.h"
#include "nexus_graphics2d.h"
#include "nexus_display.h"
#include "nexus_composite_output.h"
#include "nexus_component_output.h"
#if NEXUS_HAS_HDMI_OUTPUT
#include "nexus_hdmi_output.h"
#endif
#include "nexus_core_utils.h"
#include "bstd.h"
#include "bkni.h"
#include <stdlib.h>

/* Defines */
#define TIMEOUT     (4*1000)
#define BOX_WIDTH   150
#define BOX_HEIGHT  150
#define BOX_L_OFS_X 50
#define BOX_L_OFS_Y 100
#define BOX_R_OFS_X 400
#define BOX_R_OFS_Y 100
#define BOX1_X(ofs) (ofs)
#define BOX1_Y(ofs) (ofs)
#define BOX2_X(ofs) (BOX1_X(ofs) + 2*BOX_WIDTH/3)
#define BOX2_Y(ofs) (BOX1_Y(ofs) + 1*BOX_HEIGHT/3)
#define BOX3_X(ofs) (BOX1_X(ofs) + 1*BOX_WIDTH/3)
#define BOX3_Y(ofs) (BOX1_Y(ofs) + 2*BOX_HEIGHT/3)

static char *porterDuffToString[] =
{
    "Clear",                    /* NEXUS_PorterDuffOp_eClear   (Erase output)  */
    "Source",                   /* NEXUS_PorterDuffOp_eSrc     (Copy source to output) */
    "Destination",              /* NEXUS_PorterDuffOp_eDst     (Copy dest to output) */
    "Source OVER Destination",  /* NEXUS_PorterDuffOp_eSrcOver (Source over dest) */
    "Destination OVER Source",  /* NEXUS_PorterDuffOp_eDstOver (Dest over source) */
    "Source IN Destination",    /* NEXUS_PorterDuffOp_eSrcIn   (Part of source within dest) */
    "Destination IN Source",    /* NEXUS_PorterDuffOp_eDstIn   (Part of dest within source) */
    "Source OUT Destination",   /* NEXUS_PorterDuffOp_eSrcOut  (Part of source not in dest) */
    "Destination OUT Source",   /* NEXUS_PorterDuffOp_eDstOut  (Part of dest not in source) */
    "Source ATOP Destination",  /* NEXUS_PorterDuffOp_eSrcAtop (Part of source in dest and remainder is dest) */
    "Destination ATOP Source",  /* NEXUS_PorterDuffOp_eDstAtop (Part of dest in source and remainder is source) */
    "Source XOR Destination",   /* NEXUS_PorterDuffOp_eXor     (Part of src not in dest and part of dest not in source) */
};

static void complete(void *data, int unused)
{
    BSTD_UNUSED(unused);
    BKNI_SetEvent((BKNI_EventHandle)data);
}

int main(void)
{
    NEXUS_SurfaceHandle framebuffer, boxSurface;
    NEXUS_SurfaceCreateSettings createSettings;
    NEXUS_DisplayHandle display;
    NEXUS_DisplaySettings displaySettings;
    NEXUS_GraphicsSettings graphicsSettings;
    NEXUS_Graphics2DHandle gfx;
    NEXUS_Graphics2DSettings gfxSettings;
    NEXUS_Graphics2DPorterDuffFillSettings framebufferFillSettings;
    NEXUS_Graphics2DPorterDuffFillSettings boxFillSettings;
    NEXUS_Graphics2DPorterDuffBlitSettings blitSettings;
    NEXUS_PlatformSettings platformSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_VideoFormatInfo info;
    NEXUS_PorterDuffOp operation;
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_HdmiOutputStatus hdmiStatus;
#endif
    BKNI_EventHandle spaceAvailableEvent, checkpointEvent;
    NEXUS_Error rc;

    /* Bring up all modules for a platform in a default configuraiton for this platform */
    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init(&platformSettings);
    NEXUS_Platform_GetConfiguration(&platformConfig);

    NEXUS_Display_GetDefaultSettings(&displaySettings);
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

    /* Create box surface */
    NEXUS_Surface_GetDefaultCreateSettings(&createSettings);
    createSettings.pixelFormat = NEXUS_PixelFormat_eA8_R8_G8_B8;
    createSettings.width = BOX_WIDTH;
    createSettings.height = BOX_HEIGHT;
    boxSurface = NEXUS_Surface_Create(&createSettings);

    /* Create primary surface (framebuffer) */
    createSettings.width = info.width;
    createSettings.height = info.height;
    createSettings.heap = NEXUS_Platform_GetFramebufferHeap(0);
    framebuffer = NEXUS_Surface_Create(&createSettings);

    /* spaceAvailableEvent is unused in this app. if we were doing a rapid porter duff
    fill, the queue would run out of space. then we wait on this event, reissue
    the blit and continue. */
    BKNI_CreateEvent(&spaceAvailableEvent);
    BKNI_CreateEvent(&checkpointEvent);

    /* Open Graphics2D Module */
    gfx = NEXUS_Graphics2D_Open(0, NULL);

    /* Setup Graphics2D callback */
    NEXUS_Graphics2D_GetSettings(gfx, &gfxSettings);
    gfxSettings.packetSpaceAvailable.callback = complete;
    gfxSettings.packetSpaceAvailable.context = spaceAvailableEvent;
    gfxSettings.checkpointCallback.callback = complete;
    gfxSettings.checkpointCallback.context = checkpointEvent;
    NEXUS_Graphics2D_SetSettings(gfx, &gfxSettings);

    /* Clear whole screen to light yellow */
    NEXUS_Graphics2D_GetDefaultPorterDuffFillSettings(&framebufferFillSettings);

    framebufferFillSettings.surface = framebuffer;
    framebufferFillSettings.rect.x = 0;
    framebufferFillSettings.rect.y = 0;
    framebufferFillSettings.rect.width = createSettings.width;
    framebufferFillSettings.rect.height = createSettings.height;
    framebufferFillSettings.color = 0x40FFFF77;
    framebufferFillSettings.operation = NEXUS_PorterDuffOp_eSrc;
    NEXUS_Graphics2D_PorterDuffFill(gfx, &framebufferFillSettings);

    /* Enable Graphics layer and set framebuffer */
    NEXUS_Display_GetGraphicsSettings(display, &graphicsSettings);
    graphicsSettings.enabled = true;
    graphicsSettings.sourceBlendFactor = NEXUS_CompositorBlendFactor_eConstantAlpha;
    graphicsSettings.constantAlpha = 0xFF;
    NEXUS_Display_SetGraphicsSettings(display, &graphicsSettings);
    NEXUS_Display_SetGraphicsFramebuffer(display, framebuffer);

    /* Setup default fill settings for box surface fills */
    NEXUS_Graphics2D_GetDefaultPorterDuffFillSettings(&boxFillSettings);
    boxFillSettings.surface = boxSurface;
    boxFillSettings.rect.x = 0;
    boxFillSettings.rect.y = 0;
    boxFillSettings.rect.width = BOX_WIDTH;
    boxFillSettings.rect.height = BOX_HEIGHT;
    boxFillSettings.operation = NEXUS_PorterDuffOp_eSrc;

    /* Setup default blit settings when blitting from box surface to framebuffer */
    NEXUS_Graphics2D_GetDefaultPorterDuffBlitSettings(&blitSettings);
    blitSettings.sourceSurface = boxSurface;
    blitSettings.sourceRect.x = 0;
    blitSettings.sourceRect.y = 0;
    blitSettings.sourceRect.width = BOX_WIDTH;
    blitSettings.sourceRect.height = BOX_HEIGHT;
    blitSettings.destSurface = framebuffer;
    blitSettings.destRect.width = BOX_WIDTH;
    blitSettings.destRect.height = BOX_HEIGHT;
    blitSettings.outSurface = framebuffer;
    blitSettings.outRect.width = BOX_WIDTH;
    blitSettings.outRect.height = BOX_HEIGHT;

    fprintf(stderr, "\n*** Left Side the screen is Porter-Duff Fill, right side is Porter-Duff Blit ***\n");

    /* Test each Porter-Duff Fill and Blit Operations */
    for (operation = 0; operation < NEXUS_PorterDuffOp_eMax; operation++)
    {
        fprintf(stderr, "Performing a Porter-Duff %s composition...\n", porterDuffToString[operation]);

        /*** Left side the screen is the fill Porter-Duff composition ***/
        framebufferFillSettings.operation = operation;

        /* fill RED box with alpha */
        framebufferFillSettings.rect.x = BOX1_X(BOX_L_OFS_X);
        framebufferFillSettings.rect.y = BOX1_Y(BOX_L_OFS_Y);
        framebufferFillSettings.rect.width = BOX_WIDTH;
        framebufferFillSettings.rect.height = BOX_HEIGHT;
        framebufferFillSettings.color = 0x77ff0000;
        NEXUS_Graphics2D_PorterDuffFill(gfx, &framebufferFillSettings);

        /* fill GREEN box with alpha */
        framebufferFillSettings.rect.x = BOX2_X(BOX_L_OFS_X);
        framebufferFillSettings.rect.y = BOX2_Y(BOX_L_OFS_Y);
        framebufferFillSettings.rect.width = BOX_WIDTH;
        framebufferFillSettings.rect.height = BOX_HEIGHT;
        framebufferFillSettings.color = 0x7700ff00;
        NEXUS_Graphics2D_PorterDuffFill(gfx, &framebufferFillSettings);

        /* fill BLUE box with alpha */
        framebufferFillSettings.rect.x = BOX3_X(BOX_L_OFS_X);
        framebufferFillSettings.rect.y = BOX3_Y(BOX_L_OFS_Y);
        framebufferFillSettings.rect.width = BOX_WIDTH;
        framebufferFillSettings.rect.height = BOX_HEIGHT;
        framebufferFillSettings.color = 0x770000ff;
        NEXUS_Graphics2D_PorterDuffFill(gfx, &framebufferFillSettings);

        /*** Right side the screen is the blit Porter-Duff composition ***/
        framebufferFillSettings.operation = operation;

        /* fill RED box with alpha */
        boxFillSettings.color = 0x77ff0000;
        NEXUS_Graphics2D_PorterDuffFill(gfx, &boxFillSettings);

        /* Do a Porter-Duff Blit from the box surface to the framebuffer */
        blitSettings.operation = operation;

        blitSettings.destRect.x = BOX1_X(BOX_R_OFS_X);
        blitSettings.destRect.y = BOX1_X(BOX_R_OFS_Y);
        blitSettings.outRect.x  = BOX1_X(BOX_R_OFS_X);
        blitSettings.outRect.y  = BOX1_X(BOX_R_OFS_Y);
        NEXUS_Graphics2D_PorterDuffBlit(gfx, &blitSettings);

        /* fill GREEN box with alpha */
        boxFillSettings.color = 0x7700ff00;
        NEXUS_Graphics2D_PorterDuffFill(gfx, &boxFillSettings);

        /* Do a Porter-Duff Blit from the box surface to the framebuffer */
        blitSettings.destRect.x = BOX2_X(BOX_R_OFS_X);
        blitSettings.destRect.y = BOX2_Y(BOX_R_OFS_Y);
        blitSettings.outRect.x  = BOX2_X(BOX_R_OFS_X);
        blitSettings.outRect.y  = BOX2_Y(BOX_R_OFS_Y);
        NEXUS_Graphics2D_PorterDuffBlit(gfx, &blitSettings);

        /* fill BLUE box with alpha */
        boxFillSettings.color = 0x770000ff;
        NEXUS_Graphics2D_PorterDuffFill(gfx, &boxFillSettings);

        /* Do a Porter-Duff Blit from the box surface to the framebuffer */
        blitSettings.destRect.x = BOX3_X(BOX_R_OFS_X);
        blitSettings.destRect.y = BOX3_Y(BOX_R_OFS_Y);
        blitSettings.outRect.x  = BOX3_X(BOX_R_OFS_X);
        blitSettings.outRect.y  = BOX3_Y(BOX_R_OFS_Y);
        NEXUS_Graphics2D_PorterDuffBlit(gfx, &blitSettings);
        
        rc = NEXUS_Graphics2D_Checkpoint(gfx, NULL);
        if (rc == NEXUS_GRAPHICS2D_QUEUED) {
            BKNI_WaitForEvent(checkpointEvent, BKNI_INFINITE);
        }

        BKNI_Sleep(TIMEOUT);

        /* Clear whole screen to light yellow */
        framebufferFillSettings.rect.x = 0;
        framebufferFillSettings.rect.y = 0;
        framebufferFillSettings.rect.width = createSettings.width;
        framebufferFillSettings.rect.height = createSettings.height;
        framebufferFillSettings.color = 0x40FFFF77;
        framebufferFillSettings.operation = NEXUS_PorterDuffOp_eSrc;
        NEXUS_Graphics2D_PorterDuffFill(gfx, &framebufferFillSettings);
        
        rc = NEXUS_Graphics2D_Checkpoint(gfx, NULL);
        if (rc == NEXUS_GRAPHICS2D_QUEUED) {
            BKNI_WaitForEvent(checkpointEvent, BKNI_INFINITE);
        }
    }
    fprintf(stderr, "Done.\n");

    NEXUS_Graphics2D_Close(gfx);
    NEXUS_Display_Close(display);
    NEXUS_Surface_Destroy(framebuffer);
    NEXUS_Surface_Destroy(boxSurface);
    BKNI_DestroyEvent(spaceAvailableEvent);
    BKNI_DestroyEvent(checkpointEvent);
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
