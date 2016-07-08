/******************************************************************************
 *  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
/* Nexus example app: this example feeds data from three ASF files encoded with different bitrates */

#include "nexus_platform.h"
#include <stdio.h>

#if NEXUS_HAS_VIDEO_DECODER
#include "nexus_pid_channel.h"
#include "nexus_playpump.h"
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
#include "nexus_playpump.h"

#include <assert.h>
#include "bstd.h"
#include "bkni.h"
#include "bkni_multi.h"

/* the following define the input and its characteristics -- these will vary by input */
#define TRANSPORT_TYPE NEXUS_TransportType_eAsf
#define VIDEO_CODEC NEXUS_VideoCodec_eVc1
#define VIDEO_PID 2


/*
 * When building Nexus, you will need the following env variables:
 *
 * export MEDIA_ASF_SUPPORT=y
 */

static void play_callback(void *context, int param)
{
    BSTD_UNUSED(param);
    BKNI_SetEvent((BKNI_EventHandle)context);
}

struct clip {
    const char *file;
    unsigned header_size;
    unsigned file_size;
    unsigned change_points[8];
};

/* this table has a coded switch points in the file, different switch points should point into the same I(Key) frame in all files */
static const struct clip clips[3] = 
{
    {"ElephantsDream_SL_0500_video.Align.wmv", 6639, 37642643,
        {
            326639, /* pres_time:12000 */
            614639, /* pres_time:21000 */
            870639, /* pres_time:30000 */
            1606639, /* pres_time:39000 */
            2422639, /* pres_time:48000 */
            2950639, /* pres_time:57000 */
            3686639, /* pres_time:66000 */
            4054639  /* pres_time:75000 */
        }
    },
    {"ElephantsDream_SL_1000_video.Align.wmv", 6639, 77290643,
        {
            662639,
            1238639,
            1750639,
            3286639,
            4918639,
            6086639,
            7654639,
            8486639
        }
    },
    {"ElephantsDream_SL_1500_video.Align.wmv", 6639, 111642643,
        {
           1062639,
           1942639,
           2710639,
           4966639,
           7286639,
           9014639,
           11334639,
           12710639 
        }
    }
};

static FILE *reopen_file(unsigned clip, FILE *fin)
{
    char path[256];
    if(fin) {
        fclose(fin);
    }
    BKNI_Snprintf(path, sizeof(path), "videos/%s", clips[clip].file);
    fin = fopen(path, "rb");
    if(!fin) {
        fprintf(stderr, "can't open file:%s\n", path);
    }
    return fin;
}

static int send_data(NEXUS_PlaypumpHandle playpump, BKNI_EventHandle event, FILE *file, size_t bytes)
{
    void *buffer;
    size_t buffer_size;
    NEXUS_Error rc;
    NEXUS_PlaypumpSegment segment;

    BKNI_Memset(&segment, 0, sizeof(segment));
    segment.length = bytes+sizeof(segment);
    segment.offset = ftell(file);
    segment.signature = NEXUS_PLAYPUMP_SEGMENT_SIGNATURE;

    /* send a descriptor */
    for(;;) {
        rc = NEXUS_Playpump_GetBuffer(playpump, &buffer, &buffer_size);
        if(rc!=NEXUS_SUCCESS) {goto error;}
        if(buffer_size>=sizeof(segment)) {
            BKNI_Memcpy(buffer, &segment, sizeof(segment));
            fprintf(stderr, "segment %u:%u\n", (unsigned)segment.offset, (unsigned)bytes);
            rc = NEXUS_Playpump_WriteComplete(playpump, 0, sizeof(segment));
            if(rc!=NEXUS_SUCCESS) {goto error;}
            break;
        }
        if(buffer_size>0) {
            NEXUS_PlaypumpStatus status;
            rc = NEXUS_Playpump_GetStatus(playpump, &status);
            if(rc!=NEXUS_SUCCESS) {goto error;}
            if( (uint8_t *)buffer >= (uint8_t *)status.bufferBase + (status.fifoSize - sizeof(segment))) { /* need to recycle small buffer on wrap point */
                rc = NEXUS_Playpump_WriteComplete(playpump, buffer_size, 0);
                if(rc!=NEXUS_SUCCESS) {goto error;}
                continue;
            }
        }
        BKNI_WaitForEvent(event, BKNI_INFINITE);
    }
    /* send data */
    while(bytes>0) {
        int n;

        rc = NEXUS_Playpump_GetBuffer(playpump, &buffer, &buffer_size);
        if(rc!=NEXUS_SUCCESS) {goto error;}
        if(buffer_size>0) {
            if(buffer_size>bytes) {
                buffer_size = bytes;
            }
            n = fread(buffer, 1, buffer_size, file);
            fprintf(stderr, "Read %d\n", n);
            if(n<=0) { goto error;}
            rc = NEXUS_Playpump_WriteComplete(playpump, 0, n);
            if(rc!=NEXUS_SUCCESS) {goto error;}
            bytes -= n;
        } else {
            BKNI_WaitForEvent(event, BKNI_INFINITE);
        }
    }
    return 0;
error:
    fprintf(stderr, "Error\n");
    return -1;
}

static int send_segment(NEXUS_PlaypumpHandle playpump, BKNI_EventHandle event, FILE *file, unsigned clip, unsigned segment)
{
    unsigned offset, last;
    unsigned asf_header = clips[clip].header_size;

    fprintf(stderr, "Sending clip %u segment %u\n", clip, segment);
    file = reopen_file(clip, file);
    if(!file) {goto error;}
    fseek(file, 0, SEEK_SET);
    send_data(playpump, event, file, asf_header);
    if(segment==0) {
        offset = asf_header;
    } else {
        offset = clips[clip].change_points[segment-1];
    }
    if(segment<1+sizeof(clips[0].change_points)/sizeof(clips[0].change_points[0])) {
        last = clips[clip].change_points[segment];
    } else {
        last = clips[clip].file_size;
    }
    fseek(file, offset, SEEK_SET);
    for(;offset<last;) {
        size_t block=188*1024;
        if(offset+block>last) {
            block = last - offset;
        }
        offset += block;
        send_data(playpump, event, file, block);
    }
    return 0;
error:
    return -1;
}

int main(void)
{
    NEXUS_PlaypumpSettings playpumpSettings;
    NEXUS_PlaypumpHandle playpump;
    NEXUS_PlaypumpOpenPidChannelSettings playpumpPidChannelSettings;
    NEXUS_StcChannelHandle stcChannel;
    NEXUS_StcChannelSettings stcSettings;
    NEXUS_PidChannelHandle videoPidChannel;
    NEXUS_DisplayHandle display;
    NEXUS_VideoWindowHandle window;
    NEXUS_VideoDecoderHandle videoDecoder;
    NEXUS_VideoDecoderStartSettings videoProgram;
    NEXUS_PlatformSettings platformSettings;
    NEXUS_PlatformConfiguration platformConfig;
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_HdmiOutputStatus hdmiStatus;
    NEXUS_DisplaySettings displaySettings;
    NEXUS_Error rc;
#endif
    BKNI_EventHandle event;
    FILE *file=NULL;
    unsigned clip = 0;

    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init(&platformSettings);
    NEXUS_Platform_GetConfiguration(&platformConfig);

    BKNI_CreateEvent(&event);

    playpump = NEXUS_Playpump_Open(0, NULL);
    file = reopen_file(clip, file);
    if(!file) {goto error;}

    NEXUS_Playpump_GetSettings(playpump, &playpumpSettings);
    playpumpSettings.dataCallback.callback = play_callback;
    playpumpSettings.dataCallback.context = event;
    playpumpSettings.transportType = TRANSPORT_TYPE;
    /* when sending data with discontinuites we need to indicate location of the data in the original stream */
    playpumpSettings.mode = NEXUS_PlaypumpMode_eSegment;
    NEXUS_Playpump_SetSettings(playpump, &playpumpSettings);

    NEXUS_Playpump_Start(playpump);

    NEXUS_StcChannel_GetDefaultSettings(0, &stcSettings);
    stcSettings.timebase = NEXUS_Timebase_e0;
    stcSettings.mode = NEXUS_StcChannelMode_eAuto;
    stcChannel = NEXUS_StcChannel_Open(0, &stcSettings);

    NEXUS_Playpump_GetDefaultOpenPidChannelSettings(&playpumpPidChannelSettings);
    videoPidChannel = NEXUS_Playpump_OpenPidChannel(playpump, VIDEO_PID, &playpumpPidChannelSettings);

    NEXUS_VideoDecoder_GetDefaultStartSettings(&videoProgram);
    videoProgram.codec = VIDEO_CODEC;
    videoProgram.pidChannel = videoPidChannel;
    videoProgram.stcChannel = stcChannel;

    /* Bring up video decoder and display. No audio to keep the example simple. */
    display = NEXUS_Display_Open(0, NULL);
    window = NEXUS_VideoWindow_Open(display, 0);

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

    videoDecoder = NEXUS_VideoDecoder_Open(0, NULL); /* take default capabilities */
    NEXUS_VideoWindow_AddInput(window, NEXUS_VideoDecoder_GetConnector(videoDecoder));

    NEXUS_VideoDecoder_Start(videoDecoder, &videoProgram);

    {
        unsigned clip=0,segment=0;
        for(;;) {
            send_segment(playpump, event, file, clip, segment);
            clip = (clip + 1)%(sizeof(clips)/sizeof(clips[0]));
            segment = (segment + 1)%((sizeof(clips[0].change_points)/sizeof(clips[0].change_points[0]))-1);
            /* getchar();getchar(); */
        }
    }
    /* loops forever */

error:
    return 1;
}
#else /* NEXUS_HAS_VIDEO_DECODER*/
int main(void)
{
    printf("This application is not supported on this platform (needs video decoder)!\n");
    return 0;
}
#endif
