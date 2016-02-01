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
/* Nexus example app: blit between palettized surfaces */

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
#include "bstd.h"
#include "bkni.h"
#include <stdlib.h>
#include <stdio.h>

static void complete(void *context, int param)
{
    BSTD_UNUSED(param);
    BKNI_SetEvent((BKNI_EventHandle)context);
}

int main(void)
{
    NEXUS_SurfaceHandle framebuffer, surface;
    NEXUS_SurfaceCreateSettings createSettings;
    NEXUS_SurfaceMemory mem;
    NEXUS_DisplayHandle display;
    NEXUS_DisplaySettings displaySettings;
    NEXUS_GraphicsSettings graphicsSettings;
    NEXUS_Graphics2DHandle gfx;
    NEXUS_Graphics2DSettings gfxSettings;
    BKNI_EventHandle checkpointEvent;
    NEXUS_PlatformSettings platformSettings;
    NEXUS_PlatformConfiguration platformConfig;
    unsigned i, j;

    /* Bring up all modules for a platform in a default configuraiton for this platform */
    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init(&platformSettings);
    NEXUS_Platform_GetConfiguration(&platformConfig);

    NEXUS_Display_GetDefaultSettings(&displaySettings);
    displaySettings.displayType = NEXUS_DisplayType_eAuto;
    displaySettings.format = NEXUS_VideoFormat_eNtsc;
    display = NEXUS_Display_Open(0, &displaySettings); /* take default video format */

#if NEXUS_NUM_COMPONENT_OUTPUTS
    NEXUS_Display_AddOutput(display, NEXUS_ComponentOutput_GetConnector(platformConfig.outputs.component[0]));
#endif
#if NEXUS_NUM_COMPOSITE_OUTPUTS
    NEXUS_Display_AddOutput(display, NEXUS_CompositeOutput_GetConnector(platformConfig.outputs.composite[0]));
#endif
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_Display_AddOutput(display, NEXUS_HdmiOutput_GetVideoConnector(platformConfig.outputs.hdmi[0]));
#endif

    NEXUS_Surface_GetDefaultCreateSettings(&createSettings);
    createSettings.pixelFormat = NEXUS_PixelFormat_ePalette8;
    createSettings.width = 720;
    createSettings.height = 480;
    /* create two surfaces - one framebuffer, one off-screen surface */
    createSettings.heap = NEXUS_Platform_GetFramebufferHeap(0);
    framebuffer = NEXUS_Surface_Create(&createSettings);
    createSettings.heap = NULL;
    surface = NEXUS_Surface_Create(&createSettings);

    /* set the palette of the framebuffer with even lines = gradient of blue, odd lines = gradient of red */
    NEXUS_Surface_GetMemory(framebuffer, &mem);
    for (i=0;i<mem.numPaletteEntries;i+=2) {
        mem.palette[i] = 0xff000000 | i; /* blue */
    }
    for (i=1;i<mem.numPaletteEntries;i+=2) {
        mem.palette[i] = 0xffff0000; /* red */
    }
    /* fill surface with index 0 */
    BKNI_Memset(mem.buffer, 0, createSettings.height * mem.pitch);

    /* set the palette of the off-screen surface with a gradient of gray */
    NEXUS_Surface_GetMemory(surface, &mem);
    for (i=0;i<mem.numPaletteEntries;i++) {
        mem.palette[i] = 0xff000000 | i<<16 | i<<8 | i;
    }
    /* fill surface with index 0 */
    BKNI_Memset(mem.buffer, 0, createSettings.height * mem.pitch);

    gfx = NEXUS_Graphics2D_Open(0, NULL);
    BKNI_CreateEvent(&checkpointEvent);
    NEXUS_Graphics2D_GetSettings(gfx, &gfxSettings);
    gfxSettings.checkpointCallback.callback = complete;
    gfxSettings.checkpointCallback.context = checkpointEvent;
    NEXUS_Graphics2D_SetSettings(gfx, &gfxSettings);

    /* draw a left-to-right gradient on the framebuffer */
    NEXUS_Surface_GetMemory(framebuffer, &mem);
    for (i=0;i<createSettings.width;i++) {
        for (j=0;j<createSettings.height;j++) {
            /* palette8 is 8bpp. take only even palette indices. */
            *((uint8_t*)mem.buffer + j*mem.pitch + i) = (i % 128) * 2;
        }
    }
    NEXUS_Surface_Flush(framebuffer);

    /* drop a top-to-bottom gradient on the off-screen surface */
    NEXUS_Surface_GetMemory(surface, &mem);
    for (j=0;j<createSettings.height;j++) {
        for (i=0;i<createSettings.width;i++) {
            /* palette8 is 8bpp. take only even palette indices. */
            *((uint8_t*)mem.buffer + j*mem.pitch + i) = (j % 128) * 2;
        }
    }
    NEXUS_Surface_Flush(surface);

    NEXUS_Display_GetGraphicsSettings(display, &graphicsSettings);
    graphicsSettings.enabled = true;
    NEXUS_Display_SetGraphicsSettings(display, &graphicsSettings);
    NEXUS_Display_SetGraphicsFramebuffer(display, framebuffer);

    /* blit from the off-screen to the framebuffer */
    {
        NEXUS_Error rc;
        NEXUS_Graphics2DBlitSettings blitSettings;

        NEXUS_Graphics2D_GetDefaultBlitSettings(&blitSettings);

        blitSettings.source.surface = surface;
        blitSettings.source.rect.x = 100;
        blitSettings.source.rect.y = 0;
        blitSettings.source.rect.width = 100;
        blitSettings.source.rect.height = 256; /* take a whole gradient */

        blitSettings.output.surface = framebuffer;
        blitSettings.output.rect.x = 310; /* center it */
        blitSettings.output.rect.y = 140;
        blitSettings.output.rect.width = 100;
        blitSettings.output.rect.height = 200; /* down scale from 256 to 200 */

        blitSettings.colorOp = NEXUS_BlitColorOp_eCopySource;
        blitSettings.alphaOp = NEXUS_BlitAlphaOp_eCopySource;

        /* because our palette is not a true-color palette, we should not have any blending of index.
        with ePointSample, you'll see only blue. if you don't use ePointSample, you'll see a blend of blue and red. */
        blitSettings.horizontalFilter = NEXUS_Graphics2DFilterCoeffs_ePointSample;
        blitSettings.verticalFilter = NEXUS_Graphics2DFilterCoeffs_ePointSample;

        rc = NEXUS_Graphics2D_Blit(gfx, &blitSettings);
        BDBG_ASSERT(!rc);
        
        rc = NEXUS_Graphics2D_Checkpoint(gfx, NULL);
        if (rc == NEXUS_GRAPHICS2D_QUEUED) {
            rc = BKNI_WaitForEvent(checkpointEvent, BKNI_INFINITE);
        }
        BDBG_ASSERT(!rc);
    }

    /* You should see a small box with vertical gradient blitted in the middle of a horizontal gradient.
    Both gradients should be black->blue. The palette index is copied, not the palette colors. */

    printf("Press ENTER to exit\n");
    getchar();
    
    NEXUS_Surface_Destroy(surface);
    NEXUS_Display_Close(display);
    NEXUS_Surface_Destroy(framebuffer);
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
