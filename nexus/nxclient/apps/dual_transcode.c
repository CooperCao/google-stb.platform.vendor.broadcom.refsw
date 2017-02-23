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
 *
 *****************************************************************************/
#if NEXUS_HAS_PLAYBACK && NEXUS_HAS_SIMPLE_DECODER && NEXUS_HAS_STREAM_MUX
#include "nxclient.h"
#include "nexus_platform_client.h"
#include "nexus_simple_video_decoder.h"
#include "nexus_simple_audio_decoder.h"
#include "nexus_simple_encoder.h"
#include "nexus_playback.h"
#include "nexus_file.h"
#include "nexus_core_utils.h"
#include "media_probe.h"
#include "brecord_gui.h"
#include "nexus_platform.h"
#include "nexus_memory.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>
#include "bstd.h"
#include "bkni.h"

#include "namevalue.h"

BDBG_MODULE(transcode);

struct DecodeContext
{
    NEXUS_SimpleVideoDecoderHandle hVideoDecoder;
    NEXUS_SimpleAudioDecoderHandle hAudioDecoder;
    NEXUS_PlaybackHandle hPlayback;
    NEXUS_PlaypumpHandle hPlaypump;
    const char *filename;
    NEXUS_FilePlayHandle file;
    NEXUS_SimpleVideoDecoderStartSettings videoProgram;
    NEXUS_SimpleAudioDecoderStartSettings audioProgram;
    NEXUS_SimpleStcChannelHandle stcChannel;
    bool loop;

    NEXUS_SurfaceHandle surface[2];
    unsigned cur_surface; /* the one to render */
    BKNI_EventHandle checkpointEvent;
    NEXUS_Graphics2DHandle gfx;
    unsigned displayIndex;
    NEXUS_DisplayHandle display;
    NEXUS_SurfaceHandle videoSurfaces[NEXUS_SIMPLE_DECODER_MAX_SURFACES];
};

struct EncodeContext
{
    NEXUS_SimpleEncoderHandle hEncoder;
    NEXUS_RecpumpHandle hRecpump;
    NEXUS_DisplayHandle display;
    unsigned displayIndex;
    const char *outputfile;
    FILE *pOutputFile;
    void *pVideoBase;
    void *pAudioBase;
    size_t videoSize, lastVideoSize;
    size_t audioSize, lastAudioSize;
    unsigned streamTotal, lastsize;
    bool stopped;
    struct {
        unsigned videoBitrate;/* bps */
        unsigned width;
        unsigned height;
        NEXUS_VideoFrameRate frameRateEnum;
        unsigned frameRate;
        NEXUS_Rect window, pip_window;
    } encoderSettings;
    struct {
        unsigned videoPid;
        unsigned videoCodec;
        unsigned audioPid;
        unsigned audioCodec;
        unsigned audioSampleRate;
    } encoderStartSettings;
    NEXUS_SimpleEncoderStartSettings startSettings;
};

#define MB (1024*1024)
#define NULL_PID 0x1fff

static void complete(void *data, int unused)
{
    BSTD_UNUSED(unused);
    BKNI_SetEvent((BKNI_EventHandle)data);
}

static void print_usage(void)
{
    printf(
        "Usage: dual_transcode [OPTIONS] INPUTFILE0 INPUTFILE1 [OUTPUTFILE]\n"
        "\n"
        "Default OUTPUTFILE videos/stream#.mpg where # is the RAVE context used\n"
        "\n"
        "OPTIONS:\n"
        "  --help or -h for help\n"
        );
    printf(
        "  -video         output video pid (0 for no video)\n"
        );
    print_list_option(
        "  -video_type    output video type",g_videoCodecStrs);
    printf(
        "  -video_bitrate RATE   output video bitrate in Mbps\n"
        "  -video_size    WIDTH,HEIGHT (default is 1280,720)\n"
        "  -video_framerate HZ    video encode frame rate (for example 29.97, 30, 59.94, 60)\n"
        "  -window X,Y,WIDTH,HEIGHT    window within video_size (default is full size)\n"
        "  -pip_window X,Y,WIDTH,HEIGHT    PIP window within video_size (default is top right quarter)\n"
        );
    printf(
        "  -audio         output audio pid (0 for no audio)\n"
        );
    print_list_option(
        "  -audio_type    output audio type",g_audioCodecStrs);
    printf(
        "  -audio_samplerate     audio output sample rate in HZ\n"
        "  -loop          loop playback\n"
        "  -timeout SECONDS\n"
        "  -gui off \n"
        );
}

static unsigned b_get_time(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec*1000 + tv.tv_usec/1000;
}

static int start_encode(struct EncodeContext *pContext, struct DecodeContext *pDecodeContext)
{
    NEXUS_SimpleEncoderSettings encoderSettings;
    NEXUS_SimpleEncoderStartSettings startSettings;
    NEXUS_SimpleEncoderStatus encStatus;
    NEXUS_VideoEncoderCapabilities cap;
    NEXUS_Error rc;

    pContext->pOutputFile = fopen(pContext->outputfile, "w+");
    if (!pContext->pOutputFile) {
        BDBG_ERR(("unable to open '%s' for writing", pContext->outputfile));
        return -1;
    }

    NEXUS_SimpleEncoder_GetSettings(pContext->hEncoder, &encoderSettings);
    if (pContext->encoderSettings.videoBitrate) {
        encoderSettings.videoEncoder.bitrateMax = pContext->encoderSettings.videoBitrate;
    } else {
        pContext->encoderSettings.videoBitrate = encoderSettings.videoEncoder.bitrateMax;
    }
    encoderSettings.video.width = pContext->encoderSettings.width;
    encoderSettings.video.height = pContext->encoderSettings.height;
    encoderSettings.video.window = pContext->encoderSettings.window;
#if !NEXUS_NUM_DSP_VIDEO_ENCODERS
    if (pContext->encoderSettings.frameRateEnum) {
        encoderSettings.videoEncoder.frameRate = pContext->encoderSettings.frameRateEnum;
    }
    switch (encoderSettings.videoEncoder.frameRate) {
    case NEXUS_VideoFrameRate_e50:
    case NEXUS_VideoFrameRate_e25:
    case NEXUS_VideoFrameRate_e12_5:
        encoderSettings.video.refreshRate = 50000;
        break;
    default:
        encoderSettings.video.refreshRate = 60000;
        break;
    }
#endif

    rc = NEXUS_SimpleEncoder_SetSettings(pContext->hEncoder, &encoderSettings);
    BDBG_ASSERT(!rc);

    NEXUS_SimpleEncoder_GetStatus(pContext->hEncoder, &encStatus);
    NEXUS_GetVideoEncoderCapabilities(&cap);
    pContext->displayIndex = cap.videoEncoder[encStatus.videoEncoder.index].displayIndex;

    pContext->display = NEXUS_Display_Open(pContext->displayIndex, NULL);
    if (!pContext->display) return BERR_TRACE(-1);

    NEXUS_SimpleEncoder_GetDefaultStartSettings(&startSettings);
    startSettings.transcode.display = pContext->display;
    startSettings.input.video = pDecodeContext->hVideoDecoder;
    startSettings.input.audio = pDecodeContext->hAudioDecoder;
    startSettings.recpump = pContext->hRecpump;
    startSettings.output.transport.type = NEXUS_TransportType_eTs;
    if (pContext->encoderStartSettings.videoCodec) {
        startSettings.output.video.settings.codec = pContext->encoderStartSettings.videoCodec;
    }
    if (pContext->encoderStartSettings.videoPid != NULL_PID) {
        startSettings.output.video.pid = pContext->encoderStartSettings.videoPid;
    }
    if (pContext->encoderStartSettings.audioCodec) {
        startSettings.output.audio.codec = pContext->encoderStartSettings.audioCodec;
    }
    if (pContext->encoderStartSettings.audioSampleRate != 0) {
        startSettings.output.audio.sampleRate = pContext->encoderStartSettings.audioSampleRate;
    }
    if (pContext->encoderStartSettings.audioPid != NULL_PID) {
        startSettings.output.audio.pid = pContext->encoderStartSettings.audioPid;
    }

    rc = NEXUS_SimpleEncoder_Start(pContext->hEncoder, &startSettings);
    if (rc) {
        BDBG_ERR(("unable to start encoder"));
        return -1;
    }
    pContext->startSettings = startSettings;

    /* recpump must start after decoders start */
    rc = NEXUS_Recpump_Start(pContext->hRecpump);
    BDBG_ASSERT(!rc);
    rc = NEXUS_MemoryBlock_Lock(encStatus.video.bufferBlock, &pContext->pVideoBase);
    BDBG_ASSERT(!rc);
    rc = NEXUS_MemoryBlock_Lock(encStatus.audio.bufferBlock, &pContext->pAudioBase);
    BDBG_ASSERT(!rc);

    return 0;
}

static void stop_encode(struct EncodeContext *pContext)
{
    NEXUS_Recpump_Stop(pContext->hRecpump);
    NEXUS_SimpleEncoder_Stop(pContext->hEncoder);
    NEXUS_Display_Close(pContext->display);
    fclose(pContext->pOutputFile);
    pContext->videoSize=pContext->lastVideoSize=pContext->audioSize=pContext->lastAudioSize=0;
    pContext->streamTotal=pContext->lastsize=0;
}

static int prestart_decode(struct DecodeContext *pContext, const NxClient_AllocResults *pAllocResults)
{
    struct probe_results probe_results;
    NEXUS_PlaypumpOpenSettings openSettings;
    NEXUS_PlaybackSettings playbackSettings;
    NEXUS_PlaybackPidChannelSettings playbackPidSettings;
    NEXUS_SimpleStcChannelSettings stcSettings;
    int rc;

    pContext->file = NEXUS_FilePlay_OpenPosix(pContext->filename, pContext->filename);
    if (!pContext->file) {
        BDBG_ERR(("can't open file: %s", pContext->filename));
        return -1;
    }

    pContext->stcChannel = NEXUS_SimpleStcChannel_Create(NULL);
    BDBG_ASSERT(pContext->stcChannel);

    rc = probe_media(pContext->filename, &probe_results);
    if (rc) return BERR_TRACE(rc);

    NEXUS_Playpump_GetDefaultOpenSettings(&openSettings);
    openSettings.fifoSize /= 2;
    pContext->hPlaypump = NEXUS_Playpump_Open(NEXUS_ANY_ID, &openSettings);
    if (!pContext->hPlaypump) {BDBG_WRN(("unable to alloc transcode resources")); return -1;}
    pContext->hPlayback = NEXUS_Playback_Create();
    BDBG_ASSERT(pContext->hPlayback);

    NEXUS_SimpleStcChannel_GetSettings(pContext->stcChannel, &stcSettings);
    stcSettings.modeSettings.Auto.transportType = probe_results.transportType;
    stcSettings.mode = NEXUS_StcChannelMode_eAuto;
    rc = NEXUS_SimpleStcChannel_SetSettings(pContext->stcChannel, &stcSettings);
    if (rc) {BDBG_WRN(("unable to set stcsettings")); return -1;}

    NEXUS_Playback_GetSettings(pContext->hPlayback, &playbackSettings);
    playbackSettings.playpump = pContext->hPlaypump;
    playbackSettings.playpumpSettings.transportType = probe_results.transportType;
    playbackSettings.playpumpSettings.timestamp.type = probe_results.timestampType;
    playbackSettings.simpleStcChannel = pContext->stcChannel;
    playbackSettings.endOfStreamAction = pContext->loop ? NEXUS_PlaybackLoopMode_eLoop : NEXUS_PlaybackLoopMode_ePause;
    NEXUS_Playback_SetSettings(pContext->hPlayback, &playbackSettings);

    memset(&pContext->videoProgram, 0, sizeof(pContext->videoProgram));
    memset(&pContext->audioProgram, 0, sizeof(pContext->audioProgram));

    if (pAllocResults->simpleVideoDecoder[0].id) {
        pContext->hVideoDecoder = NEXUS_SimpleVideoDecoder_Acquire(pAllocResults->simpleVideoDecoder[0].id);
        BDBG_ASSERT(pContext->hVideoDecoder);
        NEXUS_SimpleVideoDecoder_GetDefaultStartSettings(&pContext->videoProgram);
        NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
        playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eVideo;
        playbackPidSettings.pidTypeSettings.video.codec = probe_results.video[0].codec;
        playbackPidSettings.pidTypeSettings.video.index = false;
        playbackPidSettings.pidTypeSettings.video.simpleDecoder = pContext->hVideoDecoder;
        pContext->videoProgram.settings.pidChannel = NEXUS_Playback_OpenPidChannel(pContext->hPlayback, probe_results.video[0].pid, &playbackPidSettings);
        pContext->videoProgram.settings.codec = probe_results.video[0].codec;
        pContext->videoProgram.maxWidth = probe_results.video[0].width;
        pContext->videoProgram.maxHeight = probe_results.video[0].height;
        if (probe_results.video[0].colorDepth > 8) {
            NEXUS_VideoDecoderSettings settings;
            NEXUS_SimpleVideoDecoder_GetSettings(pContext->hVideoDecoder, &settings);
            settings.colorDepth = probe_results.video[0].colorDepth;
            NEXUS_SimpleVideoDecoder_SetSettings(pContext->hVideoDecoder, &settings);
        }
        BDBG_MSG(("transcode video %#x %s", probe_results.video[0].pid, lookup_name(g_videoCodecStrs, probe_results.video[0].codec)));
        NEXUS_SimpleVideoDecoder_SetStcChannel(pContext->hVideoDecoder, pContext->stcChannel);
    }
    else {
        pContext->hVideoDecoder = NULL;
    }

    if (pAllocResults->simpleAudioDecoder.id) {
        pContext->hAudioDecoder = NEXUS_SimpleAudioDecoder_Acquire(pAllocResults->simpleAudioDecoder.id);
        BDBG_ASSERT(pContext->hAudioDecoder);
        NEXUS_SimpleAudioDecoder_GetDefaultStartSettings(&pContext->audioProgram);
        NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
        playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eAudio;
        playbackPidSettings.pidTypeSettings.audio.simpleDecoder = pContext->hAudioDecoder;
        pContext->audioProgram.primary.pidChannel = NEXUS_Playback_OpenPidChannel(pContext->hPlayback, probe_results.audio[0].pid, &playbackPidSettings);
        pContext->audioProgram.primary.codec = probe_results.audio[0].codec;
        BDBG_MSG(("transcode audio %#x %s", probe_results.audio[0].pid, lookup_name(g_audioCodecStrs, probe_results.audio[0].codec)));
        NEXUS_SimpleAudioDecoder_SetStcChannel(pContext->hAudioDecoder, pContext->stcChannel);
    }
    else {
        pContext->hAudioDecoder = NULL;
    }

    return 0;
}

static int start_decode(struct DecodeContext *pContext)
{
    int rc;
    if (pContext->videoProgram.settings.pidChannel) {
        rc = NEXUS_SimpleVideoDecoder_Start(pContext->hVideoDecoder, &pContext->videoProgram);
        if (rc) return BERR_TRACE(rc);
    }
    if (pContext->hAudioDecoder && pContext->audioProgram.primary.pidChannel) {
        NEXUS_SimpleAudioDecoder_Start(pContext->hAudioDecoder, &pContext->audioProgram);
        /* decode may fail if audio codec not supported */
    }
    rc = NEXUS_Playback_Start(pContext->hPlayback, pContext->file, NULL);
    if (rc) return BERR_TRACE(rc);
    return rc;
}

static void stop_decode(struct DecodeContext *pContext)
{
    NEXUS_Playback_Stop(pContext->hPlayback);
    if (pContext->hVideoDecoder) {
        NEXUS_SimpleVideoDecoder_Stop(pContext->hVideoDecoder);
    }
    if (pContext->hAudioDecoder) {
        NEXUS_SimpleAudioDecoder_Stop(pContext->hAudioDecoder);
    }
    NEXUS_FilePlay_Close(pContext->file);
    if (pContext->videoProgram.settings.pidChannel) {
        NEXUS_Playback_ClosePidChannel(pContext->hPlayback, pContext->videoProgram.settings.pidChannel);
    }
    if (pContext->audioProgram.primary.pidChannel) {
        NEXUS_Playback_ClosePidChannel(pContext->hPlayback, pContext->audioProgram.primary.pidChannel);
    }
    if (pContext->hVideoDecoder) {
        NEXUS_SimpleVideoDecoder_Release(pContext->hVideoDecoder);
    }
    if (pContext->hAudioDecoder) {
        NEXUS_SimpleAudioDecoder_Release(pContext->hAudioDecoder);
    }
    NEXUS_Playback_Destroy(pContext->hPlayback);
    NEXUS_Playpump_Close(pContext->hPlaypump);
}

static int checkpoint(struct DecodeContext *pContext)
{
    int rc;
    rc = NEXUS_Graphics2D_Checkpoint(pContext->gfx, NULL); /* require to execute queue */
    if (rc == NEXUS_GRAPHICS2D_QUEUED) {
        rc = BKNI_WaitForEvent(pContext->checkpointEvent, BKNI_INFINITE);
    }
    if (rc) return BERR_TRACE(rc);
    return 0;
}

static int start_graphics_pip(struct EncodeContext *pEncodeContext, struct DecodeContext *pContext)
{
    NEXUS_GraphicsSettings graphicsSettings;
    NEXUS_SurfaceCreateSettings surfaceCreateSettings;
    NEXUS_Graphics2DFillSettings fillSettings;
    NEXUS_Graphics2DSettings gfxSettings;
    int rc;
    NEXUS_SurfaceCreateSettings createSettings;
    unsigned i;
    NEXUS_SimpleVideoDecoderStartCaptureSettings captureSettings;
    NEXUS_MemoryStatus heapStatus;

    NEXUS_Surface_GetDefaultCreateSettings(&surfaceCreateSettings);
    surfaceCreateSettings.width = pEncodeContext->encoderSettings.width;
    surfaceCreateSettings.height = pEncodeContext->encoderSettings.height;
    surfaceCreateSettings.heap = NEXUS_Platform_GetFramebufferHeap(pContext->displayIndex);
    rc = NEXUS_Heap_GetStatus(surfaceCreateSettings.heap, &heapStatus);
    if (rc) {
        BDBG_ERR(("client does not have access to graphics memory on MEMC%u for encoder GFD%u", heapStatus.memcIndex, pContext->displayIndex));
        return BERR_TRACE(-1);
    }
    BDBG_WRN(("encoder GFD%u requires heap %p on MEMC%u", pContext->displayIndex, (void*)surfaceCreateSettings.heap, heapStatus.memcIndex));
    pContext->surface[0] = NEXUS_Surface_Create(&surfaceCreateSettings);
    pContext->surface[1] = NEXUS_Surface_Create(&surfaceCreateSettings);

    rc = BKNI_CreateEvent(&pContext->checkpointEvent);
    if (rc) return BERR_TRACE(rc);
    pContext->gfx = NEXUS_Graphics2D_Open(NEXUS_ANY_ID, NULL);
    NEXUS_Graphics2D_GetSettings(pContext->gfx, &gfxSettings);
    gfxSettings.checkpointCallback.callback = complete;
    gfxSettings.checkpointCallback.context = pContext->checkpointEvent;
    rc = NEXUS_Graphics2D_SetSettings(pContext->gfx, &gfxSettings);
    if (rc) return BERR_TRACE(rc);

    NEXUS_Graphics2D_GetDefaultFillSettings(&fillSettings);
    fillSettings.color = 0x0;
    fillSettings.surface = pContext->surface[0];
    rc = NEXUS_Graphics2D_Fill(pContext->gfx, &fillSettings);
    if (rc) return BERR_TRACE(rc);
    fillSettings.surface = pContext->surface[1];
    rc = NEXUS_Graphics2D_Fill(pContext->gfx, &fillSettings);
    if (rc) return BERR_TRACE(rc);

    checkpoint(pContext);

    NEXUS_Display_GetGraphicsSettings(pContext->display, &graphicsSettings);
    graphicsSettings.enabled = true;
    rc = NEXUS_Display_SetGraphicsSettings(pContext->display, &graphicsSettings);
    if (rc) return BERR_TRACE(rc);
    pContext->cur_surface = 0;
    rc = NEXUS_Display_SetGraphicsFramebuffer(pContext->display, pContext->surface[pContext->cur_surface]);
    if (rc) return BERR_TRACE(rc);
    pContext->cur_surface = 1;

    NEXUS_Surface_GetDefaultCreateSettings(&createSettings);
    createSettings.pixelFormat = NEXUS_PixelFormat_eA8_R8_G8_B8;
    createSettings.width = pEncodeContext->encoderSettings.pip_window.width;
    createSettings.height = pEncodeContext->encoderSettings.pip_window.height;
    for (i=0; i<NEXUS_SIMPLE_DECODER_MAX_SURFACES; i++) {
        pContext->videoSurfaces[i] = NEXUS_Surface_Create(&createSettings);
    }

    NEXUS_SimpleVideoDecoder_GetDefaultStartCaptureSettings(&captureSettings);
    BKNI_Memcpy(&captureSettings.surface, &pContext->videoSurfaces, sizeof(pContext->videoSurfaces));
    captureSettings.forceFrameDestripe = true;
    rc = NEXUS_SimpleVideoDecoder_StartCapture(pContext->hVideoDecoder, &captureSettings);
    if (rc) return BERR_TRACE(rc);

    return 0;
}

static int update_graphics_pip(struct EncodeContext *pEncodeContext, struct DecodeContext *pContext)
{
#define NUM_CAPTURE_SURFACES 2
    NEXUS_SurfaceHandle captureSurface[NUM_CAPTURE_SURFACES];
    NEXUS_SimpleVideoDecoderCaptureStatus captureStatus[NUM_CAPTURE_SURFACES];
    NEXUS_Graphics2DBlitSettings blitSettings;
    unsigned numReturned;
    int rc;

    rc = NEXUS_SimpleVideoDecoder_GetCapturedSurfaces(pContext->hVideoDecoder, captureSurface, captureStatus, NUM_CAPTURE_SURFACES, &numReturned);
    if (rc) return BERR_TRACE(rc);
    if (numReturned==0) {
        /* TODO: avoid busy loop. need to wait on framebuffer callback for flow control. */
        BKNI_Sleep(1);
        return 0;
    }
    if (numReturned > 1) {
        /* if we get more than one, we recycle the oldest immediately */
        NEXUS_SimpleVideoDecoder_RecycleCapturedSurfaces(pContext->hVideoDecoder, captureSurface, numReturned-1);
    }

    NEXUS_Graphics2D_GetDefaultBlitSettings(&blitSettings);
    blitSettings.source.surface = captureSurface[numReturned-1];
    blitSettings.output.surface = pContext->surface[pContext->cur_surface];
    blitSettings.output.rect = pEncodeContext->encoderSettings.pip_window;
    rc = NEXUS_Graphics2D_Blit(pContext->gfx, &blitSettings);
    BDBG_ASSERT(!rc);

    checkpoint(pContext);

    NEXUS_SimpleVideoDecoder_RecycleCapturedSurfaces(pContext->hVideoDecoder, &captureSurface[numReturned-1], 1);

    rc = NEXUS_Display_SetGraphicsFramebuffer(pContext->display, pContext->surface[pContext->cur_surface]);
    if (rc) return BERR_TRACE(rc);
    pContext->cur_surface = 1 - pContext->cur_surface;
    return 0;
}

static void stop_graphics_pip(struct DecodeContext *pContext)
{
    NEXUS_GraphicsSettings graphicsSettings;
    unsigned i;

    NEXUS_Display_GetGraphicsSettings(pContext->display, &graphicsSettings);
    graphicsSettings.enabled = false;
    NEXUS_Display_SetGraphicsSettings(pContext->display, &graphicsSettings);
    NEXUS_Surface_Destroy(pContext->surface[0]);
    NEXUS_Surface_Destroy(pContext->surface[1]);
    for (i=0; i<NEXUS_SIMPLE_DECODER_MAX_SURFACES; i++) {
        NEXUS_Surface_Destroy(pContext->videoSurfaces[i]);
    }
    NEXUS_Graphics2D_Close(pContext->gfx);
    BKNI_DestroyEvent(pContext->checkpointEvent);
}

int main(int argc, const char **argv)  {
    NxClient_JoinSettings joinSettings;
    NxClient_AllocSettings allocSettings;
    NxClient_ConnectSettings connectSettings;
    struct {
        NxClient_AllocResults allocResults;
        unsigned connectId;
    } _main, pip;
    NEXUS_Error rc = 0;
    int curarg = 1;
    NEXUS_RecpumpSettings recpumpSettings;
    NEXUS_RecpumpOpenSettings recpumpOpenSettings;
    BKNI_EventHandle dataReadyEvent;
    struct EncodeContext context;
    struct DecodeContext decodeContext[2];
    unsigned timeout = 0;
    unsigned starttime, thistime;
    bool gui = true;
    brecord_gui_t record_gui = NULL;
    bool realtime = true;
    char encodeName[128];

    BKNI_Memset(&context, 0, sizeof(context));
    BKNI_Memset(&decodeContext, 0, sizeof(decodeContext));
    context.encoderStartSettings.videoPid = context.encoderStartSettings.audioPid = NULL_PID;
    context.encoderSettings.width = 1280;
    context.encoderSettings.height = 720;
    context.encoderSettings.window.width = context.encoderSettings.width;
    context.encoderSettings.window.height = context.encoderSettings.height;
    context.encoderSettings.pip_window.width = context.encoderSettings.width/2;
    context.encoderSettings.pip_window.height = context.encoderSettings.height/2;
    context.encoderSettings.pip_window.x = context.encoderSettings.pip_window.width;

    while (curarg < argc) {
        if (!strcmp(argv[curarg], "-h") || !strcmp(argv[curarg], "--help")) {
            print_usage();
            return 0;
        }
        else if (!strcmp(argv[curarg], "-video") && curarg+1 < argc) {
            context.encoderStartSettings.videoPid = strtoul(argv[++curarg], NULL, 0);
        }
        else if (!strcmp(argv[curarg], "-audio") && curarg+1 < argc) {
            context.encoderStartSettings.audioPid = strtoul(argv[++curarg], NULL, 0);
        }
        else if (!strcmp(argv[curarg], "-audio_samplerate") && curarg+1 < argc) {
            context.encoderStartSettings.audioSampleRate = strtoul(argv[++curarg], NULL, 0);
        }
        else if (!strcmp(argv[curarg], "-video_type") && curarg+1 < argc) {
            context.encoderStartSettings.videoCodec = lookup(g_videoCodecStrs, argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-video_bitrate") && curarg+1 < argc) {
            float rate;
            sscanf(argv[++curarg], "%f", &rate);
            context.encoderSettings.videoBitrate = rate * 1000 * 1000;
        }
        else if (!strcmp(argv[curarg], "-video_size") && curarg+1 < argc) {
            if (sscanf(argv[++curarg], "%u,%u", &context.encoderSettings.width, &context.encoderSettings.height) != 2) {
                print_usage();
                return -1;
            }
        }
        else if (!strcmp(argv[curarg], "-window") && curarg+1 < argc) {
            unsigned x,y,w,h;
            if (sscanf(argv[++curarg], "%u,%u,%u,%u", &x, &y, &w, &h) != 4) {
                print_usage();
                return -1;
            }
            context.encoderSettings.window.x = x;
            context.encoderSettings.window.y = y;
            context.encoderSettings.window.width = w;
            context.encoderSettings.window.height = h;
        }
        else if (!strcmp(argv[curarg], "-pip_window") && curarg+1 < argc) {
            unsigned x,y,w,h;
            if (sscanf(argv[++curarg], "%u,%u,%u,%u", &x, &y, &w, &h) != 4) {
                print_usage();
                return -1;
            }
            context.encoderSettings.pip_window.x = x;
            context.encoderSettings.pip_window.y = y;
            context.encoderSettings.pip_window.width = w;
            context.encoderSettings.pip_window.height = h;
        }
        else if (!strcmp(argv[curarg], "-video_framerate") && curarg+1 < argc) {
            float rate;
            sscanf(argv[++curarg], "%f", &rate);
            context.encoderSettings.frameRate = rate * 1000;
        }
        else if (!strcmp(argv[curarg], "-audio_type") && curarg+1 < argc) {
            context.encoderStartSettings.audioCodec = lookup(g_audioCodecStrs, argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-timeout") && argc>curarg+1) {
            timeout = strtoul(argv[++curarg], NULL, 0);
        }
        else if (!strcmp(argv[curarg], "-loop")) {
            decodeContext[0].loop = decodeContext[1].loop = true;
        }
        else if (!strcmp(argv[curarg], "-gui") && argc>curarg+1) {
            gui = strcmp(argv[++curarg], "off");
        }
        else if (!decodeContext[0].filename) {
            decodeContext[0].filename = argv[curarg];
        }
        else if (!decodeContext[1].filename) {
            decodeContext[1].filename = argv[curarg];
        }
        else if (!context.outputfile) {
            context.outputfile = argv[curarg];
        }
        curarg++;
    }

    if (!decodeContext[0].filename || !decodeContext[1].filename) {
        print_usage();
        return -1;
    }

    NxClient_GetDefaultJoinSettings(&joinSettings);
    snprintf(joinSettings.name, NXCLIENT_MAX_NAME, "%s %s", argv[0], context.outputfile);
    rc = NxClient_Join(&joinSettings);
    if (rc) {
        printf("cannot join: %d\n", rc);
        return -1;
    }

    NxClient_GetDefaultAllocSettings(&allocSettings);
    allocSettings.simpleEncoder = 1;
    allocSettings.simpleVideoDecoder = 1;
    allocSettings.simpleAudioDecoder = 1;
    rc = NxClient_Alloc(&allocSettings, &_main.allocResults);
    if (rc) {BDBG_WRN(("unable to alloc transcode resources")); return -1;}

    NxClient_GetDefaultAllocSettings(&allocSettings);
    allocSettings.simpleVideoDecoder = 1;
    rc = NxClient_Alloc(&allocSettings, &pip.allocResults);
    if (rc) {BDBG_WRN(("unable to alloc pip transcode resources")); return -1;}

    NEXUS_LookupFrameRate(context.encoderSettings.frameRate, &context.encoderSettings.frameRateEnum);
    /* with 1000/1001 rate tracking by default */
    switch (context.encoderSettings.frameRateEnum) {
    case NEXUS_VideoFrameRate_e29_97:  context.encoderSettings.frameRateEnum = NEXUS_VideoFrameRate_e30; break;
    case NEXUS_VideoFrameRate_e59_94:  context.encoderSettings.frameRateEnum = NEXUS_VideoFrameRate_e60; break;
    case NEXUS_VideoFrameRate_e23_976: context.encoderSettings.frameRateEnum = NEXUS_VideoFrameRate_e24; break;
    case NEXUS_VideoFrameRate_e14_985: context.encoderSettings.frameRateEnum = NEXUS_VideoFrameRate_e15; break;
    default: break;
    }

    NxClient_GetDefaultConnectSettings(&connectSettings);
    connectSettings.simpleVideoDecoder[0].id = _main.allocResults.simpleVideoDecoder[0].id;
    connectSettings.simpleVideoDecoder[0].windowCapabilities.encoder = true;
    connectSettings.simpleVideoDecoder[0].decoderCapabilities.maxWidth = 1920;
    connectSettings.simpleVideoDecoder[0].decoderCapabilities.maxHeight = 1080;
    connectSettings.simpleAudioDecoder.id = _main.allocResults.simpleAudioDecoder.id;
    connectSettings.simpleEncoder[0].id = _main.allocResults.simpleEncoder[0].id;
    connectSettings.simpleEncoder[0].nonRealTime = !realtime;
    connectSettings.simpleEncoder[0].encoderCapabilities.maxFrameRate = context.encoderSettings.frameRateEnum;
    rc = NxClient_Connect(&connectSettings, &_main.connectId);
    if (rc) {BDBG_WRN(("unable to connect transcode resources")); return -1;}

    NxClient_GetDefaultConnectSettings(&connectSettings);
    connectSettings.simpleVideoDecoder[0].id = pip.allocResults.simpleVideoDecoder[0].id;
    connectSettings.simpleVideoDecoder[0].windowCapabilities.type = NxClient_VideoWindowType_eNone;
    rc = NxClient_Connect(&connectSettings, &pip.connectId);
    if (rc) {BDBG_WRN(("unable to connect pip transcode resources")); return -1;}

    rc = prestart_decode(&decodeContext[0], &_main.allocResults);
    if (rc) return BERR_TRACE(-1);
    rc = prestart_decode(&decodeContext[1], &pip.allocResults);
    if (rc) return BERR_TRACE(-1);

    BKNI_CreateEvent(&dataReadyEvent);

    context.hEncoder = NEXUS_SimpleEncoder_Acquire(_main.allocResults.simpleEncoder[0].id);
    BDBG_ASSERT(context.hEncoder);

    NEXUS_Recpump_GetDefaultOpenSettings(&recpumpOpenSettings);
    context.hRecpump = NEXUS_Recpump_Open(NEXUS_ANY_ID, &recpumpOpenSettings);
    BDBG_ASSERT(context.hRecpump);

    NEXUS_Recpump_GetSettings(context.hRecpump, &recpumpSettings);
    recpumpSettings.data.dataReady.callback = complete;
    recpumpSettings.data.dataReady.context = dataReadyEvent;
    recpumpSettings.bandHold = !realtime; /* flow control required for NRT mode */
    rc = NEXUS_Recpump_SetSettings(context.hRecpump, &recpumpSettings);
    BDBG_ASSERT(!rc);

    if (!context.outputfile) {
        static char buf[64];
        NEXUS_RecpumpStatus status;
        NEXUS_Recpump_GetStatus(context.hRecpump, &status);
        context.outputfile = buf;
        snprintf(buf, sizeof(buf), "videos/stream%d.mpg", status.rave.index);
    }

    if (gui) {
        struct brecord_gui_settings settings;
        brecord_gui_get_default_settings(&settings);
        settings.sourceName = "main and pip";
        settings.destName = context.outputfile;
        settings.recpump = context.hRecpump;
        settings.color = 0xFF00AAAA;
        record_gui = brecord_gui_create(&settings);
    }

    /* starting encode */
    rc = start_encode(&context, &decodeContext[0]);
    if (rc) return BERR_TRACE(-1);

    rc = start_decode(&decodeContext[0]);
    if (rc) return BERR_TRACE(-1);
    rc = start_decode(&decodeContext[1]);
    if (rc) return BERR_TRACE(-1);

    decodeContext[1].display = context.display;
    decodeContext[1].displayIndex = context.displayIndex;
    rc = start_graphics_pip(&context, &decodeContext[1]);
    if (rc) return BERR_TRACE(-1);

    snprintf(encodeName, sizeof(encodeName), "%s %s -> %s", decodeContext[0].filename, decodeContext[1].filename, context.outputfile);
    BDBG_WRN(("%s: started", encodeName));
    starttime = b_get_time();
    do {
        thistime = b_get_time();
        {
            const void *dataBuffer;
            size_t dataSize=0;
            NEXUS_RecpumpStatus status;

            NEXUS_Recpump_GetStatus(context.hRecpump, &status);

            if(status.started) {
                rc = NEXUS_Recpump_GetDataBuffer(context.hRecpump, &dataBuffer, &dataSize);
                if (rc) {
                    BDBG_ERR(("recpump not internally started"));
                    break;
                }
            }
            if (!dataSize) {
                /* don't sleep, we will [eventually] block on graphics framebuffer */
                thistime = b_get_time();
                goto check_for_end;
            }
            if (record_gui) {
                static unsigned lasttime = 0;
                if (thistime - lasttime > 200) {
                    brecord_gui_update(record_gui);
                    lasttime = thistime;
                }
            }
            if (dataSize) {
                context.streamTotal += dataSize;
                fwrite(dataBuffer, 1, dataSize, context.pOutputFile);
                rc = NEXUS_Recpump_DataReadComplete(context.hRecpump, dataSize);
                BDBG_ASSERT(!rc);
            }
            if (context.lastsize + MB < context.streamTotal) {
                context.lastsize = context.streamTotal;
                BDBG_WRN(("%s: %d MB stream", encodeName, context.streamTotal/MB));
            }
        }
check_for_end:
        update_graphics_pip(&context, &decodeContext[1]);
        while (0);
    } while (!timeout || (thistime - starttime)/1000 < timeout || context.stopped);

    /* encoder thread will exit when EOS is received */
    BDBG_WRN(("%s: stopped", encodeName));

    stop_graphics_pip(&decodeContext[1]);
    stop_decode(&decodeContext[1]);
    stop_decode(&decodeContext[0]);
    stop_encode(&context);

    /* Bring down system */
    if (record_gui) {
        brecord_gui_destroy(record_gui);
    }
    NEXUS_SimpleEncoder_Release(context.hEncoder);
    BKNI_DestroyEvent(dataReadyEvent);
    NEXUS_Recpump_Close(context.hRecpump);

    NxClient_Disconnect(pip.connectId);
    NxClient_Disconnect(_main.connectId);
    NxClient_Free(&pip.allocResults);
    NxClient_Free(&_main.allocResults);
    NxClient_Uninit();
    return rc;
}
#else
#include <stdio.h>
int main(void)
{
    printf("This application is not supported on this platform (needs playback, simple_decoder and stream_mux)!\n");
    return 0;
}
#endif
