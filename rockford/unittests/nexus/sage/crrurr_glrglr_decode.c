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
#include "nexus_platform.h"
#include <stdio.h>

#if NEXUS_HAS_VIDEO_DECODER
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

BDBG_MODULE(sec_unsec);

#define USE_SVP 1
#define SECURE_UNSECURE_ALL 1

#undef NEXUS_NUM_COMPOSITE_OUTPUTS
#define NEXUS_NUM_COMPOSITE_OUTPUTS 0
#undef NEXUS_NUM_COMPONENT_OUTPUTS
#define NEXUS_NUM_COMPONENT_OUTPUTS 0

#define UHD_MAIN_1080p_PIP            0
#if UHD_MAIN_1080p_PIP/* 4K */
#define FILENAME_MAIN    "videos/big_buck_bunny_4kp60_10bit.ts"
#define MAIN_TS_TYPE     NEXUS_TransportType_eTs
#define MAIN_VIDEO_PID   0x1001
#define MAIN_VIDEO_CODEC NEXUS_VideoCodec_eH265
#define MAIN_AUDIO_PID   0x1002
#define MAIN_AUDIO_CODEC NEXUS_AudioCodec_eAc3
#define FILENAME_PIP    "videos/BRCM_Logo_7445C_HEVC_1920x1080p60.v1.ts"
#define PIP_TS_TYPE     NEXUS_TransportType_eTs
#define PIP_VIDEO_PID   0x1001
#define PIP_VIDEO_CODEC NEXUS_VideoCodec_eH265
#define PIP_AUDIO_PID   0x1012
#define PIP_AUDIO_CODEC NEXUS_AudioCodec_eAc3
#else
#define FILENAME_MAIN    "videos/cnnticker.mpg"
#define MAIN_TS_TYPE     NEXUS_TransportType_eTs
#define MAIN_VIDEO_PID   0x21
#define MAIN_VIDEO_CODEC NEXUS_VideoCodec_eMpeg2
#define MAIN_AUDIO_PID   0x22
#define MAIN_AUDIO_CODEC NEXUS_AudioCodec_eMpeg
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
    NEXUS_VideoWindowHandle window, window1;
    NEXUS_VideoDecoderHandle videoDecoder;
    NEXUS_FilePlayHandle file;
    NEXUS_PlaypumpHandle playpump;
    NEXUS_PlaybackHandle playback;
    NEXUS_PidChannelHandle videoPidChannel, audioPidChannel;
    NEXUS_AudioDecoderHandle audioDecoder;
    NEXUS_AudioDecoderStartSettings audioProgram;
    NEXUS_AudioDecoderPrimerHandle primer;
};

static bool g_usePrimer = true;
static int start_decode(struct context *context)
{
    NEXUS_StcChannelSettings stcSettings;
    NEXUS_VideoDecoderSettings settings;
    NEXUS_VideoDecoderStartSettings videoProgram;
    NEXUS_PlaybackSettings playbackSettings;
    NEXUS_PlaybackPidChannelSettings playbackPidSettings;
    int rc;
#if USE_SVP
    NEXUS_VideoDecoderOpenSettings videoDecoderopenSettings;
    NEXUS_PlatformConfiguration platformConfig;
#endif

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

#if USE_SVP
    if(context->index == 0) {
        /* Context 0 is secure */
        NEXUS_Platform_GetConfiguration(&platformConfig);
        NEXUS_VideoDecoder_GetDefaultOpenSettings(&videoDecoderopenSettings);
        videoDecoderopenSettings.cdbHeap = platformConfig.heap[NEXUS_VIDEO_SECURE_HEAP];
        videoDecoderopenSettings.secureVideo = true;
        context->videoDecoder = NEXUS_VideoDecoder_Open(context->index, &videoDecoderopenSettings);
    }
    else
#endif
    {
        /* Clear buffers, not CRR or URR*/
        context->videoDecoder = NEXUS_VideoDecoder_Open(context->index, NULL);
    }
    if(context->index == 0) {
        NEXUS_VideoDecoder_GetSettings(context->videoDecoder, &settings);
        settings.maxWidth  = UHD_MAIN_1080p_PIP? 3840:1920;
        settings.maxHeight = UHD_MAIN_1080p_PIP? 2176:1088;
        NEXUS_VideoDecoder_SetSettings(context->videoDecoder, &settings);
    }
    rc = NEXUS_VideoWindow_AddInput(context->window, NEXUS_VideoDecoder_GetConnector(context->videoDecoder));
    BDBG_ASSERT(!rc);
    if (context->window1) {
        rc = NEXUS_VideoWindow_AddInput(context->window1, NEXUS_VideoDecoder_GetConnector(context->videoDecoder));
        BDBG_ASSERT(!rc);
    }
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

    if ( g_usePrimer ) {
        context->primer = NEXUS_AudioDecoderPrimer_Create(NULL);
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

static int stop_decode(struct context *context) {

    NEXUS_Playback_Stop(context->playback);


    NEXUS_VideoDecoder_Stop(context->videoDecoder);

    NEXUS_AudioDecoderPrimer_Close( context->primer );

    if ( context->stcChannel ) NEXUS_StcChannel_Close( context->stcChannel );

    NEXUS_Playback_Destroy(context->playback);
    NEXUS_Playpump_Close(context->playpump);

    NEXUS_FilePlay_Close( context->file );

    NEXUS_VideoDecoder_Close( context->videoDecoder );

    return 0;
}

#if NEXUS_NUM_COMPOSITE_OUTPUTS
#define HDSD_SIMUL 1
#endif

int main(int argc, char **argv)
{
    NEXUS_PlatformSettings platformSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_DisplayHandle display;
    NEXUS_VideoFormat displayFormat = NEXUS_VideoFormat_e480p;/* for default HD display */
    NEXUS_VideoWindowHandle window[2];
    /* need second display on SD path only if it exists */
#if HDSD_SIMUL
    NEXUS_DisplayHandle  display1;
    NEXUS_VideoWindowHandle window1[2];
#endif
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
#if USE_SVP
    NEXUS_AudioDecoderOpenSettings audioDecoderOpenSettings;
    NEXUS_MemoryConfigurationSettings memConfigSettings;
    int i, j;
#endif

    while (curarg < argc) {
        if ( !strcmp(argv[curarg], "-no_primer") ) {
            g_usePrimer = false;
        }
        else if (!strcmp(argv[curarg], "-t") && curarg+1 < argc) {
            /* set switch time between pip and main audio */
            audioTime = strtoul(argv[++curarg], NULL, 0);
            if ( audioTime < 1 ) audioTime = 1;
        }
        else if (!strcmp(argv[curarg], "-4k") && curarg+1 < argc) {
            /* set switch time between pip and main audio */
            displayFormat = NEXUS_VideoFormat_e3840x2160p60hz;
            BDBG_WRN(("Set 3840x2160p60 4:2:0 display format!"));
        }
        curarg++;
    }

    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
#if USE_SVP
    /*display 0/decoder 0 secure, others unsecure */
    NEXUS_GetDefaultMemoryConfigurationSettings(&memConfigSettings);
#if SECURE_UNSECURE_ALL
    for (i = 0; i < NEXUS_NUM_VIDEO_DECODERS; i++)
    {
        memConfigSettings.videoDecoder[i].secure = NEXUS_SecureVideo_eBoth;
    }
    for (i = 0; i < NEXUS_NUM_DISPLAYS; i++)
    {
        for (j = 0; j < NEXUS_NUM_VIDEO_WINDOWS;j++)
        {
            memConfigSettings.display[i].window[j].secure = NEXUS_SecureVideo_eBoth;
        }
    }
#else
    printf("Minimum memory footprint secure/unsecure\n");
    /*Minimum memory alloc necessary to do secure/unsecure*/
    memConfigSettings.videoDecoder[0].secure = NEXUS_SecureVideo_eSecure;
    memConfigSettings.display[0].window[0].secure = NEXUS_SecureVideo_eSecure;
#if HDSD_SIMUL
    printf("HDSD simul\n");
    memConfigSettings.display[1].window[0].secure = NEXUS_SecureVideo_eSecure;
#endif
#endif
    NEXUS_Platform_MemConfigInit(&platformSettings, &memConfigSettings);
#else
    NEXUS_Platform_Init(&platformSettings);
#endif
    NEXUS_Platform_GetConfiguration(&platformConfig);

    display = NEXUS_Display_Open(0, NULL);
#if HDSD_SIMUL
    display1 = NEXUS_Display_Open(1, NULL);
#endif
#if NEXUS_NUM_COMPONENT_OUTPUTS
    NEXUS_Display_AddOutput(display, NEXUS_ComponentOutput_GetConnector(platformConfig.outputs.component[0]));
#endif
#if HDSD_SIMUL && NEXUS_NUM_COMPOSITE_OUTPUTS
    NEXUS_Display_AddOutput(display1, NEXUS_CompositeOutput_GetConnector(platformConfig.outputs.composite[0]));
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
        displaySettings.format = displayFormat;
        if(displayFormat == NEXUS_VideoFormat_e3840x2160p60hz) {
            NEXUS_HdmiOutputSettings settings;
            NEXUS_HdmiOutput_GetSettings(platformConfig.outputs.hdmi[0], &settings);
            settings.colorSpace = NEXUS_ColorSpace_eYCbCr420;
            NEXUS_HdmiOutput_SetSettings(platformConfig.outputs.hdmi[0], &settings);
        }
        NEXUS_Display_SetSettings(display, &displaySettings);
    }
#endif

    window[0] = NEXUS_VideoWindow_Open(display, 0);
    window[1] = NEXUS_VideoWindow_Open(display, 1);
#if HDSD_SIMUL
    window1[0] = NEXUS_VideoWindow_Open(display1, 0);
    window1[1] = NEXUS_VideoWindow_Open(display1, 1);
#endif
#if BLEND_MAIN_AND_PIP
    {
        NEXUS_VideoWindowSettings settings;
        NEXUS_VideoWindow_GetSettings(window[1], &settings);
        settings.sourceBlendFactor = NEXUS_CompositorBlendFactor_eConstantAlpha;
        settings.destBlendFactor = NEXUS_CompositorBlendFactor_eInverseConstantAlpha;
        settings.constantAlpha = 0x80;
        NEXUS_VideoWindow_SetSettings(window[1], &settings);
    }
#endif

#if USE_SVP
    NEXUS_AudioDecoder_GetDefaultOpenSettings(&audioDecoderOpenSettings);
    /*audioDecoderOpenSettings.cdbHeap = platformConfig.heap[NEXUS_VIDEO_SECURE_HEAP];*/
    audioDecoder = NEXUS_AudioDecoder_Open(0, &audioDecoderOpenSettings);
#else
    audioDecoder = NEXUS_AudioDecoder_Open(0, NULL);
#endif

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
#if HDSD_SIMUL
    context[0].window1 = window1[0];
#endif
    context[0].filename      = FILENAME_MAIN;
    context[0].transportType = MAIN_TS_TYPE;
    context[0].videoPid      = MAIN_VIDEO_PID;
    context[0].videoCodec    = MAIN_VIDEO_CODEC;
    context[0].audioPid      = MAIN_AUDIO_PID;
    context[0].audioCodec    = MAIN_AUDIO_CODEC;
    context[0].audioDecoder  = audioDecoder;
    rc = start_decode(&context[0]);
    if (rc) return -1;
    context[1].index = 1;
    context[1].window = window[1];
#if HDSD_SIMUL
    context[1].window1 = window1[1];
#endif
    context[1].filename      = FILENAME_PIP;
    context[1].transportType = PIP_TS_TYPE;
    context[1].videoPid      = PIP_VIDEO_PID;
    context[1].videoCodec    = PIP_VIDEO_CODEC;
    context[1].audioPid      = PIP_AUDIO_PID;
    context[1].audioCodec    = PIP_AUDIO_CODEC;
    context[1].audioDecoder  = audioDecoder;
    rc = start_decode(&context[1]);
    if (rc) return -1;
    /* start audio */
    index = 0;

    if ( g_usePrimer == false ) {
        rc = NEXUS_AudioDecoder_Start(audioDecoder, &context[index].audioProgram);
        BDBG_ASSERT(!rc);
    }
    else {
        rc = NEXUS_AudioDecoderPrimer_StopPrimerAndStartDecode( context[index].primer, audioDecoder );
        BDBG_ASSERT(!rc);
    }

    while (1) {
        /* print status for 5 seconds, then toggle */
        unsigned count = audioTime * 2;
        while (count--) {
            NEXUS_AudioDecoderStatus status;
            NEXUS_VideoDecoderStatus videoStatus;
            NEXUS_AudioDecoder_GetStatus(audioDecoder, &status);
            NEXUS_VideoDecoder_GetStatus(context[0].videoDecoder, &videoStatus);
            BDBG_MSG(("context %d file=%s ", index, context[index].filename));
            BDBG_MSG(("video pts=%#x, stc=%#x, diff=%d", videoStatus.pts, videoStatus.pts - videoStatus.ptsStcDifference, videoStatus.ptsStcDifference));
            NEXUS_VideoDecoder_GetStatus(context[1].videoDecoder, &videoStatus);
            BDBG_MSG(("video pts=%#x, stc=%#x, diff=%d", videoStatus.pts, videoStatus.pts - videoStatus.ptsStcDifference, videoStatus.ptsStcDifference));
            BDBG_MSG(("audio pts=%#x, stc=%#x, diff=%d", status.pts, status.pts - status.ptsStcDifference, status.ptsStcDifference));
            BKNI_Sleep(500);
        }
        BDBG_WRN(("toggle t=%d" , audioTime*2));

        if ( g_usePrimer == false ) {
            NEXUS_AudioDecoder_Stop(audioDecoder);
        }
        else {
            NEXUS_AudioDecoderPrimer_StopDecodeAndStartPrimer( context[index].primer, audioDecoder );

            NEXUS_Playback_GetPidChannelSettings(context[index].playback, context[index].audioPidChannel, &playbackPidSettings);
            playbackPidSettings.pidTypeSettings.audio.primary = NULL;
            NEXUS_Playback_SetPidChannelSettings(context[index].playback, context[index].audioPidChannel, &playbackPidSettings);
        }

        if (++index == 2) index = 0;

        if ( g_usePrimer == false ) {
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
#if HDSD_SIMUL
    NEXUS_VideoWindow_Close(window1[1]);
    NEXUS_VideoWindow_Close(window1[0]);
    NEXUS_Display_Close(display1);
#endif
    NEXUS_Platform_Uninit();

    return 0;
}
#else /* NEXUS_HAS_VIDEO_DECODER */
int main(void)
{
    printf("This application is not supported on this platform (needs video decoder)!\n");
    return 0;
}
#endif
