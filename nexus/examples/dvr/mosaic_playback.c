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
******************************************************************************/
#include "nexus_platform.h"
#include <stdio.h>

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

BDBG_MODULE(mosaic_playback);

/* the following define the input file and its characteristics -- these will vary by input file */
#define FILE_NAME "videos/mosaic_6avc_6mp2_cif.ts"
#define VIDEO_CODEC NEXUS_VideoCodec_eH264
#define BEGIN_VIDEO_PID 0x1001

#define NUM_MOSAICS 6

struct {
    unsigned short videoPid;
    unsigned short pcrPid;
} g_programs[NUM_MOSAICS] = {
    {0x1001, 0x100},
    {0x1002, 0x100},
    {0x1003, 0x100},
    {0x1004, 0x100},
    {0x1005, 0x100},
    {0x1007, 0x100},
};

int main(void)
{
    NEXUS_VideoDecoderHandle videoDecoder[NUM_MOSAICS];
    NEXUS_VideoDecoderStartSettings videoProgram[NUM_MOSAICS];
    NEXUS_DisplayHandle display;
    NEXUS_VideoWindowHandle parentWindow;
    NEXUS_VideoWindowHandle mosaicWindow[NUM_MOSAICS];
    NEXUS_VideoWindowSettings windowSettings;
    NEXUS_PlatformSettings platformSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_Timebase timebase = NEXUS_Timebase_e0;
    NEXUS_TimebaseSettings timebaseSettings;
    NEXUS_FilePlayHandle file;
    NEXUS_PlaypumpHandle playpump;
    NEXUS_PlaybackHandle playback;
    NEXUS_PlaybackSettings playbackSettings;
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_HdmiOutputStatus hdmiStatus;
#endif
    NEXUS_DisplaySettings displaySettings;
    NEXUS_Error rc;
    const char *fname = FILE_NAME;
    int i;
    NEXUS_VideoFormatInfo formatInfo;
    unsigned mosaic_width;

    /* Bring up all modules for a platform in a default configuraiton for this platform */
    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init(&platformSettings);
    NEXUS_Platform_GetConfiguration(&platformConfig);

    BKNI_Memset(videoDecoder, 0, sizeof(videoDecoder));

    playpump = NEXUS_Playpump_Open(0, NULL);
    BDBG_ASSERT(playpump);
    playback = NEXUS_Playback_Create();
    BDBG_ASSERT(playback);

    file = NEXUS_FilePlay_OpenPosix(fname, NULL);
    if (!file) {
        fprintf(stderr, "can't open file:%s\n", fname);
        return -1;
    }

    NEXUS_Playback_GetSettings(playback, &playbackSettings);
    playbackSettings.playpump = playpump;
    NEXUS_Playback_SetSettings(playback, &playbackSettings);

    /* bring up display */
    NEXUS_Display_GetDefaultSettings(&displaySettings);
    displaySettings.format = NEXUS_VideoFormat_e720p;
    display = NEXUS_Display_Open(0, &displaySettings);
    parentWindow = NEXUS_VideoWindow_Open(display, 0);

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
        if ( !hdmiStatus.videoFormatSupported[displaySettings.format] ) {
            displaySettings.format = hdmiStatus.preferredVideoFormat;
            NEXUS_Display_SetSettings(display, &displaySettings);
        }
    }
#endif

    NEXUS_VideoFormat_GetInfo(displaySettings.format, &formatInfo);
    mosaic_width = formatInfo.width / NUM_MOSAICS;

    /* bring up decoder and connect to display */
    for (i=0;i<NUM_MOSAICS;i++) {
        NEXUS_VideoDecoderOpenMosaicSettings openSettings;
        NEXUS_StcChannelSettings stcSettings;
        NEXUS_PlaybackPidChannelSettings playbackPidSettings;

        NEXUS_VideoDecoder_GetDefaultStartSettings(&videoProgram[i]);
        videoProgram[i].codec = VIDEO_CODEC;

        NEXUS_VideoDecoder_GetDefaultOpenMosaicSettings(&openSettings);
        openSettings.maxWidth = 352; /* CIF */
        openSettings.maxHeight = 288; /* CIF */
        videoDecoder[i] = NEXUS_VideoDecoder_OpenMosaic(0,
            NUM_MOSAICS-i-1, /* purposefully reorder the mosaic numbering. it should not matter. */
            &openSettings);
        BDBG_ASSERT(videoDecoder[i]);

        NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
        playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eVideo;
        playbackPidSettings.pidTypeSettings.video.codec = videoProgram[i].codec;
        playbackPidSettings.pidTypeSettings.video.index = true;
        playbackPidSettings.pidTypeSettings.video.decoder = videoDecoder[i];
        videoProgram[i].pidChannel = NEXUS_Playback_OpenPidChannel(playback, BEGIN_VIDEO_PID + i, &playbackPidSettings);

        NEXUS_StcChannel_GetDefaultSettings(i, &stcSettings);
        stcSettings.timebase = timebase;
        stcSettings.autoConfigTimebase = false;
        stcSettings.mode = NEXUS_StcChannelMode_eAuto; /* PVR */
        stcSettings.stcIndex = 0; /* must have shared STC for all mosaics on a single video decoder */
        videoProgram[i].stcChannel = NEXUS_StcChannel_Open(i, &stcSettings);

        mosaicWindow[i] = NEXUS_VideoWindow_OpenMosaic(parentWindow, i);
        NEXUS_VideoWindow_GetSettings(mosaicWindow[i], &windowSettings);
        windowSettings.position.x = 50 + (mosaic_width-50) * i;
        windowSettings.position.y = 50 + 30 * i;
        windowSettings.position.width = mosaic_width;
        windowSettings.position.height = mosaic_width*3/4;
        windowSettings.zorder = NUM_MOSAICS-i;
        windowSettings.visible = true;
        rc = NEXUS_VideoWindow_SetSettings(mosaicWindow[i], &windowSettings);
        BDBG_ASSERT(!rc);
    }

    NEXUS_Playback_GetSettings(playback, &playbackSettings);
    playbackSettings.stcChannel = videoProgram[0].stcChannel; /* stc trick modes are not available because of the shared STC */
    rc = NEXUS_Playback_SetSettings(playback, &playbackSettings);
    BDBG_ASSERT(!rc);

    /* share one timebase. ideally the timebase is set to the pcr associated with the current audio. */
    NEXUS_Timebase_GetSettings(timebase, &timebaseSettings);
    timebaseSettings.sourceType = NEXUS_TimebaseSourceType_eFreeRun;
    rc = NEXUS_Timebase_SetSettings(timebase, &timebaseSettings);
    BDBG_ASSERT(!rc);

    /* connect */
    for (i=0;i<NUM_MOSAICS;i++) {
        rc = NEXUS_VideoWindow_AddInput(mosaicWindow[i], NEXUS_VideoDecoder_GetConnector(videoDecoder[i]));
        BDBG_ASSERT(!rc);
    }

    /* Start Decoders */
    for (i=0;i<NUM_MOSAICS;i++) {
        rc = NEXUS_VideoDecoder_Start(videoDecoder[i], &videoProgram[i]);
        BDBG_ASSERT(!rc);
    }

    /* Start playback */
    rc = NEXUS_Playback_Start(playback, file, NULL);
    BDBG_ASSERT(!rc);

#if 1
    for (;;) {
        for (i=0;i<NUM_MOSAICS;i++) {
            NEXUS_VideoDecoderStatus status;
            uint32_t stc = 0;

            NEXUS_VideoDecoder_GetStatus(videoDecoder[i], &status);
            NEXUS_StcChannel_GetStc(videoProgram[i].stcChannel, &stc);
            printf("decode[%d] %dx%d, pts %#x, stc %#x (diff %d)\n",
                i, status.source.width, status.source.height, status.pts, stc, status.ptsStcDifference);
        }
        BKNI_Sleep(1000);
    }
#else
    BKNI_Sleep(2000);

    /* clean shutdown */
    BDBG_WRN(("shutdown"));
    NEXUS_Playback_Stop(playback);
    for (i=0;i<NUM_MOSAICS;i++) {
        NEXUS_VideoDecoder_Stop(videoDecoder[i]);
        NEXUS_VideoWindow_RemoveAllInputs(mosaicWindow[i]);
        NEXUS_VideoWindow_Close(mosaicWindow[i]);
        NEXUS_VideoInput_Shutdown(NEXUS_VideoDecoder_GetConnector(videoDecoder[i]));
        NEXUS_Playback_ClosePidChannel(playback, videoProgram[i].pidChannel);
        NEXUS_StcChannel_Close(videoProgram[i].stcChannel);
        NEXUS_VideoDecoder_Close(videoDecoder[i]);
    }
    NEXUS_VideoWindow_Close(parentWindow);
    NEXUS_Playback_Destroy(playback);
    NEXUS_Playpump_Close(playpump);
    NEXUS_Display_Close(display);
    NEXUS_FilePlay_Close(file);
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
