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

#if NEXUS_NUM_HDMI_INPUTS && NEXUS_HAS_AUDIO && NEXUS_HAS_AUDIO_MUX_OUTPUT
#include "nexus_stc_channel.h"
#include "nexus_display.h"
#include "nexus_video_window.h"
#include "nexus_video_input.h"
#include "nexus_spdif_output.h"
#include "nexus_component_output.h"
#if NEXUS_HAS_HDMI_OUTPUT
#include "nexus_hdmi_output.h"
#endif
#include "nexus_hdmi_input.h"
#include "nexus_audio_mux_output.h"
#include "nexus_audio_encoder.h"
#include "nexus_audio_decoder.h"
#include "nexus_audio_mixer.h"
#include "nexus_audio_input.h"

#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "bstd.h"
#include "bkni.h"

BDBG_MODULE(encode_hdmi_audio);


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

void print_usage(void)
{
    printf("\nencode_hdmi_audio [-h] [-samplerate HZ]:\n");
    printf("\nOptions:\n");
    printf("  -h              - to print the usage info\n");
    printf("  -samplerate HZ  - to specify encoder sample rate in HZ.\n");
}

int main(int argc, char **argv)
{
    NEXUS_PlatformSettings platformSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_StcChannelSettings stcSettings;
    NEXUS_StcChannelHandle stcChannelTranscode;
    NEXUS_DisplayHandle display;
    NEXUS_DisplaySettings displaySettings;
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_HdmiOutputStatus hdmiStatus;
    NEXUS_Error rc;
#endif

    NEXUS_HdmiInputHandle hdmiInput;
    NEXUS_HdmiInputSettings hdmiInputSettings;
    NEXUS_TimebaseSettings timebaseSettings;
    NEXUS_AudioDecoderHandle audioDecoder;
    NEXUS_AudioDecoderStartSettings audioProgram;

    NEXUS_AudioMixerSettings audioMixerSettings;
    NEXUS_AudioMixerHandle audioMixer;
    NEXUS_AudioMuxOutputHandle audioMuxOutput;
    NEXUS_AudioMuxOutputStatus audioMuxStatus;
    NEXUS_AudioMuxOutputStartSettings audioMuxStartSettings;
    NEXUS_AudioEncoderSettings encoderSettings;
    NEXUS_AudioEncoderHandle audioEncoder;
    NEXUS_VideoWindowHandle videoWindow;
    void *pDataBuffer;
    size_t bytes = 0;
    FILE *fout, *fdesc;
    int i;
    unsigned sampleRate = 48000;
    bool displayOutput = false;

    for(i=0; i<argc; i++) {
        if(!strcmp("-h",argv[i])) {
            print_usage();
            return 0;
        }
        if(!strcmp("-samplerate",argv[i])) {
            sampleRate = atoi(argv[++i]);
            fprintf(stderr, "Output sample rate set to %u Hz\n", sampleRate);
        }
        if(!strcmp("-display",argv[i])) {
            displayOutput = true;
            fprintf(stderr, "Enabled local display audio.\n");
        }
    }
    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init(&platformSettings);
    NEXUS_Platform_GetConfiguration(&platformConfig);

    /* Bring up all modules for a platform in a default configuraiton for this platform */
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

    /* encoders/mux require different STC broadcast mode from decoder */
    NEXUS_StcChannel_GetDefaultSettings(1, &stcSettings);
    stcSettings.timebase = NEXUS_Timebase_e0;/* should be the same timebase for end-to-end locking */
    stcSettings.mode = NEXUS_StcChannelMode_eAuto;
    stcSettings.pcrBits = NEXUS_StcChannel_PcrBits_eFull42;/* ViCE2 requires 42-bit STC broadcast */
    stcSettings.autoConfigTimebase = false; /* don't let encoder stc auto config timebase */
    stcChannelTranscode = NEXUS_StcChannel_Open(1, &stcSettings);

    /* Bring up video display and outputs */
    NEXUS_Display_GetDefaultSettings(&displaySettings);
    displaySettings.format = NEXUS_VideoFormat_e480p;
    display = NEXUS_Display_Open(0, &displaySettings);

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

    videoWindow = NEXUS_VideoWindow_Open(display, 0);
    assert(videoWindow);

    NEXUS_VideoWindow_AddInput(videoWindow, NEXUS_HdmiInput_GetVideoConnector(hdmiInput));

    /* Open the audio decoder */
    audioDecoder = NEXUS_AudioDecoder_Open(0, NULL);

    /* Set up decoder Start structures now. We need to know the audio codec to properly set up the audio outputs. */
    NEXUS_AudioDecoder_GetDefaultStartSettings(&audioProgram);
    audioProgram.input = NEXUS_HdmiInput_GetAudioConnector(hdmiInput);
    NEXUS_StcChannel_GetDefaultSettings(0, &stcSettings);
    stcSettings.timebase = NEXUS_Timebase_e0;
    stcSettings.autoConfigTimebase = false; /* hdmi input module will auto config timebase clock */
    audioProgram.stcChannel = NEXUS_StcChannel_Open(0, &stcSettings);

    /* Open audio mux output */
    audioMuxOutput = NEXUS_AudioMuxOutput_Create(NULL);
    BDBG_ASSERT(audioMuxOutput);

    /* Open audio encoder */
    NEXUS_AudioEncoder_GetDefaultSettings(&encoderSettings);
    encoderSettings.codec = NEXUS_AudioCodec_eAac;
    audioEncoder = NEXUS_AudioEncoder_Open(&encoderSettings);
    BDBG_ASSERT(audioEncoder);

    /* Open audio mixer.  The mixer can be left running at all times to provide continuous audio output despite input discontinuities.  */
    NEXUS_AudioMixer_GetDefaultSettings(&audioMixerSettings);
    audioMixerSettings.mixUsingDsp = true;
    audioMixer = NEXUS_AudioMixer_Open(&audioMixerSettings);
    BDBG_ASSERT(audioMixer);
#if NEXUS_NUM_AUDIO_DUMMY_OUTPUTS
    NEXUS_AudioOutput_AddInput(
        NEXUS_AudioDummyOutput_GetConnector(platformConfig.outputs.audioDummy[0]), NEXUS_AudioMixer_GetConnector(audioMixer));
#endif

    /* Connect audio decoders to outputs */
    if(displayOutput) {
#if NEXUS_NUM_AUDIO_DACS
        NEXUS_AudioOutput_AddInput(
            NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]),
            NEXUS_AudioMixer_GetConnector(audioMixer));
#endif
#if NEXUS_NUM_HDMI_OUTPUTS
        NEXUS_AudioOutput_AddInput(
            NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]),
            NEXUS_AudioMixer_GetConnector(audioMixer));
#endif
    }

    /* Connect decoder to mixer and set as master */
    NEXUS_AudioMixer_AddInput(audioMixer,
        NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
    audioMixerSettings.master = NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo);
    audioMixerSettings.outputSampleRate = sampleRate;
    NEXUS_AudioMixer_SetSettings(audioMixer, &audioMixerSettings);

    /* Connect mixer to encoder */
    NEXUS_AudioEncoder_AddInput(audioEncoder, NEXUS_AudioMixer_GetConnector(audioMixer));

    /* Connect mux to encoder */
    NEXUS_AudioOutput_AddInput(
        NEXUS_AudioMuxOutput_GetConnector(audioMuxOutput), NEXUS_AudioEncoder_GetConnector(audioEncoder));

    /* Start audio mux output */
    NEXUS_AudioMuxOutput_GetDefaultStartSettings(&audioMuxStartSettings);
    audioMuxStartSettings.stcChannel = stcChannelTranscode;
    NEXUS_AudioMuxOutput_Start(audioMuxOutput, &audioMuxStartSettings);

    /* start mixer */
    NEXUS_AudioMixer_Start(audioMixer);

    /* Start decoders */
    NEXUS_AudioDecoder_Start(audioDecoder, &audioProgram);

    NEXUS_AudioMuxOutput_GetStatus(audioMuxOutput, &audioMuxStatus);
    NEXUS_MemoryBlock_Lock(audioMuxStatus.bufferBlock, &pDataBuffer);

    /* open file for ES capture */
    fprintf(stderr, "\n\tencode hdmi audio file: videos/hdmi_audio.aac\n\n");
    fout = fopen("videos/hdmi_audio.aac","wb");
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
            fd_set rfds;
            struct timeval tv;
            int retval;

            NEXUS_AudioDecoder_GetStatus(audioDecoder, &astatus);
            fflush(fout);
            fprintf(stderr, "written %lu bytes.... decode:%u\t\r", (unsigned long)bytes, astatus.pts);
            /* Watch stdin (fd 0) to see when it has input. */
            FD_ZERO(&rfds);
            FD_SET(0, &rfds);

            /* Wait up to 50 miliseconds. */
            tv.tv_sec = 0;
            tv.tv_usec = 50000;

            retval = select(1, &rfds, NULL, NULL, &tv);
            /* Don't rely on the value of tv now! */

            if (retval == -1)
                printf("select()\n");
            else if (retval)
            {
                BDBG_MSG(("Data is available now."));
                /* FD_ISSET(0, &rfds) will be true. */
                if(getchar() == 'q') break;
            }

            continue;
        }
        for(descs=0,j=0;j<2;j++) {
            descs+=size[j];
            for(i=0;i<size[j];i++) {
                if(desc[j][i].length > 0)
                {
                    if((desc[j][i].flags & NEXUS_AUDIOMUXOUTPUTFRAME_FLAG_METADATA) ==0) {/* ignore metadata descriptor in es capture */
                        fwrite((const uint8_t *)pDataBuffer + desc[j][i].offset, desc[j][i].length, 1, fout);
                    }
                    fprintf(fdesc, "%8x %8x   %08x%08x %8x %u %5d %8x %lux\n", desc[j][i].flags, desc[j][i].originalPts,
                        (uint32_t)(desc[j][i].pts>>32), (uint32_t)(desc[j][i].pts & 0xffffffff), desc[j][i].escr,
                        desc[j][i].ticksPerBit, desc[j][i].shr, desc[j][i].offset, (unsigned long)(desc[j][i].length));
                }
                bytes+= desc[j][i].length;
            }
        }

        NEXUS_AudioMuxOutput_ReadComplete(audioMuxOutput, descs);
        fflush(fout);
        fflush(fdesc);
    }

    /* Bring down system */
    NEXUS_AudioDecoder_Stop(audioDecoder);
    NEXUS_AudioMixer_Stop(audioMixer);
    NEXUS_AudioMuxOutput_Stop(audioMuxOutput);

    NEXUS_AudioOutput_RemoveAllInputs(NEXUS_AudioMuxOutput_GetConnector(audioMuxOutput));
    NEXUS_AudioEncoder_RemoveAllInputs(audioEncoder);
    NEXUS_AudioInput_Shutdown(NEXUS_AudioEncoder_GetConnector(audioEncoder));
    NEXUS_AudioEncoder_Close(audioEncoder);
#if NEXUS_NUM_AUDIO_DUMMY_OUTPUTS
    NEXUS_AudioOutput_RemoveAllInputs(NEXUS_AudioDummyOutput_GetConnector(platformConfig.outputs.audioDummy[0]));
    NEXUS_AudioOutput_Shutdown(NEXUS_AudioDummyOutput_GetConnector(platformConfig.outputs.audioDummy[0]));
#endif
    NEXUS_AudioMixer_RemoveAllInputs(audioMixer);
    NEXUS_AudioInput_Shutdown(NEXUS_AudioMixer_GetConnector(audioMixer));
    NEXUS_AudioMixer_Close(audioMixer);
    NEXUS_AudioInput_Shutdown(NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
    NEXUS_AudioDecoder_Close(audioDecoder);
    NEXUS_AudioOutput_Shutdown(NEXUS_AudioMuxOutput_GetConnector(audioMuxOutput));
    NEXUS_AudioMuxOutput_Destroy(audioMuxOutput);

    NEXUS_VideoInput_Shutdown(NEXUS_HdmiInput_GetVideoConnector(hdmiInput));
    NEXUS_VideoWindow_Close(videoWindow);
    NEXUS_Display_Close(display);
    NEXUS_StcChannel_Close(audioProgram.stcChannel);
    NEXUS_StcChannel_Close(stcChannelTranscode);
    NEXUS_HdmiInput_Close(hdmiInput);
    NEXUS_Platform_Uninit();

    return 0;
}
#else
int main(void)
{
	printf("This application is not supported on this platform!\n");
	return 0;
}
#endif
