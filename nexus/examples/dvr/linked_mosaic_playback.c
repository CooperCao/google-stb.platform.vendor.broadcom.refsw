/******************************************************************************
 *    (c)2015 Broadcom Corporation
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
******************************************************************************/
#include "nexus_platform.h"
#include <stdio.h>
#include <string.h>

#if NEXUS_HAS_VIDEO_DECODER
#include "nexus_pid_channel.h"
#include "nexus_parser_band.h"
#include "nexus_video_decoder.h"
#include "nexus_stc_channel.h"
#include "nexus_timebase.h"
#include "nexus_display.h"
#include "nexus_video_window.h"
#include "nexus_video_input.h"
#include "nexus_mosaic_video_decoder.h"
#include "nexus_mosaic_display.h"
#include "nexus_composite_output.h"
#include "nexus_component_output.h"
#include "nexus_core_utils.h"
#if NEXUS_HAS_HDMI_OUTPUT
#include "nexus_hdmi_output.h"
#endif
#include "nexus_playback.h"
#include "nexus_file.h"
#include "bstd.h"
#include "bkni.h"
#include <stdlib.h>
#include <time.h>

BDBG_MODULE(linked_mosaic_playback);

static struct {
    const char *filename;
    NEXUS_VideoCodec codec;
    unsigned pid;
} g_program[] = {
    {"videos/t2-hd.mpg", NEXUS_VideoCodec_eMpeg2, 0x11},
    {"videos/elvisCostello.mpg", NEXUS_VideoCodec_eH264, 0x22},
    {"videos/riddick_avc_720p.mpg", NEXUS_VideoCodec_eH264, 0x1422},
    {"videos/herbie1AvcHD.mpg", NEXUS_VideoCodec_eH264, 0x1222},
    {"videos/herbie2AvcHD.mpg", NEXUS_VideoCodec_eH264, 0x1222},
    {"videos/symphonyAvcHD.mpg", NEXUS_VideoCodec_eH264, 0x1522}
};

#define NUM_MOSAICS 6

struct context {
    /* input */
    unsigned index;
    NEXUS_VideoWindowHandle parentWindow;
    NEXUS_VideoFormatInfo formatInfo;

    /* created */
    NEXUS_VideoDecoderHandle videoDecoder;
    NEXUS_VideoDecoderStartSettings videoProgram;
    NEXUS_VideoWindowHandle mosaicWindow;
    NEXUS_FilePlayHandle file;
    NEXUS_PlaypumpHandle playpump;
    NEXUS_PlaybackHandle playback;
};

static int start_mosaic(struct context *context, unsigned program)
{
    NEXUS_VideoDecoderOpenMosaicSettings openSettings;
    NEXUS_StcChannelSettings stcSettings;
    NEXUS_PlaybackPidChannelSettings playbackPidSettings;
    NEXUS_PlaybackSettings playbackSettings;
    NEXUS_VideoWindowSettings windowSettings;
    unsigned avdIndex = context->index/(NUM_MOSAICS/2);
    unsigned mosaicIndex = context->index%(NUM_MOSAICS/2);
    int rc;

    if (context->videoDecoder) {
        return BERR_TRACE(-1);
    }
    context->playpump = NEXUS_Playpump_Open(NEXUS_ANY_ID, NULL);
    context->playback = NEXUS_Playback_Create();

    context->file = NEXUS_FilePlay_OpenPosix(g_program[program].filename, NULL);
    if (!context->file) {
        fprintf(stderr, "can't open file:%s\n", g_program[program].filename);
        return -1;
    }

    NEXUS_Playback_GetSettings(context->playback, &playbackSettings);
    playbackSettings.playpump = context->playpump;
    NEXUS_Playback_SetSettings(context->playback, &playbackSettings);

    NEXUS_VideoDecoder_GetDefaultStartSettings(&context->videoProgram);
    context->videoProgram.codec = g_program[program].codec;

    NEXUS_VideoDecoder_GetDefaultOpenMosaicSettings(&openSettings);
    openSettings.maxWidth = 1920;
    openSettings.maxHeight = 1080;
    if (avdIndex == 1) {
        openSettings.linkedDevice.enabled = true;
        openSettings.linkedDevice.avdIndex = 0;
    }
    context->videoDecoder = NEXUS_VideoDecoder_OpenMosaic(avdIndex, mosaicIndex, &openSettings);

    NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
    playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eVideo;
    playbackPidSettings.pidTypeSettings.video.codec = context->videoProgram.codec;
    playbackPidSettings.pidTypeSettings.video.index = true;
    playbackPidSettings.pidTypeSettings.video.decoder = context->videoDecoder;
    context->videoProgram.pidChannel = NEXUS_Playback_OpenPidChannel(context->playback, g_program[program].pid, &playbackPidSettings);

    NEXUS_StcChannel_GetDefaultSettings(0, &stcSettings);
    stcSettings.timebase = NEXUS_Timebase_e0;
    stcSettings.autoConfigTimebase = false;
    stcSettings.mode = NEXUS_StcChannelMode_eAuto; /* PVR */
    stcSettings.stcIndex = 0; /* must have shared STC for all mosaics on a single video decoder */
    context->videoProgram.stcChannel = NEXUS_StcChannel_Open(NEXUS_ANY_ID, &stcSettings);

    context->mosaicWindow = NEXUS_VideoWindow_OpenMosaic(context->parentWindow, context->index);
    NEXUS_VideoWindow_GetSettings(context->mosaicWindow, &windowSettings);
    windowSettings.position.width = context->formatInfo.width/((NUM_MOSAICS+1)/2);
    windowSettings.position.height = context->formatInfo.height/2;
    windowSettings.position.x = windowSettings.position.width * mosaicIndex;
    windowSettings.position.y = windowSettings.position.height * avdIndex;
    windowSettings.visible = true;
    rc = NEXUS_VideoWindow_SetSettings(context->mosaicWindow, &windowSettings);
    BDBG_ASSERT(!rc);

    NEXUS_Playback_GetSettings(context->playback, &playbackSettings);
    playbackSettings.stcChannel = context->videoProgram.stcChannel; /* stc trick modes are not available because of the shared STC */
    rc = NEXUS_Playback_SetSettings(context->playback, &playbackSettings);
    BDBG_ASSERT(!rc);

    rc = NEXUS_VideoWindow_AddInput(context->mosaicWindow, NEXUS_VideoDecoder_GetConnector(context->videoDecoder));
    BDBG_ASSERT(!rc);
    rc = NEXUS_VideoDecoder_Start(context->videoDecoder, &context->videoProgram);
    BDBG_ASSERT(!rc);
    rc = NEXUS_Playback_Start(context->playback, context->file, NULL);
    BDBG_ASSERT(!rc);

    return 0;
}

static void stop_mosaic(struct context *context)
{
    if (!context->videoDecoder) return;
    NEXUS_Playback_Stop(context->playback);
    NEXUS_VideoDecoder_Stop(context->videoDecoder);
    NEXUS_Playback_ClosePidChannel(context->playback, context->videoProgram.pidChannel);
    NEXUS_StcChannel_Close(context->videoProgram.stcChannel);
    NEXUS_Playback_Destroy(context->playback);
    NEXUS_Playpump_Close(context->playpump);
    NEXUS_FilePlay_Close(context->file);
    NEXUS_VideoWindow_Close(context->mosaicWindow);
    NEXUS_VideoDecoder_Close(context->videoDecoder);
    context->videoDecoder = NULL;
}

static struct context g_context[NUM_MOSAICS];

#define IS_PRIMARY(index) ((index) < NUM_MOSAICS/2)
static unsigned num_decode_started(bool primary)
{
    unsigned i, total = 0;
    unsigned offset = primary?0:(NUM_MOSAICS/2);
    for (i=0;i<NUM_MOSAICS/2;i++) {
        if (g_context[i+offset].videoDecoder) total++;
    }
    return total;
}

int main(void)
{
    NEXUS_DisplayHandle display;
    NEXUS_VideoWindowHandle parentWindow;
    NEXUS_PlatformSettings platformSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_HdmiOutputStatus hdmiStatus;
    NEXUS_DisplaySettings displaySettings;
    NEXUS_Error rc;
    int i;
    NEXUS_VideoFormatInfo formatInfo;

    srand(time(NULL));
    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init(&platformSettings);
    NEXUS_Platform_GetConfiguration(&platformConfig);

    NEXUS_Display_GetDefaultSettings(&displaySettings);
    displaySettings.format = NEXUS_VideoFormat_e720p;
    display = NEXUS_Display_Open(0, &displaySettings);
    parentWindow = NEXUS_VideoWindow_Open(display, 0);

    if (platformConfig.outputs.component[0]) {
        NEXUS_Display_AddOutput(display, NEXUS_ComponentOutput_GetConnector(platformConfig.outputs.component[0]));
    }
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
    NEXUS_VideoFormat_GetInfo(displaySettings.format, &formatInfo);

    for (i=0;i<NUM_MOSAICS;i++) {
        g_context[i].index = i;
        g_context[i].parentWindow = parentWindow;
        g_context[i].formatInfo = formatInfo;
    }

#if 0
    /* simple: start in order */
    for (i=0;i<NUM_MOSAICS;i++) {
        start_mosaic(&g_context[i], i);
    }
    BSTD_UNUSED(num_decode_started);
#else
    /* complex: start in random order, but with some dependencies */
    for (i=0;i<1000;i++) {
        unsigned index = rand()%NUM_MOSAICS;
        if (g_context[index].videoDecoder) {
            /* cannot stop last primary decoder unless no secondarys are started */
            if (IS_PRIMARY(index) && num_decode_started(true) == 1 && num_decode_started(false)) continue;

            BDBG_WRN(("stop %d", index));
            stop_mosaic(&g_context[index]);
        }
        else {
            /* cannot start secondary decoder unless one primary is started */
            if (!IS_PRIMARY(index) && !num_decode_started(true)) continue;

            BDBG_WRN(("start %d", index));
            start_mosaic(&g_context[index], index);
        }
        BKNI_Sleep(500);
    }
#endif

#if 0
    for (;;) {
        for (i=0;i<NUM_MOSAICS;i++) {
            NEXUS_VideoDecoderStatus status;
            uint32_t stc = 0;

            NEXUS_VideoDecoder_GetStatus(g_context[i].videoDecoder, &status);
            NEXUS_StcChannel_GetStc(g_context[i].videoProgram.stcChannel, &stc);
            printf("decode[%d] %dx%d, pts %#x, stc %#x (diff %d)\n",
                i, status.source.width, status.source.height, status.pts, stc, status.ptsStcDifference);
        }
        BKNI_Sleep(1000);
    }
#else
    BDBG_WRN(("press ENTER to shutdown"));
    getchar();

    i = NUM_MOSAICS-1;
    do {
        stop_mosaic(&g_context[i]);
    } while (i--);

    NEXUS_VideoWindow_Close(parentWindow);
    NEXUS_Display_Close(display);
    NEXUS_Platform_Uninit();
#endif

    return 0;
}
#else /* NEXUS_HAS_VIDEO_DECODER */
int main(void)
{
    printf("This application is not supported on this platform (needs video decoder)!\n");
    return 0;
}
#endif
