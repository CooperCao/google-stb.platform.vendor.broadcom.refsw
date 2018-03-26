/******************************************************************************
 * Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
/* Nexus example app: playback and decode */

#include "nexus_platform.h"
#include <stdio.h>

#if NEXUS_HAS_PLAYBACK && NEXUS_HAS_STREAM_MUX && (NEXUS_NUM_VIDEO_ENCODERS >= 2)
#include "nexus_video_decoder.h"
#include "nexus_stc_channel.h"
#include "nexus_display.h"
#include "nexus_video_window.h"
#include "nexus_video_input.h"
#include "nexus_video_adj.h"
#include "nexus_spdif_output.h"
#include "nexus_component_output.h"
#include "nexus_core_utils.h"
#if NEXUS_HAS_HDMI_OUTPUT
#include "nexus_hdmi_output.h"
#endif
#include "nexus_playback.h"
#include "nexus_record.h"
#include "nexus_file.h"
#include "nexus_video_encoder.h"
#include "nexus_audio_encoder.h"
#include "nexus_audio_mixer.h"
#include "nexus_stream_mux.h"
#include "nexus_recpump.h"
#include "nexus_record.h"
#if NEXUS_HAS_SYNC_CHANNEL
#include "nexus_sync_channel.h"
#endif

#include <assert.h>
#include "bstd.h"
#include "bkni.h"
#include "bdbg.h"

#define BTST_HAS_VIDEO_ENCODE_TEST       1
#define NEXUS_HAS_AUDIO_MUX_OUTPUT_TEST 1
#define BTST_ENABLE_AUDIO_ENCODE         1

/* Choose one of the following: */
#define BTST_AC3_TRANSCODER_PASSTHRU    1
#define BTST_MPG_TRANSCODER_PASSTHRU    0
#define BTST_AAC_TRANSCODER_PASSTHRU    0

/* the following define the input file and its characteristics -- these will vary by input file */
#define TRANSPORT_TYPE NEXUS_TransportType_eTs
#if BTST_AC3_TRANSCODER_PASSTHRU /* AC3 */   
  #define FILE_NAME "videos/avatar_AVC_15M.ts" /*"videos/WildChina_Short.ts"*/
  #define VIDEO_CODEC NEXUS_VideoCodec_eH264
  #define AUDIO_CODEC NEXUS_AudioCodec_eAc3
  #define VIDEO_PID 0x101
  #define AUDIO_PID 0x104
#elif BTST_MPG_TRANSCODER_PASSTHRU /* MPEG */   
  #define FILE_NAME "videos/cnnticker.mpg"
  #define VIDEO_CODEC NEXUS_VideoCodec_eMpeg2
  #define AUDIO_CODEC NEXUS_AudioCodec_eMpeg
  #define VIDEO_PID 0x21
  #define AUDIO_PID 0x22
#elif BTST_AAC_TRANSCODER_PASSTHRU /* AAC */
  #define FILE_NAME "videos/testjun26_188.mpg"
  #define VIDEO_CODEC NEXUS_VideoCodec_eMpeg2
  #define AUDIO_CODEC NEXUS_AudioCodec_eAac
  #define VIDEO_PID 0x100
  #define AUDIO_PID 0x110
#else
#error Please choose one of options above.
#endif

#define XCODE_FILE_NAME "videos/cnnticker.mpg"
#define XCODE_VIDEO_PID 0x21
#define XCODE_AUDIO_PID 0x22

BDBG_MODULE(transcode_playback_to_ts_dual);


#define BTST_PMT_PID 0x0055

/* multi buffer to allow the background PSI packet to update CC while foreground one is waiting for TS mux pacing */
static void *pat[NEXUS_NUM_VIDEO_ENCODERS]={NULL,};
static void *pmt[NEXUS_NUM_VIDEO_ENCODERS]={NULL,};
NEXUS_StreamMuxSystemData psi[NEXUS_NUM_VIDEO_ENCODERS][2];

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
   0x47,0x40,BTST_PMT_PID,0x30,0x9c,0x40,0xff,0xff,
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
   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
   0xff,0x00,0x02,0xb0,0x17,0x00,0x01,0xc1,
   0x00,0x00,0xe0,0x13,0xf0,0x00,0x1b,0xe0,
   0x11,0xf0,0x00,0x81,0xe0,0x12,0xf0,0x00,
   0x3d,0x19,0x07,0x2f,
};

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
    {NEXUS_VideoFormat_e720p, NEXUS_VideoFrameRate_e29_97, 6*1000*1000, 0, 0, NEXUS_VideoCodec_eH264, NEXUS_VideoCodecProfile_eBaseline, NEXUS_VideoCodecLevel_e31},
    {NEXUS_VideoFormat_e720p, NEXUS_VideoFrameRate_e29_97, 10*1000*1000, 23, 0, NEXUS_VideoCodec_eH264, NEXUS_VideoCodecProfile_eMain, NEXUS_VideoCodecLevel_e40},
    {NEXUS_VideoFormat_e480p, NEXUS_VideoFrameRate_e29_97, 2*1000*1000, 29, 0, NEXUS_VideoCodec_eH264, NEXUS_VideoCodecProfile_eBaseline, NEXUS_VideoCodecLevel_e31}
};

int main(void)
{
    NEXUS_PlatformSettings platformSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_StcChannelHandle stcChannel[NEXUS_NUM_VIDEO_ENCODERS];
    NEXUS_StcChannelSettings stcSettings;
    NEXUS_PidChannelHandle videoPidChannel[NEXUS_NUM_VIDEO_ENCODERS];
    NEXUS_DisplayHandle display=NULL;
    NEXUS_DisplaySettings displaySettings;
    NEXUS_VideoWindowHandle window=NULL;
    NEXUS_VideoWindowMadSettings windowMadSettings;
    NEXUS_VideoDecoderHandle videoDecoder[NEXUS_NUM_VIDEO_ENCODERS];
    NEXUS_VideoDecoderStartSettings videoProgram;
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_HdmiOutputStatus hdmiStatus;
    NEXUS_Error rc;
#endif
#if NEXUS_HAS_AUDIO_MUX_OUTPUT_TEST    
    NEXUS_AudioMixerSettings audioMixerSettings;
    NEXUS_AudioMixerHandle audioMixer[NEXUS_NUM_VIDEO_ENCODERS];
    NEXUS_AudioDecoderHandle audioDecoder[NEXUS_NUM_VIDEO_ENCODERS], audioPassthrough[NEXUS_NUM_VIDEO_ENCODERS];
    NEXUS_AudioDecoderStartSettings audioProgram;
    NEXUS_PidChannelHandle audioPidChannel[NEXUS_NUM_VIDEO_ENCODERS];
    NEXUS_AudioMuxOutputHandle audioMuxOutput[NEXUS_NUM_VIDEO_ENCODERS];
    NEXUS_AudioMuxOutputDelayStatus audioDelayStatus;
    NEXUS_AudioMuxOutputStartSettings audioMuxStartSettings;
    NEXUS_PlaypumpHandle playpumpTranscodeAudio[NEXUS_NUM_VIDEO_ENCODERS];
    NEXUS_PidChannelHandle pidChannelTranscodeAudio[NEXUS_NUM_VIDEO_ENCODERS];
#if BTST_ENABLE_AUDIO_ENCODE 
    NEXUS_AudioEncoderSettings encoderSettings;
    NEXUS_AudioEncoderHandle audioEncoder[NEXUS_NUM_VIDEO_ENCODERS];
#endif
    NEXUS_AudioCodec audioCodec[NEXUS_NUM_VIDEO_ENCODERS];
#endif
    NEXUS_FilePlayHandle file[NEXUS_NUM_VIDEO_ENCODERS];
    NEXUS_PlaypumpHandle playpump[NEXUS_NUM_VIDEO_ENCODERS];
    NEXUS_PlaybackHandle playback[NEXUS_NUM_VIDEO_ENCODERS];
    NEXUS_PlaybackSettings playbackSettings;
    NEXUS_PlaybackPidChannelSettings playbackPidSettings;
    NEXUS_DisplayHandle displayTranscode[NEXUS_NUM_VIDEO_ENCODERS];
    NEXUS_VideoWindowHandle windowTranscode[NEXUS_NUM_VIDEO_ENCODERS];
    NEXUS_VideoEncoderCapabilities videoEncoderCap;
#if BTST_HAS_VIDEO_ENCODE_TEST
    NEXUS_VideoEncoderHandle videoEncoder[NEXUS_NUM_VIDEO_ENCODERS];
    NEXUS_VideoEncoderSettings videoEncoderConfig;
    NEXUS_VideoEncoderStartSettings videoEncoderStartConfig;
    NEXUS_VideoEncoderDelayRange videoDelay;
    NEXUS_PlaypumpHandle playpumpTranscodeVideo[NEXUS_NUM_VIDEO_ENCODERS];
    NEXUS_PidChannelHandle pidChannelTranscodeVideo[NEXUS_NUM_VIDEO_ENCODERS];
    NEXUS_RecordPidChannelSettings recordPidSettings;
#endif
    NEXUS_StreamMuxHandle streamMux[NEXUS_NUM_VIDEO_ENCODERS];
    NEXUS_StreamMuxCreateSettings muxCreateSettings;
    BKNI_EventHandle finishEvent[NEXUS_NUM_VIDEO_ENCODERS];
    NEXUS_StreamMuxStartSettings muxConfig;
    NEXUS_StreamMuxOutput muxOutput;
    NEXUS_PlaypumpOpenSettings playpumpConfig;
    NEXUS_PlaypumpHandle playpumpTranscodePcr[NEXUS_NUM_VIDEO_ENCODERS];
    NEXUS_FileRecordHandle fileTranscode[NEXUS_NUM_VIDEO_ENCODERS];
    NEXUS_StcChannelHandle stcChannelTranscode[NEXUS_NUM_VIDEO_ENCODERS];

#if NEXUS_HAS_SYNC_CHANNEL
    NEXUS_SyncChannelSettings syncChannelSettings;
    NEXUS_SyncChannelHandle syncChannel[NEXUS_NUM_VIDEO_ENCODERS];
#endif
    NEXUS_RecpumpHandle recpump[NEXUS_NUM_VIDEO_ENCODERS];
    NEXUS_RecordHandle record[NEXUS_NUM_VIDEO_ENCODERS];
    NEXUS_RecordSettings recordSettings;
    NEXUS_PidChannelHandle pidChannelTranscodePcr[NEXUS_NUM_VIDEO_ENCODERS];
    NEXUS_PidChannelHandle pidChannelTranscodePat[NEXUS_NUM_VIDEO_ENCODERS];
    NEXUS_PidChannelHandle pidChannelTranscodePmt[NEXUS_NUM_VIDEO_ENCODERS];

    const char fname[2][40] = {
        FILE_NAME,
        XCODE_FILE_NAME
    };

    int i = 0, xcodeId=0;
    int iteration = 1;
    char key;

    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init(&platformSettings);
    NEXUS_Platform_GetConfiguration(&platformConfig);

again:
    BDBG_WRN(("Setting up transcode pipeline: format %d, fr %d, bitrate %u, gopP %d, codec %d",
                cmdSettings[i].displayFormat,
                cmdSettings[i].encoderFrameRate,
                cmdSettings[i].encoderBitrate,
                cmdSettings[i].encoderGopStructureFramesP,
                cmdSettings[i].encoderVideoCodec));

    for(xcodeId=0; xcodeId < 2; xcodeId++)
    {
        playpump[xcodeId] = NEXUS_Playpump_Open(xcodeId*4, NULL);
        assert(playpump[xcodeId]);
        playback[xcodeId] = NEXUS_Playback_Create();
        assert(playback[xcodeId]);

        file[xcodeId] = NEXUS_FilePlay_OpenPosix(fname[xcodeId], NULL);
        if (!file[xcodeId]) {
            fprintf(stderr, "can't open file:%s\n", fname[xcodeId]);
            return -1;
        }

#if NEXUS_HAS_SYNC_CHANNEL
        /* create a sync channel */
        NEXUS_SyncChannel_GetDefaultSettings(&syncChannelSettings);
        syncChannel[xcodeId] = NEXUS_SyncChannel_Create(&syncChannelSettings);
#endif

        NEXUS_StcChannel_GetDefaultSettings(xcodeId * 2, &stcSettings);
        stcSettings.timebase = NEXUS_Timebase_e0 + xcodeId;
        stcSettings.mode = NEXUS_StcChannelMode_eAuto;
        stcChannel[xcodeId] = NEXUS_StcChannel_Open(xcodeId * 2, &stcSettings);

        /* encoders/mux require different STC broadcast mode from decoder */
        NEXUS_StcChannel_GetDefaultSettings(xcodeId * 2 + 1, &stcSettings);
        stcSettings.timebase = NEXUS_Timebase_e0 + xcodeId;/* should be the same timebase for end-to-end locking */
        stcSettings.mode = NEXUS_StcChannelMode_eAuto;
        stcSettings.pcrBits = NEXUS_StcChannel_PcrBits_eFull42;/* ViCE2 requires 42-bit STC broadcast */
        stcChannelTranscode[xcodeId] = NEXUS_StcChannel_Open(xcodeId * 2 + 1, &stcSettings);

        if(xcodeId==0) {
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
            NEXUS_VideoWindow_SetMadSettings(window, &windowMadSettings);
        }

        NEXUS_Playback_GetSettings(playback[xcodeId], &playbackSettings);
        playbackSettings.playpump = playpump[xcodeId];
        /* set a stream format, it could be any audio video transport type or file format, i.e NEXUS_TransportType_eMp4, NEXUS_TransportType_eAvi ... */
        playbackSettings.playpumpSettings.transportType = TRANSPORT_TYPE;
        playbackSettings.stcChannel = stcChannel[xcodeId];
        NEXUS_Playback_SetSettings(playback[xcodeId], &playbackSettings);

#if NEXUS_HAS_AUDIO_MUX_OUTPUT_TEST
        /* Open the audio decoder */
#if BTST_ENABLE_AUDIO_ENCODE 
        audioDecoder[xcodeId] = NEXUS_AudioDecoder_Open(xcodeId, NULL);
#else
        audioPassthrough[xcodeId] = NEXUS_AudioDecoder_Open(xcodeId, NULL);
#endif
        /* Open the audio and pcr pid channel */
        NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
        playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eAudio;
        playbackPidSettings.pidTypeSettings.audio.primary = audioDecoder[xcodeId]; /* must be told codec for correct handling */
        playbackPidSettings.pidTypeSettings.audio.secondary = audioPassthrough[xcodeId];

#if BTST_MPG_TRANSCODER_PASSTHRU /* cnnticker MPG */
        audioPidChannel[xcodeId] = NEXUS_Playback_OpenPidChannel(playback[xcodeId], AUDIO_PID, &playbackPidSettings);
#else
        audioPidChannel[xcodeId] = NEXUS_Playback_OpenPidChannel(playback[xcodeId], (xcodeId==1)?XCODE_AUDIO_PID:AUDIO_PID, &playbackPidSettings);
#endif

        /* Set up decoder Start structures now. We need to know the audio codec to properly set up the audio outputs. */
        NEXUS_AudioDecoder_GetDefaultStartSettings(&audioProgram);
        audioProgram.codec = AUDIO_CODEC;
        if(xcodeId==1) {
            audioProgram.codec = NEXUS_AudioCodec_eMpeg;
        }

        audioProgram.pidChannel = audioPidChannel[xcodeId];
        audioProgram.stcChannel = stcChannel[xcodeId];

#if NEXUS_NUM_HDMI_OUTPUTS
        if(xcodeId==0) {
            NEXUS_AudioOutput_AddInput(
                NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]),
                NEXUS_AudioDecoder_GetConnector(audioDecoder[0], NEXUS_AudioDecoderConnectorType_eStereo));
        }
#endif
        /* Open audio mixer.  The mixer can be left running at all times to provide continuous audio output despite input discontinuities.  */
        NEXUS_AudioMixer_GetDefaultSettings(&audioMixerSettings);
        audioMixerSettings.mixUsingDsp = true;
        audioMixer[xcodeId] = NEXUS_AudioMixer_Open(&audioMixerSettings);
        assert(audioMixer[xcodeId]);
        /* Connect audio mixers to outputs */
        NEXUS_AudioOutput_AddInput(
            NEXUS_AudioDummyOutput_GetConnector(platformConfig.outputs.audioDummy[xcodeId]),
            NEXUS_AudioMixer_GetConnector(audioMixer[xcodeId]));

        /* Open audio mux output */
        audioMuxOutput[xcodeId] = NEXUS_AudioMuxOutput_Create(NULL);
        assert(audioMuxOutput[xcodeId]);

#if BTST_ENABLE_AUDIO_ENCODE
        /* Open audio encoder */
        NEXUS_AudioEncoder_GetDefaultSettings(&encoderSettings);
        encoderSettings.codec = NEXUS_AudioCodec_eAacPlusLoas;
        audioCodec[xcodeId] = encoderSettings.codec;
        audioEncoder[xcodeId] = NEXUS_AudioEncoder_Open(&encoderSettings);
        assert(audioEncoder[xcodeId]);
        /* Connect decoder to mixer and set as master */
        NEXUS_AudioMixer_AddInput(audioMixer[xcodeId],
            NEXUS_AudioDecoder_GetConnector(audioDecoder[xcodeId], NEXUS_AudioDecoderConnectorType_eStereo));
        audioMixerSettings.master = NEXUS_AudioDecoder_GetConnector(audioDecoder[xcodeId], NEXUS_AudioDecoderConnectorType_eStereo);
        NEXUS_AudioMixer_SetSettings(audioMixer[xcodeId], &audioMixerSettings);
        /* Connect mixer to encoder */
        NEXUS_AudioEncoder_AddInput(audioEncoder[xcodeId], NEXUS_AudioMixer_GetConnector(audioMixer[xcodeId]));
        /* Connect mux to encoder */
        NEXUS_AudioOutput_AddInput(
            NEXUS_AudioMuxOutput_GetConnector(audioMuxOutput[xcodeId]), NEXUS_AudioEncoder_GetConnector(audioEncoder[xcodeId]));
#else
        NEXUS_AudioMixer_AddInput(audioMixer[xcodeId],
            NEXUS_AudioDecoder_GetConnector(audioPassthrough[xcodeId], NEXUS_AudioDecoderConnectorType_eCompressed));
        audioMixerSettings.master = NEXUS_AudioDecoder_GetConnector(audioPassthrough[xcodeId], NEXUS_AudioDecoderConnectorType_eCompressed);
        NEXUS_AudioMixer_SetSettings(audioMixer[xcodeId], &audioMixerSettings);
        NEXUS_AudioOutput_AddInput(
            NEXUS_AudioMuxOutput_GetConnector(audioMuxOutput[xcodeId]),
            NEXUS_AudioMixer_GetConnector(audioMixer[xcodeId]));

        audioCodec[xcodeId] = audioProgram.codec;
#endif
#endif

        /* bring up decoder and connect to local display */
        videoDecoder[xcodeId] = NEXUS_VideoDecoder_Open(xcodeId, NULL); /* take default capabilities */

        /* NOTE: must open video encoder before display; otherwise open will init ViCE2 core
         * which might cause encoder display GISB error since encoder display would
         * trigger RDC to program mailbox registers in ViCE2;
         */
#if BTST_HAS_VIDEO_ENCODE_TEST
        videoEncoder[xcodeId] = NEXUS_VideoEncoder_Open(xcodeId, NULL);
        assert(videoEncoder[xcodeId]);
#endif
        NEXUS_GetVideoEncoderCapabilities(&videoEncoderCap);

        /* Bring up video encoder display */
        NEXUS_Display_GetDefaultSettings(&displaySettings);
        displaySettings.displayType = NEXUS_DisplayType_eAuto;
        displaySettings.timingGenerator = NEXUS_DisplayTimingGenerator_eEncoder;
        displaySettings.format = cmdSettings[i].displayFormat;/* source is 60hz */
        displaySettings.frameRateMaster = NULL;/* disable frame rate tracking for now */

        displaySettings.format = cmdSettings[i].displayFormat;
        displayTranscode[xcodeId] = NEXUS_Display_Open(videoEncoderCap.videoEncoder[xcodeId].displayIndex, &displaySettings);/* cmp3 for transcoder */
        assert(displayTranscode[xcodeId]);

        windowTranscode[xcodeId] = NEXUS_VideoWindow_Open(displayTranscode[xcodeId], 0);
        assert(windowTranscode[xcodeId]);
        /* enable deinterlacer to improve quality */
        NEXUS_VideoWindow_GetMadSettings(windowTranscode[xcodeId], &windowMadSettings);
        NEXUS_VideoWindow_SetMadSettings(windowTranscode[xcodeId], &windowMadSettings);

        /* connect same decoder to the encoder display;
         * NOTE: simul display + transcode mode might have limitation in audio pathre;
         * here is for video transcode bringup purpose;
         */
        NEXUS_VideoWindow_AddInput(windowTranscode[xcodeId], NEXUS_VideoDecoder_GetConnector(videoDecoder[xcodeId]));
        if(xcodeId==0)
            NEXUS_VideoWindow_AddInput(window, NEXUS_VideoDecoder_GetConnector(videoDecoder[0]));

        /* Open the video pid channel */
        NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
        playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eVideo;
        playbackPidSettings.pidTypeSettings.video.codec = VIDEO_CODEC; /* must be told codec for correct handling */
        playbackPidSettings.pidTypeSettings.video.index = true;
        playbackPidSettings.pidTypeSettings.video.decoder = videoDecoder[xcodeId];

#if BTST_MPG_TRANSCODER_PASSTHRU /* cnnticker MPG */
        videoPidChannel[xcodeId] = NEXUS_Playback_OpenPidChannel(playback[xcodeId], VIDEO_PID, &playbackPidSettings);
#else
        videoPidChannel[xcodeId] = NEXUS_Playback_OpenPidChannel(playback[xcodeId], (xcodeId==1)? XCODE_VIDEO_PID:VIDEO_PID, &playbackPidSettings);
#endif

        /* Set up decoder Start structures now. We need to know the audio codec to properly set up the audio outputs. */
        NEXUS_VideoDecoder_GetDefaultStartSettings(&videoProgram);
        videoProgram.codec = VIDEO_CODEC;
        if(xcodeId==1) videoProgram.codec = NEXUS_VideoCodec_eMpeg2;
        videoProgram.pidChannel = videoPidChannel[xcodeId];
        videoProgram.stcChannel = stcChannel[xcodeId];

#if BTST_HAS_VIDEO_ENCODE_TEST
        NEXUS_VideoEncoder_GetSettings(videoEncoder[xcodeId], &videoEncoderConfig);
        videoEncoderConfig.variableFrameRate = true; /* encoder can detect film content and follow CET */
        videoEncoderConfig.frameRate = NEXUS_VideoFrameRate_e30;
        videoEncoderConfig.frameRate = cmdSettings[i].encoderFrameRate;
        videoEncoderConfig.bitrateMax = cmdSettings[i].encoderBitrate;
        videoEncoderConfig.streamStructure.framesP = cmdSettings[i].encoderGopStructureFramesP;
        videoEncoderConfig.streamStructure.framesB = cmdSettings[i].encoderGopStructureFramesB;

        NEXUS_VideoEncoder_GetDefaultStartSettings(&videoEncoderStartConfig);
        videoEncoderStartConfig.codec = cmdSettings[i].encoderVideoCodec;
        videoEncoderStartConfig.profile = cmdSettings[i].encoderProfile;
        videoEncoderStartConfig.level = cmdSettings[i].encoderLevel;
        videoEncoderStartConfig.input = displayTranscode[xcodeId];
        videoEncoderStartConfig.stcChannel = stcChannelTranscode[xcodeId];
        {
            NEXUS_VideoFormatInfo info;
            NEXUS_VideoFormat_GetInfo(displaySettings.format, &info);
            videoEncoderStartConfig.interlaced = info.interlaced;
        }

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
            unsigned Dee;
#endif
            /* NOTE: video encoder delay is in 27MHz ticks */
            NEXUS_VideoEncoder_GetDelayRange(videoEncoder[xcodeId], &videoEncoderConfig, &videoEncoderStartConfig, &videoDelay);
            printf("\n\tVideo encoder end-to-end delay = %u ms; maximum allowed: %u ms\n", videoDelay.min/27000, videoDelay.max/27000);

#if NEXUS_HAS_AUDIO_MUX_OUTPUT_TEST
            NEXUS_AudioMuxOutput_GetDelayStatus(audioMuxOutput[xcodeId], audioCodec[xcodeId], &audioDelayStatus);
            printf("\tAudio codec %d end-to-end delay = %u ms\n", audioCodec[xcodeId], audioDelayStatus.endToEndDelay);

            Dee = audioDelayStatus.endToEndDelay * 27000; /* in 27MHz ticks */
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
            NEXUS_AudioMuxOutput_GetDefaultStartSettings(&audioMuxStartSettings);
            audioMuxStartSettings.stcChannel = stcChannelTranscode[xcodeId];
            audioMuxStartSettings.presentationDelay = Dee/27000;/* in ms */
            NEXUS_AudioMuxOutput_Start(audioMuxOutput[xcodeId], &audioMuxStartSettings);
#else
            videoEncoderConfig.encoderDelay = videoDelay.min;
#endif
        }
        /* Note: video encoder SetSettings needs to be called after the encoder delay is determined; */
        NEXUS_VideoEncoder_SetSettings(videoEncoder[xcodeId], &videoEncoderConfig);

        NEXUS_Playpump_GetDefaultOpenSettings(&playpumpConfig);
        playpumpConfig.fifoSize = 16384; /* reduce FIFO size allocated for playpump */
        playpumpConfig.numDescriptors = 64; /* set number of descriptors */
        playpumpConfig.streamMuxCompatible = true;
        playpumpTranscodeVideo[xcodeId] = NEXUS_Playpump_Open(xcodeId*4 + 1, &playpumpConfig);
        assert(playpumpTranscodeVideo[xcodeId]);
#endif

#if NEXUS_HAS_AUDIO_MUX_OUTPUT_TEST
        NEXUS_Playpump_GetDefaultOpenSettings(&playpumpConfig);
        playpumpConfig.fifoSize = 16384; /* reduce FIFO size allocated for playpump */
        playpumpConfig.numDescriptors = 64; /* set number of descriptors */
        playpumpConfig.streamMuxCompatible = true;
        playpumpTranscodeAudio[xcodeId] = NEXUS_Playpump_Open(xcodeId*4+2, &playpumpConfig);
        assert(playpumpTranscodeAudio[xcodeId]);
#endif    
        playpumpTranscodePcr[xcodeId] = NEXUS_Playpump_Open(xcodeId*4+3, &playpumpConfig);
        assert(playpumpTranscodePcr[xcodeId]);

        BKNI_CreateEvent(&finishEvent[xcodeId]);
        NEXUS_StreamMux_GetDefaultCreateSettings(&muxCreateSettings);
        muxCreateSettings.finished.callback = transcoderFinishCallback;
        muxCreateSettings.finished.context = finishEvent[xcodeId];
        streamMux[xcodeId] = NEXUS_StreamMux_Create(&muxCreateSettings);
        NEXUS_StreamMux_GetDefaultStartSettings(&muxConfig);
        muxConfig.transportType = NEXUS_TransportType_eTs;
        muxConfig.stcChannel = stcChannelTranscode[xcodeId];

#if BTST_HAS_VIDEO_ENCODE_TEST
        muxConfig.video[0].pid = 0x11;
        muxConfig.video[0].encoder = videoEncoder[xcodeId];
        muxConfig.video[0].playpump = playpumpTranscodeVideo[xcodeId];
#endif    
#if NEXUS_HAS_AUDIO_MUX_OUTPUT_TEST
        muxConfig.audio[0].pid = 0x12;
        muxConfig.audio[0].muxOutput = audioMuxOutput[xcodeId];
        muxConfig.audio[0].playpump = playpumpTranscodeAudio[xcodeId];
#endif
        muxConfig.pcr.pid = 0x13;
        muxConfig.pcr.playpump = playpumpTranscodePcr[xcodeId];
        muxConfig.pcr.interval = 50;

        /* open PidChannels */
        pidChannelTranscodePcr[xcodeId] = NEXUS_Playpump_OpenPidChannel(playpumpTranscodePcr[xcodeId], muxConfig.pcr.pid, NULL);
        assert(pidChannelTranscodePcr[xcodeId]);
        pidChannelTranscodePmt[xcodeId] = NEXUS_Playpump_OpenPidChannel(playpumpTranscodePcr[xcodeId], BTST_PMT_PID, NULL);
        assert(pidChannelTranscodePmt[xcodeId]);
        pidChannelTranscodePat[xcodeId] = NEXUS_Playpump_OpenPidChannel(playpumpTranscodePcr[xcodeId], 0, NULL);
        assert(pidChannelTranscodePat[xcodeId]);

        /* start mux */
        NEXUS_StreamMux_Start(streamMux[xcodeId],&muxConfig, &muxOutput);
#if BTST_HAS_VIDEO_ENCODE_TEST
        pidChannelTranscodeVideo[xcodeId] = muxOutput.video[0];
#endif
#if NEXUS_HAS_AUDIO_MUX_OUTPUT_TEST
        pidChannelTranscodeAudio[xcodeId] = muxOutput.audio[0];
#endif

        recpump[xcodeId] = NEXUS_Recpump_Open(xcodeId, NULL);
        assert(recpump[xcodeId]);

        record[xcodeId] = NEXUS_Record_Create();
        assert(record[xcodeId]);

        NEXUS_Record_GetSettings(record[xcodeId], &recordSettings);
        recordSettings.recpump = recpump[xcodeId];
        NEXUS_Record_SetSettings(record[xcodeId], &recordSettings);

#if BTST_HAS_VIDEO_ENCODE_TEST
        /* configure the video pid for indexing */
        NEXUS_Record_GetDefaultPidChannelSettings(&recordPidSettings);
        recordPidSettings.recpumpSettings.pidType = NEXUS_PidType_eVideo;
        recordPidSettings.recpumpSettings.pidTypeSettings.video.index = true;
        recordPidSettings.recpumpSettings.pidTypeSettings.video.codec = cmdSettings[i].encoderVideoCodec;

        /* add multiplex data to the same record */
        NEXUS_Record_AddPidChannel(record[xcodeId], pidChannelTranscodeVideo[xcodeId], &recordPidSettings);
#endif
#if NEXUS_HAS_AUDIO_MUX_OUTPUT_TEST
        NEXUS_Record_AddPidChannel(record[xcodeId], pidChannelTranscodeAudio[xcodeId], NULL);
#endif
        NEXUS_Record_AddPidChannel(record[xcodeId], pidChannelTranscodePcr[xcodeId], NULL);
        NEXUS_Record_AddPidChannel(record[xcodeId], pidChannelTranscodePat[xcodeId], NULL);
        NEXUS_Record_AddPidChannel(record[xcodeId], pidChannelTranscodePmt[xcodeId], NULL);

    {
        char outStrmName[80], outNavName[80];
        sprintf(outStrmName, "videos/stream%i.mpg", xcodeId);
        sprintf(outNavName, "videos/stream%i.nav", xcodeId);
        fileTranscode[xcodeId] = NEXUS_FileRecord_OpenPosix(outStrmName, outNavName);
        assert(fileTranscode[xcodeId]);
    }
        /* Start record of stream mux output */
        NEXUS_Record_Start(record[xcodeId], fileTranscode[xcodeId]);

#if NEXUS_HAS_SYNC_CHANNEL
        /* connect sync channel */
        NEXUS_SyncChannel_GetSettings(syncChannel[xcodeId], &syncChannelSettings);
        syncChannelSettings.videoInput = NEXUS_VideoDecoder_GetConnector(videoDecoder[xcodeId]);
#if BTST_ENABLE_AUDIO_ENCODE 
        syncChannelSettings.audioInput[0] = NEXUS_AudioDecoder_GetConnector(audioDecoder[xcodeId], NEXUS_AudioDecoderConnectorType_eStereo);
#else
        syncChannelSettings.audioInput[0] = NEXUS_AudioDecoder_GetConnector(audioPassthrough[xcodeId], NEXUS_AudioDecoderConnectorType_eCompressed);
#endif
        NEXUS_SyncChannel_SetSettings(syncChannel[xcodeId], &syncChannelSettings);
#endif

        /* Start decoder */
        NEXUS_VideoDecoder_Start(videoDecoder[xcodeId], &videoProgram);
#if NEXUS_HAS_AUDIO_MUX_OUTPUT_TEST
#if BTST_ENABLE_AUDIO_ENCODE 
        NEXUS_AudioDecoder_Start(audioDecoder[xcodeId], &audioProgram);
#else
        NEXUS_AudioDecoder_Start(audioPassthrough[xcodeId], &audioProgram);
#endif
#endif

        /* Start playback */
        NEXUS_Playback_Start(playback[xcodeId], file[xcodeId], NULL);
        {
        /* Get CRC right to be playable by VLCplayer etc 3rd party SW */
        uint32_t uiCRC = CRC32_mpeg(0, (uint8_t *) s_auiTSPacket_PAT + 184 - (8+4*1), 8+4*1);
        fprintf(stderr, "PAT crc=%x\n", uiCRC);
        NEXUS_Memory_Allocate(188, NULL, &pat[xcodeId]);
        NEXUS_Memory_Allocate(188, NULL, &pmt[xcodeId]);
        BKNI_Memcpy(pat[xcodeId], s_auiTSPacket_PAT, sizeof(s_auiTSPacket_PAT));
        BKNI_Memcpy(pmt[xcodeId], s_auiTSPacket_PMT, sizeof(s_auiTSPacket_PMT));
        ((uint8_t*)pat[xcodeId])[184] = (uiCRC >> 24) & 0xFF;
        ((uint8_t*)pat[xcodeId])[185] = (uiCRC >> 16) & 0xFF;
        ((uint8_t*)pat[xcodeId])[186] = (uiCRC >>  8) & 0xFF;
        ((uint8_t*)pat[xcodeId])[187] = (uiCRC >>  0) & 0xFF;

        /* audio stream type */
        switch(audioCodec[xcodeId])
        {
            case NEXUS_AudioCodec_eMpeg:         ((uint8_t *) pmt[xcodeId])[184-5] = 0x4; break;
            case NEXUS_AudioCodec_eMp3:          ((uint8_t *) pmt[xcodeId])[184-5] = 0x4; break;
            case NEXUS_AudioCodec_eAacAdts:      ((uint8_t *) pmt[xcodeId])[184-5] = 0xf; break; /* ADTS */
            case NEXUS_AudioCodec_eAacPlusAdts:  ((uint8_t *) pmt[xcodeId])[184-5] = 0xf; break; /* ADTS */
            case NEXUS_AudioCodec_eAacLoas:      ((uint8_t *) pmt[xcodeId])[184-5] = 0x11; break;/* LOAS */
            case NEXUS_AudioCodec_eAacPlusLoas:  ((uint8_t *) pmt[xcodeId])[184-5] = 0x11; break;/* LOAS */
            case NEXUS_AudioCodec_eAc3:          ((uint8_t *) pmt[xcodeId])[184-5] = 0x81; break;
            default:
                BDBG_ERR(("Audio encoder codec %d is not supported!\n", audioCodec[xcodeId]));
        }

        /* A+V two ES */
        uiCRC = CRC32_mpeg(0, (uint8_t *) pmt[xcodeId] + 184 - (12+5*2), 12+5*2);
        fprintf(stderr, "PMT crc=%x\n", uiCRC);

        ((uint8_t*)pmt[xcodeId])[184] = (uiCRC >> 24) & 0xFF;
        ((uint8_t*)pmt[xcodeId])[185] = (uiCRC >> 16) & 0xFF;
        ((uint8_t*)pmt[xcodeId])[186] = (uiCRC >>  8) & 0xFF;
        ((uint8_t*)pmt[xcodeId])[187] = (uiCRC >>  0) & 0xFF;
        NEXUS_Memory_FlushCache(pat[xcodeId], sizeof(s_auiTSPacket_PAT));
        NEXUS_Memory_FlushCache(pmt[xcodeId], sizeof(s_auiTSPacket_PMT));
        BKNI_Memset(psi[xcodeId], 0, sizeof(psi[xcodeId]));
        psi[xcodeId][0].size = 188;
        psi[xcodeId][0].pData = pat[xcodeId];
        psi[xcodeId][0].timestampDelta = 0;
        psi[xcodeId][1].size = 188;
        psi[xcodeId][1].pData = pmt[xcodeId];
        psi[xcodeId][1].timestampDelta = 0;
        NEXUS_StreamMux_AddSystemDataBuffer(streamMux[xcodeId], &psi[xcodeId][0]);
        NEXUS_StreamMux_AddSystemDataBuffer(streamMux[xcodeId], &psi[xcodeId][1]);
        }
        NEXUS_VideoEncoder_Start(videoEncoder[xcodeId], &videoEncoderStartConfig);
    }

    /* Playback state machine is driven from inside Nexus. */
    printf("Press ENTER to continue; type 'q' to quit\n");
    key = getchar();

    /* Bring down system */
    for(xcodeId=0; xcodeId < 2; xcodeId++)
    {
        NEXUS_Playback_Stop(playback[xcodeId]);
        NEXUS_VideoDecoder_Stop(videoDecoder[xcodeId]);
#if NEXUS_HAS_AUDIO_MUX_OUTPUT_TEST
#if BTST_ENABLE_AUDIO_ENCODE 
        NEXUS_AudioDecoder_Stop(audioDecoder[xcodeId]);
#else
        NEXUS_AudioDecoder_Stop(audioPassthrough[xcodeId]);
#endif
        NEXUS_AudioMuxOutput_Stop(audioMuxOutput[xcodeId]);
#endif
#if NEXUS_HAS_SYNC_CHANNEL
        /* disconnect sync channel */
        NEXUS_SyncChannel_GetSettings(syncChannel[xcodeId], &syncChannelSettings);
        syncChannelSettings.videoInput = NULL;
        syncChannelSettings.audioInput[0] = NULL;
        syncChannelSettings.audioInput[1] = NULL;
        NEXUS_SyncChannel_SetSettings(syncChannel[xcodeId], &syncChannelSettings);
#endif

#if BTST_HAS_VIDEO_ENCODE_TEST
        NEXUS_VideoEncoder_Stop(videoEncoder[xcodeId], NULL);
#endif
        NEXUS_StreamMux_Finish(streamMux[xcodeId]);
        /* wait for the encoder buffer model's data to be drained */
        if(BKNI_WaitForEvent(finishEvent[xcodeId], (videoEncoderConfig.encoderDelay/27000)*2)!=BERR_SUCCESS) {
            fprintf(stderr, "TIMEOUT\n");
        }
        BKNI_DestroyEvent(finishEvent[xcodeId]);
        NEXUS_Record_Stop(record[xcodeId]);
        /* Note: remove all record PID channels before stream mux stop since streammux would close the A/V PID channels */
        NEXUS_Record_RemoveAllPidChannels(record[xcodeId]);
        NEXUS_StreamMux_Stop(streamMux[xcodeId]);


        NEXUS_Record_Destroy(record[xcodeId]);
        NEXUS_Recpump_Close(recpump[xcodeId]);
        NEXUS_FileRecord_Close(fileTranscode[xcodeId]);

        NEXUS_Playback_ClosePidChannel(playback[xcodeId], audioPidChannel[xcodeId]);
        NEXUS_Playback_ClosePidChannel(playback[xcodeId], videoPidChannel[xcodeId]);
        NEXUS_Playpump_ClosePidChannel(playpumpTranscodePcr[xcodeId], pidChannelTranscodePcr[xcodeId]);
        NEXUS_Playpump_ClosePidChannel(playpumpTranscodePcr[xcodeId], pidChannelTranscodePat[xcodeId]);
        NEXUS_Playpump_ClosePidChannel(playpumpTranscodePcr[xcodeId], pidChannelTranscodePmt[xcodeId]);
        NEXUS_FilePlay_Close(file[xcodeId]);
        NEXUS_Playback_Destroy(playback[xcodeId]);
        NEXUS_Playpump_Close(playpump[xcodeId]);

        if(xcodeId==0) {
            NEXUS_VideoWindow_RemoveInput(window, NEXUS_VideoDecoder_GetConnector(videoDecoder[0]));
            NEXUS_VideoWindow_Close(window);
            NEXUS_Display_Close(display);
        }
        NEXUS_VideoWindow_RemoveInput(windowTranscode[xcodeId], NEXUS_VideoDecoder_GetConnector(videoDecoder[xcodeId]));
        NEXUS_VideoInput_Shutdown(NEXUS_VideoDecoder_GetConnector(videoDecoder[xcodeId]));
        NEXUS_VideoDecoder_Close(videoDecoder[xcodeId]);
        NEXUS_VideoWindow_Close(windowTranscode[xcodeId]);
        NEXUS_Display_Close(displayTranscode[xcodeId]);

        NEXUS_StreamMux_Destroy(streamMux[xcodeId]);

        NEXUS_Playpump_Close(playpumpTranscodePcr[xcodeId]);

#if BTST_HAS_VIDEO_ENCODE_TEST
        NEXUS_Playpump_Close(playpumpTranscodeVideo[xcodeId]);
        NEXUS_VideoEncoder_Close(videoEncoder[xcodeId]);
#endif    

#if NEXUS_HAS_AUDIO_MUX_OUTPUT_TEST
#if BTST_ENABLE_AUDIO_ENCODE 
        NEXUS_AudioOutput_RemoveAllInputs(NEXUS_AudioMuxOutput_GetConnector(audioMuxOutput[xcodeId]));
        NEXUS_AudioEncoder_RemoveAllInputs(audioEncoder[xcodeId]);
        NEXUS_AudioInput_Shutdown(NEXUS_AudioEncoder_GetConnector(audioEncoder[xcodeId]));
        NEXUS_AudioEncoder_Close(audioEncoder[xcodeId]);
#if NEXUS_NUM_HDMI_OUTPUTS
        if(xcodeId==0) {
            NEXUS_AudioOutput_RemoveAllInputs(NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]));
        }
#endif
#else    
        NEXUS_AudioOutput_RemoveAllInputs(NEXUS_AudioMuxOutput_GetConnector(audioMuxOutput[xcodeId]));
#endif
        NEXUS_AudioOutput_RemoveAllInputs(NEXUS_AudioDummyOutput_GetConnector(platformConfig.outputs.audioDummy[xcodeId]));

        NEXUS_AudioMixer_RemoveAllInputs(audioMixer[xcodeId]);
        NEXUS_AudioInput_Shutdown(NEXUS_AudioMixer_GetConnector(audioMixer[xcodeId]));
        NEXUS_AudioMixer_Close(audioMixer[xcodeId]);

        NEXUS_Playpump_Close(playpumpTranscodeAudio[xcodeId]);
        NEXUS_AudioOutput_Shutdown(NEXUS_AudioMuxOutput_GetConnector(audioMuxOutput[xcodeId]));
        NEXUS_AudioMuxOutput_Destroy(audioMuxOutput[xcodeId]);

#if BTST_ENABLE_AUDIO_ENCODE 
        NEXUS_AudioInput_Shutdown(NEXUS_AudioDecoder_GetConnector(audioDecoder[xcodeId], NEXUS_AudioDecoderConnectorType_eStereo));
        NEXUS_AudioDecoder_Close(audioDecoder[xcodeId]);
#else
        NEXUS_AudioInput_Shutdown(NEXUS_AudioDecoder_GetConnector(audioPassthrough[xcodeId], NEXUS_AudioDecoderConnectorType_eCompressed));
        NEXUS_AudioDecoder_Close(audioPassthrough[xcodeId]);
#endif
        NEXUS_AudioOutput_Shutdown(NEXUS_AudioDummyOutput_GetConnector(platformConfig.outputs.audioDummy[xcodeId]));
#endif

#if NEXUS_HAS_SYNC_CHANNEL
        NEXUS_SyncChannel_Destroy(syncChannel[xcodeId]);
#endif

        NEXUS_StcChannel_Close(stcChannel[xcodeId]);
        NEXUS_StcChannel_Close(stcChannelTranscode[xcodeId]);
        NEXUS_Memory_Free(pat[xcodeId]);
        NEXUS_Memory_Free(pmt[xcodeId]);
    }
    
    if(key != 'q')
    {
        i = iteration++%TEST_ITERATIONS;
        BDBG_WRN(("Start %d iteration.....", iteration));
        goto again;
    }

    NEXUS_Platform_Uninit();

    return 0;
}
#else /* NEXUS_HAS_PLAYBACK && NEXUS_HAS_STREAM_MUX && (NEXUS_NUM_VIDEO_ENCODERS >= 2) */
int main(void)
{
    printf("This application is not supported on this platform!\n");
    return 0;
}
#endif
