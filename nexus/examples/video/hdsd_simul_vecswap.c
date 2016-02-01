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
/* Nexus example app: single live a/v decode to HD and SD outputs, swap vec  */

#include "nexus_platform.h"
#include <stdio.h>
#if NEXUS_HAS_DISPLAY && NEXUS_NUM_DISPLAYS > 1
#include "nexus_pid_channel.h"
#include "nexus_parser_band.h"
#include "nexus_video_decoder.h"
#include "nexus_stc_channel.h"
#include "nexus_display.h"
#include "nexus_video_window.h"
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
#include <stdio.h>

/* the following define the input and its characteristics -- these will vary by input */
#define TRANSPORT_TYPE NEXUS_TransportType_eTs
#define VIDEO_CODEC NEXUS_VideoCodec_eMpeg2
#define AUDIO_CODEC NEXUS_AudioCodec_eAc3
#define VIDEO_PID 0x11
#define AUDIO_PID 0x14

#ifdef LINUX
#include <sys/time.h>

#define TIMESTAMP_GET(ptime) gettimeofday(ptime, NULL)
#define TIMESTAMP_USED_IN_MSEC(time1, time2) \
    ((time2.tv_sec-time1.tv_sec)*1000+(time2.tv_usec-time1.tv_usec)/1000)

typedef struct timeval TIMESTAMP;
#endif

int main(void)
{
    NEXUS_VideoDecoderHandle vdecode;
    NEXUS_VideoDecoderStartSettings videoProgram;
    NEXUS_ParserBand parserBand = NEXUS_ParserBand_e0;
    NEXUS_ParserBandSettings parserBandSettings;
    NEXUS_StcChannelSettings stcSettings;
    NEXUS_DisplaySettings displaySettings;
    NEXUS_DisplayHandle display0, display1;
    NEXUS_VideoWindowHandle window0, window1;
    NEXUS_AudioDecoderHandle pcmDecoder, compressedDecoder;
    NEXUS_AudioDecoderStartSettings audioProgram;
    NEXUS_PlatformSettings platformSettings;
    NEXUS_PlatformConfiguration platformConfig;
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_HdmiOutputStatus hdmiStatus;
    NEXUS_Error rc;
#endif
    char   ch = 'c';
    bool   bVecSwap = true;
#ifdef LINUX
    TIMESTAMP startTime, endTime;
#endif

    /* Bring up all modules for a platform in a default configuraiton for this platform */
    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init(&platformSettings);
    NEXUS_Platform_GetConfiguration(&platformConfig);

    /* Map input band and parser band. This could be eliminated because a straight mapping and TS config is a good default. */
    NEXUS_ParserBand_GetSettings(parserBand, &parserBandSettings);
    parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eInputBand;
    NEXUS_Platform_GetStreamerInputBand(0, &parserBandSettings.sourceTypeSettings.inputBand);
    parserBandSettings.transportType = TRANSPORT_TYPE;
    NEXUS_ParserBand_SetSettings(parserBand, &parserBandSettings);

    /* Open the pid channels */
    NEXUS_VideoDecoder_GetDefaultStartSettings(&videoProgram);
    videoProgram.codec = VIDEO_CODEC;
    videoProgram.pidChannel = NEXUS_PidChannel_Open(parserBand, VIDEO_PID, NULL);
    NEXUS_AudioDecoder_GetDefaultStartSettings(&audioProgram);
    audioProgram.codec = AUDIO_CODEC;
    audioProgram.pidChannel = NEXUS_PidChannel_Open(parserBand, AUDIO_PID, NULL);

    /* By default, StcChannel will configure NEXUS_Timebase with the info it has */
    NEXUS_StcChannel_GetDefaultSettings(0, &stcSettings);
    stcSettings.timebase = NEXUS_Timebase_e0;
    stcSettings.mode = NEXUS_StcChannelMode_ePcr; /* live */
    stcSettings.modeSettings.pcr.pidChannel = videoProgram.pidChannel; /* PCR on video pid */
    audioProgram.stcChannel = videoProgram.stcChannel = NEXUS_StcChannel_Open(0, &stcSettings);

    /* Bring up audio decoders and connect to outputs */
    pcmDecoder = NEXUS_AudioDecoder_Open(0, NULL);
    compressedDecoder = NEXUS_AudioDecoder_Open(1, NULL);
#if NEXUS_NUM_AUDIO_DACS
    NEXUS_AudioOutput_AddInput(NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]),
                               NEXUS_AudioDecoder_GetConnector(pcmDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
#endif
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_AudioOutput_AddInput(NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]),
                               NEXUS_AudioDecoder_GetConnector(pcmDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
#endif
#if NEXUS_NUM_SPDIF_OUTPUTS
    if ( audioProgram.codec == NEXUS_AudioCodec_eAc3 )
    {
        /* Only pass through AC3 */
        NEXUS_AudioOutput_AddInput(NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]),
                                   NEXUS_AudioDecoder_GetConnector(compressedDecoder, NEXUS_AudioDecoderConnectorType_eCompressed));
    }
    else
    {
        NEXUS_AudioOutput_AddInput(NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]),
                                   NEXUS_AudioDecoder_GetConnector(pcmDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
    }
#endif

    /* bring up HD and HDMI  */
    NEXUS_Display_GetDefaultSettings(&displaySettings);
    displaySettings.format = NEXUS_VideoFormat_e1080i;
    display0 = NEXUS_Display_Open(0, &displaySettings);
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

    /* bring up CVBS and SVideo  */
    display1 = NEXUS_Display_Open(1, NULL);
    window1 = NEXUS_VideoWindow_Open(display1, 0);
#if NEXUS_NUM_COMPOSITE_OUTPUTS
    NEXUS_Display_AddOutput(display1, NEXUS_CompositeOutput_GetConnector(platformConfig.outputs.composite[0]));
#endif
#if NEXUS_NUM_SVIDEO_OUTPUTS
    NEXUS_Display_AddOutput(display1, NEXUS_SvideoOutput_GetConnector(platformConfig.outputs.svideo[0]));
#endif

    /* bring up decoder and connect to display */
    vdecode = NEXUS_VideoDecoder_Open(0, NULL); /* take default capabilities */
    NEXUS_VideoWindow_AddInput(window0, NEXUS_VideoDecoder_GetConnector(vdecode));
    NEXUS_VideoWindow_AddInput(window1, NEXUS_VideoDecoder_GetConnector(vdecode));

    /* Start Decoders */
    NEXUS_VideoDecoder_Start(vdecode, &videoProgram);
    NEXUS_AudioDecoder_Start(pcmDecoder, &audioProgram);
    if ( audioProgram.codec == NEXUS_AudioCodec_eAc3 )
    {
        NEXUS_AudioDecoder_Start(compressedDecoder, &audioProgram);
    }

    /* Switch between 2 configurations:
     * Config 1:
     *      bVecSwap is true
     *      Secondary VEC: Comp + HDMI, format 1080i
     *      Primary VEC  : Cvbs + Svideo, format 480i
     * Config 2:
     *      bVecSwap is false
     *      Secondary VEC: Comp + Cvbs + Svideo, format 480i
     *      Primary VEC  : HDMI, format 480p
     */
    while(ch != 'q')
    {
        printf("==========================================================\n");
        printf("Enter to switch configuration: ");
        ch = getchar();

        /* Swap vec */
        bVecSwap = !bVecSwap;
        if(bVecSwap)
        {
            printf("Switch to Config1: \n");
            printf("\t Secondary vec: Comp + HDMI, format 1080i, \n");
            printf("\t Primary vec  : Cvbs + Svideo, format 480i, \n");
        }
        else
        {
            printf("Switch to Config2: \n");
            printf("\t Primary vec  HDMI, format 480p\n");
            printf("\t Secondary vec: Comp + Cvbs + Svideo, format 480i\n");
        }

#ifdef LINUX
        /* Get start time */
        TIMESTAMP_GET(&startTime);
#endif

        /* Stop decoder */
        NEXUS_VideoDecoder_Stop(vdecode);
        NEXUS_AudioDecoder_Stop(pcmDecoder);
        if ( audioProgram.codec == NEXUS_AudioCodec_eAc3 )
        {
            NEXUS_AudioDecoder_Stop(compressedDecoder);
        }

        /* Close Window and display */
        NEXUS_Display_RemoveAllOutputs(display0);
        NEXUS_VideoWindow_RemoveAllInputs(window0);
        NEXUS_VideoWindow_Close(window0);
        NEXUS_Display_Close(display0);

        NEXUS_Display_RemoveAllOutputs(display1);
        NEXUS_VideoWindow_RemoveAllInputs(window1);
        NEXUS_VideoWindow_Close(window1);
        NEXUS_Display_Close(display1);

        /* bring up display 0 */
        NEXUS_Display_GetDefaultSettings(&displaySettings);
        if(bVecSwap)
        {
            /* Use secondary vec */
            displaySettings.vecIndex = 1;
        }
        else
        {
            /* Use primary vec */
            displaySettings.vecIndex = 0;
        }
        displaySettings.format = NEXUS_VideoFormat_e480p;
        display0 = NEXUS_Display_Open(0, &displaySettings);
        window0 = NEXUS_VideoWindow_Open(display0, 0);
#if NEXUS_NUM_COMPONENT_OUTPUTS
        if((platformConfig.outputs.component[0]) && bVecSwap)
        {
            NEXUS_Display_AddOutput(display0, NEXUS_ComponentOutput_GetConnector(platformConfig.outputs.component[0]));
        }
#endif
#if NEXUS_NUM_HDMI_OUTPUTS
        NEXUS_Display_AddOutput(display0, NEXUS_HdmiOutput_GetVideoConnector(platformConfig.outputs.hdmi[0]));
#endif

        /* bring up display 1 */
        NEXUS_Display_GetDefaultSettings(&displaySettings);
        displaySettings.format = NEXUS_VideoFormat_eNtsc;
        if(bVecSwap)
        {
            /* Use primary vec */
            displaySettings.vecIndex = 0;
        }
        else
        {
            /* Use secondary vec */
            displaySettings.vecIndex = 1;
        }
        display1 = NEXUS_Display_Open(1, &displaySettings);
        window1 = NEXUS_VideoWindow_Open(display1, 0);
#if NEXUS_NUM_COMPONENT_OUTPUTS
        if((platformConfig.outputs.component[0]) && !bVecSwap)
        {
            NEXUS_Display_AddOutput(display1, NEXUS_ComponentOutput_GetConnector(platformConfig.outputs.component[0]));
        }
#endif
#if NEXUS_NUM_COMPOSITE_OUTPUTS
        NEXUS_Display_AddOutput(display1, NEXUS_CompositeOutput_GetConnector(platformConfig.outputs.composite[0]));
#endif
#if NEXUS_NUM_SVIDEO_OUTPUTS
        NEXUS_Display_AddOutput(display1, NEXUS_SvideoOutput_GetConnector(platformConfig.outputs.svideo[0]));
#endif

        NEXUS_VideoWindow_AddInput(window0, NEXUS_VideoDecoder_GetConnector(vdecode));
        NEXUS_VideoWindow_AddInput(window1, NEXUS_VideoDecoder_GetConnector(vdecode));

        NEXUS_VideoDecoder_Start(vdecode, &videoProgram);
        NEXUS_AudioDecoder_Start(pcmDecoder, &audioProgram);
        if ( audioProgram.codec == NEXUS_AudioCodec_eAc3 )
        {
            NEXUS_AudioDecoder_Start(compressedDecoder, &audioProgram);
        }


#ifdef LINUX
        TIMESTAMP_GET(&endTime);
        printf("startTime: %ld(sec) %ld(usec)\n", startTime.tv_sec, startTime.tv_usec);
        printf("endTime  : %ld(sec) %ld(usec)\n", endTime.tv_sec, endTime.tv_usec);
        printf("Turn-around time for vec swap is %ld milliseconds \n",
            TIMESTAMP_USED_IN_MSEC(startTime, endTime));
#endif

    }
    return 0;
}
#else
int main(void)
{
    printf("This application is not supported on this platform\n");
    return 0;
}
#endif
