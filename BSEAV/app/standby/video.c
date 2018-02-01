/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

#include "standby.h"
#include "util.h"

BDBG_MODULE(video);

extern B_StandbyNexusHandles g_StandbyNexusHandles;
extern B_DeviceState g_DeviceState;

void stc_channel_open(unsigned id)
{
    NEXUS_StcChannelSettings stcSettings;

    if(g_StandbyNexusHandles.stcChannel[id])
        return;

    NEXUS_StcChannel_GetDefaultSettings(id, &stcSettings);
    stcSettings.timebase = (id==0)?NEXUS_Timebase_e0:NEXUS_Timebase_e1;
    stcSettings.mode = NEXUS_StcChannelMode_eAuto;
    g_StandbyNexusHandles.stcChannel[id] = NEXUS_StcChannel_Open(id, &stcSettings);
}

void stc_channel_close(unsigned id)
{
    if(g_StandbyNexusHandles.stcChannel[id])
        NEXUS_StcChannel_Close(g_StandbyNexusHandles.stcChannel[id]);
    g_StandbyNexusHandles.stcChannel[id] = NULL;
}

void set_max_decode_size(unsigned id)
{
    NEXUS_Error rc;
    NEXUS_VideoDecoderSettings videoDecoderSettings;

    if(!g_StandbyNexusHandles.videoDecoder[id])
        return;

    NEXUS_VideoDecoder_GetSettings(g_StandbyNexusHandles.videoDecoder[id], &videoDecoderSettings);
    if(g_DeviceState.opts[id].width > videoDecoderSettings.maxWidth ||
       g_DeviceState.opts[id].height > videoDecoderSettings.maxHeight) {
        videoDecoderSettings.maxWidth = g_DeviceState.opts[id].width ;
        videoDecoderSettings.maxHeight = g_DeviceState.opts[id].height;
        rc = NEXUS_VideoDecoder_SetSettings(g_StandbyNexusHandles.videoDecoder[id], &videoDecoderSettings);
        BDBG_ASSERT(!rc);
    }
}

void decoder_setup(unsigned id)
{
    NEXUS_VideoDecoder_GetDefaultStartSettings(&g_StandbyNexusHandles.videoProgram[id]);
    g_StandbyNexusHandles.videoProgram[id].codec = g_DeviceState.opts[id].videoCodec;
    g_StandbyNexusHandles.videoProgram[id].pidChannel = g_StandbyNexusHandles.videoPidChannel[id];
    g_StandbyNexusHandles.videoProgram[id].stcChannel = g_StandbyNexusHandles.stcChannel[id];
    NEXUS_AudioDecoder_GetDefaultStartSettings(&g_StandbyNexusHandles.audioProgram[id]);
    g_StandbyNexusHandles.audioProgram[id].codec = g_DeviceState.opts[id].audioCodec;
    g_StandbyNexusHandles.audioProgram[id].pidChannel = g_StandbyNexusHandles.audioPidChannel[id];
    g_StandbyNexusHandles.audioProgram[id].stcChannel = g_StandbyNexusHandles.stcChannel[id];
}

int decode_start(unsigned id)
{
    NEXUS_Error rc=0;

    if(g_DeviceState.decode_started[id]) {
        BDBG_WRN(("Video Decoder %u already started", id));
        return rc;
    }

    /* Start decoders */
    if(g_StandbyNexusHandles.videoDecoder[id] && g_DeviceState.opts[id].videoPid) {
        rc = NEXUS_VideoDecoder_Start(g_StandbyNexusHandles.videoDecoder[id], &g_StandbyNexusHandles.videoProgram[id]);
        if (rc) { BERR_TRACE(rc); goto video_err;}
    }

    if(g_StandbyNexusHandles.audioDecoder[id] && g_DeviceState.opts[id].audioPid) {
        rc = NEXUS_AudioDecoder_Start(g_StandbyNexusHandles.audioDecoder[id], &g_StandbyNexusHandles.audioProgram[id]);
        if (rc) { BERR_TRACE(rc); goto audio_err;}
    }
    g_DeviceState.decode_started[id] = true;

    return rc;

audio_err:
    NEXUS_VideoDecoder_Stop(g_StandbyNexusHandles.videoDecoder[id]);
video_err:
    return rc;
}

void decode_stop(unsigned id)
{
    if(!g_DeviceState.decode_started[id])
        return;

    /* Stop decoders */
    if(g_StandbyNexusHandles.videoDecoder[id] && g_DeviceState.opts[id].videoPid)
        NEXUS_VideoDecoder_Stop(g_StandbyNexusHandles.videoDecoder[id]);
    if(g_StandbyNexusHandles.audioDecoder[id] && g_DeviceState.opts[id].audioPid)
        NEXUS_AudioDecoder_Stop(g_StandbyNexusHandles.audioDecoder[id]);

    g_DeviceState.decode_started[id] = false;
}

void decoder_open(unsigned id)
{
    /* bring up video decoder */
    g_StandbyNexusHandles.videoDecoder[id] = NEXUS_VideoDecoder_Open(id, NULL); /* take default capabilities */
    if(!g_StandbyNexusHandles.videoDecoder[id]) {
        BDBG_WRN(("Video Decoder %u not supported", id));
    }

    /* Bring up audio decoders */
    g_StandbyNexusHandles.audioDecoder[id] = NEXUS_AudioDecoder_Open(id, NULL);
    if(g_StandbyNexusHandles.audioDecoder[id]) {
#if NEXUS_NUM_AUDIO_DUMMY_OUTPUTS
    /* Add dummy output, so that decoder can be started without an actual output */
        NEXUS_AudioOutput_AddInput(
            NEXUS_AudioDummyOutput_GetConnector(g_StandbyNexusHandles.platformConfig.outputs.audioDummy[id]),
            NEXUS_AudioDecoder_GetConnector(g_StandbyNexusHandles.audioDecoder[id], NEXUS_AudioDecoderConnectorType_eStereo));
#endif
    } else {
        BDBG_WRN(("Audio Decoder %u not supported", id));
    }
}

void decoder_close(unsigned id)
{
    if(g_StandbyNexusHandles.videoDecoder[id]) {
        NEXUS_VideoInput_Shutdown(NEXUS_VideoDecoder_GetConnector(g_StandbyNexusHandles.videoDecoder[id]));
        NEXUS_VideoDecoder_Close(g_StandbyNexusHandles.videoDecoder[id]);
    }
    g_StandbyNexusHandles.videoDecoder[id] = NULL;

#if NEXUS_NUM_AUDIO_DACS
    if(g_StandbyNexusHandles.platformConfig.outputs.audioDacs[0])
        NEXUS_AudioOutput_RemoveAllInputs(NEXUS_AudioDac_GetConnector(g_StandbyNexusHandles.platformConfig.outputs.audioDacs[0]));
#endif
#if NEXUS_NUM_SPDIF_OUTPUTS
    if(g_StandbyNexusHandles.platformConfig.outputs.spdif[0])
        NEXUS_AudioOutput_RemoveAllInputs(NEXUS_SpdifOutput_GetConnector(g_StandbyNexusHandles.platformConfig.outputs.spdif[0]));
#endif
    if(g_StandbyNexusHandles.audioDecoder[id]) {
        NEXUS_AudioInput_Shutdown(NEXUS_AudioDecoder_GetConnector(g_StandbyNexusHandles.audioDecoder[id], NEXUS_AudioDecoderConnectorType_eStereo));
        NEXUS_AudioDecoder_Close(g_StandbyNexusHandles.audioDecoder[id]);
    }
    g_StandbyNexusHandles.audioDecoder[id] = NULL;

}

int picture_decode_start(void)
{
    g_StandbyNexusHandles.picSurface = picdecoder_decode(g_StandbyNexusHandles.picDecoder, g_DeviceState.picfile);

    return 0;
}

void picture_decode_stop(void)
{
    if(g_StandbyNexusHandles.picSurface)
        NEXUS_Surface_Destroy(g_StandbyNexusHandles.picSurface);
    g_StandbyNexusHandles.picSurface = NULL;
}

void picture_decoder_open(void)
{
    g_StandbyNexusHandles.picDecoder = picdecoder_open();
}

void picture_decoder_close(void)
{
    if (g_StandbyNexusHandles.picDecoder)
        picdecoder_close(g_StandbyNexusHandles.picDecoder);
}

int encode_start(unsigned id)
{
#if NEXUS_HAS_VIDEO_ENCODER && !NEXUS_NUM_DSP_VIDEO_ENCODERS
    NEXUS_Error rc=0;
    NEXUS_StreamMuxStartSettings muxConfig;
    NEXUS_StreamMuxOutput muxOutput;
    NEXUS_RecordPidChannelSettings recordPidSettings;
    NEXUS_VideoEncoderSettings videoEncoderConfig;
    NEXUS_VideoEncoderStartSettings videoEncoderStartConfig;
    NEXUS_VideoEncoderDelayRange videoDelay;
    NEXUS_AudioMuxOutputDelayStatus audioDelayStatus;
    NEXUS_AudioMuxOutputStartSettings audioMuxStartSettings;

    BSTD_UNUSED(id);

    if(g_DeviceState.encode_started)
        return rc;

    NEXUS_VideoWindow_AddInput(g_StandbyNexusHandles.windowTranscode, NEXUS_VideoDecoder_GetConnector(g_StandbyNexusHandles.videoDecoder[0]));

    NEXUS_VideoEncoder_GetSettings(g_StandbyNexusHandles.videoEncoder, &videoEncoderConfig);
    videoEncoderConfig.variableFrameRate = true; /* encoder can detect film content and follow CET */
    videoEncoderConfig.frameRate = NEXUS_VideoFrameRate_e23_976;
    videoEncoderConfig.bitrateMax = 6*1000*1000;
    videoEncoderConfig.streamStructure.framesP = 23;
    videoEncoderConfig.streamStructure.framesB = 0;

    NEXUS_VideoEncoder_GetDefaultStartSettings(&videoEncoderStartConfig);
    videoEncoderStartConfig.codec = NEXUS_VideoCodec_eH264;
    videoEncoderStartConfig.profile = NEXUS_VideoCodecProfile_eBaseline;
    videoEncoderStartConfig.level = NEXUS_VideoCodecLevel_e31;
    videoEncoderStartConfig.input = g_StandbyNexusHandles.displayTranscode;
    videoEncoderStartConfig.stcChannel = g_StandbyNexusHandles.stcChannelTranscode;

    /******************************************
     * add configurable delay to video path
     */
    /* NOTE: ITFP is encoder feature to detect and lock on 3:2/2:2 cadence in the video content to help
     * efficient coding for interlaced formats; disabling ITFP will impact the bit efficiency but reduce the encode delay. */
    videoEncoderConfig.enableFieldPairing = true;

    /* 0 to use default 750ms rate buffer delay; TODO: allow user to adjust it to lower encode delay at cost of quality reduction! */
    videoEncoderStartConfig.rateBufferDelay = 0;

    /* to allow 23.976p passthru; TODO: allow user to configure minimum framerate to achieve lower delay!
     * Note: lower minimum framerate means longer encode delay */
    videoEncoderStartConfig.bounds.inputFrameRate.min = NEXUS_VideoFrameRate_e23_976;

    /* to allow 24 ~ 60p dynamic frame rate coding TODO: allow user to config higher minimum frame rate for lower delay! */
    videoEncoderStartConfig.bounds.outputFrameRate.min = NEXUS_VideoFrameRate_e23_976;
    videoEncoderStartConfig.bounds.outputFrameRate.max = NEXUS_VideoFrameRate_e60;

    /* max encode size allows 1080p encode; TODO: allow user to choose lower max resolution for lower encode delay */
    videoEncoderStartConfig.bounds.inputDimension.max.width = 1280;
    videoEncoderStartConfig.bounds.inputDimension.max.height = 720;

    {
        unsigned Dee;

        /* NOTE: video encoder delay is in 27MHz ticks */
        NEXUS_VideoEncoder_GetDelayRange(g_StandbyNexusHandles.videoEncoder, &videoEncoderConfig, &videoEncoderStartConfig, &videoDelay);
        printf("\n\tVideo encoder end-to-end delay = %u ms; maximum allowed: %u ms\n", videoDelay.min/27000, videoDelay.max/27000);

        NEXUS_AudioMuxOutput_GetDelayStatus(g_StandbyNexusHandles.audioMuxOutput, NEXUS_AudioCodec_eAac, &audioDelayStatus);
        printf("\tAudio codec AAC end-to-end delay = %u ms\n", audioDelayStatus.endToEndDelay);

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
        audioMuxStartSettings.stcChannel = g_StandbyNexusHandles.stcChannelTranscode;
        audioMuxStartSettings.presentationDelay = Dee/27000;/* in ms */
        NEXUS_AudioMuxOutput_Start(g_StandbyNexusHandles.audioMuxOutput, &audioMuxStartSettings);
    }

    NEXUS_VideoEncoder_SetSettings(g_StandbyNexusHandles.videoEncoder, &videoEncoderConfig);

    NEXUS_StreamMux_GetDefaultStartSettings(&muxConfig);
    muxConfig.transportType = NEXUS_TransportType_eTs;
    muxConfig.stcChannel = g_StandbyNexusHandles.stcChannelTranscode;

    muxConfig.video[0].pid = 0x11;
    muxConfig.video[0].encoder = g_StandbyNexusHandles.videoEncoder;
    muxConfig.video[0].playpump = g_StandbyNexusHandles.playpumpTranscodeVideo;

    muxConfig.audio[0].pid = 0x12;
    muxConfig.audio[0].muxOutput = g_StandbyNexusHandles.audioMuxOutput;
    muxConfig.audio[0].playpump = g_StandbyNexusHandles.playpumpTranscodeAudio;

    muxConfig.pcr.pid = 0x13;
    muxConfig.pcr.playpump = g_StandbyNexusHandles.playpumpTranscodePcr;
    muxConfig.pcr.interval = 50;

    g_StandbyNexusHandles.pidChannelTranscodePcr = NEXUS_Playpump_OpenPidChannel(g_StandbyNexusHandles.playpumpTranscodePcr, muxConfig.pcr.pid, NULL);
    BDBG_ASSERT(g_StandbyNexusHandles.pidChannelTranscodePcr);

    NEXUS_StreamMux_Start(g_StandbyNexusHandles.streamMux, &muxConfig, &muxOutput);

    g_StandbyNexusHandles.pidChannelTranscodeVideo = muxOutput.video[0];
    g_StandbyNexusHandles.pidChannelTranscodeAudio = muxOutput.audio[0];

    NEXUS_Record_GetDefaultPidChannelSettings(&recordPidSettings);
    recordPidSettings.recpumpSettings.pidType = NEXUS_PidType_eVideo;
    recordPidSettings.recpumpSettings.pidTypeSettings.video.index = true;
    recordPidSettings.recpumpSettings.pidTypeSettings.video.codec = NEXUS_VideoCodec_eH264;

    /* add multiplex data to the same record */
    NEXUS_Record_AddPidChannel(g_StandbyNexusHandles.recordTranscode, g_StandbyNexusHandles.pidChannelTranscodeVideo, &recordPidSettings);
    NEXUS_Record_AddPidChannel(g_StandbyNexusHandles.recordTranscode, g_StandbyNexusHandles.pidChannelTranscodeAudio, NULL);
    NEXUS_Record_AddPidChannel(g_StandbyNexusHandles.recordTranscode, g_StandbyNexusHandles.pidChannelTranscodePcr, NULL);

    g_StandbyNexusHandles.fileTranscode = NEXUS_FileRecord_OpenPosix("videos/encode.mpg", "videos/encode.nav");
    BDBG_ASSERT(g_StandbyNexusHandles.fileTranscode);

    /* Start record of stream mux output */
    NEXUS_Record_Start(g_StandbyNexusHandles.recordTranscode, g_StandbyNexusHandles.fileTranscode);

    NEXUS_VideoEncoder_Start(g_StandbyNexusHandles.videoEncoder, &videoEncoderStartConfig);

    g_DeviceState.encode_started = true;

    return rc;
#else
    BSTD_UNUSED(id);
    return 0;
#endif
}

void encode_stop(unsigned id)
{
    BSTD_UNUSED(id);

#if NEXUS_HAS_VIDEO_ENCODER && !NEXUS_NUM_DSP_VIDEO_ENCODERS
    if(!g_DeviceState.encode_started)
        return;

    NEXUS_AudioMuxOutput_Stop(g_StandbyNexusHandles.audioMuxOutput);
    NEXUS_VideoEncoder_Stop(g_StandbyNexusHandles.videoEncoder, NULL);

    NEXUS_StreamMux_Finish(g_StandbyNexusHandles.streamMux);
    if(BKNI_WaitForEvent(g_StandbyNexusHandles.finishEvent, 2000)!=BERR_SUCCESS) {
        fprintf(stderr, "TIMEOUT\n");
    }

    NEXUS_Record_Stop(g_StandbyNexusHandles.recordTranscode);
    NEXUS_Record_RemoveAllPidChannels(g_StandbyNexusHandles.recordTranscode);
    NEXUS_StreamMux_Stop(g_StandbyNexusHandles.streamMux);
    NEXUS_FileRecord_Close(g_StandbyNexusHandles.fileTranscode);

    NEXUS_Playpump_CloseAllPidChannels(g_StandbyNexusHandles.playpumpTranscodePcr);

    NEXUS_VideoWindow_RemoveInput(g_StandbyNexusHandles.windowTranscode, NEXUS_VideoDecoder_GetConnector(g_StandbyNexusHandles.videoDecoder[0]));

    g_DeviceState.encode_started = false;
#endif
}

#if NEXUS_HAS_VIDEO_ENCODER && !NEXUS_NUM_DSP_VIDEO_ENCODERS
static void transcoderFinishCallback(void *context, int param)
{
    BKNI_EventHandle finishEvent = (BKNI_EventHandle)context;

    BSTD_UNUSED(param);
    BDBG_WRN(("Finish callback invoked, now stop the stream mux."));
    BKNI_SetEvent(finishEvent);
}
#endif

void encoder_open(unsigned id)
{
#if NEXUS_HAS_VIDEO_ENCODER && !NEXUS_NUM_DSP_VIDEO_ENCODERS
    NEXUS_StcChannelSettings stcSettings;
    NEXUS_DisplaySettings displaySettings;
    NEXUS_AudioEncoderSettings encoderSettings;
    NEXUS_VideoEncoderCapabilities videoEncoderCap;
    NEXUS_StreamMuxCreateSettings muxCreateSettings;
    NEXUS_PlaypumpOpenSettings playpumpConfig;
    NEXUS_RecordSettings recordSettings;

    BSTD_UNUSED(id);

    NEXUS_StcChannel_GetDefaultSettings(2, &stcSettings);
    stcSettings.timebase = NEXUS_Timebase_e0;/* should be the same timebase for end-to-end locking */
    stcSettings.mode = NEXUS_StcChannelMode_eAuto;
    stcSettings.pcrBits = NEXUS_StcChannel_PcrBits_eFull42;/* ViCE2 requires 42-bit STC broadcast */
    g_StandbyNexusHandles.stcChannelTranscode = NEXUS_StcChannel_Open(2, &stcSettings);
    BDBG_ASSERT(g_StandbyNexusHandles.stcChannelTranscode);

    g_StandbyNexusHandles.audioMuxOutput = NEXUS_AudioMuxOutput_Create(NULL);
    BDBG_ASSERT(g_StandbyNexusHandles.audioMuxOutput);

    NEXUS_AudioEncoder_GetDefaultSettings(&encoderSettings);
    encoderSettings.codec = NEXUS_AudioCodec_eAac;
    g_StandbyNexusHandles.audioEncoder = NEXUS_AudioEncoder_Open(&encoderSettings);
    BDBG_ASSERT(g_StandbyNexusHandles.audioEncoder);

    if(g_DeviceState.decode_started[0])
        NEXUS_AudioDecoder_Stop(g_StandbyNexusHandles.audioDecoder[0]);

    NEXUS_AudioEncoder_AddInput(g_StandbyNexusHandles.audioEncoder,
            NEXUS_AudioDecoder_GetConnector(g_StandbyNexusHandles.audioDecoder[0], NEXUS_AudioDecoderConnectorType_eStereo));

    NEXUS_AudioOutput_AddInput(
            NEXUS_AudioMuxOutput_GetConnector(g_StandbyNexusHandles.audioMuxOutput),
            NEXUS_AudioEncoder_GetConnector(g_StandbyNexusHandles.audioEncoder));

    if(g_DeviceState.decode_started[id])
        NEXUS_AudioDecoder_Start(g_StandbyNexusHandles.audioDecoder[0], &g_StandbyNexusHandles.audioProgram[0]);

    g_StandbyNexusHandles.videoEncoder = NEXUS_VideoEncoder_Open(0, NULL);
    BDBG_ASSERT(g_StandbyNexusHandles.videoEncoder);
    NEXUS_GetVideoEncoderCapabilities(&videoEncoderCap);

    NEXUS_Display_GetDefaultSettings(&displaySettings);
    displaySettings.displayType = NEXUS_DisplayType_eAuto;
    displaySettings.timingGenerator = NEXUS_DisplayTimingGenerator_eEncoder;
    displaySettings.format = NEXUS_VideoFormat_e720p;
    g_StandbyNexusHandles.displayTranscode = NEXUS_Display_Open(videoEncoderCap.videoEncoder[0].displayIndex, &displaySettings);/* cmp3 for transcoder */
    BDBG_ASSERT(g_StandbyNexusHandles.displayTranscode);

    g_StandbyNexusHandles.windowTranscode = NEXUS_VideoWindow_Open(g_StandbyNexusHandles.displayTranscode, 0);
    BDBG_ASSERT(g_StandbyNexusHandles.windowTranscode);

    NEXUS_Playpump_GetDefaultOpenSettings(&playpumpConfig);
    playpumpConfig.fifoSize = 16384; /* reduce FIFO size allocated for playpump */
    playpumpConfig.numDescriptors = 64; /* set number of descriptors */
    playpumpConfig.streamMuxCompatible = true;
    g_StandbyNexusHandles.playpumpTranscodeVideo = NEXUS_Playpump_Open(2, &playpumpConfig);
    BDBG_ASSERT(g_StandbyNexusHandles.playpumpTranscodeVideo);
    g_StandbyNexusHandles.playpumpTranscodeAudio = NEXUS_Playpump_Open(3, &playpumpConfig);
    BDBG_ASSERT(g_StandbyNexusHandles.playpumpTranscodeAudio);
    g_StandbyNexusHandles.playpumpTranscodePcr = NEXUS_Playpump_Open(4, &playpumpConfig);
    BDBG_ASSERT(g_StandbyNexusHandles.playpumpTranscodePcr);

    g_StandbyNexusHandles.recpumpTranscode = NEXUS_Recpump_Open(2, NULL);
    BDBG_ASSERT(g_StandbyNexusHandles.recpumpTranscode);
    g_StandbyNexusHandles.recordTranscode = NEXUS_Record_Create();
    BDBG_ASSERT(g_StandbyNexusHandles.recordTranscode);
    NEXUS_Record_GetSettings(g_StandbyNexusHandles.recordTranscode, &recordSettings);
    recordSettings.recpump = g_StandbyNexusHandles.recpumpTranscode;
    NEXUS_Record_SetSettings(g_StandbyNexusHandles.recordTranscode, &recordSettings);

    BKNI_CreateEvent(&g_StandbyNexusHandles.finishEvent);
    NEXUS_StreamMux_GetDefaultCreateSettings(&muxCreateSettings);
    muxCreateSettings.finished.callback = transcoderFinishCallback;
    muxCreateSettings.finished.context = g_StandbyNexusHandles.finishEvent;
    g_StandbyNexusHandles.streamMux = NEXUS_StreamMux_Create(&muxCreateSettings);
    BDBG_ASSERT(g_StandbyNexusHandles.streamMux);
#else
    BSTD_UNUSED(id);
#endif
}

void encoder_close(unsigned id)
{
    BSTD_UNUSED(id);

#if NEXUS_HAS_VIDEO_ENCODER && !NEXUS_NUM_DSP_VIDEO_ENCODERS
    if(g_StandbyNexusHandles.streamMux)
        NEXUS_StreamMux_Destroy(g_StandbyNexusHandles.streamMux);
    g_StandbyNexusHandles.streamMux = NULL;

    if(g_StandbyNexusHandles.finishEvent)
        BKNI_DestroyEvent(g_StandbyNexusHandles.finishEvent);
    g_StandbyNexusHandles.finishEvent = NULL;

    if(g_StandbyNexusHandles.recordTranscode)
        NEXUS_Record_Destroy(g_StandbyNexusHandles.recordTranscode);
    g_StandbyNexusHandles.recordTranscode = NULL;
    if(g_StandbyNexusHandles.recpumpTranscode)
        NEXUS_Recpump_Close(g_StandbyNexusHandles.recpumpTranscode);
    g_StandbyNexusHandles.recpumpTranscode = NULL;

    if(g_StandbyNexusHandles.playpumpTranscodePcr)
        NEXUS_Playpump_Close(g_StandbyNexusHandles.playpumpTranscodePcr);
    g_StandbyNexusHandles.playpumpTranscodePcr = NULL;
    if(g_StandbyNexusHandles.playpumpTranscodeAudio)
        NEXUS_Playpump_Close(g_StandbyNexusHandles.playpumpTranscodeAudio);
    g_StandbyNexusHandles.playpumpTranscodeAudio = NULL;
    if(g_StandbyNexusHandles.playpumpTranscodeVideo)
        NEXUS_Playpump_Close(g_StandbyNexusHandles.playpumpTranscodeVideo);
    g_StandbyNexusHandles.playpumpTranscodeVideo = NULL;

    if(g_StandbyNexusHandles.windowTranscode)
        NEXUS_VideoWindow_Close(g_StandbyNexusHandles.windowTranscode);
    g_StandbyNexusHandles.windowTranscode = NULL;
    if(g_StandbyNexusHandles.displayTranscode)
        NEXUS_Display_Close(g_StandbyNexusHandles.displayTranscode);
    g_StandbyNexusHandles.displayTranscode = NULL;

    if(g_StandbyNexusHandles.videoEncoder)
        NEXUS_VideoEncoder_Close(g_StandbyNexusHandles.videoEncoder);
    g_StandbyNexusHandles.videoEncoder = NULL;

    if(g_StandbyNexusHandles.audioMuxOutput) {
        NEXUS_AudioOutput_RemoveAllInputs(NEXUS_AudioMuxOutput_GetConnector(g_StandbyNexusHandles.audioMuxOutput));
        NEXUS_AudioOutput_Shutdown(NEXUS_AudioMuxOutput_GetConnector(g_StandbyNexusHandles.audioMuxOutput));
        NEXUS_AudioMuxOutput_Destroy(g_StandbyNexusHandles.audioMuxOutput);
    }
    g_StandbyNexusHandles.audioMuxOutput = NULL;

    if(g_DeviceState.decode_started[0])
        NEXUS_AudioDecoder_Stop(g_StandbyNexusHandles.audioDecoder[0]);

    if(g_StandbyNexusHandles.audioEncoder) {
        NEXUS_AudioEncoder_RemoveAllInputs(g_StandbyNexusHandles.audioEncoder);
        NEXUS_AudioInput_Shutdown(NEXUS_AudioEncoder_GetConnector(g_StandbyNexusHandles.audioEncoder));
        NEXUS_AudioEncoder_Close(g_StandbyNexusHandles.audioEncoder);
    }
    g_StandbyNexusHandles.audioEncoder = NULL;

    if(g_DeviceState.decode_started[0])
        NEXUS_AudioDecoder_Start(g_StandbyNexusHandles.audioDecoder[0], &g_StandbyNexusHandles.audioProgram[0]);

    if(g_StandbyNexusHandles.stcChannelTranscode)
        NEXUS_StcChannel_Close(g_StandbyNexusHandles.stcChannelTranscode);
    g_StandbyNexusHandles.stcChannelTranscode = NULL;
#endif
}
