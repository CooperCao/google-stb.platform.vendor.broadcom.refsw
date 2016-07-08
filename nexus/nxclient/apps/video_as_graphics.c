/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
 *****************************************************************************/
#if NEXUS_HAS_PLAYBACK && NEXUS_HAS_SIMPLE_DECODER
#include "nexus_platform_client.h"
#include "media_player.h"
#include "nexus_surface_client.h"
#include "nexus_core_utils.h"
#include "namevalue.h"
#include "nxclient.h"
#include "nexus_graphics2d.h"
#include "nxapps_cmdline.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include "bstd.h"
#include "bkni.h"

BDBG_MODULE(video_as_graphics);

/**
NOTE: this client is preliminary; not ready to be run.
**/

static void print_usage(const struct nxapps_cmdline *cmdline)
{
    printf(
    "Usage: video_as_graphics OPTIONS filename [indexfile]\n"
    "  --help or -h for help\n"
    "  -once\n"
    "  -downscale factor\n"
    "  -flip                    flip video vertically (proof that it's video as graphics)\n"
    "  -max WIDTH,HEIGHT        max video decoder resolution\n"
    "  -v                       print verbose output\n"
    "  -timeout SECONDS\n"
    "  -colorkey MIN,MAX        enable colorkey and set min and max threshold (ARGB hex format)\n"
    );
    printf(
    "  -forceFrameDestripe      destripe pictures from VideoDecoder as frames to improve quality\n"
    "  -secure\n"
    );
    nxapps_cmdline_print_usage(cmdline);
}

struct client_state
{
    BKNI_EventHandle endOfStreamEvent;
    media_player_t player;
};

static void complete(void *context)
{
    BKNI_SetEvent((BKNI_EventHandle)context);
}

static void gfx_checkpoint(void *data, int unused)
{
    BSTD_UNUSED(unused);
    BKNI_SetEvent((BKNI_EventHandle)data);
}

#include <sys/time.h>
static unsigned b_get_time(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec*1000 + tv.tv_usec/1000;
}

int main(int argc, const char **argv)
{
    NEXUS_Error rc = 0;
    int curarg = 1;
    struct client_state context, *client = &context;
    media_player_create_settings create_settings;
    media_player_start_settings start_settings;
    NxClient_JoinSettings joinSettings;
    NEXUS_ClientConfiguration clientConfig;
    NxClient_AllocSettings allocSettings;
    NxClient_AllocResults allocResults;
    NEXUS_SurfaceClientHandle surfaceClient;
    BKNI_EventHandle displayedEvent, checkpointEvent;
    NEXUS_SurfaceHandle surface, videoSurfaces[NEXUS_SIMPLE_DECODER_MAX_SURFACES];
    NEXUS_SurfaceMemory mem;
    NEXUS_SurfaceCreateSettings createSettings;
    NEXUS_Graphics2DHandle gfx;
    NEXUS_Graphics2DOpenSettings graphics2dOpenSettings;
    NEXUS_Graphics2DSettings gfxSettings;
    NEXUS_Graphics2DFillSettings fillSettings;
    NEXUS_Graphics2DBlitSettings blitSettings;
    NEXUS_SurfaceClientSettings clientSettings;
    NEXUS_SimpleVideoDecoderHandle videoDecoder;
    NEXUS_SimpleVideoDecoderStartCaptureSettings captureSettings;
    unsigned i, numReturned, downscale = 1;
    bool flip = false;
    unsigned total = 0;
    unsigned maxWidth = 0;
    unsigned maxHeight = 0;
    bool verbose = false;
    unsigned timeout = 0, starttime;
    struct {
        unsigned keyMin, keyMax;
        NEXUS_SurfaceHandle backgroundSurface;
    } colorkey;
    bool forceFrameDestripe = false;
    struct nxapps_cmdline cmdline;
    int n;

    media_player_get_default_create_settings(&create_settings);
    media_player_get_default_start_settings(&start_settings);
    memset(&colorkey, 0, sizeof(colorkey));
    nxapps_cmdline_init(&cmdline);
    nxapps_cmdline_allow(&cmdline, nxapps_cmdline_type_SurfaceComposition);

    while (argc > curarg) {
        if (!strcmp(argv[curarg], "-h") || !strcmp(argv[curarg], "--help")) {
            print_usage(&cmdline);
            return 0;
        }
        else if (!strcmp(argv[curarg], "-once")) {
            start_settings.loopMode = NEXUS_PlaybackLoopMode_ePause;
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
        else if (!strcmp(argv[curarg], "-max") && curarg+1 < argc) {
            int n = sscanf(argv[++curarg], "%u,%u", &maxWidth, &maxHeight);
            if (n != 2) {
                print_usage(&cmdline);
                return -1;
            }
        }
        else if (!strcmp(argv[curarg], "-v")) {
            verbose = true;
        }
        else if (!strcmp(argv[curarg], "-timeout") && argc>curarg+1) {
            timeout = atoi(argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-colorkey") && argc>curarg+1) {
            sscanf(argv[++curarg], "%x,%x", &colorkey.keyMin, &colorkey.keyMax);
        }
        else if (!strcmp(argv[curarg], "-forceFrameDestripe")) {
            forceFrameDestripe = true;
        }
        else if (!strcmp(argv[curarg], "-secure")) {
            start_settings.video.secure = true;
        }
        else if ((n = nxapps_cmdline_parse(curarg, argc, argv, &cmdline))) {
            if (n < 0) {
                print_usage(&cmdline);
                return -1;
            }
            curarg += n;
        }
        else if (!start_settings.stream_url) {
            start_settings.stream_url = argv[curarg];
        }
        else if (!start_settings.index_url) {
            start_settings.index_url = argv[curarg];
        }
        else {
            print_usage(&cmdline);
            return -1;
        }
        curarg++;
    }
    if (!start_settings.stream_url) {
        print_usage(&cmdline);
        return -1;
    }
    
    NxClient_GetDefaultJoinSettings(&joinSettings);
    snprintf(joinSettings.name, NXCLIENT_MAX_NAME, "%s %s", argv[0], start_settings.stream_url);
    rc = NxClient_Join(&joinSettings);
    if (rc) return -1;
    
    NEXUS_Platform_GetClientConfiguration(&clientConfig);
    BKNI_CreateEvent(&client->endOfStreamEvent);
    BKNI_CreateEvent(&displayedEvent);
    BKNI_CreateEvent(&checkpointEvent);

    NxClient_GetDefaultAllocSettings(&allocSettings);
    allocSettings.surfaceClient = 1; /* surface client required for video window */
    rc = NxClient_Alloc(&allocSettings, &allocResults);
    if (rc) return BERR_TRACE(rc);

    surfaceClient = NEXUS_SurfaceClient_Acquire(allocResults.surfaceClient[0].id);
    if (surfaceClient) {       
        if (nxapps_cmdline_is_set(&cmdline, nxapps_cmdline_type_SurfaceComposition)) {
            NEXUS_SurfaceComposition comp;
            NxClient_GetSurfaceClientComposition(allocResults.surfaceClient[0].id, &comp);
            nxapps_cmdline_apply_SurfaceComposition(&cmdline, &comp);
            NxClient_SetSurfaceClientComposition(allocResults.surfaceClient[0].id, &comp);
        }
    }

    create_settings.window.surfaceClientId = allocResults.surfaceClient[0].id;
    create_settings.window.id = 0;
    if (maxWidth || maxHeight) {
        create_settings.maxWidth = maxWidth;
        create_settings.maxHeight = maxHeight;
    }
    client->player = media_player_create(&create_settings);
    if (!client->player) return -1;

    start_settings.eof = complete;
    start_settings.context = client->endOfStreamEvent;
    start_settings.videoWindowType = NxClient_VideoWindowType_eNone;
    rc = media_player_start(client->player, &start_settings);
    if (rc) {
        BERR_TRACE(rc);
        goto err_start;
    }

    NEXUS_Surface_GetDefaultCreateSettings(&createSettings);
    createSettings.pixelFormat = NEXUS_PixelFormat_eA8_R8_G8_B8;
    createSettings.width = 720/downscale;
    createSettings.height = 480/downscale;
    if (start_settings.video.secure) {
        createSettings.heap = clientConfig.heap[NXCLIENT_SECURE_GRAPHICS_HEAP];
    }
    for (i=0; i<NEXUS_SIMPLE_DECODER_MAX_SURFACES; i++) {
        videoSurfaces[i] = NEXUS_Surface_Create(&createSettings);
    }
    surface = NEXUS_Surface_Create(&createSettings);
  
    if (colorkey.keyMin) {
        unsigned char *buffer;
        colorkey.backgroundSurface = NEXUS_Surface_Create(&createSettings);
        NEXUS_Surface_GetMemory(colorkey.backgroundSurface, &mem);
        buffer = mem.buffer;
        for (i = 0; i < createSettings.height; i++) {
            unsigned j;
            for (j = 0; j < createSettings.width; j++) {
                *buffer++ = 0xFF; /* alpha */
                *buffer++ = (j + i)/2 % 0xFF; /* R */
                *buffer++ = (j - i/2) % 0xFF; /* G */
                *buffer++ = i/2 % 0xFF; /* B */
            }
        }
    }

    NEXUS_Graphics2D_GetDefaultOpenSettings(&graphics2dOpenSettings);
    graphics2dOpenSettings.secure = start_settings.video.secure;
    gfx = NEXUS_Graphics2D_Open(NEXUS_ANY_ID, &graphics2dOpenSettings);
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

    videoDecoder = media_player_get_video_decoder(client->player);
    BDBG_ASSERT(videoDecoder);

    NEXUS_SimpleVideoDecoder_GetDefaultStartCaptureSettings(&captureSettings);
    BKNI_Memcpy(&captureSettings.surface, &videoSurfaces, sizeof(videoSurfaces));
    captureSettings.forceFrameDestripe = forceFrameDestripe;
    captureSettings.secure = start_settings.video.secure;
    rc = NEXUS_SimpleVideoDecoder_StartCapture(videoDecoder, &captureSettings);
    if ( rc == NEXUS_NOT_SUPPORTED ) {
        BDBG_WRN(("Video as graphics not supported !" ));
        goto err_noVideoGfx;
    }

    BDBG_WRN(("start"));
    starttime = b_get_time();
    while (1) {
#define NUM_CAPTURE_SURFACES 2
        NEXUS_SurfaceHandle captureSurface[NUM_CAPTURE_SURFACES];
        NEXUS_SimpleVideoDecoderCaptureStatus captureStatus[NUM_CAPTURE_SURFACES];
        if (start_settings.loopMode == NEXUS_PlaybackLoopMode_ePause) {
            rc = BKNI_WaitForEvent(client->endOfStreamEvent, 0);
            if (rc==BERR_SUCCESS) {
                break;
            }
        }

        NEXUS_SimpleVideoDecoder_GetCapturedSurfaces(videoDecoder, captureSurface, captureStatus, NUM_CAPTURE_SURFACES, &numReturned);
        if (numReturned==0) {
            BKNI_Sleep(10);
            goto done;
        }
        total += numReturned;
        if (numReturned > 1) {
            /* if we get more than one, we recycle the oldest immediately */
            NEXUS_SimpleVideoDecoder_RecycleCapturedSurfaces(videoDecoder, captureSurface, numReturned-1);
        }

        if (verbose) {
            BDBG_WRN(("capture surface %d: pts %#x", captureStatus[numReturned-1].serialNumber, captureStatus[numReturned-1].pts));
        }

        NEXUS_Graphics2D_GetDefaultBlitSettings(&blitSettings);
        blitSettings.source.surface = captureSurface[numReturned-1];
        blitSettings.output.surface = surface;
        blitSettings.mirrorOutputVertically = flip;

        if (colorkey.keyMin) {
            blitSettings.dest.surface = colorkey.backgroundSurface;
            blitSettings.colorKey.source.enabled = true;
            blitSettings.colorKey.source.lower = colorkey.keyMin;
            blitSettings.colorKey.source.upper = colorkey.keyMax;
            blitSettings.colorKey.source.mask = 0xFFFFFFF;
        }

        rc = NEXUS_Graphics2D_Blit(gfx, &blitSettings);
        BDBG_ASSERT(!rc);

        rc = NEXUS_Graphics2D_Checkpoint(gfx, NULL);
        if (rc==NEXUS_GRAPHICS2D_QUEUED) {
            rc = BKNI_WaitForEvent(checkpointEvent, BKNI_INFINITE);
            BDBG_ASSERT(!rc);
        }

        NEXUS_SimpleVideoDecoder_RecycleCapturedSurfaces(videoDecoder, &captureSurface[numReturned-1], 1);
        
        /* tell server to blit */
        rc = NEXUS_SurfaceClient_UpdateSurface(surfaceClient, NULL);
        BDBG_ASSERT(!rc);
        rc = BKNI_WaitForEvent(displayedEvent, 5000);
        BDBG_ASSERT(!rc);
        
        if (total % 100 == 0) {
            BDBG_MSG(("%d pictures", total));
        }
done:
        if (timeout && b_get_time() - starttime >= timeout*1000) {
            break;
        }
    }

err_noVideoGfx:
    NEXUS_SimpleVideoDecoder_StopCapture(videoDecoder);
    media_player_stop(client->player);

    NEXUS_SurfaceClient_Release(surfaceClient);
    NEXUS_Graphics2D_Close(gfx);
    NEXUS_Surface_Destroy(surface);
    for (i=0; i<NEXUS_SIMPLE_DECODER_MAX_SURFACES; i++) {
        NEXUS_Surface_Destroy(videoSurfaces[i]);
    }
err_start:
    BKNI_DestroyEvent(client->endOfStreamEvent);
    BKNI_DestroyEvent(displayedEvent);
    BKNI_DestroyEvent(checkpointEvent);
    media_player_destroy(client->player);
    if (colorkey.backgroundSurface) {
        NEXUS_Surface_Destroy(colorkey.backgroundSurface);
    }
    NxClient_Uninit();
    return rc;
}
#else
#include <stdio.h>
int main(void)
{
    printf("This application is not supported on this platform (needs playback and simple_decoder)!\n");
    return 0;
}
#endif
