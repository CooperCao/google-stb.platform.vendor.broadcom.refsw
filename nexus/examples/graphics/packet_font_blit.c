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
#include "nexus_platform.h"
#include <stdio.h>
#if NEXUS_HAS_GRAPHICS2D && NEXUS_HAS_DISPLAY
#include "nexus_surface.h"
#include "nexus_graphics2d.h"
#include "bm2mc_packet.h"
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

void set_event(void *data, int unused)
{
    BSTD_UNUSED(unused);
    BKNI_SetEvent((BKNI_EventHandle)data);
}

static void checkpoint(NEXUS_Graphics2DHandle gfx, BKNI_EventHandle event)
{
    NEXUS_Error rc;

    rc = NEXUS_Graphics2D_Checkpoint(gfx, NULL);
    if (rc == NEXUS_GRAPHICS2D_QUEUED) {
        rc = BKNI_WaitForEvent(event, BKNI_INFINITE);
    }
    BDBG_ASSERT(!rc);
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
    BKNI_EventHandle spaceAvailableEvent, checkpointEvent;
    NEXUS_PlatformSettings platformSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_VideoFormatInfo info;
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_HdmiOutputStatus hdmiStatus;
#endif
    int i;
    NEXUS_Error rc;
    BM2MC_PACKET_Plane framebufferPlane, glyphPlane;

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
    NEXUS_Surface_InitPlane(framebuffer, &framebufferPlane);

    NEXUS_Surface_GetDefaultCreateSettings(&createSettings);
    createSettings.pixelFormat = NEXUS_PixelFormat_eA8;
    createSettings.width = GLYPH_WIDTH;
    createSettings.height = GLYPH_HEIGHT;
    glyphSurface = NEXUS_Surface_Create(&createSettings);
    NEXUS_Surface_InitPlane(glyphSurface, &glyphPlane);
    NEXUS_Surface_GetMemory(glyphSurface, &mem);
    BDBG_ASSERT(mem.pitch == GLYPH_WIDTH);
    BKNI_Memcpy(mem.buffer, g_glyph, mem.pitch * createSettings.height);
    NEXUS_Surface_Flush(glyphSurface);

    BKNI_CreateEvent(&spaceAvailableEvent);
    BKNI_CreateEvent(&checkpointEvent);

    gfx = NEXUS_Graphics2D_Open(0, NULL);

    NEXUS_Graphics2D_GetSettings(gfx, &gfxSettings);
    gfxSettings.packetSpaceAvailable.callback = set_event;
    gfxSettings.packetSpaceAvailable.context = spaceAvailableEvent;
    gfxSettings.checkpointCallback.callback = set_event;
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

    NEXUS_Display_GetGraphicsSettings(display, &graphicsSettings);
    graphicsSettings.enabled = true;
    rc = NEXUS_Display_SetGraphicsSettings(display, &graphicsSettings);
    BDBG_ASSERT(!rc);
    rc = NEXUS_Display_SetGraphicsFramebuffer(display, framebuffer);
    BDBG_ASSERT(!rc);

    {
        NEXUS_Error rc;
        void *buffer, *next;
        size_t size;

        BM2MC_PACKET_Blend copyColor = {
            BM2MC_PACKET_BlendFactor_eSourceColor, BM2MC_PACKET_BlendFactor_eSourceAlpha, false,
            BM2MC_PACKET_BlendFactor_eDestinationColor, BM2MC_PACKET_BlendFactor_eInverseSourceAlpha, false,
            BM2MC_PACKET_BlendFactor_eZero};
        BM2MC_PACKET_Blend copyAlpha = {
            BM2MC_PACKET_BlendFactor_eOne, BM2MC_PACKET_BlendFactor_eOne, false,
            BM2MC_PACKET_BlendFactor_eZero, BM2MC_PACKET_BlendFactor_eZero, false,
            BM2MC_PACKET_BlendFactor_eZero};

        rc = NEXUS_Graphics2D_GetPacketBuffer(gfx, &buffer, &size, 1024);
        BDBG_ASSERT(!rc);
        BDBG_ASSERT(size >= 1024); /* first blit must succeed */

        next = buffer;

        {
        BM2MC_PACKET_PacketSourceFeeder *pPacket = next;
        BM2MC_PACKET_INIT(pPacket, SourceFeeder, false );
        pPacket->plane = glyphPlane;
        pPacket->color = 0xFF0000FF;
        next = ++pPacket;
        }

        {
        BM2MC_PACKET_PacketDestinationFeeder *pPacket = next;
        BM2MC_PACKET_INIT(pPacket, DestinationFeeder, false);
        pPacket->plane = framebufferPlane;
        pPacket->color = 0;
        next = ++pPacket;
        }

        {
        BM2MC_PACKET_PacketOutputFeeder *pPacket = next;
        BM2MC_PACKET_INIT(pPacket, OutputFeeder, false);
        pPacket->plane = framebufferPlane;
        next = ++pPacket;
        }

        {
        BM2MC_PACKET_PacketBlend *pPacket = (BM2MC_PACKET_PacketBlend *)next;
        BM2MC_PACKET_INIT( pPacket, Blend, false );
        pPacket->color_blend = copyColor;
        pPacket->alpha_blend = copyAlpha;
        pPacket->color = 0;
        next = ++pPacket;
        }

        rc = NEXUS_Graphics2D_PacketWriteComplete(gfx, (uint8_t*)next - (uint8_t*)buffer);
        BDBG_ASSERT(!rc);

        BDBG_WRN(("starting"));
        for(i=0;;) {
            unsigned x;
            rc = NEXUS_Graphics2D_GetPacketBuffer(gfx, &buffer, &size, 1024);
            BDBG_ASSERT(!rc);
            if (!size) {
                /* internal queue is full. wait for space to become available. */
                BKNI_WaitForEvent(spaceAvailableEvent, 0xffffffff);
                continue;
            }

            next = buffer;

            /* submit 10 characters at a time */
            for (x=0;x<10;x++) {
            BM2MC_PACKET_PacketBlendBlit *pPacket = next;
            BM2MC_PACKET_INIT(pPacket, BlendBlit, true);
            pPacket->src_rect.x = 0;
            pPacket->src_rect.y = 0;
            pPacket->src_rect.width = GLYPH_WIDTH;
            pPacket->src_rect.height = GLYPH_HEIGHT;
            pPacket->out_point.x = rand() % (info.width/(GLYPH_WIDTH+4)) * (GLYPH_WIDTH+4);
            pPacket->out_point.y = rand() % (info.height/(GLYPH_HEIGHT+4)) * (GLYPH_HEIGHT+4);
            pPacket->dst_point.x = pPacket->out_point.x;
            pPacket->dst_point.y = pPacket->out_point.y;
            next = ++pPacket;

                i++;
            }

            rc = NEXUS_Graphics2D_PacketWriteComplete(gfx, (uint8_t*)next - (uint8_t*)buffer);
            BDBG_ASSERT(!rc);

            if (i && i%50000==0) {
                checkpoint(gfx, checkpointEvent);
                BDBG_WRN(("%d blits completed", i));
            }
        }
    }
    return 0;
}
#else
int main(void)
{
    printf("This application is not supported on this platform\n");
    return 0;
}
#endif
