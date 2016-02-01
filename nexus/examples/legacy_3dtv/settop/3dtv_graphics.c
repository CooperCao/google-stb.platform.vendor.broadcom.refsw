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

/**
3dtv_graphics.c shows how to do 3DTV graphics in a O/U half and L/R half graphics framebuffer.
**/

#include "nexus_platform.h"
#include "nexus_core_utils.h"
#include "nexus_display.h"
#include "nexus_graphics2d.h"
#include "nexus_hdmi_output.h"
#include "nexus_component_output.h"
#if NEXUS_HAS_HDMI_OUTPUT
#include "nexus_hdmi_output.h"
#endif
#include "bstd.h"
#include "bkni.h"
#include <stdio.h>
#include <string.h>

#define NUM_TEST_MODES 14
struct {
    bool sbs;
    int zoffset;
} g_testmode[NUM_TEST_MODES] = {
    {true, 0},
    {true, -5},
    {true, -10},
    {true, 0},
    {true, 5},
    {true, 10},
    {true, 0},
    {false, 0},
    {false, -5},
    {false, -10},
    {false, 0},
    {false, 5},
    {false, 10},
    {false, 0}
};

#define FRAMEBUFFER_WIDTH 1440

int main(void)
{
    NEXUS_DisplayHandle display;
    NEXUS_DisplaySettings displaySettings;
    NEXUS_PlatformSettings platformSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_VideoFormatInfo displayFormatInfo;
    NEXUS_SurfaceHandle framebuffer, offscreen;
    NEXUS_SurfaceCreateSettings createSurfaceSettings;
    NEXUS_Graphics2DFillSettings fillSettings;
    NEXUS_Graphics2DHandle gfx;
    NEXUS_GraphicsSettings graphicsSettings;
    NEXUS_PixelFormat pixelFormat;
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_HdmiOutputStatus hdmiStatus;
#endif
    NEXUS_Error rc;
    unsigned i;

    /* Bring up all modules for a platform in a default configuration for this platform */
    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init(&platformSettings);
    NEXUS_Platform_GetConfiguration(&platformConfig);

    NEXUS_Display_GetDefaultSettings(&displaySettings);
    displaySettings.format = NEXUS_VideoFormat_e1080i;
    display = NEXUS_Display_Open(0, &displaySettings);

#if NEXUS_NUM_COMPONENT_OUTPUTS
    rc = NEXUS_Display_AddOutput(display, NEXUS_ComponentOutput_GetConnector(platformConfig.outputs.component[0]));
    BDBG_ASSERT(!rc);
#endif
#if NEXUS_NUM_HDMI_OUTPUTS
    rc = NEXUS_Display_AddOutput(display, NEXUS_HdmiOutput_GetVideoConnector(platformConfig.outputs.hdmi[0]));
    BDBG_ASSERT(!rc);
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

    /* older platforms cannot support 32bpp pixel format on a large framebuffer due 
       to system bandwidth limitations. you can either use a 16bpp pixel format, 
       or reduce the framebuffer width */
    NEXUS_VideoFormat_GetInfo(displaySettings.format, &displayFormatInfo);
#if (BCHP_CHIP==7401) || (BCHP_CHIP==7403) || (BCHP_CHIP==7400)
    pixelFormat = NEXUS_PixelFormat_eA4_R4_G4_B4;
#else
    pixelFormat = NEXUS_PixelFormat_eA8_R8_G8_B8;
#endif

    /* create actual framebuffer */
    NEXUS_Surface_GetDefaultCreateSettings(&createSurfaceSettings);
    createSurfaceSettings.pixelFormat = pixelFormat;
    createSurfaceSettings.width = FRAMEBUFFER_WIDTH;
    createSurfaceSettings.height = 1080;
    createSurfaceSettings.heap = NEXUS_Platform_GetFramebufferHeap(0);
    framebuffer = NEXUS_Surface_Create(&createSurfaceSettings);

    /* create app offscreen buffer */
    NEXUS_Surface_GetDefaultCreateSettings(&createSurfaceSettings);
    createSurfaceSettings.pixelFormat = pixelFormat;
    createSurfaceSettings.width = 720;
    createSurfaceSettings.height = 480;
    offscreen = NEXUS_Surface_Create(&createSurfaceSettings);

    gfx = NEXUS_Graphics2D_Open(0, NULL);

    /* build the offscreen buffer */
    NEXUS_Graphics2D_GetDefaultFillSettings(&fillSettings);
    fillSettings.surface = offscreen;
    fillSettings.color = 0xFF000000;
    rc = NEXUS_Graphics2D_Fill(gfx, &fillSettings);
    BDBG_ASSERT(!rc);
    NEXUS_Graphics2D_GetDefaultFillSettings(&fillSettings);
    fillSettings.surface = offscreen;
    fillSettings.rect.x = 100;
    fillSettings.rect.y = 100;
    fillSettings.rect.width = 100;
    fillSettings.rect.height = 100;
    fillSettings.color = 0xFFFFFFFF;
    rc = NEXUS_Graphics2D_Fill(gfx, &fillSettings);
    BDBG_ASSERT(!rc);
    NEXUS_Graphics2D_GetDefaultFillSettings(&fillSettings);
    fillSettings.surface = framebuffer;
    fillSettings.color = 0xFF000000;
    rc = NEXUS_Graphics2D_Fill(gfx, &fillSettings);
    BDBG_ASSERT(!rc);

    NEXUS_Display_GetGraphicsSettings(display, &graphicsSettings);
    graphicsSettings.enabled = true;
    graphicsSettings.clip.width = FRAMEBUFFER_WIDTH;
    graphicsSettings.clip.height = displayFormatInfo.height;
    rc = NEXUS_Display_SetGraphicsSettings(display, &graphicsSettings);
    BDBG_ASSERT(!rc);

    rc = NEXUS_Display_SetGraphicsFramebuffer(display, framebuffer);
    BDBG_ASSERT(!rc);


    for (i=0;;i++) {
        NEXUS_Graphics2DBlitSettings blitSettings;

        if (i == NUM_TEST_MODES) i = 0;

#if NO_ZNORM
/**
The zoffset == 0 case is included to keep the example code simple.
**/
        if (g_testmode[i].sbs) {
            printf("mode: L/R half\n");
            NEXUS_Graphics2D_GetDefaultBlitSettings(&blitSettings);
            blitSettings.source.surface = offscreen;
            blitSettings.output.surface = framebuffer;
            blitSettings.output.rect.x = 0;
            blitSettings.output.rect.y = 0;
            blitSettings.output.rect.width = FRAMEBUFFER_WIDTH/2;
            blitSettings.output.rect.height = displayFormatInfo.height;
            rc = NEXUS_Graphics2D_Blit(gfx, &blitSettings);
            BDBG_ASSERT(!rc);

            NEXUS_Graphics2D_GetDefaultBlitSettings(&blitSettings);
            blitSettings.source.surface = offscreen;
            blitSettings.output.surface = framebuffer;
            blitSettings.output.rect.x = FRAMEBUFFER_WIDTH/2;
            blitSettings.output.rect.y = 0;
            blitSettings.output.rect.width = FRAMEBUFFER_WIDTH/2;
            blitSettings.output.rect.height = displayFormatInfo.height;
            rc = NEXUS_Graphics2D_Blit(gfx, &blitSettings);
            BDBG_ASSERT(!rc);
        }
        else {
            printf("mode: O/U half\n");
            NEXUS_Graphics2D_GetDefaultBlitSettings(&blitSettings);
            blitSettings.source.surface = offscreen;
            blitSettings.output.surface = framebuffer;
            blitSettings.output.rect.x = 0;
            blitSettings.output.rect.y = 0;
            blitSettings.output.rect.width = FRAMEBUFFER_WIDTH;
            blitSettings.output.rect.height = displayFormatInfo.height/2;
            rc = NEXUS_Graphics2D_Blit(gfx, &blitSettings);
            BDBG_ASSERT(!rc);

            NEXUS_Graphics2D_GetDefaultBlitSettings(&blitSettings);
            blitSettings.source.surface = offscreen;
            blitSettings.output.surface = framebuffer;
            blitSettings.output.rect.x = 0;
            blitSettings.output.rect.y = displayFormatInfo.height/2;
            blitSettings.output.rect.width = FRAMEBUFFER_WIDTH;
            blitSettings.output.rect.height = displayFormatInfo.height/2;
            rc = NEXUS_Graphics2D_Blit(gfx, &blitSettings);
            BDBG_ASSERT(!rc);
        }
#else

#define abs(X) ((X)>0?(X):-(X))

/* This code handles positive, negative and zero zoffset. */
        if (g_testmode[i].sbs) {
            int zoffset = g_testmode[i].zoffset;
            int scaled_zoffset = zoffset * (int)createSurfaceSettings.width / (int)(FRAMEBUFFER_WIDTH/2);

            printf("mode: L/R half with %d zoffset\n", zoffset);
            NEXUS_Graphics2D_GetDefaultBlitSettings(&blitSettings);
            blitSettings.source.surface = offscreen;
            blitSettings.source.rect.x = scaled_zoffset<0?-scaled_zoffset:0;
            blitSettings.source.rect.y = 0;
            blitSettings.source.rect.width = createSurfaceSettings.width - abs(scaled_zoffset);
            blitSettings.source.rect.height = createSurfaceSettings.height;
            blitSettings.output.surface = framebuffer;
            blitSettings.output.rect.x = zoffset>0?zoffset:0;
            blitSettings.output.rect.y = 0;
            blitSettings.output.rect.width = FRAMEBUFFER_WIDTH/2 - abs(zoffset);
            blitSettings.output.rect.height = displayFormatInfo.height;
            rc = NEXUS_Graphics2D_Blit(gfx, &blitSettings);
            BDBG_ASSERT(!rc);
            BKNI_Printf("source %u,%u,%u,%u\n", blitSettings.source.rect.x, blitSettings.source.rect.y, blitSettings.source.rect.width, blitSettings.source.rect.height);
            BKNI_Printf("output %u,%u,%u,%u\n", blitSettings.output.rect.x, blitSettings.output.rect.y, blitSettings.output.rect.width, blitSettings.output.rect.height);

            NEXUS_Graphics2D_GetDefaultBlitSettings(&blitSettings);
            blitSettings.source.surface = offscreen;
            blitSettings.source.rect.x = scaled_zoffset>0?scaled_zoffset:0;
            blitSettings.source.rect.y = 0;
            blitSettings.source.rect.width = createSurfaceSettings.width - abs(scaled_zoffset);
            blitSettings.source.rect.height = createSurfaceSettings.height;
            blitSettings.output.surface = framebuffer;
            blitSettings.output.rect.x = FRAMEBUFFER_WIDTH/2 - (zoffset<0?zoffset:0);
            blitSettings.output.rect.y = 0;
            blitSettings.output.rect.width = FRAMEBUFFER_WIDTH/2 - abs(zoffset);
            blitSettings.output.rect.height = displayFormatInfo.height;
            rc = NEXUS_Graphics2D_Blit(gfx, &blitSettings);
            BDBG_ASSERT(!rc);
            BKNI_Printf("source %u,%u,%u,%u\n", blitSettings.source.rect.x, blitSettings.source.rect.y, blitSettings.source.rect.width, blitSettings.source.rect.height);
            BKNI_Printf("output %u,%u,%u,%u\n", blitSettings.output.rect.x, blitSettings.output.rect.y, blitSettings.output.rect.width, blitSettings.output.rect.height);

            NEXUS_Graphics2D_Checkpoint(gfx, NULL);
        }
        else {
            int zoffset = g_testmode[i].zoffset;
            int scaled_zoffset = zoffset; /* no scaling in horizontal direction */

            printf("mode: O/U half with %d zoffset\n", zoffset);
            NEXUS_Graphics2D_GetDefaultBlitSettings(&blitSettings);
            blitSettings.source.surface = offscreen;
            blitSettings.source.rect.x = scaled_zoffset<0?-scaled_zoffset:0;
            blitSettings.source.rect.y = 0;
            blitSettings.source.rect.width = createSurfaceSettings.width - abs(scaled_zoffset);
            blitSettings.source.rect.height = createSurfaceSettings.height;
            blitSettings.output.surface = framebuffer;
            blitSettings.output.rect.x = zoffset>0?zoffset:0;
            blitSettings.output.rect.y = 0;
            blitSettings.output.rect.width = FRAMEBUFFER_WIDTH - abs(zoffset);
            blitSettings.output.rect.height = displayFormatInfo.height/2;
            rc = NEXUS_Graphics2D_Blit(gfx, &blitSettings);
            BDBG_ASSERT(!rc);
            BKNI_Printf("source %u,%u,%u,%u\n", blitSettings.source.rect.x, blitSettings.source.rect.y, blitSettings.source.rect.width, blitSettings.source.rect.height);
            BKNI_Printf("output %u,%u,%u,%u\n", blitSettings.output.rect.x, blitSettings.output.rect.y, blitSettings.output.rect.width, blitSettings.output.rect.height);

            NEXUS_Graphics2D_GetDefaultBlitSettings(&blitSettings);
            blitSettings.source.surface = offscreen;
            blitSettings.source.rect.x = scaled_zoffset>0?scaled_zoffset:0;
            blitSettings.source.rect.y = 0;
            blitSettings.source.rect.width = createSurfaceSettings.width - abs(scaled_zoffset);
            blitSettings.source.rect.height = createSurfaceSettings.height;
            blitSettings.output.surface = framebuffer;
            blitSettings.output.rect.x = zoffset<0?-zoffset:0;
            blitSettings.output.rect.y = displayFormatInfo.height/2;
            blitSettings.output.rect.width = FRAMEBUFFER_WIDTH - abs(zoffset);
            blitSettings.output.rect.height = displayFormatInfo.height/2;
            rc = NEXUS_Graphics2D_Blit(gfx, &blitSettings);
            BDBG_ASSERT(!rc);
            BKNI_Printf("source %u,%u,%u,%u\n", blitSettings.source.rect.x, blitSettings.source.rect.y, blitSettings.source.rect.width, blitSettings.source.rect.height);
            BKNI_Printf("output %u,%u,%u,%u\n", blitSettings.output.rect.x, blitSettings.output.rect.y, blitSettings.output.rect.width, blitSettings.output.rect.height);

            NEXUS_Graphics2D_Checkpoint(gfx, NULL);
        }
#endif
        getchar();
    }

    return 0;
}

