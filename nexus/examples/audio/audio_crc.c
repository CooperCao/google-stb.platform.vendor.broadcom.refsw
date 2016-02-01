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

/* Nexus example app: dual audio CRC from playback buffer or output buffer */

#include "nexus_platform.h"
#include <stdio.h>

#if NEXUS_HAS_AUDIO
#include "nexus_pid_channel.h"
#include "nexus_parser_band.h"
#include "nexus_video_decoder.h"
#include "nexus_stc_channel.h"
#include "nexus_display.h"
#include "nexus_video_window.h"
#include "nexus_video_input.h"
#include "nexus_audio_dac.h"
#include "nexus_audio_decoder.h"
#include "nexus_audio_output.h"
#include "nexus_audio_input.h"
#include "nexus_spdif_output.h"
#include "nexus_audio_crc.h"
#include "nexus_composite_output.h"
#include "nexus_component_output.h"
#if NEXUS_HAS_HDMI_OUTPUT
#include "nexus_hdmi_output.h"
#endif
#include "bstd.h"
#include "bkni.h"
#include <stdlib.h>

/* the following define the input file and its characteristics -- these will vary by input file */
#define TRANSPORT_TYPE NEXUS_TransportType_eTs
#define VIDEO_CODEC NEXUS_VideoCodec_eMpeg2
#define AUDIO_CODEC NEXUS_AudioCodec_eAc3
#define VIDEO_PID 0x11
#define AUDIO_PID 0x14

#define CRC_SFIFO       1
#define CRC_MAX_READ    100
#define CRC_HDMI_PAIRS  3
#define CRC_SPDIF_PAIRS 1

static void Crc_PrintCrcData (char * name, NEXUS_AudioCrcHandle handle, unsigned numChPairs);

int main(void)
{
    NEXUS_PlatformSettings platformSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_InputBand inputBand;
    NEXUS_ParserBand parserBand;
    NEXUS_ParserBandSettings parserBandSettings;
    NEXUS_StcChannelHandle stcChannel;
    NEXUS_StcChannelSettings stcSettings;
    NEXUS_PidChannelHandle videoPidChannel, audioPidChannel;
    NEXUS_DisplayHandle display;
    NEXUS_DisplaySettings displaySettings;
    NEXUS_VideoWindowHandle window;
    NEXUS_VideoDecoderHandle vdecode;
    NEXUS_VideoDecoderStartSettings videoProgram;
    NEXUS_AudioDecoderHandle pcmDecoder, compressedDecoder;
    NEXUS_AudioDecoderStartSettings audioProgram;
    NEXUS_AudioInput spdifInput;
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_HdmiOutputStatus hdmiStatus;
    NEXUS_Error rc;
#endif
    NEXUS_AudioCrcHandle crcHandleSpdif, crcHandleHdmi;

    /* Bring up all modules for a platform in a default configuration for this platform */
    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init(&platformSettings);
    NEXUS_Platform_GetConfiguration(&platformConfig);

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

    /* Open the StcChannel to do lipsync with the PCR */
    NEXUS_StcChannel_GetDefaultSettings(0, &stcSettings);
    stcSettings.timebase = NEXUS_Timebase_e0;
    stcSettings.mode = NEXUS_StcChannelMode_ePcr; /* live */
    stcSettings.modeSettings.pcr.pidChannel = videoPidChannel; /* PCR happens to be on video pid */
    stcChannel = NEXUS_StcChannel_Open(0, &stcSettings);

    /* Set up decoder Start structures now. We need to know the audio codec to properly set up
    the audio outputs. */
    NEXUS_VideoDecoder_GetDefaultStartSettings(&videoProgram);
    videoProgram.codec = VIDEO_CODEC;
    videoProgram.pidChannel = videoPidChannel;
    videoProgram.stcChannel = stcChannel;
    NEXUS_AudioDecoder_GetDefaultStartSettings(&audioProgram);
    audioProgram.codec = AUDIO_CODEC;
    audioProgram.pidChannel = audioPidChannel;
    audioProgram.stcChannel = stcChannel;

    /* Bring up audio decoders and outputs */
    pcmDecoder = NEXUS_AudioDecoder_Open(0, NULL);
    compressedDecoder = NEXUS_AudioDecoder_Open(1, NULL);
#if NEXUS_NUM_AUDIO_DACS
    NEXUS_AudioOutput_AddInput(
        NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]),
        NEXUS_AudioDecoder_GetConnector(pcmDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
#endif
    printf("%s: pcmStereo  %p\n", __FUNCTION__, (void*)NEXUS_AudioDecoder_GetConnector(pcmDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
    printf("%s: pcmMch     %p\n", __FUNCTION__, (void*)NEXUS_AudioDecoder_GetConnector(pcmDecoder, NEXUS_AudioDecoderConnectorType_eMultichannel));
    printf("%s: compressed %p\n", __FUNCTION__, (void*)NEXUS_AudioDecoder_GetConnector(compressedDecoder, NEXUS_AudioDecoderConnectorType_eCompressed));

#if NEXUS_NUM_SPDIF_OUTPUTS
    printf("%s: SPDIF %p\n", __FUNCTION__, (void*)NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]));
#endif
#if NEXUS_NUM_HDMI_OUTPUTS
    printf("%s: HDMI  %p\n", __FUNCTION__, (void*)NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]));
#endif
#if NEXUS_NUM_AUDIO_DACS
    printf("%s: DAC   %p\n", __FUNCTION__, (void*)NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]));
#endif

    if ( audioProgram.codec == NEXUS_AudioCodec_eAc3 )
    {
        /* Only pass through AC3 */
        spdifInput = NEXUS_AudioDecoder_GetConnector(compressedDecoder, NEXUS_AudioDecoderConnectorType_eCompressed);
    }
    else
    {
        spdifInput = NEXUS_AudioDecoder_GetConnector(pcmDecoder, NEXUS_AudioDecoderConnectorType_eStereo);
    }

#if NEXUS_NUM_SPDIF_OUTPUTS
    NEXUS_AudioOutput_AddInput(
        NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]),
        spdifInput);
#endif

#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_AudioOutput_AddInput(
        NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]),
        NEXUS_AudioDecoder_GetConnector(pcmDecoder, NEXUS_AudioDecoderConnectorType_eMultichannel));
#endif

    {
        NEXUS_AudioCrcOpenSettings crcOpenSettings;
        NEXUS_AudioCrcInputSettings crcInputSettings;

        NEXUS_AudioCrc_GetDefaultOpenSettings(&crcOpenSettings);
        crcOpenSettings.dataWidth = 24;
        crcOpenSettings.initialValue = 0;
        crcOpenSettings.captureMode = NEXUS_AudioCrcMode_eFreeRun;
        crcOpenSettings.numChannelPairs = CRC_SPDIF_PAIRS;
        crcOpenSettings.numEntries = 48000/256; 
        crcOpenSettings.samplingPeriod = 48000/2;
        crcHandleSpdif = NEXUS_AudioCrc_Open(0, &crcOpenSettings);
        crcOpenSettings.numChannelPairs = CRC_HDMI_PAIRS;
        crcHandleHdmi = NEXUS_AudioCrc_Open(1, &crcOpenSettings);

#if CRC_SFIFO
        NEXUS_AudioCrc_GetDefaultInputSettings(&crcInputSettings);
        crcInputSettings.sourceType = NEXUS_AudioCrcSourceType_ePlaybackBuffer;
        crcInputSettings.input = spdifInput;
#if NEXUS_NUM_SPDIF_OUTPUTS
        crcInputSettings.output = NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]);
#endif
        NEXUS_AudioCrc_SetInput(crcHandleSpdif, &crcInputSettings);
        crcInputSettings.input = NEXUS_AudioDecoder_GetConnector(pcmDecoder, NEXUS_AudioDecoderConnectorType_eMultichannel);
#if NEXUS_NUM_HDMI_OUTPUTS
        crcInputSettings.output = NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]);
#endif
        NEXUS_AudioCrc_SetInput(crcHandleHdmi, &crcInputSettings);
#else
        NEXUS_AudioCrc_GetDefaultInputSettings(&crcInputSettings);
        crcInputSettings.sourceType = NEXUS_AudioCrcSourceType_eOutputPort;
#if NEXUS_NUM_SPDIF_OUTPUTS
        crcInputSettings.output = NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]);
#endif
        NEXUS_AudioCrc_SetInput(crcHandleSpdif, &crcInputSettings);
#if NEXUS_NUM_HDMI_OUTPUTS
        crcInputSettings.output = NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]);
#endif
        NEXUS_AudioCrc_SetInput(crcHandleHdmi, &crcInputSettings);
#endif
    }

    /* Bring up display and outputs */
    NEXUS_Display_GetDefaultSettings(&displaySettings);
    displaySettings.format = NEXUS_VideoFormat_eNtsc;
    display = NEXUS_Display_Open(0, &displaySettings);
#if NEXUS_NUM_COMPONENT_OUTPUTS
    NEXUS_Display_AddOutput(display, NEXUS_ComponentOutput_GetConnector(platformConfig.outputs.component[0]));
#endif 
#if NEXUS_NUM_COMPOSITE_OUTPUTS
    NEXUS_Display_AddOutput(display, NEXUS_CompositeOutput_GetConnector(platformConfig.outputs.composite[0]));
#endif
#if NEXUS_NUM_SVIDEO_OUTPUTS    
    NEXUS_Display_AddOutput(display, NEXUS_SvideoOutput_GetConnector(platformConfig.outputs.svideo[0]));
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

    window = NEXUS_VideoWindow_Open(display, 0);

    /* bring up decoder and connect to display */
    vdecode = NEXUS_VideoDecoder_Open(0, NULL); /* take default capabilities */
    NEXUS_VideoWindow_AddInput(window, NEXUS_VideoDecoder_GetConnector(vdecode));

    /* Start Decoders */
    NEXUS_VideoDecoder_Start(vdecode, &videoProgram);
    NEXUS_AudioDecoder_Start(pcmDecoder, &audioProgram);
    if ( audioProgram.codec == NEXUS_AudioCodec_eAc3 )
    {
        /* Only pass through AC3 */
        NEXUS_AudioDecoder_Start(compressedDecoder, &audioProgram);
    }

    for (;;)
    {
        Crc_PrintCrcData("SPDIF", crcHandleSpdif, CRC_SPDIF_PAIRS);
        Crc_PrintCrcData("HDMI", crcHandleHdmi, CRC_HDMI_PAIRS);
        BKNI_Sleep(2000);
    }

    printf("Press ENTER to stop decode\n");
    getchar();

    /* example shutdown */
    NEXUS_AudioDecoder_Stop(pcmDecoder);
    if ( audioProgram.codec == NEXUS_AudioCodec_eAc3 )
    {
        NEXUS_AudioDecoder_Stop(compressedDecoder);
    }

    NEXUS_AudioCrc_ClearInput(crcHandleSpdif);
    NEXUS_AudioCrc_ClearInput(crcHandleHdmi);
    NEXUS_AudioCrc_Close(crcHandleSpdif);
    NEXUS_AudioCrc_Close(crcHandleHdmi);
    NEXUS_AudioDecoder_Close(pcmDecoder);
    NEXUS_AudioDecoder_Close(compressedDecoder);
    NEXUS_VideoDecoder_Stop(vdecode);
    NEXUS_VideoWindow_Close(window);
    NEXUS_Display_Close(display);
    NEXUS_VideoDecoder_Close(vdecode);
    NEXUS_StcChannel_Close(stcChannel);
    NEXUS_PidChannel_Close(videoPidChannel);
    NEXUS_PidChannel_Close(audioPidChannel);
    NEXUS_Platform_Uninit();

    return 0;
}


NEXUS_AudioCrcEntry * Crc_GetEntryByIndex(
    const NEXUS_AudioCrcData * crcData,
    unsigned index
    )

{
    BDBG_ASSERT(crcData != NULL);

    switch ( index )
    {
        default:
        case 0:
            return (NEXUS_AudioCrcEntry*)&crcData->crc0;
            break;
        case 1:
            return (NEXUS_AudioCrcEntry*)&crcData->crc1;
            break;
        case 2:
            return (NEXUS_AudioCrcEntry*)&crcData->crc2;
            break;
        case 3:
            return (NEXUS_AudioCrcEntry*)&crcData->crc3;
            break;
    }

    return NULL;
}

static void Crc_PrintCrcData (char * name, NEXUS_AudioCrcHandle handle, unsigned numChPairs)
{
    NEXUS_AudioCrcData entries[CRC_MAX_READ];
    NEXUS_AudioCrcEntry * pEntry;
    unsigned numEntries = 0;
    NEXUS_Error err;
    unsigned i, j;

    err = NEXUS_AudioCrc_GetCrcData(handle, entries, CRC_MAX_READ, &numEntries);
    if ( err != BERR_SUCCESS || numEntries == 0 )
    {
        printf("%s - no data\n", __FUNCTION__);
        return;
    }

    printf("%s - NEXUS_AudioCrc_GetCrcData(%p) returned %d entries\n", name, (void*)handle, numEntries);
    for ( i = 0; i < numEntries; i++)
    {
        for ( j = 0; j < numChPairs; j++ )
        {
            pEntry = Crc_GetEntryByIndex(&(entries[i]), j);
            printf("\t%s:chPair[%d] - seqNumber %d, value %d\n", name, j, 
                pEntry->seqNumber, 
                pEntry->value);
        }
    }
}
#else /* NEXUS_HAS_AUDIO */
int main(void)
{
    printf("This application is not supported on this platform (needs audio)!\n");
    return 0;
}
#endif
