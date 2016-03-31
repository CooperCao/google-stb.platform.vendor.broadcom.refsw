/***************************************************************************
 *     Copyright (c) 2009-2010, Broadcom Corporation
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
 *    Reference JPEG Decode into a framebuffer
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *************************************************************************/
#include "nexus_platform.h"
#include "nexus_display.h"
#include "nexus_graphics2d.h"
#include "nexus_core_utils.h"
#include <stdio.h>

#include "b_decode_jpeg.h"

int main(int argc, char **argv)
{
    NEXUS_SurfaceHandle surface;
    NEXUS_SurfaceCreateSettings surfaceCreateSettings;
    NEXUS_DisplayHandle display;
    NEXUS_VideoFormatInfo videoFormatInfo;
    NEXUS_GraphicsSettings graphicsSettings;
    NEXUS_PlatformSettings platformSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_DisplaySettings displaySettings;
    NEXUS_Error rc;
    NEXUS_SurfaceHandle framebuffer;
    NEXUS_Graphics2DHandle gfx;
    NEXUS_Graphics2DSettings graphics2DSettings;
    int i;

    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init(&platformSettings);
    NEXUS_Platform_GetConfiguration(&platformConfig);

#if NEXUS_DTV_PLATFORM
    NEXUS_Display_GetDefaultSettings(&displaySettings);
    display = NEXUS_Display_Open(0, &displaySettings);
    NEXUS_Display_AddOutput(display, NEXUS_PanelOutput_GetConnector(platformConfig.outputs.panel[0]));
    NEXUS_BoardCfg_ConfigurePanel(true, true, true);
#else
    NEXUS_Display_GetDefaultSettings(&displaySettings);
    displaySettings.format = NEXUS_VideoFormat_e720p;
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
    surfaceCreateSettings.pixelFormat = NEXUS_PixelFormat_eA8_R8_G8_B8;
    surfaceCreateSettings.width = videoFormatInfo.width;
    surfaceCreateSettings.height = videoFormatInfo.height;
#if BCHP_CHIP == 7420
    surfaceCreateSettings.heap = platformConfig.heap[1];
#endif
    framebuffer = NEXUS_Surface_Create(&surfaceCreateSettings);

    NEXUS_Display_GetGraphicsSettings(display, &graphicsSettings);
    graphicsSettings.enabled = true;
    /* graphicsSettings.position will default to the display size */
    graphicsSettings.clip.width = surfaceCreateSettings.width;
    graphicsSettings.clip.height = surfaceCreateSettings.height;
    rc = NEXUS_Display_SetGraphicsSettings(display, &graphicsSettings);
    BDBG_ASSERT(!rc);

    rc = NEXUS_Display_SetGraphicsFramebuffer(display, framebuffer);
    BDBG_ASSERT(!rc);

    gfx = NEXUS_Graphics2D_Open(0, NULL);

    NEXUS_Graphics2D_GetSettings(gfx, &graphics2DSettings);
    graphics2DSettings.pollingCheckpoint = true;
    rc = NEXUS_Graphics2D_SetSettings(gfx, &graphics2DSettings);
    BDBG_ASSERT(!rc);

    for (i=1;i<argc;i++) {
        surface = b_decompress_jpeg(argv[i], surfaceCreateSettings.width, surfaceCreateSettings.height);
        if (!surface) {
            printf("Unable to decode %s\n", argv[i]);
        }
        else {
            NEXUS_Graphics2DBlitSettings blitSettings;

            /* libjpeg is sw decode; therefore we must flusht the CPU cache before blitting. */
            NEXUS_Surface_Flush(surface);

            /* this is a simple full surface -> full surface blit, without aspect ratio correction. */
            NEXUS_Graphics2D_GetDefaultBlitSettings(&blitSettings);
            blitSettings.source.surface = surface;
            blitSettings.output.surface = framebuffer;
            rc = NEXUS_Graphics2D_Blit(gfx, &blitSettings);
            BDBG_ASSERT(!rc);

            /* must know the blit has completed before we can destroy the surface.
            just do a simple polling loop. */
            while (NEXUS_Graphics2D_Checkpoint(gfx, NULL)) {
                BKNI_Sleep(1);
            }

            NEXUS_Surface_Destroy(surface);
        }
        printf("Press ENTER to continue.\n");
        getchar();
    }

    return 0;
}

