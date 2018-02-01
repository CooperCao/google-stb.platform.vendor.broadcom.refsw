/******************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
******************************************************************************/
#if NEXUS_HAS_DISPLAY && NEXUS_HAS_SIMPLE_DECODER && NEXUS_HAS_STREAM_MUX && !NEXUS_NUM_DSP_VIDEO_ENCODERS
#include "bstd.h"
#include "bkni.h"
#include "nxclient.h"
#include "nexus_platform.h"
#include "nexus_simple_video_decoder.h"
#include "nexus_simple_encoder.h"
#include "nexus_simple_stc_channel.h"
#include "nexus_graphics2d.h"
#include "nexus_video_image_input.h"
#include "nexus_video_input.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

BDBG_MODULE(encode_graphics_as_video);

/* Uncomment this to run at 15fps instead of 60 */
#if 1
#define FRAMERATE_15 1
#endif

#define NUM_SURFACES 16 /* triple buffering for image update every FRAME and a full queue */

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
    "Usage: encode_graphics_as_video <filename> OPTIONS\n"
    "  --help or -h for help\n"
    "  -ptslog to log input and output PTS\n"
    "  -timeout SECONDS\n"
    "  -framerate HZ\n"
    );
}

typedef struct EncodeThreadContext
{
    pthread_t thread;
    NEXUS_SimpleEncoderHandle hEncoder;
    FILE *pOutputFile;
    FILE *pPtsLog;
}EncodeThreadContext;

static void *encoder_thread(void *context);

int main(int argc, char **argv)
{
    NxClient_JoinSettings joinSettings;
    NEXUS_ClientConfiguration clientConfig;
    NxClient_AllocSettings allocSettings;
    NxClient_AllocResults allocResults;
    NxClient_ConnectSettings connectSettings;
    unsigned connectId;
    NEXUS_Error rc;
    NEXUS_SurfaceCreateSettings surfaceCreateSettings;
    NEXUS_SimpleVideoDecoderHandle videoDecoder=NULL;
    NEXUS_VideoImageInputHandle imageInput;
    NEXUS_Graphics2DHandle gfx;
    NEXUS_Graphics2DSettings gfxSettings;
    BKNI_EventHandle event,imageEvent;
    NEXUS_VideoImageInputSettings imageInputSetting;
    NEXUS_VideoImageInputStatus imageInputStatus;
    unsigned i;
    unsigned submitIdx = 0,releaseIdx = 0;
    unsigned timeout = 0, framerate = 30;
    int curarg = 1;
    NEXUS_SimpleVideoDecoderStartSettings startSettings;
    NEXUS_SimpleStcChannelHandle stcChannel;
    NEXUS_SimpleEncoderHandle encoder=NULL;
    EncodeThreadContext threadContext;
    NEXUS_VideoImageInputSurfaceSettings surfaceSettings;
    NEXUS_SimpleEncoderStartSettings encStartSettings;
    NEXUS_SimpleEncoderSettings encoderSettings;
    uint32_t pts=0;
    unsigned frameCount=0;
    FILE *pPtsLog=NULL;
    const char *filename = NULL;

    BKNI_Memset(&threadContext, 0, sizeof(threadContext));

    while (argc > curarg) {
        if (!strcmp(argv[curarg], "--help") || !strcmp(argv[curarg], "-h")) {
            print_usage();
            return 0;
        }
        else if (!strcmp(argv[curarg], "-timeout") && argc>curarg+1) {
            timeout = atoi(argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-framerate") && argc>curarg+1) {
            framerate = atoi(argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-ptslog")) {
            pPtsLog = fopen("input_pts.log", "wb+");
            threadContext.pPtsLog = fopen("output_pts.log", "wb+");
        }
        else if (!filename) {
            filename = argv[curarg];
        }
        else {
            print_usage();
            return -1;
        }
        curarg++;
    }

    if (!filename) {
        filename = "videos/stream.mpg";
    }

    threadContext.pOutputFile = fopen(filename, "wb+");
    if ( NULL == threadContext.pOutputFile )
    {
        fprintf(stderr, "Unable to open %s for writing", filename);
        return -1;
    }

    /* connect to server and nexus */
    NxClient_GetDefaultJoinSettings(&joinSettings);
    snprintf(joinSettings.name, NXCLIENT_MAX_NAME, "%s", argv[0]);
    rc = NxClient_Join(&joinSettings);
    if (rc) return -1;
    NEXUS_Platform_GetClientConfiguration(&clientConfig);

    NxClient_GetDefaultAllocSettings(&allocSettings);
    allocSettings.simpleVideoDecoder = 1;
    allocSettings.simpleEncoder = 1;
    rc = NxClient_Alloc(&allocSettings, &allocResults);
    if (rc) return BERR_TRACE(rc);

    NxClient_GetDefaultConnectSettings(&connectSettings);
    connectSettings.simpleVideoDecoder[0].id = allocResults.simpleVideoDecoder[0].id;
    connectSettings.simpleVideoDecoder[0].windowCapabilities.encoder = true;
    connectSettings.simpleEncoder[0].id = allocResults.simpleEncoder[0].id;
    connectSettings.simpleEncoder[0].nonRealTime = true;
    connectSettings.simpleEncoder[0].audio.cpuAccessible = true;
    connectSettings.simpleEncoder[0].video.cpuAccessible = true;
    rc = NxClient_Connect(&connectSettings, &connectId);
    if (rc) return BERR_TRACE(rc);

    if (allocResults.simpleVideoDecoder[0].id) {
        videoDecoder = NEXUS_SimpleVideoDecoder_Acquire(allocResults.simpleVideoDecoder[0].id);
        BDBG_ASSERT(NULL != videoDecoder);
    }
    if (allocResults.simpleEncoder[0].id) {
        encoder = NEXUS_SimpleEncoder_Acquire(allocResults.simpleEncoder[0].id);
        BDBG_ASSERT(NULL != encoder);
    }
    stcChannel = NEXUS_SimpleStcChannel_Create(NULL);
    BDBG_ASSERT(NULL != stcChannel);

    BKNI_CreateEvent(&event);
    BKNI_CreateEvent(&imageEvent);

    gfx = NEXUS_Graphics2D_Open(0, NULL);
    NEXUS_Graphics2D_GetSettings(gfx, &gfxSettings);
    gfxSettings.checkpointCallback.callback = complete;
    gfxSettings.checkpointCallback.context = event;
    NEXUS_Graphics2D_SetSettings(gfx, &gfxSettings);

    NEXUS_SimpleVideoDecoder_SetStcChannel(videoDecoder, stcChannel);

    NEXUS_SimpleEncoder_GetSettings(encoder, &encoderSettings);
    encoderSettings.video.width = 1280;
    encoderSettings.video.height = 720;
    encoderSettings.video.interlaced = false;
    switch(framerate) {
    case 30:
        encoderSettings.video.refreshRate = 30000;
        encoderSettings.videoEncoder.frameRate = NEXUS_VideoFrameRate_e30;
        break;
    case 24:
        encoderSettings.video.refreshRate = 24000;
        encoderSettings.videoEncoder.frameRate = NEXUS_VideoFrameRate_e24;
        break;
    case 15:
        encoderSettings.video.refreshRate = 15000;
        encoderSettings.videoEncoder.frameRate = NEXUS_VideoFrameRate_e15;
        break;
    case 50:
        encoderSettings.video.refreshRate = 50000;
        encoderSettings.videoEncoder.frameRate = NEXUS_VideoFrameRate_e50;
        break;
    case 25:
        encoderSettings.video.refreshRate = 25000;
        encoderSettings.videoEncoder.frameRate = NEXUS_VideoFrameRate_e25;
        break;
    case 20:
        encoderSettings.video.refreshRate = 20000;
        encoderSettings.videoEncoder.frameRate = NEXUS_VideoFrameRate_e20;
        break;
    default:
    case 60:
        encoderSettings.video.refreshRate = 60000;
        encoderSettings.videoEncoder.frameRate = NEXUS_VideoFrameRate_e60;
        break;
    }
    NEXUS_SimpleEncoder_SetSettings(encoder, &encoderSettings);

    NEXUS_SimpleEncoder_GetDefaultStartSettings(&encStartSettings);
    encStartSettings.input.video = videoDecoder;
    /* smaller size means lower delay */
    encStartSettings.output.video.settings.codec = NEXUS_VideoCodec_eH264;
    encStartSettings.output.video.settings.profile = NEXUS_VideoCodecProfile_eMain;
    encStartSettings.output.video.settings.level = NEXUS_VideoCodecLevel_e40;
    encStartSettings.output.video.settings.nonRealTime = true;
    encStartSettings.output.video.settings.interlaced = false;
    encStartSettings.output.video.settings.bypassVideoProcessing = true;
    encStartSettings.output.transport.type = NEXUS_TransportType_eEs;
    BDBG_ASSERT(encoder);
    rc = NEXUS_SimpleEncoder_Start(encoder, &encStartSettings);
    BDBG_ASSERT(!rc);

    NEXUS_SimpleVideoDecoder_GetDefaultStartSettings(&startSettings);
    startSettings.lowDelayImageInput = false;    /* Low delay mode bypasses xdm display management */
    imageInput = NEXUS_SimpleVideoDecoder_StartImageInput(videoDecoder, &startSettings);
    BDBG_ASSERT(imageInput);

    NEXUS_VideoImageInput_GetStatus(imageInput, &imageInputStatus);

    NEXUS_Surface_GetDefaultCreateSettings(&surfaceCreateSettings);
    surfaceCreateSettings.width  = encoderSettings.video.width;
    surfaceCreateSettings.height = encoderSettings.video.height;
    surfaceCreateSettings.pixelFormat = NEXUS_PixelFormat_eCr8_Y18_Cb8_Y08;
    for (i=0;i<NEXUS_MAX_HEAPS;i++) {
        NEXUS_MemoryStatus s;
        if (!clientConfig.heap[i] || NEXUS_Heap_GetStatus(clientConfig.heap[i], &s)) continue;
        if (s.memcIndex == imageInputStatus.memcIndex && (s.memoryType & NEXUS_MemoryType_eApplication) && s.largestFreeBlock >= 960*1080*2) {
            surfaceCreateSettings.heap = clientConfig.heap[i];
            BDBG_MSG(("found heap[%d] on MEMC%d for VideoImageInput", i, s.memcIndex));
            break;
        }
    }
    if (!surfaceCreateSettings.heap) {
        BDBG_ERR(("No client heap found for MFD RTS. Feature not supported."));
        goto err_mfd_rts;
    }

    threadContext.hEncoder = encoder;
    rc = pthread_create(&threadContext.thread, NULL, encoder_thread, &threadContext);
    BDBG_ASSERT(0==rc);

    for (i=0; i<NUM_SURFACES; i++) {
        g_surface[i].handle = NEXUS_Surface_Create(&surfaceCreateSettings);
        BDBG_ASSERT(g_surface[i].handle);
        g_surface[i].submitted = false;
    }

    NEXUS_VideoImageInput_GetSettings(imageInput, &imageInputSetting);
    imageInputSetting.imageCallback.callback = imageBufferCallback;
    imageInputSetting.imageCallback.context  = imageEvent;
    NEXUS_VideoImageInput_SetSettings(imageInput, &imageInputSetting);

    BDBG_WRN(("start writing %s", filename));
    for (i=0;!timeout || i<timeout*framerate;i++) {
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
                BDBG_ASSERT(!rc);
            }

            rc=NEXUS_VideoImageInput_RecycleSurface(imageInput, &freeSurface , 1, &num_entries);
            BDBG_ASSERT(!rc);
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
        fillSettings.rect.width = encoderSettings.video.width;
        fillSettings.rect.y = 0;
        fillSettings.rect.height = (i*4)%encoderSettings.video.height;
        if (fillSettings.rect.height) {
            rc = NEXUS_Graphics2D_Fill(gfx, &fillSettings);
            BDBG_ASSERT(!rc);
        }
        fillSettings.color = 0xFF00FF00; /* blue */
        fillSettings.rect.y = fillSettings.rect.height;
        fillSettings.rect.height = encoderSettings.video.height-fillSettings.rect.y;
        if (fillSettings.rect.height) {
            rc = NEXUS_Graphics2D_Fill(gfx, &fillSettings);
            BDBG_ASSERT(!rc);
        }
        rc = NEXUS_Graphics2D_Checkpoint(gfx, NULL);
        if (rc == NEXUS_GRAPHICS2D_QUEUED) {
            rc = BKNI_WaitForEvent(event, 3000);
            BDBG_ASSERT(!rc);
        }

        if ( pPtsLog )
        {
            fprintf(pPtsLog, "%.05d %#x\n", frameCount, pts);
        }

        NEXUS_VideoImageInput_GetDefaultSurfaceSettings(&surfaceSettings);
        surfaceSettings.pts = pts;
        pts += 45000/framerate; /* 1/15 sec in 45kHz units */
        surfaceSettings.frameRate = encoderSettings.videoEncoder.frameRate;
        surfaceSettings.ptsValid = true;
        surfaceSettings.aspectRatio = NEXUS_AspectRatio_eSquarePixel;
        /* Submit surface to VideoImageInput, wait if queue to feed VDC is already full */
        do {
            rc = NEXUS_VideoImageInput_PushSurface(imageInput, pic , &surfaceSettings);
            if(rc==NEXUS_IMAGEINPUT_QUEUE_FULL) {
                rc = BKNI_WaitForEvent(imageEvent, 3000);
                BDBG_ASSERT(!rc);
                rc = NEXUS_IMAGEINPUT_QUEUE_FULL;
            }
            else if (rc) {
                BERR_TRACE(rc);
                break;
            }
        } while (rc);
        if (rc) break;
        frameCount++;
    }

    /* Recycle at least one surface to make room for EOS */
    do {
        NEXUS_SurfaceHandle freeSurface=NULL;
        size_t num_entries = 0;
        (void)BKNI_WaitForEvent(imageEvent, 3000);
        rc = NEXUS_VideoImageInput_RecycleSurface(imageInput, &freeSurface , 1, &num_entries);
    } while (rc);

    /* Send EOS */
    NEXUS_VideoImageInput_GetDefaultSurfaceSettings(&surfaceSettings);
    surfaceSettings.endOfStream = true;
    do {
        rc = NEXUS_VideoImageInput_PushSurface(imageInput, NULL, &surfaceSettings);
        if(rc==NEXUS_IMAGEINPUT_QUEUE_FULL) {
            rc = BKNI_WaitForEvent(imageEvent, 3000);
            BDBG_ASSERT(!rc);
            rc = NEXUS_IMAGEINPUT_QUEUE_FULL;
        }
        else if (rc) {
            BERR_TRACE(rc);
            break;
        }
    } while (rc);

    /* encoder thread will exit when EOS is received */
    BDBG_WRN(("Waiting for encoder thread to finish"));
    pthread_join(threadContext.thread, NULL);
    BDBG_WRN(("Done waiting for encoder thread"));

    BDBG_WRN(("Displayed %u frames", frameCount));

err_mfd_rts:
    BDBG_ASSERT(videoDecoder);
    NEXUS_SimpleVideoDecoder_StopImageInput(videoDecoder);
    NEXUS_SimpleEncoder_Stop(encoder);

    fclose(threadContext.pOutputFile);

    BKNI_DestroyEvent(imageEvent);
    BKNI_DestroyEvent(event);

    if (gfx) NEXUS_Graphics2D_Close(gfx);
    for (i=NUM_SURFACES; i > 0; i--) {
        if (g_surface[i-1].handle) NEXUS_Surface_Destroy(g_surface[i-1].handle);
    }
    NxClient_Disconnect(connectId);
    NxClient_Free(&allocResults);
    NxClient_Uninit();

    return 0;
}

#define IS_START_OF_UNIT(pDesc) (((pDesc)->flags & (NEXUS_VIDEOENCODERDESCRIPTOR_FLAG_FRAME_START|NEXUS_VIDEOENCODERDESCRIPTOR_FLAG_EOS))?true:false)

static bool HaveCompletePacket(
    const NEXUS_VideoEncoderDescriptor *pDesc0,
    size_t size0,
    const NEXUS_VideoEncoderDescriptor *pDesc1,
    size_t size1,
    size_t *pNumDesc)
{
    *pNumDesc=0;
    if ( size0 <= 0 )
    {
        return false;
    }
    if ( size0 + size1 < 2 )
    {
        return false;
    }
    if ( IS_START_OF_UNIT(pDesc0) )
    {
        /* We have a start of frame or data unit */
        *pNumDesc=*pNumDesc+1;
        pDesc0++;
        size0--;
        /* Look for next one */
        while ( size0 > 0 )
        {
            if ( IS_START_OF_UNIT(pDesc0) )
            {
                return true;
            }
            *pNumDesc=*pNumDesc+1;
            pDesc0++;
            size0--;
        }
        while ( size1 > 0 )
        {
            if ( IS_START_OF_UNIT(pDesc1) )
            {
                return true;
            }
            *pNumDesc=*pNumDesc+1;
            pDesc1++;
            size1--;
        }
    }
    *pNumDesc=0;
    return false;
}

#define ADVANCE_DESC() do { if ( size0 > 1 ) { pDesc0++; size0--; } else { pDesc0=pDesc1; size0=size1; pDesc1=NULL; size1=0; } } while (0)
#define DESC_DATA_PTR(PDESC, BASEPTR) ((void*)&((unsigned char *)(BASEPTR))[(PDESC)->offset])

static void *encoder_thread(void *context)
{
    EncodeThreadContext *pContext = context;
    bool eos=false;
    NEXUS_SimpleEncoderStatus encStatus;
    const NEXUS_VideoEncoderDescriptor *pDesc0, *pDesc1;
    size_t size0, size1;
    void *pBufferBase;
    NEXUS_Error rc;
    void *pData;
    uint32_t frameCount=0;

    NEXUS_SimpleEncoder_GetStatus(pContext->hEncoder, &encStatus);
    rc = NEXUS_MemoryBlock_Lock(encStatus.video.bufferBlock, &pBufferBase);
    BDBG_ASSERT(!rc);

    while ( !eos )
    {
        size_t numRequired=0;
        bool foundData=false;
        rc = NEXUS_SimpleEncoder_GetVideoBuffer(pContext->hEncoder, &pDesc0, &size0, &pDesc1, &size1);
        if ( NEXUS_SUCCESS == rc && size0 > 0 )
        {
            foundData = true;
            /* Check for eos */
            if ( pDesc0->flags & NEXUS_VIDEOENCODERDESCRIPTOR_FLAG_EOS )
            {
                BDBG_WRN(("Encoder EOS received"));
                eos=true;
                break;
            }
            /* Drop metadata and empty frames first */
            if ( pDesc0->flags & (NEXUS_VIDEOENCODERDESCRIPTOR_FLAG_METADATA|NEXUS_VIDEOENCODERDESCRIPTOR_FLAG_EMPTY_FRAME) )
            {
                /* Drop this and retry */
                NEXUS_SimpleEncoder_VideoReadComplete(pContext->hEncoder, 1);
                continue;
            }
            /* See if we have a complete unit of data */
            if ( false == HaveCompletePacket(pDesc0,size0,pDesc1,size1,&numRequired) )
            {
                continue;
            }
            if ( pDesc0->flags & NEXUS_VIDEOENCODERDESCRIPTOR_FLAG_FRAME_START )
            {
                /* Frame data, just write to file */
                while ( numRequired > 0 )
                {
                    if ( pDesc0->flags & NEXUS_VIDEOENCODERDESCRIPTOR_FLAG_ORIGINALPTS_VALID )
                    {
                        if ( pContext->pPtsLog )
                        {
                            fprintf(pContext->pPtsLog, "%.05d %#x\n", frameCount, pDesc0->originalPts);
                        }
                    }
                    /* Write payload to file */
                    pData = DESC_DATA_PTR(pDesc0, pBufferBase);
                    fwrite(pData, 1, pDesc0->length, pContext->pOutputFile);
                    numRequired--;
                    NEXUS_SimpleEncoder_VideoReadComplete(pContext->hEncoder, 1);
                    ADVANCE_DESC();
                }
                if ( ((++frameCount) % 60) == 0 )
                {
                    BDBG_WRN(("Written %u frames", frameCount));
                }
            }
        }

        if ( !eos && !foundData )
        {
            BKNI_Sleep(100);
        }
    }

    BDBG_WRN(("Encoded %u frames", frameCount));

    return NULL;

}
#else
#include <stdio.h>
int main(void)
{
#if NEXUS_NUM_DSP_VIDEO_ENCODERS
    fprintf(stderr, "DSP encoding does not support graphics\n");
#else
    fprintf(stderr, "This platform does not support video encoding\n");
#endif
    return 0;
}
#endif
