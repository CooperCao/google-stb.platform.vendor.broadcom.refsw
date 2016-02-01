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
/* Nexus example app: basic file playback */

#include "nexus_platform.h"
#include <stdio.h>

#if NEXUS_HAS_AUDIO && (NEXUS_AUDIO_MODULE_FAMILY == NEXUS_AUDIO_MODULE_FAMILY_APE_RAAGA)
#include "nexus_video_decoder.h"
#include "nexus_stc_channel.h"
#include "nexus_display.h"
#include "nexus_video_window.h"
#include "nexus_video_input.h"
#include "nexus_audio_dac.h"
#include "nexus_audio_decoder.h"
#include "nexus_audio_input.h"
#include "nexus_audio_mixer.h"
#include "nexus_audio_output.h"
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
#if 1 /* AC3 Case */
static const char *fname = "/mnt/streams/streamer/bugs_toys2_jurassic_q64_cd.mpg";
#define TRANSPORT_TYPE NEXUS_TransportType_eTs
#define VIDEO_CODEC NEXUS_VideoCodec_eMpeg2
#define VIDEO_PID 0x11
#define AUDIO_CODEC NEXUS_AudioCodec_eAc3
static unsigned audio_pids[NUM_AUDIO_DECODES] = {0x14, 0x24, 0x44, 0x14};
#else
static const char *fname = "/mnt/streams/streamer/bugs_toys2_jurassic_q64_cd.mpg";
#define TRANSPORT_TYPE NEXUS_TransportType_eTs
#define VIDEO_CODEC NEXUS_VideoCodec_eMpeg2
#define VIDEO_PID 0x11
#define AUDIO_CODEC NEXUS_AudioCodec_eAc3Plus
static unsigned audio_pids[NUM_AUDIO_DECODES] = {0x14, 0x24, 0x44, 0x14};
#endif

BDBG_MODULE(audio_playpump);

int main(void)
{
    NEXUS_PlatformSettings platformSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_StcChannelHandle stcChannel;
    NEXUS_StcChannelSettings stcSettings;
    NEXUS_PidChannelHandle videoPidChannel, audioPidChannel[NUM_AUDIO_DECODES];
    NEXUS_DisplayHandle display;
    NEXUS_VideoWindowHandle window;
    NEXUS_VideoDecoderHandle videoDecoder;
    NEXUS_VideoDecoderStartSettings videoProgram;
    NEXUS_AudioDecoderHandle audioDecoder[NUM_AUDIO_DECODES];
    NEXUS_AudioDecoderHandle compressedDecoder[NUM_AUDIO_DECODES];
    NEXUS_AudioDecoderHandle * compDec = NULL;
    NEXUS_AudioDecoderStartSettings audioProgram[NUM_AUDIO_DECODES];
    NEXUS_AudioMixerHandle hdmiMixer;
    NEXUS_AudioMixerSettings mixerSettings;
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
    unsigned i;

    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    platformSettings.audioModuleSettings.allowI2sCompressed = true;
    platformSettings.audioModuleSettings.allowSpdif4xCompressed = true;
    platformSettings.audioModuleSettings.numCompressedBuffers = 0;
    platformSettings.audioModuleSettings.numCompressed4xBuffers = 4;
    NEXUS_Platform_Init(&platformSettings);
    NEXUS_Platform_GetConfiguration(&platformConfig);

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

    /* Bring up audio decoders and outputs */
    for (i = 0; i < NUM_AUDIO_DECODES; i++)
    {
        NEXUS_AudioDecoderOpenSettings audDecOpenSettings;
        NEXUS_AudioDecoder_GetDefaultOpenSettings(&audDecOpenSettings);
        audDecOpenSettings.type = NEXUS_AudioDecoderType_eDecode;
        audioDecoder[i] = NEXUS_AudioDecoder_Open(i, &audDecOpenSettings);
        assert(audioDecoder[i]);
        audDecOpenSettings.type = NEXUS_AudioDecoderType_ePassthrough;
        compressedDecoder[i] = NEXUS_AudioDecoder_Open(4+i, &audDecOpenSettings);
        assert(compressedDecoder[i]);
    }

    NEXUS_AudioMixer_GetDefaultSettings(&mixerSettings);
    /* explicitly set the output format of the HDMI mixer to upmix to 7.1chs */
    mixerSettings.fixedOutputFormatEnabled = true;
    mixerSettings.fixedOutputFormat = NEXUS_AudioMultichannelFormat_e7_1;
    hdmiMixer = NEXUS_AudioMixer_Open(&mixerSettings);
    assert(hdmiMixer);

    if (AUDIO_CODEC == NEXUS_AudioCodec_eAc3Plus)
    {
        compDec = compressedDecoder;
    }
    else
    {
        compDec = audioDecoder;
    }

    /* 4 x Compressed outputs */
    #if NEXUS_NUM_SPDIF_OUTPUTS
    NEXUS_AudioOutput_AddInput(
        NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]),
        NEXUS_AudioDecoder_GetConnector(compDec[0], NEXUS_AudioDecoderConnectorType_eCompressed));
    #endif
    #if NEXUS_NUM_SPDIF_OUTPUTS > 1
    NEXUS_AudioOutput_AddInput(
        NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[1]),
        NEXUS_AudioDecoder_GetConnector(compDec[1], NEXUS_AudioDecoderConnectorType_eCompressed));
    #endif
    #if NEXUS_NUM_I2S_OUTPUTS
    NEXUS_AudioOutput_AddInput(
        NEXUS_I2sOutput_GetConnector(platformConfig.outputs.i2s[0]),
        NEXUS_AudioDecoder_GetConnector(compDec[2], NEXUS_AudioDecoderConnectorType_eCompressed));
    #endif
    #if NEXUS_NUM_I2S_OUTPUTS > 1
    NEXUS_AudioOutput_AddInput(
        NEXUS_I2sOutput_GetConnector(platformConfig.outputs.i2s[1]),
        NEXUS_AudioDecoder_GetConnector(compDec[3], NEXUS_AudioDecoderConnectorType_eCompressed));
    #endif

    /* 4 x PCM outputs -> FMM Mixer -> HDMI */
    for (i = 0; i < NUM_AUDIO_DECODES; i++)
    {
        NEXUS_AudioDecoderSettings decoderSettings;

        BDBG_ERR(( "Adding decoder %p to hdmiMixer\n", (void*)audioDecoder[i] ));
        NEXUS_AudioMixer_AddInput(hdmiMixer, NEXUS_AudioDecoder_GetConnector(audioDecoder[i], NEXUS_AudioDecoderConnectorType_eStereo));

        BDBG_ERR(( "Setting volume coeffs for hdmiMixer input %d to be output on MAI pair %d\n", i, i ));
        NEXUS_AudioDecoder_GetSettings(audioDecoder[i], &decoderSettings);
        BKNI_Memset(&decoderSettings.volumeMatrix, 0, sizeof(decoderSettings.volumeMatrix));
        decoderSettings.volumeMatrix[2*i][0] = NEXUS_AUDIO_VOLUME_LINEAR_NORMAL;
        decoderSettings.volumeMatrix[2*i+1][1] = NEXUS_AUDIO_VOLUME_LINEAR_NORMAL;
        /*decoderSettings.muted = true;*/
        NEXUS_AudioDecoder_SetSettings(audioDecoder[i], &decoderSettings);
        #if 0
        {
            unsigned j;

            BDBG_ERR(("HDMI mixer coefficient table:"));
            BDBG_ERR(("  muted=%d", decoderSettings.muted));
            for ( j = 0; j < NEXUS_AudioChannel_eMax; j++ )
            {
                BDBG_ERR(("%6x %6x %6x %6x %6x %6x %6x %6x\n",
                       decoderSettings.volumeMatrix[j][0],
                       decoderSettings.volumeMatrix[j][1],
                       decoderSettings.volumeMatrix[j][2],
                       decoderSettings.volumeMatrix[j][3],
                       decoderSettings.volumeMatrix[j][4],
                       decoderSettings.volumeMatrix[j][5],
                       decoderSettings.volumeMatrix[j][6],
                       decoderSettings.volumeMatrix[j][7]
                       ));
            }
        }
        #endif
    }

    #if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_AudioOutput_AddInput(
        NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]),
        NEXUS_AudioMixer_GetConnector(hdmiMixer));
    #endif

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

	printf("Press ENTER to Start Playback\n");
	while (getchar() != '\n'){ }

    /* Start decoders */
    NEXUS_VideoDecoder_Start(videoDecoder, &videoProgram);

    /* Start decoders */
    /* Set up decoder Start structures now. We need to know the audio codec to properly set up the audio outputs. */
    for (i = 0; i < NUM_AUDIO_DECODES; i++)
    {
        NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
        playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eAudio;
        playbackPidSettings.pidTypeSettings.audio.primary = audioDecoder[i];
        audioPidChannel[i] = NEXUS_Playback_OpenPidChannel(playback, audio_pids[i], &playbackPidSettings);

        NEXUS_AudioDecoder_GetDefaultStartSettings(&audioProgram[i]);
        audioProgram[i].codec = AUDIO_CODEC;
        audioProgram[i].pidChannel = audioPidChannel[i];
        audioProgram[i].stcChannel = stcChannel;
    }

    for (i = 0; i < NUM_AUDIO_DECODES; i++)
    {
        NEXUS_AudioDecoder_Start(audioDecoder[i], &audioProgram[i]);
        if (AUDIO_CODEC == NEXUS_AudioCodec_eAc3Plus)
        {
            NEXUS_AudioDecoder_Start(compressedDecoder[i], &audioProgram[i]);
        }
    }

	/* Start playback */
    NEXUS_Playback_Start(playback, file, NULL);

    for (;;)
    {
        BKNI_Sleep(2000);
    }

    /* Bring down system */
	NEXUS_Playback_Stop(playback);
	NEXUS_VideoDecoder_Stop(videoDecoder);
    for (i = 0; i < NUM_AUDIO_DECODES; i++)
    {
        if (AUDIO_CODEC == NEXUS_AudioCodec_eAc3Plus)
        {
            NEXUS_AudioDecoder_Stop(compressedDecoder[i]);
        }
        NEXUS_AudioDecoder_Stop(audioDecoder[i]);
    }
    NEXUS_Playback_Destroy(playback);
    #if NEXUS_NUM_AUDIO_DACS
    NEXUS_AudioOutput_RemoveAllInputs(NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]));
    #endif
    #if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_AudioOutput_RemoveAllInputs(NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]));
    #endif
    #if NEXUS_NUM_SPDIF_OUTPUTS
    NEXUS_AudioOutput_RemoveAllInputs(NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]));
    #endif
    for (i = 0; i < NUM_AUDIO_DECODES; i++)
    {
        NEXUS_AudioDecoder_Close(compressedDecoder[i]);
        NEXUS_AudioDecoder_Close(audioDecoder[i]);
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
