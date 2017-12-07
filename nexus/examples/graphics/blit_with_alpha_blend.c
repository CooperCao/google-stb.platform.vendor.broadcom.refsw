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
 *****************************************************************************/
#include "nexus_platform.h"
#include <stdio.h>
#include <string.h>

#if NEXUS_HAS_GRAPHICS2D
#include "nexus_surface.h"
#include "nexus_graphics2d.h"
#include "nexus_display.h"
#include "nexus_component_output.h"
#if NEXUS_HAS_HDMI_OUTPUT
#include "nexus_hdmi_output.h"
#endif
#include "nexus_core_utils.h"
#include "bstd.h"
#include "bkni.h"
#include <stdlib.h>

#undef min
#define min(A,B) ((A)<(B)?(A):(B))

/* this is a blit example, so make the fill terse */
void simple_fill(NEXUS_Graphics2DHandle gfx, NEXUS_SurfaceHandle surface, uint32_t color)
{
    NEXUS_Graphics2DFillSettings fillSettings;
    NEXUS_Error rc;

    NEXUS_Graphics2D_GetDefaultFillSettings(&fillSettings);
    fillSettings.surface = surface;
    fillSettings.color = color;
    rc = NEXUS_Graphics2D_Fill(gfx, &fillSettings);
    BDBG_ASSERT(!rc);
}

int main(int argc, char **argv)
{
    NEXUS_SurfaceHandle framebuffer, red_surface, green_surface;
    NEXUS_SurfaceCreateSettings createSurfaceSettings;
    NEXUS_DisplayHandle display;
    NEXUS_DisplaySettings displaySettings;
    NEXUS_DisplayCapabilities displayCap;
    NEXUS_GraphicsSettings graphicsSettings;
    NEXUS_Graphics2DHandle gfx;
    NEXUS_Graphics2DBlitSettings blitSettings;
    NEXUS_Graphics2DSettings gfxSettings;
    NEXUS_PlatformSettings platformSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_VideoFormatInfo info;
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_HdmiOutputStatus hdmiStatus;
#endif
    NEXUS_Error rc;
    NEXUS_Rect red_rect = {100,100,200,200}; /* x,y coordinates are framebuffer coordinates */
    NEXUS_Rect green_rect = {150,150,200,200}; /* x,y coordinates are framebuffer coordinates */
    bool use_equation = false;

    if (argc > 1) {
        use_equation = !strcmp(argv[1], "-eq");
    }

    /* Bring up all modules for a platform in a default configuraiton for this platform */
    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init(&platformSettings);
    NEXUS_Platform_GetConfiguration(&platformConfig);
    NEXUS_GetDisplayCapabilities(&displayCap);

    NEXUS_Display_GetDefaultSettings(&displaySettings);
    displaySettings.format = NEXUS_VideoFormat_e720p;
    display = NEXUS_Display_Open(0, &displaySettings);

#if NEXUS_NUM_COMPONENT_OUTPUTS
    NEXUS_Display_AddOutput(display, NEXUS_ComponentOutput_GetConnector(platformConfig.outputs.component[0]));
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

    NEXUS_Surface_GetDefaultCreateSettings(&createSurfaceSettings);
    createSurfaceSettings.pixelFormat = NEXUS_PixelFormat_eA8_R8_G8_B8;
    createSurfaceSettings.width = min(displayCap.display[0].graphics.width, info.width);
    createSurfaceSettings.height = min(displayCap.display[0].graphics.height, info.height);
    createSurfaceSettings.heap = NEXUS_Platform_GetFramebufferHeap(0);
    framebuffer = NEXUS_Display_CreateFramebuffer(display, &createSurfaceSettings);

    gfx = NEXUS_Graphics2D_Open(0, NULL);
    /* checkpoint isn't needed in this app because it's only blits, but this code is included as an example of polling checkpoint */
    NEXUS_Graphics2D_GetSettings(gfx, &gfxSettings);
    gfxSettings.pollingCheckpoint = true;
    rc = NEXUS_Graphics2D_SetSettings(gfx, &gfxSettings);
    BDBG_ASSERT(!rc);

    simple_fill(gfx, framebuffer, 0x00000000); /* clear framebuffer */

    NEXUS_Display_GetGraphicsSettings(display, &graphicsSettings);
    graphicsSettings.enabled = true;
    graphicsSettings.clip.width = createSurfaceSettings.width;
    graphicsSettings.clip.height = createSurfaceSettings.height;
    NEXUS_Display_SetGraphicsSettings(display, &graphicsSettings);
    NEXUS_Display_SetGraphicsFramebuffer(display, framebuffer);

    NEXUS_Surface_GetDefaultCreateSettings(&createSurfaceSettings);
    createSurfaceSettings.pixelFormat = NEXUS_PixelFormat_eA8_R8_G8_B8;
    createSurfaceSettings.width = red_rect.width;
    createSurfaceSettings.height = red_rect.height;
    red_surface = NEXUS_Surface_Create(&createSurfaceSettings);
    simple_fill(gfx, red_surface, 0xFFFF0000); /* red */

    NEXUS_Surface_GetDefaultCreateSettings(&createSurfaceSettings);
    createSurfaceSettings.pixelFormat = NEXUS_PixelFormat_eA8_R8_G8_B8;
    createSurfaceSettings.width = green_rect.width;
    createSurfaceSettings.height = green_rect.height;
    green_surface = NEXUS_Surface_Create(&createSurfaceSettings);
    simple_fill(gfx, green_surface, 0xFF00FF00); /* green */

    printf("blit 1: no blend.\n");
    NEXUS_Graphics2D_GetDefaultBlitSettings(&blitSettings);
    blitSettings.source.surface = red_surface; /* no source.rect means blit from entire source */
    blitSettings.output.surface = framebuffer;
    blitSettings.output.rect = red_rect;
    blitSettings.colorOp = NEXUS_BlitColorOp_eCopySource;
    blitSettings.alphaOp = NEXUS_BlitAlphaOp_eCopySource;
    rc = NEXUS_Graphics2D_Blit(gfx, &blitSettings);
    BDBG_ASSERT(!rc);
    NEXUS_Graphics2D_GetDefaultBlitSettings(&blitSettings);
    blitSettings.source.surface = green_surface;
    blitSettings.output.surface = framebuffer;
    blitSettings.output.rect = green_rect;
    if (use_equation) {
        NEXUS_BlendEquation colorEq = { NEXUS_BlendFactor_eSourceColor, NEXUS_BlendFactor_eOne, 0,
            NEXUS_BlendFactor_eZero, NEXUS_BlendFactor_eZero, 0, NEXUS_BlendFactor_eZero };
        NEXUS_BlendEquation alphaEq = { NEXUS_BlendFactor_eSourceAlpha, NEXUS_BlendFactor_eOne, 0,
            NEXUS_BlendFactor_eZero, NEXUS_BlendFactor_eZero, 0, NEXUS_BlendFactor_eZero };
        blitSettings.colorOp = NEXUS_BlitColorOp_eUseBlendEquation;
        blitSettings.colorBlend = colorEq;
        blitSettings.alphaOp = NEXUS_BlitAlphaOp_eUseBlendEquation;
        blitSettings.alphaBlend = alphaEq;
    }
    else {
        blitSettings.colorOp = NEXUS_BlitColorOp_eCopySource;
        blitSettings.alphaOp = NEXUS_BlitAlphaOp_eCopySource;
    }
    rc = NEXUS_Graphics2D_Blit(gfx, &blitSettings);
    BDBG_ASSERT(!rc);
    while (NEXUS_Graphics2D_Checkpoint(gfx, NULL)); /* loop until done */
    getchar();

    simple_fill(gfx, framebuffer, 0); /* clear framebuffer */

    printf("blit 2: constant alpha.\n");
    NEXUS_Graphics2D_GetDefaultBlitSettings(&blitSettings);
    blitSettings.source.surface = red_surface;
    blitSettings.output.surface = framebuffer;
    blitSettings.output.rect = red_rect;
    blitSettings.colorOp = NEXUS_BlitColorOp_eCopySource;
    blitSettings.alphaOp = NEXUS_BlitAlphaOp_eCopySource;
    rc = NEXUS_Graphics2D_Blit(gfx, &blitSettings);
    BDBG_ASSERT(!rc);
    NEXUS_Graphics2D_GetDefaultBlitSettings(&blitSettings);
    blitSettings.source.surface = green_surface;
    blitSettings.dest.surface = framebuffer; /* dest required for blend. */
    blitSettings.dest.rect = green_rect; /* dest.rect specifies the pixels that will be blended with source */
    blitSettings.output.surface = framebuffer;
    blitSettings.output.rect = green_rect; /* output.rect can be different from dest.rect, but it's not in this app. */
    blitSettings.constantColor = 0x80 << 24;
    if (use_equation) {
        NEXUS_BlendEquation colorEq = { NEXUS_BlendFactor_eSourceColor, NEXUS_BlendFactor_eConstantAlpha, 0,
            NEXUS_BlendFactor_eDestinationColor, NEXUS_BlendFactor_eInverseConstantAlpha, 0, NEXUS_BlendFactor_eZero };
        NEXUS_BlendEquation alphaEq = { NEXUS_BlendFactor_eSourceAlpha, NEXUS_BlendFactor_eOne, 0,
            NEXUS_BlendFactor_eZero, NEXUS_BlendFactor_eZero, 0, NEXUS_BlendFactor_eZero };
        blitSettings.colorOp = NEXUS_BlitColorOp_eUseBlendEquation;
        blitSettings.colorBlend = colorEq;
        blitSettings.alphaOp = NEXUS_BlitAlphaOp_eUseBlendEquation;
        blitSettings.alphaBlend = alphaEq;
    }
    else {
        blitSettings.colorOp = NEXUS_BlitColorOp_eUseConstantAlpha;
        blitSettings.alphaOp = NEXUS_BlitAlphaOp_eCopySource;
    }
    rc = NEXUS_Graphics2D_Blit(gfx, &blitSettings);
    BDBG_ASSERT(!rc);
    while (NEXUS_Graphics2D_Checkpoint(gfx, NULL)); /* loop until done */
    getchar();

    simple_fill(gfx, framebuffer, 0); /* clear framebuffer */
    simple_fill(gfx, green_surface, 0xA000FF00); /* green_surface gets an alpha now */

    printf("blit 3: source alpha.\n");
    NEXUS_Graphics2D_GetDefaultBlitSettings(&blitSettings);
    blitSettings.source.surface = red_surface;
    blitSettings.output.surface = framebuffer;
    blitSettings.output.rect = red_rect;
    blitSettings.colorOp = NEXUS_BlitColorOp_eCopySource;
    blitSettings.alphaOp = NEXUS_BlitAlphaOp_eCopySource;
    rc = NEXUS_Graphics2D_Blit(gfx, &blitSettings);
    BDBG_ASSERT(!rc);
    NEXUS_Graphics2D_GetDefaultBlitSettings(&blitSettings);
    blitSettings.source.surface = green_surface;
    blitSettings.dest.surface = framebuffer;
    blitSettings.dest.rect = green_rect;
    blitSettings.output.surface = framebuffer;
    blitSettings.output.rect = green_rect;
    blitSettings.constantColor = 0xFF << 24; /* constant not used for blend. used for framebuffer's final alpha to blend with video. */
    if (use_equation) {
        NEXUS_BlendEquation colorEq = { NEXUS_BlendFactor_eSourceColor, NEXUS_BlendFactor_eSourceAlpha, 0,
            NEXUS_BlendFactor_eDestinationColor, NEXUS_BlendFactor_eInverseSourceAlpha, 0, NEXUS_BlendFactor_eZero };
        NEXUS_BlendEquation alphaEq = { NEXUS_BlendFactor_eConstantAlpha, NEXUS_BlendFactor_eOne, 0,
            NEXUS_BlendFactor_eZero, NEXUS_BlendFactor_eZero, 0, NEXUS_BlendFactor_eZero };
        blitSettings.colorOp = NEXUS_BlitColorOp_eUseBlendEquation;
        blitSettings.colorBlend = colorEq;
        blitSettings.alphaOp = NEXUS_BlitAlphaOp_eUseBlendEquation;
        blitSettings.alphaBlend = alphaEq;
    }
    else {
        blitSettings.colorOp = NEXUS_BlitColorOp_eUseSourceAlpha;
        blitSettings.alphaOp = NEXUS_BlitAlphaOp_eCopyConstant;
    }
    rc = NEXUS_Graphics2D_Blit(gfx, &blitSettings);
    BDBG_ASSERT(!rc);
    while (NEXUS_Graphics2D_Checkpoint(gfx, NULL)); /* loop until done */
    getchar();

    NEXUS_Display_Close(display);
    NEXUS_Surface_Destroy(framebuffer);
    NEXUS_Surface_Destroy(red_surface);
    NEXUS_Surface_Destroy(green_surface);
    NEXUS_Graphics2D_Close(gfx);
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
