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
 *****************************************************************************/
/* Nexus example app: basic file playback */

#include "nexus_platform.h"
#include <stdio.h>

#if NEXUS_HAS_AUDIO
#include "nexus_video_decoder.h"
#include "nexus_stc_channel.h"
#include "nexus_display.h"
#include "nexus_video_window.h"
#include "nexus_video_input.h"
#include "nexus_audio_dac.h"
#include "nexus_audio_decoder.h"
#include "nexus_audio_input.h"
#include "nexus_audio_output.h"
#include "nexus_audio_mixer.h"
#include "nexus_spdif_output.h"
#include "nexus_component_output.h"
#include "nexus_composite_output.h"
#if NEXUS_HAS_HDMI_OUTPUT
#include "nexus_hdmi_output.h"
#endif
#if NEXUS_HAS_PLAYBACK
#include "nexus_playback.h"
#include "nexus_file.h"
#endif
#if NEXUS_HAS_SYNC_CHANNEL
#include "nexus_sync_channel.h"
#endif

#include <assert.h>
#include "bstd.h"
#include "bkni.h"

#define NUM_AUDIO_DECODES                   4

/* the following define the input file and its characteristics -- these will vary by input file */
static const char *fname = "/mnt/streams/streamer/bugs_toys2_jurassic_q64_cd.mpg";
#define TRANSPORT_TYPE NEXUS_TransportType_eTs
#define VIDEO_CODEC NEXUS_VideoCodec_eMpeg2
#define VIDEO_PID 0x11
#define AUDIO_CODEC NEXUS_AudioCodec_eAc3
static unsigned audio_pids[NUM_AUDIO_DECODES] = {0x14, 0x24, 0x44, 0x14};

#define ENABLE_AUDIO_LOW_DELAY_DECODE       0

BDBG_MODULE(audio_playpump);

int main(void)
{
    unsigned i;
    NEXUS_PlatformSettings platformSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_StcChannelHandle stcChannel;
    NEXUS_StcChannelSettings stcSettings;
    NEXUS_PidChannelHandle videoPidChannel, audioPidChannel[NUM_AUDIO_DECODES];
    NEXUS_DisplayHandle display;
    NEXUS_VideoWindowHandle window;
    NEXUS_VideoDecoderHandle videoDecoder;
    NEXUS_VideoDecoderStartSettings videoProgram;
    NEXUS_AudioDecoderHandle pcmDecoder[NUM_AUDIO_DECODES];
    NEXUS_AudioMixerHandle hwMixer, dspMixer;
    NEXUS_AudioMixerSettings mixerSettings;
    NEXUS_AudioCapabilities audioCapabilities;
    #if ENABLE_AUDIO_LOW_DELAY_DECODE
    NEXUS_SyncChannelSettings syncChannelSettings;
    NEXUS_SyncChannelHandle syncChannel;
    #endif
    NEXUS_AudioDecoderStartSettings audioProgram[NUM_AUDIO_DECODES];
    NEXUS_FilePlayHandle file;
    NEXUS_PlaypumpHandle playpump;
    NEXUS_PlaybackHandle playback;
    NEXUS_PlaybackSettings playbackSettings;
    NEXUS_PlaybackPidChannelSettings playbackPidSettings;
    #if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_HdmiOutputStatus hdmiStatus;
    NEXUS_DisplaySettings displaySettings;
    NEXUS_Error rc;
    #endif
    NEXUS_AudioOutputHandle audioDacHandle = NULL;
    NEXUS_AudioOutputHandle audioSpdifHandle = NULL;
    NEXUS_AudioOutputHandle audioHdmiHandle = NULL;


    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    platformSettings.audioModuleSettings.numPcmBuffers += 2;
    NEXUS_Platform_Init(&platformSettings);
    NEXUS_Platform_GetConfiguration(&platformConfig);
    NEXUS_GetAudioCapabilities(&audioCapabilities);

    if (audioCapabilities.numDecoders < NUM_AUDIO_DECODES ||
        audioCapabilities.numMixers == 0 ||
        !audioCapabilities.dsp.mixer)
    {
        printf("This application is not supported on this platform (requires decoder and mixers).\n");
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

#if ENABLE_AUDIO_LOW_DELAY_DECODE
    /* create a sync channel */
    NEXUS_SyncChannel_GetDefaultSettings(&syncChannelSettings);
    syncChannel = NEXUS_SyncChannel_Create(&syncChannelSettings);
#endif

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
    /* set a stream format, it could be any audio video transport type or file format, i.e NEXUS_TransportType_eMp4, NEXUS_TransportType_eAvi ... */
    playbackSettings.playpumpSettings.transportType = TRANSPORT_TYPE;
    playbackSettings.stcChannel = stcChannel;
    NEXUS_Playback_SetSettings(playback, &playbackSettings);

    NEXUS_AudioMixer_GetDefaultSettings(&mixerSettings);
    mixerSettings.intermediate = true;
    mixerSettings.fixedOutputFormat = NEXUS_AudioMultichannelFormat_eStereo;
    mixerSettings.fixedOutputFormatEnabled = true;
    hwMixer = NEXUS_AudioMixer_Open(&mixerSettings);
    NEXUS_AudioMixer_GetDefaultSettings(&mixerSettings);
    mixerSettings.mixUsingDsp = true;
    dspMixer = NEXUS_AudioMixer_Open(&mixerSettings);

    for ( i=0; i<NUM_AUDIO_DECODES; i++)
    {
        pcmDecoder[i] = NEXUS_AudioDecoder_Open(i, NULL);
        NEXUS_AudioMixer_AddInput(hwMixer, NEXUS_AudioDecoder_GetConnector(pcmDecoder[i], NEXUS_AudioDecoderConnectorType_eStereo));
    }

    NEXUS_AudioMixer_AddInput(dspMixer, NEXUS_AudioMixer_GetConnector(hwMixer));

    #if 0
    NEXUS_AudioMixer_GetSettings(dspMixer, &mixerSettings);
    mixerSettings.master = NEXUS_AudioDecoder_GetConnector(pcmDecoder, NEXUS_AudioDecoderConnectorType_eStereo);
    NEXUS_AudioMixer_SetSettings(dspMixer, &mixerSettings);
    #endif

    if (audioDacHandle) {
        NEXUS_AudioOutput_AddInput(
            audioDacHandle,
            NEXUS_AudioMixer_GetConnector(dspMixer));
    }
    if (audioSpdifHandle) {
        NEXUS_AudioOutput_AddInput(
            audioSpdifHandle,
            NEXUS_AudioMixer_GetConnector(dspMixer));
    }
    #if NEXUS_NUM_HDMI_OUTPUTS
    if (audioHdmiHandle) {
        NEXUS_AudioOutput_AddInput(
            audioHdmiHandle,
            NEXUS_AudioMixer_GetConnector(dspMixer));
    }
    #endif

    #if 0
    printf("Press ENTER to Start Intermediate DSP Mixer\n");
    while (getchar() != '\n'){ }

    /* start the intermediate hw mixer explicitly */
    NEXUS_AudioMixer_Start(dspMixer);
    #endif

    printf("Press ENTER to Start Intermediate HW Mixer\n");
    while (getchar() != '\n'){ }

    /* start the intermediate hw mixer explicitly */
    NEXUS_AudioMixer_Start(hwMixer);

    /* Bring up video display and outputs */
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

    /* bring up decoder and connect to display */
    videoDecoder = NEXUS_VideoDecoder_Open(0, NULL); /* take default capabilities */
    NEXUS_VideoWindow_AddInput(window, NEXUS_VideoDecoder_GetConnector(videoDecoder));
    NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);

    file = NEXUS_FilePlay_OpenPosix(fname, NULL);
    if (!file) {
        fprintf(stderr, "can't open file:%s\n", fname);
        return -1;
    }

    /* Open the audio and video pid channels */
    playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eVideo;
    playbackPidSettings.pidTypeSettings.video.codec = VIDEO_CODEC; /* must be told codec for correct handling */
    playbackPidSettings.pidTypeSettings.video.index = true;
    playbackPidSettings.pidTypeSettings.video.decoder = videoDecoder;
    videoPidChannel = NEXUS_Playback_OpenPidChannel(playback, VIDEO_PID, &playbackPidSettings);

    /* Set up decoder Start structures now. We need to know the audio codec to properly set up the audio outputs. */
    NEXUS_VideoDecoder_GetDefaultStartSettings(&videoProgram);
    videoProgram.codec = VIDEO_CODEC;
    videoProgram.pidChannel = videoPidChannel;
    videoProgram.stcChannel = stcChannel;

    /* Open the audio and video pid channels */
    NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
    playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eAudio;
    for ( i=0; i<NUM_AUDIO_DECODES; i++)
    {
        playbackPidSettings.pidTypeSettings.audio.primary = pcmDecoder[i];
        audioPidChannel[i] = NEXUS_Playback_OpenPidChannel(playback, audio_pids[i], &playbackPidSettings);

        /* Set up decoder Start structures now. We need to know the audio codec to properly set up the audio outputs. */
        NEXUS_AudioDecoder_GetDefaultStartSettings(&audioProgram[i]);
        audioProgram[i].codec = AUDIO_CODEC;
        audioProgram[i].pidChannel = audioPidChannel[i];
        audioProgram[i].stcChannel = stcChannel;
        #if ENABLE_AUDIO_LOW_DELAY_DECODE
        #if 1
        audioProgram[i].latencyMode = NEXUS_AudioDecoderLatencyMode_eLow;
        #else
        audioProgram[i].latencyMode = NEXUS_AudioDecoderLatencyMode_eLowest;
        #endif
        #endif
    }


#if ENABLE_AUDIO_LOW_DELAY_DECODE
    /* connect sync channel */
    NEXUS_SyncChannel_GetSettings(syncChannel, &syncChannelSettings);
    syncChannelSettings.videoInput = NEXUS_VideoDecoder_GetConnector(videoDecoder);
    syncChannelSettings.audioInput[0] = NEXUS_AudioDecoder_GetConnector(pcmDecoder, NEXUS_AudioDecoderConnectorType_eStereo);
    NEXUS_SyncChannel_SetSettings(syncChannel, &syncChannelSettings);
#endif


    printf("Press ENTER to Start Playback\n");
    while (getchar() != '\n'){ }

    /* Start playback */
    NEXUS_Playback_Start(playback, file, NULL);

    for ( i=0; i<NUM_AUDIO_DECODES; i++)
    {
        /* Start decoders */
        NEXUS_AudioDecoder_Start(pcmDecoder[i], &audioProgram[i]);
    }

    /* Start decoders */
    NEXUS_VideoDecoder_Start(videoDecoder, &videoProgram);

    for (;;)
    {
        BKNI_Sleep(2000);
    }

    /* Bring down system */
    NEXUS_Playback_Stop(playback);
    NEXUS_VideoDecoder_Stop(videoDecoder);
    for ( i=0; i<NUM_AUDIO_DECODES; i++)
    {
        NEXUS_AudioDecoder_Stop(pcmDecoder[i]);
    }
    NEXUS_AudioMixer_Stop(dspMixer);
    NEXUS_Playback_Destroy(playback);
    if (audioDacHandle) {
        NEXUS_AudioOutput_RemoveAllInputs(audioDacHandle);
    }
    #if NEXUS_NUM_HDMI_OUTPUTS
    if (audioHdmiHandle) {
        NEXUS_AudioOutput_RemoveAllInputs(audioHdmiHandle);
    }
    #endif
    if (audioSpdifHandle) {
        NEXUS_AudioOutput_RemoveAllInputs(audioSpdifHandle);
    }
    for ( i=0; i<NUM_AUDIO_DECODES; i++)
    {
        NEXUS_AudioDecoder_Close(pcmDecoder[i]);
    }
    NEXUS_VideoDecoder_Close(videoDecoder);
    NEXUS_FilePlay_Close(file);
    NEXUS_Playpump_Close(playpump);
    NEXUS_VideoWindow_Close(window);
    NEXUS_Display_Close(display);
    NEXUS_StcChannel_Close(stcChannel);
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
