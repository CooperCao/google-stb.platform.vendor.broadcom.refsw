/******************************************************************************
 * Copyright (C) 2008-2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

/* Put decoder into I-frame only mode and capture every I-frame to disk as BMP.
This requires app flow control so that we don't miss any frames if I/O
is slower than the decoder (which is likely).
This also displays the decoded pictures, just for user feedback. This is not likely
in a real use case. */

#include "nexus_platform.h"
#include <stdio.h>
#if NEXUS_HAS_VIDEO_DECODER && NEXUS_HAS_PLAYBACK
#include "nexus_video_decoder.h"
#include "nexus_stc_channel.h"
#include "nexus_display.h"
#include "nexus_video_window.h"
#include "nexus_video_input.h"
#include "nexus_component_output.h"
#include "nexus_composite_output.h"
#if NEXUS_HAS_HDMI_OUTPUT
#include "nexus_hdmi_output.h"
#endif
#include "nexus_playback.h"
#include "nexus_file.h"
#include "nexus_graphics2d.h"

#include "bstd.h"
#include "bkni.h"
#include <string.h>

BDBG_MODULE(capture_i_frames);

/* the following define the input file and its characteristics -- these will vary by input file */
#define FILE_NAME "videos/cnnticker.mpg"
#define TRANSPORT_TYPE NEXUS_TransportType_eTs
#define VIDEO_CODEC NEXUS_VideoCodec_eMpeg2
#define VIDEO_PID 0x21

static void complete(void *data, int unused)
{
    BSTD_UNUSED(unused);
    BKNI_SetEvent((BKNI_EventHandle)data);
}

static int picdecoder_write_bmp(const char *pictureFilename, NEXUS_SurfaceHandle surface, unsigned bpp);

int main(void)
{
    NEXUS_PlatformSettings platformSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_PidChannelHandle videoPidChannel;
    NEXUS_DisplayHandle display;
    NEXUS_VideoWindowHandle window;
    NEXUS_VideoDecoderHandle videoDecoder;
    NEXUS_VideoDecoderStartSettings videoProgram;
    NEXUS_FilePlayHandle file;
    NEXUS_PlaypumpHandle playpump;
    NEXUS_PlaybackHandle playback;
    NEXUS_PlaybackTrickModeSettings trick;
    NEXUS_PlaybackSettings playbackSettings;
    NEXUS_PlaybackPidChannelSettings playbackPidSettings;
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_HdmiOutputStatus hdmiStatus;
    NEXUS_DisplaySettings displaySettings;
#endif
    const char *fname = FILE_NAME;
    NEXUS_Error rc;
    NEXUS_Graphics2DHandle gfx;
    NEXUS_Graphics2DSettings gfxSettings;
    BKNI_EventHandle checkpointEvent, endOfStreamEvent;
    NEXUS_SurfaceHandle surface;
    NEXUS_SurfaceCreateSettings surfaceCreateSettings;

    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    rc = NEXUS_Platform_Init(&platformSettings);
    if (rc) return -1;
    NEXUS_Platform_GetConfiguration(&platformConfig);

    BKNI_CreateEvent(&checkpointEvent);
    BKNI_CreateEvent(&endOfStreamEvent);
    gfx = NEXUS_Graphics2D_Open(0, NULL);
    NEXUS_Graphics2D_GetSettings(gfx, &gfxSettings);
    gfxSettings.checkpointCallback.callback = complete;
    gfxSettings.checkpointCallback.context = checkpointEvent;
    NEXUS_Graphics2D_SetSettings(gfx, &gfxSettings);

    NEXUS_Surface_GetDefaultCreateSettings(&surfaceCreateSettings);
    surfaceCreateSettings.width = 720;
    surfaceCreateSettings.height = 480;
    surface = NEXUS_Surface_Create(&surfaceCreateSettings);

    playpump = NEXUS_Playpump_Open(NEXUS_ANY_ID, NULL);
    BDBG_ASSERT(playpump);
    playback = NEXUS_Playback_Create();
    BDBG_ASSERT(playback);

    file = NEXUS_FilePlay_OpenPosix(fname, NULL);
    if (!file) {
        fprintf(stderr, "can't open file:%s\n", fname);
        return -1;
    }

    NEXUS_Playback_GetSettings(playback, &playbackSettings);
    playbackSettings.playpump = playpump;
    playbackSettings.playpumpSettings.transportType = TRANSPORT_TYPE;
    playbackSettings.startPaused = true; /* allow app to set trick state before first frame is sent */
    playbackSettings.endOfStreamAction = NEXUS_PlaybackLoopMode_ePause;
    playbackSettings.endOfStreamCallback.callback = complete;
    playbackSettings.endOfStreamCallback.context = endOfStreamEvent;
    NEXUS_Playback_SetSettings(playback, &playbackSettings);

    display = NEXUS_Display_Open(0, NULL);
    window = NEXUS_VideoWindow_Open(display, 0);

#if NEXUS_NUM_COMPONENT_OUTPUTS
    NEXUS_Display_AddOutput(display, NEXUS_ComponentOutput_GetConnector(platformConfig.outputs.component[0]));
#endif
#if NEXUS_NUM_COMPOSITE_OUTPUTS
    NEXUS_Display_AddOutput(display, NEXUS_CompositeOutput_GetConnector(platformConfig.outputs.composite[0]));
#endif
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_Display_AddOutput(display, NEXUS_HdmiOutput_GetVideoConnector(platformConfig.outputs.hdmi[0]));
    rc = NEXUS_HdmiOutput_GetStatus(platformConfig.outputs.hdmi[0], &hdmiStatus);
    if ( !rc && hdmiStatus.connected )
    {
        /* If current display format is not supported by monitor, switch to monitor's preferred format.
           If other connected outputs do not support the preferred format, a harmless error will occur. */
        NEXUS_Display_GetSettings(display, &displaySettings);
        if ( !hdmiStatus.videoFormatSupported[displaySettings.format] ) {
            displaySettings.format = hdmiStatus.preferredVideoFormat;
            NEXUS_Display_SetSettings(display, &displaySettings);
        }
    }
#endif

    /* bring up decoder and connect to display */
    videoDecoder = NEXUS_VideoDecoder_Open(0, NULL); /* take default capabilities */
    NEXUS_VideoWindow_AddInput(window, NEXUS_VideoDecoder_GetConnector(videoDecoder));

    NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
    playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eVideo;
    playbackPidSettings.pidTypeSettings.video.codec = VIDEO_CODEC; /* must be told codec for correct handling */
    playbackPidSettings.pidTypeSettings.video.index = true;
    playbackPidSettings.pidTypeSettings.video.decoder = videoDecoder;
    videoPidChannel = NEXUS_Playback_OpenPidChannel(playback, VIDEO_PID, &playbackPidSettings);

    NEXUS_VideoDecoder_GetDefaultStartSettings(&videoProgram);
    videoProgram.codec = VIDEO_CODEC;
    videoProgram.pidChannel = videoPidChannel;
    videoProgram.appDisplayManagement = true; /* app drives flow so that no pictures are missed */
    rc = NEXUS_VideoDecoder_Start(videoDecoder, &videoProgram);
    BDBG_ASSERT(!rc);

    rc = NEXUS_Playback_Start(playback, file, NULL);
    BDBG_ASSERT(!rc);

    /* another option: use playpump and NEXUS_VideoDecoderTrickState with NEXUS_VideoDecoderDecodeMode_eI and tsmEnabled = false */
    NEXUS_Playback_GetDefaultTrickModeSettings(&trick);
    trick.skipControl = NEXUS_PlaybackSkipControl_eDecoder;
    trick.mode = NEXUS_PlaybackHostTrickMode_ePlayI;
    rc = NEXUS_Playback_TrickMode(playback, &trick);
    BDBG_ASSERT(!rc);

    while (1) {
        NEXUS_VideoDecoderFrameStatus frameStatus;
        NEXUS_Graphics2DDestripeBlitSettings blitSettings;
        unsigned num;
        char filename[64];

        rc = NEXUS_VideoDecoder_GetDecodedFrames(videoDecoder, &frameStatus, 1, &num);
        BDBG_ASSERT(!rc);
        if (!num) {
            rc = BKNI_WaitForEvent(endOfStreamEvent, 1);
            if (rc == NEXUS_TIMEOUT) continue;
            break;
        }
        if (frameStatus.pictureCoding != NEXUS_PictureCoding_eI) BDBG_ERR(("unexpected picture coding %u", frameStatus.pictureCoding));

        /* destripe */
        NEXUS_Graphics2D_GetDefaultDestripeBlitSettings(&blitSettings);
        blitSettings.source.stripedSurface = NEXUS_StripedSurface_Create(&frameStatus.surfaceCreateSettings);
        blitSettings.output.surface = surface;
        rc = NEXUS_Graphics2D_DestripeBlit(gfx, &blitSettings);
        BDBG_ASSERT(!rc);
        rc = NEXUS_Graphics2D_Checkpoint(gfx, NULL);
        if (rc==NEXUS_GRAPHICS2D_QUEUED) {
            rc = BKNI_WaitForEvent(checkpointEvent, BKNI_INFINITE);
            BDBG_ASSERT(!rc);
        }
        NEXUS_StripedSurface_Destroy(blitSettings.source.stripedSurface);

        /* write to bmp */
        snprintf(filename, sizeof(filename), "videos/iframe%u.bmp", frameStatus.serialNumber);
        rc = picdecoder_write_bmp(filename, surface, 32);
        BDBG_ASSERT(!rc);
        BDBG_WRN(("writing %s (pts %#x)", filename, frameStatus.pts));

        rc = NEXUS_VideoDecoder_ReturnDecodedFrames(videoDecoder, NULL, 1);
        BDBG_ASSERT(!rc);
    }

    /* Bring down system */
    NEXUS_VideoDecoder_Stop(videoDecoder);
    NEXUS_Playback_Stop(playback);
    NEXUS_FilePlay_Close(file);
    NEXUS_Playback_Destroy(playback);
    NEXUS_Playpump_Close(playpump);
    NEXUS_VideoDecoder_Close(videoDecoder);
    NEXUS_VideoWindow_Close(window);
    NEXUS_Display_Close(display);
    NEXUS_Graphics2D_Close(gfx);
    BKNI_DestroyEvent(checkpointEvent);
    BKNI_DestroyEvent(endOfStreamEvent);
    NEXUS_Surface_Destroy(surface);
    NEXUS_Platform_Uninit();
    return 0;
}

/* picdecoder_write_bmp copied from nexus/nxclient/apps/utils/picdecoder.c */

/* see http://en.wikipedia.org/wiki/BMP_file_format */
/* 14 byte struct requires gcc attribute */
struct __attribute__ ((__packed__)) bmp_header {
    unsigned short int type;
    unsigned int size;
    unsigned short int reserved1, reserved2;
    unsigned int offset;
};

struct bmp_infoheader {
    unsigned int size;
    int width,height;
    unsigned short int planes;
    unsigned short int bits;
    unsigned int compression;
    unsigned int imagesize;
    int xresolution,yresolution;
    unsigned int ncolours;
    unsigned int importantcolours;
};

#define HEADER_SIZE (sizeof(struct bmp_header) + sizeof(struct bmp_infoheader))

static int picdecoder_write_bmp(const char *pictureFilename, NEXUS_SurfaceHandle surface, unsigned bpp)
{
    struct bmp_header header;
    struct bmp_infoheader infoheader;
    NEXUS_SurfaceCreateSettings createSettings;
    NEXUS_SurfaceMemory mem;
    FILE *fout;
    unsigned image_size;
    int rc = 0;
    unsigned rownum;

    NEXUS_Surface_GetCreateSettings(surface, &createSettings);
    NEXUS_Surface_GetMemory(surface, &mem);
    NEXUS_Surface_Flush(surface); /* flush CPU read after DMA write (avoids stale RAC) */
    /* TODO: expand support */
    if (createSettings.pixelFormat != NEXUS_PixelFormat_eA8_R8_G8_B8) {
        return BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }
    if (bpp != 24 && bpp != 32) {
        return BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }

    fout = fopen(pictureFilename, "wb+");
    if (!fout) {
        BDBG_WRN(("unable to create %s", pictureFilename));
        rc = -1;
        goto done;
    }

    image_size = createSettings.width * createSettings.height * (bpp / 8);

    memset(&header, 0, sizeof(header));
    ((unsigned char *)&header.type)[0] = 'B';
    ((unsigned char *)&header.type)[1] = 'M';
    header.size = image_size + HEADER_SIZE;
    header.offset = HEADER_SIZE;
    memset(&infoheader, 0, sizeof(infoheader));
    infoheader.size = sizeof(infoheader);
    infoheader.width = createSettings.width;
    infoheader.height = createSettings.height;
    infoheader.planes = 1;
    infoheader.bits = bpp;
    infoheader.imagesize = image_size;

    fwrite(&header, 1, sizeof(header), fout);
    fwrite(&infoheader, 1, sizeof(infoheader), fout);

    for (rownum=0;rownum<createSettings.height;rownum++) {
        unsigned rowsize = createSettings.width * infoheader.bits/8;
        unsigned char *row = (unsigned char *)mem.buffer + ((createSettings.height-rownum-1) * mem.pitch);
        if (bpp == 24) {
            unsigned i;
            for (i=0;i<(unsigned)createSettings.width*4;i+=4) {
                /* write BGR. TODO: big endian */
                fwrite(&row[i+0], 1, 1, fout);
                fwrite(&row[i+1], 1, 1, fout);
                fwrite(&row[i+2], 1, 1, fout);
            }
        }
        else {
            fwrite(row, 1, rowsize, fout);
        }
        if (rowsize % 4) {
            char pad[3] = {0,0,0};
            fwrite(pad, 1, 4 - rowsize % 4, fout);
        }
    }

done:
    if (fout) {
        fclose(fout);
    }
    return rc;
}
#else
int main(void)
{
    printf("This application is not supported on this platform\n");
    return 0;
}
#endif
