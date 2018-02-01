/******************************************************************************
 *  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.
 ******************************************************************************/
/* Nexus example app: playback and decode */

#include "nexus_platform.h"
#include <stdio.h>

#if NEXUS_HAS_PLAYBACK && NEXUS_HAS_STREAM_MUX
#include "nexus_stc_channel.h"
#include "nexus_spdif_output.h"
#include "nexus_playback.h"
#include "nexus_record.h"
#include "nexus_file.h"
#include "nexus_audio_encoder.h"
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

#define NEXUS_HAS_AUDIO_MUX_OUTPUT_TEST 1
#define BTST_ENABLE_AUDIO_ENCODE 1

/* Choose one of the following: */
#define BTST_AC3_TRANSCODER_PASSTHRU    1
#define BTST_MPG_TRANSCODER_PASSTHRU    0
#define BTST_AAC_TRANSCODER_PASSTHRU    0

/* the following define the input file and its characteristics -- these will vary by input file */
#define TRANSPORT_TYPE NEXUS_TransportType_eTs
#if BTST_AC3_TRANSCODER_PASSTHRU /* AC3 */
  #define FILE_NAME "videos/avatar_AVC_15M.ts"/*"videos/WildChina_Short.ts"*/
  #define AUDIO_CODEC NEXUS_AudioCodec_eAc3
  #define AUDIO_PID 0x104
#elif BTST_MPG_TRANSCODER_PASSTHRU /* MPEG */
  #define FILE_NAME "videos/cnnticker.mpg"
  #define AUDIO_CODEC NEXUS_AudioCodec_eMpeg
  #define AUDIO_PID 0x22
#else /* AAC */
  #define FILE_NAME "videos/testjun26_188.mpg"
  #define AUDIO_CODEC NEXUS_AudioCodec_eAac
  #define AUDIO_PID 0x110
#endif

#define XCODE_FILE_NAME "videos/cnnticker.mpg"
#define XCODE_AUDIO_PID 0x22

BDBG_MODULE(dual_audio_transcode);


#define BTST_PMT_PID 0x0055

#define NUM_ENCODERS (2) /* Hardcode to two for all chip types (A1 and B0) */

/* multi buffer to allow the background PSI packet to update CC while foreground one is waiting for TS mux pacing */
#define BTST_PSI_QUEUE_CNT 4 /* every second insert a PSI, so it takes 4 secs to circle; */
static void *pat[NUM_ENCODERS][BTST_PSI_QUEUE_CNT]={{NULL,},};
static void *pmt[NUM_ENCODERS][BTST_PSI_QUEUE_CNT]={{NULL,},};
NEXUS_StreamMuxSystemData psi[NUM_ENCODERS][2];

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
#if BTST_AC3_TRANSCODER_PASSTHRU /* avatar */
   0x11,0xf0,0x00,0x81,0xe0,0x12,0xf0,0x00,
#elif BTST_MPG_TRANSCODER_PASSTHRU /* cnnticker */
   0x11,0xf0,0x00,0x04,0xe0,0x12,0xf0,0x00,
#else /* testjun26_188 AAC */
   0x11,0xf0,0x00,0xc0,0xe0,0x12,0xf0,0x00,
#endif
   0x3d,0x19,0x07,0x2f,
};

static void
transcoderFinishCallback(void *context, int param)
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
    NEXUS_StcChannelHandle stcChannel[NUM_ENCODERS];
    NEXUS_StcChannelSettings stcSettings;
#if NEXUS_HAS_AUDIO_MUX_OUTPUT_TEST
    NEXUS_AudioDecoderHandle audioDecoder[NUM_ENCODERS], audioPassthrough[NUM_ENCODERS];
    NEXUS_AudioDecoderStartSettings audioProgram;
    NEXUS_PidChannelHandle audioPidChannel[NUM_ENCODERS];
    NEXUS_AudioMuxOutputHandle audioMuxOutput[NUM_ENCODERS];
	NEXUS_AudioMuxOutputStartSettings audioMuxStartSettings;
    NEXUS_PlaypumpHandle playpumpTranscodeAudio[NUM_ENCODERS];
    NEXUS_PidChannelHandle pidChannelTranscodeAudio[NUM_ENCODERS];
#if BTST_ENABLE_AUDIO_ENCODE
    NEXUS_AudioEncoderSettings encoderSettings;
    NEXUS_AudioEncoderHandle audioEncoder[NUM_ENCODERS];
#endif
#endif
    NEXUS_FilePlayHandle file[NUM_ENCODERS];
    NEXUS_PlaypumpHandle playpump[NUM_ENCODERS];
    NEXUS_PlaybackHandle playback[NUM_ENCODERS];
    NEXUS_PlaybackSettings playbackSettings;
    NEXUS_PlaybackPidChannelSettings playbackPidSettings;
    NEXUS_StreamMuxHandle streamMux[NUM_ENCODERS];
    NEXUS_StreamMuxCreateSettings muxCreateSettings;
    BKNI_EventHandle finishEvent[NUM_ENCODERS];
    NEXUS_StreamMuxStartSettings muxConfig;
    NEXUS_StreamMuxOutput muxOutput;
    NEXUS_PlaypumpOpenSettings playpumpConfig;
    NEXUS_PlaypumpHandle playpumpTranscodePcr[NUM_ENCODERS];
    NEXUS_FileRecordHandle fileTranscode[NUM_ENCODERS];
    NEXUS_StcChannelHandle stcChannelTranscode[NUM_ENCODERS];

    NEXUS_RecpumpHandle recpump[NUM_ENCODERS];
    NEXUS_RecordHandle record[NUM_ENCODERS];
    NEXUS_RecordSettings recordSettings;
    NEXUS_PidChannelHandle pidChannelTranscodePcr[NUM_ENCODERS];
    NEXUS_PidChannelHandle pidChannelTranscodePat[NUM_ENCODERS];
    NEXUS_PidChannelHandle pidChannelTranscodePmt[NUM_ENCODERS];

    const char *fname[NUM_ENCODERS] = {
		FILE_NAME,
		XCODE_FILE_NAME
    };

    int j=0, xcodeId=0;

    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init(&platformSettings);
    NEXUS_Platform_GetConfiguration(&platformConfig);

    audioPassthrough[0] = NULL;
    audioPassthrough[1] = NULL;

    for(xcodeId=0; xcodeId < NUM_ENCODERS; xcodeId++) {
	    playpump[xcodeId] = NEXUS_Playpump_Open(xcodeId*4, NULL);
	    assert(playpump[xcodeId]);
	    playback[xcodeId] = NEXUS_Playback_Create();
	    assert(playback[xcodeId]);

	    file[xcodeId] = NEXUS_FilePlay_OpenPosix(fname[xcodeId], NULL);
	    if (!file[xcodeId]) {
	        fprintf(stderr, "can't open file:%s\n", fname[xcodeId]);
	        return -1;
	    }

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

	    NEXUS_Playback_GetSettings(playback[xcodeId], &playbackSettings);
	    playbackSettings.playpump = playpump[xcodeId];
	    /* set a stream format, it could be any audio video transport type or file format, i.e NEXUS_TransportType_eMp4, NEXUS_TransportType_eAvi ... */
	    playbackSettings.playpumpSettings.transportType = TRANSPORT_TYPE;
	    playbackSettings.stcChannel = stcChannel[xcodeId];
	    NEXUS_Playback_SetSettings(playback[xcodeId], &playbackSettings);

#if NEXUS_HAS_AUDIO_MUX_OUTPUT_TEST
	    /* Open the audio decoder */
	    audioDecoder[xcodeId] = NEXUS_AudioDecoder_Open(2*xcodeId, NULL);
	    if(xcodeId==0)
        {
            audioPassthrough[xcodeId] = NEXUS_AudioDecoder_Open(2*xcodeId+1, NULL);
        }

	    /* Open the audio and pcr pid channel */
	    NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
	    playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eAudio;
	    playbackPidSettings.pidTypeSettings.audio.primary = audioDecoder[xcodeId]; /* must be told codec for correct handling */
        if(xcodeId==0)
        {
            playbackPidSettings.pidTypeSettings.audio.secondary = audioPassthrough[xcodeId];
        }
#if BTST_MPG_TRANSCODER_PASSTHRU /* MPG */
	    audioPidChannel[xcodeId] = NEXUS_Playback_OpenPidChannel(playback[xcodeId], AUDIO_PID, &playbackPidSettings);
#else
	    audioPidChannel[xcodeId] = NEXUS_Playback_OpenPidChannel(playback[xcodeId], (xcodeId==1)?XCODE_AUDIO_PID:AUDIO_PID, &playbackPidSettings);
#endif

	    /* Set up decoder Start structures now. We need to know the audio codec to properly set up the audio outputs. */
	    NEXUS_AudioDecoder_GetDefaultStartSettings(&audioProgram);
	    audioProgram.codec = AUDIO_CODEC;
	    if(xcodeId==1) { audioProgram.codec = NEXUS_AudioCodec_eMpeg; }

	    audioProgram.pidChannel = audioPidChannel[xcodeId];
	    audioProgram.stcChannel = stcChannel[xcodeId];

	    /* Connect audio decoders to outputs */
#if NEXUS_NUM_AUDIO_DACS
	    NEXUS_AudioOutput_AddInput(
	        NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[xcodeId]),
	        NEXUS_AudioDecoder_GetConnector(audioDecoder[xcodeId], NEXUS_AudioDecoderConnectorType_eStereo));
#endif
#if NEXUS_NUM_SPDIF_OUTPUTS
	    if(xcodeId==0)
        {
	        NEXUS_AudioOutput_AddInput(
	            NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[xcodeId]),
	            NEXUS_AudioDecoder_GetConnector(audioPassthrough[xcodeId], NEXUS_AudioDecoderConnectorType_eCompressed));
	    }
#endif

	    /* Open audio mux output */
	    audioMuxOutput[xcodeId] = NEXUS_AudioMuxOutput_Create(NULL);
	    assert(audioMuxOutput[xcodeId]);
#if BTST_ENABLE_AUDIO_ENCODE
	    /* Open audio encoder */
	    NEXUS_AudioEncoder_GetDefaultSettings(&encoderSettings);
	    encoderSettings.codec = NEXUS_AudioCodec_eAacPlusLoas;
	    audioEncoder[xcodeId] = NEXUS_AudioEncoder_Open(&encoderSettings);
	    assert(audioEncoder[xcodeId]);
	    /* Connect encoder to decoder */
	    NEXUS_AudioEncoder_AddInput(audioEncoder[xcodeId],
	                                NEXUS_AudioDecoder_GetConnector(audioDecoder[xcodeId], NEXUS_AudioDecoderConnectorType_eStereo));
	    /* Connect mux to encoder */
	    NEXUS_AudioOutput_AddInput(
	        NEXUS_AudioMuxOutput_GetConnector(audioMuxOutput[xcodeId]), NEXUS_AudioEncoder_GetConnector(audioEncoder[xcodeId]));
#else
        /* Passthrough */
        if(xcodeId==0)
        {
            NEXUS_AudioOutput_AddInput(
                NEXUS_AudioMuxOutput_GetConnector(audioMuxOutput[xcodeId]),
                NEXUS_AudioDecoder_GetConnector(audioPassthrough[xcodeId], NEXUS_AudioDecoderConnectorType_eCompressed));
        }
#endif
#endif /*NEXUS_HAS_AUDIO_MUX_OUTPUT_TEST*/

        /* Start audio mux output */
        NEXUS_AudioMuxOutput_GetDefaultStartSettings(&audioMuxStartSettings);
        audioMuxStartSettings.stcChannel = stcChannelTranscode[xcodeId];
        NEXUS_AudioMuxOutput_Start(audioMuxOutput[xcodeId], &audioMuxStartSettings);

	    NEXUS_Playpump_GetDefaultOpenSettings(&playpumpConfig);
	    playpumpConfig.fifoSize = 16384; /* reduce FIFO size allocated for playpump */
	    playpumpConfig.numDescriptors = 64; /* set number of descriptors */
	    playpumpConfig.streamMuxCompatible = true;

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

	    /* Start decoder */
#if NEXUS_HAS_AUDIO_MUX_OUTPUT_TEST
	    NEXUS_AudioDecoder_Start(audioDecoder[xcodeId], &audioProgram);
	    if(xcodeId==0) {
	        NEXUS_AudioDecoder_Start(audioPassthrough[xcodeId], &audioProgram);
	    }
#endif
	    /* Start playback */
	    NEXUS_Playback_Start(playback[xcodeId], file[xcodeId], NULL);

		for(j=0; j<BTST_PSI_QUEUE_CNT; j++)
		{
			NEXUS_Memory_Allocate(188, NULL, &pat[xcodeId][j]);
			NEXUS_Memory_Allocate(188, NULL, &pmt[xcodeId][j]);
		    BKNI_Memcpy(pat[xcodeId][j], s_auiTSPacket_PAT, sizeof(s_auiTSPacket_PAT));
		    BKNI_Memcpy(pmt[xcodeId][j], s_auiTSPacket_PMT, sizeof(s_auiTSPacket_PMT));
			NEXUS_Memory_FlushCache(pat[xcodeId][j], sizeof(s_auiTSPacket_PAT));
			NEXUS_Memory_FlushCache(pmt[xcodeId][j], sizeof(s_auiTSPacket_PMT));
		}
	    BKNI_Memset(psi[xcodeId], 0, sizeof(psi[xcodeId]));
	    psi[xcodeId][0].size = 188;
	    psi[xcodeId][0].pData = pat[xcodeId][0];
	    psi[xcodeId][0].timestampDelta = 0;
	    psi[xcodeId][1].size = 188;
	    psi[xcodeId][1].pData = pmt[xcodeId][0];
	    psi[xcodeId][1].timestampDelta = 0;
	    NEXUS_StreamMux_AddSystemDataBuffer(streamMux[xcodeId], &psi[xcodeId][0]);
	    NEXUS_StreamMux_AddSystemDataBuffer(streamMux[xcodeId], &psi[xcodeId][1]);
#if 0
	    for(;;) {
	        size_t queued=0,completed=0;
	        BKNI_Sleep(100);
	        NEXUS_StreamMux_GetCompletedSystemDataBuffers(streamMux, &completed);
	        fprintf(stderr, "queued:%u completed:%u\n", queued, completed);
	    }
#endif
	}

    /* Playback state machine is driven from inside Nexus. */
    printf("Press ENTER to continue; type 'q' to quit\n");
    getchar();

    /* Bring down system */
    for(xcodeId=0; xcodeId < NUM_ENCODERS; xcodeId++) {
	    NEXUS_Playback_Stop(playback[xcodeId]);
#if NEXUS_HAS_AUDIO_MUX_OUTPUT_TEST
	    if(xcodeId==0)
        {
            NEXUS_AudioDecoder_Stop(audioPassthrough[xcodeId]);
        }

	    NEXUS_AudioDecoder_Stop(audioDecoder[xcodeId]);
	    NEXUS_AudioMuxOutput_Stop(audioMuxOutput[xcodeId]);
#endif

	    NEXUS_StreamMux_Finish(streamMux[xcodeId]);
	    /* wait for the encoder buffer model's data to be drained */
	    if(BKNI_WaitForEvent(finishEvent[xcodeId], 5000)!=BERR_SUCCESS) {
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
	    NEXUS_Playpump_ClosePidChannel(playpumpTranscodePcr[xcodeId], pidChannelTranscodePcr[xcodeId]);
	    NEXUS_Playpump_ClosePidChannel(playpumpTranscodePcr[xcodeId], pidChannelTranscodePat[xcodeId]);
	    NEXUS_Playpump_ClosePidChannel(playpumpTranscodePcr[xcodeId], pidChannelTranscodePmt[xcodeId]);
	    NEXUS_FilePlay_Close(file[xcodeId]);
	    NEXUS_Playback_Destroy(playback[xcodeId]);
	    NEXUS_Playpump_Close(playpump[xcodeId]);

	    NEXUS_StreamMux_Destroy(streamMux[xcodeId]);

	    NEXUS_Playpump_Close(playpumpTranscodePcr[xcodeId]);

#if NEXUS_HAS_AUDIO_MUX_OUTPUT_TEST
#if BTST_ENABLE_AUDIO_ENCODE
		NEXUS_AudioOutput_RemoveInput(
			NEXUS_AudioMuxOutput_GetConnector(audioMuxOutput[xcodeId]), NEXUS_AudioEncoder_GetConnector(audioEncoder[xcodeId]));
		NEXUS_AudioEncoder_RemoveInput(audioEncoder[xcodeId],
									NEXUS_AudioDecoder_GetConnector(audioDecoder[xcodeId], NEXUS_AudioDecoderConnectorType_eStereo));
	    NEXUS_AudioInput_Shutdown(NEXUS_AudioEncoder_GetConnector(audioEncoder[xcodeId]));
	    NEXUS_AudioEncoder_Close(audioEncoder[xcodeId]);
#else
	    if(xcodeId==0)
        {
    		NEXUS_AudioOutput_RemoveInput(
                NEXUS_AudioMuxOutput_GetConnector(audioMuxOutput[xcodeId]),
                NEXUS_AudioDecoder_GetConnector(audioPassthrough[xcodeId], NEXUS_AudioDecoderConnectorType_eCompressed));
        }
#endif
#if NEXUS_NUM_AUDIO_DACS
	    NEXUS_AudioOutput_RemoveAllInputs(NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[xcodeId]));
#endif
#if NEXUS_NUM_SPDIF_OUTPUTS
	    if(xcodeId==0)
	    NEXUS_AudioOutput_RemoveAllInputs(NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[xcodeId]));
#endif
	    NEXUS_Playpump_Close(playpumpTranscodeAudio[xcodeId]);
	    NEXUS_AudioOutput_Shutdown(NEXUS_AudioMuxOutput_GetConnector(audioMuxOutput[xcodeId]));
	    NEXUS_AudioMuxOutput_Destroy(audioMuxOutput[xcodeId]);

	    NEXUS_AudioInput_Shutdown(NEXUS_AudioDecoder_GetConnector(audioDecoder[xcodeId], NEXUS_AudioDecoderConnectorType_eStereo));
	    if(xcodeId==0) {
	        NEXUS_AudioInput_Shutdown(NEXUS_AudioDecoder_GetConnector(audioPassthrough[xcodeId], NEXUS_AudioDecoderConnectorType_eCompressed));
	    }
#if NEXUS_NUM_AUDIO_DACS
	    NEXUS_AudioOutput_Shutdown(NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[xcodeId]));
#endif
#if NEXUS_NUM_SPDIF_OUTPUTS
	    if(xcodeId==0)
	    NEXUS_AudioOutput_Shutdown(NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[xcodeId]));
#endif
	    NEXUS_AudioDecoder_Close(audioDecoder[xcodeId]);
	    if(xcodeId==0)
        {
            NEXUS_AudioDecoder_Close(audioPassthrough[xcodeId]);
        }
#endif /*NEXUS_HAS_AUDIO_MUX_OUTPUT_TEST*/

	    NEXUS_StcChannel_Close(stcChannel[xcodeId]);
	    NEXUS_StcChannel_Close(stcChannelTranscode[xcodeId]);
	    NEXUS_Memory_Free(pat[xcodeId]);
	    NEXUS_Memory_Free(pmt[xcodeId]);
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

