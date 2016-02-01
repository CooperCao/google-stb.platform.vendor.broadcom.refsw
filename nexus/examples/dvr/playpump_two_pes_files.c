/******************************************************************************
 *    (c)2008-2014 Broadcom Corporation
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
/* Nexus example app: two PES files (one video, one audio) using two playpumps with lipsync */

#include "nexus_platform.h"
#include <stdio.h>

#if NEXUS_HAS_VIDEO_DECODER
#include "nexus_video_decoder.h"
#include "nexus_audio_decoder.h"
#include "nexus_stc_channel.h"
#include "nexus_display.h"
#include "nexus_video_window.h"
#include "nexus_video_input.h"
#include "nexus_component_output.h"
#include "nexus_composite_output.h"
#if NEXUS_HAS_HDMI_OUTPUT
#include "nexus_hdmi_output.h"
#endif
#include "nexus_audio_output.h"
#include "nexus_playpump.h"

#include <assert.h>
#include "bstd.h"
#include "bkni.h"
#include "bkni_multi.h"

/* the following define the input file and its characteristics -- these will vary by input file */
#define VIDEO_FILE_NAME "videos/cnnticker.video.pes"
#define AUDIO_FILE_NAME "videos/cnnticker.audio.pes"
#define TRANSPORT_TYPE NEXUS_TransportType_eMpeg2Pes
#define VIDEO_CODEC NEXUS_VideoCodec_eMpeg2
#define AUDIO_CODEC NEXUS_AudioCodec_eMpeg
#define VIDEO_PID 0xe0
#define AUDIO_PID 0xc0

static void play_callback(void *context, int param)
{
    BSTD_UNUSED(param);
    BKNI_SetEvent((BKNI_EventHandle)context);
}

int main(void)
{
    NEXUS_PlaypumpSettings playpumpSettings;
    NEXUS_PlaypumpHandle videoPlaypump, audioPlaypump;
    BKNI_EventHandle event;
    NEXUS_StcChannelHandle stcChannel;
    NEXUS_StcChannelSettings stcSettings;
    NEXUS_PidChannelHandle videoPidChannel, audioPidChannel;
    NEXUS_DisplayHandle display;
    NEXUS_VideoWindowHandle window;
    NEXUS_VideoDecoderHandle videoDecoder;
    NEXUS_VideoDecoderStartSettings videoProgram;
    NEXUS_AudioDecoderHandle audioDecoder;
    NEXUS_AudioDecoderStartSettings audioProgram;
    NEXUS_PlatformSettings platformSettings;
    NEXUS_PlatformConfiguration platformConfig;
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_HdmiOutputStatus hdmiStatus;
    NEXUS_DisplaySettings displaySettings;
    NEXUS_Error rc;
#endif
    FILE *vfile, *afile;
    const char *vfname = VIDEO_FILE_NAME;
    const char *afname = AUDIO_FILE_NAME;

    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init(&platformSettings);
    NEXUS_Platform_GetConfiguration(&platformConfig);

    BKNI_CreateEvent(&event);

    videoPlaypump = NEXUS_Playpump_Open(0, NULL);
    audioPlaypump = NEXUS_Playpump_Open(1, NULL);

    /* use stdio for file I/O to keep the example simple. */
    vfile = fopen(vfname, "rb");
    if (!vfile) {
        fprintf(stderr, "can't open file:%s\n", vfname);
        goto error;
    }

    afile = fopen(afname, "rb");
    if (!afile) {
        fprintf(stderr, "can't open file:%s\n", afname);
        goto error;
    }

    /* setup playpumps */
    NEXUS_Playpump_GetSettings(videoPlaypump, &playpumpSettings);
    playpumpSettings.transportType = TRANSPORT_TYPE;
    playpumpSettings.dataCallback.callback = play_callback;
    playpumpSettings.dataCallback.context = event;
    NEXUS_Playpump_SetSettings(videoPlaypump, &playpumpSettings);
    NEXUS_Playpump_Start(videoPlaypump);

    NEXUS_Playpump_GetSettings(audioPlaypump, &playpumpSettings);
    playpumpSettings.transportType = TRANSPORT_TYPE;
    playpumpSettings.dataCallback.callback = play_callback;
    playpumpSettings.dataCallback.context = event;
    NEXUS_Playpump_SetSettings(audioPlaypump, &playpumpSettings);
    NEXUS_Playpump_Start(audioPlaypump);

    /* setup stc channel */
    NEXUS_StcChannel_GetDefaultSettings(0, &stcSettings);
    stcSettings.timebase = NEXUS_Timebase_e0;
    stcSettings.mode = NEXUS_StcChannelMode_eAuto;
    stcChannel = NEXUS_StcChannel_Open(0, &stcSettings);

    videoPidChannel = NEXUS_Playpump_OpenPidChannel(videoPlaypump, VIDEO_PID, NULL);
    audioPidChannel = NEXUS_Playpump_OpenPidChannel(audioPlaypump, AUDIO_PID, NULL);

    /* setup decoders. both decoders get same stc channel for lipsync */
    NEXUS_VideoDecoder_GetDefaultStartSettings(&videoProgram);
    videoProgram.codec = VIDEO_CODEC;
    videoProgram.pidChannel = videoPidChannel;
    videoProgram.stcChannel = stcChannel;

    NEXUS_AudioDecoder_GetDefaultStartSettings(&audioProgram);
    audioProgram.codec = AUDIO_CODEC;
    audioProgram.pidChannel = audioPidChannel;
    audioProgram.stcChannel = stcChannel;
    
    /* bring up video decoder and display */
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

    videoDecoder = NEXUS_VideoDecoder_Open(0, NULL); /* take default capabilities */
    NEXUS_VideoWindow_AddInput(window, NEXUS_VideoDecoder_GetConnector(videoDecoder));

    NEXUS_VideoDecoder_Start(videoDecoder, &videoProgram);

    /* bring up audio decoder and dacs */
    audioDecoder = NEXUS_AudioDecoder_Open(0, NULL); /* take default capabilities */
#if NEXUS_NUM_AUDIO_DACS
    NEXUS_AudioOutput_AddInput(
        NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]),
        NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
#endif
    NEXUS_AudioDecoder_Start(audioDecoder, &audioProgram);

    while (1) {
        void *vbuffer, *abuffer;
        size_t vbuffer_size, abuffer_size;
        int nVideo, nAudio;
        NEXUS_Error rc;
        NEXUS_VideoDecoderStatus vStatus;
        NEXUS_AudioDecoderStatus aStatus;
        uint32_t stc;

        if (NEXUS_Playpump_GetBuffer(videoPlaypump, &vbuffer, &vbuffer_size))
            break;

        if (NEXUS_Playpump_GetBuffer(audioPlaypump, &abuffer, &abuffer_size))
            break;

        /* if both playpumps have no space available, wait for dataCallback from either one */
        if (vbuffer_size==0 && abuffer_size==0) {
            BKNI_WaitForEvent(event, BKNI_INFINITE);
            continue;
        }

        /* The first call to get_buffer will return the entire playback buffer.
        If we use it, we're going to have to wait until the descriptor is complete,
        and then we're going to underflow. So use a max size. */
#define MAX_READ (188*1024)
        if (vbuffer_size) {
            if (vbuffer_size > MAX_READ)
                vbuffer_size = MAX_READ;
          
            nVideo = fread(vbuffer, 1, vbuffer_size, vfile);

            if (nVideo < 0) goto error;
            if (nVideo == 0) {
                /* wait for the video decoder to reach the end of the content before looping both PES files */
                while (1) {
                    NEXUS_VideoDecoderStatus status;
                    NEXUS_VideoDecoder_GetStatus(videoDecoder, &status);
                    if (!status.queueDepth) break;
                }
                fseek(vfile, 0, SEEK_SET);
                NEXUS_Playpump_Flush(videoPlaypump);

                fseek(afile, 0, SEEK_SET);
                NEXUS_Playpump_Flush(audioPlaypump);
                printf("seek to beginning of PES files and flush playpumps\n");
                continue;
            }
            else {
                /* played (nVideo) bytes */
                rc = NEXUS_Playpump_WriteComplete(videoPlaypump, 0, nVideo);
                BDBG_ASSERT(!rc);
            }
        }

        if (abuffer_size) {
            if (abuffer_size > MAX_READ)
                abuffer_size = MAX_READ;

            nAudio = fread(abuffer, 1, abuffer_size, afile);

            if (nAudio < 0) goto error;
            if (nAudio == 0) {
                /* do nothing here and handle the looping in video instead */
            }
            else {
                /* played (nAudio) bytes */
                rc = NEXUS_Playpump_WriteComplete(audioPlaypump, 0, nAudio);
                BDBG_ASSERT(!rc);
            }
        }

#if 1 /* simple decode status */
        if (vbuffer_size && nVideo) {
            NEXUS_VideoDecoder_GetStatus(videoDecoder, &vStatus);
            NEXUS_StcChannel_GetStc(videoProgram.stcChannel, &stc);
            printf("video %.4dx%.4d, pts %#x, stc %#x (diff %5d) fifo=%d%%\n",
                vStatus.source.width, vStatus.source.height, vStatus.pts, stc, vStatus.ptsStcDifference, vStatus.fifoSize?(vStatus.fifoDepth*100)/vStatus.fifoSize:0);
            NEXUS_AudioDecoder_GetStatus(audioDecoder, &aStatus);
            printf("audio            pts %#x, stc %#x (diff %5d) fifo=%d%%\n\n",
                aStatus.pts, stc, aStatus.ptsStcDifference, aStatus.fifoSize?(aStatus.fifoDepth*100)/aStatus.fifoSize:0);
        }
#endif

    }
    return 0;

error:
    return 1;
}
#else /* NEXUS_HAS_VIDEO_DECODER */
int main(void)
{
    printf("This application is not supported on this platform (needs video decoder)!\n");
    return 0;
}
#endif
