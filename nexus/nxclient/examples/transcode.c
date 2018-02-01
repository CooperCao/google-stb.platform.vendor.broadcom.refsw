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

BDBG_MODULE(transcode_programchange);

typedef struct EncodeContext
{
    NEXUS_SimpleEncoderHandle hEncoder;
    NEXUS_SimpleVideoDecoderHandle hVideoDecoder;
    NEXUS_SimpleAudioDecoderHandle hAudioDecoder;
    NEXUS_PlaybackHandle hPlayback;
    NEXUS_PlaypumpHandle hPlaypump;
    NEXUS_RecpumpHandle hRecpump;
    NEXUS_FilePlayHandle file;
    FILE *pOutputFile;
    size_t videoSize, lastVideoSize;
    size_t audioSize, lastAudioSize;
    unsigned streamTotal, lastsize;
    NEXUS_SimpleEncoderStartSettings startSettings;
    NEXUS_SimpleVideoDecoderStartSettings videoProgram;
    NEXUS_SimpleAudioDecoderStartSettings audioProgram;
}EncodeContext;

#define MB (1024*1024)
#define INPUT_FILENAME "videos/cnnticker.mpg"
#define OUTPUT_FILENAME "videos/stream.mpg"
#define VIDEO_CODEC NEXUS_VideoCodec_eMpeg2
#define AUDIO_CODEC NEXUS_AudioCodec_eMpeg
#define VIDEO_PID 0x21
#define AUDIO_PID 0x22

static void complete(void *data, int unused)
{
    BSTD_UNUSED(unused);
    BKNI_SetEvent((BKNI_EventHandle)data);
}

static int start_encode(EncodeContext *pContext)
{
    NEXUS_SimpleEncoderStartSettings startSettings;
    NEXUS_Error rc;

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

    NEXUS_SimpleEncoder_GetDefaultStartSettings(&startSettings);
    startSettings.input.video = pContext->hVideoDecoder;
    startSettings.input.audio = pContext->hAudioDecoder;
    startSettings.recpump = pContext->hRecpump;
    startSettings.output.video.index = false;
    startSettings.output.video.settings.codec = NEXUS_VideoCodec_eH264;
    startSettings.output.video.pid = 0x11;
    startSettings.output.audio.codec = NEXUS_AudioCodec_eAac;
    startSettings.output.audio.pid = 0x14;

    rc = NEXUS_SimpleEncoder_Start(pContext->hEncoder, &startSettings);
    if (rc) {
        BDBG_ERR(("unable to start encoder"));
        return -1;
    }
    pContext->startSettings = startSettings;

    /* can't start decode before encode because encoder provides display for decoder */
    rc = NEXUS_SimpleVideoDecoder_Start(pContext->hVideoDecoder, &pContext->videoProgram);
    BDBG_ASSERT(!rc);
    rc = NEXUS_SimpleAudioDecoder_Start(pContext->hAudioDecoder, &pContext->audioProgram);
    BDBG_ASSERT(!rc);

    /* recpump must start after decoders start */
    rc = NEXUS_Recpump_Start(pContext->hRecpump);
    BDBG_ASSERT(!rc);
    rc = NEXUS_Playback_Start(pContext->hPlayback, pContext->file, NULL);
    BDBG_ASSERT(!rc);

    return 0;
}

static void stop_encode(EncodeContext *pContext)
{
    NEXUS_Playback_Stop(pContext->hPlayback);
    NEXUS_SimpleVideoDecoder_Stop(pContext->hVideoDecoder);
    NEXUS_SimpleAudioDecoder_Stop(pContext->hAudioDecoder);
    NEXUS_SimpleEncoder_Stop(pContext->hEncoder);
    NEXUS_Recpump_Stop(pContext->hRecpump);
    NEXUS_FilePlay_Close(pContext->file);
    pContext->videoSize=pContext->lastVideoSize=pContext->audioSize=pContext->lastAudioSize=0;
    pContext->streamTotal=pContext->lastsize=0;
}

int main(void)  {
    NxClient_JoinSettings joinSettings;
    NxClient_AllocSettings allocSettings;
    NxClient_AllocResults allocResults;
    NxClient_ConnectSettings connectSettings;
    unsigned connectId;
    NEXUS_PlaypumpOpenSettings openSettings;
    NEXUS_PlaybackSettings playbackSettings;
    NEXUS_PlaybackPidChannelSettings playbackPidSettings;
    NEXUS_SimpleStcChannelHandle stcChannel;
    NEXUS_SimpleStcChannelSettings stcSettings;
    NEXUS_Error rc = 0;
    NEXUS_RecpumpSettings recpumpSettings;
    NEXUS_RecpumpOpenSettings recpumpOpenSettings;
    BKNI_EventHandle dataReadyEvent;
    EncodeContext context;
    bool realtime = false;

    BKNI_Memset(&context, 0, sizeof(context));

    NxClient_GetDefaultJoinSettings(&joinSettings);
    rc = NxClient_Join(&joinSettings);
    if (rc) {
        printf("cannot join: %d\n", rc);
        return -1;
    }

    stcChannel = NEXUS_SimpleStcChannel_Create(NULL);
    BDBG_ASSERT(stcChannel);

    NEXUS_Playpump_GetDefaultOpenSettings(&openSettings);
    openSettings.fifoSize /= 2;
    context.hPlaypump = NEXUS_Playpump_Open(NEXUS_ANY_ID, &openSettings);
    BDBG_ASSERT(context.hPlaypump);
    context.hPlayback = NEXUS_Playback_Create();
    BDBG_ASSERT(context.hPlayback);

    NEXUS_SimpleStcChannel_GetSettings(stcChannel, &stcSettings);
    stcSettings.mode = NEXUS_StcChannelMode_eAuto;
    rc = NEXUS_SimpleStcChannel_SetSettings(stcChannel, &stcSettings);
    if (rc) {BDBG_WRN(("unable to set stcsettings")); return -1;}

    NEXUS_Playback_GetSettings(context.hPlayback, &playbackSettings);
    playbackSettings.playpump = context.hPlaypump;
    playbackSettings.simpleStcChannel = stcChannel;
    playbackSettings.endOfStreamAction = NEXUS_PlaybackLoopMode_ePause;
    NEXUS_Playback_SetSettings(context.hPlayback, &playbackSettings);

    NxClient_GetDefaultAllocSettings(&allocSettings);
    allocSettings.simpleVideoDecoder = 1;
    allocSettings.simpleAudioDecoder = 1;
    allocSettings.simpleEncoder = 1;
    rc = NxClient_Alloc(&allocSettings, &allocResults);
    if (rc) {BDBG_WRN(("unable to alloc transcode resources")); return -1;}

    NxClient_GetDefaultConnectSettings(&connectSettings);
    connectSettings.simpleVideoDecoder[0].id = allocResults.simpleVideoDecoder[0].id;
    connectSettings.simpleVideoDecoder[0].windowCapabilities.encoder = true;
    connectSettings.simpleVideoDecoder[0].decoderCapabilities.supportedCodecs[VIDEO_CODEC] = true;
    connectSettings.simpleAudioDecoder.id = allocResults.simpleAudioDecoder.id;
    connectSettings.simpleEncoder[0].id = allocResults.simpleEncoder[0].id;
    connectSettings.simpleEncoder[0].nonRealTime = !realtime;
    rc = NxClient_Connect(&connectSettings, &connectId);
    if (rc) {BDBG_WRN(("unable to connect transcode resources")); return -1;}

    context.hVideoDecoder = NEXUS_SimpleVideoDecoder_Acquire(allocResults.simpleVideoDecoder[0].id);
    NEXUS_SimpleVideoDecoder_GetDefaultStartSettings(&context.videoProgram);
    NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
    playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eVideo;
    playbackPidSettings.pidTypeSettings.video.codec = VIDEO_CODEC;
    playbackPidSettings.pidTypeSettings.video.index = true;
    playbackPidSettings.pidTypeSettings.video.simpleDecoder = context.hVideoDecoder;
    context.videoProgram.settings.pidChannel = NEXUS_Playback_OpenPidChannel(context.hPlayback, VIDEO_PID, &playbackPidSettings);
    context.videoProgram.settings.codec = VIDEO_CODEC;
    context.videoProgram.maxWidth = 720;
    context.videoProgram.maxHeight = 480;
    NEXUS_SimpleVideoDecoder_SetStcChannel(context.hVideoDecoder, stcChannel);

    context.hAudioDecoder = NEXUS_SimpleAudioDecoder_Acquire(allocResults.simpleAudioDecoder.id);
    NEXUS_SimpleAudioDecoder_GetDefaultStartSettings(&context.audioProgram);
    NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
    playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eAudio;
    playbackPidSettings.pidTypeSettings.audio.simpleDecoder = context.hAudioDecoder;
    context.audioProgram.primary.pidChannel = NEXUS_Playback_OpenPidChannel(context.hPlayback, AUDIO_PID, &playbackPidSettings);
    context.audioProgram.primary.codec = AUDIO_CODEC;
    NEXUS_SimpleAudioDecoder_SetStcChannel(context.hAudioDecoder, stcChannel);

    BKNI_CreateEvent(&dataReadyEvent);

    context.hEncoder = NEXUS_SimpleEncoder_Acquire(allocResults.simpleEncoder[0].id);
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

    /* starting encode */
    if(start_encode(&context) != 0)
        return -1;

    BDBG_WRN(("%s -> %s: started", INPUT_FILENAME, OUTPUT_FILENAME));
    while (1) {
        const void *dataBuffer;
        size_t dataSize;

        rc = NEXUS_Recpump_GetDataBuffer(context.hRecpump, &dataBuffer, &dataSize);
        BDBG_ASSERT(!rc);
        if (!dataSize) {
            rc = BKNI_WaitForEvent(dataReadyEvent, 250);
            if (rc == NEXUS_TIMEOUT) {
                NEXUS_PlaybackStatus status;
                NEXUS_Playback_GetStatus(context.hPlayback, &status);
                if (status.state == NEXUS_PlaybackState_ePaused) {
                    break;
                }
            }
            continue;
        }
        context.streamTotal += dataSize;
        fwrite(dataBuffer, 1, dataSize, context.pOutputFile);
        rc = NEXUS_Recpump_DataReadComplete(context.hRecpump, dataSize);
        BDBG_ASSERT(!rc);
        if (context.lastsize + MB < context.streamTotal) {
            context.lastsize = context.streamTotal;
            BDBG_WRN(("%u MB stream", context.streamTotal/MB));
        }
    }

    stop_encode(&context);

    /* Bring down system */
    NEXUS_SimpleEncoder_Release(context.hEncoder);
    BKNI_DestroyEvent(dataReadyEvent);
    NEXUS_Recpump_Close(context.hRecpump);
    NEXUS_Playback_ClosePidChannel(context.hPlayback, context.videoProgram.settings.pidChannel);
    NEXUS_Playback_ClosePidChannel(context.hPlayback, context.audioProgram.primary.pidChannel);
    NEXUS_SimpleVideoDecoder_Release(context.hVideoDecoder);
    NEXUS_SimpleAudioDecoder_Release(context.hAudioDecoder);
    NEXUS_Playback_Destroy(context.hPlayback);
    NEXUS_Playpump_Close(context.hPlaypump);
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
