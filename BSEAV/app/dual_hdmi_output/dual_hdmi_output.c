/******************************************************************************
 *    (c)2008-2013 Broadcom Corporation
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
/* Nexus example app: playback and decode */

#include "nexus_platform.h"
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
#include "nexus_playback.h"
#include "nexus_file.h"
#include "media_probe.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "bstd.h"
#include "bkni.h"

BDBG_MODULE(dual_hdmi_output);

/*
single process nexus app
dual output (including dual hdmi output)
no audio
uses nxclient/apps/utils for media probe
*/

struct output
{
    /* open */
    NEXUS_DisplayHandle display;
    NEXUS_VideoWindowHandle window;
    NEXUS_VideoDecoderHandle videoDecoder;
    NEXUS_PlaypumpHandle playpump;
    NEXUS_PlaybackHandle playback;
    NEXUS_StcChannelHandle stcChannel;

    /* start */
    bool started;
    NEXUS_FilePlayHandle file;
    NEXUS_PidChannelHandle videoPidChannel;
};

int open_output(struct output *p_output, unsigned index)
{
    NEXUS_StcChannelSettings stcSettings;
    NEXUS_PlaybackSettings playbackSettings;
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_HdmiOutputStatus hdmiStatus;
    NEXUS_DisplaySettings displaySettings;
#endif
    int rc;
    NEXUS_PlatformConfiguration platformConfig;

    memset(p_output, 0, sizeof(*p_output));

    p_output->playpump = NEXUS_Playpump_Open(NEXUS_ANY_ID, NULL);
    BDBG_ASSERT(p_output->playpump);
    p_output->playback = NEXUS_Playback_Create();
    BDBG_ASSERT(p_output->playback);

    NEXUS_StcChannel_GetDefaultSettings(0, &stcSettings);
    stcSettings.timebase = NEXUS_Timebase_e0 + index;
    stcSettings.mode = NEXUS_StcChannelMode_eAuto;
    p_output->stcChannel = NEXUS_StcChannel_Open(NEXUS_ANY_ID, &stcSettings);

    NEXUS_Playback_GetSettings(p_output->playback, &playbackSettings);
    playbackSettings.playpump = p_output->playpump;
    playbackSettings.stcChannel = p_output->stcChannel;
    NEXUS_Playback_SetSettings(p_output->playback, &playbackSettings);

    p_output->display = NEXUS_Display_Open(index, NULL);
    p_output->window = NEXUS_VideoWindow_Open(p_output->display, 0);

    NEXUS_Platform_GetConfiguration(&platformConfig);
    /* composite always goes to display1 */
#if NEXUS_NUM_COMPOSITE_OUTPUTS
    if (index == 1) {
        NEXUS_Display_AddOutput(p_output->display, NEXUS_CompositeOutput_GetConnector(platformConfig.outputs.composite[0]));
    }
#endif
    /* component/hdmi are paired with their display */
#if NEXUS_NUM_COMPONENT_OUTPUTS
    if (platformConfig.outputs.component[index]) {
        NEXUS_Display_AddOutput(p_output->display, NEXUS_ComponentOutput_GetConnector(platformConfig.outputs.component[index]));
    }
#endif
#if NEXUS_NUM_HDMI_OUTPUTS
    if (index < NEXUS_NUM_HDMI_OUTPUTS && platformConfig.outputs.hdmi[index]) {
        NEXUS_Display_AddOutput(p_output->display, NEXUS_HdmiOutput_GetVideoConnector(platformConfig.outputs.hdmi[index]));
        rc = NEXUS_HdmiOutput_GetStatus(platformConfig.outputs.hdmi[index], &hdmiStatus);
        if ( !rc && hdmiStatus.connected )
        {
            /* If current display format is not supported by monitor, switch to monitor's preferred format.
               If other connected outputs do not support the preferred format, a harmless error will occur. */
            NEXUS_Display_GetSettings(p_output->display, &displaySettings);
            if ( !hdmiStatus.videoFormatSupported[displaySettings.format] ) {
                displaySettings.format = hdmiStatus.preferredVideoFormat;
                NEXUS_Display_SetSettings(p_output->display, &displaySettings);
            }
        }
    }
#endif

    /* bring up decoder and connect to display */
    p_output->videoDecoder = NEXUS_VideoDecoder_Open(index, NULL); /* take default capabilities */
    rc = NEXUS_VideoWindow_AddInput(p_output->window, NEXUS_VideoDecoder_GetConnector(p_output->videoDecoder));
    if (rc) return BERR_TRACE(rc);

    return 0;
}

void stop_output(struct output *p_output);

int start_output(struct output *p_output, const char *filename)
{
    NEXUS_PlaybackPidChannelSettings playbackPidSettings;
    NEXUS_VideoDecoderStartSettings videoProgram;
    int rc;
    struct probe_results probe;
    NEXUS_PlaybackSettings playbackSettings;

    if (p_output->started) {
        stop_output(p_output);
    }

    rc = probe_media(filename, &probe);
    if (rc) return rc;

    if (probe.num_video == 0) {
        BDBG_WRN(("no video found"));
        return -1;
    }

    p_output->file = NEXUS_FilePlay_OpenPosix(filename, filename);
    if (!p_output->file) {
        fprintf(stderr, "can't open file:%s\n", filename);
        return -1;
    }

    NEXUS_Playback_GetSettings(p_output->playback, &playbackSettings);
    playbackSettings.playpumpSettings.transportType = probe.transportType;
    NEXUS_Playback_SetSettings(p_output->playback, &playbackSettings);

    NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
    playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eVideo;
    playbackPidSettings.pidTypeSettings.video.codec = probe.video[0].codec; /* must be told codec for correct handling */
    playbackPidSettings.pidTypeSettings.video.index = true;
    playbackPidSettings.pidTypeSettings.video.decoder = p_output->videoDecoder;
    p_output->videoPidChannel = NEXUS_Playback_OpenPidChannel(p_output->playback, probe.video[0].pid, &playbackPidSettings);

    /* Set up decoder Start structures now. We need to know the audio codec to properly set up the audio outputs. */
    NEXUS_VideoDecoder_GetDefaultStartSettings(&videoProgram);
    videoProgram.codec = probe.video[0].codec;
    videoProgram.pidChannel = p_output->videoPidChannel;
    videoProgram.stcChannel = p_output->stcChannel;

    rc = NEXUS_VideoDecoder_Start(p_output->videoDecoder, &videoProgram);
    if (rc) return BERR_TRACE(rc);

    rc = NEXUS_Playback_Start(p_output->playback, p_output->file, NULL);
    if (rc) return BERR_TRACE(rc);

    p_output->started = true;
    return 0;
}

void stop_output(struct output *p_output)
{
    if (!p_output->started) return;
    NEXUS_VideoDecoder_Stop(p_output->videoDecoder);
    NEXUS_Playback_Stop(p_output->playback);
    NEXUS_FilePlay_Close(p_output->file);
    NEXUS_Playback_ClosePidChannel(p_output->playback, p_output->videoPidChannel);
    p_output->started = false;
}

void close_output(struct output *p_output)
{
    stop_output(p_output);
    NEXUS_Playback_Destroy(p_output->playback);
    NEXUS_Playpump_Close(p_output->playpump);
    NEXUS_VideoDecoder_Close(p_output->videoDecoder);
    NEXUS_VideoWindow_Close(p_output->window);
    NEXUS_Display_Close(p_output->display);
    NEXUS_StcChannel_Close(p_output->stcChannel);
}

int main(void)
{
    NEXUS_PlatformSettings platformSettings;
    NEXUS_Error rc;
    struct output output[2];
    unsigned index = 0;

    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    rc = NEXUS_Platform_Init(&platformSettings);
    if (rc) return -1;

    rc = open_output(&output[0], 0);
    BDBG_ASSERT(!rc);
    rc = open_output(&output[1], 1);
    BDBG_ASSERT(!rc);

    while (1) {
        char buf[256];
        fprintf(stdout, "dual_hdmi_output>");
        fflush(stdout);
        if (!fgets(buf, sizeof(buf), stdin)) break;
        buf[strlen(buf)-1] = 0; /* chop \n */
        if (!strcmp(buf, "?")) {
            printf(
            "? for help\n"
            "q to quit\n"
            "ls [subdir]\n"
            "0 or 1 to switch instance\n"
            "else start playback of filename\n"
            );
        }
        else if (!strcmp(buf, "q")) {
            break;
        }
        else if (!strcmp(buf, "0")) {
            index = 0;
            printf("output %d\n", index);
        }
        else if (!strcmp(buf, "1")) {
            index = 1;
            printf("output %d\n", index);
        }
        else if (strstr(buf, "ls") == buf) {
            system(buf);
        }
        else if (buf[0]) {
            start_output(&output[index], buf);
        }
    }

    close_output(&output[0]);
    close_output(&output[1]);

    NEXUS_Platform_Uninit();
    return 0;
}
