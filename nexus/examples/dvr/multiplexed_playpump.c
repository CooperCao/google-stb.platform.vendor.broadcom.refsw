/******************************************************************************
 *  Copyright (C) 2016-2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#include "nexus_platform.h"
#include <stdio.h>

#if NEXUS_HAS_VIDEO_DECODER
#include "nexus_video_decoder.h"
#include "nexus_stc_channel.h"
#include "nexus_display.h"
#include "nexus_message.h"
#include "nexus_video_window.h"
#include "nexus_video_input.h"
#include "nexus_component_output.h"
#if NEXUS_HAS_HDMI_OUTPUT
#include "nexus_hdmi_output.h"
#endif
#include "nexus_playpump.h"

#include <assert.h>
#include <pthread.h>
#include "bstd.h"
#include "bkni.h"
#include "bkni_multi.h"

BDBG_MODULE(multiplexed_playback);

static void play_callback(void *context, int param)
{
    BSTD_UNUSED(param);
    BKNI_SetEvent((BKNI_EventHandle)context);
}

#define FILE_BUFFER_SIZE (4 * 188 * 1024) /* for an 8Mbps stream, 256K should be 1/4 second */
#define MAX_PID_CHANNELS 4

struct file_thread_data
{
    pthread_t thread;
    FILE *file;
    char *file_buffer;
    NEXUS_PidChannelHandle pidChannel[MAX_PID_CHANNELS];

    /* video decoder consumer */
    NEXUS_VideoWindowHandle window;
    NEXUS_VideoDecoderHandle videoDecoder;
    NEXUS_StcChannelHandle stcChannel;

    /* message consumer */
    NEXUS_MessageHandle message;
    unsigned message_buffer_size;

    BKNI_MutexHandle playpumpMutex;
    /* must acquire playpumpMutex before accessing these: */
    NEXUS_PlaypumpHandle playpump;
    BKNI_EventHandle dataCallbackEvent;
};

/* ft_wait_for_done should only be called with playpumpMutex already acquired */
void ft_wait_for_done(struct file_thread_data *ft_data)
{
    while (1) {
        NEXUS_PlaypumpStatus status;
        NEXUS_Error rc;

        /* we are trusting the event and status. when fifoDepth is 0, we assume every byte has gotten through. */
        rc = NEXUS_Playpump_GetStatus(ft_data->playpump, &status);
        BDBG_ASSERT(!rc);
        if (status.fifoDepth == 0) break;

        BKNI_WaitForEvent(ft_data->dataCallbackEvent, 10000);
    }
}

/* ft_set_pid_channels should only be called with playpumpMutex already acquired */
void ft_set_pid_channels(struct file_thread_data *ft_data, bool enabled)
{
    unsigned i;
    for (i=0;i<MAX_PID_CHANNELS;i++) {
        if (ft_data->pidChannel[i]) {
            NEXUS_PidChannel_SetEnabled(ft_data->pidChannel[i], enabled);
        }
    }
}

bool g_done = false;

void *decoder_file_thread(void *context)
{
    struct file_thread_data *ft_data = context;

    ft_set_pid_channels(ft_data, false);
    while (!g_done) {
        unsigned data_read;
        unsigned data_played = 0;
        NEXUS_Error rc;
        NEXUS_VideoDecoderStatus vStatus;
        unsigned amount_needed;

        /* only send what the decoder buffer can receive at that moment. this ensures
        that playpump will not block and allows streams of different bitrates to be multiplexed.
        note: this code assumes the video decoder fifo will fill. for some streams, the audio decoder fifo will fill and
        video will not. this might need to be tuned to account for both or to throttled based on bitrate.
        also, it's possible that ft_data->file_buffer must be managed as a ring buffer that wraps. */
        NEXUS_VideoDecoder_GetStatus(ft_data->videoDecoder, &vStatus);
        amount_needed = vStatus.fifoSize - vStatus.fifoDepth;
        if (amount_needed > FILE_BUFFER_SIZE) {
            amount_needed = FILE_BUFFER_SIZE;
        }
        amount_needed -= (amount_needed % 188);

        /* read the data into the buffer without holding the playpump mutex */
        /* TODO: use posix io and O_DIRECT */
        data_read = fread(ft_data->file_buffer, 1, amount_needed, ft_data->file);
        BDBG_MSG(("%p: fread %d", context, data_read));
        if (data_read < amount_needed) {
            /* end of file */
            data_read -= (data_read % 188);
            fseek(ft_data->file, 0, SEEK_SET);
            if (data_read == 0) continue;
        }

        BDBG_ASSERT(data_read % 188 == 0);

        /* acquire playpump */
        BKNI_AcquireMutex(ft_data->playpumpMutex);
        ft_set_pid_channels(ft_data, true);

        /* code in here needs to be as fast as possible */
        while (data_played < data_read) {
            void *playpump_buffer;
            size_t buffer_size;

            if (NEXUS_Playpump_GetBuffer(ft_data->playpump, &playpump_buffer, &buffer_size))
                break;
            if (buffer_size == 0) {
                /* TODO: if playpump blocks, we could simply exit and remember our position in ft_data->file_buffer.
                Make file_buffer into a ring buffer. This would allow us to decrease buffer sizes. */
                BDBG_WRN(("playpump %p is blocking", (void*)ft_data->playpump));
                BKNI_WaitForEvent(ft_data->dataCallbackEvent, BKNI_INFINITE);
                continue;
            }

            if (buffer_size > (data_read - data_played)) {
                buffer_size = data_read - data_played;
            }

            /* TODO: use playpump scatter gather to avoid a memcpy. memcpy is not efficient and is done with the playpump
            mutex acquired, which is not good. see nexus/examples/dvr/playpump_scatter_gather.c */
            BKNI_Memcpy(playpump_buffer, &ft_data->file_buffer[data_played], buffer_size);

            rc = NEXUS_Playpump_WriteComplete(ft_data->playpump, 0, buffer_size);
            BDBG_ASSERT(!rc);
            BDBG_MSG(("%p: played %u bytes", context, (unsigned)buffer_size));
            data_played += buffer_size;
        }

        /* wait for all data to be through XPT */
        ft_wait_for_done(ft_data);

        ft_set_pid_channels(ft_data, false);
        BKNI_ReleaseMutex(ft_data->playpumpMutex);
    }
    return NULL;
}

void *message_file_thread(void *context)
{
    struct file_thread_data *ft_data = context;

    ft_set_pid_channels(ft_data, false);
    while (!g_done) {
        unsigned data_read;
        unsigned data_played = 0;
        NEXUS_Error rc;
        unsigned amount_needed = FILE_BUFFER_SIZE; /* see below for assert. this must be less than message_buffer_size */

        amount_needed -= (amount_needed % 188);

        data_read = fread(ft_data->file_buffer, 1, amount_needed, ft_data->file);
        BDBG_MSG(("%p: fread %d", context, data_read));
        if (data_read < amount_needed) {
            /* end of file */
            data_read -= (data_read % 188);
            fseek(ft_data->file, 0, SEEK_SET);
            if (data_read == 0) continue;
        }

        BDBG_ASSERT(data_read % 188 == 0);

        /* acquire playpump */
        BKNI_AcquireMutex(ft_data->playpumpMutex);
        ft_set_pid_channels(ft_data, true);

        /* code in here needs to be as fast as possible */
        while (data_played < data_read) {
            void *playpump_buffer;
            size_t buffer_size;

            if (NEXUS_Playpump_GetBuffer(ft_data->playpump, &playpump_buffer, &buffer_size))
                break;
            if (buffer_size == 0) {
                BDBG_WRN(("playpump %p is blocking", (void*)ft_data->playpump));
                BKNI_WaitForEvent(ft_data->dataCallbackEvent, BKNI_INFINITE);
                continue;
            }

            if (buffer_size > (data_read - data_played)) {
                buffer_size = data_read - data_played;
            }

            BKNI_Memcpy(playpump_buffer, &ft_data->file_buffer[data_played], buffer_size);

            rc = NEXUS_Playpump_WriteComplete(ft_data->playpump, 0, buffer_size);
            BDBG_ASSERT(!rc);
            BDBG_MSG(("%p: played %u bytes", context, (unsigned)buffer_size));
            data_played += buffer_size;
        }

        /* wait for all data to be through XPT */
        ft_wait_for_done(ft_data);

        ft_set_pid_channels(ft_data, false);
        BKNI_ReleaseMutex(ft_data->playpumpMutex);

        /* now we can take out time read the messages outside of the playpump multiplex lock */
        while (1) {
            const void *buffer;
            size_t size;
            rc = NEXUS_Message_GetBuffer(ft_data->message, &buffer, &size);
            BDBG_ASSERT(!rc);
            if (!size) {
                break;
            }
            printf("%p processed %u bytes of message data\n", (void*)ft_data, (unsigned)size);
            rc = NEXUS_Message_ReadComplete(ft_data->message, size);
            BDBG_ASSERT(!rc);
        }
        /* message buffer is now empty */
    }
    return NULL;
}

#define NUM_THREADS 4

struct pvr_file {
    const char *filename;
    unsigned short videoPid;
    NEXUS_VideoCodec videoCodec;
} g_pvr_file[NUM_THREADS] = {
#if 0
    {"videos/riddick_avc_720p.mpg", 0x1422, NEXUS_VideoCodec_eH264}, /* decode an AVD HD stream */
#else
    {"videos/mummy_MI_5element_q64.mpg", 0x21, NEXUS_VideoCodec_eMpeg2}, /* decode a high bitrate stream with a duplicate pid channel */
#endif
    {"videos/cnnticker.mpg", 0x21, NEXUS_VideoCodec_eMpeg2},
    {"videos/mummy_MI_5element_q64.mpg", 0x3f, NEXUS_VideoCodec_eNone}, /* message filtering */
    {"videos/mummy_MI_5element_q64.mpg", 0x3e, NEXUS_VideoCodec_eNone}, /* message filtering */
};

int main(void)
{
    struct file_thread_data ft_data[NUM_THREADS];
    NEXUS_PlaypumpSettings playpumpSettings;
    NEXUS_PlaypumpHandle playpump;
    BKNI_EventHandle event;
    BKNI_MutexHandle mutex;
    NEXUS_DisplayHandle display;
    NEXUS_PlatformSettings platformSettings;
    NEXUS_PlatformConfiguration platformConfig;
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_HdmiOutputStatus hdmiStatus;
    NEXUS_DisplaySettings displaySettings;
#endif
    unsigned i;
    NEXUS_Error rc;

    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init(&platformSettings);
    NEXUS_Platform_GetConfiguration(&platformConfig);

    BKNI_CreateEvent(&event);
    BKNI_CreateMutex(&mutex);

    playpump = NEXUS_Playpump_Open(0, NULL);
    NEXUS_Playpump_GetSettings(playpump, &playpumpSettings);
    playpumpSettings.dataCallback.callback = play_callback;
    playpumpSettings.dataCallback.context = event;
    NEXUS_Playpump_SetSettings(playpump, &playpumpSettings);
    NEXUS_Playpump_Start(playpump);

    display = NEXUS_Display_Open(0, NULL);
#if NEXUS_NUM_COMPONENT_OUTPUTS
    NEXUS_Display_AddOutput(display, NEXUS_ComponentOutput_GetConnector(platformConfig.outputs.component[0]));
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

    BKNI_Memset(ft_data, 0, sizeof(ft_data));

    for (i=0;i<NUM_THREADS;i++) {
        NEXUS_PlaypumpOpenPidChannelSettings pidChannelSettings;
        ft_data[i].playpump = playpump;
        ft_data[i].dataCallbackEvent = event;
        ft_data[i].playpumpMutex = mutex;

        /* use stdio for file I/O to keep the example simple. */
        ft_data[i].file = fopen(g_pvr_file[i].filename, "rb");
        if (!ft_data[i].file) {
            BDBG_ERR(("can't open file:%s", g_pvr_file[i].filename));
            goto error;
        }

        ft_data[i].file_buffer = BKNI_Malloc(FILE_BUFFER_SIZE);

        NEXUS_Playpump_GetDefaultOpenPidChannelSettings(&pidChannelSettings);
        pidChannelSettings.pidSettings.pidChannelIndex = i;
        /* Nexus pid channels default to being enabled. We need them to start disabled. */
        pidChannelSettings.pidSettings.enabled = false;

        if (i < 2) {
            NEXUS_StcChannelSettings stcSettings;
            NEXUS_VideoDecoderStartSettings videoProgram;

            NEXUS_StcChannel_GetDefaultSettings(0, &stcSettings);
            stcSettings.timebase = NEXUS_Timebase_e0 + i;
            stcSettings.mode = NEXUS_StcChannelMode_eAuto;
            ft_data[i].stcChannel = NEXUS_StcChannel_Open(i, &stcSettings);

            ft_data[i].pidChannel[0] = NEXUS_Playpump_OpenPidChannel(playpump, g_pvr_file[i].videoPid, &pidChannelSettings);

            NEXUS_VideoDecoder_GetDefaultStartSettings(&videoProgram);
            videoProgram.codec = g_pvr_file[i].videoCodec;
            videoProgram.pidChannel = ft_data[i].pidChannel[0];
            videoProgram.stcChannel = ft_data[i].stcChannel;

            ft_data[i].window = NEXUS_VideoWindow_Open(display, i);
            ft_data[i].videoDecoder = NEXUS_VideoDecoder_Open(i, NULL);
            rc = NEXUS_VideoWindow_AddInput(ft_data[i].window, NEXUS_VideoDecoder_GetConnector(ft_data[i].videoDecoder));
            BDBG_ASSERT(!rc);

            rc = NEXUS_VideoDecoder_Start(ft_data[i].videoDecoder, &videoProgram);
            BDBG_ASSERT(!rc);

            rc = pthread_create(&ft_data[i].thread, NULL, decoder_file_thread, &ft_data[i]);
            BDBG_ASSERT(!rc);
        }
        else {
            NEXUS_MessageStartSettings startSettings;
            NEXUS_MessageSettings settings;

            ft_data[i].pidChannel[0] = NEXUS_Playpump_OpenPidChannel(playpump, g_pvr_file[i].videoPid, &pidChannelSettings);

            /* make sure playpump won't block flowing into the message buffer */
            ft_data[i].message_buffer_size = 1024 * 1024;
            BDBG_ASSERT(ft_data[i].message_buffer_size >= FILE_BUFFER_SIZE);

            NEXUS_Message_GetDefaultSettings(&settings);
            /* no need for callback. we will poll after feeding data through playpump. */
            settings.bufferSize = ft_data[i].message_buffer_size;
            ft_data[i].message = NEXUS_Message_Open(&settings);

            NEXUS_Message_GetDefaultStartSettings(ft_data[i].message, &startSettings);
            startSettings.pidChannel = ft_data[i].pidChannel[0];
            rc = NEXUS_Message_Start(ft_data[i].message, &startSettings);
            BDBG_ASSERT(!rc);

            rc = pthread_create(&ft_data[i].thread, NULL, message_file_thread, &ft_data[i]);
            BDBG_ASSERT(!rc);
        }
    }

    BDBG_WRN(("Press ENTER to quit"));
    getchar();

    g_done = true;

    /* TODO: pthread_join, shutdown, etc. */

    return 0;

error:
    return 1;
}
#else
int main(void)
{
    printf("This application is not supported on this platform (needs video decoder)!\n");
    return 0;
}
#endif
