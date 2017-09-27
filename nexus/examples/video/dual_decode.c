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
******************************************************************************/
/* Nexus example app:
Dual audio/video decode to two separate displays.
This was designed for the 7400. While there is dual audio decode, there is only one compressed audio path.
*/

#include "nexus_platform.h"
#include <stdio.h>
#if NEXUS_HAS_DISPLAY && NEXUS_HAS_AUDIO
#include "nexus_pid_channel.h"
#include "nexus_parser_band.h"
#include "nexus_video_decoder.h"
#include "nexus_stc_channel.h"
#include "nexus_display.h"
#include "nexus_video_window.h"
#include "nexus_audio_mixer.h"
#include "nexus_audio_dac.h"
#include "nexus_audio_output.h"
#include "nexus_audio_decoder.h"
#include "nexus_spdif_output.h"
#include "nexus_composite_output.h"
#include "nexus_component_output.h"
#if NEXUS_HAS_HDMI_OUTPUT
#include "nexus_hdmi_output.h"
#endif
#include "bstd.h"
#include "bkni.h"
#include <stdlib.h>

/* the following define the input and its characteristics -- these will vary by input */
#define TRANSPORT_TYPE NEXUS_TransportType_eTs
#define VIDEO_CODEC NEXUS_VideoCodec_eMpeg2
#define AUDIO_CODEC NEXUS_AudioCodec_eAc3
#define VIDEO_PID_0 0x11
#define AUDIO_PID_0 0x14
#define VIDEO_PID_1 0x31
#define AUDIO_PID_1 0x34

int main(void)
{
    NEXUS_VideoDecoderHandle vdecode0, vdecode1;
    NEXUS_ParserBand parserBand0 = NEXUS_ParserBand_e0;
    NEXUS_ParserBand parserBand1 = NEXUS_ParserBand_e1;
    NEXUS_ParserBandSettings parserBandSettings;
    NEXUS_StcChannelSettings stcSettings;
    NEXUS_DisplayHandle display0, display1;
    NEXUS_VideoWindowHandle window0, window1;
    NEXUS_AudioDecoderHandle pcmDecoder0;
    NEXUS_VideoDecoderStartSettings videoProgram0, videoProgram1;
    NEXUS_AudioDecoderStartSettings audioProgram0;
    NEXUS_PlatformSettings platformSettings;
    NEXUS_PlatformConfiguration platformConfig;
#if NEXUS_NUM_DISPLAYS > 2 && NEXUS_NUM_COMPOSITE_OUTPUTS > 1
    NEXUS_DisplayHandle display2;
    NEXUS_VideoWindowHandle window2;
#endif
#if NEXUS_NUM_AUDIO_DACS > 1
    NEXUS_AudioDecoderHandle pcmDecoder1;
    NEXUS_AudioDecoderStartSettings audioProgram1;
#endif
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_HdmiOutputStatus hdmiStatus;
    NEXUS_DisplaySettings displaySettings;
    NEXUS_Error rc;
#endif

    /* Bring up all modules for a platform in a default configuraiton for this platform */
    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init(&platformSettings);
    NEXUS_Platform_GetConfiguration(&platformConfig);

    /* It's important to use two parser bands so that the default maxDataRate of 25 Mbps per parser band will not be exceeded. */
    NEXUS_ParserBand_GetSettings(parserBand0, &parserBandSettings);
    parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eInputBand;
    NEXUS_Platform_GetStreamerInputBand(0, &parserBandSettings.sourceTypeSettings.inputBand);
    parserBandSettings.transportType = TRANSPORT_TYPE;
    NEXUS_ParserBand_SetSettings(parserBand0, &parserBandSettings);

    NEXUS_ParserBand_GetSettings(parserBand1, &parserBandSettings);
    parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eInputBand;
    NEXUS_Platform_GetStreamerInputBand(0, &parserBandSettings.sourceTypeSettings.inputBand);
    parserBandSettings.transportType = TRANSPORT_TYPE;
    NEXUS_ParserBand_SetSettings(parserBand1, &parserBandSettings);

    /* Open the pid channels */
    NEXUS_VideoDecoder_GetDefaultStartSettings(&videoProgram0);
    videoProgram0.codec = VIDEO_CODEC;
    videoProgram0.pidChannel = NEXUS_PidChannel_Open(parserBand0, VIDEO_PID_0, NULL);
    NEXUS_AudioDecoder_GetDefaultStartSettings(&audioProgram0);
    audioProgram0.codec = AUDIO_CODEC;
    audioProgram0.pidChannel = NEXUS_PidChannel_Open(parserBand0, AUDIO_PID_0, NULL);

    NEXUS_VideoDecoder_GetDefaultStartSettings(&videoProgram1);
    videoProgram1.codec = VIDEO_CODEC;
    videoProgram1.pidChannel = NEXUS_PidChannel_Open(parserBand1, VIDEO_PID_1, NULL);
#if NEXUS_NUM_AUDIO_DACS > 1
    NEXUS_AudioDecoder_GetDefaultStartSettings(&audioProgram1);
    audioProgram1.codec = AUDIO_CODEC;
    audioProgram1.pidChannel = NEXUS_PidChannel_Open(parserBand1, AUDIO_PID_1, NULL);
#endif

    /* Open two StcChannels */
    NEXUS_StcChannel_GetDefaultSettings(0, &stcSettings);
    stcSettings.timebase = NEXUS_Timebase_e0;
    stcSettings.mode = NEXUS_StcChannelMode_ePcr; /* live */
    stcSettings.modeSettings.pcr.pidChannel = videoProgram0.pidChannel; /* PCR on video pid */
    audioProgram0.stcChannel = videoProgram0.stcChannel = NEXUS_StcChannel_Open(0, &stcSettings);

    NEXUS_StcChannel_GetDefaultSettings(1, &stcSettings);
    stcSettings.timebase = NEXUS_Timebase_e1;
    stcSettings.mode = NEXUS_StcChannelMode_ePcr; /* live */
    stcSettings.modeSettings.pcr.pidChannel = videoProgram1.pidChannel; /* PCR on video pid */
    videoProgram1.stcChannel = NEXUS_StcChannel_Open(1, &stcSettings);
#if NEXUS_NUM_AUDIO_DACS > 1
    audioProgram1.stcChannel = videoProgram1.stcChannel;
#endif

    /* Bring up audio decoders and connect to outputs */
    pcmDecoder0 = NEXUS_AudioDecoder_Open(0, NULL);

#if NEXUS_NUM_AUDIO_DACS
    if (platformConfig.outputs.audioDacs[0]) {
        NEXUS_AudioOutput_AddInput(NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]),
                               NEXUS_AudioDecoder_GetConnector(pcmDecoder0, NEXUS_AudioDecoderConnectorType_eStereo));
    }
#endif
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_AudioOutput_AddInput(NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]),
                               NEXUS_AudioDecoder_GetConnector(pcmDecoder0, NEXUS_AudioDecoderConnectorType_eStereo));
#endif

#if NEXUS_NUM_AUDIO_DACS > 1
    pcmDecoder1 = NEXUS_AudioDecoder_Open(1, NULL);

    if (platformConfig.outputs.audioDacs[1]) {
        NEXUS_AudioOutput_AddInput(NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[1]),
                               NEXUS_AudioDecoder_GetConnector(pcmDecoder1, NEXUS_AudioDecoderConnectorType_eStereo));
    }
#endif

    /* Bring up video display and outputs */
    display0 = NEXUS_Display_Open(0, NULL);
    window0 = NEXUS_VideoWindow_Open(display0, 0);
#if NEXUS_NUM_COMPONENT_OUTPUTS
    NEXUS_Display_AddOutput(display0, NEXUS_ComponentOutput_GetConnector(platformConfig.outputs.component[0]));
#endif
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_Display_AddOutput(display0, NEXUS_HdmiOutput_GetVideoConnector(platformConfig.outputs.hdmi[0]));
    rc = NEXUS_HdmiOutput_GetStatus(platformConfig.outputs.hdmi[0], &hdmiStatus);
    if ( !rc && hdmiStatus.connected )
    {
        /* If current display format is not supported by monitor, switch to monitor's preferred format. 
           If other connected outputs do not support the preferred format, a harmless error will occur. */
        NEXUS_Display_GetSettings(display0, &displaySettings);
        if ( !hdmiStatus.videoFormatSupported[displaySettings.format] ) {
            displaySettings.format = hdmiStatus.preferredVideoFormat;
            NEXUS_Display_SetSettings(display0, &displaySettings);
		}
    }
#endif

#if NEXUS_NUM_DISPLAYS > 2 && NEXUS_NUM_COMPOSITE_OUTPUTS > 1
    display2 = NEXUS_Display_Open(2, NULL);
    NEXUS_Display_AddOutput(display2, NEXUS_CompositeOutput_GetConnector(platformConfig.outputs.composite[0]));
    window2 = NEXUS_VideoWindow_Open(display2, 0);
#endif

    display1 = NEXUS_Display_Open(1, NULL);
    if (!display1) {
        printf("This is not a dual decode capable platform.\n");
        exit(1);
    }
    window1 = NEXUS_VideoWindow_Open(display1, 0);
#if NEXUS_NUM_DISPLAYS > 2 && NEXUS_NUM_COMPOSITE_OUTPUTS > 1
    NEXUS_Display_AddOutput(display1, NEXUS_CompositeOutput_GetConnector(platformConfig.outputs.composite[1]));
#else
    NEXUS_Display_AddOutput(display1, NEXUS_CompositeOutput_GetConnector(platformConfig.outputs.composite[0]));
#endif

    /* bring up decoder and connect to display */
    vdecode0 = NEXUS_VideoDecoder_Open(0, NULL); /* take default capabilities */
    NEXUS_VideoWindow_AddInput(window0, NEXUS_VideoDecoder_GetConnector(vdecode0));
#if NEXUS_NUM_COMPOSITE_OUTPUTS > 1  && NEXUS_NUM_DISPLAYS > 2
    NEXUS_VideoWindow_AddInput(window2, NEXUS_VideoDecoder_GetConnector(vdecode0));
#endif
    vdecode1 = NEXUS_VideoDecoder_Open(1, NULL); /* take default capabilities */
    if (!vdecode1) {
        printf("This is not a dual decode capable platform.\n");
        exit(1);
    }
    NEXUS_VideoWindow_AddInput(window1, NEXUS_VideoDecoder_GetConnector(vdecode1));

    /* Start Decoders */
    NEXUS_VideoDecoder_Start(vdecode0, &videoProgram0);
    NEXUS_AudioDecoder_Start(pcmDecoder0, &audioProgram0);
    NEXUS_VideoDecoder_Start(vdecode1, &videoProgram1);
#if NEXUS_NUM_AUDIO_DACS > 1
     NEXUS_AudioDecoder_Start(pcmDecoder1, &audioProgram1);
#endif


    for (;;) {
        NEXUS_VideoDecoderStatus status;
        uint32_t stc;

        NEXUS_VideoDecoder_GetStatus(vdecode0, &status);
        NEXUS_StcChannel_GetStc(videoProgram0.stcChannel, &stc);
        printf("decode0 %dx%d, pts %#x, stc %#x (diff %d)\n",
            status.source.width, status.source.height, status.pts, stc, status.ptsStcDifference);
        NEXUS_VideoDecoder_GetStatus(vdecode1, &status);
        NEXUS_StcChannel_GetStc(videoProgram1.stcChannel, &stc);
        printf("decode1 %dx%d, pts %#x, stc %#x (diff %d)\n",
            status.source.width, status.source.height, status.pts, stc, status.ptsStcDifference);

        BKNI_Sleep(1000);
    }

    getchar();
    return 0;
}
#else
int main(void)
{
    printf("This application is not supported on this platform\n");
    return 0;
}
#endif
