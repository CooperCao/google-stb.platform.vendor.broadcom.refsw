/***************************************************************************
 *  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
 ***************************************************************************/

#include "bstd.h"
#include "bkni.h"
#include "bkni_multi.h"
#include "nexus_platform_client.h"
#include "nexus_surface_client.h"
#include "nexus_surface.h"
#include "nexus_graphics2d.h"
#include "nexus_core_utils.h"
#include "nxclient.h"
#include "nxapps_cmdline.h"
#include "binput.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

BDBG_MODULE(colorbar);

/* SMPTE color bars */
#define NUM_COLORS 7
static uint32_t g_colors[NUM_COLORS] = {
    0xFFFFFFFF, /* white */
    0xFFFFFF00, /* yellow */
    0xFF00FFFF, /* cyan */
    0xFF00FF00, /* green */
    0xFFFF00FF, /* magenta */
    0xFFFF0000, /* red */
    0xFF0000FF  /* blue */
};

static void complete(void *context, int param)
{
    BSTD_UNUSED(param);
    BKNI_SetEvent((BKNI_EventHandle)context);
}

static void print_usage(const struct nxapps_cmdline *cmdline)
{
    printf(
    "Usage: colorbar OPTIONS\n"
    "  --help or -h for help\n"
    );
    nxapps_cmdline_print_usage(cmdline);
}

static bool stopped = false;

static void process_input(b_remote_key key, bool repeat)
{
    /* only allow repeats for frame advance/reverse */
    switch (key) {
        case b_remote_key_stop:
        case b_remote_key_back:
        case b_remote_key_clear:
        {
            stopped = true;
            break;
        }
        /* fall through */
        default:
        {
            if (repeat) return;
        }
    }
}

int main(int argc, const char **argv)
{
    NxClient_JoinSettings joinSettings;
    NEXUS_ClientConfiguration clientConfig;
    NxClient_AllocSettings allocSettings;
    NxClient_AllocResults allocResults;
    NEXUS_SurfaceClientHandle colorbarClient;
    NEXUS_SurfaceClientSettings clientSettings;
    NEXUS_SurfaceHandle surface;
    NEXUS_SurfaceCreateSettings surfaceCreateSettings;
    NEXUS_Graphics2DHandle gfx;
    NEXUS_Graphics2DSettings gfxSettings;
    BKNI_EventHandle checkpointEvent, displayedEvent;
    NEXUS_Graphics2DFillSettings fillSettings;
    NEXUS_Error rc;
    int i, curarg =1;
    struct nxapps_cmdline cmdline;
    binput_t input;
    struct binput_settings inputSettings;

    nxapps_cmdline_init(&cmdline);
    nxapps_cmdline_allow(&cmdline, nxapps_cmdline_type_SurfaceComposition);

    while (argc > curarg) {
        if (!strcmp(argv[curarg], "--help") || !strcmp(argv[curarg], "-h")) {
            print_usage(&cmdline);
            return 0;
        }
        else if ((i = nxapps_cmdline_parse(curarg, argc, argv, &cmdline))) {
            if (i < 0) {
                print_usage(&cmdline);
                return -1;
            }
            curarg += i;
        }
        else {
            print_usage(&cmdline);
            return 1;
        }
        curarg++;
    }

    binput_get_default_settings(&inputSettings);

    /* connect to server and nexus */
    NxClient_GetDefaultJoinSettings(&joinSettings);
    joinSettings.ignoreStandbyRequest = true;
    snprintf(joinSettings.name, NXCLIENT_MAX_NAME, "%s", argv[0]);
    rc = NxClient_Join(&joinSettings);
    if (rc) return BERR_TRACE(rc);

    input = binput_open(&inputSettings);

    NEXUS_Platform_GetClientConfiguration(&clientConfig);

    NEXUS_Surface_GetDefaultCreateSettings(&surfaceCreateSettings);
    surfaceCreateSettings.pixelFormat = NEXUS_PixelFormat_eA8_R8_G8_B8;
    surfaceCreateSettings.width  = 1920;
    surfaceCreateSettings.height = 1080;
    surfaceCreateSettings.heap = clientConfig.heap[NXCLIENT_SECONDARY_GRAPHICS_HEAP];
    surface = NEXUS_Surface_Create(&surfaceCreateSettings);

    gfx = NEXUS_Graphics2D_Open(NEXUS_ANY_ID, NULL);
    BKNI_CreateEvent(&checkpointEvent);
    BKNI_CreateEvent(&displayedEvent);

    NEXUS_Graphics2D_GetSettings(gfx, &gfxSettings);
    gfxSettings.checkpointCallback.callback = complete;
    gfxSettings.checkpointCallback.context = checkpointEvent;
    rc = NEXUS_Graphics2D_SetSettings(gfx, &gfxSettings);
    BDBG_ASSERT(!rc);

    NEXUS_Platform_GetClientConfiguration(&clientConfig);

    NxClient_GetDefaultAllocSettings(&allocSettings);
    allocSettings.surfaceClient = 1;
    rc = NxClient_Alloc(&allocSettings, &allocResults);
    if (rc) return BERR_TRACE(rc);

    /* No NxClient_Connect needed for SurfaceClient */
    colorbarClient = NEXUS_SurfaceClient_Acquire(allocResults.surfaceClient[0].id);
    if (!colorbarClient) {
        BDBG_ERR(("NEXUS_SurfaceClient_Acquire failed"));
        return BERR_TRACE(rc);
    }

    NEXUS_SurfaceClient_GetSettings(colorbarClient, &clientSettings);
    clientSettings.displayed.callback = complete;
    clientSettings.displayed.context = displayedEvent;
    rc = NEXUS_SurfaceClient_SetSettings(colorbarClient, &clientSettings);
    BDBG_ASSERT(!rc);

    NEXUS_Graphics2D_GetDefaultFillSettings(&fillSettings);
    fillSettings.surface = surface;
    fillSettings.rect.width = surfaceCreateSettings.width/NUM_COLORS;
    fillSettings.rect.y = 0;
    fillSettings.rect.height = surfaceCreateSettings.height;

    for (i=0; i < NUM_COLORS; ++i) {
        fillSettings.rect.x = fillSettings.rect.width * i;
        fillSettings.color = g_colors[i];
        NEXUS_Graphics2D_Fill(gfx, &fillSettings);
    }

    /* Need to execute queued graphics operations */
    rc = NEXUS_Graphics2D_Checkpoint(gfx, NULL);
    if (rc == NEXUS_GRAPHICS2D_QUEUED) {
        rc = BKNI_WaitForEvent(checkpointEvent, BKNI_INFINITE);
    }
    BDBG_ASSERT(!rc);

    /* Request server to display surface */
    rc = NEXUS_SurfaceClient_SetSurface(colorbarClient, surface);
    BDBG_ASSERT(!rc);
    rc = BKNI_WaitForEvent(displayedEvent, 5000);
    BDBG_ASSERT(!rc);

    printf("\"Stop\" or \"Clear\" on the remote control exits the app.\n");
    while (!stopped)
    {
        b_remote_key key;
        bool repeat;
        if (!binput_read(input, &key, &repeat)) {
            process_input(key, repeat);
        }
        else {
            binput_wait(input, 100);
        }
    }

    binput_close(input);
    NEXUS_Graphics2D_Close(gfx);
    NEXUS_Surface_Destroy(surface);
    BKNI_DestroyEvent(checkpointEvent);
    BKNI_DestroyEvent(displayedEvent);
    NxClient_Uninit();

    return 0;
}
