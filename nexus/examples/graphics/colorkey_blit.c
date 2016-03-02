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
 *****************************************************************************/

#include "nexus_platform.h"
#include <stdio.h>

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

BDBG_MODULE(blit);

void complete(void *data, int unused)
{
    BSTD_UNUSED(unused);
    BKNI_SetEvent((BKNI_EventHandle)data);
}

int main(void)
{
    NEXUS_SurfaceHandle surface;
    NEXUS_SurfaceCreateSettings createSettings;
    NEXUS_DisplayHandle display;
    NEXUS_DisplaySettings displaySettings;
    NEXUS_GraphicsSettings graphicsSettings;
    NEXUS_Graphics2DHandle gfx;
    NEXUS_Graphics2DFillSettings fillSettings;
    NEXUS_Graphics2DFillSettings eraseBoxFillSettings;
    NEXUS_Graphics2DSettings gfxSettings;
    BKNI_EventHandle checkpointEvent, spaceAvailableEvent;
    NEXUS_PlatformSettings platformSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_VideoFormatInfo info;
    NEXUS_Error rc;
    NEXUS_Graphics2DBlitSettings blitSettings;
    NEXUS_Rect box = {50,50,100,100};

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
#endif

    NEXUS_VideoFormat_GetInfo(displaySettings.format, &info);

    NEXUS_Surface_GetDefaultCreateSettings(&createSettings);
    createSettings.pixelFormat = NEXUS_PixelFormat_eA8_R8_G8_B8;
    createSettings.width = info.width;
    createSettings.height = info.height;
    createSettings.heap = NEXUS_Platform_GetFramebufferHeap(0);
    surface = NEXUS_Surface_Create(&createSettings);

    BKNI_CreateEvent(&checkpointEvent);
    BKNI_CreateEvent(&spaceAvailableEvent);

    gfx = NEXUS_Graphics2D_Open(0, NULL);

    NEXUS_Graphics2D_GetSettings(gfx, &gfxSettings);
    gfxSettings.checkpointCallback.callback = complete;
    gfxSettings.checkpointCallback.context = checkpointEvent;
    gfxSettings.packetSpaceAvailable.callback = complete;
    gfxSettings.packetSpaceAvailable.context = spaceAvailableEvent;
    NEXUS_Graphics2D_SetSettings(gfx, &gfxSettings);

    /* yellowish-green background */
    NEXUS_Graphics2D_GetDefaultFillSettings(&fillSettings);
    fillSettings.surface = surface;
    fillSettings.color = 0xFF808000;
    NEXUS_Graphics2D_Fill(gfx, &fillSettings);
    
    /* green box */
    fillSettings.rect = box;
    fillSettings.color = 0xFF00FF00;
    NEXUS_Graphics2D_Fill(gfx, &fillSettings);
    
    /* blue box */
    fillSettings.rect.x += 20;
    fillSettings.rect.y += 20;
    fillSettings.rect.width -= 40;
    fillSettings.rect.height -= 40;
    fillSettings.color = 0xFF0000FF;
    NEXUS_Graphics2D_Fill(gfx, &fillSettings);
    
    rc = NEXUS_Graphics2D_Checkpoint(gfx, NULL);
    if (rc == NEXUS_GRAPHICS2D_QUEUED) {
        rc = BKNI_WaitForEvent(checkpointEvent, BKNI_INFINITE);
    }
    BDBG_ASSERT(!rc);

    NEXUS_Display_GetGraphicsSettings(display, &graphicsSettings);
    graphicsSettings.enabled = true;
    NEXUS_Display_SetGraphicsSettings(display, &graphicsSettings);
    NEXUS_Display_SetGraphicsFramebuffer(display, surface);
    
    NEXUS_Graphics2D_GetDefaultBlitSettings(&blitSettings);
    blitSettings.source.surface = surface;
    blitSettings.source.rect = box;
    blitSettings.output.surface = surface;
    blitSettings.output.rect = box;
    blitSettings.output.rect.x += box.width + 20; /* just to the right */
    rc = NEXUS_Graphics2D_Blit(gfx, &blitSettings);
    BDBG_ASSERT(!rc);
    rc = NEXUS_Graphics2D_Checkpoint(gfx, NULL);
    if (rc == NEXUS_GRAPHICS2D_QUEUED) {
        rc = BKNI_WaitForEvent(checkpointEvent, BKNI_INFINITE);
    }
    BDBG_ASSERT(!rc);
    printf("non-colorkey copy blit\n");
    
    /* the replace/replaceMask is only used for scaling. it is the value used instead of the color key value when filtering neighboring pixels.
    the fact that the color key pixel (or its replaced value) are used in a scale filter operation means there is a halo effect using
    color key with scaling. 
    if you set replace = 0xFF000000 and replaceMask = 0xFFFFFFFF, you will get a black halo. */
    
    printf("colors:\n");
    printf("  colorkey = 0xFF0000FF (blue)\n");
    printf("  fill     = 0xFFFF0000 (red)\n");
    printf("  no match = 0xFF00FF00 (green)\n\n");
    
    printf("Press ENTER for next\n");
    getchar();
    
    /* set up standard fill to erase box on the right before each blit */
    NEXUS_Graphics2D_GetDefaultFillSettings(&eraseBoxFillSettings);
    eraseBoxFillSettings.surface = surface;
    eraseBoxFillSettings.color = 0xFF808000;
    eraseBoxFillSettings.rect = blitSettings.output.rect;
    rc = NEXUS_Graphics2D_Fill(gfx, &eraseBoxFillSettings);
    BDBG_ASSERT(!rc);
    
    /*
    strategy:
    for match, take dest pixels.
    for non-match, take source pixels.
    */
    NEXUS_Graphics2D_GetDefaultBlitSettings(&blitSettings);
    blitSettings.source.surface = surface;
    blitSettings.source.rect = box;
    blitSettings.output.surface = surface;
    blitSettings.output.rect = box;
    blitSettings.output.rect.x += box.width + 20; /* just to the right */
    blitSettings.dest.surface = surface;
    blitSettings.dest.rect = blitSettings.output.rect;
    blitSettings.colorKey.source.enabled = true;
    blitSettings.colorKey.source.lower = 0xFF0000FF;
    blitSettings.colorKey.source.upper = 0xFF0000FF;
    blitSettings.colorKey.source.mask = 0xFFFFFFFF;
    blitSettings.colorOp = NEXUS_BlitColorOp_eCopySource;
    blitSettings.alphaOp = NEXUS_BlitAlphaOp_eCopySource;
    rc = NEXUS_Graphics2D_Blit(gfx, &blitSettings);
    BDBG_ASSERT(!rc);
    rc = NEXUS_Graphics2D_Checkpoint(gfx, NULL);
    if (rc == NEXUS_GRAPHICS2D_QUEUED) {
        rc = BKNI_WaitForEvent(checkpointEvent, BKNI_INFINITE);
    }
    BDBG_ASSERT(!rc);
    printf("Match Action=NO-COPY,  No-Match Action=COPY.\n");

    printf("Press ENTER for next\n");
    getchar();
    
    rc = NEXUS_Graphics2D_Fill(gfx, &eraseBoxFillSettings);
    BDBG_ASSERT(!rc);
    
    /*
    strategy:
    for match, take source constant color.
    for non-match, take source pixels.
    */
    NEXUS_Graphics2D_GetDefaultBlitSettings(&blitSettings);
    blitSettings.source.surface = surface;
    blitSettings.source.rect = box;
    blitSettings.output.surface = surface;
    blitSettings.output.rect = box;
    blitSettings.output.rect.x += box.width + 20; /* just to the right */
    blitSettings.dest.surface = NULL;
    blitSettings.colorKey.source.enabled = true;
    blitSettings.colorKey.source.lower = 0xFF0000FF;
    blitSettings.colorKey.source.upper = 0xFF0000FF;
    blitSettings.colorKey.source.mask = 0xFFFFFFFF;
    blitSettings.constantColor = 0xFFFF0000; /* fill red */
    rc = NEXUS_Graphics2D_Blit(gfx, &blitSettings);
    BDBG_ASSERT(!rc);
    rc = NEXUS_Graphics2D_Checkpoint(gfx, NULL);
    if (rc == NEXUS_GRAPHICS2D_QUEUED) {
        rc = BKNI_WaitForEvent(checkpointEvent, BKNI_INFINITE);
    }
    BDBG_ASSERT(!rc);
    printf("Match Action=FILL,         No-Match Action=COPY.\n");
    
    printf("Press ENTER for next\n");
    getchar();
    
    rc = NEXUS_Graphics2D_Fill(gfx, &eraseBoxFillSettings);
    BDBG_ASSERT(!rc);
    
    /**
    strategy:
    for match, take source constant color (red).
    for non-match, take dest pixels.
    */
    NEXUS_Graphics2D_GetDefaultBlitSettings(&blitSettings);
    blitSettings.source.surface = surface;
    blitSettings.source.rect = box;
    blitSettings.output.surface = surface;
    blitSettings.output.rect = box;
    blitSettings.output.rect.x += box.width + 20; /* just to the right */
    blitSettings.dest.surface = surface;
    blitSettings.dest.rect = blitSettings.output.rect;
    {
        NEXUS_BlendEquation eq = {NEXUS_BlendFactor_eConstantColor, NEXUS_BlendFactor_eOne, false, NEXUS_BlendFactor_eZero, NEXUS_BlendFactor_eZero, false, NEXUS_BlendFactor_eZero};
        blitSettings.colorBlend = eq;
    }
    blitSettings.colorOp = NEXUS_BlitColorOp_eUseBlendEquation;
    blitSettings.alphaOp = NEXUS_BlitAlphaOp_eCopySource;
    blitSettings.constantColor = 0xFFFF0000; /* fill red */
    blitSettings.colorKey.source.enabled = true;
    blitSettings.colorKey.source.lower = 0xFF0000FF;
    blitSettings.colorKey.source.upper = 0xFF0000FF;
    blitSettings.colorKey.source.mask = 0xFFFFFFFF;
    blitSettings.colorKey.dest = blitSettings.colorKey.source;
    rc = NEXUS_Graphics2D_Blit(gfx, &blitSettings);
    BDBG_ASSERT(!rc);
    rc = NEXUS_Graphics2D_Checkpoint(gfx, NULL);
    if (rc == NEXUS_GRAPHICS2D_QUEUED) {
        rc = BKNI_WaitForEvent(checkpointEvent, BKNI_INFINITE);
    }
    BDBG_ASSERT(!rc);
    printf("Match Action=FILL,         No-Match Action=NO-COPY.\n");
    
    printf("Press ENTER for next\n");
    getchar();
    
    rc = NEXUS_Graphics2D_Fill(gfx, &eraseBoxFillSettings);
    BDBG_ASSERT(!rc);
    
    /**
    strategy:
    for match, take source pixels.
    for non-match, take dest pixels.
    */
    NEXUS_Graphics2D_GetDefaultBlitSettings(&blitSettings);
    blitSettings.source.surface = surface;
    blitSettings.source.rect = box;
    blitSettings.output.surface = surface;
    blitSettings.output.rect = box;
    blitSettings.output.rect.x += box.width + 20; /* just to the right */
    blitSettings.dest.surface = surface; /* if color key matches, this will copy */
    blitSettings.dest.rect = blitSettings.output.rect;
    blitSettings.colorOp = NEXUS_BlitColorOp_eCopySource;
    blitSettings.alphaOp = NEXUS_BlitAlphaOp_eCopySource;
    blitSettings.colorKey.source.enabled = true;
    blitSettings.colorKey.source.lower = 0xFF0000FF;
    blitSettings.colorKey.source.upper = 0xFF0000FF;
    blitSettings.colorKey.source.mask = 0xFFFFFFFF;
    blitSettings.colorKey.dest = blitSettings.colorKey.source;
    rc = NEXUS_Graphics2D_Blit(gfx, &blitSettings);
    BDBG_ASSERT(!rc);
    rc = NEXUS_Graphics2D_Checkpoint(gfx, NULL);
    if (rc == NEXUS_GRAPHICS2D_QUEUED) {
        rc = BKNI_WaitForEvent(checkpointEvent, BKNI_INFINITE);
    }
    BDBG_ASSERT(!rc);
    printf("Match Action=COPY,         No-Match Action=NO-COPY.\n");
    
    printf("Press ENTER for next\n");
    getchar();
    
    rc = NEXUS_Graphics2D_Fill(gfx, &eraseBoxFillSettings);
    BDBG_ASSERT(!rc);
    
    /**
    strategy:
    for match, take dest pixels.
    for non-match, take source constant color (red).
    */
    NEXUS_Graphics2D_GetDefaultBlitSettings(&blitSettings);
    blitSettings.source.surface = surface;
    blitSettings.source.rect = box;
    blitSettings.output.surface = surface;
    blitSettings.output.rect = box;
    blitSettings.output.rect.x += box.width + 20; /* just to the right */
    blitSettings.dest.surface = surface;
    blitSettings.dest.rect = blitSettings.output.rect;
    {
        NEXUS_BlendEquation eq = {NEXUS_BlendFactor_eConstantColor, NEXUS_BlendFactor_eOne, false, NEXUS_BlendFactor_eZero, NEXUS_BlendFactor_eZero, false, NEXUS_BlendFactor_eZero};
        blitSettings.colorBlend = eq;
    }
    blitSettings.colorOp = NEXUS_BlitColorOp_eUseBlendEquation;
    blitSettings.alphaOp = NEXUS_BlitAlphaOp_eCopySource;
    blitSettings.constantColor = 0xFFFF0000; /* fill red */
    blitSettings.colorKey.source.enabled = true;
    blitSettings.colorKey.source.lower = 0xFF0000FF;
    blitSettings.colorKey.source.upper = 0xFF0000FF;
    blitSettings.colorKey.source.mask = 0xFFFFFFFF;
    blitSettings.colorKey.dest.enabled = false;
    rc = NEXUS_Graphics2D_Blit(gfx, &blitSettings);
    BDBG_ASSERT(!rc);
    rc = NEXUS_Graphics2D_Checkpoint(gfx, NULL);
    if (rc == NEXUS_GRAPHICS2D_QUEUED) {
        rc = BKNI_WaitForEvent(checkpointEvent, BKNI_INFINITE);
    }
    BDBG_ASSERT(!rc);
    printf("Match Action=NO-COPY,  No-Match Action=FILL.\n");
    
    printf("Press ENTER to exit\n");
    getchar();

    NEXUS_Display_Close(display);
    NEXUS_Graphics2D_Close(gfx);
    NEXUS_Surface_Destroy(surface);
    BKNI_DestroyEvent(checkpointEvent);
    BKNI_DestroyEvent(spaceAvailableEvent);
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
