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
 *****************************************************************************/
#if NEXUS_HAS_SIMPLE_DECODER
#include "nexus_platform_client.h"
#include "nexus_simple_audio_playback.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "bstd.h"
#include "bkni.h"
#include "bkni_multi.h"
#include "nxclient.h"
#include "wav_file.h"
#include "media_probe.h"
#include "nexus_playback.h"
#include "nexus_audio_decoder.h"
#include "nexus_audio_decode_to_memory.h"
#include "nexus_memory.h"
#include "nexus_audio_playback.h"

BDBG_MODULE(audio_decode_playback);

#define MAX_VOLUME 10

static void print_usage(void)
{
    printf(
        "Usage: audio_decode_playback [file]\n"
        "  -loop\n"
        "  -passive   Don't force HDMI and SPDIF to pcm mode. If compressed, you won't hear anything.\n"
        );
    printf(
        "  -vol {0..%d}\n",
        MAX_VOLUME
        );
}

#define CONVERT_TO_USER_VOL(vol,max_vol) (((vol) - NEXUS_AUDIO_VOLUME_LINEAR_MIN) * (max_vol) / (NEXUS_AUDIO_VOLUME_LINEAR_NORMAL - NEXUS_AUDIO_VOLUME_LINEAR_MIN))
#define CONVERT_TO_NEXUS_LINEAR_VOL(vol,max_vol) (((vol) * (NEXUS_AUDIO_VOLUME_LINEAR_NORMAL - NEXUS_AUDIO_VOLUME_LINEAR_MIN) / (max_vol)) + NEXUS_AUDIO_VOLUME_LINEAR_MIN)

static void complete(void *context, int param)
{
    BDBG_MSG(("%s callback", param?"decoder":"playback"));
    BKNI_SetEvent((BKNI_EventHandle)context);
}

#define NUM_DECODE_BUFFERS (3)

int main(int argc, char **argv)  {
    NxClient_JoinSettings joinSettings;
    NxClient_AllocSettings allocSettings;
    NxClient_AllocResults allocResults;
    NxClient_ConnectSettings connectSettings;
    unsigned connectId;
    NEXUS_SimpleAudioPlaybackHandle audioPlayback;
    NEXUS_SimpleAudioPlaybackStartSettings startSettings;
    NEXUS_AudioDecoderHandle audioDecoder;
    NEXUS_MemoryBlockHandle buffers[NUM_DECODE_BUFFERS];
    void *pBufferMemory[NUM_DECODE_BUFFERS];
    NEXUS_MemoryBlockHandle decodedBuffers[NUM_DECODE_BUFFERS];
    NEXUS_AudioDecoderFrameStatus frameStatus[NUM_DECODE_BUFFERS];
    NEXUS_PlaybackHandle playback;
    NEXUS_PlaypumpHandle playpump;
    NEXUS_PidChannelHandle audioPid;
    NEXUS_FilePlayHandle dataFile;
    BKNI_EventHandle pbEvent, decEvent;
    int rc;
    int curarg = 1;
    bool loop = false;
    const char *filename = NULL;
    bool forcePcm = true;
    unsigned sampleRate = 48000;
    int vol = -1;
    unsigned i;
    struct probe_results probeRes;
    NEXUS_PlaybackSettings pbSettings;
    NEXUS_PlaybackPidChannelSettings pidSettings;
    NEXUS_AudioDecoderOpenSettings decOpenSettings;
    NEXUS_AudioDecoderDecodeToMemorySettings decodeToMemSettings;
    NEXUS_AudioDecoderDecodeToMemoryStatus decodeToMemStatus;
    NEXUS_AudioDecoderStartSettings audioProgram;
    unsigned numBuffers;
    bool pbStarted = false;
    NEXUS_ClientConfiguration clientConfig;

    while (argc > curarg) {
        if (!strcmp(argv[curarg], "-h") || !strcmp(argv[curarg], "--help")) {
            print_usage();
            return 0;
        }
        else if (!strcmp(argv[curarg], "-loop")) {
            loop = true;
        }
        else if (!strcmp(argv[curarg], "-passive")) {
            forcePcm = false;
        }
        else if (!strcmp(argv[curarg], "-sample_rate") && curarg+1<argc) {
            sampleRate = atoi(argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-vol") && curarg+1<argc) {
            vol = atoi(argv[++curarg]);
            if (vol > MAX_VOLUME) vol = MAX_VOLUME;
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

    if ( NULL == filename )
    {
        print_usage();
    }

    /* connect to server and nexus */
    NxClient_GetDefaultJoinSettings(&joinSettings);
    snprintf(joinSettings.name, NXCLIENT_MAX_NAME, "%s", argv[0]);
    rc = NxClient_Join(&joinSettings);
    if (rc) return -1;

    NEXUS_Platform_GetClientConfiguration(&clientConfig);

    rc = probe_media(filename, &probeRes);
    if ( rc )
    {
        BDBG_ERR(("Unable to probe %s", filename));
        return -1;
    }
    if ( probeRes.num_audio == 0 )
    {
        BDBG_ERR(("No audio prorgams found"));
        return -1;
    }

    BKNI_CreateEvent(&pbEvent);
    BKNI_CreateEvent(&decEvent);

    playpump = NEXUS_Playpump_Open(NEXUS_ANY_ID, NULL);
    BDBG_ASSERT(NULL != playpump);
    playback = NEXUS_Playback_Create();
    BDBG_ASSERT(NULL != playback);

    NEXUS_Playback_GetSettings(playback, &pbSettings);
    pbSettings.playpump = playpump;
    pbSettings.endOfStreamAction = loop ? NEXUS_PlaybackLoopMode_eLoop : NEXUS_PlaybackLoopMode_ePause;
    pbSettings.playpumpSettings.transportType = probeRes.transportType;
    pbSettings.playpumpSettings.timestamp.type = probeRes.timestampType;
    pbSettings.enableStreamProcessing = probeRes.enableStreamProcessing;
    rc = NEXUS_Playback_SetSettings(playback, &pbSettings);
    BDBG_ASSERT(0==rc);

    NEXUS_AudioDecoder_GetDefaultOpenSettings(&decOpenSettings);
    decOpenSettings.type = NEXUS_AudioDecoderType_eDecodeToMemory;
    audioDecoder = NEXUS_AudioDecoder_Open(NEXUS_ANY_ID, &decOpenSettings);
    BDBG_ASSERT(audioDecoder);

    NEXUS_AudioDecoder_GetDecodeToMemorySettings(audioDecoder, &decodeToMemSettings);
    decodeToMemSettings.bitsPerSample = 16;
    decodeToMemSettings.numPcmChannels = 2;
    decodeToMemSettings.maxBuffers = NUM_DECODE_BUFFERS;
    decodeToMemSettings.bufferComplete.callback = complete;
    decodeToMemSettings.bufferComplete.context = decEvent;
    decodeToMemSettings.bufferComplete.param = 0;
    rc = NEXUS_AudioDecoder_SetDecodeToMemorySettings(audioDecoder, &decodeToMemSettings);
    BDBG_ASSERT(0==rc);

    NEXUS_Playback_GetDefaultPidChannelSettings(&pidSettings);
    pidSettings.pidSettings.pidType = NEXUS_PidType_eAudio;
    pidSettings.pidSettings.pidTypeSettings.audio.codec = probeRes.audio[0].codec;
    pidSettings.pidTypeSettings.audio.primary = audioDecoder;
    audioPid = NEXUS_Playback_OpenPidChannel(playback, probeRes.audio[0].pid, &pidSettings);
    BDBG_ASSERT(NULL != audioPid);

    dataFile = NEXUS_FilePlay_OpenPosix(filename, probeRes.useStreamAsIndex ? filename : NULL);
    BDBG_ASSERT(dataFile);

    if (forcePcm) {
        NxClient_AudioSettings audioSettings;
        NxClient_GetAudioSettings(&audioSettings);
        audioSettings.hdmi.outputMode = NxClient_AudioOutputMode_ePcm;
        audioSettings.spdif.outputMode = NxClient_AudioOutputMode_ePcm;
        NxClient_SetAudioSettings(&audioSettings);
    }

    NxClient_GetDefaultAllocSettings(&allocSettings);
    allocSettings.simpleAudioPlayback = 1;
    rc = NxClient_Alloc(&allocSettings, &allocResults);
    if (rc) {rc = BERR_TRACE(rc); goto err_request;}

    audioPlayback = NEXUS_SimpleAudioPlayback_Acquire(allocResults.simpleAudioPlayback[0].id);
    if (!audioPlayback) {
        BDBG_ERR(("unable to acquire audio playback"));
        goto err_acquire;
    }

    if (vol != -1) {
        NEXUS_SimpleAudioPlaybackSettings settings;
        NEXUS_SimpleAudioPlayback_GetSettings(audioPlayback, &settings);
        settings.rightVolume = settings.leftVolume = CONVERT_TO_NEXUS_LINEAR_VOL(vol, MAX_VOLUME);
        NEXUS_SimpleAudioPlayback_SetSettings(audioPlayback, &settings);
    }

    /* connect client resources to server's resources */
    NxClient_GetDefaultConnectSettings(&connectSettings);
    connectSettings.simpleAudioPlayback[0].id = allocResults.simpleAudioPlayback[0].id;
    rc = NxClient_Connect(&connectSettings, &connectId);
    if (rc) {rc = BERR_TRACE(rc); goto err_connect;}

    /* Allocate memory buffers */
    NEXUS_AudioDecoder_GetDecodeToMemoryStatus(audioDecoder, &decodeToMemStatus);
    for ( i = 0; i < NUM_DECODE_BUFFERS; i++ )
    {
        buffers[i] = NEXUS_MemoryBlock_Allocate(clientConfig.heap[1], decodeToMemStatus.bufferSize, 4, NULL);
        BDBG_ASSERT(NULL != buffers[i]);
        rc = NEXUS_MemoryBlock_Lock(buffers[i], &pBufferMemory[i]);
        BDBG_ASSERT(0==rc);
    }

    /* Start decoder and wait for first frame */
    NEXUS_AudioDecoder_GetDefaultStartSettings(&audioProgram);
    audioProgram.pidChannel = audioPid;
    audioProgram.codec = probeRes.audio[0].codec;
    audioProgram.targetSyncEnabled = false; /* Make sure decoder will process all frames including last */
    rc = NEXUS_AudioDecoder_Start(audioDecoder, &audioProgram);
    BDBG_ASSERT(0==rc);

    for ( i = 0; i < NUM_DECODE_BUFFERS; i++ )
    {
        rc = NEXUS_AudioDecoder_QueueBuffer(audioDecoder, buffers[i], 0, decodeToMemStatus.bufferSize);
        BDBG_ASSERT(0==rc);
    }

    rc = NEXUS_Playback_Start(playback, dataFile, NULL);
    BDBG_ASSERT(0==rc);

    for ( ;; )
    {
        NEXUS_SimpleAudioPlaybackStatus pbStatus;
        size_t bytesWritten, bytesAvailable;
        void *pPlaybackBuffer, *pFrameBuffer;
        /* Wait for decoded frame to be ready */
        do
        {
            rc = NEXUS_AudioDecoder_GetDecodedFrames(audioDecoder, decodedBuffers, frameStatus, NUM_DECODE_BUFFERS, &numBuffers);
            if ( 0 == rc && 0 == numBuffers )
            {
                BDBG_MSG(("No decoded frames, waiting..."));
                rc = BKNI_WaitForEvent(decEvent, 5000);
            }
        } while (0 == rc && 0 == numBuffers);
        if ( rc )
        {
            BDBG_WRN(("Timeout waiting for audio data (EOS?).  Stopping."));
            break;
        }

        BDBG_MSG(("Decoded frame received - PTS %#x length %u bytes", frameStatus[0].pts, (unsigned)frameStatus[0].filledBytes));

        if ( !pbStarted )
        {
            NEXUS_SimpleAudioPlayback_GetDefaultStartSettings(&startSettings);
            startSettings.bitsPerSample = 16;
            startSettings.sampleRate = frameStatus[0].sampleRate;
            startSettings.stereo = true;
            startSettings.dataCallback.callback = complete;
            startSettings.dataCallback.context = pbEvent;
            startSettings.dataCallback.param = 1;
            startSettings.startThreshold = frameStatus[0].filledBytes + 1;  /* Wait for next frame to arrive before kicking off HW */
            BDBG_LOG(("Starting playback for %u Hz", frameStatus[0].sampleRate));
            rc = NEXUS_SimpleAudioPlayback_Start(audioPlayback, &startSettings);
            if ( rc )
            {
                BDBG_ERR(("Error starting playback"));
                break;
            }
            pbStarted = true;
        }

        /* Wait for sufficient space in playback buffer */
        do
        {
            rc=0;
            NEXUS_SimpleAudioPlayback_GetStatus(audioPlayback, &pbStatus);
            BDBG_MSG(("Playback buffer %u/%u (%u) need %u+1024", (unsigned)pbStatus.queuedBytes, (unsigned)pbStatus.fifoSize, (unsigned)(pbStatus.fifoSize - pbStatus.queuedBytes), (unsigned)frameStatus[0].filledBytes));
            if ( (pbStatus.fifoSize - pbStatus.queuedBytes) < (frameStatus[0].filledBytes+1024) )
            {
                BDBG_MSG(("Wait for PB space"));
                rc = BKNI_WaitForEvent(pbEvent, 5000);
            }
        } while (rc==0 && (pbStatus.fifoSize - pbStatus.queuedBytes) < (frameStatus[0].filledBytes+1024));
        if ( rc )
        {
            BDBG_ERR(("Timeout waiting for playback space to become available"));
            break;
        }

        rc = NEXUS_MemoryBlock_Lock(decodedBuffers[0], &pFrameBuffer);
        BDBG_ASSERT(0==rc);
        NEXUS_FlushCache(pFrameBuffer, frameStatus[0].filledBytes);

        /* Copy data into playback buffer */
        bytesWritten = 0;
        while ( bytesWritten < frameStatus[0].filledBytes )
        {
            size_t bytesToWrite;

            rc = NEXUS_SimpleAudioPlayback_GetBuffer(audioPlayback, &pPlaybackBuffer, &bytesAvailable);
            BDBG_ASSERT(0==rc);
            BDBG_ASSERT(bytesAvailable > 0);

            bytesToWrite = frameStatus[0].filledBytes - bytesWritten;
            if ( bytesAvailable < bytesToWrite )
            {
                bytesToWrite = bytesAvailable;
            }
            BKNI_Memcpy(pPlaybackBuffer, (uint8_t *)pFrameBuffer + bytesWritten, bytesToWrite);
            bytesWritten += bytesToWrite;
            rc = NEXUS_SimpleAudioPlayback_WriteComplete(audioPlayback, bytesToWrite);
            BDBG_ASSERT(0==rc);
        }

        NEXUS_MemoryBlock_Unlock(decodedBuffers[0]);
        /* Consume data from decoder */
        rc = NEXUS_AudioDecoder_ConsumeDecodedFrames(audioDecoder, 1);
        BDBG_ASSERT(0==rc);
        /* Requeue buffer */
        rc = NEXUS_AudioDecoder_QueueBuffer(audioDecoder, decodedBuffers[0], 0, decodeToMemStatus.bufferSize);
        BDBG_ASSERT(0==rc);
    }

    if ( pbStarted )
    {
        NEXUS_SimpleAudioPlayback_Stop(audioPlayback);
    }
    NEXUS_Playback_Stop(playback);
    NEXUS_AudioDecoder_Stop(audioDecoder);
    NEXUS_Playback_ClosePidChannel(playback, audioPid);
    NEXUS_Playback_Destroy(playback);
    NEXUS_Playpump_Close(playpump);
    NEXUS_AudioDecoder_Close(audioDecoder);
    for ( i = 0; i < NUM_DECODE_BUFFERS; i++ )
    {
        NEXUS_MemoryBlock_Unlock(buffers[i]);
        NEXUS_MemoryBlock_Free(buffers[i]);
    }

    BKNI_DestroyEvent(pbEvent);
    BKNI_DestroyEvent(decEvent);
    if (connectId) {
        NxClient_Disconnect(connectId);
    }
err_connect:
    NEXUS_SimpleAudioPlayback_Release(audioPlayback);
err_acquire:
    NxClient_Free(&allocResults);
err_request:
    NxClient_Uninit();

    return 0;
}
#else
#include <stdio.h>
int main(void)
{
    printf("This application is not supported on this platform\n");
    return 0;
}
#endif
