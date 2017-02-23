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
#include "nexus_audio_dac.h"
#include "nexus_spdif_output.h"
#include "nexus_audio_decoder.h"
#include "nexus_audio_output.h"
#include "nexus_audio_input.h"
#include "nexus_composite_output.h"
#include "nexus_component_output.h"
#if NEXUS_HAS_HDMI_OUTPUT
#include "nexus_hdmi_output.h"
#endif
#if NEXUS_HAS_PLAYBACK
#include "nexus_playback.h"
#include "nexus_file.h"
#endif

#include <assert.h>
#include "bstd.h"
#include "bkni.h"

/* the following define the input file and its characteristics -- these will vary by input file */
#define FILE_NAME "videos/avatar_AVC_15M.ts"
#define TRANSPORT_TYPE NEXUS_TransportType_eTs
#define VIDEO_CODEC NEXUS_VideoCodec_eH264
#define AUDIO_CODEC NEXUS_AudioCodec_eAc3
#define VIDEO_PID 257
#define AUDIO_PID 260


BDBG_MODULE(audio_decoder_suspend) ;
/* switch digital outputs pcm <-> compressed by using suspend/resume */
int main (int argc, const char *argv[])
{
#if NEXUS_HAS_PLAYBACK
    NEXUS_PlatformSettings platformSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_StcChannelHandle stcChannel;
    NEXUS_StcChannelSettings stcSettings;
    NEXUS_PidChannelHandle videoPidChannel, audioPidChannel;
    NEXUS_PlaybackPidChannelSettings pidSettings;
    NEXUS_DisplayHandle display;
    NEXUS_VideoWindowHandle window;
    NEXUS_VideoDecoderHandle videoDecoder;
    NEXUS_VideoDecoderStartSettings videoProgram;
    NEXUS_AudioDecoderHandle pcmDecoder, compressedDecoder=NULL;
    NEXUS_AudioDecoderStartSettings audioProgram;
    NEXUS_FilePlayHandle file;
    NEXUS_PlaypumpHandle playpump;
    NEXUS_PlaybackHandle playback;
    NEXUS_PlaybackSettings playbackSettings;
    NEXUS_AudioOutputSettings outputSettings;
    NEXUS_SpdifOutputSettings spdifSettings;
    bool pcm = true;
    bool compressedSupported = false;
    bool mute = false;
    bool done = false;
    bool stopped = false;
    bool suspended = false;
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_HdmiOutputSettings hdmiSettings;
    NEXUS_HdmiOutputStatus hdmiStatus;
    NEXUS_DisplaySettings displaySettings;
    NEXUS_Error rc;
#endif
    NEXUS_AudioDecoderOpenSettings audioDecoderOpenSettings;
    NEXUS_AudioCapabilities audioCapabilities;
    NEXUS_AudioOutputHandle audioDummyHandle = NULL;
    NEXUS_AudioOutputHandle audioSpdifHandle = NULL;
    NEXUS_AudioOutputHandle audioHdmiHandle = NULL;


    const char *fname = FILE_NAME;

    if (argc > 1) {
        fname = argv[1];
    }

    /* Bring up all modules for a platform in a default configuration for this platform */
    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init(&platformSettings);
    NEXUS_Platform_GetConfiguration(&platformConfig);
    NEXUS_GetAudioCapabilities(&audioCapabilities);

    if (audioCapabilities.numDecoders < 2)
    {
        printf("This application is not supported on this platform (requires decoders).\n");
        return 0;
    }

    if (audioCapabilities.numOutputs.dummy > 0)
    {
        audioDummyHandle = NEXUS_AudioDummyOutput_GetConnector(platformConfig.outputs.audioDummy[0]);
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


    playpump = NEXUS_Playpump_Open(0, NULL);
    assert(playpump);
    playback = NEXUS_Playback_Create();
    assert(playback);

    /* The MKV filenames must be specified twice, both as the data file and the index file. */
    file = NEXUS_FilePlay_OpenPosix(fname, fname);
    if (!file) {
        fprintf(stderr, "can't open file:%s\n", fname);
        return -1;
    }

    /* Bring up audio decoders and outputs */
    NEXUS_AudioDecoder_GetDefaultOpenSettings(&audioDecoderOpenSettings);
    audioDecoderOpenSettings.multichannelFormat = NEXUS_AudioMultichannelFormat_e5_1;
    pcmDecoder = NEXUS_AudioDecoder_Open(0, &audioDecoderOpenSettings);

    /* Always bring up the compressed decoder and attach a dummy output. */
    audioDecoderOpenSettings.multichannelFormat = NEXUS_AudioMultichannelFormat_eNone;
    audioDecoderOpenSettings.type = NEXUS_AudioDecoderType_ePassthrough;
    compressedDecoder = NEXUS_AudioDecoder_Open(1, &audioDecoderOpenSettings);

#if NEXUS_NUM_HDMI_OUTPUTS
    if (audioHdmiHandle) {
        NEXUS_AudioOutput_AddInput(
            audioHdmiHandle,
            NEXUS_AudioDecoder_GetConnector(pcmDecoder, NEXUS_AudioDecoderConnectorType_eMultichannel));

        NEXUS_AudioOutput_GetSettings(NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]),
                                      &outputSettings);
        outputSettings.nco = NEXUS_AudioOutputNco_eMax;
        outputSettings.pll = NEXUS_AudioOutputPll_e0;
        NEXUS_AudioOutput_SetSettings(NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]),
                                      &outputSettings);
    }
#endif
    if (audioSpdifHandle) {
        NEXUS_AudioOutput_AddInput(
            audioSpdifHandle,
            NEXUS_AudioDecoder_GetConnector(pcmDecoder, NEXUS_AudioConnectorType_eStereo));

        NEXUS_AudioOutput_GetSettings(NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]),
                                      &outputSettings);
        outputSettings.nco = NEXUS_AudioOutputNco_eMax;
        outputSettings.pll = NEXUS_AudioOutputPll_e0;
        NEXUS_AudioOutput_SetSettings(NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]),
                                      &outputSettings);
    }
    if (audioDummyHandle) {
        NEXUS_AudioOutput_AddInput(
            audioDummyHandle,
            NEXUS_AudioDecoder_GetConnector(compressedDecoder, NEXUS_AudioConnectorType_eCompressed));

        NEXUS_AudioOutput_GetSettings(NEXUS_AudioDummyOutput_GetConnector(platformConfig.outputs.audioDummy[0]),
                                      &outputSettings);
        outputSettings.nco = NEXUS_AudioOutputNco_eMax;
        outputSettings.pll = NEXUS_AudioOutputPll_e0;
        NEXUS_AudioOutput_SetSettings(NEXUS_AudioDummyOutput_GetConnector(platformConfig.outputs.audioDummy[0]),
                                      &outputSettings);
    }

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

    NEXUS_StcChannel_GetDefaultSettings(0, &stcSettings);
    stcSettings.timebase = NEXUS_Timebase_e0;
    stcSettings.mode = NEXUS_StcChannelMode_eAuto;
    stcChannel = NEXUS_StcChannel_Open(0, &stcSettings);


    /* Tell playpump that it's TS. */
    NEXUS_Playback_GetSettings(playback, &playbackSettings);
    playbackSettings.playpump = playpump;
    playbackSettings.playpumpSettings.transportType = TRANSPORT_TYPE;
    playbackSettings.stcChannel = stcChannel;
    NEXUS_Playback_SetSettings(playback, &playbackSettings);

    /* Playback must be told which stream ID(track) is used for video and for audio. */
    NEXUS_Playback_GetDefaultPidChannelSettings(&pidSettings);
    pidSettings.pidSettings.pidType = NEXUS_PidType_eVideo;
    pidSettings.pidTypeSettings.video.codec = VIDEO_CODEC; /* must be told codec for correct parsing */
    pidSettings.pidTypeSettings.video.index = true;
    pidSettings.pidTypeSettings.video.decoder = videoDecoder;
    videoPidChannel = NEXUS_Playback_OpenPidChannel(playback, VIDEO_PID, &pidSettings);

    NEXUS_Playback_GetDefaultPidChannelSettings(&pidSettings);
    pidSettings.pidSettings.pidType = NEXUS_PidType_eAudio;
    pidSettings.pidSettings.pidTypeSettings.audio.codec = AUDIO_CODEC;
    pidSettings.pidTypeSettings.audio.primary = pcmDecoder;
    pidSettings.pidTypeSettings.audio.secondary = compressedDecoder;
    audioPidChannel = NEXUS_Playback_OpenPidChannel(playback, AUDIO_PID, &pidSettings);


    /* Set up decoder Start structures now. We need to know the audio codec to properly set up the audio outputs. */
    NEXUS_VideoDecoder_GetDefaultStartSettings(&videoProgram);
    videoProgram.codec = VIDEO_CODEC;
    videoProgram.pidChannel = videoPidChannel;
    videoProgram.stcChannel = stcChannel;

    NEXUS_AudioDecoder_GetDefaultStartSettings(&audioProgram);
    audioProgram.codec = AUDIO_CODEC;
    audioProgram.pidChannel = audioPidChannel;
    audioProgram.stcChannel = stcChannel;

    /* Start decoders */
    NEXUS_VideoDecoder_Start(videoDecoder, &videoProgram);
    NEXUS_AudioDecoder_Start(pcmDecoder, &audioProgram);

    NEXUS_AudioDecoder_IsPassthroughCodecSupported(compressedDecoder, audioProgram.codec, &compressedSupported);
    if (compressedSupported)
    {
        NEXUS_AudioDecoder_Start(compressedDecoder, &audioProgram);
    }

    /* Start playback */
    NEXUS_Playback_Start(playback, file, NULL);

    /* Playback state machine is driven from inside Nexus. */

    while (!done)
    {
#if NEXUS_NUM_HDMI_OUTPUTS
        bool hdmiWasMuted;
#endif
        bool spdifWasMuted;
        int tmp;

        /* Display Menu */
        printf("Main Menu\n");
        printf(" 0) Exit\n");
        printf(" 1) Reconfigure the digital outputs to %s\n", pcm?"COMPRESSED":"PCM");
        printf(" 2) Mute/Unmute (currently %s)\n",mute?"MUTED":"UNMUTE");
        printf(" 3) Stops decoders\n");
        printf(" 4) Starts decoders\n");
        printf(" 5) Suspends decoders\n");
        printf(" 6) Resumes decoders\n");
        printf("Enter Selection: \n");
        scanf("%d", &tmp);

        switch ( tmp )
        {
        case 0:
            done = true;
            break;
        case 1:
            if (compressedDecoder && !stopped && !suspended)
            {
                if (compressedSupported)
                {
                    if (pcm) /* going to compressesed */
                    {
#if NEXUS_NUM_HDMI_OUTPUTS
                        if (audioHdmiHandle) {
                            NEXUS_HdmiOutput_GetSettings(platformConfig.outputs.hdmi[0], &hdmiSettings);
                            hdmiSettings.audioBurstType = NEXUS_SpdifOutputBurstType_ePause;
                            NEXUS_HdmiOutput_SetSettings(platformConfig.outputs.hdmi[0], &hdmiSettings);
                        }
#endif
                        if (audioSpdifHandle) {
                            NEXUS_SpdifOutput_GetSettings(platformConfig.outputs.spdif[0], &spdifSettings);
                            spdifSettings.burstType = NEXUS_SpdifOutputBurstType_ePause;
                            NEXUS_SpdifOutput_SetSettings(platformConfig.outputs.spdif[0], &spdifSettings);
                        }
                    }
                    else /* going to pcm */
                    {
#if NEXUS_NUM_HDMI_OUTPUTS
                        if (audioHdmiHandle) {
                            NEXUS_HdmiOutput_GetSettings(platformConfig.outputs.hdmi[0], &hdmiSettings);
                            hdmiSettings.audioBurstType = NEXUS_SpdifOutputBurstType_eNone;
                            NEXUS_HdmiOutput_SetSettings(platformConfig.outputs.hdmi[0], &hdmiSettings);
                        }
#endif
                        if (audioSpdifHandle) {
                            NEXUS_SpdifOutput_GetSettings(platformConfig.outputs.spdif[0], &spdifSettings);
                            spdifSettings.burstType = NEXUS_SpdifOutputBurstType_eNone;
                            NEXUS_SpdifOutput_SetSettings(platformConfig.outputs.spdif[0], &spdifSettings);
                        }
                    }

                    /* save mute state and set outputs to muted */
#if NEXUS_NUM_HDMI_OUTPUTS
                    if (audioHdmiHandle) {
                        NEXUS_AudioOutput_GetSettings(audioHdmiHandle, &outputSettings);
                        hdmiWasMuted = outputSettings.muted;
                        outputSettings.muted = true;
                        NEXUS_AudioOutput_SetSettings(audioHdmiHandle, &outputSettings);
                    }
#endif
                    if (audioSpdifHandle) {
                        NEXUS_AudioOutput_GetSettings(audioSpdifHandle, &outputSettings);
                        spdifWasMuted = outputSettings.muted;
                        outputSettings.muted = true;
                        NEXUS_AudioOutput_SetSettings(audioSpdifHandle, &outputSettings);
                    }
                    /* change connections */
                    NEXUS_AudioDecoder_Suspend(pcmDecoder);
                    NEXUS_AudioDecoder_Suspend(compressedDecoder);

                    if (pcm) /* going to compressesed */
                    {
#if NEXUS_NUM_HDMI_OUTPUTS
                        if (audioHdmiHandle) {
                            NEXUS_AudioOutput_RemoveInput(audioHdmiHandle,
                                NEXUS_AudioDecoder_GetConnector(pcmDecoder, NEXUS_AudioDecoderConnectorType_eMultichannel));
                            NEXUS_AudioOutput_AddInput(audioHdmiHandle,
                                NEXUS_AudioDecoder_GetConnector(compressedDecoder, NEXUS_AudioDecoderConnectorType_eCompressed));
                        }
#endif
                        if (audioSpdifHandle) {
                            NEXUS_AudioOutput_RemoveInput(audioSpdifHandle,
                                NEXUS_AudioDecoder_GetConnector(pcmDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
                            NEXUS_AudioOutput_AddInput(audioSpdifHandle,
                                NEXUS_AudioDecoder_GetConnector(compressedDecoder, NEXUS_AudioDecoderConnectorType_eCompressed));
                        }
                        if (audioDummyHandle) {
                            NEXUS_AudioOutput_RemoveInput(audioDummyHandle,
                                NEXUS_AudioDecoder_GetConnector(compressedDecoder, NEXUS_AudioConnectorType_eCompressed));
                            NEXUS_AudioOutput_AddInput(audioDummyHandle,
                                NEXUS_AudioDecoder_GetConnector(pcmDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
                        }
                    }
                    else /* going to pcm */
                    {
#if NEXUS_NUM_HDMI_OUTPUTS
                        if (audioHdmiHandle) {
                            NEXUS_AudioOutput_RemoveInput(audioHdmiHandle,
                                NEXUS_AudioDecoder_GetConnector(compressedDecoder, NEXUS_AudioDecoderConnectorType_eCompressed));
                            NEXUS_AudioOutput_AddInput(audioHdmiHandle,
                                NEXUS_AudioDecoder_GetConnector(pcmDecoder, NEXUS_AudioDecoderConnectorType_eMultichannel));
                        }
#endif
                        if (audioSpdifHandle) {
                            NEXUS_AudioOutput_RemoveInput(audioSpdifHandle,
                                NEXUS_AudioDecoder_GetConnector(compressedDecoder, NEXUS_AudioDecoderConnectorType_eCompressed));
                            NEXUS_AudioOutput_AddInput(audioSpdifHandle,
                                NEXUS_AudioDecoder_GetConnector(pcmDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
                        }
                        if (audioDummyHandle) {
                            NEXUS_AudioOutput_RemoveInput(audioDummyHandle,
                                NEXUS_AudioDecoder_GetConnector(pcmDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
                            NEXUS_AudioOutput_AddInput(audioDummyHandle,
                                NEXUS_AudioDecoder_GetConnector(compressedDecoder, NEXUS_AudioConnectorType_eCompressed));
                        }
                    }
                    NEXUS_AudioDecoder_Resume(pcmDecoder);
                    NEXUS_AudioDecoder_Resume(compressedDecoder);

                    /* restore previous mute state */
#if NEXUS_NUM_HDMI_OUTPUTS
                    if (audioHdmiHandle) {
                        NEXUS_AudioOutput_GetSettings(audioHdmiHandle, &outputSettings);
                        outputSettings.muted = hdmiWasMuted;
                        NEXUS_AudioOutput_SetSettings(audioHdmiHandle, &outputSettings);
                    }
#endif
                    if (audioSpdifHandle) {
                        NEXUS_AudioOutput_GetSettings(audioSpdifHandle, &outputSettings);
                        outputSettings.muted = spdifWasMuted;
                        NEXUS_AudioOutput_SetSettings(audioSpdifHandle, &outputSettings);
                    }
                    pcm = !pcm;
                }
                else
                {
                    BDBG_ERR(("Codec %d does not support passthrough",audioProgram.codec));
                }
            }
            else if (stopped || suspended)
            {
                BDBG_ERR(("Decoders are currently %s",suspended?"suspended":"stopped"));
            }
            break;
        case 2: /* mutes and unmutes Spdif/HDMI */
            mute = !mute;
#if NEXUS_NUM_HDMI_OUTPUTS
            if (audioHdmiHandle) {
                NEXUS_AudioOutput_GetSettings(audioHdmiHandle, &outputSettings);
                outputSettings.muted = mute;
                NEXUS_AudioOutput_SetSettings(audioHdmiHandle, &outputSettings);
            }
#endif
            if (audioSpdifHandle) {
                NEXUS_AudioOutput_GetSettings(audioSpdifHandle, &outputSettings);
                outputSettings.muted = mute;
                NEXUS_AudioOutput_SetSettings(audioSpdifHandle, &outputSettings);
            }
            break;
        case 3: /* stops decoders */
            NEXUS_AudioDecoder_Stop(pcmDecoder);
            if (compressedSupported)
            {
                NEXUS_AudioDecoder_Stop(compressedDecoder);
            }
            stopped = true;
            suspended = false;
            break;
        case 4: /* starts decoders */
            NEXUS_AudioDecoder_Start(pcmDecoder, &audioProgram);
            if (compressedSupported)
            {
                NEXUS_AudioDecoder_Start(compressedDecoder, &audioProgram);
            }
            stopped = false;
            break;
        case 5: /* suspends decoders */
            NEXUS_AudioDecoder_Suspend(pcmDecoder);
            if (compressedSupported)
            {
                NEXUS_AudioDecoder_Suspend(compressedDecoder);
            }
            suspended = true;
            break;
        case 6: /* resumes decoders */
            NEXUS_AudioDecoder_Resume(pcmDecoder);
            if (compressedSupported)
            {
                NEXUS_AudioDecoder_Resume(compressedDecoder);
            }
            suspended = false;
            break;
        default:
            break;
        }
    }

    /* Bring down system */
    NEXUS_VideoDecoder_Stop(videoDecoder);
    NEXUS_AudioDecoder_Stop(pcmDecoder);
    NEXUS_AudioDecoder_Stop(compressedDecoder);
    NEXUS_Playback_Stop(playback);
    NEXUS_FilePlay_Close(file);
    NEXUS_Playback_Destroy(playback);
    NEXUS_Playpump_Close(playpump);
    NEXUS_AudioInput_Shutdown(NEXUS_AudioDecoder_GetConnector(pcmDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
    NEXUS_AudioInput_Shutdown(NEXUS_AudioDecoder_GetConnector(pcmDecoder, NEXUS_AudioDecoderConnectorType_eMultichannel));
    NEXUS_AudioInput_Shutdown(NEXUS_AudioDecoder_GetConnector(compressedDecoder, NEXUS_AudioDecoderConnectorType_eCompressed));
    NEXUS_VideoInput_Shutdown(NEXUS_VideoDecoder_GetConnector(videoDecoder));
    NEXUS_VideoDecoder_Close(videoDecoder);
    NEXUS_AudioDecoder_Close(pcmDecoder);
    NEXUS_AudioDecoder_Close(compressedDecoder);
    NEXUS_VideoWindow_Close(window);
    NEXUS_Display_Close(display);
    NEXUS_Platform_Uninit();

#else
    BSTD_UNUSED(argc);
    BSTD_UNUSED(argv);
    printf("This application is not supported on this platform!\n");
#endif
    return 0;
}
#else /* NEXUS_HAS_AUDIO */
int main(void)
{
    printf("This application is not supported on this platform (needs audio)!\n");
    return 0;
}
#endif
