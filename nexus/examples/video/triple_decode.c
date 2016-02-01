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
 *****************************************************************************/
#include "nexus_platform.h"
#include <stdio.h>
#if NEXUS_HAS_DISPLAY
#include "nexus_video_decoder.h"
#include "nexus_stc_channel.h"
#include "nexus_display.h"
#include "nexus_video_window.h"
#include "nexus_video_input.h"
#include "nexus_composite_output.h"
#include "nexus_component_output.h"
#if NEXUS_HAS_HDMI_OUTPUT
#include "nexus_hdmi_output.h"
#endif
#include "nexus_playback.h"
#include "nexus_file.h"
#include <string.h>
#include <assert.h>
#include "bstd.h"
#include "bkni.h"

struct context {
    unsigned index;
    unsigned stcIndex;
    const char *filename;
    unsigned pid;
    NEXUS_VideoCodec codec;
    NEXUS_StcChannelHandle stcChannel;
    NEXUS_VideoWindowHandle window;
    NEXUS_VideoDecoderHandle videoDecoder;
    NEXUS_FilePlayHandle file;
    NEXUS_PlaypumpHandle playpump;
    NEXUS_PlaybackHandle playback;
    NEXUS_PidChannelHandle videoPidChannel;
};

static int start_decode(struct context *context)
{
    NEXUS_StcChannelSettings stcSettings;
    NEXUS_VideoDecoderStartSettings videoProgram;
    NEXUS_PlaybackSettings playbackSettings;
    NEXUS_PlaybackPidChannelSettings playbackPidSettings;
    int rc;
    
    context->playpump = NEXUS_Playpump_Open(context->index, NULL);
    BDBG_ASSERT(context->playpump);
    context->playback = NEXUS_Playback_Create();
    BDBG_ASSERT(context->playback);

    context->file = NEXUS_FilePlay_OpenPosix(context->filename, NULL);
    if (!context->file) {
        fprintf(stderr, "can't open file:%s\n", context->filename);
        return -1;
    }

    NEXUS_StcChannel_GetDefaultSettings(context->stcIndex, &stcSettings);
    stcSettings.mode = NEXUS_StcChannelMode_eAuto;
    context->stcChannel = NEXUS_StcChannel_Open(context->stcIndex, &stcSettings);

    NEXUS_Playback_GetSettings(context->playback, &playbackSettings);
    playbackSettings.playpump = context->playpump;
    playbackSettings.playpumpSettings.transportType = NEXUS_TransportType_eTs;
    playbackSettings.stcChannel = context->stcChannel;
    rc = NEXUS_Playback_SetSettings(context->playback, &playbackSettings);
    BDBG_ASSERT(!rc);

    context->videoDecoder = NEXUS_VideoDecoder_Open(context->index, NULL);
    if (!context->videoDecoder) {
        return BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }
    
    rc = NEXUS_VideoWindow_AddInput(context->window, NEXUS_VideoDecoder_GetConnector(context->videoDecoder));
    BDBG_ASSERT(!rc);

    NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
    playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eVideo;
    playbackPidSettings.pidTypeSettings.video.codec = context->codec;
    playbackPidSettings.pidTypeSettings.video.index = true;
    playbackPidSettings.pidTypeSettings.video.decoder = context->videoDecoder;
    context->videoPidChannel = NEXUS_Playback_OpenPidChannel(context->playback, context->pid, &playbackPidSettings);

    NEXUS_VideoDecoder_GetDefaultStartSettings(&videoProgram);
    videoProgram.codec = context->codec;
    videoProgram.pidChannel = context->videoPidChannel;
    videoProgram.stcChannel = context->stcChannel;

    rc = NEXUS_VideoDecoder_Start(context->videoDecoder, &videoProgram);
    BDBG_ASSERT(!rc);

    rc = NEXUS_Playback_Start(context->playback, context->file, NULL);
    BDBG_ASSERT(!rc);
    
    return 0;
}

static int stop_decode(struct context *context)
{
    NEXUS_Playback_Stop(context->playback);
    NEXUS_VideoDecoder_Stop(context->videoDecoder);
    NEXUS_Playback_ClosePidChannel(context->playback, context->videoPidChannel);
    NEXUS_VideoDecoder_Close(context->videoDecoder);
    NEXUS_StcChannel_Close(context->stcChannel);
    NEXUS_FilePlay_Close(context->file);
    NEXUS_Playback_Destroy(context->playback);
    NEXUS_Playpump_Close(context->playpump);
    NEXUS_VideoWindow_Close(context->window);
    return 0;
}

int main(void)
{
    NEXUS_PlatformSettings platformSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_DisplayHandle display, display2;
    NEXUS_VideoWindowHandle window[3];
    NEXUS_DisplaySettings displaySettings;
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_HdmiOutputStatus hdmiStatus;
    NEXUS_Error errCode;
#endif
    struct context context[3];
    int rc;

    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    /* bypass memory issues */
    memset(platformSettings.videoDecoderModuleSettings.supportedCodecs, 0, sizeof(platformSettings.videoDecoderModuleSettings.supportedCodecs));
    platformSettings.videoDecoderModuleSettings.supportedCodecs[NEXUS_VideoCodec_eH264] = true;
    NEXUS_Platform_Init(&platformSettings);
    NEXUS_Platform_GetConfiguration(&platformConfig);

    NEXUS_Display_GetDefaultSettings(&displaySettings);
    displaySettings.timebase = NEXUS_Timebase_e0;
    displaySettings.displayType = NEXUS_DisplayType_eAuto;
    displaySettings.format = NEXUS_VideoFormat_e480p;
    display = NEXUS_Display_Open(0, &displaySettings);
#if NEXUS_NUM_COMPONENT_OUTPUTS
    NEXUS_Display_AddOutput(display, NEXUS_ComponentOutput_GetConnector(platformConfig.outputs.component[0]));
#endif 
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_Display_AddOutput(display, NEXUS_HdmiOutput_GetVideoConnector(platformConfig.outputs.hdmi[0]));
    errCode = NEXUS_HdmiOutput_GetStatus(platformConfig.outputs.hdmi[0], &hdmiStatus);
    if ( !errCode && hdmiStatus.connected )
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
    window[0] = NEXUS_VideoWindow_Open(display, 0);
    window[1] = NEXUS_VideoWindow_Open(display, 1);

    NEXUS_Display_GetDefaultSettings(&displaySettings);
    displaySettings.timebase = NEXUS_Timebase_e1;
    displaySettings.displayType = NEXUS_DisplayType_eAuto;
    displaySettings.format = NEXUS_VideoFormat_eNtsc;
    display2 = NEXUS_Display_Open(1, &displaySettings);
#if NEXUS_NUM_COMPOSITE_OUTPUTS
    NEXUS_Display_AddOutput(display2, NEXUS_CompositeOutput_GetConnector(platformConfig.outputs.composite[0]));
#endif 
    window[2] = NEXUS_VideoWindow_Open(display2, 0);

    BKNI_Memset(&context, 0, sizeof(context));
    
    context[0].index = 0;
    context[0].stcIndex = 1;
    context[0].window = window[0];
    context[0].filename = "videos/ironman2_AVC_15M.ts";
    context[0].codec = NEXUS_VideoCodec_eH264;
    context[0].pid = 0x101;
    rc = start_decode(&context[0]);
    
    context[1].index = 1;
    context[1].stcIndex = 0;
    context[1].window = window[1];
    context[1].filename = "videos/avatar_AVC_15M.ts";
    context[1].codec = NEXUS_VideoCodec_eH264;
    context[1].pid = 0x101;
    rc = start_decode(&context[1]);

    context[2].index = 2;
    context[2].stcIndex = 2;
    context[2].window = window[2];
    context[2].filename = "videos/avatar_AVC_15M.ts";
    context[2].codec = NEXUS_VideoCodec_eH264;
    context[2].pid = 0x101;
    rc = start_decode(&context[2]);

    printf("Press ENTER to quit\n");
    getchar();

    /* Shutdown */
    rc = stop_decode(&context[2]);
    rc = stop_decode(&context[1]);
    rc = stop_decode(&context[0]);
    NEXUS_Display_Close(display2);
    NEXUS_Display_Close(display);
    NEXUS_Platform_Uninit();

    return 0;
}
#else
int main(void)
{
    printf("This application is not supported on this platform\n");
    return 0;
}
#endif
