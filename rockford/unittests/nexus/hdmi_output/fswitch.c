/******************************************************************************
* (c) 2015 Broadcom Corporation
*
* This program is the proprietary software of Broadcom Corporation and/or its
* licensors, and may only be used, duplicated, modified or distributed pursuant
* to the terms and conditions of a separate, written license agreement executed
* between you and Broadcom (an "Authorized License").  Except as set forth in
* an Authorized License, Broadcom grants no license (express or implied), right
* to use, or waiver of any kind with respect to the Software, and Broadcom
* expressly reserves all rights in and to the Software and all intellectual
* property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
* HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
* NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
*
* Except as expressly set forth in the Authorized License,
*
* 1. This program, including its structure, sequence and organization,
*    constitutes the valuable trade secrets of Broadcom, and you shall use all
*    reasonable efforts to protect the confidentiality thereof, and to use
*    this information only in connection with your use of Broadcom integrated
*    circuit products.
*
* 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
*    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
*    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
*    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
*    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
*    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
*
* 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
*    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
*    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
*    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
*    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
*    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
*    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
*    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
******************************************************************************/
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <signal.h>
#include "bstd.h"
#include "bkni.h"

#include "nexus_platform.h"
#include "nexus_video_decoder.h"
#include "nexus_stc_channel.h"
#include "nexus_display.h"
#include "nexus_video_window.h"
#include "nexus_video_input.h"
#include "nexus_component_output.h"
#include "nexus_playback.h"
#include "nexus_file.h"
#include "namevalue.h"
#include "examples_lib.h"

BDBG_MODULE(fswitch);

const char *g_teststreams[] = {
    "streams/black-1080p-60s-fswitch.ts",
    "streams/black-480p-60s-fswitch.ts"
};

#define STREAM_TIMEOUT 30*1000
#define TICK_TIMEOUT    1*1000

static unsigned file_running;
static unsigned quit_flag = 0;

static int file_exists(const char *fname)
{
    FILE *file = fopen(fname, "r");

    if (file) {
        fclose(file);
        return 1;
    }
    return 0;
}

static void endOfStreamCallback(void *context, int param)
{
    BSTD_UNUSED(context);
    BSTD_UNUSED(param);
    file_running = 0;
}

void sig_handler(int signo)
{
    if (signo == SIGINT) {
        printf("Catching SIGINT\n");
        quit_flag = 1;
    }
}

static void print_usage(void)
{
    char formatlist[1024] = "";
    unsigned n = 0;
    unsigned i;
#if NEXUS_HAS_HDMI_OUTPUT
    {
        NEXUS_HdmiOutputHandle hdmiOutput;
        hdmiOutput = NEXUS_HdmiOutput_Open(NEXUS_ALIAS_ID + 0, NULL);
        if (hdmiOutput) {
            NEXUS_HdmiOutputStatus status;
            if (!NEXUS_HdmiOutput_GetStatus(hdmiOutput, &status) && status.connected) {
                for (i = 0 ; i < NEXUS_VideoFormat_eMax ; i++) {
                    if (status.videoFormatSupported[i]) {
                        const char *s = lookup_name(g_videoFormatStrs, i);
                        if (s) {
                            n += snprintf(&formatlist[n], sizeof(formatlist)-n, "%s%s", n?",":"", s);
                            if (n >= sizeof(formatlist)) break;
                        }
                    }
                }
            }
            NEXUS_HdmiOutput_Close(hdmiOutput);
        }
    }
#endif
    if (!formatlist[0]) {
        for (i = 0 ; g_videoFormatStrs[i].name ; i++) {
            n += snprintf(&formatlist[n], sizeof(formatlist)-n, "%s%s", n?",":"", g_videoFormatStrs[i].name);
            if (n >= sizeof(formatlist)) break;
        }
    }
    n += snprintf(&formatlist[n], sizeof(formatlist)-n, "\n");

    printf(
        "Usage: fswitch OPTIONS\n"
        "  -format %s",
        formatlist
        );
    printf(
        "  -timeout {0|xx}           0 = forever (^C to terminate), xx=seconds\n"
        );
}

int main(int argc, char **argv)
{
    NEXUS_PlatformSettings platformSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_StcChannelHandle stcChannel;
    NEXUS_StcChannelSettings stcSettings;
    NEXUS_PidChannelHandle videoPidChannel;
    NEXUS_DisplayHandle display;
    NEXUS_DisplaySettings displaySettings;
    NEXUS_VideoFormat display_format = NEXUS_VideoFormat_eUnknown;
    NEXUS_VideoWindowHandle window;
    NEXUS_VideoDecoderHandle videoDecoder;
    NEXUS_VideoDecoderStartSettings videoProgram;
    NEXUS_VideoDecoderStatus videoDecoderStatus;
    NEXUS_HdmiOutputStatus hdmiOutputStatus;
    NEXUS_FilePlayHandle file;
    NEXUS_PlaypumpHandle playpump;
    NEXUS_PlaybackHandle playback;
    NEXUS_PlaybackSettings playbackSettings;
    NEXUS_Error rc;
    NEXUS_PlaybackPidChannelSettings playbackPidCfg;

    int curarg = 1;
    const char *fname;
    unsigned findex;

    unsigned timeout = 60;

    unsigned prev_frame_rate;
    unsigned curr_frame_rate;
    unsigned prev_unstable_cnt;
    unsigned curr_unstable_cnt;

    unsigned start_time;
    unsigned curr_time;
    unsigned prev_time;
    unsigned tick_duration;

#if NEXUS_HAS_HDMI_OUTPUT
#else
    printf("ERROR. Nexus must be built with HDMI to demonstrate unstable format detection\n");
    return -1;
#endif

    if(!file_exists(g_teststreams[0])) {
        printf("ERROR. Stream file \"%s\" does not exist\n", g_teststreams[0]);
        return -1;
    }

    if(!file_exists(g_teststreams[1])) {
        printf("ERROR. Stream file \"%s\" does not exist\n", g_teststreams[1]);
        return -1;
    }

    if (signal(SIGINT, sig_handler) == SIG_ERR)
        printf("\ncan't catch SIGINT\n");

    while (argc > curarg) {
        if (!strcmp(argv[curarg], "-h") || !strcmp(argv[curarg], "--help")) {
            print_usage();
            return 0;
        }
        else if (!strcmp(argv[curarg], "-format") && argc > curarg + 1) {
            display_format = lookup(g_videoFormatStrs, argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-timeout") && argc > curarg + 1) {
            timeout = atoi(argv[++curarg]);
        }
        else {
            print_usage();
            return -1;
        }
        curarg++;
    }

    /* Bring up all modules for a platform in a default configuraiton for this platform */
    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init(&platformSettings);
    NEXUS_Platform_GetConfiguration(&platformConfig);

    if (platformConfig.outputs.hdmi[0] == NULL) {
        printf("ERROR. No HDMI output is available.\n");
        NEXUS_Platform_Uninit();
        return -1;
    }

    start_time = b_get_time();
    curr_time = start_time;
    prev_time = start_time;
    prev_frame_rate = 0;
    curr_frame_rate = 0;
    prev_unstable_cnt = 0;
    curr_unstable_cnt = 0;
    findex = 1;

    while (quit_flag == 0) {
        if (findex == 1) findex = 0;
        else             findex = 1;
        fname = g_teststreams[findex];

        printf("Opening ... %s (%s)\n", fname, lookup_name(g_videoFormatStrs, display_format));
        file = NEXUS_FilePlay_OpenPosix(fname, NULL);
        if (!file) {
            fprintf(stderr, "can't open file:%s\n", fname);
            NEXUS_Platform_Uninit();
            return -1;
        }
        file_running = 1;
        printf("Time   - FrameRate  UC\n");

        playpump = NEXUS_Playpump_Open(0, NULL);
        assert(playpump);
        playback = NEXUS_Playback_Create();
        assert(playback);

        NEXUS_StcChannel_GetDefaultSettings(0, &stcSettings);
        stcSettings.timebase = NEXUS_Timebase_e0;
        stcSettings.mode = NEXUS_StcChannelMode_eAuto;
        stcChannel = NEXUS_StcChannel_Open(0, &stcSettings);

        NEXUS_Playback_GetSettings(playback, &playbackSettings);
        playbackSettings.playpump = playpump;
        playbackSettings.playpumpSettings.transportType = NEXUS_TransportType_eTs;
        playbackSettings.stcChannel = stcChannel;
        playbackSettings.endOfStreamAction = NEXUS_PlaybackLoopMode_ePause;
        playbackSettings.endOfStreamCallback.callback = endOfStreamCallback;
        playbackSettings.endOfStreamCallback.context = 0;
        NEXUS_Playback_SetSettings(playback, &playbackSettings);
        videoDecoder = NEXUS_VideoDecoder_Open(0, NULL);

        NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidCfg);
        playbackPidCfg.pidSettings.pidType = NEXUS_PidType_eVideo;
        playbackPidCfg.pidTypeSettings.video.codec = NEXUS_VideoCodec_eH264;
        playbackPidCfg.pidTypeSettings.video.decoder = videoDecoder;
        playbackPidCfg.pidTypeSettings.video.index = true;

        videoPidChannel = NEXUS_Playback_OpenPidChannel(playback, 0x1001, &playbackPidCfg);

        NEXUS_VideoDecoder_GetDefaultStartSettings(&videoProgram);
        videoProgram.codec = NEXUS_VideoCodec_eH264;
        videoProgram.pidChannel = videoPidChannel;
        videoProgram.stcChannel = stcChannel;

        /* Bring up video display and outputs */
        display = NEXUS_Display_Open(0, NULL);

#if NEXUS_NUM_HDMI_OUTPUTS
        rc = NEXUS_Display_AddOutput(display, NEXUS_HdmiOutput_GetVideoConnector(platformConfig.outputs.hdmi[0]));
        BDBG_ASSERT(!rc);
#endif

        NEXUS_Display_GetSettings(display, &displaySettings);

        if (display_format == NEXUS_VideoFormat_eUnknown)
            display_format = displaySettings.format;

        displaySettings.format = display_format;
        rc = NEXUS_Display_SetSettings(display, &displaySettings);
        BDBG_ASSERT(!rc);

        window = NEXUS_VideoWindow_Open(display, 0);
        NEXUS_VideoWindow_AddInput(window, NEXUS_VideoDecoder_GetConnector(videoDecoder));

        rc = NEXUS_VideoDecoder_Start(videoDecoder, &videoProgram);

        /* Start playback */
        rc = NEXUS_Playback_Start(playback, file, NULL);
        BDBG_ASSERT(!rc);

        prev_time = b_get_time();
        while (file_running) {
            if (quit_flag == 1) break;

            curr_time = b_get_time();
            if (timeout != 0) {
                if ((curr_time - start_time) / 1000 >= timeout)
                    quit_flag = 1;
            }
            tick_duration = curr_time - prev_time;

            BKNI_Sleep(100);
            if (tick_duration >= TICK_TIMEOUT) {
                prev_time = curr_time;

                NEXUS_VideoDecoder_GetStatus(videoDecoder, &videoDecoderStatus);
                curr_frame_rate = videoDecoderStatus.frameRate;
                NEXUS_HdmiOutput_GetStatus(platformConfig.outputs.hdmi[0], &hdmiOutputStatus) ;
                curr_unstable_cnt = hdmiOutputStatus.txHardwareStatus.unstableFormatDetectedCounter;
                if ((curr_frame_rate != prev_frame_rate) || (curr_unstable_cnt != prev_unstable_cnt)) {
                    printf("% 6d - %5s      % 2d\n",
                        (curr_time - start_time) / 1000,
                        lookup_name(g_videoFrameRateStrs, curr_frame_rate),
                        curr_unstable_cnt
                    );

                    prev_frame_rate = curr_frame_rate;
                    prev_unstable_cnt = curr_unstable_cnt;
                }
            }
        }

        NEXUS_Playback_Stop(playback);
        NEXUS_VideoDecoder_Stop(videoDecoder);
        NEXUS_FilePlay_Close(file);
        NEXUS_VideoWindow_Close(window);
        NEXUS_Display_Close(display);
        NEXUS_StcChannel_Close(stcChannel);
        NEXUS_Playback_Destroy(playback);
        NEXUS_VideoDecoder_Close(videoDecoder);
        NEXUS_Playpump_Close(playpump);
    }

    printf("... exiting\n");
    NEXUS_Platform_Uninit();

    return (curr_unstable_cnt != 0) ? -1 : 0;
}
