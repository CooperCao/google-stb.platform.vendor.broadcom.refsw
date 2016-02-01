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
******************************************************************************/
/* Nexus example app: handling of 3DTV SEI frame packing messages. 
   To keep the example minimal, this example only illustrates how to receive and parse the SEI message.
   Refer to the other 3DTV examples on how to configure your platform for 3D. */

#include "nexus_platform.h"
#include <stdio.h>
#if NEXUS_HAS_FRONTEND && NEXUS_HAS_VIDEO_DECODER
#include "nexus_pid_channel.h"
#include "nexus_parser_band.h"
#include "nexus_frontend.h"
#include "nexus_video_decoder.h"
#include "nexus_stc_channel.h"
#include "nexus_display.h"
#include "nexus_video_window.h"
#include "nexus_video_input.h"
#include "nexus_audio_dac.h"
#include "nexus_audio_decoder.h"
#include "nexus_audio_output.h"
#include "nexus_audio_input.h"
#include "nexus_component_output.h"
#if NEXUS_HAS_HDMI_OUTPUT
#include "nexus_hdmi_output.h"
#endif
#include "nexus_video_decoder_extra.h"
#include "bstd.h"
#include "bkni.h"
#include <stdio.h>
#include <stdlib.h>

/* the following define the input and its characteristics -- these will vary by input */
#define TRANSPORT_TYPE NEXUS_TransportType_eTs
#define NUM_PROGRAMS 2
struct {
    unsigned short videoPid;
    unsigned short audioPid;
    NEXUS_VideoCodec videoCodec;
    NEXUS_AudioCodec audioCodec;
    int parser;
} g_programs[NUM_PROGRAMS] = {
    {0x11, 0x14, NEXUS_VideoCodec_eH264,  NEXUS_AudioCodec_eAc3, 0}, /* 3D stream with SEI on streamer */
    {0x11, 0x14, NEXUS_VideoCodec_eMpeg2, NEXUS_AudioCodec_eAc3, 1}  /* 2D stream on QAM */
};

static void callback(void *pContext, int param) 
{
    NEXUS_VideoDecoderHandle videoDecoder = (NEXUS_VideoDecoderHandle)pContext;
    NEXUS_VideoDecoder3DTVStatus status;
    bool valid;
    BSTD_UNUSED(param);

    NEXUS_VideoDecoder_Get3DTVStatus(videoDecoder, &status);
    printf("3DTV status: %s\n", (status.codec==NEXUS_VideoCodec_eH264)?"AVC":"MPEG2");
    if (status.codec==NEXUS_VideoCodec_eH264) {
        valid = status.codecData.avc.valid;
    }
    else {
        valid = status.codecData.mpeg2.valid;
    }
    printf("   valid %d, format %d\n", status.codecData.avc.valid, status.format);

    /* At this point, the application should set the necessary 3D configuration for its platform, 
       based on the reported 3DTV format. For example, DTVs should configure the video windows; 
       Settops should configure the HDMI VSI. */
}

int main(void)
{
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_InputBand inputBand;
    NEXUS_ParserBand parserBand;
    NEXUS_ParserBandSettings parserBandSettings;
    NEXUS_FrontendUserParameters userParams;
    NEXUS_FrontendQamSettings qamSettings;
    NEXUS_FrontendHandle frontend;
    NEXUS_StcChannelSettings stcSettings;
    NEXUS_DisplayHandle display;
    NEXUS_DisplaySettings displaySettings;
    NEXUS_VideoWindowHandle window;
    NEXUS_VideoDecoderHandle videoDecoder;
    NEXUS_VideoDecoderStartSettings videoProgram[NUM_PROGRAMS];
    NEXUS_AudioDecoderHandle audioDecoder;
    NEXUS_AudioDecoderStartSettings audioProgram[NUM_PROGRAMS];
    NEXUS_VideoDecoderExtendedSettings extSettings;
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_HdmiOutputStatus hdmiStatus;
#endif
    NEXUS_Error rc;
    int i;

    /* Bring up all modules for a platform in a default configuration for this platform */
    NEXUS_Platform_Init(NULL);
    NEXUS_Platform_GetConfiguration(&platformConfig);

    /* Get the streamer input band from Platform. Platform has already configured the FPGA with a default streamer routing */
    NEXUS_Platform_GetStreamerInputBand(0, &inputBand);

    /* PB0 = streamer */
    parserBand = NEXUS_ParserBand_e0;
    NEXUS_ParserBand_GetSettings(parserBand, &parserBandSettings);
    parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eInputBand;
    parserBandSettings.sourceTypeSettings.inputBand = inputBand;
    parserBandSettings.transportType = TRANSPORT_TYPE;
    NEXUS_ParserBand_SetSettings(parserBand, &parserBandSettings);

    /* PB1 = qam */
    frontend = platformConfig.frontend[0];
    NEXUS_Frontend_GetDefaultQamSettings(&qamSettings);
    qamSettings.frequency = 765 * 1000000;
    qamSettings.mode = NEXUS_FrontendQamMode_e64;
    qamSettings.annex = NEXUS_FrontendQamAnnex_eB;
    rc = NEXUS_Frontend_TuneQam(frontend, &qamSettings);
    NEXUS_Frontend_GetUserParameters(frontend, &userParams);

    NEXUS_ParserBand_GetSettings(NEXUS_ParserBand_e1, &parserBandSettings);
    if (userParams.isMtsif) {
        parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eMtsif;
        parserBandSettings.sourceTypeSettings.mtsif = NEXUS_Frontend_GetConnector(frontend); /* NEXUS_Frontend_TuneXyz() will connect this frontend to this parser band */
    }
    else {
        parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eInputBand;
        parserBandSettings.sourceTypeSettings.inputBand = userParams.param1;  /* Platform initializes this to input band */
    }
    rc = NEXUS_ParserBand_SetSettings(NEXUS_ParserBand_e1, &parserBandSettings);

    for (i=0; i<NUM_PROGRAMS; i++) {
        /* Open the audio and video pid channels */
        NEXUS_VideoDecoder_GetDefaultStartSettings(&videoProgram[i]);
        NEXUS_AudioDecoder_GetDefaultStartSettings(&audioProgram[i]);
        videoProgram[i].pidChannel = NEXUS_PidChannel_Open(g_programs[i].parser + NEXUS_ParserBand_e0, g_programs[i].videoPid, NULL);
        audioProgram[i].pidChannel = NEXUS_PidChannel_Open(g_programs[i].parser + NEXUS_ParserBand_e0, g_programs[i].audioPid, NULL);
        videoProgram[i].codec = g_programs[i].videoCodec;
        audioProgram[i].codec = g_programs[i].audioCodec;

        /* Open the StcChannel to do lipsync with the PCR */
        NEXUS_StcChannel_GetDefaultSettings(0, &stcSettings);
        stcSettings.timebase = NEXUS_Timebase_e0;
        stcSettings.mode = NEXUS_StcChannelMode_ePcr; /* live */
        stcSettings.modeSettings.pcr.pidChannel = videoProgram[i].pidChannel; /* PCR happens to be on video pid */
        videoProgram[i].stcChannel = NEXUS_StcChannel_Open(i, &stcSettings);
        audioProgram[i].stcChannel = videoProgram[i].stcChannel;
    }

    /* Bring up audio decoders and outputs */
    audioDecoder = NEXUS_AudioDecoder_Open(0, NULL);
#if NEXUS_NUM_AUDIO_DACS
    NEXUS_AudioOutput_AddInput(
        NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]),
        NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
#endif
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_AudioOutput_AddInput(
        NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]),
        NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
#endif

    /* Bring up display and outputs */
    NEXUS_Display_GetDefaultSettings(&displaySettings);
    displaySettings.format = NEXUS_VideoFormat_e1080i;
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

    /* bring up decoder and connect to display */
    videoDecoder = NEXUS_VideoDecoder_Open(0, NULL); /* take default capabilities */
    NEXUS_VideoWindow_AddInput(window, NEXUS_VideoDecoder_GetConnector(videoDecoder));

    /* register the 3DTV status change callback */
    NEXUS_VideoDecoder_GetExtendedSettings(videoDecoder, &extSettings);
    extSettings.s3DTVStatusEnabled = true;
    extSettings.s3DTVStatusChanged.callback = callback;
    extSettings.s3DTVStatusChanged.context = videoDecoder;
    NEXUS_VideoDecoder_SetExtendedSettings(videoDecoder, &extSettings);

    i = 0;
    while (1) {
        /* Start Decoders */
        printf("Starting program %d...\n", i);
        NEXUS_VideoDecoder_Start(videoDecoder, &videoProgram[i]);
        NEXUS_AudioDecoder_Start(audioDecoder, &audioProgram[i]);
        /* The 3DTV status change callback fires some time after the video decoder has started, 
           when then first SEI message is seen in the stream.
           The lack of SEI message in the stream implies 2D content. */

        getchar();
        NEXUS_VideoDecoder_Stop(videoDecoder);
        NEXUS_AudioDecoder_Stop(audioDecoder);

        if (++i == NUM_PROGRAMS) i = 0;
    }

    return 0;
}
#else
int main(void)
{
    printf("This application is not supported on this platform\n");
    return -1;
}
#endif
