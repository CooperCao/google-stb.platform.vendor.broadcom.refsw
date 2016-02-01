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
/* Nexus example app: playback and decode */

#include "nexus_platform.h"
#include <stdio.h>

#if NEXUS_HAS_PLAYBACK && NEXUS_HAS_STREAM_MUX
#include "nexus_video_decoder.h"
#include "nexus_stc_channel.h"
#include "nexus_display.h"
#include "nexus_video_window.h"
#include "nexus_video_input.h"
#include "nexus_video_adj.h"
#include "nexus_spdif_output.h"
#include "nexus_component_output.h"
#if NEXUS_HAS_HDMI_OUTPUT
#include "nexus_hdmi_output.h"
#endif
#include "nexus_audio_mixer.h"
#if NEXUS_HAS_PLAYBACK && NEXUS_HAS_STREAM_MUX
#include "nexus_playback.h"
#include "nexus_record.h"
#include "nexus_file.h"
#include "nexus_video_encoder.h"
#include "nexus_audio_encoder.h"
#include "nexus_stream_mux.h"
#include "nexus_recpump.h"
#include "nexus_record.h"
#endif
#if NEXUS_HAS_SYNC_CHANNEL
#include "nexus_sync_channel.h"
#endif

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "bstd.h"
#include "bkni.h"
#include "bdbg.h"

#if NEXUS_HAS_SYNC_CHANNEL
#define BTST_ENABLE_AV_SYNC 1
#else
#define BTST_ENABLE_AV_SYNC 0
#endif

/* NRT a+v transcode STC av_window enable test */
#define BTST_ENABLE_NRT_STC_AV_WINDOW 1

/* TODO: set to 0 to enable the debug monitor for NRT mode source decode; set to 1 to disable it. */
#define BTST_DISABLE_NRT_DEBUG_DISPLAY 1

#define BTST_HAS_VIDEO_ENCODE_TEST 1
#define NEXUS_HAS_AUDIO_MUX_OUTPUT_TEST 1

#define BTST_SINGLE_AUDIO_PID_PASSTHRU_6X 0

/* the following define the input file and its characteristics -- these will vary by input file */
#define TRANSPORT_TYPE NEXUS_TransportType_eTs
#if BTST_SINGLE_AUDIO_PID_PASSTHRU_6X /* AC3 */   
  #define FILE_NAME "videos/avatar_AVC_15M.ts" /*"videos/WildChina_Short.ts"*/
  #define VIDEO_CODEC NEXUS_VideoCodec_eH264
  #define AUDIO_CODEC NEXUS_AudioCodec_eAc3
  #define VIDEO_PID 0x101 /*0x201*/
  #define AUDIO_PID 0x104
#else 
  #define FILE_NAME "videos/h264and6xaudio.ts"
  #define VIDEO_CODEC NEXUS_VideoCodec_eMpeg2
  #define VIDEO_PID 0x31
  #define AUDIO_PID 0x34 /* 1st audio PID */
#if (NEXUS_HAS_PLAYBACK && NEXUS_HAS_STREAM_MUX)
static NEXUS_AudioCodec audioCodecList[6] = {
  NEXUS_AudioCodec_eAc3,
  NEXUS_AudioCodec_eMpeg,
  NEXUS_AudioCodec_eMpeg,
  NEXUS_AudioCodec_eAc3,
  NEXUS_AudioCodec_eMpeg,
  NEXUS_AudioCodec_eMpeg
};
#endif
#endif

BDBG_MODULE(transcode_playback_to_ts_6xaudio);


/* ============= utilities ==============*/

/* Generate a CRC for the specified data/length */
/* Initialize crc to 0 for new calculation.  Use an old result for subsequent calls. */
static uint32_t CRC32_mpeg(uint32_t crc, uint8_t *data, int length)
{
	int j;
	crc = ~crc;
	while (length--)
	{
		for (j=0; j<8; j++) 
			crc = (crc<<1) ^ ((((*data >> (7-j)) ^ (crc >> 31)) & 1) ? 0x04c11db7 : 0);
		data++;
	}
	return crc;
}

static void play_endOfStreamCallback(void *context, int param)
{
	BSTD_UNUSED(context);
	BSTD_UNUSED(param);

	printf("endOfStream\n");
	return;
}

static void transcoderFinishCallback(void *context, int param)
{
    BKNI_EventHandle finishEvent = (BKNI_EventHandle)context;

    BSTD_UNUSED(param);
    BDBG_WRN(("Finish callback invoked, now stop the stream mux."));
    BKNI_SetEvent(finishEvent);
}

#define TEST_ITERATIONS 3
struct cmdSettings {
    NEXUS_VideoFormat displayFormat;
    NEXUS_VideoFrameRate encoderFrameRate;
    unsigned encoderBitrate;
    unsigned encoderGopStructureFramesP;
    unsigned encoderGopStructureFramesB;
    NEXUS_VideoCodec encoderVideoCodec;
    NEXUS_VideoCodecProfile encoderProfile;
    NEXUS_VideoCodecLevel encoderLevel;
} cmdSettings[TEST_ITERATIONS] = {
    {NEXUS_VideoFormat_e480p, NEXUS_VideoFrameRate_e29_97, 2*1000*1000, 23, 0, NEXUS_VideoCodec_eH264, NEXUS_VideoCodecProfile_eBaseline, NEXUS_VideoCodecLevel_e31},
    {NEXUS_VideoFormat_e720p, NEXUS_VideoFrameRate_e29_97, 6*1000*1000, 23, 0, NEXUS_VideoCodec_eMpeg2, NEXUS_VideoCodecProfile_eMain, NEXUS_VideoCodecLevel_eHigh},
    {NEXUS_VideoFormat_e480p, NEXUS_VideoFrameRate_e59_94, 2*1000*1000, 29, 0, NEXUS_VideoCodec_eH264, NEXUS_VideoCodecProfile_eBaseline, NEXUS_VideoCodecLevel_e31}
};

void print_usage(void)
{
    printf("Usage:\n"
        "  -h|--help   - this usage information\n"
        "  -nrt        - enable NRT (or AFAP) mode transcoding\n"
        "  -1x+5p NUM  - enable 1xaudio transcode (with audio channel NUMber 0-5) and 5xaudio passthru\n"
        "  -audios NUM - set NUMber of audio programs\n");
    printf(
        "  -file PATH  - set source file PATH\n"
        "  -apid NUM   - source first audio PID\n"
        "  -vpid NUM   - source video PID\n"
        "  -acodecs CODEC0 CODEC1 ...CODEC%d - source %d codecs list of audio programs; CODECn = {ac3|mpeg|aac}\n"
        "  -vcodec {avc|mpeg} - source video codec\n"
        "  -loop       - enable source playback looping mode\n"
        "  -tts_in binary|mod300  - Input TTS transport packets prepended with 4-byte timestamps in binary or mod300 mode.\n"
        "  -mixer      - enable DSP mixer for seamless audio\n", NEXUS_SYNC_CHANNEL_NUM_AUDIO_INPUTS-1, NEXUS_SYNC_CHANNEL_NUM_AUDIO_INPUTS);
}

int main(int argc, const char *argv[])  
{
    NEXUS_PlatformSettings platformSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_StcChannelHandle stcChannel;
    NEXUS_StcChannelSettings stcSettings;
    NEXUS_PidChannelHandle videoPidChannel;
    NEXUS_DisplayHandle display;
    NEXUS_DisplaySettings displaySettings;
    NEXUS_VideoWindowHandle window;
    NEXUS_VideoWindowMadSettings windowMadSettings;
    NEXUS_VideoDecoderHandle videoDecoder;
    NEXUS_VideoDecoderStartSettings videoProgram;
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_HdmiOutputStatus hdmiStatus;
    NEXUS_Error rc;
#endif
#if NEXUS_HAS_AUDIO_MUX_OUTPUT_TEST    
    NEXUS_StcChannelHandle stcAudioChannel;
    NEXUS_AudioDecoderHandle audioDecoder[NEXUS_SYNC_CHANNEL_NUM_AUDIO_INPUTS];
    NEXUS_AudioDecoderStartSettings audioProgram[NEXUS_SYNC_CHANNEL_NUM_AUDIO_INPUTS];
    NEXUS_PidChannelHandle audioPidChannel[NEXUS_SYNC_CHANNEL_NUM_AUDIO_INPUTS];
    NEXUS_AudioMuxOutputHandle audioMuxOutput[NEXUS_SYNC_CHANNEL_NUM_AUDIO_INPUTS];
    NEXUS_AudioMuxOutputDelayStatus audioDelayStatus;
    NEXUS_AudioMuxOutputStartSettings audioMuxStartSettings;
    NEXUS_PlaypumpHandle playpumpTranscodeAudio[NEXUS_SYNC_CHANNEL_NUM_AUDIO_INPUTS];
    NEXUS_PidChannelHandle pidChannelTranscodeAudio[NEXUS_SYNC_CHANNEL_NUM_AUDIO_INPUTS];
    NEXUS_AudioEncoderSettings encoderSettings;
    NEXUS_AudioEncoderHandle audioEncoder;
    NEXUS_AudioCodec audioCodec[NEXUS_SYNC_CHANNEL_NUM_AUDIO_INPUTS];
    NEXUS_AudioMixerHandle dspMixer[NEXUS_SYNC_CHANNEL_NUM_AUDIO_INPUTS], audioMixer;
    NEXUS_AudioCodec srcAudCodec;
#endif    
    NEXUS_FilePlayHandle file;
    NEXUS_PlaypumpHandle playpump;
    NEXUS_PlaybackHandle playback;
    NEXUS_PlaybackSettings playbackSettings;
    NEXUS_PlaybackPidChannelSettings playbackPidSettings;
    NEXUS_DisplayStgSettings stgSettings;
    NEXUS_DisplayHandle displayTranscode;
    NEXUS_VideoWindowHandle windowTranscode;
    NEXUS_VideoEncoderCapabilities videoEncoderCap;
#if BTST_HAS_VIDEO_ENCODE_TEST
    NEXUS_VideoEncoderHandle videoEncoder;
    NEXUS_VideoEncoderSettings videoEncoderConfig;
    NEXUS_VideoEncoderStartSettings videoEncoderStartConfig;
    NEXUS_VideoEncoderDelayRange videoDelay;
    NEXUS_PlaypumpHandle playpumpTranscodeVideo;
    NEXUS_PidChannelHandle pidChannelTranscodeVideo;
    NEXUS_RecordPidChannelSettings recordPidSettings;
#endif    
    NEXUS_StreamMuxHandle streamMux;
    NEXUS_StreamMuxCreateSettings muxCreateSettings;
    BKNI_EventHandle finishEvent;
    NEXUS_StreamMuxStartSettings muxConfig;
    NEXUS_StreamMuxOutput muxOutput;
    NEXUS_PlaypumpOpenSettings playpumpConfig;
    NEXUS_PlaypumpHandle playpumpTranscodePcr;
    NEXUS_FileRecordHandle fileTranscode;
    NEXUS_StcChannelHandle stcChannelTranscode;

#if BTST_ENABLE_AV_SYNC
    NEXUS_SyncChannelSettings syncChannelSettings;
    NEXUS_SyncChannelHandle syncChannel;
#if BTST_ENABLE_NRT_STC_AV_WINDOW
	NEXUS_StcChannelPairSettings stcAudioVideoPair;
#endif
#endif
    NEXUS_RecpumpHandle recpump;
    NEXUS_RecpumpOpenSettings recpumpOpenSettings;
    NEXUS_RecordHandle record;
    NEXUS_RecordSettings recordSettings;
    NEXUS_PidChannelHandle pidChannelTranscodePcr;
    NEXUS_PidChannelHandle pidChannelTranscodePat;
    NEXUS_PidChannelHandle pidChannelTranscodePmt;

    void *pat;
    void *pmt;
    const char *fname = FILE_NAME;
    int i = 0, j=0;
    int iteration = 1;
    char key;
    int numAudios=NEXUS_SYNC_CHANNEL_NUM_AUDIO_INPUTS;
    bool bNonRealTime=false;
    bool bLoop=false;
    bool b1x5p=false;
    bool bDspMixer = false; /* default without DSP mixer for seamless audio */
    int audioXcodeChanNum=0;
    unsigned aPid = AUDIO_PID;
    unsigned vPid = VIDEO_PID;
    NEXUS_VideoCodec vCodec = VIDEO_CODEC;
    NEXUS_TransportTimestampType ttsInputType  = NEXUS_TransportTimestampType_eNone;

    for(j=0; j<argc; j++) {
        if(!strcmp(argv[j], "-h") || !strcmp(argv[j], "--help")) {
            print_usage();
            return 0;
        }
        if(!strcmp(argv[j], "-mixer")) {
            bDspMixer = true;
            fprintf(stderr, "Enabled DSP mixer for seamless audio...\n");
        }
        if(!strcmp(argv[j], "-nrt")) {
            bNonRealTime = true;
            fprintf(stderr, "Enabled NRT mode...\n");
        }
        if(!strcmp(argv[j], "-loop")) {
            bLoop = true;
            fprintf(stderr, "Enable source playback looping...\n");
        }
        if(!strcmp(argv[j], "-audios")) {
            numAudios = atoi(argv[++j]);
            if (NEXUS_SYNC_CHANNEL_NUM_AUDIO_INPUTS < numAudios) {
                numAudios = NEXUS_SYNC_CHANNEL_NUM_AUDIO_INPUTS;
            }
            fprintf(stderr, "%d audio channels...\n", numAudios);
        }
        if(!strcmp(argv[j], "-file")) {
            fname = argv[++j];
            fprintf(stderr, "Source file = %s.\n", fname);
        }
        if(!strcmp(argv[j], "-apid")) {
            aPid = strtoul(argv[++j], NULL, 0);
            fprintf(stderr, "Fist audio PID = %#x.\n", aPid);
        }
        if(!strcmp(argv[j], "-vpid")) {
            vPid = strtoul(argv[++j], NULL, 0);
            fprintf(stderr, "The  video PID = %#x.\n", vPid);
        }
#if !BTST_SINGLE_AUDIO_PID_PASSTHRU_6X
        if(!strcmp(argv[j], "-acodecs")) {
            for(i=0; i<numAudios; i++) {
                ++j;
                audioCodecList[i] = (!strcmp(argv[j], "ac3"))? NEXUS_AudioCodec_eAc3 :
                    ((!strcmp(argv[j], "aac"))? NEXUS_AudioCodec_eAac :
                    NEXUS_AudioCodec_eMpeg);
                fprintf(stderr, "audio codec[%d] = %d.\n", i, audioCodecList[i]);
            }
            i=0;
        }
#endif
        if(!strcmp(argv[j], "-vcodec")) {
            vCodec = !strcmp(argv[++j], "avc")? NEXUS_VideoCodec_eH264:
                    NEXUS_VideoCodec_eMpeg2;
            fprintf(stderr, "video codec = %d.\n", vCodec);
        }
        if(!strcmp(argv[j], "-1x+5p")) {
            b1x5p = true;
            audioXcodeChanNum = atoi(argv[++j]);
            if (NEXUS_SYNC_CHANNEL_NUM_AUDIO_INPUTS <= audioXcodeChanNum) {
                audioXcodeChanNum = 0;
            }
            fprintf(stderr, "Enable 1xTranscode (ch%d) + 5xPassthru...\n", audioXcodeChanNum);
        }
        if(!strcmp("-tts_in",argv[j])) {
            j++;
            if(!strcmp("binary",argv[j])) {
                ttsInputType = NEXUS_TransportTimestampType_e32_Binary;
            }
            else {
                ttsInputType = NEXUS_TransportTimestampType_e32_Mod300;
            }
            fprintf(stderr, "Input TS transport packets prepended with 4-byte timestamps mode %d.\n", ttsInputType);
        }
    }
    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    platformSettings.audioModuleSettings.maxAudioDspTasks = bDspMixer?12:6;
    platformSettings.audioModuleSettings.numCompressedBuffers = 6;
    NEXUS_Platform_Init(&platformSettings);
    NEXUS_Platform_GetConfiguration(&platformConfig);

again:
    BDBG_WRN(("Setting up transcode pipeline: format %d, fr %d, bitrate %u, gopP %d, codec %d",
                cmdSettings[i].displayFormat,
                cmdSettings[i].encoderFrameRate,
                cmdSettings[i].encoderBitrate,
                cmdSettings[i].encoderGopStructureFramesP,
                cmdSettings[i].encoderVideoCodec));

    playpump = NEXUS_Playpump_Open(0, NULL);
    assert(playpump);
    playback = NEXUS_Playback_Create();
    assert(playback);

    file = NEXUS_FilePlay_OpenPosix(fname, NULL);
    if (!file) {
        fprintf(stderr, "can't open file:%s\n", fname);
        return -1;
    }

#if BTST_ENABLE_AV_SYNC
    /* create a sync channel */
    NEXUS_SyncChannel_GetDefaultSettings(&syncChannelSettings);
    syncChannel = NEXUS_SyncChannel_Create(&syncChannelSettings);
#endif

    NEXUS_StcChannel_GetDefaultSettings(0, &stcSettings);
    stcSettings.timebase = NEXUS_Timebase_e0;
    stcSettings.mode = NEXUS_StcChannelMode_eAuto;
    stcChannel = NEXUS_StcChannel_Open(0, &stcSettings);

    /* NRT mode uses separate STCs for audio and video decoders; */
    stcAudioChannel = (bNonRealTime)? NEXUS_StcChannel_Open(2, &stcSettings) : stcChannel;

    /* encoders/mux require different STC broadcast mode from decoder */
    NEXUS_StcChannel_GetDefaultSettings(1, &stcSettings);
    stcSettings.timebase = NEXUS_Timebase_e0;/* should be the same timebase for end-to-end locking */
    stcSettings.mode = NEXUS_StcChannelMode_eAuto;
    stcSettings.pcrBits = NEXUS_StcChannel_PcrBits_eFull42;/* ViCE2 requires 42-bit STC broadcast */
    stcChannelTranscode = NEXUS_StcChannel_Open(1, &stcSettings);

    NEXUS_Playback_GetSettings(playback, &playbackSettings);
    playbackSettings.playpump = playpump;
    /* set a stream format, it could be any audio video transport type or file format, i.e NEXUS_TransportType_eMp4, NEXUS_TransportType_eAvi ... */
    playbackSettings.playpumpSettings.transportType = TRANSPORT_TYPE;
    playbackSettings.stcChannel = (bNonRealTime)? stcChannelTranscode:stcChannel;
    /* NRT mode file transcode doesn not need loop */
    playbackSettings.endOfStreamAction = (!bLoop)? NEXUS_PlaybackLoopMode_ePause : NEXUS_PlaybackLoopMode_eLoop; /* when play hits the end, wait for record */
    playbackSettings.endOfStreamCallback.callback = play_endOfStreamCallback;
    playbackSettings.playpumpSettings.timestamp.type = ttsInputType;
    NEXUS_Playback_SetSettings(playback, &playbackSettings);

    /* Bring up video display and outputs */
    NEXUS_Display_GetDefaultSettings(&displaySettings);
    displaySettings.format = NEXUS_VideoFormat_e480p;
    display = NEXUS_Display_Open(0, &displaySettings);
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

    /* enable deinterlacer to improve quality */
    NEXUS_VideoWindow_GetMadSettings(window, &windowMadSettings);
    windowMadSettings.deinterlace = true;
    windowMadSettings.enable22Pulldown = true;
    windowMadSettings.enable32Pulldown = true;
    NEXUS_VideoWindow_SetMadSettings(window, &windowMadSettings);

#if NEXUS_HAS_AUDIO_MUX_OUTPUT_TEST    
    /* Open the hw mixer -> dummy output */
    audioMixer = NEXUS_AudioMixer_Open(NULL);
    if ( !audioMixer )
    {
        fprintf(stderr, "Unable to open HW mixer\n");
        return -1;
    }
    NEXUS_AudioOutput_AddInput(
        NEXUS_AudioDummyOutput_GetConnector(platformConfig.outputs.audioDummy[0]),
        NEXUS_AudioMixer_GetConnector(audioMixer));

    /* Open the audio decoder */
    for(j=0; j<numAudios; j++) {
        NEXUS_AudioMixerSettings audioMixerSettings;

        audioDecoder[j] = NEXUS_AudioDecoder_Open(j, NULL);

        /* Open the audio and pcr pid channel */
        NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
        playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eAudio;
        /* 6 passthru audio */
        playbackPidSettings.pidTypeSettings.audio.primary = audioDecoder[j]; /* must be told codec for correct handling */
#if BTST_SINGLE_AUDIO_PID_PASSTHRU_6X /* AC3 */   
        srcAudCodec = AUDIO_CODEC;
        if(j==0) {
        	audioPidChannel[0] = NEXUS_Playback_OpenPidChannel(playback, aPid, &playbackPidSettings);
        }
        else {/* share the same audio PID for now for bringup; TODO: support 6 different audio programs. */
            audioPidChannel[j] = audioPidChannel[0];
        }
#else
        srcAudCodec = audioCodecList[j];
        audioPidChannel[j] = NEXUS_Playback_OpenPidChannel(playback, aPid+j, &playbackPidSettings);
#endif
        /* Set up decoder Start structures now. We need to know the audio codec to properly set up the audio outputs. */
        NEXUS_AudioDecoder_GetDefaultStartSettings(&audioProgram[j]);
        audioProgram[j].codec = srcAudCodec;
        audioProgram[j].pidChannel = audioPidChannel[j];
        audioProgram[j].stcChannel = bNonRealTime? stcAudioChannel : stcChannel;
        audioProgram[j].nonRealTime= bNonRealTime;

		/* Open dsp mixer.  The mixer can be left running at all times to provide continuous audio output despite input discontinuities.  */
		if(bDspMixer) {
			NEXUS_AudioMixer_GetDefaultSettings(&audioMixerSettings);
			audioMixerSettings.mixUsingDsp = true;
			if(b1x5p && j==audioXcodeChanNum) {
				audioMixerSettings.outputSampleRate = 48000;/* fixed to allow gap filling for bogus audio PID */
			}
			dspMixer[j] = NEXUS_AudioMixer_Open(&audioMixerSettings);
			assert(dspMixer[j]);
		}

        /* Connect audio decoders/mixers to outputs for RT mode */
        if(!bNonRealTime)
        {
            if(bDspMixer) {/* DspMixers */
	            if(b1x5p && j==audioXcodeChanNum) {/* sdpMixer -> dummy1 */
	                NEXUS_AudioOutput_AddInput(
	                    NEXUS_AudioDummyOutput_GetConnector(platformConfig.outputs.audioDummy[1]),
	                    NEXUS_AudioMixer_GetConnector(dspMixer[j]));
	            } else {/* dspMixer -> hwMixer -> dummy0 out */
	                NEXUS_AudioMixer_AddInput(audioMixer, 
	                    NEXUS_AudioMixer_GetConnector(dspMixer[j]));
	            }
            } else {
	            if(b1x5p && j==audioXcodeChanNum) {/* Dec -> dummy1 */
	                NEXUS_AudioOutput_AddInput(
	                    NEXUS_AudioDummyOutput_GetConnector(platformConfig.outputs.audioDummy[1]),
	                    NEXUS_AudioDecoder_GetConnector(audioDecoder[j], NEXUS_AudioDecoderConnectorType_eStereo));
	            } else {/* Dec -> hwMixer -> dummy out */
	                NEXUS_AudioMixer_AddInput(audioMixer, 
	                    NEXUS_AudioDecoder_GetConnector(audioDecoder[j], NEXUS_AudioDecoderConnectorType_eCompressed));
	            }
            }
        }

        /* Open audio mux output */
        audioMuxOutput[j] = NEXUS_AudioMuxOutput_Create(NULL);
        assert(audioMuxOutput[j]);

        if(b1x5p && j==audioXcodeChanNum) {
	        /* Open audio encoder */
	        NEXUS_AudioEncoder_GetDefaultSettings(&encoderSettings);
	        encoderSettings.codec = NEXUS_AudioCodec_eAacPlusLoas;
	        audioCodec[j] = encoderSettings.codec;
	        audioEncoder = NEXUS_AudioEncoder_Open(&encoderSettings);
	        assert(audioEncoder);

			if(!bDspMixer) {
				/* Connect decoder -> encoder */
				NEXUS_AudioEncoder_AddInput(audioEncoder,
					NEXUS_AudioDecoder_GetConnector(audioDecoder[j], NEXUS_AudioDecoderConnectorType_eStereo));
				BDBG_WRN(("No DSP mixer..."));
			}
			else {
				/* Connect decoder -> dsp mixer and set as master */
				NEXUS_AudioMixer_AddInput(dspMixer[j],
					NEXUS_AudioDecoder_GetConnector(audioDecoder[j], NEXUS_AudioDecoderConnectorType_eStereo));
				audioMixerSettings.master = NEXUS_AudioDecoder_GetConnector(audioDecoder[j], NEXUS_AudioDecoderConnectorType_eStereo);
				NEXUS_AudioMixer_SetSettings(dspMixer[j], &audioMixerSettings);
				/* Connect mixer -> encoder */
				NEXUS_AudioEncoder_AddInput(audioEncoder, 
					NEXUS_AudioMixer_GetConnector(dspMixer[j]));
			}

	        /* Connect encoder -> mux out */
	        NEXUS_AudioOutput_AddInput(
	            NEXUS_AudioMuxOutput_GetConnector(audioMuxOutput[j]), NEXUS_AudioEncoder_GetConnector(audioEncoder));
        } else {/* pass-thru */
			if(!bDspMixer) {
				/* Connect decoder -> mux out */
				NEXUS_AudioOutput_AddInput(
					NEXUS_AudioMuxOutput_GetConnector(audioMuxOutput[j]),
					NEXUS_AudioDecoder_GetConnector(audioDecoder[j], NEXUS_AudioDecoderConnectorType_eCompressed));
			}
			else {/* dspMixer -> mux out */
				NEXUS_AudioMixer_AddInput(dspMixer[j],
					NEXUS_AudioDecoder_GetConnector(audioDecoder[j], NEXUS_AudioDecoderConnectorType_eCompressed));
				audioMixerSettings.master = NEXUS_AudioDecoder_GetConnector(audioDecoder[j], NEXUS_AudioDecoderConnectorType_eCompressed);
				NEXUS_AudioMixer_SetSettings(dspMixer[j], &audioMixerSettings);
				NEXUS_AudioOutput_AddInput(
					NEXUS_AudioMuxOutput_GetConnector(audioMuxOutput[j]),
					NEXUS_AudioMixer_GetConnector(dspMixer[j]));
			}

	        audioCodec[j] = audioProgram[j].codec;
        }
    }
#endif

    /* bring up decoder and connect to local display */
    videoDecoder = NEXUS_VideoDecoder_Open(0, NULL); /* take default capabilities */

    /* NOTE: must open video encoder before display; otherwise open will init ViCE2 core
     * which might cause encoder display GISB error since encoder display would
     * trigger RDC to program mailbox registers in ViCE2;
     */
#if BTST_HAS_VIDEO_ENCODE_TEST
    videoEncoder = NEXUS_VideoEncoder_Open(0, NULL);
    assert(videoEncoder);
#endif
    NEXUS_GetVideoEncoderCapabilities(&videoEncoderCap);

    /* Bring up video encoder display */
    NEXUS_Display_GetDefaultSettings(&displaySettings);
    displaySettings.displayType = NEXUS_DisplayType_eAuto;
    displaySettings.timingGenerator = NEXUS_DisplayTimingGenerator_eEncoder;
    displaySettings.format = cmdSettings[i].displayFormat;/* source is 60hz */
    displaySettings.frameRateMaster = NULL;/* disable frame rate tracking for now */

    displaySettings.format = cmdSettings[i].displayFormat;
    displayTranscode = NEXUS_Display_Open(videoEncoderCap.videoEncoder[0].displayIndex, &displaySettings);/* cmp3 for transcoder */
    assert(displayTranscode);

	/* NRT setup - AFAP mode */
	NEXUS_Display_GetStgSettings(displayTranscode, &stgSettings);
	stgSettings.enabled     = true;
	stgSettings.nonRealTime = bNonRealTime;
	NEXUS_Display_SetStgSettings(displayTranscode, &stgSettings);

    windowTranscode = NEXUS_VideoWindow_Open(displayTranscode, 0);
    assert(windowTranscode);

    /* enable deinterlacer to improve quality */
    NEXUS_VideoWindow_GetMadSettings(windowTranscode, &windowMadSettings);
    windowMadSettings.deinterlace = true;
    windowMadSettings.enable22Pulldown = true;
    windowMadSettings.enable32Pulldown = true;
    NEXUS_VideoWindow_SetMadSettings(windowTranscode, &windowMadSettings);

    /* connect same decoder to the encoder display;
     * NOTE: simul display + transcode mode might have limitation in audio pathre;
     * here is for video transcode bringup purpose;
     */
    NEXUS_VideoWindow_AddInput(windowTranscode, NEXUS_VideoDecoder_GetConnector(videoDecoder));

#if BTST_DISABLE_NRT_DEBUG_DISPLAY
	if(!bNonRealTime)
#endif
	{
		NEXUS_VideoWindow_AddInput(window, NEXUS_VideoDecoder_GetConnector(videoDecoder));
	}

    /* Open the video pid channel */
    NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
    playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eVideo;
    playbackPidSettings.pidTypeSettings.video.codec = vCodec; /* must be told codec for correct handling */
    playbackPidSettings.pidTypeSettings.video.index = true;
    playbackPidSettings.pidTypeSettings.video.decoder = videoDecoder;
    videoPidChannel = NEXUS_Playback_OpenPidChannel(playback, vPid, &playbackPidSettings);

    /* Set up decoder Start structures now. We need to know the audio codec to properly set up the audio outputs. */
    NEXUS_VideoDecoder_GetDefaultStartSettings(&videoProgram);
    videoProgram.codec = vCodec;
    videoProgram.pidChannel = videoPidChannel;
    videoProgram.stcChannel = stcChannel;
    videoProgram.nonRealTime = bNonRealTime;

#if BTST_HAS_VIDEO_ENCODE_TEST
    NEXUS_VideoEncoder_GetSettings(videoEncoder, &videoEncoderConfig);
    videoEncoderConfig.variableFrameRate = false; 
    videoEncoderConfig.frameRate = NEXUS_VideoFrameRate_e30;
    videoEncoderConfig.frameRate = cmdSettings[i].encoderFrameRate;
    videoEncoderConfig.bitrateMax = cmdSettings[i].encoderBitrate;
    videoEncoderConfig.streamStructure.framesP = cmdSettings[i].encoderGopStructureFramesP;
    videoEncoderConfig.streamStructure.framesB = cmdSettings[i].encoderGopStructureFramesB;

    NEXUS_VideoEncoder_GetDefaultStartSettings(&videoEncoderStartConfig);
    videoEncoderStartConfig.codec = cmdSettings[i].encoderVideoCodec;
    videoEncoderStartConfig.profile = cmdSettings[i].encoderProfile;
    videoEncoderStartConfig.level = cmdSettings[i].encoderLevel;
    videoEncoderStartConfig.input = displayTranscode;
    videoEncoderStartConfig.stcChannel = stcChannelTranscode;
    videoEncoderStartConfig.nonRealTime = bNonRealTime;

    /* encode setting and startSetting to be set after end-to-end delay is determined */
    
    /* get end-to-end delay (Dee) for audio and video encoders; 
     * TODO: match AV delay! In other words,
     *   if (aDee > vDee) {
     *       vDee' = aDee' = aDee;
     *   }
     *   else {
     *       vDee' = aDee' = vDee;
     *   }
     */
    {
#if NEXUS_HAS_AUDIO_MUX_OUTPUT_TEST    
        unsigned Dee=0;
#endif
        /* NOTE: video encoder delay is in 27MHz ticks */
        NEXUS_VideoEncoder_GetDelayRange(videoEncoder, &videoEncoderConfig, &videoEncoderStartConfig, &videoDelay);
        printf("\n\tVideo encoder end-to-end delay = %u ms; maximum allowed: %u ms\n", videoDelay.min/27000, videoDelay.max/27000);
    
#if NEXUS_HAS_AUDIO_MUX_OUTPUT_TEST    
        /* pick the maximum delay of 6 audios. */
        for(j=0; j<numAudios; j++) {
            NEXUS_AudioMuxOutput_GetDelayStatus(audioMuxOutput[j], audioCodec[j], &audioDelayStatus);
            printf("\tAudio[%d] codec %d end-to-end delay = %u ms\n", j, audioCodec[j], audioDelayStatus.endToEndDelay);
            if(Dee < audioDelayStatus.endToEndDelay) Dee = audioDelayStatus.endToEndDelay;
        }
        Dee = Dee * 27000; /* in 27MHz ticks */
        if(Dee > videoDelay.min)
        {
            if(Dee > videoDelay.max)
            {
                BDBG_ERR(("\tAudio Dee is way too big! Use video Dee max!"));
                Dee = videoDelay.max;
            }
            else
            {
                printf("\tUse audio Dee %u ms %u ticks@27Mhz!\n", Dee/27000, Dee);
            }
        }
        else
        {
            Dee = videoDelay.min;
            printf("\tUse video Dee %u ms or %u ticks@27Mhz!\n\n", Dee/27000, Dee);
        }
        videoEncoderConfig.encoderDelay = Dee;

        /* Start audio mux output */
        for(j=0; j<numAudios; j++) {
            NEXUS_AudioMuxOutput_GetDefaultStartSettings(&audioMuxStartSettings);
            /* audio NRT requires mux out to take NRT decode STC */
            audioMuxStartSettings.stcChannel = bNonRealTime? stcAudioChannel : stcChannelTranscode;
            audioMuxStartSettings.presentationDelay = Dee/27000;/* in ms */
            audioMuxStartSettings.nonRealTime       = bNonRealTime;
            NEXUS_AudioMuxOutput_Start(audioMuxOutput[j], &audioMuxStartSettings);
        }
#else
        videoEncoderConfig.encoderDelay = videoDelay.min;
#endif
    }
	/* Note: video encoder SetSettings needs to be called after the encoder delay is determined; */
    NEXUS_VideoEncoder_SetSettings(videoEncoder, &videoEncoderConfig);

    NEXUS_Playpump_GetDefaultOpenSettings(&playpumpConfig);
    playpumpConfig.fifoSize = 16384; /* reduce FIFO size allocated for playpump */
    playpumpConfig.numDescriptors = 64; /* set number of descriptors */
    playpumpConfig.streamMuxCompatible = true;
    playpumpTranscodeVideo = NEXUS_Playpump_Open(1, &playpumpConfig);
    assert(playpumpTranscodeVideo);
#endif

    playpumpTranscodePcr = NEXUS_Playpump_Open(2, &playpumpConfig);
    assert(playpumpTranscodePcr);
#if NEXUS_HAS_AUDIO_MUX_OUTPUT_TEST    
    NEXUS_Playpump_GetDefaultOpenSettings(&playpumpConfig);
    playpumpConfig.fifoSize = 16384; /* reduce FIFO size allocated for playpump */
    playpumpConfig.numDescriptors = 64; /* set number of descriptors */
    playpumpConfig.streamMuxCompatible = true;
    for(j=0; j<numAudios; j++) {
        playpumpTranscodeAudio[j] = NEXUS_Playpump_Open(3+j, &playpumpConfig);
        assert(playpumpTranscodeAudio[j]);
    }
#endif    
    
    BKNI_CreateEvent(&finishEvent);
    NEXUS_StreamMux_GetDefaultCreateSettings(&muxCreateSettings);
    muxCreateSettings.finished.callback = transcoderFinishCallback;
    muxCreateSettings.finished.context = finishEvent;
    streamMux = NEXUS_StreamMux_Create(&muxCreateSettings);
    NEXUS_StreamMux_GetDefaultStartSettings(&muxConfig);
    muxConfig.transportType = NEXUS_TransportType_eTs;
    muxConfig.stcChannel = stcChannelTranscode;
    muxConfig.nonRealTime = bNonRealTime;
    muxConfig.nonRealTimeRate = 32 * NEXUS_NORMAL_PLAY_SPEED; /* AFAP */

#if BTST_HAS_VIDEO_ENCODE_TEST
    muxConfig.video[0].pid = 0x11;
    muxConfig.video[0].encoder = videoEncoder;
    muxConfig.video[0].playpump = playpumpTranscodeVideo;
#endif    
    muxConfig.pcr.pid = 0x12;
    muxConfig.pcr.playpump = playpumpTranscodePcr;
    muxConfig.pcr.interval = 50;
#if NEXUS_HAS_AUDIO_MUX_OUTPUT_TEST    
    for(j=0; j<numAudios; j++) {
        muxConfig.audio[j].pid = 0x13+j;
        muxConfig.audio[j].pesId += j; /* differentiate audio channels in TS mux PID filters with PES streamIDs */
        muxConfig.audio[j].muxOutput = audioMuxOutput[j];
        muxConfig.audio[j].playpump = playpumpTranscodeAudio[j];
    }
#endif

#define BTST_PMT_PID 0x0055
    /* open PidChannels */
    pidChannelTranscodePcr = NEXUS_Playpump_OpenPidChannel(playpumpTranscodePcr, muxConfig.pcr.pid, NULL);
    assert(pidChannelTranscodePcr);
    pidChannelTranscodePmt = NEXUS_Playpump_OpenPidChannel(playpumpTranscodePcr, BTST_PMT_PID, NULL);
    assert(pidChannelTranscodePmt);
    pidChannelTranscodePat = NEXUS_Playpump_OpenPidChannel(playpumpTranscodePcr, 0, NULL);
    assert(pidChannelTranscodePat);

    /* start mux */
    NEXUS_StreamMux_Start(streamMux,&muxConfig, &muxOutput);
#if BTST_HAS_VIDEO_ENCODE_TEST
    pidChannelTranscodeVideo = muxOutput.video[0];
#endif
#if NEXUS_HAS_AUDIO_MUX_OUTPUT_TEST    
    for(j=0; j<numAudios; j++) {
        pidChannelTranscodeAudio[j] = muxOutput.audio[j];
    }
#endif

	/******************************************
	 *  NRT workaround for the XPT band hold and data ready events sharing the same threshold: avoid band hold from occuring,
	 * otherwise, video stutter would happen!
	 * 1) The nexus_record timer fires at 250 ms interval to service record data as bottom line;
	 * 2) whenever nexus record timer fires, it'll consume up to Nx(3x(47x4096)) cdb data;
	 * 3) so if file i/o can keep up, band hold threshold = 2x(3x(47x4096)) = 1.1MB can sustain record bit rate up to
	 *       2 * 3 * 188K * 8 / 250ms = 36 Mbps without reaching band hold;
	 * 4) larger band hold threshold can sustain higher record bit rate throughput.
	 * NOTE: after SW7425-1663, recpump data ready threshold is decoupled with RAVE upper (band-hold) threshold, so
	 * we do not need to mess with data ready threshold any more!
	 */
	NEXUS_Recpump_GetDefaultOpenSettings(&recpumpOpenSettings);
    recpump = NEXUS_Recpump_Open(0, &recpumpOpenSettings);
    assert(recpump);

    record = NEXUS_Record_Create();
    assert(record);

    NEXUS_Record_GetSettings(record, &recordSettings);
    recordSettings.recpump = recpump;
    recordSettings.recpumpSettings.bandHold = NEXUS_RecpumpFlowControl_eEnable;
    NEXUS_Record_SetSettings(record, &recordSettings);

#if BTST_HAS_VIDEO_ENCODE_TEST
    /* configure the video pid for indexing */
    NEXUS_Record_GetDefaultPidChannelSettings(&recordPidSettings);
    recordPidSettings.recpumpSettings.pidType = NEXUS_PidType_eVideo;
    recordPidSettings.recpumpSettings.pidTypeSettings.video.index = true;
    recordPidSettings.recpumpSettings.pidTypeSettings.video.codec = cmdSettings[i].encoderVideoCodec;

    /* add multiplex data to the same record */
    NEXUS_Record_AddPidChannel(record, pidChannelTranscodeVideo, &recordPidSettings);
#endif
#if NEXUS_HAS_AUDIO_MUX_OUTPUT_TEST    
    for(j=0; j<numAudios; j++) {
        NEXUS_Record_AddPidChannel(record, pidChannelTranscodeAudio[j], NULL);
    }
#endif
    NEXUS_Record_AddPidChannel(record, pidChannelTranscodePcr, NULL);
    NEXUS_Record_AddPidChannel(record, pidChannelTranscodePat, NULL);
    NEXUS_Record_AddPidChannel(record, pidChannelTranscodePmt, NULL);

    fileTranscode = NEXUS_FileRecord_OpenPosix("videos/stream.mpg", "videos/stream.nav");
    assert(fileTranscode);

    /* Start record of stream mux output */
    NEXUS_Record_Start(record, fileTranscode);

#if BTST_ENABLE_AV_SYNC
    /* connect sync channel */
    NEXUS_SyncChannel_GetSettings(syncChannel, &syncChannelSettings);
    syncChannelSettings.videoInput = NEXUS_VideoDecoder_GetConnector(videoDecoder);
#if BTST_ENABLE_NRT_STC_AV_WINDOW
	/* NRT mode pairs AV stc channels */
	if(bNonRealTime) {
		NEXUS_StcChannel_GetDefaultPairSettings(&stcAudioVideoPair);
		stcAudioVideoPair.connected = true;
		stcAudioVideoPair.window = 300; /* 300ms AV window means when source discontinuity occurs, up to 300ms transition could occur with NRT transcoded stream */
		NEXUS_StcChannel_SetPairSettings(stcChannel, stcAudioChannel, &stcAudioVideoPair);
	}
#endif
    for(j=0; j<numAudios; j++) {
        if(b1x5p && j==audioXcodeChanNum) {
            syncChannelSettings.audioInput[j] = NEXUS_AudioDecoder_GetConnector(audioDecoder[j], NEXUS_AudioDecoderConnectorType_eStereo);
        } else {
            syncChannelSettings.audioInput[j] = NEXUS_AudioDecoder_GetConnector(audioDecoder[j], NEXUS_AudioDecoderConnectorType_eCompressed);
        }
    }
    NEXUS_SyncChannel_SetSettings(syncChannel, &syncChannelSettings);
#endif

    /* Start decoder */
    NEXUS_VideoDecoder_Start(videoDecoder, &videoProgram);
#if NEXUS_HAS_AUDIO_MUX_OUTPUT_TEST    
    for(j=0; j<numAudios; j++) {
        if(bDspMixer) {/* need to explicitly start mixer before decoder to seamlessly handle discontinuity and stream wraparound */
            NEXUS_AudioMixer_Start(dspMixer[j]);
        }
        NEXUS_AudioDecoder_Start(audioDecoder[j], &audioProgram[j]);
    }
#endif
    /* Start playback */
    NEXUS_Playback_Start(playback, file, NULL);
    {
static const uint8_t s_auiTSPacket_PAT[188] =
{
   0x47,0x40,0x00,0x30,0xa6,0x40,0xff,0xff,
   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff,0xff,'P','A','T',
   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0x00,0x00,0xb0,0x0d,0x00,
   0x00,0x81,0x00,0x00,0x00,0x01,0xe0,0x55,
   0x65,0x80,0x5e,0xdc,
};
#define BTST_PMT_TRANSPORT_IDX BTST_SYSTEM_TRANSPORT_IDX
static const uint8_t s_auiTSPacket_PMT[188] =
{
   0x47,0x40,BTST_PMT_PID,0x30,0x83,0x40,0xff,0xff,
   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff,0xff,'P','M','T',
   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
   0x00,0x02,0xb0,0x30,0x00,0x01,0xc1,0x00,
   0x00,0xe0,0x12,0xf0,0x00,/* PCR PID */
   0x1b,0xe0,0x11,0xf0,0x00,/* video PID */
   0x04,0xe0,0x13,0xf0,0x00,/* 6xaudio PIDs */
   0x04,0xe0,0x14,0xf0,0x00,
   0x04,0xe0,0x15,0xf0,0x00,
   0x04,0xe0,0x16,0xf0,0x00,
   0x04,0xe0,0x17,0xf0,0x00,
   0x04,0xe0,0x18,0xf0,0x00,
   0x3d,0x19,0x07,0x2f,
};
        NEXUS_StreamMuxSystemData psi[2];
        /* Get CRC right to be playable by VLCplayer etc 3rd party SW */
        uint32_t uiCRC = CRC32_mpeg(0, (uint8_t *) s_auiTSPacket_PAT + 184 - (8+4*1), 8+4*1);
        fprintf(stderr, "PAT crc=%x\n", uiCRC);
        NEXUS_Memory_Allocate(188, NULL, &pat);
        NEXUS_Memory_Allocate(188, NULL, &pmt);
        BKNI_Memcpy(pat, s_auiTSPacket_PAT, sizeof(s_auiTSPacket_PAT));
        BKNI_Memcpy(pmt, s_auiTSPacket_PMT, sizeof(s_auiTSPacket_PMT));
        ((uint8_t*)pat)[184] = (uiCRC >> 24) & 0xFF;
        ((uint8_t*)pat)[185] = (uiCRC >> 16) & 0xFF;
        ((uint8_t*)pat)[186] = (uiCRC >>  8) & 0xFF;
        ((uint8_t*)pat)[187] = (uiCRC >>  0) & 0xFF;

		/* video codec */
		switch(videoEncoderStartConfig.codec)
		{
			case NEXUS_VideoCodec_eMpeg2:         ((uint8_t *) pmt)[184-7*5] = 0x2; break;
			case NEXUS_VideoCodec_eMpeg4Part2:    ((uint8_t *) pmt)[184-7*5] = 0x10; break;
			case NEXUS_VideoCodec_eH264:          ((uint8_t *) pmt)[184-7*5] = 0x1b; break;
			case NEXUS_VideoCodec_eVc1SimpleMain: ((uint8_t *) pmt)[184-7*5] = 0xea; break;
			default:
				BDBG_ERR(("Video encoder codec %d is not supported!\n", videoEncoderStartConfig.codec));
				BDBG_ASSERT(0);
		}
        /* audio stream type */
        for(j=0; j<numAudios; j++) {
			switch(audioCodec[j])
			{
				case NEXUS_AudioCodec_eMpeg:         ((uint8_t *) pmt)[184-5*(6-j)] = 0x4; break;
				case NEXUS_AudioCodec_eMp3:          ((uint8_t *) pmt)[184-5*(6-j)] = 0x4; break;
				case NEXUS_AudioCodec_eAac    :      ((uint8_t *) pmt)[184-5*(6-j)] = 0xf; break; /* ADTS */
				case NEXUS_AudioCodec_eAacPlus:      ((uint8_t *) pmt)[184-5*(6-j)] = 0x11; break;/* LOAS */
				/* MP2TS doesn't allow 14496-3 AAC+ADTS; here is placeholder to test AAC-HE before LOAS encode is supported; */
				case NEXUS_AudioCodec_eAacPlusAdts:  ((uint8_t *) pmt)[184-5*(6-j)] = 0x11; break;
				case NEXUS_AudioCodec_eAc3:          ((uint8_t *) pmt)[184-5*(6-j)] = 0x81; break;
				default:
					BDBG_ERR(("Audio encoder codec %d is not supported!\n", audioCodec[j]));
			}
        }

        /* 6xA+V = 7x ES */
        uiCRC = CRC32_mpeg(0, (uint8_t *) pmt + 184 - (12+5*7), 12+5*7);
        fprintf(stderr, "PMT crc=%x\n", uiCRC);

        ((uint8_t*)pmt)[184] = (uiCRC >> 24) & 0xFF;
        ((uint8_t*)pmt)[185] = (uiCRC >> 16) & 0xFF;
        ((uint8_t*)pmt)[186] = (uiCRC >>  8) & 0xFF;
        ((uint8_t*)pmt)[187] = (uiCRC >>  0) & 0xFF;
        NEXUS_Memory_FlushCache(pat, sizeof(s_auiTSPacket_PAT));
        NEXUS_Memory_FlushCache(pmt, sizeof(s_auiTSPacket_PMT));
        BKNI_Memset(psi, 0, sizeof(psi));
        psi[0].size = 188;
        psi[0].pData = pat;
        psi[0].timestampDelta = 100;
        psi[1].size = 188;
        psi[1].pData = pmt;
        psi[1].timestampDelta = 100;
        NEXUS_StreamMux_AddSystemDataBuffer(streamMux, &psi[0]);
        NEXUS_StreamMux_AddSystemDataBuffer(streamMux, &psi[1]);
	}

    NEXUS_VideoEncoder_Start(videoEncoder, &videoEncoderStartConfig);

    /* Playback state machine is driven from inside Nexus. */
    printf("Press ENTER to continue; type 'q' to quit\n");
    key = getchar();

    /* Bring down system */
    NEXUS_Playback_Stop(playback);
    NEXUS_VideoDecoder_Stop(videoDecoder);
#if NEXUS_HAS_AUDIO_MUX_OUTPUT_TEST    
    for(j=0; j<numAudios; j++) {
        NEXUS_AudioDecoder_Stop(audioDecoder[j]);
        if(bDspMixer) {
            NEXUS_AudioMixer_Stop(dspMixer[j]);
        }
        NEXUS_AudioMuxOutput_Stop(audioMuxOutput[j]);
    }
#endif
#if BTST_ENABLE_AV_SYNC
    /* disconnect sync channel */
    NEXUS_SyncChannel_GetSettings(syncChannel, &syncChannelSettings);
    syncChannelSettings.videoInput = NULL;
    for(j=0; j<numAudios; j++) {
        syncChannelSettings.audioInput[j] = NULL;
    }
    NEXUS_SyncChannel_SetSettings(syncChannel, &syncChannelSettings);
#endif

    
#if BTST_HAS_VIDEO_ENCODE_TEST
    NEXUS_VideoEncoder_Stop(videoEncoder, NULL);
#endif
    NEXUS_StreamMux_Finish(streamMux);
    /* wait for the encoder buffer model's data to be drained */
    if(BKNI_WaitForEvent(finishEvent, (videoEncoderConfig.encoderDelay/27000)*2)!=BERR_SUCCESS) {
        fprintf(stderr, "TIMEOUT\n");
    }
    BKNI_DestroyEvent(finishEvent);
    NEXUS_Record_Stop(record);
	/* Note: remove all record PID channels before stream mux stop since streammux would close the A/V PID channels */
    NEXUS_Record_RemoveAllPidChannels(record);
    NEXUS_StreamMux_Stop(streamMux);

    NEXUS_Record_Destroy(record);
    NEXUS_Recpump_Close(recpump);
    NEXUS_FileRecord_Close(fileTranscode);

#if NEXUS_HAS_AUDIO_MUX_OUTPUT_TEST    
#if BTST_SINGLE_AUDIO_PID_PASSTHRU_6X /* avatar */  
    NEXUS_Playback_ClosePidChannel(playback, audioPidChannel[0]);
#else
    for(j=0; j<numAudios; j++)
    NEXUS_Playback_ClosePidChannel(playback, audioPidChannel[j]);
#endif
#endif
    NEXUS_Playback_ClosePidChannel(playback, videoPidChannel);
    NEXUS_Playpump_ClosePidChannel(playpumpTranscodePcr, pidChannelTranscodePcr);
    NEXUS_Playpump_ClosePidChannel(playpumpTranscodePcr, pidChannelTranscodePat);
    NEXUS_Playpump_ClosePidChannel(playpumpTranscodePcr, pidChannelTranscodePmt);
    NEXUS_FilePlay_Close(file);
    NEXUS_Playback_Destroy(playback);
    NEXUS_Playpump_Close(playpump);

#if BTST_DISABLE_NRT_DEBUG_DISPLAY
    if(!bNonRealTime)
#endif
    NEXUS_VideoWindow_RemoveInput(window, NEXUS_VideoDecoder_GetConnector(videoDecoder));
    NEXUS_VideoWindow_RemoveInput(windowTranscode, NEXUS_VideoDecoder_GetConnector(videoDecoder));
    NEXUS_VideoInput_Shutdown(NEXUS_VideoDecoder_GetConnector(videoDecoder));
    NEXUS_VideoDecoder_Close(videoDecoder);
    NEXUS_VideoWindow_Close(window);
    NEXUS_VideoWindow_Close(windowTranscode);
    NEXUS_Display_Close(display);
    NEXUS_Display_Close(displayTranscode);

    NEXUS_StreamMux_Destroy(streamMux);

    NEXUS_Playpump_Close(playpumpTranscodePcr);
	
#if BTST_HAS_VIDEO_ENCODE_TEST
	NEXUS_Playpump_Close(playpumpTranscodeVideo);
	NEXUS_VideoEncoder_Close(videoEncoder);
#endif    

#if NEXUS_HAS_AUDIO_MUX_OUTPUT_TEST    
	if(b1x5p) {
		NEXUS_AudioOutput_RemoveAllInputs(NEXUS_AudioDummyOutput_GetConnector(platformConfig.outputs.audioDummy[1]));
		NEXUS_AudioOutput_Shutdown(NEXUS_AudioDummyOutput_GetConnector(platformConfig.outputs.audioDummy[1]));
	}
    NEXUS_AudioMixer_RemoveAllInputs(audioMixer);
    NEXUS_AudioOutput_RemoveAllInputs(NEXUS_AudioDummyOutput_GetConnector(platformConfig.outputs.audioDummy[0]));
    NEXUS_AudioOutput_Shutdown(NEXUS_AudioDummyOutput_GetConnector(platformConfig.outputs.audioDummy[0]));
    NEXUS_AudioMixer_Close(audioMixer);
    for(j=0; j<numAudios; j++) {
		NEXUS_AudioOutput_RemoveAllInputs( NEXUS_AudioMuxOutput_GetConnector(audioMuxOutput[j]));
		if(b1x5p && j==audioXcodeChanNum) { 
			NEXUS_AudioEncoder_RemoveAllInputs(audioEncoder);
			NEXUS_AudioInput_Shutdown(NEXUS_AudioEncoder_GetConnector(audioEncoder));
			NEXUS_AudioEncoder_Close(audioEncoder);
			if(bDspMixer) {
				NEXUS_AudioMixer_RemoveAllInputs(dspMixer[j]);
				NEXUS_AudioInput_Shutdown(NEXUS_AudioMixer_GetConnector(dspMixer[j]));
				NEXUS_AudioMixer_Close(dspMixer[j]);
			}
		} else {/* pass-thru */
			if(bDspMixer) {
				NEXUS_AudioMixer_RemoveAllInputs(dspMixer[j]);
				NEXUS_AudioInput_Shutdown(NEXUS_AudioMixer_GetConnector(dspMixer[j]));
				NEXUS_AudioMixer_Close(dspMixer[j]);
			}
		}
#if NEXUS_NUM_AUDIO_DACS
	    NEXUS_AudioOutput_RemoveAllInputs(NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]));
#endif
#if NEXUS_NUM_SPDIF_OUTPUTS
	    NEXUS_AudioOutput_RemoveAllInputs(NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]));
#endif
#if NEXUS_NUM_HDMI_OUTPUTS
    	NEXUS_AudioOutput_RemoveAllInputs(NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]));
#endif
	    NEXUS_Playpump_Close(playpumpTranscodeAudio[j]);
	    NEXUS_AudioOutput_Shutdown(NEXUS_AudioMuxOutput_GetConnector(audioMuxOutput[j]));
	    NEXUS_AudioMuxOutput_Destroy(audioMuxOutput[j]);

	    NEXUS_AudioInput_Shutdown(NEXUS_AudioDecoder_GetConnector(audioDecoder[j], NEXUS_AudioDecoderConnectorType_eCompressed));
#if NEXUS_NUM_SPDIF_OUTPUTS
	    NEXUS_AudioOutput_Shutdown(NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]));
#endif

	    NEXUS_AudioDecoder_Close(audioDecoder[j]);
    }
#endif

#if BTST_ENABLE_AV_SYNC
    NEXUS_SyncChannel_Destroy(syncChannel);
#endif

    NEXUS_StcChannel_Close(stcChannel);
    NEXUS_StcChannel_Close(stcChannelTranscode);
    if(bNonRealTime) {
        NEXUS_StcChannel_Close(stcAudioChannel);
    }
    NEXUS_Memory_Free(pat);
    NEXUS_Memory_Free(pmt);
    
    if(key != 'q')
    {
        i = iteration++%TEST_ITERATIONS;
        BDBG_WRN(("Start %d iteration.....", iteration));
        goto again;
    }

    NEXUS_Platform_Uninit();

    return 0;
}
#else /* NEXUS_HAS_PLAYBACK && NEXUS_HAS_STREAM_MUX */
int main(void)
{
    printf("This application is not supported on this platform!\n");
    return 0;
}
#endif

