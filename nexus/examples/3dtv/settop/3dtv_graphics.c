/******************************************************************************
 *    (c)2008-2014 Broadcom Corporation
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
3dtv_graphics.c shows how to do 3DTV graphics in Side-by-Side half, 
Top-and-Bottom half and full-resolution 3D modes.

All examples under nexus/examples/3dtv require the new BVN in 40nm 
platforms and supporting SW.
**/

#include "nexus_platform.h"
#include "nexus_core_utils.h"
#include <stdio.h>

#if NEXUS_HAS_DISPLAY
#include "nexus_display.h"
#include "nexus_graphics2d.h"
#include "nexus_component_output.h"
#if NEXUS_HAS_HDMI_OUTPUT
#include "nexus_hdmi_output.h"
#endif
#include "bstd.h"
#include "bkni.h"
#include <string.h>

typedef enum mode3d {
    mode3d_eHalfresSbs,
    mode3d_eHalfresTab,
    mode3d_eFullres
} mode3d;

#define NUM_TEST_MODES 6
struct {
    int zoffset;
} g_testmode[NUM_TEST_MODES] = {
    {0},
    {-5},
    {-10},
    {0},
    {5},
    {10},
};

#define FRAMEBUFFER_WIDTH 1280
#define FRAMEBUFFER_HEIGHT 720

int main(int argc, char **argv)
{
    NEXUS_DisplayHandle display;
    NEXUS_DisplaySettings displaySettings;
    NEXUS_PlatformSettings platformSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_VideoFormatInfo displayFormatInfo;
    NEXUS_SurfaceHandle framebuffer, offscreen;
    NEXUS_SurfaceCreateSettings createSurfaceSettings;
    NEXUS_GraphicsFramebuffer3D framebuffer3d;
    NEXUS_Graphics2DFillSettings fillSettings;
    NEXUS_Graphics2DHandle gfx;
    NEXUS_GraphicsSettings graphicsSettings;
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_HdmiOutputStatus hdmiStatus;
#endif
    NEXUS_Error rc;
    mode3d mode = mode3d_eHalfresSbs;
    unsigned i;

    if (argc > 1) {
        if (!strcmp(argv[1], "tab")) {
            mode = mode3d_eHalfresTab;
        }
        else if (!strcmp(argv[1], "fullres")) {
            mode = mode3d_eFullres;
        }
    }

    /* Bring up all modules for a platform in a default configuration for this platform */
    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init(&platformSettings);
    NEXUS_Platform_GetConfiguration(&platformConfig);

    /* Bringup display */
    NEXUS_Display_GetDefaultSettings(&displaySettings);
    switch (mode) {
        case mode3d_eHalfresSbs:
            displaySettings.format = NEXUS_VideoFormat_e720p;
            displaySettings.display3DSettings.overrideOrientation = true;
            displaySettings.display3DSettings.orientation = NEXUS_VideoOrientation_e3D_LeftRight;
            break;
        case mode3d_eHalfresTab:
            displaySettings.format = NEXUS_VideoFormat_e720p;
            displaySettings.display3DSettings.overrideOrientation = true;
            displaySettings.display3DSettings.orientation = NEXUS_VideoOrientation_e3D_OverUnder;
            break;
        case mode3d_eFullres:
            displaySettings.format = NEXUS_VideoFormat_e720p_3DOU_AS;
            displaySettings.timingGenerator = NEXUS_DisplayTimingGenerator_eHdmiDvo; /* HDMI master mode is required */
            break;
        default:
            break;
    }
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
        NEXUS_HdmiOutputVendorSpecificInfoFrame vsi;
        NEXUS_HdmiOutput_GetVendorSpecificInfoFrame(platformConfig.outputs.hdmi[0], &vsi);
        switch (mode) {
        default: /* unused */
            vsi.hdmiVideoFormat = NEXUS_HdmiVendorSpecificInfoFrame_HDMIVideoFormat_eNone;
            break;
        case mode3d_eHalfresSbs:
            vsi.hdmiVideoFormat = NEXUS_HdmiVendorSpecificInfoFrame_HDMIVideoFormat_e3DFormat;
            vsi.hdmi3DStructure = NEXUS_HdmiVendorSpecificInfoFrame_3DStructure_eSidexSideHalf;
            break;
        case mode3d_eFullres:
        case mode3d_eHalfresTab:
            vsi.hdmiVideoFormat = NEXUS_HdmiVendorSpecificInfoFrame_HDMIVideoFormat_e3DFormat;
            vsi.hdmi3DStructure = NEXUS_HdmiVendorSpecificInfoFrame_3DStructure_eTopAndBottom;
            break;
        }
        NEXUS_HdmiOutput_SetVendorSpecificInfoFrame(platformConfig.outputs.hdmi[0], &vsi);
    }
#endif

    NEXUS_VideoFormat_GetInfo(displaySettings.format, &displayFormatInfo);

    /* create actual framebuffer */
    NEXUS_Surface_GetDefaultCreateSettings(&createSurfaceSettings);
    createSurfaceSettings.pixelFormat = NEXUS_PixelFormat_eA8_R8_G8_B8;
    switch (mode) {
        case mode3d_eHalfresSbs:
            createSurfaceSettings.width = FRAMEBUFFER_WIDTH/2;
            createSurfaceSettings.height = FRAMEBUFFER_HEIGHT;
            BDBG_ASSERT((unsigned)(createSurfaceSettings.width*2) <= displayFormatInfo.width);
            BDBG_ASSERT(createSurfaceSettings.height <= displayFormatInfo.height);
            break;
        case mode3d_eHalfresTab:
            createSurfaceSettings.width = FRAMEBUFFER_WIDTH;
            createSurfaceSettings.height = FRAMEBUFFER_HEIGHT/2;
            BDBG_ASSERT(createSurfaceSettings.width <= displayFormatInfo.width);
            BDBG_ASSERT((unsigned)(createSurfaceSettings.height*2) <= displayFormatInfo.height);
            break;
        case mode3d_eFullres:
            createSurfaceSettings.width = displayFormatInfo.width;
            createSurfaceSettings.height = displayFormatInfo.height;
            break;
    }

    createSurfaceSettings.heap = NEXUS_Platform_GetFramebufferHeap(0);
    framebuffer = NEXUS_Surface_Create(&createSurfaceSettings);

    /* create app offscreen buffer */
    NEXUS_Surface_GetDefaultCreateSettings(&createSurfaceSettings);
    createSurfaceSettings.pixelFormat = NEXUS_PixelFormat_eA8_R8_G8_B8;
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
    #define MAX_OFFSET 10 /* this is the largest value found in g_testmode.zoffset */

    /* position the surface in the middle of the display so we have room to move it around.
       VDC does not support out-of-bound surfaces */
    switch (mode) {
        case mode3d_eHalfresSbs:
            graphicsSettings.position.x = MAX_OFFSET;
            graphicsSettings.position.width = displayFormatInfo.width/2 - graphicsSettings.position.x*2;
            graphicsSettings.clip.width = FRAMEBUFFER_WIDTH/2; 
            graphicsSettings.clip.height = FRAMEBUFFER_HEIGHT;
            break;
        case mode3d_eHalfresTab:
            graphicsSettings.position.x = MAX_OFFSET;
            graphicsSettings.position.width = displayFormatInfo.width - graphicsSettings.position.x*2;
            graphicsSettings.clip.width = FRAMEBUFFER_WIDTH; 
            graphicsSettings.clip.height = FRAMEBUFFER_HEIGHT/2;
            break;
        case mode3d_eFullres:
            graphicsSettings.position.x = MAX_OFFSET;
            graphicsSettings.position.width = displayFormatInfo.width - graphicsSettings.position.x*2;
            graphicsSettings.clip.width = displayFormatInfo.width;
            graphicsSettings.clip.height = displayFormatInfo.height;
            break;
    }
    rc = NEXUS_Display_SetGraphicsSettings(display, &graphicsSettings);
    BDBG_ASSERT(!rc);
    NEXUS_Graphics_GetDefaultFramebuffer3D(&framebuffer3d);
    /* same framebuffer for both eyes */
    framebuffer3d.main = framebuffer;
    framebuffer3d.right = framebuffer;
    switch (mode) {
        case mode3d_eHalfresSbs:
            framebuffer3d.orientation = NEXUS_VideoOrientation_e3D_LeftRight;
            break;
        case mode3d_eHalfresTab:
            framebuffer3d.orientation = NEXUS_VideoOrientation_e3D_OverUnder;
            break;
        case mode3d_eFullres:
            framebuffer3d.orientation = NEXUS_VideoOrientation_e3D_OverUnder;
            break;
    }
    rc = NEXUS_Display_SetGraphicsFramebuffer3D(display, &framebuffer3d);
    BDBG_ASSERT(!rc);

    /* the code that does the blitting is common for all three test modes */
    for (i=0;;i++) {
        NEXUS_Graphics2DBlitSettings blitSettings;
        int zoffset = g_testmode[i].zoffset;
        
        if (i == NUM_TEST_MODES) i = 0;

        printf("%d zoffset\n", zoffset);
        NEXUS_Graphics2D_GetDefaultBlitSettings(&blitSettings);
        blitSettings.source.surface = offscreen;
        blitSettings.output.surface = framebuffer;
        rc = NEXUS_Graphics2D_Blit(gfx, &blitSettings);
        BDBG_ASSERT(!rc);

        /* this moves the right/bottom window */
        NEXUS_Display_GetGraphicsSettings(display, &graphicsSettings);
        graphicsSettings.graphics3DSettings.rightViewOffset = zoffset;
        rc = NEXUS_Display_SetGraphicsSettings(display, &graphicsSettings);
        BDBG_ASSERT(!rc);            

        NEXUS_Graphics2D_Checkpoint(gfx, NULL);
        getchar();
    }

    return 0;
}
#else /* NEXUS_HAS_DISPLAY */
int main(void)
{
    printf("This application is not supported on this platform (needs display)!\n");
    return 0;
}
#endif
