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
 *
 * Module Description:
 *
 *****************************************************************************/

#include "nexus_platform.h"
#include <stdio.h>

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
#include <stdio.h>

/* SMPTE color bars */

#define NUM_COLORS 7
    uint32_t g_colors[NUM_COLORS] = {
        0xFFFFFFFF, /* white */
        0xFFFFFF00, /* yellow */
        0xFF00FFFF, /* cyan */
        0xFF00FF00, /* green */
        0xFFFF00FF, /* magenta */
        0xFFFF0000, /* red */
        0xFF0000FF  /* blue */
    };

/* see nexus_video_types.h for struct typedef
TODO: add a more meaningful matrix. */
static NEXUS_ColorMatrix g_colorMatrix0 = {
    2,
    {4,0,0,0,0,
     0,1,0,0,0,
     0,0,1,0,0},
     false
};

static void complete(void *context, int param)
{
    BSTD_UNUSED(param);
    BKNI_SetEvent((BKNI_EventHandle)context);
}

int main(void)
{
    NEXUS_SurfaceHandle framebuffer;
    NEXUS_SurfaceCreateSettings createSettings;
    NEXUS_DisplayHandle display;
    NEXUS_DisplaySettings displaySettings;
    NEXUS_GraphicsSettings graphicsSettings;
    NEXUS_Graphics2DHandle gfx;
    NEXUS_Graphics2DSettings gfxSettings;
    BKNI_EventHandle checkpointEvent;
    NEXUS_Graphics2DFillSettings fillSettings;
    NEXUS_PlatformSettings platformSettings;
    NEXUS_PlatformConfiguration platformConfig;
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_HdmiOutputStatus hdmiStatus;
#endif
    NEXUS_Error rc;
    int i;
    bool done = false;

    /* Bring up all modules for a platform in a default configuraiton for this platform */
    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init(&platformSettings);
    NEXUS_Platform_GetConfiguration(&platformConfig);

    /* HD display */
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

    NEXUS_Surface_GetDefaultCreateSettings(&createSettings);
    createSettings.pixelFormat = NEXUS_PixelFormat_eA8_R8_G8_B8;
    createSettings.width = 720;
    createSettings.height = 480;
    createSettings.heap = NEXUS_Platform_GetFramebufferHeap(0);
    framebuffer = NEXUS_Surface_Create(&createSettings);

    gfx = NEXUS_Graphics2D_Open(0, NULL);
    BKNI_CreateEvent(&checkpointEvent);
    NEXUS_Graphics2D_GetSettings(gfx, &gfxSettings);
    gfxSettings.checkpointCallback.callback = complete;
    gfxSettings.checkpointCallback.context = checkpointEvent;
    NEXUS_Graphics2D_SetSettings(gfx, &gfxSettings);

    /* draw for the HD display */
    for (i=0;i<NUM_COLORS;i++) {
        NEXUS_Graphics2D_GetDefaultFillSettings(&fillSettings);
        fillSettings.surface = framebuffer;
        fillSettings.rect.width = createSettings.width/NUM_COLORS;
        fillSettings.rect.x = fillSettings.rect.width * i;
        fillSettings.rect.y = 0;
        fillSettings.rect.height = createSettings.height;
        fillSettings.color = g_colors[i];
        NEXUS_Graphics2D_Fill(gfx, &fillSettings);
    }

    /* needed to execute queued blits */
    rc = NEXUS_Graphics2D_Checkpoint(gfx, NULL);
    if (rc == NEXUS_GRAPHICS2D_QUEUED) {
        rc = BKNI_WaitForEvent(checkpointEvent, BKNI_INFINITE);
    }
    BDBG_ASSERT(!rc);

    NEXUS_Display_GetGraphicsSettings(display, &graphicsSettings);
    graphicsSettings.enabled = true;
    graphicsSettings.clip.width = createSettings.width;
    graphicsSettings.clip.height = createSettings.height;
    NEXUS_Display_SetGraphicsSettings(display, &graphicsSettings);
    NEXUS_Display_SetGraphicsFramebuffer(display, framebuffer);

    while (!done) {
        char buf[64];
        NEXUS_GraphicsColorSettings graphicsColorSettings;

        printf(
            "Select options:\n"
            "0) Exit \n"
            "1) set hue (%d)\n"
            "2) set saturation (%d)\n"
            "3) set contrast (%d)\n"
            "4) set brightness (%d)\n"
            "5) set preset color matrix\n"
            "6) unset color matrix\n",
            graphicsColorSettings.hue,
            graphicsColorSettings.saturation,
            graphicsColorSettings.contrast,
            graphicsColorSettings.brightness
            );
        fgets(buf, 64, stdin);

        switch (buf[0]) {
        case '1':
            printf("Enter hue (between -32768 and 32767):\n");
            fgets(buf, 64, stdin);
            NEXUS_Display_GetGraphicsColorSettings(display, &graphicsColorSettings);
            graphicsColorSettings.hue = strtol(buf, NULL, 0);
            rc = NEXUS_Display_SetGraphicsColorSettings(display, &graphicsColorSettings);
            BDBG_ASSERT(!rc);
            break;
        case '2':
            printf("Enter saturation (between -32768 and 32767):\n");
            fgets(buf, 64, stdin);
            NEXUS_Display_GetGraphicsColorSettings(display, &graphicsColorSettings);
            graphicsColorSettings.saturation = strtol(buf, NULL, 0);
            rc = NEXUS_Display_SetGraphicsColorSettings(display, &graphicsColorSettings);
            BDBG_ASSERT(!rc);
            break;
        case '3':
            printf("Enter contrast (between -32768 and 32767):\n");
            fgets(buf, 64, stdin);
            NEXUS_Display_GetGraphicsColorSettings(display, &graphicsColorSettings);
            graphicsColorSettings.contrast = strtol(buf, NULL, 0);
            rc = NEXUS_Display_SetGraphicsColorSettings(display, &graphicsColorSettings);
            BDBG_ASSERT(!rc);
            break;
        case '4':
            printf("Enter brightness (between -32768 and 32767):\n");
            fgets(buf, 64, stdin);
            NEXUS_Display_GetGraphicsColorSettings(display, &graphicsColorSettings);
            graphicsColorSettings.brightness = strtol(buf, NULL, 0);
            rc = NEXUS_Display_SetGraphicsColorSettings(display, &graphicsColorSettings);
            BDBG_ASSERT(!rc);
            break;
        case '5':
            rc = NEXUS_Display_SetGraphicsColorMatrix(display, &g_colorMatrix0);
            BDBG_ASSERT(!rc);
            break;
        case '6':
            rc = NEXUS_Display_SetGraphicsColorMatrix(display, NULL);
            BDBG_ASSERT(!rc);
            break;
        case '0':
            done = true;
            break;
        }
    }

    /* Shutdown */
    BKNI_DestroyEvent(checkpointEvent);
    NEXUS_Graphics2D_Close(gfx);
    NEXUS_Surface_Destroy(framebuffer);
    NEXUS_Display_Close(display);
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
