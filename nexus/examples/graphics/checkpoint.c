/******************************************************************************
 * Copyright (C) 2018 Broadcom.
 * The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to
 * the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied),
 * right to use, or waiver of any kind with respect to the Software, and
 * Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 * THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 * IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use all
 * reasonable efforts to protect the confidentiality thereof, and to use this
 * information only in connection with your use of Broadcom integrated circuit
 * products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 * OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 * RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 * IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 * A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 * ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 * THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 * OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 * INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 * RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 * HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 * EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 * WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 * FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *****************************************************************************/
/* Nexus example app: show different ways of using NEXUS_Graphics2D_Checkpoint */

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
#include "bkni_multi.h"
#include <stdlib.h>
#include <stdio.h>

BDBG_MODULE(checkpoint);

static void complete(void *data, int unused)
{
    BSTD_UNUSED(unused);
    BKNI_SetEvent((BKNI_EventHandle)data);
}

static void
async_checkpoint(NEXUS_Graphics2DHandle gfx, BKNI_EventHandle event)
{
    NEXUS_Error rc;
    rc=NEXUS_Graphics2D_Checkpoint(gfx, NULL);
    if(rc==NEXUS_SUCCESS) {
        return;
    }
    if(rc==NEXUS_GRAPHICS2D_QUEUED) {
        BKNI_WaitForEvent(event, BKNI_INFINITE);
        return;
    }
    BDBG_ASSERT(rc!=NEXUS_GRAPHICS2D_BUSY);
    return;
}

static void
poll_checkpoint(NEXUS_Graphics2DHandle gfx, BKNI_EventHandle event)
{
    NEXUS_Error rc;
    BSTD_UNUSED(event);

    for(;;) {
        rc=NEXUS_Graphics2D_Checkpoint(gfx, NULL);
        if(rc==NEXUS_SUCCESS) {
            break;
        }
        if(rc==NEXUS_GRAPHICS2D_BUSY) {
            /* fprintf(stderr,"."); */
            continue;
        }
        BDBG_ASSERT(rc!=NEXUS_GRAPHICS2D_QUEUED);
        BDBG_ASSERT(0);
    }
    /* fprintf(stderr,"\n"); */
}

static void
no_checkpoint(NEXUS_Graphics2DHandle gfx, BKNI_EventHandle event)
{
    BSTD_UNUSED(gfx);
    BSTD_UNUSED(event);
    return;
}


static void
run_fill(NEXUS_DisplayHandle display, NEXUS_Graphics2DHandle gfx, NEXUS_SurfaceHandle surface,BKNI_EventHandle event, const NEXUS_SurfaceCreateSettings *createSettings, void (*checkpoint)(NEXUS_Graphics2DHandle gfx, BKNI_EventHandle event), unsigned max_batch)
{
    NEXUS_Graphics2DFillSettings fillSettings;
    NEXUS_Error rc;
    unsigned size;
    unsigned max_size = (createSettings->height*3)/4;

    NEXUS_Graphics2D_GetDefaultFillSettings(&fillSettings);
    fillSettings.surface = surface;
    fillSettings.rect.y = 0;
    fillSettings.rect.x = 0;

    for(size=2;size<max_size;size=(size*3)/2) {
        unsigned batch;
        const unsigned pixels = 256 * (8*size*size);
        unsigned width = size;
        unsigned height = size;
        unsigned count = pixels / (width*height);

        for(batch=1;batch<max_batch;batch*=2) {
            unsigned j;
            fillSettings.rect.width = width;
            fillSettings.rect.height = height;
            for (j=0;j<count;) {
                fillSettings.color = 0xFF000000 | ((j*j*2287) & 0xFFFFFF);
                if(fillSettings.rect.x+width > createSettings->width) {
                    fillSettings.rect.x = (fillSettings.rect.x + width) - createSettings->width;
                }
                if(fillSettings.rect.y+height > createSettings->height) {
                    fillSettings.rect.y = (fillSettings.rect.y+height) - createSettings->height;
                }
                for(;;) {
                    rc=NEXUS_Graphics2D_Fill(gfx, &fillSettings);
                    if(rc==NEXUS_GRAPHICS2D_QUEUE_FULL) {
                        checkpoint(gfx,event);
                        BDBG_WRN(("repeating at %u:%u", batch, j));
                    } else {
                        BDBG_ASSERT(rc==NEXUS_SUCCESS);
                        break;
                    }
                }
                fillSettings.rect.y+=11;
                fillSettings.rect.x+=17;
                j++;
                if(j%batch==0) {
                    checkpoint(gfx, event);
                }
            }
            checkpoint(gfx, event);
            BDBG_WRN(("filled %u pixels using batch %u using %u %ux%u fills", pixels, batch, j, width, height));

            NEXUS_Display_SetGraphicsFramebuffer(display, surface);
        }
    }
    return;
}

static void
run_copy(NEXUS_DisplayHandle display, NEXUS_Graphics2DHandle gfx, NEXUS_SurfaceHandle surface,BKNI_EventHandle event, const NEXUS_SurfaceCreateSettings *createSettings, void (*checkpoint)(NEXUS_Graphics2DHandle gfx, BKNI_EventHandle event), unsigned max_batch)
{
    NEXUS_Graphics2DBlitSettings blitSettings;
    NEXUS_Error rc;
    unsigned size;
    unsigned max_size = (createSettings->height*3)/4;

    NEXUS_Graphics2D_GetDefaultBlitSettings(&blitSettings);
    blitSettings.source.surface = surface;
    blitSettings.source.rect.x = 0;
    blitSettings.source.rect.y = 0;
    blitSettings.output.surface = surface;
    blitSettings.output.rect.x = createSettings->width/2;
    blitSettings.output.rect.y = createSettings->height/2;
    blitSettings.colorOp = NEXUS_BlitColorOp_eCopySource;
    blitSettings.alphaOp = NEXUS_BlitAlphaOp_eCopySource;

    for(size=2;size<max_size;size=(size*3)/2) {
        unsigned batch;
        const unsigned pixels = 256 * (8*size*size);
        unsigned width = size;
        unsigned height = size;
        unsigned count = pixels / (width*height);

        for(batch=1;batch<max_batch;batch*=2) {
            unsigned j;
            blitSettings.source.rect.width = width;
            blitSettings.source.rect.height = height;
            blitSettings.output.rect.width = width;
            blitSettings.output.rect.height = height;
            for (j=0;j<count;) {
                if(blitSettings.source.rect.x+width > createSettings->width) {
                    blitSettings.source.rect.x = (blitSettings.source.rect.x + width) - createSettings->width;
                }
                if(blitSettings.source.rect.y+height > createSettings->height) {
                    blitSettings.source.rect.y = (blitSettings.source.rect.y + height) - createSettings->height;
                }
                if(blitSettings.output.rect.x+width > createSettings->width) {
                    blitSettings.output.rect.x = (blitSettings.output.rect.x + width) - createSettings->width;
                }
                if(blitSettings.output.rect.y+height > createSettings->height) {
                    blitSettings.output.rect.y = (blitSettings.output.rect.y + height) - createSettings->height;
                }
                for(;;) {
                    rc = NEXUS_Graphics2D_Blit(gfx, &blitSettings);
                    if(rc==NEXUS_GRAPHICS2D_QUEUE_FULL) {
                        checkpoint(gfx,event);
                        BDBG_WRN(("repeating at %u:%u", batch, j));
                    } else {
                        BDBG_ASSERT(rc==NEXUS_SUCCESS);
                        break;
                    }
                }
                blitSettings.source.rect.y+=4;
                blitSettings.source.rect.x+=6;
                blitSettings.output.rect.y+=10;
                blitSettings.output.rect.x+=18;
                j++;
                if(j%batch==0) {
                    checkpoint(gfx, event);
                }
            }
            checkpoint(gfx, event);
            BDBG_WRN(("copied %u pixels using batch %u using %u %ux%u blits", pixels, batch, j, width, height));

            NEXUS_Display_SetGraphicsFramebuffer(display, surface);
        }
    }

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
    BKNI_EventHandle event;
    NEXUS_PlatformSettings platformSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_Graphics2DOpenSettings openSettings;
    NEXUS_Graphics2DSettings graphics2DSettings;
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_HdmiOutputStatus hdmiStatus;
    NEXUS_Error rc;
#endif
    unsigned num_blits = 2048;

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

    NEXUS_Surface_GetDefaultCreateSettings(&createSettings);
    createSettings.pixelFormat = NEXUS_PixelFormat_eA8_R8_G8_B8;
    createSettings.width = 720;
    createSettings.height = 480;
    createSettings.heap = NEXUS_Platform_GetFramebufferHeap(0);
    surface = NEXUS_Surface_Create(&createSettings);

    BKNI_CreateEvent(&event);

    NEXUS_Graphics2D_GetDefaultOpenSettings(&openSettings);
    gfx = NEXUS_Graphics2D_Open(0, &openSettings);

    NEXUS_Graphics2D_GetSettings(gfx, &graphics2DSettings);
    graphics2DSettings.checkpointCallback.callback = complete;
    graphics2DSettings.checkpointCallback.context = event;
    NEXUS_Graphics2D_SetSettings(gfx, &graphics2DSettings);

    /* draw gradient on left side of black framebuffer using synchronous blits */
    NEXUS_Graphics2D_GetDefaultFillSettings(&fillSettings);
    fillSettings.surface = surface;
    fillSettings.rect.width = createSettings.width;
    fillSettings.rect.height = createSettings.height;
    fillSettings.color = 0;
    NEXUS_Graphics2D_Fill(gfx, &fillSettings);
    async_checkpoint(gfx, event);

    NEXUS_Display_GetGraphicsSettings(display, &graphicsSettings);
    graphicsSettings.enabled = true;
    graphicsSettings.clip.width = createSettings.width;
    graphicsSettings.clip.height = createSettings.height;
    NEXUS_Display_SetGraphicsSettings(display, &graphicsSettings);
    NEXUS_Display_SetGraphicsFramebuffer(display, surface);

    NEXUS_Graphics2D_GetSettings(gfx, &graphics2DSettings);
    graphics2DSettings.blockedSync = false;
    graphics2DSettings.completionTimeout = 0;
    graphics2DSettings.pollingCheckpoint = false;
    NEXUS_Graphics2D_SetSettings(gfx, &graphics2DSettings);
    fprintf(stderr, "Using async mode with checkpoint\n");
    run_fill(display, gfx, surface, event, &createSettings, async_checkpoint, num_blits);
    run_copy(display, gfx, surface, event, &createSettings, async_checkpoint, num_blits);

    NEXUS_Graphics2D_GetSettings(gfx, &graphics2DSettings);
    graphics2DSettings.blockedSync = false;
    graphics2DSettings.completionTimeout = 10;
    graphics2DSettings.pollingCheckpoint = false;
    NEXUS_Graphics2D_SetSettings(gfx, &graphics2DSettings);
    fprintf(stderr, "Using async mode with checkpoint doing 10us busy wait\n");
    run_fill(display, gfx, surface, event, &createSettings, async_checkpoint, num_blits);
    run_copy(display, gfx, surface, event, &createSettings, async_checkpoint, num_blits);

    NEXUS_Graphics2D_GetSettings(gfx, &graphics2DSettings);
    graphics2DSettings.completionTimeout = 0;
    graphics2DSettings.blockedSync = false;
    graphics2DSettings.pollingCheckpoint = true;
    NEXUS_Graphics2D_SetSettings(gfx, &graphics2DSettings);
    fprintf(stderr,"Using async mode with polling\n");
    run_fill(display, gfx, surface, event, &createSettings, poll_checkpoint, num_blits);
    run_copy(display, gfx, surface, event, &createSettings, poll_checkpoint, num_blits);

    NEXUS_Graphics2D_GetSettings(gfx, &graphics2DSettings);
    graphics2DSettings.completionTimeout = 0;
    graphics2DSettings.blockedSync = true;
    graphics2DSettings.pollingCheckpoint = false;
    NEXUS_Graphics2D_SetSettings(gfx, &graphics2DSettings);
    fprintf(stderr, "Using synchronous mode\n");
    run_fill(display, gfx, surface, event, &createSettings, no_checkpoint, 3);
    run_copy(display, gfx, surface, event, &createSettings, no_checkpoint, 3);

    fprintf(stderr, "Done. Press Enter to exit\n");
    getchar();

    NEXUS_Display_GetGraphicsSettings(display, &graphicsSettings);
    graphicsSettings.enabled = false;
    graphicsSettings.clip.width = createSettings.width;
    graphicsSettings.clip.height = createSettings.height;
    NEXUS_Display_SetGraphicsSettings(display, &graphicsSettings);
    NEXUS_Graphics2D_Close(gfx);
    BKNI_DestroyEvent(event);
    NEXUS_Display_Close(display);
    NEXUS_Surface_Destroy(surface);
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
