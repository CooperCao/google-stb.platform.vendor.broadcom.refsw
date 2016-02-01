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

#if NEXUS_HAS_AUDIO
#include "nexus_video_decoder.h"
#include "nexus_stc_channel.h"
#include "nexus_display.h"
#include "nexus_video_window.h"
#include "nexus_video_input.h"
#include "nexus_spdif_output.h"
#include "nexus_component_output.h"
#include "nexus_composite_output.h"
#if NEXUS_HAS_HDMI_OUTPUT
#include "nexus_hdmi_output.h"
#endif
#if NEXUS_HAS_PLAYBACK && NEXUS_HAS_AUDIO_MUX_OUTPUT
#include "nexus_playback.h"
#include "nexus_record.h"
#include "nexus_file.h"
#include "nexus_recpump.h"
#include "nexus_record.h"
#include "nexus_audio_mux_output.h"
#include "nexus_audio_encoder.h"
#endif

#include <assert.h>
#include "bstd.h"
#include "bkni.h"

/* Choose one of following: */
#define BTST_AC3_TRANSCODER_PASSTHRU    1
#define BTST_MPG_TRANSCODER_PASSTHRU    0
#define BTST_AAC_TRANSCODER_PASSTHRU    0

#ifdef BTST_AC3_TRANSCODER_PASSTHRU
  #define FILE_NAME "videos/avatar_AVC_15M.ts" /*AC3*/
  #define TRANSPORT_TYPE NEXUS_TransportType_eTs
  #define AUDIO_CODEC NEXUS_AudioCodec_eAc3
  #define AUDIO_PID 0x104
  #define PCR_PID 0x101
#elif BTST_MPG_TRANSCODER_PASSTHRU
  #define FILE_NAME "videos/cnnticker.mpg" /*MPG*/
  #define TRANSPORT_TYPE NEXUS_TransportType_eTs
  #define AUDIO_CODEC NEXUS_AudioCodec_eMpeg
  #define AUDIO_PID 0x22
  #define PCR_PID 0x21
#elif BTST_AAC_TRANSCODER_PASSTHRU
  #define FILE_NAME "videos/testjun26_188.mpg" /*AAC*/
  #define TRANSPORT_TYPE NEXUS_TransportType_eTs
  #define AUDIO_CODEC NEXUS_AudioCodec_eAac
  #define AUDIO_PID 0x110
  #define PCR_PID 0x1ff
#else
  #error You must choose one of the options
#endif

/* set to 0 to show decode only test */
#define BTST_ENABLE_TRANSCODE            1
#define BTST_ENABLE_PASSTHROUGH          0

int main(void)
{
#if NEXUS_HAS_PLAYBACK && NEXUS_HAS_AUDIO_MUX_OUTPUT
    NEXUS_PlatformSettings platformSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_StcChannelHandle stcChannel, stcChannelTranscode;
    NEXUS_StcChannelSettings stcSettings;
    NEXUS_PidChannelHandle audioPidChannel, pcrPidChannel;
    NEXUS_DisplayHandle display;
    NEXUS_AudioDecoderHandle audioDecoder;
#if BTST_ENABLE_PASSTHROUGH
    NEXUS_AudioDecoderHandle audioPassthrough;
#endif
    NEXUS_AudioDecoderStartSettings audioProgram;
    NEXUS_FilePlayHandle file;
    NEXUS_PlaypumpHandle playpump;
    NEXUS_PlaybackHandle playback;
    NEXUS_PlaybackSettings playbackSettings;
    NEXUS_PlaybackPidChannelSettings playbackPidSettings;
    NEXUS_AudioMuxOutputHandle audioMuxOutput;
    NEXUS_AudioMuxOutputStatus audioMuxStatus;
    NEXUS_AudioMuxOutputStartSettings audioMuxStartSettings;
#if BTST_ENABLE_TRANSCODE 
    NEXUS_AudioEncoderSettings encoderSettings;
    NEXUS_AudioEncoderHandle audioEncoder;
#endif
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_HdmiOutputStatus hdmiStatus;
    NEXUS_DisplaySettings displaySettings;
    NEXUS_Error rc;
#endif
    void *pDataBuffer;
    size_t bytes;
    FILE *fout, *fdesc;
    const char *fname = FILE_NAME;

    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init(&platformSettings);
    NEXUS_Platform_GetConfiguration(&platformConfig);

    playpump = NEXUS_Playpump_Open(0, NULL);
    assert(playpump);
    playback = NEXUS_Playback_Create();
    assert(playback);

    file = NEXUS_FilePlay_OpenPosix(fname, NULL);
    if (!file) {
        fprintf(stderr, "can't open file:%s\n", fname);
        return -1;
    }

    NEXUS_StcChannel_GetDefaultSettings(0, &stcSettings);
    stcSettings.timebase = NEXUS_Timebase_e0;
    stcSettings.mode = NEXUS_StcChannelMode_eAuto;
    stcChannel = NEXUS_StcChannel_Open(0, &stcSettings);
    
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
    playbackSettings.stcChannel = stcChannel;
    NEXUS_Playback_SetSettings(playback, &playbackSettings);

    /* Bring up video display and outputs */
    display = NEXUS_Display_Open(0, NULL);
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

    /* Open the audio decoder */
    audioDecoder = NEXUS_AudioDecoder_Open(0, NULL);
#if BTST_ENABLE_PASSTHROUGH
    audioPassthrough = NEXUS_AudioDecoder_Open(1, NULL);
#endif

    /* Open the audio and pcr pid channel */
    NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
    playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eAudio;
    playbackPidSettings.pidTypeSettings.audio.primary = audioDecoder; /* must be told codec for correct handling */
#if BTST_ENABLE_PASSTHROUGH
    playbackPidSettings.pidTypeSettings.audio.secondary = audioPassthrough;
#endif

    audioPidChannel = NEXUS_Playback_OpenPidChannel(playback, AUDIO_PID, &playbackPidSettings);
    playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eOther;
    pcrPidChannel = NEXUS_Playback_OpenPidChannel(playback, PCR_PID, &playbackPidSettings);

    /* Set up decoder Start structures now. We need to know the audio codec to properly set up the audio outputs. */
    NEXUS_AudioDecoder_GetDefaultStartSettings(&audioProgram);
    audioProgram.codec = AUDIO_CODEC;
    audioProgram.pidChannel = audioPidChannel;
    audioProgram.stcChannel = stcChannel;

    /* Connect audio decoders to outputs */
#if NEXUS_NUM_AUDIO_DACS
    NEXUS_AudioOutput_AddInput(
        NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]),
        NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
#endif
#if BTST_ENABLE_PASSTHROUGH
#if NEXUS_NUM_SPDIF_OUTPUTS
    NEXUS_AudioOutput_AddInput(
        NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]),
        NEXUS_AudioDecoder_GetConnector(audioPassthrough, NEXUS_AudioDecoderConnectorType_eCompressed));
#endif
#endif
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_AudioOutput_AddInput(
        NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]),
        NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
#endif

    /* Open audio mux output */
    audioMuxOutput = NEXUS_AudioMuxOutput_Create(NULL);
    assert(audioMuxOutput);

#if BTST_ENABLE_TRANSCODE 
    /* Open audio encoder */
    NEXUS_AudioEncoder_GetDefaultSettings(&encoderSettings);
    encoderSettings.codec = NEXUS_AudioCodec_eAacPlusLoas;
    audioEncoder = NEXUS_AudioEncoder_Open(&encoderSettings);
    assert(audioEncoder);
    /* Connect encoder to decoder */
    NEXUS_AudioEncoder_AddInput(audioEncoder,
        NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
    /* Connect mux to encoder */
    NEXUS_AudioOutput_AddInput(
        NEXUS_AudioMuxOutput_GetConnector(audioMuxOutput), NEXUS_AudioEncoder_GetConnector(audioEncoder));
#else    
    /* Connect mux to encoder */
    NEXUS_AudioOutput_AddInput(
        NEXUS_AudioMuxOutput_GetConnector(audioMuxOutput),
        NEXUS_AudioDecoder_GetConnector(audioPassthrough, NEXUS_AudioDecoderConnectorType_eCompressed));
#endif

    /* Start audio mux output */
    NEXUS_AudioMuxOutput_GetDefaultStartSettings(&audioMuxStartSettings);
    audioMuxStartSettings.stcChannel = stcChannelTranscode;
    NEXUS_AudioMuxOutput_Start(audioMuxOutput, &audioMuxStartSettings);

    /* Start decoders */
    NEXUS_AudioDecoder_Start(audioDecoder, &audioProgram);
#if BTST_ENABLE_PASSTHROUGH
    NEXUS_AudioDecoder_Start(audioPassthrough, &audioProgram);
#endif
    NEXUS_AudioMuxOutput_GetStatus(audioMuxOutput, &audioMuxStatus);
    NEXUS_MemoryBlock_Lock(audioMuxStatus.bufferBlock, &pDataBuffer);

    /* Start playback */
    NEXUS_Playback_Start(playback, file, NULL);

    fout = fopen("videos/audio.mpg","wb");
    fdesc = fopen("videos/audio_desc.txt","w");
    fprintf(fdesc, "flags(h) origPts(h)         pts(Lh)     escr(h) ticksPerBit(u) shr(d) offset(h) length(h) protocol(u)\n");

    for(;;) {
        size_t size[2];
        const NEXUS_AudioMuxOutputFrame *desc[2];
        unsigned i,j;
        unsigned descs;

        NEXUS_AudioMuxOutput_GetBuffer(audioMuxOutput, &desc[0], &size[0], &desc[1], &size[1]);
        if(size[0]==0 && size[1]==0) {
            NEXUS_AudioDecoderStatus astatus;

            NEXUS_AudioDecoder_GetStatus(audioDecoder, &astatus);
            fflush(fout);
            fprintf(stderr, "written %u bytes.... decode:%u\t\r", bytes, astatus.pts);
            BKNI_Sleep(30);
            continue;
        }
        for(descs=0,j=0;j<2;j++) {
            descs+=size[j];
            for(i=0;i<size[j];i++) {
                if(desc[j][i].length > 0)
                {
                    fwrite((const uint8_t *)pDataBuffer + desc[j][i].offset, desc[j][i].length, 1, fout);
                    fprintf(fdesc, "%8x %8x   %08x%08x %8x %5u %5d %8x %8x\n", desc[j][i].flags, desc[j][i].originalPts, 
                        (uint32_t)(desc[j][i].pts>>32), (uint32_t)(desc[j][i].pts & 0xffffffff), desc[j][i].escr, 
                        desc[j][i].ticksPerBit, desc[j][i].shr, desc[j][i].offset, desc[j][i].length);
                }
                bytes+= desc[j][i].length;
            }
        }
        NEXUS_AudioMuxOutput_ReadComplete(audioMuxOutput, descs);
        fflush(fout);
        fflush(fdesc);
    }

    /* Playback state machine is driven from inside Nexus. */
    printf("Press ENTER to quit\n");
    getchar();

    /* Bring down system */
    NEXUS_Playback_Stop(playback);
    NEXUS_FilePlay_Close(file);
#if BTST_ENABLE_PASSTHROUGH
    NEXUS_AudioDecoder_Stop(audioPassthrough);
#endif
    NEXUS_AudioDecoder_Stop(audioDecoder);
    NEXUS_Playback_Destroy(playback);
    NEXUS_Playpump_Close(playpump);
    NEXUS_AudioMuxOutput_Destroy(audioMuxOutput);
    NEXUS_Display_Close(display);
    NEXUS_StcChannel_Close(stcChannel);
    
    NEXUS_AudioDecoder_Close(audioDecoder);
#if BTST_ENABLE_PASSTHROUGH
    NEXUS_AudioDecoder_Close(audioPassthrough);
#endif    
    NEXUS_Platform_Uninit();

#else
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
