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

#if NEXUS_NUM_HDMI_INPUTS && NEXUS_HAS_STREAM_MUX
#include "nexus_video_decoder.h"
#include "nexus_stc_channel.h"
#include "nexus_display.h"
#include "nexus_video_window.h"
#include "nexus_video_input.h"
#include "nexus_spdif_output.h"
#include "nexus_component_output.h"
#if NEXUS_HAS_HDMI_OUTPUT
#include "nexus_hdmi_output.h"
#endif
#if NEXUS_NUM_HDMI_INPUTS && NEXUS_HAS_STREAM_MUX
#include "nexus_playback.h"
#include "nexus_record.h"
#include "nexus_file.h"
#include "nexus_video_encoder.h"
#include "nexus_stream_mux.h"
#include "nexus_recpump.h"
#include "nexus_record.h"
#include "nexus_audio_mux_output.h"
#include "nexus_audio_encoder.h"
#include "nexus_audio_mixer.h"
#include "nexus_audio_decoder.h"
#if NEXUS_HAS_SYNC_CHANNEL
#include "nexus_sync_channel.h"
#endif
#endif
#if NEXUS_NUM_HDMI_INPUTS
#include "nexus_hdmi_input.h"
#endif

#include <assert.h>
#include "bstd.h"
#include "bkni.h"

#define BTST_ENABLE_SYNC_CHANNEL    0

BDBG_MODULE(transcode_hdmi_to_ts);

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

void source_changed(void *context, int param)
{
    BSTD_UNUSED(context);
    BSTD_UNUSED(param);
    printf("source changed\n");
}

void avmute_changed(void *context, int param)
{
    BSTD_UNUSED(context);
    BSTD_UNUSED(param);
    printf("avmute changed\n");
}

void transcoderFinishCallback(void *context, int param)
{
	BKNI_EventHandle finishEvent = (BKNI_EventHandle)context;

	BSTD_UNUSED(param);
	BDBG_WRN(("Finish callback invoked, now stop the stream mux."));
	BKNI_SetEvent(finishEvent);
}

int main(void)
{
    NEXUS_PlatformSettings platformSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_StcChannelHandle stcChannel;
    NEXUS_StcChannelSettings stcSettings;
    NEXUS_DisplayHandle display;
    NEXUS_DisplaySettings displaySettings;
    NEXUS_VideoWindowHandle window;
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_HdmiOutputStatus hdmiStatus;
    NEXUS_Error rc;
#endif

    NEXUS_DisplayHandle displayTranscode;
    NEXUS_VideoWindowHandle windowTranscode;
    NEXUS_VideoEncoderCapabilities videoEncoderCap;
    NEXUS_VideoEncoderHandle videoEncoder;
    NEXUS_VideoEncoderSettings videoEncoderConfig;
    NEXUS_VideoEncoderStartSettings videoEncoderStartConfig;

    BKNI_EventHandle finishEvent;
    NEXUS_StreamMuxCreateSettings muxCreateSettings;
    NEXUS_StreamMuxHandle streamMux;
    NEXUS_StreamMuxStartSettings muxConfig;
    NEXUS_StreamMuxOutput muxOutput;
    NEXUS_PlaypumpOpenSettings playpumpConfig;
    NEXUS_PlaypumpHandle playpumpTranscodeVideo;
    NEXUS_PlaypumpHandle playpumpTranscodePcr;
    NEXUS_FileRecordHandle fileTranscode;
    NEXUS_StcChannelHandle stcChannelTranscode;
    NEXUS_VideoEncoderDelayRange videoDelay;

    NEXUS_RecpumpHandle recpump;
    NEXUS_RecordHandle record;
    NEXUS_RecordSettings recordSettings;
    NEXUS_RecordPidChannelSettings recordPidSettings;
    NEXUS_PidChannelHandle pidChannelTranscodeVideo;
    NEXUS_PidChannelHandle pidChannelTranscodePcr;
    NEXUS_PidChannelHandle pidChannelTranscodePat;
    NEXUS_PidChannelHandle pidChannelTranscodePmt;

    NEXUS_HdmiInputHandle hdmiInput;
    NEXUS_HdmiInputSettings hdmiInputSettings;
    NEXUS_TimebaseSettings timebaseSettings;

    NEXUS_AudioDecoderHandle audioDecoder;
    NEXUS_AudioDecoderStartSettings audioProgram;
    NEXUS_PlaypumpHandle playpumpTranscodeAudio;
    NEXUS_PidChannelHandle pidChannelTranscodeAudio;

    NEXUS_AudioMixerSettings audioMixerSettings;
    NEXUS_AudioMixerHandle audioMixer;
    NEXUS_AudioMuxOutputDelayStatus audioDelayStatus;
    NEXUS_AudioMuxOutputHandle audioMuxOutput;
    NEXUS_AudioMuxOutputStartSettings audioMuxStartSettings;
    NEXUS_AudioEncoderSettings encoderSettings;
    NEXUS_AudioEncoderHandle audioEncoder;
#if NEXUS_HAS_SYNC_CHANNEL
    NEXUS_SyncChannelSettings syncChannelSettings;
    NEXUS_SyncChannelHandle syncChannel;
#endif
    void *pat;
    void *pmt;

    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init(&platformSettings);
    NEXUS_Platform_GetConfiguration(&platformConfig);

#if NEXUS_HAS_SYNC_CHANNEL
	/* create a sync channel */
	NEXUS_SyncChannel_GetDefaultSettings(&syncChannelSettings);
	syncChannel = NEXUS_SyncChannel_Create(&syncChannelSettings);
#endif

    /**********************************************************
     * Open HDMI INPUT which also drives timebase for display/xcode 
     */
    NEXUS_Timebase_GetSettings(NEXUS_Timebase_e0, &timebaseSettings);
    timebaseSettings.sourceType = NEXUS_TimebaseSourceType_eHdDviIn;
    NEXUS_Timebase_SetSettings(NEXUS_Timebase_e0, &timebaseSettings);

    NEXUS_HdmiInput_GetDefaultSettings(&hdmiInputSettings);
    hdmiInputSettings.timebase = NEXUS_Timebase_e0;
    hdmiInputSettings.sourceChanged.callback = source_changed;
    hdmiInputSettings.avMuteChanged.callback = avmute_changed;
    hdmiInput = NEXUS_HdmiInput_Open(0, &hdmiInputSettings);
    if(!hdmiInput) {
        fprintf(stderr, "Can't get hdmi input\n");
        return -1;
    }

    /********************************************
     * Set up hdmi input audio xcode path
     */
    /* Open the hdmi input audio decoder */
    audioDecoder = NEXUS_AudioDecoder_Open(0, NULL);

    /* Set up decoder Start structures now. audio from HDMI input. */
    NEXUS_AudioDecoder_GetDefaultStartSettings(&audioProgram);
    audioProgram.input = NEXUS_HdmiInput_GetAudioConnector(hdmiInput);

    /* connect hdmi input STC channel 0 */
    NEXUS_StcChannel_GetDefaultSettings(0, &stcSettings);
    stcSettings.timebase = NEXUS_Timebase_e0;
    stcSettings.autoConfigTimebase = false;
    stcChannel = NEXUS_StcChannel_Open(0, &stcSettings);
    audioProgram.stcChannel = stcChannel;

    /* Connect audio decoders to outputs */
#if NEXUS_NUM_AUDIO_DACS
    NEXUS_AudioOutput_AddInput(
        NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]),
        NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
#endif
#if NEXUS_NUM_HDMI_OUTPUTS
	/* add hdmi pcm audio output to loopback display */
	NEXUS_AudioOutput_AddInput(
		NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]),
		NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
#endif

    /* Open audio mixer.  The mixer can be left running at all times to provide continuous audio output despite input discontinuities.  */
    NEXUS_AudioMixer_GetDefaultSettings(&audioMixerSettings);
    audioMixerSettings.mixUsingDsp = true;
    audioMixer = NEXUS_AudioMixer_Open(&audioMixerSettings);
    assert(audioMixer);
    NEXUS_AudioOutput_AddInput(
        NEXUS_AudioDummyOutput_GetConnector(platformConfig.outputs.audioDummy[0]), NEXUS_AudioMixer_GetConnector(audioMixer));

    /* Open audio mux output */
    audioMuxOutput = NEXUS_AudioMuxOutput_Create(NULL);
    assert(audioMuxOutput);

    /* Open audio encoder */
    NEXUS_AudioEncoder_GetDefaultSettings(&encoderSettings);
    encoderSettings.codec = NEXUS_AudioCodec_eAac;
    audioEncoder = NEXUS_AudioEncoder_Open(&encoderSettings);
    assert(audioEncoder);

    /* Connect decoder to mixer and set as master */
    NEXUS_AudioMixer_AddInput(audioMixer,
        NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
    audioMixerSettings.master = NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo);
    NEXUS_AudioMixer_SetSettings(audioMixer, &audioMixerSettings);

    /* Connect mixer to encoder */
    NEXUS_AudioEncoder_AddInput(audioEncoder, NEXUS_AudioMixer_GetConnector(audioMixer));

    /* Connect mux to encoder */
    NEXUS_AudioOutput_AddInput(
        NEXUS_AudioMuxOutput_GetConnector(audioMuxOutput), NEXUS_AudioEncoder_GetConnector(audioEncoder));

    /***********************************************8
     * Open transcoder STC channel 1 (binary format) 
     */
    NEXUS_StcChannel_GetDefaultSettings(1, &stcSettings);
    stcSettings.timebase = NEXUS_Timebase_e0;
    stcSettings.mode = NEXUS_StcChannelMode_eAuto;
    stcSettings.pcrBits = NEXUS_StcChannel_PcrBits_eFull42;/* 42-bit binary broadcast for encoder */
    stcChannelTranscode = NEXUS_StcChannel_Open(1, &stcSettings);

    /**********************************************
     * Bring up local video display and outputs for source
     */
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

    /*************************************
     * open video encoder
     */
    /* NOTE: must open video encoder before display; otherwise open will init ViCE2 core
     * which might cause encoder display GISB error since encoder display would
     * trigger RDC to program mailbox registers in ViCE2;
     */
    videoEncoder = NEXUS_VideoEncoder_Open(0, NULL);
    assert(videoEncoder);
    NEXUS_GetVideoEncoderCapabilities(&videoEncoderCap);
    
    /*************************************
     * Bring up video encoder display 
     */
    NEXUS_Display_GetDefaultSettings(&displaySettings);
    displaySettings.displayType = NEXUS_DisplayType_eAuto;
    displaySettings.timingGenerator = NEXUS_DisplayTimingGenerator_eEncoder;
    displaySettings.format = NEXUS_VideoFormat_e480p;/* bring up 480p first */
    displayTranscode = NEXUS_Display_Open(videoEncoderCap.videoEncoder[0].displayIndex, &displaySettings);/* cmp3 for transcoder */
    assert(displayTranscode);

    windowTranscode = NEXUS_VideoWindow_Open(displayTranscode, 0);
    assert(windowTranscode);

    /* connect same decoder to the encoder display;
        * NOTE: simul display + transcode mode might have limitation in audio pathre;
        * here is for video transcode bringup purpose;
        */
    NEXUS_VideoWindow_AddInput(windowTranscode, NEXUS_HdmiInput_GetVideoConnector(hdmiInput));
    NEXUS_VideoWindow_AddInput(window, NEXUS_HdmiInput_GetVideoConnector(hdmiInput));

    NEXUS_VideoEncoder_GetSettings(videoEncoder, &videoEncoderConfig);
    videoEncoderConfig.variableFrameRate = true;
    videoEncoderConfig.frameRate = NEXUS_VideoFrameRate_e24;
    videoEncoderConfig.bitrateMax = 6*1000*1000;
    videoEncoderConfig.streamStructure.framesP = 29;
    videoEncoderConfig.streamStructure.framesB = 0;

    NEXUS_VideoEncoder_GetDefaultStartSettings(&videoEncoderStartConfig);
    videoEncoderStartConfig.codec = NEXUS_VideoCodec_eH264;
    videoEncoderStartConfig.profile = NEXUS_VideoCodecProfile_eBaseline;
    videoEncoderStartConfig.level = NEXUS_VideoCodecLevel_e31;
    videoEncoderStartConfig.input = displayTranscode;
    videoEncoderStartConfig.stcChannel = stcChannelTranscode;/* xcode stc */

	/************************************
	 * Set up encoder AV sync.
	 * encode setting and startSetting to be set after end-to-end delay is determined */
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
		unsigned Dee;

		/* NOTE: video encoder delay is in 27MHz ticks */
		NEXUS_VideoEncoder_GetDelayRange(videoEncoder, &videoEncoderConfig, &videoEncoderStartConfig, &videoDelay);
		BDBG_WRN(("\n\tVideo encoder end-to-end delay = [%u ~ %u] ms", videoDelay.min/27000, videoDelay.max/27000));

		NEXUS_AudioMuxOutput_GetDelayStatus(audioMuxOutput, encoderSettings.codec, &audioDelayStatus);
		BDBG_WRN(("\n\tAudio codec %d end-to-end delay = %u ms", encoderSettings.codec, audioDelayStatus.endToEndDelay));

		Dee = audioDelayStatus.endToEndDelay * 27000; /* in 27MHz ticks */
		if(Dee > videoDelay.min)
		{
			if(Dee > videoDelay.max)
			{
				BDBG_ERR(("Audio Dee is way too big! Use video Dee max!"));
				Dee = videoDelay.max;
			}
			else
			{
				BDBG_WRN(("Use audio Dee %u ms %u ticks@27Mhz!", Dee/27000, Dee));
			}
		}
		else
		{
			Dee = videoDelay.min;
			BDBG_WRN(("Use video Dee %u ms %u ticks@27Mhz!", Dee/27000, Dee));
		}
		videoEncoderConfig.encoderDelay = Dee;

		/* Start audio mux output */
		NEXUS_AudioMuxOutput_GetDefaultStartSettings(&audioMuxStartSettings);
		audioMuxStartSettings.stcChannel = stcChannelTranscode;/* xcode stc */
		audioMuxStartSettings.presentationDelay = Dee/27000;/* in ms */
		NEXUS_AudioMuxOutput_Start(audioMuxOutput, &audioMuxStartSettings);
	}

    /***********************************************
     * start video encoder after end-to-end delay is set
     */
    NEXUS_VideoEncoder_SetSettings(videoEncoder, &videoEncoderConfig);
    NEXUS_VideoEncoder_Start(videoEncoder, &videoEncoderStartConfig);

    /***********************************************
     * Open stream mux playpumps for audio/video/system data
     */
    NEXUS_Playpump_GetDefaultOpenSettings(&playpumpConfig);
    playpumpConfig.fifoSize = 16384; /* reduce FIFO size allocated for playpump */
    playpumpConfig.numDescriptors = 64; /* set number of descriptors */
    playpumpConfig.streamMuxCompatible = true;
    playpumpTranscodeVideo = NEXUS_Playpump_Open(1, &playpumpConfig);
    assert(playpumpTranscodeVideo);

    /* audio playpump here is for ts muxer */
    NEXUS_Playpump_GetDefaultOpenSettings(&playpumpConfig);
    playpumpConfig.fifoSize = 16384; /* reduce FIFO size allocated for playpump */
    playpumpConfig.numDescriptors = 64; /* set number of descriptors */
    playpumpConfig.streamMuxCompatible = true;
    playpumpTranscodeAudio = NEXUS_Playpump_Open(2, &playpumpConfig);
    assert(playpumpTranscodeAudio);

    playpumpTranscodePcr = NEXUS_Playpump_Open(3, &playpumpConfig);
    assert(playpumpTranscodePcr);

	BKNI_CreateEvent(&finishEvent);
	NEXUS_StreamMux_GetDefaultCreateSettings(&muxCreateSettings);
	muxCreateSettings.finished.callback = transcoderFinishCallback;
	muxCreateSettings.finished.context = finishEvent;
	streamMux = NEXUS_StreamMux_Create(&muxCreateSettings);
	NEXUS_StreamMux_GetDefaultStartSettings(&muxConfig);
    NEXUS_StreamMux_GetDefaultStartSettings(&muxConfig);
    muxConfig.transportType = NEXUS_TransportType_eTs;
    muxConfig.stcChannel = stcChannelTranscode;/* xcode stc */

    muxConfig.video[0].pid = 0x11;
    muxConfig.video[0].encoder = videoEncoder;
    muxConfig.video[0].playpump = playpumpTranscodeVideo;

    muxConfig.audio[0].pid = 0x12;
    muxConfig.audio[0].muxOutput = audioMuxOutput;
    muxConfig.audio[0].playpump = playpumpTranscodeAudio;

    muxConfig.pcr.pid = 0x13;
    muxConfig.pcr.playpump = playpumpTranscodePcr;
    muxConfig.pcr.interval = 50;

    /* open PidChannels for system data */
    pidChannelTranscodePcr = NEXUS_Playpump_OpenPidChannel(playpumpTranscodePcr, muxConfig.pcr.pid, NULL);
    assert(pidChannelTranscodePcr);
    pidChannelTranscodePmt = NEXUS_Playpump_OpenPidChannel(playpumpTranscodePcr, 0, NULL);
    assert(pidChannelTranscodePmt);
    pidChannelTranscodePat = NEXUS_Playpump_OpenPidChannel(playpumpTranscodePcr, 1, NULL);
    assert(pidChannelTranscodePat);

    /***********************************
     * start mux 
     */
    NEXUS_StreamMux_Start(streamMux,&muxConfig, &muxOutput);
    pidChannelTranscodeVideo = muxOutput.video[0];
    pidChannelTranscodeAudio = muxOutput.audio[0];

    /******************************************8
     * Set up xcoder record output
     */
    recpump = NEXUS_Recpump_Open(0, NULL);
    assert(recpump);

    record = NEXUS_Record_Create();
    assert(record);

    NEXUS_Record_GetSettings(record, &recordSettings);
    recordSettings.recpump = recpump;
    NEXUS_Record_SetSettings(record, &recordSettings);


	/* configure the video pid for indexing */
	NEXUS_Record_GetDefaultPidChannelSettings(&recordPidSettings);
	recordPidSettings.recpumpSettings.pidType = NEXUS_PidType_eVideo;
	recordPidSettings.recpumpSettings.pidTypeSettings.video.index = true;
	recordPidSettings.recpumpSettings.pidTypeSettings.video.codec = videoEncoderStartConfig.codec;

    /* add multiplex data to the same record */
    NEXUS_Record_AddPidChannel(record, pidChannelTranscodeVideo, NULL);
    NEXUS_Record_AddPidChannel(record, pidChannelTranscodeAudio, NULL);
    NEXUS_Record_AddPidChannel(record, pidChannelTranscodePcr, NULL);
    NEXUS_Record_AddPidChannel(record, pidChannelTranscodePat, NULL);
    NEXUS_Record_AddPidChannel(record, pidChannelTranscodePmt, NULL);

    fprintf(stderr, "\n\t transcode hdmi to TS file: videos/hdmi.ts\n\n");
    fileTranscode = NEXUS_FileRecord_OpenPosix("videos/hdmi.ts", "videos/hdmi.nav");
    assert(fileTranscode);

    /* Start record of stream mux output */
    NEXUS_Record_Start(record, fileTranscode);

#if NEXUS_HAS_SYNC_CHANNEL && BTST_ENABLE_SYNC_CHANNEL
	/* connect sync channel */
	NEXUS_SyncChannel_GetSettings(syncChannel, &syncChannelSettings);
	syncChannelSettings.videoInput = NEXUS_HdmiInput_GetVideoConnector(hdmiInput);
	syncChannelSettings.audioInput[0] = NEXUS_HdmiInput_GetAudioConnector(hdmiInput);

	NEXUS_SyncChannel_SetSettings(syncChannel, &syncChannelSettings);
#endif

    /* start mixer */
    NEXUS_AudioMixer_Start(audioMixer);

    /* Start audio decoders */
    NEXUS_AudioDecoder_Start(audioDecoder, &audioProgram);

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
   0x00,0xc1,0x00,0x00,0x00,0x01,0xe0,0x01,
   0x65,0x80,0x5e,0xdc,
};
#define BTST_PMT_PID 0x0001
#define BTST_PMT_TRANSPORT_IDX BTST_SYSTEM_TRANSPORT_IDX
static const uint8_t s_auiTSPacket_PMT[188] =
{
   0x47,0x40,0x01,0x30,0xa1,0x40,0xff,0xff,
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
   0x11,0xf0,0x00,0x0f,0xe0,0x12,0xf0,0x00,
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

        /* audio stream type */
		switch(encoderSettings.codec)
		{
			case NEXUS_AudioCodec_eMpeg:         ((uint8_t *) pmt)[184-5] = 0x4; break;
			case NEXUS_AudioCodec_eMp3:          ((uint8_t *) pmt)[184-5] = 0x4; break;
			case NEXUS_AudioCodec_eAac    :      ((uint8_t *) pmt)[184-5] = 0xf; break; /* ADTS */
			case NEXUS_AudioCodec_eAacPlus:      ((uint8_t *) pmt)[184-5] = 0x11; break;/* LOAS */
			/* MP2TS doesn't allow 14496-3 AAC+ADTS; here is placeholder to test AAC-HE before LOAS encode is supported; */
			case NEXUS_AudioCodec_eAacPlusAdts:  ((uint8_t *) pmt)[184-5] = 0x11; break;
			case NEXUS_AudioCodec_eAc3:          ((uint8_t *) pmt)[184-5] = 0x81; break;
			default:
				BDBG_ERR(("Audio encoder codec %d is not supported!\n", encoderSettings.codec));
		}

        /* A+V two ES */
        uiCRC = CRC32_mpeg(0, (uint8_t *) pmt + 184 - (12+5*2), 12+5*2);
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

    /* Playback state machine is driven from inside Nexus. */
    printf("Press ENTER to quit\n");
    getchar();

    /************************************
     * Bring down system 
     */

    /**************************************************
     * NOTE: stop sequence should be in front->back order
     */
    NEXUS_AudioDecoder_Stop(audioDecoder);
    NEXUS_AudioMixer_Stop(audioMixer);
    NEXUS_AudioMuxOutput_Stop(audioMuxOutput);

#if NEXUS_HAS_SYNC_CHANNEL
    /* disconnect sync channel after decoders stop */
    NEXUS_SyncChannel_GetSettings(syncChannel, &syncChannelSettings);
    syncChannelSettings.videoInput = NULL;
    syncChannelSettings.audioInput[0] = NULL;
    syncChannelSettings.audioInput[1] = NULL;
    NEXUS_SyncChannel_SetSettings(syncChannel, &syncChannelSettings);
#endif

    NEXUS_VideoEncoder_Stop(videoEncoder, NULL);

    /* finish stream mux to clean shutdown */
    NEXUS_StreamMux_Finish(streamMux);
    if(BKNI_WaitForEvent(finishEvent, 2000)!=BERR_SUCCESS) {
        fprintf(stderr, "TIMEOUT\n");
    }
    BKNI_DestroyEvent(finishEvent);
    NEXUS_Record_Stop(record);
    NEXUS_FileRecord_Close(fileTranscode);
    /*****************************************
     * Note: remove all record PID channels before stream 
     * mux stop since streammux would close the A/V PID channels 
     */
    NEXUS_Record_RemoveAllPidChannels(record);
    NEXUS_StreamMux_Stop(streamMux);
    NEXUS_Record_Destroy(record);
    NEXUS_Recpump_Close(recpump);

    NEXUS_Playpump_ClosePidChannel(playpumpTranscodePcr, pidChannelTranscodePcr);
    NEXUS_Playpump_ClosePidChannel(playpumpTranscodePcr, pidChannelTranscodePat);
    NEXUS_Playpump_ClosePidChannel(playpumpTranscodePcr, pidChannelTranscodePmt);
    
    NEXUS_AudioOutput_RemoveAllInputs(NEXUS_AudioMuxOutput_GetConnector(audioMuxOutput));
    NEXUS_AudioEncoder_RemoveAllInputs(audioEncoder);
    NEXUS_AudioInput_Shutdown(NEXUS_AudioEncoder_GetConnector(audioEncoder));
    NEXUS_AudioOutput_Shutdown(NEXUS_AudioMuxOutput_GetConnector(audioMuxOutput));
    NEXUS_AudioEncoder_Close(audioEncoder);

#if NEXUS_NUM_AUDIO_DACS
    NEXUS_AudioOutput_RemoveAllInputs(NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]));
#endif
#if NEXUS_NUM_SPDIF_OUTPUTS
    NEXUS_AudioOutput_RemoveAllInputs(NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]));
#endif
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_AudioOutput_RemoveAllInputs(NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]));
#endif

    NEXUS_Playpump_Close(playpumpTranscodeAudio);
    NEXUS_AudioMuxOutput_Destroy(audioMuxOutput);
    NEXUS_AudioOutput_RemoveAllInputs(NEXUS_AudioDummyOutput_GetConnector(platformConfig.outputs.audioDummy[0]));
    NEXUS_AudioOutput_Shutdown(NEXUS_AudioDummyOutput_GetConnector(platformConfig.outputs.audioDummy[0]));
    NEXUS_AudioMixer_RemoveAllInputs(audioMixer);
    NEXUS_AudioInput_Shutdown(NEXUS_AudioMixer_GetConnector(audioMixer));
    NEXUS_AudioMixer_Close(audioMixer);

    NEXUS_AudioInput_Shutdown(NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
#if NEXUS_NUM_AUDIO_DACS
    NEXUS_AudioOutput_Shutdown(NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]));
#endif
    NEXUS_AudioDecoder_Close(audioDecoder);

    NEXUS_VideoInput_Shutdown(NEXUS_HdmiInput_GetVideoConnector(hdmiInput));
    NEXUS_VideoWindow_Close(window);
    NEXUS_Display_Close(display);
    
    NEXUS_VideoWindow_Close(windowTranscode);
    NEXUS_Display_Close(displayTranscode);
    NEXUS_StreamMux_Destroy(streamMux);
    NEXUS_Playpump_Close(playpumpTranscodeVideo);
    NEXUS_Playpump_Close(playpumpTranscodePcr);
    NEXUS_VideoEncoder_Close(videoEncoder);

#if NEXUS_HAS_SYNC_CHANNEL
    NEXUS_SyncChannel_Destroy(syncChannel);
#endif

    NEXUS_StcChannel_Close(stcChannel);
    NEXUS_StcChannel_Close(stcChannelTranscode);
    NEXUS_Memory_Free(pat);
    NEXUS_Memory_Free(pmt);

    NEXUS_HdmiInput_Close(hdmiInput);
    NEXUS_Platform_Uninit();

    return 0;
}
#else /* NEXUS_NUM_HDMI_INPUTS && NEXUS_HAS_STREAM_MUX */
int main(void)
{
    printf("This application is not supported on this platform!\n");
    return 0;
}
#endif
