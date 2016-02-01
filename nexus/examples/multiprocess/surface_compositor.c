/******************************************************************************
 *    (c)20153 Broadcom Corporation
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

#if NEXUS_HAS_SURFACE_COMPOSITOR
#include "nexus_platform.h"
#include "nexus_surface.h"
#include "nexus_graphics2d.h"
#include "nexus_display.h"
#include "nexus_base_mmap.h"
#include "nexus_component_output.h"
#include "nexus_composite_output.h"
#if NEXUS_HAS_HDMI_OUTPUT
#include "nexus_hdmi_output.h"
#endif
#include "nexus_core_utils.h"
#include "bstd.h"
#include "bkni.h"
#include "nexus_surface_compositor.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* single-process example of surface compositor */

BDBG_MODULE(surface_compositor);

static void complete(void *data, int unused)
{
    BSTD_UNUSED(unused);
    BKNI_SetEvent((BKNI_EventHandle)data);
}

static void checkpoint(NEXUS_Graphics2DHandle gfx, BKNI_EventHandle event)
{
    int rc;
    rc = NEXUS_Graphics2D_Checkpoint(gfx, NULL);
    if (rc == NEXUS_GRAPHICS2D_QUEUED) {
        BKNI_WaitForEvent(event, 0xffffffff);
    }
}

int main(void)
{
    NEXUS_SurfaceCompositorHandle surface_compositor;
#define NUM_CLIENTS 4
    NEXUS_SurfaceClientHandle surfaceClient[NUM_CLIENTS];
    NEXUS_SurfaceHandle surface[NUM_CLIENTS];
    NEXUS_DisplayHandle display;
    NEXUS_Graphics2DHandle gfx;
    NEXUS_Graphics2DSettings gfxSettings;
    BKNI_EventHandle event;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_PlatformSettings platformSettings;
    NEXUS_SurfaceCompositorSettings surface_compositor_settings;
    NEXUS_SurfaceCompositorClientSettings client_settings;
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_HdmiOutputStatus hdmiStatus;
#endif
    NEXUS_Error rc;
    unsigned i;

    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    rc = NEXUS_Platform_Init(&platformSettings);
    if (rc) return -1;
    NEXUS_Platform_GetConfiguration(&platformConfig);

    BKNI_CreateEvent(&event);

    display = NEXUS_Display_Open(0, NULL);
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
        NEXUS_DisplaySettings displaySettings;
        NEXUS_Display_GetSettings(display, &displaySettings);
        if ( !hdmiStatus.videoFormatSupported[displaySettings.format] ) {
            displaySettings.format = hdmiStatus.preferredVideoFormat;
            NEXUS_Display_SetSettings(display, &displaySettings);
        }
    }
#endif

    gfx = NEXUS_Graphics2D_Open(NEXUS_ANY_ID, NULL);
    NEXUS_Graphics2D_GetSettings(gfx, &gfxSettings);
    gfxSettings.checkpointCallback.callback = complete;
    gfxSettings.checkpointCallback.context = event;
    NEXUS_Graphics2D_SetSettings(gfx, &gfxSettings);

    /* create surface compositor server */
    surface_compositor = NEXUS_SurfaceCompositor_Create(0);
    NEXUS_SurfaceCompositor_GetSettings(surface_compositor, &surface_compositor_settings);
    NEXUS_Display_GetGraphicsSettings(display, &surface_compositor_settings.display[0].graphicsSettings);
    surface_compositor_settings.display[0].display = display;
    surface_compositor_settings.display[0].framebuffer.number = 2;
    surface_compositor_settings.display[0].framebuffer.width = 1280;
    surface_compositor_settings.display[0].framebuffer.height = 720;
    surface_compositor_settings.display[0].framebuffer.backgroundColor = 0x00000000; /* transparent background */
    surface_compositor_settings.display[0].framebuffer.heap = NEXUS_Platform_GetFramebufferHeap(0);
    rc = NEXUS_SurfaceCompositor_SetSettings(surface_compositor, &surface_compositor_settings);
    BDBG_ASSERT(!rc);

    for (i=0;i<NUM_CLIENTS;i++) {
        NEXUS_SurfaceCreateSettings surfaceCreateSettings;
        NEXUS_Graphics2DFillSettings fillSettings;

        surfaceClient[i] = NEXUS_SurfaceCompositor_CreateClient(surface_compositor, i);
        NEXUS_SurfaceCompositor_GetClientSettings(surface_compositor, surfaceClient[i], &client_settings);
        client_settings.composition.position.x = (i+1)*100;
        client_settings.composition.position.y = (i+1)*100;
        client_settings.composition.position.width = 200;
        client_settings.composition.position.height = 200;
        client_settings.composition.zorder = i;
        rc = NEXUS_SurfaceCompositor_SetClientSettings(surface_compositor, surfaceClient[i], &client_settings);
        BDBG_ASSERT(!rc);

        NEXUS_Surface_GetDefaultCreateSettings(&surfaceCreateSettings);
        surfaceCreateSettings.width = 200;
        surfaceCreateSettings.height = 200;
        surface[i] = NEXUS_Surface_Create(&surfaceCreateSettings);

        NEXUS_Graphics2D_GetDefaultFillSettings(&fillSettings);
        fillSettings.surface = surface[i];
        switch (i) {
        case 0: fillSettings.color = 0xFFFF0000; break; /* red on bottom */
        case 1: fillSettings.color = 0xFF00FF00; break; /* green */
        case 2: fillSettings.color = 0xFF0000FF; break; /* blue */
        case 3: fillSettings.color = 0xFFFFFFFF; break; /* white on top */
        }
        NEXUS_Graphics2D_Fill(gfx, &fillSettings);
        checkpoint(gfx, event);

        /* NEXUS_SurfaceClient_Acquire not required for single process */
        NEXUS_SurfaceClient_SetSurface(surfaceClient[i], surface[i]);
        /* waiting for recycled or displayed event not required if we're not making updates */
    }

    printf("Press ENTER to shutdown\n");
    getchar();

    for (i=0;i<NUM_CLIENTS;i++) {
        NEXUS_SurfaceCompositor_DestroyClient(surfaceClient[i]);
        NEXUS_Surface_Destroy(surface[i]);
    }
    NEXUS_SurfaceCompositor_Destroy(surface_compositor);
    NEXUS_Graphics2D_Close(gfx);
    BKNI_DestroyEvent(event);
    NEXUS_Display_Close(display);
    NEXUS_Platform_Uninit();

    return 0;
}

#else
#include <stdio.h>
int main(void)
{
    printf("ERROR: you must include surface_compositor.inc in platform_modules.inc\n");
    return -1;
}
#endif
