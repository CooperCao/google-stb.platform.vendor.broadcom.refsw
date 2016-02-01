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

#include "bstd.h"
#include "nexus_platform.h"
#if NEXUS_AUDIO_MODULE_FAMILY == NEXUS_AUDIO_MODULE_FAMILY_APE_RAAGA
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
#if NEXUS_NUM_AUDIO_DUMMY_OUTPUTS
#include "nexus_audio_dummy_output.h"
#endif
#include "nexus_spdif_output.h"
#include "nexus_composite_output.h"
#include "nexus_component_output.h"
#include "nexus_echo_canceller.h"
#include "nexus_audio_encoder.h"
#include "nexus_audio_mux_output.h"
#if NEXUS_HAS_HDMI_OUTPUT
#include "nexus_hdmi_output.h"
#endif


#include "nexus_pid_channel.h"
#include "nexus_playpump.h"
#include "nexus_playback.h"

#include "bkni.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

BDBG_MODULE(echo_canceller);

/* the following define the input and its characteristics -- these will vary by input */
#define TRANSPORT_TYPE NEXUS_TransportType_eTs
#define VIDEO_CODEC NEXUS_VideoCodec_eMpeg2
#define AUDIO_CODEC NEXUS_AudioCodec_eG726
#define VIDEO_PID 501
#define AUDIO_PID 502
#define DESC_PID  504

static void eof(void *context, int param)
{
    BSTD_UNUSED(param);
    BKNI_SetEvent((BKNI_EventHandle)context);
}

#endif /* NEXUS_AUDIO_MODULE_FAMILY == NEXUS_AUDIO_MODULE_FAMILY_APE_RAAGA */

int main(int argc, char **argv)
{
#if NEXUS_AUDIO_MODULE_FAMILY == NEXUS_AUDIO_MODULE_FAMILY_APE_RAAGA
    NEXUS_PlatformSettings platformSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_InputBand inputBand;
    NEXUS_ParserBand parserBand;
    NEXUS_ParserBandSettings parserBandSettings;
    NEXUS_StcChannelHandle stcChannel, stcChannelTranscode;
    NEXUS_StcChannelSettings stcSettings;
    NEXUS_PidChannelHandle videoPidChannel, audioPidChannel, remotePidChannel;
    NEXUS_DisplayHandle display;
    NEXUS_DisplaySettings displaySettings;
    NEXUS_VideoWindowHandle window;
    NEXUS_VideoDecoderHandle vdecode;
    NEXUS_VideoDecoderStartSettings videoProgram;
    NEXUS_AudioDecoderHandle localDecoder, remoteDecoder;
    NEXUS_AudioDecoderStartSettings audioProgram, remoteProgram;
    NEXUS_AudioDecoderOpenSettings audioOpenSettings;
    NEXUS_AudioEncoderHandle audioEncoder;
    NEXUS_EchoCancellerHandle echoCanceller;
        NEXUS_AudioMuxOutputStatus audioMuxStatus;
        NEXUS_AudioMuxOutputStartSettings audioMuxStartSettings;
    NEXUS_AudioMuxOutputHandle audioMuxOutput;
    NEXUS_AudioEncoderSettings encoderSettings;
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_HdmiOutputStatus hdmiStatus;
    NEXUS_Error rc;
#endif

    NEXUS_FilePlayHandle file;
    NEXUS_PlaypumpHandle playpump;
    NEXUS_PlaybackHandle playback;
    NEXUS_PlaybackSettings playbackSettings;
    NEXUS_PlaybackPidChannelSettings pidSettings;
    NEXUS_PlaybackStartSettings playbackStartSettings;

    NEXUS_FilePlayHandle rfile;
    NEXUS_PlaypumpHandle rplaypump;
    NEXUS_PlaybackHandle rplayback;
    NEXUS_PlaybackSettings rplaybackSettings;
    NEXUS_PlaybackPidChannelSettings rpidSettings;
    NEXUS_PlaybackStartSettings rplaybackStartSettings;
    BKNI_EventHandle event;

    /* Bring up all modules for a platform in a default configuration for this platform */
    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init(&platformSettings);
    NEXUS_Platform_GetConfiguration(&platformConfig);

    BKNI_CreateEvent(&event);
    playpump = NEXUS_Playpump_Open(0, NULL);
    BDBG_ASSERT(playpump);
    playback = NEXUS_Playback_Create();
    BDBG_ASSERT(playback);

    rplaypump = NEXUS_Playpump_Open(1, NULL);
    BDBG_ASSERT(rplaypump);
    rplayback = NEXUS_Playback_Create();
    BDBG_ASSERT(rplayback);

#define LOCAL_FILE_NAME     "local_file.bcma"
#define REMOTE_FILE_NAME    "remote_file.bcma"

    file = NEXUS_FilePlay_OpenPosix(LOCAL_FILE_NAME, NULL);
    if (!file) {
        fprintf(stderr, "can't open file:%s\n", LOCAL_FILE_NAME);
        return -1;
    }

    rfile = NEXUS_FilePlay_OpenPosix(REMOTE_FILE_NAME, NULL);
    if (!rfile) {
        fprintf(stderr, "can't open file:%s\n", REMOTE_FILE_NAME);
        return -1;
    }


    NEXUS_Playback_GetSettings(playback, &playbackSettings);
    playbackSettings.playpump = playpump;
    playbackSettings.playpumpSettings.transportType = NEXUS_TransportType_eEs;
    playbackSettings.endOfStreamCallback.callback = eof;
    playbackSettings.endOfStreamCallback.context = event;
    playbackSettings.enableStreamProcessing = false; /* this should be set to true if application needs to seek in the MP3 file, it'd cause decoder to receive MPEG-2 PES data */
    NEXUS_Playback_SetSettings(playback, &playbackSettings);

    NEXUS_Playback_GetSettings(rplayback, &rplaybackSettings);
    rplaybackSettings.playpump = rplaypump;
    rplaybackSettings.playpumpSettings.transportType = NEXUS_TransportType_eEs;
    rplaybackSettings.endOfStreamCallback.callback = eof;
    rplaybackSettings.endOfStreamCallback.context = event;
    rplaybackSettings.enableStreamProcessing = false; /* this should be set to true if application needs to seek in the MP3 file, it'd cause decoder to receive MPEG-2 PES data */
    NEXUS_Playback_SetSettings(rplayback, &rplaybackSettings);

    /* Get the streamer input band from Platform. Platform has already configured the FPGA with a default streamer routing */
    NEXUS_Platform_GetStreamerInputBand(0, &inputBand);

    /* Map a parser band to the streamer input band. */
    parserBand = NEXUS_ParserBand_e0;
    NEXUS_ParserBand_GetSettings(parserBand, &parserBandSettings);
    parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eInputBand;
    parserBandSettings.sourceTypeSettings.inputBand = inputBand;
    parserBandSettings.transportType = TRANSPORT_TYPE;
    NEXUS_ParserBand_SetSettings(parserBand, &parserBandSettings);

    /* Open the audio and video pid channels */
    videoPidChannel = NEXUS_PidChannel_Open(parserBand, VIDEO_PID, NULL);
    audioPidChannel = NEXUS_PidChannel_Open(parserBand, AUDIO_PID, NULL);
    remotePidChannel = NEXUS_PidChannel_Open(parserBand, DESC_PID, NULL);

    /* Open the StcChannel to do lipsync with the PCR */
    NEXUS_StcChannel_GetDefaultSettings(0, &stcSettings);
    stcSettings.timebase = NEXUS_Timebase_e0;
    stcSettings.mode = NEXUS_StcChannelMode_ePcr; /* live */
    stcSettings.modeSettings.pcr.pidChannel = videoPidChannel; /* PCR happens to be on video pid */
    stcChannel = NEXUS_StcChannel_Open(0, &stcSettings);

    /* Set up decoder Start structures now. We need to know the audio codec to properly set up the audio outputs. */
    NEXUS_VideoDecoder_GetDefaultStartSettings(&videoProgram);
    videoProgram.codec = VIDEO_CODEC;
    videoProgram.pidChannel = videoPidChannel;
    videoProgram.stcChannel = stcChannel;
    NEXUS_AudioDecoder_GetDefaultStartSettings(&audioProgram);
    audioProgram.codec = NEXUS_AudioCodec_ePcmWav;
    /*audioProgram.pidChannel = audioPidChannel;*/
    /*audioProgram.stcChannel = stcChannel;*/
    audioProgram.latencyMode = NEXUS_AudioDecoderLatencyMode_eNormal;
    NEXUS_AudioDecoder_GetDefaultStartSettings(&remoteProgram);

    if (argc == 2)
    {
        if (strcmp("-g711", argv[1]) == 0)
        {
            remoteProgram.codec = NEXUS_AudioCodec_eG711;
        }
        else if (strcmp("-g729", argv[1]) == 0)
        {
            remoteProgram.codec = NEXUS_AudioCodec_eG729;
        }
        else if (strcmp("-g723", argv[1]) == 0)
        {
            remoteProgram.codec = NEXUS_AudioCodec_eG723_1;
        }
        else
        {
            remoteProgram.codec = AUDIO_CODEC;
        }
    }
    else
    {
        remoteProgram.codec = AUDIO_CODEC;
    }
    /*remoteProgram.pidChannel = remotePidChannel;
    remoteProgram.stcChannel = stcChannel;*/
    remoteProgram.latencyMode = NEXUS_AudioDecoderLatencyMode_eNormal;

    /* Bring up the primary audio decoder */
    localDecoder = NEXUS_AudioDecoder_Open(0, NULL);

    /* Open audio descriptor decoder */
    NEXUS_AudioDecoder_GetDefaultOpenSettings(&audioOpenSettings);
    audioOpenSettings.type = NEXUS_AudioDecoderType_eAudioDescriptor;
    remoteDecoder = NEXUS_AudioDecoder_Open(1, &audioOpenSettings);

    /* Open the echo canceller */
    echoCanceller = NEXUS_EchoCanceller_Create(NULL);

    /* Open the encoder */
    NEXUS_AudioEncoder_GetDefaultSettings(&encoderSettings);
    encoderSettings.codec = remoteProgram.codec;
    audioEncoder = NEXUS_AudioEncoder_Open(&encoderSettings);
    audioMuxOutput = NEXUS_AudioMuxOutput_Create(NULL);

    NEXUS_AudioOutput_AddInput(NEXUS_AudioMuxOutput_GetConnector(audioMuxOutput), NEXUS_AudioEncoder_GetConnector(audioEncoder));
    NEXUS_AudioEncoder_AddInput(audioEncoder, NEXUS_EchoCanceller_GetConnector(echoCanceller));

    /* Add the two decoders to the echo module */
    NEXUS_EchoCanceller_AddLocalInput(echoCanceller, NEXUS_AudioDecoder_GetConnector(localDecoder, NEXUS_AudioDecoderConnectorType_eMono));
    NEXUS_EchoCanceller_AddRemoteInput(echoCanceller, NEXUS_AudioDecoder_GetConnector(remoteDecoder, NEXUS_AudioDecoderConnectorType_eMono));

#if NEXUS_NUM_AUDIO_DACS
    /* Add the remote decoder to the DAC */
    NEXUS_AudioOutput_AddInput(NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]), NEXUS_AudioDecoder_GetConnector(remoteDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
#endif

#if NEXUS_NUM_AUDIO_DUMMY_OUTPUTS
    /* Add the local decoder to the dummysink */
    NEXUS_AudioOutput_AddInput(NEXUS_AudioDummyOutput_GetConnector(platformConfig.outputs.audioDummy[0]), NEXUS_AudioDecoder_GetConnector(localDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
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

    /* Start Decoders */
    /* NEXUS_VideoDecoder_Start(vdecode, &videoProgram); */

    /* Setup audio encoder and mux output, start mux output before decoders...  */

    NEXUS_Playback_GetDefaultPidChannelSettings(&pidSettings);
    pidSettings.pidSettings.pidType = NEXUS_PidType_eAudio;
    pidSettings.pidSettings.pidTypeSettings.audio.codec = audioProgram.codec;
    pidSettings.pidTypeSettings.audio.primary = localDecoder;
    audioProgram.pidChannel = NEXUS_Playback_OpenPidChannel(playback, 1, &pidSettings);
    NEXUS_Playback_GetDefaultStartSettings(&playbackStartSettings);
    playbackStartSettings.mode = NEXUS_PlaybackMode_eAutoBitrate;
    NEXUS_Playback_Start(playback, file, &playbackStartSettings);

    NEXUS_Playback_GetDefaultPidChannelSettings(&rpidSettings);
    rpidSettings.pidSettings.pidType = NEXUS_PidType_eAudio;
    rpidSettings.pidSettings.pidTypeSettings.audio.codec = audioProgram.codec;
    rpidSettings.pidTypeSettings.audio.primary = remoteDecoder;
    remoteProgram.pidChannel = NEXUS_Playback_OpenPidChannel(rplayback, 1, &rpidSettings);
    NEXUS_Playback_GetDefaultStartSettings(&rplaybackStartSettings);
    rplaybackStartSettings.mode = NEXUS_PlaybackMode_eAutoBitrate;
    NEXUS_Playback_Start(rplayback, rfile, &rplaybackStartSettings);

    /* encoders/mux require different STC broadcast mode from decoder */
    NEXUS_StcChannel_GetDefaultSettings(1, &stcSettings);
    stcSettings.timebase = NEXUS_Timebase_e0;/* should be the same timebase for end-to-end locking */
    stcSettings.mode = NEXUS_StcChannelMode_eAuto;
    stcSettings.pcrBits = NEXUS_StcChannel_PcrBits_eFull42;/* ViCE2 requires 42-bit STC broadcast */
    stcChannelTranscode = NEXUS_StcChannel_Open(1, &stcSettings);

    /* Start audio mux output */
    NEXUS_AudioMuxOutput_GetDefaultStartSettings(&audioMuxStartSettings);
    audioMuxStartSettings.stcChannel = NULL;
    NEXUS_AudioMuxOutput_Start(audioMuxOutput, &audioMuxStartSettings);

#if 1
    BDBG_WRN(("Starting Remote"));
    NEXUS_AudioDecoder_Start(remoteDecoder, &remoteProgram);
#endif
    BDBG_WRN(("Starting Local"));
    NEXUS_AudioDecoder_Start(localDecoder, &audioProgram);

#if 0
    /* Print status while decoding */
    for (;;) {
        NEXUS_VideoDecoderStatus status;
        NEXUS_AudioDecoderStatus audioStatus;
        const NEXUS_AudioMuxOutputFrame *pBuf1, *pBuf2;
        size_t size1, size2;
        uint32_t stc;

        NEXUS_VideoDecoder_GetStatus(vdecode, &status);
        NEXUS_StcChannel_GetStc(videoProgram.stcChannel, &stc);
        printf("decode %.4dx%.4d, pts %#x, stc %#x (diff %d) fifo=%d%%\n",
            status.source.width, status.source.height, status.pts, stc, status.ptsStcDifference, status.fifoSize?(status.fifoDepth*100)/status.fifoSize:0);
        NEXUS_AudioDecoder_GetStatus(localDecoder, &audioStatus);
        printf("audio            pts %#x, stc %#x (diff %d) fifo=%d%%\n",
            audioStatus.pts, stc, audioStatus.ptsStcDifference, audioStatus.fifoSize?(audioStatus.fifoDepth*100)/audioStatus.fifoSize:0);
        NEXUS_AudioDecoder_GetStatus(remoteDecoder, &audioStatus);
        if ( audioStatus.started )
        {
            printf("descriptor        pts %#x, stc %#x (diff %d) fifo=%d%%\n",
                audioStatus.pts, stc, audioStatus.ptsStcDifference, audioStatus.fifoSize?(audioStatus.fifoDepth*100)/audioStatus.fifoSize:0);
        }

        /* Throw away all the mux output data */
        NEXUS_AudioMuxOutput_GetBuffer(audioMuxOutput, &pBuf1, &size1, &pBuf2, &size2);
        /*if ( size1+size2 > 0 )
            NEXUS_AudioMuxOutput_ReadComplete(audioMuxOutput, size1+size2);*/

        BKNI_Sleep(100);
    }
#else
    /*for(;;) {
        NEXUS_PlaybackStatus playbackStatus;
        NEXUS_Error rc;

        rc = BKNI_WaitForEvent(event, 1000);
        if(rc!=NEXUS_TIMEOUT) {
            break;
        }

        if(NEXUS_Playback_GetStatus(playback,&playbackStatus)==NEXUS_SUCCESS) {
            printf("audio position=%u.%03u (%u,%u) sec \n", (unsigned)playbackStatus.position/1000, (unsigned)playbackStatus.position%1000, (unsigned)playbackStatus.first/1000, (unsigned)playbackStatus.last/1000 );
        }
    }*/

    {
        size_t bytes=0;
        FILE *fOut[2];
        void *bufferBase;
        NEXUS_AudioMuxOutput_GetStatus(audioMuxOutput, &audioMuxStatus);
        NEXUS_MemoryBlock_Lock(audioMuxStatus.bufferBlock, &bufferBase);
        fprintf(stderr, "audioMuxStatus.bufferBlock: 0x%p (0x%p)\n", (void*)audioMuxStatus.bufferBlock, (void*)bufferBase);

        fOut[0] = fopen("MuxCdbOut", "wb");
        fOut[1] = fopen("MuxItbOut", "w");

        for(;;) {
            size_t size[2];
            const NEXUS_AudioMuxOutputFrame *desc[2];
            unsigned i,j;
            unsigned descs;

            NEXUS_AudioMuxOutput_GetBuffer(audioMuxOutput, &desc[0], &size[0], &desc[1], &size[1]);
            if(size[0]==0 && size[1]==0) {
                NEXUS_AudioDecoderStatus astatus;

                NEXUS_AudioDecoder_GetStatus(localDecoder, &astatus);
                fflush(fOut[0]);
                fprintf(stderr, "written %u bytes.... decode:%u\t\r", bytes, astatus.pts);
                BKNI_Sleep(30);
                continue;
            }
            for(descs=0,j=0;j<2;j++) {
                descs+=size[j];
                for(i=0;i<size[j];i++) {
                    if(desc[j][i].length > 0)
                    {
                        fwrite((const uint8_t *)bufferBase + desc[j][i].offset, desc[j][i].length, 1, fOut[0]);
                        fprintf(fOut[1], "%8x %8x   %08x%08x %8x %5u %5d %8x %8x\n", desc[j][i].flags, desc[j][i].originalPts,
                            (uint32_t)(desc[j][i].pts>>32), (uint32_t)(desc[j][i].pts & 0xffffffff), desc[j][i].escr,
                            desc[j][i].ticksPerBit, desc[j][i].shr, desc[j][i].offset, desc[j][i].length);
                    }
                    bytes+= desc[j][i].length;
                }
            }
            NEXUS_AudioMuxOutput_ReadComplete(audioMuxOutput, descs);
            fflush(fOut[0]);
            fflush(fOut[1]);
        }
        NEXUS_MemoryBlock_Unlock(audioMuxStatus.bufferBlock);
    }

    printf("Press ENTER to stop decode\n");
    getchar();

    /* example shutdown */
    NEXUS_AudioDecoder_Stop(localDecoder);
    NEXUS_AudioDecoder_Stop(remoteDecoder);
    NEXUS_Playback_Stop(playback);
    NEXUS_Playback_Stop(rplayback);
#if NEXUS_NUM_AUDIO_DACS
    NEXUS_AudioOutput_RemoveAllInputs(NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]));
#endif

    NEXUS_Playback_ClosePidChannel(playback, audioProgram.pidChannel);
    NEXUS_Playback_ClosePidChannel(rplayback, remoteProgram.pidChannel);
    NEXUS_Playback_Destroy(playback);
    NEXUS_Playback_Destroy(rplayback);
    NEXUS_Playpump_Close(playpump);
    NEXUS_Playpump_Close(rplaypump);

    NEXUS_AudioDecoder_Close(localDecoder);
    NEXUS_AudioDecoder_Close(remoteDecoder);
    /*NEXUS_VideoWindow_Close(window);*/
    NEXUS_Display_Close(display);
    /*NEXUS_VideoDecoder_Close(vdecode);*/
    NEXUS_StcChannel_Close(stcChannel);
    NEXUS_PidChannel_Close(videoPidChannel);
    /*NEXUS_PidChannel_Close(audioPidChannel);*/
    NEXUS_Platform_Uninit();
#endif

#else
        BSTD_UNUSED(argc);
        BSTD_UNUSED(argv);
#endif /* NEXUS_AUDIO_MODULE_FAMILY == NEXUS_AUDIO_MODULE_FAMILY_APE_RAAGA */
    return 0;
}

