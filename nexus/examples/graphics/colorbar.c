/***************************************************************************
 *  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 ***************************************************************************/
#include "nexus_platform.h"
#include <stdio.h>
#include <string.h>

#if NEXUS_HAS_GRAPHICS2D
#include "nexus_surface.h"
#include "nexus_graphics2d.h"
#include "nexus_display.h"
#include "nexus_composite_output.h"
#include "nexus_component_output.h"
#include "nexus_core_utils.h"
#include "bstd.h"
#include "bkni.h"
#include <stdlib.h>

#undef min
#define min(A,B) ((A)<(B)?(A):(B))

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

static void complete(void *context, int param)
{
    BSTD_UNUSED(param);
    BKNI_SetEvent((BKNI_EventHandle)context);
}

int main(int argc, char **argv)
{
    NEXUS_SurfaceHandle framebuffer0, framebuffer1=NULL, surface;
    NEXUS_SurfaceCreateSettings hdCreateSettings, sdCreateSettings;
    NEXUS_PlatformCapabilities platformCap;
    NEXUS_DisplayHandle display0, display1 = NULL;
    NEXUS_DisplaySettings displaySettings;
    NEXUS_DisplayCapabilities displayCap;
    NEXUS_GraphicsSettings graphicsSettings;
    NEXUS_Graphics2DHandle gfx;
    NEXUS_SurfaceMemory mem;
    NEXUS_Graphics2DSettings gfxSettings;
    BKNI_EventHandle checkpointEvent;
    NEXUS_PlatformSettings platformSettings;
    NEXUS_Graphics2DBlitSettings blitSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_VideoFormatInfo info;
    NEXUS_Error rc;
    int i,j;
    bool sd_display;
    bool pal;
    unsigned colorWidth, pitchinInt;

    pal = (argc > 1 && !strcmp(argv[1], "-pal"));

    /* Bring up all modules for a platform in a default configuraiton for this platform */
    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    rc = NEXUS_Platform_Init(&platformSettings);
    if (rc) return -1;
    NEXUS_Platform_GetConfiguration(&platformConfig);
    NEXUS_GetPlatformCapabilities(&platformCap);
    NEXUS_GetDisplayCapabilities(&displayCap);
    sd_display = platformCap.display[1].supported && !platformCap.display[1].encoder && displayCap.display[1].graphics.width;

    /* HD display */
    NEXUS_Display_GetDefaultSettings(&displaySettings);
    displaySettings.format = pal?NEXUS_VideoFormat_e1080i50hz:NEXUS_VideoFormat_e720p;
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
    hdCreateSettings.width = min(displayCap.display[0].graphics.width, info.width);
    hdCreateSettings.height = min(displayCap.display[0].graphics.height, info.height);
    hdCreateSettings.heap = NEXUS_Platform_GetFramebufferHeap(0);
    surface = NEXUS_Surface_Create(&hdCreateSettings);
    framebuffer0 = NEXUS_Surface_Create(&hdCreateSettings);

    if (sd_display) {
        NEXUS_Display_GetDefaultSettings(&displaySettings);
        displaySettings.format = pal?NEXUS_VideoFormat_ePalI:NEXUS_VideoFormat_eNtsc;
        display1 = NEXUS_Display_Open(1, &displaySettings);
        BDBG_ASSERT(display1);

#if NEXUS_NUM_COMPOSITE_OUTPUTS
        NEXUS_Display_AddOutput(display1, NEXUS_CompositeOutput_GetConnector(platformConfig.outputs.composite[0]));
#endif
#if NEXUS_NUM_COMPOSITE_OUTPUTS >=2
        NEXUS_Display_AddOutput(display1, NEXUS_CompositeOutput_GetConnector(platformConfig.outputs.composite[1]));
#endif
#if NEXUS_NUM_RFM_OUTPUTS
        if (platformConfig.outputs.rfm[0]) {
            NEXUS_Display_AddOutput(display1, NEXUS_Rfm_GetVideoConnector(platformConfig.outputs.rfm[0]));
        }
#endif

        NEXUS_VideoFormat_GetInfo(displaySettings.format, &info);
        NEXUS_Surface_GetDefaultCreateSettings(&sdCreateSettings);
        sdCreateSettings.pixelFormat = NEXUS_PixelFormat_eA8_R8_G8_B8;
        sdCreateSettings.width = min(displayCap.display[1].graphics.width, info.width);
        sdCreateSettings.height = min(displayCap.display[1].graphics.height, info.height);
        sdCreateSettings.heap = NEXUS_Platform_GetFramebufferHeap(1);
        framebuffer1= NEXUS_Surface_Create(&sdCreateSettings);
    }

    gfx = NEXUS_Graphics2D_Open(0, NULL);
    BKNI_CreateEvent(&checkpointEvent);
    NEXUS_Graphics2D_GetSettings(gfx, &gfxSettings);
    gfxSettings.checkpointCallback.callback = complete;
    gfxSettings.checkpointCallback.context = checkpointEvent;
    NEXUS_Graphics2D_SetSettings(gfx, &gfxSettings);

    /* draw for the framebuffer */
    NEXUS_Surface_GetMemory(surface, &mem);
    colorWidth = hdCreateSettings.width/NUM_COLORS;
    pitchinInt = mem.pitch/sizeof(uint32_t);
    for (j=0;j<hdCreateSettings.height;j++) {
        for (i=0;i<hdCreateSettings.width;i++) {
            unsigned colorIndex = min(i/colorWidth, NUM_COLORS-1);
            *((uint32_t*)mem.buffer + j*pitchinInt + i) = g_colors[colorIndex];
        }
    }
    NEXUS_Surface_Flush(surface);

    NEXUS_Graphics2D_GetDefaultBlitSettings(&blitSettings);
    blitSettings.source.surface = surface;
    blitSettings.output.surface = framebuffer0;
    NEXUS_Graphics2D_Blit(gfx, &blitSettings);
    if (display1) {
        NEXUS_Graphics2D_GetDefaultBlitSettings(&blitSettings);
        blitSettings.source.surface = surface;
        BDBG_ASSERT(framebuffer1);
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
    graphicsSettings.clip.width = hdCreateSettings.width;
    graphicsSettings.clip.height = hdCreateSettings.height;
    NEXUS_Display_SetGraphicsSettings(display0, &graphicsSettings);
    NEXUS_Display_SetGraphicsFramebuffer(display0, framebuffer0);

    if (display1) {
        NEXUS_Display_GetGraphicsSettings(display1, &graphicsSettings);
        graphicsSettings.enabled = true;
        graphicsSettings.clip.width = sdCreateSettings.width;
        graphicsSettings.clip.height = sdCreateSettings.height;
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
    NEXUS_Surface_Destroy(surface);
    NEXUS_Surface_Destroy(framebuffer0);
    BKNI_DestroyEvent(checkpointEvent);
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
