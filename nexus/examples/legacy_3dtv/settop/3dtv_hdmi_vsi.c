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
#include <stdlib.h>
#include <string.h>

/**
3dtv_hdmi_vsi.c shows how to set the HDMI VSI to trigger automatic 3DTV mode switches
**/

BDBG_MODULE(3dtv_hdmi_vsi);

    static const struct {
        NEXUS_VideoFormat nformat;
        char formatName[32];
    } b_formatString[] = {
        {NEXUS_VideoFormat_eNtsc                        ,"480i 59.94/60Hz"},
        {NEXUS_VideoFormat_e1080i                       ,"1080i 59.94/60Hz"},
        {NEXUS_VideoFormat_e1080p24hz                   ,"1080p 23.97/24Hz"},
        {NEXUS_VideoFormat_e1080p25hz                   ,"1080p 25Hz"},
        {NEXUS_VideoFormat_e1080p30hz                   ,"1080p 30Hz"},
        {NEXUS_VideoFormat_e1080i50hz                   ,"1080i 50Hz"},
        {NEXUS_VideoFormat_e1080p60hz                   ,"1080p 60Hz"},
        {NEXUS_VideoFormat_e720p                        ,"720p 59.94/60Hz"},
        {NEXUS_VideoFormat_e720p24hz                    ,"720p 24Hz"},
        {NEXUS_VideoFormat_e720p25hz                    ,"720p 25Hz"},
        {NEXUS_VideoFormat_e720p30hz                    ,"720p 30Hz"},
        {NEXUS_VideoFormat_e720p50hz                    ,"720p 50Hz"},
        {NEXUS_VideoFormat_e480p                        ,"480p 59.94/60Hz"},
        {NEXUS_VideoFormat_e576p                        ,"576p"},
        {NEXUS_VideoFormat_eVesa640x480p60hz            ,"VGA 640x480p"},
        {NEXUS_VideoFormat_e1080p50hz                   ,"1080p 50Hz"},
        {NEXUS_VideoFormat_e1440x480p60hz               ,"1440x480p 59.94/60Hz"},
        {NEXUS_VideoFormat_e1440x576p50hz               ,"1440x576p 59.94/60Hz"}
    };

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
    NEXUS_Graphics2DBlitSettings blitSettings;
    NEXUS_Graphics2DHandle gfx;
    NEXUS_GraphicsSettings graphicsSettings;
    NEXUS_HdmiOutputHandle hdmiOutput;
    NEXUS_HdmiOutputVendorSpecificInfoFrame vsi;
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_HdmiOutputStatus hdmiStatus;
#endif
    NEXUS_Error rc;
    unsigned i,j;

    /* Bring up all modules for a platform in a default configuraiton for this platform */
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
    hdmiOutput = platformConfig.outputs.hdmi[0];
    if (hdmiOutput) {
        rc = NEXUS_Display_AddOutput(display, NEXUS_HdmiOutput_GetVideoConnector(hdmiOutput));
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
	} else
#endif
	{
    	BDBG_ERR(("this example requires HDMI output"));
    	return -1;
	}

    NEXUS_VideoFormat_GetInfo(displaySettings.format, &displayFormatInfo);

    /* create actual framebuffer */
    NEXUS_Surface_GetDefaultCreateSettings(&createSurfaceSettings);
    /* older platforms cannot support 32bpp pixel format on a large framebuffer due to bandwidth issues. 
       you can either use a 16bpp pixel format, or reduce the framebuffer width */
#if (BCHP_CHIP==7401) || (BCHP_CHIP==7403) || (BCHP_CHIP==7400)
    createSurfaceSettings.pixelFormat = NEXUS_PixelFormat_eA4_R4_G4_B4;
#else
    createSurfaceSettings.pixelFormat = NEXUS_PixelFormat_eA8_R8_G8_B8;
#endif

#if BCHP_CHIP == 7420
#define FRAMEBUFFER_WIDTH 1280
#else
#define FRAMEBUFFER_WIDTH 1440
#endif
    createSurfaceSettings.width = FRAMEBUFFER_WIDTH;
    createSurfaceSettings.height = 1080;
    createSurfaceSettings.heap = NEXUS_Platform_GetFramebufferHeap(0);
    framebuffer = NEXUS_Surface_Create(&createSurfaceSettings);

    /* create app offscreen buffer */
    NEXUS_Surface_GetDefaultCreateSettings(&createSurfaceSettings);
#if (BCHP_CHIP==7401) || (BCHP_CHIP==7403) || (BCHP_CHIP==7400)
    createSurfaceSettings.pixelFormat = NEXUS_PixelFormat_eA4_R4_G4_B4;
#else
    createSurfaceSettings.pixelFormat = NEXUS_PixelFormat_eA8_R8_G8_B8;
#endif
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

    while (1) {
#define BUFSIZE 64
        char buf[BUFSIZE];

        printf(
            "1) 3D side-by-side half\n"
            "2) 3D over-under half\n"
            "3) 2D\n"
            "4) Read EDID 3D capabilities\n"
            "Select mode:\n");
        fgets(buf, BUFSIZE, stdin);

        switch (atoi(buf)) {
        case 1:
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

            NEXUS_HdmiOutput_GetVendorSpecificInfoFrame(hdmiOutput, &vsi);
            vsi.hdmiVideoFormat = NEXUS_HdmiVendorSpecificInfoFrame_HDMIVideoFormat_e3DFormat;
            vsi.hdmi3DStructure = NEXUS_HdmiVendorSpecificInfoFrame_3DStructure_eSidexSideHalf;
            rc = NEXUS_HdmiOutput_SetVendorSpecificInfoFrame(hdmiOutput, &vsi);
            BDBG_ASSERT(!rc);
            break;

        case 2:
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

            NEXUS_HdmiOutput_GetVendorSpecificInfoFrame(hdmiOutput, &vsi);
            vsi.hdmiVideoFormat = NEXUS_HdmiVendorSpecificInfoFrame_HDMIVideoFormat_e3DFormat;
            vsi.hdmi3DStructure = NEXUS_HdmiVendorSpecificInfoFrame_3DStructure_eTopAndBottom;
            rc = NEXUS_HdmiOutput_SetVendorSpecificInfoFrame(hdmiOutput, &vsi);
            BDBG_ASSERT(!rc);
            break;

        case 3:
            printf("mode: 2D\n");
            NEXUS_Graphics2D_GetDefaultBlitSettings(&blitSettings);
            blitSettings.source.surface = offscreen;
            blitSettings.output.surface = framebuffer;
            rc = NEXUS_Graphics2D_Blit(gfx, &blitSettings);
            BDBG_ASSERT(!rc);

            NEXUS_HdmiOutput_GetVendorSpecificInfoFrame(hdmiOutput, &vsi);
            vsi.hdmiVideoFormat = NEXUS_HdmiVendorSpecificInfoFrame_HDMIVideoFormat_eNone;
            /* vsi.hdmi3DStructure is a don't care */
            rc = NEXUS_HdmiOutput_SetVendorSpecificInfoFrame(hdmiOutput, &vsi);
            BDBG_ASSERT(!rc);
            break;

        case 4:
            {
            NEXUS_HdmiOutputStatus status;
            rc = NEXUS_HdmiOutput_GetStatus(hdmiOutput, &status);
            BDBG_ASSERT(!rc);
            printf("List of Broadcom supported 3D Formats in EDID:\n");
            for (i=0; i<NEXUS_VideoFormat_eMax; i++) {
                if (status.hdmi3DFormatsSupported[i]) {
                    for (j=0; j<(int)sizeof(b_formatString)/sizeof(*b_formatString); j++) {
                        if (i == b_formatString[j].nformat) {
                            printf(" %s 3D structure: \n", (char *) b_formatString[j].formatName);
                        }
                    }
                    if (status.hdmi3DFormatsSupported[i] & NEXUS_HdmiOutput_3DStructure_FramePacking)
                        printf("    FramePacking\n");
                    if (status.hdmi3DFormatsSupported[i] & NEXUS_HdmiOutput_3DStructure_FieldAlternative)
                        printf("    FieldAlternative\n");
                    if (status.hdmi3DFormatsSupported[i] & NEXUS_HdmiOutput_3DStructure_LineAlternative)
                        printf("    LineAlternative\n");
                    if (status.hdmi3DFormatsSupported[i] & NEXUS_HdmiOutput_3DStructure_SideBySideFull)
                        printf("    SidebySide Full\n");
                    if (status.hdmi3DFormatsSupported[i] & NEXUS_HdmiOutput_3DStructure_LDepth)
                        printf("    LDepth\n");
                    if (status.hdmi3DFormatsSupported[i] & NEXUS_HdmiOutput_3DStructure_LDepthGraphics)
                        printf("    LDepth + Graphics\n");
                    if (status.hdmi3DFormatsSupported[i] & NEXUS_HdmiOutput_3DStructure_TopAndBottom)
                        printf("    TopAndBottm\n");
                    if (status.hdmi3DFormatsSupported[i] & NEXUS_HdmiOutput_3DStructure_SideBySideHalfHorizontal)
                        printf("    SidebySide Half with Horizontal sub-sampling\n");
                    if (status.hdmi3DFormatsSupported[i] & NEXUS_HdmiOutput_3DStructure_SideBySideHalfQuincunx)
                        printf("    SidebySide Half with Quincunx matrix\n");
                }
            }
            }
            break;
        }
        NEXUS_Graphics2D_Checkpoint(gfx, NULL);
    }

    return 0;
}

