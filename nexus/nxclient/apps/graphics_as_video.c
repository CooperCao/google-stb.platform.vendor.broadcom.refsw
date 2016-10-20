/******************************************************************************
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
******************************************************************************/
#if NEXUS_HAS_DISPLAY && NEXUS_HAS_SIMPLE_DECODER
#include "bstd.h"
#include "bkni.h"
#include "nxclient.h"
#include "nexus_platform.h"
#include "nexus_simple_video_decoder.h"
#include "nexus_graphics2d.h"
#include "nexus_video_image_input.h"
#include "nexus_video_input.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

BDBG_MODULE(graphics_as_video);
/* this example is a refactoring of nexus/examples/graphics/image_input.c */

#define NUM_SURFACES 3 /* triple buffering for image update every FRAME and a full queue */

struct {
    NEXUS_SurfaceHandle handle;
    bool submitted;
} g_surface[NUM_SURFACES];

static void complete(void *data, int unused)
{
    BSTD_UNUSED(unused);
    BKNI_SetEvent((BKNI_EventHandle)data);
}

static void imageBufferCallback(void *context, int param)
{
    BSTD_UNUSED(param);
    BKNI_SetEvent((BKNI_EventHandle)context);
}

static void print_usage(void)
{
    printf(
    "Usage: graphics_as_video OPTIONS\n"
    "  --help or -h for help\n"
    "  -timeout SECONDS\n"
    "  -xdm (uses timestamp managed display path)\n"
    );
}

static int wait_for_idle_decoder(NEXUS_SimpleVideoDecoderHandle videoDecoder)
{
    NEXUS_SimpleVideoDecoderClientStatus status;
    int rc;
    while (1) {
        rc = NEXUS_SimpleVideoDecoder_GetClientStatus(videoDecoder, &status);
        if (rc) return BERR_TRACE(rc);
        if (status.enabled) break;
        BKNI_Sleep(250);
    }
    return 0;
}

int main(int argc, char **argv)
{
    NxClient_JoinSettings joinSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NxClient_AllocSettings allocSettings;
    NxClient_AllocResults allocResults;
    NxClient_ConnectSettings connectSettings;
    unsigned connectId;
    NEXUS_Error rc;
    NEXUS_SurfaceCreateSettings surfaceCreateSettings;
    NEXUS_SimpleVideoDecoderHandle videoDecoder;
    NEXUS_VideoImageInputHandle imageInput;
    NEXUS_Graphics2DHandle gfx;
    NEXUS_Graphics2DSettings gfxSettings;
    BKNI_EventHandle event,imageEvent;
    NEXUS_VideoImageInputSettings imageInputSetting;
    NEXUS_VideoImageInputStatus imageInputStatus;
    unsigned i, iter = 0;
    unsigned submitIdx, releaseIdx;
    unsigned timeout = 0;
    int curarg = 1;
    bool xdm=false;
    NEXUS_SimpleVideoDecoderStartSettings startSettings;

    while (argc > curarg) {
        if (!strcmp(argv[curarg], "--help") || !strcmp(argv[curarg], "-h")) {
            print_usage();
            return 0;
        }
        else if (!strcmp(argv[curarg], "-timeout") && argc>curarg+1) {
            timeout = atoi(argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-xdm")) {
            xdm = true;
        }
        curarg++;
    }

    /* connect to server and nexus */
    NxClient_GetDefaultJoinSettings(&joinSettings);
    snprintf(joinSettings.name, NXCLIENT_MAX_NAME, "%s", argv[0]);
    joinSettings.mode = NEXUS_ClientMode_eVerified; /* need access to all heaps */
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
    rc = NxClient_Connect(&connectSettings, &connectId);
    if (rc) return BERR_TRACE(rc);

    BKNI_CreateEvent(&event);
    BKNI_CreateEvent(&imageEvent);

    gfx = NEXUS_Graphics2D_Open(0, NULL);
    NEXUS_Graphics2D_GetSettings(gfx, &gfxSettings);
    gfxSettings.checkpointCallback.callback = complete;
    gfxSettings.checkpointCallback.context = event;
    NEXUS_Graphics2D_SetSettings(gfx, &gfxSettings);

    NEXUS_Surface_GetDefaultCreateSettings(&surfaceCreateSettings);
    surfaceCreateSettings.width  = 720;
    surfaceCreateSettings.height = 480;
    surfaceCreateSettings.pixelFormat = NEXUS_PixelFormat_eCr8_Y18_Cb8_Y08;
    for (i=0;i<NEXUS_MAX_HEAPS;i++) {
        NEXUS_MemoryStatus s;
        if (!platformConfig.heap[i] || NEXUS_Heap_GetStatus(platformConfig.heap[i], &s)) continue;
        if (s.memcIndex == imageInputStatus.memcIndex && (s.memoryType & NEXUS_MemoryType_eApplication) && s.largestFreeBlock >= 960*1080*2) {
            surfaceCreateSettings.heap = platformConfig.heap[i];
            BDBG_WRN(("found heap[%d] on MEMC%d for VideoImageInput", i, s.memcIndex));
            break;
        }
    }
    if (!surfaceCreateSettings.heap) {
        BDBG_ERR(("no heap found. RTS failure likely."));
    }
    for (i=0; i<NUM_SURFACES; i++) {
        g_surface[i].handle = NEXUS_Surface_Create(&surfaceCreateSettings);
        BDBG_ASSERT(g_surface[i].handle);
    }


restart:
    NEXUS_SimpleVideoDecoder_GetDefaultStartSettings(&startSettings);
    startSettings.lowDelayImageInput = !xdm;    /* Low delay mode bypasses xdm display management */
    imageInput = NEXUS_SimpleVideoDecoder_StartImageInput(videoDecoder, &startSettings);
    BDBG_ASSERT(imageInput);

    NEXUS_VideoImageInput_GetStatus(imageInput, &imageInputStatus);

    for (i=0; i<NUM_SURFACES; i++) {
        g_surface[i].submitted = false;
    }
    submitIdx = releaseIdx = 0;

    NEXUS_VideoImageInput_GetSettings(imageInput, &imageInputSetting);
    imageInputSetting.imageCallback.callback = imageBufferCallback;
    imageInputSetting.imageCallback.context  = imageEvent;
    NEXUS_VideoImageInput_SetSettings(imageInput, &imageInputSetting);

    for (;!timeout || iter<timeout*60;iter++) {
        NEXUS_Graphics2DFillSettings fillSettings;
        NEXUS_SurfaceHandle freeSurface=NULL;
        NEXUS_VideoImageInputSurfaceSettings surfaceSettings;
        NEXUS_SurfaceHandle pic;
        size_t num_entries = 0;

        /* Make sure image surface is not in use by Video Output (VDC) */
        do {
            if (g_surface[submitIdx].submitted) {
                /* our queue is all used up, need to wait until VideoImageInput returns */
                /* a surface after it has been displayed                                */
                BDBG_MSG(("g_surface[submitIdx=%d].submitted in use, wait for recycle" , submitIdx));
                rc = BKNI_WaitForEvent(imageEvent, 3000);
                if (rc) {
                    /* If the regular decoder is preempted, we monitor then restart. We can't just
                    resume because the VideoImageInput may be different. */
                    if (!wait_for_idle_decoder(videoDecoder)) {
                        goto restart;
                    }
                    goto done;
                }
            }

            rc = NEXUS_VideoImageInput_RecycleSurface(imageInput, &freeSurface , 1, &num_entries);
            if (rc) {
                if (!wait_for_idle_decoder(videoDecoder)) {
                    goto restart;
                }
                goto done;
            }
            if (num_entries) {
                /* our surface has been displayed, we can now re-use and re-queue it */
                BDBG_MSG(("g_surface[releaseIdx=%d].handle=%p  recycSurface=%p" , releaseIdx, (void*)g_surface[releaseIdx].handle , (void*)freeSurface));
                BDBG_ASSERT(g_surface[releaseIdx].handle == freeSurface);
                g_surface[releaseIdx].submitted = false;
                if (++releaseIdx == NUM_SURFACES) releaseIdx=0;
            }

        } while (num_entries || g_surface[submitIdx].submitted);

        g_surface[submitIdx].submitted = true; /* mark as inuse */
        pic = g_surface[submitIdx].handle;
        BDBG_MSG(("pic=%p" , (void*)pic));
        if (++submitIdx == NUM_SURFACES) submitIdx=0;

        /* must do M2MC fill. CPU may not have access to this surface on some non-UMA systems. */
        NEXUS_Graphics2D_GetDefaultFillSettings(&fillSettings);
        fillSettings.surface = pic;
        fillSettings.color = 0xFF00FFFF; /* magenta */
        fillSettings.rect.x = 0;
        fillSettings.rect.width = 720;
        fillSettings.rect.y = 0;
        fillSettings.rect.height = (iter*4)%960;
        if (fillSettings.rect.height > 480) fillSettings.rect.height = 960 - fillSettings.rect.height;
        if (fillSettings.rect.height) {
            rc = NEXUS_Graphics2D_Fill(gfx, &fillSettings);
            BDBG_ASSERT(!rc);
        }
        fillSettings.color = 0xFF00FF00; /* blue */
        fillSettings.rect.y = fillSettings.rect.height;
        fillSettings.rect.height = 480-fillSettings.rect.y;
        if (fillSettings.rect.height) {
            rc = NEXUS_Graphics2D_Fill(gfx, &fillSettings);
            BDBG_ASSERT(!rc);
        }
        rc = NEXUS_Graphics2D_Checkpoint(gfx, NULL);
        if (rc == NEXUS_GRAPHICS2D_QUEUED) {
            rc = BKNI_WaitForEvent(event, 3000);
            BDBG_ASSERT(!rc);
        }

        NEXUS_VideoImageInput_GetDefaultSurfaceSettings(&surfaceSettings);
        /* Submit surface to VideoImageInput, wait if queue to feed VDC is already full */
        do {
            rc = NEXUS_VideoImageInput_PushSurface(imageInput, pic , &surfaceSettings);
            if(rc==NEXUS_IMAGEINPUT_QUEUE_FULL) {
                rc = BKNI_WaitForEvent(imageEvent, 3000);
                BDBG_ASSERT(!rc);
            }
            else if (rc) {
                if (!wait_for_idle_decoder(videoDecoder)) {
                    goto restart;
                }
                BERR_TRACE(rc);
                break;
            }
        } while (rc);
        if (rc) break;
    }

done:
    NEXUS_SimpleVideoDecoder_StopImageInput(videoDecoder);

    BKNI_DestroyEvent(imageEvent);
    BKNI_DestroyEvent(event);

    if (gfx) NEXUS_Graphics2D_Close(gfx);
    for (i=NUM_SURFACES; i > 0; i--) {
        NEXUS_Surface_Destroy(g_surface[i-1].handle);
    }
    NxClient_Disconnect(connectId);
    NxClient_Free(&allocResults);
    NxClient_Uninit();

    return 0;
}
#else
#include <stdio.h>
int main(void)
{
    printf("This application is not supported on this platform (needs display and simple_decoder)!\n");
    return 0;
}
#endif
