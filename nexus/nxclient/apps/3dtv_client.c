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
#include "nexus_surface.h"
#include "nexus_graphics2d.h"
#include "nexus_surface_client.h"
#include "bstd.h"
#include "bkni.h"
#include "bkni_multi.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "bfont.h"

BDBG_MODULE(3dtv_client);

static void complete(void *data, int unused)
{
    BSTD_UNUSED(unused);
    BKNI_SetEvent((BKNI_EventHandle)data);
}

static void print_usage(void)
{
    printf(
    "Usage: 3dtv_client OPTIONS\n"
    "  --help or -h for help\n"
    "  -2d              Draw a 3D surface, but submit as 2D. You will set both left and right at the same time.\n"
    );
}

int main(int argc, const char **argv)
{
    NxClient_JoinSettings joinSettings;
    NEXUS_ClientConfiguration clientConfig;
    NxClient_AllocSettings allocSettings;
    NxClient_AllocResults allocResults;
    NEXUS_SurfaceHandle surface;
    NEXUS_SurfaceCreateSettings createSettings;
    NEXUS_Graphics2DHandle gfx;
    NEXUS_Graphics2DOpenSettings openSettings;
    NEXUS_Graphics2DSettings gfxSettings;
    NEXUS_Graphics2DFillSettings fillSettings;
    BKNI_EventHandle checkpointEvent, displayedEvent;
    NEXUS_SurfaceClientHandle blit_client;
    NEXUS_SurfaceClientSettings client_settings;
    NEXUS_Error rc;
    int curarg = 1;
    unsigned timeout = 0;
    NEXUS_VideoOrientation orientation = NEXUS_VideoOrientation_e3D_LeftRight; /* The default is the client will submit a 3D surface. */
    bfont_t font;
    const char *fontname = "nxclient/arial_18_aa.bwin_font";

    while (argc > curarg) {
        if (!strcmp(argv[curarg], "--help") || !strcmp(argv[curarg], "-h")) {
            print_usage();
            return 0;
        }
        else if (!strcmp(argv[curarg], "-timeout") && argc>curarg+1) {
            timeout = atoi(argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-2d")) {
            orientation = NEXUS_VideoOrientation_e2D;
        }
        else {
            print_usage();
            return 1;
        }
        curarg++;
    }

    /* connect to server and nexus */
    NxClient_GetDefaultJoinSettings(&joinSettings);
    snprintf(joinSettings.name, NXCLIENT_MAX_NAME, "%s", argv[0]);
    rc = NxClient_Join(&joinSettings);
    if (rc) return -1;

    font = bfont_open(fontname);
    if (!font) {
        BDBG_WRN(("unable to load font %s", fontname));
    }

    NEXUS_Platform_GetClientConfiguration(&clientConfig);
    BKNI_CreateEvent(&checkpointEvent);
    BKNI_CreateEvent(&displayedEvent);

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
    client_settings.displayed.callback = complete;
    client_settings.displayed.context = displayedEvent;
    client_settings.orientation = orientation;
    rc = NEXUS_SurfaceClient_SetSettings(blit_client, &client_settings);
    BDBG_ASSERT(!rc);

    NEXUS_Graphics2D_GetDefaultOpenSettings(&openSettings);
    openSettings.packetFifoSize = 4*1024; /* minimal fifo */
    gfx = NEXUS_Graphics2D_Open(NEXUS_ANY_ID, &openSettings);

    NEXUS_Graphics2D_GetSettings(gfx, &gfxSettings);
    gfxSettings.checkpointCallback.callback = complete;
    gfxSettings.checkpointCallback.context = checkpointEvent;
    rc = NEXUS_Graphics2D_SetSettings(gfx, &gfxSettings);
    BDBG_ASSERT(!rc);

    NEXUS_Surface_GetDefaultCreateSettings(&createSettings);
    createSettings.pixelFormat = NEXUS_PixelFormat_eA8_R8_G8_B8;
    createSettings.width = 1280*2;
    createSettings.height = 720;
    createSettings.heap = clientConfig.heap[NXCLIENT_SECONDARY_GRAPHICS_HEAP]; /* if NULL, will use NXCLIENT_DEFAULT_HEAP */
    surface = NEXUS_Surface_Create(&createSettings);

    NEXUS_Graphics2D_GetDefaultFillSettings(&fillSettings);
    {
        unsigned x, y;
#define NUM_BLOCKS_WIDE 8
#define NUM_BLOCKS_HIGH 6
#define BLOCK_WIDTH 40
        unsigned xborder = (createSettings.width/2-(NUM_BLOCKS_WIDE*BLOCK_WIDTH*2-BLOCK_WIDTH))/2;
        unsigned yborder = (createSettings.height-(NUM_BLOCKS_HIGH*BLOCK_WIDTH*2-BLOCK_WIDTH))/2;

        BDBG_MSG(("x,y border %d,%d", xborder, yborder));

        fillSettings.rect.x =
        fillSettings.rect.y =
        fillSettings.rect.width =
        fillSettings.rect.height = 0;
        fillSettings.surface = surface;
        fillSettings.color = 0;
        NEXUS_Graphics2D_Fill(gfx, &fillSettings);

        fillSettings.rect.width = BLOCK_WIDTH;
        fillSettings.rect.height = BLOCK_WIDTH;
        fillSettings.surface = surface;

        for (x=0;x<NUM_BLOCKS_WIDE;x++) {
            for (y=0;y<NUM_BLOCKS_HIGH;y++) {
                fillSettings.rect.x = xborder+x*BLOCK_WIDTH*2;
                fillSettings.rect.y = yborder+y*BLOCK_WIDTH*2;
                fillSettings.color = 0xFF0033DD;
                NEXUS_Graphics2D_Fill(gfx, &fillSettings);
                fillSettings.rect.x += createSettings.width/2;
                NEXUS_Graphics2D_Fill(gfx, &fillSettings);
            }
        }

        /* must do checkpoint before UpdateSurface because server uses different blitter fifo. */
        rc = NEXUS_Graphics2D_Checkpoint(gfx, NULL);
        if (rc == NEXUS_GRAPHICS2D_QUEUED) {
            rc = BKNI_WaitForEvent(checkpointEvent, BKNI_INFINITE);
        }
        BDBG_ASSERT(!rc);

        if (font) {
            struct bfont_surface_desc desc;
            NEXUS_Rect rect = {0,0,0,0};
            int znorm = 5;
            const char *text = "Stereoscopic Graphics";

            rect.width = createSettings.width/2;
            rect.height = createSettings.height;
            bfont_get_surface_desc(surface, &desc);
            rect.x -= znorm;
            bfont_draw_aligned_text(&desc, font, &rect, text, -1, 0xFFCCCCCC, bfont_valign_center, bfont_halign_center);
            rect.x += rect.width + znorm*2;
            bfont_draw_aligned_text(&desc, font, &rect, text, -1, 0xFFCCCCCC, bfont_valign_center, bfont_halign_center);
            NEXUS_Surface_Flush(surface);
        }

        /* tell server to blit */
        rc = NEXUS_SurfaceClient_SetSurface(blit_client, surface);
        BDBG_ASSERT(!rc);
        rc = BKNI_WaitForEvent(displayedEvent, 5000);
        BDBG_ASSERT(!rc);
    }

    if (timeout) {
        BKNI_Sleep(timeout*1000);
    }
    else {
        while (1) BKNI_Sleep(1000);
    }

    NEXUS_SurfaceClient_Release(blit_client);
    BKNI_DestroyEvent(displayedEvent);
    BKNI_DestroyEvent(checkpointEvent);
    NEXUS_Surface_Destroy(surface);
    NEXUS_Graphics2D_Close(gfx);
    NxClient_Free(&allocResults);
    NxClient_Uninit();
    return 0;
}
