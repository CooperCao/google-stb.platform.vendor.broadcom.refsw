/******************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#include <stdio.h>

#if NEXUS_HAS_AUDIO
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
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "bstd.h"
#include "bkni.h"

/* swap two decoders (pcm and compressed) between 2 audio tracks (for example, two languages) */
BDBG_MODULE(audio_dualtrack_swap);

#define FILE_NAME "videos/italyriviera_spiderman2_cc_q64.mpg"
#define VIDEO_CODEC NEXUS_VideoCodec_eMpeg2
#define VIDEO_PID 0x31
#define AUDIO_NUM 2
static struct {
    NEXUS_AudioCodec codec;
    unsigned pid;
} g_audio[AUDIO_NUM] = {
  {NEXUS_AudioCodec_eAc3, 0x34},
  {NEXUS_AudioCodec_eAc3, 0x14}
};

struct context {
    const char *filename;
    unsigned videoPid;
    NEXUS_VideoCodec videoCodec;
    NEXUS_TransportType transportType;
    NEXUS_StcChannelHandle stcChannel;
    NEXUS_VideoWindowHandle window;
    NEXUS_VideoDecoderHandle videoDecoder;
    NEXUS_FilePlayHandle file;
    NEXUS_PlaypumpHandle playpump;
    NEXUS_PlaybackHandle playback;
    NEXUS_PidChannelHandle videoPidChannel;
    NEXUS_AudioDecoderHandle audioDecoder;
    NEXUS_AudioDecoderHandle audioDecoderCompressed;
    struct {
        NEXUS_PidChannelHandle pidChannel;
        unsigned pid;
        NEXUS_AudioCodec codec;
        NEXUS_AudioDecoderStartSettings program;
        NEXUS_AudioDecoderPrimerHandle primer, secondaryPrimer;
    } audio[AUDIO_NUM]; /* toggle between 2 audio programs */
};

static int start_decode(struct context *context)
{
    NEXUS_StcChannelSettings stcSettings;
    NEXUS_VideoDecoderStartSettings videoProgram;
    NEXUS_PlaybackSettings playbackSettings;
    NEXUS_PlaybackPidChannelSettings playbackPidSettings;
    int rc;
    int i;

    context->playpump = NEXUS_Playpump_Open(0, NULL);
    BDBG_ASSERT(context->playpump);
    context->playback = NEXUS_Playback_Create();
    BDBG_ASSERT(context->playback);

    context->file = NEXUS_FilePlay_OpenPosix(context->filename, NULL);
    if (!context->file) {
        fprintf(stderr, "can't open file:%s\n", context->filename);
        return -1;
    }

    NEXUS_StcChannel_GetDefaultSettings(0, &stcSettings);
    stcSettings.timebase = NEXUS_Timebase_e0;
    stcSettings.mode = NEXUS_StcChannelMode_eAuto;
    stcSettings.modeSettings.Auto.transportType = context->transportType;
    context->stcChannel = NEXUS_StcChannel_Open(0, &stcSettings);

    NEXUS_Playback_GetSettings(context->playback, &playbackSettings);
    playbackSettings.playpump = context->playpump;
    playbackSettings.playpumpSettings.transportType = context->transportType;
    playbackSettings.stcChannel = context->stcChannel;
    rc = NEXUS_Playback_SetSettings(context->playback, &playbackSettings);
    BDBG_ASSERT(!rc);

    context->videoDecoder = NEXUS_VideoDecoder_Open(0, NULL);
    rc = NEXUS_VideoWindow_AddInput(context->window, NEXUS_VideoDecoder_GetConnector(context->videoDecoder));
    BDBG_ASSERT(!rc);

    NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
    playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eVideo;
    playbackPidSettings.pidTypeSettings.video.codec = context->videoCodec;
    playbackPidSettings.pidTypeSettings.video.index = true;
    playbackPidSettings.pidTypeSettings.video.decoder = context->videoDecoder;
    context->videoPidChannel = NEXUS_Playback_OpenPidChannel(context->playback, context->videoPid, &playbackPidSettings);


    NEXUS_VideoDecoder_GetDefaultStartSettings(&videoProgram);
    videoProgram.codec = context->videoCodec;
    videoProgram.pidChannel = context->videoPidChannel;
    videoProgram.stcChannel = context->stcChannel;

    rc = NEXUS_VideoDecoder_Start(context->videoDecoder, &videoProgram);
    BDBG_ASSERT(!rc);

    for (i = 0; i < AUDIO_NUM; i++)
    {
        NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
        playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eAudio;
        playbackPidSettings.pidTypeSettings.audio.primary = context->audioDecoder;
        playbackPidSettings.pidTypeSettings.audio.secondary = context->audioDecoderCompressed;
        context->audio[i].pidChannel = NEXUS_Playback_OpenPidChannel(context->playback, context->audio[i].pid, &playbackPidSettings);

        NEXUS_AudioDecoder_GetDefaultStartSettings(&context->audio[i].program);
        context->audio[i].program.codec = context->audio[i].codec;
        context->audio[i].program.pidChannel = context->audio[i].pidChannel;
        context->audio[i].program.stcChannel = context->stcChannel;

        context->audio[i].primer = NEXUS_AudioDecoderPrimer_Open(context->audioDecoder);
        rc = NEXUS_AudioDecoderPrimer_Start( context->audio[i].primer, &context->audio[i].program);
        BDBG_ASSERT(!rc);
        context->audio[i].secondaryPrimer = NEXUS_AudioDecoderPrimer_Open(context->audioDecoderCompressed);
        rc = NEXUS_AudioDecoderPrimer_Start( context->audio[i].secondaryPrimer, &context->audio[i].program);
        BDBG_ASSERT(!rc);
        /* register primer with playback so on loop around primer can be flushed */
        NEXUS_Playback_GetPidChannelSettings(context->playback, context->audio[i].pidChannel, &playbackPidSettings);
        playbackPidSettings.pidTypeSettings.audio.primer = context->audio[i].primer;
        playbackPidSettings.pidTypeSettings.audio.secondaryPrimer = context->audio[i].secondaryPrimer;
        NEXUS_Playback_SetPidChannelSettings(context->playback, context->audio[i].pidChannel, &playbackPidSettings);
    }

    rc = NEXUS_Playback_Start(context->playback, context->file, NULL);
    BDBG_ASSERT(!rc);

    return 0;
}

static void stop_decode(struct context *context)
{
    int i;

    NEXUS_Playback_Stop(context->playback);
    NEXUS_VideoDecoder_Stop(context->videoDecoder);
    for (i = 0; i < AUDIO_NUM; i++) {
        NEXUS_AudioDecoderPrimer_Close( context->audio[i].primer );
        NEXUS_AudioDecoderPrimer_Close( context->audio[i].secondaryPrimer );
    }
    NEXUS_StcChannel_Close( context->stcChannel );
    NEXUS_Playback_Destroy(context->playback);
    NEXUS_Playpump_Close(context->playpump);
    NEXUS_FilePlay_Close( context->file );
    NEXUS_VideoDecoder_Close( context->videoDecoder );
}

int main(void)
{
    NEXUS_PlatformSettings platformSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_DisplayHandle display;
    NEXUS_VideoWindowHandle window;
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_HdmiOutputStatus hdmiStatus;
    NEXUS_DisplaySettings displaySettings;
#endif
    NEXUS_AudioDecoderHandle audioDecoder;
    NEXUS_AudioDecoderHandle audioDecoderCompressed;
    int rc,i;
    unsigned index;
    NEXUS_PlaybackPidChannelSettings playbackPidSettings;
    struct context context;
    NEXUS_AudioCapabilities audioCapabilities;
    NEXUS_AudioOutputHandle audioDacHandle = NULL;
    NEXUS_AudioOutputHandle audioSpdifHandle = NULL;
    NEXUS_AudioOutputHandle audioHdmiHandle = NULL;

    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init(&platformSettings);
    NEXUS_Platform_GetConfiguration(&platformConfig);
    NEXUS_GetAudioCapabilities(&audioCapabilities);

    if (audioCapabilities.numDecoders < 2)
    {
        printf("This application is not supported on this platform (requires 2 decoders).\n");
        return 0;
    }

    if (audioCapabilities.numOutputs.dac > 0)
    {
        audioDacHandle = NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]);
    }

    if (audioCapabilities.numOutputs.spdif > 0)
    {
        audioSpdifHandle = NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]);
    }

    #if NEXUS_NUM_HDMI_OUTPUTS
    if (audioCapabilities.numOutputs.hdmi > 0)
    {
        audioHdmiHandle = NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]);
    }
    #endif

    display = NEXUS_Display_Open(0, NULL);

#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_Display_AddOutput(display, NEXUS_HdmiOutput_GetVideoConnector(platformConfig.outputs.hdmi[0]));
    rc = NEXUS_HdmiOutput_GetStatus(platformConfig.outputs.hdmi[0], &hdmiStatus);
    if ( !rc && hdmiStatus.connected ) {
        NEXUS_Display_GetSettings(display, &displaySettings);
        if ( !hdmiStatus.videoFormatSupported[displaySettings.format] ) {
            displaySettings.format = hdmiStatus.preferredVideoFormat;
            NEXUS_Display_SetSettings(display, &displaySettings);
        }
    }
#endif

    window = NEXUS_VideoWindow_Open(display, 0);

    audioDecoder = NEXUS_AudioDecoder_Open(0, NULL);
    audioDecoderCompressed = NEXUS_AudioDecoder_Open(1, NULL);
    if (audioDacHandle) {
        NEXUS_AudioOutput_AddInput(
            audioDacHandle,
            NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
    }
    #if NEXUS_NUM_HDMI_OUTPUTS
    if (audioHdmiHandle) {
        NEXUS_AudioOutput_AddInput(audioHdmiHandle,
                                   NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
    }
    #endif
    if (audioSpdifHandle) {
        NEXUS_AudioOutput_AddInput(audioSpdifHandle,
                                   NEXUS_AudioDecoder_GetConnector(audioDecoderCompressed, NEXUS_AudioDecoderConnectorType_eCompressed));
    }

    BKNI_Memset(&context, 0, sizeof(context));
    context.window = window;
    context.filename      = FILE_NAME;
    context.transportType = NEXUS_TransportType_eTs;
    context.videoPid      = VIDEO_PID;
    context.videoCodec    = VIDEO_CODEC;
    for (i = 0; i < AUDIO_NUM; i++) {
        context.audio[i].pid = g_audio[i].pid;
        context.audio[i].codec = g_audio[i].codec;
    }
    context.audioDecoder  = audioDecoder;
    context.audioDecoderCompressed = audioDecoderCompressed;
    rc = start_decode(&context);
    if (rc) return -1;

    index = 0;

    rc = NEXUS_AudioDecoderPrimer_StopPrimerAndStartDecode( context.audio[index].primer, audioDecoder );
    BDBG_ASSERT(!rc);
    rc = NEXUS_AudioDecoderPrimer_StopPrimerAndStartDecode( context.audio[index].secondaryPrimer, audioDecoderCompressed);
    BDBG_ASSERT(!rc);

    while (1) {
        BKNI_Sleep(2000);
        BDBG_WRN(("swap"));

        NEXUS_AudioDecoderPrimer_StopDecodeAndStartPrimer( context.audio[index].primer, audioDecoder );
        NEXUS_AudioDecoderPrimer_StopDecodeAndStartPrimer( context.audio[index].secondaryPrimer, audioDecoderCompressed);

        NEXUS_Playback_GetPidChannelSettings(context.playback, context.audio[index].pidChannel, &playbackPidSettings);
        playbackPidSettings.pidTypeSettings.audio.primary = NULL;
        playbackPidSettings.pidTypeSettings.audio.secondary = NULL;
        NEXUS_Playback_SetPidChannelSettings(context.playback, context.audio[index].pidChannel, &playbackPidSettings);

        if (++index == AUDIO_NUM) index = 0;

        NEXUS_Playback_GetPidChannelSettings(context.playback, context.audio[index].pidChannel, &playbackPidSettings);
        playbackPidSettings.pidTypeSettings.audio.primary = audioDecoder;
        playbackPidSettings.pidTypeSettings.audio.secondary = audioDecoderCompressed;
        NEXUS_Playback_SetPidChannelSettings(context.playback, context.audio[index].pidChannel, &playbackPidSettings);

        rc = NEXUS_AudioDecoderPrimer_StopPrimerAndStartDecode( context.audio[index].primer , audioDecoder );
        BDBG_ASSERT(!rc);
        rc = NEXUS_AudioDecoderPrimer_StopPrimerAndStartDecode( context.audio[index].secondaryPrimer , audioDecoderCompressed);
        BDBG_ASSERT(!rc);
    }

    NEXUS_AudioDecoder_Stop(audioDecoder);
    NEXUS_AudioDecoder_Stop(audioDecoderCompressed);
    stop_decode( &context );
    NEXUS_AudioDecoder_Close(audioDecoder);
    NEXUS_AudioDecoder_Close(audioDecoderCompressed);
    NEXUS_VideoWindow_Close(window);
    NEXUS_Display_Close(display);
    NEXUS_Platform_Uninit();

    return 0;
}
#else /* NEXUS_HAS_AUDIO */
int main(void)
{
    printf("This application is not supported on this platform (needs audio)!\n");
    return 0;
}
#endif
