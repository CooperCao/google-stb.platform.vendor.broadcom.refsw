/***************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 **************************************************************************/
#include "nxclient.h"
#include "nexus_platform_client.h"
#include "nexus_surface.h"
#include "nexus_surface_client.h"
#include "picdecoder.h"
#include "binput.h"
#include "bstd.h"
#include "bkni.h"
#include "bkni_multi.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "nxapps_cmdline.h"

BDBG_MODULE(picviewer);

static void complete(void *data, int unused)
{
    BSTD_UNUSED(unused);
    BKNI_SetEvent((BKNI_EventHandle)data);
}

static void input_client_callback(void *context, int param);

static void print_usage(const struct nxapps_cmdline *cmdline)
{
    printf(
    "Usage: picviewer OPTIONS FILES\n"
    "  --help or -h for help\n"
    "  -timeout MILLISECONDS    time to next picture\n"
    "  -q                       don't print status\n"
    "  -prompt                  prompt for user input\n"
    "  -ar {box|full}           aspect ratio correction\n"
#if NEXUS_HAS_PICTURE_DECODER
    "  -crc                     print crc\n"
#endif
    );
    nxapps_cmdline_print_usage(cmdline);
}

struct client_state
{
    binput_t input;
    bool stopped;
    BKNI_EventHandle event;
    int inc;
    unsigned timeout;
};

int main(int argc, const char **argv)
{
    NxClient_JoinSettings joinSettings;
    NEXUS_ClientConfiguration clientConfig;
    NxClient_AllocSettings allocSettings;
    NxClient_AllocResults allocResults;
    NEXUS_SurfaceHandle surface = NULL;
    NEXUS_SurfaceCreateSettings createSettings;
    BKNI_EventHandle displayedEvent;
    NEXUS_SurfaceClientHandle blit_client;
    NEXUS_SurfaceClientSettings client_settings;
    struct binput_settings input_settings;
    NEXUS_Error rc;
    unsigned i;
    int curarg = 1;
    picdecoder_t handle;
    bool quiet = false;
    bool prompt = false;
    struct client_state state, *client = &state;
    const char *pictures[1024];
    unsigned total = 0;
    bool decoded_one = false;
    enum picdecoder_aspect_ratio ar = picdecoder_aspect_ratio_box;
#if NEXUS_HAS_PICTURE_DECODER
    bool crc = false;
#endif
    struct nxapps_cmdline cmdline;
    int n;

    nxapps_cmdline_init(&cmdline);
    nxapps_cmdline_allow(&cmdline, nxapps_cmdline_type_SurfaceComposition);
    memset(client, 0, sizeof(*client));
    memset(pictures, 0, sizeof(*pictures));
    client->inc = 1;

    while (argc > curarg) {
        if (!strcmp(argv[curarg], "--help") || !strcmp(argv[curarg], "-h")) {
            print_usage(&cmdline);
            return 0;
        }
        else if (!strcmp(argv[curarg], "-timeout") && argc>curarg+1) {
            client->timeout = atoi(argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-q")) {
            quiet = true;
        }
        else if (!strcmp(argv[curarg], "-prompt")) {
            prompt = true;
        }
        else if (!strcmp(argv[curarg], "-ar") && argc>curarg+1) {
            curarg++;
            if (!strcmp(argv[curarg], "box")) {
                ar = picdecoder_aspect_ratio_box;
            }
            else if (!strcmp(argv[curarg], "full")) {
                ar = picdecoder_aspect_ratio_max;
            }
            else {
                BDBG_ERR(("invalid -ar option: %s", argv[curarg]));
                print_usage(&cmdline);
                return -1;
            }
        }
#if NEXUS_HAS_PICTURE_DECODER
        else if (!strcmp(argv[curarg], "-crc")) {
            crc = true;
        }
#endif
        else if ((n = nxapps_cmdline_parse(curarg, argc, argv, &cmdline))) {
            if (n < 0) {
                print_usage(&cmdline);
                return -1;
            }
            curarg += n;
        }
        else {
            if (total < sizeof(pictures)/sizeof(pictures[0])) {
                pictures[total++] = argv[curarg];
            }
        }
        curarg++;
    }
    if (!total) {
        print_usage(&cmdline);
        return -1;
    }

    /* connect to server and nexus */
    NxClient_GetDefaultJoinSettings(&joinSettings);
    snprintf(joinSettings.name, NXCLIENT_MAX_NAME, "%s", argv[0]);
    rc = NxClient_Join(&joinSettings);
    if (rc) return -1;

    NEXUS_Platform_GetClientConfiguration(&clientConfig);
    BKNI_CreateEvent(&displayedEvent);
    BKNI_CreateEvent(&client->event);

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

    if (nxapps_cmdline_is_set(&cmdline, nxapps_cmdline_type_SurfaceComposition)) {
        NEXUS_SurfaceComposition comp;
        NxClient_GetSurfaceClientComposition(allocResults.surfaceClient[0].id, &comp);
        nxapps_cmdline_apply_SurfaceComposition(&cmdline, &comp);
        NxClient_SetSurfaceClientComposition(allocResults.surfaceClient[0].id, &comp);
    }

    if (ar != picdecoder_aspect_ratio_max) {
        NEXUS_SurfaceClientStatus status;
        NEXUS_SurfaceClient_GetStatus(blit_client, &status);

        NEXUS_Surface_GetDefaultCreateSettings(&createSettings);
        createSettings.pixelFormat = NEXUS_PixelFormat_eA8_R8_G8_B8;
        createSettings.width = status.display.framebuffer.width;
        createSettings.height = status.display.framebuffer.height;
        createSettings.heap = clientConfig.heap[NXCLIENT_SECONDARY_GRAPHICS_HEAP]; /* if NULL, will use NXCLIENT_DEFAULT_HEAP */
        surface = NEXUS_Surface_Create(&createSettings);
    }

    handle = picdecoder_open();
    if (!handle) {
        BDBG_ERR(("not supported on this platform"));
        return -1;
    }

    binput_get_default_settings(&input_settings);
    input_settings.codeAvailable.callback = input_client_callback;
    input_settings.codeAvailable.context = client;
    client->input = binput_open(&input_settings);
    if (!prompt && !quiet) {
        printf("Use remote control \"Left\"/\"Right\" to cycle through pictures. \"Play\"/\"Stop\" controls slideshow mode. \"Clear\" exits the app.\n");
    }
    i = 0;
    while (!client->stopped) {
        NEXUS_SurfaceHandle pic;
        if (!quiet) BDBG_WRN(("display %s", pictures[i]));
        pic = picdecoder_decode(handle, pictures[i]);
        if (!pic) {
            BDBG_ERR(("unable to decode %s", pictures[i]));
        }
        else {
            NEXUS_SurfaceHandle diplaypic = pic;
            if (surface) {
                rc = picdecoder_ar_correct(handle, pic, surface, ar);
                if (!rc) {
                    diplaypic = surface;
                }
            }
            decoded_one = true;
            rc = NEXUS_SurfaceClient_SetSurface(blit_client, diplaypic);
            BDBG_ASSERT(!rc);
            rc = BKNI_WaitForEvent(displayedEvent, 5000);
            BDBG_ASSERT(!rc);
            NEXUS_Surface_Destroy(pic);

#if NEXUS_HAS_PICTURE_DECODER
            if (crc) {
                NEXUS_PictureDecoderStatus status;
                picdecoder_get_status(handle, &status);
                BDBG_WRN(("SID CRC: 0x%08x", status.crc));
            }
#endif

            /* TODO: predictively decode next */
        }
        if (prompt) {
            BDBG_WRN(("press ENTER for next"));
            getchar();
        }
        else {
            BKNI_WaitForEvent(client->event, client->timeout?client->timeout:(unsigned)BKNI_INFINITE);
        }
        if (client->inc > 0) {
            i += client->inc;
            if (i == (unsigned)total) {
                if (!decoded_one) break; /* avoid infinite loop if there are no decodable pictures */
                i = 0;
            }
        }
        else if (client->inc < 0) {
            if (i == 0) {
                i = (unsigned)total-1;
            }
            else {
                i += client->inc;
            }
        }
        else {
            client->inc = 1; /* reset */
        }
    }

    picdecoder_close(handle);

    NEXUS_SurfaceClient_Release(blit_client);
    if (surface) {
        NEXUS_Surface_Destroy(surface);
    }
    binput_close(client->input);
    NxClient_Free(&allocResults);
    BKNI_DestroyEvent(client->event);
    BKNI_DestroyEvent(displayedEvent);
    NxClient_Uninit();
    return 0;
}

static void input_client_callback(void *context, int param)
{
    struct client_state *client = context;
    BSTD_UNUSED(param);
    while (1) {
        int rc;
        b_remote_key key;
        bool repeat;

        rc = binput_read(client->input, &key, &repeat);
        if (rc) break;
        switch (key) {
        case b_remote_key_right:
            if (!repeat) {
                client->inc = 1;
                client->timeout = 0;
                BKNI_SetEvent(client->event);
            }
            break;
        case b_remote_key_left:
            if (!repeat) {
                client->inc = -1;
                client->timeout = 0;
                BKNI_SetEvent(client->event);
            }
            break;
        case b_remote_key_play:
            if (!repeat) {
                client->inc = 1;
                client->timeout = 2000;
                BKNI_SetEvent(client->event);
            }
            break;
        case b_remote_key_stop:
            if (!repeat) {
                client->inc = 0;
                client->timeout = 0;
                BKNI_SetEvent(client->event);
            }
            break;
        case b_remote_key_back:
        case b_remote_key_clear:
            client->stopped = true;
            BKNI_SetEvent(client->event);
            /* can't call media_player_stop here because we're in a callback */
            break;
        default:
            break;
        }
    }
}
