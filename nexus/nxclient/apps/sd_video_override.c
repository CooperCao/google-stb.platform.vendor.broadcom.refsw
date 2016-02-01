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
 **************************************************************************/
#include "nxclient.h"
#include "nexus_platform.h"
#include "nexus_surface.h"
#include "nexus_surface_client.h"
#include "nexus_simple_video_decoder.h"
#include "bstd.h"
#include "bkni.h"
#include "bkni_multi.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "picdecoder.h"

BDBG_MODULE(sd_video_override);

static void print_usage(void)
{
    printf(
    "Usage: sd_video_override\n"
    "\n"
    "Replace SD video with a graphics.\n"
    "\n"
    "Options:\n"
    "  --help or -h for help\n"
    "  -timeout SECONDS\n"
    );
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
    NxClient_JoinSettings joinSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NxClient_AllocSettings allocSettings;
    NxClient_AllocResults allocResults;
    NxClient_ConnectSettings connectSettings;
    unsigned connectId;
    NEXUS_SurfaceHandle surface;
    NEXUS_SurfaceCreateSettings surfaceCreateSettings;
    NEXUS_Error rc;
    int curarg = 1;
    unsigned timeout = 0, starttime;
    NEXUS_Graphics2DHandle gfx;
    NEXUS_Graphics2DFillSettings fillSettings;
    NEXUS_SimpleVideoDecoderHandle videoDecoder;
    NEXUS_VideoImageInputHandle imageInput;
    NEXUS_VideoImageInputStatus imageInputStatus;
    NEXUS_SimpleVideoDecoderStartSettings startSettings;
    unsigned i;

    while (argc > curarg) {
        if (!strcmp(argv[curarg], "--help") || !strcmp(argv[curarg], "-h")) {
            print_usage();
            return 0;
        }
        else if (!strcmp(argv[curarg], "-timeout")) {
            timeout = atoi(argv[++curarg]);
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
    NEXUS_Platform_GetConfiguration(&platformConfig);

    NxClient_GetDefaultAllocSettings(&allocSettings);
    allocSettings.simpleVideoDecoder = 1;
    rc = NxClient_Alloc(&allocSettings, &allocResults);
    if (rc) return BERR_TRACE(rc);

    if (allocResults.simpleVideoDecoder[0].id) {
        videoDecoder = NEXUS_SimpleVideoDecoder_Acquire(allocResults.simpleVideoDecoder[0].id);
    }

    NxClient_GetDefaultConnectSettings(&connectSettings);
    connectSettings.simpleVideoDecoder[0].id = allocResults.simpleVideoDecoder[0].id;
    /* NOTE: don't allocate a window */
    connectSettings.simpleVideoDecoder[0].windowCapabilities.type = NxClient_VideoWindowType_eNone;
    rc = NxClient_Connect(&connectSettings, &connectId);
    if (rc) return BERR_TRACE(rc);

    NEXUS_SimpleVideoDecoder_GetDefaultStartSettings(&startSettings);
    imageInput = NEXUS_SimpleVideoDecoder_StartImageInput(videoDecoder, &startSettings);
    BDBG_ASSERT(imageInput);

    /* NOTE: instead, override SD window. */
    NEXUS_SimpleVideoDecoder_SetSdOverride(videoDecoder, true);

    NEXUS_VideoImageInput_GetStatus(imageInput, &imageInputStatus);

    NEXUS_Surface_GetDefaultCreateSettings(&surfaceCreateSettings);
    surfaceCreateSettings.pixelFormat = NEXUS_PixelFormat_eCr8_Y18_Cb8_Y08;
    surfaceCreateSettings.width = 720;
    surfaceCreateSettings.height = 480;
    for (i=0;i<NEXUS_MAX_HEAPS;i++) {
        NEXUS_MemoryStatus s;
        if (!platformConfig.heap[i] || NEXUS_Heap_GetStatus(platformConfig.heap[i], &s)) continue;
        if (s.memcIndex == imageInputStatus.memcIndex && (s.memoryType & NEXUS_MemoryType_eApplication) && s.largestFreeBlock >= 960*1080*2) {
            surfaceCreateSettings.heap = platformConfig.heap[i];
            BDBG_WRN(("found heap[%d] on MEMC%d for VideoImageInput", i, s.memcIndex));
            break;
        }
    }
    surface = NEXUS_Surface_Create(&surfaceCreateSettings);

    gfx = NEXUS_Graphics2D_Open(NEXUS_ANY_ID, NULL);
    NEXUS_Graphics2D_GetDefaultFillSettings(&fillSettings);
    fillSettings.surface = surface;
    fillSettings.color = 0xFF00FFFF; /* magenta */
    rc = NEXUS_Graphics2D_Fill(gfx, &fillSettings);
    BDBG_ASSERT(!rc);
    NEXUS_Graphics2D_Checkpoint(gfx, NULL);

    starttime = b_get_time();
    while (1) {
        /* To keep things simple, there is no storage for the surface unless another client is actually
        trying to display video. So, the overriding app must repeatedly push the override surface so that
        the other client's video is free to come and go. During the brief time when the override is enabled
        and no surface is pushed, the SD display will be black. */
        NEXUS_VideoImageInput_PushSurface(imageInput, surface, NULL);

        if (b_get_time() >= starttime + timeout*1000) {
            break;
        }
        BKNI_Sleep(1000);
    }

    NxClient_Uninit();
    return 0;
}
