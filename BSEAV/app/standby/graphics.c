/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.

 ******************************************************************************/

#include "standby.h"
#include "util.h"

BDBG_MODULE(graphics);

extern B_StandbyNexusHandles g_StandbyNexusHandles;
extern B_DeviceState g_DeviceState;

pthread_t gfx2d_thread;

void *graphics2d_thread(void *context)
{
    NEXUS_Graphics2DFillSettings fillSettings;
    NEXUS_SurfaceCreateSettings blitSurfaceSettings;
    NEXUS_Error rc;
    unsigned i;
    uint8_t r = 0, g = 0, b = 0;
    int x_inc = 4, y_inc = 4;

    BSTD_UNUSED(context);

    if (!g_StandbyNexusHandles.blitSurface) {
        BDBG_WRN(("No surface found for blit test"));
        return NULL;
    }

    NEXUS_Surface_GetCreateSettings(g_StandbyNexusHandles.blitSurface, &blitSurfaceSettings);

    for(i=0;;i++) {

        if (g_DeviceState.stop_graphics2d) {
            rc = NEXUS_Graphics2D_Checkpoint(g_StandbyNexusHandles.gfx2d, NULL);
            if (rc == NEXUS_GRAPHICS2D_QUEUED) {
                rc = BKNI_WaitForEvent(g_StandbyNexusHandles.checkpointEvent, BKNI_INFINITE);
            }
            return NULL;
        }

        NEXUS_Graphics2D_GetDefaultFillSettings(&fillSettings);
        fillSettings.surface = g_StandbyNexusHandles.blitSurface;
        r += 1; g += 2; b += 3;
        fillSettings.color = (0xFF<<24) | (r << 16) | (g << 8) | b;

        while (1) {
            rc = NEXUS_Graphics2D_Fill(g_StandbyNexusHandles.gfx2d, &fillSettings);
            if (rc == NEXUS_GRAPHICS2D_QUEUE_FULL) {
                BKNI_WaitForEvent(g_StandbyNexusHandles.spaceAvailableEvent, BKNI_INFINITE);
            } else {
                break;
            }
        }

        rc = NEXUS_Graphics2D_Checkpoint(g_StandbyNexusHandles.gfx2d, NULL);
        if (rc == NEXUS_GRAPHICS2D_QUEUED) {
            rc = BKNI_WaitForEvent(g_StandbyNexusHandles.checkpointEvent, BKNI_INFINITE);
        }
        BDBG_ASSERT(!rc);

        g_DeviceState.blit_rect.x += x_inc;
        g_DeviceState.blit_rect.y += y_inc;
        g_DeviceState.blit_rect.width = GUI_WIDTH;
        g_DeviceState.blit_rect.height = GUI_WIDTH;
        if (g_DeviceState.blit_rect.x < 0 || g_DeviceState.blit_rect.x + g_DeviceState.blit_rect.width > blitSurfaceSettings.width) {
            x_inc = -x_inc;
            g_DeviceState.blit_rect.x += x_inc;
        }
        if (g_DeviceState.blit_rect.y < 0 || g_DeviceState.blit_rect.y + g_DeviceState.blit_rect.height > blitSurfaceSettings.height) {
            y_inc = -y_inc;
            g_DeviceState.blit_rect.y += y_inc;
        }
        render_ui();
        BKNI_Sleep(25);
    }
}

int graphics2d_start(void)
{
    if(g_DeviceState.graphics2d_started)
        return 0;

    g_DeviceState.stop_graphics2d=false;
    pthread_create(&gfx2d_thread, NULL, graphics2d_thread, NULL);

    g_DeviceState.graphics2d_started = true;

    return 0;
}

void graphics2d_stop(void)
{
    if(!g_DeviceState.graphics2d_started)
        return;

    g_DeviceState.stop_graphics2d=true;
    pthread_join(gfx2d_thread, NULL);

    g_DeviceState.graphics2d_started = false;
}

void complete(void *data, int unused)
{
    BSTD_UNUSED(unused);
    BKNI_SetEvent((BKNI_EventHandle)data);
}

void graphics2d_create(void)
{
    NEXUS_Graphics2DSettings gfxSettings;
    NEXUS_SurfaceCreateSettings createSettings;
    NEXUS_Graphics2DFillSettings fillSettings;
    NEXUS_DisplayCapabilities displayCap;

    if(g_StandbyNexusHandles.gfx2d)
        return;

    BKNI_CreateEvent(&g_StandbyNexusHandles.checkpointEvent);
    BKNI_CreateEvent(&g_StandbyNexusHandles.spaceAvailableEvent);

    g_StandbyNexusHandles.gfx2d = NEXUS_Graphics2D_Open(NEXUS_ANY_ID, NULL);
    BDBG_ASSERT(g_StandbyNexusHandles.gfx2d);
    NEXUS_Graphics2D_GetSettings(g_StandbyNexusHandles.gfx2d, &gfxSettings);
    gfxSettings.checkpointCallback.callback = complete;
    gfxSettings.checkpointCallback.context = g_StandbyNexusHandles.checkpointEvent;
    gfxSettings.packetSpaceAvailable.callback = complete;
    gfxSettings.packetSpaceAvailable.context = g_StandbyNexusHandles.spaceAvailableEvent;
    NEXUS_Graphics2D_SetSettings(g_StandbyNexusHandles.gfx2d, &gfxSettings);

    NEXUS_GetDisplayCapabilities(&displayCap);

    if(displayCap.display[0].graphics.width &&
       displayCap.display[0].graphics.height &&
       g_StandbyNexusHandles.displayHD) {

        NEXUS_Surface_GetDefaultCreateSettings(&createSettings);
        createSettings.pixelFormat = NEXUS_PixelFormat_eA8_R8_G8_B8;
        createSettings.width = displayCap.display[0].graphics.width;
        createSettings.height = displayCap.display[0].graphics.height;
        createSettings.heap = NEXUS_Platform_GetFramebufferHeap(0);
        g_StandbyNexusHandles.blitSurface = NEXUS_Surface_Create(&createSettings);

        NEXUS_Graphics2D_GetDefaultFillSettings(&fillSettings);
        fillSettings.surface = g_StandbyNexusHandles.blitSurface;
        fillSettings.rect.width = createSettings.width;
        fillSettings.rect.height = createSettings.height;
        fillSettings.color = 0xFF0000FF;
        NEXUS_Graphics2D_Fill(g_StandbyNexusHandles.gfx2d, &fillSettings);
    }

#if 0
    if(displayCap.display[1].graphics.width &&
       displayCap.display[1].graphics.height &&
       g_StandbyNexusHandles.displaySD) {

        NEXUS_Surface_GetDefaultCreateSettings(&createSettings);
        createSettings.pixelFormat = NEXUS_PixelFormat_eA8_R8_G8_B8;
        /* displayCap.display[1].graphics.width and height not reliable for all platforms (7425, 7435)*/
        createSettings.width =  720;
        createSettings.height = 576;
        createSettings.heap = NEXUS_Platform_GetFramebufferHeap(1);
        g_StandbyNexusHandles.blitSurface = NEXUS_Surface_Create(&createSettings);

        NEXUS_Graphics2D_GetDefaultFillSettings(&fillSettings);
        fillSettings.surface = g_StandbyNexusHandles.blitSurface;
        fillSettings.rect.width = createSettings.width;
        fillSettings.rect.height = createSettings.height;
        fillSettings.color = 0xFF0000FF;
        NEXUS_Graphics2D_Fill(g_StandbyNexusHandles.gfx2d, &fillSettings);
    }
#endif

    BKNI_Memset(&g_DeviceState.blit_rect, 0, sizeof(g_DeviceState.blit_rect));
}

void graphics2d_destroy(void)
{
    if(g_StandbyNexusHandles.blitSurface)
        NEXUS_Surface_Destroy(g_StandbyNexusHandles.blitSurface);
    g_StandbyNexusHandles.blitSurface = NULL;

    if(g_StandbyNexusHandles.gfx2d)
        NEXUS_Graphics2D_Close(g_StandbyNexusHandles.gfx2d);
    g_StandbyNexusHandles.gfx2d = NULL;
    if(g_StandbyNexusHandles.checkpointEvent)
        BKNI_DestroyEvent(g_StandbyNexusHandles.checkpointEvent);
    g_StandbyNexusHandles.checkpointEvent = NULL;
    if(g_StandbyNexusHandles.spaceAvailableEvent)
        BKNI_DestroyEvent(g_StandbyNexusHandles.spaceAvailableEvent);
    g_StandbyNexusHandles.spaceAvailableEvent = NULL;

    render_ui();
}
