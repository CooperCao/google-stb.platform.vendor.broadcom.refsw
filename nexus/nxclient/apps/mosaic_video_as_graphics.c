/******************************************************************************
 * Copyright (C) 2018 Broadcom.
 * The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to
 * the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied),
 * right to use, or waiver of any kind with respect to the Software, and
 * Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 * THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 * IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use all
 * reasonable efforts to protect the confidentiality thereof, and to use this
 * information only in connection with your use of Broadcom integrated circuit
 * products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 * OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 * RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 * IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 * A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 * ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 * THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 * OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 * INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 * RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 * HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 * EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 * WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 * FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *****************************************************************************/
#if NEXUS_HAS_PLAYBACK && NEXUS_HAS_SIMPLE_DECODER
#include "nexus_platform_client.h"
#include "media_player.h"
#include "nexus_core_utils.h"
#include "nxclient.h"
#include "nexus_surface_client.h"
#include "nexus_graphics2d.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

BDBG_MODULE(mosaic_video_as_graphics);

#define MAX_MOSAICS 8

static void print_usage(void)
{
    printf(
        "Usage: mosaic_video_as_graphics OPTIONS stream1 [stream2 ...]\n"
        "\n"
        "If # streams < # mosaics, the list will be cycled for subsequent programs\n"
        "If # of mosaics available is less than requested, decode will proceed\n"
        "\n"
        "OPTIONS:\n"
        "  --help or -h for help\n"
        "  -n NUM_MOSAICS           default and max is %d\n"
        "  -max WIDTH,HEIGHT        default is 352,288 (CIF)\n",
        MAX_MOSAICS
        );
    printf(
        "  -rect x,y,width,height   position in default 1920x1080 coordinates\n"
        "  -zorder #\n"
        "  -pip                     sets -rect and -zorder for picture-in-picture\n"
        );
    printf(
        "  -downscale factor\n"
        "  -flip                 flip video vertically (proof that it's video as graphics)\n"
        "  -timeout MSEC         exit app after the specified number of vsyncs\n"
        "  -q                    don't print status\n"
        "  -cycle X              randomly stop and restart mosaics every X vsyncs\n"
        );
}

static void gfx_checkpoint(void *data, int unused)
{
    BSTD_UNUSED(unused);
    BKNI_SetEvent((BKNI_EventHandle)data);
}

int main(int argc, const char **argv)
{
    NxClient_JoinSettings joinSettings;
    NEXUS_Error rc;
    unsigned i;
    media_player_t player[MAX_MOSAICS];
    struct {
        NEXUS_SimpleVideoDecoderHandle handle;
        NEXUS_SurfaceHandle videoSurfaces[NEXUS_SIMPLE_DECODER_MAX_SURFACES];
        unsigned total;
        media_player_start_settings start_settings;
        bool started;
    } mosaic[MAX_MOSAICS];
    media_player_create_settings create_settings;
    const char *filename[MAX_MOSAICS];
    NxClient_AllocSettings allocSettings;
    NxClient_AllocResults allocResults;
    NEXUS_SurfaceClientHandle surfaceClient;
    BKNI_EventHandle displayedEvent, checkpointEvent;
    NEXUS_SurfaceHandle surface;
    NEXUS_SurfaceCreateSettings createSettings;
    NEXUS_Graphics2DHandle gfx;
    NEXUS_Graphics2DSettings gfxSettings;
    NEXUS_Graphics2DFillSettings fillSettings;
    NEXUS_Graphics2DBlitSettings blitSettings;
    NEXUS_SurfaceClientSettings clientSettings;
    NEXUS_SimpleVideoDecoderStartCaptureSettings captureSettings;
    unsigned downscale = 4;
    int cur_file = 0;
    int programindex = 0;
    unsigned maxWidth = 0;
    unsigned maxHeight = 0;
    int curarg = 1;
    unsigned num_mosaics = MAX_MOSAICS;
    unsigned num_started = 0;
    unsigned cycle = 0;
    unsigned num_tiles;
    bool flip = false;
    unsigned timeout = 0;
    bool quiet = false;
    NEXUS_Rect rect = {0,0,0,0};
    unsigned zorder = 0;

    memset(filename, 0, sizeof(filename));
    memset(mosaic, 0, sizeof(mosaic));

    while (curarg < argc) {
        if (!strcmp(argv[curarg], "-h") || !strcmp(argv[curarg], "--help")) {
            print_usage();
            return 0;
        }
        else if (!strcmp(argv[curarg], "-n") && curarg+1 < argc) {
            num_mosaics = atoi(argv[++curarg]);
            if (num_mosaics > MAX_MOSAICS) {
                print_usage();
                return -1;
            }
        }
        else if (!strcmp(argv[curarg], "-max") && curarg+1 < argc) {
            int n = sscanf(argv[++curarg], "%u,%u", &maxWidth, &maxHeight);
            if (n != 2) {
                print_usage();
                return -1;
            }
        }
        else if (!strcmp(argv[curarg], "-flip")) {
            flip = true;
        }
        else if (!strcmp(argv[curarg], "-downscale") && argc>curarg+1) {
            sscanf(argv[++curarg], "%u", &downscale);
            if (downscale==0) {
                downscale = 1;
            }
        }
        else if (!strcmp(argv[curarg], "-timeout") && argc>curarg+1) {
            timeout = atoi(argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-q")) {
            quiet = true;
        }
        else if (!strcmp(argv[curarg], "-cycle") && argc>curarg+1) {
            cycle = atoi(argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-rect") && argc>curarg+1) {
            unsigned x, y, width, height;
            if (sscanf(argv[++curarg], "%u,%u,%u,%u", &x,&y,&width,&height) == 4) {
                rect.x = x;
                rect.y = y;
                rect.width = width;
                rect.height = height;
            }
        }
        else if (!strcmp(argv[curarg], "-zorder") && argc>curarg+1) {
            zorder = atoi(argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-pip")) {
            rect.x = 1920/2;
            rect.y = 0;
            rect.width = 1920/2;
            rect.height = 1080/2;
            zorder = 1;
            /* for video as graphics, "pip" is purely a graphics rendering feature */
        }
        else if (cur_file < MAX_MOSAICS) {
            filename[cur_file] = argv[curarg];
            cur_file++;
        }
        else {
            print_usage();
            return -1;
        }
        curarg++;
    }

    if (!filename[0]) {
        print_usage();
        return -1;
    }
    
    NxClient_GetDefaultJoinSettings(&joinSettings);
    snprintf(joinSettings.name, NXCLIENT_MAX_NAME, "%s %s", argv[0], filename[0]);
    rc = NxClient_Join(&joinSettings);
    if (rc) return -1;
    
    BKNI_CreateEvent(&displayedEvent);
    BKNI_CreateEvent(&checkpointEvent);

    NxClient_GetDefaultAllocSettings(&allocSettings);
    allocSettings.surfaceClient = 1; /* surface client required for video windows */
    rc = NxClient_Alloc(&allocSettings, &allocResults);
    if (rc) return BERR_TRACE(rc);

    surfaceClient = NEXUS_SurfaceClient_Acquire(allocResults.surfaceClient[0].id);
    if (!surfaceClient) return BERR_TRACE(-1);
    
    if (rect.width || zorder) {
        NEXUS_SurfaceComposition comp;
        NxClient_GetSurfaceClientComposition(allocResults.surfaceClient[0].id, &comp);
        if (rect.width) {
            comp.position = rect;
        }
        comp.zorder = zorder;
        NxClient_SetSurfaceClientComposition(allocResults.surfaceClient[0].id, &comp);
    }

    media_player_get_default_create_settings(&create_settings);
    create_settings.window.surfaceClientId = allocResults.surfaceClient[0].id;
    if (maxWidth && maxHeight) {
        create_settings.maxWidth = maxWidth;
        create_settings.maxHeight = maxHeight;
    }
    rc = media_player_create_mosaics(player, num_mosaics, &create_settings);
    if (rc) return -1;

    cur_file = 0;
    for (i=0;i<num_mosaics;i++) {
        if (!player[i]) continue;
        media_player_get_default_start_settings(&mosaic[i].start_settings);
        mosaic[i].start_settings.stream_url = filename[cur_file];
        mosaic[i].start_settings.program = programindex;
        mosaic[i].start_settings.videoWindowType = NxClient_VideoWindowType_eNone;
        mosaic[i].start_settings.quiet = quiet;
        mosaic[i].start_settings.stcTrick = false; /* doesn't work for shared STC */
        rc = media_player_start(player[i], &mosaic[i].start_settings);
        if (rc) {
            if (programindex) {
                /* try again with program 0 */
                programindex = 0;
                mosaic[i].start_settings.program = programindex;
                rc = media_player_start(player[i], &mosaic[i].start_settings);
            }
            if (rc) {
                BDBG_WRN(("unable to start %s", mosaic[i].start_settings.stream_url));
            }
            /* keep going */
        }
        if (!rc) {
            mosaic[i].started = true;
            num_started++;
        }

        if (!filename[++cur_file]) {
            cur_file = 0;
            ++programindex; /* next program for all files */
        }
    }
    if (!num_started) return -1;

    NEXUS_Surface_GetDefaultCreateSettings(&createSettings);
    createSettings.pixelFormat = NEXUS_PixelFormat_eA8_R8_G8_B8;
    createSettings.width = 720/downscale;
    createSettings.height = 480/downscale;
    for (i=0;i<num_mosaics;i++) {
        unsigned j;
        if (!player[i]) continue;
        for (j=0; j<NEXUS_SIMPLE_DECODER_MAX_SURFACES; j++) {
            mosaic[i].videoSurfaces[j] = NEXUS_Surface_Create(&createSettings);
        }
    }
    createSettings.width = 720;
    createSettings.height = 480;
    surface = NEXUS_Surface_Create(&createSettings);
  
    gfx = NEXUS_Graphics2D_Open(NEXUS_ANY_ID, NULL);
    NEXUS_Graphics2D_GetSettings(gfx, &gfxSettings);
    gfxSettings.checkpointCallback.callback = gfx_checkpoint;
    gfxSettings.checkpointCallback.context = checkpointEvent;
    rc = NEXUS_Graphics2D_SetSettings(gfx, &gfxSettings);
    BDBG_ASSERT(!rc);

    NEXUS_Graphics2D_GetDefaultFillSettings(&fillSettings);
    fillSettings.surface = surface;
    fillSettings.color = 0;
    rc = NEXUS_Graphics2D_Fill(gfx, &fillSettings);
    BDBG_ASSERT(!rc);

    rc = NEXUS_Graphics2D_Checkpoint(gfx, NULL);
    if (rc == NEXUS_GRAPHICS2D_QUEUED) {
        rc = BKNI_WaitForEvent(checkpointEvent, BKNI_INFINITE);
    }
    BDBG_ASSERT(!rc);

    NEXUS_SurfaceClient_GetSettings(surfaceClient, &clientSettings);
    clientSettings.displayed.callback = gfx_checkpoint;
    clientSettings.displayed.context = displayedEvent;
    rc = NEXUS_SurfaceClient_SetSettings(surfaceClient, &clientSettings);
    BDBG_ASSERT(!rc);

    rc = NEXUS_SurfaceClient_SetSurface(surfaceClient, surface);
    BDBG_ASSERT(!rc);
    rc = BKNI_WaitForEvent(displayedEvent, 5000);
    BDBG_ASSERT(!rc);

    NEXUS_SimpleVideoDecoder_GetDefaultStartCaptureSettings(&captureSettings);
    for (i=0;i<num_mosaics;i++) {
        if (!player[i]) continue;
        mosaic[i].handle = media_player_get_video_decoder(player[i]);
        BDBG_ASSERT(mosaic[i].handle);
        
        BKNI_Memcpy(&captureSettings.surface, &mosaic[i].videoSurfaces, sizeof(mosaic[i].videoSurfaces));
        NEXUS_SimpleVideoDecoder_StartCapture(mosaic[i].handle, &captureSettings);
        mosaic[i].total = 0;
    }

    num_tiles = num_mosaics;
    if (num_tiles % 2) {
        num_tiles++;
    }
    
    while (!timeout || --timeout) {
        NEXUS_SurfaceHandle captureSurface[MAX_MOSAICS];
        unsigned total = 0;
        
        memset(captureSurface, 0, sizeof(captureSurface));
        for (i=0;i<num_mosaics;i++) {
#define NUM_CAPTURE_SURFACES 2
            NEXUS_SimpleVideoDecoderCaptureStatus captureStatus[NUM_CAPTURE_SURFACES];
            NEXUS_SurfaceHandle surfaces[NUM_CAPTURE_SURFACES]; /* need to query more than one so that queue doesn't fill */
            unsigned numReturned;
            
            if (!mosaic[i].started) continue;

            NEXUS_SimpleVideoDecoder_GetCapturedSurfaces(mosaic[i].handle, surfaces, captureStatus, NUM_CAPTURE_SURFACES, &numReturned);
            if (numReturned==0) {
                captureSurface[i] = NULL;
                continue;
            }
            if (numReturned > 1) {
                /* if we get more than one, we recycle the oldest immediately */
                NEXUS_SimpleVideoDecoder_RecycleCapturedSurfaces(mosaic[i].handle, surfaces, numReturned-1);
                total += numReturned-1;
                mosaic[i].total += numReturned-1;
                BDBG_MSG(("decoder[%d] consumed %d", i, numReturned-1));
            }
            captureSurface[i] = surfaces[numReturned-1];
            
            total++;
            mosaic[i].total++;
            
            NEXUS_Graphics2D_GetDefaultBlitSettings(&blitSettings);
            blitSettings.source.surface = captureSurface[i];
            blitSettings.output.surface = surface;
            if (num_mosaics > 1) {
                blitSettings.output.rect.width = 720 / (num_tiles / 2);
                blitSettings.output.rect.height = 480 / 2;
                blitSettings.output.rect.x = blitSettings.output.rect.width * (i % (num_tiles/2));
                blitSettings.output.rect.y = blitSettings.output.rect.height * (i / (num_tiles/2));
            }
            blitSettings.mirrorOutputVertically = flip;
            BDBG_MSG(("decoder[%d] blit to %d,%d,%d,%d", i, blitSettings.output.rect.x, blitSettings.output.rect.y, blitSettings.output.rect.width, blitSettings.output.rect.height));
            rc = NEXUS_Graphics2D_Blit(gfx, &blitSettings);
            BDBG_ASSERT(!rc);
        }
        if (!total) {
            BKNI_Sleep(1);
        }
        else {
            rc = NEXUS_Graphics2D_Checkpoint(gfx, NULL);
            if (rc==NEXUS_GRAPHICS2D_QUEUED) {
                rc = BKNI_WaitForEvent(checkpointEvent, BKNI_INFINITE);
                BDBG_ASSERT(!rc);
            }
            
            for (i=0;i<num_mosaics;i++) {
                if (captureSurface[i]) {
                    NEXUS_SimpleVideoDecoder_RecycleCapturedSurfaces(mosaic[i].handle, &captureSurface[i], 1);
                }
            }

            /* tell server to blit */
            rc = NEXUS_SurfaceClient_UpdateSurface(surfaceClient, NULL);
            BDBG_ASSERT(!rc);
            rc = BKNI_WaitForEvent(displayedEvent, 5000);
            BDBG_ASSERT(!rc);
        }
        
        if (!quiet && mosaic[0].total && mosaic[0].total % 100 == 0) {
            char buf[256], *ptr = buf;
            unsigned total = 0;
            for (i=0;i<num_mosaics;i++) {
                ptr += sprintf(ptr, "%d ", mosaic[i].total);
                total += mosaic[i].total;
            }
            BDBG_WRN(("mosaics [%s] %d", buf, total));
        }

        if (cycle) {
            static unsigned cnt = 0;
            if (++cnt % cycle == 0) {
                unsigned i = rand() % num_mosaics;
                if (!player[i]) continue;
                if (mosaic[i].started) {
                    BDBG_WRN(("stopping %d", i));
                    NEXUS_SimpleVideoDecoder_StopCapture(mosaic[i].handle);
                    media_player_stop(player[i]);
                    mosaic[i].started = false;
                }
                else {
                    BDBG_WRN(("starting %d", i));
                    rc = media_player_start(player[i], &mosaic[i].start_settings);
                    if (!rc) {
                        NEXUS_SimpleVideoDecoder_GetDefaultStartCaptureSettings(&captureSettings);
                        BKNI_Memcpy(&captureSettings.surface, &mosaic[i].videoSurfaces, sizeof(mosaic[i].videoSurfaces));
                        rc = NEXUS_SimpleVideoDecoder_StartCapture(mosaic[i].handle, &captureSettings);
                        if (!rc) {
                            mosaic[i].started = true;
                        }
                    }
                }
            }
        }
    }

    NEXUS_SurfaceClient_Release(surfaceClient);
    NEXUS_Graphics2D_Close(gfx);
    NEXUS_Surface_Destroy(surface);
    
    for (i=0;i<num_mosaics;i++) {
        unsigned j;
        if (!player[i]) continue;
        if (mosaic[i].started) {
            NEXUS_SimpleVideoDecoder_StopCapture(mosaic[i].handle);
            media_player_stop(player[i]);
        }
        for (j=0; j<NEXUS_SIMPLE_DECODER_MAX_SURFACES; j++) {
            NEXUS_Surface_Destroy(mosaic[i].videoSurfaces[j]);
        }
    }
    BKNI_DestroyEvent(displayedEvent);
    BKNI_DestroyEvent(checkpointEvent);
    media_player_destroy_mosaics(player, num_mosaics);
    NxClient_Free(&allocResults);
    NxClient_Uninit();
    return 0;
}
#else
#include <stdio.h>
int main(void)
{
    printf("This application is not supported on this platform (needs playback and simple_decoder)!\n");
    return 0;
}
#endif
