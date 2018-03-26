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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>
#include "bstd.h"
#include "bkni.h"

BDBG_MODULE(transcode_with_local_decode);

#define MB (1024*1024)
#define INPUT_FILENAME "videos/cnnticker.mpg"
#define OUTPUT_FILENAME "videos/stream.mpg"
#define VIDEO_CODEC NEXUS_VideoCodec_eMpeg2
#define AUDIO_CODEC NEXUS_AudioCodec_eMpeg
#define VIDEO_PID 0x21
#define AUDIO_PID 0x22

struct TranscodeContext
{
    NEXUS_SimpleEncoderHandle encoder;
    NEXUS_SimpleVideoDecoderHandle videoDecoder;
    NEXUS_SimpleAudioDecoderHandle audioDecoder;
    NEXUS_PlaybackHandle playback;
    NEXUS_PlaypumpHandle playpump;
    NEXUS_RecpumpHandle recpump;
    NEXUS_FilePlayHandle file;
    FILE *pOutputFile;
    NEXUS_SimpleEncoderStartSettings startSettings;
    NEXUS_SimpleVideoDecoderStartSettings videoProgram;
    NEXUS_SimpleAudioDecoderStartSettings audioProgram;
    unsigned connectId;
    NxClient_AllocResults allocResults;
    NEXUS_SimpleStcChannelHandle stcChannel;
    size_t videoSize, lastVideoSize;
    size_t audioSize, lastAudioSize;
    unsigned streamTotal, lastsize;
};

static void stub_complete(void *context, int param)
{
    BSTD_UNUSED(context);
    BSTD_UNUSED(param);
}

static int start_transcode(struct TranscodeContext *pContext)
{
    NEXUS_Error rc;
    NxClient_AllocSettings allocSettings;
    NxClient_ConnectSettings connectSettings;
    NEXUS_PlaypumpOpenSettings openSettings;
    NEXUS_PlaybackSettings playbackSettings;
    NEXUS_PlaybackPidChannelSettings playbackPidSettings;
    NEXUS_SimpleStcChannelSettings stcSettings;
    NEXUS_RecpumpSettings recpumpSettings;

    pContext->stcChannel = NEXUS_SimpleStcChannel_Create(NULL);
    NEXUS_SimpleStcChannel_GetSettings(pContext->stcChannel, &stcSettings);
    stcSettings.mode = NEXUS_StcChannelMode_eAuto;
    NEXUS_SimpleStcChannel_SetSettings(pContext->stcChannel, &stcSettings);

    NEXUS_Playpump_GetDefaultOpenSettings(&openSettings);
    openSettings.fifoSize /= 2;
    pContext->playpump = NEXUS_Playpump_Open(NEXUS_ANY_ID, &openSettings);
    pContext->playback = NEXUS_Playback_Create();

    NEXUS_Playback_GetSettings(pContext->playback, &playbackSettings);
    playbackSettings.playpump = pContext->playpump;
    playbackSettings.simpleStcChannel = pContext->stcChannel;
    playbackSettings.endOfStreamAction = NEXUS_PlaybackLoopMode_eLoop;
    NEXUS_Playback_SetSettings(pContext->playback, &playbackSettings);

    NxClient_GetDefaultAllocSettings(&allocSettings);
    allocSettings.simpleVideoDecoder = 1;
    allocSettings.simpleAudioDecoder = 1;
    allocSettings.simpleEncoder = 1;
    rc = NxClient_Alloc(&allocSettings, &pContext->allocResults);
    if (rc) {BDBG_WRN(("unable to alloc transcode resources")); return -1;}

    NxClient_GetDefaultConnectSettings(&connectSettings);
    connectSettings.simpleVideoDecoder[0].id = pContext->allocResults.simpleVideoDecoder[0].id;
    connectSettings.simpleVideoDecoder[0].windowCapabilities.encoder = true;
    connectSettings.simpleVideoDecoder[0].decoderCapabilities.supportedCodecs[VIDEO_CODEC] = true;
    connectSettings.simpleAudioDecoder.id = pContext->allocResults.simpleAudioDecoder.id;
    connectSettings.simpleEncoder[0].id = pContext->allocResults.simpleEncoder[0].id;
    connectSettings.simpleEncoder[0].nonRealTime = true;
    rc = NxClient_Connect(&connectSettings, &pContext->connectId);
    if (rc) {BDBG_WRN(("unable to connect transcode resources")); return -1;}

    pContext->videoDecoder = NEXUS_SimpleVideoDecoder_Acquire(pContext->allocResults.simpleVideoDecoder[0].id);
    NEXUS_SimpleVideoDecoder_GetDefaultStartSettings(&pContext->videoProgram);
    NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
    playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eVideo;
    playbackPidSettings.pidTypeSettings.video.codec = VIDEO_CODEC;
    playbackPidSettings.pidTypeSettings.video.index = true;
    playbackPidSettings.pidTypeSettings.video.simpleDecoder = pContext->videoDecoder;
    pContext->videoProgram.settings.pidChannel = NEXUS_Playback_OpenPidChannel(pContext->playback, VIDEO_PID, &playbackPidSettings);
    pContext->videoProgram.settings.codec = VIDEO_CODEC;
    pContext->videoProgram.maxWidth = 720;
    pContext->videoProgram.maxHeight = 480;

    pContext->audioDecoder = NEXUS_SimpleAudioDecoder_Acquire(pContext->allocResults.simpleAudioDecoder.id);
    NEXUS_SimpleAudioDecoder_GetDefaultStartSettings(&pContext->audioProgram);
    NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
    playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eAudio;
    playbackPidSettings.pidTypeSettings.audio.simpleDecoder = pContext->audioDecoder;
    pContext->audioProgram.primary.pidChannel = NEXUS_Playback_OpenPidChannel(pContext->playback, AUDIO_PID, &playbackPidSettings);
    pContext->audioProgram.primary.codec = AUDIO_CODEC;

    NEXUS_SimpleVideoDecoder_SetStcChannel(pContext->videoDecoder, pContext->stcChannel);
    NEXUS_SimpleAudioDecoder_SetStcChannel(pContext->audioDecoder, pContext->stcChannel);

    pContext->encoder = NEXUS_SimpleEncoder_Acquire(pContext->allocResults.simpleEncoder[0].id);
    BDBG_ASSERT(pContext->encoder);

    pContext->recpump = NEXUS_Recpump_Open(NEXUS_ANY_ID, NULL);
    BDBG_ASSERT(pContext->recpump);

    NEXUS_Recpump_GetSettings(pContext->recpump, &recpumpSettings);
    recpumpSettings.data.dataReady.callback = stub_complete; /* we're going to poll, but this callback is required */
    recpumpSettings.bandHold = true; /* flow control required for NRT mode */
    rc = NEXUS_Recpump_SetSettings(pContext->recpump, &recpumpSettings);
    BDBG_ASSERT(!rc);

    pContext->file = NEXUS_FilePlay_OpenPosix(INPUT_FILENAME, NULL);
    if (!pContext->file) {
        BDBG_ERR(("can't open files: %s", INPUT_FILENAME));
        return -1;
    }
    pContext->pOutputFile = fopen(OUTPUT_FILENAME, "w+");
    if (!pContext->pOutputFile) {
        BDBG_ERR(("unable to open '%s' for writing", OUTPUT_FILENAME));
        return -1;
    }

    NEXUS_SimpleEncoder_GetDefaultStartSettings(&pContext->startSettings);
    pContext->startSettings.input.video = pContext->videoDecoder;
    pContext->startSettings.input.audio = pContext->audioDecoder;
    pContext->startSettings.recpump = pContext->recpump;
    pContext->startSettings.output.video.index = false;
    pContext->startSettings.output.video.settings.codec = NEXUS_VideoCodec_eH264;
    pContext->startSettings.output.video.pid = 0x11;
    pContext->startSettings.output.audio.codec = NEXUS_AudioCodec_eAac;
    pContext->startSettings.output.audio.pid = 0x14;

    rc = NEXUS_SimpleEncoder_Start(pContext->encoder, &pContext->startSettings);
    if (rc) {
        BDBG_ERR(("unable to start encoder"));
        return -1;
    }

    /* can't start decode before encode because encoder provides display for decoder */
    rc = NEXUS_SimpleVideoDecoder_Start(pContext->videoDecoder, &pContext->videoProgram);
    BDBG_ASSERT(!rc);
    rc = NEXUS_SimpleAudioDecoder_Start(pContext->audioDecoder, &pContext->audioProgram);
    BDBG_ASSERT(!rc);

    /* recpump must start after decoders start */
    rc = NEXUS_Recpump_Start(pContext->recpump);
    BDBG_ASSERT(!rc);
    rc = NEXUS_Playback_Start(pContext->playback, pContext->file, NULL);
    BDBG_ASSERT(!rc);

    return 0;
}

static void stop_transcode(struct TranscodeContext *pContext)
{
    NEXUS_Playback_Stop(pContext->playback);
    NEXUS_SimpleVideoDecoder_Stop(pContext->videoDecoder);
    NEXUS_SimpleAudioDecoder_Stop(pContext->audioDecoder);
    NEXUS_SimpleEncoder_Stop(pContext->encoder);
    NEXUS_Recpump_Stop(pContext->recpump);
    NEXUS_FilePlay_Close(pContext->file);

    NEXUS_SimpleEncoder_Release(pContext->encoder);
    NEXUS_Recpump_Close(pContext->recpump);
    NEXUS_Playback_ClosePidChannel(pContext->playback, pContext->videoProgram.settings.pidChannel);
    NEXUS_Playback_ClosePidChannel(pContext->playback, pContext->audioProgram.primary.pidChannel);
    NEXUS_SimpleVideoDecoder_Release(pContext->videoDecoder);
    NEXUS_SimpleAudioDecoder_Release(pContext->audioDecoder);
    NEXUS_Playback_Destroy(pContext->playback);
    NEXUS_Playpump_Close(pContext->playpump);
    NxClient_Disconnect(pContext->connectId);
    NxClient_Free(&pContext->allocResults);

    pContext->videoSize=pContext->lastVideoSize=pContext->audioSize=pContext->lastAudioSize=0;
    pContext->streamTotal=pContext->lastsize=0;
}

struct DecodeContext
{
    NEXUS_SimpleVideoDecoderHandle videoDecoder;
    NEXUS_SimpleAudioDecoderHandle audioDecoder;
    NEXUS_PlaypumpHandle playpump;
    NEXUS_SimpleVideoDecoderStartSettings videoProgram;
    NEXUS_SimpleAudioDecoderStartSettings audioProgram;
    NEXUS_SimpleStcChannelHandle stcChannel;
    NxClient_AllocResults allocResults;
    unsigned connectId;
};

static int start_local_decode(struct DecodeContext *pContext, const NEXUS_SimpleEncoderStartSettings *startSettings)
{
    NxClient_ConnectSettings connectSettings;
    NxClient_AllocSettings allocSettings;
    NEXUS_SimpleStcChannelSettings stcSettings;
    int rc;

    pContext->playpump = NEXUS_Playpump_Open(NEXUS_ANY_ID, NULL);

    pContext->stcChannel = NEXUS_SimpleStcChannel_Create(NULL);
    NEXUS_SimpleStcChannel_GetSettings(pContext->stcChannel, &stcSettings);
    stcSettings.mode = NEXUS_StcChannelMode_eAuto;
    NEXUS_SimpleStcChannel_SetSettings(pContext->stcChannel, &stcSettings);

    NxClient_GetDefaultAllocSettings(&allocSettings);
    allocSettings.simpleVideoDecoder = 1;
    allocSettings.simpleAudioDecoder = 1;
    rc = NxClient_Alloc(&allocSettings, &pContext->allocResults);
    if (rc) {BDBG_WRN(("unable to alloc transcode resources")); return -1;}

    NxClient_GetDefaultConnectSettings(&connectSettings);
    connectSettings.simpleVideoDecoder[0].id = pContext->allocResults.simpleVideoDecoder[0].id;
    connectSettings.simpleVideoDecoder[0].decoderCapabilities.supportedCodecs[startSettings->output.video.settings.codec] = true;
    connectSettings.simpleAudioDecoder.id = pContext->allocResults.simpleAudioDecoder.id;
    rc = NxClient_Connect(&connectSettings, &pContext->connectId);
    if (rc) {BDBG_WRN(("unable to connect transcode resources")); return -1;}

    pContext->videoDecoder = NEXUS_SimpleVideoDecoder_Acquire(pContext->allocResults.simpleVideoDecoder[0].id);
    NEXUS_SimpleVideoDecoder_GetDefaultStartSettings(&pContext->videoProgram);
    pContext->videoProgram.settings.pidChannel = NEXUS_Playpump_OpenPidChannel(pContext->playpump, startSettings->output.video.pid, NULL);
    pContext->videoProgram.settings.codec = startSettings->output.video.settings.codec;

    pContext->audioDecoder = NEXUS_SimpleAudioDecoder_Acquire(pContext->allocResults.simpleAudioDecoder.id);
    NEXUS_SimpleAudioDecoder_GetDefaultStartSettings(&pContext->audioProgram);
    pContext->audioProgram.primary.pidChannel = NEXUS_Playpump_OpenPidChannel(pContext->playpump, startSettings->output.audio.pid, NULL);
    pContext->audioProgram.primary.codec = startSettings->output.audio.codec;

    NEXUS_SimpleVideoDecoder_SetStcChannel(pContext->videoDecoder, pContext->stcChannel);
    NEXUS_SimpleAudioDecoder_SetStcChannel(pContext->audioDecoder, pContext->stcChannel);

    rc = NEXUS_SimpleVideoDecoder_Start(pContext->videoDecoder, &pContext->videoProgram);
    BDBG_ASSERT(!rc);
    rc = NEXUS_SimpleAudioDecoder_Start(pContext->audioDecoder, &pContext->audioProgram);
    BDBG_ASSERT(!rc);
    rc = NEXUS_Playpump_Start(pContext->playpump);
    BDBG_ASSERT(!rc);

    return 0;
}

static void feed_local_decode(struct DecodeContext *pContext, const void *dataBuffer, unsigned dataSize)
{
    while (dataSize) {
        void *buffer;
        size_t size;
        int rc;
        rc = NEXUS_Playpump_GetBuffer(pContext->playpump, &buffer, &size);
        BDBG_ASSERT(!rc);
        if (!size) {
            BKNI_Sleep(1);
            continue;
        }
        if (size > dataSize) {
            size = dataSize;
        }
        BKNI_Memcpy(buffer, dataBuffer, size);
        rc = NEXUS_Playpump_WriteComplete(pContext->playpump, 0, size);
        BDBG_ASSERT(!rc);
        dataSize -= size;
        dataBuffer = &((uint8_t*)dataBuffer)[size];
    }
}

static void stop_local_decode(struct DecodeContext *pContext)
{
    NEXUS_Playpump_Stop(pContext->playpump);
    NEXUS_SimpleVideoDecoder_Stop(pContext->videoDecoder);
    NEXUS_SimpleAudioDecoder_Stop(pContext->audioDecoder);
    NEXUS_Playpump_Close(pContext->playpump);
    NxClient_Disconnect(pContext->connectId);
    NxClient_Free(&pContext->allocResults);
}

int main(void)  {
    NxClient_JoinSettings joinSettings;
    int rc;
    struct TranscodeContext context;
    struct DecodeContext decode_context;

    BKNI_Memset(&context, 0, sizeof(context));

    NxClient_GetDefaultJoinSettings(&joinSettings);
    rc = NxClient_Join(&joinSettings);
    if (rc) {
        printf("cannot join: %d\n", rc);
        return -1;
    }

    if(start_transcode(&context) != 0)
        return -1;

    if(start_local_decode(&decode_context, &context.startSettings) != 0)
        return -1;

    BDBG_WRN(("%s -> %s: started", INPUT_FILENAME, OUTPUT_FILENAME));
    while (context.streamTotal < 100 * MB) {
        const void *dataBuffer;
        size_t dataSize;

        rc = NEXUS_Recpump_GetDataBuffer(context.recpump, &dataBuffer, &dataSize);
        BDBG_ASSERT(!rc);
        if (!dataSize) {
            BKNI_Sleep(1);
            continue;
        }
        context.streamTotal += dataSize;

        /* write transcode output to disk */
        fwrite(dataBuffer, 1, dataSize, context.pOutputFile);

        /* and write transcode output to playpump for local decode */
        feed_local_decode(&decode_context, dataBuffer, dataSize);

        rc = NEXUS_Recpump_DataReadComplete(context.recpump, dataSize);
        BDBG_ASSERT(!rc);

        if (context.lastsize + MB < context.streamTotal) {
            context.lastsize = context.streamTotal;
            BDBG_WRN(("%u MB stream", context.streamTotal/MB));
        }
    }

    stop_local_decode(&decode_context);
    stop_transcode(&context);

    NxClient_Uninit();
    return rc;
}
#else
#include <stdio.h>
int main(void)
{
    printf("This application is not supported on this platform\n");
    return 0;
}
#endif
