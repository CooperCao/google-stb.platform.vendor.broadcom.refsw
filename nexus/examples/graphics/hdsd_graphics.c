/******************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 *
 * Module Description:
 * This example creates 2 frame buffer surfaces of SD size in heap 1 and fills the SD buffer with purple color
 * using fill operation and then sets the SD buffer as the frame buffer for SD display and then for all the
 * video formats available in HD display, the SD buffer is blit_copied to HD buffer and set as frame buffer
 * for HD display.
 * 
 *****************************************************************************************************************/

#include "nexus_platform.h"
#include <stdio.h>

#if NEXUS_HAS_DISPLAY
#include "nexus_surface.h"
#if NEXUS_HAS_GRAPHICS2D
#include "nexus_graphics2d.h" /* not required */
#endif
#include "nexus_display.h"
#include "nexus_component_output.h"
#include "nexus_composite_output.h"
#if NEXUS_HAS_HDMI_OUTPUT
#include "nexus_hdmi_output.h"
#endif
#include "nexus_core_utils.h"
#include "bstd.h"
#include "bkni.h"
#include <stdlib.h>

/* For set-tops, the VEC can support multiple formats. The graphics framebuffer must adapt. */
#define TOTAL_FORMATS 4
NEXUS_VideoFormat g_format[TOTAL_FORMATS] =
{
    NEXUS_VideoFormat_eNtsc,
    NEXUS_VideoFormat_ePal,
    NEXUS_VideoFormat_e720p,
    NEXUS_VideoFormat_e1080i
};

/* Provide support for chips with M2MC */
static void fill(NEXUS_SurfaceHandle surface, uint32_t color);
static void copy(NEXUS_SurfaceHandle toSurface, NEXUS_SurfaceHandle fromSurface, const NEXUS_Rect *destRect);

static void complete(void *context, int param)
{
    BSTD_UNUSED(param);
    BKNI_SetEvent((BKNI_EventHandle)context);
}

int main(void)
{
    NEXUS_SurfaceHandle framebufferHD, framebufferSD1, framebufferSD2;
    NEXUS_SurfaceCreateSettings createSettings;
    NEXUS_DisplayHandle displayHD, displaySD1 = NULL, displaySD2 = NULL;
    NEXUS_DisplaySettings displaySettings;
    NEXUS_GraphicsSettings graphicsSettings;
    NEXUS_PlatformSettings platformSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_VideoFormatInfo videoFormatInfo;
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_HdmiOutputStatus hdmiStatus;
#endif
    NEXUS_Error rc;
    unsigned i;

    /* Bring up all modules for a platform in a default configuraiton for this platform */
    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init(&platformSettings);
    NEXUS_Platform_GetConfiguration(&platformConfig);

    NEXUS_Display_GetDefaultSettings(&displaySettings);
    displaySettings.format = NEXUS_VideoFormat_e720p;
    displayHD = NEXUS_Display_Open(0, &displaySettings);

#if NEXUS_NUM_COMPONENT_OUTPUTS
    NEXUS_Display_AddOutput(displayHD, NEXUS_ComponentOutput_GetConnector(platformConfig.outputs.component[0]));
#endif
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_Display_AddOutput(displayHD, NEXUS_HdmiOutput_GetVideoConnector(platformConfig.outputs.hdmi[0]));
    rc = NEXUS_HdmiOutput_GetStatus(platformConfig.outputs.hdmi[0], &hdmiStatus);
    if ( !rc && hdmiStatus.connected )
    {
        /* If current display format is not supported by monitor, switch to monitor's preferred format. 
           If other connected outputs do not support the preferred format, a harmless error will occur. */
        NEXUS_Display_GetSettings(displayHD, &displaySettings);
        if ( !hdmiStatus.videoFormatSupported[displaySettings.format] ) {
            displaySettings.format = hdmiStatus.preferredVideoFormat;
            NEXUS_Display_SetSettings(displayHD, &displaySettings);
        }
    }
#endif

#if NEXUS_NUM_DISPLAYS > 1 && NEXUS_NUM_COMPOSITE_OUTPUTS
    NEXUS_Display_GetDefaultSettings(&displaySettings);
    displaySettings.format = NEXUS_VideoFormat_eNtsc;
    displaySD1 = NEXUS_Display_Open(1, &displaySettings);
    NEXUS_Display_AddOutput(displaySD1, NEXUS_CompositeOutput_GetConnector(platformConfig.outputs.composite[0]));
#endif

#if NEXUS_NUM_DISPLAYS > 2 && NEXUS_NUM_COMPOSITE_OUTPUTS > 1
    NEXUS_Display_GetDefaultSettings(&displaySettings);
    displaySettings.format = NEXUS_VideoFormat_eNtsc;
    displaySD2 = NEXUS_Display_Open(2, &displaySettings);
    NEXUS_Display_AddOutput(displaySD2, NEXUS_CompositeOutput_GetConnector(platformConfig.outputs.composite[1]));
#endif

    NEXUS_Surface_GetDefaultCreateSettings(&createSettings);
    createSettings.pixelFormat = NEXUS_PixelFormat_eA8_R8_G8_B8;
    createSettings.width = 720;
    createSettings.height = 480; /* The height is fixed to 480 and will be upscaled */
    createSettings.heap = NEXUS_Platform_GetFramebufferHeap(0);
    framebufferHD = NEXUS_Surface_Create(&createSettings);

    NEXUS_Surface_GetDefaultCreateSettings(&createSettings);
    createSettings.pixelFormat = NEXUS_PixelFormat_eA8_R8_G8_B8;
    createSettings.width = 720; /* The width is fixed to 720 and will be upscaled */
    createSettings.height = 480; /* max SD height. For PAL SD outputs, use 576. */
    createSettings.heap = NEXUS_Platform_GetFramebufferHeap(1);
    framebufferSD1 = NEXUS_Surface_Create(&createSettings);

    /* fill whole fb w/ purple. blue is already used by CFE splash, so we want this to be different. */
    fill(framebufferSD1, 0xffff00ff);

    if (displaySD2) {
        NEXUS_Surface_GetDefaultCreateSettings(&createSettings);
        createSettings.pixelFormat = NEXUS_PixelFormat_eA8_R8_G8_B8;
        createSettings.width = 720; /* The width is fixed to 720 and will be upscaled */
        createSettings.height = 480; /* max SD height. For PAL SD outputs, use 576. */
        createSettings.heap = NEXUS_Platform_GetFramebufferHeap(2);
        framebufferSD2 = NEXUS_Surface_Create(&createSettings);

        /* fill whole fb w/ purple. blue is already used by CFE splash, so we want this to be different. */
        fill(framebufferSD2, 0xffff00ff);
    }

    if (displaySD1) {
        NEXUS_Display_GetSettings(displaySD1, &displaySettings);
        NEXUS_VideoFormat_GetInfo(displaySettings.format, &videoFormatInfo);
        NEXUS_Display_GetGraphicsSettings(displaySD1, &graphicsSettings);
        graphicsSettings.enabled = true;
        graphicsSettings.position.width = videoFormatInfo.width;
        graphicsSettings.position.height = videoFormatInfo.height;
        graphicsSettings.clip.width = 720;
        graphicsSettings.clip.height = videoFormatInfo.height;
        rc = NEXUS_Display_SetGraphicsSettings(displaySD1, &graphicsSettings);
        BDBG_ASSERT(!rc);
        
        rc = NEXUS_Display_SetGraphicsFramebuffer(displaySD1, framebufferSD1);
        BDBG_ASSERT(!rc);
    }
    if (displaySD2) {
        rc = NEXUS_Display_SetGraphicsFramebuffer(displaySD2, framebufferSD2);
        BDBG_ASSERT(!rc);
    }

    /* change formats on HD output */
    for (i=0;i<TOTAL_FORMATS;i++) {
        NEXUS_Display_GetSettings(displayHD, &displaySettings);
        displaySettings.format = g_format[i];
        rc = NEXUS_Display_SetSettings(displayHD, &displaySettings);
        BDBG_ASSERT(!rc);
        NEXUS_VideoFormat_GetInfo(displaySettings.format, &videoFormatInfo);

        NEXUS_Display_GetGraphicsSettings(displayHD, &graphicsSettings);

        /* when the display format changes, Nexus automatically disables graphics
           to allow the app to recomposite and scale before showing graphics again. */
        graphicsSettings.enabled = true;

        /* the position rect sets the destination and is always equal to the full display */
        graphicsSettings.position.width = videoFormatInfo.width;
        graphicsSettings.position.height = videoFormatInfo.height;

        /* the clipping is the portion of the framebuffer we will use. */
        graphicsSettings.clip.width = 720; /* the difference between clip.width and position.width will trigger a GFD horizontal upscale. */
        graphicsSettings.clip.height = 480; /* the difference between clip.height and position.height will trigger a GFD vertical upscale. */
        rc = NEXUS_Display_SetGraphicsSettings(displayHD, &graphicsSettings);
        BDBG_ASSERT(!rc);

        /* blit into the clipped region of the framebuffer that will be displayed. */
        copy(framebufferHD, framebufferSD1, &graphicsSettings.clip);
        
        /* Now we can grow graphics */
        rc = NEXUS_Display_SetGraphicsFramebuffer(displayHD, framebufferHD);
        BDBG_ASSERT(!rc);

        printf("offscreen %dx%d -> framebuffer %dx%d -> display %dx%d\n",
            createSettings.width, createSettings.height,
            graphicsSettings.clip.width, graphicsSettings.clip.height,
            videoFormatInfo.width, videoFormatInfo.height);

        BKNI_Sleep(10*1000);
    }

    return 0;
}

#if NEXUS_HAS_GRAPHICS2D
static NEXUS_Graphics2DHandle gfx = NULL;
static BKNI_EventHandle checkpointEvent;
static void b_init_gfx(void)
{
    NEXUS_Graphics2DSettings gfxSettings;
    
    gfx = NEXUS_Graphics2D_Open(0, NULL);
    BKNI_CreateEvent(&checkpointEvent);
    NEXUS_Graphics2D_GetSettings(gfx, &gfxSettings);
    gfxSettings.checkpointCallback.callback = complete;
    gfxSettings.checkpointCallback.context = checkpointEvent;
    NEXUS_Graphics2D_SetSettings(gfx, &gfxSettings);
}

static void fill(NEXUS_SurfaceHandle surface, uint32_t color)
{
    NEXUS_Graphics2DFillSettings fillSettings;
    NEXUS_Error rc;

    if (!gfx) {
        b_init_gfx();
    }

    NEXUS_Graphics2D_GetDefaultFillSettings(&fillSettings);
    fillSettings.surface = surface;
    fillSettings.color = color;
    rc = NEXUS_Graphics2D_Fill(gfx, &fillSettings);
    BDBG_ASSERT(!rc);
    
    rc = NEXUS_Graphics2D_Checkpoint(gfx, NULL);
    if (rc == NEXUS_GRAPHICS2D_QUEUED) {
        rc = BKNI_WaitForEvent(checkpointEvent, BKNI_INFINITE);
    }
    BDBG_ASSERT(!rc);
}

static void copy(NEXUS_SurfaceHandle toSurface, NEXUS_SurfaceHandle fromSurface, const NEXUS_Rect *destRect)
{
    NEXUS_Graphics2DBlitSettings blitSettings;
    NEXUS_Error rc;

    if (!gfx) {
        b_init_gfx();
    }

    NEXUS_Graphics2D_GetDefaultBlitSettings(&blitSettings);
    blitSettings.source.surface = fromSurface;
    blitSettings.output.surface = toSurface;
    blitSettings.output.rect = *destRect;
    rc = NEXUS_Graphics2D_Blit(gfx, &blitSettings);
    BDBG_ASSERT(!rc);

    rc = NEXUS_Graphics2D_Checkpoint(gfx, NULL);
    if (rc == NEXUS_GRAPHICS2D_QUEUED) {
        rc = BKNI_WaitForEvent(checkpointEvent, BKNI_INFINITE);
    }
    BDBG_ASSERT(!rc);
}
#else
static void fill(NEXUS_SurfaceHandle surface, uint32_t color)
{
    NEXUS_SurfaceMemory mem;
    NEXUS_SurfaceCreateSettings createSettings;
    uint32_t *ptr;
    unsigned i,j;

    NEXUS_Surface_GetMemory(surface, &mem);
    ptr = mem.buffer;
    NEXUS_Surface_GetCreateSettings(surface, &createSettings);
    for (i=0;i<createSettings.height;i++) {
        for (j=0;j<createSettings.width;j++) {
            ptr[j] = color;
        }
        ptr += mem.pitch / sizeof(uint32_t);
    }
    NEXUS_Surface_Flush(surface);
}

static void copy(NEXUS_SurfaceHandle toSurface, NEXUS_SurfaceHandle fromSurface, const NEXUS_Rect *destRect)
{
    /* TODO: add simple point sample scale. for now, just fill w/ a different color */
    BSTD_UNUSED(fromSurface);
    BSTD_UNUSED(destRect);
    fill(toSurface, 0xffff0000);
}
#endif

#else /* NEXUS_HAS_DISPLAY */
int main(void)
{
    printf("This application is not supported on this platform (needs display)!\n");
    return 0;
}
#endif
