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
 *
 * Module Description:
 *
 *****************************************************************************/
/* Nexus example app:
Playback and decode of MPEG PES file from memory (using Playpump) along
with a simple algorithm to detect the end of audio decode. */

#include "nexus_platform.h"
#include <stdio.h>

#if NEXUS_HAS_AUDIO
#include "nexus_pid_channel.h"
#include "nexus_playpump.h"
#include "nexus_audio_dac.h"
#include "nexus_audio_decoder.h"
#include "nexus_audio_output.h"

#include <assert.h>
#include "bstd.h"
#include "bkni.h"
#include "bkni_multi.h"

static void play_callback(void *context, int param)
{
    BSTD_UNUSED(param);
    BKNI_SetEvent((BKNI_EventHandle)context);
}

#if defined(LINUX)
#include <sys/time.h>
#elif defined(VXWORKS)
#include <timeCommon.h>
#elif defined (NEXUS_BASE_OS_ucos)
#include "type.h"
#include "ucos.h"
#endif
unsigned b_get_time(void)
{
#if defined(LINUX)
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec*1000 + tv.tv_usec/1000;
#elif defined(VXWORKS)
    struct timespec tv;
    clock_gettime(CLOCK_REALTIME, &tv);
    return tv.tv_sec*1000 + tv.tv_nsec/1000/1000;
#elif defined(NEXUS_BASE_OS_ucos)
    const unsigned msPerTick = 10;
    long osTicks = OSTimeGet();
    return (osTicks * msPerTick);
#else
#error Please implement b_get_time for your OS
#endif
}

int main(void) {
    FILE *file;
    const char *filename = "videos/cnnticker.audio.pes";
    NEXUS_PlaypumpSettings playpumpSettings;
    NEXUS_PlaypumpHandle playpump;
    BKNI_EventHandle event;
    NEXUS_AudioDecoderHandle audioDecoder;
    NEXUS_AudioDecoderStartSettings audioProgram;
    NEXUS_PidChannelHandle audioPidChannel;
    NEXUS_PlatformConfiguration platformConfig;
    unsigned pos = 0;
    unsigned pes_stream_length = 0;
    uint8_t *pes_stream;
    unsigned last_depth = 0;
    unsigned last_depth_time = 0;

    /* quick and dirty read from file into memory, then do playpump from memory */
    file = fopen(filename, "rb");
    fseek(file, 0, SEEK_END);
    pes_stream_length = ftell(file);
    pes_stream = BKNI_Malloc(pes_stream_length);
    BKNI_Printf("file size: %d\n", pes_stream_length);
    fseek(file, 0, SEEK_SET);
    pes_stream_length = fread(pes_stream, 1, pes_stream_length, file);
    BKNI_Printf("file read: %d\n", pes_stream_length);

    /* Bring up all modules for a platform in a default configuraiton for this platform */
    NEXUS_Platform_Init(NULL);
    NEXUS_Platform_GetConfiguration(&platformConfig);

    BKNI_CreateEvent(&event);

    /**
    playpump
    **/
    playpump = NEXUS_Playpump_Open(0, NULL);
    NEXUS_Playpump_GetSettings(playpump, &playpumpSettings);
    playpumpSettings.transportType = NEXUS_TransportType_eMpeg2Pes;
    playpumpSettings.dataCallback.callback = play_callback;
    playpumpSettings.dataCallback.context = event;
    NEXUS_Playpump_SetSettings(playpump, &playpumpSettings);
    audioPidChannel = NEXUS_Playpump_OpenPidChannel(playpump, 0xC0, NULL);

    /**
    audio decode
    **/
    audioDecoder = NEXUS_AudioDecoder_Open(0, NULL);
    if (platformConfig.outputs.audioDacs[0]) {
        NEXUS_AudioOutput_AddInput(
            NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]),
            NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
    }
    NEXUS_AudioDecoder_GetDefaultStartSettings(&audioProgram);
    audioProgram.codec = NEXUS_AudioCodec_eMpeg;
    audioProgram.pidChannel = audioPidChannel;

    /* start */
    NEXUS_Playpump_Start(playpump);
    NEXUS_AudioDecoder_Start(audioDecoder, &audioProgram);

    while (1) {
        void *buffer;
        size_t buffer_size;
        NEXUS_Error rc;

        if (NEXUS_Playpump_GetBuffer(playpump, &buffer, &buffer_size))
            break;
        if (buffer_size == 0) {
            BKNI_WaitForEvent(event, BKNI_INFINITE);
            continue;
        }

        /* The first call to get_buffer will return the entire XPT playback buffer.
        If we use it, we're going to have to wait until the descriptor is complete,
        and then we're going to underflow. So use a max size. */
#define MAX_READ (188*1024)
        if (buffer_size > MAX_READ)
            buffer_size = MAX_READ;
        if (buffer_size > pes_stream_length-pos)
            buffer_size = pes_stream_length-pos;

        BKNI_Memcpy(buffer, &pes_stream[pos], buffer_size);
        rc = NEXUS_Playpump_WriteComplete(playpump, 0, buffer_size);
        BDBG_ASSERT(!rc);

        pos += buffer_size;
        if (pos == pes_stream_length) {
            break; /* only play once */
        }
    }

    /* wait for the end of decode */

    /* first, wait for the playback fifo to go to zero. */
    while (1) {
        NEXUS_PlaypumpStatus status;
        NEXUS_Playpump_GetStatus(playpump, &status);
        printf("playpump fifo %d\n", status.fifoDepth);
        if (status.fifoDepth == 0)
            break;
        BKNI_Sleep(200);
    }

    /* second, wait for the audio fifo to stay static for a period of time. */
    while (1) {
        NEXUS_AudioDecoderStatus status;
        NEXUS_AudioDecoder_GetStatus(audioDecoder, &status);
        printf("audio fifo %d\n", status.fifoDepth);
        if (status.fifoDepth == last_depth) {
            if (b_get_time() - last_depth_time > 250) {
                /* if the audio CDB is static for 1/4 second, then we're done */
                break;
            }
        }
        else {
            last_depth = status.fifoDepth;
            last_depth_time = b_get_time();
        }
        BKNI_Sleep(50);
    }
    printf("audio decode is done\n");

    return 1;
}
#else /* NEXUS_HAS_AUDIO */
int main(void)
{
    printf("This application is not supported on this platform (needs audio)!\n");
    return 0;
}
#endif
