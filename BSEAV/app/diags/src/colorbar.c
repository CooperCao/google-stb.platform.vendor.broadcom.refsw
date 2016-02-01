/***************************************************************************
 *     Copyright (c) 2003-2013, Broadcom Corporation
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
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/
#include "nexus_platform.h"
#include "nexus_surface.h"
#include "nexus_graphics2d.h"
#include "nexus_display.h"
#include "nexus_composite_output.h"
#include "nexus_component_output.h"
#include "nexus_core_utils.h"
#include "bstd.h"
#include "bkni.h"
#include "prompt.h"
#include <stdlib.h>
#include <stdio.h>

/***********************************************************************
 *                       Global Variables
 ***********************************************************************/

/***********************************************************************
 *                      External References
 ***********************************************************************/
extern NEXUS_DisplayHandle bcmBackendDisplay[];

/***********************************************************************
 *                      Function Prototypes
 ***********************************************************************/

/***********************************************************************
 *                      Global variables
 ***********************************************************************/

/* SMPTE color bars */

#define NUM_COLORS 7
static uint32_t g_colors[NUM_COLORS] = {
        0xFFFFFFFF, /* white */
        0xFFFFFF00, /* yellow */
        0xFF00FFFF, /* cyan */
        0xFF00FF00, /* green */
        0xFFFF00FF, /* magenta */
        0xFFFF0000, /* red */
        0xFF0000FF  /* blue */
    };

#ifdef DEFAULT_IS_PAL
    #define DEFAULT_SD_VIDEOFORMAT NEXUS_VideoFormat_ePalI
    #define DEFAULT_HD_VIDEOFORMAT NEXUS_VideoFormat_e1080i50hz
#else
    #define DEFAULT_SD_VIDEOFORMAT NEXUS_VideoFormat_eNtsc
    #define DEFAULT_HD_VIDEOFORMAT NEXUS_VideoFormat_e720p /*NEXUS_VideoFormat_e1080i*/
#endif

int bcmColorbarTest(void)
{
    NEXUS_SurfaceHandle framebuffer0, framebuffer1;
    NEXUS_SurfaceCreateSettings hdCreateSettings, sdCreateSettings;
    NEXUS_DisplayHandle display0, display1 = NULL;
    NEXUS_DisplaySettings displaySettings;
    NEXUS_GraphicsSettings graphicsSettings;
    NEXUS_Graphics2DHandle gfx;
    NEXUS_Graphics2DFillSettings fillSettings;
    NEXUS_Graphics2DBlitSettings blitSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_VideoFormatInfo info;
    int i;  
    bool sd_display = true;

    /* Bring up all modules for a platform in a default configuraiton for this platform */
    NEXUS_Platform_GetConfiguration(&platformConfig);

    /* HD display */
    NEXUS_Display_GetDefaultSettings(&displaySettings);
    displaySettings.format = DEFAULT_HD_VIDEOFORMAT;
    display0 = NEXUS_Display_Open(0, &displaySettings);
    #if NEXUS_NUM_COMPONENT_OUTPUTS
    NEXUS_Display_AddOutput(display0, NEXUS_ComponentOutput_GetConnector(platformConfig.outputs.component[0]));
#endif
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_Display_AddOutput(display0, NEXUS_HdmiOutput_GetVideoConnector(platformConfig.outputs.hdmi[0]));
    #endif
        
    NEXUS_VideoFormat_GetInfo(displaySettings.format, &info);
    NEXUS_Surface_GetDefaultCreateSettings(&hdCreateSettings);
    hdCreateSettings.pixelFormat = NEXUS_PixelFormat_eA8_R8_G8_B8;
    hdCreateSettings.width = info.width/2;
    hdCreateSettings.height = info.height;
    hdCreateSettings.heap = NEXUS_Platform_GetFramebufferHeap(0);
    framebuffer0 = NEXUS_Surface_Create(&hdCreateSettings);

    if (sd_display) {
        NEXUS_Display_GetDefaultSettings(&displaySettings);
        displaySettings.format = DEFAULT_SD_VIDEOFORMAT;
        display1 = NEXUS_Display_Open(1, &displaySettings);
            
#if NEXUS_NUM_COMPOSITE_OUTPUTS
        NEXUS_Display_AddOutput(display1, NEXUS_CompositeOutput_GetConnector(platformConfig.outputs.composite[0]));
#endif
#if NEXUS_NUM_COMPOSITE_OUTPUTS >=2
        NEXUS_Display_AddOutput(display1, NEXUS_CompositeOutput_GetConnector(platformConfig.outputs.composite[1]));
#endif
            
        NEXUS_VideoFormat_GetInfo(displaySettings.format, &info);
        NEXUS_Surface_GetDefaultCreateSettings(&sdCreateSettings);
        sdCreateSettings.pixelFormat = NEXUS_PixelFormat_eA8_R8_G8_B8;
        sdCreateSettings.width = info.width;
        sdCreateSettings.height = info.height;
        sdCreateSettings.heap = NEXUS_Platform_GetFramebufferHeap(1);
        framebuffer1 = NEXUS_Surface_Create(&sdCreateSettings);
    }
        
    gfx = NEXUS_Graphics2D_Open(0, NULL);
        
    /* draw for the HD display */
    NEXUS_Graphics2D_GetDefaultFillSettings(&fillSettings);
    fillSettings.surface = framebuffer0;
    fillSettings.rect.width = hdCreateSettings.width/NUM_COLORS;
    fillSettings.rect.y = 0;
    fillSettings.rect.height = hdCreateSettings.height;
    for (i=0;i<NUM_COLORS;i++) {
        fillSettings.rect.x = fillSettings.rect.width * i;
        fillSettings.color = g_colors[i];
        NEXUS_Graphics2D_Fill(gfx, &fillSettings);
    }
        
    if (display1) {
        NEXUS_Graphics2D_GetDefaultBlitSettings(&blitSettings);
        blitSettings.source.surface = framebuffer0;
        blitSettings.output.surface = framebuffer1;
        NEXUS_Graphics2D_Blit(gfx, &blitSettings);
    }
    
    /* needed to execute queued blits */
    NEXUS_Graphics2D_Checkpoint(gfx, NULL);

    NEXUS_Display_GetGraphicsSettings(display0, &graphicsSettings);
    graphicsSettings.enabled = true;
    graphicsSettings.clip.x = 0;
    graphicsSettings.clip.y = 0;
    graphicsSettings.clip.width = hdCreateSettings.width;
    graphicsSettings.clip.height = hdCreateSettings.height;
    NEXUS_Display_SetGraphicsSettings(display0, &graphicsSettings);
    NEXUS_Display_SetGraphicsFramebuffer(display0, framebuffer0);
    
    if (display1) {
        NEXUS_Display_GetGraphicsSettings(display1, &graphicsSettings);
        graphicsSettings.enabled = true;
        NEXUS_Display_SetGraphicsSettings(display1, &graphicsSettings);
        NEXUS_Display_SetGraphicsFramebuffer(display1, framebuffer1);
    }
        
    printf("Press any key to exit\n");
    getchar();
      
    NEXUS_Graphics2D_Close(gfx);
    if (display1) {
        NEXUS_Display_Close(display1);
        NEXUS_Surface_Destroy(framebuffer1);
    }
    NEXUS_Display_Close(display0);
    NEXUS_Surface_Destroy(framebuffer0);
    return 0;
}
