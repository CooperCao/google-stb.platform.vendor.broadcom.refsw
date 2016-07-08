/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
#include "nexus_platform.h"
#include "nexus_video_decoder.h"
#include "nexus_stc_channel.h"
#include "nexus_display.h"
#include "nexus_video_window.h"
#include "nexus_video_input.h"
#include "nexus_audio_decoder.h"
#include "nexus_audio_decoder_primer.h"
#include "nexus_component_output.h"
#include "nexus_composite_output.h"
#include "nexus_component_output.h"
#if NEXUS_HAS_HDMI_OUTPUT
#include "nexus_hdmi_output.h"
#endif
#include "nexus_playback.h"
#include "nexus_file.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "bstd.h"
#include "bkni.h"

BDBG_MODULE(playback_pip);

#define FILENAME_MAIN    "videos/spider_cc.mpg"
#define MAIN_TS_TYPE     NEXUS_TransportType_eTs 
#define MAIN_VIDEO_PID   0x11
#define MAIN_VIDEO_CODEC NEXUS_VideoCodec_eMpeg2
#define MAIN_AUDIO_PID   0x14
#define MAIN_AUDIO_CODEC NEXUS_AudioCodec_eAc3

#if 0
#define FILENAME_PIP    "videos/sm_10cd.dss"
#define PIP_TS_TYPE     NEXUS_TransportType_eDssEs
#define PIP_VIDEO_PID   0xA
#define PIP_VIDEO_CODEC NEXUS_VideoCodec_eMpeg2
#define PIP_AUDIO_PID   0xB
#define PIP_AUDIO_CODEC NEXUS_AudioCodec_eMpeg
#else
#define FILENAME_PIP    "videos/cnnticker.mpg"
#define PIP_TS_TYPE     NEXUS_TransportType_eTs
#define PIP_VIDEO_PID   0x21
#define PIP_VIDEO_CODEC NEXUS_VideoCodec_eMpeg2
#define PIP_AUDIO_PID   0x22
#define PIP_AUDIO_CODEC NEXUS_AudioCodec_eMpeg
#endif

struct context {
    unsigned index;
    const char *filename;
    unsigned videoPid;
    NEXUS_VideoCodec videoCodec;
    unsigned audioPid;
    NEXUS_AudioCodec audioCodec;
    NEXUS_TransportType transportType;
    NEXUS_StcChannelHandle stcChannel;
    NEXUS_VideoWindowHandle window;
    NEXUS_VideoDecoderHandle videoDecoder;
    NEXUS_FilePlayHandle file;
    NEXUS_PlaypumpHandle playpump;
    NEXUS_PlaybackHandle playback;
    NEXUS_PidChannelHandle videoPidChannel, audioPidChannel;
    NEXUS_AudioDecoderHandle audioDecoder;
    NEXUS_AudioDecoderStartSettings audioProgram;
    NEXUS_AudioDecoderPrimerHandle primer;
};

static int start_decode(struct context *context, bool usePrimer)
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

    NEXUS_StcChannel_GetDefaultSettings(context->index, &stcSettings);
    stcSettings.timebase = NEXUS_Timebase_e0;
    stcSettings.mode = NEXUS_StcChannelMode_eAuto;
    stcSettings.modeSettings.Auto.transportType = context->transportType;
    context->stcChannel = NEXUS_StcChannel_Open(context->index, &stcSettings);

    NEXUS_Playback_GetSettings(context->playback, &playbackSettings);
    playbackSettings.playpump = context->playpump;
    playbackSettings.playpumpSettings.transportType = context->transportType;
    playbackSettings.stcChannel = context->stcChannel;
    rc = NEXUS_Playback_SetSettings(context->playback, &playbackSettings);
    BDBG_ASSERT(!rc);

    context->videoDecoder = NEXUS_VideoDecoder_Open(context->index, NULL);
    rc = NEXUS_VideoWindow_AddInput(context->window, NEXUS_VideoDecoder_GetConnector(context->videoDecoder));
    BDBG_ASSERT(!rc);

    NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
    playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eVideo;
    playbackPidSettings.pidTypeSettings.video.codec = context->videoCodec;
    playbackPidSettings.pidTypeSettings.video.index = true;
    playbackPidSettings.pidTypeSettings.video.decoder = context->videoDecoder;
    context->videoPidChannel = NEXUS_Playback_OpenPidChannel(context->playback, context->videoPid, &playbackPidSettings);

    NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
    playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eAudio;
    playbackPidSettings.pidTypeSettings.audio.primary = context->audioDecoder;
    context->audioPidChannel = NEXUS_Playback_OpenPidChannel(context->playback, context->audioPid, &playbackPidSettings);

    NEXUS_VideoDecoder_GetDefaultStartSettings(&videoProgram);
    videoProgram.codec = context->videoCodec;
    videoProgram.pidChannel = context->videoPidChannel;
    videoProgram.stcChannel = context->stcChannel;

    rc = NEXUS_VideoDecoder_Start(context->videoDecoder, &videoProgram);
    BDBG_ASSERT(!rc);

    NEXUS_AudioDecoder_GetDefaultStartSettings(&context->audioProgram);
    context->audioProgram.codec = context->audioCodec;
    context->audioProgram.pidChannel = context->audioPidChannel;
    context->audioProgram.stcChannel = context->stcChannel;
    
    if (usePrimer) {
        context->primer = NEXUS_AudioDecoderPrimer_Open(context->audioDecoder);
        rc = NEXUS_AudioDecoderPrimer_Start( context->primer, &context->audioProgram);
        BDBG_ASSERT(!rc);
        /* regsiter primer with playback so on loop around primer can be flushed */
        NEXUS_Playback_GetPidChannelSettings(context->playback, context->audioPidChannel, &playbackPidSettings);
        playbackPidSettings.pidTypeSettings.audio.primer = context->primer;
        NEXUS_Playback_SetPidChannelSettings(context->playback, context->audioPidChannel, &playbackPidSettings);
    }

    rc = NEXUS_Playback_Start(context->playback, context->file, NULL);
    BDBG_ASSERT(!rc);

    return 0;
}

static int stop_decode(struct context *context) 
{
    NEXUS_Playback_Stop(context->playback);
    NEXUS_VideoDecoder_Stop(context->videoDecoder);
    if (context->primer) {
        NEXUS_AudioDecoderPrimer_Close(context->primer);
    }

    if ( context->stcChannel ) {
        NEXUS_StcChannel_Close( context->stcChannel );
    }

    NEXUS_Playback_ClosePidChannel(context->playback, context->videoPidChannel);
    NEXUS_Playback_ClosePidChannel(context->playback, context->audioPidChannel);
    NEXUS_Playback_Destroy(context->playback);
    NEXUS_Playpump_Close(context->playpump);

    NEXUS_FilePlay_Close( context->file );

    NEXUS_VideoDecoder_Close( context->videoDecoder );
    
    return 0;
}

int main(int argc, char **argv)
{
    NEXUS_PlatformSettings platformSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_DisplayCapabilities displayCap;
    NEXUS_DisplayHandle display;
    NEXUS_VideoWindowHandle window[2];
    struct context context[2];
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_HdmiOutputStatus hdmiStatus;
    NEXUS_DisplaySettings displaySettings;
#endif
    NEXUS_AudioDecoderHandle audioDecoder;
    int rc,curarg=1;
    unsigned index;
    NEXUS_PlaybackPidChannelSettings playbackPidSettings;
    int audioTime = 5;
    unsigned loops = 50;
    bool usePrimer = true;

#if NEXUS_AUDIO_MODULE_FAMILY != NEXUS_AUDIO_MODULE_FAMILY_APE_RAAGA
    usePrimer = false;
#endif

    while (curarg < argc) {
        if ( !strcmp(argv[curarg], "-no_primer") ) {
            usePrimer = false;
        }
        else if (!strcmp(argv[curarg], "-t") && curarg+1 < argc) {
            /* set switch time between pip and main audio */
            audioTime = strtoul(argv[++curarg], NULL, 0);
            if ( audioTime < 1 ) audioTime = 1;
        }
        curarg++;
    }

    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    rc = NEXUS_Platform_Init(&platformSettings);
    if (rc) return rc;
    NEXUS_Platform_GetConfiguration(&platformConfig);

    NEXUS_GetDisplayCapabilities(&displayCap);
    if (displayCap.display[0].numVideoWindows < 2) {
        BDBG_WRN(("No PIP support, nothing tested."));
        goto done;
    }

    display = NEXUS_Display_Open(0, NULL);

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

    window[0] = NEXUS_VideoWindow_Open(display, 0);
    window[1] = NEXUS_VideoWindow_Open(display, 1);

    audioDecoder = NEXUS_AudioDecoder_Open(0, NULL);
#if NEXUS_NUM_AUDIO_DACS
    NEXUS_AudioOutput_AddInput(
        NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]),
        NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
#endif
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_AudioOutput_AddInput(NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]),
                               NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
#endif
        
    BKNI_Memset(&context, 0, sizeof(context));

    context[0].index = 0;
    context[0].window = window[0];
    context[0].filename      = FILENAME_MAIN;
    context[0].transportType = MAIN_TS_TYPE;
    context[0].videoPid      = MAIN_VIDEO_PID;
    context[0].videoCodec    = MAIN_VIDEO_CODEC;
    context[0].audioPid      = MAIN_AUDIO_PID;
    context[0].audioCodec    = MAIN_AUDIO_CODEC;
    context[0].audioDecoder  = audioDecoder;
    rc = start_decode(&context[0], usePrimer);
    if (rc) return -1;

    context[1].index = 1;
    context[1].window = window[1];
    context[1].filename      = FILENAME_PIP;
    context[1].transportType = PIP_TS_TYPE;
    context[1].videoPid      = PIP_VIDEO_PID;
    context[1].videoCodec    = PIP_VIDEO_CODEC;
    context[1].audioPid      = PIP_AUDIO_PID;
    context[1].audioCodec    = PIP_AUDIO_CODEC;
    context[1].audioDecoder  = audioDecoder;
    rc = start_decode(&context[1], usePrimer);
    if (rc) return -1;

    /* start audio */
    index = 0;
        
    if (!context[index].primer) {
        rc = NEXUS_AudioDecoder_Start(audioDecoder, &context[index].audioProgram);
        BDBG_ASSERT(!rc);
    }
    else {
        rc = NEXUS_AudioDecoderPrimer_StopPrimerAndStartDecode( context[index].primer, audioDecoder );
        BDBG_ASSERT(!rc);
    }

    while (--loops) {
        /* print status for 5 seconds, then toggle */
        unsigned count = audioTime * 2;
        while (count--) {
            NEXUS_AudioDecoderStatus status;
            NEXUS_VideoDecoderStatus videoStatus;
            NEXUS_AudioDecoder_GetStatus(audioDecoder, &status);
            NEXUS_VideoDecoder_GetStatus(context[0].videoDecoder, &videoStatus);
            BDBG_WRN(("context %d file=%s ", index, context[index].filename));
            BDBG_WRN(("video pts=%#x, stc=%#x, diff=%d", videoStatus.pts, videoStatus.pts - videoStatus.ptsStcDifference, videoStatus.ptsStcDifference));
            NEXUS_VideoDecoder_GetStatus(context[1].videoDecoder, &videoStatus);
            BDBG_WRN(("video pts=%#x, stc=%#x, diff=%d", videoStatus.pts, videoStatus.pts - videoStatus.ptsStcDifference, videoStatus.ptsStcDifference));
            BDBG_WRN(("audio pts=%#x, stc=%#x, diff=%d", status.pts, status.pts - status.ptsStcDifference, status.ptsStcDifference));
            BKNI_Sleep(500);
        }
        BDBG_WRN(("toggle t=%d" , audioTime*2));
        
        if (!context[index].primer) {
            NEXUS_AudioDecoder_Stop(audioDecoder);
        }
        else {
            NEXUS_AudioDecoderPrimer_StopDecodeAndStartPrimer( context[index].primer, audioDecoder );

            NEXUS_Playback_GetPidChannelSettings(context[index].playback, context[index].audioPidChannel, &playbackPidSettings);
            playbackPidSettings.pidTypeSettings.audio.primary = NULL;
            NEXUS_Playback_SetPidChannelSettings(context[index].playback, context[index].audioPidChannel, &playbackPidSettings);
        } 

        if (++index == 2) index = 0;

        if (!context[index].primer) {
            rc = NEXUS_AudioDecoder_Start(audioDecoder, &context[index].audioProgram);
            BDBG_ASSERT(!rc);
        }
        else {
            NEXUS_Playback_GetPidChannelSettings(context[index].playback, context[index].audioPidChannel, &playbackPidSettings);
            playbackPidSettings.pidTypeSettings.audio.primary = audioDecoder;
            NEXUS_Playback_SetPidChannelSettings(context[index].playback, context[index].audioPidChannel, &playbackPidSettings);

            rc = NEXUS_AudioDecoderPrimer_StopPrimerAndStartDecode( context[index].primer , audioDecoder );
            BDBG_ASSERT(!rc);

        }
    }

    NEXUS_AudioDecoder_Stop(audioDecoder);
    stop_decode( &context[1] );
    stop_decode( &context[0] );
    NEXUS_AudioDecoder_Close( audioDecoder );
    NEXUS_VideoWindow_Close(window[1]);
    NEXUS_VideoWindow_Close(window[0]);
    NEXUS_Display_Close(display);

done:
    NEXUS_Platform_Uninit();
    return 0;
}
