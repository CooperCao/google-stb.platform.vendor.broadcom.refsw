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
******************************************************************************/
/* Nexus example app: single live a/v decode from an input band, routed to hdmi output */

#include "nexus_platform.h"
#if NEXUS_NUM_HDMI_OUTPUTS && NEXUS_NUM_SPDIF_OUTPUTS
#include "nexus_pid_channel.h"
#include "nexus_parser_band.h"
#include "nexus_video_decoder.h"
#include "nexus_stc_channel.h"
#include "nexus_display.h"
#include "nexus_video_window.h"
#include "nexus_audio_dac.h"
#include "nexus_audio_decoder.h"
#include "nexus_audio_output.h"
#include "nexus_spdif_output.h"
#include "nexus_hdmi_output.h"
#include "nexus_component_output.h"
#include "nexus_hdmi_output_hdcp.h"
#include "nexus_core_utils.h"

#if NEXUS_HAS_PLAYBACK
#define ENABLE_PLAYBACK     0
#if ENABLE_PLAYBACK
#include "nexus_playback.h"
#include "nexus_file.h"
#endif
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "bstd.h"

#include "bkni.h"

BDBG_MODULE(audio_digital_output) ;

/* the following define the input file and its characteristics -- these will vary by input file */
#if ENABLE_PLAYBACK
static const char *fname = "/mnt/streams/streamer/bugs_toys2_jurassic_q64_cd.mpg";
#endif
#define TRANSPORT_TYPE NEXUS_TransportType_eTs
#define VIDEO_CODEC NEXUS_VideoCodec_eMpeg2
#define AUDIO_CODEC NEXUS_AudioCodec_eAc3
#define VIDEO_PID 0x11
#define AUDIO_PID 0x14


#if NEXUS_HAS_HDMI_OUTPUT
/*****************/
/* For HDCP TESTING  */
/*    1) insert the Production Key Set set generated by BCrypt  */
/*    2) set the USE_PRODUCTION_KEYS macro to to 1 */
/*****************/
#define USE_PRODUCTION_KEYS     0

typedef struct hotplugCallbackParameters
{
    NEXUS_HdmiOutputHandle hdmiOutput;
    NEXUS_DisplayHandle display;
} hotplugCallbackParameters;

static bool hdmiHdcpEnabled = false ;
static void initializeHdmiOutputHdcpSettings(void);
static void hdmiOutputHdcpStateChanged(void *pContext, int param);
static void hotplug_callback(void *pParam, int iParam);
#endif


static NEXUS_PlatformConfiguration platformConfig;
static NEXUS_AudioDecoderHandle pcmDecoder, compressedDecoder;
static NEXUS_AudioDecoderStartSettings audioProgram;

int main(void)
{
    NEXUS_VideoDecoderHandle videoDecoder;
    NEXUS_VideoDecoderStartSettings videoProgram;
    NEXUS_StcChannelHandle stcChannel;
    NEXUS_StcChannelSettings stcSettings;
    NEXUS_PidChannelHandle videoPidChannel, audioPidChannel;
    NEXUS_DisplayHandle display;
    NEXUS_VideoWindowHandle window;
    NEXUS_PlatformSettings platformSettings;
#if ENABLE_PLAYBACK
    NEXUS_FilePlayHandle file;
    NEXUS_PlaypumpHandle playpump;
    NEXUS_PlaybackHandle playback;
    NEXUS_PlaybackSettings playbackSettings;
    NEXUS_PlaybackPidChannelSettings playbackPidSettings;
#else
    NEXUS_ParserBand parserBand = NEXUS_ParserBand_e0;
    NEXUS_ParserBandSettings parserBandSettings;
#endif
    NEXUS_DisplaySettings displaySettings;
#if NEXUS_HAS_HDMI_OUTPUT
    NEXUS_HdmiOutputSettings hdmiSettings;
    NEXUS_HdmiOutputStatus hdmiStatus;
    hotplugCallbackParameters hotPlugCbParams;
#endif
    NEXUS_AudioOutputSettings outputSettings;
    NEXUS_SpdifOutputSettings spdifSettings;
    bool mute = false;
    bool decoderMute = false;
    bool decoderDiminished = false;
    bool started = true;
    bool pcm = true;
    bool done = false;
    bool pauseBurst = true;
    bool multichannel = true;
    
    /* Bring up all modules for a platform in a default configuration for this platform */
    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init(&platformSettings);
    NEXUS_Platform_GetConfiguration(&platformConfig);

    /* bring up decoders and connect to display */
    pcmDecoder = NEXUS_AudioDecoder_Open(0, NULL);
    compressedDecoder = NEXUS_AudioDecoder_Open(1, NULL);
    videoDecoder = NEXUS_VideoDecoder_Open(0, NULL); /* take default capabilities */
    NEXUS_Display_GetDefaultSettings(&displaySettings);
    display = NEXUS_Display_Open(0, &displaySettings);
    window = NEXUS_VideoWindow_Open(display, 0);
    NEXUS_VideoWindow_AddInput(window, NEXUS_VideoDecoder_GetConnector(videoDecoder));

#if ENABLE_PLAYBACK
    playpump = NEXUS_Playpump_Open(0, NULL);
    assert(playpump);
    playback = NEXUS_Playback_Create();
    assert(playback);

    file = NEXUS_FilePlay_OpenPosix(fname, NULL);
    if (!file) {
        fprintf(stderr, "can't open file:%s\n", fname);
        return -1;
    }
#else
    /* Map input band and parser band. This could be eliminated because a straight mapping and TS config is a good default. */
    NEXUS_ParserBand_GetSettings(parserBand, &parserBandSettings);
    parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eInputBand;
    NEXUS_Platform_GetStreamerInputBand(0, &parserBandSettings.sourceTypeSettings.inputBand);
    parserBandSettings.transportType = NEXUS_TransportType_eTs;
    NEXUS_ParserBand_SetSettings(parserBand, &parserBandSettings);
#endif

    /* By default, StcChannel will configure NEXUS_Timebase with the info it has */
    NEXUS_StcChannel_GetDefaultSettings(0, &stcSettings);

#if ENABLE_PLAYBACK
    /* configure stc channel */
    stcSettings.timebase = NEXUS_Timebase_e0;
    stcSettings.mode = NEXUS_StcChannelMode_eAuto;
    stcChannel = NEXUS_StcChannel_Open(0, &stcSettings);

    /* connect playpump and playback */
    NEXUS_Playback_GetSettings(playback, &playbackSettings);
    playbackSettings.stcChannel = stcChannel;
    playbackSettings.playpump = playpump;
    playbackSettings.playpumpSettings.transportType = TRANSPORT_TYPE;
    NEXUS_Playback_SetSettings(playback, &playbackSettings);

    /* Open the audio and video pid channels */
    NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
    playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eVideo;
    playbackPidSettings.pidTypeSettings.video.codec = VIDEO_CODEC; /* must be told codec for correct handling */
    playbackPidSettings.pidTypeSettings.video.index = true;
    playbackPidSettings.pidTypeSettings.video.decoder = videoDecoder;
    videoPidChannel = NEXUS_Playback_OpenPidChannel(playback, VIDEO_PID, &playbackPidSettings);
    NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
    playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eAudio;
    playbackPidSettings.pidTypeSettings.audio.primary = pcmDecoder;
    audioPidChannel = NEXUS_Playback_OpenPidChannel(playback, AUDIO_PID, &playbackPidSettings);
    printf("audioPidChannel %p, videoPidChannel %p\n", (void*)audioPidChannel, (void*)videoPidChannel);

#else
    /* Open the audio and video pid channels */
    videoPidChannel = NEXUS_PidChannel_Open(parserBand, VIDEO_PID, NULL);
    audioPidChannel = NEXUS_PidChannel_Open(parserBand, AUDIO_PID, NULL);

    /* configure stc channel */
    stcSettings.timebase = NEXUS_Timebase_e0;
    stcSettings.mode = NEXUS_StcChannelMode_ePcr; /* live */
    stcSettings.modeSettings.pcr.pidChannel = videoPidChannel; /* PCR on video pid */
    stcChannel = NEXUS_StcChannel_Open(0, &stcSettings);
#endif

    printf("audioPidChannel %p, videoPidChannel %p\n", (void*)audioPidChannel, (void*)videoPidChannel);
    NEXUS_VideoDecoder_GetDefaultStartSettings(&videoProgram);
    videoProgram.codec = VIDEO_CODEC;
    videoProgram.pidChannel = videoPidChannel;
    videoProgram.stcChannel = stcChannel;
    NEXUS_AudioDecoder_GetDefaultStartSettings(&audioProgram);
    audioProgram.codec = AUDIO_CODEC;
    audioProgram.pidChannel = audioPidChannel;
    audioProgram.stcChannel = stcChannel;

    /* connect audio outputs */
    /* Output PCM to DAC, SPDIF, and HDMI by default */
    #if NEXUS_NUM_AUDIO_DACS
    NEXUS_AudioOutput_AddInput(NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]),
                               NEXUS_AudioDecoder_GetConnector(pcmDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
    #endif
    #if NEXUS_NUM_SPDIF_OUTPUTS
    NEXUS_AudioOutput_AddInput(NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]),
                               NEXUS_AudioDecoder_GetConnector(pcmDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
    #endif
    NEXUS_AudioOutput_AddInput(NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]),
                               NEXUS_AudioDecoder_GetConnector(pcmDecoder, NEXUS_AudioDecoderConnectorType_eMultichannel));

    /* bring up display */
    NEXUS_HdmiOutput_GetStatus(platformConfig.outputs.hdmi[0], &hdmiStatus);
    
#if NEXUS_NUM_COMPONENT_OUTPUTS
    if(platformConfig.outputs.component[0]){
        NEXUS_Display_AddOutput(display, NEXUS_ComponentOutput_GetConnector(platformConfig.outputs.component[0]));
    }
#endif 
#if 0 /* NEXUS_NUM_COMPOSITE_OUTPUTS */
    NEXUS_Display_AddOutput(display, NEXUS_CompositeOutput_GetConnector(platformConfig.outputs.composite[0]));
#endif    
    NEXUS_Display_AddOutput(display, NEXUS_HdmiOutput_GetVideoConnector(platformConfig.outputs.hdmi[0]));

#if NEXUS_HAS_HDMI_OUTPUT
    /* Install hotplug callback -- video only for now */
    NEXUS_HdmiOutput_GetSettings(platformConfig.outputs.hdmi[0], &hdmiSettings);
    hdmiSettings.hotplugCallback.callback = hotplug_callback;
    hotPlugCbParams.hdmiOutput = platformConfig.outputs.hdmi[0];
    hotPlugCbParams.display = display;
    hdmiSettings.hotplugCallback.context = &hotPlugCbParams;
    NEXUS_HdmiOutput_SetSettings(platformConfig.outputs.hdmi[0], &hdmiSettings);

    /* initalize HDCP settings, keys, etc. */
    initializeHdmiOutputHdcpSettings() ;

    /* Force a hotplug to switch to preferred format */
    hotplug_callback(&hotPlugCbParams, 0);
#endif
    /* Start Decoders */
    NEXUS_VideoDecoder_Start(videoDecoder, &videoProgram);
    NEXUS_AudioDecoder_Start(pcmDecoder, &audioProgram);
    
    NEXUS_HdmiOutput_GetStatus(platformConfig.outputs.hdmi[0], &hdmiStatus);
    NEXUS_Display_GetSettings(display, &displaySettings);
    displaySettings.format = hdmiStatus.preferredVideoFormat;
    NEXUS_Display_SetSettings(display, &displaySettings);
    
#if ENABLE_PLAYBACK
    /* Start playback */
    NEXUS_Playback_Start(playback, file, NULL);
#endif

    while (!done)
    {
        bool hdmiWasMuted, spdifWasMuted;
        int tmp;

        /* Display Menu */
        printf("Main Menu\n");
        printf(" 0) Exit\n");      
        printf(" 1) Mute/Unmute (currently %s)\n",mute?"MUTED":"NOT MUTED");
        printf(" 2) Start/Stop Audio Decoder (currently %s)\n",started?"STARTED":"STOPED");
        printf(" 3) Switch between PCM/Compressed (currently %s)\n", pcm?"PCM":"COMPRESSED");
        printf(" 4) Disable/enable Pause Bursts (currently %s)\n", pauseBurst?"ENABLED":"DISABLED");
        printf(" 5) Reconfigure the digital outputs to %s\n", pcm?"COMPRESSED":"PCM");
        printf(" 6) Toggle HDMI PCM mode (currently %s)\n", multichannel?"MULTICHANNEL":"STEREO");
        printf(" 7) Toggle PCM Decoder Mute (currently %s)\n", decoderMute?"MUTED":"NOT MUTED");
        printf(" 8) Toggle PCM Decoder Volume Dim (currently %s)\n", decoderDiminished?"DIMINISHED":"NORMAL");
        printf("Enter Selection: \n");
        scanf("%d", &tmp);
        switch ( tmp )
        {
        case 0:
            done = true;
            break;
        case 1: /* mutes and unmutes Spdif/HDMI */
            mute = !mute;
            NEXUS_AudioOutput_GetSettings(NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]),
                &outputSettings);
            outputSettings.muted = mute;
            NEXUS_AudioOutput_SetSettings(NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]),
                &outputSettings);
            
            NEXUS_AudioOutput_GetSettings(NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]),
                &outputSettings);
            outputSettings.muted = mute;
            NEXUS_AudioOutput_SetSettings(NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]),
                &outputSettings);
            break;
        case 2: /* starts and stopps Spdif/HDMI outputs */
            if (started) 
            {
                if (!pcm) 
                {
                    NEXUS_AudioDecoder_Stop(compressedDecoder);
                }                
                NEXUS_AudioDecoder_Stop(pcmDecoder);                
            } 
            else
            {
                if (!pcm)
                {
                    NEXUS_AudioDecoder_Start(compressedDecoder, &audioProgram);
                } 
                NEXUS_AudioDecoder_Start(pcmDecoder, &audioProgram);
            }   
            started = !started;
            break;
        case 3:
            if (started) 
            {
                BDBG_ERR(("Decoders are still started, please stop the decoders before switching types"));
            } 
            else
            {
                if (pcm) 
                {
                    NEXUS_AudioOutput_RemoveInput(NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]),
                        NEXUS_AudioDecoder_GetConnector(pcmDecoder, NEXUS_AudioDecoderConnectorType_eStereo));

                    NEXUS_AudioOutput_RemoveInput(NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]),
                        NEXUS_AudioDecoder_GetConnector(pcmDecoder, NEXUS_AudioDecoderConnectorType_eStereo));

                    NEXUS_AudioOutput_AddInput(NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]),
                        NEXUS_AudioDecoder_GetConnector(compressedDecoder, NEXUS_AudioDecoderConnectorType_eCompressed));

                    NEXUS_AudioOutput_AddInput(NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]),
                        NEXUS_AudioDecoder_GetConnector(compressedDecoder, NEXUS_AudioDecoderConnectorType_eCompressed));                    
                } 
                else
                {
                    NEXUS_AudioOutput_RemoveInput(NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]),
                        NEXUS_AudioDecoder_GetConnector(compressedDecoder, NEXUS_AudioDecoderConnectorType_eCompressed));

                    NEXUS_AudioOutput_RemoveInput(NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]),
                        NEXUS_AudioDecoder_GetConnector(compressedDecoder, NEXUS_AudioDecoderConnectorType_eCompressed));

                    NEXUS_AudioOutput_AddInput(NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]),
                        NEXUS_AudioDecoder_GetConnector(pcmDecoder, NEXUS_AudioDecoderConnectorType_eStereo));

                    NEXUS_AudioOutput_AddInput(NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]),
                        NEXUS_AudioDecoder_GetConnector(pcmDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
                }
            }
            pcm = !pcm;               
            break;
        case 4:
            if (!started)
            {
                NEXUS_HdmiOutput_GetSettings(platformConfig.outputs.hdmi[0], &hdmiSettings);
                NEXUS_SpdifOutput_GetSettings(platformConfig.outputs.spdif[0], &spdifSettings);
                if(pauseBurst)
                {                    
                    hdmiSettings.audioBurstType = NEXUS_SpdifOutputBurstType_eNone;
                    spdifSettings.burstType = NEXUS_SpdifOutputBurstType_eNone;                    
                }
                else
                {
                    hdmiSettings.audioBurstType = NEXUS_SpdifOutputBurstType_ePause;
                    spdifSettings.burstType = NEXUS_SpdifOutputBurstType_ePause;
                }
                NEXUS_HdmiOutput_SetSettings(platformConfig.outputs.hdmi[0], &hdmiSettings);
                NEXUS_SpdifOutput_SetSettings(platformConfig.outputs.spdif[0], &spdifSettings);
                pauseBurst=!pauseBurst;                                                              
            }
            break;
            
        case 5: /* automaticaly switch between pcm and compressed */
            /* change burst config */
            if (pcm) /* going to compressesed */
            {
                NEXUS_HdmiOutput_GetSettings(platformConfig.outputs.hdmi[0], &hdmiSettings); 
                NEXUS_SpdifOutput_GetSettings(platformConfig.outputs.spdif[0], &spdifSettings);
                hdmiSettings.audioBurstType = NEXUS_SpdifOutputBurstType_ePause; 
                spdifSettings.burstType = NEXUS_SpdifOutputBurstType_ePause;
                NEXUS_HdmiOutput_SetSettings(platformConfig.outputs.hdmi[0], &hdmiSettings);
                NEXUS_SpdifOutput_SetSettings(platformConfig.outputs.spdif[0], &spdifSettings);
            }
            else /* going to pcm */
            {
                NEXUS_HdmiOutput_GetSettings(platformConfig.outputs.hdmi[0], &hdmiSettings);
                NEXUS_SpdifOutput_GetSettings(platformConfig.outputs.spdif[0], &spdifSettings);
                hdmiSettings.audioBurstType = NEXUS_SpdifOutputBurstType_eNone;
                spdifSettings.burstType = NEXUS_SpdifOutputBurstType_eNone;
                NEXUS_HdmiOutput_SetSettings(platformConfig.outputs.hdmi[0], &hdmiSettings);
                NEXUS_SpdifOutput_SetSettings(platformConfig.outputs.spdif[0], &spdifSettings);
            }

            /* save mute state and set outputs to muted */
            NEXUS_AudioOutput_GetSettings(NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]),
                &outputSettings);
            hdmiWasMuted = outputSettings.muted;
            outputSettings.muted = true;
            NEXUS_AudioOutput_SetSettings(NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]),
                &outputSettings);

            NEXUS_AudioOutput_GetSettings(NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]),
                &outputSettings);
            spdifWasMuted = outputSettings.muted;
            outputSettings.muted = true;
            NEXUS_AudioOutput_SetSettings(NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]),
                &outputSettings);

            /* change connections */
            if (pcm) /* going to compressesed */
            {
                NEXUS_AudioDecoder_Stop(pcmDecoder);

                if (multichannel)
                {
                    NEXUS_AudioOutput_RemoveInput(NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]),
                        NEXUS_AudioDecoder_GetConnector(pcmDecoder, NEXUS_AudioDecoderConnectorType_eMultichannel));
                }
                else
                {
                    NEXUS_AudioOutput_RemoveInput(NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]),
                        NEXUS_AudioDecoder_GetConnector(pcmDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
                }

                NEXUS_AudioOutput_RemoveInput(NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]),
                    NEXUS_AudioDecoder_GetConnector(pcmDecoder, NEXUS_AudioDecoderConnectorType_eStereo));

                NEXUS_AudioOutput_AddInput(NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]),
                    NEXUS_AudioDecoder_GetConnector(compressedDecoder, NEXUS_AudioDecoderConnectorType_eCompressed));

                NEXUS_AudioOutput_AddInput(NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]),
                    NEXUS_AudioDecoder_GetConnector(compressedDecoder, NEXUS_AudioDecoderConnectorType_eCompressed));
            
                NEXUS_AudioDecoder_Start(pcmDecoder, &audioProgram);
                NEXUS_AudioDecoder_Start(compressedDecoder, &audioProgram);
                
            } 
            else /* going to pcm */
            {
                NEXUS_AudioDecoder_Stop(pcmDecoder);
                NEXUS_AudioDecoder_Stop(compressedDecoder);
              
                NEXUS_AudioOutput_RemoveInput(NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]),
                    NEXUS_AudioDecoder_GetConnector(compressedDecoder, NEXUS_AudioDecoderConnectorType_eCompressed));

                NEXUS_AudioOutput_RemoveInput(NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]),
                    NEXUS_AudioDecoder_GetConnector(compressedDecoder, NEXUS_AudioDecoderConnectorType_eCompressed));

                NEXUS_AudioOutput_AddInput(NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]),
                    NEXUS_AudioDecoder_GetConnector(pcmDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
                
                if (multichannel)
                {
                    NEXUS_AudioOutput_AddInput(NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]),
                        NEXUS_AudioDecoder_GetConnector(pcmDecoder, NEXUS_AudioDecoderConnectorType_eMultichannel));
                }
                else
                {
                    NEXUS_AudioOutput_AddInput(NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]),
                        NEXUS_AudioDecoder_GetConnector(pcmDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
                }
                
                NEXUS_AudioDecoder_Start(pcmDecoder, &audioProgram);
            }

            /* restore previous mute state */
            NEXUS_AudioOutput_GetSettings(NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]),
                &outputSettings);
            outputSettings.muted = hdmiWasMuted;
            NEXUS_AudioOutput_SetSettings(NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]),
                &outputSettings);

            NEXUS_AudioOutput_GetSettings(NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]),
                &outputSettings);
            outputSettings.muted = spdifWasMuted;
            NEXUS_AudioOutput_SetSettings(NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]),
                &outputSettings);

            pcm = !pcm;
            break;
        case 6:
            if (pcm)
            {
                if (started)
                {
                    NEXUS_AudioDecoder_Stop(pcmDecoder);
                }

                if (multichannel)
                {
                    NEXUS_AudioOutput_RemoveInput(NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]),
                        NEXUS_AudioDecoder_GetConnector(pcmDecoder, NEXUS_AudioDecoderConnectorType_eMultichannel));
                    NEXUS_AudioOutput_AddInput(NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]),
                        NEXUS_AudioDecoder_GetConnector(pcmDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
                }
                else
                {
                    NEXUS_AudioOutput_RemoveInput(NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]),
                        NEXUS_AudioDecoder_GetConnector(pcmDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
                    NEXUS_AudioOutput_AddInput(NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]),
                        NEXUS_AudioDecoder_GetConnector(pcmDecoder, NEXUS_AudioDecoderConnectorType_eMultichannel));
                }

                if (started)
                {
                    NEXUS_AudioDecoder_Start(pcmDecoder, &audioProgram);
                }
            }
            multichannel =!multichannel;
            break;
        case 7:
            {
                NEXUS_AudioDecoderSettings decoderSettings;
                NEXUS_AudioDecoder_GetSettings(pcmDecoder, &decoderSettings);
                decoderMute = !decoderMute;
                decoderSettings.muted = decoderMute;
                NEXUS_AudioDecoder_SetSettings(pcmDecoder, &decoderSettings);
            }
        case 8:
            {
                NEXUS_AudioDecoderSettings decoderSettings;
                NEXUS_AudioDecoder_GetSettings(pcmDecoder, &decoderSettings);
                decoderDiminished = !decoderDiminished;
                if ( decoderDiminished )
                {
                    decoderSettings.volumeMatrix[0][0] = NEXUS_AUDIO_VOLUME_LINEAR_NORMAL / 4;
                    decoderSettings.volumeMatrix[1][1] = NEXUS_AUDIO_VOLUME_LINEAR_NORMAL / 4;
                    decoderSettings.volumeMatrix[2][2] = NEXUS_AUDIO_VOLUME_LINEAR_NORMAL / 4;
                    decoderSettings.volumeMatrix[3][3] = NEXUS_AUDIO_VOLUME_LINEAR_NORMAL / 4;
                    decoderSettings.volumeMatrix[4][4] = NEXUS_AUDIO_VOLUME_LINEAR_NORMAL / 4;
                    decoderSettings.volumeMatrix[5][5] = NEXUS_AUDIO_VOLUME_LINEAR_NORMAL / 4;
                    decoderSettings.volumeMatrix[6][6] = NEXUS_AUDIO_VOLUME_LINEAR_NORMAL / 4;
                    decoderSettings.volumeMatrix[7][7] = NEXUS_AUDIO_VOLUME_LINEAR_NORMAL / 4;
                }
                else
                {
                    decoderSettings.volumeMatrix[0][0] = NEXUS_AUDIO_VOLUME_LINEAR_NORMAL;
                    decoderSettings.volumeMatrix[1][1] = NEXUS_AUDIO_VOLUME_LINEAR_NORMAL;
                    decoderSettings.volumeMatrix[2][2] = NEXUS_AUDIO_VOLUME_LINEAR_NORMAL;
                    decoderSettings.volumeMatrix[3][3] = NEXUS_AUDIO_VOLUME_LINEAR_NORMAL;
                    decoderSettings.volumeMatrix[4][4] = NEXUS_AUDIO_VOLUME_LINEAR_NORMAL;
                    decoderSettings.volumeMatrix[5][5] = NEXUS_AUDIO_VOLUME_LINEAR_NORMAL;
                    decoderSettings.volumeMatrix[6][6] = NEXUS_AUDIO_VOLUME_LINEAR_NORMAL;
                    decoderSettings.volumeMatrix[7][7] = NEXUS_AUDIO_VOLUME_LINEAR_NORMAL;
                }
                NEXUS_AudioDecoder_SetSettings(pcmDecoder, &decoderSettings);
            }
        default:
            break;
        }
    }
    
    NEXUS_VideoDecoder_Close(videoDecoder);
    NEXUS_VideoWindow_Close(window);

    /* stop/remove HDMI callbacks associated with display, 
    so those callbacks do not access display once it is removed */
#if NEXUS_HAS_HDMI_OUTPUT
    NEXUS_StopCallbacks(platformConfig.outputs.hdmi[0]);
#endif
    NEXUS_Display_Close(display);

    NEXUS_AudioDecoder_Close(pcmDecoder);
    NEXUS_AudioDecoder_Close(compressedDecoder);
    NEXUS_PidChannel_Close(videoPidChannel);
    NEXUS_PidChannel_Close(audioPidChannel);
    NEXUS_Platform_Uninit();

    return 0;
}

#if NEXUS_HAS_HDMI_OUTPUT

#if USE_PRODUCTION_KEYS

/*****************************/
/* INSERT PRODUCTION KeySet HERE */
/*****************************/

#else


/**************************************/
/* HDCP Specification Test Key Set    */
/*                                    */
/* NOTE: the default declared Test    */
/* KeySet below is from the HDCP Spec */
/* and it *IS NOT* compatible with    */
/* production devices                 */
/**************************************/


static NEXUS_HdmiOutputHdcpKsv hdcpTxAksv =
{    {0x14, 0xF7, 0x61, 0x03, 0xB7} };

static NEXUS_HdmiOutputHdcpKey encryptedTxKeySet[NEXUS_HDMI_OUTPUT_HDCP_NUM_KEYS] =
{
    { 0, 0, 0, 0, 0, 0, 0, 0, 0x691e138f, 0x58a44d00},
    { 0, 0, 0, 0, 0, 0, 0, 0, 0x0950e658, 0x35821f00},
    { 0, 0, 0, 0, 0, 0, 0, 0, 0x0d98b9ab, 0x476a8a00},
    { 0, 0, 0, 0, 0, 0, 0, 0, 0xcac5cb52, 0x1b18f300},
    { 0, 0, 0, 0, 0, 0, 0, 0, 0xb4d89668, 0x7f14fb00},
    { 0, 0, 0, 0, 0, 0, 0, 0, 0x818f4878, 0xc98be000},
    { 0, 0, 0, 0, 0, 0, 0, 0, 0x412c11c8, 0x64d0a000},
    { 0, 0, 0, 0, 0, 0, 0, 0, 0x44202428, 0x5a9db300},
    { 0, 0, 0, 0, 0, 0, 0, 0, 0x6b56adbd, 0xb228b900},
    { 0, 0, 0, 0, 0, 0, 0, 0, 0xf6e46c4a, 0x7ba49100},
    { 0, 0, 0, 0, 0, 0, 0, 0, 0x589d5e20, 0xf8005600},
    { 0, 0, 0, 0, 0, 0, 0, 0, 0xa03fee06, 0xb77f8c00},
    { 0, 0, 0, 0, 0, 0, 0, 0, 0x28bc7c9d, 0x8c2dc000},
    { 0, 0, 0, 0, 0, 0, 0, 0, 0x059f4be5, 0x61125600},
    { 0, 0, 0, 0, 0, 0, 0, 0, 0xcbc1ca8c, 0xdef07400},
    { 0, 0, 0, 0, 0, 0, 0, 0, 0x6adbfc0e, 0xf6b83b00},
    { 0, 0, 0, 0, 0, 0, 0, 0, 0xd72fb216, 0xbb2ba000},
    { 0, 0, 0, 0, 0, 0, 0, 0, 0x98547846, 0x8e2f4800},
    { 0, 0, 0, 0, 0, 0, 0, 0, 0x38472762, 0x25ae6600},
    { 0, 0, 0, 0, 0, 0, 0, 0, 0xf2dd23a3, 0x52493d00},
    { 0, 0, 0, 0, 0, 0, 0, 0, 0x543a7b76, 0x31d2e200},
    { 0, 0, 0, 0, 0, 0, 0, 0, 0x2561e6ed, 0x1a584d00},
    { 0, 0, 0, 0, 0, 0, 0, 0, 0xf7227bbf, 0x82603200},
    { 0, 0, 0, 0, 0, 0, 0, 0, 0x6bce3035, 0x461bf600},
    { 0, 0, 0, 0, 0, 0, 0, 0, 0x6b97d7f0, 0x09043600},
    { 0, 0, 0, 0, 0, 0, 0, 0, 0xf9498d61, 0x05e1a100},
    { 0, 0, 0, 0, 0, 0, 0, 0, 0x063405d1, 0x9d8ec900},
    { 0, 0, 0, 0, 0, 0, 0, 0, 0x90614294, 0x67c32000},
    { 0, 0, 0, 0, 0, 0, 0, 0, 0xc34facce, 0x51449600},
    { 0, 0, 0, 0, 0, 0, 0, 0, 0x8a8ce104, 0x45903e00},
    { 0, 0, 0, 0, 0, 0, 0, 0, 0xfc2d9c57, 0x10002900},
    { 0, 0, 0, 0, 0, 0, 0, 0, 0x80b1e569, 0x3b94d700},
    { 0, 0, 0, 0, 0, 0, 0, 0, 0x437bdd5b, 0xeac75400},
    { 0, 0, 0, 0, 0, 0, 0, 0, 0xba90c787, 0x58fb7400},
    { 0, 0, 0, 0, 0, 0, 0, 0, 0xe01d4e36, 0xfa5c9300},
    { 0, 0, 0, 0, 0, 0, 0, 0, 0xae119a15, 0x5e070300},
    { 0, 0, 0, 0, 0, 0, 0, 0, 0x01fb788a, 0x40d30500},
    { 0, 0, 0, 0, 0, 0, 0, 0, 0xb34da0d7, 0xa5590000},
    { 0, 0, 0, 0, 0, 0, 0, 0, 0x409e2c4a, 0x633b3700},
    { 0, 0, 0, 0, 0, 0, 0, 0, 0x412056b4, 0xbb732500}
} ;

 #endif

/*
from HDCP Spec:
Table 51 gives the format of the HDCP SRM. All values are stored in big endian format.

Specify KSVs here in big endian;
*/
#define NUM_REVOKED_KSVS 3
static uint8_t NumRevokedKsvs = NUM_REVOKED_KSVS ;
static const NEXUS_HdmiOutputHdcpKsv RevokedKsvs[NUM_REVOKED_KSVS] =
{
    /* MSB ... LSB */
    {{0xa5, 0x1f, 0xb0, 0xc3, 0x72}},
    {{0x65, 0xbf, 0x04, 0x8a, 0x7c}},
    {{0x65, 0x65, 0x1e, 0xd5, 0x64}}
} ;

static void hotplug_callback(void *pParam, int iParam)
{
    NEXUS_HdmiOutputStatus status;
    NEXUS_HdmiOutputHandle hdmi;
    NEXUS_DisplayHandle display;
    NEXUS_DisplaySettings displaySettings;
    NEXUS_HdmiOutputSettings hdmiSettings;
    hotplugCallbackParameters *hotPlugCbParams ;

    hotPlugCbParams = (hotplugCallbackParameters *) pParam ;
    hdmi = hotPlugCbParams->hdmiOutput ;
    display = hotPlugCbParams->display ;

    NEXUS_HdmiOutput_GetStatus(hdmi, &status);
    /* the app can choose to switch to the preferred format, but it's not required. */
    if ( !status.connected )
    {
        BDBG_WRN(("No RxDevice Connected")) ;
        return ;
    }

    NEXUS_Display_GetSettings(display, &displaySettings);
    if ( !status.videoFormatSupported[displaySettings.format] )
    {
        BDBG_ERR(("Current format not supported by attached monitor. Switching to preferred format %d",
            status.preferredVideoFormat)) ;
        displaySettings.format = status.preferredVideoFormat;
    }
    NEXUS_Display_SetSettings(display, &displaySettings);

    /* force HDMI updates after a hotplug */
    NEXUS_HdmiOutput_GetSettings(hdmi, &hdmiSettings) ;
    NEXUS_HdmiOutput_SetSettings(hdmi, &hdmiSettings) ;

    /* restart HDCP if it was previously enabled */
    if (hdmiHdcpEnabled)
    {
        NEXUS_HdmiOutput_StartHdcpAuthentication(platformConfig.outputs.hdmi[0]);
    }
}


static void hdmiOutputHdcpStateChanged(void *pContext, int param)
{

    bool success = false ;
    NEXUS_HdmiOutputHandle handle = pContext;
    NEXUS_HdmiOutputHdcpStatus hdcpStatus;

    BSTD_UNUSED(param) ;

    NEXUS_HdmiOutput_GetHdcpStatus(handle, &hdcpStatus);
    switch (hdcpStatus.hdcpError)
    {
    case NEXUS_HdmiOutputHdcpError_eSuccess :
        BDBG_WRN(("HDCP Authentication Successful\n"));
        success = true ;
        hdmiHdcpEnabled = true ;
        break ;

    case NEXUS_HdmiOutputHdcpError_eRxBksvError :
        BDBG_ERR(("HDCP Rx BKsv Error")) ;
        break ;

    case NEXUS_HdmiOutputHdcpError_eRxBksvRevoked :
        BDBG_ERR(("HDCP Rx BKsv/Keyset Revoked")) ;
        break ;

    case NEXUS_HdmiOutputHdcpError_eRxBksvI2cReadError :
    case NEXUS_HdmiOutputHdcpError_eTxAksvI2cWriteError :
        BDBG_ERR(("HDCP I2C Read Error")) ;
        break ;

    case NEXUS_HdmiOutputHdcpError_eTxAksvError :
        BDBG_ERR(("HDCP Tx Aksv Error")) ;
        break ;

    case NEXUS_HdmiOutputHdcpError_eReceiverAuthenticationError :
        BDBG_ERR(("HDCP Receiver Authentication Failure")) ;
        break ;

    case NEXUS_HdmiOutputHdcpError_eRepeaterAuthenticationError :
    case NEXUS_HdmiOutputHdcpError_eRepeaterLinkFailure :    /* Repeater Error; unused */
        BDBG_ERR(("HDCP Repeater Authentication Failure")) ;
        break ;

    case NEXUS_HdmiOutputHdcpError_eRxDevicesExceeded :
        BDBG_ERR(("HDCP Repeater MAX Downstram Devices Exceeded")) ;
        break ;

    case NEXUS_HdmiOutputHdcpError_eRepeaterDepthExceeded :
        BDBG_ERR(("HDCP Repeater MAX Downstram Levels Exceeded")) ;
        break ;

    case NEXUS_HdmiOutputHdcpError_eRepeaterFifoNotReady :
        BDBG_ERR(("Timeout waiting for Repeater")) ;
        break ;

    case NEXUS_HdmiOutputHdcpError_eRepeaterDeviceCount0 : /* unused */
        break ;

    case NEXUS_HdmiOutputHdcpError_eLinkRiFailure :
        BDBG_ERR(("HDCP Ri Integrity Check Failure")) ;
        break ;

    case NEXUS_HdmiOutputHdcpError_eLinkPjFailure :
        BDBG_ERR(("HDCP Pj Integrity Check Failure")) ;
        break ;

    case NEXUS_HdmiOutputHdcpError_eFifoUnderflow :
    case NEXUS_HdmiOutputHdcpError_eFifoOverflow :
        BDBG_ERR(("Video configuration issue")) ;
        break ;

    case NEXUS_HdmiOutputHdcpError_eMultipleAnRequest : /* Should not reach here; but flag if occurs */
        BDBG_WRN(("Multiple Authentication Request... ")) ;

    default :
        BDBG_WRN(("Unknown HDCP Authentication Error %d", hdcpStatus.hdcpError)) ;
    }

    if (!success)
    {
        fprintf(stderr, "\nHDCP Authentication Failed.  Current State %d\n", hdcpStatus.hdcpState);

        /* always retry */
        NEXUS_HdmiOutput_StartHdcpAuthentication(platformConfig.outputs.hdmi[0]);
    }
}


static void initializeHdmiOutputHdcpSettings(void)
{
    NEXUS_HdmiOutputHdcpSettings hdmiOutputHdcpSettings;

    NEXUS_HdmiOutput_GetHdcpSettings(platformConfig.outputs.hdmi[0], &hdmiOutputHdcpSettings);

        /* copy the encrypted key set and its Aksv here  */
        BKNI_Memcpy(hdmiOutputHdcpSettings.encryptedKeySet, encryptedTxKeySet,
            NEXUS_HDMI_OUTPUT_HDCP_NUM_KEYS * sizeof(NEXUS_HdmiOutputHdcpKey)) ;
        BKNI_Memcpy(&hdmiOutputHdcpSettings.aksv, &hdcpTxAksv,
            NEXUS_HDMI_OUTPUT_HDCP_KSV_LENGTH) ;

        /* install HDCP success  callback */
        hdmiOutputHdcpSettings.successCallback.callback = hdmiOutputHdcpStateChanged ;
        hdmiOutputHdcpSettings.successCallback.context = platformConfig.outputs.hdmi[0];

        /* install HDCP failure callback */
        hdmiOutputHdcpSettings.failureCallback.callback = hdmiOutputHdcpStateChanged ;
        hdmiOutputHdcpSettings.failureCallback.context = platformConfig.outputs.hdmi[0];

    NEXUS_HdmiOutput_SetHdcpSettings(platformConfig.outputs.hdmi[0], &hdmiOutputHdcpSettings);

    /* install list of revoked KSVs from SRMs (System Renewability Message) if available */
    NEXUS_HdmiOutput_SetHdcpRevokedKsvs(platformConfig.outputs.hdmi[0],
        RevokedKsvs, NumRevokedKsvs) ;

 }
#endif

#else
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bstd.h"
int main(int argc, char **argv)
{
    BSTD_UNUSED(argc) ;
    fprintf(stderr, "%s not supported on this platform\n", argv[0]);
    return 0;
}
#endif
