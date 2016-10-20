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
 *****************************************************************************/
#if NEXUS_HAS_SIMPLE_DECODER
#include "nxclient.h"
#include "nexus_platform.h"
#include "nexus_simple_video_decoder.h"
#include "nexus_video_decoder.h"
#include "nexus_simple_audio_decoder.h"
#include "nexus_audio_decoder.h"
#include "nexus_audio_processing_types.h"
#include "nexus_stc_channel.h"
#include "nexus_video_input.h"
#include "nexus_component_output.h"
#include "nexus_composite_output.h"
#if NEXUS_HAS_HDMI_OUTPUT
#include "nexus_hdmi_output.h"
#endif
#include "nexus_playpump.h"

#include <stdio.h>
#include <assert.h>
#include <pthread.h>
#include "bstd.h"
#include "bkni.h"
#include "bkni_multi.h"

/* the following define the input file and its characteristics -- these will vary by input file */
#define FILE_NAME "videos/bugs_toys2_jurassic_q64_cd.mpg"
#define VIDEO_CODEC NEXUS_VideoCodec_eMpeg2
#define VIDEO_PID 0x11
#define AUDIO_CODEC NEXUS_AudioCodec_eAc3
#define AUDIO_PID 0x14
#define AUDIO_PID2 0x24

#define FADE_DURATION       20000

BDBG_MODULE(audio_fade);

static void play_callback(void *context, int param)
{
    BSTD_UNUSED(param);
    BKNI_SetEvent((BKNI_EventHandle)context);
}

#define TOTAL_BUFFERS 10
static void *buf[TOTAL_BUFFERS];
static size_t buf_size = 128*1024;
static unsigned cur_buf = 0;
static FILE *file = NULL;
static bool done = false;
static NEXUS_SimpleVideoDecoderHandle videoDecoder = NULL;
static NEXUS_PlaypumpHandle playpump = NULL;
static NEXUS_PlaypumpSettings playpumpSettings;

static void *playback_thread(void *pParam)
{
    BSTD_UNUSED(pParam);
    while ( !done )
    {
        NEXUS_Error rc;
        int n;
        n = fread(buf[cur_buf], 1, buf_size, file);
        if (n < 0) goto error;
        if (n == 0) {
            /* wait for the decoder to reach the end of the content before looping */
            while (1) {
                NEXUS_VideoDecoderStatus status;
                NEXUS_SimpleVideoDecoder_GetStatus(videoDecoder, &status);
                if (!status.queueDepth) break;
            }
            fseek(file, 0, SEEK_SET);
            NEXUS_Playpump_Flush(playpump);
        }
        else {
            NEXUS_PlaypumpScatterGatherDescriptor desc;
            size_t numConsumed = 0;
            desc.addr = buf[cur_buf];
            desc.length = n;
            if (playpumpSettings.dataNotCpuAccessible) {
                NEXUS_FlushCache(desc.addr, desc.length);
            }
            while ( numConsumed == 0 )
            {
                rc = NEXUS_Playpump_SubmitScatterGatherDescriptor(playpump, &desc, 1, &numConsumed);
                BDBG_ASSERT(!rc);
            }
            cur_buf = (cur_buf + 1) % TOTAL_BUFFERS; /* use the next buffer */
        }
    }
    error:
    return NULL;
}

int main(int argc, char **argv)
{
    pthread_t playbackThread;
    NxClient_JoinSettings joinSettings;
    NxClient_AllocSettings allocSettings;
    NxClient_AllocResults allocResults;
    NxClient_ConnectSettings connectSettings;
    unsigned connectId;
    NEXUS_PlaypumpOpenSettings playpumpOpenSettings;
    BKNI_EventHandle event;
    NEXUS_SimpleStcChannelHandle stcChannel;
    NEXUS_PidChannelHandle videoPidChannel, audioPidChannel, audioPidChannel2;
    NEXUS_SimpleVideoDecoderStartSettings videoProgram;
    NEXUS_SimpleAudioDecoderHandle audioDecoder;
    NEXUS_SimpleAudioDecoderStartSettings audioProgram;
    NEXUS_SimpleAudioDecoderSettings audioSettings;
    NEXUS_MemoryAllocationSettings memSettings;
    NEXUS_ClientConfiguration clientConfig;
    NEXUS_AudioDecoderStatus audioStatus;
    NEXUS_Error rc;

    const char *fname = FILE_NAME;
    unsigned i;
    uint32_t lastPts;

    BSTD_UNUSED(argc);

    NxClient_GetDefaultJoinSettings(&joinSettings);
    snprintf(joinSettings.name, NXCLIENT_MAX_NAME, "%s", argv[0]);
    rc = NxClient_Join(&joinSettings);
    if (rc) {
        printf("cannot join: %d\n", rc);
        return -1;
    }

    BKNI_CreateEvent(&event);

    NEXUS_Playpump_GetDefaultOpenSettings(&playpumpOpenSettings);
    playpumpOpenSettings.fifoSize = 0;
    playpump = NEXUS_Playpump_Open(0, &playpumpOpenSettings);

    /* use stdio for file I/O to keep the example simple. */
    file = fopen(fname, "rb");
    if (!file) {
        fprintf(stderr, "can't open file:%s\n", fname);
        goto error;
    }

    NEXUS_Playpump_GetSettings(playpump, &playpumpSettings);
    playpumpSettings.dataCallback.callback = play_callback;
    playpumpSettings.dataCallback.context = event;
    playpumpSettings.dataNotCpuAccessible = true;
    /* setting mode = NEXUS_PlaypumpMode_eScatterGather is deprecated */
    NEXUS_Playpump_SetSettings(playpump, &playpumpSettings);

    NEXUS_Playpump_Start(playpump);

    stcChannel = NEXUS_SimpleStcChannel_Create(NULL);

    videoPidChannel = NEXUS_Playpump_OpenPidChannel(playpump, VIDEO_PID, NULL);
    audioPidChannel = NEXUS_Playpump_OpenPidChannel(playpump, AUDIO_PID, NULL);
    audioPidChannel2 = NEXUS_Playpump_OpenPidChannel(playpump, AUDIO_PID2, NULL);

    NxClient_GetDefaultAllocSettings(&allocSettings);
    allocSettings.simpleVideoDecoder = 1;
    allocSettings.simpleAudioDecoder = 1;
    rc = NxClient_Alloc(&allocSettings, &allocResults);
    if (rc) {BDBG_WRN(("unable to alloc decode resources")); return -1;}

    NxClient_GetDefaultConnectSettings(&connectSettings);
    connectSettings.simpleVideoDecoder[0].id = allocResults.simpleVideoDecoder[0].id;
    connectSettings.simpleAudioDecoder.id = allocResults.simpleAudioDecoder.id;
    rc = NxClient_Connect(&connectSettings, &connectId);
    if (rc) {BDBG_WRN(("unable to connect decode resources")); return -1;}

    videoDecoder = NEXUS_SimpleVideoDecoder_Acquire(allocResults.simpleVideoDecoder[0].id);
    audioDecoder = NEXUS_SimpleAudioDecoder_Acquire(allocResults.simpleAudioDecoder.id);

    NEXUS_SimpleVideoDecoder_SetStcChannel(videoDecoder, stcChannel);
    NEXUS_SimpleAudioDecoder_SetStcChannel(audioDecoder, stcChannel);

    NEXUS_SimpleVideoDecoder_GetDefaultStartSettings(&videoProgram);
    videoProgram.settings.codec = VIDEO_CODEC;
    videoProgram.settings.pidChannel = videoPidChannel;
    NEXUS_SimpleVideoDecoder_Start(videoDecoder, &videoProgram);

    NEXUS_SimpleAudioDecoder_GetDefaultStartSettings(&audioProgram);
    audioProgram.primary.codec = AUDIO_CODEC;
    audioProgram.primary.pidChannel = audioPidChannel;
    #if 0
    audioProgram.description.codec = AUDIO_CODEC;
    audioProgram.description.pidChannel = audioPidChannel2;
    #endif
    NEXUS_SimpleAudioDecoder_GetSettings(audioDecoder, &audioSettings);
    audioSettings.processorSettings[NEXUS_SimpleAudioDecoderSelector_ePrimary].fade.connected = true;
    audioSettings.processorSettings[NEXUS_SimpleAudioDecoderSelector_ePrimary].fade.settings.type = 2;
    audioSettings.processorSettings[NEXUS_SimpleAudioDecoderSelector_ePrimary].fade.settings.duration = FADE_DURATION;
    audioSettings.processorSettings[NEXUS_SimpleAudioDecoderSelector_ePrimary].fade.settings.level = 100;
    audioSettings.processorSettings[NEXUS_SimpleAudioDecoderSelector_eDescription].fade.connected = true;
    audioSettings.processorSettings[NEXUS_SimpleAudioDecoderSelector_eDescription].fade.settings.type = 2;
    audioSettings.processorSettings[NEXUS_SimpleAudioDecoderSelector_eDescription].fade.settings.duration = FADE_DURATION;
    audioSettings.processorSettings[NEXUS_SimpleAudioDecoderSelector_eDescription].fade.settings.level = 0;
    NEXUS_SimpleAudioDecoder_SetSettings(audioDecoder, &audioSettings);
    NEXUS_SimpleAudioDecoder_Start(audioDecoder, &audioProgram);

    /* buffers must be from the nexus heap to be used by playpump; therefore use NEXUS_Memory_Allocate */
    NEXUS_Platform_GetClientConfiguration(&clientConfig);
    NEXUS_Memory_GetDefaultAllocationSettings(&memSettings);
    memSettings.heap = clientConfig.heap[1]; /* heap 1 is the eFull heap for the nxlient. heap 0 is the graphics heap */

    for (i=0;i<TOTAL_BUFFERS;i++) {
        NEXUS_Memory_Allocate(buf_size, &memSettings, &buf[i]);
    }

    /* Start threads for decoder and capture */
    pthread_create(&playbackThread, NULL, playback_thread, NULL);

    rc = NEXUS_SimpleAudioDecoder_GetStatus(audioDecoder, &audioStatus);
    BDBG_ASSERT(!rc);

    lastPts = audioStatus.pts;
    for(cur_buf=0;;) {
        int tmp;
        NEXUS_Error rc;
        NEXUS_AudioProcessorStatus processorStatus;
        bool toggle = false;

        printf("Main Menu\n");
        printf(" 0) Exit\n");
        printf(" 1) Toggle decoder fade levels\n");
        printf("Enter Selection: \n");
        scanf("%d", &tmp);
        switch ( tmp )
        {
        case 0:
            done = true;
            break;
        case 1:
            toggle = true;
            break;
        default:
            break;
        }

        if ( done )
        {
            pthread_join(playbackThread, NULL);
            break;
        }

        if ( toggle )
        {
            unsigned level, initialLevel;

            BDBG_ERR(("Set new fade value\n"));
            NEXUS_SimpleAudioDecoder_GetSettings(audioDecoder, &audioSettings);
            level = audioSettings.processorSettings[NEXUS_SimpleAudioDecoderSelector_ePrimary].fade.settings.level = (audioSettings.processorSettings[NEXUS_SimpleAudioDecoderSelector_ePrimary].fade.settings.level == 100) ? 20 : 100;
            BDBG_ERR(("Set fade for %d seconds, to level %u%%\n", audioSettings.processorSettings[NEXUS_SimpleAudioDecoderSelector_ePrimary].fade.settings.duration, level));
            NEXUS_SimpleAudioDecoder_SetSettings(audioDecoder, &audioSettings);

            /* wait for fade to activate */
            BDBG_ERR(("Wait for fade to activate\n"));
            do {
                BKNI_Sleep(100);
                rc = NEXUS_SimpleAudioDecoder_GetProcessorStatus(audioDecoder, NEXUS_SimpleAudioDecoderSelector_ePrimary, NEXUS_AudioPostProcessing_eFade, &processorStatus);
                BDBG_ERR(("rc %lu, type %lu, active %lu, level %lu%%\n", (unsigned long)rc, (unsigned long)processorStatus.type, (unsigned long)processorStatus.status.fade.active, (unsigned long)processorStatus.status.fade.level));
            } while ( rc != NEXUS_SUCCESS || processorStatus.type == NEXUS_AudioPostProcessing_eMax || !processorStatus.status.fade.active );

            /* fade is active... stop while fading */
            BDBG_ERR(("Stopping...\n"));
            NEXUS_SimpleAudioDecoder_Stop(audioDecoder);

            /* Set initial level values */
            initialLevel = processorStatus.status.fade.level;
            NEXUS_SimpleAudioDecoder_GetSettings(audioDecoder, &audioSettings);
            BDBG_ERR(("Setting initial level of %lu%%, while stopped\n", (unsigned long)initialLevel));
            audioSettings.processorSettings[NEXUS_SimpleAudioDecoderSelector_ePrimary].fade.settings.level = initialLevel;
            rc = NEXUS_SimpleAudioDecoder_SetSettings(audioDecoder, &audioSettings);
            BDBG_ASSERT(!rc);

            rc = NEXUS_SimpleAudioDecoder_GetProcessorStatus(audioDecoder, NEXUS_SimpleAudioDecoderSelector_ePrimary, NEXUS_AudioPostProcessing_eFade, &processorStatus);
            BDBG_ERR(("GetStatus while stopped - rc %lu, type %lu, active %lu, level %lu%%\n", (unsigned long)rc, (unsigned long)processorStatus.type, (unsigned long)processorStatus.status.fade.active, (unsigned long)processorStatus.status.fade.level));

            /* switch pid channels */
            audioProgram.primary.pidChannel = (audioProgram.primary.pidChannel == audioPidChannel) ? audioPidChannel2 : audioPidChannel;
            BDBG_ERR(("Starting\n"));
            rc = NEXUS_SimpleAudioDecoder_Start(audioDecoder, &audioProgram);
            BDBG_ASSERT(!rc);

            /* wait for initial levels to be valid */
            do {
                BKNI_Sleep(500);
                rc = NEXUS_SimpleAudioDecoder_GetProcessorStatus(audioDecoder, NEXUS_SimpleAudioDecoderSelector_ePrimary, NEXUS_AudioPostProcessing_eFade, &processorStatus);
                BDBG_ERR(("rc %lu, type %lu, active %lu, level %lu%%\n", (unsigned long)rc, (unsigned long)processorStatus.type, (unsigned long)processorStatus.status.fade.active, (unsigned long)processorStatus.status.fade.level));
            } while ( rc != NEXUS_SUCCESS || processorStatus.type == NEXUS_AudioPostProcessing_eMax || processorStatus.status.fade.level != initialLevel );
            #if 1
            /* Initial fade values should be valid now. program the fade */
            BDBG_ERR(("Resuming fade from %lu%% to %lu%%\n", (unsigned long)initialLevel, (unsigned long)level));
            audioSettings.processorSettings[NEXUS_SimpleAudioDecoderSelector_ePrimary].fade.settings.level = level;
            rc = NEXUS_SimpleAudioDecoder_SetSettings(audioDecoder, &audioSettings);
            BDBG_ASSERT(!rc);
            do {
                BKNI_Sleep(500);
                rc = NEXUS_SimpleAudioDecoder_GetProcessorStatus(audioDecoder, NEXUS_SimpleAudioDecoderSelector_ePrimary, NEXUS_AudioPostProcessing_eFade, &processorStatus);
                BDBG_ERR(("rc %lu, type %lu, active %lu, level %lu%%\n", (unsigned long)rc, (unsigned long)processorStatus.type, (unsigned long)processorStatus.status.fade.active, (unsigned long)processorStatus.status.fade.level));
            } while ( processorStatus.status.fade.active );
            #endif
        }
    }
    return 0;

    error:
    return 1;
}
#else
#include <stdio.h>
int main(void)
{
    printf("This application is not supported on this platform\n");
    return 0;
}
#endif
