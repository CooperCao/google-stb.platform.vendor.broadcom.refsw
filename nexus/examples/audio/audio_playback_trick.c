/******************************************************************************
 *    (c)2008-2012 Broadcom Corporation
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
 * $brcm_Workfile: playback_trick.c $
 * $brcm_Revision: 8 $
 * $brcm_Date: 1/26/12 3:00p $
 *
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: /nexus/examples/dvr/playback_trick.c $
 *
 * 8   1/26/12 3:00p rjlewis
 * SW7425-1136: Added HDMI Support.
 *
 * 7   6/24/09 4:55p erickson
 * PR51841: add STC trick mode
 *
 * 6   6/24/09 4:48p erickson
 * PR51841: fix audio, add printf's, add pause
 *
 * 5   2/19/09 2:00p erickson
 * PR51841: refactor
 *
 * 7   2/19/09 1:32p erickson
 * PR51841: refactor
 *
 * 6   2/19/09 11:14a erickson
 * PR51841: add NEXUS_PlaybackPidChannelSettings
 *
 * 5   2/4/09 12:49p erickson
 * PR51841: simplify example
 *
 *****************************************************************************/
/* Nexus example app: playback and decode */

#include "nexus_platform.h"
#include <stdio.h>

#if NEXUS_HAS_AUDIO
#include "nexus_video_decoder.h"
#include "nexus_stc_channel.h"
#include "nexus_display.h"
#include "nexus_video_window.h"
#include "nexus_video_input.h"
#include "nexus_audio_dac.h"
#include "nexus_audio_mixer.h"
#include "nexus_audio_decoder.h"
#include "nexus_audio_input.h"
#include "nexus_audio_output.h"
#include "nexus_spdif_output.h"
#include "nexus_component_output.h"
#include "nexus_composite_output.h"
#if NEXUS_HAS_HDMI_OUTPUT
#include "nexus_hdmi_output.h"
#endif
#include "nexus_playback.h"
#include "nexus_file.h"

#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include "bstd.h"
#include "bkni.h"

/**
Applications can chose to use STC trick modes or decoder trick modes for pause, slow motion forward and fast forward speeds.
STC trick modes are usually preferred because they provide precise rates and immediate resumption of audio on certain transitions.
The cost of STC trick modes is higher bandwidth for fast-forward speeds.

Set USE_STC_TRICK to 0 to see decoder trick modes.
**/
#define USE_STC_TRICK 1

/* the following define the input file and its characteristics -- these will vary by input file */
/* the following define the input and its characteristics -- these will vary by input */
#define FILE_NAME "videos/gormenghast.rec"
#define INDEX_NAME "videos/gormenghast.nav"
#define TRANSPORT_TYPE NEXUS_TransportType_eTs
#define VIDEO_CODEC NEXUS_VideoCodec_eMpeg2
#define AUDIO_CODEC NEXUS_AudioCodec_eMpeg
#define VIDEO_PID 37
#define AUDIO_PID 201
#define DESC_PID 202

#define MANUAL_CONTROL 0 /* Turn off for random testing */
#define DSP_MIXER 0 /* Turn on for testing with DSP Mixer */


int main(void)
{
    NEXUS_PlatformSettings platformSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_StcChannelHandle stcChannel;
    NEXUS_StcChannelSettings stcSettings;
    NEXUS_PidChannelHandle videoPidChannel, audioPidChannel,descPidChannel;
    NEXUS_DisplayHandle display;
    NEXUS_VideoWindowHandle window;
    NEXUS_VideoDecoderHandle videoDecoder;
    NEXUS_VideoDecoderStartSettings videoProgram;
    NEXUS_AudioDecoderHandle audioDecoder,descriptorDecoder;
    NEXUS_AudioDecoderStartSettings audioProgram, descProgram;
    NEXUS_FilePlayHandle file;
    NEXUS_PlaypumpHandle playpump;
    NEXUS_PlaybackHandle playback;
    NEXUS_PlaybackSettings playbackSettings;
    NEXUS_AudioMixerHandle mixer;
    NEXUS_AudioMixerSettings mixerSettings;
    NEXUS_PlaybackPidChannelSettings playbackPidSettings;
    NEXUS_PlaybackTrickModeSettings trick;
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_HdmiOutputStatus hdmiStatus;
    NEXUS_DisplaySettings displaySettings;
    NEXUS_Error rc;
#endif
#if MANUAL_CONTROL
    bool done = false;
#else
    int count = 0;
#endif
    NEXUS_AudioDecoderOpenSettings audioOpenSettings;
    const char *fname = FILE_NAME;
    const char *index = INDEX_NAME;

    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init(&platformSettings);
    NEXUS_Platform_GetConfiguration(&platformConfig);

    playpump = NEXUS_Playpump_Open(0, NULL);
    assert(playpump);
    playback = NEXUS_Playback_Create();
    assert(playback);

    file = NEXUS_FilePlay_OpenPosix(fname, index);
    if (!file) {
        fprintf(stderr, "can't open file:%s\n", fname);
        return -1;
    }

    NEXUS_StcChannel_GetDefaultSettings(0, &stcSettings);
    stcSettings.timebase = NEXUS_Timebase_e0;
    stcSettings.mode = NEXUS_StcChannelMode_eAuto;
    stcChannel = NEXUS_StcChannel_Open(0, &stcSettings);


    NEXUS_Playback_GetSettings(playback, &playbackSettings);
    playbackSettings.playpump = playpump;
    playbackSettings.playpumpSettings.transportType = TRANSPORT_TYPE;
    playbackSettings.stcChannel = stcChannel;
#if USE_STC_TRICK
    /* This setting is required so that NEXUS_Playback_Pause can use an STC pause. */
    playbackSettings.stcTrick = true;
#endif
    NEXUS_Playback_SetSettings(playback, &playbackSettings);

    /* Bring up audio decoders and outputs */
    audioDecoder = NEXUS_AudioDecoder_Open(0, NULL);

    NEXUS_AudioDecoder_GetDefaultOpenSettings(&audioOpenSettings);
    audioOpenSettings.type = NEXUS_AudioDecoderType_eAudioDescriptor;
    descriptorDecoder= NEXUS_AudioDecoder_Open(1, &audioOpenSettings);

    NEXUS_AudioMixer_GetDefaultSettings(&mixerSettings);
#if NEXUS_AUDIO_MODULE_FAMILY == NEXUS_AUDIO_MODULE_FAMILY_APE_RAAGA
#if DSP_MIXER
    mixerSettings.mixUsingDsp = true;
#endif
#endif
    mixer = NEXUS_AudioMixer_Open(&mixerSettings);

#if NEXUS_NUM_AUDIO_DACS
    NEXUS_AudioOutput_AddInput(
        NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]),
        NEXUS_AudioMixer_GetConnector(mixer));
#endif
#if NEXUS_NUM_SPDIF_OUTPUTS
    NEXUS_AudioOutput_AddInput(
        NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]),
        NEXUS_AudioMixer_GetConnector(mixer));
#endif
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_AudioOutput_AddInput(
        NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]),
        NEXUS_AudioMixer_GetConnector(mixer));
#endif

    /* Add both decoders to the mixer */
    NEXUS_AudioMixer_AddInput(mixer, NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
    NEXUS_AudioMixer_AddInput(mixer, NEXUS_AudioDecoder_GetConnector(descriptorDecoder, NEXUS_AudioDecoderConnectorType_eStereo));

#if NEXUS_AUDIO_MODULE_FAMILY == NEXUS_AUDIO_MODULE_FAMILY_APE_RAAGA
#if DSP_MIXER
    NEXUS_AudioMixer_GetSettings(mixer, &mixerSettings);
    mixerSettings.master = NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo);
    NEXUS_AudioMixer_SetSettings(mixer, &mixerSettings);
#endif
#endif

    /* Bring up video display and outputs */
    display = NEXUS_Display_Open(0, NULL);
    window = NEXUS_VideoWindow_Open(display, 0);

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

    /* Open the audio and video pid channels */
    NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
    playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eVideo;
    playbackPidSettings.pidTypeSettings.video.codec = VIDEO_CODEC; /* must be told codec for correct handling */
    playbackPidSettings.pidTypeSettings.video.index = true;
    playbackPidSettings.pidTypeSettings.video.decoder = videoDecoder;
    videoPidChannel = NEXUS_Playback_OpenPidChannel(playback, VIDEO_PID, &playbackPidSettings);

    NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
    playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eAudio;
    playbackPidSettings.pidTypeSettings.audio.primary = audioDecoder;
    playbackPidSettings.pidTypeSettings.audio.secondary = descriptorDecoder;
    audioPidChannel = NEXUS_Playback_OpenPidChannel(playback, AUDIO_PID, &playbackPidSettings);

    NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
    playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eAudio;
    playbackPidSettings.pidTypeSettings.audio.primary = audioDecoder;
    playbackPidSettings.pidTypeSettings.audio.secondary = descriptorDecoder;
    descPidChannel = NEXUS_Playback_OpenPidChannel(playback, DESC_PID, &playbackPidSettings);

    /* Set up decoder Start structures now. We need to know the audio codec to properly set up
    the audio outputs. */
    NEXUS_VideoDecoder_GetDefaultStartSettings(&videoProgram);
    videoProgram.codec = VIDEO_CODEC;
    videoProgram.pidChannel = videoPidChannel;
    videoProgram.stcChannel = stcChannel;


    NEXUS_AudioDecoder_GetDefaultStartSettings(&audioProgram);
    audioProgram.codec = AUDIO_CODEC;
    audioProgram.pidChannel = audioPidChannel;
    audioProgram.stcChannel = stcChannel;

    NEXUS_AudioDecoder_GetDefaultStartSettings(&descProgram);
    descProgram.codec = AUDIO_CODEC;
    descProgram.pidChannel = descPidChannel;
    descProgram.stcChannel = stcChannel;

    printf("normal play\n");
    NEXUS_VideoDecoder_Start(videoDecoder, &videoProgram);
    NEXUS_AudioDecoder_Start(audioDecoder, &audioProgram);
    NEXUS_AudioDecoder_Start(descriptorDecoder, &descProgram);
    NEXUS_Playback_Start(playback, file, NULL);
#if MANUAL_CONTROL
    while (!done)
    {
        int tmp;
        bool audio = false;
        printf("Menu\n");
        printf(" 0 - Exit\n");
        printf(" 1 - Play\n");
        printf(" 2 - STC Pause\n");
        printf(" 3 - Pause (rate 0x)\n");
        printf(" 4 - DSOLA FF (1.2x)\n");
        printf(" 5 - Slow Forward (0.8x)\n");
        printf(" 6 - FF (2x)\n");
        printf(" 7 - FF (8x)\n");
        printf(" 8 - RW (2x)\n");
        scanf("%d", &tmp);
        switch ( tmp )
        {
            case 0:
                done = true;
                break;
            case 1:
                printf("normal play\n");
                NEXUS_Playback_Play(playback);
                bool audio = true;
                break;
            case 2:
                printf("stc pause\n");
                NEXUS_Playback_Pause(playback);
                break;
            case 3:
                printf("0x pause\n");
                NEXUS_Playback_GetDefaultTrickModeSettings(&trick);
                trick.rate = 0;
                NEXUS_Playback_TrickMode(playback, &trick);
                break;
            case 4:
                printf("1.2x\n");
                #if !DSP_MIXER
                audio = true;
                #endif
                NEXUS_Playback_GetDefaultTrickModeSettings(&trick);
                trick.rate = 1.2*NEXUS_NORMAL_PLAY_SPEED;
                NEXUS_Playback_TrickMode(playback, &trick);
                break;
            case 5:
                printf("0.8x\n");
                #if !DSP_MIXER
                audio = true;
                #endif
                NEXUS_Playback_GetDefaultTrickModeSettings(&trick);
                trick.rate = 0.8*NEXUS_NORMAL_PLAY_SPEED;
                NEXUS_Playback_TrickMode(playback, &trick);
                break;
            case 6:
                printf("2x\n");
                NEXUS_Playback_GetDefaultTrickModeSettings(&trick);
                trick.rate = 2*NEXUS_NORMAL_PLAY_SPEED;
                NEXUS_Playback_TrickMode(playback, &trick);
                break;
            case 7:
                printf("8x\n");
                NEXUS_Playback_GetDefaultTrickModeSettings(&trick);
                trick.rate = 8*NEXUS_NORMAL_PLAY_SPEED;
                NEXUS_Playback_TrickMode(playback, &trick);
                break;
            case 8:
                printf("-2x\n");
                NEXUS_Playback_GetDefaultTrickModeSettings(&trick);
                trick.rate = -2*NEXUS_NORMAL_PLAY_SPEED;
                NEXUS_Playback_TrickMode(playback, &trick);
                break;
            default:
                printf("Invalid Value\n");
                break;
          }
#else
    while (count < 50)
    {
        int tmp;
        bool audio = false;

        tmp = (int)(rand() % 6);
        switch ( tmp )
        {
            case 0:
                printf("normal play rate 1000\n");
                NEXUS_Playback_Play(playback);
                audio = true;
                break;
            case 1:
                printf("stc pause\n");
                NEXUS_Playback_Pause(playback);
                break;
            case 2:
                printf("rate 0 pause\n");
                NEXUS_Playback_GetDefaultTrickModeSettings(&trick);
                trick.maxDecoderRate = 2*NEXUS_NORMAL_PLAY_SPEED;
                trick.rate = 0;
                NEXUS_Playback_TrickMode(playback, &trick);
            case 3: /* rewind */
                NEXUS_Playback_GetDefaultTrickModeSettings(&trick);
                trick.maxDecoderRate = 2*NEXUS_NORMAL_PLAY_SPEED;
                trick.rate = (int)-(rand() % NEXUS_NORMAL_PLAY_SPEED*10);
                printf("Rewind rate %d\n",trick.rate);
                NEXUS_Playback_TrickMode(playback, &trick);
                break;
            case 4: /* stc trick */
                #if !DSP_MIXER
                audio = true;
                #endif
                NEXUS_Playback_GetDefaultTrickModeSettings(&trick);
                trick.maxDecoderRate = 2*NEXUS_NORMAL_PLAY_SPEED;
                trick.rate = (int)(rand() % NEXUS_NORMAL_PLAY_SPEED*2);
                printf("STC Trick rate %d\n",trick.rate);
                NEXUS_Playback_TrickMode(playback, &trick);
                break;
            case 5: /* non stc trick */
                NEXUS_Playback_GetDefaultTrickModeSettings(&trick);
                trick.maxDecoderRate = 2*NEXUS_NORMAL_PLAY_SPEED;
                trick.rate = (int)(rand() % NEXUS_NORMAL_PLAY_SPEED*10)+NEXUS_NORMAL_PLAY_SPEED*2;
                printf("Non-STC Trick rate %d\n",trick.rate);
                NEXUS_Playback_TrickMode(playback, &trick);
                break;
            default:
                printf("Invalid Value\n");
                break;
        }
        count++;
#endif
        if (audio)
        {
            printf("\nAudio Should be Present Here...\n\n");
        }
        else
        {
            printf("\nAudio is Muted or Stopped Here...\n\n");
        }

#if !MANUAL_CONTROL
        BKNI_Sleep(5000);
#endif
    }

    NEXUS_Playback_Stop(playback);
    NEXUS_VideoDecoder_Stop(videoDecoder);
    NEXUS_AudioDecoder_Stop(audioDecoder);
    NEXUS_AudioDecoder_Stop(descriptorDecoder);
    NEXUS_AudioInput_Shutdown(NEXUS_AudioMixer_GetConnector(mixer));
    NEXUS_AudioMixer_Close(mixer);
    NEXUS_Playback_CloseAllPidChannels(playback);
    NEXUS_FilePlay_Close(file);
    NEXUS_Playback_Destroy(playback);
    NEXUS_Playpump_Close(playpump);
    NEXUS_VideoDecoder_Close(videoDecoder);
    NEXUS_VideoWindow_Close(window);
    NEXUS_AudioDecoder_Close(audioDecoder);
    NEXUS_AudioDecoder_Close(descriptorDecoder);
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
