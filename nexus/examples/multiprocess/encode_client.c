/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.
 ******************************************************************************/
#if NEXUS_HAS_STREAM_MUX
#include "nexus_platform_client.h"
#include "nexus_simple_video_decoder.h"
#include "nexus_simple_audio_decoder.h"
#include "nexus_simple_encoder.h"
#include "nexus_playback.h"
#include "nexus_file.h"
#include "nexus_core_utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include "bstd.h"
#include "bkni.h"

BDBG_MODULE(encode_client);

static void complete(void *data, int unused)
{
    BSTD_UNUSED(unused);
    BKNI_SetEvent((BKNI_EventHandle)data);
}

static void print_usage(void)
{
    printf(
        "Usage: nexus.client decode_client [OUTPUTFILE]\n"
        "OUTPUTFILE defaults to videos/stream.mpg\n"
        );
}

#if 0
#define FILE_NAME "videos/cnnticker.mpg"
#define TRANSPORT_TYPE NEXUS_TransportType_eTs
#define VIDEO_CODEC NEXUS_VideoCodec_eMpeg2
#define AUDIO_CODEC NEXUS_AudioCodec_eMpeg
#define VIDEO_PID 0x21
#define AUDIO_PID 0x22
#else
#define FILE_NAME "videos/spider_cc.mpg"
#define TRANSPORT_TYPE NEXUS_TransportType_eTs
#define VIDEO_CODEC NEXUS_VideoCodec_eMpeg2
#define AUDIO_CODEC NEXUS_AudioCodec_eAc3
#define VIDEO_PID 0x11
#define AUDIO_PID 0x14
#endif

int main(int argc, char **argv)  {
    NEXUS_ClientConfiguration clientConfig;
    NEXUS_SimpleVideoDecoderHandle videoDecoder;
    NEXUS_SimpleVideoDecoderStartSettings videoProgram;
    NEXUS_SimpleAudioDecoderHandle audioDecoder = NULL;
    NEXUS_SimpleAudioDecoderStartSettings audioProgram;
    NEXUS_FilePlayHandle file;
    NEXUS_PlaypumpHandle playpump;
    NEXUS_PlaybackHandle playback;
    NEXUS_PlaybackSettings playbackSettings;
    NEXUS_PlaybackPidChannelSettings playbackPidSettings;
    NEXUS_SimpleStcChannelHandle stcChannel;
    NEXUS_SimpleStcChannelSettings stcSettings;
    NEXUS_Error rc = 0;
    int curarg = 1;
    NEXUS_SimpleEncoderHandle encoder;
    NEXUS_SimpleEncoderStartSettings startSettings;
    NEXUS_RecpumpHandle recpump;
    NEXUS_RecpumpSettings recpumpSettings;
    BKNI_EventHandle dataReadyEvent;
    FILE *streamOut, *indexOut;
    unsigned streamTotal = 0, indexTotal = 0;

    while (argc > curarg) {
        if (!strcmp(argv[curarg], "-h") || !strcmp(argv[curarg], "--help")) {
            print_usage();
            return 0;
        }
        curarg++;
    }

    rc = NEXUS_Platform_AuthenticatedJoin(NULL);
    if (rc) {
        printf("cannot join: %d\n", rc);
        return -1;
    }
    NEXUS_Platform_GetClientConfiguration(&clientConfig);

    file = NEXUS_FilePlay_OpenPosix(FILE_NAME, NULL);
    if (!file) {
        BDBG_ERR(("can't open file: %s", FILE_NAME));
        rc = -1;
        goto done;
    }

    playpump = NEXUS_Playpump_Open(NEXUS_ANY_ID, NULL);
    BDBG_ASSERT(playpump);
    playback = NEXUS_Playback_Create();
    BDBG_ASSERT(playback);

    stcChannel = NEXUS_SimpleStcChannel_Create(NULL);
    BDBG_ASSERT(stcChannel);
    NEXUS_SimpleStcChannel_GetSettings(stcChannel, &stcSettings);
    stcSettings.modeSettings.Auto.transportType = TRANSPORT_TYPE;
    stcSettings.mode = NEXUS_StcChannelMode_eAuto;
    rc = NEXUS_SimpleStcChannel_SetSettings(stcChannel, &stcSettings);
    BDBG_ASSERT(!rc);

    NEXUS_Playback_GetSettings(playback, &playbackSettings);
    playbackSettings.playpump = playpump;
    playbackSettings.playpumpSettings.transportType = TRANSPORT_TYPE;
    playbackSettings.simpleStcChannel = stcChannel;
    NEXUS_Playback_SetSettings(playback, &playbackSettings);

    /* Bring up audio decoders and outputs */
    if (clientConfig.resources.simpleVideoDecoder.total) {
        videoDecoder = NEXUS_SimpleVideoDecoder_Acquire(clientConfig.resources.simpleVideoDecoder.id[0]);
    }
    else {
        videoDecoder = NULL;
    }
    if (!videoDecoder) {
        BDBG_ERR(("video decoder not available"));
        rc = -1;
        goto done;
    }
    if (clientConfig.resources.simpleAudioDecoder.total) {
        audioDecoder = NEXUS_SimpleAudioDecoder_Acquire(clientConfig.resources.simpleAudioDecoder.id[0]);
        if (!audioDecoder) {
            BDBG_WRN(("audio decoder not available"));
            /* for this example, audio is optional */
        }
    }

    if (audioDecoder) {
        NEXUS_SimpleAudioDecoder_GetDefaultStartSettings(&audioProgram);
        NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
        playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eAudio;
        playbackPidSettings.pidTypeSettings.audio.simpleDecoder = audioDecoder;
        audioProgram.primary.pidChannel = NEXUS_Playback_OpenPidChannel(playback, AUDIO_PID, &playbackPidSettings);
        audioProgram.primary.codec = AUDIO_CODEC;
        NEXUS_SimpleAudioDecoder_SetStcChannel(audioDecoder, stcChannel);
    }

    NEXUS_SimpleVideoDecoder_GetDefaultStartSettings(&videoProgram);
    NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
    playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eVideo;
    playbackPidSettings.pidTypeSettings.video.codec = VIDEO_CODEC;
    playbackPidSettings.pidTypeSettings.video.index = true;
    playbackPidSettings.pidTypeSettings.video.simpleDecoder = videoDecoder;
    videoProgram.settings.pidChannel = NEXUS_Playback_OpenPidChannel(playback, VIDEO_PID, &playbackPidSettings);
    videoProgram.settings.codec = VIDEO_CODEC;
    NEXUS_SimpleVideoDecoder_SetStcChannel(videoDecoder, stcChannel);

    BDBG_WRN(("Starting encode"));
    streamOut = fopen("videos/stream.mpg", "w+");
    BDBG_ASSERT(streamOut);
    indexOut = fopen("videos/stream.sct", "w+");
    BDBG_ASSERT(indexOut);

    BKNI_CreateEvent(&dataReadyEvent);

    recpump = NEXUS_Recpump_Open(NEXUS_ANY_ID, NULL);
    BDBG_ASSERT(recpump);

    NEXUS_Recpump_GetSettings(recpump, &recpumpSettings);
    recpumpSettings.data.dataReady.callback = complete;
    recpumpSettings.data.dataReady.context = dataReadyEvent;
    recpumpSettings.index.dataReady.callback = complete;
    recpumpSettings.index.dataReady.context = dataReadyEvent;
    rc = NEXUS_Recpump_SetSettings(recpump, &recpumpSettings);
    BDBG_ASSERT(!rc);

    encoder = NEXUS_SimpleEncoder_Acquire(0);
    BDBG_ASSERT(encoder);

    NEXUS_SimpleEncoder_GetDefaultStartSettings(&startSettings);
    startSettings.input.video = videoDecoder;
    startSettings.input.audio = audioDecoder;
    startSettings.recpump = recpump;
    startSettings.output.video.index = true;
#if 0
/* Defaults to AVC encoding. Set this for MPEG encoding. */
    startSettings.output.video.settings.codec = NEXUS_VideoCodec_eMpeg2;
    startSettings.output.video.settings.profile = NEXUS_VideoCodecProfile_eMain;
    startSettings.output.video.settings.level = NEXUS_VideoCodecLevel_eMain;
#endif
    rc = NEXUS_SimpleEncoder_Start(encoder, &startSettings);
    BDBG_ASSERT(!rc);

    if (videoProgram.settings.pidChannel) {
        rc = NEXUS_SimpleVideoDecoder_Start(videoDecoder, &videoProgram);
        BDBG_ASSERT(!rc);
    }
    if (audioDecoder && audioProgram.primary.pidChannel) {
        NEXUS_SimpleAudioDecoder_Start(audioDecoder, &audioProgram);
        /* decode may fail if audio codec not supported */
    }
    rc = NEXUS_Playback_Start(playback, file, NULL);
    BDBG_ASSERT(!rc);

    rc = NEXUS_Recpump_Start(recpump);
    BDBG_ASSERT(!rc);

    while (streamTotal < 10*1024*1024) {
        const void *dataBuffer, *indexBuffer;
        size_t dataSize, indexSize;
        rc = NEXUS_Recpump_GetDataBuffer(recpump, &dataBuffer, &dataSize);
        BDBG_ASSERT(!rc);
        rc = NEXUS_Recpump_GetIndexBuffer(recpump, &indexBuffer, &indexSize);
        BDBG_ASSERT(!rc);
        if (!dataSize && !indexSize) {
            rc = BKNI_WaitForEvent(dataReadyEvent, 3000);
            BDBG_ASSERT(!rc);
            continue;
        }
        if (dataSize) {
            streamTotal += dataSize;
            fwrite(dataBuffer, 1, dataSize, streamOut);
            rc = NEXUS_Recpump_DataReadComplete(recpump, dataSize);
            BDBG_ASSERT(!rc);
        }
        if (indexSize) {
            indexTotal += indexSize;
            fwrite(indexBuffer, 1, indexSize, indexOut);
            rc = NEXUS_Recpump_IndexReadComplete(recpump, indexSize);
            BDBG_ASSERT(!rc);
        }
        BDBG_WRN(("%d stream, %d index", streamTotal, indexTotal));
    }

    fclose(streamOut);
    fclose(indexOut);

    NEXUS_SimpleEncoder_Stop(encoder);
    NEXUS_SimpleEncoder_Release(encoder);
    NEXUS_Recpump_Close(recpump);
    BKNI_DestroyEvent(dataReadyEvent);

    BDBG_WRN(("Stopping"));

    /* Bring down system */
    NEXUS_SimpleVideoDecoder_Stop(videoDecoder);
    if (audioDecoder) {
        NEXUS_SimpleAudioDecoder_Stop(audioDecoder);
    }
    NEXUS_Playback_Stop(playback);

    if (videoProgram.settings.pidChannel) {
        NEXUS_Playback_ClosePidChannel(playback, videoProgram.settings.pidChannel);
    }
    if (audioProgram.primary.pidChannel) {
        NEXUS_Playback_ClosePidChannel(playback, audioProgram.primary.pidChannel);
    }
    NEXUS_SimpleVideoDecoder_Release(videoDecoder);
    if (audioDecoder) {
        NEXUS_SimpleAudioDecoder_Release(audioDecoder);
    }

    if (playback) NEXUS_Playback_Destroy(playback);
    if (playpump) NEXUS_Playpump_Close(playpump);

done:
    if (file) NEXUS_FilePlay_Close(file);
    NEXUS_Platform_Uninit();
    return rc;
}
#else
#include <stdio.h>
int main(void) {
    printf("This application not supported on this platform.\n");
    return -1;
}
#endif
