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
/* Nexus example app: single live a/v decode from an input band with lipsync and vec alignment */

#include "nexus_platform.h"
#include <stdio.h>
#if NEXUS_HAS_VIDEO_DECODER && NEXUS_HAS_SYNC_CHANNEL
#include "nexus_pid_channel.h"
#include "nexus_parser_band.h"
#include "nexus_video_decoder.h"
#include "nexus_stc_channel.h"
#include "nexus_display.h"
#include "nexus_video_window.h"
#include "nexus_audio_dac.h"
#include "nexus_audio_decoder.h"
#include "nexus_audio_output.h"
#include "nexus_spdif_output.h"
#include "nexus_component_output.h"
#include "nexus_composite_output.h"
#if NEXUS_HAS_HDMI_OUTPUT
#include "nexus_hdmi_output.h"
#endif
#include "nexus_sync_channel.h"
#include "bstd.h"
#include "bkni.h"
#include <stdlib.h>
#include <string.h>

#if NEXUS_NUM_DISPLAYS > 1
#define HDSD_SIMUL 1
#endif

/* the following define the input and its characteristics -- these will vary by input */
#define TRANSPORT_TYPE NEXUS_TransportType_eTs
#define VIDEO_CODEC NEXUS_VideoCodec_eMpeg2
#define AUDIO_CODEC NEXUS_AudioCodec_eAc3
#define VIDEO_PID 0x11
#define AUDIO_PID 0x14

struct format_mapping
{
    char * name;
    NEXUS_VideoFormat format;
} g_formats[] =
{
    { "ntsc", NEXUS_VideoFormat_eNtsc },
    { "480p", NEXUS_VideoFormat_e480p },
    { "720p", NEXUS_VideoFormat_e720p },
    { "1080i", NEXUS_VideoFormat_e1080i },
    { "pal", NEXUS_VideoFormat_ePal },
    { "576p", NEXUS_VideoFormat_e576p },
    { "720p50", NEXUS_VideoFormat_e720p50hz},
    { "1080i50", NEXUS_VideoFormat_e1080i50hz }
};

#define SUPPORTED_FORMAT_COUNT 8

int main(int argc, char **argv)
{
    NEXUS_VideoDecoderHandle vdecode;
    NEXUS_VideoDecoderStartSettings videoProgram;
    NEXUS_ParserBand parserBand = NEXUS_ParserBand_e0;
    NEXUS_ParserBandSettings parserBandSettings;
    NEXUS_StcChannelSettings stcSettings;
    NEXUS_DisplayHandle display0;
    NEXUS_VideoWindowHandle window0;
    #if HDSD_SIMUL
    NEXUS_DisplayHandle display1;
    NEXUS_VideoWindowHandle window1;
    #endif
    NEXUS_AudioDecoderHandle pcmDecoder, compressedDecoder;
    NEXUS_AudioDecoderStartSettings audioProgram;
    NEXUS_PlatformSettings platformSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_DisplaySettings displaySettings;
    NEXUS_VideoWindowSettings windowSettings;
    NEXUS_SyncChannelSettings syncChannelSettings;
    NEXUS_SyncChannelHandle syncChannel;
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_HdmiOutputStatus hdmiStatus;
    NEXUS_Error rc;
#endif
    unsigned vidpid = VIDEO_PID;
    unsigned audpid = AUDIO_PID;
    NEXUS_VideoFormat format = NEXUS_VideoFormat_eNtsc; /*default*/
#if HDSD_SIMUL
    NEXUS_VideoFormat simulFormat = NEXUS_VideoFormat_eNtsc;
    bool syncLockSd = false;
#endif

    if (argc > 1)
        vidpid = strtoul(argv[1], NULL, 0);
    if (argc > 2)
        audpid = strtoul(argv[2], NULL, 0);
    if (argc > 3) {
        unsigned i;
        for (i = 0; i < SUPPORTED_FORMAT_COUNT; i++) {
            if (!strcmp(argv[3], g_formats[i].name)) {
                format = g_formats[i].format;
                break;
            }
        }
    }
#if HDSD_SIMUL
    if ((argc > 4) && (!strcmp(argv[4], "pal")))
        simulFormat = NEXUS_VideoFormat_ePal;

    if ((argc > 5) && (!strcmp(argv[5], "sd")))
        syncLockSd = true;
#endif

    /* Bring up all modules for a platform in a default configuration for this platform */
    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
#if HDSD_SIMUL
    /* Use drop-frame always on HD display to match NTSC SD display, for PAL we don't do drop-frame */
    platformSettings.displayModuleSettings.dropFrame = simulFormat == NEXUS_VideoFormat_eNtsc;
#endif
    NEXUS_Platform_Init(&platformSettings);
    NEXUS_Platform_GetConfiguration(&platformConfig);

    /* create a sync channel */
    NEXUS_SyncChannel_GetDefaultSettings(&syncChannelSettings);
    syncChannel = NEXUS_SyncChannel_Create(&syncChannelSettings);

    /* Map input band and parser band. This could be eliminated because a straight mapping and TS config is a good default. */
    NEXUS_ParserBand_GetSettings(parserBand, &parserBandSettings);
    parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eInputBand;
    NEXUS_Platform_GetStreamerInputBand(0, &parserBandSettings.sourceTypeSettings.inputBand);
    parserBandSettings.transportType = TRANSPORT_TYPE;
    NEXUS_ParserBand_SetSettings(parserBand, &parserBandSettings);

    /* Open the pid channels */
    NEXUS_VideoDecoder_GetDefaultStartSettings(&videoProgram);
    videoProgram.codec = VIDEO_CODEC;
    videoProgram.pidChannel = NEXUS_PidChannel_Open(parserBand, vidpid, NULL);
    NEXUS_AudioDecoder_GetDefaultStartSettings(&audioProgram);
    audioProgram.codec = AUDIO_CODEC;
    audioProgram.pidChannel = NEXUS_PidChannel_Open(parserBand, audpid, NULL);

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
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_AudioOutput_AddInput(NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]),
                               NEXUS_AudioDecoder_GetConnector(pcmDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
#endif

    NEXUS_Display_GetDefaultSettings(&displaySettings);

#if HDSD_SIMUL
    if (syncLockSd)
    {
        /* bring up SD display first, to make it the sync-locked one */
        displaySettings.displayType = NEXUS_DisplayType_eAuto;
        displaySettings.format = simulFormat;
        display1 = NEXUS_Display_Open(1, &displaySettings);
        window1 = NEXUS_VideoWindow_Open(display1, 0);
#if NEXUS_NUM_COMPOSITE_OUTPUTS
        NEXUS_Display_AddOutput(display1, NEXUS_CompositeOutput_GetConnector(platformConfig.outputs.composite[0]));
#endif
        /* bring up HD display second */
        displaySettings.displayType = NEXUS_DisplayType_eAuto;
        displaySettings.format = format;
        displaySettings.alignmentTarget = display1; /* set the alignment target to the SD display */
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
    }
    else
#endif
    {
        /* bring up HD display first */
        displaySettings.displayType = NEXUS_DisplayType_eAuto;
        displaySettings.format = format;
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
#if HDSD_SIMUL
        /* bring up SD display second */
        displaySettings.displayType = NEXUS_DisplayType_eAuto;
        displaySettings.format = simulFormat;
        display1 = NEXUS_Display_Open(1, &displaySettings);
        window1 = NEXUS_VideoWindow_Open(display1, 0);
#if NEXUS_NUM_COMPOSITE_OUTPUTS
        NEXUS_Display_AddOutput(display1, NEXUS_CompositeOutput_GetConnector(platformConfig.outputs.composite[0]));
#endif
        NEXUS_Display_GetSettings(display0, &displaySettings);
        displaySettings.alignmentTarget = display1;
        NEXUS_Display_SetSettings(display0, &displaySettings);
#endif
    }

    /* bring up decoder and connect to display */
    vdecode = NEXUS_VideoDecoder_Open(0, NULL); /* take default capabilities */
    NEXUS_VideoWindow_AddInput(window0, NEXUS_VideoDecoder_GetConnector(vdecode));
#if HDSD_SIMUL
    NEXUS_VideoWindow_AddInput(window1, NEXUS_VideoDecoder_GetConnector(vdecode));
#endif

    /* window on display 0 is set to auto master by default, need to disable this */
    NEXUS_VideoWindow_GetSettings(window0, &windowSettings);
    windowSettings.autoMaster = false;
    NEXUS_VideoWindow_SetSettings(window0, &windowSettings);

    /* set frame rate master to NULL on both displays */
    NEXUS_Display_GetSettings(display0, &displaySettings);
    displaySettings.frameRateMaster = NULL;
    NEXUS_Display_SetSettings(display0, &displaySettings);

#if HDSD_SIMUL
    NEXUS_Display_GetSettings(display1, &displaySettings);
    displaySettings.frameRateMaster = NULL;
    NEXUS_Display_SetSettings(display1, &displaySettings);
#endif

    /* connect sync channel */
    NEXUS_SyncChannel_GetSettings(syncChannel, &syncChannelSettings);
    syncChannelSettings.videoInput = NEXUS_VideoDecoder_GetConnector(vdecode);
    syncChannelSettings.audioInput[0] = NEXUS_AudioDecoder_GetConnector(pcmDecoder, NEXUS_AudioDecoderConnectorType_eStereo);
    syncChannelSettings.audioInput[1] = NEXUS_AudioDecoder_GetConnector(compressedDecoder, NEXUS_AudioDecoderConnectorType_eCompressed);
    syncChannelSettings.enablePrecisionLipsync = false;
    NEXUS_SyncChannel_SetSettings(syncChannel, &syncChannelSettings);

    /* Start Decoders */
    NEXUS_VideoDecoder_Start(vdecode, &videoProgram);
    NEXUS_AudioDecoder_Start(pcmDecoder, &audioProgram);
    if ( audioProgram.codec == NEXUS_AudioCodec_eAc3 )
    {
        NEXUS_AudioDecoder_Start(compressedDecoder, &audioProgram);
    }

	printf("Press return to stop\n");
    getchar();

    /* Stop decoders */
    if ( audioProgram.codec == NEXUS_AudioCodec_eAc3 )
    {
        NEXUS_AudioDecoder_Stop(compressedDecoder);
    }
    NEXUS_AudioDecoder_Stop(pcmDecoder);
    NEXUS_VideoDecoder_Stop(vdecode);

    /* disconnect sync channel */
    NEXUS_SyncChannel_GetSettings(syncChannel, &syncChannelSettings);
    syncChannelSettings.videoInput = NULL;
    syncChannelSettings.audioInput[0] = NULL;
    syncChannelSettings.audioInput[1] = NULL;
    NEXUS_SyncChannel_SetSettings(syncChannel, &syncChannelSettings);

    return 0;
}
#else
int main(void)
{
    printf("This application is not supported on this platform\n");
    return -1;
}
#endif
