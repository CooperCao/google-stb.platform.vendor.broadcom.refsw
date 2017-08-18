/***************************************************************************
 * Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 * Module Description:
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
#include "nxapps_cmdline.h"
#include "namevalue.h"

BDBG_MODULE(blit_client);

#define USE_PACKET_BLIT 1

static void complete(void *data, int unused)
{
    BSTD_UNUSED(unused);
    BKNI_SetEvent((BKNI_EventHandle)data);
}

static void print_usage(const struct nxapps_cmdline *cmdline)
{
    printf(
    "Usage: blit_client OPTIONS\n"
    "  --help or -h for help\n"
    "  -move\n"
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

static unsigned check_standby(void)
{
    NxClient_StandbyStatus standbyStatus;
    NEXUS_Error rc;

    rc = NxClient_GetStandbyStatus(&standbyStatus);
    if (rc) exit(0); /* server is down, exit gracefully */
    if(standbyStatus.transition == NxClient_StandbyTransition_eAckNeeded) {
        printf("'blit_client' acknowledges standby state: %s\n", lookup_name(g_platformStandbyModeStrs, standbyStatus.settings.mode));
        NxClient_AcknowledgeStandby(true);
    }
    return (standbyStatus.settings.mode != NEXUS_PlatformStandbyMode_eOn);
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
    NEXUS_Graphics2DSettings gfxSettings;
    NEXUS_Graphics2DFillSettings fillSettings;
    BKNI_EventHandle checkpointEvent, packetSpaceAvailableEvent, displayedEvent;
    NEXUS_SurfaceClientHandle blit_client;
    NEXUS_SurfaceClientSettings client_settings;
    NEXUS_Graphics2DBlitSettings blitSettings;
    NEXUS_Error rc;
    unsigned i;
    int curarg = 1;
    unsigned timeout = 0, starttime;
    unsigned divisor = 1;
    struct nxapps_cmdline cmdline;
    int n;
    struct { int x, y; } pig_inc = {0,0};
    NEXUS_SurfaceComposition comp;

    nxapps_cmdline_init(&cmdline);
    nxapps_cmdline_allow(&cmdline, nxapps_cmdline_type_SurfaceComposition);

    while (argc > curarg) {
        if (!strcmp(argv[curarg], "--help") || !strcmp(argv[curarg], "-h")) {
            print_usage(&cmdline);
            return 0;
        }
        else if (!strcmp(argv[curarg], "-timeout") && argc>curarg+1) {
            timeout = atoi(argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-move")) {
            pig_inc.x = pig_inc.y = 4;
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

    /* connect to server and nexus */
    NxClient_GetDefaultJoinSettings(&joinSettings);
    snprintf(joinSettings.name, NXCLIENT_MAX_NAME, "%s", argv[0]);
    rc = NxClient_Join(&joinSettings);
    if (rc) return -1;

    NEXUS_Platform_GetClientConfiguration(&clientConfig);
    BKNI_CreateEvent(&checkpointEvent);
    BKNI_CreateEvent(&packetSpaceAvailableEvent);
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
    rc = NEXUS_SurfaceClient_SetSettings(blit_client, &client_settings);
    BDBG_ASSERT(!rc);

    if (pig_inc.x && !cmdline.comp.rect.set) {
        const char *argv[] = {"-rect","0,0,400,300"};
        nxapps_cmdline_parse(0, 2, argv, &cmdline);
    }

    if (nxapps_cmdline_is_set(&cmdline, nxapps_cmdline_type_SurfaceComposition)) {
        NxClient_GetSurfaceClientComposition(allocResults.surfaceClient[0].id, &comp);
        nxapps_cmdline_apply_SurfaceComposition(&cmdline, &comp);
        NxClient_SetSurfaceClientComposition(allocResults.surfaceClient[0].id, &comp);
    }

    {
    NEXUS_Graphics2DOpenSettings openSettings;
    NEXUS_Graphics2D_GetDefaultOpenSettings(&openSettings);
    openSettings.packetFifoSize = 4*1024; /* minimal fifo */
    gfx = NEXUS_Graphics2D_Open(NEXUS_ANY_ID, &openSettings);
    }

    NEXUS_Graphics2D_GetSettings(gfx, &gfxSettings);
    gfxSettings.checkpointCallback.callback = complete;
    gfxSettings.checkpointCallback.context = checkpointEvent;
    gfxSettings.packetSpaceAvailable.callback = complete;
    gfxSettings.packetSpaceAvailable.context = packetSpaceAvailableEvent;
    rc = NEXUS_Graphics2D_SetSettings(gfx, &gfxSettings);
    BDBG_ASSERT(!rc);

    if (cmdline.comp.rect.position.width) {
        divisor = cmdline.comp.rect.virtualDisplay.width/cmdline.comp.rect.position.width;
        if (divisor == 0) divisor = 1;
        if (divisor > 4) divisor = 4;
    }

    NEXUS_Surface_GetDefaultCreateSettings(&createSettings);
    createSettings.pixelFormat = NEXUS_PixelFormat_eA8_R8_G8_B8;
    createSettings.width = 720/divisor;
    createSettings.height = 480/divisor;
    createSettings.heap = clientConfig.heap[NXCLIENT_SECONDARY_GRAPHICS_HEAP]; /* if NULL, will use NXCLIENT_DEFAULT_HEAP */
    surface = NEXUS_Surface_Create(&createSettings);

    /* draw gradient on left side of black framebuffer using synchronous blits */
#define SIDEBAR_WIDTH (100/divisor)
    NEXUS_Graphics2D_GetDefaultFillSettings(&fillSettings);
    fillSettings.surface = surface;
    fillSettings.rect.width = createSettings.width;
    fillSettings.rect.height = createSettings.height;
    fillSettings.color = 0;
    rc = NEXUS_Graphics2D_Fill(gfx, &fillSettings);
    BDBG_ASSERT(!rc);
    for (i=0;i<(int)createSettings.height;i+=2) {
        fillSettings.rect.y = i;
        fillSettings.rect.width = SIDEBAR_WIDTH;
        fillSettings.rect.height = 2;
        fillSettings.color =
            (0xFF << 24) |
            (((i*divisor/2) % 0xFF) << 16) |
            (((i*divisor) % 0xFF) << 8) |
            (((i*divisor*2) % 0xFF));
        while (1) {
            rc = NEXUS_Graphics2D_Fill(gfx, &fillSettings);
            if (rc == NEXUS_GRAPHICS2D_QUEUE_FULL) {
                BKNI_WaitForEvent(packetSpaceAvailableEvent, BKNI_INFINITE);
                /* retry */
            }
            else {
                break;
            }
        }
    }

    rc = NEXUS_Graphics2D_Checkpoint(gfx, NULL); /* require to execute queue */
    if (rc == NEXUS_GRAPHICS2D_QUEUED) {
        rc = BKNI_WaitForEvent(checkpointEvent, BKNI_INFINITE);
    }
    BDBG_ASSERT(!rc);

    rc = NEXUS_SurfaceClient_SetSurface(blit_client, surface);
    BDBG_ASSERT(!rc);
    rc = BKNI_WaitForEvent(displayedEvent, 5000);
    BDBG_ASSERT(!rc);

    /* blit from left-hand gradient into the rest of the framebuffer */
    BDBG_WRN(("starting"));
    starttime = b_get_time();
    NEXUS_Graphics2D_GetDefaultBlitSettings(&blitSettings); /* don't get defaults more often than necessary */
    for(i=0;;) {
#define BOX_SIZE (20/divisor)
#if USE_PACKET_BLIT
        NEXUS_Error rc;
        void *buffer, *next;
        size_t size;
        unsigned j;
        BM2MC_PACKET_Plane surfacePlane;
        static const BM2MC_PACKET_Blend copyColor = {BM2MC_PACKET_BlendFactor_eSourceColor, BM2MC_PACKET_BlendFactor_eOne, false,
            BM2MC_PACKET_BlendFactor_eZero, BM2MC_PACKET_BlendFactor_eZero, false, BM2MC_PACKET_BlendFactor_eZero};
        static const BM2MC_PACKET_Blend copyAlpha = {BM2MC_PACKET_BlendFactor_eSourceAlpha, BM2MC_PACKET_BlendFactor_eOne, false,
            BM2MC_PACKET_BlendFactor_eZero, BM2MC_PACKET_BlendFactor_eZero, false, BM2MC_PACKET_BlendFactor_eZero};

        if(check_standby()) {
            BKNI_Sleep(100);
            continue;
        }

        NEXUS_Surface_InitPlane(surface, &surfacePlane);

        rc = NEXUS_Graphics2D_GetPacketBuffer(gfx, &buffer, &size, 2048);
        BDBG_ASSERT(!rc);
        if (!size) {
            /* internal queue is full. wait for space to become available. */
            BKNI_WaitForEvent(packetSpaceAvailableEvent, BKNI_INFINITE);
            continue;
        }

        next = buffer;

        {
        BM2MC_PACKET_PacketSourceFeeder *pPacket = next;
        /* if your app crashes here, your NEXUS_Graphics2DOpenSettings.heap (or default heap) is likely
        not CPU-accessible by the application. */
        BM2MC_PACKET_INIT(pPacket, SourceFeeder, false );
        pPacket->plane = surfacePlane;
        pPacket->color = 0;
        next = ++pPacket;
        }

        {
        BM2MC_PACKET_PacketOutputFeeder *pPacket = next;
        BM2MC_PACKET_INIT(pPacket, OutputFeeder, false);
        pPacket->plane = surfacePlane;
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

        for (j=0;j<50;j++) {
            BM2MC_PACKET_PacketScaleBlit *pPacket = next;
            BM2MC_PACKET_INIT(pPacket, ScaleBlit, true);
            pPacket->src_rect.width = SIDEBAR_WIDTH;
            pPacket->src_rect.height = createSettings.height/4;
            pPacket->src_rect.x = 0;
            pPacket->src_rect.y = (i/300) % (createSettings.height-pPacket->src_rect.height);
            pPacket->out_rect.x = (rand() % (createSettings.width-SIDEBAR_WIDTH-BOX_SIZE)) + SIDEBAR_WIDTH;
            pPacket->out_rect.y = (rand() % (createSettings.height-BOX_SIZE));
            pPacket->out_rect.width = BOX_SIZE;
            pPacket->out_rect.height = BOX_SIZE;
            next = ++pPacket;

            i++;
        }

        rc = NEXUS_Graphics2D_PacketWriteComplete(gfx, (uint8_t*)next - (uint8_t*)buffer);
        BDBG_ASSERT(!rc);
#else
        unsigned j;
        if(check_standby()) {
            BKNI_Sleep(100);
            continue;
        }
        for (j=0;j<50;j++) {
            blitSettings.source.surface = surface;
            blitSettings.source.rect.width = SIDEBAR_WIDTH;
            blitSettings.source.rect.height = createSettings.height/4;
            blitSettings.source.rect.x = 0;
            blitSettings.source.rect.y = (i/300) % (createSettings.height-blitSettings.source.rect.height);

            blitSettings.output.surface = surface;
            blitSettings.output.rect.x = (rand() % (createSettings.width-SIDEBAR_WIDTH-BOX_SIZE)) + SIDEBAR_WIDTH;
            blitSettings.output.rect.y = (rand() % (createSettings.height-BOX_SIZE));
            blitSettings.output.rect.width = BOX_SIZE;
            blitSettings.output.rect.height = BOX_SIZE;

            blitSettings.colorOp = NEXUS_BlitColorOp_eCopySource;
            blitSettings.alphaOp = NEXUS_BlitAlphaOp_eCopySource;

            while (1) {
                rc = NEXUS_Graphics2D_Blit(gfx, &blitSettings);
                if (rc == NEXUS_GRAPHICS2D_QUEUE_FULL) {
                    /* blit can fail because an internal queue is full. wait for space to open up,
                    then resubmit the blit. */
                    BKNI_WaitForEvent(packetSpaceAvailableEvent, BKNI_INFINITE);
                    /* retry */
                }
                else {
                    break;
                }
            }
            i++;
        }
#endif

        /* must do checkpoint before UpdateSurface because server uses different blitter fifo. */
        rc = NEXUS_Graphics2D_Checkpoint(gfx, NULL);
        if (rc == NEXUS_GRAPHICS2D_QUEUED) {
            rc = BKNI_WaitForEvent(checkpointEvent, BKNI_INFINITE);
        }
        BDBG_ASSERT(!rc);

        /* tell server to blit */
        rc = NEXUS_SurfaceClient_UpdateSurface(blit_client, NULL);
        BDBG_ASSERT(!rc);
        rc = BKNI_WaitForEvent(displayedEvent, 5000);
        if (rc) BERR_TRACE(rc);
        if (i && i%50000==0) {
            BDBG_WRN(("%u blits completed", i));
        }
        /* no flush is needed because we're not using the CPU */

        if (pig_inc.x) {
            comp.position.x += pig_inc.x;
            comp.position.y += pig_inc.y;
            if (!comp.position.x || comp.position.x + comp.position.width >= 1920) pig_inc.x *= -1;
            if (!comp.position.y || comp.position.y + comp.position.height >= 1080) pig_inc.y *= -1;
            NxClient_SetSurfaceClientComposition(allocResults.surfaceClient[0].id, &comp);
        }

        if (timeout && b_get_time() - starttime >= timeout*1000) {
            break;
        }
    }

    NEXUS_SurfaceClient_Release(blit_client);
    BKNI_DestroyEvent(displayedEvent);
    BKNI_DestroyEvent(checkpointEvent);
    BKNI_DestroyEvent(packetSpaceAvailableEvent);
    NEXUS_Surface_Destroy(surface);
    NEXUS_Graphics2D_Close(gfx);
    NxClient_Free(&allocResults);
    NxClient_Uninit();
    return 0;
}
