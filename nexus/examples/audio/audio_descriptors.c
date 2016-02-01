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
/* Nexus example app: single live a/v decode from an input band */

#include "nexus_platform.h"
#include <stdio.h>

#if NEXUS_HAS_AUDIO
#include "nexus_pid_channel.h"
#include "nexus_parser_band.h"
#include "nexus_video_decoder.h"
#include "nexus_stc_channel.h"
#include "nexus_display.h"
#include "nexus_video_window.h"
#include "nexus_audio_dac.h"
#include "nexus_audio_decoder.h"
#include "nexus_audio_mixer.h"
#include "nexus_audio_output.h"
#include "nexus_spdif_output.h"
#include "nexus_composite_output.h"
#include "nexus_component_output.h"
#if NEXUS_HAS_HDMI_OUTPUT
#include "nexus_hdmi_output.h"
#endif
#if NEXUS_HAS_PLAYBACK
#include "nexus_playback.h"
#include "nexus_file.h"
#endif

#include "bstd.h"
#include "bkni.h"
#include <stdlib.h>
#include <string.h>

BDBG_MODULE(audio_descriptors);

#if NEXUS_NUM_SPDIF_OUTPUTS
#define USE_SPDIF 1
#define USE_COMPRESSED_SPDIF 0
#endif

/* the following define the input and its characteristics -- these will vary by input */
/* these values correspond to UK-DTT-MUXC-20070706.trp */
#define TRANSPORT_TYPE NEXUS_TransportType_eTs
#define VIDEO_CODEC NEXUS_VideoCodec_eMpeg2
#define AUDIO_CODEC NEXUS_AudioCodec_eMpeg
#define VIDEO_PID 501
#define AUDIO_PID 502
#define DESC_PID  504

int main(int argc, char **argv)
{
    NEXUS_PlatformSettings platformSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_InputBand inputBand;
    NEXUS_ParserBand parserBand;
    NEXUS_ParserBandSettings parserBandSettings;
    NEXUS_StcChannelHandle stcChannel;
    NEXUS_StcChannelSettings stcSettings;
    NEXUS_PidChannelHandle videoPidChannel, audioPidChannel, descriptionPidChannel;
    NEXUS_DisplayHandle display;
    NEXUS_DisplaySettings displaySettings;
    NEXUS_VideoWindowHandle window;
    NEXUS_VideoDecoderHandle vdecode;
    NEXUS_VideoDecoderStartSettings videoProgram;
    NEXUS_AudioDecoderHandle pcmDecoder, descriptorDecoder;
#if USE_COMPRESSED_SPDIF
    NEXUS_AudioDecoderHandle compressedDecoder;
#endif
    NEXUS_AudioDecoderStartSettings audioProgram, audioDescProgram;
    NEXUS_AudioDecoderOpenSettings audioOpenSettings;
    NEXUS_AudioDecoderSettings audioSettings;
    NEXUS_AudioMixerHandle mixer;
    NEXUS_AudioMixerSettings mixerSettings;
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_HdmiOutputStatus hdmiStatus;
    NEXUS_Error rc;
#endif
#if NEXUS_HAS_PLAYBACK
    NEXUS_FilePlayHandle file;
    NEXUS_PlaypumpHandle playpump;
    NEXUS_PlaybackHandle playback;
    NEXUS_PlaybackSettings playbackSettings;
    NEXUS_PlaybackPidChannelSettings playbackPidSettings;
#endif
    bool audioDescription = false;
    char *fileName = NULL;
    unsigned videoPid = VIDEO_PID;
    unsigned audioPid = AUDIO_PID;
    unsigned descriptionPid = DESC_PID;
    NEXUS_TransportType transportType = TRANSPORT_TYPE;
    NEXUS_VideoCodec videoCodec = VIDEO_CODEC;
    NEXUS_AudioCodec audioCodec = AUDIO_CODEC;
    int curarg = 0;

    /* Parse command-line arguments */
    while (++curarg < argc) {
        if (!strcmp(argv[curarg], "-help"))
        {
            printf("\nUsage: nexus audio_descriptors\n");
            printf("-transport              ts, asf, avi, mp4, mkv\n");
            printf("-video_codec            none, mpeg1, mpeg2, mpeg4part2, h263, h264, vc1, divx311, h265\n");
            printf("-audio_codec            none, mpeg, mp3, aac, aac+, ac3, ac3+, wmaStd, wmaPro\n");
            printf("-video_id               video pid\n");
            printf("-audio_id               audio pid\n");
            printf("-description_id         audio description pid (if value is 0, audio description will be disabled)\n");
            printf("-file                   file name. if not setting it, streamer will be used not playback\n");
            return -1;
        }
        else if (!strcmp(argv[curarg], "-video_codec") && curarg+1 < argc)
        {
            curarg++;
            if( !strcmp(argv[curarg], "mpeg1"))
                videoCodec = NEXUS_VideoCodec_eMpeg1;
            else if( !strcmp(argv[curarg], "mpeg2"))
                videoCodec = NEXUS_VideoCodec_eMpeg2;
            else if( !strcmp(argv[curarg], "mpeg4part2"))
                videoCodec = NEXUS_VideoCodec_eMpeg4Part2;
            else if( !strcmp(argv[curarg], "h263"))
                videoCodec = NEXUS_VideoCodec_eH263;
            else if( !strcmp(argv[curarg], "h264"))
                videoCodec = NEXUS_VideoCodec_eH264;
            else if( !strcmp(argv[curarg], "vc1"))
                videoCodec = NEXUS_VideoCodec_eVc1;
            else if( !strcmp(argv[curarg], "divx311"))
                videoCodec = NEXUS_VideoCodec_eDivx311;
            else if( !strcmp(argv[curarg], "h265"))
                videoCodec = NEXUS_VideoCodec_eH265;
            else
            {
                printf("video codec %s is not supported by this application", argv[curarg]);
                return -1;
            }
        }
        else if (!strcmp(argv[curarg], "-audio_codec") && curarg+1 < argc)
        {
            curarg++;
            if( !strcmp(argv[curarg], "mpeg"))
                audioCodec = NEXUS_AudioCodec_eMpeg;
            else if( !strcmp(argv[curarg], "mp3"))
                audioCodec = NEXUS_AudioCodec_eMp3;
            else if( !strcmp(argv[curarg], "aac"))
                audioCodec = NEXUS_AudioCodec_eAac;
            else if( !strcmp(argv[curarg], "aac+"))
                audioCodec = NEXUS_AudioCodec_eAacPlus;
            else if( !strcmp(argv[curarg], "ac3"))
                audioCodec = NEXUS_AudioCodec_eAc3;
            else if( !strcmp(argv[curarg], "ac3+"))
                audioCodec = NEXUS_AudioCodec_eAc3Plus;
            else if( !strcmp(argv[curarg], "wmaStd"))
                audioCodec = NEXUS_AudioCodec_eWmaStd;
            else if( !strcmp(argv[curarg], "wmaPro"))
                audioCodec = NEXUS_AudioCodec_eWmaPro;
            else
            {
                printf("audio codec %s is not supported by this application", argv[curarg]);
                return -1;
            }
        }
        else if (!strcmp(argv[curarg], "-video_id") && curarg+1 < argc)
        {
            if((!strncmp(argv[++curarg], "0X", 2)) || (!strncmp(argv[curarg], "0x", 2)))
                videoPid = strtol(argv[curarg], NULL, 0);
            else
                videoPid = atoi(argv[curarg]);
        }
        else if (!strcmp(argv[curarg], "-audio_id") && curarg+1 < argc)
        {
            if((!strncmp(argv[++curarg], "0X", 2)) || (!strncmp(argv[curarg], "0x", 2)))
                audioPid = strtol(argv[curarg], NULL, 0);
            else
                audioPid = atoi(argv[curarg]);
        }
        else if (!strcmp(argv[curarg], "-description_id") && curarg+1 < argc)
        {
            if((!strncmp(argv[++curarg], "0X", 2)) || (!strncmp(argv[curarg], "0x", 2)))
                descriptionPid = strtol(argv[curarg], NULL, 0);
            else
                descriptionPid = atoi(argv[curarg]);
        }
        else if (!strcmp(argv[curarg], "-file") && curarg+1 < argc)
        {
            curarg++;
            #if NEXUS_HAS_PLAYBACK
            fileName = argv[curarg];
            #endif
        }
    }

    /* Bring up all modules for a platform in a default configuration for this platform */
    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init(&platformSettings);
    NEXUS_Platform_GetConfiguration(&platformConfig);

    if(descriptionPid !=0)
    {
        audioDescription = true;
    }

    /* Bring up the primary audio decoder */
    pcmDecoder = NEXUS_AudioDecoder_Open(0, NULL);

    /* Open audio descriptor decoder */
    NEXUS_AudioDecoder_GetDefaultOpenSettings(&audioOpenSettings);
    audioOpenSettings.type = NEXUS_AudioDecoderType_eAudioDescriptor;
    descriptorDecoder = NEXUS_AudioDecoder_Open(1, &audioOpenSettings);

#if USE_COMPRESSED_SPDIF
    /* Open a decoder for compressed passthrough */
    compressedDecoder = NEXUS_AudioDecoder_Open(2, NULL);
#endif

    /* Link the descriptor and primary decoders */
    NEXUS_AudioDecoder_GetSettings(pcmDecoder, &audioSettings);
    audioSettings.descriptionDecoder = descriptorDecoder;
    NEXUS_AudioDecoder_SetSettings(pcmDecoder, &audioSettings);

    /* Open mixer to mix the description and primary audio */
    NEXUS_AudioMixer_GetDefaultSettings(&mixerSettings);
#if NEXUS_AUDIO_MODULE_FAMILY == NEXUS_AUDIO_MODULE_FAMILY_APE_RAAGA
    mixerSettings.mixUsingDsp = true;
#endif    
    mixer = NEXUS_AudioMixer_Open(&mixerSettings);

    /* Add both decoders to the mixer */
    NEXUS_AudioMixer_AddInput(mixer, NEXUS_AudioDecoder_GetConnector(pcmDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
    if(audioDescription == true)
    {
        NEXUS_AudioMixer_AddInput(mixer, NEXUS_AudioDecoder_GetConnector(descriptorDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
    }

#if NEXUS_AUDIO_MODULE_FAMILY == NEXUS_AUDIO_MODULE_FAMILY_APE_RAAGA
    NEXUS_AudioMixer_GetSettings(mixer, &mixerSettings);
    mixerSettings.master = NEXUS_AudioDecoder_GetConnector(pcmDecoder, NEXUS_AudioDecoderConnectorType_eStereo);
    NEXUS_AudioMixer_SetSettings(mixer, &mixerSettings);
#endif    

    /* Add DAC and HDMI to the mixer output */
#if NEXUS_NUM_AUDIO_DACS
    NEXUS_AudioOutput_AddInput(
        NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]),
        NEXUS_AudioMixer_GetConnector(mixer));
#endif
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_AudioOutput_AddInput(
        NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]),
        NEXUS_AudioMixer_GetConnector(mixer));
#endif

#if USE_COMPRESSED_SPDIF
    /* Use a passthrough decoder for spdif */
    NEXUS_AudioOutput_AddInput(
        NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]),
        NEXUS_AudioDecoder_GetConnector(compressedDecoder, NEXUS_AudioDecoderConnectorType_eCompressed));
#elif USE_SPDIF
    /* Attach SPDIF to the mixed output */
    NEXUS_AudioOutput_AddInput(
        NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]),
        NEXUS_AudioMixer_GetConnector(mixer));
#endif

    /* Bring up video display and outputs */
    NEXUS_Display_GetDefaultSettings(&displaySettings);
    display = NEXUS_Display_Open(0, &displaySettings);
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
    vdecode = NEXUS_VideoDecoder_Open(0, NULL); /* take default capabilities */
    NEXUS_VideoWindow_AddInput(window, NEXUS_VideoDecoder_GetConnector(vdecode));

    if(fileName != NULL)
    {
        #if NEXUS_HAS_PLAYBACK
        playpump = NEXUS_Playpump_Open(NEXUS_ANY_ID, NULL);
        BDBG_ASSERT(playpump);
        playback = NEXUS_Playback_Create();
        BDBG_ASSERT(playback);

        file = NEXUS_FilePlay_OpenPosix(fileName, NULL);
        if (!file)
        {
            fprintf(stderr, "can't open file:%s\n", fileName);
            return -1;
        }

        NEXUS_StcChannel_GetDefaultSettings(0, &stcSettings);
        stcSettings.timebase = NEXUS_Timebase_e0;
        stcSettings.mode = NEXUS_StcChannelMode_eAuto;
        stcChannel = NEXUS_StcChannel_Open(0, &stcSettings);

        NEXUS_Playback_GetSettings(playback, &playbackSettings);
        playbackSettings.playpump = playpump;
        /* set a stream format, it could be any audio video transport type or file format, i.e NEXUS_TransportType_eMp4, NEXUS_TransportType_eAvi ... */
        playbackSettings.playpumpSettings.transportType = transportType;
        playbackSettings.stcChannel = stcChannel;
        NEXUS_Playback_SetSettings(playback, &playbackSettings);

        /* Open the audio and video pid channels */
        NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
        playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eVideo;
        playbackPidSettings.pidTypeSettings.video.codec = videoCodec; /* must be told codec for correct handling */
        playbackPidSettings.pidTypeSettings.video.index = true;
        playbackPidSettings.pidTypeSettings.video.decoder = vdecode;
        videoPidChannel = NEXUS_Playback_OpenPidChannel(playback, videoPid, &playbackPidSettings);

        NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
        playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eAudio;
        playbackPidSettings.pidTypeSettings.audio.primary = pcmDecoder;
#if USE_COMPRESSED_SPDIF
        playbackPidSettings.pidTypeSettings.audio.secondary = compressedDecoder;
#endif
        audioPidChannel = NEXUS_Playback_OpenPidChannel(playback,  audioPid, &playbackPidSettings);

        if(audioDescription == true)
        {
            NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
            playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eAudio;
            playbackPidSettings.pidTypeSettings.audio.primary = descriptorDecoder;
            descriptionPidChannel = NEXUS_Playback_OpenPidChannel(playback, descriptionPid, &playbackPidSettings);
        }
        #endif
    }
    else
    {
        /* Get the streamer input band from Platform. Platform has already configured the FPGA with a default streamer routing */
        NEXUS_Platform_GetStreamerInputBand(0, &inputBand);

        /* Map a parser band to the streamer input band. */
        parserBand = NEXUS_ParserBand_e0;
        NEXUS_ParserBand_GetSettings(parserBand, &parserBandSettings);
        parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eInputBand;
        parserBandSettings.sourceTypeSettings.inputBand = inputBand;
        parserBandSettings.transportType = transportType;
        NEXUS_ParserBand_SetSettings(parserBand, &parserBandSettings);

        /* Open the audio and video pid channels */
        videoPidChannel = NEXUS_PidChannel_Open(parserBand, videoPid, NULL);
        audioPidChannel = NEXUS_PidChannel_Open(parserBand, audioPid, NULL);
        if(audioDescription == true)
            descriptionPidChannel = NEXUS_PidChannel_Open(parserBand, descriptionPid, NULL);

        /* Open the StcChannel to do lipsync with the PCR */
        NEXUS_StcChannel_GetDefaultSettings(0, &stcSettings);
        stcSettings.timebase = NEXUS_Timebase_e0;
        stcSettings.mode = NEXUS_StcChannelMode_ePcr; /* live */
        stcSettings.modeSettings.pcr.pidChannel = videoPidChannel; /* PCR happens to be on video pid */
        stcChannel = NEXUS_StcChannel_Open(0, &stcSettings);
    }

    /* Set up decoder Start structures now. We need to know the audio codec to properly set up the audio outputs. */
    NEXUS_VideoDecoder_GetDefaultStartSettings(&videoProgram);
    videoProgram.codec = videoCodec;
    videoProgram.pidChannel = videoPidChannel;
    videoProgram.stcChannel = stcChannel;
    NEXUS_AudioDecoder_GetDefaultStartSettings(&audioProgram);
    audioProgram.codec = audioCodec;
    audioProgram.pidChannel = audioPidChannel;
    audioProgram.stcChannel = stcChannel;
    if(audioDescription == true)
    {
        NEXUS_AudioDecoder_GetDefaultStartSettings(&audioDescProgram);
        audioDescProgram.codec = audioCodec;
        audioDescProgram.pidChannel = descriptionPidChannel;
        audioDescProgram.stcChannel = stcChannel;
    }

    /* Start Decoders */
    NEXUS_VideoDecoder_Start(vdecode, &videoProgram);

    BDBG_WRN(("Starting PCM"));
    NEXUS_AudioDecoder_Start(pcmDecoder, &audioProgram);
    if(audioDescription == true)
    {
        BDBG_WRN(("Starting Descriptor"));
        audioDescProgram.secondaryDecoder = true;
        NEXUS_AudioDecoder_Start(descriptorDecoder, &audioDescProgram);
    }
#if USE_COMPRESSED_SPDIF
    NEXUS_AudioDecoder_Start(compressedDecoder, &audioProgram);
#endif

    if(fileName != NULL)
    {
        #if NEXUS_HAS_PLAYBACK
        /* Start playback */
        NEXUS_Playback_Start(playback, file, NULL);
        #endif
    }


#if 1
    /* Print status while decoding */
    for (;;) {
        NEXUS_VideoDecoderStatus status;
        NEXUS_AudioDecoderStatus audioStatus;
        uint32_t stc;

        NEXUS_VideoDecoder_GetStatus(vdecode, &status);
        NEXUS_StcChannel_GetStc(videoProgram.stcChannel, &stc);
        printf("decode %.4dx%.4d, pts %#x, stc %#x (diff %d) fifo=%d%%\n",
            status.source.width, status.source.height, status.pts, stc, status.ptsStcDifference, status.fifoSize?(status.fifoDepth*100)/status.fifoSize:0);
        NEXUS_AudioDecoder_GetStatus(pcmDecoder, &audioStatus);
        printf("audio            pts %#x, stc %#x (diff %d) fifo=%d%%\n",
            audioStatus.pts, stc, audioStatus.ptsStcDifference, audioStatus.fifoSize?(audioStatus.fifoDepth*100)/audioStatus.fifoSize:0);
#if USE_COMPRESSED_SPDIF
        NEXUS_AudioDecoder_GetStatus(compressedDecoder, &audioStatus);
        if ( audioStatus.started )
        {
            printf("compressed        pts %#x, stc %#x (diff %d) fifo=%d%%\n",
                audioStatus.pts, stc, audioStatus.ptsStcDifference, audioStatus.fifoSize?(audioStatus.fifoDepth*100)/audioStatus.fifoSize:0);
        }
#endif
        if(audioDescription == true)
        {
            NEXUS_AudioDecoder_GetStatus(descriptorDecoder, &audioStatus);
            if ( audioStatus.started )
            {
                printf("descriptor        pts %#x, stc %#x (diff %d) fifo=%d%%\n",
                audioStatus.pts, stc, audioStatus.ptsStcDifference, audioStatus.fifoSize?(audioStatus.fifoDepth*100)/audioStatus.fifoSize:0);
            }
        }
        BKNI_Sleep(1000);
    }
#else
    printf("Press ENTER to quit\n");
    getchar();

    /* Bring down system */
    NEXUS_VideoDecoder_Stop(vdecode);
    NEXUS_AudioDecoder_Stop(pcmDecoder);
    if(audioDescription == true)
        NEXUS_AudioDecoder_Stop(descriptorDecoder);
#if USE_COMPRESSED_SPDIF
    NEXUS_AudioDecoder_Stop(compressedDecoder);
#endif

#if NEXUS_NUM_AUDIO_DACS
    NEXUS_AudioOutput_RemoveAllInputs(NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]));
#endif
#if NEXUS_NUM_SPDIF_OUTPUTS
    NEXUS_AudioOutput_RemoveAllInputs(NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]));
#endif
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_AudioOutput_RemoveAllInputs(NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]));
#endif
    NEXUS_AudioMixer_RemoveAllInputs(mixer);
    NEXUS_AudioMixer_Close(mixer);
    NEXUS_VideoWindow_RemoveAllInputs(window);
    if(fileName != NULL)
    {
        NEXUS_Playback_Stop(playback);
        NEXUS_Playback_CloseAllPidChannels(playback);
        NEXUS_FilePlay_Close(file);
        NEXUS_Playback_Destroy(playback);
        NEXUS_Playpump_Close(playpump);
    }
    else
    {
        NEXUS_PidChannel_CloseAll(parserBand);
    }
    NEXUS_VideoDecoder_Close(vdecode);
    NEXUS_AudioDecoder_Close(pcmDecoder);
    NEXUS_AudioDecoder_Close(descriptorDecoder);
#if USE_COMPRESSED_SPDIF
    NEXUS_AudioDecoder_Close(compressedDecoder);
#endif
    NEXUS_VideoWindow_Close(window);
    NEXUS_Display_Close(display);
    NEXUS_StcChannel_Close(stcChannel);
    NEXUS_Platform_Uninit();
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
