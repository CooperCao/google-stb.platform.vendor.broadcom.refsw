/******************************************************************************
 *    (c)2010-2013 Broadcom Corporation
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
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 *****************************************************************************/
#if NEXUS_HAS_PLAYBACK && NEXUS_HAS_SIMPLE_DECODER && NEXUS_HAS_STREAM_MUX
#include "nxclient.h"
#include "nexus_simple_encoder.h"
#include "nexus_core_utils.h"
#include "media_player.h"
#include "nexus_surface_client.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include "bstd.h"
#include "bkni.h"

#include "namevalue.h"

BDBG_MODULE(encode_display);

static void complete(void *data, int unused)
{
    BSTD_UNUSED(unused);
    BKNI_SetEvent((BKNI_EventHandle)data);
}

static void print_usage(void)
{
    printf(
        "Usage: encode_display [OUTPUTFILE]\n"
        "Default OUTPUTFILE is videos/stream.mpg\n"
        "\n"
        "OPTIONS:\n"
        "  --help or -h for help\n"
        "  -sizelimit SIZE                size limit in MB. 0 is unlimited.\n"
        "  -video_bitrate RATE   output video bitrate in Mbps\n"
        "  -video_size    WIDTH,HEIGHT (default is 1280,720)\n"
        "  -audio_type    output audio codec\n"
    );
}

static NEXUS_VideoFormat lookup_format(unsigned width, unsigned height, bool interlaced)
{
    /* always use 60hz on the display */
    if (width == 1920 && height == 1080) {
        return interlaced?NEXUS_VideoFormat_e1080i:NEXUS_VideoFormat_e1080p;
    }
    else if (width == 1280 && height == 720) {
        return NEXUS_VideoFormat_e720p;
    }
    else {
        return interlaced?NEXUS_VideoFormat_eNtsc:NEXUS_VideoFormat_e480p;
    }
}

int main(int argc, char **argv)  {
    NxClient_JoinSettings joinSettings;
    NxClient_AllocSettings allocSettings;
    NxClient_AllocResults allocResults;
    NxClient_ConnectSettings connectSettings;
    NxClient_DisplaySettings displaySettings;
    unsigned connectId;
    NEXUS_Error rc = 0;
    int curarg = 1;
    unsigned lastsize = 0;
    const char *outputfile = NULL;
    unsigned sizelimit = 10;
    unsigned videoBitrate = 0;
    unsigned width = 720; /* default RTS supports 480p GFD */
    unsigned height = 480;
    bool interlaced = false;
    unsigned refreshRate = 30000; /* 30 Hz */
    NEXUS_VideoFormat format;
    unsigned audioCodec = NEXUS_AudioCodec_eAac;

    while (argc > curarg) {
        if (!strcmp(argv[curarg], "-h") || !strcmp(argv[curarg], "--help")) {
            print_usage();
            return 0;
        }
        else if (!strcmp(argv[curarg], "-sizelimit") && curarg+1<argc) {
            sizelimit = atoi(argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-video_bitrate") && curarg+1 < argc) {
            float rate;
            sscanf(argv[++curarg], "%f", &rate);
            videoBitrate = rate;
        }
        else if (!strcmp(argv[curarg], "-video_size") && curarg+1 < argc) {
            sscanf(argv[++curarg], "%u,%u", &width, &height);
        }
        else if (!strcmp(argv[curarg], "-audio_type") && curarg+1 < argc) {
            ++curarg;
            if (!strcmp(argv[curarg], "none")) {
                audioCodec = NEXUS_AudioCodec_eUnknown;
            }
            else {
                audioCodec = lookup(g_audioCodecStrs, argv[curarg]);
            }
        }
        else if (!strcmp(argv[curarg], "-interlaced")) {
            interlaced = true;
        }
        else if (!strcmp(argv[curarg], "-video_framerate") && curarg+1 < argc) {
            float rate;
            sscanf(argv[++curarg], "%f", &rate);
            refreshRate = rate * 1000;
        }
        else if (!outputfile) {
            outputfile = argv[curarg];
        }
        curarg++;
    }
    if (!outputfile) {
        outputfile = "videos/stream.mpg";
    }

    NxClient_GetDefaultJoinSettings(&joinSettings);
    snprintf(joinSettings.name, NXCLIENT_MAX_NAME, "%s", "encode_display");
    rc = NxClient_Join(&joinSettings);
    if (rc) {
        printf("cannot join: %d\n", rc);
        return -1;
    }

    NxClient_GetDefaultAllocSettings(&allocSettings);
    allocSettings.simpleEncoder = 1;
    allocSettings.surfaceClient = 1;
    rc = NxClient_Alloc(&allocSettings, &allocResults);
    if (rc) return -1;

    NxClient_GetDefaultConnectSettings(&connectSettings);
    connectSettings.simpleEncoder[0].id = allocResults.simpleEncoder[0].id;
    connectSettings.simpleEncoder[0].display = true;
    rc = NxClient_Connect(&connectSettings, &connectId);
    if (rc) return -1;

    if (joinSettings.session != 0) {
        NxClient_GetDisplaySettings(&displaySettings);
        /* TODO: support custom format for display encode. for now, support a limited set of formats. */
        format = lookup_format(width, height, interlaced);
        if ((joinSettings.session != 0) && (format != displaySettings.format)) {
            displaySettings.format = format;
            rc = NxClient_SetDisplaySettings(&displaySettings);
            BDBG_ASSERT(!rc);
        }
        else if ((joinSettings.session == 0) && (displaySettings.slaveDisplay[1].format != format)) {
            /* assume that miracast on session 0 uses a third display */
            displaySettings.slaveDisplay[1].format = format;
            rc = NxClient_SetDisplaySettings(&displaySettings);
            BDBG_ASSERT(!rc);
        }
    }

    BDBG_WRN(("display -> %s: started", outputfile));
    {
        NEXUS_SimpleEncoderHandle encoder;
        NEXUS_SimpleEncoderStartSettings startSettings;
        NEXUS_SimpleEncoderSettings encoderSettings;
        NEXUS_RecpumpHandle recpump;
        NEXUS_RecpumpSettings recpumpSettings;
        BKNI_EventHandle dataReadyEvent;
        FILE *streamOut;
        unsigned streamTotal = 0;

        streamOut = fopen(outputfile, "w+");
        BDBG_ASSERT(streamOut);

        BKNI_CreateEvent(&dataReadyEvent);

        encoder = NEXUS_SimpleEncoder_Acquire(allocResults.simpleEncoder[0].id);
        BDBG_ASSERT(encoder);

        if (videoBitrate || refreshRate) {
            NEXUS_SimpleEncoder_GetSettings(encoder, &encoderSettings);
            if (videoBitrate) {
                encoderSettings.videoEncoder.bitrateMax = videoBitrate * 1024 * 1024;
            }
            if (refreshRate) {
                NEXUS_LookupFrameRate(refreshRate, &encoderSettings.videoEncoder.frameRate);
            }
            rc = NEXUS_SimpleEncoder_SetSettings(encoder, &encoderSettings);
            BDBG_ASSERT(!rc);
        }

        recpump = NEXUS_Recpump_Open(NEXUS_ANY_ID, NULL);
        BDBG_ASSERT(recpump);

        NEXUS_Recpump_GetSettings(recpump, &recpumpSettings);
        recpumpSettings.data.dataReady.callback = complete;
        recpumpSettings.data.dataReady.context = dataReadyEvent;
        recpumpSettings.index.dataReady.callback = complete;
        recpumpSettings.index.dataReady.context = dataReadyEvent;
        rc = NEXUS_Recpump_SetSettings(recpump, &recpumpSettings);
        BDBG_ASSERT(!rc);

        NEXUS_SimpleEncoder_GetDefaultStartSettings(&startSettings);
        startSettings.input.display = true;
        /* smaller size means lower delay */
        startSettings.output.video.settings.bounds.inputDimension.max.width  = width;
        startSettings.output.video.settings.bounds.inputDimension.max.height = height;
        /* higher frame rate has lower delay; TODO: may set encode input min if display refresh rate > encode framerate to reduce latency */
        startSettings.output.video.settings.bounds.inputFrameRate.min        = encoderSettings.videoEncoder.frameRate;
        startSettings.output.video.settings.bounds.outputFrameRate.min       = encoderSettings.videoEncoder.frameRate;
        startSettings.output.video.settings.bounds.streamStructure.max.framesB = 0;
        startSettings.recpump = recpump;
        startSettings.output.audio.codec = audioCodec;
        rc = NEXUS_SimpleEncoder_Start(encoder, &startSettings);
        BDBG_ASSERT(!rc);

        rc = NEXUS_Recpump_Start(recpump);
        BDBG_ASSERT(!rc);

#define MB (1024*1024)
        while (!sizelimit || streamTotal < sizelimit*MB) {
            const void *dataBuffer, *indexBuffer;
            size_t dataSize, indexSize;
            rc = NEXUS_Recpump_GetDataBuffer(recpump, &dataBuffer, &dataSize);
            BDBG_ASSERT(!rc);
            rc = NEXUS_Recpump_GetIndexBuffer(recpump, &indexBuffer, &indexSize);
            BDBG_ASSERT(!rc);
            if (!dataSize && !indexSize) {
                BKNI_WaitForEvent(dataReadyEvent, 1000);
                /* it's normal to timeout on very low bitrate encoding. just keep reading. */
                continue;
            }
            if (dataSize) {
                streamTotal += dataSize;
                fwrite(dataBuffer, 1, dataSize, streamOut);
                rc = NEXUS_Recpump_DataReadComplete(recpump, dataSize);
                BDBG_ASSERT(!rc);
            }
            if (indexSize) {
                rc = NEXUS_Recpump_IndexReadComplete(recpump, indexSize);
                BDBG_ASSERT(!rc);
            }
            if (lastsize + MB < streamTotal) {
                lastsize = streamTotal;
                BDBG_WRN(("display -> %s: %d MB stream", outputfile, streamTotal/MB));
            }
        }
        BDBG_WRN(("display -> %s: stopped at %d MB limit", outputfile, sizelimit));

        fclose(streamOut);

        NEXUS_Recpump_Stop(recpump);
        NEXUS_SimpleEncoder_Stop(encoder);
        NEXUS_SimpleEncoder_Release(encoder);
        BKNI_DestroyEvent(dataReadyEvent);
        NEXUS_Recpump_Close(recpump);
    }

    NxClient_Disconnect(connectId);
    NxClient_Free(&allocResults);
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
