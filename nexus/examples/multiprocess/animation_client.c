/***************************************************************************
 *  Broadcom Proprietary and Confidential. (c)2011-2016 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
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
 **************************************************************************/
#include "nexus_platform_client.h"
#include <stdio.h>

#if NEXUS_HAS_GRAPHICS2D
#include "nexus_graphics2d.h"
#include "nexus_surface.h"
#include "nexus_memory.h"
#include "nexus_base_mmap.h"
#include "nexus_surface_client.h"
#include "bstd.h"
#include "bkni.h"
#include "bkni_multi.h"
#include <stdlib.h>
#include <string.h>

BDBG_MODULE(animation_client);

static void complete(void *data, int unused)
{
    BSTD_UNUSED(unused);
    BKNI_SetEvent((BKNI_EventHandle)data);
}

static void checkpoint(NEXUS_Graphics2DHandle gfx, BKNI_EventHandle checkpointEvent)
{
    int rc;
    rc = NEXUS_Graphics2D_Checkpoint(gfx, NULL);
    if (rc == NEXUS_GRAPHICS2D_QUEUED) {
        BKNI_WaitForEvent(checkpointEvent, BKNI_INFINITE);
    }
}

#define NUM_SURFACES 50
struct {
    NEXUS_SurfaceHandle handle;
    bool submitted;
} g_surface[NUM_SURFACES];
unsigned submit_ptr, recycle_ptr;

static void recycle_next(NEXUS_SurfaceClientHandle blit_client);

int main(int argc, const char * const argv[])
{
    NEXUS_SurfaceCreateSettings createSettings;
    NEXUS_Graphics2DHandle gfx;
    NEXUS_Graphics2DSettings gfxSettings;
    NEXUS_Graphics2DFillSettings fillSettings;
    BKNI_EventHandle checkpointEvent, packetSpaceAvailableEvent, recycledEvent;
    int i;
    NEXUS_Error rc;
    NEXUS_SurfaceClientHandle blit_client;
    NEXUS_SurfaceClientSettings client_settings;
    int gfx_client_id = 1;
    int curarg = 1;
    
    while (argc > curarg) {
        if (!strcmp(argv[curarg], "-client") && argc>curarg+1) {
            gfx_client_id = atoi(argv[++curarg]);
        }
        curarg++;
    }
    
    rc = NEXUS_Platform_AuthenticatedJoin(NULL);
    if (rc) {
        printf("cannot join: %d\n", rc);
        return -1;
    }

    BKNI_CreateEvent(&checkpointEvent);
    BKNI_CreateEvent(&packetSpaceAvailableEvent);
    BKNI_CreateEvent(&recycledEvent);

    gfx = NEXUS_Graphics2D_Open(NEXUS_ANY_ID, NULL);
    NEXUS_Graphics2D_GetSettings(gfx, &gfxSettings);
    gfxSettings.checkpointCallback.callback = complete;
    gfxSettings.checkpointCallback.context = checkpointEvent;
    gfxSettings.packetSpaceAvailable.callback = complete;
    gfxSettings.packetSpaceAvailable.context = packetSpaceAvailableEvent;
    rc = NEXUS_Graphics2D_SetSettings(gfx, &gfxSettings);
    BDBG_ASSERT(!rc);

    NEXUS_Surface_GetDefaultCreateSettings(&createSettings);
    NEXUS_Graphics2D_GetDefaultFillSettings(&fillSettings);
    
    createSettings.pixelFormat = NEXUS_PixelFormat_eA8_R8_G8_B8;
    createSettings.width = 200;
    createSettings.height = 200;
    for (i=0;i<NUM_SURFACES;i++) {
        g_surface[i].handle = NEXUS_Surface_Create(&createSettings);
        
        /* green background */
        fillSettings.surface = g_surface[i].handle;
        fillSettings.rect.width = 0;
        fillSettings.rect.height = 0;
        fillSettings.color = 0xFF208020;
        rc = NEXUS_Graphics2D_Fill(gfx, &fillSettings);
        BDBG_ASSERT(!rc);
        
        /* gray white box */
        fillSettings.surface = g_surface[i].handle;
        fillSettings.rect.x = i*(createSettings.width - 20)/NUM_SURFACES;
        fillSettings.rect.y = 0;
        fillSettings.rect.width = 20;
        fillSettings.rect.height = createSettings.height;
        fillSettings.color = 0xFF333355;
        rc = NEXUS_Graphics2D_Fill(gfx, &fillSettings);
        BDBG_ASSERT(!rc);
    }
    checkpoint(gfx, checkpointEvent);

    blit_client = NEXUS_SurfaceClient_Acquire(gfx_client_id);
    if (!blit_client) {
        BDBG_ERR(("NEXUS_SurfaceClient_Acquire %d failed. run client with '-client X' to change ids.", gfx_client_id));
        return -1;
    }
    
    NEXUS_SurfaceClient_GetSettings(blit_client, &client_settings);
    client_settings.recycled.callback = complete;
    client_settings.recycled.context = recycledEvent;
    rc = NEXUS_SurfaceClient_SetSettings(blit_client, &client_settings);
    BDBG_ASSERT(!rc);

    /* push already-rendered surfaces in as fast as possible. allow display vsync to flow control. */
    for(i=0;;) {
        if (g_surface[submit_ptr].submitted) {
            recycle_next(blit_client);
            rc = BKNI_WaitForEvent(recycledEvent, BKNI_INFINITE);
            BDBG_ASSERT(!rc);
            continue;
        }
        
        NEXUS_SurfaceClient_PushSurface(blit_client, g_surface[submit_ptr].handle, NULL, false);
        BDBG_ASSERT(!rc);
        g_surface[submit_ptr].submitted = true;
        if (++submit_ptr == NUM_SURFACES) {
            submit_ptr = 0;
        }
        recycle_next(blit_client);
    }
    
    NEXUS_SurfaceClient_Clear(blit_client);
    for (i=0;i<NUM_SURFACES;i++) {
        NEXUS_Surface_Destroy(g_surface[i].handle);
    } 
    NEXUS_SurfaceClient_Release(blit_client);
    BKNI_DestroyEvent(recycledEvent);
    BKNI_DestroyEvent(checkpointEvent);
    BKNI_DestroyEvent(packetSpaceAvailableEvent);
    NEXUS_Graphics2D_Close(gfx);
    NEXUS_Platform_Uninit();

    return 0;
}

static void recycle_next(NEXUS_SurfaceClientHandle blit_client)
{
    size_t num;
    do {
        NEXUS_SurfaceHandle surface;
        NEXUS_Error rc;
        rc = NEXUS_SurfaceClient_RecycleSurface(blit_client, &surface, 1, &num);
        BDBG_ASSERT(!rc);
        if (num) {
            /* they should be recycled in the order submitted */
            BDBG_ASSERT(g_surface[recycle_ptr].handle == surface);
            g_surface[recycle_ptr].submitted = false;
            if (++recycle_ptr == NUM_SURFACES) {
                recycle_ptr = 0;
            }
        }
    } while (num);
}
#else /* NEXUS_HAS_GRAPHICS2D */
int main(void)
{
    printf("This application is not supported on this platform (needs graphics2d)!\n");
    return 0;
}
#endif
