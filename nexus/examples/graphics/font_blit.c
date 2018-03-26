/******************************************************************************
 * Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#include "bstd.h"
#include "bkni.h"
#include <stdlib.h>

BDBG_MODULE(blit);

#define GLYPH_WIDTH 10
#define GLYPH_HEIGHT 20
static const uint8_t g_glyph[GLYPH_WIDTH*GLYPH_HEIGHT] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0xD0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x40,
    0x00, 0xD0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x40,
    0x00, 0xD0, 0xA0, 0xA0, 0xFF, 0xFF, 0xA0, 0xA0, 0xFF, 0x40,
    0x00, 0xD0, 0x40, 0x80, 0xFF, 0xFF, 0x80, 0x40, 0xFF, 0x40,
    0x00, 0x80, 0x40, 0x20, 0xFF, 0xFF, 0x40, 0x40, 0x80, 0x00,
    0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x40, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x40, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x40, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x40, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x40, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x40, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x40, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x40, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x20, 0xFF, 0xFF, 0x40, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x40, 0xFF, 0xFF, 0x40, 0x00, 0x00, 0x00,
    0x00, 0x20, 0xA0, 0xFF, 0xFF, 0xFF, 0xFF, 0xA0, 0x20, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

static void complete(void *data, int unused)
{
    BSTD_UNUSED(unused);
    BKNI_SetEvent((BKNI_EventHandle)data);
}

int main(void)
{
    NEXUS_SurfaceHandle framebuffer, glyphSurface;
    NEXUS_SurfaceCreateSettings createSettings;
    NEXUS_SurfaceMemory mem;
    NEXUS_DisplayHandle display;
    NEXUS_DisplaySettings displaySettings;
    NEXUS_GraphicsSettings graphicsSettings;
    NEXUS_Graphics2DHandle gfx;
    NEXUS_Graphics2DFillSettings fillSettings;
    NEXUS_Graphics2DSettings gfxSettings;
    BKNI_EventHandle checkpointEvent;
    NEXUS_PlatformSettings platformSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_VideoFormatInfo info;
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_HdmiOutputStatus hdmiStatus;
#endif
    int i;
    NEXUS_Error rc;

    /* Bring up all modules for a platform in a default configuraiton for this platform */
    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init(&platformSettings);
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
    createSettings.heap = NEXUS_Platform_GetFramebufferHeap(0);
    framebuffer = NEXUS_Surface_Create(&createSettings);

    NEXUS_Surface_GetDefaultCreateSettings(&createSettings);
    createSettings.pixelFormat = NEXUS_PixelFormat_eA8;
    createSettings.width = GLYPH_WIDTH;
    createSettings.height = GLYPH_HEIGHT;
    glyphSurface = NEXUS_Surface_Create(&createSettings);
    NEXUS_Surface_GetMemory(glyphSurface, &mem);
    BDBG_ASSERT(mem.pitch >= GLYPH_WIDTH);
    for (i=0;i<createSettings.height;i++) {
        unsigned char *ptr = (unsigned char *)mem.buffer + i * mem.pitch;
        BKNI_Memcpy(ptr, &g_glyph[GLYPH_WIDTH*i], GLYPH_WIDTH);
    }
    NEXUS_Surface_Flush(glyphSurface);

    BKNI_CreateEvent(&checkpointEvent);

    gfx = NEXUS_Graphics2D_Open(0, NULL);

    NEXUS_Graphics2D_GetSettings(gfx, &gfxSettings);
    gfxSettings.checkpointCallback.callback = complete;
    gfxSettings.checkpointCallback.context = checkpointEvent;
    NEXUS_Graphics2D_SetSettings(gfx, &gfxSettings);

    NEXUS_Graphics2D_GetDefaultFillSettings(&fillSettings);
    fillSettings.surface = framebuffer;
    fillSettings.rect.x = 0; /* TODO: rect not needed for whole surface */
    fillSettings.rect.y = 0;
    fillSettings.rect.width = info.width;
    fillSettings.rect.height= info.height;
    fillSettings.color = 0xFFFFFFFF; /* white background */
    rc = NEXUS_Graphics2D_Fill(gfx, &fillSettings);
    BDBG_ASSERT(!rc);
    rc = NEXUS_Graphics2D_Checkpoint(gfx, NULL);
    if (rc == NEXUS_GRAPHICS2D_QUEUED) {
        rc = BKNI_WaitForEvent(checkpointEvent, 3000);
        BDBG_ASSERT(!rc);
    }

    NEXUS_Display_GetGraphicsSettings(display, &graphicsSettings);
    graphicsSettings.enabled = true;
    rc = NEXUS_Display_SetGraphicsSettings(display, &graphicsSettings);
    BDBG_ASSERT(!rc);

    {
        NEXUS_Graphics2DBlitSettings blitSettings;

        /* set up the blit once. only change output coordinates on each iteration. */
        NEXUS_Graphics2D_GetDefaultBlitSettings(&blitSettings);
        blitSettings.source.surface = glyphSurface;
        blitSettings.dest.surface = framebuffer;
        blitSettings.output.surface = framebuffer;
        /* blitSettings.output.rect.x and .y set each iteration */
        blitSettings.output.rect.width = GLYPH_WIDTH; /* no scaling */
        blitSettings.output.rect.height = GLYPH_HEIGHT;
        blitSettings.colorOp = NEXUS_BlitColorOp_eUseBlendEquation;
        blitSettings.colorBlend.a = NEXUS_BlendFactor_eSourceColor;
        blitSettings.colorBlend.b = NEXUS_BlendFactor_eSourceAlpha;
        blitSettings.colorBlend.subtract_cd = false;
        blitSettings.colorBlend.c = NEXUS_BlendFactor_eDestinationColor;
        blitSettings.colorBlend.d = NEXUS_BlendFactor_eInverseSourceAlpha;
        blitSettings.colorBlend.subtract_e = false;
        blitSettings.colorBlend.e = NEXUS_BlendFactor_eZero;
        blitSettings.alphaOp = NEXUS_BlitAlphaOp_eCopyConstant;
        blitSettings.constantColor = 0xFF0000FF; /* blue, opaque font */

        BDBG_WRN(("starting"));
        for(i=0;i<100;i++) {
            blitSettings.output.rect.x = 100 + (i%10) * (GLYPH_WIDTH+4);
            blitSettings.output.rect.y = 100 + (i/10) * (GLYPH_HEIGHT+4);
            blitSettings.dest.rect = blitSettings.output.rect; /* required for blend */
            rc = NEXUS_Graphics2D_Blit(gfx, &blitSettings);
            BDBG_ASSERT(!rc);

            rc = NEXUS_Graphics2D_Checkpoint(gfx, NULL);
            if (rc == NEXUS_GRAPHICS2D_QUEUED) {
                rc = BKNI_WaitForEvent(checkpointEvent, 3000);
                BDBG_ASSERT(!rc);
            }
        }
    }

    rc = NEXUS_Display_SetGraphicsFramebuffer(display, framebuffer);
    BDBG_ASSERT(!rc);

    BDBG_WRN(("press ENTER to exit"));
    getchar();

    NEXUS_Display_Close(display);
    NEXUS_Surface_Destroy(framebuffer);
    NEXUS_Surface_Destroy(glyphSurface);
    NEXUS_Graphics2D_Close(gfx);
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
