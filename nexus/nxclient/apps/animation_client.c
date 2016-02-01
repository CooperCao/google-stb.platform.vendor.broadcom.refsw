/***************************************************************************
 *     (c)2011-2013 Broadcom Corporation
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
 **************************************************************************/
#include "nxclient.h"
#include "nexus_platform_client.h"
#include "nexus_platform.h"
#include "nexus_surface.h"
#include "nexus_graphics2d.h"
#include "nexus_memory.h"
#include "nexus_base_mmap.h"
#include "nexus_surface_client.h"
#include "bstd.h"
#include "bkni.h"
#include "bkni_multi.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "nxapps_cmdline.h"

BDBG_MODULE(animation_client);

static void complete(void *data, int unused)
{
    BSTD_UNUSED(unused);
    BKNI_SetEvent((BKNI_EventHandle)data);
}

#define MAX_SURFACES 20
struct {
    NEXUS_SurfaceCreateSettings surfaceCreateSettings;
    NEXUS_Graphics2DFillSettings fillSettings;
    struct {
        NEXUS_SurfaceHandle handle;
        bool submitted;
    } surface[MAX_SURFACES];
    unsigned submit_ptr, depth, numsurfaces;
    NEXUS_Graphics2DHandle gfx;
    BKNI_EventHandle checkpointEvent;
    uint32_t color[2];
} g_queue;

static void recycle_next(NEXUS_SurfaceClientHandle blit_client);

static void print_usage(const struct nxapps_cmdline *cmdline)
{
    printf(
    "Usage: animation_client OPTIONS\n"
    "  --help or -h for help\n"
    "  -max_depth #             max number of surfaces to push at a time\n"
    "  -infront                 push frames faster than the vsync\n"
    "  -q                       don't print status\n"
    "  -virtualRefreshRate X    refresh rate rate in units of 1/1000th Hz\n  "
    "  -n X                     number of surfaces (default 4)\n"
    "  -move\n"
    "  -bypass                  set allowCompositionBypass for this client\n"
    );
    nxapps_cmdline_print_usage(cmdline);
}

#include <sys/time.h>
static unsigned b_get_time(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec*1000 + tv.tv_usec/1000;
}

#define SURFACE_WIDTH 360
#define SURFACE_HEIGHT 240

static void draw_surface(unsigned total_pushes, NEXUS_SurfaceHandle surface)
{
    int rc;
    unsigned x;

    /* Animate based on SURFACE_WIDTH, then scale to g_queue.surfaceCreateSettings.width.
    This makes it go the same speed, regardless of size. */
    x = total_pushes % (SURFACE_WIDTH*2);
    if (x >= SURFACE_WIDTH) {
        x = SURFACE_WIDTH - (x - SURFACE_WIDTH);
    }
    x = x * g_queue.surfaceCreateSettings.width / SURFACE_WIDTH;

    if (!g_queue.color[0] || x == 0) {
        g_queue.color[0] = rand() | 0xFF000000;
    }
    if (!g_queue.color[1] || x == g_queue.surfaceCreateSettings.width) {
        g_queue.color[1] = rand() | 0xFF000000;
    }

    g_queue.fillSettings.surface = surface;
    g_queue.fillSettings.rect.y = 0;
    g_queue.fillSettings.rect.height = g_queue.surfaceCreateSettings.height;

    if (x) {
        g_queue.fillSettings.rect.x = 0;
        g_queue.fillSettings.rect.width = x;
        g_queue.fillSettings.color = g_queue.color[0];
        rc = NEXUS_Graphics2D_Fill(g_queue.gfx, &g_queue.fillSettings);
        BDBG_ASSERT(!rc);
    }
    if (x < g_queue.surfaceCreateSettings.width) {
        g_queue.fillSettings.rect.x = x;
        g_queue.fillSettings.rect.width = g_queue.surfaceCreateSettings.width - x;
        g_queue.fillSettings.color = g_queue.color[1];
        rc = NEXUS_Graphics2D_Fill(g_queue.gfx, &g_queue.fillSettings);
        BDBG_ASSERT(!rc);
    }
    rc = NEXUS_Graphics2D_Checkpoint(g_queue.gfx, NULL);
    if (rc == NEXUS_GRAPHICS2D_QUEUED) {
        rc = BKNI_WaitForEvent(g_queue.checkpointEvent, 3000);
        BDBG_ASSERT(!rc);
    }
}

int main(int argc, const char **argv)
{
    NxClient_JoinSettings joinSettings;
    NEXUS_ClientConfiguration clientConfig;
    NxClient_AllocSettings allocSettings;
    NxClient_AllocResults allocResults;
    unsigned connectId = 0;
    NEXUS_Graphics2DSettings gfxSettings;
    NEXUS_Graphics2DOpenSettings openSettings;
    BKNI_EventHandle packetSpaceAvailableEvent, recycledEvent;
    NEXUS_SurfaceClientHandle blit_client;
    NEXUS_SurfaceClientSettings client_settings;
    NEXUS_Error rc;
    unsigned i;
    int curarg = 1;
    unsigned max_depth = 3;
    bool infront = false;
    bool quiet = false;
    unsigned total_pushes = 0;
    unsigned virtualRefreshRate = 0;
    unsigned timeout = 0, starttime;
    struct nxapps_cmdline cmdline;
    int n;
    struct { unsigned cnt, time; } fps = {0,0};
    struct { int x, y; } pig_inc = {0,0};
    NEXUS_SurfaceComposition comp;
    bool bypass = false;
    
    srand(time(NULL));
    nxapps_cmdline_init(&cmdline);
    nxapps_cmdline_allow(&cmdline, nxapps_cmdline_type_SurfaceComposition);
    g_queue.numsurfaces = 4;

    while (argc > curarg) {
        if (!strcmp(argv[curarg], "--help") || !strcmp(argv[curarg], "-h")) {
            print_usage(&cmdline);
            return 0;
        }
        else if (!strcmp(argv[curarg], "-max_depth") && argc>curarg+1) {
            max_depth = atoi(argv[++curarg]);
            if (max_depth < 2) {
                BDBG_WRN(("max_depth must be at least 2 for push/recycle to work"));
                max_depth = 2;
            }
        }
        else if (!strcmp(argv[curarg], "-virtualRefreshRate") && argc>curarg+1) {
            virtualRefreshRate = atoi(argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-infront")) {
            infront = true;
        }
        else if (!strcmp(argv[curarg], "-q")) {
            quiet = true;
        }
        else if (!strcmp(argv[curarg], "-timeout") && argc>curarg+1) {
            timeout = atoi(argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-move")) {
            pig_inc.x = pig_inc.y = 4;
        }
        else if (!strcmp(argv[curarg], "-bypass")) {
            bypass = true;
        }
        else if (!strcmp(argv[curarg], "-n") && argc>curarg+1) {
            g_queue.numsurfaces = atoi(argv[++curarg]);
            if (g_queue.numsurfaces > MAX_SURFACES) {
                BDBG_ERR(("num surfaces cannot be more than %d", MAX_SURFACES));
                return -1;
            }
            else if (g_queue.numsurfaces < 2) {
                BDBG_ERR(("num surfaces must be at least 2"));
                return -1;
            }
        }
        else if ((n = nxapps_cmdline_parse(curarg, argc, argv, &cmdline))) {
            if (n < 0) {
                print_usage(&cmdline);
                return -1;
            }
            curarg += n;
        }
        else {
            print_usage(&cmdline);
            return 1;
        }
        curarg++;
    }

    NxClient_GetDefaultJoinSettings(&joinSettings);
    snprintf(joinSettings.name, NXCLIENT_MAX_NAME, "%s", argv[0]);
    rc = NxClient_Join(&joinSettings);
    if (rc) return -1;

    NEXUS_Platform_GetClientConfiguration(&clientConfig);
    BKNI_CreateEvent(&g_queue.checkpointEvent);
    BKNI_CreateEvent(&packetSpaceAvailableEvent);
    BKNI_CreateEvent(&recycledEvent);

    NxClient_GetDefaultAllocSettings(&allocSettings);
    allocSettings.surfaceClient = 1;
    rc = NxClient_Alloc(&allocSettings, &allocResults);
    if (rc) return BERR_TRACE(rc);
    
    /* No NxClient_Connect needed for SurfaceClient */
    
    blit_client = NEXUS_SurfaceClient_Acquire(allocResults.surfaceClient[0].id);
    if (!blit_client) {
        BDBG_ERR(("NEXUS_SurfaceClient_Acquire failed"));
        return -1;
    }
    
    NEXUS_SurfaceClient_GetSettings(blit_client, &client_settings);
    client_settings.recycled.callback = complete;
    client_settings.recycled.context = recycledEvent;
    client_settings.virtualRefreshRate = virtualRefreshRate;
    client_settings.allowCompositionBypass = bypass;
    rc = NEXUS_SurfaceClient_SetSettings(blit_client, &client_settings);
    BDBG_ASSERT(!rc);


    if (pig_inc.x && !nxapps_cmdline_is_set(&cmdline, nxapps_cmdline_type_SurfaceComposition)) {
        const char *argv[] = {"-rect","0,0,400,300"};
        nxapps_cmdline_parse(0, 2, argv, &cmdline);
    }

    if (nxapps_cmdline_is_set(&cmdline, nxapps_cmdline_type_SurfaceComposition)) {
        NxClient_GetSurfaceClientComposition(allocResults.surfaceClient[0].id, &comp);
        nxapps_cmdline_apply_SurfaceComposition(&cmdline, &comp);
        NxClient_SetSurfaceClientComposition(allocResults.surfaceClient[0].id, &comp);
    }

    NEXUS_Graphics2D_GetDefaultOpenSettings(&openSettings);
    openSettings.packetFifoSize = 4*1024; /* minimal fifo */
    g_queue.gfx = NEXUS_Graphics2D_Open(NEXUS_ANY_ID, &openSettings);

    NEXUS_Graphics2D_GetDefaultFillSettings(&g_queue.fillSettings);
    
    NEXUS_Graphics2D_GetSettings(g_queue.gfx, &gfxSettings);
    gfxSettings.checkpointCallback.callback = complete;
    gfxSettings.checkpointCallback.context = g_queue.checkpointEvent;
    gfxSettings.packetSpaceAvailable.callback = complete;
    gfxSettings.packetSpaceAvailable.context = packetSpaceAvailableEvent;
    rc = NEXUS_Graphics2D_SetSettings(g_queue.gfx, &gfxSettings);
    BDBG_ASSERT(!rc);

    NEXUS_Surface_GetDefaultCreateSettings(&g_queue.surfaceCreateSettings);
    
    g_queue.surfaceCreateSettings.pixelFormat = NEXUS_PixelFormat_eR5_G6_B5;
    if (bypass) {
        NEXUS_SurfaceClientStatus status;
        NEXUS_SurfaceClient_GetStatus(blit_client, &status);
        g_queue.surfaceCreateSettings.width = status.display.framebuffer.width;
        g_queue.surfaceCreateSettings.height = status.display.framebuffer.height;
        g_queue.surfaceCreateSettings.heap = NEXUS_Platform_GetFramebufferHeap(0);
    }
    else {
        g_queue.surfaceCreateSettings.width = SURFACE_WIDTH;
        g_queue.surfaceCreateSettings.height = SURFACE_HEIGHT;
        g_queue.surfaceCreateSettings.heap = clientConfig.heap[NXCLIENT_SECONDARY_GRAPHICS_HEAP]; /* if NULL, will use NXCLIENT_DEFAULT_HEAP */
    }
    for (i=0;i<g_queue.numsurfaces;i++) {
        g_queue.surface[i].handle = NEXUS_Surface_Create(&g_queue.surfaceCreateSettings);
    }

    /* push already-rendered surfaces in as fast as possible. allow display vsync to flow control. */
    fps.time = starttime = b_get_time();
    while (1) {
        if (g_queue.surface[g_queue.submit_ptr].submitted || g_queue.depth >= max_depth) {
            recycle_next(blit_client);
            if (g_queue.surface[g_queue.submit_ptr].submitted || g_queue.depth >= max_depth) {
                rc = BKNI_WaitForEvent(recycledEvent, BKNI_INFINITE);
                BDBG_ASSERT(!rc);
                continue;
            }
        }
        
        draw_surface(total_pushes, g_queue.surface[g_queue.submit_ptr].handle);

        NEXUS_SurfaceClient_PushSurface(blit_client, g_queue.surface[g_queue.submit_ptr].handle, NULL, infront);
        BDBG_ASSERT(!rc);
        g_queue.surface[g_queue.submit_ptr].submitted = true;
        if (++g_queue.submit_ptr == g_queue.numsurfaces) {
            g_queue.submit_ptr = 0;
        }
        total_pushes++;
        g_queue.depth++;
        fps.cnt++;
        if (!quiet && total_pushes % (infront?1000:100) == 0) {
            unsigned now = b_get_time();
            BDBG_WRN(("%u pushed, %u queued, %u fps", total_pushes, g_queue.depth, 1000*fps.cnt/(now-fps.time)));
            fps.cnt = 0;
            fps.time = now;
        }
        recycle_next(blit_client);

        if (timeout && b_get_time() - starttime >= timeout*1000) {
            break;
        }

        if (pig_inc.x) {
            comp.position.x += pig_inc.x;
            comp.position.y += pig_inc.y;
            if (!comp.position.x || comp.position.x + comp.position.width == 1920) pig_inc.x *= -1;
            if (!comp.position.y || comp.position.y + comp.position.height == 1080) pig_inc.y *= -1;
            NxClient_SetSurfaceClientComposition(allocResults.surfaceClient[0].id, &comp);
        }
    }
    
    NEXUS_SurfaceClient_Clear(blit_client);
    for (i=0;i<g_queue.numsurfaces;i++) {
        NEXUS_Surface_Destroy(g_queue.surface[i].handle);
    } 
    NEXUS_SurfaceClient_Release(blit_client);
    BKNI_DestroyEvent(recycledEvent);
    BKNI_DestroyEvent(g_queue.checkpointEvent);
    BKNI_DestroyEvent(packetSpaceAvailableEvent);
    NEXUS_Graphics2D_Close(g_queue.gfx);
    if (connectId) {
        NxClient_Disconnect(connectId);
    }
    NxClient_Free(&allocResults);
    NxClient_Uninit();

    return 0;
}

static void recycle_next(NEXUS_SurfaceClientHandle blit_client)
{
    unsigned num;
    do {
#define MAX_RECYCLE 10
        NEXUS_SurfaceHandle surface[MAX_RECYCLE];
        NEXUS_Error rc;
        unsigned i;
        
        rc = NEXUS_SurfaceClient_RecycleSurface(blit_client, surface, MAX_RECYCLE, &num);
        BDBG_ASSERT(!rc);
        for (i=0;i<num;i++) {
            unsigned j;
            /* if submitted infront, they may return out of order */
            for (j=0;j<g_queue.numsurfaces;j++) {
                if (g_queue.surface[j].handle == surface[i]) {
                    BDBG_ASSERT(g_queue.surface[j].submitted);
                    g_queue.surface[j].submitted = false;
                    g_queue.depth--;
                }
            }
        }
    } while (num >= MAX_RECYCLE);
}
