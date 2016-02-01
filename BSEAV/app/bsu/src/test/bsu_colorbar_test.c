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
#include <stdio.h>
#include "nexus_platform.h"
#include "nexus_surface.h"
#include "nexus_graphics2d.h"
#include "nexus_display.h"
#include "nexus_composite_output.h"
#include "nexus_component_output.h"
#include "nexus_core_utils.h"
#include "bstd.h"
#include "bkni.h"
#include "bsu_prompt.h"
#include "bsu-api.h"
#include "bsu-api2.h"

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

static void complete(void *context, int param)
{
    BSTD_UNUSED(param);
    BKNI_SetEvent((BKNI_EventHandle)context);
}

void bsu_colorbar_test(void)
{
    NEXUS_SurfaceHandle framebuffer0, framebuffer1;
    NEXUS_SurfaceCreateSettings hdCreateSettings, sdCreateSettings;
    NEXUS_DisplayHandle display0, display1 = NULL;
    NEXUS_DisplaySettings displaySettings;
    NEXUS_GraphicsSettings graphicsSettings;
    NEXUS_Graphics2DHandle gfx;
    NEXUS_Graphics2DSettings gfxSettings;
    BKNI_EventHandle checkpointEvent;
    NEXUS_Graphics2DFillSettings fillSettings;
    NEXUS_Graphics2DBlitSettings blitSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_VideoFormatInfo info;
    NEXUS_Error rc;
    int i;  
    bool sd_display = true;
    /* TODO: cmdline option for hd/sd, pal/ntsc */

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
    hdCreateSettings.width = info.width;
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
#if NEXUS_NUM_RFM_OUTPUTS
        NEXUS_Display_AddOutput(display1, NEXUS_Rfm_GetVideoConnector(platformConfig.outputs.rfm[0]));
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
    BKNI_CreateEvent(&checkpointEvent);
    NEXUS_Graphics2D_GetSettings(gfx, &gfxSettings);
    gfxSettings.checkpointCallback.callback = complete;
    gfxSettings.checkpointCallback.context = checkpointEvent;
    NEXUS_Graphics2D_SetSettings(gfx, &gfxSettings);
        
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
    rc = NEXUS_Graphics2D_Checkpoint(gfx, NULL);
    if (rc == NEXUS_GRAPHICS2D_QUEUED) {
        rc = BKNI_WaitForEvent(checkpointEvent, BKNI_INFINITE);
    }
    BDBG_ASSERT(!rc);

    NEXUS_Display_GetGraphicsSettings(display0, &graphicsSettings);
    graphicsSettings.enabled = true;
    NEXUS_Display_SetGraphicsSettings(display0, &graphicsSettings);
    NEXUS_Display_SetGraphicsFramebuffer(display0, framebuffer0);
    
    if (display1) {
        NEXUS_Display_GetGraphicsSettings(display1, &graphicsSettings);
        graphicsSettings.enabled = true;
        NEXUS_Display_SetGraphicsSettings(display1, &graphicsSettings);
        NEXUS_Display_SetGraphicsFramebuffer(display1, framebuffer1);
    }
        
    printf("Press any key to exit\n");
    GetChar();
      
    NEXUS_Graphics2D_Close(gfx);
    if (display1) {
        NEXUS_Display_Close(display1);
        NEXUS_Surface_Destroy(framebuffer1);
    }
    NEXUS_Display_Close(display0);
    NEXUS_Surface_Destroy(framebuffer0);
    BKNI_DestroyEvent(checkpointEvent);
    return;
}
